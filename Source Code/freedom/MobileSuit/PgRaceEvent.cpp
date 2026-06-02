#include "stdafx.h"
#include "bm/guid.h"
#include "lwui.h"
#include "PgRaceEvent.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgQuestMan.h"
#include "ServerLib.h"
#include "PgActor.h"
#include "PgAction.h"
#include "PgWorld.h"
#include "PgChatMgrClient.h"
#include "lwui.h"

extern int g_iEventNo;
SEventRaceData g_kRaceEventInfo;
typedef std::map<int, RACEEVENT::RANK_MODIFIED_INFO> CONT_RANK_MODIFIED_INFO;
CONT_RANK_MODIFIED_INFO kContRankModifiedInfo;

bool RACEEVENT::ProcessMsg(unsigned short const usType, BM::Stream& Packet)
{
	switch( usType )
	{
	case PT_M_C_NFY_RACE_EVENT_GROUND_MOVE_COMPLETE:
											{ Recv_PT_M_C_NFY_RACE_EVENT_GROUND_MOVE_COMPLETE(Packet); } break;
	case PT_M_C_NFY_RACE_EVENT_READY:		{ Recv_PT_M_C_NFY_RACE_EVENT_READY(Packet); } break;
	case PT_M_C_NFY_RACE_EVENT_START:		{ Recv_PT_M_C_NFY_RACE_EVENT_START(Packet); } break;
	case PT_M_C_NFY_RACE_EVENT_END:			{ Recv_PT_M_C_NFY_RACE_EVENT_END(Packet); } break;
	case PT_M_C_NFY_RACE_RANK_INFO:			{ Recv_PT_M_C_NFY_RACE_RANK_INFO(Packet); } break;
	default:
		{
			return false;
		}break;
	}
	return true;
}



void RACEEVENT::Recv_PT_M_C_NFY_RACE_EVENT_GROUND_MOVE_COMPLETE(BM::Stream& Packet)
{
	XUIMgr.Close(_T("FRM_MINIMAP"));

	int iEventNo = 0;
	DWORD dwRemainTime = 0;
	Packet.Pop(iEventNo);
	Packet.Pop(dwRemainTime);
	if(NULL == XUIMgr.Get(_T("SFRM_BIG_MAP")))
	{
		lwOnCallCenterMiniMap(lwGUID(BM::GUID()), 0);
	}
	lua_tinker::call< void, int >("CallCommunityEventMiniCard", iEventNo);
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("FRM_COMMUNITY_EVENT_MINICARD"));
	if(NULL != pkWnd)
	{
		XUI::CXUI_Wnd* pkWndSub1 = pkWnd->GetControl(_T("FRM_BG"));
		if(NULL != pkWndSub1)
		{
			XUI::CXUI_Wnd* pkWndText1 = pkWndSub1->GetControl(_T("FRM_DIFF"));
			if(NULL != pkWndText1)
			{
				BM::vstring kText(_T("{C=0xFFf7ea96/T=FTcts_13/}"));
				kText += TTW(400970);
				kText += _T("{C=0xFFfffbc7/T=FTcts_13/}");
				kText += TTW(302303);
				pkWndText1->Text(kText);
			}
		}
	}

	g_iEventNo = iEventNo;
	if( dwRemainTime )
	{
		lua_tinker::call< void, DWORD >("RunningEvent_SetBeginTimeNfy", dwRemainTime);
		lua_tinker::call< void, DWORD, int, int >("RunningEvent_NotifyTime", dwRemainTime, 302108, 302109);
	}


	CONT_DEF_EVENT_RACE const * pEventRace = NULL;
	g_kTblDataMgr.GetContDef(pEventRace);
	if( NULL == pEventRace )
	{
		return;
	}
	CONT_DEF_EVENT_RACE::const_iterator itFind = pEventRace->find(iEventNo);
	if(pEventRace->end() == itFind)
	{
		return;
	}
	g_kRaceEventInfo = itFind->second;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(NULL != pkPlayer)
	{
		if(g_kRaceEventInfo.MoveSpeed > 0)
		{
			pkPlayer->SetAbil(AT_MOVESPEED, g_kRaceEventInfo.MoveSpeed);
			pkPlayer->SetAbil(AT_C_MOVESPEED, g_kRaceEventInfo.MoveSpeed);
		}
		if(g_kRaceEventInfo.MpMax > 0)
		{
			pkPlayer->SetAbil(AT_C_MAX_MP, g_kRaceEventInfo.MpMax);

			XUI::CXUI_Wnd* pkMpBar = XUIMgr.Get(_T("FormHero"));
			if(pkMpBar)
			{
				XUI::CXUI_Wnd* pkHpMpBg = pkMpBar->GetControl(_T("SFRM_HPMP_BG"));
				if(pkHpMpBg)
				{
					XUI::CXUI_Wnd* pkMpBg = pkHpMpBg->GetControl(_T("SFRM_MP_BG"));
					if(pkMpBg)
					{
						XUI::CXUI_Wnd* pkMp = pkMpBg->GetControl(_T("MP"));
						if(pkMp)
						{
							lwUIWnd kWnd(pkMp);
							kWnd.SetPilotBar(2);
							kWnd.SetPilotBarInfo(2);
						}
					}
				}
			}
		}
	}

	CONT_RANK_MODIFIED_INFO kContTemp;
	kContRankModifiedInfo.swap(kContTemp);

}


void RACEEVENT::Recv_PT_M_C_NFY_RACE_EVENT_READY(BM::Stream& Packet)
{
	
	int iEventNo = 0;
	Packet.Pop(iEventNo);
	XUIMgr.Call(_T("FRM_RACE_COUNTDOWN10"));


	g_iEventNo = iEventNo;
}

void RACEEVENT::Recv_PT_M_C_NFY_RACE_EVENT_START(BM::Stream& Packet)
{
	XUIMgr.Call(_T("FRM_RACE_START"));

	lua_tinker::call<void>("Set_Running_Event_Timer");
}

void RACEEVENT::Recv_PT_M_C_NFY_RACE_EVENT_END(BM::Stream& Packet)
{
	//////////////
	lua_tinker::call<void>("Stop_Running_Event_Timer");
	if(NULL != XUIMgr.Get(_T("SFRM_BIG_MAP")))
	{
		lwOnCallCenterMiniMap(lwGUID(BM::GUID()), 0);
	}

	const int MAX_ITEM_REWARD = 10;
	const int MAX_ICON_SLOT = 4;
	int iCurrentUserCount = 0;// 그라운드에 접속 된 인원 수
	int iRank = 0;
	int iRewardCount = 0;
	int iRewardItem[MAX_ITEM_REWARD] = {0, };
	bool bCompleteRace = false;
	DWORD dwRecord = 0;
	std::wstring wstrName;
	BM::vstring strIconWndName(L"");

	Packet.Pop(iCurrentUserCount);//참여인원

	for(int n = 0; n < iCurrentUserCount; ++n)
	{	
		Packet.Pop(iRank);//등수
		Packet.Pop(wstrName);//ID
		Packet.Pop(dwRecord);//기록
		Packet.Pop(bCompleteRace);

		Packet.Pop(iRewardCount); //아이템 지급 갯수
		for(int nItem = 0; nItem < iRewardCount; nItem++)
		{
			Packet.Pop(iRewardItem[nItem]);
		}




		XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("SFRM_RUNNING_RANKING_EVENT_RESULT"));
		if(NULL == pWnd)
		{
			pWnd = XUIMgr.Call(_T("SFRM_RUNNING_RANKING_EVENT_RESULT"));
			if(NULL == pWnd) { return; }
		}

		XUI::CXUI_Wnd * kTitleWnd = pWnd->GetControl(_T("FRM_TITLE")); //맵이름
		if(NULL != kTitleWnd)
		{
			const std::wstring pkMapName = GetMapName(g_pkWorld->MapNo());
			if(pkMapName.empty())
			{
				kTitleWnd->Text(TTW(302303));
			}
			else
			{
				kTitleWnd->Text(pkMapName);
			}
		}


		BM::GUID kMyID;
		XUI::CXUI_Wnd* kRecordWnd = NULL;
		std::wstring kStrElem(_T("FRM_PLAYER_RESULT"));
		PgPilot* pkPilot = g_kPilotMan.FindPilot(wstrName);

		for(int n = 0; n < 2; n++)
		{
			BM::vstring kStrRes(_T(""));
			if(n == 0)
			{
				if(iRank <= 3)
				{
					kStrRes += kStrElem;
					kStrRes += (iRank - 1);
					kRecordWnd = pWnd->GetControl(kStrRes);
				}
			}
			else
			{
				if(NULL != pkPilot && pkPilot == g_kPilotMan.GetPlayerPilot())
				{ //내 정보인가?
					kStrRes += kStrElem;
					kStrRes += 3;
					kRecordWnd = pWnd->GetControl(kStrRes);
				}
				else
				{
					continue;
				}
			}

			if(NULL != kRecordWnd)
			{
				XUI::CXUI_Wnd* kRankWnd = kRecordWnd->GetControl(_T("FORM_RANK")); //순위
				if(NULL != kRankWnd)
				{
					kStrRes = iRank;
					kRankWnd->Text(kStrRes);
				}

				XUI::CXUI_Wnd* kNameWnd = kRecordWnd->GetControl(_T("FORM_NAME")); //이름
				if(NULL != kNameWnd)
				{
					kNameWnd->Text(wstrName);
				}

				XUI::CXUI_Wnd* kRecTimeWnd = kRecordWnd->GetControl(_T("FORM_ARRIVE_TIME")); //시간 기록
				if(NULL != kRecTimeWnd)
				{
					int iTimeInSec = dwRecord / 1000;
					int iMin = iTimeInSec / 60;
					int iSec = iTimeInSec % 60;
					BM::vstring str(L"");
					if(iMin > 0)
					{
						str += iMin;
						str += TTW(176);
						str += _T(" ");
					}
					if(iSec > 0)
					{
						str += iSec;
						str += TTW(177);
					}
					kRecTimeWnd->Text(str);
				}

				//아이콘 업데이트
				for(int n = 0; n < MAX_ICON_SLOT; n++)
				{
					strIconWndName = L"FRM_ITEM_SLOT";
					strIconWndName += n;
					XUI::CXUI_Wnd *kIconSlot = kRecordWnd->GetControl(strIconWndName);
					if(NULL == kIconSlot) { continue; }
					if(n < iRewardCount)
					{
						kIconSlot->SetCustomData(&iRewardItem[n], sizeof(int));
					}
					else
					{
						kIconSlot->ClearCustomData();
					}
				} //for(int n = 0; n < iRewardCount; n++)

			}
		}

		if(NULL != pkPilot)
		{
			const BM::GUID kPlayerID = pkPilot->GetGuid();
			PgActor* pkActor = g_kPilotMan.FindActor(kPlayerID);
			if(NULL != pkActor)
			{
				if(iRewardCount > 0)
				{ //보상이 있다면 승리 액션
					pkActor->ReserveTransitAction("a_victory");
				}
				else
				{ //없으면 OTL 액션
					if(true == pkActor->IsMyActor())
					{
						SReqPlayerLock kLockInfo(EPLT_RaceEvent, false); //액션 연출을 위해 입력 막기
						g_kPilotMan.LockPlayerInput(kLockInfo);
					}
					pkActor->ReserveTransitAction("a_breakdown");
				}
			}

			if(true == bCompleteRace && (3 >=iRank && 1 <= iRank) && pkPilot != NULL) //1등이라면
			{ //폭죽 연출 해주자
				if(1==iRank)
				{
					PgStatusEffectManUtil::SReservedEffect kEffect;
					kEffect.iEffectKey = 98005540;
					kEffect.iEffectID = 442401;
					kEffect.kPilotGuid = pkPilot->GetGuid();
					PgStatusEffectManUtil::AddReservedEffect(g_kStatusEffectMan, pkPilot, kEffect);
				}
				lwActor kact(g_kPilotMan.FindActor(pkPilot->GetGuid()));
				static std::string eff_name[3] = 
				{
					"se_race_run_gold",
					"se_race_run_silver",
					"se_race_run_bronze"
				};

				if(false == kact.IsNil())
				{
					kact.AttachParticleS(99999999, "p_ef_star", eff_name[iRank-1].c_str(), 0.3f);
				}
			}
		}
		::memset(iRewardItem, 0, sizeof(int) * 10);

	} //for(int n = 0; n < m_iCurrentUserCount; ++n)

	bool bFinishGame = false;
	Packet.Pop(bFinishGame);
	if(true == bFinishGame)
	{
		SChatLog kChatLog(CT_EVENT_SYSTEM);
		g_kChatMgrClient.AddLogMessage(kChatLog, TTW(302311), true);
	}

}

void RACEEVENT::Recv_PT_M_C_NFY_RACE_RANK_INFO(BM::Stream& Packet)
{
	int iNumPlayer = 0;
	Packet.Pop(iNumPlayer);

	int iRank = 0;
	std::wstring wstrName;
	DWORD dwRecord = 0;

	lua_tinker::call< void >("RunningEvent_ClearRecord"); //전체 지움

	for(int n = 0; n < iNumPlayer; n++)
	{
		bool bGoalIn = false;
		WORD wRecentPos = 0;
		Packet.Pop(iRank);
		Packet.Pop(wstrName);
		Packet.Pop(bGoalIn); //결승점에 도달했는가?
		Packet.Pop(wRecentPos); //체크포인트(트리거) 위치 정보 65535를 넘지 않아야 한다.
		Packet.Pop(dwRecord);
		int iRecentPos = bGoalIn ? -1 : wRecentPos;

		CONT_RANK_MODIFIED_INFO::iterator itRankModifiedInfo = kContRankModifiedInfo.find(iRank);
		if(itRankModifiedInfo == kContRankModifiedInfo.end())
		{
			std::pair<CONT_RANK_MODIFIED_INFO::iterator, bool> kRet = kContRankModifiedInfo.insert(std::make_pair( iRank, RANK_MODIFIED_INFO() ));
			if(kRet.second == false) { continue; }
			itRankModifiedInfo = kRet.first;
		}
		RANK_MODIFIED_INFO kModifyInfo(wstrName, iRecentPos);
		bool bUpdate = !(itRankModifiedInfo->second == kModifyInfo);
		if(true == bUpdate)
		{
			itRankModifiedInfo->second = kModifyInfo;
		}

		if(iRank <= 3) //3위 까지는 무조건 표시
		{
			lua_tinker::call< void, int, bool >("RunningEvent_UR_SetMoveSlide", iRank, bUpdate);
			lua_tinker::call< void, int, int, lwWString, int, DWORD >("RunningEvent_UpdateRecord", iRank, 0, lwWString(wstrName.c_str()), iRecentPos, dwRecord, bUpdate);
		}
		PgPilot* pkPilot = g_kPilotMan.FindPilot(wstrName);
		if(NULL != pkPilot && pkPilot == g_kPilotMan.GetPlayerPilot())
		{
			lua_tinker::call< void, int, bool >("RunningEvent_UR_SetMoveSlide", 4, bUpdate);
			lua_tinker::call< void, int, int, lwWString, int, DWORD >("RunningEvent_UpdateRecord", 4, iRank, lwWString(wstrName.c_str()), iRecentPos, dwRecord, bUpdate);
		}
	}
	lua_tinker::call< void >("RunningEvent_UR_MoveSlide");

	bool bFirstGoalIn = false;
	Packet.Pop(bFirstGoalIn);
	if(true == bFirstGoalIn)
	{
		DWORD dwRemainTime = 0;
		Packet.Pop(dwRemainTime);
		lua_tinker::call< void, DWORD >("RunningEvent_SetFinishTimeNfy", dwRemainTime);
		lua_tinker::call< void, DWORD, int, int >("RunningEvent_NotifyTime", dwRemainTime, 0, 302308);
	}
}


bool RACEEVENT::CheckSkillEvent(PgActor* pkActor, PgAction *pkNextAction)
{
	if (NULL==g_pkWorld)
	{
		return false;
	}
	if(g_pkWorld->GetAttr() != GATTR_RACE_GROUND)
	{ //레이스이벤트가 아니라면 제한 없음
		return true;
	}
	if(true == g_kRaceEventInfo.UseSkill) { return true; } //스킬 허용 상태

	//스킬 사용 금지일 때
	if(NULL == pkActor || NULL == pkNextAction) { return true; } //아직 로딩 전이면 idle 액션 초기화 등등 허용해주자
	if(g_iEventNo == 0) { return true; }

	if(EST_GENERAL != pkNextAction->GetSkillType() && EST_NONE != pkNextAction->GetSkillType())
	{ //이동 스킬, 아이들이 아닌 액션이면 실패
		lwAddWarnDataTT(302307);
		return false;
	}

	return true;
}

