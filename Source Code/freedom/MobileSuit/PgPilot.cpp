#include "StdAfx.h"
#include "Variant/Inventory.h"
#include "Variant/PgPlayer.h"
#include "Variant/PgMonster.h"
#include "Variant/PgEntity.h"
#include "Variant/PgCustomUnit.h"
#include "Variant/PgControlDefMgr.h"
//#include "Variant/MonsterDefMgr.h"
//#include "Variant/PgClassDefMgr.h"
#include "Variant/PgTotalObjectMgr.h"
#include "PgXmlLoader.h"
#include "PgActor.h"
#include "PgActorMonster.h"
#include "PgAction.h"
#include "PgActionPool.h"
#include "PgInput.h"
#include "ServerLib.h"
#include "PgSkillTree.H"
#include "PgWorld.h"
#include "PgMobileSuit.h"
#include "PgStat.h"
#include "PgPilotMan.H"
#include "PgMissionComplete.h"
#include "Variant/Constant.H"
#include "Variant/PgStringUtil.h"

#include "lwPilot.H"//이걸 추가해도 되나?
#include "lwUI.H"//이걸 추가해도 되나?
#include "PgClientParty.h"
#include "PgstatusEffect.h"
#include "PgPilot.h"
#include "PgSkillOptionMan.H"
#include "PgSkillOptionCommandKey.H"

#include "PgOption.h"
#include "PgSkillTargetMan.h"
#include "lwCommonSkillUtilFunc.h"
#include "PgCommunityEvent.h"

int const SINGLE_UKEY_RANGE = ENIKEY_TO_UKEY+NiInputKeyboard::KEY_TOTAL_COUNT;	// 단일 입력 U 키
int const iMaxClassHierarchy  = 15;
int const iMaxClassLevel = 4;

static int iAllClassID[iMaxClassHierarchy][iMaxClassLevel]={
	{UCLASS_FIGHTER,UCLASS_KNIGHT,UCLASS_PALADIN,UCLASS_DRAOON},
	{UCLASS_FIGHTER,UCLASS_WARRIOR,UCLASS_GLADIATOR,UCLASS_DESTROYER},
	{UCLASS_FIGHTER,UCLASS_DUELIST,UCLASS_BATTLEMASTER,0},

	{UCLASS_MAGICIAN,UCLASS_MAGE,UCLASS_WIZARD,UCLASS_ARCHMAGE},
	{UCLASS_MAGICIAN,UCLASS_BATTLEMAGE,UCLASS_WARMAGE,UCLASS_WARLORD},
	{UCLASS_MAGICIAN,UCLASS_DOCTOR,UCLASS_ANATOMIST,0},

	{UCLASS_ARCHER,UCLASS_HUNTER,UCLASS_TRAPPER,UCLASS_SENTINEL},
	{UCLASS_ARCHER,UCLASS_RANGER,UCLASS_SNIPER,UCLASS_LAUNCHER},
	{UCLASS_ARCHER,UCLASS_GUNSLINGER,UCLASS_DESPERADO,0},

	{UCLASS_THIEF,UCLASS_CLOWN,UCLASS_DANCER,UCLASS_MANIAC},
	{UCLASS_THIEF,UCLASS_ASSASSIN,UCLASS_NINJA,UCLASS_SHADOW},
	{UCLASS_THIEF,UCLASS_BARD,UCLASS_VOCAL,0},

	{UCLASS_SHAMAN,UCLASS_SUMMONER,UCLASS_BEAST_MASTER,UCLASS_GAIA_MASTER},
	{UCLASS_DOUBLE_FIGHTER,UCLASS_TWINS,UCLASS_MIRAGE,UCLASS_DRAGON_FIGHTER},
	{10000, 10000,10000,10000}	// 쌍둥이 캐릭터 ClassNo가 10000이기 때문 매직넘버 고칠것
};

BM::GUID PgPilot::ms_kPlayerPilotGuid;

PgPilot::PgPilot() :
	m_pkWorldObject(0),
	m_bFrozen(0),
	m_pkUnit(0),
	m_bRiding(false)
{
	m_ulLastRemoteActionTime = 0;
	m_dwLastHPSetTimeStamp = 0;
}

PgPilot::~PgPilot(void)
{
	m_pkWorldObject = 0;
	m_kInputSlotContainer.clear();
	m_kContComboActionData.clear();
	m_kContConnectComboActionData.clear();
	m_kContInputKey.clear();
	if(NULL != m_pkUnit)
	{
#ifndef EXTERNAL_RELEASE
		switch(m_pkUnit->UnitType())
		{
		case UT_PLAYER:
		case UT_SUB_PLAYER:
		case UT_ENTITY:
		case UT_BOSSMONSTER:
		case UT_MONSTER:
		case UT_SUMMONED:
		case UT_PET:
			{
				g_kSkillTargetMan.DeleteUnit(m_pkUnit->GetID());
			}break;
		}
#endif
		NILOG(PGLOG_LOG, "[PgPilot] %s pilot(%#X) is destroying\n", MB(m_pkUnit->GetID().str()), m_pkUnit);
		g_kTotalObjMgr.UnRegistUnit(m_pkUnit);
		g_kTotalObjMgr.ReleaseUnit(m_pkUnit);
	}

	for(RemoteActionContainer::iterator itor = m_kRemoteActionList.begin(); itor != m_kRemoteActionList.end(); ++itor)
	{
		SAFE_DELETE(*itor);
	}
	m_kRemoteActionList.clear();
}
PgPilot* PgPilot::CreateCopy()
{
	PgPilot* pkNewPilot = new PgPilot();

	pkNewPilot->m_kInputSlotContainer = m_kInputSlotContainer;
	pkNewPilot->m_kRidingInputSlotCont = m_kRidingInputSlotCont;
	pkNewPilot->m_kContComboActionData = m_kContComboActionData;
	pkNewPilot->m_kContConnectComboActionData = m_kContConnectComboActionData;

	return pkNewPilot;
}
void PgPilot::SetFreeze(bool bFreeze)
{
	m_bFrozen = bFreeze;
}

bool PgPilot::IsFrozen()
{
	return m_bFrozen;
}


void PgPilot::SetRidingPet(bool bRideOn)
{
	m_bRiding = bRideOn;
}

bool PgPilot::IsRidingPet(void)
{
	return m_bRiding;
}

bool PgPilot::IsShowWarning()const
{
	PgActor* pkActor = dynamic_cast<PgActor*>(m_pkWorldObject);
	if(pkActor)
	{
		return pkActor->IsShowWarning();
	}
	return false;
}

bool PgPilot::IsHide()const
{
	return GetAbil(AT_HIDE) || GetAbil(AT_EVENTSCRIPT_HIDE);
}

//	이 캐릭터가 iReqClassID 에 해당하는 캐릭터가 맞는지 체크한다.
//	만약 내가 전사라면, iReqClassID는 전사일 경우에만 true
//	만약 내가 기사라면, iReqClassID는 전사,기사일 경우에만 true
//  만약 내가 투사라면, iReqClassID는 전사,투사일 경우에만 true
//	만약 내가 성기사라면, iReqClassID는 전사,기사,성기사일 경우에만 true
bool PgPilot::IsCorrectClass(int const iReqClassID,bool bNotIncludeSelf)	
{
	int	const iClassID = GetAbil(AT_CLASS);

	if(iClassID == iReqClassID) 
	{
		if(false == bNotIncludeSelf)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	if(iReqClassID > iClassID)
	{
		return	false;
	}
	
	for(int i = 0; i < iMaxClassHierarchy; ++i)
	{
		
		bool bReqClassFound = false;
		bool bMyClassFound = false;

		for(int j = 0; j < iMaxClassLevel; ++j)
		{	
			if(iReqClassID == iAllClassID[i][j])
			{
				bReqClassFound = true;
			}
			if(iClassID == iAllClassID[i][j])
			{
				bMyClassFound = true;
			}
		}

		if(bReqClassFound && bMyClassFound)	//	동일한 계층에서 두개가 모두 존재한다면 OK!
		{
			return	true;
		}
	}

	return	false;

}
int	PgPilot::GetBaseClassID(int iReqClassID)
{
	int	const iClassID = (iReqClassID == -1) ? GetAbil(AT_CLASS) : iReqClassID;
	if(m_pkUnit && m_pkUnit->IsUnitType(UT_SUMMONED))
	{
		return iClassID;
	}

	for(int i = 0; i < iMaxClassHierarchy; ++i)
	{
		for(int j = 0; j < iMaxClassLevel; ++j)
		{	
			if(iClassID == iAllClassID[i][j])
			{
				if(10000 == iAllClassID[i][0])
				{// 애니메이션 번호 바꾸게 되면 이부분도 수정되어야함. 현재는 쌍둥이 애니메이션 진행을 위한 임시 
					return 52;
				}
				return	iAllClassID[i][0];
			}
		}
	}
	return	0;
}

PgInputSlotInfo* PgPilot::FindAction(unsigned int uiUKey)
{
	/*

	InputSlotContainer::iterator itr = std::find(m_kInputSlotContainer.begin(), m_kInputSlotContainer.end(), PgInputSlotInfo("", uiUKey));
	if(itr == m_kInputSlotContainer.end())
	{
		return 0;
	}
	return &(*itr);
	*/
	
	if( SINGLE_UKEY_RANGE > uiUKey						// 단일 입력 U키이면서
		&& 0 == g_kGlobalOption.GetUKeyToKey(uiUKey)	// 옵션에 설정되어 있지 않은 키는
		)
	{// 액션을 해선 안되고
		return NULL;
	}
	int iWeaponType = 0;
	if( g_kPilotMan.IsMyPlayer( this->GetGuid() ) )
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(pkPlayer) 
		{
			PgInventory* pkInv = pkPlayer->GetInven();
			if(pkInv) 
			{
				PgBase_Item kItem;
				if( S_OK == pkInv->GetItem(IT_FIT, EQUIP_POS_WEAPON, kItem) ) 
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef* pkDef = (CItemDef*) kItemDefMgr.GetDef(kItem.ItemNo());
					if(pkDef )
					{
						iWeaponType = pkDef->GetAbil(AT_WEAPON_TYPE);
					}
					
				}
			}
		}
	}
	if(m_bRiding)
	{
		InputSlotContainer::iterator itr = std::find(m_kRidingInputSlotCont.begin(), m_kRidingInputSlotCont.end(), PgInputSlotInfo("", uiUKey, 0, false, true, 0, 0));
		if(itr == m_kRidingInputSlotCont.end())
		{
			return 0;
		}
		return &(*itr);
	}
	else
	{
		InputSlotContainer::iterator itr = std::find(m_kInputSlotContainer.begin(), m_kInputSlotContainer.end(), PgInputSlotInfo("", uiUKey, 0, false, true, 0, iWeaponType));
		if(itr == m_kInputSlotContainer.end())
		{
			itr = std::find(m_kInputSlotContainer.begin(), m_kInputSlotContainer.end(), PgInputSlotInfo("", uiUKey, 0, false, true, 0, 0));
			if(itr == m_kInputSlotContainer.end())
			{//장착무기 타입이랑 관계없는 애니 있으면 그걸로 셋팅하자
				return 0;
			}
		}
		return &(*itr);
	}

	return 0;
}

char const* PgPilot::FindActionID(unsigned int uiUKey)const
{
/*	InputSlotContainer::iterator itr = std::find(m_kInputSlotContainer.begin(), m_kInputSlotContainer.end(), PgInputSlotInfo("", uiUKey));
	if(itr == m_kInputSlotContainer.end())
	{
		return 0;
	}

	return itr->GetActionID().c_str();*/
	int iWeaponType = 0;
	if( g_kPilotMan.IsMyPlayer( this->GetGuid() ) )
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(pkPlayer) 
		{
			PgInventory* pkInv = pkPlayer->GetInven();
			if(pkInv) 
			{
				PgBase_Item kItem;
				if( S_OK == pkInv->GetItem(IT_FIT, EQUIP_POS_WEAPON, kItem) ) 
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef* pkDef = (CItemDef*) kItemDefMgr.GetDef(kItem.ItemNo());
					if(pkDef )
					{
						iWeaponType = pkDef->GetAbil(AT_WEAPON_TYPE);
					}
					
				}
			}
		}
	}
	if(m_bRiding)
	{
		InputSlotContainer::const_iterator itr = std::find(m_kRidingInputSlotCont.begin(), m_kRidingInputSlotCont.end(), PgInputSlotInfo("", uiUKey, 0, false, true, 0, iWeaponType));
		if(itr == m_kRidingInputSlotCont.end())
		{
			itr = std::find(m_kRidingInputSlotCont.begin(), m_kRidingInputSlotCont.end(), PgInputSlotInfo("", uiUKey, 0, false, true, 0, 0));
			if(itr == m_kRidingInputSlotCont.end())
			{//장착무기 타입이랑 관계없는 애니 있으면 그걸로 셋팅하자
				return 0;
			}
		}
		return itr->GetActionID().c_str();
	}
	else
	{
		InputSlotContainer::const_iterator itr = std::find(m_kInputSlotContainer.begin(), m_kInputSlotContainer.end(), PgInputSlotInfo("", uiUKey, 0, false, true, 0, iWeaponType));
		if(itr == m_kInputSlotContainer.end())
		{
			itr = std::find(m_kInputSlotContainer.begin(), m_kInputSlotContainer.end(), PgInputSlotInfo("", uiUKey, 0, false, true, 0, 0));
			if(itr == m_kInputSlotContainer.end())
			{//장착무기 타입이랑 관계없는 애니 있으면 그걸로 셋팅하자
				return 0;
			}
		}
		return itr->GetActionID().c_str();
	}
	return 0;


}
void	PgPilot::ActivateExtendedSlot()
{
	//	LocalManager 의 ExtendedSlot 중에 이 파일럿에게 지정되어있지 않은것은 전부 Disable 시킨다.
	if(g_pkLocalManager)
	{
		g_pkLocalManager->DeactivateAllExtenedSlot();

		PgPilot::InputSlotContainer	const	&kCont = GetInputSlotContainer();
		for(PgPilot::InputSlotContainer::const_iterator itor = kCont.begin(); itor != kCont.end(); ++itor)
		{
			PgInputSlotInfo const &kInfo = *itor;
			if(kInfo.GetEnable())
			{
				g_pkLocalManager->ActivateExtendedSlot(kInfo.GetUKey());
			}
		}
	}	
}
void	PgPilot::ReloadXml()
{
	if(!m_pkUnit)
	{
		return;
	}

	int	iClassID = m_pkUnit->GetAbil(AT_CLASS);
	int	iGender = m_pkUnit->GetAbil(AT_GENDER);

	PgPilotMan::stClassInfo	kClassInfo;
	if(g_kPilotMan.GetClassInfo(iClassID,iGender,kClassInfo))
	{
		if(kClassInfo.m_kPilotPath.empty() == false)
		{
			m_kInputSlotContainer.clear();	//	Clear the input slot container
			m_kRidingInputSlotCont.clear();
			m_kContComboActionData.clear();
			m_kContConnectComboActionData.clear();
			m_kContInputKey.clear();

			PgIXmlObject::XmlObjectID eID = GetObjectID();
			PgXmlLoader::CreateObjectFromFile(kClassInfo.m_kPilotPath.c_str(), (void*)&eID, this);
			
			ActivateExtendedSlot();
		}

	}
}
//
//size_t PgPilot::DisplayHeadTransformEffect(bool const bShow)
//{// 머리 변형 스킬(HIDE_ENABLE==1)인 이펙트를 출력 하거나, 감춘다 
//	size_t FoundCnt=0;
//	PgActor* pkActor = g_kPilotMan.FindActor(GetGuid());
//	if(!pkActor)
//	{
//		return 0; 
//	}
//	PgStatusEffectInstance* pkInstance = NULL;
//	StatusEffectInstanceList& kEffectList = pkActor->GetStatusEffectInstanceList();
//	StatusEffectInstanceList::iterator itor = kEffectList.begin();	
//
//	while(itor != kEffectList.end())
//	{// 액터에 적용된 이펙트 리스트 중
//		pkInstance = (*itor);
//		PgStatusEffect const* pkStatusEffect  = pkInstance->GetStatusEffect();
//		PG_ASSERT_LOG(pkStatusEffect);
//		if(true == pkStatusEffect->GetHideEnable())
//		{// XML에서 HIDE_ENABLE이 설정된 값 중
//			if(bShow)
//			{// 감춰진 이펙트를 보이게 한다.
//				if(EVS_VISIBLE != pkInstance->GetVisualState())
//				{
//					pkInstance->SetVisualState(EVS_VISIBLE);
//					pkInstance->AttachEffect(this);					
//				}
//			}
//			else
//			{// 보이는 이펙트를 감춘다.
//				if(EVS_HIDE != pkInstance->GetVisualState())
//				{				
//					pkInstance->SetVisualState(EVS_HIDE);
//					pkInstance->DetachEffect(this);					
//				}
//			}
//			pkInstance->StartEffect(this);
//		//	g_kStatusEffectMan.UpdateEffect(this);
//			++FoundCnt; //몇개나 수정되었는지 카운트
//		}
//		++itor;
//	}
//	return FoundCnt;
//}

void PgPilot::SetAbil(int AbilType, int Value)
{
	int iPreValue = GetAbil(AbilType); // Previous Value

	PG_STAT(PgStatTimerF timer(g_kPilotStatGroup.GetStatInfo("Pilot.SetAbil"), g_pkApp->GetFrameCount()));
	if(m_pkUnit)
	{
		//switch(AbilType)
		//{
		//case AT_POSTURE_STATE:
		//	{
		//		m_pkUnit->SetAbil(AbilType, Value, true, true);
		//	}break;
		//default:
		//	{
				m_pkUnit->SetAbil(AbilType, Value);
		//	}break;
		//}
	}

	PgActor* pkActor = g_kPilotMan.FindActor(GetGuid());

	switch(AbilType)
	{
	case AT_CLASS:	//	We change the pilot xml appropriate to the new class
		{
			ReloadXml();
		}break;
	case AT_CLIENT_OPTION_SAVE:
		{
			PgOptionUtil::SClientDWORDOption const kClientOption(Value);

			if( pkActor )
			{// 투구 보이기 옵션이 설정 됐다면 해당 캐릭터의 이펙트를 보이기/감춤을 수행한다
				//g_kStatusEffectMan.DisplayHeadTransformEffect(pkActor, !kClientOption.DisplayHelmet());
				PgOptionUtil::UpdateOldAndNew(pkActor, PgOptionUtil::SClientDWORDOption(iPreValue), kClientOption);
			}
			//DisplayHeadTransformEffect(!bDisplayHelmet);
		}break;
	case AT_TEAM:
		{
			if( pkActor )
			{
				if( pkActor->IsMyActor() )
				{
					if( g_pkWorld )
					{
						g_pkWorld->AllPlayerUpdateName();
					}
				}
				pkActor->UpdateName();
				pkActor->EquipAllItem();
			}
		}break;
	case AT_ACHIEVEMENT_LEVEL:
		{ //레벨업하면 만피가 된다
			if(NULL != m_pkUnit)
			{
				iPreValue = 1;
				Value = m_pkUnit->GetAbil(AT_C_MAX_HP);
			}
		} //break 원래 없음
	case AT_HP:
		{
			if(pkActor->IsMyActor())
			{
				UpdateLowHPWarnning(iPreValue, Value);
			}
		}break;
	}

	if(m_pkWorldObject)
	{
		m_pkWorldObject->OnAbilChanged(AbilType,Value);
	}
}

void PgPilot::SetAbil64(int AbilType, __int64 Value)
{
	PG_STAT(PgStatTimerF timer(g_kPilotStatGroup.GetStatInfo("Pilot.SetAbil"), g_pkApp->GetFrameCount()));
	if(m_pkUnit)
	{
		m_pkUnit->SetAbil64(AbilType, Value);
	}
}

int PgPilot::GetAbil(int iAbilType) const
{
	PG_STAT(PgStatTimerF timer(g_kPilotStatGroup.GetStatInfo("Pilot.GetAbil"), g_pkApp->GetFrameCount()));
	if ( !m_pkUnit )
		return -1;
	return m_pkUnit->GetAbil( iAbilType );
}

__int64 PgPilot::GetAbil64(int iAbilType) const
{
	PG_STAT(PgStatTimerF timer(g_kPilotStatGroup.GetStatInfo("Pilot.GetAbil"), g_pkApp->GetFrameCount()));
	if ( !m_pkUnit )
		return -1;
	return m_pkUnit->GetAbil64( (EAbilType)iAbilType );
}

void PgPilot::SetKeyActionID(unsigned int uiUKey, std::string const& kActionID, int const iWeaponType)
{
	PgInputSlotInfo kNewInputSlot(kActionID.c_str(), uiUKey,0, false, true, 0, iWeaponType);
	InputSlotContainer::iterator itr = std::find(m_kInputSlotContainer.begin(), m_kInputSlotContainer.end(), kNewInputSlot);
	if(itr == m_kInputSlotContainer.end())
	{
		m_kInputSlotContainer.push_back(kNewInputSlot);
		return;
	}

	std::string cont_actionid = itr->GetActionID();
	if(0 == cont_actionid.compare(kActionID))
	{
		return;
	}

	itr->SetActionID(kActionID);
}

bool PgPilot::IsControllable()
{
	return (0 != m_kInputSlotContainer.size());
}

void PgPilot::RemoveActionKey(unsigned int uiUKey)
{
	InputSlotContainer::iterator itr = std::find(m_kInputSlotContainer.begin(), m_kInputSlotContainer.end(), PgInputSlotInfo("", uiUKey));
	if(itr == m_kInputSlotContainer.end())
	{
		return;
	}
	m_kInputSlotContainer.erase(itr);
}

void PgPilot::SetDirection(BYTE byDirection, DWORD dwDirectionTerm, NiPoint3& rkCurPos)
{
	PgIWorldObject* pkWorldObject = GetWorldObject();
	PgActor* pkActor = dynamic_cast<PgActor*>(pkWorldObject);
	if(!pkActor)
	{
		return;
	}

	pkActor->ReserveDirection(byDirection, dwDirectionTerm, rkCurPos);
}

PgAction* PgPilot::CreateAction(PgRemoteInput* pkRemoteAction)
{
	if(!pkRemoteAction)
	{
		return 0;
	}

	SActionInfo const& rkActionInfo = pkRemoteAction->m_kActionInfo;
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	std::string	kActionID(MB(kSkillDefMgr.GetActionName(rkActionInfo.iActionID)));
	PgAction* pkAction = g_kActionPool.CreateAction(kActionID.c_str());
	if(!pkAction)
	{
		return 0;
	}
	
	pkRemoteAction->m_kTargetList.SetActionInfo(GetGuid(), rkActionInfo.iActionInstanceID, rkActionInfo.iActionID, rkActionInfo.dwTimeStamp);

	pkAction->SetEnable(rkActionInfo.bIsDown);
	pkAction->SetRecord(false);
	pkAction->SetDirection((rkActionInfo.byDirection & 0xF0) >> 4);
	pkAction->SetActionInstanceID(rkActionInfo.iActionInstanceID);
	pkAction->SetActionParam(rkActionInfo.byActionParam);
	pkAction->SetTimeStamp(rkActionInfo.dwTimeStamp);
	pkAction->SetActionTerm(rkActionInfo.dwActionTerm);
	pkAction->SetActionStartPos(NiPoint3(rkActionInfo.ptPos.x, rkActionInfo.ptPos.y, rkActionInfo.ptPos.z));
	pkAction->SetActionNo(rkActionInfo.iActionID);

	pkAction->SetTargetList(pkRemoteAction->m_kTargetList);
	pkAction->SetBirthTime(pkRemoteAction->m_dwSyncTime);
	
	// TODO : 약간 고려해보자 (Clamp인데도 불구하고, 모으는 스킬같은 것이 있을 수 있다)
	// Action의 Cycle Type이 Clamp 이고, Action이 Disable쪽으로 전이해야 한다면 무시
	if(!pkAction->GetActionOptionEnable(PgAction::AO_LOOP) && !pkAction->GetEnable())
	{
		NILOG(PGLOG_WARNING, "[PgPilot] CreateAction %s(%d) action has not loop and disabled\n", kActionID.c_str(), rkActionInfo.iActionID);
		g_kActionPool.ReleaseAction(pkAction);
		return 0;
	}
	
	return pkAction;
}

PgAction* PgPilot::CreateAction(std::string const& kActionName)
{
	PgAction* pkAction = g_kActionPool.CreateAction(kActionName.c_str());
	if(!pkAction)
	{
		return 0;
	}

	PgActionTargetList kTargetList;
	kTargetList.SetActionInfo(GetGuid(), 0, 0, 0);
	
	// ActionInstanceID, TimeStamp, StartPos, Action-Term은 기본으로 0이다.
	pkAction->SetActionParam(ESS_FIRE);
	pkAction->SetEnable(true);
	pkAction->SetRecord(false);
	pkAction->SetDirection(0);
	pkAction->SetTargetList(kTargetList);

	// Action의 Cycle Type이 Clamp 이고, Action이 Disable쪽으로 전이해야 한다면 무시
	if(!pkAction->GetActionOptionEnable(PgAction::AO_LOOP) && !pkAction->GetEnable())
	{
		NILOG(PGLOG_WARNING, "[PgPilot] CreateAction %s(%d) action has not loop and disabled\n", kActionName.c_str(), pkAction->GetActionNo());
		g_kActionPool.ReleaseAction(pkAction);
		return 0;
	}
	
	return pkAction;
}

PgAction* PgPilot::CreateAction(PgInput* pkInput)
{
	PG_ASSERT_LOG(pkInput);

	char const* pcActionID = NULL;
	PgInputSlotInfo *pkSlotInfo = FindAction(pkInput->GetUKey());
	if(!pkSlotInfo)
	{
		return 0;
	}

	switch( pkSlotInfo->GetTargetType() )
	{// 입력이
	case PgInputSlotInfo::ETT_SUB_PLAYER:
		{// 보조 캐릭터의 액션을 유발하는 것이라면
			PgActor* pkActor = g_kPilotMan.GetPlayerActor();
			if(pkActor)
			{
				PgActor* pkSubPlayerActor = PgActorUtil::GetSubPlayerActor( pkActor );
				if(pkSubPlayerActor)
				{// 보조캐릭터에게 액션을 전달 하고
					std::string const& kActionName = pkSlotInfo->GetActionID();
					lwCommonSkillUtilFunc::TryMustChangeSubPlayerAction( pkActor, kActionName.c_str(), pkActor->GetDirection() );
				}
			}
			return 0;
		}break;
	}

	PgAction* pkAction = g_kActionPool.CreateAction(pkSlotInfo->GetActionID().c_str());
	if(!pkAction)
	{
		return 0;
	}

	// 커맨드 봉인 시스템 : 이부분은 롤백 결정났음. 사용 안함. 여기만 막으면 다 막힌 것.
	//if(false == CanExcuteByCommandKey(pkAction))
	//{
	//	g_kActionPool.ReleaseAction(pkAction);
	//	return 0;
	//}

	_PgOutputDebugString("PgPilot::CreateAction Action:%s Pressed:%d\n",pkAction->GetID().c_str(),pkInput->GetPressed());

	Direction eDir = (Direction)(int)pkSlotInfo->GetUserData();

	PgActionTargetList kTargetList;
	kTargetList.SetActionInfo(GetGuid(), 0, 0, 0);
	
	// ActionInstanceID, TimeStamp, StartPos, Action-Term은 기본으로 0이다.
	pkAction->SetActionParam(ESS_FIRE);
	pkAction->SetEnable(pkInput->GetPressed());
	pkAction->SetRecord(pkSlotInfo->IsRecord());
	pkAction->SetDirection(eDir);
	pkAction->SetTargetList(kTargetList);
	pkAction->SetInputSlotInfo(pkSlotInfo);

	PgActor* pkActor = dynamic_cast<PgActor*>(GetWorldObject());
	if(pkActor)
	{
		pkAction->SetActionStartPos(pkActor->GetPos());
	}

	// Action의 Cycle Type이 Clamp 이고, Action이 Disable쪽으로 전이해야 한다면 무시
	if(!pkAction->GetActionOptionEnable(PgAction::AO_LOOP) && !pkAction->GetEnable())
	{
		NILOG(PGLOG_WARNING, "[PgPilot] CreateAction %s(%d) action has not loop and disabled\n", pkSlotInfo->GetActionID().c_str(), pkAction->GetActionNo());
		g_kActionPool.ReleaseAction(pkAction);
		return 0;
	}

	_PgOutputDebugString("PgPilot::CreateAction UKey:%u Pressed:%d Direction:%d\n",pkInput->GetUKey(),pkInput->GetPressed(),eDir);
	
	return pkAction;
}
bool	PgPilot::CanExcuteByCommandKey(PgAction *pkAction)
{
	CSkillDef	const	*pkSkillDef = pkAction->GetSkillDef();

	if(ESK_NONE == pkSkillDef->GetAbil(AT_SKILL_KIND))
	{
		return	true;
	}

	if(pkSkillDef->GetType() != EST_ACTIVE && pkSkillDef->GetType() != EST_TOGGLE)
	{
		return	true;
	}

	DWORD	const	dwKeySkillNo = g_kSkillTree.GetKeySkillNo(pkAction);
	PgSkillOptionCommandKey	*pkCommandKeyEnable = NiDynamicCast(PgSkillOptionCommandKey,g_kSkillOptionMan.GetSkillOption(dwKeySkillNo,PgSkillOptionCommandKey::GetTypeS()));
	if(pkCommandKeyEnable)
	{
		return	pkCommandKeyEnable->GetEnableCommandKey();
	}
	return	true;
}
void PgPilot::RecvNfyAction(SActionInfo& rkActionInfo, BM::Stream* pkPacket)
{
	PG_ASSERT_LOG(pkPacket);

	CUnit* pkUnit = GetUnit();
	PG_ASSERT_LOG(pkUnit);
	if (NULL == pkUnit)
	{
		return;
	}

	if (false==pkUnit->IsInUnitType(UT_PLAYER) && false==pkUnit->IsInUnitType(UT_PET) )
	{
		EUnitState const eState = ESS_FIRE==rkActionInfo.byActionParam ? US_SKILL_FIRE : static_cast<EUnitState>(rkActionInfo.iUnitState);
		pkUnit->SetState(eState);
		if(US_SKILL_CAST == eState)
		{
			pkUnit->GetSkill()->Reserve(rkActionInfo.iActionID);
		}
	}
	else if(pkUnit->IsInUnitType(UT_PLAYER))
	{ //AddUnit 시점에서의 동기화 처리: 유닛 타입이 PC이면 actor말고 Unit 인스턴스에도 적용해준다.
		//패킷이 먼저 오고 액터가 로딩 완료되므로 유닛에 가장 최신의 액션상태를 저장해둔다.
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
		if(pkPlayer)
		{
			pkPlayer->SetActionID(rkActionInfo.iActionID, rkActionInfo.byDirection);
		}

	}

	//unsigned short sGoalRange=0;
	int iCurrentJumpHeight = 0;
#ifdef PG_SYNC_ENTIRE_TIME
	DWORD dwSyncTime;

	pkPacket->Pop(dwSyncTime);

	//NILOG(PGLOG_NETWORK, "[PgPilot] The difference of action's birthtime : (%d)\n", PgActor::GetSynchronizedTime() - dwSyncTime);
#endif
	//WriteToConsole("[PgPilot] The difference of action's birthtime : (%d)\n", PgActor::GetSynchronizedTime() - dwSyncTime);

	POINT3 pt3GoalPos;
	pkPacket->Pop(pt3GoalPos);
	if(POINT3::NullData()!=pt3GoalPos && pkUnit->UnitType()!=UT_PET)
	{
		pkUnit->GoalPos(pt3GoalPos);
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkDef = kSkillDefMgr.GetDef(rkActionInfo.iActionID);

	NILOG(PGLOG_LOG, "M_C_NFY_ACTION2 Pilot(%d) : %s(%s) SkillID: %d SkillName:%s ActionParam : %d InstanceID : %d ActionTerm  :%d ActionPos : (%f,%f,%f)\n", rkActionInfo.dwTimeStamp, MB(GetName()), MB(GetGuid().str()), rkActionInfo.iActionID, MB(kSkillDefMgr.GetActionName(rkActionInfo.iActionID)), rkActionInfo.byActionParam, rkActionInfo.iActionInstanceID, rkActionInfo.dwActionTerm, rkActionInfo.ptPos.x, rkActionInfo.ptPos.y, rkActionInfo.ptPos.z);
	_PgOutputDebugString("M_C_NFY_ACTION2 Pilot : %s(%s) IsLeft : %d SkillID: %d SkillName:%s ActionParam : %d InstanceID : %d Pos : (%f,%f,%f) \n", MB(GetName()), MB(GetGuid().str()), rkActionInfo.byDirection, rkActionInfo.iActionID, MB(kSkillDefMgr.GetActionName(rkActionInfo.iActionID)), rkActionInfo.byActionParam, rkActionInfo.iActionInstanceID, rkActionInfo.ptPos.x, rkActionInfo.ptPos.y, rkActionInfo.ptPos.z);

	BYTE byTargetNum;
	PgActionTargetList kTargetList;
 	kTargetList.SetActionInfo(rkActionInfo.guidPilot, rkActionInfo.iActionInstanceID, rkActionInfo.iActionID, rkActionInfo.dwTimeStamp);

	PgActionResultVector kActionResultVec;
	kActionResultVec.ReadFromPacket(*pkPacket);
	byTargetNum = kActionResultVec.GetValidCount();

	BM::GUID kTargetGuid;
	// 수정이 필요하다 싶은곳 (by Eric)
	//	1. kTargetInfo.m_iRemainHP,kTargetInfo.m_bySphereIndex : ActionResult에 이미 있는 중복된 Data 입니다.
	//	2. kTargetInfo.m_iRemainHP : int 형으로 표현해야 합니다.
	//	3. kTargetInfo.m_kActionResult : Copy Constructor 를 호출하게 됩니다. 더 빠른 방법은 없을런지.
	//		kTargetInfo를 사용하지 말고 처음부터 PgActionResultVector를 처음부터 사용하는것은어떨런지.
	if (byTargetNum == 0)
	{
		// Target 정보 초기화
		bool bSetTargetNull = true;
		if(UT_MONSTER&pkUnit->UnitType())
		{
			bSetTargetNull = rkActionInfo.iActionID == ACTION_NO_IDLE;	//몬스터일 경우 아이들로 돌아갈 때만 초기화 시키자
		}
		if(bSetTargetNull)
		{
			if(!(UT_PET&pkUnit->UnitType()))
			{
				pkUnit->SetTarget(BM::GUID::NullData());
			}
		}
	}
	for (BYTE byI = 0; byI < byTargetNum; ++byI)
	{
		PgActionResult* pkActionResult = kActionResultVec.GetResult(byI, kTargetGuid);
		if (NULL == pkActionResult)
		{
			NILOG(PGLOG_ERROR, "[PgPilot] %s Cannot GetActionResult Actor[%s], Index[%d], ActionResultSize[%d]\n",__FUNCTION__, MB(rkActionInfo.guidPilot.str()), (int)byI, (int)byTargetNum);
		//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] Cannot GetActionResult Actor[%s], Index[%d], ActionResultSize[%d]"), __FUNCTIONW__, rkActionInfo.guidPilot.str(), (int)byI, (int)byTargetNum);
			break;
		}

		pkUnit->SetTarget(kTargetGuid);
		PgActionTargetInfo kTargetInfo;
		kTargetInfo.SetTargetPilotGUID(kTargetGuid);
		kTargetInfo.SetRemainHP(pkActionResult->IsValidHP() ? pkActionResult->GetRemainHP() : INVALID_REMAIN_HP);
		kTargetInfo.SetActionResult(*pkActionResult);
		kTargetInfo.GetActionResult().SetRealEffect(true);
		kTargetInfo.SetSphereIndex(pkActionResult->GetCollision());

		pkActionResult->SetAbil(AT_REFLECTED_DAMAGE,rkActionInfo.byActionParam==ESS_FIRE ? pkActionResult->GetAbil(AT_REFLECTED_DAMAGE) : INVALID_REMAIN_HP);

		_PgOutputDebugString("PgPilot::RecvNfyAction TargetGUID:%s RemainHP:%d ReflectionDamage : %d ReflectionTarget:%s RemainHP: %d\n", MB(kTargetGuid.str()),kTargetInfo.GetRemainHP(),pkActionResult->GetAbil(AT_REFLECTED_DAMAGE),MB(GetGuid().str()), pkActionResult->GetAbil(AT_REFLECT_DMG_HP));

		if(0 == kTargetInfo.GetRemainHP())
		{
			_PgOutputDebugString("PgPilot::RecvNfyAction RemainHP Is 0, Attacker : %s Target : %s\n", MB(GetGuid().str()),MB(kTargetInfo.GetTargetPilotGUID().str()));

			PgPilot* pkTargetPilot = g_kPilotMan.FindPilot(kTargetGuid);
			if(pkTargetPilot)
			{
				PgActor* pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
				if(pkActor)
				{
					//pkActor->ReserveDieByActioin(true);
					_PgOutputDebugString("Die Action Reserved\n");
				}
			}
		}

		//INFO_LOG(BM::LOG_LV9, _T("M_C_NFY_ACTION2 Pilot : %s(%s) SkillID:%d, ActionParam:%d, InstanceID:%d, IsValidHP : %d Pos:(%6.2f,%6.2f,%6.2f)"),
		//	GetName(), GetGuid().str().c_str(), rkActionInfo.iActionID, rkActionInfo.byActionParam, 
		//	rkActionInfo.iActionInstanceID,pkActionResult->IsValidHP(),rkActionInfo.ptPos.x,rkActionInfo.ptPos.y,rkActionInfo.ptPos.z);

//		WriteToConsole("[==========M_C_NFY_ACTION2==========] Pilot : %s SkillID:%d, ActionParam:%d, InstanceID:%d, Pos:(%6.2f,%6.2f,%6.2f)\n",
//			MB(GetName()), rkActionInfo.iActionID, rkActionInfo.byActionParam, 
//			rkActionInfo.iActionInstanceID,rkActionInfo.ptPos.x,rkActionInfo.ptPos.y,rkActionInfo.ptPos.z);

		// HP가 0보다 작으면 Die Effect를 적용시킨다.
		// HP값은 ActionResult->IsValidHP() 일때만 유효하다.
		if (pkDef != NULL && pkDef->GetParentSkill() != ACTIONEFFECT_RESURRECTION1 && ACTIONEFFECT_RESURRECTION01 != rkActionInfo.iActionID)
		{
			if(pkActionResult->IsValidHP() && kTargetInfo.GetRemainHP() <= 0)
			{
				_PgOutputDebugString("PgPilot::RecvNfyAction RemainHP Is 0, Attacker : %s Target : %s\n", MB(GetGuid().str()),MB(kTargetInfo.GetTargetPilotGUID().str()));

				kTargetInfo.SetRemainHP((short)NiMax((float)kTargetInfo.GetRemainHP(), 0.0f));
				if(PlayerPilotGuid() != kTargetInfo.GetTargetPilotGUID())
				{
					kTargetInfo.GetActionResult().ClearEffect();
					kTargetInfo.GetActionResult().AddEffect(ACTIONEFFECT_DIE);	//	DIE EFFECT
					_PgOutputDebugString("ACTIONEFFECT_DIE Added\n");
				}
			}
		}		
		else //부활로 살아나라는 패킷이 올 경우
		{
			kTargetInfo.GetActionResult().ClearEffect();
			kTargetInfo.GetActionResult().AddEffect(ACTIONEFFECT_RESURRECTION01);	//	RESURRECTION_01 EFFECT
			_PgOutputDebugString("ACTIONEFFECT_RESURRECTION01 Added\n");
		}	

		kTargetList.GetList().push_back(kTargetInfo);
	}
	
	PgActor* pkActor = dynamic_cast<PgActor*>(GetWorldObject());
	// 내 액터일 경우.
	if(PgPilot::PlayerPilotGuid() == GetGuid() || (pkActor && pkActor->IsUnderMyControl()))
	{		
		if(rkActionInfo.byActionParam != ESS_TARGETLISTMODIFY)
		{
			if(pkActor)
			{
				PgAction* pkAction = pkActor->GetAction();
				if(pkAction && pkAction->GetActionInstanceID() == rkActionInfo.iActionInstanceID)
				{
					pkAction->ModifyTargetList(kTargetList);
				}
			}

			g_kActionTargetTransferInfoMan.ModifyTargetList(kTargetList);
		
			if(kTargetList.size() > 0)
			{
//				_PgOutputDebugString("[PgPilot::RecvNfyAction()] Apply Action Effects with Only Die Effect. TargetList : %d\n", kTargetList.size());
				kTargetList.ApplyActionEffects(true,true);
			}

			return;
		}
		else
		{
			//내가 소환한 Entity이고 Action이 NULL인 경우는
			if(pkActor && pkActor->IsUnderMyControl() && NULL == (pkActor->GetAction()))
			{
				if(kTargetList.size() > 0)
				{
					kTargetList.ApplyActionEffects(false,true);
				}
			}
		}
	}


	// Remote Input 처리.
	NILOG(PGLOG_LOG,"[PgPilot.RecvNfyAction] Actor : %s Action : %s ActionInstanceID : %d ActionTerm : %d\n", MB(GetGuid().str()), MB(GetName()), rkActionInfo.iActionInstanceID, rkActionInfo.dwActionTerm);
	
	rkActionInfo.ptPos.z += PG_CHARACTER_Z_ADJUST;

	if(pkActor)
	{
		//	캐릭터의 위치 오차가 너무 클 경우 그냥 순간이동시킨다.
		NiPoint3	kNewPos(rkActionInfo.ptPos.x,rkActionInfo.ptPos.y,rkActionInfo.ptPos.z);
		float	fPositionError = (pkActor->GetPosition() - kNewPos).SqrLength();
		if(fPositionError>3600.0f)	//60이상
		{
			NiPoint3 kDiff = pkActor->GetPosition() - kNewPos;
			kDiff.z = 0.0f;
			if(900.0f < kDiff.SqrLength())	//Z 빼고 X, Y가 오차가 30이상 크면
			{
				if(g_pkWorld)
				{
					PgAction* pkAction = pkActor->GetAction();
					if (pkAction && false == pkAction->GetActionOptionEnable(PgAction::AO_NO_CHANGE_TELEPORT_ACTOR_POS))
					{
						pkActor->SetPosition(g_pkWorld->FindActorFloorPos(kNewPos));
						NILOG(PGLOG_LOG,"[PgPilot.RecvNfyAction] Pos Diff is Over Then 60. ActionID:%d\n XDiff:%f, YDiff:%f, ZDiff:%f\n",rkActionInfo.iActionID, kDiff.x, kDiff.y, kDiff.z);
					}
				}
			}
		}
	}
	
	// TODO : rkActionInfo 복사가 너무 많다. 줄이자..
	PgRemoteInput kRemoteInput(rkActionInfo, kTargetList, dwSyncTime);
	PgAction* pkAction = CreateAction(&kRemoteInput);

	// 추가적인 Packet은 Param으로 둔다.
	bool bExtraData = false;
	pkPacket->Pop(bExtraData);
	if(pkAction && bExtraData)
	{
		BM::Stream* pkExtraPacket = new BM::Stream();
		pkExtraPacket->Push(*pkPacket);
		pkAction->SetParamAsPacket(pkExtraPacket);
	}

	ProcessAction(pkAction,true);
}

bool PgPilot::ProcessAction(PgAction* pkAction,bool bFromServer)
{
	if (!pkAction || m_pkWorldObject == NULL)
	{
		PG_ASSERT_LOG(m_pkWorldObject);
		return 0;
	}

	// Action의 Cycle Type이 Clamp 이고, Action이 Disable쪽으로 전이해야 한다면 무시
	if(!pkAction->GetActionOptionEnable(PgAction::AO_LOOP) && !pkAction->GetEnable())
	{
		std::string str;
		pkAction->GetActionName(str);
		NILOG(PGLOG_WARNING, "[PgPilot] CreateAction %s(%d) action has not loop and disabled\n", str.c_str(), pkAction->GetActionNo());
		g_kActionPool.ReleaseAction(pkAction);
		return 0;
	}

	if(m_pkWorldObject && m_pkWorldObject->IsMyActor() && false == bFromServer)
	{
		//	서버에서 온 액션이 아니라, 클라가 스스로 생성한 액션이므로, 액션 인스턴스 ID 를 새로 할당한다.
		pkAction->SetActionInstanceID();

		if(pkAction->GetTargetList())
		{
			pkAction->GetTargetList()->SetActionInfo(GetGuid(),pkAction->GetActionInstanceID(),pkAction->GetActionNo(),pkAction->GetTimeStamp());
		}

		if(pkAction->GetSkillType() == EST_ACTIVE || pkAction->GetSkillType() == EST_TOGGLE)
		{
			PgSkillTree::stTreeNode *pkNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(pkAction));
			if(pkNode)
			{
				pkAction->SetActionNo(pkNode->m_ulSkillNo);
			}
		}

		//PgActor *pkActor = dynamic_cast<PgActor *>(m_pkWorldObject);

		// TODO : 충돌나므로 주석처리.
		//pkAction->SetDirection(pkActor->GetDirection());
	}

	if(bFromServer)
	{
		pkAction->SetAddToActionEntity(true);
		pkAction->SetActionOption(PgAction::AO_IGNORE_NOTACTIONSHIFT, true);
	}

	// 액션의 실제 처리는 여기서 한다.
	return m_pkWorldObject->ProcessAction(pkAction,m_pkWorldObject->IsMyActor());
}

void PgPilot::SetWorldObject(PgIWorldObject *pkWorldObject)
{
	m_pkWorldObject = pkWorldObject;
}

PgIWorldObject* PgPilot::GetWorldObject()
{
	return m_pkWorldObject;
}

BM::GUID const& PgPilot::GetGuid() const
{
	if(m_pkUnit)
	{
		return m_pkUnit->GetID();
	}
	PG_ASSERT_LOG(!"PgPilot::GetGuid() Unit is NULL");
	return BM::GUID::NullData();
}

void PgPilot::SetGuid(BM::GUID const& rkGuid)
{
	if(m_pkUnit)
	{
		m_pkUnit->SetID(rkGuid);
	}
	PG_ASSERT_LOG(NULL&& "PgPilot::SetGuid() Unit is NULL");
}

void PgPilot::SetName(std::wstring const &wName)
{
	if(m_pkUnit)
	{
		return m_pkUnit->Name(wName);
	}
	PG_ASSERT_LOG(NULL);
}

std::wstring const PgPilot::GetName() const
{
	if(m_pkUnit)
	{
		return m_pkUnit->Name();
	}
	PG_ASSERT_LOG(NULL);
	return _T("");
}

bool PgPilot::ParseXml(TiXmlNode const* pkNode, void* pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement* pkElement = (TiXmlElement*)pkNode;
			PG_ASSERT_LOG(pkElement);

			char const* pcTagName = pkElement->Value();

			if(0 == strcmp(pcTagName, "PILOT"))
			{

				

				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
				std::string kActorID;

				while(pkAttr)
				{
					char const* pcName = pkAttr->Name();
					char const* pcValue = pkAttr->Value();

					if(0 == strcmp(pcName, "NAME"))
					{
//						SetName(std::wstring(UNI(pcValue)));
//						PG_ASSERT_LOG(NULL && "Name Is Not Use Now");
					}
					else
					{
						PgError1("PgPilot : unknown attribute tag : %s", pcName);
						break;
					}
					pkAttr = pkAttr->Next();
				}

				TiXmlNode* pkChildNode = pkElement->FirstChild();
				if(0 != pkChildNode)
				{
					if(!ParseXml(pkChildNode, pArg))
					{
						return false;
					}
				}
			}
			else if(0 == strcmp(pcTagName, "INPUT"))
			{
				TiXmlElement const* pkChildElement = pkNode->FirstChildElement();
				
				while(pkChildElement)
				{
					char const* pcChildTagName = pkChildElement->Value();

					if(0 == strcmp(pcChildTagName, "SLOT"))
					{
						TiXmlAttribute const* pkAttr = pkChildElement->FirstAttribute();
						std::string kActionID;
						unsigned int uiKey = 0;
						int iParam = 0;
						bool bRecord = false;
						bool bEnable = true;
						bool bMountedPet = false;
						int iTagetType = 0;
						float	fDelayTime = 0;
						int iWeaponType = 0;
						
						while(pkAttr)
						{
							char const* pcName = pkAttr->Name();
							char const* pcValue = pkAttr->Value();

							if(0 == strcmp(pcName, "ACTION-ID"))
							{
								kActionID = std::string(pcValue);
							}
							else if(0 == strcmp(pcName, "INPUT"))
							{
								uiKey = atoi(pcValue);
							}
							else if(0 == strcmp(pcName, "USERDATA"))
							{
								NiFixedString kParam(pcValue);
								iParam = (kParam == "DIR_LEFT") ? DIR_LEFT :
									(kParam == "DIR_RIGHT") ? DIR_RIGHT :
									(kParam == "DIR_UP") ? DIR_UP : 
									(kParam == "DIR_DOWN") ? DIR_DOWN : DIR_NONE;
							}
							else if(0 == strcmp(pcName, "RECORD"))
							{
								bRecord = (0 == strcmp(pcValue, "TRUE"));
							}
							else if(0 == strcmp(pcName, "ENABLE"))
							{
								bEnable = (0 == strcmp(pcValue, "TRUE"));
							}

							else if(0 == strcmp(pcName, "DELAY"))
							{
								fDelayTime = static_cast<float>(atof(pcValue));
							}
							else if(0 == strcmp(pcName, "MOUNTEDPET"))
							{
								bMountedPet = (0 == strcmp(pcValue, "TRUE") || 0 == strcmp(pcValue, "true") || 0 == strcmp(pcValue, "1"));
							}
							else if(0 == ::strcmp(pcName, "TARGET_TYPE"))
							{
								iTagetType = PgStringUtil::SafeAtoi(pcValue);
							}
							else if(0 == ::strcmp(pcName, "WEAPON_TYPE"))
							{
								iWeaponType = PgStringUtil::SafeAtoi(pcValue);
							}
							else
							{
								PgXmlError1(pkChildElement, "XmlParse: Incoreect Attr '%s'", pcName);
								break;
							}
							pkAttr = pkAttr->Next();
						}
						
						if(bMountedPet)
						{
							m_kRidingInputSlotCont.push_back(PgInputSlotInfo(kActionID.c_str(), uiKey, (void*)iParam, bRecord, bEnable, iTagetType, iWeaponType));
						}
						else
						{
							m_kInputSlotContainer.push_back(PgInputSlotInfo(kActionID.c_str(), uiKey, (void*)iParam, bRecord, bEnable, iTagetType, iWeaponType));
						}
					}
					else
					{		
						PgXmlError1(pkChildElement, "XmlParse: Incoreect Tag '%s'", pcChildTagName);
						break;
					}

					pkChildElement = pkChildElement->NextSiblingElement();
				}
			}
			else if(0 == strcmp(pcTagName, "COMBO_LIST"))
			{
				ParseComboData(pkNode);
			}
			else
			{
				PgError1("PgPilot : unknown element tag - %s", pcTagName);
			}
		}
		break;

	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	TiXmlNode const* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode, pArg))
		{
			return false;
		}
	}

	return true;
}
void PgPilot::ParseComboData( TiXmlNode const *pkNode )
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	TiXmlElement const* pkActionElement = pkNode->FirstChildElement();
	while(pkActionElement)
	{
		char const* pcChildTagName = pkActionElement->Value();
		int iCurrentSkillNo = 0;
		bool bConnectCombo = false;
		CONT_VEC_COMBO_KEY_DATA kContComboKeyData;
		if(0 == strcmp(pcChildTagName, "ACTION"))
		{
			tagComboKeyData kComboKeyData;
			kComboKeyData.bComboConnect = false;
			kComboKeyData.iNextSkillNo = 0;
			kComboKeyData.iNextSkillNo_SC = 0;
			kComboKeyData.iNextSkillNo_Fusion = 0;
			kComboKeyData.iWeaponType = 0;
			TiXmlAttribute const* pkAttr = pkActionElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcName = pkAttr->Name();
				char const* pcValue = pkAttr->Value();
				if(0 == strcmp(pcName, "SKILL_NO"))
				{
					iCurrentSkillNo = atoi(pcValue);
					if( !kSkillDefMgr.GetDef(iCurrentSkillNo) )
					{
						PgXmlError1(pkActionElement, "XmlParse: It's Not SkillNo '%d'", iCurrentSkillNo);
					}
				}
				else if(0 == strcmp(pcName, "TRANSIT_ACTION"))
				{
					kComboKeyData.iNextSkillNo = atoi(pcValue);
					if( !kSkillDefMgr.GetDef(kComboKeyData.iNextSkillNo) )
					{
						PgXmlError1(pkActionElement, "XmlParse: It's Not SkillNo '%d'", kComboKeyData.iNextSkillNo);
					}
				}
				else if(0 == strcmp(pcName, "SC_TRANSIT_ACTION"))
				{
					kComboKeyData.iNextSkillNo_SC = atoi(pcValue);
					if( !kSkillDefMgr.GetDef(kComboKeyData.iNextSkillNo_SC) )
					{
						PgXmlError1(pkActionElement, "XmlParse: It's Not SkillNo '%d'", kComboKeyData.iNextSkillNo_SC);
					}
				}
				else if(0 == strcmp(pcName, "FUSION_TRANSIT_ACTION"))
				{
					kComboKeyData.iNextSkillNo_Fusion = atoi(pcValue);
					if( !kSkillDefMgr.GetDef(kComboKeyData.iNextSkillNo_Fusion) )
					{
						PgXmlError1(pkActionElement, "XmlParse: It's Not SkillNo '%d'", kComboKeyData.iNextSkillNo_SC);
					}
				}
				else if(0 == strcmp(pcName, "COMBO_CONNECT"))
				{
					if( 0 < atoi(pcValue) )
					{
						kComboKeyData.bComboConnect = true;
						bConnectCombo = true;
					}
					else
					{
						kComboKeyData.bComboConnect = false;
						bConnectCombo = false;
					}
				}
				else if(0 == strcmp(pcName, "WEAPON_TYPE"))
				{
					kComboKeyData.iWeaponType = atoi(pcValue);
				}
				else
				{
					PgXmlError1(pkActionElement, "XmlParse: Incoreect Attr '%s'", pcName);
					break;
				}
				pkAttr = pkAttr->Next();
			}
			TiXmlElement const* pkConditionElement = pkActionElement->FirstChildElement();
			while(pkConditionElement)
			{
				tagKeyInfo kKeyInfo;
				TiXmlAttribute const* pkAttr = pkConditionElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcName = pkAttr->Name();
					char const* pcValue = pkAttr->Value();
					if(0 == strcmp(pcName, "U_KEY"))
					{
						kKeyInfo.iUKeyNo = atoi(pcValue);
					}
					else if(0 == strcmp(pcName, "U_KEY_STATE"))
					{
						kKeyInfo.eKeyState = static_cast<eUKeyState>(atoi(pcValue));
					}
					else if(0 == strcmp(pcName, "U_KEY_TIME"))
					{
						kKeyInfo.fKeyTime = atof(pcValue) * 0.001;
					}
					else
					{
						PgXmlError1(pkConditionElement, "XmlParse: Incoreect Attr '%s'", pcName);
						break;
					}
					pkAttr = pkAttr->Next();
				}
				kComboKeyData.kContKeyInfo.push_back(kKeyInfo);
				pkConditionElement = pkConditionElement->NextSiblingElement();
			}
			kContComboKeyData.push_back(kComboKeyData);
		}
		else
		{		
			PgXmlError1(pkActionElement, "XmlParse: Incoreect Tag '%s'", pcChildTagName);
			break;
		}
		if( bConnectCombo )
		{
			InsertComboData(pkActionElement, iCurrentSkillNo, m_kContConnectComboActionData, kContComboKeyData);
		}
		else
		{
			InsertComboData(pkActionElement, iCurrentSkillNo, m_kContComboActionData, kContComboKeyData);
		}
		pkActionElement = pkActionElement->NextSiblingElement();
	}
}
bool PgPilot::InsertComboData(TiXmlNode const *pkActionElement, int const iCurrentSkillNo, CONT_MAP_COMBO_ACTION_DATA &rkDestContComboActionData, CONT_VEC_COMBO_KEY_DATA &rkContComboKeyData)
{
	CONT_MAP_COMBO_ACTION_DATA::iterator iter_Combo = rkDestContComboActionData.find(iCurrentSkillNo);
	if( rkDestContComboActionData.end() != iter_Combo )
	{
		bool bEnableInsert = true;
		CONT_MAP_COMBO_ACTION_DATA::mapped_type &rkActionData = (*iter_Combo).second;
		CONT_VEC_COMBO_KEY_DATA::iterator iter_Temp = rkContComboKeyData.begin();
		while( rkContComboKeyData.end() != iter_Temp )
		{
			CONT_VEC_COMBO_KEY_DATA::iterator iter_Data = rkActionData.begin();
			while(rkActionData.end() != iter_Data)
			{
				CONT_VEC_INPUT_KEY::value_type kErrorData;
				CONT_VEC_INPUT_KEY::iterator iter_Key = (*iter_Data).kContKeyInfo.begin();
				CONT_VEC_INPUT_KEY::iterator iter_Key_Temp = (*iter_Temp).kContKeyInfo.begin();
				while( (*iter_Data).kContKeyInfo.end() != iter_Key &&
					(*iter_Temp).kContKeyInfo.end() != iter_Key_Temp )
				{
					if( (*iter_Temp).bComboConnect )
					{
						if( (*iter_Key).iUKeyNo == (*iter_Key_Temp).iUKeyNo 
							&& (*iter_Key).eKeyState == (*iter_Key_Temp).eKeyState
							&& (*iter_Key).fKeyTime == (*iter_Key_Temp).fKeyTime 
							&& (*iter_Data).iNextSkillNo == (*iter_Temp).iNextSkillNo 
							&& (*iter_Data).iWeaponType == (*iter_Temp).iWeaponType )
						{//중계스킬은 스킬번호가 같은지도 확인
							bEnableInsert = false;
							kErrorData = (*iter_Key_Temp);
						}
						else
						{
							bEnableInsert = true;
							break;
						}
					}
					else
					{
						if( (*iter_Key).iUKeyNo == (*iter_Key_Temp).iUKeyNo 
							&& (*iter_Key).eKeyState == (*iter_Key_Temp).eKeyState
							&& (*iter_Key).fKeyTime == (*iter_Key_Temp).fKeyTime 
							)
						{
							bEnableInsert = false;
							kErrorData = (*iter_Key_Temp);
						}
						else
						{
							bEnableInsert = true;
							break;
						}
					}
					++iter_Key_Temp;
					++iter_Key;
				}		
				if( !bEnableInsert )
				{
					BM::vstring kFailText(L"Fail Insert Combo Skill [#CURRENT#]->[#NEXT#]: Same State - UKey[#UKEY#] / KeyState[#STATE#] / KeyTime[#TIME#]");
					kFailText.Replace(L"#CURRENT#", iCurrentSkillNo );
					kFailText.Replace(L"#NEXT#", (*iter_Temp).iNextSkillNo );
					kFailText.Replace(L"#UKEY#", kErrorData.iUKeyNo );
					kFailText.Replace(L"#STATE#", kErrorData.eKeyState );
					kFailText.Replace(L"#TIME#", static_cast<int>(kErrorData.fKeyTime )) ;

					PgXmlError(pkActionElement, static_cast<std::string>(kFailText).c_str() );
					break;
				}
				++iter_Data;
			}
			rkActionData.push_back( (*iter_Temp) );
			++iter_Temp;
		}
	}
	else
	{
		std::pair<CONT_MAP_COMBO_ACTION_DATA::iterator, bool> kRet = rkDestContComboActionData.insert( std::pair<int, CONT_VEC_COMBO_KEY_DATA>(iCurrentSkillNo, rkContComboKeyData) );
		if( false == kRet.second )
		{
			PgXmlError1(pkActionElement, "Fail Insert Combo Skill - Insert Fail [%d]", (*iter_Combo).first );
			return false;
		}
	}
	return true;
}

void PgPilot::SetUnit(BM::GUID const& kGUID,int iUnitType,int iClassNo,int iLevel,int iGender)
{
	CUnit* pkUnit = g_kTotalObjMgr.CreateUnit((EUnitType)iUnitType, kGUID);//임시로 유닛을 붙여주는것이다. 알아서 없어질꺼다.
	PG_ASSERT_LOG(pkUnit);

	if (pkUnit->UnitType() == UT_PLAYER)
	{
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
		if (pkPlayer)
		{
			pkPlayer->SetAbilDirect(AT_CLASS, iClassNo);
			pkPlayer->SetAbilDirect(AT_LEVEL, iLevel);
		}		
	}
	else
	{
		pkUnit->CUnit::SetAbil(AT_CLASS, iClassNo);
		pkUnit->SetAbil(AT_LEVEL, iLevel);
	}
	pkUnit->SetAbil(AT_GENDER, iGender);

	switch(iUnitType)
	{
	case UT_ENTITY:
		{
			SEntityInfo kInfo( kGUID, SClassKey(iClassNo, iLevel) );
			pkUnit->Create(&kInfo);
		}break;
	case UT_OBJECT:
		{
			SObjUnitGroupIndex	const kIndex;
			SObjUnitBaseInfo	kBaseInfo(kIndex);
			SObjUnitCreateInfo	kInfo(kBaseInfo);

			kInfo.kGuid = kGUID;
			kInfo.iID = iClassNo;
			kInfo.dwAttribute = 0;

			pkUnit->Create(&kInfo);
		}break;
	}

	SetUnit(pkUnit);
}

void PgPilot::SetUnit(CUnit* pkUnit)
{
	PG_ASSERT_LOG(pkUnit);
	if (NULL == pkUnit)
	{
		return;
	}

	CUnit* pkOldUnit = m_pkUnit;
	PG_ASSERT_LOG(pkOldUnit != pkUnit);
	if (pkOldUnit == pkUnit)
	{
		NILOG(PGLOG_ERROR, "[PgPilot] SetUnit %s(%#X) who has same unit\n", MB(pkOldUnit->GetID().str()), pkOldUnit);
	}

	if (pkOldUnit)
	{
		if (pkOldUnit->GetID() == pkUnit->GetID())
		{
			NILOG(PGLOG_LOG, "[PgPilot] SetUnit %s(%#X) -> %s(%#X) - same GUID\n", MB(pkOldUnit->GetID().str()), pkOldUnit, MB(pkUnit->GetID().str()), pkUnit);
		}
		else
		{
			NILOG(PGLOG_LOG, "[PgPilot] SetUnit %s(%#X) -> %s(%#X)\n", MB(pkOldUnit->GetID().str()), pkOldUnit, MB(pkUnit->GetID().str()), pkUnit);
		}

		g_kTotalObjMgr.UnRegistUnit(pkOldUnit);
		g_kTotalObjMgr.ReleaseUnit(pkOldUnit);//이전 유닛 삭제.
	}

	g_kTotalObjMgr.RegistUnit(pkUnit);//파일럿이 쥐게되면 등록

	BM::CObserver< BM::Stream* > *pkObserver = dynamic_cast< BM::CObserver< BM::Stream* >* >(this);

	pkUnit->VAttach(pkObserver);

	m_pkUnit = pkUnit;

	PgIWorldObject* pkWorldObject = GetWorldObject();
	if(!pkWorldObject)
	{
		return;
	}

	std::wstring strPickupScript = _T("");
	std::wstring strMouseOverScript = _T("");
	std::wstring strMouseOutScript = _T("");
	if(pkUnit->IsUnitType(UT_NPC))
	{
		PgNpc* pkNpc = dynamic_cast<PgNpc*>(pkUnit);
		PG_ASSERT_LOG(pkNpc);

		if(pkNpc)
		{
			strPickupScript = _T("NPC_Common");
		}
	}
	else if(pkUnit->IsUnitType(UT_GROUNDBOX))
	{
		strPickupScript= _T("GroundItemBox_Pickup");
	}
	else if(pkUnit->IsUnitType(UT_PLAYER))
	{
		strPickupScript= _T("Actor_Pickup");
		//strMouseOverScript= _T("Actor_MouseOver");
		//strMouseOutScript= _T("Actor_MouseOut");
	}
	else if(pkUnit->IsUnitType(UT_PET))
	{
		if( g_kPilotMan.IsMyPlayer(pkUnit->Caller()) )
		{
			strPickupScript= _T("Pet_Pickup");
		}
	}
	else if(pkUnit->IsUnitType(UT_ENTITY))
	{
		strPickupScript= _T("Entity_Pickup");
	}
	else 
	{
		strPickupScript = _T("");
		strMouseOverScript = _T("");
		strMouseOutScript = _T("");
	}
	
	pkWorldObject->SetPickupScript(MB(strPickupScript));
	pkWorldObject->SetMouseOverScript(MB(strMouseOverScript));
	pkWorldObject->SetMouseOutScript(MB(strMouseOutScript));
}

extern void CallResponseBar();
void PgPilot::VUpdate( BM::CSubject< BM::Stream* > *const pChangedSubject, BM::Stream* pkNfy )
{
	WORD wType = 0;
	pkNfy->Pop(wType);

	switch(wType)
	{
	case PT_UNIT_NFY_ON_DIE:
		{//유닛 죽음.
			if( PlayerPilotGuid() == GetGuid() )
			{
//			lua_tinker::call<bool, void *>("OnDie", lwPilot(this)); --> 이걸로 하면 더 좋겠음.
				if (g_kMissionComplete.CompleteState() == PgMissionComplete::E_MSN_CPL_NONE
					|| g_kMissionComplete.CompleteState() == PgMissionComplete::E_MSN_CPL_SELECT_NEXT)
				{//미션 결과창에서는 죽음 UI 뜨지 않게 하기
					PgUtilAction_Revive kAction(ESMT_REVIVE_BY_CROSS, g_kPilotMan.GetPlayerPilot());
					if( false ==  kAction.DoAction(false) )
					{
						CallResponseBar();
					}
				}

				//죽었을때 사용할 수 없는 UI 닫기
				lua_tinker::call<void>("PilotDieCloseUI");
			}

			COMMUNITY_EVENT::OnPlayerDead();
		}break;
	}
}

bool PgPilot::IsHaveComboAction( int const iCurrentAction )
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef( iCurrentAction );
	int iSkillNo = iCurrentAction;
	if( pkSkillDef )
	{
		int iParentSkillNo = pkSkillDef->GetParentSkill();
		if( iParentSkillNo )
		{
			iSkillNo = iParentSkillNo;
		}
	}
	CONT_MAP_COMBO_ACTION_DATA::const_iterator iter_ActionData = m_kContComboActionData.begin();
	while( m_kContComboActionData.end() != iter_ActionData )
	{
		if( (*iter_ActionData).first == iSkillNo )
		{
			return true;
		}
		CONT_MAP_COMBO_ACTION_DATA::mapped_type const &kComboData = (*iter_ActionData).second;
		CONT_VEC_COMBO_KEY_DATA::const_iterator iter_ComboData = kComboData.begin();
		while( kComboData.end() != iter_ComboData )
		{//연계 가능한 스킬 확인
			if( (*iter_ComboData).iNextSkillNo == iSkillNo )
			{
				return true;
			}
			++iter_ComboData;
		}
		++iter_ActionData;
	}
	return false;
}

int PgPilot::GetNextComboAction( int const iCurrentAction, bool const bSearchChargeAction, int &iNextAction_SC )
{//
	int iNextSkillNo = 0;
	float fKeyTime = 0.0f;
	CONT_MAP_COMBO_ACTION_DATA::const_iterator iter_ActionData = m_kContComboActionData.find(iCurrentAction);
	if( m_kContComboActionData.end() != iter_ActionData )
	{//현재 액션에 적용되는 연계 스킬이 있으면
		CONT_MAP_COMBO_ACTION_DATA::mapped_type const &kComboData = (*iter_ActionData).second;
		CONT_VEC_COMBO_KEY_DATA::const_iterator iter_ComboData = kComboData.begin();
		while( kComboData.end() != iter_ComboData )
		{//연계 가능한 스킬 확인
			if( false == IsUseComboAction( (*iter_ComboData).iNextSkillNo ) )
			{
				++iter_ComboData;
				continue;
			}
			bool bCorrect = false;
			CONT_VEC_INPUT_KEY kData = (*iter_ComboData).kContKeyInfo;
			CONT_VEC_INPUT_KEY::iterator iter_Key = m_kContInputKey.begin();
			CONT_VEC_INPUT_KEY::iterator iter_Data = kData.begin();

			while( m_kContInputKey.end() != iter_Key
				&& kData.end() != iter_Data )
			{
				CONT_VEC_INPUT_KEY::value_type& const rkData = (*iter_Data);
				CONT_VEC_INPUT_KEY::value_type& const rkState = (*iter_Key);

				if( rkData.iUKeyNo != rkState.iUKeyNo )
				{//검색하려는 키번호가 같을 때만
					bCorrect = false;
					break;
				}
				if( bSearchChargeAction &&
					rkData.eKeyState != rkState.eKeyState )
				{//차지 결과 액션을 찾는 경우, 눌림 상태가 다르면 false
					bCorrect = false;
					break;
				}
				if( ACTIONKEY_ATTACK == rkState.iUKeyNo
					|| ACTIONKEY_USE == rkState.iUKeyNo
					|| ACTIONKEY_CHARGE == rkState.iUKeyNo )
				{
					if(rkData.eKeyState == EUKS_PRESS )
					{
						bCorrect = true;
					}
					if(	bSearchChargeAction && 
						rkData.eKeyState == EUKS_RELEASE &&
						rkState.fKeyTime >= rkData.fKeyTime)
					{//차징이고, 키타임이 맞다면 저장...
						if( fKeyTime <= rkData.fKeyTime )
						{//...하기 전에 기존에 저장된 fKeyTime보다 길 경우에만.
							fKeyTime = rkData.fKeyTime;
							bCorrect = true;
						}
					}
				}
				++iter_Key;
				++iter_Data;
			}
			if( bCorrect )
			{
				if( 0 < this->GetAbil(AT_DEL_SUB_PLAYER) &&			// 퓨전 상태인데다
					0 != (*iter_ComboData).iNextSkillNo_Fusion )
				{
					iNextSkillNo = (*iter_ComboData).iNextSkillNo_Fusion;
				}
				else
				{
					iNextSkillNo = (*iter_ComboData).iNextSkillNo;
				}
				iNextAction_SC = (*iter_ComboData).iNextSkillNo_SC;
			}
			++iter_ComboData;
		}
	}
	return iNextSkillNo;
}

bool PgPilot::IsHaveConnectComboAction(int const iWeaponType, int const iCurrentAction)
{
	CONT_MAP_COMBO_ACTION_DATA::const_iterator iter_ActionData = m_kContConnectComboActionData.begin();
	while( m_kContConnectComboActionData.end() != iter_ActionData )
	{
		CONT_MAP_COMBO_ACTION_DATA::mapped_type const &kComboData = (*iter_ActionData).second;
		CONT_VEC_COMBO_KEY_DATA::const_iterator iter_ComboData = kComboData.begin();
		while( kComboData.end() != iter_ComboData )
		{
			if( (*iter_ComboData).iNextSkillNo == iCurrentAction &&
				(*iter_ComboData).iWeaponType == iWeaponType ) 
			{
				return true;
			}
			++iter_ComboData;
		}
		++iter_ActionData;
	}
	return false;
}
int PgPilot::GetNextConnectComboAction(int const iWeaponType)
{
	PgActor	*pkPlayerActor = g_kPilotMan.GetPlayerActor();
	if(!pkPlayerActor) 
	{
		return 0;
	}
	bool	bActorLeft = false;
	NiPoint3	kLookingDir = pkPlayerActor->GetLookingDir();
	bActorLeft = ((pkPlayerActor->GetPathNormal().Cross(kLookingDir).z>0) ? true : false);

	int iNextSkillNo = 0;
	float fKeyTime = 0.0f;
	CONT_MAP_COMBO_ACTION_DATA::const_iterator iter_ActionData = m_kContConnectComboActionData.begin();
	while( m_kContConnectComboActionData.end() != iter_ActionData )
	{//현재 액션에 적용되는 연계 스킬이 있으면
		CONT_MAP_COMBO_ACTION_DATA::mapped_type const &kComboData = (*iter_ActionData).second;
		CONT_VEC_COMBO_KEY_DATA::const_iterator iter_ComboData = kComboData.begin();
		while( kComboData.end() != iter_ComboData )
		{//연계 가능한 스킬 확인
			if( (*iter_ComboData).iWeaponType != iWeaponType)
			{
				++iter_ComboData;
				continue;
			}
			bool bCorrect = false;
			CONT_VEC_INPUT_KEY kData = (*iter_ComboData).kContKeyInfo;
			CONT_VEC_INPUT_KEY::iterator iter_Key = m_kContInputKey.begin();
			CONT_VEC_INPUT_KEY::iterator iter_Data = kData.begin();

			while( m_kContInputKey.end() != iter_Key
				&& kData.end() != iter_Data )
			{
				CONT_VEC_INPUT_KEY::value_type& const rkData = (*iter_Data);
				CONT_VEC_INPUT_KEY::value_type& const rkState = (*iter_Key);

				if( bActorLeft )
				{//보는 방향에 따라 입력키 방향 수정
					int iDirKey = rkState.iUKeyNo;
					if( rkState.iUKeyNo == ACTIONKEY_LEFT )
					{
						iDirKey = ACTIONKEY_RIGHT;
					}
					else if( rkState.iUKeyNo == ACTIONKEY_RIGHT )
					{
						iDirKey = ACTIONKEY_LEFT;
					}

					if( rkData.iUKeyNo != iDirKey )
					{//검색하려는 키번호가 같을 때만
						bCorrect = false;
						break;
					}
				}
				else
				{
					if( rkData.iUKeyNo != rkState.iUKeyNo )
					{//검색하려는 키번호가 같을 때만
						bCorrect = false;
						break;
					}
				}

				if( ACTIONKEY_ATTACK == rkState.iUKeyNo
					|| ACTIONKEY_USE == rkState.iUKeyNo
					|| ACTIONKEY_CHARGE == rkState.iUKeyNo )
				{
					if(rkData.eKeyState == EUKS_PRESS )
					{
						bCorrect = true;
					}
				}
				++iter_Key;
				++iter_Data;
			}
			if( bCorrect )
			{
				return (*iter_ComboData).iNextSkillNo;
			}
			++iter_ComboData;
		}
		++iter_ActionData;
	}
	return 0;
}

bool PgPilot::GetNextComboList(int const iCurrentActionNO, std::list<int>& rkContNextCombo )
{
	rkContNextCombo.clear();
	CONT_MAP_COMBO_ACTION_DATA::const_iterator iter_ActionData = m_kContComboActionData.find(iCurrentActionNO);
	if( m_kContComboActionData.end() != iter_ActionData )
	{//현재 액션에 적용되는 연계 스킬이 있으면
		CONT_MAP_COMBO_ACTION_DATA::mapped_type const &kComboData = (*iter_ActionData).second;
		CONT_VEC_COMBO_KEY_DATA::const_iterator iter_ComboData = kComboData.begin();
		while( kComboData.end() != iter_ComboData )
		{
			rkContNextCombo.push_back( (*iter_ComboData).iNextSkillNo );
			++iter_ComboData;
		}
	}

	return rkContNextCombo.size();
}
bool PgPilot::IsUseComboAction( int const iActionNo )
{
	CONT_DEFSKILL const *pkDefSkill = NULL;
	g_kTblDataMgr.GetContDef( pkDefSkill );
	if( pkDefSkill )
	{
		CONT_DEFSKILL::const_iterator skill_itor = pkDefSkill->find( iActionNo );
		if( pkDefSkill->end() != skill_itor )
		{
			int const iLevel = this->GetAbil(AT_LEVEL);
			__int64 const i64Class = this->GetAbil(AT_CLASS);
			if( iLevel >= static_cast<int>((*skill_itor).second.sLevelLimit) )
			{
				TBL_DEF_SKILL kSkill = (*skill_itor).second;
				if( 0 == kSkill.i64ClassLimit &&
					0 == kSkill.i64DraClassLimit )
				{
					return true;
				}
				__int64 iUnionClassLimit = (kSkill.i64DraClassLimit << DRAGONIAN_LSHIFT_VAL) | kSkill.i64ClassLimit;
				if( IS_CLASS_LIMIT( iUnionClassLimit, i64Class ) )
				{
					return true;
				}
			}
		}
	}
	return false;
}
bool PgPilot::IsChargeCombo(int const iActionNo)
{
	CONT_MAP_COMBO_ACTION_DATA::const_iterator iter_ActionData = m_kContComboActionData.find(iActionNo);
	if( m_kContComboActionData.end() != iter_ActionData )
	{//현재 액션에 적용되는 연계 스킬이 있으면
		CONT_MAP_COMBO_ACTION_DATA::mapped_type const &kComboData = (*iter_ActionData).second;
		CONT_VEC_COMBO_KEY_DATA::const_iterator iter_ComboData = kComboData.begin();
		while( kComboData.end() != iter_ComboData )
		{
			CONT_VEC_INPUT_KEY::const_iterator itor_key = (*iter_ComboData).kContKeyInfo.begin();
			while( (*iter_ComboData).kContKeyInfo.end() != itor_key )
			{
				if( 0 != (*itor_key).fKeyTime )
				{
					return true;
				}
				++itor_key;
			}
			++iter_ComboData;
		}
	}
	return false;
}
bool PgPilot::GetChargeResultAction(int const iActionNo, std::list<int> &rkContAction)
{
	if( true == IsChargeCombo(iActionNo) )
	{
		return GetNextComboList(iActionNo, rkContAction);
	}
	return false;
}
bool PgPilot::IsInputAttackKey()
{
	CONT_VEC_INPUT_KEY::iterator iter_list = m_kContInputKey.begin();
	while( m_kContInputKey.end() != iter_list )
	{
		if( ACTIONKEY_ATTACK == (*iter_list).iUKeyNo 
			|| ACTIONKEY_USE == (*iter_list).iUKeyNo
			|| ACTIONKEY_CHARGE == (*iter_list).iUKeyNo )
		{
			return true;
		}
		++iter_list;
	}
	return false;
}
int PgPilot::GetInputAttackKey()
{
	CONT_VEC_INPUT_KEY::iterator iter_list = m_kContInputKey.begin();
	while( m_kContInputKey.end() != iter_list )
	{
		if( ACTIONKEY_ATTACK == (*iter_list).iUKeyNo 
			|| ACTIONKEY_USE == (*iter_list).iUKeyNo
			|| ACTIONKEY_CHARGE == (*iter_list).iUKeyNo )
		{
			return (*iter_list).iUKeyNo;
		}
		++iter_list;
	}
	return 0;
}
void PgPilot::InsertInputKey(int const iUKey, float const fStartTime, bool const bPush)
{
	if( ACTIONKEY_ATTACK == iUKey 
		|| ACTIONKEY_USE == iUKey
		|| ACTIONKEY_CHARGE == iUKey )
	{//X,Z키의 경우에만 중복을 허용하지 않는다.
		CONT_VEC_INPUT_KEY::iterator iter_list = m_kContInputKey.begin();
		while( m_kContInputKey.end() != iter_list )
		{
			if( (*iter_list).iUKeyNo == iUKey )
			{
				if( EUKS_RELEASE == (*iter_list).eKeyState )
				{
					(*iter_list).eKeyState = (bPush ? EUKS_PRESS : EUKS_RELEASE);
					(*iter_list).fKeyTime = fStartTime;
					return;
				}
				else if( EUKS_PRESS == (*iter_list).eKeyState )
				{
					(*iter_list).eKeyState = (bPush ? EUKS_PRESS : EUKS_RELEASE);
					(*iter_list).fKeyTime = fStartTime - (*iter_list).fKeyTime;
					return;
				}
			}
			++iter_list;
		}
	}
	//기존 리스트에 없으면 누른 상태일 때만 (키상태 :눌렀음, 입력시간 : 누른 타임) 상태를 추가
	if( bPush )
	{
		tagKeyInfo kInputKey;
		kInputKey.iUKeyNo = iUKey;
		kInputKey.eKeyState = EUKS_PRESS;
		kInputKey.fKeyTime = fStartTime;
		m_kContInputKey.push_back( kInputKey );
	}
}
void PgPilot::ClearInputKey()
{
	m_kContInputKey.clear();
}
void PgPilot::RefreshInputKey()
{
	if( NULL == g_pkWorld )
	{
		PG_ASSERT_LOG(g_pkWorld);
		return;
	}
	NiInputKeyboard	*pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
	if( !pkKeyboard)
	{
		return;
	}
	CONT_VEC_INPUT_KEY::iterator iter_list = m_kContInputKey.begin();
	while( m_kContInputKey.end() != iter_list )
	{
		NiInputKeyboard::KeyCode kKeycode = static_cast<NiInputKeyboard::KeyCode>(g_kGlobalOption.GetUKeyToKey((*iter_list).iUKeyNo ));
		bool const bKeyDown = pkKeyboard->KeyIsDown( kKeycode );
		if( bKeyDown && EUKS_PRESS == (*iter_list).eKeyState )
		{//현재 키가 눌려져있고, 저장된 상태로 눌려진 상태라면 새로고침할 때 현재 키입력 시간으로 스타트 시각을 초기화 한다.
			(*iter_list).fKeyTime = g_pkWorld->GetAccumTime();
		}
		else if( !bKeyDown && EUKS_PRESS == (*iter_list).eKeyState )
		{//현재 키가 떼어져있고, 저장된 상태가 눌려져있는 상태라면 저장된 키상태를 뗀 상태로 수정하고, 키입력 시간을 지금까지 눌린 시간으로 저장한다.
			 (*iter_list).eKeyState = EUKS_RELEASE;
			 (*iter_list).fKeyTime = 0.0f; //g_pkWorld->GetAccumTime() - (*iter_list).fKeyTime;
		}
		++iter_list;
	}
}
void PgPilot::SetInputKeyState_Release()
{//키입력 종료가 들어왔을 때, 강제로 모든 키를 Release 상태로 변경하여, 최대 차지 시간을 체크하기 위해 함수 제작
	if( NULL == g_pkWorld )
	{
		PG_ASSERT_LOG(g_pkWorld);
		return;
	}
	CONT_VEC_INPUT_KEY::iterator iter_list = m_kContInputKey.begin();
	while( m_kContInputKey.end() != iter_list )
	{
		if( EUKS_PRESS == (*iter_list).eKeyState )
		{
			(*iter_list).eKeyState = EUKS_RELEASE;
			(*iter_list).fKeyTime = g_pkWorld->GetAccumTime() - (*iter_list).fKeyTime;
			return;
		}
		++iter_list;
	}
}
bool PgPilot::HaveInputKeyState_Release()
{	
	if( 0 == m_kContInputKey.size() )
	{//입력된 키값이 없어도 true 리턴.(키 다 땐거니까....)
		return true;
	}
	CONT_VEC_INPUT_KEY::iterator iter_list = m_kContInputKey.begin();
	while( m_kContInputKey.end() != iter_list )
	{
		if( ACTIONKEY_ATTACK == (*iter_list).iUKeyNo 
			|| ACTIONKEY_USE == (*iter_list).iUKeyNo 
			|| ACTIONKEY_CHARGE == (*iter_list).iUKeyNo )
		{//X,Z키일때만
			if( EUKS_RELEASE == (*iter_list).eKeyState )
			{
				return true;
			}
		}
		++iter_list;
	}
	return false;
}

bool PgPilot::FindInputKey(int const iUiKey)
{
	if( 0 == m_kContInputKey.size() )
	{
		return false;
	}
	CONT_VEC_INPUT_KEY::iterator iter_list = m_kContInputKey.begin();
	while( m_kContInputKey.end() != iter_list )
	{
		if( iUiKey == (*iter_list).iUKeyNo )
		{
			return true;
		}
		++iter_list;
	}
	return false;
}
//----------------------------------------------------------------------------
// PgInputSlotInfo
//----------------------------------------------------------------------------
PgInputSlotInfo::PgInputSlotInfo(char const* pcActionID, unsigned int uiKey, 
								 void* pkUserData, bool bRecord,bool bEnable, int const iTargetType, int const iWeaponType) :
	m_kActionID(pcActionID)
	,m_uiKey(uiKey)
	,m_pkUserData(pkUserData)
	,m_bRecord(bRecord)
	,m_bEnable(bEnable)
	,m_iTargetType(iTargetType)
	,m_iWeaponType(iWeaponType)
{
}

std::string const& PgInputSlotInfo::GetActionID()const
{
	return m_kActionID;
}

void PgInputSlotInfo::SetActionID(std::string const& rkActionID)
{
	m_kActionID = rkActionID;
}

unsigned int PgInputSlotInfo::GetUKey() const
{
	return m_uiKey;
}

void* PgInputSlotInfo::GetUserData() const
{
	return m_pkUserData;
}

bool PgInputSlotInfo::IsRecord() const
{
	return m_bRecord;
}

bool PgInputSlotInfo::operator ==(const PgInputSlotInfo& rhs)const
{
	// uiKey만 같으면 같다고 하면 된다.
	// 추가 : WeaponType이 0이거나, 같아야 한다
	if( m_uiKey == rhs.m_uiKey
		&& 0 == rhs.m_iWeaponType )
	{
		return true;
	}
	else if(m_uiKey == rhs.m_uiKey
			&& m_iWeaponType == rhs.m_iWeaponType )
	{
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------
// PgRemoteInput
//-------------------------------------------------------------------------------
PgRemoteInput::PgRemoteInput(SActionInfo& rkActionInfo, 
			   PgActionTargetList& rkTargetList, 			   
			   DWORD dwSyncTime) :
	m_kActionInfo(rkActionInfo),
	m_kTargetList(rkTargetList),
	m_dwSyncTime(dwSyncTime)
{
}
