#ifndef FREEDOM_DRAGONICA_APP_MEMORYTRACKING_PGMEMSTATUSMANAGER_H
#define FREEDOM_DRAGONICA_APP_MEMORYTRACKING_PGMEMSTATUSMANAGER_H

#include <NiMain.H>
#include "NiMemTracker.H"
#include "Loki/Singleton.H"

class	PgMemStatusManager	
{
public:

	struct	stMemInfo
	{
		std::string	m_kFileName;
		int	m_iLine;
		std::string	m_kFunction;

		int	m_iAllocCount;
		int	m_iDeallocCount;
		int	m_iTotalAllocActionCount;

		DWORD	m_dwTotalAllocMemSize;
		DWORD	m_dwAverageAllocMemSize;
		DWORD	m_dwPeakMemSize;
		float	m_fAverageAllocMemTangent;
		stMemInfo(std::string const &kFileName,int iLine,std::string const &kFunctionName)
			:m_iLine(iLine),
			m_iAllocCount(0),
			m_iDeallocCount(0),
			m_iTotalAllocActionCount(0),
			m_dwTotalAllocMemSize(0),
			m_dwAverageAllocMemSize(0),
			m_fAverageAllocMemTangent(0),
			m_dwPeakMemSize(0),
			m_kFunction(kFunctionName),
			m_kFileName(kFileName)
		{
		};
		
	};
	typedef	std::list<stMemInfo>	FunctionNameMemInfoList;

	struct	stFileMemInfo
	{

		stFileMemInfo(std::string const &kFileName)
			:m_kFileName(kFileName)
		{
		};

		std::string	m_kFileName;

		FunctionNameMemInfoList	m_kMemInfoCont;
	};

	typedef	std::map<std::string,stFileMemInfo>	FileMemInfoMap;


public:

	PgMemStatusManager()
		:m_bActive(false)
	{
	}

	void	OnAlloc(NiAllocUnit *pkAllocUnit);
	void	OnDealloc(NiAllocUnit *pkAllocUnit);

	void	ClearData()
	{
		m_kMemInfoCont.clear();
	}

	void	SetActive(bool	bActive)	{	m_bActive = bActive;	}
	bool	GetActive()	const	{	return	m_bActive;	}

private:

	void	OnMemoryChange(NiAllocUnit *pkAllocUnit,int iSizeChange);

	stMemInfo*	FindMemInfo(NiAllocUnit *pkAllocUnit);
	stMemInfo*	AddMemInfo(NiAllocUnit *pkAllocUnit);

	void	OutputPeakInfoToConsole(stMemInfo *pkMemInfo);

private:


	FileMemInfoMap	m_kMemInfoCont;
	bool	m_bActive;
	
};

extern	PgMemStatusManager	g_kMemStatusManager;

#endif // FREEDOM_DRAGONICA_APP_MEMORYTRACKING_PGMEMSTATUSMANAGER_H