#include "StdAfx.h"
#include "lwSummon_Info.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgActor.h"
#include "Variant/PgSummoned.h"
#include "PgResourceIcon.h"
#include "CreateUsingNiNew.inl"
#include "PgUISound.h"
#include "lwUIToolTip.h"
#include "ServerLib.h"

namespace lwSummon_Info
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "ShowSummonList", lwShowSummonList);
		def(pkState, "SummonList_Minimize", lwSummonList_Minimize);
		def(pkState, "SummonList_Tick", lwSummonList_Tick);
		def(pkState, "CallSummonToolTip", CallSummonToolTip);
	}

	void lwShowSummonList()
	{
		XUI::CXUI_Wnd *pWnd = XUIMgr.Activate(L"FRM_SUMMON_LIST");
		if(!pWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pListWnd = pWnd->GetControl(L"LST_SUMMON_INFO");
		if(!pListWnd)
		{
			return;
		}
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
		if(!pList)
		{
			return;
		}
		//테스트용. 다되면 지우자.
		pList->ClearList();
		AddAllSummoned();

		if( 0 == pList->GetTotalItemCount() )
		{
			pWnd->Close();
			return;
		}
		ReSizeUI();
		
	}
	void ReSizeUI()
	{
		XUI::CXUI_Wnd *pWnd = XUIMgr.Get(L"FRM_SUMMON_LIST");
		if(pWnd)
		{
			XUI::CXUI_Wnd* pListWnd = pWnd->GetControl(L"LST_SUMMON_INFO");
			if(pListWnd)
			{
				XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
				if(pList)
				{
					POINT2 ptTitleSize = pWnd->ImgSize();
					int const iElementHeight = pList->ItemLen();

					pListWnd->Size( ptTitleSize.x, iElementHeight );
					pWnd->Size( ptTitleSize.x, ptTitleSize.y + iElementHeight );

					pListWnd->Invalidate(true);
				}	
			}
		}
	}
	void lwSummonList_Minimize(lwUIWnd kSelf)
	{
		if( kSelf.IsNil() )
		{
			return;
		}
		lwUIWnd kParent = kSelf.GetParent();
		if( kParent.IsNil() )
		{
			return;
		}
		lwUIWnd kList = kParent.GetControl("LST_SUMMON_INFO");
		if( kList.IsNil() )
		{
			return;
		}
		bool const bVisible = kList.IsVisible();
		kList.Visible(!bVisible);
		ReSizeUI();
	}

	void AddAllSummoned()
	{
		PgPilot* pPilot = g_kPilotMan.GetPlayerPilot();
		if(!pPilot)
		{
			return;
		}
		CUnit* pUnit = pPilot->GetUnit();
		if(!pUnit)
		{
			return;
		}
		VEC_SUMMONUNIT const& kContSummonUnit = pUnit->GetSummonUnit();
		for(VEC_SUMMONUNIT::const_iterator c_it=kContSummonUnit.begin(); c_it!=kContSummonUnit.end(); ++c_it)
		{
			lwAddSummon( c_it->kGuid );
		}
	}

	void lwAddSummon(BM::GUID const kSummonGuid)
	{
		PgPilot* pkPilot = g_kPilotMan.FindPilot(kSummonGuid);
		if(!pkPilot || !pkPilot->GetUnit() )
		{
			return;
		}
		XUI::CXUI_Wnd *pWnd = XUIMgr.Get(L"FRM_SUMMON_LIST");
		if(!pWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pListWnd = pWnd->GetControl(L"LST_SUMMON_INFO");
		if(!pListWnd)
		{
			return;
		}
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
		if(!pList)
		{
			return;
		}
		if( pkPilot->GetUnit()->GetAbil(AT_SUMMONED_NOT_SHOW_UI) )
		{//소환체 UI 표기 안함 어빌
			return;
		}
		XUI::SListItem *pItem = pList->AddItem(kSummonGuid.str());
		if( !pItem->m_pWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pIconWnd =pItem->m_pWnd->GetControl(L"ICN_SUMMON");
		XUI::CXUI_Wnd* pAniBarWnd_Hp =pItem->m_pWnd->GetControl(L"BAR_HP");
		XUI::CXUI_Wnd* pAniBarWnd_Time =pItem->m_pWnd->GetControl(L"BAR_TIME");
		if( !pIconWnd || !pAniBarWnd_Hp || !pAniBarWnd_Time)
		{
			return;
		}
		PgSummoned * pkSummoned = dynamic_cast<PgSummoned*>(pkPilot->GetUnit());
		if( pkPilot->GetUnit()->IsUnitType(UT_SUMMONED) 
		&& pkPilot->GetUnit()->GetAbil(AT_HP)
		&& pkSummoned )
		{
			{//아이콘
				XUI::CXUI_Icon *pIcon = dynamic_cast<XUI::CXUI_Icon *>(pIconWnd);
				if(pIcon)
				{
					const CONT_DEFRES* pContDefRes = NULL;
					g_kTblDataMgr.GetContDef(pContDefRes);
					if(pContDefRes)
					{
						CONT_DEFRES::const_iterator find_iter = pContDefRes->find(pkPilot->GetAbil(AT_CLASS));
						if( pContDefRes->end() != find_iter)
						{
							CONT_DEFRES::mapped_type kRes = (*find_iter).second;
							pIcon->DefaultImgName( (*find_iter).second.strIconPath );
							SUVInfo  kUVInfo = pIcon->UVInfo();
							kUVInfo.Set(kRes.U, kRes.V, kRes.UVIndex);
							pIcon->UVInfo(kUVInfo);
						}
					}
				}
			}
			{//guid 저장
				pItem->m_pWnd->SetCustomData( kSummonGuid, sizeof(kSummonGuid) );
			}
		}
	}

	void lwSummonList_Tick()
	{
		XUI::CXUI_Wnd *pWnd = XUIMgr.Get(L"FRM_SUMMON_LIST");
		if(!pWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pListWnd = pWnd->GetControl(L"LST_SUMMON_INFO");
		if(!pListWnd)
		{
			return;
		}
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
		if(!pList)
		{
			return;
		}
		if( 0 == pList->GetTotalItemCount() )
		{
			pWnd->Close();
			return;
		}
		PgActor* pActor = g_kPilotMan.GetPlayerActor();
		if(!pActor)
		{
			return;
		}
		PgPilot* pPilot = pActor->GetPilot();
		if( !pPilot )
		{
			return;
		}
		CUnit* pUnit = pPilot->GetUnit();
		if( !pUnit )
		{
			return;
		}
		int const iSummonCount = pUnit->GetSummonUnitCount();
		if( 1 > iSummonCount )
		{
			pWnd->Close();
		}

		{//소환영력 현재/최소
			int const iMaxSupply = pUnit->GetAbil(AT_C_SUMMONED_MAX_SUPPLY);
			int const iCurSupply = GetTotalSummonedSupply(pUnit);
			BM::vstring kInfo( TTW(792100) );
			kInfo.Replace(L"#CUR#", iCurSupply);
			kInfo.Replace(L"#MAX#", iMaxSupply);
			pWnd->Text(static_cast<std::wstring>(kInfo));
		}
		XUI::SListItem* pItem = pList->FirstItem();
		while( pItem )
		{
			if( !lwUpdateItem( lwUIWnd(pItem->m_pWnd) ) )
			{
				pItem = pList->DeleteItem(pItem);
			}
			else
			{
				pItem = pList->NextItem(pItem);
			}
		}
	}

	bool lwUpdateItem(lwUIWnd kSelf)
	{
		if( kSelf.IsNil() )
		{
			return false;
		}
		lwGUID kGuid = kSelf.GetCustomDataAsGuid();
		PgPilot* pPilot = g_kPilotMan.FindPilot(kGuid.GetGUID());
		if( !pPilot )
		{
			return false;
		}
		CUnit* pUnit = pPilot->GetUnit();
		if( !pUnit )
		{
			return false;
		}
		PgSummoned * pkSummoned = dynamic_cast<PgSummoned*>(pUnit);
		if(pkSummoned)
		{
			lwUIWnd kAniBar_Hp = kSelf.GetControl("BAR_HP");
			if( !kAniBar_Hp.IsNil() )
			{//소환수 HP바
				int const iMax = pkSummoned->GetAbil(AT_C_MAX_HP);
				int const iMax_Original = pkSummoned->GetAbil(AT_MAX_HP);
				int const iNow = pkSummoned->GetAbil(AT_HP);
				bool bTwinkle = false;
				if( iNow < kAniBar_Hp.GetBarNow() )
				{
					bTwinkle = true;
				}
				kAniBar_Hp.BarMax(iMax);
				kAniBar_Hp.BarNow(iNow);
				
				std::wstring wstrText;
				BM::vstring kNowText(iNow);
				wstrText += static_cast<std::wstring>(kNowText);
				wstrText += L" / ";
				if( iMax_Original != iMax)
				{
					wstrText += L"{C=0xFF00ff06/}";
				}
				BM::vstring kMaxText(iMax);
				wstrText += static_cast<std::wstring>(kMaxText);
				wstrText += L"{C=0xFFffffff/}";	//폰트 초기화

				kAniBar_Hp.GetSelf()->Text(wstrText);
				if( bTwinkle )
				{//체력 미만일 때
					if( !kAniBar_Hp.GetSelf()->IsTwinkle() )
					{
						kAniBar_Hp.GetSelf()->TwinkleTime(1000);
						kAniBar_Hp.GetSelf()->TwinkleInterTime(100);
						kAniBar_Hp.GetSelf()->SetTwinkle(true);
					}
				}
				else
				{
					kAniBar_Hp.GetSelf()->SetTwinkle(false);
				}
			}
			lwUIWnd kAniBar_Time = kSelf.GetControl("BAR_TIME");
			if( !kAniBar_Time.IsNil())
			{//소환수 남은 시간 바
				DWORD const dwLifeTimeMax = pkSummoned->GetAbil(AT_LIFETIME);
				DWORD const dwLifeTimeNow = pkSummoned->LifeTime();
				kAniBar_Time.BarMax(dwLifeTimeMax);
				kAniBar_Time.BarNow(dwLifeTimeNow);
				
				BM::vstring kTime( TTW(792101) );
				//DWORD const dwSec = dwLifeTimeNow/1000%60;
				//kTime.Replace(L"#MIN#", dwMin);
				//kTime.Replace(L"#SEC#", dwSec);
				DWORD const dwSec = dwLifeTimeNow/1000;
				kTime.Replace(L"#SEC#", dwSec);

				DWORD const dwMin = dwLifeTimeNow/1000/60;
				kAniBar_Time.GetSelf()->Text( static_cast<std::wstring>(kTime) );
				if( 1 > dwMin )
				{//1분 미만일때
					if( !kAniBar_Time.GetSelf()->IsTwinkle() )
					{
						kAniBar_Time.GetSelf()->TwinkleTime(UINT_MAX);
						kAniBar_Time.GetSelf()->TwinkleInterTime(100);
						kAniBar_Time.GetSelf()->SetTwinkle(true);
					}
				}
				else
				{
					kAniBar_Time.GetSelf()->SetTwinkle(false);
				}
			}

			return true;
		}

		return false;
	}

	void SetSelectSummonAll(bool const bShow)
	{
		XUI::CXUI_Wnd *pWnd = XUIMgr.Get(L"FRM_SUMMON_LIST");
		if(!pWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pListWnd = pWnd->GetControl(L"LST_SUMMON_INFO");
		if(!pListWnd)
		{
			return;
		}
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
		if(!pList)
		{
			return;
		}
		XUI::SListItem* pItem = pList->FirstItem();
		while( pItem )
		{
			XUI::CXUI_Wnd* pSelectWnd = pItem->m_pWnd->GetControl(L"FRM_SELECT");
			if(pSelectWnd)
			{
				pSelectWnd->Visible(bShow);
			}
			pItem = pList->NextItem(pItem);
		}
	}

	void ShowSelectSummon(BM::GUID const& rkSummonGuid)
	{
		XUI::CXUI_Wnd *pWnd = XUIMgr.Get(L"FRM_SUMMON_LIST");
		if(!pWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pListWnd = pWnd->GetControl(L"LST_SUMMON_INFO");
		if(!pListWnd)
		{
			return;
		}
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
		if(!pList)
		{
			return;
		}
		XUI::SListItem* pItem = pList->FirstItem();
		while( pItem )
		{
			XUI::CXUI_Wnd* pSelectWnd = pItem->m_pWnd->GetControl(L"FRM_SELECT");
			if(pSelectWnd)
			{
				lwUIWnd kItem = lwUIWnd(pItem->m_pWnd);
				if( !kItem.IsNil() )
				{
					lwGUID kGuid = kItem.GetCustomDataAsGuid();
					if( kGuid.IsEqual( lwGUID(rkSummonGuid) ) )
					{//선택한것만 보이도록
						pSelectWnd->Visible(true);
					}
					else
					{//선택안한건 전부 안보이도록
						pSelectWnd->Visible(false);
					}
				}
			}
			pItem = pList->NextItem(pItem);
		}
		g_kUISound.PlaySoundByID(L"sum_menu");
	}	

	void CallSummonToolTip(lwGUID kGuid, lwPoint2 &pt)
	{
		if( kGuid.IsNil() )
		{
			return;
		}
		
		PgPilot* pPilot = g_kPilotMan.FindPilot(kGuid.GetGUID());
		if( !pPilot )
		{
			return;
		}
		CUnit* pUnit = pPilot->GetUnit();
		if( !pUnit )
		{
			return;
		}
		PgSummoned * pkSummoned = dynamic_cast<PgSummoned*>(pUnit);
		if(pkSummoned)
		{
			std::wstring const ENTER_STRING = L"\n";
			std::wstring wstrTextOut;
			{//이름
				wstrTextOut += TTW(40102);
				BM::vstring kName(TTW(792103));
				const wchar_t *pNameText = NULL;
				if(GetDefString(pkSummoned->GetAbil(AT_CLASS), pNameText))
				{
					kName.Replace(L"#NAME#", pNameText);
				}
				int const iLevel = pkSummoned->GetAbil(AT_LEVEL);
				kName.Replace(L"#LEVEL#", iLevel);
				wstrTextOut += static_cast<std::wstring>(kName);
				wstrTextOut += ENTER_STRING;
			}
			wstrTextOut += TTW(40011);	//폰트 초기화
			{//가용 용량
				BM::vstring kSupply(TTW(792104));
				kSupply.Replace(L"#SUPPLY#", pkSummoned->Supply());
				wstrTextOut += static_cast<std::wstring>(kSupply);
				wstrTextOut += ENTER_STRING;
			}
			{//물리 공격력
				BM::vstring kPhyAtt(TTW(792105));
				int iPhyDmgPer = pkSummoned->GetAbil(AT_PHY_DMG_PER);
				if( 0 == iPhyDmgPer ) { iPhyDmgPer = ABILITY_RATE_VALUE; }
				int const iPhyDmgPer2 = pkSummoned->GetAbil(AT_PHY_DMG_PER2);
				int const iResultPhyDmgPer = iPhyDmgPer + ( (iPhyDmgPer*iPhyDmgPer2)/ABILITY_RATE_VALUE );
				if( ABILITY_RATE_VALUE != iResultPhyDmgPer )
				{
					wstrTextOut += TTW(40012);
				}
				int const iPhyMin = (pkSummoned->GetAbil(AT_C_PHY_ATTACK_MIN)*iResultPhyDmgPer) / ABILITY_RATE_VALUE;
				int const iPhyMax = (pkSummoned->GetAbil(AT_C_PHY_ATTACK_MAX)*iResultPhyDmgPer) / ABILITY_RATE_VALUE;
				kPhyAtt.Replace(L"#MIN#", iPhyMin);
				kPhyAtt.Replace(L"#MAX#", iPhyMax);
				wstrTextOut += static_cast<std::wstring>(kPhyAtt);
				wstrTextOut += TTW(40011);	//폰트 초기화
				wstrTextOut += ENTER_STRING;
			}
			{//마법 공격력
				BM::vstring kMagAtt(TTW(792106));
				int iMagDmgPer = pkSummoned->GetAbil(AT_MAGIC_DMG_PER);
				if( 0 == iMagDmgPer ) { iMagDmgPer = ABILITY_RATE_VALUE; }
				int const iMagDmgPer2 = pkSummoned->GetAbil(AT_MAGIC_DMG_PER2);
				int const iResultMagDmgPer = iMagDmgPer + ( (iMagDmgPer*iMagDmgPer2)/ABILITY_RATE_VALUE );
				if( ABILITY_RATE_VALUE != iResultMagDmgPer )
				{
					wstrTextOut += TTW(40012);
				}
				int const iMagMin = (pkSummoned->GetAbil(AT_C_MAGIC_ATTACK_MIN)*iResultMagDmgPer) / ABILITY_RATE_VALUE;
				int const iMagMax = (pkSummoned->GetAbil(AT_C_MAGIC_ATTACK_MAX)*iResultMagDmgPer) / ABILITY_RATE_VALUE;
				kMagAtt.Replace(L"#MIN#", iMagMin);
				kMagAtt.Replace(L"#MAX#", iMagMax);
				wstrTextOut += static_cast<std::wstring>(kMagAtt);
				wstrTextOut += TTW(40011);	//폰트 초기화
				wstrTextOut += ENTER_STRING;
			}
			{//물리 방어력
				BM::vstring kPhyDef(TTW(792107));
				int const iOriginal = pkSummoned->GetAbil(AT_PHY_DEFENCE);
				int const iFinal = pkSummoned->GetAbil(AT_C_PHY_DEFENCE);
				if( iFinal != iOriginal )
				{
					wstrTextOut += TTW(40012);
				}
				kPhyDef.Replace(L"#NUM#", iFinal);
				wstrTextOut += static_cast<std::wstring>(kPhyDef);
				wstrTextOut += TTW(40011);	//폰트 초기화
				wstrTextOut += ENTER_STRING;
			}
			{//마법 방어력
				BM::vstring kMagDef(TTW(792108));
				int const iOriginal = pkSummoned->GetAbil(AT_MAGIC_DEFENCE);
				int const iFinal = pkSummoned->GetAbil(AT_C_MAGIC_DEFENCE);
				if( iFinal != iOriginal )
				{
					wstrTextOut += TTW(40012);
				}
				kMagDef.Replace(L"#NUM#", iFinal);
				wstrTextOut += static_cast<std::wstring>(kMagDef);
				wstrTextOut += TTW(40011);	//폰트 초기화
				wstrTextOut += ENTER_STRING;
			}

			lwCallToolTipByText(0, wstrTextOut, pt, "ToolTip_Skill", 0, 0);
		}
	}
}