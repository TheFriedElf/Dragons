#ifndef FREEDOM_DRAGONICA_CONTENTS_EMPORIA_PGBREIFINGLOADUIMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_EMPORIA_PGBREIFINGLOADUIMGR_H

namespace lwUIEmBattleLoad
{
	void RegisterWrapper(lua_State *pkState);
	bool lwSetBriefingLoadUI(int const iMapNo, int const iProgress);
	void lwCloseBriefingOBJUI();
};

typedef struct tagBriefingPopUIInfo
{
	tagBriefingPopUIInfo()
		: kPos(), bCopy(false), kUIName(){}
	void operator = (tagBriefingPopUIInfo const& rhs)
	{
		kPos = rhs.kPos;
		bCopy = rhs.bCopy;
		kUIName = rhs.kUIName;
	}

	POINT2	kPos;
	bool	bCopy;
	std::wstring kUIName;
}SBriefingPopUIInfo;

typedef std::list< SBriefingPopUIInfo >	KCONT_BRIEFING_POP_UI;

typedef struct tagBriefingTickItem
{
	bool operator < (tagBriefingTickItem const& rhs) const
	{
		return (iTick < rhs.iTick)?(true):(false);
	}
	void operator = (tagBriefingTickItem const& rhs)
	{
		iTick = rhs.iTick;
		iIdx = rhs.iIdx;
		iTitleTTW = rhs.iTitleTTW;
		iContentTTW = rhs.iContentTTW;
		kUIList.insert(kUIList.end(), rhs.kUIList.begin(), rhs.kUIList.end());
	}

	int iTick;
	int iIdx;
	int iTitleTTW;
	int iContentTTW;
	KCONT_BRIEFING_POP_UI kUIList;
}SBriefingTickItem;

typedef std::map< int, SBriefingTickItem > KCONT_BRIEFING_TICK_ITEM;

typedef struct tagBriefingMapInfo
{
	void operator = (tagBriefingMapInfo const& rhs)
	{
		iMapNo = rhs.iMapNo;
		kImgPath = rhs.kImgPath;
		kSize = rhs.kSize;
		kTickCont.insert(rhs.kTickCont.begin(), rhs.kTickCont.end());
	}

	int iMapNo;
	std::wstring kImgPath;
	POINT2	kSize;
	KCONT_BRIEFING_TICK_ITEM kTickCont;
}SBriefingMapInfo;

typedef std::map< int, SBriefingMapInfo > KCONT_BRIEFING_MAP_INFO;
typedef std::list< XUI::CXUI_Wnd* > KCONT_BRIEFING_CALLED_UI;

class PgBriefingLoadUIMgr
{
public:
	bool ParseXML(char const* pcXmlPath);
	bool GetBriefingInfo(int const iMapNo, SBriefingMapInfo& kInfo);
	XUI::CXUI_Wnd* GetOBJUI(std::wstring const& kUIName, bool const bIsCopyUI = false);
	void ClearAllOBJUI();

	PgBriefingLoadUIMgr(void);
	virtual ~PgBriefingLoadUIMgr(void);

protected:
	bool RecursiveParseNode(TiXmlElement const* pkElement, KCONT_BRIEFING_MAP_INFO* pkMapCont = NULL, KCONT_BRIEFING_TICK_ITEM* pkTickCont = NULL, KCONT_BRIEFING_POP_UI* pkUICont = NULL);
	bool _ParseMapAttr(TiXmlAttribute const* pkAttr, SBriefingMapInfo& kMapInfo);
	bool _ParseTickAttr(TiXmlAttribute const* pkAttr, SBriefingTickItem& kTickItem);
	bool _ParsePopUIAttr(TiXmlAttribute const* pkAttr, SBriefingPopUIInfo& kUIInfo);

private:
	KCONT_BRIEFING_MAP_INFO		m_kBriefingAtkCont;
	KCONT_BRIEFING_MAP_INFO		m_kBriefingDefCont;
	KCONT_BRIEFING_CALLED_UI	m_kUICont;
	CLASS_DECLARATION_S(bool, Attacker);
	CLASS_DECLARATION_S(int, Idx);
};

#define g_kBriefingLoadUIMgr SINGLETON_STATIC(PgBriefingLoadUIMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_EMPORIA_PGBREIFINGLOADUIMGR_H