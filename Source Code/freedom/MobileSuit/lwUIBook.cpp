#include "stdafx.h"
#include "PgWorldMapUI.h"
#include "PgNifMan.h"
#include "variant/PgQuestInfo.h"
#include "lwUIBook.h"
#include "PgResourceIcon.h"
#include "CreateUsingNiNew.inl"
#include "PgWorldMapPopUpUI.h"
#include "ServerLib.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgQuestMan.h" 
#include "PgQuestUI.h"
#include "PgQuestDialog.h"
#include "PgNetwork.h"
#include "lwUI.h"
#include "PgUIModel.h"
#include "PgUIScene.h"
#include "PgActorUtil.h"
#include "PgAchieveNfyMgr.h"
#include "lwUIQuest.h"

extern bool lwIsEndedQuest(unsigned int sQuestID);

bool SetIconInfo(XUI::CXUI_Wnd* pkWnd, SResourceIcon const& rkRscIcon, SIconImage const& rkIconImage, int const iNo);
SPopupInfo kBookMonsterInfo;
lwUIBook::ContMonDistinctVec kBookMonsterInfoUI;
//! 스크립팅 시스템에 등록한다.
bool lwUIBook::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "SetWorldPage", lwUIBook::SetWorldPage);
	def(pkState, "SetBookMiniMap", lwUIBook::SetBookMiniMap);
	def(pkState, "SetQuestPage", lwUIBook::SetQuestPage);
	def(pkState, "SetAchievePage", lwUIBook::SetAchievePage);
	def(pkState, "OnClickBookQuest", lwUIBook::OnClickBookQuest);
	def(pkState, "OnClickBookNPC", lwUIBook::OnClickBookNPC);
	def(pkState, "OnClickBookAchieveIcon", lwUIBook::OnClickBookAchieveIcon);
	def(pkState, "OnClickBookAchieveGetItem", lwUIBook::OnClickBookAchieveGetItem);
	def(pkState, "OnClickBookRankPage", lwUIBook::OnClickBookRankPage);
	def(pkState, "OnClickSetBestItem", lwUIBook::OnClickSetBestItem);
	def(pkState, "SetClearBestItem", lwUIBook::SetClearBestItem);
	def(pkState, "CallBookItemToolTip", lwUIBook::CallBookItemToolTip);
	def(pkState, "CallActorInfo", lwUIBook::CallActorInfo);
	def(pkState, "ClickActorSlot", lwUIBook::ClickActorSlot);
	def(pkState, "ClickSearchMyLoc", lwUIBook::ClickSearchMyLoc);
	def(pkState, "ClickSearchNpcLoc", lwUIBook::ClickSearchNpcLoc);
	def(pkState, "OnClickQuestListTypeChange", lwUIBook::OnClickQuestListTypeChange);
	def(pkState, "OnCallDetailAchievementView", lwUIBook::OnCallDetailAchievementView);
	def(pkState, "OnCloseBookUI", lwUIBook::OnCloseBookUI);
	
	return true;
}

//예외처리 해야 함
int const LAVALON = 6000900;
int const LAVALON_HEALT = 6000901;
int const BOOK_MONSTERCOUNT_PER_LINE = 5;
std::wstring const BTN_LEFT = L"BTN_LEFT";
std::wstring const BTN_RIGHT = L"BTN_RIGHT";

bool lwUIBook::SetWorldPage(lwUIWnd kWnd, int const iMapNo, int const iType)
{
	if(kWnd.IsNil() || 0>=iMapNo || !lwIsOpenGroundByNo(iMapNo) )
	{
		return false;
	}

	switch(iType)
	{
	/*case 0:	//필드
		{
			SetFieldPage(kWnd, iMapNo);
		}break;*/
	case 1:	//마을
		{
			SetTownPage(kWnd, iMapNo);
		}break;
	case 0:	//보스
		{
			//SetBossPage(kWnd, iMapNo);
			int realNo = iMapNo;
			if(g_pkWorld)
			{
				int const iBossNo = lua_tinker::call<int, int>("FindCustomBossByMapNo", iMapNo);
				int const iPMapNo = lua_tinker::call<int, int>("FindPMAP_NO", g_pkWorld->MapNo());
				if(0<iPMapNo && iMapNo==iPMapNo)
				{
					realNo = g_pkWorld->MapNo();
				}
				else if(iMapNo!=g_pkWorld->MapNo() || iBossNo==g_pkWorld->MapNo())
				{
					if(0 < iBossNo)
					{
						realNo = iBossNo;
					}
				}
			}
			SetFieldPage(kWnd, realNo);
		}break;
	}

	return true;
}

void lwUIBook::SetBookMiniMap(lwUIWnd kWnd, int const iMapNo)
{
	if(kWnd.IsNil() || 0>=iMapNo)
	{
		return;
	}

	//std::wstring kMapPath;
	lwWString kMapPath = lwGetMiniMapPath(iMapNo);
	if ( kMapPath.IsNil() )//!g_kWorldMapUI.GetMiniMapPath(iMapNo, kMapPath) )
	{
		kWnd.Visible(false);
		return;
	}

	XUI::CXUI_Wnd* pkImg = kWnd();
	pkImg->DefaultImgName(kMapPath.GetWString()); 

	POINT2 kLoc(0, 156);
	pkImg->Size(POINT2(512, 384));
	pkImg->ImgSize(POINT2(512, 512));
	pkImg->Scale(0.58f);
	pkImg->Location(kLoc);
	pkImg->SetInvalidate();
	kWnd.Visible(true);
}

bool lwUIBook::GetMonsterBookInfo(int const MonID, SBookMonsterInfo& kInfo, bool const IsElite, int& BossResNo)
{
	GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
	CMonsterDef const *pDef = kMonsterDefMgr.GetDef(MonID);
	if( !pDef || 0 == pDef->NameNo() || pDef->GetAbil(AT_DONT_DISPLAY_ALL))
	{
		return false;
	}

	const wchar_t *pName = NULL;
	GetDefString(pDef->NameNo(), pName);
	if( IsElite )
	{
		BossResNo = pDef->GetAbil(AT_DEF_RES_NO);
	}

	int const iRes = pDef->GetAbil(AT_NAMENO);

	PgPilotMan::stClassInfo kClassInfo;
	g_kPilotMan.GetClassInfo(iRes, 0, kClassInfo);
	char const* pcPath = PgXmlLoader::GetPathByID(kClassInfo.m_kActorID.c_str());

	if ( !pcPath || !pName )
	{
		return false;
	}

	kInfo.kName = pName;
	kInfo.MonID = MonID;
	kInfo.kInfo.iResNo = iRes;
	kInfo.kInfo.strPath = UNI(pcPath);

	return PgWorldMapPopUpUI::FindMonsterFromXMLPath(kInfo.kInfo.kRscIcon, kInfo.kInfo.kIconImage, kInfo.kInfo.strPath);
}

void lwUIBook::SetFieldPage(lwUIWnd kWnd, int const iMapNo)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	pkWnd->Visible(true);
	std::wstring wstrMapImage;	//마을은 필요 없음
	if (g_kWorldMapUI.GetImgFromShardId(wstrMapImage, BM::vstring(iMapNo)))
	{
		XUI::CXUI_Wnd* pkImg = pkWnd->GetControl(L"IMG_MAP");
		if(pkImg)
		{
			pkImg->DefaultImgName(wstrMapImage);
		}
	}

	pkWnd->Text(GetMapName(iMapNo));
	pkWnd->SetInvalidate();

	CONT_DEFMAP const * pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);	//맵 데프
	CONT_DEFMAP::const_iterator map_it = pContDefMap->find(iMapNo);	//맵번호로 맵 데프에서 찾아서
	if (pContDefMap->end() == map_it)
	{
		return;
	}

	TBL_DEF_MAP const& rkDefMap = (*map_it).second;

	std::wstring kDescr;
	g_kWorldMapUI.GetMapDiscriptionID(iMapNo, kDescr, rkDefMap.strXMLPath);
	
	XUI::CXUI_Wnd* pkDescr = pkWnd->GetControl(L"FRM_MAP_COMMENT");
	if(pkDescr)
	{
		pkDescr->Text(kDescr);
	}

	// 리소스 테이블 가져오기.
	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);
	GET_DEF(PgMonsterBag, kMonsterBag);
	GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
	const CONT_DEF_MISSION_ROOT* pContDefRoot = NULL;
	g_kTblDataMgr.GetContDef(pContDefRoot);
	const CONT_DEF_MISSION_CANDIDATE* pContDefCanidate = NULL;
	g_kTblDataMgr.GetContDef(pContDefCanidate);

	CONT_MISSION_NO::const_iterator msn_it = rkDefMap.kContMissionNo.begin();	//미션맵이 있으면
	CONT_SET_DATA kLoadedMap;	//후보맵들이 중복될 수 있기 때문에
	kBookMonsterInfo.Clear();

	while(rkDefMap.kContMissionNo.end() != msn_it)
	{
		CONT_DEF_MISSION_ROOT::const_iterator root_it = pContDefRoot->find((*msn_it));
		if (pContDefRoot->end() != root_it)
		{
			TBL_DEF_MISSION_ROOT const& rkRoot = (*root_it).second;
			for (int i = 0; i < MAX_MISSION_LEVEL; ++i)	//미션 레벨당
			{
				int const iLev = rkRoot.aiLevel[i];

				if(0<iLev)
				{
					CONT_DEF_MISSION_CANDIDATE::const_iterator cand_it = pContDefCanidate->find(iLev);	//매션맵 후보들을
					if (pContDefCanidate->end() != cand_it)
					{
						TBL_DEF_MISSION_CANDIDATE const& rkCand = (*cand_it).second;
						for (int k = 0; k < MAX_MISSION_CANDIDATE; ++k)
						{
							int const iCandi = rkCand.aiCandidate[k];
							CONT_SET_DATA::const_iterator loaded_it = kLoadedMap.find(iCandi);
							if (0<iCandi && kLoadedMap.end() == loaded_it)	//0 이상이고 한번도 찾은적이 없다면
							{
								kLoadedMap.insert(iCandi);
								SMonsterControl kGetMon;
								if (kMonsterBag.GetMonster(iCandi, kGetMon))	//몬스터를 찾자
								{
									PgWorldMapPopUpUI::GetMonsterList(kGetMon, kMonsterDefMgr, kBookMonsterInfo);
								}
							}
						}

						int const iCandiEnd = rkCand.iCandidate_End;	//마지막 미션맵은 따로 관리
						CONT_SET_DATA::const_iterator loaded_it = kLoadedMap.find(iCandiEnd);
						if (0<iCandiEnd && kLoadedMap.end() == loaded_it)//0 이상이고 한번도 찾은적이 없다면
						{
							kLoadedMap.insert(iCandiEnd);
							SMonsterControl kGetMon;
							if (kMonsterBag.GetMonster(iCandiEnd, kGetMon))
							{
								PgWorldMapPopUpUI::GetMonsterList(kGetMon, kMonsterDefMgr, kBookMonsterInfo);
							}
						}

					}
				}
			}
		}
		++msn_it;
	}

	//이름번호, 리소스 번호로 중복된 애들 제거
	ContMonDistinct kOrderedMonsterSet;
	kBookMonsterInfoUI.clear();
	if(rkDefMap.kContMissionNo.empty())	//미션이 없는 맵이면 필드몬스터들을 출력해 주자
	{
		SMonsterControl kGetFieldMon;
		kLoadedMap.insert(iMapNo);
		if(kMonsterBag.GetMonster(iMapNo, kGetFieldMon))
		{
			PgWorldMapPopUpUI::GetMonsterList(kGetFieldMon, kMonsterDefMgr, kBookMonsterInfo);
			CONT_SET_DATA::const_iterator mon_it = kBookMonsterInfo.kSetData.begin();
			while(mon_it != kBookMonsterInfo.kSetData.end())
			{
				SBookMonsterInfo kInfo;
				int iTemp = 0;
				if( GetMonsterBookInfo((*mon_it), kInfo, false, iTemp) )	//중복없다고 전제
				{
					ContMonDistinct::const_iterator order_it = kOrderedMonsterSet.find(kInfo.kName);
					if (order_it==kOrderedMonsterSet.end())	//새로운놈
					{
						kOrderedMonsterSet.insert(std::make_pair(kInfo.kName, kInfo.kInfo.strPath));
						kBookMonsterInfoUI.push_back(kInfo);
					}
				}
				++mon_it;
			}
		}
	}

	CONT_SET_DATA::iterator elitemon_it = kBookMonsterInfo.kSetElite.begin();
	for(int i = 0; i < MAX_MISSION_LEVEL; ++i)
	{
		if(kBookMonsterInfo.kSetMission[i].empty())
		{
			continue;
		}

		CONT_SET_DATA::const_iterator mon_it = kBookMonsterInfo.kSetMission[i].begin();
		while(mon_it != kBookMonsterInfo.kSetMission[i].end())
		{
			SBookMonsterInfo kInfo;
			int iTemp = 0;
			if( GetMonsterBookInfo((*mon_it), kInfo, false, iTemp) )
			{
				ContMonDistinct::const_iterator order_it = kOrderedMonsterSet.find(kInfo.kName);
				if (order_it==kOrderedMonsterSet.end())	//새로운놈
				{
					kOrderedMonsterSet.insert(std::make_pair(kInfo.kName, kInfo.kInfo.strPath));
					kBookMonsterInfoUI.push_back(kInfo);
				}
			}
			++mon_it;
		}
	}

	bool const bExistElite = !kBookMonsterInfo.kSetElite.empty();

	XUI::CXUI_Wnd* pBoss = pkWnd->GetControl(L"FRM_BOSS");
	if( pBoss )
	{
		pBoss->Visible(bExistElite);
		if( pBoss->Visible() )
		{
			SBookMonsterInfo kInfo;
			int iResNo = 0;
			if( GetMonsterBookInfo((*kBookMonsterInfo.kSetElite.rbegin()), kInfo, true, iResNo) )
			{
				int const MonID = *kBookMonsterInfo.kSetElite.rbegin();
				pBoss->SetCustomData(&MonID, sizeof(MonID));
				XUI::CXUI_Wnd* pBossWnd = pBoss->GetControl(L"ICN_BOSS");
				if( pBossWnd )
				{//
					pBossWnd->SetCustomData(&iResNo, sizeof(iResNo));
				}

				XUI::CXUI_Wnd* pBossName = pBoss->GetControl(L"FRM_BOSS_NAME");
				if( pBossName )
				{
					pBossName->Text(kInfo.kName);
					pBossName->SetCustomData(&MonID, sizeof(MonID));
				}
			}
		}
	}

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LST_MONSTER"));
	if( !pList )
	{
		return;
	}

	int iMaxItem = kBookMonsterInfoUI.size() * 0.5f + (kBookMonsterInfoUI.size() % 2);
	XUIListUtil::SetMaxItemCount(pList, iMaxItem);

	int MonsterCount = 0;
	XUI::SListItem* pItem = pList->FirstItem();
	ContMonDistinctVec::iterator	f_iter = kBookMonsterInfoUI.begin();
	while( f_iter != kBookMonsterInfoUI.end() )
	{
		XUI::CXUI_Wnd* pItemWnd = NULL;
		if( pItem ){ pItemWnd = pItem->m_pWnd; }
		if( !pItemWnd )
		{
			break;
		}
		if( 0 == MonsterCount % 2 )
		{
			ClearBookActorSlot(pItemWnd);
		}

		BM::vstring vStr(L"FRM_MON");
		vStr += (MonsterCount % 2);
		XUI::CXUI_Wnd*	pName = pItemWnd->GetControl(vStr);
		if( pName )
		{
			pName->Text(f_iter->kName);
			pName->SetCustomData(&f_iter->MonID, sizeof(f_iter->MonID));
			XUI::CXUI_Icon*	pIcon = dynamic_cast<XUI::CXUI_Icon*>(pName->GetControl(L"ICN_ICON"));
			if( pIcon )
			{
				bool const bIsView = SetIconInfo(pIcon, f_iter->kInfo.kRscIcon, f_iter->kInfo.kIconImage, f_iter->kInfo.iResNo);
				pIcon->Visible(bIsView);
			}
			XUI::CXUI_Wnd* pCheckImg = pName->GetControl(L"IMG_CHECK");
			if( pCheckImg )
			{
				pCheckImg->UVUpdate(1);
			}
		}

		if( 1 == MonsterCount % 2 )
		{
			pItem = pList->NextItem(pItem);
		}

		++MonsterCount;
		++f_iter;
	}
}

void lwUIBook::ClearBookActorSlot(XUI::CXUI_Wnd* pWnd)
{
	int const MAX_ITEM_SLOT_COUNT = 2;
	for(int i = 0; i < MAX_ITEM_SLOT_COUNT; ++i)
	{
		BM::vstring vStr(L"FRM_MON");
		vStr += i;
		XUI::CXUI_Wnd*	pName = pWnd->GetControl(vStr);
		if( pName )
		{
			pName->Text(L"");
			XUI::CXUI_Icon*	pIcon = dynamic_cast<XUI::CXUI_Icon*>(pName->GetControl(L"ICN_ICON"));
			if( pIcon )
			{
				pIcon->SetIconInfo(SIconInfo());
				pIcon->Visible(false);
			}
		}
	}
}

void lwUIBook::SetClearBestItem(lwUIWnd kWnd)
{
	if( kWnd.IsNil() ){ return; }

	int const MAX_EQUIP_ICON_SLOT = 16;

	for(int i = 0; i < MAX_EQUIP_ICON_SLOT; ++i)
	{
		BM::vstring	vStr(_T("FRM_EQUIP"));
		vStr += i;

		XUI::CXUI_Wnd* pkRecWnd = kWnd.GetSelf()->GetControl(vStr);
		if( !pkRecWnd )
		{
			continue;
		}
		
		XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pkRecWnd->GetControl(L"ICN_EQUIP"));
		if( pIcon )
		{
			pIcon->ClearCustomData();
		}
	}
}

void lwUIBook::CallBookItemToolTip(lwUIWnd kWnd, int const iType, lwPoint2& rPT)
{
	PgBase_Item	kItem;
	kItem.ItemNo( kWnd.GetCustomData<int>() );
	SEnchantInfo kEnchantInfo;
	switch(iType)
	{
	case IG_NORMAL:
		{
		}break;
	case IG_RARE:
		{
			kEnchantInfo.Field_1(3764387898);
			kEnchantInfo.Field_2(0);
		}break;
	case IG_UNIQUE:
		{
			kEnchantInfo.Field_1(15421621141588);
			kEnchantInfo.Field_2(0);
		}break;
	case IG_ARTIFACT:
		{
			kEnchantInfo.Field_1(116569709477978);
			kEnchantInfo.Field_2(1795);
		}break;
	case IG_LEGEND:
		{
			kEnchantInfo.Field_1(18166152591048804);
			kEnchantInfo.Field_2(1795);			
		}break;
	case 5://무기 인챈트 정보
		{
			kEnchantInfo.Field_1(132096);
			kEnchantInfo.Field_2(0);			
		}break;
	case 6://방어구 인챈트 정보
		{
			kEnchantInfo.Field_1(133120);
			kEnchantInfo.Field_2(0);			
		}break;
	default:
		{
			return;
		}break;
	}
	kItem.EnchantInfo( kEnchantInfo );
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if(pDef && 0==kItem.Count())
	{
		kItem.Count(pDef->MaxAmount());
	}
	CallToolTip_SItem(&kItem, rPT);
}

void lwUIBook::CallActorInfo(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	SBBSAVEACTORINFO kSaveActorInfo;
	if( !UISelf.GetCustomDataSize() )
	{
		return;
	}
	kSaveActorInfo.ReadFromPacket((*UISelf.GetCustomDataAsPacket()()));

	std::wstring kName = kSaveActorInfo.kName;
	std::wstring kDescription;
	std::string kActorID;
	int iActionID = 1000000;
	switch( kSaveActorInfo.Type )
	{
	case ACTOR_NPC:
		{
			SNpcCreateInfo kInfo;
			if(!g_kWorldMapUI.GetNpc(UISelf.GetOwnerGuid().GetGUID(), kInfo))
			{
				return;
			}
			kDescription = TTW(kInfo.iDescriptionID);
			kActorID = MB(kInfo.wstrActor);
			iActionID = 1000001;
		}break;
	case ACTOR_MONSTER:
	case ACTOR_ELITE:
		{
			GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
			CMonsterDef const *pDef = kMonsterDefMgr.GetDef(kSaveActorInfo.iMonID);
			if( !pDef )
			{
				return;
			}

			int const iRes = pDef->GetAbil(AT_NAMENO);

			PgPilotMan::stClassInfo kClassInfo;
			g_kPilotMan.GetClassInfo(iRes, 0, kClassInfo);

			CONT_BOOK_MAPNO const& kMapCont = g_kBookActorCameraMgr.GetMonsterContants(kSaveActorInfo.iMonID);
			CONT_BOOK_MAPNO::const_iterator c_iter = kMapCont.begin();
			while( c_iter != kMapCont.end() )
			{
				if( !kDescription.empty() )
				{
					kDescription += L"\n";
				}
				kDescription += GetMapName((*c_iter));
				kDescription += L"";
				++c_iter;
			}
			kActorID = kClassInfo.m_kActorID.c_str();
		}break;
	}

	XUI::CXUI_Wnd* pActorInfoUI = XUIMgr.Call(L"FRM_ACTOR_MODEL_VIEW", true);
	if( !pActorInfoUI )
	{
		return;
	}

	PgUIModel *pkModel = g_kUIScene.FindUIModel("BookPreview");
	if(pkModel)
	{
		pkModel->Clear();
		pkModel->AddActorByID(MB(kName), kActorID.c_str());
		pkModel->SetTargetAnimation(MB(kName), iActionID);

		SBookCameraInfo kInfo;
		if( !g_kBookActorCameraMgr.GetActorCameraInfo(kActorID, kInfo) )
		{
			kInfo = g_kBookActorCameraMgr.GetDefaultInfo();
		}

		NiCamera* pCamera = pkModel->GetCamera();
		if( pCamera )
		{
			NiPoint3 ActorPos;
			NiActorManager* pMgr = pkModel->GetActor(MB(kName));
			if( pMgr )
			{
				ActorPos = pMgr->GetActorRoot()->GetWorldTranslate();
				ActorPos.z += kInfo.Pos.z;
			}
			pCamera->SetTranslate(kInfo.Pos);
			pCamera->Update(0.0f);
			pCamera->LookAtWorldPoint(ActorPos, NiPoint3::UNIT_Z);
			pCamera->Update(0.0f);
		}
		pkModel->SetCameraZoomMinMax(kInfo.ZoomMin, kInfo.ZoomMax);
		pkModel->SetOrthoZoomOld((kInfo.ZoomMin + kInfo.ZoomMax) * 0.5f);
	}

	XUI::CXUI_Wnd* pName = pActorInfoUI->GetControl(L"FRM_NAME");
	XUI::CXUI_List* pContants = dynamic_cast<XUI::CXUI_List*>(pActorInfoUI->GetControl(L"LST_CONTENT"));
	if( !pName || !pContants )
	{
		return;
	}

	std::wstring const kENTER(L"\n");
	std::string::size_type index = kName.find(kENTER);
	if( std::string::npos != index )
	{
		kName.replace(index, kENTER.size(), L" ");
	}

	pName->Text(kName);

	XUI::SListItem*	pItem = NULL;

	if( !pContants->GetTotalItemCount() )
	{
		pItem = pContants->AddItem(L"");
	}
	else
	{
		pItem = pContants->FirstItem();
	}

	if( pItem && pItem->m_pWnd )
	{
		pItem->m_pWnd->Text(kDescription);
		XUI::CXUI_Style_String kStyleStr = pItem->m_pWnd->StyleText();
		POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleStr));
		kTextSize.x = pItem->m_pWnd->Size().x;
		kTextSize.y = (kTextSize.y < pContants->Size().y)?(pContants->Size().y):(kTextSize.y);
		pItem->m_pWnd->Size(kTextSize);
	}
}

void lwUIBook::ClickActorSlot(lwUIWnd UISelf, int const Type)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf ){ return; }

	XUI::CXUI_Wnd* pThisPage = NULL;
	XUI::CXUI_Wnd* pRightPage = NULL;
	if( Type == ACTOR_ELITE )
	{
		pThisPage = pSelf->Parent();
		if( !pThisPage ){ return; }

		pRightPage = pThisPage->Parent();
		if( !pRightPage ){ return; }

		pSelf = pSelf->GetControl(L"FRM_BOSS_NAME");
		if( !pSelf ){ return; }
	}
	else
	{
		pThisPage = pSelf->Parent()->Parent()->Parent();
		if( !pThisPage ){ return; }

		pRightPage = pThisPage->Parent();
		if( !pRightPage ){ return; }
	}

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pThisPage->GetControl((ACTOR_NPC == Type)?(L"LST_NPC"):(L"LST_MONSTER")));
	if( !pList ){ return; }

	XUI::SListItem* pListItem = pList->FirstItem();
	while( pListItem && pListItem->m_pWnd )
	{
		for(int i = 0; i < 2; ++i)
		{
			BM::vstring vStr;
			switch( Type )
			{
			case ACTOR_NPC:		{ vStr = L"FRM_NPC"; }break;
			case ACTOR_MONSTER://break없다
			case ACTOR_ELITE:	{ vStr = L"FRM_MON"; }break;
			default:{}break;
			}
			vStr += i;
			XUI::CXUI_Wnd* pCheckImg = pListItem->m_pWnd->GetControl(vStr);
			if( pCheckImg )
			{
				pCheckImg = pCheckImg->GetControl(L"IMG_CHECK");
				if( pCheckImg )
				{
					pCheckImg->UVUpdate(1);
				}
			}
		}
		pListItem = pList->NextItem(pListItem);
	}
	XUI::CXUI_Wnd* pCheckImg = pSelf->GetControl(L"IMG_CHECK");
	if( pCheckImg )
	{
		pCheckImg->UVUpdate(3);
	}

	SBBSAVEACTORINFO kSaveActorInfo;

	kSaveActorInfo.kName = pSelf->Text();
	kSaveActorInfo.Type = (EActorType)Type;
	kSaveActorInfo.iMonID = lwUIWnd(pSelf).GetCustomData<int>();

	BM::Stream	kPacket;
	kSaveActorInfo.WriteFormPacket(kPacket);

	XUI::CXUI_Wnd* pButton = pRightPage->GetControl(L"BTN_PREVIEW");
	if( !pButton ){ return; }

	if( kSaveActorInfo.Type == ACTOR_NPC )
	{
		lwUIWnd(pButton).SetOwnerGuid(UISelf.GetCustomDataAsGuid());
	}
	lwUIWnd(pButton).SetCustomDataAsPacket(lwPacket(&kPacket));
}

void lwUIBook::ClickSearchMyLoc(lwUIWnd UISelf)
{
	if( UISelf.IsNil() ){ return; }

	lwUIWnd UIParent = UISelf.GetParent();
	if( UIParent.IsNil() ){ return; }

	lwUIWnd UIMain = UIParent.GetParent();
	if( UIMain.IsNil() ){ return; }

	lwUIWnd kMapInfoPage = UIMain.GetControl("IMG_LEFT_PAGE");
	if( kMapInfoPage.IsNil() ){ return; }

	if( g_pkWorld )
	{
		lua_tinker::call<void, lwUIWnd, int, int>("UI_Set_Book_MapNavi", kMapInfoPage, g_pkWorld->MapNo(), 0);
	}
}

void lwUIBook::ClickSearchNpcLoc(lwUIWnd UISelf)
{
	if( UISelf.IsNil() ){ return; }

	lwUIWnd UIParent = UISelf.GetParent();
	if( UIParent.IsNil() ){ return; }

	lwUIWnd UIMain = UIParent.GetParent();
	if( UIMain.IsNil() ){ return; }

	lwUIWnd kMapInfoPage = UIMain.GetControl("IMG_LEFT_PAGE");
	if( kMapInfoPage.IsNil() ){ return; }

	int const iQuestID = UIParent.GetCustomData<int>();
	if( iQuestID == 0 ){ return; }

	PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
	if( !pkQuestInfo || pkQuestInfo->m_kNpc_Client.empty() )
	{
		return;
	}

	SQuestNpc const& rkNPC = (*pkQuestInfo->m_kNpc_Client.begin());
	std::vector< int > kMapArray;
	if( !g_kWorldMapUI.GetNpcGuidToMapNo(rkNPC.kNpcGuid, kMapArray) )
	{
		return;
	}

	lwPacket kPacket = UISelf.GetCustomDataAsPacket();
	int const PrevQuestID = kPacket.PopInt();
	int const PrevMapNo = kPacket.PopInt();

	int iNowMapNo = 0;
	if( kMapArray.size() == 1 )
	{
		iNowMapNo = (*kMapArray.begin());
		if( !lwIsOpenGroundByNo(iNowMapNo) )
		{
			iNowMapNo = 0;
		}
	}
	else
	{
		std::vector< int >::iterator	map_it = kMapArray.begin();
		while( map_it != kMapArray.end() )
		{
			std::vector< int >::value_type const& kMapNo = (*map_it);
			++map_it;
			if( lwIsOpenGroundByNo(kMapNo) )
			{
				if( PrevQuestID == iQuestID )
				{
					if( PrevMapNo == kMapNo )
					{
						while( true )
						{
							if( map_it == kMapArray.end() )
							{
								map_it = kMapArray.begin();
							}

							std::vector< int >::value_type const& NextMapNo = (*map_it);
							++map_it;
							if( lwIsOpenGroundByNo(NextMapNo) )
							{
								iNowMapNo = NextMapNo;
								break;
							}
						}
						break;
					}
				}
				else
				{
					iNowMapNo = kMapNo;
					break;
				}
			}
		}
	}

	if( !iNowMapNo )
	{
		lwAddWarnDataTT(3077);
		return; 
	}

	BM::Stream kNewPacket;
	kNewPacket.Push(iQuestID);
	kNewPacket.Push(iNowMapNo);
	UISelf.ClearCustomData();
	UISelf.SetCustomDataAsPacket(lwPacket(&kNewPacket));

	lua_tinker::call<void, lwUIWnd, int, int>("UI_Set_Book_MapNavi", kMapInfoPage, iNowMapNo, 0);
}

void lwUIBook::OnClickQuestListTypeChange(lwUIWnd UIParent)
{
	XUI::CXUI_Wnd* pParent = UIParent.GetSelf();
	if( !pParent )
	{
		return;
	}

	XUI::CXUI_Wnd* pkNormalList = pParent->GetControl(L"LST_QUEST");
	XUI::CXUI_Wnd* pkImportanceList = pParent->GetControl(L"LST_IMPORTANCE_QUEST");
	if( !pkNormalList && !pkNormalList )
	{
		return;
	}

	pkNormalList->Visible(!pkNormalList->Visible());
	pkImportanceList->Visible(!pkNormalList->Visible());
}

void lwUIBook::OnClickBookRankPage(lwUIWnd kWnd)
{
	BM::Stream kPacket(PT_C_M_REQ_ACHIEVEMENT_RANK);
	NETWORK_SEND(kPacket);
}

void lwUIBook::OnCallDetailAchievementView()
{
	XUI::CXUI_Wnd* pkBookUI = XUIMgr.Call(L"FRM_BOOK1");
	if( !pkBookUI )
	{
		return;
	}

	XUI::CXUI_Wnd* pkAchievePage = pkBookUI->GetControl(L"CBTN_TAB1");
	if( !pkAchievePage )
	{
		return;
	}

	lua_tinker::call<void, lwUIWnd>("UI_Click_Book_Tab", lwUIWnd(pkAchievePage));
}

int GetRecommandFrmBuildIndex(EEquipPos const kPos, bool & bIsNextPos)
{
	int iNo = -1;
	switch( kPos )
	{
	case EQUIP_POS_HELMET:	{ iNo = 0; }break;
	case EQUIP_POS_SHIRTS:	{ iNo = 1; }break;
	case EQUIP_POS_GLOVE:	{ iNo = 2; }break;
	case EQUIP_POS_GLASS:	{ iNo = 3; }break;
	case EQUIP_POS_CLOAK:	{ iNo = 4; }break;
	case EQUIP_POS_NECKLACE:{ iNo = 5; }break;
	case EQUIP_POS_EARRING:	{ iNo = 6; }break;
	case EQUIP_POS_SHOULDER:{ iNo = 8; }break;
	case EQUIP_POS_PANTS:	{ iNo = 9; }break;
	case EQUIP_POS_BOOTS:	{ iNo = 10; }break;
	case EQUIP_POS_SHEILD:
	case EQUIP_POS_KICKBALL:{ iNo = 11; }break;
	case EQUIP_POS_WEAPON:	{ iNo = 12; }break;
	case EQUIP_POS_BELT:	{ iNo = 13; }break;
	case EQUIP_POS_MEDAL:	{ iNo = 14; }break;
	case EQUIP_POS_RING:
		{
			iNo = (!bIsNextPos)?(7):(15);
			bIsNextPos = true;
		}break;
	}
	return iNo;
}

void lwUIBook::OnCloseBookUI()
{
	iForceRecommandItemNo = 0;
}

int lwUIBook::iForceRecommandItemNo = 0;
void lwUIBook::OnClickSetBestItem(lwUIWnd kWnd)
{
	if( kWnd.IsNil() ){ return; }

	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer ){	return; }

	PgInventory* pkInv = pPlayer->GetInven();
	if( !pkInv ){ return; }

	GET_DEF(PgDefRecommendationItemMgr, kRecommendationItemMgr);

	CONT_ITEMNO kContItemNo;
	if( 0 < iForceRecommandItemNo )
	{
		if( kRecommendationItemMgr.GetRecommendationItemByItemNo(pPlayer->GetAbil(AT_CLASS), pPlayer->GetAbil(AT_LEVEL),iForceRecommandItemNo,kContItemNo) )
		{
			OnClickSetBestItemUpdateUI(pkInv, kWnd, kContItemNo);
		}

		RefreshMyEquipItemUI(pkInv, kWnd);
	}
	else
	{
		if( kRecommendationItemMgr.GetRecommendationItem(pPlayer->GetAbil(AT_CLASS), pPlayer->GetAbil(AT_LEVEL),kContItemNo) )
		{
			OnClickSetBestItemUpdateUI(pkInv, kWnd, kContItemNo);
		}

		RefreshMyEquipItemUI(pkInv, kWnd);
	}
}

void lwUIBook::RefreshMyEquipItemUI(PgInventory* pkInv, lwUIWnd kWnd)
{
	bool bIsNextPos = false;
	int iNo = 0;
	for(int i=0; i<EQUIP_POS_MAX; ++i)
	{
		iNo = GetRecommandFrmBuildIndex(static_cast<EEquipPos>(i), bIsNextPos);
		if(-1 == iNo)
		{
			continue;
		}

		BM::vstring	vStr(_T("FRM_MY_EQUIP"));
		vStr += iNo;
		XUI::CXUI_Wnd* pkMyWnd = kWnd.GetSelf()->GetControl(vStr);
		if( !pkMyWnd )
		{
			continue;
		}

		XUI::CXUI_Wnd* pkBg = pkMyWnd->GetControl(L"IMG_BG");
		XUI::CXUI_Icon* pEquip = dynamic_cast<XUI::CXUI_Icon*>(pkMyWnd->GetControl(L"ICN_EQUIP"));
		if( !pkBg || !pEquip )
		{
			continue;
		}

		PgBase_Item kEqupItem;
		if(S_OK == pkInv->GetItem(SItemPos(KUIG_FIT, i), kEqupItem))
		{
			pEquip->Visible(true);
			pEquip->SetCustomData(&kEqupItem.ItemNo(), sizeof(kEqupItem.ItemNo()));
		}
		else
		{
			pEquip->Visible(false);
		}
	}
}

void lwUIBook::OnClickSetBestItemUpdateUI(PgInventory* pkInv, lwUIWnd kWnd, CONT_ITEMNO const& kContItemNo)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
		
	bool bIsNextPos = false;
	int iNo = 0;
	for(CONT_ITEMNO::const_iterator iter=kContItemNo.begin(); iter!=kContItemNo.end(); ++iter)
	{
		CItemDef const *pDef = kItemDefMgr.GetDef(*iter);
		if( !pDef )
		{
			continue;
		}

		iNo = GetRecommandFrmBuildIndex(static_cast<EEquipPos>(pDef->EquipPos()), bIsNextPos);
		if(-1 == iNo)
		{
			continue;
		}

		BM::vstring	vStr(_T("FRM_EQUIP"));
		vStr += iNo;

		XUI::CXUI_Wnd* pkRecWnd = kWnd.GetSelf()->GetControl(vStr);
		if( !pkRecWnd )
		{
			continue;
		}

		XUI::CXUI_Wnd* pkBg = pkRecWnd->GetControl(L"IMG_BG");
		XUI::CXUI_Icon* pEquip = dynamic_cast<XUI::CXUI_Icon*>(pkRecWnd->GetControl(L"ICN_EQUIP"));
		XUI::CXUI_Wnd* pkMark = pkRecWnd->GetControl(L"IMG_EQUIP_MARK");
		if( !pkBg || !pEquip || !pkMark )
		{
			continue;
		}

		SItemPos kTemp;
		pEquip->GrayScale((S_OK != pkInv->GetFirstItem((*iter), kTemp)));
		pEquip->SetCustomData(&(*iter), sizeof((*iter)));

		pEquip->GrayScale() ? pkBg->Visible(false) : pkBg->Visible(true);
		pkMark->Visible(false);
		pkMark->Enable(false);

		PgBase_Item kEqupItem;
		if(S_OK == pkInv->GetItem(SItemPos(KUIG_FIT, pDef->EquipPos()), kEqupItem))
		{
			if(kEqupItem.ItemNo() == *iter)
			{
				pkMark->Visible(true);
				pkMark->Enable(true);
			}
		}
	}
}

void lwUIBook::SetTownPage(lwUIWnd kWnd, int const iMapNo)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	pkWnd->Text(GetMapName(iMapNo));
	pkWnd->SetInvalidate();
	pkWnd->Visible(true);

	std::wstring wstrMapImage;	//마을은 필요 없음
	if (g_kWorldMapUI.GetImgFromShardId(wstrMapImage, BM::vstring(iMapNo)))
	{
		XUI::CXUI_Wnd* pkImg = pkWnd->GetControl(L"IMG_MAP");
		if(pkImg)
		{
			pkImg->DefaultImgName(wstrMapImage);
		}
	}


	{
		CONT_DEFMAP const * pContDefMap = NULL;
		g_kTblDataMgr.GetContDef(pContDefMap);	//맵 데프
		CONT_DEFMAP::const_iterator map_it = pContDefMap->find(iMapNo);	//맵번호로 맵 데프에서 찾아서
		if (pContDefMap->end() == map_it)
		{
			return;
		}

		TBL_DEF_MAP const& rkDefMap = (*map_it).second;

		std::wstring kDescr;
		g_kWorldMapUI.GetMapDiscriptionID(iMapNo, kDescr, rkDefMap.strXMLPath);
		
		XUI::CXUI_Wnd* pkDescr = pkWnd->GetControl(L"FRM_MAP_COMMENT");
		if(pkDescr)
		{
			pkDescr->Text(kDescr);
		}
	}

	MapNpcSet kNpcSet;
	g_kWorldMapUI.GetNpcSet(iMapNo, kNpcSet);

	MapNpcSet::const_iterator npc_itr = kNpcSet.begin();
	int iReuseCount = 0;
	XUI::CXUI_Wnd* pkEle = NULL;
	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LST_NPC"));
	int const iNULL = 0;	//CustomData용
	if ( pkList )
	{
		pkList->ClearList();
		while(npc_itr != kNpcSet.end())
		{
			SNpcCreateInfo const& rkNpcInfo = (*npc_itr);

			SResourceIcon kRscIcon;
			SIconImage kIconImage;
			if (!PgWorldMapPopUpUI::FindNPCFromActorName(kRscIcon, kIconImage, rkNpcInfo.wstrActor))
			{
				++npc_itr;
				continue;
			}
			if (0==iReuseCount)	//하나에 두개씩 써야되
			{
				XUI::SListItem* pkItem = pkList->AddItem(L"");
				if(pkItem)
				{
					pkEle = pkItem->m_pWnd;
				}
			}

			BM::vstring kIconName(L"FRM_NPC");
			kIconName+=iReuseCount;
			XUI::CXUI_Wnd* pkName = NULL;
			if(pkEle)
			{
				pkName = pkEle->GetControl(kIconName);
			}
			if (pkName)
			{
				const std::wstring *pkNpcName = NULL;
				if (GetDefString(rkNpcInfo.iID, pkNpcName))
				{
					pkName->Text(*pkNpcName);
					pkName->SetCustomData(&rkNpcInfo.kGuid, sizeof(rkNpcInfo.kGuid));
				}
				else
				{
					pkName->Text(std::wstring(L""));
					pkName->SetCustomData(&iNULL, sizeof(iNULL));	//커스텀 데이터 초기화
				}
				SetIconInfo(pkName->GetControl(L"ICN_ICON"), kRscIcon, kIconImage, rkNpcInfo.iID);
				XUI::CXUI_Wnd* pCheckImg = pkName->GetControl(L"IMG_CHECK");
				if( pCheckImg )
				{
					pCheckImg->UVUpdate(1);
				}
			}
			if(0==iReuseCount)
			{
				++iReuseCount;
			}
			else
			{
				iReuseCount = 0;
			}
		
			++npc_itr;
		}
	}
}

void lwUIBook::SetBossPage(lwUIWnd kWnd, int const iMapNo)
{
	const CONT_DEFMAP* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);	//맵 데프
	CONT_DEFMAP::const_iterator map_it = pContDefMap->find(iMapNo);	//맵번호로 맵 데프에서 찾아서
	if (pContDefMap->end() == map_it)
	{
		return;
	}

	XUI::CXUI_Wnd* pkWnd = kWnd();
	pkWnd->Visible(true);

	GET_DEF(PgMonsterBag, kMonsterBag);
	GET_DEF(CMonsterDefMgr, kMonsterDefMgr);

	SMonsterControl kGetMon;
	if (kMonsterBag.GetMonster(iMapNo, kGetMon))	// 일반 필드몹 얻자
	{
		kBookMonsterInfo.Clear();
		PgWorldMapPopUpUI::GetMonsterList(kGetMon, kMonsterDefMgr, kBookMonsterInfo);
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LST_MONSTER"));
	if(pkList)
	{
		pkList->ClearList();
		int iCount = 0;
		XUI::CXUI_Wnd* pkEle = NULL;
		CONT_SET_DATA::iterator mon_it = kBookMonsterInfo.kSetData.begin();
		while(mon_it != kBookMonsterInfo.kSetData.end())
		{
			if(LAVALON != (*mon_it))
			{
				CMonsterDef const *pDef = kMonsterDefMgr.GetDef((*mon_it));
				if (pDef && EMGRADE_ELITE > pDef->GetAbil(AT_GRADE))
				{
					if (0==iCount)
					{
						XUI::SListItem* pkItem = pkList->AddItem(L"");
						if(pkItem)
						{
							pkEle = pkItem->m_pWnd;
						}
					}

					int iRes = pDef->GetAbil(AT_NAMENO);
					PgPilotMan::stClassInfo kClassInfo;
					g_kPilotMan.GetClassInfo(iRes,0,kClassInfo);
					char const* pcPath = PgXmlLoader::GetPathByID(kClassInfo.m_kActorID.c_str());
					if(pcPath)
					{
						BM::vstring kIconName(L"ICN_MONSTER");
						kIconName+=iCount;
						SResourceIcon kRscIcon;
						SIconImage kIconImage;
						if (PgWorldMapPopUpUI::FindMonsterFromXMLPath(kRscIcon, kIconImage, UNI(pcPath)))
						{
							SetIconInfo(pkEle->GetControl(kIconName), kRscIcon, kIconImage, iRes);
							++iCount;
						}
					}
				}
			}
			if(6<iCount)
			{
				iCount = 0;
				pkEle = NULL;
			}
			++mon_it;
		}
	}

	std::wstring kBossDiscr;
	XUI::CXUI_Wnd* pkBossName = pkWnd->GetControl(L"FRM_BOSS_NAME");
	if(pkBossName)
	{
		CONT_SET_DATA::iterator mon_it = kBookMonsterInfo.kSetElite.begin();
		if(kBookMonsterInfo.kSetElite.end() != mon_it)
		{
			CMonsterDef const *pDef = kMonsterDefMgr.GetDef((*mon_it));
			if(pDef)
			{
				int iRes = pDef->GetAbil(AT_NAMENO);
				if (LAVALON_HEALT == iRes)
				{
					iRes= LAVALON;	//라발론 심장 예외처리
				}
				
				const std::wstring *pkName = NULL;
				if (GetDefString(iRes, pkName))
				{
					pkBossName->Text(*pkName);
				}
				else
				{
					pkBossName->Text(std::wstring(L""));
				}

				int const iDescr = pDef->GetAbil(ATI_EXPLAINID);
				if(0<iDescr)
				{
					const wchar_t *pDescr = NULL;
					if(GetDefString(iDescr, pDescr) && pDescr)
					{
						kBossDiscr = pDescr;
					}
				}

				XUI::CXUI_Wnd* pkBossImg = pkWnd->GetControl(L"ICN_BOSS");
				if(pkBossImg)
				{
					int const iIconNo = pDef->GetAbil(AT_DEF_RES_NO);
					pkBossImg->SetCustomData(&iIconNo, sizeof(iIconNo));
				}
			}
		}
	}

	XUI::CXUI_Wnd* pkBossComm = pkWnd->GetControl(L"FRM_BOSS_COMMENT");
	if(pkBossComm)
	{
		pkBossComm->Text(kBossDiscr);
	}

	pkWnd->SetInvalidate();

}

void lwUIBook::SetQuestPage(lwUIWnd kWnd, int const iType)
{
	if(kWnd.IsNil() || 0>iType)
	{
		return;
	}

	switch(iType)
	{
	case 0:	//영웅
		{
			SetHeroQuestPage(kWnd);
		}break;
	case 1:	//일반
		{
			SetNormalQuestPage(kWnd);
		}break;
	}

	SetImportanceQuestList(kWnd);

	lwUIWnd UINpc = kWnd.GetControl("ICN_NPC");
	if( !UINpc.IsNil() )
	{
		UINpc.Visible(false);
	}

	lwUIWnd UIStartNpc = kWnd.GetControl("IMG_RIGHT_PAGE");
	if( !UIStartNpc.IsNil() )
	{
		UIStartNpc = UIStartNpc.GetControl("FRM_START_NPC");
		if( !UIStartNpc.IsNil() )
		{
			UIStartNpc.GetSelf()->Text(L"");
		}
	}
}

void lwUIBook::SetHeroQuestPage(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	if(NULL==pkWnd) {return;}

	ContQuestInfo kCont;
	g_kQuestMan.GetQuest(kCont);

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LST_QUEST"));
	if(NULL==pkList) {return;}

	pkList->ClearList();

	int iClass = 0;
	PgPilot *pkMyPilot = g_kPilotMan.GetPlayerPilot();
	if( pkMyPilot )
	{
		iClass = pkMyPilot->GetAbil(AT_CLASS);
	}

	const CONT_DEF_QUEST_REWARD* pContDefQuest = NULL;
	g_kTblDataMgr.GetContDef(pContDefQuest);	//퀘스트 데프
	if(!pContDefQuest) {return;}

	ContQuestInfo::const_iterator quest_it = kCont.begin();
	std::multimap< int, PgQuestInfo* > kOrderedCont;
	while(quest_it != kCont.end())
	{
		int iQNum = (*quest_it).first;
		ContQuestInfo::mapped_type const kValue = (*quest_it).second;
		if(kValue && kValue->m_kLimit.CheckClass(iClass))
		{
			CONT_DEF_QUEST_REWARD::const_iterator def_it = pContDefQuest->find((*quest_it).first);
			if(pContDefQuest->end() != def_it)
			{
				kOrderedCont.insert(std::make_pair((*def_it).second.iLevelMin, (*quest_it).second));
			}
		}
		++quest_it;
	}

	std::multimap< int, PgQuestInfo* >::const_iterator order_it = kOrderedCont.begin();
	while(order_it != kOrderedCont.end())
	{
		ContQuestInfo::mapped_type const kValue = (*order_it).second;
		if(kValue)
		{
			XUI::SListItem* pkItem = pkList->AddItem(L"");
			if (pkItem)
			{
				XUI::CXUI_Wnd* pkBtn = pkItem->m_pWnd->GetControl(L"BTN_QUEST");
				bool const bIsClear = lwIsEndedQuest(kValue->ID());
				int const iID = bIsClear ? kValue->ID() : 0;
				if (pkBtn)
				{
					pkBtn->Text(TTW(bIsClear ? kValue->m_iTitleTextNo : 3066));
					pkBtn->SetCustomData(&iID, sizeof(iID));
				}
				lwUIWnd(pkItem->m_pWnd->GetControl(L"IMG_MARK")).SetUVIndex( bIsClear ? 2 : 1 );
			}
		}
		++order_it;
	}
}

void lwUIBook::SetNormalQuestPage(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	if(NULL==pkWnd) {return;}

	ContQuestInfo kCont;
	g_kQuestMan.GetQuest(kCont, QT_None);

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LST_QUEST"));
	if(NULL==pkList) {return;}

	pkList->ClearList();

	PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return;
	}

	PgInventory* pkInv = pkPC->GetInven();
	if( !pkInv )
	{
		return;
	}

	int const iPCLevel = pkPC->GetAbil(AT_LEVEL);
	int const iPCClass = pkPC->GetAbil(AT_CLASS);

	PgMyQuest const *pkMyQuest = pkPC->GetMyQuest();
	if( !pkMyQuest )
	{
		return;
	}
	
	ContUserQuestState kIngList;

	//신규 퀘스트 목록을 생성;
	ContQuestInfo::iterator	quest_iter = kCont.begin();
	while( quest_iter != kCont.end() )
	{
		ContQuestInfo::key_type const& kQuestID = quest_iter->first;
		ContQuestInfo::mapped_type const& kQuestInfo = quest_iter->second;

		if( QBL_None != PgQuestInfoUtil::CheckBeginQuest(const_cast<PgPlayer const *>(pkPC), kQuestID, kQuestInfo ) )
		{
			++quest_iter;
			continue;
		}

		EQuestState eState = QS_Begin;
		switch( kQuestInfo->Type() )
		{
		case QT_Loop:
			{ 
				eState = QS_Begin_Loop;		
			} break;
		case QT_Event_Normal:
		case QT_Event_Loop:
		case QT_Event_Normal_Day:
			{ 
				eState = QS_Begin_Event;		
			} break;
		case QT_Scenario:		
			{ 
				eState = QS_Begin_Story;		
			} break;
		case QT_Day:
		case QT_GuildTactics:
		case QT_Normal_Day:
		case QT_Random:
		case QT_RandomTactics:
			{ 
				eState = QS_Begin_Tactics;	
			} break;
		case QT_Week:
			{
				eState = QS_Begin_Weekly;
			}break;
		case QT_Couple:
		case QT_SweetHeart:
			{
				eState = QS_Begin_Couple;
			}break;
		default:
			{
				if( PgQuestManUtil::IsInResetScheduleQuest(kQuestID) )
				{
					eState = QS_Begin_Weekly; 
				}
			} break;
		}
		
		kIngList.push_back(SUserQuestState(kQuestID, eState));
		++quest_iter;
	}

	pkMyQuest->GetQuestList(kIngList);

	std::sort(kIngList.begin(), kIngList.end(), lwUIBook::SortQuestPageListItem);

	ContUserQuestState::const_iterator quest_it = kIngList.begin();
	while(quest_it != kIngList.end())
	{
		SUserQuestState const& rkQuest = (*quest_it);
		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(rkQuest.iQuestID);
		if (pkQuestInfo /*&& pkQuestInfo->Type() != QT_Scenario*/)
		{
			XUI::SListItem* pkItem = pkList->AddItem(L"");
			if (pkItem)
			{
				int const iID = pkQuestInfo->ID();
				bool const bEndedQuest = pkMyQuest->IsEndedQuest( iID );
				XUI::CXUI_Wnd* pkBtn = pkItem->m_pWnd->GetControl(L"BTN_QUEST");
				if (pkBtn)
				{
					pkBtn->Text(TTW(pkQuestInfo->m_iTitleTextNo));
					pkBtn->SetCustomData(&iID, sizeof(iID));
				}

				XUI::CXUI_Wnd* pkMark = pkItem->m_pWnd->GetControl(L"IMG_MARK");
				if( pkMark )
				{
					SCustomQuestMarker kMarkerInfo;
					if( g_kQuestMan.CheckCustomQuestMarker(iID, kMarkerInfo) )
					{
						pkMark->UVUpdate(kMarkerInfo.iBookIconIDX);
					}
					else
					{
						pkMark->UVUpdate(GetQuestMarkUVIndex(static_cast<EQuestState>(rkQuest.byQuestState)));
					}
				}
			}
		}
		
		++quest_it;
	}
}

void lwUIBook::SetImportanceQuestList(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pParent = kWnd.GetSelf();
	if( !pParent )
	{
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{//내 퀘스트
		return;
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pParent->GetControl(L"LST_IMPORTANCE_QUEST"));
	if( !pkList )
	{
		return;
	}

	ContImportanceQuest const& kContList = g_kQuestMan.GetImportanceQuestList();
	if( kContList.empty() )
	{//중요 퀘스트 목록
		pkList->DeleteAllItem();
		return;
	}

	//리스트 갯수 미리추가
	XUIListUtil::SetMaxItemCount(pkList, kContList.size());

	XUI::SListItem* pkItem = pkList->FirstItem();
	ContImportanceQuest::const_iterator group_itor = kContList.begin();
	while( kContList.end() != group_itor )
	{//중요 퀘스트 세팅(그룹별 목록)
		ContImportanceQuest::mapped_type const& kImportanceQuest = (*group_itor).second;
		if( pkItem && pkItem->m_pWnd )
		{
			XUI::CXUI_Tree* pkTree = dynamic_cast<XUI::CXUI_Tree*>(pkItem->m_pWnd);
			if( pkTree )
			{
				pkTree->DeleteAllItem();

				int iCompleteCount = 0;
				int iBeginCount = 0;

				SImportanceQuestInfo::CONT_QUEST_ID::const_iterator quest_itor = kImportanceQuest.kQuestList.begin();
				while( kImportanceQuest.kQuestList.end() != quest_itor )
				{//등록된 퀘스트별
					SImportanceQuestInfo::CONT_QUEST_ID::value_type const& kQuestID = (*quest_itor);
					PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(kQuestID);
					if( pkQuestInfo )
					{
						bool IsEnded = pkMyQuest->IsEndedQuest(kQuestID);
						bool IsIng = pkMyQuest->IsIngQuest(kQuestID);
						bool bBegin = (PgQuestInfoUtil::CheckBeginQuest(const_cast<PgPlayer const *>(pkPlayer), kQuestID, pkQuestInfo) == QBL_None)?(true):(false);

						XUI::SListItem* pkQuestItem = pkTree->AddItem(L"");
						if( pkQuestItem && pkQuestItem->m_pWnd )
						{//트리 아이템 세팅(퀘스트 목록)
							pkQuestItem->m_pWnd->SetCustomData(&kQuestID, sizeof(kQuestID));
							pkQuestItem->m_pWnd->GrayScale(true);
							BM::vstring vStr(TTW(pkQuestInfo->m_iTitleTextNo));
							if( IsEnded )
							{
								vStr += TTW(40073);
								vStr += TTW(401029);
							}
							else if( IsIng )
							{
								vStr += TTW(40072);
								vStr += TTW(20008);
							}
							if( IsIng || IsEnded )
							{
								pkQuestItem->m_pWnd->GrayScale(false);
							}
							pkQuestItem->m_pWnd->Text(vStr);
							pkQuestItem->m_pWnd->FontColor((IsEnded || IsIng)?(0xFF4E1D0E):(0xFF433B2D));
							pkQuestItem->m_pWnd->SetCustomData(&kQuestID, sizeof(kQuestID));
						}

						//완료 목록과 시작 목록
						iCompleteCount += (IsEnded)?(1):(0);
						iBeginCount += (IsIng)?(1):(0);
					}
					++quest_itor;	
				}

				bool bIsComplete = false;
				//진행률 표시
				XUI::CXUI_AniBar* pkAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pkTree->GetControl(L"ANB_PERCENT"));
				if( pkAniBar )
				{
					int const iTotalQuestCount = kImportanceQuest.kQuestList.size();
					if( iTotalQuestCount )
					{//있을때만
						bIsComplete = (iCompleteCount == iTotalQuestCount)?(true):(false);

						pkAniBar->Now( iCompleteCount );
						pkAniBar->Max( iTotalQuestCount );
						BM::vstring vStr( iCompleteCount );
						vStr += L"{C=0xFFFFFFFF/}";
						vStr += L"/";
						vStr += iTotalQuestCount;
						pkAniBar->Text(vStr);
					}
					else
					{
						pkAniBar->Now(0);
						pkAniBar->Max(0);
						pkAniBar->Text(L"0/0");
					}
				}

				//그룹 이름 표시
				XUI::CXUI_Wnd* pkGTitle = pkTree->GetControl(L"IMG_SELECT");
				if( pkGTitle )
				{
					pkGTitle->UVUpdate(1);
					BM::vstring vStr(TTW(kImportanceQuest.iGroupNameNo));
					if( bIsComplete )
					{
						vStr += TTW(40073);
						vStr += TTW(401031);
					}
					else if( 0 != iBeginCount )
					{
						vStr += TTW(40072);
						vStr += TTW(20008);
					}
					pkGTitle->Text(vStr);
				}

				//확장
				XUI::CXUI_Wnd* pkGEx= pkTree->GetControl(L"IMG_EX");
				if( pkGEx )
				{
					pkGEx->UVUpdate(1);
				}
			}
			pkItem = pkList->NextItem(pkItem);
		}
		++group_itor;
	}
}

bool lwUIBook::SortQuestPageListItem(SUserQuestState const& rhs, SUserQuestState const lhs)
{
	int iDestRank = 0;
	int iSourRank = 0;

	switch( rhs.byQuestState )
	{
	case QS_Begin_Loop:		{ iDestRank = 5; }break;
	case QS_Begin_Story:	{ iDestRank = 1; }break;
	case QS_Begin_Tactics:	{ iDestRank = 3; }break;
	case QS_Begin_Weekly:	{ iDestRank = 2; }break;
	case QS_Ing:			{ iDestRank = 6; }break;
	case QS_Failed:			{ iDestRank = 7; }break;
	default:				{ iDestRank = 4; }break;
	}

	switch( lhs.byQuestState )
	{
	case QS_Begin_Loop:		{ iSourRank = 5; }break;
	case QS_Begin_Story:	{ iSourRank = 1; }break;
	case QS_Begin_Tactics:	{ iSourRank = 3; }break;
	case QS_Begin_Weekly:	{ iSourRank = 2; }break;
	case QS_Ing:			{ iSourRank = 6; }break;
	case QS_Failed:			{ iSourRank = 7; }break;
	default:				{ iSourRank = 4; }break;
	}

	if( iDestRank == iSourRank )
	{
		return ( rhs.iQuestID < lhs.iQuestID );
	}
	return ( iDestRank < iSourRank );
}

int lwUIBook::GetQuestMarkUVIndex(EQuestState const State)
{
	switch( State )
	{
	case QS_Begin_Story:	{ return 3; };
	case QS_End_Story:		{ return 4;	};
	case QS_Begin:			{ return 1;	};
	case QS_Ing:			{ return 10;};
	case QS_Begin_NYet:		{ return 9;	};
	case QS_End:			{ return 2;	};
	case QS_Begin_Loop:		{ return 7;	};
	case QS_End_Loop:		{ return 8;	};
	case QS_Begin_Tactics:	{ return 5;	};
	case QS_End_Tactics:	{ return 6;	};
	case QS_Begin_Weekly:	{ return 12;};
	case QS_End_Weekly:		{ return 13;};
	case QS_Begin_Couple:	{ return 14;};
	case QS_End_Couple:		{ return 15;};
	case QS_Begin_Event:	{ return 16;};
	case QS_End_Event:		{ return 17;};
	}

	return 11;
}

void lwUIBook::OnClickBookQuest(lwUIWnd kWnd)
{
	if(kWnd.IsNil()) {return;}

	int const iCustom = kWnd.GetCustomData<int>();
	PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iCustom);
	if (pkQuestInfo)
	{
		XUI::CXUI_Wnd* pkTabPage = kWnd()->Parent()->Parent()->Parent();
		pkTabPage->SetCustomData(&iCustom, sizeof(iCustom));
		XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkTabPage->GetControl(L"LST_QUEST_COMMENT"));
		if(pkList)
		{
			pkList->ClearList();
			XUI::SListItem* pkItem = pkList->AddItem(L"");
			if (pkItem)
			{
				SQuestDialog const* pkDial = NULL;
				if (pkQuestInfo->GetInfoDialog(pkDial))
				{
					if (!pkDial->kDialogText.empty())
					{
						std::wstring kText = TTW((*pkDial->kDialogText.begin()).iTextNo);
						Quest::lwQuestTextParser(kText);
						pkItem->m_pWnd->Text(kText);
						POINT2 kSize = lwUIWnd(pkItem->m_pWnd).GetTextSize()();
						pkItem->m_pWnd->Size(POINT2(pkItem->m_pWnd->Size().x, kSize.y));
					}
				}

				std::wstring kNPCName;

				XUI::CXUI_Wnd* pkIcon = pkTabPage->GetControl(L"ICN_NPC");
				if(pkIcon)
				{
					pkIcon->Visible(false);
					if (!pkQuestInfo->m_kNpc_Client.empty())
					{
						SQuestNpc const& rkNPC = (*pkQuestInfo->m_kNpc_Client.begin());
						SNpcCreateInfo kInfo;
						if (g_kWorldMapUI.GetNpc(rkNPC.kNpcGuid, kInfo))
						{
							SResourceIcon kRscIcon;
							SIconImage kIconImage;
							if (PgWorldMapPopUpUI::FindNPCFromActorName(kRscIcon, kIconImage, kInfo.wstrActor))
							{
								SetIconInfo(pkIcon, kRscIcon, kIconImage, kInfo.iID);
								pkIcon->Visible(true);
							}

							wchar_t const* pName = NULL;
							if (GetDefString(kInfo.iID, pName))
							{
								ParseNpcName(pName, kNPCName);
							}
						}
					}	
				}
				XUI::CXUI_Wnd* pkRight = kWnd()->Parent()->Parent()->Parent()->GetControl(L"IMG_RIGHT_PAGE");
				if(pkRight)
				{
					pkRight->SetCustomData(&iCustom, sizeof(iCustom));
					pkRight->Text(kWnd.GetStaticText().GetWString());
					Quest::SetCutedTextLimitLength(pkRight, kWnd.GetStaticText().GetWString(), _T("..."), 200);
					XUI::CXUI_Wnd* pkStartNpc = pkRight->GetControl(L"FRM_START_NPC");
					if( pkStartNpc )
					{
						pkStartNpc->Text( kNPCName.empty() ? L"" : (TTW(3078) + kNPCName) );
					}
					XUI::CXUI_Wnd *pkRemoteAccept = pkRight->GetControl(L"BTN_REMOTE_ACCEPT");
					if( pkRemoteAccept )
					{
						if( true == pkQuestInfo->IsCanRemoteAccept() )
						{
							pkRemoteAccept->Visible(true);
						}
						else
						{
							pkRemoteAccept->Visible(false);
						}
					}
				}
			}
			pkList->AdjustMiddleBtnPos();
		}
	}

	kWnd.SetInvalidate();
}

void lwUIBook::OnClickBookNPC(lwUIWnd kWnd)
{
	if(kWnd.IsNil()) {return;}

	BM::GUID const kCustom = kWnd.GetCustomDataAsGuid()();
	if (BM::GUID::IsNull(kCustom)) {return;}
	
	SNpcCreateInfo kInfo;
	if (!g_kWorldMapUI.GetNpc(kCustom, kInfo)) {return;}

	std::wstring kNpcName = kWnd.GetStaticText().GetWString();
	size_t const kPos = kNpcName.find(L"\n");
	if (kNpcName.size() > kPos)	//찾으면
	{
		kNpcName.replace(kPos, 1, L" ");	//개행문자 제거
	}
	
	kWnd.GetParent().GetParent().GetParent()()->Text(kNpcName);

	std::wstring kDescr;
	if (0<kInfo.iDescriptionID)
	{
		kDescr = TTW(kInfo.iDescriptionID);
	}
	lwUIWnd kParent = kWnd.GetParent();
	if(!kParent.IsNil())
	{
		lwUIWnd kPParent = kParent.GetParent();
		if (!kPParent.IsNil())
		{
			lwUIWnd kPPParent = kPParent.GetParent();
			if (!kPPParent.IsNil())
			{
				lwUIWnd kComment = kPPParent.GetControl("FRM_NPC_COMMENT");
				if(!kComment.IsNil())
				{
					kComment()->Text(kDescr);
				}

				lwUIWnd kIcon = kPPParent.GetControl("ICN_NPC0");
				if(!kIcon.IsNil())
				{
					SResourceIcon kRscIcon;
					SIconImage kIconImage;
					if (PgWorldMapPopUpUI::FindNPCFromActorName(kRscIcon, kIconImage, kInfo.wstrActor))
					{
						SetIconInfo(kIcon(), kRscIcon, kIconImage, kInfo.iID);
						kIcon.Visible(true);
					}
					else
					{
						kIcon.Visible(false);
					}
				}
			}
		}
	}
}

void lwUIBook::SetAchievePage(lwUIWnd kWnd, int const iType)
{
	if(kWnd.IsNil() || ACT_NONE == iType)
	{
		return;
	}

	XUI::CXUI_Wnd* pkEquipMedal = kWnd.GetControl("FRM_EQUIP_BOOK_MEDAL").GetSelf();
	if( !pkEquipMedal ){ return; }

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(kWnd.GetControl("LST_MEDAL").GetSelf());
	if(!pkList) {return;}

	pkList->ClearList();

	CONT_DEF_ACHIEVEMENTS const * pContDefAch = NULL;
	g_kTblDataMgr.GetContDef(pContDefAch);	//업적 데프
	
	CONT_DEF_ACHIEVEMENTS::const_iterator ach_it = pContDefAch->begin();

	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkMyPlayer) {return;}
	PgAchievements* pkAchiv = pkMyPlayer->GetAchievements();
	if(!pkAchiv) {return;}

	int iEquipAchieveNo = 0;
	{
		PgBase_Item kItem;
		if( PgActorUtil::GetEquipAchievementItem(g_kPilotMan.GetPlayerActor(), kItem) )
		{
			iEquipAchieveNo = PgActorUtil::GetAchievementsTitleNo(kItem);
		}
	}

	kAchieveSortCont	kSortResult;
	TBL_DEF_ACHIEVEMENTS const* pEquipAchieveDef = NULL;
	while(pContDefAch->end() != ach_it)
	{
		if( 0 != iEquipAchieveNo && iEquipAchieveNo == ach_it->second.iTitleNo )
		{
			pEquipAchieveDef = &ach_it->second;
		}

		if( iType == ach_it->second.iCategory && 0 < ach_it->second.iItemNo )
		{
			SAchieveSortKey	Key(ach_it->second.iIdx, pkAchiv->IsComplete(ach_it->second.iSaveIdx));
			std::pair<kAchieveSortCont::iterator, bool> Rst = kSortResult.insert(std::make_pair(Key, ach_it->second));
			if( !Rst.second )
			{
				assert(0);//중복
			}
		}
		++ach_it;
	}

	if( 0 != iEquipAchieveNo &&  NULL != pEquipAchieveDef )
	{
		SetAchieveEle(pkEquipMedal, pEquipAchieveDef->iIdx, pkAchiv, *pEquipAchieveDef, true);
	}
	else
	{
		SetClearAchieveEle(pkEquipMedal);
	}

	kAchieveSortCont::const_iterator c_iter = kSortResult.begin();
	while( kSortResult.end() != c_iter )
	{
		bool const bIsEquipAchievement = iEquipAchieveNo == c_iter->second.iTitleNo;
		XUI::SListItem* pItem = pkList->AddItem(BM::vstring(c_iter->second.iSaveIdx));
		if( pItem )
		{
			SetAchieveEle(pItem->m_pWnd, c_iter->second.iIdx, pkAchiv, c_iter->second, bIsEquipAchievement);
		}
		++c_iter;
	}
}

int lwUIBook::OnClickBookAchieveIcon(lwUIWnd kWnd)
{
	if(kWnd.IsNil()) {return 0;}

	XUI::CXUI_Wnd* pkEle = kWnd.GetParent()();
	if(!pkEle) {return 0;}
	
	int iCustom = 0;
	pkEle->GetCustomData(&iCustom, sizeof(iCustom));

	CONT_DEF_ACHIEVEMENTS const * pContDefAch = NULL;
	g_kTblDataMgr.GetContDef(pContDefAch);	//업적 데프

	if(!pContDefAch)
	{
		return 0;
	}

	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkMyPlayer) {return 0;}
	PgAchievements* pkAchiv = pkMyPlayer->GetAchievements();
	if(!pkAchiv) {return 0;}

	CONT_DEF_ACHIEVEMENTS::const_iterator ach_it = pContDefAch->find(iCustom);
	if(ach_it == pContDefAch->end())
	{
		return 0;
	}

	CONT_DEF_ACHIEVEMENTS::mapped_type const & rkAch = (*ach_it).second;

	if (pkAchiv->CheckHasItem(rkAch.iSaveIdx) && pkAchiv->IsComplete(rkAch.iSaveIdx))
	{
		XUI::CXUI_Wnd* pkBtn = pkEle->Parent()->Parent()->GetControl(L"BTN_BOOK_GET_ACHIEVE_ITEM");
		if(pkBtn)
		{
			pkBtn->Visible(bool(iCustom));
			XUI::CXUI_Wnd* pkBook1 = XUIMgr.Get(L"FRM_BOOK1");
			POINT3I kBookPos(0,0,0);
			if(pkBook1)
			{
				kBookPos = pkBook1->Location();
			}
			pkBtn->Location((XUIMgr.MousePos() - kBookPos) - pkEle->Parent()->Parent()->Location());
			pkBtn->SetCustomData(&iCustom, sizeof(iCustom));
		}
		return iCustom;
	}

	return 0;
}

void lwUIBook::OnClickBookAchieveGetItem(lwUIWnd kWnd, bool bReal)
{
	if(kWnd.IsNil()) {return;}
	XUI::CXUI_Wnd* pkParent = kWnd.GetParent()();
	int iCustom = kWnd.GetCustomData<int>();
	if(0<iCustom)
	{
		SPT_C_M_REQ_ACHIEVEMENT_TO_ITEM kStruct;
		kStruct.iAchievementIdx = iCustom;
		BM::Stream kPacket;
		kStruct.WriteToPacket(kPacket);
		NETWORK_SEND(kPacket)
		kWnd.Visible(false);
	}
} 

void lwUIBook::UpdateAchieveItem(int const iIndex, bool const bEquip)
{
	CONT_DEF_ACHIEVEMENTS const * pContDefAch = NULL;
	g_kTblDataMgr.GetContDef(pContDefAch);	//업적 데프

	if(!pContDefAch)
	{
		return;
	}

	CONT_DEF_ACHIEVEMENTS::const_iterator ach_it = pContDefAch->begin();
	for(;ach_it != pContDefAch->end();++ach_it)
	{
		if((*ach_it).second.iSaveIdx != iIndex)
		{
			continue;
		}
		break;
	}

	if(pContDefAch->end() == ach_it) 
	{ 
		return; 
	}

	XUI::CXUI_Wnd* pkBook = XUIMgr.Get(L"FRM_BOOK1");
	if (!pkBook) { return; }
	XUI::CXUI_Wnd* pkPage1 = pkBook->GetControl(L"FRM_PAGE1");
	if (!pkPage1) { return; }
	XUI::CXUI_Wnd* pkEquipMedal = pkPage1->GetControl(L"FRM_EQUIP_BOOK_MEDAL");
	if( !pkEquipMedal ){ return; }
	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkPage1->GetControl(L"LST_MEDAL"));
	if (!pkList) { return; }
	XUI::SListItem* pkItem = pkList->FindItem(BM::vstring(iIndex));
	if (!pkItem) { return; }

	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkMyPlayer) {return ;}
	PgAchievements* pkAchiv = pkMyPlayer->GetAchievements();
	if(!pkAchiv) {return ;}

	SetAchieveEle(pkEquipMedal, (*ach_it).first, pkAchiv, (*ach_it).second, bEquip);
	SetAchieveEle(pkItem->m_pWnd, (*ach_it).first, pkAchiv, (*ach_it).second, bEquip);
}

void lwUIBook::SetClearAchieveEle(XUI::CXUI_Wnd* pkEle)
{
	TBL_DEF_ACHIEVEMENTS kValue;
	SetAchieveEle(pkEle, 0, NULL, kValue, false);
}

void lwUIBook::SetAchieveEle(XUI::CXUI_Wnd *pkEle, int const iIndex, PgAchievements const* pkAchieve, TBL_DEF_ACHIEVEMENTS const& rkValue, bool const bEquip)
{
	if (!pkEle ) { return; }

	bool const bIsComplet = (pkAchieve)?(pkAchieve->IsComplete(rkValue.iSaveIdx)):(false);
	bool const bHasItem = (pkAchieve)?(pkAchieve->CheckHasItem(rkValue.iSaveIdx)):(false);

	pkEle->UVUpdate((bIsComplet)?(1):(2));
	pkEle->SetCustomData(&iIndex, sizeof(iIndex));
	XUI::CXUI_Wnd* pkTitle = pkEle->GetControl(L"SFRM_TITLE");
	if (pkTitle)
	{
		if( pkAchieve )
		{
			const wchar_t *pTitle = NULL;
			std::wstring kTitle;
			if(0 < iIndex && GetDefString(rkValue.iTitleNo, pTitle))
			{
				kTitle = pTitle;
			}
			pkTitle->Text(kTitle);
			pkTitle->GrayScale(!bIsComplet);
			pkTitle->FontColor((bIsComplet)?(0xFF790000):(0xFF433B2D));
		}
		else
		{
			pkTitle->Text(L"");
		}
	}

	XUI::CXUI_Wnd* pkDiscr = pkEle->GetControl(L"FRM_DISCRIPT");
	if(pkDiscr)
	{
		if( pkAchieve )
		{
			const wchar_t *pDisc = NULL;
			std::wstring kDisc;
			if(0 < iIndex && GetDefString(rkValue.iDiscriptionNo, pDisc))
			{
				kDisc = pDisc;
			}
			pkDiscr->Text(kDisc);
			pkDiscr->FontColor((bIsComplet)?(0xFF4E1D0E):(0xFF433B2D));
		}
		else
		{
			pkDiscr->Text(L"");
		}
	}

	XUI::CXUI_Wnd* pkImg = pkEle->GetControl(L"IMG_ITEM");
	if (pkImg)
	{
		if( pkAchieve )
		{
			PgBase_Item kItem;
			CreateSItem(rkValue.iItemNo, 1, 0, kItem);

			BM::PgPackedTime kTimeLimit;
			if( pkAchieve->GetAchievementTimeLimit(rkValue.iSaveIdx,kTimeLimit) )
			{
				__int64 i64LimitTime = 0;
				CGameTime::DBTimeEx2SecTime(static_cast<BM::DBTIMESTAMP_EX>(kTimeLimit),i64LimitTime);
				i64LimitTime -= rkValue.iUseTime;
				BM::DBTIMESTAMP_EX kGenTime;
				CGameTime::SecTime2DBTimeEx(i64LimitTime,kGenTime);

				int const iDay = std::min<int>((rkValue.iUseTime/(24*60*60)),MAX_CASH_ITEM_TIMELIMIT);
				int const iHour = std::min<int>((rkValue.iUseTime/(60*60)),MAX_CASH_ITEM_TIMELIMIT);
				int const iMin = std::min<int>((rkValue.iUseTime/(60)),MAX_CASH_ITEM_TIMELIMIT);

				if(0 < iDay)
				{
					kItem.SetUseTime(UIT_DAY,iDay);
				}
				else if(0 < iHour)
				{
					kItem.SetUseTime(UIT_HOUR,iHour);
				}
				else
				{
					kItem.SetUseTime(UIT_MIN,iMin);
				}

				kItem.CreateDate(static_cast<BM::PgPackedTime>(kGenTime));
			}

			lwUIWnd(pkImg).SetCustomDataAsSItem(kItem);
			pkImg->GrayScale(!bIsComplet);
			pkImg->SetInvalidate();
			pkImg->Visible(true);
		}
		else
		{
			pkImg->Visible(false);
		}
	}

	XUI::CXUI_Wnd* pkCount = pkEle->GetControl(L"IMG_COUNT");
	XUI::CXUI_AniBar* pkAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pkEle->GetControl(L"ANB_PERCENT"));
	if(pkCount && pkAniBar)
	{
		pkCount->Text((bIsComplet)?(TTW((bEquip)?(199):(3181))):(TTW(3308)));

		int iMax = 1;
		int iNow = 0;
		BM::vstring vStr(L"");
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			PgAchieveValueCtrl kValue(rkValue.iSaveIdx, *pkPlayer);
			iNow = kValue.Now();
			iMax = kValue.Max();

			vStr = iNow;
			vStr += L"{C=0xFFFFFFFF/}";
			vStr += L"/";
			vStr += iMax;
		}
		pkAniBar->Max(iMax);
		pkAniBar->Now(iNow);
		pkAniBar->Text(vStr);
	}

	XUI::CXUI_Wnd* pkRankPoint = pkEle->GetControl(L"IMG_RANKPOINT");
	if(pkRankPoint)
	{
		pkRankPoint->Text(BM::vstring(rkValue.iRankPoint));
	}

	XUI::CXUI_Wnd* pkEquipMark = pkEle->GetControl(L"IMG_EQUIP_MARK");
	if( pkEquipMark )
	{
		pkEquipMark->Visible(bEquip);
	}
}

bool SetIconInfo(XUI::CXUI_Wnd* pkWnd, SResourceIcon const& rkRscIcon, SIconImage const& rkIconImage, int const iNo)
{
	XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pkWnd);
	if(!pkIcon) {return false;}
	
	pkIcon->Visible(true);
	SIconInfo kIconInfo;
	kIconInfo.iIconGroup = KUIG_MONSTER;
	kIconInfo.iIconKey = iNo;
	kIconInfo.iIconResNumber = rkRscIcon.iIdx;
	pkIcon->SetIconInfo(kIconInfo);
	pkIcon->UVInfo(SUVInfo(rkIconImage.iMaxX, rkIconImage.iMaxY, rkRscIcon.iIdx));
	NiSourceTexture const* tex = g_kNifMan.GetTexture(MB(rkIconImage.wstrPath));
	if (tex)
	{
		pkIcon->ImgSize(POINT2(tex->GetWidth(), tex->GetHeight()));
		pkIcon->DefaultImgName(rkIconImage.wstrPath);		
	}
	else
	{
		return false;
	}

	return true;
}

void lwUIBook::ClearMonsterIcon(XUI::CXUI_Wnd* pkWnd)
{
	if(!pkWnd) {return;}

	SResourceIcon kRscIcon;
	kRscIcon.iIdx = 0;

	SIconImage kIconImage;

	for(int iMonCount = 0; iMonCount < BOOK_MONSTERCOUNT_PER_LINE; ++iMonCount)
	{
		BM::vstring kIconName(L"ICN_MONSTER");
		kIconName+=iMonCount;
		XUI::CXUI_Wnd *pkImg = pkWnd->GetControl(kIconName);
		if(pkImg)
		{
			pkImg->Visible(SetIconInfo(pkImg, kRscIcon, kIconImage, 0));
		}
	}
}

void lwUIBook::SetAchievementRankSlot(XUI::CXUI_Wnd* pkSlot, SACHIEVEMENTRANK const& kRankInfo)
{
	if( !pkSlot ){ return; }

	BM::vstring vStr(kRankInfo.iRank);

	if( 100 < kRankInfo.iRank )
	{
		pkSlot->Text(TTW(2604));
	}
	else
	{
		pkSlot->Text(vStr);
	}

	XUI::CXUI_Wnd* pkTemp = pkSlot->GetControl(L"SFRM_NAME");
	if( pkTemp )
	{
		pkTemp->Text(kRankInfo.kName);
	}

	pkTemp = pkSlot->GetControl(L"SFRM_ACHIEVE_POINT");
	if( pkTemp )
	{
		vStr = kRankInfo.iTotalPoint;
		pkTemp->Text(vStr);
	}

	for( int i = 1; i < 6; ++i )
	{
		std::map<int,int>::const_iterator point_itor = kRankInfo.kContPoint.find(i);
		if( point_itor != kRankInfo.kContPoint.end() )
		{
			vStr = point_itor->second;
		}
		else
		{
			vStr = 0;
		}

		std::wstring kFormName;

		switch( i )
		{
		case 1:	{ kFormName = L"SFRM_GROW";		}break;
		case 2:	{ kFormName = L"SFRM_ITEM";		}break;
		case 3:	{ kFormName = L"SFRM_QUEST";	}break;
		case 4:	{ kFormName = L"SFRM_WORLD";	}break;
		case 5:	{ kFormName = L"SFRM_MISSION";	}break;
		default:
			return;
		}

		pkTemp = pkSlot->GetControl(kFormName);
		if( pkTemp )
		{
			pkTemp->Text(vStr);
		}
	}
}

void lwUIBook::OnRecvBookRankInfo(BM::Stream& rkPacket)
{
	CONT_ACHIEVEMENT_RANK	kContRank;
	PU::TLoadTable_AM(rkPacket, kContRank);

	XUI::CXUI_Wnd* pkBookMain = XUIMgr.Get(L"FRM_BOOK5");
	if( !pkBookMain ){ return; }

	XUI::CXUI_Wnd* pkAchieveRankBtn = pkBookMain->GetControl(L"CBTN_TAB5");
	if( !pkAchieveRankBtn ){ return; }

	XUI::CXUI_Wnd* pkAchieveRankTab = pkBookMain->GetControl(L"FRM_PAGE5");
	if( !pkAchieveRankTab ){ return; }

	XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkAchieveRankTab->GetControl(L"BLD_RANK"));
	if( !pkBuild ){ return; }

	int const MAX_SLOT = pkBuild->CountX() * pkBuild->CountY();

	CONT_ACHIEVEMENT_RANK::const_iterator rank_itor = kContRank.begin();
	while( rank_itor != kContRank.end() )
	{
		CONT_ACHIEVEMENT_RANK::key_type const& kGuid = rank_itor->first;
		CONT_ACHIEVEMENT_RANK::mapped_type const& kRankInfo = rank_itor->second;

		if( g_kPilotMan.IsMyPlayer( kGuid ) )
		{
			SetAchievementRankSlot(pkAchieveRankTab->GetControl(L"SFRM_MY_RANK"), kRankInfo);
		}
		
		BM::vstring vStr(L"SFRM_RANK");
		vStr += (kRankInfo.iRank - 1);
		SetAchievementRankSlot(pkAchieveRankTab->GetControl(vStr), kRankInfo);
		++rank_itor;
	}
}

bool lwUIBook::tagAchieveSortKey::operator<(lwUIBook::tagAchieveSortKey const& rhs) const
{
	if( !bIsAttainment )
	{
		if( rhs.bIsAttainment ){ return false; }
	}
	else
	{
		if( !rhs.bIsAttainment ){ return true; }
	}

	return (iIdx < rhs.iIdx)?(true):(false);
}

PgBookActorCameraMgr::PgBookActorCameraMgr()
{
}

PgBookActorCameraMgr::~PgBookActorCameraMgr()
{
}

bool PgBookActorCameraMgr::Init()
{
	m_kCameraContainer.clear();

	TiXmlDocument kXmlDoc("bookPreviewInfo.xml");
	if( !PgXmlLoader::LoadFile(kXmlDoc, UNI("bookPreviewInfo.xml")) )
	{
		return false;
	}

	TiXmlElement const* pkElement = kXmlDoc.FirstChildElement();
	while( pkElement )
	{
		char const* pcTagName = pkElement->Value();

		if( strcmp(pcTagName, "BOOK_PREVIEW_CAMERA_INFO") == 0 )
		{
			TiXmlElement const* pkSubElem = pkElement->FirstChildElement();
			
			while( pkSubElem )
			{
				char const* pcTagName = pkSubElem->Value();
				bool bResult = false;
				if( strcmp(pcTagName, "ITEM") == 0 || strcmp((pcTagName), "DEFAULT") == 0 )
				{
					SBookCameraInfo	kInfo;
					std::string		kActorName;

					TiXmlAttribute const* pAttr = pkSubElem->FirstAttribute();
					while( pAttr )
					{
						char const* pcAttrName = pAttr->Name();
						char const* pcTagValue = pAttr->Value();

						if( strcmp( pcAttrName, "ID" ) == 0 )
						{
							kActorName = pcTagValue;
						}
						else if( strcmp( pcAttrName, "POS_X" ) == 0 )
						{
							kInfo.Pos.x = static_cast<float>(atof(pcTagValue));
						}
						else if( strcmp( pcAttrName, "POS_Y" ) == 0 )
						{
							kInfo.Pos.y = static_cast<float>(atof(pcTagValue));
						}
						else if( strcmp( pcAttrName, "POS_Z" ) == 0 )
						{
							kInfo.Pos.z = static_cast<float>(atof(pcTagValue));
						}
						else if( strcmp( pcAttrName, "ZOOM_MAX" ) == 0 )
						{
							kInfo.ZoomMax = static_cast<float>(atof(pcTagValue));
						}
						else if( strcmp( pcAttrName, "ZOOM_MIN" ) == 0 )
						{
							kInfo.ZoomMin = static_cast<float>(atof(pcTagValue));
						}
						else
						{
							
						}
						pAttr = pAttr->Next();
					}

					if( strcmp((pcTagName), "DEFAULT") == 0 )
					{
						m_kDefaultInfo = kInfo;
					}
					else
					{
						std::pair<CONT_BOOK_ACTOR_CAMERA::iterator, bool> result = m_kCameraContainer.insert(std::make_pair(kActorName, kInfo));
						if( !result.second )
						{//error
							assert(0);
						}
					}
				}
				pkSubElem = pkSubElem->NextSiblingElement();
			}
		}
		else if( strcmp(pcTagName, "BOOK_PREIVEW_MONSTER_CONTENTS") == 0 )
		{
			TiXmlElement const* pkSubElem = pkElement->FirstChildElement();
			
			while( pkSubElem )
			{
				char const* pcTagName = pkSubElem->Value();
				bool bResult = false;
				if( strcmp(pcTagName, "ITEM") == 0 )
				{
					int iMonID = 0;
					CONT_BOOK_MAPNO MapCont;

					TiXmlAttribute const* pAttr = pkSubElem->FirstAttribute();
					while( pAttr )
					{
						char const* pcAttrName = pAttr->Name();
						char const* pcTagValue = pAttr->Value();

						if( strcmp( pcAttrName, "ID" ) == 0 )
						{
							iMonID = atoi(pcTagValue);
						}
						else if( strcmp( pcAttrName, "MAPNO" ) == 0 )
						{
							typedef	std::list< std::wstring >	CUT_STRING;
							CUT_STRING kList;
							BM::vstring::CutTextByKey<std::wstring>(UNI(pcTagValue), L",", kList);

							CUT_STRING::iterator	iter = kList.begin();
							while( iter != kList.end() )
							{
								MapCont.push_back(_wtoi(iter->c_str()));
								++iter;
							}
						}
						else
						{
							
						}
						pAttr = pAttr->Next();
					}

					std::pair<CONT_BOOK_MONSTER_CONTANTS::iterator, bool> result = m_kMonContantsContainer.insert(std::make_pair(iMonID, MapCont));
					if( !result.second )
					{//error
						assert(0);
					}
				}
				pkSubElem = pkSubElem->NextSiblingElement();
			}
		}
		pkElement = pkElement->NextSiblingElement();
	}
	return true;
}

bool PgBookActorCameraMgr::GetActorCameraInfo(const std::string &ActorName, SBookCameraInfo& CameraInfo)
{
	CONT_BOOK_ACTOR_CAMERA::const_iterator	c_iter = m_kCameraContainer.find(ActorName);
	if( c_iter != m_kCameraContainer.end() )
	{
		CameraInfo = c_iter->second;
		return true;
	}
	return false;
}

CONT_BOOK_MAPNO const& PgBookActorCameraMgr::GetMonsterContants(int const MonID)
{
	CONT_BOOK_MONSTER_CONTANTS::const_iterator c_iter = m_kMonContantsContainer.find(MonID);
	if( c_iter != m_kMonContantsContainer.end() )
	{
		return c_iter->second;
	}

	static CONT_BOOK_MAPNO kTemp;
	return kTemp;
}