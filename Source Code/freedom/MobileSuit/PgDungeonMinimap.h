#pragma once

#include "XUI/XUI_Wnd.h"
#include <map>
#include <vector>


typedef struct tagPROGRESSINFO
{
	enum EDIRECTION
	{
		ED_LEFT,
		ED_RIGHT
	};

	enum ETYPE : BYTE
	{
		ET_NONE = 0x01,
		ET_CURRENT = 0x02,
		ET_VISITED = 0x04,
		ET_START = 0x08,
		ET_MAIN_ROUTE = 0x10,
		ET_SUB_ROUTE = 0x20,
		ET_BOSS = 0x40,
	};

	int ix, iy;
	EDIRECTION eDir;
	BYTE kType;

	tagPROGRESSINFO() : ix(0), iy(0), eDir(ED_RIGHT), kType(ET_NONE)
	{}
}PROGRESSINFO;

typedef struct tagPROGRESS_MAP_UI_INFO
{
	int iImgW, iImgH;
	bool bShowNpcList;
	std::wstring wstrImgPath;

	tagPROGRESS_MAP_UI_INFO() : iImgW(0), iImgH(0), wstrImgPath(), bShowNpcList(false)
	{}
}PROGRESS_MAP_UI_INFO;

typedef struct tagPROGRESS_WAY_INFO
{
	enum ETYPE : BYTE
	{
		ET_NONE = 0,
		ET_VISITED = 0x01,
		ET_MAIN_ROUTE = 0x02,
		ET_SUB_ROUTE = 0x04,
		ET_CUTTED_ROUTE = 0x08,
	};

	size_t iFrom, iTo;
	BYTE kType;

	tagPROGRESS_WAY_INFO(size_t iInitFrom, size_t iInitTo, ETYPE kInitType) : iFrom(iInitFrom), iTo(iInitTo), kType(kInitType)
	{}
	tagPROGRESS_WAY_INFO() : iFrom(0), iTo(0), kType(ET_NONE)
	{}
	bool operator == (tagPROGRESS_WAY_INFO const& rhs) const
	{
		return (iFrom == rhs.iFrom && iTo == rhs.iTo) || (iFrom == rhs.iTo && iTo == rhs.iFrom);
	}
protected:

}PROGRESS_WAY_INFO;



class PgDungeonMinimap
{
public:
	PgDungeonMinimap(void);
	~PgDungeonMinimap(void);

	bool CallUI(bool bShowToggle = true);
	bool SetMainUI(PROGRESS_MAP_UI_INFO const& kUIInfo, bool bShow = true);
	bool AddProgressArea(size_t iIndex, PROGRESSINFO const& kElem);
	bool AddProgressWay(PROGRESS_WAY_INFO const& kElem); 
	void SetCurPosPartyMember(size_t iPos) { m_iCurPos = iPos; } // 파티난입시에만 설정
	bool SetCurPos(size_t iPos, bool bInit = false);
	size_t GetCurPos(void) const { return m_iCurPos; }
	void Cleanup(void);
	PROGRESS_MAP_UI_INFO const& GetInfo(void) const { return m_kUIInfo; }

protected:
	typedef std::map<int, PROGRESSINFO> CONT_PROGRESS_INFO;
	typedef std::vector<PROGRESS_WAY_INFO> CONT_PROGRESS_WAY;

	PROGRESS_MAP_UI_INFO m_kUIInfo;
	CONT_PROGRESS_INFO m_kContProgInfo;
	CONT_PROGRESS_WAY m_kContProgWay;

	size_t m_iCurPos;

	XUI::CXUI_Wnd* m_pkTopWnd;
	static const wchar_t* m_wszTopWndID;
	static const wchar_t* m_wszIconWndBaseID;
	static const wchar_t* m_wszIconWndRunRightID;
	static const wchar_t* m_wszIconWndRunLeftID;
	static const wchar_t* m_wszIconWndStarID;
	static const wchar_t* m_wszLineWndID;
	static const int m_iDungeonMinimapMargin;
	static const std::wstring wstrImgLine[6];

protected:
	XUI::CXUI_Wnd* GetMinimap(void);
	void Init(XUI::CXUI_Wnd* pkTopWnd, PROGRESS_MAP_UI_INFO const& kUIInfo);
	PROGRESSINFO* FindElem(size_t iPos);
	bool UpdateIconObject(XUI::CXUI_Wnd* pkTopWnd, size_t iPos, PROGRESSINFO const& rkProgElem);
	bool UpdateWay(XUI::CXUI_Wnd* pkTopWnd, size_t iModifiedElem);
	bool UpdateWayUI(XUI::CXUI_Wnd* pkTopWnd, size_t iWayNum, PROGRESS_WAY_INFO const& rkWayElem, PROGRESSINFO const& rkElemV1, PROGRESSINFO const& rkElemV2);
};

#define g_kProgressMap SINGLETON_CUSTOM(PgDungeonMinimap, CreateUsingNiNew)
