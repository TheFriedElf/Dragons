#include "stdafx.h"
#include "BM/PgFilterString.h"
#include "Variant/PgWaiter.h"
#include "lwGUID.h"
#include "ServerLib.h"
#include "lwUI.h"
#include "lwPartyUI.h"
#include "PgError.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "pgchatMgrClient.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
#include "PgPartyBalloon.h"
#include "PgTextBalloon.h"
#include "PgActor.h"
#include "PgNetwork.h"
#include "PgWorld.h"
#include "PgHelpSystem.h"
#include "PgOption.h"
#include "PgUIScene.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "HardCoreDungeon.h"
#include "PgMobileSuit.h"
#include "lwStyleString.h"
#include "Variant/Global.h"

typedef enum ePartyMessageSet
{
	PMS_Is2 =					700032, //님을
	PMS_Is =					700033, //님이
	PMS_CantArea =				700034,
	PMS_Invite =				700054, //파티 초청 메시지
	PMS_Invite_Fail =			700138, //파티 초청 실패시(오프라인 or 없는이름)
	PMS_Invite_Fail2 =			700056, //파티 상대가 이미 파티에 있음
	PMS_Invite_Fail3 =			402200, //하드코어 던젼 투표중이라 가입할 수 없습니다.
	PMS_Invite_Reject =			700057,	//파티 초청 거부시
	PMS_Leave =					700058, //파티를 탈퇴 하였습니다.
	PMS_Kickout =				700059, //파티에서 추방 당하였습니다.
	PMS_NewMaster =				700060, //께서 새로운 파티장으로 임명 되었습니다.
	PMS_Join_Success =			700061, //파티에 가입하였습니다.
	PMS_Join_Fail =				700062,	//파티 가입에 실패
	PMS_Join_Full =				700063,	//파티 가입에 실패, 파티원이 꽉찾다
	PMS_Join_HardCoreVote =		402201, //하드코어 던젼 입장 투표중인 파티라 가입 할 수 없습니다.
	PMS_NotMaster =				700064,	//마스터가 아니다
	PMS_DidntMe	=				700065,	//나를 초대 하거나/ 추방 할 수 없습니다.
	PMS_SameParty =				700066,
	PMS_FailChangeMaster =		700067,
	PMS_PartyCreate =			700068,
	PMS_PartyDestroy =			700069,
	PMS_InvitePop =				700070,
	PMS_FailReqJoin_Inviter =	700071, //파티 대기자라 다른 사람을 초대 할 수 업습니다.
	PMS_NotInviteMe =			700074,//나를 초대 하지 마
	PMS_DupPartyName =			700104,
	PMS_Join_Level_Fail =		700120, //레벨이 맞지 않아 파티 참가 안됨
	PMS_ChangeName_Success =	700121, //파티명이 변경 되었습니다.
	PMS_PartyInvitePop =		700124, //님이 파티를 신청했습니다. 수락 하시겠습니까?
	PMS_PartyInvite_Success =	700123, //파티 가입신청을 하였습니다.
	PMS_DupReqJoin =			700134, //이미 다른 파티에 가입 신청을 한 상태입니다.
	PMS_AnswerWait =			700141, // 파티 수락 대기중입니다.
	PMS_KickUserNoArea =		700143, //파티원이 추방할수 없는 지역에 있습니다.
	PMS_InviteArea =			700501, //상대방이 파티를 할 수 없는 지역에 있습니다.
	PMS_HardCoreVote =			402202, // 하드코어 던젼 투표중에는 사용할 수 없는 기능이다.
	PMS_Fail_ReqJoinMyParty =	700414, // 자신의 파티에는 가입 신청을 할 수 없습니다.
	PMS_Fail_Req_Join =			30,		// 파티 가입 요청 취소
	PMS_NeedItem_PartyBreakIn = 700417,	// 대상이 현재 입장아이템을 가지고있지 않습니다.
} EPartyMessageSet;

namespace PgClientPartyUtil
{
	void PartyResultMsg(int iResult)
	{
		static int const kMsgArray[] = {
			PMS_Join_Fail,//PRC_None
			PMS_Join_Success,//PRC_Success
			0,//PRC_Success_Create
			0,//PRC_Success_Master
			PMS_PartyDestroy,//PRC_Success_Destroy
			0,//PRC_Success_Waiter
			PMS_ChangeName_Success,
			PMS_PartyInvitePop,//PRC_Success_PartyInvitePop
			PMS_PartyInvite_Success,//PRC_Success_PartyInvite
			PMS_Invite_Fail,//PRC_Fail
			PMS_Invite_Fail2,//PRC_Fail_Waiter
			PMS_Invite_Fail2,//PRC_Fail_Party
			PMS_NotMaster,//PRC_Fail_NotMaster
			PMS_Join_Full,//PRC_Fail_MemberCount
			PMS_Invite_Reject,//PRC_Fail_Refuse
			PMS_Invite_Fail,//PRC_Fail_PVP
			PMS_DidntMe,//PRC_Fail_Me
			0,//PRC_Fail_NullMaster
			0,//PRC_Fail_NullUser
			PMS_SameParty,//PRC_Fail_Duplicate
			0,//PRC_Fail_NotFind
			0,//PRC_Fail_NoWaiter
			0,//PRC_Fail_Destroy
			0,//PRC_Fail_NoMaster
			PMS_SameParty,//PRC_Fail_MineMember
			PMS_FailReqJoin_Inviter,//PRC_Fail_NotMember
			PMS_NotInviteMe,//PRC_Fail_NotInviteMe
			PMS_DupReqJoin,//PMS_Fail_DupReqJoin
			PMS_FailChangeMaster,//PRC_Fail_ChangeMaster,
			PMS_DupPartyName,//PRC_Fail_Dup_Name
			0,//PRC_Fail_Max
			700105,//PRC_Fail_BadName
			PMS_Join_Level_Fail,//PRC_Fail_Level
			PMS_AnswerWait,//PRC_Fail_AnswerWait
			PMS_KickUserNoArea,//PRC_Fail_KickUserNoArea
			PMS_InviteArea, //PRC_Fail_Area
			PMS_Invite_Fail3,//PCR_Fail_HardCoreVote1
			PMS_Join_HardCoreVote,//PCR_Fail_HardCoreVote2
			PMS_HardCoreVote,//PCR_Fail_HardCoreVote3
			0,//PRC_Fail_WaitPlayer
			0,//PRC_Fail_NoHaveKeyItem
			PMS_Fail_Req_Join,//PRC_Fail_Req_Join
			PMS_NeedItem_PartyBreakIn, //PRC_NeedItem_PartyBreakIn
			0,//PRC_Max,	
			};

		if( PRC_None > iResult 
		&& PRC_Max <= iResult )
		{return;}

		int const iTTW = kMsgArray[iResult];
		if( !iTTW )
		{return;}

		if( PgClientExpeditionUtil::IsInExpedition() == false )
		{
			std::wstring kTemp = TTW(iTTW);
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(kChatLog, kTemp, true);
		}
	}

	void UpdatePartyWndAll()
	{
		RefreshPartyInfoWnd(lwUIWnd(NULL));
		RefreshPartyStateWnd();
		lwCloseToolTip(NULL, TTC_Community);

//		char const *pszText = "SFRM_PARTY_PEOPLE_FIND";

//		std::wstring const wstr = UNI(pszText);
//		XUI::CXUI_Wnd* pWnd = NULL;
//		if(XUIMgr.IsActivate(wstr, pWnd))
//		{
//			RefreshPartyPeopleFindWnd(lwUIWnd(NULL));	
//		}
		
//		Refresh_Part_People_FindWnd();
		g_kParty.FindPartyUserListViewRefresh(g_kParty.PartyFindPeopleListNowPage(), 0);
		lua_tinker::call<void, lwUIWnd>("SetPartyIcon", lwUIWnd(NULL));	//월드맵에서 커플 위치 업데이트
		lua_tinker::call<void>("ActivatePremiumUI");
	}

	void SetCharacterPartyGuid(BM::GUID const &rkCharGuid, BM::GUID const &rkPartyGuid)
	{
		PgPilot	*pPilot = g_kPilotMan.FindPilot(rkCharGuid);
		if( NULL == pPilot )
		{
			return;
		}

		PgActor *pkActor = dynamic_cast<PgActor *>(pPilot->GetWorldObject());
		if( NULL == pkActor )
		{
			return;
		}

		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pPilot->GetUnit());
		if( NULL == pkPlayer )
		{
			return;
		}

		if( BM::GUID::IsNotNull(pkPlayer->PartyGuid())
		&&	BM::GUID::IsNull(rkPartyGuid) )
		{
			g_kParty.RemovePartyName(pkPlayer->PartyGuid(), pkPlayer->GetID());
		}

		pkPlayer->PartyGuid(rkPartyGuid);
		pkActor->UpdateName();
	}

	void AttachPartyCircle(BM::GUID const &rkPilotGuid)
	{
		PgPilot *pkPilot = g_kPilotMan.FindPilot(rkPilotGuid);
		if(NULL != pkPilot)
		{
			PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			if(NULL != pkActor)
			{
				NiAVObject *pkParticle = g_kParticleMan.GetParticle("e_ef_colorshadow_party",PgParticle::O_SCALE, pkActor->GetEffectScale());
				if(!pkParticle)
				{
					return;
				}

				if(!pkActor->AttachTo(9191, "char_root", (NiAVObject *)pkParticle))
				{
					THREAD_DELETE_PARTICLE(pkParticle);
					return;
				}
			}
		}
	}

	void DetachPartyCircle(BM::GUID const &rkPilotGuid)
	{
		PgPilot *pkPilot = g_kPilotMan.FindPilot(rkPilotGuid);
		if( pkPilot )
		{
			PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			if( pkActor )
			{
				pkActor->DetachFrom(9191);
			}
		}
	}

	bool IsCanPartyArea( bool const bChkPublic )
	{
		if( g_pkWorld )
		{
			if ( g_pkWorld->IsHaveAttr(GATTR_FLAG_NOPARTY) || g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND) 
				|| g_pkWorld->GetAttr() == GATTR_INSTANCE // 파렐 던전 같은 던전이라면 파티 해체 불가
				)
			{
				return false;
			}

			if ( bChkPublic && g_pkWorld->IsHaveAttr(GATTR_FLAG_PUBLIC_CHANNEL) )
			{
				return false;
			}
		}
		return true;
	}

	bool IsCanInviteFriendPartyArea()
	{
		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return false;
		}

		CONT_DEFMAP const* pDefMap = NULL;
		g_kTblDataMgr.GetContDef(pDefMap);
		if( NULL == pDefMap )
		{
			return false;
		}
		
		SGroundKey const& GroundKey = pPlayer->GroundKey();
		CONT_DEFMAP::const_iterator iter = pDefMap->find(GroundKey.GroundNo());
		if( pDefMap->end() == iter )
		{
			return false;
		}

		if( false == iter->second.ContItemNo.empty() )
		{
			return false;
		}

		if( false == iter->second.ContCashItemNo.empty() )
		{
			return false;
		}

		return true;
	}

	bool IsCanPartyBreakIn(int const MasterMapNo, CONT_NEED_KEY_ITEM & KeyItem, CONT_NEED_KEY_ITEM & CashKeyItem)
	{
		CONT_DEFMAP const* pDefMap = NULL;
		g_kTblDataMgr.GetContDef(pDefMap);
		if( NULL == pDefMap )
		{
			return false;
		}
		
		CONT_DEFMAP::const_iterator iter = pDefMap->find(MasterMapNo);
		if( pDefMap->end() == iter )
		{
			return false;
		}

		KeyItem = iter->second.ContItemNo;
		CashKeyItem = iter->second.ContCashItemNo;

		if( false == iter->second.ContItemNo.empty() )
		{
			return false;
		}

		if( false == iter->second.ContCashItemNo.empty() )
		{
			return false;
		}

		return true;
	}
	
	bool IsHaveKeyItemPartyBreakIn(CONT_NEED_KEY_ITEM const& KeyItem, CONT_NEED_KEY_ITEM const& CashKeyItem, int & NeedItemNo)
	{
		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return false;
		}

		PgInventory * pInv = pPlayer->GetInven();
		if( NULL == pInv )
		{
			return false;
		}

		CONT_NEED_KEY_ITEM::const_iterator iter = KeyItem.begin();
		while( KeyItem.end() != iter )
		{
			if( pInv->GetInvTotalCount(*iter) > 0 )
			{
				NeedItemNo = *iter;
				return true;
			}
			++iter;
		}

		iter = CashKeyItem.begin();
		while( CashKeyItem.end() != iter )
		{
			if( pInv->GetInvTotalCount(*iter) > 0 )
			{
				NeedItemNo = *iter;
				return true;
			}
			++iter;
		}
		
		NeedItemNo = *KeyItem.begin();
		return false;
	}

	bool IsCanPvPArea()
	{
		if( g_pkWorld && g_pkWorld->IsHaveAttr(GKIND_DEFAULT) )
		{
			return true;
		}
		return false;
	}

	bool IsInParty()
	{
		PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkMyPlayer )
		{
			return false;
		}

		if( BM::GUID::NullData() == pkMyPlayer->PartyGuid() )
		{
			return false;//파티 없다
		}
		return true;//파티 있다
	}

	bool IsInPartyMemberGuid(BM::GUID const &rkCharGuid)
	{
		if( !IsInParty() )
		{
			return false;
		}
		return g_kParty.IsMember(rkCharGuid);
	}

	bool IsPartyMaster(BM::GUID const &rkCharGuid)
	{
		if( !IsInParty() )
		{
			return false;
		}

		bool const bIsPartyMaster = (g_kParty.MasterGuid() == rkCharGuid);
		return bIsPartyMaster;
	}

	bool IsOhterPartyMaster(BM::GUID const &rkCharGuid)
	{
		PgPilot* pkPilot = g_kPilotMan.FindPilot(rkCharGuid);
		if( !pkPilot )
		{
			return false;
		}

		PgPlayer* pkPC = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
		if( !pkPC )
		{
			return false;
		}

		BM::GUID const &rkPartyGuid = pkPC->PartyGuid();
		if( BM::GUID::NullData() == rkPartyGuid )
		{
			return false;
		}

		SClientPartyName kOtherParty;
		bool const bFindOtherParty = g_kParty.GetPartyName(rkPartyGuid, kOtherParty);
		if( !bFindOtherParty )
		{
			return false;
		}

		bool const bIsMaster = (kOtherParty.kMasterGuid == rkCharGuid);
		return bIsMaster;
	}

	void Send_Request_Leave_Party()
	{
		if(!g_pkWorld)
		{
			return;
		}
		if( !IsCanPartyArea() )//파티가 가능한 지역인가?
		{
			lwAddWarnDataTT(PMS_CantArea);
			return;
		}

		if( PgClientExpeditionUtil::IsInExpedition() )
		{ // 현재 원정대에 속해 있는가?
			return;
		}

		if (g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION))	// 미션일 경우
		{//미션맵은 떠나는것도 안되요
			lwAddWarnDataTT(400231);
			return;
		}

		BM::Stream kPacket(PT_C_N_REQ_LEAVE_PARTY, true);
		NETWORK_SEND(kPacket)
	}

	void Send_RequestChangeMaster(lwGUID NewMasterGuid)
	{
		PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
		if( !pkPC )
		{return;}

		if( !IsPartyMaster(pkPC->GetID()) )
		{
			PartyResultMsg(PRC_Fail_NotMaster);
			return;
		}

		BM::Stream kPacket(PT_C_N_REQ_PARTY_CHANGE_MASTER);
		kPacket.Push(NewMasterGuid());
		NETWORK_SEND(kPacket)
	}

	void SetPartyOption1(int const iOptionExp, int const iOptionItem, int const iOptionPublicTitle, int const iLevel, bool const bStatus)
	{
		g_kParty.TempPartyOption = g_kParty.Option();

		g_kParty.TempPartyOption.SetOptionExp((EPartyOptionExp)iOptionExp);
		g_kParty.TempPartyOption.SetOptionItem((EPartyOptionItem)iOptionItem);
		g_kParty.TempPartyOption.SetOptionPublicTitle((EPartyOptionPublicTitle)iOptionPublicTitle);
		g_kParty.TempPartyOption.SetOptionLevel(iLevel);
		g_kParty.TempPartyOption.SetOptionState(bStatus);
	}

	void SetPartyOption2(int const iAttribute, int const iContinent, int const iArea, int const MaxMember, lwWString kPartySubName)
	{
		g_kParty.TempPartyOption.SetOptionAttribute(iAttribute);
		g_kParty.TempPartyOption.SetOptionContinent(iContinent);
		g_kParty.TempPartyOption.SetOptionArea_NameNo(iArea);
		g_kParty.TempPartyOption.SetOptionMaxMember(MaxMember);
		g_kParty.TempPartyOption.PartySubName(kPartySubName());
	}

	void Send_RequestChangeOption(lwWString kNewName)
	{
		/*SPartyOption rkOption = g_kParty.Option();
		rkOption.SetOptionExp((EPartyOptionExp)iOptionExp);
		rkOption.SetOptionItem((EPartyOptionItem)iOptionItem);
		rkOption.SetOptionPublicTitle((EPartyOptionPublicTitle)iOptionPublicTitle);
		rkOption.SetOptionLevel(iLevel);
		rkOption.SetOptionAttribute(iAttribute);
		rkOption.SetOptionContinent(iContinent);
		rkOption.SetOptionArea_NameNo(iArea);*/

		SPartyOption rkOption = g_kParty.TempPartyOption;

		if( !IsInParty() )
		{return;}
		
		if( PgClientExpeditionUtil::IsInExpedition() )
		{ // 현재 원정대에 속해 있는가?
			return;
		}

		if( !IsCanPartyArea() )//파티가 가능한 지역인가?
		{
			lwAddWarnDataTT(PMS_CantArea);
			return;
		}

		PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
		if( !pkPC )
		{return;}

		if( !IsPartyMaster(pkPC->GetID()) )
		{
			PartyResultMsg(PRC_Fail_NotMaster);
			return;
		}

		if( kNewName.IsNil() )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(400434, kChatLog, true);
			}
			return;
		}

		/*if( kNewSubName.IsNil() )
		{
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddMessage(401328, kChatLog, true);
			return;
		}*/

		std::wstring kNewPartyName = kNewName();
		if( kNewPartyName.empty() )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(400434, kChatLog, true);
			}
			return;
		}
		else
		{
			if ( MAX_PARTYNAME_LEN < kNewPartyName.size() )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(700413, kChatLog, true);
				return;
			}
		}

		/*std::wstring const &rkNewPartySubName = kNewSubName();
		if( rkNewPartySubName.empty() )
		{
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddMessage(401328, kChatLog, true);
			return;
		}*/

		if(!g_kUnicodeFilter.IsCorrect(UFFC_PARTY_NAME, kNewPartyName))
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(700407, kChatLog, true);
			}
			return;
		}

		/*if(!g_kUnicodeFilter.IsCorrect(UFFC_PARTY_NAME, rkNewPartySubName))
		{
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddMessage(700407, kChatLog, true);
			return;
		}*/

		kNewPartyName = kNewPartyName.substr(0,MAX_PARTYNAME_LEN);
		BM::Stream kPacket(PT_C_N_REQ_PARTY_CHANGE_OPTION);
		kPacket.Push(kNewPartyName);
		kPacket.Push(false);//Clear Name
		rkOption.WriteToPacket(kPacket);
		NETWORK_SEND(kPacket)
	}


	void Net_C_N_REQ_KICKOUT_PARTY_USER(lwGUID kGuid)
	{
		if( !IsCanPartyArea() )//파티가 가능한 지역인가?
		{
			lwAddWarnDataTT(PMS_CantArea);
			return;
		}

		if( PgClientExpeditionUtil::IsInExpedition() )
		{ // 현재 원정대에 속해 있는가?
			return;
		}

		PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
		if( !pkPC )
		{return;}

		if( !IsPartyMaster(pkPC->GetID()) )
		{
			PartyResultMsg(PRC_Fail_NotMaster);
			return;
		}

		BM::Stream kPacket(PT_C_N_REQ_KICKOUT_PARTY_USER);
		kPacket.Push((BYTE)PCT_KICK_CHARGUID);
		kPacket.Push(kGuid());
		NETWORK_SEND(kPacket)
	}

	void Net_C_N_ANS_JOIN_PARTY_EX(bool bAccept, BM::GUID const &rkCharGuid)
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgActor* pkActor = dynamic_cast<PgActor*>(g_pkWorld->FindObject(rkCharGuid));
		if( !pkActor ) return;
		PgPilot* pkPilot = pkActor->GetPilot();
		if( !pkPilot ) return;
		CUnit* pkUnit = pkPilot->GetUnit();
		if( !pkUnit ) return;
		PgPlayer* pkPC = dynamic_cast<PgPlayer*>(pkUnit);
		if( !pkPC ) return;

		if( GUID_NULL == pkPC->PartyGuid() ) return;

		Net_C_N_ANS_JOIN_PARTY(bAccept, pkPC->PartyGuid());
	}

	bool GetPartyJoinLevelCheck(BM::GUID const &rkCharGuid)
	{
		const PgPilot* pkPilot = g_kPilotMan.FindPilot(rkCharGuid);
		if( !pkPilot ) return false;
		CUnit* pkUnit = pkPilot->GetUnit();
		if( !pkUnit ) return false;
		PgPlayer* pkPC = dynamic_cast<PgPlayer*>(pkUnit);
		if( !pkPC ) return false;

		const SPartyOption& rkOption = g_kParty.Option();

		if( pkPC->GetAbil(AT_LEVEL) < rkOption.GetOptionLevel() )
		{
			PartyResultMsg(PRC_Fail_Level);
			return false;
		}

		return true;
	}

	void Net_C_N_ANS_JOIN_PARTY(bool bAccept, BM::GUID const &rkPartyGUID)
	{
		if ( g_pkWorld && (g_pkWorld->IsHaveAttr(GATTR_FLAG_NOPARTY) || g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND)) )
		{
			return;
		}

		if( IsInParty() )
		{
			return;
		}

		if( PgClientExpeditionUtil::IsInExpedition() )
		{ // 현재 원정대에 속해 있는가?
			return;
		}

		PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkMyPlayer )
		{return;}


		BM::Stream kPacket(PT_C_N_ANS_JOIN_PARTY);
		kPacket.Push(bAccept);
		kPacket.Push(rkPartyGUID);
		NETWORK_SEND(kPacket)
	}

	void Net_C_N_ANS_JOIN_PARTYFIND(bool bAccept, BM::GUID const &rkUserGUID)
	{
		if ( g_pkWorld && (g_pkWorld->IsHaveAttr(GATTR_FLAG_NOPARTY) || g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND)) )
		{
			return;
		}

		PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkMyPlayer )
		{return;}


		BM::Stream kPacket(PT_C_N_ANS_JOIN_PARTYFIND);
		kPacket.Push(rkUserGUID);
		kPacket.Push(bAccept);
		kPacket.Push(pkMyPlayer->PartyGuid());
		kPacket.Push(pkMyPlayer->GroundKey());
		NETWORK_SEND(kPacket)
	}

	void Net_C_N_REQ_JOIN_PARTY_ByName(lwWString kName)
	{
		std::wstring const &rkCharName = kName();
		
		if( !rkCharName.size() )
		{return;}

		if( PgClientExpeditionUtil::IsInExpedition() )
		{ // 현재 원정대에 속해 있는가?
			std::wstring Message = TTW(710067); //원정대 참가 중에는 파티 초대/가입을 할 수 없습니다.
			SChatLog ChatLog(CT_EVENT_SYSTEM);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
			return;
		}

		const PgPilot* pkPilot = g_kPilotMan.FindPilot(kName(), false);
		if( pkPilot )
		{ // 초대 대상이 원정대에 속해 있는가?
			PgPlayer* pPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
			if( NULL == pPlayer )
			{
				return;
			}
			if( BM::GUID::NullData() != pPlayer->ExpeditionGuid() )
			{
				BM::vstring vStr(TTW(720008));
				vStr.Replace(L"#NAME#", pPlayer->Name()); //#NAME#님께서는 이미 원정대를 등록한 상태 입니다.
				SChatLog ChatLog(CT_EVENT_SYSTEM);
				g_kChatMgrClient.AddLogMessage(ChatLog, (std::wstring)vStr, true);
				return;
			}
		}

		if( !IsCanPartyArea(true) )//파티가 가능한 지역인가?
		{
			lwAddWarnDataTT(PMS_CantArea);
			return;
		}

		if( false == IsCanInviteFriendPartyArea() )
		{//파티 난입 시 입장아이템이 필요한 지역인가?
			lwAddWarnDataTT(700414);
			return;
		}

		if( BM::GUID::NullData() != g_kParty.PartyGuid() )//파티가 있으면서 초대 할때는 파장만 가능
		{
			PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
			if( !pkPC )
			{return;}

			if( !IsPartyMaster(pkPC->GetID()) )
			{
				PartyResultMsg(PRC_Fail_NotMaster);
				return;
			}
		}

		if( g_kParty.IsMember(kName()) )
		{
			PartyResultMsg(PRC_Fail_MineMember);
			return;
		}

		if( !g_kParty.CheckLastInviteTime(rkCharName) )
		{
			return;
		}
		
		BM::Stream kPacket(PT_C_N_REQ_JOIN_PARTY);
		kPacket.Push((BYTE)PCT_REQJOIN_CHARNAME);
		kPacket.Push(rkCharName);
		NETWORK_SEND(kPacket)

		if( PgClientExpeditionUtil::IsInExpedition() == false )
		{
			std::wstring kMessage = rkCharName + TTW(PMS_Is2) + TTW(PMS_Invite); //누구 님을 초대
			SChatLog kChatLog(CT_EVENT_SYSTEM);
			g_kChatMgrClient.AddLogMessage(kChatLog, kMessage, true);
		}
	}

	void Net_PT_C_M_REQ_MOVETOPARTYMEMBER(lwGUID kGuid)
	{
		BM::GUID const &rkCharGuid = kGuid();
		if( BM::GUID::NullData() == rkCharGuid )
		{
			return;
		}

		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		if( pkPlayer->IsDead() )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				g_kChatMgrClient.LogMsgBox( 10410 );
			}
			return;
		}

		SPartyMember const* pkMember = NULL;
		if( !g_kParty.GetMember(rkCharGuid, pkMember) )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				g_kChatMgrClient.LogMsgBox( 401306 );
			}
			return;
		}

		CONT_DEFMAP const* pkDefMap = NULL;
		g_kTblDataMgr.GetContDef(pkDefMap);

		CONT_DEFMAP::const_iterator find_iter = pkDefMap->find( pkMember->GroundNo() );
		if( pkDefMap->end() == find_iter )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				g_kChatMgrClient.LogMsgBox( 401312 );
			}
			return;
		}

		bool bCant = false;
		if( true == g_kLocal.IsServiceRegion( LOCAL_MGR::NC_TAIWAN ) )
		{
			bCant = (0 != ((*find_iter).second.iAttr&GATTR_FLAG_CANT_PARTYWARP));
		}
		else
		{
			bCant = (0 != ((*find_iter).second.iAttr&GATTR_FLAG_CANT_WARP));
		}

		if( bCant )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				g_kChatMgrClient.LogMsgBox( 1930 );
			}
			return;
		}

		if( PgClientExpeditionUtil::IsInExpedition() == false )
		{
			CallYesNoMsgBox(TTW(401302), rkCharGuid, MBT_COMMON_YESNO_PARTYMEMBER);
		}
	}

	void Net_PT_C_M_REQ_MOVETOPARTYMASTERGROUND(lwGUID kGuid)
	{
		BM::GUID const& rkCharGuid = kGuid();
		if( BM::GUID::NullData() == rkCharGuid )
		{
			return;
		}

		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		if( pkPlayer->IsDead() )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				g_kChatMgrClient.LogMsgBox( 10410 );
			}
			return;
		}

		SPartyMember const* pkMember = NULL;
		if( !g_kParty.GetMember(rkCharGuid, pkMember) )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				g_kChatMgrClient.LogMsgBox( 401306 );
			}
			return;
		}

		CONT_DEFMAP const* pkDefMap = NULL;
		g_kTblDataMgr.GetContDef(pkDefMap);

		CONT_DEFMAP::const_iterator find_iter = pkDefMap->find( pkMember->GroundNo() );
		if( pkDefMap->end() == find_iter )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				g_kChatMgrClient.LogMsgBox( 401312 );
			}
			return;
		}
/*
		bool bCant = false;
		if( true == g_kLocal.IsServiceRegion( LOCAL_MGR::NC_TAIWAN ) )
		{
			bCant = (0 != ((*find_iter).second.iAttr & GATTR_FLAG_CANT_PARTYWARP));
		}
		else
		{
			bCant = (0 != ((*find_iter).second.iAttr & GATTR_FLAG_CANT_WARP));
		}

		if( bCant )
		{
			g_kChatMgrClient.LogMsgBox( 1930 );
			return;
		}
*/
		CONT_DEF_TRANSTOWER const *pkDefTransTower = NULL;
		g_kTblDataMgr.GetContDef( pkDefTransTower );

		__int64 i64Price = 0;
		__int64 i64HaveMoney = 0;
		bool bFindPrice = false;

		CONT_DEF_TRANSTOWER::const_iterator iter_tt = pkDefTransTower->begin();
		while( iter_tt != pkDefTransTower->end() )
		{
			CONT_DEF_TRANSTOWER_TARGET::const_iterator iter_target = iter_tt->second.begin();
			while( iter_target != (iter_tt->second.end()) )
			{
				if( iter_target->iGroundNo == pkMember->GroundNo() )
				{
					i64Price = iter_target->i64Price;
					g_kParty.ToPartyMasterWarpCost(i64Price);
					g_kParty.PartyMasterTransTowerGuid(iter_tt->first);
					g_kParty.PartyMasterTransTowerKey((*iter_target));
					bFindPrice = true;
					break;
				}
				++iter_target;
			}

			if( bFindPrice )
			{
				break;
			}
			++iter_tt;
		}

		if( !bFindPrice )	// 전송타워가 없는 맵.
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				g_kChatMgrClient.LogMsgBox( 1930 );
			}
			return;
		}

		std::wstring kText;
		int iPrice[3] = {0,};
		iPrice[2] = i64Price % 100i64; i64Price /= 100i64;
		iPrice[1] = i64Price % 100i64; i64Price /= 100i64;
		iPrice[0] = i64Price % 100i64;

		for( int i = 0 ; i < 3 ; ++i )
		{
			if( 0 == iPrice[i] )
			{
				continue;
			}
			char temp[10] = {0,};
			sprintf(temp, "%d", iPrice[i]);
			kText += UNI(temp) + TTW(401111 + i);
		}

		std::wstring kOutText;
		WstringFormat( kOutText, MAX_PATH, TTW(799341).c_str(), kText.c_str());
		CallYesNoMsgBox(kOutText, rkCharGuid, MBT_COMMON_YESNO_PARTYMASTERGROUND);
	}

	void Net_PT_C_M_REQ_SUMMONPARTYMEMBER(lwGUID kGuid)
	{
		BM::GUID const &rkCharGuid = kGuid();
		
		if( BM::GUID::NullData() != rkCharGuid )
		{
			std::wstring rkMsg = std::wstring();

			int iItemNo = 98000170;
			SItemPos kItemPos;
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer){return;}
			PgInventory *pInv = pkPlayer->GetInven();
			if(pInv)
			{
				if (S_OK == pInv->GetFirstItem(iItemNo, kItemPos))
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					const CItemDef *pkDef = kItemDefMgr.GetDef(iItemNo);
					if( pkDef )
					{
						if( UICT_SUMMONPARTYMEMBER == pkDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
						{
							SPT_C_M_REQ_SUMMONPARTYMEMBER kStruct;
							kStruct.kCharGuid = rkCharGuid;
							kStruct.kItemPos = kItemPos;
							BM::Stream kPacket;
							kStruct.WriteToPacket(kPacket);
							NETWORK_SEND(kPacket)
						}
					}
					rkMsg = TTW(401310);
				}
				else
				{
					rkMsg = TTW(401309);
				}

				if( rkMsg.empty())
				{
					return;
				}

				if( PgClientExpeditionUtil::IsInExpedition() == false )
				{
					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddLogMessage(kChatLog, rkMsg);
					lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(rkMsg), true);
				}
			}
		}
	}

	void Net_C_N_REQ_JOIN_PARTY_ByGuid(lwGUID kGuid)
	{	
		if ( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION) )
		{
			lwAddWarnDataTT(PMS_CantArea);
			return;
		}

		if( PgClientExpeditionUtil::IsInExpedition() )
		{ // 현재 원정대에 속해 있는가?
			std::wstring Message = TTW(710067); //원정대 참가 중에는 파티 초대/가입을 할 수 없습니다.
			SChatLog ChatLog(CT_EVENT_SYSTEM);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
			return;
		}
		
		BM::GUID const &rkCharGuid = kGuid();
		const PgPilot* pkPilot = g_kPilotMan.FindPilot(rkCharGuid);
		if( pkPilot )
		{ // 초대 대상이 원정대에 속해 있는가?
			PgPlayer* pPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
			if( NULL == pPlayer )
			{
				return;
			}
			if( BM::GUID::NullData() != pPlayer->ExpeditionGuid() )
			{
				BM::vstring vStr(TTW(720008));
				vStr.Replace(L"#NAME#", pPlayer->Name()); //#NAME#님께서는 이미 원정대를 등록한 상태 입니다.
				SChatLog ChatLog(CT_EVENT_SYSTEM);
				g_kChatMgrClient.AddLogMessage(ChatLog, (std::wstring)vStr, true);
				return;
			}
		}

		if( !IsCanPartyArea(true) )//파티가 가능한 지역인가?
		{
			lwAddWarnDataTT(PMS_CantArea);
			return;
		}

		if( false == IsCanInviteFriendPartyArea() )
		{//파티 난입 시 입장아이템이 필요한 지역인가?
			lwAddWarnDataTT(700414);
			return;
		}

		if( BM::GUID::NullData() != g_kParty.PartyGuid() )//파티가 있으면서 초대 할때는 파장만 가능
		{
			PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
			if( !pkPC )
			{return;}

			if( !IsPartyMaster(pkPC->GetID()) )
			{
				PartyResultMsg(PRC_Fail_NotMaster);
				return;
			}
		}

		if( g_kParty.IsMember(kGuid()) )
		{
			PartyResultMsg(PRC_Fail_MineMember);
			return;
		}

		//if( !GetPartyJoinLevelCheck(rkCharGuid) ) return;

		if( !g_kParty.CheckLastInviteTime(rkCharGuid) )
		{
			return;
		}
		
		BM::Stream kPacket(PT_C_N_REQ_JOIN_PARTY);
		kPacket.Push((BYTE)PCT_REQJOIN_CHARGUID);
		kPacket.Push(rkCharGuid);
		NETWORK_SEND(kPacket)

		std::wstring kMessage;
		if( pkPilot )
		{
			std::wstring const &rkCharName = pkPilot->GetName();
			kMessage = rkCharName + TTW(PMS_Is2) + TTW(PMS_Invite); //누구 님을 초대
		}
		else
		{
			kMessage = TTW(PMS_Invite);//초대
		}	

		if( PgClientExpeditionUtil::IsInExpedition() == false )
		{
			SChatLog kChatLog(CT_EVENT_SYSTEM);
			g_kChatMgrClient.AddLogMessage(kChatLog, kMessage, true);
		}
	}


	void Net_C_N_REQ_JOIN_PARTYFIND_ByGuid(lwGUID kGuid)
	{
		if( !IsCanPartyArea(true) )//파티가 가능한 지역인가?
		{
			lwAddWarnDataTT(PMS_CantArea);
			return;
		}
		if( PgClientExpeditionUtil::IsInExpedition() )
		{ // 현재 원정대에 속해 있는가?
			lwAddWarnDataTT(PMS_Invite_Fail2);
			return;
		}
		BM::GUID const &rkCharGuid = kGuid();
		if( BM::GUID::NullData() != g_kParty.PartyGuid() )
		{
			lwAddWarnDataTT(PMS_Invite_Fail2);
			return;
		}

		if( !g_kParty.CheckLastInviteTime(rkCharGuid) )
		{
			lwAddWarnDataTT(PMS_DupReqJoin);
			return;
		}

		int MasterMapNo = 0;
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_PARTY_PART_FIND"));
		if( pkTopWnd )
		{
			XUI::CXUI_Wnd* pkTopFom = pkTopWnd->GetControl(_T("FRM_ITEM"));
			if( pkTopFom )
			{
				XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkTopFom->GetControl(_T("LST_PARTY_PART_LIST")));
				if( pkListWnd )
				{
					XUI::SListItem * pItem = pkListWnd->FirstItem();
					while( pItem )
					{
						XUI::CXUI_Wnd * pWnd = pItem->m_pWnd;
						if( pWnd )
						{
							if( kGuid() == pWnd->OwnerGuid() )
							{
								XUI::CXUI_Wnd * pArea = pWnd->GetControl(L"FRM_AREA");
								if( pArea )
								{
									pArea->GetCustomData(&MasterMapNo, sizeof(MasterMapNo));
								}
								break;
							}
						}
						pItem = pkListWnd->NextItem(pItem);
					}
				}
			}
		}

		if( MasterMapNo )
		{
			int NeedItemNo = 0;
			CONT_NEED_KEY_ITEM KeyItem, CashKeyItem;
			if( false == IsCanPartyBreakIn(MasterMapNo, KeyItem, CashKeyItem) )
			{
				if( IsHaveKeyItemPartyBreakIn( KeyItem, CashKeyItem, NeedItemNo ) )
				{
					std::wstring ItemName;
					if( GetItemName(NeedItemNo, ItemName) )
					{
						BM::vstring msg(TTW(700415));
						msg.Replace(L"#ITEM#", ItemName);
						lwCallYesNoMsgBox((std::wstring)msg, rkCharGuid, MBT_CONFIRM_CONSUME_ITEM_PARTY_BREAK_IN, (int)g_kParty.GetPartyChannelNo( rkCharGuid ) );
					}
				}
				else
				{
					std::wstring ItemName;
					if( GetItemName(NeedItemNo, ItemName) )
					{
						BM::vstring msg(TTW(700416));
						msg.Replace(L"#ITEM#", ItemName);
						lwAddWarnDataStr(lwWString((std::wstring)msg), 1, true);
					}
				}
				return;
			}
		}

		short ChannelNo = g_kParty.GetPartyChannelNo( rkCharGuid );
		BM::Stream kPacket(PT_C_M_REQ_INDUN_PARTY_ENTER);
		kPacket.Push(ChannelNo);
		kPacket.Push(rkCharGuid);
		NETWORK_SEND(kPacket)
	}


	void Net_PT_C_N_REQ_PARTY_RENAME(lwWString kNewName)
	{
		if( !IsInParty() )
		{return;}

		if( !IsCanPartyArea() )//파티가 가능한 지역인가?
		{
			lwAddWarnDataTT(PMS_CantArea);
			return;
		}

		if( PgClientExpeditionUtil::IsInExpedition() )
		{ // 현재 원정대에 속해 있는가?
			return;
		}

		PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
		if( !pkPC )
		{return;}

		if( !IsPartyMaster(pkPC->GetID()) ) return;

		std::wstring kNewPartyName = kNewName();
		if( kNewPartyName.empty() )
		{
			return;
		}

		bool const bReserve = g_kParty.IsReserveName(kNewPartyName);
		if( bReserve )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(700104, kChatLog, true);
			}
			return;
		}

		kNewPartyName = kNewPartyName.substr(0,MAX_PARTYNAME_LEN);
		BM::Stream kPacket(PT_C_N_REQ_PARTY_RENAME);
		//kPacket.Push((BYTE)KIT_CharGuid);
		//kPacket.Push(pkPC->GetID());
		kPacket.Push(kNewPartyName);
		kPacket.Push(false);//Clear Name
		NETWORK_SEND(kPacket)
	}

	void Net_C_N_REQ_CREATE_PARTY(bool bIsInParty, lwWString kPartyName)
	{
		if( !IsCanPartyArea(true) )//파티가 가능한 지역인가?
		{
			lwAddWarnDataTT(PMS_CantArea);
			return;
		}

		if( PgClientExpeditionUtil::IsInExpedition() )
		{ // 현재 원정대에 속해 있는가?
			return;
		}

		std::wstring rkPartyName = kPartyName();
		if( rkPartyName.empty() )
		{
			BM::GUID kTempGuid;
			kTempGuid.Generate();
			rkPartyName = g_kParty.GeneratePartyName(kTempGuid);

			/*SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddMessage(400434, kChatLog, true);
			return;*/
		}

		/*std::wstring const& rkPartySubName = kPartySubName();
		if( rkPartySubName.empty() )
		{
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddMessage(401328, kChatLog, true);
			return;
		}*/

		if(!g_kUnicodeFilter.IsCorrect(UFFC_PARTY_NAME, rkPartyName))
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(700407, kChatLog, true);
			}
			return;
		}

		/*if(!g_kUnicodeFilter.IsCorrect(UFFC_PARTY_NAME, rkPartySubName))
		{
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddMessage(700407, kChatLog, true);
			return;
		}*/		

		const SPartyOption& rkOption = g_kParty.Option();

		rkPartyName = rkPartyName.substr(0,MAX_PARTYNAME_LEN);
		BM::Stream kPacket(PT_C_N_REQ_CREATE_PARTY);
		kPacket.Push(rkPartyName);
		rkOption.WriteToPacket(kPacket);
		NETWORK_SEND(kPacket)
	}

	bool PartyOptionStr(const SPartyOption& rkNewOption, std::wstring& rkOut, bool bChange)
	{
		//파티 설정이 어쩌구 저쩌구
		int iItemTTW = 0;
		int iSubName = 0;
		bool bState = false, bItem = false, bSubName = false, bMaxMember = false;

		SPartyOption kPartyOption =	g_kParty.Option();

		if( kPartyOption.GetOptionItem() != rkNewOption.GetOptionItem() || !bChange )
		{
			if( rkNewOption.GetOptionItem() == POI_LikeSolo )//Item
			{
				iItemTTW = 401008;
			}
			else if( rkNewOption.GetOptionItem() == POI_Order )
			{
				iItemTTW = 401009;
			}
			bItem = true;
		}

		if( kPartyOption.PartySubName() != rkNewOption.PartySubName() || !bChange )
		{
			iSubName = 401318;
			bSubName = true;
		}		

		if( kPartyOption.GetOptionMaxMember() != rkNewOption.GetOptionMaxMember() || !bChange )
		{
			bMaxMember = true;
		}

		if( kPartyOption.GetOptionState() != rkNewOption.GetOptionState() || !bChange )
		{
			bState = true;
		}

		std::wstring kTemp;

		if( iItemTTW )
		{
			kTemp += TTW(iItemTTW);
		}

		if( bState )
		{
			if( bItem )
			{
				kTemp += _T(", ");
			}
			kTemp += TTW(799391);
		}

		if( iSubName )
		{
			if( bState || bItem )
			{
				kTemp += _T(", ");
			}
			kTemp += TTW(iSubName);
		}

		if( bMaxMember )
		{
			if( iSubName || bState || bItem )
			{
				kTemp += _T(", ");
			}
			BM::vstring vStr;
			vStr = TTW(22008);
			vStr.Replace(L"#NUM#", rkNewOption.GetOptionMaxMember());
			kTemp += (std::wstring)vStr;
		}

		if( bChange )
		{
			if( bItem )
			{// 파티장이 ~ 로 변경 하였습니다
				rkOut = TTW(401000) + kTemp + TTW(401357);
			}
			if( iSubName || bState )
			{// 파티장이 ~ 를 변경 하였습니다
				rkOut = TTW(401000) + kTemp + TTW(401001);
			}
			if( bMaxMember )
			{// 파티장이 ~ 으로 변경 하였습니다
				rkOut = TTW(401000) + kTemp + TTW(22009);
			}
		}
		else
		{
			rkOut = kTemp;
		}
		
		return !kTemp.empty();
	}

	void Send_RequestPartyList()
	{
		// 기능이 바뀜
		/*BM::Stream kPacket(PT_C_M_REQ_PARTY_LIST);
		NETWORK_SEND(kPacket)*/
	}

	void Send_RequestPartyListInfo(int iPartyAttribute, int iPartyContinent, int iPartyArea_NameNo)
	{
		BM::GUID kGuid;
		kGuid.Generate();
		BM::Stream kPacket(PT_C_M_REQ_PARTY_LIST);
		kPacket.Push(kGuid);
		kPacket.Push(iPartyAttribute);
		kPacket.Push(iPartyContinent);
		kPacket.Push(iPartyArea_NameNo);
		NETWORK_SEND(kPacket)
	}

	void Send_RequestPartyPeopleListInfo(int iClass, unsigned short iLevel)
	{
		BM::Stream kPacket(PT_C_M_REQ_FIND_PARTY_USER_LIST);
		PgPilot* pkPilot = g_kPilotMan.GetPlayerPilot();
		kPacket.Push(pkPilot->GetGuid());
		int iBaseClass = 0;
		switch(iClass)
		{
		case 400168:	// 전사계열
			{
				iBaseClass = 1;
			}break;
		case 400169:	// 마법사계열
			{
				iBaseClass = 2;
			}break;
		case 400170:	// 궁수계열
			{
				iBaseClass = 3;
			}break;
		case 400171:	// 도둑계열
			{
				iBaseClass = 4;
			}break;
		case 791251:	// 소환사계열
			{
				iBaseClass = 51;
			}break;
		case 791252:	// 쌍둥이계열
			{
				iBaseClass = 52;
			}break;
		case 0:
			{
				iBaseClass = 0;
			}break;
		default:
			{
				iBaseClass = 0;
			}break;
		}
		kPacket.Push(iBaseClass);
		unsigned short iLLevel;
		if( 0 == iLevel )
		{
			iLLevel = 0;
		}
		else
		{
			iLLevel = iLevel - 10;
		}
		kPacket.Push(iLLevel);
		kPacket.Push(iLevel);
		NETWORK_SEND(kPacket);
	}

	bool Send_PrivateRegist(lwStyleString kContents)
	{
		if( IsInParty() )	// 파티있음.
		{
			return false;
		}
		if( PgClientExpeditionUtil::IsInExpedition() )
		{ // 현재 원정대에 속해 있는가?
			return false;
		}
		if( 0 == kContents.GetNormalText().Length() )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				SChatLog kChatLog((EChatType)CT_EVENT);
				g_kChatMgrClient.AddMessage(799386, kChatLog, true, 11);
			}
			return false;
		}
		else
		{
		    BM::Stream kPacket(PT_C_M_REQ_REGIST_PRIVATE_PARTY_FIND);
		    PgPilot* pkPilot = g_kPilotMan.GetPlayerPilot();
		    kPacket.Push(pkPilot->GetGuid());
		    kPacket.Push(kContents.GetStyleString().GetOriginalString());
		    NETWORK_SEND(kPacket);
			return true;
		}
	}

	void Send_RequestSearchPeopleList()
	{
		BM::Stream kPacket(PT_C_M_REQ_SEARCH_PEOPLE_LIST);
		NETWORK_SEND(kPacket)
	}

	lwGUID lwGetMemberGuidAt(int const iAt)
	{
		return lwGUID(g_kParty.GetMemberGuidAt(iAt));
	}

	lwWString lwGetMemberNameAt(int const iAt)
	{
		std::wstring kName;
		if(g_kParty.GetMemberNameAt(iAt, kName))
		{
			return lwWString(kName);
		}
		return lwWString("");
	}

	std::wstring GetClassNameTTW(int const iClassNo)
	{
		int const iTTWClassNameBase = 30000;
		return TTW(iClassNo + iTTWClassNameBase);
	}

	void lwGetPartyListAreaName(bool bAll, int const iAttribute, int const iContinent, lwUIWnd lwWnd)
	{
		g_kParty.GetPartyListAreaName(bAll, iAttribute, iContinent, lwWnd);
	}

	void lwFakePartyListViewRefresh(int iNowPage, int iAttribute)
	{
		g_kParty.FakePartyListViewRefresh(iNowPage, iAttribute);
	}

	void lwPartyListViewRefresh(int iNowPage, int iAttribute)
	{
		g_kParty.PartyListViewRefresh(iNowPage, iAttribute);
	}

	void lwFindPartyUserListViewRefresh(int iNowPage, int iAttribute)
	{
		g_kParty.FindPartyUserListViewRefresh(iNowPage, iAttribute);
	}

	int lwPartyListNowPage()
	{
		return g_kParty.PartyListNowPage();
	}

	int lwPartyFindPeopleListNowPage()
	{
		return g_kParty.PartyFindPeopleListNowPage();
	}

	int lwPartyFindPeopleListMaxPage()
	{
		return g_kParty.PartyFindPeopleListMaxPage();
	}

	int lwPartyListMaxPage()
	{
		return g_kParty.PartyListMaxPage();
	}

	void lwSetPartyListInit()
	{
		g_kParty.SetPartyListInit();
	}

	void ClearOtherPartyName()
	{
		g_kParty.ClearOtherPartyName();
	}

	lwWString lwGetPartyListToolTip(lwGUID kGuid)
	{
		BM::GUID rkCharGuid = kGuid();
		std::wstring kStr;
		if( g_kParty.GetPartyListToolTip(rkCharGuid, kStr) )
		{
			return lwWString(kStr);
		}
		return lwWString("");
	}

	lwWString lwGetPartyListStateToolTip(lwGUID kGuid)
	{
		BM::GUID rkCharGuid = kGuid();
		std::wstring kStr;
		if( g_kParty.GetPartyListStateToolTip(rkCharGuid, kStr) )
		{
			return lwWString(kStr);
		}
		return lwWString("");
	}

	void lwPromoteParty()
	{
		if( PgClientExpeditionUtil::IsInExpedition() )
		{
			return;
		}
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		SClientPartyName PartyName;

		g_kParty.GetPartyName(pkPlayer->GetPartyGuid(), PartyName);
	
		XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(FONT_TEXT);
		XUI::CXUI_Style_String	kText(XUI::PgFontDef(pFont, 0xFF00FF00), std::wstring( TTW(799380) + PartyName.kPartyName ));
		g_kChatMgrClient.ChatMode(CT_NORMAL);
		g_kChatMgrClient.SendChat_CheckSS(kText);
	}

	void PartyStateModify()
	{
		if( IsInParty() && IsPartyMaster(g_kPilotMan.GetPlayerPilot()->GetGuid()) )
		{
			g_kParty.PartyState(false);
		}
	}

	void lwSetPartyListSortMethod(int iSort)
	{
		g_kParty.PartyListSortMethod(iSort);
	}

	int lwGetPartyListSortMethod()
	{
		return g_kParty.PartyListSortMethod();
	}

	void lwSetFindPartyUserListSortMethod(int iSort)
	{
		g_kParty.FindPartyUserListSortMethod(iSort);
	}

	int lwGetFindPartyUserListSortMethod()
	{
		return g_kParty.FindPartyUserListSortMethod();
	}

	bool PartyListSort(SPartyListInfo const & lhs, SPartyListInfo const & rhs)
	{
		int iPartySort = g_kParty.PartyListSortMethod();
		switch( iPartySort )
		{
		case PLS_STATEGREATER:
			{
				return (lhs.cStatus > rhs.cStatus);
			}break;
		case PLS_STATELESS:
			{
				return (lhs.cStatus < rhs.cStatus);
			}break;
		case PLS_NAMEGREATER:
			{
				return (lhs.kPartyName > rhs.kPartyName);
			}break;
		case PLS_NAMELESS:
			{
				return (lhs.kPartyName < rhs.kPartyName);
			}break;
		case PLS_ATTRGREATER:
		case PLS_ATTRLESS:
			{
				int iLhsSortNo = 0, iRhsSortNo = 0;
				CONT_DEF_PARTY_INFO const* pkDefPartyInfo = NULL;
				g_kTblDataMgr.GetContDef(pkDefPartyInfo);
				if( pkDefPartyInfo )
				{
					CONT_DEF_PARTY_INFO::const_iterator lhs_iter = pkDefPartyInfo->find(lhs.iPartyContinent);
					CONT_DEF_PARTY_INFO::const_iterator rhs_iter = pkDefPartyInfo->find(rhs.iPartyContinent);
					if( (lhs_iter != pkDefPartyInfo->end()) && (rhs_iter != pkDefPartyInfo->end()) )
					{
						CONT_DEF_PARTY_INFO::mapped_type kContinentList = lhs_iter->second;
						CONT_DEF_PARTY_INFO::mapped_type::const_iterator iter_ContinentList = kContinentList.begin();
						while( iter_ContinentList != kContinentList.end() )
						{
							if( (*iter_ContinentList).iArea_NameNo == lhs.iPartyArea_NameNo )
							{
								iLhsSortNo = (*iter_ContinentList).iSort;
								break;
							}
							++iter_ContinentList;
						}

						kContinentList = rhs_iter->second;
						iter_ContinentList = kContinentList.begin();
						while( iter_ContinentList != kContinentList.end() )
						{
							if( (*iter_ContinentList).iArea_NameNo == rhs.iPartyArea_NameNo )
							{
								iRhsSortNo = (*iter_ContinentList).iSort;
								break;
							}
							++iter_ContinentList;
						}

						if( PLS_ATTRGREATER == iPartySort )
						{
							return (iLhsSortNo > iRhsSortNo);
						}
						else
						{
							return (iLhsSortNo < iRhsSortNo);
						}
					}
				}
			}break;
		case PLS_MEMBERGRATER:
			{
				return (lhs.cCurMember > rhs.cCurMember);
			}break;
		case PLS_MEMBERLESS:
			{
				return (lhs.cCurMember < rhs.cCurMember);
			}break;
		default:
			{
				return (lhs.cStatus < rhs.cStatus);
			}
		}
		return (lhs.cStatus < rhs.cStatus);
	}

	bool FindPartyUserListSort(SFindPartyUserListInfo const & lhs, SFindPartyUserListInfo const & rhs)
	{
		int iSort = g_kParty.FindPartyUserListSortMethod();
		switch( iSort )
		{
		case FPL_TITLEGREATER:
			{
				return (lhs.kContents > rhs.kContents);
			}break;
		case FPL_TITLELESS:
			{
				return (lhs.kContents < rhs.kContents);
			}break;
		case FPL_LEVELGREATER:
			{
				return (lhs.sLevel > rhs.sLevel);
			}break;
		case FPL_LEVELLESS:
			{
				return (lhs.sLevel < rhs.sLevel);
			}break;
		case FPL_CLASSGREATER:
			{
				return (lhs.iOriginalClass > rhs.iOriginalClass);
			}break;
		case FPL_CLASSLESS:
			{
				return (lhs.iOriginalClass < rhs.iOriginalClass);
			}break;
		default:
			{
				return (lhs.kContents < rhs.kContents);
			}
		}
		return (lhs.kContents < rhs.kContents);
	}

	lwWString GetPartyName()
	{
		PgPlayer * pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( IsPartyMaster(pkPlayer->GetID()) )
		{
			SClientPartyName kPartyName;
			BM::GUID kPartyGuid = pkPlayer->PartyGuid();
			g_kParty.GetPartyName(kPartyGuid, kPartyName);
			return lwWString(kPartyName.kPartyName);
		}

		return lwWString("");
	}

	void SendUnRegistPrivate()
	{
		BM::Stream SendPacket(PT_N_C_REQ_UNREGIST_PRIVATE);

		NETWORK_SEND(SendPacket);
	}
	bool DoPartyMemberTeleMoveIn(int const iType)
	{
		ContPartyMember kTempPartyMember;
		g_kParty.GetPartyMemberList( kTempPartyMember );

		ContPartyMember::const_iterator kItor = kTempPartyMember.begin();
		int iCount = 0;
		while(kTempPartyMember.end() != kItor)
		{
			if(*kItor)
			{
				PgActor* pkOtherActor = g_kPilotMan.FindActor( (*kItor)->kCharGuid );
				if( !pkOtherActor )
				{
					return false;
				}
				if( !pkOtherActor->IsMyActor() )
				{
					lwAction kAction = pkOtherActor->ReserveTransitAction("a_teleport_rocket");
					if(!kAction.IsNil())
					{
						kAction.SetParamInt(100, iType);
					}
				}
			}
			++kItor;
		}
		return true;
	}
	bool DoPartyMemberTeleMoveOut(int const iType)
	{
		ContPartyMember kTempPartyMember;
		g_kParty.GetPartyMemberList( kTempPartyMember );

		ContPartyMember::const_iterator kItor = kTempPartyMember.begin();
		int iCount = 0;
		while(kTempPartyMember.end() != kItor)
		{
			if(*kItor)
			{
				PgActor* pkOtherActor = g_kPilotMan.FindActor( (*kItor)->kCharGuid );
				if( !pkOtherActor )
				{
					return false;
				}
				if( !pkOtherActor->IsMyActor() )
				{
					lwAction kAction = pkOtherActor->ReserveTransitAction("a_teleport_rocket_out");
					if(!kAction.IsNil())
					{
						kAction.SetParamInt(100, iType);
					}
				}
			}
			++kItor;
		}
		return true;
	}

	lwWString GeneratePartyName()
	{
		BM::GUID kTempGuid;
		kTempGuid.Generate();
		return lwWString(g_kParty.GeneratePartyName(kTempGuid));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////

PgClientParty::PgClientParty()
	:m_kMemberPool(10)
	, m_kPartyListChannelCount(0)
	, m_kTotalChannelCount(0)
{
	Clear();

	m_kReservePartyName.push_back(TTW(400589));//선 예약된 파티 이름
	m_kReservePartyName.push_back(TTW(400590));
	m_kReservePartyName.push_back(TTW(400591));

	SetPartyTotalList();

	PartyState(false);
	PartyListSortMethod(PLS_STATELESS);
	FindPartyUserListSortMethod(FPL_TITLEGREATER);
}

PgClientParty::~PgClientParty()
{
}

bool PgClientParty::RegisterWrapper(lua_State *pkState)
{
	if( !pkState )
	{
		return false;
	}

	using namespace lua_tinker;
	def(pkState, "SendReqLeaveParty", &PgClientPartyUtil::Send_Request_Leave_Party);
	def(pkState, "IsCanPartyArea", &PgClientPartyUtil::IsCanPartyArea);
	def(pkState, "IsCanPvPArea", &PgClientPartyUtil::IsCanPvPArea);
	def(pkState, "SendReqChangeMaster", &PgClientPartyUtil::Send_RequestChangeMaster);
	def(pkState, "Send_RequestChangeOption", &PgClientPartyUtil::Send_RequestChangeOption);

	def(pkState, "SetPartyOption1", &PgClientPartyUtil::SetPartyOption1);
	def(pkState, "SetPartyOption2", &PgClientPartyUtil::SetPartyOption2);

	def(pkState, "Net_C_N_REQ_KICKOUT_PARTY_USER", &PgClientPartyUtil::Net_C_N_REQ_KICKOUT_PARTY_USER);
	def(pkState, "Net_C_N_ANS_JOIN_PARTY_EX", &PgClientPartyUtil::Net_C_N_ANS_JOIN_PARTY_EX);	
	def(pkState, "Net_C_N_REQ_JOIN_PARTY_ByName", &PgClientPartyUtil::Net_C_N_REQ_JOIN_PARTY_ByName);
	def(pkState, "Net_C_N_REQ_JOIN_PARTY_ByGuid", &PgClientPartyUtil::Net_C_N_REQ_JOIN_PARTY_ByGuid);
	def(pkState, "Net_C_N_REQ_CREATE_PARTY", &PgClientPartyUtil::Net_C_N_REQ_CREATE_PARTY);
	def(pkState, "Net_PT_C_N_REQ_PARTY_RENAME", &PgClientPartyUtil::Net_PT_C_N_REQ_PARTY_RENAME);
	def(pkState, "Net_C_N_REQ_JOIN_PARTYFIND_ByGuid", &PgClientPartyUtil::Net_C_N_REQ_JOIN_PARTYFIND_ByGuid);

	//Party
	def(pkState, "RefreshPartyInfoWnd", &PgClientPartyUtil::RefreshPartyInfoWnd);
	def(pkState, "RefreshPartyPeopleFindWnd", &PgClientPartyUtil::RefreshPartyPeopleFindWnd);	
	def(pkState, "RefreshPartyStateWnd", &PgClientPartyUtil::RefreshPartyStateWnd);
	def(pkState, "Refresh_Part_People_FindWnd", &PgClientPartyUtil::Refresh_Part_People_FindWnd);	
	def(pkState, "RefreshPartyTitleWnd", &PgClientPartyUtil::RefreshPartyTitleWnd);
	def(pkState, "SetPartyOptionArea", &PgClientPartyUtil::SetPartyOptionArea);
	def(pkState, "GetClassName", &PgClientPartyUtil::GetClassName);
	def(pkState, "GetPartyMemberLocation", &PgClientPartyUtil::GetPartyMemberLocation);
	def(pkState, "IsPartyMaster", &PgClientPartyUtil::IsPartyMaster);
	def(pkState, "IsInParty", &PgClientPartyUtil::IsInParty);
	def(pkState, "GetPartyOptionExp", &PgClientPartyUtil::GetPartyOptionExp);
	def(pkState, "GetPartyOptionItem", &PgClientPartyUtil::GetPartyOptionItem);
	def(pkState, "GetPartyOptionPublicTitle", &PgClientPartyUtil::GetPartyOptionPublicTitle);
	def(pkState, "SetPartyOption", &PgClientPartyUtil::SetPartyOption);
	def(pkState, "SetPartyOptionNew", &PgClientPartyUtil::SetPartyOptionNew);	
	def(pkState, "GetPartyOptionWStr", &PgClientPartyUtil::GetPartyOptionWStr);
	def(pkState, "GetPartyOptionState", &PgClientPartyUtil::GetPartyOptionState);
	def(pkState, "IsOtherPartyMaster", &PgClientPartyUtil::IsOhterPartyMaster);

	def(pkState, "Send_RequestPartyList", &PgClientPartyUtil::Send_RequestPartyList);
	def(pkState, "Send_RequestPartyListInfo", &PgClientPartyUtil::Send_RequestPartyListInfo);
	def(pkState, "Send_RequestSearchPeopleList", &PgClientPartyUtil::Send_RequestSearchPeopleList);
	def(pkState, "SearchPeopleUpdate", &PgClientPartyUtil::SearchPeopleUpdate);
	def(pkState, "IsInPartyMemberGuid", &PgClientPartyUtil::IsInPartyMemberGuid);
	def(pkState, "Net_PT_C_M_REQ_MOVETOPARTYMEMBER", &PgClientPartyUtil::Net_PT_C_M_REQ_MOVETOPARTYMEMBER);
	def(pkState, "Net_PT_C_M_REQ_SUMMONPARTYMEMBER", &PgClientPartyUtil::Net_PT_C_M_REQ_SUMMONPARTYMEMBER);	
	def(pkState, "GetMemberGuidAt", &PgClientPartyUtil::lwGetMemberGuidAt);	
	def(pkState, "GetMemberNameAt", &PgClientPartyUtil::lwGetMemberNameAt);	
	def(pkState, "HaveIParty", &PgClientPartyUtil::lwHaveIParty);
	def(pkState, "EnablePartyBreakIn", &PgClientPartyUtil::lwEnablePartyBreakIn);
	def(pkState, "Send_PT_C_M_REQ_JOIN_PARTY_REFUSE", &PgClientPartyUtil::lwSend_PT_C_M_REQ_JOIN_PARTY_REFUSE);
	def(pkState, "CheckCanKick", &PgClientPartyUtil::lwCheckCanKick);
	def(pkState, "CheckCanKick_Leave", &PgClientPartyUtil::lwCheckCanKick_Leave);
	def(pkState, "GetPartyListAreaName", &PgClientPartyUtil::lwGetPartyListAreaName);
	def(pkState, "FakePartyListViewRefresh", &PgClientPartyUtil::lwFakePartyListViewRefresh);
	def(pkState, "PartyListViewRefresh", &PgClientPartyUtil::lwPartyListViewRefresh);	
	def(pkState, "FindPartyUserListViewRefresh", &PgClientPartyUtil::lwFindPartyUserListViewRefresh);
	def(pkState, "PartyListNowPage", &PgClientPartyUtil::lwPartyListNowPage);	
	def(pkState, "PartyListMaxPage", &PgClientPartyUtil::lwPartyListMaxPage);	
	def(pkState, "GetPartyListToolTip", &PgClientPartyUtil::lwGetPartyListToolTip);
	def(pkState, "GetPartyListStateToolTip", &PgClientPartyUtil::lwGetPartyListStateToolTip);
	def(pkState, "SetPartyListInit", &PgClientPartyUtil::lwSetPartyListInit);	
	def(pkState, "Net_PT_C_M_REQ_MOVETOPARTYMASTERGROUND", &PgClientPartyUtil::Net_PT_C_M_REQ_MOVETOPARTYMASTERGROUND);
	def(pkState, "Send_RequestPartyPeopleListInfo", &PgClientPartyUtil::Send_RequestPartyPeopleListInfo);
	def(pkState, "Send_PrivateRegist", &PgClientPartyUtil::Send_PrivateRegist);
	def(pkState, "PartyFindPeopleListMaxPage", &PgClientPartyUtil::lwPartyFindPeopleListMaxPage);
	def(pkState, "PartyFindPeopleListNowPage", &PgClientPartyUtil::lwPartyFindPeopleListNowPage);
	def(pkState, "PromoteParty", &PgClientPartyUtil::lwPromoteParty);
	def(pkState, "PartyStateModify", &PgClientPartyUtil::PartyStateModify);
	def(pkState, "SetPartyListSortMethod", &PgClientPartyUtil::lwSetPartyListSortMethod);
	def(pkState, "GetPartyListSortMethod", &PgClientPartyUtil::lwGetPartyListSortMethod);
	def(pkState, "GetPartyName", &PgClientPartyUtil::GetPartyName);	
	def(pkState, "SendUnRegistPrivate", &PgClientPartyUtil::SendUnRegistPrivate);
	def(pkState, "SetFindPartyUserListSortMethod", &PgClientPartyUtil::lwSetFindPartyUserListSortMethod);
	def(pkState, "GetFindPartyUserListSortMethod", &PgClientPartyUtil::lwGetFindPartyUserListSortMethod);
	def(pkState, "DoPartyMemberTeleMoveIn", &PgClientPartyUtil::DoPartyMemberTeleMoveIn);
	def(pkState, "DoPartyMemberTeleMoveOut", &PgClientPartyUtil::DoPartyMemberTeleMoveOut);
	def(pkState, "GeneratePartyName", &PgClientPartyUtil::GeneratePartyName);
	def(pkState, "ClearOtherPartyName", &PgClientPartyUtil::ClearOtherPartyName);
	return true;
}

void PgClientParty::MemberMapMove(lwGUID lwkGuid, SGroundKey const &rkKey)
{
	ContPartyMember::iterator kIter = m_Members.begin();
	for(; m_Members.end() != kIter; ++kIter)
	{
		if((*kIter)->kCharGuid == lwkGuid())
		{
			(*kIter)->kChnGndKey = rkKey;
		}
	}
}


bool PgClientParty::IsMember(std::wstring const &rkName)const
{
	BM::CAutoMutex kLock(m_kMutex);
	return NULL != GetMember(rkName);
}

bool PgClientParty::IsMember(BM::GUID const &rkCharGuid)const
{
	BM::CAutoMutex kLock(m_kMutex);
	return NULL != GetMember(rkCharGuid);
}

SPartyMember* PgClientParty::GetMember(std::wstring const &rkName)const
{
	for(ContPartyMember::const_iterator itor = m_Members.begin(); itor != m_Members.end(); itor++)
	{
		const ContPartyMember::value_type pkElement = (*itor);
		if( pkElement
		&&	pkElement->kName == rkName )
		{
			return pkElement;
		}
	}
	return 0;
}
bool PgClientParty::GetMember(std::wstring const &rkName, SPartyMember const*& pkMember)const
{
	BM::CAutoMutex kLock(m_kMutex);
	pkMember = GetMember(rkName);
	return NULL != pkMember;
}

SPartyMember* PgClientParty::GetMember(BM::GUID const &rkCharGuid)const
{
	for(ContPartyMember::const_iterator itor = m_Members.begin(); itor != m_Members.end(); itor++)
	{
		const ContPartyMember::value_type pkElement = (*itor);
		if( pkElement
		&&	pkElement->kCharGuid == rkCharGuid)
		{
			return (*itor);
		}
	}
	return 0;
}
bool PgClientParty::GetMember(BM::GUID const &rkCharGuid, SPartyMember const*& pkMember)const
{
	BM::CAutoMutex kLock(m_kMutex);
	pkMember = GetMember(rkCharGuid);
	return NULL != pkMember;
}


//	신규 멤버를 추가
void PgClientParty::AddMember(const SPartyUserInfo &rkMemberInfo)
{
	SPartyMember *pMember = GetMember(rkMemberInfo.kName);
	if( !pMember )
	{
		pMember = m_kMemberPool.New();
		if( pMember )
		{
			*pMember = rkMemberInfo;
			m_Members.push_back(pMember);
		}
	}

	PgPilot	*pkPilot = g_kPilotMan.GetPlayerPilot();
	if( !pkPilot )
	{
		return;
	}
	std::wstring const MyName = pkPilot->GetName();
	//BM::GUID const &rkMyGuid = pkPilot->GetGuid();

	PgClientPartyUtil::AttachPartyCircle(rkMemberInfo.kCharGuid);
	PgClientPartyUtil::SetCharacterPartyGuid(rkMemberInfo.kCharGuid, PartyGuid());

	g_kChatMgrClient.Name2Guid_Add( CT_PARTY, rkMemberInfo.kName, rkMemberInfo.kCharGuid );
}

//	멤버를 파티에서 탈퇴시킨다.
void PgClientParty::RemoveMember( BM::GUID const &rkCharGuid, std::wstring *pkOutName )
{	//	만약 탈퇴되는 멤버가 자신이라면, 파티 이름을 "" 으로 설정하여, 파티에 가입된 상태가 아님을 설정
//	PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
//	if( !pPilot )
//	{
//		return;
//	}

//	std::wstring const MyName = pPilot->GetName();
//	BM::GUID const &rkMyGuid = pPilot->GetGuid();

	PgClientPartyUtil::DetachPartyCircle(rkCharGuid);
	PgClientPartyUtil::SetCharacterPartyGuid(rkCharGuid, BM::GUID::NullData());
	g_kUIScene.RemoveMiniMapIcon(rkCharGuid);

	ContPartyMember::iterator itor = m_Members.begin();
	while(m_Members.end() != itor)
	{
		ContPartyMember::value_type kElement = (*itor);
		if( kElement->kCharGuid == rkCharGuid )
		{
			if ( pkOutName )
			{
				*pkOutName = kElement->kName;
			}

			m_kMemberPool.Delete(kElement);
			itor = m_Members.erase(itor);
		}
		else
		{
			++itor;
		}
	}
}

//	파티 멤버를 초기화
void PgClientParty::Clear(bool const bLeave)
{
	BM::CAutoMutex kLock(m_kMutex);
	bool bLeaveMessage = false;
	for(ContPartyMember::iterator itor = m_Members.begin(); itor != m_Members.end(); itor++)
	{
		SPartyMember *pkMember = *itor;
		if( pkMember )
		{
			if( bLeave )
			{
				if( g_kPilotMan.IsMyPlayer(pkMember->kCharGuid) )
				{
					PgClientPartyUtil::SetCharacterPartyGuid(pkMember->kCharGuid, BM::GUID::NullData());
					bLeaveMessage = true;
				}
				else
				{
					g_kUIScene.RemoveMiniMapIcon(pkMember->kCharGuid);
				}
				PgClientPartyUtil::DetachPartyCircle(pkMember->kCharGuid);
			}
			m_kMemberPool.Delete(pkMember);
		}
	}

	m_Members.clear();
	m_kPartyGuid.Clear();
	m_kMasterGuid.Clear();
	m_kRejectedUser.clear();

	if( bLeave )
	{
		if ( bLeaveMessage )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddMessage(PMS_Leave, kChatLog, true);
			}
		}
		PgClientPartyUtil::UpdatePartyWndAll();	
		XUIMgr.Close( PgHardCoreDungeon::ms_SFRM_HCD_VOTE );
	}
	else
	{
		ClearCache();
	}

	g_kChatMgrClient.Name2Guid_Remove(CT_PARTY);
}

void PgClientParty::ClearCache()
{
	BM::CAutoMutex kLock(m_kMutex);
	m_kWaiter.Clear();
	AddSelfPartyName();
}

void PgClientParty::ClearOtherPartyName()
{
	BM::CAutoMutex kLock(m_kMutex);
	m_kOtherPartyName.clear();
}

void PgClientParty::UpdatePartyMiniMapIcon()
{
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkMyPlayer )
	{
		return;
	}

	ContPartyMember::const_iterator mem_it = m_Members.begin();
	while(mem_it != m_Members.end())
	{
		if(pkMyPlayer->GroundKey() != (*mem_it)->kChnGndKey)
		{
			g_kUIScene.RemoveMiniMapIcon((*mem_it)->kCharGuid);
		}

		++mem_it;
	}
}

//	가입 요청을 받은 사람에게, 가입 요청 받았음을 표시해주고, 수락,거부 응답을 기다린다.
void PgClientParty::Receive_Party_Join_Request(BM::Stream& rkPacket)
{
	std::wstring kMasterName, kPartyName;
	BM::GUID kMasterGuid, kPartyGuid;
	unsigned short sLevel = 0;
	int iClass = 0;

	rkPacket.Pop(kMasterName);
	rkPacket.Pop(kPartyGuid);
	rkPacket.Pop(kMasterGuid);
	rkPacket.Pop(kPartyName);
	rkPacket.Pop(sLevel);
	rkPacket.Pop(iClass);

	if( 0 != g_kGlobalOption.GetValue("ETC", "DENY_PARTY") )//자동 파티 거절 이면
	{
		if( PgClientExpeditionUtil::IsInExpedition() == false )
		{
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddMessage(700116, kChatLog, true, 1);
		}

		PgClientPartyUtil::Net_C_N_ANS_JOIN_PARTY(false, kPartyGuid);
		return;
	}

	if( kPartyName.empty() )
	{
		kPartyName = GeneratePartyName(kPartyGuid);
	}

	//	수락,거부 인터페이스 띄우자.
//	BM::vstring kMessage( TTW(700070) );
//	kMessage.Replace(L"$CHAR_NAME$", kMasterName);
//	kMessage.Replace(L"$LEVEL$", sLevel);
//	kMessage.Replace(L"$CLASS$", PgClientPartyUtil::GetClassNameTTW(iClass));
//	kMessage.Replace(L"$PARTY_NAME$", kPartyName);
//	CallYesNoMsgBox(kMessage, kPartyGuid, MBT_PARTY_JOIN);

	BM::vstring kMessage( TTW(401317) );
	kMessage.Replace(L"$CHAR_NAME$", kMasterName);
	kMessage.Replace(L"$LEVEL$", sLevel);
	kMessage.Replace(L"$CLASS$", PgClientPartyUtil::GetClassNameTTW(iClass));
	kMessage.Replace(L"$PARTY_NAME$", kPartyName);
	BM::vstring kNowMessage(kMessage);
	kNowMessage.Replace(L"$REMAIN_SEC$", 15);
	CallYesNoMsgBoxLimit(kNowMessage, kPartyGuid, MBT_PARTY_JOIN, kMessage);
}

void PgClientParty::Receive_PartyFind_Join_Request(BM::Stream& rkPacket)
{
	std::wstring kUserName, kPartyName;
	BM::GUID kUserGuid, kPartyGuid;
	unsigned short sLevel = 0;
	int iClass = 0;

	rkPacket.Pop(kUserName);
	rkPacket.Pop(kPartyGuid);
	rkPacket.Pop(kUserGuid);
	rkPacket.Pop(kPartyName);
	rkPacket.Pop(sLevel);
	rkPacket.Pop(iClass);

	if( kPartyName.empty() )
	{
		kPartyName = GeneratePartyName(kPartyGuid);
	}

	//	수락,거부 인터페이스 띄우자.
	//CallYesNoMsgBox(_T("(") + kUserName + _T(")") + TTW(PMS_PartyInvitePop), kUserGuid, MBT_PARTYFIND_JOIN);

	BM::vstring kMessage( TTW(401317) );
	kMessage.Replace(L"$CHAR_NAME$", kUserName);
	kMessage.Replace(L"$LEVEL$", sLevel);
	kMessage.Replace(L"$CLASS$", PgClientPartyUtil::GetClassNameTTW(iClass));
	kMessage.Replace(L"$PARTY_NAME$", kPartyName);
	BM::vstring kNowMessage(kMessage);
	kNowMessage.Replace(L"$REMAIN_SEC$", 15);
	CallYesNoMsgBoxLimit(kNowMessage, kUserGuid, MBT_PARTYFIND_JOIN, kMessage);
}


void PgClientParty::ReqOtherPartyInfo(BM::GUID const &rkPartyGuid, BM::GUID const &rkCharGuid)
{
	BM::CAutoMutex kLock(m_kMutex);
	if( GUID_NULL == rkPartyGuid
	||	GUID_NULL == rkCharGuid )
	{
		return;
	}

	ContOtherPartyName::iterator name_iter = m_kOtherPartyName.find(rkPartyGuid);
	if( m_kOtherPartyName.end() != name_iter )
	{
		return;
	}

	bool const bAddRet = m_kWaiter.AddWaiter(rkPartyGuid, rkCharGuid);
	if( bAddRet )
	{
		BM::CAutoMutex kLock(m_kMutex);
		BM::Stream kPacket(PT_C_M_REQ_PARTY_NAME);
		kPacket.Push(rkPartyGuid);
		NETWORK_SEND(kPacket)
	}
}

bool PgClientParty::GetPartyName(BM::GUID const &rkPartyGuid, SClientPartyName& rkOut) const
{
	BM::CAutoMutex kLock(m_kMutex);
	ContOtherPartyName::const_iterator name_iter = m_kOtherPartyName.find(rkPartyGuid);
	if( m_kOtherPartyName.end() != name_iter )
	{
		rkOut = (*name_iter).second;
		return true;
	}
	return false;
}

bool PgClientParty::GetPartyName(BM::GUID const &rkPartyGuid, BM::GUID const& rkCharGuid, SClientPartyName& rkOut)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContOtherPartyName::iterator name_iter = m_kOtherPartyName.find(rkPartyGuid);
	if( m_kOtherPartyName.end() != name_iter )
	{
		ContGuidSet &rkSet = (*name_iter).second.kGuidSet;
		if( rkSet.end() == rkSet.find(rkCharGuid) )
		{
			rkSet.insert(rkCharGuid);
		}

		rkOut = (*name_iter).second;
		return true;
	}
	return false;
}

void PgClientParty::RemovePartyName(BM::GUID const& rkPartyGuid, BM::GUID const& rkCharGuid)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContOtherPartyName::iterator name_iter = m_kOtherPartyName.find(rkPartyGuid);
	if( m_kOtherPartyName.end() != name_iter )
	{
		ContGuidSet &rkSet = (*name_iter).second.kGuidSet;
		ContGuidSet::iterator find_iter = rkSet.find(rkCharGuid);
		if( rkSet.end() != find_iter )
		{
			rkSet.erase(find_iter);
		}

		if( rkSet.empty() )
		{
			m_kOtherPartyName.erase(name_iter);
		}
	}
}

void PgClientParty::Recv_PT_M_C_ANS_PARTY_NAME(BM::Stream& rkPacket)
{
	BM::GUID kPartyGuid;
	SClientPartyName kNewName;

	kNewName.ReadFromPacket(rkPacket);
	AddPartyName(kNewName);
}

void PgClientParty::Recv_PT_M_C_ANS_PARTY_LIST(BM::Stream& rkPacket)
{	
	BM::CAutoMutex kLock(m_kMutex);

	BM::GUID PartyListGuid;
	short TotalChannelCount = 0;
	short sChannelNo = 0;
	BYTE cRet = 0;
	int iSize = 0;

	rkPacket.Pop(PartyListGuid);
	rkPacket.Pop(m_kTotalChannelCount);
	rkPacket.Pop(sChannelNo);

	rkPacket.Pop((BYTE)cRet);
	rkPacket.Pop(iSize);

	if( m_kPartyListGuid != PartyListGuid )
	{// 새로운 리스트들이 들어올 때
		m_kPartyListChannelCount = 1;
		m_kPartyListName.clear();
		m_kPartyListGuid = PartyListGuid;
		
		// 파티 리스트가 처음 도착하고 3초 뒤까지 모든 채널의 파티정보가 도착하지 않으면
		// 현재 있는 리스트 정보만으로 갱신한다.
		lua_tinker::call< void, bool >("StartRecvPartyList", false);
	}
	else
	{// 현재 요청한 리스트의 잔여 목록이 이어서 들어올 때
		++m_kPartyListChannelCount;
	}
	
	while ( iSize-- )
	{		
		SPartyListInfo kItemSlot;

		kItemSlot.Clear();
		kItemSlot.ReadFromPacket(rkPacket);
		kItemSlot.ChannelNo = sChannelNo; // 채널정보 저장

		std::pair<ContPartyListName::iterator, bool> kRet = m_kPartyListName.insert(std::make_pair(kItemSlot.kPartyGuid, kItemSlot));
		if( !kRet.second )//중복
		{
			ContPartyListName::iterator iter = kRet.first;
			if( iter != m_kPartyListName.end() )
			{
				(*iter).second = kItemSlot;
			}
		}
	}

	if( m_kPartyListChannelCount == m_kTotalChannelCount )
	{
		lua_tinker::call< void, bool >("StartRecvPartyList", true); // 모든 채널의 리스트가 도착했다.

		if( m_kPartyListName.empty() )
		{
			std::wstring const rkMsg = TTW(700125);
			
			if( rkMsg.empty())
			{
				return;
			}

			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(kChatLog, rkMsg);
			lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(rkMsg), true);

			int Attribute = lua_tinker::call< int >("GetSelectedListAttribute");
			NowPage(0);
			PartyListViewRefresh(0, Attribute);

			return;
		}
		else
		{
			ArrangePartyList( m_kPartyListName );
			SetPartyListAttribute( m_kPartyListName );
		}
	}
	else if( m_kPartyListChannelCount > m_kTotalChannelCount )
	{
		return;
	}

	char const *pszText = "SFRM_PARTY_PART_FIND";

	std::wstring const wstr = UNI(pszText);
	XUI::CXUI_Wnd* pWnd = NULL;
	if(!XUIMgr.IsActivate(wstr, pWnd))
	{
		lwCallUI("SFRM_PARTY_PART_FIND", false);
	}

	int Attribute = lua_tinker::call< int >("GetSelectedListAttribute");
	NowPage(0);
	PartyListViewRefresh(0, Attribute);
}

int PgClientParty::PartyListMaxPage()
{
	int iAddPage = ( m_kPartyListName.size() > PV_MAX_PARTY_VIEW_LIST_CNT ) ? 1 : 0;
	int iPage = std::max<int>((m_kPartyListName.size()/PV_MAX_PARTY_VIEW_LIST_CNT), 1) + iAddPage;
	if( PV_MAX_PARTY_VIEW_LIST_CNT < iPage )
	{
		iPage = PV_MAX_PARTY_VIEW_LIST_CNT;
	}
	return iPage;
}

int PgClientParty::PartyListMaxPage(ContTempPartyListName const& PartyListName)
{
	int iAddPage = ( PartyListName.size() > PV_MAX_PARTY_VIEW_LIST_CNT ) ? 1 : 0;
	int iPage = std::max<int>((PartyListName.size()/PV_MAX_PARTY_VIEW_LIST_CNT), 1) + iAddPage;
	if( PV_MAX_PARTY_VIEW_LIST_CNT < iPage )
	{
		iPage = PV_MAX_PARTY_VIEW_LIST_CNT;
	}
	return iPage;
}

int PgClientParty::PartyFindPeopleListMaxPage()
{
	int iAddPage = ( m_kFindPartyUserList.size() > PV_MAX_PARTY_VIEW_LIST_CNT ) ? 1 : 0;
	int iPage = std::max<int>((m_kFindPartyUserList.size() / PV_MAX_PARTY_VIEW_LIST_CNT), 1) + iAddPage;
	if( PV_MAX_PARTY_VIEW_LIST_CNT < iPage )
	{
		iPage = PV_MAX_PARTY_VIEW_LIST_CNT;
	}

	return iPage;
}

int PgClientParty::PartyListNowPage()
{
	return NowPage();
}

int PgClientParty::PartyFindPeopleListNowPage()
{
	return FindUserNowPage();
}

void PgClientParty::SetPartyListInit()
{
	m_kPartyListName.clear();	
}

int PgClientParty::GetAttributeValue(int iAttribute)
{
	int iTTW = 0;
	switch( iAttribute )
	{
	case POA_All:
		{
			iTTW = 401343;
		}break;
	case POA_Field:
		{
			iTTW = 401344;
		}break;
	case POA_Mission:
		{
			iTTW = 401345;
		}break;
	case POA_Indun:
		{
			iTTW = 401346;
		}break;
	case POA_Hidden:
		{
			iTTW = 401347;
		}break;
	case POA_Chaos:
		{
			iTTW = 401348;
		}break;
	default:
		{
		}break;
	}
	return iTTW;
}

void PgClientParty::FakePartyListViewRefresh(int iNowPage, int iAttribute)
{
	m_kPartyListChannelCount = m_kTotalChannelCount;

	char const *pszText = "SFRM_PARTY_PART_FIND";

	std::wstring const wstr = UNI(pszText);
	XUI::CXUI_Wnd* pWnd = NULL;
	if(!XUIMgr.IsActivate(wstr, pWnd))
	{
		lwCallUI("SFRM_PARTY_PART_FIND", false);
	}

	NowPage(iNowPage);
	PartyListViewRefresh(iNowPage, iAttribute);
}

void PgClientParty::PartyListViewRefresh(int iNowPage, int iAttribute)
{
	if( m_kPartyListChannelCount != m_kTotalChannelCount )
	{
		return;
	}

	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_PARTY_PART_FIND"));
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pkTopFom = pkTopWnd->GetControl(_T("FRM_ITEM"));
	if( !pkTopFom )
	{
		return;
	}

	XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkTopFom->GetControl(_T("LST_PARTY_PART_LIST")));
	if( !pkListWnd )
	{
		return;
	}

	XUI::CXUI_Form* pkPageWnd = dynamic_cast<XUI::CXUI_Form*>(pkTopWnd->GetControl(_T("FRM_PAGE")));
	if( !pkPageWnd )
	{
		return;
	}

	ContTempPartyListName TempPartyListName;
	ContPartyListName::const_iterator iter = m_kPartyListName.begin();
	while (m_kPartyListName.end() != iter)
	{
		if( (0 == iAttribute) || (iter->second.iPartyAttribute == iAttribute) )
		{
			TempPartyListName.push_back(iter->second);
		}
		++iter;
	}

	PartyListNewSort( TempPartyListName );

	if( (iNowPage+1) > PartyListMaxPage(TempPartyListName) )
	{
		return;
	}
	if( 1 > (iNowPage+1) )
	{
		return;
	}

	BM::vstring kCur(iNowPage + 1);
	BM::vstring kMax(PartyListMaxPage(TempPartyListName));
	kCur += _T("/");
	kCur += kMax;
	pkPageWnd->Text(kCur);	

	pkListWnd->ClearList();
	NowPage(iNowPage);

	XUI::SListItem* pItem = pkListWnd->FirstItem();
	int iCustomCount = 1;
	int iNowPreview = (iNowPage * PV_MAX_PARTY_VIEW_LIST_CNT);

	ContTempPartyListName::const_iterator itr = TempPartyListName.begin();
	while (TempPartyListName.end() != itr)
	{		
		if( 0 < iNowPreview )
		{
			--iNowPreview;
			++itr;
			continue;
		}

		if( !pItem )
		{
			pItem = pkListWnd->AddItem(_T(""),0);
		}

		if(	pItem )
		{
			XUI::CXUI_Form *pForm = (XUI::CXUI_Form*)pItem->m_pWnd;

			if(pForm)
			{		
				//BM::GUID const &rkPartyGuid = (*itr).second.kPartyGuid;
				BM::GUID const &rkMasterGuid = (*itr).kMasterGuid;

				if( !rkMasterGuid ) 
				{
					++itr;
					continue;
				}

				pForm->OwnerGuid(rkMasterGuid);//캐릭GUID
				pForm->SetCustomData(&iCustomCount, sizeof(iCustomCount));

				XUI::CXUI_Form *pkNameForm = (XUI::CXUI_Form*)pForm->GetControl(std::wstring(_T("FRM_NAME")));
				if( pkNameForm )
				{
					std::wstring kPartyName = _T("");

					if( (*itr).kPartyName.empty() )
					{
						kPartyName = GeneratePartyName( (*itr).kPartyGuid );
					}
					else
					{
						kPartyName = (*itr).kPartyName;
					}
					SetTextToFitWindowSize(pkNameForm, kPartyName, 0, 20);
				}

				XUI::CXUI_Wnd *pkArea = pForm->GetControl(std::wstring(_T("FRM_AREA")));
				if( pkArea )
				{
					int MasterMapNo = (*itr).iMasterMapNo;
					pkArea->SetCustomData(&MasterMapNo, sizeof(MasterMapNo));

					BM::vstring vStr;
					short const ChannelNo = g_kNetwork.NowChannelNo();
					if( ChannelNo != (*itr).ChannelNo )
					{
						if( EXPEDITION_CHANNEL_NO == (*itr).ChannelNo )
						{
							vStr += L"[";
							vStr += TTW(710037);
							vStr += L"]";
						}
						else
						{
							BM::vstring Temp(TTW(401273));
							Temp.Replace(L"#NO#", (*itr).ChannelNo);
							vStr = Temp;
						}
					}
					CONT_DEFMAP const * pkContDefMap = NULL;
					g_kTblDataMgr.GetContDef(pkContDefMap);
					if( NULL != pkContDefMap )
					{
						std::wstring const* pMapName = NULL;
						std::wstring MapName;
						CONT_DEFMAP::const_iterator map_iter = pkContDefMap->find((*itr).iMasterMapNo);
						if( pkContDefMap->end() == map_iter )
						{
							GetDefString((*itr).iMasterMapNo, pMapName);
						}
						else
						{
							TBL_DEF_MAP const& rkDefMap = (*map_iter).second;
							if( rkDefMap.iGroupTextNo )
							{
								MapName = TTW(rkDefMap.iGroupTextNo);
							}
							else
							{
								GetDefString(rkDefMap.NameNo, pMapName);
							}
						}

						if( pkContDefMap->end() != map_iter )
						{
							if( GATTR_MISSION == map_iter->second.iAttr )
							{
								vStr += L"[";
								vStr += TTW(420059);
								vStr += L"]";
							}
							else if( GATTR_CHAOS_MISSION == map_iter->second.iAttr )
							{
								vStr += L"[";
								vStr += TTW(420060);
								vStr += L"]";
							}
						}

						if( NULL != pMapName )
						{
							vStr += *pMapName;
						}
						if( MapName.size() )
						{
							vStr += MapName;
						}
					}
					SetTextToFitWindowSize(pkArea, (std::wstring)vStr, 0, 40);
				}

				XUI::CXUI_Wnd *pkAreaMark = pForm->GetControl(std::wstring(_T("FRM_AREA_MARK")));
				if( pkAreaMark )
				{
					int iAtt = (*itr).iPartyAttribute;
					iAtt = GetAttributeValue(iAtt);
					std::wstring kAreaMake = TTW(iAtt);
					pkAreaMark->Text(kAreaMake);
				}

				XUI::CXUI_Wnd* pkStatus01 = pForm->GetControl(_T("FRM_STATUS_ANI01"));
				XUI::CXUI_Wnd* pkStatus02 = pForm->GetControl(_T("FRM_STATUS_ANI02"));
				XUI::CXUI_Wnd* pkStatus03 = pForm->GetControl(_T("FRM_STATUS_ANI03"));
				if( pkStatus01 && pkStatus02 && pkStatus03 )
				{
					pkStatus01->Visible(false);
					pkStatus02->Visible(false);
					pkStatus03->Visible(false);
					switch( (*itr).cStatus )
					{
					case EPS_RUNNING:
						{
							pkStatus01->Visible(true);
						}break;
					case EPS_WAITING:
						{
							pkStatus03->Visible(true);
						}break;
					case EPS_REFUSE:
						{
							pkStatus02->Visible(true);
						}break;
					default:
						{
							pkStatus02->Visible(true);
						}break;
					}
				}

				XUI::CXUI_Wnd *pkMember = pForm->GetControl(std::wstring(_T("FRM_MEMBER")));
				if( pkMember )
				{					
					BM::vstring kCur((*itr).cCurMember);
					BM::vstring kMax((*itr).cMaxMember);

					kCur += _T("/");
					kCur += kMax;

					pkMember->Text(kCur);
				}	
			}
		}
		++iCustomCount;
		++itr;
		pItem = pkListWnd->NextItem(pItem);

		if( iCustomCount > PV_MAX_PARTY_VIEW_LIST_CNT ) 
		{
			break;
		}
	}
	while(pItem)
	{
		pItem = pkListWnd->DeleteItem(pItem);
	}
}

void PgClientParty::FindPartyUserListViewRefresh(int iNowPage, int iAttribute)
{
	if( (iNowPage + 1) > PartyFindPeopleListMaxPage() )
	{
		return;
	}
	if( 1 > (iNowPage + 1) )
	{
		return;
	}

	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_PARTY_PEOPLE_FIND"));
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pkTopFom = pkTopWnd->GetControl(_T("FRM_ITEM"));
	if( !pkTopFom )
	{
		return;
	}

	XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkTopFom->GetControl(_T("LST_PARTY_PEOPLE_LIST")));
	if( !pkListWnd )
	{
		return;
	}

	XUI::CXUI_Form* pkPageWnd = dynamic_cast<XUI::CXUI_Form*>(pkTopFom->GetControl(_T("FRM_PAGE")));
	if( !pkPageWnd )
	{
		return;
	}

	ContTempFindPartyUserList TempFindPartyUserList;
	TempFindPartyUserList.clear();
	ContFindPartyUserList::const_iterator iter = m_kFindPartyUserList.begin();
	while (m_kFindPartyUserList.end() != iter)
	{
		TempFindPartyUserList.push_back(iter->second);
		++iter;
	}

	TempFindPartyUserList.sort(PgClientPartyUtil::FindPartyUserListSort);

	BM::vstring kCur(iNowPage + 1);
	BM::vstring kMax(PartyFindPeopleListMaxPage());
	kCur += _T("/");
	kCur += kMax;
	pkPageWnd->Text(kCur);	

	pkListWnd->ClearList();
	FindUserNowPage(iNowPage);

	XUI::SListItem* pItem = pkListWnd->FirstItem();
	int iCustomCount = 1;
	int iNowPreview = (iNowPage * PV_MAX_PARTY_VIEW_LIST_CNT);
	
	ContTempFindPartyUserList::const_iterator itr = TempFindPartyUserList.begin();
	while (TempFindPartyUserList.end() != itr)
	{		
		if( 0 < iNowPreview )
		{
			--iNowPreview;
			++itr;
			continue;
		}

		if( !pItem )
		{
			pItem = pkListWnd->AddItem(_T(""),0);
		}

		if(	pItem )
		{
			XUI::CXUI_Form *pForm = (XUI::CXUI_Form*)pItem->m_pWnd;

			if(pForm)
			{
				BM::GUID const &rkCharGuid = (*itr).kCharGuid;

				if( !rkCharGuid ) 
				{
					++itr;
					continue;
				}

				pForm->OwnerGuid(rkCharGuid);//캐릭GUID
				pForm->SetCustomData(&iCustomCount, sizeof(iCustomCount));

				XUI::CXUI_Form *pkWordForm = (XUI::CXUI_Form*)pForm->GetControl(std::wstring(_T("FRM_WORD")));
				if( pkWordForm )
				{
					std::wstring kWord(_T(""));

			//		if( (*itr).second.kContents.empty() )
			//		{
			//			kWord = GeneratePartyName( (*itr).second.kPartyGuid );
			//		}
			//		else
			//		{
						kWord = (*itr).kContents;
			//		}

					if( 40 < kWord.size() )
					{
						kWord = kWord.substr(0, 8) + _T("...");
					}

					pkWordForm->Text(kWord);//한마디
				}

				XUI::CXUI_Wnd *pkLevel = pForm->GetControl(std::wstring(_T("FRM_LEVEL")));
				if( pkLevel )
				{
					char tmp[100]={0,};
					sprintf(tmp, "%dLV", (*itr).sLevel);
					std::wstring kLevel(UNI(tmp));
					pkLevel->Text(kLevel);
				}

				XUI::CXUI_Wnd *pkClass = pForm->GetControl(std::wstring(_T("FRM_CLASS")));
				if( pkClass )
				{// 직업 아이콘 설정
					XUI::CXUI_Wnd *pkClassMark = pkClass->GetControl( std::wstring(L"IMG_CLASS") );
					lwSetMiniClassIconIndex( pkClassMark,  (*itr).iOriginalClass);
				}
			}
		}
		++iCustomCount;
		++itr;
		pItem = pkListWnd->NextItem(pItem);

		if( iCustomCount > PV_MAX_PARTY_VIEW_LIST_CNT ) 
		{
			break;
		}
	}
	while(pItem)
	{
		pItem = pkListWnd->DeleteItem(pItem);
	}
}

void PgClientParty::AddPartyName(SClientPartyName const& rkPartyName)
{
	BM::CAutoMutex kLock(m_kMutex);

	BM::GUID kOldMasterGuid;

	std::pair<ContOtherPartyName::iterator, bool> kRet = m_kOtherPartyName.insert( std::make_pair(rkPartyName.kPartyGuid, rkPartyName) );
	if( !kRet.second ) // 이미 있음
	{
		ContOtherPartyName::iterator iter = kRet.first;
		SClientPartyName& rkFindPartyName = (*iter).second;

		if( !rkPartyName.kPartyName.empty()
		&&	!rkFindPartyName.kPartyName.empty()
		&&	rkFindPartyName.kPartyName != rkPartyName.kPartyName
		&&	rkPartyName.kPartyGuid == PartyGuid() )
		{
			PgClientPartyUtil::PartyResultMsg(PRC_Success_ChangeName);
		}

		kOldMasterGuid = rkFindPartyName.kMasterGuid;
		rkFindPartyName.SetOnlyInfo(rkPartyName); // 덮어 씌움 (GuidVec만 빼고)
	}
	else
	{
		if( !rkPartyName.kPartyName.empty()
		&&	PartyName() != rkPartyName.kPartyName
		&& rkPartyName.kPartyGuid == PartyGuid() )
		{
			PgClientPartyUtil::PartyResultMsg(PRC_Success_ChangeName);
		}
		kOldMasterGuid = rkPartyName.kMasterGuid;
	}

	if( rkPartyName.kPartyGuid == m_kPartyGuid )
	{
		m_kPartyName = rkPartyName.kPartyName;
		m_kMasterGuid = rkPartyName.kMasterGuid;
		if( rkPartyName.kPartyName.empty() )
		{
			m_kPartyName = GeneratePartyName(m_kPartyGuid);
		}
		PgClientPartyUtil::UpdatePartyWndAll();
	}
	
	if( kOldMasterGuid != rkPartyName.kMasterGuid )
	{
		UpdatePartyBalloon(kOldMasterGuid);
	}
	UpdatePartyBalloon(rkPartyName.kMasterGuid);

	ContInfoWaiter::ContWaiter kVec;
	bool const bRet = m_kWaiter.GetWaiter(rkPartyName.kPartyGuid, kVec);
	if( !bRet )
	{
		return;
	}
	m_kWaiter.DelWaiter(rkPartyName.kPartyGuid);

	ContInfoWaiter::ContWaiter::iterator wait_iter = kVec.begin();
	while(kVec.end() != wait_iter)
	{
		ContInfoWaiter::ContWaiter::value_type const& rkCharGuid = (*wait_iter);
		UpdatePartyBalloon(rkCharGuid);
		++wait_iter;
	}
}

void PgClientParty::UpdatePartyBalloon(BM::GUID const& rkCharGuid)
{
	if( !g_pkWorld ) return;

	if( BM::GUID::IsNotNull(rkCharGuid) )
	{
		PgActor *pkActor = dynamic_cast<PgActor*>(g_pkWorld->FindObject(rkCharGuid));
		if( pkActor )
		{
			pkActor->UpdateName();
		}
	}
}

void PgClientParty::AddSelfPartyName()
{
	if( BM::GUID::NullData() == m_kPartyGuid )
	{
		return;
	}

	if( m_kPartyName.empty() )
	{
		m_kPartyName = GeneratePartyName(m_kPartyGuid);
	}

	SClientPartyName kNewName;
	kNewName.kPartyGuid = m_kPartyGuid;
	kNewName.kPartyName = m_kPartyName;
	kNewName.cCurMember = (BYTE)m_Members.size();
	kNewName.cMaxMember = (BYTE)Option().GetOptionMaxMember();
	kNewName.kMasterGuid = m_kMasterGuid;
	kNewName.kPartyOption = Option();

	AddPartyName(kNewName);
}

std::wstring const& PgClientParty::GeneratePartyName(BM::GUID const &rkPartyGuid)
{
	BM::CAutoMutex kLock(m_kMutex);
	size_t const iPartyID = BM::CGuid_hash_compare()(rkPartyGuid);

	if( m_kReservePartyName.empty() )
	{
		static std::wstring kEmptyName = _T("New Party");
		return kEmptyName;
	}

	size_t iCurrent = iPartyID % m_kReservePartyName.size();
	if( m_kReservePartyName.size() <= iCurrent )
	{
		iCurrent = 0;
	}

	return m_kReservePartyName.at(iCurrent);
}

bool PgClientParty::IsReserveName(std::wstring const &rkNewPartyName)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContReserveName::iterator name_iter = std::find(m_kReservePartyName.begin(), m_kReservePartyName.end(), rkNewPartyName);
	if(m_kReservePartyName.end() != name_iter)
	{
		return true;
	}
	return false;
}

bool PgClientParty::CheckLastInviteTime(std::wstring const &rkCharName)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContRejectedUser::iterator iter = m_kRejectedUser.begin();
	while(m_kRejectedUser.end() != iter)
	{
		const ContRejectedUser::value_type& rkElement = (*iter);
		if( rkElement.kCharName == rkCharName )
		{
			if(	5.0f > (g_pkApp->GetAccumTime() - rkElement.fRejectedTime) )
			{
				if( PgClientExpeditionUtil::IsInExpedition() == false )
				{
					SChatLog kChatLog(CT_EVENT_SYSTEM);
					g_kChatMgrClient.AddMessage(700113, kChatLog, true);
				}
				return false;
			}
			else
			{
				iter = m_kRejectedUser.erase(iter);
				continue;
			}
		}
		++iter;
	}
	return true;
}

bool PgClientParty::CheckLastInviteTime(BM::GUID const &rkCharGuid)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContRejectedUser::iterator iter = m_kRejectedUser.begin();
	while(m_kRejectedUser.end() != iter)
	{
		const ContRejectedUser::value_type& rkElement = (*iter);
		if( rkElement.kCharGuid == rkCharGuid )
		{
			if(	5.0f > (g_pkApp->GetAccumTime() - rkElement.fRejectedTime) )
			{
				if( PgClientExpeditionUtil::IsInExpedition() == false )
				{
					SChatLog kChatLog(CT_EVENT_SYSTEM);
					g_kChatMgrClient.AddMessage(700113, kChatLog, true);
				}
				return false;
			}
			else
			{
				iter = m_kRejectedUser.erase(iter);
				continue;
			}
		}
		++iter;
	}
	return true;
}

void PgClientParty::AddCheckInvietTime(ContRejectedUser::value_type& rkNewRejectedUser)
{
	BM::CAutoMutex kLock(m_kMutex);
	rkNewRejectedUser.fRejectedTime = g_pkApp->GetAccumTime();
	m_kRejectedUser.push_back(rkNewRejectedUser);
}

bool PgClientParty::ProcessMsg(unsigned short const usType, BM::Stream& rkPacket)
{
	BM::CAutoMutex kLock(m_kMutex);
	switch( usType )
	{
	case PT_N_C_NFY_CREATE_PARTY:			{ Recv_PT_N_C_NFY_CREATE_PARTY(rkPacket); }break;
	case PT_N_C_REQ_JOIN_PARTY:				{ Receive_Party_Join_Request(rkPacket); }break;
	case PT_N_C_REQ_JOIN_PARTYFIND:			{ Receive_PartyFind_Join_Request(rkPacket); }break;
	case PT_N_C_ANS_JOIN_PARTY_TO_MASTER:	{ Recv_PT_N_C_ANS_JOIN_PARTY_TO_MASTER(rkPacket); }break;
	case PT_N_C_NFY_JOIN_PARTY:				{ Recv_PT_N_C_NFY_JOIN_PARTY(rkPacket); }break;
	case PT_N_C_ANS_JOIN_PARTY:				{ Recv_PT_N_C_ANS_JOIN_PARTY(rkPacket); }break;
	case PT_N_C_NFY_LEAVE_PARTY_USER:		{ Recv_PT_N_C_NFY_LEAVE_PARTY_USER(rkPacket); }break;
	case PT_N_C_NFY_PARTY_USER_MAP_MOVE:	{ Recv_PT_N_C_NFY_PARTY_USER_MAP_MOVE(rkPacket); }break;
	case PT_N_C_NFY_KICKOUT_PARTY_USER:		{ Recv_PT_N_C_NFY_KICKOUT_PARTY_USER(rkPacket); }break;
	case PT_N_C_NFY_PARTY_DESTROY:			{ Recv_PT_N_C_NFY_PARTY_DESTROY(rkPacket); }break;
	case PT_N_C_ANS_PARTY_CHANGE_MASTER:	{ Recv_PT_N_C_ANS_PARTY_CHANGE_MASTER(rkPacket); }break;
	case PT_N_C_ANS_PARTY_CHANGE_OPTION:	{ Recv_PT_N_C_ANS_PARTY_CHANGE_OPTION(rkPacket); }break;
	case PT_M_C_NFY_PARTY_MEMBER_ITEM:		{ Recv_PT_M_C_NFY_PARTY_MEMBER_ITEM(rkPacket); }break;
	case PT_M_C_ANS_PARTY_NAME:				{ Recv_PT_M_C_ANS_PARTY_NAME(rkPacket); }break;
	case PT_N_C_ANS_PARTY_RENAME:			{ Recv_PT_N_C_ANS_PARTY_RENAME(rkPacket); }break;
	case PT_M_C_NFY_PARTY_JOIN:				{ Recv_PT_M_C_NFY_PARTY_JOIN(rkPacket); }break;
	case PT_M_C_ANS_PARTY_LIST:				{ Recv_PT_M_C_ANS_PARTY_LIST(rkPacket); }break;
	case PT_M_C_NFY_CHANGE_MEMBER_ABIL:		{ Recv_PT_M_C_NFY_CHANGE_MEMBER_ABIL(rkPacket); }break;
	case PT_T_C_NFY_PARTY_MEMBER_MYHOME:	{ Recv_PT_T_C_NFY_PARTY_MEMBER_MYHOME(rkPacket); }break;
	case PT_M_C_ANS_FIND_PARTY_USER_LIST:	{ Recv_PT_M_C_ANS_FIND_PARTY_USER_LIST(rkPacket); }break;
	case PT_M_C_ANS_REGIST_PRIVATE_PARTY_FIND:	{ Recv_PT_M_C_ANS_REGIST_PRIVATE_PARTY_FIND(rkPacket);	}break;
	case PT_N_C_NFY_UNREGIST_PRIVATE:		{ Recv_PT_N_C_NFY_UNREGIST_PRIVATE(rkPacket); }break;
	case PT_T_C_ANS_OTHER_CHANNEL_PARTY:	{ Recv_PT_T_C_ANS_OTHER_CHANNEL_PARTY(rkPacket); }break;
	case PT_M_C_ANS_JOIN_PARTYFIND_ITEM_CHECK: { Recv_PT_M_C_ANS_JOIN_PARTYFIND_ITEM_CHECK(rkPacket); }break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

void PgClientParty::Recv_PT_N_C_NFY_UNREGIST_PRIVATE(BM::Stream & rkPacket)
{
	lua_tinker::call<void, bool>("Set_PrivateRegist", false);

	SChatLog kChatLog((EChatType)CT_EVENT);
	g_kChatMgrClient.AddMessage(799390, kChatLog, true, 11);
}

void PgClientParty::Recv_PT_N_C_NFY_CREATE_PARTY(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;

	rkPacket.Pop(hRet);

	PgClientPartyUtil::PartyResultMsg(hRet);
}

void PgClientParty::Recv_PT_N_C_ANS_JOIN_PARTY_TO_MASTER(BM::Stream& rkPacket)
{
	HRESULT hResult = 0;
	rkPacket.Pop(hResult);
	//거부 했다.
	//최대 파티원 풀이다.
	//모르겠다 등등..
	PgClientPartyUtil::PartyResultMsg(hResult);

	if( PRC_Fail_Refuse == hResult )
	{
		SRejectedInviteUser kNewRejecteUser;
		rkPacket.Pop(kNewRejecteUser.kCharGuid);
		rkPacket.Pop(kNewRejecteUser.kCharName);
		AddCheckInvietTime(kNewRejecteUser);
	}

	PgClientPartyUtil::UpdatePartyWndAll();
}

void PgClientParty::Recv_PT_N_C_NFY_JOIN_PARTY(BM::Stream& rkPacket)
{
	SPartyUserInfo kMemberInfo;
	BM::GUID kPartyGuid;

	rkPacket.Pop(kPartyGuid);
	kMemberInfo.ReadFromPacket(rkPacket);

	AddMember(kMemberInfo);//Add

	if( PgClientExpeditionUtil::IsInExpedition() == false )
	{
		std::wstring const &kNewName = kMemberInfo.kName+TTW(PMS_Is)+TTW(PMS_Join_Success);
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(kChatLog, kNewName, true);
	}

	PgClientPartyUtil::UpdatePartyWndAll();
}

void PgClientParty::Recv_PT_N_C_ANS_JOIN_PARTY(BM::Stream& rkPacket)
{
	PgPilot	*pkPilot = g_kPilotMan.GetPlayerPilot();
	if(!pkPilot)
	{return;}

	//BM::GUID const &rkMyGuid = pkPilot->GetGuid();

	int iResult = 0;
	BM::GUID kPartyGuid;
	rkPacket.Pop(kPartyGuid);
	rkPacket.Pop(iResult);

	if( PRC_Success != iResult )
	{
		PgClientPartyUtil::PartyResultMsg(iResult);
		return ;
	}

	BYTE cMemberCount = 0;//성공시에만
	Clear();
	m_kPartyGuid = kPartyGuid;

	rkPacket.Pop(cMemberCount);//총 맴버
	for(BYTE i=0; cMemberCount>i; ++i)
	{
		SPartyUserInfo kMemberInfo;
		kMemberInfo.ReadFromPacket(rkPacket);
		AddMember(kMemberInfo);//나 포함
	}

	std::wstring kPartyName;
	BM::GUID kMasterGuid;
	SPartyOption kPartyOption;

	rkPacket.Pop(kPartyName);
	rkPacket.Pop(kMasterGuid);
	kPartyOption.ReadFromPacket(rkPacket);

	m_kPartyName = kPartyName;
	m_kMasterGuid = kMasterGuid;
	m_kOption = kPartyOption;

	int iTTW = PMS_Join_Success;//파티에 성공적으로 가입했다

	g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_PARTY), 0); // 파티 도움말

	if( PgClientExpeditionUtil::IsInExpedition() == false )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(iTTW, kChatLog, true);
	}

	PgClientPartyUtil::UpdatePartyWndAll();

	AddSelfPartyName();

	lwCloseUI("SFRM_PARTY_OPTION");
	lwCloseUI("SFRM_PARTY_PART_FIND");	
	XUIMgr.Activate(L"FRM_PARTY_STATE");
}

void PgClientParty::Recv_PT_N_C_NFY_LEAVE_PARTY_USER(BM::Stream& rkPacket)
{
	BM::GUID kLeaveGuid;
	bool bMaster = false;

	rkPacket.Pop(kLeaveGuid);
	rkPacket.Pop(bMaster);
	RemovePartyName(m_kPartyGuid, kLeaveGuid);

	if( g_kPilotMan.IsMyPlayer(kLeaveGuid) ) // 나
	{
		Clear(true);
		return;
	}
	
	std::wstring kLeaveName;
	RemoveMember( kLeaveGuid, &kLeaveName );

	if( PgClientExpeditionUtil::IsInExpedition() == false )
	{
		SChatLog kChatLog(CT_EVENT);
		std::wstring const &rkMessage = kLeaveName + TTW(PMS_Is) + TTW(PMS_Leave);
		g_kChatMgrClient.AddLogMessage(kChatLog, rkMessage, true);
	}

	if( true == bMaster )
	{
		BM::GUID kNewMasterGuid;
		std::wstring kNewMasterName;

		rkPacket.Pop(kNewMasterName);
		rkPacket.Pop(kNewMasterGuid);
			
		m_kMasterGuid = kNewMasterGuid;
		//AddSelfPartyName();

		if( PgClientExpeditionUtil::IsInExpedition() == false )
		{
			SChatLog kChatLog(CT_EVENT);
			std::wstring const &rkMessage = kNewMasterName+TTW(PMS_Is)+TTW(PMS_NewMaster);
			g_kChatMgrClient.AddLogMessage(kChatLog, rkMessage, true);
		}
	}

	g_kChatMgrClient.Name2Guid_Remove(kLeaveGuid);

	PgClientPartyUtil::UpdatePartyWndAll();
	PgHardCoreDungeon::RemoveMember(kLeaveGuid);
}

void PgClientParty::Recv_PT_N_C_NFY_KICKOUT_PARTY_USER(BM::Stream& rkPacket)
{
	if(!g_pkWorld)
	{
		return;
	}
	BM::GUID kLeaveGuid;

	rkPacket.Pop(kLeaveGuid);

	RemovePartyName(m_kPartyGuid, kLeaveGuid);

	SChatLog kChatLog(CT_EVENT);
	if( g_kPilotMan.IsMyPlayer(kLeaveGuid) )
	{
		// 나
		if( g_pkWorld->IsHaveAttr(GATTR_FLAG_HAVEPARTY) )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				// 대필드로 강제 이동 된다.
				std::wstring const &rkMessage = TTW(PMS_Kickout) + TTW(18);
				g_kChatMgrClient.AddLogMessage(kChatLog, rkMessage, true);
			}
		}
		else
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				g_kChatMgrClient.AddMessage(PMS_Kickout, kChatLog, true);
			}
		}
		Clear(true);
	}
	else
	{
		// 딴 놈
		SPartyMember const *pkMember = NULL;
		if( GetMember(kLeaveGuid, pkMember) )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				std::wstring const& rkMessage = pkMember->kName + TTW(PMS_Is) + TTW(PMS_Kickout);
				g_kChatMgrClient.AddLogMessage(kChatLog, rkMessage, true);
			}
		}
		RemoveMember( kLeaveGuid, NULL );
	}
	PgClientPartyUtil::UpdatePartyWndAll();
	PgHardCoreDungeon::RemoveMember(kLeaveGuid);
}

void PgClientParty::Recv_PT_N_C_NFY_PARTY_DESTROY(BM::Stream& rkPacket)
{
	//클라이언트 파티정보를 삭제 한다.
	BM::GUID kPartyGuid;
	rkPacket.Pop(kPartyGuid);

	Clear();
	PgClientPartyUtil::PartyResultMsg(PRC_Success_Destroy);

	PgClientPartyUtil::UpdatePartyWndAll();
}

void PgClientParty::Recv_PT_N_C_ANS_PARTY_CHANGE_MASTER(BM::Stream& rkPacket)
{
	HRESULT hResult = PRC_None;
	BM::GUID kNewMasterGuid;

	rkPacket.Pop(hResult);

	if( PRC_Success != hResult && PRC_Success_Master != hResult)
	{
		PgClientPartyUtil::PartyResultMsg(hResult);
		return ;
	}

	rkPacket.Pop(kNewMasterGuid);
	m_kMasterGuid = kNewMasterGuid;
	//AddSelfPartyName();

	SPartyMember const* pkNewMaster = GetMember(kNewMasterGuid);
	if( pkNewMaster )
	{
		if( PgClientExpeditionUtil::IsInExpedition() == false )
		{
			std::wstring const &kNewMasterLog = pkNewMaster->kName + TTW(PMS_Is) + TTW(PMS_NewMaster);
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(kChatLog, kNewMasterLog, true);
		}
	}

	PgClientPartyUtil::UpdatePartyWndAll();
}

void PgClientParty::Recv_PT_N_C_ANS_PARTY_RENAME(BM::Stream& rkPacket)
{
	HRESULT hRet = 0;
	rkPacket.Pop(hRet);
	PgClientPartyUtil::PartyResultMsg(hRet);//실패만 온다
}

void PgClientParty::Recv_PT_M_C_NFY_PARTY_JOIN(BM::Stream& rkPacket)//타인의 파티 가입
{
	BM::GUID kCharGuid;
	BM::GUID kPartyGuid;

	rkPacket.Pop(kCharGuid);
	rkPacket.Pop(kPartyGuid);

	PgClientPartyUtil::SetCharacterPartyGuid(kCharGuid, kPartyGuid);
}


void PgClientParty::Recv_PT_N_C_ANS_PARTY_CHANGE_OPTION(BM::Stream& rkPacket)
{
	BYTE cRet = 0;
	SPartyOption kNewOption;

	rkPacket.Pop(cRet);

	if( PRC_Success != cRet )
	{
		PgClientPartyUtil::PartyResultMsg(cRet);
	}
	else
	{
		kNewOption.ReadFromPacket(rkPacket);

		std::wstring kTemp;
		bool const bRet = PgClientPartyUtil::PartyOptionStr(kNewOption, kTemp, true);		
		if( bRet )
		{
			if( PgClientExpeditionUtil::IsInExpedition() == false )
			{
				std::wstring kMessage = kTemp;
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage(kChatLog, kMessage, true);
			}
		}
		m_kOption = kNewOption;
		PgClientPartyUtil::UpdatePartyWndAll();
		AddSelfPartyName();
	}
}

void PgClientParty::Recv_PT_M_C_NFY_CHANGE_MEMBER_ABIL(BM::Stream& rkPacket)
{
	BM::GUID kCharGuid;
	BYTE cChangeAbilType = 0;
	unsigned short sVal = 0;
	POINT3 ptPos;
	ptPos.Clear();

	rkPacket.Pop( kCharGuid );
	rkPacket.Pop( cChangeAbilType );
	

	SPartyMember *pkMember = GetMember(kCharGuid);
	if(!pkMember) return;

	if( 0 != (cChangeAbilType & PMCAT_Class) )
	{
		rkPacket.Pop( sVal );
		pkMember->iClass = sVal;
	}
	if( 0 != (cChangeAbilType & PMCAT_Level) )
	{
		rkPacket.Pop( sVal );
		pkMember->sLevel = sVal;
	}
	if( 0 != (cChangeAbilType & PMCAT_HP) )
	{
		rkPacket.Pop( sVal );
		pkMember->sHP = sVal;
	}
	if( 0 != (cChangeAbilType & PMCAT_MP) )
	{
		rkPacket.Pop( sVal );
		pkMember->sMP = sVal;
	}
	if( 0 != (cChangeAbilType & PMCAT_POS) )
	{
		rkPacket.Pop( ptPos );
		pkMember->ptPos = ptPos;
	}
	PgClientPartyUtil::UpdatePartyWndAll();
}

void PgClientParty::Recv_PT_T_C_NFY_PARTY_MEMBER_MYHOME(BM::Stream& rkPacket)
{
	BM::GUID kMemberGuid;
	rkPacket.Pop(kMemberGuid);
	SHOMEADDR kHomeAddr;
	kHomeAddr.ReadFromPacket(rkPacket);

	SPartyMember *pkMember;
	pkMember = GetMember(kMemberGuid);
	if(!pkMember)
	{
		return;
	}
	
	pkMember->kHomeAddr = kHomeAddr;

	PgClientPartyUtil::UpdatePartyWndAll();
}

void PgClientParty::Recv_PT_T_C_ANS_OTHER_CHANNEL_PARTY(BM::Stream & rkPacket)
{	// 던전 파티 가입 신청했던 파티가 다른 채널에 있을 경우 이 패킷이 온다.
	short ChannelNo = 0;
	bool bAnsJoin = false;
	rkPacket.Pop(ChannelNo);
	rkPacket.Pop(bAnsJoin);

	if( bAnsJoin )
	{	// 수락. 채널 이동 함.
		short nRealmNo = 0;
		short nChannelNo = 0;

		g_kNetwork.TryRealmNo(g_kNetwork.NowRealmNo());
		g_kNetwork.TryChannelNo(ChannelNo);

		BM::Stream kPacket(PT_C_L_TRY_LOGIN);
		kPacket.Push(g_kNetwork.TryRealmNo());
		kPacket.Push(g_kNetwork.TryChannelNo());
		kPacket.Push(false);

		NETWORK_SEND_TO_SWITCH(kPacket);
	}
	else
	{	// 거절.
		std::wstring Temp = TTW(PMS_Fail_Req_Join);
		SChatLog ChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(ChatLog, Temp, true);
	}
}

void PgClientParty::Recv_PT_M_C_ANS_JOIN_PARTYFIND_ITEM_CHECK(BM::Stream & rkPacket)
{
	bool IsHaveItem = false;
	int NeedItemNo = 0;

	rkPacket.Pop(IsHaveItem);
	rkPacket.Pop(NeedItemNo);

	if( false == IsHaveItem )
	{
		std::wstring ItemName;
		if( GetItemName(NeedItemNo, ItemName) )
		{
			BM::vstring msg(TTW(700418));
			msg.Replace(L"#ITEM#", ItemName);
			lwAddWarnDataStr(lwWString((std::wstring)msg), 1, true);
		}
	}
}

void PgClientParty::Recv_PT_N_C_NFY_PARTY_USER_MAP_MOVE(BM::Stream& rkPacket)
{
	BM::GUID kCharGuid;
	//int iGroundNo = 0;
	SGroundKey kKey;

	rkPacket.Pop(kCharGuid);
	rkPacket.Pop(kKey);
	
	
	MemberMapMove(kCharGuid, kKey);
	
	PgClientPartyUtil::UpdatePartyWndAll();
	UpdatePartyMiniMapIcon();
}

void PgClientParty::Recv_PT_M_C_NFY_PARTY_MEMBER_ITEM(BM::Stream& rkPacket)
{
	typedef std::vector< PgBase_Item > CONT_ITEM_ARRAY;

	BM::GUID kCharGuid;
	CONT_ITEM_ARRAY kItemVec;
	
	rkPacket.Pop(kCharGuid);
	PU::TLoadArray_M(rkPacket, kItemVec);

	if( kItemVec.empty() )
	{
		return;
	}

	SPartyMember const* pkMember = g_kParty.GetMember(kCharGuid);
	if( !pkMember )
	{
		return;
	}

	XUI::CXUI_Wnd* pkPartyWnd = XUIMgr.Get(_T("FRM_PARTY_STATE"));//작은창
	if( !pkPartyWnd )
	{
		return;
	}

	/*XUI::CXUI_List* pkMemberList = dynamic_cast<XUI::CXUI_List*>(pkPartyWnd->GetControl(_T("MEMBER_LIST")));
	if( !pkMemberList )
	{
		return;
	}*/

	XUI::CXUI_Wnd* pkMemberItemFrm = NULL;
	XUI::CXUI_Image* pkMemberItemImg = NULL;
	//XUI::SListItem* pkItem = pkMemberList->FirstItem();
	//while(pkItem)//이 메시지 주인의 SmallPartyItem를 찾는다.
	for(int i=0; i<PV_MAX_MEMBER_CNT; ++i)
	{
		BM::vstring kWndName(L"PARTY_MEMBER_STATE");
		kWndName+=i;

		XUI::CXUI_Wnd* pkMemberWnd = pkPartyWnd->GetControl(kWndName);

		if( pkMemberWnd
		&&	pkMemberWnd->OwnerGuid() == pkMember->kCharGuid )//주인을 찾는다.
		{
			pkMemberItemFrm = pkMemberWnd->GetControl(_T("SFRM_ITEM"));
			if( pkMemberItemFrm )
			{
				pkMemberItemImg = dynamic_cast<XUI::CXUI_Image*>(pkMemberItemFrm->GetControl(_T("IMG_ITEM")));
				break;
			}
		}
	}

	CONT_ITEM_ARRAY::iterator item_iter = kItemVec.begin();
	GET_DEF(CItemDefMgr, kItemDefMgr);
	for(; kItemVec.end() != item_iter; ++item_iter)
	{
		const CONT_ITEM_ARRAY::value_type& rkItem = (*item_iter);
		if( !rkItem.ItemNo() ) {continue;}

		CItemDef const *pDef = kItemDefMgr.GetDef(rkItem.ItemNo());
		if( pDef )
		{
			const wchar_t* szNameStr = NULL;
			bool const bFindName = GetDefString(pDef->NameNo(), szNameStr);
			if( bFindName )
			{
				int const iTTW = (pDef->CanEquip())? 700094: 700095;
				std::wstring kMessage;//누가
				if( FormatTTW(kMessage, iTTW, pkMember->kName.c_str(), szNameStr, rkItem.Count()) )
				{
					if( PgClientExpeditionUtil::IsInExpedition() == false )
					{
						SChatLog kChatLog(CT_ITEM);
						g_kChatMgrClient.AddLogMessage(kChatLog, kMessage);
					}
				}
			}
		}

		if( pkMemberItemFrm	&& pkMemberItemImg )
		{
			POINT2 kSize = pkMemberItemImg->Size();

			PgUISpriteObject *pkSprite = g_kUIScene.GetIconTexture(rkItem.ItemNo());
			PgUIUVSpriteObject *pkUVSprite = (PgUIUVSpriteObject*)pkSprite;

			const SUVInfo &rkUV = pkUVSprite->GetUVInfo();
			POINT2 kPoint(kSize.x*rkUV.U, kSize.y*rkUV.V);//이미지 원본 사이즈 복구

			pkMemberItemImg->DefaultImgTexture(pkUVSprite);
			pkMemberItemImg->UVInfo(rkUV);
			pkMemberItemImg->ImgSize(kPoint);
			pkMemberItemImg->SetInvalidate();

			int const dwCurAliveTime = 3000;
			pkMemberItemFrm->CalledTime(BM::GetTime32());
			pkMemberItemFrm->AliveTime(dwCurAliveTime);
			pkMemberItemFrm->Visible(true);
			pkMemberItemFrm->SetInvalidate();
		}
	}
}

BM::GUID const& PgClientParty::GetMemberGuidAt(int const iAt) const
{
	if( 0<=iAt && PgClientPartyUtil::IsInParty() )
	{
		ContPartyMember::const_iterator mem_it = m_Members.begin();
		int iCount = 0;
		while(mem_it != m_Members.end())
		{
			if(iCount >=iAt && (*mem_it))
			{
				return (*mem_it)->kCharGuid;
			}
			++iCount;
			++mem_it;
		}
	}
	return BM::GUID::NullData();
}

bool PgClientParty::GetMemberNameAt(int const iAt, std::wstring& rkName)
{
	if( 0<=iAt && PgClientPartyUtil::IsInParty() )
	{
		ContPartyMember::const_iterator mem_it = m_Members.begin();
		int iCount = 0;
		while(mem_it != m_Members.end())
		{
			if(iCount >=iAt && (*mem_it))
			{
				rkName = (*mem_it)->kName;
				return true;
			}
			++iCount;
			++mem_it;
		}
	}
	return false;
}

void PgClientParty::SetPartyTotalList()
{
	//m_kPartyListContinent.clear();
	m_kPartyListAttribute.clear();

	CONT_DEF_PARTY_INFO const* pkDefPartyInfo = NULL;
	g_kTblDataMgr.GetContDef(pkDefPartyInfo);	

	int iContinent = static_cast<int>(POC_Area1);

	while( 0 < iContinent )
	{
		CONT_DEF_PARTY_INFO::const_iterator iter = pkDefPartyInfo->find(iContinent);
		if( pkDefPartyInfo->end() != iter )
		{
			CONT_ATTRIBUTE_LIST::mapped_type::value_type kValue;
			kValue.iAttribute = static_cast<int>(POA_All);
			kValue.iArea_NameNo = 401322;
			std::pair<CONT_ATTRIBUTE_LIST::iterator, bool> kRet = m_kPartyListAttribute.insert(std::make_pair(iContinent, CONT_ATTRIBUTE_LIST::mapped_type()));
			kRet.first->second.push_back(kValue);


			CONT_DEF_PARTY_INFO::mapped_type const kContinentList = iter->second;

			CONT_DEF_PARTY_INFO::mapped_type::const_iterator iter_ContinentList = kContinentList.begin();
			while(kContinentList.end() != iter_ContinentList)
			{
				/*CONT_CONTINENT_LIST::iterator iter_Continent = m_kPartyListContinent.find(iContinent);
				if( m_kPartyListContinent.end() == iter_Continent )
				{
					std::pair<CONT_CONTINENT_LIST::iterator, bool> kPair = m_kPartyListContinent.insert(std::make_pair(iContinent, CONT_CONTINENT_LIST::mapped_type()));
					iter_Continent = kPair.first;
				}
				(*iter_Continent).second.push_back(iter_ContinentList->iArea_NameNo);*/

				/////////////////

				CONT_ATTRIBUTE_LIST::iterator iter_Attribute = m_kPartyListAttribute.find(iContinent);
				if( m_kPartyListAttribute.end() == iter_Attribute )
				{
					std::pair<CONT_ATTRIBUTE_LIST::iterator, bool> kPair = m_kPartyListAttribute.insert(std::make_pair(iContinent, CONT_ATTRIBUTE_LIST::mapped_type()));
					iter_Attribute = kPair.first;
				}				
				kValue.iAttribute = iter_ContinentList->iAttribute;
				kValue.iArea_NameNo = iter_ContinentList->iArea_NameNo;
				(*iter_Attribute).second.push_back(kValue);

				++iter_ContinentList;
			}
		}
		else
		{
			return;
		}
		++iContinent;		
	}
}

void PgClientParty::GetPartyListAreaName(bool bAll, int const iAttribute, int const iContinent, lwUIWnd lwWnd)
{
	XUI::CXUI_Form *pkTopWnd = (XUI::CXUI_Form*)lwWnd();
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_List* pkAreaNameListWnd = dynamic_cast<XUI::CXUI_List*>(pkTopWnd);
	if( !pkAreaNameListWnd )
	{
		return;
	}

	/*XUI::SListItem* pAreaNameItem = pkAreaNameListWnd->FirstItem();
	if( !pAreaNameItem )
	{
		pAreaNameItem = pkAreaNameListWnd->AddItem(_T(""),0);
	}*/

	//CONT_CONTINENT_LIST		m_kPartyListContinent;
	//CONT_ATTRIBUTE_LIST		m_kPartyListAttribute;

	CONT_ATTRIBUTE_LIST::iterator iter = m_kPartyListAttribute.find(iContinent);
	if( m_kPartyListAttribute.end() != iter )
	{
		CONT_ATTRIBUTE_LIST::mapped_type const kAreaNameNoList = iter->second;
		CONT_ATTRIBUTE_LIST::mapped_type::const_iterator iter_AreaNameNoList = kAreaNameNoList.begin();
		while(kAreaNameNoList.end() != iter_AreaNameNoList)
		{
			if( 480801 == iter_AreaNameNoList->iArea_NameNo )  // 항목 이름이 '기타'인 경우 화면에 표시 안함.
			{
				++iter_AreaNameNoList;
				continue;
			}
			if( (iAttribute == iter_AreaNameNoList->iAttribute) /*|| (0 == iAttribute)*/ )
			{
				//if( pAreaNameItem )
				{
					int iValue = iter_AreaNameNoList->iArea_NameNo;
					XUI::SListItem* pAreaNameItem = pkAreaNameListWnd->AddItem(TTW(iValue));
					if( pAreaNameItem )
					{
						XUI::CXUI_Wnd* pkItemWnd = pAreaNameItem->m_pWnd;
						if( pkItemWnd )
						{
							pkItemWnd->Text( TTW(iValue) );
							int iData = static_cast<int>(iValue);
							pkItemWnd->SetCustomData( &iData, sizeof(iData));
						}
					}
					//pAreaNameItem = pkAreaNameListWnd->NextItem(pAreaNameItem);
				}
			}
			++iter_AreaNameNoList;
		}
	}

	XUI::SListItem* pItem = pItem = pkAreaNameListWnd->FirstItem();
	if( !bAll )
	{	
		if( pItem )
		{
			pkAreaNameListWnd->DeleteItem(pItem);
		}
	}
	else
	{
		if( pItem )
		{
			int iValue = 0;
			pItem->m_pWnd->GetCustomData(&iValue, sizeof(iValue));
			lua_tinker::call<void, int>("OnSelectParty_Area", iValue);
		}
	}
}

bool PgClientParty::GetPartyListStateToolTip(BM::GUID kGuid, std::wstring& kStr)
{
	BM::CAutoMutex kLock(m_kMutex);

	kStr = _T("");
	const wchar_t *pText = NULL;

	ContPartyListName::const_iterator iter = m_kPartyListName.begin();
	while( m_kPartyListName.end() != iter )
	{
		ContPartyListName::mapped_type kElement = iter->second;
		if( kGuid == kElement.kMasterGuid )
		{
			CONT_DEFMAP const * pkContDefMap = NULL;
			g_kTblDataMgr.GetContDef(pkContDefMap);

			if( 0 == kElement.iMasterMapNo )
			{	// 맵번호가 0이면 던전이나 맵이동 준비중인 상태.
				kStr += TTW(799395);
				return true;
			}

			CONT_DEFMAP::const_iterator map_iter = pkContDefMap->find(kElement.iMasterMapNo);
			if( pkContDefMap->end() == map_iter )
			{
				GetDefString(kElement.iMasterMapNo, pText);
			}
			else
			{
				TBL_DEF_MAP const& rkDefMap = (*map_iter).second;
				GetDefString(rkDefMap.NameNo, pText);
			}
			if( !pText )
			{
				return false;
			}
			std::wstring MapName(pText);

			BM::vstring kMessage;
			if( EPS_WAITING == kElement.cStatus )
			{
				kMessage = TTW(799371);
				kMessage.Replace(L"$MAP$", MapName);
			}
			else
			{
				kMessage = TTW(799369);
				kMessage.Replace(L"$MAP$", MapName);
			}
			kStr += std::wstring(kMessage);
			return true;
		}
		++iter;
	}
	return false;
}

bool PgClientParty::GetPartyListToolTip(BM::GUID kGuid, std::wstring& kStr)
{
	BM::CAutoMutex kLock(m_kMutex);

	kStr = _T("");

	ContPartyListName::const_iterator iter = m_kPartyListName.begin();
	while( m_kPartyListName.end() != iter )
	{
		ContPartyListName::mapped_type kElement = iter->second;

		if( kGuid == kElement.kMasterGuid )
		{
			kStr += TTW(401350);

			if( kElement.kPartyName.empty() )
			{
				kStr += L"\n";
				kStr += m_kPartyName;
			}
			else
			{
				kStr += L"\n";
				kStr += kElement.kPartyName;
			}
			
			kStr += L"\n";

			// 파티장
			BM::vstring Message( TTW(401354) );
			Message.Replace(L"$CLASS$", PgClientPartyUtil::GetClassNameTTW(kElement.iMasterClass));
			Message.Replace(L"$CHAR_NAME$", kElement.kMasterName);
			Message.Replace(L"$LEVEL$", kElement.iMasterLevel);
			std::wstring NowMsg(Message);
			kStr += NowMsg;
			kStr += L"\n";

			for(int i=0;i<(kElement.cCurMember - 1); ++i)
			{
				if( i < PV_MAX_MEMBER_CNT-1 )
				{
					BM::vstring kMessage( TTW(401354) );
					kMessage.Replace(L"$CLASS$", PgClientPartyUtil::GetClassNameTTW(kElement.kMemberInfo[i].iClass));
					kMessage.Replace(L"$CHAR_NAME$", kElement.kMemberInfo[i].kName);
					kMessage.Replace(L"$LEVEL$", kElement.kMemberInfo[i].sLevel);
					std::wstring kNowMsg(kMessage);
					kStr += kNowMsg;
					kStr += L"\n";
				}
			}

			int const iFilter = POT_SafeMask ^ (POE_SafeMask | POI_SafeMask);
			ePartyOptionPublicTitle ePublic = (ePartyOptionPublicTitle)(kElement.iPartyOption & iFilter);

			kStr += TTW(401359);
			kStr += L" ";

			int const iFilterItem = POI_SafeMask ^ (POE_SafeMask | POI_SafeMask);
			EPartyOptionItem eType = (EPartyOptionItem)(kElement.iPartyOption & iFilterItem);			
			if( eType == POI_LikeSolo )//Item
			{
				kStr += TTW(401008);				
			}
			else if( eType == POI_Order )
			{
				kStr += TTW(401009);				
			}
			kStr += L"\n";

			if( !kElement.kPartySubName.empty() )
			{
				kStr += TTW(401353);
				kStr += L" ";
				kStr += kElement.kPartySubName;
			}
			
			return true;
		}				
		++iter;
	}
	return false;	
}

void PgClientParty::ArrangePartyList( ContPartyListName & PartyList )
{
	CONT_DEFMAP const* pDefMap = NULL;
	g_kTblDataMgr.GetContDef(pDefMap);
	if( NULL == pDefMap )
	{
		return;
	}
	
	ContPartyListName::iterator iter = PartyList.begin();
	while(iter != PartyList.end())
	{
		ContPartyListName::mapped_type & PartyInfo = iter->second;
		CONT_DEFMAP::const_iterator map_iter = pDefMap->find(PartyInfo.iMasterMapNo);
		if( pDefMap->end() != map_iter )
		{
			if( map_iter->second.iAttr & GATTR_FLAG_HIDE_PARTY_LIST )
			{// 슈퍼그라운드, 정령던전, 원정대, 이벤트 맵 등과 같은 파티난입을 하면 안되는 그라운드는 파티 리스트에서 제거한다.
				iter = PartyList.erase(iter);
				continue;
			}
		}
		++iter;
	}
}

void PgClientParty::SetPartyListAttribute( ContPartyListName & PartyList )
{
	CONT_DEFMAP const* pDefMap = NULL;
	g_kTblDataMgr.GetContDef(pDefMap);
	if( NULL == pDefMap )
	{
		return;
	}
	
	ContPartyListName::iterator iter = PartyList.begin();
	for( ; iter != PartyList.end(); ++iter )
	{
		ContPartyListName::mapped_type & PartyInfo = iter->second;
		PartyInfo.iPartyAttribute = 1;
		CONT_DEFMAP::const_iterator map_iter = pDefMap->find(PartyInfo.iMasterMapNo);
		if( pDefMap->end() != map_iter )
		{
			if( map_iter->second.iAttr & GATTR_FLAG_MISSION )
			{// 미션에 있는 파티다.
				PartyInfo.iPartyAttribute = 2;
				continue;
			}
			if( map_iter->second.iAttr & GATTR_FLAG_HIDDEN_F )
			{// 히든필드에 있는 파티다.
				PartyInfo.iPartyAttribute = 4;
				continue;
			}
			if( (map_iter->second.iAttr & GATTR_INSTANCE)
				|| (map_iter->second.iAttr & GATTR_FLAG_STATIC_DUNGEON) )
			{// 미션이 아닌데 인스턴스이거나 스태틱 던전 속성이면 던전에 있는 파티다.
				PartyInfo.iPartyAttribute = 3;
				continue;
			}
			// 그 외 필드
			PartyInfo.iPartyContinent = map_iter->second.sContinent;
			PartyInfo.iPartyLevel = 0;
		}
	}
}

void PgClientParty::PartyListNewSort(ContTempPartyListName & PartyList)
{
	CONT_DEFMAP const* pDefMap = NULL;
	g_kTblDataMgr.GetContDef(pDefMap);
	if( NULL == pDefMap )
	{
		return;
	}

	int const MAX_LIST_ARRAY = 7;
	ContTempPartyListName TempList[MAX_LIST_ARRAY];
	ContTempPartyListName::iterator iter = PartyList.begin();
	for( ; iter != PartyList.end(); ++iter )
	{
		if( true == CheckPartyList_Status( iter->cStatus ) )
		{// 스테이터스가 거부상태인 파티는 걸러낸다.
			continue;
		}
		if( true == CheckPartyList_MemberFull( iter->cCurMember, iter->cMaxMember ) )
		{// 입장인원이 풀인 파티인지 검사해서 가장 후순위로 넣는다.
			TempList[6].insert( TempList[6].end(), *iter );
			continue;
		}
		if( true == CheckPartyList_Map( iter->iMasterMapNo ) )
		{// 신청자와 동일한 맵에 있는 파티
			TempList[0].insert( TempList[0].end(), *iter );
			continue;
		}
		if( true == CheckPartyList_MissionDungeon( pDefMap, iter->iMasterMapNo ) )
		{// 신청자와 동일한 맵과 연결된 미션, 던전 에 있는 파티
			TempList[1].insert( TempList[1].end(), *iter );
			continue;
		}
		if( true == CheckPartyList_Town( pDefMap, iter->iMasterMapNo ) )
		{// 마을에 있는 파티
			TempList[2].insert( TempList[2].end(), *iter );
			continue;
		}
		if( true == CheckPartyList_MapLevel( pDefMap, iter->iMasterMapNo ) )
		{// 신청자의 레벨과 맵의 권장레벨의 차이가 +- 5이하인 맵들에 있는 파티
			TempList[3].insert( TempList[3].end(), *iter );
			continue;
		}
		if( true == CheckPartyList_MasterLevel( iter->iMasterLevel ) )
		{// 모든 맵에서 파티리더와 신청자가 +- 5 Level 이하인 파티
			TempList[4].insert( TempList[4].end(), *iter );
			continue;
		}
		//나머지
		TempList[5].insert( TempList[5].end(), *iter );
	}

	PartyList.clear();

	for( int listArray = 0; listArray < MAX_LIST_ARRAY; ++listArray )
	{
		if( TempList[listArray].size() )
		{
			PartyList.insert( PartyList.end(), TempList[listArray].begin(), TempList[listArray].end() );
		}
	}
}

short PgClientParty::GetPartyChannelNo( BM::GUID MasterGuid ) const
{
	ContPartyListName::const_iterator iter = m_kPartyListName.begin();
	for( ; m_kPartyListName.end() != iter; ++iter )
	{
		if( MasterGuid == iter->second.kMasterGuid )
		{
			return iter->second.ChannelNo;
		}
	}
	return -1;
}

bool PgClientParty::CheckPartyList_Status( BYTE const Status )
{
	if( Status == EPS_REFUSE )
	{
		return true;
	}
	return false;
}

bool PgClientParty::CheckPartyList_MemberFull( BYTE const CurMember, BYTE const MaxMember )
{
	if( CurMember == MaxMember )
	{
		return true;
	}
	return false;
}

bool PgClientParty::CheckPartyList_Map( int const MasterMapNo )
{// 신청자와 동일한 맵에 있는 파티
	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return false;
	}

	SGroundKey const& GroundKey = pPlayer->GroundKey();

	if( GroundKey.GroundNo() == MasterMapNo )
	{
		return true;
	}

	return false;
}

bool PgClientParty::CheckPartyList_MissionDungeon( CONT_DEFMAP const* pDefMap, int const MasterMapNo )
{// 신청자와 동일한 맵과 연결된 미션, 던전에 있는 파티
	if( NULL == pDefMap )
	{
		return false;
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 신청자가 위치한 맵을 찾는다.
	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return false;
	}
	SGroundKey const& GroundKey = pPlayer->GroundKey();

	CONT_DEFMAP::const_iterator myMap_iter = pDefMap->find(GroundKey.GroundNo());
	if( pDefMap->end() == myMap_iter )
	{
		return false;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지
	
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 필요한 DB 데이터를 불러온다.
	CONT_DEF_MISSION_ROOT const * pContDefMission = NULL;
	g_kTblDataMgr.GetContDef(pContDefMission);
	if( NULL == pContDefMission )
	{
		return false;
	}

	CONT_DEF_MISSION_CANDIDATE const * pContDefMissionCandi = NULL;
	g_kTblDataMgr.GetContDef(pContDefMissionCandi);
	if ( NULL == pContDefMissionCandi )
	{
		return false;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 미션 체크
	CONT_MISSION_NO::const_iterator miss_iter = myMap_iter->second.kContMissionNo.begin();
	for( ; myMap_iter->second.kContMissionNo.end() != miss_iter; ++miss_iter )
	{
		// 미션번호로 해당 미션 찾기
		int RootLevelValue[2] = {0};
		CONT_DEF_MISSION_ROOT::const_iterator root_itor = pContDefMission->find(*miss_iter);
		if( pContDefMission->end() != root_itor )
		{// 미션의 각 맵의 번호가 저장된 Candidate에 접근하기 위한 키값을 찾는다.
		 // 참고로 index는 미션의 종류.  0-아케이드 1-카오스
		 // 미션 창의 index(빌드로 되어있음)와 배열의 순서가 같으므로 그대로 사용함.
			RootLevelValue[0] = root_itor->second.aiLevel[0]; // 아케이드 모드
			RootLevelValue[1] = root_itor->second.aiLevel[1]; // 카오스 모드
		}

		if( 0 == RootLevelValue[0] )
		{// 아케이드 모드가 없는 미션은 확인할 필요 없다.
			return false;
		}

		for( int index = 0; index < 2; ++index )
		{
			// 키 값으로 유저가 선택한 미션의 Candidate에 접근
			CONT_DEF_MISSION_CANDIDATE::const_iterator candi_itr = pContDefMissionCandi->find(RootLevelValue[index]);
			if( pContDefMissionCandi->end() == candi_itr )
			{
				return false;
			}

			// 유저가 선택한 미션의 각 맵번호가 저장된 공간에 접근 완료
			const CONT_DEF_MISSION_CANDIDATE::mapped_type Element = candi_itr->second;
			if( 0 == Element.iCandidate_End)
			{
				return false;
			}

			for( int Candi_index = 0; Candi_index < MAX_MISSION_CANDIDATE; ++Candi_index )
			{// 모든 미션 맵들에 대해 검사한다.
				if ( 0 != Element.aiCandidate[Candi_index] )
				{// 미션 맵의 번호가 유효하고
					if( MasterMapNo == Element.aiCandidate[Candi_index] )
					{
						return true;
					}
				}
			}
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지
	
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 던전 체크
	CONT_MISSION_NO::const_iterator dungeon_iter = myMap_iter->second.ContDungeonNo.begin();
	for( ; myMap_iter->second.ContDungeonNo.end() != dungeon_iter; ++dungeon_iter )
	{
		if( *dungeon_iter == MasterMapNo )
		{
			return true;
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지
	return false;
}

bool PgClientParty::CheckPartyList_Town( CONT_DEFMAP const* pDefMap, int const MasterMapNo )
{// 마을에 있는 파티
	if( NULL == pDefMap )
	{
		return false;
	}

	CONT_DEFMAP::const_iterator map_iter = pDefMap->find(MasterMapNo);
	if( pDefMap->end() == map_iter )
	{
		return false;
	}

	return map_iter->second.iAttr & GATTR_FLAG_VILLAGE;
}

bool PgClientParty::CheckPartyList_MapLevel( CONT_DEFMAP const* pDefMap, int const MasterMapNo )
{// 신청자의 레벨과 맵의 권장레벨의 차이가 +- 5이하인 맵들에 있는 파티
	if( NULL == pDefMap )
	{
		return false;
	}

	CONT_DEFMAP::const_iterator map_iter = pDefMap->find(MasterMapNo);
	if( pDefMap->end() == map_iter )
	{
		return false;
	}

	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return false;
	}

	int const MyLevel = pPlayer->GetAbil(AT_LEVEL);
	int const RecommendLevel = map_iter->second.byRecommend_Level;

	if( (MyLevel <= (RecommendLevel + 5)) && (MyLevel >= (RecommendLevel - 5)) )
	{
		return true;
	}
	return false;
}

bool PgClientParty::CheckPartyList_MasterLevel( int const MasterLevel )
{// 모든 맵에서 파티리더와 신청자가 +- 5 Level 이하인 파티
	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return false;
	}
	int const MyLevel = pPlayer->GetAbil(AT_LEVEL);
	if( (MyLevel <= (MasterLevel + 5)) && (MyLevel >= (MasterLevel - 5)) )
	{
		return true;
	}
	return false;
}

void PgClientParty::Recv_PT_M_C_ANS_FIND_PARTY_USER_LIST(BM::Stream& rkPacket)
{
	BM::CAutoMutex kLock(m_kMutex);

	BYTE cRet = 0;
	int iSize = 0;

	rkPacket.Pop((BYTE)cRet);
	rkPacket.Pop(iSize);

	m_kFindPartyUserList.clear();

	if( !iSize )
	{
		std::wstring const rkMsg = TTW(700126);
		
		if( rkMsg.empty())
		{
			return;
		}

		//lwCloseUI("SFRM_PARTY_PART_FIND");

		if( PgClientExpeditionUtil::IsInExpedition() == false )
		{
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(kChatLog, rkMsg);
			lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(rkMsg), true);
		}

		FindUserNowPage(0);
		FindPartyUserListViewRefresh(0, 0);

		return;
	}
	else
	{
		char const *pszText = "SFRM_PARTY_PEOPLE_FIND";

		std::wstring const wstr = UNI(pszText);
		XUI::CXUI_Wnd* pWnd = NULL;
		if(!XUIMgr.IsActivate(wstr, pWnd))
		{
			lwCallUI("SFRM_PARTY_PEOPLE_FIND", false);
		}
	}
	
	while ( iSize-- )
	{		
		SFindPartyUserListInfo kItemSlot;

		kItemSlot.Clear();
		kItemSlot.ReadFromPacket(rkPacket);
		
		std::pair<ContFindPartyUserList::iterator, bool> kRet = m_kFindPartyUserList.insert(std::make_pair(kItemSlot.kCharGuid, kItemSlot));
		if( !kRet.second )//중복
		{
			ContFindPartyUserList::iterator iter = kRet.first;
			if( iter != m_kFindPartyUserList.end() )
			{
				(*iter).second = kItemSlot;
			}
		}
	}

	FindUserNowPage(0);
	FindPartyUserListViewRefresh(0, 0);
}

void PgClientParty::Recv_PT_M_C_ANS_REGIST_PRIVATE_PARTY_FIND(BM::Stream& rkPacket)
{
	int iTextNo = 0;
	HRESULT hResult;
	rkPacket.Pop(hResult);

	IsPrivateRegist(false);

	switch(hResult)
	{
	case FPRC_Success:
		{
			IsPrivateRegist(true);
			iTextNo = 799374;
		}break;
	case FPRC_Fail:
		{
			iTextNo = 799377;
		}break;
	case FPRC_Fail_Max:
		{
			iTextNo = 799378;
		}break;
	case FPRC_Fail_BadName:
		{
			iTextNo = 799379;
		}break;
	default:
		{
			iTextNo = 799377;
		}break;
	}

	lua_tinker::call<void, bool>("Set_PrivateRegist", true);

	if( PgClientExpeditionUtil::IsInExpedition() == false )
	{
		if( PgClientExpeditionUtil::IsInExpedition() == false )
		{
			SChatLog kChatLog((EChatType)CT_EVENT);
			g_kChatMgrClient.AddMessage(iTextNo, kChatLog, true, 11);
		}
	}
}