#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUICHARINFO_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUICHARINFO_H

int const ABIL_INFO_MAX_SLOT = 5;

class lwCharInfo
{
public:
	static float const GUILD_ICON_SCALE;

	static bool RegisterWrapper(lua_State *pkState);

	static void lwSetCharInfoToUI(lwUIWnd UIWnd, lwGUID Guid);
	static void ChangePetActor(BM::GUID const kGuid);
	static void lwSetPetInfoToUI(lwUIWnd UIWnd, lwGUID kGuid);
	static void lwSetCharAbilInfo(lwUIWnd UIWnd, int iType, char const* szAddon, bool const bIsChanged);
	static void lwSetCharInfo(lwUIWnd UIWnd, int const iType);
	static lwWString lwViewPlusAbilValue(lwUIWnd UISelf);
	static void lwCallInfoDrop(lwUIWnd UIWnd, char const* szAddon, char const* szUIName);
	static bool lwViewEquipIconInitialize(lwUIWnd UIWnd, int const iInvType = KUIG_NONE );
	static bool lwViewEquipIconInitialize2( XUI::CXUI_Wnd* pkUI, PgInventory *pkInv, int const iInvType );
	static void lwChangeInfoTab(lwUIWnd UISelf, int const iTab);
	static void lwChangeOtherInfoTab(lwUIWnd UISelf, int const iTab);
	static bool lwIsRidingPet(void); //현재 데리고 있는 펫이 라이딩 펫인가?

	static void SetCharInfoToUI(XUI::CXUI_Wnd* pWnd, BM::GUID const& Guid, bool const bIsChanged);
	static void SetPetInfoToUI(XUI::CXUI_Wnd* pWnd, BM::GUID const& rkGuid, bool const bIsChanged);
	static bool SetDiePetInfoToUI(XUI::CXUI_Wnd* pWnd, BM::GUID const& rkGuid);
	static void SetAbilValue(XUI::CXUI_Wnd* pWnd, BM::GUID const& Guid, EAbilType Type, wchar_t const* pText = NULL);
	static void SetTotalAbilBonus(XUI::CXUI_Wnd* pWnd, BM::GUID const& Guid, wchar_t const* pText = NULL);
	static int CalcPlusAbilValue(CUnit* pkUnit, EAbilType Type);
	static void UpdateMyActor();
	static int GetBonusStatus(CUnit* pkUnit, EAbilType Type);

private:
	lwCharInfo(){}
	~lwCharInfo(){}
	
	static void ViewPlusAbilDefault(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr);

	static void ViewPlusAbil_HitSuccessValue(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr);
	static void ViewPlusAbil_DodgeSuccessValue(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr);
	static void ViewPlusAbil_BlockSuccessValue(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr);
	static void ViewPlusAbil_CriticalSuccessValue(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr);
	static void ViewPlusAbil_BlockDamage(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr);
	static void ViewPlusAbil_CriticalPower(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr);

	static void ViewPlusAbil_SubFunc(int iType, CUnit* pkUnit, BM::vstring& rvStr
									,int iAbilTypeTT
									,bool bKeepDisplay_Stat
									,bool bKeepDisplay_Inv
									,bool bKeepDisplay_Effect
									,bool bIsPercent);

	static void ViewPlusAbil_MaxLimit(int iType, CUnit* pkUnit, BM::vstring& rvStr, bool bKeepDisplay, bool bIsPercent);
	
	static void MakeValueStr(BM::vstring& rvStr, int const iTT, std::wstring const kMark, float fValue, bool const bIsPercent);
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUICHARINFO_H