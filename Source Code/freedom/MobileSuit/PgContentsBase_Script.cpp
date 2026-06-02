#include "stdafx.h"
#include "PgUIScene.h"
#include "PgNetWork.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgChatMgrClient.h"
#include "PgContentsBase.h"
#include "PgPvPGame.h"
#include "PgEmporiaBattleClient.h"
#include "PgClientParty.h"
#include "PgEmporiaMgr.h"
#include "PgWorld.h"
#include "PgGuild.h"
#include "PgMobileSuit.h"
#include "PgMtoMChat.h"
#include "PgCommandMgr.h"
#include "PgCommunityEvent.h"

void TabGame( bool bDisplay )
{
	if ( PgContentsBase::ms_pkContents )
	{
		PgContentsBase::ms_pkContents->TabGame( bDisplay );
	}
}

void CallResponseBar()
{
	if ( g_pkWorld )
	{
		switch( g_pkWorld->GetAttr() )
		{
		case GATTR_PVP:
		case GATTR_EMPORIABATTLE:
			{
				if ( PgContentsBase::ms_pkContents )
				{
					PgContentsBase::ms_pkContents->CallResponse();
				}
			}break;
		case GATTR_MISSION:
			{
				XUIMgr.Activate(_T("FRM_DEAD_MISSION"));
			}break;
		case GATTR_BATTLESQUARE:
			{
				XUIMgr.Activate( std::wstring(L"FRM_BS_BATTLE_REBIRTHTIME") );
			}break;
		case GATTR_EVENT_GROUND:
			{
				SEventScheduleData kEventSchedule;
				if(COMMUNITY_EVENT::GetEventSchedule(kEventSchedule))
				{
					XUI::CXUI_Wnd* pkWnd = XUIMgr.Activate(_T("DeadDlg_NoRevive"));
					if(pkWnd) 
					{
						XUI::CXUI_Wnd* pkBtnRevive = pkWnd->GetControl(_T("BTN_USE_COIN"));
						if(pkBtnRevive)
						{
							if(!kEventSchedule.UnUsableReviveItem)
							{
								pkBtnRevive->Enable(true);
								pkWnd = pkWnd->GetControl(_T("SFRM_COLOR"));
								if(pkWnd)
								{
									pkWnd = pkWnd->GetControl(_T("SFR_SDW"));
									if(pkWnd)
									{
										pkWnd->Text(TTW(3));
									}
								}
							}
							else
							{ //부활 아이템 사용 불가이면
								pkBtnRevive->Enable(false); //아이템 사용 버튼 비활성화
								pkWnd = pkWnd->GetControl(_T("SFRM_COLOR"));
								if(pkWnd)
								{
									pkWnd = pkWnd->GetControl(_T("SFR_SDW"));
									if(pkWnd)
									{
										pkWnd->Text(TTW(302111));
									}
								}
							}
						}//if(pkBtnRevive)
					}//if(pkWnd) 
				}//if(COMMUNITY_EVENT::GetEventSchedule(kEventSchedule))
			}break;
		case GATTR_SUPER_GROUND:
		case GATTR_SUPER_GROUND_BOSS:
		case GATTR_ELEMENT_GROUND:
		case GATTR_ELEMENT_GROUND_BOSS:
			{
				if( NULL == XUIMgr.Get( std::wstring(L"FRM_SUPERGROUND_TIME_FAILED") ) )
				{
					XUIMgr.Activate( std::wstring(L"FRM_DEAD_SG") );
				}
			}break;
		default:
			{
				XUIMgr.Activate(_T("DeadDlg"));
			}break;
		}
	}
}

extern lwUIWnd lwCallMissionRetryUI();
void CloseResponseBar()
{
	if ( g_pkWorld )
	{
		switch( g_pkWorld->GetAttr() )
		{
		case GATTR_PVP:
			{
				XUIMgr.Close(PgContentsBase::ms_FRM_PVP_RESPONSE);
			}break;
		case GATTR_MISSION:
			{
				XUIMgr.Close(_T("FRM_DEAD_MISSION"));
				lwCallMissionRetryUI();
			}break;
		case GATTR_BATTLESQUARE:
			{
				XUIMgr.Close( std::wstring(L"FRM_BS_BATTLE_REBIRTHTIME") );
				XUIMgr.Close( std::wstring(L"FRM_BS_DEATH_PENALTY") );
			}break;
		case GATTR_SUPER_GROUND:
		case GATTR_SUPER_GROUND_BOSS:
		case GATTR_ELEMENT_GROUND:
		case GATTR_ELEMENT_GROUND_BOSS:
			{
				XUIMgr.Close( std::wstring(L"FRM_SUPERGROUND_TIME_FAILED") );
				XUIMgr.Close( std::wstring(L"FRM_DEAD_SG") );
			}break;
		case GATTR_EMPORIABATTLE:
			{
				if ( PgContentsBase::ms_pkContents )
				{
					PgContentsBase::ms_pkContents->CallLocationJoinUI();
				}
			}break;
		}
	}
}

PgContentsBase::Status PgContentsBase::GetContentsStatus()
{
	if ( ms_pkContents )
	{
		return ms_pkContents->GetStatus();
	}
	return STATUS_NONE;
}

void PgContentsBase::SetPvPLock( bool const bLock )
{
	if ( true == bLock )
	{
		g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_PvpResult, true));
		g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_PvpResult, false));
	}
	else
	{
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_PvpResult, true));
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_PvpResult, false));
	}
}

void PgContentsBase::SetPvpTabListMode(int const iSetMode)
{
	if( ms_pkContents )
	{
		ms_pkContents->m_iSetMode = iSetMode;
	}
}

int PgContentsBase::GetPvpTabListMode()
{
	if( ms_pkContents )
	{
		return ms_pkContents->m_iSetMode;
	}
	return 0;
}

void Net_GuildWarBoard()
{
	CONT_DEF_EMPORIA const *pkDefEmporia = NULL;
	g_kTblDataMgr.GetContDef( pkDefEmporia );
	if ( pkDefEmporia && pkDefEmporia->size() )
	{
		CONT_DEF_EMPORIA::const_iterator itr = pkDefEmporia->begin();

		EEmporiaStatusListType eType = ESLT_NONE;
		BM::Stream kPacket( PT_C_N_REQ_EMPORIA_STATUS_LIST, itr->first );
		kPacket.Push( eType );
		NETWORK_SEND( kPacket );
	}
}

void Call_EmporiaStatusBoard( lwUIWnd kWnd )
{
	Net_GuildWarBoard();	
}

void Call_GuildEntranceBoard()
{
	static BM::PgPackedTime kNextTime;
	BM::PgPackedTime kNowTime = BM::PgPackedTime::LocalTime();

	if( kNextTime < kNowTime 
	||	g_kGuildMgr.IsEntranceOpenGuildEmpty() )
	{// 길드게시판은 24시마다 갱신되므로 매번 요청할 필요가 없다.
		BM::Stream kPacket( PT_C_N_REQ_GET_ENTRANCE_OPEN_GUILD );
		NETWORK_SEND( kPacket );

		// 다음날 0시 0분 0초로 세팅
		kNextTime = kNowTime;
		CGameTime::AddTime(kNextTime, CGameTime::OneDay);
		kNextTime.Hour(0), kNextTime.Min(0), kNextTime.Sec(0);
	}
	else
	{
		g_kGuildMgr.InitSearchGuild(); // 검색결과 초기화
		g_kGuildMgr.WrapperUpdateEntranceOpenGuildUI();
	}
}

bool IsJoinPvP()
{
	if ( !g_pkWorld )
	{
		return false;
	}

	PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
	if ( pkPilot )
	{
		// 죽었으면 입장금지
		if ( pkPilot->GetAbil(AT_HP) <= 0 )
		{
			return false;
		}

		// 자주 수정되는 부분이므로, 해당 레벨 제한을 스크립트로 노출 함
// 		if ( PgClientPartyUtil::IsInParty() )
// 		{// 파티가 있으면 진입금지
// 			Notice_Show( TTW(700035), EL_Normal );
// 			return false;
// 		}
/*
		if ( g_pkWorld->IsHaveAttr(GATTR_FLAG_NOPVP) )
		{// 입장할 수 없는 곳
			Notice_Show( TTW(19010), EL_Normal );
			return false;
		}

		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			if( true == pkPlayer->IsItemEffect(SAFE_FOAM_EFFECTNO) )
			{
				lwAddWarnDataTT(400858);
				return false;
			}
			if ( BM::GUID::NullData() != pkPlayer->VendorGuid() )
			{// 입장할 수 없는 곳
				lwAddWarnDataTT(799453);
				return false;
			}
		}
		*/
		if( ( !g_pkWorld->IsHaveAttr(GATTR_DEFAULT) && !g_pkWorld->IsHaveAttr(GATTR_VILLAGE) )
			|| g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND)
			|| g_pkWorld->IsHaveAttr(GATTR_FLAG_BATTLESQUARE)
			|| g_pkWorld->IsHaveAttr(GATTR_FLAG_EMPORIABATTLE) )
		{
			lwAddWarnDataTT(201213);
			return false;
		}
		return true;
	}
	return false;
}

bool IsSlowDie( lwActor kActor )
{
	if ( PgContentsBase::ms_pkContents )
	{
		return PgContentsBase::ms_pkContents->IsSlowMotionDie( kActor() );
	}
	return false;
}

void OnClickPvP_LobbyClose()
{
	if ( !PgContentsBase::ms_pkContents || (PgContentsBase::STATUS_LOBBY >= PgContentsBase::ms_pkContents->GetStatus()) )
	{
		XUIMgr.BlockGlobalScript(false);
		NETWORK_SEND( BM::Stream(PT_C_T_REQ_EXIT_LOBBY) )
		SAFE_DELETE(PgContentsBase::ms_pkContents);
	}
}

void OnClickPvP_ReqDuel( lwGUID kCharGuid )
{
	if ( IsJoinPvP() )
	{
		PgPilot	*pkReqPilot = g_kPilotMan.FindPilot(kCharGuid());
		if( NULL == pkReqPilot )
		{
			return;
		}

		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkReqPilot->GetUnit());
		if( NULL == pkPlayer )
		{
			return;
		}

		if( pkPlayer )
		{
			if( true == pkPlayer->IsItemEffect(SAFE_FOAM_EFFECTNO) )
			{
				lwAddWarnDataTT(400859);
				return;
			}
		}

		XUI::CXUI_Wnd *pkWnd = NULL;
		if ( !XUIMgr.IsActivate(_T("BOX_PVP_ASKOUT"), pkWnd) )
		{
			PgPilot *pkAnsPilot = g_kPilotMan.FindPilot(kCharGuid());
			if ( pkAnsPilot )
			{
				//if(pkReqPilot->GetUnit() && pkReqPilot->GetUnit()->GetPartyGuid().IsNotNull())
				//{ //내가 파티 중이면...
				//	::Notice_Show_ByTextTableNo(200125, EL_Warning);
				//	return;
				//}
				//if(pkAnsPilot->GetUnit() && pkAnsPilot->GetUnit()->GetPartyGuid().IsNotNull())
				//{ //대상이 파티 중이면...
				//	std::wstring wstrText;
				//	WstringFormat( wstrText, MAX_PATH, TTW(200126).c_str(), pkAnsPilot->GetName().c_str() );
				//	::Notice_Show( wstrText, EL_Warning );
				//	return;
				//}
				if(pkAnsPilot->GetUnit()) //결투받을놈이 이미 결투 중인가?
				{
					if(pkAnsPilot->GetUnit()->GetAbil(AT_DUEL) > 0)
					{
						::Notice_Show_ByTextTableNo(201204, EL_Warning);
						return;
					}
					else if(pkAnsPilot->GetUnit()->GetAbil(AT_DUEL) < 0)
					{
						::Notice_Show_ByTextTableNo(201219, EL_Warning);
						return;
					}
				}
				if(pkReqPilot->GetUnit()) //결투건놈이 이미 결투 중인가
				{
					if(pkReqPilot->GetUnit()->GetAbil(AT_DUEL) > 0)
					{
						::Notice_Show_ByTextTableNo(201203, EL_Warning);
						return;
					}
					else if(pkReqPilot->GetUnit()->GetAbil(AT_DUEL) < 0)
					{
						::Notice_Show_ByTextTableNo(201220, EL_Warning);
						return;
					}
				}
				if(pkAnsPilot->GetUnit() && pkReqPilot->GetUnit())
				{
					float const fDistance = POINT3::Distance(pkAnsPilot->GetUnit()->GetPos(), pkReqPilot->GetUnit()->GetPos());
					if(fDistance > 300) //두놈의 거리가 300을 넘는다면 결투는 성립되지 않는다.
					{
						::Notice_Show_ByTextTableNo(201205, EL_Warning);
						return;
					}
				}


				pkWnd = XUIMgr.Call(_T("BOX_PVP_ASKOUT"), true );
				if ( pkWnd )
				{
					XUI::CXUI_Wnd *pkTemp = pkWnd->GetControl(_T("FRM_MESSAGE"));
					if ( pkTemp )
					{
						std::wstring wstrText;
						std::wstring const &wstrClass = TTW(pkAnsPilot->GetAbil(AT_CLASS)+30000);
						int iLv = pkAnsPilot->GetAbil(AT_LEVEL);
						WstringFormat( wstrText, MAX_PATH, TTW(200121).c_str(), pkAnsPilot->GetName().c_str(), wstrClass.c_str(), iLv );
						pkTemp->Text(wstrText);


						XUI::CXUI_Wnd* pkWndBg = pkWnd->GetControl(_T("SFRM_BG"));
						if(pkWndBg)
						{
							XUI::CXUI_Wnd* pkWndBgColor = pkWndBg->GetControl(_T("SFRM_BG_COLOR"));
							if(pkWndBgColor)
							{
								XUI::CXUI_Style_String kStyleString = pkTemp->StyleText();
								POINT const ptTextSize = Pg2DString::CalculateOnlySize(kStyleString);
								//pkTemp->Location(pkTemp->Location().x, (pkWndBgColor->Size().y - ptTextSize.y) / 2);
								pkTemp->TextPos(POINT2(pkTemp->TextPos().x, (pkWndBgColor->Size().y - ptTextSize.y) / 2));
							}
						}


						NETWORK_SEND( BM::Stream( PT_C_M_REQ_DUEL_PVP, kCharGuid() ) )
					}

					XUI::CXUI_Wnd *pkOk = pkWnd->GetControl(_T("BTN_OK"));
					if ( pkOk ) { pkOk->Visible(false); }
					XUI::CXUI_Wnd *pkCancle = pkWnd->GetControl(_T("BTN_CANCEL"));
					if ( pkCancle )  { pkCancle->Visible(false); }

					pkTemp = pkWnd->GetControl(_T("BTN_CANCEL2"));
					if ( pkTemp )
					{
						pkTemp->Visible(true);
						BM::Stream kPacket(PT_C_M_NFY_DUEL_PVP_CANCEL, kCharGuid());
						pkTemp->SetCustomData(&kPacket.Data().at(0), kPacket.Data().size());
					}
				}
			}	
			else
			{
				::Notice_Show_ByTextTableNo(201212, EL_Warning);
			}
		}
	}
}

EPVPTYPE GetPvPType( int iGndNo, EPVPTYPE kType, bool bRandom )
{
	PgPvPGame::GetPvPGroundType( iGndNo, kType, bRandom );
	return kType;
}

bool IsExerciseType()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->IsExerciseType();
	}
	return false;
}

bool IsLeagueType()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->IsLeagueType();
	}
	return false;
}

int const GetRankingModeLevel()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->GetRankingModeLevel();
	}
	return 0;
}

int const GetRankingModeLevelMin()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->GetRankingModeLevelMin();
	}
	return 0;
}

lwWString GetPvPGroundName( int const iGndNo )
{
	std::wstring wstrGndName;
	PgPvPGame::GetGroundName( iGndNo, wstrGndName );
	return lwWString(wstrGndName);
}

lwWString GetPreviewImgPath( int const iGndNo )
{
	std::wstring wstrImgPath;
	PgPvPGame::GetPreviewImgPath( iGndNo, wstrImgPath );
	return lwWString(wstrImgPath);
}

bool LoadPvPMaxPlayer( lwUIWnd kWnd, int const Type )
{
	return PgPvPGame::LoadPvPMaxPlayer( dynamic_cast<XUI::CXUI_List*>(kWnd()), Type );
}

bool LoadPvPGroundList( lwUIWnd kWnd, int const Type, int const UserCount )
{
	return PgPvPGame::LoadPvPGroundList( dynamic_cast<XUI::CXUI_List*>(kWnd()), Type, UserCount );
}

bool OnClickPvP_CreateRoom( lwUIWnd kWnd )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->OnClick_CreateRoom( kWnd() );
	}
	return false;
}

bool OnClick_RegistTeam( lwUIWnd kWnd )
{
	XUI::CXUI_Wnd *pkUIWnd = kWnd.GetSelf();
	if ( pkUIWnd )
	{
		XUI::CXUI_Edit *pkTemp_Edit = dynamic_cast<XUI::CXUI_Edit*>(pkUIWnd->GetControl(_T("EDIT_NAME")));
		if ( !pkTemp_Edit )
		{
			return false;
		}

		std::wstring wstrTeamName = pkTemp_Edit->EditText();
		if ( g_kClientFS.Filter( wstrTeamName, false, FST_BADWORD ) 
			|| !g_kUnicodeFilter.IsCorrect(UFFC_PVP_ROOM_NAME, wstrTeamName)
			)
		{//필터링 걸림
			lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(200000)), true);
			return false;
		}
		else if (wstrTeamName.empty())
		{//입력 안함
			lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(560080)), true);
			return false;
		}
		else if (wstrTeamName.size() >= MAX_TEAM_TITLE_LENGTH)
		{//너무 길다
			lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(560081)), true);
			return false;
		}
		else
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( pkPlayer )
			{
				if( BM::GUID::NullData() == pkPlayer->PartyGuid() )
				{//파티 없다
					lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(560050)), true);
					return false;
				}

				if( g_kParty.MasterGuid() != pkPlayer->GetID() )
				{//파장 아님
					lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(560051)), true);
					return false;
				}

				ContPartyMember kPartyMemberList;
				g_kParty.GetPartyMemberList(kPartyMemberList);
				if( 1 >= kPartyMemberList.size() )
				{//2인 이상이어야 등록 가능
					lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(560076)), true);
					return false;
				}

				TBL_DEF_PVPLEAGUE_TEAM kTeamInfo;
				kTeamInfo.wstrTeamName = wstrTeamName;
				
				ContPartyMember::const_iterator party_iter = kPartyMemberList.begin();
				while( party_iter != kPartyMemberList.end() )
				{
					ContPartyMember::value_type const& kPartyInfo = (*party_iter);
					SPvPLeague_User kUser;
					kUser.kGuid = kPartyInfo->kCharGuid;
					kUser.wstrName = kPartyInfo->kName;
					kTeamInfo.kContUserGuid.push_back(kUser);
					++party_iter;
				}

				BM::Stream kPacket(PT_C_N_REQ_REGIST_PVPLEAGUE_TEAM);
				kPacket.Push( pkPlayer->GetID() );
				kTeamInfo.WriteToPacket(kPacket);
				NETWORK_SEND(kPacket);
				return true;

			}
		}
	}
	return false;
}

bool OnClick_GiveUpTeam( lwUIWnd kWnd )
{
	XUI::CXUI_Wnd *pkUIWnd = kWnd.GetSelf();
	if ( pkUIWnd )
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{// 서버로 팀 등록 해제 요청
			BM::Stream kPacket(PT_C_N_REQ_GIVEUP_PVPLEAGUE_TEAM);
			kPacket.Push( pkPlayer->GetID() );
			NETWORK_SEND(kPacket);
			return true;
		}
	}
	return false;
}

bool SetEditPvP_Ground( int const iGroundNo, bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_Ground( iGroundNo, bNoSend );
	}
	return false;
}

bool IsMaster()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->IsRoomMaster();
	}
	return false;
}

bool SetEditPvP_Mode( EPVPMODE kMode, bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_Mode( kMode, bNoSend );
	}
	return false;
}

bool SetEditPvP_Type( EPVPTYPE kType, bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_Type( kType, bNoSend );
	}
	return false;
}

bool SetEditPvP_Time( int const iMinTime, bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_Time( iMinTime, bNoSend );
	}
	return false;
}

bool SetEditPvP_Point( int const iMinPoint, bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_Point( iMinPoint, bNoSend );
	}
	return false;
}

bool SetEditPvP_MaxLevel( short const sLevel, bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_MaxLevel( sLevel, bNoSend );
	}
	return false;
}

bool SetEditPvP_RoundCount( BYTE const kRoundCount, bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_RoundCount( kRoundCount, bNoSend );
	}
	return false;
}

bool SetEditPvP_HandyCap( bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_Option( E_PVP_OPT_USEHANDYCAP, bNoSend );
	}
	return false;
}


bool SetEditPvP_UseItem( bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_Option( E_PVP_OPT_USEITEM, bNoSend );
	}
	return false;
}

bool SetEditPvP_DisableDashJump( bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_Option( E_PVP_OPT_DISABLEDASHJUMP, bNoSend );
	}
	return false;
}

bool SetEditPvP_BattleLevel( bool bNoSend )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SetEdit_Option( E_PVP_OPT_USEBATTLELEVEL, bNoSend );
	}
	return false;
}

/*
bool SendChangePvP_HandyCap( lwUIWnd kUIWnd )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		if ( kUIWnd() )
		{
			if ( g_kPilotMan.IsMyPlayer( kUIWnd()->OwnerGuid() ) )
			{
				return pkGame->SendChange_HandyCap( kUIWnd.GetScrollCur() );
			}
		}
	}
	return false;
}
*/

void OnClickPvP_JoinRoom( char const *szPassWord, CONT_PVPROOM_LIST::key_type iRoomNo )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		XUI::CXUI_Wnd *pkFrameWnd = XUIMgr.Get(PgContentsBase::ms_FRM_PVP_LOBBY);
		if ( pkFrameWnd )
		{
			pkFrameWnd = pkFrameWnd->GetControl(PgContentsBase::ms_FRM_CLICK_FRAME);
			if ( !iRoomNo )
			{
				if ( pkFrameWnd )
				{
					pkFrameWnd->GetCustomData( &iRoomNo, sizeof(iRoomNo) );
					if ( !pkFrameWnd->Visible() || !iRoomNo )
					{
						// 방을 선택해 주세요.
						lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(400411)), true);
						lua_tinker::call<void,lwUIWnd>("InitPvP_LobbyRoomFrame", lwUIWnd(pkFrameWnd) );
						return;
					}
				}
				else
				{
					return;
				}
			}

			if ( pkGame->OnClick_JoinRoom( iRoomNo, std::string(szPassWord) ) )
			{
				lua_tinker::call<void,lwUIWnd>("InitPvP_LobbyRoomFrame", lwUIWnd(pkFrameWnd) );
			}
		}
	}
}

void OnClickPvP_RandomJoinRoom()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		pkGame->OnClick_RandomJoinRoom();
	}
}

void OnClickPvP_AutoJoinRoom()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		pkGame->OnClick_AutoJoinRoom();
	}
}

void OnClickPvP_RandomInviteUser()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		pkGame->OnClick_RandomInviteUser();
	}
}

void OnClickPvP_ChangePage( lwUIWnd kSelf )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		if ( kSelf() )
		{
			size_t iPage = 0;
			kSelf()->GetCustomData( &iPage, sizeof(size_t) );
			pkGame->OnClick_ChangePage(iPage);
		}
	}
}

void OnClickPvP_LeagueLobbyChangePage( lwUIWnd kSelf )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		if ( kSelf() )
		{
			pkGame->OnClick_LeagueLobbyChangePage(kSelf.GetSelf());
		}
	}
}

void OnClickPvP_LeagueLobbyNextPage( lwUIWnd kSelf )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		if ( kSelf() )
		{
			pkGame->OnClick_LeagueLobbyNextPage(kSelf.GetSelf());
		}
	}
}

void OnClickPvP_LeagueLobbyPrevPage( lwUIWnd kSelf )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		if ( kSelf() )
		{
			pkGame->OnClick_LeagueLobbyPrevPage(kSelf.GetSelf());
		}
	}
}

void OnClickPvP_LeagueLobbyBeginPage( lwUIWnd kSelf )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		if ( kSelf() )
		{
			pkGame->OnClick_LeagueLobbyBeginPage(kSelf.GetSelf());
		}
	}
}

void OnClickPvP_LeagueLobbyEndPage( lwUIWnd kSelf )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		if ( kSelf() )
		{
			pkGame->OnClick_LeagueLobbyEndPage(kSelf.GetSelf());
		}
	}
}

void OnClickPvP_ViewWaitRoom( lwUIWnd kSelf )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		pkGame->OnClick_ViewWaitRoom( kSelf() );
	}
}

void OnClickPvP_ViewRanking(void)
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		pkGame->OnClick_ViewRanking();
	}
}

void OnClickPvP_TeamChange( int const iTeam )
{
	NETWORK_SEND( BM::Stream(PT_C_T_REQ_TEAM_CHANGE,iTeam) )
}

void OnClickPvP_GameStart( int const iType )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		pkGame->OnClick_GameStart( iType );
	}
}

bool OnClickPvP_RoomSlot( lwUIWnd kWnd, bool bRButton )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->ClickSlot( kWnd(), bRButton );
	}
	return false;
}

bool OnClickPvP_RoomEntry( int const iEntryNo )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->ClickEntry( iEntryNo );
	}
	return false;
}

bool OnClickPvP_KickUser( lwGUID kCharGuid, bool bKick )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->KickUser( kCharGuid(), bKick );
	}
	return false;
}

bool OnClickPvP_EntrustMaster( lwGUID kCharGuid )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->EntrustMaster( kCharGuid() );
	}
	return false;
}

void UpdatePvPPoint( lwUIWnd kWnd, lwUIWnd kGraphWnd )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		pkGame->UpdatePoint( kWnd(), kGraphWnd() );
	}
}

void PvPResult( int const iTeam )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		pkGame->Result( iTeam );
	}
}

int GetEntityPointByTriggerID( char const * pTriggerID )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->GetEntityPointByTriggerID( pTriggerID );
	}
	return 0;
}

int GetEntityLevelByTriggerID( char const * pTriggerID )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->GetEntityLevelByTriggerID( pTriggerID );
	}
	return 0;
}

int GetEntityLinkByTriggerID( char const * pTriggerID )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->GetEntityLinkByTriggerID( pTriggerID );
	}
	return 0;
}

int GetPointByLevel( int const Level )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->GetPointByLevel( Level );
	}
	return 0;
}

int GetLevelByPoint( int const Point )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->GetLevelByPoint( Point );
	}
	return 0;
}

int OnUpdateUIEmergencyEscape()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->OnUpdateUIEmergencyEscape();
	}
	return 0;
}

void OnUpdateStrongholdLink()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		pkGame->UpdateStrongholdLink();
	}
}

void OnUpdateStrongholdTrigger()
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		pkGame->UpdateStrongholdTrigger();
	}
}

void OnSetPvpDamageTwinkle( lwUIWnd UISelf, DWORD const TickTime, DWORD const TotalTime )
{
	XUI::CXUI_AniBar* pAniSelf = dynamic_cast<XUI::CXUI_AniBar*>(UISelf.GetSelf());
	if( pAniSelf )
	{
		pAniSelf->SetTwinkle(true);
		pAniSelf->TwinkleInterTime(TickTime);
		pAniSelf->TwinkleTime(TotalTime);
	}
}

void OnSetPvpDieUserSlot( lwUIWnd UISelf, bool const bDie )
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( pSelf )
	{
		XUI::CXUI_Wnd* pTemp = pSelf->GetControl(L"IMG_RANK");
		if( pTemp )
		{
			pTemp->Visible(!bDie);
		}
		pTemp = pSelf->GetControl(L"IMG_KILL");
		if( pTemp )
		{
			pTemp->GrayScale(bDie);
		}
		pTemp = pSelf->GetControl(L"IMG_DEATH");
		if( pTemp )
		{
			pTemp->GrayScale(bDie);
		}
		pTemp = pSelf->GetControl(L"FRM_NAME");
		if( pTemp )
		{
			pTemp->GrayScale(bDie);
		}
		pTemp = pSelf->GetControl(L"FRM_KILL");
		if( pTemp )
		{
			pTemp->GrayScale(bDie);
		}
		pTemp = pSelf->GetControl(L"FRM_DEATH");
		if( pTemp )
		{
			pTemp->GrayScale(bDie);
		}
		pTemp = pSelf->GetControl(L"FRM_TOTALPOINT");
		if( pTemp )
		{
			pTemp->GrayScale(bDie);
		}
		pTemp = pSelf->GetControl(L"FRM_DEADMARK");
		if( pTemp )
		{
			pTemp->Visible(bDie);
			pTemp->GrayScale(bDie);
		}
		pTemp = pSelf->GetControl(L"BAR_HP");
		if( pTemp )
		{
			pTemp->GrayScale(bDie);
		}
		pTemp = pSelf->GetControl(L"BAR_MP");
		if( pTemp )
		{
			pTemp->GrayScale(bDie);
		}
		pSelf->GrayScale(bDie);
	}
}

void OnCallPvPUserSlotChange( lwUIWnd UISelf, float const fTotalTick )
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( pSelf )
	{
		if ( true == pSelf->Visible() )
		{
			pSelf->Visible(false);
			XUI::CXUI_Wnd* pParent = pSelf->Parent();
			if( pParent )
			{
				pParent->SetCustomData(&fTotalTick, sizeof(fTotalTick));

				BM::vstring	kStr(L"SLOT_PVP_ANI_");
				kStr += pSelf->BuildIndex();
				XUI::CXUI_Wnd* pSlotAni = pParent->GetControl(kStr);
				if( pSlotAni )
				{
					int const iTotalUVIndex = lwUIWnd(pSlotAni).GetMaxUVIndex();
					float const fTime = g_pkApp->GetAccumTime() + (fTotalTick /  static_cast<float>(iTotalUVIndex));
					pSlotAni->SetCustomData(&fTime, sizeof(fTime));
					pSlotAni->Visible(true);
				}
			}
		}
	}
}

void OnUpdatePvPUserSlotUV( lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( pSelf )
	{
		float fTime = UISelf.GetCustomData<float>();
		int const iTotalUVIndex = UISelf.GetMaxUVIndex();
		float const fAccumTime = g_pkApp->GetAccumTime();

		if( fTime < fAccumTime )
		{
			XUI::CXUI_Wnd* pParent = pSelf->Parent();
			if( pParent )
			{	
				BM::vstring	kStr(L"SLOT_PVP_");
				kStr += pSelf->BuildIndex();
				XUI::CXUI_Wnd* pUserSlot = pParent->GetControl(kStr);
				if( pUserSlot )
				{
					pUserSlot->Visible(false);
				}

				if( iTotalUVIndex == pSelf->UVInfo().Index )
				{
					pSelf->UVUpdate(1);
					pSelf->Visible(false);
					if( pUserSlot )
					{
						pUserSlot->Visible(true);
					}
				}

				float const fTotalTick = lwUIWnd(pParent).GetCustomData<float>();
				fTime += fTotalTick / static_cast<float>(iTotalUVIndex);
				pSelf->UVUpdate(pSelf->UVInfo().Index + 1);
				UISelf.SetCustomData<float>(fTime);
			}
		}
	}
}

bool SortLobbyList( bool const bName )
{
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		return pkGame->SortLobbyList( bName );
	}
	return false;
}

void CloseKillEffect( lwUIWnd UISelf)
{
	PgEmporiaBattleClient *pkGame = dynamic_cast<PgEmporiaBattleClient*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		XUI::CXUI_Wnd* pWnd = UISelf.GetSelf();
		int iPoint = 0;
		XUI::CXUI_Wnd* pNum = pWnd->GetControl(_T("FRM_ONE"));
		if( pNum && pNum->Visible() )
		{
			iPoint += (pNum->UVInfo().Index - 1);
		}

		pNum = pWnd->GetControl(_T("FRM_TEN"));
		if( pNum && pNum->Visible() )
		{
			iPoint += ((pNum->UVInfo().Index - 1) * 10);
		}

		pNum = pWnd->GetControl(_T("FRM_HUN"));
		if( pNum && pNum->Visible() )
		{
			iPoint += ((pNum->UVInfo().Index - 1) * 100);
		}
		
		bool bIsCore = false;
		pWnd->GetCustomData(&bIsCore, sizeof(bIsCore));
		pkGame->CloseKillEffect(bIsCore, iPoint);
	}
}

bool IsIamGroundOwner()
{
	if ( g_pkWorld )
	{
		SGroundOwnerInfo const &kGroungOwnerInfo = g_pkWorld->GroundOwnerInfo();

		switch ( kGroungOwnerInfo.byType )
		{
		case SGroundOwnerInfo::OWNER_CHARACTER:
			{
				BM::GUID kGuid;
				g_kPilotMan.GetPlayerPilotGuid( kGuid );
				return kGroungOwnerInfo.kID == kGuid;
			}break;
		case SGroundOwnerInfo::OWNER_PARTY:
			{
				PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
				if( pkMyPlayer )
				{
					return pkMyPlayer->PartyGuid() == kGroungOwnerInfo.kID;
				}
			}break;
		case SGroundOwnerInfo::OWNER_GUILD:
			{
				return g_kGuildMgr.GuildGuid() == kGroungOwnerInfo.kID;
			}break;
		default:
			{
				return true;
			}break;
		}
	}
	return false;
}

bool lwUpdateRemainTimeBar( lwUIWnd kUIWnd )
{
	XUI::CXUI_AniBar *pkWnd = dynamic_cast<XUI::CXUI_AniBar*>(kUIWnd());
	if ( pkWnd )
	{
		__int64 i64EndTime = 0i64;
		if ( true == pkWnd->GetCustomData( &i64EndTime, sizeof(i64EndTime) ) )
		{
			__int64 i64NowTime = g_kEventView.GetLocalSecTime( CGameTime::DEFAULT );
			__int64 i64RemainTime = std::max( i64EndTime - i64NowTime, 0i64 );

			int iRemainTimeMin = static_cast<int>(i64RemainTime / CGameTime::MINUTE);
			int iRemainTimeSec = static_cast<int>(i64RemainTime / CGameTime::SECOND);
			int iRemainTimeMS = static_cast<int>((i64RemainTime / 100000i64) % 100i64);

			BM::vstring vstrTime( TTW(71017) );
			vstrTime += L" ";
			vstrTime += TTW(71020);
			vstrTime.Replace( L"#MIN#", BM::vstring( iRemainTimeMin, L"%02d") );
			vstrTime.Replace( L"#SEC#", BM::vstring( iRemainTimeSec % 60, L"%02d") );
			vstrTime.Replace( L"#MS#", BM::vstring( iRemainTimeMS, L"%02d") );

			pkWnd->Now( pkWnd->Max() - iRemainTimeSec );
			pkWnd->Text( vstrTime );
		}
	}
	return false;
}

bool SetHighRankEmporiaBattleItemRewardSlot(XUI::CXUI_Wnd* pParent, int const RewardCount, int const iItemNo)
{
	if( !pParent )
	{
		return false;
	}

	if( 0 == iItemNo )
	{
		return false;
	}

	BM::vstring vStr(L"ICN_REWARD");
	vStr += RewardCount;

	XUI::CXUI_Wnd* pkIcon = pParent->GetControl(vStr);
	if( pkIcon )
	{
		pkIcon->SetCustomData(&iItemNo, sizeof(iItemNo));
	}
	return true;
}

void SetHighRankEmporiaBattleItemReward(XUI::CXUI_Wnd* pParent)
{
	if( !pParent )
	{
		return;
	}
	
	int const MAX_SLOT = 3;

	CONT_DEF_EMPORIA const* pDefEmporia = NULL;
	g_kTblDataMgr.GetContDef(pDefEmporia);
	if( pDefEmporia )
	{
		CONT_DEF_EMPORIA::const_iterator emp_itor = pDefEmporia->find(pParent->OwnerGuid());
		if( emp_itor != pDefEmporia->end() )
		{
			CONT_DEF_EMPORIA::mapped_type const& kEmporiaInfo = emp_itor->second;

			int iRewardCount = 0;

			for(int i = 0; i < MAX_EMPORIA_GRADE; ++i)
			{
				if( SetHighRankEmporiaBattleItemRewardSlot(pParent, iRewardCount, kEmporiaInfo.kReward.kGrade[ (MAX_EMPORIA_GRADE - 1) - i ].iItem) )
				{
					++iRewardCount;
					if( MAX_SLOT == iRewardCount )
					{
						break;
					}
				}
			}

			if( iRewardCount < MAX_SLOT )
			{
				for(int i = 0; i < TBL_DEF_EMPORIA_REWARD::ECG_END; ++i)
				{
					if( SetHighRankEmporiaBattleItemRewardSlot(pParent, iRewardCount, kEmporiaInfo.kReward.kChallenge[ (TBL_DEF_EMPORIA_REWARD::ECG_END - 1) - i ].iItem) )
					{
						++iRewardCount;
						if( MAX_SLOT == iRewardCount )
						{
							break;
						}
					}
				}
			}
		}
	}
}

void lwOnOKMercenaryJoin(lwUIWnd kSelf)
{
	if( kSelf.IsNil() )
	{
		return;
	}

	lwPacket kPacket = kSelf.GetCustomDataAsPacket();
	if(NULL == kPacket()){ return; }
	NETWORK_SEND(*kPacket());
}

void lwCallMercenaryJoinConfirm(lwPacket kPacket, lwGUID kGuid)
{
	if(!g_pkWorld)
	{
		return;
	}
	if( g_pkWorld->IsHaveAttr(GATTR_FLAG_EMPORIABATTLE|GATTR_FLAG_PVP|GATTR_BATTLESQUARE) )
	{
		lwAddWarnDataTT(71572);
		return;
	}

	XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_MERCENARY_JOIN");
	if( !pMainUI )
	{
		return;
	}

	pMainUI->OwnerGuid(kGuid());

	SetHighRankEmporiaBattleItemReward(pMainUI);
	lwUIWnd(pMainUI).SetCustomDataAsPacket(kPacket);
}

void lwCallMercenaryJoinNotice(lwGUID kGuid)
{
	g_kChatMgrClient.AddMessage(70090, SChatLog(CT_EVENT), true, EL_Normal);

	SMtoMChatData kChatData;
	kChatData.kName = TTW(71559);
	kChatData.dwColor = 0xFFFFFF99;

	g_kMToMMgr.Add( kGuid(), std::wstring(), MTM_MSG_EM_ADMERCENARY, kChatData, false );
}

void lwOnClickMercenaryJoinConfirm(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	BM::GUID kGuid = pSelf->OwnerGuid();
	size_t kBattleIndex = 0;
	pSelf->GetCustomData(&kBattleIndex, sizeof(kBattleIndex));
	bool bIsAtk = pSelf->OwnerState() ? true : false;

	BM::Stream kPacket(PT_C_M_REG_JOIN_EMPORIA_MERCENARY);
	kPacket.Push(kGuid);
	kPacket.Push(kBattleIndex);
	kPacket.Push(bIsAtk);

	SGuildEmporiaInfo kInfo = g_kGuildMgr.GetEmporiaInfo();
	kInfo.byMercenaryFlag = (true == bIsAtk)?(EMPORIA_MERCENARY_ATK):(EMPORIA_MERCENARY_DEF);
	g_kGuildMgr.SetEmporiaInfo(kInfo);

	lwCallMercenaryJoinConfirm(lwPacket(&kPacket), kGuid);
}

void lwWrapperUpdateEntranceOpenGuildUI( int const iCurrentPage )
{
	g_kGuildMgr.WrapperUpdateEntranceOpenGuildUI( iCurrentPage );
}

void lwSearchGuild(lwWString lwSearchGuildName)
{
	g_kGuildMgr.SearchGuild(lwSearchGuildName);
}

void lwCallGuildApplicationUI( int const iCustomDataAsInt )
{
	g_kGuildMgr.CallGuildApplicationUI( iCustomDataAsInt );
}

void lwReqGuildEntrance()
{
	g_kGuildMgr.ReqGuildEntrance();
}

void lwOnDrawGuildMark(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( pSelf )
	{
		SGuildOtherInfo	kGuildInfo;
		if( g_kGuildMgr.GetGuildInfo(pSelf->OwnerGuid(), kGuildInfo) )
		{
			SUVInfo Info = pSelf->UVInfo();
			Info.Index = kGuildInfo.cEmblem + 1;
			pSelf->UVInfo(Info);
		}
	}
}

void EmporiaCoreHPRefresh(lwUIWnd kMainUI)
{
	PgEmporiaBattleClient* pkEmporiaBattle = dynamic_cast<PgEmporiaBattleClient*>(PgContentsBase::ms_pkContents);
	if( !pkEmporiaBattle || PgContentsBase::STATUS_PLAYING != pkEmporiaBattle->GetStatus() )
	{
		return;
	}

	XUI::CXUI_Wnd* pkMainUI = kMainUI.GetSelf();
	if( pkMainUI )
	{
		BM::vstring vStr(L"FRM_CORE_HP");
		XUI::CXUI_Wnd* pkAnibar = pkMainUI->GetControl(vStr);
		if( pkAnibar && pkAnibar->OwnerGuid() != BM::GUID::NullData() )
		{
			pkEmporiaBattle->RefreshCoreHP( pkAnibar->OwnerGuid() );
		}
	}
}

void PgContentsBase::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "GetContentsStatus", &PgContentsBase::GetContentsStatus);
	def(pkState, "Net_GuildWarBoard", &Net_GuildWarBoard);
	def(pkState, "Call_EmporiaStatusBoard", &Call_EmporiaStatusBoard );
	def(pkState, "Call_GuildEntranceBoard", &Call_GuildEntranceBoard );
	def(pkState, "UpdateRemainTimeBar", &lwUpdateRemainTimeBar);

	def(pkState, "SetPvPLock",  &PgContentsBase::SetPvPLock );
	def(pkState, "IsJoinPvP", &IsJoinPvP);
	def(pkState, "IsSlowMotionDie", &IsSlowDie);
	def(pkState, "GetRandomPvPGround", &PgPvPGame::GetRandomPvPGround);
	def(pkState, "GetPvPType", &GetPvPType);
	def(pkState, "GetPvPGroundName", &GetPvPGroundName);
	def(pkState, "GetPreviewImgPath", &GetPreviewImgPath);
	def(pkState, "LoadPvPMaxPlayer", &LoadPvPMaxPlayer);
	def(pkState, "LoadPvPGroundList", &LoadPvPGroundList);
	def(pkState, "OnClickPvP_LobbyClose", &OnClickPvP_LobbyClose);
	def(pkState, "OnClickPvP_CreateRoom", &OnClickPvP_CreateRoom);
	def(pkState, "OnClickPvP_JoinRoom", &OnClickPvP_JoinRoom);
	def(pkState, "OnClickPvP_RandomJoinRoom", &OnClickPvP_RandomJoinRoom);
	def(pkState, "OnClickPvP_RandomInviteUser", &OnClickPvP_RandomInviteUser);
	def(pkState, "OnClickPvP_ChangePage", &OnClickPvP_ChangePage);
	def(pkState, "OnClickPvP_ViewWaitRoom", &OnClickPvP_ViewWaitRoom);
	def(pkState, "OnClickPvP_ViewRanking", &OnClickPvP_ViewRanking);
	def(pkState, "OnClickPvP_TeamChange", &OnClickPvP_TeamChange);
	def(pkState, "OnClickPvP_GameStart", &OnClickPvP_GameStart);
	def(pkState, "OnClickPvP_RoomSlot", &OnClickPvP_RoomSlot);
	def(pkState, "OnClickPvP_KickUser", &OnClickPvP_KickUser);
	def(pkState, "OnClickPvP_EntrustMaster", &OnClickPvP_EntrustMaster);
	def(pkState, "OnClickPvP_ReqDuel", &OnClickPvP_ReqDuel);
	def(pkState, "OnSetPvpDamageTwinkle", &OnSetPvpDamageTwinkle);
	def(pkState, "OnSetPvpDieUserSlot", &OnSetPvpDieUserSlot);
	def(pkState, "OnCallPvPUserSlotChange", &OnCallPvPUserSlotChange);
	def(pkState, "OnUpdatePvPUserSlotUV", &OnUpdatePvPUserSlotUV);
	def(pkState, "OnClickPvP_RoomEntry", &OnClickPvP_RoomEntry);
	def(pkState, "OnClick_RegistTeam", &OnClick_RegistTeam);
	def(pkState, "OnClick_GiveUpTeam", &OnClick_GiveUpTeam);
	def(pkState, "OnClickPvP_AutoJoinRoom", &OnClickPvP_AutoJoinRoom);
	def(pkState, "OnClickPvP_LeagueLobbyChangePage", &OnClickPvP_LeagueLobbyChangePage);
	def(pkState, "OnClickPvP_LeagueLobbyNextPage", &OnClickPvP_LeagueLobbyNextPage);
	def(pkState, "OnClickPvP_LeagueLobbyPrevPage", &OnClickPvP_LeagueLobbyPrevPage);
	def(pkState, "OnClickPvP_LeagueLobbyBeginPage", &OnClickPvP_LeagueLobbyBeginPage);
	def(pkState, "OnClickPvP_LeagueLobbyEndPage", &OnClickPvP_LeagueLobbyEndPage);

	def(pkState, "SetEditPvP_Ground", &SetEditPvP_Ground);
	def(pkState, "SetEditPvP_Mode", &SetEditPvP_Mode);
	def(pkState, "SetEditPvP_Type", &SetEditPvP_Type);
	def(pkState, "SetEditPvP_Time", &SetEditPvP_Time);
	def(pkState, "SetEditPvP_Point", &SetEditPvP_Point);
	def(pkState, "SetEditPvP_MaxLevel", &SetEditPvP_MaxLevel);
	def(pkState, "SetEditPvP_RoundCount", &SetEditPvP_RoundCount);
	def(pkState, "SetEditPvP_HandyCap", &SetEditPvP_HandyCap);
	def(pkState, "SetEditPvP_UseItem", &SetEditPvP_UseItem);
	def(pkState, "SetEditPvP_DisableDashJump", &SetEditPvP_DisableDashJump);
	def(pkState, "SetEditPvP_BattleLevel", &SetEditPvP_BattleLevel);
	def(pkState, "UpdatePvPPoint", &UpdatePvPPoint);
	def(pkState, "PvPResult", &PvPResult);
	def(pkState, "GetEntityPointByTriggerID", &GetEntityPointByTriggerID);
	def(pkState, "GetEntityLevelByTriggerID", &GetEntityLevelByTriggerID);
	def(pkState, "GetEntityLinkByTriggerID", &GetEntityLinkByTriggerID);
	def(pkState, "GetPointByLevel", &GetPointByLevel);
	def(pkState, "GetLevelByPoint", &GetLevelByPoint);
	def(pkState, "OnUpdateUIEmergencyEscape", &OnUpdateUIEmergencyEscape);
	def(pkState, "OnUpdateStrongholdLink", &OnUpdateStrongholdLink);
	def(pkState, "OnUpdateStrongholdTrigger", &OnUpdateStrongholdTrigger);
	def(pkState, "SortLobbyList", &SortLobbyList);
	def(pkState, "CloseKillEffect", &CloseKillEffect);

	def(pkState, "IsIamGroundOwner", &IsIamGroundOwner);
	def(pkState, "SetPvpTabListMode", &PgContentsBase::SetPvpTabListMode);
	def(pkState, "GetPvpTabListMode", &PgContentsBase::GetPvpTabListMode);
	def(pkState, "IsExerciseType", &IsExerciseType);
	def(pkState, "IsLeagueType", &IsLeagueType);
	def(pkState, "IsMaster", &IsMaster);
	def(pkState, "GetRankingModeLevel", &GetRankingModeLevel);
	def(pkState, "GetRankingModeLevelMin", &GetRankingModeLevelMin);

	def(pkState, "EmporiaCoreHPRefresh", &EmporiaCoreHPRefresh);

	//emporia mercenary
	def(pkState, "CallMercenaryJoinConfirm", &lwCallMercenaryJoinConfirm);
	def(pkState, "CallMercenaryJoinNotice", &lwCallMercenaryJoinNotice);
	def(pkState, "OnClickMercenaryJoinConfirm", &lwOnClickMercenaryJoinConfirm);
	def(pkState, "OnOKMercenaryJoin", &lwOnOKMercenaryJoin);

	def(pkState, "UpdateEntranceOpenGuildUI", &lwWrapperUpdateEntranceOpenGuildUI);
	def(pkState, "CallGuildApplicationUI", &lwCallGuildApplicationUI);
	def(pkState, "ReqGuildEntrance", &lwReqGuildEntrance);
	def(pkState, "SearchGuild", &lwSearchGuild);

	def(pkState, "OnDrawGuildMark", &lwOnDrawGuildMark);
}
