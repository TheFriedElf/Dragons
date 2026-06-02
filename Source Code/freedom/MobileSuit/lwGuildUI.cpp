#include "Stdafx.h"
#include "Variant/PgStringUtil.h"
#include "CreateUsingNiNew.inl"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgGuild.h"
#include "lwWString.h"
#include "lwUI.h"
#include "lwGUID.h"
#include "lwGuildUI.h"
#include "PgChatMgrClient.h"
#include "PgNetwork.h"
#include "ServerLib.h"
#include "PgEmporiaMgr.h"
#include "PgUICalculator.h"
#include "PgCommandMgr.h"
#include "PgClientParty.H"
#include "PgEventTimer.h"
#include "lwUIQuest.h"

//bool RegisterWrapper(lua_State *pkState)
//{
//	using namespace lua_tinker;
//
//
//	return true;
//}

extern bool BreakMoney_WStr(__int64 const iTotal, VEC_WSTRING &rkVec);
extern void SetEmporiaBattleAlram(bool bClear=false);
extern bool lwIsFullEmporiaBattleArea(int const iIndex);
extern void lwRandomMoveBattleArea();

int lwGetEmporiaChallengeLimitCost();

char const * EVENT_EM_START_JOIN = "EVENT_EMPORIA_START_JOIN";

namespace lwGuild
{
	void lwReq_FollowEmporiaStartJoin();
	void lwNoticeEmporiaBattleRemainTime(lwWString kNoti);
}

bool lwGuild::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	//Guild
	def(pkState, "CallGuildUI", CallGuildUI);
	def(pkState, "UpdateGuildUI", UpdateGuildUI);
	def(pkState, "UpdateGuildSkillUI", UpdateGuildSkillUI);
	def(pkState, "Send_PreCreateGuild", Send_PreCreateGuild);
	def(pkState, "Send_CreateGuild", Send_CreateGuild);
	def(pkState, "Send_DestroyGuild", Send_DestroyGuild);
	def(pkState, "Send_LeaveGuild", Send_LeaveGuild);
	def(pkState, "Send_GuildKickUser", Send_GuildKickUser);
	def(pkState, "Send_ReqJoinGuildName", Send_ReqJoinGuildName);
	def(pkState, "Send_ReqJoinGuildGuid", Send_ReqJoinGuildGuid);
	def(pkState, "Send_ReqKickMemberName", Send_ReqKickMemberName);
	def(pkState, "Send_ReqGuildLvUp", Send_ReqGuildLvUp);
	def(pkState, "Send_ReqGuildInventoryCreate", Send_ReqGuildInventoryCreate);	
	def(pkState, "SendReqChangeMemberGrade", SendReqChangeMemberGrade);
	def(pkState, "ReqChangeOwner", ReqChangeOwner);
	def(pkState, "AmIGuildMaster", AmIGuildMaster);
	def(pkState, "AmIGuildOwner", AmIGuildOwner);
	def(pkState, "IsGuildMaster", IsGuildMaster);
	def(pkState, "HaveGuild", HaveGuild);
	def(pkState, "HaveGuildInventory", HaveGuildInventory);
	def(pkState, "GetGuildLevel", GetGuildLevel);
	def(pkState, "GetGuildLevelIntroText", GetGuildLevelIntroText);
	def(pkState, "GetGuildLevelResultText", GetGuildLevelResultText);
	def(pkState, "UpdateGuildSkillBuyUI", UpdateGuildSkillBuyUI);
	def(pkState, "IsHaveGuild", IsHaveGuild);
	def(pkState, "Send_CreateGuildTest", Send_CreateGuildTest);
	def(pkState, "SetTempGuildEmblem", SetTempGuildEmblem);
	def(pkState, "GetTempGuildEmblem", GetTempGuildEmblem);
	def(pkState, "SetGuildListMode", SetGuildListMode);
	def(pkState, "SetGuildNotice", SetGuildNotice);
	def(pkState, "GetGuildNotice", GetGuildNotice);

	def(pkState, "CallChangeMarkYesNo", PgGuildMgrUtil::CallChangeMarkYesNo);
	def(pkState, "GetMaxGuildMarkCount", PgGuildMgrUtil::GetMaxGuildMaxCount);

	def(pkState, "GetMyEmporiaKey", GetMyEmporiaKey);
	def(pkState, "EmporiaGiveUp", EmporiaGiveUp);
	def(pkState, "Req_JoinEmporia", Req_JoinEmporia);
	def(pkState, "Req_FollowEmporiaStartJoin", lwReq_FollowEmporiaStartJoin);
	def(pkState, "IsHaveEmporiaFunc", IsHaveEmporiaFunc);
	def(pkState, "KeepEmporiaFunc", KeepEmporiaFunc);
//	def(pkState, "GetEmporiaFuncPrice", GetEmporiaFuncPrice);

	def(pkState, "CallGuildSettingUI", CallGuildSettingUI);
	def(pkState, "OnClickChangeGuildSettingTab", OnClickChangeGuildSettingTab);
	def(pkState, "OnClickGuildEntranceSetBtn", OnClickGuildEntranceSetBtn);
	def(pkState, "OnClickGuildEntranceSetOK", OnClickGuildEntranceSetOK);
	def(pkState, "OnCallGuildEntranceLevelDrop", OnCallGuildEntranceLevelDrop);
	def(pkState, "CallGuildApplicantAcceptReject", CallGuildApplicantAcceptReject);
	def(pkState, "GuildEntranceProcess", GuildEntranceProcess);

	
	def(pkState, "OnCallMercenaryJoinLevelDrop", OnCallMercenaryJoinLevelDrop);
	def(pkState, "OnClickMercenarySetBtn", OnClickMercenarySetBtn);
	def(pkState, "OnClickMercenartSetOK", OnClickMercenartSetOK);
	def(pkState, "NoticeEmporiaBattleRemainTime", lwNoticeEmporiaBattleRemainTime);

	def(pkState, "ChangeGuildInvTab", lwChangeGuildInvTab);
	def(pkState, "SendGuildInvOpen", lwSendGuildInvOpen);
	def(pkState, "SendGuildInvClose", lwSendGuildInvClose);
	def(pkState, "SendGuildInvMoney", lwSendGuildInvMoney);
	def(pkState, "SendGuildMoney", lwSendGuildMoney);
	def(pkState, "RequestGuildInvLog", lwRequestGuildInvLog);
	def(pkState, "GuildLogPageMove", lwGuildLogPageMove);
	def(pkState, "CallGuildInvSuperVision", lwCallGuildInvSuperVision);
	def(pkState, "OnCallGuildInvAuthoritylDrop", lwOnCallGuildInvAuthoritylDrop);
	def(pkState, "OnClickGuildInvAuthoritySetBtn", lwOnClickGuildInvAuthoritySetBtn);
	def(pkState, "OnClickGuildInvAuthorityOK", lwOnClickGuildInvAuthorityOK);

	def(pkState, "IsFullEmporiaBattleArea", lwIsFullEmporiaBattleArea);
	def(pkState, "GetEmporiaChallengeLimitCost", lwGetEmporiaChallengeLimitCost);
	def(pkState, "RandomMoveBattleArea", lwRandomMoveBattleArea);
	def(pkState, "IsMyGuildMemeber", lwIsMyGuildMemeber);
	return true;
}


bool lwGuild::Send_PreCreateGuild(lwWString kString)
{
	return g_kGuildMgr.PreCreateGuild(kString());;
}

bool lwGuild::Send_CreateGuild()
{
	return g_kGuildMgr.CreateGuild();
}

bool lwGuild::Send_DestroyGuild()
{
	return g_kGuildMgr.DestroyGuild();
}

bool lwGuild::Send_LeaveGuild()
{
	return g_kGuildMgr.LeaveGuild();
}

bool lwGuild::Send_GuildKickUser(lwGUID kCharGuid)
{
	return g_kGuildMgr.KickUser(kCharGuid());
}

bool lwGuild::Send_ReqJoinGuildName(lwWString kName)
{
	g_kGuildMgr.ReqJoinGuild(kName());
	return true;
}

bool lwGuild::Send_ReqJoinGuildGuid(lwGUID kGuildGuid)
{
	g_kGuildMgr.ReqJoinGuild(kGuildGuid());
	return true;
}

bool lwGuild::Send_ReqKickMemberName(lwWString kMemberName)
{
	SGuildMemberInfo kMemberInfo;
	bool const bFindMember = g_kGuildMgr.GetMemberByName(kMemberName(), kMemberInfo);
	if( !bFindMember )
	{
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(700091, kChatLog, true);		
		return false;
	}

	g_kGuildMgr.KickUser(kMemberInfo.kCharGuid);
	return true;
}

bool lwGuild::Send_ReqGuildLvUp()
{
	BM::GUID kMyCharGuid;
	if( !g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid) )
	{
		return false;
	}

	if( !g_kGuildMgr.IamHaveGuild() )
	{
		return false;
	}

	if( !g_kGuildMgr.IsMaster(kMyCharGuid) )
	{
		return false;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_M_LvUp);
	kPacket.Push(true);//First Test
	NETWORK_SEND(kPacket)
	return true;
}

void lwGuild::lwOnClickGuildInvAuthorityOK(lwUIWnd kParent)
{
	XUI::CXUI_Wnd* pMainUI = kParent.GetSelf();
	if( !pMainUI )
	{
		return;
	}

	BYTE byAuthority_Out = 0;
	XUI::CXUI_Wnd* pkAuthority_Out = pMainUI->GetControl(L"FRM_AUTHORITY_OUT");
	if( pkAuthority_Out )
	{
		pkAuthority_Out->GetCustomData(&byAuthority_Out, sizeof(byAuthority_Out));
	}

	BYTE byAuthority_In = 0;
	XUI::CXUI_Wnd* pkAuthority_In = pMainUI->GetControl(L"FRM_AUTHORITY_IN");
	if( pkAuthority_In )
	{
		pkAuthority_In->GetCustomData(&byAuthority_In, sizeof(byAuthority_In));
	}

	if( g_kGuildMgr.GetGuildInvAuthority_In() == byAuthority_In
	&&	g_kGuildMgr.GetGuildInvAuthority_Out() == byAuthority_Out )
	{
		return ;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_M_InventorySupervision));
	kPacket.Push(byAuthority_In);
	kPacket.Push(byAuthority_Out);
	NETWORK_SEND(kPacket);
}

void lwGuild::lwOnClickGuildInvAuthoritySetBtn(lwUIWnd kSelf, int const iType)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"SFRM_GUILD_INVENTORY_SUPERVISION");
	if( !pMainUI )
	{
		pMainUI = XUIMgr.Call(L"SFRM_GUILD_INVENTORY_SUPERVISION");
		if( !pMainUI )
		{
			return ;
		}
	}

	if( iType )
	{
		XUI::CXUI_Wnd* pkAuthority_Out = pMainUI->GetControl(L"FRM_AUTHORITY_OUT");
		if( pkAuthority_Out )
		{
			BYTE byAuthority = 0;
			pSelf->GetCustomData(&byAuthority, sizeof(byAuthority));

			BM::vstring vStr;
			switch(byAuthority)
			{
			case GMG_Owner: { vStr = TTW(403403); }break;
			case GMG_Master:{ vStr = TTW(403404); }break;
			default:		{ vStr = TTW(403405); }break;
			}
			pkAuthority_Out->Text(vStr);
			pkAuthority_Out->SetCustomData(&byAuthority, sizeof(byAuthority));
		}
	}
	else
	{
		XUI::CXUI_Wnd* pkAuthority_In = pMainUI->GetControl(L"FRM_AUTHORITY_IN");
		if( pkAuthority_In )
		{
			BYTE byAuthority = 0;
			pSelf->GetCustomData(&byAuthority, sizeof(byAuthority));

			BM::vstring vStr;
			switch(byAuthority)
			{
			case GMG_Owner: { vStr = TTW(403400); }break;
			case GMG_Master:{ vStr = TTW(403401); }break;
			default:		{ vStr = TTW(403402); }break;
			}
			pkAuthority_In->Text(vStr);
			pkAuthority_In->SetCustomData(&byAuthority, sizeof(byAuthority));
		}
	}
}

void lwGuild::lwOnCallGuildInvAuthoritylDrop(lwUIWnd kSelf, const int iType)
{
	wchar_t const* const SZ_DROP_WINDOW_NAME = iType ? L"SFRM_GUILD_INV_AUTHORITY_DROP_OUT" : L"SFRM_GUILD_INV_AUTHORITY_DROP_IN";
	wchar_t const* const SZ_DROP_LIST = L"LST_ITEMS";
	wchar_t const* const SZ_DROP_BORDER = L"SFRM_BORDER";

	int const MAX_VIEW_LIST_ITEM = 3;

	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	XUI::CXUI_Wnd* pDrop = XUIMgr.Get(SZ_DROP_WINDOW_NAME);
	if( !pDrop || pDrop->IsClosed() )
	{
		pDrop = XUIMgr.Call(SZ_DROP_WINDOW_NAME);
	}
	else
	{
		pDrop->Close();
		return;
	}

	XUI::CXUI_Wnd* pkBorder = pDrop->GetControl(SZ_DROP_BORDER);
	if( !pkBorder )
	{
		pDrop->Close();
		return;
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pDrop->GetControl(SZ_DROP_LIST));
	if( !pkList )
	{
		pDrop->Close();
		return;
	}

	pkList->DeleteAllItem();

	const int iBaseText = iType ? 403403 : 403400;
	int iItemHeight = 0;
	for( BYTE i = 0; i < MAX_VIEW_LIST_ITEM; ++i )
	{
		XUI::SListItem* pkItem = pkList->AddItem(L"");
		if( pkItem && pkItem->m_pWnd )
		{
			BM::vstring vStr(TTW(iBaseText+i));
			pkItem->m_pWnd->Text(vStr);
			pkItem->m_pWnd->SetCustomData(&i, sizeof(i));
			iItemHeight = pkItem->m_pWnd->Size().y;
		}
	}

	int const iItemCount = pkList->GetTotalItemCount();
	int TotalListHeight = iItemHeight * iItemCount;
	if( MAX_VIEW_LIST_ITEM < iItemCount )
	{
		TotalListHeight = iItemHeight * MAX_VIEW_LIST_ITEM;
	}

	POINT3I kPoint = pParent->TotalLocation();
	kPoint.y += pParent->Size().y;
	pDrop->Location(kPoint);
	pkList->Size(POINT2(pkList->Size().x, TotalListHeight));
	pDrop->Size(POINT2(pDrop->Size().x, pkList->Size().y + 6));
	pkBorder->Size(POINT2(pDrop->Size().x, pDrop->Size().y + 2));
}

void lwGuild::lwCallGuildInvSuperVision()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( !g_kGuildMgr.IsOwner( pkPlayer->GetID() ) )
	{
		g_kChatMgrClient.LogMsgBox(400854);
		return ;
	}

	XUI::CXUI_Wnd* pkParent = XUIMgr.Get(L"SFRM_GUILD_INVENTORY_SUPERVISION");
	if( !pkParent )
	{
		pkParent = XUIMgr.Call(L"SFRM_GUILD_INVENTORY_SUPERVISION");
		if( !pkParent )
		{
			return ;
		}
	}

	XUI::CXUI_Wnd* pkAuthority_In = pkParent->GetControl(L"FRM_AUTHORITY_IN");
	if( pkAuthority_In )
	{
		BM::vstring vStr;
		const BYTE byAuthority = g_kGuildMgr.GetGuildInvAuthority_In();

		switch(byAuthority)
		{
		case GMG_Owner: { vStr = TTW(403400); }break;
		case GMG_Master:{ vStr = TTW(403401); }break;
		default:		{ vStr = TTW(403402); }break;
		}
		pkAuthority_In->Text(vStr);
	}

	XUI::CXUI_Wnd* pkAuthority_Out = pkParent->GetControl(L"FRM_AUTHORITY_OUT");
	if( pkAuthority_Out )
	{
		BM::vstring vStr;
		const BYTE byAuthority = g_kGuildMgr.GetGuildInvAuthority_Out();

		switch(byAuthority)
		{
		case GMG_Owner: { vStr = TTW(403403); }break;
		case GMG_Master:{ vStr = TTW(403404); }break;
		default:		{ vStr = TTW(403405); }break;
		}
		pkAuthority_Out->Text(vStr);
	}
}

bool lwGuild::Send_ReqGuildInventoryCreate()
{
	BM::GUID kMyCharGuid;
	if( !g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid) )
	{
		return false;
	}

	if( !g_kGuildMgr.IamHaveGuild() )
	{
		return false;
	}

	if( !g_kGuildMgr.IsOwner(kMyCharGuid) )
	{
		return false;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_M_InventoryCreate));
	kPacket.Push(true);
	NETWORK_SEND(kPacket);
	return true;
}

void lwGuild::SendReqChangeMemberGrade(lwGUID kCharGuid, int iNewGrade)
{
	BM::GUID kMyCharGuid;
	if( !g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid) )
	{
		return;
	}
	if( !g_kGuildMgr.IamHaveGuild() )
	{
		return;
	}
	if( !g_kGuildMgr.IsOwner(kMyCharGuid) )
	{
		return;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_M_MemberGrade);
	kPacket.Push( kCharGuid() );
	kPacket.Push( (BYTE)iNewGrade );
	NETWORK_SEND(kPacket)

}

bool lwGuild::ReqChangeOwner(lwGUID kNewOwnerGuid)
{
	SGuildMemberInfo kMemberInfo;
	if( !g_kGuildMgr.GetMemberByGuid(kNewOwnerGuid(), kMemberInfo) )
	{
		return false;
	}

	std::wstring kFormatText;

	if( !FormatTTW(kFormatText, 400829, kMemberInfo.kCharName.c_str()) )
	{
		return false;
	}

	CallYesNoMsgBox(kFormatText, kNewOwnerGuid(), MBT_CONFIRM_GUILD_OWNER_CHANGE);
	return true;
}

void lwGuild::CallGuildUI()
{
	bool const bHaveGuild = g_kGuildMgr.IamHaveGuild();

	std::wstring kUIName = L"";
	if( false == bHaveGuild )
	{
		kUIName = L"SFRM_GUILD_HELP";
	}
	else
	{
		kUIName = L"SFRM_MAIN_GUILD";
	}

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(kUIName);
	if( !pkWnd || pkWnd->IsClosed() )
	{
		XUIMgr.Call(kUIName);
	}
	else
	{
		pkWnd->Close();
	}
}

bool lwGuild::UpdateGuildUI(lwUIWnd kTopWnd)
{
	g_kGuildMgr.UpdateGuildUI(kTopWnd());
	return true;
}

bool lwGuild::UpdateGuildSkillUI(lwUIWnd kTopWnd)
{
	g_kGuildMgr.UpdateGuildSkillUI(kTopWnd());
	return true;
}

bool lwGuild::AmIGuildMaster()
{
	BM::GUID kMyCharGuid;
	bool const bFindPilot = g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid);
	if( bFindPilot )
	{
		return g_kGuildMgr.IsMaster(kMyCharGuid);
	}
	return false;
}

bool lwGuild::AmIGuildOwner()
{
	BM::GUID kMyCharGuid;
	bool const bFindPilot = g_kPilotMan.GetPlayerPilotGuid(kMyCharGuid);
	if( bFindPilot )
	{
		return g_kGuildMgr.IsOwner(kMyCharGuid);
	}
	return false;
}

bool lwGuild::IsGuildMaster(lwGUID kCharGuid)
{
	return g_kGuildMgr.IsMaster(kCharGuid());
}

bool lwGuild::HaveGuild()
{
	return g_kGuildMgr.IamHaveGuild();
}

bool lwGuild::HaveGuildInventory()
{
	return g_kGuildMgr.HaveGuildInventory();
}

int lwGuild::GetGuildLevel()
{
	return g_kGuildMgr.Level();
}

int lwGuild::GetGuildLevelIntroText(int const iGuildLv)
{
	CONT_DEF_GUILD_LEVEL const *pkDefGuildLevel = NULL;
	g_kTblDataMgr.GetContDef(pkDefGuildLevel);

	if( pkDefGuildLevel )
	{
		CONT_DEF_GUILD_LEVEL::const_iterator iter = pkDefGuildLevel->find(iGuildLv);
		if( pkDefGuildLevel->end() != iter )
		{
			return (*iter).second.iIntroText;
		}
	}
	return 0;
}

int lwGuild::GetGuildLevelResultText(int const iGuildLv)
{
	CONT_DEF_GUILD_LEVEL const *pkDefGuildLevel = NULL;
	g_kTblDataMgr.GetContDef(pkDefGuildLevel);

	if( pkDefGuildLevel )
	{
		CONT_DEF_GUILD_LEVEL::const_iterator iter = pkDefGuildLevel->find(iGuildLv);
		if( pkDefGuildLevel->end() != iter )
		{
			return (*iter).second.iResultText;
		}
	}
	return 0;
}

void lwGuild::UpdateGuildSkillBuyUI(lwUIWnd kTopWnd)
{
	//
	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return;
	}

	XUI::CXUI_Wnd *pkTopWnd = kTopWnd();
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_List *pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkTopWnd->GetControl(_T("LIST_GUILD_SKILL")));
	if( !pkListWnd )
	{
		return;
	}

	PgMySkill *pkMySkill = pkPC->GetMySkill();
	if( !pkMySkill )
	{
		return;
	}

	//
	pkListWnd->ClearList();

	// Def
	CONT_DEF_GUILD_SKILL const *pkDefGuildSkill = NULL;
	g_kTblDataMgr.GetContDef(pkDefGuildSkill);
	if( !pkDefGuildSkill )
	{
		return;
	}
	GET_DEF(CSkillDefMgr, kSkillDefMgr);

	// 
	CONT_DEF_GUILD_SKILL::const_iterator skill_iter = pkDefGuildSkill->begin();
	for(; pkDefGuildSkill->end()!=skill_iter; ++skill_iter)
	{
		//
		CONT_DEF_GUILD_SKILL::mapped_type const &rkDefGuildSkill = (*skill_iter).second;

		int const iLearnedSkillNo = pkMySkill->GetLearnedSkill(rkDefGuildSkill.iSkillNo);
		bool const bExitSkill = pkMySkill->IsExist(rkDefGuildSkill.iSkillNo) || iLearnedSkillNo > rkDefGuildSkill.iSkillNo;
		if( bExitSkill )
		{
			//이미 있는 스킬 목록 표시 하지 않음
			continue;
		}

		//
		CSkillDef const *pkSkillDef = kSkillDefMgr.GetDef(rkDefGuildSkill.iSkillNo);
		if( !pkSkillDef )
		{
			continue;
		}

		// 스킬 이름
		std::wstring const *pkSkillName = NULL;
		if( !::GetDefString(pkSkillDef->NameNo(), pkSkillName) )
		{
			continue;
		}
		
		//
		XUI::SListItem *pkNewItem = pkListWnd->AddItem(_T(""));
		if( !pkNewItem
		||	!pkNewItem->m_pWnd )
		{
			continue;
		}

		// 길드 레벨이 낮으면 구매 불가
		bool const bCanBuySkill = g_kGuildMgr.Level() >= rkDefGuildSkill.sGuildLv;

		//
		TCHAR szTemp[512] = {0, };
		if(0 < pkSkillDef->GetAbil(AT_IGNORE_SKILLLV_TOOLTIP))
		{// 스킬 레벨 표시 하지 않는 어빌이 있으면 
			_stprintf_s(szTemp, sizeof(szTemp), _T("%s"), pkSkillName->c_str());
		}
		else
		{
			_stprintf_s(szTemp, sizeof(szTemp), _T("%s %s %d"), pkSkillName->c_str(), TTW(224).c_str(), pkSkillDef->GetAbil(AT_LEVEL));
		}

		XUI::CXUI_Wnd *pkNewItemWnd = pkNewItem->m_pWnd;

		XUI::CXUI_Wnd *pkNameWnd = pkNewItemWnd->GetControl(_T("FRM_NAME"));
		if( pkNameWnd )
		{
			pkNameWnd->Text( szTemp );
		}

		// 스킬 가격
		typedef std::vector< std::wstring > ContWstr;
		ContWstr kMoneyVec;
		if (BreakMoney_WStr(rkDefGuildSkill.iGold, kMoneyVec))
		{
			XUI::CXUI_Wnd *pkGoldFrm = pkNewItemWnd->GetControl(_T("FRM_GOLD"));
			XUI::CXUI_Wnd *pkSilverFrm = pkNewItemWnd->GetControl(_T("FRM_SILVER"));
			XUI::CXUI_Wnd *pkBronzeFrm = pkNewItemWnd->GetControl(_T("FRM_BRONZE"));
			if( pkGoldFrm )		{ pkGoldFrm->Text(kMoneyVec.at(0)); }
			if( pkSilverFrm )	{ pkSilverFrm->Text(kMoneyVec.at(1)); }
			if( pkBronzeFrm )	{ pkBronzeFrm->Text(kMoneyVec.at(2)); }
		}

		// 스킬 아이콘
		XUI::CXUI_Icon *pkSkillIcon = dynamic_cast<XUI::CXUI_Icon*>(pkNewItemWnd->GetControl(_T("ICON_SKILL")));
		if( pkSkillIcon )
		{
			SIconInfo kSkillIconInfo(KUIG_SKILLTREE, rkDefGuildSkill.iSkillNo);
			kSkillIconInfo.iIconResNumber = pkSkillDef->RscNameNo();
			pkSkillIcon->SetIconInfo(kSkillIconInfo);
			pkSkillIcon->GrayScale(!bCanBuySkill);
		}

		XUI::CXUI_Button *pkBuyBtn = dynamic_cast<XUI::CXUI_Button*>(pkNewItemWnd->GetControl(_T("BTN_BUY")));
		if( pkBuyBtn )
		{
			pkBuyBtn->SetCustomData( &rkDefGuildSkill.iSkillNo, sizeof(rkDefGuildSkill.iSkillNo) );
			pkBuyBtn->Visible(bCanBuySkill && !bExitSkill);
		}
	}

	XUI::CXUI_Wnd *pkEmptyWnd = pkTopWnd->GetControl(_T("SFRM_NULL"));
	if(	pkEmptyWnd )
	{
		pkEmptyWnd->Visible( 0 == pkListWnd->GetTotalItemCount() );
	}
}

void lwGuild::ShowSingleResultWindow(SIconInfo const &rkIconInfo, std::wstring const &rkNotice, std::wstring const &rkText, const bool bGrayScale, PgBase_Item const *pkItem)
{
	XUI::CXUI_Wnd *pkTopWnd = XUIMgr.Call(_T("FRM_SINGLE_ICON_RESULT"));
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd *pkTextTitle = pkTopWnd->GetControl(_T("TEXT_TITLE"));
	if( pkTextTitle )
	{
		pkTextTitle->Text(rkNotice);
	}

	XUI::CXUI_Wnd *pkTextTitle2 = pkTopWnd->GetControl(_T("TEXT_TITLE2"));
	if( pkTextTitle2 )
	{
		pkTextTitle2->Text(rkText);
	}

	XUI::CXUI_Icon *pkIconWnd = dynamic_cast<XUI::CXUI_Icon*>( pkTopWnd->GetControl(_T("ICON_RESULT")) );
	if( pkIconWnd )
	{
		pkIconWnd->SetIconInfo(rkIconInfo);
		pkIconWnd->GrayScale(bGrayScale);

		if( pkItem )
		{
			BM::Stream kCustomData;
			pkItem->WriteToPacket(kCustomData);

			pkIconWnd->SetCustomData(kCustomData.Data());
		}
	}
}

bool lwGuild::IsHaveGuild(lwGUID kCharGuid)
{
	PgPilot *pkPilot = g_kPilotMan.FindPilot(kCharGuid());
	if( !pkPilot )
	{
		return false;
	}
	
	PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
	if( !pkPlayer )
	{
		return false;
	}
	return BM::GUID::IsNotNull(pkPlayer->GuildGuid());
}

void lwGuild::Send_CreateGuildTest()
{
	if( g_kGuildMgr.IsHaveEntranceGuild() )
	{// 가입 신청한 길드가 존재한다.		
		::Notice_Show( TTW(401086), EL_Warning );
		return ;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_PreCreate_Test);
	NETWORK_SEND(kPacket)
}

void lwGuild::SetTempGuildEmblem(int iEmblem)
{
	g_kGuildMgr.TempGuildEmblem(iEmblem);
}

int lwGuild::GetTempGuildEmblem()
{
	return g_kGuildMgr.TempGuildEmblem();
}

void lwGuild::SetGuildListMode(int iMode)
{
	if( GLM_None >= iMode
	||	GLM_Max <= iMode )
	{
		return;
	}
	g_kGuildMgr.ListMode( (EGuildListMode)iMode );
}

void lwGuild::SetGuildNotice(lwWString kNewNotice)
{
	std::wstring const &rkNewNotice = kNewNotice();
	if( g_kGuildMgr.GuildNotice() == rkNewNotice )
	{
		return;
	}
	if( !AmIGuildMaster() )
	{
		return;
	}

	std::wstring kTempNewNotice = rkNewNotice;
	if( g_kClientFS.Filter(kTempNewNotice, false, FST_BADWORD) 
		|| !g_kUnicodeFilter.IsCorrect(UFFC_GUILD_NOTICE, kTempNewNotice)
		)
	{
		int const iTTWBadWord = 700144;
		SChatLog kChatLog(CT_EVENT);
		g_kChatMgrClient.AddMessage(iTTWBadWord, kChatLog, true);
		return;
	}
	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND);
	kPacket.Push( (BYTE)GC_M_Notice );
	kPacket.Push( rkNewNotice );
	NETWORK_SEND(kPacket)
}

lwWString lwGuild::GetGuildNotice()
{
	return lwWString(g_kGuildMgr.GuildNotice());
}

BYTE lwGuild::GetMyEmporiaKey()
{
	return g_kGuildMgr.GetEmporiaInfo().byType;
}

void lwGuild::EmporiaGiveUp()
{
	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_ReserveBattleForEmporia) );
	kPacket.Push(true);// Drop
	NETWORK_SEND( kPacket )
}

bool lwGuild::Req_JoinEmporia()
{
	using namespace PgClientPartyUtil;
	if ( true == IsInParty() )
	{
		Notice_Show( TTW(19013), EL_Warning );
		return false;
	}

	T_GNDATTR const kGndAttr = ( g_pkWorld ? g_pkWorld->GetAttr() : GATTR_DEFAULT );
	bool bOK = false;

	SGuildEmporiaInfo const &kEmporiaInfo = g_kGuildMgr.GetEmporiaInfo();
	switch ( kEmporiaInfo.byType )
	{
	case EMPORIA_KEY_MINE:
		{
			if ( GATTR_EMPORIABATTLE == kGndAttr )
			{
				bOK = true;
				break;
			}
		} // break 사용을 하지 않는다.
	case EMPORIA_KEY_BATTLE_ATTACK:
	case EMPORIA_KEY_BATTLE_DEFENCE:
		{
			bOK = ( (GATTR_EMPORIA == kGndAttr) || !(GATTR_INSTANCE & kGndAttr ) );
		}break;
	case EMPORIA_KEY_NONE:
	case EMPORIA_KEY_BATTLERESERVE:
	case EMPORIA_KEY_BATTLECONFIRM:
	default:
		{
		}break;
	}

	if ( true == bOK )
	{
		if ( true == g_kPilotMan.IsLockMyInput() )
		{
			::Notice_Show( TTW(400858), EL_Warning );
			return false;
		}

		BM::Stream kPacket( PT_C_M_REQ_JOIN_EMPORIA, kEmporiaInfo.GetKey() );
		kPacket.Push( kEmporiaInfo.byGrade );

		NETWORK_SEND( kPacket );
	}

	return bOK;
}

void lwGuild::lwReq_FollowEmporiaStartJoin()
{
	SGuildEmporiaInfo const &kEmporiaInfo = g_kGuildMgr.GetEmporiaInfo();
	__int64 i64NowTime = g_kEventView.GetLocalSecTime(CGameTime::DEFAULT);
	float fRemainTime = (kEmporiaInfo.i64BattleTime-i64NowTime) / CGameTime::SECOND * 1.f;
	fRemainTime -= 4.f;//실제 게임시작 4초 전에 입장(한명도 없는 상태에서 게임에 진입하려고 하면 게임 실패가 될 수 있음)
	if( fRemainTime<0 ){ fRemainTime = 0.1f; }

	STimerEvent kTimerEvent;
	kTimerEvent.Set(fRemainTime, "Req_JoinEmporia()");
	g_kEventTimer.AddLocal(EVENT_EM_START_JOIN, kTimerEvent);
}

bool lwGuild::IsHaveEmporiaFunc( short nEmporiaFuncNo )
{
	return g_kEmporiaMgr.IsHaveFunction( nEmporiaFuncNo );
}

void lwGuild::KeepEmporiaFunc( short nEmporiaFuncNo )
{
	SCalcInfo kInfo;
	kInfo.eCallType = CCT_EMWEEK;
	kInfo.iIndex = (int)nEmporiaFuncNo;
	kInfo.iMaxValue = 10;
	CallCalculator( kInfo );
}

wchar_t const* const SZ_MERCENARY_SETTING_UI = L"FRM_MERCENARY_SETTING";
wchar_t const* const SZ_MERCENARY_USE_OK = L"CBTN_USE_MER_OK";
wchar_t const* const SZ_MERCENARY_USE_NO = L"CBTN_USE_MER_NO";
wchar_t const* const SZ_MERCENARY_LIMIT_LV = L"FRM_LEVEL";
wchar_t const* const SZ_MERCENARY_LIMIT_CLASS_F = L"CBTN_JOB_F";
wchar_t const* const SZ_MERCENARY_LIMIT_CLASS_M = L"CBTN_JOB_M";
wchar_t const* const SZ_MERCENARY_LIMIT_CLASS_A = L"CBTN_JOB_A";
wchar_t const* const SZ_MERCENARY_LIMIT_CLASS_T = L"CBTN_JOB_T";
wchar_t const* const SZ_MERCENARY_LIMIT_CLASS_S = L"CBTN_JOB_S";
wchar_t const* const SZ_MERCENARY_LIMIT_CLASS_D = L"CBTN_JOB_D";

int const EMPORIA_MERCENARY_JOIN_DEFAULT_LEVEL = 20;

void lwGuild::SetMercenaryUIBtn(XUI::CXUI_Wnd* pkBtn, bool const bIsAccess, bool const bClickLock)
{
	XUI::CXUI_CheckButton* pLimitBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkBtn);
	if( !pLimitBtn )
	{
		return;	
	}

	if( pLimitBtn->Enable() )
	{
		pLimitBtn->ClickLock(false);
		pLimitBtn->Check(bIsAccess);
		if( bClickLock )
		{
			pLimitBtn->ClickLock(bIsAccess);
		}
	}
}

void lwGuild::SetMercenaryUIBtnDisable(XUI::CXUI_Wnd* pkBtn, bool const bDisable)
{
	XUI::CXUI_CheckButton* pLimitBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkBtn);
	if( !pLimitBtn )
	{
		return;	
	}

	pLimitBtn->Disable(bDisable);
}

void lwGuild::GuildEntranceProcess(bool const bAccept)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_GUILD_APPLICANT_ACCEPT_REJECT");
	if( !pkMainUI )
	{
		pkMainUI = XUIMgr.Call(L"SFRM_GUILD_APPLICANT_ACCEPT_REJECT");
		if( !pkMainUI )
		{
			return ;
		}
	}

	XUI::CXUI_Wnd* pkHiddenCharGuid = pkMainUI->GetControl(L"FRM_HIDDEN_CHARGUID");
	if( !pkHiddenCharGuid )
	{
		return ;
	}

	BM::GUID kCharGuid;
	pkHiddenCharGuid->GetCustomData(&kCharGuid, sizeof(kCharGuid));

	BYTE byState = AS_NONE;

	if( !g_kGuildMgr.IsMaster( pkPlayer->GetID() ) )
	{
		::Notice_Show( TTW(201506), EL_Warning );
		return ;
	}

	if( bAccept )
	{
		byState = AS_ACCEPT;
		BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_GuildEntranceApplicant_Accept);
		kPacket.Push(kCharGuid);
		NETWORK_SEND(kPacket);
	}	
	else
	{
		byState = AS_REJECT;
		BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_GuildEntranceApplicant_Reject);
		kPacket.Push(kCharGuid);
		NETWORK_SEND(kPacket);
	}
}

void lwGuild::CallGuildApplicantAcceptReject(lwUIWnd kSelf)
{
	XUI::CXUI_Button* pSelf = dynamic_cast<XUI::CXUI_Button*>(kSelf.GetSelf());
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_GUILD_APPLICANT_ACCEPT_REJECT");
	if( !pkMainUI )
	{
		pkMainUI = XUIMgr.Call(L"SFRM_GUILD_APPLICANT_ACCEPT_REJECT");
		if( !pkMainUI )
		{
			return ;
		}
	}

	XUI::CXUI_Wnd* pkHiddenCharGuid = pkMainUI->GetControl(L"FRM_HIDDEN_CHARGUID");
	XUI::CXUI_Wnd* pkName = pkMainUI->GetControl(L"FRM_NAME");
	XUI::CXUI_Wnd* pkTime = pkMainUI->GetControl(L"FRM_TIME");	
	XUI::CXUI_Edit_MultiLine* pkEditMsg = dynamic_cast<XUI::CXUI_Edit_MultiLine*>(pkMainUI->GetControl(L"MEDT_CONTENTS"));
	
	if( !pkHiddenCharGuid
	||	!pkName
	||	!pkTime	
	||	!pkEditMsg )
	{
		return ;
	}

	BM::GUID kCharGuid;
	pSelf->GetCustomData(&kCharGuid, sizeof(kCharGuid));	
	pkHiddenCharGuid->SetCustomData(&kCharGuid, sizeof(kCharGuid)); // 수락, 거절할때 필요한 캐릭터 Guid

	CONT_MY_GUILD_ENTRANCE_APPLICANTS const kGuildEntranceApplicant = g_kGuildMgr.GetGuildEntranceApplicantList();
	CONT_MY_GUILD_ENTRANCE_APPLICANTS::const_iterator find_iter = kGuildEntranceApplicant.find(kCharGuid);
	if( find_iter != kGuildEntranceApplicant.end() )
	{
		CONT_MY_GUILD_ENTRANCE_APPLICANTS::mapped_type kApplicant = (*find_iter).second;	

		BM::vstring vStrTime(TTW(71056));
		vStrTime += L" ";
		vStrTime += TTW(71057);
		vStrTime.Replace( L"#YEAR#", kApplicant.kDateTime.year );
		vStrTime.Replace( L"#MONTH#", BM::vstring(kApplicant.kDateTime.month, L"%02u") );
		vStrTime.Replace( L"#DAY#", BM::vstring(kApplicant.kDateTime.day, L"%02u") );
		vStrTime.Replace( L"#HOUR#", BM::vstring(kApplicant.kDateTime.hour, L"%02u") );
		vStrTime.Replace( L"#MIN#", BM::vstring(kApplicant.kDateTime.minute, L"%02u") );

		pkName->Text(kApplicant.wstrName);
		pkTime->Text(vStrTime);
		pkEditMsg->EditText(kApplicant.wstrMessage);
	}
}

void lwGuild::CallGuildSettingUI()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( BM::GUID::IsNull(pkPlayer->GuildGuid()) )
	{
		return;
	}

	if( !XUIMgr.Get(L"SFRM_GUILD_SETTING") )
	{
		if( XUIMgr.Call(L"SFRM_GUILD_SETTING") )
		{
			GuildEntranceSetting();
		}
		else
		{
			return ;
		}
	}
}

void lwGuild::OnClickChangeGuildSettingTab(lwUIWnd kSelf, int const iBuildIndex)
{
	XUI::CXUI_CheckButton* pSelf = dynamic_cast<XUI::CXUI_CheckButton*>(kSelf.GetSelf());
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_GUILD_SETTING");
	if( !pkMainUI )
	{
		return ;
	}

	for( int iType = EGSUT_GUILD_ENTRANCE; iType <= EGSUT_MERCENARY; ++iType )
	{
		BM::vstring vStr(L"CBTN_GUILD_SETTING");
		vStr += iType;

		XUI::CXUI_Wnd* pkGuildTab = NULL;

		switch( iType )
		{
		case EGSUT_GUILD_ENTRANCE: { pkGuildTab = pkMainUI->GetControl(L"FRM_ENTRANCE_SETTING"); }break;
		case EGSUT_GUILD_APPLICANT: { pkGuildTab = pkMainUI->GetControl(L"FRM_ENTRANCE_APPLICANT"); }break;
		case EGSUT_MERCENARY: { pkGuildTab = pkMainUI->GetControl(L"FRM_MERCENARY_SETTING"); }break;
		}

		if( pkGuildTab )
		{
			XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vStr));
			if( pCheckBtn )
			{
				if( iBuildIndex == iType )
				{
					if( pCheckBtn->Check()
					||	pCheckBtn->ClickLock() )
					{
						return ;
					}

					pkGuildTab->Visible(true);
					pCheckBtn->Check(true);
					pCheckBtn->ClickLock(true);
				}
				else
				{
					pkGuildTab->Visible(false);
					pCheckBtn->ClickLock(false);
					pCheckBtn->Check(false);
				}
			}	
		}
	}

	switch( iBuildIndex )
	{
	case EGSUT_GUILD_ENTRANCE:
		{
			GuildEntranceSetting();
		}break;
	case EGSUT_GUILD_APPLICANT:
		{
			GuildApplicantSetting();
		}break;
	case EGSUT_MERCENARY:
		{
			MercenarySetting();
		}break;
	default:
		{

		}break;
	}
}

// 길드 가입 UI
void lwGuild::GuildEntranceSetting()
{
	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_GUILD_SETTING");
	if( !pkMainUI )
	{
		return ;
	}

	XUI::CXUI_Wnd* pkGuildEntranceUI = pkMainUI->GetControl(L"FRM_ENTRANCE_SETTING");
	if( !pkGuildEntranceUI )
	{
		return ;
	}

	pkGuildEntranceUI->Visible(true);
	SSetGuildEntrancedOpen const& kGuildEntranceOpen = g_kGuildMgr.GetGuildEntranceOpenInfo();
	
	XUI::CXUI_Wnd* pkLevel = pkGuildEntranceUI->GetControl(L"FRM_LEVEL");
	if( pkLevel )
	{
		BM::vstring vStr(TTW(71554));
		short const iLevel = (0 == kGuildEntranceOpen.sGuildEntranceLevel)?(1):(kGuildEntranceOpen.sGuildEntranceLevel);
		vStr.Replace(L"#LEVEL#", iLevel);
		pkLevel->Text(vStr);
		pkLevel->SetCustomData(&iLevel, sizeof(iLevel));
	}

	SetMercenaryUIBtn(pkGuildEntranceUI->GetControl(L"CBTN_GUILD_ENTRANCE_OK"), kGuildEntranceOpen.bIsGuildEntrance, true);
	SetMercenaryUIBtn(pkGuildEntranceUI->GetControl(L"CBTN_GUILD_ENTRANCE_NO"), !kGuildEntranceOpen.bIsGuildEntrance, true);

	SetMercenaryUIBtn(pkGuildEntranceUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_F), (kGuildEntranceOpen.i64GuildEntranceClass & UCLIMIT_MARKET_FIGHTER) == UCLIMIT_MARKET_FIGHTER);
	SetMercenaryUIBtn(pkGuildEntranceUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_M), (kGuildEntranceOpen.i64GuildEntranceClass & UCLIMIT_MARKET_MAGICIAN) == UCLIMIT_MARKET_MAGICIAN);
	SetMercenaryUIBtn(pkGuildEntranceUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_A), (kGuildEntranceOpen.i64GuildEntranceClass & UCLIMIT_MARKET_ARCHER) == UCLIMIT_MARKET_ARCHER);
	SetMercenaryUIBtn(pkGuildEntranceUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_T), (kGuildEntranceOpen.i64GuildEntranceClass & UCLIMIT_MARKET_THIEF) == UCLIMIT_MARKET_THIEF);
	SetMercenaryUIBtn(pkGuildEntranceUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_S), (kGuildEntranceOpen.i64GuildEntranceClass & UCLIMIT_MARKET_SHAMAN) == UCLIMIT_MARKET_SHAMAN);
	SetMercenaryUIBtn(pkGuildEntranceUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_D), (kGuildEntranceOpen.i64GuildEntranceClass & UCLIMIT_MARKET_DOUBLE_FIGHTER) == UCLIMIT_MARKET_DOUBLE_FIGHTER);

	SetDisableJobSelectBtn(pkGuildEntranceUI, !kGuildEntranceOpen.bIsGuildEntrance);
	
	typedef std::vector< std::wstring > ContWstr;
	ContWstr kMoneyVec;
	if(BreakMoney_WStr(kGuildEntranceOpen.i64GuildEntranceFee, kMoneyVec))
	{
		XUI::CXUI_Edit* pkEditGold = dynamic_cast<XUI::CXUI_Edit*>(pkGuildEntranceUI->GetControl(L"EDT_SELL_GOLD"));
		if( !pkEditGold )
		{
			return ;
		}

		pkEditGold->EditText(kMoneyVec.at(0));
	}

	XUI::CXUI_Edit* pkEditGuildPR = dynamic_cast<XUI::CXUI_Edit*>(pkGuildEntranceUI->GetControl(L"EDT_GUILD_PR"));
	if( pkEditGuildPR)
	{
		pkEditGuildPR->EditText(kGuildEntranceOpen.wstrGuildPR);
	}
}

void lwGuild::GuildApplicantSetting()
{
	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_GUILD_SETTING");
	if( !pkMainUI )
	{
		return ;
	}

	XUI::CXUI_Wnd* pkApplicant = pkMainUI->GetControl(L"FRM_ENTRANCE_APPLICANT");
	if( !pkApplicant )
	{
		return ;
	}

	XUI::CXUI_List* pkListEntranceApplicant = dynamic_cast<XUI::CXUI_List*>(pkApplicant->GetControl(L"LST_ENTRANCE_APPLICANT"));
	if( pkListEntranceApplicant )
	{
		pkListEntranceApplicant->ClearList();

		CONT_MY_GUILD_ENTRANCE_APPLICANTS const& kContGuildEntranceApplicant = g_kGuildMgr.GetGuildEntranceApplicantList();
		CONT_MY_GUILD_ENTRANCE_APPLICANTS::const_iterator kApplicant_iter = kContGuildEntranceApplicant.begin();
		
		while( kApplicant_iter != kContGuildEntranceApplicant.end() )
		{
			CONT_MY_GUILD_ENTRANCE_APPLICANTS::mapped_type const kGuildEntranceApplicant = (*kApplicant_iter).second;
			XUI::SListItem* pkNewItemElement = NULL;

			pkNewItemElement = pkListEntranceApplicant->AddItem( std::wstring() );
			
			if( pkNewItemElement )
			{
				XUI::CXUI_Wnd* pkNewItemWnd = pkNewItemElement->m_pWnd;
				if( pkNewItemWnd )
				{
					XUI::CXUI_Wnd* pkClass = pkNewItemWnd->GetControl(L"IMG_CLASS");
					XUI::CXUI_Wnd* pkLevel = pkNewItemWnd->GetControl(L"FRM_LEVEL");
					XUI::CXUI_Wnd* pkName = pkNewItemWnd->GetControl(L"FRM_NAME");
					XUI::CXUI_Button* pkDetail = dynamic_cast<XUI::CXUI_Button*>(pkNewItemWnd->GetControl(L"BTN_DETAIL"));

					if( !pkClass || !pkLevel || !pkNewItemWnd || !pkDetail )
					{
						return ;
					}

					lwSetMiniClassIconIndex(pkClass, kGuildEntranceApplicant.byClass);
					BM::vstring kLevel(kGuildEntranceApplicant.sLevel);
					pkLevel->Text(kLevel);
					pkName->Text(kGuildEntranceApplicant.wstrName);
					pkDetail->SetCustomData(&(*kApplicant_iter).second.kCharGuid, sizeof((*kApplicant_iter).second.kCharGuid));

					if( kGuildEntranceApplicant.byState == AS_ACCEPT )
					{
						pkDetail->Text(TTW(400501));
						pkDetail->Disable(true);
					}
				}
			}

			++kApplicant_iter;
		}
	}
}

void lwGuild::MercenarySetting()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( BM::GUID::IsNull(pkPlayer->GuildGuid()) )
	{
		return;
	}

	PgLimitClass const& kMercenary = g_kGuildMgr.GetMercenaryInfo();

	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_GUILD_SETTING");
	if( !pkMainUI )
	{
		pkMainUI = XUIMgr.Call(L"SFRM_GUILD_SETTING");
		if( !pkMainUI )
		{
			return ;
		}
	}

	XUI::CXUI_Wnd* pkMercenaryUI = pkMainUI->GetControl(SZ_MERCENARY_SETTING_UI);
	if( !pkMercenaryUI )
	{
		return;
	}

	XUI::CXUI_Wnd* pkLevel = pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_LV);
	if( pkLevel )
	{
		BM::vstring vStr(TTW(71554));
		short const iLevel = (0 == kMercenary.GetLevel())?(EMPORIA_MERCENARY_JOIN_DEFAULT_LEVEL):(kMercenary.GetLevel());
		vStr.Replace(L"#LEVEL#", iLevel);
		pkLevel->Text(vStr);
		pkLevel->SetCustomData(&iLevel, sizeof(iLevel));
	}

	SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_USE_OK), kMercenary.IsUsing(), true);
	SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_USE_NO), !kMercenary.IsUsing(), true);

	SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_F), (kMercenary.GetClass() & UCLIMIT_MARKET_FIGHTER) == UCLIMIT_MARKET_FIGHTER);
	SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_M), (kMercenary.GetClass() & UCLIMIT_MARKET_MAGICIAN) == UCLIMIT_MARKET_MAGICIAN);
	SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_A), (kMercenary.GetClass() & UCLIMIT_MARKET_ARCHER) == UCLIMIT_MARKET_ARCHER);
	SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_T), (kMercenary.GetClass() & UCLIMIT_MARKET_THIEF) == UCLIMIT_MARKET_THIEF);
	SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_S), (kMercenary.GetClass() & UCLIMIT_MARKET_SHAMAN) == UCLIMIT_MARKET_SHAMAN);
	SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_D), (kMercenary.GetClass() & UCLIMIT_MARKET_DOUBLE_FIGHTER) == UCLIMIT_MARKET_DOUBLE_FIGHTER);

	SetDisableJobSelectBtn(pkMercenaryUI, !kMercenary.IsUsing());
}

void lwGuild::OnCallGuildEntranceLevelDrop(lwUIWnd kSelf, int const iMaxLevel)
{
	wchar_t const* const SZ_DROP_WINDOW_NAME = L"SFRM_GUILD_JOIN_LEVEL_DROP";
	wchar_t const* const SZ_DROP_LIST = L"LST_ITEMS";
	wchar_t const* const SZ_DROP_BORDER = L"SFRM_BORDER";

	int const MAX_VIEW_LIST_ITEM = 5;

	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	XUI::CXUI_Wnd* pDrop = XUIMgr.Get(SZ_DROP_WINDOW_NAME);
	if( !pDrop || pDrop->IsClosed() )
	{
		pDrop = XUIMgr.Call(SZ_DROP_WINDOW_NAME);
	}
	else
	{
		pDrop->Close();
		return;
	}

	XUI::CXUI_Wnd* pkBorder = pDrop->GetControl(SZ_DROP_BORDER);
	if( !pkBorder )
	{
		pDrop->Close();
		return;
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pDrop->GetControl(SZ_DROP_LIST));
	if( !pkList )
	{
		pDrop->Close();
		return;
	}

	pkList->DeleteAllItem();

	int iItemHeight = 0;
	for( short i = 0; i < iMaxLevel; i += 10 )
	{
		XUI::SListItem* pkItem = pkList->AddItem(L"");
		if( pkItem && pkItem->m_pWnd )
		{
			BM::vstring vStr(TTW(401084));
			if( i != 0 )
			{
				vStr.Replace(L"#LEVEL#", i);
			}
			else
			{
				vStr.Replace(L"#LEVEL#", i + 1);
			}
			pkItem->m_pWnd->Text(vStr);
			pkItem->m_pWnd->SetCustomData(&i, sizeof(i));
			iItemHeight = pkItem->m_pWnd->Size().y;
		}
	}

	int const iItemCount = pkList->GetTotalItemCount();
	int TotalListHeight = iItemHeight * iItemCount;
	if( MAX_VIEW_LIST_ITEM < iItemCount )
	{
		TotalListHeight = iItemHeight * MAX_VIEW_LIST_ITEM;
	}

	POINT3I kPoint = pParent->TotalLocation();
	kPoint.y += pParent->Size().y;
	pDrop->Location(kPoint);
	pkList->Size(POINT2(pkList->Size().x, TotalListHeight));
	pDrop->Size(POINT2(pDrop->Size().x, pkList->Size().y + 6));
	pkBorder->Size(POINT2(pDrop->Size().x, pDrop->Size().y + 2));
}

void lwGuild::OnCallMercenaryJoinLevelDrop(lwUIWnd kSelf, int const iMaxLevel)
{
	wchar_t const* const SZ_DROP_WINDOW_NAME = L"SFRM_MERCENARY_JOIN_LEVEL_DROP";
	wchar_t const* const SZ_DROP_LIST = L"LST_ITEMS";
	wchar_t const* const SZ_DROP_BORDER = L"SFRM_BORDER";

	int const MAX_VIEW_LIST_ITEM = 5;

	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	XUI::CXUI_Wnd* pDrop = XUIMgr.Get(SZ_DROP_WINDOW_NAME);
	if( !pDrop || pDrop->IsClosed() )
	{
		pDrop = XUIMgr.Call(SZ_DROP_WINDOW_NAME);
	}
	else
	{
		pDrop->Close();
		return;
	}

	XUI::CXUI_Wnd* pkBorder = pDrop->GetControl(SZ_DROP_BORDER);
	if( !pkBorder )
	{
		pDrop->Close();
		return;
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pDrop->GetControl(SZ_DROP_LIST));
	if( !pkList )
	{
		pDrop->Close();
		return;
	}

	pkList->DeleteAllItem();

	int iItemHeight = 0;
	for( short i = EMPORIA_MERCENARY_JOIN_DEFAULT_LEVEL; i < iMaxLevel; i += 10 )
	{
		XUI::SListItem* pkItem = pkList->AddItem(L"");
		if( pkItem && pkItem->m_pWnd )
		{
			BM::vstring vStr(TTW(71554));
			vStr.Replace(L"#LEVEL#", i);
			pkItem->m_pWnd->Text(vStr);
			pkItem->m_pWnd->SetCustomData(&i, sizeof(i));
			iItemHeight = pkItem->m_pWnd->Size().y;
		}
	}

	int const iItemCount = pkList->GetTotalItemCount();
	int TotalListHeight = iItemHeight * iItemCount;
	if( MAX_VIEW_LIST_ITEM < iItemCount )
	{
		TotalListHeight = iItemHeight * MAX_VIEW_LIST_ITEM;
	}

	POINT3I kPoint = pParent->TotalLocation();
	kPoint.y += pParent->Size().y;
	pDrop->Location(kPoint);
	pkList->Size(POINT2(pkList->Size().x, TotalListHeight));
	pDrop->Size(POINT2(pDrop->Size().x, pkList->Size().y + 6));
	pkBorder->Size(POINT2(pDrop->Size().x, pDrop->Size().y + 2));
}

void lwGuild::SetDisableJobSelectBtn(XUI::CXUI_Wnd* pMainUI, bool const bDisable)
{
	if( !pMainUI )
	{
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( BM::GUID::IsNull(pkPlayer->GuildGuid()) )
	{
		return;
	}

	SetMercenaryUIBtnDisable(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_F), bDisable);
	SetMercenaryUIBtnDisable(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_M), bDisable);
	SetMercenaryUIBtnDisable(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_A), bDisable);
	SetMercenaryUIBtnDisable(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_T), bDisable);
	SetMercenaryUIBtnDisable(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_S), bDisable);
	SetMercenaryUIBtnDisable(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_D), bDisable);
}

void lwGuild::OnClickGuildEntranceSetBtn(lwUIWnd kSelf, int const Type)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
	if( !pMainUI || pMainUI->ID().compare(L"FRM_ENTRANCE_SETTING") != 0 )
	{
		XUI::CXUI_Wnd* pkTop = XUIMgr.Get(L"SFRM_GUILD_SETTING");
		if( pkTop )
		{
			pMainUI = pkTop->GetControl(L"FRM_ENTRANCE_SETTING");
			if( !pMainUI )
			{
				return;
			}
		}
	}

	XUI::CXUI_CheckButton* pkChangeBtn = NULL;
	switch( Type )
	{
	case EGEBT_ENTRANCE_OK:
	case EGEBT_ENTRANCE_NO:
		{
			bool bUseOkClicked = (Type == EMSBT_USE_OK)?(true):(false);

			SetMercenaryUIBtn(pMainUI->GetControl(L"CBTN_GUILD_ENTRANCE_OK"), bUseOkClicked, true);
			SetMercenaryUIBtn(pMainUI->GetControl(L"CBTN_GUILD_ENTRANCE_NO"), !bUseOkClicked, true);

			if( bUseOkClicked )
			{
				SetDisableJobSelectBtn(pMainUI, false);
			}
			else
			{
				SetDisableJobSelectBtn(pMainUI, true);
			}
			break;
		}break;
	case EGEBT_LEVEL:
		{
			short iTargetLv = 0;
			pSelf->GetCustomData(&iTargetLv, sizeof(iTargetLv));
			if( !iTargetLv )
			{
				iTargetLv = 1;
			}

			XUI::CXUI_Wnd* pkLevel = pMainUI->GetControl(L"FRM_LEVEL");
			if( pkLevel )
			{
				BM::vstring vStr(TTW(71554));
				vStr.Replace(L"#LEVEL#", iTargetLv);
				pkLevel->Text(vStr);
				pkLevel->SetCustomData(&iTargetLv, sizeof(iTargetLv));
			}	
			return;
		}break;
	case EGEBT_ALL_JOB:
		{
			SetMercenaryUIBtn(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_F), true);
			SetMercenaryUIBtn(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_M), true);
			SetMercenaryUIBtn(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_A), true);
			SetMercenaryUIBtn(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_T), true);
			SetMercenaryUIBtn(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_S), true);
			SetMercenaryUIBtn(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_D), true);
			return;
		}break;
	}
}

void lwGuild::OnClickMercenarySetBtn(lwUIWnd kSelf, int const Type)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pkMainUI = pSelf->Parent();
	if( !pkMainUI || pkMainUI->ID().compare(L"SFRM_GUILD_SETTING") != 0 )
	{
		pkMainUI = XUIMgr.Get(L"SFRM_GUILD_SETTING");
		if( !pkMainUI )
		{
			return;
		}
	}

	XUI::CXUI_Wnd* pkMercenaryUI = pkMainUI->GetControl(SZ_MERCENARY_SETTING_UI);
	if( !pkMercenaryUI )
	{
		return ;
	}

	XUI::CXUI_CheckButton* pkChangeBtn = NULL;
	switch( Type )
	{
	case EMSBT_USE_OK:
	case EMSBT_USE_NO:
		{
			bool bUseOkClicked = (Type == EMSBT_USE_OK)?(true):(false);

			SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_USE_OK), bUseOkClicked, true);
			SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_USE_NO), !bUseOkClicked, true);

			if( bUseOkClicked )
			{
				SetDisableJobSelectBtn(pkMercenaryUI, false);
			}
			else
			{
				SetDisableJobSelectBtn(pkMercenaryUI, true);
			}
			break;
		}break;
	case EMSBT_LEVEL:
		{
			short iTargetLv = 0;
			pSelf->GetCustomData(&iTargetLv, sizeof(iTargetLv));

			XUI::CXUI_Wnd* pkLevel = pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_LV);
			if( pkLevel )
			{
				BM::vstring vStr(TTW(71554));
				vStr.Replace(L"#LEVEL#", iTargetLv);
				pkLevel->Text(vStr);
				pkLevel->SetCustomData(&iTargetLv, sizeof(iTargetLv));
			}	
			return;
		}break;
	case EMSBT_ALL_JOB:
		{
			SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_F), true);
			SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_M), true);
			SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_A), true);
			SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_T), true);
			SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_S), true);
			SetMercenaryUIBtn(pkMercenaryUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_D), true);
			return;
		}break;
	}
}

void lwGuild::OnClickGuildEntranceSetOK(lwUIWnd kParent)
{
	XUI::CXUI_Wnd* pMainUI = kParent.GetSelf();
	if( !pMainUI )
	{
		return;
	}

	const SSetGuildEntrancedOpen kOrgGuildEntranceOpen = g_kGuildMgr.GetGuildEntranceOpenInfo();

	SSetGuildEntrancedOpen kGuildEntranceOpen;
	XUI::CXUI_Wnd* pkLevel = pMainUI->GetControl(L"FRM_LEVEL");
	if( pkLevel )
	{
		short iTargetLv = 0;
		pkLevel->GetCustomData(&iTargetLv, sizeof(iTargetLv));
		kGuildEntranceOpen.sGuildEntranceLevel = iTargetLv; // 길드가입 허용 레벨
	}

	__int64	i64LimitClass = 0i64;

	XUI::CXUI_CheckButton* pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(L"CBTN_JOB_F"));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_FIGHTER;
	}
	pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(L"CBTN_JOB_M"));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_MAGICIAN;
	}
	pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(L"CBTN_JOB_A"));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_ARCHER;
	}
	pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(L"CBTN_JOB_T"));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_THIEF;
	}
	pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(L"CBTN_JOB_S"));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_SHAMAN;
	}
	pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(L"CBTN_JOB_D"));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_DOUBLE_FIGHTER;
	}

	kGuildEntranceOpen.i64GuildEntranceClass = i64LimitClass; // 클래스 제한

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_SetGuildEntranceOpen)); // 길드가입 세팅

	XUI::CXUI_CheckButton* pkEntranceOK = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(L"CBTN_GUILD_ENTRANCE_OK"));
	XUI::CXUI_CheckButton* pkEntranceNO = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(L"CBTN_GUILD_ENTRANCE_NO"));
	if( pkEntranceOK && pkEntranceNO )
	{
		if( pkEntranceOK->Check() )
		{
			kGuildEntranceOpen.bIsGuildEntrance = true; // 길드가입 신청 가능 여부

			XUI::CXUI_Edit* pkEditGold = dynamic_cast<XUI::CXUI_Edit*>(pMainUI->GetControl(L"EDT_SELL_GOLD"));			
			if( pkEditGold )
			{
				std::wstring wstrVal;
				__int64 i64Val = 0;

				wstrVal = pkEditGold->Text();
				i64Val = (_wtoi64(wstrVal.c_str()) * 10000);
					
				kGuildEntranceOpen.i64GuildEntranceFee = i64Val; // 길드가입비용
			}

			XUI::CXUI_Edit* pkEditGuildPR = dynamic_cast<XUI::CXUI_Edit*>(pMainUI->GetControl(L"EDT_GUILD_PR"));
			if( pkEditGuildPR )
			{
				std::wstring kTemp = pkEditGuildPR->Text();

				if(g_kClientFS.Filter(kTemp, false))
				{// 욕설 필터링
					::Notice_Show( TTW(600037), 0 );
					return ;
				}

				kGuildEntranceOpen.wstrGuildPR = kTemp; // 길드소개
			}

			kGuildEntranceOpen.WriteToPacket(kPacket);
			if( !kGuildEntranceOpen.i64GuildEntranceClass )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 71563, true);
				return;
			}
		}
		else if( pkEntranceNO->Check() )		
		{
			kGuildEntranceOpen.bIsGuildEntrance = false;			
			kGuildEntranceOpen.WriteToPacket(kPacket);
		}
	}

	if( kOrgGuildEntranceOpen != kGuildEntranceOpen )
	{
		NETWORK_SEND(kPacket);
	}

	XUIMgr.Close(L"SFRM_GUILD_SETTING");
}

void lwGuild::OnClickMercenartSetOK(lwUIWnd kParent)
{
	XUI::CXUI_Wnd* pMainUI = kParent.GetSelf();
	if( !pMainUI )
	{
		return;
	}

	PgLimitClass kClassLimit;


	XUI::CXUI_Wnd* pkLevel = pMainUI->GetControl(SZ_MERCENARY_LIMIT_LV);
	if( pkLevel )
	{
		short iTargetLv = 0;
		pkLevel->GetCustomData(&iTargetLv, sizeof(iTargetLv));
		kClassLimit.SetLevel(iTargetLv);
	}	

	__int64	i64LimitClass = 0i64;

	XUI::CXUI_CheckButton* pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_F));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_FIGHTER;
	}
	pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_M));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_MAGICIAN;
	}
	pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_A));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_ARCHER;
	}
	pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_T));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_THIEF;
	}
	pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_S));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_SHAMAN;
	}
	pkChangeBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(SZ_MERCENARY_LIMIT_CLASS_D));
	if( pkChangeBtn && pkChangeBtn->Check() )
	{
		i64LimitClass |= UCLIMIT_MARKET_DOUBLE_FIGHTER;
	}

	kClassLimit.SetClass(i64LimitClass);

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_SetMercenary));

	XUI::CXUI_CheckButton* pUseOk = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(SZ_MERCENARY_USE_OK));
	XUI::CXUI_CheckButton* pUseNo = dynamic_cast<XUI::CXUI_CheckButton*>(pMainUI->GetControl(SZ_MERCENARY_USE_NO));
	if( pUseOk && pUseNo )
	{
		if( pUseOk->Check() )
		{
			kClassLimit.WriteToPacket(kPacket);
			if( kClassLimit.IsUsing() )
			{
				NETWORK_SEND(kPacket);
			}
			else
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 71563, true);
				return;
			}
		}

		if( pUseNo->Check() )
		{
			kClassLimit.SetClass( 0i64 );
			kClassLimit.WriteToPacket(kPacket);
			NETWORK_SEND(kPacket);
		}
	}

	XUIMgr.Close(L"SFRM_GUILD_SETTING");
}

void lwGuild::lwNoticeEmporiaBattleRemainTime(lwWString kNoti)
{
	g_kChatMgrClient.AddLogMessage( SChatLog(CT_EVENT), kNoti(), true, EL_Normal );
	SetEmporiaBattleAlram();
}

// short lwGuild::GetEmporiaFuncPrice( short nEmporiaFuncNo )
// {
// 	return g_kEmporiaMgr.GetFuncPrice( nEmporiaFuncNo );
// }

void lwGuild::lwChangeGuildInvTab(const BYTE byInvType)
{
	lwSendGuildInvOpen( byInvType, PgGuildInventory::kGuildInventoryNpc );
}

void lwGuild::lwSendGuildInvOpen(const BYTE byInvType, lwGUID kNpcGuid)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( BM::GUID::IsNull( pkPlayer->GuildGuid() )
	|| BM::GUID::IsNull( kNpcGuid() ) )
	{
		return;
	}
	
	PgGuildInventory::SetGuildInventoryNpc( kNpcGuid() );
	
	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_InventoryOpen));
	kPacket.Push(pkPlayer->GuildGuid());
	kPacket.Push(byInvType);
	NETWORK_SEND(kPacket);
}

void lwGuild::lwSendGuildInvMoney()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( BM::GUID::IsNull(pkPlayer->GuildGuid()) )
	{
		return;
	}
	
	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_InventoryMoney));
	kPacket.Push(pkPlayer->GuildGuid());
	NETWORK_SEND(kPacket);
}

void lwGuild::lwGuildLogPageMove(const int iMovePage)
{// 이전, 다음 페이지 또는 이전5페이지, 다음 5페이지
	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_GUILD_INVENTORY_LOG");
	if( pkMainUI )
	{
		int iCurPage = 0;
		pkMainUI->GetCustomData(&iCurPage, sizeof(iCurPage));

		 // 실제 페이지 보다 적거나 많은 페이지를 요구하면 서버에서 첫 또는 마지막 페이지를 준다.
		int iDestPage = iCurPage + iMovePage;
		SendRequestGuildInvLog( iDestPage );
	}
}

void lwGuild::SendRequestGuildInvLog(const int iPage)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( BM::GUID::IsNull( pkPlayer->GuildGuid() ) )
	{
		return;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_InventoryLog));
	kPacket.Push(pkPlayer->GuildGuid());
	kPacket.Push(iPage);
	NETWORK_SEND(kPacket)
}

void lwGuild::lwRequestGuildInvLog(lwUIWnd kSelf)
{	
	XUI::CXUI_Wnd* pkSelf = kSelf.GetSelf();
	if( !pkSelf )
	{
		return;
	}
	
	int iRequestPage = 1;
	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"SFRM_GUILD_INVENTORY_LOG");
	if( !pkMainUI )
	{
		pkMainUI = XUIMgr.Call(L"SFRM_GUILD_INVENTORY_LOG");
		if( !pkMainUI )
		{
			return ;
		}

		pkSelf->SetCustomData(&iRequestPage, sizeof(iRequestPage));
	}
	
	pkSelf->GetCustomData(&iRequestPage, sizeof(iRequestPage));
	SendRequestGuildInvLog(iRequestPage);
}

void lwGuild::lwSendGuildMoney(const int iType)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( BM::GUID::IsNull( pkPlayer->GuildGuid() ) )
	{
		return;
	}

	XUI::CXUI_Wnd* pkMoney = NULL;
	switch(iType)
	{
	case EGIT_MONEY_IN:// 입금
		{
			pkMoney = XUIMgr.Get(L"SFRM_GUILD_INVENTORY_GOLD_IN");			
		}break;
	case EGIT_MONEY_OUT:// 출금
		{
			pkMoney = XUIMgr.Get(L"SFRM_GUILD_INVENTORY_GOLD_OUT");			
		}break;
	default:
		{
			return ;
		}break;
	}

	if( !pkMoney )
	{
		return ;
	}
	
	XUI::CXUI_Edit* pkEditMoney = dynamic_cast<XUI::CXUI_Edit*>(pkMoney->GetControl(L"EDT_MONEY"));
	if( !pkEditMoney )
	{
		return ;
	}

	std::wstring wstrVal = pkEditMoney->Text();
	const __int64 i64Money = (_wtoi64(wstrVal.c_str()) * 10000);

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_InventoryUpdate));
	kPacket.Push(pkPlayer->GuildGuid());
	kPacket.Push(iType);
	kPacket.Push(i64Money);
	NETWORK_SEND(kPacket)
}

void lwGuild::lwSendGuildInvClose()
{
	PgGuildInventory::ClearGuildInventoryNpc();

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( BM::GUID::IsNull( pkPlayer->GuildGuid() ) )
	{
		return;
	}

	BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, static_cast<BYTE>(GC_InventoryClose));
	kPacket.Push(pkPlayer->GuildGuid());
	NETWORK_SEND(kPacket);
}

void lwGuild::CallGuildInv(const BYTE byInvType)
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_GUILD_INVENTORY");
	if( !pkWnd )
	{
		pkWnd = XUIMgr.Call(L"SFRM_GUILD_INVENTORY");
		if( !pkWnd )
		{
			return ;
		}
	}
	
	lwSetGuildInvViewSize(byInvType);
}


static int GetGuildLogDetailText(EGuildInvTradeType const eType)
{
    switch(eType)
    {
    case EGIT_EMPORIA_RESERVE:  return 401707;
    case EGIT_EMPORIA_THROW:    return 401708;
    case EGIT_EMPORIA_REWARD:   return 401709;
    }
    return 401111;
}

void lwGuild::CallGuildLog(BM::Stream* pkPacket)
{
	int iTotalPage = 0;
	int iCurPage = 0;
	int iCount = 0;

	pkPacket->Pop(iTotalPage);
	pkPacket->Pop(iCurPage);
	pkPacket->Pop(iCount);

	CONT_GUILD_INVENTORY_LOG kContLog;

	for( int i = 0; i < iCount; ++i )
	{
		SGuild_Inventory_Log kLog;
		kLog.ReadFromPacket(*pkPacket);
		kContLog.push_back(kLog);
	}

	XUI::CXUI_Wnd* pkLogWnd = XUIMgr.Get(L"SFRM_GUILD_INVENTORY_LOG");
	if( !pkLogWnd )
	{
		pkLogWnd = XUIMgr.Call(L"SFRM_GUILD_INVENTORY_LOG");
		if( !pkLogWnd )
		{
			return ;
		}
	}

	pkLogWnd->SetCustomData(&iCurPage, sizeof(iCurPage)); // 현재 페이지 저장

	XUI::CXUI_Wnd* pkPageWnd = pkLogWnd->GetControl(L"FRM_PAGE");
	if( !pkPageWnd )
	{
		return;
	}

	XUI::CXUI_Builder* pkBuildPageLine = dynamic_cast<XUI::CXUI_Builder*>(pkPageWnd->GetControl(L"BLD_LINE"));
	if( !pkBuildPageLine )
	{
		return ;
	}

	XUI::CXUI_Builder* pkBuildPageNum = dynamic_cast<XUI::CXUI_Builder*>(pkPageWnd->GetControl(L"BLD_NUM"));
	if( !pkBuildPageNum )
	{
		return ;
	}	

	for( int i = 0; i < 5; ++i )
	{
		BM::vstring	vStr(L"CBTN_NUM");
		vStr += i;
		XUI::CXUI_CheckButton* pkPageBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkPageWnd->GetControl(vStr));		
		if( pkPageBtn )
		{
			pkPageBtn->Visible(false);
		}

		int iCurPageNum = ( ( iCurPage - 1 ) / 5) * 5 + 1 + i;
		pkPageBtn->SetCustomData(&iCurPageNum, sizeof(iCurPageNum));

		if( iCurPageNum <= iTotalPage )
		{
			pkPageBtn->Visible(true);

			{// 페이지 번호
				BM::vstring vStrPageNum(iCurPageNum);
				pkPageBtn->Text(vStrPageNum);
			}

			// 눌려야 할 페이지의 인덱스
			int iTemp = ( ( iCurPage - 1 ) % 5 == 0) ? 0 : ( iCurPage -1 ) % 5;
			if( iTemp == i )
			{
				pkPageBtn->Check(true);
				pkPageBtn->ClickLock(true);
			}
			else
			{
				pkPageBtn->ClickLock(false);
				pkPageBtn->Check(false);
			}
		}
	}

	// 로그 내용 출력
	XUI::CXUI_Builder* pkBuildLogContent = dynamic_cast<XUI::CXUI_Builder*>(pkLogWnd->GetControl(L"BLD_LOG_CONTENT"));
	if( !pkBuildLogContent )
	{
		return ;
	}

	for( int i = 0; i < 10; ++i )
	{
		BM::vstring vStr(L"FRM_LOG_CONTENT");
		vStr += i;

		XUI::CXUI_Wnd* pkLogContentWnd = pkLogWnd->GetControl(vStr);
		if( pkLogContentWnd )
		{
			XUI::CXUI_StaticForm* pkDateFrm		= dynamic_cast<XUI::CXUI_StaticForm*>(pkLogContentWnd->GetControl(L"SFRM_DATE"));
			XUI::CXUI_StaticForm* pkCharNameFrm = dynamic_cast<XUI::CXUI_StaticForm*>(pkLogContentWnd->GetControl(L"SFRM_CHARACTER"));
			XUI::CXUI_StaticForm* pkTypeFrm		= dynamic_cast<XUI::CXUI_StaticForm*>(pkLogContentWnd->GetControl(L"SFRM_TYPE"));
			XUI::CXUI_StaticForm* pkDetailFrm	= dynamic_cast<XUI::CXUI_StaticForm*>(pkLogContentWnd->GetControl(L"SFRM_DETAILS"));
			XUI::CXUI_StaticForm* pkCountFrm	= dynamic_cast<XUI::CXUI_StaticForm*>(pkLogContentWnd->GetControl(L"SFRM_COUNT"));

			if( !pkDateFrm || !pkCharNameFrm || !pkTypeFrm || !pkDetailFrm || !pkCountFrm )
			{
				return ;
			}

			{// 이전 정보 초기화
				pkDateFrm->Text(L"");
				pkCharNameFrm->Text(L"");
				pkTypeFrm->Text(L"");
				pkDetailFrm->Text(L"");
				pkCountFrm->Text(L"");
				POINT2 kPos = pkCountFrm->TextPos();
				kPos.x = 56;
				pkCountFrm->TextPos(kPos);
				XUI::CXUI_Wnd* pkCountType = pkCountFrm->GetControl(L"FRM_COUNT_GOLD");
				if( pkCountType )
				{
					pkCountType->Visible(false);
				}
			}

			if( iCount <= i )
			{
				continue;
			}
			
			BM::vstring vStrYear(kContLog.at(i).m_kDateTime.year);
			BM::vstring vStrMonth(kContLog.at(i).m_kDateTime.month, L"%02u");
			BM::vstring vStrDay(kContLog.at(i).m_kDateTime.day, L"%02u");
			BM::vstring vStrHour(kContLog.at(i).m_kDateTime.hour, L"%02u");
			BM::vstring vStrMin(kContLog.at(i).m_kDateTime.minute, L"%02u");
			BM::vstring vStrSec(kContLog.at(i).m_kDateTime.second, L"%02u");

			BM::vstring vStrDateTime;
			vStrDateTime =	vStrYear + L"/"
							+ vStrMonth + L"/"
							+ vStrDay + L" "
							+ vStrHour + L":"
							+ vStrMin + L":"
							+ vStrSec;
			
			pkDateFrm->Text(vStrDateTime);
			pkCharNameFrm->Text(kContLog.at(i).m_kCharName);

			BM::vstring vStrType;
			EGuildInvTradeType eType = static_cast<EGuildInvTradeType>(kContLog.at(i).m_kType);
			switch( eType )
			{
			case EGIT_MONEY_IN:
			case EGIT_ITEM_IN:
            case EGIT_EMPORIA_THROW:
            case EGIT_EMPORIA_REWARD:
				{
					vStrType = TTW(401702);
				}break;
			case EGIT_MONEY_OUT:
			case EGIT_ITEM_OUT:
            case EGIT_EMPORIA_RESERVE:
				{
					vStrType = TTW(401703);
				}break;
			default:
				{
				}break;
			}
		
			pkTypeFrm->Text(vStrType); // 맡김 / 찾음
			
			switch( eType ) // 아이템 이름 / 골드
			{
			case EGIT_MONEY_IN:
			case EGIT_MONEY_OUT:
            case EGIT_EMPORIA_RESERVE:
            case EGIT_EMPORIA_THROW:
            case EGIT_EMPORIA_REWARD:
			case EGIT_GM_ORDER:
				{
                    int const iTTNo = GetGuildLogDetailText(eType);
					pkDetailFrm->Text(TTW(iTTNo));

					typedef std::vector< std::wstring > ContWstr;
					ContWstr kMoneyVec;
					if( BreakMoney_WStr( kContLog.at(i).m_kCount, kMoneyVec ) )
					{
						pkCountFrm->Text(kMoneyVec.at(0)); // 골드
						XUI::CXUI_Wnd* pkCountType = pkCountFrm->GetControl(L"FRM_COUNT_GOLD");
						if( pkCountType )
						{
							pkCountType->Visible(true);
						}
					}
				}break;
			case EGIT_ITEM_IN:
			case EGIT_ITEM_OUT:
				{
					const int iItemNo = kContLog.at(i).m_kItemNo;
					GET_DEF(CItemDefMgr, kItemDefMgr);
					const CItemDef* pItemDef = kItemDefMgr.GetDef(iItemNo);
					if( pItemDef )
					{
						const wchar_t *pName = NULL;
						if(GetDefString( pItemDef->NameNo(), pName ) )
						{
							// 아이템 이름이 너무 길면 말줄임표를 사용
							int const iWidth = pkDetailFrm->Width()+REGULATION_UI_WIDTH;
							Quest::SetCutedTextLimitLength(pkDetailFrm, pName,WSTR_UI_REGULATIONSTR, iWidth);					

							BM::vstring vStr(TTW(401706));
							vStr.Replace(L"#COUNT#",  kContLog.at(i).m_kCount);
							pkCountFrm->Text(vStr); // 수량
							
							POINT2 kPos = pkCountFrm->TextPos();
							kPos.x = 76;
							pkCountFrm->TextPos(kPos);
						}
					}
				}break;
			default:
				{
				}break;
			}
		}
	}
}
	
int lwGetEmporiaChallengeLimitCost()
{
	return g_kEmporiaMgr.GetEmporiaChallengeLimitCost();
}

bool lwGuild::lwIsMyGuildMemeber(lwGUID kGuid)
{
	SGuildMemberInfo kMemberInfo;
	return g_kGuildMgr.Member_Find_ByGuid(kGuid(), kMemberInfo);
}