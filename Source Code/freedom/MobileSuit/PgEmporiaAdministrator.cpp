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
#include "PgGuild.h"
#include "PgEmporiaAdministrator.h"
#include "PgUIDataMgr.h"
#include "PgUICalculator.h"
#include "PgQuestMan.h"

namespace lwPgEmporiaAdministrator
{
	char const* EmporiaBuildingGradeStr(int const iBuildIDX);

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "EmporiaTalkAdministrator", &EmporiaTalkAdministrator);

		def(pkState, "EmporiaAddonBuildingConstruct", &EmporiaAddonBuildingConstruct);
		def(pkState, "EmporiaAddonBuildingDestruct", &EmporiaAddonBuildingDestruct);
		def(pkState, "EmporiaAddonBuildingMaintenancePayment", &EmporiaAddonBuildingMaintenancePayment);
		def(pkState, "EmporiaAddonBuildingCreated", &EmporiaAddonBuildingCreated);
		def(pkState, "EmporiaAddonBuildingUseable", &EmporiaAddonBuildingUseable);
		def(pkState, "EmporiaEntryOpenState", &EmporiaEntryOpenState);
		def(pkState, "EmporiaEntryOpenToOtherUser", &EmporiaEntryOpenToOtherUser);
		def(pkState, "BuyEmporiaTeleCard", &BuyEmporiaTeleCard);
		def(pkState, "CheckExistGuildEmporia", &CheckExistGuildEmporia);
		def(pkState, "CheckEmporiaGuildOwner", &CheckEmporiaGuildOwner);
		def(pkState, "GetEmporiaBuildingGradeStr", &EmporiaBuildingGradeStr);
	}

	void EmporiaTalkAdministrator(lwGUID kGuid)
	{
		BM::Stream	kPacket(PT_C_M_REQ_GET_EMPORIA_ADMINISTRATOR);
		kPacket.Push(kGuid.GetGUID());
		NETWORK_SEND(kPacket);
	}


	void EmporiaAddonBuildingConstruct(int const iBuildIDX, lwGUID kGuid)
	{
		if( EmporiaAddonBuildingUseable(iBuildIDX) )
		{
			SCalcInfo kCalcInfo;
			kCalcInfo.eCallType = CCT_EMPORIA_WEEK_SELECT;
			kCalcInfo.kGuid = kGuid.GetGUID();
			kCalcInfo.iIndex = iBuildIDX;

			SGuildEmporiaInfo kEmporiaInfo = g_kGuildMgr.GetEmporiaInfo();
			
			CONT_DEF_EMPORIA const* pDefEmporia = NULL;
			g_kTblDataMgr.GetContDef(pDefEmporia);
			if( pDefEmporia )
			{
				CONT_DEF_EMPORIA::const_iterator	emporia_iter = pDefEmporia->find(kEmporiaInfo.kID);
				if( emporia_iter != pDefEmporia->end() )
				{
					CONT_DEF_EMPORIA::mapped_type const& kDefEmporia = emporia_iter->second;

					int iIndex = static_cast<int>(kEmporiaInfo.byGrade-1);
					if ( iIndex < MAX_EMPORIA_GRADE )
					{
						CONT_DEF_EMPORIA_FUNCTION kAddonCont = kDefEmporia.m_kContDefFunc[ iIndex ];
						CONT_DEF_EMPORIA_FUNCTION::iterator	addon_iter = kAddonCont.find( iBuildIDX );
						if( addon_iter != kAddonCont.end() )
						{
							CONT_DEF_EMPORIA_FUNCTION::mapped_type const& BuildingInfo = addon_iter->second;

							kCalcInfo.iBasePrice = BuildingInfo.nPrice_ForWeek;
						}
					}
				}
			}
			std::wstring kText = TTW(60512 + iBuildIDX);
			kText += L"\n";
			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(60518).c_str(), kCalcInfo.iBasePrice);
			kText += szTemp;
			UpdateNpcTalkDialog(kText);
			CallCalculator(kCalcInfo);
		}
		else
		{
			UpdateNpcTalkDialog(TTW(71998));
		}
	}

	void EmporiaAddonBuildingDestruct(int const iBuildIDX, lwGUID kGuid)
	{
		if( EmporiaAddonBuildingCreated(iBuildIDX) )
		{
			bool bAddonControl = true;
			short kBuildingNo = static_cast<short>(iBuildIDX);

			BM::Stream	kPacket(PT_C_M_REQ_EMPORIA_ADMINISTRATION);
			kPacket.Push(kGuid.GetGUID());
			kPacket.Push(bAddonControl);
			kPacket.Push(kBuildingNo);
			NETWORK_SEND(kPacket);
		}
	}

	void EmporiaAddonBuildingMaintenancePayment(int const iBuildIDX, lwGUID kGuid)
	{
		if( EmporiaAddonBuildingCreated(iBuildIDX) )
		{
			SCalcInfo kCalcInfo;
			kCalcInfo.eCallType = CCT_EMPORIA_WEEK_SELECT;
			kCalcInfo.kGuid = kGuid.GetGUID();
			kCalcInfo.iIndex = iBuildIDX;

			SGuildEmporiaInfo kEmporiaInfo = g_kGuildMgr.GetEmporiaInfo();
			
			CONT_DEF_EMPORIA const* pDefEmporia = NULL;
			g_kTblDataMgr.GetContDef(pDefEmporia);
			if( pDefEmporia )
			{
				CONT_DEF_EMPORIA::const_iterator	emporia_iter = pDefEmporia->find(kEmporiaInfo.kID);
				if( emporia_iter != pDefEmporia->end() )
				{
					CONT_DEF_EMPORIA::mapped_type const& kDefEmporia = emporia_iter->second;

					int iIndex = static_cast<int>(kEmporiaInfo.byGrade-1);
					if ( iIndex < MAX_EMPORIA_GRADE )
					{
						CONT_DEF_EMPORIA_FUNCTION kAddonCont = kDefEmporia.m_kContDefFunc[ iIndex ];
						CONT_DEF_EMPORIA_FUNCTION::iterator	addon_iter = kAddonCont.find( iBuildIDX );
						if( addon_iter != kAddonCont.end() )
						{
							CONT_DEF_EMPORIA_FUNCTION::mapped_type const& BuildingInfo = addon_iter->second;

							kCalcInfo.iBasePrice = BuildingInfo.nPrice_ForWeek;
						}
					}
				}
			}

			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(60518).c_str(), kCalcInfo.iBasePrice);
			std::wstring kText = szTemp;
			kText += L"\n";
			kText += TTW(60512);
			kText += UpdateNpcTalkBuildingUseTime( iBuildIDX );
			UpdateNpcTalkDialog(kText);
			CallCalculator(kCalcInfo);	
		}
	}

	bool EmporiaAddonBuildingUseable(int const iBuildIDX)
	{
		SGuildEmporiaInfo kEmporiaInfo = g_kGuildMgr.GetEmporiaInfo();
		
		CONT_DEF_EMPORIA const* pDefEmporia = NULL;
		g_kTblDataMgr.GetContDef(pDefEmporia);
		if( pDefEmporia )
		{
			CONT_DEF_EMPORIA::const_iterator	emporia_iter = pDefEmporia->find(kEmporiaInfo.kID);
			if( emporia_iter != pDefEmporia->end() )
			{
				CONT_DEF_EMPORIA::mapped_type const& kDefEmporia = emporia_iter->second;

				int iIndex = static_cast<int>(kEmporiaInfo.byGrade-1);
				if ( iIndex < MAX_EMPORIA_GRADE )
				{
					CONT_DEF_EMPORIA_FUNCTION kAddonCont = kDefEmporia.m_kContDefFunc[ iIndex ];
					CONT_DEF_EMPORIA_FUNCTION::iterator	addon_iter = kAddonCont.find( iBuildIDX );
					if( addon_iter != kAddonCont.end() )
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	char const* EmporiaBuildingGradeStr(int const iBuildIDX)
	{
		static char pszNameText[MAX_PATH];
		pszNameText[0] = NULL;

		int const REAL_MAX_EMPORIA_GRADE = MAX_EMPORIA_GRADE-3;
		BM::vstring vStr;
		vStr = L"";

		SGuildEmporiaInfo const& kEmporiaInfo = g_kGuildMgr.GetEmporiaInfo();
		
		CONT_DEF_EMPORIA const* pDefEmporia = NULL;
		g_kTblDataMgr.GetContDef(pDefEmporia);
		if( pDefEmporia )
		{
			CONT_DEF_EMPORIA::const_iterator	emporia_iter = pDefEmporia->find(kEmporiaInfo.kID);
			if( emporia_iter != pDefEmporia->end() )
			{
				CONT_DEF_EMPORIA::mapped_type const& kDefEmporia = emporia_iter->second;
				for(int i=0; i<REAL_MAX_EMPORIA_GRADE; ++i)
				{
					CONT_DEF_EMPORIA_FUNCTION const& kAddonCont = kDefEmporia.m_kContDefFunc[i];
					CONT_DEF_EMPORIA_FUNCTION::const_iterator addon_iter = kAddonCont.find( iBuildIDX );
					if( addon_iter != kAddonCont.end() )
					{
						if(0 < vStr.size())
						{
							vStr += L"/";
						}
						vStr += i+1;
					}
				}
			}
		}
		strcpy_s(pszNameText, sizeof(pszNameText), MB(vStr));
		return pszNameText;
	}

	bool EmporiaAddonBuildingCreated(int const iBuildIDX)
	{
		PgUIData_EmporiaAdmin *pkData = NULL;
		if( g_kUIDataMgr.Get( UIDATA_EMPORIA_ADMIN, pkData ) )
		{
			return pkData->m_kFunc.IsHaveFunction( iBuildIDX );
		}
		return false;
	}

	bool EmporiaEntryOpenState()
	{
		PgUIData_EmporiaAdmin *pkData = NULL;
		if( g_kUIDataMgr.Get( UIDATA_EMPORIA_ADMIN, pkData ) )
		{
			return (pkData->m_byGateState == EMPORIA_GATE_PUBLIC);
		}
		return false;
	}

	void EmporiaEntryOpenToOtherUser(BYTE const bState, lwGUID kGuid)
	{
		PgUIData_EmporiaAdmin *pkData = NULL;
		if( g_kUIDataMgr.Get( UIDATA_EMPORIA_ADMIN, pkData ) )
		{
			if( pkData->m_byGateState != bState )
			{
				bool bAddonControl = false;

				BM::Stream	kPacket(PT_C_M_REQ_EMPORIA_ADMINISTRATION);
				kPacket.Push(kGuid.GetGUID());
				kPacket.Push(bAddonControl);
				kPacket.Push(bState);
				NETWORK_SEND(kPacket);
			}
		}
	}

	void BuyEmporiaTeleCard(int const iCommand, lwGUID kGuid)
	{
		lua_tinker::call<void, lwGUID>("NPC_Shop", kGuid);
	}

	bool CheckExistGuildEmporia()
	{
		SGuildEmporiaInfo kEmporiaInfo = g_kGuildMgr.GetEmporiaInfo();
		return (kEmporiaInfo.byType == EMPORIA_KEY_MINE);
	}

	bool CheckEmporiaGuildOwner()
	{
		if( CheckExistGuildEmporia() )
		{
			PgUIData_EmporiaAdmin *pkData = NULL;
			if( g_kUIDataMgr.Get( UIDATA_EMPORIA_ADMIN, pkData ) )
			{
				return pkData->m_bIsOwner;
			}
		}
		return false;
	}

	bool RecvEmporia_AdminPacket(WORD wPacketType, BM::Stream& kPacket)
	{
		switch( wPacketType )
		{
		case PT_N_C_ANS_GET_EMPORIA_ADMINISTRATOR:
			{
				BM::GUID kNpcGuid;
				bool bResult = false;//이게 false면 오픈권한이 없는거다 그냥 기본 메세지를 보여주면 된다.
				kPacket.Pop(kNpcGuid);
				kPacket.Pop(bResult);

				if( CheckNpc(kNpcGuid) )
				{
					if ( true == bResult )
					{
						PgUIData_EmporiaAdmin *pkData = new_tr PgUIData_EmporiaAdmin;
						if ( pkData )
						{
							pkData->ReadFromPacket( kPacket );
							g_kUIDataMgr.Add( pkData );
						}
					}
					lua_tinker::call<void, int, lwGUID, char const*, int>("CallNpcFunctionUI", 201, lwGUID(kNpcGuid), "c_William", 509374);
				}
			}break;
		case PT_N_C_ANS_EMPORIA_ADMINISTRATION://
			{
				BM::GUID kNpcGuid;
				E_EMPORIA_FUNC_RESULT kResult = EFUNC_ERROR;
				kPacket.Pop(kNpcGuid);
				kPacket.Pop(kResult);

				if( CheckNpc(kNpcGuid) )
				{
					switch( kResult )
					{
					case EFUNC_SUCCESS:
						{
							bool bIsAddonControl = false;
							kPacket.Pop(bIsAddonControl);

							PgUIData_EmporiaAdmin *pkData = NULL;
							if( g_kUIDataMgr.Get( UIDATA_EMPORIA_ADMIN, pkData ) )
							{
								if( bIsAddonControl )
								{
									short kBuildingNo = 0;
									short kUseableWeek = 0;

									kPacket.Pop(kBuildingNo);
									kPacket.Pop(kUseableWeek);

									bool bPrevHaveFunc = pkData->m_kFunc.IsHaveFunction( kBuildingNo );
									SEmporiaFunction kFunc;
									pkData->m_kFunc.GetFunction( kBuildingNo, kFunc );
									kFunc.AddTime(kUseableWeek);
									pkData->m_kFunc.AddFunction( kBuildingNo, kFunc );

									int iDialogNo = (bPrevHaveFunc)?(206):(203);

									std::wstring kText = TTW(509364 + kBuildingNo);

									if( bPrevHaveFunc )
									{
										kText += TTW(60507);
									}
									else
									{
										kText += TTW((kBuildingNo < 3)?(60504):(60505));
										kText += TTW(60503);
									}
									kText += L"\n";
									GetBuildingUseTimeToText(kFunc.GetRemainTime(), kText);
									lua_tinker::call<void, int, lwGUID, char const*, lwWString>("CallNpcFunctionUI2", iDialogNo, lwGUID(kNpcGuid), "c_William", lwWString(kText));
								}
								else
								{
									BYTE kState;
									kPacket.Pop(kState);
									pkData->m_byGateState = kState;
									int iTTWID = (kState == EMPORIA_GATE_PUBLIC)?(60502):(60501);
									lua_tinker::call<void, int, lwGUID, char const*, int>("CallNpcFunctionUI", 207, lwGUID(kNpcGuid), "c_William", iTTWID);
								}
							}
						}break;
					case EFUNC_ACCESSDENIED:
						{
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 60509, true);
						}break;
					case EFUNC_GUILDEXP:
						{
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 60510, true);
						}break;
					case EFUNC_NOT:
						{
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 60511, true);
						}break;
					case EFUNC_ERROR:
					default:
						{//정의되지 않은 오류
						}break;
					}
				}
			}break;
		default:
			{
				return false;
			}
		}
		return true;
	}

	bool CheckNpc(BM::GUID const& kGuid)
	{
		PgPilot* pkPilot = g_kPilotMan.FindPilot(kGuid);
		if( !pkPilot )
		{
			return false;
		}

		PgNpc* pkNpc = dynamic_cast<PgNpc*>(pkPilot->GetUnit());
		if( !pkNpc )
		{
			return false;
		}

		PgActor *pkActor = g_kPilotMan.GetPlayerActor();
		if( pkActor )
		{
			POINT3 pt3Pos( pkActor->GetPosition().x, pkActor->GetPosition().y, pkActor->GetPosition().z );
			float fRange = POINT3::Distance(pt3Pos, pkNpc->GetPos());
			if( (PgNpcTalkUtil::fNPC_TALK_ENABLE_RANGE - 40.f) > fRange )
			{
				return true;
			}
		}
		return false;
	}

	void GetBuildingUseTimeToText(__int64 const iRemainTime, std::wstring& kText)
	{
		int iDay = static_cast<int>(iRemainTime / 60 / 24);
		int iHour = static_cast<int>(iRemainTime / 60 % 24);
		int iMin = static_cast<int>(iRemainTime % 60);

		wchar_t szTemp[MAX_PATH]={0,};
		swprintf_s(szTemp, MAX_PATH, TTW(60506).c_str(), iDay, iHour, iMin);

		kText += szTemp;
	}

	void SendEmporiaAddonBuildingOrder(short const iBuildIDX, BM::GUID const& kGuid, short const iWeekDate)
	{
		bool bAddonControl = true;
		BM::Stream	kPacket(PT_C_M_REQ_EMPORIA_ADMINISTRATION);
		kPacket.Push(kGuid);
		kPacket.Push(bAddonControl);
		kPacket.Push(iBuildIDX);
		kPacket.Push(iWeekDate);
		NETWORK_SEND(kPacket);
	}

	std::wstring const UpdateNpcTalkBuildingUseTime(int const iBuildingNo)
	{
		PgUIData_EmporiaAdmin *pkData = NULL;
		if( g_kUIDataMgr.Get( UIDATA_EMPORIA_ADMIN, pkData ) )
		{
			if( pkData->m_kFunc.IsHaveFunction( iBuildingNo ) )
			{
				SEmporiaFunction kFunc;
				pkData->m_kFunc.GetFunction( iBuildingNo, kFunc );

				std::wstring kText = TTW(509364 + iBuildingNo);
				kText += TTW(60508);
				kText += TTW(60512);
				GetBuildingUseTimeToText(kFunc.GetRemainTime(), kText);

				return kText;
			}
		}
		return std::wstring();
	}

	void UpdateNpcTalkDialog(std::wstring const& kText)
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get( PgQuestManUtil::kFullQuestWndName );
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Wnd* pTalkMain = pMainUI->GetControl(L"FRM_MAIN");
		if( !pTalkMain )
		{
			return;
		}

		XUI::CXUI_Wnd* pTalkBG = pTalkMain->GetControl(L"FRM_TEXT_BG");
		if( !pTalkBG )
		{
			return;
		}

		XUI::CXUI_Wnd* pTalk = pTalkBG->GetControl(L"FRM_TEXT");
		if( !pTalk )
		{
			return;
		}

		pTalk->Text(kText);
	}
}
