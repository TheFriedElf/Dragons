#include "stdafx.h"
#include "PgMyHomeMgr.h"
#include "JobDispatcher.h"
#include "Global.h"
//#include <chrono>

PgMyHomeMgrImpl::PgMyHomeMgrImpl()
{
	m_kGlobalChatRoomGuid.Generate();
	m_kContChatRoom.insert(std::make_pair(m_kGlobalChatRoomGuid,SHomeChatRoom(m_kGlobalChatRoomGuid,L"",L"",L"")));
}

bool PgMyHomeMgrImpl::IsLocked(BM::GUID const kGuid) const
{
	return (m_kContLock.find(kGuid) != m_kContLock.end());
}

void PgMyHomeMgrImpl::Lock(BM::GUID const kGuid)
{
	std::pair<CONT_HOME_LOCK::iterator, bool> kPair = m_kContLock.insert(std::make_pair(kGuid,1));
	if(true == kPair.second)
	{
		return;
	}

	++(*kPair.first).second;
}

void PgMyHomeMgrImpl::Unlock(BM::GUID const kGuid)
{
	CONT_HOME_LOCK::iterator iter = m_kContLock.find(kGuid);
	if(iter == m_kContLock.end())
	{
		return;
	}
	
	--(*iter).second;

	if(0 < (*iter).second)
	{
		return;
	}

	m_kContLock.erase(iter);
}

bool PgMyHomeMgrImpl::GetHome(short const kStreetNo,int const kHouseNo,SMYHOME & kMyHome)const
{
	CONT_MYHOMEBUILDINGS::const_iterator iter = m_kCont.find(kStreetNo);
	if(iter == m_kCont.end())
	{
		return false;
	}

	CONT_MYHOMEBUILDINGS::mapped_type::const_iterator home_iter = (*iter).second.find(CONT_MYHOME::key_type(kStreetNo,kHouseNo));
	if(home_iter == (*iter).second.end())
	{
		return false;
	}

	kMyHome = (*home_iter).second;
	return true;
}

bool PgMyHomeMgrImpl::SetHome(SMYHOME const & kMyHome)
{
	SMYHOME kOldHome;
	if(false == GetHome(kMyHome.siStreetNo,kMyHome.iHouseNo,kOldHome))
	{
		return false;
	}
	m_kCont[kMyHome.siStreetNo][CONT_MYHOME::key_type(kMyHome.siStreetNo,kMyHome.iHouseNo)] = kMyHome;
	return true;
}

__int64 const BIDDING_DELAY_TIME = 5 * 60; // 재입찰 가능 시간

void PgMyHomeMgrImpl::ClearBiddingDelayTime()
{
	__int64 const i64CurTime = g_kEventView.GetLocalSecTime();

	// 재입찰 가능 시간이 지난 유저는 여기서 삭제 해준다.
	CONT_BIDDING_TIME::iterator iter = m_kContTime.begin();
	while(iter != m_kContTime.end())
	{
		if(i64CurTime >= (*iter).second)
		{
			iter = m_kContTime.erase(iter);
			continue;
		}

		++iter;
	}
}

bool PgMyHomeMgrImpl::AddBiddingTime(BM::GUID const & kOwnerGuid)
{
	__int64 const i64BiddingDelay = g_kEventView.GetLocalSecTime() + BIDDING_DELAY_TIME;

	// 여기서 아직 입찰자 리스트에 남아 있으면? 아직 시간 안된거지 더 기다려
	if(false == m_kContTime.insert(std::make_pair(kOwnerGuid,i64BiddingDelay)).second)
	{
		return false;
	}

	return true;
}

bool PgMyHomeMgrImpl::IsBidding(BM::GUID const & kOwnerGuid)
{
	return (m_kContBidder.find(kOwnerGuid) != m_kContBidder.end());
}

bool PgMyHomeMgrImpl::AddBidder(BM::GUID const & kOwnerGuid)
{
	return m_kContBidder.insert(kOwnerGuid).second;
}

void PgMyHomeMgrImpl::RemoveBidder(BM::GUID const & kOwnerGuid)
{
	m_kContBidder.erase(kOwnerGuid);
}

bool PgMyHomeMgrImpl::HasBiddingDelayTime(BM::GUID const & kOwnerGuid)
{
	return (m_kContTime.find(kOwnerGuid) != m_kContTime.end());
}

void PgMyHomeMgrImpl::ProcessSideJob(SMYHOME const & kMyHome,BM::PgPackedTime const & kCurTime)
{
	if(true == kMyHome.kOwnerGuid.IsNull())	// 주인 없는 집은 처리할 필요 없다.
	{
		return;
	}

	CONT_PLAYER_MODIFY_ORDER kOrder;

	CONT_DEFSIDEJOBRATE const *pkContDef = NULL;
	g_kTblDataMgr.GetContDef(pkContDef);

	if(NULL == pkContDef)
	{
		return;
	}

	for(CONT_MYHOME_SIDE_JOB::const_iterator iter = kMyHome.kContSideJob.begin();iter != kMyHome.kContSideJob.end();++iter)
	{
		if(kCurTime < (*iter).second.kEndTime)// 아르바이트 시간이 남아 있으면
		{
			continue;
		}

		CONT_DEFSIDEJOBRATE::const_iterator cont_iter = pkContDef->find((*iter).first);
		if(cont_iter == pkContDef->end()) // 테이블이 없어서 금액을 지급 못하는 경우? 일단 테이블 수정될 때 까지는 보관 해주자..
		{
			continue;
		}

		kOrder.push_back(SPMO(IMET_SIDEJOB_REMOVE, kMyHome.kHomeInSideGuid, SMOD_MyHome_SideJob_Remove(kMyHome.siStreetNo, kMyHome.iHouseNo, (*iter).first)));
		kOrder.push_back(SPMO(IMET_SIDEJOB_COMPLETE, kMyHome.kHomeInSideGuid, SMOD_MyHome_SideJob_Complete(kMyHome.kOwnerGuid, (*iter).first, kMyHome.siStreetNo, kMyHome.iHouseNo)));
	}

	if(true == kOrder.empty())
	{
		return;
	}

	SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
	pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
	pkActionOrder->kCause = CIE_Home_SideJob_Remove;
	pkActionOrder->kContOrder = kOrder;
	g_kJobDispatcher.VPush(pkActionOrder);
}

void PgMyHomeMgrImpl::ProcessBidding(SMYHOME const & kMyHome,BM::PgPackedTime const & kCurTime)
{
	if(kCurTime < kMyHome.kAuctionTime)			// 경매 시간이 남아 있으면 그냥 리턴....
	{
		return;
	}

	CONT_PLAYER_MODIFY_ORDER kOrder;

	kOrder.push_back(SPMO(IMET_MYHOME_AUCTION_END, kMyHome.kHomeInSideGuid,kMyHome));

	if(true == kMyHome.kBidderGuid.IsNotNull()) // 누군가 낙찰되었다.
	{
		kOrder.push_back( SPMO(IMET_MYHOME_RETURN_ITEM, kMyHome.kHomeInSideGuid,
			SMOD_MyHome_ReturnItem(kMyHome.kOwnerGuid, kMyHome.siStreetNo, 
			kMyHome.iHouseNo, RIT_SELL))); // 내부 아이템 돌려 보내
	}

	SActionOrder* pkActionOrder = PgJobWorker::AllocJob();

	pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
	pkActionOrder->kCause = CIE_Home_Auction_End;
	pkActionOrder->kContOrder = kOrder;
	g_kJobDispatcher.VPush(pkActionOrder);

	if(true == kMyHome.kBidderGuid.IsNotNull())
	{
		BM::Stream kPacket(PT_M_C_HOME_NOTI_BIDDING);
		kPacket.Push(kMyHome.siStreetNo);
		kPacket.Push(kMyHome.iHouseNo);
		SendToUser(kMyHome.kBidderGuid,kPacket,false);
	}
}

void PgMyHomeMgrImpl::ProcessAttachment(SMYHOME const & kMyHome,BM::PgPackedTime const & kCurTime)
{
	if(true == kMyHome.kOwnerGuid.IsNull())	// 주인 없는 집은 세금 처리할 필요 없다.
	{
		return;
	}

	__int64 i64PayTextTime = 0;
	CGameTime::DBTimeEx2SecTime(BM::DBTIMESTAMP_EX(kMyHome.kPayTexTime),i64PayTextTime);
	i64PayTextTime = g_kEventView.GetLocalSecTime() - i64PayTextTime;

	if(i64PayTextTime <= 0)
	{
		return;	// 세금 납부 기간이 되지 않았다.
	}

	CONT_PLAYER_MODIFY_ORDER kOrder;
	kOrder.push_back(SPMO(IMET_MYHOME_RESERVE,kMyHome.kHomeInSideGuid));// 내부 유닛 예약

	if(MAX_MYHOME_PAY_TEX_TIME <= i64PayTextTime) // 최대 납부 기간을 넘기면 강제로 집을 빼앗긴다.
	{
		CONT_DEFMYHOMEBUILDINGS const * pkCont = NULL;
		g_kTblDataMgr.GetContDef(pkCont);
		if(!pkCont)
		{
			return;
		}

		CONT_DEFMYHOMEBUILDINGS::const_iterator iter = pkCont->find(kMyHome.siStreetNo);
		if(iter == pkCont->end())
		{
			return;
		}

		if(0 == (*iter).second.iGrade)	// 등급이 0 이면 아파트 이다.
		{
			return;
		}

		__int64 const i64TexCost = MYHOMEUTIL::CalcTex(kMyHome);	// 접속해 있는 플레이어에게 집 차압 소식을 전한다. 
		BM::Stream kPacket(PT_M_C_HOME_NOTI_ATTACHMENT);
		kPacket.Push(kMyHome.siStreetNo);
		kPacket.Push(kMyHome.iHouseNo);
		kPacket.Push(i64TexCost);
		SendToUser(kMyHome.kOwnerGuid,kPacket,false);

		for(CONT_MYHOME_SIDE_JOB::const_iterator c_it = kMyHome.kContSideJob.begin();c_it != kMyHome.kContSideJob.end();++c_it)
		{
			kOrder.push_back(SPMO(IMET_SIDEJOB_REMOVE, kMyHome.kHomeInSideGuid, SMOD_MyHome_SideJob_Remove(kMyHome.siStreetNo, kMyHome.iHouseNo, (*c_it).first)));
			kOrder.push_back(SPMO(IMET_SIDEJOB_COMPLETE, kMyHome.kHomeInSideGuid, SMOD_MyHome_SideJob_Complete(kMyHome.kOwnerGuid, (*c_it).first, kMyHome.siStreetNo, kMyHome.iHouseNo)));
		}

		// 내부 아이템 돌려 주기
		kOrder.push_back(SPMO(IMET_MYHOME_RETURN_ITEM, kMyHome.kHomeInSideGuid,SMOD_MyHome_ReturnItem(kMyHome.kOwnerGuid,kMyHome.siStreetNo,kMyHome.iHouseNo, RIT_ATTACHMENT)));	// 내부 아이템 돌려 보내
		kOrder.push_back(SPMO(IMET_MYHOME_ATTACHMENT, kMyHome.kHomeInSideGuid,kMyHome));	// 홈에서 유저 정보 지워

		SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
		pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
		pkActionOrder->kCause = CIE_Home_Attachment;
		pkActionOrder->kContOrder = kOrder;
		g_kJobDispatcher.VPush(pkActionOrder);
		return;
	}

	if(kCurTime < kMyHome.kPayTexNotiTime) 
	{
		return; // 다음 독촉 메시지 전송시간이 지나지 않았다.
	}

	kOrder.push_back(SPMO(IMET_MYHOME_NOTI_PAY_TEX, kMyHome.kHomeInSideGuid,kMyHome));

	{
		SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
		pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
		pkActionOrder->kCause = CIE_Home_Noti_PayTex;
		pkActionOrder->kContOrder = kOrder;
		g_kJobDispatcher.VPush(pkActionOrder);
	}

	__int64 const i64TexCost = MYHOMEUTIL::CalcTex(kMyHome);	//세금...
	BM::Stream kPacket(PT_M_C_HOME_NOTI_TEX);
	kPacket.Push(kMyHome.siStreetNo);
	kPacket.Push(kMyHome.iHouseNo);
	kPacket.Push(i64TexCost);
	kPacket.Push(kMyHome.kPayTexTime);
	SendToUser(kMyHome.kOwnerGuid,kPacket,false);

	{// 장기 체납자
		SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
		pkActionOrder->InsertTarget(kMyHome.kOwnerGuid);
		pkActionOrder->kCause = CAE_Achievement;
		pkActionOrder->kContOrder.push_back(SPMO(IMET_SET_ABIL,kMyHome.kOwnerGuid,SPMOD_AddAbil(AT_ACHIEVEMENT_TEXTIME, static_cast<int>(i64PayTextTime))));
		g_kJobDispatcher.VPush(pkActionOrder);
	}
}

void PgMyHomeMgrImpl::Tick()
{
	ClearBiddingDelayTime();

	BM::DBTIMESTAMP_EX kLocalTime;
	g_kEventView.GetLocalTime(kLocalTime);

	BM::PgPackedTime const kCurTime = static_cast<BM::PgPackedTime>(kLocalTime);

	for(CONT_MYHOMEBUILDINGS::const_iterator town_iter = m_kCont.begin();town_iter != m_kCont.end();++town_iter)
	{
		for(CONT_MYHOME::const_iterator iter = (*town_iter).second.begin();iter != (*town_iter).second.end();++iter)
		{
			SMYHOME const & kMyHome = (*iter).second;
			switch(kMyHome.bAuctionState)
			{
			case MAS_NOT_BIDDING:
				{
					ProcessSideJob(kMyHome,kCurTime);
				}// break 걸지 말것 아래 동작은 MAS_NOT_BIDDING/MAS_IS_BLOCK 공통으로 사용 됨
			case MAS_IS_BLOCK:
				{
					ProcessAttachment(kMyHome,kCurTime);
				}break;
			case MAS_IS_BIDDING:
				{
					ProcessBidding(kMyHome,kCurTime);
				}break;
			}
		}
	}
}

HRESULT PgMyHomeMgrImpl::ProcessModifyPlayer(EItemModifyParentEventType const kCause, SPMO const &kOrder, PgDoc_Player* pkDocPlayer,BM::Stream const &kAddonPacket, BM::Stream & rkPacket, DB_ITEM_STATE_CHANGE_ARRAY &kChangeArray, PgContLogMgr & kContLogMgr)
{
	HRESULT hSubRet = E_FAIL;

	switch(kOrder.Cause())
	{
	case IMET_MYHOME_REMOVE:
		{
			SMOD_MyHome_MyHome_Set_Addr kData;
			kOrder.Read(kData);

			hSubRet = S_OK;

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_REMOVE,kOrder.Cause(), kOrder.OwnerGuid());
				kData.WriteToPacket(kDBChange.kAddonData);
				kDBChange.kAddonData.Push(kMyHome.kHomeInSideGuid);
				kChangeArray.push_back(kDBChange);
			}

			{
				SSendMailInfo kMailData;
				std::wstring kText;
				kMailData.FromGuid(kOrder.OwnerGuid());
				kMailData.TargetGuid(kOrder.OwnerGuid());
				GetDefString(MMC_SELL_NOTI_MAIL_FROM,kText);		kMailData.FromName(kText);
				GetDefString(MMC_SELL_NOTI_MAIL_TITLE,kText);		kMailData.MailTitle(kText);
				GetDefString(MMC_SELL_NOTI_MAIL_TEXT,kText);		kMailData.MailText(kText);

				tagDBItemStateChange kDBChange(DISCT_CREATE_MAIL_BYGUID, kOrder.Cause(), kOrder.OwnerGuid());
				kMailData.WriteToPacket(kDBChange.kAddonData);
				BM::GUID kMailGuid;// 로그 기록을 위해 메일 고유 아이디를 외부에서 제공한다.
				kMailGuid = BM::GUID::Create();
				kDBChange.kAddonData.Push(kMailGuid);
				kChangeArray.push_back(kDBChange);

				MYHOMEUTIL::MailLog(kOrder.OwnerGuid(),kMailGuid,kMailData,kContLogMgr);
			}

		}break;
	case IMET_MYHOME_ADD_VISITLOGCOUNT:
		{
			SMOD_MyHome_Sync_VisitLogCount kData;
			kOrder.Read(kData);

			hSubRet = S_OK;

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			tagDBItemStateChange kDBChange(DISCT_ADD_MYHOME_VISITLOGCOUNT,kOrder.Cause(), kOrder.OwnerGuid());
			kData.WriteToPacket(kDBChange.kAddonData);
			kChangeArray.push_back(kDBChange);
		}break;
	case IMET_MYHOME_SET_VISITLOGCOUNT:
		{
			SMOD_MyHome_Sync_VisitLogCount kData;
			kOrder.Read(kData);

			hSubRet = S_OK;

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			tagDBItemStateChange kDBData(DISCT_SET_MYHOME_VISITLOGCOUNT, kOrder.Cause(), kOrder.OwnerGuid());
			kData.WriteToPacket(kDBData.kAddonData);
			kChangeArray.push_back(kDBData);
		}break;
	case IMET_ADD_MYHOME:
		{
			SMOD_Add_MyHome kData;
			kOrder.Read(kData);

			hSubRet = S_OK;

			if(false == pkDocPlayer->HomeAddr().IsNull())
			{
				return E_MYHOME_ALREADY_HAVE;
			}

			if(true == IsBidding(pkDocPlayer->GetID()))
			{
				return E_MYHOME_CANT_BUY_BIDDING;
			}

			tagDBItemStateChange kDBData(DISCT_ADD_HOME, kOrder.Cause(), kOrder.OwnerGuid());
			kData.WriteToPacket(kDBData.kAddonData);
			kChangeArray.push_back(kDBData);
		}break;
	case IMET_MYHOME_SET_EQUIP_COUNT:
		{
			SMOD_MyHome_Sync_EquipItemCount kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			hSubRet = S_OK;

			tagDBItemStateChange kDBData(DISCT_SIDEJOB_EQUIPCOUNT, kOrder.Cause(), kOrder.OwnerGuid());
			kData.WriteToPacket(kDBData.kAddonData);
			kChangeArray.push_back(kDBData);
		}break;
	case IMET_SIDEJOB_INSERT:
		{
			SMOD_MyHome_SideJob_Insert kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			CONT_MYHOME_SIDE_JOB::const_iterator iter = kMyHome.kContSideJob.find(kData.SideJob());
			if(iter != kMyHome.kContSideJob.end())
			{
				return E_MYHOME_ALREADY_STARTED_JOB;
			}

			hSubRet = S_OK;

			{
				int const iAbilType = AT_HOME_SIDEJOB;
				int const iValue = pkDocPlayer->GetAbil(iAbilType) | kData.SideJob();
				pkDocPlayer->SetAbil(iAbilType, iValue);

				tagDBItemStateChange kDBData(DISCT_MODIFY_VALUE, kOrder.Cause(), kOrder.OwnerGuid());
				kDBData.IsQuery(false); // 저장 하지 않는다.
				kDBData.kAddonData.Push(iAbilType);
				kDBData.kAddonData.Push(static_cast<__int64>(iValue));
				kChangeArray.push_back(kDBData);
			}
			{
				tagDBItemStateChange kDBData(DISCT_SIDEJOB_INSERT, kOrder.Cause(), kOrder.OwnerGuid());
				kDBData.kAddonData.Push(pkDocPlayer->GetID());
				kData.WriteToPacket(kDBData.kAddonData);
				kChangeArray.push_back(kDBData);
			}
		}break;
	case IMET_SIDEJOB_REMOVE:
		{
			SMOD_MyHome_SideJob_Remove kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			CONT_MYHOME_SIDE_JOB::iterator iter = kMyHome.kContSideJob.find(kData.SideJob());
			if(iter == kMyHome.kContSideJob.end())
			{
				return E_MYHOME_NOT_FOUND_JOB;
			}

			hSubRet = S_OK;

			{
				int const iAbilType = AT_HOME_SIDEJOB;
				int const iValue = pkDocPlayer->GetAbil(iAbilType) & ~kData.SideJob();
				pkDocPlayer->SetAbil(iAbilType, iValue);

				tagDBItemStateChange kDBData(DISCT_MODIFY_VALUE, kOrder.Cause(), kOrder.OwnerGuid());
				kDBData.IsQuery(false); // 저장 하지 않는다.
				kDBData.kAddonData.Push(iAbilType);
				kDBData.kAddonData.Push(static_cast<__int64>(iValue));
				kChangeArray.push_back(kDBData);
			}
			{
				tagDBItemStateChange kDBData(DISCT_SIDEJOB_REMOVE, kOrder.Cause(), kOrder.OwnerGuid());
				kDBData.kAddonData.Push(pkDocPlayer->GetID());
				kData.WriteToPacket(kDBData.kAddonData);
				kChangeArray.push_back(kDBData);
			}
		}break;
	case IMET_SIDEJOB_COMPLETE:
		{
			SMOD_MyHome_SideJob_Complete kData;
			kOrder.Read(kData);

			hSubRet = S_OK;

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			CONT_MYHOME_SIDE_JOB::iterator iter = kMyHome.kContSideJob.find(kData.SideJob());
			if(iter == kMyHome.kContSideJob.end())
			{
				return E_MYHOME_NOT_FOUND_JOB;
			}

			CONT_DEFSIDEJOBRATE const *pkContDef = NULL;
			g_kTblDataMgr.GetContDef(pkContDef);

			if(NULL == pkContDef)
			{
				return E_FAIL;
			}

			CONT_DEFSIDEJOBRATE::const_iterator cont_iter = pkContDef->find(kData.SideJob());
			if(cont_iter == pkContDef->end()) // 테이블이 없어서 금액을 지급 못하는 경우? 일단 테이블 수정될 때 까지는 보관 해주자..
			{
				return E_FAIL;
			}

			__int64 const i64SellCost = (*iter).second.i64TotalSellCost * (static_cast<double>((*cont_iter).second.iPayRate)/static_cast<double>(ABILITY_RATE_VALUE));
			if(0 < i64SellCost)
			{
				SSendMailInfo kMailData;
				std::wstring kText;
				kMailData.FromGuid(kData.OwnerGuid());
				kMailData.TargetGuid(kData.OwnerGuid());
				GetDefString(MMC_SIDEJOB_NOTI_MAIL_FROM,kText);		kMailData.FromName(kText);
				GetDefString(MMC_SIDEJOB_NOTI_MAIL_TITLE,kText);	kMailData.MailTitle(kText);
				GetDefString(MMC_SIDEJOB_NOTI_MAIL_TEXT,kText);		kMailData.MailText(kText);
				kMailData.Money(i64SellCost);

				tagDBItemStateChange kDBChange(DISCT_CREATE_MAIL_BYGUID, kOrder.Cause(), kOrder.OwnerGuid());
				kMailData.WriteToPacket(kDBChange.kAddonData);

				BM::GUID kMailGuid;// 로그 기록을 위해 메일 고유 아이디를 외부에서 제공한다.
				kMailGuid = BM::GUID::Create();
				kDBChange.kAddonData.Push(kMailGuid);
				kChangeArray.push_back(kDBChange);

				MYHOMEUTIL::MailLog(kOrder.OwnerGuid(),kMailGuid,kMailData,kContLogMgr);
			}
		}break;
	case IMET_SIDEJOB_MODIFY:
		{
			SMOD_MyHome_SideJob_Modify kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			CONT_MYHOME_SIDE_JOB::iterator iter = kMyHome.kContSideJob.find(kData.SideJob());
			if(iter == kMyHome.kContSideJob.end())
			{
				return E_MYHOME_NOT_FOUND_JOB;
			}

			hSubRet = S_OK;

			{
				tagDBItemStateChange kDBData(DISCT_SIDEJOB_MODIFY, kOrder.Cause(), kOrder.OwnerGuid());
				kDBData.kAddonData.Push(pkDocPlayer->GetID());
				kData.WriteToPacket(kDBData.kAddonData);
				kChangeArray.push_back(kDBData);
			}
		}break;
	case IMET_MYHOME_SET_HOMEADDR:
		{
			SMOD_MyHome_MyHome_Set_Addr kData;
			kOrder.Read(kData);
			pkDocPlayer->HomeAddr(SHOMEADDR(kData.StreetNo(),kData.HouseNo()));

			hSubRet = S_OK;

			tagDBItemStateChange kDBChange(DISCT_SET_HOME_ADDR,kOrder.Cause(), kOrder.OwnerGuid());
			kData.WriteToPacket(kDBChange.kAddonData);
			kChangeArray.push_back(kDBChange);

			PgLog kLog(ELOrderMain_Home_Addr,ELOrderSub_None);
			kLog.Set(0,static_cast<int>(kData.StreetNo()));
			kLog.Set(1,static_cast<int>(kData.HouseNo()));
			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);
		}break;
	case IMET_MYHOME_RESERVE:
		{
			// 이녀석은 정말 아무것도 하지 않는다 유닛 잠금을 확보하기 위해서 사용한다. 무조건 성공 처리 
			hSubRet = S_OK;
		}break;
	case IMET_MYHOME_SET_HOME_STATE:
		{
			SMOD_MyHome_MyHome_Set_State kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			kMyHome.bAuctionState = kData.State();

			hSubRet = S_OK;

			{
				pkDocPlayer->SetAbil(AT_MYHOME_STATE,kData.State());
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_STATE,kOrder.Cause(), kOrder.OwnerGuid());
				kDBChange.kAddonData.Push(kMyHome.bAuctionState);
				kChangeArray.push_back(kDBChange);
			}
			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
				kMyHome.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);
			}
		}break;
	case IMET_MYHOME_SET_OWNER_INFO:
		{
			SMOD_MyHome_MyHome_Set_OwnerInfo kData;
			kOrder.Read(kData);

			PgDoc_MyHome * pkDocHome = dynamic_cast<PgDoc_MyHome *>(pkDocPlayer);
			if(pkDocHome)
			{
				pkDocHome->OwnerGuid(kData.OwnerGuid());
				pkDocHome->OwnerName(kData.OwnerName());
			}

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			kMyHome.kOwnerGuid = kData.OwnerGuid();
			kMyHome.kName = kData.OwnerName();

			hSubRet = S_OK;

			{
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_OWNER_INFO,kOrder.Cause(), kOrder.OwnerGuid());
				kData.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);
			}
			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
				kMyHome.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);
			}
		}break;
	case IMET_MYHOME_TEX_TIME:
		{
			SMOD_MyHome_Modify_Time kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			kMyHome.kPayTexTime = kMyHome.kPayTexTime = kData.Time();
			kMyHome.kPayTexNotiTime = kMyHome.kPayTexTime = kData.Time();

			hSubRet = S_OK;

			tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
			kMyHome.WriteToPacket(kDBChange.kAddonData);
			kChangeArray.push_back(kDBChange);

			PgLog kLog(ELOrderMain_Home_TexTime,ELOrderSub_Modify);
			kLog.Set(0,pkDocPlayer->Name());
			kLog.Set(2,pkDocPlayer->GetID().str().c_str());
			kLog.Set(0,static_cast<int>(kData.StreetNo()));
			kLog.Set(1,static_cast<int>(kData.HouseNo()));

			__int64 i64TexTime = 0;
			CGameTime::DBTimeEx2SecTime(BM::DBTIMESTAMP_EX(kMyHome.kPayTexTime),i64TexTime);
			kLog.Set(0,i64TexTime);

			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);

		}break;
	case IMET_MYHOME_AUCTION_TIME:
		{
			SMOD_MyHome_Modify_Time kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			kMyHome.kAuctionTime.SetLocalTime();// 강제로 종료 시킨다.
			hSubRet = S_OK;

			tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
			kMyHome.WriteToPacket(kDBChange.kAddonData);
			kChangeArray.push_back(kDBChange);

			PgLog kLog(ELOrderMain_Home_AuctionTime,ELOrderSub_Modify);
			kLog.Set(0,pkDocPlayer->Name());
			kLog.Set(2,pkDocPlayer->GetID().str().c_str());

			kLog.Set(0,static_cast<int>(kData.StreetNo()));
			kLog.Set(1,static_cast<int>(kData.HouseNo()));

			__int64 i64AuctionTime = 0;
			CGameTime::DBTimeEx2SecTime(BM::DBTIMESTAMP_EX(kMyHome.kAuctionTime),i64AuctionTime);
			kLog.Set(0,i64AuctionTime);

			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);
		}break;
	case IMET_MYHOME_STYLE_MODIFY:
		{
			SMOD_SetHomeStyle kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			if(kMyHome.kOwnerGuid != kData.OwnerGuid())	// 집주인이 아니면 안됨
			{
				return E_MYHOME_NOT_HAVE_AUTHORITY;
			}

			switch(kData.EquipPos())
			{
			case EQUIP_POS_HOME_COLOR:
				{
					kMyHome.iHomeColor = kData.ItemNo();
				}break;
			case EQUIP_POS_HOME_STYLE:
				{
					kMyHome.iHomeStyle = kData.ItemNo();
				}break;
			case EQUIP_POS_HOME_FENCE:
				{
					kMyHome.iHomeFence = kData.ItemNo();
				}break;
			case EQUIP_POS_HOME_GARDEN:
				{
					kMyHome.iHomeGarden = kData.ItemNo();
				}break;
			case EQUIP_POS_ROOM_WALL:
				{
					kMyHome.iRoomWall = kData.ItemNo();
				}break;
			case EQUIP_POS_ROOM_FLOOR:
				{
					kMyHome.iRoomFloor = kData.ItemNo();
				}break;
			default:
				{
					return E_MYHOME_INVALID_EQUIP_POS;
				}break;
			}

			pkDocPlayer->SetDefaultItem(kData.EquipPos(),kData.ItemNo());

			hSubRet = S_OK;

			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
				kMyHome.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);	
			}
			
			{
				pkDocPlayer->SetDefaultItem(kData.EquipPos(),kData.ItemNo());
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_DEFAULT_ITEM,kOrder.Cause(), kOrder.OwnerGuid());
				kData.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);	
			}

			PgLog kLog(ELOrderMain_Home_StyleItem,ELOrderSub_Modify);
			kLog.Set(0,pkDocPlayer->Name());

			kLog.Set(2,pkDocPlayer->GetID().str().c_str());

			kLog.Set(0,static_cast<int>(kData.StreetNo()));
			kLog.Set(1,static_cast<int>(kData.HouseNo()));
			kLog.Set(2,static_cast<int>(kData.EquipPos()));
			kLog.Set(3,static_cast<int>(kData.ItemNo()));

			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);

			if(CIE_Home_Unit_Sync == kCause)
			{
				break;
			}

			SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
			pkActionOrder->kCause = CIE_Home_Unit_Sync;
			pkActionOrder->kContOrder.push_back(SPMO(IMET_MYHOME_STYLE_MODIFY, kMyHome.kHomeInSideGuid,kData));	// 다른 홈유닛 정보 동기화
			g_kJobDispatcher.VPush(pkActionOrder);
		}break;
	case IMET_MYHOME_PAY_TEX:
		{
			SMOD_MyHome_PayTex kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			kMyHome.bAuctionState = MAS_NOT_BIDDING;

			__int64 i64PayTime = 0;
			CGameTime::DBTimeEx2SecTime(BM::DBTIMESTAMP_EX(kMyHome.kPayTexTime),i64PayTime);
			__int64 const i64CurTime = g_kEventView.GetLocalSecTime();

			__int64 i64NextPayTexTime = 0;

			if(i64PayTime < i64CurTime)
			{
				i64NextPayTexTime = i64CurTime + MYHOME_PAY_TEX_TIME;
			}
			else
			{
				i64NextPayTexTime = i64PayTime + MYHOME_PAY_TEX_TIME;
			}

			BM::DBTIMESTAMP_EX kNextTexPayTime;
			CGameTime::SecTime2DBTimeEx(i64NextPayTexTime,kNextTexPayTime);

			kMyHome.kPayTexTime = static_cast<BM::PgPackedTime>(kNextTexPayTime);
			kMyHome.kPayTexNotiTime = kMyHome.kPayTexTime;

			hSubRet = S_OK;
			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
				kMyHome.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);
			}
			{
				pkDocPlayer->SetAbil(AT_MYHOME_STATE,kMyHome.bAuctionState);
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_STATE,kOrder.Cause(), kMyHome.kHomeInSideGuid);
				kDBChange.kAddonData.Push(kMyHome.bAuctionState);
				kChangeArray.push_back(kDBChange);
			}

			PgLog kLog(ELOrderMain_Home_PayTex,ELOrderSub_None);
			kLog.Set(0,pkDocPlayer->Name());
			
			kLog.Set(2,pkDocPlayer->GetID().str().c_str());

			kLog.Set(0,static_cast<int>(kData.StreetNo()));
			kLog.Set(1,static_cast<int>(kData.HouseNo()));
			kLog.Set(3,static_cast<int>(kMyHome.bAuctionState));

			kLog.Set(0,i64PayTime);			// 이전 세금 납부 시간
			kLog.Set(0,i64NextPayTexTime);	// 다음 세금 납부 시간

			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);

		}break;
	case IMET_MYHOME_ATTACHMENT:
		{
			SMYHOME kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.siStreetNo,kData.iHouseNo,kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			if(MAS_IS_BIDDING == kMyHome.bAuctionState)
			{
				return E_MYHOME_ALREADY_REG_AUCTION;
			}

			BM::GUID kOldOwnerGuid = kMyHome.kOwnerGuid;

			__int64 const i64TexCost = MYHOMEUTIL::CalcTex(kMyHome);	// 접속해 있는 플레이어에게 집 차압 소식을 전한다. 

			kMyHome.bAuctionState = MAS_IS_BIDDING;											// 자동으로 경매에 등록
			kMyHome.kBidderGuid.Clear();													// 경매 참가자 삭제
			kMyHome.kOwnerGuid.Clear();														// 집주인 guid 삭제
			kMyHome.bEnableVisitBit = 0;													// 방문자 옵션 모두 초기화
			kMyHome.kName = std::wstring();													// 집주인 이름 삭제

			__int64 const i64BiddingCost = (((kMyHome.i64HomePrice - (kMyHome.i64HomePrice * MATR_ATTACHMENT)/100) + UPTOGOLD)/10000)*10000;// 경매 등록 금액 설정
			kMyHome.i64FirstBiddingCost = kMyHome.i64LastBiddingCost = i64BiddingCost;

			__int64 const i64AuctionTime = g_kEventView.GetLocalSecTime() + DEFAULT_MYHOME_AUCTION_REG_TIME;
			BM::DBTIMESTAMP_EX kAuctionTime;
			CGameTime::SecTime2DBTimeEx(i64AuctionTime,kAuctionTime);
			kMyHome.kAuctionTime = static_cast<BM::PgPackedTime>(kAuctionTime); // 경매 종료 시간 다시 설정

			{
				SSendMailInfo kMailData;
				std::wstring kText;
				kMailData.FromGuid(kOldOwnerGuid);
				kMailData.TargetGuid(kOldOwnerGuid);
				GetDefString(MMC_ATTACHMENT_NOTI_MAIL_FROM,kText);		kMailData.FromName(kText);
				GetDefString(MMC_ATTACHMENT_NOTI_MAIL_TITLE,kText);		kMailData.MailTitle(kText);
				GetDefString(MMC_ATTACHMENT_NOTI_MAIL_TEXT,kText);		kMailData.MailText(kText);

				tagDBItemStateChange kDBChange(DISCT_CREATE_MAIL_BYGUID, kOrder.Cause(), kOrder.OwnerGuid());
				kMailData.WriteToPacket(kDBChange.kAddonData);
				BM::GUID kMailGuid;// 로그 기록을 위해 메일 고유 아이디를 외부에서 제공한다.
				kMailGuid = BM::GUID::Create();
				kDBChange.kAddonData.Push(kMailGuid);
				kChangeArray.push_back(kDBChange);

				MYHOMEUTIL::MailLog(kOrder.OwnerGuid(),kMailGuid,kMailData,kContLogMgr);
			}

			hSubRet = S_OK;

			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_INVITATION_CLEAR,kOrder.Cause(), kOrder.OwnerGuid()); // 차압 당하면 그동안 발송한 초청장은 모두 삭제 한다.
				kDBChange.kAddonData.Push(kData.siStreetNo);
				kDBChange.kAddonData.Push(kData.iHouseNo);
				kChangeArray.push_back(kDBChange);
			}

			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
				kMyHome.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);
			}

			SMOD_MyHome_MyHome_Set_OwnerInfo kHomeOwnerInfo(kMyHome.siStreetNo,kMyHome.iHouseNo,BM::GUID::NullData(),std::wstring());

			PgDoc_MyHome * pkDocHome = dynamic_cast<PgDoc_MyHome *>(pkDocPlayer);
			if(pkDocHome)
			{
				BM::GUID const kPreOwner(pkDocHome->OwnerGuid());
				pkDocHome->OwnerGuid(BM::GUID::NullData());
				pkDocHome->OwnerName(std::wstring());
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_OWNER_INFO,kOrder.Cause(), kMyHome.kHomeInSideGuid);
				kHomeOwnerInfo.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);

				//친구 갱신 요청
				{
					BM::Stream kRefresh(PT_N_N_NFY_COMMUNITY_STATE_HOMEADDR_FRIEND);
					kRefresh.Push(kPreOwner);
					kRefresh.Push(kData.siStreetNo);
					kRefresh.Push(kData.iHouseNo);
					::SendToFriendMgr(kRefresh);
				}
				{//길드 갱신 요청
					BM::Stream kRefresh(PT_N_N_NFY_COMMUNITY_STATE_HOMEADDR_GUILD);
					kRefresh.Push(kPreOwner);
					kRefresh.Push(kData.siStreetNo);
					kRefresh.Push(kData.iHouseNo);
					::SendToGuildMgr(kRefresh);
				}
				{//커플 갱신 요청
					BM::Stream kRefresh(PT_N_N_NFY_COMMUNITY_STATE_HOMEADDR_COUPLE);
					kRefresh.Push(kPreOwner);
					kRefresh.Push(kData.siStreetNo);
					kRefresh.Push(kData.iHouseNo);
					::SendToCoupleMgr(kRefresh);
				}
				{//파티 갱신 요청
					//데드락 때문에 위치 수정

					SHOMEADDR addr(kData.siStreetNo, kData.iHouseNo);
					BM::Stream kRefresh(PT_N_T_NFY_COMMUNITY_STATE_HOMEADDR_PARTY);
					kRefresh.Push(kPreOwner);
					kRefresh.Push(addr);
					SendToMyhomeMgr(kRefresh);
	
					/*SContentsUser rkUser;
					if(S_OK ==  GetPlayerByGuid(kPreOwner, false, rkUser) )
					{
						BM::Stream kRefresh(PT_N_T_NFY_COMMUNITY_STATE_HOMEADDR_PARTY);
						kRefresh.Push(rkUser.kCharGuid);
						kRefresh.Push(kData.siStreetNo);
						kRefresh.Push(kData.iHouseNo);
						::SendToGlobalPartyMgr(rkUser.sChannel, kRefresh);
					}*/
				}
			}

			{
				pkDocPlayer->SetAbil(AT_MYHOME_STATE,kMyHome.bAuctionState);
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_STATE,kOrder.Cause(), kMyHome.kHomeInSideGuid);
				kDBChange.kAddonData.Push(kMyHome.bAuctionState);
				kChangeArray.push_back(kDBChange);
			}

			SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
			pkActionOrder->kCause = CIE_Home_Attachment;
			pkActionOrder->kContOrder.push_back(SPMO(IMET_MYHOME_SET_HOMEADDR, kOldOwnerGuid,SMOD_MyHome_MyHome_Set_Addr(0,0)));	// 유저 주소 정보 설정
			g_kJobDispatcher.VPush(pkActionOrder);

			PgLog kLog(ELOrderMain_Home_Attachment,ELOrderSub_None);
			kLog.Set(0,pkDocPlayer->Name());
			
			kLog.Set(2,pkDocPlayer->GetID().str().c_str());

			kLog.Set(0,static_cast<int>(kData.siStreetNo));
			kLog.Set(1,static_cast<int>(kData.iHouseNo));
			kLog.Set(2,static_cast<int>(kMyHome.bAuctionState));

			kLog.Set(0,i64BiddingCost);		// 강제 경매 등록 금액
			kLog.Set(0,i64AuctionTime);		// 경매 종료 시간

			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);

		}break;
	case IMET_MYHOME_RETURN_ITEM:
		{
			SMOD_MyHome_ReturnItem kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			int iFromNo = 0;
			int iTitleNo = 0;

			switch(kData.ReturnType())
			{
			case RIT_ATTACHMENT:
				{
					iFromNo = MMC_ATTACHMENT_NOTI_MAIL_FROM;
					iTitleNo = MMC_ATTACHMENT_NOTI_MAIL_TITLE;
				}break;
			case RIT_SELL:
				{
					iFromNo = MMC_SELL_NOTI_MAIL_FROM;
					iTitleNo = MMC_SELL_NOTI_MAIL_TITLE;
				}break;
			}

			std::wstring kFrom,kText;
			GetDefString(iFromNo,kFrom);
			GetDefString(iTitleNo,kText);

			PgInventory * pkInv = pkDocPlayer->GetInven();
			for(int i = 0;i < pkInv->GetMaxIDX(IT_HOME);++i)
			{
				PgBase_Item kItem;
				SItemPos kPos(IT_HOME,i);
				if(S_OK != pkInv->GetItem(kPos,kItem))
				{
					continue;
				}

				SSendMailInfo kMailData;
				kMailData.FromGuid(kData.OwnerGuid());
				kMailData.TargetGuid(kData.OwnerGuid());
				kMailData.FromName(kFrom);
				kMailData.MailTitle(kText);
				kMailData.ItemGuid(kItem.Guid());

				hSubRet = pkDocPlayer->ItemProcess(SPMO(IMET_MODIFY_POS,kData.OwnerGuid(),SPMOD_Modify_Pos(SItemPos(IT_POST,0),kPos, PgBase_Item::NullData(), kItem)), kChangeArray, kContLogMgr );
				if(S_OK != hSubRet)
				{
					return E_MYHOME_CANT_MODIFY;
				}

				tagDBItemStateChange kDBChange(DISCT_CREATE_MAIL_BYGUID, kOrder.Cause(), kData.OwnerGuid());
			
				kMailData.WriteToPacket(kDBChange.kAddonData);

				BM::GUID kMailGuid;// 로그 기록을 위해 메일 고유 아이디를 외부에서 제공한다.
				kMailGuid = BM::GUID::Create();

				kDBChange.kAddonData.Push(kMailGuid);

				kChangeArray.push_back(kDBChange);

				MYHOMEUTIL::MailLog(kOrder.OwnerGuid(),kMailGuid,kMailData,kContLogMgr);
			}

			hSubRet = S_OK;
		}break;
	case IMET_MYHOME_NOTI_PAY_TEX:
		{
			SMYHOME kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.siStreetNo,kData.iHouseNo,kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			__int64 const i64NextNotiTime = g_kEventView.GetLocalSecTime() + MYHOME_PAY_TEX_TIME;
			BM::DBTIMESTAMP_EX kNextNotiTime;
			CGameTime::SecTime2DBTimeEx(i64NextNotiTime,kNextNotiTime);
			kMyHome.kPayTexNotiTime = static_cast<BM::PgPackedTime>(kNextNotiTime);
			kMyHome.bAuctionState = MAS_IS_BLOCK;

			{
				SSendMailInfo kMailData;
				std::wstring kText;
				kMailData.FromGuid(kMyHome.kOwnerGuid);
				kMailData.TargetGuid(kMyHome.kOwnerGuid);
				GetDefString(MMC_TEX_NOTI_MAIL_FROM,kText);		kMailData.FromName(kText);
				GetDefString(MMC_TEX_NOTI_MAIL_TITLE,kText);	kMailData.MailTitle(kText);
				GetDefString(MMC_TEX_NOTI_MAIL_TEXT,kText);		kMailData.MailText(kText);

				tagDBItemStateChange kDBChange(DISCT_CREATE_MAIL_BYGUID, kOrder.Cause(), kOrder.OwnerGuid());
				kMailData.WriteToPacket(kDBChange.kAddonData);
				BM::GUID kMailGuid;// 로그 기록을 위해 메일 고유 아이디를 외부에서 제공한다.
				kMailGuid = BM::GUID::Create();
				kDBChange.kAddonData.Push(kMailGuid);
				kChangeArray.push_back(kDBChange);

				MYHOMEUTIL::MailLog(kOrder.OwnerGuid(),kMailGuid,kMailData,kContLogMgr);
			}

			hSubRet = S_OK;

			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
				kMyHome.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);
			}

			{
				pkDocPlayer->SetAbil(AT_MYHOME_STATE,kMyHome.bAuctionState);
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_STATE,kOrder.Cause(), kMyHome.kHomeInSideGuid);
				kDBChange.kAddonData.Push(kMyHome.bAuctionState);
				kChangeArray.push_back(kDBChange);
			}
		}break;
	case IMET_MYHOME_AUCTION_UNREG:
		{
			SMOD_MyHome_Auction_Unreg kData;
			kOrder.Read(kData);
		
			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			if(MAS_IS_BIDDING != kMyHome.bAuctionState)
			{
				return E_MYHOME_IS_NOT_AUCTION_ARITCLE;
			}

			if(true == kMyHome.kBidderGuid.IsNotNull())
			{
				RemoveBidder(kMyHome.kBidderGuid);

				SSendMailInfo kMailData;
				std::wstring kText;
				kMailData.FromGuid(kMyHome.kBidderGuid);
				kMailData.TargetGuid(kMyHome.kBidderGuid);
				GetDefString(MMT_BIDDING_RETURN_MAIL_FROM,kText);
				kMailData.FromName(kText);
				GetDefString(MMT_BIDDING_RETURN_MAIL_TITLE,kText);
				kMailData.MailTitle(kText);
				GetDefString(MMT_BIDDING_RETURN_MAIL_TEXT,kText);
				kMailData.MailText(kText);
				kMailData.Money(kMyHome.i64LastBiddingCost);	// 이전 입찰자의 입찰금을 메일로 돌려 보내 줘야 한다.
				kMailData.PaymentType(0);

				tagDBItemStateChange kDBChange(DISCT_CREATE_MAIL_BYGUID, kOrder.Cause(), kMyHome.kBidderGuid);
				
				kMailData.WriteToPacket(kDBChange.kAddonData);

				BM::GUID kMailGuid;// 로그 기록을 위해 메일 고유 아이디를 외부에서 제공한다.
				kMailGuid = BM::GUID::Create();

				kDBChange.kAddonData.Push(kMailGuid);

				kChangeArray.push_back(kDBChange);

				MYHOMEUTIL::MailLog(kOrder.OwnerGuid(),kMailGuid,kMailData,kContLogMgr);

				BM::Stream kNotiPacket(PT_M_C_HOME_NOTI_AUCTION_CANCELED);
				kNotiPacket.Push(kData.StreetNo());
				kNotiPacket.Push(kData.HouseNo());
				SendToUser(kMyHome.kBidderGuid,kNotiPacket,false);
			}

			kMyHome.bAuctionState = MAS_NOT_BIDDING;	// 경매 모드 변경	
			kMyHome.kBidderGuid.Clear();				// 입찰자 정보 초기화
			kMyHome.i64FirstBiddingCost = 0;			// 입찰 금액 추기화
			kMyHome.i64LastBiddingCost = 0;				// 입찰 금액 추기화

			hSubRet = S_OK;

			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
				kMyHome.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);
			}
			{
				pkDocPlayer->SetAbil(AT_MYHOME_STATE,kMyHome.bAuctionState);
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_STATE,kOrder.Cause(), kMyHome.kHomeInSideGuid);
				kDBChange.kAddonData.Push(kMyHome.bAuctionState);
				kChangeArray.push_back(kDBChange);
			}

			PgLog kLog(ELOrderMain_Home_Auction,ELOrderSub_Auction_Cancel);
			kLog.Set(0,static_cast<int>(kData.StreetNo()));
			kLog.Set(1,static_cast<int>(kData.HouseNo()));
			kLog.Set(2,static_cast<int>(kMyHome.bAuctionState));

			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);
		}break;
	case IMET_MYHOME_AUCTION_END:
		{
			SMYHOME kData;
			kOrder.Read(kData);
		
			SMYHOME kMyHome;
			if(false == GetHome(kData.siStreetNo,kData.iHouseNo,kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			kMyHome.bAuctionState = MAS_NOT_BIDDING;

			BM::GUID kOldOwnerGuid = kMyHome.kOwnerGuid;

			if(true == kMyHome.kBidderGuid.IsNotNull()) // 누군가 낙찰되었다.
			{
				RemoveBidder(kMyHome.kBidderGuid);

				{
					SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
					pkActionOrder->kCause = CIE_Home_Auction_End;
					pkActionOrder->kContOrder.push_back(SPMO(IMET_MYHOME_SET_HOMEADDR, kMyHome.kBidderGuid,SMOD_MyHome_MyHome_Set_Addr(kMyHome.siStreetNo,kMyHome.iHouseNo)));	// 유저 주소 정보 설정
					g_kJobDispatcher.VPush(pkActionOrder);
				}
				
				{// 업적..
					SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
					pkActionOrder->InsertTarget(kMyHome.kBidderGuid);
					pkActionOrder->kCause = CAE_Achievement;
					pkActionOrder->kContOrder.push_back(SPMO(IMET_ADD_ABIL,kMyHome.kBidderGuid,SPMOD_AddAbil(AT_ACHIEVEMENT_BIDDING,1)));
					g_kJobDispatcher.VPush(pkActionOrder);
				}

				kMyHome.kOwnerGuid = kMyHome.kBidderGuid;	// 주인 변경됨
				kMyHome.kBidderGuid.Clear();				// 입찰자 정보 삭제
				kMyHome.i64HomePrice = kMyHome.i64LastBiddingCost;// 이 집의 판매 금액 변경
				kMyHome.i64FirstBiddingCost = 0;
				kMyHome.i64LastBiddingCost = 0;
				kMyHome.kName = kMyHome.kBidderName;
				kMyHome.kBidderName = std::wstring();

				__int64 const i64NextPayTexTime = g_kEventView.GetLocalSecTime() + MYHOME_PAY_TEX_TIME;
				BM::DBTIMESTAMP_EX kNextPayTime;
				CGameTime::SecTime2DBTimeEx(i64NextPayTexTime,kNextPayTime);
				kMyHome.kPayTexTime = static_cast<BM::PgPackedTime>(kNextPayTime); // 다음 세금 납부일자 변경

				__int64 const i64BiddingResultCost = kMyHome.i64HomePrice - (kMyHome.i64HomePrice * MATR_END)/100; // 세금을 뺀다. ㅋㅋ

				if(true == kOldOwnerGuid.IsNotNull())
				{// 이전 집주인에게 수수료뺀 판매 금액을 준다.
					SSendMailInfo kMailData;
					std::wstring kText;
					kMailData.FromGuid(kOldOwnerGuid);
					kMailData.TargetGuid(kOldOwnerGuid);
					GetDefString(MMC_AUCTION_SUCCESS_MAIL_FROM,kText);	kMailData.FromName(kText);
					GetDefString(MMC_AUCTION_SUCCESS_MAIL_TITLE,kText);	kMailData.MailTitle(kText);
					GetDefString(MMC_AUCTION_SUCCESS_MAIL_TEXT,kText);	kMailData.MailText(kText);
					kMailData.Money(i64BiddingResultCost);
					kMailData.PaymentType(0);

					tagDBItemStateChange kDBChange(DISCT_CREATE_MAIL_BYGUID, kOrder.Cause(), kOrder.OwnerGuid());
					kMailData.WriteToPacket(kDBChange.kAddonData);
					BM::GUID kMailGuid;// 로그 기록을 위해 메일 고유 아이디를 외부에서 제공한다.
					kMailGuid = BM::GUID::Create();
					kDBChange.kAddonData.Push(kMailGuid);
					kChangeArray.push_back(kDBChange);

					MYHOMEUTIL::MailLog(kOrder.OwnerGuid(),kMailGuid,kMailData,kContLogMgr);

					{
						SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
						pkActionOrder->kCause = CIE_Home_Auction_End;
						pkActionOrder->kContOrder.push_back(SPMO(IMET_MYHOME_SET_HOMEADDR, kOldOwnerGuid,SMOD_MyHome_MyHome_Set_Addr(0,0)));	// 유저 주소 정보 설정
						g_kJobDispatcher.VPush(pkActionOrder);
					}
				}

				{// 새 집주인에게 집 구매 성공에 대한 메시지를 전송한다.


					SSendMailInfo kMailData;
					std::wstring kText;
					kMailData.FromGuid(kMyHome.kOwnerGuid);
					kMailData.TargetGuid(kMyHome.kOwnerGuid);
					GetDefString(MMC_BIDDING_SUCCESS_MAIL_FROM,kText); kMailData.FromName(kText);
					GetDefString(MMC_BIDDING_SUCCESS_MAIL_TITLE,kText); kMailData.MailTitle(kText);
					GetDefString(MMC_BIDDING_SUCCESS_MAIL_TEXT,kText); kMailData.MailText(kText);
					kMailData.Money(0);
					kMailData.PaymentType(0);

					const CONT_MYHOME_DEFAULTITEM* pkCont = NULL;
					g_kTblDataMgr.GetContDef(pkCont);
					if(pkCont)
					{
						CONT_MYHOME_DEFAULTITEM::const_iterator iter = pkCont->find(kData.siStreetNo);
						if(iter != pkCont->end())
						{
							PgBase_Item kItem;
							if(S_OK == CreateSItem((*iter).second,1,GIOT_NONE,kItem))
							{
								kMailData.ItemGuid(kItem.Guid());
								kMailData.ItemNo(kItem.ItemNo());
								kMailData.ItemNum(kItem.Count());

								PgItemWrapper kItemWrapper(kItem,SItemPos(IT_POST,0));
								PgItemWrapper kBlankWrapper;
								kBlankWrapper.Pos(kItemWrapper.Pos());
								const DB_ITEM_STATE_CHANGE kItemChange(DISCT_CREATE, kOrder.Cause(),kMyHome.kOwnerGuid, kMyHome.kOwnerGuid, kBlankWrapper, kItemWrapper);//
								kChangeArray.push_back(kItemChange);
							}
						}
					}

					tagDBItemStateChange kDBChange(DISCT_CREATE_MAIL_BYGUID, kOrder.Cause(), kOrder.OwnerGuid());
					kMailData.WriteToPacket(kDBChange.kAddonData);
					BM::GUID kMailGuid;// 로그 기록을 위해 메일 고유 아이디를 외부에서 제공한다.
					kMailGuid = BM::GUID::Create();
					kDBChange.kAddonData.Push(kMailGuid);
					kChangeArray.push_back(kDBChange);

					MYHOMEUTIL::MailLog(kOrder.OwnerGuid(),kMailGuid,kMailData,kContLogMgr);
				}

				SMOD_MyHome_MyHome_Set_OwnerInfo kHomeOwnerInfo(kMyHome.siStreetNo,kMyHome.iHouseNo,kMyHome.kOwnerGuid,kMyHome.kName);

				PgDoc_MyHome * pkDocHome = dynamic_cast<PgDoc_MyHome *>(pkDocPlayer);
				if(pkDocHome)
				{
					pkDocHome->OwnerGuid(kMyHome.kOwnerGuid);
					pkDocHome->OwnerName(kMyHome.kName);
					tagDBItemStateChange kDBChange(DISCT_SET_HOME_OWNER_INFO,kOrder.Cause(), kMyHome.kHomeInSideGuid);
					kHomeOwnerInfo.WriteToPacket(kDBChange.kAddonData);
					kChangeArray.push_back(kDBChange);

					//친구 갱신 요청
					{
						BM::Stream kRefresh(PT_N_N_NFY_COMMUNITY_STATE_HOMEADDR_FRIEND);
						kRefresh.Push(kMyHome.kOwnerGuid);
						kRefresh.Push(pkDocHome->HomeAddr());
						::SendToFriendMgr(kRefresh);
					}
					{//길드 갱신 요청
						BM::Stream kRefresh(PT_N_N_NFY_COMMUNITY_STATE_HOMEADDR_GUILD);
						kRefresh.Push(kMyHome.kOwnerGuid);
						kRefresh.Push(pkDocHome->HomeAddr());
						::SendToGuildMgr(kRefresh);
					}
					{//커플 갱신 요청
						BM::Stream kRefresh(PT_N_N_NFY_COMMUNITY_STATE_HOMEADDR_COUPLE);
						kRefresh.Push(kMyHome.kOwnerGuid);
						kRefresh.Push(pkDocHome->HomeAddr());
						::SendToCoupleMgr(kRefresh);
					}
					{//파티 갱신 요청
						//데드락 때문에 위치 수정
						BM::Stream kRefresh(PT_N_T_NFY_COMMUNITY_STATE_HOMEADDR_PARTY);
						kRefresh.Push(kMyHome.kOwnerGuid);
						kRefresh.Push(pkDocHome->HomeAddr());
						SendToMyhomeMgr(kRefresh);

						/*SContentsUser rkUser;
						if(S_OK ==  GetPlayerByGuid(kMyHome.kOwnerGuid, false, rkUser) )
						{
							BM::Stream kRefresh(PT_N_T_NFY_COMMUNITY_STATE_HOMEADDR_PARTY);
							kRefresh.Push(rkUser.kCharGuid);
							kRefresh.Push(pkDocHome->HomeAddr());
							::SendToGlobalPartyMgr(rkUser.sChannel, kRefresh);
						}*/
					}
				}
				{
					tagDBItemStateChange kDBChange(DISCT_MYHOME_INVITATION_CLEAR,kOrder.Cause(), kMyHome.kHomeInSideGuid); // 발송한 초청장은 모두 삭제 한다.
					kDBChange.kAddonData.Push(kData.siStreetNo);
					kDBChange.kAddonData.Push(kData.iHouseNo);
					kChangeArray.push_back(kDBChange);
				}
			}
			else // 아무도 구매 하지 않았다.
			{
				if(true == kMyHome.kOwnerGuid.IsNull()) // 주인이 없는 집이면 바로 다시 경매에 등록한다.
				{
					kMyHome.bAuctionState = MAS_IS_BIDDING;

					__int64 const i64AuctionTime = g_kEventView.GetLocalSecTime() + DEFAULT_MYHOME_AUCTION_REG_TIME;
					BM::DBTIMESTAMP_EX kAuctionTime;
					CGameTime::SecTime2DBTimeEx(i64AuctionTime,kAuctionTime);

					kMyHome.kAuctionTime = static_cast<BM::PgPackedTime>(kAuctionTime); // 경매 종료 시간 다시 설정
				}
				else
				{// 경매 실패를 메일로 알려준다.
					SSendMailInfo kMailData;
					std::wstring kText;
					kMailData.FromGuid(kMyHome.kOwnerGuid);
					kMailData.TargetGuid(kMyHome.kOwnerGuid);
					GetDefString(MMC_AUCTION_FAIL_MAIL_FROM,kText);		kMailData.FromName(kText);
					GetDefString(MMC_AUCTION_FAIL_MAIL_TITLE,kText);	kMailData.MailTitle(kText);
					GetDefString(MMC_AUCTION_FAIL_MAIL_TEXT,kText);		kMailData.MailText(kText);
					kMailData.Money(0);
					kMailData.PaymentType(0);

					tagDBItemStateChange kDBChange(DISCT_CREATE_MAIL_BYGUID, kOrder.Cause(), kOrder.OwnerGuid());
					kMailData.WriteToPacket(kDBChange.kAddonData);
					BM::GUID kMailGuid;// 로그 기록을 위해 메일 고유 아이디를 외부에서 제공한다.
					kMailGuid = BM::GUID::Create();
					kDBChange.kAddonData.Push(kMailGuid);
					kChangeArray.push_back(kDBChange);

					MYHOMEUTIL::MailLog(kOrder.OwnerGuid(),kMailGuid,kMailData,kContLogMgr);
				}
			}

			hSubRet = S_OK;

			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
				kMyHome.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);
			}
			{
				pkDocPlayer->SetAbil(AT_MYHOME_STATE,kMyHome.bAuctionState);
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_STATE,kOrder.Cause(), kMyHome.kHomeInSideGuid);
				kDBChange.kAddonData.Push(kMyHome.bAuctionState);
				kChangeArray.push_back(kDBChange);
			}

			PgLog kLog(ELOrderMain_Home_Auction,ELOrderSub_Auction_End);
			kLog.Set(0,static_cast<int>(kData.siStreetNo));
			kLog.Set(1,static_cast<int>(kData.iHouseNo));
			kLog.Set(2,static_cast<int>(kMyHome.bAuctionState));

			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);
		}break;
	case IMET_MYHOME_BIDDING:
		{
			SMOD_MyHome_Bidding kData;
			kOrder.Read(kData);

			if(false == pkDocPlayer->HomeAddr().IsNull())
			{
				return E_MYHOME_ALREADY_HAVE_HOME;
			}

			if(true == IsBidding(kOrder.OwnerGuid()))
			{
				return E_MYHOME_ALREADY_BIDDINIG;
			}

			if(true == HasBiddingDelayTime(kOrder.OwnerGuid()))
			{
				return E_MYHOME_HAVE_BIDDING_DEALYTIME;
			}

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			if(MAS_IS_BIDDING != kMyHome.bAuctionState)
			{
				return E_MYHOME_NOT_REG_AUCTION;
			}

			if((0 < kMyHome.iLvLimitMin) && (kMyHome.iLvLimitMin <= kMyHome.iLvLimitMax))
			{
				if(pkDocPlayer->GetAbil(AT_LEVEL) < kMyHome.iLvLimitMin || kMyHome.iLvLimitMax < pkDocPlayer->GetAbil(AT_LEVEL))
				{
					return E_MYHOME_INVALID_LEVEL_LIMIT;
				}
			}

			__int64 const i64OldBiddingCost = kMyHome.i64LastBiddingCost;

			__int64 const iBiddingMin = (((kMyHome.i64LastBiddingCost + (kMyHome.i64LastBiddingCost*NEXT_BIDDING_COST_RATE)/100) + UPTOGOLD)/10000)*10000;

			if(kData.Cost() < iBiddingMin)
			{
				return E_MYHOME_NEED_MORE_BIDDING_COST;
			}

			if(true == kMyHome.kBidderGuid.IsNotNull())
			{
				SSendMailInfo kMailData;
				std::wstring kText;
				kMailData.FromGuid(kMyHome.kBidderGuid);
				kMailData.TargetGuid(kMyHome.kBidderGuid);
				GetDefString(MMT_BIDDING_RETURN_MAIL_FROM,kText);
				kMailData.FromName(kText);
				GetDefString(MMT_BIDDING_RETURN_MAIL_TITLE,kText);
				kMailData.MailTitle(kText);
				GetDefString(MMT_BIDDING_RETURN_MAIL_TEXT,kText);
				kMailData.MailText(kText);
				kMailData.Money(kMyHome.i64LastBiddingCost);	// 이전 입찰자의 입찰금을 메일로 돌려 보내 줘야 한다.
				kMailData.PaymentType(0);

				tagDBItemStateChange kDBChange(DISCT_CREATE_MAIL_BYGUID, kOrder.Cause(), kOrder.OwnerGuid());
				
				kMailData.WriteToPacket(kDBChange.kAddonData);

				BM::GUID kMailGuid;// 로그 기록을 위해 메일 고유 아이디를 외부에서 제공한다.
				kMailGuid = BM::GUID::Create();

				kDBChange.kAddonData.Push(kMailGuid);

				kChangeArray.push_back(kDBChange);

				MYHOMEUTIL::MailLog(kOrder.OwnerGuid(),kMailGuid,kMailData,kContLogMgr);

				RemoveBidder(kMyHome.kBidderGuid);

				BM::Stream kNotiPacket(PT_M_C_HOME_NOTI_CHANGE_TOP_BIDDER);
				kNotiPacket.Push(kData.StreetNo());
				kNotiPacket.Push(kData.HouseNo());
				SendToUser(kMyHome.kBidderGuid,kNotiPacket,false);
			}

			BM::GUID kOldBidderGuid = kMyHome.kBidderGuid;
			std::wstring kOldBidderName = kMyHome.kBidderName;

			kMyHome.kBidderGuid = kOrder.OwnerGuid();	
			kMyHome.kBidderName = pkDocPlayer->Name();
			kMyHome.i64LastBiddingCost = kData.Cost();
			AddBidder(kOrder.OwnerGuid());
			AddBiddingTime(kOrder.OwnerGuid());

			hSubRet = S_OK;

			tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
			kMyHome.WriteToPacket(kDBChange.kAddonData);
			kChangeArray.push_back(kDBChange);

			PgLog kLog(ELOrderMain_Home_Bidding,ELOrderSub_None);
			
			kLog.Set(0,kOldBidderName.c_str());
			kLog.Set(1,kMyHome.kBidderName.c_str());
			kLog.Set(2,kOldBidderGuid.str().c_str());
			kLog.Set(3,kMyHome.kBidderGuid.str().c_str());

			kLog.Set(0,static_cast<int>(kData.StreetNo()));
			kLog.Set(1,static_cast<int>(kData.HouseNo()));

			kLog.Set(0,i64OldBiddingCost);
			kLog.Set(1,kMyHome.i64LastBiddingCost);

			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);
		}break;
	case IMET_MYHOME_VISITFLAG:
		{
			SMOD_MyHome_VisitFlag kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			if(MAS_NOT_BIDDING != kMyHome.bAuctionState)
			{
				return E_MYHOME_CANT_MODIFY;
			}

			int const iOldEanbleVisitBit = kMyHome.bEnableVisitBit;
			kMyHome.bEnableVisitBit = kData.VisitFlag();

			hSubRet = S_OK;

			if(kOrder.OwnerGuid() == kMyHome.kHomeInSideGuid)
			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kOrder.OwnerGuid());
				kMyHome.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);
			}

			{
				pkDocPlayer->SetAbil(AT_MYHOME_VISITFLAG,kMyHome.bEnableVisitBit);
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_VISITFLAG,kOrder.Cause(), kOrder.OwnerGuid());
				kDBChange.kAddonData.Push(kMyHome.bEnableVisitBit);
				kChangeArray.push_back(kDBChange);
			}

			PgLog kLog(ELOrderMain_Home_VisitFlag,ELOrderSub_None);
			kLog.Set(0,static_cast<int>(kData.StreetNo()));
			kLog.Set(1,static_cast<int>(kData.HouseNo()));
			kLog.Set(2,iOldEanbleVisitBit);
			kLog.Set(3,static_cast<int>(kMyHome.bEnableVisitBit));

			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);

		}break;
	case IMET_MYHOME_AUCTION_REG:
		{
			SMOD_MyHome_Auction_Reg kData;
			kOrder.Read(kData);

			SMYHOME kMyHome;
			if(false == GetHome(kData.StreetNo(),kData.HouseNo(),kMyHome))
			{
				return E_MYHOME_NOT_FOUND;
			}

			switch(kMyHome.bAuctionState)
			{
			case MAS_IS_BIDDING:
				{
					return E_MYHOME_ALREADY_REG_AUCTION;
				}break;
			case MAS_IS_BLOCK:
				{
					return E_MYHOME_IS_BLOCKED;
				}break;
			}

			__int64 const i64CurTime = g_kEventView.GetLocalSecTime() + (kData.Hour() * 3600); // 들어온 시간을 다시 초로 환산 하자 미리 초로 받을걸 그랬나 ㅡㅡ;;

			BM::DBTIMESTAMP_EX kAuctionEndTime;
			CGameTime::SecTime2DBTimeEx(i64CurTime,kAuctionEndTime);

			kMyHome.kAuctionTime = static_cast<BM::PgPackedTime>(kAuctionEndTime);
			kMyHome.i64FirstBiddingCost = kData.Cost();
			kMyHome.i64LastBiddingCost = kData.Cost();
			kMyHome.kBidderGuid.Clear(); // 혹시 모를 예전 입찰자 정보는 한번더 비워 주자
			kMyHome.bAuctionState = MAS_IS_BIDDING;

			hSubRet = S_OK;

			{
				tagDBItemStateChange kDBChange(DISCT_MYHOME_MODIFY,kOrder.Cause(), kMyHome.kHomeInSideGuid);
				kMyHome.WriteToPacket(kDBChange.kAddonData);
				kChangeArray.push_back(kDBChange);
			}
			{
				pkDocPlayer->SetAbil(AT_MYHOME_STATE,kMyHome.bAuctionState);
				tagDBItemStateChange kDBChange(DISCT_SET_HOME_STATE,kOrder.Cause(), kMyHome.kHomeInSideGuid);
				kDBChange.kAddonData.Push(kMyHome.bAuctionState);
				kChangeArray.push_back(kDBChange);
			}

			PgLog kLog(ELOrderMain_Home_Auction,ELOrderSub_None);
			kLog.Set(0,static_cast<int>(kData.StreetNo()));
			kLog.Set(1,static_cast<int>(kData.HouseNo()));
			kLog.Set(2,static_cast<int>(kData.Hour()));
			kLog.Set(3,static_cast<int>(kMyHome.bAuctionState));

			kLog.Set(0,kMyHome.i64LastBiddingCost);

			kContLogMgr.AddLog(kOrder.OwnerGuid(),kLog);

		}break;
	}

	return hSubRet;
}

void PgMyHomeMgrImpl::AddMyHome(short const siStreetNo, CONT_MYHOME const & kCont)
{
	m_kCont[siStreetNo].insert(kCont.begin(),kCont.end());
}

bool PgMyHomeMgrImpl::RemoveMyHome(short const siStreetNo, int const iHouseNo)
{
	CONT_MYHOMEBUILDINGS::iterator iter = m_kCont.find(siStreetNo);
	if(iter == m_kCont.end())
	{
		return false;
	}

	CONT_MYHOMEBUILDINGS::mapped_type::iterator house_iter = (*iter).second.find(SHOMEADDR(siStreetNo,iHouseNo));
	if(house_iter == (*iter).second.end())
	{
		return false;
	}

	(*iter).second.erase(house_iter);
	return true;
}

void PgMyHomeMgrImpl::SendToUser(BM::GUID const &kGuidKey, BM::Stream const &rkPacket, bool const IsMemberGuid)
{
	if (kGuidKey == BM::GUID::NullData())
	{
		return;
	}

	SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
	pkActionOrder->InsertTarget(kGuidKey);
	pkActionOrder->kCause = CNE_CONTENTS_SENDTOUSER;
	
	ContentsActionEvent_SendPacket kEvent(ECEvent_SendToUser);
	kEvent.MemberGuid(IsMemberGuid);
	kEvent.Guid(kGuidKey);
	kEvent.SendType(ContentsActionEvent_SendPacket::E_SendUser_ToOneUser);
	kEvent.m_kPacket.Push(rkPacket);

	SPMO kOrder(IMET_CONTENTS_EVENT, kGuidKey, kEvent);
	pkActionOrder->kContOrder.push_back(kOrder);
	g_kJobDispatcher.VPush(pkActionOrder);
}

HRESULT PgMyHomeMgrImpl::Proc_PT_M_I_REQ_MYHOME_AUCTION_REG(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	__int64 i64Cost = 0;
	__int64 i64Tex = 0;
	int iHour = 0;
	SHOMEADDR kAddr;
	pkPacket->Pop(kAddr);
	pkPacket->Pop(i64Cost);
	pkPacket->Pop(i64Tex);
	pkPacket->Pop(iHour);

	SMYHOME kMyHome;
	if(false == GetHome(kAddr.StreetNo(),kAddr.HouseNo(),kMyHome))
	{
		return E_MYHOME_NOT_FOUND;
	}

	if(kMyHome.kOwnerGuid != kOwnerGuid)
	{
		return E_MYHOME_NOT_HAVE_AUTHORITY;
	}

	if(false == kMyHome.kContSideJob.empty())
	{
		return E_MYHOME_HAVE_SIDE_JOB;
	}

	CONT_PLAYER_MODIFY_ORDER kOrder;
	kOrder.push_back(SPMO(IMET_ADD_MONEY, kOwnerGuid,SPMOD_Add_Money(-i64Tex)));
	kOrder.push_back(SPMO(IMET_MYHOME_AUCTION_REG, kMyHome.kHomeInSideGuid,SMOD_MyHome_Auction_Reg(kAddr.StreetNo(),kAddr.HouseNo(),i64Cost,iHour)));

	BM::Stream kPacket(PT_M_C_ANS_MYHOME_AUCTION_REG);
	kPacket.Push(i64Cost);
	kPacket.Push(iHour);

	SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
	pkActionOrder->InsertTarget(kOwnerGuid);
	pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
	pkActionOrder->kCause = CIE_Home_Auction_Reg;
	pkActionOrder->kContOrder = kOrder;
	pkActionOrder->kAddonPacket.Push(kPacket.Data());
	g_kJobDispatcher.VPush(pkActionOrder);
	return S_OK;
}

HRESULT PgMyHomeMgrImpl::Proc_PT_M_I_REQ_START_SIDE_JOB(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	SHOMEADDR kAddr;
	pkPacket->Pop(kAddr);
	eMyHomeSideJob kSideJob;
	pkPacket->Pop(kSideJob);
	int iSellTime = 0;
	pkPacket->Pop(iSellTime);

	SMYHOME kMyHome;
	if(false == GetHome(kAddr.StreetNo(),kAddr.HouseNo(),kMyHome))
	{
		return E_MYHOME_NOT_FOUND;
	}

	if(kMyHome.kOwnerGuid != kOwnerGuid)
	{
		return E_MYHOME_NOT_HAVE_AUTHORITY;
	}

	CONT_MYHOME_SIDE_JOB::const_iterator iter = kMyHome.kContSideJob.find(kSideJob);
	if(iter != kMyHome.kContSideJob.end())
	{
		return E_MYHOME_ALREADY_STARTED_JOB;
	}

	__int64 const i64EndTime = g_kEventView.GetLocalSecTime() + iSellTime;

	BM::DBTIMESTAMP_EX kEndTime;
	CGameTime::SecTime2DBTimeEx(i64EndTime, kEndTime);

	// 기본으로 실행 가능한 아르바이트는 무조건 돈만 할인 되도록 한다.

	CONT_DEFSIDEJOBRATE const *pkContDef = NULL;
	g_kTblDataMgr.GetContDef(pkContDef);

	if(!pkContDef)
	{
		return E_MYHOME_NOT_FOUND_JOB;
	}

	CONT_DEFSIDEJOBRATE::const_iterator rate_iter = pkContDef->find(kSideJob);
	if(rate_iter == pkContDef->end())
	{
		return E_MYHOME_NOT_FOUND_JOB;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkItem = kItemDefMgr.GetDef(kMyHome.iHomeStyle);
	if(NULL == pkItem)
	{
		return E_MYHOME_NOT_FOUND_JOB;
	}

	int const iHomeLevel = pkItem->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_3);

	CONT_DEFMYHOMESIDEJOBTIME const * pkCont = NULL;
	g_kTblDataMgr.GetContDef(pkCont);
	if(!pkCont)
	{
		return E_MYHOME_NOT_FOUND_JOB;
	}

	CONT_DEFMYHOMESIDEJOBTIME::const_iterator time_iter = pkCont->find(iHomeLevel);
	if(time_iter == pkCont->end())
	{
		return E_MYHOME_NOT_FOUND_JOB;
	}

	int const iLimitTime = (*time_iter).second;

	if(iLimitTime < iSellTime)
	{
		return E_MYHOME_INVALID_TIME_LIMIT;
	}

	int const iRateValue = (*rate_iter).second.iCostRate;

	CONT_PLAYER_MODIFY_ORDER kOrder;
	kOrder.push_back(SPMO(IMET_SIDEJOB_INSERT, kMyHome.kHomeInSideGuid,SMOD_MyHome_SideJob_Insert(kAddr.StreetNo(), kAddr.HouseNo(), kSideJob, MSJRT_GOLD, iRateValue, static_cast<BM::PgPackedTime>(kEndTime))));

	BM::Stream kPacket(PT_M_C_ANS_START_SIDE_JOB);
	kPacket.Push(kSideJob);

	SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
	pkActionOrder->InsertTarget(kOwnerGuid);
	pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
	pkActionOrder->kCause = CIE_Home_SideJob_Insert;
	pkActionOrder->kContOrder = kOrder;
	pkActionOrder->kAddonPacket.Push(kPacket.Data());
	g_kJobDispatcher.VPush(pkActionOrder);
	return S_OK;
}

HRESULT PgMyHomeMgrImpl::Proc_PT_M_I_REQ_CANCEL_SIDE_JOB(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	SHOMEADDR kAddr;
	pkPacket->Pop(kAddr);
	eMyHomeSideJob kSideJob;
	pkPacket->Pop(kSideJob);

	SMYHOME kMyHome;
	if(false == GetHome(kAddr.StreetNo(),kAddr.HouseNo(),kMyHome))
	{
		return E_MYHOME_NOT_FOUND;
	}

	if(kMyHome.kOwnerGuid != kOwnerGuid)
	{
		return E_MYHOME_NOT_HAVE_AUTHORITY;
	}

	CONT_MYHOME_SIDE_JOB::const_iterator iter = kMyHome.kContSideJob.find(kSideJob);
	if(iter == kMyHome.kContSideJob.end())
	{
		return E_MYHOME_NOT_FOUND_JOB;
	}

	CONT_PLAYER_MODIFY_ORDER kOrder;
	kOrder.push_back(SPMO(IMET_SIDEJOB_COMPLETE, kMyHome.kHomeInSideGuid, SMOD_MyHome_SideJob_Complete(kMyHome.kOwnerGuid, (*iter).first, kAddr.StreetNo(), kAddr.HouseNo())));
	kOrder.push_back(SPMO(IMET_SIDEJOB_REMOVE, kMyHome.kHomeInSideGuid,SMOD_MyHome_SideJob_Remove(kAddr.StreetNo(), kAddr.HouseNo(), kSideJob)));

	BM::Stream kPacket(PT_M_C_ANS_CANCEL_SIDE_JOB);
	kPacket.Push(kSideJob);

	SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
	pkActionOrder->InsertTarget(kOwnerGuid);
	pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
	pkActionOrder->kCause = CIE_Home_SideJob_Remove;
	pkActionOrder->kContOrder = kOrder;
	pkActionOrder->kAddonPacket.Push(kPacket.Data());
	g_kJobDispatcher.VPush(pkActionOrder);
	return S_OK;
}

void PgMyHomeMgrImpl::RecvPacketHandler(BM::Stream::DEF_STREAM_TYPE const kPacketType,SERVER_IDENTITY const & kSI,SGroundKey const & kGndKey, BM::Stream * const pkPacket)
{
	switch(kPacketType)
	{
	case PT_M_I_REQ_MYHOME_SELL:
		{
			BM::GUID kOwnerGuid;
			SHOMEADDR kAddr;
			pkPacket->Pop(kOwnerGuid);
			pkPacket->Pop(kAddr);

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_SELL);

			SMYHOME kMyHome;
			if(true == GetHome(kAddr.StreetNo(),kAddr.HouseNo(),kMyHome))
			{
				if(kOwnerGuid != kMyHome.kOwnerGuid)
				{
					kPacket.Push(E_MYHOME_NOT_HAVE_AUTHORITY);
					SendToUser(kOwnerGuid,kPacket,false);
					return;
				}

				__int64 const i64Tex = MYHOMEUTIL::CalcTex(kMyHome);
				if(i64Tex > 0)
				{
					kPacket.Push(E_MYHOME_NEED_PAY_TEX);
					SendToUser(kOwnerGuid,kPacket,false);
					return;
				}

				SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
				pkActionOrder->InsertTarget(kOwnerGuid);
				pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
				pkActionOrder->kCause = CIE_Home_Modify;

				for(CONT_MYHOME_SIDE_JOB::const_iterator iter = kMyHome.kContSideJob.begin();iter != kMyHome.kContSideJob.end();++iter)
				{
					pkActionOrder->kContOrder.push_back(SPMO(IMET_SIDEJOB_REMOVE, kMyHome.kHomeInSideGuid, SMOD_MyHome_SideJob_Remove(kMyHome.siStreetNo, kMyHome.iHouseNo, (*iter).first)));
					pkActionOrder->kContOrder.push_back(SPMO(IMET_SIDEJOB_COMPLETE, kMyHome.kHomeInSideGuid, SMOD_MyHome_SideJob_Complete(kMyHome.kOwnerGuid, (*iter).first, kMyHome.siStreetNo, kMyHome.iHouseNo)));
				}

				pkActionOrder->kContOrder.push_back(SPMO(IMET_MYHOME_RETURN_ITEM, kMyHome.kHomeInSideGuid,SMOD_MyHome_ReturnItem(kMyHome.kOwnerGuid,kMyHome.siStreetNo,kMyHome.iHouseNo, RIT_SELL)));	// 내부 아이템 돌려 보내
				pkActionOrder->kContOrder.push_back(SPMO(IMET_MYHOME_SET_HOMEADDR, kOwnerGuid,SMOD_MyHome_MyHome_Set_Addr(0,0)));	// 유저 주소 정보 설정
				pkActionOrder->kContOrder.push_back(SPMO(IMET_MYHOME_REMOVE, kOwnerGuid,SMOD_MyHome_MyHome_Set_Addr(kAddr.StreetNo(),kAddr.HouseNo())));	// 홈삭제
				pkActionOrder->kAddonPacket.Push(kPacket.Data());
				g_kJobDispatcher.VPush(pkActionOrder);
			}
			else
			{
				kPacket.Push(E_MYHOME_NOT_FOUND);
				SendToUser(kOwnerGuid,kPacket,false);
			}
		}break;
	case PT_M_I_REQ_EXCHANGE_LOGCOUNTTOEXP:
		{
			BM::GUID kOwnerGuid;
			SHOMEADDR kAddr;
			pkPacket->Pop(kOwnerGuid);
			pkPacket->Pop(kAddr);

			BM::Stream kPacket(PT_M_C_ANS_EXCHANGE_LOGCOUNTTOEXP);

			SMYHOME kMyHome;
			if(true == GetHome(kAddr.StreetNo(),kAddr.HouseNo(),kMyHome))
			{
				__int64 const i64Exp = int(std::pow(kMyHome.iTotalVisitLogCount,1.02)) * 100;

				kPacket.Push(i64Exp);

				SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
				pkActionOrder->InsertTarget(kOwnerGuid);
				pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
				pkActionOrder->kCause = CIE_Home_Modify;
				pkActionOrder->kContOrder.push_back(SPMO(IMET_ADD_ABIL64, kOwnerGuid, SPMOD_AddAbil(AT_EXPERIENCE, i64Exp)));
				pkActionOrder->kContOrder.push_back(SPMO(IMET_SET_ABIL, kMyHome.kHomeInSideGuid, SPMOD_AddAbil(AT_MYHOME_VISITLOGCOUNT, 0)));
				pkActionOrder->kContOrder.push_back(SPMO(IMET_MYHOME_SET_VISITLOGCOUNT, kMyHome.kHomeInSideGuid, SMOD_MyHome_Sync_VisitLogCount(kAddr.StreetNo(),kAddr.HouseNo(), 0, true)));
				pkActionOrder->kAddonPacket.Push(kPacket.Data());
				g_kJobDispatcher.VPush(pkActionOrder);
			}
			else
			{
				kPacket.Push(E_MYHOME_NOT_FOUND);
				SendToUser(kOwnerGuid,kPacket,false);
			}
		}break;
	case PT_M_I_REQ_START_SIDE_JOB:
		{
			BM::GUID kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT kErr = Proc_PT_M_I_REQ_START_SIDE_JOB(kOwnerGuid,pkPacket);
			if(S_OK == kErr)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_START_SIDE_JOB);
			kPacket.Push(kErr);
			SendToUser(kOwnerGuid,kPacket,false);
		}break;
	case PT_M_I_REQ_CANCEL_SIDE_JOB:
		{
			BM::GUID kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT kErr = Proc_PT_M_I_REQ_CANCEL_SIDE_JOB(kOwnerGuid,pkPacket);
			if(S_OK == kErr)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_CANCEL_SIDE_JOB);
			kPacket.Push(kErr);
			SendToUser(kOwnerGuid,kPacket,false);
		}break;
	case PT_M_I_REQ_MYHOME_AUCTION_REG:
		{
			BM::GUID kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT kErr = Proc_PT_M_I_REQ_MYHOME_AUCTION_REG(kOwnerGuid,pkPacket);
			if(S_OK == kErr)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_AUCTION_REG);
			kPacket.Push(kErr);
			SendToUser(kOwnerGuid,kPacket,false);
		}break;
	case PT_M_I_REQ_MYHOME_VISITORS:
		{
			BM::GUID	kOwnerGuid;
			SHOMEADDR	kAddr;
			pkPacket->Pop(kOwnerGuid);
			pkPacket->Pop(kAddr);

			CEL::DB_QUERY kQuery( DT_PLAYER, DQT_VISITORS_SELECT,L"EXEC [dbo].[up_User_MyHome_Visitors_Select]");
			kQuery.InsertQueryTarget(kOwnerGuid);
			kQuery.QueryOwner(kOwnerGuid);
			kQuery.PushStrParam(kOwnerGuid);
			kQuery.PushStrParam(kAddr.StreetNo());
			kQuery.PushStrParam(kAddr.HouseNo());
			g_kCoreCenter.PushQuery(kQuery);
		}break;
	case PT_M_I_REQ_MYHOME_INVITATION_CARD:
		{
			BM::GUID	kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			CEL::DB_QUERY kQuery( DT_PLAYER, DQT_INVITATION_SELECT,L"EXEC [dbo].[up_User_MyHome_Invitation_Select]");
			kQuery.InsertQueryTarget(kOwnerGuid);
			kQuery.QueryOwner(kOwnerGuid);
			kQuery.PushStrParam(kOwnerGuid);
			g_kCoreCenter.PushQuery(kQuery);
		}break;
	case PT_M_I_REQ_MYHOME_POST_INVITATION_CARD:
		{
			BM::GUID	kOwnerGuid;
			std::wstring kVisitorName;
			short		siStreetNo = 0;
			int			iHouseNo = 0;

			pkPacket->Pop(kOwnerGuid);
			pkPacket->Pop(siStreetNo);
			pkPacket->Pop(iHouseNo);
			pkPacket->Pop(kVisitorName);

			SMYHOME kMyHome;
			if(true == GetHome(siStreetNo,iHouseNo,kMyHome))
			{
				CEL::DB_QUERY kQuery( DT_PLAYER, DQT_INVITATION_CREATE,L"EXEC [dbo].[up_User_MyHome_Invitation_Create]");
				kQuery.InsertQueryTarget(kOwnerGuid);
				kQuery.QueryOwner(kOwnerGuid);
				kQuery.PushStrParam(kOwnerGuid);
				kQuery.PushStrParam(kVisitorName);
				kQuery.PushStrParam(siStreetNo);
				kQuery.PushStrParam(iHouseNo);

				int iInvitationCardNum = 0;

				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const * pkItem = kItemDefMgr.GetDef(kMyHome.iHomeStyle);
				if(pkItem)
				{
					iInvitationCardNum = pkItem->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_4);
				}

				kQuery.PushStrParam(iInvitationCardNum);

				kQuery.contUserData.Push(kVisitorName);

				g_kCoreCenter.PushQuery(kQuery);
			}
		}break;
	case PT_M_I_REQ_HOME_VISITLOG_ADD:
		{
			BM::GUID	kOwnerGuid,
						kTargetGuid,
						kMemberGuid;

			pkPacket->Pop(kOwnerGuid);
			pkPacket->Pop(kTargetGuid);
			pkPacket->Pop(kMemberGuid);

			std::wstring kVisitLog;
			pkPacket->Pop(kVisitLog);

			bool bPrivate = false;
			pkPacket->Pop(bPrivate);

			CEL::DB_QUERY kQuery( DT_PLAYER, DQT_VISITLOG_ADD,L"EXEC [dbo].[up_User_MyHome_VisitLog_Add]");
			kQuery.InsertQueryTarget(kOwnerGuid);
			kQuery.InsertQueryTarget(kMemberGuid);
			kQuery.QueryOwner(kOwnerGuid);
			kQuery.PushStrParam(kTargetGuid);
			kQuery.PushStrParam(kOwnerGuid);
			kQuery.PushStrParam(kMemberGuid);
			kQuery.PushStrParam(kVisitLog);
			kQuery.PushStrParam(bPrivate);
			kQuery.contUserData.Push(*pkPacket);
			g_kCoreCenter.PushQuery(kQuery);
		}break;
	case PT_M_I_REQ_HOME_VISITLOG_LIST:
		{
			BM::GUID	kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);
			BM::GUID	kHomeOwnerGuid;
			pkPacket->Pop(kHomeOwnerGuid);

			CEL::DB_QUERY kQuery( DT_PLAYER, DQT_VISITLOG_LIST,L"EXEC [dbo].[up_User_MyHome_VisitLog_List]");
			kQuery.InsertQueryTarget(kOwnerGuid);
			kQuery.QueryOwner(kOwnerGuid);
			kQuery.PushStrParam(kHomeOwnerGuid);
			kQuery.contUserData.Push(kHomeOwnerGuid);
			g_kCoreCenter.PushQuery(kQuery);
		}break;
	case PT_M_I_REQ_HOME_VISITLOG_DELETE:
		{
			BM::GUID	kOwnerGuid,
						kLogGuid;
			pkPacket->Pop(kOwnerGuid);
			pkPacket->Pop(kLogGuid);

			CEL::DB_QUERY kQuery( DT_PLAYER, DQT_VISITLOG_DELETE,L"EXEC [dbo].[up_User_MyHome_VisitLog_Delete]");
			kQuery.InsertQueryTarget(kOwnerGuid);
			kQuery.QueryOwner(kOwnerGuid);
			kQuery.PushStrParam(kLogGuid);
			kQuery.PushStrParam(kOwnerGuid);
			kQuery.contUserData.Push(kLogGuid);
			g_kCoreCenter.PushQuery(kQuery);
		}break;
	case PT_M_I_REQ_MYHOME_AUCTION_UNREG:
		{
			BM::GUID kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT kErr = Proc_PT_M_I_REQ_MYHOME_AUCTION_UNREG(kOwnerGuid,pkPacket);
			if(S_OK == kErr)
			{
				return;
			}
			BM::Stream kPacket(PT_M_C_ANS_MYHOME_AUCTION_UNREG);
			kPacket.Push(kErr);
			SendToUser(kOwnerGuid,kPacket,false);
		}break;
	case PT_M_I_REQ_MYHOME_PAY_TEX:
		{
			BM::GUID kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT kErr = Proc_PT_M_I_REQ_MYHOME_PAY_TEX(kOwnerGuid,pkPacket);
			if(S_OK == kErr)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_PAY_TEX);
			kPacket.Push(kErr);
			SendToUser(kOwnerGuid,kPacket,false);
		}break;
	case PT_M_I_REQ_MYHOME_INFO:
		{
			BM::GUID kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);
			SHOMEADDR kAddr;
			pkPacket->Pop(kAddr);

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_INFO);

			SMYHOME kMyHome;
			if(true == GetHome(kAddr.StreetNo(),kAddr.HouseNo(),kMyHome))
			{
				kPacket.Push(S_OK);
				kPacket.Push(*pkPacket);
				kMyHome.WriteToPacket(kPacket);
			}
			else
			{
				kPacket.Push(E_MYHOME_NOT_FOUND);
				kPacket.Push(*pkPacket);
			}

			SendToUser(kOwnerGuid,kPacket,false);
		}break;
	case PT_M_I_REQ_ENTER_SIDE_JOB:
		{
			BM::GUID kOwnerGuid;
			short	siStreetNo = 0;
			int		iHouseNo = 0;

			pkPacket->Pop(kOwnerGuid);
			pkPacket->Pop(siStreetNo);
			pkPacket->Pop(iHouseNo);

			CEL::DB_QUERY kQuery( DT_PLAYER, DQT_CHECK_ENABLE_VISIT_OPTION, L"EXEC [dbo].[up_User_MyHome_Check_EanbleVisitOption]" );
			kQuery.InsertQueryTarget( kOwnerGuid );
			kQuery.QueryOwner( kOwnerGuid );
			kQuery.PushStrParam( siStreetNo );
			kQuery.PushStrParam( iHouseNo );
			kQuery.PushStrParam( kOwnerGuid );

			kQuery.contUserData.Push(siStreetNo);
			kQuery.contUserData.Push(iHouseNo);

			g_kCoreCenter.PushQuery( kQuery );
		}break;
	case PT_M_I_REQ_MYHOME_ENTER:
		{
			BM::GUID kOwnerGuid;
			short	siStreetNo = 0;
			int		iHouseNo = 0;

			pkPacket->Pop(kOwnerGuid);
			pkPacket->Pop(siStreetNo);
			pkPacket->Pop(iHouseNo);

			CEL::DB_QUERY kQuery( DT_PLAYER, DQT_CHECK_ENABLE_VISIT_OPTION, L"EXEC [dbo].[up_User_MyHome_Check_EanbleVisitOption]" );
			kQuery.InsertQueryTarget( kOwnerGuid );
			kQuery.QueryOwner( kOwnerGuid );
			kQuery.PushStrParam( siStreetNo );
			kQuery.PushStrParam( iHouseNo );
			kQuery.PushStrParam( kOwnerGuid );

			kQuery.contUserData.Push(siStreetNo);
			kQuery.contUserData.Push(iHouseNo);

			g_kCoreCenter.PushQuery( kQuery );
		}break;
	case PT_M_I_REQ_HOMETOWN_INFO:
		{
			BM::GUID kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);
			BM::Stream kPacket(PT_M_C_ANS_HOMETOWN_INFO);

			CONT_DEFMYHOMEBUILDINGS const * pkContDef = NULL;
			g_kTblDataMgr.GetContDef(pkContDef);
			if(pkContDef)
			{
				size_t kHomeCount = 0;
				BM::Stream kHomeInfo;
				for(CONT_MYHOMEBUILDINGS::const_iterator iter = m_kCont.begin();iter != m_kCont.end();++iter)
				{
					CONT_DEFMYHOMEBUILDINGS::const_iterator b_iter = pkContDef->find((*iter).first);
					if(b_iter != pkContDef->end() && (0 < (*b_iter).second.iGrade))
					{
						kHomeCount += (*iter).second.size();
						for(CONT_MYHOME::const_iterator h_iter = (*iter).second.begin();h_iter != (*iter).second.end();++h_iter)
						{
							kHomeInfo.Push((*h_iter).first);
							(*h_iter).second.WriteToPacket(kHomeInfo);
						}
					}
				}

				kPacket.Push(kHomeCount);
				kPacket.Push(kHomeInfo);
			}

			kPacket.Push(*pkPacket);
			SendToUser(kOwnerGuid,kPacket,false);
		}break;
	}
}

bool PgMyHomeMgrImpl::FindGuestFromChatRoom(BM::GUID const & kPlayerGuid, BM::GUID & kChatRoomGuid)
{
	CONT_MYHOME_CHAT_PLAYER::const_iterator iter = ContChatPlayer().find(kPlayerGuid);
	if(iter != ContChatPlayer().end())
	{
		kChatRoomGuid = (*iter).second;
		return true;
	}
	return false;
}

void PgMyHomeMgrImpl::SendToRoom(BM::GUID const & kRoomGuid, BM::Stream const & kPacket, BM::GUID const & kIgnoreGuid)
{
	CONT_MYHOMECHATROOM::iterator iter = m_kContChatRoom.find(kRoomGuid);
	if(iter == m_kContChatRoom.end())
	{
		return;
	}

	CONT_MYHOMECHATGUEST const & kGuestList = (*iter).second.kContGuest;
	for(CONT_MYHOMECHATGUEST::const_iterator guest_iter = kGuestList.begin();guest_iter != kGuestList.end();++guest_iter)
	{
		if(kIgnoreGuid == (*guest_iter).first)
		{
			continue;
		}
		SendToUser((*guest_iter).first, kPacket, false);
	}
}

HRESULT PgMyHomeMgrImpl::Recv_PT_C_M_REQ_MYHOME_CHAT_ENTER(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	BM::GUID	kRoomGuid;
	pkPacket->Pop(kRoomGuid);

	std::wstring kPassword;
	pkPacket->Pop(kPassword);

	SHomeChatGuest kGuest;
	kGuest.ReadFromPacket(*pkPacket);

	BM::GUID kFindRoomGuid;

	if(true == FindGuestFromChatRoom(kOwnerGuid,kFindRoomGuid))
	{
		return E_MYHOME_ALREADY_CHAT_ROOM;
	}

	if(BM::GUID::NullData() == kRoomGuid)
	{
		kRoomGuid = GlobalChatRoomGuid(); // 공용 체널도 하나의 방이다. 그냥 넣어 주자
	}

	CONT_MYHOMECHATROOM::iterator iter = m_kContChatRoom.find(kRoomGuid);
	if(iter == m_kContChatRoom.end())
	{
		return E_MYHOME_NOT_FOUND_ROOM;
	}

	CONT_MYHOMECHATROOM::mapped_type & kRoom = (*iter).second;

	if(kRoom.kContKick.find(kOwnerGuid) != kRoom.kContKick.end())
	{
		return E_MYHOME_IS_KICKED_ROOM;
	}

	if(0 < kRoom.kMaxUserCount && (kRoom.kMaxUserCount <= kRoom.kContGuest.size()))
	{
		return E_MYHOME_THIS_ROOM_FULL;
	}

	if(kRoom.kRoomPassword != kPassword)
	{
		return E_MYHOME_NOT_MATCH_PASSWORD;
	}

	if(false == kRoom.kContGuest.insert(std::make_pair(kOwnerGuid, kGuest)).second)
	{
		return E_MYHOME_ALREADY_CHAT_ROOM;
	}

	m_kContChatPlayer.insert(std::make_pair(kOwnerGuid,kRoomGuid));

	BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_ENTER);
	kPacket.Push(S_OK);
	kRoom.WriteToPacket(kPacket);
	SendToUser(kOwnerGuid, kPacket, false);

	BM::Stream kNoti(PT_M_C_NOTI_MYHOME_CHAT_ROOM_ENTER);
	kGuest.WriteToPacket(kNoti);
	SendToRoom(kRoomGuid, kNoti, kOwnerGuid);

	return S_OK;
}

HRESULT PgMyHomeMgrImpl::Recv_PT_C_M_REQ_MYHOME_CHAT_EXIT(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	BM::GUID kFindRoomGuid;
	if(false == FindGuestFromChatRoom(kOwnerGuid,kFindRoomGuid))
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	CONT_MYHOMECHATROOM::iterator iter = m_kContChatRoom.find(kFindRoomGuid);
	if(iter == m_kContChatRoom.end())
	{
		return E_MYHOME_NOT_FOUND_ROOM;
	}

	CONT_MYHOMECHATROOM::mapped_type & kRoom = (*iter).second;

	CONT_MYHOMECHATGUEST::iterator guest_iter = kRoom.kContGuest.find(kOwnerGuid);
	if(guest_iter == kRoom.kContGuest.end())
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	bool bDeleteMaster = (*guest_iter).second.bRoomMaster;

	kRoom.kContGuest.erase(guest_iter);

	if(false == kRoom.kContGuest.empty())
	{
		if(true == bDeleteMaster)
		{
			CONT_MYHOMECHATGUEST::mapped_type & kTarget = (*kRoom.kContGuest.begin()).second;
			kTarget.bRoomMaster = true;
			BM::Stream kNoti(PT_M_C_NOTI_MYHOME_CHAT_GUEST_MODIFY);
			kTarget.WriteToPacket(kNoti);
			SendToRoom(kFindRoomGuid, kNoti);
		}
	}
	else if((*iter).first != GlobalChatRoomGuid())
	{
		m_kContChatRoom.erase(iter);
	}

	m_kContChatPlayer.erase(kOwnerGuid);

	BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_EXIT);
	kPacket.Push(S_OK);
	SendToUser(kOwnerGuid, kPacket, false);

	BM::Stream kNoti(PT_M_C_NOTI_MYHOME_CHAT_ROOM_EXIT);
	kNoti.Push(kOwnerGuid);
	SendToRoom(kFindRoomGuid, kNoti, kOwnerGuid);

	return S_OK;
}

size_t const MAX_ROOM_COUNT = 50;
size_t const MIN_GUEST_COUNT = 2;
size_t const MAX_GUEST_COUNT = 10;

HRESULT PgMyHomeMgrImpl::Recv_PT_C_M_REQ_MYHOME_CHAT_ROOM_CREATE(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	SHomeChatGuest kGuest;
	kGuest.ReadFromPacket(*pkPacket);

	CONT_MYHOMECHATROOM::mapped_type kRoom;
	kRoom.ReadFromPacket(*pkPacket);

	BM::GUID kFindRoomGuid;
	if(true == FindGuestFromChatRoom(kOwnerGuid,kFindRoomGuid))
	{
		return E_MYHOME_ALREADY_CHAT_ROOM;
	}

	if(MAX_ROOM_COUNT <= m_kContChatRoom.size())
	{
		return E_MYHOME_ALL_CHAT_ROOM_OPENED;
	}

	if((MIN_GUEST_COUNT > kRoom.kMaxUserCount) || (MAX_GUEST_COUNT < kRoom.kMaxUserCount))
	{
		return E_MYHOME_INVALID_GUEST_COUNT;
	}
	
	kRoom.kContGuest.insert(std::make_pair(kOwnerGuid, kGuest));
	m_kContChatRoom.insert(std::make_pair(kRoom.kRoomGuid, kRoom));
	m_kContChatPlayer.insert(std::make_pair(kOwnerGuid, kRoom.kRoomGuid));

	BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_ROOM_CREATE);
	kPacket.Push(S_OK);
	kRoom.WriteToPacket(kPacket);
	SendToUser(kOwnerGuid, kPacket, false);
	return S_OK;
}

HRESULT PgMyHomeMgrImpl::Recv_PT_C_M_REQ_MYHOME_CHAT_MODIFY_ROOM(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	CONT_MYHOMECHATROOM::mapped_type kRoom;
	kRoom.ReadFromPacket(*pkPacket);

	BM::GUID kFindRoomGuid;
	if(false == FindGuestFromChatRoom(kOwnerGuid,kFindRoomGuid))
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	if((MIN_GUEST_COUNT > kRoom.kMaxUserCount) || (MAX_GUEST_COUNT < kRoom.kMaxUserCount))
	{
		return E_MYHOME_INVALID_GUEST_COUNT;
	}

	CONT_MYHOMECHATROOM::iterator room_iter = m_kContChatRoom.find(kFindRoomGuid);
	if(room_iter == m_kContChatRoom.end())
	{
		return E_MYHOME_NOT_FOUND_ROOM;
	}

	CONT_MYHOMECHATGUEST::iterator guest_iter = (*room_iter).second.kContGuest.find(kOwnerGuid);
	if(guest_iter == (*room_iter).second.kContGuest.end())
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	if(false == (*guest_iter).second.bRoomMaster)
	{
		return E_MYHOME_NOT_HAVE_AUTHORITY;
	}

	kRoom.kRoomGuid = kFindRoomGuid;
	kRoom.kContGuest.swap((*room_iter).second.kContGuest);// 접속자 정보만 보관 나머진 갱신한다.
	kRoom.kContKick.swap((*room_iter).second.kContKick);
	(*room_iter).second = kRoom;

	BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_MODIFY_ROOM);
	kPacket.Push(S_OK);
	kRoom.WriteToPacket(kPacket);
	SendToUser(kOwnerGuid, kPacket, false);

	BM::Stream kNoti(PT_M_C_NOTI_MYHOME_CHAT_ROOM_MODIFY);
	kRoom.WriteToPacket(kNoti);
	SendToRoom(kFindRoomGuid, kNoti, kOwnerGuid);
	return S_OK;
}

HRESULT PgMyHomeMgrImpl::Recv_PT_C_M_REQ_MYHOME_CHAT_MODIFY_GUEST(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	bool bEnablePrivateMsg = false;
	bool bNotStanding = false;
	pkPacket->Pop(bEnablePrivateMsg);
	pkPacket->Pop(bNotStanding);

	BM::GUID kFindRoomGuid;
	if(false == FindGuestFromChatRoom(kOwnerGuid,kFindRoomGuid))
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	CONT_MYHOMECHATROOM::iterator room_iter = m_kContChatRoom.find(kFindRoomGuid);
	if(room_iter == m_kContChatRoom.end())
	{
		return E_MYHOME_NOT_FOUND_ROOM;
	}

	CONT_MYHOMECHATGUEST::iterator guest_iter = (*room_iter).second.kContGuest.find(kOwnerGuid);
	if(guest_iter == (*room_iter).second.kContGuest.end())
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	(*guest_iter).second.bEnablePrivateMsg = bEnablePrivateMsg;
	(*guest_iter).second.bNotStanding = bNotStanding;

	BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_MODIFY_GUEST);
	kPacket.Push(S_OK);
	(*guest_iter).second.WriteToPacket(kPacket);
	SendToUser(kOwnerGuid, kPacket, false);

	BM::Stream kNoti(PT_M_C_NOTI_MYHOME_CHAT_GUEST_MODIFY);
	(*guest_iter).second.WriteToPacket(kNoti);
	SendToRoom(kFindRoomGuid, kNoti, kOwnerGuid);
	return S_OK;
}

HRESULT PgMyHomeMgrImpl::Recv_PT_C_M_REQ_MYHOME_CHAT_SET_ROOMMASTER(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	BM::GUID kTargetId;
	pkPacket->Pop(kTargetId);

	BM::GUID kFindRoomGuid;
	if(false == FindGuestFromChatRoom(kOwnerGuid,kFindRoomGuid))
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	CONT_MYHOMECHATROOM::iterator room_iter = m_kContChatRoom.find(kFindRoomGuid);
	if(room_iter == m_kContChatRoom.end())
	{
		return E_MYHOME_NOT_FOUND_ROOM;
	}

	CONT_MYHOMECHATROOM::mapped_type & kRoom = (*room_iter).second;

	CONT_MYHOMECHATGUEST::iterator guest_iter = kRoom.kContGuest.find(kOwnerGuid);
	if(guest_iter == kRoom.kContGuest.end())
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	CONT_MYHOMECHATGUEST::mapped_type & kGuest = (*guest_iter).second;
	if(false == kGuest.bRoomMaster)
	{
		return E_MYHOME_NOT_HAVE_AUTHORITY;
	}

	CONT_MYHOMECHATGUEST::iterator target_iter = kRoom.kContGuest.find(kTargetId);
	if(target_iter == kRoom.kContGuest.end())
	{
		return E_MYHOME_NOT_FOUND_TARGET;
	}

	CONT_MYHOMECHATGUEST::mapped_type & kTarget = (*target_iter).second;

	kGuest.bRoomMaster = false;
	kTarget.bRoomMaster = true;
	
	{
		BM::Stream kNoti(PT_M_C_NOTI_MYHOME_CHAT_GUEST_MODIFY);
		kGuest.WriteToPacket(kNoti);
		SendToRoom(kFindRoomGuid, kNoti);
	}
	{
		BM::Stream kNoti(PT_M_C_NOTI_MYHOME_CHAT_GUEST_MODIFY);
		kTarget.WriteToPacket(kNoti);
		SendToRoom(kFindRoomGuid, kNoti);
	}

	BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_SET_ROOMMASTER);
	kPacket.Push(S_OK);
	SendToUser(kOwnerGuid, kPacket, false);
	return S_OK;
}

HRESULT PgMyHomeMgrImpl::Recv_PT_C_M_REQ_MYHOME_CHAT_KICK_GEUST(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	BM::GUID kTargetId;
	pkPacket->Pop(kTargetId);

	BM::GUID kFindRoomGuid;
	if(false == FindGuestFromChatRoom(kOwnerGuid,kFindRoomGuid))
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	CONT_MYHOMECHATROOM::iterator room_iter = m_kContChatRoom.find(kFindRoomGuid);
	if(room_iter == m_kContChatRoom.end())
	{
		return E_MYHOME_NOT_FOUND_ROOM;
	}

	CONT_MYHOMECHATROOM::mapped_type & kRoom = (*room_iter).second;

	CONT_MYHOMECHATGUEST::iterator guest_iter = kRoom.kContGuest.find(kOwnerGuid);
	if(guest_iter == kRoom.kContGuest.end())
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	CONT_MYHOMECHATGUEST::mapped_type & kGuest = (*guest_iter).second;
	if(false == kGuest.bRoomMaster)
	{
		return E_MYHOME_NOT_HAVE_AUTHORITY;
	}

	CONT_MYHOMECHATGUEST::iterator target_iter = kRoom.kContGuest.find(kTargetId);
	if(target_iter == kRoom.kContGuest.end())
	{
		return E_MYHOME_NOT_FOUND_TARGET;
	}

	BM::Stream kNoti(PT_M_C_NOTI_MYHOME_CHAT_KICK_GEUST);
	kNoti.Push(kTargetId);
	SendToRoom(kFindRoomGuid, kNoti);

	kRoom.kContKick.insert(kTargetId);
	kRoom.kContGuest.erase(target_iter);
	m_kContChatPlayer.erase(kTargetId);

	BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_KICK_GEUST);
	kPacket.Push(S_OK);
	SendToUser(kOwnerGuid, kPacket, false);
	return S_OK;
}

HRESULT PgMyHomeMgrImpl::Recv_PT_C_M_REQ_CHAT(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	BYTE byChatType = 0;
	std::wstring wstrChat;
	pkPacket->Pop(byChatType);
	pkPacket->Pop(wstrChat);

	BM::GUID kFindRoomGuid;
	if(false == FindGuestFromChatRoom(kOwnerGuid,kFindRoomGuid))
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	CONT_MYHOMECHATROOM::iterator room_iter = m_kContChatRoom.find(kFindRoomGuid);
	if(room_iter == m_kContChatRoom.end())
	{
		return E_MYHOME_NOT_FOUND_ROOM;
	}

	CONT_MYHOMECHATROOM::mapped_type const & kRoom = (*room_iter).second;

	CONT_MYHOMECHATGUEST::const_iterator guest_iter = kRoom.kContGuest.find(kOwnerGuid);
	if(guest_iter == kRoom.kContGuest.end())
	{
		return E_MYHOME_NOT_IN_CHAT_ROOM;
	}

	CONT_MYHOMECHATGUEST::mapped_type const & kGuest = (*guest_iter).second;

	switch(byChatType)
	{
	case CT_MYHOME_PUBLIC:
		{
			BM::Stream kPacket( PT_M_C_NFY_CHAT, byChatType );
			kPacket.Push(kGuest.kID);
			kPacket.Push(kGuest.kName);
			//kPacket.Push(std::chrono::system_clock::now());
			kPacket.Push(wstrChat);
			kPacket.Push(*pkPacket);
			SendToRoom(kFindRoomGuid,kPacket);
		}break;
	case CT_MYHOME_PRIVATE:
		{
			BM::GUID kTargetGuid;
			pkPacket->Pop(kTargetGuid);

			CONT_MYHOMECHATGUEST::const_iterator target_iter = kRoom.kContGuest.find(kTargetGuid);
			if(target_iter == kRoom.kContGuest.end())
			{
				return E_MYHOME_NOT_FOUND_TARGET;
			}

			if(true == (*target_iter).second.bEnablePrivateMsg)
			{
				return E_MYHOME_DISABLE_PRIVATEMSG;
			}

			BM::Stream kPacket( PT_M_C_NFY_CHAT, byChatType );
			kPacket.Push(kGuest.kID);
			kPacket.Push(kGuest.kName);
			//kPacket.Push(std::chrono::system_clock::now());
			kPacket.Push(wstrChat);
			kPacket.Push(*pkPacket);
			SendToUser(kTargetGuid, kPacket, false);
			SendToUser(kOwnerGuid, kPacket, false);
		}break;
	}

	return S_OK;
}

void PgMyHomeMgrImpl::RecvHomeChatHandler(BM::Stream::DEF_STREAM_TYPE const kPacketType, BM::Stream * const pkPacket)
{
	switch(kPacketType)
	{
	case PT_C_M_REQ_CHAT:
		{
			BM::GUID kOwnerGuid;

			pkPacket->Pop(kOwnerGuid);
			HRESULT const hResult = Recv_PT_C_M_REQ_CHAT(kOwnerGuid, pkPacket);
			if(S_OK == hResult)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_NOTI_MYHOME_CHAT_RESULT);
			kPacket.Push(hResult);
			SendToUser(kOwnerGuid, kPacket, false);
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_ENTER:
		{
			BM::GUID	kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT const hResult = Recv_PT_C_M_REQ_MYHOME_CHAT_ENTER(kOwnerGuid, pkPacket);
			if(S_OK == hResult)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_ENTER);
			kPacket.Push(hResult);
			SendToUser(kOwnerGuid, kPacket, false);
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_EXIT:
		{
			BM::GUID	kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT const hResult = Recv_PT_C_M_REQ_MYHOME_CHAT_EXIT(kOwnerGuid, pkPacket);
			if(S_OK == hResult)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_EXIT);
			kPacket.Push(hResult);
			SendToUser(kOwnerGuid, kPacket, false);
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_MODIFY_ROOM:
		{
			BM::GUID	kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT const hResult = Recv_PT_C_M_REQ_MYHOME_CHAT_MODIFY_ROOM(kOwnerGuid, pkPacket);
			if(S_OK == hResult)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_MODIFY_ROOM);
			kPacket.Push(hResult);
			SendToUser(kOwnerGuid, kPacket, false);
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_MODIFY_GUEST:
		{
			BM::GUID	kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT const hResult = Recv_PT_C_M_REQ_MYHOME_CHAT_MODIFY_GUEST(kOwnerGuid, pkPacket);
			if(S_OK == hResult)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_MODIFY_GUEST);
			kPacket.Push(hResult);
			SendToUser(kOwnerGuid, kPacket, false);
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_ROOM_LIST:
		{
			BM::GUID kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);
			BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_ROOM_LIST);
			PU::TWriteTable_AM(kPacket,ContChatRoom());
			SendToUser(kOwnerGuid,kPacket,false);
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_ROOM_CREATE:
		{
			BM::GUID	kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT const hResult = Recv_PT_C_M_REQ_MYHOME_CHAT_ROOM_CREATE(kOwnerGuid, pkPacket);
			if(S_OK == hResult)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_ROOM_CREATE);
			kPacket.Push(hResult);
			SendToUser(kOwnerGuid, kPacket, false);
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_SET_ROOMMASTER:
		{
			BM::GUID	kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT const hResult = Recv_PT_C_M_REQ_MYHOME_CHAT_SET_ROOMMASTER(kOwnerGuid, pkPacket);
			if(S_OK == hResult)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_SET_ROOMMASTER);
			kPacket.Push(hResult);
			SendToUser(kOwnerGuid, kPacket, false);
		}break;
	case PT_C_M_REQ_MYHOME_CHAT_KICK_GEUST:
		{
			BM::GUID	kOwnerGuid;
			pkPacket->Pop(kOwnerGuid);

			HRESULT const hResult = Recv_PT_C_M_REQ_MYHOME_CHAT_KICK_GEUST(kOwnerGuid, pkPacket);
			if(S_OK == hResult)
			{
				return;
			}

			BM::Stream kPacket(PT_M_C_ANS_MYHOME_CHAT_KICK_GEUST);
			kPacket.Push(hResult);
			SendToUser(kOwnerGuid, kPacket, false);
		}break;
	}
}

HRESULT PgMyHomeMgrImpl::Proc_PT_M_I_REQ_MYHOME_AUCTION_UNREG(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	CONT_PLAYER_MODIFY_ORDER kOrder;

	SHOMEADDR kAddr;
	pkPacket->Pop(kAddr);

	SMYHOME kMyHome;
	if(false == GetHome(kAddr.StreetNo(),kAddr.HouseNo(),kMyHome))
	{
		return E_MYHOME_NOT_FOUND;
	}

	if(MAS_NOT_BIDDING == kMyHome.bAuctionState)
	{
		return E_MYHOME_IS_NOT_AUCTION_ARITCLE;
	}

	if(kOwnerGuid != kMyHome.kOwnerGuid)
	{
		return E_MYHOME_NOT_HAVE_AUTHORITY;
	}

	// 경매 취소 수수료를 뺀다.
	__int64 const i64Tex = (kMyHome.i64FirstBiddingCost*MATR_CANCEL)/100;

	kOrder.push_back(SPMO(IMET_ADD_MONEY,kOwnerGuid,SPMOD_Add_Money(-i64Tex)));
	kOrder.push_back(SPMO(IMET_MYHOME_AUCTION_UNREG, kMyHome.kHomeInSideGuid,SMOD_MyHome_Auction_Unreg(kAddr.StreetNo(),kAddr.HouseNo())));

	BM::Stream kPacket(PT_M_C_ANS_MYHOME_AUCTION_UNREG);
	SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
	pkActionOrder->InsertTarget(kOwnerGuid);
	pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
	pkActionOrder->kCause = CIE_Home_Auction_Unreg;
	pkActionOrder->kContOrder = kOrder;
	pkActionOrder->kAddonPacket.Push(kPacket.Data());
	g_kJobDispatcher.VPush(pkActionOrder);

	return S_OK;
}

HRESULT PgMyHomeMgrImpl::Proc_PT_M_I_REQ_MYHOME_PAY_TEX(BM::GUID const & kOwnerGuid, BM::Stream * const pkPacket)
{
	short	siStreetNo = 0;
	int		iHouseNo = 0;

	pkPacket->Pop(siStreetNo);
	pkPacket->Pop(iHouseNo);

	SMYHOME kMyHome;
	if(false == GetHome(siStreetNo,iHouseNo,kMyHome))
	{
		return E_MYHOME_NOT_FOUND;
	}

	if(kMyHome.kOwnerGuid != kOwnerGuid)
	{
		return E_MYHOME_NOT_HAVE_AUTHORITY;
	}

	if(MAS_IS_BIDDING == kMyHome.bAuctionState)
	{
		return E_MYHOME_IS_NOT_PAY_TEX_TIME; // 어차피 팔집에 무슨 세금을 내나? 내지마 ㅡㅡ;;
	}

	__int64 const i64TexCost = MYHOMEUTIL::CalcTex(kMyHome);
	if(0 >= i64TexCost)
	{
		return E_MYHOME_IS_NOT_PAY_TEX_TIME;
	}

	CONT_PLAYER_MODIFY_ORDER kOrder;
	kOrder.push_back(SPMO(IMET_ADD_MONEY,kOwnerGuid,SPMOD_Add_Money(-i64TexCost)));
	kOrder.push_back(SPMO(IMET_MYHOME_PAY_TEX,kMyHome.kHomeInSideGuid,SMOD_MyHome_PayTex(siStreetNo,iHouseNo)));

	BM::Stream kPacket(PT_M_C_ANS_MYHOME_PAY_TEX);
	SActionOrder* pkActionOrder = PgJobWorker::AllocJob();
	pkActionOrder->InsertTarget(kOwnerGuid);
	pkActionOrder->InsertTarget(kMyHome.kHomeInSideGuid);
	pkActionOrder->kCause = CIE_Home_PayTex;
	pkActionOrder->kContOrder = kOrder;
	pkActionOrder->kAddonPacket.Push(kPacket.Data());
	g_kJobDispatcher.VPush(pkActionOrder);

	return S_OK;
}

//===============================================================================================================================================================================================

bool PgMyHomeMgr::GetHome(short const kStreetNo,int const kHouseNo,SMYHOME & kMyHome)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_);
	return Instance()->GetHome(kStreetNo,kHouseNo,kMyHome);
}

bool PgMyHomeMgr::SetHome(SMYHOME & kMyHome)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->SetHome(kMyHome);
}

bool PgMyHomeMgr::AddBiddingTime(BM::GUID const & kOwnerGuid)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->AddBiddingTime(kOwnerGuid);
}

bool PgMyHomeMgr::AddBidder(BM::GUID const & kOwnerGuid)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->AddBidder(kOwnerGuid);
}

void PgMyHomeMgr::RemoveBidder(BM::GUID const & kOwnerGuid)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->RemoveBidder(kOwnerGuid);
}

bool PgMyHomeMgr::IsBidding(BM::GUID const & kOwnerGuid)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_);
	return Instance()->IsBidding(kOwnerGuid);
}

bool PgMyHomeMgr::HasBiddingDelayTime(BM::GUID const & kOwnerGuid)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->HasBiddingDelayTime(kOwnerGuid);
}

void PgMyHomeMgr::Tick()
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_);
	return Instance()->Tick();
}

HRESULT PgMyHomeMgr::ProcessModifyPlayer(EItemModifyParentEventType const kCause, SPMO const &kOrder, PgDoc_Player* pkDocPlayer,BM::Stream const &kAddonPacket, BM::Stream & rkPacket, DB_ITEM_STATE_CHANGE_ARRAY &kChangeArray, PgContLogMgr & kContLogMgr)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->ProcessModifyPlayer(kCause,kOrder,pkDocPlayer,kAddonPacket,rkPacket,kChangeArray,kContLogMgr);
}

void PgMyHomeMgr::AddMyHome(short const siStreetNo,CONT_MYHOME const & kCont)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->AddMyHome(siStreetNo, kCont);
}

bool PgMyHomeMgr::RemoveMyHome(short const siStreetNo, int const iHouseNo)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->RemoveMyHome(siStreetNo, iHouseNo);
}

void PgMyHomeMgr::RecvPacketHandler(BM::Stream::DEF_STREAM_TYPE const kPacketType,SERVER_IDENTITY const & kSI,SGroundKey const & kGndKey, BM::Stream * const pkPacket)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->RecvPacketHandler(kPacketType,kSI,kGndKey,pkPacket);
}

bool PgMyHomeMgr::IsLocked(BM::GUID const kGuid) const
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,false);
	return Instance()->IsLocked(kGuid);
}

void PgMyHomeMgr::Lock(BM::GUID const kGuid)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->Lock(kGuid);
}

void PgMyHomeMgr::Unlock(BM::GUID const kGuid)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->Unlock(kGuid);
}

void PgMyHomeMgr::RecvHomeChatHandler(BM::Stream::DEF_STREAM_TYPE const kPacketType, BM::Stream * const pkPacket)
{
	BM::CAutoMutex lock(m_kMutex_Wrapper_,true);
	return Instance()->RecvHomeChatHandler(kPacketType, pkPacket);
}