#include "StdAfx.h"
#include "lohengrin/packettype.h"
#include "Variant/item.h"
#include "Variant/PgPlayer.h"
#include "Variant/PgEmporia.h"
#include "Variant/PgSocketFormula.h"
#include "Variant/PgItemRarityUpgradeFormula.h"
#include "lwUI.h"
#include "lwUIItemConvertSystem.h"
#include "lwUIToolTip.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "PgNifMan.H"
#include "PgUIModel.h"
#include "PgChatMgrClient.h"
#include "PgSoundMan.h"
#include "lwUIQuest.h"

namespace PgItemConvertSystemUtil
{
	std::wstring const kItemConvertSystemUIName(_T("FRM_ITEM_CONVERT"));

	lwUIWnd CallItemConvertSystemUI(lwGUID kNpcGuid)
	{
		g_kItemConvertSystemMgr.NpcGuid( kNpcGuid() );
		CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kItemConvertSystemUIName );
		if( pkTopWnd )
		{
			RegistUIAction(pkTopWnd);
			return lwUIWnd(pkTopWnd);
		}
		return lwUIWnd(NULL);
	}
};

lwUIItemConvertSystem::lwUIItemConvertSystem(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}

bool lwUIItemConvertSystem::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "CallItemConvertSystemUI", PgItemConvertSystemUtil::CallItemConvertSystemUI);

	class_<lwUIItemConvertSystem>(pkState, "ItemConvertSystemWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "InProgress", &lwUIItemConvertSystem::InProgress)
		.def(pkState, "SetInProgress", &lwUIItemConvertSystem::SetInProgress)		
		.def(pkState, "DisplaySrcIcon", &lwUIItemConvertSystem::DisplaySrcIcon)
		.def(pkState, "Clear", &lwUIItemConvertSystem::Clear)
		.def(pkState, "SetMaterialItem", &lwUIItemConvertSystem::SetMaterialItem)		
		.def(pkState, "OnItemConvertCallInit", &lwUIItemConvertSystem::OnItemConvertCallInit)
		.def(pkState, "OnItemConvertInit", &lwUIItemConvertSystem::OnItemConvertInit)
		.def(pkState, "SendItemConvert", &lwUIItemConvertSystem::SendItemConvert)
		.def(pkState, "PreCheckItemConvert", &lwUIItemConvertSystem::PreCheckItemConvert)		
		.def(pkState, "ItemConvertMaxCount", &lwUIItemConvertSystem::ItemConvertMaxCount)
	;


	return true;
}

void lwUIItemConvertSystem::Clear(bool const bClearAll)
{
	g_kItemConvertSystemMgr.Clear(bClearAll);
}

void lwUIItemConvertSystem::DisplaySrcIcon()
{
	g_kItemConvertSystemMgr.DisplaySrcItem(self);
}

bool lwUIItemConvertSystem::SetMaterialItem()
{
	return g_kItemConvertSystemMgr.SetMaterialItem(self->Parent());
}

bool lwUIItemConvertSystem::InProgress()
{
	return g_kItemConvertSystemMgr.InProgress();
}

void lwUIItemConvertSystem::SetInProgress()
{
	g_kItemConvertSystemMgr.InProgress(true);
}

void lwUIItemConvertSystem::OnItemConvertCallInit()
{
	g_kItemConvertSystemMgr.OnItemConvertInit(self, 0, true);
}

void lwUIItemConvertSystem::OnItemConvertInit(int iCount)
{
	g_kItemConvertSystemMgr.OnItemConvertInit(self->Parent(), iCount, false);
}

void lwUIItemConvertSystem::SendItemConvert()
{
	g_kItemConvertSystemMgr.SendItemConvert();
}

bool lwUIItemConvertSystem::PreCheckItemConvert(int const iCount)
{
	return g_kItemConvertSystemMgr.PreCheckItemConvert(iCount);
}

int lwUIItemConvertSystem::ItemConvertMaxCount()
{
	return g_kItemConvertSystemMgr.ItemConvertMaxCount();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PgItemConvertSystemMgr
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgItemConvertSystemMgr::PgItemConvertSystemMgr()
{
	Clear(true);
}

void PgItemConvertSystemMgr::Clear(bool const bAllClear)
{
	m_kSrcItemPos.Clear();
	m_kItem.Clear();
	InProgress(false);

	iFirstItemNo = 0;
	iSecondItemNo = 0;

	m_iItemTotalSourceCount = 0;
	m_iItemFirstCount = 0;
	m_iItemSecondCount = 0;
	m_iItemCount = 0;

	if( bAllClear )
	{
		m_kNpcGuid.Clear();
	}
}

bool PgItemConvertSystemMgr::SetItem(SIconInfo const & rkInfo)
{
	if (0!=rkInfo.iIconGroup && 0<=rkInfo.iIconKey)
	{
		if( SetSrcItem(SItemPos(rkInfo.iIconGroup, rkInfo.iIconKey)) )
		{
			return true;
		}
	}
	return false;
}

bool PgItemConvertSystemMgr::SetMaterialItem(XUI::CXUI_Wnd* pWnd)
{
	if( !pWnd ){ return false; }
	if( m_kItem.IsEmpty() ){ return false; }

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){ return false; }
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv ){ return false; }

	SItemPos	rkPos;

	DWORD dwItemNo = 0;

	// ItemNo 뽑아 내기


	if( S_OK != pkInv->GetFirstItem(dwItemNo, rkPos) )
	{
		return false;
	}

	return SetItem(SIconInfo(rkPos.x, rkPos.y));	
}

int PgItemConvertSystemMgr::GetItemNo()
{
	return m_kItem.ItemNo();
}

void PgItemConvertSystemMgr::UpdateUI_Init(XUI::CXUI_Wnd* pWnd, int const iFirstItemNo, int const iSecondItemNo)
{
	CXUI_Wnd* pkTopWnd = pWnd;
	if( !pkTopWnd )
	{
		return;
	}
	CXUI_Wnd* pkUIWnd = pkTopWnd->GetControl( std::wstring(_T("FRM_UI")) );
	if( pkUIWnd )
	{
		CXUI_Wnd* pkFirstNametWnd = pkUIWnd->GetControl( std::wstring(_T("FRM_SOUL_NAME")) );
		CXUI_Wnd* pkSecondNameWnd = pkUIWnd->GetControl( std::wstring(_T("FRM_SCROLL_NAME")) );

		TCHAR const* kFirstName = _T("");
		TCHAR const* kSecondName = _T("");

		if( !GetItemName(iFirstItemNo, kFirstName) )
		{
			// 아이템 이름 못얻음
			kFirstName = _T("");
		}
		if( !GetItemName(iSecondItemNo, kSecondName) )
		{
			// 아이템 이름 못얻음
			kSecondName = _T("");
		}

		if( pkFirstNametWnd )	{ pkFirstNametWnd->Text( std::wstring( kFirstName ) ); }
		if( pkSecondNameWnd )	{ pkSecondNameWnd->Text( std::wstring( kSecondName ) ); }


		CXUI_Wnd* pkImg = pkTopWnd->GetControl( std::wstring(_T("ICON_Item2")) );
		if( pkImg )
		{
			pkImg->SetCustomData(&iSecondItemNo, sizeof(iSecondItemNo));
		}
	}
}

void PgItemConvertSystemMgr::OnItemConvertInit(XUI::CXUI_Wnd* pWnd, int iCount, bool bInit)
{
	CXUI_Wnd* pkTopWnd = pWnd;
	if( !pkTopWnd )
	{
		return;
	}

	m_iItemTotalSourceCount = 0;
	m_iItemFirstCount = 0;
	m_iItemSecondCount = 0;
	m_iItemCount = 0;

	iFirstItemNo = 0;
	iSecondItemNo = 0;

	int iErrorMsg = 0;
	// 올려진 아이템에 따른 변환될 아이템을 체크한다.
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return;}

	if( 0 != iCount )
	{
		if( 0 == m_kItem.ItemNo() )
		{
			if( SItemPos::NullData() == m_kSrcItemPos )
			{
				iErrorMsg = 790401;
				return;
			}

			if(S_OK != pInv->GetItem(m_kSrcItemPos, m_kItem))
			{
				iErrorMsg = 790401;			
				return;
			}
		}
	}

	if( !m_kItem.IsEmpty() )
	{
		CONT_DEF_CONVERTITEM const * pkDef = NULL;
		g_kTblDataMgr.GetContDef(pkDef);
		CONT_DEF_CONVERTITEM::const_iterator iter = pkDef->find(m_kItem.ItemNo());
		if( pkDef->end() == iter )
		{
			// 교환할수 있는 아이템 목록이 없다.
			lwAddWarnDataTT(790531);
			iCount = 0;
		}
		else
		{
			m_iItemTotalSourceCount = pInv->GetTotalCount(m_kItem.ItemNo());
			m_iItemFirstCount = (*iter).second.sSourceItemNum * iCount;

			if( m_iItemFirstCount > m_iItemTotalSourceCount )
			{
				lwAddWarnDataTT(790535);
				return;
			}

			m_iItemSecondCount = (*iter).second.sTargetItemNum * iCount;
			m_iItemCount = iCount;

			iFirstItemNo = (*iter).second.iSourceItemNo;
			iSecondItemNo = (*iter).second.iTargetItemNo;
		}
	}

	UpdateUI_Init(pWnd, iFirstItemNo, iSecondItemNo );
	CXUI_Wnd* pkSoulCountWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_COUL_COUNT")) );
	CXUI_Wnd* pkScrollCountWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_SCROLL_COUNT")) );
	CXUI_Wnd* pkIncBtn = pkTopWnd->GetControl( std::wstring(_T("BTN_INC_COUNT")) );
	CXUI_Wnd* pkDecBtn = pkTopWnd->GetControl( std::wstring(_T("BTN_DEC_COUNT")) );
	CXUI_Wnd* pkOnBtn = pkTopWnd->GetControl( std::wstring(_T("BTN_OK")) );

	if( pkSoulCountWnd )	{ pkSoulCountWnd->Text( BM::vstring(m_iItemFirstCount) ); }
	if( pkScrollCountWnd )	{ pkScrollCountWnd->Text( BM::vstring(m_iItemSecondCount) ); }
	if( pkIncBtn )			{ pkIncBtn->SetCustomData(&iCount, sizeof(iCount)); }
	if( pkDecBtn )			{ pkDecBtn->SetCustomData(&iCount, sizeof(iCount)); }
	if( pkOnBtn )			{ pkOnBtn->SetCustomData(&iCount, sizeof(iCount)); }

	if( 0 != iErrorMsg && false == bInit )
	{
		lwAddWarnDataTT(iErrorMsg);
	}
}

void PgItemConvertSystemMgr::DisplaySrcItem(XUI::CXUI_Wnd *pWnd)
{//네임 필드를 찾아서 이름 셋팅. 아이템이 없거나 하면 올리세요 라는 메세지로 셋팅.
	if (!pWnd) { return; }
	int const iNull = 0;

	std::wstring wstrName;
	POINT2 rPT;
	if(m_kItem.IsEmpty()){goto __HIDE;}
	if(!m_kItem.ItemNo()){goto __HIDE;}

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
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		m_kSrcItemPos.Clear();
	}
	return;
}

bool PgItemConvertSystemMgr::SetSrcItem(const SItemPos &rkItemPos)
{
	if(InProgress())
	{
		lwAddWarnDataTT(790537);
		return false;
	}

	PgBase_Item	rkItem;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	CONT_DEF_CONVERTITEM const * pkDef = NULL;
	g_kTblDataMgr.GetContDef(pkDef);

	int iErrorNo = 790401;

	CONT_DEF_CONVERTITEM::const_iterator iter;
	if( SItemPos::NullData() == rkItemPos )
	{
		Clear();
		lwAddWarnDataTT(iErrorNo);
		return false;
	}
	
	if(S_OK != pInv->GetItem(rkItemPos, rkItem))
	{
		if( KUIG_ITEMCONVERT_SYSTEM != rkItemPos.x )
		{
			Clear();
			lwAddWarnDataTT(iErrorNo);
		}
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(rkItem.ItemNo());
	if( !pItemDef )
	{
		Clear();
		lwAddWarnDataTT(iErrorNo);
		return false;
	}

	iter = pkDef->find(rkItem.ItemNo());
	if( pkDef->end() == iter )
	{
		iErrorNo = 790531;
		lwAddWarnDataTT(iErrorNo);
		return false;
	}

	int const m_iItemTotalSourceCount = pInv->GetTotalCount(rkItem.ItemNo());
	int const m_iItemFirstCount = (*iter).second.sSourceItemNum * 1;

	if( m_iItemFirstCount > m_iItemTotalSourceCount )
	{
		lwAddWarnDataTT(790536);
		return false;
	}

	if (rkItemPos.x && rkItemPos.y)
	{
		Clear();//클리어 밟아야함.
	}
	m_kItem = rkItem;
	m_kSrcItemPos = rkItemPos;

	return true;
}

void PgItemConvertSystemMgr::SendItemConvert()
{
	if( m_kItem.IsEmpty() )
	{
		lwAddWarnDataTT(790401);
		InProgress(false);
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		InProgress(false);
		return;
	}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv)
	{
		InProgress(false);
		return;
	}

	int const m_iItemTotalSourceCount = pInv->GetTotalCount(m_kItem.ItemNo());

	if( GetItemFirstCount() > m_iItemTotalSourceCount )
	{
		lwAddWarnDataTT(790536);
		InProgress(false);
		return;
	}
		
	if( BM::GUID::IsNull(NpcGuid()) )
	{
		InProgress(false);
		return;
	}

	int const iItemNo = m_kItem.ItemNo();

	BM::Stream kPacket(PT_C_M_REQ_CONVERTITEM);
	kPacket.Push(NpcGuid()); // Npc Guid
	kPacket.Push(iItemNo);	// 변환할 아이템 번호
	kPacket.Push(m_iItemCount); // 생성될 목표 아이템의 개수
	NETWORK_SEND(kPacket)

	InProgress(true);
}

int PgItemConvertSystemMgr::GetItemFirstNo()
{
	return iFirstItemNo;
}

int PgItemConvertSystemMgr::GetItemFirstCount()
{
	return m_iItemFirstCount;
}

int PgItemConvertSystemMgr::GetItemSecondNo()
{
	return iSecondItemNo;
}

int PgItemConvertSystemMgr::GetItemSecondCount()
{
	return m_iItemSecondCount;
}

bool PgItemConvertSystemMgr::PreCheckItemConvert(int const iCount)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	if( SItemPos::NullData() == m_kSrcItemPos )
	{
		return false;
	}

	if( m_kItem.IsEmpty() )
	{
		return false;
	}

	int const m_iItemTotalSourceCount = pInv->GetTotalCount(m_kItem.ItemNo());

	if( iCount > m_iItemTotalSourceCount )
	{
		return false;
	}

	return true;
}

int PgItemConvertSystemMgr::ItemConvertMaxCount()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return 0;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return 0;}

	if( m_kItem.IsEmpty() )
	{
		return 0;
	}

	CONT_DEF_CONVERTITEM const * pkDef = NULL;
	g_kTblDataMgr.GetContDef(pkDef);

	CONT_DEF_CONVERTITEM::const_iterator iter = pkDef->find(m_kItem.ItemNo());
	if( pkDef->end() != iter )
	{
		int const m_iItemTotalSourceCount = pInv->GetTotalCount(m_kItem.ItemNo());
		int const m_iItemFirstCount = (*iter).second.sSourceItemNum;

		int const iTotalCount = (m_iItemTotalSourceCount / m_iItemFirstCount);

		return iTotalCount;
	}
	return 0;
}

void Recv_PT_M_C_ANS_CONVERTITEM(BM::Stream* pkPacket)
{
	HRESULT hResult;
	
	pkPacket->Pop(hResult);	

	g_kItemConvertSystemMgr.InProgress(false);

	int iWarnMessage = 0;

	switch( hResult )
	{
	case S_OK:
		{
			iWarnMessage = 790532;
		}break;
	case E_CI_NOT_FOUND_SOURCEITEM:// 변환할대상아이템을찾을수없다.
		{
			lwAddWarnDataTT(790533);
			return;
		}break;
	case E_CI_IS_NOT_CONVERTITEM:// 변환가능한아이템이아니다.
		{
			lwAddWarnDataTT(790534);
			return;
		}break;
	case E_CI_INVALID_CONVERTNUM:// 변환할수있는수량이아니다.(0 보다작거나같다.)
		{
			lwAddWarnDataTT(790535);
			return;
		}break;
	case E_CI_NOT_ENOUGH_SOURCEITEM:// 변환할아이템수량이부족하다.
		{
			lwAddWarnDataTT(790536);
			return;
		}break;
	default:
		{
			return;
		}break;
	}

	int const iResultItemNo = g_kItemConvertSystemMgr.GetItemSecondNo();

	GET_DEF(CItemDefMgr, kItemDefMgr);
	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);

	CItemDef const *pItemDef = kItemDefMgr.GetDef(iResultItemNo);
	if (!pItemDef)
	{
		return;
	}

	CONT_DEFRES::const_iterator itrDest = pContDefRes->find(pItemDef->ResNo());
	if(itrDest == pContDefRes->end())
	{
		// 아이템 테이블에서 못 찾으면 패스!
		return ;
	}

	// 아웃풋이 하나밖에 없으니. 지정 UI를 띄우자.
	lwUIWnd kResultUI = lwActivateUI("FRM_ITEM_MAKING_RESULT_SINGLE");
	if (!kResultUI())
	{
		return ;
	}
	lwUIWnd kImgUI = kResultUI.GetControl("IMG_GET");
	if (!kImgUI())
	{
		return;
	}

	const TBL_DEF_RES& rkRes = (*itrDest).second;
	std::string strIconPath = MB(rkRes.strIconPath);
	kImgUI.ChangeImage( (char*)strIconPath.c_str(), false );
	kImgUI.SetUVIndex(rkRes.UVIndex);
	kImgUI.SetUVIndex(rkRes.UVIndex);
	kImgUI.SetCustomData<int>(iResultItemNo);

	const wchar_t *pText = NULL;
	std::wstring wstrText;
	GetDefString(iResultItemNo, pText);
	wstrText += pText;
	wstrText += _T("\n");
	wstrText += (std::wstring)(BM::vstring(g_kItemConvertSystemMgr.GetItemSecondCount()));
	wstrText += TTW(1709);		// '개'
	wstrText += _T(" ");
	wstrText += TTW(1600);

	kResultUI()->Text(wstrText);



	char szName[100] = "EnchantFail";
	ENoticeLevel eLevel = EL_Warning;

	if( S_OK == hResult )
	{
		sprintf(szName, "EnchantSuccess");
		eLevel = EL_Normal;
	}

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
	Notice_Show_ByTextTableNo(iWarnMessage, eLevel);
	g_kItemConvertSystemMgr.InProgress(false);
}

 void Recv_PT_M_C_NFY_EMPORIA_FUNCTION( PgWorld *pkWorld, BM::Stream &rkPacket )
 {
	 size_t iSize = 0;
	 rkPacket.Pop( iSize );

	 while ( iSize-- )
	 {
		std::string strTriggerID;
		SEmporiaGuildInfo kGuildInfo;

		rkPacket.Pop( strTriggerID );
		kGuildInfo.ReadFromPacket( rkPacket );

		 if ( pkWorld )
		 {
			 pkWorld->UpdatePortalObject( strTriggerID, kGuildInfo );
		 }
	 }
 }
