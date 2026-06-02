#ifndef FREEDOM_DRAGONICA_CONTENTS_SUPERGROUND_PGSUPERGOUNDUI_H
#define FREEDOM_DRAGONICA_CONTENTS_SUPERGROUND_PGSUPERGOUNDUI_H
namespace PgSuperGroundUI
{
	bool IsCanUseFloorMoveItem(int const iItemCustomValue);
	void Clear();
	void ReadFromMapMove(BM::Stream& rkPacket);
	void ProcessMsg(BM::Stream::DEF_STREAM_TYPE const wPacketType, BM::Stream& rkPacket);

	bool RegisterWrapper(lua_State *pkState);
};

//
int const MAX_SUPER_GROUND_ITEM = 4;
struct SSuperGroundModeItem
{
	SSuperGroundModeItem();
	SSuperGroundModeItem(SSuperGroundModeItem const& rhs);
	~SSuperGroundModeItem();
	bool ParseXml(TiXmlElement const* pkNode);

	int iClass;
	CONT_ITEMNO kItemNo;
};
typedef std::list< SSuperGroundModeItem > CONT_SUPER_GROUND_MODE_ITEM;
struct SSuperGroundMode
{
	SSuperGroundMode();
	SSuperGroundMode(SSuperGroundMode const& rhs);
	bool ParseXml(TiXmlElement const* pkNode);
	SSuperGroundModeItem GetItem() const;

	int iTitleTextID;
	std::wstring kBtnImgName;
	CONT_SUPER_GROUND_MODE_ITEM kContItem;
	SSuperGroundModeItem kDefaultItem;
};
struct SSuperGroundEnterUI
{
	SSuperGroundEnterUI();
	SSuperGroundEnterUI(SSuperGroundEnterUI const& rhs);

	bool ParseXml(TiXmlElement const* pkNode);

	int iTitleTextID;
	int iDescTextID;
	int iSuperGroundNo;
	SSuperGroundMode kMode[MAX_SUPER_GROUND_MODE_NO];
};
typedef std::map< int, SSuperGroundEnterUI > CONT_SUPER_GROUND_ENTER;
//
class PgSuperGroundEnterUIMgr : public PgIXmlObject
{
public:
	PgSuperGroundEnterUIMgr();
	~PgSuperGroundEnterUIMgr();

	void Clear();
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	void Call(int const iSuperGroundNo, std::string const& rkTriggerID) const;
	void CallMiniItemHelp(int const iSuperGroundNo, int const iModeNo, int iItemNo = 0, int iIncVal = 0) const;
	bool IsHaveItemHelp(int const iSuperGroundNo, int const iModeNo, int iItemNo = 0, int iIncVal = 0) const;

private:
	void UpdateEnterUI(CONT_SUPER_GROUND_ENTER::mapped_type const& rkSuperGroundUI, std::string const& rkTriggerID) const;
	void UpdateModeBtn(XUI::CXUI_Wnd* pkTopWnd, int const iModeNo, SSuperGroundMode const& rkMode, BM::Stream const& rkCustomData) const;

private:
	CONT_SUPER_GROUND_ENTER m_kContEnter;
};
#define g_kSuperGroundEnterUIMgr SINGLETON_STATIC(PgSuperGroundEnterUIMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_SUPERGROUND_PGSUPERGOUNDUI_H