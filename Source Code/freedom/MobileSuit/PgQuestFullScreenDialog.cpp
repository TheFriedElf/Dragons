#include "stdafx.h"
#include "Variant/PgStringUtil.h"
#include "Variant/PgQuestInfo.h"
#include "PgMobileSuit.h"
#include "PgPilot.h"
#include "PgPilotman.h"
#include "PgOption.h"
#include "lwUI.h"
#include "lwUIQuest.h"
#include "PgQuestUI.h"
#include "PgQuestDialog.h"
#include "Pg2DString.h"
#include "PgUIScene.h"
#include "PgUIModel.h"
#include "PgQuestFullScreenDialog.h"
#include "PgStitchImageMng.h"
#include "PgQuestMan.h"
#include "PgTextTypistWnd.h"
#include "PgMovieMgr.h"

TCHAR const *szStitchImageWndID = _T("FRM_STITCH_IMAGE%u");
TCHAR const *szStitchTextWndID = _T("FRM_STITCH_TEXT%u");

extern XUI::CXUI_Wnd* CallQuestWnd(std::wstring const &rkFormName);
extern bool ParseNpcName(const wchar_t *pName, std::wstring& rkOut);

/////////////////////////////////////////////////////////////////////////////////////////////////////////
tagFullScreenDialog::tagFullScreenDialog(SSayItem const &rkItem, std::wstring const &rkName)
{
	Clear();
	eType = QDT_NpcTalk;
	kObjectName = rkName;
	kDialogText = TTW(rkItem.iTTW);
	kFaceID = UNI(rkItem.kFaceID);
	iFaceAniID = rkItem.iFaceAniID;
	kCamPos = rkItem.kCamPos;
}

tagFullScreenDialog::tagFullScreenDialog(EQuestDialogType const Type, SQuestDialogText const &rkDialogText)
{
	Clear();
	eType = Type;
	kDialogText = TTW(rkDialogText.iTextNo);
	kFaceID = rkDialogText.kFaceID;
	iFaceAniID = rkDialogText.iFaceAniID;
	kModelID = rkDialogText.kModelID;
	kStitchImageID = rkDialogText.kStitchImageID;
	kCamPos = rkDialogText.kCamPos;
	iMovieNo = rkDialogText.iMovieNo;
}

tagFullScreenDialog::tagFullScreenDialog(EQuestDialogType const Type)
{
	Clear();
	eType = Type;
}

tagFullScreenDialog::tagFullScreenDialog()
{
	Clear();
}

void tagFullScreenDialog::Clear()
{
	eType = QDT_None;
	kObjectName.clear();
	kTitleText.clear();
	kDialogText.clear();
	kFaceID.clear();
	iFaceAniID = 0;
	kModelID.clear();
	kStitchImageID.clear();
	kCamPos.Clear();
	iMovieNo = 0;
}

bool tagFullScreenDialog::Empty()
{
	return QDT_None == eType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
PgQuestFullScreenDialog::PgQuestFullScreenDialog()
{
	ClearDialogStep();
	ClearSelectedItem();
}

PgQuestFullScreenDialog::~PgQuestFullScreenDialog()
{
}

bool PgQuestFullScreenDialog::ParseXmlForNpcFace(char const *pcFileName)
{
	BM::CAutoMutex kLock(m_kFullScreenMutex);
	if (!pcFileName)
	{
		return false;
	}

	TiXmlDocument kXmlDoc(pcFileName);

	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcFileName)))
	{
		PgError1("Parse Failed [%s]", pcFileName);
		return false;
	}

	m_ContNPCFacePath.clear();

	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();
	PG_ASSERT_LOG(strcmp(pkElement->Value(), "NPC_2D_FACE") == 0);

	pkElement = pkElement->FirstChildElement();
	while (pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "NPC_FACE") == 0)
		{
			char const *szID = pkElement->Attribute("ID");
			char const *szPath = pkElement->Attribute("PATH");
			char const *szLimitWidth = pkElement->Attribute("LIMIT_WIDTH");
			PG_ASSERT_LOG(szID && szPath);

			std::wstring const kID = UNI(szID);
			std::wstring const kPath = UNI(szPath);
			const SNpcFaceInfo kFaceInfo(kID, kPath, PgStringUtil::SafeAtoi(szLimitWidth));

			m_ContNPCFacePath.insert(std::make_pair(kID, kFaceInfo));
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcFileName);
		}

		pkElement = pkElement->NextSiblingElement();
	}
	return true;
}

bool PgQuestFullScreenDialog::GetNpcFacePath(std::wstring const &kFaceID, SNpcFaceInfo& rkOut) const
{
	BM::CAutoMutex kLock(m_kFullScreenMutex);
	FacePathContainer::const_iterator face_Itor = m_ContNPCFacePath.find(kFaceID);
	if( face_Itor != m_ContNPCFacePath.end())
	{
		rkOut = (*face_Itor).second;
		return true;
	}
	return false;
}

bool PgQuestFullScreenDialog::IsLastStepDialog() const
{
	return (m_iNowDialogStep == m_iNowDialogMaxStep);
}

bool PgQuestFullScreenDialog::CanNextSelect() const
{
	if( m_pkNowDialogInfo )
	{
		return !m_pkNowDialogInfo->kSelectList.empty();
	}
	return true;
}

void PgQuestFullScreenDialog::ClearDialogStep()
{
	m_iNowDialogStep = 0;
	m_iNowDialogMaxStep = 0;

	m_ContDialogStep.clear();

	m_kNowInfo.Clear();

	m_pkNowQuestInfo = NULL;
	m_pkNowDialogInfo = NULL;

	m_kNowSelectMenu = false;
}
void PgQuestFullScreenDialog::ClearSelectedItem()
{
	m_kSelectItem1 = 0;
	m_kSelectItem2 = 0;
}


bool PgQuestFullScreenDialog::CallFullScreenTalk(const SFullScreenDialogInfo& rkInfo, const PgQuestInfo* pkQuestInfo, const SQuestDialog* pkDialogInfo)
{
	ClearDialogStep();

	m_kNowInfo = rkInfo;
	m_pkNowQuestInfo = pkQuestInfo;
	m_pkNowDialogInfo = pkDialogInfo;

	switch( rkInfo.eType )
	{
	case QDT_Movie:
		{
			std::string const strMovieName = g_kMovieMgr.GetQuestMovieName(rkInfo.iMovieNo);
			if( !strMovieName.empty() && false == g_kMovieMgr.IsPlay())
			{
				g_kMovieMgr.Play(strMovieName);
			}
		}break;
	default:
		{
			Quest::lwQuestTextParser(m_kNowInfo.kDialogText);
			PgQuestManUtil::ParseDialogForStep(m_kNowInfo.kDialogText, _T("%%"), m_ContDialogStep);

			m_iNowDialogMaxStep = (m_ContDialogStep.empty())? 0: m_ContDialogStep.size()-1;

			UpdateFullScreenTalk();
		}break;
	}
	return true;
}

bool PgQuestFullScreenDialog::NextFullScreenTalk()
{
	if( IsLastStepDialog() )
	{
		return false;//막 스탭이다.
	}
	else
	{
		m_iNowDialogStep += 1;//같은 다이얼로그면 스텝만 증가
	}

	UpdateFullScreenTalk();
	return true;
}

//>>
typedef struct tagQuestFullDialogWnd
{
	EQuestType eType;
	XUI::CXUI_Wnd* pkQuestWnd;
	XUI::CXUI_Wnd* pkQuestTitleBgWnd_Normal;//사용 제목 배경(보통)
	XUI::CXUI_Wnd* pkQuestTitleBgWnd_Scenario;//사용 제목 배경(시나리오)
	XUI::CXUI_Wnd* pkQuestFaceWnd_Normal;//사용 얼굴(보통)
	XUI::CXUI_Wnd* pkQuestFaceWnd_Scenario;//사용 얼굴(시나리오)
	XUI::CXUI_Wnd* pkQuestBlackBGWnd_Normal;
	XUI::CXUI_Wnd* pkQuestBlackBGWnd_Scenario;
	XUI::CXUI_Wnd* pkQuestModelWnd;//모델 배경
	XUI::CXUI_Wnd* pkStitchImageBgWnd;//조각 이미지 배경
	XUI::CXUI_Wnd* pkQuestTitleWnd;//사용 제목
	XUI::CXUI_Wnd* pkDialogMainWnd;//이름/대사 통합
	XUI::CXUI_Wnd* pkQuestNameWnd_Normal;//NPC 이름
	XUI::CXUI_Wnd* pkQuestNameWnd_Scenario;//NPC 이름
	XUI::CXUI_Wnd* pkQuestNameBgWnd_Normal;//이름 배경
	XUI::CXUI_Wnd* pkQuestTextBgWnd;// 대사 배경
	XUI::CXUI_Wnd* pkQuestTextWnd;//사용 대사
	XUI::CXUI_Wnd* pkQuestEnterBtn;//엔터 키
	XUI::CXUI_Wnd* pkQuestSkipBtn;//스킵 버튼

	PgQUI_QuestDialog* pkInfoDialogWnd;//정보 창 (수락/거부, 완료/닫기)
	XUI::CXUI_Wnd *pkGuildMenuWnd;
	XUI::CXUI_Wnd *pkQuestMenuWnd;

	tagQuestFullDialogWnd()
	{
		pkQuestWnd = NULL;
		pkQuestTitleBgWnd_Normal = NULL;
		pkQuestTitleBgWnd_Scenario = NULL;
		pkQuestFaceWnd_Normal = NULL;
		pkQuestFaceWnd_Scenario = NULL;
		pkQuestBlackBGWnd_Normal = NULL;
		pkQuestBlackBGWnd_Scenario = NULL;
		pkQuestModelWnd = NULL;
		pkStitchImageBgWnd = NULL;
		pkQuestTitleWnd = NULL;
		pkDialogMainWnd = NULL;
		pkQuestNameWnd_Normal = NULL;
		pkQuestNameWnd_Scenario = NULL;
		pkQuestNameBgWnd_Normal = NULL;
		pkQuestTextBgWnd = NULL;
		pkQuestTextWnd = NULL;
		pkQuestEnterBtn = NULL;
		pkQuestSkipBtn = NULL;
		pkInfoDialogWnd = NULL;
		pkGuildMenuWnd = NULL;
		pkQuestMenuWnd = NULL;
	}

	bool Call(PgQuestInfo const *pkQuestInfo)
	{
		pkQuestWnd = ::CallQuestWnd( PgQuestManUtil::kFullQuestWndName );
		if( !pkQuestWnd )
		{
			return false;
		}

		eType = (pkQuestInfo)? pkQuestInfo->Type(): QT_None;

		lua_tinker::call<void, char*, bool>("OnSet_FullScreenUIShow", "SCENARIO", (QT_Scenario == eType) );

		pkQuestTitleBgWnd_Normal = pkQuestWnd->GetControl( std::wstring(_T("IMG_TITLE_BG_NORMAL")) );
		pkQuestTitleBgWnd_Scenario = pkQuestWnd->GetControl( std::wstring(_T("IMG_TITLE_BG_SCENARIO")) );

		pkQuestBlackBGWnd_Normal = pkQuestWnd->GetControl( std::wstring(_T("IMG_BLACK_BG_NORMAL")) );;
		pkQuestBlackBGWnd_Scenario = pkQuestWnd->GetControl( std::wstring(_T("IMG_BLACK_BG_SCENARIO")) );;

		pkQuestFaceWnd_Normal = pkQuestWnd->GetControl( std::wstring(_T("FRM_3DMODEL_NORMAL")) );
		pkQuestFaceWnd_Scenario = pkQuestWnd->GetControl( std::wstring(_T("FRM_3DMODEL_SCENARIO")) );

		pkQuestModelWnd = pkQuestWnd->GetControl( std::wstring(_T("FRM_MODEL")) );
		pkStitchImageBgWnd = pkQuestWnd->GetControl( std::wstring(_T("FRM_STITCH_IMAGE_BG")) );

		pkQuestTitleWnd = pkQuestWnd->GetControl( std::wstring(_T("IMG_TITLE")) );

		pkDialogMainWnd = pkQuestWnd->GetControl( std::wstring(_T("FRM_MAIN")) );
		if( pkDialogMainWnd )
		{
			pkQuestNameWnd_Normal = pkDialogMainWnd->GetControl( std::wstring(_T("FRM_NAME_NORMAL")) );
			pkQuestNameWnd_Scenario = pkDialogMainWnd->GetControl( std::wstring(_T("FRM_NAME_SCENARIO")) );
			pkQuestNameBgWnd_Normal = pkDialogMainWnd->GetControl( std::wstring(_T("IMG_NAME_BG_NORMAL")) );
			pkQuestTextBgWnd = pkDialogMainWnd->GetControl( std::wstring(_T("FRM_TEXT_BG")) );
			if( pkQuestTextBgWnd )
			{
				pkQuestTextWnd = pkQuestTextBgWnd->GetControl( std::wstring(_T("FRM_TEXT")) );
			}
		}
		pkQuestEnterBtn = pkQuestWnd->GetControl( std::wstring(_T("IMG_ENTER")) );
		pkQuestSkipBtn = pkQuestWnd->GetControl( std::wstring(_T("IMG_SKIP")) );

		pkInfoDialogWnd = dynamic_cast<PgQUI_QuestDialog*>( pkQuestWnd->GetControl( std::wstring(_T("FRM_QUEST_REQ")) ) );//정보 창 (수락/거부, 완료/닫기)
		pkGuildMenuWnd = pkQuestWnd->GetControl( std::wstring(_T("FRM_GUILD_MENU")) );
		pkQuestMenuWnd = pkQuestWnd->GetControl( std::wstring(_T("FRM_QUEST_MENU")) );
		return true;
	}

	void Visible()
	{
		// 전체를 조정 (나머진 부분적으로 스스로 조정)
		bool const bScenario = QT_Scenario == eType;
		if( pkQuestWnd )						{ pkQuestWnd->Visible(true); }
		//if( pkQuestTitleBgWnd_Normal )			{ pkQuestTitleBgWnd_Normal->Visible(false); }
		//if( pkQuestTitleBgWnd_Scenario )		{ pkQuestTitleBgWnd_Scenario->Visible(false); }
		//if( pkQuestFaceWnd_Normal )				{ pkQuestFaceWnd_Normal->Visible(false); }
		//if( pkQuestFaceWnd_Scenario )			{ pkQuestFaceWnd_Scenario->Visible(false); }
		if( pkQuestBlackBGWnd_Normal )			{ pkQuestBlackBGWnd_Normal->Visible( false ); }
		if( pkQuestBlackBGWnd_Scenario )		{ pkQuestBlackBGWnd_Scenario->Visible( bScenario ); }
		if( pkQuestModelWnd )					{ pkQuestModelWnd->Visible(false); }
		if( pkStitchImageBgWnd )				{ pkStitchImageBgWnd->Visible(false); }
		//if( pkQuestTitleWnd )					{ pkQuestTitleWnd->Visible(false); }
		//if( pkDialogMainWnd )					{ pkDialogMainWnd->Visible(false); }
		if( pkQuestNameWnd_Normal )				{ pkQuestNameWnd_Normal->Visible(false); }
		if( pkQuestNameWnd_Scenario )			{ pkQuestNameWnd_Scenario->Visible(false); }
		if( pkQuestNameBgWnd_Normal )			{ pkQuestNameBgWnd_Normal->Visible(false); }
		//if( pkQuestTextBgWnd )					{ pkQuestTextBgWnd->Visible(false); }
		//if( pkQuestTextWnd )					{ pkQuestTextWnd->Visible(false); }
		if( pkQuestEnterBtn )					{ pkQuestEnterBtn->Visible(true); }
		if( pkQuestSkipBtn )					{ pkQuestSkipBtn->Visible(true); }
		//
		if( pkInfoDialogWnd )					{ pkInfoDialogWnd->Visible(false); }
		if( pkGuildMenuWnd )					{ pkGuildMenuWnd->Visible(false); }
		if( pkQuestMenuWnd )					{ pkQuestMenuWnd->Visible(false); }
	}

	bool Title(std::wstring const &rkTitle)
	{
		NiRenderer *pkRenderer = NiRenderer::GetRenderer();
		if( !pkRenderer )
		{
			return false;
		}

		if( !pkQuestTitleBgWnd_Normal || !pkQuestTitleBgWnd_Scenario || !pkQuestTitleWnd )
		{
			return false;
		}

		CXUI_Wnd *pkTitleWnd = pkQuestTitleWnd;
		CXUI_Wnd *pkTitleBgWnd = (QT_Scenario == eType)? pkQuestTitleBgWnd_Scenario: pkQuestTitleBgWnd_Normal;

		bool const bTitle = !rkTitle.empty();
		pkTitleWnd->Visible( bTitle );
		pkTitleBgWnd->Visible( bTitle );
		if( !bTitle )
		{
			return false;
		}

		const unsigned int iMaxWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
		
		XUI::CXUI_Wnd* pkLeftImgWnd = pkTitleBgWnd->GetControl(_T("IMG_LEFT"));
		XUI::CXUI_Wnd* pkCenterImgWnd = pkTitleBgWnd->GetControl(_T("IMG_CENTER"));
		XUI::CXUI_Wnd* pkRightImgWnd = pkTitleBgWnd->GetControl(_T("IMG_RIGHT"));
		if( !pkLeftImgWnd || !pkCenterImgWnd || !pkRightImgWnd )
		{
			return false;
		}

		pkTitleWnd->Text( rkTitle ); // Title
		CXUI_Style_String kStyleText = pkTitleWnd->StyleText();
		POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleText));

		POINT2 const kTargetSize(kTextSize.x+20, pkCenterImgWnd->Size().y);	// Left
		pkCenterImgWnd->Size(kTargetSize);
		pkCenterImgWnd->ImgSize(kTargetSize);
		pkCenterImgWnd->SetInvalidate();
		int const iTargetPosX = pkLeftImgWnd->Size().x + pkCenterImgWnd->Size().x; // Right
		pkRightImgWnd->Location(iTargetPosX, pkRightImgWnd->Location().y);
		pkRightImgWnd->SetInvalidate();
		int const iTotalSize = pkLeftImgWnd->Size().x + pkCenterImgWnd->Size().x + pkRightImgWnd->Size().x; // Bg
		pkTitleBgWnd->Size(iTotalSize, pkTitleBgWnd->Size().y);
		pkTitleBgWnd->Location((iMaxWidth/2)-(iTotalSize/2), pkTitleBgWnd->Location().y);
		pkTitleBgWnd->SetInvalidate();

		return true;
	}

	void StitchImage(std::wstring const &rkStitchImageID )
	{
		if( !pkStitchImageBgWnd )
		{
			return;
		}

		bool const bStitchImage = !rkStitchImageID.empty();
		if( !bStitchImage )
		{
			return;
		}

		pkStitchImageBgWnd->Visible( !bStitchImage );

		size_t iCurStitchImageWnd = 0;
		size_t iCurStitchTextWnd = 0;
		TCHAR szTemp[512] = {0, };
		SStitchImage kStitchImage;
		if( g_kStitchImageMng.Get(rkStitchImageID, kStitchImage) )
		{
			ContImagePeice const &rkVec = kStitchImage.kPeices;
			ContImagePeice::const_iterator image_iter = rkVec.begin();
			while( rkVec.end() != image_iter )
			{
				ContImagePeice::value_type const &rkImagePeice = (*image_iter);

				_stprintf_s(szTemp, szStitchImageWndID, iCurStitchImageWnd);
				CXUI_Wnd *pkStitchImageWnd = pkStitchImageBgWnd->GetControl( std::wstring(szTemp) );
				if( pkStitchImageWnd )
				{
					pkStitchImageWnd->ImgPos( POINT2(0, 0) );
					pkStitchImageWnd->ImgSize( rkImagePeice.kSize );
					pkStitchImageWnd->Size( rkImagePeice.kSize );
					pkStitchImageWnd->Location( kStitchImage.kPos + rkImagePeice.kPos );
					pkStitchImageWnd->DefaultImgName( rkImagePeice.kImagePath );
					pkStitchImageWnd->Visible(true);
					pkStitchImageWnd->SetInvalidate();
				}

				++image_iter;
				++iCurStitchImageWnd;
			}

			ContTextPeice const &rkTextVec = kStitchImage.kTexts;
			ContTextPeice::const_iterator text_iter = rkTextVec.begin();
			while( rkTextVec.end() != text_iter )
			{
				ContTextPeice::value_type const &rkText = (*text_iter);

				_stprintf_s(szTemp, szStitchTextWndID, iCurStitchTextWnd);
				CXUI_Wnd *pkStitchTextWnd = pkStitchImageBgWnd->GetControl( std::wstring(szTemp) );
				if( pkStitchTextWnd )
				{
					pkStitchTextWnd->Text( TTW(rkText.iTextID) );
					if( 0 != (rkText.dwFontOption & (DWORD)XUI::XTF_ALIGN_CENTER) )
					{
						pkStitchTextWnd->TextPos( POINT2(rkText.kSize.y / 2, 0) );
					}
					else if( 0 != (rkText.dwFontOption & (DWORD)XUI::XTF_ALIGN_RIGHT) )
					{
						pkStitchTextWnd->TextPos( POINT2(rkText.kSize.y, 0) );
					}
					else
					{
						pkStitchTextWnd->TextPos( POINT2(0, 0) );
					}
					pkStitchTextWnd->Size( rkText.kSize );
					pkStitchTextWnd->Location( kStitchImage.kPos + rkText.kPos );
					pkStitchTextWnd->Font( rkText.kFontName );
					pkStitchTextWnd->FontFlag( rkText.dwFontOption );
					pkStitchTextWnd->FontColor( rkText.dwFontColor );
					pkStitchTextWnd->OutLineColor( rkText.dwOutlineColor );
					pkStitchTextWnd->Visible(true);
					pkStitchTextWnd->SetInvalidate();
				}

				++text_iter, ++iCurStitchTextWnd;
			}

			pkStitchImageBgWnd->SortChildGreater();
		}

		for(; iMaxStitchImageWndCount > iCurStitchImageWnd; ++iCurStitchImageWnd)
		{
			// 사용하고 남은것 전부 감춤
			_stprintf_s(szTemp, szStitchImageWndID, iCurStitchImageWnd);
			CXUI_Wnd *pkStitchImageWnd = pkStitchImageBgWnd->GetControl( std::wstring(szTemp) );
			if( pkStitchImageWnd )
			{
				pkStitchImageWnd->Visible(false);
				pkStitchImageWnd->SetInvalidate();
			}
		}
		for(; iMaxStitchTextWndCount > iCurStitchTextWnd; ++iCurStitchTextWnd)
		{
			_stprintf_s(szTemp, szStitchTextWndID, iCurStitchTextWnd);
			CXUI_Wnd *pkStitchTextWnd = pkStitchImageBgWnd->GetControl( std::wstring(szTemp) );
			if( pkStitchTextWnd )
			{
				pkStitchTextWnd->Visible(false);
				pkStitchTextWnd->SetInvalidate();
			}
		}

		pkStitchImageBgWnd->SetInvalidate();
		pkStitchImageBgWnd->Visible( !kStitchImage.kID.empty() );
	}

	void Name(std::wstring const &rkName)
	{
		std::wstring kName = rkName;
		if( !pkQuestNameWnd_Normal || !pkQuestNameWnd_Scenario || !pkQuestNameBgWnd_Normal )
		{
			return ;
		}
		bool const bScenario = QT_Scenario == eType;

		CXUI_Wnd *pkQuestNameWnd = (bScenario)? pkQuestNameWnd_Scenario: pkQuestNameWnd_Normal;

		bool const bName = 0 < kName.size();
		if( bName )
		{
			ParseNpcName(kName.c_str(), kName);
			if( bScenario )
			{
				//PgStringUtil::ReplaceStrW(kName, _T("\n"), _T(" "), kName); // 개행 문자 공백로 치환
				//PgStringUtil::EraseStr(kName, std::wstring(_T("\n")), kName); // 개행 문자 삭제
			}

			std::wstring::size_type Pos = kName.find(L"%name%");
			if( std::wstring::npos != Pos )
			{
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( pkPlayer )
				{
					BM::vstring vStr(TTW(401172));
					vStr.Replace(L"#NAME#",  pkPlayer->Name());
					pkQuestNameWnd->Text( vStr );
				}
			}
			else
			{
				pkQuestNameWnd->Text( kName );
			}
		}
		pkQuestNameWnd->Visible( bName );
		pkQuestNameBgWnd_Normal->Visible( bName && !bScenario );
	}

	void Face(SNpcFaceInfo const &rkNpcFace)
	{
		bool const bFace = !rkNpcFace.kFaceID.empty();
		if( !pkQuestFaceWnd_Scenario || !pkQuestFaceWnd_Normal )
		{
			return;
		}

		bool const bScenario = QT_Scenario == eType;
		CXUI_Wnd *pkFaceWnd = (bScenario)? pkQuestFaceWnd_Scenario: pkQuestFaceWnd_Normal;
		if( bFace )
		{
			pkFaceWnd->DefaultImgName( rkNpcFace.kFaceImgPath );
			if( bScenario )
			{
				POINT3I kPos = pkFaceWnd->Location();
				int const iFaceImageWidth = 512;
				//kPos.x = 256 - (rkNpcFace.iLimitWidth / 2) + (512 - rkNpcFace.iLimitWidth) / 2;
				kPos.x = (XUIMgr.GetResolutionSize().x/2) - (iFaceImageWidth - rkNpcFace.iLimitWidth) - (rkNpcFace.iLimitWidth/2);
				pkFaceWnd->Location(kPos);
			}
		}

		//pkFaceWnd->Visible( bFace );
		lua_tinker::call<void, char*, bool>("OnSet_FullScreenUIShow", "FACE", bFace);
	}

	void Contents(std::wstring const &rkText, int const iLimitWidth)
	{
		if( !pkQuestTextWnd || !pkDialogMainWnd || !pkQuestTextBgWnd )
		{
			return;
		}

		int const iSizeX = (QT_Scenario == eType)? 600: pkDialogMainWnd->Size().x - pkQuestTextBgWnd->Location().x - iLimitWidth;
		pkQuestTextWnd->Size( iSizeX, pkQuestTextWnd->Size().y );
		
		PgTextTypistWnd* pTypist = dynamic_cast<PgTextTypistWnd*>(pkQuestTextWnd);
		if( pTypist )
			pTypist->Text(rkText);
//		pkQuestTextWnd->Text( rkText );

		lua_tinker::call<void, char*, bool>("OnSet_FullScreenUIShow", "CONTENTS", !rkText.empty());
	}

	void Model3D(std::wstring const &rkModelID)
	{
		if( !pkQuestModelWnd )
		{
			return;
		}

		bool bModel = !rkModelID.empty();
		if( bModel )
		{
			char const *szDependUIModelName = "QuestDepend";
			char const *szDependActorName = "DependActor%d";
			PgUIModel* pkUIModel = g_kUIScene.FindUIModel(szDependUIModelName);
			bModel = NULL != pkUIModel;

			if( pkUIModel )
			{
				static NiPoint3 const kModelDefaultPos[] = {
					NiPoint3(-30.f, 0.f, 0.f),	//Center
					NiPoint3(-75.f, 0.f, 0.f),	//One Left
					NiPoint3(15.f, 0.f, 0.f),	//One Right
					NiPoint3(-120.f, 0.f, 0.f),	//Two Left
					NiPoint3(60.f, 0.f, 0.f),	//Two Right
					NiPoint3(0.f, 0.f, 0.f)};		//not use
				VEC_WSTRING kModelIDVec;
				PgStringUtil::BreakSep(rkModelID, kModelIDVec, _T("/"));
				size_t iCurIndex = 0;
				VEC_WSTRING::iterator model_iter = kModelIDVec.begin();
				while(kModelIDVec.end() != model_iter)
				{
					if( kModelDefaultPos[iCurIndex] == NiPoint3(0.f, 0.f, 0.f) )
					{
						PG_ASSERT_LOG(false && "Overflow model default Position index");
						break;
					}

					char szTemp[1024] = {0, };
					sprintf_s(szTemp, szDependActorName, iCurIndex);

					pkUIModel->AddActorByID(szTemp, MB(*model_iter));
					NiActorManager* pkAM = pkUIModel->GetActor(szTemp);
					if( pkAM )
					{
						NiAVObject* pkRoot = pkAM->GetNIFRoot();
						if( pkRoot )
						{
							pkRoot->SetTranslate( kModelDefaultPos[iCurIndex] + NiPoint3(-20.f, 150.f, -65.f) );
						}
					}

					++model_iter, ++iCurIndex;
				}
			}
		}

		pkQuestModelWnd->Visible( bModel );
	}

	bool Menu(BM::GUID const &rkGuid, EQuestDialogType const eDialogType, bool const bIsLastStep, SQuestDialog const *pkDialogInfo)
	{
		bool bRet = false;
		if( bIsLastStep )
		{
			CXUI_Wnd *pkMenuWnd = NULL;
			if( QDT_Talk_Menu == eDialogType )
			{
				pkMenuWnd = pkQuestMenuWnd;
				bRet = true;
			}
			else if( QDT_TextDialog == eDialogType )
			{
				pkMenuWnd = pkGuildMenuWnd;
				bRet = true;
			}
			else
			{
				bRet = lua_tinker::call<bool, lwUIWnd, int, lwGUID>("ShowNpcTalkMenu", pkQuestWnd, eDialogType, lwGUID(rkGuid));
			}

			if( pkMenuWnd )// 메뉴를 생성해준다.
			{
				lua_tinker::call<void, lwUIWnd>("OffAllSubMenu", pkQuestWnd);

				XUI::CXUI_List *pkMenuListWnd = dynamic_cast<XUI::CXUI_List*>(pkMenuWnd->GetControl(_T("LIST_COMMAND")));
				if( pkMenuListWnd
				&&	pkDialogInfo )
				{
					pkMenuListWnd->ClearList();
					ContQuestSelect::const_iterator select_iter = pkDialogInfo->kSelectList.begin();
					while( pkDialogInfo->kSelectList.end() != select_iter )
					{
						ContQuestSelect::value_type const &rkSelect = (*select_iter);
						lua_tinker::call<void, lwUIWnd, int, int, lwGUID, bool>("AddTalkMenuItem", pkMenuListWnd, rkSelect.iSelectTextNo, rkSelect.iSelectID, lwGUID(rkGuid), false);
						++select_iter;
					}

					if( !pkDialogInfo->kSelectList.empty() )
					{
						//lua_tinker::call<void, lwUIWnd, lwUIWnd>("ResizeTalkMenuWnd", pkMenuWnd, pkMenuListWnd);
						lua_tinker::call<void, lwUIWnd, lwUIWnd>("CommonMenuSetter", pkMenuWnd, pkMenuListWnd);
					}
					else
					{
						bRet = false;
					}
				}
			}
		}
		else
		{
			lua_tinker::call<void, lwUIWnd>("OffAllSubMenu", pkQuestWnd);
		}
		return bRet;
	}

	bool Info(bool const bIsLastStep, PgQuestInfo const *pkQuestInfo, SQuestDialog const *pkDialogInfo )
	{
		if( !pkInfoDialogWnd )
		{
			return false;
		}

		bool bVisible = false;
		if( //bIsLastStep
			pkQuestInfo
		&&	pkDialogInfo )
		{
			if( QDT_Prolog == pkDialogInfo->eType
			||	QDT_Complete == pkDialogInfo->eType )
			{
				pkInfoDialogWnd->Show(false, pkQuestInfo->ID(), pkDialogInfo->iDialogID);
				bVisible = true;
			}
		}

		pkInfoDialogWnd->Visible( bVisible );
		pkQuestEnterBtn->Visible( !bIsLastStep );
		//pkQuestSkipBtn->Visible( !bIsLastStep );
		lua_tinker::call<void, char*, bool>("OnSet_FullScreenUIShow", "INFO", bVisible);
		return bVisible;
	}

	void Invalidate()
	{
		if( !pkQuestWnd )
		{
			return;
		}
		pkQuestWnd->SetInvalidate();
	}
} SQuestFullDialogWnd;
//<<

bool PgQuestFullScreenDialog::UpdateFullScreenTalk()
{
	SQuestFullDialogWnd kDialogWnd;
	if( !kDialogWnd.Call( m_pkNowQuestInfo ) )
	{
		return true;
	}
	kDialogWnd.Visible();


	//
	std::wstring kTitleText = NowInfo().kTitleText;
#ifndef USE_INB
	if( g_pkApp->VisibleClassNo()
	&& m_pkNowQuestInfo )
	{
		BM::vstring kQuestID(m_pkNowQuestInfo->ID());
		kTitleText += _T(" ") + (std::wstring)kQuestID;
	}
#endif

	//if( !m_ContDialogStep.empty() )
	//{
		bool const bCanTitle = kDialogWnd.Title( NowInfo().kTitleText );
		lua_tinker::call<void, char*, bool>("OnSet_FullScreenUIShow", "TITLE", bCanTitle);

		kDialogWnd.StitchImage( NowInfo().kStitchImageID );

		kDialogWnd.Name( NowInfo().kObjectName );

		SNpcFaceInfo kNpcFace;
		GetNpcFacePath( NowInfo().kFaceID, kNpcFace);
		kDialogWnd.Face( kNpcFace );
		int const iLimitWidth = (kNpcFace.iLimitWidth)? kNpcFace.iLimitWidth: 200;

		//
		std::wstring kText;
		ContDialogStep::const_iterator text_iter = m_ContDialogStep.find(NowDialogStep());//대사
		if( text_iter != m_ContDialogStep.end() )
		{
			kText = (*text_iter).second;
		}
		kDialogWnd.Contents( kText, iLimitWidth );

		kDialogWnd.Model3D( NowInfo().kModelID );
	//}

	bool const bIsLastStep = IsLastStepDialog();

	bool const bIsInfoDialog = kDialogWnd.Info( bIsLastStep, m_pkNowQuestInfo, m_pkNowDialogInfo );
	kDialogWnd.Invalidate();

	bool const bIsMenu = kDialogWnd.Menu( NowInfo().kGuid, NowInfo().eType, bIsLastStep, m_pkNowDialogInfo );
	m_kNowSelectMenu = bIsMenu;

	if( m_pkNowDialogInfo
	&&	QDT_Complete == m_pkNowDialogInfo->eType )
	{
		ClearSelectedItem(); // 이전에 선택 정보 초기화
	}
	return true;
}

void PgQuestFullScreenDialog::UpdateWanted(const SFullScreenDialogInfo& rkInfo, const PgQuestInfo* pkQuestInfo, const SQuestDialog* pkQuestDialog)
{
	if( !pkQuestInfo
	||	!pkQuestDialog )
	{
		return ;
	}

	m_kNowInfo = rkInfo;
	m_pkNowQuestInfo = pkQuestInfo;
	m_pkNowDialogInfo = pkQuestDialog;

	typedef std::vector< XUI::CXUI_Wnd* > ContXuiWnd;
	ContXuiWnd kBtnVec;
	XUI::CXUI_Wnd* pkQuestWnd = NULL;
	switch( rkInfo.eType )
	{
	case QDT_Wanted:
		{
			pkQuestWnd = CallQuestWnd( PgQuestManUtil::kWantedQuestWndName );
			if( !pkQuestWnd )
			{
				return;
			}
			kBtnVec.push_back( pkQuestWnd->GetControl(PgQuestManUtil::kAcceptButtonName) );
			kBtnVec.push_back( pkQuestWnd->GetControl(PgQuestManUtil::kRejectButtonName) );
		}break;
	case QDT_Wanted_Complete:
		{
			pkQuestWnd = CallQuestWnd( PgQuestManUtil::kWantedQuestCompleteWndName );
			if( !pkQuestWnd )
			{
				return;
			}
			kBtnVec.push_back( pkQuestWnd->GetControl(PgQuestManUtil::kCompleteButtonName) );
		}break;
	case QDT_VillageBoard:
		{
			pkQuestWnd = CallQuestWnd( PgQuestManUtil::kBoardQuestWndName );
			if( !pkQuestWnd )
			{
				return;
			}
			XUI::CXUI_Wnd* pkTitleWnd = pkQuestWnd->GetControl( std::wstring(L"FRM_TITLE") );
			if( pkTitleWnd )
			{
				pkTitleWnd->Text( TTW(pkQuestInfo->m_iTitleTextNo) );
			}
			kBtnVec.push_back( pkQuestWnd->GetControl(PgQuestManUtil::kAcceptButtonName) );
			kBtnVec.push_back( pkQuestWnd->GetControl(PgQuestManUtil::kRejectButtonName) );
		}break;
	}
	pkQuestWnd->Visible(true);

	XUI::CXUI_Wnd* pkTextWnd = pkQuestWnd->GetControl(_T("FRM_TEXT"));
	XUI::CXUI_Wnd* pkModelWnd = pkQuestWnd->GetControl(_T("FRM_MODEL"));

	//Form
	PgQUI_QuestDialog* pkUIQuestWnd = dynamic_cast<PgQUI_QuestDialog*>(pkQuestWnd);
	if( pkUIQuestWnd )
	{
		pkUIQuestWnd->Show(false, pkQuestInfo->ID(), pkQuestDialog->iDialogID);
	}

	//Text
	if( pkTextWnd )
	{
		std::wstring kText = rkInfo.kDialogText;
		Quest::lwQuestTextParser(kText);
		pkTextWnd->Text(kText);
	}

	//Face
	if( pkModelWnd )
	{
		char const *szUIModelName = "VillageBoard";
		char const *szActorName = "VillageBoardActor";
		PgUIModel* pkUIModel = g_kUIScene.FindUIModel(szUIModelName);
		if( !pkUIModel )
		{
			return;
		}

		if( pkUIModel->AddActorByID(szActorName, MB(rkInfo.kFaceID)) )
		{
			if( rkInfo.iFaceAniID )
			{
				pkUIModel->SetTargetAnimation(szActorName, rkInfo.iFaceAniID);
			}

			NiCamera *pkCamera = pkUIModel->GetCamera();
			if( pkCamera
			&&	POINT3() != rkInfo.kCamPos )
			{
				pkCamera->SetTranslate(rkInfo.kCamPos.x, rkInfo.kCamPos.y, rkInfo.kCamPos.z);
				pkCamera->Update(0.f);
			}
			else
			{
				NiActorManager* pkAM = pkUIModel->GetActor(szActorName);
				if( pkAM )
				{
					NiAVObject* pkRoot = pkAM->GetNIFRoot();
					if( pkRoot )
					{
						pkRoot->SetTranslate(NiPoint3(0.f, 0.f, -25.f));
					}
				}
			}
		}
	}

	//Button
	const ContQuestSelect& rkSelect = pkQuestDialog->kSelectList;
	ContXuiWnd::iterator btn_iter = kBtnVec.begin();
	ContQuestSelect::const_iterator select_iter = rkSelect.begin();
	while(	kBtnVec.end() != btn_iter
	&&		rkSelect.end() != select_iter )
	{
		const ContQuestSelect::value_type& rkElement = (*select_iter);
		ContXuiWnd::value_type pkBtn = (*btn_iter);
		if( pkBtn )
		{
			pkBtn->SetCustomData(&rkElement.iSelectID, sizeof(rkElement.iSelectID));
		}
		
		++btn_iter;
		++select_iter;
	}
	
}

bool PgQuestFullScreenDialog::IsQuestDialog()
{
	if( m_kNowInfo.Empty() )
	{
		return false;//퀘스트 대화 창 상태가 아님
	}
	return true;//퀘스트 대화 창 상태
}