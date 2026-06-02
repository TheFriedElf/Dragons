#pragma once

#include "PgScripting.h"
#include "lwGUID.h"
#include "lwWString.h"
#include "lwPacket.h"
#include "lwPoint2.h"
#include "lwPoint3.h"
#include "lwActor.h"
#include "XUI/XUI_Manager.h"
#include "lwUI.h"

typedef std::map< WORD, int >	CONT_ABILS;
typedef TBL_TRIPLE_KEY_INT		SBasicOptionAmpKey;		//EquipPos, LevelLimit, BasicAmpLv
//extern bool MakeItemName(PgBase_Item const * pItem, std::wstring &wstrName);
extern void GetAbilObject(int const iType, int const iEquipPos, int const iTypeLevel, int const iGroup, int iIndex, CONT_ENCHANT_ABIL& kAbilOjbect, SBasicOptionAmpKey const& kAmpKey=SBasicOptionAmpKey(0,0,0));
extern std::wstring const MakeSlotAbilToolTipText(PgBase_Item const& kItem, CItemDef const* pDef, CONT_ENCHANT_ABIL const& kAbilCont, bool bRoundBracket = false, bool bIncludeName = true, bool bShowBuilUpInfo = false);
extern std::wstring const MakeSlotAbilToolTipText(PgBase_Item const& kItem, CItemDef const* pDef, CONT_ABILS const& kContAbils, bool bRoundBracket = false, bool bIncludeName = true, bool bAddEnter = true);
extern void MakeAbilStringColor(WORD const wType, std::wstring &wstrText);
extern void MakeAbilString(PgBase_Item const &kItem, CItemDef const* pkItemDef, WORD const wType, int iValue, std::wstring &wstrText, TBL_DEF_ITEM_OPTION_ABIL* pkOptionAbil=NULL, PgBase_Item const * const pkEquipItem = NULL, bool bRoundBracket = false, bool bIncludeName = true, int const iAmpLevel = 0, bool bAddEnter = true, bool const bColor=true, bool bSetColor = true);
extern bool GetAbilFromSetItemOption(TBL_DEF_ITEM_OPTION_ABIL* pkOptionAbil, int const iAbilType, int& iResultValue);
extern void MakeEquipAddOption(PgBase_Item const& kItem, CItemDef const* pDef, bool const bOrtherActor, std::wstring& wstrText, bool const bColor=true);
extern float GetRarityAmplifyRate(SItemExtOptionKey const &kKey, int const iAmpLv);

typedef enum eUpgradeFlag
{
	E_UF_BASE = 0x0000,
	E_UF_PLUS = 0x0001,
	E_UF_RAIRITY = 0x0002,
	E_UF_SOCKET = 0x0004,
	E_UF_SET = 0x0008,
	E_UF_OPT = 0x0010,
	E_UF_EFFECT = 0x0020,
	E_UF_CASH = 0x0400,
	E_UF_MEDAL = 0x0800,
	E_UF_ALL = 0xFFFF,
};

typedef WORD E_UPGRADE_FLAG;

typedef std::vector<int> CONT_INT;

class PgItemUpgradeInfo
{
public:
	PgItemUpgradeInfo();
	PgItemUpgradeInfo(eUpgradeFlag const flag);
	virtual ~PgItemUpgradeInfo(){ m_kEtcAbils.Clear(); }

	void operator -= (PgItemUpgradeInfo const &rhs);
	void operator += (PgItemUpgradeInfo const &rhs);
	PgItemUpgradeInfo operator - (PgItemUpgradeInfo const &rhs) const;
	PgItemUpgradeInfo operator + (PgItemUpgradeInfo const &rhs) const;

	bool IsEmpty();
	void Clear();

	bool MakeToolTipText(std::wstring &rkText, CItemDef const* pkItemDef, bool bIsPhyInfo, bool bShowType = true);
	void MakeToolTipText_Total(std::wstring &rkText, CItemDef const* pkItemDef, bool bIsPhyInfo);
	void MakeToolTipText_Diff(std::wstring &rkText, CItemDef const* pkItemDef, bool bIsPhyInfo, PgItemUpgradeInfo const &rkItemInfo);
	bool MakeToolTipText_Status(std::wstring &rkText, WORD const wAbilType);
	int MakeToolTipText_All(CUnit const* const pkUnit, std::wstring &rkText, PgBase_Item const& rkItem, CItemDef const* pkItemDef, bool bIsPhyInfo);
	
	void SetBaseInfo(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef);
	void SetPlusUpgrade(PgBase_Item const& rkItem, CItemDef const* pkItemDef, SEnchantInfo const& rkEnchantInfo, int const iEnchantLv);
	void SetRarityUpgrade(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, SEnchantInfo const& rkEnchantInfo);
	void SetRareOption(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, SEnchantInfo const& rkEnchantInfo);
	void SetPackageOption(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, CONT_INT* pkContSetNo = NULL);
	void SetSocketOption(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, SEnchantInfo const& rkEnchantInfo);

	int GetAbil(WORD const wAbilType);

	void MakeCantAbil(std::wstring &rkText, CItemDef const* pkItemDef);

	std::wstring GetTypeName();
	E_UPGRADE_FLAG GetType() { return eFlag; }
	void SetType(E_UPGRADE_FLAG Flag) { eFlag = Flag; }

	void SetUnit(CUnit* pUnit)
	{
		m_pUnit = pUnit;
	}

	static void MakeValueString(BM::vstring &rText, WORD const wAbilType, __int64 value, 
		bool const usedecimal = true, int const iUnitLevel = 1);
	static float CalculateAbil(CUnit const* pUnit, WORD const abil, 
		EGetAbilType::Enum const getabiltype);

private:
	bool SetAbil(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, CAbilObject const &rkPlusAbil);
	bool SetAbil(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, CONT_ENCHANT_ABIL const &rkEnchantAbil);
	bool SetAbil(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, WORD const wAbilType, int iAbilValue);
	bool SetAbil(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, WORD const wBaseAbilType, WORD const wAbilType, int iAbilValue);

	void SetAbil(WORD const wAbilType, int iAbilValue);
	int CalcAbil(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, WORD const wBaseAbilType, WORD const wAbilType, int iAbilValue);

	bool FindSetNo(CONT_INT const * const pkContSetNo, int const iSetNo)const;
private:
	CLASS_DECLARATION_S(int, MinATK);
	CLASS_DECLARATION_S(int, MaxATK);
	CLASS_DECLARATION_S(int, MinMATK);
	CLASS_DECLARATION_S(int, MaxMATK);
	CLASS_DECLARATION_S(int, DEF);
	CLASS_DECLARATION_S(int, MDEF);
	CLASS_DECLARATION_S(int, HP);
	CLASS_DECLARATION_S(int, MP);
	CLASS_DECLARATION_S(int, STR);
	CLASS_DECLARATION_S(int, CON);
	CLASS_DECLARATION_S(int, INT);
	CLASS_DECLARATION_S(int, DEX);
	CLASS_DECLARATION_S(int, HIT_RATE);
	CLASS_DECLARATION_S(int, DODGE_RATE);
	CLASS_DECLARATION_S(int, HP_RECOVERY);
	CLASS_DECLARATION_S(int, MP_RECOVERY);

	E_UPGRADE_FLAG eFlag;

	CAbilObject	m_kEtcAbils;

	CUnit*	m_pUnit;
};