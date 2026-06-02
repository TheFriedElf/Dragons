#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_PGQUEST_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_PGQUEST_H
using namespace XUI;

extern void Net_PT_M_C_ANS_NPC_QUEST(BM::Stream *pkPacket);
extern void Net_PT_M_C_SHOWDIALOG(BM::Stream *pkPacket);
extern void UpdateQuestMiniIngToolTip(int const iQuestID, CXUI_Wnd const* pkTopWnd = NULL);

class PgQuestMiniListMng
{
	//
	typedef struct tagMiniState
	{
		tagMiniState(SUserQuestState const &rkState);

		void UserState(SUserQuestState const &rkUserState);

		int ID() const										{ return m_kUserState.iQuestID; }
		BYTE State() const									{ return m_kUserState.byQuestState; }
		int GetParamCondition() const						{ return m_kUserState.byParam[iCurChangedParam]; }
		int GetParamCondition(size_t const iCur) const		{ return m_kUserState.byParam[iCur]; }

		size_t iOldChangedParam;
		size_t iCurChangedParam;
		CLASS_DECLARATION_S_NO_SET(SUserQuestState, UserState);
		CLASS_DECLARATION_S(bool, New);
	} SMiniState;
	typedef std::map< size_t, SMiniState > ContMiniState;

	//
	typedef struct tagCompareHelper
	{
	private:
		typedef ContMiniState::value_type pair_type;

	public:
		tagCompareHelper(SUserQuestState const &rkLeft);
		bool operator() (SUserQuestState const &rkRight) const;
		bool operator() (pair_type const &rkRight) const;
	private:
		SUserQuestState const &m_kLeft;
	} SCompareHelper;

public:
	PgQuestMiniListMng();
	~PgQuestMiniListMng();

	void Save();
	void Load();
	void Destroy();

	bool Add(int const iQuestID);
	bool Del(int const iQuestID);
	bool IsInList(int const iQuestID) const;
	bool Update(); // MiniQuest 목록에 추가된 갯수를 업데이트 한다.
	size_t Count() const;
	void Visible(bool const bVisible); // 진행중인 퀘스트의 미니 인터페이스를 보여준다.

protected:
	void UpdateAll();
	void CloseAll();
	void UpdateUI(size_t const iNo, SMiniState const &rkState);
	void CloseUI(size_t const iNo);
	std::wstring GetUIName(size_t const iNo);
	bool GetEmptySlotNo(size_t& riOut) const;

	CLASS_DECLARATION_NO_SET(int, m_iPrevMiniListCount, PrevMiniListCount);
	CLASS_DECLARATION_NO_SET(int, m_iCurMiniListCount, CurMiniListCount);
	CLASS_DECLARATION_NO_SET(bool, m_bMiniQuestStatus, MiniQuestStatus);
	CLASS_DECLARATION_S_NO_SET(bool, FirstRun);

private:
	//Loki::Mutex m_kMiniMutex;
	ContMiniState m_kIng;
};


//
class PgMiniIngToolTip
{
public:
	PgMiniIngToolTip();
	~PgMiniIngToolTip();

	void Add(std::wstring const& rkDesc, int const iCur, int const iMax);
	void Add(std::wstring const& rkDesc);
	bool Call(XUI::CXUI_Wnd const* pkTopWnd, PgQuestInfo const* pkQuestInfo = NULL);
private:
	std::wstring m_kMsg;
	int m_iCount;
};
#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_PGQUEST_H