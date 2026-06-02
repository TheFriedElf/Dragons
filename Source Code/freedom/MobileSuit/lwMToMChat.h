#ifndef FREEDOM_DRAGONICA_CONTENTS_1_1_LWMOTOMCHAT_H
#define FREEDOM_DRAGONICA_CONTENTS_1_1_LWMOTOMCHAT_H

#include "PgMToMChat.h"

void lwMToMChatClose(lwUIWnd UIParent);
void lwMToMChatDlgStateChange(lwUIWnd UIParent);
void lwMToMMiniFlash(lwUIWnd UIParent);
void lwMToMRefreshUI(lwUIWnd UIParent);
void lwMToMChatNewDlg(lwGUID Guid);
void lwMToMSendChatLog(lwUIWnd UIParent);
void lwMToMMiniToolTip(lwUIWnd UIParent, int const iTextNo, lwPoint2 Pt);
void lwMToMMapMoveReCheck();

void MToMAlignMiniDlg();
void MToMChatNewDlg( lwGUID Guid, BYTE const kType );
XUI::CXUI_Wnd* MToMChatNewDlgMini( lwGUID Guid, BYTE const kType );
void MToMChatUIListClear(lwUIWnd& UIParent);
void MToMChatUIListAdd(lwUIWnd& UIParent, BM::GUID const& Guid, SMtoMChatData const& ChatData);

#endif // FREEDOM_DRAGONICA_CONTENTS_1_1_LWMOTOMCHAT_H