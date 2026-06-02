#include "stdafx.h"
#include "PgBaseItemSet.h"

PgBaseItemSet::PgBaseItemSet(int iItemType, int iGenderLimit, int iSetGroup, int iNbExposedSlot) :
	m_iItemType(iItemType),
	m_iNbExposedSlot(iNbExposedSlot),
	m_iFirstExposedSlot(0),
	m_iSetGroup(iSetGroup),
	m_iGenderLimit(iGenderLimit)
{
	if(iSetGroup != 0)
	{
		// 현재 우리 게임에서 쓰는 Set들은 총 6개이다.
		m_kBaseItemList.reserve(6);
	}
}

PgBaseItemSet::~PgBaseItemSet()
{
	BaseItemList::iterator itr = m_kBaseItemList.begin();
	while(itr != m_kBaseItemList.end())
	{
		SAFE_DELETE(*itr);
		++itr;
	}
	m_kBaseItemList.clear();
}

void PgBaseItemSet::AddItem(PgBaseItem *pkBaseItem)
{
	m_kBaseItemList.push_back(pkBaseItem);
}

PgBaseItemSet::PgBaseItem *PgBaseItemSet::GetItem(int iPos)
{
	if((int)m_kBaseItemList.size() <= iPos)
	{
		return 0;
	}
	return m_kBaseItemList.at(iPos);
}

void PgBaseItemSet::SetExposedSlot(int iNbExposedSlot)
{
	m_iNbExposedSlot = iNbExposedSlot;
}

int PgBaseItemSet::GetSize()
{
	return m_kBaseItemList.size();
}

int PgBaseItemSet::GetType()
{
	return m_iItemType;
}

int PgBaseItemSet::GetSetGroup()
{
	return m_iSetGroup;
}

int PgBaseItemSet::GetGenderLimit()
{
	return m_iGenderLimit;
}

PgBaseItemSet::PgBaseItem *PgBaseItemSet::GetItemSet(int iClassNo, int iSetNo)
{
	BaseItemList::iterator itr = m_kBaseItemList.begin();
	while(itr != m_kBaseItemList.end())
	{
		PgBaseItem *pkItem = *itr;
		if(pkItem && pkItem->GetClassNo() == iClassNo && 
			pkItem->GetSetNo() == iSetNo)
		{
			return pkItem;
		}
		++itr;
	}
	return 0;
}

PgBaseItemSet::PgBaseItem *PgBaseItemSet::GetBaseItem(int iExposedSlot)
{
	if(iExposedSlot >= m_iNbExposedSlot)
	{
		return 0;
	}

	int iPos = m_iFirstExposedSlot + iExposedSlot;
	iPos = iPos % m_kBaseItemList.size();

	return m_kBaseItemList.at(iPos);
}

void PgBaseItemSet::NextSlot()
{
	++m_iFirstExposedSlot;
	m_iFirstExposedSlot = m_iFirstExposedSlot % m_kBaseItemList.size();
}

void PgBaseItemSet::PrevSlot()
{
	--m_iFirstExposedSlot;
	if(m_iFirstExposedSlot < 0)
	{
		m_iFirstExposedSlot = m_kBaseItemList.size()-1;
	}
}

void PgBaseItemSet::SetFirstExposedSlot(int iFirstExposedSlot)
{
	m_iFirstExposedSlot = iFirstExposedSlot;
}


//----------------------------------------------------------------------------
// PgBaseItem
//----------------------------------------------------------------------------

PgBaseItemSet::PgBaseItem::PgBaseItem(std::string &rkIconPath, int iClassNo, int iSetNo) :
	m_kIconPath(rkIconPath),
	m_iClassNo(iClassNo),
	m_iSetNo(iSetNo)
{
}

PgBaseItemSet::PgBaseItem::~PgBaseItem()
{
	m_kItemNoContainer.clear();
}

void PgBaseItemSet::PgBaseItem::AddItem(int iInfo)
{
	m_kItemNoContainer.push_back(iInfo);
}

std::string &PgBaseItemSet::PgBaseItem::GetIconPath()
{
	return m_kIconPath;
}

int PgBaseItemSet::PgBaseItem::GetItemNo(int iPos)
{
	if((int)m_kItemNoContainer.size() <= iPos )
	{
		return 0;
	}

	return m_kItemNoContainer.at(iPos);
}

int PgBaseItemSet::PgBaseItem::GetNbItemNo()
{
	return m_kItemNoContainer.size();
}

int PgBaseItemSet::PgBaseItem::GetSetNo()
{
	return m_iSetNo;
}

int PgBaseItemSet::PgBaseItem::GetClassNo()
{
	return m_iClassNo;
}
