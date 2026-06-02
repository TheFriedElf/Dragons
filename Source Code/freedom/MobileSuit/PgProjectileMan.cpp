#include "stdafx.h"
#include "PgProjectileMan.h"
#include "PgProjectile.h"
#include "PgAction.H"
#include "lohengrin/PacketStruct4Map.h"
#include "PgNetwork.h"
#include "PgPilot.H"
#include "PgPilotMan.H"
#include "PgWorld.h"
#include "PgMobileSuit.h"

PgProjectileMan	g_kProjectileMan;
PgProjectile*	PgProjectileMan::GetProjectile(BM::GUID const &kPilotGUID,int iActionInstanceID,int iUID)
{
	PgProjectile	*pProjectile = NULL;
	for(ProjectileList::iterator itor = m_ProjectileInstanceList.begin(); itor != m_ProjectileInstanceList.end();itor++)
	{
		pProjectile = *itor;
		PG_ASSERT_LOG(pProjectile);

		if(pProjectile)
		{
			if ( pProjectile->GetUID() != iUID ) continue;
			if ( pProjectile->GetParentActionInstanceID() != iActionInstanceID) continue;
			if ( pProjectile->GetParentPilotGUID() != kPilotGUID) continue;

			return pProjectile;
		}
	}
	return	NULL;
}

PgProjectile*	PgProjectileMan::CreateNewProjectile(std::string const& kProjectileID, PgAction *pkParentAction, BM::GUID const &kParentPilotGUID)
{
	const	char	*strProjectileID = (kProjectileID.length() == 0) ? "PROJECTILE_THUNDER_ARROW" : kProjectileID.c_str();

	PgProjectile	*pSource = NULL;
	ProjectileMap::iterator itor = m_ProjectileMap.find(std::string(strProjectileID));
	if(itor == m_ProjectileMap.end())
	{
		//	새로 만든다.
		pSource = (PgProjectile*)PgXmlLoader::CreateObject(strProjectileID);
		if(!pSource) return NULL;

		m_ProjectileMap.insert(std::make_pair(std::string(strProjectileID),pSource));
	}
	else
	{
		pSource = itor->second;
	}

	PgProjectile	*pNew = pSource->CreateClone();
	if(pkParentAction)
	{
		int iParentAction = pkParentAction->GetActionNo();
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const * pkDef = kSkillDefMgr.GetDef(pkParentAction->GetActionNo());
		if(pkDef)
		{
			if(pkDef->GetAbil(AT_PROJECTILE_DAMAGE_ACTION_NO))
			{
				iParentAction = pkDef->GetAbil(AT_PROJECTILE_DAMAGE_ACTION_NO);
			}
		}

		pNew->SetParentActionInfo(iParentAction,pkParentAction->GetActionInstanceID(),pkParentAction->GetTimeStamp());
		pNew->SetParentPilot(kParentPilotGUID);
		pNew->SetUID(pkParentAction->GetNextProjectileUID());

		SProjectileActionReserve kReserved;
		if(FineFromReserveProjectile(pNew, kReserved))	//액션 인스턴스 아이디는 비교하지 않음
		{			
			pkParentAction->SetTargetList(kReserved.kTargetList);
			pNew->SetTargetObjectList(kReserved.kTargetList);
			pNew->ClearTargetList(false);
			if(kReserved.iActionInstanceID==pkParentAction->GetActionInstanceID())	//같으면
			{
				pNew->HomingTime(kReserved.fLifeTime);//살아있을 시간 부여
			}
		}
	}
	m_ProjectileInstanceList.push_back(pNew);

	return	pNew;
}
void	PgProjectileMan::Update(float fAccumeTime,float fFrameTime)
{
	PgProjectile	*pProjectile = NULL;
	for(ProjectileList::iterator itor = m_ProjectileInstanceList.begin(); itor != m_ProjectileInstanceList.end();)
	{
		pProjectile = *itor;
		if(pProjectile->IsDestroyMe())
		{
			itor = m_ProjectileInstanceList.erase(itor);
			continue;
		}
		pProjectile->Update(fAccumeTime,fFrameTime);
		itor++;
	}
}
void	PgProjectileMan::Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	PgProjectile	*pProjectile = NULL;
	for(ProjectileList::iterator itor = m_ProjectileInstanceList.begin(); itor != m_ProjectileInstanceList.end();itor++)
	{
		pProjectile = *itor;
		pProjectile->Draw(pkRenderer,pkCamera,fFrameTime);
	}
}
void PgProjectileMan::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	PgProjectile	*pProjectile = NULL;
	for(ProjectileList::iterator itor = m_ProjectileInstanceList.begin(); itor != m_ProjectileInstanceList.end();itor++)
	{
		pProjectile = *itor;
		pProjectile->DrawImmediate(pkRenderer,pkCamera,fFrameTime);
	}
}

void	PgProjectileMan::DeleteProjectile(PgProjectile *pkProjectile)
{
	PG_ASSERT_LOG(pkProjectile);
	if(pkProjectile == NULL) return;

	PgProjectile	*pProjectile = NULL;
	for(ProjectileList::iterator itor = m_ProjectileInstanceList.begin(); itor != m_ProjectileInstanceList.end();itor++)
	{
		pProjectile = *itor;
		if( pProjectile == pkProjectile )
		{
			pkProjectile->OnRemove();
			return;
		}
	}	
}
void	PgProjectileMan::DeleteProjectile(BM::GUID const &kPilotGUID,int iActionInstanceID,int iUID)
{
	PgProjectile	*pProjectile = NULL;
	for(ProjectileList::iterator itor = m_ProjectileInstanceList.begin(); itor != m_ProjectileInstanceList.end();itor++)
	{
		pProjectile = *itor;
		PG_ASSERT_LOG(pProjectile);

		if(pProjectile)
		{
			if ( pProjectile->GetUID() != iUID ) continue;
			if ( pProjectile->GetParentActionInstanceID() != iActionInstanceID) continue;
			if ( pProjectile->GetParentPilotGUID() != kPilotGUID) continue;

			pProjectile->OnRemove();
			return;
		}
	}	
}

void	PgProjectileMan::Create()
{
}

void PgProjectileMan::Init()
{
}
void	PgProjectileMan::RemoveAll()
{
	m_kReserveList.clear();
	m_ProjectileInstanceList.clear();
}

void	PgProjectileMan::Destroy()
{

	RemoveAll();

	m_ProjectileMap.clear();

}
void	PgProjectileMan::BroadCast_PAction(PgProjectile *pkProjectile,PgActionTargetList *pkTargetList)
{
	if(!g_pkWorld)
	{
		return;
	}
	if(pkProjectile->IsMultipleAttack() == false)
	{
		//	이미 브로드캐스트 했다면, 다시 브로드캐스트 하면 안된다.
		if(pkProjectile->GetTargetListBroadCasted())	
		{
			return;
		}
	}

	BM::Stream kPacket(PT_C_M_REQ_PACTION);
	SProjectileAction	kPAction;

	kPAction.kCasterGuid = pkProjectile->GetParentPilotGUID();
	kPAction.iActionID = pkProjectile->GetParentActionNo();
	kPAction.iActionInstanceID = pkProjectile->GetParentActionInstanceID();
	kPAction.uProjectileUID = static_cast<unsigned int>(pkProjectile->GetUID());
	kPAction.iPenetrationCount = pkProjectile->PenetrationCount();

	_PgOutputDebugString("[PgProjectileMan::BroadCast_PAction] CasterGUID:%s ActionID:%d ActionInstanceID:%d PID:%u\n",	MB(kPAction.kCasterGuid.str()),kPAction.iActionID,kPAction.iActionInstanceID,kPAction.uProjectileUID);

	kPacket.Push(kPAction);

	float const fLifeTime = g_pkWorld->GetAccumTime() - pkProjectile->GetStartTime();
	kPacket.Push(fLifeTime);

	BYTE	byTargetNum = static_cast<BYTE>(pkTargetList->size());
	kPacket.Push(byTargetNum);

	PgActionTargetInfo	*pkTargetInfo = NULL;
	for(ActionTargetList::iterator itor = pkTargetList->begin();itor != pkTargetList->end(); ++itor)
	{
		pkTargetInfo = &(*itor);
		_PgOutputDebugString("[PgProjectileMan::BroadCast_PAction] TargetGUID : %s\n",MB(pkTargetInfo->GetTargetPilotGUID().str()));
		kPacket.Push(pkTargetInfo->GetTargetPilotGUID());
		kPacket.Push(pkTargetInfo->GetSphereIndex());
	}
	
	NETWORK_SEND(kPacket)

	pkProjectile->SetTargetListBroadCasted(true);

#ifndef EXTERNAL_RELEASE
	if(g_pkApp->IsSingleMode())
	{
		pkProjectile->SetTargetObjectList(*pkTargetList);
		pkProjectile->OnArrivedAtTarget();
	}
#endif
}
void	PgProjectileMan::Notify_PAction(BM::Stream *pkPacket)
{
	SProjectileAction	kPAction;
	PgActionTargetList	kTargetList;

	pkPacket->Pop(kPAction);

	kTargetList.SetActionInfo(kPAction.kCasterGuid,kPAction.iActionInstanceID,kPAction.iActionID,0);

	BYTE	byTargetNum = 0;

	PgActionResultVector kActionResultVec;
	kActionResultVec.ReadFromPacket(*pkPacket);
	byTargetNum = kActionResultVec.GetValidCount();	
	
	PgPilot* pkPilot = g_kPilotMan.FindPilot(kPAction.kCasterGuid);

	BM::GUID kTargetGuid;
	for (BYTE byI=0; byI<byTargetNum; ++byI)
	{
		PgActionResult* pkActionResult = kActionResultVec.GetResult(byI, kTargetGuid);
		if (pkActionResult == NULL)
		{
			NILOG(PGLOG_ERROR, "[PgPilot] %s Cannot GetActionResult Actor[%s], Index[%d], ActionResultSize[%d]\n", __FUNCTION__, MB(kPAction.kCasterGuid.str()), (int)byI, (int)byTargetNum);
		//	VERIFY_INFO_LOG(false, BM::LOG_LV5, _T("[%s] Cannot GetActionResult Actor[%s], Index[%d], ActionResultSize[%d]"), __FUNCTIONW__, kPAction.kCasterGuid.str(), (int)byI, (int)byTargetNum);
			break;
		}
		PgActionTargetInfo kTargetInfo;
		kTargetInfo.SetTargetPilotGUID(kTargetGuid);
		kTargetInfo.SetRemainHP(pkActionResult->IsValidHP() ? pkActionResult->GetRemainHP() : INVALID_REMAIN_HP);
		kTargetInfo.SetActionResult(*pkActionResult);
		kTargetInfo.GetActionResult().SetRealEffect(true);
		kTargetInfo.SetSphereIndex(pkActionResult->GetCollision());

		if(kTargetInfo.GetRemainHP() == 0)
		{
			_PgOutputDebugString("[PgProjectileMan::Notify_PAction] RemainHP Is 0, Attacker : %s Target : %s\n", MB(kPAction.kCasterGuid.str()),MB(kTargetInfo.GetTargetPilotGUID().str()));
			PgPilot	*pkTargetPilot = g_kPilotMan.FindPilot(kTargetGuid);
			if(pkTargetPilot)
			{
				PgActor	*pkActor = dynamic_cast<PgActor*>(pkTargetPilot->GetWorldObject());
				if(pkActor)
				{
					//pkActor->ReserveDieByActioin(true);
					_PgOutputDebugString("[PgProjectileMan::Notify_PAction] Die Action Reserved\n");
				}
			}
		}

		if(pkActionResult->IsValidHP() && kTargetInfo.GetRemainHP() <= 0)
		{
			_PgOutputDebugString("[PgProjectileMan::Notify_PAction] RemainHP Is 0, Attacker : %s Target : %s\n", MB(kPAction.kCasterGuid.str()),MB(kTargetInfo.GetTargetPilotGUID().str()));

			kTargetInfo.SetRemainHP((short)NiMax((float)kTargetInfo.GetRemainHP(), 0.0f));
			if(g_kPilotMan.IsMyPlayer(kTargetInfo.GetTargetPilotGUID()) == false )
			{
				kTargetInfo.GetActionResult().ClearEffect();
				kTargetInfo.GetActionResult().AddEffect(ACTIONEFFECT_DIE);	//	DIE EFFECT
				_PgOutputDebugString("[PgProjectileMan::Notify_PAction] ACTIONEFFECT_DIE Added\n");
			}
		}			
		kTargetList.GetList().push_back(kTargetInfo);
	}

	//	발사체를 찾아보자

	PgProjectile	*pkProjectile = GetProjectile(kPAction.kCasterGuid,kPAction.iActionInstanceID,kPAction.uProjectileUID);
	if(!pkProjectile || pkProjectile->IsDestroyMe()) 	//	발사체가 없거나, 제거 예약되어있다면
	{	
		bool bOnlyDie = true;
		if(pkPilot)
		{
			PgActor	*pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
			if(pkActor && !pkProjectile)	//화살이 없을때만 해당한다
			{
				PgAction const *pkActon = pkActor->GetAction();
				if(pkActon)
				{
					if(kPAction.iActionID == pkActon->GetActionNo())
					{
						if(kPAction.iActionInstanceID != pkActon->GetActionInstanceID())
						{
							bOnlyDie = false;
						}
						else	//같으면 예약
						{
							float fLifeTime = 0;
							pkPacket->Pop(fLifeTime);
							ReserveProjectileInfo(kPAction, kTargetList, fLifeTime);
						}
					}
				}
			}
		}
		//	액션 결과를 바로 적용하고 리턴.
		kTargetList.ApplyActionEffects(bOnlyDie);
		return;
	}

	if(0 < kPAction.iPenetrationCount)
	{
		pkProjectile->SetMultipleAttack(true);
	}

	//	발사체가 있다면 
	//	타겟 리스트 교체
	pkProjectile->SetTargetObjectList(kTargetList,true);
	pkProjectile->OnTargetListModified();
}

void	PgProjectileMan::ReserveProjectileInfo(SProjectileAction const& rkAction, PgActionTargetList const& rkTargetList, float const fLifeTime)
{
	if(0==rkAction.iActionID) {return;}
	SProjectileActionReserve kInfo;
	kInfo.clone(rkAction);
	if(!kInfo.IsNull())
	{
		kInfo.fLifeTime = 0.0f==fLifeTime ? 0.1f : fLifeTime;
		kInfo.kTargetList = rkTargetList;
		m_kReserveList.push_back(kInfo);
	}
}

bool	PgProjectileMan::FineFromReserveProjectile(PgProjectile const* pkProj, SProjectileActionReserve& kReserv)
{
	if(!pkProj) {return false;}

	ReserveProjectileList::iterator re_it = m_kReserveList.begin();
	while(re_it!=m_kReserveList.end())
	{
		if((*re_it)==pkProj)
		{
			kReserv = (*re_it);//복사하고 지우자
			m_kReserveList.erase(re_it);
			return true;
		}
		++re_it;
	}

	return false;
}