#include "StdAfx.h"
#include "variant/item.h"
#include "PgMissionCard.h"
#include "PgQuestMan.h"

PgMissionCard::PgMissionCard(void)
{
}

PgMissionCard::~PgMissionCard(void)
{
}

void PgMissionCard::MQCardList_Clear()
{
	m_MQCardList.clear();
}

void PgMissionCard::MQCardList_Insert(ContHaveItemNoCount const &List)
{
	m_MQCardList.clear();
	m_MQCardList.insert(List.begin(), List.end());
}

int const PgMissionCard::MQCardList_SelectCard(int const iCnt) const
{
	size_t	iCount = 0;

	ContHaveItemNoCount::const_iterator	iter = m_MQCardList.begin();
	while( iter != m_MQCardList.end() )
	{
		iCount += iter->second;

		if( iCnt <= iCount )
		{
			return iter->first;
		}

		++iter;
	}
	return 0;
}

size_t const PgMissionCard::MQCardList_GetCardCount() const
{
	size_t	iCount = 0;

	ContHaveItemNoCount::const_iterator	iter = m_MQCardList.begin();
	while( iter != m_MQCardList.end() )
	{
		iCount += iter->second;
		++iter;
	}
	return	iCount;
}
