#include "stdafx.h"
#include "lwUI.h"
#include "lwBase.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "PgPilotMan.h"
#include "PgPetitionMgr.h"
#include "lwUIPetition.h"

lwUIPetition::lwUIPetition(PgPetitionMgr* pkPetitionMgr)
{
	m_pkPetitionMgr = pkPetitionMgr;
}

lwUIPetition::~lwUIPetition(void)
{
}

//! 스크립팅 시스템에 등록한다.
bool lwUIPetition::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwUIPetition>(pkState, "PetitionDlg")
		.def(pkState, constructor<PgPetitionMgr *>())
	.def(pkState, "Send_Petition_To_GM", &lwUIPetition::Send_Petition_To_Gm)
	.def(pkState, "Remainder_Petition", &lwUIPetition::Remainder_Petition)
	;

	return true;
}

bool lwUIPetition::Send_Petition_To_Gm()
{
	//Title 내용을 얻고
	XUI::CXUI_Wnd *pkTop = XUIMgr.Get(_T("SFRM_PETITIONDLG"));
	if(pkTop == NULL)
		return false;
	XUI::CXUI_Wnd *pkWnd = pkTop->GetControl(_T("SFRM_COLOR1"));
	if(pkWnd == NULL)
		return false;
	XUI::CXUI_Edit *pkTitle = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(_T("SFRM_TITLE_EDIT")));
	if(pkTitle == NULL)
		return false;
	std::wstring const wsTitle = pkTitle->EditText();
	//제목은 있어야 한다.
	if( wsTitle.c_str() == NULL)
	{
		lua_tinker::call<void, char const*, bool, int >("CommonMsgBox", MB(TTW(810042)), true, 0);
		return true;
	}
	//진정 내용을 가저오고
	pkWnd = pkTop->GetControl(_T("SFRM_COLOR3"));
	if(pkWnd == NULL)
		return false;
	XUI::CXUI_Edit *pkNote = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(_T("SFRM_NOTE_EDIT")));
	if(pkNote == NULL)
		return false;
	std::wstring const wsNote = pkNote->EditText();

	if (m_pkPetitionMgr)
	{
		m_pkPetitionMgr->Send_Petition(wsTitle, wsNote);
	}
	else
	{
		return false;
	}
	return true;	
}

bool lwUIPetition::Remainder_Petition()
{
	if (m_pkPetitionMgr)
	{
		return m_pkPetitionMgr->Select_Remainder_Petition();
	}

	return false;
}