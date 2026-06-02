#include "stdafx.h"
#include "Variant/PgHardCoreDungeonParty.h"
#include "PgNetwork.h"
#include "PgPilotMan.h"
#include "lwGuid.h"
#include "PgChatMgrClient.h"
#include "PgClientParty.H"
#include "HardCoreDungeon.h"

std::wstring const PgHardCoreDungeon::ms_SFRM_HCD_VOTE = std::wstring(L"SFRM_HCD_VOTE");

typedef bool (*BOOL_FUN_IN_INT)(int const);
void PgHardCoreDungeon::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def<BOOL_FUN_IN_INT>(pkState, "Net_JoinHardCoreDungeon", &lwNet_JoinHardCoreDungeon );
}

bool PgHardCoreDungeon::lwNet_JoinHardCoreDungeon( int const iType )
{
	PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if ( pkMyPlayer )
	{
		if(		BM::GUID::IsNotNull(pkMyPlayer->PartyGuid()) 
			&&	g_kParty.MasterGuid() != pkMyPlayer->GetID() )
		{
			// 파티장 또는 개인만 입장 할 수 있습니다.
			Notice_Show( TTW(2), EL_Warning );
			return false;
		}

		PgInventory *pkInv = pkMyPlayer->GetInven();

		ContHaveItemNoCount	kItemCont;
		if( SUCCEEDED(pkInv->GetItems( UICT_HARDCORE_KEY, kItemCont, true )) )
		{	
			ContHaveItemNoCount::const_iterator itemno_itr = kItemCont.begin();
			for ( ; itemno_itr != kItemCont.end() ; ++itemno_itr )
			{
				SItemPos kItemPos;
				if ( SUCCEEDED(pkInv->GetFirstItem( itemno_itr->first, kItemPos, true, true )) )
				{
					BM::Stream kPacket( PT_C_M_REQ_JOIN_LOBBY, HardCoreDungeon );
					kPacket.Push( kItemPos );
					NETWORK_SEND( kPacket );
					return true;
				}
			}
		}

		// 입장권이 있어야 입장 하실 수 있습니다.
		Notice_Show( TTW(402004), EL_Warning );
	}
	
	return false;
}

void PgHardCoreDungeon::Recv_PT_M_C_ANS_REGIST_HARDCORE_VOTE( BM::Stream &rkPacket )
{
	PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if ( pkMyPlayer )
	{
		XUIMgr.Close( L"FRM_PVP_SELECTOR" );
		XUI::CXUI_Wnd *pkUIMain = XUIMgr.Activate( ms_SFRM_HCD_VOTE );
		if ( pkUIMain )
		{
			PgPartyContents_HardCoreDungeon kContents;
			kContents.ReadFromPacket( rkPacket );

			struct SSetUI
			{
				explicit SSetUI( XUI::CXUI_Wnd *pkUIMain, PgPlayer *_pkMyPlayer )
					:	pkUIWnd(pkUIMain)
					,	iIndex(1)
					,	iMyIndex(0)
					,	pkMyPlayer(_pkMyPlayer)
					,	bVoteEnd(false)
				{}

				bool operator()( BM::GUID const &kCharGuid, PgPartyContents_HardCoreDungeon::SMemberState const &kElement )
				{
					BM::vstring vstr(L"SFRM_VOTE_");
					vstr += iIndex++;

					XUI::CXUI_Wnd * pkUI = pkUIWnd->GetControl( static_cast<std::wstring>(vstr) );
					if ( pkUI )
					{
						pkUI->OwnerGuid( kCharGuid );
						pkUI->SetCustomData( &kElement.byState, sizeof(kElement.byState) );

						bool const bIsMyUI = ( kCharGuid == pkMyPlayer->GetID() );
						
						if ( true == bIsMyUI )
						{
							pkUI->Text( pkMyPlayer->Name() );
							pkUI->FontColor( 0xFFFFF568);
							iMyIndex = iIndex - 1;
						}
						else
						{
							SPartyMember const * pkPartyMember = NULL;
							if ( true == g_kParty.GetMember( kCharGuid, pkPartyMember ) )
							{
								pkUI->Text( pkPartyMember->kName );
							}
							else
							{
								pkUI->Text( std::wstring() );
							}

							pkUI->FontColor( 0xFFFFFFFF );
						}

						XUI::CXUI_Wnd *pkControl = pkUI->GetControl( L"IMG_OK" );
						if ( pkControl )
						{
							if ( kElement.byState )
							{
								pkControl->UVUpdate( ((E_HCT_V_OK == kElement.byState) ? 3 : 4) );
								pkControl->Enable(false);
								if ( true == bIsMyUI )
								{
									bVoteEnd = true;
								}
							}
							else
							{
								pkControl->UVUpdate( (bIsMyUI ? 1 : 4) );
								pkControl->Enable(bIsMyUI);
							}
						}

						pkControl = pkUI->GetControl( L"IMG_CANCEL" );
						if ( pkControl )
						{
							if ( kElement.byState )
							{
								pkControl->UVUpdate( ((E_HCT_V_OK < kElement.byState) ? 3 : 4) );
								pkControl->Enable(false);
								if ( true == bIsMyUI )
								{
									bVoteEnd = true;
								}
							}
							else
							{
								pkControl->UVUpdate( (bIsMyUI ? 1 : 4) );
								pkControl->Enable(bIsMyUI);
							}
						}
					}
					return true;
				}

				XUI::CXUI_Wnd	*pkUIWnd;
				size_t			iIndex;
				size_t			iMyIndex;
				PgPlayer		*pkMyPlayer;
				bool			bVoteEnd;
			};

			SSetUI kSetUIFn( pkUIMain, pkMyPlayer );
			kContents.ForEach_Member( kSetUIFn );

			BM::vstring vstr( L"SFRM_VOTE_" );
			vstr += kSetUIFn.iIndex++;
			XUI::CXUI_Wnd *pkUI = pkUIMain->GetControl( static_cast<std::wstring>(vstr) );
			while ( pkUI )
			{
				SetEmptyUI( pkUI );

				BM::vstring vstr( L"SFRM_VOTE_" );
				vstr += kSetUIFn.iIndex++;
				pkUI = pkUIMain->GetControl( static_cast<std::wstring>(vstr) );
			}

			bool const bIsEnable = ( BM::GUID::IsNull(pkMyPlayer->PartyGuid()) || (g_kParty.MasterGuid() == pkMyPlayer->GetID()));
			if ( !bIsEnable )
			{
				pkUIMain->SetCustomData( &kSetUIFn.iMyIndex, sizeof(kSetUIFn.iMyIndex) );
			}

			pkUI = pkUIMain->GetControl( L"BTN_OK" );
			if ( pkUI )
			{
				pkUI->Enable( bIsEnable || !kSetUIFn.bVoteEnd );
			}

			pkUI = pkUIMain->GetControl( L"BTN_CANCEL" );
			if ( pkUI )
			{
				pkUI->Enable( bIsEnable );
			}
		}
	}
}

void PgHardCoreDungeon::SetEmptyUI( XUI::CXUI_Wnd *pkUI )
{
	pkUI->Text( std::wstring() );
	pkUI->ClearCustomData();

	XUI::CXUI_Wnd *pkControl = pkUI->GetControl( L"IMG_OK" );
	if ( pkControl )
	{
		pkControl->UVUpdate(4);
		pkControl->Enable(false);
	}

	pkControl = pkUI->GetControl( L"IMG_CANCEL" );
	if ( pkControl )
	{
		pkControl->UVUpdate(4);
		pkControl->Enable(false);
	}
}

void PgHardCoreDungeon::RemoveMember( BM::GUID const &kCharGuid )
{
	XUI::CXUI_Wnd *pkUIMain = XUIMgr.Get( PgHardCoreDungeon::ms_SFRM_HCD_VOTE );
	if ( pkUIMain )
	{
		size_t iIndex = 1;
		bool bFind = false; 

		BM::vstring vstr(L"SFRM_VOTE_");
		vstr += iIndex++;
		XUI::CXUI_Wnd *pkUI = pkUIMain->GetControl( static_cast<std::wstring>(vstr) );
		while ( pkUI )
		{
			if ( pkUI->OwnerGuid() == kCharGuid )
			{
				SetEmptyUI( pkUI );
				break;
			}

			BM::vstring vstr(L"SFRM_VOTE_");
			vstr += iIndex++;
			pkUI = pkUIMain->GetControl( static_cast<std::wstring>(vstr) );
		}
	}
}

bool PgHardCoreDungeon::UpdateRemainTime( __int64 const i64EndTime )
{
	__int64 i64RemainTime = (i64EndTime  - g_kEventView.GetLocalSecTime( CGameTime::DEFAULT )) / CGameTime::SECOND;
	if ( 0i64 < i64RemainTime )
	{
		XUI::CXUI_Wnd * pkUI = XUIMgr.Activate( L"FRM_HCD_ING" );
		if ( pkUI )
		{
			int iRemainTime = static_cast<int>(i64RemainTime) + 1;//1을보정
			if ( 0 > iRemainTime )
			{
				iRemainTime = INT_MAX;
			}
			pkUI->SetCustomData( &iRemainTime, sizeof(iRemainTime) );
		}
		return true;
	}
	return false;
}

void PgHardCoreDungeon::UpdateBossGndInfo( BM::GUID const &kPartyGuid, __int64 const i64EndTime )
{
	XUI::CXUI_Wnd *pkUI = XUIMgr.Activate( L"FRM_HCD_BOSS_ING" );
	if ( pkUI )
	{
		pkUI->OwnerGuid( kPartyGuid );

		if ( 0i64 < i64EndTime )
		{
			XUI::CXUI_Wnd *pkControl = pkUI->GetControl(  L"FRM_DUMMY" );
			if ( pkControl )
			{
				__int64 const i64RemainTime = (i64EndTime  - g_kEventView.GetLocalSecTime( CGameTime::DEFAULT )) / CGameTime::SECOND;
				int iRemainTime = static_cast<int>(i64RemainTime) + 1;//1을보정
				if ( 0 > iRemainTime )
				{
					iRemainTime = INT_MAX;
				}
				pkControl->SetCustomData( &iRemainTime, sizeof(iRemainTime) );
			}

			
			pkControl = XUIMgr.Activate( L"FRM_HCD_EXIT" );
			if ( pkControl )
			{
				POINT3I pt3Loc = pkUI->Location();
				pt3Loc.y += ( pkUI->Height() + 5 );
				pkControl->Location( pt3Loc );
			}
		}
		else
		{
			XUI::CXUI_Wnd *pkControl = XUIMgr.Activate( L"FRM_HCD_EXIT" );
			if ( pkControl )
			{
				POINT3I pt3Loc = pkUI->Location();
				pt3Loc.y += 5;
				pkControl->Location( pt3Loc );
			}
			pkUI->Close();
		}
	}
}

bool PgHardCoreDungeon::IsVote()
{
	XUI::CXUI_Wnd *pkWnd = NULL;
	return XUIMgr.IsActivate( ms_SFRM_HCD_VOTE, pkWnd );
}
