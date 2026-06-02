#include "StdAfx.h"
#include "lwbase.h"
#include "lwUI.h"
#include "lwUIItemSocketSystem.h"
#include "lwUIToolTip.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "Variant/PgPlayer.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "PgNifMan.H"
#include "PgUIModel.h"

#include "variant/item.h"
#include "lohengrin/packettype.h"
#include "PgChatMgrClient.h"
#include "PgSoundMan.h"
#include "Variant/PgSocketFormula.h"
#include "Variant/PgItemRarityUpgradeFormula.h"
#include "lwUIQuest.h"
#include "lwHomeUI_Script.h"
#include "PgMobileSuit.h"
#include "Variant/PgStringUtil.h"

float const SOCKET_PROGRESS_TIME = 0.667f; //진행 시간은 1초
float const UIMODEL_SOCKET_EFFECT_RESULT_TIME = 0.53f;	//결과 이펙트 지속 시간
extern int const SOUL_ITEMNO = 79000030;
//int const CRYSTAL_ITEM_NO_BASE = 20700000;
int const ISURANCE_ITEM_NO_BASE = 79000310;	//파괴석 아이템.
int const PROBABILITY_ITEM_NO_BASE = 98000670; // 확률 아이템.
int const MAX_SOCKET = 100;

namespace PgSocketSystemUtil
{
	std::wstring const kSocketSystemUIName(_T("SFRM_SOCKET_SYSTEM"));

	lwUIWnd CallSocketSystemUI(lwGUID kNpcGuid, int iMenu)
	{
		g_kSocketSystemMgr.NpcGuid( kNpcGuid() );
		g_kSocketSystemMgr.SetMenuType(iMenu);
		g_kSocketSystemMgr.SetSelectOrderMenu(0);

		CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kSocketSystemUIName );
		if( pkTopWnd )
		{
			RegistUIAction(pkTopWnd);
			return lwUIWnd(pkTopWnd);
		}
		return lwUIWnd(NULL);
	}
};

namespace PgMonsterCardUtil
{
	int const DEFAULT_SUCCESS_RATE = 7000;// 어빌에 값이 없으면 70% 고정

	bool IsMonsterCardSocketState(SEnchantInfo const kEhtInfo, int const iOrderIndex)
	{
		switch( iOrderIndex )
		{
		case PgSocketFormula::SII_FIRST:
			{
				if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState()) && (0 == kEhtInfo.MonsterCard()) )
				{
					return true;
				}				
			}break;
		case PgSocketFormula::SII_SECOND:
			{
				if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState2()) && (0 == kEhtInfo.MonsterCard2()) )
				{
					return true;
				}
			}break;
		case PgSocketFormula::SII_THIRD:
			{
				if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState3()) && (0 == kEhtInfo.MonsterCard3()) )
				{
					return true;
				}				
			}break;
		default:
			{
			}break;
		}
		return false;
	}

	EMonsterCardItemType ItemMonsterCardAdd(XUI::CXUI_Wnd* pWnd, PgBase_Item const* pkItem, SItemPos rkPos, int const iEquipLimit, int const iOrderIndex, int const iItemLevelMin, int const iItemLevelMax)
	{
		XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pWnd);
		if( !pkList )
		{
			return MCIT_FAIL;
		}

		SEnchantInfo const kEhtInfo = pkItem->EnchantInfo();
		if( IsMonsterCardSocketState(kEhtInfo, iOrderIndex) )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pItemDef = kItemDefMgr.GetDef(pkItem->ItemNo());
			if(!pItemDef)
			{
				return MCIT_FAIL;
			}

			if(0 == (pItemDef->GetAbil(AT_EQUIP_LIMIT) & iEquipLimit))
			{
				return MCIT_EQUIP_LIMIT;
			}

			if( (iItemLevelMin > pItemDef->GetAbil(AT_LEVELLIMIT)) || (iItemLevelMax < pItemDef->GetAbil(AT_LEVELLIMIT)) )
			{
				return MCIT_FAIL;
			}

			XUI::SListItem* Item = pkList->AddItem(_T(""));
			if(Item)
			{
				XUI::CXUI_Wnd* pkItemWnd = Item->m_pWnd;
				pkItemWnd->SetCustomData(&rkPos, sizeof(rkPos));
				XUI::CXUI_Wnd* pkIcon = pkItemWnd->GetControl(_T("IMG_ICON"));
				if(pkIcon)
				{
					DWORD ItemNo = pkItem->ItemNo();
					pkIcon->SetCustomData(&ItemNo, sizeof(ItemNo));
				}

				XUI::CXUI_Wnd* pkName = pkItemWnd->GetControl(_T("SFRM_ITEM_NAME"));
				if(pkName)
				{
					std::wstring const*	pszName = NULL;
					if(GetDefString(pItemDef->NameNo(), pszName))
					{
						int const iWidth = pkName->Width()-30;
						Quest::SetCutedTextLimitLength(pkName, *pszName, _T("..."));					
					}
				}
			}
			return MCIT_SUCCESS;
		}
		return MCIT_FAIL;
	}

	int CalcurateSocketCardRate(XUI::CXUI_Wnd* pkTopWnd)
	{
		XUI::CXUI_Wnd * pIcon = pkTopWnd->GetControl(L"ICN_CARDITEM");
		DWORD ItemNo = 0;
		pIcon->GetCustomData(&ItemNo, sizeof(ItemNo));
		if( 0 == ItemNo )
		{ return 0; }

		XUI::CXUI_Wnd* pkInsuranceWnd = pkTopWnd->GetControl(_T("SFRM_INSURANCE"));
		if(NULL == pkInsuranceWnd)
		{ return 0; }		
		XUI::CXUI_Wnd* pkInsuranceNum = pkInsuranceWnd->GetControl(_T("FRM_NUM_BG"));
		if(NULL == pkInsuranceNum)
		{ return 0; }
		int iInsuranceNum = 0;
		pkInsuranceNum->GetCustomData( &iInsuranceNum, sizeof(iInsuranceNum) );

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(ItemNo);
		if( !pDef )
		{
			return 0;
		}
		int const iSuccessRate = pDef->GetAbil(AT_SOCKETCARD_INSERT); // 소켓카드 적용 성공확률
		int iRate = iSuccessRate ? iSuccessRate : DEFAULT_SUCCESS_RATE; 

		if( 0 != iInsuranceNum )
		{
			int const iAddRate = (iInsuranceNum-1) * 3000;
			iRate += iAddRate;
		}
		iRate /= 100;
		if( 100 < iRate )
		{
			iRate = 100;
		}
		return iRate;
	}

	void lwItemSocketSystemProcessRequest(lwUIWnd ItemParent)
	{
		if(ItemParent.IsNil()){ return; }
		XUI::CXUI_Wnd* pWnd = ItemParent.GetSelf();

		SItemPos rkPos;
		pWnd->GetCustomData(&rkPos, sizeof(rkPos));

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer){ return; }

		PgInventory *pkInven = pkPlayer->GetInven();
		if(!pkInven){ return; }

		XUI::CXUI_Wnd*	pkMainWnd = XUIMgr.Get(_T("SFRM_MONSTER_CARD"));
		if( !pkMainWnd )
		{ return; }
		XUI::CXUI_Wnd* pkInsuranceWnd = pkMainWnd->GetControl(_T("SFRM_INSURANCE"));
		if(NULL == pkInsuranceWnd)
		{ return; }
		XUI::CXUI_Wnd* pkInsuranceNum = pkInsuranceWnd->GetControl(_T("FRM_NUM_BG"));
		if(NULL == pkInsuranceNum)
		{ return; }
		XUI::CXUI_Wnd* pkInsuranceIcon = pkInsuranceWnd->GetControl(_T("INC_INSURANCE"));
		if(NULL == pkInsuranceIcon)
		{ return; }

		int iInsuranceNum = 0;
		pkInsuranceNum->GetCustomData( &iInsuranceNum, sizeof(iInsuranceNum) );
		
		int iInsuranceItemNo = 0;
		pkInsuranceIcon->GetCustomData( &iInsuranceItemNo, sizeof(iInsuranceItemNo) );
		size_t const uiHaveInsuranceCount = pkInven->GetInvTotalCount(iInsuranceItemNo);

		SItemPos	kScrollPos;
		pkMainWnd->GetCustomData(&kScrollPos, sizeof(kScrollPos));
		PgBase_Item kItem;
		if(E_FAIL == pkInven->GetItem(kScrollPos, kItem))
		{// 인벤토리에 해당 아이템이 없다면
			unsigned long ulTextID = 790416;
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", ulTextID, true);
		}
		else if( uiHaveInsuranceCount < iInsuranceNum )
		{
			lwAddWarnDataTT( 1299 );
		}
		else
		{// 인벤토리에 아이템을 가지고 있다면				
			BM::Stream kPacket(PT_C_M_REQ_SET_MONSTERCARD);
			kPacket.Push(kScrollPos);
			kPacket.Push(rkPos);
			kPacket.Push(iInsuranceItemNo);
			kPacket.Push(iInsuranceNum);

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pkDefItem = kItemDefMgr.GetDef(kItem.ItemNo());
			if(!pkDefItem)
			{ return; }
			if( pkDefItem->GetAbil(AT_OLD_MONSTERCARD) )
			{ // 이 어빌을 갖고 있으면 몬스터 카드
				int iTT = 790417;
				if( 0 < iInsuranceNum )
				{
					iTT = 760003;
				}
				lwCallCommonMsgYesNoBox(MB(TTW(iTT)), lwPacket(&kPacket), true, MBT_CONFIRM_MONSTERCARD);
			}
			else
			{ // 아니면 소켓 카드
				int iTT = 790398;
				if( 0 < iInsuranceNum )
				{
					iTT = 760002;
				}
				lwCallCommonMsgYesNoBox(MB(TTW(iTT)), lwPacket(&kPacket), true, MBT_CONFIRM_MONSTERCARD);
			}
		}
	}

	void UpdateUISocketCard(XUI::CXUI_Wnd* pWnd)
	{		
		if( !pWnd )
		{ return; }
		XUI::CXUI_Wnd *Ratewnd = pWnd->GetControl(L"SFRM_AUBOX");
		if( !Ratewnd )
		{ return; }

		int const iRate = CalcurateSocketCardRate(pWnd);
		BM::vstring vStr(TTW(759997));
		vStr.Replace(L"#NUM#", iRate);
		Ratewnd->Text(vStr);
	}

	void SetTextSocketCardType(XUI::CXUI_Wnd* pWnd)
	{
		XUI::CXUI_Wnd * pIcon = pWnd->GetControl(L"ICN_CARDITEM");
		if( !pIcon )
		{ return; }

		DWORD ItemNo = 0;
		pIcon->GetCustomData(&ItemNo, sizeof(ItemNo));
		if( 0 == ItemNo )
		{ return; }

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(ItemNo);
		if( !pDef )
		{ return; }

		XUI::CXUI_Form * Info = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"SFRM_TITLE_VIEW"));
		XUI::CXUI_StaticForm * Title = dynamic_cast<XUI::CXUI_StaticForm*>(pWnd->GetControl(L"SFRM_TITLE"));
		if( NULL == Info || NULL == Title )
		{ return; }
		XUI::CXUI_StaticForm * ColorBox = dynamic_cast<XUI::CXUI_StaticForm*>(Title->GetControl(L"SFRM_COLOR"));
		if( NULL == ColorBox )
		{ return; }
		XUI::CXUI_StaticForm * TitleText = dynamic_cast<XUI::CXUI_StaticForm*>(ColorBox->GetControl(L"SFRM_TITLE_SDW"));
		if( NULL == TitleText )
		{ return; }

		int const CardType = pDef->GetAbil(AT_OLD_MONSTERCARD); // 구몬스터카드에만 있는 어빌
		if( CardType )
		{ // 이 어빌이 존재하면 구몬스터카드라는 뜻이므로
			Info->Text(TTW(790108));
			TitleText->Text(TTW(790104));
		}
		else
		{ // 없으면 신소켓카드다.
			Info->Text(TTW(759999));
			TitleText->Text(TTW(759998));
		}
	}
	void SetMonsterCardInsurance( XUI::CXUI_Wnd* pkTopWnd )
	{
		if( !pkTopWnd )
		{ return; }
		XUI::CXUI_Wnd * pIcon = pkTopWnd->GetControl(L"ICN_CARDITEM");
		if( !pIcon )
		{ return; }

		DWORD ItemNo = 0;
		pIcon->GetCustomData(&ItemNo, sizeof(ItemNo));
		if( 0 == ItemNo )
		{ return; }
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDefCardItem = kItemDefMgr.GetDef(ItemNo);
		if( !pDefCardItem )
		{ return; }

		int iInsuranceItemNo = pDefCardItem->GetAbil(AT_USE_INSURANCE_ITEM_NO);
		if( 0 == iInsuranceItemNo )
		{
			lwAddWarnDataTT(401304);
			return;
		}

		XUI::CXUI_Wnd* pkInsuranceWnd = pkTopWnd->GetControl(_T("SFRM_INSURANCE"));
		if(NULL == pkInsuranceWnd)
		{ return; }

		CItemDef const *pDefInsuranceItem = kItemDefMgr.GetDef(iInsuranceItemNo);
		if( NULL == pDefInsuranceItem )
		{ 
			lwAddWarnDataTT(401304);
			return;
		}		
		std::wstring wInsuranceItemName;
		if( false == g_kTblDataMgr.GetStringName(pDefInsuranceItem->NameNo(), wInsuranceItemName) )
		{ return; }
		BM::vstring kInsuranceInfo( TTW(760000) );
		kInsuranceInfo.Replace( L"#INSURANCE_ITEM#", wInsuranceItemName );
		pkInsuranceWnd->Text( static_cast<std::wstring>(kInsuranceInfo) );

		XUI::CXUI_Wnd* pkInsuranceIcon = pkInsuranceWnd->GetControl(_T("INC_INSURANCE"));
		if(NULL == pkInsuranceIcon)
		{ return; }
		pkInsuranceIcon->SetCustomData( &iInsuranceItemNo, sizeof(iInsuranceItemNo) );
		pkInsuranceIcon->GrayScale(true);

		XUI::CXUI_Wnd* pkInsuranceNum = pkInsuranceWnd->GetControl(_T("FRM_NUM_BG"));
		if(NULL == pkInsuranceNum)
		{ return; }
		pkInsuranceNum->ClearCustomData();
		BM::vstring kNum(L"#NUM#");
		kNum.Replace(L"#NUM#", 0);
		pkInsuranceNum->Text( static_cast<std::wstring>(kNum) );

		XUI::CXUI_Wnd* pkInsurancePlus = pkInsuranceWnd->GetControl(_T("BTN_PLUS"));
		if(NULL == pkInsurancePlus)
		{ return; }
		pkInsurancePlus->Visible(true);

		XUI::CXUI_Wnd* pkInsuranceMinus = pkInsuranceWnd->GetControl(_T("BTN_MINUS"));
		if(NULL == pkInsuranceMinus)
		{ return; }
		pkInsuranceMinus->Visible(false);
	}

	void AddSocketInsurance()
	{
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_MONSTER_CARD"));
		if( NULL == pkTopWnd )
		{ return; }
		XUI::CXUI_Wnd* pkInsuranceWnd = pkTopWnd->GetControl(_T("SFRM_INSURANCE"));
		if(NULL == pkInsuranceWnd)
		{ return; }
		XUI::CXUI_Wnd* pkInsuranceIcon = pkInsuranceWnd->GetControl(_T("INC_INSURANCE"));
		if(NULL == pkInsuranceIcon)
		{ return; }
		int iInsuranceItemNo = 0;
		pkInsuranceIcon->GetCustomData( &iInsuranceItemNo, sizeof(iInsuranceItemNo) );
		if( 0 == iInsuranceItemNo )
		{ return; }
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer){ return; }
		PgInventory *pkInven = pkPlayer->GetInven();
		if(!pkInven){ return; }
		size_t iHaveInsuranceItemCount = pkInven->GetTotalCount( iInsuranceItemNo, false );

		XUI::CXUI_Wnd* pkInsuranceNum = pkInsuranceWnd->GetControl(_T("FRM_NUM_BG"));
		if(NULL == pkInsuranceNum)
		{ return; }
		int iInsuranceNum = 0;
		pkInsuranceNum->GetCustomData( &iInsuranceNum, sizeof(iInsuranceNum) );
		int iRate = CalcurateSocketCardRate( pkTopWnd );
		if( 100 != iRate )
		{
			++iInsuranceNum;
			if( iInsuranceNum > iHaveInsuranceItemCount )
			{
				lwAddWarnDataTT( 1299 );
				return;
			}
			pkInsuranceNum->SetCustomData( &iInsuranceNum, sizeof(iInsuranceNum) );
			BM::vstring kNum(L"#NUM#");
			kNum.Replace(L"#NUM#", iInsuranceNum);
			pkInsuranceNum->Text( static_cast<std::wstring>(kNum) );

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDefInsuranceItem = kItemDefMgr.GetDef(iInsuranceItemNo);
			if( NULL == pDefInsuranceItem )
			{ 
				lwAddWarnDataTT(401304);
				return;
			}		
			std::wstring wInsuranceItemName;
			if( false == g_kTblDataMgr.GetStringName(pDefInsuranceItem->NameNo(), wInsuranceItemName) )
			{ return; }
			BM::vstring kInsuranceInfo( TTW(760001) );
			kInsuranceInfo.Replace( L"#INSURANCE_ITEM#", wInsuranceItemName );
			pkInsuranceWnd->Text( static_cast<std::wstring>(kInsuranceInfo) );
		}
		UpdateUISocketCard( pkTopWnd );
		iRate = CalcurateSocketCardRate( pkTopWnd );

		XUI::CXUI_Wnd* pkkInsurancePlus = pkInsuranceWnd->GetControl(L"BTN_PLUS");
		XUI::CXUI_Wnd* pkkInsuranceMinus = pkInsuranceWnd->GetControl(L"BTN_MINUS");
		if( NULL == pkkInsurancePlus
			|| NULL == pkkInsuranceMinus )
		{ return; }
		bool const bItemNum_Empty = ( 0 == iInsuranceNum );
		bool const bItemNum_Max = ( 100 == iRate );
		pkInsuranceIcon->GrayScale( bItemNum_Empty );
		pkkInsurancePlus->Visible( !bItemNum_Max );
		pkkInsuranceMinus->Visible( !bItemNum_Empty );
	}
	
	void DelSocketInsurance()
	{
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_MONSTER_CARD"));
		if( NULL == pkTopWnd )
		{ return; }

		XUI::CXUI_Wnd* pkInsuranceWnd = pkTopWnd->GetControl(_T("SFRM_INSURANCE"));
		if(NULL == pkInsuranceWnd)
		{ return; }

		XUI::CXUI_Wnd* pkInsuranceNum = pkInsuranceWnd->GetControl(_T("FRM_NUM_BG"));
		if(NULL == pkInsuranceNum)
		{ return; }
		int iInsuranceNum = 0;
		pkInsuranceNum->GetCustomData( &iInsuranceNum, sizeof(iInsuranceNum) );
		if( 2 >= iInsuranceNum && 0 < iInsuranceNum)
		{
			--iInsuranceNum;
			pkInsuranceNum->SetCustomData( &iInsuranceNum, sizeof(iInsuranceNum) );
			BM::vstring kNum(L"#NUM#");
			kNum.Replace(L"#NUM#", iInsuranceNum);
			pkInsuranceNum->Text( static_cast<std::wstring>(kNum) );
		}
		UpdateUISocketCard( pkTopWnd );
		int iRate = CalcurateSocketCardRate( pkTopWnd );

		XUI::CXUI_Wnd* pkInsuranceIcon = pkInsuranceWnd->GetControl(_T("INC_INSURANCE"));
		XUI::CXUI_Wnd* pkkInsurancePlus = pkInsuranceWnd->GetControl(L"BTN_PLUS");
		XUI::CXUI_Wnd* pkkInsuranceMinus = pkInsuranceWnd->GetControl(L"BTN_MINUS");
		if( NULL == pkkInsurancePlus
			|| NULL == pkkInsuranceMinus 
			|| NULL == pkInsuranceIcon )
		{ return; }
		bool const bItemNum_Empty = ( 0 == iInsuranceNum );
		bool const bItemNum_Max = ( 100 == iRate );
		pkInsuranceIcon->GrayScale( bItemNum_Empty );
		pkkInsurancePlus->Visible( !bItemNum_Max );
		pkkInsuranceMinus->Visible( !bItemNum_Empty );
		if( true == bItemNum_Empty )
		{		
			int iInsuranceItemNo = 0;
			pkInsuranceIcon->GetCustomData( &iInsuranceItemNo, sizeof(iInsuranceItemNo) );
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDefInsuranceItem = kItemDefMgr.GetDef(iInsuranceItemNo);
			if( NULL == pDefInsuranceItem )
			{ 
				lwAddWarnDataTT(401304);
				return;
			}		
			std::wstring wInsuranceItemName;
			if( false == g_kTblDataMgr.GetStringName(pDefInsuranceItem->NameNo(), wInsuranceItemName) )
			{ return; }
			BM::vstring kInsuranceInfo( TTW(760000) );
			kInsuranceInfo.Replace( L"#INSURANCE_ITEM#", wInsuranceItemName );
			pkInsuranceWnd->Text( static_cast<std::wstring>(kInsuranceInfo) );
		}
	}

	void MonsterCardListUpdate(DWORD const dwUseItemType, int const iItemNo, SItemPos const & rkItemInvPos)
	{
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Activate(_T("SFRM_MONSTER_CARD"));
		if( !pkTopWnd )
		{
			return;
		}

		XUI::CXUI_List *pkList = (XUI::CXUI_List*)pkTopWnd->GetControl(std::wstring(_T("LST_MONSTERCARD_LIST")));
		if( !pkList )
		{
			return;
		}		

		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}

		PgInventory *pkInven = pkPlayer->GetInven();
		if(!pkInven)
		{
			return;
		}

		pkTopWnd->SetCustomData(&rkItemInvPos, sizeof(SItemPos));

		XUI::CXUI_Wnd* pkIconCardItem = pkTopWnd->GetControl(_T("ICN_CARDITEM"));
		if(pkIconCardItem)
		{
			pkIconCardItem->SetCustomData(&iItemNo, sizeof(iItemNo));
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
		if( !pItemDef )
		{
			return;
		}

		int const iItemLevelMin = pItemDef->GetAbil(AT_LEVELLIMIT);
		int const iItemLevelMax = pItemDef->GetAbil(AT_MAX_LEVELLIMIT);
		int const iEquipLimit = pItemDef->GetAbil(AT_EQUIP_LIMIT);
		int const iOrderIndex = pItemDef->GetAbil(AT_MONSTER_CARD_ORDER);

		SetMonsterCardInsurance( pkTopWnd );
		UpdateUISocketCard( pkTopWnd );
		SetTextSocketCardType( pkTopWnd );

		ContHaveItemNoCount	kEquipCount;
		kEquipCount.clear();

		if(E_FAIL == pkInven->GetItems(IT_EQUIP, kEquipCount))
		{
			return;
		}
		if(E_FAIL == pkInven->GetItems(IT_CASH, kEquipCount))
		{
			return;
		}
		if(!kEquipCount.size())
		{
			return;
		}

		pkList->ClearList();
		pkList->DeleteAllItem();

		ContHaveItemNoCount::iterator	it = kEquipCount.begin();
		bool bIsExist = false;
		while(it != kEquipCount.end())
		{
			SItemPos	rkPos;
			if(S_OK == pkInven->GetFirstItem(it->first, rkPos))
			{
				while(true)
				{
					PgBase_Item kItem;
					if(S_OK == pkInven->GetItem(rkPos, kItem))
					{
						EMonsterCardItemType eMCType = ItemMonsterCardAdd(pkList, &kItem, rkPos, iEquipLimit, iOrderIndex, iItemLevelMin, iItemLevelMax);

						switch( eMCType )
						{
						case MCIT_SUCCESS:
							{
								bIsExist = true;
							}break;
						default:
							{
							}break;
						}
					}

					if(E_FAIL == pkInven->GetNextItem(it->first, rkPos))
					{
						break;
					}
				}
			}
			++it;
		}

		if( (!bIsExist) )
		{
			lwCloseUI("SFRM_MONSTER_CARD");
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790423, true);
		}
		/*else
		{
			if( !bIsExist )
			{
				lwCloseUI("SFRM_MONSTER_CARD");
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790415, true);
			}
		}*/
	}
};

void SocketSystemUpdateUI(int const iCreate)
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( PgSocketSystemUtil::kSocketSystemUIName );
	if (!pkWnd)
	{
		return;
	}
	XUI::CXUI_Wnd* pkSrc = pkWnd->GetControl(L"ICN_SRC");
	if (pkSrc)
	{
		g_kSocketSystemMgr.UpdateSoulItemView();
		lua_tinker::call<void, lwUIWnd, int>("SocketSystemUIUpdate", lwUIWnd(pkSrc), iCreate);
	}
}

void ClearSocketSystemUI()
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( PgSocketSystemUtil::kSocketSystemUIName );
	if (!pkWnd)
	{
		return;
	}

	for (int i = 0; i < 4; ++i)
	{
		BM::vstring kString(L"ICN_MATERIAL");
		kString+=i;
		XUI::CXUI_Wnd* pkMat = pkWnd->GetControl(kString);
		if (pkMat)
		{
			lwUIWnd(pkMat).SetCustomData<int>(0);
		}

		kString = L"BTN_REG";
		kString += i;
		XUI::CXUI_Wnd* pReg = pkWnd->GetControl(kString);
		if( pReg )
		{
			pReg->Visible(false);
		}

		kString = L"BTN_DEREG";
		kString += i;
		XUI::CXUI_Wnd* pDeReg = pkWnd->GetControl(kString);
		if( pReg )
		{
			pDeReg->Visible(false);
		}

		kString = L"IMG_LOCK";
		kString += i;
		XUI::CXUI_Wnd* pImgLock = pkWnd->GetControl(kString);
		if( pImgLock )
		{
			pImgLock->Visible(true);
		}
	}	

	XUI::CXUI_Wnd* pkSrcItem = pkWnd->GetControl(L"IMG_ITEM");
	if( pkSrcItem )
	{
		pkSrcItem->Visible(true);
	}

	XUI::CXUI_Wnd* pkSrc = pkWnd->GetControl(L"ICN_SRC");
	if (pkSrc)
	{
		lwUIWnd(pkSrc).SetCustomData<int>(0);
	}

	XUI::CXUI_Wnd* pkSoul = pkWnd->GetControl(L"ICN_MATERIAL_SOUL");
	if (pkSoul)
	{
		lwUIWnd(pkSoul).SetCustomData<int>(0);
	}
	XUI::CXUI_Wnd* pkMaterialCount = pkWnd->GetControl(L"SFRM_MATERIAL_ITEM_COUNT");
	if (pkMaterialCount)
	{
		std::wstring wstr;
		wstr = TTW(790168);
		pkMaterialCount->Text(wstr);
		pkMaterialCount->Visible(false);
	}

	XUI::CXUI_Wnd* pkSdw = pkWnd->GetControl(L"SFRM_SHADOW");
	if (pkSdw)
	{
		XUI::CXUI_Wnd* pkItemNeed = pkSdw->GetControl(L"SFRM_COLOR_ITEM_NEED");
		if( pkItemNeed )
		{
			std::wstring wstrMsg (_T("{C=0xFF7b2e00/}") );
			WstringFormat( wstrMsg, MAX_PATH, TTW(790177).c_str(), 0 );
			pkItemNeed->Text(wstrMsg);
		}
	}
	g_kSocketSystemMgr.SetSoulText();

	XUI::CXUI_CheckButton* pkBtn1 = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"BTN_1"));
	XUI::CXUI_CheckButton* pkBtn2 = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"BTN_2"));
	XUI::CXUI_CheckButton* pkBtn3 = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"BTN_3"));
	if( pkBtn1 ) { pkBtn1->Visible(false); }
	if( pkBtn2 ) { pkBtn2->Visible(false); }
	if( pkBtn3 ) { pkBtn3->Visible(false); }

	pkWnd->SetInvalidate();
}

void InitSocketMaterialBtnState()
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( PgSocketSystemUtil::kSocketSystemUIName );
	if (!pkWnd)
	{
		return;
	}

	if(g_kSocketSystemMgr.GetMenuType() ==  PgItemSocketSystemMgr::SIM_EXTRACTION )
	{
		return;
	}

	for(int i = PgItemSocketSystemMgr::SIT_SOUL; i <= PgItemSocketSystemMgr::SIT_INSUR_ITEM; ++i)
	{
		g_kSocketSystemMgr.SetSrcMaterialBtnInit(pkWnd, i, true);
	}
}

lwUIItemSocketSystem::lwUIItemSocketSystem(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

bool lwUIItemSocketSystem::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "CallSocketSystemUI", PgSocketSystemUtil::CallSocketSystemUI);
	def(pkState, "MonsterCardListUpdate", &PgMonsterCardUtil::MonsterCardListUpdate);
	def(pkState, "ItemSocketSystemProcessRequest", &PgMonsterCardUtil::lwItemSocketSystemProcessRequest);
	def(pkState, "AddSocketInsurance", &PgMonsterCardUtil::AddSocketInsurance);
	def(pkState, "DelSocketInsurance", &PgMonsterCardUtil::DelSocketInsurance);
	
	class_<lwUIItemSocketSystem>(pkState, "ItemSocketSystemWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "DisplaySrcIcon", &lwUIItemSocketSystem::DisplaySrcIcon)
		.def(pkState, "DisplayNeedItemIcon", &lwUIItemSocketSystem::DisplayNeedItemIcon)
		.def(pkState, "DisplayResultItemIcon", &lwUIItemSocketSystem::DisplayResultItem)
		.def(pkState, "ClearUpgradeData", &lwUIItemSocketSystem::ClearUpgradeData)
		.def(pkState, "SendReqSocketSystem", &lwUIItemSocketSystem::SendReqSocketSystem)
		.def(pkState, "GetUpgradeCreateNeedMoney", &lwUIItemSocketSystem::GetUpgradeCreateNeedMoney)
		.def(pkState, "GetUpgradeRemoveNeedMoney", &lwUIItemSocketSystem::GetUpgradeRemoveNeedMoney)
		.def(pkState, "GetUpgradeResetNeedMoney", &lwUIItemSocketSystem::GetUpgradeResetNeedMoney)
		.def(pkState, "GetUpgradeExtractionNeedMoney", &lwUIItemSocketSystem::GetUpgradeExtractionNeedMoney)
		.def(pkState, "CallComfirmMessageBox", &lwUIItemSocketSystem::CallComfirmMessageBox)
		.def(pkState, "Clear", &lwUIItemSocketSystem::Clear)
		.def(pkState, "CheckOK", &lwUIItemSocketSystem::CheckOK)
		.def(pkState, "GetNowNeedItemCount", &lwUIItemSocketSystem::GetNowNeedItemCount)
		.def(pkState, "OnDisplay", &lwUIItemSocketSystem::OnDisplay)
		.def(pkState, "OnTick", &lwUIItemSocketSystem::OnTick)
		.def(pkState, "InProgress", &lwUIItemSocketSystem::InProgress)
		.def(pkState, "SetAttachElement", &lwUIItemSocketSystem::SetAttachElement)
		.def(pkState, "GetExplane", &lwUIItemSocketSystem::GetExplane)
		.def(pkState, "SetLockSlot", &lwUIItemSocketSystem::SetLockSlot)
		.def(pkState, "SetMaterialItem", &lwUIItemSocketSystem::SetMaterialItem)
		.def(pkState, "GetSoulCreateCount", &lwUIItemSocketSystem::GetSoulCreateCount)
		.def(pkState, "GetSoulRemoveCount", &lwUIItemSocketSystem::GetSoulRemoveCount)
		.def(pkState, "GetSoulResetCount", &lwUIItemSocketSystem::GetSoulResetCount)
		.def(pkState, "GetHaveCount", &lwUIItemSocketSystem::GetHaveCount)
		.def(pkState, "GetExtractionCount", &lwUIItemSocketSystem::GetExtractionCount)
		.def(pkState, "GetSocketSystemEmpty", &lwUIItemSocketSystem::GetSocketSystemEmpty)
		.def(pkState, "IsSocketItemEmpty", &lwUIItemSocketSystem::IsSocketItemEmpty)
		.def(pkState, "IsInsureItemPos", &lwUIItemSocketSystem::IsInsureItemPos)
		.def(pkState, "IsProbabilityItemPos", &lwUIItemSocketSystem::IsProbabilityItemPos)
		.def(pkState, "CallSocketItem", &lwUIItemSocketSystem::CallSocketItem)
		.def(pkState, "SetMenuType", &lwUIItemSocketSystem::SetMenuType)
		.def(pkState, "GetMenuType", &lwUIItemSocketSystem::GetMenuType)	
		.def(pkState, "CheckMenuSocketItem", &lwUIItemSocketSystem::CheckMenuSocketItem)	
		.def(pkState, "SetSelectOrderMenu", &lwUIItemSocketSystem::SetSelectOrderMenu)
		.def(pkState, "GetSelectOrderMenu", &lwUIItemSocketSystem::GetSelectOrderMenu)	
		.def(pkState, "GetMenuTypeOrder", &lwUIItemSocketSystem::GetMenuTypeOrder)	
		.def(pkState, "GetSelectOrderMenuCheck", &lwUIItemSocketSystem::GetSelectOrderMenuCheck)		
		.def(pkState, "IsSelectOrderMenuIndex", &lwUIItemSocketSystem::IsSelectOrderMenuIndex)		
		.def(pkState, "DisplaySocketRemoveText", &lwUIItemSocketSystem::DisplaySocketRemoveText)				
		.def(pkState, "SetSocketItemMenuUIUpdate", &lwUIItemSocketSystem::SetSocketItemMenuUIUpdate)
		.def(pkState, "SetMateriaItemExtractionBtn", &lwUIItemSocketSystem::lwSetMateriaItemExtraction)
		.def(pkState, "SetSoulText", &lwUIItemSocketSystem::SetSoulText)
		
	;
	return true;
}

void lwUIItemSocketSystem::SetSoulText()
{
	g_kSocketSystemMgr.SetSoulText();
}

void lwUIItemSocketSystem::SetMaterialItem(int iType, bool bCreate, bool bNoBuyMsg)
{
	g_kSocketSystemMgr.SetMaterialItem(self->Parent(), iType, bCreate, bNoBuyMsg);
}

void lwUIItemSocketSystem::lwSetMateriaItemExtraction(int iCount,int iCashBtnRegCount)
{
	if( g_kSocketSystemMgr.IsSocketItemEmpty() )
	{
		return;
	}
	g_kSocketSystemMgr.SetMaterialExtractionBtnInit(self->Parent(),iCount,iCashBtnRegCount);
	
}


void lwUIItemSocketSystem::CallSocketItem()
{
	if( g_kSocketSystemMgr.IsSocketItemEmpty() )
	{
		lwAddWarnDataTT(790115);
		return;
	}

	ContHaveItemNoCount	kItemCont;
	if( UIItemUtil::SearchEqualTypeItemList(UICT_SOCKET_SUCCESS, kItemCont) )
	{
		/*if( 1 < kItemCont.size() )
		{
			UIItemUtil::CallCommonUseCustomTypeItems(kItemCont, UIItemUtil::ECIUT_CUSTOM_DEFINED, UIItemUtil::CONT_CUSTOM_PARAM(), UIItemUtil::CONT_CUSTOM_PARAM_STR());
		}
		else
		{*/
			if( self )
			{
				g_kSocketSystemMgr.SetMaterialItem(self->Parent(), PgItemSocketSystemMgr::SIT_PROBABILITY, false, false);
			}			
		//}
	}
	else
	{
		//lua_tinker::call<void, int, int>("OnCallStaticCashItemBuy", 3, 0);
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790401, true);
	}
}

void lwUIItemSocketSystem::SetLockSlot(int const iIndex)
{
	if (!self)
	{
		return;
	}

	if( iIndex == 1 || iIndex == 2 || iIndex == 3 )
	{
		if(g_kSocketSystemMgr.AttachElement() == E_PPTY_CURSE)
		{
			if(!self->Visible())
			{
				self->Visible(true);
			}
		}
		else
		{
			self->Visible(false);
		}
	}
}

bool lwUIItemSocketSystem::IsInsureItemPos() const
{
	return g_kSocketSystemMgr.IsInsureItemPos();
}

bool lwUIItemSocketSystem::IsProbabilityItemPos() const
{
	return g_kSocketSystemMgr.IsProbabilityItemPos();
}

void lwUIItemSocketSystem::Clear(bool const bClearAll)
{
	g_kSocketSystemMgr.Clear(bClearAll);
}

void lwUIItemSocketSystem::DisplaySrcIcon()
{
	g_kSocketSystemMgr.DisplaySrcItem(self);
}

void lwUIItemSocketSystem::DisplayResultItem()
{
	g_kSocketSystemMgr.DisplayResultItem(self);
}

void lwUIItemSocketSystem::DisplayNeedItemIcon(int iIndex)
{
	if (!self)
	{
		return;
	}

	g_kSocketSystemMgr.DisplayNeedItemIcon( iIndex, self );
	assert(NULL && "lwUIItemSocketSystem::DisplayNeedItemIcon");
}

void lwUIItemSocketSystem::ClearUpgradeData()
{
	g_kSocketSystemMgr.RunProgressEffect(false);
	g_kSocketSystemMgr.Clear();
}

bool lwUIItemSocketSystem::SendReqSocketSystem(bool bIsTrueSend)
{
	//if (!bIsTrueSend)
	{
		g_kSocketSystemMgr.RunProgressEffect();
		g_kSocketSystemMgr.InProgress(true);
		g_kSocketSystemMgr.StartTime(g_pkApp->GetAccumTime());
	}
	return true;
	
	//return g_kSocketSystemMgr.SendReqSocketSystem(bIsTrueSend);
}

int lwUIItemSocketSystem::GetUpgradeCreateNeedMoney()
{
	return g_kSocketSystemMgr.GetUpgradeCreateNeedMoney();
}

int lwUIItemSocketSystem::GetUpgradeRemoveNeedMoney()
{
	return g_kSocketSystemMgr.GetUpgradeRemoveNeedMoney();
}

int lwUIItemSocketSystem::GetUpgradeResetNeedMoney()
{
	return g_kSocketSystemMgr.GetUpgradeResetNeedMoney();
}
int lwUIItemSocketSystem::GetUpgradeExtractionNeedMoney()
{
	return g_kSocketSystemMgr.GetUpgradeExtractionNeedMoney();
}

void lwUIItemSocketSystem::CallComfirmMessageBox( bool const bIsModal )
{
	g_kSocketSystemMgr.CallComfirmMessageBox( bIsModal );
}

bool lwUIItemSocketSystem::GetSocketSystemEmpty()
{
	return g_kSocketSystemMgr.GetSocketSystemEmpty();
}

bool lwUIItemSocketSystem::IsSocketItemEmpty() const
{
	return g_kSocketSystemMgr.IsSocketItemEmpty();
}

void lwUIItemSocketSystem::SetAttachElement(int iType)
{
	g_kSocketSystemMgr.AttachElement((EPropertyType)iType);
}

void Recv_PT_M_C_ANS_GEN_SOCKET(BM::Stream* pkPacket)
{
	HRESULT hResult;
	bool bRet = false;

	pkPacket->Pop(hResult);	
	
	int iWarnMessage = 0;
	
	if( S_OK == hResult )
	{
		pkPacket->Pop(bRet);
		if( true == bRet )
		{
			iWarnMessage = 790400;	// 소켓 성공(뚫림)
		}
		else
		{
			iWarnMessage = 790150;	// 소켓 성공 했지만, 실패(막힘)

			/*std::wstring kTempMsg = TTW(790150);
			if( !kTempMsg.empty() )
			{
				PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#NUM#", BM::vstring(g_kSocketSystemMgr.GetSelectOrderMenu()), kTempMsg);
				::Notice_Show(kTempMsg, EL_Warning, true);
			}*/
		}
	}
	else
	{
		iWarnMessage = g_kSocketSystemMgr.GetSocketSystemErrorMsg(hResult);
	}

	g_kSocketSystemMgr.StartTime(g_pkApp->GetAccumTime());	//결과 이펙트용

	char szName[100] = "EnchantFail";
	ENoticeLevel eLevel = EL_Warning;

	if( true == bRet )
	{
		sprintf(szName, "EnchantSuccess");
		eLevel = EL_Normal;
	}			

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
	if( 0 < iWarnMessage )
	{
		Notice_Show_ByTextTableNo(iWarnMessage, eLevel);
	}
	g_kSocketSystemMgr.InProgress(false);
	ClearSocketSystemUI();
	g_kSocketSystemMgr.Clear();
	SocketSystemUpdateUI( ( S_OK == hResult ) ? 2 : 1  );
}

void Recv_PT_M_C_ANS_REMOVE_MONSTERCARD(BM::Stream* pkPacket)
{
	HRESULT hResult;

	pkPacket->Pop(hResult);	

	int iWarnMessage = 0;

	if( S_OK == hResult )
	{
		//iWarnMessage = 790135;

		std::wstring kTempMsg = TTW(790159);
		if( !kTempMsg.empty() )
		{
			PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#NUM#", BM::vstring(g_kSocketSystemMgr.GetSelectOrderMenu()), kTempMsg);
			::Notice_Show(kTempMsg, EL_Normal, true);
		}
	}
	else
	{
		iWarnMessage = g_kSocketSystemMgr.GetSocketSystemErrorMsg(hResult);
	}

	g_kSocketSystemMgr.StartTime(g_pkApp->GetAccumTime());	//결과 이펙트용

	char szName[100] = "EnchantFail";
	ENoticeLevel eLevel = EL_Warning;

	if( S_OK == hResult )
	{
		sprintf(szName, "EnchantSuccess");
		eLevel = EL_Normal;
	}			

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
	if( 0 < iWarnMessage )
	{
		Notice_Show_ByTextTableNo(iWarnMessage, eLevel);
	}
	g_kSocketSystemMgr.InProgress(false);
	ClearSocketSystemUI();
	g_kSocketSystemMgr.Clear();
	SocketSystemUpdateUI( ( S_OK == hResult ) ? 1 : 2  );
}

void Recv_PT_M_C_ANS_RESET_MONSTERCARD(BM::Stream* pkPacket)
{
	HRESULT hResult;

	pkPacket->Pop(hResult);	
	
	int iWarnMessage = 0;
	
	if( S_OK == hResult )
	{
		iWarnMessage = 790136;
	}
	else
	{
		iWarnMessage = g_kSocketSystemMgr.GetSocketSystemErrorMsg(hResult);
	}

	g_kSocketSystemMgr.StartTime(g_pkApp->GetAccumTime());	//결과 이펙트용

	char szName[100] = "EnchantFail";
	ENoticeLevel eLevel = EL_Warning;

	if( S_OK == hResult )
	{
		sprintf(szName, "EnchantSuccess");
		eLevel = EL_Normal;
	}			

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
	Notice_Show_ByTextTableNo(iWarnMessage, eLevel);
	g_kSocketSystemMgr.InProgress(false);
	ClearSocketSystemUI();
	g_kSocketSystemMgr.Clear();
	SocketSystemUpdateUI( ( S_OK == hResult ) ? 1 : 2  );
}

void Recv_PT_M_C_ANS_EXTRACTION_MONSTERCARD(BM::Stream* pkPacket)
{
	HRESULT hResult;
	int iCardItemNo = 0;
	pkPacket->Pop(hResult);	
	pkPacket->Pop(iCardItemNo);	

	int iWarnMessage = 0;
	
	if( S_OK == hResult )
	{
		iWarnMessage = 790173;
	}
	else
	{
		iWarnMessage = g_kSocketSystemMgr.GetSocketSystemErrorMsg(hResult);
	}

	g_kSocketSystemMgr.StartTime(g_pkApp->GetAccumTime());	//결과 이펙트용

	char szName[100] = "ExtractionFail";
	ENoticeLevel eLevel = EL_Warning;

	if( S_OK == hResult )
	{
		sprintf(szName, "ExtractionSuccess");
		std::wstring kTempStr;
		wchar_t const* pkItemName = NULL;
		if( GetDefString(iCardItemNo, pkItemName) )
		{
			if( FormatTTW(kTempStr, 460032, pkItemName) )
			{
				g_kChatMgrClient.AddLogMessage(SChatLog(CT_ITEM), kTempStr, true);
			}
		}
		eLevel = EL_Normal;
	}			

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
	Notice_Show_ByTextTableNo(iWarnMessage, eLevel);
	g_kSocketSystemMgr.InProgress(false);
	ClearSocketSystemUI();
	g_kSocketSystemMgr.Clear();
	SocketSystemUpdateUI( ( S_OK == hResult ) ? 1 : 2  );
}

void Recv_PT_M_C_ANS_SET_MONSTERCARD(BM::Stream* pkPacket)
{
	HRESULT hResult;
	bool bSuccess =false ;
	DWORD CardItemNo = 0;
	pkPacket->Pop(hResult);	
	pkPacket->Pop(bSuccess);
	pkPacket->Pop(CardItemNo);
	
	int iWarnMessage = 0;
	
	bool MonsterCard = true;
	if( CardItemNo )
	{
		GET_DEF(CItemDefMgr, ItemDefMgr);
		CItemDef const *pItemDef = ItemDefMgr.GetDef(CardItemNo);
		if( pItemDef )
		{
			if( !pItemDef->GetAbil(AT_OLD_MONSTERCARD) )
			{// 이 어빌이 없으면 소켓카드다.
				MonsterCard = false;
			}
		}
	}

	if(S_OK == hResult)
	{
		if(bSuccess)
		{
			if( MonsterCard )
			{
				iWarnMessage = 790419;
			}
			else
			{
				iWarnMessage = 790396;
			}
		}
		else
		{
			if( MonsterCard )
			{
				iWarnMessage = 790427;
			}
			else
			{
				iWarnMessage = 790397;
			}
		}
	}
	else
	{
		iWarnMessage = g_kSocketSystemMgr.GetSocketSystemErrorMsg(hResult);		
	}

	g_kSocketSystemMgr.StartTime(g_pkApp->GetAccumTime());	//결과 이펙트용

	char szName[100] = "EnchantFail";
	ENoticeLevel eLevel = EL_Warning;

	if(bSuccess && S_OK == hResult)
	{
		sprintf(szName, "EnchantSuccess");
		eLevel = EL_Normal;
	}

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
	Notice_Show_ByTextTableNo(iWarnMessage, eLevel);
	lwCloseUI("SFRM_MONSTER_CARD");
}

int lwUIItemSocketSystem::CheckOK()
{
	if (g_kSocketSystemMgr.IsChangedGuid())
	{
		return 790113;
	}
	__int64 iNeedMoney = 0;

	switch( GetMenuType() )
	{
	case PgItemSocketSystemMgr::SIM_CREATE:
		{
			iNeedMoney = g_kSocketSystemMgr.GetUpgradeCreateNeedMoney();
		}break;
	case PgItemSocketSystemMgr::SIM_REMOVE:		
		{
			iNeedMoney = g_kSocketSystemMgr.GetUpgradeRemoveNeedMoney();
		}break;
	case PgItemSocketSystemMgr::SIM_RESET:
		{
			iNeedMoney = g_kSocketSystemMgr.GetUpgradeResetNeedMoney();
		}break;
	case PgItemSocketSystemMgr::SIM_EXTRACTION:
		{
			iNeedMoney = g_kSocketSystemMgr.GetUpgradeExtractionNeedMoney();
		}break;
	default:
		{
			return 790113;
		}break;
	}

	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(pkUnit)
	{
		__int64 const iMoney = pkUnit->GetAbil64(AT_MONEY);
		if (iNeedMoney > iMoney)
		{
			return 790112;
		}
	}

	int const iItemNo = g_kSocketSystemMgr.CheckNeedItem();
	if( iItemNo  )
	{
		if( g_kSocketSystemMgr.DisplayNeedItemText(iItemNo) )
		{
			return 790111;
		}		
	}
	return 0;
}

int const lwUIItemSocketSystem::GetNowNeedItemCount(int const iNeed) const
{
	return g_kSocketSystemMgr.GetNowNeedItemCount(iNeed);
}

void lwUIItemSocketSystem::OnDisplay()
{
	g_kSocketSystemMgr.OnDisplay();
}

void lwUIItemSocketSystem::OnTick(lwUIWnd kWnd)
{
	if (g_kSocketSystemMgr.InProgress() && SOCKET_PROGRESS_TIME < g_pkApp->GetAccumTime() - g_kSocketSystemMgr.StartTime())
	{
		g_kSocketSystemMgr.InProgress(false);	//시간 다 됬으니까

		bool bCreate = false;
		if( true == g_kSocketSystemMgr.GetSocketSystemEmpty() )
		{
			bCreate = false;
		}
		else
		{
			bCreate = true;
		}

		g_kSocketSystemMgr.SendReqSocketSystem(bCreate);
	}
	else if (!kWnd.IsNil())
	{
		g_kSocketSystemMgr.OnTick(kWnd.GetTotalLocation()());
	}
}

bool lwUIItemSocketSystem::InProgress()
{
	return g_kSocketSystemMgr.InProgress();
}

lwWString lwUIItemSocketSystem::GetExplane()
{
	return lwWString((std::wstring const &)g_kSocketSystemMgr.GetExplane());
}

int lwUIItemSocketSystem::GetSoulCreateCount()
{
	return g_kSocketSystemMgr.GetSoulCreateCount();
}

int lwUIItemSocketSystem::GetSoulRemoveCount()
{
	return g_kSocketSystemMgr.GetSoulRemoveCount();
}

int lwUIItemSocketSystem::GetSoulResetCount()
{
	return g_kSocketSystemMgr.GetSoulResetCount();
}

int lwUIItemSocketSystem::GetExtractionCount()
{
	return g_kSocketSystemMgr.GetExtractionCount();
}

int lwUIItemSocketSystem::GetHaveCount()
{
	return g_kSocketSystemMgr.GetHaveCount();
}

void lwUIItemSocketSystem::SetMenuType(int iMenu)
{
	return g_kSocketSystemMgr.SetMenuType(iMenu);
}

int lwUIItemSocketSystem::GetMenuType()
{
	return g_kSocketSystemMgr.GetMenuType();
}

void lwUIItemSocketSystem::CheckMenuSocketItem()
{
	g_kSocketSystemMgr.CheckMenuSocketItem();
}

void lwUIItemSocketSystem::SetSelectOrderMenu(int iSelectOrderMenu)
{
	g_kSocketSystemMgr.SetSelectOrderMenu(iSelectOrderMenu);
}

int lwUIItemSocketSystem::GetSelectOrderMenu()
{
	return g_kSocketSystemMgr.GetSelectOrderMenu();
}

int lwUIItemSocketSystem::GetMenuTypeOrder()
{
	return g_kSocketSystemMgr.GetMenuTypeOrder();
}

bool lwUIItemSocketSystem::GetSelectOrderMenuCheck(int iSelectMenu)
{
	return g_kSocketSystemMgr.GetSelectOrderMenuCheck(iSelectMenu);
}

bool lwUIItemSocketSystem::IsSelectOrderMenuIndex(int iIndex)
{
	return g_kSocketSystemMgr.IsSelectOrderMenuIndex(iIndex);
}

void lwUIItemSocketSystem::DisplaySocketRemoveText(int iIndex)
{
	g_kSocketSystemMgr.DisplaySocketRemoveText(iIndex);
}

void lwUIItemSocketSystem::SetSocketItemMenuUIUpdate()
{
	g_kSocketSystemMgr.SetSocketItemMenuUIUpdate();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PgItemSocketSystemMgr
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char const UIMODEL_SOCKET_EFFECT_NAME[] = "ef_UImix_01";
char const UIMODEL_SOCKET_EFFECT_NIF_NAME[] = "ef_UImix_01_NIF";
char const UIMODEL_SOCKET_EFFECT_PATH[] = "../Data/5_Effect/4_UI/ef_UImix_01_spin.nif";
char const UIMODEL_SOCKET_EFFECT_NIF_SUCC_NAME[] = "ef_UImix_01_SUCC_NIF";
char const UIMODEL_SOCKET_EFFECT_SUCC_PATH[] = "../Data/5_Effect/4_UI/ef_UImix_02_succ.nif";
char const UIMODEL_SOCKET_EFFECT_NIF_FAIL_NAME[] = "ef_UImix_01_FAIL_NIF";
char const UIMODEL_SOCKET_EFFECT_FAIL_PATH[] = "../Data/5_Effect/4_UI/ef_UImix_03_fail.nif";

const POINT2 UIMODEL_SOCKET_POS(0, 0);

HRESULT PgItemSocketSystemMgr::CheckSocketBundle(PgBase_Item const& kItem)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( !pItemDef )
	{
		return SCE_FALSE;
	}

	if( !pItemDef->CanEquip() )
	{
		return SCE_FALSE;
	}

	if( (ICMET_Cant_GenSocket & pItemDef->GetAbil(AT_ATTRIBUTE)) == ICMET_Cant_GenSocket )
	{
		return SCE_FALSE;
	}

	E_ITEM_GRADE const eItemGrade = GetItemGrade(kItem);
	
	switch(eItemGrade)
	{
	case IG_SEAL:
		{
			return SCE_SEAL;
		}break;
	case IG_CURSE:
		{
			return SCE_CURSE;
		}
#if 0
	case IG_GOD:
		{
			return SEC_GOD;
		}break;
#endif
	}
	return SCE_OK;
}

PgItemSocketSystemMgr::PgItemSocketSystemMgr()
{
	InitUIModel();
	Clear();
	StartTime(0);
	RecentResult(ISOC_NONE);
	AttachElement(E_PPTY_NONE);
	MyElement(E_PPTY_NONE);
}

void PgItemSocketSystemMgr::InitUIModel()
{
	m_pkWndUIModel = NULL;
	m_pkWndUIModel_Result = NULL;

	g_kUIScene.InitRenderModel(UIMODEL_SOCKET_EFFECT_NAME, POINT2(230,230), UIMODEL_SOCKET_POS, false);
	m_pkWndUIModel = g_kUIScene.FindUIModel(UIMODEL_SOCKET_EFFECT_NAME);
	if (m_pkWndUIModel)
	{
		m_pkWndUIModel->AddNIF(UIMODEL_SOCKET_EFFECT_NIF_NAME, g_kNifMan.GetNif(UIMODEL_SOCKET_EFFECT_PATH), false, true);
		m_pkWndUIModel->AddNIF(UIMODEL_SOCKET_EFFECT_NIF_SUCC_NAME, g_kNifMan.GetNif(UIMODEL_SOCKET_EFFECT_SUCC_PATH), false, true);
		m_pkWndUIModel->AddNIF(UIMODEL_SOCKET_EFFECT_NIF_FAIL_NAME, g_kNifMan.GetNif(UIMODEL_SOCKET_EFFECT_FAIL_PATH), false, true);
		m_pkWndUIModel->SetCameraZoomMinMax(-300, 300);
		m_pkWndUIModel->CameraZoom(210.0f);
	}
}

void PgItemSocketSystemMgr::RunProgressEffect(bool bOn)
{
	if (m_pkWndUIModel)
	{
		m_pkWndUIModel->SetNIFEnableUpdate(UIMODEL_SOCKET_EFFECT_NIF_NAME,bOn);
		if (bOn)
		{
			m_pkWndUIModel->SetEnableUpdate(bOn);
			m_pkWndUIModel->ResetNIFAnimation(UIMODEL_SOCKET_EFFECT_NIF_NAME);
			m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_SOCKET_POS);
			m_pkWndUIModel->SetNIFEnableUpdate(UIMODEL_SOCKET_EFFECT_NIF_SUCC_NAME,!bOn);
			m_pkWndUIModel->SetNIFEnableUpdate(UIMODEL_SOCKET_EFFECT_NIF_FAIL_NAME,!bOn);
		}
	}
}

void PgItemSocketSystemMgr::RecentResult(EItemSocketSystemResult const & eResult)
{
	bool bSucc = false;
	bool bFail = false;
	m_RecentResult = eResult;
	if (ISOC_SUCCESS==eResult)
	{
		bSucc = true;
		m_pkWndUIModel->SetNIFEnableUpdate(UIMODEL_SOCKET_EFFECT_NIF_SUCC_NAME, bSucc);
		m_pkWndUIModel->ResetNIFAnimation(UIMODEL_SOCKET_EFFECT_NIF_SUCC_NAME);
	}
	else if (ISOC_FAIL==eResult || ISOC_FAIL_AND_BROKEN==eResult)
	{
		bFail = true;
		m_pkWndUIModel->SetNIFEnableUpdate(UIMODEL_SOCKET_EFFECT_NIF_FAIL_NAME, bFail);
		m_pkWndUIModel->ResetNIFAnimation(UIMODEL_SOCKET_EFFECT_NIF_FAIL_NAME);
	}
	
	m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_SOCKET_POS);
}

void PgItemSocketSystemMgr::OnDisplay()
{
	if (InProgress() || ISOC_NONE!=RecentResult())
	{
		if(m_pkWndUIModel)
		{
			g_kUIScene.AddToDrawListRenderModel(UIMODEL_SOCKET_EFFECT_NAME);
		}
	}
}

void PgItemSocketSystemMgr::OnTick(POINT2 kPt)
{
	if (InProgress())
	{
		if(m_pkWndUIModel)
		{
			m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), kPt);
		}
	}
	else
	{
		if (UIMODEL_SOCKET_EFFECT_RESULT_TIME < g_pkApp->GetAccumTime() - g_kSocketSystemMgr.StartTime())
		{
			RecentResult(ISOC_NONE);
		}
		else if (ISOC_NONE!=RecentResult())
		{
			if(m_pkWndUIModel)
			{
				m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), kPt);
			}
		}
	}
}

void PgItemSocketSystemMgr::Clear(bool const bAllClear)
{
	m_kResultItem.Clear();
	m_guidSrcItem = BM::GUID();
	m_kSrcItemPos.Clear();
	m_kInsureItemPos.Clear();
	m_kProbabilityItemPos.Clear();
	m_kItem.Clear();
	m_iExtractionInGameItemNameNum = 0;
	m_iHaveCashItemExtractionNum = 0;
	m_iUserHaveCashItemCount = 0;
	m_iNeedCashItemExtractionCount= 0;
	m_iNeedItemExtractionCount = 0;
	m_iNeedItemExtractionCountTemp = 0;
	m_iHaveItemCount = 0;
	m_iCardItemNo = 0;
	m_iHaveIngameItemCount = 0;


	for (int i = 0; i < SIT_INSUR_ITEM+1; ++i)
	{
		m_kItemArray[i].Init();
	}

	InProgress(false);
	ClearSocketSystemUI();
	m_kExplane = L"";
	if( bAllClear )
	{
		m_kNpcGuid.Clear();
	}
}

int PgItemSocketSystemMgr::CallComfirmMessageBox( bool const bIsModal )
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return 0;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return 0;}

	if(S_OK != pInv->GetItem(m_kSrcItemPos, m_kItem))
	{
		return 0;
	}

	std::wstring kTempMsg = _T("");

	if(!m_kItem.IsEmpty())
	{
		E_ITEM_GRADE const eItemGrade = GetItemGrade(m_kItem);

		kTempMsg = TTW(790117);

		SEnchantInfo kEnchant = m_kItem.EnchantInfo();
		bool bRet = true; //kEnchant.EanbleGenSocket(eItemGrade);
		if( true == bRet )
		{
			if( true == GetSocketSystemEmpty() )
			{
				if( false == IsProbabilityItemPos() )
				{
					kTempMsg = TTW(790148);
					if( !kTempMsg.empty() )
					{						
						int const iItemNo = m_kItemArray[SIT_PROBABILITY].iItemNo;

						GET_DEF(CItemDefMgr, kItemDefMgr);
						CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
						if(pItemDef)
						{						
							wchar_t const* pItemName = NULL;
							if( ::GetDefString(pItemDef->NameNo(), pItemName) )
							{
								PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#ITEM#", pItemName, kTempMsg);
								PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#NUM#", BM::vstring(GetSelectOrderMenu()), kTempMsg);
							}
							else
							{
								return 0;
							}
						}
						else
						{
							return 0;
						}
					}
				}
				else
				{
					kTempMsg = TTW(790147);
					if( !kTempMsg.empty() )
					{
						PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#NUM#", BM::vstring(GetSelectOrderMenu()), kTempMsg);
					}
				}
			}
			else
			{
				bool bRet = false;

				switch( GetMenuType() )
				{
					case PgSocketFormula::SII_SECOND:
						{
							switch( GetSelectOrderMenu() )
							{
							case PgSocketFormula::SII_FIRST:
								{
									if( (GSS_GEN_SUCCESS == kEnchant.GenSocketState()) && (0 < kEnchant.MonsterCard()) )
									{
										bRet = true;
									}
								}break;
							case PgSocketFormula::SII_SECOND:
								{
									if( (GSS_GEN_SUCCESS == kEnchant.GenSocketState2()) && (0 < kEnchant.MonsterCard2()) )
									{
										bRet = true;
									}
								}break;
							case PgSocketFormula::SII_THIRD:
								{
									if( (GSS_GEN_SUCCESS == kEnchant.GenSocketState3()) && (0 < kEnchant.MonsterCard3()) )
									{
										bRet = true;
									}
								}break;
							default:
								{
									return 0;
								}break;
							}
						}break;
					case PgSocketFormula::SII_THIRD:
						{
							switch( GetSelectOrderMenu() )
							{
							case PgSocketFormula::SII_FIRST:
								{
									if( (GSS_GEN_FAIL == kEnchant.GenSocketState()) )
									{
										bRet = true;
									}
								}break;
							case PgSocketFormula::SII_SECOND:
								{
									if( (GSS_GEN_FAIL == kEnchant.GenSocketState2()) )
									{
										bRet = true;
									}
								}break;
							case PgSocketFormula::SII_THIRD:
								{
									if( (GSS_GEN_FAIL == kEnchant.GenSocketState3()) )
									{
										bRet = true;
									}
								}break;
							default:
								{
									return 0;
								}break;
							}
						}break;
					case PgSocketFormula::SII_FOURTH:
						{
							switch( GetExtractionSocketItemOrder() )
							{
							case PgSocketFormula::SII_FIRST:
								{
									if( (GSS_GEN_SUCCESS == kEnchant.GenSocketState()) && (0 < kEnchant.MonsterCard()) )
									{
										bRet = true;
									}
								}break;
							case PgSocketFormula::SII_SECOND:
								{
									if( (GSS_GEN_SUCCESS == kEnchant.GenSocketState2()) && (0 < kEnchant.MonsterCard2()) )
									{
										bRet = true;
									}
								}break;
							case PgSocketFormula::SII_THIRD:
								{
									if( (GSS_GEN_SUCCESS == kEnchant.GenSocketState3()) && (0 < kEnchant.MonsterCard3()) )
									{
										bRet = true;
									}
								}break;
							default:
								{
									return 0;
								}break;
							}
						}break;
					default:
						{
							return 0;
						}break;
				}				


				if( false == bRet )
				{
					if (!m_kItem.IsEmpty())
					{
						m_kItem.Clear();
						ClearSocketSystemUI();
						Clear();
					}
					lwAddWarnDataTT(790422);
					return 0;
				}
				else
				{
					switch( GetMenuType() )
					{
						case PgSocketFormula::SII_SECOND:
							{
								kTempMsg = TTW(790145);
							}break;
						case PgSocketFormula::SII_THIRD:
							{
								kTempMsg = TTW(790146);
							}break;
						case PgSocketFormula::SII_FOURTH:
							{
								kTempMsg = TTW(790169);
							}break;
						default:
							{
								return 0;
							}break;
					}
					if( !kTempMsg.empty() )
					{
						PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#NUM#", BM::vstring(GetSelectOrderMenu()), kTempMsg);
					}
				}
			}
		}
		else
		{
			goto __ERROR;
		}
		
		XUI::CXUI_Wnd *pWnd =  XUIMgr.Call(_T("SFRM_MSG_SOCKETSYSTEM_REFINE"), bIsModal );
		if(pWnd)
		{
			XUI::CXUI_Wnd *pColorWnd =  pWnd->GetControl(_T("SFRM_COLOR"));
			if(pColorWnd)
			{
				XUI::CXUI_Wnd *pSdwWnd =  pColorWnd->GetControl(_T("SFR_SDW"));
				if(pSdwWnd)
				{//커스텀 데이터 넘겨줘.
					pSdwWnd->Text(kTempMsg);
					return 0;
				}
			}
		}

		assert(pWnd);
		return 0;
	}
__ERROR:
	{
		if (!m_kItem.IsEmpty())
		{
			m_kItem.Clear();
			Clear();
		}		
		lwAddWarnDataTT(790114);
		return 0;
	}

	return 0;
}

int PgItemSocketSystemMgr::GetCreateSocketItemOrder()
{
	m_iSocket_CreateOrder = PgSocketFormula::GetCreateSocketItemOrder(m_kItem);

	return m_iSocket_CreateOrder;
}

void PgItemSocketSystemMgr::SetSelectOrderMenu(int iSelectOrderMenu)
{
	m_iSelectOrderMenu = iSelectOrderMenu;
}

int PgItemSocketSystemMgr::GetSelectOrderMenu()
{
	return m_iSelectOrderMenu;
}

int PgItemSocketSystemMgr::GetRemoveSocketItemOrder()
{
	m_iSocket_RemoveOrder = PgSocketFormula::GetRemoveSocketItemOrder(m_kItem, GetSelectOrderMenu());

	return m_iSocket_RemoveOrder;
}

int PgItemSocketSystemMgr::GetResetSocketItemOrder()
{
	m_iSocket_ResetOrder = PgSocketFormula::GetResetSocketItemOrder(m_kItem);

	return m_iSocket_ResetOrder;
}
int PgItemSocketSystemMgr::GetExtractionSocketItemOrder()
{
	int iCheckState = 0;
	int iGenSocketState = 0;
	int iGenSocketCard = 0;
	for(int i=PgSocketFormula::SII_FIRST; i<=PgSocketFormula::SII_THIRD; ++i)
	{
		if( true == PgSocketFormula::GetEnchantInfoIndex(m_kItem, i, iGenSocketState, iGenSocketCard) )
		{
			if( (GSS_GEN_SUCCESS == iGenSocketState) && (0 != iGenSocketCard) )
			{
				iCheckState = i;
				if(iCheckState == m_iSelectOrderMenu)
				{
					return i;
				}

			}
		}
	}

	return iCheckState;
}

void PgItemSocketSystemMgr::SetSocketItemMenuUIUpdate()
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( PgSocketSystemUtil::kSocketSystemUIName );
	if (!pkWnd)
	{
		return;
	}
	XUI::CXUI_CheckButton* pkBtn1 = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"BTN_1"));
	XUI::CXUI_CheckButton* pkBtn2 = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"BTN_2"));
	XUI::CXUI_CheckButton* pkBtn3 = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"BTN_3"));

	if( !pkBtn1 )
	{
		return;
	}
	if( !pkBtn2 )
	{
		return;
	}
	if( !pkBtn3 )
	{
		return;
	}


/*	{
		g_kSocketSystemMgr.UpdateSoulItemView();
		lua_tinker::call<void, lwUIWnd, int>("SocketSystemUIUpdate", lwUIWnd(pkSrc), iCreate);
	}*/

	if(m_kItem.IsEmpty())
	{
		return;
	}

	int iCheckState = 0;
	int iSocketIndex = 0;
	SEnchantInfo const& kEhtInfo = m_kItem.EnchantInfo();

	switch( GetMenuType() )
	{
	case SIM_CREATE:
		{
			iSocketIndex = PgSocketFormula::GetCreateSocketItemOrder(m_kItem);

			switch( iSocketIndex )
			{
			case PgSocketFormula::SII_FIRST:
				{
					pkBtn1->Check(true);
					pkBtn1->Visible(true);
					pkBtn2->Visible(false);
					pkBtn3->Visible(false);
				}break;
			case PgSocketFormula::SII_SECOND:
				{
					pkBtn2->Check(true);
					pkBtn1->Visible(false);
					pkBtn2->Visible(true);
					pkBtn3->Visible(false);
				}break;
			case PgSocketFormula::SII_THIRD:
				{
					pkBtn3->Check(true);
					pkBtn1->Visible(false);
					pkBtn2->Visible(false);
					pkBtn3->Visible(true);
				}break;
			default:
				{
					pkBtn1->Visible(false);
					pkBtn2->Visible(false);
					pkBtn3->Visible(false);
				}break;
			}
		}break;
	case SIM_REMOVE:
		{			
			XUI::CXUI_Wnd* pkTitle = pkWnd->GetControl(L"SFRM_EXPLANE1");
			if( pkTitle )
			{
				pkTitle->Text(TTW(790156));
			}

			for(int i=PgSocketFormula::SII_FIRST; i<=PgSocketFormula::SII_THIRD; ++i)
			{
				int iGenSocketState = 0;
				int iGenSocketCard = 0;

				if( true == PgSocketFormula::GetEnchantInfoIndex(m_kItem, i, iGenSocketState, iGenSocketCard) )
				{
					bool bVisible = false;
					if( (GSS_GEN_SUCCESS == iGenSocketState) && (0 != iGenSocketCard) )
					{
						bVisible = true;
						if( 0 == iCheckState )
						{
							iCheckState = i;
						}						
					}

					switch( i )
					{
					case PgSocketFormula::SII_FIRST:
						{
							pkBtn1->Visible(bVisible);
						}break;
					case PgSocketFormula::SII_SECOND:
						{
							pkBtn2->Visible(bVisible);
						}break;
					case PgSocketFormula::SII_THIRD:
						{
							pkBtn3->Visible(bVisible);
						}break;
					default:
						{
							pkBtn1->Visible(false);
							pkBtn2->Visible(false);
							pkBtn3->Visible(false);
						}break;
					}
				}
				else
				{
					pkBtn1->Visible(false);
					pkBtn2->Visible(false);
					pkBtn3->Visible(false);
				}
			}

			switch( iCheckState )
			{
			case PgSocketFormula::SII_FIRST:
				{
					pkBtn1->Check(true);
				}break;
			case PgSocketFormula::SII_SECOND:
				{
					pkBtn2->Check(true);
				}break;
			case PgSocketFormula::SII_THIRD:
				{
					pkBtn3->Check(true);
				}break;
			default:
				{
				}break;
			}
		}break;
	case SIM_RESET:
		{
			iSocketIndex = PgSocketFormula::GetResetSocketItemOrder(m_kItem);

			switch( iSocketIndex )
			{
			case PgSocketFormula::SII_FIRST:
				{
					pkBtn1->Check(true);
					pkBtn1->Visible(true);
					pkBtn2->Visible(false);
					pkBtn3->Visible(false);
				}break;
			case PgSocketFormula::SII_SECOND:
				{
					pkBtn2->Check(true);
					pkBtn1->Visible(false);
					pkBtn2->Visible(true);
					pkBtn3->Visible(false);
				}break;
			case PgSocketFormula::SII_THIRD:
				{
					pkBtn3->Check(true);
					pkBtn1->Visible(false);
					pkBtn2->Visible(false);
					pkBtn3->Visible(true);
				}break;
			default:
				{
					pkBtn1->Visible(false);
					pkBtn2->Visible(false);
					pkBtn3->Visible(false);
				}break;
			}
		}break;
	case SIM_EXTRACTION:
		{
			XUI::CXUI_Wnd* pkTitle = pkWnd->GetControl(L"SFRM_EXPLANE1");
			if( pkTitle )
			{
				pkTitle->Text(TTW(790165));
			}

			for(int i=PgSocketFormula::SII_FIRST; i<=PgSocketFormula::SII_THIRD; ++i)
			{
				int iGenSocketState = 0;
				int iGenSocketCard = 0;

				if( true == PgSocketFormula::GetEnchantInfoIndex(m_kItem, i, iGenSocketState, iGenSocketCard) )
				{
					bool bVisible = false;
					if( (GSS_GEN_SUCCESS == iGenSocketState) && (0 != iGenSocketCard) )
					{
						bVisible = true;
						iCheckState = i;
					}

					switch( i )
					{
					case PgSocketFormula::SII_FIRST:
						{
							pkBtn1->Visible(bVisible);
						}break;
					case PgSocketFormula::SII_SECOND:
						{
							pkBtn2->Visible(bVisible);
						}break;
					case PgSocketFormula::SII_THIRD:
						{
							pkBtn3->Visible(bVisible);
						}break;
					default:
						{
							pkBtn1->Visible(false);
							pkBtn2->Visible(false);
							pkBtn3->Visible(false);
						}break;
					}
				}
				else
				{
					pkBtn1->Visible(false);
					pkBtn2->Visible(false);
					pkBtn3->Visible(false);
				}
			}

			switch( iCheckState )
			{
			case PgSocketFormula::SII_FIRST:
				{
					pkBtn1->Check(true);
				}break;
			case PgSocketFormula::SII_SECOND:
				{
					pkBtn2->Check(true);
				}break;
			case PgSocketFormula::SII_THIRD:
				{
					pkBtn3->Check(true);
				}break;
			default:
				{
				}break;
			}

		}break;
	default:
		{
		}break;
	}
}

bool PgItemSocketSystemMgr::IsSelectOrderMenuIndex(int iIndex)
{
	switch( GetMenuType() )
	{
	case SIM_CREATE:
		{
		}break;
	case SIM_REMOVE:
		{
			if(!m_kItem.IsEmpty())
			{
				SEnchantInfo const& kEhtInfo = m_kItem.EnchantInfo();

				switch( iIndex )
				{
				case PgSocketFormula::SII_FIRST:
					{
						if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState()) && (0 < kEhtInfo.MonsterCard()) )
						{
							return true;
						}
					}break;
				case PgSocketFormula::SII_SECOND:
					{
						if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState2()) && (0 < kEhtInfo.MonsterCard2()) )
						{
							return true;
						}
					}break;
				case PgSocketFormula::SII_THIRD:
					{
						if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState3()) && (0 < kEhtInfo.MonsterCard3()) )
						{
							return true;
						}
					}break;
				default:
					{
					}break;
				}
			}
		}break;
	case SIM_RESET:
		{
		}break;
	case SIM_EXTRACTION:
		{
			if(!m_kItem.IsEmpty())
			{
				SEnchantInfo const& kEhtInfo = m_kItem.EnchantInfo();

				switch( iIndex )
				{
				case PgSocketFormula::SII_FIRST:
					{
						if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState()) && (0 < kEhtInfo.MonsterCard()) )
						{
							return true;
						}
					}break;
				case PgSocketFormula::SII_SECOND:
					{
						if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState2()) && (0 < kEhtInfo.MonsterCard2()) )
						{
							return true;
						}
					}break;
				case PgSocketFormula::SII_THIRD:
					{
						if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState3()) && (0 < kEhtInfo.MonsterCard3()) )
						{
							return true;
						}
					}break;
				default:
					{
					}break;
				}
			}
		}break;
	default:
		{
		}break;
	}

	// 선택 할수가 없다.
	return false;
}

bool PgItemSocketSystemMgr::GetSelectOrderMenuCheck(int iSelectMenu)
{
	switch( GetMenuType() )
	{
	case SIM_CREATE:
	case SIM_REMOVE:
	case SIM_EXTRACTION:
		{
			if( 0 < PgSocketFormula::GetRemoveSocketItemOrder(m_kItem, iSelectMenu) )
			{
				lua_tinker::call<void, lwUIWnd, int>("SetSocketIndexSelect", lwUIWnd(), iSelectMenu);
				
				SetSelectOrderMenu(iSelectMenu);
				return true;
			}
		}break;
	case SIM_RESET:
		{
			// 자동 선택
		}break;
	default:
		{
		}break;
	}

	return false;
}

__int64 PgItemSocketSystemMgr::GetUpgradeCreateNeedMoney()
{
	if(!m_kItem.IsEmpty())
	{
		return PgSocketFormula::GetCreateNeedCost(m_kItem, GetCreateSocketItemOrder());
	}
	return 0i64;
}

__int64 PgItemSocketSystemMgr::GetUpgradeRemoveNeedMoney()
{
	if(!m_kItem.IsEmpty())
	{
		return PgSocketFormula::GetRemoveNeedCost(m_kItem, GetRemoveSocketItemOrder());
	}
	return 0i64;
}

__int64 PgItemSocketSystemMgr::GetUpgradeResetNeedMoney()
{
	if(!m_kItem.IsEmpty())
	{
		return PgSocketFormula::GetRemoveNeedCost(m_kItem, GetResetSocketItemOrder());
	}
	return 0i64;
}
__int64 PgItemSocketSystemMgr::GetUpgradeExtractionNeedMoney()
{
	if(!m_kItem.IsEmpty())
	{
		return (0, 0);
	}
	return 0i64;
}

bool PgItemSocketSystemMgr::GetSocketSystemEmpty()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	if(S_OK != pInv->GetItem(m_kSrcItemPos, m_kItem))
	{
		return false;
	}

	if(!m_kItem.IsEmpty())
	{
		SEnchantInfo const& kEhtInfo = m_kItem.EnchantInfo();


		switch( GetMenuType() )
		{
		case SIM_CREATE:
			{
				if( (GSS_GEN_NONE == kEhtInfo.GenSocketState()) || (GSS_GEN_NONE == kEhtInfo.GenSocketState2()) || (GSS_GEN_NONE == kEhtInfo.GenSocketState3()) )
				{
					return true;
				}
			}break;
		case SIM_REMOVE:
			{
			}break;
		case SIM_RESET:
			{
			}break;
		case SIM_EXTRACTION:
			{
			}break;
		default:
			{
			}break;
		}

		/*switch( kEhtInfo.GenSocketState() )
		{
		case GSS_GEN_NONE:
			{
				return true;
			}break;
		case GSS_GEN_SUCCESS:
		case GSS_GEN_FAIL:
			{
				return false;
			}break;
		default:
			{
			}break;
		}*/
	}
	return false;
}

bool PgItemSocketSystemMgr::SetSrcItem(const SItemPos &rkItemPos)
{
	if(InProgress())
	{
		lwAddWarnDataTT(790110);
		return false;
	}
	if (rkItemPos.x && rkItemPos.y)
	{
		ClearSocketSystemUI();
		Clear();//클리어 밟아야함.
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	int iErrorNo = 790114;
	//MyElement((EPropertyType)pkPlayer->GetAbil(AT_FIVE_ELEMENT_TYPE_AT_BODY));	//내 속성
	switch(rkItemPos.x)
	{
	case KUIG_EQUIP:
	case KUIG_CASH:
		{
			if(S_OK != pInv->GetItem(rkItemPos, m_kItem))
			{
				return false;
			}

			SEnchantInfo const& kEhtInfo = m_kItem.EnchantInfo();
			if(m_kItem.ItemNo() == SOUL_ITEMNO 
			   || kEhtInfo.IsBinding()
			   )
			{// 크래프트 할 수 없음 메세지
				iErrorNo = 790114;
				goto __ERROR;
			}
			if(m_kItem.IsEmpty())
			{
				goto __ERROR;
			}

			/*if( (GSS_GEN_SUCCESS == m_kItem.EnchantInfo().GenSocketState()) && ( 0 == m_kItem.EnchantInfo().MonsterCard() ) )
			{
				iErrorNo = 790116;
				goto __ERROR;
			}*/

			switch( CheckSocketBundle(m_kItem) )
			{
			case SCE_CURSE:
				{
					if (E_PPTY_CURSE!=AttachElement())
					{
						iErrorNo = 1497;
						goto __ERROR;
					}
				}break;
			case SCE_SEAL:
				{
					iErrorNo = 1409;
					goto __ERROR;
				}break;
#if 0
			case SCE_GOD:
				{
					iErrorNo = 790116;
					goto __ERROR;
				}break;
#endif
			case SCE_OK:
				{
					/*if (!IsOnlyUseSoul())
					{
						int iType = pItemDef->GetAbil(AT_EQUIP_LIMIT);
						if (EQUIP_LIMIT_WEAPON!=iType && EQUIP_LIMIT_SHIRTS!=iType)
						{
							goto __ERROR;
						}
						if (PROPERTY_LEVEL_LIMIT <= m_kItem.EnchantInfo().AttrLv())
						{
							iErrorNo = 59006;
							goto __ERROR;
						}
					}*/

					m_guidSrcItem = m_kItem.Guid();//GUID 기록
					m_kResultItem = m_kItem;

					m_kSrcItemPos = rkItemPos;
					
					for (int i = 0; i<4; ++i)
					{
						if (!SetElementInfo(i, m_kItem, pInv, m_kItemArray))
						{
							iErrorNo = 790114;
							goto __ERROR;
						}
					}
				}break;
			default:
				{
					goto __ERROR;
				}break;
			}
		}break;
	case KUIG_FIT:
		{
			lwAddWarnDataTT(1406);
			return false;
		}break;
	case KUIG_CONSUME:
	case KUIG_ETC:
		{
			iErrorNo = 790114;
			goto __ERROR;
		}break;
	default:
		{
			
		}break;
	}

	MakeExplane(m_kItemArray);

	return true;

__ERROR:
	Clear();
	lwAddWarnDataTT(iErrorNo);
	return false;
}


void PgItemSocketSystemMgr::DisplayNeedItemIcon(int const iNeedIndex, XUI::CXUI_Wnd *pWnd)
{
	if(g_kSocketSystemMgr.GetMenuType() ==  PgItemSocketSystemMgr::SIM_EXTRACTION )
	{
		return;
	}

	if (!pWnd) { return; }
	int const iNull = 0;
	BM::vstring kString;
	//SRenderTextInfo kRenderTextInfo;
	if(m_kItem.IsEmpty()){goto __HIDE;}
	if(!m_kItem.ItemNo()){goto __HIDE;}
	if(IsChangedGuid()){goto __HIDE;}//guid 바뀌었으면 찾아가든가.

	int iItemNo = 0;
	DWORD	dwMaterialItemNo = 0;
	bool bGray = false;	//흑백으로 그릴지

	switch(iNeedIndex)
	{
	/*case SIT_SOUL:
		{
			if (IsOnlyUseSoul() && SIT_SOUL!=iNeedIndex)	//영력만 올리는거면
			{
				goto __HIDE;
			}
			bGray = !(m_kItemArray[iNeedIndex].IsOK(IsOnlyUseSoul()));
			iItemNo = CRYSTAL_ITEM_NO_BASE+(AttachElement()*10);
		} break;	//환혼*/
	case SIT_PROBABILITY:	{ bGray = (SItemPos::NullData() == m_kProbabilityItemPos)?(true):(false);	}break;	//생성
	case SIT_SET_ELEMENT:	//환원
	case SIT_INSUR_ITEM:	{ bGray = (SItemPos::NullData() == m_kInsureItemPos)?(true):(false);		}break;	//복원
	default:
		{
			goto __HIDE;
		}
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){ return; }
	m_kItemArray[iNeedIndex].iNowNum = (true == bGray)?(0):(pkPlayer->GetInven()->GetTotalCount(m_kItemArray[iNeedIndex].iItemNo));
	kString = __min(m_kItemArray[iNeedIndex].iNowNum, m_kItemArray[iNeedIndex].iNeedNum);
	if( bGray )
	{
		kString+=L"/";
		kString+=m_kItemArray[iNeedIndex].iNeedNum;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(m_kItemArray[iNeedIndex].iItemNo);
	if(pItemDef)
	{
		g_kUIScene.RenderIcon(pItemDef->ResNo(), pWnd->TotalLocation(), false, 40, 40, bGray);
	}
	
	pWnd->SetCustomData(&(m_kItemArray[iNeedIndex].iItemNo), sizeof(m_kItemArray[iNeedIndex].iItemNo));
	return;
	
__HIDE:
	{
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		pWnd->Text(std::wstring(L""));
	}
}

void PgItemSocketSystemMgr::DisplaySrcItem(XUI::CXUI_Wnd *pWnd)
{//네임 필드를 찾아서 이름 셋팅. 아이템이 없거나 하면 올리세요 라는 메세지로 셋팅.
	if (!pWnd) { return; }
	int const iNull = 0;

	std::wstring wstrName;
	POINT2 rPT;
	if(m_kItem.IsEmpty()){goto __HIDE;}
	if(!m_kItem.ItemNo()){goto __HIDE;}
	if(IsChangedGuid()){goto __HIDE;}//guid 바뀌었으면 찾아가든가.

	rPT =	pWnd->TotalLocation();

{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(m_kItem.ItemNo());
	if(pItemDef)
	{
		g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT, false );
	}

	pWnd->SetCustomData(&m_kItem.ItemNo(), sizeof(m_kItem.ItemNo()));
}

	return;
__HIDE:
	{
		/*assert(pSrcNameWnd);
		if(pSrcNameWnd)
		{
			pSrcNameWnd->Text(TTW(59001));
		}*/
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		m_kSrcItemPos.Clear();
	}
	return;
}

void PgItemSocketSystemMgr::DisplayResultItem(XUI::CXUI_Wnd *pWnd)
{
	XUI::CXUI_Wnd *pFormWnd = NULL;
	XUI::CXUI_Wnd *pShadowWnd = NULL;
	XUI::CXUI_Wnd *pSrcNameWnd = NULL;

	int const iNull = 0;

	if (!pWnd)
	{
		return;
	}
	pFormWnd = pWnd->Parent();
	assert(pFormWnd);
	if (!pFormWnd)
	{
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		return;
	}
	if(pFormWnd)
	{
		pShadowWnd = pFormWnd->Parent();
		assert(pShadowWnd);
		if(pShadowWnd)
		{
			pSrcNameWnd = pShadowWnd->GetControl(_T("SFRM_DEST_NAME"));
		}
		else
		{
			pWnd->SetCustomData(&iNull, sizeof(iNull));
			return;
		}
	}

	std::wstring wstrName;
	POINT2 rPT;
	if(m_kItem.IsEmpty()){goto __HIDE;}
	if(!m_kItem.ItemNo()){goto __HIDE;}
	if(IsChangedGuid()){goto __HIDE;}//guid 바뀌었으면 찾아가든가.
	if(!m_kResultItem.ItemNo()){goto __HIDE;}

//		pWnd->Visible(true);

	rPT = pWnd->TotalLocation();
	int const iItemNo = m_kResultItem.ItemNo();
	bool bQVisible = true;
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);

		if(pItemDef)
		{
			g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT, false );
			bQVisible = false;
		}
	}

	MakeItemName(iItemNo, m_kResultItem.EnchantInfo(), wstrName);
	pWnd->SetCustomData(&iItemNo, sizeof(iItemNo));
	
	XUI::CXUI_Wnd* pkQ = pWnd->GetControl(_T("IMG_Q"));
	if (pkQ)
	{
		pkQ->Visible(bQVisible);
		if (bQVisible)
		{
			int const iNull = 0;
			pWnd->SetCustomData(&iNull , sizeof(iNull ));
		}
	}


	assert(pSrcNameWnd);
	if(pSrcNameWnd)
	{
		pSrcNameWnd->Text(wstrName);
	}
	return;
__HIDE:
	{
		assert(pSrcNameWnd);
		if(pSrcNameWnd)
		{
			pSrcNameWnd->Text(_T(""));
			pWnd->SetCustomData(NULL, sizeof(iItemNo));
		}
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		XUI::CXUI_Wnd* pkQ = pWnd->GetControl(_T("IMG_Q"));
		if (pkQ)
		{
			pkQ->Visible(false);
		}
		m_kResultItem.Clear();
	}
}

bool PgItemSocketSystemMgr::SendReqSocketSystem(bool bIsTrueSend)
{
	if(m_kItem.IsEmpty()){return false;}
	if(!m_kItem.ItemNo()){return false;}
	if(IsChangedGuid()){return false;}//guid 바뀌었으면 찾아가든가.

	int iMenu = GetMenuType();

	if( SIM_CREATE == iMenu)//소켓 제작
	{
		BM::Stream kPacket(PT_C_M_REQ_GEN_SOCKET);
		kPacket.Push(m_kNpcGuid);
		kPacket.Push(m_kSrcItemPos);
		kPacket.Push(m_kProbabilityItemPos);
		kPacket.Push(GetCreateSocketItemOrder());
		NETWORK_SEND(kPacket)
	}
	else if( SIM_REMOVE == iMenu) // 소켓 환원
	{
		BM::Stream kPacket(PT_C_M_REQ_REMOVE_MONSTERCARD);
		kPacket.Push(m_kNpcGuid);
		kPacket.Push(m_kInsureItemPos);
		kPacket.Push(m_kSrcItemPos);
		kPacket.Push(GetRemoveSocketItemOrder());
		NETWORK_SEND(kPacket)
	}
	else if( SIM_RESET == iMenu) // 소켓 복원
	{
		BM::Stream kPacket(PT_C_M_REQ_RESET_MONSTERCARD);
		kPacket.Push(m_kNpcGuid);
		kPacket.Push(m_kInsureItemPos);
		kPacket.Push(m_kSrcItemPos);
		kPacket.Push(GetResetSocketItemOrder());
		NETWORK_SEND(kPacket)
	}
	else if( SIM_EXTRACTION == iMenu) // 소켓 추출
	{
		BM::Stream kPacket(PT_C_M_REQ_EXTRACTION_MONSTERCARD);
		kPacket.Push(m_kNpcGuid);
		kPacket.Push(m_kSrcItemPos);
		kPacket.Push(m_iSelectOrderMenu);//몇번째 소켓인가.
		kPacket.Push(m_iUserHaveCashItemCount); //유저가 가지고 있는 캐쉬아이템 수량
		NETWORK_SEND(kPacket)
	}

	return true;
}

int PgItemSocketSystemMgr::CheckNeedItem()
{
	for (int i = 0; i < SIT_INSUR_ITEM+1; ++i)
	{
		//if( (i == SIT_INSUR_ITEM) && (true == GetSocketSystemEmpty()) )
		if( ((i == SIT_INSUR_ITEM) || (i == SIT_SET_ELEMENT)) && (true == GetSocketSystemEmpty()) )
		{
			continue;
		}
		if( (i == SIT_PROBABILITY) )
		{
			if( false == GetSocketSystemEmpty() )
			{
				continue;
			}
			else
			{
				if( !IsProbabilityItemPos() )
				{
					continue;
				}
			}
		}	

		if (!m_kItemArray[i].IsOK())
		{
			return m_kItemArray[i].iItemNo;
		}
	}

	return 0;
}

int const PgItemSocketSystemMgr::GetNowNeedItemCount(int const iNeed) const
{
//	if (MAX_ITEM_Rarity_UPGRADE_NEED_ARRAY+1 < iNeed || 0 > iNeed )
//	{
		return 0;
//	}
//	return __min(m_kItemArray[iNeed].iNowNum, m_kItemArray[iNeed].iNeedNum);
}

bool PgItemSocketSystemMgr::IsChangedGuid() const
{
	if (m_kItem.IsEmpty())
	{
		return true;
	}
	return (m_kItem.Guid() != m_guidSrcItem);
}

bool PgItemSocketSystemMgr::IsSocketItemEmpty() const
{
	return m_kItem.IsEmpty();
}

bool PgItemSocketSystemMgr::IsInsureItemPos() const
{
	return ( SItemPos::NullData() != m_kInsureItemPos );
}

bool PgItemSocketSystemMgr::IsProbabilityItemPos() const
{
	return ( SItemPos::NullData() != m_kProbabilityItemPos );
}

bool PgItemSocketSystemMgr::SetElementInfo(int const iIndex, PgBase_Item const & rkSItem, PgInventory *pInv, SNeedItemSocketSystem* pkArray)
{
	if (!pkArray)
	{
		return false;
	}
	
	DWORD dwItemNo = 0;
	int iCount = 0;
	E_ITEM_GRADE const Grade = GetItemGrade(rkSItem);
	switch(iIndex)
	{
	case SIT_SOUL:	
		{
			dwItemNo = SOUL_ITEMNO;			
			if(SIM_EXTRACTION == GetMenuType())
			{
				dwItemNo = m_kItem.ItemNo();	
			}
		} break;
	case SIT_INSUR_ITEM:	
	case SIT_SET_ELEMENT:
		{ dwItemNo = GetSocketNeedItemNo();	} break;//{ dwItemNo = ISURANCE_ITEM_NO_BASE;	} break;
	case SIT_PROBABILITY:	
		{
			/*ContHaveItemNoCount	kItemCont;
			if( UIItemUtil::SearchEqualTypeItemList(UICT_SOCKET_SUCCESS, kItemCont) )
			{
				if( 1 >= kItemCont.size() )
				{
					ContHaveItemNoCount::const_iterator item_c_itor = kItemCont.begin();
					if( kItemCont.end() != item_c_itor )
					{
						dwItemNo = item_c_itor->first;
						break;
					}
				}
				else
				{
					if( SItemPos::NullData() != m_kProbabilityItemPos )
					{
						PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
						if(!pkPlayer){return false;}
						PgInventory *pInv = pkPlayer->GetInven();
						if(!pInv){return false;}

						PgBase_Item kItem;
						if(S_OK != pInv->GetItem(m_kProbabilityItemPos, kItem)){ return false; }

						dwItemNo = kItem.ItemNo();
						break;
					}					
				}
			}*/
			//dwItemNo = PROBABILITY_ITEM_NO_BASE;
			dwItemNo = GetSocketNeedItemNo();

			// 지금은 확률 아이템 사용을 안한다.
			/*switch( Grade )
			{
			case IG_NORMAL:
			case IG_RARE:
			case IG_UNIQUE:
			case IG_ARTIFACT:
				{
					dwItemNo = PROBABILITY_ITEM_NO_BASE + (Grade * 10);
				}break;
			default:
				{
					if( AttachElement()!=E_PPTY_CURSE )
					{
						return false;
					}
				}break;
			}*/			
		} break;
	default:{} break;
	}

	int const iMaxItemNo = pInv->GetTotalCount(dwItemNo);

	if( true == GetSocketSystemEmpty() )
	{
		pkArray[iIndex].SetInfo(iIndex, dwItemNo, (SIT_SOUL == iIndex)?(PgSocketFormula::GetCreateNeedSoul(rkSItem, GetCreateSocketItemOrder())):(1), iMaxItemNo);
	}	
	else
	{
		// 현재 메뉴에 따라서 호출을 다르게 구현
		if( SIM_REMOVE == GetMenuType() )
		{
			pkArray[iIndex].SetInfo(iIndex, dwItemNo, (SIT_SOUL == iIndex)?(PgSocketFormula::GetRemoveNeedSoul(rkSItem, GetRemoveSocketItemOrder())):(1), iMaxItemNo);
		}
		else if(SIM_EXTRACTION == GetMenuType())
		{
			SetExtractionCashItem(iCount,0);
			ExtractionNeedItem();						
			GetSelectOrderMenuCheck(iCount);
			SocketSystemUpdateUI(iCount);
		}
		else // 소켓 아이템 리셋(막혔을 경우)
		{
			pkArray[iIndex].SetInfo(iIndex, dwItemNo, (SIT_SOUL == iIndex)?(PgSocketFormula::GetRemoveNeedSoul(rkSItem, GetResetSocketItemOrder())):(1), iMaxItemNo);
		}		
	}
	return true;
}
int PgItemSocketSystemMgr::ExtractionCheckCard()
{
	int iCheckState = 0;
	int iGenSocketState = 0;
	int iGenSocketCardtemp = 0;
	int iGenSocketCard = 0;
	for(int i=PgSocketFormula::SII_FIRST; i<=PgSocketFormula::SII_THIRD; ++i)
	{
		if( true == PgSocketFormula::GetEnchantInfoIndex(m_kItem, i, iGenSocketState, iGenSocketCardtemp) )
		{
			if( (GSS_GEN_SUCCESS == iGenSocketState) && (0 != iGenSocketCardtemp) )
			{
					iCheckState = i;
					if(iCheckState == m_iSelectOrderMenu )
					{
						iGenSocketCard = iGenSocketCardtemp;	
					}
			}
		}
	}

	int iCardItemNo = 0;
	CONT_MONSTERCARD const *kCont = NULL;
	g_kTblDataMgr.GetContDef(kCont);
	if( kCont )
	{
		CONT_MONSTERCARD::key_type kKey(m_iSelectOrderMenu, iGenSocketCard);
		CONT_MONSTERCARD::const_iterator iter = kCont->find(kKey);
		if( kCont->end() != iter )
		{
			iCardItemNo =  iter->second;
		}
	}
	return iCardItemNo;
}
void PgItemSocketSystemMgr::ExtractionNeedItem()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(m_iCardItemNo);
	if(!pDef){return;}
	
	m_iNeedItemExtractionCount = pDef->GetAbil(AT_SOCET_CARD_EXTRACTION_ITEM_COUNT);//ingame에 필요한 아이템 수량.
	m_iExtractionInGameItemNameNum =  pDef->GetAbil(AT_SOCET_CARD_EXTRACTION_ITEM_NAME);//ingame에 필요한 아이템 번호.

	m_iNeedItemExtractionCountTemp = m_iNeedItemExtractionCount;
	PgSocketFormula::GetExtractionItemCount(pkPlayer, pDef, m_iUserHaveCashItemCount, m_iNeedItemExtractionCountTemp);

	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("SFRM_SOCKET_SYSTEM"));
	if( pWnd )
	{
		XUI::CXUI_Wnd* pSrc = pWnd->GetControl(L"SFRM_SHADOW");
		if(pSrc)
		{
			XUI::CXUI_Icon*	pIcon = dynamic_cast<XUI::CXUI_Icon*>(pSrc->GetControl(L"ICN_IN_GAME_ITEM"));
			if(pIcon)
			{
				pIcon->SetCustomData(&m_iExtractionInGameItemNameNum, sizeof(m_iExtractionInGameItemNameNum));
				//SocketSystemUpdateUI(iNeedInGameItem);
			}
		
			wchar_t const* pkItemName = NULL;
			if( GetDefString(m_iExtractionInGameItemNameNum, pkItemName) )
			{
				Quest::SetCutedTextLimitLength(pSrc, pkItemName, L"...");
			}
		}		
	}
}

void PgItemSocketSystemMgr::SetExtractionCashItem(int iHaveCashitemCountPlus,int iCashBtnRegCount)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return;}

	GET_DEF(CItemDefMgr, kItemDefMgr);

	m_iCardItemNo = ExtractionCheckCard();

	CItemDef const *pMonsterCardItemDef = kItemDefMgr.GetDef(m_iCardItemNo);
	if(pMonsterCardItemDef)
	{
		m_iNeedCashItemExtractionCount = pMonsterCardItemDef->GetAbil(AT_SOCET_CARD_EXTRACTION_CASH_ITEM_COUNT);//소켓 카드 추출시 필요한 캐시 아이템 수량
		m_iHaveCashItemExtractionNum = pMonsterCardItemDef->GetAbil(AT_SOCET_CARD_EXTRACTION_CASH_ITEM);//소켓 카드 추출시 필요한 캐시 아이템 넘버
		m_iHaveItemCount = pInv->GetTotalCount(m_iHaveCashItemExtractionNum);
		m_iExtractionInGameItemNameNum =  pMonsterCardItemDef->GetAbil(AT_SOCET_CARD_EXTRACTION_ITEM_NAME);//ingame에 필요한 아이템 번호.
		m_iHaveIngameItemCount = pInv->GetTotalCount(m_iExtractionInGameItemNameNum);//ingame에 필요한 아이템 수량.
		
		if(m_iHaveItemCount == 0)
		{
			iHaveCashitemCountPlus = 0;
			if(0 < iCashBtnRegCount)
			{//캐쉬아이템 + 버튼 눌렀을 경우 해당 아이템이 없다면 메시지 박스 출력.
				lwCallUI("SFRM_NO_CASH_ITEM_MESSAGEBOX");
			}
		}

		
		m_iUserHaveCashItemCount += iHaveCashitemCountPlus;
		
		if( m_iNeedCashItemExtractionCount <= m_iUserHaveCashItemCount)
		{
			m_iUserHaveCashItemCount = m_iNeedCashItemExtractionCount;
		}
		else if(m_iUserHaveCashItemCount < 0)
		{
			m_iUserHaveCashItemCount = 0;
		}

		m_iUserHaveCashItemCount = std::max(m_iUserHaveCashItemCount,0);

		BM::vstring vStrText;
		vStrText = TTW(790167);
		vStrText.Replace(L"#Have#", m_iUserHaveCashItemCount);
		vStrText.Replace(L"#Need#", m_iNeedCashItemExtractionCount);

		XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("SFRM_SOCKET_SYSTEM"));
		if( pWnd )
		{
			XUI::CXUI_Wnd*	pWndMaterialItemCount = pWnd->GetControl(L"SFRM_MATERIAL_ITEM_COUNT");
			if(pWndMaterialItemCount )
			{
				pWndMaterialItemCount ->Text(vStrText);
			}

			XUI::CXUI_Icon*	pWndMaterial = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_MATERIAL_EXTRACTION"));
			if(pWndMaterial)
			{
				pWndMaterial->SetCustomData(&m_iHaveCashItemExtractionNum, sizeof(m_iHaveCashItemExtractionNum));
				if(0 == m_iHaveItemCount)
				{
					pWndMaterial->GrayScale(true);
				}
				else
				{
					pWndMaterial->GrayScale(false);
				}
			}

			XUI::CXUI_Wnd* pReg = pWnd->GetControl(L"BTN_EXTRACTION_REG");
			XUI::CXUI_Wnd* pDeReg = pWnd->GetControl(L"BTN_EXTRACTION_DEREG");
			if( !pReg || !pDeReg ){ return; }
			bool bCheckVisible = false;

			if(0 < m_iUserHaveCashItemCount && (m_iNeedCashItemExtractionCount > m_iUserHaveCashItemCount) )
			{
				pReg->Visible(true);
				pDeReg->Visible(true);
				return;
			}

			if(m_iUserHaveCashItemCount == m_iNeedCashItemExtractionCount)
			{
				pReg->Visible(bCheckVisible);
				pDeReg->Visible(!pReg->Visible());
			}
			
			if(0 == m_iUserHaveCashItemCount)
			{
				pDeReg->Visible(bCheckVisible);
				pReg->Visible(!pDeReg->Visible());
			}


		}
	}
}
void PgItemSocketSystemMgr::SetSoulText()
{
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("SFRM_SOCKET_SYSTEM"));
	if( pWnd )
	{
		XUI::CXUI_Wnd* pSrc = pWnd->GetControl(L"SFRM_SHADOW");
		if(pSrc)
		{
			pSrc->Text(L"");
			XUI::CXUI_Icon*	pIcon = dynamic_cast<XUI::CXUI_Icon*>(pSrc->GetControl(L"ICN_IN_GAME_ITEM"));
			if(pIcon)
			{
				pIcon->ClearCustomData();
			}

			XUI::CXUI_Wnd* pIconSoul = pSrc->GetControl(L"ICN_SOULITEM");
			if(pIconSoul)
			{
				pIconSoul->ClearCustomData();
				XUI::CXUI_Wnd* pIconSoulText = pIconSoul->GetControl(L"FRM_SOULITEM_TEXT");
				if(pIconSoulText)
				{
					pIconSoulText->Text(L"");
				}
			}

			XUI::CXUI_Wnd* pkItemNeed = pSrc->GetControl(L"SFRM_COLOR_ITEM_NEED");
			if( pkItemNeed )
			{
				std::wstring wstrMsg;
				WstringFormat( wstrMsg, MAX_PATH, TTW(790177).c_str(), 0 );
				pkItemNeed->FontColor(0xFF7b2e00);
				pkItemNeed->Text(wstrMsg);				
			}
		}		
	}
}

bool PgItemSocketSystemMgr::SetMaterialItem(EKindUIIconGroup const kGroup, SItemPos const& rkItemPos)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	PgBase_Item kItem;
	if(S_OK != pInv->GetItem(rkItemPos, kItem)){ return false; }

	switch( kGroup )
	{
	case KUIG_ITEM_RARITY_UPGRADE_INSURENCE:	{ m_kInsureItemPos = rkItemPos;		 } break;
	case KUIG_ITEM_RARITY_UPGRADE_PROBABILITY:	{ m_kProbabilityItemPos = rkItemPos; } break;
	default:
		{
			assert(0);
			return false;
		}
	}
	return true;
}

void PgItemSocketSystemMgr::SetItem(EKindUIIconGroup const kType, SIconInfo const & rkInfo)
{
	if(InProgress())
	{
		lwAddWarnDataTT(1482);
	}
	else if (0!=rkInfo.iIconGroup && 0<=rkInfo.iIconKey)
	{
		bool bSetItem = false;

		switch( kType )
		{
		case KUIG_SOCKET_SYSTEM:
			{	
				if( SetSrcItem(SItemPos(rkInfo.iIconGroup, rkInfo.iIconKey)) )
				{
					bSetItem = true;
					InitSocketMaterialBtnState();
				}
			} break;
		case KUIG_ITEM_RARITY_UPGRADE_INSURENCE:
		case KUIG_ITEM_RARITY_UPGRADE_PROBABILITY:
			{
				if( SetMaterialItem(kType, SItemPos(rkInfo.iIconGroup, rkInfo.iIconKey)) )
				{
					bSetItem = true;
				}
			}break;
		}

		if( bSetItem )
		{
			XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("SFRM_SOCKET_SYSTEM"));
			if( pWnd )
			{
				XUI::CXUI_Wnd* pSrc = pWnd->GetControl(L"IMG_ITEM");
				if( pSrc )
				{
					pSrc->Visible(false);
				}
			}
		}
	}
}

void PgItemSocketSystemMgr::SetSrcMaterialBtnInit(XUI::CXUI_Wnd* pWnd, int const iType, bool const bVisible)//
{
	if( !pWnd ){ return; }
	BM::vstring vStr(L"BTN_REG");
	vStr += iType;
	XUI::CXUI_Wnd* pReg = pWnd->GetControl(vStr);

	vStr = L"BTN_DEREG";
	vStr += iType;
	XUI::CXUI_Wnd* pDeReg = pWnd->GetControl(vStr);
	if( !pReg || !pDeReg ){ return; }
	pReg->Visible(bVisible);
	pDeReg->Visible(!pReg->Visible());
}

void PgItemSocketSystemMgr::SetMaterialExtractionBtnInit(XUI::CXUI_Wnd* pWnd,int iCount,int iCashBtnRegCount)
{
	SetExtractionCashItem(iCount,iCashBtnRegCount);
	ExtractionNeedItem();
	SocketSystemUpdateUI(m_iSelectOrderMenu);	
}

void PgItemSocketSystemMgr::SetMaterialItem(int iType, SItemPos kItemPos, bool bCreate)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){ return; }
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv ){ return; }

	PgBase_Item kItem;
	if(S_OK != pkInv->GetItem(kItemPos, kItem))
	{
		return;
	}

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( PgSocketSystemUtil::kSocketSystemUIName );
	if (!pkWnd)
	{
		return;
	}

	BM::vstring kString;
	kString = L"BTN_REG";
	kString += iType;
	XUI::CXUI_Wnd* pReg = pkWnd->GetControl(kString);
	if( pReg )
	{
		//SetMaterialItem(pkWnd, iType, bCreate);
		SetItem(KUIG_ITEM_RARITY_UPGRADE_PROBABILITY, SIconInfo(kItemPos.x, kItemPos.y));
		SetSrcMaterialBtnInit(pkWnd, iType, false);
		SetElementInfo(SIT_PROBABILITY, kItem, pkInv, m_kItemArray);
	}
}


int PgItemSocketSystemMgr::GetMenuTypeOrder()
{
	int iOrder = 0;

	switch( GetMenuType() )
	{
	case SIM_CREATE:
		{
			iOrder = GetCreateSocketItemOrder();
		}break;
	case SIM_REMOVE:
		{
			iOrder = GetRemoveSocketItemOrder();
		}break;
	case SIM_RESET:
		{
			iOrder = GetResetSocketItemOrder();
		}break;
	case SIM_EXTRACTION:
		{
			iOrder = GetExtractionSocketItemOrder();
		}break;
	default:
		{
		}break;
	}
	return iOrder;
}

int PgItemSocketSystemMgr::GetSocketNeedItemNo()
{
	if( m_kItem.IsEmpty() ){ return 0; }

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(m_kItem.ItemNo());
	if(!pDef)
	{
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return 0"));
		return 0;
	}

	if( SIM_EXTRACTION == GetMenuType() )
	{
		CItemDef const *pItemDef = kItemDefMgr.GetDef(m_kItem.ItemNo());
		if(!pItemDef)
		{
			return 0;
		}
		
		GET_DEF(CItemDefMgr, kItemDefMgr);
		int iCardItemNo = ExtractionCheckCard();
		
		CItemDef const *pMonsterCardItemDef = kItemDefMgr.GetDef(iCardItemNo);
		if(!pMonsterCardItemDef)
		{
			return 0;
		}
	
		return pMonsterCardItemDef->GetAbil(AT_SOCET_CARD_EXTRACTION_CASH_ITEM);
	}

	CONT_DEF_SOCKET_ITEM const * pCont = NULL;
	g_kTblDataMgr.GetContDef(pCont);

	int iEquipPos = PgSocketFormula::GetEquipType(m_kItem.ItemNo()); // 함수 사용
	int iLevelLimit = pDef->GetAbil(AT_LEVELLIMIT);
	int iOrder = GetMenuTypeOrder();

	CONT_DEF_SOCKET_ITEM::key_type kKey(iEquipPos, iLevelLimit, iOrder);
	CONT_DEF_SOCKET_ITEM::const_iterator const_iter = pCont->find(kKey);
	if(const_iter == pCont->end())
	{
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return 0"));
		return 0;
	}

	CONT_DEF_SOCKET_ITEM::mapped_type kValue = const_iter->second;

	switch( GetMenuType() )
	{
	case SIM_CREATE:
		{
			return static_cast<int>(kValue.iSocketItemNo);
		}break;
	case SIM_REMOVE:
		{
			return static_cast<int>(kValue.iRestorationItemNo);
		}break;
	case SIM_RESET:
		{
			return static_cast<int>(kValue.iResetItemNo);
		}break;
	default:
		{
		}break;
	}
	return 0;
}

void PgItemSocketSystemMgr::SetMaterialItem(XUI::CXUI_Wnd* pWnd, int iType, bool bCreate, bool bNoBuyMsg)
{

	if(g_kSocketSystemMgr.GetMenuType() ==  PgItemSocketSystemMgr::SIM_EXTRACTION )
	{
		return;
	}

	if( !pWnd ){ return; }
	if( m_kItem.IsEmpty() ){ return; }

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){ return; }
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv ){ return; }

	SItemPos	rkPos;

	DWORD dwItemNo = 0;
	switch( iType )
	{
	case SIT_SET_ELEMENT:
	case SIT_INSUR_ITEM:
		{
			/*if( SItemPos::NullData() != m_kInsureItemPos )
			{
				m_kInsureItemPos.Clear();
				goto _DEREG;
			}*/
			if( true == bCreate )
			{
				m_kInsureItemPos.Clear();
				goto _DEREG;
			}
			//dwItemNo = ISURANCE_ITEM_NO_BASE; 
			dwItemNo = GetSocketNeedItemNo();
		} break;
	case SIT_PROBABILITY:	
		{ 
			// 아직 확률 아이템 안쓴다.
			/*if( SItemPos::NullData() != m_kProbabilityItemPos )
			{
				m_kProbabilityItemPos.Clear();
				goto _DEREG;
			}*/
			if( true == bCreate )
			{
				m_kProbabilityItemPos.Clear();
				goto _DEREG;
			}

			/*ContHaveItemNoCount	kItemCont;
			if( UIItemUtil::SearchEqualTypeItemList(UICT_SOCKET_SUCCESS, kItemCont) )
			{
				if( 1 >= kItemCont.size() )
				{
					ContHaveItemNoCount::const_iterator item_c_itor = kItemCont.begin();
					if( kItemCont.end() != item_c_itor )
					{
						dwItemNo = item_c_itor->first;
						break;
					}
				}
			}*/

			//dwItemNo = PROBABILITY_ITEM_NO_BASE;
			dwItemNo = GetSocketNeedItemNo();

		} break;
	}

	if( S_OK != pkInv->GetFirstItem(dwItemNo, rkPos) )
	{
		if( false == bNoBuyMsg )
		{
			/*switch( iType )
			{
			case SIT_SET_ELEMENT:
			case SIT_INSUR_ITEM:	
				{ 
					lua_tinker::call<void, int, int>("OnCallStaticCashItemBuy", 4, 0);
				}break;
			case SIT_PROBABILITY:	
				{ 
					lua_tinker::call<void, int, int>("OnCallStaticCashItemBuy", 3, 0);
				}break;
			}*/

			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790401, true);
		}
		return;
	}

	switch( iType )
	{
	case SIT_SET_ELEMENT:
	case SIT_INSUR_ITEM:	
		{ 
			SetItem(KUIG_ITEM_RARITY_UPGRADE_INSURENCE, SIconInfo(rkPos.x, rkPos.y));
		}break;
	case SIT_PROBABILITY:	
		{ 
			SetItem(KUIG_ITEM_RARITY_UPGRADE_PROBABILITY, SIconInfo(rkPos.x, rkPos.y));	
		}break;
	}

	SetSrcMaterialBtnInit(pWnd, iType, false);
	return;
_DEREG:
	SetSrcMaterialBtnInit(pWnd, iType, true);
	return;
}

bool PgItemSocketSystemMgr::DisplayNeedItemText(int const dwItemNo)
{
	std::wstring kTempMsg = _T("");

	switch( GetMenuType() )
	{
	case SIM_CREATE:
		{
			if( SOUL_ITEMNO != dwItemNo )
			{
				// 생성시 아이템 없어도 된다.
				return false;				
			}
			kTempMsg = TTW(790406);
		}break;
	case SIM_REMOVE:			
		{
			kTempMsg = TTW(790141);
		}break;
	case SIM_RESET:
		{
			kTempMsg = TTW(790142);
		}break;
	case SIM_EXTRACTION:
		{
			if( SOUL_ITEMNO != dwItemNo )
			{
				// 생성시 아이템 없어도 된다.
				return false;				
			}
		}
	}

	if( !kTempMsg.empty() )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(dwItemNo);
		if(pItemDef)
		{						
			wchar_t const* pItemName = NULL;
			if( ::GetDefString(pItemDef->NameNo(), pItemName) )
			{
				PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#ITEM#", pItemName, kTempMsg);
				::Notice_Show(kTempMsg, EL_Warning, true);
			}
		}
	}

	return true;
}

void PgItemSocketSystemMgr::MakeExplane(SNeedItemSocketSystem const * pkArray )
{
	if( !pkArray ) { return; }
	/*m_kExplane = L"{C=0xFF4D3413/}";

	m_kExplane+= CheckNeedItem() ? TTW(1471) : TTW(1470);

	for (int i = 0; i < SIT_INSUR_ITEM; ++i)
	{
		if ((IsOnlyUseSoul() && (i!=SIT_SOUL)) || i == SIT_PROBABILITY)
		{
			continue;
		}
		m_kExplane+=TTW(1472+i);
		SNeedItemSocketSystem &rkItem = m_kItemArray[i];
		if(!rkItem.IsOK(IsOnlyUseSoul()))
		{
			m_kExplane+=L"{C=0xFFFF0000/}";	
		}
		m_kExplane+=__min(rkItem.iNowNum,rkItem.iNeedNum);
		m_kExplane+=L"{C=0xFF4D3413/}";
		m_kExplane+=L"/";
		m_kExplane+=rkItem.iNeedNum;
		m_kExplane+=L"\n";
	}*/

	SNeedItemSocketSystem &rkItem = m_kItemArray[0];

	if( true == GetSocketSystemEmpty() )
	{
		m_iSoulCreateCount = rkItem.iNeedNum;
	}
	else
	{
		if( SIM_REMOVE == GetMenuType() )
		{
			m_iSoulRemoveCount = rkItem.iNeedNum;
		}
		else
		{
			m_iSoulResetCount = rkItem.iNeedNum;
		}
		
	}		
}

void PgItemSocketSystemMgr::UpdateSoulItemView()
{
	if(!m_kItem.IsEmpty())
	{
		m_iSoulCreateCount = PgSocketFormula::GetCreateNeedSoul(m_kItem, GetCreateSocketItemOrder());
		m_iSoulRemoveCount = PgSocketFormula::GetRemoveNeedSoul(m_kItem, GetRemoveSocketItemOrder());
		m_iSoulResetCount = PgSocketFormula::GetRemoveNeedSoul(m_kItem, GetResetSocketItemOrder());
	}
}

void PgItemSocketSystemMgr::CheckMenuSocketItem()
{
	if(!m_kItem.IsEmpty())
	{
		SEnchantInfo const kEhtInfo = m_kItem.EnchantInfo();

		E_ITEM_GRADE const eItemGrade = GetItemGrade(m_kItem);
		bool bRet = true; //kEhtInfo.EanbleGenSocket(eItemGrade);

		switch( GetMenuType() )
		{
		case SIM_CREATE:	// 생성
			{
				if( false == bRet )
				{
					lwAddWarnDataTT(790399);//무기/방어구류만 소켓을 생성할 수 있습니다."/>
					break;
				}

				if( 0 == GetSocketNeedItemNo() )
				{
					lwAddWarnDataTT(790114);//"소켓 제작이 불가능한 아이템입니다.
					break;
				}

				if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState()) && (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState2()) && (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState3()) )
				{
					lwAddWarnDataTT(790149);//소켓을 3개 모두 생성하였습니다. 더 이상 &#39;소켓생성&#39;이 불가능 합니다.
					break;
				}
				GetSelectOrderMenuCheck(SIM_CREATE);

				if( GSS_GEN_NONE == kEhtInfo.GenSocketState() )
				{
					return;
				}
				if( GSS_GEN_NONE == kEhtInfo.GenSocketState2() && ( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState()) /*&& (0 != kEhtInfo.MonsterCard())*/ ) )
				{
					return;
				}
				if( GSS_GEN_NONE == kEhtInfo.GenSocketState3() && ( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState2()) /*&& (0 != kEhtInfo.MonsterCard2())*/ ) )
				{
					return;
				}

				int iIndex = 0;
				if( GSS_GEN_FAIL == kEhtInfo.GenSocketState() )
				{
					iIndex = static_cast<int>(PgSocketFormula::SII_FIRST);
				}
				if( GSS_GEN_FAIL == kEhtInfo.GenSocketState2() )
				{
					iIndex = static_cast<int>(PgSocketFormula::SII_SECOND);
				}
				if( GSS_GEN_FAIL == kEhtInfo.GenSocketState3() )
				{
					iIndex = static_cast<int>(PgSocketFormula::SII_THIRD);
				}

				if( 0 != iIndex )
				{
					std::wstring kTempMsg = TTW(790143);

					if( !kTempMsg.empty() )
					{
						PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#NUM#", BM::vstring(iIndex), kTempMsg);
						::Notice_Show(kTempMsg, EL_Warning, true);
					}
				}
				else
				{
					return;
					//lwAddWarnDataTT(790422);
				}
			}break;
		case SIM_REMOVE: // 환원
			{
				if( 0 == GetSocketNeedItemNo() )
				{
					lwAddWarnDataTT(790114);
					break;
				}
				GetSelectOrderMenuCheck(SIM_REMOVE);

				if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState()) && (0 != kEhtInfo.MonsterCard()) )
				{
					return;
				}
				if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState2()) && (0 != kEhtInfo.MonsterCard2()) )
				{
					return;
				}
				if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState3()) && (0 != kEhtInfo.MonsterCard3()) )
				{
					return;
				}

				XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( PgSocketSystemUtil::kSocketSystemUIName );
				if( pkWnd )
				{
					XUI::CXUI_Wnd* pkTitle = pkWnd->GetControl(L"SFRM_EXPLANE1");
					if( pkTitle )
					{
						pkTitle->Text(TTW(790138));
					}
				}
				lwAddWarnDataTT(790133);
			}break;
		case SIM_RESET:	// 복원
			{
				if( 0 == GetSocketNeedItemNo() )
				{
					lwAddWarnDataTT(790114);
					break;
				}
				GetSelectOrderMenuCheck(SIM_RESET);

				if( GSS_GEN_FAIL == kEhtInfo.GenSocketState() )
				{
					return;
				}
				if( GSS_GEN_FAIL == kEhtInfo.GenSocketState2() )
				{
					return;
				}
				if( GSS_GEN_FAIL == kEhtInfo.GenSocketState3() )
				{
					return;
				}
				lwAddWarnDataTT(790134);
			}break;
		case SIM_EXTRACTION:	// 추출
			{
				if( 0 == GetSocketNeedItemNo() )
				{
					lwAddWarnDataTT(790171);
					SetSoulText();
					break;
				}

				if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState()) && (0 != kEhtInfo.MonsterCard()) )
				{
					return;
				}
				if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState2()) && (0 != kEhtInfo.MonsterCard2()) )
				{
					return;
				}
				if( (GSS_GEN_SUCCESS == kEhtInfo.GenSocketState3()) && (0 != kEhtInfo.MonsterCard3()) )
				{
					return;
				}

				XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( PgSocketSystemUtil::kSocketSystemUIName );
				if( pkWnd )
				{
					XUI::CXUI_Wnd* pkTitle = pkWnd->GetControl(L"SFRM_EXPLANE1");
					if( pkTitle )
					{
						pkTitle->Text(TTW(790171));
					}
				}
				lwAddWarnDataTT(790170);
			}break;
		default:
			{
			}break;
		}
		m_kItem.Clear();
		ClearSocketSystemUI();
		Clear();
	}
	return;
}

int PgItemSocketSystemMgr::GetSocketSystemErrorMsg(HRESULT hResult)
{
	RecentResult(ISOC_INVALID_PROPERTY);

	switch( hResult )
	{
	case S_OK:
		{
			RecentResult(ISOC_SUCCESS);
			return 790400;
		}break;
	case E_GS_NOT_FOUND_ITEM:
		{
			RecentResult(ISOC_NOT_FOUND_TARGET_ITEM);
			return 790401;
		}break;
	case E_GS_INVALID_IDX:
		{
			return 790402;
		}break;
	case E_GS_CANNOT_GEN:
		{
			RecentResult(ISOC_NOT_ABLE_PROPERTY_ITEM);			
			return 790403;
		}break;
	case E_GS_ALREADY_GEN:
		{
			return 790404;
		}break;
	case E_GS_NOT_ENOUGH_MONEY:
		{
			RecentResult(ISOC_NOT_ENOUGH_MONEY);			
			return 790405;
		}break;
	case E_GS_NOT_ENOUGH_SOUL:
		{
			RecentResult(ISOC_NOT_ENOUGH_SOUL);			
			return 790406;
		}break;
	case E_GS_NOT_GEN:
		{
			return 790407;
		}break;
	case E_GS_SOCKET_FULL:
		{
			return 790408;
		}break;
	case E_GS_OVER_MAX_IDX:
		{
			return 790409;
		}break;
	case E_GS_GEN_SOCKET_FAIL:
		{
			RecentResult(ISOC_FAIL);			
			return 790410;
		}break;
	case E_GS_CARD_FULL:
		{
			return 790411;
		}break;
	case E_GS_NOT_SET_CARD:
		{
			return 790412;
		}break;
	case E_GS_SYSTEM_ERROR:
		{
			return 790413;
		}break;
	case E_GS_NOT_ENOUGH_ITEM:
		{
			return 790175;
		}
	case E_GS_FAIL_SOCKET:
		{
			return 790174;
		}
	default:
		{
			return 790413;
		}break;
	}
	return 790413;
}

BM::vstring PgItemSocketSystemMgr::GetExplane()	//설명
{
	return m_kExplane;
}

int PgItemSocketSystemMgr::GetSoulCreateCount()
{
	return m_iSoulCreateCount;
}

int PgItemSocketSystemMgr::GetSoulRemoveCount()
{
	return m_iSoulRemoveCount;
}

int PgItemSocketSystemMgr::GetSoulResetCount()
{
	return m_iSoulResetCount;
}

int PgItemSocketSystemMgr::GetExtractionCount()
{
	return m_iNeedItemExtractionCountTemp;
}

int PgItemSocketSystemMgr::GetHaveCount()
{
	switch(GetMenuType())
	{
	case SIM_CREATE:
	case SIM_REMOVE:
	case SIM_RESET:
		{
			return m_kItemArray[0].iNowNum;
		}break;
	case SIM_EXTRACTION:
		{
			return m_iHaveIngameItemCount;
		}break;
	}
	return 0;
}

void PgItemSocketSystemMgr::SetMenuType(int iMenu)
{
	m_iMenu = iMenu;
}

int PgItemSocketSystemMgr::GetMenuType()
{
	return m_iMenu;
}

void PgItemSocketSystemMgr::DisplaySocketRemoveText(int iIndex)
{
	if( PgSocketFormula::SII_SECOND == GetMenuType() )
	{
		std::wstring kTempMsg = TTW(790144);

		if( !kTempMsg.empty() )
		{
			PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#NUM#", BM::vstring(iIndex), kTempMsg);
			::Notice_Show(kTempMsg, EL_Warning, true);
		}
	}
}