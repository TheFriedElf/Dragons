#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUISHINESTONE_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUISHINESTONE_H

#include "XUI/XUI_Manager.h"

class lwUIShineStoneUpgrade
{
public:
	lwUIShineStoneUpgrade(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

	void Clear();
	void SetDefItemMakingNo(int iIdx, int iMakingNo);
	void SelectItem();
	void DisplayTodoItem();
	void DisplayDetailSrcItem();
	void DisplayDetailDestItem();	
	void MouseOver();
	void MouseOut();
	void DisplayDetailText();
	void SendReqUpgrade();

protected:
	XUI::CXUI_Wnd *self;
};

class PgShineStoneUpgrade
{
public:
	typedef struct tagUpgradeElement
	{
		tagUpgradeElement()
		{
			iMakingNo = 0;
			iItemNo = 0;
			iResultItemNo = 0;
			iItemCount = 0;
			iResultItemCount = 0;
		}
		int iMakingNo;
		int iItemNo;
		int iItemCount;
		int iResultItemNo;
		int iResultItemCount;
	}SUpgradeElement;
	typedef std::unordered_map< int, SUpgradeElement > CONT_UPGRADEELEMENT;
public:
	PgShineStoneUpgrade();
	virtual ~PgShineStoneUpgrade();

	void Clear();
	void SetDefItemMakingNo(int iIdx, int iMakingNo);
	void SelectItem(XUI::CXUI_Wnd *pWnd, int iBuildIndex);
	void DisplayTodoItem(XUI::CXUI_Wnd *pWnd, int iBuildIndex);
	void DisplayDetail(XUI::CXUI_Wnd *pWnd, int iItemNo, int iItemCount);
	void DisplayDetailSrcItem(XUI::CXUI_Wnd *pWnd);
	void DisplayDetailDestItem(XUI::CXUI_Wnd *pWnd);
	void MouseOver(XUI::CXUI_Wnd *pWnd, int iBuildIndex);
	void MouseOut();
	void DisplayDetailText(XUI::CXUI_Wnd *pWnd);
	void SendReqUpgrade();

protected:
	CONT_UPGRADEELEMENT m_kUpgradeElement;
	int m_iSelectIndex;
	std::wstring m_wstrDetailText;
};
#define g_kShineStoneUpgrade SINGLETON_STATIC(PgShineStoneUpgrade)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUISHINESTONE_H