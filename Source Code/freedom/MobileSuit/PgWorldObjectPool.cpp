#include "stdafx.h"
#include "PgWorldObjectPool.H"
#include "PgIWorldObject.H"

PgWorldObjectPool	g_kWorldObjectPool;

void	PgWorldObjectPool::Init()
{
}
void	PgWorldObjectPool::Terminate()
{
	BM::CAutoMutex kLock(m_kPoolLock);
	for(PoolDataCont::iterator itor_1 = m_kPoolDataCont.begin(); itor_1 != m_kPoolDataCont.end();itor_1++ )
	{
		PoolDataList	&kPoolDataList = itor_1->second;
		for(PoolDataList::iterator itor_2 = kPoolDataList.begin(); itor_2 != kPoolDataList.end(); itor_2 ++)
		{
			stPoolData	*pkData = *itor_2;
			if(pkData)
			{
				ObjectList &kObjectList = pkData->m_kObjectList;
				for(ObjectList::iterator itor_3 = kObjectList.begin(); itor_3 != kObjectList.end(); itor_3++)
				{
					PgIWorldObject *pkObject = *itor_3;
					if(pkObject)
						NiDelete pkObject;
					pkObject = NULL;
				}

				delete pkData;
			}
		}
	}

	m_kPoolDataCont.clear();
}

PgIWorldObject*	PgWorldObjectPool::CreateObject(std::string const &kObjectID,
	PgIXmlObject::XmlObjectID kObjectTypeID,
	char const *pcObjectTypeStr)
{
	BM::CAutoMutex kLock(m_kPoolLock);
	if(kObjectID.empty())
	{
		return	NULL;
	}

	std::string kObjectTypeStr = "";
	if(pcObjectTypeStr)
	{
		kObjectTypeStr = pcObjectTypeStr;
	}

	PgIWorldObject	*pkObject = NULL;


	char	const *pkObjectTypeStr = kObjectTypeStr.empty() ? NULL : kObjectTypeStr.c_str();
	pkObject = dynamic_cast<PgIWorldObject *>(PgXmlLoader::CreateObject(kObjectID.c_str(), (void*)&kObjectTypeID, NULL, pkObjectTypeStr));

	return	pkObject;


	stPoolData	*pkPoolData = GetPoolData(kObjectID,
		kObjectTypeID,
		kObjectTypeStr);

	if(!pkPoolData)
	{
		pkPoolData = AddPoolData(kObjectID,kObjectTypeID,kObjectTypeStr);
		if(!pkPoolData)
		{
			return	NULL;
		}
	}

	ObjectList &kList = pkPoolData->m_kObjectList;

	PgIWorldObject *pkWorldObject = NULL;
	if(kList.size() == 0)
	{
		char	const *pkObjectTypeStr = kObjectTypeStr.empty() ? NULL : kObjectTypeStr.c_str();
		pkWorldObject = dynamic_cast<PgIWorldObject *>(PgXmlLoader::CreateObject(kObjectID.c_str(), (void*)&kObjectTypeID, NULL, pkObjectTypeStr));
		if(!pkWorldObject)
		{
			return	NULL;
		}

		kList.push_back(pkWorldObject);
	}
	else
	{
		pkWorldObject = *kList.begin();
	}
	if(!pkWorldObject)
	{
		return	NULL;
	}
	pkObject = pkWorldObject->CreateCopy();
	if(pkObject)
	{
		pkObject->SetID(pkWorldObject->GetID().c_str());
	}
	else
	{
		char	const *pkObjectTypeStr = kObjectTypeStr.empty() ? NULL : kObjectTypeStr.c_str();
		pkObject = dynamic_cast<PgIWorldObject *>(PgXmlLoader::CreateObject(kObjectID.c_str(), (void*)&kObjectTypeID, NULL, pkObjectTypeStr));
	}

	return	pkObject;
}
PgWorldObjectPool::stPoolData*	PgWorldObjectPool::AddPoolData(std::string const &kObjectID,
	PgIXmlObject::XmlObjectID kObjectTypeID,
	std::string const &kObjectTypeStr)
{
	stPoolData	*pkNewPoolData = new stPoolData(kObjectID,kObjectTypeID,kObjectTypeStr);

	PoolDataCont::iterator itor = m_kPoolDataCont.find(kObjectID);
	if(itor == m_kPoolDataCont.end())
	{
		PoolDataList kNewList;

		kNewList.push_back(pkNewPoolData);

		m_kPoolDataCont.insert(std::make_pair(kObjectID,kNewList));

		return	pkNewPoolData;

	}

	PoolDataList	&kPoolDataList = itor->second;

	kPoolDataList.push_back(pkNewPoolData);
	return	pkNewPoolData;
}

PgWorldObjectPool::stPoolData*	PgWorldObjectPool::GetPoolData(std::string const &kObjectID,
	PgIXmlObject::XmlObjectID kObjectTypeID,
	std::string const &kObjectTypeStr) const
{
	PoolDataCont::const_iterator itor = m_kPoolDataCont.find(kObjectID);
	if(itor == m_kPoolDataCont.end())
	{
		return	NULL;
	}

	PoolDataList const &kDataList = itor->second;

	for(PoolDataList::const_iterator itor = kDataList.begin(); itor != kDataList.end(); itor++)
	{
		stPoolData	*pkData = *itor;
		if(pkData->m_kObjectTypeID == kObjectTypeID &&
			pkData->m_kObjectTypeStr == kObjectTypeStr)
		{
			return	pkData;
		}
	}

	return	NULL;
}
