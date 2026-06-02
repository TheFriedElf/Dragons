
#include "StdAfx.h"
#include "lwPilot.h"
#include "lwDropBox.h"
#include "PgPilot.h"
#include "PgActor.h"
#include "PgDropBox.h"
#include "Variant/PgPlayer.h"
#include "Variant/PgMonster.h"
#include "ServerLib.h"
#include "PgSkillTree.H"
#include "PgPilotMan.H"
#include "Variant/PgClassDefMgr.h"
#include "PgObject.H"
#include "lwObject.H"
#include "PgMobileSuit.h"

lwPilot::lwPilot(PgPilot *pkPilot)
{
	m_pkPilot = pkPilot;
}

bool lwPilot::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwPilot>(pkState, "Pilot")
		.def(pkState, constructor<PgPilot *>())
		.def(pkState, "IsNil", &lwPilot::IsNil)
		.def(pkState, "GetGuid", &lwPilot::GetGuid)
		.def(pkState, "GetActor", &lwPilot::GetActor)
		.def(pkState, "GetObject", &lwPilot::GetObject)
		.def(pkState, "GetPos", &lwPilot::GetPos)
		.def(pkState, "SetAbil", &lwPilot::SetAbil)
		.def(pkState, "GetAbil", &lwPilot::GetAbil)
		.def(pkState, "SetHP", &lwPilot::SetHP)
		.def(pkState, "GetDropBox", &lwPilot::GetDropBox)
		.def(pkState, "SetDropBox", &lwPilot::SetDropBox)
		.def(pkState, "SetInfo", &lwPilot::SetInfo)
		.def(pkState, "GetName", &lwPilot::GetName)
		.def(pkState, "SetName", &lwPilot::SetName)
		.def(pkState, "GetAbil64", &lwPilot::GetAbil64)
		.def(pkState, "SetAbil64", &lwPilot::SetAbil64)
		.def(pkState, "SetMyPet", &lwPilot::SetMyPet)
		.def(pkState, "GetMyPet", &lwPilot::GetMyPet)
		.def(pkState, "ChangePetName", &lwPilot::ChangePetName)
		.def(pkState, "SetMonsterNo", &lwPilot::SetMonsterNo)
		.def(pkState, "GetMonsterNo", &lwPilot::GetMonsterNo)
		.def(pkState, "GetUpgradeCount", &lwPilot::GetUpgradeCount)
		.def(pkState, "GetUpgradeClass", &lwPilot::GetUpgradeClass)
		.def(pkState, "ReanalyseSkillTree", &lwPilot::ReanalyseSkillTree)
		.def(pkState, "IsPlayerPilot", &lwPilot::IsPlayerPilot)
		.def(pkState, "IsMyPlayerPilot", &lwPilot::IsMyPlayerPilot)
		.def(pkState, "GetState", &lwPilot::GetState)
		.def(pkState, "ChangeClass", &lwPilot::ChangeClass)
		.def(pkState, "SetGoalPos", &lwPilot::SetGoalPos)
		.def(pkState, "UnitType", &lwPilot::UnitType)
		.def(pkState, "GetUnit", &lwPilot::GetUnit)
		.def(pkState, "GetBaseClassID", &lwPilot::GetBaseClassID)
		.def(pkState, "IsCorrectClass", &lwPilot::IsCorrectClass)
		.def(pkState, "IsUnitType", &lwPilot::IsUnitType)
		.def(pkState, "SetUnit", &lwPilot::SetUnit)
		.def(pkState, "GetFrontDirection", &lwPilot::GetFrontDirection)
		.def(pkState, "IsAlive", &lwPilot::IsAlive)
		.def(pkState, "GetCoupleGuid", &lwPilot::GetCoupleGuid)
		.def(pkState, "GetTarget", &lwPilot::GetTarget)
		.def(pkState, "IsHaveComboAction", &lwPilot::IsHaveComboAction)
		;

	return true;
}	
void	lwPilot::ReanalyseSkillTree()
{
	PG_ASSERT_LOG(m_pkPilot);
	if ( m_pkPilot )
	{
		PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(m_pkPilot->GetUnit());
		if ( pkPlayer )
		{
			g_kSkillTree.CreateSkillTree( pkPlayer );
		}
	}
}

bool	lwPilot::IsPlayerPilot()
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot || !m_pkPilot->GetUnit())
	{
		return false;
	}

	return m_pkPilot->GetUnit()->IsUnitType(UT_PLAYER);
}

bool	lwPilot::IsMyPlayerPilot()	//	나의 플레이어인가.
{
	PG_ASSERT_LOG(m_pkPilot);
	if (m_pkPilot != NULL && g_kPilotMan.GetPlayerPilot() == m_pkPilot) 
	{
		return true;
	}

	return false;
}

int	lwPilot::GetState()
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return 0;}

	if(m_pkPilot->GetUnit())
	{
		return	m_pkPilot->GetUnit()->GetState();
	}
	return	0;
}
PgPilot* lwPilot::operator()()
{
	return	m_pkPilot;
}
bool lwPilot::IsNil()
{
	return (m_pkPilot == 0);
}
void lwPilot::SetUnit(lwGUID kGUID,int iUnitType,int iClassNo,int iLevel,int iGender)
{
	PG_ASSERT_LOG(m_pkPilot);
	m_pkPilot->SetUnit(kGUID(),iUnitType,iClassNo,iLevel,iGender);
}
int	lwPilot::GetBaseClassID()
{
	PG_ASSERT_LOG(m_pkPilot);
	return	m_pkPilot->GetBaseClassID();
}
bool lwPilot::IsCorrectClass(int const iReqClassID,bool bNotIncludeSelf)	//	이 캐릭터가 iReqClassID 에 해당하는 캐릭터가 맞는지 체크한다.
{
	PG_ASSERT_LOG(m_pkPilot);
	return	m_pkPilot->IsCorrectClass(iReqClassID,bNotIncludeSelf);
}

lwGUID lwPilot::GetGuid()
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return lwGUID(BM::GUID::NullData());}

	return lwGUID(m_pkPilot->GetGuid());
}
void	lwPilot::SetName(lwWString kName)
{
	PG_ASSERT_LOG(m_pkPilot);
	if (m_pkPilot == NULL)
	{
		return;
	}

	return	m_pkPilot->SetName(kName());

}
lwWString lwPilot::GetName()
{
	PG_ASSERT_LOG(m_pkPilot);
	if (m_pkPilot == NULL)
	{
		return lwWString("");
	}

	return	lwWString(m_pkPilot->GetName().c_str(),m_pkPilot->GetName().length());
}
lwObject lwPilot::GetObject()
{
	PG_ASSERT_LOG(m_pkPilot);
	if (m_pkPilot == NULL)
	{
		return lwObject(NULL); //! 별로 좋은 건 아닌거 같다.
	}

	return lwObject(dynamic_cast<PgObject *>(m_pkPilot->GetWorldObject()));
}

lwActor lwPilot::GetActor()
{
	PG_ASSERT_LOG(m_pkPilot);
	if (m_pkPilot == NULL)
	{
		return lwActor(NULL); //! 별로 좋은 건 아닌거 같다.
	}

	return lwActor(dynamic_cast<PgActor *>(m_pkPilot->GetWorldObject()));
}

lwPoint3 lwPilot::GetPos()
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return lwPoint3(0,0,0);}

	POINT3 const &pt3Pos = m_pkPilot->GetUnit()->GetPos();
	return lwPoint3(pt3Pos.x, pt3Pos.y, pt3Pos.z);
}

void lwPilot::SetHP(int Hp,int iTimeStamp)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return;}
	DWORD	dwTimeStamp = (DWORD)iTimeStamp;
	if(dwTimeStamp ==0 || m_pkPilot->GetLastHPSetTimeStamp() < dwTimeStamp )
	{
		if(dwTimeStamp>0)
			m_pkPilot->SetLastHPSetTimeStamp(dwTimeStamp);

		m_pkPilot->GetUnit()->SetAbil(AT_HP, Hp);
	}
}

void lwPilot::SetAbil(int const iAbilType, int const iValue)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return;}

	CUnit *pkUnit = m_pkPilot->GetUnit();
	if(pkUnit)
	{
#ifdef EXTERNAL_RELEASE
		if( false == g_pkApp->IsSingleMode() )
		{
			PgActor* pkActor = g_kPilotMan.FindActor( pkUnit->GetID() );
			if( pkActor
				&& ( pkActor->IsMyActor() || pkActor->IsMyPet() )
				)
			{
				MakeCrash_SetAbilFromLuaScript(iAbilType);
			}
		}
#endif
		pkUnit->SetAbil(iAbilType, iValue);
		if (iAbilType == AT_CLASS || iAbilType == AT_LEVEL)
		{
			pkUnit->NftChangedAbil(AT_EVENT_LEVELUP, E_SENDTYPE_NONE);
			if (pkUnit->IsUnitType(UT_PLAYER))
			{
				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
				pkPlayer->CalculateInitAbil();
			}
		}
	}
}
int	lwPilot::GetAbil(int iAbilType)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return -1;}

	CUnit *pkUnit = m_pkPilot->GetUnit();
	if(!pkUnit)
	{
		return -1;
	}
	return pkUnit->GetAbil(iAbilType);
}

lwDropBox lwPilot::GetDropBox()
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return lwDropBox(NULL);}

	return lwDropBox(dynamic_cast<PgDropBox *>(m_pkPilot->GetWorldObject()));
}

void lwPilot::SetDropBox(lwDropBox kDropBox)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(m_pkPilot)
	{
		m_pkPilot->SetWorldObject(kDropBox());
	}
}

void lwPilot::SetInfo(lwGUID kMemberGuid, lwWString pcName, BYTE byGender, BYTE byClass)
{
	__asm int 3;// 일부러 이렇게 했음. 호출 안되는게 맞음.
	PG_ASSERT_LOG(m_pkPilot);
	if(m_pkPilot)
	{
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(m_pkPilot->GetUnit());

		if(pkPlayer)
		{
//			pkPlayer->SetMemberGUID(kMemberGuid.GetGUID());
			pkPlayer->Name(pcName().c_str());
			pkPlayer->SetAbil(AT_GENDER, byGender);
			pkPlayer->SetAbil(AT_CLASS, byClass);
		}
	}
}

void lwPilot::SetMyPet(lwGUID kPetGuid)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return;}

	PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(m_pkPilot->GetUnit());
	if(!pkPlayer)
	{
		return;
	}

//	pkPlayer->PetGuid(kPetGuid());
}

lwGUID lwPilot::GetMyPet()
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot)
	{
		return lwGUID(BM::GUID());
	}

//	PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(m_pkPilot->GetUnit());
//	if(!pkPlayer)
//	{
		return lwGUID(BM::GUID());
//	}

//	return lwGUID(pkPlayer->PetGuid());
}

void lwPilot::ChangePetName(char const *pcName)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return;}

	CUnit *pkUnit = m_pkPilot->GetUnit();
	if(pkUnit)
	{
		pkUnit->Name(UNI(pcName));
	}
	assert(NULL);
}

void lwPilot::SetAbil64(int AbilType, lwInt64 kInt64)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return;}

	CUnit *pkUnit = m_pkPilot->GetUnit();
	if(pkUnit)
	{
		pkUnit->SetAbil64((EAbilType)AbilType, kInt64.GetValue());
	}
}

lwInt64 lwPilot::GetAbil64(int iAbilType)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return -1;}

	CUnit *pkUnit = m_pkPilot->GetUnit();
	if(!pkUnit)
	{
		return lwInt64(-1);
	}
	return lwInt64(pkUnit->GetAbil64((EAbilType)iAbilType));	
}


void lwPilot::SetMonsterNo(int iMonNo)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return;}

	CUnit *pkUnit = m_pkPilot->GetUnit();
	if(!pkUnit){return ;}

	PgMonster *pkMon = dynamic_cast<PgMonster*>(pkUnit);
	if(pkMon)
	{
		pkMon->SetAbil(AT_CLASS, iMonNo);
	}
}

int	lwPilot::GetUpgradeCount()	//	전직 횟수 리턴
{
	PG_ASSERT_LOG(m_pkPilot);
	CUnit *pkUnit = m_pkPilot->GetUnit();
	if(!pkUnit)
	{
		return 0;
	}

	int const iClassID = pkUnit->GetAbil(AT_CLASS);
	return	GetClassUpgradeOrder(iClassID,NULL);
}
int	lwPilot::GetUpgradeClass(int iGrade)	//	iGrade 에 해당하는 클래스 번호 리턴
{
	PG_ASSERT_LOG(m_pkPilot);
	CUnit *pkUnit = m_pkPilot->GetUnit();
	if(!pkUnit)
	{
		return 0;
	}
	int	iClassID = pkUnit->GetAbil(AT_CLASS);
	int	iClass[10];
	GetClassUpgradeOrder(iClassID,iClass);
	return	iClass[iGrade];
}

int lwPilot::GetMonsterNo()
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return 0;}

	CUnit *pkUnit = m_pkPilot->GetUnit();
	if(!pkUnit)
	{
		return 0;
	}

	if( pkUnit->IsUnitType(UT_MONSTER))
	{//몬스터만
		return pkUnit->GetAbil(AT_CLASS);
	}
	assert(NULL);
	return 0;
}

void lwPilot::ChangeClass(BYTE byClass, short int sLevel)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return;}

	CUnit* pkUnit = m_pkPilot->GetUnit();
	if( pkUnit->IsUnitType(UT_PLAYER) == false )
	{
		return;
	}

//	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
//	if( pkPlayer )
//	{
//		pkPlayer->(byClass);
//	}
}

void lwPilot::SetGoalPos(lwPoint3 kPos)
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return;}

	CUnit* pkUnit = m_pkPilot->GetUnit();
	if ( !pkUnit )
	{
		return ;
	}

	if ( pkUnit->IsUnitType(UT_MONSTER) == false  )
	{
		return ;
	}

	PgMonster *pkMonster = dynamic_cast<PgMonster*>(pkUnit);
	POINT3 ptPos(kPos.GetX(),kPos.GetY(),kPos.GetZ());
	pkMonster->GoalPos(ptPos);
}

int lwPilot::UnitType()
{
	PG_ASSERT_LOG(m_pkPilot);
	if (m_pkPilot == NULL)
	{
		return UT_NONETYPE;
	}
	CUnit* pkUnit = m_pkPilot->GetUnit();
	return pkUnit->UnitType();
}

lwUnit lwPilot::GetUnit()
{
	PG_ASSERT_LOG(m_pkPilot);

	
	if(!m_pkPilot){ return lwUnit(NULL);}

	return lwUnit(m_pkPilot->GetUnit());
}

bool lwPilot::IsUnitType(int iType)
{
	PG_ASSERT_LOG(m_pkPilot);

	if(!m_pkPilot){return false;}
	return m_pkPilot->GetUnit()->IsUnitType((EUnitType)iType);
}

int lwPilot::GetFrontDirection()
{
	PG_ASSERT_LOG(m_pkPilot);
	if(!m_pkPilot){return 0;}

	if( !m_pkPilot )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkPilot is NULL"), __FUNCTIONW__);
		return 0;
	}
	PgActor* pkActor = dynamic_cast<PgActor *>(m_pkPilot->GetWorldObject());
	if( !pkActor )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] Actor is NULL"), __FUNCTIONW__);
		return 0;
	}
	return pkActor->GetDirection();
}

bool lwPilot::IsAlive()
{
	PG_ASSERT_LOG(m_pkPilot);
	if( !m_pkPilot )
	{
	//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] m_pkPilot is NULL"), __FUNCTIONW__);
		return false;
	}

	if(m_pkPilot->GetUnit())
	{
		return m_pkPilot->GetUnit()->IsAlive();
	}
	return false;
}

lwGUID lwPilot::GetCoupleGuid()
{
	PG_ASSERT_LOG(m_pkPilot);
	if( !m_pkPilot || !m_pkPilot->GetUnit() )
	{
		return lwGUID(BM::GUID());
	}

	return lwGUID(m_pkPilot->GetUnit()->GetCoupleGuid());
}

lwGUID lwPilot::GetTarget() const
{
	if( !m_pkPilot || !m_pkPilot->GetUnit() )
	{
		return lwGUID(BM::GUID());
	}

	return lwGUID(m_pkPilot->GetUnit()->GetTarget());
}

bool lwPilot::IsHaveComboAction( int const iCurrentAction )
{
	return m_pkPilot->IsHaveComboAction(iCurrentAction);
}
int lwPilot::GetNextComboAction( int const iCurrentAction, bool const bSearchChargeAction, int &iNextAction_SC )
{
	return m_pkPilot->GetNextComboAction( iCurrentAction, bSearchChargeAction, iNextAction_SC);
}
bool lwPilot::IsHaveConnectComboAction(int const iWeaponType, int const iCurrentAction)
{
	return m_pkPilot->IsHaveConnectComboAction(iWeaponType, iCurrentAction);
}
int lwPilot::GetNextConnectComboAction(int const iWeaponType)
{
	return m_pkPilot->GetNextConnectComboAction(iWeaponType);
}
bool lwPilot::GetNextComboList( int const iCurrentActionNO, std::list<int>& rkContNextCombo )
{
	return m_pkPilot->GetNextComboList(iCurrentActionNO, rkContNextCombo );
}
bool lwPilot::IsUseComboAction( int const iActionNo )
{
	return m_pkPilot->IsUseComboAction( iActionNo );
}
bool lwPilot::IsChargeCombo( int const iActionNo )
{
	return m_pkPilot->IsChargeCombo( iActionNo );
}
void lwPilot::InsertInputKey(int const iUKey, float const fStartTime, bool const bPush)
{
	m_pkPilot->InsertInputKey( iUKey, fStartTime, bPush );
}
void lwPilot::ClearInputKey()
{
	m_pkPilot->ClearInputKey();
}
void lwPilot::RefreshInputKey()
{
	m_pkPilot->RefreshInputKey();
}
void lwPilot::SetInputKeyState_Release()
{
	m_pkPilot->SetInputKeyState_Release();
}
bool lwPilot::HaveInputKeyState_Release()
{
	return m_pkPilot->HaveInputKeyState_Release();
}

bool lwPilot::IsInputAttackKey()
{
	return m_pkPilot->IsInputAttackKey();
}

int lwPilot::GetInputAttackKey()
{
	return m_pkPilot->GetInputAttackKey();
}

bool lwPilot::FindInputKey(int const iUiKey)
{
	return m_pkPilot->FindInputKey(iUiKey);
}

char const* lwPilot::FindActionID(unsigned int uiUKey)
{
	return m_pkPilot->FindActionID(uiUKey);
}

bool MakeCrash_SetAbilFromLuaScript(int const iAbilType)
{
	switch(iAbilType)
	{
	case AT_MOVESPEED:
	case AT_R_MOVESPEED:
	case AT_C_MOVESPEED:
	case AT_MOVESPEED_SAVED:

	case AT_ATTACK_SPEED:
	case AT_R_ATTACK_SPEED:
	case AT_C_ATTACK_SPEED:

	case AT_WALK_SPEED:
	case AT_R_WALK_SPEED:
	case AT_C_WALK_SPEED:

	case AT_CASTING_SPEED:
	case AT_R_CASTING_SPEED:
	case AT_C_CASTING_SPEED:

	case AT_VILLAGE_MOVESPEED:
	case AT_R_VILLAGE_MOVESPEED:
	case AT_C_VILLAGE_MOVESPEED:
	case AT_VILLAGE_MOVESPEED_SAVED:
	
	case AT_TEAM:
	case AT_DUEL:
	case AT_DAMAGEACTION_TYPE:
	////////////////////////////////////////////////////////////////////// 이하는 set해도 상관없지만 시도해도 크래시
	case AT_BLOCK_SUCCESS_VALUE:
	case AT_R_BLOCK_SUCCESS_VALUE:
	case AT_C_BLOCK_SUCCESS_VALUE:

	case AT_DODGE_SUCCESS_VALUE:
	case AT_R_DODGE_SUCCESS_VALUE:
	case AT_C_DODGE_SUCCESS_VALUE:

	case AT_CRITICAL_SUCCESS_VALUE:
	case AT_R_CRITICAL_SUCCESS_VALUE:
	case AT_C_CRITICAL_SUCCESS_VALUE:

	case AT_CRITICAL_POWER:
	case AT_R_CRITICAL_POWER:
	case AT_C_CRITICAL_POWER:

	case AT_CANNOT_DAMAGE:
		{
			__asm int 3;
			return false;
		}break;
	}
	return true;
}