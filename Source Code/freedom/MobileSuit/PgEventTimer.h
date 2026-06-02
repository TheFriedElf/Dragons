#ifndef FREEDOM_DRAGONICA_UTIL_TIMER_PGEVENTIMER_H
#define FREEDOM_DRAGONICA_UTIL_TIMER_PGEVENTIMER_H

//이벤트 이름과 이벤트 스크립트를 지정하면 지정한 시간뒤에 이벤트 스크립트를 실행 한다.
//이벤트 추가시에 이벤트 카운트를 둔다.
//카운트가 0 과 같거나 작으면 이벤트를 제거 한다.

typedef struct tagTimerEventkey
{
	std::string kSceneID;
	std::string kEventID;

	tagTimerEventkey(char const* szSceneID, char const* szEventID)
	{
		kSceneID = szSceneID;
		kEventID = szEventID;
	}

	void Clear()
	{
		kSceneID.clear();
		kEventID.clear();
	}

	bool const operator < (const tagTimerEventkey& rhs) const
	{
		bool const bSceneCompare = kSceneID < rhs.kSceneID;
		bool const bEventCompare = kEventID < rhs.kEventID;
		if( bSceneCompare && bEventCompare )
		{
			return true;
		}
		else if( !bSceneCompare && bEventCompare )
		{
			return true;
		}
		else if( bSceneCompare && !bEventCompare )
		{
			return false;
		}
		//else if( !bSceneCompare && !bEventCompare )
		//{
		//}
		return false;
	}

	bool const operator == (const tagTimerEventkey& rhs) const
	{
		return (kSceneID == rhs.kSceneID) && (kEventID == rhs.kEventID);
	}
} STimerEventKey;

typedef struct tagTimerEvent
{
public:
	tagTimerEvent();

	void Clear();

	void Set(float const fTime, char const* szEventScript, int const iRunCount = 1);
	bool Empty() const;

	void Start(float const fAccumTime);
	void Reset(float const fAccumTime, int const iNewCount);

	bool RunAble(float const fAccumTime);
	bool RunScript(float const fAccumTime);

	bool const operator < (const tagTimerEvent& rhs) const;

	void Pause(float const fAccumTime);
	void Resume(float const fAccumTime);
	bool Pause() const;

	CLASS_DECLARATION_S(STimerEventKey, Key);
protected:
	//초기값
	float fInterval;
	int iInitCount;
	std::string kScript;

	//Control 변수
	float fTargetTime;
	int iRemainCount;
	bool bPause;
} STimerEvent;

class PgEventTimer
{
	typedef std::map< STimerEventKey, STimerEvent* > ContTimerEvent;
	typedef std::vector< STimerEventKey > ContTimerEventKey;
	//typedef std::list< STimerEvent* > ContTimerEventList;
	//typedef std::map< STimerEventKey, ContTimerEventList::iterator > ContTimerEventListMap;

public:
	PgEventTimer();
	~PgEventTimer();

	void Clear();//모두 초기화
	void Update(float const fAccumTime, float const fFrameTime);
	void ChangeScene(char const* szNewSceneID);

	bool AddLocal(char const* szEventID ,const STimerEvent& rkTimerEvent, bool const bImmediateRun = false);
	bool AddGlobal(char const* szEventID ,const STimerEvent& rkTimerEvent, bool const bImmediateRun = false);
	bool Del(char const* szSceneID, char const* szEventID);
	bool Pause(char const* szSceneID, char const* szEventID);
	bool Resume(char const* szSceneID, char const* szEventID);
	bool Reset(char const* szSceneID, char const* szEventID, int const iNewCount = 0);
	bool IsHaveLocal(char const* szEventID);
	bool IsHaveGlobal(char const* szEventID);
	bool RightNowLocal(char const* szEventID);
	bool RightNowGlobal(char const* szEventID);

protected:
	bool Get(char const* szSceneID, char const* szEventID, STimerEvent*& pkOut);
	bool Get(const STimerEventKey& rkKey, ContTimerEvent::iterator& iter_out);
	bool Add(const STimerEventKey& rkKey, const STimerEvent& rkTimerEvent, bool const bImmediateRun = false);
	//bool Del(const ContTimerEventList::iterator iter, ContTimerEventList::iterator& iter_out);
	bool Del(const ContTimerEvent::iterator iter, ContTimerEvent::iterator& iter_out, bool const bReserve = false);
	bool Del(const STimerEventKey& rkKey);
	bool RunEvent(float const fAccumTime, ContTimerEvent::mapped_type pkTimerEvent, bool& bEraseOut) const;

	float UpdatedTime()const;
protected:
	ContTimerEvent m_kTimerEvent;

	ContTimerEventKey m_kDelReserve;

	//ContTimerEventList m_kTimerEventList;//Update용
	//ContTimerEventListMap m_kTimerEventListMap;//Key to 

	mutable Loki::Mutex m_kMutex;
};

#define g_kEventTimer SINGLETON_STATIC(PgEventTimer)
#endif // FREEDOM_DRAGONICA_UTIL_TIMER_PGEVENTIMER_H