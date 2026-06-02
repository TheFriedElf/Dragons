#include "stdafx.h"
#include "PgNetwork.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgChatMgrClient.h"
#include "PgMarkBalloon.h"
#include "PgGuild.h"
#include "PgEmporiaBattleClient.h"
#include "PgMoveAnimateWnd.h"
#include "lwGuildUI.h"
#include "PgMobileSuit.h"

int const DEF_BGBOX_SIZE = 20;
int const DEF_CORE_INTERVAL = 3;
int const MAX_KILL_EFT_FORM = 10;
int const MAX_USER_INFO_ITEM = 10;
int const MAX_LOC_COUNT = 10;

std::wstring const EM_BATTLE_LANK_IMG_PATH = std::wstring(_T("../Data/6_UI/mission/msRkNum.tga"));

bool lwIsFullEmporiaBattleArea(int const iIndex)
{
	if( PgContentsBase::ms_pkContents )
	{
		return PgContentsBase::ms_pkContents->IsFullArea(iIndex);
	}

	return false;
}

void lwRandomMoveBattleArea()
{
	PgEmporiaBattleClient* pkEmporiaContents = dynamic_cast<PgEmporiaBattleClient*>(PgContentsBase::ms_pkContents);
	if( pkEmporiaContents )
	{
		pkEmporiaContents->RandomMoveBattleArea();
	}
}


PgEmporiaBattleClient::PgEmporiaBattleClient()
:	m_iMyTeam(TEAM_NONE)
,	m_iUserCount_MaxInTeam(0)
,	m_dwReBirthTime(0)
,	m_bTimeOver(false)
,	m_iWinTeam(TEAM_NONE)
,	m_bCoreDestory(false)
{
}

PgEmporiaBattleClient::~PgEmporiaBattleClient()
{
	Clear();
}

void PgEmporiaBattleClient::Clear()
{
	PgContentsBase::Clear();
	m_kDefenceGuild.Clear();
	m_kAttackGuild.Clear();
	m_kContUser.clear();
	m_iMyTeam = TEAM_NONE;
	m_iUserCount_MaxInTeam = 0;
	m_dwReBirthTime = 0;
	m_bTimeOver = false;
	m_iWinTeam = TEAM_NONE;
	m_bCoreDestory = false;

	g_kMarkBalloonMan.RemoveType( T_MARK_BATTLE_ENEMY|T_MARK_BATTLE_MVP );
}

bool PgEmporiaBattleClient::CallResponse()
{
	if ( STATUS_PLAYING != GetStatus() )
	{
		return false;
	}

	DWORD const dwCurServerElapsedTime = g_kEventView.GetServerElapsedTime();
	DWORD const dwRemainReBirthTime = 10000 - ::DifftimeGetTime( m_dwReBirthTime, dwCurServerElapsedTime ) % 10000;
	DWORD const dwRemainTimeSec = dwRemainReBirthTime / 1000;

	lua_tinker::call< void, DWORD, EPVPTYPE >("EMRebirthCall", dwRemainTimeSec, this->GetType() );
	return true;
}

void PgEmporiaBattleClient::CallLocationJoinUI(bool const bTabGame)
{
	XUI::CXUI_Wnd* pkMainUI = NULL;
	if( false==bTabGame )
	{
		XUIMgr.Close( L"FRM_EMBATTLE_REBIRTHTIME" );
		pkMainUI = XUIMgr.Call(L"FRM_SELECT_LOCATION");
	}
	else
	{
		pkMainUI = XUIMgr.Call(L"FRM_SELECT_LOCATION_TAB");
	}

	if( !pkMainUI )
	{
		return;
	}

	XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkMainUI->GetControl(L"BLD_ITEM"));
	if( !pkBuild )
	{
		return;
	}

	int const MAX_SLOT = pkBuild->CountX() * pkBuild->CountY() + pkBuild->StartIndex();

	for( int i = pkBuild->StartIndex(); i < MAX_SLOT; ++i )
	{
		BM::vstring vStr(L"FRM_ITEM");
		vStr += i;

		XUI::CXUI_Wnd* pkItem = pkMainUI->GetControl(vStr);
		if( pkItem )
		{
			int iCountATK = 0;
			int iCountDEF = 0;
			CONT_BATTLEAREA_INFO::const_iterator btl_itr = m_kContBattleAreaInfo.find(i);
			if( btl_itr != m_kContBattleAreaInfo.end() )
			{
				iCountATK = btl_itr->second.kContAtkUser.size();
				iCountDEF = btl_itr->second.kContDefUser.size();
			}


			XUI::CXUI_Wnd* pkTemp = pkItem->GetControl(L"FRM_ATK");
			if( pkTemp )
			{
				BM::vstring vStr(iCountATK);
				vStr += L"/";
				vStr += MAX_LOC_COUNT;
				pkTemp->Text(vStr);
			}
			pkTemp = pkItem->GetControl(L"FRM_DEF");
			if( pkTemp )
			{
				BM::vstring vStr(iCountDEF);
				vStr += L"/";
				vStr += MAX_LOC_COUNT;
				pkTemp->Text(vStr);
			}
			pkTemp = pkItem->GetControl(L"BTN_JOIN");
			if( pkTemp )
			{
				BM::vstring vStr(TTW(71564));
				vStr.Replace(L"#NUM#", i);
				pkTemp->Text(vStr);
			}
		}
	}
}

bool PgEmporiaBattleClient::IsFullArea(int const iIndex)const
{
	CONT_BATTLEAREA_INFO::const_iterator btl_itr = m_kContBattleAreaInfo.find(iIndex);
	if( btl_itr == m_kContBattleAreaInfo.end() )
	{
		return false;
	}

	int const iCountATK = btl_itr->second.kContAtkUser.size();
	int const iCountDEF = btl_itr->second.kContDefUser.size();
	int const iCount = (TEAM_ATTACKER==m_iMyTeam) ? iCountATK : iCountDEF;

	return iCount >= MAX_LOC_COUNT;
}

void PgEmporiaBattleClient::RandomMoveBattleArea()const
{
	typedef std::set<int> CONT_EXCEPT_AREA;
	typedef std::map<int, int> CONT_AREASORT;//지역인원수,지역위치

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"FRM_SELECT_LOCATION");

	int MAX_SLOT = 0;
	if( pkWnd )
	{
		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_ITEM"));
		if( pkBuild )
		{
			MAX_SLOT = pkBuild->CountX() * pkBuild->CountY();
		}
	}

	// 유저가 많은 순으로 지역 정렬시키기, 유저수가 중복되면 랜덤으로 지역이 선택됨
	CONT_BATTLEAREA_INFO::const_iterator btl_itr = m_kContBattleAreaInfo.begin();
	CONT_EXCEPT_AREA kContExceptArea;
	CONT_AREASORT kContSort;
	while( btl_itr != m_kContBattleAreaInfo.end() )
	{
		CONT_BATTLEAREA_INFO::mapped_type const& rkContUser = (*btl_itr).second;
		int const iUserCount = (TEAM_ATTACKER==m_iMyTeam) ? rkContUser.kContAtkUser.size() : rkContUser.kContDefUser.size();
		int const iAreaIdx = (*btl_itr).first;
		if(0 != iAreaIdx)
		{
			std::pair<CONT_AREASORT::iterator, bool> ret = kContSort.insert(std::make_pair(iUserCount, iAreaIdx));
			if(!ret.second)
			{
				(*ret.first).second = BM::Rand_Index(2) ? (*ret.first).second : iAreaIdx;
			}
		}

		if(0==iAreaIdx || iUserCount>=MAX_LOC_COUNT)
		{
			kContExceptArea.insert(iAreaIdx);
		}

		++btl_itr;
	}

	bool bMove = false;	
	
	// 이동가능한 지역 중 유저가 가장 많은 지역으로 이동 시키기
	CONT_AREASORT::const_reverse_iterator sort_itr = kContSort.rbegin();
	while( sort_itr != kContSort.rend() )
	{
		int const iAreaIdx = (*sort_itr).second;
		if((*sort_itr).second < MAX_LOC_COUNT)
		{
			lua_tinker::call<void, int>("Net_Req_PT_C_M_REQ_SELECT_BATTLE_AREA", iAreaIdx);
			bMove = true;
			break;
		}

		++sort_itr;
	}

	// 이동가능 지역중 랜덤으로 이동시키기
	if( false==bMove )
	{
		VEC_INT kContUseMove;
		for(int i=1; i<=MAX_SLOT; ++i)
		{
			CONT_EXCEPT_AREA::const_iterator it = kContExceptArea.find(i);
			if(it==kContExceptArea.end())
			{
				kContUseMove.push_back(i);
			}
		}
		std::random_shuffle(kContUseMove.begin(), kContUseMove.end());
		
		if( false==kContUseMove.empty() )
		{
			lua_tinker::call<void, int>("Net_Req_PT_C_M_REQ_SELECT_BATTLE_AREA", kContUseMove[0]+1);
			bMove = true;
		}
	}

	// 강제로 첫번째 지역으로 이동
	if( false==bMove )
	{
		XUIMgr.Close( L"FRM_SELECT_LOCATION" );
		BM::Stream kPacket(PT_C_M_REQ_SELECT_BATTLE_AREA);
		kPacket.Push(static_cast<int>(1));
		NETWORK_SEND(kPacket)
	}
}

void PgEmporiaBattleClient::CountDown( DWORD const dwRemainTime )
{
	if ( SetStatus(STATUS_COUNTDOWN) )
	{
		XUIMgr.Close( ms_FRM_EMBATTLE_WAIT_STATUS );
		lua_tinker::call<void, DWORD>("CallPvPCountDown", dwRemainTime/1000);
	}
}

void PgEmporiaBattleClient::Start( BM::Stream& kPacket )
{
	m_kContUser.clear();
	m_kContObejctInfo.clear();
	m_bTimeOver = false;
	m_iWinTeam = TEAM_NONE;
	m_bCoreDestory = false;

	PU::TLoadTable_AM( kPacket, m_kContObejctInfo );
	PU::TLoadTable_AM( kPacket, m_kContUser );
	kPacket.Pop( m_dwReBirthTime );
	kPacket.Pop( m_iUserCount_MaxInTeam );
	this->ReadFromStartAddOnPacket( kPacket );

	SetMyTeam();
    RemoveNPC();

	if ( SetStatus(STATUS_PLAYING) )
	{
		XUIMgr.Close( ms_FRM_EMBATTLE_WAIT_STATUS );
		XUIMgr.Call(ms_FRM_PVP_START);
		lwOnCallCenterMiniMap( lwGUID(BM::GUID::NullData()) );

		this->StartUI();
		SyncPoint( m_kAttackGuild.nCharacterPoint, m_kAttackGuild.nCorePoint, m_kDefenceGuild.nCharacterPoint, m_kDefenceGuild.nCorePoint );
	}
}

void PgEmporiaBattleClient::RemoveNPC()const
{
    if( !g_pkWorld ){ return; }
    UNIT_PTR_ARRAY kAddUnitArray;
    if( g_kPilotMan.FindUnit(UT_NPC, kAddUnitArray) )
    {
        UNIT_PTR_ARRAY::const_iterator c_it = kAddUnitArray.begin();
        while(c_it != kAddUnitArray.end())
        {
            if( c_it->pkUnit )
            {
                PgActor* pkActor = g_kPilotMan.FindActor(c_it->pkUnit->GetID());
                if( pkActor )
                {
                    pkActor->SetHide(true);
                    pkActor->SetHideShadow(true);
					pkActor->SetHideMiniMap(true);
                }
            }
            ++c_it;
        }
    }
}

void PgEmporiaBattleClient::StartUI()
{
	XUI::CXUI_Wnd *pkControl = NULL;
	CONT_EMBATTLE_OBJECT_INFO_CLIENT::iterator info_itr = m_kContObejctInfo.begin();
	for ( ; info_itr != m_kContObejctInfo.end() ; ++info_itr )
	{
		XUI::CXUI_Wnd* pMainUI = GetCoreWnd( info_itr->second.iTeam, true );
		if( pMainUI )
		{
			BM::vstring vStr(L"FRM_CORE_HP");
			BM::vstring vEftStr(L"FRM_CORE_HP_RATE");

			pkControl = pMainUI->GetControl( vStr );
			if ( pkControl )
			{
				info_itr->second.wstrUI = pkControl->ID();
				info_itr->second.wstrEftUI = (std::wstring const&)vEftStr;

				pkControl->OwnerGuid( info_itr->first );
			}
		}
	}
}

bool PgEmporiaBattleClient::RecvHP( CUnit *pkUnit, int const iBeforeHP, int const iNewHP )
{
	int const iTeam = pkUnit->GetAbil(AT_TEAM);

	switch ( pkUnit->UnitType() )
	{
	case UT_OBJECT:
		{
			RefreshCoreHP( pkUnit->GetID(), iNewHP, pkUnit->GetAbil(AT_C_MAX_HP) );
		}break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

void PgEmporiaBattleClient::ReadFromStartAddOnPacket(BM::Stream &kPacket)
{
	BuildBattleAreaInfo();

	size_t iSize = 0;
	kPacket.Pop( iSize );

	while ( iSize-- )
	{
		int iIndex = 0;
		kPacket.Pop( iIndex );

		CONT_BATTLEAREA_INFO::iterator btl_itr = m_kContBattleAreaInfo.find( iIndex );
		size_t iSize2 = 0;
		kPacket.Pop( iSize2 );

		while ( iSize2-- )
		{
			BM::GUID kGuid;
			int iTeam = 0;
			kPacket.Pop( kGuid );
			kPacket.Pop( iTeam );

			if ( btl_itr != m_kContBattleAreaInfo.end() )
			{
				if ( TEAM_ATTACKER == iTeam )
				{
					btl_itr->second.kContAtkUser.insert( kGuid );
				}
				else if ( TEAM_DEFENCER == iTeam )
				{
					btl_itr->second.kContDefUser.insert( kGuid );
				}
			}
		}
	}

	if( WAR_TYPE_DESTROYCORE == GetType() )
	{
		kPacket.Pop(m_usATKAuthTotalPoint);
		SetMaxPoint( m_usATKAuthTotalPoint );
		BYTE byLeaderTeam = 0;
		kPacket.Pop(byLeaderTeam);
		if ( byLeaderTeam )
		{
			SetChangeLeaderTeam( static_cast<int>(byLeaderTeam) );
		}
	}
}

void PgEmporiaBattleClient::StartKillEffect(int const Team, WORD const Point, POINT2 const StartPos, bool const bIsCore)
{
	for( int i = 0; i < MAX_KILL_EFT_FORM; ++i)
	{
		BM::vstring vStr(_T("FMA_KILL_EFFECT"));
		vStr += i;
		
		PgMoveAnimateWnd* pWnd = dynamic_cast<PgMoveAnimateWnd*>(XUIMgr.Get(vStr));
		if( !pWnd )
		{
			pWnd = dynamic_cast<PgMoveAnimateWnd*>(XUIMgr.Call(_T("FMA_KILL_EFFECT"), false, vStr));
			if( !pWnd )
			{
				continue;
			}
		}
		else
		{
			if( !pWnd->IsClosed() )
			{
				continue;
			}
		}

		vStr = Point;
		int const NumberLen = vStr.size();
		XUI::CXUI_Wnd* pNum = NULL;
		pNum = pWnd->GetControl(_T("FRM_ONE"));
		if( NumberLen > 0 )
		{
			if( pNum )
			{
				pNum->UVUpdate((Point % 10) + 1);
				pNum->Location(15 * NumberLen, pNum->Location().y);
			}
		}
		else
		{
			pWnd->Close();
		}

		pNum = pWnd->GetControl(_T("FRM_TEN"));
		if( NumberLen > 1 )
		{
			if( pNum )
			{
				pNum->UVUpdate((Point / 10 % 10) + 1);
				pNum->Location(15 * (NumberLen - 1), pNum->Location().y);
			}
		}
		else
		{
			pNum->Visible(false);
		}

		pNum = pWnd->GetControl(_T("FRM_HUN"));
		if( NumberLen > 2 )
		{
			if( pNum )
			{
				pNum->UVUpdate((Point / 100 % 10) + 1);
				pNum->Location(15 * (NumberLen - 2), pNum->Location().y);
			}
		}
		else
		{
			pNum->Visible(false);
		}

		{
			XUI::CXUI_Wnd *pkUpBar = GetWnd( false );
			if ( pkUpBar )
			{
				BM::vstring vStr(_T("FRM_CORE_ATK_POS_"));
				vStr += Team;
				XUI::CXUI_Wnd *pkPointWnd = pkUpBar->GetControl( vStr );
				if ( pkPointWnd )
				{
					pWnd->EndPoint(NiPoint3(pkPointWnd->Location().x + pkPointWnd->Size().x * 0.5f, 
						pkPointWnd->Location().y + pkPointWnd->Size().y * 0.5f, pWnd->EndPoint().z));
				}
			}
			pWnd->StartPoint(NiPoint3(static_cast<float>(StartPos.x - (15 * NumberLen)), static_cast<float>(StartPos.y), pWnd->Location().z));
			pWnd->ModifyPoint1(NiPoint3(pWnd->EndPoint().x, pWnd->StartPoint().y, pWnd->ModifyPoint1().z));
			pWnd->StartTime(g_pkApp->GetAccumTime());
			pWnd->SetCustomData(&bIsCore, sizeof(bIsCore));
		}

		break;
	}
}

void PgEmporiaBattleClient::CloseKillEffect(bool const bIsCore, int const iPoint)
{
	SGuildBattleInfo* pkGuildInfo = NULL;
	switch( m_iMyTeam )
	{
	case TEAM_ATTACKER: { pkGuildInfo = &m_kAttackGuild; } break;
	case TEAM_DEFENCER: { pkGuildInfo = &m_kDefenceGuild; } break;
	default: { return; } break;
	}

	if( bIsCore )
	{
		pkGuildInfo->nCorePoint += static_cast<WORD>(iPoint);
		m_MyKillPoint.CORE_DESTROY -= static_cast<WORD>(iPoint);
	}
	else
	{
		pkGuildInfo->nCharacterPoint += static_cast<WORD>(iPoint);
		m_MyKillPoint.PC_KILL -= static_cast<WORD>(iPoint);
	}

	SyncPoint( m_kAttackGuild.nCharacterPoint, m_kAttackGuild.nCorePoint, m_kDefenceGuild.nCharacterPoint, m_kDefenceGuild.nCorePoint );
}

void PgEmporiaBattleClient::TabGame(bool bDisplay)
{
	if( !bDisplay )
	{
		XUIMgr.Close(ms_FRM_EMBATTLE_TABSTATE);
		return;
	}
	
	if(( STATUS_READY != m_eStatus )
	&& ( STATUS_PLAYING != m_eStatus ) )
	{
		return;
	}

	// Tab 키가 귓말 대상 변경과 겹쳐서 ChatBar에 포커스가 있으면
	// Tab키 눌렀을 때 나오는 스코어, 이동 UI를 띄우지 않음.
	XUI::CXUI_Wnd* pChatBar = XUIMgr.Get(L"ChatBar");
	if( NULL == pChatBar )
	{
		return;
	}

	XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pChatBar->GetControl(_T("EDT_CHAT")));
	if( NULL == pEdit )
	{
		return;
	}

	bool IsChatBarFocus = pEdit->IsFocus();
	if( true == IsChatBarFocus )
	{
		return;
	}

	XUI::CXUI_Wnd* pWnd = XUIMgr.Activate(ms_FRM_EMBATTLE_TABSTATE);
	if( pWnd )
	{
		XUI::CXUI_Wnd* pTitle = NULL;

		pTitle = pWnd->GetControl(L"FRM_TEAM_TEXT1");
		if( pTitle )
		{
			pTitle->Text(TTW( WAR_TYPE_DESTROYCORE==this->GetType() ? 70100 : 5993 ));
		}

		pTitle = pWnd->GetControl(L"FRM_TEAM_TEXT2");
		if( pTitle )
		{
			pTitle->Text(TTW( WAR_TYPE_DESTROYCORE==this->GetType() ? 70101 : 5994 ));
		}

		CONT_EMPORIA_BATTLE_RESULT	kATK, kDEF;
		int iCurrentHeight = 0;

		//유저 정보를 나누고
		DivideAndSortTeamUserInfo(kATK, kDEF);

		//유저 정보를 세팅하고
		bool IsATKExist = SetGuildUserInfo(pWnd, m_kAttackGuild, kATK, _T("ATK"), iCurrentHeight);
		bool IsDEFExist = SetGuildUserInfo(pWnd, m_kDefenceGuild, kDEF, _T("DEF"), iCurrentHeight);
		
		bool bIsExistMyInfo = (IsATKExist || IsDEFExist)?(true):(false);
		if( !bIsExistMyInfo )
		{//내정보는 없었다 따로 세팅하자
			CONT_EMPORIA_BATTLE_RESULT const* pMyCont = NULL;
			int InfoPosX = 0;
			switch( m_iMyTeam )
			{
			case TEAM_ATTACKER: { pMyCont = &kATK; InfoPosX = 14;  } break;
			case TEAM_DEFENCER: { pMyCont = &kDEF; InfoPosX = 379; } break;
			default: { return; } break;
			}

			int iLank = 0;
			CONT_EMPORIA_BATTLE_RESULT::const_iterator c_iter = pMyCont->begin();
			while( c_iter != pMyCont->end() )
			{//내정보 찾아서 뿌려준다
				++iLank;
				if( g_kPilotMan.IsMyPlayer(c_iter->kCharGuid) )
				{
					XUI::CXUI_Wnd* pTemp = pWnd->GetControl(_T("FRM_MY_INFO"));
					if( pTemp )
					{
						pTemp = pTemp->GetControl(_T("FRM_USER_INFO_LIST_ITEM"));
						if( pTemp )
						{
							bool bTemp = false;
							SetGuildUserItem(pTemp, (*c_iter), iLank, bTemp);
							pTemp->Location(InfoPosX, pTemp->Location().y);
						}
					}
					break;
				}
				++c_iter;
			}
		}

		//폼사이즈를 조정하자
		if( iCurrentHeight )
		{
			SetGuildUserInfoBG(pWnd, iCurrentHeight, _T("ATK"), bIsExistMyInfo);
			SetGuildUserInfoBG(pWnd, iCurrentHeight, _T("DEF"), bIsExistMyInfo);
		}

		//지역UI창
		XUI::CXUI_Wnd* pButtom= pWnd->GetControl(L"FRM_BG_BUTTOM_ATK");
		int iButtomLocY = pButtom ? pButtom->TotalLocation().y : 0;
		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		bool const bMyUnitAlive = pkPlayer ? pkPlayer->IsAlive() : false;
		bool const bPlay = STATUS_PLAYING==GetStatus();
		bool const IsNotOpenLocationUI = !XUIMgr.Get(L"FRM_SELECT_LOCATION");
		if(bMyUnitAlive && bPlay && IsNotOpenLocationUI)
		{
			CallLocationJoinUI(true);
		}

		XUI::CXUI_Wnd* kWndTab = XUIMgr.Get(L"FRM_SELECT_LOCATION_TAB");
		if( kWndTab )
		{
			volatile static const int SPACE = 15;
			POINT2 kLoc( kWndTab->Location().x, iButtomLocY+SPACE );
			kWndTab->Location(kLoc);
		}
	}
}

void PgEmporiaBattleClient::RefreshCoreHP( BM::GUID const &kCoreGuid )
{
	CONT_EMBATTLE_OBJECT_INFO_CLIENT::iterator itr = m_kContObejctInfo.find( kCoreGuid );
	if ( itr != m_kContObejctInfo.end() )
	{
		int iMaxHP = itr->second.iHP;
		int iNowHP = itr->second.iHP;

		PgPilot* pPilot = g_kPilotMan.FindPilot( kCoreGuid );
		if( pPilot )
		{
			iMaxHP = pPilot->GetAbil(AT_C_MAX_HP);
			iNowHP = pPilot->GetAbil(AT_HP);
		}
		RefreshCoreHP( itr->second, iNowHP, iMaxHP );
	}
}

void PgEmporiaBattleClient::UpdateGuildUserInfo(BM::GUID const& kKill, BM::GUID const& kDie, int const iAddPoint)
{
	CONT_EMPORIA_BATTLE_USER::iterator	iter = m_kContUser.find(kKill);
	if( iter != m_kContUser.end() )
	{
		iter->second.kResult.Kill( iAddPoint );
	}

	iter = m_kContUser.find(kDie);
	if( iter != m_kContUser.end() )
	{
		iter->second.kResult.Dead(0);
	}
}

void PgEmporiaBattleClient::DivideAndSortTeamUserInfo(CONT_EMPORIA_BATTLE_RESULT& kATK, CONT_EMPORIA_BATTLE_RESULT& kDEF)
{
	CONT_EMPORIA_BATTLE_RESULT* pMyTeam = NULL;
	CONT_EMPORIA_BATTLE_RESULT* pOtherTeam = NULL;

	switch( m_iMyTeam )
	{
	case TEAM_ATTACKER: { pMyTeam = &kATK; pOtherTeam = &kDEF; } break;
	case TEAM_DEFENCER: { pMyTeam = &kDEF; pOtherTeam = &kATK; } break;
	default: { return; } break;
	}

	CONT_EMPORIA_BATTLE_USER::const_iterator c_iter = m_kContUser.begin();
	while( c_iter != m_kContUser.end() )
	{
		if( c_iter->second.iTeam == m_iMyTeam )
		{
			pMyTeam->push_back(c_iter->second);	
		}
		else
		{
			pOtherTeam->push_back(c_iter->second);
		}

		++c_iter;
	}

	std::sort( kATK.begin(), kATK.end(), std::greater<CONT_EMPORIA_BATTLE_RESULT::value_type>() );
	std::sort( kDEF.begin(), kDEF.end(), std::greater<CONT_EMPORIA_BATTLE_RESULT::value_type>() );
}

void PgEmporiaBattleClient::SetGuildUserItem(XUI::CXUI_Wnd* pWnd, SGuildBattleUser const& kUser, int const iLank, bool& IsMe)
{
	if( !pWnd )	{ return; }

	//내꺼는 하이라이트
	XUI::CXUI_Wnd* pTemp = pWnd->GetControl(_T("FRM_HIGHLIGHT"));
	if( pTemp )
	{
		IsMe = true;
		if( g_kPilotMan.IsMyPlayer(kUser.kCharGuid) )
		{
			pTemp->Visible(true);
		}
		else
		{
			pTemp->Visible(false);
		}
	}

	//등수를 세팅하고
	pTemp = pWnd->GetControl(_T("FRM_LANK"));
	if( pTemp )
	{
		if( iLank > 3 )
		{//4위부턴 텍스트
			pTemp->DefaultImgName(_T(""));
			BM::vstring vLank(iLank);
			pTemp->Text(vLank);
		}
		else
		{//1,2,3등은 이미지
			pTemp->DefaultImgName(EM_BATTLE_LANK_IMG_PATH);
			SUVInfo	kInfo(3, 1, iLank);
			pTemp->UVInfo(kInfo);
		}
	}

	//직업을 설정하고
	pTemp = pWnd->GetControl(_T("FRM_CLASS"));
	if( pTemp )
	{
		lwSetMiniClassIconIndex(pTemp, kUser.iClass);
	}

	//이름, 킬, 데스, 포인트 정보를 세팅한다
	SetUIText(pWnd, _T("FRM_NAME"), kUser.wstrName);
	SetUIText(pWnd, _T("FRM_KILL"), kUser.kResult.usKillCount);
	SetUIText(pWnd, _T("FRM_DEATH"), kUser.kResult.usDieCount);
	SetUIText(pWnd, _T("FRM_POINT"), kUser.kResult.iPoint);
}

bool PgEmporiaBattleClient::SetGuildUserInfo(XUI::CXUI_Wnd* pWnd, SGuildBattleInfo const& kTeam, CONT_EMPORIA_BATTLE_RESULT const& kUser, std::wstring const& AddText, int& rTotalHeight)
{
	if( !pWnd )
	{
		return false;
	}
	
	//길드 이름 세팅
	std::wstring kText(_T("FRM_TEAM_NAME_"));
	kText += AddText;
	SetUIText(pWnd, kText, kTeam.wstrName);

	//길드 인원 세팅
	kText = _T("FRM_TEAM_JOIN_");
	kText += AddText;
	wchar_t	szTemp[MAX_PATH] = {0,};
	swprintf(szTemp, MAX_PATH - 1, _T("%u{C=0xFFF9F9EB/}/%u"), kUser.size(), m_iUserCount_MaxInTeam );
	SetUIText(pWnd, kText, szTemp);

	//길드 유저 세팅(최대 10명만 한다)
	kText = _T("LST_TEAM_USER_");
	kText += AddText;
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(kText));
	if( !pList )
	{
		return false;
	}

	int MaxItemCount = pList->GetTotalItemCount();
	int MaxItem = (kUser.size() < MAX_USER_INFO_ITEM)?(kUser.size()):(MAX_USER_INFO_ITEM);
	if( MaxItemCount < MaxItem )
	{//사람수보다 적으면 만들고
		for(int i = MaxItemCount; i < MaxItem; ++i)
		{
			pList->AddItem(_T(""));			
		}
	}
	else
	{
		if( MaxItemCount > MaxItem )
		{//사람수보다 많으면 지워
			for(int i = MaxItemCount; i > MaxItem; --i)
			{
				pList->DeleteItem(pList->FirstItem());
			}			
		}
	}
	
	//유저정보의 첫번째를 받아 놓고
	CONT_EMPORIA_BATTLE_RESULT::const_iterator c_iter = kUser.begin();
	int iLank = 0;
	bool bIsExistMyInfo = false;

	//리스트의 아이템을 얻는다
	XUI::SListItem* pkItem = pList->FirstItem();
	while( pkItem && pkItem->m_pWnd )
	{
		XUI::CXUI_Wnd* pUserWnd = pkItem->m_pWnd;

		if( c_iter != kUser.end() )
		{//유저정보가 있으면
			++iLank;
			SetGuildUserItem(pUserWnd, (*c_iter), iLank, bIsExistMyInfo);
			++c_iter;
		}

		//다음 유저를 세팅한다 아이템
		pkItem = pList->NextItem(pkItem);
	}

	int const iTotal_H = static_cast<int>(pList->GetTotalItemLen());
	pList->Size(POINT2(pList->Size().x, iTotal_H));
	if( rTotalHeight < iTotal_H )
	{
		rTotalHeight = iTotal_H;
	}

	return bIsExistMyInfo;
}

void PgEmporiaBattleClient::SetGuildUserInfoBG(XUI::CXUI_Wnd* pWnd, int const iMiddleHeight, std::wstring const& AddText, bool const bIsExist)
{
	std::wstring wStr(_T("FRM_BG_TOP_"));
	wStr += AddText;
	XUI::CXUI_Wnd* pTop = pWnd->GetControl(wStr);
	if( !pTop )
	{
		return;
	}

	wStr = _T("FRM_BG_MIDDLE_");
	wStr += AddText;
	XUI::CXUI_Wnd* pMiddle = pWnd->GetControl(wStr);
	if( pMiddle )
	{
		pMiddle->Size(pMiddle->Size().x, iMiddleHeight);
		pMiddle->ImgSize(POINT2(pMiddle->ImgSize().x, iMiddleHeight));
	}

	wStr = _T("FRM_BG_BUTTOM_");
	wStr += AddText;
	XUI::CXUI_Wnd* pButtom = pWnd->GetControl(wStr);
	if( pButtom )
	{
		int iMyAddonInfoHeight = 0;
		if( !bIsExist )
		{
			XUI::CXUI_Wnd* pMyInfo = pWnd->GetControl(_T("FRM_MY_INFO"));
			pMyInfo->Location(pMyInfo->Location().x, pTop->Size().y + iMiddleHeight);
			iMyAddonInfoHeight = pWnd->Size().y;
		}

		pButtom->Location(pButtom->Location().x, pTop->Size().y + iMiddleHeight + iMyAddonInfoHeight);
	}
}

void PgEmporiaBattleClient::BuildBattleAreaInfo(void)
{
	m_kContBattleAreaInfo.clear();

	CONT_EMBATTLE_OBJECT_INFO_CLIENT::iterator obj_itr = m_kContObejctInfo.begin();
	for( ; obj_itr != m_kContObejctInfo.end() ; ++obj_itr )
	{
		CONT_EMBATTLE_OBJECT_INFO_CLIENT::key_type const& kGuid = obj_itr->first;
		CONT_EMBATTLE_OBJECT_INFO_CLIENT::mapped_type& kObject = obj_itr->second;

		std::pair<CONT_BATTLEAREA_INFO::iterator, bool> Area_rst = m_kContBattleAreaInfo.insert( std::make_pair( kObject.iBattleAreaIndex, SBattleAreaInfo(kGuid, kObject) ) );
	}
}

void PgEmporiaBattleClient::SetCoreBalance( XUI::CXUI_Wnd *pkWnd )
{
	if ( pkWnd )
	{
		int const iAttackerCoreTeam = ( m_kDefenceGuild.GetPoint() > m_kAttackGuild.GetPoint() ? TEAM_DEFENCER : TEAM_ATTACKER );

		XUI::CXUI_Wnd* pkControl = pkWnd->GetControl( L"FRM_BALANCE_MSG" );
		if ( pkControl )
		{
			std::wstring wstrMsg;
			
			if ( TEAM_DEFENCER == iAttackerCoreTeam )
			{
				WstringFormat( wstrMsg, MAX_PATH, TTW(71058).c_str(), m_kDefenceGuild.wstrName.c_str() );
			}
			else
			{
				WstringFormat( wstrMsg, MAX_PATH, TTW(71058).c_str(), m_kAttackGuild.wstrName.c_str() );
			}

			DWORD const dwFontColor = GetTeamFontColor( iAttackerCoreTeam );
			pkControl->FontColor( dwFontColor );
			pkControl->Text( wstrMsg );
		}

		pkControl = pkWnd->GetControl( L"FRM_SUB_MSG" );
		if ( pkControl )
		{
			pkControl->Visible( iAttackerCoreTeam != m_iMyTeam );
		}
	}
}

bool PgEmporiaBattleClient::SyncPoint( WORD const nUserKillPoint_Atk, WORD const nCoreBreakPoint_Atk, WORD const nUserKillPoint_Def, WORD const nCoreBreakPoint_Def )
{
	WORD iPoint[2] = { nUserKillPoint_Atk + nCoreBreakPoint_Atk, nUserKillPoint_Def + nCoreBreakPoint_Def };

	XUI::CXUI_Wnd *pkWnd = XUIMgr.Get(ms_FRM_EMBATTLE_CORE_STATUS);
	SetCoreBalance( pkWnd );
	
	pkWnd = GetWnd( true );
	if ( pkWnd )
	{
		XUI::CXUI_Wnd* pkControl = NULL;
		XUI::CXUI_AniBar* pkAniControl = NULL;

		WORD const nUserKillPoint[2] = { nUserKillPoint_Atk, nUserKillPoint_Def };
		WORD const nCoreBreakPoint[2] = { nCoreBreakPoint_Atk, nCoreBreakPoint_Def };

		TCHAR szTemp[MAX_PATH] = {0,};
		for ( int i=TEAM_ATTACKER; i<TEAM_MAX; ++i )
		{
			int iIndex = i-1;

			_stprintf_s( szTemp, MAX_PATH, _T("FRM_ATK_%d"), i );
			pkAniControl = dynamic_cast<XUI::CXUI_AniBar*>(pkWnd->GetControl( szTemp ));
			if( pkAniControl )
			{
				pkAniControl->Now( iPoint[ iIndex ] );

				float const fRate = pkAniControl->Now() / static_cast<float>(pkAniControl->Max());

				_stprintf_s( szTemp, MAX_PATH, _T("FRM_CORE_ATK_RATE_%d"), i );
				XUI::CXUI_Wnd* pkRate = pkWnd->GetControl( szTemp );
				if( pkRate )
				{
					int const Rate = static_cast<int>(fRate * 100);
					pkRate->Text(BM::vstring(Rate));
				}

				_stprintf_s( szTemp, MAX_PATH, _T("FRM_CORE_ATK_POS_%d"), i );
				XUI::CXUI_Wnd* pkPos = pkWnd->GetControl( szTemp );
				if( pkPos )
				{
					float const fRealRate = (TEAM_ATTACKER != i)?(1.0f - fRate):(fRate);
					int iLocationX = pkAniControl->Location().x;
					iLocationX += static_cast<int>(pkAniControl->Size().x * fRealRate);
					iLocationX -= static_cast<int>(pkPos->Size().x * 0.5f);

					pkPos->Location(iLocationX, pkPos->Location().y);
				}
			}
		}
		return true;
	}
	return false;
}

void PgEmporiaBattleClient::SetMaxPoint( int const iPoint )
{
	XUI::CXUI_Wnd* pkWnd = GetWnd( true );
	if ( pkWnd )
	{
		XUI::CXUI_Wnd* pkControl = NULL;
		XUI::CXUI_AniBar* pkAniControl = NULL;

		TCHAR szTemp[MAX_PATH] = {0,};
		for ( int i=TEAM_ATTACKER; i<TEAM_MAX; ++i )
		{
			int iIndex = i-1;
			_stprintf_s( szTemp, MAX_PATH, _T("FRM_ATK_%d"), i );
			pkAniControl = dynamic_cast<XUI::CXUI_AniBar*>(pkWnd->GetControl( szTemp ));
			if( pkAniControl )
			{
				pkAniControl->Max( iPoint );
			}
		}
	}
}

void PgEmporiaBattleClient::DrawTime( size_t iRemainTimeSec, size_t iRemainTimeMS )
{
	XUI::CXUI_Wnd *pkWnd = GetWnd( false );
	if ( !pkWnd )
	{
		return;
	}

	pkWnd = pkWnd->GetControl( L"FRM_TIMER" );
	if ( pkWnd )
	{
		size_t iRemainTimeMin = iRemainTimeSec / 60;
		iRemainTimeSec %= 60;

		XUI::CXUI_Wnd *pkTemp = pkWnd->GetControl(_T("FRM_COUNT_MIN_100"));
		if ( pkTemp )
		{
			pkTemp->UVUpdate( iRemainTimeMin / 100 + 1 );
		}
		iRemainTimeMin %= 100;

		pkTemp = pkWnd->GetControl(_T("FRM_COUNT_MIN_10"));
		if ( pkTemp )
		{
			pkTemp->UVUpdate( iRemainTimeMin / 10 + 1 );
		}

		pkTemp = pkWnd->GetControl(_T("FRM_COUNT_MIN_01"));
		if ( pkTemp )
		{
			pkTemp->UVUpdate( iRemainTimeMin % 10 + 1 );
		}

		pkTemp = pkWnd->GetControl(_T("FRM_COUNT_SEC_10"));
		if ( pkTemp )
		{
			pkTemp->UVUpdate( iRemainTimeSec / 10 + 1 );
		}

		pkTemp = pkWnd->GetControl(_T("FRM_COUNT_SEC_01"));
		if ( pkTemp )
		{
			pkTemp->UVUpdate( iRemainTimeSec % 10 + 1 );
		}

		pkTemp = pkWnd->GetControl(_T("FRM_COUNT_MSEC_10"));
		if ( pkTemp )
		{
			pkTemp->UVUpdate( iRemainTimeMS / 10 + 1 );
		}

		pkTemp = pkWnd->GetControl(_T("FRM_COUNT_MSEC_01"));
		if ( pkTemp )
		{
			pkTemp->UVUpdate( iRemainTimeMS % 10 + 1 );
		}
	}
}

bool PgEmporiaBattleClient::Update( float const fAccumTime, float const fFrameTime )
{
	switch ( GetStatus() )
	{
	case STATUS_PLAYING:
		{
			size_t iRemainTimeSec = 0;
			size_t iRemainTimeMS = 0;
			GetRemainTime( iRemainTimeSec, &iRemainTimeMS );
			DrawTime( iRemainTimeSec, iRemainTimeMS/10 );
		}break;
	case STATUS_ENDBEFORE:
		{
			//SlowMotion Update
			lua_tinker::call<void>("Ending_Emporia_OnUpdate");
		}break;
	case STATUS_RESULT:
		{
			SetStatus(STATUS_NONE);

			//SlowMotion Cleanup
			lua_tinker::call<void>("Ending_Emporia_OnCleanUp");

			SGuildEmporiaInfo const &kEmporiaInfo = g_kGuildMgr.GetEmporiaInfo();
			switch ( kEmporiaInfo.byType )
			{
			case EMPORIA_KEY_NONE:
			case EMPORIA_KEY_BATTLERESERVE:
			case EMPORIA_KEY_BATTLECONFIRM:
				{
					Net_RecentMapMove();
				}break;
			case EMPORIA_KEY_MINE:
				{
					lwGuild::Req_JoinEmporia();
				}break;
			default:
				{
					// 문제가 있는데?
#ifndef EXTERNAL_RELEASE
					_PgMessageBox( "[ERROR]", "EmporiaInfo's Status[%u]", kEmporiaInfo.byType );
#endif
					Net_RecentMapMove();
				}break;
			}
			return false;
		}break;
	}
	return true;
}

bool PgEmporiaBattleClient::ProcessPacket( BM::Stream::DEF_STREAM_TYPE const wType, BM::Stream& kPacket )
{
	switch( wType )
	{
	case PT_M_C_NFY_GAME_WAIT:
		{
			SetStatus( STATUS_READY );
			RecvSyncTeamPoint( kPacket, true );

			__int64 i64BattleReadyTime = 0i64;
			__int64 i64BattleStartTime = 0i64;
			kPacket.Pop( i64BattleReadyTime );
			kPacket.Pop( i64BattleStartTime );

			XUI::CXUI_Wnd * pkWaitState = XUIMgr.Activate( ms_FRM_EMBATTLE_WAIT_STATUS );
			if ( pkWaitState )
			{
				XUI::CXUI_AniBar *pkAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pkWaitState->GetControl(L"BAR_TIMER"));
				if ( pkAniBar )
				{
					pkAniBar->SetCustomData( &i64BattleStartTime, sizeof(i64BattleStartTime) );

					__int64 const i64RemainTime = std::max<__int64>( i64BattleStartTime - i64BattleReadyTime, CGameTime::SECOND );
					pkAniBar->Max( static_cast<int>(i64RemainTime / CGameTime::SECOND) );
				}
			}

			for( int i = TEAM_ATTACKER; i <= TEAM_DEFENCER; ++i )
			{
				XUI::CXUI_Wnd* pkMainUI = GetCoreWnd( i, false );
				if( pkMainUI )
				{
					BM::vstring vStr(L"FRM_CORE_HP");
					XUI::CXUI_Wnd* pkAnibar = pkMainUI->GetControl(vStr);
					if( pkAnibar )
					{
						pkAnibar->OwnerGuid(BM::GUID::NullData());
					}
				}
			}
		}break;
	case PT_M_C_NFY_GAME_START:
		{
			RecvSyncRemainTime(kPacket);
			RecvSyncTeamPoint(kPacket);
			Start(kPacket);
		}break;
	case PT_M_C_NFY_GAME_INTRUDER:
		{
			Recv_PT_M_C_NFY_GAME_INTRUDER( kPacket );
		}break;
	case PT_M_C_NFY_GAME_EXIT:
		{
			Recv_PT_M_C_NFY_GAME_EXIT( kPacket );
		}break;
	case PT_M_C_NFY_GAME_EVENT_KILL:
		{
			Recv_PT_M_C_NFY_GAME_EVENT_KILL( kPacket );
		}break;
	case PT_M_C_NFY_ADD_POINT_USER:
		{
			Recv_PT_M_C_NFY_ADD_POINT_USER(kPacket);
		}break;
	case PT_M_C_NFY_GAME_END:
		{
			Recv_PT_M_C_NFY_GAME_END(kPacket);
		}break;
	case PT_M_C_NFY_GAME_RESULT_END:
		{
			Recv_PT_M_C_NFY_GAME_RESULT_END(kPacket);
		}break;
	case PT_M_C_NFY_BATTLE_AREA_USER:
		{
			Recv_PT_M_C_NFY_BATTLE_AREA_USER( kPacket );
		}break;
	default:
		{
			return PgContentsBase::ProcessPacket( wType, kPacket );
		}break;
	}
	return true;
}

void PgEmporiaBattleClient::Recv_PT_M_C_NFY_GAME_INTRUDER( BM::Stream &kPacket )
{
	BM::GUID kCharGuid;
	CONT_EMPORIA_BATTLE_USER::mapped_type kElement;
	kPacket.Pop( kCharGuid );
	kElement.ReadFromPacket( kPacket );
	m_kContUser.insert( std::make_pair( kCharGuid, kElement ) );
}

void PgEmporiaBattleClient::Recv_PT_M_C_NFY_GAME_EXIT( BM::Stream &kPacket )
{
	BM::GUID kCharGuid;
	kPacket.Pop( kCharGuid );
	m_kContUser.erase( kCharGuid );
}

void PgEmporiaBattleClient::SetChangeLeaderTeam( int const iLeaderTeam )
{
	//lua_tinker::call< void, int >("EMRebirthAtkColorChange", iLeaderTeam );

	XUI::CXUI_Wnd* pkWnd = GetWnd( true );
	if ( pkWnd )
	{
		for( int i = TEAM_ATTACKER; i < TEAM_MAX; ++i )
		{
			BM::vstring vStr(L"FRM_CORE_ATK_BG_");
			vStr += i;

			XUI::CXUI_Wnd* pkBg = pkWnd->GetControl( vStr );
			if( pkBg )
			{
				pkBg->Visible( iLeaderTeam == i );
			}
		}
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		int const kMyTeam = pkPlayer->GetAbil(AT_TEAM);
		if( kMyTeam == iLeaderTeam )
		{
			lwAddWarnDataTT(71568);
		}
		else
		{
			lwAddWarnDataTT(71567);
		}
	}
}

void PgEmporiaBattleClient::Recv_PT_M_C_NFY_ADD_POINT_USER( BM::Stream &kPacket )
{
	BM::GUID kCharGuid;
	WORD usAddPoint = 0;
	BYTE byLeaderTeam = 0;
	kPacket.Pop( kCharGuid );
	kPacket.Pop( usAddPoint );
	kPacket.Pop( byLeaderTeam );
	kPacket.Pop( m_kDefenceGuild.nCharacterPoint );
	kPacket.Pop( m_kAttackGuild.nCharacterPoint );

	CONT_EMPORIA_BATTLE_USER::iterator user_itr = m_kContUser.find(kCharGuid);
	if( user_itr != m_kContUser.end() )
	{
		user_itr->second.kResult.AddPoint( static_cast<int>(usAddPoint) );
	}

	if ( byLeaderTeam )
	{
		SetChangeLeaderTeam( static_cast<int>(byLeaderTeam) );
	}

	this->SyncPoint( m_kAttackGuild.nCharacterPoint, m_kAttackGuild.nCorePoint, m_kDefenceGuild.nCharacterPoint, m_kDefenceGuild.nCorePoint );
}

void PgEmporiaBattleClient::Recv_PT_M_C_NFY_BATTLE_AREA_USER( BM::Stream &kPacket )
{
	int iOldIndex = 0;
	int iNewIndex = 0;
	BM::GUID kGuid;
	int iTeam = 0;

	kPacket.Pop( iOldIndex );
	kPacket.Pop( iNewIndex );
	kPacket.Pop( kGuid );
	kPacket.Pop( iTeam );
	
	CONT_BATTLEAREA_INFO::iterator btl_itr = m_kContBattleAreaInfo.find( iOldIndex );
	if ( btl_itr != m_kContBattleAreaInfo.end() )
	{
		btl_itr->second.kContAtkUser.erase( kGuid );
		btl_itr->second.kContDefUser.erase( kGuid );
	}

	std::pair<CONT_BATTLEAREA_INFO::iterator, bool> Area_rst = m_kContBattleAreaInfo.insert( std::make_pair( iNewIndex, SBattleAreaInfo() ) );
	if ( TEAM_ATTACKER == iTeam )
	{
		Area_rst.first->second.kContAtkUser.insert( kGuid );
	}
	else if ( TEAM_DEFENCER == iTeam )
	{
		Area_rst.first->second.kContDefUser.insert( kGuid );
	}
}

void PgEmporiaBattleClient::Recv_PT_M_C_NFY_GAME_EVENT_KILL( BM::Stream &kPacket )
{
	BYTE byLeaderTeam = 0;
	kPacket.Pop( byLeaderTeam );
	kPacket.Pop( m_kDefenceGuild.nCharacterPoint );
	kPacket.Pop( m_kAttackGuild.nCharacterPoint );
	
	if ( byLeaderTeam )
	{
		SetChangeLeaderTeam( static_cast<int>(byLeaderTeam) );
	}

	BM::GUID kDieGuid;
	BM::GUID kKillGuid;
	int iAddPoint = 0;
	bool bDestroyCore = false;
	kPacket.Pop( kDieGuid );
	kPacket.Pop( kKillGuid );
	kPacket.Pop( iAddPoint );
	kPacket.Pop( bDestroyCore );

	UpdateGuildUserInfo(kKillGuid, kDieGuid, iAddPoint);

	bool bPointEffect = true;
	if ( bDestroyCore )
	{
		bPointEffect = ( false == this->DestroyCore( kDieGuid ) );
	}
	else
	{
		if ( g_kPilotMan.IsMyPlayer(kDieGuid) )
		{// 내가 죽은거면
			g_kMarkBalloonMan.AddMarkPoint( kKillGuid, T_MARK_BATTLE_ENEMY, 1 );
		}
		else if ( g_kPilotMan.IsMyPlayer(kKillGuid) )
		{// 내가 죽인거면
			g_kMarkBalloonMan.RemoveMarkPoint( kDieGuid, T_MARK_BATTLE_ENEMY );
		}
	}

	if ( true == bPointEffect )
	{
		this->SetPointEffect( kKillGuid, kDieGuid, iAddPoint, bDestroyCore );

		// 공격이냐 방어냐
		SGuildBattleInfo* pkGuildInfo = NULL;
		switch( m_iMyTeam )
		{
		case TEAM_ATTACKER: { pkGuildInfo = &m_kAttackGuild; } break;
		case TEAM_DEFENCER: { pkGuildInfo = &m_kDefenceGuild; } break;
		}

		if( pkGuildInfo )
		{
			// 포인트를 까는데
			pkGuildInfo->nCorePoint -= m_MyKillPoint.CORE_DESTROY;
			pkGuildInfo->nCharacterPoint -= m_MyKillPoint.PC_KILL;
		}

		this->SyncPoint( m_kAttackGuild.nCharacterPoint, m_kAttackGuild.nCorePoint, m_kDefenceGuild.nCharacterPoint, m_kDefenceGuild.nCorePoint );
	}
}

void PgEmporiaBattleClient::SetPointEffect( BM::GUID const &kKillerID, BM::GUID const &kDieID, short const usAddPoint, bool const bDestroyCore )
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
				CONT_EMPORIA_BATTLE_USER::const_iterator user_itr = m_kContUser.find( kKillerID );
				if ( user_itr != m_kContUser.end() )
				{
					pkTemp->FontColor( (IsKillerIsMine)?(0xFFFFE98E):(GetTeamFontColor(user_itr->second.iTeam)) );
					pkTemp->Text(user_itr->second.wstrName);
				}
				else
				{
					// 자살
					pkTemp->Text( _T("") );
				}
			}

			pkTemp = pkItem->m_pWnd->GetControl(_T("FRM_INJURE"));
			if( pkTemp )
			{
				if( bDestroyCore )
				{
					pkTemp->FontColor(0xFFBAFF00);
					pkTemp->Text(TTW(71036));
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

	if ( true == IsKillerIsMine )
	{
		if( true == bDestroyCore )
		{
			m_MyKillPoint.CORE_DESTROY += usAddPoint;
		}
		else
		{
			m_MyKillPoint.PC_KILL += usAddPoint;
		}

		if ( pkWnd )
		{
			StartKillEffect(m_iMyTeam, usAddPoint, FormPos, bDestroyCore);
		}
	}
}

void PgEmporiaBattleClient::Recv_PT_M_C_NFY_GAME_END( BM::Stream &kPacket )
{
	DrawTime( 0, 0 );
	SetStatus(STATUS_ENDBEFORE);
	lwCloseAllUI(true);

	kPacket.Pop( m_bTimeOver );
	kPacket.Pop( m_iWinTeam );
	kPacket.Pop( m_bCoreDestory );

	m_kContUser.clear();
	PU::TLoadTable_AM( kPacket, m_kContUser );

	ShowResult();

	//SlowMotion Enter
	lua_tinker::call<void>("Ending_Emporia_OnEnter");
}

void PgEmporiaBattleClient::ShowResult()
{
	CONT_EMPORIA_BATTLE_RESULT kContBattleResult_Attack;
	CONT_EMPORIA_BATTLE_RESULT kContBattleResult_Defence;
	kContBattleResult_Attack.reserve( m_kContUser.size() );
	kContBattleResult_Defence.reserve( m_kContUser.size() );
	
	CONT_EMPORIA_BATTLE_USER::const_iterator user_itr = m_kContUser.begin();
	for ( ; user_itr!=m_kContUser.end() ; ++user_itr )
	{
		if ( user_itr->second.iTeam == TEAM_ATTACKER )
		{
			kContBattleResult_Attack.push_back( user_itr->second );
		}
		else
		{
			kContBattleResult_Defence.push_back( user_itr->second );
		}
	}

	std::sort( kContBattleResult_Attack.begin(), kContBattleResult_Attack.end(), std::greater<CONT_EMPORIA_BATTLE_RESULT::value_type>() );
	std::sort( kContBattleResult_Defence.begin(), kContBattleResult_Defence.end(), std::greater<CONT_EMPORIA_BATTLE_RESULT::value_type>() );

	BM::GUID kMVPGuid;
	if ( m_iWinTeam == TEAM_ATTACKER )
	{
		if ( kContBattleResult_Attack.size() )
		{
			kMVPGuid = kContBattleResult_Attack.begin()->kCharGuid;
		}
	}
	else
	{
		if ( kContBattleResult_Defence.size() )
		{
			kMVPGuid = kContBattleResult_Defence.begin()->kCharGuid;
		}
	}

	g_kMarkBalloonMan.SetType( T_MARK_BATTLE_MVP );

	if ( BM::GUID::IsNotNull(kMVPGuid) )
	{
		g_kMarkBalloonMan.AddMarkPoint( kMVPGuid, T_MARK_BATTLE_MVP, 1 );
	}

	XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( ms_FRM_EMBATTLE_RESULT_MYINFO );
	if ( pkWnd )
	{
		PgActor*	pMyActor = g_kPilotMan.GetPlayerActor();
		if( pMyActor )
		{
			CONT_EMPORIA_BATTLE_USER::const_iterator my_iter = m_kContUser.find(pMyActor->GetGuid());
			if( my_iter != m_kContUser.end() )
			{
				int iRank = 0;
				if( my_iter->second.iTeam == TEAM_ATTACKER )
				{
					CONT_EMPORIA_BATTLE_RESULT::iterator	iter = kContBattleResult_Attack.begin();
					while( iter != kContBattleResult_Attack.end() )
					{
						++iRank;
						if( iter->kCharGuid == my_iter->second.kCharGuid )
						{
							break;
						}
						++iter;
					}
				}
				else
				{
					CONT_EMPORIA_BATTLE_RESULT::iterator	iter = kContBattleResult_Defence.begin();
					while( iter != kContBattleResult_Defence.end() )
					{
						++iRank;
						if( iter->kCharGuid == my_iter->second.kCharGuid )
						{
							break;
						}
						++iter;
					}
				}

				XUI::CXUI_Wnd *pkTemp = pkWnd->GetControl(_T("FRM_NAME"));
				if( pkTemp )
				{
					pkTemp->Text(my_iter->second.wstrName);
				}

				wchar_t wszTemp[MAX_PATH] = {0,};
				pkTemp = pkWnd->GetControl(_T("FRM_POINT01"));
				if( pkTemp )
				{
					::swprintf_s( wszTemp, MAX_PATH, L"%u", my_iter->second.kResult.usKillCount );
					pkTemp->Text(wszTemp);
				}

				pkTemp = pkWnd->GetControl(_T("FRM_POINT02"));
				if( pkTemp )
				{
					::swprintf_s( wszTemp, MAX_PATH, L"%u", my_iter->second.kResult.usDieCount );
					pkTemp->Text(wszTemp);
				}

				pkTemp = pkWnd->GetControl(_T("FRM_POINT03"));
				if( pkTemp )
				{
					::swprintf_s( wszTemp, MAX_PATH, L"%d", my_iter->second.kResult.iPoint );
					pkTemp->Text(wszTemp);
				}

				pkTemp = pkWnd->GetControl(_T("FRM_NUMBER"));
				if( pkTemp )
				{
					XUI::CXUI_Wnd* pkSubTemp = pkTemp->GetControl(_T("FRM_TEN"));
					if( pkSubTemp )
					{
						SUVInfo	Info = pkSubTemp->UVInfo();
						Info.Index = (iRank / 10) + 1;
						if( Info.Index > 1 )
						{
							pkSubTemp->UVInfo(Info);
							pkSubTemp->Visible(true);
						}
						else
						{
							pkSubTemp->Visible(false);
						}
					}

					pkSubTemp = pkTemp->GetControl(_T("FRM_ONE"));
					if( pkSubTemp )
					{
						SUVInfo	Info = pkSubTemp->UVInfo();
						Info.Index = (iRank % 10) + 1;
						pkSubTemp->UVInfo(Info);
					}
				}
			}
		}
	}

	pkWnd = XUIMgr.Activate( ms_FRM_EMBATTLE_RESULT );
	if ( pkWnd )
	{
		XUI::CXUI_Wnd *pkTemp = NULL;

		pkTemp = pkWnd->GetControl(L"FRM_ATTACK_TITLE");
		if( pkTemp )
		{
			pkTemp->Text(TTW( WAR_TYPE_DESTROYCORE==this->GetType() ? 70100 : 202 ));
		}

		pkTemp = pkWnd->GetControl(L"FRM_DEFENCE_TITLE");
		if( pkTemp )
		{
			pkTemp->Text(TTW( WAR_TYPE_DESTROYCORE==this->GetType() ? 70101 : 203 ));
		}

		pkTemp = pkWnd->GetControl( _T("FRM_ATTACK_NAME") );
		if ( pkTemp )
		{
			pkTemp->Text( m_kAttackGuild.wstrName );
		}

		pkTemp = pkWnd->GetControl( _T("FRM_DEFENCE_NAME") );
		if ( pkTemp )
		{
			pkTemp->Text( m_kDefenceGuild.wstrName );
		}

		wchar_t wszTemp[MAX_PATH] = {0,};
		size_t iIndex = 0;
		XUI::CXUI_Wnd *pkControl = NULL;
		CONT_EMPORIA_BATTLE_USER::mapped_type *pkElement = NULL;
		for ( int i=1; i<21; ++i )
		{
			pkElement = NULL;

			::swprintf_s( wszTemp, MAX_PATH, L"FRM_RET_%d", i );
			pkControl = pkWnd->GetControl(wszTemp);
			if ( pkControl )
			{
				iIndex = (i-1) / 2;
				if ( (i % 2) == TEAM_ATTACKER )
				{
					if ( kContBattleResult_Attack.size() > iIndex )
					{
						pkElement = &(kContBattleResult_Attack.at(iIndex));
					}
				}
				else
				{
					if ( kContBattleResult_Defence.size() > iIndex )
					{
						pkElement = &(kContBattleResult_Defence.at(iIndex));
					}
				}

				if ( pkElement )
				{
					pkControl->Visible( true );
					pkTemp = pkControl->GetControl(_T("IMG_CLASS"));
					if ( pkTemp )
					{// 직업 아이콘 설정
						lwSetMiniClassIconIndex( pkTemp, pkElement->iClass);
					}

					pkTemp = pkControl->GetControl(_T("FRM_NAME"));
					if ( pkTemp )
					{
						pkTemp->Text(pkElement->wstrName);
					}

					pkTemp = pkControl->GetControl(_T("FRM_KILL"));
					if ( pkTemp )
					{
						::swprintf_s( wszTemp, MAX_PATH, L"%u", pkElement->kResult.usKillCount );
						pkTemp->Text( wszTemp );
					}

					pkTemp = pkControl->GetControl(_T("FRM_DEATH"));
					if ( pkTemp )
					{
						::swprintf_s( wszTemp, MAX_PATH, L"%u", pkElement->kResult.usDieCount );
						pkTemp->Text( wszTemp );
					}

					pkTemp = pkControl->GetControl(_T("FRM_POINT"));
					if ( pkTemp )
					{
						::swprintf_s( wszTemp, MAX_PATH, L"%d", pkElement->kResult.iPoint );
						pkTemp->Text( wszTemp );
					}
				}
				else
				{
					pkControl->Visible( false );
				}
			}
		}
	}

	pkWnd = NULL;
	if ( m_bTimeOver )
	{
		pkWnd = XUIMgr.Activate( ms_FRM_EMBATTLE_TIMEOVER );
	}
	else
	{
		if( WAR_TYPE_DESTROYCORE==this->GetType() )
		{
			pkWnd = XUIMgr.Activate( ms_FRM_EMBATTLE_COREDESTROY );
		}
		else
		{
			pkWnd = XUIMgr.Activate( ms_FRM_EMBATTLE_PROTECTDRAGON );
		}
	}

	if ( pkWnd )
	{
		int i = (m_iWinTeam == m_iMyTeam) ? 1 : 0;
		pkWnd->SetCustomData( &i, sizeof(int) );
	}
}

void PgEmporiaBattleClient::Recv_PT_M_C_NFY_GAME_RESULT_END( BM::Stream &kPacket )
{
	SetStatus(STATUS_RESULT);
}

void PgEmporiaBattleClient::RecvSyncTeamPoint( BM::Stream& kPacket, bool const bReady )
{
	g_kMarkBalloonMan.SetType( T_MARK_BATTLE_ENEMY );

	m_kDefenceGuild.ReadFromPacket( kPacket );
	m_kAttackGuild.ReadFromPacket( kPacket );

	XUI::CXUI_Wnd *pkWnd = GetWnd( true );
	if ( pkWnd )
	{
		XUI::CXUI_Wnd *pkTemp = pkWnd->GetControl(_T("FRM_GUILD_NAME_ATTACK"));
		if ( pkTemp )
		{
			pkTemp->Text( m_kAttackGuild.wstrName );
		}

		pkTemp = pkWnd->GetControl(_T("FRM_GUILD_MARK_ATTACK"));
		if( pkTemp )
		{
			pkTemp->OwnerGuid( m_kAttackGuild.kGuildGuid );
		}

		pkTemp = pkWnd->GetControl( _T("FRM_GUILD_NAME_DEFENCE") );
		if ( pkTemp )
		{
			pkTemp->Text( m_kDefenceGuild.wstrName );
		}

		pkTemp = pkWnd->GetControl(_T("FRM_GUILD_MARK_DEFENCE"));
		if( pkTemp )
		{
			pkTemp->OwnerGuid( m_kDefenceGuild.kGuildGuid );
		}

		pkTemp = pkWnd->GetControl( _T("FRM_TIMER") );
		if ( pkTemp )
		{
			pkTemp->Visible( !bReady );
		}
	}

	SyncPoint( m_kAttackGuild.nCharacterPoint, m_kAttackGuild.nCorePoint, m_kDefenceGuild.nCharacterPoint, m_kDefenceGuild.nCorePoint );
}

void PgEmporiaBattleClient::SetVisibleSubCoreBG(XUI::CXUI_Wnd* pkWnd, int const iTeam, int iNum, bool const bIsView)
{
	if( !pkWnd ){ return; }

	BM::vstring vStr, vEftStr;
	switch( iTeam )
	{
	case TEAM_ATTACKER:	{ vStr = _T("FRM_ATK_SUB_BG"); vEftStr = _T("FMA_ATK_COREHEAL"); }break;
	case TEAM_DEFENCER:	{ vStr = _T("FRM_DEF_SUB_BG"); vEftStr = _T("FMA_DEF_COREHEAL");  }break;
	}
	vStr += iNum;
	vEftStr += iNum;

	XUI::CXUI_Wnd* pkBg = pkWnd->GetControl(vStr);
	if( !pkBg ){ return; }
	pkBg->Visible(bIsView);

	PgMoveAnimateWnd* pkEft = dynamic_cast<PgMoveAnimateWnd*>(pkWnd->GetControl(vEftStr));
	if( !pkEft ){ return; }
	pkEft->Visible(false);
	{
		NiPoint3 EftStartPos( static_cast<float>(pkBg->Location().x) + (pkBg->Size().x * 0.5f),
							  static_cast<float>(pkBg->Location().y) + (pkBg->Size().y * 0.5f),
							  static_cast<float>(pkEft->Location().z) );
		pkEft->StartPoint(EftStartPos);
		pkEft->Location( static_cast<int>(pkEft->StartPoint().x + pkEft->Size().x * 0.5f),
						 static_cast<int>(pkEft->StartPoint().y + pkEft->Size().y * 0.5f),
						 static_cast<int>(pkEft->StartPoint().z) );
		
		NiPoint3 ModifyPos( pkEft->StartPoint().x + (pkEft->EndPoint().x - pkEft->StartPoint().x) * 0.5f, 80.0f, 0.0f );
		pkEft->ModifyPoint1(ModifyPos);
	}
	
	if( bIsView )
	{
		int const iSizeX = DEF_BGBOX_SIZE + (DEF_CORE_INTERVAL * iNum) + ((iNum + 1) * pkBg->Size().x);

		std::wstring wBGBox;
		std::wstring wBGBoxSide;
		std::wstring wBGBoxEft;
		switch( iTeam )
		{
		case TEAM_ATTACKER:	{ wBGBox = _T("IMG_ATK_BG_G"); wBGBoxSide = _T("IMG_ATK_BG_C"); }break;
		case TEAM_DEFENCER:	{ wBGBox = _T("IMG_DEF_BG_G"); wBGBoxSide = _T("IMG_DEF_BG_C"); }break;
		}

		XUI::CXUI_Wnd* pBGBox = pkWnd->GetControl(wBGBoxSide);
		if( pBGBox )
		{
			pBGBox->ImgSize(POINT2(iSizeX, pBGBox->ImgSize().y));
			pBGBox->Size(pBGBox->ImgSize());
			pBGBox->Visible(true);
			switch( iTeam )
			{
			case TEAM_ATTACKER:
				{
					pBGBox->Location(pkBg->Location().x, pBGBox->Location().y);
				}break;
			case TEAM_DEFENCER:	
				{
					int const iX = pkBg->Location().x + pkBg->Size().x - iSizeX;
					pBGBox->Location(iX, pBGBox->Location().y);
				}break;
			}
		}

		pBGBox = pkWnd->GetControl(wBGBox);
		if( pBGBox )
		{
			POINT2	PtPos(pBGBox->Location().x, pBGBox->Location().y);
			switch( iTeam )
			{
			case TEAM_ATTACKER:
				{
					PtPos.x = pkBg->Location().x - pBGBox->Size().x;
				}break;
			case TEAM_DEFENCER:	
				{ 
					PtPos.x = pkBg->Location().x + pkBg->Size().x;
				}break;
			}
			pBGBox->Location(PtPos);
			pBGBox->Visible(true);
		}
	}
}

// 리턴값은 메인코어
bool PgEmporiaBattleClient::DestroyCore( BM::GUID const &kCoreGuid )
{
	bool bMainCore = false;

	std::wstring wstrMsg;
	std::wstring kChildFormName;
	CONT_EMBATTLE_OBJECT_INFO_CLIENT::iterator itr = m_kContObejctInfo.find( kCoreGuid );
	if ( itr != m_kContObejctInfo.end() )
	{
		kChildFormName = itr->second.wstrUI;
		bMainCore = ( EOGRADE_MAINCORE == itr->second.byGrade );
		if ( !bMainCore )
		{
			if ( TEAM_ATTACKER == itr->second.iTeam )
			{
				::WstringFormat( wstrMsg, MAX_PATH, TTW(72022).c_str(), m_kAttackGuild.wstrName.c_str() );
			}
			else
			{
				::WstringFormat( wstrMsg, MAX_PATH, TTW(72022).c_str(), m_kDefenceGuild.wstrName.c_str() );
			}
			
		}

		bMainCore = ( EOGRADE_MAINCORE == itr->second.byGrade );
		itr->second.iHP = 0;
	}

	XUI::CXUI_Wnd *pkWnd = XUIMgr.Get( ms_FRM_EMBATTLE_CORE_STATUS );
	if ( pkWnd )
	{
		pkWnd = pkWnd->GetControl( kChildFormName );
		if ( pkWnd )
		{
			pkWnd->ClearCustomData();
			pkWnd->OwnerGuid( BM::GUID::NullData() );
			pkWnd->Visible( false );
		}
	}

	if ( !wstrMsg.empty() )
	{
		Notice_Show( wstrMsg, EL_PvPMsg );
	}

	return bMainCore;
}

// iMaxHP가 0이면 iValueHP 만큼 더해준다.
void PgEmporiaBattleClient::RefreshCoreHP( BM::GUID const &kCoreGuid, int const iValueHP, int const iMaxHP )
{
	CONT_EMBATTLE_OBJECT_INFO_CLIENT::iterator itr = m_kContObejctInfo.find( kCoreGuid );
	if ( itr != m_kContObejctInfo.end() )
	{
		RefreshCoreHP( itr->second, iValueHP, iMaxHP );
	}
}

void PgEmporiaBattleClient::RefreshCoreHP( CONT_EMBATTLE_OBJECT_INFO_CLIENT::mapped_type &kCoreInfo, int const iValueHP, int const iMaxHP )
{
	XUI::CXUI_Wnd *pkWnd = GetCoreWnd( kCoreInfo.iTeam, true );
	if ( pkWnd )
	{
		XUI::CXUI_AniBar* pHPBar = dynamic_cast<XUI::CXUI_AniBar*>(pkWnd->GetControl( kCoreInfo.wstrUI ));
		if ( pHPBar )
		{
			if( iMaxHP != pHPBar->Max() )
			{
				pHPBar->Max( iMaxHP );
			}

			kCoreInfo.iHP = iValueHP;
			pHPBar->Now( iValueHP );
			//pHPBar->Text( BM::vstring(iValueHP) );

			pkWnd = pkWnd->GetControl(kCoreInfo.wstrEftUI);
			if( pkWnd )
			{
				float const fRate = iValueHP / static_cast<float>(pHPBar->Max());
				BM::vstring vStr(static_cast<int>(fRate * 100));
				vStr += L"%";
				pkWnd->Text(vStr);

				int iLocationY = pHPBar->Location().y;								//기본 피바 높이
				iLocationY += static_cast<int>(pHPBar->Size().y * (1.0f - fRate));	// + 피가 단 높이
				iLocationY -= static_cast<int>(pkWnd->Size().y * 0.5f);				// 중앙 정렬

				pkWnd->Location( pkWnd->Location().x, iLocationY );
			}
		}
	}
}

DWORD PgEmporiaBattleClient::GetTeamFontColor( int const iTeam )
{
	switch ( iTeam )
	{
	case TEAM_ATTACKER:{return 0xFFFF5842;}break;
	case TEAM_DEFENCER:{return 0xFF3EA5FF;}break;
	}
	return 0xFF000000;
}

void PgEmporiaBattleClient::SetMyTeam()
{
	if ( g_kGuildMgr.GuildGuid() == m_kAttackGuild.kGuildGuid )
	{
		m_iMyTeam = TEAM_ATTACKER;
	}
	else if ( g_kGuildMgr.GuildGuid() == m_kDefenceGuild.kGuildGuid )
	{
		m_iMyTeam = TEAM_DEFENCER;
	}
	else
	{
		PgActor * pMyActor = g_kPilotMan.GetPlayerActor();
		if( !pMyActor ){ return; }
		
		CONT_EMPORIA_BATTLE_USER::iterator iter = m_kContUser.find(pMyActor->GetGuid());
		if( iter==m_kContUser.end() ){ return; }

		m_iMyTeam = iter->second.iTeam;
	}
}
