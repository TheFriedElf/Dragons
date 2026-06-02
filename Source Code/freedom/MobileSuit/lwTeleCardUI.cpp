#include "StdAfx.h"
#include "Variant/Global.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "lwTeleCardUI.h"

void lwTeleCardUI::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "AddTeleCarMapInfo", lwAddTeleCarMapInfo);
	def(pkState, "SendMapMove", lwSendMapMove);
	def(pkState, "SelectShard", lwSelectShard);
	def(pkState, "CheckOpenShard", lwCheckOpenShard);
	def(pkState, "CallTeleCardUI", CallTeleCardUI);
}

void lwTeleCardUI::lwCheckOpenShard()
{
	g_kTeleCard.CheckOpenShard();
}

void lwTeleCardUI::lwAddTeleCarMapInfo(int const iShardNo, int const iIndex, int const iMapNo)
{
	g_kTeleCard.AddTeleCarMapInfo(iShardNo, iIndex, iMapNo);
}

void lwTeleCardUI::CallTeleCardUI(DWORD const kItemNo)
{
	XUI::CXUI_Wnd* pTeleCard = XUIMgr.Call(_T("SFRM_TELEPORT"));
	if( !pTeleCard )
	{
		return;
	}

	pTeleCard->SetCustomData(&kItemNo, sizeof(kItemNo));

	lua_tinker::call<void>("TeleCardMapDef");
	g_kTeleCard.InitUI(pTeleCard);

	XUI::CXUI_Wnd* pkBG = pTeleCard->GetControl(L"FRM_BG");
	if(pkBG)
	{
		XUI::CXUI_Wnd* pkTitle = pkBG->GetControl(L"SFRM_TITLE");
		if(pkTitle)
		{
			BM::vstring kText(TTW(2911).c_str());
			if(0==kItemNo)
			{
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if(!pkPlayer)
				{
					return; 
				}

				BM::vstring	kString(GetMoneyString(PgHometownPortalUtil::UsePortalCost(pkPlayer->GetAbil(AT_LEVEL)), true, false).c_str());

				/*int const iMoney = PgHometownPortalUtil::UsePortalCost(pkPlayer->GetAbil(AT_LEVEL));

				int iGold = iMoney * 0.0001f;
				int iSilver = (int)(iMoney * 0.01f) % 100;
				int iCopper = iMoney % 100;

				
				if( 0 != iGold )
				{
					kString += iGold;
					kString += TTW(401111);
					if( 0 != iCopper || 0 != iSilver ){ kString += L" "; };
				}
				if( 0 != iSilver )
				{
					kString += iSilver;
					kString += TTW(401112);
					if( 0 != iCopper ){ kString += L" "; };
				}
				if( 0 != iCopper )
				{
					kString += iCopper;
					kString += TTW(401113);
				}*/

				kText = TTW(201114);
				if( !pkPlayer->HomeAddr().IsNull() )
				{
					kText += TTW(50106);
				}
				else
				{
					kText += kString;
				}
			}
			pkTitle->Text((std::wstring const&)kText);
		}
	}
}

void lwTeleCardUI::lwSendMapMove(lwUIWnd UISelf)
{
	if(UISelf.IsNil())
	{
		return; 
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return; 
	}

	PgInventory *pkInven = pkPlayer->GetInven();
	if(!pkInven)
	{
		return;
	}

	XUI::CXUI_Wnd* pkHighWnd = UISelf.GetParent().GetSelf();
	while( pkHighWnd && pkHighWnd->Parent() )
	{
		pkHighWnd = pkHighWnd->Parent();
	}

	DWORD	dwItemNo = 0;
	if( pkHighWnd )
	{
		pkHighWnd->GetCustomData(&dwItemNo, sizeof(dwItemNo));
	}

	if(0<dwItemNo)	//진짜 순간이동 카드일 경우
	{
		SItemPos TeleCardItemPos;
		if(E_FAIL == pkInven->GetFirstItem(dwItemNo, TeleCardItemPos))
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2914, true);
			return;
		}

		//	요청 날림
		BM::Stream	kPacket;
		kPacket.Push(PT_C_M_REQ_USER_MAP_MOVE);
		kPacket.Push(UISelf.GetCustomData<int>());
		kPacket.Push(TeleCardItemPos);
		NETWORK_SEND(kPacket)
	}
	else
	{
		__int64 const iMoney = PgHometownPortalUtil::UsePortalCost(pkPlayer->GetAbil(AT_LEVEL));
		if(iMoney>pkPlayer->GetAbil64(AT_MONEY))
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1500, true);
			return;
		}

		//	요청 날림
		BM::Stream	kPacket;
		kPacket.Push(PT_C_M_HOME_REQ_MAPMOVE);
		kPacket.Push(UISelf.GetCustomData<int>());
		NETWORK_SEND(kPacket)
	}
}

void lwTeleCardUI::lwSelectShard(lwUIWnd UISelf)
{
	if( UISelf.IsNil() )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = UISelf.GetParent().GetSelf();
	if( !pParent )
	{
		return;
	}
	
	for(int i = 0; i < g_kTeleCard.GetShardCount(); ++i)
	{
		BM::vstring vShard(_T("SFRM_SHARD"));
		vShard += i;

		XUI::CXUI_Wnd* pShard = pParent->GetControl(vShard);
		if( !pShard )
		{
			return;
		}

		XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pShard->GetControl(_T("SFRM_OVERIMG")));
		if( !pBtn )
		{
			return;
		}

		pBtn->Check(false);
		pBtn->ClickLock(false);
	}

	lwUIWnd UIBtn = UISelf.GetControl("SFRM_OVERIMG");
	if( UIBtn.IsNil() )
	{
		return;
	}
	UIBtn.CheckState(true);
	UIBtn.LockClick(true);

	XUI::CXUI_Wnd*	pList = pParent->GetControl(_T("LST_AREA"));
	if( !pList )
	{
		return;
	}

	g_kTeleCard.SetShardAreaItem(dynamic_cast<XUI::CXUI_List*>(pList), UISelf.GetCustomData<int>());
}

PgTeleCard::PgTeleCard(void)
{
}

PgTeleCard::~PgTeleCard(void)
{
}

void PgTeleCard::CheckOpenShard()
{
	kShardCont::iterator Shard_iter = m_kShard.begin();
	while( Shard_iter != m_kShard.end() )
	{
		Shard_iter->second.bIsOpen = false;
		kMapCont::iterator Map_iter = Shard_iter->second.kCont.begin();
		while( Map_iter != Shard_iter->second.kCont.end() )
		{
			if( lwIsOpenGroundByNo(Map_iter->second) )
			{
				Shard_iter->second.bIsOpen = true;
				break;
			}
			++Map_iter;
		}
		++Shard_iter;
	}
}

void PgTeleCard::AddTeleCarMapInfo(int const iShardNo, int const iIndex, int const iMapNo)
{
	SShardInfo	Info;
	std::pair<kShardCont::iterator, bool> Shard_Rst = m_kShard.insert(std::make_pair(iShardNo, Info));
	std::pair<kMapCont::iterator, bool> Map_Rst = Shard_Rst.first->second.kCont.insert(std::make_pair(iIndex, iMapNo));
	if( !Map_Rst.second )
	{
		//또들어왔네?
	}
}

void PgTeleCard::InitUI(XUI::CXUI_Wnd* pParent)
{
	if( !m_kShard.size() )
	{
		return;
	}

	CheckOpenShard();
	kShardCont::iterator	Shard_iter = m_kShard.begin();
	for(int i = 0; i < 10; ++i)
	{
		BM::vstring vShard(_T("SFRM_SHARD"));
		vShard += i;

		XUI::CXUI_Wnd* pShard = pParent->GetControl(vShard);
		if( pShard )
		{
			if( Shard_iter == m_kShard.end() || !Shard_iter->second.bIsOpen )
			{
				pShard->Visible(false);
				continue;
			}

			pShard->Visible(true);
			pShard->Text(TTW(Shard_iter->first));
			pShard->SetCustomData(&Shard_iter->first, sizeof(Shard_iter->first));
			++Shard_iter;

			XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pShard->GetControl(_T("SFRM_OVERIMG")));
			if( !pBtn )
			{
				return;
			}

			if( i == 0 )
			{
				pBtn->Check(true);
				pBtn->ClickLock(true);
			}
			else
			{
				pBtn->Check(false);
				pBtn->ClickLock(false);
			}
		}
	}

	int iDefaultShardNo = m_kShard.begin()->first;
	SetShardAreaItem(dynamic_cast<XUI::CXUI_List*>(pParent->GetControl(_T("LST_AREA"))), iDefaultShardNo);
}

void PgTeleCard::SetShardAreaItem(XUI::CXUI_List* pList, int const iShardNo)
{
	if( !pList )
	{
		return;
	}

	pList->DeleteAllItem();
	CONT_DEFMAP const * pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);	//맵 데프

	if( !pContDefMap )
	{
		return;
	}

	kShardCont::iterator	Shard_iter = m_kShard.find(iShardNo);
	if( Shard_iter != m_kShard.end() )
	{
		kMapCont::iterator	Map_iter = Shard_iter->second.kCont.begin();
		while( Map_iter != Shard_iter->second.kCont.end() )
		{
			CONT_DEFMAP::const_iterator map_it = pContDefMap->find(Map_iter->second);	//맵번호로 맵 데프에서 찾아서
			if( pContDefMap->end() != map_it
			&&	0 == ((*map_it).second.iAttr & GATTR_FLAG_CANT_WARP)
			&& lwIsOpenGroundByNo(Map_iter->second) )
			{
				XUI::SListItem*	pItem = pList->AddItem(_T(""));	
				if( !pItem || !pItem->m_pWnd )
				{
					return;
				}
				XUI::CXUI_Wnd* pMapName = pItem->m_pWnd->GetControl(_T("SFRM_TEL_AREA_ITEM"));
				XUI::CXUI_Button* pSend = dynamic_cast<XUI::CXUI_Button*>(pItem->m_pWnd->GetControl(_T("BTN_SEND_TEL")));
				if( !pMapName || !pSend )
				{
					return;
				}
				pMapName->Text(GetMapName(Map_iter->second));
				pSend->SetCustomData(&Map_iter->second, sizeof(Map_iter->second));
			}
			++Map_iter;
		}
	}
}
