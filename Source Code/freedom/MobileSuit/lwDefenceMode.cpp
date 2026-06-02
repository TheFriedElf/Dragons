#include "stdafx.h"
#include "lwDefenceMode.h"
#include "BM/PgFilterString.h"
#include "lwGUID.h"
#include "ServerLib.h"
#include "lwUI.h"
#include "lwUIMission.h"
#include "PgError.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgClientParty.h"
#include "PgNetwork.h"
#include "PgWorld.h"
#include "PgMobileSuit.h"
#include "Lohengrin/dbtables.h"
#include "PgChatMgrClient.h"
#include "lwWorld.h"
#include "lwUIGemStore.h"

int StrategicPointChargeCount = 0; // 포인트 복제기 누적 횟수.
int KillableMonsterCount = 0; // 포인트 복제기로 제거 가능한 몬스터 숫자.
int const COE_DEFENCEMODE_EXP = 2000;

namespace lwDefenceMode
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "OnMouseUpChangeAntiqueIcon", lwOnMouseUpChangeAntiqueIcon);
		def(pkState, "ShowTargetAntiqueItem", lwShowTargetAntiqueItem);
		def(pkState, "ClickChangeAntiqueButton", lwClickChangeAntiqueButton);
		def(pkState, "ClickAntiqueButtonUpDown", lwClickAntiqueButtonUpDown);
		def(pkState, "InitCollectAntiqueWnd", lwInitCollectAntiqueWnd);
		def(pkState, "OnMouseUpChangeIcon", lwOnMouseUpChangeIcon);
		def(pkState, "ShowTargetItem", lwShowTargetItem);
		def(pkState, "ClickChangeItemButton", lwClickChangeItemButton);
		def(pkState, "ClickItemButtonUpDown", lwClickItemButtonUpDown);
		def(pkState, "InitExchangeItemWnd", lwInitExchangeItemWnd);
		def(pkState, "GetDefenceFatigue", lwGetDefenceFatigue);
		def(pkState, "GetMaxDefenceFatigue", lwGetMaxDefenceFatigue);
		def(pkState, "CallDefenceFatigueWindow", lwCallDefenceFatigueWindow);
		def(pkState, "Send_PT_C_M_REQ_OPEN_DEFENCE_WINDOW", lwSend_PT_C_M_REQ_OPEN_DEFENCE_WINDOW);
		def(pkState, "Send_PT_C_M_REQ_ENTER_DEFENCE", lwSend_PT_C_M_REQ_ENTER_DEFENCE);
		def(pkState, "Send_PT_C_M_REQ_CANCLE_DEFENCE", lwSend_PT_C_M_REQ_CANCLE_DEFENCE);
		def(pkState, "UseRestoreItem", lwUseRestoreItem);
		def(pkState, "GetHideDamageStageNo", lwGetHideDamageStageNo);
		def(pkState, "GetCopyPointChargeCount", lwGetCopyPointChargeCount);
		def(pkState, "GetMaxCopyPointChargeCount", lwGetMaxCopyPointChargeCount);
		def(pkState, "Send_C_M_REQ_EXCHANGE_ACCUMPOINT_TO_STRATEGICPOINT", lwSend_C_M_REQ_EXCHANGE_ACCUMPOINT_TO_STRATEGICPOINT);
		def(pkState, "Send_PT_C_M_REQ_KILL_MONSTER_BY_ACCUMPOINT", lwSend_PT_C_M_REQ_KILL_MONSTER_BY_ACCUMPOINT);
		def(pkState, "SetMonsterKillButtonText", SetMonsterKillButtonText);
	}
	
	bool lwOnMouseUpChangeAntiqueIcon(lwUIWnd MaterialSelf)
	{
		if( !MaterialSelf.IsNil() && MaterialSelf.CursorToIcon() )
		{
			XUI::CXUI_Wnd* MaterialWnd = MaterialSelf.GetSelf();

			SItemPos ItemPos;
			MaterialWnd->GetCustomData(&ItemPos, sizeof(ItemPos));
			if( ItemPos == SItemPos::NullData() )
			{
				return false;
			}

			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pPlayer )
			{
				return false;
			}

			PgInventory* pInv = pPlayer->GetInven();
			if( !pInv )
			{
				return false;
			}

			PgBase_Item	Item;
			if( S_OK != pInv->GetItem(ItemPos, Item) )
			{
				MaterialWnd->ClearCustomData();
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
				return false;
			}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(Item.ItemNo());
			if(!pDef)
			{//아이템이 디비에 없다
				MaterialWnd->ClearCustomData();
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 460034, true);
				return false;
			}

			if( !IsCanChangeAntique( g_kGemStore.GetGemStoreType(), Item.ItemNo() ) )
			{//교환 가능한 아이템이 아니다
				MaterialWnd->ClearCustomData();
				XUI::CXUI_Wnd * const pParent = dynamic_cast<XUI::CXUI_Wnd*>(MaterialWnd->Parent());
				if( pParent )
				{
					XUI::CXUI_Wnd * const TargetWnd = dynamic_cast<XUI::CXUI_Wnd*>(pParent->GetControl(L"ICN_TARGET"));
					if( TargetWnd )
					{
						TargetWnd->ClearCustomData();
					}
				}
				int const iNo = (EGT_ANTIQUE == g_kGemStore.GetGemStoreType()) ? 750009 : 790544;
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", iNo, true);
				return false;
			}
			return true;
		}
		return false;
	}

	bool lwShowTargetAntiqueItem(lwUIWnd TopWnd)
	{//모든 과정을 통과하면 해당 아이템의 교환 가능한 아이템을 보여준다.
		XUI::CXUI_Wnd * const pWnd = (XUI::CXUI_Wnd*)TopWnd();
		if( NULL == pWnd )
		{
			return false;
		}
		
		XUI::CXUI_Icon * const MaterialWnd = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_MATERIAL"));
		XUI::CXUI_Icon * const TargetWnd = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_TARGET"));
		if( NULL == MaterialWnd || NULL == TargetWnd )
		{
			return false;
		}

		SItemPos ItemPos;
		MaterialWnd->GetCustomData(&ItemPos, sizeof(ItemPos));
		if( ItemPos == SItemPos::NullData() )
		{
			return false;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return false;
		}

		PgInventory* pInv = pPlayer->GetInven();
		if( !pInv )
		{
			return false;
		}

		PgBase_Item	Item;
		if( S_OK != pInv->GetItem(ItemPos, Item) )
		{
			MaterialWnd->ClearCustomData();
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
			return false;
		}

		TABLE_LOCK(CONT_DEFANTIQUE) kObjLock;
		g_kTblDataMgr.GetContDef(kObjLock);
		CONT_DEFANTIQUE const * pDef = kObjLock.Get();
		if( pDef )
		{
			int ChangeItemCount = 0;
			TBL_PAIR_KEY_INT const kKey(g_kGemStore.GetGemStoreType(),Item.ItemNo());
			CONT_DEFANTIQUE::const_iterator Total_iter = pDef->find(kKey);
			if( Total_iter != pDef->end() )
			{//교환가능한 아이템 목록중에서 내가 등록한 아이템과 같은 정보가 있다면
				ChangeItemCount = Total_iter->second.kContArticles.size();
			}
			XUI::CXUI_Form * const pBackGround = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"SFRM_COLOR_BG"));
			if( NULL == pBackGround )
			{
				return false;
			}
			XUI::CXUI_Form * const pHelpText = dynamic_cast<XUI::CXUI_Form*>(pBackGround->GetControl(L"FRM_HELP_TEXT"));
			if( NULL == pHelpText )
			{
				return false;
			}
			if( 0 == ChangeItemCount )
			{
				return false;
			}
			else if( 1 == ChangeItemCount )
			{
				int const iNo = (EGT_ANTIQUE == g_kGemStore.GetGemStoreType()) ? 750003 : 790543;
				pHelpText->Text(TTW(iNo));
			}
			else if( 1 < ChangeItemCount )
			{
				pHelpText->Text(TTW(750004));
			}

			XUI::CXUI_Button * const pBtnUp = dynamic_cast<XUI::CXUI_Button*>(pBackGround->GetControl(L"BTN_UP"));
			XUI::CXUI_Button * const pBtnDown = dynamic_cast<XUI::CXUI_Button*>(pBackGround->GetControl(L"BTN_DOWN"));
			if( pBtnUp || pBtnDown )
			{
				if( 1 == ChangeItemCount )
				{
					pBtnUp->Visible(false);
					pBtnDown->Visible(false);
				}
				else if( 1 < ChangeItemCount )
				{
					pBtnUp->Visible(true);
					pBtnDown->Visible(true);
				}
			}
			CONT_DEF_ANTIQUE_ARTICLE::const_iterator Target_iter = Total_iter->second.kContArticles.begin();
			DWORD TargetItemNo = Target_iter->second.first;
			TargetWnd->SetCustomData(&TargetItemNo, sizeof(TargetItemNo));
			TargetWnd->OwnerGuid(Item.Guid());
			return true;
		}
		return false;
	}

	void lwInitCollectAntiqueWnd(lwUIWnd TopWnd)
	{
		XUI::CXUI_Wnd * const pWnd = (XUI::CXUI_Wnd*)TopWnd();
		if( NULL == pWnd )
		{
			return;
		}
		
		XUI::CXUI_Icon * const MaterialWnd = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_MATERIAL"));
		XUI::CXUI_Icon * const TargetWnd = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_TARGET"));
		if( NULL == MaterialWnd || NULL == TargetWnd )
		{
			return;
		}

		MaterialWnd->ClearCustomData();
		TargetWnd->ClearCustomData();

		XUI::CXUI_Wnd * const pTitleBg = pWnd->GetControl(L"SFRM_ANTIQUE_TITLE");
		if( pTitleBg )
		{
			XUI::CXUI_Wnd * const pTitleText = pTitleBg->GetControl(L"IMG_TITLE_TAB");
			if( pTitleText )
			{
				int const iNo = (EGT_ANTIQUE == g_kGemStore.GetGemStoreType()) ? 750000 : 790542;
				pTitleText->Text(TTW(iNo));
			}
		}

		XUI::CXUI_Wnd * const pBackGround = pWnd->GetControl(L"SFRM_COLOR_BG");
		if( pBackGround )
		{
			XUI::CXUI_Wnd * const pHelpText = pBackGround->GetControl(L"FRM_HELP_TEXT");
			if( pHelpText )
			{
				int const iNo = (EGT_ANTIQUE == g_kGemStore.GetGemStoreType()) ? 750003 : 790543;
				pHelpText->Text(TTW(iNo));
			}
		}
	}

	bool IsCanChangeAntique(E_GEMSTORE_TYPE	const eGemStoreType, DWORD const & ItemNo)
	{
		TABLE_LOCK(CONT_DEFANTIQUE) kObjLock;
		g_kTblDataMgr.GetContDef(kObjLock);
		CONT_DEFANTIQUE const * pDef = kObjLock.Get();
		if( pDef )
		{
			TBL_PAIR_KEY_INT const kKey(eGemStoreType,ItemNo);
			CONT_DEFANTIQUE::const_iterator iter = pDef->find(kKey);
			if( iter == pDef->end() )
			{
				return false;
			}
			return true;
		}
		return false;
	}

	void lwClickChangeAntiqueButton(lwUIWnd lwSelf, bool const bConfirm)
	{
		XUI::CXUI_Wnd* pParent = lwSelf.GetSelf();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd * const pMaterial = pParent->GetControl(L"ICN_MATERIAL");
		XUI::CXUI_Wnd * const pTarget = pParent->GetControl(L"ICN_TARGET");
		if( !pMaterial || !pTarget )
		{
			return;
		}

		SItemPos MaterialPos;
		pMaterial->GetCustomData(&MaterialPos, sizeof(MaterialPos));
		if(MaterialPos == SItemPos::NullData())
		{//등록된게 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
			return;
		}

		DWORD TargetNo = 0;
		pTarget->GetCustomData(&TargetNo, sizeof(TargetNo));
		if(TargetNo == 0 )
		{//등록된게 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return;
		}

		PgInventory* pInv = pPlayer->GetInven();
		if( !pInv )
		{
			return;
		}

		PgBase_Item	MaterialItem;
		if( S_OK != pInv->GetItem(MaterialPos, MaterialItem) )
		{
			lwInitCollectAntiqueWnd(lwSelf);
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
			return;
		}

		TABLE_LOCK(CONT_DEFANTIQUE) kObjLock;
		g_kTblDataMgr.GetContDef(kObjLock);
		CONT_DEFANTIQUE const * pDef = kObjLock.Get();
		if( pDef )
		{
			int ChangeItemCount = 0;
			TBL_PAIR_KEY_INT const kKey(g_kGemStore.GetGemStoreType(),MaterialItem.ItemNo());
			CONT_DEFANTIQUE::const_iterator Total_iter = pDef->find(kKey);
			if( Total_iter != pDef->end() )
			{//교환가능한 아이템 목록중에서 내가 등록한 아이템과 같은 정보가 있다면
				ChangeItemCount = Total_iter->second.kContArticles.size();
			}
			if( 0 == ChangeItemCount )
			{
				return;
			}
			CONT_DEF_ANTIQUE_ARTICLE::const_iterator Target_iter = Total_iter->second.kContArticles.begin();
			while( Target_iter != Total_iter->second.kContArticles.end() )
			{
				DWORD TargetItemNo = Target_iter->second.first;
				if( TargetNo == TargetItemNo )
				{
					if( bConfirm )
					{
						BM::Stream Packet(PT_C_M_REQ_COLLECT_ANTIQUE);
						Packet.Push(pParent->OwnerGuid());
						Packet.Push(g_kGemStore.GetGemStoreType());
						Packet.Push(MaterialPos);
						Packet.Push(MaterialItem.ItemNo());
						Packet.Push(Target_iter->first);
						NETWORK_SEND(Packet);

						XUI::CXUI_Form * const pColorBox = dynamic_cast<XUI::CXUI_Form*>(pParent->GetControl(L"SFRM_COLOR_BG"));
						if( pColorBox )
						{
							XUI::CXUI_Button * pUpBtn = dynamic_cast<XUI::CXUI_Button*>(pColorBox->GetControl(L"BTN_UP"));
							XUI::CXUI_Button * pDwBtn = dynamic_cast<XUI::CXUI_Button*>(pColorBox->GetControl(L"BTN_DOWN"));
							if( pUpBtn && pDwBtn )
							{
								pUpBtn->Visible(false);
								pDwBtn->Visible(false);
							}
						}
					}
					else
					{
						BM::vstring vStr(TTW(750005));
						vStr += TTW(750006);
						wchar_t const * pItemName = NULL;
						GetDefString(TargetItemNo, pItemName);	// 아이템 이름
						if( pItemName )
						{
							vStr.Replace(L"#ITEMNAME#", pItemName);
						}
						CallYesNoMsgBox((std::wstring)vStr, MaterialItem.Guid(), MBT_CHANGE_ANTIQUE);
					}
				}
				++Target_iter;
			}
		}
	}

	void lwClickAntiqueButtonUpDown(lwUIWnd lwSelf, bool const IsUpButton)
	{
		XUI::CXUI_Wnd * pParent = lwSelf.GetSelf();
		if( NULL == pParent )
		{
			return;
		}

		XUI::CXUI_Wnd * const pMaterial = dynamic_cast<XUI::CXUI_Wnd*>(pParent->GetControl(L"ICN_MATERIAL"));
		XUI::CXUI_Wnd * const pTarget = dynamic_cast<XUI::CXUI_Wnd*>(pParent->GetControl(L"ICN_TARGET"));
		if( !pMaterial || !pTarget )
		{
			return;
		}

		SItemPos MaterialPos;
		pMaterial->GetCustomData(&MaterialPos, sizeof(MaterialPos));
		if(MaterialPos == SItemPos::NullData())
		{//등록된게 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
			return;
		}

		DWORD TargetNo = 0;
		pTarget->GetCustomData(&TargetNo, sizeof(TargetNo));
		if(TargetNo == 0 )
		{//등록된게 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return;
		}

		PgInventory* pInv = pPlayer->GetInven();
		if( !pInv )
		{
			return;
		}

		PgBase_Item	MaterialItem;
		if( S_OK != pInv->GetItem(MaterialPos, MaterialItem) )
		{
			lwInitCollectAntiqueWnd(lwSelf);
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
			return;
		}

		TABLE_LOCK(CONT_DEFANTIQUE) kObjLock;
		g_kTblDataMgr.GetContDef(kObjLock);
		CONT_DEFANTIQUE const * pDef = kObjLock.Get();
		if( pDef )
		{
			int ChangeItemCount = 0;
			TBL_PAIR_KEY_INT const kKey(g_kGemStore.GetGemStoreType(),MaterialItem.ItemNo());
			CONT_DEFANTIQUE::const_iterator Total_iter = pDef->find(kKey);
			if( Total_iter != pDef->end() )
			{//교환가능한 아이템 목록중에서 내가 등록한 아이템과 같은 정보가 있다면
				ChangeItemCount = Total_iter->second.kContArticles.size();
			}
			if( 2 > ChangeItemCount )
			{
				return;
			}
			CONT_DEF_ANTIQUE_ARTICLE::const_iterator Target_iter = Total_iter->second.kContArticles.begin();
			while( Target_iter != Total_iter->second.kContArticles.end() )
			{//교환가능한 아이템을 순회하면서
				DWORD TargetItemNo = Target_iter->second.first;
				if( TargetNo == TargetItemNo )
				{//현재 보여지는 아이템을 찾고
					bool bWndState = false;
					XUI::CXUI_Button * pUpBtn = NULL;
					XUI::CXUI_Button * pDwBtn = NULL;
					XUI::CXUI_Form * const pColorBox = dynamic_cast<XUI::CXUI_Form*>(pParent->GetControl(L"SFRM_COLOR_BG"));
					if( pColorBox )
					{
						pUpBtn = dynamic_cast<XUI::CXUI_Button*>(pColorBox->GetControl(L"BTN_UP"));
						pDwBtn = dynamic_cast<XUI::CXUI_Button*>(pColorBox->GetControl(L"BTN_DOWN"));
					}
					if( pUpBtn || pDwBtn )
					{
						bWndState = true;
					}
					if( IsUpButton )
					{//업 버튼을 눌렀다면 인덱스 증가
						if( bWndState )
						{//다운 버튼 활성화
							pDwBtn->Disable( false );
						}
						++Target_iter;
						if( Target_iter != Total_iter->second.kContArticles.end() )
						{//증가 된 인덱스가 컨테이너의 끝이 아닐 때만 다음 위치 검사
							++Target_iter;
							if( Target_iter == Total_iter->second.kContArticles.end() )
							{//다음 인덱스가 마지막이라면 업버튼 비활성화
								if( bWndState )
								{
									pUpBtn->Disable( true );
								}
							}
							else
							{
								if( bWndState )
								{
									pUpBtn->Disable( false );
								}
							}
							--Target_iter;
						}
						else
						{//컨테이너 끝이면 되돌린다.
							--Target_iter;
							if( bWndState )
							{
								pUpBtn->Disable( false );
							}
						}
					}
					else
					{//다운 버튼을 눌렀다면 인덱스 감소
						if( bWndState )
						{//업 버튼 활성화
							pUpBtn->Disable( false );
						}
						if( Target_iter != Total_iter->second.kContArticles.begin() )
						{//현재 인덱스가 시작이 아니면 인덱스 감소가 가능하다는 뜻이므로 감소시키고
							--Target_iter;
							if( Target_iter == Total_iter->second.kContArticles.begin() )
							{//감소된 인덱스가 컨테이너 처음이라면 감소가 불가능하므로
								if( bWndState )
								{//다운 버튼을 비활성화 시킨다.
									pDwBtn->Disable( true );
								}
							}
							else
							{//아니면 다운 버튼을 활성화 시킨다.
								if( bWndState )
								{
									pDwBtn->Disable( false );
								}
							}
						}
						else
						{
							if( bWndState )
							{
								pDwBtn->Disable( true );
							}
						}
					}
					DWORD TargetItemNo = Target_iter->second.first;
					pTarget->SetCustomData(&TargetItemNo, sizeof(TargetItemNo));
					pTarget->OwnerGuid(MaterialItem.Guid());
					return;
				}
				++Target_iter;
			}
		}
	}

	bool lwOnMouseUpChangeIcon(lwUIWnd MaterialSelf, int const Menu)
	{
		if( !MaterialSelf.IsNil() && MaterialSelf.CursorToIcon() )
		{
			XUI::CXUI_Wnd* MaterialWnd = MaterialSelf.GetSelf();

			SItemPos ItemPos;
			MaterialWnd->GetCustomData(&ItemPos, sizeof(ItemPos));
			if( ItemPos == SItemPos::NullData() )
			{
				return false;
			}

			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pPlayer )
			{
				return false;
			}

			PgInventory* pInv = pPlayer->GetInven();
			if( !pInv )
			{
				return false;
			}

			PgBase_Item	Item;
			if( S_OK != pInv->GetItem(ItemPos, Item) )
			{
				MaterialWnd->ClearCustomData();
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
				return false;
			}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(Item.ItemNo());
			if(!pDef)
			{//아이템이 디비에 없다
				MaterialWnd->ClearCustomData();
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 460034, true);
				return false;
			}

			XUI::CXUI_Wnd * const pParent = dynamic_cast<XUI::CXUI_Wnd*>(MaterialWnd->Parent());
			if( NULL == pParent )
			{
				return false;
			}
			
			XUI::CXUI_Wnd * const TargetWnd = dynamic_cast<XUI::CXUI_Wnd*>(pParent->GetControl(L"ICN_TARGET"));
			if( NULL == TargetWnd )
			{
				return false;
			}

			DWORD TargetItemNo = 0;
			if( !(IsCanExchangeItem( Item.ItemNo(), TargetItemNo, Menu )) )
			{//교환 가능한 아이템이 아니다
				MaterialWnd->ClearCustomData();
				TargetWnd->ClearCustomData();

				int InitTextNo = 0;
				int WarnTextNo = 0;
				for( int FormCount = 0; FormCount < ExchangeFormMaxSize; ++FormCount )
				{
					if( XUIMgr.Get( szExchangeUI[FormCount].szFrom ) )
					{
						InitTextNo = szExchangeUI[FormCount].ContentMsgNo;
						WarnTextNo = szExchangeUI[FormCount].ErrorMsgNo;
						break;
					}
				}
				
				XUI::CXUI_Form * const pBackGround = dynamic_cast<XUI::CXUI_Form*>(pParent->GetControl(L"SFRM_COLOR_BG"));
				if( pBackGround )
				{
					XUI::CXUI_Form * const pHelpText = dynamic_cast<XUI::CXUI_Form*>(pBackGround->GetControl(L"FRM_HELP_TEXT"));
					if( pHelpText )
					{
						if( InitTextNo )
						{
							pHelpText->Text(TTW(InitTextNo));
						}
					}
				}
				
				if( WarnTextNo )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", WarnTextNo, true);
				}
				return false;
			}
			else
			{
				TargetWnd->SetCustomData(&TargetItemNo, sizeof(TargetItemNo));
				return true;
			}
		}
		return false;
	}

	bool lwShowTargetItem(lwUIWnd TopWnd, int const Menu)
	{//모든 과정을 통과하면 해당 아이템의 교환 가능한 아이템을 보여준다.
		XUI::CXUI_Wnd * const pWnd = (XUI::CXUI_Wnd*)TopWnd();
		if( NULL == pWnd )
		{
			return false;
		}
		
		XUI::CXUI_Icon * const MaterialWnd = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_MATERIAL"));
		XUI::CXUI_Icon * const TargetWnd = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_TARGET"));
		if( NULL == MaterialWnd || NULL == TargetWnd )
		{
			return false;
		}

		SItemPos ItemPos;
		MaterialWnd->GetCustomData(&ItemPos, sizeof(ItemPos));
		if( ItemPos == SItemPos::NullData() )
		{
			return false;
		}

		DWORD TargetNo = 0;
		TargetWnd->GetCustomData(&TargetNo, sizeof(TargetNo));
		if(TargetNo == 0 )
		{//등록된게 없다
			return false;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return false;
		}

		PgInventory* pInv = pPlayer->GetInven();
		if( !pInv )
		{
			return false;
		}

		PgBase_Item	Item;
		if( S_OK != pInv->GetItem(ItemPos, Item) )
		{
			lwInitExchangeItemWnd(TopWnd);
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
			return false;
		}

		DWORD NeedItemCnt = 0;
		DWORD TargetItemCnt = 0;
		if ( IsCanExchangeItem(Item.ItemNo(), TargetNo, NeedItemCnt, TargetItemCnt, Menu) )
		{
			int HaveCount = pInv->GetInvTotalCount(Item.ItemNo());
			XUI::CXUI_Button * pOk = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_OK"));
			if( pOk )
			{
				if( HaveCount < NeedItemCnt ) // 보유한 양이 필요 한 양보다 작으면.
				{
					pOk->Disable(true);
				}
				else
				{
					pOk->Disable(false);
				}
			}

			XUI::CXUI_Form * const pBackGround = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"SFRM_COLOR_BG"));
			if( NULL == pBackGround )
			{
				return false;
			}
			XUI::CXUI_Form * const pHelpText = dynamic_cast<XUI::CXUI_Form*>(pBackGround->GetControl(L"FRM_HELP_TEXT"));
			if( NULL == pHelpText )
			{
				return false;
			}

			BM::vstring vItemStr;

			CONT_DEFGEMSTORE const * pDef = NULL;
			g_kTblDataMgr.GetContDef(pDef);
			if( pDef )
			{
				std::map<int, int> ContAbleChangeItem;
				GetAbleChangeItem(pDef, Item.ItemNo(), Menu, ContAbleChangeItem);
				if( 2 <= ContAbleChangeItem.size() )
				{
					vItemStr = TTW(750004);
					vItemStr += L"\n";
				}
			}
			
			vItemStr += TTW(750005);
			wchar_t const * pItemName = NULL;
			GetDefString(TargetNo, pItemName);	// 아이템 이름
			if( pItemName )
			{
				vItemStr.Replace(L"#ITEMNAME#", pItemName);
			}

			for( int FormCount = 0; FormCount < ExchangeFormMaxSize; ++FormCount )
			{
				if( XUIMgr.Get( szExchangeUI[FormCount].szFrom ) )
				{
					switch( szExchangeUI[FormCount].ResultTextType )
					{
					case EEIResultTT::EIRTT_TARGET_CNT:
						{		
							vItemStr += L"{C=0xFFB4100F/}";
							vItemStr += L" X ";
							vItemStr += TargetItemCnt;
						}break;
					case EEIResultTT::EIRTT_MATERIAL_CNT:
						{		
							BM::vstring vNumStr(TTW(759902));
							vNumStr.Replace(L"#NUM#", pInv->GetTotalCount(Item.ItemNo()));
							vNumStr.Replace(L"#MAX#", NeedItemCnt);
							vItemStr += L" ";
							vItemStr += vNumStr;
						}break;
					}
					break;
				}
			}
			pHelpText->Text(vItemStr);
			return true;
		}
		return false;
	}

	void lwClickChangeItemButton(lwUIWnd lwSelf, bool const bConfirm, int const Menu)
	{
		XUI::CXUI_Wnd* pParent = lwSelf.GetSelf();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd * const pMaterial = pParent->GetControl(L"ICN_MATERIAL");
		XUI::CXUI_Wnd * const pTarget = pParent->GetControl(L"ICN_TARGET");
		if( !pMaterial || !pTarget )
		{
			return;
		}

		SItemPos MaterialPos;
		pMaterial->GetCustomData(&MaterialPos, sizeof(MaterialPos));
		if(MaterialPos == SItemPos::NullData())
		{//등록된게 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
			return;
		}

		DWORD TargetNo = 0;
		pTarget->GetCustomData(&TargetNo, sizeof(TargetNo));
		if(TargetNo == 0 )
		{//등록된게 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return;
		}

		PgInventory* pInv = pPlayer->GetInven();
		if( !pInv )
		{
			return;
		}

		PgBase_Item	MaterialItem;
		if( S_OK != pInv->GetItem(MaterialPos, MaterialItem) )
		{
			lwInitExchangeItemWnd(lwSelf);
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
			return;
		}
		
		if( bConfirm )
		{
			BM::Stream Packet(PT_C_M_REQ_EXCHANGE_GEMSTORE);
			Packet.Push(Menu);
			Packet.Push(MaterialPos);
			Packet.Push(TargetNo);
			NETWORK_SEND(Packet);

			XUI::CXUI_Form * const pColorBox = dynamic_cast<XUI::CXUI_Form*>(pParent->GetControl(L"SFRM_COLOR_BG"));
			if( pColorBox )
			{
				XUI::CXUI_Button * pUpBtn = dynamic_cast<XUI::CXUI_Button*>(pColorBox->GetControl(L"BTN_UP"));
				XUI::CXUI_Button * pDwBtn = dynamic_cast<XUI::CXUI_Button*>(pColorBox->GetControl(L"BTN_DOWN"));
				if( pUpBtn && pDwBtn )
				{
					pUpBtn->Visible(false);
					pDwBtn->Visible(false);
				}
			}
		}
		else
		{
			BM::vstring vStr(TTW(750005));
			vStr += TTW(750006);
			wchar_t const * pItemName = NULL;
			GetDefString(TargetNo, pItemName);	// 아이템 이름
			if( pItemName )
			{
				vStr.Replace(L"#ITEMNAME#", pItemName);
			}
			CallYesNoMsgBox((std::wstring)vStr, MaterialItem.Guid(), MBT_EXCHANGE_GEMSTORE);
		}
	}

	void lwClickItemButtonUpDown(lwUIWnd lwSelf, int const Menu, bool const IsUpButton) // 교환할 아이템 선택
	{
		XUI::CXUI_Wnd * pParent = lwSelf.GetSelf();
		if( NULL == pParent )
		{
			return;
		}

		XUI::CXUI_Wnd * const pMaterial = dynamic_cast<XUI::CXUI_Wnd*>(pParent->GetControl(L"ICN_MATERIAL"));
		XUI::CXUI_Wnd * const pTarget = dynamic_cast<XUI::CXUI_Wnd*>(pParent->GetControl(L"ICN_TARGET"));
		if( !pMaterial || !pTarget )
		{
			return;
		}

		XUI::CXUI_Form * const pColorBox = dynamic_cast<XUI::CXUI_Form*>(pParent->GetControl(L"SFRM_COLOR_BG"));
		if( NULL == pColorBox )
		{
			return;
		}
		
		XUI::CXUI_Button * pUpBtn = dynamic_cast<XUI::CXUI_Button*>(pColorBox->GetControl(L"BTN_UP"));
		XUI::CXUI_Button * pDwBtn = dynamic_cast<XUI::CXUI_Button*>(pColorBox->GetControl(L"BTN_DOWN"));
		if( (NULL == pUpBtn) || (NULL == pDwBtn) )
		{
			return;
		}

		pUpBtn->Visible(false);
		pDwBtn->Visible(false);

		SItemPos MaterialPos;
		pMaterial->GetCustomData(&MaterialPos, sizeof(MaterialPos));
		if(MaterialPos == SItemPos::NullData())
		{//등록된게 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
			return;
		}

		DWORD TargetNo = 0;
		pTarget->GetCustomData(&TargetNo, sizeof(TargetNo));
		if(TargetNo == 0 )
		{//등록된게 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return;
		}

		PgInventory* pInv = pPlayer->GetInven();
		if( !pInv )
		{
			return;
		}

		PgBase_Item	MaterialItem;
		if( S_OK != pInv->GetItem(MaterialPos, MaterialItem) )
		{
			lwInitExchangeItemWnd(lwSelf);
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
			return;
		}

		CONT_DEFGEMSTORE const * pDef = NULL;
		g_kTblDataMgr.GetContDef(pDef);
		if( NULL == pDef )
		{
			return;
		}

		std::map<int, int> ContAbleChangeItem;
		GetAbleChangeItem(pDef, MaterialItem.ItemNo(), Menu, ContAbleChangeItem);
		if( 2 > ContAbleChangeItem.size() )
		{
			return;
		}

		pUpBtn->Visible(true);
		pDwBtn->Visible(true);

		std::map<int, int>::const_iterator Target_iter = ContAbleChangeItem.begin();
		while( Target_iter != ContAbleChangeItem.end() )
		{//교환가능한 아이템을 순회하면서
			DWORD TargetItemNo = Target_iter->first;
			if( TargetNo == TargetItemNo )
			{//현재 보여지는 아이템을 찾고
				if( IsUpButton )
				{//업 버튼을 눌렀다면 인덱스 증가
					pDwBtn->Disable( false ); //다운 버튼 활성화
					++Target_iter;
					if( Target_iter != ContAbleChangeItem.end() )
					{//증가 된 인덱스가 컨테이너의 끝이 아닐 때만 다음 위치 검사
						++Target_iter;
						if( Target_iter == ContAbleChangeItem.end() )
						{//다음 인덱스가 마지막이라면 업버튼 비활성화
							pUpBtn->Disable( true );
						}
						else
						{
							pUpBtn->Disable( false );
						}
						--Target_iter;
					}
					else
					{//컨테이너 끝이면 되돌린다.
						--Target_iter;
						pUpBtn->Disable( false );
					}
				}
				else
				{//다운 버튼을 눌렀다면 인덱스 감소
					pUpBtn->Disable( false ); //업 버튼 활성화
					if( Target_iter != ContAbleChangeItem.begin() )
					{//현재 인덱스가 시작이 아니면 인덱스 감소가 가능하다는 뜻이므로 감소시키고
						--Target_iter;
						if( Target_iter == ContAbleChangeItem.begin() )
						{//감소된 인덱스가 컨테이너 처음이라면 감소가 불가능하므로 다운 버튼을 비활성화 시킨다.
							pDwBtn->Disable( true );
						}
						else
						{//아니면 다운 버튼을 활성화 시킨다.
							pDwBtn->Disable( false );
						}
					}
					else
					{
						pDwBtn->Disable( true );
					}
				}
				DWORD TargetItemNo = Target_iter->first;
				pTarget->SetCustomData(&TargetItemNo, sizeof(TargetItemNo));
				pTarget->OwnerGuid(MaterialItem.Guid());
				return;
			}
			++Target_iter;
		}
	}

	void GetAbleChangeItem(CONT_DEFGEMSTORE const * pDef, int const MaterialItemNo, int const Menu, std::map<int, int> & ContAbleChangeItem )
	{
		if( NULL == pDef )
		{
			return;
		}

		ContAbleChangeItem.clear();

		CONT_DEFGEMSTORE::const_iterator iter = pDef->begin();
		for( ; iter != pDef->end(); ++iter )
		{
			CONT_DEFGEMSTORE_ARTICLE::const_iterator SubIter = iter->second.kContArticles.begin();
			for( ; SubIter != iter->second.kContArticles.end(); ++SubIter )
			{
				if( Menu == SubIter->first.iMenu )
				{
					CONT_DEFGEMS::const_iterator GemIter = SubIter->second.kContGems.begin();
					for( ; GemIter != SubIter->second.kContGems.end(); ++GemIter )
					{
						if( MaterialItemNo == GemIter->iGemNo )
						{
							ContAbleChangeItem.insert( std::make_pair(SubIter->first.iItemNo, MaterialItemNo) );
						}
					}
				}
			}
		}
	}

	void lwInitExchangeItemWnd(lwUIWnd TopWnd)
	{
		XUI::CXUI_Wnd * const pWnd = (XUI::CXUI_Wnd*)TopWnd();
		if( NULL == pWnd )
		{
			return;
		}
		
		XUI::CXUI_Icon * const MaterialWnd = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_MATERIAL"));
		XUI::CXUI_Icon * const TargetWnd = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_TARGET"));
		if( NULL == MaterialWnd || NULL == TargetWnd )
		{
			return;
		}

		MaterialWnd->ClearCustomData();
		TargetWnd->ClearCustomData();

		XUI::CXUI_Form * const pBackGround = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"SFRM_COLOR_BG"));
		if( NULL == pBackGround )
		{
			return;
		}
		XUI::CXUI_Form * const pHelpText = dynamic_cast<XUI::CXUI_Form*>(pBackGround->GetControl(L"FRM_HELP_TEXT"));
		if( NULL == pHelpText )
		{
			return;
		}

		for( int FormCount = 0; FormCount < ExchangeFormMaxSize; ++FormCount )
		{
			if( XUIMgr.Get( szExchangeUI[FormCount].szFrom ) )
			{
				pHelpText->Text(TTW(szExchangeUI[FormCount].ContentMsgNo));
				break;
			}
		}
	}

	bool IsCanExchangeItem(DWORD const & MaterialItemNo, int const Menu)
	{
		CONT_DEFGEMSTORE const * pDef = NULL;
		g_kTblDataMgr.GetContDef(pDef);
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
						CONT_DEFGEMS::const_iterator GemIter = SubIter->second.kContGems.begin();
						for( ; GemIter != SubIter->second.kContGems.end(); ++GemIter )
						{
							if( MaterialItemNo == GemIter->iGemNo )
							{ // DB에 해당 아이템이 존재하면
								return true;
							}
						}
					}
				}
			}
		}
		return false;
	}
	
	bool IsCanExchangeItem(DWORD const & MaterialItemNo, DWORD & TargetItemNo, int const Menu)
	{
		CONT_DEFGEMSTORE const * pDef = NULL;
		g_kTblDataMgr.GetContDef(pDef);
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
						CONT_DEFGEMS::const_iterator GemIter = SubIter->second.kContGems.begin();
						for( ; GemIter != SubIter->second.kContGems.end(); ++GemIter )
						{
							if( MaterialItemNo == GemIter->iGemNo )
							{ // DB에 해당 아이템이 존재하면
								TargetItemNo = SubIter->first.iItemNo; // 첫번째 교환 아이템
								return true;
							}
						}
					}
				}
			}
		}
		return false;
	}

	bool IsCanExchangeItem(CONT_DEFGEMSTORE const * pDef, DWORD const & MaterialItemNo, int const Menu)
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
						CONT_DEFGEMS::const_iterator GemIter = SubIter->second.kContGems.begin();
						for( ; GemIter != SubIter->second.kContGems.end(); ++GemIter )
						{
							if( MaterialItemNo == GemIter->iGemNo )
							{ // DB에 해당 아이템이 존재하면
								return true;
							}
						}
					}
				}
			}
		}
		return false;
	}

	bool IsCanExchangeItem(DWORD const & MaterialItemNo, DWORD const & TargetItemNo, DWORD & NeedItemCnt, DWORD & TargetItemCnt, int const Menu)
	{
		CONT_DEFGEMSTORE const * pDef = NULL;
		g_kTblDataMgr.GetContDef(pDef);
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
						CONT_DEFGEMS::const_iterator GemIter = SubIter->second.kContGems.begin();
						for( ; GemIter != SubIter->second.kContGems.end(); ++GemIter )
						{
							if( MaterialItemNo == GemIter->iGemNo )
							{ // DB에 해당 아이템이 존재하면	
								if( TargetItemNo == SubIter->first.iItemNo )
								{
									TargetItemCnt = SubIter->second.iItemCount;
									NeedItemCnt = GemIter->sCount;
									return true;
								}
							}
						}
					}
				}
			}
		}
		return false;
	}

	int lwGetDefenceFatigue()
	{
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return 0;
		}
		int const MyFatigue = pPlayer->GetAbil(AT_STRATEGY_FATIGABILITY);
		return MyFatigue;
	}
	
	int lwGetMaxDefenceFatigue()
	{
		SSyncVariable SyncVarialbe = g_kEventView.VariableCont();
		int const MaxStrategyFatigability = SyncVarialbe.iMaxStrategyFatigability;
		return MaxStrategyFatigability;
	}

	lwWString lwCallDefenceFatigueWindow(lwUIWnd lwWnd)
	{
		std::wstring wStrText;
		XUI::CXUI_Wnd * const pWnd = lwWnd();
		if( NULL == pWnd )
		{
			return lwWString(wStrText);
		}

		BM::vstring vStr(L"FRM_EXP_TEXT");

		for( int index = 0; ; ++index )
		{
			vStr += index;
			XUI::CXUI_Form * const pText = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(static_cast<std::wstring>(vStr)));
			if( !pText )
			{
				break;
			}
			pText->FontColor(0xFF959595);
		}

		int const CurStrategyFatigability = lwGetDefenceFatigue();
		int iRate = (int)(((float)CurStrategyFatigability / (float)COE_DEFENCEMODE_EXP) * 10);
		
		vStr = L"FRM_EXP_TEXT";
		vStr += iRate;

		XUI::CXUI_Form * const pText1 = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"FRM_FATIGUE_TEXT"));
		XUI::CXUI_Form * const pText2 = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(static_cast<std::wstring>(vStr)));
		if( !pText1 || !pText2 )
		{
			return lwWString(wStrText);
		}

		vStr = TTW(750022);
		wStrText = pText2->Text();
		std::wstring::size_type Pos = wStrText.find('%');
		if(std::wstring::npos != Pos )
		{
			wStrText.erase(Pos);
		}
		vStr.Replace(L"#FP#", CurStrategyFatigability);
		vStr.Replace(L"#PER#", wStrText);
		pText1->Text(static_cast<std::wstring>(vStr));

		pText2->FontColor(0xFFFFFFFF);
		wStrText = wStrText + L"%";
		pText2->Text(wStrText);

		return lwWString(wStrText);
	}

	void lwSend_PT_C_M_REQ_OPEN_DEFENCE_WINDOW()
	{
		BM::Stream Packet(PT_C_M_REQ_OPEN_DEFENCE_WINDOW);

		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"FRM_CREATE_DEFENCE");
		if( NULL == pWnd )
		{
			return;
		}

		if( PgClientPartyUtil::IsInParty() && PgClientPartyUtil::IsPartyMaster(pPlayer->GetID()) )
		{
			NETWORK_SEND(Packet);
		}
	}

	void lwSend_PT_C_M_REQ_ENTER_DEFENCE()
	{
		BM::Stream Packet(PT_C_M_REQ_ENTER_DEFENCE);
		NETWORK_SEND(Packet);
	}

	void lwSend_PT_C_M_REQ_CANCLE_DEFENCE()
	{
		BM::Stream Packet(PT_C_M_REQ_CANCLE_DEFENCE);
		NETWORK_SEND(Packet);
	}

	void lwUseRestoreItem(lwUIWnd lwWnd)
	{
		XUI::CXUI_Wnd * pWnd = lwWnd();
		if( NULL == pWnd )
		{
			return;
		}

		XUI::CXUI_Icon * pIcon = dynamic_cast<XUI::CXUI_Icon*>(pWnd->GetControl(L"ICN_ITEM"));
		int ItemNo = 0;
		pIcon->GetCustomData(&ItemNo, sizeof(ItemNo));

		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		PgInventory * pInven = pPlayer->GetInven();
		if( NULL == pInven )
		{
			return;
		}

		int const HaveNo = pInven->GetTotalCount(ItemNo);
		if( HaveNo > 0 )
		{
			SItemPos ItemPos;
			pInven->GetFirstItem(ItemNo, ItemPos);

			PgBase_Item Item;
			if( S_OK == pInven->GetItem(ItemPos, Item) )
			{			
				BM::Stream Packet(PT_C_M_REQ_ITEM_ACTION, ItemPos);
				Packet.Push(lwGetServerElapsedTime32());
				NETWORK_SEND(Packet)
			}
		}
		else
		{
			std::wstring const& Message = TTW(750039);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
		}
	}

	int lwGetHideDamageStageNo()
	{
		return g_iF7DamageHideStage;
	}

	int lwGetCopyPointChargeCount()
	{
		return StrategicPointChargeCount;
	}

	int lwGetMaxCopyPointChargeCount()
	{
		return MAX_CHARGE_COPY_MACHINE;
	}

	void lwSend_C_M_REQ_EXCHANGE_ACCUMPOINT_TO_STRATEGICPOINT()
	{
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( PgClientPartyUtil::IsInParty() )
		{ // 파티에 속해있으면
			if( false == PgClientPartyUtil::IsPartyMaster(pPlayer->GetID()) )
			{ // 파티장이 아니면 요청 불가
				return;
			}
		}

		BM::vstring Message( TTW(750033) );
		Message.Replace(L"#NUM#", MAX_CHARGE_COPY_MACHINE - StrategicPointChargeCount);
		CallYesNoMsgBox(Message, BM::GUID::NullData(), MBT_EXCHANGE_ACCUMPOINT_TO_STRATEGICPOINT);
	}

	void lwSend_PT_C_M_REQ_KILL_MONSTER_BY_ACCUMPOINT()
	{
		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( PgClientPartyUtil::IsInParty() )
		{	// 파티에 속해 있으면.
			if( false == PgClientPartyUtil::IsPartyMaster(pPlayer->GetID()) )
			{ // 파티장이 아니면 요청 불가.
				return;
			}
		}

		BM::vstring Message( TTW(750044) );
		Message.Replace(L"#NUM#", KillableMonsterCount);
		CallYesNoMsgBox(Message, BM::GUID::NullData(), MBT_KILL_MONSTER_BY_POINT_COPY);
	}

	void SetMonsterKillButtonText(int Num)
	{
		BM::vstring KillCount( TTW(750045) );
		KillCount.Replace(L"#NUM#", Num);

		XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"FRM_POINT_COPY_MACHINE");
		if( pWnd )
		{
			XUI::CXUI_Button * pBtn = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"FRM_KILL_MONSTER"));
			if( pBtn )
			{
				pBtn->Text( std::wstring(KillCount) );
			}
		}
	}

	void SetUVIndextMarbleForm(int const Count)
	{
		if( Count > MAX_CHARGE_COPY_MACHINE )
		{// 최대스테이지를 벗어나면 구슬을 증가시키지 않는다.
			return;
		}
		XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"FRM_POINT_COPY_MACHINE");
		if( pWnd )
		{
			XUI::CXUI_Form * pMarbleForm = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"FRM_MARBLE"));
			if( pMarbleForm )
			{
				SUVInfo UVInfo = pMarbleForm->UVInfo();
				if( Count )
				{
					UVInfo.Index = Count + 15 - MAX_CHARGE_COPY_MACHINE;
				}
				else
				{
					UVInfo.Index = 1;
				}
				pMarbleForm->UVInfo(UVInfo);
				pMarbleForm->SetInvalidate();
			}
		}
	}

	void SetPoint(std::wstring & FormName, int const Point)
	{
		if( Point > 999999 )
		{// UI가 최대 6자리 이므로 100만 이상이면 안됨.
			return;
		}
		XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"FRM_POINT_COPY_MACHINE");
		if( pWnd )
		{
			XUI::CXUI_Form * pPoint = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(FormName));
			if( pPoint )
			{// Form에 바뀐 값을 넣고 업데이트 스크립트를 Call한다.
				pPoint->SetCustomData(&Point, sizeof(Point));
				lua_tinker::call<void, lwUIWnd>("UpdateCopyPointNumber", lwUIWnd(pPoint));
			}
		}
	}

	void OnOffExchangeButton(bool bValue)
	{
		XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"FRM_POINT_COPY_MACHINE");
		if( pWnd )
		{
			XUI::CXUI_Button * pBtn = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"FRM_POINT_EXCHANGE"));
			if( pBtn )
			{
				pBtn->Disable(!bValue);
			}

			XUI::CXUI_Button * pKillBtn = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"FRM_KILL_MONSTER"));
			if( pKillBtn )
			{
				if( false == bValue )
				{
					KillableMonsterCount = 0;
				}
				pKillBtn->Disable(!bValue);
			}
		}
	}

	void RecvDefenceMode_Command(WORD const wPacketType, BM::Stream& Packet)
	{
		switch( wPacketType )
		{
		case PT_M_C_ANS_COLLECT_ANTIQUE:
			{
				HRESULT hResult = 0;
				Packet.Pop(hResult);
				if( S_OK == hResult )
				{
					DWORD TargetItemNo = 0;
					Packet.Pop(TargetItemNo);
					BM::vstring vStr;
					wchar_t const * pItemName = NULL;
					GetDefString(TargetItemNo, pItemName);	// 아이템 이름
					if( pItemName )
					{
						vStr = L"[";
						vStr += pItemName;
						vStr += L"]";
						vStr += TTW(750007);

						std::wstring const& Message = (std::wstring)vStr;
						SChatLog ChatLog(CT_EVENT);
						g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
					}
					XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"SFRM_COLLECT_ANTIQUE");
					if( pWnd )
					{
						lwInitCollectAntiqueWnd(lwUIWnd(pWnd));
					}
				}
				else
				{
					if( E_NOT_FOUND_ANTIQUE == hResult )
					{//골동품을 찾을수 없음
						lwAddWarnDataTT(1261);
					}
					else if( E_NOT_FOUND_ARTICLE == hResult )
					{//결과 아이템을 찾을 수 없음
						lwAddWarnDataTT(671);
					}
					else if( E_NOT_ENOUGH_GEMS == hResult )
					{//필요한 재료아이템 수량이 부족
						lwAddWarnDataTT(1704);
					}
				}
			}break;
		case PT_M_C_ANS_EXCHANGE_GEMSTORE:
			{
				HRESULT hResult = 0;
				Packet.Pop(hResult);
				if( S_OK == hResult )
				{
					DWORD TargetItemNo = 0;
					Packet.Pop(TargetItemNo);
					BM::vstring vStr;
					wchar_t const * pItemName = NULL;
					GetDefString(TargetItemNo, pItemName);	// 아이템 이름
					if( pItemName )
					{
						vStr = L"[";
						vStr += pItemName;
						vStr += L"]";
						vStr += TTW(750007);

						std::wstring const& Message = (std::wstring)vStr;
						SChatLog ChatLog(CT_EVENT);
						g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
					}
					
					XUI::CXUI_Wnd * pWnd = NULL;
					for( int FormCount = 0; FormCount < ExchangeFormMaxSize; ++FormCount )
					{
						pWnd = XUIMgr.Get( szExchangeUI[FormCount].szFrom );
						if( pWnd )
						{
							lwInitExchangeItemWnd(lwUIWnd(pWnd));
							break;
						}
					}
				}
				else
				{
					if( E_NOT_FOUND_ANTIQUE == hResult )
					{//소켓카드를 찾을수 없음
						lwAddWarnDataTT(1261);
					}
					else if( E_NOT_FOUND_ARTICLE == hResult )
					{//결과 아이템을 찾을 수 없음
						lwAddWarnDataTT(671);
					}
					else if( E_NOT_ENOUGH_GEMS == hResult )
					{//필요한 재료아이템 수량이 부족
						lwAddWarnDataTT(1704);
					}
				}
			}break;
		case PT_M_C_NFY_OPEN_DEFENCE_WINDOW:
			{
				lua_tinker::call<void>("CallDefenceModEnter");
			}break;
		case PT_M_C_NFY_ENTER_DEFENCE:
			{
				lua_tinker::call<void>("InitDefenceMode7DelayTime");
			}break;
		case PT_M_C_NFY_CANCLE_DEFENCE:
			{
				std::wstring const& Message = TTW(750040);
				SChatLog ChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);

				lua_tinker::call<void>("InitF7EnterTime");	// 타이머 초기화 및 동작 중지
			}break;
		case PT_M_C_NFY_USE_POINT_COPY_ITEM:
			{//포인트복제기 아이템 사용 알림
				std::wstring CharName;
				int NowPoint = 0;
				int NextPoint = 0;
				Packet.Pop(CharName);
				Packet.Pop(StrategicPointChargeCount);
				Packet.Pop(NowPoint);
				Packet.Pop(NextPoint);
				Packet.Pop(KillableMonsterCount);

				std::wstring const& Message = CharName + TTW(750031);
				SChatLog ChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);

				SetUVIndextMarbleForm(StrategicPointChargeCount);
				SetPoint(std::wstring(L"FRM_CURRENT_POINT"), NowPoint);
				SetPoint(std::wstring(L"FRM_NEXT_POINT"), NextPoint);
				SetMonsterKillButtonText(KillableMonsterCount);

				OnOffExchangeButton(true); // 전략포인트 전환버튼 활성화

				lua_tinker::call<void, bool>("SetUsingCopyMachine", true);
				lua_tinker::call<void>("StartAnimateCopyMax");
			}break;
		case PT_M_C_NFY_ACCUMPOINT_TO_STRATEGICPOINT:
			{//누적포인트가 전략포인트로 전환됨을 알림
				int Point = 0;
				Packet.Pop(Point);

				std::wstring Message(TTW(750034));
				g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), Message, true);

				BM::vstring vStr(TTW(401151));
				vStr.Replace(L"#POINT#", Point);
				g_kChatMgrClient.AddLogMessage(SChatLog(CT_NOTICE1), vStr, true);

				SetUVIndextMarbleForm(0); // 0이면 초기화
				SetPoint(std::wstring(L"FRM_CURRENT_POINT"), 0);
				SetPoint(std::wstring(L"FRM_NEXT_POINT"), 0);
				StrategicPointChargeCount = 0;
				KillableMonsterCount = 0;

				SetMonsterKillButtonText(KillableMonsterCount);

				OnOffExchangeButton(false); // 전략포인트 전환버튼 비활성화

				lua_tinker::call<void, bool>("SetUsingCopyMachine", false);
				lua_tinker::call<void>("StartAnimateCopyMax");
			}break;
		case PT_M_C_NFY_KILL_MONSTER_BY_ACCUMPOINT:
			{
				SetUVIndextMarbleForm(0); // 0이면 초기화
				SetPoint(std::wstring(L"FRM_CURRENT_POINT"), 0);
				SetPoint(std::wstring(L"FRM_NEXT_POINT"), 0);
				StrategicPointChargeCount = 0;
				KillableMonsterCount = 0;

				SetMonsterKillButtonText(KillableMonsterCount);

				OnOffExchangeButton(false); // 전략포인트 전환버튼 비활성화

				lua_tinker::call<void, bool>("SetUsingCopyMachine", false);
				lua_tinker::call<void>("StartAnimateCopyMax");
			}break;
		case PT_M_C_NFY_CURRENT_ACCUMPOINT:
			{//현재 누적포인트 알림
				int NowPoint = 0;
				int NextPoint = 0;
				Packet.Pop(StrategicPointChargeCount);
				Packet.Pop(NowPoint);
				Packet.Pop(NextPoint);
				Packet.Pop(KillableMonsterCount);

				SetUVIndextMarbleForm(StrategicPointChargeCount);
				SetPoint(std::wstring(L"FRM_CURRENT_POINT"), NowPoint);
				SetPoint(std::wstring(L"FRM_NEXT_POINT"), NextPoint);
				SetMonsterKillButtonText(KillableMonsterCount);

				lua_tinker::call<void>("StartAnimateCopyMax");
			}break;
		default:
			{
				//None Def Packet Type
			}break;
		}
	}
};
