#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIPET_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIPET_H

enum E_PET_TOUCH_ACTION
{
	EPA_MEAL		= 0,
	EPA_TRAINING,
	EPA_GAME,
};

class PgPilot;

class PgPetUIManager
{
public:
	typedef struct stFeedItem
	{
		SItemPos m_kFeedItemPos;	//사용한 사료 아이템 위치
		SItemPos m_kPetItemPos;		//사료를 사용할 펫 아이템 위치
		PgBase_Item m_kFeedItem;	//사용한 사료 아이템
		PgBase_Item m_kPetItem;		//사료를 사용할 펫 아이템
		int m_iPetResNo;
		stFeedItem() {Clear();}
		void Clear()
		{
			m_kFeedItemPos.Clear();
			m_kPetItemPos.Clear();
			m_kFeedItem.Clear();
			m_kPetItem.Clear();
			m_iPetResNo = 0;
		}
	}SFeedItem;

	void Clear();
	void DisplaySrcItem(XUI::CXUI_Wnd *pWnd, int const iNo = 0);
	void UseFeedItem(SItemPos const & rkItemInvPos, int const iNo = 0);
	bool SetSrcItem(SItemPos const &rkItemPos, int const iNo = 0);
	bool SetSrcItem(SItemPos const &rkItemPos, PgInventory* pkInv, XUI::CXUI_Wnd *pWnd);
	bool SetFeedItem(SItemPos const  &rkItemPos, int const iNo = 0);
	void SendReqFeedPet(bool bIsTrueSend, int const iNo = 0);
	void CallResultUI( PgItemWrapper const& rkItemWrap, PgItemWrapper const& rkPrevItemWrap, PgInventory *pkInv);

	static bool RegisterWrapper(lua_State *pkState);
protected:
	static int const MAX_FEEDITEM = 5;
	SFeedItem m_kFeedItem[MAX_FEEDITEM];
};

#define g_kPetUIMgr SINGLETON_STATIC(PgPetUIManager)

class lwUIWnd;

class PgPetExpItemUIManager
{
public:
	typedef struct stExpItem
	{
		BM::GUID	m_kSrcGuid;//같은 아이템 중복을 막기 위해 GUID를 저장하자
		SItemPos	m_kSrcItemPos;
		PgBase_Item	m_kSrcItem;
		int			m_iSrcResNo;

		PgBase_Item	m_kResultItem;
		int			m_iResultResNo;

		stExpItem() {Clear();}
		void Clear()
		{
			m_kSrcGuid.Clear();
			m_kSrcItemPos.Clear();
			m_kSrcItem.Clear();
			m_iSrcResNo = 0;

			m_kResultItem.Clear();
			m_iResultResNo = 0;	
		}
	}SExpItem;

	void Clear();
	void DisplaySrcItem(XUI::CXUI_Wnd *pWnd, int const iNo);
	void DisplayResultItem(XUI::CXUI_Wnd *pWnd, int const iNo);
	bool SetSrcItem(SItemPos const &rkItemPos, int const iNo);

	void SendChangeExpItem(XUI::CXUI_Wnd* pkWnd, bool const bIsTrueSend, bool const bClear);

	CLASS_DECLARATION_S(BM::GUID, NpcGuid);

	static bool RegisterWrapper(lua_State *pkState);
protected:
	static int const MAX_EXPITEM = 6;
	SExpItem m_kExpItem[MAX_EXPITEM];
};

#define g_kPetExpUIMgr SINGLETON_STATIC(PgPetExpItemUIManager)

extern PgPet* GetMySelectedPet();
extern PgInventory* GetMySelectedPetInven();
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIPET_H