#include "stdafx.h"
#include "PgIInputManager.h"
#include "PgIInputObserver.h"
#include "PgRenderMan.H"

PgIInputManager::ObserverContainer PgIInputManager::m_kObserverContainer;
PgIInputManager::ObserverContainer PgIInputManager::m_kWaitObserverContainer;
PgIInputManager::ObserverContainer PgIInputManager::m_kRemoveObserverContainer;
   
bool PgIInputManager::CompareObserver(PgIInputObserver *lhs, PgIInputObserver *rhs)
{
	return lhs->GetInputPriority() < rhs->GetInputPriority();     
} 
 
bool PgIInputManager::AddInputObserver(PgIInputObserver *pkIInputObserver) 
{
	PG_ASSERT_LOG(pkIInputObserver);
	if (pkIInputObserver == NULL)
	{
		return false;
	}

	NILOG(PGLOG_LOG, "[PgIInputManager] AddInputObserver %#X, (%d)\n", pkIInputObserver, m_kWaitObserverContainer.size());

	//	We should remove the observer which reside in the m_kRemoveObserverContainer from the container.
	ObserverContainer::iterator itor = std::find(m_kRemoveObserverContainer.begin(),m_kRemoveObserverContainer.end(),pkIInputObserver);
	if(itor != m_kRemoveObserverContainer.end())
	{
		m_kRemoveObserverContainer.erase(itor);
	}

	//	We don't need to add the observer if it is already in the m_kObserverContainer
	itor = std::find(m_kObserverContainer.begin(),m_kObserverContainer.end(),pkIInputObserver);
	if(itor != m_kObserverContainer.end())
	{
		return false;
	}
  
	m_kWaitObserverContainer.push_back(pkIInputObserver);
	 
	return true;
}

bool PgIInputManager::RemoveInputObserver(PgIInputObserver *pkIInputObserver)
{
	PG_ASSERT_LOG(pkIInputObserver);
	if (pkIInputObserver == NULL)
	{
		return false;
	}

	NILOG(PGLOG_LOG, "[PgIInputManager] RemoveInputObserver %#X, (%d)\n", pkIInputObserver, m_kRemoveObserverContainer.size());

	ObserverContainer::iterator itor = std::find(m_kWaitObserverContainer.begin(),m_kWaitObserverContainer.end(),pkIInputObserver);
	if(itor != m_kWaitObserverContainer.end())
	{
		NILOG(PGLOG_LOG, "[PgIInputManager] RemoveInputObserver from WaitObserver %#X\n", pkIInputObserver);
		m_kWaitObserverContainer.erase(itor);
	}

	itor = std::find(m_kObserverContainer.begin(),m_kObserverContainer.end(),pkIInputObserver);
	if(itor == m_kObserverContainer.end())
	{
		return	false;
	}

	m_kRemoveObserverContainer.push_back(pkIInputObserver);
	
	return true;
}

void PgIInputManager::ClearInputObserver()
{
	NILOG(PGLOG_LOG, "[PgIInputManager] ClearInputObserver\n");
	m_kWaitObserverContainer.clear();
	m_kRemoveObserverContainer.clear();
	m_kObserverContainer.clear();
}

PgIInputObserver* PgIInputManager::GetInputObserver(unsigned int uiIndex)
{
	// 무슨 의미가 있을까나..
	return m_kObserverContainer.at(uiIndex);
}

bool PgIInputManager::InvalidateInputObserverContainer()
{
	if(m_kRemoveObserverContainer.size())
	{
		NILOG(PGLOG_LOG, "[PgIInputManager] Remove %d Observer\n", m_kRemoveObserverContainer.size());
		for(ObserverContainer::iterator itr = m_kRemoveObserverContainer.begin();
			itr != m_kRemoveObserverContainer.end();
			++itr)
		{
			ObserverContainer::iterator itrFind = std::find(m_kObserverContainer.begin(), m_kObserverContainer.end(), *itr);
			if(itrFind != m_kObserverContainer.end())
			{
				NILOG(PGLOG_LOG, "[PgIInputManager] Remove %#X Observer\n", *itrFind);
				m_kObserverContainer.erase(itrFind);
			}
		}

		sort(m_kObserverContainer.begin(), m_kObserverContainer.end(), PgIInputManager::CompareObserver);
		m_kRemoveObserverContainer.clear();
		NILOG(PGLOG_LOG, "[PgIInputManager] After Remove %d observer remains\n", m_kObserverContainer.size());
	}

	//	leesg213	먼저 지울 것을 지우고, 추가하도록 순서를 바꿈. 2007-02-14(발렌타인데이의 밤 9시 회사에서.........)
	if(m_kWaitObserverContainer.size())
	{
		NILOG(PGLOG_LOG, "[PgIInputManager] Add %d Observer\n", m_kWaitObserverContainer.size());
		for(ObserverContainer::iterator itr = m_kWaitObserverContainer.begin();
			itr != m_kWaitObserverContainer.end();
			++itr)
		{
			bool bFoundSameOb = false;
			for(ObserverContainer::iterator itr2 = m_kObserverContainer.begin();
				itr2 != m_kObserverContainer.end();
				++itr2)
			{
				if (*itr2 == *itr)
				{
					bFoundSameOb = true;
					break;
				}
			}

			if (bFoundSameOb == false)
			{
				m_kObserverContainer.push_back(*itr);
			}
			else
			{
				NILOG(PGLOG_LOG, "[PgIInputManager] Add %#X Observer, but Already inserted\n", *itr);
			}
		}

		sort(m_kObserverContainer.begin(), m_kObserverContainer.end(), PgIInputManager::CompareObserver);
		m_kWaitObserverContainer.clear();
		NILOG(PGLOG_LOG, "[PgIInputManager] After Add %d observer exists\n", m_kObserverContainer.size());
	}

	return true;
}

bool PgIInputManager::TransferToObserver(PgInput &rkInput)
{

	// 발생한 Input을 Priority의 순서에 따라 처리함
	ObserverContainer::iterator itr = m_kObserverContainer.begin();
	PgIInputObserver	*pObserver = NULL;

	static	int	i = 0,iTotal = 0;
	
	iTotal = m_kObserverContainer.size();
	for(i=0;i<iTotal;i++)
	{
		pObserver = m_kObserverContainer[i];
		if (pObserver && pObserver->ProcessInput(&rkInput))
		{
			return true;
		}
	}

	return false;
}
