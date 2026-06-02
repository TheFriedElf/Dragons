#include "stdafx.h"
#include "ServerLib.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "Variant/PgPlayer.h"
#include "PgNetwork.h"
#include "PgWorld.h"
#include "PgMobileSuit.h"
#include "PgCommunityEvent.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
#include "variant/PgSimpleTime.h"
#include "bm/guid.h"
#include "PgEventScriptSystem.h"
#include "PgChatMgrClient.h"
#include "variant/PgQuestInfo.h"
#include "PgQuestMan.h"

int g_iEventNo = 0;
VEC_INT OpenCommunityEventNo;

namespace lwCommunityEvent
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "SendReqCommunityEventEnter", lwSendReqCommunityEventEnter);
		def(pkState, "SetEventList", lwSetEventList);
		def(pkState, "SendReqEventProgressInfo", lwSendReqEventProgressInfo);
	}

	void SortEventList(CONT_CHECK_SCHEDULE & outCont)
	{
		CONT_DEF_EVENT_SCHEDULE const* pEventSchedule = NULL;
		g_kTblDataMgr.GetContDef(pEventSchedule);
		if( NULL == pEventSchedule )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pkPlayer )
		{
			return;
		}
		
		outCont.clear();

		CONT_SCHEDULE_DATA ScheduleType1, ScheduleType2, ScheduleType3;
		CONT_DEF_EVENT_SCHEDULE::const_iterator iter;
		for( iter = pEventSchedule->begin(); iter != pEventSchedule->end(); ++iter )
		{
			DWORD Result = COMMUNITY_EVENT::CheckCanEnter(pkPlayer, iter->second);
			if( Result & COMMUNITY_EVENT::EER_ERROR_EVENT_END_DAY )
			{
				continue;
			}

			bool bHit = false;
			VEC_INT::const_iterator open_iter;
			for( open_iter = OpenCommunityEventNo.begin(); open_iter != OpenCommunityEventNo.end(); ++open_iter )
			{
				if( iter->second.EventNo == *open_iter )
				{
					ScheduleType1.push_back(iter->second);
					bHit = true;
					break;
				}
			}

			if( false == bHit )
			{
				if( Result == COMMUNITY_EVENT::EER_OK )
				{// 입장조건에는 맞지만 시간이 안맞는 항목을 복사한다.
					ScheduleType2.push_back(iter->second);
				}
				else
				{// 입장 조건이 안맞는 항목을 복사한다.
					ScheduleType3.push_back(iter->second);
				}
			}
		}
		if( ScheduleType1.size() )
		{
			outCont.insert( std::make_pair( ES_ALL_CLEAR, ScheduleType1 ) );
		}
		if( ScheduleType2.size() )
		{
			outCont.insert( std::make_pair( ES_NOT_ENTER_TIME, ScheduleType2 ) );
		}
		if( ScheduleType3.size() )
		{
			outCont.insert( std::make_pair( ES_CHECK_CONDITION, ScheduleType3 ) );
		}
	}

	void lwSetEventList(lwUIWnd lwWnd, char const* TypeName, char const* ModeName)
	{
		XUI::CXUI_Wnd * pWnd = lwWnd();
		if( !pWnd )
		{
			return;
		}
		
		if( !TypeName )
		{
			return;
		}
		BM::vstring const vTypeName(TypeName);

		XUI::CXUI_List * pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(_T("LST_SEARCH_EVENT_LIST")));
		if( !pList )
		{
			return;
		}

		pList->ClearList();

		// 테이블에서 커뮤니티 이벤트 정보를 가져와 이벤트 갯수만큼 리스트에 등록한다.
		CONT_CHECK_SCHEDULE EventSchedule;
		SortEventList(EventSchedule);

		CONT_CHECK_SCHEDULE::const_iterator iter;
		for( iter = EventSchedule.begin(); iter != EventSchedule.end(); ++iter )
		{
			if( iter->second.size() )
			{
				CONT_SCHEDULE_DATA::const_iterator data_iter;
				for( data_iter = iter->second.begin(); data_iter != iter->second.end(); ++data_iter )
				{
					bool bAddItem = false;
					if( vTypeName == "COMMUNITYEVENT" )
					{
						//if( data_iter->Menu_Section == 1 )
						{
							if( !ModeName )
							{
								bAddItem = true;
							}
							else
							{
								BM::vstring vModeName(ModeName);
								if( (std::wstring)vModeName == data_iter->EventType )
								{
									bAddItem = true;
								}
							}
						}
					}
					else if( vTypeName == "BATTLESQUARE" )
					{
						//if( data_iter->Menu_Section == 2 )
						{
							if( !ModeName )
							{
								bAddItem = true;
							}
							else
							{
								BM::vstring vModeName(ModeName);
								if( (std::wstring)vModeName == data_iter->EventType )
								{
									bAddItem = true;
								}
							}
						}
					}

					if( bAddItem )
					{
						XUI::SListItem* pItem = pList->AddItem(_T("")); // 리스트에 등록시킨다.
						if( pItem )
						{
							XUI::CXUI_Wnd* pItemWnd = pItem->m_pWnd;
							if( pItemWnd )
							{
								pItemWnd->SetCustomData(&data_iter->EventNo, sizeof(data_iter->EventNo)); // 이벤트 번호를 등록
								
								XUI::CXUI_Image* pOX = dynamic_cast<XUI::CXUI_Image*>(pItemWnd->GetControl(_T("IMG_OX")));
								XUI::CXUI_Wnd* pTitle = pItemWnd->GetControl(_T("SFRM_COLOR_BOX2"));
								XUI::CXUI_Wnd* pCondition = pItemWnd->GetControl(_T("FRM_CONDITION_CHECK_TEXT"));
								if( pOX &&  pTitle && pCondition )
								{
									int TitleMaxLength = pTitle->Size().x - 20;
									pTitle->FontColor(0xFF4D3413);
									pCondition->Visible(false);

									SUVInfo UVInfo = pOX->UVInfo();
									
									if( (iter->first == ES_ALL_CLEAR) || (iter->first == ES_NOT_ENTER_TIME ) )
									{
										bool bOpen = false;
										if( false == OpenCommunityEventNo.empty() )
										{
											VEC_INT::const_iterator vec_iter;
											for( vec_iter = OpenCommunityEventNo.begin(); vec_iter != OpenCommunityEventNo.end(); ++vec_iter )
											{
												if( data_iter->EventNo == *vec_iter )
												{
													bOpen = true;
													break;
												}
											}
										}
										if( bOpen )
										{
											UVInfo.Index = 1;
											pOX->GrayScale(false);
										}
										else
										{
											UVInfo.Index = 2;
											pOX->GrayScale(false);
										}
									}
									else
									{
										UVInfo.Index = 2;
										pOX->GrayScale(true);
										pTitle->FontColor(0xFF7F7F7F);
										pCondition->Visible(true);
										TitleMaxLength = pTitle->Size().x - pCondition->Size().x - 10; // [조건 확인] 이 보여지므로 제목 길이 제한이 짧아진다.
									}

									pOX->UVInfo(UVInfo);
									pOX->SetInvalidate();

									int TextWidth = 0;
									std::wstring titleText(data_iter->MenuText);
									XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(pTitle->Font());
									if( pFont )
									{
										TextWidth = pFont->CalcWidth(titleText);
									}

									if( TextWidth && TitleMaxLength < TextWidth )
									{
										while( TitleMaxLength < TextWidth )
										{
											int endpos = titleText.size() - 2;
											if( endpos > 0 )
											{
												titleText = titleText.substr(0, endpos);
												TextWidth = pFont->CalcWidth(titleText);
											}
											else
											{
												break;
											}
										}
										titleText += _T("...");
									}
									pTitle->Text(titleText);
								}
							}
						}
					}
					else
					{
						continue;
					}
				}
			}
		}
	}

	void lwSendReqCommunityEventEnter(int const EventNo)
	{
		CONT_DEF_EVENT_SCHEDULE const* pEventSchedule = NULL;
		g_kTblDataMgr.GetContDef(pEventSchedule);
		if( NULL == pEventSchedule )
		{
			return;
		}
		CONT_DEF_EVENT_SCHEDULE::const_iterator iter = pEventSchedule->find(EventNo);
		if( iter == pEventSchedule->end() )
		{
			return;
		}

		if( PgClientPartyUtil::IsInParty() )
		{// 파티에 속해 있으면 파티장만 신청할 수 있다.
			PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
			if( pPlayer )
			{
				if( PgClientPartyUtil::IsPartyMaster( pPlayer->GetID() ) )
				{
					DWORD Result = COMMUNITY_EVENT::CheckCanEnter(pPlayer, iter->second);
					if( Result == COMMUNITY_EVENT::EER_OK )
					{
						BM::Stream Packet(PT_C_M_REQ_MOVE_EVENT_GROUND);
						Packet.Push(EventNo);
						NETWORK_SEND(Packet);
					}
					else
					{ //파티장이 입장 제한에 걸리면
						CommunityEventsCheckResult(Result);
					}
				}
				else
				{// 내가 파티장이 아니다.
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 302101, true);
				}
			}
		}
		else
		{// 파티가 아닐경우는 상관 없다.
			PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
			DWORD Result = COMMUNITY_EVENT::CheckCanEnter(pPlayer, iter->second);
			if( Result == COMMUNITY_EVENT::EER_OK )
			{
				BM::Stream Packet(PT_C_M_REQ_MOVE_EVENT_GROUND);
				Packet.Push(EventNo);
				NETWORK_SEND(Packet);
			}
			else
			{// 에러 처리
				CommunityEventsCheckResult(Result);
			}
		}
	}

	void lwSendReqEventProgressInfo()
	{
		BM::Stream Packet(PT_C_M_REQ_EVENT_PROGRESS_INFO);
		NETWORK_SEND(Packet);
	}

	void CommunityEventsCheckResult(DWORD const& Result)
	{
		if( (Result & COMMUNITY_EVENT::EER_ERROR_LV_MIN)
			|| (Result & COMMUNITY_EVENT::EER_ERROR_LV_MAX)
			|| (Result & COMMUNITY_EVENT::EER_ERROR_PARTY_MIN)
			|| (Result & COMMUNITY_EVENT::EER_ERROR_HUMAN_CLASS)
			|| (Result & COMMUNITY_EVENT::EER_ERROR_DRAGON_CLASS)
			|| (Result & COMMUNITY_EVENT::EER_ERROR_QUEST)
			|| (Result & COMMUNITY_EVENT::EER_ERROR_ITEM) )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 302102, true);
		}
		else if( (Result & COMMUNITY_EVENT::EER_ERROR_EVENT_BEGIN_DAY)
			|| (Result & COMMUNITY_EVENT::EER_ERROR_EVENT_END_DAY)
			|| (Result & COMMUNITY_EVENT::EER_ERROR_DAY) )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 302100, true);
		}
	}

	bool ProcessMsg(unsigned short const usType, BM::Stream& Packet)
	{
		switch( usType )
		{
		case PT_M_C_ANS_MOVE_EVENT_GROUND:				{ Recv_PT_M_C_ANS_MOVE_EVENT_GROUND(Packet); }break;
		case PT_M_C_NFY_COMMUNITY_EVENT_READY:			{ Recv_PT_M_C_NFY_COMMUNITY_EVENT_READY(Packet); }break;
		case PT_M_C_NFY_COMMUNITY_EVENT_START:			{ Recv_PT_M_C_NFY_COMMUNITY_EVENT_START(Packet); }break;
		case PT_M_C_NFY_COMMUNITY_EVENT_END:			{ Recv_PT_M_C_NFY_COMMUNITY_EVENT_END(Packet); }break;
		case PT_M_C_NFY_EVENT_GROUND_MOVE_COMPLETE:		{ Recv_PT_M_C_NFY_EVENT_GROUND_MOVE_COMPLETE(Packet); }break;
		case PT_M_C_NFY_EVENT_GROUND_USER_COUNT_MODIFY: { Recv_PT_M_C_NFY_EVENT_GROUND_USER_COUNT_MODIFY(Packet); }break;
		case PT_M_C_NFY_REMAIN_TIME_MONSTER_GENERATE:	{ Recv_PT_M_C_NFY_REMAIN_TIME_MONSTER_GENERATE(Packet); }break;
		case PT_M_C_ANS_EVENT_PROGRESS_INFO:			{ Recv_PT_M_C_ANS_EVENT_PROGRESS_INFO(Packet); }break;
		default:
			{
				return false;
			}break;
		}
		return true;
	}

	void Recv_PT_M_C_ANS_MOVE_EVENT_GROUND(BM::Stream& Packet)
	{
		eErrorEventMapMove Result = EEMM_LIMIT_NONE;
		Packet.Pop(Result);

		switch( Result )
		{
		case EEMM_LIMIT_NONE:
			{
			}break;
		case EEMM_LIMIT_MINIMUM_PARTYMEMBER:// 파티 최소 인원수 미달.
		case EEMM_LIMIT_LEVEL_MIN:// 최소 레벨이 맞지 않음.
		case EEMM_LIMIT_LEVEL_MAX:// 최대 레벨이 맞지 않음.
		case EEMM_LIMIT_CLASS:// 직업이 맞지 않음.
		case EEMM_LIMIT_QUEST:// 요구 퀘스트가 없음.
		case EEMM_LIMIT_ITEM:// 요구 아이템이 없음.
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 302116, true);
			}break;
		case EEMM_LIMIT_MAP_FULL:
			{// 해당 이벤트 맵 인원초과.
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 302103, true);
			}break;
		case EEMM_LIMIT_EVENT_TIME:
			{// 이벤트 진행 시간이 아님.
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 302100, true);
			}break;
		case EEMM_NO_PARTY_MEMBER_SAME_GROUND:
			{// 파티 멤버가 같은 맵에 있지 않다.
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 400906, true);
			}break;
		case EEMM_SUCCESS:
			{// 성공.
			}break;
		}
	}

	void Recv_PT_M_C_NFY_COMMUNITY_EVENT_READY(BM::Stream& Packet)
	{
		int EventNo = 0;
		Packet.Pop(EventNo);
		g_iEventNo = EventNo;

		XUIMgr.Call(_T("FRM_COMMUNITY_EVENT_DELAY"));
	}

	void Recv_PT_M_C_NFY_COMMUNITY_EVENT_START(BM::Stream& Packet)
	{
		int EventNo = 0;
		Packet.Pop(EventNo);
		g_iEventNo = EventNo;

		CONT_DEF_EVENT_SCHEDULE const* pEventSchedule = NULL;
		g_kTblDataMgr.GetContDef(pEventSchedule);
		if( NULL == pEventSchedule )
		{
			return;
		}
		
		CONT_DEF_EVENT_SCHEDULE::const_iterator iter = pEventSchedule->find(EventNo);
		if( iter == pEventSchedule->end() )
		{
			return;
		}

		if((*iter).second.PKOption)
		{ //PVP 가능하면
			COMMUNITY_EVENT::AttachTeamEffect(); //팀 표시 파티클 붙임
		}

		int LimitTime = 0;
		std::wstring sConvEventType(iter->second.EventType);
		BM::vstring::ConvToUPR(sConvEventType);
		if( sConvEventType == _T("BOSSBATTLE") )
		{
			CONT_DEF_EVENT_BOSSBATTLE const* pBossBattle = NULL;
			g_kTblDataMgr.GetContDef(pBossBattle);
			if( NULL == pBossBattle )
			{
				return;
			}
			CONT_DEF_EVENT_BOSSBATTLE::const_iterator iter = pBossBattle->find(EventNo);
			if( iter == pBossBattle->end() )
			{
				return;
			}

			LimitTime = iter->second.LimitTime;
		}
		//else if( iter->second.EventType == _T("다른모드") )
		//{
		//}

		if( LimitTime )
		{
			LimitTime = LimitTime * 1000; // LimitTime: Sec => MilliSec으로 변경
			lua_tinker::call< void, int >("Set_Defence_Timer", LimitTime);
		}
	}
	
	void Recv_PT_M_C_NFY_COMMUNITY_EVENT_END(BM::Stream& Packet)
	{
		int EventNo = 0;
		bool Result = false;
		Packet.Pop(EventNo);
		Packet.Pop(Result);

		//디태치 해주지 않아도 맵 이동시 클래스 소멸되므로
		//COMMUNITY_EVENT::DetachTeamEffect();

		if( Result )
		{
			XUIMgr.Call(_T("FRM_COMMUNITY_EVENT_SUCCESS"));
		}
		else
		{
			//XUIMgr.Call(_T("FRM_COMMUNITY_EVENT_FAILED")); //클라에서 자체적으로 연출 하므로 현재 사용하지 않음.
		}
	}
	
	void Recv_PT_M_C_NFY_EVENT_GROUND_MOVE_COMPLETE(BM::Stream& Packet)
	{
		DWORD RemainWaitTime = 0;

		Packet.Pop(g_iEventNo);
		Packet.Pop(RemainWaitTime);

		lua_tinker::call< void, int >("CallCommunityEventMiniCard", g_iEventNo);

		if( RemainWaitTime )
		{
			lua_tinker::call< void, int >("Set_CommunityEvent_Wait_Timer", RemainWaitTime);
		}
	}
	
	void Recv_PT_M_C_NFY_EVENT_GROUND_USER_COUNT_MODIFY(BM::Stream& Packet)
	{
		int UserCount = 0;
		Packet.Pop(UserCount);

		XUI::CXUI_Wnd * pWnd = XUIMgr.Activate(_T("FRM_COMMUNITY_EVENT_MINICARD"));
		if( !pWnd )
		{
			return;
		}

		XUI::CXUI_Wnd * BgWnd = pWnd->GetControl(_T("FRM_BG"));
		if( !BgWnd )
		{
			return;
		}
		
		XUI::CXUI_Wnd * MemberWnd = BgWnd->GetControl(_T("FRM_MEMBER"));
		if( !MemberWnd )
		{
			return;
		}

		std::wstring wStr;
		wStr = TTW(400219);
		wStr += (std::wstring const&)(BM::vstring)UserCount;
		wStr += TTW(400221);
		
		MemberWnd->Text(wStr);
		MemberWnd->Visible(true);
	}

	void Recv_PT_M_C_NFY_REMAIN_TIME_MONSTER_GENERATE(BM::Stream& Packet)
	{
		DWORD RemainSec = 0;
		int MonsterNo = 0;
		Packet.Pop(RemainSec);
		Packet.Pop(MonsterNo);

		std::wstring wText;

		if( RemainSec <= 0 )
		{//0보다 작은 값이 오면 안됨.
			return;
		}

		if( RemainSec > 10 )
		{
			int Minute = RemainSec / 60;
			int Sec = RemainSec % 60;

			if( Minute )
			{
				wText = (std::wstring const&)(BM::vstring)Minute;
				wText += TTW(8);
				wText += _T(" ");
			}
			if( Sec )
			{
				wText += (std::wstring const&)(BM::vstring)Sec;
				wText += TTW(9);
				wText += _T(" ");
			}

			GET_DEF(CMonsterDefMgr, MonsterDefMgr);
			CMonsterDef const * pDef = MonsterDefMgr.GetDef(MonsterNo);
			if(pDef)
			{
				wchar_t const * pMonName = NULL;
				if(g_pkWorld
					&& true == GetDefString(pDef->NameNo(), pMonName)
					)
				{
					BM::vstring vStr(TTW(302107));
					vStr.Replace(_T("#MONSTER#"), pMonName);
					wText += (std::wstring)vStr;

					::Notice_Show(wText, EL_Normal);
				}
			}
		}
		else
		{
			XUIMgr.Call(_T("FRM_DEFENCE_TIMELIMIT"));
		}
	}

	void Recv_PT_M_C_ANS_EVENT_PROGRESS_INFO(BM::Stream& Packet)
	{
		OpenCommunityEventNo.clear();
		PU::TLoadArray_A(Packet, OpenCommunityEventNo);

		lua_tinker::call<void>("CallSetEventList");
	}
}

void COMMUNITY_EVENT::AttachTeamEffect(void)
{
	PgPilot *pkPlayerPilot = g_kPilotMan.GetPlayerPilot();
	if(NULL == pkPlayerPilot)
	{
		return;
	}
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPlayerPilot->GetUnit());
	if(NULL == pkPlayer)
	{
		return;
	}

	if(pkPlayer->GetExpeditionGuid().IsNotNull())
	{// 내가 원정대에 소속 되어있다면
		ContExpeditionMember kMemberList = g_kExpedition.GetMembers();
		ContExpeditionMember::const_iterator iterMember = kMemberList.begin();
		while(iterMember != kMemberList.end())
		{// 원정대원들에게
			ContExpeditionMember::value_type const& pkMember = (*iterMember);
			++iterMember;

			if(NULL == pkMember)
			{
				continue;
			}

			PgActor* pkActor = g_kPilotMan.FindActor(pkMember->kCharGuid);
			if(NULL == pkActor )
			{
				continue;
			}

			NiAVObject *pkParticle = g_kParticleMan.GetParticle("e_ef_colorshadow_pvp_red", PgParticle::O_SCALE,pkActor->GetEffectScale() );
			if(NULL == pkParticle)
			{
				continue;
			}
			pkActor->AttachTo(4444, "char_root", pkParticle);	// 팀 파티클을 붙여주고
		}
	}
	else if(pkPlayer->GetPartyGuid().IsNotNull())
	{// 파티에 소속 되어있다면
		ContPartyMember kMemberList;
		g_kParty.GetPartyMemberList(kMemberList);
		ContPartyMember::const_iterator iterMember = kMemberList.begin();
		while( iterMember != kMemberList.end() )
		{// 파티원들에게
			ContPartyMember::value_type const& pkPartyInfo = (*iterMember);
			++iterMember;

			if( NULL == pkPartyInfo )
			{
				continue;
			}

			PgActor* pkActor = g_kPilotMan.FindActor(pkPartyInfo->kCharGuid);
			if( NULL == pkActor )
			{
				continue;
			}
			NiAVObject *pkParticle = g_kParticleMan.GetParticle("e_ef_colorshadow_pvp_red", PgParticle::O_SCALE,pkActor->GetEffectScale() );
			if( NULL == pkParticle)
			{
				continue;
			}
			pkActor->AttachTo(4444, "char_root", pkParticle);	//팀 파티클을 붙여주고
		}
	}
	else
	{// 어디에 속해있지 않은 개인이라면
		PgActor* pkActor = g_kPilotMan.GetPlayerActor();
		if(pkActor)
		{// 자신에게만
			NiAVObject *pkParticle = g_kParticleMan.GetParticle("e_ef_colorshadow_pvp_red", PgParticle::O_SCALE,pkActor->GetEffectScale() );
			if(NULL != pkParticle)
			{// 파티클을 붙여준다
				pkActor->AttachTo(4444, "char_root", pkParticle);
			}

		}
	}

}

void COMMUNITY_EVENT::DetachTeamEffect(void)
{
	PgPilot *pkPlayerPilot = g_kPilotMan.GetPlayerPilot();
	if( NULL == pkPlayerPilot)
	{
		return;
	}
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPlayerPilot->GetUnit());
	if( NULL == pkPlayer)
	{
		return;
	}

	if(pkPlayer->GetExpeditionGuid().IsNotNull()) //내가 원정대에 소속이 돼있나?
	{
		ContExpeditionMember kMemberList = g_kExpedition.GetMembers();
		ContExpeditionMember::const_iterator iterMember = kMemberList.begin();
		while(iterMember != kMemberList.end())
		{
			ContExpeditionMember::value_type const& pkMember = (*iterMember);
			++iterMember;

			if(NULL == pkMember)
			{
				continue;
			}

			PgActor* pkActor = g_kPilotMan.FindActor(pkMember->kCharGuid);
			if( NULL == pkActor )
			{
				continue;
			}
			pkActor->DetachFrom(4444);
		}
	}
	else if(pkPlayer->GetPartyGuid().IsNotNull()) //아니면 내가 파티에 소속이 돼있나?
	{
		ContPartyMember kMemberList;
		g_kParty.GetPartyMemberList(kMemberList);
		ContPartyMember::const_iterator iterMember = kMemberList.begin();
		while( iterMember != kMemberList.end() )
		{
			ContPartyMember::value_type const& pkPartyInfo = (*iterMember);
			++iterMember;

			if( NULL == pkPartyInfo )
			{
				continue;
			}

			PgActor* pkActor = g_kPilotMan.FindActor(pkPartyInfo->kCharGuid);
			if( NULL == pkActor )
			{
				continue;
			}
			pkActor->DetachFrom(4444);
		}
	}
	else //개인인가?
	{
		PgActor* pkActor = g_kPilotMan.GetPlayerActor();
		if(NULL != pkActor)
		{
			pkActor->DetachFrom(4444);
		}
	}
}

DWORD COMMUNITY_EVENT::CheckCanEnter(PgPlayer const* pkPlayer, SEventScheduleData const& rkEventSchedule)
{
	//PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return EER_ERROR_SYSTEM;
	}

	DWORD dwRet = EER_OK;
	if(rkEventSchedule.LevelMin != 0 && pkPlayer->GetAbil(AT_LEVEL) < rkEventSchedule.LevelMin)
	{ //최소 레벨 제한 체크
		dwRet |= EER_ERROR_LV_MIN;
	}
	if(rkEventSchedule.LevelMax != 0 && pkPlayer->GetAbil(AT_LEVEL) > rkEventSchedule.LevelMax)
	{ //최소 레벨 제한 체크
		dwRet |= EER_ERROR_LV_MAX;
	}

	if(rkEventSchedule.PartyMemeberMin != 0)
	{ //파티 최소 인원수가 걸려있다면
		SExpeditionInfo kExpeditionInfo;
		SClientPartyName kPartyInfo;
		if(pkPlayer->GetExpeditionGuid().IsNotNull() && g_kExpedition.GetExpedition(pkPlayer->GetExpeditionGuid(), kExpeditionInfo))
		{ //원정대에 속해 있다면 원정대 인원수 체크
			if(kExpeditionInfo.cCurMember < rkEventSchedule.PartyMemeberMin)
			{
				dwRet |= EER_ERROR_PARTY_MIN;
			}
		}
		else if(pkPlayer->PartyGuid().IsNotNull() && g_kParty.GetPartyName(pkPlayer->PartyGuid(), kPartyInfo))
		{ //파티에 속해 있다면 파티 인원수 체크
			if(kPartyInfo.cCurMember < rkEventSchedule.PartyMemeberMin)
			{
				dwRet |= EER_ERROR_PARTY_MIN;
			}
		}
		else if(1 < rkEventSchedule.PartyMemeberMin)
		{ //원정대, 파티에 속해있지 않을 경우 1이 아니면 실패
			dwRet |= EER_ERROR_PARTY_MIN;
		}
	}

	__int64 iUnionClassLimit = (rkEventSchedule.ClassLimitDragon << DRAGONIAN_LSHIFT_VAL) | rkEventSchedule.ClassLimitHuman;
	if(pkPlayer->UnitRace() == ERACE_HUMAN)
	{ //인간족 클래스 제한 체크
		if( rkEventSchedule.ClassLimitHuman == 0 ||
			!IS_CLASS_LIMIT(iUnionClassLimit, pkPlayer->GetAbil(AT_CLASS)) )
		{ //클래스 제한 값이 0이면 불가. 허용되는 클래스만 입장 가능
			dwRet |= EER_ERROR_HUMAN_CLASS;
		}
	}
	else if(pkPlayer->UnitRace() == ERACE_DRAGON)
	{ //용족 클래스 제한 체크
		if( rkEventSchedule.ClassLimitDragon == 0 ||
			!IS_CLASS_LIMIT(iUnionClassLimit, pkPlayer->GetAbil(AT_CLASS)) )
		{
			dwRet |= EER_ERROR_DRAGON_CLASS;
		}
	}

	SYSTEMTIME kSysTime;
	g_kEventView.GetLocalTime(&kSysTime);
	BM::DBTIMESTAMP_EX kStartDay(rkEventSchedule.StartDate);
	BM::DBTIMESTAMP_EX kEndDay(rkEventSchedule.EndDate);
	BM::DBTIMESTAMP_EX kCurTime(kSysTime);
	if(!kStartDay.IsNull())
	{ //이벤트 시작 날짜 제한이 걸려있다면..
		kStartDay.hour = 0;
		kStartDay.minute = 0;
		kStartDay.second = 0;
		if(kStartDay > kCurTime)
		{
			dwRet |= EER_ERROR_EVENT_BEGIN_DAY;
		}
	}
	if(!kEndDay.IsNull())
	{ //이벤트 종료 날짜 제한이 걸려있다면..
		kEndDay.hour = 23;
		kEndDay.minute = 59;
		kEndDay.second = 59;
		if(kEndDay < kCurTime)
		{
			dwRet |= EER_ERROR_EVENT_END_DAY;
		}
	}

	//요일 체크
	SDAYOFWEEK kDayOfWeek(EDOW_NONE);
	kDayOfWeek.SetDayofWeek(rkEventSchedule.DayofWeek);

	if( false == kDayOfWeek.IsInDay(static_cast<EDAYOFWEEK>(eDayOfWeekIdx[kSysTime.wDayOfWeek])) )
	{
		dwRet |= EER_ERROR_DAY;
	}

	//필요 퀘스트 상태 조건 체크
	typedef enum tagENEEDQUESTSTATE
	{
		ENQS_NONE = 0,
		ENQS_ACCEPT,
		ENQS_COMPLETE,
	}ENEEDQUESTSTATE;

	if(0 < rkEventSchedule.NeedQuest && 0 < rkEventSchedule.NeedQuestState )
	{
		PgQuestInfo const* pQuestInfo = g_kQuestMan.GetQuest( rkEventSchedule.NeedQuest );
		if( pQuestInfo )
		{
			PgMyQuest const* pMyQuest = pkPlayer->GetMyQuest();
			if( pMyQuest )
			{
				switch( rkEventSchedule.NeedQuestState )
				{
				case ENQS_ACCEPT:
					{
						if( !pMyQuest->IsIngQuest( rkEventSchedule.NeedQuest ) )
						{
							dwRet |= EER_ERROR_QUEST;
						}
					}break;
				case ENQS_COMPLETE:
					{
						if( !pMyQuest->IsEndedQuest( rkEventSchedule.NeedQuest ) )
						{
							dwRet |= EER_ERROR_QUEST;
						}
					}break;
				}
			} //if( pMyQuest )
		}
		else
		{
			dwRet |= EER_ERROR_SYSTEM;
		}
	} //if(rkEventSchedule.NeedQuest)

	//필요 아이템 수량 조건 체크
	if(rkEventSchedule.NeedItemNo)
	{
		PgInventory const* pkInv = pkPlayer->GetInven();
		if(pkInv)
		{
			size_t nItem = pkInv->GetInvTotalCount(rkEventSchedule.NeedItemNo);
			size_t nNeedItemCount = (rkEventSchedule.NeedItemCount == 0) ? 1 : rkEventSchedule.NeedItemCount;
			if(nItem < nNeedItemCount)
			{
				dwRet |= EER_ERROR_ITEM;
			}
		}
		else
		{
			dwRet |= EER_ERROR_SYSTEM;
		}
	}

	return dwRet;
}

bool COMMUNITY_EVENT::GetEventSchedule(SEventScheduleData& rkOut)
{
	CONT_DEF_EVENT_SCHEDULE const* pkContEventSchedule;
	g_kTblDataMgr.GetContDef(pkContEventSchedule);

	//int iEventNo = lua_tinker::call<bool>("GetSelectedCommunityEventNo");
	CONT_DEF_EVENT_SCHEDULE::const_iterator itFound = pkContEventSchedule->find(g_iEventNo);
	if(itFound != pkContEventSchedule->end())
	{
		rkOut = (*itFound).second;
		return true;
	}
	return false;
}

void COMMUNITY_EVENT::OnPlayerDead(void)
{
	if( NULL != g_pkWorld
		&& !g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND))
	{
		return;
	}

	PgPilot *pkPlayerPilot = g_kPilotMan.GetPlayerPilot();
	if(!pkPlayerPilot)
	{
		return;
	}
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPlayerPilot->GetUnit());
	if(!pkPlayer)
	{
		return;
	}

	if(pkPlayer->GetExpeditionGuid().IsNotNull()) //내가 원정대에 소속이 돼있나?
	{
		ContExpeditionMember kMemberList = g_kExpedition.GetMembers();
		ContExpeditionMember::const_iterator iterMember = kMemberList.begin();
		while(iterMember != kMemberList.end())
		{
			ContExpeditionMember::value_type const& pkMember = (*iterMember);
			++iterMember;

			if(!pkMember)
			{
				continue;
			}

			PgActor* pkActor = g_kPilotMan.FindActor(pkMember->kCharGuid);
			if( pkActor && pkActor->GetUnit() && pkActor->GetUnit()->IsAlive() )
			{ //내 파티원 중 한명이라도 살아 있다면 연출 없음
				return;
			}
		}
	}
	else if(pkPlayer->GetPartyGuid().IsNotNull()) //아니면 내가 파티에 소속이 돼있나?
	{
		ContPartyMember kMemberList;
		g_kParty.GetPartyMemberList(kMemberList);
		ContPartyMember::const_iterator iterMember = kMemberList.begin();
		while( iterMember != kMemberList.end() )
		{
			ContPartyMember::value_type const& pkPartyInfo = (*iterMember);
			++iterMember;

			if( !pkPartyInfo )
			{
				continue;
			}

			PgActor* pkActor = g_kPilotMan.FindActor(pkPartyInfo->kCharGuid);
			if( !pkActor || !pkActor->GetUnit() || pkActor->GetUnit()->IsAlive() )
			{ //내 파티원 중 한명이라도 살아 있다면 연출 없음
				return;
			}
		}
	}
	else //개인인가?
	{
		PgActor* pkActor = g_kPilotMan.GetPlayerActor();
		if(pkActor && pkActor->GetUnit() && pkActor->GetUnit()->IsAlive() )
		{ //내가 살아 있다면 연출 없음
			return;
		}
	}

	SEventScheduleData kEventSchedule;
	if(GetEventSchedule(kEventSchedule) && kEventSchedule.UnUsableReviveItem)
	{ //테이블 설정이 부활아이템 사용 불가인가
		lwCallUI("FRM_COMMUNITY_EVENT_FAILED");
	}
}