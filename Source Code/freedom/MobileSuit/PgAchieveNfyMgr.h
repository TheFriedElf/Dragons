#ifndef FREEDOM_DRAGONICA_CONTENTS_LWPGACHIEVENFYMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_LWPGACHIEVENFYMGR_H

class PgSlideTextMgr
{
public:
	typedef enum eAddTextType
	{
		EATT_NONE = 0,
		EATT_ACHIEVE_FIRST_NOTIFY = 1,
		EATT_ITEM_RARE_NOTIFY = 2,
	}E_ADD_TEXT_TYPE;

	typedef struct tagSlideTextInfo
	{
		E_ADD_TEXT_TYPE Type;
		std::wstring kText;
		BM::Stream kAddon;
	}S_SLIDE_TEXT_INFO;

	typedef std::list< S_SLIDE_TEXT_INFO > CONT_SLIDE_TEXT;

	void Push(S_SLIDE_TEXT_INFO const& kInfo)
	{
		m_ContText.push_back(kInfo);
	}

	bool Pop(S_SLIDE_TEXT_INFO& kInfo)
	{
		if( m_ContText.empty() )
		{
			return false;
		}
		kInfo = m_ContText.front();
		m_ContText.pop_front();
		return true;
	}

private:
	CONT_SLIDE_TEXT m_ContText;
};

#define g_kSlideTextMgr	SINGLETON_STATIC(PgSlideTextMgr)

namespace lwAchieveNfyMgr
{
	void RegisterWrapper(lua_State *pkState);

	void lwCheckNewAchieveNfy();
	void AddNewSlideText(PgSlideTextMgr::E_ADD_TEXT_TYPE const Type, std::wstring const& kText, BM::Stream const& kAddon);
	void SetSlideTextViewUI(XUI::CXUI_Wnd* pkMainUI);
	void lwUpdateSlideTextViewUI(lwUIWnd kParent);
	void lwOnClickAddonButton(lwUIWnd kParent);

	void CallFirstAchieveUserNfy(int const iAchievementNo, std::wstring const& kUserName);
	void CallGenericNfy(std::wstring const& Content, CONT_GENERIC_NOTICE const& ContNotice);
	void lwSendAchieveGreetMessage(lwUIWnd kSelf);
	
	void lwOpenAchievementInfo(lwUIWnd kWnd);

	void ShowAchieve(DWORD const iSaveIdx);
	void AddAchievementNfyMsg(DWORD const AchimentNum);
	bool GetAchievementInfo(DWORD const AchimentNum, TBL_DEF_ACHIEVEMENTS& AchievementInfo, bool const bIsInfoCall = true);
	bool GetAchievementInfo(DWORD const AchimentNum, DWORD const AchieveIdx, TBL_DEF_ACHIEVEMENTS& AchievementInfo);
	bool GetNearAchievementInfo(int const iAchievementCount, CONT_DEF_ACHIEVEMENTS const& Def, TBL_DEF_ACHIEVEMENTS& kAchievementInfo, bool const bIsInfoCall);

	std::wstring const GetAchievementString(int const iIndex, int const iTitleNo);
}

typedef struct tagAchieveNfyInfo
{
	int iTickCount;
	float fCloseTime;
}SAchieveNfyInfo;

typedef std::map< int, int > CONT_CACHE_IDX_TO_TYPE;
typedef std::map< int, int > CONT_CACHE_SIDX_TO_IDX;
typedef std::deque<DWORD> CONT_SHOWACHIEVE;

class PgAchieveNfyMgr
{
public:
	PgAchieveNfyMgr(void);
	~PgAchieveNfyMgr(void);
	
	bool OnBuild();
	int IdxToType(DWORD const Idx);
	int SIdxToIdx(DWORD const SIdx);
	int SIdxToType(DWORD const SIdx);
	bool SIdxToIdxAndType(DWORD const SIdx, DWORD& Type, DWORD& Idx);

	float GetCheckTime(){ return m_fCheckTime; }
	float GetCheckProgress(){ return m_fCheckProgress; }
	int GetStatusAliveTime(){ return m_iStatusAliveTime; }
	int GetIconAliveTime(){ return m_iIconAliveTime; }
	bool ParseXml(char const* pcXmlPath);
	void Update(float fAccumTime);
	void AddShowAchieve(DWORD const iSaveIdx);
	void ClearShowAchievement();
	void ContShowAchieve_PopFront();

private:
	CONT_CACHE_SIDX_TO_IDX	m_kReverseCacheSIdxToIdx;
	CONT_CACHE_IDX_TO_TYPE	m_kReverseCacheIdxToType;
	CONT_SHOWACHIEVE		m_ContShowAchieve;
	
	float m_fCheckTime;
	float m_fCheckProgress;
	int m_iStatusAliveTime;
	int m_iIconAliveTime;
};

#define g_kAchieveNfyMgr	SINGLETON_STATIC(PgAchieveNfyMgr)


class PgPlayer;
class PgAchieveValueCtrl
{
public:
	PgAchieveValueCtrl(int const iSaveIdx, PgPlayer & pkPlayer);
	~PgAchieveValueCtrl(){}

	int Max()const;
	int Now()const;
	int Rate()const;

private:
	int m_iSaveIdx;
	int m_iValue;
	int m_iType;
	int m_iGroupNo;
	bool m_bComplete;
	PgPlayer & m_rkPlayer;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_LWPGACHIEVENFYMGR_H