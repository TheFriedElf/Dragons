#include "StdAfx.h"
#include "PgIResourceLoadController.h"

PgIResourceLoadController::PgIResourceLoadController()
{	
	m_kStageMap.clear();
}

PgIResourceLoadController::~PgIResourceLoadController()
{
	m_kStageMap.clear();
}

bool PgIResourceLoadController::LoadResourcesAtStage(int iStage)
{
	STAGE_MAP::iterator stage_iter = m_kStageMap.find(iStage);
	if (stage_iter == m_kStageMap.end())
		return false;

	RESOURCE_PATH_LIST& rkList = stage_iter->second;

	RESOURCE_PATH_LIST::iterator list_iter = rkList.begin();
	while (list_iter != rkList.end())
	{
		if (LoadResource((*list_iter).c_str()) == false)
			return false;

		++list_iter;
	}
	return true;
}

bool PgIResourceLoadController::LoadAllResource()
{
	STAGE_MAP::iterator stage_iter = m_kStageMap.begin();
	while (stage_iter != m_kStageMap.end())
	{
		if (LoadResourcesAtStage(stage_iter->first) == false)
			return false;
	}

	return true;
}

void PgIResourceLoadController::RegisterResource(char const* pcResourcePath, int iStage)
{
	STAGE_MAP::iterator stage_iter = m_kStageMap.find(iStage);
	if (stage_iter == m_kStageMap.end())
	{
		std::pair<STAGE_MAP::iterator, bool> ret = m_kStageMap.insert(std::make_pair(iStage, RESOURCE_PATH_LIST()));
		if (ret.second == false)
			return;
		stage_iter = ret.first;
	}

	RESOURCE_PATH_LIST& rkList = stage_iter->second;

	rkList.push_back(pcResourcePath);
}

bool PgIResourceLoadController::IsStageLoaded(int iStage)
{
	STAGE_MAP::iterator stage_iter = m_kStageMap.find(iStage);
	if (stage_iter == m_kStageMap.end())
	{
		return true;	// QUESTION 의미상은 false일까?
	}

	if (stage_iter->second.size() > 0)
		return false;

	return true;
}

bool PgIResourceLoadController::IsAllResourceLoaded()
{
	STAGE_MAP::iterator stage_iter = m_kStageMap.begin();
	while (stage_iter != m_kStageMap.end())
	{
		if (stage_iter->second.size() > 0)
			return false;
	}

	return true;
}