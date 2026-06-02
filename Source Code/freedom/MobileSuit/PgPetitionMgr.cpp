#include "stdafx.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgPetitionMgr.h"
#include "lwUI.h"

PgPetitionMgr::PgPetitionMgr(void)
{
}

PgPetitionMgr::~PgPetitionMgr(void)
{
}
bool PgPetitionMgr::ReceiptPetition_Success(int ReceiptIndex)
{
	XUI::CXUI_Wnd *pkTop = XUIMgr.Get(_T("SFRM_PETITIONDLG"));
	if(pkTop == NULL)
		return false;
	pkTop->Close();

	wchar_t szBuf[1024] = {0,};
	wsprintfW(szBuf, TTW(810039).c_str(), ReceiptIndex);
	lua_tinker::call<void, char const*, bool, int >("CommonMsgBox", MB(szBuf), true, 0);	

	return true;
}

bool PgPetitionMgr::Send_Petition(std::wstring const &wsTitle, std::wstring const &wsNote)
{
	BM::Stream kPacket(PT_C_GM_REQ_RECEIPT_PETITION);
	//패킷에 넣고 보내자!!
	PgPilot	*pkPilot = g_kPilotMan.GetPlayerPilot();
	if(pkPilot == NULL)
		return false;
	CUnit *pkUnit = pkPilot->GetUnit();
	if (pkUnit == NULL)
		return false;
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
	std::wstring const kName =  pkPlayer->Name();
	BM::GUID const kMemberId = pkPlayer->GetMemberGUID();
	kPacket.Push(kMemberId);
	kPacket.Push(kName);
	kPacket.Push(wsTitle);
	kPacket.Push(wsNote);
	NETWORK_SEND(kPacket)
	return bSendRet;
}

bool PgPetitionMgr::Select_Remainder_Petition()
{
	
	BM::Stream kPacket(PT_C_GM_REQ_REMAINDER_PETITION);
	//패킷에 넣고 보내자!!
	PgPilot	*pkPilot = g_kPilotMan.GetPlayerPilot();
	if(pkPilot == NULL)
		return false;
	CUnit *pkUnit = pkPilot->GetUnit();
	if (pkUnit == NULL)
		return false;
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
	BM::GUID const MemberId = pkPlayer->GetMemberGUID();
	kPacket.Push(MemberId);

	NETWORK_SEND(kPacket)
	return bSendRet;
}

bool PgPetitionMgr::PetitionDlg_Show(int iNumber)
{
	char* szTemp = 0;
	iNumber = 6 - iNumber;
	if(iNumber <= 0)
		lua_tinker::call<void, char const*, bool, int >("CommonMsgBox", MB(TTW(810040)), true, 0);	
	else
	{
		BM::vstring vsNum(iNumber);

		lwCallUI("SFRM_PETITIONDLG");

		//Title 내용을 얻고
		XUI::CXUI_Wnd *pkTop = XUIMgr.Get(_T("SFRM_PETITIONDLG"));
		if(pkTop == NULL)
			return false;
		XUI::CXUI_Wnd *pkWnd = pkTop->GetControl(_T("SFRM_COLOR4"));
		if(pkWnd == NULL)
			return false;
		XUI::CXUI_Wnd *pkLabel = (XUI::CXUI_Edit*)pkWnd->GetControl(_T("SFRM_REMAINDER_TEXT"));
		if(pkLabel == NULL)
			return false;
		pkLabel->Text(vsNum);
	}

	return true;	


}

void PgPetitionMgr::WaitReceipt_Petition()
{
	lua_tinker::call<void, char const*, bool, int >("CommonMsgBox", MB(TTW(810041)), true, 0);	
}