#include "stdafx.h"
#include "lwActionTargetList.h"
#include "PgAction.H"
#include "lwActionTargetInfo.H"
#include "lwGUID.H"
#include "FreedomPool.h"
#include "lwAction.h"

using namespace lua_tinker;

lwActionTargetList	lwCreateActionTargetList(lwGUID kCasterGUID,int iActionInstanceID,int iActionNo)
{
	PgActionTargetList	*pkList = g_kActionTargetListPool.New();
	pkList->clear();
	pkList->SetActionInfo(kCasterGUID(),iActionInstanceID,iActionNo,0);
	pkList->SetActionEffectApplied(false);
	return	lwActionTargetList(pkList);
}

void lwActionTargetList::Release()
{
	if (m_pkActionTargetList)
	{
		g_kActionTargetListPool.Delete(m_pkActionTargetList);
	}
	m_pkActionTargetList = NULL;
}


void	lwDeleteActionTargetList(lwActionTargetList kList)
{
	kList.Release();
}

lwActionTargetList::~lwActionTargetList()
{
}

void lwActionTargetList::RegisterWrapper(lua_State *pkState)
{
	def(pkState, "CreateActionTargetList", lwCreateActionTargetList);
	def(pkState, "DeleteActionTargetList",lwDeleteActionTargetList);

	LW_REG_CLASS(ActionTargetList)
		LW_REG_METHOD(ActionTargetList, size)
		LW_REG_METHOD(ActionTargetList, clear)
		LW_REG_METHOD(ActionTargetList, AddTarget)
		LW_REG_METHOD(ActionTargetList, GetTargetInfo)
		LW_REG_METHOD(ActionTargetList, IsActionEffectApplied)
		LW_REG_METHOD(ActionTargetList, SetActionEffectApplied)
		LW_REG_METHOD(ActionTargetList, ApplyActionEffects)
		LW_REG_METHOD(ActionTargetList, SwapPosition)
		LW_REG_METHOD(ActionTargetList, DeleteTargetInfo)
		LW_REG_METHOD(ActionTargetList, ApplyOnlyDamage)
		LW_REG_METHOD(ActionTargetList, ApplyActionEffectsTarget)
		LW_REG_METHOD(ActionTargetList, CopyFromActionGUIDCont)
		LW_REG_METHOD(ActionTargetList, CopyToActionGUIDCont)
		LW_REG_METHOD(ActionTargetList, DeleteInActionGUIDCont)
		LW_REG_METHOD(ActionTargetList, AddToActionGUIDCont)
		;
}

void	lwActionTargetList::CopyFromActionGUIDCont(lwAction kAction)
{
	m_pkActionTargetList->clear();

	PgAction::GUIDContainer	*pkCont = kAction()->GetGUIDContainer();

	for(PgAction::GUIDContainer::iterator itor = pkCont->begin();
		itor != pkCont->end();++itor)
	{
		m_pkActionTargetList->GetList().push_back(PgActionTargetInfo(*itor,0));
	}
}
void	lwActionTargetList::CopyToActionGUIDCont(lwAction kAction)
{
	PgAction::GUIDContainer	*pkCont = kAction()->GetGUIDContainer();
	pkCont->clear();

	AddToActionGUIDCont(kAction);

}
void	lwActionTargetList::DeleteInActionGUIDCont(lwAction kAction)
{
	PgAction::GUIDContainer	*pkCont = kAction()->GetGUIDContainer();

	for(PgAction::GUIDContainer::iterator itor = pkCont->begin();
		itor != pkCont->end();++itor)
	{
		DeleteTargetInfoGUID(*itor);
	}
}
void	lwActionTargetList::AddToActionGUIDCont(lwAction kAction)
{
	PgAction::GUIDContainer	*pkCont = kAction()->GetGUIDContainer();

	PgActionTargetInfo	*pkTargetInfo = NULL;
	for(ActionTargetList::iterator itor = m_pkActionTargetList->GetList().begin();
		itor != m_pkActionTargetList->GetList().end();++itor)
	{
	
		pkTargetInfo = &(*itor);
		pkCont->push_back(pkTargetInfo->GetTargetPilotGUID());
	}
}

void	lwActionTargetList::AddTarget(lwActionTargetInfo kActionTargetInfo)
{
	ActionTargetList	&kTargetList = m_pkActionTargetList->GetList();
	PgActionTargetInfo	kInfo;

	kInfo = *(kActionTargetInfo());

	kTargetList.push_back(kInfo);
}
bool	lwActionTargetList::IsActionEffectApplied()
{
	return	m_pkActionTargetList->IsActionEffectApplied();
}
void	lwActionTargetList::SetActionEffectApplied(bool bApplied)
{
	m_pkActionTargetList->SetActionEffectApplied(bApplied);
}
void	lwActionTargetList::ApplyActionEffects(bool bOnlyDieEffect,bool bNoShowDamageNum, float fRandomPosRange)
{
	m_pkActionTargetList->ApplyActionEffects(bOnlyDieEffect,bNoShowDamageNum, fRandomPosRange);
}
void	lwActionTargetList::ApplyActionEffectsTarget(lwGUID kTargetGUID)	//	kTargetGUID 의 액션 이펙트만을 적용한다.
{
	m_pkActionTargetList->ApplyActionEffects(kTargetGUID());
}

void	lwActionTargetList::ApplyOnlyDamage(int iDivide,bool bApplyEffects, float fRandomPosRange)
{
	m_pkActionTargetList->ApplyOnlyDamage(iDivide,bApplyEffects, fRandomPosRange);
}

int	lwActionTargetList::size()
{
	return	m_pkActionTargetList->size();
}

void lwActionTargetList::clear()
{
	return	m_pkActionTargetList->clear();
}
void	lwActionTargetList::SwapPosition(int iIndex,int iIndex2)
{
	PG_ASSERT_LOG(m_pkActionTargetList);
	if (m_pkActionTargetList == NULL)
		return;

	int	iSize = m_pkActionTargetList->size();

	PG_ASSERT_LOG(iIndex>=0 && iIndex<iSize);
	PG_ASSERT_LOG(iIndex2>=0 && iIndex2<iSize);

	if (iIndex == iIndex2 || iIndex < 0 || iIndex >=iSize
		|| iIndex2 < 0 || iIndex2 >=iSize)
	{
		return;
	}

	int iCount = 0;
	PgActionTargetInfo	kInfo[2];
	for(ActionTargetList::iterator itor = m_pkActionTargetList->begin(); itor != m_pkActionTargetList->end(); ++itor)
	{
		if(iCount == iIndex)
			kInfo[0] = *itor;
		if(iCount == iIndex2)
			kInfo[1] = *itor;
		iCount++;
	}	
	iCount = 0;
	for(ActionTargetList::iterator itor = m_pkActionTargetList->begin(); itor != m_pkActionTargetList->end(); ++itor)
	{
		if(iCount == iIndex)
			*itor = kInfo[1];
		if(iCount == iIndex2)
			*itor = kInfo[0];
		iCount++;
	}	
}

lwActionTargetInfo lwActionTargetList::GetTargetInfo(int iIndex)
{
	int iSize = m_pkActionTargetList->size();
	if (iIndex >= 0 && iIndex < iSize)
	{
		int iCount = 0;
		for(ActionTargetList::iterator itor = m_pkActionTargetList->begin(); itor != m_pkActionTargetList->end(); ++itor)
		{
			if(iCount == iIndex)
			{
				return	lwActionTargetInfo(&(*itor));
			}
			iCount++;
		}
	}
	return	lwActionTargetInfo(NULL);
}

void	lwActionTargetList::DeleteTargetInfoGUID(lwGUID kGUID)
{
	PgActionTargetInfo	*pkTargetInfo = NULL;
	for(ActionTargetList::iterator itor = m_pkActionTargetList->begin(); 
		itor != m_pkActionTargetList->end(); ++itor)
	{
		pkTargetInfo = &(*itor);

		if(pkTargetInfo->GetTargetPilotGUID() == kGUID())
		{
			m_pkActionTargetList->GetList().erase(itor);
			return;
		}
	}
}

void lwActionTargetList::DeleteTargetInfo(int iIndex)
{
	int iSize = m_pkActionTargetList->size();
	if (iIndex < 0 || iIndex >= iSize)
		return;
	
	int iCount = 0;
	for(ActionTargetList::iterator itor = m_pkActionTargetList->begin(); itor != m_pkActionTargetList->end(); ++itor)
	{
		if(iCount == iIndex)
		{
			m_pkActionTargetList->GetList().erase(itor);
			return;
		}
		iCount++;
	}
}
