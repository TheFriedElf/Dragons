#include "stdafx.h"
#include "PgMobileSuit.h"
#include "lwUI.h"
#include "PgMovieMgr.h"
#include "PgVmr.h"
#include "PgAXPlay.h"
#include "PgUIScene.h"
#include "PgSoundMan.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "Variant/PgPlayer.h"
#include "PgNiFile.h"
#include <shlobj.h>
#include "PgDshowPlay.h"
#include "PgQuestMan.h"
#include "lwQuestMan.h"

////////////////////////////////////////////////
// lwMovieMgr
lwMovieMgr::lwMovieMgr(PgMovieMgr* pMovieMgr)
{
	m_pkMovieMgr = pMovieMgr;
}
bool lwMovieMgr::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	class_<lwMovieMgr>(pkState, "MovieMgr")
		.def(pkState, constructor<PgMovieMgr *>())

		.def(pkState, "Play", &lwMovieMgr::Play)
		.def(pkState, "IsPlay", &lwMovieMgr::IsPlay)
		.def(pkState, "ScenarioMovie", &lwMovieMgr::ScenarioMovie)
		//.def(pkState, "Stop", &lwMovieMgr::Stop)
		;
	return true;
}
void lwMovieMgr::Play(char const *pcID)
{
	m_pkMovieMgr->Play(pcID);
}

bool lwMovieMgr::IsPlay()
{
	return m_pkMovieMgr->IsPlay();
}

bool lwMovieMgr::ScenarioMovie(int iMapNum)
{
	std::string strName = m_pkMovieMgr->GetSecnarioMovieName(iMapNum);
	if( !strName.empty() )
	{
		m_pkMovieMgr->Play(strName);
		return true;
	}

	return false;
}

bool lwMovieMgr::QuestMovie(int iQuestNum)
{
	std::string strName = m_pkMovieMgr->GetQuestMovieName(iQuestNum);
	if( !strName.empty() )
	{
		m_pkMovieMgr->Play(strName);
		return true;
	}
	return false;
}

////////////////////////////////////////////////
// PgMovieMgr
PgMovieMgr::PgMovieMgr()
: m_bPlay(false), m_eCurType(MT_NONE), m_dwEndTime(0), m_kVmr(NULL), m_kAXPlay(NULL), m_bIntroPlayed(false), m_kDshowPlay(NULL), m_pfnMovieFinished(NULL)
, m_dwFadeEndTime(0), m_bRenderFrame(false)
{
	//ParseXml("movie.xml");
	m_kVmr = new PgVmr;
	m_kAXPlay = new PgAXPlay;
	m_kDshowPlay = new PgDshowPlay;
}
PgMovieMgr::~PgMovieMgr()
{
	Release();

	if(m_kVmr)
	{
		delete m_kVmr;
		m_kVmr = NULL;
	}

	if(m_kAXPlay)
	{
		delete m_kAXPlay;
		m_kAXPlay = NULL;
	}

	if(m_kDshowPlay)
	{
		delete m_kDshowPlay;
		m_kDshowPlay = NULL;
	}
}

bool PgMovieMgr::Init(EMovieType eType)
{
	if(eType == MT_VMR)
	{
		m_kVmr->Init();
	}
	else if(eType == MT_ACTIVEX)
	{
		m_kAXPlay->Init();
	}
	else if(eType == MT_DSHOW ||
		eType == MT_DSHOW_INGAME)
	{
		m_kDshowPlay->Init();
	}

	return true;
}
void PgMovieMgr::Release()
{
	if(m_eCurType == MT_VMR)
	{
		if(m_kVmr->IsPlay())
		{
			m_kVmr->StopGraph();
		}
		m_kVmr->ReleaseAllInterfaces();
	}
	else if(m_eCurType == MT_ACTIVEX)
	{
		if(m_kAXPlay->IsPlay())
		{
			m_kAXPlay->Stop();
		}
		m_kAXPlay->Terminate();
	}
	else if(m_eCurType == MT_DSHOW ||
		m_eCurType == MT_DSHOW_INGAME )
	{
		if(m_kDshowPlay->IsPlay())
		{
			m_kDshowPlay->Stop();
		}
		m_kDshowPlay->Terminate();
	}
	m_pfnMovieFinished = NULL;
}
void PgMovieMgr::Terminate()
{
	if(m_kVmr->IsPlay())
	{
		m_kVmr->StopGraph();
	}
	m_kVmr->ReleaseAllInterfaces();

	if(m_kAXPlay->IsPlay())
	{
		m_kAXPlay->Stop();
	}
	m_kAXPlay->Terminate();

	if(m_kDshowPlay->IsPlay())
	{
		m_kDshowPlay->Stop();
	}
	m_kDshowPlay->Terminate();
	m_pfnMovieFinished = NULL;
}

bool PgMovieMgr::Play(std::string strID)
{
	
	NiString kString = strID.c_str();
	kString.ToUpper();
	strID = kString;

	PathContainer::iterator itr = m_smPathContainer.find(strID);
	if(itr == m_smPathContainer.end())
	{
		return false;
	}

	m_kMovieValue = itr->second;

	// 파일 있는지 체크
	if(PgNiFile::CheckFileExist(m_kMovieValue.strPath.c_str()) == false)
	{
		return false;
	}
	if( 0 == m_kMovieValue.fFadeInTime)
	{
		if( !PlayCore() )
		{
			return false;
		}
		UpdateSequence(US_DEFAULT);
	}
	else
	{
		m_dwFadeEndTime = BM::GetTime32() + (m_kMovieValue.fFadeInTime * 1000);
		SetFade(true);
		UpdateSequence(US_FADE_IN);
	}

	m_bPlay = true;
	g_kSoundMan.StopBgSound(true);
	g_kSoundMan.SetSilence(true);
	g_kSoundMan.SetEffectVolume(0.0f);

	m_eCurType = (EMovieType)m_kMovieValue.byType;
	SetEndTime(m_kMovieValue.dwTime);

	m_kCurrentPlayingMovie = UNI(strID);

	return true;
}

bool PgMovieMgr::PlayCore()
{
	if(g_pkApp && g_pkApp->GetFullscreen())
		g_pkApp->RendererChange(true, NiDX9Renderer::PRESENT_INTERVAL_ONE);

	// 초기화
	if(!Init((EMovieType)m_kMovieValue.byType))
	{
		return false;
	}

	bool bRt = false;
	if(m_kMovieValue.byType == MT_VMR)
	{
		bRt = m_kVmr->PlayGraph(m_kMovieValue.strPath);
	}
	else if(m_kMovieValue.byType == MT_ACTIVEX)
	{
		bRt = m_kAXPlay->Play(m_kMovieValue.strPath);
	}
	else if(m_kMovieValue.byType == MT_DSHOW)
	{
		bRt = m_kDshowPlay->Play(m_kMovieValue.strPath, m_kMovieValue.fStartTime, m_kMovieValue.fEndTime, m_kMovieValue.fVolumeDecTime, true);
	}
	else if(m_kMovieValue.byType == MT_DSHOW_INGAME)
	{
		bRt = m_kDshowPlay->Play(m_kMovieValue.strPath, m_kMovieValue.fStartTime, m_kMovieValue.fEndTime, m_kMovieValue.fVolumeDecTime, false);
	}

	if(!bRt)
	{
		return false;
	}

	g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MovieMgr, false));

	m_bRenderFrame = true;

	return true;
}

void PgMovieMgr::StopMovie()
{
	if(m_eCurType == MT_VMR)
	{
		m_kVmr->StopGraph();
	}
	else if(m_eCurType == MT_ACTIVEX)
	{
		m_kAXPlay->Stop();
	}
	else if(m_eCurType == MT_DSHOW ||
		m_eCurType == MT_DSHOW_INGAME )
	{
		m_kDshowPlay->Stop();
	}

	if( g_kQuestMan.IsQuestDialog() )
	{
		lwQuestMan kQuestMan(&g_kQuestMan);
		kQuestMan.NextQuestDialog(lwUIWnd(NULL));
	}

	m_bRenderFrame = false;
	m_eCurType = MT_NONE;
}

bool PgMovieMgr::Stop()
{
	if( UpdateSequence() == US_PLAY )
	{
		m_dwFadeEndTime = BM::GetTime32() + (m_kMovieValue.fFadeOutTime * 1000);
		StopMovie();
		SetFade(false);
		UpdateSequence(US_FADE_OUT);
		return true;
	}

	StopMovie();

	m_bPlay = false;

	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MovieMgr, false));

	g_kSoundMan.SetSilence(false);
	g_kSoundMan.PlayBgSound(0);
	g_kSoundMan.SetEffectVolume(g_kSoundMan.GetConfigEffectVolume());

	if (m_pfnMovieFinished != NULL)
	{
		m_pfnMovieFinished(m_kCurrentPlayingMovie);
		m_pfnMovieFinished = NULL;
		m_kCurrentPlayingMovie = TEXT("");
	}

	return true;
}

void PgMovieMgr::SetFade(bool bFadeIn)
{
	if( !g_pkWorld )
	{
		return;
	}

	DWORD dwColor = 0;
	if( bFadeIn )
	{
		g_pkWorld->SetShowWorldFocusFilter(dwColor, 0.0f, 1.0f, m_kMovieValue.fFadeInTime, true, false );
	}
	else
	{
		g_pkWorld->SetShowWorldFocusFilter(dwColor, 1.0f, 0.0f, m_kMovieValue.fFadeOutTime, false, false );
	}
}

void PgMovieMgr::Update()
{
	if(!IsPlay())
		return;

	switch( UpdateSequence() )
	{
	case US_DEFAULT:
		{
			HWND hWnd = ::GetFocus();
			if(g_pkApp)
			{
				HWND hGame = g_pkApp->GetWindowReference();
				if(hGame && hWnd != hGame)
				{
					::SetFocus(hGame);
				}
			}

			bool bRtn = false;

			switch(m_eCurType)
			{
			case MT_VMR:
				{
					bRtn = m_kVmr->Update();
				}
				break;
			case MT_ACTIVEX:
				{
					bRtn = m_kAXPlay->Update();
				}
				break;
			case MT_DSHOW:
			case MT_DSHOW_INGAME:
				{
					bRtn = m_kDshowPlay->Update();
				}
				break;
			}

			if(!bRtn)
			{
				Stop();
			}
		}break;
	case US_FADE_IN:
		{
			DWORD dwTime = BM::GetTime32();
			if( dwTime > m_dwFadeEndTime )
			{
				if( !PlayCore() )
				{
					Stop();
				}
				UpdateSequence(US_PLAY);
			}
		}break;
	case US_PLAY:
		{
			HWND hWnd = ::GetFocus();
			if(g_pkApp)
			{
				HWND hGame = g_pkApp->GetWindowReference();
				if(hGame && hWnd != hGame)
				{
					::SetFocus(hGame);
				}
			}

			bool bRtn = false;

			switch(m_eCurType)
			{
			case MT_VMR:
				{
					bRtn = m_kVmr->Update();
				}
				break;
			case MT_ACTIVEX:
				{
					bRtn = m_kAXPlay->Update();
				}
				break;
			case MT_DSHOW:
			case MT_DSHOW_INGAME:
				{
					bRtn = m_kDshowPlay->Update();
				}
				break;
			}

			if(!bRtn)
			{
				m_dwFadeEndTime = BM::GetTime32() + (m_kMovieValue.fFadeOutTime * 1000);
				StopMovie();
				SetFade(false);
				UpdateSequence(US_FADE_OUT);
			}
		}break;
	case US_FADE_OUT:
		{
			DWORD dwTime = BM::GetTime32();
			if( dwTime > m_dwFadeEndTime )
			{
				Stop();
			}
		}break;
	}

	DWORD dwTime = BM::GetTime32();
	if( dwTime > m_dwEndTime)
	{
		Stop();
	}
}

void PgMovieMgr::SetEndTime(DWORD dwTimeSizeSecond)
{
	m_dwEndTime = BM::GetTime32() + (dwTimeSizeSecond*1000);
}
bool PgMovieMgr::ParseXml(char const *pcXmlPath)
{
	TiXmlDocument kXmlDoc(pcXmlPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
	{
		PgError1("Parse Failed [%s]", pcXmlPath);
		return false;
	}

	// Root 'MOVIE'
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();

	assert(strcmp(pkElement->Value(), "MOVIE") == 0);

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "ITEM") == 0)
		{
			char const *pcID = 0;
			char const *pcPath = pkElement->GetText();
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			DWORD dwType = 0;
			DWORD dwTime = 0;
			float fStartTime = -1;
			float fEndTime = -1;
			float fFadeInTime = 0;
			float fFadeOutTime = 0;
			float fVolumeDecTime = 0;

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					NiString kString = pcAttrValue;
					kString.ToUpper();
					pcID = (char const*)kString;
				}
				else if(strcmp(pcAttrName, "TYPE") == 0)
				{
					sscanf_s(pcAttrValue, "%d", &dwType);
				}
				else if(strcmp(pcAttrName, "TIME") == 0)
				{
					sscanf_s(pcAttrValue, "%d", &dwTime);
				}
				else if(strcmp(pcAttrName, "STARTTIME") == 0)
				{
					sscanf_s(pcAttrValue, "%f", &fStartTime);
				}
				else if(strcmp(pcAttrName, "ENDTIME") == 0)
				{
					sscanf_s(pcAttrValue, "%f", &fEndTime);
				}
				else if(strcmp(pcAttrName, "FADE_IN_TIME") == 0)
				{
					sscanf_s(pcAttrValue, "%f", &fFadeInTime);
				}
				else if(strcmp(pcAttrName, "FADE_OUT_TIME") == 0)
				{
					sscanf_s(pcAttrValue, "%f", &fFadeOutTime);
				}
				else if(strcmp(pcAttrName, "VOLUME_DEC_TIME") == 0)
				{
					sscanf_s(pcAttrValue, "%f", &fVolumeDecTime);
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			pcPath = pkElement->GetText();

			if(!pcID || !pcPath)
			{
				PgXmlError(pkElement, "Not Enough Effect Data");

			}
			else
			{
				std::string strID(pcID);
				std::string strPath(pcPath);
				SMovieValue sValue;
				sValue.strID = strID;
				sValue.byType = (BYTE)dwType;
				sValue.dwTime = dwTime;
				sValue.strPath = strPath;
				sValue.fStartTime = fStartTime;
				sValue.fEndTime = fEndTime;
				sValue.fFadeInTime = fFadeInTime;
				sValue.fFadeOutTime = fFadeOutTime;
				sValue.fVolumeDecTime = fVolumeDecTime;
				// 처리
				m_smPathContainer.insert(std::make_pair(strID, sValue));
			}
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
		}

		pkElement = pkElement->NextSiblingElement();
	}

	return true;
}

void PgMovieMgr::BackupFile(std::string strID)
{
	PathContainer::iterator itr = m_smPathContainer.find(strID);
	if(itr == m_smPathContainer.end())
	{
		return ;
	}

	std::string strPath = itr->second.strPath;
	std::string strSource;
	std::string strDes;
	std::string::size_type kPos;
	kPos = strPath.rfind(".");
	if(kPos == std::string::npos)
	{
		return;
	}

	strSource = strPath.substr(0, kPos+1) + "xml";
	strDes = strPath.substr(0, kPos+1) + "bak";
	::rename(strSource.c_str(), strDes.c_str());

	m_strBuckupID = strID;

	//FILE *in, *out;
	//char c;
	//in = fopen( strDes.c_str(), "r" );
	//out = fopen( strPath.c_str(), "w" );
	//while ( !feof( in ) )
	//{
	//	c = fgetc( in );
	//	fputc( c, out );
	//}
	//fclose( in );
	//fclose( out );
}
void PgMovieMgr::RecoveryFile(std::string strID)
{
	PathContainer::iterator itr = m_smPathContainer.find(strID);
	if(itr == m_smPathContainer.end())
	{
		return ;
	}

	std::string strPath = itr->second.strPath;
	std::string strSource;
	std::string strDes;
	std::string::size_type kPos;
	kPos = strPath.rfind(".");
	if(kPos == std::string::npos)
	{
		return;
	}

	strDes = strPath.substr(0, kPos+1) + "bak";
	strSource = strPath.substr(0, kPos+1) + "xml";
	::remove(strSource.c_str());
	::rename(strDes.c_str(), strSource.c_str());

	m_strBuckupID.clear();
}
void PgMovieMgr::ChangeCharName2(std::string strID, std::wstring strName)
{
	PathContainer::iterator itr = m_smPathContainer.find(strID);
	if(itr == m_smPathContainer.end())
		return;

	std::string strPath = itr->second.strPath;
	std::string strSource;
	std::string strDes;
	std::string::size_type kPos;
	kPos = strPath.rfind(".");
	if(kPos == std::string::npos)
	{
		return;
	}
	strSource = strPath.substr(0, kPos+1) + "xml";
	strDes = strPath.substr(0, kPos+1) + "bak";

	TiXmlDocument kXmlDoc;
	if(!kXmlDoc.LoadFile(strDes.c_str(), TIXML_ENCODING_UTF8))
		return;

	TiXmlElement *pkRoot = kXmlDoc.RootElement();
	if(!pkRoot)
		return;

	std::string strCharName = "***";
	std::string strMultiName(MB(strName));

	char szMultiBuff[MAX_PATH]={0,};
	WideCharToMultiByte(CP_UTF8, 0, strName.c_str(), strName.length(), szMultiBuff, sizeof(szMultiBuff), 0, 0);

	TiXmlElement* pkFirstChild = pkRoot->FirstChildElement();
	while(pkFirstChild)
	{
		TiXmlElement* pkChild = pkFirstChild->FirstChildElement();
		while(pkChild)
		{
			TiXmlAttribute *pkAttirbute = pkChild->FirstAttribute();
			while(pkAttirbute)
			{
				char const* szAttrName = pkAttirbute->Name();
				char const* szAttrValue = pkAttirbute->Value();
				if(!strcmp(szAttrName, "sTitle"))
				{
					std::string strTmp(szAttrValue);
					std::string::size_type idx;	
					bool bFindLoop = true;
					bool bFind = false;

					//{
					//	wchar_t utf8_uni[200];
					//	int utf8_uniLen = MultiByteToWideChar(CP_UTF8, 0,  (LPCSTR)strTmp.c_str(), strTmp.length(), utf8_uni, sizeof(utf8_uni)/sizeof(wchar_t));
					//	utf8_uni[utf8_uniLen] = 0;
					//}

					while(bFindLoop)
					{
						idx = strTmp.find(strCharName);
						if(std::string::npos != idx)
						{
							strTmp = strTmp.substr(0, idx) + szMultiBuff + strTmp.substr(idx + strCharName.size());
							bFind = true;
						}
						else
						{
							bFindLoop = false;
						}
					}
					if(bFind)
					{
						pkAttirbute->SetValue(strTmp.c_str());
					}
				}
				pkAttirbute = pkAttirbute->Next();
			}
			pkChild = pkChild->NextSiblingElement();
		}
		pkFirstChild = pkFirstChild->NextSiblingElement();
	}

	kXmlDoc.SaveFile(strSource.c_str());
}
void PgMovieMgr::ChangeCharName(std::string strID, std::wstring strName)
{
	// 안쓴다.
	setlocale(LC_ALL, "kor");
	std::locale::global( std::locale( "kor" ) ); 
	PathContainer::iterator itr = m_smPathContainer.find(strID);
	if(itr == m_smPathContainer.end())
	{
		return;
	}

	std::string strPath = itr->second.strPath;
	std::string strSource;
	std::string strDes;
	std::string::size_type kPos;
	kPos = strPath.rfind(".");
	if(kPos == std::string::npos)
	{
		return;
	}
	strSource = strPath.substr(0, kPos+1) + "xml";
	strDes = strPath.substr(0, kPos+1) + "bak";

	FILE *pfin, *pfout;
	//pfin = _tfopen( UNI(strDes.c_str()), _T("rt") );
	//pfout = _tfopen(UNI(strSource.c_str()), _T("wt"));
	//TCHAR szTmp[MAX_PATH] = {0,};
	//TCHAR* szResult;
	pfin = fopen( strDes.c_str(), "r" );
	pfout = fopen( strSource.c_str(), "w");
	char szTmp[1000] = {0,};
	char* szResult;
	std::wstring strGet;

	std::wstring strCharName = _T("***");
	while ( true )
	{
		//memset(szTmp, 0, sizeof(szTmp));
		//szResult = _fgetts(szTmp, MAX_PATH, pfin);
		szResult = fgets(szTmp, 1000, pfin);
		if(!szResult)// 파일끝
			break;	

		//strGet = szTmp;
		std::string strTmp(szTmp);
		strGet = UNI(szTmp);
		std::wstring::size_type idx;	
		bool bFindLoop = true;
		while(bFindLoop)
		{
			idx = strGet.find(strCharName);
			if(std::wstring::npos != idx)
			{
				std::wstring strTmp = strGet.substr(0, idx) + strName + strGet.substr(idx + strCharName.size());
				strGet = strTmp;
			}
			else
			{
				bFindLoop = false;
			}
		}

		fputws(strGet.c_str(), pfout);
	}
	fclose( pfin );
	fclose( pfout );
}

int const TUTORIAL_MAP1 = 9910100;
int const TUTORIAL_MAP2 = 9910200;
int const TUTORIAL_MAP3 = 9910300;
int const TUTORIAL_MAP4 = 9910400;

void PgMovieMgr::PlayOpeningMoveFromGUID(lwGUID kGuid)
{
	if(IsPlayedIntroMovie(kGuid) == false)
	{
		const PgPilot* pPilot = g_kPilotMan.FindPilot(kGuid.GetGUID());
		if(pPilot)
		{
			const CUnit* pUnit = pPilot->GetUnit();
			if(pUnit)
			{
				const PgPlayer* pkPlayer = dynamic_cast<const PgPlayer*>(pUnit);
				if(pkPlayer)
				{
					int const iMap = pkPlayer->GetRecentMapNo(GATTR_DEFAULT);
					if (iMap == 0 || iMap == TUTORIAL_MAP1 || iMap == TUTORIAL_MAP2 || iMap == TUTORIAL_MAP3 || iMap == TUTORIAL_MAP4)
					{// 튜토리얼 진입시
						std::string strPlay;
						if(g_kLocal.IsServiceRegion(LOCAL_MGR::NC_USA))
						{// 북미는 2D Intro를 보여주고
							strPlay = "2DIntro";
						}
						else
						{// 그외는 캐릭터 시나리오 동영상을 보여준다
							int const iClass = pkPlayer->GetAbil(AT_CLASS);
							if(iClass == 1)
							{
								strPlay ="fighter";
							}
							else if(iClass == 2)
							{
								strPlay ="magician";
							}
							else if(iClass == 3)
							{
								strPlay ="archer";
							}
							else if(iClass == 4)
							{
								strPlay ="thief";
							}
						}

						if(!strPlay.empty())
						{
							// 동영상으로 플레이 하는 나라들.
							switch(g_kLocal.ServiceRegion())
							{
							case LOCAL_MGR::NC_EU:
							case LOCAL_MGR::NC_FRANCE:
							case LOCAL_MGR::NC_GERMANY:
							case LOCAL_MGR::NC_USA:
							case LOCAL_MGR::NC_RUSSIA:
							case LOCAL_MGR::NC_JAPAN:
								{
								}break;
							default:
								{
									//BackupFile(strPlay);
									SaveCharMoviePlayInfoFile(kGuid);
									ChangeCharName2(strPlay, pPilot->GetName());
								}break;
							}
							if (true == Play(strPlay))
							{
								return;
							}							
						}
					}
				}
			}
		}
	}

	// 플레이 안할 경우에
	if (m_pfnMovieFinished != NULL)
	{
		m_pfnMovieFinished(m_kCurrentPlayingMovie);
		m_pfnMovieFinished = NULL;
		m_kCurrentPlayingMovie = TEXT("");
	}
	return;
}

bool PgMovieMgr::IsPlayedIntroMovie(lwGUID kCharGuid)
{
	m_bIntroPlayed = false;
	LoadCharMoviePlayInfoFile(kCharGuid);
	return m_bIntroPlayed;
}

void PgMovieMgr::LoadCharMoviePlayInfoFile(lwGUID kCharGuid)
{
	wchar_t UserInfoPath[MAX_PATH] = {0,};
	wchar_t UserDataPath[MAX_PATH]= {0,};

	if(S_OK != SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, UserDataPath))
	{
		return;
	}

	std::wstring kGUID = kCharGuid().str();
#ifndef EXTERNAL_RELEASE
	if(g_pkApp->IsSingleMode())
	{
		kGUID = _T("SingleModeCharacter");
	}
#endif

	swprintf_s(UserInfoPath, MAX_PATH, L"%s\\%s\\Users\\%s\\", UserDataPath, GetClientNameW(),kGUID.c_str());
	BM::ReserveFolder(UserInfoPath);

	std::wstring kFilePath(UserInfoPath);

	FILE	*fp = _wfopen((kFilePath+_T("MoviePlayInfo.Dat")).c_str(),_T("rb"));
	if(!fp)
	{
		return;
	}

	//	파일 버전 로드
	unsigned	short	usVersion = 0;
	fread(&usVersion,sizeof(unsigned short),1,fp);

	// 인트로 동영상 플레이 여부
	bool	bPlayIntroMovie = false;
	fread(&bPlayIntroMovie,sizeof(bool),1,fp);

	m_bIntroPlayed = bPlayIntroMovie;

	fclose(fp);
}
void PgMovieMgr::SaveCharMoviePlayInfoFile(lwGUID kCharGuid)
{
	wchar_t UserInfoPath[MAX_PATH] = {0,};
	wchar_t UserDataPath[MAX_PATH]= {0,};

	if(S_OK != SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, UserDataPath))
	{
		return;
	}

	std::wstring kGUID = kCharGuid().str();
#ifndef EXTERNAL_RELEASE
	if(g_pkApp->IsSingleMode())
	{
		kGUID = _T("SingleModeCharacter");
	}
#endif

	swprintf_s(UserInfoPath, MAX_PATH, L"%s\\%s\\Users\\%s\\", UserDataPath, GetClientNameW(),kGUID.c_str());

	std::wstring kFilePath(UserInfoPath);

	FILE	*fp = _wfopen((kFilePath+_T("MoviePlayInfo.Dat")).c_str(),_T("wb"));
	if(!fp)
	{
		return;
	}

	//	파일 버전 저장
	unsigned	short	usVersion = 0;
	fwrite(&usVersion,sizeof(unsigned short),1,fp);

	// 인트로 동영상 플레이 여부 저장
	bool	bPlayIntroMovie = true;
	fwrite(&bPlayIntroMovie	,sizeof(bool),1,fp);

	fclose(fp);
}

bool PgMovieMgr::ParseScenarioXml(char const *pcXmlPath)
{
	TiXmlDocument kXmlDoc(pcXmlPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
	{
		PgError1("Parse Failed [%s]", pcXmlPath);
		return false;
	}

	// Root 'MOVIE'
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();

	assert(strcmp(pkElement->Value(), "MOVIE") == 0);

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "ITEM") == 0)
		{
			char const *pcPath = pkElement->GetText();
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			int iMapNum = 0;

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					sscanf_s(pcAttrValue, "%d", &iMapNum);
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			pcPath = pkElement->GetText();

			if( !pcPath )
			{
				PgXmlError(pkElement, "Not Enough Effect Data");
			}
			else
			{
				m_smScenarioPath.insert(std::make_pair(iMapNum, pcPath));
			}
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
		}

		pkElement = pkElement->NextSiblingElement();
	}

	return true;
}

std::string const PgMovieMgr::GetSecnarioMovieName(int iMapNum)
{
	ScenarioMoviePath::iterator itr = m_smScenarioPath.find(iMapNum);
	if( itr != m_smScenarioPath.end() )
	{
		return itr->second;
	}

	return std::string();
}


bool PgMovieMgr::ParseQuestXml(char const *pcXmlPath)
{
	TiXmlDocument kXmlDoc(pcXmlPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
	{
		PgError1("Parse Failed [%s]", pcXmlPath);
		return false;
	}

	// Root 'MOVIE'
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();

	assert(strcmp(pkElement->Value(), "MOVIE") == 0);

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "ITEM") == 0)
		{
			char const *pcPath = pkElement->GetText();
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			int iQuestNum = 0;

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					sscanf_s(pcAttrValue, "%d", &iQuestNum);
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			pcPath = pkElement->GetText();

			if( !pcPath )
			{
				PgXmlError(pkElement, "Not Enough Effect Data");
			}
			else
			{
				m_smQuestPath.insert(std::make_pair(iQuestNum, pcPath));
			}
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
		}

		pkElement = pkElement->NextSiblingElement();
	}

	return true;
}

std::string const PgMovieMgr::GetQuestMovieName(int const iQuestNum)
{
	if(!m_smQuestPath.empty() )
	{
		ScenarioMoviePath::iterator itr = m_smQuestPath.find(iQuestNum);
		if( itr != m_smQuestPath.end() )
		{
			return itr->second;
		}
	}

	return std::string();
}