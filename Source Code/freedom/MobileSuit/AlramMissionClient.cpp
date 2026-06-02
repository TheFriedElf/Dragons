#include "stdafx.h"
#include "Variant/AlramMission.h"
#include "ServerLib.h"
#include "PgPilotMan.h"
#include "lwUI.h"
#include "AlramMissionClient.h"

using namespace ALRAM_MISSION;

bool PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_BEGIN >::operator()( BM::Stream &kPacket )
{
	int iID;
	DWORD dwBeginTime;
	if (	true == kPacket.Pop( iID) 
		&&	true == kPacket.Pop( dwBeginTime )
		)
	{
		PgPlayer * pkPlayer = g_kPilotMan.GetPlayerUnit();
		if ( !pkPlayer )
		{
			return false;
		}

		// 이전 미션은 종료 처리
		PgAlramMission & rkAlramMission = pkPlayer->GetAlramMission();
		if ( iID != rkAlramMission.GetNextID() )
		{
			PgAlramMissionClient<PT_M_C_NFY_ALRAMMISSION_END>::Call( rkAlramMission );
		}

		CONT_DEF_ALRAM_MISSION const *pkContDefAlramMission = NULL;
		g_kTblDataMgr.GetContDef( pkContDefAlramMission );

		CONT_DEF_ALRAM_MISSION::const_iterator itr = pkContDefAlramMission->find( iID );
		if ( itr != pkContDefAlramMission->end() )
		{
			XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( L"FRM_ALRAM_MISSION" );
			if ( !pkWnd )
			{
				return false;
			}

			XUI::CXUI_Wnd *pkControl = NULL;

			TBL_DEF_ALRAM_MISSION const &kElement = itr->second;
			if ( RET_SUCCESS == rkAlramMission.Begin( &kElement, dwBeginTime ) )
			{
				std::wstring wstrTitle;
				std::wstring const* pkTitle = NULL;
				if ( true == GetDefString( kElement.iTitleNo, pkTitle) )
				{
					wstrTitle = *pkTitle;
				}
				else
				{
					BM::vstring vstrTemp(L"UnknownDefStr#");
					vstrTemp += kElement.iTitleNo;
					wstrTitle = static_cast<std::wstring>(vstrTemp);
				}

				if ( kPacket.RemainSize() )
				{
					PgAlramMission::VEC_PARAMS kParamList;
					PU::TLoadArray_A( kPacket, kParamList );

					size_t index = 0;
					for ( ; index < kParamList.size() ; ++index )
					{
						rkAlramMission.SetParam( index, kParamList.at(index) );
					}
				}
				else
				{
					BM::vstring vstr( TTW(120104) );			
					vstr.Replace( L"#TITLE#", wstrTitle );
					Notice_Show( vstr, EL_Notice1 );
				}

				// 남은 시간을 계산
				DWORD const dwCurTime = g_kEventView.GetServerElapsedTime();
				DWORD const dwEndTime = dwBeginTime + (static_cast<DWORD>(kElement.iTime) * 1000) + PgAlramMission::ms_dwDelayTime;
				DWORD  dwRemainTime = 0;
				if ( dwEndTime > dwCurTime )
				{
					dwRemainTime = dwEndTime - dwCurTime;
				}

				pkControl = pkWnd->GetControl( L"FRM_TIMER" );
				if ( pkControl )
				{
					lua_tinker::call< void, lwUIWnd, DWORD >("SetUITimer", lwUIWnd(pkControl), dwRemainTime / 1000 );
				}

				pkControl = pkWnd->GetControl( L"FRM_TITLE" );
				if ( pkControl )
				{
					pkControl->Text( wstrTitle );
				}

				if ( kElement.kActionList.size() )
				{
					pkControl = pkWnd->GetControl( L"FRM_TOTAL" );
					if ( pkControl )
					{
						int iCount = 0;
						rkAlramMission.GetParam( 0, iCount );
						BM::vstring vstr( TTW(120101) );
						vstr.Replace( L"#COUNT#", iCount );
						vstr.Replace( L"#TOTAL#", kElement.kActionList.at(0).iCount );
						pkControl->Text( vstr );
					}
				}

				pkControl = pkWnd->GetControl( L"FRM_POINT" );
				if ( pkControl )
				{
					if ( kElement.nPoint )
					{
						pkControl->Visible( true );

						BM::vstring vstr( TTW(120103) );
						vstr.Replace( L"#VALUE#", kElement.nPoint );
						pkControl->Text( vstr );
					}
					else
					{
						pkControl->Visible( false );
					}
				}

				pkControl = pkWnd->GetControl( L"FRM_EXP" );
				if ( pkControl )
				{
					if ( kElement.iExp )
					{
						pkControl->Visible( true );

						BM::vstring vstr( TTW(120102) );
						vstr.Replace( L"#VALUE#", kElement.iExp );
						pkControl->Text( vstr );
					}
					else
					{
						pkControl->Visible( false );
					}
				}
			}
			return true;
		}
	}
	return false;
}

bool PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_END >::operator()( BM::Stream &rkPacket )
{
	PgPlayer * pkPlayer = g_kPilotMan.GetPlayerUnit();
	if ( !pkPlayer )
	{
		XUIMgr.Close( L"FRM_ALRAM_MISSION" );
		return false;
	}

	return Call( pkPlayer->GetAlramMission() );
}

bool PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_END >::Call( PgAlramMission &rkAlramMission )
{
	XUI::CXUI_Wnd * pkWnd = XUIMgr.Get( L"FRM_ALRAM_MISSION" );
	if ( pkWnd )
	{
		TBL_DEF_ALRAM_MISSION const * const pkDef = rkAlramMission.GetDef();
		if ( pkDef )
		{
			BM::vstring vstr( TTW(120106) );
			std::wstring const* pkTitle = NULL;
			if ( true == GetDefString( pkDef->iTitleNo, pkTitle) )
			{
				vstr.Replace( L"#TITLE#", *pkTitle );
			}
			Notice_Show( vstr, EL_Warning );
		}

		rkAlramMission = PgAlramMission();//초기화
		return true;
	}

	return false;
}

bool PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_SUCCESS >::operator()( BM::Stream &kPacket )
{
	PgPlayer * pkPlayer = g_kPilotMan.GetPlayerUnit();
	if ( !pkPlayer )
	{
		return false;
	}

	return Call( pkPlayer->GetAlramMission() );
}

bool PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_SUCCESS >::Call( PgAlramMission &rkAlramMission )
{
	// 성공 처리
	XUI::CXUI_Wnd *pkWnd = XUIMgr.Get( L"FRM_ALRAM_MISSION" );
	if ( pkWnd )
	{
		TBL_DEF_ALRAM_MISSION const * const pkDef = rkAlramMission.GetDef();
		if ( pkDef )
		{
			if ( pkDef->kActionList.size() )
			{
				XUI::CXUI_Wnd *pkControl = pkWnd->GetControl( L"FRM_TOTAL" );
				if ( pkControl )
				{
					BM::vstring vstr( TTW(120101) );
					vstr.Replace( L"#COUNT#", pkDef->kActionList.at(0).iCount );
					vstr.Replace( L"#TOTAL#", pkDef->kActionList.at(0).iCount );
					pkControl->Text( vstr );
				}
			}
		}

		BM::vstring vstr( TTW(120105) );
		std::wstring const* pkTitle = NULL;
		if ( true == GetDefString( pkDef->iTitleNo, pkTitle) )
		{
			vstr.Replace( L"#TITLE#", *pkTitle );
		}
		Notice_Show( vstr, EL_Notice1 );

		pkWnd->Close();
		return true;
	}
	return false;
}

bool PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_PARAM >::operator()( BM::Stream &kPacket )
{
	size_t iIndex;
	int iCount;
	if (	true == kPacket.Pop( iIndex ) 
		&&	true == kPacket.Pop( iCount )
		)
	{
		PgPlayer * pkPlayer = g_kPilotMan.GetPlayerUnit();
		if ( !pkPlayer )
		{
			return false;
		}

		PgAlramMission & rkAlramMission = pkPlayer->GetAlramMission();
		if ( true == rkAlramMission.SetParam( iIndex, iCount ) )
		{
			TBL_DEF_ALRAM_MISSION const *pkDef = rkAlramMission.GetDef();
			if ( pkDef )
			{
				// 메세지 처리
				if ( iIndex < pkDef->kActionList.size() )
				{
					BM::vstring vstr;
					std::wstring const* pkTitle = NULL;
					if ( true == GetDefString( pkDef->kActionList.at(iIndex).iDiscriptionNo, pkTitle ) )
					{
						vstr = *pkTitle;
						vstr.Replace( L"#COUNT#", iCount );
						vstr.Replace( L"#TOTAL#", pkDef->kActionList.at(iIndex).iCount );
					}
					else
					{
						vstr = L"UnknownDefStr#";
						vstr += pkDef->kActionList.at(iIndex).iDiscriptionNo;
					}

					Notice_Show( vstr, EL_Normal );

					if ( pkDef->kActionList.size() == rkAlramMission.GetSuccessCount() )
					{
						PgAlramMissionClient< PT_M_C_NFY_ALRAMMISSION_SUCCESS >::Call( rkAlramMission );
					}
					else
					{
						XUI::CXUI_Wnd *pkWnd = XUIMgr.Get( L"FRM_ALRAM_MISSION" );
						if ( pkWnd )
						{
							XUI::CXUI_Wnd *pkControl = pkWnd->GetControl( L"FRM_TOTAL" );
							if ( pkControl )
							{
								BM::vstring vstr( TTW(120101) );
								vstr.Replace( L"#COUNT#", iCount );
								vstr.Replace( L"#TOTAL#", pkDef->kActionList.at(iIndex).iCount );
								pkControl->Text( vstr );
							}
						}
					}
					return true;
				}
			}
		}
	}

	return false;
}
