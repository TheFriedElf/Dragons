#ifndef FREEDOM_DRAGONICA_UTIL_PGSTAT_H
#define FREEDOM_DRAGONICA_UTIL_PGSTAT_H

//#ifdef EXTERNAL_RELEASE
#define PG_STAT_H(x)	/##/
#define PG_STAT(x)		/##/
#undef PG_STAT_ENABLED
//#else
//#define PG_STAT_H(x)	x;
//#define PG_STAT(x)		x;
//#define PG_STAT_ENABLED
//#endif

typedef struct _StatInfoF
{
	_StatInfoF() { iCount = 0; fTotalTime = 0.0f; fMaxTime = 0.0f; fMinTime = 1000.0f; }
	unsigned long iCount;
	float fTotalTime;
	float fMaxTime;
	float fMinTime;
	float fLastTime;
} StatInfoF;

typedef struct _FrameStat
{
	_FrameStat() { iCountPerFrame = 0; iCurrentFrame = 0; fCurrentTotalTime = 0.0f; iMaxCount = 0; iLastCount = 0; iStartFrame = 0; iFrameOfMax = 0; }
	unsigned long iCountPerFrame;
	unsigned long iCurrentFrame;
	unsigned long iMaxCount;	
	unsigned long iLastCount;
	unsigned long iStartFrame;
	unsigned long iFrameOfMax;
	float fCurrentTotalTime;
	StatInfoF frameStat;
	StatInfoF totalStat;
} FrameStat;

extern void UpdateStatInfo(StatInfoF& info, float fTime);
extern void UpdateStatInfo(StatInfoF* info, float fTime);
extern void PrintStatInfo(StatInfoF& info, char const* header = NULL, char const* footer = NULL);

extern void UpdateFrameCount(FrameStat& info, unsigned long iFrame, float fTime);
extern void PrintCSVHeader();
extern void PrintFrameCount(FrameStat& info, char const* header = NULL, char const* footer = NULL, float fGroupTotalTime = 0.0f);
extern void PrintFrameCountForCSV(FrameStat& info, char const* header = NULL, char const* footer = NULL, float fGroupTotalTime = 0.0f);

class PgStatTimerF
{
public:
	PgStatTimerF(FrameStat* info, unsigned long iFrame);
	~PgStatTimerF();
	void Start();
	void Stop();

protected:
	float fTime;
	bool bStart;
	unsigned long iFrame;
	FrameStat* pStatInfo;
};


class	PgSimpleTimeAccumulator
{
public:

	PgSimpleTimeAccumulator(std::string const	&kName)
		:m_fAccumTime(0),m_kName(kName)
	{
	};

	~PgSimpleTimeAccumulator()
	{
		_PgOutputDebugString("[SimpleTimeAccumulator][%s] %f\n",m_kName.c_str(),m_fAccumTime);
	}

	void	Add(float fTime)
	{
		m_fAccumTime+=fTime;
	}

private:

	float	m_fAccumTime;
	std::string	m_kName;
};

class	PgSimpleTimeChecker
{
public:

	PgSimpleTimeChecker(std::string const	&kName,PgSimpleTimeAccumulator &kAccumulator,bool bStart = true)
		:m_kName(kName),m_fStartTime(0)
	{
		m_pkAccumulator = &kAccumulator;
		if(bStart)
		{
			Start();
		}
	}
	void	Start()
	{
		m_fStartTime = NiGetCurrentTimeInSec();
	}
	void	End()
	{
		if(m_fStartTime != 0)
		{
			float	fElapsedTime = NiGetCurrentTimeInSec() - m_fStartTime;
			if(m_pkAccumulator)
			{
				m_pkAccumulator->Add(fElapsedTime);
			}
			_PgOutputDebugString("[SimpleTimer][%s] %f\n",m_kName.c_str(),fElapsedTime);
			m_fStartTime = 0;
		}
	}

	~PgSimpleTimeChecker()
	{
		End();
	}

private:

	float	m_fStartTime;
	std::string	m_kName;
	PgSimpleTimeAccumulator	*m_pkAccumulator;

};

class PgStatGroup
{
public:
	PgStatGroup();
	PgStatGroup(char* groupName);
	PgStatGroup(std::string groupName);
	~PgStatGroup();
	void ToggleRecording();
	void StartRecording();
	void StopRecording();
	bool GetRecording();
	FrameStat* GetStatInfo(std::string statName);
	void UpdateStat(std::string statName, float fTime, unsigned long iFrame);
	void ResetStatGroup();
	void PrintStatGroup();
	void PrintStatGroupForCSV();

protected:
	typedef std::map<std::string, FrameStat> StatGroupMap;
	StatGroupMap m_kStatInfoList;
	std::string m_kStatGroupName;
	bool m_bIsRecording;
	Loki::Mutex m_kStatGroupLock;	
};
#endif // FREEDOM_DRAGONICA_UTIL_PGSTAT_H