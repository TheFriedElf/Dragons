#include "StdAfx.h"
#include "lwUI.h"
#include "PgNetwork.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgWorld.h"
#include "ServerLib.h"
#include "PgMobileSuit.h"
#include "PgXmlLoader.h"
#include "PgItemEx.h"
#include "PgDropBox.h"
#include "PgAction.h"
#include "Variant/PgPlayer.h"
#include "Variant/constant.h"
#include "PgQuest.h"
#include "PgRemoteManager.h"
#include "PgNifMan.h"

#include "PgClientParty.h"
#include "PgRenderMan.h"

#include "BM/PgFilterString.h"

#include "PgChatMgrClient.h"
#include "PgFriendMgr.h"

void lwAddWarnDataStr(lwWString Str, int const iLevel, bool const bCheckEqualMessage)		//공지 및 정보사항창. level로 색상설정
{
	::Notice_Show( Str(), iLevel, bCheckEqualMessage);
}

void lwAddWarnData(int const iDefStringNo)
{
	::Notice_Show_ByDefStringNo(iDefStringNo, EL_Warning);
}

void lwAddWarnDataTT(int const iTextTableNo, bool const bCheckEqualMessage)
{
	::Notice_Show_ByTextTableNo(iTextTableNo, EL_Warning, bCheckEqualMessage);
}

void lwUIWnd::RefreshMessageLog()
{
	return ;
	//XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	//g_kChatMgrClient.XUI_DrawItem(pList);
}

bool lwUIWnd::GetCheckState()
{
	XUI::CXUI_CheckButton *pChkBtn= dynamic_cast<XUI::CXUI_CheckButton*>(self);
	assert(pChkBtn);
	if(pChkBtn)
	{
		return pChkBtn->Check();
	}
	return false;
}

void lwUIWnd::CheckState(bool bIsCheck)
{
	XUI::CXUI_CheckButton *pChkBtn= dynamic_cast<XUI::CXUI_CheckButton*>(self);
	assert(pChkBtn);
	if(pChkBtn)
	{
		pChkBtn->Check(bIsCheck);
	}
}
void lwUIWnd::LockClick(bool bLock)
{
	XUI::CXUI_CheckButton *pChkBtn= dynamic_cast<XUI::CXUI_CheckButton*>(self);
	assert(pChkBtn);
	if(pChkBtn)
	{
		pChkBtn->ClickLock(bLock);
	}
}

void lwUIWnd::Disable(bool bDisable)
{
	CXUI_Button *pBtn = dynamic_cast<XUI::CXUI_Button*>(self);
	assert(pBtn);
	if (pBtn)	pBtn->Disable(bDisable);
}

bool lwUIWnd::IsDisable()
{
	CXUI_Button *pBtn = dynamic_cast<XUI::CXUI_Button*>(self);
	assert(pBtn);
	if(pBtn) return !pBtn->Enable();
	return false;
}

void lwUIWnd::SetButtonTextColor(int iState, float fColor)
{
	CXUI_Button *pBtn = dynamic_cast<XUI::CXUI_Button*>(self);
	assert(pBtn);
	if (pBtn)	pBtn->m_kFontColorByState.Set( __min(3, __max(0,iState)), static_cast<DWORD>(fColor) );
}
