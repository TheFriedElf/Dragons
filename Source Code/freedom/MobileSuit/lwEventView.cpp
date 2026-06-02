#include "StdAfx.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "lwEventView.h"
#include "lwUIQuest.h"
#include "ServerLib.h"

void lwEventView::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "CallEventViewUI", lwCallEventViewUI);
	def(pkState, "ViewSelectEventInfo", lwViewSelectEventInfo);
	def(pkState, "GetEventItem", lwGetEventItem);

}

void lwEventView::lwCallEventViewUI()
{
	BM::Stream	kPacket;
	kPacket.Push(PT_C_M_REQ_COUPON_EVENT_SYNC);
	NETWORK_SEND(kPacket);
}

void lwEventView::lwViewSelectEventInfo(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pParent = UISelf.GetParent().GetParent().GetSelf();
	if( !pParent )
	{
		return;
	}

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(UISelf.GetParent().GetSelf());
	if( !pList )
	{
		return;
	}

	for(int i = 0; i < 2; ++i)
	{
		BM::vstring vStr(L"LST_EVENT");
		vStr += i;
		
		XUI::CXUI_List* pDeSelList = dynamic_cast<XUI::CXUI_List*>(pParent->GetControl(vStr));
		if( pDeSelList )
		{
			XUI::SListItem* pItem = pDeSelList->FirstItem();
			while( pItem && pItem->m_pWnd )
			{
				XUI::CXUI_CheckButton* pBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pItem->m_pWnd);
				if( !pBtn )
				{
					return;
				}

				pBtn->ClickLock(false);
				pBtn->Check(false);

				pItem = pDeSelList->NextItem(pItem);
			}
		}
	}
	UISelf.CheckState(true);
	UISelf.LockClick(true);

	if( UISelf.GetCustomDataSize() )
	{
		int Index = 0;
		pList->GetCustomData(&Index, sizeof(Index));
		ViewSelectEventInfo(UISelf.GetCustomData<int>(), pParent, (EVT_COUPON != Index));
	}
}

void lwEventView::lwGetEventItem(lwUIWnd UISelf)
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

	for( int i = 0; i < 2; ++i )
	{
		BM::vstring vStr(L"SFRM_EVENT_TYPE");
		vStr += i;
		XUI::CXUI_Wnd* pMain = pParent->GetControl(vStr);
		if( pMain && pMain->Visible() )
		{
			switch( i )
			{
			case EVT_COUPON:
				{
					lwActivateUI("SFRM_EVENT_COUPON", true);
				}break;
			case EVT_NEWTYPE:
				{
					int const iEventID = UISelf.GetCustomData<int>();
					SCOUPON_EVENT_INFO		kInfo;
					TBL_EVENT				kEvt;

					bool Result = g_kEventViewClient.GetKeyMatchInfo(iEventID, kInfo, kEvt);
					if( Result )
					{
						if( ET_MANUAL_EVENT_MIN <= kEvt.iEventType && ET_MANUAL_EVENT_MAX > kEvt.iEventType )
						{
							BM::Stream kPacket(PT_C_M_REQ_JOIN_EVENT);
							kPacket.Push(iEventID);
							NETWORK_SEND(kPacket);
						}
					}
				}break;
			default: break;
			}
		}
	}				
}

int const lwEventView::SetEventList(XUI::CXUI_Wnd* pParent)
{
	if( !pParent )
	{
		return 0;
	}

	int iDefaultEventKey = 0;
	for(int i = 0; i < 2; ++i)
	{
		BM::vstring vStr(L"LST_EVENT");
		vStr += i;

		XUI::CXUI_List*	pList = dynamic_cast<XUI::CXUI_List*>(pParent->GetControl(vStr));
		if( pList )
		{
			pList->SetCustomData(&i, sizeof(i));
			CONT_EVENT_TINY_INFO_LIST	kEventList;
			bool	bResult = false;
			switch( i )
			{
			case EVT_COUPON:
				{
					bResult = g_kEventViewClient.GetActivatedCouponEvent(kEventList);
				}break;
			case EVT_NEWTYPE:
				{
					bResult = g_kEventViewClient.GetActivatedServerEvent(kEventList);
				}break;
			default:{ continue; };
			}

			if( !bResult )
			{
				pList->DeleteAllItem();
				continue;
			}

			XUIListUtil::SetMaxItemCount(pList, kEventList.size());

			XUI::SListItem* pItem = pList->FirstItem();
			CONT_EVENT_TINY_INFO_LIST::iterator Coupon_iter = kEventList.begin();
			while( Coupon_iter != kEventList.end() )
			{
				CONT_EVENT_TINY_INFO_LIST::key_type const& kKey = Coupon_iter->first;
				CONT_EVENT_TINY_INFO_LIST::mapped_type const& kVal = Coupon_iter->second;

				if( pItem && pItem->m_pWnd )
				{
					int const iWidth = pItem->m_pWnd->Width() - 20;
					Quest::SetCutedTextLimitLength(pItem->m_pWnd, kVal, _T("..."), iWidth);
					pItem->m_pWnd->SetCustomData(&kKey, sizeof(kKey));
					pItem = pList->NextItem(pItem);
				}
				++Coupon_iter;
			}

			if( i == 0 )
			{
				iDefaultEventKey = kEventList.begin()->first;
			}
		}
	}
	return iDefaultEventKey;
}

bool lwEventView::ViewSelectEventInfo(int const& EventKey, XUI::CXUI_Wnd* pParent, bool bIsNewType)
{
	XUI::CXUI_Wnd* pTitle = pParent->GetControl(_T("SFRM_EVENT_TITLE"));
	XUI::CXUI_Wnd* pPeriod = pParent->GetControl(_T("SFRM_PERIOD"));
	XUI::CXUI_Wnd* pContent = pParent->GetControl(_T("SFRM_CONTENT"));
	XUI::CXUI_Wnd* pTakeBtn = pParent->GetControl(_T("BTN_TAKE_ITEM"));
	if( !pTitle || !pPeriod || !pContent || !pTakeBtn )
	{
		return false;
	}

	SCOUPON_EVENT_INFO		kInfo;
	CONT_EVENT_REWARD_LIST	kReward;
	TBL_EVENT				kEvt;

	bool Result = false;
	if( true == bIsNewType )
	{
		Result = g_kEventViewClient.GetKeyMatchInfo(EventKey, kInfo, kEvt);
	}
	else
	{
		Result = g_kEventViewClient.GetKeyMatchInfo(EventKey, kInfo, kReward);
	}

	XUI::CXUI_Wnd* pRewardMain = NULL;
	{
		int const iType = (true == bIsNewType)?(1):(0);

		XUI::CXUI_Wnd* pBgMain = pParent->GetControl(L"FRM_BACKGROUND");
		if( pBgMain )
		{
			for( int i = 0; i < 2; ++i )
			{
				BM::vstring vStr(L"SFRM_EVENT_ITEM_BG");
				vStr += i;

				XUI::CXUI_Wnd* pBgType = pBgMain->GetControl(vStr);
				if( pBgType )
				{
					pBgType->Visible(i == iType);
				}					
			}
		}

		for( int i = 0; i < 2; ++i )
		{
			BM::vstring vStr(L"SFRM_EVENT_TYPE");
			vStr += i;

			XUI::CXUI_Wnd* pReward = pParent->GetControl(vStr);
			if( pReward )
			{
				pReward->Visible(i == iType);
				if( i == iType )
				{
					pRewardMain = pReward;
				}
			}					
		}
	}

	if( !pRewardMain )
	{
		return false;
	}

	if( true == Result )
	{
		pTitle->Text(kInfo.kTitle);
		pContent->Text(kInfo.kDiscription);
		std::wstring strPeriod = TTW(1989);
		strPeriod += _T(" : ");
		strPeriod += GetTimeToString(kInfo.kStartTime);
		strPeriod += _T(" ~ ");
		strPeriod += GetTimeToString(kInfo.kEndTime);
		pPeriod->Text(strPeriod);
		pTakeBtn->SetCustomData(&EventKey, sizeof(EventKey));
		
		if( bIsNewType )
		{
			pTakeBtn->Text(TTW(1961));
			
			for( int i = 0; i < MAX_EVENT_ARG; ++i )
			{
				BM::vstring kStr(L"SFRM_ITEM");
				kStr += i;
				XUI::CXUI_Wnd* pItem = pRewardMain->GetControl(kStr);
				if( pItem ){ pItem->Visible(false); };
			}

			switch( kEvt.iEventType )
			{
			case ET_PCROOMEVENT_EFFECT:
				{
					pRewardMain->Text(TTW(19931));

					CONT_DEFEFFECT const *pkDefEffect = NULL;
					g_kTblDataMgr.GetContDef(pkDefEffect);
					if( !pkDefEffect )
					{//이펙트 컨테이너가 없다
						return false;
					}

					for( int i = 0; i < MAX_EVENT_ARG; ++i )
					{
						BM::vstring kStr(L"SFRM_ITEM");
						kStr += i;

						XUI::CXUI_Wnd* pItem = pRewardMain->GetControl(kStr);
						if( !pItem )
						{//정의가 잘못되있다
							continue;
						}

						pItem->Visible(false);

						int const& EffectID = kEvt.aIntData[i];
						if( EffectID )
						{
							const wchar_t* pName = NULL;

							CONT_DEFEFFECT::const_iterator iter = pkDefEffect->find(EffectID);
							if( pkDefEffect->end() == iter )
							{
								continue;
							}

							CONT_DEFEFFECT::mapped_type const& kElement = iter->second;
							GetDefString(kElement.iName, pName);

							if( NULL == pName )
							{//이펙트 이름이 없는건 쓸 수 없다.
								continue;
							}

							pItem->Visible(true);
							pItem->Text(pName);

							XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pItem->GetControl(L"ICN_ICON"));
							if( pIcon )
							{
								SIconInfo	Info(pIcon->IconInfo().iIconGroup, EffectID);
								pIcon->SetIconInfo(Info);
								{
									BM::Stream kPacket;
									kPacket.Push(EffectID);
									kPacket.Push(EffectID);
									lwUIWnd klwWnd(pIcon);
									klwWnd.SetCustomDataAsPacket(lwPacket(&kPacket));
								}
								//pIcon->SetCustomData(&EffectID, sizeof(EffectID));
							}
						}
					}
				}break;
			case ET_PCROOMEVENT_COUPON:
			default:
				{ 
					pRewardMain->Text(L""); 
				}break;
			}
		}
		else
		{
			pTakeBtn->Text(TTW(1987));

			CONT_EVENT_REWARD_LIST::iterator	Reward_iter = kReward.begin();
			for(int i = 0; i < 3; ++i)
			{
				BM::vstring	vStr(_T("FRM_REWARD"));
				vStr += i;

				XUI::CXUI_Wnd* pRewardText = pRewardMain->GetControl(vStr);
				if( !pRewardText )
				{
					assert(0);
					continue;
				}

				if( Reward_iter == kReward.end() )
				{
					pRewardText->Text(_T(""));
				}
				else
				{
					CONT_EVENT_REWARD_LIST::key_type const& kKey = Reward_iter->first;
					CONT_EVENT_REWARD_LIST::mapped_type const& kVal = Reward_iter->second;

					pRewardText->Text(kVal->kRewardTitle);

					for(int j = 0; j < MAX_COUPON_EVENT_REWARD; ++j)
					{
						BM::vstring vSubStr(_T("ICON_REWARD"));
						vSubStr += ((i * MAX_COUPON_EVENT_REWARD) + j);
						XUI::CXUI_Wnd* pIcon = pRewardMain->GetControl(vSubStr);
						vSubStr = _T("FRM_DISABLE");
						vSubStr += ((i * MAX_COUPON_EVENT_REWARD) + j);
						XUI::CXUI_Wnd* pDisable = pRewardMain->GetControl(vSubStr);
						if( !pIcon || !pDisable)
						{
							assert(0);
							continue;
						}

						int const iItemNo = kVal->iItemNo[j];
						if(iItemNo != 0)
						{
							pIcon->SetCustomData(&iItemNo, sizeof(iItemNo));
							pIcon->Visible(true);
							//pDisable->Visible(false);
						}
						else
						{
							pIcon->Visible(false);
							//pDisable->Visible(true);
						}
					}
					++Reward_iter;
				}
			}
		}
		return true;
	}
	else
	{
		pTitle->Text(_T(""));
		pContent->Text(_T(""));
		pPeriod->Text(_T(""));
		pTakeBtn->ClearCustomData();

		for(int i = 0; i < 3; ++i)
		{
			BM::vstring	vStr(_T("FRM_REWARD"));
			vStr += i;

			XUI::CXUI_Wnd* pRewardText = pRewardMain->GetControl(vStr);
			if( !pRewardText )
			{
				assert(0);
				continue;
			}
			pRewardText->Text(_T(""));

			for(int j = 0; j < MAX_COUPON_EVENT_REWARD; ++j)
			{
				BM::vstring vSubStr(_T("ICON_REWARD"));
				vSubStr += (i * MAX_COUPON_EVENT_REWARD) + j;
				XUI::CXUI_Wnd* pIcon = pRewardMain->GetControl(vSubStr);
				vSubStr = _T("FRM_DISABLE");
				vSubStr += ((i * MAX_COUPON_EVENT_REWARD) + j);
				XUI::CXUI_Wnd* pDisable = pRewardMain->GetControl(vSubStr);
				if( !pIcon || !pDisable)
				{
					assert(0);
					continue;
				}
				pIcon->Visible(false);
				//pDisable->Visible(true);
			}
		}
	}
	return false;
}

std::wstring const lwEventView::GetTimeToString(BM::PgPackedTime const& kTimeInfo)
{
	SYSTEMTIME	kTime = kTimeInfo;
	BM::vstring vStr(_T(""));

// 	switch(g_kLocal.ServiceRegion())
// 	{
// 	case LOCAL_MGR::NC_USA:
// 		{// mm/dd/yyyy
// 			vStr += kTime.wMonth;
// 			vStr += _T("/");	//vStr += TTW(173) + _T(" ");	
// 			vStr += kTime.wDay;
// 			vStr += _T("/");	//vStr += TTW(172) + _T(" ");	
// 			vStr += kTime.wYear;				
// 			vStr += _T(" ");	//vStr += TTW(174) + _T(" ");
// 			vStr += kTime.wHour;
// 			vStr += _T(":");
// 			vStr += kTime.wMinute;
// 		}break;
// 	default:
// 		{// yyyy/mm/dd
// 			vStr += kTime.wYear;
// 			vStr += _T("-");	//vStr += TTW(172) + _T(" ");	
// 			vStr += kTime.wMonth;
// 			vStr += _T("-");	//vStr += TTW(173) + _T(" ");	
// 			vStr += kTime.wDay;	
// 			vStr += _T(" ");	//vStr += TTW(174) + _T(" ");
// 			vStr += kTime.wHour;
// 			vStr += _T(":");
// 			vStr += kTime.wMinute;
// 		}break;
// 	}

	// yyyy/mm/dd
	vStr += kTime.wYear;
	vStr += _T("-");	//vStr += TTW(172) + _T(" ");	
	vStr += kTime.wMonth;
	vStr += _T("-");	//vStr += TTW(173) + _T(" ");	
	vStr += kTime.wDay;	
	vStr += _T(" ");	//vStr += TTW(174) + _T(" ");
	vStr += kTime.wHour;
	vStr += _T(":");
	vStr += kTime.wMinute;

	return (std::wstring const&)vStr;
}

void lwEventView::ReceivePacket(WORD const iPacketType, BM::Stream& rkPacket)
{
	switch(iPacketType)
	{
	case PT_M_C_ANS_COUPON_EVENT_SYNC:
		{
			g_kEventViewClient.ReadFromPacket(rkPacket);

			BM::Stream	kPacket;
			kPacket.Push(PT_C_M_REQ_EVENT_LIST);
			NETWORK_SEND(kPacket);
		}break;
	case PT_M_C_ANS_EVENT_LIST:
		{
			g_kEventViewClient.ReadFromPacketToServerEvent(rkPacket);
			XUI::CXUI_Wnd* pParent = XUIMgr.Activate(_T("SFRM_EVENT"));
			if( !pParent )
			{
				return;
			}

			lwRegistUIAction(lwUIWnd(pParent), "CloseUI");
			int iEventKey = SetEventList(pParent);
			ViewSelectEventInfo(iEventKey, pParent, false);
		}break;
	case PT_M_C_ANS_JOIN_EVENT:
		{
			HRESULT	hResult = S_OK;
			rkPacket.Pop(hResult);
			if( S_OK != hResult )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1962 + hResult - E_ME_INVALID_ACTION, true);
			}
		}break;
	default:
		{
		}break;
	}
}

void PgEventViewClient::ReadFromPacket(BM::Stream& kPacket)
{
	PgCouponEventView::ReadFromPacket(kPacket);
	BM::CAutoMutex kLockFree(m_kMutex);

	if( m_kContCouponEvent.empty() )
	{
		return;
	}

	m_kEventCache.clear();
	CONT_COUPON_EVENT_INFO::iterator	Info_iter = m_kContCouponEvent.begin();
	while( Info_iter != m_kContCouponEvent.end() )
	{
		SEventContIterator	IterInfo;		
		IterInfo.Info_iter	= Info_iter;

		CONT_COUPON_EVENT_REWARD::iterator	Reward_iter = m_kContCouponEventReward.begin();
		while( Reward_iter != m_kContCouponEventReward.end() )
		{
			if( Reward_iter->iEventKey == Info_iter->iEventKey )
			{
				std::pair<CONT_EVENT_REWARD_LIST::iterator, bool> Rst = IterInfo.RewardList.insert(std::make_pair(Reward_iter->iIDX, Reward_iter));
				if( !Rst.second )
				{//등급도 중복일 순 없다
				}
			}
			++Reward_iter;
		}

		std::pair<CONT_CP_EVENT_CACHE::iterator, bool> Rst = m_kEventCache.insert(std::make_pair(Info_iter->iEventKey, IterInfo));
		if( !Rst.second )
		{//이벤트키가 중복일리 없다
		}
		++Info_iter;
	}
}

bool PgEventViewClient::GetActivatedCouponEvent(CONT_EVENT_TINY_INFO_LIST& List)
{
	BM::CAutoMutex kLockFree(m_kMutex);

	CONT_CP_EVENT_CACHE::iterator	iter = m_kEventCache.begin();
	while( iter != m_kEventCache.end() )
	{
		if( iter->second.Info_iter == m_kContCouponEvent.end() )
//		||  iter->second.Reward_iter == m_kContCouponEventReward.end() )
		{
			return false;
		}

		__int64 const i64CurTime = g_kEventView.GetLocalSecTime();
		__int64 i64StartTime = 0,
				i64EndTime = 0;

		CGameTime::DBTimeEx2SecTime(BM::DBTIMESTAMP_EX(iter->second.Info_iter->kStartTime), i64StartTime);
		CGameTime::DBTimeEx2SecTime(BM::DBTIMESTAMP_EX(iter->second.Info_iter->kEndTime), i64EndTime);

		if((i64StartTime < i64CurTime) && (i64CurTime < i64EndTime))
		{
			std::pair<CONT_EVENT_TINY_INFO_LIST::iterator, bool> Rst = List.insert(std::make_pair(iter->first, iter->second.Info_iter->kTitle));
			if( !Rst.second )
			{//이벤트키가 중복일리 없다
				return false;
			}
		}
		++iter;
	}
	return !List.empty();
}

bool PgEventViewClient::GetKeyMatchInfo(int const& EventKey, SCOUPON_EVENT_INFO& Info, CONT_EVENT_REWARD_LIST& Reward)
{
	BM::CAutoMutex kLockFree(m_kMutex);

	CONT_CP_EVENT_CACHE::iterator	iter = m_kEventCache.find(EventKey);
	if( iter == m_kEventCache.end() )
	{
		return false;
	}

	if( iter->second.Info_iter == m_kContCouponEvent.end() )
//	||  iter->second.Reward_iter == m_kContCouponEventReward.end() )
	{
		return false;
	}

	Info = *iter->second.Info_iter;
	Reward.insert(iter->second.RewardList.begin(), iter->second.RewardList.end());
	return true;
}

void PgEventViewClient::ReadFromPacketToServerEvent(BM::Stream& kPacket)
{
	m_kServerEventContainer.clear();
	PU::TLoadArray_M(kPacket, m_kServerEventContainer);
}

bool PgEventViewClient::GetActivatedServerEvent(CONT_EVENT_TINY_INFO_LIST& List)
{
	BM::CAutoMutex kLockFree(m_kMutex);

	CONT_EVENT_LIST::iterator	iter = m_kServerEventContainer.begin();
	while( iter != m_kServerEventContainer.end() )
	{
		CONT_EVENT_LIST::value_type const&	kElement = (*iter);

		__int64 const i64CurTime = g_kEventView.GetLocalSecTime();
		__int64 i64StartTime = 0,
				i64EndTime = 0;

		CGameTime::DBTimeEx2SecTime(kElement.timeStartDate, i64StartTime);
		CGameTime::DBTimeEx2SecTime(kElement.timeEndDate, i64EndTime);

		if((i64StartTime < i64CurTime) && (i64CurTime < i64EndTime))
		{
			std::pair<CONT_EVENT_TINY_INFO_LIST::iterator, bool> Rst = List.insert(std::make_pair(kElement.iEventNo, kElement.kTitle));
			if( !Rst.second )
			{//이벤트키가 중복일리 없다
				return false;
			}
		}
		++iter;
	}
	return true;
}

bool PgEventViewClient::GetKeyMatchInfo(int const& EventKey, SCOUPON_EVENT_INFO& Info, TBL_EVENT& evt)
{
	CONT_EVENT_LIST::iterator	iter = m_kServerEventContainer.begin();
	while( iter != m_kServerEventContainer.end() )
	{
		CONT_EVENT_LIST::value_type const&	kElement = (*iter);

		if( kElement.iEventNo == EventKey )
		{
			Info.iEventKey = kElement.iEventNo;
			Info.kTitle = kElement.kTitle;
			Info.kDiscription = kElement.kDescription;
			Info.kStartTime = kElement.timeStartDate;
			Info.kEndTime = kElement.timeEndDate;
			evt = kElement;
			return true;
		}
		++iter;
	}
	return false;	
}
