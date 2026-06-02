#include "stdafx.h"
#include "PgStat.h"

inline void UpdateStatInfo(StatInfoF& info, float fTime)
{
	info.iCount++;
	info.fTotalTime += fTime;
	if (fTime > info.fMaxTime)
		info.fMaxTime = fTime;
	if (fTime < info.fMinTime)
		info.fMinTime = fTime;
	info.fLastTime = fTime;
}

inline void UpdateStatInfo(StatInfoF* info, float fTime)
{
	if (info == NULL)
		return;

	info->iCount++;
	info->fTotalTime += fTime;
	if (fTime > info->fMaxTime)
		info->fMaxTime = fTime;
	if (fTime < info->fMinTime)
		info->fMinTime = fTime;
	info->fLastTime = fTime;
}

inline void PrintStatInfo(StatInfoF& info, char const* header, char const* footer)
{
	std::string headerString;
	std::string footerString;
	if (header)
	{
		headerString = header;
	}
	if (footer)
	{
		footerString = footer;
	}

	NILOG(PGLOG_STAT, "%s (%d times called, min %f, max %f, average %f, total %f, last %f) %s\n", headerString.c_str(),  info.iCount, info.iCount > 0 ? info.fMinTime : 0.0f, info.fMaxTime, info.iCount > 0 ? info.fTotalTime / (float)info.iCount : 0.0f, info.fTotalTime, info.fLastTime, footerString.c_str());
}

inline void UpdateFrameCount(FrameStat& info, unsigned long iFrame, float fTime)
{
	if (info.iCurrentFrame == iFrame)
	{
		info.iCountPerFrame++;
		info.fCurrentTotalTime += fTime;		
	}
	else
	{
		if (info.fCurrentTotalTime < 1.5f)
		{
			UpdateStatInfo(info.frameStat, info.fCurrentTotalTime);
			if (info.frameStat.fMaxTime == info.fCurrentTotalTime)
			{
				info.iMaxCount = info.iCountPerFrame;
				info.iFrameOfMax = iFrame;
			}
		}
		if (info.iStartFrame == 0)
			info.iStartFrame = iFrame;

		info.iLastCount = info.iCountPerFrame;
		info.iCurrentFrame = iFrame;
		info.iCountPerFrame = 1;
		info.fCurrentTotalTime = fTime;
	}
	UpdateStatInfo(info.totalStat, fTime);
}

inline void PrintFrameCount(FrameStat& info, char const* header, char const* footer, float fGroupTotalTime)
{
	std::string headerString;
	std::string footerString;
	if (header)
	{
		headerString = header;
	}
	if (footer)
	{
		footerString = footer;
	}

	NILOG(PGLOG_STAT, "%s Total(%d times called, min %f, max %f, average %f, total %f(%.2f%%), last %f) Frame(%d times called, min %f, max %f(%d,%d), average %f, total %f, last %f(%d)) %s\n", headerString.c_str(), info.totalStat.iCount, info.totalStat.iCount > 0 ? info.totalStat.fMinTime : 0.0f, info.totalStat.fMaxTime, info.totalStat.iCount > 0 ? info.totalStat.fTotalTime / (float)info.totalStat.iCount : 0.0f, info.totalStat.fTotalTime, fGroupTotalTime > 0.0f ? info.totalStat.fTotalTime / fGroupTotalTime * 100.0f : 100.0f, info.totalStat.fLastTime, info.frameStat.iCount, info.frameStat.iCount > 0 ? info.frameStat.fMinTime : 0.0f, info.frameStat.fMaxTime, info.iMaxCount, info.iFrameOfMax, info.frameStat.iCount > 0 ? info.frameStat.fTotalTime / (float)info.frameStat.iCount : 0.0f, info.frameStat.fTotalTime, info.frameStat.fLastTime, info.iLastCount, footerString.c_str());
}

inline void PrintCSVHeader()
{
	NILOG(PGLOG_STAT, "\n\ngroup, name, percentage(total), call(total), min time(total), max time(total), average(total), time(total), call(frame), min time(frame), max time(frame), max count(frame), average(frame), time(frame)\n"); 
}

inline void PrintFrameCountForCSV(FrameStat& info, char const* header, char const* footer, float fGroupTotalTime)
{
	std::string headerString;
	std::string footerString;
	if (header)
	{
		headerString = header;
		std::string::size_type index = headerString.find_first_of(',');
		while (index != std::string::npos)
		{
			headerString.replace(index, 1, " ");
			index = headerString.find_first_of(',', index);
		}
	}

	if (footer)
	{
		footerString = footer;
		std::string::size_type index = footerString.find_first_of(',');
		while (index != std::string::npos)
		{
			footerString.replace(index, 1, " ");
			index = footerString.find_first_of(',', index);
		}
	}

	NILOG(PGLOG_STAT, "%s, %s, %.2f, %d, %f, %f, %f, %f, %d, %f, %f, %d, %f, %f\n", headerString.c_str(), footerString.c_str(), fGroupTotalTime > 0.0f ? info.totalStat.fTotalTime / fGroupTotalTime * 100.0f : 100.0f, info.totalStat.iCount, info.totalStat.iCount > 0 ? info.totalStat.fMinTime : 0.0f, info.totalStat.fMaxTime, info.totalStat.iCount > 0 ? info.totalStat.fTotalTime / (float)info.totalStat.iCount : 0.0f, info.totalStat.fTotalTime, info.frameStat.iCount, info.frameStat.iCount > 0 ? info.frameStat.fMinTime : 0.0f, info.frameStat.fMaxTime, info.iMaxCount, info.frameStat.iCount > 0 ? info.frameStat.fTotalTime / (float)info.frameStat.iCount : 0.0f, info.frameStat.fTotalTime);
}

PgStatTimerF::PgStatTimerF(FrameStat* info, unsigned long iFrame)
{
	if (info)
		pStatInfo = info;
	else
		pStatInfo = NULL;
	this->iFrame = iFrame;
	Start();
}

PgStatTimerF::~PgStatTimerF()
{
	Stop();
}

void PgStatTimerF::Start()
{
	bStart = true;
	fTime = NiGetCurrentTimeInSec();
}

void PgStatTimerF::Stop()
{
	if (bStart && pStatInfo)
	{
		UpdateFrameCount(*pStatInfo, iFrame, NiGetCurrentTimeInSec() - fTime);
		bStart = false;
	}
}

PgStatGroup::PgStatGroup()
{
	m_bIsRecording = false;
}

PgStatGroup::PgStatGroup(char* groupName)
{
	if (groupName)	
		m_kStatGroupName = groupName;
	m_bIsRecording = false;
}

PgStatGroup::PgStatGroup(std::string groupName)
{
	m_kStatGroupName = groupName;
	m_bIsRecording = false;
}

PgStatGroup::~PgStatGroup()
{
	m_kStatInfoList.clear();
	m_bIsRecording = false;
}

void PgStatGroup::ToggleRecording()
{
	m_bIsRecording = !m_bIsRecording;
}

void PgStatGroup::StartRecording()
{
	m_bIsRecording = true;
}

void PgStatGroup::StopRecording()
{
	m_bIsRecording = false;
}

bool PgStatGroup::GetRecording()
{
	return m_bIsRecording;
}

FrameStat* PgStatGroup::GetStatInfo(std::string statName)
{
	if (m_bIsRecording == false)
		return NULL;

	StatGroupMap::iterator iter = m_kStatInfoList.find(statName);
	if (iter == m_kStatInfoList.end())
	{
		BM::CAutoMutex kLock(m_kStatGroupLock);
		std::pair<StatGroupMap::iterator, bool> ret = m_kStatInfoList.insert(std::make_pair(statName, FrameStat()));
		iter = ret.first;
	}
	return &(iter->second);
}

void PgStatGroup::UpdateStat(std::string statName, float fTime, unsigned long iFrame)
{
	UpdateFrameCount(*GetStatInfo(statName), iFrame, fTime);
}

void PgStatGroup::ResetStatGroup()
{
	FrameStat kNullStat;
	StatGroupMap::iterator iter = m_kStatInfoList.begin();
	while (iter != m_kStatInfoList.end())
	{
		iter->second = kNullStat;
		++iter;
	}
}

void PgStatGroup::PrintStatGroup()
{
	StatGroupMap::iterator iter = m_kStatInfoList.begin();
	float totalTime = 0.0f;
	while (iter != m_kStatInfoList.end())
	{
		totalTime += iter->second.totalStat.fTotalTime;
		++iter;
	}

	iter = m_kStatInfoList.begin();
	while (iter != m_kStatInfoList.end())
	{
		PrintFrameCount(iter->second, m_kStatGroupName.c_str(), iter->first.c_str(), totalTime);
		++iter;
	}
}

void PgStatGroup::PrintStatGroupForCSV()
{
	StatGroupMap::iterator iter = m_kStatInfoList.begin();
	float totalTime = 0.0f;
	while (iter != m_kStatInfoList.end())
	{
		totalTime += iter->second.totalStat.fTotalTime;
		++iter;
	}

	PrintCSVHeader();
	iter = m_kStatInfoList.begin();
	while (iter != m_kStatInfoList.end())
	{
		PrintFrameCountForCSV(iter->second, m_kStatGroupName.c_str(), iter->first.c_str(), totalTime);
		++iter;
	}
}