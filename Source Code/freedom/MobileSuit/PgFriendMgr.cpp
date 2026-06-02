#include "stdafx.h"

#include "PgChatMgrClient.h"
#include "PgFriendMgr.h"
#include "PgNetwork.h"
#include "PgScripting.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgOption.h"

typedef enum EFriendGroupDefulat_TTW
{
	EFGD_TTW_REQUESTER			= 400521,
	EFGD_TTW_ACCEPTWAIT			= 400501,
	EFGD_TTW_NOGROUP			= 400502,
	//EFGD_TTW_ONLINE				= 400503,
	//EFGD_TTW_OFFLINE			= 400504,
} eFriendGroupDefault_TTW;

extern int const iByteFilter = 0x000000FF;

PgFriendMgr::PgFriendMgr()
{
	Friend_Clear();

	MyLinkStatus(FLS_OFFLINE);


	m_dwLastUpdateFriendList = BM::GetTime32()-EFP_FriendPalam_Last_Update;
}
PgFriendMgr::~PgFriendMgr()
{
}

bool PgFriendMgr::SendFriend_Command(int const iCommandType, SFriendItem &rkFriendItem, int const iPalam)
{
	ContFriend kCont;
	kCont.push_back(rkFriendItem);
	return SendFriend_Command_Multi(iCommandType, kCont, iPalam);
}

bool PgFriendMgr::SendFriend_Command_Multi(int const iCommandType, const ContFriend &rkCont, int const iPalam)
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);

	BM::Stream kPacket(PT_C_N_REQ_MSN_FRIENDCOMMAND);
	kPacket.Push((BYTE)iCommandType);
	SContentsUser kUser;

	switch(iCommandType)
	{
	case FCT_ADD_BYGUID:
	case FCT_ADD_BYNAME:
	case FCT_MODIFY_CHATSTATUS:
	case FCT_ADD_ACCEPT:
	case FCT_ADD_REJECT:
	case FCT_DELETE:
	case FCT_MODIFY:
	case FCT_GROUP:
		{
			PU::TWriteArray_M(kPacket, rkCont);
		}break;
	case FCT_LINKSTATUS:
		{
			kPacket.Push((BYTE)MyLinkStatus());
		}break;
	default:
		{
			PG_ASSERT_LOG(0);
			return false;
		}break;
	}

	NETWORK_SEND(kPacket)
	return bSendRet;
}

//
bool PgFriendMgr::RecvFriend_Command(BM::Stream* pkPacket)
{

	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);
	BM::GUID kCharGuid;

	BYTE cFriendCommand = -1;
	BYTE cErrorCode = -1;
	pkPacket->Pop(cFriendCommand);
	pkPacket->Pop(cErrorCode);
	int iNumFriend = 0;
	//SFriendItem kFriendItem;

	SChatLog kChatLog(CT_EVENT);

	switch(cFriendCommand)
	{
	case FCT_ADD_BYGUID:
		{	
			switch(cErrorCode)
			{
			case FEC_Success:
				{
					pkPacket->Pop(iNumFriend);

					for(int i=0; iNumFriend > i; ++i)
					{
						SFriendItem kFriendItem;
						kFriendItem.ReadFromPacket(*pkPacket);
						Friend_Add(kFriendItem);//추가

						if(0 != kFriendItem.ChatStatus())
						{
							std::wstring kTTW;
							if(0 != (kFriendItem.ChatStatus()&FCS_ADD_WAIT_ANS))//추가 당한 사람이(수락자))
							{//요청자(는 허가자 정보가 온다)
								//kTTW = TTW(EM_FriendRecver);
								PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
								if(pkPlayer)
								{
									std::wstring const kReqMessage = kFriendItem.ExtendName() + TTW(EM_FriendRecver_Req);
									lwCallYesNoMsgBox(kReqMessage, lwGUID(kFriendItem.CharGuid()), MBT_FRIEND_ADD);
								}
								break;
							}
							else if(0 != (kFriendItem.ChatStatus()&FCS_ADD_WAIT_REQ))//추가 행동을 한 사람(신청자)
							{//허가자(는 요청자 정보가 온다)
								kTTW = TTW(EM_FriendAdder);
							}

							//성공메시지
							std::wstring const &kFriendName = kFriendItem.ExtendName();
							std::wstring const kContents = kFriendName + kTTW; //누구가 무엇을 하였습니다.
							g_kChatMgrClient.AddLogMessage(kChatLog, kContents, true);
						}
					}
				}break;
			case FEC_NotFound:
				{
					g_kChatMgrClient.AddLogMessage(kChatLog, TTW(EM_FreindFailed), true);
				}break;
			case FEC_Already_Added:
				{
					/*BM::GUID kGuid;
					pkPacket->Pop(kGuid);
					PgPilot *pkPilot = g_kPilotMan.FindPilot(kGuid);
					if (pkPilot)
					{
						CUnit* pkUnit = pkPilot->GetUnit();
					}*/
					g_kChatMgrClient.AddLogMessage(kChatLog, TTW(EM_FriendAdded), true);
				}break;
			case FEC_Failed_Me:
				{
					g_kChatMgrClient.AddLogMessage(kChatLog, TTW(700039), true);
				}break;
			case FEC_Failed:
				{
					g_kChatMgrClient.AddLogMessage(kChatLog, TTW(700072), true);
				}break;
			case FEC_Failed_Limit:
				{
					g_kChatMgrClient.AddLogMessage(kChatLog, TTW(700073), true);
				}break;
			}
		}break;
	case FCT_ADD_ACCEPT:
		{	
			std::wstring kTTW;
			std::wstring kFriendName;

			pkPacket->Pop(kFriendName);

			switch(cErrorCode)
			{
			case FEC_Success:
				{
					kTTW = TTW(EM_FriendAdder);
					std::wstring const kContents = kFriendName + kTTW;
					g_kChatMgrClient.AddLogMessage(kChatLog, kContents, true);
				}break;
			case FEC_Accept:
				{
					kTTW = TTW(EM_FriendRecver);
					std::wstring const kContents = kFriendName + kTTW;
					g_kChatMgrClient.AddLogMessage(kChatLog, kContents, true);
				}break;
			}
		}break;
	case FCT_ADD_REJECT:
		{
			BM::GUID kGuid;
			std::wstring kFriendName;
			
			pkPacket->Pop(kGuid);
			pkPacket->Pop(kFriendName);

			BM::vstring kString(kFriendName);
			kString+=TTW(400596);
			g_kChatMgrClient.AddLogMessage(kChatLog, (std::wstring const&)kString, true);

			SFriendItem kFriendItem;
			if(Friend_Find_ByGuid(kGuid, kFriendItem))
			{
				SendFriend_Command(FCT_DELETE, kFriendItem);
			}
		}break;
	case FCT_DELETE:
		{
			pkPacket->Pop(iNumFriend);
			for(int i=0; iNumFriend > i; ++i)
			{
				SFriendItem kFriendItem;
				kFriendItem.ReadFromPacket(*pkPacket);
				ContFriend::iterator kIter = m_kFriendList.begin();
				while (m_kFriendList.end() != kIter)
				{
					const SFriendItem &pkItem = *kIter;
					if (pkItem.CharGuid() == kFriendItem.CharGuid())
					{
						m_kFriendList.erase(kIter);
						lwCloseToolTip();
						break;
					}
					++kIter;
				}

				g_kChatMgrClient.Name2Guid_Remove(kFriendItem.CharGuid());
			}
		}break;
	case FCT_MODIFY:
		{
			pkPacket->Pop(iNumFriend); 
			ContFriend kFriendVec;
			kFriendVec.reserve(iNumFriend);
			for(int i=0; iNumFriend > i; ++i)
			{
				SFriendItem kFriendItem;
				kFriendItem.ReadFromPacket(*pkPacket);
				kFriendVec.push_back(kFriendItem);
				if( 0 != g_kGlobalOption.GetValue(XML_ELEMENT_ETC, "NOTIFY_FRIEND") )
				{
					SFriendItem* pkItem = Friend_Find_ByGuid(kFriendItem.CharGuid());
					if( pkItem)
					{	
						if( (0 == pkItem->GroundKey().GroundNo()) && (0 != kFriendItem.Class()) )
						{
							g_kChatMgrClient.ShowNotifyConnectInfo(NCI_FRIEND, kFriendItem.ExtendName(), kFriendItem.Class(), SUVInfo(), kFriendItem.CharGuid());
						}
					}
				}
			}
			ModifyList(kFriendVec);
		}break;
	case FCT_MODIFY_CHATSTATUS:
		{//처음 허락시 
			BYTE eChat = 0;
			pkPacket->Pop(kCharGuid);	
			pkPacket->Pop(eChat);
			SFriendItem *pkFriendItem = Friend_Find_ByGuid(kCharGuid);
			if (pkFriendItem)
			{
				pkFriendItem->ChatStatus(eChat);
			}
		}break;
	case FCT_FULL_DATA:
		{
			Friend_Clear();
			pkPacket->Pop(iNumFriend);
			ContFriend kFriendVec;
			for(int i=0; iNumFriend > i; ++i)
			{
				SFriendItem kFriendItem;
				kFriendItem.Clear();
				kFriendItem.ReadFromPacket(*pkPacket);
				Friend_Add(kFriendItem);
			}
		}break;
	case FCT_NOT_DELETE:
		{
			g_kChatMgrClient.AddLogMessage(kChatLog, TTW(700500), true);
		}break;
	default:
		{
		}break;
	}

	Friend_ListUpdate();

	return true;
}

//
int PgFriendMgr::Friend_Count()
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);

	return (int)m_kFriendList.size();
}

bool PgFriendMgr::Friend_Clear()
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);

	//목록
	m_kFriendList.erase(m_kFriendList.begin(), m_kFriendList.end());
	return true;
}
int PgFriendMgr::Friend_Add(const SFriendItem &rkFriendItem)
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);

	std::wstring kGroupName;
	SFriendItem kFriendItem = rkFriendItem;//복사

	//입력순 저장
	{//Iter 스콥
		bool bDuplicate = false;
		ContFriend::iterator kIter = m_kFriendList.begin();//유효성 검사
		while(m_kFriendList.end() != kIter)
		{
			if(kFriendItem == *kIter)
			{//같으면 데이터만 새로 갱신
				*kIter = kFriendItem;
				bDuplicate = true;
			}
			++kIter;
		}

		if(!bDuplicate)
		{
			m_kFriendList.push_back(kFriendItem);
		}

		/*if( FLS_ONLINE == rkFriendItem.LinkStatus() )
		{
			g_kChatMgrClient.Name2Guid_Add( CT_FRIEND, kFriendItem.ExtendName(), kFriendItem.CharGuid() );
		}*/
	}

	return m_kFriendList.size();
}

void PgFriendMgr::Friend_ListUpdate()
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);

	ContFriend m_kFriendListOnOffLineTemp;
	m_kFriendListOnOffLineTemp.clear();
	g_kChatMgrClient.Name2Guid_Remove(CT_FRIEND);

	for(int i=0; i<4; i++)
	{
		ContFriend::iterator kIter = m_kFriendList.begin();
		while(m_kFriendList.end() != kIter)
		{
			SFriendItem &kListItem = (*kIter); 

			bool bReq = (0 != (kListItem.ChatStatus()&FCS_ADD_WAIT_ANS));
			bool bWait = (0 != (kListItem.ChatStatus()&FCS_ADD_WAIT_REQ));

			if( (i == 0 && (bReq) && (!bWait)) || (i == 1 && (bWait) && (!bReq)) || (i == 2 && kListItem.LinkStatus() == FLS_ONLINE && (!bWait) && (!bReq)) || (i == 3 && kListItem.LinkStatus() != FLS_ONLINE && (!bWait) && (!bReq)) )
			{
				m_kFriendListOnOffLineTemp.push_back(kListItem);

				if( i == 2 )
				{					
					g_kChatMgrClient.Name2Guid_Add( CT_FRIEND, kListItem.ExtendName(), kListItem.CharGuid() );
				}
			}
			++kIter;
		}
	}

	m_kFriendList.clear();

	ContFriend::iterator kIterOnOff = m_kFriendListOnOffLineTemp.begin();
	while(m_kFriendListOnOffLineTemp.end() != kIterOnOff )
	{
		SFriendItem &kListItem = (*kIterOnOff);

		m_kFriendList.push_back(kListItem);

		++kIterOnOff;
	}

	m_kFriendListOnOffLineTemp.clear();
}

bool PgFriendMgr::Friend_Find_ByGuid(BM::GUID const &rkCharGuid, SFriendItem &rkFriendItem)
{//친구 찾기 Guid로
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);

	if (BM::GUID::NullData() == rkCharGuid)
	{
		return false;
	}

	ContFriend::iterator kIter = m_kFriendList.begin();
	while(m_kFriendList.end() != kIter)
	{
		if((*kIter).CharGuid() == rkCharGuid)
		{
			rkFriendItem = *kIter;
			return true;
		}
		++kIter;
	}

	return false;
}
bool PgFriendMgr::Friend_Find_ByName(std::wstring const &rkFriendName, SFriendItem &rkFriendItem)
{//친구 찾기 이름으로
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);

	ContFriend::iterator kIter = m_kFriendList.begin();
	while(m_kFriendList.end() != kIter)
	{
		if((*kIter).ExtendName() == rkFriendName)
		{
			rkFriendItem = *kIter;
			return true;
		}
		++kIter;
	}

	return false;
}

//내부용
SFriendItem* PgFriendMgr::Friend_Find_ByGuid(BM::GUID const &rkCharGuid)
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);

	if (BM::GUID::NullData() == rkCharGuid)
	{
		return NULL;
	}

	ContFriend::iterator kIter = m_kFriendList.begin();
	while(m_kFriendList.end() != kIter)
	{
		if(kIter->CharGuid() == rkCharGuid)
		{
			return &(*kIter);
		}
		++kIter;
	}

	return NULL;
}
/*
SFriendItem* PgFriendMgr::Friend_Find_ByName(std::wstring const &rkFriendName)
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);
	ContFriend::iterator kIter = m_kFriendList.begin();
	while(m_kFriendList.end() != kIter)
	{
		if(_tccmp(kIter->kExtendName, rkFriendName.c_str()) == 0)
		{
			return &(*kIter);
		}
		++kIter;
	}
	return NULL;
}*/


bool PgFriendMgr::Friend_Find_ByAt(const unsigned int iNo, SFriendItem &rkFriendItem)
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);
	if(m_kFriendList.size()!=0 && m_kFriendList.size()<=iNo)
	{
		NILOG(PGLOG_ERROR, "Critical out of range array %d\n", iNo); assert(0);
		return false;
	}

	rkFriendItem = m_kFriendList[iNo];

	return true;
}

//
bool PgFriendMgr::Find_GroupItem(std::wstring const &rkGroupName, SFriendItem &rkFriendItem)
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);

	ContFriend::const_iterator kIter = m_kFriendList.begin();
	while (m_kFriendList.end() != kIter)
	{
		const SFriendItem &kItem = (*kIter);
		if (kItem.GroupName() == rkGroupName && 
			kItem.CharGuid() == BM::GUID::NullData())
		{
			rkFriendItem = kItem;
			return true;
		}
		++kIter;
	}
	
	return false;
}

bool PgFriendMgr::Friend_Find_ByGroup(std::wstring const &rkGroupName, ContFriend &rkCont)
{
	rkCont.clear();
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);
	ContFriend::const_iterator frnd_iter = m_kFriendList.begin();
	while(m_kFriendList.end() != frnd_iter)
	{
		SFriendItem rkFriend = (*frnd_iter);
		if (rkFriend.GroupName() == rkGroupName)
		{
			if (rkFriend.CharGuid() != BM::GUID::NullData())
			{
				rkCont.push_back(rkFriend);
			}
		}
		++frnd_iter;
	}

	return !rkCont.empty();
}

int PgFriendMgr::CountOfGroup(std::wstring &rkGroupName, bool bLogin)
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);
	int iCount=0;

	ContFriend::const_iterator kIter = m_kFriendList.begin();

	while (m_kFriendList.end() != kIter)
	{
		SFriendItem kItem = (*kIter);
		if (kItem.CharGuid() != BM::GUID::NullData() &&
			kIter->GroupName() == rkGroupName)
		{
			if(!bLogin || kIter->LinkStatus() != FLS_OFFLINE)
			{
				++iCount;
			}
		}
		++kIter;
	}

	return iCount;
}
int PgFriendMgr::CountOfLogin(bool bLogin)
{
	int iCount = 0;

	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);
	ContFriend::const_iterator kIter = m_kFriendList.begin();
	for(; m_kFriendList.end() != kIter; ++kIter)
	{
		if(kIter->CharGuid() == BM::GUID::NullData())
		{
			continue;
		}
		bool bCount = (bLogin)? kIter->LinkStatus()!=FLS_OFFLINE: kIter->LinkStatus()==FLS_OFFLINE; //(bLogin)? 온라인: 오프라인;
		if(bCount)
		{
			++iCount;
		}
	}

	return iCount;
}
int PgFriendMgr::CountInGroupFriend(std::wstring const &rkGroupName)
{
	int iCount = 0;
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);
	ContFriend::const_iterator kIter = m_kFriendList.begin();
	while (m_kFriendList.end() != kIter)
	{
		const SFriendItem &kItem = (*kIter);
		if(kItem.GroupName() == rkGroupName)
		{
			if(kItem.CharGuid() != BM::GUID::NullData())
			{
				++iCount;
			}
		}

		++kIter;
	}
	
	return iCount;
}

//
bool PgFriendMgr::Friend_List_UpdateReq(bool bImmediate)
{
	DWORD dwNowTime = BM::GetTime32();
	DWORD dwCur = dwNowTime-m_dwLastUpdateFriendList;

	bool bSend = false;
/*
	if(EFP_FriendPalam_Last_Update < dwCur || bImmediate)
	{
		bSend = true;
	}

	if(bSend)
	{
		m_dwLastUpdateFriendList = dwNowTime;//시간기록

		SendFriend_Command(FCT_ListReq, NULL, 0);
	}
*/
	return bSend;
}

bool PgFriendMgr::RemoveGroup(std::wstring const &kName)
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);
	ContFriend::iterator frnd_iter = m_kFriendList.begin();

	while (m_kFriendList.end() != frnd_iter)
	{
		SFriendItem &kGroup = (*frnd_iter); 
		if (kGroup.CharGuid() == BM::GUID::NullData() && kGroup.ExtendName().empty() && kGroup.GroupName() == kName)
		{
			m_kFriendList.erase(frnd_iter);
			return true;
		}
		++frnd_iter;
	}

	return false;
}

bool PgFriendMgr::ChangeGroupName(std::wstring const &kOldName, std::wstring const &kNewName)
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);
	ContFriend::iterator frnd_iter = m_kFriendList.begin();

	while (m_kFriendList.end() != frnd_iter)
	{
		SFriendItem &kGroup = (*frnd_iter); 
		if (kGroup.CharGuid() == BM::GUID::NullData() && kGroup.ExtendName().empty() && kGroup.GroupName() == kOldName)
		{
			kGroup.GroupName(kNewName);
			return true;
		}
		++frnd_iter;
	}

	return false;
}

void PgFriendMgr::ModifyList( ContFriend const &rkCont, int const iCommandType)
{
	BM::CAutoMutex kLock_1(m_kFriendList_Mutex);
	ContFriend::const_iterator mdf_iter = rkCont.begin();

	bool bTwinkle = false;
	while (rkCont.end() != mdf_iter)
	{
		const SFriendItem &rkModify_Item = (*mdf_iter);
		SFriendItem* pkItem = Friend_Find_ByGuid(rkModify_Item.CharGuid());
		if (pkItem)
		{    //이전상태에서 바뀌고, 가공기계 상태가 (고장/동작/완료) 인지 확인

			std::wstring const kPastGroupName = pkItem->GroupName();
			*pkItem = rkModify_Item;
			if (CountInGroupFriend(kPastGroupName)<=0)	//더 이상 이 그룹에 사람이 없으면
			{
				RemoveGroup(kPastGroupName);
			}
		}
		++mdf_iter;
	}

	//lua_tinker::call<void>("Update_FriendList");
}