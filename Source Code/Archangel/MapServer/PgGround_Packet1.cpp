#include "stdafx.h"
#include "bm/pgfilterstring.h"
#include "Collins/Log.h"
#include "Lohengrin/VariableContainer.h"
#include "variant/constant.h"
#include "Variant/PgEventView.h"
#include "Variant/PgMissionInfo.h"
#include "Variant/PgLogUtil.h"
#include "Variant/PgMission.h"
#include "variant/PgStore.h"
#include "Variant/PgQuestInfo.h"
#include "Variant/PgParty.h"
#include "Variant/PgPartyMgr.h"
#include "variant/PgMacroCheckTable.h"
#include "Variant/PgEventQuestBase.h"
#include "Variant/Emoticon.h"
#include "Variant/ClientOption.h"
#include "variant/pggamblemachine.h"
#include "Global.h"
#include "PgLocalPartyMgr.h"
#include "PgGround.h"
#include "PgMissionGround.h"
#include "PgPacketManager.h"
#include "PgQuest.h"
#include "PgAction.h"
#include "PgAction_Pet.h"
#include "PgActionQuest.h"
#include "PgStoneMgr.h"
#include "PgPartyItemRule.h"
#include "PgLocalPartyMgr.h"
#include "PgItemTradeMgr.h"
#include "PgRequest.h"
#include "PgActionPost.h"
#include "PgActionUserMarket.h"
#include "PgMissionMan.h"
#include "PublicMap/PgWarMode.h"
#include "PgGroundUnitMgr.h"
#include "PgActionAchievement.h"
#include "PgMonKillCountReward.h"
#include "PgEffectFunction.h"
#include "publicmap/PgBSGround.h"
#include "PgSuperGround.h"
#include "PgActionJobSkill.h"
#include "PgHackDualKeyboard.h"
#include "PgSkillAbilHandleManager.h"
#include "Variant/PgNpcEvent.h"
//#include <chrono>

namespace PgGroundUtil
{
	inline bool IsPvpGround(T_GNDATTR const& rAttr)
	{
		if( GATTR_PVP == rAttr )
		{
			return true;
		}
		if( GATTR_EMPORIABATTLE == rAttr )
		{
			return true;
		}
		return false;
	}

	bool IsVillageGround(T_GNDATTR const& rAttr)
	{
		return GATTR_VILLAGE == rAttr;
	}

	bool IsDefaultGround(T_GNDATTR const& rAttr)
	{
		return GATTR_DEFAULT == rAttr;
	}

	bool IsIndunGround(T_GNDATTR const& rAttr)
	{
		if(GATTR_MYHOME == rAttr)
		{
			return false;	// 마이홈은 예외다.
		}
		return 0 != (rAttr & GATTR_INSTANCE);
	}

	bool CheckCanKickGround(T_GNDATTR const& rAttr)
	{
		return (rAttr & GATTR_FLAG_ENABLE_KICK) ? false : true;
	}

	inline bool IsCanProtectedEventID(int const iEventID)
	{
		int const iMinimumCanProtectedEventID = 1000; // 1000 이상의 이벤트는 무적을 걸지 않는다.
		return iMinimumCanProtectedEventID > iEventID;
	}

	bool IsableOpenMarket(CUnit* pkPC, T_GNDATTR const& rAttr, int const iGroundNo, ESpecialUnitStatusType const eSpecType, TCHAR const* szFunc, size_t const iLine)
	{
		if( !pkPC )
		{
			return false;
		}

		if ( false == PgGround::ms_kAntiHackCheckVariable.bUse_OpenMarket )
		{
			return true;
		}
		if( !IsVillageGround(rAttr) )
		{
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][OpenMarket] Can't use OpenMarket is this Ground[") << iGroundNo << _T("] GUID[") << pkPC->GetID() << _T("], USER[") << pkPC->Name() << _T("]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV1, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPC);
				if (pkPlayer)
				{
					pkPlayer->SendWarnMessageStr(kLogMsg);
				}
			}

			return false;
		}
		if( SUST_OpenMarket != eSpecType )
		{
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][OpenMarket] Can't use, user status is not OpenMarket[NOW:") << static_cast< int >(eSpecType) << _T("], at this Ground[") << iGroundNo << _T("] GUID[") << pkPC->GetID() << _T("] USER[") << pkPC->Name() << _T("]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV1, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPC);
				if (pkPlayer)
				{
					pkPlayer->SendWarnMessageStr(kLogMsg);
				}
			}

			return false;
		}
		return true;
	}

	bool IsableVendor(CUnit* pkPC, T_GNDATTR const& rAttr, int const iGroundNo, ESpecialUnitStatusType const eSpecType, TCHAR const* szFunc, size_t const iLine)
	{
		if( !pkPC )
		{
			return false;
		}

		if ( false == PgGround::ms_kAntiHackCheckVariable.bUse_OpenMarket )
		{
			return true;
		}
		if( !IsVillageGround(rAttr) && !IsDefaultGround(rAttr) )
		{
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][OpenMarket] Can't use Vendor is this Ground[") << iGroundNo << _T("] GUID[") << pkPC->GetID() << _T("], USER[") << pkPC->Name() << _T("]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV1, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPC);
				if (pkPlayer)
				{
					pkPlayer->SendWarnMessageStr(kLogMsg);
				}
			}

			return false;
		}
		if( SUST_Vendor != eSpecType )
		{
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][OpenMarket] Can't use, user status is not Vendor[NOW:") << static_cast< int >(eSpecType) << _T("], at this Ground[") << iGroundNo << _T("] GUID[") << pkPC->GetID() << _T("] USER[") << pkPC->Name() << _T("]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV1, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPC);
				if (pkPlayer)
				{
					pkPlayer->SendWarnMessageStr(kLogMsg);
				}
			}

			return false;
		}
		return true;
	}

	bool IsableCashshop(CUnit* pkPC, T_GNDATTR const& rAttr, int const iGroundNo, ESpecialUnitStatusType const eSpecType, TCHAR const* szFunc, size_t const iLine)
	{
		if( !pkPC )
		{
			return false;
		}

		if ( false == PgGround::ms_kAntiHackCheckVariable.bUse_CashShop )
		{
			return true;
		}

		if( IsIndunGround(rAttr) )
		{
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][CashShop] Can't use CashShop is this Ground[") << iGroundNo << _T("] GUID[") << pkPC->GetID() << _T("] USER[") 
				<< pkPC->Name() << _T("]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV1, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPC);
			if (pkPlayer && g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				pkPlayer->SendWarnMessageStr(kLogMsg);
			}

			return false;
		}
		if( SUST_CashShop != eSpecType )
		{
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][CashShop] Can't use, user status is not CashShop[NOW:") << static_cast< int >(eSpecType) << _T("], at this Ground[") 
				<< iGroundNo << _T("] GUID[") << pkPC->GetID() << _T("] USER[") << pkPC->Name() << _T("]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV1, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPC);
			if (pkPlayer && g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				pkPlayer->SendWarnMessageStr(kLogMsg);
			}

			return false;
		}
		return true;
	}

	bool IsCanTalkableRange(CUnit* pkPC, BM::GUID const& rkNpcGuid, CUnit* pkNPC, int const iCurGround, ENpcMenuType const eMenuType, TCHAR const* szFunc, size_t const iLine)
	{
		if( pkPC )
		{
			return IsCanTalkableRange(pkPC, pkPC->GetPos(), rkNpcGuid, pkNPC, iCurGround, eMenuType, szFunc, iLine);
		}
		return false;
	}

	bool IsCanTalkableRange(CUnit* pkPC, POINT3 const& rkPos, BM::GUID const& rkNpcGuid, CUnit* pkNPC, int const iCurGround, ENpcMenuType const eMenuType, TCHAR const* szFunc, size_t const iLine)
	{
		if( !pkPC )
		{
			return false;
		}

		PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkPC) ;
		if( !pkPlayer )
		{
			return false;
		}

		if ( false == PgGround::ms_kAntiHackCheckVariable.bUse_NPCRange )
		{
			return true;
		}

		if( !pkNPC )
		{
			POINT3 const& rkPlayerPos = pkPC->GetPos();
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][NPCRange] Can't Talk to NPC(Can't find NPC Unit)[Guid: ") << rkNpcGuid << 
				_T("], GUID[") << pkPC->GetID() << _T("] USER[") << pkPC->Name() << _T("], G:") << iCurGround << _T(", Pos(") << rkPlayerPos.x << _T(", ") << rkPlayerPos.y << _T(", ") << rkPlayerPos.z << _T(")]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV5, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				pkPlayer->SendWarnMessageStr(kLogMsg);
			}
			return false;
		}

		if( rkNpcGuid != pkNPC->GetID() )
		{
			POINT3 const& rkPlayerPos = pkPC->GetPos();
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][NPCRange] not equal NPC GUID[Guid: ") << rkNpcGuid << _T("] NPC Unit[Guid: ") << pkNPC->GetID() << _T(", Name: ") << pkNPC->Name() <<
				_T("], GUID[") << pkPC->GetID() << _T("] USER[") << pkPC->Name() << _T("], G:") << iCurGround << _T(", Pos(") << rkPlayerPos.x << _T(", ") << rkPlayerPos.y << _T(", ") << rkPlayerPos.z << _T(")]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV5, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				pkPlayer->SendWarnMessageStr(kLogMsg);
			}
			return false;
		}

		if(UT_MYHOME == pkNPC->UnitType())
		{
			return true;
		}

		PgNpc* pkNonePlayer = dynamic_cast< PgNpc* >(pkNPC);
		if( !pkNonePlayer )
		{
			POINT3 const& rkPlayerPos = pkPC->GetPos();
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][NPCRange] Target is not NPC Unit[Guid: ") << pkNPC->GetID() << _T(", Name: ") << pkNPC->Name() <<
				_T("], Player[Guid: ") << pkPC->GetID() << _T(", Name: ") << pkPC->Name() << _T(", G:") << iCurGround << _T(", Pos(") << rkPlayerPos.x << _T(", ") << rkPlayerPos.y << _T(", ") << rkPlayerPos.z << _T(")]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV5, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				pkPlayer->SendWarnMessageStr(kLogMsg);
			}
			return false;
		}

		CONT_ACTIVATE_NPC_EVENT_ACTION kContNpcEventAction;
		if(g_kNpcEventMgr.Check(iCurGround, pkPC, kContNpcEventAction))
		{
			if( g_kNpcEventMgr.IsHiddenNpc(rkNpcGuid, kContNpcEventAction) )
			{
				return false;
			}
		}

		if( NMT_ClientOnly != eMenuType
		&&	!pkNonePlayer->IsCanMenu(eMenuType) )
		{
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][NPCRange] Can't Talk Menu to NPC[Guid: ") << pkNonePlayer->GetID() << _T(", Name: ") << pkNonePlayer->Name() << 
				_T(", Menu: ") << pkNonePlayer->MenuStr() << _T("] have not the MENU[") << eMenuType << 
				_T("], GUID[") << pkPlayer->GetID() << _T("] USER[") << pkPlayer->Name() << _T("], G:") << iCurGround << _T("]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV5, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				pkPlayer->SendWarnMessageStr(kLogMsg);
			}
			return false;
		}

		POINT3 const& rkNpcPos = pkNonePlayer->GetPos();
		POINT3 const& rkPlayerPos = rkPos;
		float const fDistance = GetDistanceQ(rkPlayerPos, rkNpcPos); // Quick Vector Length
		float const fLimitDistance = PgNpcTalkUtil::fNPC_TALK_ENABLE_RANGE * PgNpcTalkUtil::fNPC_TALK_ENABLE_RANGE; // 2제곱 값
		if( 0.f > fDistance
		||	fLimitDistance < fDistance )
		{
			BM::vstring kLogMsg;
			kLogMsg << __FL2__(szFunc, iLine) << _T("[HACKING][NPCRange] Can't Talk to NPC[Guid: ") << pkNonePlayer->GetID() << _T(", Name: ") << pkNonePlayer->Name() << 
				_T(" Pos(") << rkNpcPos.x << _T(", ") << rkNpcPos.y << _T(", ") << rkNpcPos.z << _T(")] out of range[") << fLimitDistance << _T(" < ") << fDistance << 
				_T("], GUID[") << pkPlayer->GetID() << _T("] USER[") << pkPlayer->Name() << _T("], G:") << iCurGround << _T(", Pos(") << rkPlayerPos.x << _T(", ") << rkPlayerPos.y << _T(", ") << rkPlayerPos.z << _T(")]");
			//VERIFY_INFO_LOG(false, BM::LOG_LV5, kLogMsg);
			HACKING_LOG(BM::LOG_LV0, kLogMsg);
			if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
			{
				pkPlayer->SendWarnMessageStr(kLogMsg);
			}
			return false;
		}
		return true;
	}

	bool IsNeedNpcTestChangeItemEvent(SItemPos const& rkPos)
	{
		switch(rkPos.x)
		{
		case KUIG_SAFE:
		case KUIG_RENTAL_SAFE1:
		case KUIG_RENTAL_SAFE2:
		case KUIG_RENTAL_SAFE3:
		case KUIG_SHARE_RENTAL_SAFE1:
		case KUIG_SHARE_RENTAL_SAFE2:
		case KUIG_SHARE_RENTAL_SAFE3:
		case KUIG_SHARE_RENTAL_SAFE4:
			{
				return true;
			}break;
		}
		return false;
	}

	bool IsCallSafe(CUnit * pkUnit,SItemPos const & kSourcePos,SItemPos const & kTargetPos)
	{
		switch(kSourcePos.x)
		{
		case IT_SAFE:
		case IT_SAFE_ADDITION:
		case IT_RENTAL_SAFE1:
		case IT_RENTAL_SAFE2:
		case IT_RENTAL_SAFE3:
			{
				return (1 == pkUnit->GetAbil(AT_CALL_SAFE));
			}break;
		case IT_SHARE_RENTAL_SAFE1:
		case IT_SHARE_RENTAL_SAFE2:
		case IT_SHARE_RENTAL_SAFE3:
		case IT_SHARE_RENTAL_SAFE4:
			{
				return (1 == pkUnit->GetAbil(AT_CALL_SHARE_SAFE));
			}break;
		}

		switch(kTargetPos.x)
		{
		case IT_SAFE:
		case IT_SAFE_ADDITION:
		case IT_RENTAL_SAFE1:
		case IT_RENTAL_SAFE2:
		case IT_RENTAL_SAFE3:
			{
				return (1 == pkUnit->GetAbil(AT_CALL_SAFE));
			}break;
		case IT_SHARE_RENTAL_SAFE1:
		case IT_SHARE_RENTAL_SAFE2:
		case IT_SHARE_RENTAL_SAFE3:
		case IT_SHARE_RENTAL_SAFE4:
			{
				return (1 == pkUnit->GetAbil(AT_CALL_SHARE_SAFE));
			}break;
		}
		return false;
	}

	bool IsMyHomeHasFuctionItem(CUnit * pkPlayerUnit,CUnit * pkNpcUnit,EUseItemCustomType const kType)
	{
		if(NULL == pkNpcUnit || UT_MYHOME != pkNpcUnit->UnitType())
		{
			return false;
		}

		PgMyHome * pkHome = dynamic_cast<PgMyHome *>(pkNpcUnit);
		if(NULL == pkHome)
		{
			return false;
		}

		if(pkHome->OwnerGuid() != pkPlayerUnit->GetID())
		{
			return false;
		}

		ContHaveItemNoCount kOut;
		if(S_OK != pkNpcUnit->GetInven()->GetItems(IT_HOME,kType,kOut))
		{
			return false;
		}

		ContHaveItemNoCount::const_iterator itor = kOut.begin();
		if(itor == kOut.end())
		{
			return false;
		}

		SItemPos kItemPos;
		if(S_OK != pkNpcUnit->GetInven()->GetFirstItem(IT_HOME,(*itor).first,kItemPos))
		{
			return false;
		}

		PgBase_Item kItem;
		if(S_OK != pkNpcUnit->GetInven()->GetItem(kItemPos,kItem))
		{
			return false;
		}

		if(true == kItem.IsUseTimeOut())
		{
			return false;
		}

		return true;
	}

	bool IsMyHomeHasSideJob(CUnit * pkPlayerUnit,CUnit * pkNpcUnit,eMyHomeSideJob const kSideJob)
	{
		if(NULL == pkNpcUnit || UT_MYHOME != pkNpcUnit->UnitType())
		{
			return false;
		}

		PgMyHome * pkHome = dynamic_cast<PgMyHome *>(pkNpcUnit);
		if(NULL == pkHome)
		{
			return false;
		}

		int const iSideJob = pkHome->GetAbil(AT_HOME_SIDEJOB);
		if(0 == (iSideJob & kSideJob))
		{
			return false;
		}

		return true;
	}

	bool IsSaveHPMP(EGndAttr const eGndAttr)
	{
		bool bReturn = true;
		switch(eGndAttr)
		{
		case GATTR_PVP:
		case GATTR_EMPORIABATTLE:
		case GATTR_BATTLESQUARE:
			{
				bReturn = false;
			}break;
		default:
			{
				bReturn = true;
			}break;
		}
		return bReturn;
	}

};

namespace GroundHackUtil
{
	void Log(SGroundKey const& rkGndKey, BM::GUID const& rkPartyGuid, BM::GUID const& rkCharGuid, std::wstring const& rkName, int const iCategoryNo, int const iActionID, POINT3 const& rkPoint3, DWORD const dwRecvTime)
	{
		//CAUTION_LOG(BM::LOG_LV1, L"GND:" << rkGndKey.GroundNo() << L" " << rkGndKey.Guid() << L"Action History - "<< rkPartyGuid << L" " << rkCharGuid<<L" "<<rkName
		//	<< L" "<< iCategoryNo << L" " << iActionID << L" " 
		//	<< rkPoint3.x << L"," << rkPoint3.y << L"," << rkPoint3.z <<L"-ALV_SEC:"<< dwRecvTime);
		SEventMessage kMsg(PMET_HACK_CHECK_DUALKEYBOARD, HackDualKeyboard::ACMM_ACTION);
		rkGndKey.WriteToPacket( kMsg );
		kMsg.Push( rkCharGuid );
		kMsg.Push( static_cast< HackDualKeyboard::EHackActionCheckType >(iCategoryNo) );
		kMsg.Push( dwRecvTime );
		g_kTask.PutMsg(kMsg);
	}
	bool IsCanLogStatus(CUnit* pkUnit)
	{
		if( pkUnit )
		{
			return 0 == pkUnit->GetAbil(AT_CANNOT_DAMAGE); // 무적어빌 설정되 있으면 무시
		}
		return false;
	}
	void AddLog(SGroundKey const& rkGndKey, CUnit* pkActionUnit, int const iActionID, DWORD dwRecvTime)
	{
		if( !IsCanLogStatus(pkActionUnit) )
		{
			return;
		}
		switch( g_kLocal.ServiceRegion() )
		{
		case LOCAL_MGR::NC_KOREA:
		case LOCAL_MGR::NC_DEVELOP:
			{
				CONT_DEFSKILL const* pkContDefSkill = NULL;
				g_kTblDataMgr.GetContDef(pkContDefSkill);
				if( pkContDefSkill )
				{
					CONT_DEFSKILL::const_iterator find_iter = pkContDefSkill->find(iActionID);
					if( pkContDefSkill->end() != find_iter )
					{
						if( 0 < (*find_iter).second.iActionCategoryNo )
						{
							Log(rkGndKey, pkActionUnit->GetPartyGuid(), pkActionUnit->GetID(), pkActionUnit->Name(), (*find_iter).second.iActionCategoryNo, iActionID, pkActionUnit->GetPos(), dwRecvTime);
						}
					}
				}
			}break;
		default:
			{
			}break;
		}
	}
	void AddLogChangeMoveDirection(SGroundKey const& rkGndKey, CUnit* pkActionUnit, Direction const eMoveDirection, DWORD dwRecvTime)
	{
		if( !IsCanLogStatus(pkActionUnit) )
		{
			return;
		}

		int const ACT_MOVE_LEFT = 2;
		int const ACT_MOVE_RIGHT = 3;
		int const ACT_MOVE_TOP = 4;
		int const ACT_MOVE_BOTTOM = 5;

		int eMoveActionType = 0;
		switch( eMoveDirection )
		{
		case DIR_LEFT:	{ eMoveActionType = ACT_MOVE_LEFT; }break;
		case DIR_RIGHT:	{ eMoveActionType = ACT_MOVE_RIGHT; }break;
		case DIR_UP:	{ eMoveActionType = ACT_MOVE_TOP; }break;
		case DIR_DOWN:	{ eMoveActionType = ACT_MOVE_BOTTOM; }break;
		default:
			{
				return;
			}break;
		}
		Log(rkGndKey, pkActionUnit->GetPartyGuid(), pkActionUnit->GetID(), pkActionUnit->Name(), eMoveActionType, 100005426, pkActionUnit->GetPos(), dwRecvTime);
	}
};

extern bool IsCashShopOrOpenMarketPacketType(BM::Stream::DEF_STREAM_TYPE wkType);
extern bool IsPacketType(BM::Stream::DEF_STREAM_TYPE wkType);

bool PgGround::FakeRemoveUnit( CUnit * pkCaller, int const iHideMode )
{
	BM::CAutoMutex kLock( m_kRscMutex );
	if(!pkCaller)
	{
		VERIFY_INFO_LOG( false, BM::LOG_LV0, __FL__);
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("pkCaller is NULL"));
		return false;
	}

	if( pkCaller->GetAbil(AT_FAKE_REMOVE_UNIT) )	// 클라이언트에서 여러번 메시지를 전송할경우 어빌 수치가 꼬일수 있으므로 막음
	{
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("GetAbil is Failed!"));
		return false;
	}
	
	pkCaller->AddCountAbil( AT_CANNOT_DAMAGE, AT_CF_FAKEREMOVE, true, E_SENDTYPE_BROADALL_EFFECTABIL );// 타겟팅 안되도록 설정
	pkCaller->AddCountAbil( AT_CANNOT_ATTACK, AT_CF_FAKEREMOVE, true, E_SENDTYPE_BROADALL_EFFECTABIL );// 공격못하도록 설정

	if ( FRT_ONLY_HIDE < iHideMode )
	{
		pkCaller->SetAbil( AT_FAKE_REMOVE_UNIT, iHideMode );
		pkCaller->SetSyncType( SYNC_TYPE_NONE );// 나도 안보이고, 남(몬스터)도 안보인다.
	}
	else
	{
		pkCaller->SetAbil( AT_FAKE_REMOVE_UNIT, FRT_ONLY_HIDE );
		pkCaller->AddAbil( AT_HIDE, 1 );
		pkCaller->SendAbil( AT_HIDE, E_SENDTYPE_BROADALL );
	}
	return true;
}

bool PgGround::FakeAddUnit(CUnit * pkCaller)
{
	BM::CAutoMutex kLock( m_kRscMutex );
	if(!pkCaller)
	{
		VERIFY_INFO_LOG( false, BM::LOG_LV0, __FL__ );
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("pkCaller is NULL"));
		return false;
	}

	int const iFakeRemoveAbil = pkCaller->GetAbil(AT_FAKE_REMOVE_UNIT);
	switch( iFakeRemoveAbil )
	{
	case FRT_ONLY_HIDE:
		{
			if ( pkCaller->GetAbil(AT_HIDE) > 0 )
			{
				pkCaller->AddAbil( AT_HIDE, -1 );
				pkCaller->SendAbil( AT_HIDE, E_SENDTYPE_BROADALL );
			}
		}break;
	case FRT_HIDE_AND_NO_MSG:
	case FRT_HIDE_AND_NO_MSG_BUT_MAPMOVE:
	default:
		{
			BYTE kSyncType = SYNC_TYPE_DEFAULT;
			PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkCaller);
			if ( pkPlayer )
			{
				if ( pkPlayer->GMLevel() )
				{
					if ( true == pkPlayer->GmAbilCheck( GMABIL_HIDDEN ) )
					{
						kSyncType = SYNC_TYPE_RECV_ADD;
					}
				}
			}
			else
			{
				VERIFY_INFO_LOG( false, BM::LOG_LV1, __FL__ << _T("Is Not Player?<") << pkCaller->Name() << _T("> CharGuid<") << pkCaller->GetID() << _T(">") );
			}
			
			pkCaller->SetSyncType( kSyncType );
		}break;
	}

	pkCaller->SetAbil( AT_FAKE_REMOVE_UNIT, 0 );
	pkCaller->AddCountAbil( AT_CANNOT_DAMAGE, AT_CF_FAKEREMOVE, false, E_SENDTYPE_BROADALL_EFFECTABIL );// 타겟팅 안되도록 설정 해제
	pkCaller->AddCountAbil( AT_CANNOT_ATTACK, AT_CF_FAKEREMOVE, false, E_SENDTYPE_BROADALL_EFFECTABIL );// 공격못하도록 설정 해제
	return true;
}

bool PgGround::VUpdate( CUnit* pkUnit, WORD const wType, BM::Stream* pkNfy )
{
	{// 캐시샵이나 오픈마켓 패킷이면 ON/OFF 검사(GM캐릭은 점검 때도 이용 가능)
		PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkUnit);
		if( pkPlayer )
		{
			if(IsCashShopOrOpenMarketPacketType(wType))
			{
				if( !g_kEventView.VariableCont().bCashShopOpen
				&&	!pkPlayer->GMLevel() )
				{// 캐시샵, 오픈마켓을 CLOSE 한 상태
					pkUnit->SendWarnMessage(18055); // 서비스 점검 중입니다.
					return false;
				}
			}
		}
	}	

	switch(wType)
	{
	case PT_C_M_REQ_ENCHANT_SHIFT:
		{
			PgAction_Enchant_Shift kAction(GroundKey(), *pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_ENTER:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit && UT_MYHOME == pkNpcUnit->UnitType())
			{
				SHomeChatGuest kGuest(pkUnit->GetID(),pkUnit->Name(),false,false,false,pkUnit->GetAbil(AT_CLASS),pkUnit->GetAbil(AT_GENDER));
				BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_ENTER);
				kPacket.Push(pkUnit->GetID());
				kPacket.Push(*pkNfy);
				kGuest.WriteToPacket(kPacket);
				SendToMyhomeMgr(kPacket);
			}
			else
			{
				BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_ENTER);
				kPacket.Push(E_MYHOME_NOT_IN_HOME);
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_EXIT:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit && UT_MYHOME == pkNpcUnit->UnitType())
			{
				BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_EXIT);
				kPacket.Push(pkUnit->GetID());
				SendToMyhomeMgr(kPacket);
			}
			else
			{
				BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_EXIT);
				kPacket.Push(E_MYHOME_NOT_IN_HOME);
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_MODIFY_ROOM:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit && UT_MYHOME == pkNpcUnit->UnitType())
			{
				std::wstring	kTitle, 
								kNoti, 
								kPassword;
				size_t kMaxUserCount = 0;

				pkNfy->Pop(kTitle);
				pkNfy->Pop(kNoti);
				pkNfy->Pop(kPassword);
				pkNfy->Pop(kMaxUserCount);

				SHomeChatRoom kRoom(BM::GUID::Create(), kTitle, kNoti, kPassword, kMaxUserCount);

				BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_MODIFY_ROOM);
				kPacket.Push(pkUnit->GetID());
				kRoom.WriteToPacket(kPacket);
				SendToMyhomeMgr(kPacket);
			}
			else
			{
				BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_MODIFY_ROOM);
				kPacket.Push(E_MYHOME_NOT_IN_HOME);
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_MODIFY_GUEST:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit && UT_MYHOME == pkNpcUnit->UnitType())
			{
				BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_MODIFY_GUEST);
				kPacket.Push(pkUnit->GetID());
				kPacket.Push(*pkNfy);
				SendToMyhomeMgr(kPacket);
			}
			else
			{
				BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_MODIFY_GUEST);
				kPacket.Push(E_MYHOME_NOT_IN_HOME);
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_ROOM_LIST:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit && UT_MYHOME == pkNpcUnit->UnitType())
			{
				BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_ROOM_LIST);
				kPacket.Push(pkUnit->GetID());
				SendToMyhomeMgr(kPacket);
			}
			else
			{
				BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_ROOM_LIST);
				kPacket.Push(E_MYHOME_NOT_IN_HOME);
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_KICK_GEUST:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit && UT_MYHOME == pkNpcUnit->UnitType())
			{
				BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_KICK_GEUST);
				kPacket.Push(pkUnit->GetID());
				kPacket.Push(*pkNfy);
				SendToMyhomeMgr(kPacket);
			}
			else
			{
				BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_KICK_GEUST);
				kPacket.Push(E_MYHOME_NOT_IN_HOME);
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_SET_ROOMMASTER:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit && UT_MYHOME == pkNpcUnit->UnitType())
			{
				BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_SET_ROOMMASTER);
				kPacket.Push(pkUnit->GetID());
				kPacket.Push(*pkNfy);
				SendToMyhomeMgr(kPacket);
			}
			else
			{
				BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_ROOM_CREATE);
				kPacket.Push(E_MYHOME_NOT_IN_HOME);
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_ROOM_CREATE:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit && UT_MYHOME == pkNpcUnit->UnitType())
			{
				PgMyHome * pkHome = dynamic_cast<PgMyHome*>(pkNpcUnit);
				if(pkHome)
				{
					if(pkHome->OwnerGuid() != pkUnit->GetID())
					{
						BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_ROOM_CREATE);
						kPacket.Push(E_MYHOME_NOT_HAVE_AUTHORITY);
						pkUnit->Send(kPacket);
					}
					else
					{
						std::wstring	kTitle, 
										kNoti, 
										kPassword;

						size_t kMaxUserCount = 0;

						pkNfy->Pop(kTitle);
						pkNfy->Pop(kNoti);
						pkNfy->Pop(kPassword);
						pkNfy->Pop(kMaxUserCount);

						
						SHomeChatGuest kGuest(pkUnit->GetID(),pkUnit->Name(),false,false,true,pkUnit->GetAbil(AT_CLASS),pkUnit->GetAbil(AT_GENDER));
						SHomeChatRoom kRoom(BM::GUID::Create(), kTitle, kNoti, kPassword, kMaxUserCount);

						BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_ROOM_CREATE);
						kPacket.Push(pkUnit->GetID());
						kGuest.WriteToPacket(kPacket);
						kRoom.WriteToPacket(kPacket);
						SendToMyhomeMgr(kPacket);
					}
				}
			}
			else
			{
				BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_ROOM_CREATE);
				kPacket.Push(E_MYHOME_NOT_IN_HOME);
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_START_SIDE_JOB:
		{
			PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkPlayer)
			{
				BM::Stream kPacket(PT_M_I_REQ_START_SIDE_JOB, pkPlayer->GetID());
				kPacket.Push(pkPlayer->HomeAddr());
				kPacket.Push(*pkNfy);
				SendToItem(GroundKey(),kPacket);
			}
		}break;
	case PT_C_M_REQ_CANCEL_SIDE_JOB:
		{
			PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkPlayer)
			{
				BM::Stream kPacket(PT_M_I_REQ_CANCEL_SIDE_JOB, pkPlayer->GetID());
				kPacket.Push(pkPlayer->HomeAddr());
				kPacket.Push(*pkNfy);
				SendToItem(GroundKey(),kPacket);
			}
		}break;
	case PT_C_M_REQ_ROULETTE_MIXUP_RESULT:
		{
			BM::Stream kPacket(PT_M_C_ANS_ROULETTE_MIXUP_RESULT);
			CONT_PLAYER_MODIFY_ORDER kOrder;
			kOrder.push_back(SPMO(IMET_GAMBLE_MIXUP_REMOVE,pkUnit->GetID()));
			PgAction_ReqModifyItem kItemModifyAction(CIE_GambleMachine_Mixup, GroundKey(), kOrder, kPacket);
			kItemModifyAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_RELOAD_ROULETTE_MIXUP:
		{
			BM::Stream kPacket(PT_M_C_ANS_RELOAD_ROULETTE_MIXUP);
			CONT_PLAYER_MODIFY_ORDER kOrder;
			kOrder.push_back(SPMO(IMET_GAMBLE_MIXUP_MODIFY,pkUnit->GetID()));
			PgAction_ReqModifyItem kItemModifyAction(CIE_GambleMachine_Mixup, GroundKey(), kOrder, kPacket);
			kItemModifyAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_ROULETTE_RESULT:
		{
			BM::Stream kPacket(PT_M_C_ANS_ROULETTE_RESULT);
			CONT_PLAYER_MODIFY_ORDER kOrder;
			kOrder.push_back(SPMO(IMET_GAMBLE_REMOVE,pkUnit->GetID()));
			PgAction_ReqModifyItem kItemModifyAction(CIE_GambleMachine_Shop, GroundKey(), kOrder, kPacket);
			kItemModifyAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_RELOAD_ROULETTE:
		{
			BM::Stream kPacket(PT_M_C_ANS_RELOAD_ROULETTE);
			CONT_PLAYER_MODIFY_ORDER kOrder;
			kOrder.push_back(SPMO(IMET_GAMBLE_MODIFY,pkUnit->GetID()));
			PgAction_ReqModifyItem kItemModifyAction(CIE_GambleMachine_Shop, GroundKey(), kOrder, kPacket);
			kItemModifyAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_NPC_TALK:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit * pkNpc = GetUnit(kNpcGuid);
			if(pkNpc)
			{
				PgAddAchievementValue kMA(AT_ACHIEVEMENT_NPC_TALK,1,GroundKey());
				kMA.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_USE_REDICE_OPTION_ITEM:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_USE_REDICE_OPTION_ITEM);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			if( false == kAction.DoAction( pkUnit, pkUnit ) )
			{
				kPacket.Push(kAction.Ret());
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_EXCHANGE_LOGCOUNTTOEXP:
		{
			PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkPlayer)
			{
				if(true == pkPlayer->HomeAddr().IsNull())
				{
					BM::Stream kPacket(PT_M_C_ANS_EXCHANGE_LOGCOUNTTOEXP);
					kPacket.Push(E_MYHOME_NOT_FOUND);
					pkPlayer->Send(kPacket);
				}
				else
				{
					BM::Stream kPacket(PT_M_I_REQ_EXCHANGE_LOGCOUNTTOEXP);
					kPacket.Push(pkPlayer->GetID());
					kPacket.Push(pkPlayer->HomeAddr());
					SendToItem(GroundKey(), kPacket);
				}
			}
		}break;
	case PT_C_M_REQ_RESET_ATTATCHED:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_RESET_ATTATCHED);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			if( false == kAction.DoAction( pkUnit, pkUnit ) )
			{
				kPacket.Push(kAction.Ret());
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_SKILL_EXTEND:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_SKILL_EXTEND);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			if( false == kAction.DoAction( pkUnit, pkUnit ) )
			{
				kPacket.Push(kAction.Ret());
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_USE_UPGRADE_OPTION_ITEM:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_USE_UPGRADE_OPTION_ITEM);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			if( false == kAction.DoAction( pkUnit, pkUnit ) )
			{
				kPacket.Push(kAction.Ret());
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_BASIC_OPTION_AMP:
		{
			PgAction_BasicOptionAmp kAction(GroundKey(), *pkNfy);
			kAction.DoAction(pkUnit,pkUnit);
		}break;
	case PT_C_M_REQ_USE_REDICE_OPTION_PET:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_USE_REDICE_OPTION_PET);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			if( false == kAction.DoAction( pkUnit, pkUnit ) )
			{
				kPacket.Push(kAction.Ret());
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_USE_REPAIR_MAX_DURATION_ITEM:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_USE_REPAIR_MAX_DURATION_ITEM);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			if( false == kAction.DoAction( pkUnit, pkUnit ) )
			{
				kPacket.Push(kAction.Ret());
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_USE_TELEPORT_ITEM:
		{
			PgPlayer * pkPlayer = dynamic_cast<PgPlayer *>(pkUnit);
			if(pkPlayer)
			{
				OnReqItemAction( pkPlayer, pkNfy, NULL);
			}
		}break;
	case PT_C_M_REQ_SORT_ITEM:
		{
			DWORD const dwNowTime = BM::GetTime32();
			DWORD dwLastUseInvSortTime = static_cast<DWORD>(pkUnit->GetAbil(AT_USE_INV_SORT_TIME));

			if( 2000 < DifftimeGetTime( dwLastUseInvSortTime, dwNowTime ) )
			{
				pkUnit->SetAbil( AT_USE_INV_SORT_TIME, static_cast<int>(dwNowTime) );

				EInvType kInvType;
				pkNfy->Pop(kInvType);

				CONT_PLAYER_MODIFY_ORDER kContModifyOrder;

				BM::Stream kPacket(PT_M_C_ANS_SORT_ITEM);

				HRESULT const hRes = pkUnit->GetInven()->MakeSortOrder(kInvType, pkUnit, g_kQuestMan.ContQuestItems(), kContModifyOrder);
				if(S_OK == hRes)
				{
					BM::Stream kPacket(PT_M_C_ANS_SORT_ITEM);
					PgAction_ReqModifyItem kItemModifyAction(CIE_Inventory_Sort, GroundKey(), kContModifyOrder, kPacket);
					kItemModifyAction.DoAction(pkUnit, NULL);
				}
				else
				{
					kPacket.Push(hRes);
					pkUnit->Send(kPacket);
				}
			}			
		}break;
	case PT_C_M_REQ_USE_REPAIR_ITEM:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_USE_REPAIR_ITEM);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			if( false == kAction.DoAction( pkUnit, pkUnit ) )
			{
				kPacket.Push(kAction.Ret());
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_USE_ENCHANT_ITEM:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_USE_ENCHANT_ITEM);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			if( false == kAction.DoAction( pkUnit, pkUnit ) )
			{
				kPacket.Push(kAction.Ret());
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_GAMBLEMACHINEINFO:
		{
			BM::Stream kPacket(PT_M_C_ANS_GAMBLEMACHINEINFO);
			kPacket.Push(*pkNfy);
			g_kGambleMachine.WriteToPacket(kPacket);
			pkUnit->Send(kPacket);

			BM::Stream kReqCash(PT_M_I_REQ_GAMBLEMACHINEINFO);
			kReqCash.Push(pkUnit->GetID());
			SendToItem(GroundKey(),kReqCash);
		}break;
	case PT_C_M_REQ_USE_GAMBLEMACHINE_CASH:
		{
			BM::Stream kPacket(PT_M_I_REQ_USE_GAMBLEMACHINE);
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToItem(GroundKey(),kPacket);
		}break;
	case PT_C_M_REQ_USE_GAMBLEMACHINE:
		{
			PgAction_ReqUseGambleMachine kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,pkUnit);
		}break;
	case PT_C_M_REQ_USE_GAMBLEMACHINE_READY:
		{
			PgAction_ReqUseGambleMachine_CashShop kAction(this, GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_USE_GAMBLEMACHINE_MIXUP_READY:
		{
			PgAction_ReqUseGambleMachine_Mixup kAction(this, GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_MYHOME_BUY:
		{
			bool bIsReqPrice = false;
			pkNfy->Pop( bIsReqPrice );

			if( bIsReqPrice )
			{
				int iCost = DEFAULT_MYHOME_APT_COST;
				g_kVariableContainer.Get(EVar_Kind_MyHome, EVar_MyHome_Apt_Gold, iCost);

				BM::Stream kPacket( PT_M_C_ANS_MYHOME_PRICE ); // 가격 요청 응답
				kPacket.Push( iCost );
				pkUnit->Send( kPacket );
			}
			else
			{
				PgAction_Myhome_Buy kAction(GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_MYHOME_SELL:
		{
			PgAction_Myhome_Sell kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_HOME_REQ_MAPMOVE:
		{
			PgAction_ReqHomeMapMove kAction(this,GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_MIXUPITEM:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_MixupItem, __F_P__(PT_C_M_REQ_MIXUPITEM), __LINE__) )
			{
				PgAction_ReqMixupItem kAction(kNpcGuid,GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_EXPCARD_USE:
		{
			PgAction_ReqUseExpCard kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_EXPCARD_ACTIVATE:
		{
			PgAction_ReqSetExpCard kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_EXPCARD_DEACTIVATE:
		{
			PgInventory * pkInv = pkUnit->GetInven();
			pkInv->ExpCardItem(BM::GUID::NullData());
			BM::Stream kPacket(PT_M_C_ANS_EXPCARD_DEACTIVATE);
			kPacket.Push(S_OK);
			pkUnit->Send(kPacket);
		}break;
	case PT_C_M_REQ_CONVERTITEM:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_ItemConvert, __F_P__(PT_C_M_REQ_CONVERTITEM), __LINE__) )
			{
				PgAction_ReqConvertItem kAction(GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_EVENT_LIST:
		{
			BM::Stream kPacket(PT_M_C_ANS_EVENT_LIST);
			g_kEventView.WriteToPacketEventList(kPacket);
			pkUnit->Send(kPacket);
		}break;
	case PT_C_M_REQ_JOIN_EVENT:
		{
			PgAction_ReqJoinEvent kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_OPEN_GAMBLE:
		{
			PgAction_ReqOpenGamble kAction( GroundKey(), *pkNfy, GetEventAbil() );
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_HIDDEN_ITEM_PACK:
		{
			PgAction_HiddenItemBag kAction( GroundKey(), *pkNfy, GetEventAbil() );
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_OPEN_LOCKED_CHEST:
		{
			PgAction_ReqOpenLockedChest kAction( GroundKey(), *pkNfy, GetEventAbil() );
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_USE_EMOTION:
		{		
			int const iRdPos = pkNfy->RdPos();
			PgAction_ReqUseEmotion kAction(GroundKey(),*pkNfy);
			if(kAction.DoAction(pkUnit,NULL))
			{
				pkNfy->RdPos(iRdPos);

				BYTE bType = 0;
				int iEmotionNo = 0;
				POINT3 ptClientPos;

				pkNfy->Pop(bType);	
				pkNfy->Pop(iEmotionNo);
				pkNfy->Pop(ptClientPos);

				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if(pkPlayer)
				{
					// 이모션은 액션 정보가 없기 때문에 idle로 세팅한다. 
					pkPlayer->GetPlayerActionInfo()->iActionID = 100000101;

					POINT3 ptPos = pkPlayer->GetPos();

					// 클라이언트에서 보낸 좌표와 서버에서 가지고 있는 좌표의 거리가 일정 거리 이상 차이가 날 경우
					if(AI_Z_LIMIT <= abs(POINT3::Distance(ptClientPos, ptPos)))
					{
						NxRay kRay(NxVec3(ptPos.x, ptPos.y, ptPos.z+30), NxVec3(0, 0, -1.0f));
						NxRaycastHit kHit;
						NxShape *pkHitShape = RayCast(kRay, kHit);
						if(pkHitShape)
						{
							POINT3 ptNewPos(kHit.worldImpact.x, kHit.worldImpact.y, kHit.worldImpact.z);

							// 클라이언트의 데이터를 믿지 않고 서버에서 Ray를 다시 쏴서 Z 좌표를 세팅 한다.
							ptPos.z = kHit.worldImpact.z;
							pkPlayer->SetPos(ptPos);
						}
					}
				}
			}
		}break;
	case PT_C_M_REQ_ROLLBACK_ENCHANT:
		{
			PgAction_ReqRollbackEnchant kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_GEN_SOCKET:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit)
			{
				switch(pkNpcUnit->UnitType())
				{
				case UT_MYHOME:
					{
						PgAction_ReqGenSocket kAction(GroundKey(),*pkNfy);
						kAction.DoAction(pkUnit,pkNpcUnit);
					}break;
				default:
					{
						if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_Socket, __F_P__(PT_C_M_REQ_GEN_SOCKET), __LINE__) )
						{
							PgAction_ReqGenSocket kAction(GroundKey(),*pkNfy);
							kAction.DoAction(pkUnit,NULL);
						}
					}break;
				}
			}
		}break;
	case PT_C_M_REQ_SET_MONSTERCARD:
		{
			PgAction_ReqSetMonsterCard kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_RESET_MONSTERCARD:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_Socket, __F_P__(PT_C_M_REQ_RESET_MONSTERCARD), __LINE__) )
			{
				PgAction_ReqDestroySocket kAction(GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_EXTRACTION_MONSTERCARD:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_Socket, __F_P__(PT_C_M_REQ_EXTRACTION_MONSTERCARD), __LINE__) )
			{
				PgAction_ReqExtractionSocket kAction(GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_REMOVE_MONSTERCARD:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_Socket, __F_P__(PT_C_M_REQ_REMOVE_MONSTERCARD), __LINE__) )
			{
				PgAction_ReqRemoveSocket kAction(GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_GEMSTOREINFO:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_GemStore, __F_P__(PT_C_M_REQ_GEMSTOREINFO), __LINE__) )
			{
				CONT_GEMSTORE const * pkDef = NULL;
				g_kTblDataMgr.GetContDef(pkDef);
				if(NULL != pkDef)
				{
					CONT_GEMSTORE::const_iterator iter = pkDef->find(kNpcGuid);
					if(iter != pkDef->end())
					{
						BM::Stream kPacket(PT_M_C_ANS_GEMSTOREINFO);
						(*iter).second.WriteToPacket(kPacket);
						pkUnit->Send(kPacket);
					}
				}
			}
		}break;
	case PT_C_M_REQ_GEMSTORE_BUY:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_GemStore, __F_P__(PT_C_M_REQ_GEMSTORE_BUY), __LINE__) )
			{
				PgAction_ReqGemStoreBuy kAction(kNpcGuid,GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_DEFGEMSTOREINFO:
		{
			BM::GUID kNpcGuid;
			int iMenu = 0;
			pkNfy->Pop(kNpcGuid);
			pkNfy->Pop(iMenu);
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_GemStore, __F_P__(PT_C_M_REQ_DEFGEMSTOREINFO), __LINE__) ||
				PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_ElementDungeon, __F_P__(PT_C_M_REQ_DEFGEMSTOREINFO), __LINE__) )
			{
				CONT_DEFGEMSTORE const * pkDef = NULL;
				g_kTblDataMgr.GetContDef(pkDef);
				if(NULL != pkDef)
				{
					CONT_DEFGEMSTORE::const_iterator iter = pkDef->find(kNpcGuid);
					if(iter != pkDef->end())
					{
						BM::Stream kPacket(PT_M_C_ANS_DEFGEMSTOREINFO);
						(*iter).second.WriteToPacket(kPacket);
						kPacket.Push(iMenu);
						pkUnit->Send(kPacket);
					}
				}
			}
		}break;
	case PT_C_M_REQ_DEFGEMSTORE_BUY:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);			
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_GemStore, __F_P__(PT_C_M_REQ_DEFGEMSTORE_BUY), __LINE__) )
			{
				PgAction_ReqDefGemStoreBuy kAction(kNpcGuid,GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_COLLECT_ANTIQUE:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_GemStore, __F_P__(PT_C_M_REQ_COLLECT_ANTIQUE), __LINE__) )
			{
				PgAction_ReqDefCollectAntique kAction(GroundKey(), *pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_EXCHANGE_GEMSTORE:
		{
			PgAction_ReqDefExchangeGemStore Action(GroundKey(), *pkNfy);
			Action.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_USE_PORTAL:
		{
			OnReqItemAction( dynamic_cast<PgPlayer*>(pkUnit), pkNfy );
		}break;
	case PT_C_M_REQ_REG_PORTAL:
		{
			PgAction_ReqRegUserPortal kAction(this,*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_SEARCH_MATCH_CARD:
		{
			BM::Stream kPacket(PT_M_I_REQ_SEARCH_MATCH_CARD);
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToItem(GroundKey(),kPacket);
		}break;
	case PT_C_M_REQ_CHARACTER_CARD_INFO:
		{
			BM::Stream kPacket(PT_M_I_REQ_CHARACTER_CARD_INFO);
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToItem(GroundKey(),kPacket);
		}break;
	case PT_C_M_REQ_CREATE_CHARACTER_CARD:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_CREATE_CHARACTER_CARD);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			if( false == kAction.DoAction( pkUnit, pkUnit ) )
			{
				kPacket.Push(kAction.Ret());
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_MODIFY_CHARACTER_CARD:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_MODIFY_CHARACTER_CARD);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			if( false == kAction.DoAction( pkUnit, pkUnit ) )
			{
				kPacket.Push(kAction.Ret());
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_MODIFY_CHARACTER_CARD_COMMENT:
		{
			CONT_PLAYER_MODIFY_ORDER kCont;
			std::wstring kString;
			pkNfy->Pop(kString);

			BM::Stream kPacket(PT_M_C_ANS_MODIFY_CHARACTER_CARD_COMMENT);

			bool const bCommentFiltered = g_kFilterString.Filter(kString, false, FST_ALL);
			bool const bCommentUniCodeFiltered = (false == g_kUnicodeFilter.IsCorrect(UFFC_CHAR_CARD_COMMENT, kString));
			if( bCommentFiltered
			||	bCommentUniCodeFiltered )
			{
				kPacket.Push(E_BADSTRING);
				pkUnit->Send(kPacket);
				break;
			}

			kCont.push_back(SPMO(IMET_MODIFY_CARD_COMMENT,pkUnit->GetID(),SMOD_Modify_Comment(kString)));

			PgAction_ReqModifyItem kItemModifyAction(CIE_Modify_CharacterCard,GroundKey(), kCont, kPacket);
			kItemModifyAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_RECOMMEND_CHARACTER:
		{
			CONT_PLAYER_MODIFY_ORDER kCont;
			BM::GUID kTargetGuid;
			pkNfy->Pop(kTargetGuid);

			BM::Stream kPacket(PT_M_C_ANS_RECOMMEND_CHARACTER);

			CUnit const * pkTarget = GetUnit(kTargetGuid);
			if(!pkTarget)
			{
				kPacket.Push(E_NOT_FOUND_CARD);
				pkUnit->Send(kPacket);
			}
			else
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if(pkPlayer)
				{
					int const iRP = pkPlayer->RecommendPoint();

					if(0 < iRP)
					{
						if(ENABLE_RECOMMEND_LEVEL <= pkUnit->GetAbil(AT_LEVEL))
						{
							kCont.push_back(SPMO(IMET_ADD_RECOMMENDPOINT,pkUnit->GetID(),SMOD_AddRecommendPoint(-1)));
							kCont.push_back(SPMO(IMET_ADD_POPULARPOINT,kTargetGuid,SMOD_AddPopularPoint(1,pkPlayer->Name())));
							
							ECharacterCardRecommendType kType = CCRT_RECOMMEND;
							BM::Stream kPacket(PT_M_C_ANS_RECOMMEND_CHARACTER);
							kPacket.Push(pkPlayer->GetID());
							kPacket.Push(static_cast<BYTE>(kType));
							kPacket.Push(pkTarget->Name());
							kPacket.Push(iRP-1);
							PgAction_ReqModifyItem kItemModifyAction(CIE_Modify_RecommendPoint,GroundKey(), kCont, kPacket);
							kItemModifyAction.DoAction(pkPlayer, NULL);

							{// 내가 다른 캐릭터 추천 한 회수에 따른 업적
								PgAddAchievementValue kMA( AT_ACHIEVEMENT_RECOMMEND_POINT, 1, GroundKey() );
								kMA.DoAction( pkPlayer, NULL );
							}
						}
						else
						{
							kPacket.Push(E_NOT_ENOUGH_LEVEL);
							pkUnit->Send(kPacket);
						}
					}
					else
					{
						kPacket.Push(E_NOT_ENOUGH_RP);
						pkUnit->Send(kPacket);
					}
				}
			}
		}break;
	case PT_C_M_REQ_MODIFY_CHARACTER_CARD_STATE:
		{
			CONT_PLAYER_MODIFY_ORDER kCont;
			bool bEnable = false;
			pkNfy->Pop(bEnable);
			kCont.push_back(SPMO(IMET_MODIFY_CARD_STATE,pkUnit->GetID(),SMOD_Modify_CardState(bEnable)));

			BM::Stream kPacket(PT_M_C_ANS_MODIFY_CHARACTER_CARD_STATE);
			PgAction_ReqModifyItem kItemModifyAction(CIE_Modify_CharacterCard,GroundKey(), kCont, kPacket);
			kItemModifyAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_INVENTORY_EXTEND:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_INVENTORY_EXTEND);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			kAction.DoAction( pkUnit, pkUnit );
		}break;
	case PT_C_M_REQ_INVENTORY_EXTENDIDX:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_INVENTORY_EXTENDIDX);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			kAction.DoAction( pkUnit, pkUnit );
		}break;
	case PT_C_M_REQ_ITEM_ACTION_BIND:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_ITEM_ACTION_BIND);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			kAction.DoAction( pkUnit, pkUnit );
		}break;
	case PT_C_M_REQ_ITEM_ACTION_UNBIND:
		{
			SItemPos kItemPos;
			PgBase_Item kOutItem;
			pkNfy->Pop(kItemPos);
			if(S_OK == pkUnit->GetInven()->GetItem(kItemPos, kOutItem))
			{
				BM::Stream kPacket(PT_M_C_ANS_ITEM_ACTION_UNBIND, kOutItem.ItemNo());
				PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
				kAction.DoAction( pkUnit, pkUnit );
			}
		}break;
	case PT_C_M_NOTI_DETECTION_HACKING:
		{
			if ( PgGround::ms_kAntiHackCheckVariable.bUse_ClientNotify )
			{
				EDETECTION_HACK_TYPE kHackType;
				pkNfy->Pop(kHackType);

				BM::vstring const kLogMsg = BM::vstring() << __FL__ << _T("[HACKING][ClientNotify] Detection Hack Use GUID[") << pkUnit->GetID() << _T("] ") << _T("USER[") << pkUnit->Name() << _T("]") << _T(" Type[") << kHackType << _T("]");
				//VERIFY_INFO_LOG(false, BM::LOG_LV0, kLogMsg);
				HACKING_LOG(BM::LOG_LV5, kLogMsg);

				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if (pkPlayer)
				{
					if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
					{
						pkPlayer->SendWarnMessageStr(kLogMsg);
					}
					if ( pkPlayer->SuspectHacking(EAHP_ClientNotify, PgGround::ms_kAntiHackCheckVariable.sClientNotify_AddIndex))
					{
						// SpeedHack이 의심스러우니 강제접속 해제 시킨다.
						BM::Stream kDPacket( PT_A_S_NFY_USER_DISCONNECT, static_cast<BYTE>(CDC_ClientNotify_Hack) );
						kDPacket.Push( pkPlayer->GetMemberGUID() );
						SendToServer( pkPlayer->GetSwitchServer(), kDPacket );
					}
				}
			}
		}break;
	case PT_C_M_REQ_MACRO_INPUT_PASSWORD:
		{
			if ( false == PgGround::ms_kAntiHackCheckVariable.bUse_AntiMacro )
			{
				return true;
			}

			int iCount = 0;
			pkNfy->Pop(iCount);

			PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if(!pkPlayer)
			{
				return false;
			}

			if(false == pkPlayer->MacroWaitAns())
			{
				BM::vstring kLogMsg;
				kLogMsg << __FL__ << _T("[MACRO] Detection Hack Use GUID[") << pkUnit->GetID() << _T("] ") << _T("USER[") << pkUnit->Name() << _T("]") << _T(" Use CallUI(FRM_CALL_ORDER)");
				//VERIFY_INFO_LOG(false, BM::LOG_LV5, kLogMsg);
				HACKING_LOG(BM::LOG_LV5,kLogMsg);
				if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
				{
					pkPlayer->SendWarnMessageStr(kLogMsg);
				}

				BM::Stream kDPacket( PT_A_S_NFY_USER_DISCONNECT, static_cast<BYTE>(CDC_Invalid_Macro_Time) );
				kDPacket.Push( pkPlayer->GetMemberGUID() );
				SendToServer( pkPlayer->GetSwitchServer(), kDPacket );
				return false;
			}

			if(true == pkPlayer->ProcessMacroCheckPassword(iCount))
			{
				pkPlayer->MacroClear();
				int const iNewHP = std::min(pkPlayer->GetAbil(AT_HP)+1000,pkPlayer->GetAbil(AT_C_MAX_HP));
				int const iNewMP = std::min(pkPlayer->GetAbil(AT_MP)+1000,pkPlayer->GetAbil(AT_C_MAX_MP));
				pkPlayer->Alive( EALIVE_NONE, E_SENDTYPE_BROADALL,iNewHP, iNewMP);
				PgDeathPenalty kAction(GroundKey(), LURT_Macro);
				kAction.DoAction(pkPlayer, NULL);
				BM::Stream kPacket(PT_M_C_ANS_MACRO_INPUT_PASSWORD);
				pkPlayer->Send(kPacket);

				BM::Stream Packet(PT_M_N_NFY_EXPEDITION_MODIFY_MEMBER_STATE); // 멤버 상태 변화 알림.
				Packet.Push(pkPlayer->GetID());
				Packet.Push(pkPlayer->IsAlive());
				::SendToGlobalPartyMgr(Packet);
			}
			else
			{
				if(pkPlayer->MacroInputFailCount() >= 3)
				{
					pkPlayer->MacroClear();
					SReqMapMove_MT kRMM(MMET_None);
					kRMM.kTargetKey.GroundNo(pkPlayer->GetAbil(AT_LAST_VILLAGE));
					kRMM.nTargetPortal = 1;
					PgReqMapMove kMapMove( this, kRMM, NULL );
					if ( kMapMove.Add( pkPlayer ) )
					{
						kMapMove.DoAction();
					}
				}
				else
				{
					SMACRO_CHECK_TABLE kTable = g_kMacroCheckTable.GetMacroCheckTable();
					wchar_t kCharKey = 0;
					int iCount = 0;
					pkPlayer->MakeMacroCheckPassword(kCharKey,iCount);
					BM::Stream kPacket(PT_M_C_NOTI_MACRO_INPUT_PASSWORD);
					kPacket.Push(kCharKey);
					kPacket.Push(iCount);
					kPacket.Push(pkPlayer->MacroInputFailCount());
					kPacket.Push(kTable.iMacroInputTime);
					pkPlayer->Send(kPacket,E_SENDTYPE_SELF | E_SENDTYPE_MUSTSEND);
				}
			}
		}break;
	case PT_C_M_REQ_OXQUIZ_ENTER:
		{
			BM::Stream kPacket(PT_M_I_REQ_OXQUIZ_ENTER);
			g_kProcessCfg.ServerIdentity().WriteToPacket(kPacket);
			kPacket.Push(GroundKey());
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToOXQuizEvent(kPacket);
		}break;
	case PT_C_M_REQ_OXQUIZ_EXIT:
		{
			BM::Stream kPacket(PT_M_I_REQ_OXQUIZ_EXIT);
			g_kProcessCfg.ServerIdentity().WriteToPacket(kPacket);
			kPacket.Push(GroundKey());
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToOXQuizEvent(kPacket);
		}break;
	case PT_C_M_REQ_OXQUIZ_ANS_QUIZ:
		{
			BM::Stream kPacket(PT_M_I_REQ_OXQUIZ_ANS_QUIZ);
			g_kProcessCfg.ServerIdentity().WriteToPacket(kPacket);
			kPacket.Push(GroundKey());
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToOXQuizEvent(kPacket);
		}break;
	case PT_C_M_REQ_LUCKYSTAR_ENTER:
		{
			BM::Stream kPacket(PT_M_I_REQ_LUCKYSTAR_ENTER);
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToLuckyStarEvent(kPacket);
		}break;
	case PT_C_M_REQ_LUCKYSTAR_CHANGE_STAR:
		{
			BM::Stream kPacket(PT_M_I_REQ_LUCKYSTAR_CHANGE_STAR);
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToLuckyStarEvent(kPacket);
		}break;
	case PT_C_M_REQ_FIT2PRIMARYINV:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);

			PgFitToPrimaryInv kAction( kItemPos, this );
			kAction.DoAction( pkUnit, NULL );
		}break;
	case PT_C_M_REQ_MOVETOSUMMONER:
		{
			PgAction_ReqMoveToSummoner kAction(this, GroundKey(), this->m_kLocalPartyMgr, *pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_SUMMONPARTYMEMBER:
		{
			PgAction_ReqSummonPartyMember kAction(this, GroundKey(), this->m_kLocalPartyMgr, *pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_COUPON_EVENT_SYNC:
		{
			BM::Stream kPacket(PT_M_C_ANS_COUPON_EVENT_SYNC);
			g_kTask.WriteToPacket(PMET_COUPON_EVENT_VIEW_MAP, kPacket);
			pkUnit->Send(kPacket);
		}break;
	case PT_C_M_REQ_RENTALSAFE_EXTEND:
		{
			PgAction_ReqRentalSafeExtend kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_MOVETOPARTYMEMBER:
		{
			PgAction_ReqMoveToPartyMember kAction(this,GroundKey(), this->m_kLocalPartyMgr,*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_MOVETOPARTYMASTERGROUND:
		{
			PgAction_ReqMoveToPartyMemberGround kAction(this, GroundKey(), this->m_kLocalPartyMgr, *pkNfy);
			kAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_USER_MAP_MOVE:
		{
			if( PgGroundUtil::SUST_HardCoreDungeonVote == GetSpecStatus(pkUnit->GetID()) )
			{
				pkUnit->SendWarnMessage( 402204 );// 하드코어 던젼 투표중에는 이동 금지
			}
			else
			{
				PgAction_ReqUserMapMove kAction(this,GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_REQ_ACHIEVEMENT_TO_ITEM:
		{
			int iAchievementIdx = 0;
			pkNfy->Pop(iAchievementIdx);
			PgAchievementToItem kAction(iAchievementIdx,GroundKey());
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_ACHIEVEMENT_RANK:
		{
			BM::Stream kPacket(PT_M_I_REQ_ACHIEVEMENT_RANK,pkUnit->GetID());
			SendToItem(GroundKey(),kPacket);
		}break;
	case PT_C_M_REQ_SYNCTIME_CHECK:
		{
			BM::Stream kAnsPacket( PT_M_C_ANS_SYNCTIME_CHECK, g_kEventView.GetLocalSecTime(CGameTime::DEFAULT) );
			pkUnit->Send( kAnsPacket );
		}break;
	case PT_C_M_NFY_RECENT_MAP_MOVE:
		{
			this->RecvRecentMapMove( dynamic_cast<PgPlayer*>(pkUnit) );
		}break;
	case PT_C_M_TRY_TAKE_COUPON:
		{//맵서버가 알 필요도 없고.
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);

			if(pkUser)
			{
				std::wstring kStr;
				pkNfy->Pop(kStr);
				BM::Stream kPacket(PT_M_N_TRY_TAKE_COUPON);
				kPacket.Push(pkUser->GetMemberGUID());
				kPacket.Push(pkUser->GetID());
				kPacket.Push(kStr);	
				return SendToContents(kPacket);
			}
		}break;
	case PT_C_M_REQ_INDUN_PARTY_LIST:
		{
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				BM::Stream kPacket(PT_C_M_REQ_INDUN_PARTY_LIST);
				kPacket.Push(pkUser->GetID());
				kPacket.Push(*pkNfy);
				return SendToContents(kPacket);
			}
		}break;
	case PT_C_M_REQ_CONSTELLATION_PARTY_LIST:
		{
			VEC_INT ContMapNo;
			BM::GUID KeyGuid;
			int Type = 0;
			pkNfy->Pop(KeyGuid);
			pkNfy->Pop(Type);

			Constellation_GetMapNo(ContMapNo);

			if( false == ContMapNo.empty() )
			{
				PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
				if(pkUser)
				{
					BM::Stream kPacket(PT_C_M_REQ_INDUN_PARTY_LIST);
					kPacket.Push(pkUser->GetID());
					kPacket.Push(KeyGuid);
					kPacket.Push(Type);
					kPacket.Push(ContMapNo);
					return SendToContents(kPacket);
				}
			}
		}
	case PT_C_M_REQ_REALTYDEALER:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop( kNpcGuid );
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( false == PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_RealtyDealer, __F_P__(PT_C_M_REQ_HOUSESHOP_INFO), __LINE__) )
			{
				return false;
			}

			CONT_REALTYDEALER const * pkDef = NULL;
			g_kTblDataMgr.GetContDef(pkDef);
			if(NULL == pkDef)
			{
				return false;
			}

			CONT_REALTYDEALER::const_iterator iter = pkDef->find(kNpcGuid);
			if(iter == pkDef->end())
			{
				return false;
			}

			BM::Stream kPacket(PT_M_C_ANS_REALTYDEALER);
			(*iter).second.WriteToPacket(kPacket);
			pkUnit->Send(kPacket);
		}break;
	case PT_C_M_REQ_HOME_ITEM_MODIFY:
		{
			BM::GUID kHomeGuid;
			pkNfy->Pop(kHomeGuid);
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				CUnit * pkHome = GetUnit(kHomeGuid);
				if(pkHome)
				{
					PgAction_ReqHomeItemModify kAction(GroundKey(),*pkNfy);
					kAction.DoAction(pkUnit,pkHome);
				}
			}		
		}break;
	case PT_C_M_REQ_HOME_USE_HOME_STYLEITEM:
		{
			OnReqItemAction( dynamic_cast<PgPlayer*>(pkUnit), pkNfy );
		}break;
	case PT_C_M_REQ_MYHOME_VISITORS:
		{
			BM::Stream kPacket(PT_M_I_REQ_MYHOME_VISITORS);
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				kPacket.Push(pkUnit->GetID());
				kPacket.Push(pkUser->HomeAddr());
				SendToItem(GroundKey(),kPacket);
			}
		}break;
	case PT_C_M_REQ_MYHOME_POST_INVITATION_CARD:
		{
			PgAction_ReqMyHomePostInvitationCard kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_MYHOME_INVITATION_CARD:
		{
			BM::Stream kPacket(PT_M_I_REQ_MYHOME_INVITATION_CARD);
			kPacket.Push(pkUnit->GetID());
			SendToItem(GroundKey(),kPacket);
		}break;
	case PT_C_M_REQ_MYHOME_PAY_TEX:
		{
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				SHOMEADDR const & kAddr = pkUser->HomeAddr();
				BM::Stream kPacket(PT_M_I_REQ_MYHOME_PAY_TEX);
				kPacket.Push(pkUser->GetID());
				kPacket.Push(kAddr.StreetNo());
				kPacket.Push(kAddr.HouseNo());
				SendToItem(GroundKey(),kPacket);
			}
		}break;
	case PT_C_M_REQ_MYHOME_INFO:
		{
			BM::Stream kPacket(PT_M_I_REQ_MYHOME_INFO);
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToItem(GroundKey(),kPacket);
		}break;
	case PT_C_M_REQ_MYHOME_ENTER:
		{
			if( 0 == (GetAttr() & GATTR_FLAG_MYHOME) )
			{//내가 있는 지역이 마이홈이면, 마이홈 이동 허용
				if(0 != (GetAttr() & GATTR_FLAG_CANT_HOMEWARP))
				{
					BM::Stream kPacket(PT_M_C_ANS_MYHOME_ENTER);
					kPacket.Push(E_MYHOME_CHECK_FRIENDLIST);
					pkUnit->Send(kPacket);
					break;
				}
			}

			if( PgGroundUtil::SUST_HardCoreDungeonVote == GetSpecStatus(pkUnit->GetID()) )
			{
				pkUnit->SendWarnMessage( 402204 );// 하드코어 던젼 투표중에는 이동 금지
			}
			else
			{
				short siStreetNo = 0;
				int iHouseNo = 0;

				pkNfy->Pop(siStreetNo);
				pkNfy->Pop(iHouseNo);

				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if(pkPlayer)
				{
					// 원정대 로비일 경우에는 원정대 탈퇴 시킨 후에 마이홈에 가도록 함.
					// 원정대장이 이동 요청하는 경우에는 원정대 해산.
					if( pkPlayer->HaveExpedition() )
					{
						BM::GUID MasterGuid;
						m_kLocalPartyMgr.GetExpeditionMasterGuid(pkPlayer->ExpeditionGuid(), MasterGuid);

						if( pkPlayer->GetID() == MasterGuid )
						{ // 원정대장인 경우는 원정대 해산.
							BM::Stream Packet(PT_C_N_REQ_DISPERSE_EXPEDITION);
							Packet.Push(pkPlayer->GetID());
							SendToGlobalPartyMgr(Packet);
						}
						else	// 아니면 탈퇴.
						{
							BM::Stream Packet(PT_C_N_REQ_LEAVE_EXPEDITION);
							Packet.Push(pkPlayer->ExpeditionGuid());
							Packet.Push(pkPlayer->GetID());
							SendToGlobalPartyMgr(Packet);
						}
					}

					BM::Stream kPacket(PT_M_I_REQ_MYHOME_ENTER);
					kPacket.Push(pkPlayer->GetID());
					kPacket.Push(siStreetNo);
					kPacket.Push(iHouseNo);
					SendToItem(GroundKey(),kPacket);
				}
			}
		}break;
	case PT_C_M_REQ_ENTER_SIDE_JOB:
		{
			short siStreetNo = 0;
			int iHouseNo = 0;

			pkNfy->Pop(siStreetNo);
			pkNfy->Pop(iHouseNo);

			PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkPlayer)
			{
				BM::Stream kPacket(PT_M_I_REQ_ENTER_SIDE_JOB);
				kPacket.Push(pkPlayer->GetID());
				kPacket.Push(siStreetNo);
				kPacket.Push(iHouseNo);
				SendToItem(GroundKey(),kPacket);
			}
		}break;
	case PT_C_M_REQ_MYHOME_EXIT:
		{
			BM::GUID kHomeGuid;
			pkNfy->Pop(kHomeGuid);
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				CUnit * pkHome = GetUnit(kHomeGuid);
				if(pkHome)
				{
					PgAction_ReqHomeExit kAction(this,GroundKey(),*pkNfy);
					kAction.DoAction(pkUnit,pkHome);
				}
			}		
		}break;
	case PT_C_M_REQ_HOMETOWN_INFO:
		{
			BM::Stream kPacket(PT_M_I_REQ_HOMETOWN_INFO);
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToItem(GroundKey(),kPacket);
		}break;
	case PT_C_M_REQ_HOMETOWN_ENTER:
		{
			PgAction_ReqHometownEnter kAction(this,GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_HOMETOWN_EXIT:
		{
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);

			if(pkUser)
			{
				SReqMapMove_MT kRMM(MMET_BackToChannel);
				kRMM.pt3TargetPos = pkUser->GetRecentPos(GATTR_DEFAULT);
				pkUser->GetRecentGround( kRMM.kTargetKey, GATTR_DEFAULT );
				PgReqMapMove kMapMove( this, kRMM, NULL );
				if ( kMapMove.Add( pkUser ) )
				{
					kMapMove.DoAction();
				}
			}
		}break;
	case PT_C_M_REQ_HOME_EQUIP:
		{
			BM::GUID kHomeGuid;
			pkNfy->Pop(kHomeGuid);
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				CUnit * pkHome = GetUnit(kHomeGuid);
				if(pkHome)
				{
					PgAction_ReqHomeEquipItem kAction(GroundKey(),*pkNfy);
					kAction.DoAction(pkUnit,pkHome);
				}
			}		
		}break;
	case PT_C_M_REQ_HOME_UNEQUIP:
		{
			BM::GUID kHomeGuid;
			pkNfy->Pop(kHomeGuid);
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				CUnit * pkHome = GetUnit(kHomeGuid);
				if(pkHome)
				{
					PgAction_ReqHomeUnEquipItem kAction(GroundKey(),*pkNfy);
					kAction.DoAction(pkUnit,pkHome);
				}
			}		
		}break;
	case PT_C_M_REQ_HOME_VISITLOG_ADD:
		{
			BM::GUID kHomeGuid;
			pkNfy->Pop(kHomeGuid);
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				PgMyHome * pkHome = dynamic_cast<PgMyHome*>(GetUnit(kHomeGuid));
				if(pkHome)
				{
					if(MAS_NOT_BIDDING != pkHome->GetAbil(AT_MYHOME_STATE))
					{
						BM::Stream kPacket(PT_M_C_ANS_HOME_VISITLOG_ADD);
						kPacket.Push(E_MYHOME_NOT_HAVE_AUTHORITY);
						pkUser->Send(kPacket);
					}
					else
					{
						BM::Stream kPacket(PT_M_I_REQ_HOME_VISITLOG_ADD);
						kPacket.Push(pkUnit->GetID());
						kPacket.Push(pkHome->OwnerGuid());
						kPacket.Push(pkUser->GetMemberGUID());
						kPacket.Push(*pkNfy);
						SendToItem(GroundKey(),kPacket);
					}
				}
				else
				{
					BM::Stream kPacket(PT_M_C_ANS_HOME_VISITLOG_ADD);
					kPacket.Push(E_MYHOME_NOT_FOUND);
					pkUser->Send(kPacket);
				}
			}
		}break;
	case PT_C_M_REQ_HOME_VISITLOG_DELETE:
		{
			BM::GUID kHomeGuid;
			pkNfy->Pop(kHomeGuid);
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				BM::Stream kPacket(PT_M_C_ANS_HOME_VISITLOG_DELETE);
				PgMyHome * pkHome = dynamic_cast<PgMyHome*>(GetUnit(kHomeGuid));
				if(pkHome)
				{
					BM::Stream kPacket(PT_M_I_REQ_HOME_VISITLOG_DELETE);
					kPacket.Push(pkUnit->GetID());
					kPacket.Push(*pkNfy);
					SendToItem(GroundKey(),kPacket);
				}
				else
				{
					kPacket.Push(E_MYHOME_NOT_FOUND);
					pkUser->Send(kPacket);
				}
			}
		}break;
	case PT_C_M_REQ_HOME_VISITLOG_LIST:
		{
			BM::GUID kHomeGuid;
			pkNfy->Pop(kHomeGuid);
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				PgMyHome * pkHome = dynamic_cast<PgMyHome*>(GetUnit(kHomeGuid));
				if(pkHome)
				{
					BM::Stream kPacket(PT_M_I_REQ_HOME_VISITLOG_LIST);
					kPacket.Push(pkUnit->GetID());
					kPacket.Push(pkHome->OwnerGuid());
					kPacket.Push(*pkNfy);
					SendToItem(GroundKey(),kPacket);
				}
			}
		}break;
	case PT_C_M_REQ_HOME_USE_ITEM_EFFECT:
		{
			BM::GUID kHomeGuid;
			pkNfy->Pop(kHomeGuid);
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkUser)
			{
				PgMyHome * pkHome = dynamic_cast<PgMyHome*>(GetUnit(kHomeGuid));
				if(pkHome)
				{
					PgAction_ReqHomeUseItemEffect kAction(GroundKey(),*pkNfy);
					kAction.DoAction(pkUser,pkHome);
				}
			}
		}break;
	case PT_C_M_REQ_HOME_VISITFLAG_MODIFY:
		{
			BM::GUID kHomeGuid;
			pkNfy->Pop(kHomeGuid);
			PgMyHome * pkHome = dynamic_cast<PgMyHome*>(GetUnit(kHomeGuid));
			if(pkHome)
			{
				PgAction_ReqMyHomeVisitFlag kAction(GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,pkHome);
			}
		}break;
	case PT_C_M_REQ_MYHOME_BIDDING:
		{
			PgAction_ReqMyHomeBidding kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_MYHOME_AUCTION_REG:
		{
			PgAction_ReqMyHomeAuctionReg kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_MYHOME_AUCTION_UNREG:
		{
			PgAction_ReqMyHomeAuctionUnreg kAction(GroundKey(),*pkNfy);
			kAction.DoAction(pkUnit,NULL);
		}break;
	// 오픈 마켓
	case PT_C_M_UM_REQ_MARKET_ENTER:
		{	
			int const iUseCallMarket = pkUnit->GetAbil(AT_CALL_MARKET);
			if(iUseCallMarket || PgGroundUtil::IsVillageGround(GetAttr()) ) // 입장/퇴장은 그라운드 체크만
			{
				if( PgGroundUtil::SUST_OpenMarket == EnterSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_OpenMarket) )
				{
					FakeRemoveUnit( pkUnit, FRT_HIDE_AND_NO_MSG );

					PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
					if(pkUser)
					{
						{// 이패킷을 받아야 클라이언트에서 오픈마켓을 띄운다.
							BM::Stream kAnsPacket( PT_M_C_UM_ANS_MARKET_ENTER );
							pkUser->Send( kAnsPacket );
						}

						BM::Stream kPacket(PT_M_I_UM_REQ_MARKET_ENTER);
						kPacket.Push(pkUser->GetID());
						kPacket.Push(pkUser->GetMemberGUID());
						return SendToItem(GroundKey(),kPacket);							
					}
				}
			}

		}break;
	case PT_C_M_UM_REQ_MARKET_EXIT:
		{
			pkUnit->SetAbil(AT_CALL_MARKET,0,true);

			if(LeaveSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_OpenMarket) )
			{
				FakeAddUnit(pkUnit);
			}
		}break;
	case PT_C_M_UM_REQ_ARTICLE_REG:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_ARTICLE_REG), __LINE__) )
			{
				PgUMReqArticleReg kReqReg(GroundKey(),*pkNfy);
				return kReqReg.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_UM_REQ_MARKET_CLOSE:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_MARKET_CLOSE), __LINE__) )
			{
				BM::Stream kPacket(PT_M_I_UM_REQ_MARKET_CLOSE);
				kPacket.Push(pkUnit->GetID());
				return SendToItem(GroundKey(),kPacket);
			}
		}break;
	case PT_C_M_UM_REQ_BEST_MARKET_LIST:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_BEST_MARKET_LIST), __LINE__) )
			{
				BM::Stream kPacket(PT_M_I_UM_REQ_BEST_MARKET_LIST);
				kPacket.Push(pkUnit->GetID());
				return SendToItem(GroundKey(),kPacket);
			}
		}break;
	case PT_C_M_UM_REQ_ARTICLE_DEREG:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_ARTICLE_DEREG), __LINE__) )
			{
				PgUMReqArticleDereg kReqDereg(GroundKey(),*pkNfy);
				return kReqDereg.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_UM_REQ_MARKET_QUERY:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_MARKET_QUERY), __LINE__) )
			{
				PgUMReqArticleQuery kReqQuery(GroundKey(),*pkNfy);
				return kReqQuery.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_UM_REQ_ARTICLE_BUY:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) 
				|| PgGroundUtil::IsableVendor(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_ARTICLE_BUY), __LINE__)
				|| PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_ARTICLE_BUY), __LINE__) )
			{
				PgUMReqArticleBuy kReqBuy(GroundKey(),*pkNfy);
				return kReqBuy.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_UM_REQ_DEALINGS_READ:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_DEALINGS_READ), __LINE__) )
			{
				PgUMReqDealingsRead kReqRead(GroundKey(),*pkNfy);
				return kReqRead.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_UM_REQ_MINIMUM_COST_QUERY:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_MINIMUM_COST_QUERY), __LINE__) )
			{
				PgUMReqQueryMinimumCost kReqCost(GroundKey(),*pkNfy);
				return kReqCost.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_UM_REQ_MY_MARKET_QUERY:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_MY_MARKET_QUERY), __LINE__) )
			{
				PgUMReqMyMarketQuery kReqMyArticle(GroundKey(),*pkNfy);
				return kReqMyArticle.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_UM_REQ_MARKET_OPEN:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_MARKET_OPEN), __LINE__) )
			{
				PgUMReqMarketOpen kMarketOpen(GroundKey(),*pkNfy);
				return kMarketOpen.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_UM_REQ_USE_MARKET_MODIFY_ITEM:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_USE_MARKET_MODIFY_ITEM), __LINE__) )
			{
				PgUMReqUseMarketModifyItem kAction(GroundKey(),*pkNfy);
				return kAction.DoAction(pkUnit, NULL);
			}
		}break;
	case PT_C_M_UM_REQ_MARKET_ARTICLE_QUERY:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_MARKET_ARTICLE_QUERY), __LINE__) )
			{
				BM::Stream kPacket(PT_M_I_UM_REQ_MARKET_ARTICLE_QUERY);
				kPacket.Push(pkUnit->GetID());
				kPacket.Push(*pkNfy);
				return SendToItem(GroundKey(),kPacket);
			}
		}break;
	case PT_C_M_UM_REQ_MARKET_MODIFY_STATE:
		{
			if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableOpenMarket(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_MARKET_MODIFY_STATE), __LINE__) )
			{
				BM::Stream kPacket(PT_M_I_UM_REQ_MARKET_MODIFY_STATE);
				kPacket.Push(pkUnit->GetID());
				kPacket.Push(*pkNfy);
				return SendToItem(GroundKey(),kPacket);
			}
		}break;
	// 오픈 마켓 끝
	//노점
	case PT_C_M_UM_REQ_MY_VENDOR_QUERY:
		{
			PgUMReqMyVendorQuery kReqVendorArticle(GroundKey(),*pkNfy);
			if(true == kReqVendorArticle.DoAction(pkUnit,NULL) )
			{
				return true;
			}
		}break;
	case PT_C_M_REQ_VENDOR_CREATE:
		{
			int const iUseCallMarket = pkUnit->GetAbil(AT_CALL_MARKET);
			if(iUseCallMarket || PgGroundUtil::IsVillageGround(GetAttr()) || PgGroundUtil::IsDefaultGround(GetAttr()) ) // 입장/퇴장은 그라운드 체크만
			{
				if( PgGroundUtil::SUST_Vendor == EnterSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_Vendor) )
				{//노점에 들어간 상태이되, 기존 오픈마켓처럼 유닛 사라지게는 하지 않는다.
					if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableVendor(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_MY_VENDOR_QUERY), __LINE__) )
					{
						BM::GUID kOwnerGuid;
						pkNfy->Pop(kOwnerGuid);
						std::wstring VendorTitle;
						pkNfy->Pop(VendorTitle);
						if(pkUnit->GetID() != kOwnerGuid)
						{
							LeaveSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_Vendor);
						}
						PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
						if(pkPlayer)
						{
							if( true == RecvReqVendorCreate(pkUnit, kOwnerGuid, VendorTitle) )
							{
								pkPlayer->OpenVendor(true);
								pkPlayer->VendorTitle(VendorTitle);
								pkPlayer->VendorGuid(pkPlayer->GetID());

								BM::Stream kPacket(PT_M_C_NFY_VENDOR_STATE);
								kPacket.Push(true);
								kPacket.Push(pkPlayer->GetID());
								kPacket.Push(pkPlayer->OpenVendor());
								kPacket.Push(pkPlayer->VendorTitle());
								kPacket.Push(pkPlayer->VendorGuid());
								Broadcast(kPacket);
							}
							else
							{
								LeaveSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_Vendor);
							}
						}
					}
					else
					{
						LeaveSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_Vendor);
					}
				}
			}
		}break;
	case PT_C_M_REQ_VENDOR_DELETE:
		{
			BM::GUID kOwnerGuid;
			pkNfy->Pop(kOwnerGuid);
			if(pkUnit->GetID() == kOwnerGuid)
			{
				if(true == RecvReqVendorDelete(pkUnit, kOwnerGuid) )
				{
					PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
					if(pkPlayer)
					{
						pkPlayer->OpenVendor(false);
						std::wstring kTemp;
						pkPlayer->VendorTitle(kTemp);
						pkPlayer->VendorGuid(BM::GUID::NullData());

						BM::Stream kPacket(PT_M_C_NFY_VENDOR_STATE);
						kPacket.Push(true);
						kPacket.Push(pkPlayer->GetID());
						kPacket.Push(pkPlayer->OpenVendor());
						kPacket.Push(pkPlayer->VendorTitle());
						kPacket.Push(pkPlayer->VendorGuid());
						Broadcast(kPacket);
					}
				}
			}
		}break;
	case PT_C_M_REQ_VENDOR_RENAME:
		{
			BM::GUID kOwnerGuid;
			pkNfy->Pop(kOwnerGuid);
			std::wstring VendorTitle;
			pkNfy->Pop(VendorTitle);

			if(pkUnit->GetID() == kOwnerGuid)
			{
				if( true == RecvReqVendorRename(pkUnit, kOwnerGuid, VendorTitle) )
				{
					PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
					if(pkPlayer)
					{
						pkPlayer->OpenVendor(true);
						pkPlayer->VendorTitle(VendorTitle);
						pkPlayer->VendorGuid(pkPlayer->GetID());

						BM::Stream kPacket(PT_M_C_NFY_VENDOR_STATE);
						kPacket.Push(true);
						kPacket.Push(pkPlayer->GetID());
						kPacket.Push(pkPlayer->OpenVendor());
						kPacket.Push(pkPlayer->VendorTitle());
						kPacket.Push(pkPlayer->VendorGuid());
						Broadcast(kPacket);
					}
				}
			}
		}break;
	case PT_C_M_REQ_VENDOR_STATE:
		{
			BM::GUID kOwnerGuid;
			pkNfy->Pop(kOwnerGuid);
			
			BM::Stream kPacket;
			if( true == RecvReqReadToPacketVendorName(pkUnit, kOwnerGuid, kPacket) )
			{
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_UM_REQ_VENDOR_ENTER:
		{
			int const iUseCallMarket = pkUnit->GetAbil(AT_CALL_MARKET);
			if(iUseCallMarket || PgGroundUtil::IsVillageGround(GetAttr()) || PgGroundUtil::IsDefaultGround(GetAttr()) ) // 입장/퇴장은 그라운드 체크만
			{
				if( PgGroundUtil::SUST_Vendor == EnterSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_Vendor) )
				{//노점에 들어간 상태이되, 기존 오픈마켓처럼 유닛 사라지게는 하지 않는다.
					if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableVendor(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_VENDOR_ENTER), __LINE__) )
					{
						BM::GUID kOwnerGuid;
						pkNfy->Pop(kOwnerGuid);
						PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
						if(pkPlayer)
						{
							pkPlayer->VendorGuid(kOwnerGuid);
						}
						BM::Stream kPacket( PT_M_I_UM_REQ_VENDOR_ENTER);
						kPacket.Push( pkUnit->GetID() );
						kPacket.Push( kOwnerGuid );	
						return SendToItem(GroundKey(), kPacket);
					}
					else
					{
						LeaveSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_Vendor);
					}
				}
			}
		}break;
	case PT_C_M_UM_REQ_VENDOR_REFRESH_QUERY:
		{
			int const iUseCallMarket = pkUnit->GetAbil(AT_CALL_MARKET);
			if(iUseCallMarket || PgGroundUtil::IsVillageGround(GetAttr()) || PgGroundUtil::IsDefaultGround(GetAttr()) ) // 입장/퇴장은 그라운드 체크만
			{
				if( PgGroundUtil::SUST_Vendor == EnterSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_Vendor) )
				{//노점에 들어간 상태이되, 기존 오픈마켓처럼 유닛 사라지게는 하지 않는다.
					if(pkUnit->GetAbil(AT_CALL_MARKET) || PgGroundUtil::IsableVendor(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_UM_REQ_VENDOR_ENTER), __LINE__) )
					{
						BM::Stream kPacket( PT_M_I_UM_REQ_VENDOR_REFRESH_QUERY);
						kPacket.Push( pkUnit->GetID() );
						kPacket.Push(*pkNfy);	
						return SendToItem(GroundKey(), kPacket);
					}
					else
					{
						LeaveSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_Vendor);
					}
				}
			}
		}break;
	case PT_C_M_UM_REQ_VENDOR_EXIT:
		{
			pkUnit->SetAbil(AT_CALL_MARKET,0,true);
			LeaveSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_Vendor);
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkPlayer)
			{
				pkPlayer->OpenVendor(false);
				std::wstring kTemp;
				pkPlayer->VendorTitle(kTemp);
				pkPlayer->VendorGuid(BM::GUID::NullData());
			}
		}break;
	//노점 끝
	case PT_C_M_CS_REQ_SIMPLE_ENTER_CASHSHOP:
		{
			PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if ( pkPlayer )
			{
				if( PgGroundUtil::SUST_CashShop == EnterSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_CashShop) )
				{
					pkPlayer->CashShopGuid(BM::GUID::Create());	// 상점 입장시 마다 다른 GUID를 생성 한다. 만약 이전 정보가 남아서 엉뚱한 결과가 나오지 않게 하자.
					BM::Stream kPacket( PT_M_I_CS_REQ_SIMPLE_ENTER_CASHSHOP, pkPlayer->GetID() );
					kPacket.Push(pkPlayer->CashShopGuid());
					kPacket.Push(*pkNfy);
					return SendToItem( GroundKey(),kPacket );
				}
			}
		}break;
	case PT_C_M_CS_REQ_SIMPLE_EXIT_CASHSHOP:
		{
			if( true == LeaveSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_CashShop) )
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if ( pkPlayer )
				{
					BM::Stream kPacket( PT_M_I_CS_REQ_EXIT_CASHSHOP, pkPlayer->GetID() );
					kPacket.Push(pkPlayer->CashShopGuid());
					kPacket.Push(*pkNfy);
					return SendToItem(GroundKey(),kPacket);
				}
			}
		}break;
	case PT_C_M_CS_REQ_SIMPLE_BUY_ARTICLE:
		{
			if( PgGroundUtil::SUST_CashShop == GetSpecStatus(pkUnit->GetID()))
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if ( pkPlayer )
				{
					BM::Stream kPacket( PT_M_I_CS_REQ_BUY_ARTICLE, pkPlayer->GetID() );
					kPacket.Push(*pkNfy);
					kPacket.Push(pkPlayer->CashShopGuid());
					return SendToItem(GroundKey(),kPacket);
				}
			}
		}break;
	case PT_C_M_CS_REQ_SIMPLE_ADD_TIMELIMIT:
		{
			if(PgGroundUtil::SUST_CashShop == GetSpecStatus(pkUnit->GetID()))
			{
				PgAction_CashItem_Add_TimeLimit kAction(GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	// 캐쉬 샵
	case PT_C_M_CS_REQ_ENTER_CASHSHOP:
		{
			//if( g_kLocal.IsServiceRegion( LOCAL_MGR::NC_KOREA ) )
			//{ //한국은 캐쉬샵 이용 불가
			//	break;
			//}

			bool const bCantEnterCashShopGround = PgGroundUtil::IsIndunGround(GetAttr()) || PgGroundUtil::IsBSGround(GetAttr());
			if( !bCantEnterCashShopGround ) // 입장/퇴장은 그라운드 체크만 한다.
			{
				bool bPass = true;
				switch( GetAttr() )
				{
				case GATTR_BATTLESQUARE:
					{
						pkUnit->SendWarnMessage(1935);
						bPass = false;
					}break;
				}
				if( bPass )
				{
					PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
					if ( pkPlayer )
					{
						if( PgGroundUtil::SUST_CashShop == EnterSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_CashShop) )
						{
							FakeRemoveUnit( pkUnit, FRT_HIDE_AND_NO_MSG );

							pkPlayer->CashShopGuid(BM::GUID::Create());	// 상점 입장시 마다 다른 GUID를 생성 한다. 만약 이전 정보가 남아서 엉뚱한 결과가 나오지 않게 하자.

							BM::Stream kPacket( PT_M_I_CS_REQ_ENTER_CASHSHOP, pkPlayer->GetID() );
							kPacket.Push(pkPlayer->CashShopGuid());
							kPacket.Push(*pkNfy);
							return SendToItem( GroundKey(),kPacket );
						}
					}
				}
			}
		}break;
	case PT_C_M_CS_REQ_EXIT_CASHSHOP:
		{
			bool const bCantEnterCashShopGround = PgGroundUtil::IsIndunGround(GetAttr()) || PgGroundUtil::IsBSGround(GetAttr());
			if( !bCantEnterCashShopGround ) // 입장/퇴장은 그라운드 체크만 한다.
			{
				if( LeaveSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_CashShop) )
				{
					PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
					if ( pkPlayer )
					{
						FakeAddUnit(pkUnit);

						BM::Stream kPacket( PT_M_I_CS_REQ_EXIT_CASHSHOP, pkPlayer->GetID() );
						kPacket.Push(pkPlayer->CashShopGuid());
						kPacket.Push(*pkNfy);
						return SendToItem(GroundKey(),kPacket);
					}
				}
			}
		}break;
	case PT_C_M_CS_REQ_RECV_GIFT:
		{
			if( PgGroundUtil::IsableCashshop(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_CS_REQ_RECV_GIFT), __LINE__) )
			{
				BM::Stream kPacket(PT_M_I_CS_REQ_RECV_GIFT);
				kPacket.Push(pkUnit->GetID());
				kPacket.Push(*pkNfy);
				return SendToItem(GroundKey(),kPacket);
			}
		}break;
	case PT_C_M_CS_REQ_MODIFY_VISABLE_RANK:
		{
			if( PgGroundUtil::IsableCashshop(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_CS_REQ_MODIFY_VISABLE_RANK), __LINE__) )
			{
				BM::Stream kPacket(PT_M_I_CS_REQ_MODIFY_VISABLE_RANK);
				kPacket.Push(pkUnit->GetID());
				kPacket.Push(*pkNfy);
				return SendToItem(GroundKey(),kPacket);
			}
		}break;
	case PT_C_M_CS_REQ_SEND_GIFT:
		{
			if( PgGroundUtil::IsableCashshop(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_CS_REQ_SEND_GIFT), __LINE__) )
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if ( pkPlayer )
				{
					BM::Stream kPacket( PT_M_I_CS_REQ_SEND_GIFT, pkPlayer->GetID() );
					kPacket.Push(pkPlayer->Name());
					kPacket.Push(*pkNfy);
					kPacket.Push(pkPlayer->CashShopGuid());
					return SendToItem(GroundKey(),kPacket);
				}	
			}
		}break;
	case PT_C_M_CS_REQ_ADD_TIMELIMIT:
		{
			if( PgGroundUtil::IsableCashshop(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_CS_REQ_ADD_TIMELIMIT), __LINE__) )
			{
				PgAction_CashItem_Add_TimeLimit kAction(GroundKey(),*pkNfy);
				kAction.DoAction(pkUnit,NULL);
			}
		}break;
	case PT_C_M_CS_REQ_LAST_RECVED_GIFT:
		{
			BM::Stream kPacket(PT_M_I_CS_REQ_LAST_RECVED_GIFT);
			kPacket.Push(pkUnit->GetID());
			SendToItem(GroundKey(),kPacket);
		}break;
	case PT_C_M_CS_REQ_BUY_ARTICLE:
		{
			if( PgGroundUtil::IsableCashshop(pkUnit, GetAttr(), GetGroundNo(), GetSpecStatus(pkUnit->GetID()), __F_P__(PT_C_M_CS_REQ_BUY_ARTICLE), __LINE__) )
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if ( pkPlayer )
				{
					BM::Stream kPacket( PT_M_I_CS_REQ_BUY_ARTICLE, pkPlayer->GetID() );
					kPacket.Push(*pkNfy);
					kPacket.Push(pkPlayer->CashShopGuid());
					return SendToItem(GroundKey(),kPacket);
				}
			}
		}break;
	// 캐쉬샵 끝

	case PT_C_M_REQ_SYSTEM_INVENTORY_REMOVE:
		{
			PgAction_SysItemRemove kModify(GroundKey(),*pkNfy);
			return kModify.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_REQ_SYSTEM_INVENTORY_RECV:
		{
			PgAction_Sys2Inv kSys2Inv(GroundKey(),*pkNfy);
			return kSys2Inv.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_POST_REQ_MAIL_MODIFY:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop( kNpcGuid );
			if( kNpcGuid.IsNull() )
			{// 미니맵의 UI를 열어서 우편을 사용한 경우
				PgPostReqMailModify kReqMailModify(GroundKey(),*pkNfy);
				return kReqMailModify.DoAction(pkUnit,NULL);
			}
			else
			{// NPC를 클릭해서 사용한 경우
				CUnit* pkNpcUnit = GetUnit(kNpcGuid);
				if(pkNpcUnit)
				{
					switch(pkNpcUnit->UnitType())
					{
					case UT_MYHOME:
						{
							if(true == PgGroundUtil::IsMyHomeHasFuctionItem(pkUnit,pkNpcUnit,UICT_HOME_POSTBOX))
							{
								PgPostReqMailModify kReqMailModify(GroundKey(),*pkNfy);
								return kReqMailModify.DoAction(pkUnit,NULL);
							}
						}break;
					default:
						{
							if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_Post, __F_P__(PT_C_M_POST_REQ_MAIL_MODIFY), __LINE__) )
							{
								PgPostReqMailModify kReqMailModify(GroundKey(),*pkNfy);
								return kReqMailModify.DoAction(pkUnit,NULL);
							}
						}break;
					}
				}
			}
		}break;
	case PT_C_M_POST_REQ_MAIL_RECV:
		{
			PgPostReqMailRecv kReqMailRecv(GroundKey(),*pkNfy);
			return kReqMailRecv.DoAction(pkUnit,NULL);
		}break;
	case PT_C_M_POST_REQ_MAIL_SEND:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop( kNpcGuid );
			if( kNpcGuid.IsNull() )
			{// 미니맵의 UI를 열어서 우편을 사용한 경우
				PgPostReqMailSend kReqMailSend(GroundKey(),*pkNfy);
				return kReqMailSend.DoAction(pkUnit,NULL);
			}
			else
			{// NPC를 클릭해서 사용한 경우
				CUnit* pkNpcUnit = GetUnit(kNpcGuid);
				if(pkNpcUnit)
				{
					switch(pkNpcUnit->UnitType())
					{
					case UT_MYHOME:
						{
							if(true == PgGroundUtil::IsMyHomeHasFuctionItem(pkUnit,pkNpcUnit,UICT_HOME_POSTBOX))
							{
								PgPostReqMailSend kReqMailSend(GroundKey(),*pkNfy);
								return kReqMailSend.DoAction(pkUnit,NULL);
							}
						}break;
					default:
						{
							if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_Post, __F_P__(PT_C_M_POST_REQ_MAIL_SEND), __LINE__) )
							{
								PgPostReqMailSend kReqMailSend(GroundKey(),*pkNfy);
								return kReqMailSend.DoAction(pkUnit,NULL);
							}
						}break;
					}
				}
			}
		}break;
	case PT_C_M_POST_REQ_MAIL_MIN:
		{
			BM::Stream kPacket(PT_M_I_POST_REQ_MAIL_MIN);
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			return SendToItem(GroundKey(),kPacket);
		}break;
	case PT_C_M_NFY_MAPLOADED:
		{
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if ( pkUser )
			{
				{// 맵 이동시 마다 플레이 타임 업적 세팅(분단위)
					WORD wTemp = 0;
					int iPlayTime = PgValueCtrl_PlayTime::GetValue(wTemp,pkUser);
					PgSyncClinetAchievementHandler<> kSA( AT_ACHIEVEMENT_FAMOUS, iPlayTime, pkUser->GroundKey() );
					kSA.DoAction( pkUser, NULL );
				}

				BYTE cMapMoveCause = 0;
				SGroundKey kGroundKey;
				pkNfy->Pop( cMapMoveCause );
				kGroundKey.ReadFromPacket(*pkNfy);

				if ( pkUser->IsMapLoading() )
				{
					INFO_LOG(BM::LOG_LV8, __FL__<<L"[PT_C_M_NFY_MAPLOADED] "<<pkUnit->Name()<<L" : RecvMapLoadComplete Ground["<<GetGroundNo()<<L"-"<<GroundKey().Guid()<<L"]" );
					this->SendMapLoadComplete(pkUser);

					BM::Stream kNfyPacket(PT_M_C_NFY_MAPMOVE_COMPLETE);
					kNfyPacket.Push( pkUnit->GetID() );
					kNfyPacket.Push( cMapMoveCause );
					pkUnit->Send(kNfyPacket, E_SENDTYPE_BROADALL);
				}
				else
				{
					INFO_LOG(BM::LOG_LV5, __FL__<<L"[PT_C_M_NFY_MAPLOADED] "<<pkUser->Name()<<L" : Not MapLoading User Ground["<<GetGroundNo()<<L"-"<<GroundKey().Guid()<<L"]" );
					this->SendMapLoadComplete(pkUser);

					BM::Stream kNfyPacket(PT_M_C_NFY_MAPMOVE_COMPLETE);
					kNfyPacket.Push( pkUnit->GetID() );
					kNfyPacket.Push( cMapMoveCause );
					pkUnit->Send(kNfyPacket, E_SENDTYPE_BROADALL);
				}
			}
		}break;
	case PT_C_M_NFY_POSITION:
		{
			POINT3 pt3Pos;
			pkNfy->Pop(pt3Pos);

			pkUnit->SetPos(pt3Pos);
			AdjustArea( pkUnit, true, pkUnit->IsCheckZoneTime(0) );
			//INFO_LOG(BM::LOG_LV9, _T("PT_C_M_NFY_POSITION x[%4.2f]/y[%4.2f]/z[%4.2f]"),pt3Pos.x,pt3Pos.y,pt3Pos.z);
			
		}break;	
	case PT_C_M_REQ_ENTIRE_SYNC_TIME:
		{
			BM::Stream kPacket(PT_M_C_NFY_ENTIRE_SYNC_TIME);
			kPacket.Push(BM::GetTime32());

			pkUnit->Send(kPacket);
		}break;
	case PT_C_M_REQ_ACTION2:
		{
			DWORD const dwRecvTime = g_kEventView.GetServerElapsedTime();
			SActionInfo kInfo;
			pkNfy->Pop(kInfo);
			
			CUnit *pkActionUnit = ::CheckRecvActionPacket( pkUnit, kInfo, this, PT_C_M_REQ_ACTION2 );
			if ( pkActionUnit )
			{
				if(UT_ENTITY == pkActionUnit->UnitType())
				{// 스킬 제한 이펙트가
					CUnit* pkCaller = NULL;
					if(pkUnit->GetID() == pkActionUnit->Caller())
					{
						pkCaller = pkUnit;
					}
					else
					{
						pkCaller = this->GetUnit(pkActionUnit->Caller());
					}
					if(pkCaller)
					{
						PgUnitEffectMgr& rkEffectMgr = pkCaller->GetEffectMgr();
						CEffect* pkEffect = NULL;
						ContEffectItor kItor;
						rkEffectMgr.GetFirstEffect(kItor);
						while ((pkEffect = rkEffectMgr.GetNextEffect(kItor)) != NULL)
						{
							if(pkEffect
								&& GlobalHelper::IsAdjustSkillEffect(pkEffect)
								&& NULL == pkActionUnit->GetEffect(pkEffect->GetEffectNo())
								)
							{// Caller에게 걸려있고, Entity에겐 걸려있지 않다면 똑같이 Entity에게도 적용해준다.
								SEffectCreateInfo kCreate;
								kCreate.eType = EFFECT_TYPE_NORMAL;
								kCreate.iEffectNum = pkEffect->GetEffectNo();
								kCreate.eOption = SEffectCreateInfo::ECreateOption_CallbyServer;
								pkActionUnit->AddEffect(kCreate);
							}
						}
					}
				}

				RecvReqAction2( pkActionUnit, kInfo, pkNfy );

				GroundHackUtil::AddLog(GroundKey(), pkActionUnit, kInfo.iActionID, dwRecvTime);
			}
		}break;
	case PT_C_M_REQ_TOGGLSKILL_OFF:
		{
			BM::GUID kGuid;
			int iSkillNo = 0;
			
			pkNfy->Pop(kGuid);
			pkNfy->Pop(iSkillNo);
			
			GET_DEF( CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
			if(pSkillDef)
			{
				if( 0 < pSkillDef->GetAbil(AT_WHEN_ACTION_END_THEN_TOGGLE_OFF) )
				{
					PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>( GetUnit(kGuid) );
					if(pkPlayer)
					{
						SActArg kArg;
						PgGroundUtil::SetActArgGround(kArg, this);
						g_kSkillAbilHandleMgr.SkillToggle(pkPlayer, iSkillNo, &kArg, false);
					}
				}
			}
		}break;
	case PT_C_M_REQ_UPDATE_DIRECTION:
		{
			DWORD const dwRecvTime = g_kEventView.GetServerElapsedTime();
			BYTE byDirection;
			DWORD dwActionTerm;
			POINT3BY ptDirection;
			POINT2BY ptPathNormal;
			POINT3 kCurPos;

			pkNfy->Pop(byDirection);
			pkNfy->Pop(ptDirection);
			pkNfy->Pop(ptPathNormal);
			pkNfy->Pop(dwActionTerm);
			pkNfy->Pop(kCurPos);

			if(pkUnit->UnitType() == UT_PLAYER)
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkUnit);
				pkPlayer->MoveDirection(ptDirection);
				pkPlayer->PathNormal(POINT3BY(ptPathNormal.x, ptPathNormal.y, 0));
				pkPlayer->SetPos(kCurPos);

				if(((byDirection & 0xF0) >> 4) != DIR_NONE)
				{
					pkPlayer->MoveDir(static_cast<Direction>((byDirection & 0xF0) >> 4));
				}
				if((byDirection & 0x0F) != DIR_NONE)
				{
					pkPlayer->FrontDirection(static_cast<Direction>(byDirection & 0x0F));
				}		

				//INFO_LOG(BM::LOG_LV9, _T("ptDirection [%d,%d,%d]"), (int)ptDirection.x, (int)ptDirection.y, (int)ptDirection.z);
			}

			BM::GUID kGuid(pkUnit->GetID());
			BM::Stream kPacket(PT_M_C_NFY_UPDATE_DIRECTION);
			kPacket.Push(kGuid);
			kPacket.Push(byDirection);
			kPacket.Push(dwActionTerm);
			kPacket.Push(kCurPos);

			SendToArea( &kPacket, pkUnit->LastAreaIndex(), pkUnit->GetID(), SYNC_TYPE_RECV_ADD, E_SENDTYPE_NONE );

			GroundHackUtil::AddLogChangeMoveDirection(GroundKey(), pkUnit, static_cast<Direction>((byDirection&0xf0)>>4), dwRecvTime);
		}break;
	case PT_C_M_REQ_ACTOR_SIMULATE:
		{
			bool bSimulate;
			POINT3 kNewPos;

			pkNfy->Pop(bSimulate);
			pkNfy->Pop(kNewPos);
			
			if(pkUnit->UnitType() == UT_PLAYER)
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkUnit);
				pkPlayer->SetPos(kNewPos);
				pkPlayer->DoSimulate(bSimulate);
			}
		}break;
	case PT_C_M_REQ_MISSION_TRIGGER_ACTION:
	case PT_C_M_REQ_TRIGGER_ACTION:
		{
			RecvTriggerAction( pkUnit, pkNfy );
		}break;
	case PT_C_M_REQ_MISSION_CHAOS_ACTION:
		{
			int iMissionNo = 0;

			pkNfy->Pop(iMissionNo);

			CONT_DEF_MISSION_ROOT const *pkMissionRoot = NULL;
			g_kTblDataMgr.GetContDef( pkMissionRoot );
			if( pkMissionRoot )
			{
				CONT_DEF_MISSION_ROOT::const_iterator itr = pkMissionRoot->find( iMissionNo );
				if ( itr != pkMissionRoot->end() )
				{
					if( pkUnit )
					{
						PgGTrigger_Mission pkTemp;
						pkTemp.Set(itr->second.iKey, iMissionNo);
						pkTemp.MissionEvent(pkUnit, this, pkNfy);
					}					
				}
			}
		}break;
	case PT_C_M_REQ_TRANSTOWER:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);

			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( true == PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_TransTower, __F_P__(PT_C_M_REQ_TRANSTOWER), __LINE__) )
			{
				PgGTrigger_TransTower::Event( pkUnit, this, pkNfy, kNpcGuid, static_cast<__int64>(pkNpcUnit->GetAbil(AT_CUSTOMDATA1)) );
			}
		}break;
	case PT_C_M_REQ_TRANSTOWER_BY_PET:
		{
			BM::GUID kPetGuid, kNpcGuid;
			pkNfy->Pop(kPetGuid);
			pkNfy->Pop(kNpcGuid);
			PgPet* pkPet = dynamic_cast<PgPet*>(GetUnit(kPetGuid));
			if(pkPet && pkPet->GetPetType() == EPET_TYPE_3)
			{
				PgGTrigger_TransTower::Event( pkUnit, this, pkNfy, kNpcGuid, 0, true );
			}
		}break;
	case PT_C_M_REQ_JOIN_EMPORIA:
	case PT_C_M_REG_JOIN_EMPORIA_MERCENARY:
		{			
			PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkUnit);
			if ( pkPlayer )
			{
				if ( pkPlayer->HaveParty() )
				{
					// 파티가 있으면 진입할 수 없습니다.
					INFO_LOG( BM::LOG_LV5, _T("[ReqJoinEmporia] FAILED ") << pkPlayer->Name() << _T("<") << pkPlayer->GetID() << _T("> Had Party") );
					pkPlayer->SendWarnMessage( 19013 );
				}
				else
				{
					SReqMapMove_MT kRMM(MMET_None);
					kRMM.nTargetPortal = 1;
					kRMM.kCasterKey = GroundKey();
					kRMM.kCasterSI = g_kProcessCfg.ServerIdentity();

					BM::Stream kCheckPacket( PT_M_N_REQ_MAP_MOVE_CHECK, pkPlayer->GetID() );

					kCheckPacket.Push(kRMM);
					kCheckPacket.Push(pkPlayer->GuildGuid());

					bool const bIsMercenary = ( wType == PT_C_M_REG_JOIN_EMPORIA_MERCENARY );
					kCheckPacket.Push( bIsMercenary );
					if ( true == bIsMercenary )
					{
						PgLimitClass kClassInfo( pkPlayer->GetAbil(AT_CLASS), static_cast<short>(pkPlayer->GetAbil(AT_LEVEL)) );
						kClassInfo.WriteToPacket( kCheckPacket );
					}

					kCheckPacket.Push(*pkNfy);
					::SendToRealmContents( PMET_EMPORIA, kCheckPacket );	
				}
			}
		}break;
	case PT_S_A_REQ_AUTOLOGIN:
		{
			SReqSwitchReserveMember kRSRM;
			kRSRM.ReadFromPacket( *pkNfy );
			
			if ( true == SaveUnit( pkUnit, &kRSRM, pkNfy ) )
			{
				// Release Unit을 해야 한다.
				ReleaseUnit( pkUnit, true );
			}
		}break;
	case PT_C_M_REQ_CHAT://(BroadCasting in zone)
		{
			RecvChat( dynamic_cast<PgPlayer*>(pkUnit), pkNfy );
		}break;
	case PT_C_M_REQ_CHAT_INPUTNOW://Chat Input Now...(BroadCasting in zone)
		{
			RecvChatInputState(pkUnit, pkNfy);
		}break;

		//>>Party
	/*case PT_C_N_REQ_PARTY_RENAME:
		{
			BM::Stream kPacket(wType);
			kPacket.Push(*pkNfy);
			SendToGlobalPartyMgr(kPacket);
		}break;*/
	case PT_C_N_ANS_JOIN_PARTYFIND:
		{
			BM::GUID kUserGuid;

			pkNfy->Pop(kUserGuid);

			BM::Stream kPacket(wType, kUserGuid);
			kPacket.Push(*pkNfy);
			SendToGlobalPartyMgr(kPacket);
		}break;
	case PT_C_N_REQ_JOIN_PARTY://Route packet to Contents	
	case PT_C_N_REQ_JOIN_PARTYFIND:
	case PT_C_N_ANS_JOIN_PARTY:
	case PT_C_N_REQ_LEAVE_PARTY:
	case PT_C_N_REQ_KICKOUT_PARTY_USER:
	case PT_C_N_REQ_PARTY_CHANGE_MASTER:
	case PT_C_N_REQ_PARTY_CHANGE_OPTION:
	case PT_C_N_REQ_CREATE_PARTY:
	case PT_C_N_REQ_PARTY_RENAME:
		{
			bool bPass = true;
			if( GATTR_BATTLESQUARE == GetAttr() )
			{
				switch( wType )
				{
				case PT_C_N_REQ_JOIN_PARTY:
					{
						BM::Stream kTempPacket(*pkNfy);
						BYTE byCmdType = 0;
						kTempPacket.Pop(byCmdType);
						switch( byCmdType )
						{
						case PCT_REQJOIN_CHARNAME:
							{
								std::wstring kCharName;
								kTempPacket.Pop( kCharName );

								if( NULL == GetPlayer(kCharName) )
								{
									bPass = false;
									pkUnit->SendWarnMessage(460061);
								}
							}break;
						default:
							{
							}break;
						}

					}break;
				default:
					{
						// none
					}break;
				}
			}
			if( PgGroundUtil::IsBSGround(GetAttr()) )
			{
				bPass = false;
				pkUnit->SendWarnMessage(460050);
			}

			if( bPass )
			{
				BM::Stream kPacket(wType, pkUnit->GetID());
				kPacket.Push(*pkNfy);
				switch(wType)
				{
				case PT_C_N_REQ_JOIN_PARTY:
				case PT_C_N_REQ_CREATE_PARTY:
					{
						BYTE byPartyRefuse = 0;
						byPartyRefuse |= (false==IsPartyBreakIn()) ? EPR_MAP : 0;
						byPartyRefuse |= m_bIsBossRoom ? EPR_BOSSROOM : 0;
						if(GKIND_MISSION == GetKind())
						{
							if(PgMissionGround * pkMissionGround = dynamic_cast<PgMissionGround*>(this))
							{
								byPartyRefuse |= pkMissionGround->IsBossStage() ? EPR_BOSSROOM : 0;
							}
						}

						kPacket.Push(byPartyRefuse);
					}break;
				}
				SendToGlobalPartyMgr(kPacket);
			}
		}break;
	case PT_C_M_REQ_PARTY_NAME:
		{
			BM::GUID kPartyGuid;

			pkNfy->Pop(kPartyGuid);

			BM::Stream kPacket;
			//bool const bFindParty = g_kLocalPartyMgr.WriteToPacketPartyName(kPartyGuid, kPacket);
			bool const bFindParty = m_kLocalPartyMgr.WriteToPacketPartyName(kPartyGuid, kPacket);			
			if( bFindParty )
			{
				pkUnit->Send(kPacket, E_SENDTYPE_BROADALL | E_SENDTYPE_MUSTSEND);
			}
		}break;

	case PT_C_M_REQ_PARTY_LIST:
		{
			BM::Stream kPacket(PT_C_M_REQ_PARTY_LIST);
			kPacket.Push(pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToContents(kPacket);
		}break;
	case PT_C_M_REQ_FIND_PARTY_USER_LIST:
		{
			BM::Stream kPacket(PT_C_M_REQ_FIND_PARTY_USER_LIST);
			kPacket.Push(*pkNfy);
			SendToGlobalPartyMgr(kPacket);
		}break;
	case PT_C_M_REQ_REGIST_PRIVATE_PARTY_FIND:
		{
			BM::Stream kPacket(PT_C_M_REQ_REGIST_PRIVATE_PARTY_FIND);
			kPacket.Push(*pkNfy);
			SendToGlobalPartyMgr(kPacket);
		}break;
	case PT_N_C_REQ_UNREGIST_PRIVATE:
		{
			BM::Stream SendPacket(PT_N_C_REQ_UNREGIST_PRIVATE);
			SendPacket.Push(pkUnit->GetID());
			SendPacket.Push(*pkNfy);
			SendToGlobalPartyMgr(SendPacket);
		}break;
	case PT_C_M_REQ_JOIN_PARTY_REFUSE:
		{
			bool bRefuse = false;
			pkNfy->Pop(bRefuse);

			PgGroundUtil::SendPartyMgr_Refuse(pkUnit->GetID(), EPR_CLIENT, bRefuse);
		}break;
		//<<Party

		//>>Expedition
	case PT_C_N_REQ_JOIN_EXPEDITION:
		{
			PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pPlayer)
			{
				if( false == pPlayer->HaveExpedition() )
				{		
					if( (GetAttr() & GKIND_EXPEDITION_LOBBY) )
					{	// 원정대 로비이면 채널이동이 필요없기 때문에 일반적인 원정대 가입 신청.
						BM::Stream Packet(PT_C_N_REQ_JOIN_EXPEDITION);
						Packet.Push(*pkNfy);
						Packet.Push((int)EJT_DEFAULT);	// 일반적인 원정대 가입 신청.
						SendToGlobalPartyMgr(Packet);
					}
					else
					{	// 원정대 로비가 아니라면 채널이동이 필요하다.
						BM::Stream Packet(wType);
						Packet.Push(*pkNfy);
						SendToExpeditionListMgr(Packet);
					}
				}
				else
				{
					BM::Stream Packet(PT_N_C_ANS_JOIN_EXPEDITION, PRC_Fail_Party);
					pkUnit->Send(Packet);
				}
			}
		}break;
	case PT_C_N_REQ_LEAVE_EXPEDITION:
		{
			BM::Stream Packet(PT_C_N_REQ_LEAVE_EXPEDITION);
			Packet.Push(*pkNfy);
			SendToGlobalPartyMgr(Packet);
		}break;
	case PT_C_N_REQ_DISPERSE_EXPEDITION:
		{
			BM::Stream Packet(PT_C_N_REQ_DISPERSE_EXPEDITION);
			if( 0 == (GetAttr() & GKIND_EXPEDITION_LOBBY) )
			{	// 원정대 던전에서는 해산 할 수 없다.
				BM::Stream FailPacket(PT_N_C_ANS_DISPERSE_EXPEDITION);
				FailPacket.Push(PRC_Fail);
				pkUnit->Send(FailPacket);
				break;
			}
			Packet.Push(*pkNfy);
			SendToGlobalPartyMgr(Packet);
		}break;
	case PT_C_N_REQ_INFO_EXPEDITION:
		{
			BM::GUID ExpeditionGuid;
			pkNfy->Pop(ExpeditionGuid);
			BM::Stream Packet;
			bool bFindExpedition = m_kLocalPartyMgr.WriteToPacketExpeditionName(ExpeditionGuid, Packet);
			if( bFindExpedition )
			{
				pkUnit->Send(Packet, E_SENDTYPE_BROADALL | E_SENDTYPE_MUSTSEND);
			}
		}break;
	case PT_C_N_REQ_CHANGEMASTER_EXPEDITION:
	case PT_C_N_REQ_RENAME_EXPEDITION:
	case PT_C_N_REQ_CHANGEOPTION_EXPEDITION:
	case PT_C_N_REQ_KICKOUT_EXPEDITION:
		{
			BM::Stream Packet(wType);
			Packet.Push(pkUnit->GetID());
			Packet.Push(*pkNfy);
			SendToGlobalPartyMgr(Packet);
		}break;
	case PT_C_N_REQ_LIST_EXPEDITION:
		{
			BM::Stream Packet(wType);
			Packet.Push(pkUnit->GetID());
			Packet.Push(*pkNfy);
			SendToExpeditionListMgr(Packet);
		}break;
		//<<Expedition

	case PT_C_M_REQ_SEARCH_PEOPLE_LIST:
		{			
			CONT_SEARCH_UNIT_INFO kUnitArray;

			g_kGroundUnitMgr.Get(pkUnit->GetID(), kUnitArray);

			BM::Stream kPacket(PT_M_C_ANS_SEARCH_PEOPLE_LIST);
			PU::TWriteArray_M(kPacket, kUnitArray);
			pkUnit->Send(kPacket);
		}break;
	case PT_C_M_REQ_EXIT_SAFE:
		{
			bool bIsShareSafe = false;
			pkNfy->Pop(bIsShareSafe);
			if(false == bIsShareSafe)
			{
				pkUnit->SetAbil(AT_CALL_SAFE,0,true);
			}
			else
			{
				pkUnit->SetAbil(AT_CALL_SHARE_SAFE,0,true);
			}
		}break;
	case PT_C_M_REQ_ITEM_CHANGE:
		{
			SItemPos kSourcePos;
			SItemPos kTargetPos;
			DWORD dwClientTime;
			BM::GUID kNpcGuid;
			pkNfy->Pop( kSourcePos );
			pkNfy->Pop( kTargetPos );
			pkNfy->Pop( dwClientTime );
			pkNfy->Pop( kNpcGuid );

			if ( pkUnit->IsUnitType(UT_PLAYER) )
			{
				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if ( pkPlayer )
				{
					DWORD dwServerTime = 0;
					if ( S_OK == CheckClientNotifyTime(pkPlayer, dwClientTime, dwServerTime) )
					{
						bool bCanChange = true;

						CUnit* pkNpcUnit = GetUnit(kNpcGuid);

						if( PgGroundUtil::IsNeedNpcTestChangeItemEvent(kSourcePos) || PgGroundUtil::IsNeedNpcTestChangeItemEvent(kTargetPos) )
						{
							if(	(false == PgGroundUtil::IsCallSafe(pkUnit,kSourcePos,kTargetPos)) && 
								(false == PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_InvenSafe, __F_P__(PT_C_M_REQ_ITEM_CHANGE), __LINE__)) &&
								(false == PgGroundUtil::IsMyHomeHasFuctionItem(pkUnit,pkNpcUnit,UICT_HOME_SAFEBOX)) )
							{
								bCanChange = false;
							}
						}

						if( bCanChange )
						{
							PgAction_MoveItem kAction(kSourcePos, kTargetPos, this, dwClientTime );
							kAction.DoAction( pkUnit, NULL );
						}
					}
					else
					{
						INFO_LOG(BM::LOG_LV5, __FL__ << _T("Player kicked, Cause[CDC_SuspectedSpeedHack], Name=") << pkPlayer->Name());
						// SpeedHack이 의심스러우니 강제접속 해제 시킨다.
						BM::Stream kDPacket( PT_A_S_NFY_USER_DISCONNECT, static_cast<BYTE>(CDC_SuspectedSpeedHack) );
						kDPacket.Push( pkPlayer->GetMemberGUID() );
						SendToServer( pkPlayer->GetSwitchServer(), kDPacket );
					}
				}
			}
		}break;
	case PT_C_M_REQ_TRIGGER:
		{
			int iType = 0;
			pkNfy->Pop(iType);
			RecvReqTrigger(pkUnit, iType, pkNfy);
		}break;
	case PT_C_M_REQ_QUESTSCROLL:
		{
			int iQuestID = 0;
			BM::GUID kNpcGuid;
			DWORD dwClientTime;

			pkNfy->Pop(iQuestID);
			pkNfy->Pop(kNpcGuid);
			pkNfy->Pop(dwClientTime);

			// 쿨타임 체크
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pkPlayer )
			{
				PgInventory *pkInven = pkPlayer->GetInven();
				if( !pkInven )
				{
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
					return false;
				}

				CONT_HAVE_ITEM_POS kContHaveItemPos; //아이템 번호 + (수량, 위치들)
				if( S_OK == pkInven->GetItems( IT_CONSUME, UICT_SCROLL_QUEST, kContHaveItemPos ) )
				{
					CONT_HAVE_ITEM_POS::const_iterator c_iter = kContHaveItemPos.begin();
					while( c_iter != kContHaveItemPos.end() )
					{
						CONT_HAVE_ITEM_POS::mapped_type kItemPos = c_iter->second;
						SItemPos kQuestScrollPos = (*kItemPos.kContItemPos.begin());

						GET_DEF(CItemDefMgr, kItemDefMgr);
						CItemDef const *pDef = kItemDefMgr.GetDef(c_iter->first);
						if( pDef )
						{
							if( iQuestID == pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1) )
							{
								if( pkInven->CheckCoolTime( c_iter->first, PgInventory::EICool_UseItem, dwClientTime ) )
								{
									ReqIntroQuest(pkUnit, iQuestID, kNpcGuid);
									break;
								}
								else
								{
									pkPlayer->SendWarnMessage(20028);
									LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
									return false;
								}
							}
						}

						++c_iter;
					}
				}
			}			
		}break;
		//>>Quest
	case PT_C_M_ANS_SHOWDIALOG://퀘스트 한개만
		{
			BM::GUID kSessionGuid;
			BM::GUID kObjectGuid;
			int iQuestID = 0;
			int iDialogID = 0;
			int iNextDialogID = 0;
			int iSelect1 = 0;
			int iSelect2 = 0;
			pkNfy->Pop(kSessionGuid);
			pkNfy->Pop(kObjectGuid);
			pkNfy->Pop(iQuestID);
			pkNfy->Pop(iDialogID);		//From
			pkNfy->Pop(iNextDialogID);	//Next
			pkNfy->Pop(iSelect1);		//User Select 1
			pkNfy->Pop(iSelect2);		//User Select 2

			PgQuestInfo const* pkQuestInfo = NULL;
			if( !g_kQuestMan.GetQuest(iQuestID, pkQuestInfo) )
			{
				break;
			}

			if( !m_kQuestTalkSession.SummitNextTalk(pkUnit->GetID(), kObjectGuid, kSessionGuid, iQuestID, iDialogID, iNextDialogID) )
			{
				break;
			}

			CUnit* pkNPC = GetNPC(kObjectGuid);
			if( !pkQuestInfo->IsCanRemoteComplete() )
			{
				if( !pkNPC )
				{
					break;
				}

				if( !PgGroundUtil::IsCanTalkableRange(pkUnit, kObjectGuid, pkNPC, GetGroundNo(), NMT_Quest, __F_P__(PT_C_M_ANS_SHOWDIALOG), __LINE__) )
				{
#ifndef _MDo_
					::NfyShowQuestDialog(pkUnit, kObjectGuid, QSDT_NormalDialog, iQuestID, QRDID_Close);
					pkUnit->SendWarnMessage(700136);
#endif
					break;
				}
			}

			PgAction_AnsQuestDialog kActionQuest(GroundKey(), kObjectGuid, iQuestID, iDialogID, iNextDialogID, iSelect1, iSelect2);
			kActionQuest.DoAction(pkUnit, pkNPC);
		}break;
	case PT_C_M_REQ_INTROQUEST://여러 퀘스트 중에 하나
		{
			int iQuestID = 0;
			BM::GUID kObjectID;
			bool bRemoteAccept = false;
			pkNfy->Pop(iQuestID);
			pkNfy->Pop(kObjectID);
			pkNfy->Pop(bRemoteAccept);

			if( true == bRemoteAccept )
			{
				ReqIntroQuest(pkUnit, iQuestID, kObjectID);
			}
			else
			{
				CUnit* pkNpc = GetNPC(kObjectID);
				if( pkNpc )
				{
					if( PgGroundUtil::IsCanTalkableRange(pkUnit, kObjectID, pkNpc, GetGroundNo(), NMT_Quest, __F_P__(PT_C_M_REQ_INTROQUEST), __LINE__) )
					{
						ReqIntroQuest(pkUnit, iQuestID, kObjectID);
					}
				}
			}
		}break;
	case PT_C_M_REQ_END_DAILYQUEST:
		{
			BM::GUID kNpcGuid;
			int iQuestID = 0;
			pkNfy->Pop(kNpcGuid);
			pkNfy->Pop(iQuestID);

			CUnit* pkNPC = GetNPC(kNpcGuid);
			if( pkNPC )
			{
				if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNPC, GetGroundNo(), NMT_Quest, __F_P__(PT_C_M_REQ_END_DAILYQUEST), __LINE__) )
				{
					PgAction_AnsQuestDialog kActionQuest(GroundKey(), kNpcGuid, iQuestID, QRDID_CanComplete, QRDID_COMPLETE_Start, 0, 0);
					kActionQuest.DoAction(pkUnit, pkNPC);
				}
			}
		}break;
	case PT_C_M_REQ_RANDOMQUEST_BUILD:
		{
			PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkUnit);
			if( pkPlayer )
			{
				if( RandomQuest::iRandomQuestMinLimitLevel <= pkPlayer->GetAbil(AT_LEVEL) )
				{
					PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
					if( pkMyQuest )
					{
						bool const bIsCanRebuildByTimeLimitItem = (RandomQuest::IsCanRebuild(pkMyQuest, QT_Random) && RandomQuest::IsHasRebuildItem(pkPlayer, QT_Random, UICT_REBUILD_RAND_QUEST));
						bool const bIsCanRebuildByConsumeItem = (RandomQuest::IsCanRebuild(pkMyQuest, QT_Random) && RandomQuest::IsHasRebuildItem(pkPlayer, QT_Random, UICT_REBUILD_RAND_QUEST_CONSUME));
						if( false == pkMyQuest->BuildedRandomQuest()
						||	bIsCanRebuildByTimeLimitItem
						||	bIsCanRebuildByConsumeItem )
						{
							CONT_PLAYER_MODIFY_ORDER kOrder;
							PgBase_Item kConsumeItem;
							SItemPos kConsumeItemPos;
							if( bIsCanRebuildByConsumeItem
							&&	RandomQuest::GetRebuildItem(pkPlayer, QT_Random, UICT_REBUILD_RAND_QUEST_CONSUME, kConsumeItem, kConsumeItemPos) )
							{
								kOrder.push_back( SPMO(IMET_MODIFY_COUNT|IMC_DEC_DUR_BY_USE, pkUnit->GetID(), SPMOD_Modify_Count(kConsumeItem, kConsumeItemPos, -1)) ); //1씩 감소.
							}
							kOrder.push_back( SPMO(IMET_BUILD_RANDOM_QUEST, pkUnit->GetID()) );
							EItemModifyParentEventType const eType = (bIsCanRebuildByTimeLimitItem||bIsCanRebuildByConsumeItem? IMEPT_QUEST_BUILD_RANDOM_BY_ITEM: IMEPT_QUEST_BUILD_RANDOM);
							PgAction_ReqModifyItem kItemModifyAction(eType, GroundKey(), kOrder);
							kItemModifyAction.DoAction(pkUnit, NULL);
						}
					}
				}
			}
		}break;
	case PT_C_M_REQ_WANTEDQUEST_BUILD:
		{
			PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkUnit);
			if( pkPlayer )
			{
				PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
				if( pkMyQuest )
				{
					if( false == pkMyQuest->BuildedWantedQuest() )
					{// 현상수배 퀘스트가 빌드가 안되있어야 새로 빌드한다.
						CONT_PLAYER_MODIFY_ORDER kOrder;
						kOrder.push_back( SPMO( IMET_BUILD_WANTED_QUEST, pkUnit->GetID() ) );
						PgAction_ReqModifyItem kItemModifyAction( IMEPT_QUEST, GroundKey(),kOrder );
						kItemModifyAction.DoAction( pkUnit, NULL );
					}
				}
			}
		}break;
	case PT_C_M_REQ_RANDOMTACTICSQUEST_BUILD:
		{
			PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkUnit);
			if( pkPlayer )
			{
				if( BM::GUID::IsNotNull(pkPlayer->GuildGuid())
				&&	RandomQuest::iRandomTacticsQuestMinLimitLevel <= pkPlayer->GetAbil(AT_LEVEL) )
				{
					PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
					if( pkMyQuest )
					{
						bool const bIsCanRebuildByTimeLimitItem = (RandomQuest::IsCanRebuild(pkMyQuest, QT_RandomTactics) && RandomQuest::IsHasRebuildItem(pkPlayer, QT_RandomTactics, UICT_REBUILD_RAND_QUEST));
						bool const bIsCanRebuildByConsumeItem = (RandomQuest::IsCanRebuild(pkMyQuest, QT_RandomTactics) && RandomQuest::IsHasRebuildItem(pkPlayer, QT_RandomTactics, UICT_REBUILD_RAND_QUEST_CONSUME));
						if( false == pkMyQuest->BuildedTacticsQuest()
						||	bIsCanRebuildByTimeLimitItem
						||	bIsCanRebuildByConsumeItem )
						{
							CONT_PLAYER_MODIFY_ORDER kOrder;
							PgBase_Item kConsumeItem;
							SItemPos kConsumeItemPos;
							if( bIsCanRebuildByConsumeItem
							&&	RandomQuest::GetRebuildItem(pkPlayer, QT_RandomTactics, UICT_REBUILD_RAND_QUEST_CONSUME, kConsumeItem, kConsumeItemPos) )
							{
								kOrder.push_back( SPMO(IMET_MODIFY_COUNT|IMC_DEC_DUR_BY_USE, pkUnit->GetID(), SPMOD_Modify_Count(kConsumeItem, kConsumeItemPos, -1)) ); //1씩 감소.
							}
							kOrder.push_back( SPMO(IMET_BUILD_TACTICS_QUEST, pkUnit->GetID()) );
							EItemModifyParentEventType const eType = (bIsCanRebuildByTimeLimitItem||bIsCanRebuildByConsumeItem? IMEPT_QUEST_BUILD_TACTICS_RANDOM_BY_ITEM: IMEPT_QUEST_BUILD_RANDOM);
							PgAction_ReqModifyItem kItemModifyAction(eType, GroundKey(), kOrder);
							kItemModifyAction.DoAction(pkUnit, NULL);
						}
					}
				}
			}
		}break;
	case PT_C_M_REQ_BEGIN_RANDOMQUEST:
	case PT_C_M_REQ_END_RANDOMQUEST:
	case PT_C_M_REQ_BEGIN_WANTEDQUEST:
	case PT_C_M_REQ_END_WANTEDQUEST:
		{
			int iQuestID = 0;
			pkNfy->Pop( iQuestID );

			PgQuestInfo const* pkQuestInfo = NULL;
			if( !g_kQuestMan.GetQuest(iQuestID, pkQuestInfo) )
			{
				break;
			}
			if( QT_Random == pkQuestInfo->Type()
			||	QT_RandomTactics == pkQuestInfo->Type() 
			||  QT_Wanted == pkQuestInfo->Type() )
			{
			}
			else
			{
				break;
			}

			switch( wType )
			{
			case PT_C_M_REQ_BEGIN_RANDOMQUEST:
			case PT_C_M_REQ_BEGIN_WANTEDQUEST:
				{
					BM::GUID kObjectGuid;
					if( pkQuestInfo->GetFirstClientNpc(kObjectGuid) )
					{
						PgAction_AnsQuestDialog kActionQuest(GroundKey(), kObjectGuid, iQuestID, QRDID_Begin, QRDID_ACCEPT_Start, 0, 0);
						kActionQuest.DoAction(pkUnit, NULL);
					}
				}break;
			case PT_C_M_REQ_END_RANDOMQUEST:
			case PT_C_M_REQ_END_WANTEDQUEST:
				{
					if( 0 == (GetAttr() & GATTR_FLAG_MISSION) ) // 미션에서 완료 안되
					{
						BM::GUID kObjectGuid;
						if( pkQuestInfo->GetFirstPayerNpc(kObjectGuid) )
						{
							PgAction_AnsQuestDialog kActionQuest(GroundKey(), kObjectGuid, iQuestID, QRDID_CanComplete, QRDID_COMPLETE_Start, 0, 0);
							kActionQuest.DoAction(pkUnit, NULL);
						}
					}
				}break;
			}
		}break;
	case PT_C_M_REQ_DROPQUEST:
		{
			int iQuestID = 0;
			pkNfy->Pop(iQuestID);

			PgQuestInfo const *pkQuestInfo = NULL;
			if( g_kQuestMan.GetQuest(iQuestID, pkQuestInfo) )
			{
				CONT_PLAYER_MODIFY_ORDER kOrder;

				SPMOD_AddIngQuest kAddQuestData(iQuestID, QS_None); // 포기 처리
				kOrder.push_back( SPMO(IMET_ADD_INGQUEST, pkUnit->GetID(), kAddQuestData) );

				PgAction_QuestDeleteItem kQuestAction(pkQuestInfo->m_kDropDeleteItem, kOrder); // 관련 아이템 삭제
				kQuestAction.DoAction(pkUnit, NULL);

				PgAction_ReqModifyItem kItemModifyAction(IMEPT_QUEST_DROP, GroundKey(), kOrder);
				kItemModifyAction.DoAction(pkUnit, NULL);

				if( (QT_Couple == pkQuestInfo->Type()) || (QT_SweetHeart == pkQuestInfo->Type()) )
				{
					PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
					if( pkPlayer )
					{
						BM::GUID const &rkCoupleGuid = pkPlayer->CoupleGuid();
						if( rkCoupleGuid )
						{
							BM::Stream kPacket(PT_C_N_REQ_COUPLE_COMMAND);
							kPacket.Push( pkUnit->GetID() );
							kPacket.Push( (BYTE) CC_SweetHeartQuest_Cancel );
							kPacket.Push( rkCoupleGuid );
							kPacket.Push( iQuestID );
							SendToCoupleMgr(kPacket);
						}
					}
				}
			}

		}break;
	case PT_C_M_REQ_SHAREQUEST:
		{
			int iShareQuestID = 0;

			pkNfy->Pop( iShareQuestID );

			PgQuestInfo const* pkQuestInfo = NULL;
			if( !g_kQuestMan.GetQuest(iShareQuestID, pkQuestInfo) )
			{
				break;
			}

			ContQuestShareRet kRetVec;
			EQuestShareRet eRet = QSR_CantShare;
			if( pkQuestInfo->IsCanShare() )
			{
				eRet = QSR_OnlyParty;
				PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkUnit);
				if( pkPlayer )
				{
					if( BM::GUID::IsNotNull(pkPlayer->PartyGuid()) )
					{
						PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
						if( pkMyQuest )
						{
							eRet = QSR_OnlyHave;
							SUserQuestState const* pkState = pkMyQuest->Get(iShareQuestID);
							if( pkState )
							{
								eRet = QSR_CantShare;
							}

							if( pkState
							&&	(QS_Ing && pkState->byQuestState || QS_End && pkState->byQuestState) )
							{
								VEC_GUID kVec;
								m_kLocalPartyMgr.GetPartyMemberGround(pkPlayer->PartyGuid(), GroundKey(), kVec, pkPlayer->GetID());

								eRet = QSR_Success;
								VEC_GUID::const_iterator iter = kVec.begin();
								while( kVec.end() != iter )
								{
									VEC_GUID::value_type const& rkGuid = (*iter);
									PgPlayer* pkAnswer = dynamic_cast< PgPlayer* >(GetUnit(rkGuid));
									if( pkAnswer )
									{
										bool bCanShare = (bCanShare = pkPlayer->PartyGuid() == pkAnswer->PartyGuid());
										if( bCanShare
										&&	PgGroundUtil::SUST_None != GetSpecStatus(pkAnswer->GetID()) )
										{
											bCanShare = false;
											kRetVec.push_back( SQuestShareRet(rkGuid, QBL_State) );
										}
										if( bCanShare
										&&	QTS_None != pkAnswer->GetAbil(AT_QUEST_TALK) )
										{
											bCanShare = false;
											kRetVec.push_back( SQuestShareRet(rkGuid, QBL_NowTalking) );
										}
										if( bCanShare
										&&	0 != pkAnswer->GetAbil(AT_EVENT_SCRIPT) )
										{
											bCanShare = false;
											kRetVec.push_back( SQuestShareRet(rkGuid, QBL_NowEventScript) );
										}
										PgMyQuest const* pkAnswerQuest = pkAnswer->GetMyQuest();
										if( pkAnswerQuest )
										{
											if( bCanShare )
											{
												if( (QT_Scenario == pkQuestInfo->Type()) || (QT_Soul == pkQuestInfo->Type()) )
												{
													bCanShare = !(MAX_ING_SCENARIO_QUEST == pkAnswerQuest->GetIngScenarioQuestNum());
												}
												else
												{
													bCanShare = !(MAX_ING_QUESTNUM == pkAnswerQuest->GetIngQuestNum());
												}

												if( !bCanShare )
												{
													kRetVec.push_back( SQuestShareRet(rkGuid, QBL_Max) );
												}
											}
											if( bCanShare )
											{
												switch( pkQuestInfo->Type() )
												{
												case QT_Couple:
												case QT_SweetHeart:
													{
														if( pkPlayer->CoupleGuid() != pkAnswer->GetID()
														||	pkPlayer->GetID() != pkAnswer->CoupleGuid() )
														{
															bCanShare = false;
															kRetVec.push_back( SQuestShareRet(rkGuid, QBL_Couple) );
														}
														if( bCanShare )
														{
															if( PgGroundQuestUtil::IsHaveCoupleQuest(pkAnswerQuest) )
															{
																bCanShare = false;
																kRetVec.push_back( SQuestShareRet(rkGuid, QBL_HaveCoupleQuest) );
															}
														}
													}break;
												default:
													{
														if( pkAnswerQuest->IsIngQuest(iShareQuestID) )
														{
															bCanShare = false;
															kRetVec.push_back( SQuestShareRet(rkGuid, QBL_Begined) );
														}
													}break;
												}
											}
										}

										if( bCanShare ) // 마지막 확인
										{
											PgCheckQuestBegin kAction(iShareQuestID);
											if( !kAction.DoAction(pkAnswer, NULL) )
											{
												kRetVec.push_back( SQuestShareRet(rkGuid, static_cast< BYTE >(kAction.Limit())) );
											}
											else
											{
												kRetVec.push_back( SQuestShareRet(rkGuid, QBL_None) ); //성공

												BM::Stream kNfyPacket(PT_M_C_ANS_SHAREQUEST);
												kNfyPacket.Push( static_cast< BYTE >(QSR_Notify) );
												kNfyPacket.Push( pkUnit->GetID() );
												kNfyPacket.Push( iShareQuestID );
												pkAnswer->Send( kNfyPacket );
											}
										}
									}
									++iter;
								}
							}
						}
					}
				}
			}

			{
				BM::Stream kPacket(PT_M_C_ANS_SHAREQUEST);
				kPacket.Push( static_cast< BYTE >(eRet) );
				kPacket.Push( kRetVec );
				pkUnit->Send( kPacket );
			}
		}break;
	case PT_C_M_REQ_ACCEPT_SHAREQUEST:
		{
			bool bSayYes = false;
			int iShareQuestID = 0;
			BM::GUID kOrgPlayerGuid;

			pkNfy->Pop( bSayYes );
			pkNfy->Pop( iShareQuestID );
			pkNfy->Pop( kOrgPlayerGuid );

			if( !bSayYes )
			{
				CUnit* pkOrgUnit = GetUnit(kOrgPlayerGuid);
				if( pkOrgUnit )
				{
					ContQuestShareRet kRetVec;
					kRetVec.push_back( SQuestShareRet(pkUnit->GetID(), QBL_Reject) );

					BM::Stream kPacket(PT_M_C_ANS_SHAREQUEST);
					kPacket.Push( static_cast< BYTE >(QSR_Success) );
					kPacket.Push( kRetVec );
					pkOrgUnit->Send( kPacket );
				}
				break;
			}

			PgQuestInfo const* pkQuestInfo = NULL;
			if( !g_kQuestMan.GetQuest(iShareQuestID, pkQuestInfo) )
			{
				break;
			}

			if( !pkQuestInfo->IsCanShare() )
			{
				BM::Stream kNfyPacket(PT_M_C_ANS_ACCEPT_SHAREQUEST);
				kNfyPacket.Push( static_cast< BYTE >(QSR_CantShare) );
				pkUnit->Send( kNfyPacket );
				break;
			}

			{
				CUnit* pkOrgUnit = GetUnit(kOrgPlayerGuid);
				if( !pkOrgUnit )
				{
					BM::Stream kNfyPacket(PT_M_C_ANS_ACCEPT_SHAREQUEST);
					kNfyPacket.Push( static_cast< BYTE >(QSR_Failed) );
					pkUnit->Send( kNfyPacket );
					break;
				}

				PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkOrgUnit);
				if( !pkPlayer )
				{
					break;
				}

				PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
				if( !pkMyQuest )
				{
					break;
				}

				if( !pkMyQuest->IsIngQuest(iShareQuestID) )
				{
					BM::Stream kNfyPacket(PT_M_C_ANS_ACCEPT_SHAREQUEST);
					kNfyPacket.Push( static_cast< BYTE >(QSR_Failed) );
					pkUnit->Send( kNfyPacket );
					break;
				}
			}

			{
				PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkUnit);
				if( !pkPlayer )
				{
					break;
				}

				PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
				if( !pkMyQuest )
				{
					break;
				}

				if( pkMyQuest->IsIngQuest(iShareQuestID)
				||	pkMyQuest->IsEndedQuest(iShareQuestID) )
				{
					BM::Stream kNfyPacket(PT_M_C_ANS_ACCEPT_SHAREQUEST);
					kNfyPacket.Push( static_cast< BYTE >(QSR_Ing) );
					pkUnit->Send( kNfyPacket );
					break;
				}
			}

			{
				BM::Stream kAddonPacket;
				kAddonPacket.Push( iShareQuestID );

				CONT_PLAYER_MODIFY_ORDER kOrder;

				SPMOD_AddIngQuest kAddQuestData(iShareQuestID, QS_Begin);
				kOrder.push_back(SPMO(IMET_ADD_INGQUEST, pkUnit->GetID(), kAddQuestData));

				PgAction_ReqModifyItem kBeinQuestAction(IMEPT_QUEST_SHARE, GroundKey(), kOrder, kAddonPacket);
				kBeinQuestAction.DoAction(pkUnit, NULL);
			}

			// 원 공유자에게 알림
			CUnit* pkOrgUnit = GetUnit( kOrgPlayerGuid );
			if( pkOrgUnit )
			{
				BM::Stream kNfyPacket(PT_M_C_ANS_SHAREQUEST);
				kNfyPacket.Push( static_cast< BYTE >(QSR_Yes) );
				kNfyPacket.Push( pkUnit->GetID() );
				pkOrgUnit->Send( kNfyPacket );
			}
		}break;
	case PT_C_M_REQ_COMPLETE_QUEST:
		{
			int iQuestID = 0;

			pkNfy->Pop( iQuestID );

			ReqRemoteCompleteQuest(pkUnit, iQuestID);
		}break;
		//<<Quest

	case PT_C_N_REQ_REALM_QUEST_INFO:
		{
			BM::Stream kPacket(wType, pkUnit->GetID());
			kPacket.Push( *pkNfy );
			::SendToRealmContents(PMET_REALM_EVENT, kPacket);
		}break;

	case PT_C_M_REQ_RUN_EVENT_SCRIPT:
		{
            m_kContEventScriptPlayer.insert(pkUnit->GetID());
			int iEventScriptID = 0;
			pkNfy->Pop( iEventScriptID );

			const bool bIsPvpGround = PgGroundUtil::IsPvpGround(GetAttr());
			if( !bIsPvpGround )
			{
				int const iPreActivatedEventID = pkUnit->GetAbil(AT_EVENT_SCRIPT);
				const bool bIsVillageGround = PgGroundUtil::IsVillageGround(GetAttr());

				if(	pkUnit )
				{
					pkUnit->SetAbil(AT_EVENT_SCRIPT, iEventScriptID);
					pkUnit->SetAbil(AT_EVENT_SCRIPT_TIME, 0); // 초기화

					if(false == IsCheckEventScriptNoLock(iEventScriptID))
					{
						BM::Stream LockPacket(PT_M_C_REQ_LOCK_INPUT_EVENT_SCRIPT);
						pkUnit->Send(LockPacket, E_SENDTYPE_SELF);

						//SetEventScript_LockPartyMember(const_cast<CUnit const*>(pkUnit), iEventScriptID);
					}

					if( PgGroundUtil::IsCanProtectedEventID(iEventScriptID)
					&&	!bIsVillageGround // 마을이 아닐때만
					&&	0 == iPreActivatedEventID ) // 이전 이벤트가 없을 때만
					{
						pkUnit->AddCountAbil( AT_CANNOT_DAMAGE, AT_CF_EVENTSCRIPT, true, E_SENDTYPE_BROADALL_EFFECTABIL );// 타겟팅 안되도록 설정
						
						if( 0==pkUnit->GetCountAbil(AT_CANNOT_ATTACK, AT_CF_EVENTSCRIPT) )
						{
							pkUnit->AddCountAbil( AT_CANNOT_ATTACK, AT_CF_EVENTSCRIPT, true, E_SENDTYPE_BROADALL_EFFECTABIL );// 공격못하도록 설정
						}
					}
				}
			}
			else
			{
				CAUTION_LOG(BM::LOG_LV5, __FL__<<L"Can't run this ground["<<GetGroundNo()<<L", hase a ATTR PVP/EmporiaBattle] EventScript[ID: "<<iEventScriptID<<L"] plyaer[Name: "<<pkUnit->Name()<<L", Guid: "<<pkUnit->GetID()<<L"]");
			}

			if(IsCheckEventScriptStopAI(iEventScriptID) && !m_kContEventScriptPlayer.empty())
			{
				CUnit* pkPlayer = NULL;
				CONT_OBJECT_MGR_UNIT::iterator kItor;
				PgObjectMgr::GetFirstUnit( UT_PLAYER, kItor );
				while (pkPlayer = PgObjectMgr::GetNextUnit( UT_PLAYER, kItor))
				{
					if( 0==pkPlayer->GetCountAbil(AT_CANNOT_ATTACK, AT_CF_EVENTSCRIPT) )
					{
						pkPlayer->AddCountAbil( AT_CANNOT_ATTACK, AT_CF_EVENTSCRIPT, true, E_SENDTYPE_BROADALL_EFFECTABIL );// 공격못하도록 설정
					}
				}
				StopAI();
			}
		}break;
	case PT_C_M_REQ_END_EVENT_SCRIPT:
		{
            m_kContEventScriptPlayer.erase(pkUnit->GetID());
			int iEventScriptID = 0;
			pkNfy->Pop( iEventScriptID );

			int const iPreActivatedEventID = pkUnit->GetAbil(AT_EVENT_SCRIPT);
			bool const bStopAI = IsCheckEventScriptStopAI(iEventScriptID);
			if( pkUnit
			&&	0 != iEventScriptID )
			{
				if( iPreActivatedEventID != iEventScriptID )
				{
					CAUTION_LOG(BM::LOG_LV5, __FL__<<L"Missmatch Pre-EventID["<<iPreActivatedEventID<<L"] : DeActivateEventID["<<iEventScriptID<<L"], Player[Name: "<<pkUnit->Name()<<L", Guid: "<<pkUnit->GetID()<<L"]");
				}

				bool bUnLockPlayer = false;
				if( m_kContEventScriptPlayer.empty() )
				{// 컨테이너가 비어있으므로 무조건 언락한다.
					bUnLockPlayer = true;
				}

				BM::GUID const& PartyGuid = pkUnit->GetPartyGuid();
				if( true == PartyGuid.IsNull() )
				{// 파티가 없으면 바로 언락한다.
					bUnLockPlayer = true;
				}

				if( false == bUnLockPlayer )
				{// 파티가 있으면 파티원들이 m_kContEventScriptPlayer 컨테이너에 들어있는지 검사해야한다.
					VEC_GUID PartyMember;
					GetPartyMember( PartyGuid, PartyMember );
					
					bool bWaitParty = false;
					VEC_GUID::const_iterator iter;
					for( iter = PartyMember.begin(); iter != PartyMember.end(); ++iter )
					{
						SET_GUID::iterator event_iter = m_kContEventScriptPlayer.find(*iter);
						if( m_kContEventScriptPlayer.end() != event_iter )
						{// 이벤트 스크립트가 끝나기를 기다릴 파티원이 존재한다.
							bWaitParty = true;
							break;
						}
					}
					if( false == bWaitParty )
					{// 기다릴 파티원이 없으면 언락한다.
						bUnLockPlayer = true;
					}
				}

				if( bUnLockPlayer )
				{
					pkUnit->SetAbil(AT_EVENT_SCRIPT, 0); // 초기화
					pkUnit->SetAbil(AT_EVENT_SCRIPT_TIME, 0); // 초기화
					BM::Stream LockPacket(PT_M_C_REQ_UNLOCK_INPUT_EVENT_SCRIPT);
					pkUnit->Send(LockPacket, E_SENDTYPE_SELF);
					SetEventScript_UnLockPartyMember(const_cast<CUnit const*>(pkUnit));
				}

				if( PgGroundUtil::IsCanProtectedEventID(iEventScriptID)
				&&	!PgGroundUtil::IsVillageGround(GetAttr()) )
				{
					pkUnit->AddCountAbil( AT_CANNOT_DAMAGE, AT_CF_EVENTSCRIPT, false, E_SENDTYPE_BROADALL_EFFECTABIL );// 타겟팅 안되도록 설정 해제

					if(false==bStopAI)
					{
						pkUnit->AddCountAbil( AT_CANNOT_ATTACK, AT_CF_EVENTSCRIPT, false, E_SENDTYPE_BROADALL_EFFECTABIL );// 공격못하도록 설정 해제
					}
				}
			}

			if(bStopAI && m_kContEventScriptPlayer.empty())
			{
				CUnit* pkPlayer = NULL;
				CONT_OBJECT_MGR_UNIT::iterator kItor;
				PgObjectMgr::GetFirstUnit( UT_PLAYER, kItor );
				while (pkPlayer = PgObjectMgr::GetNextUnit( UT_PLAYER, kItor))
				{
					if( pkPlayer->GetCountAbil(AT_CANNOT_ATTACK, AT_CF_EVENTSCRIPT) )
					{
						pkPlayer->AddCountAbil( AT_CANNOT_ATTACK, AT_CF_EVENTSCRIPT, false, E_SENDTYPE_BROADALL_EFFECTABIL );// 공격못하도록 설정 해제
					}
				}

				StartAI();
			}
		}break;

	case PT_C_M_REQ_REGQUICKSLOT:
		{
			size_t slot_idx;
			SQuickInvInfo kQuickInvInfo;

			pkNfy->Pop(slot_idx);
			pkNfy->Pop(kQuickInvInfo);

			RecvReqRegQuickSlot(pkUnit, slot_idx, kQuickInvInfo);
		}break;
	case PT_C_M_REQ_REGQUICKSLOT_VIEWPAGE:
		{
			char cViewPage;
			pkNfy->Pop(cViewPage);
			RecvReqRegQuickSlotViewPage(pkUnit, cViewPage);
		}break;
	case PT_C_M_REQ_BEGINCAST:
		{
			// Charging 공격을 시도하겠다고 요청
			int iSkillNo;
			DWORD dwCurrentTime;
			pkNfy->Pop(iSkillNo);
			pkNfy->Pop(dwCurrentTime);
			RecvReqCastBegin(pkUnit, iSkillNo, dwCurrentTime);
		}break;
	case PT_C_M_REQ_PICKUPGBOX:
		{
			BM::GUID kBoxGuid;
			BM::GUID kLooterGuid;
			pkNfy->Pop(kBoxGuid);
			pkNfy->Pop(kLooterGuid);

			CUnit * pkLooterUnit = GetUnit(kLooterGuid);

			PickUpGroundBox(pkUnit, kBoxGuid, pkLooterUnit);
		}break;
	case PT_C_M_REQ_MON_BLOWSTATUS: // 뛰워진 몹의 정보는 일단 Client에 잠시 의존하자!
		{
#ifdef AI_DEBUG
			INFO_LOG(BM::LOG_LV8, __FL__<<L"PT_C_M_REQ_MON_BLOWSTATUS entered");
#endif
			BM::GUID guidMonster;
			POINT3 pt3MonsterPos;
			bool bFirstFloor = false;
			bool bNoUseStandUpTime = false;

			pkNfy->Pop(guidMonster);
			pkNfy->Pop(pt3MonsterPos);
			pkNfy->Pop(bFirstFloor);
			pkNfy->Pop(bNoUseStandUpTime);

			CUnit* pkTarget = PgObjectMgr::GetUnit( guidMonster);
			if (pkTarget == NULL)
			{
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("pkTarget is NULL"));
				break;
			}

			if(pkTarget->IsDead())
			{
				break;
			}

			EAIActionType const kAIType = pkTarget->GetAI()->eCurrentAction;
			int iEffectDurTime = pkTarget->GetAbil(AT_DAMAGE_EFFECT_DURATION);
			int iDelayTime = 0;
			//pkUnit->SendWarnMessageStr((BM::vstring)__FL__<<" Real iEffectDurTime : "<<iEffectDurTime);
			if(bFirstFloor && kAIType == EAI_ACTION_BLOWUP)
			{
				//처음 땅에 닿은것이라면 얼마나 튕길지 모르기 때문에(경사면, 절벽) 좀 길게 잡아놓자
				iEffectDurTime = 3000;
				iDelayTime = 3000;
			}
			else
			{
				if( UT_MONSTER==pkTarget->UnitType() )
				{
					iEffectDurTime = std::min(1000,iEffectDurTime);
				}
				else
				{
					iEffectDurTime = std::max(1000,iEffectDurTime);
				}
			}

			iEffectDurTime = __max(iEffectDurTime, 300); //적어도 0.3초 정도는 누워 있어야 하지 않나?
			if(300==iEffectDurTime && kAIType == EAI_ACTION_BLOWUP)
			{
				iDelayTime = 1000;
			}
			
			if(!bNoUseStandUpTime)
			{
				pkTarget->SetAbil(AT_POSTURE_STATE, 1);	//누웠다
				if(!bFirstFloor)	//두번째 패킷이면
				{
					pkTarget->StandUpTime(std::min(iEffectDurTime, pkTarget->StandUpTime()));//둘중에 작은 값으로 하자
				}
				else
				{
					pkTarget->StandUpTime(iEffectDurTime);
				}
			}
			else
			{
				pkTarget->StandUpTime(300); //StandUpTime을 사용하지 않아도 데미지 액션의 딜레이가 있는 경우가 있기 때문에 0.3초 정도 값을 넣음
			}

			//pkUnit->SendWarnMessageStr((BM::vstring)__FL__<<" iEffectDurTime : "<<iEffectDurTime<<" iDelayTime : "<<iDelayTime<<" StandUpTime : "<<pkTarget->StandUpTime()<<" Name : "<<pkTarget->Name()<<" bFirstFloor : "<<(bFirstFloor?"TRUE":"FALSE")<<" Posture : "<<pkTarget->GetAbil(AT_POSTURE_STATE)<<" AIType : "<<EAI_ACTION_BLOWUP);

			//INFO_LOG(BM::LOG_LV6, "AT_DAMAGE_EFFECT_DURATION : "<<iEffectDurTime<<" bFirstFloor : "<<bFirstFloor);
			
			CUnit* pkAttacker = GetUnit(pkTarget->GetBlowAttacker());
			if(pkTarget->IsInUnitType(UT_PLAYER)) //플레이어는 패스
			{
				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkTarget);
				if(pkPlayer)
				{
					PLAYER_ACTION_INFO* pkOldActionInfo = pkPlayer->GetOldPlayerActionInfo();
					if(pkOldActionInfo)
					{
						pkOldActionInfo->ptPos = pt3MonsterPos;	//넘어질때 좌표는 액션으로 가지 않아서 해킹으로 간주되는 현상 방지
					}
				}
				if(pkAttacker 
					&& (UT_PLAYER==pkAttacker->UnitType() || UT_SUB_PLAYER == pkAttacker->UnitType())
					)
				{
					break;
				}
			}

			bool bSameAttacker = pkUnit->GetID() == pkTarget->GetBlowAttacker() || pkUnit->IsSummonUnit(pkTarget->GetBlowAttacker(),true);

			if (!bSameAttacker && kAIType==EAI_ACTION_BLOWUP)	//공중으로 띄워졌을 때
			{
				//INFO_LOG(BM::LOG_LV6, _T("[%s] - BlowAttacker mismatched TargetGUID[%s]"), __FUNCTIONW__, pkTarget->GetID().str().c_str());
				//여기선 어짜피 클라가 보내준 좌표를 사용하지 않으니까 상관없음
				
				if( pkAttacker )
				{
					switch( pkAttacker->UnitType() )
					{
					case UT_ENTITY:
						{
							PgEntity* pkEntity = dynamic_cast<PgEntity*>(pkAttacker);
							if( pkEntity 
								&& pkUnit->GetID() == pkEntity->Caller()
								)
							{
								bSameAttacker = true;
							}
						}break;
					case UT_SUB_PLAYER:
						{
							PgSubPlayer* pkSubPlayer = dynamic_cast<PgSubPlayer*>(pkAttacker);
							if( pkSubPlayer 
								&& pkUnit->GetID() == pkSubPlayer->Caller()
								)
							{
								bSameAttacker = true;
							}
						}break;
					}
				}
				
				if(!bSameAttacker)
				{
					pt3MonsterPos = pkTarget->GetPos();
				}
			}

			NxRay kRay(NxVec3(pt3MonsterPos.x, pt3MonsterPos.y, pt3MonsterPos.z+20.0f), NxVec3(0, 0, -1.0f));
			NxRaycastHit kHit;
			NxShape *pkHitShape = RayCast(kRay, kHit, 1000.0f);
			if(pkHitShape)
			{
				if (PgGround::ms_kAntiHackCheckVariable.bUse_Blowup)
				{
					pt3MonsterPos.z = pkTarget->GetPos().z;//kHit.worldImpact.z;//단순 거리체크. 단 위에서 떨어졌을때는 무시
					//float const fDist = POINT3::Distance(pkTarget->GetBlowStartPos(), pt3MonsterPos);
					float const fDist = POINT3::Distance(pkTarget->GetPos(), pt3MonsterPos);
					CSkill *pkSkill = pkUnit->GetSkill();
					float fMaxDist = static_cast<float>(pkSkill->GetAbil(AT_BLOW_DISTANCE));
					if(0.0f==fMaxDist)
					{
						fMaxDist = AI_BLOWUP_LEN;
					}

					if(fMaxDist <= fDist)
					{	//해커 인덱스 증가
						PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
						if(pkPlayer)
						{
							BM::vstring kLogMsg;
							kLogMsg << __FL__ << "[HACKING][Blowup] User[" << pkPlayer->Name() << "] GUID[" << pkPlayer->GetID() 
								<< "]  MemberGUID[" << pkPlayer->GetMemberGUID() << _T("] GroundNo[") << GroundKey().GroundNo() 
								<< _T("] HackingType[Blowup] Distance=") << fDist;
							//VERIFY_INFO_LOG(false, BM::LOG_LV0, kLogMsg);
							HACKING_LOG(BM::LOG_LV0, kLogMsg);
							if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
							{
								pkPlayer->SendWarnMessageStr(kLogMsg);
							}

							if (pkPlayer->SuspectHacking(EAHP_Blowup, PgGround::ms_kAntiHackCheckVariable.sBlowup_AddIndex))
							{
								// SpeedHack이 의심스러우니 강제접속 해제 시킨다.
								BM::Stream kDPacket( PT_A_S_NFY_USER_DISCONNECT, static_cast<BYTE>(CDC_Blowup_Hack) );
								kDPacket.Push( pkPlayer->GetMemberGUID() );
								SendToServer( pkPlayer->GetSwitchServer(), kDPacket );
							}
							break;
						}
					}
				}
			}
			else	//실패했으면
			{
				INFO_LOG(BM::LOG_LV5, __FUNCTIONW__<<" raycastClosestShape Fail!! GUID : "<<pkTarget->GetID()<<" GroundNo : "<<GetGroundNo());
				pkTarget->SetAbil(AT_HP, 0);
			}

			pt3MonsterPos.z = kHit.worldImpact.z;	//해킹체크 블록 내부에 있는 코드를 밖으로 꺼냄
			pkTarget->SetPos(pt3MonsterPos);
			if ( pkTarget->IsCheckZoneTime(100) )
			{
				AdjustArea( pkTarget, true, true );
			}

			if(!bSameAttacker) {break;}
			
			if(!bFirstFloor)
			{
				EUnitState eState = pkTarget->GetState();
				if (eState != US_PUSHED_BY_DAMAGE && eState != US_DAMAGEDELAY)
				{
					//INFO_LOG(BM::LOG_LV9, __FL__<<L"SetBlowAttacker(NULL) TargetGUID["<<pkTarget->GetID()<<L"]");
					pkTarget->SetBlowAttacker(BM::GUID::NullData());

					pkTarget->SetDelay(iDelayTime);
					pkTarget->SetAbil(AT_DAMAGE_EFFECT_DURATION,0);
					//pkUnit->SendWarnMessageStr((BM::vstring)__FL__<<"Clear AT_DAMAGE_EFFECT_DURATION");
#ifdef AI_DEBUG
					INFO_LOG(BM::LOG_LV8, __FL__<<L"BlowUp ended");
#endif
				}
			}
		}break;
	case PT_C_M_REQ_SPEND_MONEY:
		{
			ESpendMoneyType kType = ESMT_NONE;
			pkNfy->Pop(kType);

			PgAction_SpendMoney kAction(kType,this);
			kAction.DoAction(pkUnit);
		}break;
	case PT_C_M_REQ_ITEM_ACTION:
		{
			OnReqItemAction( dynamic_cast<PgPlayer*>(pkUnit), pkNfy );
		}break;
	case PT_C_M_REQ_JOIN_LOBBY:
		{
			PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pPlayer )
			{
				if( pPlayer->HaveExpedition() )
				{	// 원정대 상태라면 원정대 컨텐츠만 할 수 있다..
					pkUnit->SendWarnMessage(720051);
					break;
				}
			}
			Recv_PT_C_M_REQ_JOIN_LOBBY( pkUnit, *pkNfy );
		}break;
	case PT_C_M_REQ_RET_HARDCORE_VOTE:
		{
			Recv_PT_C_M_REQ_RET_HARDCORE_VOTE( pkUnit, *pkNfy );
		}break;

	case PT_C_M_REQ_DUEL_PVP:
		{ //어떤놈이 결투 하자고 했으니 대상에게 알려주자
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			BM::GUID kAskCharGuid;
			pkNfy->Pop(kAskCharGuid);
			PgPlayer *pkAskUser = GetUser(kAskCharGuid);
			if ( !pkUser || !pkAskUser )
			{
				// 없는 유저이다.
				//pkUser->Send( BM::Stream(PT_M_C_ERROR_DUEL_PVP,(int)200127), E_SENDTYPE_SELF );
				break;
			}

			//나중에 아래 예외 검사들은 CheckCanEnter() 함수를 만들어 빼내자
			if(pkUser->GetAbil(AT_DUEL)) //결투받을놈이 이미 결투 중인가?
			{
				if(pkUser->GetAbil(AT_DUEL) > 0)
				{ //결투중인가?
					pkUser->SendWarnMessage(201203);
				}
				else if(pkUser->GetAbil(AT_DUEL) == -1)
				{ //신청/응답중인가?
					pkUser->SendWarnMessage(201219);
				}
				BM::Stream kPacket(PT_M_C_ANS_DUEL_PVP, static_cast<BYTE>(1));
				pkUser->Send(kPacket);
				break;
			}
			if(pkAskUser->GetAbil(AT_DUEL)) //결투건놈이 이미 결투 중인가
			{
				if(pkAskUser->GetAbil(AT_DUEL) > 0)
				{
					pkUser->SendWarnMessage(201204);
				}
				else if(pkAskUser->GetAbil(AT_DUEL) == -1)
				{
					pkUser->SendWarnMessage(201220);
				}
				BM::Stream kPacket(PT_M_C_ANS_DUEL_PVP, static_cast<BYTE>(1));
				pkUser->Send(kPacket);
				break;
			}
			float const fDistance = POINT3::Distance(pkUser->GetPos(), pkAskUser->GetPos());
			if(fDistance > 300) //두놈의 거리가 300을 넘는다면 결투는 성립되지 않는다.
			{
				pkUser->SendWarnMessage(201205);
				BM::Stream kPacket(PT_M_C_ANS_DUEL_PVP, static_cast<BYTE>(1));
				pkUser->Send(kPacket);
				break;
			}

			pkUser->SetAbil(AT_DUEL, -1, true, true);
			pkAskUser->SetAbil(AT_DUEL, -1, true, true);

			BM::Stream kAskPacket( PT_M_C_REQ_DUEL_PVP, pkUser->GetID() );
			kAskPacket.Push( pkUser->Name() );
			pkAskUser->Send( kAskPacket, E_SENDTYPE_SELF );
		}break;
	case PT_C_M_ANS_DUEL_PVP:
		{ //대상한테 결투할꺼냐고 물어보니까 대답이 왔다.
			BM::GUID kReqCharGuid;
			if ( pkNfy->Pop(kReqCharGuid) )
			{
				bool bOK = false;
				pkNfy->Pop(bOK);

				PgPlayer* pkPlayer = GetUser(pkUnit->GetID());
				PgPlayer* pkReqPlayer = GetUser( kReqCharGuid );
				if(pkPlayer)
				{
					pkPlayer->SetAbil(AT_DUEL, 0, true, true);
				}
				if(pkReqPlayer)
				{
					pkReqPlayer->SetAbil(AT_DUEL, 0, true, true);
				}
				if ( pkPlayer && pkReqPlayer )
				{
					if(bOK)
					{ //수락
						//이때 다시 한번 거리체크를 더해보자
						float const fDistance = POINT3::Distance(pkReqPlayer->GetPos(), pkPlayer->GetPos());
						if(fDistance > 300) //두놈의 거리가 300을 넘는다면 결투는 성립되지 않는다.
						{
							pkPlayer->SendWarnMessage(201205);
							pkReqPlayer->SendWarnMessage(201205);
							BM::Stream kPacket(PT_M_C_ANS_DUEL_PVP, static_cast<BYTE>(1));
							pkReqPlayer->Send(kPacket);
							break;
						}

						//신청자 또는 대상이 다른 유저와 신청중일 수도 있으므로 검색해본다.
						if(m_kDuelMgr.FindExistPlayer(pkPlayer->GetID(), pkReqPlayer->GetID()))
						{
							pkPlayer->SendWarnMessage(201211);
							pkReqPlayer->SendWarnMessage(201211);
							BM::Stream kPacket(PT_M_C_ANS_DUEL_PVP, static_cast<BYTE>(1));
							pkReqPlayer->Send(kPacket);
							break;
						}

						m_kDuelMgr.Create(this, pkReqPlayer->GetID(), pkPlayer->GetID());
						BM::Stream kPacket(PT_M_C_ANS_DUEL_PVP, static_cast<BYTE>(1));
						pkReqPlayer->Send(kPacket);
					}
					else
					{ //거절
						BM::Stream kPacket(PT_M_C_ANS_DUEL_PVP, static_cast<BYTE>(0));
						kPacket.Push(pkUnit->Name());
						pkReqPlayer->Send(kPacket);
						//pkReqPlayer->Send( BM::Stream(PT_M_C_ANS_DUEL_PVP, pkUnit->Name() ), E_SENDTYPE_SELF );
					}
				}
			}
		}break;
	case PT_C_M_NFY_DUEL_PVP_CANCEL:
		{ //결투 건놈이 취소했다. 대상에게도 알려주자
			BM::GUID kAnsCharGuid;
			if ( pkNfy->Pop(kAnsCharGuid) )
			{
				PgPlayer *pkReqPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				PgPlayer *pkAnsPlayer = dynamic_cast<PgPlayer*>(GetUnit(kAnsCharGuid));
				if ( pkReqPlayer )
				{
					pkReqPlayer->SetAbil(AT_DUEL, 0, true, true);
				}
				if ( pkAnsPlayer )
				{
					pkAnsPlayer->SetAbil(AT_DUEL, 0, true, true);
					pkAnsPlayer->Send( BM::Stream(PT_M_C_ERROR_DUEL_PVP,(int)200123), E_SENDTYPE_SELF );
				}
			}
		}break;
/*
	case PT_C_M_REQ_DUEL_PVP:
		{
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if( pkUser->HaveParty() )
			{
				// 파티가 있으면 PvP신청을 할 수 없습니다.
				pkUser->Send( BM::Stream(PT_M_C_ERROR_DUEL_PVP,(int)200125), E_SENDTYPE_SELF );
				break;
			}
			else if ( pkUser->GetAbil(AT_LEVEL) < PgWarMode::ms_kSetting.LevelLimit() )
			{
				pkUser->Send( BM::Stream(PT_M_C_ERROR_DUEL_PVP,(int)200135), E_SENDTYPE_SELF );
				break;
			}

			BM::GUID kAskCharGuid;
			pkNfy->Pop(kAskCharGuid);
			PgPlayer *pkAskUser = GetUser(kAskCharGuid);
			if ( !pkAskUser )
			{
				// 없는 유저이다.
				pkUser->Send( BM::Stream(PT_M_C_ERROR_DUEL_PVP,(int)200127), E_SENDTYPE_SELF );
			}
			else if( pkAskUser->HaveParty() )
			{
				// 파티가 있는 유저라 신청할 수 없다.
				BM::Stream kPacket(PT_M_C_ERROR_INPARTY_PVP);
				kPacket.Push(pkAskUser->Name());
				kPacket.Push((int)200126);
				pkUser->Send( kPacket, E_SENDTYPE_SELF );
			}
			else if ( pkAskUser->GetAbil(AT_LEVEL) < PgWarMode::ms_kSetting.LevelLimit() )
			{
				pkUnit->Send( BM::Stream(PT_M_C_ERROR_DUEL_PVP,(int)200136), E_SENDTYPE_SELF );
			}
			else
			{
				BM::Stream kAskPacket( PT_M_C_REQ_DUEL_PVP, pkUser->GetID() );
				kAskPacket.Push( pkUser->Name() );
				pkAskUser->Send( kAskPacket, E_SENDTYPE_SELF );
			}
		}break;
	case PT_C_M_ANS_DUEL_PVP:
		{
			if ( pkUnit->GetAbil(AT_LEVEL) < PgWarMode::ms_kSetting.LevelLimit() )
			{
				pkUnit->Send( BM::Stream(PT_M_C_ERROR_DUEL_PVP,(int)200135), E_SENDTYPE_SELF );
			}
			else
			{
				BM::GUID kReqCharGuid;
				if ( pkNfy->Pop(kReqCharGuid) )
				{
					bool bOK = false;
					pkNfy->Pop(bOK);

					PgPlayer *pkReqPlayer = GetUser( kReqCharGuid );
					if ( pkReqPlayer )
					{
						if ( pkReqPlayer->GetAbil(AT_LEVEL) >= PgWarMode::ms_kSetting.LevelLimit() )
						{
							if ( bOK )
							{
								SReqMapMove_MT kRMM(MMET_PvP);
								kRMM.kTargetKey.GroundNo( PvP_Lobby_GroundNo_Exercise );

								PgReqMapMove kMapMove( this, kRMM, NULL );
								kMapMove.Add( pkReqPlayer );
								kMapMove.Add( dynamic_cast<PgPlayer*>(pkUnit) );
								kMapMove.DoAction();
							}
							else
							{// 거절했다.
								pkReqPlayer->Send( BM::Stream(PT_M_C_ANS_DUEL_PVP, pkUnit->Name() ), E_SENDTYPE_SELF );
							}
						}
					}
				}
			}
		}break;
	case PT_C_M_NFY_DUEL_PVP_CANCEL:
		{
			BM::GUID kReqCharGuid;
			if ( pkNfy->Pop(kReqCharGuid) )
			{
				CUnit *pkReqPlayer = GetUnit(kReqCharGuid);
				if ( pkReqPlayer )
				{
					pkReqPlayer->Send( BM::Stream(PT_M_C_ERROR_DUEL_PVP,(int)200123), E_SENDTYPE_SELF );
				}
			}
		}break;
	case PT_C_M_ERROR_DUEL_PVP:
		{
			BM::GUID kReqCharGuid;
			if ( pkNfy->Pop(kReqCharGuid) )
			{
				CUnit *pkReqPlayer = GetUnit(kReqCharGuid);
				if ( pkReqPlayer )
				{
					pkReqPlayer->Send( BM::Stream(PT_M_C_ERROR_DUEL_PVP,(int)200124), E_SENDTYPE_SELF );
				}
			}
		}break;
*/
	case PT_C_M_REQ_PVP_SELECTOR:
		{
			if( PgGroundUtil::SUST_PvPModeSelectting == EnterSpecStatus(pkUnit->GetID(), PgGroundUtil::SUST_PvPModeSelectting) )
			{
				BM::Stream kAnsPacket( PT_M_C_ANS_PVP_SELECTOR );
				pkUnit->Send( kAnsPacket );
			}
		}break;
	case PT_C_M_REQ_PVP_SELECTOR_CANCEL:
		{
			if ( true == LeaveSpecStatus( pkUnit->GetID(), PgGroundUtil::SUST_PvPModeSelectting ) )
			{
				FakeAddUnit( pkUnit );
			}
		}break;
	case PT_C_M_REQ_OBMODE_TARGET_CHANGE:
		{
			PgNetModule<> kNetModule;
			if ( SUCCEEDED( PgObjectMgr2::GetObserver( pkUnit->GetID(), kNetModule) ) )
			{
				ProcessObserverPacket( pkUnit->GetID(), kNetModule, dynamic_cast<PgPlayer*>(pkUnit), wType, *pkNfy );
			}
			else
			{
				CAUTION_LOG( BM::LOG_LV5, _T("[PT_C_M_REQ_OBMODE_TARGET_CHANGE] Player<") << pkUnit->Name() << _T("> is Not Observer!!") );
			}
		}break;
	case PT_C_N_REQ_CHAT_NOTICE://공지
	case PT_C_N_REQ_MSN_FRIENDCOMMAND://메신져, 친구목록
		{
			//컨텐츠로 중계
			BM::Stream kPacket(wType, pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToFriendMgr(kPacket);
		}break;
	case PT_C_O_REQ_GMCHAT :
		{
			BM::Stream kPacket(PT_C_O_ANS_SEND_GM_CHAT);
			kPacket.Push(*pkNfy);
			SendToContents(kPacket);
		}break;
	case PT_C_N_REQ_COUPLE_COMMAND:
		{
			PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( !pkPlayer )
			{
				break;
			}

			BYTE cCmdType = CC_None;
			pkNfy->Pop( cCmdType );

			switch( cCmdType )
			{
			case CC_Req_Warp:
				{
					if( PgGroundUtil::SUST_HardCoreDungeonVote == GetSpecStatus(pkUnit->GetID()) )
					{
						pkUnit->SendWarnMessage( 402204 );// 하드코어 던젼 투표중에는 이동 금지
					}
					else
					{
						int iRecvItemNo = 0;
						pkNfy->Pop( iRecvItemNo );

						ECoupleCommandResult eRet = CoupleCR_None;

						PgInventory *pkInven = pkPlayer->GetInven();
						if( pkInven )
						{
							if( COUPLE_WARP_ITEM_NO == iRecvItemNo
								&&	0 < pkInven->GetTotalCount(COUPLE_WARP_ITEM_NO) )
							{
								eRet = CoupleCR_Success;
							}

							if( CoupleCR_Success == eRet )
							{
								if( true == pkPlayer->IsItemEffect(SAFE_FOAM_EFFECTNO) )
								{
									eRet = CoupleCR_NotMapMove;
								}
								if( pkPlayer->IsDead() )
								{
									eRet = CoupleCR_NotMapMove;
								}
							}
						}

						if( CoupleCR_Success == eRet )
						{
							BM::Stream kPacket(wType, pkUnit->GetID());
							kPacket.Push( cCmdType );
							SendToCoupleMgr(kPacket);
						}
						else
						{
							BM::Stream kFailPacket(PT_N_C_ANS_COUPLE_COMMAND, cCmdType);
							kFailPacket.Push( (BYTE)eRet );
							pkUnit->Send(kFailPacket);
						}
					}
					
					cCmdType = CC_None;
				}break;
			case CC_Req_SweetHeart_Quest:
			case CC_Ans_SweetHeart_Quest:
				{
					BM::GUID kCoupleGuid;
					int iQuestID = 0;

					pkNfy->Pop( kCoupleGuid );
					pkNfy->Pop( iQuestID );

					PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
					if( !pkPlayer )
					{
						break;
					}

					PgMyQuest const *pkMyQuest = pkPlayer->GetMyQuest();
					if( pkMyQuest )
					{
						bool bRet = true;
						ContUserQuestState kStateVec;
						bool const bGetQuestRet = pkMyQuest->GetQuestList(kStateVec);
						if( bGetQuestRet )
						{
							ContUserQuestState::iterator iter = kStateVec.begin();
							while(kStateVec.end() != iter)
							{
								ContUserQuestState::value_type const& rkQuestState = (*iter);
								int const iIngQuestID = rkQuestState.iQuestID;
								
								PgQuestInfo const* pkQuestIngInfo = NULL;
								if( g_kQuestMan.GetQuest(iIngQuestID, pkQuestIngInfo) )
								{
									if( (QT_Couple == pkQuestIngInfo->Type()) || (QT_SweetHeart == pkQuestIngInfo->Type()) )
									{
										// 커플/연인 퀘스트는 1개만 수행할 수 있습니다.
										pkPlayer->SendWarnMessage(450104);
										bRet = false;
										break;
									}
								}
								++iter;
							}
						}

						if( true == bRet )
						{
							if( MAX_ING_QUESTNUM == (pkMyQuest->GetIngQuestNum() - pkMyQuest->GetIngScenarioQuestNum()) )
							{
								pkPlayer->SendWarnMessage(700401);
							}
							else if( true == pkMyQuest->IsIngQuest(iQuestID) )
							{
								pkPlayer->SendWarnMessage(450105);
							}
							else
							{
								BM::Stream kPacket(wType, pkUnit->GetID());
								kPacket.Push( cCmdType );
								kPacket.Push( kCoupleGuid );
								kPacket.Push( iQuestID );
								kPacket.Push( *pkNfy );
								SendToCoupleMgr(kPacket);
							}
						}
					}
					cCmdType = CC_None;
				}break;
			case CC_Ans_SweetHeart_Complete:
				{
					BM::GUID kMineGuid;
					bool bSayYes = false;
					int iQuestID = 0;

					pkNfy->Pop( kMineGuid );
					pkNfy->Pop( bSayYes );
					pkNfy->Pop( iQuestID );

					PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
					if( pkPlayer )
					{
						PgMyQuest const *pkMyQuest = pkPlayer->GetMyQuest();
						if( pkMyQuest )
						{
							bool bRet = true;
							int iSkillNo = 0;
							PgQuestInfo const* pkQuestInfo = NULL;
							if( g_kQuestMan.GetQuest(iQuestID, pkQuestInfo) )
							{
								iSkillNo = pkQuestInfo->m_kReward.iSkillNo;
							}
							else
							{
								if( true == bSayYes )
								{
									bRet = false;
								}
							}
							if( true == bRet )
							{
								BM::Stream kPacket(wType, pkUnit->GetID());
								kPacket.Push( cCmdType );
								kPacket.Push( kMineGuid );
								kPacket.Push( bSayYes );
								kPacket.Push( iQuestID );
								kPacket.Push( iSkillNo );
								kPacket.Push( *pkNfy );
								SendToCoupleMgr(kPacket);
							}
						}
					}
					cCmdType = CC_None;
				}break;
			default:
				{
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Invalid CaseType"));
				}break;
			}

			if( CC_None != cCmdType )
			{
				BM::Stream kPacket(wType, pkUnit->GetID());
				kPacket.Push( cCmdType );
				kPacket.Push( *pkNfy );
				SendToCoupleMgr(kPacket);
			}
		}break;
	case PT_C_M_REQ_MARRY_COMMAND:
		{
			PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( !pkPlayer )
			{
				break;
			}

			BYTE cCmdType = 0;
			pkNfy->Pop( cCmdType );

			switch( cCmdType )
			{
			case MC_ReqMarry://결혼신청
				{
					if( pkPlayer->GetCoupleStatus() == (CoupleS_None) )
					{
						pkPlayer->SendWarnMessage(450135);
						break;
					}

					if( pkPlayer->GetCoupleStatus() == (CoupleS_Normal | CoupleS_SweetHeart) )
					{
						pkPlayer->SendWarnMessage(450134);
						break;
					}

					int iGoldMoney = 0;

					pkNfy->Pop( iGoldMoney );

					BM::Stream kPacket(wType, cCmdType);
					kPacket.Push(pkPlayer->GetID());
					kPacket.Push(GroundKey());
					kPacket.Push(iGoldMoney);
					SendToCenter(kPacket);
				}break;
			case MC_ReqSendMoney://기부하기
				{
					// 여기는 결혼식장
					int iGoldMoney = 0;

					pkNfy->Pop( iGoldMoney );

					BYTE eType = 0;
					if( true == m_kMarryMgr.GetActionState(eType) )
					{
						if( static_cast<BYTE>(EM_TALK) <= eType )
						{
							// 이미 결혼 진행중
							pkPlayer->SendWarnMessage(450130);
							break;
						}

						BM::Stream kPacket(wType, cCmdType);
						kPacket.Push(pkPlayer->GetID());
						kPacket.Push(GroundKey());
						kPacket.Push(iGoldMoney);
						SendToCenter(kPacket);
					}
				}break;
			case MC_SetActionState:
				{
					// 여기는 결혼식장
					int iType = 0;

					pkNfy->Pop( iType );

					if( static_cast<BYTE>(EM_TALK) == static_cast<BYTE>(iType) )
					{
						BYTE eType = 0;
						if( true == m_kMarryMgr.GetActionState(eType) )
						{
							if( EM_TALK <= eType )
							{
								// 이미 결혼 진행중
								pkPlayer->SendWarnMessage(450129);
								break;
							}
							else
							{
								// 내가 결혼시작하는지, 신청은 했는지?
								BM::Stream kPacket(wType, cCmdType);
								kPacket.Push(pkPlayer->GetID());
								kPacket.Push(GroundKey());
								kPacket.Push(iType);
								SendToCenter(kPacket);
							}
						}
					}
					else if( static_cast<BYTE>(EM_NONE) == static_cast<BYTE>(iType) )
					{// 초기화 용도(게임 상태에서는 들어 올수가 없다)
						BM::Stream kPacket(wType, cCmdType);
						kPacket.Push(pkPlayer->GetID());
						kPacket.Push(GroundKey());
						kPacket.Push(iType);
						SendToCenter(kPacket);

						m_kMarryMgr.Clear();
					}
				}break;
			case MC_TotalMoney:
				{
					BM::Stream kPacket(wType, cCmdType);
					kPacket.Push(pkPlayer->GetID());
					kPacket.Push(GroundKey());
					SendToCenter(kPacket);
				}break;
			default:
				{
				}
				break;
			}
		}break;
	case PT_C_N_REQ_GUILD_COMMAND:
		{
			BYTE cCommand = 0;
			pkNfy->Pop(cCommand);

			switch(cCommand)//컨탠츠 이전에 선처리 할경우는 여기서 처리 한다.
			{
			case GC_PreCreate_Test:
				{
					EGuildCommandRet const eRet = GuildUtil::CheckCreateGuild(pkUnit);

					INFO_LOG(BM::LOG_LV1, __FL__<<L"[GUILD-PreCreate_Test: "<<pkUnit->Name()<<L"] Success - "<<eRet);

					BM::Stream kPacket(PT_N_C_ANS_GUILD_COMMAND, cCommand);
					kPacket.Push((BYTE)eRet);
					pkUnit->Send(kPacket);

					cCommand = 0;//Don't send Contents
				}break;
			case GC_PreCreate:
				{
					PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
					if( pkPlayer )
					{
						int iRet = GCR_Success;
						if(BM::GUID::NullData() != pkPlayer->GuildGuid())
						{
							iRet = GCR_Member;//이미 길드에 가입되있다.
						}
						else
						{
							iRet = GuildUtil::CheckCreateGuild(pkUnit);
						}

						if( GCR_Success != iRet )
						{
							BM::Stream kPacket(PT_N_C_ANS_GUILD_COMMAND, cCommand);
							kPacket.Push((BYTE)iRet);
							pkUnit->Send(kPacket);

							CAUTION_LOG(BM::LOG_LV1, __FL__<<L"[GUILD-PreCreate: "<<pkUnit->Name()<<L"] Failed - Ret: "<<iRet);
						}
						else//성공이면 센터로
						{
							BM::Stream kPacket(PT_M_T_REQ_GUILD_COMMAND, pkPlayer->GetMemberGUID());
							kPacket.Push(cCommand);
							kPacket.Push(*pkNfy);
							SendToGuildMgr(kPacket);
						}
					}
					cCommand = 0;//Don't send Contents
				}break;
			case GC_Create:
				{
					BM::Stream kAddOnPacket;
					kAddOnPacket.Push(*pkNfy);//ReqModifyItem AddOnPacket에서 .Data() 로 추가하기 때문에 Rd, wr pos 무시된다.

					__int64 const iGuildPrice = GuildUtil::GetHowMuchCreateGuild();//길드 가격

					CONT_PLAYER_MODIFY_ORDER kOrder;
					
					SPMOD_Add_Money kDelMoneyData(-iGuildPrice);//필요머니 빼기.
					kOrder.push_back( SPMO(IMET_ADD_MONEY, pkUnit->GetID(), kDelMoneyData) );

					PgAction_ReqModifyItem kItemModifyAction(MCE_CreateGuild, GroundKey(), kOrder, kAddOnPacket);
					kItemModifyAction.DoAction(pkUnit, NULL);

					cCommand = 0;//Don't send Contents
				}break;
			default:
				{
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Invalid CaseType"));
					//None task
				}break;
			}
			//
			if( cCommand )
			{
				BM::Stream kPacket(wType, pkUnit->GetID());
				kPacket.Push(cCommand);
				kPacket.Push(*pkNfy);
				SendToGuildMgr(kPacket);
			}
		}break;
	case PT_C_N_REQ_GET_ENTRANCE_OPEN_GUILD:
	case PT_C_N_REQ_GUILD_ENTRANCE:
	case PT_C_N_REQ_GUILD_ENTRANCE_CANCEL:	
		{
			//컨텐츠로 중계
			BM::Stream kPacket(wType, pkUnit->GetID());
			kPacket.Push(*pkNfy);
			SendToGuildMgr(kPacket);
		}break;
	case PT_N_C_REQ_PAYMENT_GUILD_ENTRANCE_FEE:
		{// 길드 가입 비용 지불
			BM::GUID kGuildGuid;
			__int64 iMoney;

			pkNfy->Pop(kGuildGuid);
			pkNfy->Pop(iMoney);
			
			if( pkUnit->GetAbil64(AT_MONEY) < iMoney )
			{// 돈이 모자르다.
				BM::Stream kPacket(PT_N_C_ANS_GUILD_COMMAND, (BYTE)GC_Join);
				kPacket.Push((BYTE)GCR_Money);
				pkUnit->Send(kPacket);				
				return false;
			}

			// 컨텐츠 보내서 길드 가입 처리 시키고
			// 가입 완료 되면 맵서버로 다시 보내서 길드 가입 비용 처리 오더 만들자.
			BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, pkUnit->GetID());
			kPacket.Push((BYTE)GC_Join);
			kPacket.Push(kGuildGuid);
			kPacket.Push(true);
			kPacket.Push(iMoney); // 길드가입 시스템을 통해 가입할 때만 가입비용이 발생 할 수 있다.
			SendToGuildMgr(kPacket);
		}break;
	case PT_C_M_REQ_NPC_TALK_MAP_MOVE:
		{
			BM::GUID kNpcGuid;
			int iTargetGroundNo = 0;

			pkNfy->Pop( kNpcGuid );
			pkNfy->Pop( iTargetGroundNo );

			CONT_TBL_DEF_NPC_TALK_MAP_MOVE const* pkDefNpcTalkMapMove = NULL;
			g_kTblDataMgr.GetContDef(pkDefNpcTalkMapMove);
			CONT_TBL_DEF_NPC_TALK_MAP_MOVE::const_iterator iter = pkDefNpcTalkMapMove->find( GetGroundNo() );
			CUnit* pkNpc = GetUnit(kNpcGuid);
			PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkUnit);
			if( pkPlayer
			&&	pkNpc
			&&	PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpc, GetGroundNo(), NMT_ClientOnly, __F_P__(PT_C_M_REQ_NPC_TALK_MAP_MOVE), __LINE__)
			&&	pkDefNpcTalkMapMove->end() != iter )
			{
				CONT_TBL_DEF_NPC_TALK_MAP_MOVE::mapped_type const& rkContTrgMap = (*iter).second;
				CONT_TBL_DEF_NPC_TALK_MAP_MOVE::mapped_type::const_iterator trg_iter = rkContTrgMap.begin();
				while( rkContTrgMap.end() != trg_iter )
				{
					if( (*trg_iter).kNpcGuid == kNpcGuid
					&&	(*trg_iter).iTargetGroundNo == iTargetGroundNo )
					{
						SReqMapMove_MT kRMM;
						kRMM.kCasterKey = GroundKey();
						kRMM.kCasterSI = g_kProcessCfg.ServerIdentity();
						kRMM.kTargetKey.GroundNo( iTargetGroundNo );
						kRMM.nTargetPortal = (*trg_iter).nTargetPortalNo;
						PgReqMapMove kMapMove( this, kRMM, NULL );
						if( kMapMove.Add(pkPlayer) )
						{
							kMapMove.DoAction();
						}	
						break;
					}
					++trg_iter;
				}
			}
		}break;
	case PT_C_M_REQ_DEFAULT_MAP_MOVE:	// Default Map Move
		{
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if ( pkPlayer )
			{
				SReqMapMove_MT kRMM;
				kRMM.kCasterKey = GroundKey();
				kRMM.kCasterSI = g_kProcessCfg.ServerIdentity();

				if ( true == pkPlayer->TransTowerSaveRct().IsEmpty() )
				{
					pkPlayer->GetRecentGround( kRMM.kTargetKey, GATTR_VILLAGE );//마을로 가야 한다.
					kRMM.pt3TargetPos = pkPlayer->GetRecentPos(GATTR_VILLAGE);
				}
				else
				{
					SRecentInfo const &kRecentInfo = pkPlayer->TransTowerSaveRct();
					kRMM.kTargetKey.GroundNo( kRecentInfo.iMapNo );
					kRMM.pt3TargetPos = kRecentInfo.ptPos3;
				}

				if ( kRMM.kTargetKey.GroundNo() )
				{
					PgReqMapMove kMapMove( this, kRMM, NULL );
					if ( kMapMove.Add( pkPlayer ) )
					{
						kMapMove.DoAction();
					}	
				}
				else
				{
					// 최근 마을이 없는 경우는 제자리에서 살릴 수 밖에...
					INFO_LOG( BM::LOG_LV5, __FL__<<L"[PT_C_M_REQ_DEFAULT_MAP_MOVE] Last Village is 0["<<pkPlayer->Name()<<L"-"<<pkPlayer->GetID()<<L"]" );
					pkPlayer->Alive( EALIVE_PVP, E_SENDTYPE_BROADALL );

					PgDeathPenalty kAction(GroundKey(), LURT_MapMove, this->IsDeathPenalty() );//그냥 살릴때는. 패널티를 준다.
					kAction.DoAction(pkPlayer, NULL);
				}

				//파티장 교체
				BM::GUID const& kPartyGuid = pkUnit->GetPartyGuid();
				BM::GUID const& kCharGuid = pkUnit->GetID();
				if( BM::GUID::IsNotNull(kPartyGuid) )
				{
					size_t const iMinSize = 1;
					VEC_GUID kContGuid;
					m_kLocalPartyMgr.GetPartyMemberGround(kPartyGuid, GroundKey(), kContGuid, kCharGuid);
					if( m_kLocalPartyMgr.IsMaster(kPartyGuid, kCharGuid)
					&&	iMinSize <= kContGuid.size() )
					{
						// 오너를 교체해야 한다.
						BM::Stream kNPacket(PT_C_N_REQ_PARTY_CHANGE_MASTER, kCharGuid);
						kNPacket.Push( (*kContGuid.begin()) );
						::SendToGlobalPartyMgr(kNPacket);
					}
				}
			}
			else
			{
				INFO_LOG(BM::LOG_LV0, __FL__<<L"PT_C_M_REQ_DEFAULT_MAP_MOVE : Cannot convect CUnit -> PgPlayer : UnitType["<<pkUnit->UnitType()<<L"] Guid["<<pkUnit->GetID()<<L"]");
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("pkPlayer is NULL"));
			}
		}break;
	case PT_UNIT_NFY_ON_DIE://패킷이 아님.
		{
			this->RecvUnitDie(pkUnit);
		}break;
	case PT_U_G_NFY_COMBAT_REVIVE:
		{
			int const iDelEffectNo = pkUnit->GetAbil(AT_COMBAT_REVIVE_DEL_EFFECT_NO);
			int const iAddEffectNo = pkUnit->GetAbil(AT_COMBAT_REVIVE_ADD_EFFECT_NO);
			pkUnit->SetAbil(AT_HP, 1, true); // HP를 최소로 만들어주고
			//pkUnit->SetAbil( AT_HP, pkUnit->GetAbil(AT_C_MAX_HP) );
			pkUnit->SetAbil(AT_COMBAT_REVIVE_DEL_EFFECT_NO, 0, true);
			pkUnit->SetAbil(AT_COMBAT_REVIVE_ADD_EFFECT_NO, 0, true);
			
			SActArg kArg;
			PgGroundUtil::SetActArgGround(kArg, this);
			pkUnit->AddEffect(EFFECTNO_CANNOT_DAMAGE, 0, &kArg, pkUnit);	// 무적 버프
			pkUnit->AddEffect(iAddEffectNo, 0, &kArg, pkUnit);

			//액션 하라고 알려주고
			BM::Stream kPacket(PT_M_C_DO_COMBAT_REVIVE_ACTION);
			kPacket.Push( pkUnit->GetID() );
			pkUnit->Send(kPacket);
			pkUnit->DeleteEffect(iDelEffectNo, true);
		}break;
	case PT_C_M_REMOVEITEM:	// 아이템을 버렸다.
		{
			SItemPos kPos;
			int iDeleteType = 0;
			pkNfy->Pop(kPos);
			pkNfy->Pop(iDeleteType);

			if(pkUnit && pkUnit->IsUnitType(UT_PLAYER))
			{
				ContItemRemoveOrder kContOrder;

				SItemRemoveOrder kElement;
				kElement.kCasterPos = kPos;
				kContOrder.push_back(kElement);

				PgAction_ReqRemoveInvItem kAction(GroundKey(), kContOrder,(iDeleteType ? IRT_BREAK_REMOVE : IRT_NORMAL_REMOVE));
				kAction.DoAction(pkUnit, NULL);
			}
		}break;
	case PT_C_M_REQ_ITEM_DIVIDE:
		{
			SItemPos kItemPos;
			int iItemNo = 0;
			BM::GUID kItemGuid;
			int iCount = 0;
			
			pkNfy->Pop(kItemPos);
			pkNfy->Pop(iItemNo);
			pkNfy->Pop(kItemGuid);
			pkNfy->Pop(iCount);

			PgAction_ReqDivideItem kAction(GroundKey(), kItemPos, iItemNo, kItemGuid, iCount);
			kAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_ITEM_SMS:
		{	
			bool bGenericSMS;
			BM::GUID kItemGuid;
			int iItemNo;
			SItemPos kItemPos;
			std::wstring strMemo;
			pkNfy->Pop(bGenericSMS);
			pkNfy->Pop(iItemNo);
			pkNfy->Pop(kItemGuid);
			pkNfy->Pop(kItemPos);

			PgAction_ReqSMS kAction(this, kItemPos, iItemNo, kItemGuid, *pkNfy);
			kAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_STORE_ITEM_LIST:
		{
			BM::GUID kStoreGuid;
            BYTE byType = 0;
			pkNfy->Pop(kStoreGuid);
            pkNfy->Pop(byType);

			CUnit* pkNpcUnit = GetNPC(kStoreGuid);

			if(PgGroundUtil::IsCanTalkableRange(pkUnit, kStoreGuid, pkNpcUnit, GetGroundNo(), NMT_Shop, __F_P__(PT_C_M_REQ_STORE_ITEM_LIST), __LINE__) )
			{
				PgAction_ReqStoreItemList kAction(kStoreGuid, byType);
				kAction.DoAction(pkUnit, NULL);
			}
		}break;
	case PT_C_M_REQ_STORE_ITEM_BUY:
		{
			BM::GUID kStoreGuid;
            BYTE byType = 0;
			BYTE bySecondType = 0;
			int iItemNo;
			int iCount;
			pkNfy->Pop(kStoreGuid);
            pkNfy->Pop(byType);
            pkNfy->Pop(bySecondType);
			pkNfy->Pop(iItemNo);
			pkNfy->Pop(iCount);

			CUnit* pkNpcUnit = GetNPC(kStoreGuid);
			if((0 < pkUnit->GetAbil(AT_CALL_STORE)) || PgGroundUtil::IsCanTalkableRange(pkUnit, kStoreGuid, pkNpcUnit, GetGroundNo(), NMT_Shop, __F_P__(PT_C_M_REQ_STORE_ITEM_BUY), __LINE__) )
			{
				PgAction_ReqStoreItemBuy kAction( kStoreGuid, byType, bySecondType, iItemNo, iCount, GroundKey(), GetEventAbil() );
				kAction.DoAction(pkUnit, NULL);
			}
		}break;
	case PT_C_M_REQ_STORE_CLOSE:
		{
			pkUnit->SetAbil(AT_CALL_STORE,0,false);
		}break;
	case PT_C_M_REQ_EMPORIA_ADMINISTRATION:
	case PT_C_M_REQ_GET_EMPORIA_ADMINISTRATOR:
		{
			PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if ( pkPlayer )
			{
				BM::GUID kNpcGuid;
				pkNfy->Pop( kNpcGuid );

				CUnit *pkNpc = GetNPC(kNpcGuid);
				if(		BM::GUID::IsNotNull( pkPlayer->GuildGuid() )
					&&	PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpc, GetGroundNo(), NMT_EmporiaAdmin, __F_P__(PT_C_M_REQ_GET_EMPORIA_ADMINISTRATOR), __LINE__)
					)
				{
					BM::Stream kPacket( ((wType == PT_C_M_REQ_EMPORIA_ADMINISTRATION) ? PT_M_N_REQ_EMPORIA_ADMINISTRATION : PT_M_N_REQ_GET_EMPORIA_ADMINISTRATOR ) );
					kPacket.Push( pkPlayer->GuildGuid() );
					kPacket.Push( pkPlayer->GetID() );
					kPacket.Push( kNpcGuid );
					kPacket.Push( *pkNfy );
					::SendToGuildMgr( kPacket );
				}
				else
				{
					BM::Stream kAnsPacket( PT_N_C_ANS_GET_EMPORIA_ADMINISTRATOR, kNpcGuid );
					kAnsPacket.Push( false );
					pkPlayer->Send( kAnsPacket );
				}
			}
		}break;
	case PT_C_M_REQ_STORE_ITEM_SELL:
		{
			BM::GUID kNpcGuid;
            BYTE byType = 0;
			SItemPos kItemPos;
			int iItemNo = 0;
			int iCount = 0;
			bool bIsStockShop = false;
			pkNfy->Pop(kNpcGuid);
            pkNfy->Pop(byType);
			pkNfy->Pop(kItemPos);
			pkNfy->Pop(iItemNo);
			pkNfy->Pop(iCount);
			pkNfy->Pop(bIsStockShop);
			
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if((0 < pkUnit->GetAbil(AT_CALL_STORE)) || PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_Shop, __F_P__(PT_C_M_REQ_STORE_ITEM_SELL), __LINE__) )
			{
				PgAction_ReqStoreItemSell kAction(kItemPos, iItemNo, iCount, GroundKey(), kNpcGuid, byType, bIsStockShop);
				kAction.DoAction(pkUnit, NULL);
			}
		}break;
	case PT_C_M_REQ_RANK_PAGE:
		{
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if ( pkUser )
			{
				BM::Stream kPacket(wType,pkUser->GetMemberGUID());
				kPacket.Push(*pkNfy);
				SendToRankMgr(kPacket);
			}
		}break;
	case PT_C_M_REQ_RANK_TOP:
		{
			PgPlayer *pkUser = dynamic_cast<PgPlayer*>(pkUnit);
			if ( pkUser )
			{
				BM::Stream kPacket(wType,pkUser->GetMemberGUID());
				kPacket.Push(*pkNfy);
				SendToRankMgr(kPacket);
			}
		}break;
	case PT_C_M_REQ_EFFECT_CONTROL:
		{
			int iEffectNo;
			pkNfy->Pop(iEffectNo);
			PgAction_EffectControl kAction(GroundKey(),iEffectNo);
			kAction.DoAction(pkUnit);
		}break;
	case PT_C_S_NFY_UNIT_POS:
		{
			PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if ( pkPlayer )
			{
				DWORD dwLatancy = 0;
				POINT3 pt3Pos;
				bool bIncludeScPos;
				pkNfy->Pop(dwLatancy);
				pkNfy->Pop(pt3Pos);
				pkNfy->Pop(bIncludeScPos);

				pkPlayer->RecvLatency(dwLatancy, pt3Pos);
				if(bIncludeScPos)
				{
					pkNfy->Pop(pt3Pos);
					PgSubPlayer* pkSubPlayer = GetSubPlayer( pkPlayer );
					if(pkSubPlayer)
					{
						pkSubPlayer->SetPos(pt3Pos);
					}
				}
			}
			AdjustArea(pkUnit, true, pkUnit->IsCheckZoneTime(100) );
		}break;
	case PT_C_M_REQ_ITEM_PLUS_UPGRADE:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit)
			{
				switch(pkNpcUnit->UnitType())
				{
				case UT_MYHOME:
					{
						if(true == PgGroundUtil::IsMyHomeHasSideJob(pkUnit, pkNpcUnit, MSJ_ENCHANT))
						{
							PgAction_ItemPlusUpgrade kAction(GroundKey(), *pkNfy);
							kAction.DoAction(pkUnit, pkNpcUnit);
						}
					}break;
				default:
					{
						if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_Enchant, __F_P__(PT_C_M_REQ_ITEM_PLUS_UPGRADE), __LINE__) )
						{
							PgAction_ItemPlusUpgrade kAction(GroundKey(), *pkNfy);
							kAction.DoAction(pkUnit, NULL);
						}
					}break;
				}
			}
		}break;
	case PT_C_M_REQ_SEAL_ITEM:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			BM::Stream kPacket(PT_M_C_ANS_SEAL_ITEM);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy,kPacket);
			kAction.DoAction( pkUnit, pkUnit );
		}break;
	case PT_C_M_REQ_ITEM_DISCHARGE:
		{
			SItemPos kItemPos;
			pkNfy->Pop(kItemPos);
			PgAction_ReqUseItem kAction(kItemPos,this,false,*pkNfy);
			kAction.DoAction( pkUnit, pkUnit );
		}break;
	case PT_C_M_REQ_ITEM_RARITY_UPGRADE:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop(kNpcGuid);
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit)
			{
				switch(pkNpcUnit->UnitType())
				{
				case UT_MYHOME:
					{
						if(true == PgGroundUtil::IsMyHomeHasSideJob(pkUnit, pkNpcUnit, MSJ_SOULCRAFT))
						{
							PgAction_ItemRarityUpgrade kAction(GroundKey(),*pkNfy);
							kAction.DoAction(pkUnit, pkNpcUnit);
						}
					}break;
				default:
					{
						if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_SoulCraft, __F_P__(PT_C_M_REQ_ITEM_RARITY_UPGRADE), __LINE__) )
						{
							PgAction_ItemRarityUpgrade kAction(GroundKey(),*pkNfy);
							kAction.DoAction(pkUnit, NULL);
						}
					}
				}
			}
		}break;
	case PT_C_M_REQ_ITEM_REPAIR:
		{
			BM::GUID kNpcGuid;
			CONT_REQ_ITEM_REPAIR kContRepair;

			pkNfy->Pop(kNpcGuid);
			pkNfy->Pop(kContRepair);

			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if(pkNpcUnit)
			{
				switch(pkNpcUnit->UnitType())
				{
				case UT_MYHOME:
					{
						if(true == PgGroundUtil::IsMyHomeHasSideJob(pkUnit,pkNpcUnit,MSJ_REPAIR))
						{
							PgAction_ItemRepair kAction(GroundKey(), kContRepair);
							kAction.DoAction(pkUnit, pkNpcUnit);
						}
					}break;
				default:
					{
						if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_Repair, __F_P__(PT_C_M_REQ_ITEM_REPAIR), __LINE__) )
						{
							PgAction_ItemRepair kAction(GroundKey(), kContRepair);
							kAction.DoAction(pkUnit, NULL);
						}
					}break;
				}
			}
		}break;
	case PT_C_M_REQ_EXCHANGE_ITEM_REQ:
		{
			BM::GUID kTargetGuid;
			pkNfy->Pop(kTargetGuid);
			if (kTargetGuid == pkUnit->GetID())
			{
				INFO_LOG(BM::LOG_LV3, __FL__ << _T("Cannot trade item with SELF Guid=") << kTargetGuid);
				CAUTION_LOG(BM::LOG_LV3, __FL__ << _T("Cannot trade item with SELF Guid=") << kTargetGuid);
				break;
			}
			if (BM::GUID::NullData() != kTargetGuid)
			{
				STradeInfo kTempInfo;
				CUnit* pkPilot = PgObjectMgr::GetUnit(kTargetGuid);
				
				if (pkPilot)
				{
					BM::Stream kPacket(PT_M_C_NFY_EXCHANGE_ITEM_REQ, kTargetGuid);
					if(!pkUnit->IsAlive())	//죽은 상태에서는 교환 불가
					{	//클라에서 막는데도 뚫고 왔음
						INFO_LOG(BM::LOG_LV6, __FL__ << _T("Cannot trade item. Caster Is Dead. Guid : ") << pkUnit->GetID());
						kPacket.Push(TR_CASTER_IS_DEAD);
					}
					else if(!pkPilot->IsAlive())	//죽은 놈에게 거래 신청
					{
						INFO_LOG(BM::LOG_LV6, __FL__ << _T("Cannot trade item. Target Is Dead. Guid : ") << kTargetGuid);
						kPacket.Push(TR_TARGET_IS_DEAD);
					}
					else if (g_kItemTradeMgr.FindTradeByChar(kTargetGuid, kTempInfo) || g_kItemTradeMgr.FindTradeByChar(pkUnit->GetID(), kTempInfo))
					{
						kPacket.Push(TR_ON_TRADEING);
					}
					else if( 3000 < BM::GetTime32() - g_kItemTradeMgr.FineTradeEndTime(pkPilot->GetID()))	// 3초 이상
					{
						const BM::GUID kNewGuid = g_kItemTradeMgr.NewItemTrade(pkUnit->GetID(), kTargetGuid);

						if (BM::GUID::NullData() != kNewGuid)
						{
							kPacket.Push(TR_SUCCESS);
							kPacket.Push(kNewGuid);
							BM::Stream kTargetPacket(PT_M_C_NFY_EXCHANGE_ITEM_REQ, pkUnit->GetID());
							kTargetPacket.Push(TR_REQUEST);
							kTargetPacket.Push(kNewGuid);
							pkPilot->Send(kTargetPacket);
						}
						else
						{
							kPacket.Push(TR_FAIL);
						}
					}
					else
					{
						kPacket.Push(TR_TRADED_IN_3SEC);
					}

					pkUnit->Send(kPacket);	//거래 신청자에게 보냄
				}
				else
				{
					INFO_LOG(BM::LOG_LV0, __FL__<<L"PT_C_M_REQ_EXCHANGE_ITEM_REQ : Cannot Find CUnit : Target Guid["<<kTargetGuid<<L"] Owner Guid["<<pkUnit->GetID()<<L"]");
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("pkPilot is NULL"));
				}
			}
			else
			{
				INFO_LOG(BM::LOG_LV0, __FL__<<L"PT_C_M_REQ_EXCHANGE_ITEM_REQ : Target Guid Is Null : Owner Guid["<<pkUnit->GetID()<<L"]");
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("kTargetGuid is Null"));
			}
			
		}break;
	case PT_C_M_NFY_EXCHANGE_ITEM_ANS:
		{
			BM::GUID kTradeGuid;
			bool bRet = false;
			pkNfy->Pop(kTradeGuid);
			pkNfy->Pop(bRet);

			STradeInfo kTradeInfo;

			BM::Stream kFromPacket(PT_M_C_NFY_EXCHANGE_ITEM, kTradeGuid);
			BM::Stream kToPacket(PT_M_C_NFY_EXCHANGE_ITEM, kTradeGuid);

			BM::GUID kCasterGuid = BM::GUID::NullData();
			BM::GUID kToGuid = pkUnit->GetID();
			CUnit* pkPilot = NULL;

			if(!g_kItemTradeMgr.FindTrade(kTradeGuid, kTradeInfo))
			{
				INFO_LOG(BM::LOG_LV0, "PT_C_M_NFY_EXCHANGE_ITEM_ANS : Cannot Find From g_kItemTradeMgr. Trade Guid["<<kTradeGuid.str().c_str()<<"] User : "<<pkUnit->Name().c_str());
				break;	// goto문사용금지 (초기화되지 않은 지역변수가 goto이후에 사용된다)
			}

			if (pkUnit->GetID() == kTradeInfo.kCasterGuid)
			{
				pkPilot = PgObjectMgr::GetUnit(kTradeInfo.kTargetGuid);	
			}
			else if (pkUnit->GetID() == kTradeInfo.kTargetGuid)
			{
				pkPilot = PgObjectMgr::GetUnit(kTradeInfo.kCasterGuid);	
			}
			else
			{
				break;
			}

			if (!pkPilot)
			{
				INFO_LOG(BM::LOG_LV0, __FL__<<L"PT_C_M_NFY_EXCHANGE_ITEM_ANS : Cannot Find CUnit : kTradeInfo.kGuidFrom["<<kTradeInfo.kCasterGuid<<L"]");
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("pkPilot is NULL"));
				goto __FAILD;
			}

			kCasterGuid = pkPilot->GetID();	//여기까지 왔으면 유닛을 찾았으므로

			int eRet = TR_ACCEPT;

			if (!bRet)
			{
				if (!g_kItemTradeMgr.DelItemTrade(kTradeGuid))
				{
					INFO_LOG(BM::LOG_LV0, "PT_C_M_NFY_EXCHANGE_ITEM_ANS : Cannot Find From g_kItemTradeMgr. Trade Guid["<<kTradeGuid.str().c_str()<<"] User : "<<pkUnit->Name().c_str());
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("DelItemTrade is Failed!"));
				}
				eRet = TR_REJECT;
			}

			goto __SUCCESS;
__FAILD:
			{
				g_kItemTradeMgr.DelItemTrade(kTradeGuid);	
				
				eRet = TR_FAIL;
			}

__SUCCESS:
			{

			}

			kFromPacket.Push(kToGuid);
			kFromPacket.Push(eRet);

			kToPacket.Push(kCasterGuid);
			kToPacket.Push(eRet);
			pkUnit->Send(kToPacket);
			if (pkPilot && BM::GUID::NullData() != pkPilot->GetID())
			{
				pkPilot->Send(kFromPacket);
			}

		}break;
	case PT_C_M_NFY_EXCHANGE_ITEM_ITEM:
		{
			STradeInfoPacket kInfoPacket;
			kInfoPacket.ReadFromPacket(*pkNfy);
			bool bConfirmOrModify = false;
			bool bIsConfirm = false;
			pkNfy->Pop(bConfirmOrModify);
			pkNfy->Pop(bIsConfirm);

			BM::Stream kPacket(PT_M_C_NFY_EXCHANGE_ITEM_ITEM);

			// Owner정보 강제 세팅
			kInfoPacket.kOwnerGuid = pkUnit->GetID();

			PgBase_Item kItem;
			CONT_TRADE_ITEM::const_iterator item_it = kInfoPacket.kItemCont.begin();
			while (kInfoPacket.kItemCont.end() != item_it)
			{
				if((S_OK != pkUnit->GetInven()->GetItem((*item_it).kPos,kItem)) ||	//아이템이 없거나
					(kItem.ItemNo()!=(*item_it).kItem.ItemNo()) ||						//번호가 다르거나
					(kItem.Guid()!=(*item_it).kItem.Guid()) ||						//GUID가 다르거나
					(kItem.Count()<(*item_it).kItem.Count()) )	//갯수가 서버가 알고있는것보다 많으면
				{
					INFO_LOG(BM::LOG_LV0, (BM::vstring)__FL__<<" Not Find Trade Item. TradeGuid["<<kInfoPacket.kTradeGuid.str().c_str()<<"] MemberName : "<<pkUnit->Name()<<" MemberGuid["<<pkUnit->GetID().str().c_str()<<"] ItemNo["<<(*item_it).kItem.ItemNo()<<"]");  
					kPacket.Push(TR_NOT_FIND_ITEM);
					pkUnit->Send(kPacket);
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
					return false;
				}


				if(false == CheckEnableTrade(kItem,ICMET_Cant_PlayerTrade))
				{
					INFO_LOG(BM::LOG_LV0, __FL__<<L"PT_C_M_NFY_EXCHANGE_ITEM_ITEM : Cannot Trade TimeLimit | Quest Item. TradeGuid["<<kInfoPacket.kTradeGuid<<L"]"); 
					kPacket.Push(TR_QUEST_ITEM);
					pkUnit->Send(kPacket);
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
					return false;
				}

				++item_it;
			}

			/* bool const bConfirm = */bConfirmOrModify && bIsConfirm;

			STradeInfo kInfo;
			int eRet = TR_SUCCESS;
			//bool bFrom = false;
			BM::GUID kOtherGuid;
			if (!g_kItemTradeMgr.FindTrade(kInfoPacket.kTradeGuid, kInfo))
			{
				INFO_LOG(BM::LOG_LV0, __FL__<<L"PT_C_M_NFY_EXCHANGE_ITEM_ITEM : Cannot Find From g_kItemTradeMgr. Trade Guid["<<kInfoPacket.kTradeGuid<<L"]");
				break;	// goto문사용금지 (초기화되지 않은 지역변수가 goto이후에 사용된다)
			}

			__int64 i64MyMoney = pkUnit->GetInven()->Money();
			i64MyMoney = std::max(0i64, i64MyMoney);
			__int64 i64TradeMoney = __min(i64MyMoney, kInfoPacket.kMoney);
			i64TradeMoney = std::max(0i64, i64TradeMoney);
			kInfoPacket.kMoney = i64TradeMoney;
			if ( pkUnit->GetID() == kInfo.kCasterGuid)	//패킷 보낸 놈이 누구냐
			{
			//	bFrom = true;
				kInfo.kCasterItemCont = kInfoPacket.kItemCont;
				kInfo.bCasterConfirm = bConfirmOrModify;
				kInfo.i64CasterMoney = i64TradeMoney;
				kOtherGuid = kInfo.kTargetGuid;
				if(!kInfo.bCasterConfirm)	//확인을 취소했으면 다른놈도 취소
				{
					kInfo.bTargetConfirm = false;
					kInfo.bTargetOk = false;
				}
			}
			else if (pkUnit->GetID() == kInfo.kTargetGuid)
			{
			//	bFrom = false;
				kInfo.kTargetItemCont = kInfoPacket.kItemCont;
				kInfo.bTargetConfirm = bConfirmOrModify;
				kInfo.i64TargetMoney = i64TradeMoney;
				kOtherGuid = kInfo.kCasterGuid;
				if(!kInfo.bTargetConfirm)	//확인을 취소했으면 다른놈도 취소
				{
					kInfo.bCasterConfirm = false;
					kInfo.bCasterOk = false;
				}
			}
			else
			{
				INFO_LOG(BM::LOG_LV0, "PT_C_M_NFY_EXCHANGE_ITEM_ITEM : Cannot Find From g_kItemTradeMgr. Trade Guid["<<kInfo.kTradeGuid.str().c_str()<<"] User : "<<pkUnit->Name().c_str());
				goto __FAILD1;
			}
 
			goto __SUCCESS1;
__FAILD1:
			{
				eRet = TR_FAIL;
				kPacket.Push(eRet);
				pkUnit->Send(kPacket);
				break;
			}

__SUCCESS1:
			{
				g_kItemTradeMgr.ModifyTrade(kInfo);
				kPacket.Push(eRet);
				kInfoPacket.WriteToPacket(kPacket);
				kPacket.Push(bConfirmOrModify);

				pkUnit->Send(kPacket);
				CUnit* pkPilot = PgObjectMgr::GetUnit(kOtherGuid);
				if (pkPilot)
				{
					pkPilot->Send(kPacket);
				}
				else
				{
					INFO_LOG(BM::LOG_LV0, __FL__<<L"PT_C_M_NFY_EXCHANGE_ITEM_ITEM : Cannot Find User From PgObjectMgr:: Guid["<<kOtherGuid<<L"]");
				}
			}
		
		}break;
	case PT_C_M_NFY_EXCHANGE_ITEM_QUIT:
		{//클라쪽에서 UI를 꺼버렸다.
			BM::GUID kTradeGuid;
			pkNfy->Pop(kTradeGuid);

			STradeInfo kTradeInfo;

			if(!g_kItemTradeMgr.FindTrade(kTradeGuid, kTradeInfo))
			{
				INFO_LOG(BM::LOG_LV0, "PT_C_M_NFY_EXCHANGE_ITEM_QUIT : Cannot Find From g_kItemTradeMgr. Trade Guid["<<kTradeGuid.str().c_str()<<"] User : "<<pkUnit->Name().c_str());
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("FindTrade is Failed!"));
				break;
			}

			BM::Stream kPacket(PT_M_C_NFY_EXCHANGE_ITEM_RESULT, (int)TR_FAIL);
			CUnit* pkCasterPilot = PgObjectMgr::GetUnit(kTradeInfo.kCasterGuid);
			if (pkCasterPilot)
			{
				pkCasterPilot->Send(kPacket);
			}
			CUnit* pkTargetPilot = PgObjectMgr::GetUnit(kTradeInfo.kTargetGuid);
			if (pkTargetPilot)
			{
				pkTargetPilot->Send(kPacket);
			}

			g_kItemTradeMgr.DelItemTrade(kTradeGuid);
		}break;
	case PT_C_M_NFY_EXCHANGE_ITEM_READY:
		{
			BM::GUID kTradeGuid;
			pkNfy->Pop(kTradeGuid);
			BM::GUID kOtherGuid = BM::GUID::NullData();
			STradeInfo kInfo;
			ETRADERESULT eRetCaster = TR_FAIL;
			ETRADERESULT eRetTarget = TR_FAIL;
			if(!g_kItemTradeMgr.FindTrade(kTradeGuid, kInfo))
			{
				INFO_LOG(BM::LOG_LV0, "PT_C_M_NFY_EXCHANGE_ITEM_READY : Cannot Find From g_kItemTradeMgr. Trade Guid["<<kTradeGuid.str().c_str()<<"] User : "<<pkUnit->Name().c_str());
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("FindTrade is Failed!"));
				break;	// goto 문 사용금지 (중간에 pkCaster/pkTarget 변수가 초기화 되지 않는다.)
			}

			if (kInfo.kCasterGuid == pkUnit->GetID())	//최초 신청자
			{
				if (kInfo.bCasterConfirm)	//고정 되 있어야 눌러진다
				{
					kInfo.bCasterOk = true;
				}

				kOtherGuid = kInfo.kTargetGuid;
			}
			else if (kInfo.kTargetGuid == pkUnit->GetID())
			{
				if (kInfo.bTargetConfirm)	//고정 되 있어야 눌러진다
				{
					kInfo.bTargetOk = true;
				}

				kOtherGuid = kInfo.kCasterGuid;
			}
			else
			{
				INFO_LOG(BM::LOG_LV0, __FL__<<L"PT_C_M_NFY_EXCHANGE_ITEM_READY : Cannot Find Guid From STradeInfo. Guid["<<kTradeGuid<<L"]");
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Not Matched Guid"));
				break;
			}

			g_kItemTradeMgr.ModifyTrade(kInfo);

			CUnit* pkCaster = PgObjectMgr::GetUnit(kInfo.kCasterGuid);
			CUnit* pkTarget = PgObjectMgr::GetUnit(kInfo.kTargetGuid);
				
			if (kInfo.bCasterOk && kInfo.bTargetOk)	//둘다 확인일 때만
			{
				eRetCaster = g_kItemTradeMgr.CheckTradeItem(kInfo.kCasterItemCont, pkCaster);
				if(TR_NONE!=eRetCaster)
				{
					goto __FAILD2;
				}

				eRetTarget = g_kItemTradeMgr.CheckTradeItem(kInfo.kTargetItemCont, pkTarget);
				if(TR_NONE!=eRetTarget)
				{
					goto __FAILD2;
				}
				

				//실제로 바꾸자
				PgAction_ExchangeItem kAction(GroundKey(), kInfo);
				if(!kAction.DoAction(pkCaster, pkTarget))
				{
					eRetCaster = TR_FAIL;
					eRetTarget = TR_FAIL;
					INFO_LOG(BM::LOG_LV6, "PT_C_M_NFY_EXCHANGE_ITEM_READY. PgAction_ExchangeItem is Fail. Caster : "<<kInfo.kCasterGuid<<" Target : "<<kInfo.kTargetGuid);
					goto __FAILD2;
				}
				g_kItemTradeMgr.DelItemTrade(kInfo.kTradeGuid);	//아이템 서버로 역할이 넘어갔다
			}
			else	//둘중에 한명만 눌렀으면 눌른 정보만 보내주자
			{
				const BM::GUID kGuid = pkUnit->GetID();
				BM::Stream kPacket(PT_M_C_NFY_EXCHANGE_ITEM_READY, kGuid);
				
				pkUnit->Send(kPacket);

				CUnit* pkOtherPilot = PgObjectMgr::GetUnit(kOtherGuid);
				if (pkOtherPilot)
				{
					pkOtherPilot->Send(kPacket);
					break;	//뭐 잘 끝났다.
				}
				else
				{
					INFO_LOG(BM::LOG_LV0, __FL__<<L"PT_C_M_NFY_EXCHANGE_ITEM_READY : Cannot Find CUnit From PgObjectMgr:: Guid["<<kOtherGuid<<L"]");
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("pkOtherPilot is Null"));
					break;
				}
			}

			break;

		__FAILD2:
			{
				if(pkCaster)
				{
					BM::Stream kPacket(PT_M_C_NFY_EXCHANGE_ITEM_RESULT, (int)eRetCaster);
					pkCaster->Send(kPacket);
				}
				if(pkTarget)
				{
					BM::Stream kPacket(PT_M_C_NFY_EXCHANGE_ITEM_RESULT, (int)eRetTarget);
					pkTarget->Send(kPacket);
				}
			
				g_kItemTradeMgr.DelItemTrade(kInfo.kTradeGuid);	//아이템 서버로 역할이 넘어갔다
			}
		}break;
	case PT_C_M_REQ_ITEM_MAKING:
		{
			PgAction_ItemMaking kAction(GroundKey(), pkNfy);
			kAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_SOULSTONE_TRADE:
		{
			PgAction_SoulStoneTrade kAction(GroundKey(), pkNfy);
			kAction.DoAction(pkUnit, NULL);
		}break;
// 	case PT_C_M_REQ_CREATE_ENTITY:
// 	case PT_C_M_REQ_DELETE_ENTITY:
// 		{
// 			GetUnitControl(pkUnit, NULL, wType, *pkNfy);
// 		}break;
	case PT_C_M_REQ_PACTION:
		{
			ReqProjectileAction(pkUnit, pkNfy);
		}break;
	case PT_C_M_REQ_VIEW_OTHER_EQUIP:
		{
			BM::GUID kTargetGUID;
			if(pkNfy->Pop(kTargetGUID))
			{
				CUnit* pkTarget = PgObjectMgr::GetUnit(kTargetGUID);

				PgAction_ViewOtherEquip kAction(GroundKey());
				kAction.DoAction(pkUnit, pkTarget);
			}
		}break;
	case PT_C_M_REQ_FOLLOWING:
		{
			EPlayer_Follow_Mode eMode;
			BM::GUID kHeadGuid;
			bool	bFriend = false;

			pkNfy->Pop(eMode);
			pkNfy->Pop(kHeadGuid);
			pkNfy->Pop(bFriend);

			CUnit* pkHead = PgObjectMgr::GetUnit(UT_PLAYER, kHeadGuid);
			
			bool bChange = false;
			if( EFollow_TakePerson == eMode )
			{
				eMode = EFollow_Request;
				bChange = true;
			}
			else if( EFollow_CancelTakePerson == eMode )
			{
				eMode = EFollow_Cancel;
				bChange = true;
			}
			
			if( !pkHead )
			{
				PgPlayer* pkCasterPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				BM::GUID const & rkHead = pkCasterPlayer->FollowingHeadGuid();
				pkHead = PgObjectMgr::GetUnit(UT_PLAYER, rkHead);
			}

			PgAction_FollowingMode kAction(eMode, bFriend, this);
			if( bChange )
			{
				kAction.DoAction(pkHead, pkUnit);
			}
			else
			{
				kAction.DoAction(pkUnit, pkHead);
			}
		}break;
	case PT_C_M_REQ_REWORD_FRAN_EXP:
		{
			BM::GUID kNpcGuid;
			pkNfy->Pop( kNpcGuid );
			CUnit* pkNpcUnit = GetNPC(kNpcGuid);
			if( !PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_Fran, __F_P__(PT_C_M_REQ_REWORD_FRAN_EXP), __LINE__) )
			{
				break;
			}

			__int64 const iFranExp = pkUnit->GetAbil64(AT_FRAN);
			__int64 const iAbsVal = static_cast<__int64>((iFranExp * FRAN_REWORD_MONEY)/100);

			__int64 const i64SendCost = iAbsVal;
			__int64 const iCasterMoney = pkUnit->GetAbil64(AT_MONEY);

			if(iCasterMoney < i64SendCost)
			{
				// 돈이 부족하다...
				pkUnit->SendWarnMessage(80024);
				break;
			}

			if( pkUnit->GetAbil(AT_LEVEL) >= PgClassDefMgr::s_sMaximumLevel )
			{
				pkUnit->SendWarnMessage(550013);
				break;
			}

			CONT_PLAYER_MODIFY_ORDER kOrder;

			SPMOD_Add_Money kDelMoneyData(-i64SendCost);//필요머니 빼기.
			kOrder.push_back(SPMO(IMET_ADD_MONEY, pkUnit->GetID(), kDelMoneyData));

			SPMOD_AddAbil kAddAbil_Fran(AT_FRAN, -iFranExp);//Fran 0으로
			kOrder.push_back(SPMO(IMET_ADD_ABIL64, pkUnit->GetID(), kAddAbil_Fran));

			SPMOD_AddAbil kAddAbilData(AT_EXPERIENCE, iFranExp);//경험치 추가.
			kOrder.push_back(SPMO(IMET_ADD_ABIL64, pkUnit->GetID(), kAddAbilData));

			if (pkUnit->GetAbil64(AT_REST_EXP_ADD_MAX) > 0)
			{
				SPMOD_AddAbil kAddAbilRest(AT_REST_EXP_ADD_MAX, iFranExp);//경험치 추가.
				kOrder.push_back(SPMO(IMET_ADD_ABIL64_MAPUSE, pkUnit->GetID(), kAddAbilRest));
			}

			BM::Stream kPacket(PT_M_C_ANS_REWORD_FRAN_EXP);//성공시 가는 메세지.
			kPacket.Push(iFranExp);

			PgAction_ReqModifyItem kItemModifyAction(MCE_Fran, GroundKey(), kOrder, kPacket);
			kItemModifyAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_LEARN_SKILL:
		{
			int iSkillNo = 0;
			pkNfy->Pop( iSkillNo );

			PgAction_LearnSkill kAction(iSkillNo, GroundKey(), CC_LearnSkill);
			int const iRet = kAction.DoAction(pkUnit);

			if( iRet != LS_RET_SUCCEEDED )
			{
				BM::Stream kPacket(PT_M_C_RES_LEARN_SKILL, iSkillNo );
				kPacket.Push( iRet );
				kPacket.Push( (short)pkUnit->GetAbil(AT_SP) );
				pkUnit->Send( kPacket, E_SENDTYPE_SELF );
			}
		}break;
	case PT_C_M_GODCMD:
	case PT_C_NT_GODCMD:
		{
			int iCmdType = 0;
			if ( pkNfy->Pop(iCmdType) )
			{
				GMCommand( dynamic_cast<PgPlayer*>(pkUnit), (EGMCmdType)iCmdType, pkNfy );
			}	
		}break;
	case PT_C_M_REQ_MSGBOX_CALL:
		{
			bool bRet = false;
			DWORD m_iItemNo = 0;

			pkNfy->Pop(bRet);
			pkNfy->Pop(m_iItemNo);

			if( !bRet )
			{
				if(m_iItemNo > 0)
				{
					pkUnit->DeleteEffect(m_iItemNo); //Effect No : 402001
				}

				/*BM::Stream kPacket(PT_M_C_ANS_MSGBOX_CALL_RESULT);
				kPacket.Push(450053);
				pkUnit->Send(kPacket);*/
			}
		}break;
	case PT_C_M_REQ_EFFECT_ESCAPE_KEYDOWN:
		{
			PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if ( pkPlayer )
			{
				int iEffectID = 0;
				pkNfy->Pop(iEffectID);

				pkPlayer->EffectEscapeKeyDown(iEffectID);
			}
		}break;
	case PT_C_M_REQ_DELETEEFFECT:
		{
			BM::GUID kCharGuid = BM::GUID::NullData();
			int iEffectNo = 0;

			pkNfy->Pop(kCharGuid);
			pkNfy->Pop(iEffectNo);

			CUnit *pkUnitPlayer = GetUnit(kCharGuid);
			if( pkUnitPlayer )
			{
				if( iEffectNo )
				{
					pkUnitPlayer->DeleteEffect(iEffectNo);
				}
			}
		}break;
	case PT_C_M_REQ_DELETE_BUFFEFFECT:
		{
			BM::GUID kCharGuid = BM::GUID::NullData();
			int iEffectNo = 0;

			pkNfy->Pop(kCharGuid);
			pkNfy->Pop(iEffectNo);

			CUnit *pkUnitPlayer = GetUnit(kCharGuid);
			if( pkUnitPlayer )
			{
				if( iEffectNo )
				{
					pkUnitPlayer->DeleteBuffEffect(iEffectNo);
				}
			}
		}break;
	case PT_C_M_REQ_CLIENT_CUSTOMDATA:
		{
			EClientCustomDataType eType = ECCDT_NONE;
			int iValue = 0;
			pkNfy->Pop(eType);
			pkNfy->Pop(iValue);

			switch(eType)
			{
			case ECCDT_SKILLTABNO:
				{
					PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
					if(pkPlayer && pkPlayer->GetQInven())
					{
						SPMOD_ADD_StrategySkillTabChange kData( *pkPlayer->GetQInven() );
						kData.iValue = iValue;

						CONT_PLAYER_MODIFY_ORDER kOrder;
						kOrder.push_back(SPMO(IMET_SET_STRATEGYSKILL_TABNO, pkUnit->GetID(), kData));

						PgAction_ReqModifyItem kAction(IMEPT_NONE, GroundKey(), kOrder);
						kAction.DoAction(pkUnit, NULL);
					}
				}break;
			}
		}break;
	case PT_C_T_REQ_SAVE_OPTION:
		{			
			ClientOption kOption;
			if ( true == pkNfy->Pop(kOption) )
			{
				// 옵션이 바뀌는것을 내 주변에 모두에게 브로드캐스팅한다.
				pkUnit->SetAbil(AT_CLIENT_OPTION_SAVE, static_cast<int>(kOption.dwShowOption), true, true);

				BM::Stream kSendContents(PT_C_T_REQ_SAVE_OPTION, pkUnit->GetID());
				kSendContents.Push(kOption);
				SendToContents(kSendContents);
			}
		}break;
	case PT_C_T_REQ_SAVE_SKILLSET:
		{
			size_t const iRDPos = pkNfy->RdPos();
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkPlayer)
			{
				PgSaveSkillSetPacket kSavePacket;
				if(kSavePacket.ReadFromPacket(*pkNfy))
				{
					pkPlayer->GetMySkill()->ContSkillSet( kSavePacket.ContSkillSet() );
				}
			}
			pkNfy->RdPos(iRDPos);

			BM::Stream kSendContents(PT_C_T_REQ_SAVE_SKILLSET, pkUnit->GetID());
			kSendContents.Push(*pkNfy);
			SendToContents(kSendContents);
		}break;
	case PT_C_M_REQ_START_HYPER_MOVE:
		{
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkPlayer)
			{
				size_t const iRDPos = pkNfy->RdPos();
				int kType = 0;
				std::string kTriggerID;
				pkNfy->Pop(kType);
				pkNfy->Pop(kTriggerID);

				CONT_GTRIGGER::const_iterator itr = m_kContTrigger.find( kTriggerID );	//보내준 이름에 맞는 트리거를 찾자
				if(m_kContTrigger.end() != itr)
				{
					CONT_GTRIGGER::mapped_type const kTrigger = (*itr).second;
					if(kTrigger)
					{
						pkNfy->RdPos( iRDPos );
						if(kTrigger->Event( pkUnit, this, pkNfy ) )	//이벤트 실행!
						{
							pkPlayer->SendHyperMoveInfoToClient();
						}
					}
				}
				else
				{
					INFO_LOG( BM::LOG_LV5, __FUNCTIONW__<<" NotFound Trigger["<<kTriggerID.c_str()<<"] GroundNo["<<GetGroundNo()<<"] / User["<<pkUnit->Name().c_str()<<"-"<<pkUnit->GetID().str().c_str()<<"]");
				}
			}
		}break;
	case PT_C_M_NFY_END_HYPER_MOVE:
		{
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if(pkPlayer)
			{
				POINT3 kEndPos;
				pkNfy->Pop(kEndPos);
				BM::GUID kGuidFromClient;
				pkNfy->Pop(kGuidFromClient);
				bool bRealyJump = false;
				pkNfy->Pop(bRealyJump);
				float fErrorDist = 0;

				HRESULT hResult = pkPlayer->EndHyperMove(kEndPos, kGuidFromClient, bRealyJump, fErrorDist);
				if (ms_kAntiHackCheckVariable.bUse_HyperMove)
				{
					switch (hResult)
					{
					case E_ANTIHACK_WRONG_GUID:
						{
							BM::vstring kLogMsg;
							kLogMsg << __FL__ << "[HACKING][HyperMove] User[" << pkPlayer->Name() << "] GUID[" << pkPlayer->GetID() 
								<< "]  MemberGUID[" << pkPlayer->GetMemberGUID() << _T("] GroundNo[") << GroundKey().GroundNo() 
								<< _T("] HackingType[HyperMove:E_ANTIHACK_WRONG_GUID]");
							//VERIFY_INFO_LOG(false, BM::LOG_LV0, kLogMsg);
							HACKING_LOG(BM::LOG_LV0, kLogMsg);
							if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
							{
								pkPlayer->SendWarnMessageStr(kLogMsg);
							}

							if (pkPlayer->SuspectHacking(EAHP_HyperMove, ms_kAntiHackCheckVariable.sHyperMove_WrongGuid_AddIndex))
							{
								BM::Stream kDPacket( PT_A_S_NFY_USER_DISCONNECT, static_cast<BYTE>(CDC_HyperMove_Hack) );
								kDPacket.Push( pkPlayer->GetMemberGUID() );
								SendToServer( pkPlayer->GetSwitchServer(), kDPacket );
							}
						}break;
					case E_ANTIHACK_DISTANCE:
						{
							BM::vstring kLogMsg;
							kLogMsg << __FL__ << "[HACKING][HyperMove] User[" << pkPlayer->Name() << "] GUID[" << pkPlayer->GetID() 
								<< "]  MemberGUID[" << pkPlayer->GetMemberGUID() << _T("] GroundNo[") << GroundKey().GroundNo() 
								<< _T("] HackingType[HyperMove:E_ANTIHACK_DISTANCE] Dist["<<fErrorDist<<"] EndPos X["<<kEndPos.x<<"] Y["<<kEndPos.y<<"] Z["<<kEndPos.z<<"] RealyJump["<<(bRealyJump?"true":"false")<<"]");
							//VERIFY_INFO_LOG(false, BM::LOG_LV0, kLogMsg);
							HACKING_LOG(BM::LOG_LV0, kLogMsg);
							if (g_kProcessCfg.RunMode() & CProcessConfig::E_RunMode_Debug)
							{
								pkPlayer->SendWarnMessageStr(kLogMsg);
							}
							if (pkPlayer->SuspectHacking(EAHP_HyperMove, ms_kAntiHackCheckVariable.sHyperMove_WrongDistance_AddIndex))
							{
								BM::Stream kDPacket( PT_A_S_NFY_USER_DISCONNECT, static_cast<BYTE>(CDC_HyperMove_Hack) );
								kDPacket.Push( pkPlayer->GetMemberGUID() );
								SendToServer( pkPlayer->GetSwitchServer(), kDPacket );
							}
						}break;
					default:	// S_OK
						{
						}break;
					}
				}
			}
		}break;
	case PT_C_M_CHECK_WORLDEVENT_CONDITION:
		{
			WORD kWorldEventID;
			POINT3 kPos;

			pkNfy->Pop( kWorldEventID );
			pkNfy->Pop( kPos );

			pkUnit->SetPos(kPos);

			CUnit * pActor = NULL; // 이벤트를 발생 시킬 캐릭터가 누구인지..(파티장/원정대장만 가능 or 파티/원정대원 아무나)
			if( GetAttr() & GKIND_EXPEDITION )		// 원정대 던전인 경우..
			{
				pActor = pkUnit;
				
				/*PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if( pPlayer )
				{
					if( BM::GUID::NullData() != pPlayer->ExpeditionGuid() )
					{
						BM::GUID MasterGuid;
						m_kLocalPartyMgr.GetExpeditionMasterGuid(pPlayer->ExpeditionGuid(), MasterGuid);
						if( MasterGuid != pPlayer->GetID() )
						{
							pActor = GetUnit(MasterGuid);
						}
					}
				}*/
			}
			else
			{
				pActor = pkUnit;
				if( BM::GUID::NullData() != pkUnit->GetPartyGuid() )
				{
					BM::GUID kMasterGuid;
					if ( GetPartyMasterGuid( pkUnit->GetPartyGuid(), kMasterGuid ) )
					{
						if ( kMasterGuid != pkUnit->GetID() )
						{
							pActor = GetUnit(kMasterGuid);
						}
					}
				}
			}
			if( NULL != pActor )
			{
				PgWorldEventMgr::DoWorldEventCondition(GetGroundNo(), kWorldEventID, pActor);
			}
		}break;
	case PT_C_M_REQ_EVENT_TW_USE_EFFECTQUEST:
		{
			BM::GUID kNpcGuid;

			pkNfy->Pop( kNpcGuid );

			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_EventTWEffectQuest, __F_P__(PT_C_M_REQ_TW_EVENT_USE_EFFECT), __LINE__) )
			{
				PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkUnit);
				if( pkPlayer )
				{
					if( 0 == pkPlayer->GetAbil(AT_ADD_EXP_PER) )	// 이미 경험치 증가 버프 받은 것이 없다면...
					{
						int const iQuestID = EventTaiwanEffectQuest::GetEnableQuestID(pkPlayer->GetMyQuest());
						if( 0 != iQuestID )
						{
							CONT_PLAYER_MODIFY_ORDER kOrder;

							SPMOD_SetEndQuest kAddQuestData(iQuestID, false, true, true);
							kOrder.push_back( SPMO(IMET_ADD_ENDQUEST, pkUnit->GetID(), kAddQuestData) );

							PgAction_ReqModifyItem kItemModifyAction(IMEPT_EVENT_TW_EFFECTQUEST, GroundKey(), kOrder);
							kItemModifyAction.DoAction(pkUnit, NULL);
						}
						else
						{
							pkUnit->SendWarnMessage(EventTaiwanEffectQuest::iNotHaveQuestMsg);
						}
					}
					else
					{
						pkUnit->SendWarnMessage(EventTaiwanEffectQuest::iCantDupExpPotionMsg);
					}
				}
			}
		}break;
	case PT_C_M_REQ_RAGNAROK_EFFECT:
		{
			BM::GUID NpcGuid;

			pkNfy->Pop( NpcGuid );

			CUnit* pNpcUnit = GetUnit(NpcGuid);
			if( PgGroundUtil::IsCanTalkableRange(pkUnit, NpcGuid, pNpcUnit, GetGroundNo(), NMT_RagnarokTheme, __F_P__(PT_C_M_REQ_RAGNAROK_EFFECT), __LINE__) )
			{
				PgNpc* pNpc = dynamic_cast<PgNpc*>(pNpcUnit);
				if( pNpc )
				{
					GET_DEF(CEffectDefMgr, EffectDefMgr);
					const CEffectDef *pDef = EffectDefMgr.GetDef(pNpc->GiveEffectNo());
					if ( !pDef )
					{
						LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
						return false;
					}

					PgPlayer* pPlayer = dynamic_cast< PgPlayer* >(pkUnit);
					if( pPlayer )
					{
						pPlayer->AddEffectQueue(EffectQueueData(EQT_ADD_EFFECT, pNpc->GiveEffectNo()));
					}
					else
					{
						LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
						return false;
					}
				}
				else
				{
					LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Return false"));
					return false;
				}
			}
		}break;
	case PT_C_M_REQ_EVENT_QUEST_TALK:
		{
			BM::GUID kNpcGuid;
			bool bIntro = false;

			pkNfy->Pop( kNpcGuid );
			pkNfy->Pop( bIntro );

			EventQuestTalkReturn eRet = EQTR_Time;
			CUnit* pkNpcUnit = GetUnit(kNpcGuid);
			if( pkNpcUnit )
			{
				if( PgGroundUtil::IsCanTalkableRange(pkUnit, kNpcGuid, pkNpcUnit, GetGroundNo(), NMT_EventQuestSys, __F_P__(PT_C_M_REQ_EVENT_QUEST_TALK), __LINE__) )
				{
					if( m_kEventQuest.IsCanRun(BM::PgPackedTime::LocalTime()) )
					{
						eRet = EQTR_NotEnd;
						PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkUnit);
						if( pkPlayer )
						{
							BM::Stream kPacket(PT_M_N_REQ_EVENT_QUEST_CHECKUSER);
							kPacket.Push( pkUnit->GetID() );
							kPacket.Push( kNpcGuid );
							::SendToRealmContents(PMET_EVENTQUEST, kPacket);
						}
					}
				}
			}

			if( EQTR_None != eRet )
			{
				BM::Stream kPacket(PT_M_C_ANS_EVENT_QUEST_TALK);
				kPacket.Push( static_cast< int >(eRet) );
				kPacket.Push( kNpcGuid );
				pkUnit->Send( kPacket );
			}
		}break;
	case PT_C_M_REQ_PET_RENAME:
		{
			SItemPos kItemPos;
			std::wstring wstrName;
			pkNfy->Pop( kItemPos );
			pkNfy->Pop( wstrName );

			/*if ( LOCAL_MGR::NC_TAIWAN == g_kLocal.ServiceRegion() )
			{
				PgActPet_Rename kAction( this, kItemPos );
				kAction.DoAction( pkUnit, wstrName );
			}
			else*/
			{
				SItemPos kRenameItemPos;
				if ( true == pkNfy->Pop( kRenameItemPos ) )
				{
					PgActPet_Rename_ForItem kAction( this, kItemPos, kRenameItemPos );
					kAction.DoAction( pkUnit, wstrName );
				}
			}
		}break;
	case PT_C_M_REQ_PET_COLOR_CHANGE:
		{
			SItemPos kItemPos;
			char cColorIndex = 0;
			pkNfy->Pop( kItemPos );
			pkNfy->Pop( cColorIndex );

			bool bSuccess = false;
			/*if ( LOCAL_MGR::NC_TAIWAN == g_kLocal.ServiceRegion() )
			{
				PgActPet_ColorChange kAction( this, kItemPos );
				bSuccess = kAction.DoAction( pkUnit, cColorIndex );
			}
			else*/
			{
				SItemPos kColorChangeItemPos;
				if ( true == pkNfy->Pop( kColorChangeItemPos ) )
				{
					PgActPet_ColorChange_ForItem kAction( this, kItemPos, kColorChangeItemPos );
					bSuccess = kAction.DoAction( pkUnit, cColorIndex );
				}
			}

			if(bSuccess)
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if ( pkPlayer )
				{
					pkPlayer->SendWarnMessage(7520, EL_Normal);
				}
			}
		}break;
	case PT_C_M_REQ_ITEM_CHANGE_TO_PET:
		{
			SItemPos kPlayerItemPos;
			SItemPos kPetItemPos;
			DWORD dwClientTime = 0;
			bool bMovePet = false;
			pkNfy->Pop( kPlayerItemPos );
			pkNfy->Pop( kPetItemPos );
			pkNfy->Pop( dwClientTime );
			pkNfy->Pop( bMovePet );

			PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if ( pkPlayer )
			{
				DWORD dwServerTime = 0;
				if ( S_OK == ::CheckClientNotifyTime(pkPlayer, dwClientTime, dwServerTime) )
				{
					PgPet * pkPet = GetPet( pkPlayer );
					if ( pkPet )
					{
						PgAction_MoveItem kActionMoveItem( kPlayerItemPos, kPetItemPos, this, dwClientTime, bMovePet );
						kActionMoveItem.DoAction( pkUnit, pkPet );
					}
					else
					{
						BM::GUID const kPetID = pkPlayer->SelectedPetID();
						if ( BM::GUID::IsNotNull( kPetID ) )
						{
							PgAction_PopPetItem kActionPopPetItem( kPetID, kPetItemPos, kPlayerItemPos, this );
							kActionPopPetItem.DoAction( pkUnit, NULL );
						}
					}
				}
				else
				{
					INFO_LOG(BM::LOG_LV5, __FL__ << _T("Player kicked, Cause[CDC_SuspectedSpeedHack], Name=") << pkPlayer->Name());
					// SpeedHack이 의심스러우니 강제접속 해제 시킨다.
					BM::Stream kDPacket( PT_A_S_NFY_USER_DISCONNECT, static_cast<BYTE>(CDC_SuspectedSpeedHack) );
					kDPacket.Push( pkPlayer->GetMemberGUID() );
					SendToServer( pkPlayer->GetSwitchServer(), kDPacket );
				}
			}
		}break;
	case PT_C_M_REQ_PET_ACTION:
		{
			Recv_PT_C_M_REQ_PET_ACTION( dynamic_cast<PgPlayer*>(pkUnit), pkNfy );
		}break;
	case PT_C_M_REQ_PET_UPGRADE:
		{
			SItemPos kPetItemPos;
			DWORD dwClientTime = 0;
			pkNfy->Pop( kPetItemPos );
			pkNfy->Pop( dwClientTime );

			PgInventory* pkInven = pkUnit->GetInven();
			if(NULL == pkInven)
			{
				return false;
			}
			PgBase_Item kPetItem;
			if ( S_OK != pkInven->GetItem( kPetItemPos, kPetItem ) )
			{
				return false;
			}

			PgItem_PetInfo *pkPetInfo = NULL;
			if ( !kPetItem.GetExtInfo( pkPetInfo ) )
			{
				return false;
			}

			int iGrade = PgClassPetDef::GetClassGrade( pkPetInfo->ClassKey().iClass);
			if(iGrade >= 2)
			{
				pkUnit->SendWarnMessage(3417); //MSG: 3단계 등급의 펫은 전직을 할 수 없다
				return false;
			}

			__int64 const i64HaveMoney = pkUnit->GetAbil64( AT_MONEY );
			__int64 i64UpgradeCost = PgClassPetDef::GetPetUpgradeCost(pkPetInfo);
			if(i64UpgradeCost > i64HaveMoney)
			{ //돈이 부족한가?
				pkUnit->SendWarnMessage(700036); //소지금이 부족합니다.
				return false;
			}

			//펫 전직
			CONT_PLAYER_MODIFY_ORDER kOrder;
			PgActPet_MakeClassChangeOrder kAction( kOrder, false, kPetItem, kPetItemPos );
			if ( false == kAction.DoAction( pkUnit, iGrade + 1 ) )
			{
				return false;
			}
			PgAction_ReqModifyItem kItemModifyAction( CIE_SetAbilPet, GroundKey(), kOrder, BM::Stream(), true );//죽음무시
			kItemModifyAction.DoAction( pkUnit->GetID() );

			//전직 비용 감소
			SPMOD_Add_Money kDelMoneyData(-i64UpgradeCost);//필요머니 빼기.
			SPMO kIMO(IMET_ADD_MONEY, pkUnit->GetID(), kDelMoneyData);
			kOrder.push_back(kIMO);
			PgAction_ReqModifyItem kItemModifyMoneyAction( CIE_PetUpgrade, GroundKey(), kOrder );
			if(true == kItemModifyMoneyAction.DoAction( pkUnit, NULL ) )
			{
				BM::Stream kPacket(PT_M_C_NFY_PET_UPGRADE);
				pkUnit->Send(kPacket);
			}

		}break;
	case PT_C_T_REQ_WANT_JOIN_BS_CHANNEL:
		{
			PgPlayer* pkPlayer = dynamic_cast< PgPlayer* >(pkUnit);
			if( pkPlayer )
			{
				if( BM::GUID::IsNull(pkPlayer->GetPartyGuid()) )
				{
					BM::Stream kPacket( wType );
					kPacket.Push( pkPlayer->GetID() );
					kPacket.Push( pkPlayer->GetAbil(AT_LEVEL) );
					kPacket.Push( *pkNfy );
					::SendToRealmContents(PMET_BATTLESQUARE, kPacket);
				}
				else
				{
					BM::Stream kPacket(PT_T_C_ANS_WANT_JOIN_BS_CHANNEL);
					kPacket.Push( BSJR_PARTY );
					pkPlayer->Send(kPacket);
				}
			}
		}break;
	case PT_C_M_REQ_STATUS_CHANGE:
		{
			VEC_ABILINFO kAbilVec;
			pkNfy->Pop(kAbilVec);
			// Data verify
			VEC_ABILINFO::const_iterator itor_abil = kAbilVec.begin();
			if (kAbilVec.size() > 4)
			{
				// hacking user
				return true;
			}
			HRESULT hResult = S_OK;
			SPMO_ActiveStatus kSPMO;
			while (itor_abil != kAbilVec.end())
			{
				if ((*itor_abil).iValue < 0)
				{
					hResult = E_WRONG_REQUEST;
					break;
				}
				switch ((*itor_abil).wType)
				{
				case AT_STR_ADD:	{ kSPMO.Str((*itor_abil).iValue); }break;
				case AT_INT_ADD:	{ kSPMO.Int((*itor_abil).iValue); }break;
				case AT_CON_ADD:	{ kSPMO.Con((*itor_abil).iValue); }break;
				case AT_DEX_ADD:	{ kSPMO.Dex((*itor_abil).iValue); }break;
				default:
					{
						VERIFY_INFO_LOG(false, BM::LOG_LV5, __FL__ << _T("[PT_C_M_REQ_STATUS_CHANGE] Cannot set abil in this packet Abil=") << wType);
						hResult = E_WRONG_REQUEST;
					}break;
				}
				++itor_abil;
			}
			if (hResult == S_OK)
			{
				BM::Stream kAddonPacket(PT_M_C_ANS_STATUS_CHANGE);
				CONT_PLAYER_MODIFY_ORDER kOrder;
				kOrder.push_back(SPMO(IMET_ACTIVE_STATUS, pkUnit->GetID(), kSPMO));

				PgAction_ReqModifyItem kAction(CIE_ACTIVE_STATUS_SET, GroundKey(), kOrder, kAddonPacket);
				kAction.DoAction(pkUnit, NULL);
			}
			else
			{
				BM::Stream kEPacket(PT_M_C_ANS_STATUS_CHANGE, hResult);
				pkUnit->Send(kEPacket);
			}
		}break;
	case PT_C_M_REQ_HIDDEN_MOVE_CHECK:
		{
			OnReqItemAction( dynamic_cast<PgPlayer*>(pkUnit), pkNfy );
		}break;
	case PT_C_M_REQ_HIDDEN_ITEM:
		{
			int iItemNo = 0;
			SItemPos rkItemInvPos;

			pkNfy->Pop( iItemNo );
			pkNfy->Pop( rkItemInvPos );

			BM::DBTIMESTAMP_EX kNow;
			g_kEventView.GetLocalTime(kNow);

			if( pkUnit )
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if( pkPlayer )
				{
					BM::Stream kCPacket(PT_M_C_ANS_HIDDEN_ITEM);
					kCPacket.Push(kNow);
					kCPacket.Push(iItemNo);
					kCPacket.Push(rkItemInvPos);
					pkPlayer->Send(kCPacket);
				}
			}
		}break;
	case PT_C_M_REQ_HIDDEN_REWORDITEM:
		{
			if( pkUnit )
			{
				HiddenMapRewordItem(pkUnit);
			}
		}break;
	case PT_C_M_REQ_HIDDEN_GIVE_REWORDITEM:
		{
			if( pkUnit )
			{
				PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				if( pkPlayer )
				{
					size_t const iItemCount = pkUnit->GetInven()->GetTotalCount(ms_iHiddenRewordItemNo);
					if( 0 < iItemCount )
					{
						int const iLevel = pkPlayer->GetAbil(AT_LEVEL);
						int const iNeedItemCount = m_kHiddenRewordItemMgr.GetMyItem_ViewItemNo(iLevel, iItemCount, true);
						int const iItemNo = m_kHiddenRewordItemMgr.GetMyItem_ViewItemNo(iLevel, iNeedItemCount);

						typedef std::vector< PgBase_Item > RESULT_SITEM_BAG_LIST_CONT;	
						RESULT_SITEM_BAG_LIST_CONT kContItem;
						CONT_PLAYER_MODIFY_ORDER kOrder;

						if( true == m_kHiddenRewordItemMgr.GiveHiddenRewordItem(kOrder, kContItem, pkUnit, iItemNo, iNeedItemCount, PgGround::ms_iHiddenRewordItemNo) )
						{
							SItemPos kItemPos;

							BM::Stream kPacket;
							kContItem.at(0).WriteToPacket( kPacket );
							kPacket.Push(kItemPos);

							PgAction_ReqModifyItem kItemModifyAction(CIE_HiddenReword, GroundKey(), kOrder, kPacket);
							kItemModifyAction.DoAction(pkUnit, NULL);
						}
					}
				}
			}
		}break;
	case PT_C_T_REQ_BS_CHANNEL_INFO:
		{
			BM::Stream kPacket(PT_C_T_REQ_BS_CHANNEL_INFO);
			kPacket.Push( pkUnit->GetID() );
			::SendToRealmContents(PMET_BATTLESQUARE, kPacket);
		}break;
	case PT_C_M_REQ_DEFENCE_ITEM_USE:
		{
			Defence7ItemUse(pkUnit, pkNfy);
		}break;
	case PT_C_M_REQ_DEFENCE_PARTYLIST:
		{
			SendDefencePartyAllList(pkUnit);
		}break;
	case PT_C_M_REQ_GENMONSTER_GROUP_NUM:
		{
			int const iCount = GetMonRegenPointGroupTotalCount();
			Broadcast( BM::Stream(PT_M_C_ANS_GENMONSTER_GROUP_NUM, iCount) );
		}break;
	case PT_C_M_REQ_CANCEL_JOBSKILL:
		{// 클라로 부터 온 채집 스킬 취소 요청 처리
			std::string kTriggerID;
			pkNfy->Pop(kTriggerID);
			if(!kTriggerID.empty())
			{
				BM::GUID kGuid;
				pkNfy->Pop(kGuid);
				if(kGuid.IsNotNull())
				{
					JobSkillLocationMgr().DelUser(pkUnit, kTriggerID, kGuid);
				}
			}
		}break;
	case PT_C_M_REQ_LEARN_JOBSKILL:
		{
			int iSkillNo = 0;
			pkNfy->Pop(iSkillNo);
			PgAction_JobSkill_Learn kAction(GroundKey());
			kAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_DELETE_JOBSKILL:
		{
			int iSkillNo = 0;
			pkNfy->Pop(iSkillNo);
			PgAction_JobSkill_Delete kAction(GroundKey());
			kAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_JOBSKILL3_CREATEITEM:
		{
			PgAction_JobSkill3_CreateItem kAction(GroundKey(), *pkNfy);
			kAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_INTERACTIVE_EMOTION_REQUEST:
		{// 인터펙티브 이모션 1.나 얘랑 이모션 할래
			int iActionNo = 0;
			pkNfy->Pop(iActionNo);
			if(iActionNo)
			{
				BM::GUID kReqPlayerGUID;
				pkNfy->Pop(kReqPlayerGUID);

				BM::GUID kTargetPlayerGUID;
				pkNfy->Pop(kTargetPlayerGUID);

				CUnit* pkTargetUnit = GetUnit(kTargetPlayerGUID);
				if(pkTargetUnit)
				{// 2.너 쟤랑 이모션 할꺼야?
					BM::Stream kPacket(PT_M_C_NFY_INTERACTIVE_EMOTION_REQUEST);
					kPacket.Push(iActionNo);
					kPacket.Push(kReqPlayerGUID);
					pkTargetUnit->Send(kPacket);
				}
			}
		}break;
	case PT_C_M_ANS_INTERACTIVE_EMOTION_REQUEST:
		{// 3. 걔랑 이모션 할(안할)꺼야.
			bool bAnswer = false;
			pkNfy->Pop(bAnswer);

			int iActionNo = 0;
			pkNfy->Pop(iActionNo);

			BM::GUID kReqPlayerGUID;
			pkNfy->Pop(kReqPlayerGUID);

			BM::GUID kTargetPlayerGUID;
			pkNfy->Pop(kTargetPlayerGUID);

			CUnit* pkReqUnit= GetUnit(kReqPlayerGUID);
			if(pkReqUnit)
			{// 4.너네 둘이 이모션해
				BM::Stream kPacket(PT_M_C_NFY_DO_INTERACTIVE_EMOTION);
				kPacket.Push(bAnswer);
				if(bAnswer)
				{
					kPacket.Push(iActionNo);
					//kPacket.Push(kTargetPlayerGUID);
				}
				pkReqUnit->Send(kPacket);
			}
			
		}break;
	case PT_C_M_REQ_SOULTRANSFER_EXTRACT:
		{
			PgAction_SoulTransfer_Extract kItemModifyAction(GroundKey(), *pkNfy);
			kItemModifyAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_SOULTRANSFER_TRANSITION:
		{
			PgAction_SoulTransfer_Transition kItemModifyAction(GroundKey(), *pkNfy);
			kItemModifyAction.DoAction(pkUnit, NULL);
		}break;
	case PT_C_M_REQ_OPEN_TREASURE_CHEST:
		{
			SItemPos kChestPos, kKeyPos;
			DWORD dwClientTime;
			pkNfy->Pop(kChestPos);
			pkNfy->Pop(kKeyPos);
			pkNfy->Pop(dwClientTime);

			PgAction_ReqOpenTreasureChest kAction(GroundKey(), kChestPos, kKeyPos);
			kAction.DoAction(pkUnit, pkUnit);
		}break;
	case PT_C_M_REQ_TREASURE_CHEST_INFO:
		{
			DWORD dwClientTime;
			pkNfy->Pop(dwClientTime);

			//보물상자 테이블 목록을 요청하면 바로 보내준다.
			TABLE_LOCK(CONT_TREASURE_CHEST) kObjLock;
			g_kTblDataMgr.GetContDef(kObjLock);
			const CONT_TREASURE_CHEST* pkCont = kObjLock.Get();
			if(pkCont)
			{
				BM::Stream kPacket(PT_M_C_ANS_TREASURE_CHEST_INFO);
				kPacket.Push(S_OK);
				PU::TWriteTable_AM(kPacket, *pkCont);
				pkUnit->Send(kPacket);
			}
			else
			{
				BM::Stream kPacket(PT_M_C_ANS_TREASURE_CHEST_INFO);
				kPacket.Push(E_TREASURE_CHEST_FAILED_TO_GET_TABLE);
				pkUnit->Send(kPacket);
			}
		}break;
	case PT_C_M_REQ_MANUFACTURE:
		{
			PgAction_Manufacture kAction(GroundKey(), *pkNfy);
			kAction.DoAction(pkUnit, pkUnit);
		}break;
	case PT_C_M_REQ_BUNDLE_MANUFACTURE:
		{
			PgAction_BundleManufacture kAction(GroundKey(), *pkNfy);
			kAction.DoAction(pkUnit, pkUnit);
		}break;
	case PT_C_M_REQ_ELEMENT_EXTRACT:
		{
			PgAction_Extract_Element kAction(GroundKey(), *pkNfy);
			kAction.DoAction(pkUnit, pkUnit);
		}break;
	case PT_C_M_REQ_OPEN_DEFENCE_WINDOW:
	case PT_C_M_REQ_ENTER_DEFENCE:
		{
			PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pPlayer
				&& pPlayer->HaveParty() )
			{
				BM::GUID MasterGuid;
				m_kLocalPartyMgr.GetPartyMasterGuid(pPlayer->PartyGuid(), MasterGuid);

				if( MasterGuid == pPlayer->GetID() )
				{
					VEC_GUID MemberList;
					m_kLocalPartyMgr.GetPartyMemberGround(pPlayer->PartyGuid(), GroundKey(), MemberList);

					BM::Stream NfyPacket;
					switch( wType )
					{
					case PT_C_M_REQ_OPEN_DEFENCE_WINDOW:
						{
							NfyPacket.Push(PT_M_C_NFY_OPEN_DEFENCE_WINDOW);
						}break;
					case PT_C_M_REQ_ENTER_DEFENCE:
						{
							NfyPacket.Push(PT_M_C_NFY_ENTER_DEFENCE);
						}break;
					}
					SendToUser_ByGuidVec(MemberList, NfyPacket);
				}
			}
		}break;
	case PT_C_M_REQ_CANCLE_DEFENCE:
		{
			PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pPlayer )
			{
				if( pPlayer->HaveParty() )
				{
					BM::GUID MasterGuid;
					m_kLocalPartyMgr.GetPartyMasterGuid(pPlayer->PartyGuid(), MasterGuid);
					if( pPlayer->GetID() == MasterGuid )
					{
						BM::Stream Packet(PT_M_C_NFY_CANCLE_DEFENCE);
						VEC_GUID CharList;
						m_kLocalPartyMgr.GetPartyMemberGround(pPlayer->PartyGuid(), GroundKey(), CharList);
						SendToUser_ByGuidVec(CharList, Packet);
					}
				}
			}
		}break;
	case PT_C_M_REQ_ELEMENT_GROUND_MOVE:
		{
			int iSuperGroundNo = 0;
			int iSuperGroundMode = 0;
			pkNfy->Pop(iSuperGroundNo);
			pkNfy->Pop(iSuperGroundMode);

			SuperGroundUtil::ReqEnterSuperGround(pkUnit, GroundKey(), iSuperGroundNo, iSuperGroundMode, 1, 1, NULL, true);
		}break;
	case PT_C_M_REQ_ALARM_MINIMAP:
		{
			POINT2 MousePos;
			pkNfy->Pop(MousePos);

			PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pPlayer )
			{
				if( GATTR_PVP & GetAttr() )
				{
					VEC_GUID CharList;
					CUnit * pUser = NULL;
					int const MyTeam = pPlayer->GetAbil(AT_TEAM);
					if( MyTeam )
					{
						CONT_OBJECT_MGR_UNIT::iterator unit_iter;
						PgObjectMgr::GetFirstUnit(UT_PLAYER, unit_iter);
						while( (pUser = PgObjectMgr::GetNextUnit(UT_PLAYER, unit_iter)) != NULL )
						{
							if( MyTeam == pUser->GetAbil(AT_TEAM) )
							{
								CharList.push_back(pUser->GetID());
							}
						}
					}
					BM::Stream Packet(PT_M_C_NFY_ALARM_MINIMAP);
					Packet.Push(MousePos);
					SendToUser_ByGuidVec(CharList, Packet);
				}
				else if( pPlayer->HaveParty() )
				{
					BM::Stream Packet(PT_M_C_NFY_ALARM_MINIMAP);
					Packet.Push(MousePos);
					VEC_GUID CharList;
					m_kLocalPartyMgr.GetPartyMemberGround(pPlayer->PartyGuid(), GroundKey(), CharList);
					SendToUser_ByGuidVec(CharList, Packet);
				}
			}
		}break;
	case PT_C_M_REQ_TRIGGER_ONENTER:
		{
			std::string TriggerName;
			pkNfy->Pop(TriggerName);
			PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pPlayer )
			{
				if( GATTR_PVP & GetAttr() )
				{
					VEC_GUID CharList;
					CUnit * pUser = NULL;
					int const MyTeam = pPlayer->GetAbil(AT_TEAM);
					if( MyTeam )
					{
						CONT_OBJECT_MGR_UNIT::iterator unit_iter;
						PgObjectMgr::GetFirstUnit(UT_PLAYER, unit_iter);
						while( (pUser = PgObjectMgr::GetNextUnit(UT_PLAYER, unit_iter)) != NULL )
						{
							if( MyTeam != pUser->GetAbil(AT_TEAM) )
							{
								CharList.push_back(pUser->GetID());
							}
						}
					}
					BM::Stream Packet(PT_M_C_NFY_TRIGGER_ONENTER);
					Packet.Push(TriggerName);
					Packet.Push(pPlayer->GetID());
					SendToUser_ByGuidVec(CharList, Packet);
				}
			}
		}break;
	case PT_C_M_REQ_TRIGGER_ONLEAVE:
		{
			PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pPlayer )
			{
				if( GATTR_PVP & GetAttr() )
				{
					VEC_GUID CharList;
					CUnit * pUser = NULL;
					int const MyTeam = pPlayer->GetAbil(AT_TEAM);
					if( MyTeam )
					{
						CONT_OBJECT_MGR_UNIT::iterator unit_iter;
						PgObjectMgr::GetFirstUnit(UT_PLAYER, unit_iter);
						while( (pUser = PgObjectMgr::GetNextUnit(UT_PLAYER, unit_iter)) != NULL )
						{
							if( MyTeam != pUser->GetAbil(AT_TEAM) )
							{
								CharList.push_back(pUser->GetID());
							}
						}
					}
					BM::Stream Packet(PT_M_C_NFY_TRIGGER_ONLEAVE);
					Packet.Push(pPlayer->GetID());
					SendToUser_ByGuidVec(CharList, Packet);
				}
			}
		}break;
	case PT_C_M_REQ_SCORE_TRIGGER_ONENTER:
		{
			size_t const iRDPos = pkNfy->RdPos();
			std::string TriggerID;
			pkNfy->Pop(TriggerID);

			CONT_GTRIGGER::const_iterator itr = m_kContTrigger.find( TriggerID );	//보내준 이름에 맞는 트리거를 찾자
			if( m_kContTrigger.end() != itr )
			{
				CONT_GTRIGGER::mapped_type const Trigger = (*itr).second;
				if( Trigger )
				{
					pkNfy->RdPos(iRDPos);
					
					if( !Trigger->Event(pkUnit, this, pkNfy) )
					{
						//
					}
				}
			}
			else
			{
				INFO_LOG( BM::LOG_LV5, __FUNCTIONW__ << " NotFound Trigger[" << TriggerID.c_str() << "] GroundNo["<< GetGroundNo() << "] / User[" << pkUnit->Name().c_str() << "-" << pkUnit->GetID().str().c_str() << "]");
			}
		}break;
	case PT_C_M_REQ_REGIST_CHAT_BLOCK:
		{
			BYTE BlockType = 0;
			std::wstring BlockName;
			
			pkNfy->Pop(BlockName);
			pkNfy->Pop(BlockType);

			PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pPlayer )
			{	// 캐릭터의 차단리스트에 등록.
				if( BlockName == pPlayer->Name() )
				{	// 자기 자신은 차단할 수 없다.
					pPlayer->SendWarnMessage(404027);
				}
												
				CONT_BLOCKCHAT_LIST ChatBlockList;
				pPlayer->GetChatBlockList(ChatBlockList);
				if( ChatBlockList.size() >= MAX_BLOCKCHAT_LIST )
				{	// 더 이상 추가할 수없다.
					pPlayer->SendWarnMessage(404026);
				}
				
				bool bDuplicate = pPlayer->CheckChatBlockList(BlockName);
				if( bDuplicate  )
				{	// 중복.
					pPlayer->SendWarnMessage(404025);	// 이미 차단된 캐릭터.					
				}
				else
				{	// 중복 되지 않고 제대로 리스트에 등록이 되었다면.
					BM::Stream Packet(PT_M_T_REQ_REGIST_CHAT_BLOCK);
					Packet.Push(pPlayer->GetID());
					Packet.Push(BlockName);
					Packet.Push(BlockType);
					SendToContents(Packet);
				}
			}
		}break;
	case PT_C_M_REQ_UNREGIST_CHAT_BLOCK:
		{
			std::wstring BlockName;

			pkNfy->Pop(BlockName);

			PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pPlayer )
			{
				bool bExist = pPlayer->CheckChatBlockList(BlockName);
				if( bExist )
				{
					pPlayer->RemoveChatBlockList(BlockName);

					BM::Stream Packet(PT_M_T_REQ_UNREGIST_CHAT_BLOCK);
					Packet.Push(pPlayer->GetID());
					Packet.Push(BlockName);
					SendToContents(Packet);
				}
				else
				{	// 리스트에 없는데 삭제 시도.
					pPlayer->SendWarnMessage(404038);	// 잘못된 아이디 입니다.
				}
			}
		}break;
	case PT_C_M_REQ_CHANGE_CHAT_BLOCK_OPTION:
		{
			BYTE BlockType = 0;
			std::wstring BlockName;

			pkNfy->Pop(BlockName);
			pkNfy->Pop(BlockType);

			PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			if( pPlayer )
			{
				bool bExist = pPlayer->CheckChatBlockList(BlockName);
				if( bExist )
				{
					pPlayer->ModifyChatBlockCharacter(BlockName, BlockType);
					BM::Stream Packet(PT_M_T_REQ_CHANGE_CHAT_BLOCK_OPTION);
					Packet.Push(pPlayer->GetID());
					Packet.Push(BlockName);
					Packet.Push(BlockType);
					SendToContents(Packet);
				}
				else
				{	// 리스트에 없는데 변경 시도.
					pPlayer->SendWarnMessage(404038);	// 잘못된 아이디 입니다.
				}
			}
		}break;
	case PT_C_M_REQ_MOVE_EVENT_GROUND:
		{	// 이벤트 맵으로 이동 요청.
			BM::Stream kPacketForLog(*pkNfy);
			EErrorEventMapMove Result = Recv_PT_C_M_REQ_MOVE_EVENT_GROUND(pkUnit, *pkNfy);
			if( Result != EEMM_SUCCESS )
			{
				BM::Stream FailPacket(PT_M_C_ANS_MOVE_EVENT_GROUND);
				FailPacket.Push(Result);

				pkUnit->Send(FailPacket);

				//입장 실패시 로그DB에 기록 남김
				{
					int EventNo = 0;
					kPacketForLog.Pop(EventNo);

					SYSTEMTIME time;
					g_kEventView.GetLocalTime(&time);

					PgLogCont LogCont(ELogMain_Community_Event, ELogSub_Community_Event_Failed_Enter);
					LogCont.GroundNo(GroundKey().GroundNo());			// 그라운드 번호.
					LogCont.ChannelNo( g_kProcessCfg.ChannelNo() );		// 채널 번호.

					PgLog Log(ELOrderMain_CommunityEvent, ELOrderSub_Fail);
					Log.Set(0, static_cast<int>(EventNo) );		// 요청한 이벤트 번호.

					wchar_t wszTemp[200] = {0,};
					swprintf_s(wszTemp, 200, _T("%d-%d-%d %d:%d:%d"), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

					std::wstring LogMsg(wszTemp);

					Log.Set(0, LogMsg);		// 현재 시간.

					LogCont.Add(Log);
					LogCont.Commit();
				}
			}
		}break;
	case PT_C_M_REQ_EVENT_PROGRESS_INFO:
		{
			SYSTEMTIME time;
			g_kEventView.GetLocalTime(&time);

			// 유저가 진행 중인 이벤트 정보를 요청할 때 남기는 로그.
			PgLogCont LogCont(ELogMain_Community_Event, ELogSub_Community_Event_NowEvent);
			LogCont.GroundNo(GroundKey().GroundNo());				// 그라운드 번호.
			LogCont.ChannelNo( g_kProcessCfg.ChannelNo() ); 		// 채널 번호.

			PgLog Log(ELOrderMain_CommunityEvent);

			int LogIndex = 0;
			VEC_INT ProgressEventList;
			CONT_EVENT_MAP_ENTERABLE_ENTER_LIST::const_iterator map_iter = m_EventMapEnterableList.begin();
			for( ; map_iter != m_EventMapEnterableList.end() ; ++map_iter)
			{
				if( map_iter->second )
				{
					ProgressEventList.push_back(map_iter->first);
					if( LogIndex < 4 )
					{	// 최대 남길수 있는 로그의 메세지 숫자가 4개.
						Log.Set(LogIndex, static_cast<int>(map_iter->first));
						++LogIndex;
					}
				}
			}

			BM::Stream AnsPacket(PT_M_C_ANS_EVENT_PROGRESS_INFO);
			PU::TWriteArray_A(AnsPacket, ProgressEventList);
			pkUnit->Send(AnsPacket);

			wchar_t wszTemp[200] = {0,};
			swprintf_s(wszTemp, 200, _T("%d-%d-%d %d:%d:%d"), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

			std::wstring LogMsg(wszTemp);

			Log.Set(0, LogMsg);		// 현재 시간.

			LogCont.Add(Log);
			LogCont.Commit();
		}break;
	case PT_C_M_NFY_PROGRESS_POS:
		{
			PgPlayer * pPlayer = dynamic_cast<PgPlayer *>(pkUnit);
			if( NULL != pPlayer )
			{
				int iProgressPos = 0;
				pkNfy->Pop(iProgressPos);
				pPlayer->SetAbil(AT_PARTY_MASTER_PROGRESS_POS, iProgressPos, true, true);
			}
		}break;
	case PT_C_M_REQ_INDUN_PARTY_ENTER:
		{
			PgPlayer * pPlayer = dynamic_cast<PgPlayer *>(pkUnit);
			if( NULL != pPlayer )
			{
				if( BM::GUID::IsNotNull(pPlayer->PartyGuid()) )
				{
					pPlayer->SendWarnMessage(401360);
				}
				else
				{
					BM::Stream Packet(PT_C_M_REQ_INDUN_PARTY_ENTER);
					Packet.Push(pPlayer->GetID());
					Packet.Push(*pkNfy);

					SendToGlobalPartyMgr(Packet);
				}
			}
		}break;
	case PT_C_T_NFY_MAPMOVE_TO_PARTYMGR:
		{
			BM::Stream Packet(PT_C_T_NFY_MAPMOVE_TO_PARTYMGR);
			Packet.Push(pkUnit->GetID());

			SendToGlobalPartyMgr(Packet);
		}break;
	case PT_C_M_REQ_CHECK_CAN_KICK:
		{
			BM::vstring Type;
			BM::GUID CharGuid;
			pkNfy->Pop(Type);
			pkNfy->Pop(CharGuid);
 
			if( (BM::GUID::NullData() != CharGuid)
				&& (BM::GUID::NullData() != pkUnit->GetPartyGuid()) )
			{
				BM::GUID MasterGuid;
				if ( GetPartyMasterGuid( pkUnit->GetPartyGuid(), MasterGuid ) )
				{
					if( Type == "KICKOUT" )
					{
						if ( MasterGuid != pkUnit->GetID() )
						{// 파티장만 추방 할 수 있다.
							pkUnit->SendWarnMessage(700064);
							return false;
						}
					}

					PgPlayer * pPlayer = dynamic_cast<PgPlayer*>(pkUnit);
					if( pPlayer )
					{
						if( BM::GUID::NullData() != pPlayer->GetExpeditionGuid() )
						{// 원정대에 속해 있는가?
							return false;
						}
					}

					bool bAbleKick = false;
					if( true == PgGroundUtil::CheckCanKickGround(GetAttr()) )
					{// 추방/탈퇴 제한이 없는 맵이라면
						bAbleKick = true;
					}
					else
					{// 추방/탈퇴 제한이 있는 맵이라면 유저가 파티장과 같은 맵에 있는지 검사한다.
						int ErrorMsg = 0;
						BM::GUID TargetGuid;
						if( Type == "KICKOUT" )
						{
							ErrorMsg = 700143;
							TargetGuid = CharGuid;
						}
						else if( Type == "LEAVE" )
						{
							ErrorMsg = 401365;
							TargetGuid = MasterGuid;
						}

						CUnit * pUnit = PgObjectMgr::GetUnit(TargetGuid);
						if( pUnit )
						{// 같은 맵에 있다.
							pkUnit->SendWarnMessage(ErrorMsg);
							bAbleKick = false;
						}
						else
						{// 같은 맵에 없다.
							bAbleKick = true;
						}
					}
					
					if( bAbleKick )
					{
						BM::Stream Packet(PT_M_C_ANS_CHECK_CAN_KICK);
						Packet.Push(Type);
						Packet.Push(CharGuid);
						Packet.Push(bAbleKick);
						pkUnit->Send(Packet);
					}
				}
			}
		}break;
	case PT_C_M_NFY_RACE_CHECK_POINT:
		{
			PgPlayer * pPlayer = dynamic_cast<PgPlayer *>(pkUnit);
			if( NULL != pPlayer )
			{
				std::string strTriggerID;
				pkNfy->Pop( strTriggerID, MAX_PATH );
			}
		}break;
	case PT_C_M_REQ_ENTER_CONSTELLATION:
		{
			PgPlayer * pPlayer = dynamic_cast<PgPlayer *>(pkUnit);
			if( NULL != pPlayer )
			{
				int Position, Difficulty;
				bool UseCashItem;
				pkNfy->Pop(Position);
				pkNfy->Pop(Difficulty);
				pkNfy->Pop(UseCashItem);

				Constellation::ConstellationKey Key(Position, Difficulty);
				
				DWORD dwRet = Constellation::ECE_OK;
				
				int NeedItemNo, NeedItemCount;
				dwRet = Constellation_CheckCondition(pPlayer, Key, UseCashItem, NeedItemNo, NeedItemCount); // 조건 체크

				// 여기서 맵이동을 하기 때문에 항상 맨 마지막에 처리 //////////////////////////////
				if( Constellation::ECE_OK == dwRet )
				{// 조건 패스
					Constellation_MoveDungeon(pPlayer, Key, NeedItemNo, NeedItemCount);
				}
				///////////////////////////////////////////////////////////////////////////////////
			}
		}break;
	case PT_C_M_REQ_ENTER_CONSTELLATION_BOSS:
		{
			PgPlayer * pPlayer = dynamic_cast<PgPlayer *>(pkUnit);
			if( NULL != pPlayer )
			{
				Constellation::SConstellationKey constellationKey;
				constellationKey.ReadFromPacket(*pkNfy);
				if( 0 != ( this->GetKind() & GKIND_CONSTELLATION) )
				{
					Constellation_MoveDungeon_Boss(pPlayer, constellationKey);
				}
			}
		}break;
	default:
		{
			LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Invalid CaseType"));
			return false;
//			ProcNotifyMsg(pkUnit, wType, pkNfy);
		}break;
	}

	return true;
}

void PgGround::VUpdate( BM::CSubject< BM::Stream* > *const pChangedSubject, BM::Stream* pkNfy )
{
	BM::CAutoMutex Lock(m_kRscMutex); 
	CUnit* pkUnit = dynamic_cast<CUnit*>(pChangedSubject);

	if(!pkUnit)
	{//컨버팅 안됨.
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("pkUnit is NULL"));
		return;
	}

	pkUnit = PgObjectMgr::GetUnit(pkUnit->GetID());	//이 그라운드에 있는가.

	if(!pkUnit)
	{//컨버팅 안됨.
		INFO_LOG(BM::LOG_LV0, __FL__<<L"There None Unit");
		LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("pkUnit is NULL"));
		return;
	}

	size_t const iRdPos = pkNfy->RdPos();
	BM::Stream::DEF_STREAM_TYPE wType = 0;
	if ( pkNfy->Pop(wType) )
	{
		if ( !this->VUpdate( pkUnit, wType, pkNfy ) )
		{
			this->ProcNotifyMsg(pkUnit, wType, pkNfy);
		}
	}
	pkNfy->RdPos(iRdPos);
}

void PgGround::ChangeArea( CUnit *pkUnit, PgSmallArea *pkNewArea, PgSmallArea *pkLastArea, bool const bSendAreaData )
{
	if ( !pkNewArea )
	{
		return;
	}

	if ( pkLastArea )
	{
		pkLastArea->RemoveUnit( pkUnit->GetID() );
	}

	//이게 위로 올라가면 Area이동이 안됨
	pkNewArea->AddUnit( pkUnit );

	// Unit SmallArea가 이동했다
	// SummonUnit 중에서 인접한 SmallArea에 있지 않은 녀석은 삭제를 해 줘라.
	//int iSummonIndex = 0;
	//BM::GUID kSummonGuid;
	//while ((kSummonGuid = pkUnit->GetSummonUnit(iSummonIndex)) != BM::GUID::NullData())
	//{
	//	CUnit* pkSummonUnit = GetUnit(kSummonGuid);
	//	if ( pkSummonUnit != NULL && ( !pkNewArea->IsAdjacentArea(pkSummonUnit->GetPos()) ) )
	//	{
	//		ReleaseUnit(pkSummonUnit, true);
	//	}
	//	++iSummonIndex;
	//}

	if ( true == bSendAreaData )
	{// 나를 먼저 SendAreaData처리해야 펫에 문제가 안생긴다.
		SendAreaData( pkUnit, pkNewArea, pkLastArea );
	}

	if ( UT_PLAYER == pkUnit->UnitType() )
	{
		PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
		if ( pkPlayer )
		{
			{
				PgPet * pkPet = GetPet( pkPlayer );
				if ( pkPet )
				{
					pkPet->SetPos( pkPlayer->GetPos() );

					PgSmallArea *pkPetLastArea = GetArea( pkPet->LastAreaIndex() );
					ChangeArea( pkPet, pkNewArea, pkPetLastArea, bSendAreaData );
				}
			}
			{
				PgSubPlayer* pkSubPlayer = GetSubPlayer( pkPlayer );
				if( pkSubPlayer )
				{
					pkSubPlayer->SetPos( pkPlayer->GetPos() );

					PgSmallArea *pkSubPlayerLastArea = GetArea( pkSubPlayer->LastAreaIndex() );
					ChangeArea( pkSubPlayer, pkNewArea, pkSubPlayerLastArea, bSendAreaData );
				}
			}
		}
	}
}

bool PgGround::AdjustArea( CUnit *pkUnit, bool const bIsSendAreaData, bool const bIsCheckPos )// Area 에 유닛 넣어주는것 까지 다함.
{
	BM::CAutoMutex Lock(m_kRscMutex);

	bool bReturn = true;
	POINT3 const ptNow = pkUnit->GetPos();

	PgSmallArea *pkTgtArea = NULL;
	PgSmallArea *pkLastArea = GetArea( pkUnit->LastAreaIndex() );
	if ( pkLastArea )
	{
		if ( pkLastArea->IsBigArea() )
		{
			pkTgtArea = pkLastArea;
		}
	}
	else
	{
		// LastArea가 없으면 AddUnit
		if (	(UT_PLAYER != pkUnit->UnitType())
			&&	(0 < pkUnit->GetAbil( AT_USENOT_SMALLAREA )) )
		{
			// PlayerUnit은 BigArea를 사용하면 안된다.

			// BigArea Unit은 HP/MP는 브로드캐스트 해야함
			pkUnit->SetAbil( AT_HPMP_BROADCAST, 1, false, false );

			pkTgtArea = GetArea( PgSmallArea::BIG_AREA_INDEX );
		}
	}

	if ( NULL == pkTgtArea )
	{
		POINT3 ptOutPos1;
		HRESULT const hFirstRet = GetArea( ptNow, ptOutPos1, pkTgtArea, true, bIsCheckPos );
		switch ( hFirstRet )
		{
		case S_OK:
			{// 이동영역이 정상적이다.
			}break;
		case GA_FORCE_MOVE_TO_SPAWN:
			{// 스폰으로 이동
				switch ( pkUnit->UnitType() )
				{
				case UT_MONSTER:
				case UT_BOSSMONSTER:
					{
						PgMonster* pkMonster = dynamic_cast<PgMonster*>(pkUnit);
						if ( pkMonster )
						{
							INFO_LOG(	BM::LOG_LV0, __FL__ << _T("UT_MONSTER Kill_BY_FORCE GroundNo<") << GetGroundNo() << _T("> Monster<") << pkUnit->Name() << _T("/CLASS:") << pkUnit->GetAbil(AT_CLASS) << _T("> GenPos : ")
								<<	pkMonster->GenInfo().pt3Pos.x << _T(", ") << pkMonster->GenInfo().pt3Pos.y	<< _T(", ") << pkMonster->GenInfo().pt3Pos.z
								<<	_T("NowPos=") << ptNow.x << _T(", ") << ptNow.y << _T(", ") << ptNow.z 
								);
						}
						
						pkUnit->SetAbil(AT_HP, HP_VALUE_KILL_NOTADDED_UNIT);				// 클라이언트에 패킷을 보내줘야한다.

						LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("bReturn is false"));
						bReturn = false;
					}break;
				default:
					{
						pkUnit->SetPos(ptOutPos1);
						SendUnitPosChange( pkUnit, MMET_Normal, E_SENDTYPE_SELF );
					}break;
				}
			}break;
		case E_FAIL:
		default:
			{
				VERIFY_INFO_LOG( false, BM::LOG_LV0, __FL__ << _T("Can't AdjustArea Ret<") << hFirstRet << L"> GroundNo<" << GetGroundNo() << L"> " );
				LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("bReturn is false"));
				return false;
			}break;
		}
	}

#ifdef ZONE_OPTIMIZE_20090624
	if ( pkTgtArea != NULL && (pkTgtArea != pkLastArea) && bReturn )
#else
	if ( (pkTgtArea != pkLastArea) && bReturn )
#endif
	{// 영역이 다르네
		ChangeArea( pkUnit, pkTgtArea, pkLastArea, bIsSendAreaData );
	}

	return bReturn;
}

void PgGround::RecvChat( PgPlayer *pkPlayer, BM::Stream *pkPacket )
{
	if ( !pkPlayer )
	{
		LIVE_CHECK_LOG( BM::LOG_LV0, __FL__ << L"Player is NULL" );
		return;
	}

	BYTE byChatType = 0;
	std::wstring wstrChat;
	DWORD dwFontColor = 0;

	pkPacket->Pop(byChatType);//1
	pkPacket->Pop(wstrChat);//2
	pkPacket->Pop(dwFontColor);//3

	// 채팅 금지인지 먼저 검사
	switch( byChatType )
	{
	case CT_NORMAL:
	case CT_TEAM:
	case CT_TRADE:
		{
			if ( pkPlayer->GetAbil(AT_MUTE) )
			{
				pkPlayer->SendWarnMessage(700405);
				return;// 채팅 금지
			}

			if ( BM::GUID::IsNotNull( GroundMute() ) && ( 0 == pkPlayer->GMLevel() ) )
			{
				pkPlayer->SendWarnMessage(700405);
				return;// 채팅 금지
			}
		}break;
	default:
		{
		}break;
	}

	// 필터 검사
	std::set<int> kCont;
	if ( true == Emoticon::FilterEmoticon( pkPlayer, wstrChat, kCont) )
	{
		if(false == kCont.empty())
		{
			PgAddAchievementValue kMA(AT_ACHIEVEMENT_USE_EMOTICON,1,GroundKey());
			kMA.DoAction(pkPlayer,NULL);
		}

		if ( true == g_kUnicodeFilter.IsCorrect( UFFC_CHAT, wstrChat ) )
		{
			XUI::PgExtraDataPackInfo kSendExtraDataPackInfo;
			GetExtraDataPackInfo( pkPlayer->GetInven(), pkPacket, kSendExtraDataPackInfo );//4

			int iEmoticonNo = 0;
			if( S_OK == g_kVariableContainer.Get(EVar_Kind_GM_Emoticon, EVar_GM_Emoticon_Default, iEmoticonNo) )
			{
				Emoticon::AddGMEmoticon( pkPlayer, wstrChat, iEmoticonNo);
			}

			if( S_OK == g_kVariableContainer.Get(EVar_Kind_Premium_Service, EVar_VIP_Emoticon, iEmoticonNo) )
			{
				Emoticon::AddVIPEmoticon(pkPlayer, wstrChat, iEmoticonNo);
			}

			switch( byChatType )
			{
			case CT_NORMAL:
			case CT_EMOTION:
				{
					BM::Stream kPacket( PT_M_C_NFY_CHAT, byChatType );
					kPacket.Push(pkPlayer->GetID());
					kPacket.Push(pkPlayer->Name());
					//kPacket.Push(std::chrono::system_clock::now());
					kPacket.Push(wstrChat);
					kPacket.Push(dwFontColor);
					kSendExtraDataPackInfo.PushToPacket(kPacket);
					kPacket.Push(*pkPacket);

					//if (GetMapAbil(AT_CHAT_BROADCAST_TYPE) == 0)
					//{
					//	SendToArea( &kPacket, pkPlayer->LastAreaIndex(), BM::GUID::NullData(), SYNC_TYPE_NONE, E_SENDTYPE_NONE );
					//}
					//else
					//{
						Broadcast(kPacket);
					//}

					PgChatLogUtil::Log( ELogSub_Chat_Nomal, pkPlayer, wstrChat, GetGroundNo());
				}break;
			case CT_TEAM:
				{
					BM::Stream kPacket( PT_M_C_NFY_CHAT, byChatType );
					kPacket.Push(pkPlayer->GetID());
					kPacket.Push(pkPlayer->Name());
					//kPacket.Push(std::chrono::system_clock::now());
					kPacket.Push(wstrChat);
					kPacket.Push(dwFontColor);
					kSendExtraDataPackInfo.PushToPacket(kPacket);
					kPacket.Push(*pkPacket);

					int const iTeam = pkPlayer->GetAbil( AT_TEAM );

					CUnit* pkTarget = NULL;
					CONT_OBJECT_MGR_UNIT::iterator kItor;
					PgObjectMgr::GetFirstUnit( UT_PLAYER, kItor );
					while ((pkTarget = PgObjectMgr::GetNextUnit( UT_PLAYER, kItor)) != NULL)
					{
						if ( iTeam == pkTarget->GetAbil(AT_TEAM) )
						{
							pkTarget->Send( kPacket, E_SENDTYPE_SELF );
						}
					}

					PgChatLogUtil::Log( ELogSub_Chat_TeamChat, pkPlayer, wstrChat, GetGroundNo());
				}break;
			case CT_PARTY:
				{
					BM::Stream kPacket( PT_C_M_REQ_CHAT, pkPlayer->GetID());
					kPacket.Push(byChatType);
					kPacket.Push(wstrChat);
					kPacket.Push(dwFontColor);
					kSendExtraDataPackInfo.PushToPacket(kPacket);
					kPacket.Push(*pkPacket);
					::SendToChannelChatMgr(kPacket);
				}break;
			case CT_MYHOME_PUBLIC:
			case CT_MYHOME_PRIVATE:
				{
					BM::Stream kPacket( PT_C_M_REQ_CHAT, pkPlayer->GetID());
					kPacket.Push(byChatType);
					kPacket.Push(wstrChat);
					kPacket.Push(dwFontColor);
					kPacket.Push(*pkPacket);
					kSendExtraDataPackInfo.PushToPacket(kPacket);
					::SendToMyhomeMgr(kPacket);
				}break;
			case CT_WHISPER_BYNAME:
				{
					std::wstring kName;
					pkPacket->Pop(kName);
					if( 0 == kName.compare(pkPlayer->Name()) )
					{
						pkPlayer->SendWarnMessage(799340);
					}
					else
					{
						BM::Stream kPacket( PT_C_M_REQ_CHAT, pkPlayer->GetID());
						kPacket.Push(byChatType);
						kPacket.Push(wstrChat);
						kPacket.Push(dwFontColor);
						kPacket.Push(kName);
						kPacket.Push(*pkPacket);
						kSendExtraDataPackInfo.PushToPacket(kPacket);
						::SendToRealmChatMgr(kPacket);
					}
				}break;
			case CT_WHISPER_BYGUID:
				{
					BM::GUID kGuid;
					pkPacket->Pop(kGuid);
					if( kGuid == pkPlayer->GetID() )
					{
						pkPlayer->SendWarnMessage(799340);
					}
					else
					{
						BM::Stream kPacket( PT_C_M_REQ_CHAT, pkPlayer->GetID());
						kPacket.Push(byChatType);
						kPacket.Push(wstrChat);
						kPacket.Push(dwFontColor);
						kPacket.Push(kGuid);
						kPacket.Push(*pkPacket);
						kSendExtraDataPackInfo.PushToPacket(kPacket);
						::SendToRealmChatMgr(kPacket);
					}
				}break;
			default:
				{
					BM::Stream kPacket( PT_C_M_REQ_CHAT, pkPlayer->GetID());
					kPacket.Push(byChatType);
					kPacket.Push(wstrChat);
					kPacket.Push(dwFontColor);
					kPacket.Push(*pkPacket);
					kSendExtraDataPackInfo.PushToPacket(kPacket);
					::SendToRealmChatMgr(kPacket);
				}break;
			}
		}
		else
		{
			pkPlayer->SendWarnMessage(700407);
		}
	}
	else
	{
		pkPlayer->SendWarnMessage(700408);
	}
}

void PgGround::RecvChatInputState(CUnit *pkUnit, BM::Stream *pkPacket)
{
	BM::CAutoMutex Lock(m_kRscMutex);
	BYTE cChatMode =0;
	bool bPopup = false;
	pkPacket->Pop(cChatMode);
	pkPacket->Pop(bPopup);

	switch(cChatMode)
	{
	case CT_NORMAL:
		{//존 한정 전송
			BM::Stream kPacket(PT_M_C_NFY_CHAT_INPUTNOW, pkUnit->GetID());
			kPacket.Push(cChatMode);
			kPacket.Push(bPopup);
			SendToArea(&kPacket, pkUnit->LastAreaIndex(), BM::GUID::NullData(), SYNC_TYPE_RECV_ADD, E_SENDTYPE_NONE );//rkReqMove.guidPilot
		}break;
	case CT_PARTY:
		{//파티 한정 전송
			BM::Stream kPacket(PT_C_M_REQ_CHAT_INPUTNOW, pkUnit->GetID());
			kPacket.Push(cChatMode);
			kPacket.Push(bPopup);
			SendToGlobalPartyMgr(kPacket);//아직은 컨텐츠로(파티관리를 컨텐츠가 아직 한다.)
		}break;
	case CT_GUILD:
		{
			BM::Stream Packet(PT_C_M_REQ_CHAT_INPUTNOW, pkUnit->GetID());
			Packet.Push(cChatMode);
			Packet.Push(bPopup);
			SendToRealmChatMgr(Packet);
		}break;
	default:
		{
			LIVE_CHECK_LOG(BM::LOG_LV1, __FL__ << _T("Invalid CaseType"));
		};
	}
}

void PgGround::RecvUnitDie(CUnit *pkUnit)
{
	BM::CAutoMutex Lock(m_kRscMutex);
	if( !pkUnit )
	{
		return;
	}

	BM::GUID const kCasterGuid = pkUnit->GetTarget();
	CUnit * pkCaster = GetUnit(kCasterGuid);
	BM::GUID kKillerGuid(kCasterGuid);
	if(pkCaster)
	{
		switch(pkCaster->UnitType())
		{
		case UT_SUMMONED:
			{
				pkCaster = GetUnit(pkCaster->Caller());
			}break;
		default:
			{
				kKillerGuid = GlobalHelper::FindOwnerGuid(pkCaster);
			}break;
		}
	}

	if (	pkCaster
		&&	UT_PLAYER == pkCaster->UnitType() )
	{
		PgPlayer * pkCasterPlayer = dynamic_cast<PgPlayer*>(pkCaster);
		if ( pkCasterPlayer )
		{
			if( g_kMonKillCountReward.MonsterKillSystem() ) // 몬스터 킬 시스템을 사용할 경우
			{
				if( pkUnit->IsInUnitType(UT_MONSTER) && 0==GetMapAbil(AT_NO_KILLCOUNT_MAP) ) // 몬스터 킬 시스템은 몬스터만 카운팅 된다.
				{
					// 죽을 경우 캐스터의 몬스터 킬 카운트를 증가 시켜준다.
					if( BM::GUID::IsNotNull(kKillerGuid)
						&&	0 == (GATTR_FLAG_NO_KILLCOUNT_UI & GetAttr()) )
					{
						SAggroOwner kAggroOwner(kKillerGuid, pkCasterPlayer->CreatePetID());
						if(pkUnit->AggroMeter()->IsDiffCaller(kAggroOwner))
						{
							BM::Stream kNfyGndPacket(PT_U_G_NFY_ADD_KILLCOUNTER);
							kNfyGndPacket.Push( pkCasterPlayer->GetID() );
							pkUnit->VNotify(&kNfyGndPacket); // Monster에 대고 Notify 한다.
						}
					}
				}
			}
			ALRAM_MISSION::PgClassAlramMissionEvent<ALRAM_MISSION::EVENT_KILL_ANYBODY>()( pkCasterPlayer, 1, this, pkUnit );
		}
	}

	switch( pkUnit->UnitType() )
	{
	case UT_PLAYER:
		{	
			pkUnit->SendAbil(AT_HP, E_SENDTYPE_BROADCAST);//HP 0 이라고 알려줘

//			PgPlayer *pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
//			if ( pkPlayer )
//			{
//				PgComboCounter* pkCounter = NULL;
//				if( GetComboCounter( pkPlayer->GetID(), pkCounter) )
//				{
//					pkPlayer->Update_RankingPoint( E_RANKPOINT_MAXCOMBO, pkCounter->GetComboCount(ECOMBO_MAX) );
//				}
//			}

			if( 0 == (GATTR_FLAG_NO_INIT_KILLCOUNT & GetAttr()) ) // 그라운드 체크
			{
				pkUnit->SetAbil(AT_MONSTER_KILL_COUNT, 0, true);
			}

			ALRAM_MISSION::PgClassAlramMissionEvent<ALRAM_MISSION::EVENT_DIE>()( dynamic_cast<PgPlayer*>(pkUnit), 1, this, (pkCaster ? pkCaster : pkUnit) );
		}break;
	case UT_MONSTER:
	case UT_BOSSMONSTER:
		{
		}break;
	}
}

void PgGround::OnReqItemAction( PgPlayer *pkPlayer, BM::Stream * pkNfy, PgPet * pkPet )
{// Pet이 있으면 PetItemAction이다.
	if ( pkPlayer )
	{
		if( PgGroundUtil::SUST_HardCoreDungeonVote == GetSpecStatus(pkPlayer->GetID() ) )
		{
			pkPlayer->SendWarnMessage( 402203 );// 하드코어 던젼 투표중에는 아이템 사용 금지
			return;
		}

		SItemPos kCasterPos;
		DWORD dwClientTime;
		pkNfy->Pop(kCasterPos);
		pkNfy->Pop(dwClientTime);

		DWORD dwServerTime = 0;
		if ( E_FAIL == CheckClientNotifyTime( pkPlayer, dwClientTime, dwServerTime ) )
		{
			INFO_LOG(BM::LOG_LV5, __FL__ << _T("Player kicked, Cause[CDC_SuspectedSpeedHack], Name=") << pkPlayer->Name());
			// SpeedHack이 의심스러우니 강제접속 해제 시킨다.
			BM::Stream kDPacket( PT_A_S_NFY_USER_DISCONNECT, static_cast<BYTE>(CDC_SuspectedSpeedHack) );
			kDPacket.Push( pkPlayer->GetMemberGUID() );
			SendToServer( pkPlayer->GetSwitchServer(), kDPacket );
			return;
		}

		bool const bIsPet = ( NULL != pkPet );

		// PgPet이 있으면 Pet의 인벤에서 검사해야 한다.
		PgInventory *pkInven = ( bIsPet ? pkPet->GetInven() : pkPlayer->GetInven() ); 
		CUnit *pkItemOwnerUnit = ( bIsPet ? dynamic_cast<CUnit*>(pkPet) : dynamic_cast<CUnit*>(pkPlayer) );

		/*
		아이템 사용 맵이동 처리를 맵이동후 아이템 사용으로 동작 변경 하도록 수정
		기존 아이템 사용 맵 이동으로 들어가지 않도록 한다.
		*/

		PgAction_ReqUseItem kAction( kCasterPos, this, false, *pkNfy );
		kAction.SetCoolTime(dwClientTime);

		PgBase_Item kItem;
		if( S_OK == pkInven->GetItem( kCasterPos,kItem ) )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( pItemDef )
			{
				int const iMapNo = pItemDef->GetAbil(AT_MAP_NUM);
				int const iUseType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);

				if( (0 == iMapNo) && (UICT_USER_PORTAL != iUseType) && (UICT_TELEPORT != iUseType) && (UICT_SUPER_GROUND_ENTER_FLOOR != iUseType) )
				{
					kAction.DoAction( pkItemOwnerUnit, pkPlayer );
					return;
				}

				/*
				여기서 부터는 맵 이동 아이템 사용
				*/

				if ( !bIsPet )
				{
					if( true == kAction.DoAction( pkItemOwnerUnit, NULL) )
					{
						SReqMapMove_MT kRMM;
						kRMM.kCasterKey = GroundKey();
						kRMM.kCasterSI = g_kProcessCfg.ServerIdentity();

						switch(iUseType)
						{
						case UICT_TELEPORT:
							{
								POINT3 kTargetPos;
								pkNfy->Pop(kTargetPos);

								NxRay kRay(NxVec3(kTargetPos.x, kTargetPos.y, kTargetPos.z+20), NxVec3(0, 0, -1.0f));
								NxRaycastHit kHit;
								NxShape *pkHitShape = RayCast(kRay, kHit);

								if(NULL == pkHitShape)
								{
									return;
								}

								kRMM.kTargetKey = GroundKey();
								kRMM.nTargetPortal = 0;
								kRMM.pt3TargetPos = kTargetPos;
								kRMM.pt3TargetPos.z = kHit.worldImpact.z;
							}break;
						case UICT_USER_PORTAL:
							{
								BM::GUID kGuid;
								pkNfy->Pop(kGuid);
								CONT_USER_PORTAL::const_iterator iter = pkPlayer->ContPortal().find(kGuid);
								if( iter == pkPlayer->ContPortal().end() )
								{
									return;
								}

								int const iTargetGroundNo = (*iter).second.iGroundNo;

								CONT_DEFMAP const* pkDefMap = NULL;
								g_kTblDataMgr.GetContDef(pkDefMap);

								if( !pkDefMap )
								{
									return;
								}

								CONT_DEFMAP::const_iterator find_iter = pkDefMap->find(iTargetGroundNo);
								if( pkDefMap->end() == find_iter )
								{
									return;
								}

								if( 0 != ((*find_iter).second.iAttr & GATTR_FLAG_CANT_WARP) )
								{
									return;
								}

								if( true == pkPlayer->IsItemEffect(SAFE_FOAM_EFFECTNO) )
								{
									pkPlayer->SendWarnMessage(10410);
									return;
								}

								kRMM.kTargetKey.GroundNo((*iter).second.iGroundNo);
								kRMM.nTargetPortal = 0;
								kRMM.pt3TargetPos = (*iter).second.kPos;
							}break;
						case UICT_SUPER_GROUND_ENTER_FLOOR:
							{
								PgAction_MissionPartyMemberCheck kPartyMemberCheck( this );
								if( false == kPartyMemberCheck.DoAction( pkPlayer, NULL ) )
								{
									pkPlayer->SendWarnMessage(400906);
									return;
								}
							}break;
						default:
							{
								kRMM.kTargetKey.GroundNo(iMapNo);
								kRMM.nTargetPortal = static_cast<short>(pItemDef->GetAbil(AT_PORTAL_NUM));
							}break;
						}

						// 처리
						switch(iUseType)
						{
						case UICT_SUPER_GROUND_ENTER_FLOOR:
							{
								int const iSuperGroundNo = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
								int const iModeNo = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
								int const iFloorNo = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_3);
								int const iSpawnNo = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_4);

								SPMO const kModifyItem(IMET_MODIFY_COUNT | IMC_DEC_DUR_BY_USE, pkPlayer->GetID(), SPMOD_Modify_Count(kItem,kCasterPos,-1));

								SuperGroundUtil::ReqEnterSuperGround(pkPlayer, GroundKey(), iSuperGroundNo, iModeNo, iSpawnNo, iFloorNo, &kModifyItem);
							}break;
						default:
							{
								if( kRMM.kCasterKey.GroundNo() == kRMM.kTargetKey.GroundNo() )
								{
									kRMM.kTargetKey.Guid(kRMM.kCasterKey.Guid());
								}

								PgReqMapMove kMapMove(this, kRMM, NULL );
								if( true == kMapMove.Add( pkPlayer ))
								{
									kMapMove.AddModifyOrder(SPMO(IMET_MODIFY_COUNT | IMC_DEC_DUR_BY_USE,pkPlayer->GetID(),SPMOD_Modify_Count(kItem,kCasterPos,-1)));
									kMapMove.DoAction();
								}
							}break;
						}
					}
				}
			}
		}
	}
}

void PgGround::SetEventScript_LockPartyMember(CUnit const *pkUnit, int const iEventScriptID)
{// 모두 스크립트를 보지 않았기 때문에 인풋 막도록 보냄.
	if( !pkUnit )
	{
		return;
	}
	SetEventScript_PartyLock(pkUnit, iEventScriptID, PT_M_C_REQ_LOCK_INPUT_EVENT_SCRIPT);
}
void PgGround::SetEventScript_UnLockPartyMember(CUnit const *pkUnit)
{// 모두 스크립트를 보지 않았기 때문에 인풋 막도록 보냄.
	if( !pkUnit )
	{
		return;
	}
	SetEventScript_PartyLock(pkUnit, 0, PT_M_C_REQ_UNLOCK_INPUT_EVENT_SCRIPT);
}

void PgGround::Set_LockPlayerInGround()
{
	CUnit * pUnit = NULL;
	CONT_OBJECT_MGR_UNIT::iterator iter;
	PgObjectMgr::GetFirstUnit(UT_PLAYER, iter);
	while((pUnit = PgObjectMgr::GetNextUnit(UT_PLAYER, iter)) != NULL)
	{
		pUnit->SetAbil(AT_EVENT_SCRIPT, INT_MAX); // 초기화
		pUnit->SetAbil(AT_EVENT_SCRIPT_TIME, 0); // 초기화
		BM::Stream LockPacket(PT_M_C_REQ_LOCK_INPUT_EVENT_SCRIPT);
		pUnit->Send(LockPacket, E_SENDTYPE_SELF);
	}
}

void PgGround::SetEventScript_PartyLock( CUnit const *pkUnit, int const iEventScriptID, BM::Stream::DEF_STREAM_TYPE const kPacketType )
{
	if( !pkUnit )
	{
		return;
	}
	BM::GUID const kPartyGuid = pkUnit->GetPartyGuid();
	if( kPartyGuid.IsNull() )
	{
		return;
	}
	VEC_GUID kPartyMember;
	GetPartyMember( kPartyGuid, kPartyMember );
	VEC_GUID::const_iterator itor_party = kPartyMember.begin();
	while( kPartyMember.end() != itor_party )
	{
		CUnit *pkPartyUnit = GetUnit( (*itor_party) );
		if( (pkPartyUnit) && (pkPartyUnit != pkUnit) )
		{
			int const iPreEventID = pkPartyUnit->GetAbil(AT_EVENT_SCRIPT);
			if( ( 0 != iEventScriptID && 0 == iPreEventID )
				|| ( 0 == iEventScriptID ) )
			{
				pkPartyUnit->SetAbil(AT_EVENT_SCRIPT, iEventScriptID); // 초기화
				pkPartyUnit->SetAbil(AT_EVENT_SCRIPT_TIME, 0); // 초기화
				BM::Stream LockPacket(kPacketType);
				pkPartyUnit->Send(LockPacket, E_SENDTYPE_SELF);
			}
		}
		++itor_party;
	}
}