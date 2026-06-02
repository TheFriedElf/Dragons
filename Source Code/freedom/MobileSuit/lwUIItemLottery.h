#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMLOTTERY_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMLOTTERY_H

#include "XUI/XUI_Manager.h"

class lwUIItemLottery
{
public:
	lwUIItemLottery(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

	void DisplaySrcItem();
	void SendReqItemLottery(bool bIsSend);
	void Clear();
	bool IsAvailable();

protected:
	XUI::CXUI_Wnd *self;
};




class PgItemLottery
{
public:
	typedef struct tagLotteryElement
	{
		tagLotteryElement()
		{
			kItem.Clear();
			iItemCount = 0;
			kSrcItem = BM::GUID();
		}
		PgBase_Item kItem;
		SItemPos kItemPos;
		int iItemCount;
		BM::GUID kSrcItem;//아이템 위치가 변경되거나 할 수 있으므로.
	}SLotteryElement;
	typedef std::unordered_map< int, SLotteryElement >	CONT_LOTTERYELEMENT;

public:
	PgItemLottery();
	virtual ~PgItemLottery();

	void Clear();
	bool DragToSlot(const SItemPos &rkItemPos, int iBuildIndex);
	bool SetSrcItem(const SItemPos &rkItemPos, int iBuildIndex, int iItemCount);
	void DisplaySrcItem(XUI::CXUI_Wnd *pWnd, int iBuildIndex);
	bool IsAvailable()const;
	bool SendReqItemLottery(bool bIsSend)const;

protected:

	CONT_LOTTERYELEMENT m_kLotteryElement;
};

#define g_kItemLottery SINGLETON_STATIC(PgItemLottery)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMLOTTERY_H