#ifndef FREEDOM_DRAGONICA_CONTENTS_COUPLE_PGCOUPLEMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_COUPLE_PGCOUPLEMGR_H

class PgCoupleMgr
{
	static size_t const iFindOnePageItems = 10;
public:

	PgCoupleMgr();
	~PgCoupleMgr();

	void Clear();
	void CallFindNext();
	void CallFindPrev();
	bool Have();
	bool IsSweetHeart();
	SCouple const& GetMyInfo()const		{ return m_kMyCouple; }
	bool ProcessPacket(BM::Stream &rkPacket);
	bool ProcessMarryPacket(BM::Stream &rkPacket);
protected:
	void AddSkill(int const iSkillNo);
	void Breaked(BM::GUID const &rkCharGuid);
	void CallAutoMatch(ContContentsUser const &rkVec) const;
	void CallFind(ContContentsUser const &rkVec, size_t const iPage = 0) const;
	void RecvAutoMatch(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvAnsCouple(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvAddSkill(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvReqBreak(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvReqCouple(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvReqFind(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvReqInfo(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvReqWarp(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvTimeLimit(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvReqStatus(BYTE const cCmdResult, BM::Stream &rkPacket);
	void MakeCouple(BM::GUID const &rkCharGuid, BM::GUID const &rkCoupleGuid);
	void UpdateItemWnd(XUI::CXUI_Wnd *pkItemWnd, SContentsUser const &rkUser) const;
	void UpdateBtnWnd(XUI::CXUI_Button *pkBtnWnd, SContentsUser const &rkUser) const;	

	void RecvReqSweetHeartQuest(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvAnsSweetHeartQuest(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvSweetHeartQuestTimeInfo(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvReqSweetHeartComplete(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvAnsSweetHeartComplete(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvGndCoupleStatus(BYTE const cCmdResult, BM::Stream &rkPacket);
	void RecvCoupleInit(BYTE const cCmdResult, BM::Stream &rkPacket);

	CLASS_DECLARATION_S_NO_SET(size_t, CurPage);
private:
	SCouple m_kMyCouple;
	ContContentsUser m_kFindResult;

public:
	void	SetSweetHeartQuestTimeUsed(bool bUse);
	void	SweetHeartQuestTimeUpdate(__int64 const iNowTime, __int64 const PlayTimeGap);
	void	CheckSweetHeartQuestTime();
	bool	GetPlayTimeUse();
	void	CoupleSweetHeartUI();
private:
	bool	bPlayTimeUse;
};

#define g_kCoupleMgr SINGLETON_STATIC(PgCoupleMgr)


//
class PgMapMoveCompleteEventMgr
{
	typedef std::map< BM::GUID, BYTE > ContEvent;
	typedef ContEvent::key_type key_type;
	typedef ContEvent::mapped_type mapped_type;
	typedef ContEvent::iterator iterator;
	typedef ContEvent::const_iterator const_iterator;
public:
	PgMapMoveCompleteEventMgr();
	~PgMapMoveCompleteEventMgr();
	void Clear();
	void Push(key_type const &rkKey, mapped_type const &rkVal);
	bool Pop(key_type const &rkKey);

protected:
	bool Process(key_type const &rkKey);

private:
	Loki::Mutex m_kMutex;
	ContEvent m_kEventMap;
};
#define g_kMapMoveCompleteEventMgr SINGLETON_STATIC(PgMapMoveCompleteEventMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_COUPLE_PGCOUPLEMGR_H