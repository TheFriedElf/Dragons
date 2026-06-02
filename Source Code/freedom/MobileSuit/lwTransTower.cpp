#include "stdafx.h"
#include "ServerLib.h"
#include "PgNetWork.h"
#include "PgPilotMan.h"
#include "lwTransTower.h"
#include "PgItemUtil.h"
#include "PgPilot.h"
#include "Variant/PgPlayer.h"
#include "Variant/PgPet.h"

namespace lwTransTower
{
	void PgTransPortListUI::Refresh()
	{
		if ( m_pWnd )
		{
			XUI::CXUI_Wnd * pkControl = m_pWnd->GetControl( L"SFRM_TARGET" );
			if ( pkControl )
			{
				pkControl->Text(m_wstrName);
			}

			pkControl = m_pWnd->GetControl( L"SFRM_PRICE" );
			if ( pkControl )
			{
				__int64 i64Price = m_kData.i64Price;
				PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
				if(pkPlayer)
				{
					if( S_PST_TranstowerDiscount const* pkPremiumDiscount = pkPlayer->GetPremium().GetType<S_PST_TranstowerDiscount>() )
					{
						i64Price -= std::max( SRateControl::GetValueRate(i64Price, static_cast<__int64>(pkPremiumDiscount->iDiscount)), 0i64);
					}
				}

				std::wstring const wstrMoney = GetMoneyString( i64Price, false );
				pkControl->Text( wstrMoney );
			}

			XUI::CXUI_Button *pkBtn = dynamic_cast<XUI::CXUI_Button*>(m_pWnd->GetControl( L"BTN_TRANSPORT" ));
			if ( pkBtn )
			{
				pkBtn->Disable( !m_bIsOpen );
				if(m_bIsOpen
					&& m_bHere
					)
				{// 이동이 열린 지역이라도, 현재 맵이면 이동 불가
					pkBtn->Disable( m_bHere );
				}
				if(m_bHere)
				{
					pkBtn->Text( TTW(2709) );
				}
				else
				{
					pkBtn->Text( TTW(2703) );
				}
			}

			pkControl = m_pWnd->GetControl( L"IMG_CHECKBT" );
			if ( pkControl )
			{
				pkControl->UVUpdate( m_bIsOpen ? 3 : 1 );
			}

			pkControl = m_pWnd->GetControl( L"IMG_COVER" );
			if ( pkControl )
			{
				pkControl->Visible( !m_bIsOpen );
			}
			pkControl = m_pWnd->GetControl( L"IMG_HERE" );
			if ( pkControl )
			{
				pkControl->Visible( m_bHere );
			}
		}
	}

	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "CallTransTowerList", CallTransTowerList);
		def(pkState, "OnClickTransPort", OnClickTransPort);
		def(pkState, "OnClickOpenMap", OnClickOpenMap);
		def(pkState, "OnClickSelectOpenMapItem", OnClickSelectOpenMapItem);
		return true;
	}

	bool CallTransTowerList( lwGUID kGuid, lwUIWnd kListWnd, short int sContinent )
	{
		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if ( !pkPlayer )
		{
			return false;
		}

		XUI::CXUI_List2 *pkList = dynamic_cast<XUI::CXUI_List2*>(kListWnd());
		if ( !pkList )
		{
			return false;
		}

		if ( sContinent )
		{
			pkList->SetCustomData( &sContinent, sizeof(sContinent) );
		}
		else
		{
			pkList->GetCustomData( &sContinent, sizeof(sContinent) );
		}

		pkList->ClearList();

		CONT_DEFMAP const *pkDefMap = NULL;
		CONT_DEF_TRANSTOWER const *pkDefTransTower = NULL;
		g_kTblDataMgr.GetContDef( pkDefMap );
		g_kTblDataMgr.GetContDef( pkDefTransTower );

		CONT_DEF_TRANSTOWER::const_iterator def_itr = pkDefTransTower->find( kGuid() );
		if ( def_itr != pkDefTransTower->end() )
		{
			pkList->OwnerGuid( def_itr->first );

			int iDiscountRate = 0;

			// 할인권이 있는지 찾아본다.
			{
				int iItemNo = 0;
				PgItemUtil kItemUtil( *(pkPlayer->GetInven()) );
				if ( S_OK == kItemUtil.Get< PgItemUtil::E_ABIL_GREATER >( iItemNo, IT_CASH, UICT_TRANSTOWER_DISCOUNT, AT_USE_ITEM_CUSTOM_VALUE_1 ) )
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
					if ( pkItemDef )
					{
						iDiscountRate = std::min( pkItemDef->GetAbil( AT_USE_ITEM_CUSTOM_VALUE_1 ), ABILITY_RATE_VALUE );
					}
				}
			}

			{
				PgPilot* pkPilot = g_kPilotMan.GetPlayerPilot();
				PgPlayer* pkPlayer = NULL;
				PgPilot* pkPetPilot = NULL;
				CUnit* pkPetUnit = NULL;
				if(pkPilot)
				{
					PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
					if(pkPlayer)
					{
						pkPetPilot = g_kPilotMan.FindPilot(pkPlayer->SelectedPetID());
					}
				}
				if(pkPetPilot)
				{
					pkPetUnit = pkPetPilot->GetUnit();
					if(pkPetUnit)
					{
						iDiscountRate += pkPetUnit->GetAbil(AT_TRANS_DC_PET_SKILL);
					}
				}
			}

			// 자유이용권 이펙트가 있는지 찾아본다.
			bool const bIsFree = ( pkPlayer->IsItemEffect( EFFECTNO_TRANSTOWER_FREE ) || pkPlayer->IsItemEffect( EFFECTNO_TRANSTOWER_FREE + static_cast<int>(sContinent) ) );

			CONT_DEF_TRANSTOWER_TARGET const &kDefTTT = def_itr->second;
			CONT_DEF_TRANSTOWER_TARGET::const_iterator ttt_itr = kDefTTT.begin();
			for ( ; ttt_itr != kDefTTT.end() ; ++ttt_itr )
			{
				CONT_DEFMAP::const_iterator map_itr = pkDefMap->find( ttt_itr->iGroundNo );
				if( map_itr != pkDefMap->end() )
				{
					if ( map_itr->second.sContinent == sContinent )
					{
						bool bHere = false;
						if(g_pkWorld
							&& g_pkWorld->MapNo() == ttt_itr->iGroundNo 
							)
						{
							bHere = true;
						}
						bool const bIsOpen = ( bIsFree || SUCCEEDED(pkPlayer->IsOpenWorldMap( ttt_itr->iGroundNo )) );
						PgTransPortListUI_CreatePolicy<> kCreatePolicy( *ttt_itr, iDiscountRate, bIsOpen, (map_itr->second.iAttr & GATTR_VILLAGE), bHere );

						::GUID kGuid;
						::memset( &kGuid, 0, sizeof(kGuid) );
						kGuid.Data1 = ttt_itr->iGroundNo;
						kGuid.Data2 = ttt_itr->nTargetSpawn;//

						pkList->AddItem( BM::GUID(kGuid), kCreatePolicy );
					}
				}
			}

			pkList->Sort( PgTransPortListUI::SSort() );
		}

		return true;
	}

	bool OnClickTransPort( lwUIWnd kUIWnd )
	{
		XUI::CXUI_Wnd * pkWnd = kUIWnd();
		if ( pkWnd )
		{
			pkWnd = pkWnd->Parent();
			if ( pkWnd )
			{
				XUI::CXUI_List2 *pkList = dynamic_cast<XUI::CXUI_List2*>(pkWnd->Parent());
				if ( pkList )
				{
					PgTransPortListUI *pkTPLUI = dynamic_cast<PgTransPortListUI*>(pkList->GetItem( pkWnd->OwnerGuid() ));
					if ( pkTPLUI )
					{
						XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_TRANSTOWER"));
						if(pkTopWnd)
						{
							XUI::CXUI_Wnd* pkWndPetInfo = pkTopWnd->GetControl(_T("FRM_CALLER_IS_PET"));
							if(pkWndPetInfo && pkWndPetInfo->Visible())
							{ //전송타워가 아닌 펫에 의해 호출되었다면
								BM::GUID kPetGuid;
								pkWndPetInfo->GetCustomData(&kPetGuid, sizeof(BM::GUID));
								BM::Stream kPacket( PT_C_M_REQ_TRANSTOWER_BY_PET, kPetGuid );
								kPacket.Push( static_cast<BM::GUID>(pkList->OwnerGuid()) );
								kPacket.Push( static_cast<int>(E_TRANSTOWER_MAPMOVE) );
								kPacket.Push( static_cast<TBL_DEF_TRANSTOWER_TARGET_KEY>(pkTPLUI->m_kData) );
								kPacket.Push( false );
								NETWORK_SEND(kPacket);
								return true;
							}
						}

						BM::Stream kPacket( PT_C_M_REQ_TRANSTOWER, pkList->OwnerGuid() );
						kPacket.Push( static_cast<int>(E_TRANSTOWER_MAPMOVE) );
						kPacket.Push( static_cast<TBL_DEF_TRANSTOWER_TARGET_KEY>(pkTPLUI->m_kData) );

						bool bFindDiscountItem = false;

						PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
						if ( pkPlayer )
						{
							SItemPos kItemPos;
							PgItemUtil kItemUtil( *(pkPlayer->GetInven()) );
							if ( S_OK == kItemUtil.Get< PgItemUtil::E_ABIL_GREATER >( kItemPos, IT_CASH, UICT_TRANSTOWER_DISCOUNT, AT_USE_ITEM_CUSTOM_VALUE_1 ) )
							{
								bFindDiscountItem = true;

								kPacket.Push( bFindDiscountItem );
								kPacket.Push( kItemPos );
							}
						}

						if ( !bFindDiscountItem )
						{
							kPacket.Push(  bFindDiscountItem );
						}

						NETWORK_SEND(kPacket);
						return true;
					}
				}
			}
		}
		return false;
	}

	bool OnClickOpenMap( lwUIWnd kUIWnd )
	{
		PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
		if ( !pkPlayer )
		{
			return false;
		}

		XUI::CXUI_Wnd * pkWnd = kUIWnd();
		if ( pkWnd )
		{
			pkWnd = pkWnd->Parent();
			if ( pkWnd )
			{
				XUI::CXUI_List2 *pkList = dynamic_cast<XUI::CXUI_List2*>(pkWnd->Parent());
				if ( pkList )
				{
					PgTransPortListUI *pkTPLUI = dynamic_cast<PgTransPortListUI*>(pkList->GetItem( pkWnd->OwnerGuid() ));
					if ( pkTPLUI && !(pkTPLUI->m_bIsOpen) )
					{
						short int sContinent = 0;
						pkList->GetCustomData( &sContinent, sizeof(sContinent) );

						// 아이템이 있는지 찾아본다.
						struct SItemFilter
						{
							SItemFilter( int const _iContient )
								:	iContient(_iContient)
							{}

							bool operator()( CItemDef const *pkItemDef )const
							{
								if ( pkItemDef )
								{
									int const iValue = pkItemDef->GetAbil( AT_USE_ITEM_CUSTOM_VALUE_1 );
									return !iValue || iValue == iContient;
								}
								return false;
							}

							int const iContient;
						};

						SItemFilter kFilter( static_cast<int>(sContinent) );
						ContHaveItemNoCount	kItemCont;
						PgItemUtil kItemUtil( *(pkPlayer->GetInven()) );
						if ( SUCCEEDED(kItemUtil.GetItemNos( kItemCont, IT_CASH, UICT_TRANSTOWER_OPENMAP, kFilter )) )
						{
							XUI::CXUI_Wnd *pkPopUpWnd = XUIMgr.Activate( L"SFRM_TRANSTOWER_SELECT_FREEITEM", true );
							if ( pkPopUpWnd )
							{
								XUI::CXUI_Icon *pkIcon = NULL;
								size_t i = 0;
								ContHaveItemNoCount::const_iterator item_itr = kItemCont.begin();
								while ( 1 )
								{
									BM::vstring vstr( L"ICON_ITEM_" );
									vstr += (i + 1);

									pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pkPopUpWnd->GetControl( vstr ));
									if ( !pkIcon )
									{
										break;
									}

									SIconInfo kIConInfo = pkIcon->IconInfo();
									kIConInfo.iIconResNumber = 0;
									
									if ( item_itr != kItemCont.end() )
									{
										kIConInfo.iIconResNumber = item_itr->first;
										++item_itr;

										SItemPos kItemPos;
										if ( SUCCEEDED(pkPlayer->GetInven()->GetFirstItem( kIConInfo.iIconResNumber, kItemPos )) )
										{
											BM::Stream kPacket( PT_C_M_REQ_TRANSTOWER, pkList->OwnerGuid() );
											kPacket.Push( static_cast<int>(E_TRANSTOWER_OPENMAP) );
											kPacket.Push( static_cast<TBL_DEF_TRANSTOWER_TARGET_KEY>(pkTPLUI->m_kData) );
											kPacket.Push( kItemPos );
											pkIcon->SetCustomData( kPacket.Data() );
										}
									}

									pkIcon->SetIconInfo( kIConInfo );
									++i;
								}
							}
							return true;
						}		
					}
				}
			}
		}
		return false;
	}

	bool OnClickSelectOpenMapItem( lwUIWnd kUIWnd )
	{
		XUI::CXUI_Wnd * pkWnd = kUIWnd();
		if ( pkWnd )
		{
			BM::Stream kPacket;
			pkWnd->GetCustomData( kPacket.Data() );
			kPacket.PosAdjust();
			if ( kPacket.Size() )
			{
				NETWORK_SEND( kPacket );
				return bSendRet;
			}
		}
		return false;
	}

};//<- namespace lwTransTower
