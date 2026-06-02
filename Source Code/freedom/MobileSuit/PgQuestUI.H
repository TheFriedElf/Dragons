#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTUI_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTUI_H

#define USE_HEAD_SNAP (1)

using namespace XUI;

class PgQuestUI : public CXUI_Form
{
public:

	PgQuestUI() {};
	virtual	~PgQuestUI() {};

	CLASS_DECLARATION_S(bool, Udpated);
};

typedef enum eSnapPosType
{
	SnapPT_None = 0,
	SnapPT_Top,
	SnapPT_Bottom,
	SnapPT_Left,
	SnapPT_Right,
} ESnapPosType;

//
typedef std::vector< std::wstring > ContWndID;
class PgFormSnapedWnd : public CXUI_Wnd
{
	typedef struct tagSnapSaveInfo
	{
		std::wstring kSnapParentID;
		bool bSnaped;
		ContWndID kSnapChildTop;
		ContWndID kSnapChildBottom;
		ContWndID kSnapChildLeft;
		ContWndID kSnapChildRight;
		POINT3I kLocation;
		void WriteToPacket(BM::Stream& rkPacket) const;
		void ReadFromPacket(BM::Stream& rkPacket);
		size_t min_size() const;
	} SSnapSaveInfo;
	typedef std::map< std::wstring, SSnapSaveInfo > ContSnapSaveInfo;
	typedef std::map< size_t, ContSnapSaveInfo > ContSnapGroupSaveInfo;
	typedef std::map< size_t, ContWndID > ContSnapGroup;
public:
	PgFormSnapedWnd();
	virtual ~PgFormSnapedWnd();

	virtual bool VDisplay();
	virtual void VOnClose();
	virtual void VOnCall();
	virtual void VRegistAttr(std::wstring const &wstrName, std::wstring const &wstrValue);
	virtual void Size(POINT2 const &rPT, bool const bIsModifyBoundingBox = true );

	CLASS_DECLARATION(size_t, m_iSnapGroup, SnapGroup);

	void RegistSnapChild(std::wstring const &rkID, ESnapPosType const eSanpPos);
	ESnapPosType RemoveSnapChild(std::wstring const &rkID);
	ESnapPosType FindSnapChild(std::wstring const &rkID);
	void LocationSnapChild(POINT3I const &rPT, bool const bIsModifyBoundingBox);
	void NewSnapParnet(std::wstring const &rkNewParentID);
	void ClearSnapChild();
	void ClearSnapParent()			{ m_kSnapParentID.clear(); }
#ifdef USE_HEAD_SNAP
	int GetSnapedNextY();
#else
	int GetSnapedNextY(std::wstring &rkOutID);
#endif
	void ReAlignSnapChild(ESnapPosType const eType);
	void SaveSnapInfo(SSnapSaveInfo &rkOut) const;
	void LoadSnapInfo(SSnapSaveInfo const &rkIn);
	static void SaveSnapInfo(size_t const iGroup);
	static void LoadSnapInfo(size_t const iGroup);
	static void WriteToPacketSnapInfo(size_t const iGroup, BM::Stream& rkPacket);
	static void ReadFromPacketSnapInfo(size_t const iGroup, BM::Stream& rkPacket);

	CLASS_DECLARATION_S_NO_SET(std::wstring, SnapParentID);
	CLASS_DECLARATION_NO_SET(bool, m_bSnaped, Snaped);
protected:
	static ContSnapGroup m_kSnapGroup;
	static void RegistSnapGroup(size_t const iGroup, std::wstring const &rkID);
	static void RemoveSnapGroup(size_t const iGroup, std::wstring const &rkID);
	static void GetSnapGroup(size_t const iGroup, ContWndID &rkOut);
	static bool CanRegistSnapChild(std::wstring const &rkMyID, PgFormSnapedWnd *pkParent);

	static ContSnapGroupSaveInfo m_kSaveSnapInfo;

private:
	ContWndID m_kSnapChildTop;
	ContWndID m_kSnapChildBottom;
	ContWndID m_kSnapChildLeft;
	ContWndID m_kSnapChildRight;
};

//
class PgFormSnapedHeadWnd : public PgFormSnapedWnd
{
public:
	PgFormSnapedHeadWnd();
	virtual ~PgFormSnapedHeadWnd();

	virtual CXUI_Wnd* VCreate()const	{ return new PgFormSnapedHeadWnd; }

	virtual void Location(POINT3I const &rPT, bool const bIsModifyBoundingBox = true);
	void ReLocationSnapChild();
};

//
class PgFormSnapedSubWnd : public PgFormSnapedWnd
{
public:
	PgFormSnapedSubWnd();
	virtual ~PgFormSnapedSubWnd();

	virtual void VOnCall();
	virtual CXUI_Wnd* VCreate()const	{ return new PgFormSnapedSubWnd; }
	virtual void Location(POINT3I const &rPT, bool const bIsModifyBoundingBox = true);

	void RegistHeadSnapChild();

	CLASS_DECLARATION_S_NO_SET(POINT2, LastLocation);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_UI_PGQUESTUI_H