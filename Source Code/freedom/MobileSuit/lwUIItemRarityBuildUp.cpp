#include "StdAfx.h"
#include "lwUIItemRarityBuildUp.h"
#include "lwUIToolTip.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgMobileSuit.h"
#include "PgSoundMan.h"

int const MAX_OPTION_COUNT = 4;//최대 옵션 갯수
float const BUILDUP_PROGRESS_TIME = 6.0f;//연출 소요 시간
float const BUILDUP_PROGRESS_SPEED = 0.2f;
DWORD const HIGHLIGHT_COLOR = 0xFFFF0000;

void lwUIItemRarityBuildUp::RegisterWrapper( lua_State *pkState )
{
	using namespace lua_tinker;
	def(pkState, "Clear_RarityBuildUp", lwUIItemRarityBuildUp::Clear);
	def(pkState, "Display_RarityBuildUp", lwUIItemRarityBuildUp::DisplaySrcIcon);
	def(pkState, "DisplayResultItem_RarityBuildUp", lwUIItemRarityBuildUp::DisplayResultItem);
	def(pkState, "OnTick_RarityBuildUp", lwUIItemRarityBuildUp::OnTick);
	def(pkState, "CheckUI_RarityBuildUp", lwUIItemRarityBuildUp::CheckUI);
	def(pkState, "Start_RarityBuildUp", lwUIItemRarityBuildUp::Start);
	def(pkState, "CallItemBuildUpUI", lwUIItemRarityBuildUp::CallItemBuildUpUI);
	def(pkState, "CallItemBuildUpSelectUI", lwUIItemRarityBuildUp::CallItemBuildUpSelectUI);
	def(pkState, "Cancle_RarityBuildUp", lwUIItemRarityBuildUp::Cancle);
	def(pkState, "Check_RarityBuildUp", lwUIItemRarityBuildUp::Check);

}

void lwUIItemRarityBuildUp::CallItemBuildUpSelectUI()
{
	CXUI_Wnd* pkTopWnd = XUIMgr.Call(L"ITEM_RARITY_BUILD_UP_SELECT");
	if( pkTopWnd )
	{
		RegistUIAction(pkTopWnd);
	}
}

void lwUIItemRarityBuildUp::CallItemBuildUpUI()
{
	CXUI_Wnd* pkTopWnd = XUIMgr.Call(L"SFRM_ITEM_RARITY_BUILDUP");
	if( pkTopWnd )
	{
		RegistUIAction(pkTopWnd);
	}
}

void lwUIItemRarityBuildUp::Clear()
{
	g_kItemRarityBuildUpMgr.Clear();
}

void lwUIItemRarityBuildUp::DisplaySrcIcon()
{
	g_kItemRarityBuildUpMgr.DisplaySrcIcon();
}

void lwUIItemRarityBuildUp::DisplayResultItem()
{
	g_kItemRarityBuildUpMgr.DisplayResultItem();
}

void lwUIItemRarityBuildUp::OnTick(  )
{
	g_kItemRarityBuildUpMgr.OnTick();
}

bool lwUIItemRarityBuildUp::CheckUI(lwUIWnd kWnd)
{
	XUI::CXUI_CheckButton* pkSelf = dynamic_cast<XUI::CXUI_CheckButton*>(kWnd.GetSelf());
	if( !pkSelf )
	{
		return false;
	}

	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
	if( pkWndTop )
	{
		bool bCheck = false;
		for( int iIndex=0; iIndex<MAX_OPTION_COUNT; ++iIndex)
		{
			BM::vstring vStrWndName(L"SFRM_OPTION");
			vStrWndName += iIndex;
			XUI::CXUI_Wnd* pkWndOption = pkWndTop->GetControl(vStrWndName.operator wchar_t const*());
			if( !pkWndOption )
			{
				continue;
			}

			vStrWndName = L"BTN_CHECK_OPTION";
			vStrWndName += iIndex;
			XUI::CXUI_CheckButton* pkWndButton = dynamic_cast<XUI::CXUI_CheckButton*>(pkWndTop->GetControl(vStrWndName.operator wchar_t const*()));
			if( !pkWndButton )
			{
				continue;
			}

			if( pkSelf == pkWndButton )
			{// 체크는 스크립트 실행 이후에 행해진다.
				bCheck = bCheck | (!pkWndButton->Check());
			}
			else
			{
				bCheck = bCheck | pkWndButton->Check();
			}
		}

		XUI::CXUI_Wnd* pkWndItem = pkWndTop->GetControl(L"FRM_ITEM");
		if( pkWndItem )
		{
			XUI::CXUI_Wnd* pkWndIcon = pkWndItem->GetControl(L"ICN_SRC");
			if( pkWndIcon )
			{
				if( !pkWndIcon->Visible() )
				{
					bCheck = false;
				}
			}
		}

		XUI::CXUI_Button* pkOkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWndTop->GetControl(_T("BTN_BUILD_UP")));
		if( pkOkBtn )
		{
			if( bCheck )
			{
				pkOkBtn->Disable(false);
			}
			else
			{
				pkOkBtn->Disable(true);
			}
		}
	}
	
	return true;
}

bool lwUIItemRarityBuildUp::Start()
{
	return g_kItemRarityBuildUpMgr.Start();
}

bool lwUIItemRarityBuildUp::Cancle()
{
	return g_kItemRarityBuildUpMgr.Cancle();
}

extern bool lwUIItemRarityBuildUp::Check()
{
	return g_kItemRarityBuildUpMgr.Check();
}
//////////////////////////////////////////////////////////////////////////
// PgItemRarityBuildUpMgr
//////////////////////////////////////////////////////////////////////////


PgItemRarityBuildUpMgr::PgItemRarityBuildUpMgr()
{
	Clear();
}

void PgItemRarityBuildUpMgr::Clear()
{
	m_eState =E_RS_READY;
	m_kNeedItemPos.Clear();
	m_kTargetItemPos.Clear();
	m_kTargetItem.Clear();
	m_kNeedtItem.Clear();
	std::queue<int> empty;
	std::swap(m_kContReserveIndex, empty);
	m_fStartTime = 0;
	m_fDelayStartTime = 0;
	m_fDelayTime = 0;
	m_iProcessIndex = -1;
	m_kContPosItemCount.clear();
	m_kContResultText.clear();

	ClearUI();
}

void PgItemRarityBuildUpMgr::ClearUI()
{
	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
	if( !pkWndTop )
	{
		return;
	}

	pkWndTop->GetCustomData(&m_fDelayTime, sizeof(m_fDelayTime));

	XUI::CXUI_Wnd* pkWndBuildUp =pkWndTop->GetControl(L"BTN_BUILD_UP");
	if( !pkWndBuildUp )
	{
		return;
	}
	pkWndBuildUp->Enable(false);
	pkWndBuildUp->Invalidate();

	for( int iIndex=0; iIndex<MAX_OPTION_COUNT; ++iIndex)
	{
		BM::vstring vStrWndName(L"SFRM_OPTION");
		vStrWndName += iIndex;
		XUI::CXUI_Wnd* pkWndOption = pkWndTop->GetControl(vStrWndName.operator wchar_t const*());
		if( !pkWndOption )
		{
			continue;
		}

		//연출 끄고 애니 스피드 초기화
		float fAniSpeed = BUILDUP_PROGRESS_SPEED;
		pkWndOption->SetCustomData(&fAniSpeed, sizeof(fAniSpeed));

		XUI::CXUI_Wnd* pkWndProcess = pkWndOption->GetControl(L"FRM_PROCESS");
		if( !pkWndProcess )
		{
			continue;
		}
		pkWndProcess->UVUpdate(1);
		pkWndProcess->Visible(false);

		//결과 하이라이트 ui 초기화
		XUI::CXUI_Wnd* pkWndText = pkWndOption->GetControl(L"OPTION_TEXT0");
		if( !pkWndText )
		{
			continue;
		}

		XUI::CXUI_Wnd* pkWndResult = pkWndOption->GetControl(L"IMG_RESULT");
		if( !pkWndResult )
		{
			continue;
		}
		POINT3I kPos = pkWndText->Location();
		pkWndResult->Location(kPos);
		pkWndResult->Visible(false);


		//체크박스 초기화
		vStrWndName = L"BTN_CHECK_OPTION";
		vStrWndName += iIndex;
		XUI::CXUI_CheckButton* pkWndButton = dynamic_cast<XUI::CXUI_CheckButton*>(pkWndTop->GetControl(vStrWndName.operator wchar_t const*()));
		if( !pkWndButton )
		{
			continue;
		}
		pkWndButton->Enable(true);
		pkWndButton->Check(false);

		//옵션 이름 초기화
		XUI::CXUI_Wnd* pkWndOptionName = pkWndOption->GetControl(L"OPTION_NAME");
		if( !pkWndOptionName )
		{
			continue;
		}
		pkWndOptionName->Visible(false);

		for( int iTextIndex=0; iTextIndex<4; ++iTextIndex)
		{//옵션 텍스트 초기화
			BM::vstring vStrWndName(L"OPTION_TEXT");
			vStrWndName << iTextIndex;
			XUI::CXUI_Wnd* pkWndOptionText = pkWndOption->GetControl(vStrWndName.operator wchar_t const*());
			if( !pkWndOptionText )
			{
				continue;
			}
			pkWndOptionText->Text(vStrWndName.operator wchar_t const*());
			pkWndOptionText->Visible(false);
		}
	}
}

void PgItemRarityBuildUpMgr::DisplaySrcIcon()
{
	if(m_kTargetItem.IsEmpty())
	{
		return;
	}
	if( !m_kTargetItem.ItemNo() )
	{
		return;
	}

	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
	if( !pkWndTop )
	{
		return;
	}

	XUI::CXUI_Wnd* pkWndItem = pkWndTop->GetControl(L"FRM_ITEM");
	if( !pkWndItem )
	{
		return;
	}
	pkWndItem->SetCustomData(&m_kTargetItem.ItemNo(), sizeof(m_kTargetItem.ItemNo()));
	POINT2 kPtLocation =	pkWndItem->TotalLocation();

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(m_kTargetItem.ItemNo());
	if(pItemDef)
	{
		g_kUIScene.RenderIcon( pItemDef->ResNo(), kPtLocation, false );
	}
}

void PgItemRarityBuildUpMgr::DisplayResultItem()
{

}

void PgItemRarityBuildUpMgr::OnTick( )
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	switch(m_eState)
	{
	case E_RS_READY:
		{
			int const iNeedItemCount  = GetBuildUpCount();

			int iHaveItemCount = 0;
			CONT_HAVE_ITEM_POS kContHaveItemPos;
			if( S_OK == pkInv->GetItems(IT_CASH, UICT_UPGRADE_OPTION, kContHaveItemPos ) )
			{
				iHaveItemCount = kContHaveItemPos.begin()->second.iCount;
				if( iNeedItemCount < iHaveItemCount )
				{
					iHaveItemCount = iNeedItemCount;
				}
			}

			BM::vstring vStrCount, vStrText(::TTW(1507));
			vStrCount = iHaveItemCount;
			vStrText.Replace(L"#Have#", vStrCount);

			vStrCount = iNeedItemCount;
			vStrText.Replace(L"#Need#", vStrCount);

			XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
			if( !pkWndTop )
			{
				return;
			}
			XUI::CXUI_Wnd* pkWndSoulBg = pkWndTop->GetControl(L"IMG_SOUL_BG");
			if( !pkWndSoulBg )
			{
				return;
			}
			XUI::CXUI_Wnd* pkWndText = pkWndSoulBg->GetControl(L"TEXT");
			if( !pkWndText )
			{
				return;
			}
			
			pkWndText->Text(vStrText.operator wchar_t const*());
		}break;
	case E_RS_WAIT_ANS:
		{//할거 하면 댐.
			return;
		}break;
	case E_RS_PROCESS:
		{
			if( BUILDUP_PROGRESS_TIME < g_pkApp->GetAccumTime() - m_fStartTime )
			{//연출 완료 되었음, 패킷 보내면 됨.
				Send_ReqBuildUp();
			}
		}break;
	case E_RS_CLEAR_DELAY:
		{
			if( m_fDelayTime < g_pkApp->GetAccumTime() - m_fDelayStartTime )
			{// x 초 지나면 자동으로 초기화

				PgBase_Item kItem;
				if( S_OK != pkInv->GetItem(m_kTargetItemPos, kItem) )
				{
					return;
				}

				m_kTargetItem = kItem;
				m_eState = E_RS_READY;
				XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
				if( !pkWndTop )
				{
					return;
				}

				XUI::CXUI_Wnd* pkWndBuildUp =pkWndTop->GetControl(L"BTN_BUILD_UP");
				if( !pkWndBuildUp )
				{
					return;
				}
				pkWndBuildUp->Enable(true);
				pkWndBuildUp->Invalidate();

				for( int iIndex=0; iIndex<MAX_OPTION_COUNT; ++iIndex)
				{
					if( !SetOptionText( iIndex, pkWndTop) )
					{
						continue;
					}
				}
			}
		}break;
	}
}

bool PgItemRarityBuildUpMgr::Check()
{
	size_t const iBuildUpOptionCount = GetBuildUpCount();
	if( !iBuildUpOptionCount )
	{// 선택한 옵션이 없음.
		::Notice_Show(::TTW(1517), EL_Warning);
		return false;
	}

	CONT_HAVE_ITEM_POS kContHaveItemPos;
	if( !CheckNeedItem(iBuildUpOptionCount, kContHaveItemPos) )
	{// 강화석 없음 또는 모자람
		return false;
	}

	return Check(m_kTargetItemPos);
}

bool PgItemRarityBuildUpMgr::Check(SItemPos const &rkItemPos)
{
	if( E_RS_READY != m_eState )
	{// 이미 진행중이다.
		return false;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return false;
	}

	PgBase_Item kItem;
	if( S_OK != pkInv->GetItem(rkItemPos, kItem) )
	{// 대상 아이템이 없는데??
		::Notice_Show(::TTW(1512), EL_Warning);
		return false;
	}

	E_ITEM_GRADE const eItemGrade = ::GetItemGrade(kItem);
	switch( eItemGrade )
	{
	case IG_SEAL:
	case IG_CURSE:
		{//봉인/저주는 사용 불가
			::Notice_Show(::TTW(1514), EL_Warning);
			return false;;
		}
	default: {}break;
	}

	if( kItem.EnchantInfo().IsBinding() )
	{//잠김 아이템은 사용 불가
		::Notice_Show(::TTW(1514), EL_Warning);
		return false;
	}

	switch(rkItemPos.x)
	{
	case KUIG_EQUIP:
	case KUIG_CASH:
		{
			if( LOCAL_MGR::NC_JAPAN == g_kLocal.ServiceRegion() 
				&& KUIG_CASH == rkItemPos.x )
			{//일본의 경우 캐시 아이템은 소울 크래프트 관련 작업 불가
				lwAddWarnDataTT(59002);
				return false;
			}
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( !pkItemDef )
			{
				return false;
			}

			if( false == FindCraftOption(kItem, pkItemDef) )
			{//증폭할 옵션이 존재하지 않음. 소울크래프트부터 하셈.
				::Notice_Show(::TTW(1515), EL_Warning);
				return false;
			}
			
			m_kTargetItemPos = rkItemPos;
			m_kTargetItem = kItem;
			return true;
		}break;
	case KUIG_FIT:
	case KUIG_CONSUME:
	case KUIG_ETC:
		{
			Notice_Show(TTW(59007), EL_Warning);
		}break;
	}
	return false;
}

bool PgItemRarityBuildUpMgr::Send_ReqBuildUp()
{
	//	PT_C_M_REQ_USE_UPGRADE_OPTION_ITEM
	//SItemPos : 옵션 강화석
	//SItemPos : 대상 아이템 ( 장착 아이템이여야 함 물론 옵션도 있어야함 )
	//Int : 강화할 옵션 인덱스 0 ~ 3 까지 총 4개

	//if( !Check(m_kTargetItemPos) )
	//{//연출하는 중간에 뭔가 바뀔수도 있으니 패킷 보내기전 다시 한번 확인
	//	return false;
	//}
	if( E_RS_PROCESS != m_eState )
	{
		return false;
	}

	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
	if( !pkWndTop )
	{
		return false;
	}

	int iSendCount = 0;
	for( int iIndex=0; iIndex<MAX_OPTION_COUNT; ++iIndex)
	{
		BM::vstring vStrWndName(L"SFRM_OPTION");
		vStrWndName += iIndex;

		XUI::CXUI_Wnd* pkWndOption = pkWndTop->GetControl(vStrWndName.operator wchar_t const*());
		if( !pkWndOption )
		{
			continue;
		}

		vStrWndName = L"BTN_CHECK_OPTION";
		vStrWndName += iIndex;
		XUI::CXUI_CheckButton* pkWndButton = dynamic_cast<XUI::CXUI_CheckButton*>(pkWndTop->GetControl(vStrWndName.operator wchar_t const*()));
		if( !pkWndButton )
		{
			continue;
		}
		if( pkWndButton->Check() )
		{//연출 끄고 인덱스를 큐에 넣는다.
			float fAniSpeed = BUILDUP_PROGRESS_SPEED;
			pkWndOption->SetCustomData(&fAniSpeed, sizeof(fAniSpeed));

			XUI::CXUI_Wnd* pkWndProcess = pkWndOption->GetControl(L"FRM_PROCESS");
			if( !pkWndProcess )
			{
				continue;
			}
			pkWndProcess->UVUpdate(1);
			pkWndProcess->Visible(false);

			m_kContReserveIndex.push(iIndex);
			++iSendCount;
		}
	}

	if( iSendCount )
	{
		Send();
		m_eState = E_RS_WAIT_ANS;
		return true;
	}

	return false;
}

bool PgItemRarityBuildUpMgr::Send()
{
	m_iProcessIndex = -1;
	while( false == m_kContReserveIndex.empty() )
	{
		m_iProcessIndex = m_kContReserveIndex.front();
		m_kContReserveIndex.pop();
		if( 0 > m_iProcessIndex )
		{//잘못된 인덱스임
			continue;	
		}
		else
		{
			break;
		}
	}
	
	if( 0 > m_iProcessIndex )
	{
		return false;
	}

	m_kNeedItemPos = GetNeedItemPos();
	if(SItemPos::NullData() == m_kNeedItemPos)
	{
		return false;
	}

	BM::Stream kPacket(PT_C_M_REQ_USE_UPGRADE_OPTION_ITEM);
	kPacket.Push(m_kNeedItemPos);
	kPacket.Push(m_kTargetItemPos);
	kPacket.Push(m_iProcessIndex);
	NETWORK_SEND(kPacket);
	return true;
}
void PgItemRarityBuildUpMgr::Recv( BM::Stream &rkPacket )
{
	HRESULT kResult = EC_OK;
	if( false == rkPacket.Pop(kResult) )
	{
		return;
	}
	
	if( 0 > m_iProcessIndex )
	{
		return;
	}

	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
	if( !pkWndTop )
	{
		return;
	}

	BM::vstring vStrWndName(L"SFRM_OPTION");
	vStrWndName += m_iProcessIndex;
	XUI::CXUI_Wnd* pkWndOption = pkWndTop->GetControl(vStrWndName.operator wchar_t const*());
	if( !pkWndOption )
	{
		return;
	}

	switch( kResult )
	{
	case EC_OK:
		{
			int iOptionIndex = 0;
			int iNewOptLevel = 0;
			rkPacket.Pop(iOptionIndex);
			rkPacket.Pop(iNewOptLevel);

			int iType = 0, iCurOptLevel = 0, iAmpLevel = 0;
			GetOptionInfo(m_kTargetItem, iOptionIndex, iType, iCurOptLevel, iAmpLevel);

			int const iSelectedIndex = iNewOptLevel - iCurOptLevel;
			if( 0 < iSelectedIndex )
			{
				std::wstring wstrEnchantText;
				if( GetChangedOptionText(iOptionIndex, iNewOptLevel, wstrEnchantText) )
				{
					BM::vstring vStrText(wstrEnchantText);
					vStrText.Replace(L"\n", L" ");
					vStrText += ::TTW(1234);// 상승하였습니다.	
					m_kContResultText.insert( std::make_pair(iOptionIndex+1, vStrText) );//일단 컨테이너에 넣는다.
				}
			}

			XUI::CXUI_Wnd* pkWndResult = pkWndOption->GetControl(L"IMG_RESULT");
			if( !pkWndResult )
			{
				break;
			}

			int iOriginPosX = 0;
			pkWndResult->GetCustomData(&iOriginPosX, sizeof(iOriginPosX));
			int const iPosX = iOriginPosX+(iSelectedIndex*64);
			pkWndResult->Location(iPosX, 3);
			pkWndResult->Visible(true);
			pkWndResult->Invalidate(true);
		}break;
	case E_UPGRADE_OPTION_NOT_FOUND_ITEM:		{ ::Notice_Show(::TTW(1512), EL_Warning); }break; // 아이템옵션강화( 대상아이템없음)
	case E_UPGRADE_OPTION_NOT_EQUIP_ITEM:		{ ::Notice_Show(::TTW(1513), EL_Warning); }break; // 아이템옵션강화( 장착아이템이아님)
	case E_UPGRADE_OPTION_CANT_UPGRADE_ITEM:	{ ::Notice_Show(::TTW(1514), EL_Warning); }break; // 아이템옵션강화( 업그레이드할수없는녀석임)
	case E_UPGRADE_OPTION_NOT_HAVE_OPTION:	{ ::Notice_Show(::TTW(1515), EL_Warning); }break; // 아이템옵션강화( 옵션없음)
	default:
		{//선언되지 않은 에러
			::Notice_Show(::TTW(790413), EL_Warning);
		}break;
	}

	char szName[100] = "EnchantFail";
	if( EC_OK != kResult )
	{//실패했다면 더이상 진행할 필요가 없음.
		Clear();
		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
		return;
	}

	if( false == Send() )
	{// 더이상 보낼게 없다.
		m_eState = E_RS_CLEAR_DELAY;
		m_fDelayStartTime = g_pkApp->GetAccumTime();

		if( m_kContResultText.empty() )
		{//강화실패했습니다
			::Notice_Show(::TTW(1545), EL_Warning);
			g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
			return;
		}

		sprintf(szName, "EnchantSuccess");
		//::Notice_Show(::TTW(1518), EL_Normal);//강화성공했습니다
		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());

		std::map<int, BM::vstring>::iterator itor = m_kContResultText.begin();
		while( m_kContResultText.end() != itor )
		{//성공한 옵션을 모두 출력
			BM::vstring vStrText(::TTW(1553));//{C=0xFF00FF00/}#INDEX# 번째{C=0xFFFFFFFF/} 옵션을 강화하는데 성공하였습니다."/>
			BM::vstring vStrIndex(itor->first);
			vStrText.Replace(L"#INDEX#", vStrIndex);
			::Notice_Show( vStrText, EL_Normal );

			vStrText = itor->second;
			::Notice_Show( vStrText, EL_Normal );
			++itor;
		}
	}
}

SItemPos PgItemRarityBuildUpMgr::GetNeedItemPos()
{
	for(CONT_POST_ITEMCOUNT::iterator iter = m_kContPosItemCount.begin();iter != m_kContPosItemCount.end();++iter)
	{
		if(0 >= iter->second)
		{
			continue;
		}
		--iter->second;
		return (*iter).first;
	}
	return SItemPos::NullData();
}

bool PgItemRarityBuildUpMgr::CheckNeedItem( size_t const iBuildUpOptionCount, CONT_HAVE_ITEM_POS& rkContHaveItemPos )
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return false;
	}

	if( S_OK != pkInv->GetItems(IT_CASH, UICT_UPGRADE_OPTION, rkContHaveItemPos ) )
	{// 강화석이 없네여
		::Notice_Show(::TTW(1516), EL_Warning);
		return false;
	}

	m_kContPosItemCount.clear();

	int iCount = 0;
	for(CONT_HAVE_ITEM_POS::const_iterator itor = rkContHaveItemPos.begin();itor != rkContHaveItemPos.end();++itor)
	{
		iCount += itor->second.iCount;

		CONT_HAVE_ITEM_POS::mapped_type const & kCont = itor->second;

		for(std::vector<SItemPos>::const_iterator pos_iter = kCont.kContItemPos.begin();pos_iter != kCont.kContItemPos.end();++pos_iter)
		{
			PgBase_Item kItem;
			if(S_OK != pkInv->GetItem((*pos_iter),kItem))
			{
				return false;
			}
			m_kContPosItemCount[(*pos_iter)] = kItem.Count();
		}
	}

	if( iBuildUpOptionCount > iCount )
	{// 갯수가 모잘라여~~
		::Notice_Show(::TTW(1501), EL_Warning);
		return false;
	}
	return true;
}

void PgItemRarityBuildUpMgr::SetItem( EKindUIIconGroup const kType, SIconInfo const & rkInfo )
{
	if( E_RS_READY != m_eState )
	{//연출중/서버응답대기중일때는 할수 없음.
		return;
	}

	if( 0 == rkInfo.iIconGroup || 0 > rkInfo.iIconKey)
	{
		return;
	}

	if( KUIG_ITEM_RARITY_BUILDUP_SRC != kType )
	{
		return;
	}

	if( Check(SItemPos(rkInfo.iIconGroup, rkInfo.iIconKey)) )
	{
		ClearUI();
		XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
		if( !pkWndTop )
		{
			return;
		}

		XUI::CXUI_Wnd* pkWndItem = pkWndTop->GetControl(L"FRM_ITEM");
		if( !pkWndItem )
		{
			return;
		}
		
		XUI::CXUI_Wnd* pkWndIcon = pkWndItem->GetControl(L"ICN_SRC");
		if( !pkWndIcon )
		{
			return;
		}

		//아이콘 보여주고..
		pkWndIcon->Visible(true);

		//옵션 하나씩 텍스트로..
		for( int iIndex=0; iIndex<MAX_OPTION_COUNT; ++iIndex)
		{
			SetOptionText( iIndex, pkWndTop);
		}
	}
}

bool PgItemRarityBuildUpMgr::Start()
{
	if( false == Check() )
	{
		return false;
	}

	// 연출 시작
	m_kContResultText.clear();

	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
	if( !pkWndTop )
	{
		return false;
	}
	pkWndTop->GetCustomData(&m_fDelayTime, sizeof(m_fDelayTime));

	XUI::CXUI_Wnd* pkWndBuildUp =pkWndTop->GetControl(L"BTN_BUILD_UP");
	if( !pkWndBuildUp )
	{
		return false;
	}
	pkWndBuildUp->Enable(false);

	XUI::CXUI_Wnd* pkWndItem = pkWndTop->GetControl(L"FRM_ITEM");
	if( !pkWndItem )
	{
		return false;
	}

	for( int iIndex=0; iIndex<MAX_OPTION_COUNT; ++iIndex)
	{
		BM::vstring vStrWndName(L"SFRM_OPTION");
		vStrWndName += iIndex;
		XUI::CXUI_Wnd* pkWndOption = pkWndTop->GetControl(vStrWndName.operator wchar_t const*());
		if( !pkWndOption )
		{
			continue;
		}

		vStrWndName = L"BTN_CHECK_OPTION";
		vStrWndName += iIndex;
		XUI::CXUI_CheckButton* pkWndButton = dynamic_cast<XUI::CXUI_CheckButton*>(pkWndTop->GetControl(vStrWndName.operator wchar_t const*()));
		if( !pkWndButton )
		{
			continue;
		}

		if( pkWndButton->Check() )
		{
			pkWndButton->SetCustomData(&BUILDUP_PROGRESS_SPEED, sizeof(BUILDUP_PROGRESS_SPEED));
			XUI::CXUI_Wnd* pkWndProcess = pkWndOption->GetControl(L"FRM_PROCESS");
			if( !pkWndProcess )
			{
				continue;
			}
			pkWndProcess->Visible(true);
			pkWndProcess->UVUpdate(1);
			int const iState = 1;
			pkWndProcess->SetCustomData( &iState, sizeof(iState) );

			XUI::CXUI_Wnd* pkWndDelay = pkWndProcess->GetControl(L"FRM_DELAY");
			if( !pkWndDelay )
			{
				continue;
			}
			float const fDelay = 0;
			pkWndDelay->SetCustomData( &fDelay, sizeof(fDelay) );
		}
		pkWndButton->Enable(false);
		pkWndButton->Invalidate();

		XUI::CXUI_Wnd* pkWndText = pkWndOption->GetControl(L"OPTION_TEXT0");
		if( !pkWndText )
		{
			continue;
		}

		XUI::CXUI_Wnd* pkWndResult = pkWndOption->GetControl(L"IMG_RESULT");
		if( !pkWndResult )
		{
			continue;
		}
		POINT3I kPos = pkWndText->Location();
		pkWndResult->Location(kPos);
		pkWndResult->Visible(false);
	}

	m_fStartTime = g_pkApp->GetAccumTime();
	m_eState = E_RS_PROCESS;
	return true;
}

size_t PgItemRarityBuildUpMgr::GetBuildUpCount()
{
	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("SFRM_ITEM_RARITY_BUILDUP"));
	if( !pkWndTop )
	{
		return 0;
	}
	
	size_t iSelectedCount = 0;
	for( int iIndex=0; iIndex<MAX_OPTION_COUNT; ++iIndex)
	{
		BM::vstring vStrWndName(L"SFRM_OPTION");
		vStrWndName += iIndex;
		XUI::CXUI_Wnd* pkWndOption = pkWndTop->GetControl(vStrWndName.operator wchar_t const*());
		if( !pkWndOption )
		{
			continue;
		}

		vStrWndName = L"BTN_CHECK_OPTION";
		vStrWndName += iIndex;
		XUI::CXUI_CheckButton* pkWndButton = dynamic_cast<XUI::CXUI_CheckButton*>(pkWndTop->GetControl(vStrWndName.operator wchar_t const*()));
		if( pkWndButton )
		{
			iSelectedCount += pkWndButton->Check();
		}
	}

	return iSelectedCount;
}

void PgItemRarityBuildUpMgr::GetOptionInfo(PgBase_Item& rkItem, int const iIndex, int& iOutType, int& iOutLevel, int& iOutAmpLevel)
{
	SEnchantInfo const& kEnchantInfo = rkItem.EnchantInfo();

	switch( iIndex )
	{
	case 0:
		{
			iOutType = static_cast<int>(kEnchantInfo.BasicType1());
			iOutLevel = static_cast<int>(kEnchantInfo.BasicLv1());
		}break;
	case 1:
		{
			iOutType = static_cast<int>(kEnchantInfo.BasicType2());
			iOutLevel = static_cast<int>(kEnchantInfo.BasicLv2());
		}break;
	case 2:
		{
			iOutType = static_cast<int>(kEnchantInfo.BasicType3());
			iOutLevel = static_cast<int>(kEnchantInfo.BasicLv3());
		}break;
	case 3:
		{
			iOutType = static_cast<int>(kEnchantInfo.BasicType4());
			iOutLevel = static_cast<int>(kEnchantInfo.BasicLv4());
		}break;
	}

	iOutAmpLevel = static_cast<int>(kEnchantInfo.BasicAmpLv());
}

bool PgItemRarityBuildUpMgr::SetOptionText(int const iOptIndex, XUI::CXUI_Wnd* pkWnd)
{
	//옵션도 보여줘야함.
	BM::vstring vStrWndName(L"SFRM_OPTION");
	vStrWndName += iOptIndex;
	XUI::CXUI_Wnd* pkWndOption = pkWnd->GetControl(vStrWndName.operator wchar_t const*());
	if( !pkWndOption )
	{
		return false;
	}

	vStrWndName = L"BTN_CHECK_OPTION";
	vStrWndName += iOptIndex;
	XUI::CXUI_CheckButton* pkWndButton = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(vStrWndName.operator wchar_t const*()));
	if( !pkWndButton )
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(m_kTargetItem.ItemNo());
	if( !pkItemDef )
	{
		return false;
	}

	int iType = 0, iCurOptLevel = 0, iAmpLevel = 0;
	GetOptionInfo(m_kTargetItem, iOptIndex, iType, iCurOptLevel, iAmpLevel);

	int const iEquipPos = pkItemDef->EquipPos();
	int const iLevelLimit = pkItemDef->GetAbil(AT_LEVELLIMIT);
	SBasicOptionAmpKey const kAmpKey(GetEquipType(pkItemDef), iLevelLimit, iAmpLevel);
	CONT_ENCHANT_ABIL kEnchantAbil;
	::GetAbilObject(iType, iEquipPos, iCurOptLevel, 0, iOptIndex, kEnchantAbil, kAmpKey);
	if( kEnchantAbil.empty() )
	{
		pkWndButton->Enable(false);
		pkWndButton->Check(false);
		return false;
	}
	std::wstring wstrText = ::MakeSlotAbilToolTipText(m_kTargetItem, pkItemDef, kEnchantAbil);

	int const iMaxOptLevel = std::max<int>(pkItemDef->GetAbil(AT_LEVELLIMIT)/5 + 6, MAX_DICE_ITEM_OPTION_LEVEL);
	bool bShowText = true;
	if( iMaxOptLevel <= iCurOptLevel )
	{
		bShowText = false;
	}
	pkWndButton->Enable(bShowText);
	pkWndButton->Check(false);

	XUI::CXUI_Wnd* pkWndResult = pkWndOption->GetControl(L"IMG_RESULT");
	if( !pkWndResult )
	{
		return false;
	}
	pkWndResult->Visible(false);

	XUI::CXUI_Wnd* pkWndOptionName = pkWndOption->GetControl(L"OPTION_NAME");
	if( !pkWndOptionName )
	{
		return false;
	}
	pkWndOptionName->Text(wstrText);
	pkWndOptionName->Visible(true);

	int iShowIndex = 0;
	for( int iIndex=0, iTargetOptLevel=iCurOptLevel; iIndex<MAX_OPTION_COUNT; ++iIndex, ++iTargetOptLevel)
	{
		kEnchantAbil.clear();
		::GetAbilObject(iType, iEquipPos, iTargetOptLevel, 0, iIndex, kEnchantAbil, kAmpKey);

		BM::vstring vStrWndName(L"OPTION_TEXT");
		vStrWndName << iIndex;
		XUI::CXUI_Wnd* pkWndOptionText = pkWndOption->GetControl(vStrWndName.operator wchar_t const*());
		if( !pkWndOptionText )
		{
			return false;
		}

		bool bShowText = true;
		if( 0 == iIndex && iMaxOptLevel <= iTargetOptLevel)
		{
			bShowText = false;
		}

		BM::vstring vStrOptionText;
		if( iMaxOptLevel >= iTargetOptLevel )
		{
			vStrOptionText = ::MakeSlotAbilToolTipText(m_kTargetItem, pkItemDef, kEnchantAbil, false, false);
			++iShowIndex;
		}

		wstrText.clear();
		if( false == GetChangedOptionText(iOptIndex, iTargetOptLevel, wstrText) )
		{//값이 작거나 같다면 하이트라이트 컬러로 표시한다.
			pkWndOptionText->FontColor(HIGHLIGHT_COLOR);
		}
		else
		{
			pkWndOptionText->FontColor(0xFFFFFFFF);
		}

		pkWndOptionText->Text(vStrOptionText.operator wchar_t const*());
		pkWndOptionText->Visible(bShowText);
	}

	XUI::CXUI_Wnd* pkWndProcess = pkWndOption->GetControl(L"FRM_PROCESS");
	if( !pkWndProcess )
	{
		return false;
	}
	XUI::CXUI_Wnd* pkWndMaxIndex = pkWndProcess->GetControl(L"FRM_MAX_INDEX");
	if( !pkWndMaxIndex )
	{
		return false;
	}
	pkWndMaxIndex->SetCustomData( &iShowIndex, sizeof(iShowIndex) );

	return true;
}

bool PgItemRarityBuildUpMgr::GetChangedOptionText(int const iIndex, int const iNewLevel, std::wstring& rkOutText)
{
	int iType = 0, iCurLevel = 0, iAmpLevel = 0;
	GetOptionInfo(m_kTargetItem, iIndex, iType, iCurLevel, iAmpLevel);

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(m_kTargetItem.ItemNo());
	if( !pkItemDef )
	{
		return false;
	}
	int const iEquipPos = pkItemDef->EquipPos();
	int const iLevelLimit = pkItemDef->GetAbil(AT_LEVELLIMIT);
	SBasicOptionAmpKey const kAmpKey(GetEquipType(pkItemDef), iLevelLimit, iAmpLevel);

	CONT_ENCHANT_ABIL kPrevEnchantAbil, kNewEnchantAbil;
	::GetAbilObject(iType, iEquipPos, iNewLevel, 0, iIndex, kNewEnchantAbil, kAmpKey);
	::GetAbilObject(iType, iEquipPos, iCurLevel, 0, iIndex, kPrevEnchantAbil, kAmpKey);

	if( kNewEnchantAbil.empty() || kPrevEnchantAbil.empty() ) 
	{
		return false;
	}

	if( kNewEnchantAbil[0].iValue <= kPrevEnchantAbil[0].iValue )
	{
		return false;
	}

	kNewEnchantAbil[0].iValue -= kPrevEnchantAbil[0].iValue;
	rkOutText = ::MakeSlotAbilToolTipText(m_kTargetItem, pkItemDef, kNewEnchantAbil, true);
	return true;
}

bool PgItemRarityBuildUpMgr::Cancle()
{
	bool bRet = (m_eState == E_RS_READY ? true : false);
	if( E_RS_PROCESS == m_eState )
	{//연출중에만 취소가 가능함.
		::Notice_Show(::TTW(1221), EL_Warning);
		Clear();
	}

	return bRet;
}

bool PgItemRarityBuildUpMgr::FindCraftOption(PgBase_Item const& rkItem, CItemDef const* pkItemDef)
{
	SEnchantInfo const kEnchantInfo = rkItem.EnchantInfo();
	int const iEquipPos = pkItemDef->EquipPos();
	int const iLevelLimit = pkItemDef->GetAbil(AT_LEVELLIMIT);
	SBasicOptionAmpKey const kAmpKey(GetEquipType(pkItemDef), iLevelLimit, kEnchantInfo.BasicAmpLv());

	CONT_ENCHANT_ABIL kEnchantAbil;
	::GetAbilObject(kEnchantInfo.BasicType1(), iEquipPos, kEnchantInfo.BasicLv1(), 0, 0, kEnchantAbil, kAmpKey);
	::GetAbilObject(kEnchantInfo.BasicType2(), iEquipPos, kEnchantInfo.BasicLv2(), 0, 1, kEnchantAbil, kAmpKey);
	::GetAbilObject(kEnchantInfo.BasicType3(), iEquipPos, kEnchantInfo.BasicLv3(), 0, 2, kEnchantAbil, kAmpKey);
	::GetAbilObject(kEnchantInfo.BasicType4(), iEquipPos, kEnchantInfo.BasicLv4(), 0, 3, kEnchantAbil, kAmpKey);

	if( kEnchantAbil.empty() )
	{
		return false;
	}
	return true;
}
