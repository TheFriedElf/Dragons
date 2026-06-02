#include "stdafx.h"
#include "lwUI.h"
#include "lwPoint2.h"
#include "lwGUID.h"
#include "ServerLib.h"
#include "PgSkillTree.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "Variant/PgPlayer.h"
#include "Variant/ItemMakingDefMgr.h"
#include "Pg2DString.H"
#include "PgMobileSuit.h"
#include "variant/PgItemOptionMgr.h"
#include "PgSkillTree.H"
#include "PgGuild.h"
#include "PgCashShop.h"
#include "Variant/Global.h"
#include "PgChatMgrClient.h"
#include "variant/PgClassPetDefMgr.h"
#include "lwUnit.h"
#include "lwMarket.h"
#include "variant/inventoryutil.h"
#include "PgFontSwitchTable.h"
#include "lwUIItemSocketSystem.h"
#include "lwUIItemRarityUpgrade.h"
#include "lwUIItemPlusUpgrade.h"
#include "lwUIToolTip.h"
#include "lwUICostumeMix.h"
#include "lwUIQuest.h"
#include "PgAchieveNfyMgr.h"
#include "lwUIMission.h"
#include "variant/PgJobSkillTool.h"
#include "variant/PgJobSkill.h"
#include "variant/ItemSkillUtil.h"
#include "lwJobSkillItem.h"
#include "variant/PgItemRarityUpgradeFormula.h"
#include "Variant/PgSocketFormula.h"
#include "variant/PgSoulTransfer.h"
#include "lwDefenceMode.h"
#include "PgOption.h"
#include "PgQuestMan.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "variant/PgSimpleTime.h"
#include "variant/Global.h"

std::wstring const NULL_STRING = L"";
std::wstring const ENTER_STRING = L"\n";
std::wstring const NULL_ENTER_STRING = L"\n ";
std::wstring const ADD_MARK_STRING = L"  + ";
std::wstring const AREA_MARK_STRING = L" - ";
std::wstring const SPACE_STRING = L" ";
std::wstring const CUT_TOOLTIP = L"#CUTTOOLTIP#";
std::wstring const CUT_TOOLTIP2 = L"#CUTTOOLTIP2#";

int const TITLE_COLOR_ORANGE = 40012;
int const TITLE_COLOR_YELLOW = 40013;
int const CONTENTS_COLOR_WHITE = 40011;
int const CONTENTS_COLOR_BLUE = 40062;
int const CONTENTS_COLOR_RED = 89999;
int const CONTENTS_COLOR_GRAY = 40061;
int const Item_SWAP = 16;
int const Item_PLUS = 17;

char const* szSuperGirlToolTipID = "ToolTipSuperGirl";
char const* szMonsterCardToolTipID = "ToolTipMonsterCard";
char const* szSkillExtendToolTipID = "ToolTipSkillExtend";
char const* szCardImgID = "ToolTip_Image";
char const* szCardAddImg1ID = "ToolTip_Image_Add01";
char const* szCardAddImg2ID = "ToolTip_Image_Add02";
char const* szCardAddImg3ID = "ToolTip_Image_Add03";
char const* szDefaultToolTipID = "TOOLTIP2";
char const* szDefaultMutableToolTipID = "M_ToolTip";
char const* szTTEABoxID = "SFRM_TTEA_BOX";
char const* szTTImgID = "ToolTip_Image";
char const* szToolTipPackageItemID = "TOOLTIP_PACKAGEITEM";

extern bool FormatMoney(__int64 const iTotal, std::wstring &rkOut);
extern bool g_bIsEqComp;
extern PgInventory g_kOtherViewInv;//상대보기 인벤.
extern PgStore g_kViewStore;//지금 보고있는 
extern bool lwGetHaveSkill(lwUnit kUnit, int iSkillNo, bool const bOverSkill = false);
extern bool MakeMonsterCardTimeLimit(PgBase_Item const &kItem, std::wstring & wstrText);
extern bool CheckEnchantBundle(PgBase_Item const& kItem);
extern PgPet* GetMySelectedPet();
extern bool GetMySelectedPetInfo( PgItem_PetInfo &rkOutPetInfo );
extern bool MakeActivateSkillToolTipText_Pet( BM::vstring &vstrText, PgItem_PetInfo const &kPetInfo, unsigned int iSkillIndex );

void UserCharacterIcon(int iIconNo,std::wstring & wstrText, char const* wndName = NULL);

void MakeBaseAbilStr(PgBase_Item const& kItem, CItemDef const* pDef, std::wstring& kOutText);

void GetCantAbilString(int const iCantAbil, std::wstring & wstrCantAbil);

unsigned __int64 FastLog64 (unsigned __int64 uiNum)
{
	unsigned __int64 uiLog = 0;
	unsigned __int64 uiTemp = uiNum >> 1;
	while ( uiTemp & 0xFFFFFFFFFFFFFFFF )
	{
		uiTemp >>= 1;
		uiLog++;
	}
 
	return uiLog;
}

__int64 GetMoneyMU( EMonetaryUnit const kMU, __int64 const i64Money )
{
	switch( kMU )
	{
	case EMoney_Gold://골드
		{
			return i64Money / 10000i64;
		}break;
	case EMoney_Silver://실버
		{
			return (i64Money / 100i64) % 100i64;
		}break;
	case EMoney_Copper://쿠퍼
		{
			return i64Money % 100i64;
		}break;
	case EMoney_Cash://캐쉬
		{
			return i64Money;
		}break;
	case EMoeny_GuildMoney://길드머니(길드금고는 골드만)
		{
			return i64Money / 10000i64;
		}break;
	}
	return i64Money;
}

std::wstring const GetMoneyString( __int64 const i64Money, bool const bFilterZero, bool const bUseIcon )
{
	BM::vstring	vStr;

	__int64 i64Temp = GetMoneyMU( EMoney_Gold, i64Money );
	if ( i64Temp || !bFilterZero )
	{
		vStr += i64Temp;
		if( bUseIcon )
		{
			vStr += _T(" ");
			vStr += g_kEmoFontMgr.Trans_Key_SysFontString(std::string("GOLD"));
		}
		else
		{
			vStr += TTW(401111).c_str();
		}
	}

	i64Temp = GetMoneyMU( EMoney_Silver, i64Money);
	if ( i64Temp || !bFilterZero )
	{
		vStr += _T("  ");
		vStr += i64Temp;
		if( bUseIcon )
		{
			vStr += _T(" ");
			vStr += g_kEmoFontMgr.Trans_Key_SysFontString(std::string("SILVER"));
		}
		else
		{
			vStr += TTW(401112).c_str();
		}
	}

	i64Temp = GetMoneyMU( EMoney_Copper, i64Money );
	if ( 0 == vStr.size() || i64Temp || !bFilterZero )
	{
		vStr += _T("  ");
		vStr += i64Temp;
		if( bUseIcon )
		{
			vStr += _T(" ");
			vStr += g_kEmoFontMgr.Trans_Key_SysFontString(std::string("COPPER"));
		}
		else
		{
			vStr += TTW(401113).c_str();
		}
	}
	
	return (std::wstring const&)(vStr);
}

bool MakeTextJobSkill3Resource(int const iRecipeItemNo, std::wstring & wstrTextOut)
{
	bool bRet = false;
	if( iRecipeItemNo )
	{
		CONT_DEF_JOBSKILL_RECIPE::mapped_type kJobSkillRecipe;
		if( JobSkill_Third::GetJobSkill3_Recipe(iRecipeItemNo, kJobSkillRecipe) )
		{
			bool bFirst = false;
			for(int i=0; i<MAX_JS3_RECIPE_RES; ++i)
			{
				SJobSkillRes const & kRes = kJobSkillRecipe.kResource[i];

				if(0 == kRes.iCount)
				{
					continue;
				}

				wchar_t const* pkItemName = NULL;
				if(false == GetItemName(kRes.iGroupNo, pkItemName))
				{
					continue;
				}

				if(false==bFirst)
				{
					wstrTextOut += ENTER_STRING;
					wstrTextOut += TTW(40055);	// 재료
					bFirst = true;
				}
				else
				{
					wstrTextOut += ENTER_STRING;
				}

				BM::vstring vStr(TTW(0<kRes.iGrade ? 799809 : 799825));
				vStr.Replace(L"#NAME#", pkItemName);
				vStr.Replace(L"#GRADE#", kRes.iGrade);
				vStr.Replace(L"#COUNT#", kRes.iCount);

				wstrTextOut += TTW(40051);	// 글씨 색깔.
				wstrTextOut += static_cast<std::wstring>(vStr);
				bRet = true;
			}
		}
	}
	return bRet;
}

bool MakeTextJobSkill_Info(SJobSkillSaveIdx const * const pkJobSkillSaveIdx, std::wstring & wstrTextOut)
{
	bool bRet = false;
	PgPlayer const * pkPlayer = g_kPilotMan.GetPlayerUnit();

	if( pkJobSkillSaveIdx && pkPlayer )
	{
		BM::vstring kGatherType;
		GetJobSkillText_GatherType(pkJobSkillSaveIdx->iNeedSkillNo01, kGatherType);

		//주스킬 이름
		wchar_t const* pName = NULL;
		GetDefString(pkJobSkillSaveIdx->iNeedSkillNo01,pName);

		if( 0<kGatherType.size() && pName )
		{
			wstrTextOut += ENTER_STRING;
			wstrTextOut += TTW(799676);
			wstrTextOut += ENTER_STRING;

			bool bCantUse = false;
			if(false == JobSkill_LearnUtil::IsEnableUseJobSkill(pkPlayer, pkJobSkillSaveIdx->iNeedSkillNo01) )
			{
				bCantUse = true;
			}
			int const iMyExpertness = pkPlayer->JobSkillExpertness().Get(pkJobSkillSaveIdx->iNeedSkillNo01);
			if( iMyExpertness < pkJobSkillSaveIdx->iNeedSkillExpertness01 )
			{
				bCantUse = true;
			}
			if(bCantUse)
			{
				wstrTextOut += TTW(CONTENTS_COLOR_RED);
			}

			BM::vstring kNeedSkill(TTW(799659));
			kNeedSkill.Replace( L"#GATHER#", BM::vstring(kGatherType) );
			kNeedSkill.Replace( L"#SKILL#", BM::vstring(pName) );
			kNeedSkill.Replace( L"#EXP#",  pkJobSkillSaveIdx->iNeedSkillExpertness01/ABILITY_RATE_VALUE);
			wstrTextOut += static_cast<std::wstring>(kNeedSkill);
			if(bCantUse)
			{
				wstrTextOut += TTW(40011);	//폰트 초기화
			}

			wstrTextOut += ENTER_STRING;
			bRet = true;
		}

		//
		if( MakeTextJobSkill3Resource(pkJobSkillSaveIdx->iRecipeItemNo, wstrTextOut) )
		{
			wstrTextOut += ENTER_STRING;
			bRet = true;
		}
	}
	return bRet;
}

void MakeAbil_Enchant(SItemExtOptionKey const &kKey, CONT_ENCHANT_ABIL & rkOut, int const iIdx, SBasicOptionAmpKey const kAmpKey)
{
	GET_DEF(PgItemOptionMgr, kItemOptionMgr);
	CAbilObject kTmp;
	kItemOptionMgr.GetBasicAbil(kKey, &kTmp);

	PgItemRarityUpgradeFormula::ApplyBasicOptionAmp(static_cast<EEquipType>(kAmpKey.kPriKey), kAmpKey.kSecKey, kAmpKey.kTrdKey, kTmp);

	SAbilIterator kIter;
	kTmp.FirstAbil(&kIter);
	while(kTmp.NextAbil(&kIter))
	{
		rkOut.push_back(SEnchantAbil(kKey.kSecKey, kKey.kQudKey, kKey.kTrdKey, kIter.wType, kIter.iValue, iIdx));
	}
}

void MakeCantAbil(std::wstring &wstrText, CItemDef const* pkItemDef)
{
	int	const iCantAbil = pkItemDef->GetAbil(AT_ATTRIBUTE);
	if(iCantAbil)
	{
		wstrText += ENTER_STRING;
		wstrText += TTW(TITLE_COLOR_YELLOW);
		wstrText += TTW(2501);
		wstrText += TTW(40011);	//폰트 초기화
		wstrText += ENTER_STRING;

		std::wstring	wstrCantAbil;
		GetCantAbilString(iCantAbil,wstrCantAbil);

		wstrText += wstrCantAbil;
		wstrText += ENTER_STRING;
		wstrText += TTW(40011);	//폰트 초기화
	}
}

float GetRarityAmplifyRate(EEquipType const eEquipType, int const iLevelLimit, int const iBasicAmpLv)
{
	SDefBasicOptionAmp const * pkOptionAmpInfo = PgItemRarityUpgradeFormula::GetBasicOptionAmp(eEquipType, iLevelLimit, iBasicAmpLv);
	if(pkOptionAmpInfo)
	{
		return static_cast<float>( pkOptionAmpInfo->iAmpRate ) / ABILITY_RATE_VALUE;
	}

	return 0.f;
}

void GetAbilObject(int const iType, int const iEquipPos, int const iTypeLevel, int const iGroup, int iIndex, CONT_ENCHANT_ABIL& kAbilOjbect, SBasicOptionAmpKey const& kAmpKey)
{
	SItemExtOptionKey kOptionKey( iType, iEquipPos, iTypeLevel, iGroup);
	MakeAbil_Enchant(kOptionKey, kAbilOjbect, iIndex, kAmpKey);
}

bool GetTimeString(int iTime, std::wstring &rkOutString, bool const bEndLine)
{
	if(0>=iTime)
	{
		return false;
	}

	BM::vstring kStr;
	bool bSpace = false;
	int iDay = iTime / 86400;
	if( 0 < iDay )			//일 단위 이상이면
	{
		kStr+=iDay;
		kStr+=TTW(174);//일
		iTime-=(iDay*86400);
		bSpace = true;
	}

	iDay = iTime / 3600;
	if( 0 < iDay )			//시간 단위 이상이면
	{
		if(bSpace)
		{
			kStr+=L" ";
		}
		kStr+=iDay;
		kStr+=TTW(90004);//시간
		iTime-=(iDay*3600);
		bSpace = true;
	}

	iDay = iTime / 60;
	if( 0 < iDay )			//분 단위 이상이면
	{
		if(bSpace)
		{
			kStr+=L" ";
		}
		kStr+=iDay;
		kStr+=TTW(90003);//분
		iTime-=(iDay*60);
		bSpace = true;
	}

	if ( 0 < iTime )
	{
		if(bSpace)
		{
			kStr+=L" ";
		}
		kStr+=iTime;
		kStr += TTW(90002);//초
	}

	rkOutString+=((std::wstring const&)kStr);
	if(bEndLine)
	{
		rkOutString+=L"\n";
	}

	return !rkOutString.empty();
}

void GetDefaultOption(PgBase_Item const &kItem, CONT_ENCHANT_ABIL & kEnchantAbil, bool const bRareType, SEnchantInfo const * pkEnchantInfo )
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( pDef )
	{
		SEnchantInfo const &kEnchantInfo = (pkEnchantInfo) ? *pkEnchantInfo : kItem.EnchantInfo();
		int const iEquipPos = pDef->EquipPos();
		int const iLevelLimit = pDef->GetAbil(AT_LEVELLIMIT);
		SBasicOptionAmpKey const kAmpKey(GetEquipType(pDef), iLevelLimit, kEnchantInfo.BasicAmpLv());

		GetAbilObject(static_cast<int>(kEnchantInfo.BasicType1()), iEquipPos, static_cast<int>(kEnchantInfo.BasicLv1()), 0, 0, kEnchantAbil, kAmpKey);
		GetAbilObject(static_cast<int>(kEnchantInfo.BasicType2()), iEquipPos, static_cast<int>(kEnchantInfo.BasicLv2()), 0, 1, kEnchantAbil, kAmpKey);
		GetAbilObject(static_cast<int>(kEnchantInfo.BasicType3()), iEquipPos, static_cast<int>(kEnchantInfo.BasicLv3()), 0, 2, kEnchantAbil, kAmpKey);
		GetAbilObject(static_cast<int>(kEnchantInfo.BasicType4()), iEquipPos, static_cast<int>(kEnchantInfo.BasicLv4()), 0, 3, kEnchantAbil, kAmpKey);
		
		if ( true == bRareType )
		{
			int const iOptionGroup = pDef->OptionGroup();

			GetAbilObject(static_cast<int>(kEnchantInfo.RareOptType1()), iEquipPos, static_cast<int>(kEnchantInfo.RareOptLv1()), iOptionGroup, 0, kEnchantAbil);
			GetAbilObject(static_cast<int>(kEnchantInfo.RareOptType2()), iEquipPos, static_cast<int>(kEnchantInfo.RareOptLv2()), iOptionGroup, 1, kEnchantAbil);
			GetAbilObject(static_cast<int>(kEnchantInfo.RareOptType3()), iEquipPos, static_cast<int>(kEnchantInfo.RareOptLv3()), iOptionGroup, 2, kEnchantAbil);
			GetAbilObject(static_cast<int>(kEnchantInfo.RareOptType4()), iEquipPos, static_cast<int>(kEnchantInfo.RareOptLv4()), iOptionGroup, 3, kEnchantAbil);
		}
	}
}

DWORD SetGradeColor(E_ITEM_GRADE const eItemLv, bool bIsCash, std::wstring& wstrName)
{
	std::wstring	wstrColor = L"0xFFFFFFFF";
	wchar_t	szTemp[MAX_PATH] = {0,};

	if( bIsCash )
	{
		wstrColor = TTW(4097);
	}
	else
	{
		wstrColor = TTW(4100+eItemLv);
	}

	swprintf(szTemp, MAX_PATH, TTW(4000).c_str(), wstrColor.c_str());
	wstrName += szTemp;
	DWORD Color = 0;
	sscanf(MB(wstrColor), "%x", &Color);
	return	Color;
}

DWORD GetDisplayGradeColor(E_ITEM_DISPLAY_GRADE const eDisplayLv, std::wstring& wstrName)
{
	std::wstring	wstrColor = L"0xFFFFFFFF";
	wchar_t	szTemp[MAX_PATH] = {0,};

	wstrColor = TTW(4203+eDisplayLv);
	swprintf(szTemp, MAX_PATH, TTW(4000).c_str(), wstrColor.c_str());
	wstrName += szTemp;
	DWORD Color = 0;
	sscanf(MB(wstrColor), "%x", &Color);
	return Color;
}

void GetCantAbilString(int const iCantAbil, std::wstring & wstrCantAbil)
{//다음 행동이 제한 됨
	if((iCantAbil & ICMET_Cant_ShopSell) == ICMET_Cant_ShopSell)
	{ 
		wstrCantAbil += TTW(2502); 
	}
	if((iCantAbil & ICMET_Cant_PlayerTrade) == ICMET_Cant_PlayerTrade)	
	{ // 유저거래가 막히면 길드금고 불가
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2503);
		wstrCantAbil += L"/";
		wstrCantAbil += TTW(401073);
	}
	if((iCantAbil & ICMET_Cant_MoveItemBox) == ICMET_Cant_MoveItemBox)
	{ 
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2504); 
	}
	if((iCantAbil & ICMET_Cant_SendMail) == ICMET_Cant_SendMail)
	{ 
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2505); 
	}
	if((iCantAbil & ICMET_Cant_DropItem) == ICMET_Cant_DropItem)
	{ 
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2506); 
	}
	if((iCantAbil & ICMET_Cant_Auction) == ICMET_Cant_Auction)
	{ 
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2507); 
	}
	if((iCantAbil & ICMET_Cant_UsePVP) == ICMET_Cant_UsePVP)
	{ 
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2508); 
	}
	if((iCantAbil & ICMET_Cant_Enchant) == ICMET_Cant_Enchant)
	{// 인챈트 금지
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2509); 
	}
	if((iCantAbil & ICMET_Cant_SoulCraft) == ICMET_Cant_SoulCraft)
	{// 소울 크래프트 금지
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2510); 
	}
	if((iCantAbil & ICMET_Cant_GenSocket) == ICMET_Cant_GenSocket)
	{// 소캣 생성 금지
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2511); 
	}
	if((iCantAbil & ICMET_Cant_SelfUse) == ICMET_Cant_SelfUse)
	{// 유저가 마우스 오른쪽 클릭으로 사용 못한다.
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2512); 
	}
	if((iCantAbil & ICMET_Cant_Seal) == ICMET_Cant_Seal)
	{// 봉인 불가
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2513); 
	}
	if((iCantAbil & ICMET_Cant_Rollback) == ICMET_Cant_Rollback)
	{// 소울 크래프트 행운권 사용 금지
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2514); 
	}
	if((iCantAbil & ICMET_Cant_Repair) == ICMET_Cant_Repair)
	{// 수리 불가
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2515); 
	}
	if((iCantAbil & ICMET_Cant_UseShareRental) == ICMET_Cant_UseShareRental)
	{// 계정금고 불가
		if(!wstrCantAbil.empty())	{ wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2920); 
	}
	if ((iCantAbil & ICMET_Cant_UseQuickSlot) == ICMET_Cant_UseQuickSlot)
	{// Can't place in quickslot
		if (!wstrCantAbil.empty()) { wstrCantAbil += L"/"; }
		wstrCantAbil += TTW(2516);
	}
}

//UI의 이름을 지정하거나, 지정하지 않고, 카테고리를 지정하면 그 카테고리로 불렀던 모든 ToolTip이 닫힌다.
void lwCloseToolTip(char const* wndName, int const iTargetCategoly)
{
	if( wndName )
	{
		int iCustomeInt = 0;
		XUI::CXUI_Wnd* pkToolTipWnd = XUIMgr.Get(UNI(wndName));
		if( !pkToolTipWnd )
		{
			return;
		}

		if( iTargetCategoly
		&&	!pkToolTipWnd->GetCustomData(&iCustomeInt, sizeof(iCustomeInt)) )
		{
			return;
		}

		if( iTargetCategoly
		&&	iTargetCategoly != iCustomeInt )//카테고리는 설정 하기 나름( EToolTipCategory 참조)
		{
			return;
		}
		
		if(pkToolTipWnd->ID() == UNI(szDefaultToolTipID))
		{
			pkToolTipWnd->Size(iTOOLTIP_X_SIZE_MAX, pkToolTipWnd->Size().y);
		}

		if(pkToolTipWnd->ID() == UNI(szToolTipPackageItemID))
		{
			pkToolTipWnd->Size(iTOOLTIP_X_SIZE_MAX, pkToolTipWnd->Size().y);
		}

		pkToolTipWnd->Close();
	}
	else
	{
		static char const* szToolTip[] = { szDefaultToolTipID, "ToolTip2_Extend", "ToolTip_Equip_Extend",
			"ToolTipItemExtend", "ToolTip_Equip", "ToolTip_Timer", "ToolTip_Skill", "ToolTip_SkillDebug",
			"ToolTip_EffectDebug", "ToolTip_MiniMap", "M_ToolTip", "TOOLTIP_SYSTEM_MENU", szMonsterCardToolTipID,
			szSuperGirlToolTipID, szToolTipPackageItemID, "TOOLTIP_FIX_POS", szSkillExtendToolTipID, "M_ToolTip2",
			"ToolTip_Premium",
			NULL };
		size_t iCur = 0;
		while( szToolTip[iCur] )
		{
			lwCloseToolTip(szToolTip[iCur], iTargetCategoly);
			++iCur;
		}
	}
}

void lwCallMutableToolTipByText(lwWString strText, lwPoint2& pt, int iIconNo, char const* wndName, bool bDrawTextCenter)
{
	if( !wndName )
	{
		wndName = szDefaultMutableToolTipID;
	}

	//step 1. 툴팁 윈도우
	XUI::CXUI_Wnd* pWnd = XUIMgr.Call(UNI(wndName), false);
	if( !pWnd )
	{
		return;
	}

	int iGap = 0;
	//step 2. 아이콘 있니?
	XUI::CXUI_Wnd* pIcon = pWnd->GetControl(UNI(szTTImgID));
	if( pIcon )
	{
		if( !iIconNo )
		{
			pIcon->Visible(false);
		}
		else
		{
			pIcon->Visible(true);
			SUVInfo	kUVInfo = pIcon->UVInfo();
			if(kUVInfo.Index != iIconNo)
			{
				kUVInfo.Index = iIconNo;
				pIcon->UVInfo(kUVInfo);
			}

			iGap = pIcon->Size().x;
		}
	}

	int const TT_OL_SIZE_HALF = 12;
	int const IMG_POS_GAP_HALF = 4;
	int const TT_OL_SIZE = TT_OL_SIZE_HALF * 2;
	int const IMG_POS_GAP = IMG_POS_GAP_HALF * 2;

	//step 3. 텍스트 사이즈
	pWnd->Size(iTOOLTIP_X_SIZE_MAX, pWnd->Size().y);
	pWnd->Text(strText());
	XUI::CXUI_Style_String	kStyleString = pWnd->StyleText();
	POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleString, (iTOOLTIP_X_SIZE_MAX - (iGap + TT_OL_SIZE)), true));

	//step 4. 텍스트 출력 위치
	POINT2 TextPos;
	TextPos.x = TT_OL_SIZE_HALF+iGap;
	if(bDrawTextCenter)
	{// 텍스트 가운데 정렬
		TextPos.x += kTextSize.x / 2;
		pWnd->FontFlag( pWnd->FontFlag() | XUI::XTF_ALIGN_CENTER );
	}
	else
	{
		pWnd->FontFlag( pWnd->FontFlag() & (~XUI::XTF_ALIGN_CENTER) );
	}

	if( kTextSize.y > iGap )
	{
		TextPos.y = TT_OL_SIZE_HALF;
	}
	else
	{
		TextPos.y = IMG_POS_GAP_HALF + (iGap - kTextSize.y) / 2;
	}

	pWnd->TextPos(TextPos);

	//step 5. 전체 사이즈 조정
	POINT2 ptNew = pt();
	ptNew.x += 2;
	ptNew.y += 2;
	pWnd->Size(TT_OL_SIZE + kTextSize.x + iGap, ((kTextSize.y > iGap)?(kTextSize.y + TT_OL_SIZE):(iGap + IMG_POS_GAP)));
	pWnd->Location(ptNew);
	pWnd->Invalidate(true);
}

void lwCallToolTipByText(int iIconNo, lwWString strText, lwPoint2 &pt, char const* wndName, int const iCategory, char const* szEx)
{
	int iTooltipIcon = 0; //일반 아이템 툴팁에서도 아이콘 리소스가 있다면 툴팁 아이콘을 표기
	if( !wndName )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(iIconNo);
		if(pDef)
		{
			iTooltipIcon = pDef->GetAbil(AT_CARD_BGNDNO);
		}

		if(iTooltipIcon > 0)
		{
			wndName = szMonsterCardToolTipID;
		}
		else
		{
			wndName = szDefaultToolTipID;
		}
	}

	XUI::CXUI_Wnd *pWnd = XUIMgr.Call(UNI(wndName), false);
	if( !pWnd )
	{
		return;
	}
	if(iTooltipIcon > 0)
	{
		MakeToopTipIcon_Common(pWnd, iTooltipIcon);
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 툴팁크기가 해상도 벗어나는지 체크
	std::wstring wStrText = strText();

	if(iTooltipIcon > 0)
	{
		wStrText.insert(0, L" \n\n\n");
		wStrText.insert(0, TTW(40002));
	}
	
	std::wstring::size_type Pos = wStrText.find(CUT_TOOLTIP);
	if(std::wstring::npos != Pos )
	{														// CUT_TOOLTIP의 위치를 찾아서
		std::wstring Text1(wStrText, 0, Pos);				// 처음부터 CUT_TOOLTIP의 바로 전까지의 내용을 Text1에 넣는다.
		Pos += CUT_TOOLTIP.size();							// CUT_TOOLTIP의 바로 다음 내용부터
		std::wstring Text2(wStrText, Pos);					// 끝까지 Text2에 넣는다.
		std::wstring Text3(L"");

		std::wstring::size_type Pos2 = Text2.find(CUT_TOOLTIP2);
		if(std::wstring::npos != Pos2 )
		{													// CUT_TOOLTIP2의 위치를 찾아서
			std::wstring Temp1(Text2, 0, Pos2);				// Text2의 처음부터 CUT_TOOLTIP2의 바로 전까지의 내용을 Text2에 넣는다.
			Pos2 += CUT_TOOLTIP2.size();					// CUT_TOOLTIP2의 바로 다음 내용부터
			std::wstring Temp2(Text2, Pos2);				// 끝까지 Text3에 넣는다.
			Text2 = Temp1;
			Text3 = Temp2;
		}

		wStrText = Text1 + Text2 + Text3;
		pWnd->Text(wStrText);								// CUT_TOOLTIP을 제외한 텍스트를 입력한다.

		if( pWnd->GetResolutionSize().y <= pWnd->Size().y )
		{ // 툴팁의 세로길이가 해상도 보다 크다면 내용을 지우고 확장 툴팁을 불러서 텍스트를 분할해서 넣는다.
			pWnd->Text(L"");
			pWnd->Text(Text1 + Text2);

			if( pWnd->GetResolutionSize().y <= pWnd->Size().y )
			{ // 나눴는데도 크다면 다시 나눈다.
				pWnd->Text(Text1);
				Text3 = Text2 + Text3;
			}
			std::wstring ExtendWndName = pWnd->ID() + L"_Extend";
			XUI::CXUI_Wnd *pExtendWnd = XUIMgr.Call(ExtendWndName);
			if( pExtendWnd )
			{ // 확장 툴팁 창을 불러와서 나머지 내용을 채운다.
				Text3 = TTW(40003) + Text3;
				pExtendWnd->Text(Text3);
				lwUIWnd(pExtendWnd).SetCustomDataAsStr(wndName); // 자신을 호출한 윈도우의 ID를 저장
			}
		}
	}
	else
	{ // 컷라인이 없으면 바로 텍스트 입력
		pWnd->Text(wStrText);
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지
	
	XUI::CXUI_Wnd *pAddWnd = pWnd->GetControl(UNI(szTTEABoxID));
	if(pAddWnd)
	{
		if(szEx != NULL && strlen(szEx) != 0)
		{
			pAddWnd->Visible(true);
			pAddWnd->Text(UNI(szEx));
		}
		else
		{
			pAddWnd->Visible(false);
		}
	}

	pWnd->Location(pt());
	pWnd->Invalidate(true);
	pWnd->SetCustomData(&iCategory, sizeof(iCategory));//카테고리 설정
	
}

void lwCallToolTipByStatus( WORD wAbilType, lwPoint2 &pt, char const* wndName, int const iCategory, char const* szEx )
{
	std::wstring wstrText;
	wstrText += TTW(4208);//타이틀 폰트
	switch( wAbilType )
	{
	case AT_MAX_HP:
		{
			wstrText += TTW(5052);//HP증가 상세정보
			wstrText += ENTER_STRING;
		}break;
	case AT_MAX_MP:
		{
			wstrText += TTW(5053);//MP증가 상세정보
			wstrText += ENTER_STRING;
		}break;
	default: {}break;
	}
	wstrText += ENTER_STRING;
	wstrText += TTW(40011);	//폰트 초기화

	CUnit* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	int const iBaseAbil = GetBasicAbil(wAbilType);
	int iBaseValue = pkPlayer->GetAbil(iBaseAbil);
	const TCHAR* pAbilTypeName = NULL;
	if( false == ::GetAbilName(iBaseAbil, pAbilTypeName) )
	{
		return;
	}

	BM::vstring vStrCharToolTip;
	vStrCharToolTip << ::TTW(40012) << L"[" << ::TTW(3010) << L"]\n";

	//캐릭터 총합
	vStrCharToolTip << ::TTW(1301) << ::TTW(3010) << L" " << ::TTW(5050);
	vStrCharToolTip << L": " << iBaseValue << L"\n";
	vStrCharToolTip << ::TTW(40011);

	vStrCharToolTip << ::TTW(5992);//기본
	if( pAbilTypeName )
	{
		vStrCharToolTip << L" " << pAbilTypeName;
	}
	vStrCharToolTip << L": " << iBaseValue << L"\n\n";

	//[아이템] 타이틀
	vStrCharToolTip << ::TTW(40012) << L"[" << ::TTW(3004) << L"]";
	vStrCharToolTip << ::TTW(40011);//폰트 초기화	

	wstrText += static_cast<std::wstring>(vStrCharToolTip);
	::MakeToolTipText_Status(wAbilType, wstrText);

	lwCallToolTipByText(0, wstrText, pt, wndName, iCategory, szEx);
}

void CallToolTip_ItemNo(int const iItemNo, lwPoint2 &rPT, bool const bForceSeal, bool bHideRarity, bool bOrtherActor, int const iTimeType, int const iTimeValue)
{
	XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
	if(pCursor)
	{
		if(!pCursor->IconInfo().iIconGroup && !pCursor->IconInfo().iIconKey)
		{
			std::wstring wstrText;
			std::wstring wstrLank;
			PgBase_Item kItem;

			if( S_OK != CreateSItem( iItemNo, 1, 0, kItem, bForceSeal) )
			{
				return;
			}

			if( 0 <= iTimeType
			&&	0 != iTimeValue )
			{
				kItem.SetUseTime( iTimeType, iTimeValue );
			}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( !pDef ){ return; }
			
			if( 0 == kItem.Count() )
			{
				kItem.Count(pDef->MaxAmount());
			}

			if( 0 != JobSkillToolUtil::GetToolType(kItem.ItemNo()) )
			{
				MakeToolTipText_JobSkill_Tool(kItem, wstrText, wstrLank, TBL_SHOP_IN_GAME::NullData());
				lwCallToolTipByText(iItemNo, wstrText, rPT, 0, 0, MB(wstrLank));
				return;
			}
			else if(JobSkill_Util::IsJobSkill_Item(kItem.ItemNo()) )
			{
				MakeToolTipText_JobSkill_Item(kItem, wstrText, wstrLank, TBL_SHOP_IN_GAME::NullData());
				lwCallToolTipByText(iItemNo, wstrText, rPT, 0, 0, MB(wstrLank));
				return;
			}
			else if( JobSkill_Util::IsJobSkill_SaveIndex(kItem.ItemNo()) )
			{
				MakeToolTipText_JobSkill_SaveIdx(kItem, wstrText, wstrLank, TBL_SHOP_IN_GAME::NullData());
				lwCallToolTipByText(iItemNo, wstrText, rPT, 0, 0, MB(wstrLank));
				return;
			}
			else if(SoulTransitionUtil::IsSoulTransitionItem(kItem.ItemNo()))
			{
				MakeToolTipText_SoulTransition(kItem, wstrText, wstrLank, TBL_SHOP_IN_GAME::NullData());
					return;
			}

			int const iCardType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
			bool MakeRet = false;
			switch( iCardType )
			{
			case UICT_MONSTERCARD:	
				{ 
					MakeToolTipText_MonsterCard(kItem, rPT); 
					return;		
				}break;
			case UICT_SKILL_EXTEND:
				{
					MakeToolTipText_SkillExtend(kItem, rPT); 
					return;
				}break;
			case UICT_SUPER_GIRL:	
				{ 
					MakeToolTipText_SuperGirl(kItem, rPT, TBL_SHOP_IN_GAME::NullData()); 
					return;	
				}break;
			case UICT_PET:			
				{ 
					MakeToolTipText_Pet(kItem, wstrText, wstrLank);		
				}break;
			default:				
				{ 
					MakeToolTipText(kItem, wstrText, TBL_SHOP_IN_GAME::NullData(), wstrLank, bHideRarity, bOrtherActor);	
				}break;
			}

			lwCallToolTipByText(iItemNo, wstrText, rPT, 0, 0, MB(wstrLank));
		}
	}
	return;
}

void CallToolTip_SItem(PgBase_Item const* pItem, lwPoint2 &pt, TBL_SHOP_IN_GAME const &kShopItemInfo, char const* pkWndName, const wchar_t* pkHeadStr, bool bOrtherActor, SToolTipFlag const &kFlag, bool const bShowExtend)
{//+1 낡은 스마트 단검 오브 임페리얼
	XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));

	if(pCursor && pItem && !PgBase_Item::IsEmpty(pItem))
	{
		if(	!pCursor->IconInfo().iIconGroup
		&&	!pCursor->IconInfo().iIconKey)
		{
			std::wstring wstrText;
			std::wstring wstrLank;

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const * pCardDef = kItemDefMgr.GetDef(pItem->ItemNo());
			if( pCardDef )
			{
				int const iCardType = pCardDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
				switch( iCardType )
				{
				case UICT_MONSTERCARD:				
					{
						if (pkHeadStr)
						{
							wstrText = pkHeadStr;
						}
						MakeToolTipText_MonsterCard(*pItem, pt, kFlag);
						return;
					}break;
				case UICT_SKILL_EXTEND:
					{
						if (pkHeadStr)
						{
							wstrText = pkHeadStr;
						}
						MakeToolTipText_SkillExtend(*pItem, pt, kFlag);
						return;
					}break;
				case UICT_SUPER_GIRL:
					{
						if (pkHeadStr)
						{
							wstrText = pkHeadStr;
						}
						MakeToolTipText_SuperGirl(*pItem, pt, kShopItemInfo);
						return;
					}break;
				case UICT_PET:
					{
						if (pkHeadStr)
						{
							wstrText = pkHeadStr;
						}
				
						MakeToolTipText_Pet(*pItem, wstrText, wstrLank);
						lwCallToolTipByText(pItem->ItemNo(), wstrText, pt, pkWndName, 0, MB(wstrLank));
						return;
					}
				default:
					{
					}break;
				}
			}
			if( 0 != JobSkillToolUtil::GetToolType(pItem->ItemNo()) )
			{
				MakeToolTipText_JobSkill_Tool(*pItem, wstrText, wstrLank, kShopItemInfo);
				lwCallToolTipByText(pItem->ItemNo(), wstrText, pt, pkWndName, 0, MB(wstrLank));
				return;
			}
			else if(JobSkill_Util::IsJobSkill_Item(pItem->ItemNo()) )
			{
				MakeToolTipText_JobSkill_Item(*pItem, wstrText, wstrLank, kShopItemInfo, kFlag);
				lwCallToolTipByText(pItem->ItemNo(), wstrText, pt, pkWndName, 0, MB(wstrLank));
				return;
			}
			else if( JobSkill_Util::IsJobSkill_SaveIndex(pItem->ItemNo()) )
			{
				MakeToolTipText_JobSkill_SaveIdx(*pItem, wstrText, wstrLank, kShopItemInfo, kFlag);
				lwCallToolTipByText(pItem->ItemNo(), wstrText, pt, pkWndName, 0, MB(wstrLank));
				return;
			}
			else if(SoulTransitionUtil::IsSoulTransitionItem(pItem->ItemNo()))
			{
				MakeToolTipText_SoulTransition(*pItem, wstrText, wstrLank, kShopItemInfo, kFlag);
				lwCallToolTipByText(pItem->ItemNo(), wstrText, pt, pkWndName, 0, MB(wstrLank));
				return;
			}

			if (pkHeadStr)
			{
				wstrText = pkHeadStr;
			}
			if(MakeToolTipText(*pItem, wstrText, kShopItemInfo, wstrLank, false, bOrtherActor, kFlag))
			{
				lwCallToolTipByText(pItem->ItemNo(), wstrText, pt, pkWndName, 0, MB(wstrLank));
			}

			if( !bShowExtend )
			{
				return;
			}

			wstrText.clear();
			if(MakeToolTip_ItemExtend(*pItem, wstrText, kShopItemInfo, wstrLank, false, bOrtherActor, kFlag))
			{
				XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
				XUI::CXUI_Wnd *pkTooltip = XUIMgr.Get(_T("ToolTip2"));
				if (pCursor && pkTooltip)
				{
					pt.SetX(pCursor->Location().x + pCursor->Size().x + pkTooltip->Size().x);
					pt.SetY(pCursor->Location().y + pCursor->Size().y);
				}

				lwCallMutableToolTipByText(wstrText, pt, pItem->ItemNo(), "ToolTipItemExtend", false);
			}
		}
	}
	return;
}

void ToolTipComp_SItem(PgBase_Item const &kItem, lwPoint2 kPoint, SToolTipFlag const &kFlag)
{
	int iPrice = 0;
	int iFitPos = 0;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if (pItemDef == NULL)
	{
		return;
	}

	if(false == lwGetUIWnd("FRM_SHOP").IsNil())
	{
		iPrice = pItemDef->SellPrice();
	}

	if (pItemDef)
	{
		iFitPos = pItemDef->EquipPos();
	}

	g_bIsEqComp = false;
	if( pItemDef->IsType(ITEM_TYPE_AIDS) )
	{
		CallToolTip_SItem(&kItem, kPoint, TBL_SHOP_IN_GAME::NullData(), 0, 0, false, kFlag, false);
	}
	else
	{
		g_bIsEqComp = false;

		PgBase_Item kEqupItem;
		CUnit* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if (pkPlayer) 
		{
			int const iGenderLimit = pItemDef->GetAbil(AT_GENDERLIMIT);
			if(iGenderLimit & pkPlayer->GetAbil(AT_GENDER))//성별 검사.
			{
				if ( IS_CLASS_LIMIT(pItemDef->GetAbil64(AT_CLASSLIMIT), pkPlayer->GetAbil(AT_CLASS)))
				{
					PgInventory* pkInv = pkPlayer->GetInven();
					if (pkInv)	
					{
						if(S_OK == pkInv->GetItem(SItemPos(KUIG_FIT, iFitPos), kEqupItem))
						{
							g_bIsEqComp = true;
						}
					}
				}
			}
		}

		if( 0 != JobSkillToolUtil::GetToolType(kEqupItem.ItemNo()) 
			|| 0 != JobSkillToolUtil::GetToolType(kItem.ItemNo()) )
		{//직업 도구의 경우, 비교 툴팁이 뜨지 않도록 수정
			g_bIsEqComp = false;
		}

		if( true==pItemDef->IsType(ITEM_TYPE_ENCHANT) )	//인첸트 아이템(소켓카드 일 경우)
		{
			g_bIsEqComp = false;
		}
		if(pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) == UICT_FORCED_ENCHANT)
		{ //인챈트 변경 아이템
			g_bIsEqComp = false;
		}

		CallToolTip_SItem(&kItem, kPoint, TBL_SHOP_IN_GAME::NullData(), 0, 0, false, kFlag, !g_bIsEqComp);

		if( g_bIsEqComp )
		{
			XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
			XUI::CXUI_Wnd *pkTooltip = XUIMgr.Get(_T("ToolTip2"));
			if (pCursor && pkTooltip)
			{
				kPoint.SetX(pCursor->Location().x + pCursor->Size().x + pkTooltip->Size().x);
				kPoint.SetY(pCursor->Location().y + pCursor->Size().y);
			}

			CallToolTip_SItem(&kEqupItem, kPoint, TBL_SHOP_IN_GAME::NullData(), "ToolTip_Equip", TTW(40060).c_str(), false, kFlag, false);
		}
		g_bIsEqComp = false;
	}
}

void CallSkillToolTip(PgSkillTree::stTreeNode *pTreeNode, lwPoint2 &pt)
{
	std::wstring wstrText1,wstrText2;
	std::wstring wstrType;
	if(MakeSkillToolTipText(pTreeNode, wstrText1, false, wstrType))
	{// 다음레벨. 안보여줌
		if(MakeSkillToolTipText(pTreeNode, wstrText2,true, wstrType))
		{
			if(wstrText2.size())
			{
				wstrText1+=_T("\n\n\n");
				wstrText1+=wstrText2;
			}
			lwCallToolTipByText(0, wstrText1, pt, "ToolTip_Skill", 0, MB(wstrType));
		}
	}
	return;
}

void CallSkillToolTip(int iSkillNo, lwPoint2 &pt)
{
	std::wstring wstrText1;
	std::wstring wstrType;
	if(MakeSkillToolTipText(iSkillNo, wstrText1,wstrType))
	{// 해당 스킬만 보여줌
		lwCallToolTipByText(0, wstrText1, pt, "ToolTip_Skill", 0, MB(wstrType));
	}
}

void CallComboSkillToolTip(int iSkillNo, lwPoint2 &pt)
{
	std::wstring wstrText1;
	std::wstring wstrType;
	if(MakeComboSkillToolTip(iSkillNo, wstrText1,wstrType))
	{// 해당 스킬만 보여줌
		lwCallToolTipByText(0, wstrText1, pt, "ToolTip_Skill", 0, MB(wstrType));
	}
}

void CallSkillToolTip_Pet(int const iSkillNo, lwPoint2 &pt)
{
	std::wstring wstrType, wstrText1;
	if( MakeSkillToolTipText_Pet(iSkillNo, wstrText1, wstrType) )
	{
		lwCallToolTipByText(0, wstrText1, pt, "ToolTip_Skill", 0, MB(wstrType));
	}
}

void CallSkillAbilToolTip(int iSkillNo, lwPoint2 &pt)
{
	std::wstring wstrText;
	wstrText += UNI("SkillNo : ");
	TCHAR szNo[32] = {0,};
	_itow_s(iSkillNo, szNo, 32, 10);
	wstrText += szNo;
	wstrText += UNI("\n");

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	const CSkillDef	*pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	MakeSkillDefaultAbilToString(pkSkillDef, wstrText);
	MakeAbilObjectToString(pkSkillDef, wstrText);

	lwCallToolTipByText(0, wstrText, pt, "ToolTip_SkillDebug");

	if(pkSkillDef)
	{
		std::wstring wstrText2;
		int iEffectNo = pkSkillDef->GetEffectNo();
		wstrText2 += UNI("EffectID :");
		_itow_s(iEffectNo, szNo, 32, 10);
		wstrText2 += szNo;
		wstrText2 += UNI("\n");

		GET_DEF(CEffectDefMgr, kEffectDefMgr);
		const CEffectDef *pkEffectDef = kEffectDefMgr.GetDef(iEffectNo);
		MakeEffectDefaultAbilToString(pkEffectDef,wstrText2);
		MakeAbilObjectToString(pkEffectDef, wstrText2);

		if(0 != iEffectNo)
		{
			lwCallToolTipByText(0, wstrText2, pt, "ToolTip_EffectDebug");
		}
	}
}

bool MakeAbilNameString(WORD const Type, std::wstring &wstrText)
{
	const TCHAR* pText = NULL;
	switch(Type)
	{
	case AT_PHY_ATTACK_MIN:		{ wstrText += TTW(5003); }break;
	case AT_MAGIC_ATTACK_MIN:	{ wstrText += TTW(5004); }break;
	case AT_HP_POTION_ADD_RATE:	{ wstrText += TTW(5203); }return true;
	default:
		{
			if(GetAbilName(Type, pText))
			{
				wstrText += pText;
				return true;
			}
		}break;
	}

	return false;
}

bool MakeAbilNameString2(WORD const Type, std::wstring &wstrText)
{
	const TCHAR* pText = NULL;
	if(GetAbilName(Type, pText))
	{
		wstrText += pText;
		return true;
	}
	return false;
}

bool CheckAddLineAbilType(WORD const wAbilType)
{
	if(( AT_CALCUATEABIL_MIN <= wAbilType && AT_CALCUATEABIL_MAX >= wAbilType )
	|| ( AT_MONSTER_CARD_ABIL_MIN <= wAbilType && AT_MONSTER_CARD_ABIL_MAX >= wAbilType ))
	{
		return true;
	}
	else
	{
		switch( wAbilType )
		{
		case AT_R_CRITICAL_POWER:
			{
				return false;
			}break;//해당 어빌은 사용하지 않는다.
		case AT_HIT_DAMAGE_HP_RATE_ACTIVATE:
		case AT_HIT_DECREASE_MP_RATE_ACTIVATE:
		case AT_CRITICAL_DAMAGE_RATE_ACTIVATE:
		case AT_CRITICAL_DAMAGE_ACTIVATE:

		case AT_INVEN_PET_ADD_TIME_EXP_RATE:
		case AT_INVEN_PET_ADD_HUNT_EXP_RATE:
		case AT_GIVE_TO_CALLER_ADD_EXP_RATE:

		case AT_MAX_HP:
		case AT_R_MAX_HP:
		case AT_MAX_MP:
		case AT_R_MAX_MP:
		case AT_C_MAX_MP:
		case AT_STR:
		case AT_INT:
		case AT_CON:
		case AT_DEX:
		case AT_MOVESPEED:
		case AT_R_MOVESPEED:
		case AT_PHY_DEFENCE:
		case AT_R_PHY_DEFENCE:
		case AT_MAGIC_DEFENCE:
		case AT_R_MAGIC_DEFENCE:
		case AT_ATTACK_SPEED:
		case AT_R_ATTACK_SPEED:
		case AT_BLOCK_SUCCESS_VALUE:
		case AT_R_BLOCK_SUCCESS_VALUE:
		case AT_DODGE_SUCCESS_VALUE:
		case AT_R_DODGE_SUCCESS_VALUE:
		case AT_CRITICAL_SUCCESS_VALUE:
		case AT_R_CRITICAL_SUCCESS_VALUE:
		case AT_CRITICAL_POWER:
		case AT_PHY_ATTACK_MAX:
		case AT_R_PHY_ATTACK_MAX:
		case AT_PHY_ATTACK_MIN:
		case AT_R_PHY_ATTACK_MIN:
		case AT_MAGIC_ATTACK_MAX:
		case AT_R_MAGIC_ATTACK_MAX:
		case AT_MAGIC_ATTACK_MIN:
		case AT_R_MAGIC_ATTACK_MIN:
		case AT_DMG_REFLECT_RATE_FROM_ITEM01:	// 아이템으로 인한 데미지 반사 관련
		case AT_DMG_REFLECT_RATE_FROM_ITEM02:
		case AT_DMG_REFLECT_RATE_FROM_ITEM03:
		case AT_DMG_REFLECT_RATE_FROM_ITEM04:
		case AT_DMG_REFLECT_RATE_FROM_ITEM05:
		case AT_DMG_REFLECT_RATE_FROM_ITEM06:
		case AT_DMG_REFLECT_RATE_FROM_ITEM07:
		case AT_DMG_REFLECT_RATE_FROM_ITEM08:
		case AT_DMG_REFLECT_RATE_FROM_ITEM09:
		case AT_DMG_REFLECT_RATE_FROM_ITEM10:
		case AT_JS_1ST_ADD_SKILL_TURNTIME:
		case AT_JS_1ST_ADD_USE_EXHAUSTION:
		case AT_JS_1ST_ADD_TOOL_USE_DURATION_RATE:
		case AT_JS_1ST_ADD_RESULT_ITEM_COUNT:
		case AT_JS_1ST_ADD_EXPERTNESS_RATE:
		case AT_JS_1ST_SUB_ADD_SKILL_TURNTIME:
		case AT_JS_1ST_SUB_ADD_USE_EXHAUSTION:
		case AT_JS_1ST_SUB_ADD_TOOL_USE_DURATION_RATE:
		case AT_JS_1ST_SUB_ADD_RESULT_ITEM_COUNT:
		case AT_JS_1ST_SUB_ADD_EXPERTNESS_RATE:
			{
				return true;
			}
		}
	}
	return false;
}

// return S_OK이면 이 안에서 전체 문장을 만든경우
// return S_FALSE이면 외부에서 추가로 문장을 완성해주어야 함 
HRESULT MakeAbilValueString(PgBase_Item const &kItem, CAbilObject const* pDef, WORD const Type, int iValue, std::wstring &wstrText, bool const EqComp)
{
	if(iValue == 0) return E_FAIL; //어빌값 0은 출력하지 않고 패스

	PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
	if(pkPilot == NULL)
	{
		return E_FAIL;
	}

	//HP/MP 회복속도만 '-' -> '+'로 보여지도록
	if(Type == AT_R_HP_RECOVERY_INTERVAL || Type == AT_R_MP_RECOVERY_INTERVAL)
	{
		iValue = (-iValue);
	}
	
	wstrText+=TTW(1548); // 값 색깔(녹색)
	bool bIgnoreSignCheck = false;
	switch(Type)
	{
	case AT_R_CRITICAL_POWER:
		{
			return E_FAIL;
		}break;//해당 어빌은 사용하지 않는다.
	case AT_PHY_DMG_100PERECNT_REFLECT_RATE://= 3914,	// 물리 Damage 100% 반사 확률( 만분률 )// 여기에 값이 데미지 반사량(%)
	case AT_R_MAX_HP:
	case AT_R_HP_RECOVERY_INTERVAL:
	case AT_R_MAX_MP:
	case AT_R_MP_RECOVERY_INTERVAL:
	case AT_R_STR:
	case AT_R_INT:
	case AT_R_CON:
	case AT_R_DEX:
	case AT_R_MOVESPEED:
	case AT_R_PHY_DEFENCE:
	case AT_R_MAGIC_DEFENCE:
	case AT_R_ATTACK_SPEED:
	case AT_R_BLOCK_SUCCESS_VALUE:
	case AT_R_DODGE_SUCCESS_VALUE:
	case AT_R_CRITICAL_SUCCESS_VALUE:
	case AT_R_INVEN_SIZE:
	case AT_R_EQUIPS_SIZE:
	case AT_R_ATTACK_RANGE:
	case AT_R_HP_RECOVERY:
	case AT_R_MP_RECOVERY:
	case AT_R_JUMP_HEIGHT:
	case AT_R_PHY_ATTACK_MAX:
	case AT_R_PHY_ATTACK_MIN:
	case AT_R_NEED_MP:
	case AT_R_NEED_HP:
	case AT_R_MAGIC_ATTACK:
	case AT_R_PHY_ATTACK:
	case AT_R_CRITICAL_MPOWER:
	case AT_R_HIT_SUCCESS_VALUE:
	case AT_R_WALK_SPEED:
	case AT_R_MAGIC_ATTACK_MAX:
	case AT_R_MAGIC_ATTACK_MIN:
	case AT_R_PHY_DMG_DEC:
	case AT_R_MAGIC_DMG_DEC:
	case AT_R_CASTING_SPEED:
	case AT_R_RESIST_FIRE:
	case AT_R_RESIST_WATER:
	case AT_R_RESIST_TREE:
	case AT_R_RESIST_IRON:
	case AT_R_RESIST_EARTH:
	case AT_R_HP_POTION_ADD_RATE:
	case AT_R_MP_POTION_ADD_RATE:
	case AT_R_DMG_REFLECT_RATE:
	case AT_R_DMG_REFLECT_HITRATE:
	case AT_R_PENETRATION_RATE:
	case AT_R_MAX_HP_DEAD:
	case AT_R_MAX_MP_DEAD:
	case AT_R_ABS_ADDED_DMG_PHY:
	case AT_R_ABS_ADDED_DMG_MAGIC:
	case AT_PHY_DMG_DEC:
	case AT_MAGIC_DMG_DEC:
	case AT_ADD_R_NEED_HP:
	case AT_ADD_R_NEED_MP:
	case AT_I_PHY_DEFENCE_ADD_RATE:
	case AT_I_MAGIC_DEFENCE_ADD_RATE:
	case AT_R_ATTACK_ADD_FIRE:
	case AT_R_ATTACK_ADD_ICE:
	case AT_R_ATTACK_ADD_NATURE:
	case AT_R_ATTACK_ADD_CURSE:
	case AT_R_ATTACK_ADD_DESTROY:
	case AT_R_ATTACK_ADD_FIRE_PHY:
	case AT_R_ATTACK_ADD_ICE_PHY:
	case AT_R_ATTACK_ADD_NATURE_PHY:
	case AT_R_ATTACK_ADD_CURSE_PHY:
	case AT_R_ATTACK_ADD_DESTROY_PHY:
	case AT_R_ATTACK_ADD_FIRE_MAGIC:
	case AT_R_ATTACK_ADD_ICE_MAGIC:
	case AT_R_ATTACK_ADD_NATURE_MAGIC:
	case AT_R_ATTACK_ADD_CURSE_MAGIC:
	case AT_R_ATTACK_ADD_DESTROY_MAGIC:
	case AT_R_RESIST_ADD_FIRE:
	case AT_R_RESIST_ADD_ICE:
	case AT_R_RESIST_ADD_NATURE:
	case AT_R_RESIST_ADD_CURSE:
	case AT_R_RESIST_ADD_DESTROY:
	case AT_R_RESIST_ADD_FIRE_PHY:
	case AT_R_RESIST_ADD_ICE_PHY:
	case AT_R_RESIST_ADD_NATURE_PHY:
	case AT_R_RESIST_ADD_CURSE_PHY:
	case AT_R_RESIST_ADD_DESTROY_PHY:
	case AT_R_RESIST_ADD_FIRE_MAGIC:
	case AT_R_RESIST_ADD_ICE_MAGIC:
	case AT_R_RESIST_ADD_NATURE_MAGIC:
	case AT_R_RESIST_ADD_CURSE_MAGIC:
	case AT_R_RESIST_ADD_DESTROY_MAGIC:
	case AT_R_ATTACK_ADD_4ELEMENT:    
	case AT_R_ATTACK_ADD_5ELEMENT:     
	case AT_R_ATTACK_ADD_4ELEMENT_PHY:
	case AT_R_ATTACK_ADD_5ELEMENT_PHY:
	case AT_R_ATTACK_ADD_4ELEMENT_MAGIC:
	case AT_R_ATTACK_ADD_5ELEMENT_MAGIC:
	case AT_R_RESIST_ADD_4ELEMENT:
	case AT_R_RESIST_ADD_5ELEMENT:
	case AT_R_RESIST_ADD_4ELEMENT_PHY:
	case AT_R_RESIST_ADD_5ELEMENT_PHY:
	case AT_R_RESIST_ADD_4ELEMENT_MAGIC:
	case AT_R_RESIST_ADD_5ELEMENT_MAGIC:
	case AT_HP_POTION_ADD_RATE:
	case AT_CRITICAL_POWER:
	case AT_BLOCK_DAMAGE_RATE:				// 블록을 한후에 받을 데미지를 감소시킬 비율
	case AT_FINAL_HIT_SUCCESS_RATE:
	case AT_FINAL_DODGE_SUCCESS_RATE:
	case AT_FINAL_BLOCK_SUCCESS_RATE:
	case AT_FINAL_CRITICAL_SUCCESS_RATE:
	// 	case AT_R_MAX_DP:
// 	case AT_R_DP_RECOVERY_INTERVAL:
// 	case AT_R_DP_RECOVERY:
		{// % 단위가 붙는 항목들, %단위에 맞게 맞춘다.
			float const fRate = iValue / (ABILITY_RATE_VALUE * 0.01f);
			BM::vstring vStr(fRate, L"%.1f");
			vStr+="%";
			wstrText += vStr.operator const std::wstring &();
		}break;
	case AT_ADD_REPAIR_COST_RATE:
		{
			// -를 꺼꾸로 해야함
			iValue = -iValue;
		}// break을 사용하지 않음
	case AT_ADD_ENCHANT_RATE:
		{
			float const fRate = iValue * 0.001f;
			wstrText += lwMarketUtil::FloatConvertToStr(fRate, 2);
			wstrText += L"%";
		}break;
	case AT_ADD_SOULCRAFT_RATE:
	case AT_ADD_SOUL_RATE_REFUND:
	case AT_INVEN_PET_ADD_TIME_EXP_RATE:
	case AT_INVEN_PET_ADD_HUNT_EXP_RATE:
	case AT_GIVE_TO_CALLER_ADD_EXP_RATE:
		{// % 단위가 붙는 항목들, %단위에 맞게 맞춘다.
			float const fRate = iValue * 0.01f;
			wstrText += lwMarketUtil::FloatConvertToStr(fRate, 2);
			wstrText += L"%";
		}break;
	case AT_HIT_DAMAGE_HP_RATE_ACTIVATE:
	case AT_HIT_DECREASE_MP_RATE_ACTIVATE:
	case AT_CRITICAL_DAMAGE_RATE_ACTIVATE:
		{
			if( !MakeAbilNameString( Type, wstrText ) )
			{
				return E_FAIL;
			}

			BM::vstring vstrText(wstrText); 

			float fPer = static_cast<float>(iValue) / 100.0f;
			vstrText.Replace( L"{PER}", BM::vstring( fPer, L"%.1f") );

			fPer = static_cast<float>(pDef->GetAbil(Type+1)) / 100.0f;
			vstrText.Replace( L"{VALUE}", BM::vstring( fPer, L"%.1f" ) << L"%" );

			wstrText = static_cast<std::wstring>(vstrText);
			bIgnoreSignCheck = true;
		}break;
	case AT_CRITICAL_DAMAGE_ACTIVATE:
		{
			if( !MakeAbilNameString( Type, wstrText ) )
			{
				return E_FAIL;
			}

			BM::vstring vstrText(wstrText); 
			float const fPer = static_cast<float>(iValue) / 100.0f;
			vstrText.Replace( L"{PER}", BM::vstring( fPer, L"%.1f") );
			vstrText.Replace( L"{VALUE}", pDef->GetAbil(Type+1) );

			wstrText = static_cast<std::wstring>(vstrText);
			bIgnoreSignCheck = true;
		}break;
	case AT_DMG_REFLECT_RATE_FROM_ITEM01:
	case AT_DMG_REFLECT_RATE_FROM_ITEM02:
	case AT_DMG_REFLECT_RATE_FROM_ITEM03:
	case AT_DMG_REFLECT_RATE_FROM_ITEM04:
	case AT_DMG_REFLECT_RATE_FROM_ITEM05:
	case AT_DMG_REFLECT_RATE_FROM_ITEM06:
	case AT_DMG_REFLECT_RATE_FROM_ITEM07:
	case AT_DMG_REFLECT_RATE_FROM_ITEM08:
	case AT_DMG_REFLECT_RATE_FROM_ITEM09:
	case AT_DMG_REFLECT_RATE_FROM_ITEM10:
		{// Damage 반사할 때, 반사가 일어날 확률
			int const iDmgAbilType = Type+10;
			wstrText = TTW(790800); //L"피격시 {PER}%의 확률로 ";
			int const iReflectRate = 0; // Damage 반사할 때, 반사되는 양(Damage에 대한 만분율값) 아이템으로 인한 반사확률은 스킬과 별도로 만들어져야함(현재 없음)
			int const iReflectValue = pDef->GetAbil(iDmgAbilType);	// Damage 반사할때 , 반사되는 데미지 절대값 (100이면 100데미지줌)
			if(iReflectRate && !iReflectValue)
			{// 반사 확률만 있을 경우
				wstrText +=TTW(790801);//L"데미지의 {PER1}%반사";
			}
			else if(!iReflectRate && iReflectValue)
			{// 반사 데미지값만 있을경우
				wstrText +=TTW(790802);//L"데미지 {VALUE}반사";
			}
			else
			{// 양쪽 다 있는 경우 이거나, 둘다 없는 경우
				wstrText +=TTW(790803);//L"데미지의 {PER1}%와 데미지 {VALUE}반사";
			}
			BM::vstring vstrText(wstrText); 
			float const fPer = static_cast<float>(iValue) / 100.0f;
			vstrText.Replace( L"#PER#", BM::vstring( fPer, L"%.1f") );

			float const fReflectRate = static_cast<float>(iReflectRate)/100.0f;
			vstrText.Replace( L"#PER1#", BM::vstring( fReflectRate, L"%.1f"));

			vstrText.Replace( L"#VALUE#", iReflectValue );

			wstrText = static_cast<std::wstring>(vstrText);
			bIgnoreSignCheck = true;
		}break;
	case AT_HP_RESTORE_WHEN_HIT_RATE:
		{//타격시 HP 회복시켜줄 확률:
			if( !MakeAbilNameString( Type, wstrText ) )
			{
				return E_FAIL;
			}
			//wstrText = L"타격시 {PER}%의 확률로 {VALUE}HP 회복";
			BM::vstring vstrText(wstrText); 
			float const fPer = static_cast<float>(iValue) / 100.0f;
			vstrText.Replace( L"{PER}", BM::vstring( fPer, L"%.1f") );
			vstrText.Replace( L"{VALUE}", pDef->GetAbil(AT_HP_RESTORE_WHEN_HIT) );//타격시 HP 회복 시켜주는 값(절대값)

			wstrText = static_cast<std::wstring>(vstrText);
			bIgnoreSignCheck = true;
		}break;

	case AT_MP_RESTORE_WHEN_HIT_RATE:
		{//타격시 MP 회복시켜줄 확률
			if( !MakeAbilNameString( Type, wstrText ) )
			{
				return E_FAIL;
			}
			//wstrText = L"타격시 {PER}%의 확률로 {VALUE}MP 회복";
			BM::vstring vstrText(wstrText); 
			float const fPer = static_cast<float>(iValue) / 100.0f;
			vstrText.Replace( L"{PER}", BM::vstring( fPer, L"%.1f") );
			vstrText.Replace( L"{VALUE}", pDef->GetAbil(AT_MP_RESTORE_WHEN_HIT) );//타격시 MP 회복 시켜주는 값(절대값)

			wstrText = static_cast<std::wstring>(vstrText);
			bIgnoreSignCheck = true;
		}break;
	case AT_IMMUNITY_FREEZE_RATE:
		{//동빙 무시 확률
			if( !MakeAbilNameString( Type, wstrText ) )
			{
				return E_FAIL;
			}
			//wstrText = L"{PER}%확률로 동빙 무시";
			BM::vstring vstrText(wstrText); 
			float const fPer = static_cast<float>(iValue) / 100.0f;
			vstrText.Replace( L"{PER}", BM::vstring( fPer, L"%.1f") );

			wstrText = static_cast<std::wstring>(vstrText);
			bIgnoreSignCheck = true;
		}break;
	case AT_IMMUNITY_COLD_RATE:
		{//결빙 무시 확률
			if( !MakeAbilNameString( Type, wstrText ) )
			{
				return E_FAIL;
			}
			//wstrText = L"{PER}%확률로 결빙 무시";
			BM::vstring vstrText(wstrText); 
			float const fPer = static_cast<float>(iValue) / 100.0f;
			vstrText.Replace( L"{PER}", BM::vstring( fPer, L"%.1f") );
			
			wstrText = static_cast<std::wstring>(vstrText);
			bIgnoreSignCheck = true;
		}break;
	case AT_JS_1ST_ADD_SKILL_TURNTIME:
		{
			if( 0 != iValue )
			{
				int const iVal = iValue / 1000;
				if( 0 < iValue )
				{
					// 1/1000 초 시간 증가
					BM::vstring kTemp( TTW(799648) );	kTemp.Replace( L"$VALUE$", BM::vstring(iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				else
				{
					// 1/1000 초 시간 감소
					BM::vstring kTemp( TTW(799649) );	kTemp.Replace( L"$VALUE$", BM::vstring(-iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				bIgnoreSignCheck = true;
			}
		}break;
	case AT_JS_1ST_ADD_USE_EXHAUSTION:
		{
			if( 0 != iValue )
			{
				if( 0 < iValue )
				{
					// 절대값 소모 피로도 증가
					BM::vstring kTemp( TTW(799650) );	kTemp.Replace( L"$VALUE$", BM::vstring(iValue) );		wstrText = kTemp.operator std::wstring const&();
				}
				else
				{
					// 절대값 소모 피로도 감소
					BM::vstring kTemp( TTW(799651) );	kTemp.Replace( L"$VALUE$", BM::vstring(-iValue) );		wstrText = kTemp.operator std::wstring const&();
				}
				bIgnoreSignCheck = true;
			}
		}break;
	case AT_JS_1ST_ADD_TOOL_USE_DURATION_RATE:
		{
			if( 0 != iValue )
			{
				int iVal = iValue / 100;
				if( 0 < iValue )
				{
					// 10000분률 아이템 내구도 감소 확률 감소
					BM::vstring kTemp( TTW(799653) );	kTemp.Replace( L"$VALUE$", BM::vstring(iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				else
				{
					// 10000분률 아이템 내구도 감소 확률 증가
					BM::vstring kTemp( TTW(799652) );	kTemp.Replace( L"$VALUE$", BM::vstring(-iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				bIgnoreSignCheck = true;
			}
		}break;
	case AT_JS_1ST_ADD_RESULT_ITEM_COUNT:
		{
			if( 0 != iValue )
			{
				if( 0 < iValue )
				{
					// 절대값 획득 아이템 수량 증가
					BM::vstring kTemp( TTW(799654) );	kTemp.Replace( L"$VALUE$", BM::vstring(iValue) );		wstrText = kTemp.operator std::wstring const&();
					bIgnoreSignCheck = true;
				}
			}
		}break;
	case AT_JS_1ST_ADD_EXPERTNESS_RATE:
		{
			if( 0 != iValue )
			{
				int iVal = iValue / 100;
				if( 0 < iValue )
				{
					// 10000분률 습득 숙련도 상승
					BM::vstring kTemp( TTW(799655) );	kTemp.Replace( L"$VALUE$", BM::vstring(iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				else
				{
					// 10000분률 습득 숙련도 감소
					BM::vstring kTemp( TTW(799656) );	kTemp.Replace( L"$VALUE$", BM::vstring(-iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				bIgnoreSignCheck = true;
			}
		}break;
		case AT_JS_1ST_SUB_ADD_SKILL_TURNTIME:
		{
			if( 0 != iValue )
			{
				int const iVal = iValue / 1000;
				if( 0 < iValue )
				{
					// 1/1000 초 시간 증가
					BM::vstring kTemp( TTW(799688) );	kTemp.Replace( L"$VALUE$", BM::vstring(iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				else
				{
					// 1/1000 초 시간 감소
					BM::vstring kTemp( TTW(799689) );	kTemp.Replace( L"$VALUE$", BM::vstring(-iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				bIgnoreSignCheck = true;
			}
		}break;
	case AT_JS_1ST_SUB_ADD_USE_EXHAUSTION:
		{
			if( 0 != iValue )
			{
				if( 0 < iValue )
				{
					// 절대값 소모 피로도 증가
					BM::vstring kTemp( TTW(799690) );	kTemp.Replace( L"$VALUE$", BM::vstring(iValue) );		wstrText = kTemp.operator std::wstring const&();
				}
				else
				{
					// 절대값 소모 피로도 감소
					BM::vstring kTemp( TTW(799691) );	kTemp.Replace( L"$VALUE$", BM::vstring(-iValue) );		wstrText = kTemp.operator std::wstring const&();
				}
				bIgnoreSignCheck = true;
			}
		}break;
	case AT_JS_1ST_SUB_ADD_TOOL_USE_DURATION_RATE:
		{
			if( 0 != iValue )
			{
				int iVal = iValue / 100;
				if( 0 < iValue )
				{
					// 10000분률 아이템 내구도 감소 확률 감소
					BM::vstring kTemp( TTW(799693) );	kTemp.Replace( L"$VALUE$", BM::vstring(iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				else
				{
					// 10000분률 아이템 내구도 감소 확률 증가
					BM::vstring kTemp( TTW(799692) );	kTemp.Replace( L"$VALUE$", BM::vstring(-iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				bIgnoreSignCheck = true;
			}
		}break;
	case AT_JS_1ST_SUB_ADD_RESULT_ITEM_COUNT:
		{
			if( 0 != iValue )
			{
				if( 0 < iValue )
				{
					// 절대값 획득 아이템 수량 증가
					BM::vstring kTemp( TTW(799694) );	kTemp.Replace( L"$VALUE$", BM::vstring(iValue) );		wstrText = kTemp.operator std::wstring const&();
					bIgnoreSignCheck = true;
				}
			}
		}break;
	case AT_JS_1ST_SUB_ADD_EXPERTNESS_RATE:
		{
			if( 0 != iValue )
			{
				int iVal = iValue / 100;
				if( 0 < iValue )
				{
					// 10000분률 습득 숙련도 상승
					BM::vstring kTemp( TTW(799695) );	kTemp.Replace( L"$VALUE$", BM::vstring(iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				else
				{
					// 10000분률 습득 숙련도 감소
					BM::vstring kTemp( TTW(799696) );	kTemp.Replace( L"$VALUE$", BM::vstring(-iVal) );		wstrText = kTemp.operator std::wstring const&();
				}
				bIgnoreSignCheck = true;
			}
		}break;


	default:
//	case AT_HIT_SUCCESS_VALUE:
//	case AT_DODGE_SUCCESS_VALUE:
//	case AT_BLOCK_SUCCESS_VALUE:
//	case AT_CRITICAL_SUCCESS_VALUE:
		{
			wstrText += (std::wstring)BM::vstring(iValue);
		}break;
	}
	wstrText+=TTW(1554); // 값 색깔(흰색)
	if(bIgnoreSignCheck)
	{		
		return S_OK;
	}
	return S_FALSE;
}

bool MakeMonsterCardTimeLimit(PgBase_Item const &kItem, std::wstring & wstrText)
{
	SMonsterCardTimeLimit kTimeLimit;
	if(false == kItem.Get(kTimeLimit))
	{
		return false;
	}

	// 기간제 옵션이면 기간 만료가 아닐때 출력
	wstrText += std::wstring(L" (");
	wstrText += TTW(5024);
	wstrText += std::wstring(L":");

	__int64 const i64UseTime = kTimeLimit.GetUseAbleTime() + 3599;// 1시간 올림
	__int64 const i64Days = i64UseTime/86400;
	if(0 < i64Days)
	{
		wstrText += static_cast<std::wstring>(BM::vstring(i64Days));
		wstrText += TTW(5025);
	}
	else
	{
		__int64 const i64Hours = (i64UseTime%86400)/3600;
		if(0 < i64Hours)
		{
			wstrText += static_cast<std::wstring>(BM::vstring(i64Hours));
			wstrText += TTW(5026);
		}
		else
		{
			wstrText += TTW(89999);//빨강
			wstrText += TTW(5027);
			wstrText += TTW(40011);
		}
	}
	wstrText += std::wstring(L")");
	return true;
}

void MakeItemEffectAbilString(CItemDef const* pkItemDef, int const iIdx,int const iEffectMin,int const iRateMin, int const iTargetMin, std::wstring & wstrText)
{
	int const iEffectNo = pkItemDef->GetAbil(iEffectMin+iIdx);

	if(0 == iEffectNo)
	{
		return;
	}

	int const iRate = pkItemDef->GetAbil(iRateMin+iIdx);
	int const iTarget = pkItemDef->GetAbil(iTargetMin+iIdx);

	CONT_DEFEFFECT const *pkDefEffect = NULL;
	g_kTblDataMgr.GetContDef(pkDefEffect);
	if( NULL == pkDefEffect )
	{
		return;
	}

	CONT_DEFEFFECT::const_iterator iter = pkDefEffect->find(iEffectNo);
	if( pkDefEffect->end() == iter )
	{
		return;
	}

	CONT_DEFEFFECT::mapped_type const & kEffect = (*iter).second;

	const wchar_t *pName = NULL;
	if(false == GetDefString(kEffect.iName, pName))
	{
		return;
	}

	wstrText += pName;
	wstrText += _T("(");
	GetTimeString(kEffect.iDurationTime/1000,wstrText,false);
	wstrText += _T(")");
	wstrText += _T(" : ");

	double const dlRealRate = static_cast<double>(iRate) / static_cast<double>(ABILITY_RATE_VALUE) * 100.0;
	wstrText += static_cast<std::wstring>(BM::vstring(dlRealRate,_T("%0.2f")));
	wstrText += _T("% : ");

	if(0 < iTarget)
	{
		wstrText += TTW(5028);
	}
	else
	{
		wstrText += TTW(5029);
	}
}

void MakeAbilStringColor(WORD const wType, std::wstring &wstrText)
{
	int iTTBase = 0;
	int iAdd = 0;

	switch(wType)
	{
	case AT_ATTACK_ADD_FIRE:
	case AT_ATTACK_ADD_ICE:
	case AT_ATTACK_ADD_NATURE:
	case AT_ATTACK_ADD_CURSE:
	case AT_ATTACK_ADD_DESTROY:
	case AT_R_ATTACK_ADD_FIRE:
	case AT_R_ATTACK_ADD_ICE:
	case AT_R_ATTACK_ADD_NATURE:
	case AT_R_ATTACK_ADD_CURSE:
	case AT_R_ATTACK_ADD_DESTROY:
	case AT_R_ATTACK_ADD_FIRE_PHY:
	case AT_R_ATTACK_ADD_ICE_PHY:
	case AT_R_ATTACK_ADD_NATURE_PHY:
	case AT_R_ATTACK_ADD_CURSE_PHY:
	case AT_R_ATTACK_ADD_DESTROY_PHY:
	case AT_R_ATTACK_ADD_FIRE_MAGIC:
	case AT_R_ATTACK_ADD_ICE_MAGIC:
	case AT_R_ATTACK_ADD_NATURE_MAGIC:
	case AT_R_ATTACK_ADD_CURSE_MAGIC:
	case AT_R_ATTACK_ADD_DESTROY_MAGIC:
	case AT_RESIST_ADD_FIRE:
	case AT_RESIST_ADD_ICE:
	case AT_RESIST_ADD_NATURE:
	case AT_RESIST_ADD_CURSE:
	case AT_RESIST_ADD_DESTROY:
	case AT_R_RESIST_ADD_FIRE:
	case AT_R_RESIST_ADD_ICE:
	case AT_R_RESIST_ADD_NATURE:
	case AT_R_RESIST_ADD_CURSE:
	case AT_R_RESIST_ADD_DESTROY:
	case AT_R_RESIST_ADD_FIRE_PHY:
	case AT_R_RESIST_ADD_ICE_PHY:
	case AT_R_RESIST_ADD_NATURE_PHY:
	case AT_R_RESIST_ADD_CURSE_PHY:
	case AT_R_RESIST_ADD_DESTROY_PHY:
	case AT_R_RESIST_ADD_FIRE_MAGIC:
	case AT_R_RESIST_ADD_ICE_MAGIC:
	case AT_R_RESIST_ADD_NATURE_MAGIC:
	case AT_R_RESIST_ADD_CURSE_MAGIC:
	case AT_R_RESIST_ADD_DESTROY_MAGIC:
		{
			iTTBase = 792021;
			iAdd = static_cast<int>(wType)%10 - 1;
		}break;
	case AT_ATTACK_ADD_4ELEMENT:
	case AT_ATTACK_ADD_5ELEMENT:
	case AT_R_ATTACK_ADD_4ELEMENT:
	case AT_R_ATTACK_ADD_5ELEMENT:
	case AT_R_ATTACK_ADD_4ELEMENT_PHY:
	case AT_R_ATTACK_ADD_5ELEMENT_PHY:
	case AT_R_ATTACK_ADD_4ELEMENT_MAGIC:
	case AT_R_ATTACK_ADD_5ELEMENT_MAGIC:
	case AT_RESIST_ADD_4ELEMENT:
	case AT_RESIST_ADD_5ELEMENT:
	case AT_R_RESIST_ADD_4ELEMENT:
	case AT_R_RESIST_ADD_5ELEMENT:
	case AT_R_RESIST_ADD_4ELEMENT_PHY:
	case AT_R_RESIST_ADD_5ELEMENT_PHY:
	case AT_R_RESIST_ADD_4ELEMENT_MAGIC:
	case AT_R_RESIST_ADD_5ELEMENT_MAGIC:
		{
			iTTBase = 792031;
			iAdd = static_cast<int>(wType)%10;
		}break;
	default:
		{
			iTTBase = 0;
			iAdd = 0;
		}
	}

	if(0!=iTTBase)
	{
		wstrText += TTW(iTTBase + iAdd);
	}
}

void MakeAbilString(PgBase_Item const &kItem, CItemDef const* pkItemDef, WORD const Type, int Value, std::wstring &wstrText, TBL_DEF_ITEM_OPTION_ABIL* pkOptionAbil, PgBase_Item const * const pkEquipItem, bool bRoundBracket, bool bIncludeName, int const iAmpLevel, bool bAddEnter, bool const bColor,bool bSetColor)
{
	std::wstring wstrAbilName;
	const TCHAR *pText = NULL;
	if( bIncludeName && Type != AT_ITEM_SKILL)
	{//스킬시전 어빌은 별도 동작하도록 하자
		if(!GetAbilName(Type, pText))
		{
			return;
		}
	}
	else
	{
		pText = L"";
	}
	wstrAbilName = pText;

	BM::vstring wstrExplain;
	if( iAmpLevel )
	{
		if(bColor) wstrAbilName += ::TTW(1548);//녹색
		int const iMaxOptLevel = std::max<int>(pkItemDef->GetAbil(AT_LEVELLIMIT)/5 + 6, MAX_DICE_ITEM_OPTION_LEVEL);

		if(kItem.EnchantInfo().BasicAmpLv())
		{
			wstrExplain = TTW(1606);
			static wchar_t szTimeText[MAX_PATH] = {0,};
			
			int const iRate = PgItemRarityUpgradeFormula::GetBasicOptionAmpRate(Type, GetEquipType(pkItemDef), pkItemDef->GetAbil(AT_LEVELLIMIT), kItem.EnchantInfo().BasicAmpLv());
			swprintf_s(szTimeText, MAX_PATH, L"%.2f", iRate / ABILITY_RATE_VALUE_FLOAT);
			wstrExplain.Replace(L"#AMP_RATE#", szTimeText);
		}
#ifndef EXTERNAL_RELEASE
		//if( iAmpLevel < iMaxOptLevel )
		//{//(옵션강화 가능)
		//	if(bColor) wstrExplain += ::TTW(1554);
		//	wstrExplain += ::TTW(1547);
		//}
#endif
		if(bColor) wstrExplain += ::TTW(226);
	}

	if( bRoundBracket )
	{
		wstrText += L"[ ";
	}

	PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
	if(pkPilot == NULL)
	{
		return;
	}

	if(pkItemDef->IsPetItem())
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(pkPlayer)
		{
			BM::GUID const kPetGuid = pkPlayer->SelectedPetID();
			if(kPetGuid.IsNotNull())
			{
				PgPilot* pkPetPilot = g_kPilotMan.FindPilot(kPetGuid);
				if(pkPetPilot)
				{
					pkPilot = pkPetPilot;
				}
			}
		}
	}

	switch(Type)
	{
	case AT_LEVELLIMIT://	레벨 제한
	case AT_CLASSLIMIT://	직업 제한
	case AT_MAX_LEVELLIMIT://	상위 레벨 제한
	case AT_DEFAULT_AMOUNT:
	case AT_WEAPON_TYPE:
	case AT_EQUIP_LIMIT://착용위치
	case AT_ITEM_SKILL://스킬시전
		{
		}break;
	case AT_MP_RECOVERY_INTERVAL:
	case AT_HP_RECOVERY_INTERVAL:
	case AT_R_HP_RECOVERY_INTERVAL:
	case AT_R_MP_RECOVERY_INTERVAL:
	case AT_ADD_REPAIR_COST_RATE:	// 수리비용은 - 값으로 들어가야 감소되며, 툴팁에는 +로 보여준다
		{//HP/MP 회복속도는 '-' -> '+'로 보여지도록 (주기가 감소할수록 빨리 회복되므로)
			Value = (-Value);
		}//break; //고의적인 주석
	default:
		{
			E_ITEM_GRADE const eItemGrade = ::GetItemGrade(kItem);
			if(IG_CURSE == eItemGrade)
			{// 봉인 안된 저주는 빨갛게
				if(bColor) wstrText += TTW(89999);
			}
		}break;
	}	

	int iCompVal = 0;
	if(g_bIsEqComp)
	{
		switch(Type)
		{
		case AT_PHY_ATTACK_MAX:
		case AT_MAGIC_ATTACK_MAX:
		case AT_PHY_DEFENCE:
		case AT_MAGIC_DEFENCE:
			{
				CUnit* pkPlayer = pkPilot->GetUnit();
				
				if(!pkPlayer)
				{
					return;
				}

				PgInventory* pkInv = pkPlayer->GetInven();
				if(!pkInv)
				{
					return;
				}

				if (IS_CLASS_LIMIT(pkItemDef->GetAbil64(AT_CLASSLIMIT), pkPlayer->GetAbil(AT_CLASS)))
				{
					PgBase_Item kEqItem;
					if(S_OK == pkInv->GetItem(SItemPos(KUIG_FIT, pkItemDef->EquipPos()), kEqItem))
					{
						GET_DEF(CItemDefMgr, kItemDefMgr);
						CItemDef const* pEqDef = kItemDefMgr.GetDef(kEqItem.ItemNo());

						iCompVal = pEqDef->ImproveAbil((WORD&)Type, kEqItem);
					}
				}
			}break;
		}
	}

	if(pkEquipItem)
	{
		SMonsterCardTimeLimit kCardTimeLimit;
		if(true == pkEquipItem->Get(kCardTimeLimit) && true == kCardTimeLimit.IsUseTimeOut())
		{
			if(bColor) wstrText += TTW(40061);
		}
	}


	switch(Type)
	{
	case ATS_COOL_TIME:
		{
			if( Value == 0 ){ return; }

			wstrText += TTW(48);
			wstrText += WSTR_ATTR_EXPLAIN;

			float const fCalcValue = (Value / 100.0f) * 0.1f;
			if ( 60.0f<fCalcValue )	//1분 이상이면
			{
				GetTimeString(static_cast<int>(fCalcValue), wstrText, true);
				if( !bRoundBracket )
				{//1분 이상일 때 ENTER_STRING 한번 더 들어가는 것을 막는다.
					return;
				}
			}
			else
			{
				wchar_t	szTimeText[30] = {0,};
				if( (Value % 10) > 0 )
				{
					swprintf_s(szTimeText, 29, L"%.3f", fCalcValue);
				}
				else if( (Value % 100 ) > 9 )
				{
					swprintf_s(szTimeText, 29, L"%.2f", fCalcValue);
				}
				else
				{
					swprintf_s(szTimeText, 29, L"%.1f", fCalcValue);
				}
				wstrText += szTimeText;
				wstrText += TTW(9);
			}
		}break;
	case AT_GENDERLIMIT://1 남자 2 여자 3 상관없음이고		캐릭의 AT_GENDER 로 체크
		{
			if( Value && Value < 3 )
			{
				wstrText += wstrAbilName;
				wstrText += WSTR_ATTR_EXPLAIN;

				if( pkPilot->GetAbil(AT_GENDER) == Value )
				{
					wstrText += TTW(50200+Value);
				}
				else
				{
					if(bColor) wstrText += TTW(89999);
					wstrText += TTW(50200+Value);
					if(bColor) wstrText += TTW(40011);
				}
			}
		}break;
	case AT_ATTR_ATTACK:// 공격속성
	case AT_ATTR_DEFENCE:// 방어속성
		{
			wstrText += wstrAbilName;
			wstrText += WSTR_ATTR_EXPLAIN;
			wstrText += TTW(50100+Value);	//value = 1부터 들어옴. 50101은 불
		}break;
	case AT_EQUIP_LIMIT://착용위치	
		{
//			break; //표시 안함
			bool const bPetItem = pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE)==UICT_PET_EQUIP;
			wstrText += wstrAbilName;
			wstrText += WSTR_ATTR_EXPLAIN;
			GetEqiupPosString(Value, bPetItem, wstrText);
		}break;
	case AT_INVEN_SIZE:// 인벤 사이즈
	//case AT_PRICE:// 유저가 사는 가격
	//case AT_SELLPRICE:// 유저가 파는 가격
		{
			if(0 < Value)
			{
				wstrText += wstrAbilName;
				wstrText += WSTR_ATTR_EXPLAIN;
				wstrText += (std::wstring)BM::vstring(Value);
			}
		}break;
/*
	case 5005://착용가능 성별
		{
			switch( Value )	//남녀 공용은 3번. 표시할 필요 없음
			{
			case 1:
				{
					wstrText += wstrAbilName;
					wstrText += WSTR_ATTR_EXPLAIN;
					wstrText += TTW(50200+Value);
					wstrText += _T("\n");
				}break;
			case 2:
				{
					wstrText += wstrAbilName;
					wstrText += WSTR_ATTR_EXPLAIN;
					wstrText += TTW(50200+Value);
					wstrText += _T("\n");
				}break;
			}
			
		}break;
*/
	case ATI_EXPLAINID:
	case AT_CHARACTERFILTER://	캐릭터 필터링
	case AT_ITEMFILTER://	아이템 필터링
	case AT_CLASSLIMIT://	직업 제한
		{
			wstrText += wstrAbilName;
			wstrText += WSTR_ATTR_EXPLAIN;
			//int iAbil = pkPilot->GetAbil(AT_CLASS);
			__int64 iLimitFlag = pkPilot->GetAbil64(AT_CLASSLIMIT);
			if(0 == (iLimitFlag&pkItemDef->m_i64ClassLimit))//Equip disable
			{
				if(bColor) wstrText += TTW(89999);//빨간색 글자
				wstrText += TTW(30000+Value);
				if(bColor) wstrText += TTW(40011);//원래 글자
			}
			else//Equip able
			{
				wstrText += TTW(30000+Value);
			}
		}break;
	case AT_LEVELLIMIT://	레벨 제한
		{
			int const iMaxLevelLimit = pkItemDef->GetAbil(AT_MAX_LEVELLIMIT);
			int const iAddLv = pkPilot->GetAbil(AT_EQUIP_LEVELLIMIT_MIN+pkItemDef->EquipPos());
			int const iAbil = pkPilot->GetAbil(AT_LEVEL) + iAddLv;

			if( !Value && !iMaxLevelLimit )
			{
				return;
			}

			std::wstring kText = wstrAbilName;
			kText += WSTR_ATTR_EXPLAIN;

			if( (iAbil < Value) 
				|| (0 < iMaxLevelLimit && (iAbil > iMaxLevelLimit) )
				)
			{
				if(bColor) wstrText += TTW(89999);//빨간색 글자
			}

			if( Value != 0 )
			{
				kText += (std::wstring)BM::vstring(Value);
			}

			if(0 < iAddLv)
			{
				if(bColor) kText += TTW(40062);
				kText += L"(" + TTW(5001) + L" " + (std::wstring)BM::vstring(iAddLv) + L")";
				if(bColor) kText += TTW(40011);//원래 글자
			}

			if( iMaxLevelLimit != 0 )
			{
				if( Value )
				{
					kText += L" - ";
				}
				kText += (std::wstring)BM::vstring(iMaxLevelLimit);
			}
			if(bColor) kText += TTW(40011);//원래 글자
			wstrText += kText;
		}break;
	case AT_MAX_LEVELLIMIT://	상위 레벨 제한
		{
			if(Value == 0){ return; }
			wstrText += wstrAbilName;
			wstrText += WSTR_ATTR_EXPLAIN;
			int iAbil = pkPilot->GetAbil(AT_LEVEL);
			if( iAbil > Value)
			{
				if(bColor) wstrText += TTW(89999);//빨간색 글자
				wstrText += (std::wstring)BM::vstring(Value);
				if(bColor) wstrText += TTW(40011);//원래 글자
			}
			else
			{
				wstrText += (std::wstring)BM::vstring(Value);
			}
		}break;
	case AT_DEFAULT_AMOUNT:
		{
			bool const bIsCashItem = pkItemDef->IsType(ITEM_TYPE_AIDS);
			bool const bIsEquipItem = pkItemDef->IsType(ITEM_TYPE_EQUIP);

			if( bIsCashItem && bIsEquipItem )	// 캐시 장비 아이템은 내구도 툴팁 표시 안한다.
			{
				return;
			}

			int const iCustomType = pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);

			int const iCustomType4 = pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_4);
			if( UICT_NOT_DISPLAY_DURATION == iCustomType4)
			{
				break;
			}
			if(!pkItemDef->IsAmountItem())
			{
				int const iCurDurability = ((kItem.EnchantInfo().IsNeedRepair())?0:kItem.Count());//수리요구면 내구도를 0으로 표시.

				switch( iCustomType )
				{
				case UICT_ELIXIR:{ wstrText += TTW(50410); } break;//잔여 회복량
				case UICT_REST://break 없음
				case UICT_REST_EXP:{ wstrText += TTW(5020); } break;
				default:
					{
						if(!iCurDurability)
						{
							if(bColor) wstrText += TTW(89999);//이전 색깔.
						}
						
						wstrText += TTW(50411);//내구도						
					}break;
				}

				wstrText += WSTR_ATTR_EXPLAIN;
				wstrText += (std::wstring const&)(BM::vstring)iCurDurability;

				if(!pkItemDef->CanConsume())
				{//사용 불가능이면 Max 내구 표시.
					int const iMaxDurability = kItem.MaxDuration();// pkItemDef->MaxAmount();
					wstrText += _T("/");

					if(iMaxDurability != pkItemDef->MaxAmount())
					{
						if(bColor) wstrText += TTW(89999);//빨강
						wstrText += (std::wstring const&)(BM::vstring)iMaxDurability;
						if(bColor) wstrText += TTW(40011);
					}
					else
					{
						wstrText += (std::wstring const&)(BM::vstring)iMaxDurability;
					}
				}

				if(	iCustomType != UICT_ELIXIR 
				&& !iCurDurability)
				{
					if(bColor) wstrText += TTW(40011);//원래 색깔로 돌림
				}
			}
		}break;
	case AT_WEAPON_TYPE:
		{
			break;//표시 안함
			wstrText += wstrAbilName;
			wstrText += WSTR_ATTR_EXPLAIN;
			int iType = (int)(NiFloor((float)(NiFastLog(pkItemDef->GetAbil(AT_WEAPON_TYPE)) / NiFastLog(2))) - 1);
			wstrText += TTW(2000 + iType);//타입
		}break;
	case AT_R_PHY_ATTACK_MIN:
	case AT_R_PHY_ATTACK_MAX:
	case AT_R_PHY_DEFENCE:
	case AT_R_MAGIC_DEFENCE:
	case AT_R_MAGIC_ATTACK_MIN:
	case AT_R_MAGIC_ATTACK_MAX:
		{
			wstrText += wstrAbilName;

			WORD BaseType = GetBasicAbil(Type);

			int iTargetValue = pkItemDef->ImproveAbil(BaseType, kItem);
			int iVal = static_cast<int>(iTargetValue * (Value / ABILITY_RATE_VALUE_FLOAT));

			if( 0 == iTargetValue 
				|| AT_R_PHY_DEFENCE == Type 
				|| AT_R_MAGIC_DEFENCE == Type 
				|| AT_R_PHY_ATTACK_MIN == Type 
				|| AT_R_PHY_ATTACK_MAX == Type 
				|| AT_R_MAGIC_ATTACK_MIN == Type 
				|| AT_R_MAGIC_ATTACK_MAX == Type )
			{
				float fRate = Value / (ABILITY_RATE_VALUE * 0.01f);

				wchar_t szTemp[MAX_PATH] = {0,};
				swprintf(szTemp, MAX_PATH, _T("%.1f%%"), fRate);				

				if( Value > 0 )
				{
					wstrText += _T(" +");
					wstrText += szTemp;
				}
				else
				{
					wstrText += _T(" ");
					if(bColor) wstrText += TTW(89999);//이전 색깔.
					wstrText += szTemp;
					if(bColor) wstrText += TTW(40011);//원래 색깔로 돌림
				}
			}
			else
			{
				if(Value > 0)
				{
					wstrText += _T(" +");
					wstrText += (std::wstring const&)BM::vstring(iVal);
				}
				else
				{
					wstrText += _T(" ");
					if(bColor) wstrText += TTW(89999);//이전 색깔.
					wstrText += (std::wstring const&)BM::vstring(iVal);
					if(bColor) wstrText += TTW(40011);//원래 색깔로 돌림
				}
			}
		}break;
	case AT_MP_RECOVERY_INTERVAL:
	case AT_HP_RECOVERY_INTERVAL:
		{// 실 초(second)값
			float fSec = Value / (ABILITY_RATE_VALUE * 0.1f);
			wstrText += wstrAbilName;

			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf(szTemp, MAX_PATH, _T("%.3f"), fSec);
			if(0 < Value)
			{
				wstrText += _T(" +");
				wstrText += szTemp;
			}
			else
			{
				wstrText += _T(" ");
				if(bColor) wstrText += TTW(89999);//이전 색깔.
				wstrText +=  szTemp;
				if(bColor) wstrText += TTW(40011);//원래 색깔로 돌림
			}
			wstrText +=TTW(90002);	//초(second)
		}break;
	case AT_R_CRITICAL_POWER:
		{
			return;
		}break;//해당 어빌은 사용하지 않는다.
	case AT_R_HP_RECOVERY_INTERVAL:
	case AT_R_MP_RECOVERY_INTERVAL:
	case AT_R_JUMP_HEIGHT:
	case AT_R_RESIST_FIRE:
	case AT_R_RESIST_WATER:
	case AT_R_RESIST_TREE:
	case AT_R_RESIST_IRON:
	case AT_R_RESIST_EARTH:
	case AT_R_STR:
	case AT_R_INT:
	case AT_R_CON:
	case AT_R_DEX:
	case AT_R_MAX_HP:
	case AT_R_MAX_MP:
	case AT_R_HIT_SUCCESS_VALUE:
	case AT_R_MOVESPEED:
	case AT_R_BLOCK_SUCCESS_VALUE:
	case AT_R_DODGE_SUCCESS_VALUE:
	case AT_PHY_DMG_DEC:
	case AT_MAGIC_DMG_DEC:
	case AT_R_PHY_DMG_DEC:
	case AT_R_HP_RECOVERY:
	case AT_R_MP_RECOVERY:
	case AT_R_ATTACK_RANGE:
	case AT_R_ATTACK_SPEED:
	case AT_R_MAGIC_DMG_DEC:
	case AT_R_CASTING_SPEED:
	case AT_R_CRITICAL_SUCCESS_VALUE:
	case AT_CRITICAL_POWER:
	case AT_I_PHY_DEFENCE_ADD_RATE:
	case AT_I_MAGIC_DEFENCE_ADD_RATE:
	case AT_GRADE_NORMAL_DMG_ADD_RATE:
	case AT_GRADE_UPGRADED_DMG_ADD_RATE:
	case AT_GRADE_ELITE_DMG_ADD_RATE:
	case AT_GRADE_BOSS_DMG_ADD_RATE:
	case AT_ADD_R_NEED_HP:
	case AT_ADD_R_NEED_MP:
	case AT_ADD_SOULCRAFT_RATE:
	case AT_ADD_SOUL_RATE_REFUND:
	case AT_ADD_REPAIR_COST_RATE:
	case AT_PHY_DMG_100PERECNT_REFLECT_RATE:
	case AT_MAGIC_DMG_100PERECNT_REFLECT_RATE:
	case AT_PHY_DEFENCE_IGNORE_RATE:
	case AT_MAGIC_DEFENCE_IGNORE_RATE:
	case AT_100PERECNT_BLOCK_RATE:
	case AT_HIT_HEAL_HP_RATE:
	case AT_HIT_HEAL_MP_RATE:
	case AT_KILL_HEAL_HP_RATE:
	case AT_KILL_HEAL_MP_RATE:
	case AT_DEC_SOUL_RATE:	
	case AT_R_INVEN_SIZE:
	case AT_R_EQUIPS_SIZE:
	case AT_R_NEED_MP:
	case AT_R_NEED_HP:
	case AT_R_MAGIC_ATTACK:
	case AT_R_PHY_ATTACK:
	case AT_R_CRITICAL_MPOWER:
	case AT_R_WALK_SPEED:
	case AT_R_HP_POTION_ADD_RATE:
	case AT_R_MP_POTION_ADD_RATE:
	case AT_R_DMG_REFLECT_RATE:
	case AT_R_DMG_REFLECT_HITRATE:
	case AT_R_PENETRATION_RATE:
	case AT_R_MAX_HP_DEAD:
	case AT_R_MAX_MP_DEAD:
	case AT_R_ABS_ADDED_DMG_PHY:
	case AT_R_ABS_ADDED_DMG_MAGIC:
// 	case AT_R_MAX_DP:
// 	case AT_R_DP_RECOVERY_INTERVAL:
// 	case AT_R_DP_RECOVERY:	
	case AT_FINAL_HIT_SUCCESS_RATE:
	case AT_FINAL_DODGE_SUCCESS_RATE:
	case AT_FINAL_BLOCK_SUCCESS_RATE:
	case AT_FINAL_CRITICAL_SUCCESS_RATE:
		{
			if(!bSetColor)
			{
				wstrText += TTW(40063);
			}

			float fRate = Value / (ABILITY_RATE_VALUE * 0.01f);
			wstrText += wstrAbilName;

			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf(szTemp, MAX_PATH, _T("%.1f%%"), fRate);

			if( Value > 0 )
			{
				wstrText += _T(" +");
				wstrText += szTemp;
				bSetColor = true;
			}
			else
			{
				wstrText += _T(" ");
				if(bColor) wstrText += TTW(89999);//이전 색깔.
				wstrText += szTemp;
				if(bColor) wstrText += TTW(40011);//원래 색깔로 돌림
			}
		}break;
	case AT_ADD_ENCHANT_RATE:
		{
			float fRate = Value / (ABILITY_RATE_VALUE * 0.1f);
			wstrText += wstrAbilName;

			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf(szTemp, MAX_PATH, _T("%.1f%%"), fRate);

			if( Value > 0 )
			{
				wstrText += _T(" +");
				wstrText += szTemp;
			}
			else
			{
				wstrText += _T(" ");
				if(bColor) wstrText += TTW(89999);//이전 색깔.
				wstrText += szTemp;
				if(bColor) wstrText += TTW(40011);//원래 색깔로 돌림
			}
		}break;
	case AT_STR:
	case AT_INT:
	case AT_DEX:
	case AT_CON:
	case AT_MAX_HP:
	case AT_MAX_MP:
	case AT_PHY_DEFENCE:
	case AT_MAGIC_DEFENCE:
	case AT_PHY_ATTACK_MIN:// 최소 데미지
	case AT_PHY_ATTACK_MAX:// 최대 데미지
	case AT_MAGIC_ATTACK_MIN: //최소마법 공격력
	case AT_MAGIC_ATTACK_MAX://마법 최대 공격력
	case AT_MOVESPEED:
	case AT_ATTACK_SPEED:
	case AT_HIT_HEAL_HP:
	case AT_HIT_HEAL_MP:
	case AT_KILL_HEAL_HP:
	case AT_KILL_HEAL_MP:
	case AT_PHY_DMG_SUBTRACT:
	case AT_MAGIC_DMG_SUBTRACT:
	case AT_PROJECTILE_DMG_SUBTRACT:
		{
			if(false == bSetColor)
			{
				wstrText += TTW(40063);
			}

			if(Value > 0)	//정상색
			{
				wstrText += wstrAbilName;
				wstrText += _T(" +");
				wstrText += (std::wstring const&)(BM::vstring)Value;
				if(bColor) wstrText += TTW(40011);//원래 색깔로 돌림
				bSetColor = true;
			}
			else			//빨간색
			{
				wstrText += _T(" ");
				if(bColor) wstrText += TTW(89999);//이전 색깔.
				wstrText += (std::wstring const&)(BM::vstring)Value;
				if(bColor) wstrText += TTW(40011);//원래 색깔로 돌림
			}
		}break;	
	case AT_HIT_SUCCESS_VALUE:
	case AT_DODGE_SUCCESS_VALUE:
	case AT_CRITICAL_SUCCESS_VALUE:
		{
			//float fRate = Value / (ABILITY_RATE_VALUE * 0.01f);
			wstrText += wstrAbilName;

			wchar_t szTemp[MAX_PATH] = {0,};
			//swprintf(szTemp, MAX_PATH, _T("%.1f"), fRate);
			swprintf(szTemp, MAX_PATH, _T("%d"), Value);
			if( Value > 0 )
			{
				wstrText += _T(" +");
				wstrText += szTemp;
			}
			else
			{
				wstrText += _T(" ");
				if(bColor) wstrText += TTW(89999);//이전 색깔.
				wstrText += szTemp;
				if(bColor) wstrText += TTW(40011);//원래 색깔로 돌림
			}
		}break;
	case AT_ATTACK_EFFECTNUM01:
	case AT_ATTACK_EFFECTNUM02:
	case AT_ATTACK_EFFECTNUM03:
	case AT_ATTACK_EFFECTNUM04:
	case AT_ATTACK_EFFECTNUM05:
	case AT_ATTACK_EFFECTNUM06:
	case AT_ATTACK_EFFECTNUM07:
	case AT_ATTACK_EFFECTNUM08:
	case AT_ATTACK_EFFECTNUM09:
	case AT_ATTACK_EFFECTNUM10:
		{
			wstrText += L"    ";
			wstrText += wstrAbilName;
			if( Value > 0 )
			{
				GET_DEF(CSkillDefMgr, kSkillDefMgr);			
				CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(Value);
				if( pSkillDef )
				{
					const wchar_t *pName = NULL;
					GetDefString(pSkillDef->NameNo(), pName);
					if( pName )
					{
						WCHAR	temp[500] = {0,};
						swprintf(temp, 500, _T("%s %d"), TTW(224).c_str(), pSkillDef->GetAbil(AT_LEVEL));
						
						wstrText += L" ";
						wstrText += pName;
						wstrText += L" ";
						if(bColor) wstrText += L"{C=0xFF000000}";
						wstrText += temp;									
					}
				}
			}
		}break;
	case AT_EXCEPT_EFFECT_TICK:
	case AT_EXCEPT_CHAOS_EFFECT_TICK:
		{
			if( Value <= 0 )
			{
				break;
			}
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(Value);
			if( pSkillDef )
			{
				wchar_t const* pName = NULL;
				GetDefString(pSkillDef->NameNo(), pName);
				if( pName )
				{
					wstrText += pName;
					wstrText += L" ";
					if(bColor) wstrText += L"{C=0xFF000000}";
					if( 0 == pSkillDef->GetAbil(AT_IGNORE_SKILLLV_TOOLTIP) )
					{
						WCHAR	temp[500] = {0,};
						swprintf(temp, 500, _T("%s %d"), TTW(224).c_str(), pSkillDef->GetAbil(AT_LEVEL));
						wstrText += temp;
					}
					if( AT_EXCEPT_EFFECT_TICK == Type)
					{
						wstrText += wstrAbilName;
					}
				}
			}
		}break;
	case AT_ATTACK_EFFECTNUM01_RATE:
	case AT_ATTACK_EFFECTNUM02_RATE:
	case AT_ATTACK_EFFECTNUM03_RATE:
	case AT_ATTACK_EFFECTNUM04_RATE:
	case AT_ATTACK_EFFECTNUM05_RATE:
	case AT_ATTACK_EFFECTNUM06_RATE:
	case AT_ATTACK_EFFECTNUM07_RATE:
	case AT_ATTACK_EFFECTNUM08_RATE:
	case AT_ATTACK_EFFECTNUM09_RATE:
	case AT_ATTACK_EFFECTNUM10_RATE:
		{
			wstrText += L"    ";
			float fRate = Value / (ABILITY_RATE_VALUE * 0.01f);
			wstrText += wstrAbilName;

			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf(szTemp, MAX_PATH, _T("%.1f%%"), fRate);
			if( Value > 0 )
			{
				wstrText += _T(" ");
				wstrText += szTemp;
				//wstrText += TTW(790424);
			}
			else
			{
				wstrText += _T(" ");
				if(bColor) wstrText += TTW(89999);//이전 색깔.
				wstrText += szTemp;
				if(bColor) wstrText += TTW(40011);//원래 색깔로 돌림
			}
		}break;
	case AT_ATTACK_EFFECTNUM01_TARGET_TYPE_IS_ME:
	case AT_ATTACK_EFFECTNUM02_TARGET_TYPE_IS_ME:
	case AT_ATTACK_EFFECTNUM03_TARGET_TYPE_IS_ME:
	case AT_ATTACK_EFFECTNUM04_TARGET_TYPE_IS_ME:
	case AT_ATTACK_EFFECTNUM05_TARGET_TYPE_IS_ME:
	case AT_ATTACK_EFFECTNUM06_TARGET_TYPE_IS_ME:
	case AT_ATTACK_EFFECTNUM07_TARGET_TYPE_IS_ME:
	case AT_ATTACK_EFFECTNUM08_TARGET_TYPE_IS_ME:
	case AT_ATTACK_EFFECTNUM09_TARGET_TYPE_IS_ME:
	case AT_ATTACK_EFFECTNUM10_TARGET_TYPE_IS_ME:
		{
			wstrText += L"    ";
			wstrText += wstrAbilName;
			wstrText += L" ";
			if(Value > 0)
			{
				wstrText += TTW(790425);
			}
			else
			{
				wstrText += TTW(790426);
			}
		}break;
	case AT_DMG_REFLECT_RATE_FROM_ITEM01:
	case AT_DMG_REFLECT_RATE_FROM_ITEM02:
	case AT_DMG_REFLECT_RATE_FROM_ITEM03:
	case AT_DMG_REFLECT_RATE_FROM_ITEM04:
	case AT_DMG_REFLECT_RATE_FROM_ITEM05:
	case AT_DMG_REFLECT_RATE_FROM_ITEM06:
	case AT_DMG_REFLECT_RATE_FROM_ITEM07:
	case AT_DMG_REFLECT_RATE_FROM_ITEM08:
	case AT_DMG_REFLECT_RATE_FROM_ITEM09:
	case AT_DMG_REFLECT_RATE_FROM_ITEM10:
		{// Damage 반사할 때, 반사가 일어날 확률
			std::wstring kTempText = TTW(790800); //L"피격시 {PER}%의 확률로 ";
			int const iDmgAbilType = Type+10;
			int iReflectRate = 0;	// Damage 반사할 때, 반사되는 양(Damage에 대한 만분율값)
			//GetAbilFromSetItemOption(pkOptionAbil, AT_DMG_REFLECT_RATE, iReflectRate ); //아이템으로 인한 반사확률은 스킬과 별도로 만들어져야함(현재 없음)

			int iReflectValue = 0;	// Damage 반사할때 , 반사되는 데미지 절대값 (100이면 100데미지줌)
			GetAbilFromSetItemOption(pkOptionAbil, iDmgAbilType, iReflectValue);

			if(iReflectRate && !iReflectValue)
			{// 반사 확률만 있을 경우
				kTempText +=TTW(790801);//L"데미지의 {PER1}%반사";
			}
			else if(!iReflectRate && iReflectValue)
			{// 반사 데미지값만 있을경우
				kTempText +=TTW(790802);//L"데미지 {VALUE}반사";
			}
			else
			{// 양쪽 다 있는 경우 이거나, 둘다 없는 경우
				kTempText +=TTW(790803);//L"데미지의 {PER1}%와 데미지 {VALUE}반사";
			}
			BM::vstring vstrText(kTempText);
			float const fPer = static_cast<float>(Value) / 100.0f;
			vstrText.Replace( L"#PER#", BM::vstring( fPer, L"%.1f") );
			
			float const fReflectRate = static_cast<float>(iReflectRate)/100.0f;
			vstrText.Replace( L"#PER1#", BM::vstring( fReflectRate, L"%.1f"));

			vstrText.Replace( L"#VALUE#", iReflectValue );

			wstrText += static_cast<std::wstring>(vstrText);
		}break;
	case AT_HP_RESTORE_WHEN_HIT_RATE:
		{//타격시 HP 회복시켜줄 확률:
			std::wstring kTempText;
			if( MakeAbilNameString( Type, kTempText ) )
			{//wstrText = L"타격시 {PER}%의 확률로 {VALUE}HP 회복";
				BM::vstring vstrText(kTempText); 
				float const fPer = static_cast<float>(Value) / 100.0f;
				vstrText.Replace( L"{PER}", BM::vstring( fPer, L"%.1f") );
				
				int iHPRestoreVal = 0;
				GetAbilFromSetItemOption(pkOptionAbil, AT_HP_RESTORE_WHEN_HIT, iHPRestoreVal);
				vstrText.Replace( L"{VALUE}", iHPRestoreVal );//타격시 HP 회복 시켜주는 값(절대값)
				wstrText += static_cast<std::wstring>(vstrText);
			}
		}break;

	case AT_MP_RESTORE_WHEN_HIT_RATE:
		{//타격시 MP 회복시켜줄 확률
			std::wstring kTempText;
			if( MakeAbilNameString( Type, kTempText ) )
			{
				//wstrText = L"타격시 {PER}%의 확률로 {VALUE}MP 회복";
				BM::vstring vstrText(kTempText); 
				float const fPer = static_cast<float>(Value) / 100.0f;
				vstrText.Replace( L"{PER}", BM::vstring( fPer, L"%.1f") );
				int iMPRestoreVal = 0;
				GetAbilFromSetItemOption(pkOptionAbil, AT_MP_RESTORE_WHEN_HIT, iMPRestoreVal);
				vstrText.Replace( L"{VALUE}", iMPRestoreVal );//타격시 MP 회복 시켜주는 값(절대값)

				wstrText += static_cast<std::wstring>(vstrText);
			}
			
		}break;
	case AT_IMMUNITY_FREEZE_RATE:
		{//동빙 무시 확률
			std::wstring kTempText;
			if( MakeAbilNameString( Type, kTempText ) )
			{
				//wstrText = L"{PER}%확률로 동빙 무시";
				BM::vstring vstrText(kTempText); 
				float const fPer = static_cast<float>(Value) / 100.0f;
				vstrText.Replace( L"{PER}", BM::vstring( fPer, L"%.1f") );

				wstrText += static_cast<std::wstring>(vstrText);
			}
		}break;
	case AT_IMMUNITY_COLD_RATE:
		{//결빙 무시 확률
			std::wstring kTempText;
			if( MakeAbilNameString( Type, kTempText ) )
			{
				//wstrText = L"{PER}%확률로 결빙 무시";
				BM::vstring vstrText(kTempText); 
				float const fPer = static_cast<float>(Value) / 100.0f;
				vstrText.Replace( L"{PER}", BM::vstring( fPer, L"%.1f") );

				wstrText += static_cast<std::wstring>(vstrText);
			}
		}break;
	case AT_ITEM_SKILL:
		{
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(Value);
			if( pkSkillDef )
			{
				const TCHAR *pSkillName = NULL;
				int const iNameNo = pkSkillDef->RscNameNo();
				if( !GetDefString(iNameNo, pSkillName) )
				{
					return;
				}
				BM::vstring kstrText(TTW(799473));
				kstrText.Replace(L"#SKILL#", pSkillName);
				wstrText += static_cast<std::wstring>(kstrText);
			}
		}break;
	case AT_ADD_R_NEED_MP_2:
		{
			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf(szTemp, MAX_PATH, _T("%.1f"), Value/100.0f);

			wstrText += wstrAbilName;
			wstrText += WSTR_ATTR_EXPLAIN;
			wstrText += szTemp;
			wstrText += L"%";
		}break;
	case AT_R_ATTACK_ADD_FIRE:
	case AT_R_ATTACK_ADD_ICE:
	case AT_R_ATTACK_ADD_NATURE:
	case AT_R_ATTACK_ADD_CURSE:
	case AT_R_ATTACK_ADD_DESTROY:
	case AT_R_ATTACK_ADD_FIRE_PHY:
	case AT_R_ATTACK_ADD_ICE_PHY:
	case AT_R_ATTACK_ADD_NATURE_PHY:
	case AT_R_ATTACK_ADD_CURSE_PHY:
	case AT_R_ATTACK_ADD_DESTROY_PHY:
	case AT_R_ATTACK_ADD_FIRE_MAGIC:
	case AT_R_ATTACK_ADD_ICE_MAGIC:
	case AT_R_ATTACK_ADD_NATURE_MAGIC:
	case AT_R_ATTACK_ADD_CURSE_MAGIC:
	case AT_R_ATTACK_ADD_DESTROY_MAGIC:
	case AT_R_RESIST_ADD_FIRE:
	case AT_R_RESIST_ADD_ICE:
	case AT_R_RESIST_ADD_NATURE:
	case AT_R_RESIST_ADD_CURSE:
	case AT_R_RESIST_ADD_DESTROY:
	case AT_R_RESIST_ADD_FIRE_PHY:
	case AT_R_RESIST_ADD_ICE_PHY:
	case AT_R_RESIST_ADD_NATURE_PHY:
	case AT_R_RESIST_ADD_CURSE_PHY:
	case AT_R_RESIST_ADD_DESTROY_PHY:
	case AT_R_RESIST_ADD_FIRE_MAGIC:
	case AT_R_RESIST_ADD_ICE_MAGIC:
	case AT_R_RESIST_ADD_NATURE_MAGIC:
	case AT_R_RESIST_ADD_CURSE_MAGIC:
	case AT_R_RESIST_ADD_DESTROY_MAGIC:
	case AT_R_ATTACK_ADD_4ELEMENT:    
	case AT_R_ATTACK_ADD_5ELEMENT:     
	case AT_R_ATTACK_ADD_4ELEMENT_PHY:
	case AT_R_ATTACK_ADD_5ELEMENT_PHY:
	case AT_R_ATTACK_ADD_4ELEMENT_MAGIC:
	case AT_R_ATTACK_ADD_5ELEMENT_MAGIC:
	case AT_R_RESIST_ADD_4ELEMENT:
	case AT_R_RESIST_ADD_5ELEMENT:
	case AT_R_RESIST_ADD_4ELEMENT_PHY:
	case AT_R_RESIST_ADD_5ELEMENT_PHY:
	case AT_R_RESIST_ADD_4ELEMENT_MAGIC:
	case AT_R_RESIST_ADD_5ELEMENT_MAGIC:
		{
			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf(szTemp, MAX_PATH, _T("%.1f"), Value/100.0f);
			wstrText += wstrAbilName;
			wstrText += L": ";
			wstrText += szTemp;
			wstrText += L"%";
		}break;
	case AT_EFFECT_DELETE_ALL_DEBUFF:
		{
			if( Value && pkItemDef->CanConsume() )
			{
				wchar_t szTemp[MAX_PATH] = {0,};
				swprintf(szTemp, MAX_PATH, _T("%d"), Value);
				wstrText += szTemp;
				wstrText += TTW(40906);
			}
		}break;
	case AT_HP:
		{
			if( Value && pkItemDef->CanConsume() )
			{
				wchar_t szTemp[MAX_PATH] = {0,};
				swprintf(szTemp, MAX_PATH, _T("%d"), Value);
				wstrText += TTW(40907);
				wstrText += szTemp;
			}
		}break;
	case AT_MP:
		{
			if( Value && pkItemDef->CanConsume() )
			{
				wchar_t szTemp[MAX_PATH] = {0,};
				swprintf(szTemp, MAX_PATH, _T("%d"), Value);
				wstrText += TTW(40908);
				wstrText += szTemp;
			}
		}break;
	default:
		{
			wstrText += wstrAbilName;
			wstrText += L": ";
			wstrText += (std::wstring const&)(BM::vstring)Value; 
		}break;
	}

	if( bRoundBracket )
	{
		wstrText += L"] ";
	}
	else
	{
		wstrText += static_cast<std::wstring>(wstrExplain);
		if( bAddEnter )
		{
			wstrText += ENTER_STRING;
		}
		else
		{
			wstrText += L" ";
		}
	}
}
bool MakeSkillLimitClassText(const	CSkillDef *pkSkillDef,std::wstring &kOut)
{
	kOut.clear();

	int iClassID = 0;
	for(int i=0;i<UCLASS_MAX;++i)
	{
		iClassID = i+1;

		if(pkSkillDef->m_i64ClassLimit&(1i64<<iClassID))
		{
			kOut+=TTW(30000+iClassID);
			return	true;
		}
	}

	return	false;
}
bool MakeSkillToolTipText( PgSkillTree::stTreeNode *pTreeNode, std::wstring &wstrTextOut,bool bNextLevel, std::wstring& wstrType)
{
	WCHAR	temp[500];
	wstrTextOut.clear();

	CSkillDef const* pSkillDef = pTreeNode->m_pkSkillDef;
	int iSkillNo = pTreeNode->m_ulSkillNo;
	int iOverSkillLevel = pTreeNode->GetOverSkillLevel();
	int iSkillLevel = iSkillNo % 100;

	if(!(pTreeNode->m_bLearned == false && pTreeNode->m_bTemporaryLearned == false) && 1 <= iSkillLevel && iOverSkillLevel)
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		pSkillDef = kSkillDefMgr.GetDef(pTreeNode->m_ulSkillNo + iOverSkillLevel);
	}

	if(bNextLevel)
	{
		int AddLevel = 1;
		if(!(pTreeNode->m_bLearned == false && pTreeNode->m_bTemporaryLearned == false) && 1 <= iSkillLevel && iOverSkillLevel)
		{
			AddLevel += iOverSkillLevel;
		}

		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		pSkillDef = kSkillDefMgr.GetDef(pTreeNode->m_ulSkillNo + AddLevel);
		if(!pSkillDef)
		{
			return true;
		}

		++iSkillNo;
	}

	if(!pSkillDef)
	{
		return false;
	}

	bool const bCouple = pSkillDef->GetAbil(AT_IS_COUPLE_SKILL);
	
	//	현재 레벨 스킬의 정보를 보여준다.
	wchar_t const* pName = NULL;
	GetDefString(pSkillDef->NameNo(),pName);

	//	이름용 색상 
	if(!bNextLevel)
	{
		wstrTextOut += TTW(40102); //이름용
	}
	else
	{
		wstrTextOut += TTW(234); //이름용
	}

	//	이름
	if(pName)
	{	
		int iLevel = pSkillDef->GetAbil(AT_LEVEL);
		
		swprintf(temp,500,_T("%s {C=0xFF000000}%s %d"),pName,TTW(224).c_str(),iLevel);
		wstrTextOut+=temp;
	}
	else
	{
		wstrTextOut+=_T("Undefined Name");
	}

	//	분류
	if(!bNextLevel)
	{
		wstrType+=TTW(40140+pSkillDef->m_byType);
	}

	bool const bGuildSkill = SDT_Guild == pSkillDef->GetAbil(AT_SKILL_DIVIDE_TYPE);
	if( bGuildSkill )
	{
		wstrTextOut += _T("\n ") + TTW(400823);//[길드 스킬]
	}

#ifndef USE_INB
	if( g_pkApp->VisibleClassNo() )
	{
		BM::vstring kSkillNo(pSkillDef->No());
		wstrTextOut += _T("\n SkillNo : ") + (std::wstring)kSkillNo;
	}
#endif

	//	공백
	wstrTextOut+=_T("\n\n");

	//	일반 색상
	if(!bNextLevel)
		wstrTextOut += TTW(40011);	//폰트 초기화
	else
		wstrTextOut += TTW(226);	//폰트 초기화

	if(!bNextLevel)
	{
		//	선행스킬
		int kArray[LEAD_SKILL_COUNT] = {0,};
		if (g_kSkillTree.GetNeedSkill(iSkillNo, kArray))
		{
			wstrTextOut += TTW(40130);
			wstrTextOut += _T(" : ");

			for (int i = 0; i < LEAD_SKILL_COUNT; ++i)
			{
				int const iNo = kArray[i];
				if (0!=iNo)
				{
					int const iKey = g_kSkillTree.GetKeySkillNo(iNo);
					PgSkillTree::stTreeNode* pNode = g_kSkillTree.GetNode(iKey);
					if (pNode)
					{
						const CSkillDef* pkLeadDef = pNode->GetSkillDef();
						if (pkLeadDef)
						{
							if (i>0)
							{
								wstrTextOut+=_T(", ");
							}
							if((pNode->IsLearned() || pNode->m_bTemporaryLearned) && pNode->m_ulSkillNo >= iNo)
							{		
								wstrTextOut+=TTW(40106);
							}
							else
							{
								wstrTextOut+=TTW(40105);
							}

							std::wstring const *pLeadName = NULL;

							if(GetDefString(pkLeadDef->NameNo(),pLeadName))
							{
								wstrTextOut += std::wstring(*pLeadName);
								wstrTextOut += L" ";
								wstrTextOut += TTW(224);
								wstrTextOut += std::wstring(BM::vstring(iNo-iKey+1));
							}
						}
					}
				}
			}

			wstrTextOut+=_T('\n');
		}

		//	색상 초기화
		if(!bNextLevel)
			wstrTextOut += TTW(40011);	//폰트 초기화
		else
			wstrTextOut += TTW(226);	//폰트 초기화
	}

	const CONT_DEFSKILL *pkContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pkContDefMap);
	if( !bNextLevel )
	{
		//	스킬 커맨드
		if (pkContDefMap)
		{
			CONT_DEFSKILL::const_iterator def_it = pkContDefMap->find(pTreeNode->m_ulSkillNo);
			if (pkContDefMap->end() != def_it)
			{
				CONT_DEFSKILL::mapped_type kT = (*def_it).second;
				if( 0 != kT.iCmdStringNo )
				{
					wstrTextOut+=TTW(40107);
					wstrTextOut+=_T(" : ");

					const wchar_t *pText = NULL;
					if(GetDefString(kT.iCmdStringNo, pText))
					{
						wstrTextOut+=pText;
					}
					wstrTextOut+=_T("\n");
				}
			}
		}
	}

	//	필요 SP
	if(!bCouple)
	{
		int const iNeed = pSkillDef->GetAbil(AT_NEED_SP);

		if(iNeed > 0)
		{
			wstrTextOut+=TTW(40131);
			wstrTextOut+=_T(" : ");
			if (g_kSkillTree.GetRemainSkillPoint() < iNeed)
			{
				wstrTextOut+=TTW(40105);
			}
			swprintf(temp,500,_T("%d\n"),iNeed);			
			wstrTextOut+=temp;
		}

		//	색상 초기화
		if(!bNextLevel)
			wstrTextOut += TTW(40011);	//폰트 초기화
		else
			wstrTextOut += TTW(226);	//폰트 초기화
	}

	//	종류(보류)
//	swprintf(temp,100,_T("%s : \n"),TTW(228).c_str());
//	wstrTextOut+=temp;

	//	소모 MP
	if (EST_PASSIVE != pSkillDef->m_byType)	//패시브 스킬아닌 경우
	{
		if(0<pSkillDef->m_sMP)
		{
			swprintf(temp,500,_T("%s : %d\n"),TTW(40133).c_str(), pSkillDef->m_sMP);
			wstrTextOut+=temp;
		}
		else
		{
			//소모 MP가 MAXMP의 %를 소모 할 경우
			int const iNeedRateMaxMP = pSkillDef->GetAbil(AT_NEED_MAX_R_MP);
			if(0 < iNeedRateMaxMP)
			{
				float const fRateMP = static_cast<float>(iNeedRateMaxMP / 100.0f);
				swprintf(temp,500,_T("%s : %s %.1f%%\n"),TTW(40133).c_str(), TTW(512).c_str(), fRateMP);
				wstrTextOut+=temp;
			}
		}

		if(0<pSkillDef->GetAbil(AT_NEED_AWAKE) || 0<pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE))
		{
			wstrTextOut += TTW(1306);//노랑
			//if(3000<=pSkillDef->GetAbil(AT_NEED_AWAKE) 
			//	|| 6000<=pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE))
			//{// 대량 소모한다 표시
			//	wstrTextOut += TTW(5060);
			//}
			//else
			//{// 그냥 소모 한다 표시
			//	wstrTextOut += TTW(5044);
			//}
			{
				wstrTextOut += TTW(5061);
				wstrTextOut += L" : ";
				BM::vstring vStr;
				if(0 < pSkillDef->GetAbil(AT_NEED_AWAKE))
				{
					vStr += pSkillDef->GetAbil(AT_NEED_AWAKE);
				}
				else if(0 < pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE))
				{
					vStr += pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE)/ABILITY_RATE_VALUE;
					vStr += L"%";
				}
				wstrTextOut += static_cast<std::wstring>(vStr);
			}
			wstrTextOut += TTW(40011);
			wstrTextOut += L"\n";
		}
		else
		{
			if(0<pSkillDef->GetAbil(AT_ALL_NEED_AWAKE))
			{
				wstrTextOut += TTW(1306);//노랑
				wstrTextOut += TTW(5045);
				wstrTextOut += TTW(40011);
				wstrTextOut += L"\n";
			}
		}
	}

	// 소모 HP
	if (0 < pSkillDef->m_sHP)
	{
		swprintf(temp,500,_T("%s : %d\n"),TTW(40134).c_str(), pSkillDef->m_sHP);
		wstrTextOut+=temp;		
	}
	else
	{
		//소모 MP가 MAXHP의 %를 소모 할 경우
		int const iNeedRateMaxHP = pSkillDef->GetAbil(AT_NEED_MAX_R_HP);
		if(0 < iNeedRateMaxHP)
		{
			float const fRateHP = static_cast<float>(iNeedRateMaxHP / 100.0f);
			swprintf(temp,500,_T("%s : %s %.1f%%\n"),TTW(40134).c_str(), TTW(511).c_str(), fRateHP);
			wstrTextOut+=temp;
		}
	}

	CSkillDef const* pBackupSkillDef = NULL;
	//실제 공격 스킬은 따로 존재 하는 경우
	int iRealSkill = pSkillDef->GetAbil(AT_CHILD_CAST_SKILL_NO);
	if(0 < iRealSkill)
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pChildSkillDef = kSkillDefMgr.GetDef(iRealSkill);
		if(pChildSkillDef)
		{
			pBackupSkillDef = pSkillDef;
			pSkillDef = pChildSkillDef;
		}
	}

	//데미지를 나눠서 공격하는 스킬은 한발 데미지로 나누어 주어야 한다.
	int iComboCount = pSkillDef->GetAbil(AT_COMBO_HIT_COUNT);

	//	물리대미지
	int iAtk = pSkillDef->GetAbil(AT_I_PHY_ATTACK_ADD_RATE);
	if(0 < iComboCount)
	{
		iAtk /= iComboCount;
	}

	if( 0 < iAtk )
	{
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(40136).c_str(), ((float)(iAtk) * 0.01f));
		wstrTextOut+=temp;
		iAtk = pSkillDef->GetAbil(AT_PHY_ATTACK);
		if(0 < iComboCount)
		{
			iAtk /= iComboCount;
		}

		if( 0 < iAtk )
		{
			swprintf(temp, 499, _T(" + %d"), iAtk);
			wstrTextOut+=temp;
		}
		wstrTextOut+=L"\n";
	}

	//	마법대미지
	int iMAtk = pSkillDef->GetAbil(AT_I_MAGIC_ATTACK_ADD_RATE);
	if(0 < iComboCount)
	{
		iMAtk /= iComboCount;
	}

	if( 0 < iMAtk )
	{
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(40137).c_str(), ((float)(iMAtk) * 0.01f));
		wstrTextOut+=temp;
		iMAtk = pSkillDef->GetAbil(AT_MAGIC_ATTACK);
		if(0 < iComboCount)
		{
			iMAtk /= iComboCount;
		}
		if( 0 < iMAtk )
		{
			swprintf(temp, 499, _T(" + %d"), iMAtk);
			wstrTextOut+=temp;
		}
		wstrTextOut+=L"\n";
	}

	//데미지만 실제 공격 스킬에서 얻어 오고 원래 스킬로 돌려 준다.
	if(NULL != pBackupSkillDef)
	{
		pSkillDef = pBackupSkillDef;
	}

	wstrTextOut += TTW(40011);	//폰트 초기화

	//	색상 초기화
	if(!bNextLevel)
	{
		wstrTextOut += TTW(40011);	//폰트 초기화
	}
	else
	{
		wstrTextOut += TTW(226);	//폰트 초기화
	}

	//	제한직업
	/*std::wstring kLimitClass;
	if(MakeSkillLimitClassText(pSkillDef,kLimitClass))
	{
		swprintf(temp,100,_T("%s : %s %s\n"),TTW(229).c_str(),kLimitClass.c_str(),TTW(233).c_str());
		wstrTextOut+=temp;
	}*/

	int iGroggyLine = 0;
	int const iGroggyAccPointLv = lua_tinker::call<int, int>("GetGroggyAccPointLv", pSkillDef->GetAbil(AT_ADD_GROGGYPOINT));
	if( iGroggyAccPointLv )
	{
		++iGroggyLine;
		
		swprintf(temp, sizeof(temp), _T("%s%s %d : %s %s%d"), TTW(40149).c_str(), TTW(40146).c_str(), iGroggyLine, TTW(40147).c_str(), TTW(224).c_str(), iGroggyAccPointLv);
		wstrTextOut+=temp;
		wstrTextOut+=L"\n";
	}

	int const iGroggyRelaxRateLv = lua_tinker::call<int, int>("GetGroggyRelaxRateLv", pSkillDef->GetAbil(AT_DEC_GROGGYPOINT_RATE));
	if( iGroggyRelaxRateLv )
	{
		++iGroggyLine;

		swprintf(temp, sizeof(temp), _T("%s%s %d : %s %s%d"), TTW(40150).c_str(), TTW(40146).c_str(), iGroggyLine, TTW(40148).c_str(), TTW(224).c_str(), iGroggyRelaxRateLv);
		wstrTextOut+=temp;
		wstrTextOut+=L"\n";
	}

	if( iGroggyLine )
	{
		wstrTextOut += TTW(40011);	//폰트 초기화
	}

	//	제한레벨
	PgPilot	*pkPlayerPilot = g_kPilotMan.GetPlayerPilot();
	if(pkPlayerPilot)
	{
		if(pkPlayerPilot->GetAbil(AT_LEVEL)<pSkillDef->m_sLevelLimit)	//	빨간색으로 표시하자
		{
			wstrTextOut+=TTW(225);
		}
	}

	if(0<pSkillDef->m_sLevelLimit)
	{
		swprintf(temp,100,_T("%s : %d %s\n"),TTW(230).c_str(),pSkillDef->m_sLevelLimit,TTW(232).c_str());
		wstrTextOut+=temp;
	}

	if(!bNextLevel)
		wstrTextOut += TTW(40011);	//폰트 초기화
	else
		wstrTextOut += TTW(226);	//폰트 초기화


	//	길드 제한 레벨
	CONT_DEF_GUILD_SKILL const *pkDefGuildSkill = NULL;
	g_kTblDataMgr.GetContDef(pkDefGuildSkill);
	if( pkDefGuildSkill
	&&	bGuildSkill )
	{
		CONT_DEF_GUILD_SKILL::const_iterator guildskill_iter = pkDefGuildSkill->find(iSkillNo);
		if( pkDefGuildSkill->end() != guildskill_iter )
		{
			CONT_DEF_GUILD_SKILL::mapped_type const &rkDefGuildSkill = (*guildskill_iter).second;

			if( g_kGuildMgr.Level() < rkDefGuildSkill.sGuildLv )
			{
				wstrTextOut+=TTW(225);//붉은색
			}

			swprintf(temp,100,_T("%s : %d %s\n"),TTW(400824).c_str(),rkDefGuildSkill.sGuildLv,TTW(232).c_str());
			wstrTextOut+=temp;

			if( !bNextLevel )	{ wstrTextOut += TTW(40011); }	//폰트 초기화
			else				{ wstrTextOut += TTW(226); }	//폰트 초기화
		}
	}

	//	제한무기
	if (pkContDefMap)
	{
		CONT_DEFSKILL::const_iterator def_it = pkContDefMap->find(pTreeNode->m_ulSkillNo);
		if (pkContDefMap->end() != def_it)
		{
			CONT_DEFSKILL::mapped_type kT = (*def_it).second;
			if(kT.iWeaponLimit)
			{
				wstrTextOut+=TTW(40132);
				wstrTextOut+=_T(" : ");
				int iCount = 0;
				for (int i = 0; i< 12; ++i)
				{
					int const iWeaponType = 1<<i;
					if (kT.iWeaponLimit & iWeaponType)
					{ 
						if (iCount>0)
						{
							wstrTextOut+=_T(", ");
						}
						int iType = (int)(NiFastLog(iWeaponType) + 1);
						wstrTextOut+=TTW(2000+iType);
						++iCount;
					}
				}
				//int iType = (int)(NiFastLog(kT.iWeaponLimit) + 1);
				//swprintf(temp,500,_T("%s : %s\n"),TTW(40132).c_str(), TTW(2000+iType).c_str());
				wstrTextOut+=_T("\n");
			}
		}
	}
	
	//	시전 시간
	int const iCast = pSkillDef->GetAbil(AT_CAST_TIME);
	
	if (0<iCast)
	{
		if( 0 < ((iCast % 1000) * 0.01) )
			swprintf(temp,500,_T("%s : %.1f%s\n"),TTW(47).c_str(), iCast*0.001f, TTW(54).c_str());
		else
			swprintf(temp,500,_T("%s : %d%s\n"),TTW(47).c_str(), static_cast<int>(iCast*0.001f), TTW(54).c_str());
	}
	else
	{
		swprintf(temp, 500, _T("%s : %s\n"), TTW(47).c_str(), TTW(251).c_str());
	}
	wstrTextOut += temp;

	//	쿨타임
	int const iCool = pSkillDef->GetAbil(ATS_COOL_TIME);
	if (0<iCool)
	{
		if( 0 < ((iCool % 1000) * 0.01) )
			swprintf(temp,500,_T("%s : %.1f%s\n"),TTW(250).c_str(), iCool*0.001f, TTW(54).c_str());
		else
			swprintf(temp,500,_T("%s : %d%s\n"),TTW(250).c_str(), static_cast<int>(iCool*0.001f), TTW(54).c_str());
		wstrTextOut+=temp;
	}
	
	int const iMaxTarget = pSkillDef->GetAbil(AT_MAX_TARGETNUM);
	if(1 < iMaxTarget)
	{// 최대타격개체수
		BM::vstring vTemp( TTW(40151) ); 
		vTemp.Replace(L"#NUM#", iMaxTarget);
		vTemp+="\n";
		wstrTextOut+=vTemp.operator const std::wstring &();
	}
	int const iMaxPenetration = pSkillDef->GetAbil(AT_PENETRATION_COUNT);
	if(0 < iMaxPenetration)
	{// 최대관통개체수
		BM::vstring vTemp( TTW(40152) ); 
		vTemp.Replace(L"#NUM#", iMaxPenetration);
		vTemp+="\n";
		wstrTextOut+=vTemp.operator const std::wstring &();
	}

	bool bCantBlockTxt = false;
	//지속 시간
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	const	CEffectDef* pEffDef = kEffectDefMgr.GetDef(pSkillDef->No());
	if (pEffDef)
	{
		int const iDuration = pEffDef->GetDurationTime()/1000;
		if (0<iDuration)
		{
			swprintf(temp,500,_T("%s : "),TTW(40135).c_str());
			wstrTextOut+=temp;
			GetTimeString(iDuration, wstrTextOut, true);
		}
		if( pEffDef->GetAbil(AT_CAN_NOT_BLOCK_BY_EFFECT) )
		{
			bCantBlockTxt = true;
		}
	}

	//	설명
	const TCHAR *pText = NULL;
	int Value = pSkillDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		if (strlenT(pText))
		{
			swprintf(temp,100,_T("\n%s : \n"),TTW(231).c_str());
			wstrTextOut+= ::TTW(750010);	//노랑
			wstrTextOut+=temp;
			wstrTextOut+= ::TTW(1554);		//흰색
			wstrTextOut+=pText;			
		}
	}
	
	if( !bCantBlockTxt
		&& 0 < pSkillDef->GetAbil(AT_CAN_NOT_BLOCK_BY_ACTION) 
		)
	{
		bCantBlockTxt = true;
	}
	if( bCantBlockTxt )
	{// 블록 할 수 없습니다.
		wstrTextOut += ENTER_STRING; 
		wstrTextOut += ENTER_STRING;
		wstrTextOut += TTW(750010);	// 컬러(노랑), 폰트
		wstrTextOut += TTW(791611);
	}
	return	true;
}	


bool MakeSkillToolTipText(int iSkillNo, std::wstring &wstrTextOut,/*bool bNextLevel,*/ std::wstring& wstrType)
{
	WCHAR	temp[500];
	wstrTextOut.clear();

	//CSkillDef const* pSkillDef = pTreeNode->m_pkSkillDef;
	//int iSkillNo = pTreeNode->m_ulSkillNo;
	//int iOverSkillLevel = pTreeNode->GetOverSkillLevel();
	int iSkillLevel = iSkillNo % 100;

	//if(!(pTreeNode->m_bLearned == false && pTreeNode->m_bTemporaryLearned == false) && 1 <= iSkillLevel && iOverSkillLevel)
	//{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo /*+ iOverSkillLevel*/);
	//}

	// if(bNextLevel)
	// {
		// int AddLevel = 1;
		// if(!(pTreeNode->m_bLearned == false && pTreeNode->m_bTemporaryLearned == false) && 1 <= iSkillLevel && iOverSkillLevel)
		// {
			// AddLevel += iOverSkillLevel;
		// }

		// GET_DEF(CSkillDefMgr, kSkillDefMgr);
		// pSkillDef = kSkillDefMgr.GetDef(pTreeNode->m_ulSkillNo + AddLevel);
		// if(!pSkillDef)
		// {
			// return true;
		// }

		// ++iSkillNo;
	// }

	if(!pSkillDef)
	{
		return false;
	}

	bool const bCouple = pSkillDef->GetAbil(AT_IS_COUPLE_SKILL);
	
	//	현재 레벨 스킬의 정보를 보여준다.
	wchar_t const* pName = NULL;
	GetDefString(pSkillDef->NameNo(),pName);

	//	이름용 색상 
//	if(!bNextLevel)
	{
		wstrTextOut += TTW(40102); //이름용
	}
	// else
	// {
		// wstrTextOut += TTW(234); //이름용
	// }

	//	이름
	if(pName)
	{	
		int iLevel = pSkillDef->GetAbil(AT_LEVEL);
		
		swprintf(temp,500,_T("%s {C=0xFF000000}%s %d"),pName,TTW(224).c_str(),iLevel);
		wstrTextOut+=temp;
	}
	else
	{
		wstrTextOut+=_T("Undefined Name");
	}

	//	분류
//	if(!bNextLevel)
	{
		wstrType+=TTW(40140+pSkillDef->m_byType);
	}

	bool const bGuildSkill = SDT_Guild == pSkillDef->GetAbil(AT_SKILL_DIVIDE_TYPE);
	if( bGuildSkill )
	{
		wstrTextOut += _T("\n ") + TTW(400823);//[길드 스킬]
	}

#ifndef USE_INB
	if( g_pkApp->VisibleClassNo() )
	{
		BM::vstring kSkillNo(pSkillDef->No());
		wstrTextOut += _T("\n SkillNo : ") + (std::wstring)kSkillNo;
	}
#endif

	//	공백
	wstrTextOut+=_T("\n\n");

	//	일반 색상
	//if(!bNextLevel)
		wstrTextOut += TTW(40011);	//폰트 초기화
	// else
		// wstrTextOut += TTW(226);	//폰트 초기화

//	if(!bNextLevel)
	{
		//	선행스킬
		int kArray[LEAD_SKILL_COUNT] = {0,};
		if (g_kSkillTree.GetNeedSkill(iSkillNo, kArray))
		{
			wstrTextOut += TTW(40130);
			wstrTextOut += _T(" : ");

			for (int i = 0; i < LEAD_SKILL_COUNT; ++i)
			{
				int const iNo = kArray[i];
				if (0!=iNo)
				{
					int const iKey = g_kSkillTree.GetKeySkillNo(iNo);
					PgSkillTree::stTreeNode* pNode = g_kSkillTree.GetNode(iKey);
					if (pNode)
					{
						const CSkillDef* pkLeadDef = pNode->GetSkillDef();
						if (pkLeadDef)
						{
							if (i>0)
							{
								wstrTextOut+=_T(", ");
							}
							if((pNode->IsLearned() || pNode->m_bTemporaryLearned) && pNode->m_ulSkillNo >= iNo)
							{		
								wstrTextOut+=TTW(40106);
							}
							else
							{
								wstrTextOut+=TTW(40105);
							}

							std::wstring const *pLeadName = NULL;

							if(GetDefString(pkLeadDef->NameNo(),pLeadName))
							{
								wstrTextOut += std::wstring(*pLeadName);
								wstrTextOut += L" ";
								wstrTextOut += TTW(224);
								wstrTextOut += std::wstring(BM::vstring(iNo-iKey+1));
							}
						}
					}
				}
			}

			wstrTextOut+=_T('\n');
		}

		//	색상 초기화
//		if(!bNextLevel)
			wstrTextOut += TTW(40011);	//폰트 초기화
		// else
			// wstrTextOut += TTW(226);	//폰트 초기화
	}

	const CONT_DEFSKILL *pkContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pkContDefMap);
//	if( !bNextLevel )
	{
		//	스킬 커맨드
		if (pkContDefMap)
		{
			CONT_DEFSKILL::const_iterator def_it = pkContDefMap->find(iSkillNo);
			if (pkContDefMap->end() != def_it)
			{
				CONT_DEFSKILL::mapped_type kT = (*def_it).second;
				if( 0 != kT.iCmdStringNo )
				{
					wstrTextOut+=TTW(40107);
					wstrTextOut+=_T(" : ");

					const wchar_t *pText = NULL;
					if(GetDefString(kT.iCmdStringNo, pText))
					{
						wstrTextOut+=pText;
					}
					wstrTextOut+=_T("\n");
				}
			}
		}
	}

	//	필요 SP
	if(!bCouple)
	{
		int const iNeed = g_kSkillTree.GetNeedSkillPoint(iSkillNo);

		if(iNeed > 0)
		{
			wstrTextOut+=TTW(40131);
			wstrTextOut+=_T(" : ");
			if (g_kSkillTree.GetRemainSkillPoint() < iNeed)
			{
				wstrTextOut+=TTW(40105);
			}
			swprintf(temp,500,_T("%d\n"),iNeed);			
			wstrTextOut+=temp;
		}

		//	색상 초기화
//		if(!bNextLevel)
			wstrTextOut += TTW(40011);	//폰트 초기화
		// else
			// wstrTextOut += TTW(226);	//폰트 초기화
	}

	//	종류(보류)
//	swprintf(temp,100,_T("%s : \n"),TTW(228).c_str());
//	wstrTextOut+=temp;

	//	소모 MP
	if (EST_PASSIVE != pSkillDef->m_byType)	//패시브 스킬아닌 경우
	{
		if(0<pSkillDef->m_sMP)
		{
			swprintf(temp,500,_T("%s : %d\n"),TTW(40133).c_str(), pSkillDef->m_sMP);
			wstrTextOut+=temp;
		}
		else
		{
			//소모 MP가 MAXMP의 %를 소모 할 경우
			int const iNeedRateMaxMP = pSkillDef->GetAbil(AT_NEED_MAX_R_MP);
			if(0 < iNeedRateMaxMP)
			{
				float const fRateMP = static_cast<float>(iNeedRateMaxMP / 100.0f);
				swprintf(temp,500,_T("%s : %s %.1f%%\n"),TTW(40133).c_str(), TTW(512).c_str(), fRateMP);
				wstrTextOut+=temp;
			}
		}

		if(0<pSkillDef->GetAbil(AT_NEED_AWAKE) || 0<pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE))
		{
			wstrTextOut += TTW(1306);//노랑
			//if(3000<=pSkillDef->GetAbil(AT_NEED_AWAKE) 
			//	|| 6000<=pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE))
			//{// 대량 소모한다 표시
			//	wstrTextOut += TTW(5060);
			//}
			//else
			//{// 그냥 소모 한다 표시
			//	wstrTextOut += TTW(5044);
			//}
			{
				wstrTextOut += TTW(5061);
				wstrTextOut += L" : ";
				BM::vstring vStr;
				if(0 < pSkillDef->GetAbil(AT_NEED_AWAKE)) 
				{
					vStr += pSkillDef->GetAbil(AT_NEED_AWAKE);
				}
				else if(0 < pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE))
				{
					vStr += pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE)/ABILITY_RATE_VALUE;
					vStr += L"%";
				}
				wstrTextOut += static_cast<std::wstring>(vStr);
			}
			wstrTextOut += TTW(40011);
			wstrTextOut += L"\n";
		}
		else
		{
			if(0<pSkillDef->GetAbil(AT_ALL_NEED_AWAKE))
			{
				wstrTextOut += TTW(1306);//노랑
				wstrTextOut += TTW(5045);
				wstrTextOut += TTW(40011);
				wstrTextOut += L"\n";
			}
		}
	}

	// 소모 HP
	if (0 < pSkillDef->m_sHP)
	{
		swprintf(temp,500,_T("%s : %d\n"),TTW(40134).c_str(), pSkillDef->m_sHP);
		wstrTextOut+=temp;		
	}
	else
	{
		//소모 MP가 MAXHP의 %를 소모 할 경우
		int const iNeedRateMaxHP = pSkillDef->GetAbil(AT_NEED_MAX_R_HP);
		if(0 < iNeedRateMaxHP)
		{
			float const fRateHP = static_cast<float>(iNeedRateMaxHP / 100.0f);
			swprintf(temp,500,_T("%s : %s %.1f%%\n"),TTW(40134).c_str(), TTW(511).c_str(), fRateHP);
			wstrTextOut+=temp;
		}
	}

	CSkillDef const* pBackupSkillDef = NULL;
	//실제 공격 스킬은 따로 존재 하는 경우
	int iRealSkill = pSkillDef->GetAbil(AT_CHILD_CAST_SKILL_NO);
	if(0 < iRealSkill)
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pChildSkillDef = kSkillDefMgr.GetDef(iRealSkill);
		if(pChildSkillDef)
		{
			pBackupSkillDef = pSkillDef;
			pSkillDef = pChildSkillDef;
		}
	}

	//데미지를 나눠서 공격하는 스킬은 한발 데미지로 나누어 주어야 한다.
	int iComboCount = pSkillDef->GetAbil(AT_COMBO_HIT_COUNT);

	//	물리대미지
	int iAtk = pSkillDef->GetAbil(AT_I_PHY_ATTACK_ADD_RATE);
	if(0 < iComboCount)
	{
		iAtk /= iComboCount;
	}

	if( 0 < iAtk )
	{
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(40136).c_str(), ((float)(iAtk) * 0.01f));
		wstrTextOut+=temp;
		iAtk = pSkillDef->GetAbil(AT_PHY_ATTACK);
		if(0 < iComboCount)
		{
			iAtk /= iComboCount;
		}

		if( 0 < iAtk )
		{
			swprintf(temp, 499, _T(" + %d"), iAtk);
			wstrTextOut+=temp;
		}
		wstrTextOut+=L"\n";
	}

	//	마법대미지
	int iMAtk = pSkillDef->GetAbil(AT_I_MAGIC_ATTACK_ADD_RATE);
	if(0 < iComboCount)
	{
		iMAtk /= iComboCount;
	}

	if( 0 < iMAtk )
	{
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(40137).c_str(), ((float)(iMAtk) * 0.01f));
		wstrTextOut+=temp;
		iMAtk = pSkillDef->GetAbil(AT_MAGIC_ATTACK);
		if(0 < iComboCount)
		{
			iMAtk /= iComboCount;
		}
		if( 0 < iMAtk )
		{
			swprintf(temp, 499, _T(" + %d"), iMAtk);
			wstrTextOut+=temp;
		}
		wstrTextOut+=L"\n";
	}

	//데미지만 실제 공격 스킬에서 얻어 오고 원래 스킬로 돌려 준다.
	if(NULL != pBackupSkillDef)
	{
		pSkillDef = pBackupSkillDef;
	}

	wstrTextOut += TTW(40011);	//폰트 초기화

	//	색상 초기화
//	if(!bNextLevel)
	{
		wstrTextOut += TTW(40011);	//폰트 초기화
	}
	// else
	// {
		// wstrTextOut += TTW(226);	//폰트 초기화
	// }

	//	제한직업
	/*std::wstring kLimitClass;
	if(MakeSkillLimitClassText(pSkillDef,kLimitClass))
	{
		swprintf(temp,100,_T("%s : %s %s\n"),TTW(229).c_str(),kLimitClass.c_str(),TTW(233).c_str());
		wstrTextOut+=temp;
	}*/

	//	제한레벨
	PgPilot	*pkPlayerPilot = g_kPilotMan.GetPlayerPilot();
	if(pkPlayerPilot)
	{
		if(pkPlayerPilot->GetAbil(AT_LEVEL)<pSkillDef->m_sLevelLimit)	//	빨간색으로 표시하자
		{
			wstrTextOut+=TTW(225);
		}
	}

	if(0<pSkillDef->m_sLevelLimit)
	{
		swprintf(temp,100,_T("%s : %d %s\n"),TTW(230).c_str(),pSkillDef->m_sLevelLimit,TTW(232).c_str());
		wstrTextOut+=temp;
	}

//	if(!bNextLevel)
		wstrTextOut += TTW(40011);	//폰트 초기화
	// else
		// wstrTextOut += TTW(226);	//폰트 초기화


	//	길드 제한 레벨
	CONT_DEF_GUILD_SKILL const *pkDefGuildSkill = NULL;
	g_kTblDataMgr.GetContDef(pkDefGuildSkill);
	if( pkDefGuildSkill
	&&	bGuildSkill )
	{
		CONT_DEF_GUILD_SKILL::const_iterator guildskill_iter = pkDefGuildSkill->find(iSkillNo);
		if( pkDefGuildSkill->end() != guildskill_iter )
		{
			CONT_DEF_GUILD_SKILL::mapped_type const &rkDefGuildSkill = (*guildskill_iter).second;

			if( g_kGuildMgr.Level() < rkDefGuildSkill.sGuildLv )
			{
				wstrTextOut+=TTW(225);//붉은색
			}

			swprintf(temp,100,_T("%s : %d %s\n"),TTW(400824).c_str(),rkDefGuildSkill.sGuildLv,TTW(232).c_str());
			wstrTextOut+=temp;

			/*if( !bNextLevel )*/	{ wstrTextOut += TTW(40011); }	//폰트 초기화
			//else				{ wstrTextOut += TTW(226); }	//폰트 초기화
		}
	}

	//	제한무기
	if (pkContDefMap)
	{
		CONT_DEFSKILL::const_iterator def_it = pkContDefMap->find(iSkillNo);
		if (pkContDefMap->end() != def_it)
		{
			CONT_DEFSKILL::mapped_type kT = (*def_it).second;
			if(kT.iWeaponLimit)
			{
				wstrTextOut+=TTW(40132);
				wstrTextOut+=_T(" : ");
				int iCount = 0;
				for (int i = 0; i< 12; ++i)
				{
					int const iWeaponType = 1<<i;
					if (kT.iWeaponLimit & iWeaponType)
					{ 
						if (iCount>0)
						{
							wstrTextOut+=_T(", ");
						}
						int iType = (int)(NiFastLog(iWeaponType) + 1);
						wstrTextOut+=TTW(2000+iType);
						++iCount;
					}
				}
				//int iType = (int)(NiFastLog(kT.iWeaponLimit) + 1);
				//swprintf(temp,500,_T("%s : %s\n"),TTW(40132).c_str(), TTW(2000+iType).c_str());
				wstrTextOut+=_T("\n");
			}
		}
	}
	
	//	시전 시간
	int const iCast = pSkillDef->GetAbil(AT_CAST_TIME);
	
	if (0<iCast)
	{
		if( 0 < ((iCast % 1000) * 0.01) )
			swprintf(temp,500,_T("%s : %.1f%s\n"),TTW(47).c_str(), iCast*0.001f, TTW(54).c_str());
		else
			swprintf(temp,500,_T("%s : %d%s\n"),TTW(47).c_str(), static_cast<int>(iCast*0.001f), TTW(54).c_str());
	}
	else
	{
		swprintf(temp, 500, _T("%s : %s\n"), TTW(47).c_str(), TTW(251).c_str());
	}
	wstrTextOut += temp;

	//	쿨타임
	int const iCool = pSkillDef->GetAbil(ATS_COOL_TIME);
	if (0<iCool)
	{
		if( 0 < ((iCool % 1000) * 0.01) )
			swprintf(temp,500,_T("%s : %.1f%s\n"),TTW(250).c_str(), iCool*0.001f, TTW(54).c_str());
		else
			swprintf(temp,500,_T("%s : %d%s\n"),TTW(250).c_str(), static_cast<int>(iCool*0.001f), TTW(54).c_str());
		wstrTextOut+=temp;
	}

	int const iMaxTarget = pSkillDef->GetAbil(AT_MAX_TARGETNUM);
	if(1 < iMaxTarget)
	{// 최대타격개체수
		BM::vstring vTemp( TTW(40151) ); 
		vTemp.Replace(L"#NUM#", iMaxTarget);
		vTemp+="\n";
		wstrTextOut+=vTemp.operator const std::wstring &();
	}
	
	int const iMaxPenetration = pSkillDef->GetAbil(AT_PENETRATION_COUNT);
	if(0 < iMaxPenetration)
	{// 최대관통개체수
		BM::vstring vTemp( TTW(40152) ); 
		vTemp.Replace(L"#NUM#", iMaxPenetration);
		vTemp+="\n";
		wstrTextOut+=vTemp.operator const std::wstring &();
	}

	bool bCantBlockTxt = false;
	//지속 시간
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	const	CEffectDef* pEffDef = kEffectDefMgr.GetDef(pSkillDef->No());
	if (pEffDef)
	{
		int const iDuration = pEffDef->GetDurationTime()/1000;
		if (0<iDuration)
		{
			swprintf(temp,500,_T("%s : "),TTW(40135).c_str());
			wstrTextOut+=temp;
			GetTimeString(iDuration, wstrTextOut, true);
		}
		if( pEffDef->GetAbil(AT_CAN_NOT_BLOCK_BY_EFFECT) )
		{
			bCantBlockTxt = true;
		}
	}

	//	설명
	const TCHAR *pText = NULL;
	int Value = pSkillDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		if (strlenT(pText))
		{
			swprintf(temp,100,_T("\n%s : \n"),TTW(231).c_str());
			wstrTextOut+= ::TTW(750010);	//노랑
			wstrTextOut+=temp;
			wstrTextOut+= ::TTW(1554);		//흰색
			wstrTextOut+=pText;
		}
	}

	if( !bCantBlockTxt
		&& 0 < pSkillDef->GetAbil(AT_CAN_NOT_BLOCK_BY_ACTION) 
		)
	{
		bCantBlockTxt = true;
	}
	if( bCantBlockTxt )
	{// 블록 할 수 없습니다.
		wstrTextOut += ENTER_STRING; 
		wstrTextOut += ENTER_STRING;
		wstrTextOut += TTW(750010);	// 컬러(노랑), 폰트
		wstrTextOut += TTW(791611);
	}

	return	true;
}	

bool MakeNpcToolTipText(int iIconKey, std::wstring &wstrTextOut)
{
	const wchar_t *pText = NULL;
	if(GetDefString(iIconKey, pText))
	{
		// Name
		wstrTextOut += TTW(40002);
		wstrTextOut += pText;

		return true;
	}

	return false;
}

//#define PG_USE_TOOLTIP_MON_DETAIL
bool MakeMonsterToolTipText(int iIconKey, std::wstring &wstrTextOut, std::wstring & wstrLank)
{
	if (!iIconKey)
	{
		return false;
	}

	GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
	const CMonsterDef *pDef = kMonsterDefMgr.GetDef(iIconKey);
	if (!pDef)
	{
		// NPC.
		return MakeNpcToolTipText(iIconKey, wstrTextOut);
	}

	// Name
	wstrTextOut += TTW(40002);
	const wchar_t *pName = NULL;
	GetDefString(pDef->NameNo(), pName);
	wstrTextOut += std::wstring((pName ? pName : _T("")));

	//
	wstrLank += TTW(7516);
	wstrLank += L"[";
	wstrLank += TTW(3350+pDef->GetAbil(AT_GRADE));
	wstrLank += L"]";
	//

	//
	wstrTextOut += _T("\n");
	wstrTextOut += _T("\n");
	wstrTextOut += TTW(40009);
	wstrTextOut += BM::vstring(pDef->GetAbil(AT_LEVEL)).operator std::wstring const&();
	//

	int const iDescr = pDef->GetAbil(ATI_EXPLAINID);
	if(0<iDescr)
	{
		const wchar_t *pDescr = NULL;
		if(GetDefString(iDescr, pDescr) && pDescr)
		{
			wstrTextOut += _T("\n");
			wstrTextOut += _T("\n");
			wstrTextOut += TTW(40050);
			wstrTextOut += TTW(40051);
			wstrTextOut += pDescr;
		}
	}

#ifdef PG_USE_TOOLTIP_MON_DETAIL
	wstrTextOut += TTW(40011);	//폰트 초기화
	wstrTextOut += _T("\n");
	wstrTextOut += _T("\n");

	// 정예, 보스들은 능력치를 보여주지 않는다.
	int iGrade = pDef->GetAbil(AT_GRADE);
	if (iGrade == EMGRADE_ELITE ||
		iGrade == EMGRADE_BOSS )
	{
		std::wstring wstrQue = _T(": ???");
		GetDefString(7001,pName);
		wstrTextOut += pName;
		wstrTextOut += _T(": ???\n");
		GetDefString(7021,pName);
		wstrTextOut += pName;
		wstrTextOut += _T(": ???\n");
		GetDefString(7223,pName);
		wstrTextOut += pName;
		wstrTextOut += _T(": ???\n");
		GetDefString(7091,pName);
		wstrTextOut += pName;
		wstrTextOut += _T(": ???\n");
		GetDefString(7281,pName);
		wstrTextOut += pName;
		wstrTextOut += _T(": ???\n");
		GetDefString(7101,pName);
		wstrTextOut += pName;
		wstrTextOut += _T(": ???\n");

		return true;
	}

	GetDefString(7001,pName);
	wstrTextOut += pName;
	wstrTextOut += _T(": ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_MAX_HP)));		// Basic Max HP (DB value)
	wstrTextOut += _T("\n");
	//wstrTextOut += _T("HP Recovery Interval: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_HP_RECOVERY_INTERVAL)));	// Basic HP Recovery Speed
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("HP Recovery: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_HP_RECOVERY)));
	//wstrTextOut += _T("\n");
	GetDefString(7021,pName);
	wstrTextOut += pName;
	wstrTextOut += _T(": ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_MAX_MP)));		// Basic Max MP
	wstrTextOut += _T("\n");
	//wstrTextOut += _T("MP Recovery Interval: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_MP_RECOVERY_INTERVAL)));	// Basic MP Recovery Speed
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("MP Recovery: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_MP_RECOVERY)));
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("힘: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_STR)));			// Basic Strength
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("지력: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_INT)));			// Basic Intelligence
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("체질: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_CON)));			// Basic Constitution
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("솜씨: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_DEX)));			// Basic Dexterity
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("이동 속도: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_MOVESPEED)));		// Basic Move speed
	//wstrTextOut += _T("\n");
	GetDefString(7223,pName);
	wstrTextOut += pName;
	wstrTextOut += _T(": ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_PHY_ATTACK_MAX)));	// Basic Physics Defence
	wstrTextOut += _T("\n");
	GetDefString(7091,pName);
	wstrTextOut += pName;
	wstrTextOut += _T(": ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_PHY_DEFENCE)));	// Basic Physics Defence
	wstrTextOut += _T("\n");
	GetDefString(7281,pName);
	wstrTextOut += pName;
	wstrTextOut += _T(": ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_MAGIC_ATTACK_MAX)));	// Basic Magic Defence
	wstrTextOut += _T("\n");
	GetDefString(7101,pName);
	wstrTextOut += pName;
	wstrTextOut += _T(": ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_MAGIC_DEFENCE)));	// Basic Magic Defence
	wstrTextOut += _T("\n");
	//wstrTextOut += _T("공격 속도: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_ATTACK_SPEED)));		// Basic Attack speed
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("Block Attack Rat: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_BLOCK_SUCCESS_VALUE)));		// Basic Block-attack rate
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("Dodge Attack Rate: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_DODGE_SUCCESS_VALUE)));		// Basic Dodge-attack rate
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("Critical Rate: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_CRITICAL_SUCCESS_VALUE)));	// AT_CRITICAL_SUCCESS_VALUE : Basic Critical Rate
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("Critical Power: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_CRITICAL_POWER)));	// AT_CRITICAL_POWER : Basic Critical Power
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("Hit Rate: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_HIT_SUCCESS_VALUE)));
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("AI Type: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_AI_TYPE)));	// AT_AI_TYPE : AI Type
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("Inven Size: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_INVEN_SIZE)));		// AT_INVEN_SIZE
	//wstrTextOut += _T("\n");
	//wstrTextOut += _T("Equips Size: ") + (std::wstring)(BM::vstring(pDef->GetAbil(AT_EQUIPS_SIZE)));		// AT_EQUIPS_SIZE
#endif

#ifndef EXTERNAL_RELEASE
	wstrTextOut += _T("\n");
	wstrTextOut += _T("\n");
	wstrTextOut += _T("NUMBER : ") + (std::wstring)(BM::vstring(iIconKey));
#endif
	return true;
}

void CallMonsterToolTip(int iIconKey, lwPoint2 &pt)
{
	std::wstring wstrText1;
	std::wstring wstrLink;
	if (MakeMonsterToolTipText(iIconKey, wstrText1, wstrLink))
	{
		lwCallToolTipByText(0, wstrText1, pt, NULL, 0, MB(wstrLink));
	}
	//if(MakeSkillToolTipText(pTreeNode, wstrText1, false, wstrType))
	//{
	//	if(MakeSkillToolTipText(pTreeNode, wstrText2,true, wstrType))
	//	{
	//		wstrText1+=_T('\n');
	//		wstrText1+=_T('\n');
	//		wstrText1+=_T('\n');
	//		wstrText1+=wstrText2;

	//		lwCallToolTipByText(0, wstrText1, pt);
	//	}
	//}
	return;
}

void CallGuardianToolTip(int iIconKey, lwPoint2 &pt)
{
	std::wstring wstrText1;
	if (g_kMissionMgr.MakeGuardianToolTipText(iIconKey, wstrText1))
	{
		lwCallToolTipByText(0, wstrText1, pt);
	}
	return;
}

void CallStratagySkillToolTip(int const iGuardianNo, int const iSkillNo, lwPoint2 &pt)
{
	std::wstring wstrText1,wstrText2;
	std::wstring wstrType;
	if(g_kMissionMgr.MakeStratagySkillToolTipText(iGuardianNo, iSkillNo, wstrText1, wstrType, false))
	{// 해당 스킬만 보여줌
		if(g_kMissionMgr.MakeStratagySkillToolTipText(iGuardianNo, iSkillNo, wstrText2, wstrType, true))
		{
			wstrText1+=_T("\n\n\n");
			wstrText1+=wstrText2;
		}
		
		lwCallToolTipByText(0, wstrText1, pt, "ToolTip_Skill", 0, MB(wstrType));
	}
	return;
}
void CallJobSkillToolTip(int iSkillNo, lwPoint2 &pt)
{
	PgPlayer const* pPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pPlayer)
	{
		return;
	}
	//일단 배웠는지 안배웠는지 체크
	bool const bLearned = JobSkill_LearnUtil::IsEnableUseJobSkill(pPlayer, iSkillNo);
	PgJobSkillExpertness const& rkExpertness =  pPlayer->JobSkillExpertness();
	//직업기술 타입
	EJobSkillType const eJobSkillType =  JobSkill_Util::GetJobSkillType(iSkillNo);

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	const CSkillDef	*pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	if( !pkSkillDef )
	{
		return;
	}

	CONT_DEF_JOBSKILL_SKILL const* pkContDefJobSkill;
	g_kTblDataMgr.GetContDef(pkContDefJobSkill);
	if(0 == pkContDefJobSkill->size())
	{
		return;
	}
	CONT_DEF_JOBSKILL_SKILL::const_iterator iter_Skill = pkContDefJobSkill->find(iSkillNo);
	if( iter_Skill == pkContDefJobSkill->end() )
	{
		return;
	}
	CONT_DEF_JOBSKILL_SKILL::mapped_type const &rkJobSkill = (*iter_Skill).second;
	
	std::wstring wstrTextOut;
	std::wstring wstrType;
	bool bMaxExpertness = false;	//최대숙련도까지 배웠는지 체크 - 숙련도 표기부분에서 체크
	//이름
	const wchar_t *pNameText = NULL;
	if(GetDefString(pkSkillDef->NameNo(), pNameText))
	{
		// Name
		wstrTextOut += TTW(40102);
		wstrTextOut += pNameText;
#ifndef USE_INB
		if( g_pkApp->VisibleClassNo() )
		{
			BM::vstring kSkillNo(iSkillNo);
			wstrTextOut += _T("\n SkillNo : ") + static_cast<std::wstring>(kSkillNo);
		}
#endif
		wstrTextOut += TTW(40011);	//폰트 초기화
		wstrTextOut += ENTER_STRING;
		wstrTextOut += ENTER_STRING;
	}
		//필요기술 : [채집기술](주스킬이름)
	{
		BM::vstring kGatherType;
		GetJobSkillText_GatherType(iSkillNo, kGatherType);
		if( 0 == kGatherType.size() )
		{
			return;
		}
		//주스킬 이름
		BM::vstring kNeedSkill(TTW(799632));
		kNeedSkill.Replace( L"#GATHER#", BM::vstring(kGatherType) );
		wstrType += static_cast<std::wstring>(kNeedSkill);
		wstrTextOut += ENTER_STRING;
	}
	//배웠으면 현재숙련도
	if(bLearned)
	{
		BM::vstring kExpertness(TTW(799625));
		int const iCurExpertness = rkExpertness.Get(iSkillNo);
		int const iMaxExpertness = JobSkillExpertnessUtil::GetBiggestMaxExpertness(iSkillNo);
		if( iMaxExpertness )
		{
			if( g_pkApp->VisibleClassNo() )
			{
				kExpertness.Replace(L"#NOW#", iCurExpertness);
				kExpertness.Replace(L"#MAX#", iMaxExpertness);
			}
			else
			{
				kExpertness.Replace(L"#NOW#", iCurExpertness / ABILITY_RATE_VALUE);
				kExpertness.Replace(L"#MAX#", iMaxExpertness / ABILITY_RATE_VALUE);
			}
			wstrTextOut += static_cast<std::wstring>(kExpertness);
			wstrTextOut += ENTER_STRING;
		}
		if(iCurExpertness == iMaxExpertness)
		{
			bMaxExpertness = true;
		}
	}
	//기본시전시간
	{
		BM::vstring kTurnTime(TTW(799626));
		int const iBasicTurnTime = JobSkillExpertnessUtil ::GetBasicTurnTime(iSkillNo, rkExpertness.Get(iSkillNo));
		if( 0 != iBasicTurnTime)
		{
			kTurnTime.Replace( L"#SEC#", iBasicTurnTime / JSE_SKILL_TIME_DEVIDE );
			kTurnTime += TTW(9);//초
			wstrTextOut += static_cast<std::wstring>(kTurnTime);
			wstrTextOut += ENTER_STRING;
		}
		else
		{
			kTurnTime.Replace( L"#SEC#", TTW(251) );
			wstrTextOut += static_cast<std::wstring>(kTurnTime);
			wstrTextOut += ENTER_STRING;
		}
	}
	//배웠으면 재사용시간
	if(bLearned)
	{
		BM::vstring kCoolTime(TTW(799627));
		int const iCoolTime = pkSkillDef->GetAbil(ATS_COOL_TIME);
		if( 0 != iCoolTime)
		{
			kCoolTime.Replace( L"#SEC#", iCoolTime / JSE_SKILL_TIME_DEVIDE );
			kCoolTime += TTW(9);//초
			wstrTextOut += static_cast<std::wstring>(kCoolTime);
			wstrTextOut += ENTER_STRING;
		}
	}
	//배웠으면 소모피로도
	if(bLearned)
	{
		BM::vstring kUseExhaustion;
		if( eJobSkillType == JST_2ND_MAIN )
		{
			kUseExhaustion = TTW(799636);
		}
		else
		{
			kUseExhaustion = TTW(799628);
		}

		int const iUseExhaustion = JobSkillExpertnessUtil::GetUseExhaustion(iSkillNo, rkExpertness.Get(iSkillNo));
		if( 0 < iUseExhaustion)
		{
			kUseExhaustion.Replace( L"#NUM#", iUseExhaustion );
			wstrTextOut += static_cast<std::wstring>(kUseExhaustion);
			wstrTextOut += ENTER_STRING;
		}
	}
	//발생 확률
	{
		int const iExpertness = rkExpertness.Get(iSkillNo);
		CONT_DEF_JOBSKILL_SKILLEXPERTNESS const* pkContDefJobSkillExpertness;
		g_kTblDataMgr.GetContDef(pkContDefJobSkillExpertness);
		CONT_DEF_JOBSKILL_SKILLEXPERTNESS::value_type rkExpertnessArea;
		JobSkillExpertnessUtil::FindExpertnessArea(iSkillNo, iExpertness, *pkContDefJobSkill, *pkContDefJobSkillExpertness, rkExpertnessArea);
		BM::vstring kUseProbability(TTW(799633));
		if( 0 != rkExpertnessArea.iSkill_Probability)
		{
			kUseProbability.Replace( L"#PER#", ( (rkExpertnessArea.iSkill_Probability * 100) / JSE_SKILL_PROBABILITY_DEVIDE ) );
			wstrTextOut += static_cast<std::wstring>(kUseProbability);
			wstrTextOut += ENTER_STRING;
		}
	}
	//필요도구 : ??류 (테이블 필드 작업 필요)
	{
		BM::vstring kNeedTool(TTW(799629));
		BM::vstring kToolName;
		if( GetJobSkillText_ToolType(rkJobSkill.iGatherType, iSkillNo, kToolName) )
		{
			kNeedTool.Replace(L"#TOOL#", kToolName);
			wstrTextOut += static_cast<std::wstring>(kNeedTool);
			wstrTextOut += ENTER_STRING;
		}
	}
	wstrTextOut += ENTER_STRING;
	//배웠고, 숙련도 만땅 아니면 숙련도 설명 (테이블 필드 작업 필요)
	if( (bLearned) 
		&& (!bMaxExpertness)
		&& (JST_1ST_SUB != rkJobSkill.eJobSkill_Type) )
	{
		CONT_DEF_JOBSKILL_SKILLEXPERTNESS::value_type rkNextExpertness;
		int const iNexExpertness_Min = JobSkillExpertnessUtil::FindNextExpertness_Min(iSkillNo, rkExpertness.Get(iSkillNo));
		JobSkillExpertnessUtil::FindExpertnessArea(iSkillNo, iNexExpertness_Min, rkNextExpertness);
		if( iNexExpertness_Min )
		{
			BM::vstring kInfoExpertness(TTW(799630));
			wstrTextOut += TTW(40011);	//폰트 초기화
			wstrTextOut += static_cast<std::wstring>(kInfoExpertness);
			BM::vstring kInfoText(TTW(799631));

			kInfoText.Replace( L"#NUM#", iNexExpertness_Min / ABILITY_RATE_VALUE);
			kInfoText.Replace( L"#LEVEL#", rkNextExpertness.iNeed_CharLevel);
			wstrTextOut += static_cast<std::wstring>(kInfoText);
			wstrTextOut += ENTER_STRING;
		}
	}
	wstrTextOut += ENTER_STRING;
	if( (!bLearned) && (JST_1ST_SUB == rkJobSkill.eJobSkill_Type) )
	{
		BM::vstring kNeedExpertness(TTW(799635));
		wstrTextOut += static_cast<std::wstring>(kNeedExpertness);
		wstrTextOut += TTW(40011);	//폰트 초기화
		BM::vstring kInfoText(TTW(799536));

		kInfoText.Replace( L"#NEED#", rkJobSkill.i01NeedParent_JobSkill_Expertness / JSE_SKILL_PROBABILITY_DEVIDE);
		wstrTextOut += static_cast<std::wstring>(kInfoText);
		wstrTextOut += ENTER_STRING;
		wstrTextOut += ENTER_STRING;
	}
	//설명
	const TCHAR *pExplainText = NULL;
	int Value = pkSkillDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pExplainText))
	{
		wstrTextOut += TTW(40050);
		wstrTextOut += TTW(40051);
		wstrTextOut += pExplainText;
		wstrTextOut += _T("\n");
	}

	lwCallToolTipByText(0, wstrTextOut, pt, "ToolTip_Skill", 0, MB(wstrType));
}
void MakeToolTipText_JobSkill_Tool(PgBase_Item const &kItem, std::wstring &wstrTextOut, std::wstring &wstrType, TBL_SHOP_IN_GAME const &kShopItemInfo)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( !pDef )
	{
		return;
	}
	CONT_DEF_JOBSKILL_TOOL::mapped_type kToolInfo;
	if( !JobSkillToolUtil::GetToolInfo( kItem.ItemNo(), kToolInfo ) )
	{// 도구 정보를 찾을 수 없음
		return;
	}
	//이름
	const wchar_t *pNameText = NULL;
	if(GetDefString(pDef->NameNo(), pNameText))
	{
		// Name
		wstrTextOut += TTW(40102);
		wstrTextOut += pNameText;
#ifndef USE_INB
		if(g_pkApp->VisibleClassNo())
		{
			BM::vstring kClassNo(kItem.ItemNo());
			wstrTextOut += (std::wstring)kClassNo;
		}
#endif
		wstrTextOut += TTW(40011);	//폰트 초기화
		wstrTextOut += ENTER_STRING;
	}
	//[채집도구]
	{
		wstrType += TTW(40011);
		wstrType += TTW(799600);
		BM::vstring kToolType;
		GetJobSkillText_ToolType(kItem.ItemNo(), kToolType);
		if( 0 == kToolType.size() )
		{
			return;
		}
		//[??류]
		BM::vstring kstrTool(TTW(799601));
		kstrTool.Replace(L"#TOOL#", kToolType);
		wstrType += static_cast<std::wstring>(kToolType);
	}
	wstrTextOut += ENTER_STRING;
	wstrTextOut += ENTER_STRING;
	//필요기술 : [채집기술](주스킬이름)
	if( 0 != kToolInfo.i01Need_Skill_No )
	{
		wstrTextOut += TTW(799610);
		wstrTextOut += WSTR_ATTR_EXPLAIN;
		BM::vstring kGatherType;
		GetJobSkillText_GatherType(kToolInfo.i01Need_Skill_No, kGatherType);
		if( 0 == kGatherType.size() )
		{
			return;
		}
		//주스킬 이름
		wchar_t const* pName = NULL;
		if(!GetDefString(kToolInfo.i01Need_Skill_No,pName))
		{
			return;
		}
		PgPlayer const* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return;
		}
		bool const bUse = JobSkill_LearnUtil::IsEnableUseJobSkill(pPlayer, kToolInfo.i01Need_Skill_No);
		if(!bUse)
		{
			wstrTextOut += TTW(CONTENTS_COLOR_RED);
		}
		BM::vstring kNeedSkill(TTW(799617));
		kNeedSkill.Replace( L"#GATHER#", BM::vstring(kGatherType) );
		kNeedSkill.Replace( L"#NAME#", BM::vstring(pName) );
		wstrTextOut += static_cast<std::wstring>(kNeedSkill);
		if(!bUse)
		{
			wstrTextOut += TTW(40011);	//폰트 초기화
		}
		wstrTextOut += ENTER_STRING;
	}
	//착용위치
	{
		EEquipPos const eEquipPos = static_cast<EEquipPos>(pDef->EquipPos());
		BM::vstring kEquipPos(TTW(799634));
		switch(eEquipPos)
		{
		case EQUIP_POS_WEAPON:
			{
				kEquipPos.Replace(L"#EQUIP#", TTW(70006));
			}break;
		case EQUIP_POS_SHEILD:
			{
				kEquipPos.Replace(L"#EQUIP#", TTW(70007));
			}break;
		}
		wstrTextOut += static_cast<std::wstring>(kEquipPos);;
		wstrTextOut += ENTER_STRING;
	}
	//내구도
	{			
		int const iCurDurability = ((kItem.EnchantInfo().IsNeedRepair())?0:kItem.Count());//수리요구면 내구도를 0으로 표시.
		wstrTextOut += TTW(50411);//내구도						

		wstrTextOut += WSTR_ATTR_EXPLAIN;
		wstrTextOut += (std::wstring const&)(BM::vstring)iCurDurability;

		if(!pDef->CanConsume())
		{//사용 불가능이면 Max 내구 표시.
			int const iMaxDurability = kItem.MaxDuration();// pkItemDef->MaxAmount();
			wstrTextOut += _T("/");

			if(iMaxDurability != pDef->MaxAmount())
			{
				wstrTextOut += TTW(89999);//빨강
				wstrTextOut += (std::wstring const&)(BM::vstring)iMaxDurability;
				wstrTextOut += TTW(40011);
			}
			else
			{
				wstrTextOut += (std::wstring const&)(BM::vstring)iMaxDurability;
			}
		}
		wstrTextOut += ENTER_STRING;
		wstrTextOut += ENTER_STRING;
	}
	//-옵션
	{
		wstrTextOut += TTW(40020);
		wstrTextOut += TTW(40021);
		//1회 획득량 : ??~??개
		{
			BM::vstring kGetCount(TTW(799611)); 
			kGetCount.Replace( L"#MIN_COUNT#", BM::vstring(kToolInfo.iGetCount) );
			kGetCount.Replace( L"#MAX_COUNT#", BM::vstring(kToolInfo.iGetCountMax) );
			wstrTextOut += static_cast<std::wstring>(kGetCount);
			wstrTextOut += ENTER_STRING;
		}
		//기술 쿨타임 감소 : ??
		if( 0 != kToolInfo.iOption_CoolTime )
		{
		BM::vstring kCoolTime(TTW(799612)); 
		float fCoolTime = kToolInfo.iOption_CoolTime / JSE_SKILL_TIME_DEVIDE;
		kCoolTime.Replace( L"#SEC#", BM::vstring( fCoolTime, L"%.1f") );
		wstrTextOut += static_cast<std::wstring>(kCoolTime);
		wstrTextOut += ENTER_STRING;
		}
		//기술 사용시간 감소
		if( 0 != kToolInfo.iOption_TurnTime )
		{
		BM::vstring kTurnTime(TTW(799613)); 
		float fTurnTime = kToolInfo.iOption_TurnTime / JSE_SKILL_TIME_DEVIDE;
		kTurnTime.Replace( L"#SEC#", BM::vstring( fTurnTime, L"%.1f") );
		wstrTextOut += static_cast<std::wstring>(kTurnTime);
		wstrTextOut += ENTER_STRING;
		}
		//소모 피로도 감소 :
		if( 0 != kToolInfo.iOption_User_Exhaustion )
		{
		BM::vstring kExhaustion(TTW(799614)); 
		kExhaustion.Replace( L"#PER#", BM::vstring( kToolInfo.iOption_User_Exhaustion / JSE_EXHAUSTION_RATE_DEVIDE, L"%d") );
		wstrTextOut += static_cast<std::wstring>(kExhaustion);
		wstrTextOut += ENTER_STRING;
		}
	}
	MakeBaseAbilStr(kItem, pDef, wstrTextOut);
	wstrTextOut += ENTER_STRING;

	//다음 행동이 제한됨	
	int	const iCantAbil = pDef->GetAbil(AT_ATTRIBUTE);
	if(iCantAbil)
	{
		wstrTextOut += TTW(TITLE_COLOR_YELLOW);
		wstrTextOut += TTW(2501);
		wstrTextOut += TTW(40011);	//폰트 초기화
		wstrTextOut += ENTER_STRING;

		std::wstring	wstrCantAbil;
		GetCantAbilString(iCantAbil,wstrCantAbil);

		wstrTextOut += TTW(799615);	//도구 툴팁엔 무조건 전투불가/수리불가 추가
		if( 0 != wstrCantAbil.size() )
		{//제한 어빌이 있다면 "/" 추가
			wstrTextOut += L"/";
		}

		wstrTextOut += wstrCantAbil;
		wstrTextOut += ENTER_STRING;
		wstrTextOut += ENTER_STRING;
	}
	//설명
	const TCHAR *pExplainText = NULL;
	int Value = pDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pExplainText))
	{
		wstrTextOut += TTW(40050);
		wstrTextOut += TTW(40051);
		wstrTextOut += pExplainText;
		wstrTextOut += _T("\n");
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 골동품 교환 가능한 아이템 한정
	if( lwDefenceMode::IsCanChangeAntique(EGT_ANTIQUE, kItem.ItemNo()) )
	{
		wstrTextOut += ENTER_STRING;
		wstrTextOut += TTW(750010);	// 컬러(노랑), 폰트
		wstrTextOut += TTW(750011);
		wstrTextOut += ENTER_STRING;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지

	//가격
	if(!kShopItemInfo.IsNull())
	{
		std::wstring kPrice;

		if( kShopItemInfo.iPrice )
		{
			//FormatMoney(kShopItemInfo.iPrice, kPrice);
			kPrice = GetMoneyString(kShopItemInfo.iPrice);
		}
		else
		{
			wchar_t	szCPValue[30] = {0,};
			swprintf(szCPValue, 29, TTW(5006).c_str(), kShopItemInfo.iCP * 10);
			kPrice = szCPValue;
			
		}

		wstrTextOut += _T("\n");
		wstrTextOut += TTW(40052);
		wstrTextOut += TTW(40053);	
		wstrTextOut += kPrice;
		wstrTextOut += _T("\n");//Pg2DString에서 높이를 계산하는 공식이 마지막줄의 폰트 크기와 줄바꿈 횟수를 곱해서 문제가 발생함 그걸 위한 임시 방편.
	}
	else
	{
		bool const bIsStockShopOn = (!lwGetUIWnd("FRM_SHOP_STOCK").IsNil());
		if( !lwGetUIWnd("FRM_SHOP").IsNil() 
		||	!lwGetUIWnd("FRM_SHOP_CP").IsNil()
		||	!lwGetUIWnd("FRM_SHOP_JOB").IsNil()
		||	bIsStockShopOn)
		{
			int iSellPrice = pDef->SellPrice();

			if(bIsStockShopOn)
			{
				PgStore::CONT_GODDS::value_type kItemInfo;
				if(S_OK == g_kViewStore.GetGoods(kItem.ItemNo(), kItemInfo))
				{
					iSellPrice = kItemInfo.iSellPrice;
				}
			}

			if(iSellPrice)
			{
				__int64 iEarnPrice = 0;
				CalcSellMoney(kItem, iEarnPrice, iSellPrice, kItem.Count(), g_kPilotMan.GetPlayerUnit()->GetPremium());

				std::wstring kPrice;
				//FormatMoney(iEarnPrice, kPrice);
				kPrice = GetMoneyString(iEarnPrice);

				wstrTextOut += _T("\n");
				wstrTextOut += TTW(40052);
				wstrTextOut += TTW(40053);
				wstrTextOut += kPrice;
				wstrTextOut += _T("\n");
			}
			else
			{
				wstrTextOut += _T("\n");
				wstrTextOut += TTW(40105);
				wstrTextOut += TTW(5014);
				wstrTextOut += _T("\n");
			}
		}
	}
}
void MakeToolTipText_JobSkill_SaveIdx(PgBase_Item const &kItem, std::wstring &wstrTextOut, std::wstring &wstrType, TBL_SHOP_IN_GAME const &kShopItemInfo, SToolTipFlag const &kFlag)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( !pDef )
	{
		return;
	}

	CONT_DEF_JOBSKILL_SAVEIDX::mapped_type rkSaveIdx;
	if( false == JobSkill_Util::GetJobSkill_SaveIndex(kItem.ItemNo(), rkSaveIdx) )
	{
		return;
	}
	//이름
	const wchar_t *pNameText = NULL;
	if(GetDefString(pDef->NameNo(), pNameText))
	{
		// Name
		wstrTextOut += TTW(40102);
		wstrTextOut += pNameText;
#ifndef USE_INB
		if(g_pkApp->VisibleClassNo())
		{
			BM::vstring kClassNo(kItem.ItemNo());
			wstrTextOut += (std::wstring)kClassNo;
		}
#endif
		wstrTextOut += TTW(40011);	//폰트 초기화
		wstrTextOut += ENTER_STRING;
	}
	//요구레벨
	int const iLevelLimit = pDef->GetAbil(AT_LEVELLIMIT);
	if( 0 != iLevelLimit)
	{
		wstrTextOut += ENTER_STRING;
		BM::vstring kLevelLimit(TTW(799619));
		kLevelLimit.Replace( L"#LEVEL#", iLevelLimit);
		wstrTextOut += static_cast<std::wstring>(kLevelLimit);
	
	}
	//필요직업기술
	{
		BM::vstring kGatherType;
		GetJobSkillText_GatherType(rkSaveIdx.iNeedSkillNo01, kGatherType);
		if( 0 == kGatherType.size() )
		{
			return;
		}
		//주스킬 이름
		wchar_t const* pName = NULL;
		if(!GetDefString(rkSaveIdx.iNeedSkillNo01,pName))
		{
			return;
		}
		PgPlayer const* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return;
		}
		bool bCantUse = false;
		if(false == JobSkill_LearnUtil::IsEnableUseJobSkill(pPlayer, rkSaveIdx.iNeedSkillNo01) )
		{
			bCantUse = true;
		}
		int const iMyExpertness = pPlayer->JobSkillExpertness().Get(rkSaveIdx.iNeedSkillNo01);
		if( iMyExpertness < rkSaveIdx.iNeedSkillExpertness01 )
		{
			bCantUse = true;
		}
		if(bCantUse)
		{
			wstrTextOut += TTW(CONTENTS_COLOR_RED);
		}
		wstrTextOut += ENTER_STRING;
		BM::vstring kNeedSkill(TTW(799620));
		kNeedSkill.Replace( L"#GATHER#", BM::vstring(kGatherType) );
		kNeedSkill.Replace( L"#SKILL#", BM::vstring(pName) );
		kNeedSkill.Replace( L"#EXP#",  rkSaveIdx.iNeedSkillExpertness01/ABILITY_RATE_VALUE);
		wstrTextOut += static_cast<std::wstring>(kNeedSkill);
		wstrTextOut += ENTER_STRING;
		wstrTextOut += ENTER_STRING;
		if(bCantUse)
		{
			wstrTextOut += TTW(40011);	//폰트 초기화
		}

		//채집3차 재료
		if( rkSaveIdx.iRecipeItemNo )
		{
			CONT_DEF_JOBSKILL_RECIPE::mapped_type kJobSkillRecipe;
			if( JobSkill_Third::GetJobSkill3_Recipe(rkSaveIdx.iRecipeItemNo, kJobSkillRecipe) )
			{
				CONT_DEF_JOBSKILL_SKILL const* pkDefJobSkill = NULL;
				CONT_DEF_JOBSKILL_SKILLEXPERTNESS const* pkDefJobSkillExpertness = NULL;
				g_kTblDataMgr.GetContDef(pkDefJobSkill);
				g_kTblDataMgr.GetContDef(pkDefJobSkillExpertness);

				int const iExpertness = pPlayer->JobSkillExpertness().Get(kJobSkillRecipe.iNeedSkillNo);
				int const iOrgAddExpertness = JobSkillExpertnessUtil::GetExpertnessUp(kJobSkillRecipe.iNeedSkillNo, iExpertness, *pkDefJobSkill, *pkDefJobSkillExpertness);
				float const fRate = static_cast<float>(kJobSkillRecipe.iExpertnessGain)/JSE_EXPERTNESS_DEVIDE;

				int const iAddExpertness = iOrgAddExpertness * fRate;;


				wchar_t pkTemp[MAX_PATH] = {0,};

				BM::vstring vStr;

				wstrTextOut += ENTER_STRING;
				vStr = TTW(799657);
				vStr.Replace( L"#VALUE#", kJobSkillRecipe.iNeedProductPoint );
				wstrTextOut += static_cast<std::wstring>(vStr);
		
				wstrTextOut += ENTER_STRING;
				vStr = TTW(799658);
				::swprintf_s( pkTemp, MAX_PATH, L"%.2f", MAKE_ABIL_RATE(fRate) );
				vStr.Replace( L"#VALUE#", pkTemp );
				wstrTextOut += static_cast<std::wstring>(vStr);

				wstrTextOut += ENTER_STRING;
				vStr = TTW(799677);
				::swprintf_s( pkTemp, MAX_PATH, L"%.2f", iAddExpertness/ABILITY_RATE_VALUE_FLOAT );
				vStr.Replace( L"#VALUE#", pkTemp );
				wstrTextOut += static_cast<std::wstring>(vStr);
				wstrTextOut += ENTER_STRING;
			}
		}
	}
	
	//채집3차 재료
	if( MakeTextJobSkill3Resource(rkSaveIdx.iRecipeItemNo, wstrTextOut) )
	{
		wstrTextOut += ENTER_STRING;
		wstrTextOut += ENTER_STRING;
	}

	//설명
	const TCHAR *pExplainText = NULL;
	int Value = pDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pExplainText))
	{
		wstrTextOut += TTW(40050);
		wstrTextOut += TTW(40051);
		wstrTextOut += pExplainText;
		wstrTextOut += _T("\n");
	}
	//가격
	if(!kShopItemInfo.IsNull())
	{
		std::wstring kPrice;

		if( kShopItemInfo.iPrice )
		{
			//FormatMoney(kShopItemInfo.iPrice, kPrice);
			kPrice = GetMoneyString(kShopItemInfo.iPrice);
		}
		else
		{
			wchar_t	szCPValue[30] = {0,};
			swprintf(szCPValue, 29, TTW(5006).c_str(), kShopItemInfo.iCP * 10);
			kPrice = szCPValue;
			
		}

		wstrTextOut += _T("\n");
		wstrTextOut += TTW(40052);
		wstrTextOut += TTW(40053);	
		wstrTextOut += kPrice;
		wstrTextOut += _T("\n");//Pg2DString에서 높이를 계산하는 공식이 마지막줄의 폰트 크기와 줄바꿈 횟수를 곱해서 문제가 발생함 그걸 위한 임시 방편.
	}
	else
	{
		bool const bIsStockShopOn = (!lwGetUIWnd("FRM_SHOP_STOCK").IsNil());
		if( !lwGetUIWnd("FRM_SHOP").IsNil() 
		||	!lwGetUIWnd("FRM_SHOP_CP").IsNil()
		||	!lwGetUIWnd("FRM_SHOP_JOB").IsNil()
		||	bIsStockShopOn)
		{
			int iSellPrice = pDef->SellPrice();

			if(bIsStockShopOn)
			{
				PgStore::CONT_GODDS::value_type kItemInfo;
				if(S_OK == g_kViewStore.GetGoods(kItem.ItemNo(), kItemInfo))
				{
					iSellPrice = kItemInfo.iSellPrice;
				}
			}

			if(iSellPrice)
			{
				__int64 iEarnPrice = 0;
				CalcSellMoney(kItem, iEarnPrice, iSellPrice, kItem.Count(), g_kPilotMan.GetPlayerUnit()->GetPremium());

				std::wstring kPrice;
				//FormatMoney(iEarnPrice, kPrice);
				kPrice = GetMoneyString(iEarnPrice);

				wstrTextOut += _T("\n");
				wstrTextOut += TTW(40052);
				wstrTextOut += TTW(40053);
				wstrTextOut += kPrice;
				wstrTextOut += _T("\n");
			}
			else
			{
				wstrTextOut += _T("\n");
				wstrTextOut += TTW(40105);
				wstrTextOut += TTW(5014);
				wstrTextOut += _T("\n");
			}
		}
	}

	//수량 나누기
	if( pDef->CanConsume() && !kFlag.Enable(TTF_NOT_METHOD) ) // 사용 가능 아이템
	{
		wstrTextOut += _T("\n");
		wstrTextOut += L"{C=0xFFFFA500/}";
		wstrTextOut += TTW(4999);
	}
	if(!pDef->CanEquip() && !kFlag.Enable(TTF_NOT_AMONUT) )// 장착 가능 아이템
	{
		int const iMaxLimit = pDef->GetAbil(AT_MAX_LIMIT);
		if( iMaxLimit > 1 )
		{			
			wstrTextOut += TTW(1306);//40064//노랑
			wstrTextOut += TTW(5015);
			wstrTextOut += TTW(40011);
		}
	}
	wstrTextOut += ENTER_STRING;
	wstrTextOut += ENTER_STRING;
}

void MakeToolTipText_JobSkill_Item(PgBase_Item const &kItem, std::wstring &wstrTextOut, std::wstring &wstrType, TBL_SHOP_IN_GAME const &kShopItemInfo, SToolTipFlag const &kFlag)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( !pDef )
	{
		return;
	}
	CONT_DEF_JOBSKILL_ITEM_UPGRADE const* pkDefJobSkill_ItemUpgrade = NULL;
	g_kTblDataMgr.GetContDef(pkDefJobSkill_ItemUpgrade);
	CONT_DEF_JOBSKILL_ITEM_UPGRADE::const_iterator find_iter = pkDefJobSkill_ItemUpgrade->find(kItem.ItemNo());
	if( pkDefJobSkill_ItemUpgrade->end() == find_iter)
	{//아이템 업그레이드 정보 없으면 실패
		return;
	}
	CONT_DEF_JOBSKILL_ITEM_UPGRADE::mapped_type const &rkItemUpgrade = (*find_iter).second;
	//이름
	const wchar_t *pNameText = NULL;
	if(GetDefString(pDef->NameNo(), pNameText))
	{
		// Name
		wstrTextOut += TTW(40102);
		wstrTextOut += pNameText;
#ifndef USE_INB
		if(g_pkApp->VisibleClassNo())
		{
			BM::vstring kClassNo(kItem.ItemNo());
			wstrTextOut += (std::wstring)kClassNo;
		}
#endif
		wstrTextOut += TTW(40011);	//폰트 초기화
		wstrTextOut += ENTER_STRING;
	}
	//가공재료
	{
		wstrType += TTW(40011);
		wstrType += TTW(799621);
	}
	wstrTextOut += ENTER_STRING;
	wstrTextOut += ENTER_STRING;
	//등급 : ?등급
	if( 0 < rkItemUpgrade.iGrade)
	{
		BM::vstring kGrade(TTW(799680));
		kGrade.Replace(L"#GRADE#", rkItemUpgrade.iGrade);
		wstrTextOut += static_cast<std::wstring>(kGrade);
		wstrTextOut += ENTER_STRING;
	}
	//사용 구간 :LV??~??
	{
		BM::vstring kUseLevel(TTW(799681));
		kUseLevel.Replace(L"#MIN#", pDef->GetAbil(AT_LEVELLIMIT) );
		kUseLevel.Replace(L"#MAX#", pDef->GetAbil(AT_MAX_LEVELLIMIT) );
		wstrTextOut += static_cast<std::wstring>(kUseLevel);
		wstrTextOut += ENTER_STRING;
	}
	//재료 종류
	{
		BM::vstring kMaterial(TTW(799663));
		BM::vstring kMaterialType;
		GetJobSkillText_MaterialType(kItem.ItemNo(), kMaterialType);
		if(kMaterialType.size())
		{
			kMaterial.Replace(L"#SOURCE#", kMaterialType);
			wstrTextOut += static_cast<std::wstring>(kMaterial);
			wstrTextOut += ENTER_STRING;
		}
	}

	//직업기술 정보
	SJobSkillSaveIdx const * const pkJobSkillSaveIdx = JobSkill_Third::GetJobSkillSaveIdx(pDef->GetAbil(AT_JOBSKILL_SAVEIDX));
	std::wstring JobSkillText;
	MakeTextJobSkill_Info(pkJobSkillSaveIdx, JobSkillText);
	if( JobSkillText.size() )
	{
		wstrTextOut += JobSkillText;
		wstrTextOut += ENTER_STRING;
	}

	//설명
	const TCHAR *pExplainText = NULL;
	int Value = pDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pExplainText))
	{
		wstrTextOut += ENTER_STRING;
		wstrTextOut += TTW(40050);
		wstrTextOut += TTW(40051);
		wstrTextOut += pExplainText;
		wstrTextOut += _T("\n");
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 골동품 교환 가능한 아이템 한정
	if( lwDefenceMode::IsCanChangeAntique(EGT_ANTIQUE, kItem.ItemNo()) )
	{
		wstrTextOut += ENTER_STRING;
		wstrTextOut += TTW(750010);	// 컬러(노랑), 폰트
		wstrTextOut += TTW(750011);
		wstrTextOut += ENTER_STRING;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지

	//가격
	if(!kShopItemInfo.IsNull())
	{
		std::wstring kPrice;

		if( kShopItemInfo.iPrice )
		{
			//FormatMoney(kShopItemInfo.iPrice, kPrice);
			kPrice = GetMoneyString(kShopItemInfo.iPrice);
		}
		else
		{
			wchar_t	szCPValue[30] = {0,};
			swprintf(szCPValue, 29, TTW(5006).c_str(), kShopItemInfo.iCP * 10);
			kPrice = szCPValue;
			
		}

		wstrTextOut += _T("\n");
		wstrTextOut += TTW(40052);
		wstrTextOut += TTW(40053);	
		wstrTextOut += kPrice;
		wstrTextOut += _T("\n");//Pg2DString에서 높이를 계산하는 공식이 마지막줄의 폰트 크기와 줄바꿈 횟수를 곱해서 문제가 발생함 그걸 위한 임시 방편.
	}
	else
	{
		bool const bIsStockShopOn = (!lwGetUIWnd("FRM_SHOP_STOCK").IsNil());
		if( !lwGetUIWnd("FRM_SHOP").IsNil() 
		||	!lwGetUIWnd("FRM_SHOP_CP").IsNil()
		||	!lwGetUIWnd("FRM_SHOP_JOB").IsNil()
		||	bIsStockShopOn)
		{
			int iSellPrice = pDef->SellPrice();

			if(bIsStockShopOn)
			{
				PgStore::CONT_GODDS::value_type kItemInfo;
				if(S_OK == g_kViewStore.GetGoods(kItem.ItemNo(), kItemInfo))
				{
					iSellPrice = kItemInfo.iSellPrice;
				}
			}

			if(iSellPrice)
			{
				__int64 iEarnPrice = 0;
				CalcSellMoney(kItem, iEarnPrice, iSellPrice, kItem.Count(), g_kPilotMan.GetPlayerUnit()->GetPremium());

				std::wstring kPrice;
				//FormatMoney(iEarnPrice, kPrice);
				kPrice = GetMoneyString(iEarnPrice);

				wstrTextOut += _T("\n");
				wstrTextOut += TTW(40052);
				wstrTextOut += TTW(40053);
				wstrTextOut += kPrice;
				wstrTextOut += _T("\n");
			}
			else
			{
				wstrTextOut += _T("\n");
				wstrTextOut += TTW(40105);
				wstrTextOut += TTW(5014);
				wstrTextOut += _T("\n");
			}
		}
	}	

	AddToolTip_Manufacture(pDef, wstrTextOut); //즉석 가공 텍스트 추가

	if( pDef->CanConsume() && !kFlag.Enable(TTF_NOT_METHOD) ) // 사용 가능 아이템
	{
		wstrTextOut += _T("\n");
		wstrTextOut += L"{C=0xFFFFA500/}";
		wstrTextOut += TTW(4999);
	}
	if(!pDef->CanEquip() && !kFlag.Enable(TTF_NOT_AMONUT) )// 장착 가능 아이템
	{
		int const iMaxLimit = pDef->GetAbil(AT_MAX_LIMIT);
		if( iMaxLimit > 1 )
		{			
			wstrTextOut += TTW(1306);//40064//노랑
			wstrTextOut += TTW(5015);
			wstrTextOut += TTW(40011);
		}
	}
	wstrTextOut += ENTER_STRING;
	wstrTextOut += ENTER_STRING;
}

bool MakeQuickSkillToolTipText( PgSkillTree::stTreeNode* pTreeNode, std::wstring& wstrTextOut, std::wstring& wstrType, int const iSkillNo, bool const bNotShowZeroValue, bool const bUseOverSkillLevel)
{
	WCHAR	temp[500];
	wstrTextOut.clear();

	CSkillDef const* pSkillDef = pTreeNode->m_pkSkillDef;
	if (0<iSkillNo)
	{
		int iRealSkillNo = iSkillNo;

		if(bUseOverSkillLevel)
		{
			int const iOverSkillLevel = pTreeNode->GetOverSkillLevel();
			iRealSkillNo += iOverSkillLevel;
		}

		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		pSkillDef = kSkillDefMgr.GetDef(iRealSkillNo);
	}

	if (!pSkillDef)
	{
		return false;
	}

	wchar_t const* pName = NULL;
	GetDefString(pSkillDef->NameNo(),pName);
	if(pName)
	{	
		wstrTextOut += TTW(40102); //이름용
		if(0 < pSkillDef->GetAbil(AT_IGNORE_SKILLLV_TOOLTIP))
		{// 스킬 레벨 표시 하지 않는 어빌이 있으면
			swprintf(temp,500,_T("%s"), pName);
		}
		else
		{
			swprintf(temp,500,_T("%s %s %d"), pName, TTW(224).c_str(), pSkillDef->GetAbil(AT_LEVEL));
		}
		wstrTextOut+=temp;
		swprintf(temp,500,_T(""));//초기화 - 스킬 제목이 두번 들어가지 않도록 수정
	}
	else
	{
		wstrTextOut+=_T("Undefined Name");
	}
	wstrTextOut+= _T("\n\n");

	wstrTextOut += TTW(40011);	//폰트 초기화

	if(!bNotShowZeroValue && pSkillDef->m_byType)
	{
		//swprintf(temp,100,_T("%s"), Type[pSkillDef->m_byType]);
		wstrType+=TTW(pSkillDef->m_byType+40140);
	}
	
	//	스킬 커맨드
	const CONT_DEFSKILL *pkContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pkContDefMap);
	if (pkContDefMap)
	{
		CONT_DEFSKILL::const_iterator def_it = pkContDefMap->find(pTreeNode->m_ulSkillNo);
		if (pkContDefMap->end() != def_it)
		{
			CONT_DEFSKILL::mapped_type kT = (*def_it).second;
			if( 0 != kT.iCmdStringNo )
			{
				wstrTextOut+=TTW(40107);
				wstrTextOut+=_T(" : ");

				const wchar_t *pText = NULL;
				if(GetDefString(kT.iCmdStringNo, pText))
				{
					wstrTextOut+=pText;
				}
				wstrTextOut+=_T("\n");
			}
		}
	}

	PgPilot	*pkPlayerPilot = g_kPilotMan.GetPlayerPilot(); 
	//	소모 MP
	if(!bNotShowZeroValue && pSkillDef->m_sMP)
	{
		swprintf(temp,500,_T("%s : "),TTW(40133).c_str());
		wstrTextOut+=temp;
		if (pkPlayerPilot)
		{
			if (pkPlayerPilot->GetAbil(AT_MP)< pSkillDef->m_sMP)
			{
				wstrTextOut+=TTW(40105);
			}
		}
		swprintf(temp,500,_T("%d\n"),pSkillDef->m_sMP);
		wstrTextOut+=temp;
	}
	else
	{
		//소모 MP가 MAXMP의 %를 소모 할 경우
		int const iNeedRateMaxMP = pSkillDef->GetAbil(AT_NEED_MAX_R_MP);
		if(!bNotShowZeroValue && 0 < iNeedRateMaxMP)
		{
			swprintf(temp,500,_T("%s : "),TTW(40133).c_str());
			wstrTextOut+=temp;

			if (pkPlayerPilot)
			{				
				int const iNeedMP = static_cast<int>(static_cast<float>(pkPlayerPilot->GetAbil(AT_C_MAX_MP)) * (static_cast<float>(iNeedRateMaxMP) / ABILITY_RATE_VALUE_FLOAT));
				if (pkPlayerPilot->GetAbil(AT_MP)< iNeedMP)
				{
					wstrTextOut+=TTW(40105);
				}
			}

			float const fRateMP = static_cast<float>(iNeedRateMaxMP / 100.0f);
			swprintf(temp,500,_T("%s %.1f%%\n"),TTW(512).c_str(), fRateMP);
			 
			wstrTextOut+=temp;
		}
	}

	wstrTextOut += TTW(40011);	//폰트 초기화

	// 소모 HP
	if (0 < pSkillDef->m_sHP)
	{
		swprintf(temp,500,_T("%s : "),TTW(40134).c_str());
		wstrTextOut+=temp;
		if (pkPlayerPilot)
		{
			if (pkPlayerPilot->GetAbil(AT_HP)< pSkillDef->m_sHP)
			{
				wstrTextOut+=TTW(225);
			}
		}
		swprintf(temp,500,_T("%d\n"),pSkillDef->m_sHP);
		wstrTextOut+=temp;
	}
	else
	{
		//소모 MP가 MAXHP의 %를 소모 할 경우
		int const iNeedRateMaxHP = pSkillDef->GetAbil(AT_NEED_MAX_R_HP);
		if(0 < iNeedRateMaxHP)
		{
			swprintf(temp,500,_T("%s : "),TTW(40134).c_str());
			wstrTextOut+=temp;

			if (pkPlayerPilot)
			{				
				int const iNeedHP = static_cast<int>(static_cast<float>(pkPlayerPilot->GetAbil(AT_C_MAX_HP)) * (static_cast<float>(iNeedRateMaxHP) / ABILITY_RATE_VALUE_FLOAT));
				if (pkPlayerPilot->GetAbil(AT_MP)< iNeedHP)
				{
					wstrTextOut+=TTW(225);
				}
			}

			float const fRateHP = static_cast<float>(iNeedRateMaxHP / 100.0f);
			swprintf(temp,500,_T("%s %.1f%%\n"),TTW(511).c_str(), fRateHP);
			 
			wstrTextOut+=temp;
		}
	}

	wstrTextOut += TTW(40011);	//폰트 초기화

	CSkillDef const* pBackUpSkillDef = NULL;
	int iRealSkill = pSkillDef->GetAbil(AT_CHILD_CAST_SKILL_NO);
	if( 0 < iRealSkill )
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pChildSkillDef = kSkillDefMgr.GetDef(iRealSkill);
		if( pChildSkillDef )
		{
			pBackUpSkillDef = pSkillDef;
			pSkillDef = pChildSkillDef;
		}
	}

	int iComboCount = pSkillDef->GetAbil(AT_COMBO_HIT_COUNT);

	//	물리대미지
	int iAtk = pSkillDef->GetAbil(AT_I_PHY_ATTACK_ADD_RATE);
	if( 0 < iComboCount )
	{
		iAtk /= iComboCount;
	}
	if( 0 < iAtk )
	{
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(40136).c_str(), ((float)(iAtk) * 0.01f));
		wstrTextOut+=temp;
		iAtk = pSkillDef->GetAbil(AT_PHY_ATTACK);
		if( 0 < iComboCount )
		{
			iAtk /= iComboCount;
		}
		if( 0 < iAtk )
		{
			swprintf(temp, 499, _T(" + %d"), iAtk);
			wstrTextOut+=temp;
		}
		wstrTextOut+=L"\n";
	}

	//	마법대미지
	int iMAtk = pSkillDef->GetAbil(AT_I_MAGIC_ATTACK_ADD_RATE);
	if( 0 < iComboCount )
	{
		iMAtk /= iComboCount;
	}

	if( 0 < iMAtk )
	{
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(40137).c_str(), ((float)(iMAtk) * 0.01f));
		wstrTextOut+=temp;
		iMAtk = pSkillDef->GetAbil(AT_MAGIC_ATTACK);
		if( 0 < iComboCount )
		{
			iMAtk /= iComboCount;
		}

		if( 0 < iMAtk )
		{
			swprintf(temp, 499, _T(" + %d"), iMAtk);
			wstrTextOut+=temp;
		}
		wstrTextOut+=L"\n";
	}

	if( NULL != pBackUpSkillDef )
	{
		pSkillDef = pBackUpSkillDef;
	}

	//	시전 시간
	int const iCast = pSkillDef->GetAbil(AT_CAST_TIME);

	if (0<iCast)
	{
		if( 0 < ((iCast % 1000) * 0.01f) )
			swprintf(temp, 500, _T("%s : %.1f%s\n"), TTW(47).c_str(), iCast*0.001f, TTW(54).c_str());
		else
			swprintf(temp, 500, _T("%s : %d%s\n"), TTW(47).c_str(), static_cast<int>(iCast*0.001f), TTW(54).c_str());

		wstrTextOut += temp;
	}
	else if(!bNotShowZeroValue)
	{
		swprintf(temp, 500, _T("%s : %s\n"), TTW(47).c_str(), TTW(251).c_str());
		wstrTextOut += temp;
	}

	//	쿨타임
	int const iCool = pSkillDef->GetAbil(ATS_COOL_TIME);
	if (0<iCool)
	{
		if( 0 < ((iCool % 1000) * 0.01f) )
			swprintf(temp,500,_T("%s : %.1f%s\n"),TTW(250).c_str(), iCool*0.001f, TTW(54).c_str());
		else
			swprintf(temp,500,_T("%s : %d%s\n"),TTW(250).c_str(), static_cast<int>(iCool*0.001f), TTW(54).c_str());
		wstrTextOut+=temp;
	}

	int const iMaxTarget = pSkillDef->GetAbil(AT_MAX_TARGETNUM);
	if(1 < iMaxTarget)
	{// 최대타격개체수
		BM::vstring vTemp( TTW(40151) ); 
		vTemp.Replace(L"#NUM#", iMaxTarget);
		vTemp+="\n";
		wstrTextOut+=vTemp.operator const std::wstring &();
	}
	
	int const iMaxPenetration = pSkillDef->GetAbil(AT_PENETRATION_COUNT);
	if(0 < iMaxPenetration)
	{// 최대관통개체수
		BM::vstring vTemp( TTW(40152) ); 
		vTemp.Replace(L"#NUM#", iMaxPenetration);
		vTemp+="\n";
		wstrTextOut+=vTemp.operator const std::wstring &();
	}

	bool bCantBlockTxt = false;
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	const	CEffectDef* pEffDef = kEffectDefMgr.GetDef(pSkillDef->No());
	if (pEffDef)
	{
		int const iDuration = pEffDef->GetDurationTime()/1000;
		if (0<iDuration)
		{
			swprintf(temp,500,_T("%s : "),TTW(40135).c_str());
			wstrTextOut+=temp;
			GetTimeString(iDuration, wstrTextOut, true);
		}
		if( pEffDef->GetAbil(AT_CAN_NOT_BLOCK_BY_EFFECT) )
		{
			bCantBlockTxt = true;
		}
	}


	const TCHAR *pText = NULL;
	int Value = pSkillDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		if (strlenT(pText))
		{
			swprintf(temp,100,_T("\n%s : \n"),TTW(231).c_str());
			wstrTextOut+= ::TTW(750010);	//노랑
			wstrTextOut+=temp;
			wstrTextOut+= ::TTW(1554);		//흰색
			wstrTextOut+=pText;
		}
	}

	if( !bCantBlockTxt
		&& 0 < pSkillDef->GetAbil(AT_CAN_NOT_BLOCK_BY_ACTION) 
		)
	{
		bCantBlockTxt = true;
	}
	if( bCantBlockTxt )
	{// 블록 할 수 없습니다.
		wstrTextOut += ENTER_STRING; 
		wstrTextOut += ENTER_STRING;
		wstrTextOut += TTW(750010);	// 컬러(노랑), 폰트
		wstrTextOut += TTW(791611);
	}

	return	true;
}

void CallQuickSkillToolTip(PgSkillTree::stTreeNode* pTreeNode, lwPoint2& pt, int const iSkillNo, bool const bNotShowZeroValue, bool const bUseOverSkillLevel)
{
	std::wstring wstrText;
	std::wstring wstrType;
	if(MakeQuickSkillToolTipText(pTreeNode, wstrText, wstrType, iSkillNo, bNotShowZeroValue, bUseOverSkillLevel))
	{
		lwCallToolTipByText(0, wstrText, pt, "ToolTip_Skill", 0, MB(wstrType));
	}
	return;
}

bool MakeItemName(int const iItemNo, const SEnchantInfo &rkEnchant, std::wstring &wstrName)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(iItemNo);

	if ( pDef == NULL )
	{
		return false;
	}
	PgBase_Item kItem;
	kItem.ItemNo(iItemNo);
	kItem.EnchantInfo(rkEnchant);
	
	E_ITEM_GRADE const eItemGrade = ::GetItemGrade(kItem);

	const wchar_t *pName = NULL;
	if(GetDefString(pDef->NameNo(), pName) )//이름 디스플레이
	{
		wstrName.clear();

		switch(eItemGrade)
		{
		case IG_CURSE:
			{
				wstrName += TTW(50380);//저주받은
				wstrName += _T(" ");
			}break;
		case IG_SEAL:
			{
				wstrName += TTW(50381);//봉인된
				wstrName += _T(" ");
			}break;
		default:
			{
//				int const iOptCount = rkEnchant.OptionCount();
//				if(iOptCount)
//				{
//					wstrName += TTW(50350+iOptCount);
//					wstrName += _T(" ");
//				}
			}break;
		}

		if(IG_SEAL != eItemGrade //봉인이 아니고.
		&& rkEnchant.IsPlus()) //플러스 아이템이냐.
		{
			wstrName += _T("+");
			wstrName += (std::wstring const)BM::vstring((int)rkEnchant.PlusLv());
			wstrName += _T(" ");
		}

		wstrName += pName;

#ifndef USE_INB
		if(g_pkApp->VisibleClassNo())
		{
			BM::vstring kClassNo(iItemNo);
			wstrName += (std::wstring)kClassNo;
		}
#endif
	}
	return true;
}
void _MakeClassLimitText(CItemDef const *pDef, std::wstring &wstrText)
{
	if(!pDef)
	{
		return;
	}

	__int64 const iNoLimitDisplayFilter = -1;
	__int64 const iClassLimitDisplayFilter = pDef->GetAbil64(AT_CLASSLIMIT_DISPLAY_FILTER);
	__int64 const iClassLimit = (0 != iClassLimitDisplayFilter)? pDef->GetAbil64(AT_CLASSLIMIT)&iClassLimitDisplayFilter: pDef->GetAbil64(AT_CLASSLIMIT);
	//클래스 제한은 예외처리해야 함
	int iClassLimitLog = (int)NiFloor((float)(FastLog64(iClassLimit) / (__int64)NiFastLog(2)));
	if( iClassLimitLog )
	{
		PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
		if (pkPilot)
		{
			int iAbil = pkPilot->GetAbil(AT_CLASS);
			switch( GetCommonClassEquipItemType( pDef->m_i64ClassLimit ) )
			{
			case ECCE_ALL_CLASS:
				{
					wstrText += TTW(30000+253);
				}break;
			case ECCE_ALL_HUMAN_CLASS:
				{
					wstrText += TTW(30000+254);
				}break;
			case ECCE_ALL_DRAGONIAN_CLASS:
				{
					wstrText += TTW(30000+255);
				}break;
			case ECCE_NOT_COMMON:
				{
					__int64 iAbilBIT = 1i64<<iAbil;
					if((iAbilBIT & iClassLimit) != iAbilBIT)
					{
						wstrText += TTW(89999);//빨간색 글자
					}
					size_t iCount = 0;
					for(int i = 1; i <= UCLASS_MAX; ++i)
					{
						if(1i64<<i & iClassLimit)
						{
							if( 0 < iCount )
							{
								wstrText += L", ";
							}
							wstrText += TTW(30000+i);
							if( iNoLimitDisplayFilter == iClassLimitDisplayFilter )
							{
								break; // 필터가 설정 안되어져 있으면, 최초 한개만 보여준다
							}
							++iCount;
						}
					}
					if((iAbilBIT & iClassLimit) != iAbilBIT)
					{				
						wstrText += TTW(40011);//원래 글자
					}
				}break;
			}
		}
	}
}

void MakeClassLimitText(CItemDef const *pDef, std::wstring &wstrText)
{
	if( pDef )
	{
		const TCHAR *pText = NULL;
		const int texNO = 9500;
		__int64 const iClassLimit =  pDef->GetAbil64(AT_CLASSLIMIT);
		//클래스 제한은 예외처리해야 함
		int iClassLimitLog = (int)NiFloor((float)(FastLog64(iClassLimit) / (__int64)NiFastLog(2)));
		if( iClassLimitLog )
		{
			if(GetDefString(texNO,pText) )
			{
				wstrText += pText;
				wstrText += WSTR_ATTR_EXPLAIN;
				_MakeClassLimitText(pDef, wstrText);
				wstrText += L"\n";

			}
			else
			{
				return;
			}
		}
	}
}

void MakePetClassLimitText(CItemDef const* pDef, std::wstring& wstrText)
{
	if( pDef )
	{
		int const iClass = static_cast<int>(pDef->GetAbil64(AT_CLASSLIMIT));
		wchar_t szTemp[ MAX_PATH ] = {0,};

		wstrText += TTW(349);
		wstrText += WSTR_ATTR_EXPLAIN;

		if( iClass < 10 )
		{
			wstrText += TTW(351);
			wstrText += L" ";
			swprintf_s(szTemp, MAX_PATH, TTW(350).c_str(), iClass + 1);
			wstrText += szTemp;
			wstrText += L" ";
			wstrText += TTW(1804);
			wstrText += L"\n";
		}
		else
		{
			int iBaseClass = PgClassPetDef::GetBaseClass(iClass);
			int iGrade = PgClassPetDef::GetClassGrade(iClass);

			std::wstring const* pClassName = NULL;
			if( GetDefString(iBaseClass, pClassName) )
			{
				wstrText += (*pClassName);
				wstrText += L" ";
				swprintf_s(szTemp, MAX_PATH, TTW(350).c_str(), iGrade + 1);
				wstrText += szTemp;
				wstrText += L" ";
				wstrText += TTW(1804);
				wstrText += L"\n";
			}
		}
	}
}

int GetMonsterCardItemNo(int const iOrderIndex, int const iCardNo)
{
	CONT_MONSTERCARD const *kCont = NULL;
	g_kTblDataMgr.GetContDef(kCont);
	if( kCont )
	{
		CONT_MONSTERCARD::key_type kKey(iOrderIndex, iCardNo);

		CONT_MONSTERCARD::const_iterator iter = kCont->find(kKey);
		if( kCont->end() != iter )
		{
			return iter->second;
		}
	}
	return 0;
}

void MakeSetItemOptionString(PgBase_Item const &kItem, CItemDef const* pkItemDef, int const iAbilNum, std::wstring &wstrText, bool const bColor,bool bSetColor)
{
	CONT_DEFEFFECTABIL const *pkDefEffectAbil = NULL;
	g_kTblDataMgr.GetContDef(pkDefEffectAbil);
	if( pkDefEffectAbil )
	{
		CONT_DEFEFFECTABIL::const_iterator iter = pkDefEffectAbil->find(iAbilNum);
		if( pkDefEffectAbil->end() != iter )
		{
			for(int i=0; i<MAX_EFFECT_ABIL_ARRAY; i++)
			{
				int iType = iter->second.iType[i];
				int iValue = iter->second.iValue[i];

				if( 0 != iType )
				{
					MakeAbilString(kItem, pkItemDef, iType, iValue, wstrText, NULL, NULL, false, true, 0, true, bColor,bSetColor);//레벨
				}
			}
		}
	}
}

void MakeOverSkillToolTip(std::wstring & wstrText, int const iOverSkillNo, int const iAddLevel, bool const bAddTitle)
{
	if( (0<iOverSkillNo) && (0!=iAddLevel) )
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iOverSkillNo);
		if( pkSkillDef )
		{
			int iMaxSkillLv = 0;

			do
			{
				CSkillDef const* pkMaxSkillDef = kSkillDefMgr.GetDef(iOverSkillNo + iMaxSkillLv);
				if( pkMaxSkillDef )
				{
					++iMaxSkillLv;
				}
				else
				{
					break;
				}
			}while( 0 != iOverSkillNo );


			wchar_t const* pNeedSkillName = NULL;
			if( GetDefString(pkSkillDef->NameNo(), pNeedSkillName) )
			{
				if( bAddTitle )
				{
					wstrText += TTW(40068);
					wstrText += TTW(40021);
				}

				std::wstring kTempMsg = TTW(790160);
				if( !kTempMsg.empty() )
				{
					std::wstring kClassName;
					MakeSkillLimitClassText(pkSkillDef, kClassName);
					
					PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#CLASS#", kClassName, kTempMsg);
					PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#NAME#", BM::vstring(pNeedSkillName), kTempMsg);
					PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#LEVEL#", BM::vstring(iAddLevel), kTempMsg);
					PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#MAX_LEVEL#", BM::vstring(iMaxSkillLv), kTempMsg);
					wstrText += kTempMsg;
				}
				wstrText += ENTER_STRING;

				///////////////////////할로윈 아이템 스킬배우고 안배우고에 따른 출력./////////////////////////////
				PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pPlayer ){	return; }

				PgMySkill* pMySkill = pPlayer->GetMySkill();
				if( !pMySkill ){ return; }

				if( false == pMySkill->IsExist(iOverSkillNo, true))
				{
					wstrText += L"{C=";
					wstrText += TTW(4101);
					wstrText += L"/}";
					wstrText += TTW(460511);
					wstrText += TTW(40011);	//폰트 초기화
					wstrText += ENTER_STRING;
				}
			}
		}
	}
}

void MakeMonsterCardOptionString(PgBase_Item const &kItem, CItemDef const* pkItemDef, int const iAbilNum, std::wstring &wstrText, bool const bAddTitle, PgBase_Item const * const pkEquipItem)
{
	CONT_DEFEFFECTABIL const *pkDefEffectAbil = NULL;
	g_kTblDataMgr.GetContDef(pkDefEffectAbil);
	if( pkDefEffectAbil )
	{
		bool bFirstOption = true;
		CONT_DEFEFFECTABIL::const_iterator iter = pkDefEffectAbil->find(iAbilNum);
		if( pkDefEffectAbil->end() != iter )
		{
			for(int i=0; i<MAX_EFFECT_ABIL_ARRAY; i++)
			{
				int iType = iter->second.iType[i];
				int iValue = iter->second.iValue[i];

				if( 0 != iType )
				{
					if( true == bFirstOption )
					{
						bFirstOption = false;
						if( bAddTitle )
						{
							wstrText += TTW(40068);
							wstrText += TTW(40021);		
						}
					}
					MakeAbilStringColor(iType, wstrText); // 타입이 속성에 관계된 것이면 컬러를 바꾼다.
					MakeAbilString(kItem, pkItemDef, iType, iValue, wstrText, NULL, pkEquipItem);//레벨
				}
			}
			/*if( false == bFirstOption )
			{
				if( bAddTitle )
				{
					wstrText += _T("\n");
				}
			}*/
		}
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( pDef )
		{
			MakeOverSkillToolTip(wstrText, pDef->GetAbil(AT_EFFECTNUM9), pDef->GetAbil(AT_EFFECTNUM10), bFirstOption);
		}
	}
}

void MakeToopTipIcon_Common(XUI::CXUI_Wnd* pMainWnd, int iIconNo)
{
	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);

	//기본 아이콘
	CONT_DEFRES::const_iterator itrDest = pContDefRes->find(iIconNo);
	XUI::CXUI_Wnd *pImgWnd = pMainWnd->GetControl(UNI(szCardImgID));
	if( pImgWnd )
	{
		if(itrDest != pContDefRes->end() && iIconNo != 0)
		{
			pImgWnd->DefaultImgName(itrDest->second.strXmlPath);
			pImgWnd->Visible(true);
		}
		else
		{
			pImgWnd->Visible(false);
		}
	}
	else return;

	//추가 아이콘 1, 2, 3
	char const* szAddIconWndName[3] = {szCardAddImg1ID, szCardAddImg2ID, szCardAddImg3ID};
	XUI::CXUI_Wnd *pAddImgWnd = NULL;
	for(int n = 0; n < 3; n++)
	{
		pAddImgWnd = pMainWnd->GetControl(UNI(szAddIconWndName[n]));
		if(!pAddImgWnd) continue;
		if(pContDefRes->end()==itrDest)
		{
			pAddImgWnd->Visible(false);
			continue;	//pContDefRes->find(iIconNo) 가 실패하더라도 클라이언트 크래쉬 방지
		}	
		CONT_DEFRES::const_iterator itrAddRes = pContDefRes->find(itrDest->second.AddResNo[n]);
		if(itrAddRes == pContDefRes->end() || itrAddRes->second.strXmlPath.empty() || itrAddRes->second.strXmlPath.compare(UNI(".")) == 0)
		{
			pAddImgWnd->Visible(false);
		}
		else
		{
			pAddImgWnd->DefaultImgName(itrAddRes->second.strXmlPath);
			pAddImgWnd->Visible(true);
		}
	}

}

void MakeToolTipText_MonsterCard(PgBase_Item const &kOrgItem, lwPoint2 &pt, SToolTipFlag const &kFlag)
{
	std::wstring wstrText = _T("");
	PgBase_Item kItem = kOrgItem;
	SEnchantInfo const &kEnchantInfo = kItem.EnchantInfo();
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if(!pDef)
	{
		return;
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 등급 색상
	wstrText += TTW(40002);
	E_ITEM_GRADE const eItemLv = GetItemGrade(kItem);
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 이름
	std::wstring kItemName;
	MakeItemName(kItem.ItemNo(), kEnchantInfo, kItemName);
	if(pDef->GetAbil(AT_CARD_BGNDNO) > 0)
	{
		wstrText += _T(" \n\n\n");
	}
	wstrText += kItemName;
	wstrText += TTW(40011);	//폰트 초기화
	wstrText += _T("\n");
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 사용 구간
	wstrText += ENTER_STRING;
	BM::vstring kUseLevel(TTW(799681));
	kUseLevel.Replace(L"#MIN#", pDef->GetAbil(AT_LEVELLIMIT) );
	kUseLevel.Replace(L"#MAX#", pDef->GetAbil(AT_MAX_LEVELLIMIT) );
	wstrText += static_cast<std::wstring>(kUseLevel);
	wstrText += ENTER_STRING;
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 어빌리티 설명
	//다음 행동이 제한됨	
	MakeCantAbil(wstrText,pDef);
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 설명
	const TCHAR *pText = NULL;
	int Value = pDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		wstrText += ENTER_STRING;
		wstrText += TTW(40050);
		wstrText += TTW(40051);
		wstrText += pText;
		wstrText += ENTER_STRING;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 슬롯 위치
	Value = pDef->GetAbil(AT_MONSTER_CARD_ORDER);
	if( Value )
	{
		wstrText += ENTER_STRING;
		wstrText += TTW(790157);
		wstrText += TTW(40051);
		wstrText += ENTER_STRING;

		std::wstring kTempMsg = TTW(790158);
		if( !kTempMsg.empty() )
		{
			PgStringUtil::ReplaceStr< std::wstring >(kTempMsg, L"#NUM#", BM::vstring(Value), kTempMsg);
			wstrText += kTempMsg;
		}		
		wstrText += ENTER_STRING;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 착용 위치 툴팁
	int const iItemEquipLimit = pDef->GetAbil(AT_EQUIP_LIMIT);
	if( 0 < iItemEquipLimit )
	{
		/*int const Defence = (EQUIP_LIMIT_HELMET | EQUIP_LIMIT_SHIRTS | EQUIP_LIMIT_PANTS | EQUIP_LIMIT_SHOULDER | EQUIP_LIMIT_GLOVE | EQUIP_LIMIT_BOOTS | EQUIP_LIMIT_SHEILD);
		int const Weapon_Defence = (EQUIP_LIMIT_WEAPON | Defence);
		int const EquipTypeValue = (iItemEquipLimit & 0xFFFFFFFF);
		if( Weapon_Defence == EquipTypeValue )
		{
			wstrText += TTW(40069);
			wstrText += TTW(40021);
			
			wstrText += TTW(1852);
			wstrText += L" / ";
			wstrText += TTW(40070);
			wstrText += _T("\n");
		}
		else if(Defence == EquipTypeValue )
		{
			wstrText += TTW(40069);
			wstrText += TTW(40021);

			wstrText += TTW(40070);
			wstrText += _T("\n");
		}
		else if(EQUIP_LIMIT_WEAPON == EquipTypeValue )
		{
			wstrText += TTW(40069);
			wstrText += TTW(40021);

			wstrText += TTW(1852);
			wstrText += _T("\n");
		}*/

		wstrText += ENTER_STRING;
		wstrText += TTW(40069);
		wstrText += TTW(40021);
		GetEqiupPosStringMonsterCard(pDef->GetAbil(AT_EQUIP_LIMIT), wstrText);
		wstrText += ENTER_STRING;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<착용 위치 툴팁 여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 효과 설명
	if(IG_SEAL != eItemLv)//봉인은 추가 옵션이 보이지 않음.
	{
		int iAbilNum = kItemDefMgr.GetAbil(kItem.ItemNo(), AT_EFFECTNUM1);
		std::wstring CardOptionText;
		MakeMonsterCardOptionString(kItem, pDef, iAbilNum, CardOptionText);
		if( CardOptionText.size() )
		{
			wstrText += ENTER_STRING;
			wstrText += CardOptionText;
		}
	}
	//소켓 추출 관련 툴팁.
	MakeToolTipTextExtraction(pDef,wstrText,kItem);
		
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 소켓카드 교환 가능한 아이템 한정
	if( lwDefenceMode::IsCanExchangeItem(kItem.ItemNo(), EGemStoreMenu::EGM_SOCKETCARD) )
	{
		wstrText += ENTER_STRING;
		wstrText += TTW(750010);	// 컬러(노랑), 폰트
		wstrText += TTW(750012);
		wstrText += ENTER_STRING;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지
	
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 가격
	bool const bIsStockShopOn = (!lwGetUIWnd("FRM_SHOP_STOCK").IsNil());
	if( !lwGetUIWnd("FRM_SHOP").IsNil() 
	||	!lwGetUIWnd("FRM_SHOP_CP").IsNil()
	||	!lwGetUIWnd("FRM_SHOP_JOB").IsNil()
	||	bIsStockShopOn)
	{
		int iSellPrice = pDef->SellPrice();

		if(bIsStockShopOn)
		{
			PgStore::CONT_GODDS::value_type kItemInfo;
			if(S_OK == g_kViewStore.GetGoods(kItem.ItemNo(), kItemInfo))
			{
				iSellPrice = kItemInfo.iSellPrice;
			}
		}

		if(iSellPrice)
		{
			__int64 iEarnPrice = 0;
			CalcSellMoney(kItem, iEarnPrice, iSellPrice, kItem.Count(), g_kPilotMan.GetPlayerUnit()->GetPremium());

			std::wstring kPrice;
			//FormatMoney(iEarnPrice, kPrice);
			kPrice = GetMoneyString(iEarnPrice);

			wstrText += _T("\n");
			wstrText += TTW(40052);
			wstrText += TTW(40053);
			wstrText += kPrice;
			wstrText += _T("\n");
		}
		else
		{
			wstrText += _T("\n");
			wstrText += TTW(40105);
			wstrText += TTW(5014);
			wstrText += _T("\n");
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 수량 아이템 툴팁
	if( pDef->CanConsume() && !kFlag.Enable(TTF_NOT_METHOD) ) // 사용 가능 아이템
	{
		wstrText += _T("\n");
		wstrText += L"{C=0xFFFFA500/}";
		wstrText += TTW(4999);
	}
	if(!pDef->CanEquip() && !kFlag.Enable(TTF_NOT_AMONUT))// 장착 가능 아이템
	{
		int const iMaxLimit = pDef->GetAbil(AT_MAX_LIMIT);
		if( iMaxLimit > 1 )
		{			
			wstrText += TTW(1306);//40064//노랑
			wstrText += TTW(5015);
			wstrText += TTW(40011);
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	XUI::CXUI_Wnd* pWnd = XUIMgr.Call(UNI(szMonsterCardToolTipID));
	if( pWnd )
	{
		int const BGResourceNo = pDef->GetAbil(AT_CARD_BGNDNO);
		MakeToopTipIcon_Common(pWnd, BGResourceNo);
		// 출력
		pWnd->Text(wstrText);

		pWnd->Location(pt());
		pWnd->Invalidate(true);
		pWnd->SetCustomData(0, sizeof(size_t));//카테고리 설정
	}

	return;
}
bool MakeToolTipTextConstellation(CONT_DEFGEMSTORE const * pDef, DWORD const & MaterialItemNo, int const Menu)
{
	if( pDef )
	{
		CONT_DEFGEMSTORE::const_iterator iter = pDef->begin();
		for( ; iter != pDef->end(); ++iter )
		{
			CONT_DEFGEMSTORE_ARTICLE::const_iterator SubIter = iter->second.kContArticles.begin();
			for( ; SubIter != iter->second.kContArticles.end(); ++SubIter )
			{
				if( Menu == SubIter->first.iMenu )
				{
					if( MaterialItemNo == SubIter->first.iItemNo)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}
void MakeToolTipTextExtraction(CItemDef const *pDef,std::wstring &wstrText,PgBase_Item const &kItem)
{
	if(!pDef)
	{
		return;
	}
	CONT_DEFGEMSTORE const * pDefGemStore = NULL;
	g_kTblDataMgr.GetContDef(pDefGemStore);


	if(pDef->GetAbil(AT_SOCET_CARD_EXTRACTION_CASH_ITEM) || pDef->GetAbil( AT_SOCET_CARD_EXTRACTION_ITEM_NAME))
	{
		if(MakeToolTipTextConstellation(pDefGemStore, kItem.ItemNo(), Item_SWAP))
		{
			wstrText += TTW(790182);
		}
		else if((MakeToolTipTextConstellation(pDefGemStore, kItem.ItemNo(), Item_PLUS)))
		{
			wstrText += TTW(790183);
		}
		else
		{
			wstrText += TTW(790178);
		}
		wstrText += ENTER_STRING;
		wstrText += TTW(790179);
		wstrText += ENTER_STRING;
		wstrText += TTW(CONTENTS_COLOR_WHITE);
		
		BM::vstring vstrMsg(TTW(790180));
		if(vstrMsg.size() != 0 )
		{
			wchar_t const * pTextCash = NULL;
			wchar_t const * pTextCashCount = NULL;

			int iCashItem = pDef->GetAbil(AT_SOCET_CARD_EXTRACTION_CASH_ITEM);
			int iCashItemCount = pDef->GetAbil(AT_SOCET_CARD_EXTRACTION_CASH_ITEM_COUNT);
			
			GetDefString(iCashItem, pTextCash);
			
			vstrMsg.Replace(L"#NAME#", pTextCash);
			vstrMsg.Replace(L"#COUNT#", iCashItemCount);
			wstrText += std::wstring(vstrMsg);
			wstrText += ENTER_STRING;
		}

		BM::vstring vstrMsgNext(TTW(790181));
		if(vstrMsgNext.size() != 0)
		{
			wchar_t const * pTextInGameItem = NULL;
			wchar_t const * pTextInGameItemCount = NULL;
						
			int iInGameItem = pDef->GetAbil(AT_SOCET_CARD_EXTRACTION_ITEM_NAME);
			int iInGameItemCount = pDef->GetAbil(AT_SOCET_CARD_EXTRACTION_ITEM_COUNT);
			GetDefString(iInGameItem, pTextInGameItem);
			
			vstrMsgNext.Replace(L"#NAME#", pTextInGameItem);
			vstrMsgNext.Replace(L"#COUNT#", iInGameItemCount);
					
			wstrText += std::wstring(vstrMsgNext);
		}

		wstrText += ENTER_STRING;

	}
			
}

void MakeToolTipText_SuperGirl(PgBase_Item const &kOrgItem, lwPoint2 &pt, TBL_SHOP_IN_GAME const &kShopItemInfo, SToolTipFlag const &kFlag)
{
	std::wstring wstrText = _T("");
	PgBase_Item kItem = kOrgItem;
	SEnchantInfo const &kEnchantInfo = kItem.EnchantInfo();
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if(!pDef)
	{
		return;
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 등급 색상
	wstrText += TTW(40002);
	E_ITEM_GRADE const eItemLv = GetItemGrade(kItem);
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 이름
	std::wstring kItemName;
	MakeItemName(kItem.ItemNo(), kEnchantInfo, kItemName);
	if(pDef->GetAbil(AT_CARD_BGNDNO) > 0)
	{
		wstrText += _T(" \n\n\n\n\n");
	}
	wstrText += kItemName;
	wstrText += TTW(40011);	//폰트 초기화
	wstrText += _T("\n");
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 설명
	const TCHAR *pText = NULL;
	int Value = pDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		wstrText += TTW(40050);
		wstrText += TTW(40051);
		wstrText += pText;
		wstrText += _T("\n");
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 수량 아이템 툴팁
	if( pDef->CanConsume() && !kFlag.Enable(TTF_NOT_METHOD) ) // 사용 가능 아이템
	{
		wstrText += _T("\n");
		wstrText += L"{C=0xFFFFA500/}";
		wstrText += TTW(4999);
	}
	if(!pDef->CanEquip() && !kFlag.Enable(TTF_NOT_AMONUT) )// 장착 가능 아이템
	{
		int const iMaxLimit = pDef->GetAbil(AT_MAX_LIMIT);
		if( iMaxLimit > 1 )
		{			
			wstrText += TTW(1306);//40064//노랑
			wstrText += TTW(5015);
			wstrText += TTW(40011);
		}
	}
	
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 판매 가격
	if(!kShopItemInfo.IsNull())
	{
		std::wstring kPrice;

		if( kShopItemInfo.iPrice )
		{
			//FormatMoney(kShopItemInfo.iPrice, kPrice);
			kPrice = GetMoneyString(kShopItemInfo.iPrice);
		}
		else
		{
			wchar_t	szCPValue[30] = {0,};
			swprintf(szCPValue, 29, TTW(5006).c_str(), kShopItemInfo.iCP * 10);
			kPrice = szCPValue;
			
		}

		wstrText += _T("\n");
		wstrText += TTW(40052);
		wstrText += TTW(40053);	
		wstrText += kPrice;
		wstrText += _T("\n");
	}
	else
	{
		bool const bIsStockShopOn = (!lwGetUIWnd("FRM_SHOP_STOCK").IsNil());

		if( !lwGetUIWnd("FRM_SHOP").IsNil() 
		||	!lwGetUIWnd("FRM_SHOP_CP").IsNil()
		||	!lwGetUIWnd("FRM_SHOP_JOB").IsNil()
		||	bIsStockShopOn)
		{
			int iSellPrice = pDef->SellPrice();

			if(bIsStockShopOn)
			{
				PgStore::CONT_GODDS::value_type kItemInfo;
				if(S_OK == g_kViewStore.GetGoods(kItem.ItemNo(), kItemInfo))
				{
					iSellPrice = kItemInfo.iSellPrice;
				}
			}

			if(iSellPrice)
			{
				__int64 iEarnPrice = 0;
				CalcSellMoney(kItem, iEarnPrice, iSellPrice, kItem.Count(), g_kPilotMan.GetPlayerUnit()->GetPremium());

				std::wstring kPrice;
				//FormatMoney(iEarnPrice, kPrice);
				kPrice = GetMoneyString(iEarnPrice);

				wstrText += _T("\n");
				wstrText += TTW(40052);
				wstrText += TTW(40053);
				wstrText += kPrice;
				wstrText += _T("\n");
			}
			else
			{
				wstrText += _T("\n");
				wstrText += TTW(40105);
				wstrText += TTW(5014);
				wstrText += _T("\n");
			}
		}
	}

	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
 	XUI::CXUI_Wnd* pWnd = XUIMgr.Call(UNI(szSuperGirlToolTipID));
	if( pWnd )
	{
		int const BGResourceNo = pDef->GetAbil(AT_CARD_BGNDNO);
		MakeToopTipIcon_Common(pWnd, BGResourceNo);

		// 출력
		pWnd->Text(wstrText);

		pWnd->Location(pt());
		pWnd->Invalidate(true);
		pWnd->SetCustomData(0, sizeof(size_t));//카테고리 설정
	}

	return;
}

void MakeToolTipText_Pet(PgBase_Item const &kOrgItem, std::wstring &wstrTextOut, std::wstring& wstrLank)
{
	PgItem_PetInfo *pkPetInfo = NULL;
	kOrgItem.GetExtInfo( pkPetInfo );

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kOrgItem.ItemNo());
	if(!pDef)
	{
		return;
	}

	GET_DEF(PgClassPetDefMgr, kClassDefMgr);

	PgClassPetDef kPetDef;
	if( pkPetInfo )
	{
		kClassDefMgr.GetDef(pkPetInfo->ClassKey(), &kPetDef);
	}

	bool b1stTypePet = (EPET_TYPE_1==kPetDef.GetPetType());

	std::wstring wstrText;
	if( pkPetInfo )
	{
		if( b1stTypePet )
		{
			wstrText += TTW(7518);
		}
		else
		{
			wstrText += TTW(7517);
		}
		
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 성장/비성장 구분
		wstrLank += TTW(7516);
		wstrLank += L"[";
		if(b1stTypePet)
		{
			if(0<kPetDef.GetAbil(AT_ADDITIONAL_INVEN_SIZE))
			{
				wstrLank += TTW(264);
			}
			else
			{
				wstrLank += TTW(7513);
			}
		}
		else
		{
			wstrLank += TTW(7514);
		}
		wstrLank += L"]";
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 이름
		wstrText += pkPetInfo->Name();
#ifndef USE_INB
		if(g_pkApp->VisibleClassNo())
		{
			wstrText += BM::vstring(pDef->No()).operator const std::wstring &();
		}
#endif
		wstrText += TTW(40011);	//폰트 초기화
		wstrText += _T("\n\n");
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 레벨
		wstrText += TTW(46);
		wstrText += L" : {C=0xFFFFFFFF/}";
		wchar_t	szTimeText[5] = {0,};
		swprintf_s(szTimeText, 4, L"%d", pkPetInfo->ClassKey().nLv);
		wstrText += szTimeText;
		wstrText += L"\n";
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 레벨
		if(!b1stTypePet)
		{ 
			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(362).c_str(), (PgClassPetDef::GetClassGrade(pkPetInfo->ClassKey().iClass) + 1));
			wstrText += TTW(361);
			wstrText += WSTR_ATTR_EXPLAIN;
			wstrText += szTemp;
			wstrText += L"\n";
		}
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

		CONT_DEFCLASS_PET_SKILL const *pkDefPetSkill = NULL;
		g_kTblDataMgr.GetContDef(pkDefPetSkill);

		GET_DEF(CSkillDefMgr, kSkillDefMgr);

		CONT_DEFCLASS_PET_SKILL::const_iterator skill_itr = pkDefPetSkill->find( kPetDef.GetSkillDefID() );
		if ( skill_itr != pkDefPetSkill->end() )
		{
			CONT_DEFCLASS_PET_SKILL::mapped_type const &kContElement = skill_itr->second;
			size_t const index = kContElement.find( PgDefClassPetSkillFinder(pkPetInfo->ClassKey().nLv) );
			if ( BM::PgApproximate::npos != index )
			{
				int iCheckFlag = 1;
				CONT_DEFCLASS_PET_SKILL::mapped_type::value_type const &kElement = kContElement.at(index);
				for ( size_t i = 0; i< PgItem_PetInfo::MAX_SKILL_SIZE ; ++i )
				{
					if ( (kElement.iSkillUseTimeBit & iCheckFlag) && 
						kElement.iSkillNo[i] )
					{
						// 툴팁을 만들기 시작
						CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(kElement.iSkillNo[i]);
						if( pkSkillDef)
						{
							BM::vstring vstrText(TTW(190));
							vstrText += L" : ";
							wchar_t const* pkName = NULL;
							if ( true == GetDefString( pkSkillDef->NameNo(), pkName ) )
							{
								vstrText.Replace( L"#NAME#", pkName );
							}
							else
							{
								vstrText.Replace( L"#NAME#", std::wstring(L"Undefined Name") );
							}

							if ( !MakeActivateSkillToolTipText_Pet( vstrText, *pkPetInfo, i ) )
							{
								vstrText += TTW(3331);
							}

							vstrText += L"\n";
							wstrText += static_cast<std::wstring>(vstrText);
						}
					}
					iCheckFlag <<= 1;
				}
			}
		}

		wstrText += L"\n";

		if( true == kOrgItem.IsUseTimeOut() )
		{
			wstrText += TTW(336);
			wstrText += L"\n";
		}
	}
	else
	{
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 이름
		wstrText += TTW(40002);
		std::wstring kItemName;
		MakeItemName(kOrgItem.ItemNo(), kOrgItem.EnchantInfo(), kItemName);
		wstrText += kItemName;
		wstrText += TTW(40011);	//폰트 초기화
		wstrText += _T("\n\n");
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 레벨
		MakeAbilString(kOrgItem, pDef, AT_LEVELLIMIT, pDef->GetAbil(AT_LEVELLIMIT), wstrText);	//착용 제한 최저 레벨
		wstrText += _T("\n");
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 어빌리티 설명
	MakeCantAbil(wstrText,pDef);
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	
	if( pkPetInfo )
	{
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 경험치
		PgClassPetDef kPetDef_NextLv;
		SClassKey kClassKey = pkPetInfo->ClassKey();
		++kClassKey.nLv;

		if ( true == kClassDefMgr.GetDef( kClassKey, &kPetDef_NextLv ) )
		{
			wstrText += ENTER_STRING;
			wstrText += TTW(180);
			double dPer = 0.0;

			__int64 const i64LevelUpExp = kPetDef_NextLv.GetAbil64( AT_EXPERIENCE );
			__int64 const i64CurrentLvUpExp = kPetDef.GetAbil64( AT_EXPERIENCE );
			__int64 const i64ExpGap = i64LevelUpExp - i64CurrentLvUpExp;
			if ( 0i64 < i64ExpGap )
			{
				__int64 const i64ExpValue = pkPetInfo->Exp() - i64CurrentLvUpExp;

				dPer = static_cast<double>(i64ExpValue) / static_cast<double>(i64ExpGap);
				dPer *= 100;
				
				if(dPer > 100)
				{
					dPer = 0.0;//100보다 넘어 가는 경우는 없으나 테스트에서 렙업을 하였다가 다운을 했을때 2222222.346이런식으로 나와 처리함
				}
			}

			wchar_t	szText[32] = {0,};
			swprintf_s( szText, 32, L" %.2f%%\n", dPer );
			wstrText += szText;
			wstrText += ENTER_STRING;
		}
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

		/*//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 스킬	
		std::wstring kPetSkillText = TTW(139);
		kPetSkillText += ENTER_STRING;
		bool bExistSkill = false;

		CONT_DEFCLASS_PET_SKILL const *pkDefPetSkill = NULL;
		g_kTblDataMgr.GetContDef(pkDefPetSkill);

		CONT_DEFCLASS_PET_SKILL::const_iterator skill_itr = pkDefPetSkill->find( kPetDef.GetSkillDefID() );
		if ( skill_itr != pkDefPetSkill->end() )
		{
			CONT_DEFCLASS_PET_SKILL::mapped_type const &kContElement = skill_itr->second;
			size_t const index = kContElement.find( PgDefClassPetSkillFinder(static_cast<short>(pkPetInfo->GetAbil(AT_LEVEL))) );
			if ( BM::PgApproximate::npos != index )
			{
				GET_DEF(CSkillDefMgr, kSkillDefMgr);
				CONT_DEFCLASS_PET_SKILL::mapped_type::value_type const &kElement = kContElement.at(index);
				for ( size_t i=0 ; i<MAX_PET_SKILLCOUNT; ++i )
				{
					int const iSkillNo = kElement.iSkillNo[i];
					if ( iSkillNo )
					{
						CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
						if(pkSkillDef)
						{
							wchar_t const* pName = NULL;
							if(GetDefString(pkSkillDef->NameNo(),pName))
							{
								bExistSkill = true;
								kPetSkillText += pName;
								kPetSkillText += ENTER_STRING;
							}
						}
					}
				}
			}
		}

		if(bExistSkill)
		{
			wstrText += kPetSkillText;
			wstrText += ENTER_STRING;
		}*/
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	}		

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 옵션 설명
	CONT_ENCHANT_ABIL kEnchantAbil;
	GetDefaultOption(kOrgItem, kEnchantAbil, true);
	//아이템 생성시 정해지는 랜덤 옵션
	if( !kEnchantAbil.empty() )
	{
		wstrText += TTW(40020);
		wstrText += TTW(40021);

		CONT_ENCHANT_ABIL::const_iterator kItor = kEnchantAbil.begin();
		while( kItor != kEnchantAbil.end() )
		{
			MakeAbilString(kOrgItem, pDef, (*kItor).wType, (*kItor).iValue, wstrText);//레벨
			++kItor;
		}
		wstrText += ENTER_STRING;
	}
	else
	{
		if(0<kPetDef.GetAbil(AT_ADDITIONAL_INVEN_SIZE))
		{
			wstrText += TTW(40020);
			wstrText += TTW(40021);

			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(271).c_str(), (kPetDef.GetAbil(AT_ADDITIONAL_INVEN_SIZE)*8));
			wstrText += szTemp;

			wstrText += ENTER_STRING;
			wstrText += ENTER_STRING;
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 설명
	const TCHAR *pText = NULL;
	int Value = pDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		wstrText += TTW(40050);
		wstrText += TTW(40051);
		wstrText += pText;
		wstrText += _T("\n");
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>전직 설명
	if(EPET_TYPE_2==kPetDef.GetPetType() || EPET_TYPE_3==kPetDef.GetPetType())
	{
		wstrText += L"\n";
		wstrText += TTW(3419);
		wstrText += L"\n";
		wstrText += L"\n";
		wstrText += TTW(3420);
		wstrText += L"\n";
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 캐쉬 아이템 한정
	std::wstring kUseLimitTime;
	if( kOrgItem.GetUseAbleTime() )
	{
		if( kOrgItem.GetUseAbleTime() <= 0 )
		{
			kUseLimitTime = TTW(89999);//빨강
			kUseLimitTime += TTW(179);
			kUseLimitTime += TTW(40011);
		}
		else
		{
			int const iDay = kOrgItem.GetUseAbleTime() / 60 / 60 / 24;
			int const iHour = kOrgItem.GetUseAbleTime() / 60 / 60 % 24;
			int const iMin = kOrgItem.GetUseAbleTime() / 60 % 60;

			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(403059).c_str(), iDay, iHour, iMin);
			kUseLimitTime = szTemp;
		}
	}
	else
	{
		bool const bIsCash = CheckIsCashItem(kOrgItem);
		if( true == bIsCash )
		{
			kUseLimitTime = TTW(1894);
		}
	}

	if( !kUseLimitTime.empty() )
	{
		wstrText += L"\n";
		wstrText += TTW(1895); //사용시간
		wstrText += L"\n";
		wstrText += TTW(40011);
		wstrText += kUseLimitTime;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	wstrTextOut+=wstrText;
}

void AddToolTip_InchantForceUpgrade(PgBase_Item const &kOrgItem, std::wstring &wstrTextOut)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kOrgItem.ItemNo());
	if(!pDef || pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) != UICT_FORCED_ENCHANT)
	{ //인챈트 점프 아이템이 아니라면 통과
		return;
	}

	//대상 아이템의 현재 인챈트 요구 레벨
	//최소 요구 레벨: AT_USE_ITEM_CUSTOM_VALUE_3, AT_USE_ITEM_CUSTOM_VALUE_4 중 최소 값
	//최대 요구 레벨: AT_USE_ITEM_CUSTOM_VALUE_1(결과 최소 레벨) - 1
	int iCustomValue1 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
	int iCustomValue2 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
	int iCustomValue3 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_3);
	int iCustomValue4 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_4);

	int iRequireLvMin = iCustomValue3;
	int iRequireLvMax = (iCustomValue4 == 0) ? (iCustomValue1 - 1) : std::max(iCustomValue3, iCustomValue4);
	int iResultLvMin = iCustomValue1;
	int iResultLvMax = std::max(iCustomValue1, iCustomValue2);


	int const iEquipPos = pDef->GetAbil(AT_EQUIP_LIMIT);
	std::wstring wstrEquipType;
	GetEqiupPosStringMonsterCard(iEquipPos, wstrEquipType);

	wstrTextOut += ENTER_STRING;
	wstrTextOut += TTW(40900); // - 사용조건 -
	wstrTextOut += TTW(40021);
	if(!wstrEquipType.empty())
	{
		wstrTextOut += TTW(40903); //착용위치:
		wstrTextOut += wstrEquipType;
		wstrTextOut += ENTER_STRING;
	}
	wstrTextOut += L"\n";
	std::wstring wstrTemp(TTW(40902)); //인챈트 등급:
	wchar_t szTemp[MAX_PATH];
	if(iRequireLvMin == iRequireLvMax)
	{
		swprintf_s(szTemp, MAX_PATH, L"+%d", iRequireLvMin); //+xx
	}
	else
	{
		swprintf_s(szTemp, MAX_PATH, L"+%d ~ +%d", iRequireLvMin, iRequireLvMax); //+xx ~ +xx
	}
	wstrTemp += szTemp;
	wstrTextOut += wstrTemp;
	wstrTextOut += L"\n\n";
	wstrTextOut += TTW(40901); //- 사용결과 -
	wstrTextOut += TTW(40021);
	wstrTemp = TTW(40902); //인챈트 등급:
	if(iResultLvMin == iResultLvMax)
	{
		swprintf_s(szTemp, MAX_PATH, L"+%d", iResultLvMin); //+xx
	}
	else
	{
		swprintf_s(szTemp, MAX_PATH, L"+%d ~ +%d", iResultLvMin, iResultLvMax); //+xx ~ +xx
	}
	wstrTemp += szTemp;
	wstrTextOut += wstrTemp;
	wstrTextOut += L"\n";
}

void AddToolTip_Manufacture(CItemDef const *pDef, std::wstring &wstrTextOut)
{
	if(UICT_MANUFACTURE != pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE))
	{
		return;
	}

	int iManType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
	int iTTNo = 0;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	PgMySkill* pkSkill = NULL;
	if(pkPlayer)
	{
		pkSkill = pkPlayer->GetMySkill();
	}

	switch(iManType)
	{
	case EMANT_SPELL:
		{
			if(pkSkill && pkSkill->GetLearnedSkill(11901))
			{
				iTTNo = 26001;
			}
		}break;
	case EMANT_COOK:
		{
			if(pkSkill && pkSkill->GetLearnedSkill(11701))
			{
				iTTNo = 26002;
			}
		}break;
	case EMANT_WORKMANSHIP:
		{
			if(pkSkill && pkSkill->GetLearnedSkill(11801))
			{
				iTTNo = 26003;
			}
		}break;
	}
	if(iTTNo != 0)
	{
		wstrTextOut += ENTER_STRING;
		wstrTextOut += L"{C=0xFFFFFF00/}";
		wstrTextOut += TTW(iTTNo);
		wstrTextOut += ENTER_STRING;
	}
}


typedef struct tagEffectInfo
{
	tagEffectInfo()
	{}

	tagEffectInfo(tagEffectInfo const & kCopy)
	{
		kIconInfo = kCopy.kIconInfo;
		kText = kCopy.kText;
	}

	tagEffectInfo(SIconInfo const & _kInfo, std::wstring const & _kText)
	{
		kIconInfo = _kInfo;
		kText = _kText;
	}

	SIconInfo kIconInfo;
	std::wstring kText;
}SEffectInfo;
typedef std::vector<SEffectInfo> CONT_EFFECTINFO;

void MakeToolTipText_SkillExtend(PgBase_Item const &kOrgItem, lwPoint2 &pt, SToolTipFlag const &kFlag)
{
	int const iOutlineSize = 12;//XUI_ToolTip 정의

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(UNI(szSkillExtendToolTipID));
	if( !pkWnd ){ return; }

	std::wstring wstrText = _T("");
	PgBase_Item kItem = kOrgItem;
	SEnchantInfo const &kEnchantInfo = kItem.EnchantInfo();
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( !pDef ){ return; }

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 등급 색상
	int const iGrade = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
	BM::vstring vStr(TTW(790433));
	vStr.Replace(L"#COLOR#", TTW(790445+iGrade));
	wstrText += std::wstring(vStr);
	E_ITEM_GRADE const eItemLv = GetItemGrade(kItem);
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 이름
	std::wstring kItemName;
	MakeItemName(kItem.ItemNo(), kEnchantInfo, kItemName);
	if(pDef->GetAbil(AT_CARD_BGNDNO) > 0)
	{
		wstrText += _T(" \n\n\n");
	}
	wstrText += L"[";
	wstrText += TTW(790434+iGrade);
	wstrText += L"]";
	wstrText += kItemName;
	wstrText += TTW(40011);	//폰트 초기화
	wstrText += _T("\n");
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 어빌리티 설명
	MakeCantAbil(wstrText,pDef);
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 설명
	const TCHAR *pText = NULL;
	int Value = pDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		wstrText += _T("\n");
		wstrText += TTW(40050);
		wstrText += TTW(40051);
		wstrText += pText;
		wstrText += _T("\n");
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 효과 설명
	POINT2 kSlotSize;

	CONT_EFFECTINFO kContEffectInfo;

	CONT_DEFSKILLEXTENDITEM const* kContDefSkillExtend;
	g_kTblDataMgr.GetContDef(kContDefSkillExtend);

	GET_DEF(CSkillDefMgr, kSkillDefMgr);

	if(IG_SEAL != eItemLv && kContDefSkillExtend)//봉인은 추가 옵션이 보이지 않음.
	{
		int const iExtendIdx = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
		
		CONT_DEFSKILLEXTENDITEM::const_iterator kDefSkillExtend_iter = kContDefSkillExtend->find( iExtendIdx );
		if( kDefSkillExtend_iter!=kContDefSkillExtend->end() )
		{
			CONT_DEFSKILLEXTENDITEM::mapped_type const & kSkillExtendItem = kDefSkillExtend_iter->second;
			
			bool bFristOption = false;
			for(CONT_EXTEND_SKILLSET::const_iterator skillno_iter=kSkillExtendItem.kCont.begin();
				skillno_iter!=kSkillExtendItem.kCont.end();++skillno_iter)
			{
				PgSkillTree::stTreeNode *pTreeNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(*skillno_iter));
				CSkillDef const * pkSkillDef = NULL;
				int iMaxLevel = MAX_SKILL_LEVEL;
				if( pTreeNode && pTreeNode->GetSkillDef() )
				{
					pkSkillDef = pTreeNode->GetSkillDef();
					iMaxLevel = pTreeNode->GetMaxSkillLevel();
				}
				else
				{
					pkSkillDef = kSkillDefMgr.GetDef(*skillno_iter);
				}
				
				if( !pkSkillDef ){ continue; }

				std::wstring kClassName;
				MakeSkillLimitClassText(pkSkillDef, kClassName);

				wchar_t const* pSkillName = NULL;
				GetDefString(pkSkillDef->NameNo(),pSkillName);

				BM::vstring vStr(TTW(790461));
				vStr.Replace(L"#CLASS#", kClassName);
				vStr.Replace(L"#SKILL#", pSkillName);
				vStr.Replace(L"#MAX_LEVEL#", iMaxLevel);
				vStr.Replace(L"#EXTEND_LEVEL#", MAX_SKILL_LEVEL+iGrade);

				CONT_EFFECTINFO::value_type kElement;
				
				kElement.kIconInfo.iIconKey = pkSkillDef->No();
				kElement.kIconInfo.iIconResNumber = pkSkillDef->RscNameNo();
				kElement.kText = static_cast<std::wstring>(vStr);

				kContEffectInfo.push_back(kElement);

				if( !bFristOption )
				{
					bFristOption = true;
					wstrText += _T("\n");
					wstrText += TTW(40068);
					if( iGrade )
					{
						BM::vstring vStr(TTW(790463));
						vStr.Replace(L"#LEVEL#", MAX_SKILL_LEVEL+iGrade);
						wstrText += static_cast<std::wstring>(vStr);
					}

					pkWnd->Text(wstrText);
					XUI::CXUI_Style_String kStyleName = pkWnd->StyleText(); //스타일 텍스트 뽑고
					POINT kPoint = Pg2DString::CalculateOnlySize(kStyleName);
					kSlotSize.Set(kPoint.x, kPoint.y+iOutlineSize); //길이 계산 하고
				}
				wstrText += TTW(790462);//슬롯 공간
			}
			wstrText += TTW(40021);
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 가격
	bool const bIsStockShopOn = (!lwGetUIWnd("FRM_SHOP_STOCK").IsNil());
	if( !lwGetUIWnd("FRM_SHOP").IsNil() 
	||	!lwGetUIWnd("FRM_SHOP_CP").IsNil()
	||	!lwGetUIWnd("FRM_SHOP_JOB").IsNil()
	||	bIsStockShopOn)
	{
		int iSellPrice = pDef->SellPrice();

		if(bIsStockShopOn)
		{
			PgStore::CONT_GODDS::value_type kItemInfo;
			if(S_OK == g_kViewStore.GetGoods(kItem.ItemNo(), kItemInfo))
			{
				iSellPrice = kItemInfo.iSellPrice;
			}
		}

		if(iSellPrice)
		{
			__int64 iEarnPrice = 0;
			CalcSellMoney(kItem, iEarnPrice, iSellPrice, kItem.Count(), g_kPilotMan.GetPlayerUnit()->GetPremium());

			std::wstring kPrice;
			//FormatMoney(iEarnPrice, kPrice);
			kPrice = GetMoneyString(iEarnPrice);

			wstrText += _T("\n");
			wstrText += TTW(40052);
			wstrText += TTW(40053);
			wstrText += kPrice;
			wstrText += _T("\n");
		}
		else
		{
			wstrText += _T("\n");
			wstrText += TTW(40105);
			wstrText += TTW(5014);
			wstrText += _T("\n");
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 수량 아이템 툴팁
	if( pDef->CanConsume() && !kFlag.Enable(TTF_NOT_METHOD) ) // 사용 가능 아이템
	{
		wstrText += _T("\n");
		wstrText += L"{C=0xFFFFA500/}";
		wstrText += TTW(4999);
	}
	if(!pDef->CanEquip() && !kFlag.Enable(TTF_NOT_AMONUT))// 장착 가능 아이템
	{
		int const iMaxLimit = pDef->GetAbil(AT_MAX_LIMIT);
		if( iMaxLimit > 1 )
		{			
			wstrText += TTW(1306);//40064//노랑
			wstrText += TTW(5015);
			wstrText += TTW(40011);
		}
	}
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 수량 아이템 툴팁

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 툴팁 입력
	int const BGResourceNo = pDef->GetAbil(AT_CARD_BGNDNO);
	MakeToopTipIcon_Common(pkWnd, BGResourceNo);

	XUI::CXUI_Wnd* kFrmPos = pkWnd->GetControl(L"FRM_SLOT_POS");
	if(kFrmPos)
	{
		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(kFrmPos->GetControl(L"BLD_SLOT"));
		if(pkBuild)
		{
			int const iMaxSlot = pkBuild->CountY();
			int iSlot = 0;
			int iSlotSizeY = 0;
			for(CONT_EFFECTINFO::const_iterator c_iter=kContEffectInfo.begin(); c_iter!=kContEffectInfo.end() && iSlot<iMaxSlot;++c_iter,++iSlot)
			{
				BM::vstring kStr(L"FRM_SLOT");
				kStr += iSlot;

				XUI::CXUI_Wnd* kFrm = kFrmPos->GetControl(kStr);
				if( !kFrm ){ continue; }

				iSlotSizeY += pkBuild->GabY();
				
				XUI::CXUI_Icon *pkIcon = dynamic_cast<XUI::CXUI_Icon*>(kFrm->GetControl(L"ICN_SLOT"));
				XUI::CXUI_Wnd *pkText = kFrm->GetControl(L"FRM_TEXT");

				if(pkIcon)
				{
					SIconInfo kIconInfo;
					kIconInfo = pkIcon->IconInfo();
					kIconInfo.iIconKey = c_iter->kIconInfo.iIconKey;
					kIconInfo.iIconResNumber = c_iter->kIconInfo.iIconResNumber;

					pkIcon->SetIconInfo(kIconInfo);
				}

				if(pkText)
				{
					pkText->Text(c_iter->kText);
				}

				kFrm->Visible(true);
			}

			int const iLastSlot = iSlot;
			kFrmPos->Location(kFrmPos->Location().x,kSlotSize.y);
			kFrmPos->Size(kFrmPos->Size().x, iSlotSizeY);

			for(;iSlot<iMaxSlot;++iSlot)
			{
				BM::vstring kStr(L"FRM_SLOT");
				kStr += iSlot;

				XUI::CXUI_Wnd* kFrm = kFrmPos->GetControl(kStr);
				if( !kFrm ){ continue; }

				kFrm->Visible(false);
			}
		}
	}

	// 출력
	pkWnd->Text(wstrText);

	XUI::CXUI_Style_String kStyleName = pkWnd->StyleText(); //스타일 텍스트 뽑고
	POINT kPoint = Pg2DString::CalculateOnlySize(kStyleName);
	
	pkWnd->Size(pkWnd->Size().x, kPoint.y+(iOutlineSize*2));
	pkWnd->Location(pt());
	pkWnd->Invalidate(true);
	pkWnd->SetCustomData(0, sizeof(size_t));//카테고리 설정

	return;
}

typedef std::set<int> CONT_MAP_GROUP_NAME;

bool MakeToolTipTextDropInfo(CItemDef const *pDef,std::wstring & wstrTextOut)
{
	if(NULL == pDef)
	{
		return false;
	}

	CONT_SET_DATA const & kCont = pDef->GetDropGround();

	CONT_MAP_GROUP_NAME kContGroup;
	GET_DEF(PgDefMapMgr, kDefMap);

	for(CONT_SET_DATA::const_iterator gnd_iter = kCont.begin();gnd_iter != kCont.end();++gnd_iter)
	{
		int const iMapGroupName = kDefMap.GetAbil((*gnd_iter),AT_MAP_GROUP_NAME_NO);
		if(0 < iMapGroupName)
		{
			kContGroup.insert(iMapGroupName);
		}
	}

	int i = 0;
	for(CONT_MAP_GROUP_NAME::const_iterator iter = kContGroup.begin();iter != kContGroup.end(); ++iter, ++i)
	{
		if(0 == i)
		{
			wstrTextOut += TTW(40071);
			wstrTextOut += _T("\n");
			wstrTextOut += TTW(40011);
		}

		if(1 == (i%2))
		{
			wstrTextOut += _T(", ");
		}

		if((0 < i) && (0 == (i%2)))
		{
			wstrTextOut += _T("\n");
		}

		const std::wstring *pkName = NULL;
		if(false == GetDefString((*iter), pkName))
		{
			continue;
		}

		wstrTextOut += *pkName;
	}

	if(false == kContGroup.empty())
	{
		return true;
	}
	return false;
}

void MakeItemEffectAbilText(PgBase_Item const & kItem,CItemDef const *pkItemDef, std::wstring &wstrTextOut)
{
	{// 일반공격시 발동 이팩트
		std::wstring kString;
		for(int i = 0;i < MAX_ITEM_EFFECT_ABIL_NUM;++i)
		{
			MakeItemEffectAbilString(pkItemDef, i, AT_N_ATTACK_EFFECT_NO_MIN, AT_N_ATTACK_EFFECT_RATE_MIN, AT_N_ATTACK_EFFECT_TARGET_MIN, kString);
		}

		if(false == kString.empty())
		{
			wstrTextOut += _T("\n");
			wstrTextOut += TTW(5031);
			wstrTextOut += _T("\n");
			wstrTextOut += kString;
			wstrTextOut += _T("\n\n");
		}
	}

	{// 크리티컬시 발동 이팩트
		std::wstring kString;
		for(int i = 0;i < MAX_ITEM_EFFECT_ABIL_NUM;++i)
		{
			MakeItemEffectAbilString(pkItemDef, i, AT_C_ATTACK_EFFECT_NO_MIN, AT_C_ATTACK_EFFECT_RATE_MIN, AT_C_ATTACK_EFFECT_TARGET_MIN, kString);
		}

		if(false == kString.empty())
		{
			wstrTextOut += _T("\n");
			wstrTextOut += TTW(5032);
			wstrTextOut += _T("\n");
			wstrTextOut += kString;
			wstrTextOut += _T("\n\n");
		}
	}

	{// 피격상태에 발동 이팩트
		std::wstring kString;
		for(int i = 0;i < MAX_ITEM_EFFECT_ABIL_NUM;++i)
		{
			MakeItemEffectAbilString(pkItemDef, i, AT_BEATTACKED_EFFECT_NO_MIN, AT_BEATTACKED_EFFECT_RATE_MIN, AT_BEATTACKED_EFFECT_TARGET_MIN, kString);
		}

		if(false == kString.empty())
		{
			wstrTextOut += _T("\n");
			wstrTextOut += TTW(5033);
			wstrTextOut += _T("\n");
			wstrTextOut += kString;
			wstrTextOut += _T("\n\n");
		}
	}

	{// 블럭상태에 발동 이팩트
		std::wstring kString;
		for(int i = 0;i < MAX_ITEM_EFFECT_ABIL_NUM;++i)
		{
			MakeItemEffectAbilString(pkItemDef, i, AT_BLOCKED_EFFECT_NO_MIN, AT_BLOCKED_EFFECT_RATE_MIN, AT_BLOCKED_EFFECT_TARGET_MIN, kString);
		}

		if(false == kString.empty())
		{
			wstrTextOut += _T("\n");
			wstrTextOut += TTW(5034);
			wstrTextOut += _T("\n");
			wstrTextOut += kString;
			wstrTextOut += _T("\n\n");
		}
	}
}

void MakeBaseAbilStr(PgBase_Item const& kItem, CItemDef const* pDef, std::wstring& kOutText)
{
	if( !pDef ){ return; }

	std::wstring kAbilString;
	SAbilIterator kAbil_itor;
	pDef->FirstAbil(&kAbil_itor);
	while(pDef->NextAbil(&kAbil_itor))
	{
		WORD wAbilType =  kAbil_itor.wType;
		switch(wAbilType)
		{
		case AT_PHY_ATTACK_MIN:
		case AT_PHY_ATTACK_MAX:
		case AT_MAGIC_ATTACK_MIN:
		case AT_MAGIC_ATTACK_MAX:
		case AT_PHY_DEFENCE:
		case AT_MAGIC_DEFENCE:
		case AT_ATTACK_RANGE:
			{// 무시할 어빌
			}break;
		default:
			{
				if( CheckAddLineAbilType(wAbilType) )
				{
					std::wstring kStr;
					int const iValue = pDef->ImproveAbil(wAbilType, kItem);
					HRESULT const hRet = MakeAbilValueString(kItem, pDef, wAbilType, iValue, kStr, g_bIsEqComp);
					switch( hRet )
					{
					case S_FALSE:
						{
							if(!kStr.empty() && 0 != iValue)
							{
								std::wstring kSign(TTW(1548));
								//HP/MP 회복 속도 표기를 '-'에서 '+'로 수정. '-'일때는 값이 두번 들어가 '--'가 되어 따로 넣지 않음.
								if(wAbilType == AT_R_HP_RECOVERY_INTERVAL || wAbilType == AT_R_MP_RECOVERY_INTERVAL)
								{
									if(0 > iValue)
									{// -
										kSign += L" -";
									}
								}
								else
								{
									if(0 < iValue)
									{// +
										kSign += L" +";
									}
								}
								if( !MakeAbilNameString( wAbilType, kAbilString ) )
								{
									break;
								}
								kStr = kSign + kStr;
							}
						}// break을 사용하지 않음
					case S_OK:
						{
							kStr += L"\n";
							kAbilString += _T(":");
							kAbilString += kStr;
						}break;
					default:
						{

						}break;
					}
				}
			}break;
		}
	}

	if( !kAbilString.empty() )
	{
		kOutText += kAbilString;
	}
}


void MakeEquipAddOption(PgBase_Item const& kItem, CItemDef const* pDef, bool const bOrtherActor, std::wstring& wstrText, bool const bColor)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
	int const iSetNo = kItemSetDefMgr.GetItemSetNo(kItem.ItemNo());
	if( !iSetNo ){ return; }

	CONT_DEF_ITEM_SET const *pkDefItemSet = NULL;
	CONT_TBL_DEF_ITEM_OPTION_ABIL const *pkDefItemOptAbil = NULL;

	g_kTblDataMgr.GetContDef(pkDefItemSet);
	g_kTblDataMgr.GetContDef(pkDefItemOptAbil);

	CONT_DEF_ITEM_SET::const_iterator def_set_itor = pkDefItemSet->find(iSetNo);
	if(def_set_itor == pkDefItemSet->end()){ return; }
	
	std::wstring kSetText = _T("");

	//Def에서 찾는다.
	const CItemSetDef *pkOrgSetDef = kItemSetDefMgr.GetDef(iSetNo);
	if( !pkOrgSetDef ){ return; }

	//장착 중인 아이템을 가져 온다.
	CONT_HAVE_ITEM_DATA kContHaveItems;
	if( bOrtherActor )
	{
		g_kOtherViewInv.GetItems(IT_FIT, kContHaveItems);
		g_kOtherViewInv.GetItems(IT_FIT_CASH, kContHaveItems);
	}
	else
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}
		pkPlayer->GetInven()->GetItems(IT_FIT, kContHaveItems);
		pkPlayer->GetInven()->GetItems(IT_FIT_CASH, kContHaveItems);
	}

	bool bCompleteSet = false;
	bool bSetColor = true;
	int const iEquipPiece = pkOrgSetDef->CheckNeedItem(kContHaveItems, g_kPilotMan.GetPlayerUnit(), bCompleteSet);

	SAbilIterator kItor;
	pkOrgSetDef->FirstAbil(&kItor);

	PgBase_Item kItemForSetText = kItem;
	SEnchantInfo kForSetEnchantInfo;
	kForSetEnchantInfo.IsCurse(false);

	kItemForSetText.EnchantInfo(kForSetEnchantInfo);


	//옵션 설명서는 굉장히 예외 처리된 것이므로 계산식이 바뀌면 툴팁도 같이 바꿔야한다.
	{
		size_t const iOnePiece = 1;
		size_t const iTotalNeedCount = pkOrgSetDef->NeedItem().size();
		int const iCurIdx = 0; // 첫번째 것만 검사
		CONT_TBL_DEF_ITEM_OPTION_ABIL::const_iterator def_opt_abil_itor = pkDefItemOptAbil->find(def_set_itor->second.aiAbilNo[iCurIdx]);

		int iLastDefPiece = 0;
		if(def_opt_abil_itor != pkDefItemOptAbil->end())
		{
			CONT_TBL_DEF_ITEM_OPTION_ABIL_PIECE const& rkContPieceAbil = (*def_opt_abil_itor).second.kContPieceAbil;
			CONT_TBL_DEF_ITEM_OPTION_ABIL_PIECE::const_iterator pieceabil_iter = rkContPieceAbil.begin();
			while( rkContPieceAbil.end() != pieceabil_iter )
			{
				CONT_TBL_DEF_ITEM_OPTION_ABIL_PIECE::value_type const& rkPieceAbil = (*pieceabil_iter);
				for(int j = 0; MAX_OPTION_ABIL_ARRAY > j; j++)
				{
					int const iTargetAbilNo = rkPieceAbil.aiType[j];
					int const iTargetAbilValue = rkPieceAbil.aiValue[j];
					if( iTargetAbilNo )
					{
						CItemSetDef const *pEquipkOrgSetDef = kItemSetDefMgr.GetEquipAbilDef(iSetNo, iEquipPiece);
						if( iLastDefPiece != rkPieceAbil.iPieceCount )
						{
							iLastDefPiece = rkPieceAbil.iPieceCount;
							if(	iEquipPiece >= rkPieceAbil.iPieceCount )// 
							{
								if(bColor) wstrText += TTW(CONTENTS_COLOR_WHITE);
								bSetColor = false;
								
							}
							else
							{
								if(bColor) wstrText += TTW(40061);//40061//회색
								bSetColor = true;
							}

							if( iOnePiece < iTotalNeedCount )
							{
								wstrText += (std::wstring)BM::vstring(iLastDefPiece);
								wstrText += TTW(40067);
							}
						}
						else
						{// 같은 피스에 능력이 더있을 경우
							iLastDefPiece = rkPieceAbil.iPieceCount;
							if(	iEquipPiece >= rkPieceAbil.iPieceCount )// 
							{
								bSetColor = false;								
							}
							else
							{
								if(bColor) wstrText += TTW(40061);//40061//회색
								bSetColor = true;
							}
							std::wstring::reverse_iterator kIter = wstrText.rbegin();
							if( wstrText.rend() != kIter && L'\n' == (*kIter)	)
							{// 개행 문자를 제거하고 능력치를 붙일수 있게 하고
								(*kIter) = L','; 
								wstrText +=L' ';
							}
						}

						if( (AT_EFFECTNUM1 <= iTargetAbilNo  && AT_EFFECTNUM10 >= iTargetAbilNo)
							&& 0 < iTargetAbilValue
							)
						{
							//MakeSetItemOptionString(kItemForSetText, pDef, iTargetAbilValue, wstrText, bColor);
							GET_DEF(CSkillDefMgr, kSkillDefMgr);
							CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iTargetAbilValue);
							if( pSkillDef )
							{
								wchar_t const* pName = NULL;
								GetDefString(pSkillDef->NameNo(), pName);
								if( pName )
								{
									wstrText += pName;
									wstrText += L" ";
									if(bColor) wstrText += L"{C=0xFF000000}";
									if( 0 == pSkillDef->GetAbil(AT_IGNORE_SKILLLV_TOOLTIP) )
									{
										WCHAR	temp[500] = {0,};
										swprintf(temp, 500, _T("%s %d"), TTW(224).c_str(), pSkillDef->GetAbil(AT_LEVEL));
										wstrText += temp;
									}
									wstrText += L"\n";
								}
							}
						}
						TBL_DEF_ITEM_OPTION_ABIL kOptionAbil = def_opt_abil_itor->second;
						MakeAbilString(kItemForSetText, pDef, iTargetAbilNo, iTargetAbilValue, wstrText, &kOptionAbil, NULL, false, true, 0, true, bColor,bSetColor);
					}
				}
				++pieceabil_iter;
			}
		}
	}
}

bool GetAbilFromSetItemOption(TBL_DEF_ITEM_OPTION_ABIL* pkOptionAbil, int const iAbilType, int& iResultValue)
{
	if(pkOptionAbil)
	{
		CONT_TBL_DEF_ITEM_OPTION_ABIL_PIECE::const_iterator iter = pkOptionAbil->kContPieceAbil.begin();
		while( pkOptionAbil->kContPieceAbil.end() != iter )
		{
			for(int i = 0; MAX_OPTION_ABIL_ARRAY > i; ++i)
			{
				int const iTargetAbilNo = (*iter).aiType[i];
				if(iAbilType == iTargetAbilNo)
				{
					iResultValue = (*iter).aiValue[i];
					return true;
				}
			}
		}
	}
	iResultValue= 0;
	return false;
}

void MakeSetItemAbilString(PgBase_Item const& kItem, CItemDef const* pDef, bool const bOrtherActor, std::wstring& wstrText, bool bSetItemView = true)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
	int const iSetNo = kItemSetDefMgr.GetItemSetNo(kItem.ItemNo());
	if( iSetNo )
	{//세트 번호 있다
		std::wstring kSetText = _T("");

		//Def에서 찾는다.
		const CItemSetDef *pkOrgSetDef = kItemSetDefMgr.GetDef(iSetNo);
		if(pkOrgSetDef)
		{//Def에 있다
			//장착 중인 아이템을 가져 온다.
			CONT_HAVE_ITEM_DATA kContHaveItems;
			if( bOrtherActor )
			{
				g_kOtherViewInv.GetItems(IT_FIT, kContHaveItems);
				g_kOtherViewInv.GetItems(IT_FIT_CASH, kContHaveItems);
				//kContHaveItems.insert(std::make_pair());
			}
			else
			{
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pkPlayer )
				{
					return;
				}
				pkPlayer->GetInven()->GetItems(IT_FIT, kContHaveItems);
				pkPlayer->GetInven()->GetItems(IT_FIT_CASH, kContHaveItems);
			}

			//세트 장비 리스트를 가져온다
			CONT_HAVE_ITEM_NO const &rkNeedItems = pkOrgSetDef->NeedItem();
			int iHaveSetCount = 0;

			//세트에 해당하는 것만 찾을까? 텍스트를 나눠야 하나?
			CONT_HAVE_ITEM_NO::const_iterator need_itor = rkNeedItems.begin();
			while(need_itor != rkNeedItems.end())
			{
				int const iNeedItemNo = (*need_itor);
				
				//장비중 목록에서 찾는다
				CONT_HAVE_ITEM_DATA::const_iterator have_cont = kContHaveItems.find(iNeedItemNo);
				if( kContHaveItems.end() != have_cont )//착용중인데
				{//
					++iHaveSetCount;
					if(false == PgInventoryUtil::CheckDisableItem((*have_cont).second,g_kPilotMan.GetPlayerUnit()))
					{
						kSetText += TTW(40062);//40062//파랑
					}
					else
					{
						kSetText += TTW(89999);//빨강
					}
				}
				else
				{
					kSetText += TTW(40061);//40061//회색
				}
				
				//GET_DEF(CItemDefMgr, kItemDefMgr);
				wchar_t const* pText = NULL;
				CItemDef const *pDef = kItemDefMgr.GetDef(iNeedItemNo);
				if( GetDefString(pDef->NameNo(), pText) )
				{
					kSetText += pText;
					kSetText += _T("\n");
				}
				++need_itor;
			}

			//세트 텍스트 출력 시작
			if( bSetItemView )
			{
				wstrText += _T("\n");
				wstrText += TTW(40064);//40064//노랑			
				const wchar_t *pText = NULL;
				if(GetDefString(pkOrgSetDef->NameNo(), pText))
				{
					wstrText += TTW(5204);
				}
				else
				{
					wstrText += _T("Error StringNo!");
				}

				wchar_t szTemp[MAX_PATH] = {0,};
				swprintf_s(szTemp, MAX_PATH, _T("(%d/%d)"), iHaveSetCount, rkNeedItems.size());
				wstrText += szTemp;
				wstrText += _T("\n");
				wstrText += kSetText;
			}

			wstrText += TTW(40064);//40064//노랑
			wstrText += _T("\n");
			wstrText += TTW(5205);//40065//추가세트 옵션
			wstrText += _T("\n");
			
			MakeEquipAddOption(kItem, pDef, bOrtherActor, wstrText);
		}
	}
}

void MakeClassLimitText(__int64 i64HumanClassLimit, __int64 i64DragonClassLimit, std::wstring &wstrText)
{
	PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		int PlayerClass = pPlayer->GetAbil(AT_CLASS);
		if( PlayerClass < EPCLASS_TYPE::UCLASS_SHAMAN )
		{// 인간족
			if( !IS_CLASS_LIMIT(i64HumanClassLimit, PlayerClass) )
			{
				wstrText += TTW(CONTENTS_COLOR_RED);
			}
		}
		else
		{// 용족
			PlayerClass = PlayerClass - EPCLASS_TYPE::UCLASS_SHAMAN + 1;				
			if( !IS_CLASS_LIMIT(i64DragonClassLimit, PlayerClass) )
			{
				wstrText += TTW(CONTENTS_COLOR_RED);
			}
		}
	}

	bool HumanClassLimit = true;
	bool DragonClassLimit = true;
	if( UCLIMIT_ALL_HUMAN == i64HumanClassLimit && UCLIMIT_ALL_HUMAN == i64DragonClassLimit )
	{//모든 직업
		wstrText += TTW(30253);
		wstrText += _T("\n");
		return;
	}
	else if( UCLIMIT_ALL_HUMAN == i64HumanClassLimit )
	{// 모든 인간 직업
		wstrText += TTW(30254);
		wstrText += _T(" ");
		HumanClassLimit = false;
	}
	else if( UCLIMIT_ALL_HUMAN == i64DragonClassLimit )
	{// 모든 용족 직업
		wstrText += TTW(30255);
		wstrText += _T(" ");
		DragonClassLimit = false;
	}

	if( HumanClassLimit )
	{
		for(int i = 1; i < UCLASS_MAX; ++i)
		{
			if(1i64<<i & i64HumanClassLimit)
			{
				wstrText += TTW(30000+i);
				wstrText += _T(" ");
			}
		}
	}
	if( DragonClassLimit )
	{
		for(int i = 1; i < UCLASS_MAX; ++i)
		{
			if(1i64<<i & i64DragonClassLimit)
			{
				wstrText += TTW(30050+i);
				wstrText += _T(" ");
			}
		}
	}
	wstrText += _T("\n");
	wstrText += TTW(CONTENTS_COLOR_WHITE);
}

void lwCallEventScheduleToolTip(int const EventNo, lwPoint2 &pt)
{
	CONT_DEF_EVENT_SCHEDULE const* pEventSchedule = NULL;
	g_kTblDataMgr.GetContDef(pEventSchedule);
	if( NULL == pEventSchedule )
	{
		return;
	}
	
	CONT_DEF_EVENT_SCHEDULE::const_iterator iter = pEventSchedule->find( EventNo );
	if( iter == pEventSchedule->end() )
	{
		return;
	}

	PgPlayer const* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return;
	}

	// 모드 이름 //
	std::wstring wstrTextOut = TTW(40102); // 이름용 폰트
	if( iter->second.EventType == _T("BossBattle") )
	{
		wstrTextOut += TTW(302106);
	}
	else if( iter->second.EventType == _T("BattleSquare") )
	{
		wstrTextOut += TTW(460014);
	}
	else if( iter->second.EventType == _T("LoveMode") )
	{
		wstrTextOut += TTW(400368);
	}
	else if( iter->second.EventType == _T("DominationMode") )
	{
		wstrTextOut += TTW(400363);
	}
	else if( iter->second.EventType == _T("Race") )
	{
		wstrTextOut += TTW(302303);
	}

	wstrTextOut += TTW(226); // 폰트 초기화
	wstrTextOut += ENTER_STRING;
	wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
	wstrTextOut += ENTER_STRING;
	

	// 이벤트기간 //
	if( !iter->second.StartDate.IsNull() || !iter->second.EndDate.IsNull() )
	{
		wstrTextOut += TTW(1989);
		wstrTextOut += _T(" : ");
		wstrTextOut += ENTER_STRING;

		if( !iter->second.StartDate.IsNull() )
		{
			wstrTextOut += (std::wstring const&)(BM::vstring)iter->second.StartDate.year;
			wstrTextOut += _T("-");
			wstrTextOut += (std::wstring const&)(BM::vstring)iter->second.StartDate.month;
			wstrTextOut += _T("-");
			wstrTextOut += (std::wstring const&)(BM::vstring)iter->second.StartDate.day;
		}

		wstrTextOut += _T(" ~ ");

		if( !iter->second.EndDate.IsNull() )
		{
			if(iter->second.EndDate.year >= 2100)
			{ //무제한으로 간주
				wstrTextOut += TTW(193);
			}
			else
			{
				wstrTextOut += (std::wstring const&)(BM::vstring)iter->second.EndDate.year;
				wstrTextOut += _T("-");
				wstrTextOut += (std::wstring const&)(BM::vstring)iter->second.EndDate.month;
				wstrTextOut += _T("-");
				wstrTextOut += (std::wstring const&)(BM::vstring)iter->second.EndDate.day;
			}
		}
		wstrTextOut += ENTER_STRING;
	}
	// 이벤트요일 //
	if( false == iter->second.DayofWeek.empty() )
	{
		wstrTextOut += TTW(302000);
		
		bool DayofWeek[7] = { false };
		std::vector< std::wstring >::const_iterator day_iter;
		for( day_iter = iter->second.DayofWeek.begin(); day_iter != iter->second.DayofWeek.end(); ++day_iter )
		{
			if( (*day_iter) == L"MON" )
			{
				DayofWeek[0] = true;
			}
			else if( (*day_iter) == L"TUE" )
			{
				DayofWeek[1] = true;
			}
			else if( (*day_iter) == L"WED" )
			{
				DayofWeek[2] = true;
			}
			else if( (*day_iter) == L"THU" )
			{
				DayofWeek[3] = true;
			}
			else if( (*day_iter) == L"FRI" )
			{
				DayofWeek[4] = true;
			}
			else if( (*day_iter) == L"SAT" )
			{
				DayofWeek[5] = true;
			}
			else if( (*day_iter) == L"SUN" )
			{
				DayofWeek[6] = true;
			}
		}

		for( int day = 0; day < 7; ++day )
		{
			if( !DayofWeek[day] )
			{
				wstrTextOut += TTW(CONTENTS_COLOR_RED);
			}
			wstrTextOut += TTW(302016 + day);
			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
			if( 6 != day )
			{
				wstrTextOut += _T("/");
			}
		}
		wstrTextOut += ENTER_STRING;
	}
	// 이벤트시간 //
	{
		wstrTextOut += TTW(302001);

		SYSTEMTIME SysTime;
		g_kEventView.GetLocalTime(&SysTime);
		BM::DBTIMESTAMP_EX NowTime(SysTime);

		BM::DBTIMESTAMP_EX AllTime(iter->second.StartTime);

		//이벤트 주기 개수 만큼 시간대를 만들어 준다
		DWORD const PlayTimeSec = iter->second.EventPeriod;
		DWORD const TotalEventSec = PlayTimeSec * iter->second.EventCount;

		BM::DBTIMESTAMP_EX EndTime(iter->second.StartTime);
		EndTime.hour += TotalEventSec / 3600;
		EndTime.minute += (TotalEventSec % 3600) / 60;
		if(EndTime.minute >= 60) //분이 60을 넘어가면 그만큼 시간을 플러스 해준다
		{
			EndTime.hour += static_cast<int>(EndTime.minute / 60);
			EndTime.minute = EndTime.minute % 60;
		}
		if(EndTime.hour >= 24) //시가 24를 넘어갈 경우
		{ //넘어가는 시간은 그냥 24시까지로 잘라주자.. 만약 요일 추가가 되어야 한다면 이곳을 수정
			EndTime.hour = 23;
			EndTime.minute = 59;
		}

		while( ( PlayTimeSec > 0 ) && ( AllTime < EndTime ) )
		{
			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);

			if( AllTime != iter->second.StartTime )
			{
				wstrTextOut += _T("/");
			}

			{// 입장대기 시간까지 넘어가야 시간을 빨간색으로 표시한다.
				SSIMPLETIME CheckTime(AllTime);
				CheckTime.byHour += g_kEventView.VariableCont().CommunityEventWaitTime / 3600;
				CheckTime.byMin += (g_kEventView.VariableCont().CommunityEventWaitTime % 3600) / 60;
				if(CheckTime.byMin >= 60) //분이 60을 넘어가면 그만큼 시간을 플러스 해준다
				{
					CheckTime.byHour += static_cast<int>(CheckTime.byMin / 60);
					CheckTime.byMin = CheckTime.byMin % 60;
				}
				if(CheckTime.byHour >= 24) //시가 24를 넘어갈 경우
				{ //넘어가는 시간은 그냥 24시까지로 잘라주자.. 만약 요일 추가가 되어야 한다면 이곳을 수정
					CheckTime.byHour = 23;
					CheckTime.byMin = 59;
				}

				if( CheckTime.byHour < NowTime.hour )
				{
					wstrTextOut += TTW(CONTENTS_COLOR_RED);
				}
				else if( CheckTime.byHour == NowTime.hour )
				{
					if( CheckTime.byMin <= NowTime.minute )
					{
						wstrTextOut += TTW(CONTENTS_COLOR_RED);
					}
				}
			}

			if( AllTime.hour < 10 )
			{
				wstrTextOut += _T("0");
			}
			wstrTextOut += (std::wstring const&)(BM::vstring)AllTime.hour;
			wstrTextOut += _T(":");
			if( AllTime.minute < 10 )
			{
				wstrTextOut += _T("0");
			}
			wstrTextOut += (std::wstring const&)(BM::vstring)AllTime.minute;

			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);

			AllTime.hour += PlayTimeSec / 3600;
			AllTime.minute += (PlayTimeSec % 3600) / 60;
			if(AllTime.minute >= 60) //분이 60을 넘어가면 그만큼 시간을 플러스 해준다
			{
				AllTime.hour += static_cast<int>(AllTime.minute / 60);
				AllTime.minute = AllTime.minute % 60;
			}
			if(AllTime.hour >= 24) //시가 24를 넘어갈 경우
			{ //넘어가는 시간은 그냥 24시까지로 잘라주자.. 만약 요일 추가가 되어야 한다면 이곳을 수정
				AllTime.hour = 23;
				AllTime.minute = 59;
			}
		}
		wstrTextOut += ENTER_STRING;
	}
	// 입장레벨 //
	if( iter->second.LevelMax )
	{
		wstrTextOut += TTW(302002);

		int const Level = pPlayer->GetAbil(AT_LEVEL);
		if( Level < iter->second.LevelMin || Level > iter->second.LevelMax )
		{
			wstrTextOut += TTW(CONTENTS_COLOR_RED);
		}

		wstrTextOut += (std::wstring const&)(BM::vstring)iter->second.LevelMin;
		wstrTextOut += _T(" ~ ");
		wstrTextOut += (std::wstring const&)(BM::vstring)iter->second.LevelMax;
		wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
		wstrTextOut += ENTER_STRING;
	}
	// 입장가능직업 //
	{
		wstrTextOut += TTW(302024);
		MakeClassLimitText(iter->second.ClassLimitHuman, iter->second.ClassLimitDragon, wstrTextOut);
	}
	// 최소입장인원 //
	if( iter->second.PartyMemeberMin )
	{
		wstrTextOut += TTW(302003);

		SExpeditionInfo ExpeditionInfo;
		SClientPartyName PartyInfo;
		if(pPlayer->GetExpeditionGuid().IsNotNull() && g_kExpedition.GetExpedition(pPlayer->GetExpeditionGuid(), ExpeditionInfo))
		{ //원정대에 속해 있다면 원정대 인원수 체크
			if(ExpeditionInfo.cCurMember < iter->second.PartyMemeberMin)
			{
				wstrTextOut += TTW(CONTENTS_COLOR_RED);
			}
		}
		else if(pPlayer->PartyGuid().IsNotNull() && g_kParty.GetPartyName(pPlayer->PartyGuid(), PartyInfo))
		{ //파티에 속해 있다면 파티 인원수 체크
			if(PartyInfo.cCurMember < iter->second.PartyMemeberMin)
			{
				wstrTextOut += TTW(CONTENTS_COLOR_RED);
			}
		}
		else if(1 < iter->second.PartyMemeberMin)
		{ //원정대, 파티에 속해있지 않을 경우 1이 아니면 실패
			wstrTextOut += TTW(CONTENTS_COLOR_RED);
		}

		wstrTextOut +=  (std::wstring const&)(BM::vstring)(int)iter->second.PartyMemeberMin;
		wstrTextOut += TTW(302014);
		wstrTextOut += _T(" ");
		wstrTextOut += TTW(302015);
		wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
		wstrTextOut += ENTER_STRING;
	}
	// 최대참가인원제한 //
	if( iter->second.UserCountMax )
	{
		wstrTextOut += TTW(302004);
		wstrTextOut += (std::wstring const&)(BM::vstring)(int)iter->second.UserCountMax;
		wstrTextOut += TTW(302014);
		wstrTextOut += ENTER_STRING;
	}
	// 필요아이템 //
	if( iter->second.NeedItemNo )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDefItem = kItemDefMgr.GetDef( iter->second.NeedItemNo );
		if( pDefItem )
		{
			wchar_t const* pItemName = NULL;
			if(GetDefString(pDefItem->NameNo(), pItemName))
			{
				wstrTextOut += TTW(302005);

				PgInventory const* pInv = pPlayer->GetInven();
				if( pInv )
				{
					int const ItemCount = pInv->GetInvTotalCount( iter->second.NeedItemNo );
					if( ItemCount )
					{
						if( ItemCount < iter->second.NeedItemCount )
						{
							wstrTextOut += TTW(CONTENTS_COLOR_RED);
						}
					}
					else
					{
						wstrTextOut += TTW(CONTENTS_COLOR_RED);
					}
				}

				wstrTextOut += std::wstring((pItemName ? pItemName : _T("")));
				if( iter->second.NeedItemCount )
				{
					wstrTextOut += _T(" ( ");
					wstrTextOut += (std::wstring const&)(BM::vstring)(int)iter->second.NeedItemCount;
					wstrTextOut += TTW(1709);
					wstrTextOut += _T(" )");
				}
				wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
				wstrTextOut += ENTER_STRING;
			}
		}
	}

	//필요 퀘스트 상태 조건 체크
	typedef enum tagENEEDQUESTSTATE
	{
		ENQS_NONE = 0,
		ENQS_ACCEPT,
		ENQS_COMPLETE,
	}ENEEDQUESTSTATE;

	// 필요퀘스트 //
	if( 0 < iter->second.NeedQuest && 0 < iter->second.NeedQuestState )
	{
		PgQuestInfo const* pQuestInfo = g_kQuestMan.GetQuest( iter->second.NeedQuest );
		if( pQuestInfo )
		{
			wstrTextOut += TTW(302006);
			wstrTextOut += ENTER_STRING;

			PgMyQuest const* pMyQuest = pPlayer->GetMyQuest();
			if( pMyQuest )
			{
				switch( iter->second.NeedQuestState )
				{
				case ENQS_ACCEPT:
					{
						if( !pMyQuest->IsIngQuest( iter->second.NeedQuest ) )
						{
							wstrTextOut += TTW(CONTENTS_COLOR_RED);
						}
					}break;
				case ENQS_COMPLETE:
					{
						if( !pMyQuest->IsEndedQuest( iter->second.NeedQuest ) )
						{
							wstrTextOut += TTW(CONTENTS_COLOR_RED);
						}
					}break;
				}
			}

			wstrTextOut += _T("  ");
			wstrTextOut += TTW( pQuestInfo->m_iTitleTextNo );
			wstrTextOut += _T(" ( ");
			switch( iter->second.NeedQuestState )
			{
			case ENQS_ACCEPT:
				{
					wstrTextOut += TTW(20000);
				}break;
			case ENQS_COMPLETE:
				{
					wstrTextOut += TTW(20001);
				}break;
			}
			wstrTextOut += _T(" )");
			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
			wstrTextOut += ENTER_STRING;
		}
	}

	
	if(iter->second.EventType != _T("Race"))
	{	
		wstrTextOut += ENTER_STRING;
		// -옵션- //
		wstrTextOut += TTW(TITLE_COLOR_YELLOW);
		wstrTextOut += TTW(302023);
		wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
		wstrTextOut += ENTER_STRING;
		// PK //
		{
			wstrTextOut += TTW(302007);
			if( iter->second.PKOption )
			{
				wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
				wstrTextOut += TTW(302010);
			}
			else
			{
				wstrTextOut += TTW(CONTENTS_COLOR_RED);
				wstrTextOut += TTW(302011);
			}
			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
			wstrTextOut += ENTER_STRING;
		}
		// 부활아이템사용 //
		{
			wstrTextOut += TTW(302008);
			if( iter->second.UnUsableReviveItem )
			{
				wstrTextOut += TTW(CONTENTS_COLOR_RED);
				wstrTextOut += TTW(302012);
			}
			else
			{
				wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
				wstrTextOut += TTW(302010);
			}
			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
			wstrTextOut += ENTER_STRING;
		}
		// 소비아이템사용 //
		{
			wstrTextOut += TTW(302009);
			if( iter->second.UnUsableConsumeItem )
			{
				wstrTextOut += TTW(CONTENTS_COLOR_RED);
				wstrTextOut += TTW(302012);
			}
			else
			{
				wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
				wstrTextOut += TTW(302010);
			}
			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
			wstrTextOut += ENTER_STRING;
		}
	}

	if(iter->second.EventType == _T("Race"))
	{//달리기 이벤트 일때.
		CONT_DEF_EVENT_RACE const * pEventRace = NULL;
		g_kTblDataMgr.GetContDef(pEventRace);
		
		if( NULL == pEventRace )
		{
			return;
		}

		CONT_DEF_EVENT_RACE::const_iterator itFind = pEventRace->find(EventNo);
		if(pEventRace->end() == itFind)
		{
			return;
		}

		int TrackAround = itFind->second.TrackAround;
		int MoveSpeed = itFind->second.MoveSpeed;			
		int MpMax = itFind->second.MpMax;
		bool UseSkill = itFind->second.UseSkill;
		
		// -옵션- //
		wstrTextOut += ENTER_STRING;
		wstrTextOut += TTW(TITLE_COLOR_YELLOW);
		wstrTextOut += TTW(302023);
		wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
		wstrTextOut += ENTER_STRING;
		// 반복 //
		{
			std::wostringstream outstream;
			outstream << TrackAround;

			wstrTextOut += TTW(401062);
			wstrTextOut += L" : ";

			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
			wstrTextOut += outstream.str();

			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
			wstrTextOut += ENTER_STRING;
		}
		// 이동속도 //
		{
			std::wostringstream outstream;
			outstream << MoveSpeed;

			wstrTextOut += TTW(302305);
			wstrTextOut += L" ";
			if(itFind->second.MoveSpeed)
			{
				wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
				wstrTextOut += outstream.str();
			}
			else
			{
				wstrTextOut += TTW(CONTENTS_COLOR_RED);
				wstrTextOut += outstream.str();

			}
			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
			wstrTextOut += ENTER_STRING;
		}
		// 최대 MP //
		{
			std::wostringstream outstream;
			outstream << MpMax;

			wstrTextOut += TTW(302306);
			wstrTextOut += L"  ";
			if(itFind->second.MpMax)
			{
				wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
				wstrTextOut += outstream.str();
			}
			else
			{
				wstrTextOut += TTW(CONTENTS_COLOR_RED);
				wstrTextOut += outstream.str();
			}
			
			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
			wstrTextOut += ENTER_STRING;
		}
		//스킬 사용
		{
			wstrTextOut += TTW(302309);
			wstrTextOut += L"  ";
			if(true == UseSkill)
			{
				wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
				wstrTextOut += TTW(2220);
			}
			else
			{
				wstrTextOut += TTW(CONTENTS_COLOR_RED);
				wstrTextOut += TTW(2221);
			}
			wstrTextOut += TTW(CONTENTS_COLOR_WHITE);
			wstrTextOut += ENTER_STRING;

		}
		//입장권 안내 
		{
			wstrTextOut += ENTER_STRING;
			wstrTextOut += TTW(302310);
		}
	}

	lwCallMutableToolTipByText(wstrTextOut, pt, 0, 0, false);
}

void lwCallChannelPartyListToolTip(lwPacket packet, lwPoint2 &pt)
{
	BM::Stream * Packet = packet();
	if( NULL == Packet )
	{
		return;
	}

	short channelNo = 0;
	SIndunPartyInfo PartyInfo;
	Packet->Pop(channelNo);
	PartyInfo.ReadFromPacket(*Packet);

	std::wstring wstrTextOut;

	if( !PartyInfo.kPartyTitle.empty() )
	{
		wstrTextOut = TTW(401350);
		wstrTextOut += ENTER_STRING;
		wstrTextOut += PartyInfo.kPartyTitle;
		wstrTextOut += ENTER_STRING;
	}

	BM::vstring Message( TTW(401354) );
	Message.Replace(L"$CLASS$", PgClientPartyUtil::GetClassNameTTW(PartyInfo.iMasterClass));
	Message.Replace(L"$CHAR_NAME$", PartyInfo.kMasterName);
	Message.Replace(L"$LEVEL$", PartyInfo.iMasterLevel);

	wstrTextOut += (std::wstring)Message;
	wstrTextOut += ENTER_STRING;

	int const iFilterItem = POI_SafeMask ^ (POE_SafeMask | POI_SafeMask);
	EPartyOptionItem eType = (EPartyOptionItem)(PartyInfo.iPartyOption & iFilterItem);			
	if( eType == POI_LikeSolo )//Item
	{
		wstrTextOut += TTW(401008);
	}
	else if( eType == POI_Order )
	{
		wstrTextOut += TTW(401009);
	}
	wstrTextOut += ENTER_STRING;

	if( !PartyInfo.kPartySubName.empty() )
	{
		wstrTextOut += TTW(401353);
		wstrTextOut += L" ";
		wstrTextOut += PartyInfo.kPartySubName;
	}

	lwCallMutableToolTipByText(wstrTextOut, pt, 0, 0, false);
	return;
}

void MakeItemColorForToolTip(PgBase_Item const &kOrgItem, std::wstring& wstrText, std::wstring& wstrRank)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kOrgItem.ItemNo());
	if(!pDef)	{return;}

	int const iCustomType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
	int const iCustomValue2 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
	bool const bGambleItem = ( (UICT_GAMBLE==iCustomType) && (0<iCustomValue2));

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 등급 색상
	std::wstring kItemColor;
	wstrText += TTW(40002);

	int iGrade = 0;
	if((true == pDef->IsType(ITEM_TYPE_EQUIP)) && (EQUIP_POS_MEDAL == pDef->EquipPos()))
	{
		iGrade = pDef->GetAbil(AT_GRADE);//등급명
	}
	else
	{
		iGrade = GetItemGrade(kOrgItem);
	}

	bool const bIsCash = CheckIsCashItem(kOrgItem);

	E_ITEM_GRADE const eItemLv = static_cast<E_ITEM_GRADE>(iGrade);
	int const iDisplayLank = pDef->GetAbil(AT_ITEM_DISPLAY_GRADE);
	
	if(	IG_SEAL != eItemLv )
	{//봉인 아니면.
		if( 0 != iDisplayLank )
		{
			kItemColor += g_kEmoFontMgr.Trans_Key_SysFontString(iDisplayLank);
		}

		if( g_kLocal.IsServiceRegion(LOCAL_MGR::NC_CHINA) )
		{
			if( pDef->CanEquip() )
			{
				SetGradeColor(eItemLv, false, kItemColor);
				SetGradeColor(eItemLv, bIsCash, wstrRank);
			}
		}
		else
		{
			SetGradeColor(eItemLv, bIsCash, kItemColor);
			SetGradeColor(eItemLv, bIsCash, wstrRank);
		}
		wstrText += kItemColor;
	}
}

std::wstring const MakeSlotAbilToolTipText(PgBase_Item const& kItem, CItemDef const* pDef, CONT_ENCHANT_ABIL const& kAbilCont, bool bRoundBracket, bool bIncludeName, bool bShowBuilUpInfo)
{
	if( kAbilCont.empty() )
	{
		return NULL_STRING;
	}

	std::wstring kSlotAbilStr;

	CONT_ENCHANT_ABIL::const_iterator kItor = kAbilCont.begin();
	while( kItor != kAbilCont.end() )
	{
		CONT_ENCHANT_ABIL::value_type const& kAbil = (*kItor);
		int const iAmpLevel = (bShowBuilUpInfo == true ? kAbil.iLevel : 0);
		MakeAbilString(kItem, pDef, kAbil.wType, kAbil.iValue, kSlotAbilStr, NULL, NULL, bRoundBracket, bIncludeName, iAmpLevel);//레벨
		++kItor;
	}

	return kSlotAbilStr;
}

std::wstring const MakeSlotAbilToolTipText(PgBase_Item const& kItem, CItemDef const* pDef, CONT_ABILS const& kContAbils, bool bRoundBracket, bool bIncludeName, bool bAddEnter)
{
	if( kContAbils.empty() )
	{
		return NULL_STRING;
	}

	std::wstring kSlotAbilStr;

	CONT_ABILS::const_iterator kItor = kContAbils.begin();
	while( kItor != kContAbils.end() )
	{
		MakeAbilString(kItem, pDef, kItor->first, kItor->second, kSlotAbilStr, NULL, NULL, bRoundBracket, bIncludeName, 0, bAddEnter);//레벨
		++kItor;
	}

	return kSlotAbilStr;
}

std::wstring const MakeSoulCraftToolTip(PgBase_Item const& kItem, CItemDefMgr const& ItemDef)
{
	CItemDef const* pDef = ItemDef.GetDef(kItem.ItemNo());
	if( !pDef )
	{
		return NULL_STRING;
	}

	SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();
	int const iEquipPos = pDef->EquipPos();
	int const iLevelLimit = pDef->GetAbil(AT_LEVELLIMIT);
	SBasicOptionAmpKey const kAmpKey(GetEquipType(pDef), iLevelLimit, kEnchantInfo.BasicAmpLv());

	CONT_ENCHANT_ABIL kEnchantAbil;
	GetAbilObject(kEnchantInfo.BasicType1(), iEquipPos, kEnchantInfo.BasicLv1(), 0, 0, kEnchantAbil, kAmpKey);
	GetAbilObject(kEnchantInfo.BasicType2(), iEquipPos, kEnchantInfo.BasicLv2(), 0, 1, kEnchantAbil, kAmpKey);
	GetAbilObject(kEnchantInfo.BasicType3(), iEquipPos, kEnchantInfo.BasicLv3(), 0, 2, kEnchantAbil, kAmpKey);
	GetAbilObject(kEnchantInfo.BasicType4(), iEquipPos, kEnchantInfo.BasicLv4(), 0, 3, kEnchantAbil, kAmpKey);

	int const SimplyToolTip = g_kGlobalOption.GetValue("ETC", "SIMPLY_TOOLTIP");
	if( SimplyToolTip && kEnchantAbil.empty() )
	{//간소화 옵션이 켜져있고 옵션이 하나도 없으면
		return NULL_STRING;
	}

	BM::vstring vSoulCraftStr(TTW(TITLE_COLOR_ORANGE));
	vSoulCraftStr += TTW(5036);
	vSoulCraftStr += TTW(CONTENTS_COLOR_WHITE);
	vSoulCraftStr += ENTER_STRING;

	if( kEnchantInfo.BasicAmpLv() )
	{
		vSoulCraftStr += TTW(1546);//옵션 증폭
		vSoulCraftStr += kEnchantInfo.BasicAmpLv();
		vSoulCraftStr += TTW(226);	//폰트 초기화
		vSoulCraftStr += ENTER_STRING;
	}

	if( kEnchantAbil.empty() )
	{//소울 옵션이 없음
		if( PgItemRarityUpgradeMgr::RCE_OK != PgItemRarityUpgradeMgr::CheckRarityBundle(kItem) )
		{
			return NULL_STRING;
		}
		if(0==kEnchantInfo.BasicAmpLv())
		{
			vSoulCraftStr += TTW(5039);
			vSoulCraftStr += ENTER_STRING;
		}
	}
	else
	{//소울 옵션이 있음
		vSoulCraftStr += MakeSlotAbilToolTipText(kItem, pDef, kEnchantAbil, false, true, true);

		if( kEnchantInfo.BasicAmpLv() )
		{//PVP시에 옵션증폭이 적용 안된다는 설명
			vSoulCraftStr += TTW(750010); // 폰트 컬러 노란색
			vSoulCraftStr += TTW(749992);
			vSoulCraftStr += ENTER_STRING;

			// 툴팁이 해상도를 벗어났을 경우 텍스트를 나눌 위치를 찾기 위해 심어놓는 글자
			// 출력될때는 제외하고 출력한다.
			vSoulCraftStr += CUT_TOOLTIP2;
		}
	}

	return vSoulCraftStr.operator const std::wstring &();
}

std::wstring const MakePlusEnchantToolTip(PgBase_Item const& kItem, CItemDefMgr const& ItemDef)
{
	CItemDef const* pDef = ItemDef.GetDef(kItem.ItemNo());
	if( !pDef )
	{
		return NULL_STRING;
	}

	if( false == CheckEnchantBundle(kItem) )
	{
		return NULL_STRING;
	}

	SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();

	int const SimplyToolTip = g_kGlobalOption.GetValue("ETC", "SIMPLY_TOOLTIP");
	if( SimplyToolTip && !kEnchantInfo.IsPlus() )
	{//간소화 옵션이 켜져있고 인첸트가 되어있지 않으면
		return NULL_STRING;
	}

	std::wstring vPlusStr;
	vPlusStr += TTW(TITLE_COLOR_ORANGE);
	vPlusStr += TTW(5037);//인챈트 효과
	vPlusStr += TTW(CONTENTS_COLOR_WHITE);
	vPlusStr += ENTER_STRING;

	if( !kEnchantInfo.IsPlus() )
	{//인챈트가 되어있지 않으면 설명 문구를 보여준다.
		vPlusStr += TTW(5040);//인챈트를 하면 물리 공격력/마법 공격력이 증가한다.(위치 : 마이홈)
		vPlusStr += ENTER_STRING;
		return vPlusStr;
	}

	PgItemUpgradeInfo kCurrentInfo, kNextInfo;
	kCurrentInfo.SetPlusUpgrade(kItem, pDef, kEnchantInfo, kEnchantInfo.PlusLv());
	if( kEnchantInfo.PlusLv() )
	{
		kNextInfo.SetPlusUpgrade(kItem, pDef, kEnchantInfo, kEnchantInfo.PlusLv()+1);
	}

	if( !kCurrentInfo.IsEmpty() )
	{
		int iTextNo = 5000;
		if( EQUIP_LIMIT_WEAPON == (EQUIP_LIMIT_WEAPON & pDef->GetAbil(AT_EQUIP_LIMIT)) || true==pDef->IsPetItem() )
		{
			iTextNo = 5003;//물리/마법 공격력
		}
		else
		{
			iTextNo = 5041;//물리/마법 방어력
		}

		vPlusStr += TTW(iTextNo);
		vPlusStr += ADD_MARK_STRING;
		kCurrentInfo.MakeToolTipText(vPlusStr, pDef, true, false);
		if( !kNextInfo.IsEmpty() )
		{
			vPlusStr += TTW(5051);//(Next: 
			kNextInfo.MakeToolTipText(vPlusStr, pDef, true, false);
			vPlusStr += L")";
			vPlusStr += TTW(40011);	//폰트 초기화
		}
		vPlusStr += ENTER_STRING;

		vPlusStr += TTW(++iTextNo);
		vPlusStr += ADD_MARK_STRING;
		kCurrentInfo.MakeToolTipText(vPlusStr, pDef, false, false);
		if( !kNextInfo.IsEmpty() )
		{
			vPlusStr += TTW(5051);//(Next: 
			kNextInfo.MakeToolTipText(vPlusStr, pDef, false, false);
			vPlusStr += L")";
			vPlusStr += TTW(40011);	//폰트 초기화
		}
		vPlusStr += ENTER_STRING;
	}
	
	{// 영력 등급에 따른 인첸트 횟수 제한 설명
		E_ITEM_GRADE const eSrcItemGrade = ::GetItemGrade(kItem);
		if( IG_LEGEND > eSrcItemGrade )
		{// 전설 등급은 제외한다.
			BM::vstring vStr(TTW(40909));
			int MaxLv = 0;
			std::wstring GradeColor;
			std::wstring const Grade(TTW(3151 + eSrcItemGrade));
			SetGradeColor(eSrcItemGrade, false, GradeColor);
			PgItemRarityUpgradeFormula::GetMaxGradeLevelRestrict(eSrcItemGrade, false, MaxLv, kItem);
			
			vStr.Replace(L"#GRADE_COLOR#", GradeColor);
			vStr.Replace(L"#GRADE#", Grade);
			vStr.Replace(L"#MAX#", MaxLv);
			vPlusStr += (std::wstring)vStr;
			vPlusStr += ENTER_STRING;
		}
	}

	return vPlusStr;
}

std::wstring const MakeSocketToolTip(PgBase_Item const& kItem, CItemDefMgr const& ItemDef)
{
	CItemDef const* pDef = ItemDef.GetDef(kItem.ItemNo());
	if( !pDef )
	{
		return NULL_STRING;
	}

	/*if( PgItemSocketSystemMgr::SCE_OK != PgItemSocketSystemMgr::CheckSocketBundle( kItem ) )
	{
		return NULL_STRING;
	}*/

	if( (ICMET_Cant_GenSocket & pDef->GetAbil(AT_ATTRIBUTE)) == ICMET_Cant_GenSocket )
	{
		return NULL_STRING;
	}

	SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();

	int iSocketState = kEnchantInfo.GenSocketState();
	if( GSS_GEN_NONE == iSocketState )
	{//소켓이 하나도 없고
		int const SimplyToolTip = g_kGlobalOption.GetValue("ETC", "SIMPLY_TOOLTIP");
		if( SimplyToolTip )
		{//간소화 옵션이 켜져있으면
			return NULL_STRING;
		}
	}
	
	BM::vstring SocketStr(TTW(TITLE_COLOR_ORANGE));
	SocketStr += TTW(5035);
	SocketStr += TTW(CONTENTS_COLOR_WHITE);//폰트 초기화
	SocketStr += ENTER_STRING;

	for(int i=PgSocketFormula::SII_FIRST; i<=PgSocketFormula::SII_THIRD; ++i)
	{
		int iSocketState = 0;
		int iSocketCard = 0;

		switch( i )
		{
		case PgSocketFormula::SII_FIRST:
			{
				iSocketState = kEnchantInfo.GenSocketState();
				iSocketCard = kEnchantInfo.MonsterCard();
			}break;
		case PgSocketFormula::SII_SECOND:
			{
				iSocketState = kEnchantInfo.GenSocketState2();
				iSocketCard = kEnchantInfo.MonsterCard2();
			}break;
		case PgSocketFormula::SII_THIRD:
			{
				iSocketState = kEnchantInfo.GenSocketState3();
				iSocketCard = kEnchantInfo.MonsterCard3();
			}break;
		default:
			{
			}break;
		}

		switch( iSocketState )
		{
		case GSS_GEN_NONE://소켓이 없음
			{
				if( i == PgSocketFormula::SII_FIRST )
				{
					SocketStr += TTW(5038);
					SocketStr += ENTER_STRING;
				}
			}break;
		case GSS_GEN_SUCCESS://소켓이 있음
			{
				if( iSocketCard )
				{
					SocketStr += L"#201";
					std::wstring wstrTimeLimit;
					if(true == MakeMonsterCardTimeLimit(kItem, wstrTimeLimit))
					{
						SocketStr += wstrTimeLimit;
					}

					SocketStr += ENTER_STRING;

					// 옵션 추가
					//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 효과 설명
					{
						std::wstring wstrTemp = L"";
						PgBase_Item kCardItem;
						int const iCardItemNo = GetMonsterCardItemNo( i, iSocketCard );
						kCardItem.ItemNo( iCardItemNo );
						int iAbilNum = ItemDef.GetAbil(iCardItemNo, AT_EFFECTNUM1);
						MakeMonsterCardOptionString(kCardItem, pDef, iAbilNum, wstrTemp,true,&kItem);
						SocketStr += wstrTemp;
						SocketStr += TTW(790178);
						SocketStr += ENTER_STRING;
					}
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
				}
				else
				{
					SocketStr += _T("#200\n");
					SocketStr += TTW(790420);
					SocketStr += ENTER_STRING;
				}					
			}break;
		case GSS_GEN_FAIL://소켓이 파괴됨
			{
				SocketStr += _T("#202\n");
				SocketStr += TTW(790421);
				SocketStr += ENTER_STRING;
			}break;
		default:
			{
				return NULL_STRING;
			}break;
		}
	}
	return g_kChatMgrClient.ConvertUserCommand((std::wstring const&)(SocketStr),_T("EmoticonFont13x13"));
}

bool MakeToolTip_ItemExtend(PgBase_Item const &kOrgItem, std::wstring &wstrTextOut, TBL_SHOP_IN_GAME const &kShopItemInfo, std::wstring& wstrRank, bool bHideRarity, bool bOrtherActor, SToolTipFlag const &kFlag)
{
/*	CUnit* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	int iResult = 0;
	PgBase_Item kItem = kOrgItem;
	SEnchantInfo const &kEnchantInfo = kItem.EnchantInfo();
	std::wstring wstrText;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if(!pDef)
	{
		return false;
	}

	wstrText += TTW(4208);//Font&Color
	wstrText += ENTER_STRING;

	if( EQUIP_LIMIT_WEAPON == (EQUIP_LIMIT_WEAPON & pDef->GetAbil(AT_EQUIP_LIMIT)) || true==pDef->IsPetItem() )
	{
		wstrText += TTW(5046);
		wstrText += ENTER_STRING;
		wstrText += TTW(TITLE_COLOR_ORANGE);
		wstrText += ENTER_STRING;
		wstrText += AREA_MARK_STRING;
		wstrText += TTW(5003);// 물리 공격력
	}
	else
	{
		wstrText += TTW(5047);
		wstrText += ENTER_STRING;
		wstrText += TTW(TITLE_COLOR_ORANGE);
		wstrText += ENTER_STRING;
		wstrText += AREA_MARK_STRING;
		wstrText += TTW(5041);// 물리 방어력
	}
	wstrText += AREA_MARK_STRING;
	wstrText += TTW(40011);	//폰트 초기화
	wstrText += ENTER_STRING;

	PgItemUpgradeInfo kItemUpgradeInfo;
	iResult += kItemUpgradeInfo.MakeToolTipText_All(pkPlayer, wstrText, kItem, pDef, true);

	wstrText += TTW(TITLE_COLOR_ORANGE);
	wstrText += ENTER_STRING;

	wstrText += AREA_MARK_STRING;
	if( EQUIP_LIMIT_WEAPON == (EQUIP_LIMIT_WEAPON & pDef->GetAbil(AT_EQUIP_LIMIT)) || true==pDef->IsPetItem() )
	{
		wstrText += TTW(5004);
	}
	else
	{
		wstrText += TTW(5042);
	}
	wstrText += AREA_MARK_STRING;
	wstrText += TTW(40011);	//폰트 초기화
	wstrText += ENTER_STRING;

	iResult += kItemUpgradeInfo.MakeToolTipText_All(pkPlayer, wstrText, kItem, pDef, false);

	if( !iResult )
	{
		return false;
	}

	wstrText += ENTER_STRING;
	wstrText += TTW(5048);//상세정보 창 닫기/열기 [Alt 키] 
	wstrText += TTW(40011);	//폰트 초기화
	wstrTextOut = wstrText;
	*/
	return false; //아이템 툴팁 상세정보 삭제
}

bool MakeToolTipText_Status( WORD wAbilType, std::wstring &wstrTextOut, CUnit* pkUnit,int* pkiOutResult )
{
	int iResult = 0;
	//장착 중인 아이템을 가져 온다.
	CONT_HAVE_ITEM_DATA kContHaveItems;
	CUnit* pkPlayer = pkUnit;
	if( !pkPlayer )
	{
		pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}
	}

	EUnitType kType = pkPlayer->UnitType();
	GET_DEF(CItemDefMgr, kItemDefMgr);

	SItemPos kItemPosMedal;
	PgItemUpgradeInfo kBaseInfo(E_UF_BASE), kPlusInfo(E_UF_PLUS), kCraftInfo(E_UF_RAIRITY),
		kRareInfo(E_UF_OPT), kPackageInfo(E_UF_SET), kSocketInfo(E_UF_SOCKET), kMedalInfo(E_UF_MEDAL);
	pkPlayer->GetInven()->GetItems(IT_FIT, kContHaveItems);//템들을 가져와 봅세.
	if( false == kContHaveItems.empty() )
	{
		CONT_INT kContSetNo;//세트 번호 중복 검사용 컨테이너
		CONT_HAVE_ITEM_DATA::const_iterator itor = kContHaveItems.begin();

		while(kContHaveItems.end() != itor)
		{
			PgBase_Item const& kItem = itor->second;
			SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();
			CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());

			SItemPos kItemPos;
			pkPlayer->GetInven()->GetFirstItem(kItem.ItemNo(), kItemPos);
			if( IT_FIT == kItemPos.x && EQUIP_POS_MEDAL == kItemPos.y ) 
			{//업적 예외처리
				kItemPosMedal = kItemPos;
				++itor;
				continue;
			}

			kBaseInfo.SetBaseInfo(pkPlayer, kItem, pkItemDef);
			kPlusInfo.SetPlusUpgrade(kItem, pkItemDef, kEnchantInfo, kEnchantInfo.PlusLv());
			kCraftInfo.SetRarityUpgrade(pkPlayer, kItem, pkItemDef, kEnchantInfo);
			kSocketInfo.SetSocketOption(pkPlayer, kItem, pkItemDef, kEnchantInfo);
			kRareInfo.SetRareOption(pkPlayer, kItem, pkItemDef, kEnchantInfo);
			kPackageInfo.SetPackageOption(pkPlayer, kItem, pkItemDef, &kContSetNo);
			++itor;
		}
	}

	if( SItemPos::NullData() != kItemPosMedal )
	{//업적 있음
		PgBase_Item kItem;
		pkPlayer->GetInven()->GetItem(kItemPosMedal, kItem);
		SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();
		CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());

		kMedalInfo.SetPackageOption(pkPlayer, kItem, pkItemDef);
	}

	kContHaveItems.clear();
	PgItemUpgradeInfo kCashInfo(E_UF_CASH);
	pkPlayer->GetInven()->GetItems(IT_FIT_CASH, kContHaveItems);//캐시템들을 가져와 봅세.
	if( false == kContHaveItems.empty() )
	{
		CONT_INT kContSetNo;//세트 번호 중복 검사용 컨테이너
		CONT_HAVE_ITEM_DATA::const_iterator itor = kContHaveItems.begin();

		while(kContHaveItems.end() != itor)
		{
			PgBase_Item const& kItem = itor->second;
			SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();
			CItemDef const *pkItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			
			// 기간 만료 아이템 검사
			// 만료되면 패스
			if( !kItem.IsUseTimeOut() )
			{
				kCashInfo.SetBaseInfo(pkPlayer, kItem, pkItemDef);
				kCashInfo.SetPlusUpgrade(kItem, pkItemDef, kEnchantInfo, kEnchantInfo.PlusLv());
				kCashInfo.SetRarityUpgrade(pkPlayer, kItem, pkItemDef, kEnchantInfo);
				kCashInfo.SetSocketOption(pkPlayer, kItem, pkItemDef, kEnchantInfo);
				kCashInfo.SetRareOption(pkPlayer, kItem, pkItemDef, kEnchantInfo);
				kCashInfo.SetPackageOption(pkPlayer, kItem, pkItemDef, &kContSetNo);
			}
			++itor;
		}
	}

	switch( wAbilType )
	{//전환속성 어빌용
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
			kBaseInfo.SetUnit(pkUnit);
		}break;
	default: {}break;
	}

	std::wstring wstrText;
	iResult += kBaseInfo.MakeToolTipText_Status(wstrText, wAbilType);

	switch( wAbilType )
	{//인챈트 정보는 아래 어빌에 대해서만 표시해줌.
	case AT_C_PHY_DEFENCE:				
	case AT_C_MAGIC_DEFENCE:			
	case AT_C_PHY_ATTACK_MAX:		
	case AT_C_MAGIC_ATTACK_MAX:		
		{
			iResult += kPlusInfo.MakeToolTipText_Status(wstrText, wAbilType);	
		}break;
	default: {}break;
	}
	
	if(kType!=UT_PET)
	{
		iResult += kCraftInfo.MakeToolTipText_Status(wstrText, wAbilType);

		iResult += kSocketInfo.MakeToolTipText_Status(wstrText, wAbilType);
	}

	iResult += kRareInfo.MakeToolTipText_Status(wstrText, wAbilType);

	if(kType!=UT_PET)
	{
		iResult += kPackageInfo.MakeToolTipText_Status(wstrText, wAbilType);

		iResult += kCashInfo.MakeToolTipText_Status(wstrText, wAbilType);

		iResult += kMedalInfo.MakeToolTipText_Status(wstrText, wAbilType);
	}

	//캐릭터 총합
	BM::vstring vstrTotalInfo;
	kBaseInfo = kBaseInfo + kPlusInfo + kCraftInfo + kRareInfo + kPackageInfo + kSocketInfo + kCashInfo + kMedalInfo;
	vstrTotalInfo = L"\n";
	vstrTotalInfo += ::TTW(1301);
	vstrTotalInfo +=  ::TTW(3004);
	vstrTotalInfo += L" ";
	vstrTotalInfo += ::TTW(5050);
	vstrTotalInfo += L": ";
	PgItemUpgradeInfo::MakeValueString(vstrTotalInfo, wAbilType, kBaseInfo.GetAbil(wAbilType), pkPlayer->GetAbil(AT_LEVEL));
	vstrTotalInfo += L"\n";
	vstrTotalInfo += ::TTW(226);

	wstrTextOut += vstrTotalInfo.operator const std::wstring &();
	wstrTextOut += wstrText;
	if(pkiOutResult)
	{
		*pkiOutResult = kBaseInfo.GetAbil(wAbilType);
	}
	return true;
}

struct EleAbil_
{
	int kValue;
	std::wstring kString;
	bool bIsRate;//% 수치인가?
	explicit EleAbil_() : kValue(0), bIsRate(false)
	{			
	}

	explicit EleAbil_(int const value, std::wstring const& string)
	{
		Set(value, string);
	}

	bool operator < (EleAbil_ const& rhs) const
	{
		return kValue < rhs.kValue;
	}

	void Set(int const value, std::wstring const& string)
	{
		kValue = value;
		kString = string;
		if(string.empty()) { return; }
		std::wstring::value_type k = string.at(string.size()-1);
		if(L'%'==k)
		{
			bIsRate = true;
		}	
	}
};

template<typename T>
bool MakeEleAbilNameString(T const* pDef, PgBase_Item const& kItem, int const iAbil, 
						   int const iValue, int const iBaseTT, std::wstring &kOut)
{
	std::wstring kName;
	if(MakeAbilNameString(iAbil, kName))
	{		
		kOut+=TTW(iBaseTT);
		kOut+=kName;
		kOut+=WSTR_ATTR_EXPLAIN;
		MakeAbilValueString(kItem, pDef, iAbil, iValue, kOut, false);

		return true;
	}

	return false;
}


typedef std::map<int, EleAbil_> EleAbilList;

void GetEleInfoFromItemDef(PgBase_Item const& kItem, CAbilObject const* pDef, EleAbilList &rkList, 
						   bool needstring)
{
	if(NULL==pDef)	{return;}

	std::wstring kOut;
	int iTTBase = 0;
	int iAdd = 0;
	SAbilIterator kItor;
	pDef->FirstAbil(&kItor);
	while(pDef->NextAbil(&kItor))
	{
		switch(kItor.wType)
		{
		case AT_ATTACK_ADD_FIRE:
		case AT_ATTACK_ADD_ICE:
		case AT_ATTACK_ADD_NATURE:
		case AT_ATTACK_ADD_CURSE:
		case AT_ATTACK_ADD_DESTROY:
		case AT_R_ATTACK_ADD_FIRE:
		case AT_R_ATTACK_ADD_ICE:
		case AT_R_ATTACK_ADD_NATURE:
		case AT_R_ATTACK_ADD_CURSE:
		case AT_R_ATTACK_ADD_DESTROY:
		case AT_R_ATTACK_ADD_FIRE_PHY:
		case AT_R_ATTACK_ADD_ICE_PHY:
		case AT_R_ATTACK_ADD_NATURE_PHY:
		case AT_R_ATTACK_ADD_CURSE_PHY:
		case AT_R_ATTACK_ADD_DESTROY_PHY:
		case AT_R_ATTACK_ADD_FIRE_MAGIC:
		case AT_R_ATTACK_ADD_ICE_MAGIC:
		case AT_R_ATTACK_ADD_NATURE_MAGIC:
		case AT_R_ATTACK_ADD_CURSE_MAGIC:
		case AT_R_ATTACK_ADD_DESTROY_MAGIC:
		case AT_RESIST_ADD_FIRE:
		case AT_RESIST_ADD_ICE:
		case AT_RESIST_ADD_NATURE:
		case AT_RESIST_ADD_CURSE:
		case AT_RESIST_ADD_DESTROY:
		case AT_R_RESIST_ADD_FIRE:
		case AT_R_RESIST_ADD_ICE:
		case AT_R_RESIST_ADD_NATURE:
		case AT_R_RESIST_ADD_CURSE:
		case AT_R_RESIST_ADD_DESTROY:
		case AT_R_RESIST_ADD_FIRE_PHY:
		case AT_R_RESIST_ADD_ICE_PHY:
		case AT_R_RESIST_ADD_NATURE_PHY:
		case AT_R_RESIST_ADD_CURSE_PHY:
		case AT_R_RESIST_ADD_DESTROY_PHY:
		case AT_R_RESIST_ADD_FIRE_MAGIC:
		case AT_R_RESIST_ADD_ICE_MAGIC:
		case AT_R_RESIST_ADD_NATURE_MAGIC:
		case AT_R_RESIST_ADD_CURSE_MAGIC:
		case AT_R_RESIST_ADD_DESTROY_MAGIC:
			{
				iTTBase = 792021;
				iAdd = static_cast<int>(kItor.wType)%10 - 1;
			}break;
		case AT_ATTACK_ADD_4ELEMENT:
		case AT_ATTACK_ADD_5ELEMENT:
		case AT_R_ATTACK_ADD_4ELEMENT:
		case AT_R_ATTACK_ADD_5ELEMENT:
		case AT_R_ATTACK_ADD_4ELEMENT_PHY:
		case AT_R_ATTACK_ADD_5ELEMENT_PHY:
		case AT_R_ATTACK_ADD_4ELEMENT_MAGIC:
		case AT_R_ATTACK_ADD_5ELEMENT_MAGIC:
		case AT_RESIST_ADD_4ELEMENT:
		case AT_RESIST_ADD_5ELEMENT:
		case AT_R_RESIST_ADD_4ELEMENT:
		case AT_R_RESIST_ADD_5ELEMENT:
		case AT_R_RESIST_ADD_4ELEMENT_PHY:
		case AT_R_RESIST_ADD_5ELEMENT_PHY:
		case AT_R_RESIST_ADD_4ELEMENT_MAGIC:
		case AT_R_RESIST_ADD_5ELEMENT_MAGIC:
			{
				iTTBase = 792031;
				iAdd = static_cast<int>(kItor.wType)%10;
			}break;
		default:
			{
				iTTBase = 0;
				iAdd = 0;
			}
		}

		if(0!=iTTBase)
		{
			if(true==needstring)
			{
				MakeEleAbilNameString(pDef, kItem, kItor.wType, kItor.iValue, iTTBase+iAdd, kOut);
			}
			rkList.insert(std::make_pair(kItor.wType, EleAbil_(kItor.iValue, kOut)));
			kOut.clear();
		}
	}
}

std::list<int> g_EleAbilOrder;
void MakeEleAbilOrder(std::list<int> &rkList)
{
	if(false==rkList.empty())
	{
		return;
	}

	//공격
	for(int i = 0; i<5; ++i)
	{
		rkList.push_back(AT_ATTACK_ADD_FIRE + i);			//30011
		rkList.push_back(AT_R_ATTACK_ADD_FIRE + i);			//30021
		rkList.push_back(AT_R_ATTACK_ADD_FIRE_PHY + i);		//30031
		rkList.push_back(AT_R_ATTACK_ADD_FIRE_MAGIC + i);	//30041
	}

	for(int i = 0; i<4; ++i)
	{
		rkList.push_back(AT_ATTACK_ADD_4ELEMENT + i * 10);
		rkList.push_back(AT_ATTACK_ADD_5ELEMENT + i * 10);
	}

	//저향력
	for(int i = 0; i<5; ++i)
	{
		rkList.push_back(AT_RESIST_ADD_FIRE + i);			//30051
		rkList.push_back(AT_R_RESIST_ADD_FIRE + i);			//30061
		rkList.push_back(AT_R_RESIST_ADD_FIRE_PHY + i);		//30071
		rkList.push_back(AT_R_RESIST_ADD_FIRE_MAGIC + i);	//30081
	}

	for(int i = 0; i<4; ++i)
	{
		rkList.push_back(AT_RESIST_ADD_4ELEMENT + i * 10);
		rkList.push_back(AT_RESIST_ADD_5ELEMENT + i * 10);
	}
}

void MakeElementToolTipCompText(EleAbil_ const& ele, EleAbil_ const& eleComp, std::wstring &Out)
{
	int iValue = ele.kValue;
	int iCompVal = eleComp.kValue;
	int iDiff = iValue - iCompVal;
	wchar_t szCompRst[30] = {0,};
	if(0 < iDiff)
	{
		Out += TTW(89999);
		if(true==ele.bIsRate)
		{
			swprintf(szCompRst, 29, L" (%s %.1f)", TTW(5000).c_str(), (float)iDiff*0.01f);
		}
		else
		{
			swprintf(szCompRst, 29, L" (%s %d)", TTW(5000).c_str(), iDiff);
		}
		Out += szCompRst;
	}
	else if(0 >iDiff)
	{
		Out += TTW(89998);
		if(true==ele.bIsRate)
		{
			swprintf(szCompRst, 29, L" (%s %.1f)", TTW(5001).c_str(), fabs((float)iDiff)*0.01f);
		}
		else
		{
			swprintf(szCompRst, 29, L" (%s %d)", TTW(5001).c_str(), abs(iDiff));//장착중인게 더 좋다
		}
		Out += szCompRst;
	}
	else
	{
		swprintf(szCompRst, 29, L" (%s)", TTW(5002).c_str());
		Out += szCompRst;
	}
}

std::wstring const MakeElementToolTip(PgBase_Item const& kItem, CItemDef const* pDef)
{
	std::wstring kString = NULL_STRING;
	if( !pDef )
	{
		return NULL_STRING;
	}

	std::wstring kTempStr;
	std::wstring kName;
	int iAbil = AT_ATTACK_ADD_FIRE;
	int iValue = 0;

	EleAbilList kAbilList, kEquipAbilList;

	bool bShowDiffInfo = false;
	
	if(g_bIsEqComp)
	{
		CUnit* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return NULL_STRING;
		}
		PgInventory* pkInv = pkPlayer->GetInven();
		if(!pkInv)
		{
			return NULL_STRING;
		}

		if (IS_CLASS_LIMIT(pDef->GetAbil64(AT_CLASSLIMIT), pkPlayer->GetAbil(AT_CLASS)))
		{
			PgBase_Item kEqItem;
			if(S_OK == pkInv->GetItem(SItemPos(KUIG_FIT, pDef->EquipPos()), kEqItem))
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const * pEqDef = kItemDefMgr.GetDef(kEqItem.ItemNo());
				if( pEqDef && kEqItem != kItem )
				{
					bShowDiffInfo = true;
					GetEleInfoFromItemDef(kEqItem, pEqDef, kEquipAbilList, false);
				}
			}
		}
	}

	GetEleInfoFromItemDef(kItem, pDef, kAbilList, true);

	MakeEleAbilOrder(g_EleAbilOrder);

	std::list<int>::const_iterator order_it = g_EleAbilOrder.begin();

	EleAbilList::mapped_type eleInfo, eq_eleInfo5;

	while(g_EleAbilOrder.end()!=order_it)
	{
		int abil = (*order_it);
		EleAbilList::const_iterator kAbilList_it = kAbilList.find(abil);
		if(kAbilList.end()!=kAbilList_it)
		{
			EleAbilList::mapped_type const& Data = (*kAbilList_it).second;
			kString+=Data.kString;
			if(true==bShowDiffInfo)
			{
				EleAbilList::const_iterator eq_it = kEquipAbilList.find((*kAbilList_it).first);
				if(kEquipAbilList.end()!=eq_it)
				{
					EleAbilList::mapped_type const& eqData = (*eq_it).second;
					MakeElementToolTipCompText(Data, eqData, kString);
				}
			}
			kString += ENTER_STRING;
		}
		++order_it;
	}

	kString+=TTW(1554);

	return kString;
}

bool MakeToolTipText(PgBase_Item const &kOrgItem, std::wstring &wstrTextOut, TBL_SHOP_IN_GAME const &kShopItemInfo, std::wstring& wstrRank, bool bHideRarity, bool bOrtherActor, SToolTipFlag const &kFlag)
{
	static const std::wstring UNKNOWN_NAME(L"??");

	PgBase_Item kItem = kOrgItem;
	SEnchantInfo const &kEnchantInfo = kItem.EnchantInfo();
	std::wstring wstrText;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if(!pDef)
	{
		return false;
	}
	int const iCostumeMixGrade = pDef->GetAbil(AT_COSTUME_GRADE);
	int const iCustomType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
	int const iCustomValue2 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
	bool const bGambleItem = ( (UICT_GAMBLE==iCustomType) && (0<iCustomValue2));

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 등급 색상
	
	wstrText += TTW(40002);

	bool const bIsCash = CheckIsCashItem(kItem);

	int iGrade = 0;
	if((true == pDef->IsType(ITEM_TYPE_EQUIP)) && (EQUIP_POS_MEDAL == pDef->EquipPos())  && false == bIsCash )
	{//업적은 등급 어빌로 판단하면 되고
		iGrade = pDef->GetAbil(AT_GRADE);//등급명
	}
	else
	{//기타 아이템들은 영력에 따라 등급이 정해짐
		iGrade = GetItemGrade(kItem);;
	}

	E_ITEM_GRADE const eItemLv = static_cast<E_ITEM_GRADE>(iGrade);
	
	MakeItemColorForToolTip(kOrgItem, wstrText, wstrRank);

	if(	IG_SEAL == eItemLv )
	{
		SEnchantInfo kEnchantInfo = kItem.EnchantInfo();
		kEnchantInfo.Rarity(0);
		kItem.EnchantInfo(kEnchantInfo);
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 이름
	
	if ( true == bGambleItem )
	{
		wstrText += UNKNOWN_NAME;
	}
	else
	{
		std::wstring kItemName;
		MakeItemName(kItem.ItemNo(), kEnchantInfo, kItemName);
		wstrText += kItemName;
	}

	wstrText += TTW(40011);	//폰트 초기화
	wstrText += _T("\n ");//Pg2DString에서 높이를 계산하는 공식이 마지막줄의 폰트 크기와 줄바꿈 횟수를 곱해서 문제가 발생함 그걸 위한 임시 방편.
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 등급 - 특수 : 캐쉬
	if( true == bIsCash && !pDef->CanEquip() )
	{
		wstrRank += _T("[");
		wstrRank += TTW(50402);
		wstrRank += _T("]");
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 마켓 캐시<->골드 환전용 임시 아이템 처리
	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_USA:
	case LOCAL_MGR::NC_FRANCE:
	case LOCAL_MGR::NC_GERMANY:
		{
		}break;
	default :
		{
			if( kItem.ItemNo() == MARKET_ARTICLE_CASHNO 
			||  kItem.ItemNo() == MARKET_ARTICLE_MONEYNO )
			{
				wstrTextOut += wstrText;
				return true;
			}
		}break;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 기본 정보
	if(pDef->CanEquip())// 장착 가능 아이템
	{
		wstrText += L"\n";
		//if( false == bIsCash || g_kLocal.IsServiceRegion(LOCAL_MGR::NC_CHINA) ) //캐쉬아이템 일 경우 무시
		{
			bool const bShowRarity = (IG_SEAL == eItemLv) || !bHideRarity; // 봉인이거나, 숨김 옵션이 없어야 보여준다
			if( bShowRarity )
			{
				//영력
				SetGradeColor(eItemLv, false, wstrText);
				//wstrText += TTW(1306);
				wstrText += TTW(261);
				wstrText += WSTR_ATTR_EXPLAIN;
				wstrText += TTW(40003);

				//아이템 등급
				//std::wstring kRank(L"[");
				std::wstring kRank(L"  /  ");
				if ( true == bGambleItem )
				{
					wstrText += UNKNOWN_NAME;
					kRank += UNKNOWN_NAME;
				}
				else
				{
					kRank += TTW(262);
					kRank += WSTR_ATTR_EXPLAIN;

					switch(eItemLv)
					{
					case IG_SEAL:
						{
							kRank += TTW(50401);//봉인
							wstrText += TTW(50415);//알 수 없음
						}break;
					case IG_CURSE:
						{
							kRank += TTW(50400);//저주
							BM::vstring strRarity((int)kEnchantInfo.Rarity());
							wstrText += (wchar_t const*)strRarity;//레어도 수치.
						}break;
					default:
						{
							kRank += TTW(50300 + eItemLv);//등급명
							BM::vstring strRarity((int)kEnchantInfo.Rarity());
							wstrText += (wchar_t const*)strRarity;//레어도 수치.
						}break;
					}
				}

				//kRank += L"]";

				wstrText += TTW(40011);
				wstrText += kRank;

				//TTW(40010);//기능
				wstrText += L"\n";
			}
			if(0 < iCostumeMixGrade)
			{// 코스튬 등급 표시
				wstrRank += TTW(790780+iCostumeMixGrade);
				wstrRank += L"[";
				wstrRank += GetCostumeMixGradeString(iCostumeMixGrade);
				wstrRank += L"]";
				wstrRank += L"{C=";
				wstrRank += TTW(4097);
				wstrRank += L"/}";
			}

			if( EQUIP_POS_MEDAL != pDef->EquipPos() || bIsCash )
			{//업적아이템일 경우 영력 수치 표시 안하도록
				wstrRank += L"[";
				GetEqiupPosString(pDef->GetAbil(AT_EQUIP_LIMIT), pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE)==UICT_PET_EQUIP, wstrRank);
				wstrRank += L"]";
			}
			else
			{
				CONT_DEF_ITEM2ACHIEVEMENT const *pkContDefItem2Ach = NULL;
				g_kTblDataMgr.GetContDef(pkContDefItem2Ach);

				CONT_DEF_ACHIEVEMENTS_SAVEIDX const * pContDefAch = NULL;
				g_kTblDataMgr.GetContDef(pContDefAch);	//업적 데프
				int iCategory = 0;
				if(pkContDefItem2Ach && pContDefAch)
				{
					CONT_DEF_ITEM2ACHIEVEMENT::const_iterator iter = pkContDefItem2Ach->find(kItem.ItemNo());
					if(iter != pkContDefItem2Ach->end())
					{
						int const iSaveIdx = (*iter).second.iSaveIdx;

						CONT_DEF_ACHIEVEMENTS_SAVEIDX::const_iterator c_iter = pContDefAch->find(iSaveIdx);
						if(c_iter != pContDefAch->end())
						{
							iCategory = c_iter->second.iCategory;
						}
					}
				}

				if(0 != iCategory)
				{
					wstrRank += L"[";
					wstrRank += (6 == iCategory) ? TTW(3035) : TTW(3019+iCategory);
					wstrRank += L"]";
				}
				else
				{
					wstrRank += L"[";
					GetEqiupPosString(pDef->GetAbil(AT_EQUIP_LIMIT), pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE)==UICT_PET_EQUIP, wstrRank);
					wstrRank += L"]";
				}
			}
		}

		//장비 플래그
		int const iWeaponFlag = EQUIP_LIMIT_WEAPON;
		int const iAmorFlag	  =	EQUIP_LIMIT_SHOULDER| EQUIP_LIMIT_SHEILD| EQUIP_LIMIT_CLOAK|
			EQUIP_LIMIT_NECKLACE| EQUIP_LIMIT_EARRING|EQUIP_LIMIT_RING|EQUIP_LIMIT_MEDAL|
			EQUIP_LIMIT_BELT| EQUIP_LIMIT_HELMET| EQUIP_LIMIT_SHIRTS|
			EQUIP_LIMIT_PANTS| EQUIP_LIMIT_BOOTS| EQUIP_LIMIT_GLOVE|
			EQUIP_LIMIT_RING| EQUIP_LIMIT_NECKLACE
			|EQUIP_LIMIT_KICKBALL;

		//착용 제한
		int const iEquipLimit = pDef->GetAbil(AT_EQUIP_LIMIT);
		if ( true == bGambleItem )
		{
			const TCHAR *pText = NULL;
			if( GetAbilName(AT_LEVELLIMIT, pText) )
			{
				wstrText += pText;
				wstrText += WSTR_ATTR_EXPLAIN;
				wstrText += L"??\n";
			}

			if( GetAbilName(AT_CLASSLIMIT, pText) )
			{
				wstrText += pText;
				wstrText += WSTR_ATTR_EXPLAIN;
				wstrText += L"??\n";
			}

			if( GetAbilName(AT_EQUIP_LIMIT, pText) )
			{
				wstrText += pText;
				wstrText += WSTR_ATTR_EXPLAIN;
				wstrText += L"??\n";
			}
		}
		else
		{
			//장비 공통
			MakeAbilString(kItem, pDef, AT_LEVELLIMIT, pDef->GetAbil(AT_LEVELLIMIT), wstrText);	//착용 제한 최저 레벨
			if( true == pDef->IsPetItem() )
			{
				MakePetClassLimitText(pDef, wstrText);
			}
			else
			{
				MakeClassLimitText(pDef, wstrText);	//착용 제한 직업
			}

			if( iEquipLimit != EQUIP_LIMIT_MEDAL )
			{
				MakeAbilString(kItem, pDef, AT_DEFAULT_AMOUNT, pDef->MaxAmount(), wstrText);	//내구도
			}

			//MakeAbilString(kItem, pDef, AT_EQUIqP_LIMIT, pDef->GetAbil(AT_EQUIP_LIMIT), wstrText); // 사용가능직업
			//-> 착용 레벨 / 착용 직업 변경
			
		}
		
		UserCharacterIcon(kItem.ItemNo(), wstrText);
		MakeCantAbil(wstrText,pDef);
		
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 총 공격력/방어력
		bool bShowItemBaseInfo = false;
		{
			bool bPetItem = false;
			if( pDef->IsPetItem() )
			{
				bPetItem = (pDef->GetAbil(AT_PHY_ATTACK_MAX) || pDef->GetAbil(AT_PHY_ATTACK_MIN) || 
					pDef->GetAbil(AT_MAGIC_ATTACK_MAX) || pDef->GetAbil(AT_MAGIC_ATTACK_MIN));
			}
			if( EQUIP_LIMIT_WEAPON == (EQUIP_LIMIT_WEAPON & pDef->GetAbil(AT_EQUIP_LIMIT)) || bPetItem )
			{//무기
				if( 0 < pDef->ImproveAbil(AT_PHY_ATTACK_MAX, kItem) ||
				0 < pDef->ImproveAbil(AT_MAGIC_ATTACK_MAX, kItem) )
				{
					bShowItemBaseInfo = true;
				}
			}
			else
			{//방어구
				if( 0 < pDef->ImproveAbil(AT_PHY_DEFENCE, kItem) ||
				0 < pDef->ImproveAbil(AT_MAGIC_DEFENCE, kItem) )
				{
					bShowItemBaseInfo = true;
				}
			}
		}
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 총 공격력/방어력
		CUnit* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return false;
		}
		PgInventory* pkInv = pkPlayer->GetInven();
		if(!pkInv)
		{
			return false;
		}

		if( bShowItemBaseInfo )
		{
			PgItemUpgradeInfo kItemUpgradeInfo, kEqItemInfo;
			SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();

			kItemUpgradeInfo.SetBaseInfo(pkPlayer, kItem, pDef);
			kItemUpgradeInfo.SetPlusUpgrade(kItem, pDef, kEnchantInfo, kEnchantInfo.PlusLv());
			kItemUpgradeInfo.SetRarityUpgrade(pkPlayer, kItem, pDef, kEnchantInfo);
			kItemUpgradeInfo.SetSocketOption(pkPlayer, kItem, pDef, kEnchantInfo);
			kItemUpgradeInfo.SetRareOption(pkPlayer, kItem, pDef, kEnchantInfo);

			bool bShowDiffInfo = false;
			if( g_bIsEqComp )
			{
				if (IS_CLASS_LIMIT(pDef->GetAbil64(AT_CLASSLIMIT), pkPlayer->GetAbil(AT_CLASS)))
				{
					PgBase_Item kEqItem;
					if(S_OK == pkInv->GetItem(SItemPos(KUIG_FIT, pDef->EquipPos()), kEqItem))
					{
						GET_DEF(CItemDefMgr, kItemDefMgr);
						CItemDef const * pEqDef = kItemDefMgr.GetDef(kEqItem.ItemNo());
						if( pEqDef && kEqItem != kItem )
						{
							bShowDiffInfo = true;
							SEnchantInfo const& kEqEnchantInfo = kEqItem.EnchantInfo();
							kEqItemInfo.SetBaseInfo(pkPlayer, kEqItem, pEqDef);
							kEqItemInfo.SetPlusUpgrade(kEqItem, pEqDef, kEqEnchantInfo, kEqEnchantInfo.PlusLv());
							kEqItemInfo.SetRarityUpgrade(pkPlayer, kEqItem, pEqDef, kEqEnchantInfo);
							kEqItemInfo.SetSocketOption(pkPlayer, kItem, pEqDef, kEqEnchantInfo);
							kEqItemInfo.SetRareOption(pkPlayer, kEqItem, pEqDef, kEqEnchantInfo);
						}
					}
				}
			}
			kItemUpgradeInfo.MakeToolTipText_Total(wstrText, pDef, true);
			if( bShowDiffInfo )
			{
				kItemUpgradeInfo.MakeToolTipText_Diff(wstrText, pDef, true, kEqItemInfo);
			}
			wstrText += ENTER_STRING;

			kItemUpgradeInfo.MakeToolTipText_Total(wstrText, pDef, false);
			if( bShowDiffInfo )
			{
				kItemUpgradeInfo.MakeToolTipText_Diff(wstrText, pDef, false, kEqItemInfo);
			}
			wstrText += ENTER_STRING;
		}
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
		
		if(IG_CURSE == eItemLv)
		{// 저주는 빨갛게
			wstrText += TTW(89999);
		}

		bool EquipOption = false;
		{
			if ( true == bGambleItem )
			{
				wstrText += L"?? - ??\n?? - ??\n";
			}
			else if( bShowItemBaseInfo )
			{
				std::wstring kTempStr;
				if(iEquipLimit&iWeaponFlag || true==pDef->IsPetItem())
				{// 무기 공격력
					if( false == EquipOption )
					{
						wstrText += ENTER_STRING;
						wstrText += TTW(TITLE_COLOR_ORANGE);
						wstrText += TTW(5206); //착용옵션
						wstrText += TTW(CONTENTS_COLOR_WHITE);//폰트 초기화
						wstrText += ENTER_STRING;
						EquipOption = true;
					}

					if( S_FALSE == MakeAbilValueString(kItem, pDef, AT_PHY_ATTACK_MIN, pDef->ImproveAbil(AT_PHY_ATTACK_MIN, kItem), kTempStr, false) )
					{
						kTempStr += AREA_MARK_STRING;
						if( S_FALSE == MakeAbilValueString(kItem, pDef, AT_PHY_ATTACK_MAX, pDef->ImproveAbil(AT_PHY_ATTACK_MAX, kItem), kTempStr, g_bIsEqComp) )
						{
							MakeAbilNameString(AT_PHY_ATTACK_MIN, wstrText);
							wstrText += WSTR_ATTR_EXPLAIN;
							kTempStr += ENTER_STRING;
							wstrText += kTempStr;
						}
					}
					kTempStr.clear();
					if( S_FALSE == MakeAbilValueString(kItem, pDef, AT_MAGIC_ATTACK_MIN, pDef->ImproveAbil(AT_MAGIC_ATTACK_MIN, kItem), kTempStr, false) )
					{
						kTempStr += AREA_MARK_STRING;
						if( S_FALSE == MakeAbilValueString(kItem, pDef, AT_MAGIC_ATTACK_MAX, pDef->ImproveAbil(AT_MAGIC_ATTACK_MAX, kItem), kTempStr, g_bIsEqComp) )
						{
							MakeAbilNameString(AT_MAGIC_ATTACK_MIN, wstrText);
							wstrText += WSTR_ATTR_EXPLAIN;
							kTempStr += ENTER_STRING;
							wstrText += kTempStr;
						}
					}
				}
				if(iEquipLimit&iAmorFlag && false==pDef->IsPetItem())
				{// 방어구 방어력
					if( false == EquipOption )
					{
						wstrText += ENTER_STRING;
						wstrText += TTW(TITLE_COLOR_ORANGE);
						wstrText += TTW(5206);
						wstrText += TTW(CONTENTS_COLOR_WHITE);//폰트 초기화
						wstrText += ENTER_STRING;
						EquipOption = true;
					}

					kTempStr.clear();
					if( S_FALSE == MakeAbilValueString(kItem, pDef, AT_PHY_DEFENCE, pDef->ImproveAbil(AT_PHY_DEFENCE, kItem), kTempStr, g_bIsEqComp) )
					{
						MakeAbilNameString(AT_PHY_DEFENCE, wstrText);
						wstrText += WSTR_ATTR_EXPLAIN;
						kTempStr += ENTER_STRING;
						wstrText += kTempStr;
					}
					kTempStr.clear();
					if( S_FALSE == MakeAbilValueString(kItem, pDef, AT_MAGIC_DEFENCE, pDef->ImproveAbil(AT_MAGIC_DEFENCE, kItem), kTempStr, g_bIsEqComp) )
					{
						MakeAbilNameString(AT_MAGIC_DEFENCE, wstrText);
						wstrText += WSTR_ATTR_EXPLAIN;
						kTempStr += ENTER_STRING;
						wstrText += kTempStr;
					}
				}
			}
		}

		wstrText+=MakeElementToolTip(kItem, pDef);

		//아이템 기본 옵션
		std::wstring BaseAbilText;
		MakeBaseAbilStr(kItem, pDef, BaseAbilText);
		if( !BaseAbilText.empty() )
		{
			if(false == EquipOption )
			{
				wstrText += ENTER_STRING;
				wstrText += TTW(TITLE_COLOR_ORANGE);
				wstrText += TTW(5206);
				wstrText += TTW(CONTENTS_COLOR_WHITE);//폰트 초기화
				wstrText += ENTER_STRING;
				EquipOption = true;
			}

			wstrText += BaseAbilText;
		}

		//장비 속성
		if( iEquipLimit&iWeaponFlag || iAmorFlag == EQUIP_LIMIT_SHIRTS )
		{
			if( true == bGambleItem )
			{
				wstrText += TTW(207);
				wstrText += WSTR_ATTR_EXPLAIN;
				wstrText += UNKNOWN_NAME;
				wstrText += ENTER_STRING;
			}
			else if( kEnchantInfo.Attr() != 0 && kEnchantInfo.AttrLv() != 0 )
			{
				wstrText += TTW(207);
				wstrText += WSTR_ATTR_EXPLAIN;
				wstrText += TTW(5007 + kEnchantInfo.Attr());
				wchar_t	szValue[30] = {0,};
				swprintf(szValue, 29, TTW(5013).c_str(), kEnchantInfo.AttrLv());
				wstrText += szValue;
				wstrText += ENTER_STRING;
			}

			MakeItemEffectAbilText(kOrgItem, pDef, wstrText);
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 세트 아이템 한정
		MakeSetItemAbilString(kItem, pDef, bOrtherActor, wstrText);

#ifndef EXTERNAL_RELEASE // 익스터널 버전에서는 보이지 않음
		std::wstring wstrDropInfo;
		if(true == MakeToolTipTextDropInfo(pDef,wstrDropInfo))
		{
			wstrText += ENTER_STRING;
			wstrText += wstrDropInfo;
		}
#endif

	}
	else// 장착 불능 아이템
	{
		MakeCantAbil(wstrText,pDef);
		std::wstring	wstrUseType = L"";
		if( pDef->GetAbil(AT_JS3_NOT_EQUIP_ITEM) )
		{
			wstrText += ENTER_STRING;
			BM::vstring kLevelLimit(TTW(799881));
			kLevelLimit.Replace( L"#MIN#", pDef->GetAbil(AT_LEVELLIMIT));
			kLevelLimit.Replace( L"#MAX#", pDef->GetAbil(AT_MAX_LEVELLIMIT));
			wstrText += static_cast<std::wstring>(kLevelLimit);
			wstrText += ENTER_STRING;
		}
		else if(NULL != !(pDef->IsType(ITEM_TYPE_EQUIP))) 
		{
			const TCHAR *pText = NULL;
			int const iLevelLimit = pDef->GetAbil(AT_LEVELLIMIT);
			int const iMaxLevelLimit = pDef->GetAbil(AT_MAX_LEVELLIMIT);
			if(pDef->IsType(ITEM_TYPE_ETC)||pDef->IsType(ITEM_TYPE_QUEST))
			{
			}
			else
			{
				int const iUseLevel = 4;
				GetDefString(iUseLevel, pText);
				wstrText += ENTER_STRING;
				wstrText += pText;
				wstrText += L":  ";
				wstrText += BM::vstring(iLevelLimit).operator const std::wstring &();
				if(iMaxLevelLimit > 0)
				{
					wstrText += L" ~ ";
					wstrText += BM::vstring(iMaxLevelLimit).operator const std::wstring &();
				}
				wstrText += ENTER_STRING;
			}
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 수량
		{
			std::wstring kAbilText;
			MakeAbilString(kItem, pDef, AT_DEFAULT_AMOUNT, pDef->MaxAmount(), kAbilText, NULL, NULL, false, false, 0, false);//아이템 수량
			if( kAbilText.size() > 1)
			{
				wstrText += ENTER_STRING;
				wstrText += kAbilText;
				wstrText += ENTER_STRING;
			}
		}
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		bool bIsOnlyQuest = true;
		int i = 0, j = 0;
		for(i = ITEM_TYPE_ETC; i <= ITEM_TYPE_EVENT; i*=2, ++j)
		{
			//비 장비 공통
			if( pDef->IsType((EItemType)i) )
			{
				if( ((EItemType)i) != ITEM_TYPE_QUEST )
				{
					bIsOnlyQuest = false;
				}

				if(wstrUseType.size())
				{
					if(wstrUseType.substr(wstrUseType.size()-1, wstrUseType.size()).compare(L"/") != 0)
					{
						wstrUseType += L"/";
					}
				}
				wstrUseType += TTW(50007 + j);
			}
		}
		if(wstrUseType.size())
		{
			wstrText += ENTER_STRING;
			wstrText += TTW(50016);
			wstrText += ENTER_STRING;
			wstrText += TTW(40011);
			wstrText += wstrUseType;
			wstrText += ENTER_STRING;
			if( bIsOnlyQuest )
			{
				wstrText += ENTER_STRING;
				wstrText += TTW(89999);
				wstrText += TTW(5014);
				wstrText += TTW(40011);
				wstrText += ENTER_STRING;
			}
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 공통 출력 정보
	{
		std::wstring kAbilText;
		MakeAbilString(kItem, pDef, ATS_COOL_TIME, pDef->GetAbil(ATS_COOL_TIME), kAbilText);//사용 딜레이
		if( kAbilText.size() > 1 )
		{
			wstrText += ENTER_STRING;
			wstrText += kAbilText;
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 상태이상 회복 아이템
	{
		std::wstring kAbilText;
		MakeAbilString(kItem, pDef, AT_EFFECT_DELETE_ALL_DEBUFF, pDef->GetAbil(AT_EFFECT_DELETE_ALL_DEBUFF), kAbilText, NULL, NULL, false, false, 0, false);//상태이상 회복
		if( kAbilText.size() > 1 )
		{
			wstrText += ENTER_STRING;
			wstrText += kAbilText;
			wstrText += ENTER_STRING;
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 체력, 마력 회복 아이템
	if( pDef->CanConsume() && !kFlag.Enable(TTF_NOT_METHOD)) // 사용 가능 아이템
	{
		std::wstring kAbilText1, kAbilText2;
		MakeAbilString(kItem, pDef, AT_HP, pDef->GetAbil(AT_HP), kAbilText1, NULL, NULL, false, false, 0, false);//체력 회복
		MakeAbilString(kItem, pDef, AT_MP, pDef->GetAbil(AT_MP), kAbilText2, NULL, NULL, false, false, 0, false);//마력 회복
		
		if( kAbilText1.size() > 1 || kAbilText2.size() > 1 )
		{
			wstrText += ENTER_STRING;
		}
		if( kAbilText1.size() > 1 )
		{
			wstrText += kAbilText1;
		}
		if( kAbilText1.size() > 1 && kAbilText2.size() > 1 )
		{
			wstrText += ENTER_STRING;
		}
		if( kAbilText2.size() > 1 )
		{
			wstrText += kAbilText2;
		}
		if( kAbilText1.size() > 1 || kAbilText2.size() > 1 )
		{
			wstrText += ENTER_STRING;
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 체력, 마력 % 회복 아이템
	if( pDef->CanConsume() && !kFlag.Enable(TTF_NOT_METHOD)) // 사용 가능 아이템
	{
		BM::vstring vStr;
		int const RecoveryHP = pDef->GetAbil(AT_R_MAX_HP) / 100;
		int const RecoveryMP = pDef->GetAbil(AT_R_MAX_MP) / 100;

		if( RecoveryHP > 0 || RecoveryMP > 0 )
		{
			wstrText += ENTER_STRING;
		}
		if( RecoveryHP > 0 )
		{
			vStr = TTW(40907);
			vStr += RecoveryHP;
			vStr += L"%";
			wstrText += (std::wstring)vStr;
		}
		if( RecoveryHP > 0 && RecoveryMP > 0 )
		{
			wstrText += ENTER_STRING;
		}
		if( RecoveryMP > 0 )
		{
			vStr = TTW(40908);
			vStr += RecoveryMP;
			vStr += L"%";
			wstrText += (std::wstring)vStr;
		}
		if( RecoveryHP > 0 || RecoveryMP > 0 )
		{
			wstrText += ENTER_STRING;
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 지속형 체력, 마력 회복 아이템
	if( pDef->CanConsume() && !kFlag.Enable(TTF_NOT_METHOD)) // 사용 가능 아이템
	{
		int const DurationSec = pDef->GetAbil(AT_DURATION_TIME) / (ABILITY_RATE_VALUE * 0.1f);
		if( DurationSec > 0 )
		{
			for( int AbilType = AT_EFFECTNUM1; AbilType <= AT_EFFECTNUM10; ++AbilType )
			{
				int const EffectNo = pDef->GetAbil(AbilType);
				
				if( EffectNo )
				{
					GET_DEF(CEffectDefMgr, EffectDefMgr);
					CEffectDef const* pEffDef = EffectDefMgr.GetDef(EffectNo);
					if (pEffDef)
					{	
						int const RecoveryHP = pEffDef->GetAbil(AT_HP);
						int const RecoveryMP = pEffDef->GetAbil(AT_MP);
						int const Interval = pEffDef->m_sInterval / (ABILITY_RATE_VALUE * 0.1f);
						if( Interval > 0
							&& (RecoveryHP > 0 || RecoveryMP > 0 ) )
						{
							BM::vstring vStr;
							{// 지속시간
								vStr = TTW(40135);
								vStr += L" : ";
								vStr += DurationSec;
								vStr += TTW(9);
								wstrText += (std::wstring)vStr;
								wstrText += ENTER_STRING;
							}
							if( RecoveryHP > 0 )
							{
								vStr = TTW(40907);
								vStr += TTW(40910);
								vStr.Replace(L"#SEC#", Interval);
								vStr.Replace(L"#VALUE#", RecoveryHP);
								vStr.Replace(L"#TOTAL#", RecoveryHP * (DurationSec / Interval));
								wstrText += (std::wstring)vStr;
							}
							if( RecoveryHP && RecoveryMP )
							{
								wstrText += ENTER_STRING;
							}
							if( RecoveryMP > 0 )
							{
								vStr = TTW(40908);
								vStr += TTW(40910);
								vStr.Replace(L"#SEC#", Interval);
								vStr.Replace(L"#VALUE#", RecoveryMP);
								vStr.Replace(L"#TOTAL#", RecoveryMP * (DurationSec / Interval));
								wstrText += (std::wstring)vStr;
							}
							wstrText += ENTER_STRING;
							break;
						}
					}
				}
			}
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 직업기술 정보
	SJobSkillSaveIdx const * const pkJobSkillSaveIdx = JobSkill_Third::GetJobSkillSaveIdx(pDef->GetAbil(AT_JOBSKILL_SAVEIDX));
	MakeTextJobSkill_Info(pkJobSkillSaveIdx, wstrText);
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	// 툴팁이 해상도를 벗어났을 경우 텍스트를 나눌 위치를 찾기 위해 심어놓는 글자
	// 출력될때는 제외하고 출력한다.
	wstrText += CUT_TOOLTIP;

	if( pDef->CanEquip() && IG_SEAL != eItemLv )
	{//장비만 인챈트 가능. 봉인된 녀석은 옵션을 감춘다.
		//소켓 옵션을 출력할 곳
		std::wstring kEnchantText;
		kEnchantText = MakeSocketToolTip(kItem, kItemDefMgr);
		if( !kEnchantText.empty() )
		{
			wstrText += ENTER_STRING;
			wstrText += kEnchantText;
		}
		//인챈트 옵션을 출력할 곳
		kEnchantText = MakePlusEnchantToolTip(kItem, kItemDefMgr);
		if( !kEnchantText.empty() )
		{
			wstrText += ENTER_STRING;
			wstrText += kEnchantText;
		}

		//소울 크래프트 옵션을 출력할 곳
		kEnchantText = MakeSoulCraftToolTip(kItem, kItemDefMgr); // 이 함수 안에서 CUT_TOOLTIP2 삽입함
		if( !kEnchantText.empty() )
		{
			wstrText += ENTER_STRING;
			wstrText += kEnchantText;
		}
	}
		
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 옵션 설명
	if(IG_SEAL != eItemLv)//봉인은 추가 옵션이 보이지 않음.
	{
		std::wstring kEchantText;

		CONT_IDX2SKILLNO const *pkSkillIdxConvert = NULL;
		g_kTblDataMgr.GetContDef(pkSkillIdxConvert);
		if( pkSkillIdxConvert )
		{
			CONT_IDX2SKILLNO::const_iterator cvt_itor = pkSkillIdxConvert->find( kEnchantInfo.SkillOpt() );
			if( cvt_itor != pkSkillIdxConvert->end() )
			{
				CONT_IDX2SKILLNO::mapped_type const& kSkillNo = cvt_itor->second;
				MakeOverSkillToolTip(kEchantText, kSkillNo, kEnchantInfo.SkillOptLv(), false);
			}
		}

		int const iItemNo = GetMonsterCardItemNo(PgSocketFormula::SII_FIRST, kEnchantInfo.MagicOpt());//마법옵션
		if(0 < iItemNo)
		{
			int const iEffectNo = kItemDefMgr.GetAbil(iItemNo, AT_EFFECTNUM1);
			if(0 < iEffectNo )
			{
				kEchantText += TTW(5022);
				kEchantText += WSTR_ATTR_EXPLAIN;
				kEchantText += ENTER_STRING;
				MakeMonsterCardOptionString(kItem, pDef, iEffectNo, kEchantText, false);
			}
		}

		int const iEquipPos = pDef->EquipPos();
		int const iOptionGroup = pDef->OptionGroup();

		CONT_ENCHANT_ABIL kEnchantAbil;//대미궁 아이템 옵션
		GetAbilObject(kEnchantInfo.RareOptType1(), iEquipPos,  kEnchantInfo.RareOptLv1(), iOptionGroup, 0, kEnchantAbil);
		GetAbilObject(kEnchantInfo.RareOptType2(), iEquipPos,  kEnchantInfo.RareOptLv2(), iOptionGroup, 1, kEnchantAbil);
		GetAbilObject(kEnchantInfo.RareOptType3(), iEquipPos,  kEnchantInfo.RareOptLv3(), iOptionGroup, 2, kEnchantAbil);
		GetAbilObject(kEnchantInfo.RareOptType4(), iEquipPos,  kEnchantInfo.RareOptLv4(), iOptionGroup, 3, kEnchantAbil);
		CONT_ENCHANT_ABIL::const_iterator kItor = kEnchantAbil.begin();
		while( kItor != kEnchantAbil.end() )
		{
			MakeAbilStringColor((*kItor).wType, kEchantText); // 타입이 속성에 관계된 것이면 컬러를 바꾼다.
			MakeAbilString(kItem, pDef, (*kItor).wType, (*kItor).iValue, kEchantText);//레벨
			++kItor;
		}

		if( !kEchantText.empty() )
		{
			wstrText += ENTER_STRING;
			wstrText += TTW(40022);
			wstrText += TTW(40021);
			wstrText += ENTER_STRING;
			wstrText += kEchantText;
		}

		kEnchantAbil.clear();
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 어빌리티 설명
	//else
	//{
	//	if( pDef->CanEquip() )
	//	{
	//		wstrText += ENTER_STRING;
	//	}
	//}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 인챈트 점프
	AddToolTip_InchantForceUpgrade(kItem, wstrText);
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 인챈트 점프 끝

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 설명
	std::wstring strElement;
	bool const bHaveExtractElement = MakeElementTypeText(kItem.ItemNo(), strElement);
	if( bHaveExtractElement )
	{//원소 추출
		wstrText += ENTER_STRING;
		wstrText += TTW(792000);
		wstrText += ENTER_STRING;
		BM::vstring kText(TTW(792001));
		kText.Replace(L"#ELEMENT#", strElement);
		wstrText += static_cast<std::wstring>( kText );
		wstrText += ENTER_STRING;
		wstrText += TTW(40011);	//폰트 초기화
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 설명
	const TCHAR *pText = NULL;
	int Value = pDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
		int const iSetNo = kItemSetDefMgr.GetItemSetNo(kItem.ItemNo());
		if( false == iSetNo )
		{// 세트 아이템은 설명 안함 ㅇㅇ
			wstrText += ENTER_STRING;
			wstrText += TTW(40050);
			wstrText += TTW(40051);
			wstrText += pText;
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	// 안내
	if( pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) == UICT_SCROLL_QUEST )
	{
		wstrText += _T("\n");
		wstrText += TTW(40049);
		wstrText += TTW(40048);
		wstrText += _T("\n");
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 달성정보
	if( EQUIP_POS_MEDAL == pDef->EquipPos() )
	{
		CONT_DEF_ITEM2ACHIEVEMENT const *pkContDefItem2Ach = NULL;
		g_kTblDataMgr.GetContDef(pkContDefItem2Ach);

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();

		BM::vstring vStr;
		BM::vstring vStrPer;
		if(pkContDefItem2Ach && pkPlayer )
		{
			CONT_DEF_ITEM2ACHIEVEMENT::const_iterator iter = pkContDefItem2Ach->find(kItem.ItemNo());
			if(iter != pkContDefItem2Ach->end())
			{
				PgAchieveValueCtrl kValue((*iter).second.iSaveIdx, *pkPlayer);

				vStr = kValue.Now();
				vStr += L"/";
				vStr += kValue.Max();
				vStrPer = L"(";
				vStrPer += kValue.Rate();
				vStrPer += L"%)";
			}
		}

		if( vStr.size() )
		{
			wstrText += TTW(40059);
			wstrText += static_cast<std::wstring>(vStrPer);
			wstrText += ENTER_STRING;
			wstrText += TTW(40051);
			wstrText += static_cast<std::wstring>(vStr);
			wstrText += ENTER_STRING;
			wstrText += ENTER_STRING;
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 봉인 & 저주 한정
	switch(eItemLv)
	{
	case IG_SEAL:
		{
			wstrText += _T("\n");
			wstrText += TTW(50404);//특이사항

			wstrText += _T("\n");
			wstrText += TTW(40051);
			wstrText += TTW(50405);//봉인 설명
			wstrText += _T("\n");
		}break;
	case IG_CURSE:
		{
			wstrText += _T("\n");
			wstrText += TTW(50404);//특이사항

			wstrText += _T("\n");
			wstrText += TTW(40051);
			wstrText += TTW(50406);//저주 설명
			wstrText += _T("\n");
		}break;
	default:
		{
			if( true == bIsCash && kEnchantInfo.IsTimeLimit() )
			{
				if( !kItem.GetUseAbleTime() )
				{
					wstrText += _T("\n");
					wstrText += TTW(50404);//특이사항

					wstrText += _T("\n");
					wstrText += TTW(40051);
					wstrText += TTW(50407);//기간연장 설명
					wstrText += _T("\n");
				}
			}
		}break;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 요리 레시피 한정
	int const iMakingType = kItemDefMgr.GetAbil(kItem.ItemNo(), AT_MAKING_TYPE);
	int const iMakingNo = kItemDefMgr.GetAbil(kItem.ItemNo(), AT_ITEMMAKING_NO);
	if(	iMakingNo && (0 != (iMakingType & EMAKING_TYPE_COOKING)) )
	{
		SDefItemMakingData kMakingData;
		GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
		if (kItemMakingDefMgr.GetDefItemMakingInfo(kMakingData, iMakingNo))
		{
			wstrText += _T("\n");
			wstrText += TTW(40055);	// 요리 재료
			for (int i=0 ; i<MAX_SUCCESS_RATE_ARRAY ; i++)
			{
				int const iNeedItemNo = kMakingData.kNeedElements.aElement[i];
				int const iNeedItemCount = kMakingData.kNeedCount.aCount[i];
				if(	!iNeedItemNo ||
					!iNeedItemCount ||
					!GetItemName(iNeedItemNo, pText) )
				{
					continue;
				}
				wstrText += TTW(40051);	// 글씨 색깔.
				wstrText += pText;
				wstrText += _T(" ");
				wstrText += (std::wstring)(BM::vstring(iNeedItemCount));
				wstrText += TTW(1709);		// '개'
				wstrText += _T("\n");
			}
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 캐쉬 아이템 한정
	std::wstring kUseLimitTime;
	if( kItem.GetUseAbleTime() )
	{
		if( kItem.GetUseAbleTime() <= 0 )
		{
			kUseLimitTime = TTW(89999);//빨강
			kUseLimitTime += TTW(179);
			kUseLimitTime += TTW(40011);
		}
		else
		{
			int const iDay = kItem.GetUseAbleTime() / 60 / 60 / 24;
			int const iHour = kItem.GetUseAbleTime() / 60 / 60 % 24;
			int const iMin = kItem.GetUseAbleTime() / 60 % 60;

			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(403059).c_str(), iDay, iHour, iMin);
			kUseLimitTime = szTemp;
		}
	}
	else
	{
		if( true == bIsCash )
		{
			if( 0 != kItem.EnchantInfo().UseTime() )
			{
				kUseLimitTime = TTW(1894);
			}
		}
	}

	if( !kUseLimitTime.empty() )
	{
		wstrText += L"\n";
		if( EQUIP_POS_MEDAL == pDef->EquipPos() 
			&& false == bIsCash)
		{//업적인 경우
			bool bComplete = false;
			CONT_DEF_ITEM2ACHIEVEMENT const * pConvDef = NULL;
			g_kTblDataMgr.GetContDef(pConvDef);	//맵 데프
			CONT_DEF_ITEM2ACHIEVEMENT::const_iterator conv_itr = pConvDef->find(kItem.ItemNo());	//맵번호로 맵 데프에서 찾아서
			if( conv_itr != pConvDef->end() )
			{
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( pkPlayer )
				{
					PgAchievements* pkAchiv = pkPlayer->GetAchievements();
					if( pkAchiv )
					{
						bComplete = pkAchiv->IsComplete( conv_itr->second.iSaveIdx );
					}
				}
			}

			if( bComplete )
			{
				wstrText += TTW(1898); //사용시간			
			}
			else
			{
				wstrText += TTW(1899 ); //사용시간			
			}
		}
		else
		{//풍선인 경우
			wstrText += TTW(1895); //사용시간
		}
		wstrText += ENTER_STRING;
		wstrText += TTW(40011);
		wstrText += kUseLimitTime;
		wstrText += ENTER_STRING;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 경험치 카드 툴팁
	if(UICT_EXPCARD == pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE))
	{
		SExpCard kCard;
		if( kItem.Get(kCard) )
		{
			BM::vstring kStr(L"\n");
			kStr += TTW(50768);
			kStr += TTW(40106);
			kStr += L"\n";
			kStr += TTW(50769);
			kStr += WSTR_ATTR_EXPLAIN;
			kStr += kCard.CurExp();
			kStr += L"\n";
			kStr += TTW(50770);
			kStr += WSTR_ATTR_EXPLAIN;
			kStr += kCard.MaxExp();
			kStr += L"\n";
			wstrText += kStr.operator const std::wstring &();
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	if(0 < iCostumeMixGrade)
	{// 코스튬 조합 가능 하다는 텍스트
		wstrText += ENTER_STRING;
		wstrText += TTW(790741);
		wstrText += ENTER_STRING;
		wstrText += TTW(790742);
		wstrText += ENTER_STRING;
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 골동품 교환 가능한 아이템 한정
	if( lwDefenceMode::IsCanChangeAntique(EGT_ANTIQUE, kItem.ItemNo()) )
	{
		wstrText += ENTER_STRING;
		wstrText += TTW(750010);	// 컬러(노랑), 폰트
		wstrText += TTW(750011);
		wstrText += ENTER_STRING;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지

	CONT_DEFGEMSTORE const * pDefGemStore = NULL;
	g_kTblDataMgr.GetContDef(pDefGemStore);

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 에르늄 추출 가능한 아이템 한정(발키리 장비)
	if( lwDefenceMode::IsCanExchangeItem(pDefGemStore, kItem.ItemNo(), EGemStoreMenu::EGM_ELUNIUM) )
	{
		wstrText += ENTER_STRING;
		wstrText += TTW(750010);	// 컬러(노랑), 폰트
		wstrText += TTW(310008);
		wstrText += ENTER_STRING;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 순은 열쇠 만들기 가능한 아이템 한정
	if( lwDefenceMode::IsCanExchangeItem(pDefGemStore, kItem.ItemNo(), EGemStoreMenu::EGM_PURESILVER_KEY) )
	{
		wstrText += ENTER_STRING;
		wstrText += TTW(750010);	// 컬러(노랑), 폰트
		wstrText += TTW(310014);
		wstrText += ENTER_STRING;
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 아이템 판매 가격
	if(!kShopItemInfo.IsNull())
	{
		std::wstring kPrice;

		if( kShopItemInfo.iPrice )
		{
			//FormatMoney(kShopItemInfo.iPrice, kPrice);
			kPrice = GetMoneyString(kShopItemInfo.iPrice);
		}
		else
		{
			wchar_t	szCPValue[30] = {0,};
			swprintf(szCPValue, 29, TTW(5006).c_str(), kShopItemInfo.iCP * 10);
			kPrice = szCPValue;
			
		}

		wstrText += _T("\n");
		wstrText += TTW(40052);
		wstrText += TTW(40053);	
		wstrText += kPrice;
		wstrText += _T("\n");//Pg2DString에서 높이를 계산하는 공식이 마지막줄의 폰트 크기와 줄바꿈 횟수를 곱해서 문제가 발생함 그걸 위한 임시 방편.
	}
	else
	{
		bool const bIsStockShopOn = (!lwGetUIWnd("FRM_SHOP_STOCK").IsNil());

		if( !lwGetUIWnd("FRM_SHOP").IsNil() 
		||	!lwGetUIWnd("FRM_SHOP_CP").IsNil()
		||	!lwGetUIWnd("FRM_SHOP_JOB").IsNil()
		||	bIsStockShopOn)
		{
			int iSellPrice = pDef->SellPrice();

			if(bIsStockShopOn)
			{
				PgStore::CONT_GODDS::value_type kItemInfo;
				if(S_OK == g_kViewStore.GetGoods(kItem.ItemNo(), kItemInfo))
				{
					iSellPrice = kItemInfo.iPrice;
				}
			}

			if(iSellPrice)
			{
				__int64 iEarnPrice = 0;
				CalcSellMoney(kItem, iEarnPrice, iSellPrice, kItem.Count(), g_kPilotMan.GetPlayerUnit()->GetPremium());

				std::wstring kPrice;
				//FormatMoney(iEarnPrice, kPrice);
				kPrice = GetMoneyString(iEarnPrice);

				wstrText += _T("\n");
				wstrText += TTW(40052);
				wstrText += TTW(40053);
				wstrText += kPrice;
				wstrText += _T("\n");//Pg2DString에서 높이를 계산하는 공식이 마지막줄의 폰트 크기와 줄바꿈 횟수를 곱해서 문제가 발생함 그걸 위한 임시 방편.
			}
			else
			{
				wstrText += _T("\n");
				wstrText += TTW(40105);
				wstrText += TTW(5014);
				wstrText += _T("\n");
			}
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	AddToolTip_Manufacture(pDef, wstrText); //즉석 가공 텍스트 추가

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 수량 아이템 툴팁
	if( pDef->CanConsume() && !kFlag.Enable(TTF_NOT_METHOD)) // 사용 가능 아이템
	{
		wstrText += ENTER_STRING;
		wstrText += L"{C=0xFFFFA500/}";
		wstrText += TTW(4999);
	}
	if(!pDef->CanEquip() && !kFlag.Enable(TTF_NOT_AMONUT))// 장착 가능 아이템
	{
		int const iMaxLimit = pDef->GetAbil(AT_MAX_LIMIT);
		if( iMaxLimit > 1 )
		{			
			wstrText += TTW(1306);//40064//노랑
			wstrText += TTW(5015);
			wstrText += TTW(40011);
			wstrText += ENTER_STRING;
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<여기까지

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 귀속 아이템 툴팁
	if(pDef->CanEquip())// 장착 가능 아이템
	{
		int const bIsReversion = pDef->GetAbil(AT_REVERSION_ITEM);
		if( bIsReversion )
		{
			wstrText += _T("\n");
			wstrText += TTW(1306);//40064//노랑
			wstrText += _T("<");
			wstrText += TTW(5016 + kEnchantInfo.IsAttached());
			wstrText += _T(">");
			wstrText += TTW(40011);
		}
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< <<<<<<<<<<<여기까지
	
	wstrTextOut += wstrText;
	return true;
}

int GetWidthForCashToolTip(XUI::CXUI_Wnd *pkWnd, BM::vstring wstrText)
{
	BM::vstring wstrTemp(TTW(40003));
	wstrTemp += wstrText;
	pkWnd->Text(wstrTemp);
	//스타일 텍스트 뽑고
	XUI::CXUI_Style_String	kStyleName = pkWnd->StyleText();
	//길이 계산 하고
	POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleName));

	int iRtn = kTextSize.x + 60;

	return iRtn;
}

/*
//수정본
void CallToolTip_Cash(XUI::CXUI_Wnd* pkWnd, POINT3I &pt)
{
	typedef std::set< std::wstring > EQ_LIMIT_CONT;

	int iIndex = 0;
	pkWnd->GetCustomData(&iIndex, sizeof(iIndex));
	if(0>=iIndex)
	{
		return;
	}

	TBL_DEF_CASH_SHOP_ARTICLE kArticle;
	if(g_kCashShopMgr.GetItem(iIndex, kArticle))
	{
		XUI::CXUI_Wnd *pkToolTip = XUIMgr.Call(UNI(szToolTipPackageItemID), false);
		if(pkToolTip)
		{
			std::wstring kSetAbilString;

			BM::vstring wstrText;
			wstrText += TTW(40002);
			wstrText += kArticle.wstrName;//아이템 이름만 따와서
			wstrText += TTW(40011);	//폰트 초기화

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CONT_CASH_SHOP_ITEM::const_iterator item_it = kArticle.kContCashItem.begin();
			EQ_LIMIT_CONT	kLimitCont;
			int iIndex = 0;
			XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkToolTip->GetControl(L"BLD_ITEM_SLOT"));
			if( pBuild )
			{ //10개의 칸을 모두 비활성화 시켜 초기화 함
				for( int i=0; i<pBuild->CountY(); ++i )
				{
					BM::vstring vStr(L"FRM_ITEM_SLOT");
					vStr += i;
					XUI::CXUI_Wnd* pSlot = pkToolTip->GetControl(vStr);
					if( pSlot )
					{
						pSlot->Visible(false);
					}
				}
			}

			int const TT_OL_SIZE = 25;	//얘는 글과 경계 사이
			int const SLOT_PITCH = 10;
			int const TT_BOX_PITCH = 4;

			POINT2 const ptToolTipSize = pkToolTip->Size();
			pkToolTip->Size(600, pkToolTip->Size().y);//사이즈는 화면 절반만큼 미리 잡아주고
			BM::vstring wstrName;
			wstrName += TTW(40002);
			wstrName += kArticle.wstrName;//아이템 이름만 따와서

			XUI::CXUI_Wnd* pkTitleBox = pkToolTip->GetControl(L"FRM_TITLE_TEXT");
			if(pkTitleBox)
			{
				pkTitleBox->Size(pkToolTip->Size().x, pkTitleBox->Size().y);
				pkTitleBox->Text(wstrName);

				//스타일 텍스트 뽑고
				XUI::CXUI_Style_String kStyleName = pkTitleBox->StyleText();
				//길이 계산 하고
				POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleName));

				//길이 비교해서 아이템 이름이 더 길면 늘리고 짧으면 보통 크기로 냅두고
				if(iTOOLTIP_X_SIZE_MAX < kTextSize.x + TT_OL_SIZE)
				{
					pkToolTip->Size(TT_OL_SIZE + kTextSize.x, pkToolTip->Size().y);
				}
				else
				{
					pkToolTip->Size(iTOOLTIP_X_SIZE_MAX, pkToolTip->Size().y);
				}
				pkTitleBox->Size(pkToolTip->Size().x, pkTitleBox->Size().y);
				pkTitleBox->Location(TT_OL_SIZE / 2, TT_OL_SIZE / 2);
			}
			int iOriPrice = 0;
			std::vector<int> vecPrice;
			XUI::CXUI_Wnd* pkDay = pkWnd->Parent()->GetControl(L"FRM_DAY");
			BM::vstring strCashType;
			if( pkDay )
			{
				switch(g_kCashShopMgr.GetShopType())
				{
				case PgCashShop::ECASH_SHOP:
					{
						strCashType = TTW(403062);
					}break;
				case PgCashShop::EMILEAGE_SHOP:
					{
						strCashType = TTW(1819);
					}break;
				default:
					{
						_PgMessageBox("lwCashShop::MakeBuyValueStr", "Unknown shop type");
					}break;
				}

				while(kArticle.kContCashItem.end() != item_it)
				{
					TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
					pkDay->GetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));	//현재 선택된 가격
					int iItemPrice = g_kCashShopMgr.GetItemPrice(item_it->iItemNo, kPrice.bTimeType, kPrice.iItemUseTime, kPrice.IsDiscount );
					if( 0 == iItemPrice )
					{
						iOriPrice = 0;
						break;
					}
					vecPrice.push_back(iItemPrice);
					iOriPrice += iItemPrice;
					++item_it;
				}
			}
			size_t iStretchSize = pkTitleBox->Location().y + pkTitleBox->Size().y + TT_OL_SIZE;
			item_it = kArticle.kContCashItem.begin();
			int iPriceVecCount = 0;

			if(pBuild)
			{
				pBuild->Location(pBuild->Location().x, iStretchSize);
			}

			while(kArticle.kContCashItem.end() != item_it)
			{
				CItemDef const *pDef = kItemDefMgr.GetDef((*item_it).iItemNo);
				if(pDef)
				{
					bool bIconVisible = false;
					if( pBuild && kArticle.kContCashItem.size() > 1)
					{
						BM::vstring vStr(L"FRM_ITEM_SLOT");
						vStr += iIndex;
						XUI::CXUI_Wnd* pSlot = pkToolTip->GetControl(vStr);
						if( pSlot )
						{
							XUI::CXUI_Wnd* pIcon = pSlot->GetControl(L"IMG_ICON");
							if(pIcon)
							{
								const CONT_DEFRES* pContDefRes = NULL;
								g_kTblDataMgr.GetContDef(pContDefRes);

								CONT_DEFRES::const_iterator itrDest = pContDefRes->find(pDef->ResNo());
								if(itrDest != pContDefRes->end() )
								{
									pSlot->Visible(true);
									pSlot->Size(pkToolTip->Size().x - SLOT_PITCH, 100);
									pSlot->Location(SLOT_PITCH / 5, pSlot->Location().y);

									pIcon->UVUpdate(itrDest->second.UVIndex);
									pIcon->DefaultImgName(itrDest->second.strIconPath);
									bIconVisible = true;
								}
							}

						}
					}

					const wchar_t *pName = NULL;
					GetDefString(pDef->NameNo(), pName);//이름 디스플레이
					if( kArticle.bSaleType != ECST_PACKEGE )
					{
						wstrText = std::wstring((pName ? pName : _T("")));
						wstrText += L" x ";
						wstrText += (*item_it).iItemCount;
						wstrText += TTW(50413);
					}
					else
					{
						BM::vstring wstrLine;
						wstrLine = std::wstring((pName ? pName : _T("")));
						wstrLine += L" x ";
						wstrLine += (*item_it).iItemCount;
						wstrLine += TTW(50413);

						BM::vstring vStr(L"FRM_ITEM_SLOT");
						vStr += iIndex;
						XUI::CXUI_Wnd* pSlot = pkToolTip->GetControl(vStr);
						if(pSlot)
						{
							pSlot->Location(pSlot->Location().x, iStretchSize);

							XUI::CXUI_Wnd* pTextArea = pSlot->GetControl(L"FRM_TTEA_BOX");
							if(pTextArea)
							{
								//int iWidth = GetWidthForCashToolTip(pTextArea, wstrLine);
								pTextArea->Location( (SLOT_PITCH / 2) + 40, TT_BOX_PITCH);
								pTextArea->Size(pSlot->Size().x - 45, pSlot->Size().y);
								pTextArea->TextRect(POINT2(pTextArea->Size().x, pTextArea->Size().y));
								pTextArea->Text(wstrLine);
								//스타일 텍스트 뽑고
								XUI::CXUI_Style_String kStyleName = pTextArea->StyleText();
								//실제 텍스트 영역 계산
								POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleName));
								pTextArea->Size(pTextArea->Size().x, kTextSize.y);
							}
							//슬롯 높이 재조정 (남는 공백이 생기지 않도록)
							int iSlotHeight = 10;
							if(pTextArea)
							{
								iSlotHeight = pTextArea->Size().y;
							}
							if(bIconVisible)
							{
								XUI::CXUI_Wnd* pkIcon = pSlot->GetControl(L"IMG_ICON");
								if(pkIcon && pTextArea)
								{
									iSlotHeight = std::max(iSlotHeight, static_cast<int>(pkIcon->Size().y) );
								}
							}
							iSlotHeight += SLOT_PITCH; //각 슬롯간의 간격
							pSlot->Size(pSlot->Size().x, iSlotHeight);
							iStretchSize += iSlotHeight;
						}
					}

					if( pDef->CanEquip() )
					{
						std::wstring kLimit;

						PgBase_Item kItem;
						kItem.ItemNo( (*item_it).iItemNo );
						kItem.Count( pDef->MaxAmount() );

						if( kArticle.bSaleType != ECST_PACKEGE )
						{//패키지는 아이템 개별 옵션을 출력하지 않는다.
							std::wstring kBaseAbil;
							MakeBaseAbilStr(kItem, pDef, kBaseAbil);
							if( !kBaseAbil.empty() )
							{
								wstrText += _T("\n");
								wstrText += TTW(40020);
								wstrText += TTW(40051);
								wstrText += kBaseAbil;
							}
						}

						if( true == pDef->IsPetItem() )
						{
							MakePetClassLimitText(pDef, kLimit);
						}
						else
						{
							_MakeClassLimitText(pDef, kLimit);//착용 제한 직업
						}

						if( kSetAbilString.empty() )
						{//패키지 아이템은 무조건 1가지 세트로 구성한다. 출력은 나중에
							GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
							int const iSetNo = kItemSetDefMgr.GetItemSetNo(kItem.ItemNo());
							if( iSetNo )
							{//세트 번호 있다
								bool bSetItemView = true;
								if( kArticle.bSaleType == ECST_PACKEGE )
								{
									bSetItemView = false;
								}
								MakeSetItemAbilString(kItem, pDef, false, kSetAbilString, bSetItemView);
							}
						}

						kLimitCont.insert(kLimit);
					}
				}

				++iIndex;
				++item_it;
			}

			wstrText = _T("");
			wstrText += _T("\n");
			wstrText += TTW(1892);
			wstrText += TTW(40051);
			wstrText += _T("\n");
			wstrText += kArticle.wstrDiscription;
			wstrText += _T("\n");
			
			if( !kLimitCont.empty() )
			{
				wstrText += _T("\n");
				wstrText += TTW(1896);
				wstrText += TTW(40051);
				wstrText += _T("\n");

				EQ_LIMIT_CONT::iterator iter = kLimitCont.begin();
				if( iter != kLimitCont.end() )
				{
					wstrText += (*iter);
					wstrText += _T("\n");
				}
			}

			if( !kSetAbilString.empty() )
			{
				wstrText += kSetAbilString;
			}

			//XUI::CXUI_Wnd* pkDay = pkWnd->Parent()->GetControl(L"FRM_DAY");
			if(!pkDay)
			{
				for(int i = 0; i < 3; ++i)
				{
					BM::vstring vStr(L"FRM_DAY");
					vStr += i;

					XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(vStr));
					if( pBtn && pBtn->Check() )
					{
						pkDay = pBtn;
						break;
					}
				}
			}

			if(pkDay)
			{
				TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
				pkDay->GetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));	//현재 선택된 가격
				wstrText += _T("\n");
				wstrText += TTW(1893);
				wstrText += TTW(40051);
				wstrText += _T("\n");

				int iCash;
				if(kPrice.IsDiscount)
				{
					iCash = kPrice.iDiscountCash;
				}
				else
				{
					iCash = kPrice.iCash;
				}

				if( kArticle.bSaleType == ECST_PACKEGE )
				{
					if( iOriPrice )
					{
						wstrText += TTW(89999);//빨강
						wstrText += TTW(5201);
						wstrText += _T(" ");
						wstrText += iOriPrice;
						wstrText += _T(" ");
						wstrText += strCashType;
						wstrText += TTW(40021);
						wstrText += _T("\n");

						wstrText += TTW(5202);
					}
					wstrText += _T(" ");
					wstrText += iCash;
					wstrText += _T(" ");
					wstrText += strCashType;
				}
				else
				{
					wstrText += iCash;
					wstrText += _T(" ");
					wstrText += strCashType;
				}
				wstrText += _T("\n");
				

				
			}

			//pkToolTip->TextPos(POINT2(pkToolTip->TextPos().x, pkToolTip->TextPos().y + iStretchSize));
			//pkToolTip->Text(wstrText);
			XUI::CXUI_Wnd* pkPrice = pkToolTip->GetControl(L"FRM_PRICE_TEXT");
			if(pkPrice)
			{
				iStretchSize += TT_OL_SIZE;
				pkPrice->Location(TT_OL_SIZE / 2, iStretchSize);
				pkPrice->Size(pkToolTip->Size().x - TT_OL_SIZE, pkToolTip->Size().y);
				pkPrice->Text(wstrText);

				//텍스트 영역 만큼 크기 재조정
				//스타일 텍스트 뽑고
				XUI::CXUI_Style_String kStyleName = pkPrice->StyleText();
				//실제 텍스트 영역 계산
				POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleName));
				pkPrice->Size(pkToolTip->Size().x - TT_OL_SIZE, kTextSize.y);
				iStretchSize += kTextSize.y + (TT_OL_SIZE / 2);
			}
			pkToolTip->Size(pkToolTip->Size().x, iStretchSize);
			pkToolTip->Location(pt);		//툴팁 위치 잡아주고
			pkToolTip->Invalidate(true);	//그려주고
		}
	}
}
*/


//원본
void CallToolTip_Cash(XUI::CXUI_Wnd* pkWnd, POINT3I &pt)
{
	typedef std::set< std::wstring > EQ_LIMIT_CONT;

	int iIndex = 0;
	pkWnd->GetCustomData(&iIndex, sizeof(iIndex));
	if(0>=iIndex)
	{
		return;
	}

	TBL_DEF_CASH_SHOP_ARTICLE kArticle;
	if(g_kCashShopMgr.GetItem(iIndex, kArticle))
	{
		XUI::CXUI_Wnd *pkToolTip = XUIMgr.Call(UNI(szToolTipPackageItemID), false);
		if(pkToolTip)
		{
			std::wstring kSetAbilString;

			BM::vstring wstrText;
			wstrText += TTW(40002);
			wstrText += kArticle.wstrName;//아이템 이름만 따와서
			wstrText += TTW(40011);	//폰트 초기화
			wstrText += _T("\n\n");
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CONT_CASH_SHOP_ITEM::const_iterator item_it = kArticle.kContCashItem.begin();
			EQ_LIMIT_CONT	kLimitCont;
			int iIndex = 0;
			XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkToolTip->GetControl(L"BLD_ITEM_SLOT"));
			if( pBuild )
			{
				for( int i=0; i<pBuild->CountY(); ++i )
				{
					BM::vstring vStr(L"FRM_ITEM_SLOT");
					vStr += i;
					XUI::CXUI_Wnd* pSlot = pkToolTip->GetControl(vStr);
					if( pSlot )
					{
						pSlot->Visible(false);
					}
				}
			}

			int const TT_OL_SIZE = 25;	//얘는 글과 경계 사이

			pkToolTip->Size(600, pkToolTip->Size().y);//사이즈는 화면 절반만큼 미리 잡아주고
			BM::vstring wstrName;
			wstrName += TTW(40002);
			wstrName += kArticle.wstrName;//아이템 이름만 따와서

			pkToolTip->Text(wstrName);
			//스타일 텍스트 뽑고
			XUI::CXUI_Style_String	kStyleName = pkToolTip->StyleText();
			//길이 계산 하고
			POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleName));

			//길이 비교해서 아이템 이름이 더 길면 늘리고 짧으면 보통 크기로 냅두고
			if(iTOOLTIP_X_SIZE_MAX < kTextSize.x + TT_OL_SIZE)
			{
				pkToolTip->Size(TT_OL_SIZE + kTextSize.x, pkToolTip->Size().y);
			}
			else
			{
				pkToolTip->Size(iTOOLTIP_X_SIZE_MAX, pkToolTip->Size().y);
			}

			int iOriPrice = 0;
			std::vector<int> vecPrice;
			XUI::CXUI_Wnd* pkDay = pkWnd->Parent()->GetControl(L"FRM_DAY");
			BM::vstring strCashType;
			if( pkDay )
			{
				switch(g_kCashShopMgr.GetShopType())
				{
				case PgCashShop::ECASH_SHOP:
					{
						strCashType = TTW(403062);
					}break;
				case PgCashShop::EMILEAGE_SHOP:
					{
						strCashType = TTW(1819);
					}break;
				default:
					{
						_PgMessageBox("lwCashShop::MakeBuyValueStr", "Unknown shop type");
					}break;
				}

				while(kArticle.kContCashItem.end() != item_it)
				{
					TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
					pkDay->GetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));	//현재 선택된 가격
					int iItemPrice = g_kCashShopMgr.GetItemPrice(item_it->iItemNo, kPrice.bTimeType, kPrice.iItemUseTime, kPrice.IsDiscount );
					if( 0 == iItemPrice )
					{
						iOriPrice = 0;
						break;
					}
					vecPrice.push_back(iItemPrice);
					iOriPrice += iItemPrice;
					++item_it;
				}
			}

			item_it = kArticle.kContCashItem.begin();
			int iPriceVecCount = 0;
			while(kArticle.kContCashItem.end() != item_it)
			{
				CItemDef const *pDef = kItemDefMgr.GetDef((*item_it).iItemNo);
				if(pDef)
				{
					if( pBuild && kArticle.kContCashItem.size() > 1)
					{
						BM::vstring vStr(L"FRM_ITEM_SLOT");
						vStr += iIndex;
						XUI::CXUI_Wnd* pSlot = pkToolTip->GetControl(vStr);
						if( pSlot )
						{
							const CONT_DEFRES* pContDefRes = NULL;
							g_kTblDataMgr.GetContDef(pContDefRes);

							CONT_DEFRES::const_iterator itrDest = pContDefRes->find(pDef->ResNo());
							if(itrDest != pContDefRes->end() )
							{
								pSlot->Visible(true);
								pSlot->UVUpdate(itrDest->second.UVIndex);
								pSlot->DefaultImgName(itrDest->second.strIconPath);

								pkToolTip->Text(wstrText);
								POINT3I ptPos = pSlot->Location();
								ptPos.y = pkToolTip->Size().y - 14;
								pSlot->Location(ptPos);

								++iIndex;
							}
						}
					}

					const wchar_t *pName = NULL;
					GetDefString(pDef->NameNo(), pName);//이름 디스플레이
					if( kArticle.bSaleType != ECST_PACKEGE )
					{
						wstrText += std::wstring((pName ? pName : _T("")));
						wstrText += L" x ";
						wstrText += (*item_it).iItemCount;
						wstrText += TTW(50413);
						wstrText += _T("\n");
					}
					else
					{
						BM::vstring wstrLine;
						wstrLine += _T("        ");
						wstrLine += std::wstring((pName ? pName : _T("")));
						wstrLine += L" x ";
						wstrLine += (*item_it).iItemCount;
						wstrLine += TTW(50413);
						wstrText += wstrLine;

						int iWidth = GetWidthForCashToolTip(pkToolTip, wstrLine);
						POINT2 ptSize = pkToolTip->Size();
						if( iWidth > ptSize.x )
						{
							ptSize.x = iWidth;
							pkToolTip->Size(ptSize);
						}

						wstrText += _T("\n");
						if( iOriPrice )
						{
							wstrText += TTW(89999);//빨강
							wstrText += _T("        ");
							wstrText += vecPrice[iPriceVecCount++];
							wstrText += _T(" ");
							wstrText += strCashType;
							wstrText += TTW(40021);
						}
						wstrText += _T("\n");
						wstrText += _T("\n");
					}

					if( pDef->CanEquip() )
					{
						std::wstring kLimit;

						PgBase_Item kItem;
						kItem.ItemNo( (*item_it).iItemNo );
						kItem.Count( pDef->MaxAmount() );

						if( kArticle.bSaleType != ECST_PACKEGE )
						{//패키지는 아이템 개별 옵션을 출력하지 않는다.
							std::wstring kBaseAbil;
							MakeBaseAbilStr(kItem, pDef, kBaseAbil);
							if( !kBaseAbil.empty() )
							{
								wstrText += _T("\n");
								wstrText += TTW(40020);
								wstrText += TTW(40051);
								wstrText += kBaseAbil;
							}
						}

						if( true == pDef->IsPetItem() )
						{
							MakePetClassLimitText(pDef, kLimit);
						}
						else
						{
							_MakeClassLimitText(pDef, kLimit);//착용 제한 직업
						}

						if( kSetAbilString.empty() )
						{//패키지 아이템은 무조건 1가지 세트로 구성한다. 출력은 나중에
							GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
							int const iSetNo = kItemSetDefMgr.GetItemSetNo(kItem.ItemNo());
							if( iSetNo )
							{//세트 번호 있다
								bool bSetItemView = true;
								if( kArticle.bSaleType == ECST_PACKEGE )
								{
									bSetItemView = false;
								}
								MakeSetItemAbilString(kItem, pDef, false, kSetAbilString, bSetItemView);
							}
						}

						kLimitCont.insert(kLimit);
					}
				}

				++item_it;
			}

			wstrText += _T("\n");
			wstrText += TTW(1892);
			wstrText += TTW(40051);
			wstrText += _T("\n");
			wstrText += kArticle.wstrDiscription;
			wstrText += _T("\n");
			
			if( !kLimitCont.empty() )
			{
				wstrText += _T("\n");
				wstrText += TTW(1896);
				wstrText += TTW(40051);
				wstrText += _T("\n");

				EQ_LIMIT_CONT::iterator iter = kLimitCont.begin();
				if( iter != kLimitCont.end() )
				{
					wstrText += (*iter);
					wstrText += _T("\n");
				}
			}

			if( !kSetAbilString.empty() )
			{
				wstrText += kSetAbilString;
			}

			//XUI::CXUI_Wnd* pkDay = pkWnd->Parent()->GetControl(L"FRM_DAY");
			if(!pkDay)
			{
				for(int i = 0; i < 3; ++i)
				{
					BM::vstring vStr(L"FRM_DAY");
					vStr += i;

					XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(vStr));
					if( pBtn && pBtn->Check() )
					{
						pkDay = pBtn;
						break;
					}
				}
			}

			if(pkDay)
			{
				TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
				pkDay->GetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));	//현재 선택된 가격
				wstrText += _T("\n");
				wstrText += TTW(1893);
				wstrText += TTW(40051);
				wstrText += _T("\n");

				int iCash;
				if(kPrice.IsDiscount)
				{
					iCash = kPrice.iDiscountCash;
				}
				else
				{
					iCash = kPrice.iCash;
				}

				if( kArticle.bSaleType == ECST_PACKEGE )
				{
					if( iOriPrice )
					{
						wstrText += TTW(89999);//빨강
						wstrText += TTW(5201);
						wstrText += _T(" ");
						wstrText += iOriPrice;
						wstrText += _T(" ");
						wstrText += strCashType;
						wstrText += TTW(40021);
						wstrText += _T("\n");

						wstrText += TTW(5202);
					}
					wstrText += _T(" ");
					wstrText += iCash;
					wstrText += _T(" ");
					wstrText += strCashType;
				}
				else
				{
					wstrText += iCash;
					wstrText += _T(" ");
					wstrText += strCashType;
				}
				wstrText += _T("\n");
			}

			pkToolTip->Text(wstrText);

			pkToolTip->Location(pt);		//툴팁 위치 잡아주고
			pkToolTip->Invalidate(true);	//그려주고
		}
	}
}


void CallToolTip_GiftInCashShop(XUI::CXUI_Wnd* pkWnd, POINT3I &pt, SCASHGIFTINFO const& kCashShopGiftInfo)
{
	typedef std::set< std::wstring > EQ_LIMIT_CONT;

	int iIndex = 0;
	pkWnd->GetCustomData(&iIndex, sizeof(iIndex));
	if(0>=iIndex)
	{
		return;
	}

	TBL_DEF_CASH_SHOP_ARTICLE kArticle;
	if(g_kCashShopMgr.GetItem(iIndex, kArticle))
	{
		std::wstring kSetAbilString;

		BM::vstring wstrText;
		wstrText += TTW(40002);
		wstrText += kArticle.wstrName;//아이템 이름만 따와서
		wstrText += TTW(40011);	//폰트 초기화
		wstrText += _T("\n\n");
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CONT_CASH_SHOP_ITEM::const_iterator item_it = kArticle.kContCashItem.begin();
		EQ_LIMIT_CONT	kLimitCont;
		while(kArticle.kContCashItem.end() != item_it)
		{
			CItemDef const *pDef = kItemDefMgr.GetDef((*item_it).iItemNo);
			if(pDef)
			{
				const wchar_t *pName = NULL;
				GetDefString(pDef->NameNo(), pName);//이름 디스플레이
				wstrText += std::wstring((pName ? pName : _T("")));
				wstrText += L" x ";
				if(4 == kCashShopGiftInfo.bTimeType)
				{// 갯수를 표현 하는거라면
					wstrText += kCashShopGiftInfo.iUseTime;
				}
				else
				{// 갯수가 아니라면(기간제, 무기한)
					wstrText += (*item_it).iItemCount;
				}

				wstrText += TTW(50413);
				wstrText += _T("\n");
				if( pDef->CanEquip() )
				{
					std::wstring kLimit;

					PgBase_Item kItem;
					kItem.ItemNo( (*item_it).iItemNo );
					kItem.Count( pDef->MaxAmount() );

					if( kArticle.bSaleType != ECST_PACKEGE )
					{//패키지는 아이템 개별 옵션을 출력하지 않는다.
						std::wstring kBaseAbil;
						MakeBaseAbilStr(kItem, pDef, kBaseAbil);
						if( !kBaseAbil.empty() )
						{
							wstrText += _T("\n");
							wstrText += TTW(40020);
							wstrText += TTW(40051);
							wstrText += kBaseAbil;
						}
					}

					if( true == pDef->IsPetItem() )
					{
						MakePetClassLimitText(pDef, kLimit);
					}
					else
					{
						_MakeClassLimitText(pDef, kLimit);//착용 제한 직업
					}

					if( kSetAbilString.empty() )
					{//패키지 아이템은 무조건 1가지 세트로 구성한다. 출력은 나중에
						GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
						int const iSetNo = kItemSetDefMgr.GetItemSetNo(kItem.ItemNo());
						if( iSetNo )
						{//세트 번호 있다
							MakeSetItemAbilString(kItem, pDef, false, kSetAbilString);
						}
					}

					kLimitCont.insert(kLimit);
				}
			}

			++item_it;
		}

		wstrText += _T("\n");
		wstrText += TTW(1892);
		wstrText += TTW(40051);
		wstrText += _T("\n");
		wstrText += kArticle.wstrDiscription;
		wstrText += _T("\n");
		
		if( !kLimitCont.empty() )
		{
			wstrText += _T("\n");
			wstrText += TTW(1896);
			wstrText += TTW(40051);
			wstrText += _T("\n");

			EQ_LIMIT_CONT::iterator iter = kLimitCont.begin();
			if( iter != kLimitCont.end() )
			{
				wstrText += (*iter);
				wstrText += _T("\n");
			}
		}

		if( !kSetAbilString.empty() )
		{
			wstrText += kSetAbilString;
		}

		XUI::CXUI_Wnd* pkDay = pkWnd->Parent()->GetControl(L"FRM_DAY");
		if(!pkDay)
		{
			for(int i = 0; i < 3; ++i)
			{
				BM::vstring vStr(L"FRM_DAY");
				vStr += i;

				XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(vStr));
				if( pBtn && pBtn->Check() )
				{
					pkDay = pBtn;
					break;
				}
			}
		}

		if(pkDay)
		{
			TBL_DEF_CASH_SHOP_ITEM_PRICE kPrice;
			pkDay->GetCustomData(&kPrice, sizeof(TBL_DEF_CASH_SHOP_ITEM_PRICE));	//현재 선택된 가격
			wstrText += _T("\n");
			wstrText += TTW(1893);
			wstrText += TTW(40051);
			wstrText += _T("\n");
			if(kPrice.IsDiscount)
			{
				wstrText += kPrice.iDiscountCash;
			}
			else
			{
				wstrText += kPrice.iCash;
				
			}
			wstrText += _T(" ");
			switch(g_kCashShopMgr.GetShopType())
			{
			case PgCashShop::ECASH_SHOP:
				{
					wstrText += TTW(403062);
				}break;
			case PgCashShop::EMILEAGE_SHOP:
				{
					wstrText += TTW(1819);
				}break;
			default:
				{
					_PgMessageBox("lwCashShop::MakeBuyValueStr", "Unknown shop type");
				}break;
			}
			wstrText += _T("\n");
		}

		XUI::CXUI_Wnd *pkToolTip = XUIMgr.Call(UNI(szDefaultToolTipID), false);
		if(pkToolTip)
		{
			int const TT_OL_SIZE = 25;	//얘는 글과 경계 사이

			pkToolTip->Size(600, pkToolTip->Size().y);//사이즈는 화면 절반만큼 미리 잡아주고
			BM::vstring wstrName;
			wstrName += TTW(40002);
			wstrName += kArticle.wstrName;//아이템 이름만 따와서

			pkToolTip->Text(wstrName);
			//스타일 텍스트 뽑고
			XUI::CXUI_Style_String	kStyleName = pkToolTip->StyleText();
			//길이 계산 하고
			POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleName));

			//길이 비교해서 아이템 이름이 더 길면 늘리고 짧으면 보통 크기로 냅두고
			if(iTOOLTIP_X_SIZE_MAX < kTextSize.x + TT_OL_SIZE)
			{
				pkToolTip->Size(TT_OL_SIZE + kTextSize.x, pkToolTip->Size().y);
				pkToolTip->Text(wstrText); 
			}
			else
			{
				pkToolTip->Size(iTOOLTIP_X_SIZE_MAX, pkToolTip->Size().y);
				pkToolTip->Text(wstrText);
			}
			
			pkToolTip->Location(pt);		//툴팁 위치 잡아주고
			pkToolTip->Invalidate(true);	//그려주고
		}
	}
}

bool MakeActivateSkillToolTipText_Pet( BM::vstring &vstrText, PgItem_PetInfo const &kPetInfo, unsigned int iSkillIndex )
{
	size_t iIndex;
	bool bIsActivated = kPetInfo.FindSkillIndex( iSkillIndex+1, iIndex, false );
	if ( true == bIsActivated )
	{// 활성화 상태이다
		PgItem_PetInfo::SStateValue kSkillStateValue;
		kPetInfo.GetSkill( iIndex, kSkillStateValue );

		PgItem_PetInfo::SStateValue const kTemp(UINT_MAX);
		if ( kTemp.Time() == kSkillStateValue.Time() )
		{// 무제한
			vstrText += TTW(193);
		}
		else
		{
			__int64 const i64NowMinTime = g_kEventView.GetLocalSecTime(CGameTime::MINUTE);
			__int64 const i64StateTime = static_cast<__int64>(kSkillStateValue.Time()) + PgItem_PetInfo::SKILL_BASE_TIME;
			__int64 const i64RemainTime = ( i64StateTime - i64NowMinTime );

			if ( 1440i64 < i64RemainTime )
			{
				vstrText += TTW(191);

				__int64 i64RamainHourTime = ( i64RemainTime / 60i64 );
				if ( i64RemainTime % 60i64 )
				{
					++i64RamainHourTime;
				}
				__int64 i64RemainDayTime = i64RamainHourTime / 24i64;
				i64RamainHourTime %= 24i64;

				vstrText.Replace( L"#HOUR#", i64RamainHourTime );
				vstrText.Replace( L"#DAY#", i64RemainDayTime );
			}
			else if ( 0i64 > i64RemainTime )
			{
				bIsActivated = false;
			}
			else
			{
				vstrText += TTW(192);
				vstrText.Replace( L"#HOUR#", ( i64RemainTime / 60i64 ) );
				vstrText.Replace( L"#MIN#", ( i64RemainTime % 60i64 ) );
			}
		}
	}
	return bIsActivated;
}

int GetPetActivateSkillItemNo(size_t const iIndex)
{
	switch( iIndex )
	{
	case 1:{return 65002740;}
	case 11:{return 65002790;}
	case 12:{return 65002800;}
	}
	return 0;
}

bool MakeSkillToolTipText_Pet(int const iSkillNo, std::wstring &wstrTextOut, std::wstring &wstrType)
{	
	PgItem_PetInfo kPetInfo;
	if ( !GetMySelectedPetInfo( kPetInfo ) )
	{
		return false;
	}

	GET_DEF( PgClassPetDefMgr, kClassPetDefMgr);
	PgClassPetDef kPetDef;
	if ( !kClassPetDefMgr.GetDef( kPetInfo.ClassKey(), &kPetDef ) )
	{
		return false;
	}

	CONT_DEFCLASS_PET_SKILL const *pkDefPetSkill = NULL;
	g_kTblDataMgr.GetContDef(pkDefPetSkill);

	CONT_DEFCLASS_PET_SKILL::const_iterator skill_itr = pkDefPetSkill->find( kPetDef.GetSkillDefID() );
	if ( skill_itr == pkDefPetSkill->end() )
	{
		return false;
	}

	CONT_DEFCLASS_PET_SKILL::mapped_type const &kContElement = skill_itr->second;
	size_t const index = kContElement.find( PgDefClassPetSkillFinder(kPetInfo.ClassKey().nLv) );
	if ( BM::PgApproximate::npos == index )
	{
		return false;
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	if(!pSkillDef)
	{
		return false;
	}

	WCHAR	temp[500] = {0,};

	//	현재 레벨 스킬의 정보를 보여준다.
	wchar_t const* pName = NULL;
	GetDefString(pSkillDef->NameNo(),pName);

	//	이름용 색상 
	wstrTextOut += TTW(40102); //이름용

	//	이름
	if(pName)
	{	
		swprintf(temp,500,_T("%s {C=0xFF000000}%s %d"),pName,TTW(224).c_str(),pSkillDef->GetAbil(AT_LEVEL));
		wstrTextOut+=temp;
	}
	else
	{
		wstrTextOut+=_T("Undefined Name");
	}

	//분류
	wstrType+=TTW(40140+pSkillDef->m_byType);

	#ifndef USE_INB
	if( g_pkApp->VisibleClassNo() )
	{
		BM::vstring kSkillNo(pSkillDef->No());
		wstrTextOut += _T("\n SkillNo : ") + (std::wstring)kSkillNo;
	}
#endif

	//	공백
	wstrTextOut+=_T("\n\n");

	wstrTextOut += TTW(226);	//폰트 초기화

	//	소모 MP
	if (EST_PASSIVE != pSkillDef->m_byType)	//패시브 스킬아닌 경우
	{
		if(0<pSkillDef->m_sMP)
		{
			swprintf(temp,500,_T("%s : %d\n"),TTW(40133).c_str(), pSkillDef->m_sMP);
			wstrTextOut+=temp;
		}
		else
		{
			//소모 MP가 MAXMP의 %를 소모 할 경우
			int const iNeedRateMaxMP = pSkillDef->GetAbil(AT_NEED_MAX_R_MP);
			if(0 < iNeedRateMaxMP)
			{
				float const fRateMP = static_cast<float>(iNeedRateMaxMP / 100.0f);
				swprintf(temp,500,_T("%s : %s %.1f%%\n"),TTW(40133).c_str(), TTW(512).c_str(), fRateMP);
				wstrTextOut+=temp;
			}
		}
	}

	if (0 < pSkillDef->m_sHP)
	{
		swprintf(temp,500,_T("%s : %d\n"),TTW(40134).c_str(), pSkillDef->m_sHP);
		wstrTextOut+=temp;		
	}
	else
	{
		//소모 MP가 MAXHP의 %를 소모 할 경우
		int const iNeedRateMaxHP = pSkillDef->GetAbil(AT_NEED_MAX_R_HP);
		if(0 < iNeedRateMaxHP)
		{
			float const fRateHP = static_cast<float>(iNeedRateMaxHP / 100.0f);
			swprintf(temp,500,_T("%s : %s %.1f%%\n"),TTW(40134).c_str(), TTW(511).c_str(), fRateHP);
			wstrTextOut+=temp;
		}
	}

	CSkillDef const* pBackupSkillDef = NULL;
	//실제 공격 스킬은 따로 존재 하는 경우
	int iRealSkill = pSkillDef->GetAbil(AT_CHILD_CAST_SKILL_NO);
	if(0 < iRealSkill)
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pChildSkillDef = kSkillDefMgr.GetDef(iRealSkill);
		if(pChildSkillDef)
		{
			pBackupSkillDef = pSkillDef;
			pSkillDef = pChildSkillDef;
		}
	}

	//데미지를 나눠서 공격하는 스킬은 한발 데미지로 나누어 주어야 한다.
	int iComboCount = pSkillDef->GetAbil(AT_COMBO_HIT_COUNT);

	//	물리대미지
	int iAtk = pSkillDef->GetAbil(AT_PHY_DMG_PER);
	if(0 < iComboCount)
	{
		iAtk /= iComboCount;
	}

	if( 0 < iAtk )
	{
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(384).c_str(), ((float)(iAtk) * 0.01f));
		wstrTextOut+=temp;
		
		wstrTextOut+=L"\n";
	}

	//	마법대미지
	int iMAtk = pSkillDef->GetAbil(AT_MAGIC_DMG_PER);
	if(0 < iComboCount)
	{
		iMAtk /= iComboCount;
	}

	if( 0 < iMAtk )
	{
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(385).c_str(), ((float)(iMAtk) * 0.01f));
		wstrTextOut+=temp;
		
		wstrTextOut+=L"\n";
	}

	//데미지만 실제 공격 스킬에서 얻어 오고 원래 스킬로 돌려 준다.
	if(NULL != pBackupSkillDef)
	{
		pSkillDef = pBackupSkillDef;
	}

	wstrTextOut += TTW(40011);	//폰트 초기화

	//	제한레벨
	if(kPetInfo.GetAbil(AT_LEVEL)<pSkillDef->m_sLevelLimit)	//	빨간색으로 표시하자
	{
		wstrTextOut+=TTW(225);
	}

	if(0<pSkillDef->m_sLevelLimit)
	{
		swprintf(temp,100,_T("%s : %d %s\n"),TTW(230).c_str(),pSkillDef->m_sLevelLimit,TTW(232).c_str());
		wstrTextOut+=temp;
	}

	wstrTextOut += TTW(40011);	//폰트 초기화

	// 활성화 여부
	CONT_DEFCLASS_PET_SKILL::mapped_type::value_type const &kDefPetSkill = kContElement.at(index);
	for ( size_t i = 0 ; i<MAX_PET_SKILLCOUNT; ++i )
	{
		if ( kDefPetSkill.iSkillNo[i] == iSkillNo )
		{
			int const iCheckBit = ( 1 << i );
			if ( kDefPetSkill.iSkillUseTimeBit & iCheckBit )
			{
				BM::vstring vstrText(TTW(190));
				vstrText.Replace( L"#NAME# ", std::wstring(L"") );
				vstrText += L" : ";

				if ( !MakeActivateSkillToolTipText_Pet( vstrText, kPetInfo, i )  )
				{// 비활성화 상태
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pDef = kItemDefMgr.GetDef( GetPetActivateSkillItemNo(i) );
					if ( pDef )
					{
						const wchar_t *pkName = NULL;
						if(GetDefString(pDef->NameNo(), pkName) )
						{
							std::wstring wstrItemName(pkName);
							size_t iFindPos = wstrItemName.find( L"(" );
							if ( std::wstring::npos != iFindPos )
							{
								while( 0 < iFindPos )
								{
									if ( L' ' != wstrItemName.at(--iFindPos) )
									{
										++iFindPos;
										break;
									}
								}
								wstrItemName.erase( iFindPos, wstrItemName.size() );
							}

							vstrText += TTW(189);
							vstrText.Replace( L"#NAME#", wstrItemName );
						}
					}
				}

				wstrTextOut += static_cast<std::wstring>(vstrText);
				wstrTextOut += std::wstring(L"\n");
			}

			break;// break
		}
	}

	//	시전 시간
	int const iCast = pSkillDef->GetAbil(AT_CAST_TIME);
	
	if (0<iCast)
	{
		if( 0 < ((iCast % 1000) * 0.01) )
			swprintf(temp,500,_T("%s : %.1f%s\n"),TTW(47).c_str(), iCast*0.001f, TTW(54).c_str());
		else
			swprintf(temp,500,_T("%s : %d%s\n"),TTW(47).c_str(), static_cast<int>(iCast*0.001f), TTW(54).c_str());
	}
	else
	{
		swprintf(temp, 500, _T("%s : %s\n"), TTW(47).c_str(), TTW(251).c_str());
	}
	wstrTextOut += temp;

	//	쿨타임
	int const iCool = pSkillDef->GetAbil(ATS_COOL_TIME);
	if (0<iCool)
	{
		if( 0 < ((iCool % 1000) * 0.01) )
			swprintf(temp,500,_T("%s : %.1f%s\n"),TTW(250).c_str(), iCool*0.001f, TTW(54).c_str());
		else
			swprintf(temp,500,_T("%s : %d%s\n"),TTW(250).c_str(), static_cast<int>(iCool*0.001f), TTW(54).c_str());
		wstrTextOut+=temp;
	}

	int const iMaxTarget = pSkillDef->GetAbil(AT_MAX_TARGETNUM);
	if(1 < iMaxTarget)
	{// 최대타격개체수
		BM::vstring vTemp( TTW(40151) ); 
		vTemp.Replace(L"#NUM#", iMaxTarget);
		vTemp+="\n";
		wstrTextOut+=vTemp.operator const std::wstring &();
	}
	
	int const iMaxPenetration = pSkillDef->GetAbil(AT_PENETRATION_COUNT);
	if(0 < iMaxPenetration)
	{// 최대관통개체수
		BM::vstring vTemp( TTW(40152) ); 
		vTemp.Replace(L"#NUM#", iMaxPenetration);
		vTemp+="\n";
		wstrTextOut+=vTemp.operator const std::wstring &();
	}


	//지속 시간
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	const	CEffectDef* pEffDef = kEffectDefMgr.GetDef(pSkillDef->No());
	if (pEffDef)
	{
		int const iDuration = pEffDef->GetDurationTime()/1000;
		if (0<iDuration)
		{
			swprintf(temp,500,_T("%s : "),TTW(40135).c_str());
			wstrTextOut+=temp;
			GetTimeString(iDuration, wstrTextOut, true);
		}
	}
	bool bCantBlockTxt = false;
	//	설명
	const TCHAR *pText = NULL;
	int Value = pSkillDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		if (strlenT(pText))
		{
			swprintf(temp,100,_T("\n%s : \n"),TTW(231).c_str());
			wstrTextOut+= ::TTW(750010);	//노랑
			wstrTextOut+=temp;
			wstrTextOut+= ::TTW(1554);		//흰색
			wstrTextOut+=pText;
		}
		if(pEffDef
			&& pEffDef->GetAbil(AT_CAN_NOT_BLOCK_BY_EFFECT) 
			)
		{
			bCantBlockTxt = true;
		}
	}

	if( !bCantBlockTxt
		&& 0 < pSkillDef->GetAbil(AT_CAN_NOT_BLOCK_BY_ACTION) 
		)
	{
		bCantBlockTxt = true;
	}
	if( bCantBlockTxt )
	{// 블록 할 수 없습니다.
		wstrTextOut += ENTER_STRING; 
		wstrTextOut += ENTER_STRING;
		wstrTextOut += TTW(750010);	// 컬러(노랑), 폰트
		wstrTextOut += TTW(791611);
	}

	return true;
}

int const MINIMUM_ENDURANCE_VALUE = 5;
size_t FindRepairInfo(BaseItemMap &rkMap, PgInventory *pkInv)
{
	rkMap.clear();
	if(NULL==pkInv)
	{
		PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
		if(!pPilot){return 0;}

		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pPilot->GetUnit());
		if(!pkPlayer){return 0;}
		pkInv = pkPlayer->GetInven();

		if(NULL==pkInv)
		{
			return 0;
		}
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	for (int i = 0;  i< iEquipArrSize; ++i)
	{
		PgBase_Item kItem;;
		if(S_OK == pkInv->GetItem(SItemPos(IT_FIT, iEquipArr[i]), kItem))
		{
			CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if (pDef)
			{
				if (kItem.EnchantInfo().IsNeedRepair() || (MINIMUM_ENDURANCE_VALUE >= kItem.Count()))
				{
					rkMap.insert(std::make_pair(i, kItem));//필요한 것들만 모으자
				}
			}
		}
	}

	return rkMap.size();
}

void MakeToolTipText_Repair(std::wstring& rkText, BaseItemMap const& rkMap)
{
	BaseItemMap::const_iterator repair_it = rkMap.begin();
	BM::vstring kString;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	std::wstring kTemp;
	while(rkMap.end()!=repair_it)
	{
		PgBase_Item const & rkItem = (*repair_it).second;
		if(rkItem.IsEmpty())
		{
			++repair_it;
			continue;
		}
		CItemDef const *pkItemDef = kItemDefMgr.GetDef(rkItem.ItemNo());	
		if(!pkItemDef)
		{
			++repair_it;
			continue;
		}

		wchar_t const* pkItemName = NULL;
		if( false == GetItemName(rkItem.ItemNo(), pkItemName) )
		{
			++repair_it;
			continue;
		}
		kTemp.clear();
		int const iCurDurability = ((rkItem.EnchantInfo().IsNeedRepair())?0:rkItem.Count());//수리요구면 내구도를 0으로 표시.
		WstringFormat(kTemp, MAX_PATH, TTW(1292).c_str(), pkItemName, iCurDurability, pkItemDef->MaxAmount());
		rkText+=kTemp;
		
		++repair_it;
	}

	rkText+=TTW(1282);
}

void lwCallToolTipRepair(lwPoint2 &pt)
{
	BaseItemMap kMap;
	size_t const kCount = FindRepairInfo(kMap);
	if(0>=kCount)
	{
		return;
	}

	std::wstring kTooltipText;
	MakeToolTipText_Repair(kTooltipText, kMap);

	lwCallMutableToolTipByText(kTooltipText, pt);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PgItemUpgradeInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PgItemUpgradeInfo::PgItemUpgradeInfo()
{
	Clear();
}

PgItemUpgradeInfo::PgItemUpgradeInfo(eUpgradeFlag const flag)
{
	Clear();
	SetType(flag);
}

void PgItemUpgradeInfo::operator -= (PgItemUpgradeInfo const &rhs)
{
	this->m_kMinATK -= rhs.MinATK();
	this->m_kMaxATK -= rhs.MaxATK();
	this->m_kMinMATK -= rhs.MinMATK();
	this->m_kMaxMATK -= rhs.MaxMATK();
	this->m_kDEF -= rhs.DEF();
	this->m_kMDEF -= rhs.MDEF();
	this->m_kHP -= rhs.HP();
	this->m_kMP -= rhs.MP();
	this->m_kSTR -= rhs.STR();
	this->m_kCON -= rhs.CON();
	this->m_kINT -= rhs.INT();
	this->m_kDEX -= rhs.DEX();
	this->m_kHIT_RATE -= rhs.HIT_RATE();
	this->m_kDODGE_RATE -= rhs.DODGE_RATE();
	this->m_kHP_RECOVERY -= rhs.HP_RECOVERY();
	this->m_kMP_RECOVERY -= rhs.MP_RECOVERY();

	//쓰지 않는 함수. m_kEtcAbils는 어찌 함?
}

void PgItemUpgradeInfo::operator += (PgItemUpgradeInfo const &rhs)
{
	this->m_kMinATK += rhs.MinATK();
	this->m_kMaxATK += rhs.MaxATK();
	this->m_kMinMATK += rhs.MinMATK();
	this->m_kMaxMATK += rhs.MaxMATK();
	this->m_kDEF += rhs.DEF();
	this->m_kMDEF += rhs.MDEF();
	this->m_kHP += rhs.HP();
	this->m_kMP += rhs.MP();
	this->m_kSTR += rhs.STR();
	this->m_kCON += rhs.CON();
	this->m_kINT += rhs.INT();
	this->m_kDEX += rhs.DEX();
	this->m_kHIT_RATE += rhs.HIT_RATE();
	this->m_kDODGE_RATE += rhs.DODGE_RATE();
	this->m_kHP_RECOVERY += rhs.HP_RECOVERY();
	this->m_kMP_RECOVERY += rhs.MP_RECOVERY();

	this->m_kEtcAbils+=rhs.m_kEtcAbils;
}

PgItemUpgradeInfo PgItemUpgradeInfo::operator - (PgItemUpgradeInfo const &rhs) const
{
	PgItemUpgradeInfo temp = *this;
	temp -= rhs;
	return temp;
}

PgItemUpgradeInfo PgItemUpgradeInfo::operator + (PgItemUpgradeInfo const &rhs) const
{
	PgItemUpgradeInfo temp = *this;
	temp += rhs;
	return temp;
}

bool PgItemUpgradeInfo::IsEmpty()
{
	if( MinATK()
		&& !MaxATK()
		&& !MinMATK()
		&& !MaxMATK()
		&& !DEF()
		&& !MDEF() 
		&& !HP() 
		&& !MP() 
		&& !STR()
		&& !CON()
		&& !INT()
		&& !DEX()
		&& !HIT_RATE()
		&& !DODGE_RATE()
		&& !HP_RECOVERY()
		&& !MP_RECOVERY()
		&& m_kEtcAbils.IsEmpty())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void PgItemUpgradeInfo::Clear()
{
	MinATK(0);
	MaxATK(0);
	MinMATK(0);
	MaxMATK(0);
	DEF(0);
	MDEF(0);
	HP(0);
	MP(0);
	STR(0);
	CON(0);
	INT(0);
	DEX(0);
	HIT_RATE(0);
	DODGE_RATE(0);
	HP_RECOVERY(0);
	MP_RECOVERY(0);

	eFlag = E_UF_BASE;
	m_kEtcAbils.Clear();

	m_pUnit = NULL;
}


void PgItemUpgradeInfo::SetPlusUpgrade(PgBase_Item const& rkItem, CItemDef const* pkItemDef, SEnchantInfo const& rkEnchantInfo, int const iEnchantLv)
{
	if( !pkItemDef ) return;
	SetType(E_UF_PLUS);

	CAbilObject kPlusAbil;
	int const iGenderLimit = pkItemDef->GetAbil(AT_GENDERLIMIT);
	SItemEnchantKey const kKey( static_cast<int>(rkEnchantInfo.PlusType()) + ( pkItemDef->IsPetItem() ? SItemEnchantKey::ms_iPetBaseType : 0 ), iEnchantLv );

	GET_DEF(CItemEnchantDefMgr, kItemEnchantDefMgr);
	const CItemEnchantDef *pEnchantDef = kItemEnchantDefMgr.GetDef(kKey);
	if( pEnchantDef )
	{ 
		kPlusAbil += *pEnchantDef;
		if( SetAbil(NULL, rkItem, pkItemDef, kPlusAbil) )
		{
			eFlag |= E_UF_PLUS;
		}
	}
}

void PgItemUpgradeInfo::SetRarityUpgrade(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, SEnchantInfo const& rkEnchantInfo)
{
	if( !pkItemDef ) return;
	SetType(E_UF_RAIRITY);
	int const iEquipPos = pkItemDef->EquipPos();
	int const iLevelLimit = pkItemDef->GetAbil(AT_LEVELLIMIT);
	SBasicOptionAmpKey const kAmpKey(GetEquipType(pkItemDef), iLevelLimit, rkEnchantInfo.BasicAmpLv());
	CONT_ENCHANT_ABIL kEnchantAbil;
	::GetAbilObject(rkEnchantInfo.BasicType1(), iEquipPos, rkEnchantInfo.BasicLv1(), 0, 0, kEnchantAbil, kAmpKey);
	::GetAbilObject(rkEnchantInfo.BasicType2(), iEquipPos, rkEnchantInfo.BasicLv2(), 0, 1, kEnchantAbil, kAmpKey);
	::GetAbilObject(rkEnchantInfo.BasicType3(), iEquipPos, rkEnchantInfo.BasicLv3(), 0, 2, kEnchantAbil, kAmpKey);
	::GetAbilObject(rkEnchantInfo.BasicType4(), iEquipPos, rkEnchantInfo.BasicLv4(), 0, 3, kEnchantAbil, kAmpKey);

	CONT_ENCHANT_ABIL::iterator iter = kEnchantAbil.begin();
	while( iter != kEnchantAbil.end() )
	{// 소울크래프트 인첸트 어빌 중에 공격력, 방어력을 %로 올려주는 것들에 증폭이 걸려있다면 증폭 값만큼 더해준다.
		if( (AT_R_PHY_ATTACK_MIN			== iter->wType)
			|| (AT_R_MAGIC_ATTACK_MIN		== iter->wType)
			|| (AT_R_PHY_ATTACK_MAX			== iter->wType)
			|| (AT_R_MAGIC_ATTACK_MAX		== iter->wType)
			|| (AT_I_PHY_DEFENCE_ADD_RATE	== iter->wType)
			|| (AT_I_MAGIC_DEFENCE_ADD_RATE == iter->wType) )
		{
			int const Rate = PgItemRarityUpgradeFormula::GetBasicOptionAmpRate(iter->wType, GetEquipType(pkItemDef), iLevelLimit, rkEnchantInfo.BasicAmpLv());
			if( Rate )
			{
				iter->iValue = iter->iValue * (Rate / ABILITY_RATE_VALUE_FLOAT);
			}
		}
		++iter;
	}
	if( SetAbil(pkUnit, rkItem, pkItemDef, kEnchantAbil) )
	{
		eFlag |= E_UF_RAIRITY;
	}
}

void PgItemUpgradeInfo::SetRareOption( CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, SEnchantInfo const& rkEnchantInfo )
{
	if( !pkItemDef ) return;
	SetType(E_UF_RAIRITY);
	if( IG_SEAL != ::GetItemGrade(rkItem) )//봉인은 추가 옵션 사용안함
	{
		int const iEquipPos = pkItemDef->EquipPos();
		int const iOptionGroup = pkItemDef->OptionGroup();

		CONT_ENCHANT_ABIL kEnchantAbil;//대미궁 아이템 옵션
		::GetAbilObject(rkEnchantInfo.RareOptType1(), iEquipPos,  rkEnchantInfo.RareOptLv1(), iOptionGroup, 0, kEnchantAbil);
		::GetAbilObject(rkEnchantInfo.RareOptType2(), iEquipPos,  rkEnchantInfo.RareOptLv2(), iOptionGroup, 1, kEnchantAbil);
		::GetAbilObject(rkEnchantInfo.RareOptType3(), iEquipPos,  rkEnchantInfo.RareOptLv3(), iOptionGroup, 2, kEnchantAbil);
		::GetAbilObject(rkEnchantInfo.RareOptType4(), iEquipPos,  rkEnchantInfo.RareOptLv4(), iOptionGroup, 3, kEnchantAbil);
		if( SetAbil(pkUnit, rkItem, pkItemDef, kEnchantAbil) )
		{
			eFlag |= E_UF_OPT;
		}
	}
}

void PgItemUpgradeInfo::SetPackageOption( CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, CONT_INT* pkContSetNo )
{
	if( !pkItemDef ) return;
	SetType(E_UF_SET);
	GET_DEF(CItemSetDefMgr, kItemSetDefMgr);
	int const iSetNo = kItemSetDefMgr.GetItemSetNo(rkItem.ItemNo());
	if( !iSetNo )
	{// 세트번호를 못찾았음.
		return;
	}

	if( pkContSetNo )
	{
		if( FindSetNo(pkContSetNo, iSetNo) )
		{// 이미 적용한 세트옵션임. 중복하지마셈.
			return;
		}
		else
		{
			pkContSetNo->push_back(iSetNo);
		}
	}
	
	const CItemSetDef *pkOrgSetDef = kItemSetDefMgr.GetDef(iSetNo);
	if( !pkOrgSetDef )
	{//Def에 업ㅂ다
		return;
	}
	//장착 중인 아이템을 가져 온다.
	CONT_HAVE_ITEM_DATA kContHaveItems;
	//if( bOrtherActor )
	//{
	//	g_kOtherViewInv.GetItems(IT_FIT, kContHaveItems);
	//	g_kOtherViewInv.GetItems(IT_FIT_CASH, kContHaveItems);
	//}
	//else
	//{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{// 플레이어를 못찾겠엉ㅋ
		return;
	}
	pkPlayer->GetInven()->GetItems(IT_FIT, kContHaveItems);
	pkPlayer->GetInven()->GetItems(IT_FIT_CASH, kContHaveItems);
	//}

	bool bCompleteSet = false;
	int const iEquipPiece = pkOrgSetDef->CheckNeedItem(kContHaveItems, g_kPilotMan.GetPlayerUnit(), bCompleteSet);

	SAbilIterator kItor;
	pkOrgSetDef->FirstAbil(&kItor);

	SEnchantInfo kForSetEnchantInfo;
	kForSetEnchantInfo.IsCurse(false);

	PgBase_Item kItemForSetText = rkItem;
	kItemForSetText.EnchantInfo(kForSetEnchantInfo);

	CONT_DEF_ITEM_SET const *pkDefItemSet = NULL;
	g_kTblDataMgr.GetContDef(pkDefItemSet);

	CONT_TBL_DEF_ITEM_OPTION_ABIL const *pkDefItemOptAbil = NULL;
	g_kTblDataMgr.GetContDef(pkDefItemOptAbil);

	//옵션 설명서는 굉장히 예외 처리된 것이므로 계산식이 바뀌면 툴팁도 같이 바꿔야한다..
	CONT_DEF_ITEM_SET::const_iterator def_set_itor = pkDefItemSet->find(iSetNo);
	if(pkDefItemSet->end() == def_set_itor)
	{//그런 세트 아이템 업ㅂ다.
		return;
	}

	{
		int const iIdx = 0; // 1개 어빌만
		CONT_TBL_DEF_ITEM_OPTION_ABIL::const_iterator def_opt_abil_itor = pkDefItemOptAbil->find(def_set_itor->second.aiAbilNo[iIdx]);
		if( pkDefItemOptAbil->end() != def_opt_abil_itor )
		{
			CONT_TBL_DEF_ITEM_OPTION_ABIL_PIECE::const_iterator pieceabil_iter = (*def_opt_abil_itor).second.kContPieceAbil.begin();
			while( (*def_opt_abil_itor).second.kContPieceAbil.end() != pieceabil_iter )
			{
				CONT_TBL_DEF_ITEM_OPTION_ABIL_PIECE::value_type const& rkPeiceAbil = (*pieceabil_iter);
				for(int j = 0; MAX_OPTION_ABIL_ARRAY > j; j++)
				{
					int const iTargetAbilNo = rkPeiceAbil.aiType[j];
					int const iTargetAbilValue = rkPeiceAbil.aiValue[j];
					if( 0 < iTargetAbilNo )
					{
						if( iEquipPiece >= rkPeiceAbil.iPieceCount )
						{
							if( AT_EFFECTNUM1 == iTargetAbilNo )
							{// 
								CONT_DEFEFFECTABIL const *pkDefEffectAbil = NULL;
								g_kTblDataMgr.GetContDef(pkDefEffectAbil);
								if( pkDefEffectAbil )
								{
									CONT_DEFEFFECTABIL::const_iterator iter = pkDefEffectAbil->find(iTargetAbilValue);
									if( pkDefEffectAbil->end() != iter )
									{
										for(int i=0; i<MAX_EFFECT_ABIL_ARRAY; i++)
										{
											int iType = iter->second.iType[i];
											int iValue = iter->second.iValue[i];

											if( 0 != iType )
											{
												SetAbil(pkUnit, kItemForSetText, pkItemDef, iType, iValue);
											}
										}
									}
								}
							}

							//kItemForSetText 요거 베이스 아이템임. 네이밍이 뭐 이럼...-_-
							if( SetAbil(pkUnit, kItemForSetText, pkItemDef, iTargetAbilNo, iTargetAbilValue) )
							{
								eFlag |= E_UF_SET;
							}
						}
					}
				}

				++pieceabil_iter;
			}
		}
	}
}

bool PgItemUpgradeInfo::SetAbil(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, CAbilObject const &rkPlusAbil)
{
	if( rkPlusAbil.IsEmpty() )
	{
		return false;
	}
	int iResult = 0;
	SAbilIterator kItor;
	rkPlusAbil.FirstAbil(&kItor);
	while( rkPlusAbil.NextAbil(&kItor) )
	{
		WORD	wBaseAbilType = ::GetBasicAbil(kItor.wType);
		int		iValue = 0;

		iResult += SetAbil(pkUnit, rkItem, pkItemDef, wBaseAbilType, kItor.wType, kItor.iValue);
	}
	if( iResult )
	{
		return true;
	}
	return false;
}

bool PgItemUpgradeInfo::SetAbil(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, CONT_ENCHANT_ABIL const &rkEnchantAbil)
{
	if( rkEnchantAbil.empty() )
	{
		return false;
	}

	int iResult = 0;
	CONT_ENCHANT_ABIL::const_iterator kItor = rkEnchantAbil.begin();
	while( rkEnchantAbil.end() != kItor )
	{
		WORD	wBaseAbilType = ::GetBasicAbil(kItor->wType);
		int		iValue = 0;

		iResult += SetAbil(pkUnit, rkItem, pkItemDef, wBaseAbilType, kItor->wType, kItor->iValue);
		++kItor;
	}

	if( iResult )
	{
		return true;
	}
	return false;
}

bool PgItemUpgradeInfo::SetAbil( CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, WORD const wAbilType, int iAbilValue )
{
	WORD wBaseAbilType = ::GetBasicAbil(wAbilType);
	int		iValue = 0;
	
	return SetAbil(pkUnit, rkItem, pkItemDef, wBaseAbilType, wAbilType, iAbilValue);
}

int PgItemUpgradeInfo::CalcAbil(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, WORD const wBaseAbilType, WORD const wAbilType, int iAbilValue)
{
	int iValue = 0;

	switch( wAbilType )
	{
	case AT_PHY_ATTACK_MIN:
	case AT_MAGIC_ATTACK_MIN:
	case AT_PHY_ATTACK_MAX:
	case AT_MAGIC_ATTACK_MAX:
	case AT_PHY_DEFENCE:
	case AT_MAGIC_DEFENCE:
		{
			if(E_UF_BASE==GetType())
			{
				WORD wType = wAbilType;
				iValue = pkItemDef->ImproveAbil(wType, rkItem);
			}
			else
			{
				iValue = iAbilValue;
			}
		}break;
	case AT_I_MAGIC_ATTACK_ADD_RATE:
	case AT_I_PHY_ATTACK_ADD_RATE:
	case AT_I_PHY_DEFENCE_ADD_RATE:
	case AT_I_MAGIC_DEFENCE_ADD_RATE:
	case AT_R_PHY_DEFENCE:
	case AT_R_MAGIC_DEFENCE:
	case AT_R_PHY_ATTACK_MIN:
	case AT_R_PHY_ATTACK_MAX:
	case AT_R_MAGIC_ATTACK_MIN:
	case AT_R_MAGIC_ATTACK_MAX:
		{//아이템 어빌에 만분율로 계산
			iValue = (int)(pkItemDef->ImproveAbil(static_cast<EAbilType>(wBaseAbilType), rkItem) * (iAbilValue / ABILITY_RATE_VALUE_FLOAT));
		}break;
	case AT_R_CRITICAL_POWER:
		{}break;//해당 어빌은 사용하지 않는다.
	case AT_R_STR:		
	case AT_R_INT:		
	case AT_R_CON:	
	case AT_R_DEX:	
	case AT_R_MAX_HP:
	case AT_R_MAX_MP:
	case AT_R_HP_RECOVERY:	
	case AT_R_MP_RECOVERY:
		{//캐릭터 어빌에 만분율로 계산되는 어빌임
			if( !pkUnit )
			{
				break;
			}
			iValue = (int)( pkUnit->GetAbil(wBaseAbilType) * (iAbilValue / ABILITY_RATE_VALUE_FLOAT));
		}break;
	case AT_R_DODGE_SUCCESS_VALUE:
	case AT_R_HIT_SUCCESS_VALUE:
	//case AT_DODGE_SUCCESS_VALUE:
	//case AT_HIT_SUCCESS_VALUE:
		{
			iValue = pkUnit->GetAbil(wBaseAbilType)*(iAbilValue / ABILITY_RATE_VALUE_FLOAT);
		}break;
	default:
		{// 나머지 어빌은 절대값임
			iValue = iAbilValue;
		}break;
	}

	iValue = abs(iValue);
	return iValue;
}

void PgItemUpgradeInfo::SetAbil(WORD const wAbilType, int iAbilValue)
{
	switch( wAbilType )
	{
	case AT_I_PHY_DEFENCE_ADD_RATE:
	case AT_R_PHY_DEFENCE:
	case AT_PHY_DEFENCE:				
		{ 
			m_kDEF += iAbilValue;			
		}break;
	case AT_I_MAGIC_DEFENCE_ADD_RATE:
	case AT_R_MAGIC_DEFENCE:
	case AT_MAGIC_DEFENCE:			
		{ 
			m_kMDEF += iAbilValue;		
		}break;
	case AT_R_PHY_ATTACK_MIN:
	case AT_PHY_ATTACK_MIN:		
		{ 
			m_kMinATK += iAbilValue;		
		}break;
	case AT_R_PHY_ATTACK_MAX:
	case AT_PHY_ATTACK_MAX:		
		{ 
			m_kMaxATK += iAbilValue;		
		}break;
	case AT_R_MAGIC_ATTACK_MIN:
	case AT_MAGIC_ATTACK_MIN:	
		{ 
			m_kMinMATK += iAbilValue;	
		}break;
	case AT_R_MAGIC_ATTACK_MAX:
	case AT_MAGIC_ATTACK_MAX:	
		{ 
			m_kMaxMATK += iAbilValue;	
		}break;
	case AT_R_MAX_HP:
	case AT_MAX_HP:					
		{ 
			m_kHP += iAbilValue;			
		}break;
	case AT_R_MAX_MP:
	case AT_MAX_MP:					
		{ 
			m_kMP += iAbilValue;			
		}break;
	case AT_R_STR:
	case AT_STR:							
		{ 
			m_kSTR += iAbilValue;			
		}break;
	case AT_R_INT:
	case AT_INT:							
		{ 
			m_kINT += iAbilValue;			
		}break;
	case AT_R_CON:	
	case AT_CON:						
		{ 
			m_kCON += iAbilValue;			
		}break;
	case AT_R_DEX:	
	case AT_DEX:						
		{ 
			m_kDEX += iAbilValue;			
		}break;
	case AT_R_HP_RECOVERY:
	case AT_C_HP_RECOVERY:			
	case AT_HP_RECOVERY:
		{ 
			m_kHP_RECOVERY += iAbilValue;	
		}break;
	case AT_R_MP_RECOVERY:
	case AT_C_MP_RECOVERY:	
	case AT_MP_RECOVERY:
		{ 
			m_kMP_RECOVERY += iAbilValue;
		}break;
	case AT_R_DODGE_SUCCESS_VALUE:
	case AT_C_DODGE_SUCCESS_VALUE:			
	case AT_DODGE_SUCCESS_VALUE:			
		{ 
			m_kDODGE_RATE += iAbilValue;	
		}break;
	case AT_R_HIT_SUCCESS_VALUE:
	case AT_C_HIT_SUCCESS_VALUE:				
	case AT_HIT_SUCCESS_VALUE:			
		{ 
			m_kHIT_RATE += iAbilValue;		
		}break;
	default: 
		{
			m_kEtcAbils.AddAbil(wAbilType, iAbilValue);
		}break;
	}
}

bool PgItemUpgradeInfo::SetAbil(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, WORD const wBaseAbilType, WORD const wAbilType, int iAbilValue )
{
	int iValue = CalcAbil(pkUnit, rkItem, pkItemDef, wBaseAbilType, wAbilType, iAbilValue);

	if( !iValue )
	{
		return false;
	}

	SetAbil(wBaseAbilType, iValue);
	return true;
}

void PgItemUpgradeInfo::SetBaseInfo( CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef )
{
	if( !pkItemDef ) return;
	SetType(E_UF_BASE);
	SAbilIterator kAbil_itor;
	pkItemDef->FirstAbil(&kAbil_itor);
	while(pkItemDef->NextAbil(&kAbil_itor))
	{
		WORD wAbilType =  kAbil_itor.wType;
		WORD wBaseAbilType = ::GetBasicAbil(wAbilType);
		int	iAbilValue = kAbil_itor.iValue;
		SetAbil( pkUnit, rkItem, pkItemDef, wBaseAbilType, wAbilType, iAbilValue );
	}
}

bool PgItemUpgradeInfo::MakeToolTipText( std::wstring &rkText, CItemDef const* pkItemDef , bool bIsPhyInfo, bool bShowType )
{
	BM::vstring vstrTemp;

	if( IsEmpty() )
	{
		return false;
	}

	if( bShowType )
	{
		vstrTemp += GetTypeName();
		vstrTemp += L":  ";
	}

	if( EQUIP_LIMIT_WEAPON == (EQUIP_LIMIT_WEAPON & pkItemDef->GetAbil(AT_EQUIP_LIMIT)) || true==pkItemDef->IsPetItem() )
	{
		if( bIsPhyInfo )
		{
			if( !MinATK() && !MaxATK()) return false;
			vstrTemp += MinATK();
			vstrTemp += AREA_MARK_STRING;
			vstrTemp += MaxATK();
		}
		else
		{
			if( !MinMATK() && !MaxMATK()) return false;
			vstrTemp += MinMATK();
			vstrTemp += AREA_MARK_STRING;
			vstrTemp += MaxMATK();
		}
	}
	else
	{
		if( bIsPhyInfo )
		{
			if( !DEF() ) return false;
			vstrTemp += DEF();
		}
		else
		{
			if( !MDEF() ) return false;
			vstrTemp += MDEF();
		}
	}
	if( bShowType )
	{
		vstrTemp += ENTER_STRING;
	}

	rkText += (vstrTemp.operator const std::wstring &());

	return true;
}

void PgItemUpgradeInfo::MakeToolTipText_Total(std::wstring& rkText, CItemDef const* pkItemDef, bool bIsPhyInfo)
{
	BM::vstring kStrTotalInfo;
	if( eFlag != E_UF_BASE )
	{// 업그레이 되었다면 녹색~
		kStrTotalInfo += L"{C=";
		kStrTotalInfo += TTW(4101);
		kStrTotalInfo += L"/}";
	}

	bool bPetItem = false;
	if(pkItemDef->IsPetItem())
	{
		bPetItem = (pkItemDef->GetAbil(AT_PHY_ATTACK_MAX) || pkItemDef->GetAbil(AT_PHY_ATTACK_MIN) || 
			pkItemDef->GetAbil(AT_MAGIC_ATTACK_MAX) || pkItemDef->GetAbil(AT_MAGIC_ATTACK_MIN));
	}

	if( EQUIP_LIMIT_WEAPON == (EQUIP_LIMIT_WEAPON & pkItemDef->GetAbil(AT_EQUIP_LIMIT)) || bPetItem )
	{
		if( bIsPhyInfo )
		{
			kStrTotalInfo += ENTER_STRING;
			kStrTotalInfo += TTW(5050);//종합
			kStrTotalInfo += SPACE_STRING;
			kStrTotalInfo += TTW(5003);//물리 공격력
			kStrTotalInfo += L":  ";
			kStrTotalInfo += MinATK();
			kStrTotalInfo += AREA_MARK_STRING;
			kStrTotalInfo += MaxATK();
		}
		else
		{
			kStrTotalInfo += TTW(5050);//종합
			kStrTotalInfo += SPACE_STRING;
			kStrTotalInfo += TTW(5004);//마법 공격력	
			kStrTotalInfo += L":  ";
			kStrTotalInfo += MinMATK();
			kStrTotalInfo += AREA_MARK_STRING;
			kStrTotalInfo += MaxMATK();
		}
	}
	else
	{
		if( bIsPhyInfo )
		{
			kStrTotalInfo += ENTER_STRING;
			kStrTotalInfo += TTW(5050);//종합
			kStrTotalInfo += SPACE_STRING;
			kStrTotalInfo += TTW(5041);//물리 방어력
			kStrTotalInfo += L":  ";
			kStrTotalInfo += DEF();
		}
		else
		{
			kStrTotalInfo += TTW(5050);//종합
			kStrTotalInfo += SPACE_STRING;
			kStrTotalInfo += TTW(5042);//마법 방어력
			kStrTotalInfo += L":  ";
			kStrTotalInfo += MDEF();
		}
	}
	kStrTotalInfo += TTW(CONTENTS_COLOR_WHITE);
	rkText += (kStrTotalInfo.operator const std::wstring &());
}

void PgItemUpgradeInfo::MakeToolTipText_Diff( std::wstring &rkText, CItemDef const* pkItemDef, bool bIsPhyInfo, PgItemUpgradeInfo const &rkItemInfo )
{
	int iValue = 0, iCompVal = 0;
	if( EQUIP_LIMIT_WEAPON == (EQUIP_LIMIT_WEAPON & pkItemDef->GetAbil(AT_EQUIP_LIMIT)) )
	{
		if( bIsPhyInfo )
		{
			iValue = MaxATK();
			iCompVal = rkItemInfo.MaxATK();
		}
		else
		{
			iValue = MaxMATK();
			iCompVal = rkItemInfo.MaxMATK();
		}
	}
	else
	{
		if( bIsPhyInfo )
		{
			iValue = DEF();
			iCompVal = rkItemInfo.DEF();
		}
		else
		{
			iValue = MDEF();
			iCompVal = rkItemInfo.MDEF();
		}
	}
	wchar_t szCompRst[30] = {0,};
	if(iCompVal < iValue)
	{
		rkText += TTW(89999);
		swprintf(szCompRst, 29, L" (%s %d)", TTW(5000).c_str(), iValue - iCompVal);
		rkText += szCompRst;
		rkText += TTW(40011);
	}
	else if(iValue < iCompVal)
	{
		rkText += TTW(89998);
		swprintf(szCompRst, 29, L" (%s %d)", TTW(5001).c_str(), iCompVal - iValue);
		rkText += szCompRst;
		rkText += TTW(40011);
	}
	else
	{
		swprintf(szCompRst, 29, L" (%s)", TTW(5002).c_str());
		rkText += szCompRst;
	}
}

bool PgItemUpgradeInfo::MakeToolTipText_Status( std::wstring &rkText, WORD const wAbilType)
{
	int iValue = 0;
	int iAddValue = 0;//4,5대 속성용 추가 어빌 값.

	switch(wAbilType)
	{
	case AT_MAX_HP:					{	iValue = HP();			}break;
	case AT_MAX_MP:					{	iValue = MP();			}break;
	case AT_C_STR:					{	iValue = STR();			}break;
	case AT_C_INT:					{	iValue = INT();			}break;
	case AT_C_CON:					{	iValue = CON();			}break;
	case AT_C_DEX:					{	iValue = DEX();			}break;
	case AT_C_PHY_ATTACK_MAX:		{	iValue = MaxATK();		}break;
	case AT_C_MAGIC_ATTACK_MAX:		{	iValue = MaxMATK();		}break;
	case AT_C_PHY_DEFENCE:			{	iValue = DEF();			}break;
	case AT_C_MAGIC_DEFENCE:		{	iValue = MDEF();		}break;
	case AT_C_HP_RECOVERY:			{	iValue = HP_RECOVERY();	}break;
	case AT_C_MP_RECOVERY:			{	iValue = MP_RECOVERY();	}break;
	case AT_C_DODGE_SUCCESS_VALUE:			{	iValue = DODGE_RATE();	}break;
	case AT_C_HIT_SUCCESS_VALUE:				{	iValue = HIT_RATE();	}break;
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
			if(E_UF_BASE==eFlag && NULL!=m_pUnit)	//E_UF_BASE는 0이기 때문에 bit연산 아님
			{
				iValue = m_pUnit->GetAbil(wAbilType+330);
			}

			m_kEtcAbils.SetAbil(wAbilType, iValue);
		}break;
	case AT_C_CRITICAL_POWER:
		{
			iValue = m_kEtcAbils.GetAbil(AT_C_CRITICAL_POWER);
			//iValue += m_kEtcAbils.GetAbil(AT_R_CRITICAL_POWER);
			iValue += m_kEtcAbils.GetAbil(AT_CRITICAL_POWER);
			m_kEtcAbils.SetAbil(wAbilType, iValue);
		}break;
	default: { return false; }break;
	}

	if (iValue==0)
	{
		return false;
	}

	std::wstring wstrTypeName = GetTypeName();
	wstrTypeName += SPACE_STRING;
	const TCHAR* pAbilTypeName = NULL;
	if( ::GetAbilName(wAbilType, pAbilTypeName) )
	{
		wstrTypeName += pAbilTypeName;
	}
	wstrTypeName += L":  ";

	BM::vstring vStatusText;

	int iUnitLevel = 1;
	if( NULL != m_pUnit )
	{
		iUnitLevel = m_pUnit->GetAbil(AT_LEVEL);
	}
	PgItemUpgradeInfo::MakeValueString(vStatusText, wAbilType, iValue, true, iUnitLevel);		
	
	vStatusText += ENTER_STRING;
	rkText += wstrTypeName;
	rkText += (vStatusText.operator const std::wstring &());
	return true;
}

void PgItemUpgradeInfo::MakeValueString(BM::vstring &rText, WORD const wAbilType,  __int64 value,
										bool usedecimal, int const iUnitLevel)
{
	float fAbilValueLimit = 0.0f;
	switch(wAbilType)
	{
	case AT_C_CRITICAL_POWER:
	case AT_C_FINAL_BLOCK_SUCCESS_RATE:
	case AT_C_DODGE_SUCCESS_VALUE:		// 회피수치
	case AT_C_CRITICAL_SUCCESS_VALUE:	// 최대 크리티컬 성공률
		{
			fAbilValueLimit = value;
		}break;
	case AT_C_BLOCK_DAMAGE_RATE:
		{// 블록 데미지
			fAbilValueLimit = (PgSkillHelpFunc::MAX_BLOCK_DAMAGE_RATE/ABILITY_RATE_VALUE_FLOAT)*100;
		}break;
	}

	switch(wAbilType)
	{
	case AT_NONE:
		{
			return;
		}break;
	case AT_C_CRITICAL_POWER:
	case AT_C_BLOCK_DAMAGE_RATE:
	case AT_C_FINAL_BLOCK_SUCCESS_RATE:
		{
			char sz[255] = {0,};
			value*=100;
			float convert_value = ((value)/ABILITY_RATE_VALUE_FLOAT);
			if( fAbilValueLimit < convert_value )
			{
				convert_value = fAbilValueLimit;
				rText+= TTW(791580);	// 폰트 색
			}
			if(usedecimal)
			{
				sprintf_s(sz, 255, "%.2f%%", convert_value);
			}
			else
			{
				sprintf_s(sz, 255, "%d%%", static_cast<int>(convert_value));
			}
			rText+=sz;
		}break;
	case AT_C_DODGE_SUCCESS_VALUE:
	case AT_C_CRITICAL_SUCCESS_VALUE:
		{
			char sz[255] = {0,};
			int convert_value = value;
			if( fAbilValueLimit < convert_value )
			{
				convert_value = fAbilValueLimit;
				rText+= TTW(791580);	// 폰트 색
			}
			rText+=convert_value;
		}break;
	case AT_C_ATTACK_SPEED:
		{
			char sz[255] = {0,};
			value*=100;
			float convert_value = ((value)/ABILITY_RATE_VALUE_FLOAT);
			if(usedecimal)
			{
				sprintf_s(sz, 255, "%.2f%%", convert_value);
			}
			else
			{
				sprintf_s(sz, 255, "%d%%", static_cast<int>(convert_value));
			}
			rText+=sz;
		}break;
	default:
		{
			rText+=value;
		}break;
	}
}

std::wstring PgItemUpgradeInfo::GetTypeName()
{
	std::wstring wstrTypeName;	

	if(E_UF_BASE==eFlag)		{ wstrTypeName += TTW(5992);		}//기본 
	else if(E_UF_PLUS&eFlag)	{ wstrTypeName += TTW(1872);		}//인챈트
	else if(E_UF_RAIRITY&eFlag)	{ wstrTypeName += TTW(1434);		}//소울 크래프트
	else if(E_UF_SOCKET&eFlag)	{ wstrTypeName += TTW(790104);		}//몬스터카드
	else if(E_UF_CASH&eFlag)	{ wstrTypeName += TTW(5997);		}//캐시
	else if(E_UF_SET&eFlag)		{ wstrTypeName += TTW(1860);		}//세트
	else if(E_UF_OPT&eFlag)		{ wstrTypeName += TTW(70025);		}//기타
	else if(E_UF_EFFECT&eFlag)	{ wstrTypeName += TTW(5054);		}//버프
	else if(E_UF_MEDAL&eFlag)	{ wstrTypeName += TTW(3001);		}//업적
	else if(E_UF_ALL&eFlag)		{ wstrTypeName += TTW(5050);		}//종합

	return wstrTypeName;
}

bool PgItemUpgradeInfo::FindSetNo( CONT_INT const * const pkContSetNo, int const iSetNo ) const
{
	if( NULL == pkContSetNo || pkContSetNo->empty() )
	{
		return false;
	}

	CONT_INT::const_iterator itor = pkContSetNo->begin();
	while(pkContSetNo->end() != itor)
	{
		if( iSetNo == *itor )
		{
			return true;
		}

		++itor;
	}
	return false;
}

int PgItemUpgradeInfo::MakeToolTipText_All( CUnit const* const pkUnit, std::wstring &rkText, PgBase_Item const& rkItem, CItemDef const* pkItemDef, bool bIsPhyInfo )
{
	Clear();
	int iResult = 0;
	SEnchantInfo kEnchantInfo = rkItem.EnchantInfo();
	int iEnchantLv = kEnchantInfo.PlusLv();

	SetBaseInfo(pkUnit, rkItem, pkItemDef);
	MakeToolTipText(rkText, pkItemDef, bIsPhyInfo);

	Clear();
	SetPlusUpgrade(rkItem, pkItemDef, kEnchantInfo, iEnchantLv);
	iResult += MakeToolTipText(rkText, pkItemDef, bIsPhyInfo);

	Clear();
	SetRarityUpgrade(pkUnit, rkItem, pkItemDef, kEnchantInfo);
	iResult += MakeToolTipText(rkText, pkItemDef, bIsPhyInfo);

	Clear();
	SetSocketOption(pkUnit, rkItem, pkItemDef, kEnchantInfo);
	iResult += MakeToolTipText(rkText, pkItemDef, bIsPhyInfo);

	Clear();
	SetRareOption(pkUnit, rkItem, pkItemDef, kEnchantInfo);
	iResult += MakeToolTipText(rkText, pkItemDef, bIsPhyInfo);

	Clear();
	SetPackageOption(pkUnit, rkItem, pkItemDef);
	iResult += MakeToolTipText(rkText, pkItemDef, bIsPhyInfo);

	return iResult;
}

void PgItemUpgradeInfo::SetSocketOption(CUnit const* const pkUnit, PgBase_Item const& rkItem, CItemDef const* pkItemDef, SEnchantInfo const& rkEnchantInfo)
{
	if( !pkItemDef ) return;
	SetType(E_UF_SOCKET);
	if( PgItemSocketSystemMgr::SCE_OK != PgItemSocketSystemMgr::CheckSocketBundle( rkItem ) )
	{
		return;
	}

	for(int i=PgSocketFormula::SII_FIRST; i<=PgSocketFormula::SII_THIRD; ++i)
	{
		int iSocketState = 0;
		int iSocketCard = 0;

		switch( i )
		{
		case PgSocketFormula::SII_FIRST:
			{
				iSocketState = rkEnchantInfo.GenSocketState();
				iSocketCard = rkEnchantInfo.MonsterCard();
			}break;
		case PgSocketFormula::SII_SECOND:
			{
				iSocketState = rkEnchantInfo.GenSocketState2();
				iSocketCard = rkEnchantInfo.MonsterCard2();
			}break;
		case PgSocketFormula::SII_THIRD:
			{
				iSocketState = rkEnchantInfo.GenSocketState3();
				iSocketCard = rkEnchantInfo.MonsterCard3();
			}break;
		default:
			{
			}break;
		}

		if( GSS_GEN_SUCCESS != iSocketState )
		{//소켓이 없음
			continue;
		}

		if( !iSocketCard )
		{//몬스터카드가 없음
			continue;
		}

		//기간제 몬스터 카드 처리 안함.
		//기간 초과 / 기존 기간제 아이템 등에 대한 결정이 필요함.
		//위 부분이 정해지고 기간제 몬스터 카드 아이템이 추가되면 아래 주석 풀고 작업해야함.

		//SMonsterCardTimeLimit kCardTimeLimit;
		//if( false == rkItem.Get(kCardTimeLimit) || true == kCardTimeLimit.IsUseTimeOut() )
		//{//기한이 지났음
		//	return;
		//}

		PgBase_Item kCardItem;
		int const iCardItemNo = GetMonsterCardItemNo( i, iSocketCard );
		kCardItem.ItemNo( iCardItemNo );
		GET_DEF(CItemDefMgr, kItemDefMgr);
		int iAbilNum = kItemDefMgr.GetAbil(iCardItemNo, AT_EFFECTNUM1);

		CONT_DEFEFFECTABIL const *pkDefEffectAbil = NULL;
		g_kTblDataMgr.GetContDef(pkDefEffectAbil);
		if( !pkDefEffectAbil )
		{
			continue;
		}

		CONT_DEFEFFECTABIL::const_iterator iter = pkDefEffectAbil->find(iAbilNum);
		if( pkDefEffectAbil->end() == iter )
		{
			continue;
		}

		for(int i=0; i<MAX_EFFECT_ABIL_ARRAY; i++)
		{
			int iType = iter->second.iType[i];
			int iValue = iter->second.iValue[i];
			SetAbil(pkUnit, rkItem, pkItemDef,  iType, iValue);
		}

		eFlag |= E_UF_SOCKET;
	}
}

int PgItemUpgradeInfo::GetAbil( WORD const wAbilType )
{
	switch(wAbilType)
	{
	case AT_MAX_HP:					{	 return HP();			}break;
	case AT_MAX_MP:					{	 return MP();			}break;
	case AT_C_STR:					{	 return STR();			}break;
	case AT_C_INT:					{	 return INT();			}break;
	case AT_C_CON:					{	 return CON();			}break;
	case AT_C_DEX:					{	 return DEX();			}break;
	case AT_C_PHY_ATTACK_MAX:		{	 return MaxATK();		}break;
	case AT_C_MAGIC_ATTACK_MAX:		{	 return MaxMATK();		}break;
	case AT_C_PHY_DEFENCE:			{	 return DEF();			}break;
	case AT_C_MAGIC_DEFENCE:		{	 return MDEF();			}break;
	case AT_C_HP_RECOVERY:			{	 return HP_RECOVERY();	}break;
	case AT_C_MP_RECOVERY:			{	 return MP_RECOVERY();	}break;
	case AT_C_DODGE_SUCCESS_VALUE:			{	 return DODGE_RATE();	}break;
	case AT_C_HIT_SUCCESS_VALUE:				{	 return HIT_RATE();		}break;
	case AT_C_ATTACK_SPEED:
	{
		return m_kEtcAbils.GetAbil(AT_ATTACK_SPEED);

	}break;
	default:
		{
			return m_kEtcAbils.GetAbil(wAbilType);
		}
	}

	return 0;
}

float PgItemUpgradeInfo::CalculateAbil(CUnit const *pUnit, const WORD abil, 
									   EGetAbilType::Enum const getabiltype)
{
	float value = 0.0f;
	if(NULL==pUnit)
	{
		return value;
	}
	if (false==IsCalculatedAbil(abil))
	{
		return static_cast<float>(pUnit->GetAbil(abil));
	}

	WORD basic = GetBasicAbil(abil);	
	WORD rate = GetRateAbil(abil);
	float basicValue = pUnit->GetAbil(basic);

	if(EGetAbilType::EGAT_BASE&getabiltype)
	{
		value += basicValue + pUnit->GetEffectMgr().GetAbil(basic);
		value += basicValue * (pUnit->GetAbil(rate) + pUnit->GetEffectMgr().GetAbil(rate)) /
			ABILITY_RATE_VALUE_FLOAT;
	}
	if(EGetAbilType::EGAT_INV&getabiltype)
	{
		value += pUnit->GetInven()->GetAbil(basic);
		value += basicValue * pUnit->GetInven()->GetAbil(rate);
	}

	return value;
}

void MakeToolTipText_SoulTransition(PgBase_Item const &kItem, std::wstring &wstrTextOut, std::wstring &wstrType, TBL_SHOP_IN_GAME const &kShopItemInfo, SToolTipFlag const &kFlag)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( !pDef )
	{
		return;
	}
	//이름
	const wchar_t *pNameText = NULL;
	int const iGrade = GetItemGrade(kItem);;
	E_ITEM_GRADE const eItemLv = static_cast<E_ITEM_GRADE>(iGrade);
	if(GetDefString(pDef->NameNo(), pNameText))
	{// Name
		E_ITEM_GRADE const eGrade = GetItemGrade(kItem);;
		wstrTextOut += TTW(40102);
		SetGradeColor(eGrade, false, wstrTextOut);
		SetGradeColor(eGrade, false, wstrType);
		wstrTextOut += pNameText;
#ifndef USE_INB
		if(g_pkApp->VisibleClassNo())
		{
			BM::vstring kClassNo(kItem.ItemNo());
			wstrTextOut += (std::wstring)kClassNo;
		}
#endif
		wstrTextOut += TTW(40011);	//폰트 초기화
		wstrTextOut += ENTER_STRING;
		wstrTextOut += ENTER_STRING;
	}
	//영력
	{
		SetGradeColor(eItemLv, false, wstrTextOut);
		//wstrText += TTW(1306);
		wstrTextOut += TTW(261);
		wstrTextOut += WSTR_ATTR_EXPLAIN;
		wstrTextOut += TTW(40003);
		//아이템 등급
		std::wstring kRank(L"  /  ");
		kRank += TTW(262);
		kRank += WSTR_ATTR_EXPLAIN;
		kRank += TTW(50300 + eItemLv);//등급명
		BM::vstring strRarity((int)kItem.EnchantInfo().Rarity());
		wstrTextOut += (wchar_t const*)strRarity;//레어도 수치
		wstrTextOut += kRank;
		wstrTextOut += TTW(40011);	//폰트 초기화
		wstrTextOut += ENTER_STRING;
	}
	//가능레벨
	{
		int const iLevelLimit = pDef->GetAbil(AT_LEVELLIMIT);
		if( 0 != iLevelLimit)
		{
			BM::vstring kLevelLimit(TTW(799881));
			switch(g_kLocal.ServiceRegion())
			{
			case LOCAL_MGR::NC_JAPAN:
			case LOCAL_MGR::NC_TAIWAN:
				{//일본, 대만은 허용 레벨 범위가 [1Lv ~ 아이템레벨 + 10Lv]
					kLevelLimit.Replace(L"#MIN#", 1);
				}break;
			default:
				{//기본 허용 레벨 범위는 [아이템 레벨 ~ 아이템 레벨 +10]
					kLevelLimit.Replace(L"#MIN#", iLevelLimit);
				}break;
			}
			kLevelLimit.Replace( L"#MAX#", iLevelLimit+10);
			wstrTextOut += static_cast<std::wstring>(kLevelLimit);
			wstrTextOut += ENTER_STRING;
		}
	}
	//착용위치
	{
		BM::vstring kEquipText(TTW(799634));
		std::wstring wstrEquipPos;
		GetEqiupPosString( pDef->GetAbil(AT_EQUIP_LIMIT), false, wstrEquipPos);
		if( 0 != wstrEquipPos.size() )
		{
			kEquipText.Replace(L"#EQUIP#", wstrEquipPos);
		}
		wstrTextOut += static_cast<std::wstring>(kEquipText);
		wstrTextOut += ENTER_STRING;
	}
	//-저장옵션-
	{
		SEnchantInfo kEnchantInfo = kItem.EnchantInfo();
		//아이템 생성시 정해지는 랜덤 옵션
		if( kEnchantInfo.Rarity() )
		{
			wstrTextOut += TTW(799880);
			wstrTextOut += TTW(40021);
			wstrTextOut += ENTER_STRING;
			SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();
			if( kEnchantInfo.BasicType1() )
			{
				MakeAbilStringOnlyType(kItem, pDef, kEnchantInfo.BasicType1(), kEnchantInfo.BasicLv1(), wstrTextOut);
			}
			if( kEnchantInfo.BasicType2() )
			{
				MakeAbilStringOnlyType(kItem, pDef, kEnchantInfo.BasicType2(), kEnchantInfo.BasicLv2(), wstrTextOut);
			}
			if( kEnchantInfo.BasicType3() )
			{
				MakeAbilStringOnlyType(kItem, pDef, kEnchantInfo.BasicType3(), kEnchantInfo.BasicLv3(), wstrTextOut);
			}
			if( kEnchantInfo.BasicType4() )
			{
				MakeAbilStringOnlyType(kItem, pDef, kEnchantInfo.BasicType4(), kEnchantInfo.BasicLv4(), wstrTextOut);
			}
		}
		wstrTextOut += ENTER_STRING;
	}
	//다음 행동이 제한됨	
	int	const iCantAbil = pDef->GetAbil(AT_ATTRIBUTE);
	if(iCantAbil)
	{
		wstrTextOut += TTW(TITLE_COLOR_YELLOW);
		wstrTextOut += TTW(2501);
		wstrTextOut += TTW(40011);	//폰트 초기화
		wstrTextOut += ENTER_STRING;
		std::wstring	wstrCantAbil;
		GetCantAbilString(iCantAbil,wstrCantAbil);
		if( 0 != wstrCantAbil.size() )
		{//제한 어빌이 있다면 "/" 추가
			wstrTextOut += wstrCantAbil;
			wstrTextOut += ENTER_STRING;
		}
		wstrTextOut += ENTER_STRING;
	}
	//설명
	const TCHAR *pExplainText = NULL;
	int Value = pDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pExplainText))
	{
		wstrTextOut += ENTER_STRING;
		wstrTextOut += TTW(40050);
		wstrTextOut += TTW(40051);
		wstrTextOut += pExplainText;
		wstrTextOut += _T("\n");
	}
	//가격
	if(!kShopItemInfo.IsNull())
	{
		std::wstring kPrice;

		if( kShopItemInfo.iPrice )
		{
			//FormatMoney(kShopItemInfo.iPrice, kPrice);
			kPrice = GetMoneyString(kShopItemInfo.iPrice);
		}
		else
		{
			wchar_t	szCPValue[30] = {0,};
			swprintf(szCPValue, 29, TTW(5006).c_str(), kShopItemInfo.iCP * 10);
			kPrice = szCPValue;

		}

		wstrTextOut += _T("\n");
		wstrTextOut += TTW(40052);
		wstrTextOut += TTW(40053);	
		wstrTextOut += kPrice;
		wstrTextOut += _T("\n");//Pg2DString에서 높이를 계산하는 공식이 마지막줄의 폰트 크기와 줄바꿈 횟수를 곱해서 문제가 발생함 그걸 위한 임시 방편.
	}
	else
	{
		bool const bIsStockShopOn = (!lwGetUIWnd("FRM_SHOP_STOCK").IsNil());
		if( !lwGetUIWnd("FRM_SHOP").IsNil() 
			||	!lwGetUIWnd("FRM_SHOP_CP").IsNil()
			||	!lwGetUIWnd("FRM_SHOP_JOB").IsNil()
			||	bIsStockShopOn)
		{
			int iSellPrice = pDef->SellPrice();

			if(bIsStockShopOn)
			{
				PgStore::CONT_GODDS::value_type kItemInfo;
				if(S_OK == g_kViewStore.GetGoods(kItem.ItemNo(), kItemInfo))
				{
					iSellPrice = kItemInfo.iSellPrice;
				}
			}

			if(iSellPrice)
			{
				__int64 iEarnPrice = 0;
				CalcSellMoney(kItem, iEarnPrice, iSellPrice, kItem.Count(), g_kPilotMan.GetPlayerUnit()->GetPremium());

				std::wstring kPrice;
				//FormatMoney(iEarnPrice, kPrice);
				kPrice = GetMoneyString(iEarnPrice);

				wstrTextOut += _T("\n");
				wstrTextOut += TTW(40052);
				wstrTextOut += TTW(40053);
				wstrTextOut += kPrice;
				wstrTextOut += _T("\n");
			}
			else
			{
				wstrTextOut += _T("\n");
				wstrTextOut += TTW(40105);
				wstrTextOut += TTW(5014);
				wstrTextOut += _T("\n");
			}
		}
	}
}

void MakeAbilStringOnlyType(PgBase_Item const &kItem, CItemDef const* pkItemDef, WORD const wType, int iValue, std::wstring &wstrText)
{
	CONT_TBL_DEF_ITEM_OPTION const* pkDefItemOption = NULL;
	g_kTblDataMgr.GetContDef(pkDefItemOption);
	if(!pkDefItemOption || NULL==pkItemDef)
	{
		return;
	}
	CONT_TBL_DEF_ITEM_OPTION::const_iterator find_iter = pkDefItemOption->begin();
	int const pos = pkItemDef->GetAbil(AT_EQUIP_LIMIT);
	while( find_iter != pkDefItemOption->end() )
	{
		CONT_TBL_DEF_ITEM_OPTION::value_type const &rkOption = (*find_iter);
		if( wType == rkOption.iOptionType )
		{
			if(rkOption.iAbleEquipPos & pos)
			{
				if(pkItemDef->OptionGroup() == rkOption.iOptionGroup)
				{
					MakeAbilString(kItem, pkItemDef, rkOption.iNameNo, rkOption.aiValue[iValue-1], wstrText);
					return;
				}
			}
		}
		++find_iter;
	}
}

void lwCallPremiumToolTip(lwPoint2 &pt)
{
	GET_DEF(PgDefPremiumMgr, kDefPremium);
	SPremiumData const* pkDefPremium = kDefPremium.GetDef(g_kPilotMan.MemberPremium().iServiceNo);
	if(!pkDefPremium)
	{
		return;
	}

	BM::vstring wstrTextOut;
	//타이틀
	wstrTextOut += TTW(64008);//Color
	wstrTextOut += pkDefPremium->kTitle;
	wstrTextOut += ENTER_STRING;
	wstrTextOut += ENTER_STRING;

	//남은시간
	BM::DBTIMESTAMP_EX kNowDate;
	g_kEventView.GetLocalTime(kNowDate);
	kNowDate.second = 0;

	BM::DBTIMESTAMP_EX kEndDate = g_kPilotMan.MemberPremium().kEndDate;
	kEndDate.second = 0;
	
	__int64 i64NowTime = 0;
	__int64 i64EndTime = 0;
	CGameTime::DBTimeEx2SecTime(kNowDate, i64NowTime);
	CGameTime::DBTimeEx2SecTime(kEndDate, i64EndTime);
	
	std::wstring kTimeText;
	TimeToString(i64EndTime-i64NowTime, kTimeText);

	wstrTextOut += TTW(64009);
	wstrTextOut.Replace(L"#TIME#", kTimeText.empty() ? TTW(64006) : kTimeText);
	wstrTextOut += ENTER_STRING;
	wstrTextOut += ENTER_STRING;

	//효과
	wstrTextOut += TTW(64010);
	wstrTextOut += ENTER_STRING;

	PgPremiumMgr kPremium;
	kPremium.SetService(g_kPilotMan.MemberPremium().iServiceNo, BM::Stream());

	{//경험치 획득량 증가율
		S_PST_AddExperience const* pkArticle = kPremium.GetType<S_PST_AddExperience>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64011) );
			kArticleText.Replace(L"#VALUE#", BM::vstring(pkArticle->iRate/100.0f, L"%.1f"));
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//인챈트 성공률 증가율
		S_PST_AddEnchant const* pkArticle = kPremium.GetType<S_PST_AddEnchant>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64012) );
			kArticleText.Replace(L"#VALUE#", BM::vstring(pkArticle->iRate/100.0f, L"%.1f"));
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//소울크래프트 성공률 증가율
		S_PST_SoulCraft const* pkArticle = kPremium.GetType<S_PST_SoulCraft>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64013) );
			kArticleText.Replace(L"#VALUE#", BM::vstring(pkArticle->iRate/100.0f, L"%.1f"));
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//워프타워 이용료 할인율
		S_PST_TranstowerDiscount const* pkArticle = kPremium.GetType<S_PST_TranstowerDiscount>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64014) );
			kArticleText.Replace(L"#VALUE#", BM::vstring(pkArticle->iDiscount/100.0f, L"%.1f"));
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//NPC에게 아이템 팔 때 웃된 획득율
		S_PST_StoreSellPremium const* pkArticle = kPremium.GetType<S_PST_StoreSellPremium>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64015) );
			kArticleText.Replace(L"#VALUE#", BM::vstring(pkArticle->iPremiumRate/100.0f, L"%.1f"));
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//계정 금고 확장
		S_PST_ShareRentalSafeExpand const* pkArticle = kPremium.GetType<S_PST_ShareRentalSafeExpand>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64016) );
			kArticleText.Replace(L"#VALUE#", pkArticle->byExpand);
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//헌터 퀘스트 & 길드 퀘스트 끝까지 완료 후 한번 더 진행 가능
		S_PST_QuestOnceMore const* pkArticle = kPremium.GetType<S_PST_QuestOnceMore>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64017) );
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//직업생산 1차 : 피로도 증가값
		S_PST_JS1_MaxExhaustion const* pkArticle = kPremium.GetType<S_PST_JS1_MaxExhaustion>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64018) );
			kArticleText.Replace(L"#VALUE#", pkArticle->iMax);
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//직업생산 1차 : 피로도 회복량 증가
		S_PST_JS1_RecoveryExhaustion const* pkArticle = kPremium.GetType<S_PST_JS1_RecoveryExhaustion>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64019) );
			kArticleText.Replace(L"#VALUE#", BM::vstring(pkArticle->iRate/100.0f, L"%.1f"));
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//직업생산 1차 : 채집 숙련도 획득량 증가율
		S_PST_JS1_AddExpertness const* pkArticle = kPremium.GetType<S_PST_JS1_AddExpertness>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64020) );
			kArticleText.Replace(L"#VALUE#", BM::vstring(pkArticle->iRate/100.0f, L"%.1f"));
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//직업생산 2차 : 최대 축복 게이지 증가값
		S_PST_JS2_MaxBless const* pkArticle = kPremium.GetType<S_PST_JS2_MaxBless>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64021) );
			kArticleText.Replace(L"#VALUE#", pkArticle->iMax);
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//아이템 분해시 획득 소울 양 증가율
		S_PST_SoulRefund const* pkArticle = kPremium.GetType<S_PST_SoulRefund>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64022) );
			kArticleText.Replace(L"#VALUE#", BM::vstring(pkArticle->iRate/100.0f, L"%.1f"));
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//오픈마켓에 상점 등록시 최고급 상점으로 등록됨(거상 포인트는 따로 구매해야 함)
		S_PST_OpenmarketState const* pkArticle = kPremium.GetType<S_PST_OpenmarketState>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64023) );
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//퀘스트 원격 수락
		S_PST_QuestRemoteAccept const* pkArticle = kPremium.GetType<S_PST_QuestRemoteAccept>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64024) );
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//채팅창에 붙는 VIP 표시
		S_PST_VIP_ChatDisplay const* pkArticle = kPremium.GetType<S_PST_VIP_ChatDisplay>();
		if(pkArticle)
		{
			BM::vstring kArticleText( TTW(64026) );
			wstrTextOut += kArticleText;
			wstrTextOut += ENTER_STRING;
		}
	}
	{//이펙트적용
		CONT_DEFEFFECT const *pkDefEffect = NULL;
		g_kTblDataMgr.GetContDef(pkDefEffect);

		S_PST_ApplyEffect const* pkArticle = kPremium.GetType<S_PST_ApplyEffect>();
		if(pkDefEffect && pkArticle)
		{
			VEC_INT::const_iterator c_it = pkArticle->kContEffect.begin();
			while(c_it != pkArticle->kContEffect.end())
			{
				CONT_DEFEFFECT::const_iterator eff_it = pkDefEffect->find( *c_it );
				if( pkDefEffect->end() != eff_it )
				{
					const wchar_t *pName = NULL;
					if(GetDefString((*eff_it).second.iName, pName))
					{
						BM::vstring kArticleText( TTW(64027) );
						kArticleText.Replace(L"#TITLE#", pName);
						
						wstrTextOut += kArticleText;
						wstrTextOut += ENTER_STRING;
					}
				}
				++c_it;
			}
		}
	}
	//{//하루에 한번 아이템 지급
	//	S_PST_GiftItem const* pkArticle = kPremium.GetType<S_PST_GiftItem>();
	//	if(pkArticle)
	//	{
	//		BM::vstring kArticleText( TTW(64025) );
	//		kArticleText.Replace(L"#VALUE#", pkArticle->iItemNo);
	//		wstrTextOut += kArticleText;
	//		wstrTextOut += ENTER_STRING;
	//	}
	//}

	//
	lwCallMutableToolTipByText(static_cast<std::wstring>(wstrTextOut), pt, 0, "ToolTip_Premium", false);
}

bool MakeComboSkillToolTip(int iSkillNo, std::wstring &wstrTextOut, std::wstring& wstrType)
{	
	{//스킬타입 :[기본콤보]
		wstrType+=TTW(799972);
	}
	wstrTextOut.clear();
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	if(!pSkillDef)
	{
		return false;
	}
	//	현재 레벨 스킬의 정보를 보여준다.
	wchar_t const* pName = NULL;
	GetDefString(pSkillDef->NameNo(),pName);
	//	이름용 색상 / 폰트
	wstrTextOut += TTW(40102); //이름용
	//	이름
	if(pName)
	{	
		BM::vstring kName(L"#NAME#");
		kName.Replace(L"#NAME#", pName);
		wstrTextOut += static_cast<std::wstring>(kName);
	}
	else
	{
		wstrTextOut+=_T("Undefined Name");
	}

#ifndef USE_INB
	if( g_pkApp->VisibleClassNo() )
	{
		BM::vstring kSkillNo(pSkillDef->No());
		wstrTextOut += _T("\n SkillNo : ") + (std::wstring)kSkillNo;
	}
#endif
	//	공백
	wstrTextOut+=_T("\n\n");
/*	//	제한레벨
	PgPilot	*pkPlayerPilot = g_kPilotMan.GetPlayerPilot();
	if(pkPlayerPilot)
	{
		if(pkPlayerPilot->GetAbil(AT_LEVEL)<pSkillDef->m_sLevelLimit)	//	빨간색으로 표시하자
		{
			wstrTextOut+=TTW(225);
			wstrTextOut += TTW(40011);	//폰트 초기화
		}
	}
	if(0 < pSkillDef->m_sLevelLimit)
	{
		WCHAR	temp[100] = {0,};
		swprintf(temp,100,_T("%s : %d %s\n"),TTW(230).c_str(),pSkillDef->m_sLevelLimit,TTW(232).c_str());
		wstrTextOut+=temp;
		wstrTextOut += TTW(40011);	//폰트 초기화
	}
	//	제한직업
	std::wstring kLimitClass;
	if(MakeSkillLimitClassText(pSkillDef,kLimitClass))
	{
		WCHAR	temp[100] = {0,};
		swprintf(temp,100,_T("%s : %s\n"),TTW(229).c_str(),kLimitClass.c_str());
		wstrTextOut+=temp;
		wstrTextOut += TTW(40011);	//폰트 초기화
	} 
	//	제한무기
	const CONT_DEFSKILL *pkContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pkContDefMap);
	if (pkContDefMap)
	{
		CONT_DEFSKILL::const_iterator def_it = pkContDefMap->find(iSkillNo);
		if (pkContDefMap->end() != def_it)
		{
			CONT_DEFSKILL::mapped_type kT = (*def_it).second;
			if(kT.iWeaponLimit)
			{
				wstrTextOut+=TTW(40132);
				wstrTextOut+=_T(" : ");
				int iCount = 0;
				for (int i = 0; i< 12; ++i)
				{
					int const iWeaponType = 1<<i;
					if (kT.iWeaponLimit & iWeaponType)
					{ 
						if (iCount>0)
						{
							wstrTextOut+=_T(", ");
						}
						int iType = (int)(NiFastLog(iWeaponType) + 1);
						wstrTextOut+=TTW(2000+iType);
						++iCount;
					}
				}
				wstrTextOut+=_T("\n");
			}
		}
		wstrTextOut += TTW(40011);	//폰트 초기화
	}
*/
	//	일반 색상
	wstrTextOut += TTW(40011);	//폰트 초기화
	PgPilot *pPilot = g_kPilotMan.GetPlayerPilot();
	if( pPilot && pPilot->IsChargeCombo(iSkillNo) )
	{
		//차지공격
		wstrTextOut += TTW(799971);
		wstrTextOut += _T("\n");
		std::list<int> kContSkill;
		if( pPilot->GetChargeResultAction( iSkillNo, kContSkill) )
		{
			int iCharge = 1;
			std::list<int>::const_iterator itor_skill = kContSkill.begin();
			while( kContSkill.end() != itor_skill )
			{
				BM::vstring kChargeText(TTW(799973));
				kChargeText.Replace(L"#CHARGE#", iCharge);
				wstrTextOut += static_cast<std::wstring>( kChargeText );
				MakeComboDamageText( (*itor_skill), wstrTextOut);

				iCharge++;
				++itor_skill;
			}
		}
	}
	else
	{
		MakeComboDamageText(iSkillNo, wstrTextOut);
	}
	wstrTextOut += TTW(40011);	//폰트 초기화
	//	설명
	const TCHAR *pText = NULL;
	int Value = pSkillDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		if (strlenT(pText))
		{
			WCHAR	temp[100] = {0,};
			swprintf(temp,100,_T("%s : \n"),TTW(231).c_str());
			wstrTextOut+=temp;
			wstrTextOut+=pText;
		}
	}

	bool bCantBlockTxt = false;
	if( !bCantBlockTxt
		&& 0 < pSkillDef->GetAbil(AT_CAN_NOT_BLOCK_BY_ACTION) 
		)
	{
		bCantBlockTxt = true;
	}
	if( bCantBlockTxt )
	{// 블록 할 수 없습니다.
		wstrTextOut += ENTER_STRING; 
		wstrTextOut += ENTER_STRING;
		wstrTextOut += TTW(750010);	// 컬러(노랑), 폰트
		wstrTextOut += TTW(791611);
	}

	return	true;
}	

void MakeComboDamageText( int iSkillNo, std::wstring &wstrTextOut )
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	if(!pSkillDef)
	{
		return;
	}
	//데미지를 나눠서 공격하는 스킬은 한발 데미지로 나누어 주어야 한다.
	int iComboCount = pSkillDef->GetAbil(AT_COMBO_HIT_COUNT);
	//	물리대미지
	int iAtk = pSkillDef->GetAbil(AT_PHY_DMG_PER);
	if(0 < iComboCount)
	{
		iAtk /= iComboCount;
	}
	if( 0 < iAtk )
	{
		WCHAR	temp[500] = {0,};
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(40136).c_str(), ((float)(iAtk) * 0.01f));
		wstrTextOut+=temp;
		iAtk = pSkillDef->GetAbil(AT_PHY_ATTACK);
		if(0 < iComboCount)
		{
			iAtk /= iComboCount;
		}
		if( 0 < iAtk )
		{
			swprintf(temp, 499, _T(" + %d"), iAtk);
			wstrTextOut+=temp;
		}
		wstrTextOut+=L"\n";
	}

	//	마법대미지
	int iMAtk = pSkillDef->GetAbil(AT_MAGIC_DMG_PER);
	if(0 < iComboCount)
	{
		iMAtk /= iComboCount;
	}
	if( 0 < iMAtk )
	{
		WCHAR	temp[500] = {0,};
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(40137).c_str(), ((float)(iMAtk) * 0.01f));
		wstrTextOut+=temp;
		iMAtk = pSkillDef->GetAbil(AT_MAGIC_ATTACK);
		if(0 < iComboCount)
		{
			iMAtk /= iComboCount;
		}
		if( 0 < iMAtk )
		{
			swprintf(temp, 499, _T(" + %d"), iMAtk);
			wstrTextOut+=temp;
		}
		wstrTextOut+=L"\n";
	}
	wstrTextOut += TTW(40011);	//폰트 초기화
}

void UserCharacterIcon(int iIconNo, std::wstring & wstrText, char const* wndName)
{
	if(!wndName)
	{
		wndName = szDefaultToolTipID;
	}
	else
	{
		return;
	}

	CONT_DEFITEM const* pkDefItem = NULL;
	g_kTblDataMgr.GetContDef(pkDefItem);
	if( !pkDefItem )
	{
		return;
	}

	CONT_DEFITEM::const_iterator def_item = pkDefItem->find( iIconNo );
	if( pkDefItem->end() == def_item )
	{
		return;
	}

	int const iGenderLimit = def_item->second.byGender;
	if( 0 == iGenderLimit )
	{
		return;
	}

	std::wstring GenderText = TTW(5208);
	
	if(def_item->second.i64ClassLimit > 0i64)
	{
		switch(iGenderLimit)
		{
		case 1:
			{
				GenderText += L"#200";
				GenderText = g_kChatMgrClient.ConvertUserCommand((std::wstring const&)(GenderText),_T("EmoticonFont32x32"));
			}break;
		case 2:
			{
				GenderText += L"#201";
				GenderText = g_kChatMgrClient.ConvertUserCommand((std::wstring const&)(GenderText),_T("EmoticonFont32x32"));
			}break;
		case 3:
			{
				GenderText += L"#200";
				GenderText = g_kChatMgrClient.ConvertUserCommand((std::wstring const&)(GenderText),_T("EmoticonFont32x32"));
				GenderText += L"#201";
				GenderText = g_kChatMgrClient.ConvertUserCommand((std::wstring const&)(GenderText),_T("EmoticonFont32x32"));
			}break;
		}
	}

	if(def_item->second.i64DraClassLimit > 0i64)
	{
		switch(iGenderLimit)
		{
		case 1:
			{
				GenderText += L"#202";
				GenderText = g_kChatMgrClient.ConvertUserCommand((std::wstring const&)(GenderText),_T("EmoticonFont32x32"));
			}break;
		case 2:
			{
				GenderText += L"#500";
				GenderText = g_kChatMgrClient.ConvertUserCommand((std::wstring const&)(GenderText),_T("EmoticonFont32x32"));
			}break;
		case 3:
			{
				GenderText += L"#202";
				GenderText = g_kChatMgrClient.ConvertUserCommand((std::wstring const&)(GenderText),_T("EmoticonFont32x32"));
				GenderText += L"#500";
				GenderText = g_kChatMgrClient.ConvertUserCommand((std::wstring const&)(GenderText),_T("EmoticonFont32x32"));
			}break;
		}
	}
	wstrText += GenderText;
}

namespace TooltipUtil
{
	BM::vstring GetValueColor(float const fValue, int const iAbilType)
	{
		BM::vstring vStr;
		if(0.0f < fValue)
		{// 녹색
			vStr = ::TTW(1548);
		}
		else if(0.0f > fValue)
		{// 빨강
			vStr = ::TTW(89999);
		}
		else
		{// 흰색			
			vStr = ::TTW(1554);
		}
		return vStr;
	}
}