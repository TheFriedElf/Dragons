#ifndef FREEDOM_DRAGONICA_CONTENTS_EMPORIA_PGEMPORIAMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_EMPORIA_PGEMPORIAMGR_H

#include "Variant/PgEmporia.h"
#include "Variant/PgEmporiaFunction.h"
#include "Variant/EmporiaTournament.h"

typedef enum
{
	ADMT_NONE		= 0,
	ADMT_MINI		= 1,
	ADMT_WINDOW		= 2,
}EmporiaViewADMercenaryType;

class PgEmporiaStateUI
{
public:
	typedef enum eState
	{
		// 숫자 함부로 바꾸지 말것 lua에서도 사용
		EMUI_ENTRANCE_BOARD					= 0,
		EMUI_STATE_EMPORIA_STATUSBOARD		= 1,
		EMUI_STATE_EMPORIA_HELP				= 2,
	}EState;

public:
	PgEmporiaStateUI()
		:	m_iState(EMPORIA_CLOSE)
		,	m_i64BattleTime(0i64)
		,	m_iChallengeGuildCount(0)
		,	m_i64TotalChallengeExp(0i64)
	{}

	virtual ~PgEmporiaStateUI(){}

	// Contents Server의 PgEmporiaPack::WriteToPacket 과 짝
	bool ReadFromPacket( BM::Stream &kPacket );
	void Update( bool const bIsInfo, XUI::CXUI_Wnd *pkWnd );
	void HelpReawardUpdate( XUI::CXUI_Wnd* pTap );
	__int64 GetBattleTime()const{ return m_i64BattleTime; }
	bool IsNowJoinMercenary()const;
	bool ProcessJoinMercenary()const;
	BM::GUID const & GetEmporiaID()const{ return m_kEmporiaID; }

protected:
	void SetUIOwnerName( XUI::CXUI_Wnd *pkControl, std::wstring const &wstrName )const;
	void SetUIGuildInfo( XUI::CXUI_Wnd *pkControl, SEmporiaGuildInfo const &kGuildInfo, bool const bLinkParent, bool const bIsLose )const;
	void SetMercenaryJoinBtn( XUI::CXUI_Wnd* pkControl, PgEmporiaTournamentElement const& kBattleInfo, SEmporiaGuildInfoEx const &kGuildInfo, PgLimitClass const& kLimitClass, bool const bIsAtkGuild );
	void SetBattleRewardSlot(XUI::CXUI_Wnd* pParent, int const iRank, TBL_DEF_EMPORIA_REWARD::SRank const& kReward, bool const bIsTournament = false);

protected:
	BM::GUID				m_kEmporiaID;
	int						m_iState;
	__int64					m_i64BattleTime;
	size_t					m_iChallengeGuildCount;
	__int64					m_i64TotalChallengeExp;
	PgEmporiaTournament		m_kTournament;
	PgEmporia				m_kEmporia[MAX_EMPORIA_GRADE];
};

class PgEmporiaMgr
{
public:
	static std::wstring const ms_wstrEmporiaUIWnd;

public:
	PgEmporiaMgr();
	~PgEmporiaMgr();

	void Clear();
	

	bool ProcessPacket( BM::Stream::DEF_STREAM_TYPE const usType, BM::Stream &rkPacket );
	void ADMercenary( BM::Stream &rkPacket );

	void RefreshThrowButton( BYTE const byEmpriaStatus );

	XUI::CXUI_Wnd* GetWnd( bool bActivate )const;

	void RefreshUI( bool const bIsInfo, XUI::CXUI_Wnd *pkControl );
	__int64 GetChallengeEndTime()const;

	bool IsNowJoinMercenary()const;
	void SetViewADMercenaryType(EmporiaViewADMercenaryType eType);
	EmporiaViewADMercenaryType GetViewADMercenaryType()const;

	int GetEmporiaChallengeLimitCost()const;

protected:
	PgEmporiaStateUI	m_kUIInfo;

public:
	bool IsHaveFunction( CONT_EMPORIA_FUNC::key_type nFuncNo )const{return m_kEmporiaFunction.IsHaveFunction(nFuncNo);}

protected:
	void SetResult( EEmporiaAdminstrationRet const eRet );

protected:
	PgEmporiaFunction		m_kEmporiaFunction;

private:
	EmporiaViewADMercenaryType	m_eViewADMercenaryType;
};

#define g_kEmporiaMgr SINGLETON_STATIC(PgEmporiaMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_EMPORIA_PGEMPORIAMGR_H