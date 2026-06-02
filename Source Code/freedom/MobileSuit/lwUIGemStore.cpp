#include "StdAfx.h"
#include "PgNetwork.h"
#include "ServerLib.h"
#include "Lohengrin/ErrorCode.h "
#include "Variant/Inventory.H"
#include "Variant/PgJobSkill.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "lwUIGemStore.h"

std::wstring const WSTR_GEM_STORE(_T("FRM_GEM_STORE"));
std::wstring const WSTR_CP_STORE(_T("FRM_CP_STORE"));
std::wstring const WSTR_STORE_LIST(_T("LIST_TRADE_LIST"));
std::wstring const WSTR_ITEM_NAME(_T("SFRM_ITEM_NAME"));
std::wstring const WSTR_LV_CLASS(_T("FRM_LV_CLASS"));
std::wstring const WSTR_CHKBTN(_T("CBTN_SELECT_ITEM"));
std::wstring const WSTR_JEWEL(_T("FRM_JEWEL"));
std::wstring const WSTR_ICON_JEWEL(_T("ICON_JEWEL"));
std::wstring const WSTR_ICON_ITEM(_T("ICON_ITEM"));
std::wstring const WSTR_FRM_GET_ITEM(_T("FRM_GET_ITEM"));
std::wstring const WSTR_FRM_MSG(_T("FRM_MSG"));
std::wstring const WSTR_ICON(_T("ICON"));
std::wstring const WSTR_FRM_CP(_T("FRM_CP"));
std::wstring const WSTR_ARROW(_T("FRM_ARROW"));
std::wstring const WSTR_ITEM_ICON_SLOT_BG(_T("ICON_SLOT_BG"));
std::wstring const WSTR_GEM_STORE_ITEMTREE(_T("SFRM_JEWEL_CONSTELLATION"));

int const JEWEL_SLOT_CNT = 5;
int const JEWEL_CONSTELLATION_SLOT_X_CNT = 5;
int const JEWEL_CONSTELLATION_SLOT_Y_CNT = 6;
int const JEWEL_SLOT_COL_MAX = 4;
int const ITEM_TREE_UP_ITEM_SLOT_MAX = 8;
int const ITEM_TREE_CENTER_ITEM_NO_SIDE_ARROW = 5;
int const ITEM_TREE_TOTAL_INDEX = JEWEL_CONSTELLATION_SLOT_X_CNT*JEWEL_CONSTELLATION_SLOT_Y_CNT;
int const ITEM_TREE_HISTORY_COUNT = 7;
int const ITEM_TREE_NO_DISPLAY = -1;

namespace Quest
{
	extern void SetCutedTextLimitLength(XUI::CXUI_Wnd *pkWnd, std::wstring const &rkText, std::wstring kTail, long const iCustomTargetWidth = 0);
}

namespace GEM_STORE
{
	int GetDefOrderIndex(int const iItemNo, E_GEMSTORE_TYPE	const eGemStoreType)
	{
		CONT_DEFGEMSTORE const * pDef = NULL;
		g_kTblDataMgr.GetContDef(pDef);
		if( NULL == pDef )
		{
			return 0;
		}
	
		CONT_DEFGEMSTORE::const_iterator iter = pDef->begin();
		while( pDef->end() != iter )
		{
			for(CONT_DEFGEMSTORE_ARTICLE::const_iterator SubIter = iter->second.kContArticles.begin(); SubIter != iter->second.kContArticles.end(); ++SubIter )
			{
				if( eGemStoreType == SubIter->first.iMenu )
				{
					if(iItemNo == SubIter->first.iItemNo)
					{
						return SubIter->first.iOrderIndex;
					}
				}
			}
			++iter;
		}
		return 0;
	}

	struct SDefGems
	{
		SDefGems() : iItemNo(0), iItemCount(0), iOrderIndex(0), bTop(false), pkDefGems(NULL) {}
		int iItemNo;	//만들어질 아이템
		int iItemCount;
		int iOrderIndex;
		bool bTop;
		CONT_DEFGEMS const* pkDefGems;	//재료 아이템들
	};

	bool FindUpDefGems(int const iGemItemNo, E_GEMSTORE_TYPE const eGemStoreType, SDefGems & rkData)
	{
		CONT_DEFGEMSTORE const * pDef = NULL;
		g_kTblDataMgr.GetContDef(pDef);
		if( NULL == pDef )
		{
			return false;
		}
	
		CONT_DEFGEMSTORE::const_iterator iter = pDef->begin();
		while( pDef->end() != iter )
		{
			CONT_DEFGEMSTORE_ARTICLE::const_iterator SubIter = iter->second.kContArticles.begin();
			for( ; SubIter != iter->second.kContArticles.end(); ++SubIter )
			{
				if( eGemStoreType == SubIter->first.iMenu )
				{
					if( (false == SubIter->second.kContGems.empty())
					 && (SubIter->second.kContGems.at(0).iGemNo == iGemItemNo) )
					{
						rkData.iItemNo = SubIter->first.iItemNo;
						rkData.iItemCount = SubIter->second.iItemCount;
						rkData.iOrderIndex = SubIter->first.iOrderIndex;
						rkData.pkDefGems = &SubIter->second.kContGems;
						return true;
					}
				}
			}
			++iter;
		}
	
		return false;
	}

	bool FindDefGems(int const iGemItemNo, E_GEMSTORE_TYPE const eGemStoreType, SDefGems & rkData)
	{
		CONT_DEFGEMSTORE const * pDef = NULL;
		g_kTblDataMgr.GetContDef(pDef);
		if( NULL == pDef )
		{
			return false;
		}

		CONT_DEFGEMSTORE::const_iterator iter = pDef->begin();
		while( pDef->end() != iter )
		{
			CONT_DEFGEMSTORE_ARTICLE::const_iterator SubIter  = iter->second.kContArticles.begin();
			for( ; SubIter  != iter->second.kContArticles.end(); ++SubIter  )
			{	
				if( eGemStoreType != SubIter->first.iMenu )
				{
					continue;
				}

				if(iGemItemNo == SubIter->first.iItemNo)
				{
					rkData.iItemNo = SubIter->first.iItemNo;
					rkData.iItemCount = SubIter->second.iItemCount;
					rkData.iOrderIndex = SubIter->first.iOrderIndex;
					rkData.pkDefGems = &SubIter->second.kContGems;
					return true;
				}
			}
			++iter;
		}
		return false;
	}
} //namespace GEM_STORE

PgGemStore::PgGemStore()
: m_iEnoughCnt(0)
, m_bIsCPStore(false)
, m_bCheckClass(false)
, m_iTreeItemNo(0)
, m_iTreeOrderIndex(0)
{}

PgGemStore::~PgGemStore()
{}

void PgGemStore::SendReqGemStoreInfo(BM::GUID& rkNpcGuid)
{// 상점정보를 요청하고
	m_kNpcGuid = rkNpcGuid;
	BM::Stream kPacket(PT_C_M_REQ_GEMSTOREINFO);
	kPacket.Push(rkNpcGuid);
	NETWORK_SEND(kPacket)
	m_eGemStoreType = EGT_OTHER;
}

void PgGemStore::SendReqEventStoreInfo(BM::GUID& rkNpcGuid)
{// 상점정보를 요청하고
	SendReqGemStoreInfo(rkNpcGuid);
	m_eGemStoreType = EGT_EVENT;
}

void PgGemStore::RecvGemstoreInfo(BM::Stream& rkPacket)
{// 보석상점 정보를 받은 후	
	SGEMSTORE kStoreInfo;
	kStoreInfo.ReadFromPacket(rkPacket);

	//m_eGemStoreType = static_cast<E_GEMSTORE_TYPE>(EGT_OTHER);//미리 셋팅하도록 바꿈

	m_kContAtricle = kStoreInfo.kContArticles;

	// 상점에서 파는 아이템들 중
	CONT_GEMSTORE_ARTICLE::iterator itor_Article = m_kContAtricle.begin();
	while( itor_Article != m_kContAtricle.end() )
	{//아무것도 없네.
		CONT_GEMSTORE_ARTICLE::key_type const& kArticleItemNo = itor_Article->first;
		CONT_GEMSTORE_ARTICLE::mapped_type const& kArticle = itor_Article->second;

		//CP 상점인가.
		m_bIsCPStore = (kArticle.iCP != 0);
		if( !CheckMyClassItem( kArticleItemNo ) )
		{
			itor_Article = m_kContAtricle.erase(itor_Article);
			continue;
		}
		++itor_Article;
	}

	XUI::CXUI_Wnd* pkParentWnd = NULL;
	if( m_bIsCPStore )
	{
		pkParentWnd = CallCPStoreUIandGetListWnd();
	}
	else
	{
		pkParentWnd = CallGemStoreUIandGetListWnd();
	}

	if(!pkParentWnd)
	{
		return;
	}

	XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkParentWnd->GetControl(L"BLD_SLOT"));
	if( !pkBuild )
	{
		return;
	}

	int const MAX_ITEM_SLOT = pkBuild->CountX() * pkBuild->CountY();
	m_kPage.SetPageAttribute(MAX_ITEM_SLOT, 2);
	m_kPage.SetMaxItem(m_kContAtricle.size());
	UIPageUtil::PageControl(pkParentWnd->GetControl(L"FRM_PAGE"), m_kPage);
	SetStoreSlot(pkParentWnd);
}

bool PgGemStore::SendReqGemTradeTree()
{
	return SendReqGemTrade(m_iTreeItemNo, m_iTreeOrderIndex);
}

bool PgGemStore::SendReqGemTrade()
{
	return SendReqGemTrade(m_iSelectedItemNo, m_iOrderIndex);
}

bool PgGemStore::SendReqGemTrade(int const iSelectedItemNo, int const iOrderIndex)
{// 아이템의 교환 요청을 보내고

	PgGemStore::E_CHECK_ITEM_RESULT eType;
	if( IsOtherMode() || IsEventMode() )
	{
		eType = isEnoughTrade(iSelectedItemNo, iOrderIndex);
	}
	else
	{
		eType = isDefEnoughTrade(iSelectedItemNo, iOrderIndex);
	}

	switch(eType)
	{// 1차 구매 가능여부
	case EIR_LACK_CP:
		{
			lwAddWarnDataTT(700127), EL_Warning;
		}return false;
	case EIR_LACK_JEWEL:
		{
			::Notice_Show(TTW(790125), EL_Warning);
		}return false;
	case EIR_LACK_HERO_MARK:
		{
			lwAddWarnDataTT(460058), EL_Warning;
		}return false;
	case EIR_FAIL:
		return false;
	}

	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pPlayer)
	{ 
		return false; 
	}
	PgInventory* pInv = pPlayer->GetInven();
	if(!pInv)
	{ 
		return false; 
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	EInvType InvType = static_cast<EInvType>(kItemDefMgr.GetAbil(iSelectedItemNo, AT_PRIMARY_INV));

	if(IT_NONE==InvType)
	{
		return false;
	}

	SItemPos kTempPos;
	if(!pInv->GetNextEmptyPos(InvType, kTempPos))
	{// 인벤토리가 가득 찼다면 메세지를 보냄
		::Notice_Show(TTW(20023), EL_Warning);
		return false;
	}

	if( IsOtherMode() || IsEventMode() )
	{
		BM::Stream kPacket(PT_C_M_REQ_GEMSTORE_BUY);
		kPacket.Push(m_kNpcGuid);
		kPacket.Push(iSelectedItemNo);
		NETWORK_SEND(kPacket)
	}
	else
	{
		BM::Stream kPacket(PT_C_M_REQ_DEFGEMSTORE_BUY);
		kPacket.Push(m_kNpcGuid);
		kPacket.Push(iSelectedItemNo);
		kPacket.Push(static_cast<int>(m_eGemStoreType));
		kPacket.Push(iOrderIndex);
		NETWORK_SEND(kPacket)
		XUI::CXUI_Wnd* pMainGemTrade = XUIMgr.Get(WSTR_GEM_STORE);
		g_kGemStore.ClearNeedItemImg(pMainGemTrade);
	}
	return true;
}

void PgGemStore::RecvGemTradeResult(BM::Stream& rkPacket)
{// 교환 결과를 받아 처리 
	HRESULT kResult;
	rkPacket.Pop(kResult);
	switch(kResult)
	{
	case E_NOT_FOUND_GEMSTORE:
		{// 보석교환 상인을 찾을 수 없음
			::Notice_Show(TTW(790123), EL_Warning);
		}break;
	case E_NOT_FOUND_ARTICLE:
		{// 교환물품을 찾을 수 없음
			::Notice_Show(TTW(790124), EL_Warning);
		}break;
	case E_NOT_ENOUGH_GEMS:
		{// 보석이 부족함
			::Notice_Show(TTW(790125), EL_Warning);
		}break;
	case S_OK:
		{
			XUI::CXUI_Wnd* pkParentWnd = NULL;
			if( !m_bIsCPStore )
			{
				pkParentWnd = CallGemStoreUIandGetListWnd();
			}

			if(pkParentWnd)
			{
				XUI::CXUI_Wnd* pkPage = dynamic_cast<XUI::CXUI_Wnd*>(pkParentWnd->GetControl(L"FRM_PAGE"));
				if( !pkPage )
				{
					return;
				}				
				ResetCursor(lwUIWnd(pkPage));
			}
		}break;
	default:
		{
		}break;
	}
}

void PgGemStore::BeginPage(lwUIWnd kSelf)
{
	if( kSelf.IsNil() ){ return; }

	lwUIWnd kParent = kSelf.GetParent();
	if( kParent.IsNil() ){ return; }

	lwUIWnd kMainUI = kParent.GetParent();
	if( kMainUI.IsNil() ){ return; }

	int const NowPage = m_kPage.Now();
	if( NowPage == m_kPage.PageBegin() )
	{
		return;
	}
	UIPageUtil::PageControl(kParent.GetSelf(), m_kPage);

	if( IsOtherMode() || IsEventMode())
	{
		SetStoreSlot(kMainUI.GetSelf());
	}
	else
	{
		SetDefStoreSlot(kMainUI.GetSelf());
	}

	ResetCursor(kParent);
}

void PgGemStore::EndPage(lwUIWnd kSelf)
{
	if( kSelf.IsNil() ){ return; }

	lwUIWnd kParent = kSelf.GetParent();
	if( kParent.IsNil() ){ return; }

	lwUIWnd kMainUI = kParent.GetParent();
	if( kMainUI.IsNil() ){ return; }

	int const NowPage = m_kPage.Now();
	if( NowPage == m_kPage.PageEnd() )
	{
		return;
	}
	UIPageUtil::PageControl(kParent.GetSelf(), m_kPage);

	if( IsOtherMode() || IsEventMode())
	{
		SetStoreSlot(kMainUI.GetSelf());
	}
	else
	{
		SetDefStoreSlot(kMainUI.GetSelf());
	}

	ResetCursor(kParent);
}

void PgGemStore::JumpPrevPage(lwUIWnd kSelf)
{
	if( kSelf.IsNil() ){ return; }

	lwUIWnd kParent = kSelf.GetParent();
	if( kParent.IsNil() ){ return; }

	lwUIWnd kMainUI = kParent.GetParent();
	if( kMainUI.IsNil() ){ return; }

	int const NowPage = m_kPage.Now() + 1;
	if( NowPage == m_kPage.PagePrevJump() )
	{
		return;
	}
	UIPageUtil::PageControl(kParent.GetSelf(), m_kPage);

	if( IsOtherMode() || IsEventMode())
	{
		SetStoreSlot(kMainUI.GetSelf());
	}
	else
	{
		SetDefStoreSlot(kMainUI.GetSelf());
	}

	ResetCursor(kParent);
}

void PgGemStore::JumpNextPage(lwUIWnd kSelf)
{
	if( kSelf.IsNil() ){ return; }

	lwUIWnd kParent = kSelf.GetParent();
	if( kParent.IsNil() ){ return; }

	lwUIWnd kMainUI = kParent.GetParent();
	if( kMainUI.IsNil() ){ return; }

	int const NowPage = m_kPage.Now() + 1;
	if( NowPage == m_kPage.PageNextJump() )
	{
		return;
	}
	UIPageUtil::PageControl(kParent.GetSelf(), m_kPage);

	if( IsOtherMode() || IsEventMode() )
	{
		SetStoreSlot(kMainUI.GetSelf());
	}
	else
	{
		SetDefStoreSlot(kMainUI.GetSelf());
	}

	ResetCursor(kParent);
}

void PgGemStore::Page(lwUIWnd kSelf)
{
	if( kSelf.IsNil() ){ return; }

	lwUIWnd kParent = kSelf.GetParent();
	if( kParent.IsNil() ){ return; }

	lwUIWnd kMainUI = kParent.GetParent();
	if( kMainUI.IsNil() ){ return; }


	int const NowPage = m_kPage.Now();
	int iNewPage = (NowPage / m_kPage.GetMaxViewPage()) * m_kPage.GetMaxViewPage() + kSelf.GetBuildIndex();
	if( NowPage == iNewPage )
	{
		return;
	}

	if( iNewPage >= m_kPage.Max() )
	{
		iNewPage = m_kPage.Max() - 1;
	}

	m_kPage.PageSet(iNewPage);
	UIPageUtil::PageControl(kParent.GetSelf(), m_kPage);

	if( IsOtherMode() || IsEventMode() )
	{
		SetStoreSlot(kMainUI.GetSelf());
	}
	else
	{
		SetDefStoreSlot(kMainUI.GetSelf());
	}

	ResetCursor(kParent);
}

XUI::CXUI_Wnd* PgGemStore::CallGemStoreUIandGetListWnd()
{// 보석상점 UI를 열고, 이것의 List Wnd를 반환한다
	XUI::CXUI_Wnd* pkStoreWnd = XUIMgr.Get(WSTR_GEM_STORE.c_str());
	if(!pkStoreWnd)
	{
		pkStoreWnd = XUIMgr.Call(WSTR_GEM_STORE.c_str());
		if(!pkStoreWnd)	
		{
			return NULL;
		}
	}
	return pkStoreWnd;
}

XUI::CXUI_Wnd* PgGemStore::CallCPStoreUIandGetListWnd()
{
	XUI::CXUI_Wnd* pkStoreWnd = XUIMgr.Get(WSTR_CP_STORE.c_str());
	if(!pkStoreWnd)
	{
		pkStoreWnd = XUIMgr.Call(WSTR_CP_STORE.c_str());
		if(!pkStoreWnd)	
		{
			return NULL;
		}
	}
	return pkStoreWnd;
}

bool PgGemStore::CheckMyClassItem(int const iItemNo)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(iItemNo);

	if( NULL == pItemDef )
	{
		return false;
	}

	__int64 const iClassLimit_Item = pItemDef->GetAbil64(AT_CLASSLIMIT);

	int iClassNo=0;
	if(UCLIMIT_ALL != iClassLimit_Item)
	{// 모든 클래스 사용이 아니면
		for(int iClass=UCLASS_FIGHTER; iClass<UCLASS_MAX; ++iClass)
		{// 아이템 착용 클래스
			if(iClassLimit_Item & 1i64<<iClass)
			{
				iClassNo = iClass;
				break;
			}
		}
	}
	
	CUnit* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}
	__int64 iPlayerClass = 1i64 << pkPlayer->GetAbil(AT_CLASS);
	if( 0 == (iPlayerClass & iClassLimit_Item) )
	{
		return false;
	}

	return true;
}

bool PgGemStore::SetStoreSlot(XUI::CXUI_Wnd* pkMain)
{
	if(!pkMain)
	{
		return false;
	}


	CONT_GEMSTORE_ARTICLE::const_iterator itor_Article = m_kContAtricle.begin();

	int const iIgnoreItemCount = m_kPage.Now() * m_kPage.GetMaxItemSlot();
	for(int i = 0; i < iIgnoreItemCount; ++i)
	{
		if( itor_Article == m_kContAtricle.end() )
		{
			return false;
		}
		++itor_Article;
	}

	for( int i = 0; i < m_kPage.GetMaxItemSlot(); ++i )
	{
		BM::vstring kStr(L"ICON_SLOT");
		kStr += i;

		XUI::CXUI_Wnd* pkImg = pkMain->GetControl(kStr);
		if( !pkImg )
		{
			continue;
		}

		BM::vstring kStrCount(L"ICON_SLOT_COUNT");
		kStrCount += i;
		
		XUI::CXUI_Wnd* pkCount = pkMain->GetControl(kStrCount);
		if( pkCount )
		{
			pkCount->Text(L"");
		}		

		pkImg->ClearCustomData();
		if(itor_Article != m_kContAtricle.end())
		{// 이 아이템을 ( SGEMSTORE_ARTICLE(아이템번호, CONT_GEM-필요보석들) )
			CONT_GEMSTORE_ARTICLE::key_type kArticleItemNo = itor_Article->first;
			SetSlotItem(pkImg, kArticleItemNo);
			++itor_Article;
		}
 	}

	return true;
}

bool PgGemStore::SetSlotItem(XUI::CXUI_Wnd* pkIcon, int const iItemNo)
{
	if( pkIcon )
	{
		if( CheckMyClassItem(iItemNo) )
		{
			pkIcon->SetCustomData(&iItemNo, sizeof(iItemNo));

			XUI::CXUI_Image* pkIconImg = dynamic_cast<XUI::CXUI_Image*>(pkIcon);
			if(pkIconImg)
			{
				SetUIAddResToImage(pkIconImg, iItemNo);
			}
			return true;
		}
	}
	return false;
}

void PgGemStore::ClickSlotItem(lwUIWnd& klwChkBtn)
{// 해당 버튼만 체크 상태로 만들고, 나머지 list item의 체크를 푼다
	XUI::CXUI_Wnd* pkIconWnd = klwChkBtn.GetSelf();
	if( !pkIconWnd ){ return; }

	XUI::CXUI_Wnd* pMainWnd = pkIconWnd->Parent();
	if(!pMainWnd){ return; }

	pkIconWnd->GetCustomData(&m_iSelectedItemNo, sizeof(m_iSelectedItemNo));

	CONT_GEMSTORE_ARTICLE::const_iterator itor_Article = m_kContAtricle.find(m_iSelectedItemNo);
	if(itor_Article == m_kContAtricle.end())
	{
		return;
	}
	
	SGEMSTORE_ARTICLE const& kItem = (*itor_Article).second;		
	CONT_GEMS const& kContGems = kItem.kContGems;
	CONT_GEMS::const_iterator itor_ReqGem = kContGems.begin();

	if( kItem.iCP != 0 )
	{//CP상점이다.
		XUI::CXUI_Wnd* pkCPWnd = pMainWnd->GetControl(WSTR_FRM_CP);
		if( pkCPWnd )
		{
			pkCPWnd->FontColor(0xFFFFF568);
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( pkPlayer )
			{
				int const iPlayerCP = pkPlayer->GetAbil(AT_CP);
				if( iPlayerCP < kItem.iCP )
				{
					pkCPWnd->FontColor(0xFFFF0000);
				}
			}
			pkCPWnd->Text(BM::vstring(kItem.iCP*10).operator const std::wstring &());
		}
	}

	for(int i=0; i<JEWEL_SLOT_CNT; ++i)
	{// 필요한		
		BM::vstring vstr(WSTR_JEWEL);
		vstr+=i;
		XUI::CXUI_Wnd* pJewelWnd = pMainWnd->GetControl(vstr);
		if(pJewelWnd)
		{
			XUI::CXUI_Icon* pIconJewel = dynamic_cast<XUI::CXUI_Icon*>(pJewelWnd->GetControl(WSTR_ICON_JEWEL));
			if(pIconJewel)
			{
				pIconJewel->ClearCustomData();
				pJewelWnd->Text(_T(""));
				if(itor_ReqGem != kContGems.end())
				{
					// 보석 번호와
					int const iGemNo = (*itor_ReqGem).first;
					pIconJewel->SetCustomData(&iGemNo, sizeof(iGemNo));

					// 필요 갯수를 찾고
					short const sReqCnt = (*itor_ReqGem).second;

					std::wstring kJewelNumStrForm = TTW(790121);
					// 플레이어가 소지한 해당 보석의 갯수를 찾고
					int iJewelNumIhave = GetAmountItemPlayerHave(iGemNo);
					if(sReqCnt <= iJewelNumIhave )
					{
						iJewelNumIhave = sReqCnt;
					}
					// 하단부 UI에 수량을 표현하고
					wchar_t buf[100] = {0,};
					wsprintfW(buf, kJewelNumStrForm.c_str(), iJewelNumIhave, sReqCnt);
					pJewelWnd->Text(buf);
					{// 아이템의 갯수가 부족하면 회색, 아니면 정상 출력한다				
						pIconJewel->GrayScale(iJewelNumIhave<sReqCnt);
					}
					++itor_ReqGem;
				}
			}
		}
	}
}
void PgGemStore::ItemTreeClear()
{

	XUI::CXUI_Wnd* pMainWndItemTree = XUIMgr.Get(WSTR_GEM_STORE_ITEMTREE);
	if(pMainWndItemTree)
	{
		pMainWndItemTree = pMainWndItemTree->GetControl(L"FRM_UI");
		if(NULL == pMainWndItemTree)
		{
			return;
		}

		XUI::CXUI_Wnd* pTopJewel = pMainWndItemTree->GetControl(L"FRM_TOP_JEWEL");
		XUI::CXUI_Wnd* pIconJewelTopIcon = pTopJewel->GetControl(WSTR_ICON_JEWEL);
		pIconJewelTopIcon->Visible(false);

		XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pTopJewel->GetControl(L"BTN_CURSOR"));
		 
		pkBtn->Visible(false);
		XUI::CXUI_Wnd* pIconSlotTopBg = pMainWndItemTree->GetControl(L"ICON_SLOT_TOP_BG");
		pIconSlotTopBg->Visible(false);


		for(int i=0; i<ITEM_TREE_TOTAL_INDEX; ++i)
		{
			BM::vstring vstr(WSTR_JEWEL);
			BM::vstring vstrArrow(WSTR_ARROW);
			BM::vstring vstrItemIconSlotBg(WSTR_ITEM_ICON_SLOT_BG);
			vstr+=i;
			vstrArrow+=i;
			vstrItemIconSlotBg +=i;

			XUI::CXUI_Wnd* pJewelWnd = pMainWndItemTree->GetControl(vstr);
			XUI::CXUI_Wnd* pArrowWnd = pMainWndItemTree->GetControl(vstrArrow);
			XUI::CXUI_Wnd* pItemIconSlotBg = pMainWndItemTree->GetControl(vstrItemIconSlotBg);

			if(pJewelWnd)
			{
				XUI::CXUI_Wnd* pkNoItem = pJewelWnd->GetControl( std::wstring(L"IMG_NO_ITEM"));
				XUI::CXUI_Wnd* pkNoItemBg = pJewelWnd->GetControl( std::wstring(L"IMG_NUM_BG"));
				XUI::CXUI_Wnd* pkCountText = pJewelWnd->GetControl(std::wstring(L"FRM_COUNT_TEXT"));
				XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pJewelWnd->GetControl(L"BTN_CURSOR"));
							
				if(pkNoItem){pkNoItem->Visible(false);}
				if(pkNoItemBg){pkNoItemBg->Visible(false);}
				if(pkCountText){pkCountText->Text(_T(""));}
				if(pkBtn){ pkBtn->Visible(false);}

				XUI::CXUI_Icon* pIconJewel = dynamic_cast<XUI::CXUI_Icon*>(pJewelWnd->GetControl(WSTR_ICON_JEWEL));
				if(pIconJewel)
				{
					pIconJewel->ClearCustomData();

					XUI::CXUI_Wnd* pOrderIdx = pIconJewel->GetControl(L"FRM_ORDER_INDEX");
					if(pOrderIdx)
					{
						pOrderIdx->ClearCustomData();
					}
				}
			}

			if(pItemIconSlotBg)
			{
				pItemIconSlotBg->ClearCustomData();
				pItemIconSlotBg->Visible(false);
			}

			if(pArrowWnd)
			{
				XUI::CXUI_Image* pArrowUp = dynamic_cast<XUI::CXUI_Image*>(pArrowWnd->GetControl(L"IMG_UP_ARROW"));
				XUI::CXUI_Image* pArrowSide = dynamic_cast<XUI::CXUI_Image*>(pArrowWnd->GetControl(L"IMG_SIDE_ARROW"));

				if(pArrowUp){pArrowUp->Visible(false);}
				if(pArrowSide){pArrowSide->Visible(false);}
			}
		}
	}
}

void PgGemStore::Clear()
{
	m_iSelectedItemNo = 0;
	m_iOrderIndex = 0;
	m_kNpcGuid.Clear();
	m_kContAtricle.clear();
	m_kDefContAtricle.clear();
	m_eGemStoreType = EGT_OTHER;
	XUI::CXUI_Wnd* pMainWnd = XUIMgr.Get(WSTR_GEM_STORE.c_str());
	if(!pMainWnd)
	{
		pMainWnd = XUIMgr.Get(WSTR_CP_STORE.c_str());
	}
	if(pMainWnd)
	{
		for(int i=0; i<JEWEL_SLOT_CNT; ++i)
		{
			BM::vstring vstr(WSTR_JEWEL);
			vstr+=i;
			XUI::CXUI_Wnd* pJewelWnd = pMainWnd->GetControl(vstr);
			if(pJewelWnd)
			{
				XUI::CXUI_Icon* pIconJewel = dynamic_cast<XUI::CXUI_Icon*>(pJewelWnd->GetControl(WSTR_ICON_JEWEL));
				if(pIconJewel)
				{
					pIconJewel->ClearCustomData();
				}
				pJewelWnd->Text(_T(""));
			}
		}	
	}
}

void PgGemStore::DrawIconImage(lwUIWnd& UISelf)
{
	XUI::CXUI_Image* pIcon = dynamic_cast<XUI::CXUI_Image*>(UISelf.GetSelf());
	if( !pIcon )
	{ 
		return; 
	}

	int iItemNo;
	pIcon->GetCustomData(&iItemNo, sizeof(iItemNo));

	GET_DEF(CItemDefMgr, kItemDefMgr);
	const CItemDef* pItemDef = kItemDefMgr.GetDef(iItemNo);
	PgUISpriteObject* pkSprite = g_kUIScene.GetIconTexture(iItemNo);
	if( !pkSprite )
	{
		pIcon->DefaultImgTexture(NULL);
		pIcon->SetInvalidate();
		return;
	}

	PgUIUVSpriteObject* pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);
	if( !pkUVSprite ){ return; }
	
	pIcon->DefaultImgTexture(pkUVSprite);
	SUVInfo& rkUV = pkUVSprite->GetUVInfo();
	pIcon->UVInfo(rkUV);
	POINT2	kPoint(pIcon->Width()*rkUV.U, pIcon->Height()*rkUV.V);
	pIcon->ImgSize(kPoint);
	pIcon->SetInvalidate();
}

void PgGemStore::DrawItemTooltip(lwUIWnd& UISelf)
{
	int const iItemNo = UISelf.GetCustomData<int>();
	CallToolTip_ItemNo(iItemNo,UISelf.GetTotalLocation());
}

int PgGemStore::GetAmountItemPlayerHave(int const iItemNo)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pPlayer)
	{ 
		return 0; 
	}
	PgInventory* pInv = pPlayer->GetInven();
	if(!pInv)
	{ 
		return 0; 
	}
	return pInv->GetTotalCount(iItemNo);
}

int PgGemStore::GetItemPlayerHave(int const iItemNo)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pPlayer)
	{ 
		return 0; 
	}
	PgInventory* pInv = pPlayer->GetInven();
	if(!pInv)
	{ 
		return 0; 
	}

	return pInv->GetInvTotalCount(iItemNo);
}

PgGemStore::E_CHECK_ITEM_RESULT PgGemStore::isEnoughTrade(int const iSelectedItemNo,int const iOrderIndex)
{// 보석 교환이 가능한가?
	XUI::CXUI_Wnd* pMainWnd = XUIMgr.Get(WSTR_GEM_STORE.c_str());
	if(!pMainWnd)
	{
		pMainWnd = XUIMgr.Get(WSTR_CP_STORE.c_str());
		if( !pMainWnd )
		{
			return EIR_FAIL;
		}
	}
	CONT_GEMSTORE_ARTICLE::const_iterator itor_Article = m_kContAtricle.find(iSelectedItemNo);
	if(itor_Article == m_kContAtricle.end())
	{// 플레이어가 교환하고자 하는 아이템에 필요한
		return EIR_FAIL;
	}

	E_CHECK_ITEM_RESULT isEnough = EIR_OK;
	SGEMSTORE_ARTICLE const& kItem = (*itor_Article).second;		
	CONT_GEMS const& kContGems = kItem.kContGems;
	CONT_GEMS::const_iterator itor_ReqGem = kContGems.begin();	

	if( kItem.iCP != 0 )
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			if( pkPlayer->GetAbil(AT_CP) < kItem.iCP )
			{
				return EIR_LACK_CP;
			}
		}
	}

	for(int i=0; i<JEWEL_SLOT_CNT; ++i)
	{// 충분한 보석이 있는지를 검사 하고
		BM::vstring vstr(WSTR_JEWEL);
		vstr+=i;
		XUI::CXUI_Wnd* pJewelWnd = pMainWnd->GetControl(vstr);
		if(pJewelWnd)
		{
			XUI::CXUI_Icon* pIconJewel = dynamic_cast<XUI::CXUI_Icon*>(pJewelWnd->GetControl(WSTR_ICON_JEWEL));
			if(pIconJewel)
			{
				// 더불어 보석 요구량 아이콘을 갱신하고 (수량 변화가 있었을수 있으므로)
				pIconJewel->ClearCustomData();
				pJewelWnd->Text(_T(""));
				if(itor_ReqGem != kContGems.end())
				{
					int const iGemNo = (*itor_ReqGem).first;
					pIconJewel->SetCustomData(&iGemNo, sizeof(iGemNo));
					short const sReqCnt = (*itor_ReqGem).second;
					std::wstring kJewelNumStrForm = TTW(790121);				
					int iJewelNumIhave = GetAmountItemPlayerHave(iGemNo);
					if(sReqCnt <= iJewelNumIhave )
					{
						iJewelNumIhave = sReqCnt;
					}
					else
					{// 보석이 하나라도 부족하다면, 트레이드 불가 설정하여
						isEnough = (m_bIsCPStore)?(EIR_LACK_HERO_MARK):(EIR_LACK_JEWEL);
					}
					wchar_t buf[100] = {0,};
					wsprintfW(buf, kJewelNumStrForm.c_str(), iJewelNumIhave, sReqCnt);
					pJewelWnd->Text(buf);
					{
						pIconJewel->GrayScale(iJewelNumIhave<sReqCnt);
					}
					++itor_ReqGem;
				}
			}
		}
	}
	// 교환 가능 여부를 전달한다
	return isEnough;
}

void PgGemStore::ClearNeedItemImg(XUI::CXUI_Wnd* pMainWnd)
{// 해당 버튼만 체크 상태로 만들고, 나머지 list item의 체크를 푼다
	if(!pMainWnd){ return; }

	for(int i=0; i<JEWEL_SLOT_CNT; ++i)
	{// 필요한		
		BM::vstring vstr(WSTR_JEWEL);
		vstr+=i;
		XUI::CXUI_Wnd* pJewelWnd = pMainWnd->GetControl(vstr);
		if(pJewelWnd)
		{
			XUI::CXUI_Icon* pIconJewel = dynamic_cast<XUI::CXUI_Icon*>(pJewelWnd->GetControl(WSTR_ICON_JEWEL));
			if(pIconJewel)
			{
				pIconJewel->ClearCustomData();
			}
			pJewelWnd->Text(_T(""));
		}
	}
}

void PgGemStore::SendReqDefGemStoreInfo(BM::GUID& rkNpcGuid, int const iMenu, bool const bCheckClass)
{// 상점정보를 요청하고
	m_kNpcGuid = rkNpcGuid;
	m_bCheckClass = bCheckClass;
	BM::Stream kPacket(PT_C_M_REQ_DEFGEMSTOREINFO);
	kPacket.Push(rkNpcGuid);
	kPacket.Push(iMenu);
	NETWORK_SEND(kPacket)
}

void PgGemStore::RecvDefGemstoreInfo(BM::Stream& rkPacket)
{// 보석상점 정보를 받은 후	
	SDEFGEMSTORE kStoreInfo;
	int iMenu = 0;

	kStoreInfo.ReadFromPacket(rkPacket);
	rkPacket.Pop( iMenu );

	m_eGemStoreType = static_cast<E_GEMSTORE_TYPE>(iMenu);

	m_kDefContAtricle = kStoreInfo.kContArticles;

	// 상점에서 파는 아이템들 중
	CONT_DEFGEMSTORE_ARTICLE::iterator itor_Article = m_kDefContAtricle.begin();
	while( itor_Article != m_kDefContAtricle.end() )
	{//아무것도 없네.
		CONT_DEFGEMSTORE_ARTICLE::key_type const& kArticleKey = itor_Article->first;
		CONT_DEFGEMSTORE_ARTICLE::mapped_type const& kArticle = itor_Article->second;

		if( iMenu != kArticle.iMenu )
		{
			itor_Article = m_kDefContAtricle.erase(itor_Article);
			continue;
		}
		++itor_Article;
	}

	XUI::CXUI_Wnd* pkParentWnd = NULL;

	pkParentWnd = CallGemStoreUIandGetListWnd();

	if(!pkParentWnd)
	{
		return;
	}

	SetDefStoreSlot(pkParentWnd);
}

bool PgGemStore::SetDefStoreSlot(XUI::CXUI_Wnd* pkMain)
{
	BM::CAutoMutex kLock(m_kMutex);

	if(!pkMain)
	{
		return false;
	}

	VEC_GEMITEM_INFO kTemp;
	{
		typedef std::set<std::pair<int, int> > CONT_SORT;
		CONT_SORT kContSort;
		CONT_DEFGEMSTORE_ARTICLE::iterator itor_Article = m_kDefContAtricle.begin();
		while( itor_Article != m_kDefContAtricle.end() )
		{
			CONT_DEFGEMSTORE_ARTICLE::key_type const& kArticleKey = itor_Article->first;
			CONT_DEFGEMSTORE_ARTICLE::mapped_type const& kArticle = itor_Article->second;

			kContSort.insert(std::make_pair(kArticle.iOrderIndex,kArticleKey.iItemNo));
			++itor_Article;
		}
		CONT_SORT::iterator sort_it = kContSort.begin();
		while( sort_it != kContSort.end() )
		{
			VEC_GEMITEM_INFO::value_type kValue;
			kValue.iOrderIndex = (*sort_it).first;
			kValue.iItemNo = (*sort_it).second;

			
			if(ITEM_TREE_NO_DISPLAY == kValue.iOrderIndex)
			{
				++sort_it;
				continue;
			}

			if( m_bCheckClass )
			{
				if( CheckMyClassItem(kValue.iItemNo) )
				{
					kTemp.push_back(kValue);
				}
			}
			else
			{
				kTemp.push_back(kValue);
			}
			++sort_it;
		}
	}

	int MAX_ITEM_SLOT = 20;
	XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkMain->GetControl(L"BLD_SLOT"));
	if( pkBuild )
	{
		MAX_ITEM_SLOT = pkBuild->CountX() * pkBuild->CountY();
	}
	m_kPage.SetPageAttribute(MAX_ITEM_SLOT, 2);
	m_kPage.SetMaxItem(kTemp.size());
	UIPageUtil::PageControl(pkMain->GetControl(L"FRM_PAGE"), m_kPage);

	VEC_GEMITEM_INFO::const_iterator itor_Article = kTemp.begin();

	int const iIgnoreItemCount = m_kPage.Now() * m_kPage.GetMaxItemSlot();
	for(int i = 0; i < iIgnoreItemCount; ++i)
	{
		if( itor_Article == kTemp.end() )
		{
			return false;
		}
		++itor_Article;
	}

	for( int i = 0; i < m_kPage.GetMaxItemSlot(); ++i )
	{
		BM::vstring kStr(L"ICON_SLOT");
		kStr += i;

		XUI::CXUI_Wnd* pkImg = pkMain->GetControl(kStr);
		if( !pkImg )
		{
			continue;
		}

		BM::vstring kStrCount(L"ICON_SLOT_COUNT");
		kStrCount += i;
		
		XUI::CXUI_Wnd* pkCount = pkMain->GetControl(kStrCount);
		if( !pkCount )
		{
			continue;
		}

		BM::vstring kStrOrderIndex(L"ICON_SLOT_ORDER_INDEX");
		kStrOrderIndex += i;

		XUI::CXUI_Wnd* pkOrderIndex = pkMain->GetControl(kStrOrderIndex);
		if( !pkOrderIndex )
		{
			continue;
		}		

		pkImg->ClearCustomData();
		pkCount->Text(L"");
		pkOrderIndex->ClearCustomData();
		pkOrderIndex->Text(L"");
		if(itor_Article != kTemp.end())
		{// 이 아이템을 ( SGEMSTORE_ARTICLE(아이템번호, CONT_GEM-필요보석들) )	
			VEC_GEMITEM_INFO::value_type kValue = (*itor_Article);
			SetDefSlotItem(pkImg, kValue.iItemNo);
			SetDefSlotItem(pkOrderIndex, kValue.iOrderIndex);

			++itor_Article;

			CONT_DEFGEMSTORE_ARTICLE::key_type kKey(kValue.iItemNo, static_cast<int>(m_eGemStoreType), kValue.iOrderIndex);
			CONT_DEFGEMSTORE_ARTICLE::const_iterator itor = m_kDefContAtricle.find(kKey);
			if( itor != m_kDefContAtricle.end() )
			{
				int const iCount = itor->second.iItemCount;

				if( 1 < iCount )
				{
					BM::vstring const kCount( itor->second.iItemCount );
					pkCount->Text(kCount);
					continue;
				}
			}
			pkCount->Text(L"");
		}
 	}

	return true;
}

bool PgGemStore::SetDefSlotItem(XUI::CXUI_Wnd* pkIcon, int const iItemNo)
{
	if( pkIcon )
	{
		//if( CheckMyClassItem(iItemNo) )
		{
			pkIcon->SetCustomData(&iItemNo, sizeof(iItemNo));

			XUI::CXUI_Image* pkIconImg = dynamic_cast<XUI::CXUI_Image*>(pkIcon);
			if(pkIconImg)
			{
				SetUIAddResToImage(pkIconImg, iItemNo);
			}
			return true;
		}
	}
	return false;
}

void PgGemStore::ClickDefSlotItem(lwUIWnd& klwChkBtn)
{// 해당 버튼만 체크 상태로 만들고, 나머지 list item의 체크를 푼다
	XUI::CXUI_Wnd* pkIconWnd = klwChkBtn.GetSelf();
	if( !pkIconWnd ){ return; }

	XUI::CXUI_Wnd* pMainWnd = pkIconWnd->Parent();
	if(!pMainWnd){ return; }

	pkIconWnd->GetCustomData(&m_iSelectedItemNo, sizeof(m_iSelectedItemNo));
	
	BM::vstring kStrOrderIndex(L"ICON_SLOT_ORDER_INDEX");
	kStrOrderIndex += pkIconWnd->BuildIndex();

	XUI::CXUI_Wnd* pkOrderIndex = pMainWnd->GetControl(kStrOrderIndex);
	if( !pkOrderIndex )
	{
		return;
	}		
	
	pkOrderIndex->GetCustomData(&m_iOrderIndex, sizeof(m_iOrderIndex));

	CONT_DEFGEMSTORE_ARTICLE::key_type kKey(m_iSelectedItemNo, static_cast<int>(m_eGemStoreType), m_iOrderIndex);
	CONT_DEFGEMSTORE_ARTICLE::const_iterator itor_Article = m_kDefContAtricle.find(kKey);
	if(itor_Article == m_kDefContAtricle.end())
	{
		return;
	}

	SDEFGEMSTORE_ARTICLE const& kItem = (*itor_Article).second;		
	CONT_DEFGEMS const& kContGems = kItem.kContGems;
	CONT_DEFGEMS::const_iterator itor_ReqGem = kContGems.begin();

	for(int i=0; i<JEWEL_SLOT_CNT; ++i)
	{// 필요한		
		BM::vstring vstr(WSTR_JEWEL);
		vstr+=i;
		XUI::CXUI_Wnd* pJewelWnd = pMainWnd->GetControl(vstr);
		if(pJewelWnd)
		{
			XUI::CXUI_Icon* pIconJewel = dynamic_cast<XUI::CXUI_Icon*>(pJewelWnd->GetControl(WSTR_ICON_JEWEL));
			if(pIconJewel)
			{
				pIconJewel->ClearCustomData();
				pJewelWnd->Text(_T(""));
				if(itor_ReqGem != kContGems.end())
				{
					// 보석 번호와
					int const iGemNo = (*itor_ReqGem).iGemNo;
					pIconJewel->SetCustomData(&iGemNo, sizeof(iGemNo));

					// 필요 갯수를 찾고
					short const sReqCnt = (*itor_ReqGem).sCount;

					std::wstring kJewelNumStrForm = TTW(790121);
					// 플레이어가 소지한 해당 보석의 갯수를 찾고
					int iJewelNumIhave = GetItemPlayerHave(iGemNo);
					if(sReqCnt <= iJewelNumIhave )
					{
						iJewelNumIhave = sReqCnt;
					}
					// 하단부 UI에 수량을 표현하고
					wchar_t buf[100] = {0,};
					wsprintfW(buf, kJewelNumStrForm.c_str(), iJewelNumIhave, sReqCnt);
					pJewelWnd->Text(buf);
					{// 아이템의 갯수가 부족하면 회색, 아니면 정상 출력한다				
						pIconJewel->GrayScale(iJewelNumIhave<sReqCnt);
					}
					++itor_ReqGem;
				}
			}
		}
	}
}

void PgGemStore::UpdateItemTree()
{
	SetItemTreeSlot(m_iTreeItemNo, m_iTreeOrderIndex);
}

bool PgGemStore::IsItemTreeSlot(int const iSelectedItemNo, int const iOrderIndex)const
{
	if((m_eGemStoreType != EGT_TREE_ITEM) &&
	   (m_eGemStoreType != EGT_ITEM_SHOP) &&
	   (m_eGemStoreType != EGT_ITEM_PLUS))
	{	
		return false;
	}

	CONT_DEFGEMSTORE_ARTICLE::key_type const kKey(iSelectedItemNo, static_cast<int>(m_eGemStoreType), iOrderIndex);
	CONT_DEFGEMSTORE_ARTICLE::const_iterator itor_Article = m_kDefContAtricle.find(kKey);
	if(itor_Article == m_kDefContAtricle.end())
	{
		lwAddWarnDataTT(798830);
		return false;
	}
	return true;
}

void PgGemStore::SetItemTreeSlot(int const iSelectedItemNo, int const iOrderIndex)
{
	if(false == IsItemTreeSlot(iSelectedItemNo, iOrderIndex))
	{
		return;
	}

	XUI::CXUI_Wnd * pMainWnd = XUIMgr.Get(_T("SFRM_JEWEL_CONSTELLATION"));
	if(NULL == pMainWnd)
	{
		m_kContTreeHistory.clear();
		pMainWnd = XUIMgr.Call(_T("SFRM_JEWEL_CONSTELLATION"));
	}
	else
	{
		CONT_TREE_HISTORY::value_type kData;
		if(false == m_kContTreeHistory.empty())
		{
			kData = m_kContTreeHistory.back();
		}
		if( ((m_iTreeItemNo!=iSelectedItemNo) && (m_iTreeOrderIndex!=iOrderIndex)) && ((kData.first!=m_iTreeItemNo) && (kData.second!=m_iTreeOrderIndex)) )
		{
			m_kContTreeHistory.push_back(std::make_pair(m_iTreeItemNo, m_iTreeOrderIndex));
		}
		if(m_kContTreeHistory.size() > ITEM_TREE_HISTORY_COUNT)
		{
			m_kContTreeHistory.pop_front();
		}
	}

	m_iTreeItemNo = iSelectedItemNo;
	m_iTreeOrderIndex = iOrderIndex;


	XUI::CXUI_Wnd * pTabBg = pMainWnd->GetControl(L"SFRM_TAB_BG");
	if(NULL == pTabBg)
	{
		return;
	}
	
	wchar_t const * pItemName = NULL;
	BM::vstring vStr;
	GetDefString(m_iTreeItemNo, pItemName);	// 아이템 이름
	if( pItemName )
	{
		vStr = L"[";
		vStr += pItemName;
		vStr += L"]";
		vStr += L"  ";
		vStr += TTW(798831);
	}

	Quest::SetCutedTextLimitLength(pTabBg,vStr,std::wstring(_T("...")));


	if( pMainWnd )
	{
		//CenterFind
		typedef std::deque<GEM_STORE::SDefGems> CONT_DATA;
		CONT_DATA kCont;

		GEM_STORE::SDefGems kDefGems;
		int iCenterNo = m_iTreeItemNo;
		int iUpCount = 0;
		while(GEM_STORE::FindUpDefGems(iCenterNo, m_eGemStoreType, kDefGems) )
		{
			iCenterNo = kDefGems.iItemNo;
			kCont.push_front(kDefGems);
			++iUpCount;
		}

		GEM_STORE::FindDefGems(m_iTreeItemNo, m_eGemStoreType, kDefGems);
		iCenterNo = (false == kDefGems.pkDefGems->empty()) ? kDefGems.pkDefGems->begin()->iGemNo : 0;
		kCont.push_back(kDefGems);
		int const iIndex = kCont.size();

		int iDownCount = 0;
		while(GEM_STORE::FindDefGems(iCenterNo, m_eGemStoreType, kDefGems) )
		{
			iCenterNo = (false == kDefGems.pkDefGems->empty()) ? kDefGems.pkDefGems->begin()->iGemNo : 0;
			kCont.push_back(kDefGems);
			++iDownCount;
		}

		if(false==kCont.empty())
		{
			kCont.push_front( kCont.front() );
			kCont.front().bTop = true;
		}

		for(int i = iUpCount; 2 < i && kCont.size() > JEWEL_CONSTELLATION_SLOT_Y_CNT-1; --i)
		{// 오버한 수량 빼기
			kCont.pop_front();
		}

		//
		ItemTreeClear();
		UpdateItemTreeHistory(pMainWnd);
		for(int iRow = 0; (iRow < ITEM_TREE_TOTAL_INDEX) && (false == kCont.empty()); iRow += JEWEL_CONSTELLATION_SLOT_X_CNT)
		{
			DrawItemTree(pMainWnd->GetControl(L"FRM_UI"), iRow, kCont.front());
			kCont.pop_front();
		}
	}
}


void PgGemStore::UpdateItemTreeHistory(XUI::CXUI_Wnd * pMainWnd)
{
	if( NULL == pMainWnd )
	{
		return;
	}

	CONT_TREE_HISTORY::const_iterator history_it = m_kContTreeHistory.begin();
	for(int index = 0; index < ITEM_TREE_HISTORY_COUNT; ++index)
	{
		BM::vstring vstrSlotBg(L"SFRM_HISTORY_SLOT_BG");
		vstrSlotBg += index;

		if(XUI::CXUI_Wnd* pIconSlotBg = pMainWnd->GetControl(vstrSlotBg))
		{
			XUI::CXUI_Icon* pIconJewel = pIconSlotBg ? dynamic_cast<XUI::CXUI_Icon*>(pIconSlotBg->GetControl(L"ICON_JEWEL")) : NULL;
			XUI::CXUI_Wnd* pOrderIndex = pIconJewel ? pIconJewel->GetControl(L"FRM_ORDER_INDEX") : NULL;
			if(pIconJewel && pOrderIndex)
			{
				int iItemNo = 0;
				int iOrderIndex = 0;
				if(history_it != m_kContTreeHistory.end())
				{
					iItemNo = (*history_it).first;
					iOrderIndex = (*history_it).second;
					++history_it;
				}
				pIconJewel->SetCustomData(&iItemNo, sizeof(iItemNo));
				pOrderIndex->SetCustomData(&iOrderIndex, sizeof(iOrderIndex));
			}
		}
	}
	
}

void PgGemStore::DrawItemTree(XUI::CXUI_Wnd* pMainWnd, int const Row, GEM_STORE::SDefGems const& kSlotItem)
{
	if(NULL == pMainWnd)
	{
		return;
	}

	CONT_DEFGEMS::const_iterator next_it;
	if(kSlotItem.pkDefGems)
	{
		next_it = kSlotItem.pkDefGems->begin();
	}

	for( int index = 0; index < JEWEL_CONSTELLATION_SLOT_X_CNT; ++index )
	{
		BM::vstring vstr(WSTR_JEWEL);
		BM::vstring vstrArrow(WSTR_ARROW);
		BM::vstring vstrItemIconSlotBg(WSTR_ITEM_ICON_SLOT_BG);
		vstr += (index + Row);
		vstrArrow += (index + Row);//화살표
		vstrItemIconSlotBg += (index + Row);

		XUI::CXUI_Wnd* pJewelWnd = pMainWnd->GetControl(vstr);
		XUI::CXUI_Wnd* pArrowWnd = pMainWnd->GetControl(vstrArrow);
		XUI::CXUI_Wnd* pItemIconSlotBg = pMainWnd->GetControl(vstrItemIconSlotBg);
		XUI::CXUI_Icon* pIconJewel = pJewelWnd ? dynamic_cast<XUI::CXUI_Icon*>(pJewelWnd->GetControl(WSTR_ICON_JEWEL)) : NULL;

		bool bVisibleArrowUp = false;
		bool bVisibleArrowSide = false;
		bool bVisibleItemIconSlotBg = false;

		//조합할 아이템 선택UI 표시
		if(pJewelWnd && pItemIconSlotBg && pArrowWnd)
		{
			pJewelWnd->Text(_T(""));

			if(0 == index && m_iTreeItemNo == kSlotItem.iItemNo && false == kSlotItem.bTop)
			{
				if(XUI::CXUI_Wnd* pCenterWnd = pMainWnd->GetControl(L"FRM_CENTER_ITEM"))
				{
					POINT3I kPos = pCenterWnd->Location();
					kPos.y = pJewelWnd->Location().y - (pItemIconSlotBg->Size().y+pArrowWnd->Size().y);
					pCenterWnd->Location( kPos );
				}
			}
		}

		//아이템 세팅
		if(0 == index && kSlotItem.bTop)
		{
			XUI::CXUI_Wnd* pTopJewel = pMainWnd->GetControl(L"FRM_TOP_JEWEL");
			XUI::CXUI_Wnd* pIconSlotTopBg = pMainWnd->GetControl(L"ICON_SLOT_TOP_BG");
			XUI::CXUI_Icon* pIconJewelTopIcon = pTopJewel ? dynamic_cast<XUI::CXUI_Icon*>(pTopJewel->GetControl(WSTR_ICON_JEWEL)) : NULL;
			XUI::CXUI_Button* pkBtn = pTopJewel ? dynamic_cast<XUI::CXUI_Button*>(pTopJewel->GetControl(L"BTN_CURSOR")) : NULL;

			ItemTreeSetCustomData(pTopJewel,pIconJewelTopIcon, kSlotItem.iItemNo, kSlotItem.iItemCount, kSlotItem.iOrderIndex);
			
			pkBtn->Visible(true);
			pIconSlotTopBg->Visible(true);
			pIconJewelTopIcon->Visible(true);
		}
		else if(!kSlotItem.bTop && kSlotItem.pkDefGems && (next_it != kSlotItem.pkDefGems->end()))
		{
			int const iItemNo = (*next_it).iGemNo;
			int const iCount = (*next_it).sCount;
			int const iOrderIndex = GEM_STORE::GetDefOrderIndex(iItemNo, m_eGemStoreType);
			ItemTreeSetCustomData(pJewelWnd, pIconJewel, iItemNo, iCount, iOrderIndex);
			++next_it;

			bVisibleArrowUp = (0 == index);
			bVisibleArrowSide = (next_it != kSlotItem.pkDefGems->end());
			bVisibleItemIconSlotBg = true;
		}

		//아이콘 배경 표시
		if(pItemIconSlotBg)
		{
			pItemIconSlotBg->Visible(bVisibleItemIconSlotBg);
		}

		//화살표 표시
		if(pArrowWnd)
		{
			XUI::CXUI_Image* pArrowUp = dynamic_cast<XUI::CXUI_Image*>(pArrowWnd->GetControl(L"IMG_UP_ARROW"));
			XUI::CXUI_Image* pArrowSide = dynamic_cast<XUI::CXUI_Image*>(pArrowWnd->GetControl(L"IMG_SIDE_ARROW"));
			if(pArrowUp && pArrowSide)
			{
				pArrowUp->Visible(bVisibleArrowUp);
				pArrowSide->Visible(bVisibleArrowSide);
			}
		}
	}
	return;
}

void PgGemStore::ItemTreeSetCustomData(XUI::CXUI_Wnd* pJewelWnd,XUI::CXUI_Icon* pIconJewel,int iItemNo,int iNeedItemNo, int const iOrderIndex)
{
	if(pIconJewel)
	{
		pIconJewel->SetCustomData(&iItemNo, sizeof(iItemNo));
		if(XUI::CXUI_Wnd* pOrderIdx = pIconJewel->GetControl(L"FRM_ORDER_INDEX"))
		{
			pOrderIdx->SetCustomData(&iOrderIndex, sizeof(iOrderIndex));
		}
	}

	if(pJewelWnd)
	{
		XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pJewelWnd->GetControl(L"BTN_CURSOR"));
		pkBtn->Visible(true);
		XUI::CXUI_Wnd* pkNoItem = pJewelWnd->GetControl( std::wstring(L"IMG_NO_ITEM"));
		if(NULL == pkNoItem){return;}
		XUI::CXUI_Wnd* pkNoItemBg = pJewelWnd->GetControl( std::wstring(L"IMG_NUM_BG"));
		XUI::CXUI_Wnd* pkCountText = pJewelWnd->GetControl(std::wstring(L"FRM_COUNT_TEXT"));
		

		// 필요 갯수를 찾고
		bool bNoItem = true;
		pkNoItem->Visible(bNoItem);
		pkNoItemBg->Visible(bNoItem);

		std::wstring kJewelNumStrForm = TTW(790121);
		// 플레이어가 소지한 해당 보석의 갯수를 찾고
		int iJewelNumIhave = GetItemPlayerHave(iItemNo);

		// 하단부 UI에 수량을 표현하고
		wchar_t buf[100] = {0,};
		wsprintfW(buf, kJewelNumStrForm.c_str(), iJewelNumIhave, iNeedItemNo);
		pkCountText->Text(buf);
		{// 아이템의 갯수가 부족하면 회색, 아니면 정상 출력한다
			if(iNeedItemNo <= iJewelNumIhave)
			{
				bNoItem = false;
				pkNoItem->Visible(bNoItem);
			}
		}
	}
}

PgGemStore::E_CHECK_ITEM_RESULT PgGemStore::isDefEnoughTrade(int const iSelectedItemNo, int const iOrderIndex)
{// 보석 교환이 가능한가?
	XUI::CXUI_Wnd* pMainWnd = XUIMgr.Get(WSTR_GEM_STORE.c_str());
	if(!pMainWnd)
	{
		pMainWnd = XUIMgr.Get(WSTR_CP_STORE.c_str());
		if( !pMainWnd )
		{
			return EIR_FAIL;
		}
	}

	CONT_DEFGEMSTORE_ARTICLE::key_type kKey(iSelectedItemNo, static_cast<int>(m_eGemStoreType), iOrderIndex);
	CONT_DEFGEMSTORE_ARTICLE::const_iterator itor_Article = m_kDefContAtricle.find(kKey);
	if(itor_Article == m_kDefContAtricle.end())
	{// 플레이어가 교환하고자 하는 아이템에 필요한
		return EIR_FAIL;
	}

	E_CHECK_ITEM_RESULT isEnough = EIR_OK;
	SDEFGEMSTORE_ARTICLE const& kItem = (*itor_Article).second;		
	CONT_DEFGEMS const& kContGems = kItem.kContGems;
	CONT_DEFGEMS::const_iterator itor_ReqGem = kContGems.begin();	

	/*if( kItem.iCP != 0 )
	{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
	if( pkPlayer->GetAbil(AT_CP) < kItem.iCP )
	{
	return EIR_LACK_CP;
	}
	}
	}*/

	for(int i=0; i<JEWEL_SLOT_CNT; ++i)
	{// 충분한 보석이 있는지를 검사 하고
		BM::vstring vstr(WSTR_JEWEL);
		vstr+=i;
		XUI::CXUI_Wnd* pJewelWnd = pMainWnd->GetControl(vstr);
		if(pJewelWnd)
		{
			XUI::CXUI_Icon* pIconJewel = dynamic_cast<XUI::CXUI_Icon*>(pJewelWnd->GetControl(WSTR_ICON_JEWEL));
			if(pIconJewel)
			{// 더불어 보석 요구량 아이콘을 갱신하고 (수량 변화가 있었을수 있으므로)
				pIconJewel->ClearCustomData();
				pJewelWnd->Text(_T(""));
				if(itor_ReqGem != kContGems.end())
				{
					int const iGemNo = (*itor_ReqGem).iGemNo;
					pIconJewel->SetCustomData(&iGemNo, sizeof(iGemNo));
					short const sReqCnt = (*itor_ReqGem).sCount;
					std::wstring kJewelNumStrForm = TTW(790121);				
					int iJewelNumIhave = GetItemPlayerHave(iGemNo);
					if(sReqCnt <= iJewelNumIhave )
					{
						iJewelNumIhave = sReqCnt;
					}
					else
					{// 보석이 하나라도 부족하다면, 트레이드 불가 설정하여
						isEnough = (m_bIsCPStore)?(EIR_LACK_HERO_MARK):(EIR_LACK_JEWEL);
					}
					wchar_t buf[100] = {0,};
					wsprintfW(buf, kJewelNumStrForm.c_str(), iJewelNumIhave, sReqCnt);
					pJewelWnd->Text(buf);
					{
						pIconJewel->GrayScale(iJewelNumIhave<sReqCnt);
					}
					++itor_ReqGem;
				}
			}
		}
	}
	// 교환 가능 여부를 전달한다
	return isEnough;
}

void PgGemStore::RecvDefGemTradeResult(BM::Stream& rkPacket)
{// 교환 결과를 받아 처리 
	HRESULT kResult;
	rkPacket.Pop(kResult);
	switch(kResult)
	{
	case E_NOT_FOUND_GEMSTORE:
		{// 보석교환 상인을 찾을 수 없음
			::Notice_Show(TTW(790123), EL_Warning);
		}break;
	case E_NOT_FOUND_ARTICLE:
		{// 교환물품을 찾을 수 없음
			::Notice_Show(TTW(790124), EL_Warning);
		}break;
	case E_NOT_ENOUGH_GEMS:
		{// 보석이 부족함
			::Notice_Show(TTW(790125), EL_Warning);
		}break;
	case S_OK:
		{
			XUI::CXUI_Wnd* pkParentWnd = NULL;
			if( !m_bIsCPStore )
			{
				pkParentWnd = CallGemStoreUIandGetListWnd();
			}

			if(pkParentWnd)
			{
				XUI::CXUI_Wnd* pkPage = dynamic_cast<XUI::CXUI_Wnd*>(pkParentWnd->GetControl(L"FRM_PAGE"));
				if( !pkPage )
				{
					return;
				}				
				ResetCursor(lwUIWnd(pkPage));
			}
		}break;
	default:
		{
		}break;
	}
}

int PgGemStore::GetGemStoreTitleMenu()
{
	if( true == IsOtherMode() )
	{
		return static_cast<int>(EGT_OTHER);
	}

	if( EGT_JEWEL_1 == m_eGemStoreType )
	{
		return static_cast<int>(EGT_JEWEL_1);
	}
	else if( EGT_JOBSKILL_1 == m_eGemStoreType )
	{
		return static_cast<int>(EGT_JOBSKILL_1);
	}
	else if( EGT_JOBSKILL_2 == m_eGemStoreType )
	{
		return static_cast<int>(EGT_JOBSKILL_2);
	}
	else if( EGT_ELEMENT == m_eGemStoreType )
	{
		return static_cast<int>(EGT_ELEMENT);
	}
	else if( EGT_CARDBOX == m_eGemStoreType )
	{
		return static_cast<int>(EGT_CARDBOX);
	}
	else if( EGT_GATHER == m_eGemStoreType )
	{
		return static_cast<int>(EGT_GATHER);
	}
	else if( true == IsEventMode() )
	{
		return static_cast<int>(EGT_EVENT);
	}
	else if( EGT_VALKYRIE == m_eGemStoreType )
	{
		return static_cast<int>(EGT_VALKYRIE);
	}
	else if( EGT_ITEM == m_eGemStoreType )
	{
		return static_cast<int>(EGT_ITEM);
	}
	else if( EGT_TREE_ITEM == m_eGemStoreType )
	{
		return static_cast<int>(EGT_TREE_ITEM);
	}
	else if( EGT_ITEM_SHOP == m_eGemStoreType )
	{
		return static_cast<int>(EGT_ITEM_SHOP);
	}
	else if( EGT_ITEM_PLUS == m_eGemStoreType )
	{
		return static_cast<int>(EGT_ITEM_PLUS);
	}

	return static_cast<int>(EGT_JEWEL_2);

}

void PgGemStore::ResetCursor(lwUIWnd& kSelf)
{
	if( kSelf.IsNil() ){ return; }

	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf ){ return; }

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent ){ return; }

	XUI::CXUI_Wnd* pSelectImg = pParent->GetControl(L"IMG_SELECT");
	if( !pSelectImg ){ return; }

	int const iNewBuildIndex = pSelf->BuildIndex();
	int const iOldBuildIndex = lwUIWnd(pSelectImg).GetCustomData<int>();

	if( pSelectImg->Visible() )
	{
		pSelectImg->Visible(false);
		pSelectImg->ClearCustomData();

		g_kGemStore.ClearNeedItemImg(pParent);
	}

	m_iSelectedItemNo = 0;
	m_iOrderIndex = 0;
}