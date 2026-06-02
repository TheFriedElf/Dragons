#include "StdAfx.h"
#include "lwUI.h"
#include "lwUIItemPlusUpgrade.h"
#include "lwUIToolTip.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "Variant/PgPlayer.h"
#include "ServerLib.h"
#include "PgNetwork.h"

#include "variant/item.h"
#include "lohengrin/packettype.h"
#include "PgChatMgrClient.h"
#include "PgSoundMan.h"
#include "PgHelpSystem.h"
#include "Variant/PgItemRarityUpgradeFormula.h"
#include "lwUIQuest.h"
#include "lwHomeUI_Script.h"
#include "lwCashshop.h"

namespace lwMarketUtil
{
	extern std::wstring const FloatConvertToStr(float const fValue, int const iDecimalCount);
}

int const PLUS_ISURANCE_ITEM_NO_BASE = 98000040;	//보험 아이템.

std::wstring const PLUS_STR(_T("+"));

namespace PgUIItemPlusUtil
{
	std::wstring const kItemPlusUIName(_T("SFRM_ITEM_PLUS_UPGRADE"));

	void SetSuccessRateUI()
	{
		XUI::CXUI_Wnd* pWnd = XUIMgr.Get(kItemPlusUIName);
		if( !pWnd ){ return; }

		pWnd = pWnd->GetControl(L"SFRM_COLOR");
		if( !pWnd ){ return; }

		pWnd = pWnd->GetControl(L"SFRM_SHADOW");
		if( !pWnd ){ return; }

		XUI::CXUI_Wnd* pSelf = pWnd->GetControl(L"SFRM_SUC_RATE");
		if( pSelf )
		{
			SItemPos kProbPos = g_kItemPlusUpgradeMgr.GetProbabilityItemPos();
			if( SItemPos::NullData() == kProbPos )
			{
				XUI::CXUI_Wnd* Temp = pSelf->GetControl(L"FRM_NOW");
				if( Temp )
				{
					float fRate = 0.f;
					PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
					if( pkPlayer )
					{
						if( S_PST_AddEnchant const* pkPremiumAddEnchant = pkPlayer->GetPremium().GetType<S_PST_AddEnchant>() )
						{
							fRate = static_cast<float>(pkPremiumAddEnchant->iRate)/ABILITY_RATE_VALUE_FLOAT;
						}
					}

					BM::vstring vStr(fRate, L"%.1f");
					vStr += L"%";
					Temp->Text(vStr);
				}

				Temp = pSelf->GetControl(L"FRM_MAX");
				if( Temp )
				{
					BM::vstring vStr(0);

					int const iProbItemNo = g_kItemPlusUpgradeMgr.GetProbAbilityNo();
					if( iProbItemNo )
					{
						TBL_DEF_ITEMPLUSUPGRADE const* pDef = g_kItemPlusUpgradeMgr.GetPlusInfo();
						if( pDef )
						{
							int const iItemCount = pDef->aNeedItemCount[ENEEDIO_PROBABILITY];
							vStr = PgItemRarityUpgradeFormula::GetEnchantBonusRate(iItemCount);
						}
					}

					vStr += L"%";
					Temp->Text(vStr);
				}
			}
			else
			{
				int iProbItemCount = g_kItemPlusUpgradeMgr.GetUseProbabilityItemCount();

				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pkPlayer )
				{
					return;
				}

				PgInventory* pkInv = pkPlayer->GetInven();
				if( !pkInv )
				{
					return;
				}

				PgBase_Item kItem;
				if( S_OK != pkInv->GetItem(kProbPos, kItem) )
				{
					return;
				}

				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const* pDef = kItemDefMgr.GetDef(kItem.ItemNo());
				if( !pDef )
				{
					return;
				}

				XUI::CXUI_Wnd* Temp = pSelf->GetControl(L"FRM_NOW");
				if( Temp )
				{
					BM::vstring vStr(PgItemRarityUpgradeFormula::GetEnchantBonusRate(iProbItemCount));
					vStr += L"%";
					Temp->Text(vStr);
				}

				Temp = pSelf->GetControl(L"FRM_MAX");
				if( Temp )
				{
					TBL_DEF_ITEMPLUSUPGRADE const* pDef = g_kItemPlusUpgradeMgr.GetPlusInfo();
					if( pDef )
					{
						BM::vstring vStr;
						int const iItemCount = pDef->aNeedItemCount[ENEEDIO_PROBABILITY];
						vStr = PgItemRarityUpgradeFormula::GetEnchantBonusRate(iItemCount);
						vStr += L"%";
						Temp->Text(vStr);
					}
				}
			}
		}
	}

	void CallItemPlusUI(lwGUID kNpcGuid)
	{
		g_kItemPlusUpgradeMgr.NpcGuid( kNpcGuid() );
		CXUI_Wnd* pkTopWnd = XUIMgr.Call(kItemPlusUIName);
		if( pkTopWnd )
		{
			SetSuccessRateUI();
			RegistUIAction(pkTopWnd);
		}
	}

	void ClearPlusUI()
	{
		XUI::CXUI_Wnd* pWnd = XUIMgr.Get(kItemPlusUIName);
		if( !pWnd ){ return; }

		pWnd = pWnd->GetControl(_T("SFRM_COLOR"));
		if( !pWnd ){ return; }

		pWnd = pWnd->GetControl(_T("SFRM_SHADOW"));
		if( !pWnd ){ return; }

		for(int i = ENEEDIO_INSURANCE; i < ENEEDIO_END; ++i)
		{
			BM::vstring	vStr(L"FRM_MATERIAL");
			vStr += i;
			XUI::CXUI_Wnd* pTemp = pWnd->GetControl(vStr);
			if( pTemp )
			{
				XUI::CXUI_Wnd* pTemp2 = pTemp->GetControl(_T("BTN_REG"));
				if( pTemp2 ){ pTemp2->Visible(false); };
				pTemp2 = pTemp->GetControl(_T("BTN_DEREG"));
				if( pTemp2 ){ pTemp2->Visible(false); };
			}
		}

		SetSuccessRateUI();

		XUI::CXUI_Wnd *pkArrow = pWnd->GetControl(_T("FRM_DOWN_ARROW_BG"));
		if (pkArrow)
		{// 화살표 크기 초기화
			XUI::CXUI_Wnd *pkArrowImg = pkArrow->GetControl(_T("IMG_ARROW"));
			if (pkArrowImg)
			{
				POINT2 kImgSize = pkArrowImg->ImgSize();
				kImgSize.y = 0;
				pkArrowImg->ImgSize(kImgSize);
			}
		}
	}

	void UpdatePlusUI()
	{
		XUI::CXUI_Wnd* pWnd = XUIMgr.Get(kItemPlusUIName);
		if( !pWnd ){ return; }

		pWnd = pWnd->GetControl(_T("SFRM_COLOR"));
		if( !pWnd ){ return; }

		pWnd = pWnd->GetControl(_T("SFRM_SHADOW"));
		if( !pWnd ){ return; }

		for(int i = ENEEDIO_INSURANCE; i < ENEEDIO_END; ++i)
		{
			BM::vstring	vStr(L"FRM_MATERIAL");
			vStr += i;
			XUI::CXUI_Wnd* pTemp = pWnd->GetControl(vStr);
			if( pTemp )
			{
				XUI::CXUI_Wnd* pTemp2 = pTemp->GetControl(_T("BTN_REG"));
				if( pTemp2 ){ pTemp2->Visible(false); };
				pTemp2 = pTemp->GetControl(_T("BTN_DEREG"));
				if( pTemp2 ){ pTemp2->Visible(false); };
			}
		}
	}

	void InitMaterialBtnState()
	{
		XUI::CXUI_Wnd* pWnd = XUIMgr.Get(kItemPlusUIName);
		if( !pWnd ){ return; }

		pWnd = pWnd->GetControl(_T("SFRM_COLOR"));
		if( !pWnd ){ return; }

		pWnd = pWnd->GetControl(_T("SFRM_SHADOW"));
		if( !pWnd ){ return; }

		for(int i = ENEEDIO_INSURANCE; i < ENEEDIO_END; ++i)
		{
			BM::vstring	vStr(L"FRM_MATERIAL");
			vStr += i;
			XUI::CXUI_Wnd* pTemp = pWnd->GetControl(vStr);
			if( pTemp )
			{
				g_kItemPlusUpgradeMgr.SetSrcMaterialBtnInit(pTemp);
			}
		}		
	}

	bool ReSetUpdateDataToInsureItem(int const WeaponLv, DWORD const kItemNo)
	{
		PgPlayer*	pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		PgInventory*	pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return false;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pDef = kItemDefMgr.GetDef(kItemNo);
		if( pDef )
		{
			int const iMin = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			int const iMax = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);

			if( iMin <= WeaponLv && WeaponLv <= iMax )
			{
				return true;
			}
		}

		return false;
	}
}

lwUIItemPlusUpgrade::lwUIItemPlusUpgrade(lwUIWnd kWnd)
{
	self = kWnd.GetSelf();
}
bool lwUIItemPlusUpgrade::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "CallItemPlusUI", PgUIItemPlusUtil::CallItemPlusUI);

	class_<lwUIItemPlusUpgrade>(pkState, "ItemPlusUpgradeWnd")
		.def(pkState, constructor<lwUIWnd>())
		.def(pkState, "DisplaySrcIcon", &lwUIItemPlusUpgrade::DisplaySrcIcon)
		.def(pkState, "DisplayNeedItemIcon", &lwUIItemPlusUpgrade::DisplayNeedItemIcon)
		.def(pkState, "DisplayResultItemIcon", &lwUIItemPlusUpgrade::DisplayResultItem)
		.def(pkState, "SetMaterialItem", &lwUIItemPlusUpgrade::SetMaterialItem)
		.def(pkState, "SetProbabilityMinus", &lwUIItemPlusUpgrade::SetProbabilityMinus)
		.def(pkState, "ClearUpgradeData", &lwUIItemPlusUpgrade::ClearUpgradeData)
		.def(pkState, "ReSetUpgradeData", &lwUIItemPlusUpgrade::ReSetUpgradeData)
		.def(pkState, "SendReqPlusUpgrade", &lwUIItemPlusUpgrade::SendReqPlusUpgrade)
		.def(pkState, "GetUpgradeNeedMoney", &lwUIItemPlusUpgrade::GetUpgradeNeedMoney)
		.def(pkState, "CallComfirmMessageBox", &lwUIItemPlusUpgrade::CallComfirmMessageBox)
		.def(pkState, "CallCheckInsureItem", &lwUIItemPlusUpgrade::CallCheckInsureItem)
		.def(pkState, "Clear", &lwUIItemPlusUpgrade::Clear)
		.def(pkState, "CheckOK", &lwUIItemPlusUpgrade::CheckOK)
		.def(pkState, "GetNowNeedItemCount", &lwUIItemPlusUpgrade::GetNowNeedItemCount)
		;
	return true;
}

void lwUIItemPlusUpgrade::Clear(bool const bAllClear)
{
	g_kItemPlusUpgradeMgr.Clear(bAllClear);
	PgUIItemPlusUtil::UpdatePlusUI();
}

void lwUIItemPlusUpgrade::DisplaySrcIcon()
{
	g_kItemPlusUpgradeMgr.DisplaySrcItem(self);
}

void lwUIItemPlusUpgrade::DisplayResultItem()
{
	g_kItemPlusUpgradeMgr.DisplayResultItem(self);
}

void lwUIItemPlusUpgrade::SetMaterialItem(bool bNoBuyMsg)
{
	g_kItemPlusUpgradeMgr.SetMaterialItem(self, bNoBuyMsg);
	XUI::CXUI_Wnd* pParent = self->Parent();
	if( ENEEDIO_PROBABILITY == self->BuildIndex() && pParent )
	{
		PgUIItemPlusUtil::SetSuccessRateUI();
	}
}

void lwUIItemPlusUpgrade::SetProbabilityMinus()
{
	g_kItemPlusUpgradeMgr.SetProbAbilityMinus();
	XUI::CXUI_Wnd* pParent = self->Parent();
	if( pParent )
	{
		PgUIItemPlusUtil::SetSuccessRateUI();
	}
}

void lwUIItemPlusUpgrade::DisplayNeedItemIcon()
{
	CXUI_Wnd *pParent = self->Parent();//SFRM_MATERIAL
	if(pParent)
	{
		CXUI_Wnd *pGrandParent = pParent->Parent();//FRM_MATERIAL
		if(pGrandParent)
		{
			g_kItemPlusUpgradeMgr.DisplayNeedItemIcon( pGrandParent->BuildIndex(), self );
			return;
		}
	}
	assert(NULL && "lwUIItemPlusUpgrade::DisplayNeedItemIcon");
}

void lwUIItemPlusUpgrade::ClearUpgradeData()
{
	g_kItemPlusUpgradeMgr.Clear();
	PgUIItemPlusUtil::ClearPlusUI();
}

void lwUIItemPlusUpgrade::ReSetUpgradeData()
{
	SItemPos	kItemPos = g_kItemPlusUpgradeMgr.GetSrcItemPos();
	SItemPos	kInsurePos = g_kItemPlusUpgradeMgr.GetInsureItemPos();
	SItemPos	kProbPos = g_kItemPlusUpgradeMgr.GetProbabilityItemPos();
	int const iProbCount = g_kItemPlusUpgradeMgr.GetUseProbabilityItemCount();

	g_kItemPlusUpgradeMgr.Clear();
	PgUIItemPlusUtil::ClearPlusUI();

	SIconInfo	kInfo;
	kInfo.iIconGroup = kItemPos.x;
	kInfo.iIconKey = kItemPos.y;
	g_kItemPlusUpgradeMgr.SetItem(KUIG_ITEM_PLUS_UPGRADE_SRC, kInfo);

	if( g_kItemPlusUpgradeMgr.GetSrcItemPos() == SItemPos::NullData() )
	{
		return;
	}

	//부속재료
	PgPlayer*	pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	PgInventory*	pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);

	PgBase_Item kTargetItem;
	if( S_OK != pkInv->GetItem(kItemPos, kTargetItem) )
	{
		return;
	}

	g_kItemPlusUpgradeMgr.SetResultItem(kTargetItem);

	int const iWeaponLv = static_cast<int>(kTargetItem.EnchantInfo().PlusLv());

	PgBase_Item kItem;
	ContHaveItemNoCount rkOut;
	if( kInsurePos != SItemPos::NullData() && S_OK == pkInv->GetItems(UICT_ENCHANT_INSURANCE, rkOut) )
	{
		if( g_kItemPlusUpgradeMgr.InsureItemNo() != 0 )
		{
			bool bSetInsure = false;

			ContHaveItemNoCount::iterator iter = rkOut.find(g_kItemPlusUpgradeMgr.InsureItemNo());
			if( iter != rkOut.end() )
			{
				ContHaveItemNoCount::key_type const& kItemNo = iter->first;

				if( PgUIItemPlusUtil::ReSetUpdateDataToInsureItem( iWeaponLv, kItemNo ) )
				{
					if( S_OK == pkInv->GetFirstItem(kItemNo, kInsurePos) )
					{
						g_kItemPlusUpgradeMgr.SetMaterialItem(ENEEDIO_INSURANCE, kInsurePos, true);
						bSetInsure = true;
					}
				}
			}

			if( !bSetInsure )
			{
				iter = rkOut.begin();
				while( iter != rkOut.end() )
				{
					ContHaveItemNoCount::key_type const& kItemNo = iter->first;

					if( PgUIItemPlusUtil::ReSetUpdateDataToInsureItem( iWeaponLv, kItemNo ) )
					{
						if( S_OK == pkInv->GetFirstItem(kItemNo, kInsurePos) )
						{
							g_kItemPlusUpgradeMgr.SetMaterialItem(ENEEDIO_INSURANCE, kInsurePos, true);
							break;
						}
					}
					++iter;
				}
			}
		}
	}

	rkOut.clear();

	//인챈트 보험 스크롤이 해제된 경우 경고메시지 출력 & 인챈트할 아이템 제거
	bool const bOldUseInsure = SItemPos::NullData()!=kInsurePos;
	kInsurePos = g_kItemPlusUpgradeMgr.GetInsureItemPos();
	bool const bUseInsure = SItemPos::NullData()!=kInsurePos;
	if( (true==bOldUseInsure) && (false==bUseInsure) )
	{
		g_kItemPlusUpgradeMgr.Clear();
		PgUIItemPlusUtil::ClearPlusUI();
		Notice_Show_ByTextTableNo(1229, EL_Warning);
		return;
	}

	if( kProbPos != SItemPos::NullData() )
	{
		if( S_OK == pkInv->GetItems(UICT_PLUSE_SUCCESS, rkOut) )
		{
			if( S_OK == pkInv->GetItem(kItemPos, kItem) )
			{
				E_ITEM_GRADE eGrade = GetItemGrade(kItem);
				SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();

				DWORD dwItemNo = 0;

				ContHaveItemNoCount::iterator iter = rkOut.begin();
				while( iter != rkOut.end() )
				{
					ContHaveItemNoCount::key_type const& kItemNo = iter->first;

					CItemDef const *pDef = kItemDefMgr.GetDef(kItemNo);
					if(pDef)
					{
						int const iMin = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
						int const iMax = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);

						if( iMin <= kEnchantInfo.PlusLv() && kEnchantInfo.PlusLv() <= iMax )
						{
							dwItemNo = kItemNo;
						}
					}
					++iter;
				}

				if( dwItemNo )
				{
					if( S_OK == pkInv->GetFirstItem( dwItemNo, kProbPos ) )
					{
						for( int i = 0; i < iProbCount; ++i )
						{
							g_kItemPlusUpgradeMgr.SetMaterialItem(ENEEDIO_PROBABILITY, kProbPos, true);
						}
					}
				}
			}
		}
	}

	PgUIItemPlusUtil::SetSuccessRateUI();
}

bool lwUIItemPlusUpgrade::SendReqPlusUpgrade(bool bIsTrueSend)
{
	return g_kItemPlusUpgradeMgr.SendReqPlusUpgrade(bIsTrueSend);
}

int lwUIItemPlusUpgrade::GetUpgradeNeedMoney()
{
	return g_kItemPlusUpgradeMgr.GetUpgradeNeedMoney();
}

void lwUIItemPlusUpgrade::CallComfirmMessageBox( const bool bIsModal )
{
	g_kItemPlusUpgradeMgr.CallComfirmMessageBox( bIsModal );
}

void Recv_PT_C_M_ANS_ITEM_PLUS_UPGRADE(BM::Stream* pkPacket)
{
	PLUS_ITEM_UPGRADE_RESULT kPIUR;

	if(pkPacket->Pop(kPIUR))
	{
		bool bCanInsurance;
		pkPacket->Pop(bCanInsurance);
		char szName[100] = "EnchantFail";
		std::wstring wstrWarnMessage;
		ENoticeLevel eLevel = EL_Warning;
		g_kItemPlusUpgradeMgr.RecentResult(kPIUR.eResult);
		switch(kPIUR.eResult)
		{
		case PIUR_SUCCESS://	= 1,
			{
				eLevel = EL_Normal;
				sprintf(szName, "EnchantSuccess");
				wstrWarnMessage = ::TTW(1201);//	제련에 성공 하였습니다
			}break;
		case PIUR_NOT_ENOUGH_RES: //	= 2,//재료 부족
			{
				wstrWarnMessage = ::TTW(1202);//	제련에 필요한 재료가 부족합니다
			}break;
		case PIUR_CANT_DELETE_RES: //	= 3,//재료 부족
			{
				wstrWarnMessage = ::TTW(1203);//	제련에 사용된 재료를 소모 할 수 없어 실패 하였습니다
			}break;
		case PIUR_NORMAL_FAILED: //	= 4,//걍 실패(초급)
			{
				if(bCanInsurance)
				{
					wstrWarnMessage = ::TTW(1209);//인첸트에 실패 하였지만, 보험의 효과로 아이템은 지장이 없습니다.
				}
				else
				{
					wstrWarnMessage = ::TTW(1204);//	제련에 실패 하였습니다
				}
			}break;
		case PIUR_PANALTY_FAILED: //	= 5,//등급이 깎이는 실패(중급)
			{
				if(bCanInsurance)
				{
					wstrWarnMessage = ::TTW(1199);//인첸트에 실패 하였지만, 보험의 효과로 아이템은 지장이 없습니다. 단, 등급하락은 있음
				}
				else
				{
					wstrWarnMessage = ::TTW(1205);//	제련에 실패 하여 등급이 하락 되었습니다
				}
			}break;
		case PIUR_DESTROY_FAILED:
			{
				if(bCanInsurance)
				{
					wstrWarnMessage = ::TTW(1209);//인첸트에 실패 하였지만, 보험의 효과로 아이템은 지장이 없습니다.
				}
				else
				{
					wstrWarnMessage = ::TTW(1228);
				}
			}break;
		case PIUR_NOT_ENOUGH_MONEY: //	= 2,//재료 부족
			{
				wstrWarnMessage = ::TTW(1207);
			}break;
		case PIUR_NOT_FOUND_ITEM:// 가공 아이템 찾을수 없음
		case PIUR_OVER_LEVELLIMIT:// 인첸트 제한 레벨에 도달했다?
		case PIUR_CAN_NOT_ENCHANT:// 인첸트 할수 없는 아이템이다.
			{
				wstrWarnMessage = ::TTW(1239 + kPIUR.eResult);
			}break;
		default:
			{
				return;
			}break;
		}

		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szName, 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
		Notice_Show(wstrWarnMessage, eLevel, false);
	}
	g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_BLACKSMITH), 0);
}

int lwUIItemPlusUpgrade::CheckOK()
{
	if (g_kItemPlusUpgradeMgr.IsChangedGuid())
	{
		return 1222;
	}
	__int64 const iNeedMoney = g_kItemPlusUpgradeMgr.GetUpgradeNeedMoney();
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(pkUnit)
	{
		__int64 const iMoney = pkUnit->GetAbil64(AT_MONEY);
		if (iNeedMoney > iMoney)
		{
			return 1207;
		}
	}
	if (g_kItemPlusUpgradeMgr.CheckNeedItem())
	{
		return 1202;
	}
	return 0;
}

int const lwUIItemPlusUpgrade::GetNowNeedItemCount(int const iNeed) const
{
	return g_kItemPlusUpgradeMgr.GetNowNeedItemCount(iNeed);
}

void lwUIItemPlusUpgrade::CallCheckInsureItem()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		PgInventory* pkInv = pkPlayer->GetInven();
		if( pkInv )
		{
			ContHaveItemNoCount	rkOut;
			if( S_OK != pkInv->GetItems(UICT_ENCHANT_INSURANCE, rkOut) )
			{
				CallCommonMsgYesNoBox(TTW(3301), 3302, 3303, lwPacket(NULL), true, MBT_PLUS_INSURE_OKCANCEL, NULL);
				return;
			}
			CallComfirmMessageBox();
		}
	}
}

void lwUIItemPlusUpgrade::ResultProcess()
{
	g_kItemPlusUpgradeMgr.ResultProcess();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
tagNeedItemPlusUpgrade::tagNeedItemPlusUpgrade()
{
	Init();
}
void tagNeedItemPlusUpgrade::Init()
{
	iIndex = 0; 
	iItemNo = 0;
	iNeedNum = 0;
	iNowNum = 0;
}
void tagNeedItemPlusUpgrade::SetInfo(int iIdx, int iItmNo, int iNeed, int iNow)
{
	iIndex = iIdx; 
	iItemNo = iItmNo;
	iNeedNum = iNeed;
	iNowNum = iNow;
}
bool tagNeedItemPlusUpgrade::IsOK()	//재련 가능 상태?
{
	bool bOk = false;
	switch(iIndex)
	{
	case 0: // 재료 아이템
		{
		}break;
	case 1: // 확율 증가 아이템
	case 2: // 보험
	case 3:
		{
			return true;
		}break;
	}
	return (iNowNum >= iNeedNum);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PgItemPlusUpgradeMgr
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgItemPlusUpgradeMgr::PgItemPlusUpgradeMgr()
{
	Clear();
}

void PgItemPlusUpgradeMgr::Clear(bool const bAllClear)
{
	m_kResultItem.Clear();
	m_guidSrcItem.Clear();
	m_kItem.Clear();
	m_kSrcItemPos.Clear();
	m_kInsureItemPos.Clear();
	m_kProbabilityItemPos.Clear();
	for (int i = 0; i < MAX_ITEM_PLUS_UPGRADE_NEED_ARRAY+1; ++i)
	{
		m_kItemArray[i].Init();
	}
	m_kInsureItemPos.Clear();
	if( bAllClear )
	{
		m_kNpcGuid.Clear();
	}

	PgUIItemPlusUtil::ClearPlusUI();
	m_iProbabilityUseCount = 0;
	m_iProbAbilityNo = 0;
	g_kSoundMan.StopAudioSourceByID("Enchant");
}

int PgItemPlusUpgradeMgr::CallComfirmMessageBox( const bool bIsModal )
{
	//		const TBL_DEF_ITEMPLUSUPGRADE* pDef = GetPlusInfo();
	if(!PgBase_Item::IsEmpty(&m_kItem))
	{
		int iTextNo = 1350;

		BYTE const byLv = m_kItem.EnchantInfo().PlusLv();
		bool bUseInsure = SItemPos::NullData() != m_kInsureItemPos;

		if(byLv >= 16) //16~20
		{
			if(bUseInsure) //인챈트 보험 사용
			{
				switch(g_kLocal.ServiceRegion())
				{
				case LOCAL_MGR::NC_EU: //유럽, 북미는 보험 사용시 등급 하락 없다.
				case LOCAL_MGR::NC_USA:
					{
						iTextNo = 1355;
					}break;
				default: //그외 국가는 16등급부터 인챈트 보험 써도 실패 하면 1단계 등급 하락
					{
						iTextNo = iTextNo = 1354;
					}break;
				}
			}
			else
			{
				iTextNo = 1353;
			}
		}
		else if(byLv >= 9)
		{
			if(bUseInsure) //인챈트 보험 사용
			{
				iTextNo = 1355;
			}
			else
			{
				iTextNo = 1353;
			}
		}
		else if(byLv >= 4)
		{
			if(bUseInsure) //인챈트 보험 사용
			{
				iTextNo = 1352;
			}
			else
			{
				iTextNo = 1351;
			}
		}
		else if(byLv >= 0)
		{
			iTextNo = 1350;
		}
		else
		{
			if(!PgBase_Item::IsEmpty(&m_kItem))
			{
				m_kItem.Clear();
			}
			lwAddWarnDataTT(60002);
			return 0;
		}
/*
		if( 0 <= byLv && 1 >= byLv)
		{
			iTextNo = 1215;
		}
		else if( 2 <= byLv && 3 >= byLv)
		{
			iTextNo = 1216;
		}
		else if( 4 <= byLv && 8 >= byLv)
		{
			iTextNo = 1217;
		}
		else if( 9 <= byLv && IPULL_LIMIT_MAX >= byLv)
		{
			iTextNo = 1218;
		}
		else
		{
			goto __ERROR;
		}
*/
		XUI::CXUI_Wnd *pWnd =  XUIMgr.Call(_T("SFRM_MSG_REFINE"), bIsModal );
		if(pWnd)
		{
			XUI::CXUI_Wnd *pColorWnd =  pWnd->GetControl(_T("SFRM_COLOR"));
			if(pColorWnd)
			{
				XUI::CXUI_Wnd *pSdwWnd =  pColorWnd->GetControl(_T("SFR_SDW"));
				if(pSdwWnd)
				{//커스텀 데이터 넘겨줘.
					//pSdwWnd->Text(TTW(iTextNo));

					//폼의 싸이즈를 텍스트량에 맞게 늘린다.
					BM::vstring strMsg(L"");
					strMsg = TTW(iTextNo);
					pSdwWnd->Text(strMsg);

					CXUI_Style_String kStyle;
					const POINT2 ptTextSize = pSdwWnd->AdjustText(pSdwWnd->Font(), strMsg, kStyle, pSdwWnd->Width() );
					int iStretchOffset = ptTextSize.y - (pSdwWnd->Size().y - (pSdwWnd->TextPos().y * 2) );

					if(iStretchOffset > 0)
					{
						pSdwWnd->Size(POINT2(pSdwWnd->Size().x, pSdwWnd->Size().y + iStretchOffset));
						pColorWnd->Size(POINT2(pColorWnd->Size().x, pColorWnd->Size().y + iStretchOffset));
						pWnd->Size(POINT2(pWnd->Size().x, pWnd->Size().y + iStretchOffset));

						XUI::CXUI_Wnd* pBtnOK = pWnd->GetControl(_T("BTN_TRY_REFINE"));
						XUI::CXUI_Wnd* pBtnCancel = pWnd->GetControl(_T("BTN_CANCLE"));
						if(pBtnOK && pBtnCancel)
						{
							pBtnOK->Location(POINT2(pBtnOK->Location().x, pBtnOK->Location().y + iStretchOffset) );
							pBtnCancel->Location(POINT2(pBtnCancel->Location().x, pBtnCancel->Location().y + iStretchOffset) );
						}
					}

					return 0;
				}
			}
		}

		assert(pWnd);
	}
	return 0;
}

__int64 PgItemPlusUpgradeMgr::GetUpgradeNeedMoney() const
{
	/*const TBL_DEF_ITEMPLUSUPGRADE* pDef = GetPlusInfo();
	if(pDef)
	{
	return pDef->iNeedMoney;
	}*/

	__int64 i64Money = PgItemRarityUpgradeFormula::GetPlusUpgradeCost(m_kItem);
	if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_MYHOME))// 마이홈 내부에 있으면
	{
		i64Money = static_cast<int>(static_cast<float>(i64Money) * ((100.0f - lwHomeUI::GetMyHomeSideJobDiscountRate(MSJ_ENCHANT, MSJRT_GOLD)) / 100.0f ));
	}
	return i64Money;
}

void PgItemPlusUpgradeMgr::SetItem(EKindUIIconGroup const kType, SIconInfo const & rkInfo)
{
	if (IsInProgress())
	{
		lwAddWarnDataTT(1208);
	}
	else if (0!=rkInfo.iIconGroup && 0<=rkInfo.iIconKey)
	{
		bool bSetItem = false;

		switch( kType )
		{
		case KUIG_ITEM_PLUS_UPGRADE_SRC:
			{
				if( SetSrcItem(SItemPos(rkInfo.iIconGroup, rkInfo.iIconKey)) )
				{
					bSetItem = true;
					PgUIItemPlusUtil::InitMaterialBtnState();
					PgUIItemPlusUtil::SetSuccessRateUI();
				}
			}break;
		case KUIG_ITEM_PLUS_UPGRADE_INSURENCE:
		case KUIG_ITEM_PLUS_UPGRADE_PROBABILITY:
			{
				if( SetMaterialItem(kType, SItemPos(rkInfo.iIconGroup, rkInfo.iIconKey)) )
				{
					bSetItem = true;
				}
			}break;
		default:
			{
				assert(0);
			}break;
		}

		if( bSetItem )
		{
			XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("SFRM_ITEM_PLUS_UPGRADE"));
			if( pWnd )
			{
				XUI::CXUI_Wnd* pSrc = pWnd->GetControl(L"IMG_ITEM");
				if( pSrc )
				{
					pSrc->Visible(false);
				}
			}
		}
	}
}

bool  PgItemPlusUpgradeMgr::SetMaterialItem(EKindUIIconGroup const kGroup, const SItemPos& rkItemPos)
{
	if(IsInProgress())
	{
		lwAddWarnDataTT(1208);
		return false;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	PgBase_Item kItem;
	if(S_OK != pInv->GetItem(rkItemPos, kItem)){ return false; }

	switch( kGroup )
	{
	case KUIG_ITEM_PLUS_UPGRADE_INSURENCE:	
		{ 
			m_kInsureItemPos = rkItemPos;			
			InsureItemNo(kItem.ItemNo());
		}break;
	case KUIG_ITEM_PLUS_UPGRADE_PROBABILITY:
		{
			m_kProbabilityItemPos = rkItemPos;
			++m_iProbabilityUseCount;

			XUI::CXUI_Wnd* pWnd = XUIMgr.Get(L"SFRM_ITEM_PLUS_UPGRADE");
			if( !pWnd ){ return false; }

			pWnd = pWnd->GetControl(L"SFRM_COLOR");
			if( !pWnd ){ return false; }

			pWnd = pWnd->GetControl(L"SFRM_SHADOW");
			if( !pWnd ){ return false; }

			BM::vstring vStr(L"FRM_MATERIAL");
			vStr += (int)ENEEDIO_PROBABILITY;
			pWnd = pWnd->GetControl(vStr);
			if( !pWnd ){ return false; }

			int const iTotalCount = pInv->GetTotalCount( kItem.ItemNo() );
			if( m_iProbabilityUseCount > iTotalCount )
			{
				m_iProbabilityUseCount = iTotalCount;
				SetSrcMaterialBtnInit(pWnd, false);
			}
			else
			{
				int iItemCount = 0;
				TBL_DEF_ITEMPLUSUPGRADE const* pDef = GetPlusInfo();
				if( pDef )
				{
					iItemCount = pDef->aNeedItemCount[ENEEDIO_PROBABILITY];
				}

				if( m_iProbabilityUseCount == iItemCount )
				{
					SetSrcMaterialBtnInit(pWnd, false);
				}
				else
				{
					SetSrcMaterialBtnTrue(pWnd, true);
				}
			}
		}break;
	default:
		{
			assert(0);
			return false;
		}
	}
	return true;
}

bool PgItemPlusUpgradeMgr::SetSrcItem(const SItemPos &rkItemPos)
{
	if (rkItemPos.x && rkItemPos.y)
	{
		Clear();//클리어 밟아야함.		
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return false;}

	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return false;}

	switch(rkItemPos.x)
	{
	case KUIG_EQUIP:
	case KUIG_CASH:
	case KUIG_PET:
	case KUIG_SAFE_ADDITION:
		{
			pInv->GetItem(rkItemPos, m_kItem);
			if (PgBase_Item::IsEmpty(&m_kItem))
			{
				return false;
			}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pItemDef = kItemDefMgr.GetDef(m_kItem.ItemNo());
			if( !pItemDef )
			{
				return false;
			}

			if( !pItemDef->CanEquip() )
			{
				return false;
			}

			if( (ICMET_Cant_Enchant & pItemDef->GetAbil(AT_ATTRIBUTE)) == ICMET_Cant_Enchant )
			{
				return false;
			}

			__int64 const iNowLv = m_kItem.EnchantInfo().PlusLv();
			E_ITEM_GRADE const eItemGrade = GetItemGrade(m_kItem);

			if(	IG_CURSE == eItemGrade
				||	IG_SEAL == eItemGrade )
			{//등급 문제로 제련이 안됨.
				lwAddWarnDataTT(60002);
				return false;
			}

			int iOutLevel = 0;
			if( !PgItemRarityUpgradeFormula::GetMaxGradeLevelRestrict( eItemGrade, pItemDef->IsPetItem(), iOutLevel, m_kItem ) )
			{
				lwAddWarnDataTT(1248);
				return false;
			}
			else
			{
				if( iOutLevel <= m_kItem.EnchantInfo().PlusLv() )
				{
					lwAddWarnDataTT(1247);
					return false;
				}
			}

			TBL_DEF_ITEMPLUSUPGRADE const * pDef = GetPlusInfo();
			if(pDef )//다음 업글 할 수 있는게냐?
			{
				m_guidSrcItem = m_kItem.Guid();//GUID 기록
				m_kResultItem = m_kItem;

				SEnchantInfo kTempEnchant = m_kResultItem.EnchantInfo();
				if(0==kTempEnchant.PlusType())
				{
					if(EQUIP_POS_WEAPON == pItemDef->EquipPos())
					{
						kTempEnchant.PlusType(1);
					}
					else
					{
						kTempEnchant.PlusType(2);
					}
				}
				kTempEnchant.PlusLv(iNowLv+1i64);

				m_kResultItem.EnchantInfo(kTempEnchant);
				m_kSrcItemPos = rkItemPos;
				m_iProbAbilityNo = pDef->aNeedItemNo[ENEEDIO_PROBABILITY - 1];
				return true;
			}
			else
			{
				if(!PgBase_Item::IsEmpty(&m_kItem))
				{
					m_kItem.Clear();
				}
				lwAddWarnDataTT(60002);
			}
		}break;
	case KUIG_FIT:
		{
			lwAddWarnDataTT(1206);
		}break;
		//{
		//	lwAddWarnDataTT(59007);
		//}break;
	default:
		{
		}break;
	}
	//	lwAddWarnDataTT(60002);
	return false;
}

void PgItemPlusUpgradeMgr::DisplayNeedItemIcon(int const iNeedIndex, XUI::CXUI_Wnd *pWnd)
{
	XUI::CXUI_Wnd *pFormWnd = NULL;
	XUI::CXUI_Wnd *pShadowWnd = NULL;
	XUI::CXUI_Wnd *pSrcNameWnd = NULL;

	int const iNull = 0;

	if (!pWnd)
	{
		return;
	}

	pFormWnd = pWnd->Parent();
	if (!pFormWnd)
	{
		return;
	}
	pShadowWnd = pFormWnd->Parent();
	assert(pShadowWnd);
	if(pShadowWnd)
	{
		pSrcNameWnd = pShadowWnd->GetControl(_T("SFRM_MATERIAL_NAME"));
	}
	XUI::CXUI_Wnd *pTopWnd = pFormWnd->Parent();
	assert(pTopWnd);
	if (!pTopWnd)
	{
		return;
	}

	if(!m_kItem.ItemNo()){goto __HIDE;}
	if(m_kItem.Guid() != m_guidSrcItem){goto __HIDE;}//guid 바뀌었으면 찾아가든가.

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		assert(pkPlayer);
		return;
	}

	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		assert(pkInv);
		return;
	}

	//		pWnd->Visible(true);
	m_kItemArray[iNeedIndex].Init();
	const TBL_DEF_ITEMPLUSUPGRADE* pDef = GetPlusInfo();
	if(pDef)
	{
		if( iNeedIndex < 0 || iNeedIndex > MAX_ITEM_PLUS_UPGRADE_NEED_ARRAY )
		{
			return;
		}

		int iNeedArrayIndex = 0;
		bool bIsInsure		= false;
		bool bIsProbability	= false;
		bool bUseInsure			= SItemPos::NullData()!=m_kInsureItemPos;
		bool bUseProbability	= SItemPos::NullData()!=m_kProbabilityItemPos;

		int iNeedItemNo = 0;
		switch( iNeedIndex )
		{
		case ENEEDIO_INSURANCE:
			{
				iNeedArrayIndex = iNeedIndex + 1;
				bIsInsure = true;
				if( bUseInsure )
				{
					PgBase_Item kItem;
					if( S_OK != pkInv->GetItem(m_kInsureItemPos, kItem) )
					{
						return;
					}

					iNeedItemNo = kItem.ItemNo();
				}
				else
				{
					iNeedItemNo = pDef->aNeedItemNo[iNeedArrayIndex];
				}
			}break;
		case ENEEDIO_PROBABILITY:	
			{ 
				iNeedArrayIndex = iNeedIndex - 1; 
				bIsProbability = true;
				iNeedItemNo = pDef->aNeedItemNo[iNeedArrayIndex];
			}break;
		default:
			{
				iNeedItemNo = pDef->aNeedItemNo[iNeedArrayIndex];
			}break;
		}

		const POINT2 rPT =	pWnd->TotalLocation();

		int iNeedItemCount;
		switch( iNeedIndex )
		{
		case ENEEDIO_PROBABILITY:	
			{ 
				iNeedItemCount = pDef->aNeedItemCount[ENEEDIO_PROBABILITY]; 
			}break;
		default:
			{ 
				iNeedItemCount = pDef->aNeedItemCount[iNeedArrayIndex]; 
			}break;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(iNeedItemNo);
		if(pItemDef)
		{
			bool bGrayScale = false;
			switch( iNeedIndex )
			{
			case ENEEDIO_INSURANCE:		{ bGrayScale = !bUseInsure;			}break;
			case ENEEDIO_PROBABILITY:	{ bGrayScale = !bUseProbability;	}break;
			}
			g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT, false, 40, 40, bGrayScale );
		}
		pWnd->SetCustomData(&iNeedItemNo, sizeof(iNeedItemNo));

		const wchar_t *pName = NULL;
		if(	pDef 
			&&	GetItemName(iNeedItemNo, pName))
		{
			int iMyCount = pkPlayer->GetInven()->GetTotalCount(iNeedItemNo);
			if( iNeedIndex == ENEEDIO_PROBABILITY )
			{
				iMyCount = m_iProbabilityUseCount;
			}

			BM::vstring vstr;

			if(ENEEDIO_DUST == iNeedIndex)
			{
				int const iCustomType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
				if(0 < iCustomType)
				{
					ContHaveItemNoCount kCont;
					pkInv->GetItems(static_cast<EUseItemCustomType>(iCustomType), kCont);
					iMyCount = 0;
					for(ContHaveItemNoCount::const_iterator iter = kCont.begin();iter != kCont.end();++iter)
					{
						iMyCount += (*iter).second;
					}
				}

				if(iMyCount < iNeedItemCount){vstr += TTW(1302);}//칼라지정.
				else if(iNeedItemCount<=0) {vstr += TTW(1303);}
				else{	vstr += TTW(1301);}
			}

			BM::vstring vNameStr = vstr;
			std::wstring wstrText(pName);
			vNameStr += wstrText;

			vstr += TTW(1230+iNeedIndex);
			vstr += _T(" ");
			int iNowCount = __min(iMyCount,iNeedItemCount);
			if(( ENEEDIO_INSURANCE == iNeedIndex && !bUseInsure)
				|| ( ENEEDIO_PROBABILITY == iNeedIndex && !bUseProbability) )
			{
				iNowCount = 0;
			}
			vstr += iNowCount;
			vstr += TTW(1300);//화이트
			vstr += _T(" / ");
			vstr += iNeedItemCount;

			assert(pSrcNameWnd);

			if(pSrcNameWnd)
			{//리소스 아이템 이름이 길면 말줄임 표를 사용한다
				int const iWidth = pSrcNameWnd->Width()+REGULATION_UI_WIDTH;
				Quest::SetCutedTextLimitLength(pSrcNameWnd, static_cast<std::wstring>(vNameStr), WSTR_UI_REGULATIONSTR, iWidth);
			}
			if (pTopWnd)
			{
				pTopWnd->Text((const wchar_t*)vstr);
			}

			m_kItemArray[iNeedIndex].SetInfo(iNeedIndex, iNeedItemNo, iNeedItemCount, ((ENEEDIO_DUST == iNeedIndex) || bUseInsure || bUseProbability)?(iMyCount):(0));
		}
		else
		{
			assert(NULL);
		}
	}
	return;
__HIDE:
	{
		assert(pSrcNameWnd);
		if(pSrcNameWnd)
		{
			pSrcNameWnd->Text(_T(""));
		}
		if (pTopWnd)
		{
			pTopWnd->Text(_T(""));
		}
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		m_kItemArray[iNeedIndex].Init();
	}
}

void PgItemPlusUpgradeMgr::DisplaySrcItem(XUI::CXUI_Wnd *pWnd)
{//네임 필드를 찾아서 이름 셋팅. 아이템이 없거나 하면 올리세요 라는 메세지로 셋팅.
	XUI::CXUI_Wnd *pFormWnd = NULL;
	XUI::CXUI_Wnd *pShadowWnd = NULL;
	XUI::CXUI_Wnd *pSrcNameWnd = NULL;

	int const iNull = 0;

	if (!pWnd)
	{
		return;
	}

	pFormWnd = pWnd->Parent();
	assert(pFormWnd);
	if (!pFormWnd)
	{
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		return;
	}
	if(pFormWnd)
	{
		pShadowWnd = pFormWnd->Parent();
		assert(pShadowWnd);
		if(pShadowWnd)
		{
			pSrcNameWnd = pShadowWnd->GetControl(_T("SFRM_SRC_NAME"));
		}
		else
		{
			pWnd->SetCustomData(&iNull, sizeof(iNull));
			return;
		}
	}

	std::wstring wstrName;
	POINT2 rPT;

	if(!m_kItem.ItemNo()){goto __HIDE;}
	if(IsChangedGuid()){goto __HIDE;}//guid 바뀌었으면 찾아가든가.

	rPT =	pWnd->TotalLocation();

	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(m_kItem.ItemNo());
		if(pItemDef)
		{
			g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT, false );
		}
		MakeItemName(m_kItem.ItemNo(), m_kItem.EnchantInfo(), wstrName);

		pWnd->SetCustomData(&m_kItem.ItemNo(), sizeof(m_kItem.ItemNo()));
	}


	assert(pSrcNameWnd);
	if(pSrcNameWnd)
	{// 아이템 이름이 너무 길면 말줄임을 사용한다.
		std::wstring kCutStr(WSTR_UI_REGULATIONSTR);
		//std::wstring::size_type const PlusIndex = wstrName.find(PLUS_STR);		
		//if(std::wstring::npos  !=  PlusIndex)
		//{
		//	std::wstring::size_type const size = wstrName.size() - PlusIndex;			
		//	kCutStr += std::wstring(wstrName,PlusIndex, size);
		//}
		int const iWidth = pSrcNameWnd->Width()+REGULATION_UI_WIDTH;
		Quest::SetCutedTextLimitLength(pSrcNameWnd, wstrName, kCutStr, iWidth);
	}
	return;
__HIDE:
	{
		assert(pSrcNameWnd);
		if(pSrcNameWnd)
		{
			pSrcNameWnd->Text(TTW(60001));
		}
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		m_kItem.Clear();
		m_kSrcItemPos.Clear();
	}
	return;
}

void PgItemPlusUpgradeMgr::DisplayResultItem(XUI::CXUI_Wnd *pWnd)
{
	if (!pWnd)
	{
		return;
	}

	XUI::CXUI_Wnd *pSrcNameWnd = NULL;

	static int const iNull = 0;
	XUI::CXUI_Wnd *pFormWnd = pWnd->Parent();
	assert(pFormWnd);
	if (!pFormWnd)
	{
		pWnd->SetCustomData(&iNull, sizeof(iNull));
		return;
	}
	if(pFormWnd)
	{
		if(XUI::CXUI_Wnd *pShadowWnd = pFormWnd->Parent())
		{
			pSrcNameWnd = pShadowWnd->GetControl(_T("SFRM_DEST_NAME"));
		}
		else
		{
			pWnd->SetCustomData(&iNull, sizeof(iNull));
			return;
		}
	}
	std::wstring wstrName;

	if(!m_kItem.ItemNo()){goto __HIDE;}
	if(IsChangedGuid()){goto __HIDE;}//guid 바뀌었으면 찾아가든가.
	if(!m_kResultItem.ItemNo()){goto __HIDE;}

	int const iItemNo = m_kResultItem.ItemNo();
	GET_DEF(CItemDefMgr, kItemDefMgr);
	if(CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo))
	{
		g_kUIScene.RenderIcon( pItemDef->ResNo(), pWnd->TotalLocation(), false );
	}

	MakeItemName(iItemNo, m_kResultItem.EnchantInfo(), wstrName);
	pWnd->SetCustomData(&iItemNo, sizeof(iItemNo));

	assert(pSrcNameWnd);
	if(pSrcNameWnd)
	{// 아이템 이름이 너무 길면 말줄임을 사용한다.
		std::wstring kCutStr(WSTR_UI_REGULATIONSTR);
		//std::wstring::size_type const PlusIndex = wstrName.find(PLUS_STR);		
		//if(std::wstring::npos  !=  PlusIndex)
		//{
		//	std::wstring::size_type const size = wstrName.size() - PlusIndex;			
		//	kCutStr += std::wstring(wstrName,PlusIndex, size);
		//}
		int const iWidth = pSrcNameWnd->Width()+REGULATION_UI_WIDTH;
		Quest::SetCutedTextLimitLength(pSrcNameWnd, wstrName, kCutStr, iWidth);
	}
	return;
__HIDE:
	{
		assert(pSrcNameWnd);
		if(pSrcNameWnd)
		{
			pSrcNameWnd->Text(_T(""));
			pWnd->SetCustomData(NULL, sizeof(iItemNo));
		}
		pWnd->SetCustomData(&iNull, sizeof(iNull));

		if (XUI::CXUI_Wnd* pkQ = pWnd->GetControl(_T("IMG_Q")))
		{
			pkQ->Visible(false);
		}
		m_kResultItem.Clear();
	}
}

void PgItemPlusUpgradeMgr::SetMaterialItem(XUI::CXUI_Wnd* pWnd, bool bNoBuyMsg)
{
	if (IsInProgress())
	{
		lwAddWarnDataTT(1208);
		return;
	}

	if( !pWnd ){ return; }

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){ return; }
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv ){ return; }

	PgBase_Item kTargetItem;
	if( S_OK != pkInv->GetItem(m_kSrcItemPos, kTargetItem) )
	{
		return;
	}

	int iItemNo = 0;

	int const MaterialIdx = pWnd->BuildIndex();
	int DBIdx = 0;
	SItemPos	rkPos;

	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_DEVELOP:
		{
			bNoBuyMsg = false;
		}break;
	default:
		{
			bNoBuyMsg = true;
		}break;
	}

	switch( MaterialIdx )
	{
	case ENEEDIO_INSURANCE:
		{ 
			if( SItemPos::NullData() != m_kInsureItemPos )
			{
				m_kInsureItemPos.Clear();
				goto _DEREG;
			}

			ContHaveItemNoCount	rkItemCont;
			if( E_FAIL == pkInv->GetItems(UICT_ENCHANT_INSURANCE, rkItemCont) )
			{
				if( false == bNoBuyMsg )
				{
					lwCashShop::lwOnCallStaticCashItemBuy(0, kTargetItem.EnchantInfo().PlusLv());
				}
				else
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790401, true);
				}
				goto _DEREG;
			}

			ContHaveItemNoCount::iterator iter = rkItemCont.begin();
			while( iter != rkItemCont.end() )
			{
				ContHaveItemNoCount::key_type const& kItemNo = iter->first;
				if( !PgUIItemPlusUtil::ReSetUpdateDataToInsureItem( kTargetItem.EnchantInfo().PlusLv(), kItemNo ) )
				{
					iter = rkItemCont.erase(iter);
					continue;
				}
				++iter;
			}

			if( rkItemCont.size() > 1 )
			{
				UIItemUtil::CONT_CUSTOM_PARAM	kParam;
				std::pair<UIItemUtil::CONT_CUSTOM_PARAM::iterator, bool> ParamRst = kParam.insert(std::make_pair(std::wstring(L"CALL_UI"), UIItemUtil::EICUT_PLUS_UPGRADE));
				if( !ParamRst.second )
				{
					goto _DEREG;
				}
				UIItemUtil::CallCommonUseCustomTypeItems(rkItemCont, UIItemUtil::ECIUT_CUSTOM_DEFINED, kParam, UIItemUtil::CONT_CUSTOM_PARAM_STR());
				return;
			}
			else if( rkItemCont.size() == 1 )
			{
				iItemNo = rkItemCont.begin()->first;
			}
		}break;
	case ENEEDIO_PROBABILITY: 
		{ 
			TBL_DEF_ITEMPLUSUPGRADE const* pDef = GetPlusInfo();
			if( pDef )
			{
				if( m_iProbabilityUseCount >= pDef->aNeedItemCount[ENEEDIO_PROBABILITY-1] )
				{
					return;
					//if( SItemPos::NullData() != m_kProbabilityItemPos )
					//{
					//	m_kProbabilityItemPos.Clear();
					//	goto _DEREG;
					//}
				}

				iItemNo = pDef->aNeedItemNo[ENEEDIO_PROBABILITY-1];
			}
		}break;
	default: 
		{ 
			return; 
		}
	}

	if( S_OK != pkInv->GetFirstItem(iItemNo, rkPos) )
	{
		if( false == bNoBuyMsg )
		{
			switch( MaterialIdx )
			{
			case ENEEDIO_PROBABILITY:	
				{
					TBL_DEF_ITEMPLUSUPGRADE const* pDef = GetPlusInfo();
					if( pDef )
					{
						lua_tinker::call<void, int, int>("OnCallStaticCashItemBuy", 1, pDef->aNeedItemNo[MaterialIdx - 1]);
					}
				} break;
			}
		}
		else
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790401, true);
		}
		return;
	}
	else
	{
		// 수량 체크
		int const iIdx = (MaterialIdx - 1);
		TBL_DEF_ITEMPLUSUPGRADE const* pkDef = GetPlusInfo();
		if( pkDef )
		{
			int const iNeedItemNo = pkDef->aNeedItemNo[PUNT_INSURANCE_ITEM];
			int const iNeedItemCount = pkDef->aNeedItemCount[PUNT_INSURANCE_ITEM];
			switch( MaterialIdx )
			{
			case ENEEDIO_INSURANCE:
				{
					if( static_cast< size_t >(iNeedItemCount) > pkInv->GetTotalCount(iNeedItemNo) )
					{
						if( false == bNoBuyMsg )
						{
							lwCashShop::lwOnCallStaticCashItemBuy(0, kTargetItem.EnchantInfo().PlusLv());
						}
						else
						{
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790401, true);
						}
						goto _DEREG; // 수량 부족
					}
				}break;
			default:
				{
				}break;
			}
		}
	}

	SetMaterialItem(static_cast<ENEEDITEM_ORDER>(MaterialIdx), rkPos);
	if( ENEEDIO_PROBABILITY != MaterialIdx )
	{
		SetSrcMaterialBtnInit(pWnd, false);
	}
	return;
_DEREG:
	SetSrcMaterialBtnInit(pWnd, true);
}

void PgItemPlusUpgradeMgr::SetMaterialItem(ENEEDITEM_ORDER const Type, SItemPos kItemPos, bool bIsSlotUpdate)
{
	if( kItemPos.x == 0 && kItemPos.y == 0 )
	{
		return;
	}

	SIconInfo kIconInfo( kItemPos.x, kItemPos.y );
	EKindUIIconGroup Gruop = KUIG_NONE;

	switch( Type )
	{
	case ENEEDIO_INSURANCE:   { Gruop = KUIG_ITEM_PLUS_UPGRADE_INSURENCE;   }break;
	case ENEEDIO_PROBABILITY: { Gruop = KUIG_ITEM_PLUS_UPGRADE_PROBABILITY; }break;
	default: 
		{ 
			return; 
		}
	}

	SetItem(Gruop, kIconInfo);

	if( bIsSlotUpdate && ENEEDIO_PROBABILITY != Type )
	{
		XUI::CXUI_Wnd* pWnd = XUIMgr.Get(L"SFRM_ITEM_PLUS_UPGRADE");
		if( !pWnd ){ return; }

		pWnd = pWnd->GetControl(L"SFRM_COLOR");
		if( !pWnd ){ return; }

		pWnd = pWnd->GetControl(L"SFRM_SHADOW");
		if( !pWnd ){ return; }

		BM::vstring vStr(L"FRM_MATERIAL");
		vStr += (int)Type;
		pWnd = pWnd->GetControl(vStr);
		if( !pWnd ){ return; }

		SetSrcMaterialBtnInit(pWnd, false);
	}
}

void PgItemPlusUpgradeMgr::SetProbAbilityMinus()
{
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(L"SFRM_ITEM_PLUS_UPGRADE");
	if( !pWnd ){ return; }

	pWnd = pWnd->GetControl(L"SFRM_COLOR");
	if( !pWnd ){ return; }

	pWnd = pWnd->GetControl(L"SFRM_SHADOW");
	if( !pWnd ){ return; }

	BM::vstring vStr(L"FRM_MATERIAL");
	vStr += (int)ENEEDIO_PROBABILITY;
	pWnd = pWnd->GetControl(vStr);
	if( !pWnd ){ return; }

	if( m_iProbabilityUseCount )
	{
		--m_iProbabilityUseCount;

		if( m_iProbabilityUseCount <= 0 )
		{
			m_iProbabilityUseCount = 0;
			m_kProbabilityItemPos.Clear();

			SetSrcMaterialBtnInit(pWnd, true);
		}
		else
		{
			SetSrcMaterialBtnTrue(pWnd, true);
		}
	}
}

void PgItemPlusUpgradeMgr::SetSrcMaterialBtnInit(XUI::CXUI_Wnd* pWnd, bool const bVisible)
{
	if( !pWnd ){ return; }
	XUI::CXUI_Wnd* pReg = pWnd->GetControl(L"BTN_REG");
	XUI::CXUI_Wnd* pDeReg = pWnd->GetControl(L"BTN_DEREG");
	if( !pReg || !pDeReg ){ return; }
	pReg->Visible(bVisible);
	pDeReg->Visible(!pReg->Visible());
}

void PgItemPlusUpgradeMgr::SetSrcMaterialBtnTrue(XUI::CXUI_Wnd* pWnd, bool const bVisible)
{
	if( !pWnd ){ return; }
	XUI::CXUI_Wnd* pReg = pWnd->GetControl(L"BTN_REG");
	XUI::CXUI_Wnd* pDeReg = pWnd->GetControl(L"BTN_DEREG");
	if( !pReg || !pDeReg ){ return; }
	pReg->Visible(bVisible);
	pDeReg->Visible(bVisible);
}

const TBL_DEF_ITEMPLUSUPGRADE* PgItemPlusUpgradeMgr::GetPlusInfo(SItemPlusUpgradeKey const &rkKey)
{
	const CONT_DEF_ITEM_PLUS_UPGRADE* pCont = NULL;
	g_kTblDataMgr.GetContDef(pCont);

	CONT_DEF_ITEM_PLUS_UPGRADE::const_iterator itor = pCont->find(rkKey);
	if(itor != pCont->end())
	{
		return &(*itor).second;
	}
	return NULL;
}

const TBL_DEF_ITEMPLUSUPGRADE* PgItemPlusUpgradeMgr::GetPlusInfo()//렙업 할 정보가 나온다.
{
	if(!PgBase_Item::IsEmpty(&m_kItem))
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(m_kItem.ItemNo());
		if(pItemDef)
		{
			int const iEquipPos = pItemDef->GetAbil(AT_EQUIP_LIMIT);
			SItemPlusUpgradeKey kKey(  pItemDef->IsPetItem(), iEquipPos, m_kItem.EnchantInfo().PlusLv()+1i64 );//장착위치를 넣어야함.

			CONT_DEF_ITEM_PLUS_UPGRADE const *pCont = NULL;
			g_kTblDataMgr.GetContDef(pCont);

			CONT_DEF_ITEM_PLUS_UPGRADE::const_iterator itor = pCont->find(kKey);
			if(itor != pCont->end())
			{
				return &(*itor).second;
			}
		}
	}
	return NULL;
}

bool PgItemPlusUpgradeMgr::SendReqPlusUpgrade(bool bIsTrueSend)
{
	if(!m_kItem.ItemNo()){goto __FAILED;}
	if(m_kItem.Guid() != m_guidSrcItem){goto __FAILED;}//guid 바뀌었으면 찾아가든가.

	goto __SUCCESS;
	//될 수 있다 없다로 판별 해야 하는데.
__SUCCESS:
	{
		if(bIsTrueSend)
		{
			BM::Stream kPacket(PT_C_M_REQ_ITEM_PLUS_UPGRADE);
			// PgBase_Item 을 보내주
			//			kPacket.Push(m_kItem);
			kPacket.Push(m_kNpcGuid);
			//m_kItem.WriteToPacket(kPacket);
			kPacket.Push(m_kSrcItemPos);
			kPacket.Push(m_kInsureItemPos);
			kPacket.Push(m_kProbabilityItemPos);
			kPacket.Push(m_iProbabilityUseCount);

			NETWORK_SEND(kPacket)
		}
		return true;
	}
__FAILED:
	{
		return false;
	}
}

int PgItemPlusUpgradeMgr::CheckNeedItem()
{
	for (int i = 0; i < MAX_ITEM_PLUS_UPGRADE_NEED_ARRAY; ++i)
	{
		if (!m_kItemArray[i].IsOK())
		{
			return m_kItemArray[i].iItemNo;
		}
	}

	return 0;
}

int const PgItemPlusUpgradeMgr::GetNowNeedItemCount(int const iNeed) const
{
	if (MAX_ITEM_PLUS_UPGRADE_NEED_ARRAY+1 < iNeed || 0 > iNeed )
	{
		return 0;
	}
	return __min(m_kItemArray[iNeed].iNowNum, m_kItemArray[iNeed].iNeedNum);
}

bool PgItemPlusUpgradeMgr::IsChangedGuid() const
{
	if(PgBase_Item::IsEmpty(&m_kItem))
	{
		return true;
	}
	return (m_kItem.Guid() != m_guidSrcItem);
}

bool PgItemPlusUpgradeMgr::IsInProgress()
{
	if(PgBase_Item::IsEmpty(&m_kItem))
	{
		return false;
	}

	bool bRet = false;
	XUI::CXUI_Wnd *pWnd =  XUIMgr.Get(_T("SFRM_ITEM_PLUS_UPGRADE"));
	if(pWnd)
	{
		XUI::CXUI_Wnd *pColorWnd =  pWnd->GetControl(_T("SFRM_COLOR"));
		if(pColorWnd)
		{
			XUI::CXUI_Wnd *pSdwWnd =  pColorWnd->GetControl(_T("SFRM_SHADOW"));
			if(pSdwWnd)
			{
				XUI::CXUI_Wnd *pkArrow = pSdwWnd->GetControl(_T("FRM_DOWN_ARROW_BG"));
				if (pkArrow)
				{
					XUI::CXUI_Wnd *pkArrowImg = pkArrow->GetControl(_T("IMG_ARROW"));
					if (pkArrowImg)
					{
						const POINT2& rkImgSize = pkArrowImg->ImgSize();
						const POINT2& rkSize = pkArrowImg->Size();

						bRet = rkImgSize.y && rkImgSize.y < rkSize.y;
					}
				}
			}
		}
	}

	return bRet;
}

void PgItemPlusUpgradeMgr::ResultProcess()
{
	if( PIUR_SUCCESS == m_RecentResult )
	{
		RecentResult(PIUR_NONE);

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pDef = kItemDefMgr.GetDef(m_kItem.ItemNo());
		if( !pDef )
		{
			return;
		}

		SEnchantInfo const& kEnchantInfo = m_kItem.EnchantInfo();
		if( kEnchantInfo.IsPlus() )
		{
			int EnchantLv = kEnchantInfo.PlusLv();
			PgItemUpgradeInfo kItemInfo, kPrevItemInfo;
			kItemInfo.SetPlusUpgrade(m_kItem, pDef, kEnchantInfo, EnchantLv);
			kPrevItemInfo.SetPlusUpgrade(m_kItem, pDef, kEnchantInfo, EnchantLv-1);
			kItemInfo -= kPrevItemInfo;

			BM::vstring vPlusStr;
			if( EQUIP_LIMIT_WEAPON == (EQUIP_LIMIT_WEAPON & pDef->GetAbil(AT_EQUIP_LIMIT)) || true==pDef->IsPetItem() )
			{
				vPlusStr = ::TTW(5003);//물리 공격력
				vPlusStr += L" [";
				vPlusStr += ::TTW(1235);//최소
				vPlusStr += L" +";
				vPlusStr += kItemInfo.MinATK();
				vPlusStr += L" / ";

				vPlusStr += ::TTW(1236);//최대
				vPlusStr += L" +";
				vPlusStr += kItemInfo.MaxATK();
				vPlusStr += L"] ";
				vPlusStr += ::TTW(1234);//상승하였습니다.
				Notice_Show(vPlusStr, EL_Normal, false);

				vPlusStr = ::TTW(5004);//마법 공격력
				vPlusStr += L" [";
				vPlusStr += ::TTW(1235);//최소
				vPlusStr += L" +";
				vPlusStr += kItemInfo.MinMATK();
				vPlusStr += L" / ";

				vPlusStr += ::TTW(1236);//최대
				vPlusStr += L" +";
				vPlusStr += kItemInfo.MaxMATK();
				vPlusStr += L"] ";
				vPlusStr += ::TTW(1234);//상승하였습니다.
				Notice_Show(vPlusStr, EL_Normal, false);
			}
			else
			{
				vPlusStr = TTW(5041);//물리 방어력
				vPlusStr += L" [ +";
				vPlusStr += kItemInfo.DEF();
				vPlusStr += L"] ";
				vPlusStr += ::TTW(1234);//상승하였습니다.
				Notice_Show(vPlusStr, EL_Normal, false);

				vPlusStr = TTW(5042);//마법 방어력
				vPlusStr += L" [ +";
				vPlusStr += kItemInfo.MDEF();
				vPlusStr += L"] ";
				vPlusStr += ::TTW(1234);//상승하였습니다.
				Notice_Show(vPlusStr, EL_Normal, false);
			}
		}
		
		E_ITEM_GRADE const eItemGrade = GetItemGrade(m_kItem);

		int iOutLevel = 0;
		if( !PgItemRarityUpgradeFormula::GetMaxGradeLevelRestrict( eItemGrade, pDef->IsPetItem(), iOutLevel, m_kItem ) 
			|| iOutLevel <= m_kItem.EnchantInfo().PlusLv() )
		{
			m_kItem.Clear();
			return;
		}
	}
}

void PgItemPlusUpgradeMgr::SetResultItem(PgBase_Item const & rkResultItem)
{
	if( rkResultItem.IsEmpty() )
	{
		return;
	}

	m_kItem = rkResultItem;
}