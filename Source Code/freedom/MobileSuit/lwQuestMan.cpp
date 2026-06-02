#include "stdafx.h"
#include "lwGUID.h"
#include "Variant/PgQuestInfo.h"
#include "PgQuest.h"
#include "PgQuestUI.h"
#include "lwWString.h"
#include "PgQuestMan.h"
#include "lwQuestMan.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgActorNpc.h"
#include "PgWorld.h"
#include "Pg2DString.h"
#include "PgNetwork.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgChatMgrClient.h"
#include "lwUIQuest.h"

namespace NpcTalkHelper
{
	bool IsMapNo(int const iMapNo)
	{
		CONT_DEFMAP const* pkDefMap = NULL;
		g_kTblDataMgr.GetContDef(pkDefMap);
		return pkDefMap->end() != pkDefMap->find(iMapNo);
	}
	void SendReqNpcTalkMapNove(lwGUID kNpcGuid, int const iTargetGroundNo)
	{
		BM::Stream kPacket(PT_C_M_REQ_NPC_TALK_MAP_MOVE);
		kPacket.Push( kNpcGuid() );
		kPacket.Push( iTargetGroundNo );
		NETWORK_SEND(kPacket);
	}
};

	//
void Test_WriteQuestItemInfo()
{
#ifndef USE_INB
	CONT_DEF_QUEST_REWARD const* pkDefQuest = NULL;
	g_kTblDataMgr.GetContDef(pkDefQuest);
	CONT_ITEMNO kContItemNo;

	CONT_DEF_QUEST_REWARD::const_iterator loop_iter = pkDefQuest->begin();
	while( pkDefQuest->end() != loop_iter )
	{
		CONT_DEF_QUEST_REWARD::mapped_type const& rkDefQuest = (*loop_iter).second;
		if( PgQuestInfoUtil::IsLoadXmlType(rkDefQuest.iDBQuestType) )
		{
			PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(rkDefQuest.iQuestID);
			if( pkQuestInfo )
			{
				ContQuestDependItem const& rkContDependItem = pkQuestInfo->m_kDepend_Item;
				ContQuestDependItem::const_iterator iter = rkContDependItem.begin();
				while( rkContDependItem.end() != iter )
				{
					ContQuestDependItem::mapped_type const& rkDependItem = (*iter).second;
					if( QET_ITEM_ChangeCount == rkDependItem.iType )
					{
						std::back_inserter(kContItemNo) = rkDependItem.iItemNo;
					}
					++iter;
				}
			}
		}
		++loop_iter;
	}

	std::fstream kFile(".\\QuestDependItem.txt", std::ios_base::out| std::ios_base::trunc);
	CONT_ITEMNO::const_iterator item_iter = kContItemNo.begin();
	while( kContItemNo.end() != item_iter )
	{
		kFile << MB(BM::vstring((*item_iter))) << "\n";
		++item_iter;
	}
	kFile.close();
#endif
}

lwQuestMan GetQuestMan()
{
	return lwQuestMan(&g_kQuestMan);
}

bool lwIsIngQuest(unsigned short sQuestID)
{
	PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
	if(pkPC)
	{
		PgMyQuest const *pkMyQuest = pkPC->GetMyQuest();
		if(pkMyQuest)
		{
			bool const bIngQuestRet = pkMyQuest->IsIngQuest(sQuestID);
			return bIngQuestRet;
		}
	}
	return false;
}

bool lwIsEndQuest(unsigned short sQuestID)
{
	bool const bIsIngRet = lwIsIngQuest(sQuestID);
	if( !bIsIngRet )
	{
		return false;
	}

	PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return false;
	}

	PgMyQuest const *pkMyQuest = pkPC->GetMyQuest();
	if( !pkMyQuest )
	{
		return false;
	}

	SUserQuestState const *pkState = pkMyQuest->Get(sQuestID);
	if( pkState )
	{
		bool const bIsEndRet = (QS_End == pkState->byQuestState);
		return bIsEndRet;
	}
	return false;
}

extern bool lwIsEndedQuest(unsigned int sQuestID)
{
	PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
	if(pkPC)
	{
		PgMyQuest const *pkMyQuest = pkPC->GetMyQuest();
		if(pkMyQuest)
		{
			bool const bEndedQuestRet = pkMyQuest->IsEndedQuest(sQuestID);
			return bEndedQuestRet;
		}
	}
	return false;
}

#ifndef EXTERNAL_RELEASE
void ReloadQuestXML(int iQuestID, int iEndQuestID)
{
	if( !iQuestID )
	{
		return;
	}

	if( iQuestID
	&&	!iEndQuestID )
	{
		iEndQuestID = iQuestID;
	}

	CONT_DEF_QUEST_REWARD const* pkQuestReward = NULL;
	g_kTblDataMgr.GetContDef(pkQuestReward);

	for( int iCur=iQuestID; iEndQuestID >= iCur; ++iCur )
	{
		CONT_DEF_QUEST_REWARD::const_iterator iter = pkQuestReward->find(iCur);
		if( pkQuestReward->end() != iter )
		{
			CONT_DEF_QUEST_REWARD::mapped_type const& rkDefQuest = (*iter).second;
			g_kQuestMan.RemoveQuest(rkDefQuest.iQuestID);
			// 캐쉬에서 삭제
			std::string const kXmlPath( MB(rkDefQuest.kXmlPath) );
			PgXmlLoader::ReleaseXmlDocumentInCacheByPath( kXmlPath.c_str() );

			// 다시 로드
			PgXmlLoader::PrepareXmlDocument( kXmlPath.c_str(), PXDAW_ParseQuest );
		}
	}
}
#endif

void NET_C_M_REQ_TRIGGER(int const iObjectType, lwGUID kGuid, int const iActionType)
{
	switch( iObjectType )
	{
	case QOT_NPC:
	case QOT_Trigger:
	case QOT_Item:
	case QOT_Time:
	case QOT_ShineStone:
	case QOT_Location:
	case QOT_Dialog:
		{
			if( !lwCanQuestTalk(kGuid) )
			{
				return;
			}
		}break;
	case QOT_Monster:
	case QOT_Trap:
		{
		}break;
	case QOT_None:
	default:
		{
			_PgMessageBox("Error", "Wrong object type: %d", iObjectType);
			return;
		}break;
	}
	
	BM::Stream kPacket(PT_C_M_REQ_TRIGGER);
	kPacket.Push( iObjectType );
	kPacket.Push( kGuid() );
	kPacket.Push( iActionType );
	NETWORK_SEND(kPacket)
}

bool lwCanQuestTalk(lwGUID kNpcGuid)
{
	size_t const iCountRet = g_kQuestMan.CanNPCQuestInfo(kNpcGuid());
	if( !iCountRet )
	{
		return false;
	}
	return true;
}

void lwSendReqCloseDialog()
{
	int const iQuestID = g_kQuestMan.NowQuestID();
	int const iDialogID = g_kQuestMan.NowDialogID();
	EQuestDialogType const eType = g_kQuestMan.NowInfo().eType;

	bool bSendClose = false;
	switch( eType )
	{
	case QDT_VillageBoard:
		{
			bSendClose = true;
		}break;
	default:
		{
			bSendClose = QDT_None < eType && QDT_Wanted_Complete >= eType;
		}break;
	}
	
	if( bSendClose )
	{
		g_kQuestMan.Send_ReqDialogClose(iQuestID, iDialogID);
	}
	else
	{
		PgQuestManUtil::CloseAllQuestTalkUI();
	}
}

bool lwQuestMan::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

#ifndef EXTERNAL_RELEASE
	def(pkState, "ReloadQuestXML", ReloadQuestXML);
#endif

	def(pkState, "GetQuestMan", &GetQuestMan);

	class_<lwQuestMan>(pkState, "QuestMan")
		.def(pkState, constructor<PgQuestMan*>())
		//.def(pkState, "SetRecentQObject", &lwQuestMan::SetRecentQObject)
		.def(pkState, "ClearNPCQuestInfo", &lwQuestMan::ClearNPCQuestInfo)
		.def(pkState, "NextQuestDialog", &lwQuestMan::NextQuestDialog)
		.def(pkState, "ClearRecentQuestInfo", &lwQuestMan::ClearRecentQuestInfo)
		.def(pkState, "GetMiniQuestState", &lwQuestMan::GetMiniQuestState)
		.def(pkState, "SetMiniQuestState", &lwQuestMan::SetMiniQuestState)
		.def(pkState, "UpdateQuestOut", &lwQuestMan::UpdateQuestOut)
		.def(pkState, "CallNpcTalk", &lwQuestMan::CallNpcTalk)
		.def(pkState, "PrevLetter", &lwQuestMan::PrevLetter)
		.def(pkState, "NextLetter", &lwQuestMan::NextLetter)
		.def(pkState, "CallNpcWarning", &lwQuestMan::CallNpcWarning)
		//.def(pkState, "CallFullScreenTalk", &lwQuestMan::CallFullScreenTalk)
		.def(pkState, "CallEventFullScreenTalk", &lwQuestMan::CallEventFullScreenTalk)
		.def(pkState, "IsFullQuestDialog", &lwQuestMan::IsFullQuestDialog)
		.def(pkState, "IsEventScriptDialog", &lwQuestMan::IsEventScriptDialog)
		.def(pkState, "Load", &lwQuestMan::Load)
		.def(pkState, "SendDialogTrigger", &lwQuestMan::SendDialogTrigger)
		.def(pkState, "ResumeNextQuestTalk", &lwQuestMan::ResumeNextQuestTalk)
		.def(pkState, "IsMenuDialog", &lwQuestMan::IsMenuDialog)
		.def(pkState, "RemoteAcceptQuest", &lwQuestMan::RemoteAcceptQuest);
		;

	def(pkState, "CanQuestTalk", lwCanQuestTalk);
	def(pkState, "IsIngQuest", lwIsIngQuest);
	def(pkState, "IsEndQuest", lwIsEndQuest);
	def(pkState, "IsEndedQuest", lwIsEndedQuest);
	def(pkState, "CallGuildNpcFunctionUI", CallGuildNpcFunctionUI);
	def(pkState, "CallNpcFunctionUI", &CallNpcFunctionUI);
	def(pkState, "CallNpcFunctionUI2", &CallNpcFunctionUI2);
	def(pkState, "CallQuestMiniIngToolTip", CallQuestMiniIngToolTip);
	def(pkState, "CallLevelUpMsg", CallLevelUpMsg);
	def(pkState, "NET_C_M_REQ_TRIGGER", NET_C_M_REQ_TRIGGER);
	def(pkState, "SendReqCloseDialog", lwSendReqCloseDialog);

	def(pkState, "LockPlayerInput", PgQuestManUtil::LockPlayerInput);
	def(pkState, "UnLockPlayerInput", PgQuestManUtil::UnLockPlayerInput);
	def(pkState, "LockPlayerInputMove", PgQuestManUtil::LockPlayerInputMove);
	def(pkState, "UnLockPlayerInputMove", PgQuestManUtil::UnLockPlayerInputMove);

	def(pkState, "IsMapNo", NpcTalkHelper::IsMapNo);
	def(pkState, "SendReqNpcTalkMapNove", NpcTalkHelper::SendReqNpcTalkMapNove);

	def(pkState, "Test_WriteQuestItemInfo", Test_WriteQuestItemInfo);
	return true;
}

lwQuestMan::lwQuestMan(PgQuestMan *pQuestMan)
{
	m_pQuestMan = pQuestMan;
}

void lwQuestMan::SendDialogTrigger(lwGUID kGuid, int const iQuestID, int const iDialogID)
{
	PgActor* pkActor = g_kPilotMan.GetPlayerActor();
	if( pkActor )
	{
		NiPoint3 const& rkCurPos = pkActor->GetPos();
		POINT3 const kSyncPos(rkCurPos.x, rkCurPos.y, rkCurPos.z);

		BM::Stream kPacket(PT_C_M_REQ_TRIGGER);
		kPacket.Push( QOT_Dialog);
		kPacket.Push( kGuid() );
		kPacket.Push( iQuestID );
		kPacket.Push( iDialogID );
		kPacket.Push( kSyncPos );
		NETWORK_SEND(kPacket)
	}
}

bool	lwQuestMan::IsFullQuestDialog()
{
	return	g_kQuestMan.IsQuestDialog();
}

bool	lwQuestMan::IsEventScriptDialog()
{
	return g_kQuestMan.EventScriptDialog();
}

void	lwQuestMan::CallEventFullScreenTalk(lwWString kObjectName,lwWString kTitleText,lwWString kDialogText,lwWString kFaceID,char const *szStitchImageID)
{
	SFullScreenDialogInfo	kInfo(QDT_EventScriptTalk);
	kInfo.kGuid = BM::GUID::NullData();
	kInfo.kObjectName = kObjectName();
	kInfo.kTitleText = kTitleText();
	kInfo.kDialogText = kDialogText();
	kInfo.kFaceID = kFaceID();
	kInfo.kStitchImageID = (szStitchImageID)? UNI(szStitchImageID): std::wstring();

	g_kQuestMan.CallFullScreenTalk(kInfo);
	g_kQuestMan.EventScriptDialog(true);
}

void lwQuestMan::ClearNPCQuestInfo()
{
	g_kQuestMan.ClearNPCQuestInfo();
}

void lwQuestMan::ClearRecentQuestInfo()
{
	g_kQuestMan.ClearRecentQuestInfo();
}

bool lwQuestMan::NextQuestDialog(lwUIWnd kTop)
{
	switch( g_kQuestMan.NowInfo().eType )
	{
	case QDT_Wanted:
	case QDT_VillageBoard:
		{
		}break;
	case QDT_Prolog:
	case QDT_Normal:
	case QDT_Complete:
	case QDT_Movie:
		{
			int const& iQuestID = g_kQuestMan.RecentQuestID();
			int const& iDialogID = g_kQuestMan.RecentDialogID();

			PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
			if( !pkQuestInfo )
			{
				g_kQuestMan.Send_ReqDialogClose(iQuestID, iDialogID); // 서버로 닫기 요청 패킷 전송
				return false;
			}

			SQuestDialog const* pkCurQuestDialog = NULL;
			bool bFindDialog = pkQuestInfo->GetDialog(iDialogID, pkCurQuestDialog);
			if( !bFindDialog )
			{
				g_kQuestMan.Send_ReqDialogClose(iQuestID, iDialogID); // 서버로 닫기 요청 패킷 전송
				return false;
			}

			if( !kTop.IsNil() )
			{
				XUI::CXUI_Wnd* pDialogMainWnd = kTop.GetSelf()->GetControl(_T("FRM_MAIN"));
				if (pDialogMainWnd)
				{
					XUI::CXUI_Wnd* pQuestTextBgWnd = pDialogMainWnd->GetControl(_T("FRM_TEXT_BG"));
					if (pQuestTextBgWnd)
					{
						XUI::CXUI_Wnd* pQuestTextWnd = pQuestTextBgWnd->GetControl(_T("FRM_TEXT"));//사용 대사
						if (pQuestTextWnd)
						{

							Pg2DString	*pk2DString = (Pg2DString*)pQuestTextWnd->m_pText;
							if (pk2DString)
							{
								pk2DString->IsDrawingDone( !pk2DString->IsDrawingDone() );
							}
						}
					}
				}
			}

			//Dialog Text Seperate Step
			if( g_kQuestMan.NowDialogStep() < g_kQuestMan.NowDialogMaxStep() ) //아직 스텝이 남아 있으면
			{
				g_kQuestMan.CallQuestDialog(g_kQuestMan.NowInfo().kGuid, iQuestID, iDialogID);
				return false; // 닫지 마라
			}

			//Auto Next Dialog
			size_t const iSelectCount = pkCurQuestDialog->kSelectList.size();
			if( 0 == iSelectCount )
			{
				g_kQuestMan.Send_ReqDialogClose(iQuestID, iDialogID); // 서버로 닫기 요청 패킷 전송
				return false;
			}

			SQuestSelect const& rkQuestSelect = (*pkCurQuestDialog->kSelectList.begin());
			int const iNextDialogID = rkQuestSelect.iSelectID;
			bool const bFindNextDialog = pkQuestInfo->FindDialog(iNextDialogID);
			
			bool const bCanNext = g_kQuestMan.CanNextDialog(pkQuestInfo, pkCurQuestDialog, iNextDialogID);
			if( !bCanNext )
			{
				// 서버 전송 없다, 클라이언트 결정이 끝나지 않았다
				return false;//넘어가면 안됨/닫지 마라
			}

			bool bFuncNextID = false;
			if( 10000 <= iNextDialogID
			&&	39999 >=iNextDialogID )
			{
				bFuncNextID = true;//기능 다음 다이얼로그
			}

			if( !bFuncNextID ) // 수락/완료 아닐때 만 전송
			{
				g_kQuestMan.Send_Dialog_Select(iQuestID, iDialogID, iNextDialogID, g_kQuestMan.SelectItem1(), g_kQuestMan.SelectItem2()); // 서버로 패킷 전송
			}

			return false; // 항상 닫지 마라
		}break;
	case QDT_EventScriptTalk:
		{
			if( g_kQuestMan.IsLastStepDialog() )
			{
				g_kQuestMan.EventScriptDialog(false); // EventScript는 강제로 닫는 스크립트가 있다
			}
			else
			{
				g_kQuestMan.CallNextDialog();
			}
			return false; // 닫지 마 (닫는 스크립트가 따로 있다)
		}break;
	case QDT_Guild_Menu:
	case QDT_NpcTalk:
	default:
		{
			if( !g_kQuestMan.IsLastStepDialog() )
			{
				g_kQuestMan.CallNextDialog();
				return false;//닫지 마
			}

			if( g_kQuestMan.IsLastStepDialog()	//마지막이고
			&&	g_kQuestMan.NowSelectMenu()		//
			&&	!g_kQuestMan.CanNextSelect() )	//선택지가 없으면
			{
				return true;//닫아줘
			}

			if( g_kQuestMan.NowSelectMenu() )
			{
				return false;//닫지 마
			}
		}break;
	}

	return true;//닫아라
}

bool lwQuestMan::GetMiniQuestState()
{
	return g_kQuestMan.MiniQuestStatus();
}
void lwQuestMan::SetMiniQuestState(bool bVisible)
{
	g_kQuestMan.ShowMiniQuest(bVisible);
}

void lwQuestMan::UpdateQuestOut(lwUIWnd lwUI)
{
	if (false == lwUI.IsNil())
	{
		g_kQuestMan.UpdateQuestOut(lwUI.GetSelf());
	}
}

void lwQuestMan::CallNpcTalk(lwGUID kNpcGuid)
{
	if(!g_pkWorld)
	{
		return;
	}
	PgActorNpc* pkNpcActor = dynamic_cast<PgActorNpc*>( g_pkWorld->FindObject(kNpcGuid()) );
	if( pkNpcActor )
	{
		pkNpcActor->OnClickTalk();
	}
}

void lwQuestMan::CallNpcWarning(lwGUID kNpcGuid)
{
	if(!g_pkWorld)
	{
		return;
	}
	PgActorNpc* pkNpcActor = dynamic_cast<PgActorNpc*>( g_pkWorld->FindObject(kNpcGuid()) );
	if( pkNpcActor )
	{
		pkNpcActor->OnClickWarning();
	}
	
}

void lwQuestMan::NextLetter()
{
	g_kQuestMan.NextLetter();
}

void lwQuestMan::PrevLetter()
{
	g_kQuestMan.PrevLetter();
}

void lwQuestMan::Load()
{
	g_kQuestMan.LoadMiniQuest();
}

bool lwQuestMan::ResumeNextQuestTalk()
{
	SResumeNextQuestTalk const& rkResumeInfo = g_kQuestMan.ResumeNextQuestTalk();
	if( BM::GUID::IsNotNull(rkResumeInfo.kNpcGuid)
	&&	0 != rkResumeInfo.iQuestID )
	{
		Quest::lwOnQuestItemSelected_From_SelectiveQuestList(rkResumeInfo.iQuestID, rkResumeInfo.kNpcGuid);
		g_kQuestMan.ResumeNextQuestTalk(SResumeNextQuestTalk());
		return true;
	}
	return false;
}

bool lwQuestMan::IsMenuDialog()
{
	return g_kQuestMan.NowSelectMenu();
}

void lwQuestMan::RemoteAcceptQuest(lwUIWnd kSelf)
{
	PgPilot *pPilot = g_kPilotMan.GetPlayerPilot();
	if( NULL == pPilot )
	{
		return;
	}
	CUnit *pUnit = pPilot->GetUnit();
	if( NULL == pUnit )
	{
		return;
	}
	if( true == pUnit->IsDead() )
	{
		lwAddWarnDataTT(75009);
		return;
	}
	if( true == kSelf.IsNil() )
	{
		lwAddWarnDataTT(200106);
		return;
	}
	lwUIWnd kParent = kSelf.GetParent();
	if( true == kParent.IsNil() )
	{
		lwAddWarnDataTT(200106);
		return;
	}
	int const iQuestID = kParent.GetCustomData<int>();
	if( 0 == iQuestID )
	{
		lwAddWarnDataTT(200106);
		return;
	}
	PgQuestInfo const *pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
	if( NULL == pkQuestInfo )
	{
		return;
	}
	if( false == pkQuestInfo->IsCanRemoteAccept() )
	{
		return;
	}
	BM::GUID kNpcGuid;
	if( false == pkQuestInfo->GetFirstClientNpc(kNpcGuid) )
	{
		return;
	}
	Quest::lwOnQuestItemSelected_From_SelectiveQuestList(pkQuestInfo->ID(), kNpcGuid, true);
}

void CallGuildNpcFunctionUI(int const iDialogType, int const iTTW)
{
	CallNpcFunctionUI(iDialogType, g_kQuestMan.NowInfo().kGuid, "Elvis_normal", iTTW);
}

void CallNpcFunctionUI(int const iDialogType, lwGUID kNpcGuid, char const *szFaceID, int const iTTW)
{
	if( !iTTW )
	{
		XUIMgr.Close( PgQuestManUtil::kFullQuestWndName );
		return;
	}

	CallNpcFunctionUI2(iDialogType, kNpcGuid, szFaceID, TTW(iTTW));
}

void CallNpcFunctionUI2(int const iDialogType, lwGUID kNpcGuid, char const *szFaceID, lwWString kText)
{
	if( BM::GUID::IsNull(kNpcGuid()) )
	{
		_PgMessageBox("Error", "[%s] NpcGuid is NULL\nDialog[%d] Text[%s]", __FUNCTION__, iDialogType, MB(kText()));
		return;
	}

	PgPilot *pkPilot = g_kPilotMan.FindPilot(kNpcGuid());
	if( pkPilot )
	{
		SFullScreenDialogInfo kInfo((EQuestDialogType)iDialogType);
		kInfo.kGuid = kNpcGuid();
		kInfo.kDialogText = kText();
		kInfo.kFaceID = UNI(szFaceID);
		kInfo.kObjectName = pkPilot->GetName();
		g_kQuestMan.CallNewDialog(kInfo, NULL, NULL);
	}
}

void CallQuestMiniIngToolTip(lwUIWnd kTopWnd)
{
	CXUI_Wnd *pkTopWnd = kTopWnd();
	if( !pkTopWnd )
	{
		return;
	}
	
	int iQuestID = 0;
	if( !pkTopWnd->GetCustomData(&iQuestID, sizeof(iQuestID)) )
	{
		return;
	}

	UpdateQuestMiniIngToolTip(iQuestID, pkTopWnd);
}

void CallLevelUpMsg()
{
	if( !g_kQuestMan.TempLevelMsg().empty() )
	{
		SChatLog kChatLog(CT_EVENT_SYSTEM);
		g_kChatMgrClient.AddLogMessage(kChatLog, g_kQuestMan.TempLevelMsg(), true, EL_Notice4);
		g_kQuestMan.TempLevelMsg(std::wstring());
		g_kChatMgrClient.AddMessage(700027, kChatLog, true, EL_Level2);
	}
}