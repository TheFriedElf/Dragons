#ifndef FREEDOM_DRAGONICA_CONTENTS_CASHITEM_LWTELECARDUI_H
#define FREEDOM_DRAGONICA_CONTENTS_CASHITEM_LWTELECARDUI_H

typedef std::map< int, int >	kMapCont;
typedef struct tagShardInfo
{
	tagShardInfo()
		: bIsOpen(false)
		, kCont()
	{
	}
	bool		bIsOpen;
	kMapCont	kCont;
}SShardInfo;
typedef std::map< int, SShardInfo > kShardCont;

namespace lwTeleCardUI
{
	void RegisterWrapper(lua_State *pkState);
	void lwAddTeleCarMapInfo(int const iShardNo, int const iIndex, int const iMapNo);
	void lwCheckOpenShard();
	void lwSendMapMove(lwUIWnd UISelf);
	void lwSelectShard(lwUIWnd UISelf);

	void CallTeleCardUI(DWORD const kItemNo);
}

class PgTeleCard
{
public:
	void AddTeleCarMapInfo(int const iShardNo, int const iIndex, int const iMapNo);
	void InitUI(XUI::CXUI_Wnd* pParent);
	void SetShardAreaItem(XUI::CXUI_List* pList, int const iShardNo);
	void CheckOpenShard();

	int GetShardCount() const { return m_kShard.size(); };

	PgTeleCard(void);
	virtual ~PgTeleCard(void);

private:
	kShardCont	m_kShard;
};

#define	g_kTeleCard	SINGLETON_STATIC(PgTeleCard)

#endif // FREEDOM_DRAGONICA_CONTENTS_CASHITEM_LWTELECARDUI_H