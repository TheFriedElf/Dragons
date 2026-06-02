#include "stdafx.h"
#include "Variant/PgControlDefMgr.h"
#include "PgDamageNumMan.H"
#include "PgChatMgrClient.h"
#include "PgActionTargetList.H"
#include "lwActionResult.H"
#include "PgActor.h"
#include "PgPilot.H"
#include "PgPilotMan.H"
#include "PgProjectile.H"
#include "PgProjectileMan.H"
#include "PgWorld.H"
#include "lwActor.H"
#include "lwPilot.h"
#include "PgMath.h"
#include "PgAction.h"
#include "PgContentsBase.h"
#include "PgOption.h"
#include "lwUI.h"
#include "lwUIMission.h"

PgActionTargetTransferInfoMan	g_kActionTargetTransferInfoMan;

char const* const ATTACH_DAMAGE_NUM_NODE = "p_ef_dmgcount01";

//>>local function
void AddDamageLog(const PgPilot *pkCaster, const PgPilot *pkTarget, int iDamage, PgActionResult *pkActionResult = NULL)
{
	if(NULL == pkCaster) { return; }
	if(NULL == pkTarget) { return; }

	bool bCasterMe = g_kPilotMan.IsMyPlayer(pkCaster->GetGuid());
	bool bTargetMe = g_kPilotMan.IsMyPlayer(pkTarget->GetGuid());

	if(!bCasterMe)
	{
		bCasterMe = g_kPilotMan.IsMyPlayer(pkCaster->GetUnit()->Caller());
	}

	if(!bCasterMe && !bTargetMe) { return; }//내 메시지가 아니라면 출력하지 않는다.

	bool bCritical = false, bMissed = false, bDodged = false;
	PgActionResult::eBlockedType eBlockEype = PgActionResult::EBT_NONE;
	if( pkActionResult )
	{
		bCritical = pkActionResult->GetCritical();
		bMissed = pkActionResult->GetMissed();
		bDodged = pkActionResult->GetDodged();
		eBlockEype = static_cast<PgActionResult::eBlockedType>( pkActionResult->GetBlocked() );
	}

	int iTTW = 0;
	bool bDamaged = false;
	if( bTargetMe && !bCritical && !bMissed && !bDodged && (PgActionResult::EBT_NONE == eBlockEype || PgActionResult::EBT_REACTION == eBlockEype) )	{iTTW = 700003; bDamaged = true;}	//Other -> Me
	if( bTargetMe && bCritical && !bMissed && !bDodged && (PgActionResult::EBT_NONE == eBlockEype || PgActionResult::EBT_REACTION == eBlockEype ) )	{iTTW = 700004; bDamaged = true;}	//Other -> Me Cri
	if( !bTargetMe && !bCritical && !bMissed && !bDodged && (PgActionResult::EBT_NONE == eBlockEype || PgActionResult::EBT_REACTION == eBlockEype) )	{iTTW = 700005; bDamaged = true;}	//Me -> Other
	if( !bTargetMe && bCritical && !bMissed && !bDodged && (PgActionResult::EBT_NONE == eBlockEype || PgActionResult::EBT_REACTION == eBlockEype) )	{iTTW = 700006; bDamaged = true;}	//Me -> Other Cri
	if( bDamaged &&	0 == iDamage ) //데미지 로그인데 데미지가 0 이면 아무것도 없다
	{
		return;
	}
	if( !bTargetMe && bMissed )		{iTTW = 700107; bDamaged = false;}		//Me -> Other Missed
	if( bTargetMe && bMissed )		{iTTW = 700108; bDamaged = false;}		//Other -> Me Missed
	if( !bTargetMe)
	{
		if( PgActionResult::EBT_NONE_REACTION == eBlockEype )
		{//Me -> Other Blocked
			iTTW = 700109; 
			bDamaged = false;
		}	
		else if( PgActionResult::EBT_REACTION == eBlockEype )
		{
			iTTW = 791590;
			bDamaged = true;
		}
	}
	if( bTargetMe )
	{
		if( PgActionResult::EBT_NONE_REACTION == eBlockEype )
		{//Me -> Other Blocked
			iTTW = 700110;
			bDamaged = false;
		}	
		else if( PgActionResult::EBT_REACTION == eBlockEype )
		{
			iTTW = 791591;
			bDamaged = true;
		}
	}
	if( !bTargetMe && bDodged )		{iTTW = 700111; bDamaged = false;}		//Me -> Other Dodged
	if( bTargetMe && bDodged )		{iTTW = 700112; bDamaged = false;}		//Other -> Me Dodged
	
	std::wstring kName;
	if( bTargetMe )
	{
		kName = pkCaster->GetName();//적 이름
	}
	else
	{
		kName = pkTarget->GetName();//적 이름
	}

	PgPilot* pPilot = NULL;
	if( kName.empty() )
	{
		if( bTargetMe )
		{
			CUnit* pUnit = pkCaster->GetUnit();
			if(pUnit)
			{
				pPilot = g_kPilotMan.FindPilot( pUnit->Caller() );
			}
		}
		else
		{
			CUnit* pUnit = pkTarget->GetUnit();
			if(pUnit)
			{
				pPilot = g_kPilotMan.FindPilot( pUnit->Caller() );
			}
		}

		if(pPilot)
		{
 			kName = pPilot->GetName();
		}
	}

	if( !iTTW )
	{
		return;
	}
	
	BM::vstring vLog( TTW(iTTW) );
	if( bDamaged )
	{
		vLog.Replace( L"#WHO#", kName );
		vLog.Replace( L"#VAL#", ::abs(iDamage) );
	}
	else
	{
		vLog.Replace( L"#WHO#", kName );
	}

	if( vLog.size() )
	{
		SChatLog kChatLog(CT_BATTLE);
		kChatLog.bMine = bTargetMe;
		g_kChatMgrClient.AddLogMessage( kChatLog, vLog.operator const std::wstring &() );
	}
}
//<<local function

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionTargetList
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool	PgActionTargetList::CopyActionResultTo(PgActionTargetList &kTarget)	//	이펙트만 복사한다.
{
	PgActionTargetInfo	*pkTargetInfo=NULL, *pkTargetInfo2=NULL;
	for(ActionTargetList::iterator itor = m_kActionTargetList.begin(); itor != m_kActionTargetList.end(); ++itor)
	{
		pkTargetInfo = &(*itor);
		pkTargetInfo2 = kTarget.GetTargetByGUID(pkTargetInfo->GetTargetPilotGUID());
		if(!pkTargetInfo2) 
		{
			return	false;
		}

		pkTargetInfo2->CopyFrom(*pkTargetInfo);
	}
	return	true;
}

void	PgActionTargetList::SetEffectReal(bool const bReal)
{
	PgActionTargetInfo* pkTargetInfo = NULL;
	for(ActionTargetList::iterator itor = m_kActionTargetList.begin(); itor != m_kActionTargetList.end(); ++itor)
	{
		pkTargetInfo = &(*itor);
		pkTargetInfo->GetActionResult().SetRealEffect(bReal);
	}
}

PgActionTargetInfo*	PgActionTargetList::GetTargetByGUID(BM::GUID const& kGUID)
{
	for(ActionTargetList::iterator itor = m_kActionTargetList.begin(); itor != m_kActionTargetList.end(); ++itor)
	{
		if((*itor).GetTargetPilotGUID() == kGUID)
		{
			return	&(*itor);
		}
	}
	return	NULL;
}
void	PgActionTargetList::ApplyOnlyDamage(int const iDivide, bool const bApplyEffects, float const fRandomPosRange)
{
	if(!g_pkWorld
		|| m_kActionTargetList.empty()
		)
	{
		return;
	}

	PgPilot* pkCasterPilot = g_kPilotMan.FindPilot(m_kCasterGUID);
	if(!pkCasterPilot) 
	{
		NILOG(PGLOG_LOG,"PgActionTargetList::ApplyOnlyDamage() No Caster Pilot. CasterGUID : %s\n",MB(m_kCasterGUID.str()));
		return;
	}

	PgActor* pkCasterActor = (PgActor*)pkCasterPilot->GetWorldObject();
	if(!pkCasterActor) 
	{
		NILOG(PGLOG_LOG,"PgActionTargetList::ApplyOnlyDamage() No Caster Actor. CasterGUID : %s CasterName : %s\n",MB(m_kCasterGUID.str()),MB(pkCasterPilot->GetName()));
		return;
	}

	//	아래 내용 처리중에 m_kActionTargetList 가 Clear 될수 있다. 따라서 복사본을 가지고 작업을 하자.
	ActionTargetList kActionTargetList = m_kActionTargetList;
	
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(m_iActionNo);
	if(!pkSkillDef)
	{
		return;
	}

	NiPoint3	kCasterActorPos = pkCasterActor->GetPos();

	PgPilot* pkPilot = NULL;
	PgActor* pkActor = NULL;
	CUnit* pkUnit = NULL;
	PgActionResult* pkActionResult = NULL;
	PgActionTargetInfo* pkActionTargetInfo = NULL;

	for(ActionTargetList::iterator itor = kActionTargetList.begin(); itor != kActionTargetList.end(); ++itor)
	{
		pkActionTargetInfo = &(*itor);
		if(!pkActionTargetInfo)
		{
			continue;
		}
		pkActionResult = &(pkActionTargetInfo->GetActionResult());
		if(!pkActionResult)
		{
			continue;
		}

		pkPilot = g_kPilotMan.FindPilot(pkActionTargetInfo->GetTargetPilotGUID());
		if(pkPilot && pkPilot->GetUnit())
		{
			pkUnit = pkPilot->GetUnit();

			//복구처리만 해주어야 한다
			if(true == pkActionResult->GetRestore())
			{
				int iNowHP = pkPilot->GetAbil(AT_HP);				
				pkPilot->SetAbil(AT_HP, std::max(pkActionResult->GetRemainHP(), 1));
				pkUnit->CUnit::SetState(US_IDLE);

				continue;
			}
			pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			if(!pkActor)
			{
				continue;
			}
			bool const bBlockReaction = PgActionResult::EBT_REACTION == pkActionResult->GetBlocked();
			bool const bActionMissed = lwActionResult(pkActionResult).IsMissed();
			ShowActionResultText(pkActionResult,pkCasterActor,pkActor);

			if(bActionMissed == false)
			{//공격이 성공하고
				if(pkActor->IsDownState() == false && pkActor->IsBlowUp() == false && pkSkillDef->GetAbil(AT_DAMAGEACTION_TYPE)==0)
				{
					if (pkPilot->GetAbil(AT_NOT_SEE_PLAYER) == 0 && pkUnit->UnitType() != UT_PLAYER ) // 맞았을 때 돌아보는지. 0이면 보통 101이면 돌아보지 않음
					{
						pkActor->LookAt(pkCasterActor->GetPosition(),true,true,true);
					}
					if(PgActionTargetListUtil::CheckDoDamageAction(pkPilot, pkActionResult))
					{// 데미지 액션을 해야 하면 하고
						pkActor->AddEffect(ACTIONEFFECT_DMG, 0, 0, m_kCasterGUID, m_iActionInstanceID, 0, true, pkSkillDef);	// a_dmg effect
					}
				}
				bool bTargetType = false;
				if(pkSkillDef->GetTargetType() & ESTARGET_ENEMY)
				{
					bTargetType = true;
					//스킬의 타켓 타입이 적일때만 콤보가 증가 한다.
				}

				if(pkCasterActor->IsUnderMyControl() && bTargetType)
				{
					pkCasterActor->IncreaseChainAttackCount(m_iActionNo);
				}

				if(pkActionResult->GetValue() > 0)
				{
					int	iDamage = (int)(pkActionResult->GetValue()/((float)iDivide));

					if(iDamage>0)
					{
						ShowDamageNum(pkCasterActor,pkActor,iDamage, fRandomPosRange, pkActionResult->GetCritical(), pkActionResult);

						int	const iCurrentHP = pkUnit->GetAbil(AT_HP); 
						int	const iRemainHP = iCurrentHP-iDamage;
						if(iRemainHP >= 0 && pkActionResult->IsValidHP())
						{
							pkActor->RefreshHPGaugeBar(iCurrentHP,iRemainHP,pkCasterActor, true);
						}
					}
				}

				int const iGroggyPoint = GetGroggyPoint(pkSkillDef);
				if(iGroggyPoint > 0 && EPS_FRENZY==pkPilot->GetAbil(AT_ELITEPATTEN_STATE))
				{
					int const iPoint = (int)(iGroggyPoint/((float)iDivide));
					if(iPoint)
					{
						int const iCurrent = pkPilot->GetAbil(AT_GROGGY_NOW);
						int const iRemain = iCurrent+iPoint;
						pkPilot->SetAbil(AT_GROGGY_NOW,std::max<int>(iRemain,0));
					}
				}
			}

			if(bApplyEffects)
			{
				int	iEffectID = 0;
				for(size_t i = 0; i < pkActionResult->GetEffectNum(); ++i)
				{
					iEffectID = pkActionResult->GetEffect(i);
					_PgOutputDebugString("iEffectID:%d Pilot : GUID %s NAME : %s\n",iEffectID,MB(pkPilot->GetGuid().str()),MB(pkPilot->GetName()));

					if(iEffectID == ACTIONEFFECT_DIE || iEffectID == ACTIONEFFECT_VOLUNTARILY_DIE)
					{
						continue;
					}

					if (pkActionResult->IsDamageAction())
					{
						_PgOutputDebugString("pkActor->AddEffect iEffectID:%d Pilot : GUID %s NAME : %s\n",iEffectID,MB(pkPilot->GetGuid().str()),MB(pkPilot->GetName()));
						pkActor->AddEffect(pkActionResult->GetEffect(i), 0,0, m_kCasterGUID, m_iActionInstanceID, m_dwTimeStamp, pkActionResult->IsRealEffect() == false, pkSkillDef);
					}
				}
			}
		}
	}
}
void	PgActionTargetList::ShowActionResultText(PgActionResult* pkActionResult,PgActor* pkCaster,PgActor* pkTarget)
{
	if(NULL == pkTarget
		|| NULL == pkActionResult
		)
	{
		return;
	}
	
	NiAVObjectPtr spkAvObj = pkTarget->GetCharRoot();
	if(NULL == spkAvObj)
	{
		return;
	}
	
	NiPoint3 kTargetPos = pkTarget->GetPos();
	NiAVObject* pkDummy = spkAvObj->GetObjectByName(ATTACH_DAMAGE_NUM_NODE);
	
	if(NULL == pkDummy)
	{
		//pkDummy = pkTarget->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
		pkDummy = pkTarget->GetNodePointStar();
	}

	if(pkDummy)
	{
		kTargetPos = pkDummy->GetWorldTranslate();
	}

	if(pkActionResult->GetAbil(AT_DAMAGEACTION_TYPE) != 0 && pkActionResult->GetValue() == 0)
	{
		return;
	}

	//스킬에 의해서 데미지를 흡수 했을 때
	if(pkActionResult->GetAbsorbValue())
	{
		return;
	}

	if(pkActionResult->GetMissed())
	{
		pkTarget->ShowSimpleText(kTargetPos,PgDamageNumMan::STT_MISS);
	}
	else if(0 < pkActionResult->GetBlocked())
	{
		switch( pkActionResult->GetBlocked() ) 
		{
		case PgActionResult::EBT_NONE_REACTION:
			{
				lwActor(pkTarget).AttachParticleToPoint(300,lwActor(pkTarget).GetNodeWorldPos("char_root"),"ef_blocking_01");
			}//의도된 break제거
		case PgActionResult::EBT_REACTION:
			{
				pkTarget->ShowSimpleText(kTargetPos,PgDamageNumMan::STT_BLOCK);
			}break;
		}
	}
	//if(pkActionResult->GetCritical())
	//{// 않쓰임
	//	//pkTarget->ShowSimpleText(kTargetPos,PgDamageNumMan::STT_CRITICAL);
	//}
	//if(pkActionResult->GetDodged())
	//{// 않쓰임
	//	pkTarget->ShowSimpleText(kTargetPos,PgDamageNumMan::STT_DODGE);
	//}

	//
	PgPilot* pkCasterPilot = NULL;
	PgPilot* pkTargetPilot = NULL;
	if( pkCaster ) {pkCasterPilot = pkCaster->GetPilot();}
	if( pkTarget ) {pkTargetPilot = pkTarget->GetPilot();}
	AddDamageLog(pkCasterPilot, pkTargetPilot, 0, pkActionResult);//헛침, 미스, 방어, 등등..
}

void	PgActionTargetList::ApplyActionEffects(BM::GUID const &kTargetGUID)
{
	if(!g_pkWorld)
	{
		return;
	}

	PgPilot* pkCasterPilot = g_kPilotMan.FindPilot(m_kCasterGUID);
	if(!pkCasterPilot) 
	{
		NILOG(PGLOG_LOG,"PgActionTargetList::ApplyActionEffects()[1] No Caster Pilot. CasterGUID : %s\n",MB(m_kCasterGUID.str()));
		return;
	}

	PgActor* pkCasterActor = (PgActor*)pkCasterPilot->GetWorldObject(); 
	if(!pkCasterActor)
	{ 
		NILOG(PGLOG_LOG,"PgActionTargetList::ApplyActionEffects()[1] No Caster Actor. CasterGUID : %s CasterName : %s\n",MB(m_kCasterGUID.str()),MB(pkCasterPilot->GetName())); 
		return; 
	} 

	//	아래 내용 처리중에 m_kActionTargetList 가 Clear 될수 있다. 따라서 복사본을 가지고 작업을 하자.
	ActionTargetList	kActionTargetList = m_kActionTargetList;	

	if( kActionTargetList.empty() )
	{
		return;
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(m_iActionNo);
	if(!pkSkillDef) { return; }

	_PgOutputDebugString("PgActionTargetList::ApplyActionEffects[1] ActionInstanceID : %d Pilot : GUID %s NAME : %s EffectSize : %d\n",m_iActionInstanceID,MB(m_kCasterGUID.str()),MB(pkCasterPilot->GetName()),kActionTargetList.size());

	NiPoint3 kCasterActorPos = pkCasterActor->GetPos();

	PgPilot* pkPilot = NULL;
	PgActor* pkActor = NULL;
	CUnit* pkUnit = NULL;
	PgActionResult* pkActionResult = NULL;
	PgActionTargetInfo* pkActionTargetInfo = NULL;
	for(ActionTargetList::iterator itor = kActionTargetList.begin(); itor != kActionTargetList.end(); ++itor)
	{
		pkActionTargetInfo = &(*itor);
		if(!pkActionTargetInfo)
		{
			continue;
		}
		if(pkActionTargetInfo->GetTargetPilotGUID() != kTargetGUID) { continue; }

		pkActionResult = &(pkActionTargetInfo->GetActionResult());
		if(!pkActionResult)
		{
			continue;
		}
		pkPilot = g_kPilotMan.FindPilot(pkActionTargetInfo->GetTargetPilotGUID());
		if(pkPilot && pkPilot->GetUnit())
		{
			pkUnit = pkPilot->GetUnit();

			//복구처리만 해주어야 한다
			if(true == pkActionResult->GetRestore())
			{
				int iNowHP = pkPilot->GetAbil(AT_HP);				
				pkPilot->SetAbil(AT_HP, std::max(pkActionResult->GetRemainHP(), 1));
				pkUnit->CUnit::SetState(US_IDLE);

				continue;
			}

			pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
			if(!pkActor)
			{
				continue;
			}
			bool const bBlockReaction = PgActionResult::EBT_REACTION == pkActionResult->GetBlocked();
			bool const bActionMissed = lwActionResult(pkActionResult).IsMissed();
			ShowActionResultText(pkActionResult,pkCasterActor,pkActor);

			if(0 == m_dwTimeStamp || pkPilot->GetLastHPSetTimeStamp() < m_dwTimeStamp )
			{
				if(0 < m_dwTimeStamp)
				{
					pkPilot->SetLastHPSetTimeStamp(m_dwTimeStamp);
				}

				if(0 <= pkActionTargetInfo->GetRemainHP() && false==bActionMissed)
				{
					int	const iCurrentHP = pkPilot->GetAbil(AT_HP);
					int	const iNewHP = pkActionTargetInfo->GetRemainHP();
					if(pkActionResult->IsValidHP())
					{
						pkActor->RefreshHPGaugeBar(iCurrentHP,iNewHP,pkCasterActor, true);
					}
				}
			}

			if(false == bActionMissed)
			{
				if(false == pkActor->IsDownState() && false == pkActor->IsBlowUp() && 0 == pkSkillDef->GetAbil(AT_DAMAGEACTION_TYPE))
				{
					if (0 == pkPilot->GetAbil(AT_NOT_SEE_PLAYER) && UT_PLAYER != pkUnit->UnitType()) // 맞았을 때 돌아보는지. 0이면 보통 101이면 돌아보지 않음
					{
						//pkActor->FindPathNormal();
						pkActor->LookAt(pkCasterActor->GetPosition(), true, true, true);
					}
					if(PgActionTargetListUtil::CheckDoDamageAction(pkPilot, pkActionResult))
					{// 데미지 액션을 해야 하면 하고
						pkActor->AddEffect(ACTIONEFFECT_DMG, 0, 0, m_kCasterGUID, m_iActionInstanceID, 0, true, pkSkillDef);	// a_dmg effect
					}
				}

				bool bTargetType = false;
				if(ESTARGET_ENEMY == pkSkillDef->GetTargetType())
				{
					bTargetType = true;
					//스킬의 타켓 타입이 적일 경우 콤보 증가
				}

				if(pkCasterActor->IsUnderMyControl() && bTargetType)
				{
					pkCasterActor->IncreaseChainAttackCount(m_iActionNo);
				}
			}

			ShowDamageNum(pkCasterActor,pkActor,pkActionResult->GetValue(), 0, pkActionResult->GetCritical(), pkActionResult);
			ShowReflectDamageNum(pkCasterActor,pkActor,pkActionResult->GetAbil(AT_REFLECTED_DAMAGE),pkActionResult);

			bool bDie= false;
			int	iEffectID = 0;
			for(size_t i = 0; i < pkActionResult->GetEffectNum(); ++i)
			{
				iEffectID = pkActionResult->GetEffect(i);
				_PgOutputDebugString("iEffectID:%d Pilot : GUID %s NAME : %s\n",iEffectID,MB(pkPilot->GetGuid().str()),MB(pkPilot->GetName()));

				bDie = (iEffectID==ACTIONEFFECT_DIE || iEffectID==ACTIONEFFECT_VOLUNTARILY_DIE);
				int const iType = pkPilot->GetAbil(AT_DAMAGEACTION_TYPE);
				if (bDie || 0 == iType
					|| (1 == iType && pkPilot->GetAbil(AT_DAMAGEACTION_RATE) && !pkActionResult->GetEndure())//무적어빌이 있지만 확률어빌이 있고 확률굴림을 실패했을 때
					|| (E_DMGACT_TYPE_PART_SUPERARMOUR==iType && iEffectID && pkActionResult->IsDamageAction()))

				{
					_PgOutputDebugString("pkActor->AddEffect iEffectID:%d Pilot : GUID %s NAME : %s\n",iEffectID,MB(pkPilot->GetGuid().str()),MB(pkPilot->GetName()));
					pkActor->AddEffect(pkActionResult->GetEffect(i), 0,0, m_kCasterGUID, m_iActionInstanceID, m_dwTimeStamp, pkActionResult->IsRealEffect() == false, pkSkillDef);
				}
			}

			if(0 < pkActionResult->GetEffectNum())
			{
				// Blow값이 있을 경우만 세팅해 줘야 한다.
				bool bBlowValue = false;
				int const iEffectSize = static_cast<int>(pkActionResult->GetEffectNum());
				for(int i = 0; i < iEffectSize; ++i)
				{
					GET_DEF(CEffectDefMgr, kEffectDefMgr);
					CEffectDef const* pkEffectDef = kEffectDefMgr.GetDef(pkActionResult->GetEffect(0));
					if(pkEffectDef)
					{
						if(0 < pkEffectDef->GetAbil(AT_BLOW_VALUE))
						{
							bBlowValue = true;
						}
					}
				}

				if(bBlowValue)
				{					
					pkUnit->SetBlowAttacker(m_kCasterGUID);
					pkActor->SetSendBlowStatus(pkCasterActor->IsUnderMyControl() || g_kPilotMan.IsMySummoned(pkCasterActor->GetUnit()) );
				}
			}

			if(false == bDie)
			{
				for(size_t i = 0; i < pkActionResult->GetStatusEffectNum(); ++i)
				{
					PgActionResult::stStatusEffect* pkEffectInfo = pkActionResult->GetStatusEffect(i);
					if(!pkEffectInfo) 
					{
						continue;
					}
					iEffectID = pkEffectInfo->m_iEffectID;
					_PgOutputDebugString("iEffectID:%d Pilot : GUID %s NAME : %s\n",iEffectID,MB(pkPilot->GetGuid().str()),MB(pkPilot->GetName()));
					pkActor->AddEffect(iEffectID, pkEffectInfo->m_iEffectValue,0, m_kCasterGUID, m_iActionInstanceID, m_dwTimeStamp, pkActionResult->IsRealEffect() == false, pkSkillDef);
				}
			}
		}
		else
		{
			_PgOutputDebugString("Target Pilot Not Found\n");
			ApplyDieEffectToStillLoadingPilot(pkActionTargetInfo->GetTargetPilotGUID(),pkActionResult);
		}
	}
}

void	PgActionTargetList::ApplyActionEffects(bool const bOnlyDieEffect, bool const bNoShowDamageNum, float const fRandomPosRange, bool bReleaseAction)
{
	if(IsActionEffectApplied()
		|| m_kActionTargetList.empty()
		|| NULL == g_pkWorld
		)
	{
		return;
	}
	PgPilot* pkCasterPilot = g_kPilotMan.FindPilot(m_kCasterGUID);
	if(!pkCasterPilot) 
	{
		NILOG(PGLOG_LOG,"PgActionTargetList::ApplyActionEffects() No Caster Pilot. CasterGUID : %s\n",MB(m_kCasterGUID.str()));
		return;
	}
	PgActor* pkCasterActor = dynamic_cast<PgActor*>(pkCasterPilot->GetWorldObject());
	if(!pkCasterActor) 
	{
		NILOG(PGLOG_LOG,"PgActionTargetList::ApplyActionEffects() No Caster Actor. CasterGUID : %s \n",MB(m_kCasterGUID.str()));
		return;
	}
	
	SetActionEffectApplied(true);

	bool bCasterActorIsMyActor = false;
	BM::GUID kPlayerPilotGuid;
	if(g_kPilotMan.GetPlayerPilotGuid(kPlayerPilotGuid))
	{
		bCasterActorIsMyActor = (kPlayerPilotGuid == m_kCasterGUID);
		if(!bCasterActorIsMyActor
			&& pkCasterPilot
			)
		{
			CUnit* pkUnit = pkCasterPilot->GetUnit();
			if(pkUnit)
			{
				if(pkUnit->IsUnitType(UT_SUB_PLAYER))
				{
					bCasterActorIsMyActor = (pkUnit->Caller() == kPlayerPilotGuid);
				}
				else if(pkUnit->IsUnitType(UT_SUMMONED))
				{
					bCasterActorIsMyActor = g_kPilotMan.IsMySummoned(pkUnit);
				}
			}
		}
	}

	//	아래 내용 처리중에 m_kActionTargetList 가 Clear 될수 있다. 따라서 복사본을 가지고 작업을 하자.
	ActionTargetList kActionTargetList = m_kActionTargetList;
	
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(m_iActionNo);
	if(!pkSkillDef)
	{
		return;
	}

	{//	공격자 앞으로 밀기
		int	iCasterMoveRange = pkSkillDef->GetAbil(AT_CASTER_MOVERANGE);
		if(0 < iCasterMoveRange && pkCasterActor)
		{
			pkCasterActor->Walk((Direction)pkCasterActor->GetDirection(),(float)iCasterMoveRange,1,true);
		}

		int	iTargetMoveRange = pkSkillDef->GetAbil(AT_TARGET_MOVERANGE);	//	맞는 놈이 뒤로 밀릴 거리
		Direction kTargetMoveDirection = DIR_LEFT;	//	맞는 놈이 뒤로 밀릴 방향
		if(pkCasterActor && DIR_LEFT == pkCasterActor->GetDirection())
		{
			kTargetMoveDirection = DIR_RIGHT;
		}
	}

	NiPoint3 kCasterActorPos(0.0f, 0.0f, 0.0f);
	if(pkCasterActor)
	{
		kCasterActorPos = pkCasterActor->GetPos();
	}

	PgPilot* pkPilot = NULL;
	PgActor* pkActor = NULL;
	CUnit* pkUnit = NULL;
	PgActionResult* pkActionResult = NULL;
	PgActionTargetInfo* pkActionTargetInfo = NULL;
	
	GET_DEF(CEffectDefMgr, kEffectDefMgr);

	for(ActionTargetList::iterator itor = kActionTargetList.begin(); itor != kActionTargetList.end(); ++itor)
	{
		pkActionTargetInfo = &(*itor);
		if(!pkActionTargetInfo)
		{
			continue;
		}
		pkActionResult = &(pkActionTargetInfo->GetActionResult());
		if(!pkActionResult)
		{
			continue;
		}

		pkPilot = g_kPilotMan.FindPilot(pkActionTargetInfo->GetTargetPilotGUID());
		if(!pkPilot)
		{
			ApplyDieEffectToStillLoadingPilot(pkActionTargetInfo->GetTargetPilotGUID(),pkActionResult);
			continue;
		}
		pkUnit = pkPilot->GetUnit();
		if(!pkUnit)
		{
			continue;
		}
		
		if(true == pkActionResult->GetRestore())
		{//복구처리만 해주어야 한다
			int iNowHP = pkPilot->GetAbil(AT_HP);				
			pkPilot->SetAbil(AT_HP, std::max(pkActionResult->GetRemainHP(), 1));
			pkUnit->CUnit::SetState(US_IDLE);
			continue;
		}
		
		pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
		if(!pkActor)
		{
			continue;
		}

		bool const bBlockReaction = PgActionResult::EBT_REACTION == pkActionResult->GetBlocked();
		int const iType = pkPilot->GetAbil(AT_DAMAGEACTION_TYPE);
		int const iDmgActionRate = pkPilot->GetAbil(AT_DAMAGEACTION_RATE);
		int const iDieMotion = pkPilot->GetAbil(AT_DIE_MOTION);
		bool const bActionMissed = lwActionResult(pkActionResult).IsMissed();
		bool const bShowDamgeNum = ( false==bNoShowDamageNum && 0 == pkUnit->GetAbil(AT_NOSHOWDAMAGENUM) );	//데미지 숫자를 보여주는가

		if(0 == m_dwTimeStamp 
			|| pkPilot->GetLastHPSetTimeStamp() < m_dwTimeStamp 
			)
		{
			if(0 < m_dwTimeStamp)
			{
				pkPilot->SetLastHPSetTimeStamp(m_dwTimeStamp);
			}

			if( 0 <= pkActionTargetInfo->GetRemainHP() 
				&& false==bActionMissed
				)
			{
				int	const iCurrentHP = pkPilot->GetAbil(AT_HP);
				int	const iNewHP = pkActionTargetInfo->GetRemainHP();
				if(	pkActionTargetInfo->GetActionResult().IsValidHP() )
				{
					if(0 < pkActionTargetInfo->GetActionResult().GetValue())
					{
						if(!pkUnit->IsDead())
						{// 대상이 이미 죽었을때는(HP=0) 뒤늦게 남은HP를 적용하지 않음
							pkActor->RefreshHPGaugeBar( iCurrentHP, iNewHP, pkCasterActor, iNewHP > 0 );
						}
					}
					else if(!pkUnit->IsDead())
					{
						pkActor->RefreshHPGaugeBar( iCurrentHP, iNewHP, pkCasterActor, iNewHP > 0 );
					}
				}
			}
		}

		if(false == bActionMissed)
		{
			if(!bOnlyDieEffect)
			{
				if(false == pkActor->IsDownState()
					&& false == pkActor->IsBlowUp()
					&& pkActionResult->IsDamageAction()
					)
				{
					if (0 == pkPilot->GetAbil(AT_NOT_SEE_PLAYER) 
						&& UT_PLAYER != pkUnit->UnitType()
						) 
					{// 맞았을 때 돌아보는지. 0이면 보통 101이면 돌아보지 않음
						pkActor->LookAt(kCasterActorPos,true,true,true);
					}

					if(PgActionTargetListUtil::CheckDoDamageAction(pkPilot, pkActionResult))
					{
						pkActor->AddEffect(ACTIONEFFECT_DMG, 0, 0, m_kCasterGUID, m_iActionInstanceID, 0, true, pkSkillDef);	// a_dmg effect
					}
				}

				if (ESTARGET_ENEMY == pkSkillDef->GetTargetType())
				{
					//스킬의 타켓 타입이 나 / 파티 인경우 콤보가 증가하지 않는다.
					if (pkCasterActor && pkCasterActor->IsUnderMyControl())
					{
						pkCasterActor->IncreaseChainAttackCount(m_iActionNo);
					}
				}
			}
			ApplyElementalDmgEffect(pkCasterPilot->GetUnit(), pkActor, *pkActionResult);
		}

		if( bShowDamgeNum )
		{// 느림
			int const iHitCount = pkActionResult->GetAbil(AT_HIT_COUNT);
			if( 0 < iHitCount)
			{
				int iDevideDamage = pkActionResult->GetValue()/iHitCount;
				for( int iCount=0; iCount<iHitCount; ++iCount)
				{
					ShowActionResultText(pkActionResult,pkCasterActor,pkActor);
					ShowDamageNum(pkCasterActor,pkActor,iDevideDamage, fRandomPosRange, pkActionResult->GetCritical(), pkActionResult);
				}
			}
			else
			{
				ShowActionResultText(pkActionResult,pkCasterActor,pkActor);
				ShowDamageNum(pkCasterActor,pkActor,pkActionResult->GetValue(), fRandomPosRange, pkActionResult->GetCritical(), pkActionResult);
			}
			if(false == bReleaseAction)
			{
				ShowReflectDamageNum(pkCasterActor,pkActor,pkActionResult->GetAbil(AT_REFLECTED_DAMAGE),pkActionResult);
			}
		}

		bool bDie = false;
		CEffectDef const* pkEffectDef = NULL;
		int	iEffectID = 0;
		bool bBlowValue = false;
		size_t const iEffectSize = pkActionResult->GetEffectNum();

		for(size_t i = 0;i < iEffectSize; ++i)
		{
			iEffectID = pkActionResult->GetEffect(i);
			bDie = (iEffectID==ACTIONEFFECT_DIE || iEffectID==ACTIONEFFECT_VOLUNTARILY_DIE);
			if (bDie
				|| iType == 0
				|| (iType == 1 && iDmgActionRate && !pkActionResult->GetEndure())//무적어빌이 있지만 확률어빌이 있고 확률굴림을 실패했을 때
				|| (E_DMGACT_TYPE_PART_SUPERARMOUR==iType && iEffectID && pkActionResult->IsDamageAction())
				)
			{
				if(iEffectID==ACTIONEFFECT_RESURRECTION01)
				{
					pkActor->AddEffect(pkActionResult->GetEffect(i), 0,0, m_kCasterGUID, m_iActionInstanceID, m_dwTimeStamp, pkActionResult->IsRealEffect() == false, pkSkillDef);
				}
				else if( (iEffectID!=ACTIONEFFECT_DIE && !bOnlyDieEffect) 
					|| iEffectID==ACTIONEFFECT_DIE
					)
				{
					if(0 == iDieMotion)
					{
						pkActor->AddEffect(pkActionResult->GetEffect(i), 0,0, m_kCasterGUID, m_iActionInstanceID, m_dwTimeStamp, pkActionResult->IsRealEffect() == false, pkSkillDef);
					}
				}
			}

			if(false==bBlowValue) //No Block
			{// Blow값이 있을 경우만 세팅해 줘야 한다.
				pkEffectDef = kEffectDefMgr.GetDef(iEffectID);
				if(pkEffectDef
					&& 0 < pkEffectDef->GetAbil(AT_BLOW_VALUE)
					)
				{
					bBlowValue = true;
				}
			}
		}

		if(bBlowValue 
			&& pkActionResult->IsDamageAction()
			)
		{
			pkUnit->SetBlowAttacker(m_kCasterGUID);
			pkActor->SetSendBlowStatus(bCasterActorIsMyActor || pkActor->IsMyActor());
		}

		if(bDie)
		{
			continue;
		}

		{
			size_t const iEffectSize = pkActionResult->GetStatusEffectNum();
			for(size_t i = 0; i < iEffectSize; ++i)
			{
				PgActionResult::stStatusEffect* pkEffectInfo = pkActionResult->GetStatusEffect(i);
				if(!pkEffectInfo)
				{
					continue;
				}
				iEffectID = pkEffectInfo->m_iEffectID;
				pkActor->AddEffect(iEffectID, pkEffectInfo->m_iEffectValue,0, m_kCasterGUID, m_iActionInstanceID, m_dwTimeStamp, pkActionResult->IsRealEffect() == false, pkSkillDef);
			}
		}
	}
}
void	PgActionTargetList::ApplyDieEffectToStillLoadingPilot(BM::GUID const& kTargetGUID, PgActionResult* pkActionResult)
{
	_PgOutputDebugString("PgActionTargetList::ApplyDieEffectToStillLoadingPilot() TargetGUID : %s\n",MB(kTargetGUID.str()));
	if(!pkActionResult) { return; }

	bool bDie = false;

	_PgOutputDebugString("EffectNum : %d\n",pkActionResult->GetEffectNum());

	int	iEffectID=0;
	for(size_t i = 0; i < pkActionResult->GetEffectNum(); ++i)
	{
		iEffectID = pkActionResult->GetEffect(i);
		_PgOutputDebugString("iEffectID:%d Pilot : GUID %s\n",iEffectID,MB(kTargetGUID.str()));

		//if(bOnlyDieEffect && iEffectID!=ACTIONEFFECT_DIE) continue;

		if(ACTIONEFFECT_DIE==iEffectID || ACTIONEFFECT_VOLUNTARILY_DIE==iEffectID)
		{
			bDie = true;
			break;
		}
	}

	if(!bDie)
	{
		return;
	}

	PgPilotManUtil::RemoveReservePilotUnit(kTargetGUID, __FUNCTION__, __LINE__);
}
void	PgActionTargetList::ShowReflectDamageNum(PgActor* pkCaster, PgActor* pkTarget, int iReflectDamage, PgActionResult* pkActionResult)
{
	if(!pkCaster) { return; }

	bool bDeliverDamageCaller = false;
	if(pkCaster->GetPilot() && pkCaster->GetPilot()->GetUnit())
	{
		if(0 < pkCaster->GetPilot()->GetAbil(AT_DELIVER_DAMAGE_CALLER))
		{
			pkCaster = g_kPilotMan.FindActor(pkCaster->GetPilot()->GetUnit()->Caller());
			bDeliverDamageCaller = true;
		}
	}

	if(!pkCaster) { return; }

	if(0 < iReflectDamage)
	{
		pkCaster->StartDamageBlink(true);
		
		NiPoint3 kDamageNumPos = pkCaster->GetPos();
		if(pkCaster->GetActorManager() && pkCaster->GetActorManager()->GetNIFRoot())
		{
			NiAVObject* pkDummy = pkCaster->GetCharRoot()->GetObjectByName(ATTACH_DAMAGE_NUM_NODE);

			if(NULL == pkDummy)
			{
				//pkDummy = pkCaster->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
				pkDummy = pkCaster->GetNodePointStar();
			}

			if(pkDummy)
			{
				kDamageNumPos = pkDummy->GetWorldTranslate();
				kDamageNumPos.z += 25;
			}
		}
		pkCaster->ShowDamageNum(pkCaster->GetPosition(),kDamageNumPos,iReflectDamage);

		if(0 == pkActionResult->GetAbil(AT_REFLECT_DMG_HP))
		{
			pkCaster->GetPilot()->SetAbil(AT_HP,0);
			pkCaster->AddEffect(ACTIONEFFECT_DIE,0);	//	반사 대미지로 입은 후 HP 가 0이되었을 경우 죽인다.
		}
		else
		{
			int	const iCurrentHP = pkCaster->GetPilot()->GetAbil(AT_HP);
			int	const iNewHP = pkActionResult->GetAbil(AT_REFLECT_DMG_HP);
			if(pkActionResult->IsValidHP())
			{
				pkCaster->RefreshHPGaugeBar(iCurrentHP,iNewHP,pkTarget, true);
			}
		}

		lwActor(pkTarget).AttachParticleToPoint(801299,lwActor(pkTarget).GetNodeWorldPos("p_ef_heart"),"ef_revenge_01_char_root");

		AddDamageLog(pkCaster->GetPilot(), pkCaster->GetPilot(), iReflectDamage, pkActionResult);
	}
}
void	PgActionTargetList::ShowDamageNum(PgActor *pkCaster,PgActor *pkTarget,int iDamage, float fRandomPosRange, bool bCritical, PgActionResult *pkActionResult)
{
	if(!pkTarget)
	{
		return;
	}
	CUnit* pkTargetUnit = pkTarget->GetUnit();
	if(pkTargetUnit
		&& pkTargetUnit->IsUnitType(UT_SUB_PLAYER)
		)
	{// SUB_PLAYER 타입은 데미지를 띄우지 않고
		return;
	}

	if( g_pkWorld
		&& g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION)
        && (g_pkWorld->GetDifficultyLevel() > 7)
		&& g_kMissionMgr.NowStage() >= g_iF7DamageHideStage
		)
	{//전략디펜스 모드의 현재 스테이지가 데미지 표시 제한 스테이지라면
		return;
	}
	

	int const ShowOtherPlayerDamege = g_kGlobalOption.GetCurrentGraphicOption("DISPLAY_OTHERPLAYER_DAMAGE");
	if( pkCaster && false == ShowOtherPlayerDamege )
	{ // 타 플레이어의 데미지를 보이고 싶지 않다면 자신과 관계된 데미지만 표시함
		bool bMyDamege = pkCaster->IsMyActor();						//캐스터가 나 이거나
		bMyDamege |= pkTarget->IsMyActor();							//타겟이 나 이거나
		bMyDamege |= pkCaster->IsUnderMyControl();					//캐스터가(Entity, SubPlayer, Pet) 내 것이거나
		bMyDamege |= pkTarget->IsUnderMyControl();					//타겟이(Entity, SubPlayer, Pet) 내 것이거나
		bMyDamege |= g_kPilotMan.IsMySummoned(pkCaster->GetUnit()); //캐스터를 소환한 사람이 나이거나
		bMyDamege |= g_kPilotMan.IsMySummoned(pkTargetUnit); //타겟을 소환한 사람이 나 이거나
		if( false == bMyDamege )
		{ // 나와 관계된 데미지가 아니면 무시
			return;
		}
	}
	if(0 < iDamage)
	{
		pkTarget->StartDamageBlink(true);
		
		PgPilot* pkTargetPilot = pkTarget->GetPilot();
		if ( pkTargetPilot && (0 < pkTargetPilot->GetAbil(AT_INVINCIBLE2)) )
		{
			return;
		}

		NiPoint3	kDamageNumPos = pkTarget->GetPos();
		
		if(pkTarget->GetActorManager() && pkTarget->GetActorManager()->GetNIFRoot())
		{
			NiAVObject* pkDummy = pkTarget->GetNIFRoot()->GetObjectByName(ATTACH_DAMAGE_NUM_NODE);

			if(NULL == pkDummy)
			{
				//pkDummy = pkTarget->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
				pkDummy = pkTarget->GetNodePointStar();
			}

			if(pkDummy)
			{
				kDamageNumPos = pkDummy->GetWorldTranslate();
				kDamageNumPos.z+=25;
			}
		}

		if(fRandomPosRange > 0.0f)
		{
			NiPoint3 kAddPos = GetfPoint3Range(-fRandomPosRange, fRandomPosRange, -fRandomPosRange, fRandomPosRange, -fRandomPosRange, fRandomPosRange); 
			kDamageNumPos += kAddPos;
		}

		int iDiffEnchantLevel = 0;
		
		if(pkCaster && pkCaster->GetPilot() && pkCaster->GetPilot()->GetUnit() &&
			pkTargetPilot && pkTargetPilot->GetUnit())
		{
			CUnit* pkCasterUnit = pkCaster->GetPilot()->GetUnit();
			CUnit* pkTargetUnit = pkTargetPilot->GetUnit();

			//pkCasterUnit가 인챈트속성을 가질수 있는 것 이면 실제 Player로 교채해주어야 한다.
			if( (pkCasterUnit->IsUnitType(UT_MONSTER) || pkCasterUnit->IsUnitType(UT_ENTITY) || pkCasterUnit->IsUnitType(UT_OBJECT))
			&&	pkTargetUnit->IsUnitType(UT_PLAYER) )
			{
				std::swap(pkCasterUnit, pkTargetUnit);
			}

			iDiffEnchantLevel = pkCasterUnit->GetDiffEnchantLevel(pkTargetUnit);
		}

		if(pkCaster)
		{
			bool bIsMyActor = pkCaster->IsMyActor(); //캐스터가 나 이거나
			bIsMyActor |= g_kPilotMan.IsMyPlayer(pkCaster->GetPilot()->GetUnit()->Caller()); //캐스터를 소환한 사람이 나이거나

			pkTarget->ShowDamageNum(pkCaster->GetPosition(),kDamageNumPos,iDamage, bIsMyActor, bCritical, 5, iDiffEnchantLevel, pkTargetPilot->GetAbil(AT_EXCEPT_DAMAGE_FONT));
		}
		else
		{
			pkTarget->ShowDamageNum(kDamageNumPos,kDamageNumPos,iDamage, false, bCritical, 5, iDiffEnchantLevel, pkTargetPilot->GetAbil(AT_EXCEPT_DAMAGE_FONT));
		}

		if(pkCaster)
		{
			AddDamageLog(pkCaster->GetPilot(), pkTargetPilot, iDamage, pkActionResult);
		}
	}
}

void PgActionTargetList::ApplyElementalDmgEffect(CUnit* pkCaster, lwActor kTargetActor, PgActionResult& rkActionResult)
{
	if( !pkCaster
		|| kTargetActor.IsNil() 
		)
	{
		return;
	}
	
	{// 속성 데미지 계산 부분
		lwPilot kPilot = kTargetActor.GetPilot();
		if(kPilot.IsNil())
		{
			return;
		}
		CUnit* pkTarget = kPilot.GetUnit()();
		if(!pkTarget)
		{
			return;
		}

		int const iDestroyDmg = pkCaster->GetAbil(AT_C_ATTACK_ADD_DESTROY) 
			* (1.0f - CalcTotalElementalResistRate(pkCaster, pkTarget, AT_C_RESIST_ADD_DESTROY));
		int const iFireDmg = pkCaster->GetAbil(AT_C_ATTACK_ADD_FIRE)
			* (1.0f - CalcTotalElementalResistRate(pkCaster, pkTarget, AT_C_RESIST_ADD_FIRE));
		int const iIceDmg = pkCaster->GetAbil(AT_C_ATTACK_ADD_ICE)
			* (1.0f -CalcTotalElementalResistRate(pkCaster, pkTarget, AT_C_RESIST_ADD_ICE));
		int const iNatureDmg = pkCaster->GetAbil(AT_C_ATTACK_ADD_NATURE)
			* (1.0f -CalcTotalElementalResistRate(pkCaster, pkTarget, AT_C_RESIST_ADD_NATURE));
		int const iCurseDmg = pkCaster->GetAbil(AT_C_ATTACK_ADD_CURSE)
			* (1.0f -CalcTotalElementalResistRate(pkCaster, pkTarget, AT_C_RESIST_ADD_CURSE));
		rkActionResult.SetElemDmg(pkCaster, iDestroyDmg, iFireDmg, iIceDmg, iNatureDmg, iCurseDmg);
	}
	
	lwPoint3 kPos = kTargetActor.GetNodeTranslate("p_ef_heart");
	float const fScale = kTargetActor.GetEffectScale();
	if( rkActionResult.ElemDmgDestroy() )
	{
		kTargetActor.AttachParticleToPointS(20110307, kPos, "eff_hit_lightning01", fScale);
	}
	if( rkActionResult.ElemDmgFire() )
	{
		kTargetActor.AttachParticleToPointS(20110307, kPos, "ef_mag_cannon_dmg", fScale);
	}
	if( rkActionResult.ElemDmgIce() )
	{
		kTargetActor.AttachParticleToPointS(20110307, kPos, "ef_common_dmg_ice_01", fScale);
	}
	if( rkActionResult.ElemDmgNature() )
	{
		kTargetActor.AttachParticleToPointS(20110307, kPos, "eff_hit_nature00", fScale);
	}
	if( rkActionResult.ElemDmgCurse() )
	{
		kTargetActor.AttachParticleToPointS(20110307, kPos, "eff_hit_curse00", fScale);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionTargetTransferInfoMan
/////////////////////////////////////////////////////////////////////////////////////////////////
void	PgActionTargetTransferInfoMan::Create()
{
}
void	PgActionTargetTransferInfoMan::Destroy()
{
	PG_ASSERT_LOG(m_Cont.size() == 0);
	if (m_Cont.size() > 0)
	{
		NILOG(PGLOG_LOG, "[PgActionTargetTransferInfoMan] Destroy() - not all released!(%d)\n", m_Cont.size());
	}
	m_Cont.clear();
}

void	PgActionTargetTransferInfoMan::AddTransferInfo(BM::GUID const& kOriginalActor,
	int const iOriginalActionNo,int const iOriginalActionInstanceID,
	BM::GUID const& kTransferredActorGUID)
{
	m_Cont.push_back(
		stActionTargetTransferInfo(
		kOriginalActor,
		iOriginalActionNo,
		iOriginalActionInstanceID,
		kTransferredActorGUID));
}
void	PgActionTargetTransferInfoMan::AddTransferInfo(BM::GUID const& kOriginalActor,
	int const iOriginalActionNo,int const iOriginalActionInstanceID,
	int const iProjectileUID)
{
	m_Cont.push_back(
		stActionTargetTransferInfo(
		kOriginalActor,
		iOriginalActionNo,
		iOriginalActionInstanceID,
		iProjectileUID));
}

void	PgActionTargetTransferInfoMan::DeleteTransferInfo(BM::GUID const& kOriginalActor,
	int const iOriginalActionNo,int const iOriginalActionInstanceID,
	BM::GUID const& kTransferredActorGUID)
{
	stActionTargetTransferInfo* pkInfo = NULL;
	for(ActionTargetListTransferInfoCont::iterator itor = m_Cont.begin();
		itor != m_Cont.end();++itor)
	{
		pkInfo = &(*itor);

		if(pkInfo->m_kOriginalActorGUID == kOriginalActor &&
			pkInfo->m_iOriginalActionNo == iOriginalActionNo &&
			pkInfo->m_iOrigianlActionInstanceID == iOriginalActionInstanceID &&
			pkInfo->m_kTransferredActorGUID == kTransferredActorGUID)
		{
			m_Cont.erase(itor);
			return;
		}

	}
}
void	PgActionTargetTransferInfoMan::DeleteTransferInfo(BM::GUID const& kOriginalActor,
	int const iOriginalActionNo,int const iOriginalActionInstanceID,
	int const iProjectileUID)
{
	stActionTargetTransferInfo* pkInfo = NULL;
	for(ActionTargetListTransferInfoCont::iterator itor = m_Cont.begin();
		itor != m_Cont.end();++itor)
	{

		pkInfo = &(*itor);

		if(pkInfo->m_kOriginalActorGUID == kOriginalActor &&
			pkInfo->m_iOriginalActionNo == iOriginalActionNo &&
			pkInfo->m_iOrigianlActionInstanceID == iOriginalActionInstanceID &&
			pkInfo->m_iTransferredProjectileUID == iProjectileUID)
		{
			m_Cont.erase(itor);
			return;
		}

	}
}
void	PgActionTargetTransferInfoMan::ModifyTargetList(PgActionTargetList& kTargetList)
{
	if(!g_pkWorld) { return; }

	stActionTargetTransferInfo* pkInfo = NULL;
	for(ActionTargetListTransferInfoCont::iterator itor = m_Cont.begin(); itor != m_Cont.end();)
	{
		pkInfo = &(*itor);

		if(pkInfo->m_iOrigianlActionInstanceID == kTargetList.GetActionInstanceID() &&
			pkInfo->m_kOriginalActorGUID == kTargetList.GetCasterGUID() &&
			pkInfo->m_iOriginalActionNo == kTargetList.GetActionNo())
		{
			if(0 < kTargetList.size())
			{
				if(0 <= pkInfo->m_iTransferredProjectileUID)
				{
					PgProjectile* pkProjectile = g_kProjectileMan.GetProjectile(
						pkInfo->m_kOriginalActorGUID,
						pkInfo->m_iOrigianlActionInstanceID,
						pkInfo->m_iTransferredProjectileUID);

					if(pkProjectile)
					{
						pkProjectile->ModifyTargetList(kTargetList);
					}
				}
				else
				{
					PgActor* pkActor = dynamic_cast<PgActor*>(g_pkWorld->FindObject(pkInfo->m_kTransferredActorGUID));
					if(pkActor)
					{
						PgAction* pkAction = pkActor->GetAction();
						if(!pkAction)
						{
							pkAction = pkActor->GetReservedTransitAction();
						}

						if(pkAction && pkAction->GetTargetList() &&
							pkAction->GetTargetList()->GetActionInstanceID() == kTargetList.GetActionInstanceID())
						{
							pkAction->ModifyTargetList(kTargetList);
						}
					}
				}
			}

			itor = m_Cont.erase(itor);
			continue;
		}

		++itor;

	}

}
namespace PgActionTargetListUtil
{
	bool CheckDoDamageAction(PgPilot *pkPilot, PgActionResult* pkActionResult)
	{
		if(!pkPilot || !pkActionResult)
		{
			return false;
		}

		if(0 < pkPilot->GetAbil(AT_DIE_MOTION))
		{
			return false;
		}

		if( 0 < pkActionResult->GetBlocked() )
		{
			return false;
		}

		bool bRealAction = false;
		if(0 < pkActionResult->GetValue() || pkActionResult->GetAbil(AT_NO_DAMAGE_USE_DAMAGEACTION) == 1)//데미지가 0보다 크거나 성기사 스킬이냐?
		{
			int const iType = pkPilot->GetAbil(AT_DAMAGEACTION_TYPE);

			if(E_DMGACT_TYPE_PART_SUPERARMOUR==iType)
			{
				bRealAction = pkActionResult->IsDamageAction();
			}
			else if(pkPilot->GetAbil(AT_DAMAGEACTION_RATE) && !pkActionResult->GetEndure())//무적어빌이 있지만 확률어빌이 있고 확률굴림을 실패했을 때
			{
				bRealAction = true;
			}
			else if(iType == E_DMGACT_TYPE_DEFAULT || iType == E_DMGACT_TYPE_ONLY_DAMAGE)//0 과 10일 때만 데미지 모션 가능
			{
				bRealAction = true;
			}

			if(bRealAction)
			{
				bRealAction = (0 == pkActionResult->GetAbil(AT_DAMAGEACTION_TYPE)); //액션리절트에 타입이 있으면
			}
		}

		return bRealAction;
	}
}