#ifndef FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGITEMMIX_SCRIPT_H
#define FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGITEMMIX_SCRIPT_H

#include "PgEqItemMixCreator.h"
#include "PgMonsterCardMixer.h"
#include "PgUIDataMgr.h"

namespace lwItemMix
{
	struct S_UPDATE_TIME
	{
		float fTotalTime;
		float fStartTime;
	};

	namespace MixUIUtil
	{
		enum eInsuranceBtnState
		{
			EIBS_NO_USE	= 0,
			EIBS_USE_REG,
			EIBS_USE_DEREG,
		};

		void InitMixGearAni(XUI::CXUI_Wnd* pSelf, float const fStartTime);
		void UpdateMixGearAni(XUI::CXUI_Wnd* pSelf);
		void ClearMixGearAni(XUI::CXUI_Wnd* pSelf);

		void InitMixProgressing(XUI::CXUI_Wnd* pSelf, S_UPDATE_TIME const& kTimeInfo);
		bool UpdateMixProgressing(XUI::CXUI_Wnd* pSelf);
		void ClearMixProgressing(XUI::CXUI_Wnd* pSelf);

		void SetMixInsurance(XUI::CXUI_Wnd* pSelf, S_MIX_ITEM_INFO const& kItemInfo, int const DEFAULT_INS_ITEMNO); 
		void SetMixInsuranceSlotBtn(XUI::CXUI_Wnd* pSelf, eInsuranceBtnState const State); 
		void ClearMixInsuranceSlot(XUI::CXUI_Wnd* pSelf, int const DEFAULT_INS_ITEMNO);

		void SetMixMerterialSlot(XUI::CXUI_Wnd* pSelf, S_MIX_ITEM_INFO const& kItemInfo);
		void ClearMixMarterialSlot(XUI::CXUI_Wnd* pSelf);

		void SetMixResultSlot(XUI::CXUI_Wnd* pSelf, DWORD const kItemNo);
		void ClearMixResultSlot(XUI::CXUI_Wnd* pSelf);
	}

	void RegisterWrapper(lua_State *pkState);

	void StopEpItemMixing(lwUIWnd kParent);
	void StartEpItemMixing(lwUIWnd kParent);
	void UpdateEqItemMixCreatorUI(lwUIWnd kSelf);
	void UpdateEpItemMixInsuranc(lwUIWnd kSelf);
	void UpdateEpItemMixProbability(lwUIWnd kSelf);
	void CallEqItemMixToolTip(lwUIWnd kSelf);
	void CallEqItemMixCreatorUI();
	void UpdateEpItemMixProgressing(XUI::CXUI_Wnd* pSelf);
	void UpdateEpItemMixGearAni(XUI::CXUI_Wnd* pSelf);
	bool SetEpItemMixItemSlot(XUI::CXUI_Wnd* pSelf, int const RecipeCnt, S_MIX_ITEM_INFO const& kItemInfo);
	void ReSetEpItemMixProgressing(XUI::CXUI_Wnd* pSelf);
	void SetEpItemMixResultItem(int const iRecipeNo);
	void SetEpItemInsuranceEffect(XUI::CXUI_Wnd* pSelf, bool const bVisible);

	//이건 코스튬 조합
	enum ESLOT_TYPE : BYTE
	{
		EST_LEFT_ITEM = 0,
		EST_RIGHT_ITEM,
		EST_RATE_ITEM,
	};

	class PgUIData_CostumeMix : public PgUIData_Base
	{
	public:
		PgUIData_CostumeMix(void) : m_kAutoRegRateItem(false), m_kProgressing(false) {}
		virtual ~PgUIData_CostumeMix(void){}

		virtual int GetType(void) const { return UIDATA_COSTUME_MIX; }

		void WriteToPacket(BM::Stream& kPacket);
		void ItemClear();

	public:
		BM::GUID	m_kNpcGUID;
		SItemPos	m_kLeftItem;
		SItemPos	m_kRightItem;
		SItemPos	m_kRateItem;
		bool		m_kAutoRegRateItem;
		bool		m_kProgressing;
	};

	void CallCostumeMixNpcMenu(lwGUID kGuid);
	void CallCostumeMixUI(lwUIWnd kParent);
	void StartCostumeMixing(lwUIWnd kParent);
	void UpdateCostumeMixUI(lwUIWnd kParent);
	void OnDragDropCostumeMixItem(lwUIWnd kSelf, int const iSlot);
	void ClearCostumeMixUI(XUI::CXUI_Wnd* pMainUI);
	bool CheckComstumeMixCompareSlot(ESLOT_TYPE const kInItemSlot, PgBase_Item const& kItem, PgUIData_CostumeMix* pkCostumeMixData);
	void SetCostumeLogMessage(std::wstring const& kText);

	void RecvCostumeMix_Command(WORD wPacketType, BM::Stream& kPacket);

	//몬스터 카드 조합
	void CallMonsterCardMixerUI();
	void UpdateMonsterCardMixerUI(lwUIWnd kParent);
	void StartMonsterCardMixerUI(lwUIWnd kParent);

	class PgUIData_CommonMixer : public PgUIData_Base
	{
	public:
		explicit PgUIData_CommonMixer(E_UIDATA_TYPE const kType);
		virtual ~PgUIData_CommonMixer(void);
		
		virtual int GetType(void) const { return m_kType; }
		PgCommonTypeMixer* GetMixer() { return m_pkMixer; }
	protected:
		PgCommonTypeMixer* m_pkMixer;
		E_UIDATA_TYPE	m_kType;
	};

	void AddNewCommonMixerInfo(SItemPos const& kItemPos);
	void DeleteCommonMixerInfo();

	void CallCommonTypeMixer();
	void UpdateCommonTypeMixer(lwUIWnd kParent);
	void StartCommonTypeMixer(lwUIWnd kParent);
	bool SetCommonTypeMixerSlot(XUI::CXUI_Wnd* pSlot, int const RecipeCnt, S_MIX_ITEM_INFO const& kItemInfo);
	void ClearCommonTypeMixer(XUI::CXUI_Wnd* pMainUI);
	void InitCommonTypeMixer(XUI::CXUI_Wnd* pMainUI, PgCommonTypeMixer::SMIXER_INIT_INFO const& kInfo);

	//몬스터 카드 합성
	void lwOnCallMonsterCardMixerType2();
	void lwOnUpdateMonsterCardMixerType2(lwUIWnd kSelf);
	void lwOnCloseMonsterCardMixerType2();
	void lwSetDefaultInsItemNo(DWORD const InsItemNo);
	void lwOnClickMonsterCardMixerType2Start();
	void lwSetMonsterCardMixerType2Ins();

	void ClearMonsterCardMixerType2(XUI::CXUI_Wnd* pMainUI, int const DEFAULT_INS_ITEMNO);
	void SetMonsterCardMixerType2MixItem();
	void SetMonsterCardMixerType2MixItem(SItemPos const& kItemPos, bool const bIsNoMSG = false);
	bool SetMonsterCardMixerType2InsItem(PgMonsterCardMixer* pkMixer, bool const bIsNoMSG = false);
	bool SetMonsterCardMixerType2InsItemToPos(PgMonsterCardMixer* pkMixer);
	void OnCallMonsterCardMixerType2Result(CItemDef const* pDef, CONT_DEFRES const* pResDef, PgBase_Item const& kItem);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGITEMMIX_SCRIPT_H