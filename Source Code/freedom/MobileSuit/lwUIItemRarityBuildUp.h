#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMRARITYBUILDUP_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMRARITYBUILDUP_H

#include "PgScripting.h"
#include "lwPacket.h"
#include "XUI/XUI_Manager.h"
#include "lwUI.h"

namespace lwUIItemRarityBuildUp
{
	extern void CallItemBuildUpSelectUI();
	extern void CallItemBuildUpUI();
	extern void Clear();

	extern void DisplaySrcIcon();
	extern void DisplayResultItem();

	extern void OnTick();
	
	extern bool CheckUI(lwUIWnd kWnd);
	extern bool Check();

	extern bool Start();
	extern bool Cancle();
	extern void RegisterWrapper(lua_State *pkState);
};

typedef enum eRarityBuildUpState
{
	E_RS_READY = 0, //대기중
	E_RS_PROCESS = 1, //연출중
	E_RS_WAIT_ANS = 2, //서버 응답을 대기하는중
	E_RS_CLEAR_DELAY = 3,//초기화 딜레이중
}E_RARITY_BUILDUP_STATE;

typedef std::map<SItemPos,size_t> CONT_POST_ITEMCOUNT;

class PgItemRarityBuildUpMgr
{
public:
	PgItemRarityBuildUpMgr();
	virtual ~PgItemRarityBuildUpMgr() {}

public:
	void Clear();

	void DisplaySrcIcon();
	void DisplayResultItem();

	void SetItem(EKindUIIconGroup const kType, SIconInfo const & rkInfo);

	void OnTick();

	bool Check();
	bool Check(SItemPos const &rkItemPos);
	bool Start();
	bool Cancle();

	void Recv(BM::Stream &rkPacket);

	SItemPos const& GetSrcItemPos() const { return m_kTargetItemPos; }

	SItemPos GetNeedItemPos();
private:
	bool FindCraftOption(PgBase_Item const& rkItem, CItemDef const* pkItemDef);
	void ClearUI();
	size_t GetBuildUpCount();
	bool CheckNeedItem(size_t const iBuildUpOptionCount, CONT_HAVE_ITEM_POS& rkContHaveItemPos);

	bool IsLowOptionValue(PgBase_Item const& rkItem, CItemDef const* pkItemDef, int const iOptIndex, int const iCurOptLevel, int const iTargetOptLevel);
	bool SetOptionText(int const iOptIndex, XUI::CXUI_Wnd* pkWnd);
	bool GetChangedOptionText(int const iIndex, int const iNewLevel, std::wstring& rkOutText);
	void GetOptionInfo(PgBase_Item& rkItem, int const iIndex, int& iOutType, int& iOutLevel, int& iOutAmpLevel);

	bool Send_ReqBuildUp();
	bool Send();

private:
	SItemPos m_kNeedItemPos;
	SItemPos m_kTargetItemPos;

	E_RARITY_BUILDUP_STATE m_eState;
	PgBase_Item m_kTargetItem;
	PgBase_Item m_kNeedtItem;

	float m_fDelayStartTime;
	float m_fDelayTime;
	float m_fStartTime;
	int m_iProcessIndex;

	std::queue<int> m_kContReserveIndex;
	std::map<int, BM::vstring> m_kContResultText;

	CONT_POST_ITEMCOUNT	m_kContPosItemCount;
};

#define g_kItemRarityBuildUpMgr SINGLETON_STATIC(PgItemRarityBuildUpMgr)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMRARITYBUILDUP_H