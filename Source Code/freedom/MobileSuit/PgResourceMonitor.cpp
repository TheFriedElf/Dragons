#include "StdAfx.h"
#include "PgResourceMonitor.h"

PgResourceMonitor g_kResourceMonitor;

char const* g_pcResourceTypeName[PgResourceMonitor::MAX_RESOURCE_TYPE_NUM] =
{
	"GENERAL",
	"NIF",
	"TEXTURE",
	"PARTICLE",
	"AUDIO",
	"ACTION",
};

PgResourceMonitor::PgResourceMonitor()
{

}

PgResourceMonitor::~PgResourceMonitor()
{
	m_kResourceMap.clear();
}

void PgResourceMonitor::IncreaseResourceCounter(char const* pcResourceName, ResourceType eType)
{
	if (pcResourceName == NULL)
		return;

	std::string kResourceName = pcResourceName;
	UPR(kResourceName);

	IncreaseResourceCounter(kResourceName, eType);
}

void PgResourceMonitor::IncreaseResourceCounter(const std::string& rkResourceName, ResourceType eType)
{
	if (rkResourceName.empty() || eType >= MAX_RESOURCE_TYPE_NUM || eType <= RESOURCE_TYPE_NONE)
		return;

	BM::CAutoMutex kLock(m_kMutex);

	ResourceMap::iterator iter = m_kResourceMap.find(rkResourceName);
	if (iter == m_kResourceMap.end())
	{
		std::pair<ResourceMap::iterator, bool> ret = m_kResourceMap.insert(std::make_pair(rkResourceName, ResourceCounter()));
		iter = ret.first;
		if (ret.second == false)
			return;
	}

	ResourceCounter& rkCounter = iter->second;
	float fCurrentTime = NiGetCurrentTimeInSec();

	if (rkCounter.iTotalCount == 0)
	{
		rkCounter.fFirstResourceRequestTime = fCurrentTime;
		rkCounter.fLastResourceRequestTime = fCurrentTime;
		rkCounter.eResourceType = eType;
	}
	else
	{
		PG_ASSERT_LOG(eType == rkCounter.eResourceType);
		float fGapTotal = rkCounter.iTotalCount * rkCounter.fAverageResourceReuqestGap + fCurrentTime - rkCounter.fLastResourceRequestTime;
		rkCounter.fAverageResourceReuqestGap = fGapTotal / rkCounter.iTotalCount; // gap의 갯수는 totalcount보다 하나 작다.
		rkCounter.fLastResourceRequestTime = fCurrentTime;
	}
	rkCounter.iTotalCount++;
}

void PgResourceMonitor::PrintResourceCounters(ResourceType eType)
{
	BM::CAutoMutex kLock(m_kMutex);
	ResourceMap::iterator iter = m_kResourceMap.begin();
	while (iter != m_kResourceMap.end())
	{
		ResourceCounter& rkCounter = iter->second;
		if (rkCounter.eResourceType >= MAX_RESOURCE_TYPE_NUM || rkCounter.eResourceType <= RESOURCE_TYPE_NONE)
		{
			continue;
		}

		if (eType != MAX_RESOURCE_TYPE_NUM && eType != rkCounter.eResourceType)
		{
			continue;
		}

		NILOG(PGLOG_STAT, "%s(%s) %d times called, first call %f, last call %f, average gap %f\n", iter->first.c_str(), g_pcResourceTypeName[rkCounter.eResourceType],rkCounter.iTotalCount, rkCounter.fFirstResourceRequestTime, rkCounter.fLastResourceRequestTime, rkCounter.fAverageResourceReuqestGap);
		++iter;
	}
}

void PgResourceMonitor::PrintResourceCountersForCSV(ResourceType eType)
{
	BM::CAutoMutex kLock(m_kMutex);
	NILOG(PGLOG_STAT, "\n\n, time, name, type, count, first, last, average\n"); 
	ResourceMap::iterator iter = m_kResourceMap.begin();
	while (iter != m_kResourceMap.end())
	{
		ResourceCounter& rkCounter = iter->second;
		if (rkCounter.eResourceType >= MAX_RESOURCE_TYPE_NUM || rkCounter.eResourceType <= RESOURCE_TYPE_NONE)
		{
			continue;
		}

		if (eType != MAX_RESOURCE_TYPE_NUM && eType != rkCounter.eResourceType)
		{
			continue;
		}

		NILOG(PGLOG_STAT, "%s, %s, %d, %f, %f, %f\n", iter->first.c_str(), g_pcResourceTypeName[rkCounter.eResourceType],rkCounter.iTotalCount, rkCounter.fFirstResourceRequestTime, rkCounter.fLastResourceRequestTime, rkCounter.fAverageResourceReuqestGap);
		++iter;
	}
}
