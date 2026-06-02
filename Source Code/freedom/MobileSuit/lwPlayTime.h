#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWPLAYTIME_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWPLAYTIME_H

#include "lwUI.h"

class lwPlayTime
{
public:
	lwPlayTime();
	~lwPlayTime();

	void RunInternetBrowser();
	bool CallFatigueUI();
	void Update();
	void Activate(char *pszName);
	bool CallPlayTimeWarn(DWORD const dwCurTime);
	void CallTimerToolTip(lwUIWnd UIWnd);
	void SetImgName();
	bool SetRate(int const iRate);
	void SetNextWarnMsgTime(int const iAccConnSec);
	bool IsPickUpItem()const;
	int	GetTimeTextValue();
	void SetRateTime();
	void SetNextWarnSec(int const iSec);

	void SetLastLogOutTime(BM::DBTIMESTAMP_EX const dtLastLogout);
	BM::DBTIMESTAMP_EX GetLastLogOutTime();

	static TCHAR ms_szURL[MAX_PATH];

protected:
	int				m_iRate;
	bool			m_bViewClock;
	DWORD			m_dwNextWarnSec;
	DWORD			m_dwNextTimerUpdate;
	std::wstring	m_wstrImgName;
	BM::PgPackedTime m_kMsgNextTime;
	PgDefPlayerPlayTimeImpl::SSUBMSG m_kMsg;
	BM::DBTIMESTAMP_EX m_dtLastLogout;
};

#define g_klwPlayTime SINGLETON_STATIC(lwPlayTime)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWPLAYTIME_H

