#include "stdafx.h"
#include "Variant/PgEmporia.h"
#include "PgNetwork.h"
#include "PgChatMgrClient.h"
#include "PgPilotMan.h"
#include "PgGuild.h"
#include "PgEmporiaMgr.h"
#include "lwUI.h"
#include "lwGUID.h"
#include "PgEventTimer.h"
#include "PgMToMChat.h"
#include "lwGuildUI.h"

extern void lwCallMercenaryJoinNotice(lwGUID kGuid);
extern void SetHighRankEmporiaBattleItemReward(XUI::CXUI_Wnd* pParent);
extern bool IsIamGroundOwner();

namespace Quest
{
	extern void SetCutedTextLimitLength(XUI::CXUI_Wnd *pkWnd, std::wstring const &rkText, std::wstring kTail, long const iCustomTargetWidth = 0);
}

char const * EVENT_EM_AD_MERCENARY = "EVENT_EMPORIA_AD_MERCENARY";

bool PgEmporiaStateUI::ReadFromPacket( BM::Stream &kPacket )
{
	m_iChallengeGuildCount = 0;
	m_i64TotalChallengeExp = 0i64;
	m_i64BattleTime = 0i64;

	kPacket.Pop( m_kEmporiaID );
	kPacket.Pop( m_iState );

	switch ( m_iState )
	{
	case EMPORIA_TERM_OF_CHALLENGE:
		{
			kPacket.Pop( m_iChallengeGuildCount );
			kPacket.Pop( m_i64TotalChallengeExp );	
		}// break 사용 금지
	case EMPORIA_PEACE:
	case EMPORIA_BEFORE_CHALLENGE_OF_TOURNAMENT:
	case EMPORIA_CHALLENGE_OF_TOURNAMENT:
	case EMPORIA_OWNERSHIP_OF_TOURNAMENT:
		{
			kPacket.Pop( m_i64BattleTime );
			m_kTournament.ReadFromPacket( kPacket );

			for ( size_t i = 0; i<MAX_EMPORIA_GRADE ; ++i )
			{
				m_kEmporia[i].ReadFromPacket( kPacket );
			}
		}break;
	default:
		{
			m_kTournament.Release();
			for ( size_t i = 0; i<MAX_EMPORIA_GRADE ; ++i )
			{
				m_kEmporia[i] = PgEmporia();
			}
		}break;
	}
	return true;
}

void PgEmporiaStateUI::SetUIOwnerName( XUI::CXUI_Wnd *pkControl, std::wstring const &wstrName )const
{
	if ( pkControl )
	{
		XUI::CXUI_Wnd *pkControlSub = pkControl->GetControl( L"FRM_GUILD_NAME" );
		if ( pkControlSub )
		{
			Quest::SetCutedTextLimitLength(pkControlSub, wstrName, _T(".."));
		}
	}
}

void PgEmporiaStateUI::SetUIGuildInfo( XUI::CXUI_Wnd *pkControl, SEmporiaGuildInfo const &kGuildInfo, bool const bLinkParent, bool const bIsLose )const
{
	if ( pkControl )
	{
		bool const bIsEmptyGuild =	( PgEmporiaTournament::ms_kEmptyGuildInfo.kGuildID == kGuildInfo.kGuildID ) || BM::GUID::IsNull(kGuildInfo.kGuildID);
		pkControl->OwnerGuid(kGuildInfo.kGuildID);

		XUI::CXUI_Wnd *pkControlSub = pkControl->GetControl( L"IMG_LINK" );
		if ( pkControlSub )
		{
			pkControlSub->Visible( bLinkParent && !bIsEmptyGuild );
		}

		pkControlSub = pkControl->GetControl( L"IMG_MARK" );
		if ( pkControlSub )
		{
			pkControlSub->Visible( !bIsEmptyGuild );
			pkControlSub->UVUpdate( kGuildInfo.nEmblem + 1 );
			pkControlSub->SetCustomData( kGuildInfo.wstrName.c_str(), sizeof(std::wstring::value_type) * kGuildInfo.wstrName.size() );
		}

		pkControlSub = pkControl->GetControl( L"IMG_MARK_X" );
		if ( pkControlSub )
		{
			pkControlSub->Visible( bIsEmptyGuild || bIsLose );
		}

		pkControlSub = pkControl->GetControl( L"FRM_NAME" );
		if( pkControlSub )
		{
			Quest::SetCutedTextLimitLength(pkControlSub, kGuildInfo.wstrName, _T(".."));
		}
	}
}

void PgEmporiaStateUI::SetMercenaryJoinBtn( XUI::CXUI_Wnd* pkControl, PgEmporiaTournamentElement const& kBattleInfo, SEmporiaGuildInfoEx const &kGuildInfo, PgLimitClass const& kLimitClass, bool const bIsAtkGuild )
{
	if( !pkControl 
		|| !g_pkWorld
		)
	{
		return;
	}

	XUI::CXUI_Button* pkButton = dynamic_cast<XUI::CXUI_Button*>(pkControl->GetControl( L"BTN_MERCENARY_JOIN" ));
	if ( pkButton )
	{
		if( g_pkWorld->IsHaveAttr(GATTR_FLAG_EMPORIABATTLE) )
		{
			pkButton->Disable( true );
			return;
		}

		bool bDisable = true;
		bool bIsAtk = bIsAtkGuild;
		size_t Index = kBattleInfo.GetIndex();
		if ( true == kGuildInfo.m_kLimitMercenary.IsAccess( kLimitClass ) )
		{
			PgEmporiaTournamentElement const * pkTempElement = &kBattleInfo;
			while ( pkTempElement )
			{
				Index = pkTempElement->GetIndex();

				if ( PgEmporiaTournamentElement::FLAG_CHECK_END & pkTempElement->GetState() )
				{
					bIsAtk = pkTempElement->IsParentLinkAttack();
					pkTempElement = pkTempElement->GetParent();
					continue;
				}

				if ( PgEmporiaTournamentElement::STATE_CREATED & pkTempElement->GetState() )
				{
					if( (kGuildInfo.kGuildID==pkTempElement->GetGuildGuid(true))
					|| (kGuildInfo.kGuildID==pkTempElement->GetGuildGuid(false)) )
					{
						bDisable = false;
					}
				}

				break;
			}
		}

		pkButton->Disable( bDisable );
		pkButton->Text( (bDisable ? std::wstring() : TTW(71070)) );
		pkButton->SetCustomData(&Index, sizeof(Index));
		pkButton->OwnerState((int)bIsAtk);
		pkButton->OwnerGuid(m_kEmporiaID);
	}
}

void PgEmporiaStateUI::Update( bool const bIsInfo, XUI::CXUI_Wnd *pkWnd )
{
	if ( !pkWnd )
	{
		return;
	}

	XUI::CXUI_Wnd *pkControl = NULL;

	if ( true == bIsInfo )
	{
		pkControl = pkWnd->GetControl( L"FRM_INFO_1" );
		if ( pkControl )
		{// 도전 남은 시간
			SYSTEMTIME kSystemTime;
			::memset( &kSystemTime, 0, sizeof(SYSTEMTIME) );

			BM::vstring vStr(TTW(71055));

			if ( EMPORIA_TERM_OF_CHALLENGE == m_iState )
			{
				__int64 const i64ChallengeEndTime = m_i64BattleTime - PgEmporiaTournament::EMPORIA_BATTLE_CHALLENGE_ENDTIME_BEFORE_BATTLETIME;
				__int64 const i64NowTime = g_kEventView.GetLocalSecTime( CGameTime::DEFAULT );
				__int64 const i64RemainTime = i64ChallengeEndTime - i64NowTime;
				if ( 0i64 < i64RemainTime )
				{
					CGameTime::SecTime2SystemTime( i64RemainTime, kSystemTime, CGameTime::DEFAULT );
					if ( 1 < kSystemTime.wDay )
					{
						--kSystemTime.wDay;
					}
				}
			}

			vStr.Replace( L"#DAY#", BM::vstring(kSystemTime.wDay, L"%02u") );
			vStr.Replace( L"#TIME#", BM::vstring(kSystemTime.wHour, L"%02u") );
			vStr.Replace( L"#MIN#", BM::vstring(kSystemTime.wMinute, L"%02u") );

			pkControl->Text( vStr );
		}

		pkControl = pkWnd->GetControl( L"FRM_INFO_2" );
		if ( pkControl )
		{
			BM::vstring vStr(TTW(71060));
			vStr.Replace( L"#NUM#", m_iChallengeGuildCount );
			pkControl->Text( vStr );
		}

		pkControl = pkWnd->GetControl( L"FRM_INFO_3" );
		if ( pkControl )
		{
			if ( 0 == m_iChallengeGuildCount )
			{
				BM::vstring vStr( m_i64TotalChallengeExp );
				pkControl->Text( vStr );
			}
			else
			{
				__int64 const iMoney = std::max<__int64>( m_i64TotalChallengeExp / static_cast<__int64>(m_iChallengeGuildCount), 1i64 );
				pkControl->Text( BM::vstring(GetMoneyMU(EMoney_Gold,iMoney)) );
			}
		}
		return;
	}

	pkWnd->OwnerGuid( m_kEmporiaID );

	bool bAmIGuildOwner = false;
	PgLimitClass kLimitClass;

	{
		PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		if ( pkMyPlayer )
		{
			bAmIGuildOwner = g_kGuildMgr.IsOwner(pkMyPlayer->GetID());
			if ( EMPORIA_KEY_NONE == g_kGuildMgr.GetEmporiaInfo().byType )
			{
				kLimitClass.SetClass( GET_CLASS_LIMIT(pkMyPlayer->GetAbil(AT_CLASS)) );
				kLimitClass.SetLevel( static_cast<short>(pkMyPlayer->GetAbil(AT_LEVEL)) );
			}
		}
	}

	// 정보를 만들어 보자
	BM::vstring const vstrOwnerBoxNameBase(L"FRM_OWNER_GRADE_");
	BM::vstring	const vstrBoxNameBase(L"FRM_BOX_");
	BM::vstring const vstrPanBoxNameBase(L"FRM_PAN_BOX_");
	BM::vstring const vstrPanArrowNameBase(L"FRM_PAN_ARROW_");

	BM::vstring vstrControlName;
	XUI::CXUI_Wnd *pkControlSub = NULL;
	XUI::CXUI_Button *pkButton = NULL;

	struct SPanState
	{
		enum eState{STATE_NONE,STATE_ING,STATE_WILL,STATE_END,};
		SPanState()
			:	iBoxUV(STATE_END)
			,	i64Time(_I64_MAX)
			,	i64EndTime(_I64_MAX)
		{}

		size_t	iBoxUV;
		__int64	i64Time;
		__int64 i64EndTime;

		static size_t GetIndex( size_t i )
		{
			switch ( i )
			{
			case 3:case 4:case 5:case 6:{return 1;}break;
			case 1:case 2:{return 2;}break;
			case 0:{return 3;}break;
			case 100:{return 4;}break;
			case 101:{return 5;}break;
			}
			return 0;
		}

		void Check( PgEmporiaTournamentElement const &rkElement, E_EMPORIA_STATE const eEmporiaState )
		{
			__int64 const i64ElementTime = rkElement.GetBattleTime();
			if ( 0i64 == i64ElementTime )
			{
				return;
			}

			BYTE const byState = rkElement.GetState();
			if ( PgEmporiaTournamentElement::STATE_CREATED & byState )
			{
				iBoxUV = SPanState::STATE_ING;
			}
			else if ( 0 == (PgEmporiaTournamentElement::FLAG_CHECK_END & byState) )
			{
				if ( (SPanState::STATE_END==iBoxUV) && (EMPORIA_PEACE!=eEmporiaState) )
				{
					iBoxUV = SPanState::STATE_WILL;
				}
			}
				
			if ( i64Time > i64ElementTime )
			{
				i64Time = i64ElementTime;
			}
		}

		DWORD GetTitleColor()const
		{
			if ( STATE_WILL == iBoxUV )
			{
				return 0xFF1B2100;
			}
			else if ( STATE_END == iBoxUV )
			{
				return 0xFF282828;
			}
			return 0xFFFFFFFF;
		}

		DWORD GetColor()const
		{
			if ( STATE_END == iBoxUV )
			{
				return 0xFFD7D7D7;
			}
			return 0xFFFFFFFF;
		}
	};

	SPanState kPanState[6];
	size_t const iPanStateMax = _countof(kPanState);

	switch ( m_iState )
	{
	case EMPORIA_CLOSE:
		{
		}break;
	case EMPORIA_PEACE:
		{
			kPanState[0].iBoxUV  = SPanState::STATE_WILL;
			kPanState[0].i64Time = m_i64BattleTime - PgEmporiaTournament::EMPORIA_BATTLE_CHALLENGE_STARTTIME_BEFORE_BATTLETIME;
			kPanState[0].i64EndTime = m_i64BattleTime - PgEmporiaTournament::EMPORIA_BATTLE_CHALLENGE_ENDTIME_BEFORE_BATTLETIME;
		}break;
	case EMPORIA_TERM_OF_CHALLENGE:
		{
			kPanState[0].iBoxUV  = SPanState::STATE_ING;
			kPanState[0].i64Time = m_i64BattleTime - PgEmporiaTournament::EMPORIA_BATTLE_CHALLENGE_STARTTIME_BEFORE_BATTLETIME;
			kPanState[0].i64EndTime = m_i64BattleTime - PgEmporiaTournament::EMPORIA_BATTLE_CHALLENGE_ENDTIME_BEFORE_BATTLETIME;
		}break;
	case EMPORIA_BEFORE_CHALLENGE_OF_TOURNAMENT:
	case EMPORIA_CHALLENGE_OF_TOURNAMENT:
	case EMPORIA_OWNERSHIP_OF_TOURNAMENT:
		{
			kPanState[0].i64Time = m_i64BattleTime - PgEmporiaTournament::EMPORIA_BATTLE_CHALLENGE_STARTTIME_BEFORE_BATTLETIME;
			kPanState[0].i64EndTime = m_i64BattleTime - PgEmporiaTournament::EMPORIA_BATTLE_CHALLENGE_ENDTIME_BEFORE_BATTLETIME;
		}break;
	default:
		{
		}break;
	}

	// 엠포리아 소유 정보
	size_t iIndex = 0;
	for ( ; iIndex < MAX_EMPORIA_GRADE ; ++iIndex )
	{
		vstrControlName = vstrOwnerBoxNameBase + (iIndex + 1);
		pkControl = pkWnd->GetControl( vstrControlName );
		if ( pkControl )
		{
			if ( true == m_kEmporia[iIndex].IsOpen() )
			{
				pkControl->GrayScale( false );

				SEmporiaGuildInfo kOwnerGuildInfo;
				m_kEmporia[iIndex].GetOwner(kOwnerGuildInfo);
				pkControl->OwnerGuid(kOwnerGuildInfo.kGuildID);
				SetUIOwnerName( pkControl, kOwnerGuildInfo.wstrName );
			}
			else
			{
				pkControl->GrayScale( true );
				SetUIOwnerName( pkControl, TTW(71061) );
			}
		}
	}
	
	// 도전자 토너먼트
	iIndex = 0;
	vstrControlName = vstrBoxNameBase + iIndex;
	pkControl = pkWnd->GetControl( vstrControlName );
	while ( pkControl )
	{
		PgEmporiaTournamentElement *pkElement = m_kTournament.GetElement( iIndex );
		if ( pkElement )
		{
			bool const bWinAttack = ( PgEmporiaTournamentElement::FLAG_WIN_ATTACK & pkElement->GetState() );
			bool const bWinDefence = ( PgEmporiaTournamentElement::FLAG_WIN_DEFENCE & pkElement->GetState() );

			// Attack
			pkControlSub = pkControl->GetControl( L"FRM_ATTACK" );
			if ( pkControlSub )
			{
				SEmporiaGuildInfoEx kAttackGuildInfo;
				pkElement->GetGuildInfoEx( true, kAttackGuildInfo );
				SetUIGuildInfo( pkControlSub, kAttackGuildInfo, bWinAttack, bWinDefence );
				SetMercenaryJoinBtn(pkControlSub, *pkElement, kAttackGuildInfo, kLimitClass, true);
			}

			// Defence
			pkControlSub = pkControl->GetControl( L"FRM_DEFENCE" );
			if ( pkControlSub )
			{
				SEmporiaGuildInfoEx kDefenceGuildInfo;
				pkElement->GetGuildInfoEx( false, kDefenceGuildInfo );
				SetUIGuildInfo( pkControlSub, kDefenceGuildInfo, bWinDefence, bWinAttack );
				SetMercenaryJoinBtn(pkControlSub, *pkElement, kDefenceGuildInfo, kLimitClass, false);
			}

			size_t const iPanStateIndex = SPanState::GetIndex( iIndex );
			if ( 0 < iPanStateIndex )
			{
				kPanState[iPanStateIndex].Check( *pkElement, static_cast<E_EMPORIA_STATE>(m_iState) );
			}
		}
		else
		{
			SEmporiaGuildInfo kEmptyGuildInfo;
			pkControlSub = pkControl->GetControl( L"FRM_ATTACK" );
			if ( pkControlSub )
			{
				SetUIGuildInfo( pkControlSub, kEmptyGuildInfo, false, true );
				pkButton = dynamic_cast<XUI::CXUI_Button*>(pkControlSub->GetControl( L"BTN_MERCENARY_JOIN" ));
				if ( pkButton )
				{
					pkButton->Text(L"");
					pkButton->Disable( true );
				}
			}

			pkControlSub = pkControl->GetControl( L"FRM_DEFENCE" );
			if ( pkControlSub )
			{
				SetUIGuildInfo( pkControlSub, kEmptyGuildInfo, false, true );
				pkButton = dynamic_cast<XUI::CXUI_Button*>(pkControlSub->GetControl( L"BTN_MERCENARY_JOIN" ));
				if ( pkButton )
				{
					pkButton->Text(L"");
					pkButton->Disable( true );
				}
			}
		}

		++iIndex;
		vstrControlName = vstrBoxNameBase + iIndex;
		pkControl = pkWnd->GetControl( vstrControlName );
	}

	// 엠포리아 토너먼트
	iIndex = PgEmporiaTournament::EMPORIA_BATTLE_TOURNAMENT_OWNERSHIP_STARTINDEX;
	vstrControlName = vstrBoxNameBase + iIndex;
	pkControl = pkWnd->GetControl( vstrControlName );
	while ( pkControl )
	{
		size_t const iEmporiaIndex = iIndex - PgEmporiaTournament::EMPORIA_BATTLE_TOURNAMENT_OWNERSHIP_STARTINDEX;
		if ( MAX_EMPORIA_GRADE <= iEmporiaIndex )
		{
			break;
		}

		// 엠포리아 소유 정보
		BM::vstring vstrOwnerGradeName;
		vstrOwnerGradeName = vstrOwnerBoxNameBase + (iEmporiaIndex + 1);
		XUI::CXUI_Wnd *pkDefenceLeft = pkWnd->GetControl( vstrOwnerGradeName );
		vstrOwnerGradeName += L"_RIGHT";
		XUI::CXUI_Wnd *pkAttackRight = pkWnd->GetControl( vstrOwnerGradeName );

		PgEmporiaTournamentElement *pkElement = m_kTournament.GetElement( iIndex );
		if ( pkElement )
		{
			bool const bWinAttack = ( PgEmporiaTournamentElement::FLAG_WIN_ATTACK & pkElement->GetState() );
			bool const bWinDefence = ( PgEmporiaTournamentElement::FLAG_WIN_DEFENCE & pkElement->GetState() );
			
			bool bLinkParent = false;
			if(pkElement->GetParent())
			{
				if(S_OK == pkElement->GetParent()->IsBattle())
				{
					bLinkParent = true;
				}
			}
			

			// Attack
			SEmporiaGuildInfoEx kAttackGuildInfo;
			pkElement->GetGuildInfoEx( true, kAttackGuildInfo );
			SetUIGuildInfo( pkControl->GetControl( L"FRM_ATTACK" ), kAttackGuildInfo, bWinAttack&&bLinkParent, bWinDefence );

			// Defence
			SEmporiaGuildInfoEx kDefenceGuildInfo;
			pkElement->GetGuildInfoEx( false, kDefenceGuildInfo );
			SetUIGuildInfo( pkControl->GetControl( L"FRM_DEFENCE" ), kDefenceGuildInfo, bWinDefence&&bLinkParent, bWinAttack );
			SetMercenaryJoinBtn(pkControl->GetControl( L"FRM_DEFENCE" ), *pkElement, kDefenceGuildInfo, kLimitClass, false);

			size_t const iPanStateIndex = SPanState::GetIndex( iIndex );
			if ( 0 < iPanStateIndex )
			{
				kPanState[iPanStateIndex].Check( *pkElement, static_cast<E_EMPORIA_STATE>(m_iState) );
			}

			pkDefenceLeft ? pkDefenceLeft->GrayScale(bWinAttack) : 0;
			pkAttackRight ? pkAttackRight->Visible(bWinAttack) : 0;
			SetUIOwnerName( pkDefenceLeft, kDefenceGuildInfo.wstrName );
			SetUIOwnerName( pkAttackRight, kAttackGuildInfo.wstrName );
		}
		else
		{
			SEmporiaGuildInfo kGuildInfo;
			SetUIGuildInfo( pkControl->GetControl( L"FRM_ATTACK" ), kGuildInfo, false, true );
				
			m_kEmporia[iEmporiaIndex].GetOwner( kGuildInfo );
			pkControlSub = pkControl->GetControl( L"FRM_DEFENCE" );
			if ( pkControlSub )
			{
				SetUIGuildInfo( pkControlSub, kGuildInfo, false, false );
				pkButton = dynamic_cast<XUI::CXUI_Button*>(pkControlSub->GetControl( L"BTN_MERCENARY_JOIN" ));
				if ( pkButton )
				{
					pkButton->Text(L"");
					pkButton->Disable( true );
				}
			}

			pkDefenceLeft ? pkDefenceLeft->GrayScale(false) : 0;
			pkAttackRight ? pkAttackRight->Visible(false) : 0;
		}

		++iIndex;
		vstrControlName = vstrBoxNameBase + iIndex;
		pkControl = pkWnd->GetControl( vstrControlName );
	}

	iIndex = 0;
	vstrControlName = vstrPanBoxNameBase + iIndex;
	pkControl = pkWnd->GetControl( vstrControlName );
	while ( pkControl )
	{
		SPanState &rkPanState = kPanState[iIndex];

		pkControl->UVUpdate( rkPanState.iBoxUV );
		pkControlSub = pkControl->GetControl( L"FRM_TITLE" );
		if ( pkControlSub )
		{
			pkControlSub->FontColor( rkPanState.GetTitleColor() );
		}

		pkControlSub = pkControl->GetControl( L"FRM_STATE" );
		if ( pkControlSub )
		{
			pkControlSub->FontColor( rkPanState.GetColor() );
			pkControlSub->Text( TTW(71066 + rkPanState.iBoxUV ) );
		}

		pkControlSub = pkControl->GetControl( L"FRM_TIME" );
		if ( pkControlSub )
		{
			pkControlSub->FontColor( rkPanState.GetColor() );

			if ( _I64_MAX > rkPanState.i64Time )
			{
				SYSTEMTIME kSystime;
				CGameTime::SecTime2SystemTime( rkPanState.i64Time, kSystime, CGameTime::DEFAULT );

				BM::vstring vstrTemp( TTW(11000 + kSystime.wDayOfWeek) );

				if ( _I64_MAX > rkPanState.i64EndTime )
				{
					SYSTEMTIME kEndSystime;
					CGameTime::SecTime2SystemTime( rkPanState.i64EndTime, kEndSystime, CGameTime::DEFAULT );

					vstrTemp += L" ~ ";
					vstrTemp += TTW(11000 + kEndSystime.wDayOfWeek);
					pkControlSub->Text( vstrTemp );

					vstrTemp = TTW(71056);
					vstrTemp += L" ";
					vstrTemp += TTW(71057);
					vstrTemp.Replace( L"#YEAR#", kSystime.wYear );
					vstrTemp.Replace( L"#MONTH#", BM::vstring(kSystime.wMonth, L"%02u") );
					vstrTemp.Replace( L"#DAY#", BM::vstring(kSystime.wDay, L"%02u") );
					vstrTemp.Replace( L"#HOUR#", BM::vstring(kSystime.wHour, L"%02u") );
					vstrTemp.Replace( L"#MIN#", BM::vstring(kSystime.wMinute, L"%02u") );
					vstrTemp += L" ~ ";
					vstrTemp += TTW(71056);
					vstrTemp += L" ";
					vstrTemp += TTW(71057);
					vstrTemp.Replace( L"#YEAR#", kSystime.wYear );
					vstrTemp.Replace( L"#MONTH#", BM::vstring(kEndSystime.wMonth, L"%02u") );
					vstrTemp.Replace( L"#DAY#", BM::vstring(kEndSystime.wDay, L"%02u") );
					vstrTemp.Replace( L"#HOUR#", BM::vstring(kEndSystime.wHour, L"%02u") );
					vstrTemp.Replace( L"#MIN#", BM::vstring(kEndSystime.wMinute, L"%02u") );
					pkControl->SetCustomData( static_cast<wchar_t const*>(vstrTemp), sizeof(wchar_t) * vstrTemp.size() );
				}
				else
				{
					vstrTemp += L" ";
					vstrTemp += TTW(71057);
					vstrTemp.Replace( L"#HOUR#", BM::vstring(kSystime.wHour, L"%02u") );
					vstrTemp.Replace( L"#MIN#", BM::vstring(kSystime.wMinute, L"%02u") );
					pkControlSub->Text( vstrTemp );

					vstrTemp = TTW(71056);
					vstrTemp += L" ";
					vstrTemp += TTW(71057);
					vstrTemp.Replace( L"#YEAR#", kSystime.wYear );
					vstrTemp.Replace( L"#MONTH#", BM::vstring(kSystime.wMonth, L"%02u") );
					vstrTemp.Replace( L"#DAY#", BM::vstring(kSystime.wDay, L"%02u") );
					vstrTemp.Replace( L"#HOUR#", BM::vstring(kSystime.wHour, L"%02u") );
					vstrTemp.Replace( L"#MIN#", BM::vstring(kSystime.wMinute, L"%02u") );
					pkControl->SetCustomData( static_cast<wchar_t const*>(vstrTemp), sizeof(wchar_t) * vstrTemp.size() );
				}
			}
			else
			{
				pkControlSub->Text( std::wstring() );
			}
		}
		
		++iIndex;
		vstrControlName = vstrPanBoxNameBase + iIndex;
		pkControl = pkWnd->GetControl( vstrControlName );
	}

	iIndex = 1;
	vstrControlName = vstrPanArrowNameBase + iIndex;
	pkControl = pkWnd->GetControl( vstrControlName );
	while ( pkControl )
	{
		pkControl->UVUpdate( ( 3 == kPanState[iIndex].iBoxUV ? 2 : 1 ) );

		++iIndex;
		vstrControlName = vstrPanArrowNameBase + iIndex;
		pkControl = pkWnd->GetControl( vstrControlName );
	}

	// Button
	pkButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl( L"BTN_JOIN" ));
	if ( pkButton )
	{// 도전 버튼
		pkButton->OwnerGuid(BM::GUID::NullData());
		switch ( m_iState )
		{
		case EMPORIA_TERM_OF_CHALLENGE:
			{
				pkButton->Text( TTW(71015) );
				bool const bDisable = !bAmIGuildOwner || (EMPORIA_KEY_NONE != g_kGuildMgr.GetEmporiaInfo().byType);
				pkButton->Disable( bDisable );
				pkButton->OwnerGuid(m_kEmporiaID);
			}break;
		case EMPORIA_CHALLENGE_OF_TOURNAMENT:
		case EMPORIA_OWNERSHIP_OF_TOURNAMENT:
			{
				pkButton->Text( TTW(74000) );
				bool const bDisable = ( (EMPORIA_KEY_BATTLE_ATTACK != g_kGuildMgr.GetEmporiaInfo().byType) || (EMPORIA_KEY_BATTLE_DEFENCE != g_kGuildMgr.GetEmporiaInfo().byType) );
				pkButton->Disable( bDisable );
			}break;
		default:
			{
				pkButton->Text( TTW(71015) );
				pkButton->Disable( true );
			}break;
		}
	}

	pkButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl( L"BTN_CHALLENGE_CANCEL"));
	if ( pkButton )
	{
		// 포기 버튼
		pkButton->Visible( false );
		if ( true == bAmIGuildOwner )
		{
			if ( EMPORIA_TERM_OF_CHALLENGE == m_iState )
			{
				if ( EMPORIA_KEY_BATTLERESERVE == g_kGuildMgr.GetEmporiaInfo().byType )
				{
					pkButton->Visible( true );
				}
			}
		}
	}
}

void PgEmporiaStateUI::SetBattleRewardSlot(XUI::CXUI_Wnd* pParent, int const iRank, TBL_DEF_EMPORIA_REWARD::SRank const& kReward, bool const bIsTournament)
{
	if( !pParent )
	{
		return;
	}

	BM::vstring kStr(bIsTournament ? L"ICN_REWARD" : L"ICN_EM_REWARD");
	kStr += iRank;
	XUI::CXUI_Wnd* pkTemp = pParent->GetControl(kStr);
	if( pkTemp )
	{
		pkTemp->SetCustomData(&kReward.iItem, sizeof(kReward.iItem));
	}				

	kStr = bIsTournament ? L"ICN_ACHIEVE" : L"ICN_EM_ACHIEVE";
	kStr += iRank;
	pkTemp = pParent->GetControl(kStr);
	if( pkTemp )
	{
		int const iAchieveSaveIdx = PgEmporia::GetRewardAchievementNo(iRank,bIsTournament);
		int iAchievement = 0;

		CONT_DEF_ACHIEVEMENTS_SAVEIDX const * pkContDef = NULL;
		g_kTblDataMgr.GetContDef(pkContDef);
		if( pkContDef )
		{
			CONT_DEF_ACHIEVEMENTS_SAVEIDX::const_iterator c_iter = pkContDef->find(iAchieveSaveIdx);
			if( c_iter!=pkContDef->end() )
			{ 
				iAchievement = c_iter->second.iItemNo;
			}
		}

		pkTemp->SetCustomData(&iAchievement, sizeof(iAchievement));
	}				
}

void PgEmporiaStateUI::HelpReawardUpdate( XUI::CXUI_Wnd* pTap )
{
	if( !pTap )
	{
		return;
	}

	CONT_DEF_EMPORIA const* pDefEmporia = NULL;
	g_kTblDataMgr.GetContDef(pDefEmporia);
	if( pDefEmporia )
	{
		CONT_DEF_EMPORIA::const_iterator emp_itor = pDefEmporia->find(m_kEmporiaID);
		if( emp_itor != pDefEmporia->end() )
		{
			CONT_DEF_EMPORIA::mapped_type const& kEmporiaInfo = emp_itor->second;

			for(int i = 0; i < TBL_DEF_EMPORIA_REWARD::ECG_END; ++i)
			{
				SetBattleRewardSlot(pTap, i, kEmporiaInfo.kReward.kChallenge[i], true);
			}
			for(int i = 0; i < MAX_EMPORIA_GRADE; ++i)
			{
				SetBattleRewardSlot(pTap, i, kEmporiaInfo.kReward.kGrade[i]);
			}
		}
	}
}

bool PgEmporiaStateUI::IsNowJoinMercenary()const
{
	PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkMyPlayer ){ return false; }

	PgLimitClass kLimitClass(pkMyPlayer->GetAbil(AT_CLASS), pkMyPlayer->GetAbil(AT_LEVEL));
	float fRemainTime = 0.f;
	return m_kTournament.ProcessJoinMercenary(pkMyPlayer->GetID(), pkMyPlayer->GetGuildGuid(), kLimitClass, fRemainTime);
}

bool PgEmporiaStateUI::ProcessJoinMercenary()const
{	//길드전 시작할 때 용병의 참여 유도창을 띄우기 위한 용도

	//길드전에 참여하지 않았어야함
	PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkMyPlayer ){ return false; }

	PgLimitClass kLimitClass(pkMyPlayer->GetAbil(AT_CLASS), pkMyPlayer->GetAbil(AT_LEVEL));
	float fRemainTime = 0.f;
	if( m_kTournament.ProcessJoinMercenary(pkMyPlayer->GetID(), pkMyPlayer->GetGuildGuid(), kLimitClass, fRemainTime) )
	{
		if(ADMT_MINI == g_kEmporiaMgr.GetViewADMercenaryType())
		{
			char szEventScript[MAX_PATH] = {0, };
			sprintf_s(szEventScript, MAX_PATH, "CallMercenaryJoinNotice(GUID('%s'))", MB(m_kEmporiaID.str()));

			STimerEvent kTimerEvent;
			kTimerEvent.Set(fRemainTime, szEventScript);
			g_kEventTimer.AddLocal(EVENT_EM_AD_MERCENARY, kTimerEvent);
		}
		return true;
	}

	return false;
}

std::wstring const PgEmporiaMgr::ms_wstrEmporiaUIWnd(L"FRM_GUILDWAR");

//PgEmporiaMgr
PgEmporiaMgr::PgEmporiaMgr()
	: m_eViewADMercenaryType(ADMT_NONE)
{

}

PgEmporiaMgr::~PgEmporiaMgr()
{

}

void PgEmporiaMgr::Clear()
{
	m_kEmporiaFunction.Clear();
}

bool PgEmporiaMgr::ProcessPacket( BM::Stream::DEF_STREAM_TYPE const usType, BM::Stream &rkPacket )
{
	switch ( usType )
	{
	case PT_N_C_ANS_EMPORIA_STATUS_LIST:
		{
			if ( rkPacket.RemainSize() > 0 )
			{
				EEmporiaStatusListType eType = ESLT_NONE;
				rkPacket.Pop(eType);
				m_kUIInfo.ReadFromPacket( rkPacket );
				
				if(ESLT_MERCENARY==eType)
				{
					m_kUIInfo.ProcessJoinMercenary();
				}
				else
				{
					if(ADMT_WINDOW == GetViewADMercenaryType())
					{
						SetViewADMercenaryType(ADMT_NONE);

						if( !m_kUIInfo.ProcessJoinMercenary() )
						{
							XUIMgr.Close( ms_wstrEmporiaUIWnd );
							lwAddWarnDataTT(70089);
						}
					}
					g_kEmporiaMgr.RefreshUI( true, NULL );
					g_kEmporiaMgr.RefreshUI( false, NULL );
				}
			}

			lwGuild::lwSendGuildInvMoney();
		}break;
	case PT_M_C_NFY_EMPORIA_FUNCTION:
		{
			CONT_EMPORIA_FUNC kContFunc;
			m_kEmporiaFunction.GetFunctionCont( kContFunc );

			m_kEmporiaFunction.ReadFromPacket( rkPacket );

			if ( g_pkWorld )
			{
				bool const bOwner = IsIamGroundOwner();
				CONT_EMPORIA_FUNC::const_iterator func_itr = m_kEmporiaFunction.BeginFunc();
				for ( ; func_itr!=m_kEmporiaFunction.EndFunc() ; ++func_itr )
				{
					g_pkWorld->BuildingControl( func_itr->first, func_itr->second, bOwner, false );
					kContFunc.erase( func_itr->first );
				}

				func_itr = kContFunc.begin();
				for ( ; func_itr != kContFunc.end() ; ++func_itr )
				{
					g_pkWorld->BuildingControl( func_itr->first, func_itr->second, false, false );
				}
			}
		}break;
	case PT_M_C_NFY_EMPORIA_FUNCTION_REMOVE:
		{
			CONT_EMPORIA_FUNCKEY kContKey;
			rkPacket.Pop( kContKey );

			CONT_EMPORIA_FUNCKEY::const_iterator itr = kContKey.begin();
			for ( ; itr!=kContKey.end() ; ++itr )
			{
				m_kEmporiaFunction.RemoveFunction( *itr );

				if ( g_pkWorld )
				{
					SEmporiaFunction kFunc;
					g_pkWorld->BuildingControl( *itr, kFunc, false, true );
				}
			}
		}break;	
// 	case PT_M_C_ANS_EMPORIA_ADMINISTRATION:
// 		{
// 			Recv_PT_M_C_ANS_EMPORIA_ADMINISTRATION( rkPacket );
// 		}break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

void PgEmporiaMgr::ADMercenary( BM::Stream &rkPacket )
{
	PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkMyPlayer ){ return; }
	if( EMPORIA_KEY_NONE!=g_kGuildMgr.GetEmporiaInfo().byType ){ return; }

	BM::GUID kEmporiaID;
	PgLimitClass kLimit;
	rkPacket.Pop( kEmporiaID );
	rkPacket.Pop( kLimit );

	PgLimitClass kMyLimit(pkMyPlayer->GetAbil(AT_CLASS),pkMyPlayer->GetAbil(AT_LEVEL));
	if( kLimit.IsAccess(kMyLimit) )
	{
		g_kEmporiaMgr.SetViewADMercenaryType(ADMT_MINI);

		EEmporiaStatusListType eType = ESLT_MERCENARY;
		BM::Stream kPacket( PT_C_N_REQ_EMPORIA_STATUS_LIST, kEmporiaID );
		kPacket.Push( eType );
		NETWORK_SEND( kPacket );
	}
}

void PgEmporiaMgr::RefreshThrowButton( BYTE const byEmpriaStatus )
{
	XUI::CXUI_Wnd *pkWnd = GetWnd( false );
	if ( pkWnd )
	{
		XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl( _T("BTN_THROW") ));
		if ( pkBtn )
		{
			pkBtn->Disable( EMPORIA_KEY_BATTLERESERVE!=byEmpriaStatus );
		}
	}
}

XUI::CXUI_Wnd* PgEmporiaMgr::GetWnd( bool bActivate )const
{
	if ( bActivate )
	{
		return XUIMgr.Activate( ms_wstrEmporiaUIWnd );
	}
	else
	{
		return XUIMgr.Get( ms_wstrEmporiaUIWnd );
	}
}

void PgEmporiaMgr::SetResult( EEmporiaAdminstrationRet const eRet )
{
	switch (eRet)
	{
	case EAR_SUEECSS:
		{
			Notice_Show( TTW(72016), EL_Normal );
		}break;
	case EAR_DBERROR:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 72018, true);
		}break;
	case EAR_ONLY_MASTER:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", PgGuildMgrUtil::iMsgOnlyMaster, true);
		}break;
	case EAR_NOT_ENOUGH_GUILDEXP:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 72017, true);
		}break;
	case EAR_CRITICAL_ERROR:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 72019, true);
		}break;
	}
}

void PgEmporiaMgr::RefreshUI( bool const bIsInfo, XUI::CXUI_Wnd *pkControl )
{
	if ( NULL == pkControl )
	{
		XUI::CXUI_Wnd * pkWnd = GetWnd(false);
		if( !pkWnd )
		{
			return;
		}

		if ( true == bIsInfo )
		{
			pkControl = pkWnd;
			XUI::CXUI_Wnd* pkTap = pkWnd->GetControl(L"FRM_EMPORIA_HELP");
			if( pkTap )
			{
				pkTap = pkTap->GetControl(L"FRM_HELP_CONTENTS_4");
				if( pkTap )
				{
					m_kUIInfo.HelpReawardUpdate( pkTap );
				}
			}
		}
		else
		{
			int iMode;
			if ( !pkWnd->GetCustomData( &iMode, sizeof(iMode) ) )
			{
				return;
			}

			if ( iMode != PgEmporiaStateUI::EMUI_STATE_EMPORIA_STATUSBOARD )
			{
				return;
			}

			pkControl = pkWnd->GetControl( L"FRM_STATUSBOARD");
		}
	}

	m_kUIInfo.Update( bIsInfo, pkControl );
}


__int64 PgEmporiaMgr::GetChallengeEndTime()const
{
	return m_kUIInfo.GetBattleTime() - PgEmporiaTournament::EMPORIA_BATTLE_CHALLENGE_ENDTIME_BEFORE_BATTLETIME;
}

bool PgEmporiaMgr::IsNowJoinMercenary()const
{
	return m_kUIInfo.IsNowJoinMercenary();
}

void PgEmporiaMgr::SetViewADMercenaryType(EmporiaViewADMercenaryType eType)
{
	m_eViewADMercenaryType = eType;
}

EmporiaViewADMercenaryType PgEmporiaMgr::GetViewADMercenaryType()const
{
	return m_eViewADMercenaryType;
}

int PgEmporiaMgr::GetEmporiaChallengeLimitCost()const
{
	CONT_DEF_EMPORIA const* pDefEmporia = NULL;
	g_kTblDataMgr.GetContDef(pDefEmporia);
	if( pDefEmporia )
	{
		CONT_DEF_EMPORIA::const_iterator emp_itor = pDefEmporia->find(m_kUIInfo.GetEmporiaID());
		if( emp_itor != pDefEmporia->end() )
		{
			CONT_DEF_EMPORIA::mapped_type const& kEmporiaInfo = emp_itor->second;
			return kEmporiaInfo.iLimitCost;
		}
	}
	return 0;
}