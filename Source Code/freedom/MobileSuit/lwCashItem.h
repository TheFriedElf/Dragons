#ifndef FREEDOM_DRAGONICA_CONTENTS_CASHITEM_LWITEM_H
#define FREEDOM_DRAGONICA_CONTENTS_CASHITEM_LWITEM_H

#include "PgUIDataMgr.h"

//------------------------------------------------------------------------------------------------------------------
//	Style Change UI
//------------------------------------------------------------------------------------------------------------------
enum E_STYLE_TYPE
{
	ESTYLE_BEGIN	= 0,
	ESTYLE_COLOR	= 0,
	ESTYLE_HAIR		= 1,
	ESTYLE_FACE,
	ESTYLE_END,
};

void PgCashItemUIRegWrapper(lua_State *pkState);

namespace lwStyleModifier
{
	void RegisterWrapper(lua_State *pkState);

	void lwStyleChangeTabControl(lwUIWnd UISelf);
	void lwSetChangeStyleConfirm(lwUIWnd UISelf);
	void lwCloseStyleUI();
	void lwSelectStyleItem(lwUIWnd UISelf);
	void lwDrawStyleItem(lwUIWnd UISelf);
	void lwCallStyleItemToolTip(lwUIWnd UISelf);
	void lwClearStyleItem(lwUIWnd UISelf);

	void CallStyleModifyUI(E_STYLE_TYPE const Type, DWORD const dwItemNo);
	void SetStyleUI(XUI::CXUI_Wnd* pStyleUI, E_STYLE_TYPE const Type);
	void SetStyleItemInv(XUI::CXUI_Wnd* pItemUI, E_STYLE_TYPE const Type);
	void SetSelectStyleItem(XUI::CXUI_Wnd* pMainUI, E_STYLE_TYPE const Type, DWORD const dwItemNo, bool const bReset = false);
}

typedef std::map< SItemPos, DWORD >	CONT_STYLE_ITEM;

class PgStyleModifier
{
public:
	void Build();
	XUI::CXUI_Wnd* GetWnd();
	void GetStyleTypeList(E_STYLE_TYPE const Type, CONT_STYLE_ITEM& kList);
	void Clear();
	void SendPacket(PgInventory* pkInv);

	bool IsAlive() const { return (NULL != m_pMainUI); };

	PgStyleModifier(void);
	~PgStyleModifier(void);

protected:
	bool SendPacketIsAction(PgInventory* pkInv, DWORD& dwItemNo);

private:
	CONT_STYLE_ITEM	m_kContStyleItem[ESTYLE_END];
	XUI::CXUI_Wnd*	m_pMainUI;

	CLASS_DECLARATION_S(DWORD, ColorItem);
	CLASS_DECLARATION_S(DWORD, HairItem);
	CLASS_DECLARATION_S(DWORD, FaceItem);
	CLASS_DECLARATION_S_NO_SET(DWORD, BaseColorItem);
	CLASS_DECLARATION_S_NO_SET(DWORD, BaseHairItem);
	CLASS_DECLARATION_S_NO_SET(DWORD, BaseFaceItem);
	CLASS_DECLARATION_S_NO_SET(bool, SendPacket);
};

#define	g_kStyleModifier	SINGLETON_STATIC(PgStyleModifier)

//------------------------------------------------------------------------------------------------------------------
//	Uni Emoti UI
//------------------------------------------------------------------------------------------------------------------
namespace lwUniEmotiUI
{
	enum E_EMOTI_TAB_TYPE
	{
		EMOTITT_CHAT	= 0,
		EMOTITT_EMOTION	= 1,
		EMOTITT_BALLOON,
	};

	void RegisterWrapper(lua_State *pkState);

	void lwCallUniEmotiUI();
	void lwOnClickEmotiPage(lwUIWnd UISelf);
	void lwOnClickEmotiCategory(lwUIWnd UISelf);
	void lwOnSelectSubTypeItem(lwUIWnd UISelf);
	void lwOnSelectEmotiItem(lwUIWnd UISelf);
	void lwOnOverEmotiItem(lwUIWnd UISelf);

	void SetTabPage(E_EMOTI_TAB_TYPE const Type, XUI::CXUI_Wnd* pParent);
	void SetChatEmoticonPage(XUI::CXUI_Wnd* pPage, int const iGroup);
	void SetBalloonPage(XUI::CXUI_Wnd* pPage, int const iGroup);
	void SetEmotionPage(XUI::CXUI_Wnd* pPage, int const iGroup);
	bool GetEmotiCategoryList(E_EMOTI_TAB_TYPE const Type, std::vector< int >& kArray);

	void ReceivePacket_Command(WORD const wPacketType, BM::Stream& kPacket);

	bool lwIsMyPlayerLearnInteractiveEmotion();
};



class PgEmotiMgr
	: public PgIXmlObject
{
public:
	typedef std::list< std::wstring > CONT_KEYWORD;
	typedef std::vector< int >	CONT_IDARRAY;
	typedef std::map< int, CONT_IDARRAY >	CONT_BUILDDATA;
	
	class tagEmotiBase
	{
	public:
		virtual ~tagEmotiBase(){};

		virtual bool IsEmpty()const
		{
			return ( !ID() && !Group() );
		};

		tagEmotiBase() : m_kID(0), m_kGroup(0){};
		CLASS_DECLARATION_S(int, ID);
		CLASS_DECLARATION_S(int, Group);
	};

	class tagEmotiExtend : public tagEmotiBase
	{
	public:
		virtual ~tagEmotiExtend(){};

		virtual bool IsEmpty()const
		{
			return ( KeywordCont().empty() || IconPath().empty() || tagEmotiBase::IsEmpty() );
		};

		bool ParseKeyward(std::wstring const& kString)
		{
			BM::vstring::CutTextByKey<std::wstring>(kString, L",", m_kKeywordCont);
			return !m_kKeywordCont.empty();
		}

		virtual std::wstring const GetStringToolTip() const
		{
			BM::vstring vStr;
			CONT_KEYWORD::const_iterator c_iter = KeywordCont().begin();
			while( c_iter != KeywordCont().end() )
			{
				if( vStr.size() )
				{
					vStr += L", ";
				}
				vStr += L"@";
				vStr += (*c_iter);
				++c_iter;
			}

			return std::wstring(vStr);
		}

		CLASS_DECLARATION_S(CONT_KEYWORD, KeywordCont);
		CLASS_DECLARATION_S(std::wstring, IconPath);
	};

	class tagEmotion : public tagEmotiExtend
	{
	public:
		struct tagParam
		{
			tagParam(std::wstring const& _KEY, std::wstring const& _VAL)
				: KEY(_KEY), VAL(_VAL)
			{
			}
			std::wstring KEY;
			std::wstring VAL;
		};
		typedef std::map< std::wstring, tagParam > CONT_PARAM;

		virtual ~tagEmotion(){};
		
		bool AddParam(std::wstring const& KEY, std::wstring const& VAL)
		{
			std::pair<CONT_PARAM::iterator, bool> Result = m_kParamCont.insert(std::make_pair(KEY, tagParam(KEY, VAL)));
			return Result.second;
		}

		virtual std::wstring const GetStringToolTip() const
		{
			BM::vstring vStr;
			CONT_KEYWORD::const_iterator c_iter = KeywordCont().begin();
			while( c_iter != KeywordCont().end() )
			{
				if( vStr.size() )
				{
					vStr += L", ";
				}
				vStr += L"/";
				vStr += (*c_iter);
				++c_iter;
			}

			return std::wstring(vStr);
		}

		CLASS_DECLARATION_S(CONT_PARAM, ParamCont);
		CLASS_DECLARATION_S(std::wstring, ActionID);
	};

	typedef std::map< int, tagEmotion >		CONT_EMOTION;
	typedef std::map< int, tagEmotiBase >	CONT_CHAT_EMOTI;
	typedef std::map< int, tagEmotiExtend >	CONT_BALLOON_EMOTI;

	virtual bool ParseXml(TiXmlNode const* pkNode, void* pArg = 0, bool bUTF8 = false);
	bool OnBuild();

	bool FindEmotionByKeyward( std::wstring const &InWord, int& ID, std::wstring::size_type const pos = std::wstring::npos ) const;
	bool FindBalloonByKeyward( std::wstring const &InWord, int& ID, std::wstring::size_type const pos = std::wstring::npos ) const;
	tagEmotion const &GetEmotion(int const ID)const;
	std::wstring const& GetBalloonIcon(int const ID);
	std::wstring const GetChatString(EEMOTION_TYPE Type, int const ID);
	bool GetEmotiGroupArray(EEMOTION_TYPE Type, CONT_IDARRAY& kArray);
	bool GetEmotiGroupItem(EEMOTION_TYPE Type, int const GroupID, CONT_IDARRAY& kArray);

	PgEmotiMgr();
	~PgEmotiMgr();

protected:
	bool ParseEmotionXml(TiXmlElement const* pkElement);
	bool ParseBalloonKeywardXml(TiXmlElement const* pkElement);
	bool AddBuildData(CONT_BUILDDATA& kCont, int const GroupID, int const EmotiID);

private:
	CONT_EMOTION		m_kEmotionContainer;
	CONT_CHAT_EMOTI		m_kChatEmoticonContainer;
	CONT_BALLOON_EMOTI	m_kBalloonEmoticonContainer;
	CONT_BUILDDATA		m_kEmoticon;
	CONT_BUILDDATA		m_kEmotion;
	CONT_BUILDDATA		m_kBalloon;
};

typedef PgEmotiMgr::tagEmotion::tagParam EMOTI_PARAM;

#define	g_kPgEmotiMgr	SINGLETON_STATIC(PgEmotiMgr)

void SendEmotionPacket(PgActor* pkActor, BYTE btEmotionType, int iEmotionID);
bool CheckCanEmotion(int const iEmotionID);

//------------------------------------------------------------------------------------------------------------------
//	Item Option Change UI
//------------------------------------------------------------------------------------------------------------------
namespace lwLuckyChanger
{
	void RegisterWrapper(lua_State *pkState);

	void lwOnClickLuckyUISlot(lwUIWnd UISelf);
	void lwOnOKLuckyChange(lwUIWnd UISelf);

	void ReceivePacket_Command(WORD const wPacketType, BM::Stream& rkPacket);
	void CallLuckyChanger();
	void OnDragUpItem(XUI::CXUI_Wnd* pSelf);
	bool SetItem(SItemPos const & kItemPos);
};

//------------------------------------------------------------------------------------------------------------------
//	경험치 카드
//------------------------------------------------------------------------------------------------------------------
namespace lwAccumlationExpCard
{
	class PgUIData_ExpCard
		: public PgUIData_Base
	{
	public:
		PgUIData_ExpCard() : m_kActivateGuid(){};
		virtual ~PgUIData_ExpCard(){};

		virtual int GetType(void) const { return UIDATA_EXPCARD; };
	public:
		BM::GUID m_kActivateGuid;
	};

	void RegisterWrapper(lua_State *pkState);

	void lwCallFullAccumulateCardUse(lwGUID kGuid);
	void lwUdateAccumulateCardUsing(lwGUID kGuid);
	void lwOnClickFullAccumulateCardUseOK(lwUIWnd kSelf);
	void lwOnClickFullAccumulateCardUseRealOK(lwUIWnd kSelf);
	void lwCheckUsingExpCard();
	void lwCheckExpCardPercentText(lwUIWnd UISelf);
	void lwUpdateUsingExpCardInfo(lwUIWnd kSelf);

	void ReceivePacket_Command(WORD const wPacketType, BM::Stream& rkPacket);
	void SendActivateExpCard(SItemPos kPos);
	void SendDeactivateExpCard();
	void CheckUseExpCard(SItemPos const& kItemPos);
	void CheckUsingExpCardInfo(XUI::CXUI_Wnd* pIcon, PgBase_Item const& kItem, bool const bIsBuild = true, bool const bHaveToggle = false);
	bool GetExpToCardItem(BM::GUID const& kGuid, SItemPos& kItemPos, int& iLevel, __int64& iExpValue, int& iNameNo);
	bool GetExpToBookItem(int const iExpLv, SItemPos& kItemPos);
};

//------------------------------------------------------------------------------------------------------------------
//	크래프트 옵션 체인저
//------------------------------------------------------------------------------------------------------------------
namespace lwCraftOptChanger
{
	typedef enum eCraftOpt_Error
	{
		ECOE_OK				= 0,
		ECOE_WRONG_ITEM		= 1,
		ECOE_NO_CRAFT_ITEM,
		ECOE_NOT_EXIST_CRAFT_OPT,
		ECOE_CLIENT_ERROR,
		ECOE_NO_ITEM_REGIST,
		ECOE_NO_EXIST_OPT_CHANGE_ITEM,
		ECOE_NOT_EXIST_DB,
		ECOE_LACK_OPT_CHANGE_ITEM,
		ECOE_NO_DEFINED_ERROR,
	}E_CRAFT_OPT_ERROR;

	typedef enum eCraftOpt_State
	{
		ECOS_NOTHING				= 0,
		ECOS_PREV_EFFECT_STARTED	= 1,
		ECOS_NEXT_EFFECT_STARTED,
		
	}E_CRAFT_OPT_STATE;

	void RegisterWrapper(lua_State *pkState);
	void Recv_ANS_Packet(WORD const wPacketType, BM::Stream& kPacket);

	void lwOnClickCraftOptChangeAction(lwUIWnd kSelf);
	void lwOnEffectViewAndSending(lwUIWnd kSelf);
	void lwCloseCraftOptChanger(lwUIWnd kSelf);
	void lwUnRegCraftOptItem(lwUIWnd kSelf);

	void CallCraftOptChanger(SItemPos const kItemPos);
	void CallCraftOptChanger(SEnchantInfo const& kEnchantInfo);
	HRESULT SetCraftOptTargetItem(XUI::CXUI_Wnd* pkMain, SItemPos const& kItemPos, PgBase_Item const& kItem, bool bIsResult);
	void SetCraftOptSlot(XUI::CXUI_Wnd* pkMain, PgBase_Item const& kItem, CItemDef const* pkDef, CONT_ENCHANT_ABIL const& kAbilCont, bool bIsResult);
	void MoveCraftOptSlotValue(XUI::CXUI_Wnd* pkMain);

	HRESULT CheckSendOK(SItemPos const& kTargetItemPos, SItemPos const& kRediceItemPos);
};

namespace lwPetOptChanger
{
	void RegisterWrapper(lua_State *pkState);
	bool lwRediceOptionInit(lwUIWnd kWnd);
	void Recv_ANS_Packet(WORD const wPacketType, BM::Stream& rkPacket);
	bool IsRegItem(PgBase_Item const & kItem);
	bool SetSrcItem(const SItemPos &rkItemPos);
	void Call(SItemPos const & rkItemInvPos);
}

namespace lwPetUpgrade
{
	void RegisterWrapper(lua_State *pkState);
	bool lwPetUpgradeInit(lwUIWnd kWnd);
	void lwInitUpgradeEffect(void);
	void Call(void);
	bool SetSrcItem(const SItemPos &rkItemPos);
	bool RequestUpgrade(void);
}

//------------------------------------------------------------------------------------------------------------------
//	스킬 레벨 확장 카드
//------------------------------------------------------------------------------------------------------------------
namespace lwSkillExtend
{
	void RegisterWrapper(lua_State *pkState);
	void Recv_ANS_Packet(WORD const wPacketType, BM::Stream& rkPacket);
	void Call(SItemPos const & rkItemInvPos);
}

//------------------------------------------------------------------------------------------------------------------
//	귀속 해제 아이템
//------------------------------------------------------------------------------------------------------------------
namespace lwUseUnlockItem
{
	enum eUnlockingState
	{
		EUS_PREPARE	= 0,
		EUS_START	= 1,
		EUS_WAIT	= 2,
		EUS_RESULT	= 3,
	};

	class PgUIData_UseUnlockItem
		: public PgUIData_Base
	{
	public:
		enum ERESULT
		{
			E_OK			= 0,
			E_NO_DEFINE_ERR,
			E_NO_SPECIAL,
			E_NO_REG_ITEM,
			E_NO_LOCKED_ITEM,
			E_NO_EXIST_ITEM,
			E_NO_EXIST_UNLOCK_ITEM,
			E_NO_EQUIP_ITEM,
		};

		struct ITEMINFO
		{
			ITEMINFO()
				: dwItemNo(0), kItemPos()
			{}
			ITEMINFO( ITEMINFO const& rhs )
				: dwItemNo( rhs.dwItemNo ), kItemPos( rhs.kItemPos )
			{}
			ITEMINFO( DWORD const ItemNo, SItemPos const& kPos )
				: dwItemNo( ItemNo ), kItemPos( kPos )
			{}
			DWORD		dwItemNo;
			SItemPos	kItemPos;
		};

		ERESULT SetUnLockItem( SItemPos const& kItemPos );
		ERESULT SetTargetItem( SItemPos const& kItemPos );
		void SendPacket();
		void Clear();

		PgUIData_UseUnlockItem(){};
		virtual ~PgUIData_UseUnlockItem(){};

		virtual int GetType(void) const { return UIDATA_USE_UNLOCK_ITEM; };
	public:
		ITEMINFO m_kUnLockItem;
		ITEMINFO m_kTargetItem;
	};

	void RegisterWrapper(lua_State *pkState);

	void lwOnClickStartAction(lwUIWnd kSelf);
	void lwUpdate(lwUIWnd kSelf);
	void lwClose(lwUIWnd kSelf);

	void CreateCall(SItemPos const& rkItemInvPos);
	void Call(SItemPos const& rkItemInvPos, bool bResult);
	void Start(XUI::CXUI_Wnd* pMainUI);
	void Result(void);
	void SetTargetItem(SItemPos const& rkItemInvPos);
	void Recv_ANS_Packet(WORD const wPacketType, BM::Stream& rkPacket);

	bool CheckSendOK(PgUIData_UseUnlockItem* pkSystem);
};

//------------------------------------------------------------------------------------------------------------------
//	인첸트 전이
//------------------------------------------------------------------------------------------------------------------
namespace lwEnchantShift
{
	void lwCall();
	void lwClose();
	void lwUpdate();
	void lwBtnDownInsurance(lwUIWnd kSelf);
	void lwSetInsuranc_Ex();
	void lwStartMixing();
	void lwSetInsurance_Display(int const iType, int iItemNo);
};

class PgEnchantShift
{
public:
	struct S_ITEM_INFO
	{
		S_ITEM_INFO() : kItemNo(0), kItemPos() {};

		void Clear()
		{
			kItemNo = 0;
			kItemPos.Clear();
		}
		bool IsEmpty() const
		{
			return (kItemNo == 0);
		}
		DWORD		kItemNo;
		SItemPos	kItemPos;
	};

	typedef enum {
		ES_NONE			= 0,
		ES_OPEN			= 0,
		ES_STEP_1,
		ES_STEP_2,
		ES_STEP_3,
		ES_PROCESS_START,
		ES_PROCESSING,
		ES_RESET,
		ES_RESET_OK,
		ES_ERROR,
	}E_STATE;

	typedef enum {
		EPS_NONE		= 0,
		EPS_INIT,
		EPS_NORMAL,
		EPS_SENDING,
		EPS_RECV_WAIT,
		EPS_RECV,
		EPS_FINISH,
	}E_PROCESS_STATE;

	typedef enum {
		EINSUR_DESTORY	= 0,	//파괴
		EINSUR_MINUS_2,			//-2
		EINSUR_MINUS_1,			//-1
		EINSUR_ZERO,			//0
		EINSUR_PLUS_1,			//+1
		EINSUR_PLUS_2,			//+2
		EINSUR_MAX,
	}E_INSURANCETYPE;

	typedef enum {
		EAT_NONE	= 0,
		EAT_SOURCE,
		EAT_TARGET,
		EAT_INSURANCE,
	}E_ARROWTYPE;

	PgEnchantShift();
	~PgEnchantShift();

	friend void lwEnchantShift::lwCall();
	friend void lwEnchantShift::lwClose();
	friend void lwEnchantShift::lwUpdate();
	friend void lwEnchantShift::lwBtnDownInsurance(lwUIWnd kSelf);
	friend void lwEnchantShift::lwStartMixing();
	friend void lwEnchantShift::lwSetInsurance_Display(int const iType, int iItemNo);

	void Clear(bool const bClearState=false);
	static void RegisterWrapper(lua_State *pkState);
	
	void RecvPacket(BM::Stream& kPacket);
	
	bool SetSrcItem(SItemPos const & rkItem);
	bool SetTargetItem(SItemPos const & rkItem, bool const bIsMsg=true);
	bool SetInsurance(E_INSURANCETYPE const eType, int const iItemNo, bool const bReal);
	void GetInsuranceItem(E_INSURANCETYPE const eType, ContHaveItemNoCount & rkItemCont)const;

	E_STATE GetState()const { return m_eState; }
	E_PROCESS_STATE GetProcessState()const { return m_eProcessState; }
	eEnchantShiftRate GetSendResult()const { return m_eResult; }

	void Update();

	bool IsRunning()const;

	void Refresh(bool const bReset=true);
	void RefreshResultUI();

	void ClearSrcItem();
	void ClearTargetItem();
	bool IsUpgradeEnchantTargetItem()const;
	bool IsReset()const;
	void DoStartMixing();
	bool IsUseableItem(SItemPos const& rkItemPos, bool & rbGrayVisible)const;
	bool DestoryInuranceEmpty()const;

	int GetCurrentInsurance()const;
	int UsableInsuranceCount( int const iItemNo );
private:
	void CallWndInv();

	void UpdateCheckOption(XUI::CXUI_Wnd * const pkWnd);
	void UpdateNeedItems(XUI::CXUI_Wnd * const pkWnd);
	void UpdateTargetImg(XUI::CXUI_Wnd * const pkWnd);
	void UpdateInsurance(XUI::CXUI_Wnd * const pkWnd);
	void UpdateEnchantShiftRate(XUI::CXUI_Wnd * const pkWnd);
	void UpdateResultSlot(XUI::CXUI_Wnd * const pkWnd);
	void UpdateArrow(XUI::CXUI_Wnd * const pkWnd, E_ARROWTYPE const eType);

	void RefreshInsurance();

	void SetState(E_STATE const eState, bool const bLoop=false);
	void SetProcessState(E_PROCESS_STATE const eState);

	HRESULT CheckSrcItem(SItemPos const & rkItem)const;
	HRESULT CheckSrcItem(PgBase_Item const& rkItem)const;
	HRESULT CheckTargetItem(SItemPos const & rkItemPos)const;
	HRESULT CheckTargetItem(PgBase_Item const & rkItem, SItemPos const & rkItemPos)const;
	S_ITEM_INFO const & GetInsurance(E_INSURANCETYPE const eType)const;

	void ClearInsurance(E_INSURANCETYPE const eType);

	bool IsShiftSoul()const;
	bool IsShiftSpecial()const;
	bool IsShiftSocket()const;
	bool IsOpenResultSlot(E_INSURANCETYPE const eType, SItemPos const& rkTargetItemPos)const;
	bool IsBeforOpenInv()const { return m_bIsOpenInv; }
	E_ARROWTYPE GetNowArrowType()const;

	bool ErrorCheck(HRESULT const hResult, bool const bIsMsg=true)const;
	void Send()const;

private:
	E_STATE m_eState;
	E_PROCESS_STATE m_eProcessState;

	S_ITEM_INFO m_kSrcItem;
	S_ITEM_INFO m_kTargetItem;
	S_ITEM_INFO m_kInsurance[EINSUR_MAX];
	bool m_bIsOpenInv;
	eEnchantShiftRate m_eResult;
	std::wstring m_kResultText;
	ENoticeLevel m_eNoticeLevel;
	bool m_bHaveNeedItems;
	bool m_bHaveShiftCost;
	CLASS_DECLARATION_S(bool, CheckDestoryInurance);
	CLASS_DECLARATION_S(E_INSURANCETYPE, eReadyInsertInsurance);
};
#define g_kEnchantShift SINGLETON_STATIC(PgEnchantShift)

#endif // FREEDOM_DRAGONICA_CONTENTS_CASHITEM_LWITEM_H