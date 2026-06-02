#include "stdafx.h"
#include "lwUI.h"
#include "lwChatMgrClient.h"
#include "lwStyleString.h"
#include "PgPilotMan.h"
#include "PgUICalculator.h"

lwChatMgrClient::lwChatMgrClient(PgChatMgrClient* pkChatMgrClient)
{
	m_pkChatMgrClient = pkChatMgrClient;
	m_bInputNow = false;
	m_dwInputNow_Time = BM::GetTime32();
	InputNow_TimeMin(5000);//인터벌 5초
}
	
typedef void (*VOID_FUN_IN_INT)(const int);

//! 스크립팅 시스템에 등록한다.
bool lwChatMgrClient::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwChatMgrClient>(pkState, "ChatMgrClient")
		.def(pkState, constructor<PgChatMgrClient *>())
		.def(pkState, "SendChatW", &lwChatMgrClient::SendChatW)
		.def(pkState, "SendChatStyleString", &lwChatMgrClient::SendChatStyleString)
		//
		.def(pkState, "SendChat_InputNow", &lwChatMgrClient::SendChat_InputNow)
		//
		.def(pkState, "RecvChat", &lwChatMgrClient::RecvChat)
		//
		.def(pkState, "RecvChat_InputNow", &lwChatMgrClient::RecvChat_InputNow)
		//
		//.def(pkState, "Set_IsGodCommand", &lwChatMgrClient::Set_IsGodCommand)
		//
		.def(pkState, "Character_ChatBaloon_Set", &lwChatMgrClient::Character_ChatBaloon_Set)
		//
		.def(pkState, "Notice_Show", &lwChatMgrClient::Notice_Show)
		.def(pkState, "Notice_Show_ByDefStringNo", &lwChatMgrClient::Notice_Show_ByDefStringNo)
		.def(pkState, "Notice_Show_ByTextTableNo", &lwChatMgrClient::Notice_Show_ByTextTableNo)
		.def(pkState, "Notice_Show_ByTextTableNo2", &lwChatMgrClient::Notice_Show_ByTextTableNo_CheckEqualMessage)
		//
		.def(pkState, "InputNow_TimeMinimum", &lwChatMgrClient::InputNow_TimeMinimum)

		//
		.def(pkState, "ChatFilter_ModeClear", &lwChatMgrClient::ChatFilter_ModeClear)
		.def(pkState, "ChatFilter_ModeAdd", &lwChatMgrClient::ChatFilter_ModeAdd)
		.def(pkState, "ChatFilter_ModeDel", &lwChatMgrClient::ChatFilter_ModeDel)
		//
		.def(pkState, "NoticeLevelColor_Add", &lwChatMgrClient::NoticeLevelColor_Add)
		.def(pkState, "NoticeLevelColor_Clear", &lwChatMgrClient::NoticeLevelColor_Clear)
		//
		//.def(pkState, "Chat_Refresh", &lwChatMgrClient::Chat_Refresh)

		//.def(pkState, "LimitInput", &lwChatMgrClient::LimitInput)
		//.def(pkState, "LimitPrevInput", &lwChatMgrClient::LimitPrevInput)
		//.def(pkState, "SameInput", &lwChatMgrClient::SameInput)
		//.def(pkState, "LimitCount", &lwChatMgrClient::LimitCount)

		.def(pkState, "SetSpamChkFlag", &lwChatMgrClient::SetSpamChkFlag)
		.def(pkState, "SetMaxInputLog", &lwChatMgrClient::SetMaxInputLog)
		.def(pkState, "SetMaxBlockTime", &lwChatMgrClient::SetMaxBlockTime)
		.def(pkState, "SetBlockSpamChat_ChainInput", &lwChatMgrClient::SetBlockSpamChat_ChainInput)
		.def(pkState, "SetBlockSpamChat_LooseChainInput", &lwChatMgrClient::SetBlockSpamChat_LooseChainInput)
		.def(pkState, "SetBlockSpamChat_MacroInput", &lwChatMgrClient::SetBlockSpamChat_MacroInput)
		.def(pkState, "SetBlockSpamChat_SameInput", &lwChatMgrClient::SetBlockSpamChat_SameInput)

		.def(pkState, "ChatMode_Get", &lwChatMgrClient::ChatMode_Get)
		.def(pkState, "ChatMode_Set", &lwChatMgrClient::ChatMode_Set)
		.def(pkState, "ToggleConsecutiveChat", &lwChatMgrClient::ToggleConsecutiveChat)
		.def(pkState, "GetToggleConsecutiveChat", &lwChatMgrClient::GetToggleConsecutiveChat)
		.def(pkState, "ToggleConsecutiveChatUI", &lwChatMgrClient::ToggleConsecutiveChatUI)

		.def(pkState, "AddEventMessage", &lwChatMgrClient::AddEventMessage)
		.def(pkState, "AddLogMessage", &lwChatMgrClient::AddLogMessage)
		.def(pkState, "CheckChatOut", &lwChatMgrClient::CheckChatOut)
		.def(pkState, "ClearNotice", &lwChatMgrClient::ClearNotice)
		.def(pkState, "CheckChatCommand", &lwChatMgrClient::CheckChatCommand)
		.def(pkState, "SetWhisperDlg", &lwChatMgrClient::SetWhisperDlg)
		.def(pkState, "SetWhisperByTab", &lwChatMgrClient::SetWhisperByTab)
		.def(pkState, "ConvertEmoticonCommandToHex", &lwChatMgrClient::ConvertEmoticonCommandToHex)
		.def(pkState, "GetFontColor", &lwChatMgrClient::GetFontColor)
		.def(pkState, "SetFontColor", &lwChatMgrClient::SetFontColor)
		.def(pkState, "GuidToName", &lwChatMgrClient::Name2Guid_Find_ByGuid)
		.def(pkState, "OnFocus_ChatEdit", &lwChatMgrClient::OnFocus_ChatEdit)
		.def(pkState, "SetChatStation", &lwChatMgrClient::SetChatStation)
		.def(pkState, "Self_InputNow", &lwChatMgrClient::Self_InputNow)
		.def(pkState, "ChatAram", &lwChatMgrClient::ChatAram)
		.def(pkState, "ClearChatBalloon", &lwChatMgrClient::ClearChatBalloon)
		.def(pkState, "SetChatOutDefaultPos", &lwChatMgrClient::SetChatOutDefaultPos)
		.def(pkState, "SetSysChatOutDefaultPos", &lwChatMgrClient::SetSysChatOutDefaultPos)
		.def(pkState, "ShowNotifyConnectInfoUI", &lwChatMgrClient::ShowNotifyConnectInfoUI)
		.def(pkState, "SetExistWhisperList", &lwChatMgrClient::SetExistWhisperList)
		.def(pkState, "GetExistWhisperList", &lwChatMgrClient::GetExistWhisperList)
		.def(pkState, "SetSysChatOutSnap", &lwChatMgrClient::SetSysChatOutSnap)
		.def(pkState, "GetSysChatOutSnap", &lwChatMgrClient::GetSysChatOutSnap)
		.def(pkState, "CheckSnapPos", &lwChatMgrClient::CheckSnapPos)
		.def(pkState, "PopNotifyList", &lwChatMgrClient::PopNotifyList)
		.def(pkState, "GetSysOutHide", &lwChatMgrClient::GetSysOutHide)
		.def(pkState, "SetSysOutHide", &lwChatMgrClient::SetSysOutHide)
		.def(pkState, "GetNotifyGuid", &lwChatMgrClient::GetNotifyGuid)
		.def(pkState, "Name2Guid_Find_ByName", &lwChatMgrClient::Name2Guid_Find_ByName)
		.def(pkState, "CheckChatTag", &lwChatMgrClient::CheckChatTag)
		.def(pkState, "Regist_ChatBlockUser", &lwChatMgrClient::Regist_ChatBlockUser)
		.def(pkState, "UnRegist_ChatBlockUser", &lwChatMgrClient::UnRegist_ChatBlockUser)
		.def(pkState, "Modify_ChatBlockMode", &lwChatMgrClient::Modify_ChatBlockMode)
		.def(pkState, "UpdateChatBlockList", &lwChatMgrClient::UpdateChatBlockList)
		;
		//채팅 블록 상수값 
		lua_tinker::table LOCALE(pkState, "SPAM_CHAT");
		LOCALE.set("CHAININPUT", ESpamChkState::ESCS_ChainInput);
		LOCALE.set("LOOSECHAININPUT",  ESpamChkState::ESCS_LooseChainInput);
		LOCALE.set("MACROINPUT",  ESpamChkState::ESCS_MacroInput);
		LOCALE.set("SAMEINPUT",  ESpamChkState::ESCS_SameInput);
		
		def<VOID_FUN_IN_INT>(pkState, "CallMegaPhone", &lwChatMgrClient::CallMegaPhone);
	return true;
}

void lwChatMgrClient::CallMegaPhone(int const Type)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		PgInventory* pkInven = pkPlayer->GetInven();
		if( pkInven )
		{
			bool bHaveItem = false;
			tagSMSInfo kInfo;

			ContHaveItemNoCount kCont;
			if( S_OK == pkInven->GetItems(UICT_SMS, kCont) )
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);

				ContHaveItemNoCount::iterator item_itor = kCont.begin();
				while( item_itor != kCont.end() )
				{
					CItemDef const* pkDef = kItemDefMgr.GetDef( item_itor->first );
					if( pkDef )
					{
						kInfo.iValue = pkDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
						kInfo.iValue2 = pkDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);

						if( 0 == kInfo.iValue2 && Type == kInfo.iValue )
						{
							SItemPos kItemPos;
							if( S_OK == pkInven->GetFirstItem( item_itor->first, kItemPos) )
							{
								PgBase_Item kItem;
								if( S_OK == pkInven->GetItem(kItemPos, kItem) )
								{
									bHaveItem = true;

									kInfo.iItemNo = kItem.ItemNo();
									kInfo.kItemGuid = kItem.Guid();
									kInfo.kItemPos = kItemPos;
								}
							}
						}
					}
					++item_itor;
				}
			}

			if( false == bHaveItem )
			{
				lua_tinker::call<void, int, int>("OnCallStaticCashItemBuy", 10, Type);
			}
			else
			{
				CallSMS(kInfo);
			}
		}
	}
}

void lwChatMgrClient::ChatAram(lwWString Text, bool bShowNow)
{
	if( bShowNow)
	{
		bool bSend = false;
		DWORD dwNowTime = BM::GetTime32();
		if(InputNow_TimeMin() < dwNowTime - InputNow_Time())//지정한 시간 이상 경과 했을 경우에만
		{
			bSend = true;
		}
		if(bSend)
		{
			InputNow_Time(dwNowTime);
			InputNow(bShowNow);
			m_pkChatMgrClient->ChatAram(Text.GetWString(), bShowNow);
		}
	}
	else
	{
		m_pkChatMgrClient->ChatAram(Text.GetWString(), bShowNow);
	}
}

void lwChatMgrClient::SetChatStation(int const iType)
{
	m_pkChatMgrClient->ChatStation((EChatStation)iType);
}

bool lwChatMgrClient::OnFocus_ChatEdit()
{
	if( !m_pkChatMgrClient->CheckChatOut() )
	{
		lwUIWnd UIChatBar = lwActivateUI("ChatBar");
		if( !UIChatBar.IsNil() )
		{
			lwUIWnd UIChatEdit = UIChatBar.GetControl("EDT_CHAT");
			if( !UIChatEdit.IsNil() )
			{
				if( !UIChatEdit.IsFocus() )
				{
					lwClearEditFocus();
					UIChatEdit.SetEditFocus(false);
					lua_tinker::call<void>("OnDelTimer_HideChatBar");
					return true;
				}
			}
		}
	}
	else
	{
		return m_pkChatMgrClient->SetChatOutFocus();
	}

	return false;
}

lwWString lwChatMgrClient::Name2Guid_Find_ByGuid(lwGUID Guid)
{
	std::wstring	Name;
	if(m_pkChatMgrClient->Name2Guid_Find_ByGuid(Guid(), Name))
	{
		return lwWString(Name);
	}
	return lwWString("");
}
bool lwChatMgrClient::CheckChatCommand(lwWString Text)
{
	return m_pkChatMgrClient->CheckChatCommand(Text());
}

void lwChatMgrClient::AddEventMessage(int iEventMessageNo, bool bNotice, int iLevel, int iChatType)
{
	SChatLog kChatLog((EChatType)iChatType);//CT_EVENT
	m_pkChatMgrClient->AddMessage(iEventMessageNo, kChatLog, bNotice, iLevel);
}
void lwChatMgrClient::AddLogMessage(lwWString kContents, bool bNotice, int iLevel, int iChatType)
{
	SChatLog kChatLog((EChatType)iChatType);//CT_EVENT
	m_pkChatMgrClient->AddLogMessage(kChatLog, kContents(), bNotice, iLevel);
}
unsigned int lwChatMgrClient::GetFontColor()
{
	return m_pkChatMgrClient->FontColor();
}
void lwChatMgrClient::SetFontColor(unsigned int iColor)
{
//	m_pkChatMgrClient->FontColor(iColor);
//	m_pkChatMgrClient->UseColor(true);
	m_pkChatMgrClient->UseFontColor(iColor);
}

void lwChatMgrClient::SetSpamChkFlag(int const iChkFlag)
{
	if(ESCS_MAX_FLAG < iChkFlag
		&& 0 >= iChkFlag)
	{
		return;
	}
	m_pkChatMgrClient->SpamChkFlag(iChkFlag);
}

void lwChatMgrClient::SetMaxInputLog(int const iMaxInputLog)
{
	m_pkChatMgrClient->MaxInputLog(iMaxInputLog);
}

void lwChatMgrClient::SetMaxBlockTime(float const fMaxBlockTime)
{
	m_pkChatMgrClient->SetMaxBlockTime(fMaxBlockTime);
}

void lwChatMgrClient::SetBlockSpamChat_ChainInput(float const fWatchT, float const fBlockTime)
{
	m_pkChatMgrClient->SetBlockSpamChat_ChainInput(fWatchT, fBlockTime);
}

void lwChatMgrClient::SetBlockSpamChat_LooseChainInput(float const fWatchT, int const iCnt, float const fBlockTime)
{
	m_pkChatMgrClient->SetBlockSpamChat_LooseChainInput(fWatchT, iCnt, fBlockTime);
}

void lwChatMgrClient::SetBlockSpamChat_MacroInput(int const iCnt, float const fRangeTime, float const fBlockTime)
{
	m_pkChatMgrClient->SetBlockSpamChat_MacroInput(iCnt, fRangeTime, fBlockTime);
}

void lwChatMgrClient::SetBlockSpamChat_SameInput(float const fWatchT, int iCnt, float const fBlockTime)
{
	m_pkChatMgrClient->SetBlockSpamChat_SameInput(fWatchT, iCnt, fBlockTime);
}

bool lwChatMgrClient::CheckChatOut()
{
	return m_pkChatMgrClient->CheckChatOut();
}

//모든 채팅의 기본
bool lwChatMgrClient::SendChatW(lwWString lwkChat, bool const bCheckSpamChat)
{
	return m_pkChatMgrClient->SendChat_Check(lwkChat(), bCheckSpamChat);
}
bool lwChatMgrClient::SendChatStyleString(lwStyleString lwkChat, bool const bCheckSpamChat)//기본 채팅(나머지는 임의적으로 필요할때만 사용)
{
	return m_pkChatMgrClient->SendChat_CheckSS(lwkChat(), bCheckSpamChat);
}

//이모션
bool lwChatMgrClient::SendChat_InputNow(bool bPopup)
{
	bool bSend = false;
	DWORD dwNowTime = BM::GetTime32();
	if(InputNow_TimeMin() < dwNowTime - InputNow_Time())//지정한 시간 이상 경과 했을 경우에만
	{
		bSend = true;
	}

	if(InputNow() != bPopup)//이전과 다르거나
	{
		bSend = true;
	}
	else
	{
		bSend = false;
	}

	if(bSend)
	{
		InputNow_Time(dwNowTime);
		InputNow(bPopup);
		return m_pkChatMgrClient->SendChat_InputNow(bPopup);
	}
	return false;
}

//스스로 채팅중 표기 띄우기
bool lwChatMgrClient::Self_InputNow(bool bPopup, int iChatMode)
{	
	bool bSend = false;
	DWORD dwNowTime = BM::GetTime32();
	if(InputNow_TimeMin() < dwNowTime - InputNow_Time())//지정한 시간 이상 경과 했을 경우에만
	{
		bSend = true;
	}

	if(InputNow() != bPopup)//이전과 다르거나
	{
		bSend = true;
	}
	else
	{
		bSend = false;
	}

	if(bSend)
	{
		InputNow_Time(dwNowTime);
		InputNow(bPopup);
		return m_pkChatMgrClient->Self_InputNow(bPopup, iChatMode);
	}
	return false;
}

void lwChatMgrClient::ClearChatBalloon()
{
	m_pkChatMgrClient->ClearChatBalloon();
}

//
void lwChatMgrClient::InputNow_TimeMinimum(int nInterval)
{
	InputNow_TimeMin((DWORD)nInterval);
}

//Recv
bool lwChatMgrClient::RecvChat(lwPacket &rkPacket)
{
	return m_pkChatMgrClient->RecvChat(rkPacket());
}
bool lwChatMgrClient::RecvChat_InputNow(lwPacket &rkPacket)
{
	return m_pkChatMgrClient->RecvChat_InputNow(rkPacket());
}


bool lwChatMgrClient::Character_ChatBaloon_Set(lwGUID lwkGuid, lwWString lwkChat, int iChatType)
{
	m_pkChatMgrClient->Character_ChatBallon_Set(lwkGuid(), lwkChat(), iChatType);
	return true;
}

//채팅모드
int lwChatMgrClient::ChatMode_Get()
{
	return m_pkChatMgrClient->ChatMode();
}
int lwChatMgrClient::ChatMode_Set(int iChatType)
{
	m_pkChatMgrClient->ChatMode(iChatType);
	return m_pkChatMgrClient->ChatMode();
}

//
/*
void lwChatMgrClient::Set_IsGodCommand()
{
	m_pkChatMgrClient->GodCommand(true);
}
*/


//
void lwChatMgrClient::Notice_Show(lwWString kString, int const iLevel)
{
	::Notice_Show( kString(), iLevel);
}

void lwChatMgrClient::Notice_Show_ByDefStringNo(int const iDefStringNo)
{
	::Notice_Show_ByDefStringNo(iDefStringNo, EL_Warning);
}

void lwChatMgrClient::Notice_Show_ByTextTableNo(int const iTextTableNo)
{
	::Notice_Show_ByTextTableNo(iTextTableNo, EL_Warning);
}

void lwChatMgrClient::Notice_Show_ByTextTableNo_CheckEqualMessage(int const iTextTableNo)
{
	::Notice_Show_ByTextTableNo(iTextTableNo, EL_Warning, true);
}

void lwChatMgrClient::ChatFilter_ModeClear()//채팅모드로 필터링 초기화
{
	PgChatMgrUtil::ChatFilterClear();
}

void lwChatMgrClient::ChatFilter_ModeAdd(int iFilterSet, int iChatMode, char const *szXuiListID)//채팅모드로 필더링 추가
{
	int const iByteFilter = 0xFF;
#ifdef _DEBUG//목적 데이터형 초과 검사
	if(iByteFilter<iChatMode || (iByteFilter>iChatMode)&&(0>iChatMode))
	{
		NILOG(PGLOG_ERROR, "Logical Error: int variable is great then target byte"); assert(0);
	}
#endif
	BYTE cChatMode = (BYTE)(iChatMode&iByteFilter);
	std::wstring kID = UNI(szXuiListID);
	PgChatMgrUtil::ChatFilterAdd(iFilterSet, cChatMode, kID);
}

void lwChatMgrClient::ChatFilter_ModeDel(int iFilterSet, int iChatMode, char const *szXuiListID)//채팅모드로 필더링 삭제
{
	int const iByteFilter = 0xFF;
#ifdef _DEBUG//목적 데이터형 초과 검사
	if(iByteFilter<iChatMode || (iByteFilter>iChatMode)&&(0>iChatMode))
	{
		NILOG(PGLOG_ERROR, "Logical Error: int variable is great then target byte"); assert(0);
	}
#endif
	BYTE cChatMode = (BYTE)(iChatMode&iByteFilter);
	std::wstring kID = UNI(szXuiListID);
	PgChatMgrUtil::ChatFilterDel(iFilterSet, cChatMode, kID);
}

void lwChatMgrClient::NoticeLevelColor_Add(int const iLevel, const unsigned int dwFontColor, const unsigned int dwOutlineColor, bool const bShowBG)//경고 레벨 컬러를 추가한다.
{
	PgChatMgrUtil::NoticeLevelColor_Add(iLevel, dwFontColor, dwOutlineColor, bShowBG);
}
void lwChatMgrClient::NoticeLevelColor_Clear()//경고 레벨 컬러를 초기화 한다.
{
	PgChatMgrUtil::NoticeLevelColor_Clear();
}
void lwChatMgrClient::ClearNotice()
{
	m_pkChatMgrClient->ClearNotice();
}
bool lwChatMgrClient::SetWhisperDlg(lwUIWnd Wnd)
{
	return m_pkChatMgrClient->SetWhisperDlg(Wnd.GetSelf());
}

lwWString lwChatMgrClient::SetWhisperByTab(void)
{
	return lwWString(m_pkChatMgrClient->SetWhisperByTab());
}

lwWString lwChatMgrClient::ConvertEmoticonCommandToHex(lwWString Str)
{
	return lwWString(m_pkChatMgrClient->ConvertUserCommand(Str()));
}

void lwChatMgrClient::SetChatOutDefaultPos(lwUIWnd Wnd)
{
	if(false==Wnd.IsNil() && 0==strcmp("CHATOUT", Wnd.GetID().GetStr()))
	{
		m_pkChatMgrClient->SetChatOutDefaultPos(Wnd.GetLocation()());
	}
}

void lwChatMgrClient::SetSysChatOutDefaultPos(lwUIWnd Wnd)
{
	if( false == Wnd.IsNil() && 0 == strcmp("SYSCHATOUT", Wnd.GetID().GetStr()) )
	{
		m_pkChatMgrClient->SetSysChatOutDefaultPos(Wnd.GetLocation()());
	}
}

bool lwChatMgrClient::ToggleConsecutiveChat(void)
{
	return m_pkChatMgrClient->TogglingConsecutiveChat();
}

bool lwChatMgrClient::GetToggleConsecutiveChat(void)
{
	return m_pkChatMgrClient->ToggleConsecutiveChat();
}

void lwChatMgrClient::ToggleConsecutiveChatUI(void)
{
	m_pkChatMgrClient->ToggleConsecutiveChatUI();
}

void lwChatMgrClient::ShowNotifyConnectInfoUI(void)
{
	m_pkChatMgrClient->ShowNotifyConnectInfoUI();
}

void lwChatMgrClient::PopNotifyList(void)
{
	m_pkChatMgrClient->PopNotifyList();
}

void lwChatMgrClient::SetExistWhisperList(bool bSet)
{
	m_pkChatMgrClient->ExistWhisperList(bSet);
}

bool lwChatMgrClient::GetExistWhisperList(void)
{
	return m_pkChatMgrClient->ExistWhisperList();
}

void lwChatMgrClient::SetSysChatOutSnap(bool bSnap)
{
	m_pkChatMgrClient->SysChatOutSnap(bSnap);
}

bool lwChatMgrClient::GetSysChatOutSnap(void)
{
	return m_pkChatMgrClient->SysChatOutSnap();
}

void lwChatMgrClient::CheckSnapPos(lwPoint2 const& rkPos)
{
	m_pkChatMgrClient->CheckSnapPos(POINT2(rkPos.GetX(), rkPos.GetY()));
}

void lwChatMgrClient::SetSysOutHide(bool bHide)
{
	m_pkChatMgrClient->SysOutHide(bHide);
}

bool lwChatMgrClient::GetSysOutHide(void)
{
	return m_pkChatMgrClient->SysOutHide();
}

lwGUID lwChatMgrClient::GetNotifyGuid(void)
{
	return m_pkChatMgrClient->GetNotifyGuid();
}

lwGUID lwChatMgrClient::Name2Guid_Find_ByName(lwWString kName)
{
	BM::GUID OutGuid;
	m_pkChatMgrClient->Name2Guid_Find_ByName(kName(), OutGuid);

	return OutGuid;
}

bool lwChatMgrClient::CheckChatTag(lwWString Text)
{
	return m_pkChatMgrClient->CheckChatTag(Text.GetWString());
}

void lwChatMgrClient::Regist_ChatBlockUser(lwWString & Name)
{
	m_pkChatMgrClient->Regist_ChatBlockUser(Name());
}

void lwChatMgrClient::UnRegist_ChatBlockUser(lwWString & Name)
{
	m_pkChatMgrClient->UnRegist_ChatBlockUser(Name());
}

void lwChatMgrClient::Modify_ChatBlockMode(lwWString & Name, BYTE BlockMode)
{
	m_pkChatMgrClient->Modify_ChatBlockMode(Name(), BlockMode);
}

void lwChatMgrClient::UpdateChatBlockList(lwUIWnd BlockWnd)
{
	m_pkChatMgrClient->UpdateChatBlockList(BlockWnd());
}

//void lwChatMgrClient::Chat_Refresh()
//{
//	m_pkChatMgrClient->RefreshLog(true);//추가 안됬는데 추가 됬다 하는것만으로도 Refresh 효과가 있다.
//}
