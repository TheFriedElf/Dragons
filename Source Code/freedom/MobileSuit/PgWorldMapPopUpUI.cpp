#include "StdAfx.h"
#include "PgWorldMapUI.h"
#include "PgNifMan.h"
#include "Variant/PgMonsterBag.h"
#include "Variant/MonsterDefMgr.h"
#include "PgWorld.h"
#include "ServerLib.h"
#include "PgNiFile.h"
#include "PgPilotMan.h"
#include "PgXmlLoader.h"
#include "PgWorldMapPopUpUI.h"

#define PG_USE_NEW_POP

PgWorldMapPopUpUI::PgWorldMapPopUpUI(void) :
	m_kIconState(0),
	m_kFormState(0),
	m_kSubForm(0),
	m_kSubField(0),
	m_kSubTown(0),
	m_pContDefMap(NULL), m_pContDefCanidate(NULL), m_pContDefRoot(NULL)
{
	m_iMapNo = 0;
	g_kTblDataMgr.GetContDef(m_pContDefMap);
	g_kTblDataMgr.GetContDef(m_pContDefRoot);
	g_kTblDataMgr.GetContDef(m_pContDefCanidate);
	m_kContPopupInfo.clear();
}

PgWorldMapPopUpUI::~PgWorldMapPopUpUI(void)
{
	m_pkForm = 0;

	{
		XUIFormCont::iterator itr = m_kUI_IconCont.begin();
		while(itr != m_kUI_IconCont.end())
		{
			((*itr).second) = 0;
			++itr;
		}
	}
	m_kUI_IconCont.clear();
	{
		XUIFormCont::iterator itr = m_kUI_FormCont.begin();
		while(itr != m_kUI_FormCont.end())
		{
			((*itr).second) = 0;
			++itr;
		}
	}
	m_kUI_FormCont.clear();
}

bool PgWorldMapPopUpUI::Initialize(XUI::CXUI_Form* pkForm, lwUIWnd pkIconState, lwUIWnd pkFormState)
{
	m_pkForm = pkForm;
	lwUIWnd kMainForm(m_pkForm);
	m_kSubForm = kMainForm.GetControl("FRM_SUB_WORLDMAP");
	m_kSubField = kMainForm.GetControl("FRM_SUB_FIELD");
	m_kSubTown = kMainForm.GetControl("FRM_SUB_TOWN");
	m_kIconState = pkIconState;
	m_kFormState = pkFormState;
	RefreshForm(g_kWorldMapUI.ClickedMapNo());

	return true;
}

void PgWorldMapPopUpUI::Terminate()
{
	ReleaseControl();
}

void PgWorldMapPopUpUI::ReleaseControl()
{
	XUIFormCont::iterator wnd_itr = m_kUI_IconCont.begin();
	// 기존 UI들을 Visible을 꺼준다.
	while(wnd_itr != m_kUI_IconCont.end())
	{
		XUI::CXUI_Form* pkForm = ((*wnd_itr).second);
		if (pkForm)
		{
			m_pkForm->RemoveControl(((*wnd_itr).first));
			m_kSubForm()->RemoveControl(((*wnd_itr).first));
			m_kSubField()->RemoveControl(((*wnd_itr).first));
			m_kSubTown()->RemoveControl(((*wnd_itr).first));
		}
		++wnd_itr;
	}
	m_kUI_IconCont.clear();

	wnd_itr = m_kUI_FormCont.begin();
	// 기존 UI들을 Visible을 꺼준다.
	while(wnd_itr != m_kUI_FormCont.end())
	{
		XUI::CXUI_Form* pkForm = ((*wnd_itr).second);
		if (pkForm)
		{
			m_pkForm->RemoveControl(((*wnd_itr).first));
			m_kSubForm()->RemoveControl(((*wnd_itr).first));
			m_kSubField()->RemoveControl(((*wnd_itr).first));
			m_kSubTown()->RemoveControl(((*wnd_itr).first));
		}
		++wnd_itr;
	}
	m_kUI_FormCont.clear();
}

void PgWorldMapPopUpUI::MouseClickWorldMapPU(const POINT2& pt)
{
}

void PgWorldMapPopUpUI::MouseOverWorldMapPU(const POINT2& pt)
{
}

lwUIWnd PgWorldMapPopUpUI::NewXuiForm(	EXuiType const eType,
										XUIFormCont& rkFormCont,
										XUIFormCont::iterator &frm_itr,
										std::wstring wstrID,
										lwPoint3 const& ptPos,
										lwPoint2& ptSize,
										std::string strImg,
										SUVInfo const& rkUVInfo,
										bool bIncreaseFrmItr)
{
	bool bRet = false;
	bRet = PgNiFile::CheckFileExist(strImg.c_str());

	if (!bRet)
	{
		return lwUIWnd(0);
	}

	NiSourceTexture* tex = g_kNifMan.GetTexture(strImg);
	if (!tex)
	{
		return lwUIWnd(0);
	}

	XUI::CXUI_Form *pkSrcForm = 0;
	switch(eType)
	{
	case XT_ICON:
		{
			pkSrcForm = dynamic_cast<XUI::CXUI_Form*>(m_kIconState());
		}break;
	case XT_FORM:
		{
			pkSrcForm = dynamic_cast<XUI::CXUI_Form*>(m_kFormState());
		}break;
	default:
		{
			return lwUIWnd(0);
		}
	}
	pkSrcForm->IsIgnoreParentRect(false);
	
	XUI::CXUI_Form* pkCloneForm = 0;
	if (frm_itr != rkFormCont.end())
	{
		pkCloneForm = ((*frm_itr).second);
		if (bIncreaseFrmItr)
		{
			++frm_itr;
		}
	}
	else
	{
		pkCloneForm = (XUI::CXUI_Form*)pkSrcForm->VClone();
		rkFormCont.insert(std::make_pair(wstrID
			, pkCloneForm));
	}

	// 사이즈가 없으면 텍스쳐 사이즈로 함.
	if (ptSize.GetX() == 0 && ptSize.GetY() == 0)
	{
		ptSize.SetX(tex->GetWidth());
		ptSize.SetY(tex->GetHeight());
	}

	lwUIWnd kChild = lwUIWnd(pkCloneForm);
	pkCloneForm->UVInfo(rkUVInfo);
	kChild.SetID(wstrID);
	kChild.SetLocation3(ptPos);
	kChild.ChangeImage( (char*)strImg.c_str(), false );
	kChild.SetImgSize(ptSize);
	//lwActivateUI_Wnd(kChild());
	pkCloneForm->Visible(true);

	return kChild;
}

bool PgWorldMapPopUpUI::FindNPCFromActorName(SResourceIcon& rkRscIcon, SIconImage& rkIconImage, std::wstring const &rkActorName)
{
	std::wstring wstrIconID;
	if (!g_kResourceIcon.GetIconIDFromActorName(rkActorName, wstrIconID))
	{
		return false;
	}
	if (!g_kResourceIcon.GetIcon(rkRscIcon, wstrIconID))
	{
		return false;
	}
	if (!g_kResourceIcon.GetIconImage(rkIconImage, rkRscIcon.wstrImageID))
	{
		return false;
	}

	return true;
}

bool PgWorldMapPopUpUI::FindMonsterFromXMLPath(SResourceIcon& rkRscIcon, SIconImage& rkIconImage, std::wstring const &rkXMLPath)
{
	std::wstring wstrIconID;
	if (!g_kResourceIcon.GetIconIDFromXMLPath(rkXMLPath, wstrIconID))
	{
		// 못찾으면 물음표를 띄우자. 그래서 return false를 안함.
	}
	if (!g_kResourceIcon.GetIcon(rkRscIcon, wstrIconID))
	{
		return false;
	}
	if (!g_kResourceIcon.GetIconImage(rkIconImage, rkRscIcon.wstrImageID))
	{
		return false;
	}

	return true;
}

bool PgWorldMapPopUpUI::AddIcon( const SResourceIcon& rkRscIcon,
								 const SIconImage& rkIconImage,
								 int iCount,
								 std::wstring wstrIconName,
								 XUIFormCont& rkIconCont,
								 XUIFormCont::iterator &icon_itr,
								 char* szControlName,
								 int iNo,
								 POINT3 kDefaultPos,
								 POINT2 kGab,
								 int iMaxX )
{
	POINT2 kImageSize;
	kImageSize.x = rkIconImage.iWidth;
	kImageSize.y = rkIconImage.iHeight;

	int iCountX = (int)(iCount % iMaxX);
	int iCountY = (int)(iCount / iMaxX);
	NiPoint3 kIconPos3;
	kIconPos3.x = (float)(iCountX * kGab.x);
	kIconPos3.y = (float)((iCountY * kGab.y)%200);
	kIconPos3.z = kDefaultPos.z;
	kIconPos3.x += kDefaultPos.x;
	kIconPos3.y += kDefaultPos.y;

	SUVInfo kUVInfo = SUVInfo(rkIconImage.iMaxX, rkIconImage.iMaxY, rkRscIcon.iIdx);
	
	lwUIWnd kWnd = NewXuiForm(XT_ICON, m_kUI_IconCont, icon_itr, wstrIconName, lwPoint3(kIconPos3), lwPoint2(kImageSize), MB(rkIconImage.wstrPath), kUVInfo);
	if (!kWnd())
	{
		return false;
	}
	kWnd.SetParent(m_kSubForm);
	kWnd.SetLocation3(lwPoint3(kIconPos3));

	lwUIWnd kIcon = kWnd.GetControl(szControlName);
	XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(kIcon());
	if (pkIcon)
	{
		SIconInfo kIconInfo;
		kIconInfo.iIconGroup = KUIG_MONSTER;
		kIconInfo.iIconKey = iNo;
		kIconInfo.iIconResNumber = rkRscIcon.iIdx;
		pkIcon->SetIconInfo(kIconInfo);
	} 

	kWnd.Visible(iCount<20);

	return true;
}

void PgWorldMapPopUpUI::RefreshForm_Field()
{
	if( !m_pkForm || !m_kSubForm() || !m_kIconState() || !m_kFormState() )
	{
		return;
	}

	// BG Form Size
	{
		POINT2 ptMainFormSize;
		ptMainFormSize.x = 950;
		ptMainFormSize.y = 668;
		m_kSubForm()->Size(ptMainFormSize);
		POINT3I ptMainFormLoc;
		ptMainFormLoc.x = 37;
		ptMainFormLoc.y = 50;
		ptMainFormLoc.z = -2;
		m_kSubForm()->Location(ptMainFormLoc);

		//Close Btn
		lwUIWnd kMainForm(m_kSubForm());
		lwUIWnd kClose = kMainForm.GetControl("Close");
		XUI::CXUI_Wnd* pkClose = dynamic_cast<XUI::CXUI_Wnd*>(kClose());
		if (pkClose)
		{
			POINT3I ptCloseBtnLoc;
			ptCloseBtnLoc.x = 871;
			ptCloseBtnLoc.y = 42;
			ptCloseBtnLoc.z = 0;
			pkClose->Location(ptCloseBtnLoc);
		}
	}
	
	if (m_kSubField())
	{
		m_kSubField()->Visible(true);
	}
	if (m_kSubTown())
	{
		m_kSubTown()->Visible(false);
	}

	XUIFormCont::iterator form_itr = m_kUI_FormCont.begin();
	XUIFormCont::iterator icon_itr = m_kUI_IconCont.begin();
	// 기존 UI들을 Visible을 꺼준다.
	while(form_itr != m_kUI_FormCont.end())
	{
		XUI::CXUI_Form* pkForm = ((*form_itr).second);
		if (pkForm)
		{
			pkForm->Visible(false);
		}
		++form_itr;
	}
	while(icon_itr != m_kUI_IconCont.end())
	{
		XUI::CXUI_Form* pkForm = ((*icon_itr).second);
		if (pkForm)
		{
			pkForm->Visible(false);
		}
		++icon_itr;
	}
	form_itr = m_kUI_FormCont.begin();
	icon_itr = m_kUI_IconCont.begin();

	lwUIWnd kWnd(0);

	CONT_DEFMAP::const_iterator map_it = m_pContDefMap->find(m_iMapNo);	//맵번호로 맵 데프에서 찾아서
	bool bChaos = false;
	
	// 미션 이미지.
	{
		if (m_pContDefMap->end() != map_it)	//있으면
		{
			TBL_DEF_MAP const& rkDefMap = (*map_it).second;

			std::wstring wstrMapNo = (std::wstring)(BM::vstring(m_iMapNo));
			wstrMapNo.erase(wstrMapNo.begin(), wstrMapNo.begin()+1);
			std::wstring wstrMissionNo = _T("../Data/6_UI/mission/ms");
			wstrMissionNo += wstrMapNo;

			static lwPoint3 const kCardPos[4] = {lwPoint3(425,122,2), lwPoint3(532,122,2), lwPoint3(639,122,2), lwPoint3(746,122,2)};
			static lwPoint3 const kChaosPos[4] = {lwPoint3(504,122,5), lwPoint3(611,122,5), lwPoint3(718,122,5), lwPoint3(827,122,5)};
			lwPoint2 kCardSize(103,123);
			bChaos = rkDefMap.iAttr && GATTR_CHAOS_F;

			std::wstring wstrNo;
			std::wstring wstrName;
			std::wstring wstrChaos = L"FRM_CHAOS_";
			for (int i = 1; i<5; ++i)
			{
				wstrNo = wstrMissionNo;
				wstrNo += _T("_");
				wstrNo += (std::wstring)(BM::vstring(i));
				wstrNo += _T(".tga");
				wstrName = L"MISSION_";
				wstrName+=(std::wstring)(BM::vstring(i));
				kWnd = NewXuiForm(XT_FORM, m_kUI_FormCont, form_itr, wstrName, kCardPos[i-1], kCardSize, MB(wstrNo));
				if (kWnd()) 
				{
					kWnd.SetParent(m_kSubForm());
					if (bChaos)
					{
						lwUIWnd kChaosIcon = NewXuiForm(XT_FORM, m_kUI_FormCont, form_itr, wstrChaos+=(std::wstring)(BM::vstring(i)), kChaosPos[i-1], lwPoint2(24,25), "../Data/6_UI/common/cmChaosMbig.tga");
						if (kChaosIcon())
						{
							kChaosIcon.SetParent(m_kSubForm);
						}	
					}
				}
			}
		}
	}

	// 맵 이미지.
	// 맵 이미지. (텍스쳐 사이즈를 참고).
	std::wstring wstrMapNo = (std::wstring)(BM::vstring(m_iMapNo));
	std::wstring wstrMapImage;
	if (g_kWorldMapUI.GetImgFromShardId(wstrMapImage, wstrMapNo))
	{
		// 223 143
		NiSourceTexture* tex = g_kNifMan.GetTexture(MB(wstrMapImage));
		if (tex)
		{
			NiPoint3 const kIconPt(223 - (tex->GetWidth() * 0.5f), 143 - (tex->GetHeight() * 0.5f), 2);
			kWnd = NewXuiForm(XT_FORM, m_kUI_FormCont, form_itr, L"MAP_IMAGE", lwPoint3(kIconPt), lwPoint2(0,0), MB(wstrMapImage));
			if (kWnd()) kWnd.SetParent(m_kSubForm());
		}
	}

	// 중복 체크할 컨테이너
	

	// 리소스 테이블 가져오기.
	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);
	SMonsterControl kMonControlData;
	GET_DEF(PgMonsterBag, kMonsterBag);
	GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
	int iCount = 0;

	// 몬스터 번호 알아보자.
	int iMapNo = m_iMapNo;
	
	
	CONT_SET_DATA kAddedMonster;
	kAddedMonster.clear();
	
	// 일반 몹이 아닌 정예 등.
	SPopupInfo* pkPopupInfo = NULL;
	CONT_MAP_POPUP_INFO::iterator info_it = m_kContPopupInfo.find(iMapNo);	//먼저 찾아보자

	if (m_kContPopupInfo.end() == info_it)	//없으면
	{
		SPopupInfo kNewInfo;

//		CONT_DEFMAP::const_iterator map_it = m_pContDefMap->find(iMapNo);	//맵번호로 맵 데프에서 찾아서
		if (m_pContDefMap->end() != map_it)	//있으면
		{
			TBL_DEF_MAP const& rkDefMap = (*map_it).second;
			CONT_MISSION_NO::const_iterator msn_it = rkDefMap.kContMissionNo.begin();	//미션맵이 있으면
			kNewInfo.iMapNo = iMapNo;
			CONT_SET_DATA kLoadedMap;
			while (rkDefMap.kContMissionNo.end() != msn_it)
			{
				CONT_DEF_MISSION_ROOT::const_iterator root_it = m_pContDefRoot->find((*msn_it));
				if (m_pContDefRoot->end() != root_it)
				{
					TBL_DEF_MISSION_ROOT const& rkRoot = (*root_it).second;
					for (int i = 0; i < MAX_MISSION_LEVEL; ++i)	//미션 레벨당
					{
						int const iLev = rkRoot.aiLevel[i];
						
						if(0<iLev)
						{
							CONT_DEF_MISSION_CANDIDATE::const_iterator cand_it = m_pContDefCanidate->find(iLev);
							if (m_pContDefCanidate->end() != cand_it)
							{
								CONT_SET_DATA& rkSetMission = kNewInfo.kSetMission[i];
								
								TBL_DEF_MISSION_CANDIDATE const& rkCand = (*cand_it).second;
								for (int k = 0; k < MAX_MISSION_CANDIDATE; ++k)
								{
									int const iCandi = rkCand.aiCandidate[k];
									CONT_SET_DATA::const_iterator loaded_it = kLoadedMap.find(iCandi);
									if (0<iCandi && kLoadedMap.end() == loaded_it)	//0 이상이고 한번도 찾은적이 없다면
									{
										kLoadedMap.insert(iCandi);
										SMonsterControl kGetMon;
										if (kMonsterBag.GetMonster(iCandi, kGetMon))
										{
											GetMonsterList(kGetMon, kMonsterDefMgr, kNewInfo);
										}
									}
								}

								int const iCandiEnd = rkCand.iCandidate_End;
								CONT_SET_DATA::const_iterator loaded_it = kLoadedMap.find(iCandiEnd);
								if (0<iCandiEnd && kLoadedMap.end() == loaded_it)//0 이상이고 한번도 찾은적이 없다면
								{
									kLoadedMap.insert(iCandiEnd);
									SMonsterControl kGetMon;
									if (kMonsterBag.GetMonster(iCandiEnd, kGetMon))
									{
										GetMonsterList(kGetMon, kMonsterDefMgr, kNewInfo);
									}
								}
							}
						}
					}
				}
				++msn_it;
			}
			SMonsterControl kGetMon;
			if (kMonsterBag.GetMonster(iMapNo, kGetMon))	// 일반 필드몹 얻자
			{
				GetMonsterList(kGetMon, kMonsterDefMgr, kNewInfo);
			}
		}	

		if (kNewInfo.iMapNo)
		{
			std::pair<CONT_MAP_POPUP_INFO::iterator, bool> ib = m_kContPopupInfo.insert(std::make_pair(kNewInfo.iMapNo, kNewInfo));
			CONT_MAP_POPUP_INFO::iterator it = ib.first;

			pkPopupInfo = &((*it).second);
		}
	}
	else
	{
		pkPopupInfo = &(*info_it).second;
	}
	// 미션맵 레벨별로 뽑아두고 레벨별로 표시해 준다. 16개까지가 미션맵이 될 수 있기 때문에.
	
	if (pkPopupInfo)
	{
		CONT_SET_DATA::const_iterator mon_itr = pkPopupInfo->kSetData.begin();
		while(mon_itr != pkPopupInfo->kSetData.end())
		{
			int const iMonNo = (*mon_itr);
			CMonsterDef const *pDef = kMonsterDefMgr.GetDef(iMonNo);

			if (pDef)
			{
				int iRes = pDef->GetAbil(AT_NAMENO);
#ifdef PG_USE_NEW_POP

				PgPilotMan::stClassInfo kClassInfo;
				g_kPilotMan.GetClassInfo(iRes,0,kClassInfo);

				char const *pcClassID = kClassInfo.m_kActorID.c_str();

				CONT_SET_DATA::const_iterator check_itr = kAddedMonster.find(pDef->NameNo());
				if (pcClassID && iCount < 10 && check_itr == kAddedMonster.end())
				{
					char const* pcPath = PgXmlLoader::GetPathByID(pcClassID);
					if (!pcPath)
					{
						++mon_itr;
						continue;
					}
					SResourceIcon kRscIcon;
					SIconImage kIconImage;
					if (!FindMonsterFromXMLPath(kRscIcon, kIconImage, UNI(pcPath)))
#else
				TBL_DEF_RES kRes;
				CONT_DEFRES::const_iterator itrDest = pContDefRes->find(iRes);
				if(itrDest != pContDefRes->end() && iCount < 10)
				{
					kRes = itrDest->second;
					SResourceIcon kRscIcon;
					SIconImage kIconImage;
					if (!FindMonsterFromXMLPath(kRscIcon, kIconImage, kRes.strXmlPath))
#endif
					{
						++mon_itr;
						continue;
					}

					POINT3 kDefaultPos;
					kDefaultPos.x = 106;
					kDefaultPos.y = 377;
					kDefaultPos.z = 5;
					POINT2 kGab;
					kGab.y = kGab.x = 50;

					std::wstring wstrIconName = _T("ICON_FIELD_") + (std::wstring)(BM::vstring(iCount));
					if (AddIcon(kRscIcon, kIconImage, iCount, wstrIconName, m_kUI_IconCont, icon_itr, "MAIN_ICON", iMonNo, kDefaultPos, kGab, 5))
					{
#ifdef PG_USE_NEW_POP
						kAddedMonster.insert(pDef->NameNo());
#endif
						if (bChaos)
						{
							std::wstring wstrChaos = L"FRM_CHAOSMON_";
							lwUIWnd kChaosIcon = NewXuiForm(XT_FORM, m_kUI_FormCont, form_itr, wstrChaos+=(std::wstring)(BM::vstring(iCount)), 
								lwPoint3(kDefaultPos.x + kGab.x * (iCount%5) + 23, kDefaultPos.y + kGab.y * (int)(iCount/5),kDefaultPos.z+10), 
								lwPoint2(17,18), "../Data/6_UI/common/cmChaosMsm.tga");
							if (kChaosIcon())
							{
								kChaosIcon.SetParent(m_kSubForm);
							}	
						}

						iCount += 1;
					}
				}
			}

			++mon_itr;
		}

	// 일반 미션 몹들만 표시 해준다.
	int iTempNameCount = 0;
	for (int line=1 ; line<5/*MBE_MAX*/ ; line++)
	{
		iCount = 0;
		CONT_SET_DATA const & rkMonData = pkPopupInfo->kSetMission[line]; // 미션 몹
		CONT_SET_DATA::const_iterator mon_itr = rkMonData.begin();
		kAddedMonster.clear();

		while(mon_itr != rkMonData.end())
		{
			int const &riMonNo = (*mon_itr);
			CMonsterDef const *pDef = kMonsterDefMgr.GetDef(riMonNo);

			if (pDef && iCount < 5)
			{
				int iRes = pDef->GetAbil(AT_NAMENO);
#ifdef PG_USE_NEW_POP
				PgPilotMan::stClassInfo kClassInfo;
				g_kPilotMan.GetClassInfo(iRes,0,kClassInfo);
				char const *pcClassID = kClassInfo.m_kActorID.c_str();
				CONT_SET_DATA::const_iterator check_itr = kAddedMonster.find(pDef->NameNo());
				if (pcClassID && check_itr == kAddedMonster.end())
				{
					char const* pcPath = PgXmlLoader::GetPathByID(pcClassID);
					if (!pcPath)
					{
						++mon_itr;
						continue;
					}
					SResourceIcon kRscIcon;
					SIconImage kIconImage;
					if (!FindMonsterFromXMLPath(kRscIcon, kIconImage, UNI(pcPath)))
#else
				TBL_DEF_RES kRes;
				CONT_DEFRES::const_iterator itrDest = pContDefRes->find(iRes);
				if(itrDest != pContDefRes->end())
				{
					// 아이콘 추가 해주자.
					kRes = itrDest->second;
					SResourceIcon kRscIcon;
					SIconImage kIconImage;
					if (!FindMonsterFromXMLPath(kRscIcon, kIconImage, kRes.strXmlPath))
#endif
					{
						++mon_itr;
						continue;
					}

					POINT3 kDefaultPos;
					kDefaultPos.x = 602;
					kDefaultPos.y = 357.0f + ((line-1) * 56);		// line 값으로 4줄을 표현 한다.
					kDefaultPos.z = 5;
					POINT2 kGab;
					kGab.x = 50;
					kGab.y = 0;
					std::wstring wstrIconName = _T("ICON_MISSION_") + (std::wstring)(BM::vstring(line));
					wstrIconName = wstrIconName + _T("_") + (std::wstring)(BM::vstring(iCount));
					if (AddIcon(kRscIcon, kIconImage, iCount, wstrIconName, m_kUI_IconCont, icon_itr, "MAIN_ICON", riMonNo, kDefaultPos, kGab, 5))
					{
#ifdef PG_USE_NEW_POP
						kAddedMonster.insert(pDef->NameNo());
#endif
						if (bChaos)
						{
							std::wstring wstrChaos = L"FRM_CHAOSMISSOINMON_";
							lwUIWnd kChaosIcon = NewXuiForm(XT_FORM, m_kUI_FormCont, form_itr, wstrChaos+=(std::wstring)(BM::vstring(iTempNameCount++)), 
								lwPoint3(kDefaultPos.x + kGab.x * (iCount%5) + 23, kDefaultPos.y /*+ kGab.y * (int)(iCount/5)*/,kDefaultPos.z+10), 
								lwPoint2(17,18), "../Data/6_UI/common/cmChaosMsm.tga");
							if (kChaosIcon())
							{
								kChaosIcon.SetParent(m_kSubForm);
							}	
						}
						
						iCount += 1;
					}
				}
			}

			++mon_itr;
		}
	}

	// 보스, 정예 등 표시.
	kAddedMonster.clear();
	iCount = 0;
	CONT_SET_DATA::const_iterator grademon_itr = pkPopupInfo->kSetElite.begin();
	while(grademon_itr != pkPopupInfo->kSetElite.end())
	{
		int const &riMonNo = (*grademon_itr);
		const CMonsterDef *pDef = kMonsterDefMgr.GetDef(riMonNo);

		if (pDef && iCount < 5)
		{
			int iRes = pDef->GetAbil(AT_NAMENO);
#ifdef PG_USE_NEW_POP
			PgPilotMan::stClassInfo	kClassInfo;
			g_kPilotMan.GetClassInfo(iRes,0,kClassInfo);
			char const *pcClassID = kClassInfo.m_kActorID.c_str();

			CONT_SET_DATA::const_iterator check_itr = kAddedMonster.find(pDef->NameNo());
			if (pcClassID && check_itr == kAddedMonster.end())
			{
				char const* pcPath = PgXmlLoader::GetPathByID(pcClassID);
				if (!pcPath)
				{
					++grademon_itr;
					continue;
				}
				SResourceIcon kRscIcon;
				SIconImage kIconImage;
				if (!FindMonsterFromXMLPath(kRscIcon, kIconImage, UNI(pcPath)))
#else
			TBL_DEF_RES kRes;
			CONT_DEFRES::const_iterator itrDest = pContDefRes->find(iRes);
			if(itrDest != pContDefRes->end())
			{
				// 중복 이름이 이미 추가 됐으면 continue
				std::wstring wstrXmlPath = kRes.strXmlPath;
				UPR(wstrXmlPath);
				CONT_SET_WSTRING::iterator find_grade_itr = kAddedGradeMonster.find(wstrXmlPath);
				if (find_grade_itr != kAddedGradeMonster.end())
				{
					++grademon_itr;
					continue;
				}

				// 아이콘 추가.
				kRes = itrDest->second;
				SResourceIcon kRscIcon;
				SIconImage kIconImage;
				if (!FindMonsterFromXMLPath(kRscIcon, kIconImage, kRes.strXmlPath))
#endif
				{
					++grademon_itr;
					continue;
				}

				POINT3 kDefaultPos;
				kDefaultPos.x = 106;
				kDefaultPos.y = 534;
				kDefaultPos.z = 5;
				POINT2 kGab;
				kGab.x = 50;
				kGab.y = 0;
				std::wstring wstrIconName = _T("ICON_GRADE_") + (std::wstring)(BM::vstring(iCount));
				wstrIconName = wstrIconName + _T("_") + (std::wstring)(BM::vstring(iCount));
				if (AddIcon(kRscIcon, kIconImage, iCount, wstrIconName, m_kUI_IconCont, icon_itr, "MAIN_ICON", riMonNo, kDefaultPos, kGab, 5))
				{
					if (bChaos)
					{
						std::wstring wstrChaos = L"FRM_CHAOSGRADE_";
						lwUIWnd kChaosIcon = NewXuiForm(XT_FORM, m_kUI_FormCont, form_itr, wstrChaos+=(std::wstring)(BM::vstring(iCount)), 
							lwPoint3(kDefaultPos.x + kGab.x * (iCount%5) + 23, kDefaultPos.y + kGab.y * (int)(iCount/5),kDefaultPos.z+10), 
							lwPoint2(17,18), "../Data/6_UI/common/cmChaosMsm.tga");
						if (kChaosIcon())
						{
							kChaosIcon.SetParent(m_kSubForm);
						}	
					}
					iCount += 1;
#ifdef PG_USE_NEW_POP
					kAddedMonster.insert(pDef->NameNo());
#else
					kAddedGradeMonster.insert(wstrXmlPath);
#endif
				}
			}
		}

		++grademon_itr;
	}
	}

	m_kIconState()->Close();
	m_kFormState()->Close();

	// 텍스트. 이름
	bool bFindShard = false;
	SWorldMapShard kShard;
	if (g_kWorldMapUI.GetShardFromId(kShard, wstrMapNo))
	{
		bFindShard = true;
	}
	int iFindMapNo = 0;
	if (bFindShard && kShard.wstrNameDefString.size() != 0)
	{
		iFindMapNo = _wtoi(kShard.wstrNameDefString.c_str());
	}
	else
	{
		iFindMapNo = iMapNo;
	}

	wchar_t const *pText = NULL;
	if(iFindMapNo != 0 && GetDefString(iFindMapNo, pText))
	{
		lwUIWnd kTextBox = m_kSubField.GetControl("FRM_TEXT_MAPNAME");
		if (!kTextBox.IsNil())
		{
			std::wstring wstrText;	//폰트 초기화
			wstrText += pText;
			kTextBox()->Text(wstrText);
			kTextBox()->Visible(true);
		}
	}
	// 텍스트. 위치
	if (bFindShard)
	{
		lwUIWnd kTextBox_Pos = m_kSubField.GetControl("FRM_TEXT_POSITION");
		if (!kTextBox_Pos.IsNil())
		{
			std::wstring wstrText;	//폰트 초기화
			wstrText += TTW(kShard.iLocationTTW);
			kTextBox_Pos()->Text(wstrText);
			kTextBox_Pos()->Visible(true);
		}
	}

	/*PgPilot* pkPilot = g_kPilotMan.GetPlayerPilot();
	if( pkPilot )
	{
	PgPlayer* pkPC = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
	if( pkPC )
	{
	const CONT_DEF_MISSION_ROOT* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);
	PgUserMissionReport& rkRep = pkPC->MissionReport();
	//rkRep.Get();
	}
	}*/

	//sort
	m_kSubForm()->SortChildGreater();
	m_kSubForm()->VOnLocationChange();
	m_kSubForm()->VRefresh();
}

void PgWorldMapPopUpUI::RefreshForm_Town()
{
	if( !m_pkForm || !m_kSubForm() || !m_kIconState() || !m_kFormState() )
	{
		return;
	}

	// BG Form Size
	{
		POINT2 ptMainFormSize;
		ptMainFormSize.x = 447;
		ptMainFormSize.y = 668;
		m_kSubForm()->Size(ptMainFormSize);
		POINT3I ptMainFormLoc;
		ptMainFormLoc.x = 288;
		ptMainFormLoc.y = 50;
		ptMainFormLoc.z = -2;
		m_kSubForm()->Location(ptMainFormLoc);

		//Close Btn
		lwUIWnd kMainForm(m_kSubForm());
		lwUIWnd kClose = kMainForm.GetControl("Close");
		XUI::CXUI_Wnd* pkClose = dynamic_cast<XUI::CXUI_Wnd*>(kClose());
		if (pkClose)
		{
			POINT3I ptCloseBtnLoc;
			ptCloseBtnLoc.x = 369;
			ptCloseBtnLoc.y = 43;
			ptCloseBtnLoc.z = 0;
			pkClose->Location(ptCloseBtnLoc);
		}
	}
	
	if (m_kSubField())
	{
		m_kSubField()->Visible(false);
	}
	if (m_kSubTown())
	{
		m_kSubTown()->Visible(true);
	}
	
	XUIFormCont::iterator form_itr = m_kUI_FormCont.begin();
	XUIFormCont::iterator icon_itr = m_kUI_IconCont.begin();
	// 기존 UI들을 Visible을 꺼준다.
	while(form_itr != m_kUI_FormCont.end())
	{
		XUI::CXUI_Form* pkForm = ((*form_itr).second);
		if (pkForm)
		{
			pkForm->Visible(false);
		}
		++form_itr;
	}
	while(icon_itr != m_kUI_IconCont.end())
	{
		XUI::CXUI_Form* pkForm = ((*icon_itr).second);
		if (pkForm)
		{
			pkForm->Visible(false);
		}
		++icon_itr;
	}
	form_itr = m_kUI_FormCont.begin();
	icon_itr = m_kUI_IconCont.begin();

	lwUIWnd kWnd(0);
	// 맵 이미지. (텍스쳐 사이즈를 참고).
	std::wstring wstrMapNo = (std::wstring)(BM::vstring(m_iMapNo));
	std::wstring wstrMapImage;
	if (g_kWorldMapUI.GetImgFromShardId(wstrMapImage, wstrMapNo))
	{
		// 223 143
		NiSourceTexture* tex = g_kNifMan.GetTexture(MB(wstrMapImage));
		NiPoint3 kIconPt;
		kIconPt.x = 223 - (tex->GetWidth() * 0.5f);
		kIconPt.y = 143 - (tex->GetHeight() * 0.5f);
		kIconPt.z = 2;
		kWnd = NewXuiForm(XT_FORM, m_kUI_FormCont, form_itr, L"MAP_IMAGE", lwPoint3(kIconPt), lwPoint2(0,0), MB(wstrMapImage));
		if (kWnd()) kWnd.SetParent(m_kSubForm());
	}

	// NPC Data
	MapNpcSet kNpcSet;
	g_kWorldMapUI.GetNpcSet(m_iMapNo, kNpcSet);

	int iCount = 0;
	int iMax = 40;
	MapNpcSet::iterator npc_itr = kNpcSet.begin();
	while(npc_itr != kNpcSet.end())
	{
		if (iMax <= iCount)
		{
			break;
		}
		SNpcCreateInfo kNpcInfo = (*npc_itr);

		SResourceIcon kRscIcon;
		SIconImage kIconImage;
		if (!FindNPCFromActorName(kRscIcon, kIconImage, kNpcInfo.wstrActor))
		{
			++npc_itr;
			continue;
		}

		POINT3 kDefaultPos;
		kDefaultPos.x = 103;
		kDefaultPos.y = 377;
		kDefaultPos.z = 5;
		POINT2 kGab;
		kGab.x = 50;
		kGab.y = 50;

		std::wstring wstrIconName = _T("ICON_NPC_") + (std::wstring)(BM::vstring(iCount));
		if (AddIcon(kRscIcon, kIconImage, iCount, wstrIconName, m_kUI_IconCont, icon_itr, "MAIN_ICON", kNpcInfo.iID, kDefaultPos, kGab, 5))
		{
			iCount += 1;
		}

		++npc_itr;
	}

	m_kIconState()->Close();
	m_kFormState()->Close();

	// 텍스트. 이름
	bool bFindShard = false;
	SWorldMapShard kShard;
	if (g_kWorldMapUI.GetShardFromId(kShard, wstrMapNo))
	{
		bFindShard = true;
	}
	int iFindMapNo = 0;
	if (bFindShard && kShard.wstrNameDefString.size() != 0)
	{
		iFindMapNo = _wtoi(kShard.wstrNameDefString.c_str());
	}
	else
	{
		iFindMapNo = m_iMapNo;
	}

	const wchar_t *pText = NULL;
	if(iFindMapNo != 0 && GetDefString(iFindMapNo, pText))
	{
		lwUIWnd kTextBox = m_kSubTown.GetControl("FRM_TEXT_MAPNAME");
		if (!kTextBox.IsNil())
		{
			std::wstring wstrText;	//폰트 초기화
			wstrText += pText;
			kTextBox()->Text(wstrText);
			kTextBox()->Visible(true);
		}
	}
	// 텍스트. 위치
	if (bFindShard)
	{
		lwUIWnd kTextBox_Pos = m_kSubTown.GetControl("FRM_TEXT_POSITION");
		if (!kTextBox_Pos.IsNil())
		{
			std::wstring wstrText;	//폰트 초기화
			wstrText += TTW(kShard.iLocationTTW);
			kTextBox_Pos()->Text(wstrText);
			kTextBox_Pos()->Visible(true);
		}
	}
	//sort
	m_kSubForm()->SortChildGreater();
	m_kSubForm()->VOnLocationChange();
	m_kSubForm()->VRefresh();
}

void PgWorldMapPopUpUI::RefreshForm_Dungeon()
{
}

void PgWorldMapPopUpUI::RefreshForm(int iMapNo)
{
	m_iMapNo = iMapNo;
	std::wstring wstrMapNo = (std::wstring)(BM::vstring(m_iMapNo));

	SWorldMapShard kShard;
	if (g_kWorldMapUI.GetShardFromId(kShard, wstrMapNo))
	{
		// Monster 리스트.
		// NPC 리스트.

		switch(kShard.eShardType)
		{
		case SWorldMapShard::EShardType_Field :
			{
				RefreshForm_Field();
			}break;
		case SWorldMapShard::EShardType_Town :
			{
				RefreshForm_Town();
			}break;
		case SWorldMapShard::EShardType_Dungeon :
			{
				RefreshForm_Dungeon();
			}break;
		default :
			{
			}break;
		}
	}
}

void PgWorldMapPopUpUI::GetMonsterList(SMonsterControl const& rkGetMon, CMonsterDefMgr const& rkMonsterDefMgr, SPopupInfo& rkPopInfo)
{
	for (int j=0 ; j<5/*MBE_MAX*/ ; j++)
	{
		CONT_SET_DATA const& rkMonData = rkGetMon.kSetMonster[j]; //
		CONT_SET_DATA::const_iterator mon_itr = rkMonData.begin();
		while (rkMonData.end() != mon_itr)
		{
			int const iMonNo = (*mon_itr);
			CMonsterDef const *pDef = rkMonsterDefMgr.GetDef(iMonNo);
			if (pDef)
			{
				int iGrade = pDef->GetAbil(AT_GRADE);
				
				if (iGrade == EMGRADE_ELITE || iGrade == EMGRADE_BOSS )	//정예몹 이상
				{
					rkPopInfo.kSetElite.insert(iMonNo);
				}
				else if(!j)									//일반 필드
				{
					rkPopInfo.kSetData.insert(iMonNo);
				}
				else													//미션맵
				{
					rkPopInfo.kSetMission[j].insert(iMonNo);
				}
			}

			++mon_itr;
		}
	}
}