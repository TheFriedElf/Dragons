#include "stdafx.h"
#include "PgUIScene.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgChatMgrClient.h"
#include "PgEmporiaBattleClient_ProtectDragon.h"
#include "PgMobileSuit.h"

PgEmporiaBattleClient_ProtectDragon::PgEmporiaBattleClient_ProtectDragon()
:	m_usDefWinPointLimit(600)
,	m_bCallBigDragon(false)
//,	m_iRebirthCount(600)
{

}

PgEmporiaBattleClient_ProtectDragon::~PgEmporiaBattleClient_ProtectDragon()
{

}

bool PgEmporiaBattleClient_ProtectDragon::ProcessPacket( BM::Stream::DEF_STREAM_TYPE const wType, BM::Stream& kPacket )
{
	switch( wType )
	{
	case PT_M_C_NFY_GAME_START:
		{
			RecvSyncRemainTime(kPacket);
			RecvSyncTeamPoint(kPacket);
			Start(kPacket);
			UpdateDefWinLimitCount(GetWnd(false));
		}break;
	case PT_M_C_NFY_BATTLE_CHANGE_AREA_TO_BIGDRAGON:
		{
			Recv_PT_M_C_NFY_BATTLE_CHANGE_AREA_TO_BIGDRAGON( kPacket );
		}break;
	default:
		{
			return PgEmporiaBattleClient::ProcessPacket( wType, kPacket );
		}break;
	}
	return true;
}

void PgEmporiaBattleClient_ProtectDragon::StartUI()
{
	Notice_Show( TTW(74013), EL_Warning );
}

void PgEmporiaBattleClient_ProtectDragon::SetDragonAreaInfo( SBattleAreaInfo const &kInfo, E_SLOT_TYPE const SlotType )const
{
	XUI::CXUI_Wnd* pMainUI = GetWnd(false);
	if ( pMainUI )
	{
		XUI::CXUI_Wnd* pSlot = pMainUI->GetControl(L"FRM_BIG_DRAGON");
		if( pSlot )
		{
			SetDragonAreaInfo( pSlot, kInfo, SlotType );
		}
	}
}

void PgEmporiaBattleClient_ProtectDragon::SetDragonAreaInfo( XUI::CXUI_Wnd* pSlot, SBattleAreaInfo const& kInfo, E_SLOT_TYPE const SlotType ) const
{
	if( !pSlot )
	{
		return;
	}

	XUI::CXUI_Wnd* pRedPoint = pSlot->GetControl(L"FRM_POINT_RED");
	if( pRedPoint )
	{
		pRedPoint->Text(BM::vstring( GetTeamUserCount(TEAM_ATTACKER) ));
	}

	XUI::CXUI_Wnd* pBluePoint = pSlot->GetControl(L"FRM_POINT_BLUE");
	if( pBluePoint )
	{
		pBluePoint->Text(BM::vstring( GetTeamUserCount(TEAM_DEFENCER) ));
	}

	XUI::CXUI_AniBar* pDragonHP = dynamic_cast<XUI::CXUI_AniBar*>(pSlot->GetControl(L"BAR_HP"));
	if( pDragonHP )
	{
		int const iNowHP = (kInfo.NowDragonHP<=kInfo.MaxDragonHP) ? kInfo.NowDragonHP : kInfo.MaxDragonHP;
		int const iMaxHP = kInfo.MaxDragonHP;

		if( pDragonHP->Max() != iMaxHP )
		{
			pDragonHP->Max( iMaxHP );
		}
		pDragonHP->Now( iNowHP );
		pDragonHP->GrayScale(0 == iNowHP);

		int const iHPPer = (iMaxHP>0) ? std::max<int>(1,(iNowHP/static_cast<float>(iMaxHP)*100)) : 0;
		BM::vstring vStr(TTW(71569));
		vStr.Replace(L"#VAL#",iHPPer);
		pDragonHP->Text(vStr);

		int iPercent = static_cast<int>((kInfo.NowDragonHP / static_cast<float>(kInfo.MaxDragonHP)) * 100);
		XUI::CXUI_Wnd* pHpBg = pSlot->GetControl(L"IMG_HP_BG");
		if( pHpBg )
		{
			if( iPercent < 25 )
			{
				pHpBg->Visible(false);
			}
			else
			{
				pHpBg->Visible(true);
				SUVInfo kInfo = pHpBg->UVInfo();
				float fUVIndex = iPercent / (100.0f / static_cast<float>((kInfo.U * kInfo.V)));
				pHpBg->Visible(fUVIndex > 1.0f);
				pHpBg->UVUpdate(static_cast<int>(fUVIndex));
			}
		}
	}
}

void PgEmporiaBattleClient_ProtectDragon::UpdateDefWinLimitCount(XUI::CXUI_Wnd* pMainUI) const
{
	if( !pMainUI )
	{
		pMainUI = GetWnd(false);
	}

	if ( pMainUI )
	{
		XUI::CXUI_AniBar* pDefWinLimit = dynamic_cast<XUI::CXUI_AniBar*>(pMainUI->GetControl(L"BAR_HP"));
		if( pDefWinLimit )
		{
			if( pDefWinLimit->Max() != m_usDefWinPointLimit )
			{
				pDefWinLimit->Max(m_usDefWinPointLimit);
			}

			pDefWinLimit->Now(m_kDefenceGuild.GetPoint());

			BM::vstring vStr(TTW(71570));
			vStr.Replace(L"#NOW#", pDefWinLimit->Now());
			vStr.Replace(L"#MAX#", pDefWinLimit->Max());
			pDefWinLimit->Text(vStr);

			if ( BM::GUID::IsNull( pDefWinLimit->OwnerGuid()) )
			{// OwnerGuid가 있으면 경고메세지를 뿌린거다.
				float const fRate = static_cast<float>(pDefWinLimit->Now()) / static_cast<float>(pDefWinLimit->Max());
				if ( fRate >= 9.0f )
				{
					pDefWinLimit->OwnerGuid( BM::GUID::Create() );

					int const iTextNo = ( TEAM_ATTACKER == m_iMyTeam ? 74011 : 74012 );
					Notice_Show( TTW(iTextNo), EL_Warning );
				}
			}
		}
	}
}

void PgEmporiaBattleClient_ProtectDragon::ReadFromStartAddOnPacket(BM::Stream &kPacket)
{
	PgEmporiaBattleClient::ReadFromStartAddOnPacket(kPacket);
	kPacket.Pop( m_usDefWinPointLimit );
}

void PgEmporiaBattleClient_ProtectDragon::Recv_PT_M_C_NFY_GAME_INTRUDER( BM::Stream &kPacket )
{
	PgEmporiaBattleClient::Recv_PT_M_C_NFY_GAME_INTRUDER(kPacket);
	CONT_BATTLEAREA_INFO::iterator btl_itr = m_kContBattleAreaInfo.find( 1 );
	if( btl_itr != m_kContBattleAreaInfo.end() )
	{
		SetDragonAreaInfo( btl_itr->second );
	}
}

void PgEmporiaBattleClient_ProtectDragon::Recv_PT_M_C_NFY_GAME_EXIT( BM::Stream &kPacket )
{
	PgEmporiaBattleClient::Recv_PT_M_C_NFY_GAME_EXIT(kPacket);
	CONT_BATTLEAREA_INFO::iterator btl_itr = m_kContBattleAreaInfo.find( 1 );
	if( btl_itr != m_kContBattleAreaInfo.end() )
	{
		SetDragonAreaInfo( btl_itr->second );
	}
}

void PgEmporiaBattleClient_ProtectDragon::Recv_PT_M_C_NFY_BATTLE_AREA_USER( BM::Stream &kPacket )
{
	PgEmporiaBattleClient::Recv_PT_M_C_NFY_BATTLE_AREA_USER(kPacket);

	CONT_BATTLEAREA_INFO::iterator btl_itr = m_kContBattleAreaInfo.find( 1 );
	if( btl_itr != m_kContBattleAreaInfo.end() )
	{
		SetDragonAreaInfo( btl_itr->second );
	}
}

void PgEmporiaBattleClient_ProtectDragon::Recv_PT_M_C_NFY_BATTLE_CHANGE_AREA_TO_BIGDRAGON( BM::Stream& kPacket)
{
	BM::GUID kGuid;
	SEmporiaBattleInfo_Client kObject;

	m_kContObejctInfo.clear();
	PU::TLoadTable_AM( kPacket, m_kContObejctInfo );

	BuildBattleAreaInfo();

	XUIMgr.Close(ms_SFRM_EMBATTLE_PTDRA_TAB);
}

bool PgEmporiaBattleClient_ProtectDragon::RecvHP( CUnit *pkUnit, int const iBeforeHP, int const iNewHP )
{
	int const iTeam = pkUnit->GetAbil(AT_TEAM);

	switch ( pkUnit->UnitType() )
	{
	case UT_MONSTER:
	case UT_BOSSMONSTER:
		{
			CONT_EMBATTLE_OBJECT_INFO_CLIENT::iterator obj_itr = m_kContObejctInfo.find( pkUnit->GetID() );
			if ( obj_itr != m_kContObejctInfo.end() )
			{
				CONT_EMBATTLE_OBJECT_INFO_CLIENT::mapped_type const& kObjInfo = obj_itr->second;

				CONT_BATTLEAREA_INFO::iterator btl_itr = m_kContBattleAreaInfo.find( kObjInfo.iBattleAreaIndex );
				if ( btl_itr != m_kContBattleAreaInfo.end() )
				{
					CONT_BATTLEAREA_INFO::key_type const& kAreaID = btl_itr->first;
					CONT_BATTLEAREA_INFO::mapped_type& kAreaInfo = btl_itr->second;

					if ( kAreaInfo.NowDragonHP > 0 )
					{
						kAreaInfo.NowDragonHP = iNewHP;
						if( kObjInfo.iBattleAreaIndex == 1 )
						{
							SetDragonAreaInfo( kAreaInfo );				

							double const fMaxHP = static_cast<double>(kAreaInfo.MaxDragonHP);
							double const fNewRate = static_cast<double>(iNewHP) / fMaxHP;
							double const fOldRate = static_cast<double>(iBeforeHP) /fMaxHP;

							if ( 0.2 >= fNewRate && 0.2 < fOldRate )
							{
								Notice_Show( TTW(71571), EL_Warning );
							}
						}
					}
				}
			}
		}break;
	default:
		{

		}break;
	}

	return true;
}

bool PgEmporiaBattleClient_ProtectDragon::SyncPoint( WORD const nUserKillPoint_Atk, WORD const nCoreBreakPoint_Atk, WORD const nUserKillPoint_Def, WORD const nCoreBreakPoint_Def )
{
	UpdateDefWinLimitCount();
	return true;
}

// 리턴값은 메인코어
bool PgEmporiaBattleClient_ProtectDragon::DestroyCore( BM::GUID const &kID )
{
	std::wstring wstrMsg;
	std::wstring kChildFormName;
	CONT_EMBATTLE_OBJECT_INFO_CLIENT::iterator obj_itr = m_kContObejctInfo.find( kID );
	if ( obj_itr != m_kContObejctInfo.end() )
	{
		CONT_BATTLEAREA_INFO::iterator btl_itr = m_kContBattleAreaInfo.find( obj_itr->second.iBattleAreaIndex );
		if ( btl_itr != m_kContBattleAreaInfo.end() )
		{
			btl_itr->second.NowDragonHP = 0;
			if( obj_itr->second.iBattleAreaIndex == 1 )
			{
				SetDragonAreaInfo( btl_itr->second );
			}

			std::wstring wstrMsg;
			WstringFormat( wstrMsg, MAX_PATH, TTW(74009).c_str(), TTW( 74000 + btl_itr->second.iIndex ).c_str() );
			Notice_Show( wstrMsg, EL_Warning );
		}
	}

	return false;//
}

void PgEmporiaBattleClient_ProtectDragon::BuildBattleAreaInfo(void)
{
	PgEmporiaBattleClient::BuildBattleAreaInfo();

	CONT_BATTLEAREA_INFO::iterator Area_rst = m_kContBattleAreaInfo.begin();
	while( Area_rst != m_kContBattleAreaInfo.end() )
	{
		if( Area_rst->first == 1 )
		{
			SetDragonAreaInfo( Area_rst->second );
		}
		++Area_rst;
	}

	XUI::CXUI_Wnd* pMainUI = GetWnd(false);
	if ( pMainUI )
	{
		XUI::CXUI_Wnd* pSlot = pMainUI->GetControl(L"FRM_BIG_DRAGON");
		if( pSlot )
		{
			pSlot->Visible(true);
		}
	}

}

void PgEmporiaBattleClient_ProtectDragon::SetPointEffect( BM::GUID const &kKillerID, BM::GUID const &kDieID, short const usAddPoint, bool const bDestroyCore )
{
	bool const IsKillerIsMine = g_kPilotMan.IsMyPlayer(kKillerID);
	POINT2 FormPos;

	XUI::CXUI_List *pkWnd = dynamic_cast<XUI::CXUI_List*>(XUIMgr.Activate( ms_LST_EMBATTLE_KILL_MSG ));
	if ( pkWnd )
	{
		XUI::SListItem*	pkItem = pkWnd->AddItem(_T(""));
		if( pkItem && pkItem->m_pWnd )
		{
			float const fNowTime = g_pkApp->GetAccumTime();
			pkItem->m_pWnd->SetCustomData(&fNowTime, sizeof(fNowTime));
			FormPos.x = pkWnd->Location().x;
			FormPos.y = pkWnd->Location().y + (pkItem->m_pWnd->Size().y * (pkWnd->GetTotalItemCount() - 1)) + (pkItem->m_pWnd->Size().y * 0.5f);

			XUI::CXUI_Wnd*	pkTemp = pkItem->m_pWnd->GetControl(_T("FRM_ICON"));
			if( pkTemp )
			{
				SUVInfo	Info = pkTemp->UVInfo();
				Info.Index = (BM::GUID::IsNull(kKillerID))?(KMM_SELFKILL):(KMM_TARGETKILL);
				pkTemp->UVInfo(Info);
			}

			pkTemp = pkItem->m_pWnd->GetControl(_T("FRM_KILLER"));
			if( pkTemp )
			{
				bool bFind = BM::GUID::IsNotNull( kKillerID );

				if ( true == bFind )
				{
					CONT_EMPORIA_BATTLE_USER::const_iterator user_itr = m_kContUser.find( kKillerID );
					if ( user_itr != m_kContUser.end() )
					{
						pkTemp->FontColor( (IsKillerIsMine)?(0xFFFFE98E):(GetTeamFontColor(user_itr->second.iTeam)) );
						pkTemp->Text(user_itr->second.wstrName);
					}
					else
					{
						CONT_EMBATTLE_OBJECT_INFO_CLIENT::const_iterator obj_itr = m_kContObejctInfo.find( kKillerID );
						if ( obj_itr != m_kContObejctInfo.end() )
						{
							pkTemp->FontColor(0xFFBAFF00);
							pkTemp->Text(TTW(73008));
						}
						else
						{
							bFind = false;
						}
					}
				}

				if ( !bFind )
				{
					// 자살
					pkTemp->Text( std::wstring() );
				}
			}

			pkTemp = pkItem->m_pWnd->GetControl(_T("FRM_INJURE"));
			if( pkTemp )
			{
				if( bDestroyCore )
				{
					pkTemp->FontColor(0xFFBAFF00);
					pkTemp->Text(TTW(73008));
				}
				else
				{
					bool const IsMyActor = g_kPilotMan.IsMyPlayer(kDieID);

					CONT_EMPORIA_BATTLE_USER::const_iterator user_itr = m_kContUser.find( kDieID );
					if ( user_itr != m_kContUser.end() )
					{
						pkTemp->FontColor( (IsMyActor)?(0xFFFFE98E):(GetTeamFontColor(user_itr->second.iTeam)) );
						pkTemp->Text(user_itr->second.wstrName);
					}
				}
			}

			int const iMaxListItem = pkWnd->GetTotalItemCount();
			for(int i = 0; i < (iMaxListItem - MAX_KILL_NFY_MSG); ++i)
			{
				pkWnd->DeleteItem(pkWnd->FirstItem());
			}
		}
	}
}

int const PgEmporiaBattleClient_ProtectDragon::GetTeamUserCount( int const iTeam ) const
{
	int iTotalCount = 0;
	CONT_EMPORIA_BATTLE_USER::const_iterator user_itr = m_kContUser.begin();
	while( user_itr != m_kContUser.end() )
	{
		CONT_EMPORIA_BATTLE_USER::mapped_type const& kUser = user_itr->second;
		if( iTeam == kUser.iTeam )
		{
			++iTotalCount;
		}
		++user_itr;
	}
	return iTotalCount;
}
