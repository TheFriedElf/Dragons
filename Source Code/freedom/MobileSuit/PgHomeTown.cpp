#include "stdafx.h"
#include "PgHomeTown.h"
#include "ServerLib.h"

void PgHomeTown::Build()
{
	//const CONT_DEFMAP* pContDefMap = NULL;
	//g_kTblDataMgr.GetContDef(pContDefMap);
	//m_kHomeTownList.clear();
	//if(pContDefMap)
	//{
	//	CONT_DEFMAP::const_iterator it = pContDefMap->begin();
	//	while(pContDefMap->end() != it)
	//	{
	//		TBL_DEF_MAP const * pkDef = &((*it).second);
	//		if(pkDef && pkDef->sHometownNo)
	//		{
	//			m_kHomeTownList.push_back(pkDef);
	//			m_kTownNo.insert(pkDef->sHometownNo);
	//		}
	//		++it;
	//	}
	//}

	m_kHomeTownList.clear();
	CONT_DEFMYHOMEBUILDINGS const * pkCont = NULL;
	g_kTblDataMgr.GetContDef(pkCont);
	if(pkCont)
	{
		CONT_DEFMYHOMEBUILDINGS::const_iterator itr = pkCont->begin();
		for( ; itr != pkCont->end(); ++itr )
		{
			if( 0 != itr->second.iGrade )
			{
				m_kTownNo.insert(itr->second.iBuildingNo);
			}
		}
	}
}

void PgHomeTown::GetFirstHomeTown(HomeTownList::const_iterator & rkIter)
{
	rkIter = m_kHomeTownList.begin();
}

TBL_DEF_MAP const * PgHomeTown::GetNextHomeTown(HomeTownList::const_iterator & rkIter) const
{
	TBL_DEF_MAP const * pkDef = NULL;
	while(m_kHomeTownList.end()!=rkIter && NULL==pkDef)
	{
		pkDef = (*rkIter);
		++rkIter;
	}

	return pkDef;
}

int PgHomeTown::GetHomeTownMapNoAt(size_t const iAt) const
{
	if(0==GetHomeTownCount() || GetHomeTownCount() <= iAt)
	{
		return 0;
	}

	TBL_DEF_MAP const* pkDef = m_kHomeTownList.at(iAt);
	if(!pkDef)
	{
		return 0;
	}

	return pkDef->iMapNo;
}