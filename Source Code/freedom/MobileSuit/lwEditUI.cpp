#include "StdAfx.h"
#include "lwUI.h"
#include "PgNetwork.h"
#include "BM/PgFilterString.h"
#include "PgCommandMgr.h"

#include "PgChatMgrClient.h"

//기타 채팅 도우미
void lwUIWnd::ExploreTypingHistory(bool const bIsPrev)
{
	XUI::CXUI_Edit *pEdit= dynamic_cast<XUI::CXUI_Edit*>(self);
	assert(pEdit);
	if( !pEdit )
	{
		return;
	}

	if( !pEdit->IsFocus() )
	{
		return;
	}

	std::wstring kInputedChat;
	g_kChatMgrClient.InputChatLog_Get(kInputedChat, bIsPrev);

	pEdit->EditText(kInputedChat);
}

void lwUIWnd::ResetTypingHistory()
{
	XUI::CXUI_Edit *pEdit= dynamic_cast<XUI::CXUI_Edit*>(self);
	assert(pEdit);
	if( !pEdit )
	{
		return;
	}
	
	if( !pEdit->IsFocus() )
	{
		return;
	}

	g_kChatMgrClient.InputChatLog_Reset();
}
