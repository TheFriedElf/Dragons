#include "stdafx.h"
#include "lwUI.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "variant/Global.h"
#include "PgGuild.h"
#include "lwUICharInfo.h"
#include "PgMyActorViewMgr.h"
#include "lwUIPet.h"
#include "ServerLib.h"
#include "lwUIToolTip.h"
#include "PgActor.h"

#include "Variant/Item.h"

float const _10000_TO_1PER = 100.0f;

extern PgInventory g_kOtherViewInv;
extern bool GetDefString(int const iTextNo, wchar_t const *&pString);
extern void WstringFormat( std::wstring& rkOutMsg, size_t const iSize, const wchar_t* szMsg, ... );

namespace PgPetUIUtil
{
	void lwUI_RefreshPetSkillUI(lwUIWnd kWnd);
	void lwDrawTextToPetState(lwUIWnd kWnd = lwUIWnd(NULL));
	XUI::CXUI_Wnd* SetPetUIDefaultState(bool bEnableCover);
	XUI::CXUI_Wnd* SetPetUIDefaultState(PgBase_Item const& rkItem);
	void lwSetPetAbilInfo(BM::GUID const& rkGuid, XUI::CXUI_Wnd* pInfoMain, int iType, char const* szAddon);
	void lwSetPetUseRateInfo(BM::GUID const& rkGuid, XUI::CXUI_Wnd* pInfoMain, int iType, char const* szAddon);
	void InitPetInfoUI(XUI::CXUI_Wnd* pWnd);
	int GetPetDyeingRandColorMax(void);
};

float const lwCharInfo::GUILD_ICON_SCALE = 0.8125f;

bool lwCharInfo::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	//-----> 케릭터 정보창 관련
	def(pkState, "SetCharInfoToUI", &lwCharInfo::lwSetCharInfoToUI);
	def(pkState, "SetCharInfo", &lwCharInfo::lwSetCharInfo);
	def(pkState, "SetCharAbilInfo", &lwCharInfo::lwSetCharAbilInfo);
	def(pkState, "ViewPlusAbilValue", &lwCharInfo::lwViewPlusAbilValue);
	def(pkState, "CallInfoDrop", &lwCharInfo::lwCallInfoDrop);
	def(pkState, "ViewEquipIconInitialize", &lwCharInfo::lwViewEquipIconInitialize);
	def(pkState, "ChangeInfoTab", &lwCharInfo::lwChangeInfoTab);
	def(pkState, "ChangeOtherInfoTab", &lwCharInfo::lwChangeOtherInfoTab);
	def(pkState, "SetPetInfoToUI", &lwCharInfo::lwSetPetInfoToUI);
	def(pkState, "IsRidingPet", &lwCharInfo::lwIsRidingPet);

	//<----- 여기까지
	return true;
}

void lwCharInfo::lwSetCharInfoToUI(lwUIWnd UIWnd, lwGUID Guid)
{
	SetCharInfoToUI(UIWnd.GetSelf(), Guid(), true);
	bool bIsMyActor = g_kPilotMan.IsMyPlayer(Guid());
	lwViewEquipIconInitialize(UIWnd, (bIsMyActor)?(KUIG_FIT):(KUIG_VIEW_OTHER_EQUIP));
}

void lwCharInfo::SetCharInfoToUI(XUI::CXUI_Wnd* pWnd, BM::GUID const& Guid, bool const bIsChanged)
{
	if( !pWnd )
	{
		return;
	}
	pWnd->OwnerGuid(Guid);
	pWnd->SetCustomData(&Guid, sizeof(Guid));

	PgPilot* pPilot = g_kPilotMan.FindPilot(Guid);
	if( !pPilot )
	{
		return;
	}

	CUnit* pUnit = pPilot->GetUnit();
	if( !pUnit )
	{
		return;
	}

	// 직업
	XUI::CXUI_Wnd* pCharIcon = pWnd->GetControl(_T("FRM_CHAR_ICON"));
	if( pCharIcon )
	{
		SUVInfo	Info = pCharIcon->UVInfo();
		Info.Index = pUnit->GetAbil(AT_CLASS);
		if(50 < Info.Index) 
		{
			Info.Index -= 22;
		}
		pCharIcon->UVInfo(Info);
		pCharIcon->Visible(true);
	}

	// 이름
	XUI::CXUI_Wnd* pCharText = pWnd->GetControl(_T("FRM_CHAR_TEXT"));
	if( pCharText )
	{
		BM::vstring	vStrName;
		vStrName = _T("[");
		vStrName += TTW(224);
		vStrName += _T(".");
		vStrName += pUnit->GetAbil(AT_LEVEL);
		vStrName += _T("]");
		vStrName += _T("\n");
		vStrName += pUnit->Name();

		pCharText->Text(vStrName);
		pCharText->Visible(true);
	}

	// 길드
	XUI::CXUI_Wnd* pGuildIcon = pWnd->GetControl(_T("FRM_GUILD_ICON"));
	XUI::CXUI_Wnd* pGuildText = pWnd->GetControl(_T("FRM_GUILD_TEXT"));
	if( pGuildIcon && pGuildText )
	{
		SGuildOtherInfo	kGuildInfo;
		if( g_kGuildMgr.GetGuildInfo(pUnit->GetGuildGuid(), pUnit->GetID(), kGuildInfo) )
		{
			pGuildIcon->Scale(GUILD_ICON_SCALE);
			SUVInfo	Info = pGuildIcon->UVInfo();
			Info.Index = kGuildInfo.cEmblem + 1;//UVIndex는 1부터 시작
			pGuildIcon->UVInfo(Info);

			pGuildText->Text(kGuildInfo.kName);
		}
		else
		{
			pGuildText->Text(_T(""));
		}

		pGuildIcon->Visible(false);
		pGuildText->Visible(false);
	}

	XUI::CXUI_CheckButton* pMyInfoBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pWnd->GetControl(_T("CBTN_MY_INFO")));
	XUI::CXUI_CheckButton* pGuildInfoBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pWnd->GetControl(_T("CBTN_GUILD_INFO")));
	if( !pMyInfoBtn || !pGuildInfoBtn )
	{
		return;
	}
	pMyInfoBtn->Check(true);
	pMyInfoBtn->ClickLock(true);
	pGuildInfoBtn->Check(false);
	pGuildInfoBtn->ClickLock(false);

	int iSet = lwConfig_GetValue("CHARINFO", "CHARINFO_LEFT");
	if (0 == iSet)
	{
		iSet = lwConfig_GetDefaultValue("CHARINFO", "CHARINFO_LEFT");
	}
	lwSetCharAbilInfo(lwUIWnd(pWnd), iSet-1, "L", bIsChanged);
	iSet = lwConfig_GetValue("CHARINFO", "CHARINFO_RIGHT");
	if (0 == iSet)
	{
		iSet = lwConfig_GetDefaultValue("CHARINFO", "CHARINFO_RIGHT");
	}
	lwSetCharAbilInfo(lwUIWnd(pWnd), iSet-1, "R", bIsChanged);
}

void lwCharInfo::lwSetCharAbilInfo(lwUIWnd UIWnd, int iType, char const* szAddon, bool const bIsChanged)
{
	XUI::CXUI_Wnd* pInfoMain = UIWnd.GetSelf();
	if( !pInfoMain )
	{
		return;
	}

	bool bIsPet = false;
	if( pInfoMain->ID().compare(L"FRM_CHAR_INFO") != 0 )
	{
		if(0==pInfoMain->ID().compare(L"FRM_INFO"))	//펫용
		{
			bIsPet = true;
		}
		else
		{
			pInfoMain = pInfoMain->GetControl(L"FRM_CHAR_INFO");
			if( !pInfoMain )
			{
				return;
			}
		}
	}

	BM::GUID kGuid = pInfoMain->OwnerGuid();
	if(bIsPet)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		kGuid = pkPlayer->SelectedPetID();
	}

	BM::vstring vTabTitle(_T("FRM_INFO_TYPE_"));
	vTabTitle += szAddon;

	bool bSaveConfig = false;
	XUI::CXUI_Wnd*	pTitle = pInfoMain->GetControl(vTabTitle);
	if( pTitle )
	{
		if( bIsChanged )
		{
			std::wstring	kStr(TTW(5995).c_str());
			kStr += _T("(");

			int iTTW = 790680 + iType-3;// 텍스트 번호가 다닥 다닥 붙어있어 이런 코드로 분리함 

			switch(iType)
			{
			case 0:
			case 1:
			case 2:
				{//기본,공격,방어
					iTTW = 5992 + iType;
					bSaveConfig = true;
				}break;
			case 3:
				{//보너스
					bSaveConfig = true;
				}break;
			case 4:
				{//저항력
					iTTW = 792040;
					bSaveConfig = true;
				}break;
			case 5:
				{//속성
					iTTW = 207;
					bSaveConfig = true;
				}break;
			case 6:
				{//특성
					iTTW = 791550;
					bSaveConfig = true;
				}break;
		//캐릭터 정보창에서, 펫부분
			case 100:
				{
					iTTW = 3010;
				}break;
			case 101:
				{
					iTTW = 1804;
				}break;
			}

			pTitle->SetCustomData(&iType, sizeof(iType));
			
			kStr += TTW(iTTW);
			kStr += _T(")");
			pTitle->Text(kStr);
		}
		else
		{
			pTitle->GetCustomData(&iType, sizeof(iType));
		}
	}

	if(bIsPet)
	{
		PgPetUIUtil::lwSetPetAbilInfo(kGuid, pInfoMain, iType, szAddon);
		PgPetUIUtil::lwSetPetUseRateInfo(kGuid, pInfoMain, iType, szAddon);
		
		lwConfig_SetValue("CHARINFO", "PETINFO_LEFT", iType);
		return;//더 이상 돌 필요 없음
	}

	for(int i = 0; i < ABIL_INFO_MAX_SLOT; ++i)
	{
		BM::vstring vStr(_T("FRM_INFO_"));
		vStr += szAddon;
		vStr += i;

		XUI::CXUI_Wnd* pAbilSlot = pInfoMain->GetControl(vStr);
		if( !pAbilSlot )
		{
			continue;
		}

		int const iAbil = (iType * ABIL_INFO_MAX_SLOT) + i;
		switch( iAbil )
		{
		// 능력치 정보(기본)
		case  0:{ SetAbilValue(pAbilSlot, kGuid, AT_C_STR); }break;
		case  1:{ SetAbilValue(pAbilSlot, kGuid, AT_C_INT); }break;
		case  2:{ SetAbilValue(pAbilSlot, kGuid, AT_C_CON); }break;
		case  3:{ SetAbilValue(pAbilSlot, kGuid, AT_C_DEX); }break;
		case  4:{ SetAbilValue(pAbilSlot, kGuid, AT_C_MOVESPEED); }break;
		// 능력치 정보(공격)
		case  5:{ SetAbilValue(pAbilSlot, kGuid, AT_C_PHY_ATTACK_MAX, TTW(400142).c_str());	}break;
		case  6:{ SetAbilValue(pAbilSlot, kGuid, AT_C_MAGIC_ATTACK_MAX, TTW(400143).c_str());	}break;
		case  7:{ SetAbilValue(pAbilSlot, kGuid, AT_C_HIT_SUCCESS_VALUE);		}break;
		case  8:{ SetAbilValue(pAbilSlot, kGuid, AT_C_CRITICAL_POWER);	}break;
		case  9:{ SetAbilValue(pAbilSlot, kGuid, AT_C_ATTACK_SPEED);	}break;
		// 능력치 정보(방어)
		case 10:{ SetAbilValue(pAbilSlot, kGuid, AT_C_PHY_DEFENCE, TTW(400144).c_str());		}break;
		case 11:{ SetAbilValue(pAbilSlot, kGuid, AT_C_MAGIC_DEFENCE, TTW(400145).c_str());	}break;
		case 12:{ SetAbilValue(pAbilSlot, kGuid, AT_C_HP_RECOVERY);	}break;
		case 13:{ SetAbilValue(pAbilSlot, kGuid, AT_C_MP_RECOVERY);	}break;
		case 14:{ SetAbilValue(pAbilSlot, kGuid, AT_C_DODGE_SUCCESS_VALUE);	}break;	//회피율
		// 능력치 정보(보너스)
		case 15:{ SetAbilValue(pAbilSlot, kGuid, AT_STR_ADD);	}break;
		case 16:{ SetAbilValue(pAbilSlot, kGuid, AT_INT_ADD);	}break;
		case 17:{ SetAbilValue(pAbilSlot, kGuid, AT_CON_ADD);	}break;
		case 18:{ SetAbilValue(pAbilSlot, kGuid, AT_DEX_ADD);	}break;
		case 19:{ SetTotalAbilBonus(pAbilSlot, kGuid); }break;//보너스 총합
		// 능력치 정보(저항력)
		case 20:{ SetAbilValue(pAbilSlot, kGuid, AT_RESIST_ADD_CURSE);	}break;
		case 21:{ SetAbilValue(pAbilSlot, kGuid, AT_RESIST_ADD_FIRE);	}break;
		case 22:{ SetAbilValue(pAbilSlot, kGuid, AT_RESIST_ADD_ICE);	}break;
		case 23:{ SetAbilValue(pAbilSlot, kGuid, AT_RESIST_ADD_NATURE);	}break;
		case 24:{ SetAbilValue(pAbilSlot, kGuid, AT_RESIST_ADD_DESTROY);	}break;
		// 능력치 정보(속성)
		case 25:{ SetAbilValue(pAbilSlot, kGuid, AT_ATTACK_ADD_CURSE);	}break;
		case 26:{ SetAbilValue(pAbilSlot, kGuid, AT_ATTACK_ADD_FIRE);	}break;
		case 27:{ SetAbilValue(pAbilSlot, kGuid, AT_ATTACK_ADD_ICE);	}break;
		case 28:{ SetAbilValue(pAbilSlot, kGuid, AT_ATTACK_ADD_NATURE);	}break;
		case 29:{ SetAbilValue(pAbilSlot, kGuid, AT_ATTACK_ADD_DESTROY);	}break;
		//능력치 정보(특수)
		case 30:{ SetAbilValue(pAbilSlot, kGuid, AT_C_BLOCK_SUCCESS_VALUE);	}break;
		case 31:{ SetAbilValue(pAbilSlot, kGuid, AT_C_BLOCK_DAMAGE_RATE);		}break;
		case 32:{ SetAbilValue(pAbilSlot, kGuid, AT_C_CRITICAL_SUCCESS_VALUE);}break;
		case 33:{ SetAbilValue(pAbilSlot, kGuid, AT_C_CRITICAL_POWER);		}break;
		case 34:{ SetAbilValue(pAbilSlot, kGuid, AT_NONE);	}break;
		default:
			{
				//error
			}break;
		}
	}

	if(bSaveConfig)
	{
		if( 0 == strcmp("R", szAddon) )
		{
			lwConfig_SetValue("CHARINFO", "CHARINFO_RIGHT", iType+1);
		}
		else if( 0 == strcmp("L", szAddon) )
		{
			lwConfig_SetValue("CHARINFO", "CHARINFO_LEFT", iType+1);
		}
		lwConfig_ApplyConfig();
		lwConfig_Save(true);
	}
}

void lwCharInfo::lwSetCharInfo(lwUIWnd UIWnd, int const iType)
{
	XUI::CXUI_Wnd* pInfoMain = UIWnd.GetSelf();
	if( !pInfoMain )
	{
		return;
	}
	BM::GUID	Guid = UIWnd.GetOwnerGuid().GetGUID();

	XUI::CXUI_Wnd* pCharIcon = pInfoMain->GetControl(_T("FRM_CHAR_ICON"));
	XUI::CXUI_Wnd* pCharText = pInfoMain->GetControl(_T("FRM_CHAR_TEXT"));
	XUI::CXUI_Wnd* pGuildIcon = pInfoMain->GetControl(_T("FRM_GUILD_ICON"));
	XUI::CXUI_Wnd* pGuildText = pInfoMain->GetControl(_T("FRM_GUILD_TEXT"));
	XUI::CXUI_CheckButton* pCharBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pInfoMain->GetControl(_T("CBTN_MY_INFO")));
	XUI::CXUI_CheckButton* pGuildBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pInfoMain->GetControl(_T("CBTN_GUILD_INFO")));
	if( !pCharIcon || !pCharText || !pGuildIcon || !pGuildText || !pCharBtn || !pGuildBtn )
	{
		return;
	}

	pCharIcon->Visible(false);
	pCharText->Visible(false);
	pGuildIcon->Visible(false);
	pGuildText->Visible(false);
	pCharBtn->ClickLock(false);
	pCharBtn->Check(false);
	pGuildBtn->ClickLock(false);
	pGuildBtn->Check(false);
	switch(iType)
	{
	case 0:
		{
			pCharIcon->Visible(true);
			pCharText->Visible(true);
			pCharBtn->Check(true);	
			pCharBtn->ClickLock(true);
		}break;
	case 1:
		{
			PgPilot* pPilot = g_kPilotMan.FindPilot(Guid);
			if( !pPilot )
			{
				return;
			}

			CUnit* pUnit = pPilot->GetUnit();
			if( !pUnit )
			{
				return;
			}

			SGuildOtherInfo	kGuildInfo;
			if( g_kGuildMgr.GetGuildInfo(pUnit->GetGuildGuid(), pUnit->GetID(), kGuildInfo) )
			{
				pGuildIcon->Visible(true);
				pGuildText->Visible(true);
			}
			pGuildBtn->Check(true);
			pGuildBtn->ClickLock(true);
		}break;
	}
}

void lwCharInfo::SetAbilValue(XUI::CXUI_Wnd* pWnd, BM::GUID const& Guid, EAbilType Type, wchar_t const* pText)
{
	PgPilot* pPilot = g_kPilotMan.FindPilot(Guid);
	if( !pPilot )
	{
		return;
	}

	CUnit* pUnit = pPilot->GetUnit();
	if( !pUnit )
	{
		return;
	}

	int const iBaseAbil = GetBasicAbil(Type);

	if( !pText )
	{
		std::wstring AbilName;
		switch(Type)
		{
		case AT_STR_ADD:				{AbilName = TTW(790658);}break;
		case AT_INT_ADD:				{AbilName = TTW(790659);}break;
		case AT_CON_ADD:				{AbilName = TTW(790660);}break;
		case AT_DEX_ADD:				{AbilName = TTW(790661);}break;
		//case AT_C_HIT_SUCCESS_VALUE:
		//case AT_HIT_SUCCESS_VALUE:		{AbilName = TTW(791555);}break;
		//case AT_C_DODGE_SUCCESS_VALUE:
		//case AT_DODGE_SUCCESS_VALUE:	{AbilName = TTW(791556);}break;
		//case AT_C_BLOCK_SUCCESS_VALUE:
		//case AT_BLOCK_SUCCESS_VALUE:	{AbilName = TTW(791551);}break;
		//case AT_C_BLOCK_DAMAGE_RATE:
		//case AT_BLOCK_DAMAGE_RATE:		{AbilName = TTW(791552);}break;
		//case AT_C_CRITICAL_SUCCESS_VALUE:
		//case AT_CRITICAL_SUCCESS_VALUE:	{AbilName = TTW(791553);}break;
		//case AT_C_CRITICAL_POWER:
		//case AT_CRITICAL_POWER:			{AbilName = TTW(791554);}break;
		default:
			{
				MakeAbilNameString2(Type, AbilName);
			}break;
		}
		pWnd->Text(AbilName);
	}
	else
	{
		pWnd->Text(pText);
	}

	XUI::CXUI_Wnd* pAbliValue = pWnd->GetControl(_T("FRM_VALUE"));
	if( !pAbliValue )
	{
		return;
	}

	BM::vstring	vStr;

	switch(Type)
	{
	case AT_STR_ADD:
	case AT_INT_ADD:
	case AT_CON_ADD:
	case AT_DEX_ADD:
		{// 보너스 스테이터스(Active Status)
			vStr = lua_tinker::call<char*, int>("GetStrColorInCharInfoUI", 0);
		}break;
	default:
		{
			int const CalcPlusAbil = CalcPlusAbilValue(pUnit, Type);
			if( CalcPlusAbil != 0 )
			{
				if( CalcPlusAbil > 0 )
				{
					vStr = _T("{C=0xFF008E21/}");
				}
				else
				{
					vStr = _T("{C=0xFFFF0000/}");
				}
			}
		}break;
	}

	int iNewType = Type;
	switch(Type)
	{
	case AT_RESIST_ADD_CURSE:
	case AT_RESIST_ADD_FIRE:	
	case AT_RESIST_ADD_ICE:
	case AT_RESIST_ADD_NATURE:
	case AT_RESIST_ADD_DESTROY:
	case AT_ATTACK_ADD_CURSE:
	case AT_ATTACK_ADD_FIRE:
	case AT_ATTACK_ADD_ICE:
	case AT_ATTACK_ADD_NATURE:
	case AT_ATTACK_ADD_DESTROY:
		{
			iNewType = Type + 5;
			break;
		}
	}

	int iResultValue = GetAbilValue(pUnit, iNewType);

	int const iUnitLevel = pUnit->GetAbil(AT_LEVEL);
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pUnit);
	float fMax = PgSkillHelpFunc::AdjustMaxValue(Type, FLT_MAX , iUnitLevel, iUnitLevel,pkPlayer);

	if(fMax != 0)
	{
		if(iResultValue > fMax)
		{
			vStr+= TTW(791580);	// 폰트 색
		}
	}

	PgItemUpgradeInfo::MakeValueString(vStr, Type, iResultValue, false, iUnitLevel);

	pAbliValue->Text(vStr);
	pAbliValue->SetCustomData(&Type, sizeof(Type));
}

void lwCharInfo::SetTotalAbilBonus(XUI::CXUI_Wnd* pWnd, BM::GUID const& Guid, wchar_t const* pText)
{
	PgPilot* pPilot = g_kPilotMan.FindPilot(Guid);
	if( !pPilot )
	{
		return;
	}

	CUnit* pUnit = pPilot->GetUnit();
	if( !pUnit )
	{
		return;
	}

	if(NULL==pWnd)
	{
		return;
	}

	if( !pText )
	{
		pWnd->Text(TTW(280));
	}
	else
	{
		pWnd->Text(pText);
	}

	XUI::CXUI_Wnd* pAbliValue = pWnd->GetControl(_T("FRM_VALUE"));
	if( !pAbliValue )
	{
		return;
	}

	int iResultValue = 0;

	for(int i = 0; i<4; ++i)
	{
		iResultValue += GetAbilValue(pUnit, AT_STR_ADD + i*3);
	}

	BM::vstring	vStr;
	vStr+=iResultValue;
	pAbliValue->Text(vStr);
	int iZero = 0;
	pAbliValue->SetCustomData(&iZero, sizeof(iZero));
}

int lwCharInfo::CalcPlusAbilValue(CUnit* pkUnit, EAbilType Type)
{
	int const iBaseAbil = GetBasicAbil(Type);
	int const iBaseValue = pkUnit->GetAbil(iBaseAbil);
	int iCompleteValue = pkUnit->GetAbil(Type);
	switch(Type)
	{
	case AT_C_PHY_ATTACK_MAX:
		{// 물리 공격력 툴팁을 보여줄때, %만큼 감소, 증가가 적용되어 있다면
			int iTotalPer = pkUnit->GetAbil(AT_PHY_DMG_PER);
			if(0 == iTotalPer)
			{
				iTotalPer = ABILITY_RATE_VALUE;
			}
			iTotalPer += pkUnit->GetAbil(AT_PHY_DMG_PER2);
			float fC_Phy_Dmg_Per = iTotalPer/ABILITY_RATE_VALUE_FLOAT;
			if(0 >= fC_Phy_Dmg_Per)
			{
				fC_Phy_Dmg_Per = 1.0f;
			}
			iCompleteValue = static_cast<int>(iCompleteValue*fC_Phy_Dmg_Per);
		}break;
	case AT_C_MAGIC_ATTACK_MAX:
		{// 마법 공격력 툴팁을 보여줄때, %만큼 감소, 증가가 적용되어 있다면
			int iTotalPer = pkUnit->GetAbil(AT_MAGIC_DMG_PER);
			if(0 == iTotalPer)
			{
				iTotalPer = ABILITY_RATE_VALUE;
			}
			iTotalPer += pkUnit->GetAbil(AT_MAGIC_DMG_PER2);
			float fC_Mag_Dmg_Per = iTotalPer/ABILITY_RATE_VALUE_FLOAT;
			if(0 >= fC_Mag_Dmg_Per)
			{
				fC_Mag_Dmg_Per = 1.0f;
			}
			iCompleteValue = static_cast<int>(iCompleteValue*fC_Mag_Dmg_Per);
		}break;
	//case AT_C_BLOCK_SUCCESS_VALUE:
	//	{
	//		if(0 < pkUnit->GetAbil(AT_C_FINAL_BLOCK_SUCCESS_RATE) )
	//		{
	//			return true;
	//		}
	//	}break;
	//case AT_C_DODGE_SUCCESS_VALUE:
	//	{
	//		if(0 < pkUnit->GetAbil(AT_C_FINAL_DODGE_SUCCESS_RATE) )
	//		{
	//			return true;
	//		}
	//	}break;
	//case AT_C_CRITICAL_SUCCESS_VALUE:
	//	{
	//		if(0 < pkUnit->GetAbil(AT_C_FINAL_CRITICAL_SUCCESS_RATE) );
	//		{
	//			return true;
	//		}
	//	}break;
	//case AT_C_HIT_SUCCESS_VALUE:
	//	{
	//		if(0 < pkUnit->GetAbil(AT_C_FINAL_HIT_SUCCESS_RATE) );
	//		{
	//			return true;
	//		}
	//	}break;
	default:
		{
		}break;
	}

	int const iPlusValue = iCompleteValue - iBaseValue;
	return iPlusValue;
}

lwWString lwCharInfo::lwViewPlusAbilValue(lwUIWnd UISelf)
{//캐릭 스텟 
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return lwWString("");
	}

	lwUIWnd UIParent = UISelf.GetParent().GetParent();
	BM::GUID	Guid = UIParent.GetOwnerGuid().GetGUID();

	EAbilType eType;
	pSelf->GetCustomData(&eType, sizeof(eType));

	EAbilType PlusType = (EAbilType)GetCalculateAbil(eType);
	PgPilot* pPilot = g_kPilotMan.FindPilot(Guid);
	if( !pPilot )
	{
		return lwWString("");
	}

	CUnit* pkUnit = pPilot->GetUnit();
//	if( !pkUnit )
	if( !pkUnit || pkUnit->IsUnitType(UT_PET) ) //펫 능력치 툴팁이 잘못 표시되어 일단 막아놓음 (향후 작업 예정)
	{
		return lwWString("");
	}
	
	switch( eType )
	{
	case AT_STR:
	case AT_INT:
	case AT_CON:
	case AT_DEX://펫의 기본능력치는 툴팁 필요 없음.
	case AT_STR_ADD:
	case AT_INT_ADD:
	case AT_CON_ADD:
	case AT_DEX_ADD:
		{//보너스 스테이터스는 툴팁 필요 없음.
			return lwWString("");
		}break;
	case AT_HEALTH:
		{
			eType = AT_R_MP_RECOVERY;
		}break;
	case AT_MENTAL:
		{
			eType = AT_SKILL_EFFICIENCY;
		}break;
	}

	//타이틀
	BM::vstring vStr;
	const TCHAR* pAbilTypeName = NULL;
	if( ::GetAbilName(eType, pAbilTypeName) )
	{
		vStr += ::TTW(4208);//Font&Color
		vStr += pAbilTypeName;
		vStr += L" ";
		vStr += ::TTW(5049);//상세정보
		vStr += ::TTW(40011);//폰트 초기화
		vStr += L"\n";
		vStr += L"\n";
	}
	
	int const iBaseAbil = GetBasicAbil(eType);
	int iBaseValue = pkUnit->GetAbil(iBaseAbil);

	int iTTW = 3010;//캐릭터3010
	if(UT_PET==pkUnit->UnitType())
	{
		iTTW = 1804;//펫
	}

	switch(eType)
	{
	case AT_C_HIT_SUCCESS_VALUE:
	case AT_HIT_SUCCESS_VALUE:
	case AT_C_DODGE_SUCCESS_VALUE:
	case AT_DODGE_SUCCESS_VALUE:
	case AT_C_BLOCK_SUCCESS_VALUE:
	case AT_BLOCK_SUCCESS_VALUE:
	case AT_C_BLOCK_DAMAGE_RATE:
	case AT_BLOCK_DAMAGE_RATE:
	case AT_C_CRITICAL_SUCCESS_VALUE:
	case AT_CRITICAL_SUCCESS_VALUE:
	case AT_C_CRITICAL_POWER:
	case AT_CRITICAL_POWER:
		{
			//캐릭터 총합
			vStr += ::TTW(40011);	// 폰트 복구
			vStr +=  ::TTW(iTTW);
			vStr += L" ";
			vStr += ::TTW(5050);
			vStr += L": ";
			int const iCalcAbil = GetCalculateAbil(iBaseAbil);
			int const iUnitLevel = pkUnit->GetAbil(AT_LEVEL);
			int iResultValue = 0;
			iResultValue = GetAbilValue(pkUnit, eType);
					
			PgItemUpgradeInfo::MakeValueString(vStr, eType, iResultValue, iUnitLevel, iUnitLevel);

			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
			float fMax = PgSkillHelpFunc::AdjustMaxValue(eType, FLT_MAX , iUnitLevel, iUnitLevel,pkPlayer);
			if(fMax != 0)
			{
				if(iResultValue > fMax)
				{
					vStr+= TTW(791580);	// 폰트 색
					vStr += TTW(791586);
				}		

			}

			vStr += L"\n";
			
			if(eType == AT_C_HIT_SUCCESS_VALUE ||
			   eType == AT_C_DODGE_SUCCESS_VALUE ||
			   eType == AT_C_CRITICAL_SUCCESS_VALUE)
			{
				vStr += ::TTW(40012);
				vStr += ::TTW(791584);
				vStr += L": ";
				vStr += ::TTW(40014);
				vStr += ::TTW(7951585);
				vStr += ::TTW(40011);
				vStr += L" X ";
				switch(eType)
				{
				case AT_C_HIT_SUCCESS_VALUE:
					{
						vStr += PgSkillHelpFunc::iHitValueStatic;
					}break;
				case AT_C_DODGE_SUCCESS_VALUE:
					{
						vStr +=PgSkillHelpFunc::iEvadeValueValueStatic;
					}break;
				case AT_C_CRITICAL_SUCCESS_VALUE:
					{
						int const iCriticalValueint = 15;
						vStr += BM::vstring(PgSkillHelpFunc::iCriticalStatic, L"%.1f");
						vStr += L" + ";
						vStr += iCriticalValueint;
					}break;
				}
				vStr += L"\n";
			}
		}break;
	default:
		{
			vStr += ::TTW(40012);
			vStr += L"[";
			vStr += ::TTW(iTTW);//캐릭터3010
			vStr += L"]\n";
			//캐릭터 총합
			vStr += ::TTW(1301);
			vStr +=  ::TTW(iTTW);
			vStr += L" ";
			vStr += ::TTW(5050);
			vStr += L": ";
			
			int const iUnitLevel = pkUnit->GetAbil(AT_LEVEL);
			PgItemUpgradeInfo::MakeValueString(vStr, eType, iBaseValue, iUnitLevel, iUnitLevel);
			vStr += L"\n";
			vStr += ::TTW(40011);
		}break;
	}

	switch(eType)
	{
	case AT_C_HIT_SUCCESS_VALUE:
	case AT_HIT_SUCCESS_VALUE:
		{
			ViewPlusAbil_HitSuccessValue(eType, pkUnit, vStr);
		}break;
	case AT_C_DODGE_SUCCESS_VALUE:
	case AT_DODGE_SUCCESS_VALUE:
		{
			ViewPlusAbil_DodgeSuccessValue(eType, pkUnit, vStr);
		}break;
	case AT_C_BLOCK_SUCCESS_VALUE:
	case AT_BLOCK_SUCCESS_VALUE:
		{
			ViewPlusAbil_BlockSuccessValue(eType, pkUnit, vStr);
		}break;
	case AT_C_BLOCK_DAMAGE_RATE:
	case AT_BLOCK_DAMAGE_RATE:
		{
			ViewPlusAbil_BlockDamage(eType, pkUnit, vStr);
		}break;
	case AT_C_CRITICAL_SUCCESS_VALUE:
	case AT_CRITICAL_SUCCESS_VALUE:
		{
			ViewPlusAbil_CriticalSuccessValue(eType, pkUnit, vStr);
		}break;
	case AT_C_CRITICAL_POWER:
	case AT_CRITICAL_POWER:
		{
			ViewPlusAbil_CriticalPower(eType, pkUnit, vStr);
		}break;
	default:
		{
			ViewPlusAbilDefault(eType, pkUnit, vStr);
		}break;
	}

	return lwWString((std::wstring const&)vStr);
}

void lwCharInfo::lwCallInfoDrop(lwUIWnd UIWnd, char const* szAddon, char const* szUIName)
{
	XUI::CXUI_Wnd* pDrop = XUIMgr.Get(_T("SFRM_CHARINFO_POPUP"));
	if( !pDrop || pDrop->IsClosed() )
	{
		pDrop = XUIMgr.Call(_T("SFRM_CHARINFO_POPUP"));
		if( !pDrop )
		{
			return;
		}
	}
	else
	{
		pDrop->Close();
	}

	XUI::CXUI_Wnd* pParent = UIWnd.GetSelf();
	if( !pParent )
	{
		return;
	}
	std::wstring wstrAddon = UNI(szAddon);

	std::wstring wstrName(_T("FRM_INFO_TYPE_"));
	wstrName += wstrAddon;
	XUI::CXUI_Wnd* pLabel = pParent->GetControl(wstrName);
	if( !pLabel )
	{
		return;
	}

	pDrop->SetCustomData(wstrAddon.c_str(), sizeof(std::wstring::value_type)*wstrAddon.size());
	POINT2	Pt(pLabel->TotalLocation().x, pLabel->TotalLocation().y + pLabel->Size().y);
	pDrop->Location(Pt);

	XUI::CXUI_Wnd* pTemp = pDrop->GetControl(_T("FRM_Option_BG"));
	if( pTemp )
	{
		std::wstring wstrFormName(UNI(szUIName));
		if( wstrFormName.size() )
		{
			pTemp->SetCustomData(wstrFormName.c_str(), sizeof(std::wstring::value_type)*wstrFormName.size());
		}
	}
}

bool lwCharInfo::lwViewEquipIconInitialize(lwUIWnd UIWnd, int const iInvType)
{
	XUI::CXUI_Wnd* pParent = UIWnd.GetSelf();
	if( !pParent )
	{
		return false;
	}

	BM::GUID	Guid = UIWnd.GetOwnerGuid().GetGUID();
	bool const bIsMyActor = g_kPilotMan.IsMyPlayer(Guid);

	PgPilot* pPilot = g_kPilotMan.FindPilot(Guid);
	if( !pPilot )
	{
		lwAddWarnDataTT(6006);
		return false;
	}

	CUnit* pUnit = pPilot->GetUnit();
	if( !pUnit )
	{
		return false;
	}

	PgInventory* pInv = NULL;
	if( bIsMyActor )
	{
		pInv = pUnit->GetInven();
	}
	else
	{
		pInv = &g_kOtherViewInv;
	}

	if( pInv )
	{
		if ( true == lwViewEquipIconInitialize2( pParent, pInv, iInvType ) )
		{
			if( iInvType != 0 )
			{
				pParent->SetCustomData( &iInvType, sizeof(iInvType) );
			}
			return true;
		}
	}

	return false;
}

bool lwCharInfo::lwViewEquipIconInitialize2( XUI::CXUI_Wnd* pkUI, PgInventory *pkInv, int const iInvType )
{
	if ( KUIG_NONE == iInvType )
	{
		int iValue = KUIG_NONE;
		pkUI->GetCustomData( &iValue, sizeof(iValue) );

		if ( KUIG_NONE != iValue )
		{
			return lwViewEquipIconInitialize2( pkUI, pkInv, iValue );
		}
		return false;
	}

	int const INV_POS_MAX = 16;
	int INV_POS[INV_POS_MAX+1] = { EQUIP_POS_GLASS, EQUIP_POS_BELT, EQUIP_POS_NECKLACE, EQUIP_POS_PANTS,
		EQUIP_POS_HELMET, EQUIP_POS_BOOTS, EQUIP_POS_SHOULDER, EQUIP_POS_RING_L, EQUIP_POS_CLOAK,
		EQUIP_POS_RING_R, EQUIP_POS_SHIRTS, EQUIP_POS_EARRING, EQUIP_POS_GLOVE, EQUIP_POS_MEDAL,
		EQUIP_POS_WEAPON, EQUIP_POS_SHEILD, EQUIP_POS_KICKBALL};

	XUI::CXUI_CheckButton* pButton = dynamic_cast<XUI::CXUI_CheckButton*>(pkUI->GetControl(_T("BTN_DEFAULT_VIEW")));
	XUI::CXUI_CheckButton* pButton2 = dynamic_cast<XUI::CXUI_CheckButton*>(pkUI->GetControl(_T("BTN_CASH_VIEW")));
	if( pButton && pButton2 )
	{
		switch(iInvType)
		{
		case KUIG_VIEW_OTHER_EQUIP:
		case KUIG_FIT:		
			{ 
				pButton->Check(true);
				pButton->ClickLock(true);
				INV_POS[0] = EQUIP_POS_PET;
			} break;
		case KUIG_VIEW_OTHER_EQUIP_CASH:
		case KUIG_FIT_CASH:	
			{ 
				pButton->ClickLock(false);
				pButton->Check(false);
			} break;
		default:
			{
			}break;
		}
		pButton2->ClickLock(!pButton->ClickLock());
		pButton2->Check(!pButton->Check());
	}

	int iInvPosSize = INV_POS_MAX+1;
	for( int i = 0; i < iInvPosSize; ++i )
	{
		int iIconNum = i;
		if( EQUIP_POS_KICKBALL == INV_POS[i] )
		{
			iIconNum = INV_POS_MAX-1;
		}

		BM::vstring	vStr(_T("FRM_EQUIP_ICON"));
		vStr += iIconNum;
		XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pkUI->GetControl(vStr));
		if( !pIcon )
		{
			continue;
		}
		SIconInfo Info = pIcon->IconInfo();
		Info.iIconGroup = iInvType;
		Info.iIconKey = INV_POS[i];
		pIcon->SetIconInfo(Info);

		XUI::CXUI_Wnd* pBgMain = pkUI->GetControl(_T("FRM_BG"));
		if( pBgMain )
		{
			BM::vstring vStr(_T("FRM_ICON_OUTLINE"));
			vStr += iIconNum;
			XUI::CXUI_Wnd* pTemp = pBgMain->GetControl(vStr);
			if( !pTemp )
			{
				continue;
			}

			vStr = _T("FRM_ICON_NAME");
			vStr += iIconNum;
			XUI::CXUI_Wnd* pTemp2 = pBgMain->GetControl(vStr);
			if( !pTemp2 )
			{
				continue;
			}
			EInvType InvType = static_cast<EInvType>(Info.iIconGroup);
			if ( InvType > KUIG_INV_VIEW )
			{
				InvType = ((Info.iIconGroup == KUIG_VIEW_OTHER_EQUIP)?(IT_FIT):(IT_FIT_CASH));
			}

			PgBase_Item kItem;
			if( pkInv->GetItem( InvType, Info.iIconKey, kItem) != S_OK )
			{
				pTemp->Visible(false);
				pTemp2->Visible(true);
				continue;
			}

			pTemp2->Visible(false);
			E_ITEM_GRADE const eGrade = GetItemGrade(kItem);
			if( eGrade >= 0 )
			{
				pTemp->Visible(true);
				SUVInfo UVInfo = pTemp->UVInfo();
				UVInfo.Index = (eGrade + 1);
				pTemp->UVInfo(UVInfo);
			}
			else
			{
				pTemp->Visible(false);
			}
			if( EQUIP_POS_SHEILD == INV_POS[i] )
			{
				break;
			}
		}
	}
	return true;
}

void lwCharInfo::UpdateMyActor()
{
	BM::GUID kGuid;
	g_kPilotMan.GetPlayerPilotGuid(kGuid);
	if( g_kMyActorViewMgr.ChangeEquip("CHARINFO", kGuid) )
	{
		XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("CharInfo"));
		if( !pWnd )
		{
			pWnd = XUIMgr.Get(_T("SFRM_OTHER_CharInfo"));
			if( !pWnd )
			{
				return;
			}
		}
		BM::GUID	kGuid;
		pWnd->GetCustomData(&kGuid, sizeof(kGuid));
		SetCharInfoToUI(pWnd, kGuid, false);
	}
}

void lwCharInfo::lwChangeInfoTab(lwUIWnd UISelf, int const iTab)
{	
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	BYTE const MAX_TAB_COUNT = 3;
	XUI::CXUI_Wnd* pTab[MAX_TAB_COUNT] = {0,};
	pTab[0] = pParent->GetControl(L"FRM_CHAR_INFO");
	pTab[1] = pParent->GetControl(L"FRM_PET_INFO");
	pTab[2] = pParent->GetControl(L"FRM_CHAR_CARD");

	for( int i = 0; i < MAX_TAB_COUNT; ++i )
	{
		BM::vstring	vStr(L"CBTN_TAB_");
		vStr += i;

		XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vStr));
		bool bThisTab = i==iTab;
		if( pBtn )
		{
			pBtn->ClickLock(bThisTab);
			pBtn->Check(bThisTab);
			pTab[i]->Visible(bThisTab);
		}
	}
}

void lwCharInfo::lwChangeOtherInfoTab(lwUIWnd UISelf, int const iTab)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = (pSelf->Parent())?(pSelf->Parent()->Parent()):(NULL);
	if( !pParent )
	{
		return;
	}

	BYTE const MAX_TAB_COUNT = 2;
	XUI::CXUI_Wnd* pTab[MAX_TAB_COUNT] = {0,};
	pTab[0] = pParent->GetControl(L"FRM_CHAR_INFO");
	pTab[1] = pParent->GetControl(L"FRM_CHAR_CARD");

	for( int i = 0; i < MAX_TAB_COUNT; ++i )
	{
		BM::vstring	vStr(L"SFRM_TAB");
		vStr += i;

		XUI::CXUI_Wnd* pkTab = pParent->GetControl(vStr);
		if( pkTab )
		{
			XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkTab->GetControl(L"CBTN_TAB"));
			bool bThisTab = i==iTab;
			if( pBtn )
			{
				pBtn->ClickLock(bThisTab);
				pBtn->Check(bThisTab);
				pTab[i]->Visible(bThisTab);
			}
		}
	}
}

void lwCharInfo::lwSetPetInfoToUI(lwUIWnd UIWnd, lwGUID kGuid)
{
	SetPetInfoToUI(UIWnd.GetSelf(), kGuid(), true);
}

void lwCharInfo::ChangePetActor(BM::GUID const kGuid)
{
	g_kMyActorViewMgr.DeleteActor("PetActor");
	g_kMyActorViewMgr.UpdatePet("PetActor", kGuid);
	PgPetUIUtil::lwDrawTextToPetState();
}

void lwCharInfo::SetPetInfoToUI(XUI::CXUI_Wnd* pWnd, BM::GUID const& rkGuid, bool const bIsChanged)
{
	if( !pWnd )
	{
		return;
	}
	pWnd->SetCustomData(&rkGuid, sizeof(rkGuid));

	PgPetUIUtil::SetPetUIDefaultState(false);
	PgPilot* pPilot = g_kPilotMan.FindPilot(rkGuid);
	if( !pPilot )
	{
		if( false == SetDiePetInfoToUI(pWnd, rkGuid) )
		{
			PgPetUIUtil::SetPetUIDefaultState(true);
		}
		else
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(pkPlayer)
			{
				PgInventory* pkInv = pkPlayer->GetInven();
				if( pkInv )
				{
					PgBase_Item kItem;
					if(S_OK==pkInv->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kItem) )	//유닛은 없는데 아이템은 있으면
					{
						XUI::CXUI_Wnd* pkCover = PgPetUIUtil::SetPetUIDefaultState(kItem);	//아이템이 시간만 다 된거라면
						if(pkCover)
						{
							pkCover->Text(TTW(149));
						}
					}
				}
			}
		}
		PgPetUIUtil::InitPetInfoUI(pWnd);
		return;
	}

	PgPet* pkPet = dynamic_cast<PgPet*>(pPilot->GetUnit());
	if( !pkPet )
	{
		return;
	}

	//////////////////////////////////////////////////////////////////////
	//1. 1차펫 2차펫에 따라 달라저야 할 UI
	//2. 라이딩펫인가 아닌가에 따라 달라져야 할 UI
	bool const b1stType = (EPET_TYPE_1 == static_cast<EPetType>(pkPet->GetPetType()));	//전투펫 이외는 모두 비활성화
	bool const b3rdType = (EPET_TYPE_3 == static_cast<EPetType>(pkPet->GetPetType()));	//전투펫 이외는 모두 비활성화

	XUI::CXUI_Wnd* pkBalloon = pWnd->GetControl(L"IMG_BALLOON");
	if( pkBalloon )
	{
		pkBalloon->Visible(false);
	}

	//XUI::CXUI_Wnd* pkBtn = pWnd->GetControl(L"BTN_EXPEND_USED");	//기간연장 버튼 숨김
	//if(pkBtn)
	//{
	//	pkBtn->Visible(b1stType);
	//}

	XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_COLOR_CHANGE"));	//컬러변경 버튼 숨김
	if(pkBtn)
	{
		bool const bShowBtn = (b1stType || b3rdType) && (PgPetUIUtil::GetPetDyeingRandColorMax() == 0);
		pkBtn->Disable(bShowBtn);
	}

	/*XUI::CXUI_Wnd* pkTab = pWnd->GetControl(L"FRM_USE_TITLE");	//사용시간
	if(pkTab)
	{
		pkTab->Text(TTW(b1stType ? 120 : 7508));
	}*/

	XUI::CXUI_Wnd* pkBuildTarget = NULL;	//1차펫용 아이템, 펫 동작버튼 중 선택
	/*for(int i=0; i<4; ++i)
	{
		BM::vstring kName1(L"IMG_ITEM_ICON_BG");
		BM::vstring kName2(L"BTN_PET_TRAIN");
		kName1+=i; kName2+=i;
		pkBuildTarget = pWnd->GetControl((std::wstring const&)kName1);
		if(pkBuildTarget)
		{
			pkBuildTarget->Visible(b1stType);
		}

		pkBuildTarget = pWnd->GetControl((std::wstring const&)kName2);
		if(pkBuildTarget)
		{
			pkBuildTarget->Visible(!b1stType);
		}
	}*/

	pkBuildTarget = pWnd->GetControl(L"FRM_TIRED_TEXT");	//피로도 대신 마나
	if(pkBuildTarget)
	{
		pkBuildTarget->Visible(!b3rdType);
		pkBuildTarget->Text(b1stType ? L"" : TTW(7509));
	}

	XUI::CXUI_AniBar* pkBar = dynamic_cast<XUI::CXUI_AniBar*>(pWnd->GetControl(L"BAR_TIRED"));	//피로도 대신 마나
	if(pkBar)
	{
		pkBar->Visible(!b3rdType);
		pkBar->Max(b1stType ? 100 : pkPet->GetAbil(AT_C_MAX_MP));
		pkBar->Now(b1stType ? 100 : pkPet->GetAbil(AT_MP));
	}

	/*pkBar = dynamic_cast<XUI::CXUI_AniBar*>(pWnd->GetControl(L"BAR_STATE"));	//피로도 대신 마나
	if(pkBar)
	{
		pkBar->Visible(!b1stType);
		if(!b1stType)
		{
			pkBar->Max(PgItem_PetInfo::MAX_PET_STATE_VALUE);
			pkBar->Now(PgItem_PetInfo::MAX_PET_STATE_VALUE);
//			pkBar->Now(PgPetUIUtil::CalcPetStateAvg(pkPet));
		}
	}*/

	XUI::CXUI_CheckButton *pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pWnd->GetControl(L"CBTN_SKILL"));
	if(pkChkBtn)
	{
		pkChkBtn->Visible(!b1stType);
		if(false==pkChkBtn->Check())
		{
			lua_tinker::call<void, lwUIWnd, int>("UI_PetInfoTab", lwUIWnd(pWnd->GetControl(L"CBTN_SKILL")), 0);	//항상 정보창으로 초기화
		}
	}

	//
	//////////////////////////////////////////////////////////////////////

	XUI::CXUI_Wnd* pkNameUI = pWnd->GetControl(L"SFRM_NAME_SHADOW");
	if(pkNameUI)
	{
		pkNameUI->Text(pkPet->Name());
	}

	XUI::CXUI_Wnd* pkUseUI = pWnd->GetControl(L"SFRM_USE_SHADOW");
	if(pkUseUI)
	{
		std::wstring wstrText;
		wstrText = TTW(179);
//		if(b1stType)
//		{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(pkPlayer)
		{
			PgInventory *pInv = pkPlayer->GetInven();
			if(pInv)
			{
				SItemPos const kPos = PgItem_PetInfo::ms_kPetItemEquipPos;
				PgBase_Item kBaseItem;
				if(S_OK==pInv->GetItem( kPos, kBaseItem))
				{
					__int64 i64AbilTime = kBaseItem.GetUseAbleTime();//초단위
					if(0i64==i64AbilTime)	//무기한 이거나 기간 만료
					{
						if(0i64<kBaseItem.EnchantInfo().IsTimeLimit())	//기간이 있는놈이면
						{
							wstrText = TTW(179);
						}
						else	//없는 놈
						{
							wstrText = TTW(178);
						}
					}
					else
					{
						wchar_t	szValue[40] = {0,};
						int const iRemainHour = static_cast<int>( i64AbilTime / 3600i64 );
						WstringFormat( wstrText, MAX_PATH, TTW(122).c_str(), iRemainHour / 24, iRemainHour % 24 );
					}					
				}
			}
		}
/*		}
		else
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(pkPlayer && pkPlayer->GetInven())
			{
				PgBase_Item kOut;
				if(S_OK==pkPlayer->GetInven()->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kOut) && !kOut.IsEmpty())
				{
					PgItem_PetInfo *pkPetInfo = NULL;
					if(kOut.GetExtInfo(pkPetInfo))
					{
						if(pkPetInfo->IsDead())
						{
							wstrText = TTW(329);
						}
					}
				}
			}
			if(wstrText.empty())
			{
				WstringFormat( wstrText, MAX_PATH, TTW(405001).c_str(), pkPet->GetConditionGrade()+1);
			}
		}*/

		pkUseUI->Text(wstrText);
	}

	int const iLev = pkPet->GetAbil(AT_LEVEL);

	XUI::CXUI_Wnd* pkLvUI = pWnd->GetControl(L"FRM_LV_TEXT");
	if(pkLvUI)
	{
		wchar_t	szValue[30] = {0,};
		swprintf(szValue, 29, TTW(5013).c_str(), iLev);
		pkLvUI->Text(szValue);
	}

	XUI::CXUI_Wnd* pkTypeUI = pWnd->GetControl(L"SFRM_TYPE_SHADOW");
	if(pkTypeUI)
	{
		int const iClass = pkPet->GetAbil(AT_CLASS);

		GET_DEF(PgClassPetDefMgr, kClassDefMgr);
		PgClassPetDef kPetDef;
		if ( true == kClassDefMgr.GetDef( SClassKey(iClass, iLev), &kPetDef ) )
		{
			int const iNameNo = kPetDef.GetAbil( AT_NAMENO );
			wchar_t const *pName = NULL;
			if( GetDefString(iNameNo, pName) )
			{
				pkTypeUI->Text(pName);
			}
			else
			{
				pkTypeUI->Text(L"");
			}
		}
	}

	int iSet = lwConfig_GetValue("CHARINFO", "PETINFO_LEFT");
	if (0 == iSet)
	{
		iSet = 100;
	}
	lwSetCharAbilInfo(lwUIWnd(pWnd->GetControl(L"FRM_INFO")), iSet, "L", true);
	PgPetUIUtil::lwUI_RefreshPetSkillUI(pWnd);
}

bool lwCharInfo::SetDiePetInfoToUI(XUI::CXUI_Wnd* pWnd, BM::GUID const& rkGuid)
{
	if( !pWnd ){ return false; }
	pWnd->SetCustomData(&rkGuid, sizeof(rkGuid));

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer ){ return false; }

	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv ){ return false; }

	PgBase_Item kItem;
	if( S_OK != pkInv->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kItem) )
	{
		return false;
	}

	PgItem_PetInfo* pkPetInfo = NULL;
	if( !kItem.GetExtInfo(pkPetInfo) )
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////
	//1차펫 2차펫에 따라 달라저야 할 UI
	GET_DEF(PgClassPetDefMgr, kClassPetDefMgr);
	PgClassPetDef kPetDef;
	if ( !kClassPetDefMgr.GetDef( pkPetInfo->ClassKey(), &kPetDef ) )
	{
		return false;
	}

	bool const b1stType = (EPET_TYPE_1 == static_cast<EPetType>(kPetDef.GetPetType()));
	bool const b3rdType = (EPET_TYPE_3 == static_cast<EPetType>(kPetDef.GetPetType()));

	XUI::CXUI_Wnd* pkBalloon = pWnd->GetControl(L"IMG_BALLOON");
	if( pkBalloon )
	{
		pkBalloon->Visible(false);
	}

	//XUI::CXUI_Wnd* pkBtn = pWnd->GetControl(L"BTN_EXPEND_USED");	//기간연장 버튼 숨김
	//if(pkBtn)
	//{
	//	pkBtn->Visible(b1stType);
	//}

	XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_COLOR_CHANGE"));	//컬러변경 버튼 숨김
	if(pkBtn)
	{
		bool const bShowBtn = (b1stType || b3rdType) && (PgPetUIUtil::GetPetDyeingRandColorMax() == 0);
		pkBtn->Disable(bShowBtn);
	}

	/*XUI::CXUI_Wnd* pkTab = pWnd->GetControl(L"FRM_USE_TITLE");	//사용시간
	if(pkTab)
	{
		pkTab->Text(TTW(b1stType ? 120 : 7508));
	}*/

	XUI::CXUI_Wnd* pkBuildTarget = NULL;	//1차펫용 아이템, 펫 동작버튼 중 선택
	/*for(int i=0; i<4; ++i)
	{
		BM::vstring kName1(L"IMG_ITEM_ICON_BG");
		BM::vstring kName2(L"BTN_PET_TRAIN");
		kName1+=i; kName2+=i;
		pkBuildTarget = pWnd->GetControl((std::wstring const&)kName1);
		if(pkBuildTarget)
		{
			pkBuildTarget->Visible(b1stType);
		}

		pkBuildTarget = pWnd->GetControl((std::wstring const&)kName2);
		if(pkBuildTarget)
		{
			pkBuildTarget->Visible(!b1stType);
		}
	}*/

	pkBuildTarget = pWnd->GetControl(L"FRM_TIRED_TEXT");	//피로도 대신 마나
	if(pkBuildTarget)
	{
		pkBuildTarget->Visible(!b3rdType);
		pkBuildTarget->Text(b1stType ? L"" : TTW(7509));
	}

	XUI::CXUI_AniBar* pkBar = dynamic_cast<XUI::CXUI_AniBar*>(pWnd->GetControl(L"BAR_TIRED"));	//피로도 대신 마나
	if(pkBar)
	{
		pkBar->Visible(!b3rdType);
		pkBar->Max(b1stType ? 100 : kPetDef.GetAbil(AT_C_MAX_MP));
		pkBar->Now(b1stType ? 100 : pkPetInfo->GetAbil(AT_MP));
	}

	/*pkBar = dynamic_cast<XUI::CXUI_AniBar*>(pWnd->GetControl(L"BAR_STATE"));	//피로도 대신 마나
	if(pkBar)
	{
		pkBar->Visible(!b1stType);
		if(!b1stType)
		{
			pkBar->Max(PgItem_PetInfo::MAX_PET_STATE_VALUE);
			pkBar->Now(0);
		}
	}*/
	//
	//////////////////////////////////////////////////////////////////////

	XUI::CXUI_Wnd* pkNameUI = pWnd->GetControl(L"SFRM_NAME_SHADOW");
	if(pkNameUI)
	{
		pkNameUI->Text(pkPetInfo->Name());
	}

	XUI::CXUI_Wnd* pkUseUI = pWnd->GetControl(L"SFRM_USE_SHADOW");
	if(pkUseUI)
	{
		std::wstring wstrText;
		wstrText = TTW(179);
		__int64 i64AbilTime = kItem.GetUseAbleTime();//초단위
		if(0i64>=i64AbilTime)	//무기한 이거나 기간 만료
		{
			if(0i64<kItem.EnchantInfo().IsTimeLimit())	//기간이 있는놈이면
			{
				wstrText = TTW(179);
			}
			else	//없는 놈
			{
				wstrText = TTW(178);
			}
		}
		else
		{
			wchar_t	szValue[40] = {0,};
			int const iRemainHour = static_cast<int>( i64AbilTime / 3600i64 );
			WstringFormat( wstrText, MAX_PATH, TTW(122).c_str(), iRemainHour / 24, iRemainHour % 24 );
		}		

		pkUseUI->Text(wstrText);
	}

	int const iLev = pkPetInfo->ClassKey().nLv;

	XUI::CXUI_Wnd* pkLvUI = pWnd->GetControl(L"FRM_LV_TEXT");
	if(pkLvUI)
	{
		wchar_t	szValue[30] = {0,};
		swprintf(szValue, 29, TTW(5013).c_str(), iLev);
		pkLvUI->Text(szValue);
	}

	XUI::CXUI_Wnd* pkTypeUI = pWnd->GetControl(L"SFRM_TYPE_SHADOW");
	if(pkTypeUI)
	{
		int const iNameNo = kPetDef.GetAbil( AT_NAMENO );
		wchar_t const *pName = NULL;
		if( GetDefString(iNameNo, pName) )
		{
			pkTypeUI->Text(pName);
		}
		else
		{
			pkTypeUI->Text(L"");
		}
	}

	PgPetUIUtil::lwUI_RefreshPetSkillUI(pWnd);
	PgPetUIUtil::lwDrawTextToPetState();
	return true;
}

int lwCharInfo::GetBonusStatus( CUnit* pkUnit, EAbilType Type )
{
	switch(Type)
	{
	case AT_C_STR:
		{
			return pkUnit->GetAbil(AT_STR_ADD);
		}break;
	case AT_C_INT:
		{
			return pkUnit->GetAbil(AT_INT_ADD);
		}break;
	case AT_C_CON:
		{
			return pkUnit->GetAbil(AT_CON_ADD);
		}break;
	case AT_C_DEX:
		{
			return pkUnit->GetAbil(AT_DEX_ADD);
		}break;
	}

	return 0;
}

bool lwCharInfo::lwIsRidingPet(void)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	PgPilot* pkPetPilot = g_kPilotMan.FindPilot(pkPlayer->SelectedPetID());
	if(pkPetPilot == NULL)
	{
		return false;
	}

	PgPet* pkPet = dynamic_cast<PgPet*>(pkPetPilot->GetUnit());
	if(pkPet == NULL || pkPet->UnitType() != UT_PET || pkPet->GetPetType() != EPET_TYPE_3)
	{ //펫이 없거나 라이딩 펫이 아니면
		return false;
	}
	return true;
}

void lwCharInfo::ViewPlusAbilDefault(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr)
{
	const TCHAR* pAbilTypeName = NULL;

	int const iBaseAbil = GetBasicAbil(eType);
	int iBaseValue = pkUnit->GetAbil(iBaseAbil);
	switch (eType)
	{
	case AT_SKILL_EFFICIENCY:
		{
			iBaseValue = static_cast<int>(pkUnit->GetAbil(eType) * 0.01f);
		}break;
	case AT_C_DODGE_SUCCESS_VALUE:
	case AT_C_HIT_SUCCESS_VALUE:
		{
			iBaseValue = PgItemUpgradeInfo::CalculateAbil(pkUnit, eType, EGetAbilType::EGAT_UNIT);
		}break;
	case AT_R_MP_RECOVERY:
		{
			iBaseValue = pkUnit->GetAbil(AT_HEALTH);
		}break;
	case AT_ATTACK_ADD_FIRE:
	case AT_ATTACK_ADD_ICE:
	case AT_ATTACK_ADD_NATURE:
	case AT_ATTACK_ADD_CURSE:
	case AT_ATTACK_ADD_DESTROY:
	case AT_RESIST_ADD_FIRE:
	case AT_RESIST_ADD_ICE:
	case AT_RESIST_ADD_NATURE:
	case AT_RESIST_ADD_CURSE:
	case AT_RESIST_ADD_DESTROY:
		{
			iBaseValue = pkUnit->GetAbil(eType+320);
		}break;
	}

	//캐릭터 기본
	rvStr += ::TTW(5992);//기본
	if( pAbilTypeName )
	{
		rvStr += L" ";
		rvStr += pAbilTypeName;
	}
	rvStr += L": ";

	switch( eType )
	{
	case AT_STR:
	case AT_INT:
	case AT_CON:
	case AT_DEX:
		{
			int const iBnsStatus = GetBonusStatus(pkUnit, eType);
			rvStr += (iBaseValue - iBnsStatus);//보너스 만큼 빼야 기본값임.
		}break;
	default:
		{
			int const iUnitLevel = pkUnit->GetAbil(AT_LEVEL);
			PgItemUpgradeInfo::MakeValueString(rvStr, eType, iBaseValue, iUnitLevel, iUnitLevel);
		}break;
	}

	rvStr += L"\n";

	//보너스 스테이터스(Active Status)
	switch( eType )
	{
	case AT_STR_ADD:
	case AT_INT_ADD:
	case AT_CON_ADD:
	case AT_DEX_ADD:
		{
			rvStr += TTW(790680);//보너스
			if( pAbilTypeName )
			{
				rvStr += L" ";
				rvStr += pAbilTypeName;
			}
			rvStr += L": ";
			int const iBnsStatus = GetBonusStatus(pkUnit, eType);
			rvStr += iBnsStatus;
			rvStr += _T("{C=0xFFFF0000/}");
			rvStr += L"\n";
		}break;
	}

	std::wstring wstrEnchantText = L"\n";
	wstrEnchantText += ::TTW(40012);
	wstrEnchantText += L"[";
	wstrEnchantText += ::TTW(3004);//아이템
	wstrEnchantText += L"]";
	wstrEnchantText += ::TTW(40011);//폰트 초기화	

	//인챈트된 수치
	int iItemTotalValue = 0;
	if( ::MakeToolTipText_Status(eType, wstrEnchantText, pkUnit, &iItemTotalValue) )
	{
		rvStr += wstrEnchantText;
	}
	
	if( AT_PHY_ATTACK_MAX == iBaseAbil
		|| AT_MAGIC_ATTACK_MAX == iBaseAbil
		)
	{// 증폭률 ( 기본은 10000이다 )
		rvStr += L"\n";
		rvStr += ::TTW(40012);
		rvStr += L"[";
		rvStr += ::TTW(790804);		//[증폭률]
		rvStr += L"]\n";
		
		rvStr += ::TTW(1301);	// 폰트 초기화
		
		// 총합
		rvStr +=  ::TTW(790805);
		rvStr += L": ";

		int const iDmg_Per_Abil = (AT_PHY_ATTACK_MAX == iBaseAbil) ? AT_PHY_DMG_PER : AT_MAGIC_DMG_PER;
		int const iDmg_Per_Abil2 = (AT_PHY_ATTACK_MAX == iBaseAbil) ? AT_PHY_DMG_PER2 : AT_MAGIC_DMG_PER2;
		int iDmg_Per_Abil_Vale = pkUnit->GetAbil(iDmg_Per_Abil);
		int const iDmg_Per_Abil_Vale2 = pkUnit->GetAbil(iDmg_Per_Abil2);
		if( 0 == iDmg_Per_Abil_Vale  ) 
		{
			iDmg_Per_Abil_Vale  = ABILITY_RATE_VALUE;
		}
		iDmg_Per_Abil_Vale += iDmg_Per_Abil_Vale2;
		int const iRealIncreaseRate = iDmg_Per_Abil_Vale  - ABILITY_RATE_VALUE;
		float const fPer = MAKE_ABIL_RATE(iRealIncreaseRate / ABILITY_RATE_VALUE_FLOAT);
		rvStr += L"#PER#%";
		rvStr.Replace( L"#PER#", BM::vstring( fPer, L"%.1f") );

		rvStr += L"(";
		rvStr += ::TTW(790807);
		rvStr += L" ";
		int const iSum = (iBaseValue +iItemTotalValue); // 캐릭터 총합 + 아이템 총합 공격력
		int const iApproximatelyValue = ( iSum * iRealIncreaseRate ) / ABILITY_RATE_VALUE;
		rvStr += iApproximatelyValue;
		rvStr += L")";
		rvStr += L"\n";
		rvStr += ::TTW(40011);
	}
	

	//스킬에 의한 어쩌고 
	rvStr += L"\n";
	rvStr += ::TTW(40012);
	rvStr += ::TTW(5070);
}

void lwCharInfo::ViewPlusAbil_HitSuccessValue(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr)
{
	int const iType = GetBasicAbil( static_cast<int>(eType) );
	ViewPlusAbil_MaxLimit(iType, pkUnit, rvStr, false, false);
	ViewPlusAbil_SubFunc(iType, pkUnit, rvStr, 791555
		,true, true, true
		,false);

	int const iPerType = GetRateAbil(iType);
	ViewPlusAbil_SubFunc(iPerType, pkUnit, rvStr, 791558
		,false, true, true
		,true);

	int const iFianlRateType = PgSkillHelpFunc::GetFinalRateAbil(iType);
	ViewPlusAbil_SubFunc(iFianlRateType, pkUnit, rvStr, 791559
		,false, true, true
		, true);

	{//"최종 명중률"
		rvStr+=::TTW(40012); rvStr+="\n[";
		rvStr+=TTW(791560);					
		rvStr+="]\n";rvStr+=::TTW(40011);	// 폰트 초기화
	}
	{// 계산
		int const iDodgeSucessValue = pkUnit->GetAbil(AT_DODGE_SUCCESS_VALUE);
		rvStr+=TTW(791569); rvStr+="\n";
		rvStr.Replace(L"#VAL#", iDodgeSucessValue);
		
		int const iUnitLevel = pkUnit->GetAbil(AT_LEVEL);
		int const iDodgeRate = PgSkillHelpFunc::CalcSuccessValueToRate( AT_DODGE_SUCCESS_VALUE, iDodgeSucessValue, AT_FINAL_DODGE_SUCCESS_RATE, 0, iUnitLevel, iUnitLevel );
		int const iHitRate = PgSkillHelpFunc::CalcSuccessValueToRate( PgSkillHelpFunc::EAT_HIT, pkUnit, pkUnit, NULL, 0);
		float fRate = (iHitRate - iDodgeRate + ABILITY_RATE_VALUE);
		fRate = fRate/ _10000_TO_1PER;
		fRate = std::min(fRate, 100.0f);
		fRate = std::max(fRate, 0.0f);

		BM::vstring vValue = TooltipUtil::GetValueColor(fRate);
		vValue += BM::vstring(fRate, L"%.1f");
		vValue += L"%";
		vValue += ::TTW(1554);
		rvStr.Replace(L"#RATE#", vValue);
	}
}

void lwCharInfo::ViewPlusAbil_DodgeSuccessValue(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr)
{
	int const iType = GetBasicAbil( static_cast<int>(eType) );
	ViewPlusAbil_MaxLimit(iType, pkUnit, rvStr, true, false);
	ViewPlusAbil_SubFunc(iType, pkUnit, rvStr, 791556
		,true, true, true
		,false);

	int const iPerType = GetRateAbil(iType);
	ViewPlusAbil_SubFunc(iPerType, pkUnit, rvStr, 791561
		,false, true, true
		,true);

	int const iFianlRateType = PgSkillHelpFunc::GetFinalRateAbil(iType);
	ViewPlusAbil_SubFunc(iFianlRateType, pkUnit, rvStr, 791562
		,false, true, true
		, true);

	{// 최종회피율
		rvStr+=::TTW(40012); rvStr+="\n[";
		rvStr+=TTW(791563);					
		rvStr+="]\n";rvStr+=::TTW(40011);	// 폰트 초기화
	}
	{// 계산
		int const iHitSuccessValue= pkUnit->GetAbil(AT_HIT_SUCCESS_VALUE);
		rvStr+=TTW(791570); rvStr+="\n";
		rvStr.Replace(L"#VAL#", iHitSuccessValue);
		
		int const iUnitLevel = pkUnit->GetAbil(AT_LEVEL);
		int const iHitRate = PgSkillHelpFunc::CalcSuccessValueToRate( AT_HIT_SUCCESS_VALUE, iHitSuccessValue, AT_FINAL_HIT_SUCCESS_RATE, 0, iUnitLevel, iUnitLevel );
		int const iDodgeRate = PgSkillHelpFunc::CalcSuccessValueToRate( PgSkillHelpFunc::EAT_DODGE, pkUnit, pkUnit, NULL, 0 );
		float fRate = (iDodgeRate - iHitRate);
		fRate = fRate/ _10000_TO_1PER;
		fRate = std::min(fRate, 100.0f);
		fRate = std::max(fRate, 0.0f);
		
		BM::vstring vValue = TooltipUtil::GetValueColor(fRate);
		vValue += BM::vstring(fRate, L"%.1f");
		vValue += L"%";
		vValue += ::TTW(1554);
		rvStr.Replace(L"#RATE#", vValue);
	}
}

void lwCharInfo::ViewPlusAbil_BlockSuccessValue(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr)
{
	int const iType = GetBasicAbil( static_cast<int>(eType) );
	ViewPlusAbil_MaxLimit(iType, pkUnit, rvStr, true, false);
	ViewPlusAbil_SubFunc(iType, pkUnit, rvStr, 791551
		,false, true, true
		,false);

	int const iPerType = GetRateAbil(iType);
	ViewPlusAbil_SubFunc(iPerType, pkUnit, rvStr, 791581
		,false, true, true
		,true);

	int const iFianlRateType = PgSkillHelpFunc::GetFinalRateAbil(iType);
	ViewPlusAbil_SubFunc(iFianlRateType, pkUnit, rvStr, 791582
		,false, true, true
		, true);

	{//최종블록성공률
		rvStr+=::TTW(40012); rvStr+="\n[";
		rvStr+=TTW(791583);					
		rvStr+="]\n";rvStr+=::TTW(40011);	// 폰트 초기화
	}
	{// 계산
		rvStr+=TTW(791571); rvStr+="\n";
		float fRate = PgSkillHelpFunc::CalcSuccessValueToRate( PgSkillHelpFunc::EAT_BLOCK, pkUnit, pkUnit, NULL, 0 );
		fRate = fRate/ _10000_TO_1PER;
		fRate = std::min(fRate, 100.0f);
		fRate = std::max(fRate, 0.0f);

		BM::vstring vValue = TooltipUtil::GetValueColor(fRate);
		vValue += BM::vstring(fRate, L"%.1f");
		vValue += L"%";
		vValue += ::TTW(1554);
		rvStr.Replace(L"#RATE#", vValue);
	}
	{// 설명
		rvStr+="\n";
		rvStr+=::TTW(40012);
		rvStr+=TTW(791573);
	}
}

void lwCharInfo::ViewPlusAbil_CriticalSuccessValue(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr)
{
	int const iType = GetBasicAbil( static_cast<int>(eType) );
	ViewPlusAbil_MaxLimit(iType, pkUnit, rvStr, true, false);
	ViewPlusAbil_SubFunc(iType, pkUnit, rvStr, 791553
		,true, true, true
		,false);

	int const iPerType = GetRateAbil(iType);
	ViewPlusAbil_SubFunc(iPerType, pkUnit, rvStr, 791564
		,false, true, true
		,true);

	int const iFianlRateType = PgSkillHelpFunc::GetFinalRateAbil(iType);
	ViewPlusAbil_SubFunc(iFianlRateType, pkUnit, rvStr, 791565
		,false, true, true
		, true);
	{// 최종크리티컬성공률
		rvStr+=::TTW(40012); rvStr+="\n[";
		rvStr+=TTW(791566);					
		rvStr+="]\n";rvStr+=::TTW(40011);	// 폰트 초기화
	}
	{// 계산
		rvStr+=TTW(791572); rvStr+="\n";
		float fRate = PgSkillHelpFunc::CalcSuccessValueToRate( PgSkillHelpFunc::EAT_CRITICAL, pkUnit, pkUnit , NULL, 0 );
		fRate = fRate/ _10000_TO_1PER;
		fRate = std::min(fRate, 100.0f);
		fRate = std::max(fRate, 0.0f);

		BM::vstring vValue = TooltipUtil::GetValueColor(fRate);
		vValue += BM::vstring(fRate, L"%.1f");
		vValue += L"%";
		vValue += ::TTW(1554);
		rvStr.Replace(L"#RATE#", vValue);
	}
	{// 설명
		rvStr+="\n";
		rvStr+=::TTW(40012);
		rvStr+=TTW(791576);
	}
}

void lwCharInfo::ViewPlusAbil_BlockDamage(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr)
{
	int const iType = GetBasicAbil( static_cast<int>(eType) );
	ViewPlusAbil_MaxLimit(iType, pkUnit, rvStr, true, true);
	ViewPlusAbil_SubFunc(iType, pkUnit, rvStr, 791567
		,false, true, true
		,true);
	{// 설명
		rvStr+="\n";
		rvStr+=::TTW(40012);
		rvStr+=TTW(791573);
	}
}

void lwCharInfo::ViewPlusAbil_CriticalPower(EAbilType eType, CUnit* pkUnit, BM::vstring& rvStr)
{
	int const iType = GetBasicAbil( static_cast<int>(eType) );
	ViewPlusAbil_MaxLimit(iType, pkUnit, rvStr, true, true);
	ViewPlusAbil_SubFunc(iType, pkUnit, rvStr, 791568
		,true, true, true
		,true);
	{// 설명
		rvStr+="\n";
		rvStr+=::TTW(40012);
		rvStr+=TTW(791576);
	}
}

void lwCharInfo::ViewPlusAbil_SubFunc(int iType, CUnit* pkUnit, BM::vstring& rvStr
									,int iAbilTypeTT
									,bool bKeepDisplay_Stat
									,bool bKeepDisplay_Inv
									,bool bKeepDisplay_Effect
									,bool bIsPercent)
{
	if(!pkUnit
		|| !iType
		)
	{
		return;
	}
	int const iUnitLevel = pkUnit->GetAbil(AT_LEVEL);
	int const iUnitVal = pkUnit->GetAbil(iType);

	int iInvVal = 0;
	PgInventory* pkInv = pkUnit->GetInven();
	if(pkInv)
	{
		iInvVal = pkInv->GetAbil(iType);
	}

	int const iEffectVal = pkUnit->GetEffectMgr().GetAbil(iType);

	if(iUnitVal
		|| iInvVal
		|| iEffectVal
		|| (bKeepDisplay_Stat || bKeepDisplay_Inv || bKeepDisplay_Effect)
		)
	{
		rvStr+=::TTW(40012);		// 주황색
		rvStr+="\n[";
		rvStr+=TTW(iAbilTypeTT);	/*"[수치증가]"*/
		rvStr+="]\n";
		rvStr+=::TTW(40011);		// 폰트 초기화

		if(iUnitVal
			|| bKeepDisplay_Stat
			)
		{// Stat
			MakeValueStr(rvStr, 791577, L"#VAL#", static_cast<float>(iUnitVal), bIsPercent);
			rvStr+="\n";
		}
		if(iInvVal
			|| bKeepDisplay_Inv
			)
		{// 아이템
			MakeValueStr(rvStr, 791578, L"#VAL#", static_cast<float>(iInvVal), bIsPercent);
			rvStr+="\n";
		}
		if(iEffectVal
			|| bKeepDisplay_Effect
			)
		{// 이펙트
			MakeValueStr(rvStr, 791579, L"#VAL#", static_cast<float>(iEffectVal), bIsPercent);
			rvStr+="\n";
		}
	}
}

void lwCharInfo::ViewPlusAbil_MaxLimit(int iType, CUnit* pkUnit, BM::vstring& rvStr, bool bKeepDisplay, bool bIsPercent)
{//	(최대제한 : 0000)  - @없을경우 안띄움
	if(!pkUnit)
	{
		return;
	}
	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkUnit);
	int const iUnitLevel = pkUnit->GetAbil(AT_LEVEL);
	float fMax = PgSkillHelpFunc::AdjustMaxValue(iType, FLT_MAX , iUnitLevel, iUnitLevel,pkPlayer);
	if( FLT_MAX == fMax ) 
	{ 
		fMax = 0.0f;
	};

	if(fMax 
		|| bKeepDisplay
		)
	{
		rvStr+=TTW(791580);	// 폰트 색
		rvStr+="(";
		MakeValueStr(rvStr,791557, L"#MAX#", fMax, bIsPercent); 
		rvStr+=")";
		rvStr+="\n";
	}
}

void lwCharInfo::MakeValueStr(BM::vstring& rvStr, int const iTT, std::wstring const kMark, float fValue, bool const bIsPercent)
{
	if(bIsPercent) { fValue = fValue / _10000_TO_1PER; }
	if( 0 > fValue )
	{
		rvStr += TTW(89999);
	}
	rvStr+=TTW(iTT);
	if(bIsPercent) { rvStr+="%"; }
	rvStr.Replace( kMark, bIsPercent ? BM::vstring(fValue, L"%.1f") : BM::vstring(fValue, L"%.0f") );
	if( 0 > fValue )
	{
		rvStr += TTW(40011);
	}
}