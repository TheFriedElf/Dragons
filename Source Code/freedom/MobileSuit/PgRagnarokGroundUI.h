#ifndef FREEDOM_DRAGONICA_CONTENTS_RAGNAROKGROUND_PGRAGNAROKGOUNDUI_H
#define FREEDOM_DRAGONICA_CONTENTS_RAGNAROKGROUND_PGRAGNAROKGOUNDUI_H

class lwTrigger;
class lwGUID;

int const MAX_RAGNAROK_GROUND_ITEM = 9;
int const MAX_RAGNAROK_GROUND_MODE_NO = 3;

typedef enum eRagnarokGroundMode
{
	RGM_NORMAL = 0,
	RGM_RARE = 1,
	RGM_HERO = 2,
} ERagGroundMode;

namespace PgRagnarokGroundUI
{
	void ProcessMsg(BM::Stream::DEF_STREAM_TYPE const PacketType, BM::Stream& Packet);
	bool RegisterWrapper(lua_State *pState);

	void lwCallRagUI(int const GroundKey, char const* TriggerName);
	void lwEnterRagDungeon(int const ModeNo);
	
	void lwReqRagnarokBless(lwGUID lwNpcGuid); // 엔피시에게 버프 요청(조건 없이 즉시 동작)

	bool CheckCanEnterDungeon(int const ModeNo);
	bool CheckCanEnterLevel(int const PlayerLevel, int const ModeNo);
	bool FindCousumItem(ContHaveItemNoCount & ContItemNoCount);
};

struct SRagnarokGroundModeItem
{
	SRagnarokGroundModeItem();
	SRagnarokGroundModeItem(SRagnarokGroundModeItem const& rhs);
	~SRagnarokGroundModeItem();
	bool ParseXml(TiXmlElement const* pNode);

	int ClassNo;
	CONT_ITEMNO Cont_ItemNo;
};
typedef std::list< SRagnarokGroundModeItem > CONT_RAGNAROK_GROUND_MODE_ITEM;
struct SRagnarokGroundMode
{
	SRagnarokGroundMode();
	SRagnarokGroundMode(SRagnarokGroundMode const& rhs);
	bool ParseXml(TiXmlElement const* pNode);
	SRagnarokGroundModeItem GetItem() const;

	int TitleTextID;
	int MinLevel;
	int MaxLevel;
	std::wstring BtnImgName;
	CONT_RAGNAROK_GROUND_MODE_ITEM ContItem;
	SRagnarokGroundModeItem DefaultItem;
};
struct SRagnarokGroundEnterUI
{
	SRagnarokGroundEnterUI();
	SRagnarokGroundEnterUI(SRagnarokGroundEnterUI const& rhs);

	bool ParseXml(TiXmlElement const* pNode);

	int TitleTextNo;
	int ContentTextNo;
	int GroundKey;
	SRagnarokGroundMode m_mode[MAX_RAGNAROK_GROUND_MODE_NO];
};
typedef std::map< int, SRagnarokGroundEnterUI > CONT_RAGNAROK_GROUND_ENTER;
//
class PgRagnarokGroundEnterUIMgr : public PgIXmlObject
{
public:
	PgRagnarokGroundEnterUIMgr();
	~PgRagnarokGroundEnterUIMgr();

	void Clear();
	virtual bool ParseXml(const TiXmlNode *pNode, void *pArg = 0, bool bUTF8 = false);
	void Call(int const GroundKey, char const* TriggerName);
	bool CheckCanEnterLevel(int const PlayerLevel, int const ModeNo);

	std::wstring GetTriggerName() const;
private:
	void UpdateEnterUI(CONT_RAGNAROK_GROUND_ENTER::mapped_type const& RagnarokGroundUI) const;
	void UpdateModeBtn(XUI::CXUI_Wnd * pWnd, int const MyLevel, ERagGroundMode const& Mode, SRagnarokGroundMode const& RagnarokGroundUI) const;
	void UpdateModeItem(XUI::CXUI_Wnd * pWnd, int const MyClass, ERagGroundMode const& Mode, SRagnarokGroundMode const& RagnarokGroundUI) const;
private:
	int m_GroundKey;
	std::wstring m_TriggerName;
	CONT_RAGNAROK_GROUND_ENTER m_ContEnter;
};
#define g_kRagnarokGroundEnterUIMgr SINGLETON_STATIC(PgRagnarokGroundEnterUIMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_RAGNAROKGROUND_PGRAGNAROKGOUNDUI_H