#include "StdAfx.h"
#include "PgNetwork.h"
#include "ServerLib.h"
#include "Lohengrin/ErrorCode.h "
#include "Variant/Inventory.H"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "lwUIFireLove.h"

PgFireLove::tagFireloveSMSInfo::tagFireloveSMSInfo()
:iDurationTime(0)
{
	::memset(&kSMSRecvTime, static_cast<WORD>(0), sizeof(SYSTEMTIME));
}

PgFireLove::tagFireloveSMSInfo::tagFireloveSMSInfo(std::wstring const& kBoyName_in, std::wstring const& kGirlName_in, int iTime)
:kBoyName(kBoyName_in)
,kGirlName(kGirlName_in)
,iDurationTime(iTime)
{	
	::GetLocalTime(&kSMSRecvTime);
}

PgFireLove::PgFireLove()
{
}
PgFireLove::~PgFireLove()
{
	Clear();
}

bool PgFireLove::Pop() 
{	
	if(m_kCont.empty())
	{	
		return false;
	}
	m_kCont.pop_front();
	return true;
}

void PgFireLove::Clear()
{
	m_kCont.clear();
}

bool PgFireLove::Empty() const
{
	return m_kCont.empty();
}

bool PgFireLove::PopAndRemainSmsShow()
{
	Pop();
	if(Empty())
	{
		return false;
	}
	
	//XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(kFormName);
	//if(pkWnd)
	//{// 사랑의 불꽃은 call이 재호출 되어야 하므로 닫아버리고 다시 연다
	//	pkWnd->Close();
	//	pkWnd = NULL;
	//}
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(_T("FRM_EVT_FIRELOVE_MSG"));
	if( !pkWnd )
	{
		return false;
	}
	return SetDataToUI(pkWnd);
}

bool PgFireLove::PushDataAndSetToUI(XUI::CXUI_Wnd* const pkWnd, SFireLoveSMSInfo const& kInfo)
{
	if(!pkWnd)
	{
		return false;
	}
	m_kCont.push_back(kInfo);
	return SetDataToUI(pkWnd);
}

bool PgFireLove::SetDataToUI(XUI::CXUI_Wnd* const pkWnd) const
{
	if(Empty())
	{
		return false;
	}
	CONT_FIRELOVE::const_iterator Info_itor = m_kCont.begin();
	pkWnd->AliveTime(Info_itor->iDurationTime);
	XUI::CXUI_Wnd* const pkBoyName = pkWnd->GetControl(_T("FRM_BOY_NAME"));
	XUI::CXUI_Wnd* const pkGirlName = pkWnd->GetControl(_T("FRM_GIRL_NAME"));
	if(!pkBoyName || !pkGirlName)
	{
		return false;
	}

	pkBoyName->Text(Info_itor->kBoyName);
	pkGirlName->Text(Info_itor->kGirlName);

	XUI::CXUI_Wnd* const pkRecvTime = pkWnd->GetControl(_T("SFRM_RECV_TIME"));	
	if(pkRecvTime)
	{//메세지 받은 시간 보여줌
		wchar_t szBuf[MAX_PATH] ={0,};
		WORD const& wHour = Info_itor->kSMSRecvTime.wHour;
		WORD const& wMin = Info_itor->kSMSRecvTime.wMinute;
		WORD const& wSec = Info_itor->kSMSRecvTime.wSecond;
		wsprintfW(szBuf, TTW(790573).c_str(), wHour, wMin, wSec);		
		pkRecvTime->Text(szBuf);
	}
	
	return true;
}

namespace lwUIFireLove
{
	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "Show_FireLoveRemainSMS", &Show_FireLoveRemainSMS);
		def(pkState, "Clear_FireLoveRemainSMS", &Clear_FireLoveRemainSMS);
		
		return true;
	}	
	
	bool Show_FireLoveRemainSMS()
	{
		return g_kFireLove.PopAndRemainSmsShow();
	}

	void Clear_FireLoveRemainSMS()
	{
		g_kFireLove.Clear();
		XUIMgr.Close(_T("FRM_EVT_FIRELOVE_MSG"));
	}
}
