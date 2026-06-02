#ifndef FREEDOM_DRAGONICA_UTIL_SIMPLETIMER_H
#define FREEDOM_DRAGONICA_UTIL_SIMPLETIMER_H

class SimpleTimer
{
public:
	SimpleTimer(DWORD dwGap) { m_dwTimerGap = dwGap; m_dwLastTime = 0; }
	~SimpleTimer() { m_dwTimerGap = 0; m_dwLastTime = 0; }

	bool Tick()
	{
		bool ret = false;
		DWORD dwCurTime = BM::GetTime32();
		if ((dwCurTime - m_dwLastTime) >= m_dwTimerGap)
		{
			ret = true;
			m_dwLastTime = dwCurTime;
		}
		return ret;
	}

protected:
	DWORD m_dwLastTime;
	DWORD m_dwTimerGap;
};
#endif // FREEDOM_DRAGONICA_UTIL_SIMPLETIMER_H