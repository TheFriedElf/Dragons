#ifndef FREEDOM_DRAGONICA_MOVIE_PGMOVIEMGR_H
#define FREEDOM_DRAGONICA_MOVIE_PGMOVIEMGR_H

//#include "PgVmr.h"
//#include "PgAXPlay.h"

#include "CreateUsingNiNew.inl"
#include "lwGUID.h"

typedef enum eMovieType
{
	MT_NONE         = 0,
	MT_VMR	        = 1,
	MT_ACTIVEX      = 2,
	MT_DSHOW_INGAME = 3,
	MT_DSHOW        = 4,
}EMovieType;

typedef struct tagMovieValue
{
	std::string strID;
	BYTE byType;
	DWORD dwTime;
	std::string strPath;
	float fStartTime;
	float fEndTime;
	float fFadeInTime;
	float fFadeOutTime;
	float fVolumeDecTime;
}SMovieValue;
class PgVmr;
class PgAXPlay;
class PgDshowPlay;
class PgMovieMgr
{
public:
	typedef std::map<std::string, SMovieValue> PathContainer;
	typedef std::map<int, std::string> ScenarioMoviePath;
	typedef std::map<int, std::string> QuestMoviePath;
	typedef void (CALLBACK *LP_MOVIE_FINISHED)(std::wstring kMovieName) ;
public:
	PgMovieMgr();
	~PgMovieMgr();

	bool Init(EMovieType eType);
	void Release();
	void Terminate();

	bool Play(std::string strID);
	bool PlayCore();
	bool Stop();
	void StopMovie();
	void Update();

	bool IsPlay() { return m_bPlay; }
	bool IsRenderFrame() { return m_bRenderFrame; }

	void SetEndTime(DWORD dwTimeSizeSecond);
	bool ParseXml(char const *pcXmlPath);

	// 캐릭터 생성 동영상에서 해당 xml 파일을 백업, 복구한다.
	// 이름을 캐릭터 이름으로 치환 플레이를 위한 작업
	void BackupFile(std::string strID);
	void RecoveryFile(std::string strID);
	void ChangeCharName(std::string strID, std::wstring strName);
	void ChangeCharName2(std::string strID, std::wstring strName);

	void PlayOpeningMoveFromGUID(lwGUID kGuid);

	bool IsPlayedIntroMovie(lwGUID kCharGuid);
	void LoadCharMoviePlayInfoFile(lwGUID kCharGuid);
	void SaveCharMoviePlayInfoFile(lwGUID kCharGuid);

	void SetMovieFinishedCallBack(LP_MOVIE_FINISHED pfnMovieFinished) { m_pfnMovieFinished = pfnMovieFinished; }

	bool ParseScenarioXml(char const *pcXmlPath);
	std::string const GetSecnarioMovieName(int iMapNum);

	bool ParseQuestXml(char const *pcXmlPath);
	std::string const GetQuestMovieName(int const iQuestNum);

	typedef enum eUpdateSequence
	{
		US_DEFAULT	= 0,
		US_FADE_IN	= 1,
		US_PLAY		= 2,
		US_FADE_OUT	= 3,
	}EUpdateSequence;

private:
	PgVmr* m_kVmr;
	PgAXPlay* m_kAXPlay;
	PgDshowPlay* m_kDshowPlay;
	LP_MOVIE_FINISHED m_pfnMovieFinished;
	std::wstring m_kCurrentPlayingMovie;

	bool m_bPlay;
	bool m_bRenderFrame;
	EMovieType m_eCurType;
	DWORD m_dwEndTime;// ms

	PathContainer m_smPathContainer;
	ScenarioMoviePath m_smScenarioPath;
	QuestMoviePath m_smQuestPath;

	std::string m_strBuckupID;// 복구 파일 아이디

	bool m_bIntroPlayed;

	SMovieValue m_kMovieValue;
	DWORD m_dwFadeEndTime;

	EUpdateSequence m_eUpdateSequence;
	void UpdateSequence(EUpdateSequence eES) { m_eUpdateSequence = eES; }
	EUpdateSequence UpdateSequence() { return m_eUpdateSequence; }

	void SetFade(bool bFadeIn);
};

//#define g_kMovieMgr PgMovieMgr::Instance()
#define g_kMovieMgr SINGLETON_CUSTOM(PgMovieMgr, CreateUsingNiNew)

class lwMovieMgr
{
public:
	lwMovieMgr(PgMovieMgr* pMovieMgr);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void Play(char const *pcID);
	bool IsPlay();
	bool ScenarioMovie(int iMapNum);
	bool QuestMovie(int iQuestNum);

protected:
	PgMovieMgr *m_pkMovieMgr;
};

#endif // FREEDOM_DRAGONICA_MOVIE_PGMOVIEMGR_H