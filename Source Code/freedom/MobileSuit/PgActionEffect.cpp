#include "stdafx.h"
#include "PgActionEffect.H"
#include "lwActor.H"
#include "lwGUID.H"
#include "PgActor.H"
#include "PgAction.H"
#include "PgActionPool.H"
#include "PgPilot.H"
///////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionEffect
///////////////////////////////////////////////////////////////////////////////////////////////////////

PgActionEffect::PgActionEffect(PgActor *pkParentActor,BM::GUID const &kCasterGUID,int iActionInstanceID,int iActionID,bool bIsTemporaryEffect,DWORD dwTimeStamp)
{
	m_pkParentActor = pkParentActor;
	m_bIsTemporaryEffect = bIsTemporaryEffect;
	m_dwTimeStamp = dwTimeStamp;
	m_iActionInstanceID = iActionInstanceID;
	m_kCasterGUID = kCasterGUID;
	m_iActionID = iActionID;
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	m_kActionID = MB(kSkillDefMgr.GetActionName(iActionID));
	m_bDoNotTransitToLastAction = false;
}
PgActionEffect::~PgActionEffect()
{
}
void	PgActionEffect::SetToRealEffect(DWORD dwTimeStamp)
{
	m_bIsTemporaryEffect = false;
	m_dwTimeStamp = dwTimeStamp;
}
void	PgActionEffect::OnTopLevelEnter()
{
}
void	PgActionEffect::OnTopLevelLeave()
{
}
void	PgActionEffect::OnEnter()
{
	//m_pkParentActor->TransitAction(m_kActionID.c_str());
	PgAction *pkAction = m_pkParentActor->ReserveTransitAction(m_kActionID.c_str());
	if(pkAction)
	{
		pkAction->SetEffectCasterGuid(m_kCasterGUID);
	}
}
void	PgActionEffect::OnLeave()
{
}
bool	PgActionEffect::OnUpdate(float fAccumTime,float fFrameTime)
{
	if(m_pkParentActor->GetAction() && m_pkParentActor->GetAction()->GetActionNo() == m_iActionID)
	{
		return	m_pkParentActor->GetAction()->GetActionFinished();
	}
	return	true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgActionEffectStack
///////////////////////////////////////////////////////////////////////////////////////////////////////

PgActionEffectStack::PgActionEffectStack(PgActor *pkParentActor)
{
	m_pkParentActor = pkParentActor;
	m_pkSavedLastAction = NULL;
}
PgActionEffectStack::~PgActionEffectStack()
{
	ClearAll();
}
bool	PgActionEffectStack::IsTopLevelActionEffect(PgAction *pkAction)
{
	if(m_kActionEffectStack.size() == 0) return false;
             
	return	false;
}

void	PgActionEffectStack::AddNewEffect(BM::GUID const &kCasterGUID,int iActionInstanceID,bool bIsTemporaryEffect,int iEffectID,DWORD dwTimeStamp)
{
	PgActionEffect	*pNewEffect = NULL;

	//_PgOutputDebugString("PgActionEffectStack::AddNewEffect iEffectID : %d CasterGUID : %s ActorGUID : %s ActorName : %s\n",iEffectID,MB(kCasterGUID.str()),MB(m_pkParentActor->GetPilot()->GetGuid().str()),MB(m_pkParentActor->GetPilot()->GetName()));
	PG_ASSERT_LOG(m_pkParentActor);
	PG_ASSERT_LOG(m_pkParentActor->GetPilot());

	NILOG(PGLOG_LOG,"PgActionEffectStack::AddNewEffect iEffectID : %d CasterGUID : %s ActorGUID : %s ActorName : %s\n",iEffectID,MB(kCasterGUID.str()),MB(m_pkParentActor->GetPilot()->GetGuid().str()),MB(m_pkParentActor->GetPilot()->GetName()));

	if(iEffectID > 0) 
	{
		ReleaseAllEffect(ACTIONEFFECT_DMG,false);	//	기본 대미지 이펙트를 모두 제거한다.
		ReleaseAllEffect(iEffectID,false);	//	같은 이펙트 제거
	}

	if(iEffectID == ACTIONEFFECT_DMG)
	{
		if(m_kActionEffectStack.size()>0) return;	//	기본 대미지 이펙트는 우선순위가 최하위이다.
	}

	if(iEffectID != ACTIONEFFECT_RESURRECTION01 && iEffectID != ACTIONEFFECT_REVIVE)
	{
		//	죽기 이펙트가 존재하는지 찾아보자.
		if(GetActionEffect(ACTIONEFFECT_DIE) || GetActionEffect(ACTIONEFFECT_VOLUNTARILY_DIE))
		{
			//	존재한다면 리턴!
			return;
		}
	}

	bool	bClearAll = (iEffectID == ACTIONEFFECT_DIE || 
						iEffectID == ACTIONEFFECT_VOLUNTARILY_DIE || 
						iEffectID == ACTIONEFFECT_REVIVE || 
						iEffectID == ACTIONEFFECT_RESURRECTION01
						|| ACTIONEFFECT_BLOCK == iEffectID
						);
//	if(bClearAll)
	{
		ClearAll();
	}	

	//	이미 존재하는 이펙트인지 찾아보자.
	pNewEffect = GetActionEffect(kCasterGUID,iActionInstanceID);
	
	if(pNewEffect )	//	이미 존재한다면,
	{
		if(iEffectID == 0)	//	지워야한다.
		{
			ReleaseEffect(pNewEffect);
			return;
		}
		if(bIsTemporaryEffect == false)
		{
			//	진짜 이펙트로 승격시킨다.
			SetToRealEffect(pNewEffect,dwTimeStamp);
		}

		return;
	}

	pNewEffect = new PgActionEffect(m_pkParentActor,kCasterGUID,iActionInstanceID,iEffectID,bIsTemporaryEffect,dwTimeStamp);
	if(iEffectID == ACTIONEFFECT_DIE || iEffectID == ACTIONEFFECT_VOLUNTARILY_DIE)
	{
		pNewEffect->DoNotTransitToLastAction(true);
	}

	if(bIsTemporaryEffect)
	{
		//	맨 상위에 추가한다.
		m_kActionEffectStack.push_front(pNewEffect);
		pNewEffect->OnEnter();
	}
	else
	{
		//	진짜 이펙트라면,
		m_kActionEffectStack.push_back(pNewEffect);
		if(m_kActionEffectStack.size() == 1)
		{
			pNewEffect->OnEnter();
		}
		SetToRealEffect(pNewEffect,dwTimeStamp);
	}
}
bool	PgActionEffectStack::SaveLastAction(PgAction *pkAction)
{
	//if(pkAction)
	//{
	//	_PgOutputDebugString("Actor GUID:%s Name:%s SaveLastAction : %s\n",MB(m_pkParentActor->GetPilot()->GetGuid().str()),MB(m_pkParentActor->GetPilot()->GetName()),pkAction->GetID().c_str());
	//}
	//else
		//_PgOutputDebugString("Actor GUID:%s Name:%s SaveLastAction To Null\n",MB(m_pkParentActor->GetPilot()->GetGuid().str()),MB(m_pkParentActor->GetPilot()->GetName()));

	if(m_pkSavedLastAction)
	{
		if(!m_pkParentActor->IsUnderMyControl() && m_pkSavedLastAction->GetActionStartPos() != NiPoint3::ZERO && pkAction->GetActionStartPos() == NiPoint3::ZERO)
		{
			return	false;
		}
		g_kActionPool.ReleaseAction(m_pkSavedLastAction);
	}

	m_pkSavedLastAction = pkAction;
	return	true;
}
void	PgActionEffectStack::ReleaseAllEffect(int iEffectID,bool bDoSavedAction)	//	iEffectID 에 해당하는 모든 이펙트를 제거한다.
{
	bool	bIsTopLevel = true;
	bool	bIsTopLevelDeleted = false;
	bool	bDoNotTransitToLastAction = false;
	for(ActionEffectList::iterator itor = m_kActionEffectStack.begin(); itor != m_kActionEffectStack.end();)
	{
		if((*itor)->GetActionID() == iEffectID)
		{
			if(bIsTopLevel)
			{
				bIsTopLevelDeleted = true;
			}

			if(!bDoNotTransitToLastAction)
				bDoNotTransitToLastAction = (*itor)->DoNotTransitToLastAction();

			SAFE_DELETE(*itor);
			itor = m_kActionEffectStack.erase(itor);
			bIsTopLevel = false;
			continue;
		}
		itor++;
		bIsTopLevel = false;
	}

	if(bIsTopLevelDeleted)
	{
		if(m_kActionEffectStack.size() == 0)	//	비어있다면,
		{
			if(bDoNotTransitToLastAction == false)
			{
				if(bDoSavedAction)
					DoSavedAction();
			}
		}
		else
		{
			PgActionEffect	*pkNextActionEffect = *m_kActionEffectStack.begin();
			pkNextActionEffect->OnEnter();
		}
	}
}

void	PgActionEffectStack::SetToRealEffect(PgActionEffect	*pkActionEffect,DWORD dwTimeStamp)
{
	pkActionEffect->SetToRealEffect(dwTimeStamp);

	//	액션이펙트 리스트에 자신을 제외한 다른 이펙트들 중, 리얼이펙트가 있는지 찾아본다.

	PgActionEffect*	pkRealEffect=NULL;
	for(ActionEffectList::iterator itor = m_kActionEffectStack.begin(); itor != m_kActionEffectStack.end(); itor++)
	{
		if( (*itor) != pkActionEffect && (*itor)->IsTemporaryEffect() == false)
		{
			pkRealEffect = *itor;
			break;
		}
	}	

	if(pkRealEffect)
	{
		//	다른 리얼 이펙트가 존재한다면,
		//	둘중에 TimeStamp 가 큰 쪽이 제거된다.
		if(pkRealEffect->GetTimeStamp()>dwTimeStamp)
			ReleaseEffect(pkRealEffect);
		else
			ReleaseEffect(pkActionEffect);
	}
}
void	PgActionEffectStack::ReleaseEffect(PgActionEffect *pkActionEffect)
{
	bool	bIsTopLevel = true;
	bool	bIsEmpty = m_kActionEffectStack.size() == 0;
	bool	bDoNotTransitToLastAction = false;
	for(ActionEffectList::iterator itor = m_kActionEffectStack.begin(); itor != m_kActionEffectStack.end(); itor++)
	{
		if( (*itor) == pkActionEffect)
		{
			if(!bDoNotTransitToLastAction)
				bDoNotTransitToLastAction = (*itor)->DoNotTransitToLastAction();

			m_kActionEffectStack.erase(itor);

			SAFE_DELETE(pkActionEffect);
			if(bIsTopLevel)
			{
				//	지워지는 이펙트가 최상위 이펙트라면,
				//	그 다음 위치에 있는 이펙트를 최상위 이펙트로 만들어준다.
				if(m_kActionEffectStack.size()>0)
				{
					PgActionEffect	*pkNextActionEffect = *m_kActionEffectStack.begin();
					pkNextActionEffect->OnEnter();
				}
			}

			break;
		}

		bIsTopLevel = false;
	}		

	if(bIsEmpty == false && m_kActionEffectStack.size() == 0 && bDoNotTransitToLastAction == false)	//	비어있다면,
	{
		DoSavedAction();
	}
}
PgActionEffect*	PgActionEffectStack::GetActionEffect(int iEffectID)
{
	for(ActionEffectList::iterator itor = m_kActionEffectStack.begin(); itor != m_kActionEffectStack.end(); itor++)
	{
		if( (*(itor))->GetActionID() == iEffectID)
		{
			return	*itor;
		}
	}
	return	NULL;
}

PgActionEffect*	PgActionEffectStack::GetActionEffect(BM::GUID const &kCasterGUID,int iActionInstanceID)
{
	for(ActionEffectList::iterator itor = m_kActionEffectStack.begin(); itor != m_kActionEffectStack.end(); itor++)
	{
		if( (*(itor))->GetCasterGUID() == kCasterGUID && (*(itor))->GetActionInstanceID() == iActionInstanceID)
		{
			return	*itor;
		}
	}
	return	NULL;
}

void	PgActionEffectStack::AddToTopLevel(PgActionEffect	*pkActionEffect)
{
	PgActionEffect	*pkCurrentTopLevel = NULL;
	pkActionEffect->OnEnter();
	m_kActionEffectStack.push_front(pkActionEffect);
}

void	PgActionEffectStack::ClearAll()
{
	for(ActionEffectList::iterator itor = m_kActionEffectStack.begin(); itor != m_kActionEffectStack.end(); itor++)
	{
		SAFE_DELETE(*itor);
	}

	m_kActionEffectStack.clear();

	if(m_pkSavedLastAction)
	{
		g_kActionPool.ReleaseAction(m_pkSavedLastAction);
	}
	m_pkSavedLastAction = NULL;
}

void	PgActionEffectStack::Update(float fAccumTime,float fFrameTime)
{
	bool	bIsEmpty = m_kActionEffectStack.size() == 0;
	bool	bDoNotTransitToLastAction = false;
	bool	bIsTopLevel = false;
	PgActionEffect	*pkActionEffect;
	for(ActionEffectList::iterator itor = m_kActionEffectStack.begin(); itor != m_kActionEffectStack.end(); )
	{
		pkActionEffect = *itor;
		if(pkActionEffect->OnUpdate(fAccumTime,fFrameTime))
		{
			bIsTopLevel = false;
			if (itor == m_kActionEffectStack.begin())
			{
				bIsTopLevel = true;
			}

			if(!bDoNotTransitToLastAction)
				bDoNotTransitToLastAction = pkActionEffect->DoNotTransitToLastAction();

			SAFE_DELETE(pkActionEffect);
			
			itor = m_kActionEffectStack.erase(itor);
			PgActionEffect	*pkNextActionEffect = NULL;
			
			if(m_kActionEffectStack.size()>0)
			{
				pkNextActionEffect = *m_kActionEffectStack.begin();
				pkNextActionEffect->OnEnter();
			}
			continue;
		}

		itor++;
	}	

	if(bIsEmpty == false && m_kActionEffectStack.size() == 0 && bDoNotTransitToLastAction == false)	//	비어있다면,
	{
		DoSavedAction();
	}
}
void	PgActionEffectStack::DoSavedAction()
{
	if(m_pkSavedLastAction)
	{
		//_PgOutputDebugString("Actor GUID:%s Name:%s Do Saved Action : %s\n",MB(m_pkParentActor->GetPilot()->GetGuid().str()),MB(m_pkParentActor->GetPilot()->GetName()),m_pkSavedLastAction->GetID().c_str());
		m_pkSavedLastAction->SetActionStartPos(m_pkParentActor->GetPosition());
		m_pkParentActor->ReserveTransitAction(m_pkSavedLastAction);
		m_pkSavedLastAction = NULL;

		//if(m_pkParentActor->Action(m_pkSavedLastAction)==false)
		//{
		//	//_PgOutputDebugString("Actor GUID:%s Name:%s Do Saved Action : %s Failed \n",MB(m_pkParentActor->GetPilot()->GetGuid().str()),MB(m_pkParentActor->GetPilot()->GetName()),m_pkSavedLastAction->GetID().c_str());
		//	g_kActionPool.ReleaseAction(m_pkSavedLastAction);
		//}
		////else
		//	//_PgOutputDebugString("Actor GUID:%s Name:%s Do Saved Action : %s Success \n",MB(m_pkParentActor->GetPilot()->GetGuid().str()),MB(m_pkParentActor->GetPilot()->GetName()),m_pkSavedLastAction->GetID().c_str());
		//m_pkSavedLastAction = NULL;
	}
	else
	{
			//_PgOutputDebugString("Actor GUID:%s Name:%s Do Idle Action Instead SavedAction \n",MB(m_pkParentActor->GetPilot()->GetGuid().str()),MB(m_pkParentActor->GetPilot()->GetName()));
		//	액터의 액션을 기본 액션으로 처리해주자.
		PgAction* pkCurAction = m_pkParentActor->GetAction();
		if(pkCurAction)
		{
			std::string const kNextActionName = pkCurAction->GetNextActionName();
			if( kNextActionName.empty() )
			{
				m_pkParentActor->DoReservedAction(PgActor::RA_IDLE);
			}
		}
	}
}
