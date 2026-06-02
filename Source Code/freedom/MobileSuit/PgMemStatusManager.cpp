#include "stdafx.h"
#include "PgMemStatusManager.H"

PgMemStatusManager	g_kMemStatusManager;

void	PgMemStatusManager::OnAlloc(NiAllocUnit *pkAllocUnit)
{
	if(!GetActive())
	{
		return;
	}

	OnMemoryChange(pkAllocUnit,pkAllocUnit->m_stSizeRequested);
}
void	PgMemStatusManager::OnDealloc(NiAllocUnit *pkAllocUnit)
{
	if(!GetActive())
	{
		return;
	}

	OnMemoryChange(pkAllocUnit,-((int)pkAllocUnit->m_stSizeRequested));
}

void	PgMemStatusManager::OnMemoryChange(NiAllocUnit *pkAllocUnit,int iSizeChange)
{
	stMemInfo	*pkMemInfo = FindMemInfo(pkAllocUnit);
	if(!pkMemInfo)
	{
		pkMemInfo = AddMemInfo(pkAllocUnit);
	}
	if(!pkMemInfo)
	{
		return;
	}

	(iSizeChange > 0) ? (pkMemInfo->m_iAllocCount++) : (pkMemInfo->m_iDeallocCount++);
	

	DWORD	dwPrevAverageAllocMemSize = pkMemInfo->m_dwAverageAllocMemSize;
	pkMemInfo->m_iTotalAllocActionCount++;
	pkMemInfo->m_dwTotalAllocMemSize+=iSizeChange;
	if(pkMemInfo->m_dwTotalAllocMemSize>pkMemInfo->m_dwPeakMemSize)
	{
		pkMemInfo->m_dwPeakMemSize = pkMemInfo->m_dwTotalAllocMemSize;
		OutputPeakInfoToConsole(pkMemInfo);
	}
}
void	PgMemStatusManager::OutputPeakInfoToConsole(stMemInfo *pkMemInfo)
{

	char	str[1000];
	sprintf_s(str,"MemStatusInfo Peak Updated : %u File : %s Line : %u Func : %s\n",
		pkMemInfo->m_dwPeakMemSize,
		pkMemInfo->m_kFileName.c_str(),
		pkMemInfo->m_iLine,
		pkMemInfo->m_kFunction.c_str());

	OutputDebugStringA(str);

}
PgMemStatusManager::stMemInfo*	PgMemStatusManager::FindMemInfo(NiAllocUnit *pkAllocUnit)
{
	FileMemInfoMap::iterator itor = m_kMemInfoCont.find(std::string(pkAllocUnit->m_kFLF.m_pcFile));
	if(itor == m_kMemInfoCont.end())
	{
		return	NULL;
	}

	stFileMemInfo	*pkFileMemInfo = &itor->second;
	
	for(FunctionNameMemInfoList::iterator itor = pkFileMemInfo->m_kMemInfoCont.begin(); itor != pkFileMemInfo->m_kMemInfoCont.end();++itor)
	{
		stMemInfo	*pkMemInfo = &(*itor);
		if(	pkMemInfo->m_iLine == pkAllocUnit->m_kFLF.m_uiLine &&
			pkMemInfo->m_kFunction == pkAllocUnit->m_kFLF.m_pcFunc)
		{
			return	pkMemInfo;
		}
	}

	return	NULL;
}
PgMemStatusManager::stMemInfo*	PgMemStatusManager::AddMemInfo(NiAllocUnit *pkAllocUnit)
{
	FileMemInfoMap::iterator itor = m_kMemInfoCont.find(std::string(pkAllocUnit->m_kFLF.m_pcFile));
	if(itor == m_kMemInfoCont.end())
	{
		m_kMemInfoCont.insert(std::make_pair(std::string(pkAllocUnit->m_kFLF.m_pcFile),stFileMemInfo(std::string(pkAllocUnit->m_kFLF.m_pcFile))));
		itor = m_kMemInfoCont.find(std::string(pkAllocUnit->m_kFLF.m_pcFile));
	}

	stFileMemInfo	*pkFileMemInfo = &itor->second;
	pkFileMemInfo->m_kMemInfoCont.push_back(stMemInfo(pkAllocUnit->m_kFLF.m_pcFile,pkAllocUnit->m_kFLF.m_uiLine,pkAllocUnit->m_kFLF.m_pcFunc));

	return	FindMemInfo(pkAllocUnit);
}
