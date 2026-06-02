#include "stdafx.h"
#include "PgMobileSuit.h"
#include "lwUI.h"
#include "PgNetwork.h"
#include "PgSystemInventory.h"
#include "PgChatMgrClient.h"
#include "PgEventTimer.h"
#include "PgWorld.h"
#include "PgRenderman.h"
#include "PgPilotMan.h"

std::wstring const kSYSTEM_INVEN_FRM_ORG( _T("SFRM_SYSTEM_INVEN") );
TCHAR const *szSYSTEM_INVEN_FRM = _T("SFRM_SYSTEM_INVEN%u");
char const *szEVENTTIMER_ID = "SYSTEN_INVEN_%u";
TCHAR const *szSYSTEM_INVEN_ICON_FRM = _T("ICN_SYSTEM_INVEN%u");
std::wstring const kUSER_INVEN_UI_NAME( _T("Inv") );
size_t const iMaxSystemInvenSetCount = 100;
size_t const iMaxOneSetInvenCount = 12;

float const PgSystemInventory::fUpTime = SYSTEM_INVENTORY_TIME_LIMIT * 0.0001f;

namespace PgSystemInventoryUtil
{
	bool CheckCanInsertTargetPos(PgPlayer* pkPlayer, CItemDef const* pkDefItem, SItemPos const& rkTargetPos)
	{
		// pkPlayer, pkDefItem NULL은 외부에서
		PgInventory* pkInven = pkPlayer->GetInven();
		if( !pkInven )
		{
			return false;
		}

		PgBase_Item	kTargetItem;
		HRESULT const iRet = pkInven->GetItem(rkTargetPos, kTargetItem); // 목적지에 아이템이 이미 존재 한가?
		if( S_OK == iRet )
		{
			return false;
		}
		else
		{
			if( pkDefItem->PrimaryInvType() != rkTargetPos.x ) // 인벤토리 위치가 Def 정보와 다르면
			{
				return false;
			}
		}
		return true;
	}
};

PgSystemInventory::PgSystemInventory()
{
	Clear();
}

PgSystemInventory::~PgSystemInventory()
{
}

void PgSystemInventory::Clear()
{
	BM::CAutoMutex kLock(m_kMutex);
	m_kEmptyList.clear();
	for(size_t iCur = 0; iMaxSystemInvenSetCount > iCur; ++iCur)
	{
		std::back_inserter(m_kEmptyList) = iCur;
	}
	m_kSysInv.clear();
}

void PgSystemInventory::Update(SSystemInven const &rkSystemInven)
{
	if( rkSystemInven.Empty() )
	{
		return;
	}

	float const fCurTime = g_pkApp->GetAccumTime();
	float const fDiffTime = rkSystemInven.fEndTime - fCurTime;
	if( 0 > fDiffTime )
	{
		return;
	}

	TCHAR szTemp[512] = {0, };
	_stprintf_s(szTemp, szSYSTEM_INVEN_FRM, rkSystemInven.iSetNo);

	std::wstring const kCurWndName(szTemp);

	XUI::CXUI_Wnd *pkTopWnd = XUIMgr.Get( kCurWndName );
	if( !pkTopWnd )
	{
		pkTopWnd = XUIMgr.Call( kSYSTEM_INVEN_FRM_ORG, false, kCurWndName );
		if( !pkTopWnd )
		{
			return;
		}
	}

	{
		size_t const iOneColumnPerWindow = 7;
		POINT3I const kColumnToColumnOffset(40, 0, 0);
		POINT3I const kOneWindowPerOffset(20, 30, 0);
		POINT3I const kBasePos(260, 100, 0); // iMaxSystemInvenSetCount - rkSystemInven.iSetNo);

		int const iFromTopOffsetPos = rkSystemInven.iSetNo % iOneColumnPerWindow; // From Top pos
		int const iFromLeftOffsetPos = rkSystemInven.iSetNo / iOneColumnPerWindow; // From Left pos

		pkTopWnd->Location( kBasePos + (kOneWindowPerOffset * iFromTopOffsetPos) + (kColumnToColumnOffset * iFromLeftOffsetPos) );
	}

	XUI::CXUI_Wnd *pkTitleWnd = pkTopWnd->GetControl(_T("SFRM_TITLE"));
	if( pkTitleWnd )
	{
		std::wstring kTemp;
		if( FormatTTW(kTemp, 400429, rkSystemInven.iSetNo + 1) )
		{
			pkTitleWnd->Text( kTemp );
		}
	}

	XUI::CXUI_Wnd *pkTimeWnd = pkTopWnd->GetControl(_T("SFRM_TIMER_COUNT"));
	if( pkTimeWnd )
	{
		pkTimeWnd->SetCustomData(&rkSystemInven.fEndTime, sizeof(float));
	}

	pkTopWnd->SetCustomData(&rkSystemInven.iSetNo, sizeof(rkSystemInven.iSetNo));

	//KUIG_SYSTEM_INVEN
	typedef std::vector< XUI::CXUI_Icon * > ContIconVec;
	ContIconVec kIconVec;
	size_t iCur = 0;
	for( ; iMaxOneSetInvenCount > iCur; ++iCur )
	{
		//vector< CXUI_Icon* >::push_back( *ICN_SYSTEM_INVEN%u ); // 0 <= %u < iMaxOneSetInvenCount
		_stprintf_s(szTemp, szSYSTEM_INVEN_ICON_FRM, iCur);
		std::back_inserter(kIconVec) = dynamic_cast<XUI::CXUI_Icon *>( pkTopWnd->GetControl(std::wstring(szTemp)) );
	}

	iCur = iMaxOneSetInvenCount * rkSystemInven.iSetNo;
	ContIconVec::iterator wnd_iter = kIconVec.begin();
	ContSystemInven::const_iterator info_iter = rkSystemInven.begin();
	while( rkSystemInven.end() != info_iter && kIconVec.end() != wnd_iter )
	{
		ContIconVec::value_type pkIconWnd = (*wnd_iter);
		if( pkIconWnd )
		{
			SIconInfo kNewIconInfo(KUIG_SYSTEM_INVEN, iCur);
			pkIconWnd->SetIconInfo( kNewIconInfo );
			pkIconWnd->Visible(true);
		}
		++info_iter, ++wnd_iter, ++iCur;
	}

	while( kIconVec.end() != wnd_iter )
	{
		if( *wnd_iter )
		{
			(*wnd_iter)->Visible(false);
		}
		++wnd_iter;
	}

	XUI::CXUI_Wnd *pkInvWnd = XUIMgr.Get( kUSER_INVEN_UI_NAME );
	if( !pkInvWnd )
	{
		XUIMgr.Call( kUSER_INVEN_UI_NAME );
	}

	{
		char szEventID[512] = {0, }, szScript[512] = {0, };
		sprintf_s(szEventID, szEVENTTIMER_ID, rkSystemInven.iSetNo);
		sprintf_s(szScript, "TimeOutSystemInventory(%u)", rkSystemInven.iSetNo);

		STimerEvent kNewEvent;
		kNewEvent.Set(fDiffTime, szScript);
		g_kEventTimer.AddLocal(szEventID, kNewEvent);
	}
}

void PgSystemInventory::Remove(size_t const iSetNo)
{
	ContSystemInvenList::iterator find_iter = std::find( m_kSysInv.begin(), m_kSysInv.end(), iSetNo );
	if( m_kSysInv.end() == find_iter )
	{
		return;
	}
	
	m_kSysInv.erase( find_iter );

	TCHAR szTemp[512] = {0, };
	_stprintf_s(szTemp, szSYSTEM_INVEN_FRM, iSetNo);
	std::wstring const kCurWndName( szTemp );
	XUIMgr.Close( kCurWndName );

	m_kEmptyList.push_front(iSetNo);
	m_kEmptyList.sort();

	char szEventID[512] = {0, };
	sprintf_s(szEventID, szEVENTTIMER_ID, iSetNo);

	std::string kSceneID;
	if( g_kRenderMan.GetFirstTypeID< PgWorld >(kSceneID) )
	{
		g_kEventTimer.Del(kSceneID.c_str(), szEventID);
	}
}

void PgSystemInventory::RemoveProcess(CONT_ITEMGUID const &rkVec)
{
	ContSetNoVec kReserveErase;
	ContSystemInvenList::iterator iter = m_kSysInv.begin();
	while( m_kSysInv.end() != iter )
	{
		ContSystemInvenList::value_type &rkElement = (*iter);
		bool const bUpdated = rkElement.Remove( rkVec );

		if( bUpdated && rkElement.Empty() )
		{
			std::back_inserter(kReserveErase) = rkElement.iSetNo;
		}
		else
		{
			if( bUpdated )
			{
				Update( rkElement );
			}
		}

		++iter;
	}

	ContSetNoVec::iterator remove_iter = kReserveErase.begin();
	while( kReserveErase.end() != remove_iter )
	{
		Remove( *remove_iter );
		++remove_iter;
	}
}

PgSystemInventory::value_type const* PgSystemInventory::GetAt(size_t const iCur)
{
	BM::CAutoMutex kLock(m_kMutex);
	size_t const iSetNo = (size_t)(iCur / iMaxOneSetInvenCount);
	size_t const iCurAt = iCur % iMaxOneSetInvenCount;
	ContSystemInvenList::iterator find_iter = std::find( m_kSysInv.begin(), m_kSysInv.end(), iSetNo );
	if( m_kSysInv.end() != find_iter )
	{
		return (*find_iter).Get(iCurAt);
	}
	return NULL;
}

void PgSystemInventory::SendItemGetAt(int const iAt, SItemPos const &rkTargetPos)
{
	BM::CAutoMutex kLock(m_kMutex);
	value_type const *pkSystemInvenItem = GetAt(iAt);
	if( !pkSystemInvenItem )
	{
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pkDefItem = kItemDefMgr.GetDef( pkSystemInvenItem->ItemNo() );
	if( !pkDefItem )
	{
		return;
	}

	SItemPos kTargetPos = rkTargetPos;
	bool bInsertAnyPos = (rkTargetPos == rkTargetPos.NullData());
	if( !bInsertAnyPos )
	{
		if( !PgSystemInventoryUtil::CheckCanInsertTargetPos(pkPlayer, pkDefItem, rkTargetPos) )
		{
			bInsertAnyPos = true;
			kTargetPos.Clear();
		}
	}

	SSys2InvInfo kOneGetItem;
	kOneGetItem.kItemGuid = pkSystemInvenItem->Guid();
	kOneGetItem.kItemPos = kTargetPos;
	kOneGetItem.bAnyPos = bInsertAnyPos;

	CONT_SYS2INV_INFO kList;
	std::back_inserter(kList) = kOneGetItem;

	SendItemGet(kList);
}

void PgSystemInventory::SendItemGet(CONT_SYS2INV_INFO const &rkReqList)
{
	BM::CAutoMutex kLock(m_kMutex);
	BM::Stream kPacket;
	SPT_C_M_REQ_SYSTEM_INVENTORY_RECV kReqInfo(rkReqList);
	kReqInfo.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)
}

void PgSystemInventory::RecvSystemInven(BM::Stream &rkPacket)
{
	BM::CAutoMutex kLock(m_kMutex);

	ContRecvSystemInven kList;
	PU::TLoadArray_M(rkPacket, kList);

	if( m_kEmptyList.empty() )
	{
		CONT_ITEMGUID kGiveupVec;
		ContRecvSystemInven::const_iterator giveup_iter = kList.begin();
		while( kList.end() != giveup_iter )
		{
			std::back_inserter(kGiveupVec) = (*giveup_iter).Guid();
			++giveup_iter;
		}
		SendDropSysInven( kGiveupVec );
		return;
	}

	size_t const iCurSetNo = m_kEmptyList.front();

	SSystemInven kNewInven(iCurSetNo, kList, g_pkApp->GetAccumTime());
	if( kNewInven.Empty() )
	{
		return;
	}

	if( m_kSysInv.end() != std::find( m_kSysInv.begin(), m_kSysInv.end(), iCurSetNo ) )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV1, _T("[%s]-[%d] Can't insert system-inven set[%d]"), __FUNCTIONW__, __LINE__, iCurSetNo);
		return;
	}

	m_kSysInv.push_back( kNewInven );

	m_kEmptyList.pop_front();
}

void PgSystemInventory::RecvModifySystemInven(BM::Stream &rkPacket)
{
	BM::CAutoMutex kLock(m_kMutex);
	SPT_M_C_ANS_SYSTEM_INVENTORY_RECV kRecvInfo;
	kRecvInfo.ReadFromPacket( rkPacket );

	if( S_OK != kRecvInfo.Result() )
	{
		g_kChatMgrClient.LogMsgBox(400432);
		return;
	}

	CONT_ITEMGUID kVec;
	CONT_SYS2INV_INFO::const_iterator iter = kRecvInfo.ContSysItem().begin();
	while( kRecvInfo.ContSysItem().end() != iter )
	{
		std::back_inserter(kVec) = (*iter).kItemGuid;
		++iter;
	}

	RemoveProcess( kVec );
}

void PgSystemInventory::RecvRemove(BM::Stream &rkPacket)
{
	BM::CAutoMutex kLock(m_kMutex);
	SPT_M_C_ANS_SYSTEM_INVENTORY_REMOVE kRecvInfo;
	kRecvInfo.ReadFromPacket(rkPacket);

	if( S_OK != kRecvInfo.Result() )
	{
		return;
	}

	RemoveProcess( kRecvInfo.ContItemGuid() );
}

bool PgSystemInventory::SysInvenSetToGuid(size_t const iSetNo, CONT_ITEMGUID &rkOut)
{
	ContSystemInvenList::iterator find_iter = std::find( m_kSysInv.begin(), m_kSysInv.end(), iSetNo );
	if( m_kSysInv.end() == find_iter )
	{
		return false;
	}

	ContSystemInvenList::value_type const &rkSysInven = (*find_iter);
	ContSystemInven::const_iterator iter = rkSysInven.begin();
	while( rkSysInven.end() != iter )
	{
		if( (*iter).ItemNo() )
		{
			std::back_inserter(rkOut) = (*iter).Guid();
		}
		++iter;
	}

	return !rkOut.empty();
}

void PgSystemInventory::SendDropSysInven(size_t const iSetNo)
{
	BM::CAutoMutex kLock(m_kMutex);
	CONT_ITEMGUID kVec;
	if( !SysInvenSetToGuid(iSetNo, kVec) )
	{
		return;
	}
	SendDropSysInven(kVec);
}

void PgSystemInventory::SendDropSysInven(CONT_ITEMGUID const &rkVec)
{
	BM::Stream kPacket;
	SPT_C_M_REQ_SYSTEM_INVENTORY_REMOVE kSendInfo(rkVec);
	kSendInfo.WriteToPacket( kPacket );
	NETWORK_SEND(kPacket)
}

void PgSystemInventory::SendGetSysInven(size_t const iSetNo)
{
	BM::CAutoMutex kLock(m_kMutex);
	CONT_ITEMGUID kVec;
	if( !SysInvenSetToGuid(iSetNo, kVec) )
	{
		return;
	}

	CONT_SYS2INV_INFO kReqVec;
	CONT_ITEMGUID::const_iterator iter = kVec.begin();
	while( kVec.end() != iter )
	{
		CONT_SYS2INV_INFO::value_type kNewElement;
		kNewElement.kItemGuid = (*iter);
		kNewElement.bAnyPos = true;
		std::back_inserter(kReqVec) = kNewElement;
		++iter;
	}

	SendItemGet(kReqVec);
}

void PgSystemInventory::CheckSystemInventory()
{
	BM::CAutoMutex kLock(m_kMutex);
	ContSetNoVec kReserveErase;
	float const fNowTime = g_pkApp->GetAccumTime();
	ContSystemInvenList::iterator iter = m_kSysInv.begin();
	while( m_kSysInv.end() != iter )
	{
		ContSystemInvenList::value_type const &rkElement = (*iter);

		float const fDiffTime = rkElement.fEndTime - fNowTime;
		if( 0 < fDiffTime
		&&	!rkElement.Empty() )
		{
			Update( rkElement );
		}
		else
		{
			std::back_inserter(kReserveErase) = rkElement.iSetNo; //시간이 지나거나, 비었으면 삭제 하자
		}
		++iter;
	}

	ContSetNoVec::const_iterator remove_iter = kReserveErase.begin();
	while( kReserveErase.end() != remove_iter )
	{
		Remove( *remove_iter );
		++remove_iter;
	}
}

void PgSystemInventory::TimeOutSystemInventory(size_t const iSetNo)
{
	BM::CAutoMutex kLock(m_kMutex);

	Remove( iSetNo );

	std::wstring kMessage;
	if( FormatTTW(kMessage, 400433, iSetNo + 1) )
	{
		g_kChatMgrClient.LogMsgBox(kMessage);
	}

	XUI::CXUI_Cursor *pkCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
	if( !pkCursor )
	{
		return;
	}

	SIconInfo &rkIconInfo = pkCursor->IconInfo();
	if( KUIG_SYSTEM_INVEN == rkIconInfo.iIconGroup
	&&	iSetNo == (size_t)(rkIconInfo.iIconKey / iMaxOneSetInvenCount) )
	{
		rkIconInfo.Clear();
	}
}


void lwSystemInventory::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "SendGetSystemInventory", SendGetSystemInventory);
	def(pkState, "DropSystemInventory", DropSystemInventory);
	def(pkState, "CheckSystemInventory", CheckSystemInventory);
	def(pkState, "TimeOutSystemInventory", TimeOutSystemInventory);
}

void lwSystemInventory::SendGetSystemInventory(int const iSetNo)
{
	g_kSystemInven.SendGetSysInven((size_t)iSetNo);
}

void lwSystemInventory::DropSystemInventory(int const iSetNo)
{
	std::wstring kMessage;
	if( FormatTTW(kMessage, 400431, iSetNo + 1) )
	{
		CallYesNoMsgBox(kMessage, BM::GUID::NullData(), MBT_CONFIRM_DROP_SYSTEM_INVEN, iSetNo);
	}
}

void lwSystemInventory::CheckSystemInventory()
{
	g_kSystemInven.CheckSystemInventory();
}

void lwSystemInventory::TimeOutSystemInventory(int const iSetNo)
{
	g_kSystemInven.TimeOutSystemInventory(iSetNo);
}