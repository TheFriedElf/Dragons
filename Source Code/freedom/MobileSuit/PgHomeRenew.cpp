#include "stdafx.h"
#include "PgHomeRenew.h"
#include "PgRenderMan.h"
#include "PgRenderer.h"
#include "PgBatchRender.h"
#include "Pg2DString.h"
#include "lwUI.h"
#include "PgHouse.h"
#include "PgPuppet.h"
#include "PgNetwork.h"
#include "lwHomeUI_Script.h"
#include "PgMobileSuit.h"
#include "PgHomeTown.h"
#include "PgPilotMan.h"
#include "PgCoupleMgr.h"
#include "PgGuild.h"
#include "PgFriendMgr.h"
#include "PgHome.h"
#include "PgPilot.h"
#include "ServerLib.h"
#include "NewWare/Renderer/DrawWorkflow.h"
#include "NewWare/Renderer/Kernel/RenderStateTagExtraData.h"
#include "NewWare/Renderer/TextBoardTextures.h"
#include "lwUIItemSocketSystem.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "PgUIDrawObject.h"
#include "PgEventTimer.h"
#include "PgCommandMgr.h"
#include "lwCashItem.h"

std::wstring const kMyHomeBoardName(L"Myhome_Board1001_01 0");
std::wstring const kMyHomeTagName(L"NameTag");
std::wstring const FONT_MYHOME_BOARD = _T("Font_Myhome_board");
int const MAX_MYHOME_BOARD_NUM = 6;
int const MAX_MYHOME_NAMETAG_NUM = 10;
float const DRAWBOARD_DELAY_TIME = 5.0f;
int const BUILDING_MAX_HOUSE_COUNT = 10;
int const BUILDING_MAP_NUMBER = 9018100;
int const MAX_CHAT_COUNT = 500;
int const MAX_PUBLIC_CHAT_USER_COUNT = 0;// 0이면 전체 채팅방

namespace PgChatMgrUtil
{
	extern DWORD ChatInputColor(EChatType const Type);
}

bool tagHOME_BOARD_KEY::operator<(tagHOME_BOARD_KEY const& rhs) const
{
	if( sFirst < rhs.sFirst )
	{
		return true;
	}
	else if( sFirst > rhs.sFirst )
	{
		return false;
	}

	return (sSecond < rhs.sSecond)?(true):(false);
}

PgHomeRenew::PgHomeRenew() 
    : m_fDrawBoardDelay(0), 
      m_iMyhomeEnterPos(0), 
      m_pkTextBoardTextures(NULL),
	  m_iChatFontColor(0xFF4D3413)
{
	m_kMyhomEnterPage.SetPageAttribute(10, 5);
	m_kMyhomeVisitLogPage.SetPageAttribute(3, 5);
	m_kMyhomeChatRoomPage.SetPageAttribute(10, 5);
}

PgHomeRenew::~PgHomeRenew()
{
}

void PgHomeRenew::Build()
{
	InitHomeBoard();
}

void PgHomeRenew::InitHomeBoard()
{
	for( int i=1; i<=MAX_MYHOME_BOARD_NUM; ++i )
	{
		for( int j=1; j<=MAX_MYHOME_NAMETAG_NUM; ++j )
		{
			SHomeBoardKey kKey(i,j);
			m_kHomeBoard.insert(std::make_pair(kKey, TTW(201014)));// 경매중
		}
	}
}

void PgHomeRenew::SetHomeBoard()
{
	for( int i=1; i<=MAX_MYHOME_BOARD_NUM; ++i )
	{
		CONT_MYHOME	kHomeList;
		g_kHomeTownMgr.GetTownHouseCont( i, kHomeList );
		CONT_MYHOME::iterator itr = kHomeList.begin();
		for( ; itr != kHomeList.end(); ++itr )
		{
			SHomeBoardKey kKey(i,itr->second.iHouseNo);
			std::pair<ContHomeBoardMap::iterator, bool> Result = m_kHomeBoard.insert(std::make_pair(kKey, itr->second.kName ) );
			if( false == Result.second )
			{
				Result.first->second = itr->second.kName;
			}
		}
	}

	ContHomeBoardMap::iterator itr_bd = m_kHomeBoard.begin();
	for( ; itr_bd != m_kHomeBoard.end(); ++itr_bd )
	{
		SetBoardNameTag(itr_bd->first.sFirst, itr_bd->first.sSecond, itr_bd->second);
	}
}

void PgHomeRenew::Update(float fAccumTime, float fFrameTime)
{
}

void PgHomeRenew::Draw( PgRenderer* pkRenderer, float fFrameTime )
{
    if ( NULL != m_pkTextBoardTextures )
    {
        // Render To Texture는 프레임 랜더링 구간에서 랜더링 해야함!(SceneDrawer::DrawRenderToTexture()에서)
        NiTexture::FormatPrefs kFormat;
        kFormat.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
        static const NiColorA s_kBoardColor(47.0f/255.0f, 28.0f/255.0f,11.0f/255.0f, 0.0f);
        m_pkTextBoardTextures->ClickAll( pkRenderer->GetRenderer(), kFormat, true, 
                                         s_kBoardColor, NiRenderer::CLEAR_ALL );

        delete m_pkTextBoardTextures;
        m_pkTextBoardTextures = NULL;
    }
}

void PgHomeRenew::SetNameforTownBoard( NiAVObject* pkMeshRoot, std::string const& rkNodeName, 
                                       std::wstring const& rkOwnerName, NiColorA const& rkTextColor )
{
	if ( pkMeshRoot == NULL )
	{
		PgError("PgHomeRenew::SetNameforTownBoard can't get NIFRoot.\n");
		return;
	}

	PgRenderer* pkRenderer = g_kRenderMan.GetRenderer();
	NiAVObject* pkNamePlate = pkMeshRoot->GetObjectByName(rkNodeName.c_str());
	if ( pkRenderer && pkNamePlate )
	{
		NiGeometry* pkGeom = NiDynamicCast(NiGeometry, pkNamePlate);
        if ( NULL == pkGeom )
		{
			NiNode* pkNode = NiDynamicCast(NiNode, pkNamePlate);
			if ( pkNode )
			{
				NiAVObject* pkObject = pkNode->GetAt(0);
				if ( pkObject )
				{
					pkGeom = NiDynamicCast(NiGeometry, pkObject);
				}
			}
		}
		if ( pkGeom )
		{
            if ( NULL == m_pkTextBoardTextures )
            {
                m_pkTextBoardTextures = new NewWare::Renderer::TextBoardTextures( PgRenderer::GetPgRenderer() );
                m_pkTextBoardTextures->Init( FONT_MYHOME_BOARD, 5, 256, 32, rkTextColor, NiColorA(0.0f, 0.0f, 0.0f, 0.0f) );
            }
            if ( m_pkTextBoardTextures->Add(PgRenderer::GetPgRenderer()->GetRenderer(), pkGeom, rkOwnerName) == false )
            {
#ifndef EXTERNAL_RELEASE
                _asm { int 3 }; // 추가 실패(pkGeom이 NULL포인터)
#endif //#ifndef EXTERNAL_RELEASE
            }
		}
	}
}

void PgHomeRenew::SetBoardNameTag(int iBoardNo, int iPos, std::wstring const& strName)
{
	if(!g_pkWorld)
	{
		return;
	}
	//SHomeBoardKey kKey(iBoardNo,iPos);
	//ContHomeBoardMap::iterator itr = m_kHomeBoard.find(kKey);
	//if( itr != m_kHomeBoard.end() )
	//{
	//	itr->second = strName;
	//}

	BM::vstring strBoard(kMyHomeBoardName);
	strBoard += iBoardNo;
	NiAVObject* pkObject = g_pkWorld->GetSceneRoot()->GetObjectByName(NiFixedString(MB(strBoard)));
	if( pkObject )
	{
		BM::vstring strTag(kMyHomeTagName);
		strTag += BM::vstring(iPos, L"%02d");
		NiColorA kTextColor(251.0f/255.0f, 251.0f/255.0f, 236.0f/255.0f, 1.0f);
		g_kHomeRenewMgr.SetNameforTownBoard(pkObject, MB(strTag), strName, kTextColor);
		pkObject->Update(0);
	}
}

bool PgHomeRenew::ProcessMsg(unsigned short const usType, BM::Stream& rkPacket)
{
	switch( usType )
	{
	case PT_M_C_ANS_MYHOME_BUY:
		{
			Recv_PT_M_C_ANS_MYHOME_BUY(rkPacket);
		}break;
	case PT_M_C_ANS_MYHOME_EXIT:
		{
			Recv_PT_M_C_ANS_MYHOME_EXIT(rkPacket);
		}break;
	case PT_M_C_ANS_EXCHANGE_LOGCOUNTTOEXP:
		{
			Recv_PT_M_C_ANS_EXCHANGE_LOGCOUNTTOEXP(rkPacket);
		}break;
	case PT_M_C_ANS_HOME_VISITLOG_ADD:
		{
			Recv_PT_M_C_ANS_HOME_VISITLOG_ADD(rkPacket);
		}break;
	case PT_M_C_ANS_HOME_VISITLOG_LIST:
		{
			Recv_PT_M_C_ANS_HOME_VISITLOG_LIST(rkPacket);
		}break;
	case PT_M_C_ANS_HOME_VISITLOG_DELETE:
		{
			Recv_PT_M_C_ANS_HOME_VISITLOG_DELETE(rkPacket);
		}break;;
	case PT_M_C_ANS_MYHOME_SELL:
		{
			Recv_PT_M_C_ANS_MYHOME_SELL(rkPacket);
		}break;
	case PT_M_C_ANS_MYHOME_CHAT_ENTER:
		{
			Recv_PT_M_C_ANS_MYHOME_CHAT_ENTER(rkPacket);
		}break;
	case PT_M_C_ANS_MYHOME_CHAT_EXIT:
		{
			Recv_PT_M_C_ANS_MYHOME_CHAT_EXIT(rkPacket);
		}break;
	case PT_M_C_ANS_MYHOME_CHAT_MODIFY_ROOM:
		{
			Recv_PT_M_C_ANS_MYHOME_CHAT_MODIFY_ROOM(rkPacket);
		}break;
	case PT_M_C_ANS_MYHOME_CHAT_MODIFY_GUEST:
		{
			Recv_PT_M_C_ANS_MYHOME_CHAT_MODIFY_GUEST(rkPacket);
		}break;
	case PT_M_C_ANS_MYHOME_CHAT_ROOM_LIST:
		{
			Recv_PT_M_C_ANS_MYHOME_CHAT_ROOM_LIST(rkPacket);
		}break;
	case PT_M_C_NOTI_MYHOME_CHAT_ROOM_ENTER:
		{
			Recv_PT_M_C_NOTI_MYHOME_CHAT_ROOM_ENTER(rkPacket);
		}break;
	case PT_M_C_NOTI_MYHOME_CHAT_ROOM_EXIT:
		{
			Recv_PT_M_C_NOTI_MYHOME_CHAT_ROOM_EXIT(rkPacket);
		}break;
	case PT_M_C_ANS_MYHOME_CHAT_ROOM_CREATE:
		{
			Recv_PT_M_C_ANS_MYHOME_CHAT_ROOM_CREATE(rkPacket);
		}break;
	case PT_M_C_NOTI_MYHOME_CHAT_ROOM_MODIFY:
		{
			Recv_PT_M_C_NOTI_MYHOME_CHAT_ROOM_MODIFY(rkPacket);
		}break;
	case PT_M_C_NOTI_MYHOME_CHAT_GUEST_MODIFY:
		{
			Recv_PT_M_C_NOTI_MYHOME_CHAT_GUEST_MODIFY(rkPacket);
		}break;
	case PT_M_C_ANS_MYHOME_CHAT_SET_ROOMMASTER:
		{
			Recv_PT_M_C_ANS_MYHOME_CHAT_SET_ROOMMASTER(rkPacket);
		}break;
	case PT_M_C_NOTI_MYHOME_CHAT_RESULT:
		{
			Recv_PT_M_C_NOTI_MYHOME_CHAT_RESULT(rkPacket);
		}break;
	case PT_M_C_ANS_MYHOME_CHAT_KICK_GEUST:
		{
			Recv_PT_M_C_ANS_MYHOME_CHAT_KICK_GEUST(rkPacket);
		}break;
	case PT_M_C_NOTI_MYHOME_CHAT_KICK_GEUST:
		{
			Recv_PT_M_C_NOTI_MYHOME_CHAT_KICK_GEUST(rkPacket);
		}break;
	case PT_M_C_ANS_MYHOME_PRICE:
		{
			int iPrice = 0;
			rkPacket.Pop( iPrice );
			lwHomeRenew::AnsOpenMyhomeBuy(iPrice);
		}break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_BUY(bool const bReqPrice, short sNum)
{
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_BUY);
	kPacket.Push(bReqPrice);
	kPacket.Push(sNum);// SHORT : 자신이 속해 있는 그라운드에 있는 아파트 건물 번호 
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_EXIT(BM::GUID const& rkGuid, bool bPrevPos)
{
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_EXIT);
	kPacket.Push(rkGuid);// Guid : 내부 홈 유닛 guid
	kPacket.Push(bPrevPos);// Bool : false : 마이홈이 위치한 마을로 이동,       True : 마이홈으로 이동하기 이전 위치로 복귀
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_EXCHANGE_LOGCOUNTTOEXP()
{
	BM::Stream kPacket(PT_C_M_REQ_EXCHANGE_LOGCOUNTTOEXP);
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_HOME_VISITLOG_ADD(BM::GUID const& rkGuid, std::wstring const& strText, CONT_ITEMPOS const& kPos, bool const bPrivate)
{
	BM::Stream kPacket(PT_C_M_REQ_HOME_VISITLOG_ADD);
	kPacket.Push(rkGuid);// Guid : 마이홈 유닛 guid
	kPacket.Push(strText);// Wstring : 방문록 내용 (MAX_HOME_VISITLOG_LEN 제한 있음)
	kPacket.Push(bPrivate);
	kPacket.Push(kPos);// CONT_ITEMPOS : 이펙트를 줄 아이템 위치 리스트
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_HOME_VISITLOG_LIST(BM::GUID const& rkHomeGuid)
{
	BM::Stream kPacket(PT_C_M_REQ_HOME_VISITLOG_LIST);
	kPacket.Push(rkHomeGuid);// 마이홈 유닛 guid
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_HOME_VISITLOG_DELETE(BM::GUID const& rkHomeGuid, BM::GUID const& rkLogGuid)
{
	BM::vstring strText(TTW(201700));
	BM::Stream kPacket(PT_C_M_REQ_HOME_VISITLOG_DELETE);
	kPacket.Push(rkHomeGuid);// 홈 guid
	kPacket.Push(rkLogGuid);// 로그 guid
	CallCommonMsgYesNoBox(strText, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
	//NETWORK_SEND(kPacket);
}

bool MyhomeVisitLogListSort(SHOMEVISITLOG const& lhs, SHOMEVISITLOG const& rhs)
{
	return !(lhs.kLogTime < rhs.kLogTime);
}

void PgHomeRenew::Recv_PT_M_C_ANS_HOME_VISITLOG_LIST(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
		CONT_HOMEVISITLOG kTemp;
		bool const bResult = PU::TLoadArray_M(rkPacket, kTemp);
		if( bResult )
		{
			std::vector<SHOMEVISITLOG> kTempVec;
			CONT_HOMEVISITLOG::iterator itr = kTemp.begin();
			for( ; itr != kTemp.end(); ++itr)
			{
				kTempVec.push_back(*itr);
			}
			std::sort( kTempVec.begin(), kTempVec.end(), MyhomeVisitLogListSort );

			m_kMyhomeVisitLog.clear();
			std::vector<SHOMEVISITLOG>::iterator itrV = kTempVec.begin();
			for( ; itrV != kTempVec.end(); ++itrV)
			{
				m_kMyhomeVisitLog.push_back(*itrV);
			}

			m_kMyhomeVisitLogPage.SetMaxItem(m_kMyhomeVisitLog.size());
			lwHomeRenew::OpenMyhomeVisitListWnd();
		}
		else
		{
			m_kMyhomeVisitLog.clear();
		}
	}
}

void PgHomeRenew::Recv_PT_M_C_ANS_HOME_VISITLOG_DELETE(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201701, true);
		// 리스트 재정리가 필요하다.
		// 등록 부분도 리스트 재정리가 필요
		lwHomeRenew::OnMyhomeVisitListCall();
	}
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_BUY(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( E_MYHOME_NOT_ENOUGH_MONEY == hRet )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201716, true);
	}
	else if( E_MYHOME_INVALID_LEVEL_LIMIT == hRet )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201718, true);
	}
	else if( lwHomeUI::CheckErrorMsg(hRet) )
	{
		//lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201710, true);
		XUIMgr.Call(L"SFRM_MYHOME_WELCOME");
	}
	XUIMgr.Close(L"SFRM_MYHOME_BUY");
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_EXIT(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	return;
}
void PgHomeRenew::Recv_PT_M_C_ANS_EXCHANGE_LOGCOUNTTOEXP(BM::Stream& rkPacket)
{
	__int64 i64Exp = 0;
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	rkPacket.Pop(i64Exp);// __INT64 : 획득된 경험치
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
		BM::vstring strText(TTW(201704));
		strText.Replace( L"#MARK1#", i64Exp);
		lua_tinker::call<void, char const*, bool, int >("CommonMsgBox", MB(strText), true, 0);
	}
	XUIMgr.Close(L"SFRM_MYHOME_USE_VISIT_COUNT");
}

void PgHomeRenew::Recv_PT_M_C_ANS_HOME_VISITLOG_ADD(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201705, true);
		XUIMgr.Close(L"SFRM_MYHOME_WRITE_VISIT_COMMENT");
		m_dwVisitlogAddTime = BM::GetTime32();
	}
	return;
}

void PgHomeRenew::SetMyhomeEnterPageMaxItem(int iCount)
{
	m_kMyhomEnterPage.SetMaxItem(iCount);
}

void PgHomeRenew::SetMyhomeChatRoomPageMaxItem(int iCount)
{
	m_kMyhomeChatRoomPage.SetMaxItem(iCount);
}

void PgHomeRenew::SetMyhomeEnterList(CONT_MYHOME const& kHomeList)
{
	m_kMyhomeEnter.clear();
	m_kMyhomeEnter.insert(kHomeList.begin(), kHomeList.end());
}

void PgHomeRenew::GetMyhomeEnterDisplayItem(CONT_MYHOME& kHomeList)
{
	CONT_MYHOME::const_iterator home_itor = m_kMyhomeEnter.begin();
	int const ignoreCount = m_kMyhomEnterPage.Now() * m_kMyhomEnterPage.GetMaxItemSlot();
	int iLoopCount = 0;
	while( home_itor != m_kMyhomeEnter.end() )
	{
		++iLoopCount;
		if( ignoreCount >= iLoopCount )
		{
			++home_itor;
			continue;
		}

		kHomeList.insert(std::make_pair(home_itor->first, home_itor->second));
		++home_itor;

		if( (iLoopCount - ignoreCount) == m_kMyhomEnterPage.GetMaxItemSlot() )
		{
			break;
		}
	}
}

void PgHomeRenew::GetMyhomeChatRoomDisplayItem(CONT_MYHOMECHATROOM& kChatRoom)
{
	CONT_MYHOMECHATROOM::const_iterator home_itor = m_kMyhomeChatRoom.begin();
	int const ignoreCount = m_kMyhomeChatRoomPage.Now() * m_kMyhomeChatRoomPage.GetMaxItemSlot();
	int iLoopCount = 0;
	while( home_itor != m_kMyhomeChatRoom.end() )
	{
		++iLoopCount;
		if( ignoreCount >= iLoopCount )
		{
			++home_itor;
			continue;
		}

		kChatRoom.insert(std::make_pair(home_itor->first, home_itor->second));
		++home_itor;

		if( (iLoopCount - ignoreCount) == m_kMyhomeChatRoomPage.GetMaxItemSlot() )
		{
			break;
		}
	}
}

SHomeChatRoom* PgHomeRenew::FindMyhomeChatRoom(BM::GUID const& kRoomGuid)
{
	SHomeChatRoom* kRoom = GetPublicChstRoomInfo();
	if( kRoom && kRoomGuid == kRoom->kRoomGuid )
	{
		return kRoom;
	}

	CONT_MYHOMECHATROOM::iterator itr = m_kMyhomeChatRoom.find(kRoomGuid);
	if( itr != m_kMyhomeChatRoom.end() )
	{
		return (&itr->second);
	}

	return NULL;
}

SHomeChatRoom* PgHomeRenew::FindMyhomeChatRoombyGuest(BM::GUID const& kGuestGuid)
{
	SHomeChatRoom* kRoom = GetPublicChstRoomInfo();
	if( kRoom  )
	{
		if( !kRoom->kContGuest.empty() )
		{
			CONT_MYHOMECHATGUEST::iterator itr = kRoom->kContGuest.begin();
			for( ; itr != kRoom->kContGuest.end(); ++itr )
			{
				if( kGuestGuid == itr->second.kID )
				{
					return kRoom;
				}
			}
		}
	}

	CONT_MYHOMECHATROOM::iterator itr = m_kMyhomeChatRoom.begin();
	for( ; itr != m_kMyhomeChatRoom.end(); ++itr )
	{
		CONT_MYHOMECHATGUEST::iterator itr_guest = itr->second.kContGuest.begin();
		for( ; itr_guest != itr->second.kContGuest.end(); ++itr_guest )
		{
			if( itr_guest->second.kID == kGuestGuid )
			{
				return &itr->second;
			}
		}
	}

	return NULL;
}

SHomeChatGuest* PgHomeRenew::FindMyhomeChatRoomMaster(SHomeChatRoom& kChatRoom)
{
	CONT_MYHOMECHATGUEST::iterator itr = kChatRoom.kContGuest.begin();
	for( ; itr != kChatRoom.kContGuest.end(); ++itr )
	{
		if( itr->second.bRoomMaster )
		{
			return (&itr->second);
		}
	}

	return NULL;
}

SHomeChatGuest* PgHomeRenew::FindMyhomeChatRoomGuest(SHomeChatRoom& kChatRoom, BM::GUID const& kGuid)
{
	CONT_MYHOMECHATGUEST::iterator itr = kChatRoom.kContGuest.find(kGuid);
	if( itr != kChatRoom.kContGuest.end() )
	{
		return (&itr->second);
	}

	return NULL;
}

SHomeChatGuest* PgHomeRenew::FindMyhomeChatRoomGuest(BM::GUID const& kGuid)
{
	SHomeChatGuest* pRtn = NULL;
	SHomeChatRoom* kRoom = GetPublicChstRoomInfo();
	if( kRoom )
	{
		if( pRtn = FindMyhomeChatRoomGuest(*kRoom, kGuid) )
		{
			return pRtn;
		}
	}

	CONT_MYHOMECHATROOM::iterator itr = m_kMyhomeChatRoom.begin();
	for( ; itr != m_kMyhomeChatRoom.end(); ++itr )
	{
		if( pRtn = FindMyhomeChatRoomGuest(itr->second, kGuid) )
		{
			break;
		}
	}

	return pRtn;
}

bool PgHomeRenew::ModifyMyhomeChatRoomGuest(BM::GUID const& kGuid, SHomeChatGuest const& _kGuest)
{
	
	SHomeChatRoom* kRoom = GetPublicChstRoomInfo();
	if( kRoom && kGuid == kRoom->kRoomGuid )
	{
		CONT_MYHOMECHATGUEST::iterator itr_guest = kRoom->kContGuest.begin();
		for( ; itr_guest != kRoom->kContGuest.end(); ++itr_guest )
		{
			SHomeChatGuest& kChatGuest = itr_guest->second;
			if( kChatGuest.kID == _kGuest.kID)
			{
				kChatGuest = _kGuest;
				return true;
			}
		}
		lwHomeRenew::SetMyhomeChatMemberList(*kRoom);
	}
	else
	{
		CONT_MYHOMECHATROOM::iterator itr = m_kMyhomeChatRoom.find(kGuid);
		if( itr == m_kMyhomeChatRoom.end() )
		{
			return false;
		}

		CONT_MYHOMECHATGUEST& kConGuest = itr->second.kContGuest;
		CONT_MYHOMECHATGUEST::iterator itr_guest = kConGuest.begin();
		for( ; itr_guest != kConGuest.end(); ++itr_guest )
		{
			SHomeChatGuest& kGuest = itr_guest->second;
			if( kGuest.kID == _kGuest.kID)
			{
				kGuest = _kGuest;
				return true;
			}
		}
	}
	

	return false;
}

bool PgHomeRenew::AddMyhomeChatRoomGuest(BM::GUID const& kRoomGuid, SHomeChatGuest const& kGuest)
{
	SHomeChatRoom* kRoom = GetPublicChstRoomInfo();
	if( kRoom && kRoomGuid == kRoom->kRoomGuid )
	{
		kRoom->kContGuest.insert(std::make_pair(kGuest.kID, kGuest));
	}
	else
	{
		CONT_MYHOMECHATROOM::iterator itr = m_kMyhomeChatRoom.find(kRoomGuid);
		if( itr == m_kMyhomeChatRoom.end() )
		{
			return false;
		}

		itr->second.kContGuest.insert(std::make_pair(kGuest.kID, kGuest));
	}
	return true;
}

bool PgHomeRenew::DeleteMyhomeChatRoomGuest(BM::GUID const& kRoomGuid, BM::GUID const& kGuestID)
{
	SHomeChatRoom* kRoom = GetPublicChstRoomInfo();
	if( kRoom && kRoomGuid == kRoom->kRoomGuid )
	{
		kRoom->kContGuest.erase(kGuestID);
	}
	else
	{
		CONT_MYHOMECHATROOM::iterator itr = m_kMyhomeChatRoom.find(kRoomGuid);
		if( itr == m_kMyhomeChatRoom.end() )
		{
			return false;
		}
		
		itr->second.kContGuest.erase(kGuestID);
	}

	return true;
}

void PgHomeRenew::AddHomeChatLog(SChatLog& kChatLog)
{
	m_kHomeChatLog.push_back(kChatLog);
}

PgMyHome* PgHomeRenew::GetCurrentHomeUnit()
{
	if( g_pkWorld )
	{
		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			return pkHomeUnit;
		}
	}

	return NULL;
}

void PgHomeRenew::GetMyhomeVisitLogDisplayItem(CONT_HOMEVISITLOG& kList)
{
	CONT_HOMEVISITLOG::const_iterator itr = m_kMyhomeVisitLog.begin();
	int const ignoreCount = m_kMyhomeVisitLogPage.Now() * m_kMyhomeVisitLogPage.GetMaxItemSlot();
	int iLoopCount = 0;
	while( itr != m_kMyhomeVisitLog.end() )
	{
		++iLoopCount;
		if( ignoreCount >= iLoopCount )
		{
			++itr;
			continue;
		}

		kList.push_back(*itr);
		++itr;

		if( (iLoopCount - ignoreCount) == m_kMyhomeVisitLogPage.GetMaxItemSlot() )
		{
			break;
		}
	}
}

bool PgHomeRenew::IsMyhomeApt(int iBuildingNum)
{
	CONT_DEFMYHOMEBUILDINGS const * pkCont = NULL;
	g_kTblDataMgr.GetContDef(pkCont);
	if(pkCont)
	{
		CONT_DEFMYHOMEBUILDINGS::const_iterator itr = pkCont->find(iBuildingNum);
		if( itr != pkCont->end() )
		{
			if( 0 == itr->second.iGrade )
			{
				return true;
			}
		}
	}

	return false;
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_SELL()
{
	BM::vstring strText(TTW(201733));
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_SELL);
	CallCommonMsgYesNoBox(strText, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
	//NETWORK_SEND(kPacket);
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_SELL(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201734, true);
	}
}

void PgHomeRenew::SetMyhomeChatRoomList(CONT_MYHOMECHATROOM const& kChatRoomList)
{
	m_kMyhomeChatRoom.clear();
	m_kMyhomeChatRoom.insert(kChatRoomList.begin(), kChatRoomList.end());
	SetMyhomeChatRoomPageMaxItem(m_kMyhomeChatRoom.size());
}

void PgHomeRenew::Test_AddMyhomeChatPopupList(std::wstring const & rkTitle)
{
	SHomeChatRoom kItem(BM::GUID::Create(), rkTitle, L"", L"");
	m_kMyhomeChatRoom.insert(std::make_pair(kItem.kRoomGuid,kItem));
	SetMyhomeChatRoomPageMaxItem(m_kMyhomeChatRoom.size());
}

void PgHomeRenew::ModifyMyhomeChatRoom(SHomeChatRoom const& kChatRoom)
{
	CONT_MYHOMECHATROOM::iterator itr = m_kMyhomeChatRoom.find(kChatRoom.kRoomGuid);
	if( itr != m_kMyhomeChatRoom.end() )
	{
		m_kMyhomeChatRoom.erase(itr);
	}

	m_kMyhomeChatRoom.insert(std::make_pair(kChatRoom.kRoomGuid, kChatRoom));
	SetMyhomeChatRoomPageMaxItem(m_kMyhomeChatRoom.size());
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_CHAT_ENTER(BM::GUID const& rkHomeUnitGuid, BM::GUID const& rkRoomGuid, std::wstring const& strPW)
{
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_ENTER);
	kPacket.Push(rkHomeUnitGuid);// 홈 유닛 GUID
	kPacket.Push(rkRoomGuid);//들어갈 룸 GUID (NULL GUID 로 날리면 공용 채널에 들어감)
	kPacket.Push(strPW);//패스워드
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_CHAT_EXIT(BM::GUID const& rkHomeUnitGuid)
{
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_EXIT);
	kPacket.Push(rkHomeUnitGuid);// 홈 유닛 GUID
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_CHAT_MODIFY_ROOM(BM::GUID const& rkHomeUnitGuid, std::wstring const& strTitle, 
		std::wstring const& strNotice, std::wstring const& strPW, int iNum)
{
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_MODIFY_ROOM);
	kPacket.Push(rkHomeUnitGuid);// 홈 유닛 GUID
	kPacket.Push(strTitle);// 방 타이틀
	kPacket.Push(strNotice);// 공지(?) 참가자가 방에 들어오면 상단에 출력될 메시지
	kPacket.Push(strPW);// 방 페스워드
	kPacket.Push(iNum);// 최대 참가자수
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_CHAT_MODIFY_GUEST(BM::GUID const& rkHomeUnitGuid, bool bEnablePrivateMsg, bool bNotStanding)
{
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_MODIFY_GUEST);
	kPacket.Push(rkHomeUnitGuid);// 홈 유닛 GUID
	kPacket.Push(bEnablePrivateMsg);// 귓말 허용 여부(false 허용, true 거부 )
	kPacket.Push(bNotStanding);// 자리 비움 여부(false 자리에 있음, true 자리 비움)
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_CHAT_ROOM_LIST(BM::GUID const& rkHomeUnitGuid)
{
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_ROOM_LIST);
	kPacket.Push(rkHomeUnitGuid);// 홈 유닛 GUID
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_CHAT_ROOM_CREATE(BM::GUID const& rkHomeUnitGuid, std::wstring const& strTitle, 
		std::wstring const& strNotice, std::wstring const& strPW, int iNum)
{
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_ROOM_CREATE);
	kPacket.Push(rkHomeUnitGuid);// 홈 유닛 GUID
	kPacket.Push(strTitle);// 방 타이틀
	kPacket.Push(strNotice);// 공지(?) 참가자가 방에 들어오면 상단에 출력될 메시지
	kPacket.Push(strPW);// 방 페스워드
	kPacket.Push(iNum);// 최대 참가자수
	NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_CHAT_SET_ROOMMASTER(BM::GUID const& kHomeUnitGuid, BM::GUID const& kTargetGuid)
{
	BM::vstring strText(TTW(201807));
	SHomeChatGuest* pkGuest = FindMyhomeChatRoomGuest(kTargetGuid);
	if( pkGuest )
	{
		strText.Replace(L"#MARK1#", pkGuest->kName);
	}
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_SET_ROOMMASTER);
	kPacket.Push(kHomeUnitGuid);// 홈 유닛 GUID
	kPacket.Push(kTargetGuid);// 대상 플레이어 GUID
	CallCommonMsgYesNoBox(strText, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
	//NETWORK_SEND(kPacket);
}

void PgHomeRenew::Send_PT_C_M_REQ_MYHOME_CHAT_KICK_GEUST(BM::GUID const& kHomeUnitGuid, BM::GUID const& kTargetGuid)
{
	BM::vstring strText(TTW(201812));
	SHomeChatGuest* pkGuest = FindMyhomeChatRoomGuest(kTargetGuid);
	if( pkGuest )
	{
		strText.Replace(L"#MARK1#", pkGuest->kName);
	}
	BM::Stream kPacket(PT_C_M_REQ_MYHOME_CHAT_KICK_GEUST);
	kPacket.Push(kHomeUnitGuid);// 홈 유닛 GUID
	kPacket.Push(kTargetGuid);// 타겟 guid
	CallCommonMsgYesNoBox(strText, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
	//NETWORK_SEND(kPacket);
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_CHAT_ENTER(BM::Stream& rkPacket)
{
	XUIMgr.Close(L"SFRM_MYHOME_CHAT_ROOM_LIST");
	XUIMgr.Close(L"SFRM_MYHOME_CHAT_POPUP");
	
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
		SHomeChatRoom kChatRoom;
		kChatRoom.ReadFromPacket(rkPacket);
		if( IsPublicChatRoom() )
		{
			SetPublicChstRoomInfo(kChatRoom);
			lwHomeRenew::OpenMyhomeChatAllChat(kChatRoom);
		}
		else
		{
			ModifyMyhomeChatRoom(kChatRoom);
			lwHomeRenew::OpenMyhomeChatRoomChat(kChatRoom);
		}
		//lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201734, true);
	}
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_CHAT_EXIT(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	//if( lwHomeUI::CheckErrorMsg(hRet) ) // 에러 메세지는 찍지 않는다.
	{
		XUIMgr.Close(L"SFRM_MYHOME_CHAT_ALL_CHAT");
		XUIMgr.Close(L"SFRM_MYHOME_CHAT_ROOM_CHAT");
	}
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_CHAT_MODIFY_ROOM(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
		SHomeChatRoom kRoom;
		kRoom.ReadFromPacket(rkPacket);
		ModifyMyhomeChatRoom(kRoom);
		lwHomeRenew::ModifyMyhomeChatRoomChat(kRoom);
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201803, true);
	}
	XUIMgr.Close(L"SFRM_MYHOME_CHAT_MAKE");
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_CHAT_MODIFY_GUEST(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
		SHomeChatGuest kGuest;
		kGuest.ReadFromPacket(rkPacket);
		BM::GUID kRoomGuid;
		if( lwHomeRenew::FindMyhomeChatRoomGuid(kRoomGuid) )
		{
			ModifyMyhomeChatRoomGuest(kRoomGuid, kGuest);
			SHomeChatRoom* kRoom = FindMyhomeChatRoom(kRoomGuid);
			if( kRoom )
			{
				lwHomeRenew::SetMyhomeChatMemberList(*kRoom);
			}
		}
		//lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201734, true);
	}
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_CHAT_ROOM_LIST(BM::Stream& rkPacket)
{
	CONT_MYHOMECHATROOM kTemp;
	kTemp.clear();
	bool const bResult = PU::TLoadTable_AM(rkPacket, kTemp);
	if( bResult )
	{
		SetMyhomeChatRoomList(kTemp);
		lwHomeRenew::OpenMyhomeChatRoomList();
	}
	else
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201784, true);
	}
}

void PgHomeRenew::Recv_PT_M_C_NOTI_MYHOME_CHAT_ROOM_ENTER(BM::Stream& rkPacket)
{
	SHomeChatGuest kGuest;
	kGuest.ReadFromPacket(rkPacket);
	BM::GUID kRoomGuid;
	if( lwHomeRenew::FindMyhomeChatRoomGuid(kRoomGuid) )
	{
		AddMyhomeChatRoomGuest(kRoomGuid, kGuest);
		SHomeChatRoom* kRoom = FindMyhomeChatRoom(kRoomGuid);
		if( kRoom )
		{
			{// 시스템 메세지
				DWORD dwColor = 0xFF4D3413;
				BM::vstring strText(TTW(201800));
				strText.Replace(L"#MARK1#", kGuest.kName);
				std::wstring strContent(strText);
				SChatLog kChatLog(CT_MYHOME_PUBLIC);
				kChatLog.kContents = strContent;
				kChatLog.dwColor = dwColor;
				lwHomeRenew::AddMyhomeChatAllChatContent(kChatLog, true);
			}
			lwHomeRenew::SetMyhomeChatMemberList(*kRoom);
		}
	}
}

void PgHomeRenew::Recv_PT_M_C_NOTI_MYHOME_CHAT_ROOM_EXIT(BM::Stream& rkPacket)
{
	BM::GUID kGuestID;
	rkPacket.Pop(kGuestID);
	BM::GUID kRoomGuid;
	if( lwHomeRenew::FindMyhomeChatRoomGuid(kRoomGuid) )
	{
		SHomeChatGuest* pkGuest = FindMyhomeChatRoomGuest(kGuestID);
		if( pkGuest )
		{// 시스템 메세지
			DWORD dwColor = 0xFF4D3413;
			BM::vstring strText(TTW(201801));
			strText.Replace(L"#MARK1#", pkGuest->kName);
			std::wstring strContent(strText);
			SChatLog kChatLog(CT_MYHOME_PUBLIC);
			kChatLog.kContents = strContent;
			kChatLog.dwColor = dwColor;
			lwHomeRenew::AddMyhomeChatAllChatContent(kChatLog, true);
		}
		DeleteMyhomeChatRoomGuest(kRoomGuid, kGuestID);
		SHomeChatRoom* kRoom = FindMyhomeChatRoom(kRoomGuid);
		if( kRoom )
		{	
			lwHomeRenew::SetMyhomeChatMemberList(*kRoom);
		}
	}
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_CHAT_ROOM_CREATE(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
		SHomeChatRoom kChatRoom;
		kChatRoom.ReadFromPacket(rkPacket);
		ModifyMyhomeChatRoom(kChatRoom);
		lwHomeRenew::OpenMyhomeChatRoomChat(kChatRoom);
	}
}

void PgHomeRenew::Recv_PT_M_C_NOTI_MYHOME_CHAT_ROOM_MODIFY(BM::Stream& rkPacket)
{
	SHomeChatRoom kChatRoom;
	kChatRoom.ReadFromPacket(rkPacket);
	ModifyMyhomeChatRoom(kChatRoom);
	lwHomeRenew::ModifyMyhomeChatRoomChat(kChatRoom);
}

void PgHomeRenew::Recv_PT_M_C_NOTI_MYHOME_CHAT_GUEST_MODIFY(BM::Stream& rkPacket)
{
	SHomeChatGuest kGuest;
	kGuest.ReadFromPacket(rkPacket);
	
	BM::GUID kRoomGuid;
	if( lwHomeRenew::FindMyhomeChatRoomGuid(kRoomGuid) )
	{
		if( g_kPilotMan.IsMyPlayer( kGuest.kID ) )
		{
			SHomeChatGuest* pkChatGuest = FindMyhomeChatRoomGuest(kGuest.kID);
			if( pkChatGuest )
			{
				if( !pkChatGuest->bRoomMaster && kGuest.bRoomMaster )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201805, true);
				}
			}
		}

		ModifyMyhomeChatRoomGuest(kRoomGuid, kGuest);
		SHomeChatRoom* kRoom = FindMyhomeChatRoom(kRoomGuid);
		if( kRoom )
		{
			lwHomeRenew::SetMyhomeChatMemberList(*kRoom);
			lwHomeRenew::SetMyhomeChatRoomChatTitleBG(*kRoom);
		}
	}
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_CHAT_SET_ROOMMASTER(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
	}
}

void PgHomeRenew::Recv_PT_M_C_NOTI_MYHOME_CHAT_RESULT(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
	}
}

void PgHomeRenew::Recv_PT_M_C_ANS_MYHOME_CHAT_KICK_GEUST(BM::Stream& rkPacket)
{
	HRESULT hRet = PRC_Fail;
	rkPacket.Pop(hRet);
	if( lwHomeUI::CheckErrorMsg(hRet) )
	{
	}
}

void PgHomeRenew::Recv_PT_M_C_NOTI_MYHOME_CHAT_KICK_GEUST(BM::Stream& rkPacket)
{
	BM::GUID kTargetGuid;
	rkPacket.Pop(kTargetGuid);
	SHomeChatRoom* pkChatRoom = FindMyhomeChatRoombyGuest(kTargetGuid);
	if( !pkChatRoom )
	{
		return;
	}
	if( g_kPilotMan.IsMyPlayer(kTargetGuid ) )
	{
		XUIMgr.Close(L"SFRM_MYHOME_CHAT_ALL_CHAT");
		XUIMgr.Close(L"SFRM_MYHOME_CHAT_ROOM_CHAT");
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201814, true);
		return;
	}
	SHomeChatGuest* pkChatMaster = FindMyhomeChatRoomMaster(*pkChatRoom);
	if( pkChatMaster )
	{
		if( g_kPilotMan.IsMyPlayer(pkChatMaster->kID) )
		{
			SHomeChatGuest* pkChatGuest = FindMyhomeChatRoomGuest(*pkChatRoom, kTargetGuid);
			if( pkChatGuest )
			{
				BM::vstring strText(TTW(201813));
				strText.Replace(L"#MARK1#",pkChatGuest->kName);
				lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(strText), true);
			}
		}
	}

	if( DeleteMyhomeChatRoomGuest(pkChatRoom->kRoomGuid, kTargetGuid) )
	{
		lwHomeRenew::SetMyhomeChatMemberList(*pkChatRoom);
	}
}


namespace lwHomeRenew
{
	void Test_AddMyhomeChatPopupList(char const * szTitle, bool bNotCall);

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "DoHomeRenew", lwHomeRenew::DoHomeRenew);
		def(pkState, "OnRecvHomeInvite", lwHomeRenew::OnRecvHomeInvite);
		def(pkState, "SetBoardNameTag", lwHomeRenew::SetBoardNameTag);
		def(pkState, "OpenMyhomeEnterWnd", lwHomeRenew::OpenMyhomeEnterWnd);
		def(pkState, "OpenMyhomeBuy", lwHomeRenew::OpenMyhomeBuy);
		def(pkState, "OnMyhomeAptRelive", lwHomeRenew::OnMyhomeAptRelive);
		def(pkState, "OnSendExchangeLogCountToExp", lwHomeRenew::OnSendExchangeLogCountToExp);
		def(pkState, "OnMyhomeVisitListCall", lwHomeRenew::OnMyhomeVisitListCall);
		def(pkState, "OnMyhomeEnterMoveMine", lwHomeRenew::OnMyhomeEnterMoveMine);
		def(pkState, "OnMyhomeEnterMoveOther", lwHomeRenew::OnMyhomeEnterMoveOther);
		def(pkState, "OnClickMyhomeEnterJumpPrevPage", lwHomeRenew::OnClickMyhomeEnterJumpPrevPage);
		def(pkState, "OnClickMyhomeEnterJumpNextPage", lwHomeRenew::OnClickMyhomeEnterJumpNextPage);
		def(pkState, "OnClickMyhomeEnterPage", lwHomeRenew::OnClickMyhomeEnterPage);
		def(pkState, "OnClickMyhomeVisitLogJumpPrevPage", lwHomeRenew::OnClickMyhomeVisitLogJumpPrevPage);
		def(pkState, "OnClickMyhomeVisitLogJumpNextPage", lwHomeRenew::OnClickMyhomeVisitLogJumpNextPage);
		def(pkState, "OnClickMyhomeVisitLogPage", lwHomeRenew::OnClickMyhomeVisitLogPage);
		def(pkState, "ReserveMyhomeVisitComment", lwHomeRenew::ReserveMyhomeVisitComment);
		def(pkState, "OnCallMyhomeVisitComment", lwHomeRenew::OnCallMyhomeVisitComment);
		def(pkState, "OnMyhomeVisitListDelete", lwHomeRenew::OnMyhomeVisitListDelete);
		def(pkState, "OnMyhomeVisitListAddFrient", lwHomeRenew::OnMyhomeVisitListAddFrient);
		def(pkState, "OnMyhomeVisitListUseCountBtn", lwHomeRenew::OnMyhomeVisitListUseCountBtn);
		def(pkState, "SendMyhomeVisitComment", lwHomeRenew::SendMyhomeVisitComment);
		def(pkState, "SendMyhomeBuy", lwHomeRenew::SendMyhomeBuy);
		def(pkState, "OnSendMyhomeEnter", lwHomeRenew::OnSendMyhomeEnter);
		def(pkState, "SendMyhomeExit", lwHomeRenew::SendMyhomeExit);
		def(pkState, "OnClickMyHomesSocket", lwHomeRenew::OnClickMyHomesSocket);
		def(pkState, "IsMyhomeApt", lwHomeRenew::IsMyhomeApt);
		def(pkState, "SendMyhomeInvitebyFriend", lwHomeRenew::SendMyhomeInvitebyFriend);
		def(pkState, "SendMyhomeInvitebyGuild", lwHomeRenew::SendMyhomeInvitebyGuild);
		def(pkState, "SendMyhomeInvitebyParty", lwHomeRenew::SendMyhomeInvitebyParty);
		def(pkState, "SendMyhomeInvitebyCouple", lwHomeRenew::SendMyhomeInvitebyCouple);
		def(pkState, "EnterMyhomeApt", lwHomeRenew::EnterMyhomeApt);
		def(pkState, "OnMyhomeBuffDisplay", lwHomeRenew::OnMyhomeBuffDisplay);
		def(pkState, "SendMyhomeGobyFriend", lwHomeRenew::SendMyhomeGobyFriend);
		def(pkState, "SendMyhomeGobyGuild", lwHomeRenew::SendMyhomeGobyGuild);
		def(pkState, "SendMyhomeGobyParty", lwHomeRenew::SendMyhomeGobyParty);
		def(pkState, "SendMyhomeGobyCouple", lwHomeRenew::SendMyhomeGobyCouple);
		def(pkState, "OnCallMyhomeDetailView", lwHomeRenew::OnCallMyhomeDetailView);
		def(pkState, "OnSendBuild_HomeList", lwHomeRenew::OnSendBuild_HomeList);
		def(pkState, "OnClickMinimapMyhomePopup", lwHomeRenew::OnClickMinimapMyhomePopup);
		def(pkState, "OnClickMinimapMyhomePopupItem", lwHomeRenew::OnClickMinimapMyhomePopupItem);
		def(pkState, "IsHaveMyhomebyFriend", lwHomeRenew::IsHaveMyhomebyFriend);
		def(pkState, "OnCallMyhomeInviteLetter", lwHomeRenew::OnCallMyhomeInviteLetter);
		def(pkState, "SendMyhomeAptRelive_Packet", lwHomeRenew::SendMyhomeAptRelive_Packet);
		def(pkState, "SetBuildNameBoard", lwHomeRenew::SetBuildNameBoard);
		def(pkState, "SetBuildNameBoardforOptionChange", lwHomeRenew::SetBuildNameBoardforOptionChange);
		def(pkState, "OnCallMyhomeSellCommonMessageBox", lwHomeRenew::OnCallMyhomeSellCommonMessageBox);
		
		def(pkState, "IsInsideHome", lwHomeRenew::IsInsideHome);
		def(pkState, "MyhomeChatPopupEnter", lwHomeRenew::MyhomeChatPopupEnter);
		def(pkState, "IsMyRoomMasterChatRoom", lwHomeRenew::IsMyRoomMasterChatRoom);
		def(pkState, "OpenMyhomeChatPopup", lwHomeRenew::OpenMyhomeChatPopup);
		def(pkState, "MyhomeChatPopupList", lwHomeRenew::MyhomeChatPopupList);
		def(pkState, "MyhomeChatMakeMemberDropDown", lwHomeRenew::MyhomeChatMakeMemberDropDown);
		def(pkState, "MyhomeChatMakeDropDownItem", lwHomeRenew::MyhomeChatMakeDropDownItem);
		def(pkState, "MyhomeChatMakePublicOpen", lwHomeRenew::MyhomeChatMakePublicOpen);
		def(pkState, "MyhomeChatMakeNotice", lwHomeRenew::MyhomeChatMakeNotice);
		def(pkState, "MyhomeChatMakeOK", lwHomeRenew::MyhomeChatMakeOK);
		def(pkState, "MyhomeChatRoomListEnter", lwHomeRenew::MyhomeChatRoomListEnter);
		def(pkState, "MyhomeChatRoomListMsgBoxOK", lwHomeRenew::MyhomeChatRoomListMsgBoxOK);
		def(pkState, "MyhomeChatRoomListPageFirst", lwHomeRenew::MyhomeChatRoomListPageFirst);
		def(pkState, "MyhomeChatRoomListPageJumpPrev", lwHomeRenew::MyhomeChatRoomListPageJumpPrev);
		def(pkState, "MyhomeChatRoomListPagePrev", lwHomeRenew::MyhomeChatRoomListPagePrev);
		def(pkState, "MyhomeChatRoomListPageNext", lwHomeRenew::MyhomeChatRoomListPageNext);
		def(pkState, "MyhomeChatRoomListPageJumpNext", lwHomeRenew::MyhomeChatRoomListPageJumpNext);
		def(pkState, "MyhomeChatRoomListPageEnd", lwHomeRenew::MyhomeChatRoomListPageEnd);
		def(pkState, "MyhomeChatRoomListPageNum", lwHomeRenew::MyhomeChatRoomListPageNum);
		def(pkState, "MyhomeChatRoomListMake", lwHomeRenew::MyhomeChatRoomListMake);
		def(pkState, "MyhomeChatRoomChatChange", lwHomeRenew::MyhomeChatRoomChatChange);
		def(pkState, "MyhomeChatEnablePrivateMsg", lwHomeRenew::MyhomeChatEnablePrivateMsg);
		def(pkState, "MyhomeChatSend", lwHomeRenew::MyhomeChatSend);
		def(pkState, "MyhomeChatNotStanding", lwHomeRenew::MyhomeChatNotStanding);
		def(pkState, "MyhomeChatClean", lwHomeRenew::MyhomeChatClean);
		def(pkState, "MyhomeChatExit", lwHomeRenew::MyhomeChatExit);
		def(pkState, "SetMyhomeChatFontColor", lwHomeRenew::SetMyhomeChatFontColor);
		def(pkState, "GetMyhomeChatFontColor", lwHomeRenew::GetMyhomeChatFontColor);
		def(pkState, "PopupMyhomeChatWhisper", lwHomeRenew::PopupMyhomeChatWhisper);
		def(pkState, "PopupMyhomeChatAddFriend", lwHomeRenew::PopupMyhomeChatAddFriend);
		def(pkState, "PopupMyhomeChatInvitation", lwHomeRenew::PopupMyhomeChatInvitation);
		def(pkState, "PopupMyhomeChatRoomMaster", lwHomeRenew::PopupMyhomeChatRoomMaster);
		def(pkState, "PopupMyhomeChatRoomListWhisper", lwHomeRenew::PopupMyhomeChatRoomListWhisper);
		def(pkState, "PopupMyhomeChatRoomListAddFriend", lwHomeRenew::PopupMyhomeChatRoomListAddFriend);
		def(pkState, "PopupMyhomeChatKickGuest", lwHomeRenew::PopupMyhomeChatKickGuest);
		def(pkState, "CallMyhomeEmoticonUI", lwHomeRenew::CallMyhomeEmoticonUI);
		def(pkState, "MyhomeSelectEmotiItem", lwHomeRenew::MyhomeSelectEmotiItem);

		//Test
		def(pkState, "Test_AddMyhomeChatPopupList", lwHomeRenew::Test_AddMyhomeChatPopupList);
	}

	int const MAX_EMOTI_LINE_ITEM_SLOT = 8;
	int const MAX_EMOTI_ITEM_LINE = 3;
	int const MAX_EMOTI_ITEM_SLOT = MAX_EMOTI_LINE_ITEM_SLOT * MAX_EMOTI_ITEM_LINE;

	void OnMyhomeEnterMoveOther(lwUIWnd kWnd)// 남의 집 들어갈때
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}

		XUI::CXUI_Wnd* pkBG3 = pkWnd->Parent();
		if( !pkBG3 )
		{
			return;
		}

		XUI::CXUI_Wnd* pkSlot = pkBG3->Parent();
		if( !pkSlot )
		{
			return;
		}

		SHOMEADDR kPoint;
		if( pkSlot->GetCustomData(&kPoint, sizeof(kPoint)) )
		{
			SendEnterHome(kPoint.StreetNo(), kPoint.HouseNo());
		}

	}

	void OnMyhomeEnterMoveMine(lwUIWnd kWnd)// 자신의 집 들어갈때
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( pkWnd )
		{
			lwUIWnd kParent = kWnd.GetTopParent();
			XUI::CXUI_Wnd* pkParent = kParent.GetSelf();
			if( pkParent )
			{
				SHOMEADDR kPoint;
				if( pkParent->GetCustomData(&kPoint, sizeof(kPoint)) )
				{
					SendEnterHome(kPoint.StreetNo(), kPoint.HouseNo());
				}
			}
		}
	}

	void OnMyhomeVisitListCall()// 방문자 리스트 열때
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		if( pkHomeUnit->OwnerGuid().IsNull() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201741, true);
			return;
		}

		g_kHomeRenewMgr.Send_PT_C_M_REQ_HOME_VISITLOG_LIST(pkHomeUnit->GetID());
	}

	void OnMyhomeVisitListDelete(lwUIWnd kWnd)// 방문자 리스트 로그 삭제
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( pkWnd )
		{
			BM::GUID kGuid = pkWnd->OwnerGuid();
			if( kGuid != BM::GUID::NullData() )
			{
				g_kHomeRenewMgr.Send_PT_C_M_REQ_HOME_VISITLOG_DELETE( pkHomeUnit->GetID(), kGuid );
			}
		}
	}

	void OnMyhomeVisitListAddFrient(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( pkWnd )
		{
			BM::GUID kGuid = pkWnd->OwnerGuid();
			if( kGuid != BM::GUID::NullData() )
			{// 친구 추가
				CONT_HOMEVISITLOG kDisplay;
				g_kHomeRenewMgr.GetMyhomeVisitLogDisplayItem(kDisplay);
				CONT_HOMEVISITLOG::const_iterator itr = kDisplay.begin();
				while( itr != kDisplay.end() )
				{
					if( itr->kLogGuid == kGuid )
					{
						SFriendItem kFriendItem;
						kFriendItem.CharGuid(itr->kVisitorGuid);
						g_kFriendMgr.SendFriend_Command(FCT_ADD_BYGUID, kFriendItem);
						break;
					}
					++itr;
				}
			}
		}
	}

	void OnMyhomeVisitListUseCountBtn()
	{
		if(!g_pkWorld)
		{
			return;
		}
		int iCount = 0;
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		if( !pkHome->IsMyHome() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201725, true);
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		iCount = pkHomeUnit->GetAbil(AT_MYHOME_VISITLOGCOUNT);
		if( 0 == iCount )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201709, true);
			return;
		}
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"SFRM_MYHOME_USE_VISIT_COUNT");
		if( pkWnd )
		{// 카운트 수와 경험치를 찍어준다.
			XUI::CXUI_Wnd* pkShd = pkWnd->GetControl(L"SFRM_BODY_SHD");
			if( pkShd )
			{
				XUI::CXUI_Wnd* pkCount = pkShd->GetControl(L"SFRM_VISTOR_COUNT");
				if( pkCount )// 카운트
				{
					BM::vstring strText(iCount);
					pkCount->Text(strText);
				}
				XUI::CXUI_Wnd* pkExp = pkShd->GetControl(L"IMG_EXP_BG");
				if( pkExp )// 경험치
				{
					__int64 const i64Exp = int(std::pow(iCount,1.02)) * 100;
					BM::vstring strText(i64Exp);
					pkExp->Text(strText);
				}

				XUIMgr.Close( L"SFRM_MYHOME_VISIT_LIST" );
			}
		}
	}

	void SendMyhomeVisitComment(lwUIWnd kWnd)
	{
		if(!g_pkWorld)
		{
			return;
		}
		// 1분 검사
		if( g_kHomeRenewMgr.GetVisitlogAddTime() + (60 * 1000) > BM::GetTime32() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201703, true);
			return;
		}
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( pkWnd )
		{
			std::wstring strText;
			XUI::CXUI_Wnd* pkShd = pkWnd->GetControl(L"SFRM_BODY_SHD");
			if( pkShd )
			{
				XUI::CXUI_Wnd* pkComment = pkShd->GetControl(L"SFRM_COMMENT_WRITE");
				if( pkComment )
				{
					XUI::CXUI_Edit_MultiLine* pkEdit = dynamic_cast<XUI::CXUI_Edit_MultiLine*>(pkComment->GetControl(L"MEDT_COMMENT_WRITE"));
					if( pkEdit )
					{
						strText = pkEdit->XUI::CXUI_Edit::EditText();
						if( strText.empty() )// 아무것도 안썼으면
						{
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201706, true);
							return;
						}

						if ( g_kClientFS.Filter( strText, false, FST_ALL ) )
						{
							pkEdit->EditText(L"");
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50513, true);
							return;
						}
					}
				}

				PgHome* pkHome = g_pkWorld->GetHome();
				if( !pkHome )
				{
					return;
				}
				PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
				if( !pkHomeUnit )
				{
					return;
				}

				int iState = pkHomeUnit->GetAbil(AT_MYHOME_STATE);
				if( MAS_IS_BLOCK == iState )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201707, true);
					return;
				}
				else if( MAS_IS_BIDDING == iState )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201708, true);
					return;
				}

				XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LIST_MYHOME_BUFF_LIST"));
				if(!pkListWnd)
				{
					return;
				}
				int iUseCnt = 0;
				CONT_ITEMPOS kItemPos;
				XUI::SListItem* pkListItem = pkListWnd->FirstItem();
				while(pkListItem)
				{
					XUI::CXUI_Wnd* pWnd = pkListItem->m_pWnd;
					XUI::CXUI_CheckButton* pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pWnd->GetControl(L"CBTN_CHECK"));
					if(true == pkChkBtn->Check())
					{// 아이템 버프를 사용 요청을 보내고, 체크된 버튼은 해제 시킴
						pkChkBtn->Check(false);
						SItemPos kPos;
						pWnd->GetCustomData(&kPos, sizeof(kPos));
						kItemPos.insert(kItemPos.end(), kPos);
						++iUseCnt;
					}
					pkListItem = pkListWnd->NextItem(pkListItem);
				}
				
				bool bPrivate = false;
				XUI::CXUI_CheckButton* pkPrivate = dynamic_cast<XUI::CXUI_CheckButton*>(pkShd->GetControl(L"BTN_PRIVATE"));
				if( pkPrivate )
				{
					bPrivate = pkPrivate->Check();
				}

				if( iUseCnt )
				{
					lua_tinker::call<void, int>("MyHome_DecoBuf_Particle", iUseCnt);// 사용한 버프가 있다면 연출을 사용
				}
				g_kHomeRenewMgr.Send_PT_C_M_REQ_HOME_VISITLOG_ADD(pkHomeUnit->GetID(), strText, kItemPos, bPrivate);
			}
		}
	}

	void OpenMyhomeVisitListWnd()
	{
		if(!g_pkWorld)
		{
			return;
		}
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"SFRM_MYHOME_VISIT_LIST");
		if( pkWnd )
		{
			// 카운트 표시
			XUI::CXUI_Wnd* pkLine = pkWnd->GetControl(L"SFRM_LINE_BOX");
			if( pkLine )
			{
				XUI::CXUI_Wnd* pkAuBox = pkLine->GetControl(L"SFRM_AU_BOX");
				if( pkAuBox )
				{
					int iCount = 0;
					PgHome* pkHome = g_pkWorld->GetHome();
					if( pkHome )
					{
						PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
						if( pkHomeUnit )
						{
							iCount = pkHomeUnit->GetAbil(AT_MYHOME_VISITLOGCOUNT);
						}
					}
					BM::vstring strText(iCount);
					pkAuBox->Text(strText);
				}
			}

			XUI::CXUI_Wnd* pMainUI = pkWnd->GetControl(L"SFRM_BODY_SHD");
			if( pMainUI )
			{
				SetMyhomeVisitLogSlotList(pMainUI);
				SetMyhomeVisitLogPageControl(pMainUI);
			}
		}
	}

	void OnSendExchangeLogCountToExp()// 방문자 카운트 경험치 받을때
	{
		g_kHomeRenewMgr.Send_PT_C_M_REQ_EXCHANGE_LOGCOUNTTOEXP();
	}

	std::wstring const kMyHomeBuyUIName(L"SFRM_MYHOME_BUY");
	void OpenMyhomeBuy()
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get( kMyHomeBuyUIName );
		if( NULL == pkWnd )
		{
			bool const bReqPrice = true;
			g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_BUY(bReqPrice, 0);
		}
	}

	void AnsOpenMyhomeBuy(int const iGold)
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(kMyHomeBuyUIName);
		if( NULL == pkWnd )
		{
			pkWnd = XUIMgr.Call(kMyHomeBuyUIName);
		}
		if( pkWnd )
		{
			XUI::CXUI_Wnd* pkGold = pkWnd->GetControl(L"SFRM_GOLD_BG");
			if( pkGold )
			{
				int const iGoldValue = 10000;
				BM::vstring kText(iGold / iGoldValue);
				pkGold->Text(kText);
			}
		}
	}

	void SendMyhomeBuy(lwUIWnd kWnd)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer){return;}

		//if(pkPlayer->GetAbil(AT_LEVEL) < 25)	//레벨 부족
		//{
		//	kWnd.Close();
		//	lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201718, true);
		//	return;
		//}

		//if(pkPlayer->GetAbil64(AT_MONEY) < 1000000)	//소지금 부족
		//{
		//	kWnd.Close();
		//	lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201716, true);
		//	return;
		//}

		short sNum = -1;
		const CONT_DEFGROUNDBUILDINGS* pCont = NULL;
		g_kTblDataMgr.GetContDef(pCont);
		if(pCont && g_pkWorld)
		{
			//CONT_DEFGROUNDBUILDINGS::const_iterator itr = pCont->find(g_pkWorld->MapNo());
			CONT_DEFGROUNDBUILDINGS::const_iterator itr = pCont->find(BUILDING_MAP_NUMBER);
			if( itr != pCont->end())
			{
				CONT_SET_BUILDINGS kSetBuilding = (*itr).second.kCont;
				CONT_SET_BUILDINGS::iterator it_bd = kSetBuilding.begin();
				while( it_bd != kSetBuilding.end() )
				{
					if( it_bd->iGrade == 0 )
					{
						sNum = it_bd->iBuildingNo;
						break;
					}
					++it_bd;
				}
			}
		}
		
		if( sNum > -1 )
		{
			bool const bReqPrice = false;
			g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_BUY(bReqPrice, sNum);
		}
	}

	void OnMyhomeAptRelive(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( pkWnd )
		{
			XUI::CXUI_Wnd* pkGold1 = pkWnd->GetControl(L"SFRM_GOLD_BG1");
			XUI::CXUI_Wnd* pkGold2 = pkWnd->GetControl(L"SFRM_GOLD_BG2");
			XUI::CXUI_Wnd* pkGold3 = pkWnd->GetControl(L"SFRM_GOLD_BG3");
			if( pkGold1 )
			{
				BM::vstring kText(100);
				pkGold1->Text(kText);
			}
			if( pkGold2 )
			{
				BM::vstring kText(100);
				pkGold2->Text(kText);
			}
			if( pkGold2 )
			{
				BM::vstring kText(100);
				pkGold3->Text(kText);
			}
		}
	}

	bool ReserveMyhomeAptRelive()
	{// 세금 납부창 호출 이벤트 예약
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer && !pkPlayer->HomeAddr().IsNull() )
		{
			lwHomeUI::OnSendHomeInfoRequest(pkPlayer->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_APT_RELIVE);
		}
		else
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201055, true);
			return false;
		}
		return true;
	}

	bool OnCallMyhomeAptRelive(SMYHOME const& kHome)
	{//세금 납부 UI에 값 세팅 부분
		//플레이어의 홈 정보를 얻어와
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call( L"SFRM_MYHOME_APT_RELIVE" );
		if(pkWnd)
		{
			// 세금 납부 시간을 초로 변환
			__int64 i64Temp =0;
			CGameTime::SystemTime2SecTime(kHome.kPayTexTime, i64Temp);
			__int64 const& i64TexPayTimeSec = i64Temp;

			// 현재 시간을 초로 변환해서
			SYSTEMTIME kNowTime;
			g_kEventView.GetLocalTime(&kNowTime);
			__int64 i64NowTimeSec = 0;
			CGameTime::SystemTime2SecTime(kNowTime, i64NowTimeSec);
			// 연체 여부를 확인
			__int64 const i64DistTimeSec = i64TexPayTimeSec - i64NowTimeSec;

			XUI::CXUI_Wnd* pkShd = pkWnd->GetControl( L"SFRM_BODY_SHD" );
			__int64 i64DelayWeek = 1;
			if( pkShd && 0 > i64DistTimeSec)
			{// 세금 값 세팅을 세팅하고(연체된 값 만큼)
				__int64 i64TaxValue = MYHOMEUTIL::CalcTex(kHome);
				pkWnd->SetCustomData(&i64TaxValue, sizeof(i64TaxValue));
				__int64 i64Gold = GetMoneyMU( EMoney_Gold, i64TaxValue );
				__int64 i64Silver = GetMoneyMU( EMoney_Silver, i64TaxValue );
				__int64 i64Copper = GetMoneyMU( EMoney_Copper, i64TaxValue );
				
				__int64 i64DelayMoney = kHome.i64HomePrice/2;
				__int64 i64DelayGold = GetMoneyMU( EMoney_Gold, i64DelayMoney );
				__int64 i64DelaySilver = GetMoneyMU( EMoney_Silver, i64DelayMoney );
				__int64 i64DelayCopper = GetMoneyMU( EMoney_Copper, i64DelayMoney );
				
				XUI::CXUI_Wnd* pkMoney = pkShd->GetControl(L"SFRM_GOLD_BG1");
				if(pkMoney)
				{
					XUI::CXUI_Wnd* pkTemp = pkMoney->GetControl(L"FRM_GOLD_NUM");
					if( pkTemp )
					{
						pkTemp->Text(BM::vstring(i64Gold-i64DelayGold));
					}
					pkTemp = pkMoney->GetControl(L"FRM_SILVER_NUM");
					if( pkTemp )
					{
						pkTemp->Text(BM::vstring(i64Silver-i64DelaySilver));
					}
					pkTemp = pkMoney->GetControl(L"FRM_BRONZE_NUM");
					if( pkTemp )
					{
						pkTemp->Text(BM::vstring(i64Copper-i64DelayCopper));
					}
				}
				pkMoney = pkShd->GetControl(L"SFRM_GOLD_BG2");
				if( pkMoney )
				{
					XUI::CXUI_Wnd* pkTemp = pkMoney->GetControl(L"FRM_GOLD_NUM");
					if( pkTemp )
					{
						pkTemp->Text(BM::vstring(i64DelayGold));
					}
					pkTemp = pkMoney->GetControl(L"FRM_SILVER_NUM");
					if( pkTemp )
					{
						pkTemp->Text(BM::vstring(i64DelaySilver));
					}
					pkTemp = pkMoney->GetControl(L"FRM_BRONZE_NUM");
					if( pkTemp )
					{
						pkTemp->Text(BM::vstring(i64DelayCopper));
					}
				}
				pkMoney = pkShd->GetControl(L"SFRM_GOLD_BG3");
				if( pkMoney )
				{
					XUI::CXUI_Wnd* pkTemp = pkMoney->GetControl(L"FRM_GOLD_NUM");
					if( pkTemp )
					{
						pkTemp->Text(BM::vstring(i64Gold));
					}
					pkTemp = pkMoney->GetControl(L"FRM_SILVER_NUM");
					if( pkTemp )
					{
						pkTemp->Text(BM::vstring(i64Silver));
					}
					pkTemp = pkMoney->GetControl(L"FRM_BRONZE_NUM");
					if( pkTemp )
					{
						pkTemp->Text(BM::vstring(i64Copper));
					}
				}

				//XUI::CXUI_Wnd* pkTaxSilver = pkWnd->GetControl(_T("FRM_COIN2"));
				//if(pkTaxSilver)
				//{
				//	pkTaxSilver->Text(BM::vstring(i64Silver));
				//}
				//XUI::CXUI_Wnd* pkTaxCopper = pkWnd->GetControl(_T("FRM_COIN3"));
				//if(pkTaxCopper)
				//{
				//	pkTaxCopper->Text(BM::vstring(i64Copper));
				//}
			}
		}
		return true;
	}

	bool SendMyhomeAptRelive_Packet()
	{// 세금 납부
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return false;
		}
		if(pkPlayer->HomeAddr().IsNull())
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201055, true);
			return false;
		}

		// UI를 호출하면서 저장한 세금 값을 얻어와
		__int64 i64TaxValue = 0;
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_MYHOME_APT_RELIVE");
		if(!pkWnd)
		{
			return false;
		}
		pkWnd->GetCustomData(&i64TaxValue, sizeof(i64TaxValue));
		__int64 const& i64MyMoney = pkPlayer->GetInven()->Money();
		if(i64MyMoney < i64TaxValue)
		{// 소지금과 비교하여
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 405115, true);
			return false;
		}
		// 패킷을 보낸다
		BM::Stream kPacket(PT_C_M_REQ_MYHOME_PAY_TEX);
		NETWORK_SEND(kPacket);
		return true;
	}

	void OnSendMyhomeEnter(int iPos)
	{
		g_kHomeRenewMgr.SetMyhomeEnterPos(iPos);
		BM::Stream kPacket(PT_C_M_REQ_HOMETOWN_INFO);
		kPacket.Push((BYTE)PgHomeUIUtil::ETLRT_OPEN_MYHOME_ENTER);
		NETWORK_SEND(kPacket);
	}

	void SetMyhomeAddrText(XUI::CXUI_Wnd* pkWnd, int iNum)
	{
		if( pkWnd )
		{
			XUI::CXUI_Wnd* pkLineBox = pkWnd->GetControl(L"SFRM_LINE_BOX");
			if( pkLineBox )
			{
				XUI::CXUI_Wnd* pkAuBox = pkLineBox->GetControl(L"SFRM_AU_BOX");
				if( pkAuBox )
				{
					XUI::CXUI_Wnd* pkTextAddr = pkAuBox->GetControl(L"FRM_TEXT_ADDR");
					if( pkTextAddr )
					{
						if( 0 == iNum )
						{
							pkTextAddr->Text(L"");
						}
						else
						{
							BM::vstring strText(TTW(201682));
							strText.Replace( L"#MARK1#", iNum);
							pkTextAddr->Text(strText);
						}
					}
				}
			}
		}
	}

	void SetMyHomeEnterPageControl(XUI::CXUI_Wnd* pPageMainUI)
	{
		if( !pPageMainUI )
		{
			return;
		}

		lwHomeUIUtil::CommonPageControl(pPageMainUI, g_kHomeRenewMgr.GetMyhomeEnterPage());
	}

	void SetMyhomeVisitLogPageControl(XUI::CXUI_Wnd* pPageMainUI)
	{
		if( !pPageMainUI )
		{
			return;
		}

		lwHomeUIUtil::CommonPageControl(pPageMainUI, g_kHomeRenewMgr.GetMyhomeVisitLogPage());
	}

	void SetMyhomeEnterSlot(XUI::CXUI_Wnd* pSlot, SMYHOME const& kHome)
	{
		if( !pSlot )
		{
			return;
		}

		if( 0 == kHome.siStreetNo && 0 == kHome.iHouseNo )
		{
			pSlot->Visible(false);
		}
		else
		{
			pSlot->Visible(true);
		}

		pSlot->OwnerGuid(kHome.kOwnerGuid);

		SHOMEADDR kPoint(kHome.siStreetNo, kHome.iHouseNo);
		pSlot->SetCustomData(&kPoint, sizeof(kPoint));

		wchar_t szTemp[MAX_PATH] = {0,};

		XUI::CXUI_Wnd* pTemp = pSlot->GetControl(L"SFRM_ITEM_BG1");
		if( pTemp )
		{
			BM::vstring strText(TTW(201682));
			strText.Replace( L"#MARK1#", kHome.iHouseNo );
			pTemp->Text(strText);
		}

		pTemp = pSlot->GetControl(L"SFRM_ITEM_BG2");
		if( pTemp )
		{
			if( kHome.bAuctionState == MAS_IS_BIDDING )
			{
				pTemp->Text(TTW(201014));
			}
			else
			{
				pTemp->Text(kHome.kName);
			}
		}

		pTemp = pSlot->GetControl(L"SFRM_ITEM_BG3");
		if( pTemp )
		{
			XUI::CXUI_Wnd* pBtn = pTemp->GetControl(L"BTN_MOVE");
			if( pBtn )
			{
				bool bIsDisable = true;
				if( kHome.bAuctionState == MAS_IS_BIDDING )
				{
					bIsDisable = false;
				}
				else if( (kHome.bEnableVisitBit & MEV_ALL) == MEV_ALL )
				{
					bIsDisable = false;
				}
				else if( g_kPilotMan.IsMyPlayer(kHome.kOwnerGuid) )
				{
					bIsDisable = false;
				}
				else if( (kHome.bEnableVisitBit & MEV_COUPLE) == MEV_COUPLE )
				{
					SCouple kMyCouple = g_kCoupleMgr.GetMyInfo();

					if( kMyCouple.CoupleGuid() != BM::GUID::NullData()
					&& kMyCouple.CoupleGuid() == kHome.kOwnerGuid )
					{
						bIsDisable = false;
					}
				}
				else if( (kHome.bEnableVisitBit & MEV_GUILD) == MEV_GUILD )
				{
					SGuildMemberInfo kTemp;
					if( g_kGuildMgr.IamHaveGuild()
					&& g_kGuildMgr.GetMemberByGuid( kHome.kOwnerGuid, kTemp ) )
					{
						bIsDisable = false;
					}
				}
				else if( (kHome.bEnableVisitBit & MEV_FRIEND) == MEV_FRIEND )
				{
					SFriendItem kTemp;
					if( g_kFriendMgr.Friend_Find_ByGuid( kHome.kOwnerGuid, kTemp ) )
					{
						bIsDisable = false;
					}
				}

				XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pBtn);
				if( pButton )
				{
					pButton->Disable(bIsDisable);
					if( bIsDisable )
					{
						pButton->Text(L"");
					}
					else
					{
						pButton->Text(TTW(2703));
					}
				}
			}
		}
	}

	void SetMyhomeVisitLogSlot(XUI::CXUI_Wnd* pSlot, SHOMEVISITLOG const& kInfo, int iNum)
	{
		if(!g_pkWorld)
		{
			return;
		}
		if( !pSlot )
		{
			return;
		}

		if( BM::GUID::NullData() == kInfo.kLogGuid )
		{
			pSlot->Visible(false);
		}
		else
		{
			pSlot->Visible(true);
		}

		pSlot->OwnerGuid(kInfo.kLogGuid);

		wchar_t szTemp[MAX_PATH] = {0,};

		XUI::CXUI_Wnd* pTemp = pSlot->GetControl(L"FRM_ARROW");
		if( pTemp )
		{
			BM::vstring strText(TTW(201675));
			strText.Replace( L"#MARK1#", iNum );
			strText.Replace( L"#MARK2#", kInfo.kVisitorName );
			pTemp->Text(strText);
		}

		pTemp = pSlot->GetControl(L"SFRM_COMMENT");
		if( pTemp )
		{
			XUI::CXUI_Wnd* pkPrivate = pTemp->GetControl(L"FRM_PRIVATE");
			if( pkPrivate )
			{
				bool bIsMine = g_kPilotMan.IsMyPlayer(kInfo.kVisitorGuid);
				bool bIsOwner = false;
				PgHome* pkHome = g_pkWorld->GetHome();
				if( pkHome )
				{
					bIsOwner = pkHome->IsMyHome();
				}

				if( bIsMine || bIsOwner || !kInfo.bPrivate )
				{
					pTemp->Text(kInfo.kVisitLog);
					pkPrivate->Visible(false);
				}
				else
				{
					pTemp->Text(L"");
					pkPrivate->Visible(true);
				}
			}
		}

		pTemp = pSlot->GetControl(L"BTN_DELETE");
		if( pTemp )
		{
			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pTemp);
			PgHome* pkHome = g_pkWorld->GetHome();
			if( pkHome && pButton )
			{
				pButton->Disable(false);
				if( !pkHome->IsMyHome() )
				{
					if( !g_kPilotMan.IsMyPlayer(kInfo.kVisitorGuid) )
					{
						pButton->Disable(true);
					}
				}
			}
			
		}
	}

	void SetMyHomeEnterSlotList(XUI::CXUI_Wnd* pMainUI)
	{
		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pMainUI->GetControl(L"BLD_ITEM_SLOT"));
		if( !pBuild )
		{
			return;
		}

		CONT_MYHOME	kDisplay;
		g_kHomeRenewMgr.GetMyhomeEnterDisplayItem(kDisplay);

		CONT_MYHOME::const_iterator home_iter = kDisplay.begin();
		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pHomeSlot = pMainUI->GetControl(vStr);
			if( pHomeSlot )
			{
				if( home_iter != kDisplay.end() )
				{
					CONT_MYHOME::mapped_type const& kHomeInfo = home_iter->second;
					SetMyhomeEnterSlot(pHomeSlot, kHomeInfo);
					++home_iter;
				}
				else
				{
					SetMyhomeEnterSlot(pHomeSlot, SMYHOME());
				}
			}
		}
	}

	void SetMyhomeVisitLogSlotList(XUI::CXUI_Wnd* pMainUI)
	{
		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pMainUI->GetControl(L"BLD_ITEM_SLOT"));
		if( !pBuild )
		{
			return;
		}

		CONT_HOMEVISITLOG kDisplay;
		g_kHomeRenewMgr.GetMyhomeVisitLogDisplayItem(kDisplay);

		CONT_HOMEVISITLOG::const_iterator itr = kDisplay.begin();
		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM_SLOT");
			vStr += i;

			int iNum = ((g_kHomeRenewMgr.GetMyhomeVisitLogPage().Now()) * g_kHomeRenewMgr.GetMyhomeVisitLogPage().GetMaxItemSlot()) + (i + 1);
			XUI::CXUI_Wnd* pSlot = pMainUI->GetControl(vStr);
			if( pSlot )
			{
				if( itr != kDisplay.end() )
				{
					SHOMEVISITLOG const& kInfo = *itr;
					SetMyhomeVisitLogSlot(pSlot, kInfo, iNum);
					++itr;
				}
				else
				{
					SetMyhomeVisitLogSlot(pSlot, SHOMEVISITLOG(), iNum);
				}
			}
		}
	}

	void OnClickMyhomeEnterJumpPrevPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		
		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeEnterPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PagePrevJump() )
		{
			return;
		}

		SetMyHomeEnterSlotList(pMainUI);
		SetMyHomeEnterPageControl(pMainUI);
	}

	void OnClickMyhomeEnterJumpNextPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		
		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeEnterPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PageNextJump() )
		{
			return;
		}

		SetMyHomeEnterSlotList(pMainUI);
		SetMyHomeEnterPageControl(pMainUI);
	}

	void OnClickMyhomeEnterPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		if( kSelf.GetCheckState() )
		{
			return;
		}

		int const iBuildIndex = pSelf->BuildIndex();

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();

		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeEnterPage();
		int const NowPage = kPage.Now();
		int iNewPage = (NowPage / kPage.GetMaxViewPage()) * kPage.GetMaxViewPage() + iBuildIndex;
		if( NowPage == iNewPage )
		{
			return;
		}

		if( iNewPage >= kPage.Max() )
		{
			iNewPage = kPage.Max() - 1;
		}

		kPage.PageSet(iNewPage);
		SetMyHomeEnterSlotList(pMainUI);
		SetMyHomeEnterPageControl(pMainUI);
	}

	void OnClickMyhomeVisitLogJumpPrevPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		
		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeVisitLogPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PagePrevJump() )
		{
			return;
		}

		SetMyhomeVisitLogSlotList(pMainUI);
		SetMyhomeVisitLogPageControl(pMainUI);
	}

	void OnClickMyhomeVisitLogJumpNextPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		
		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeVisitLogPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PageNextJump() )
		{
			return;
		}

		SetMyhomeVisitLogSlotList(pMainUI);
		SetMyhomeVisitLogPageControl(pMainUI);
	}

	void OnClickMyhomeVisitLogPage(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		if( kSelf.GetCheckState() )
		{
			return;
		}

		int const iBuildIndex = pSelf->BuildIndex();

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();

		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeVisitLogPage();
		int const NowPage = kPage.Now();
		int iNewPage = (NowPage / kPage.GetMaxViewPage()) * kPage.GetMaxViewPage() + iBuildIndex;
		if( NowPage == iNewPage )
		{
			return;
		}

		if( iNewPage >= kPage.Max() )
		{
			iNewPage = kPage.Max() - 1;
		}

		kPage.PageSet(iNewPage);
		SetMyhomeVisitLogSlotList(pMainUI);
		SetMyhomeVisitLogPageControl(pMainUI);
	}

	void OpenMyhomeEnterWnd()
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"SFRM_MYHOME_ENTER");
		if( pkWnd )
		{
			RegistUIAction(pkWnd, L"CloseUI");

			int iPos = g_kHomeRenewMgr.GetMyhomeEnterPos();
			pkWnd->SetCustomData(&iPos, sizeof(iPos));

			CONT_MYHOME	kHomeList;
			g_kHomeTownMgr.GetTownHouseCont( iPos, kHomeList );
			g_kHomeRenewMgr.SetMyhomeEnterList(kHomeList);
			g_kHomeRenewMgr.SetMyhomeEnterPageMaxItem(kHomeList.size());
			// 자신의 집을 찾는다.
			bool bIsDisable = true;
			SetMyhomeAddrText(pkWnd, 0);
			CONT_MYHOME::iterator itr = kHomeList.begin();
			while(itr != kHomeList.end())
			{
				CONT_MYHOME::mapped_type const& kHome = itr->second;
				if( g_kPilotMan.IsMyPlayer(kHome.kOwnerGuid) )
				{
					SetMyhomeAddrText(pkWnd, kHome.iHouseNo);
					pkWnd->OwnerGuid(kHome.kOwnerGuid);
					SHOMEADDR kPoint(kHome.siStreetNo, kHome.iHouseNo);
					pkWnd->SetCustomData(&kPoint, sizeof(kPoint));
					bIsDisable = false;
					break;
				}
				++itr;
			}
			XUI::CXUI_Wnd* pkLineBox = pkWnd->GetControl(L"SFRM_LINE_BOX");
			if( pkLineBox )
			{
				if( bIsDisable )
				{
					pkLineBox->Text(L"");
				}
				else
				{
					BM::vstring strName(TTW(201680));
					PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
					if( pPilot )
					{
						std::wstring kMyName = pPilot->GetName();
						strName.Replace( L"#MARK1#", kMyName );
						pkLineBox->Text(strName);
					}
				}

				XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkLineBox->GetControl(L"BTN_MOVE"));
				if( pButton )
				{
					pButton->Disable(bIsDisable);
					if( bIsDisable )
					{
						pButton->Text(L"");
						pkWnd->OwnerGuid(BM::GUID::NullData());
						SHOMEADDR kPoint(0, 0);
						pkWnd->SetCustomData(&kPoint, sizeof(kPoint));
					}
					else
					{
						pButton->Text(TTW(2703));
					}
				}
			}
			// 타인의 집을 넣는다.
			XUI::CXUI_Wnd* pMainUI = pkWnd->GetControl(L"SFRM_BODY_SHD");
			if( pMainUI )
			{
				XUI::CXUI_Wnd* pArrow = pMainUI->GetControl(L"FRM_ARROW");
				if( pArrow )
				{
					BM::vstring strText(TTW(201683));
					strText.Replace( L"#MARK1#", lwHomeUIUtil::GetTownName(iPos));
					pArrow->Text(strText);
				}
				
				SetMyHomeEnterSlotList(pMainUI);
				SetMyHomeEnterPageControl(pMainUI);
			}
		}
	}

	bool ReserveMyhomeVisitComment()
	{
		if(!g_pkWorld)
		{
			return false;
		}
		if( lwHomeUI::IsHomeBlock() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201722, true);
			return false;
		}

		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return false;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( pkHomeUnit )
		{
			lwHomeUI::OnSendHomeInfoRequest(pkHomeUnit->HomeAddr(), lwHomeUIUtil::EEVT_CALL_MYHOME_BUFF);
		}
		else
		{// 집으로 들어왔는데 집이 없다?
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1923, true);
			return false;
		}
		return true;
	}

	bool OnCallMyhomeVisitComment(SMYHOME const& kHome)
	{
		{// 홈 Unit에 장착된 아이템들을 얻어온다
			PgMyHome* pkMyHome = NULL;
			{// 내 홈의
				PgPilot* pkTemp = g_kPilotMan.FindPilot(kHome.kHomeInSideGuid);
				if(!pkTemp)
				{
					return false;
				}
				pkMyHome = dynamic_cast<PgMyHome*>(pkTemp->GetUnit());
				if(!pkMyHome)
				{
					return false;
				}
			}
			PgInventory* pkHomeInv = pkMyHome->GetInven();
			if(!pkHomeInv)
			{
				return false;
			}

			XUI::CXUI_Wnd* pkVisit = XUIMgr.Call(L"SFRM_MYHOME_WRITE_VISIT_COMMENT");
			if(pkVisit)
			{
				XUI::CXUI_Wnd* pkBody = pkVisit->GetControl(L"SFRM_BODY_SHD");
				if( !pkBody )
				{
					return false;
				}

				XUI::CXUI_Wnd* pkComment = pkBody->GetControl(L"SFRM_COMMENT_WRITE");
				if( pkComment )
				{
					XUI::CXUI_Edit_MultiLine* pkEdit = dynamic_cast<XUI::CXUI_Edit_MultiLine*>(pkComment->GetControl(L"MEDT_COMMENT_WRITE"));
					if( pkEdit )
					{
						pkEdit->EditText(L"");
						pkEdit->SetEditFocus(true);
					}
				}
				
				BM::vstring strText(TTW(201686));
				strText.Replace( L"#MARK1#", kHome.kName);
				pkBody->Text(strText);

				XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkVisit->GetControl(L"LIST_MYHOME_BUFF_LIST"));
				if(!pkListWnd)
				{
					return false;
				}
				pkListWnd->ClearList();

				CONT_HOMEBUFF contBuff; // 이펙트타입, 레벨
				if( !GetMyhomeBuff(pkMyHome, contBuff) )
				{
					return false;
				}

				for( CONT_HOMEBUFF::iterator itr = contBuff.begin(); itr != contBuff.end(); ++itr )
				{
					SHomeBuff const& homeBuff = itr->second;

					XUI::SListItem* pkListItem = pkListWnd->AddItem(_T(""));
					if( pkListItem
						&&	pkListItem->m_pWnd )
					{//UI에 넣어주고
						XUI::CXUI_Wnd* pWnd = pkListItem->m_pWnd;
						XUI::CXUI_Wnd* pkEffName = pWnd->GetControl(L"FRM_BUFF_NAME");
						if(pkEffName)
						{
							pkEffName->Text(homeBuff.strName.c_str());
						}

						// 아이템 위치를 ListItem UI에 저장한다
						SItemPos kPos;
						pkHomeInv->GetFirstItem(IT_HOME, homeBuff.iItemNo, kPos);
						pWnd->SetCustomData(&kPos, sizeof(kPos));
					}
				}
			}
		}
		return true;
	}

	void RestoreMyhomeChatHeaderName()
	{
		XUI::CXUI_Wnd* pParent = XUIMgr.Get(_T("ChatBar"));
		if( !pParent )
		{
			return; 
		}

		XUI::CXUI_Wnd* pHead = pParent->GetControl(_T("BTN_TELL_TYPE"));
		if( !pHead )
		{
			return; 
		}

		g_kChatMgrClient.CheckChatCommand(L"/MODE 2 ");

		pHead->SetTwinkle(false);
	}

	void SendMyhomeExit(bool bPrevPos)
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_EXIT(pkHomeUnit->GetID(), bPrevPos);

		//RestoreMyhomeChatHeaderName();
	}

	void OnClickMyHomesSocket()
	{
		if(!g_pkWorld)
		{
			return;
		}
		if( lwHomeUI::IsHomeBlock() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201720, true);
			return;
		}

		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			if( pkHomeUnit )
			{
				std::wstring const kSocketSystemUIName(_T("SFRM_SOCKET_SYSTEM"));
				g_kSocketSystemMgr.NpcGuid( pkHomeUnit->GetID() );
				XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kSocketSystemUIName );
				if( pkTopWnd )
				{
					RegistUIAction(pkTopWnd);
				}
			}
		}
	}

	void DoHomeRenew()
	{
		
	}

	void OnRecvHomeInvite()
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("FRM_MINIMAP"));
		if( pkWnd )
		{
			lwPoint2 pt( pkWnd->Location().x, pkWnd->Location().y );
			lwCallToolTipByText(0, TTW(790603), pt, "TOOLTIP_FIX_POS" );
		}
	}

	void SetBoardNameTag(int iBoard, int iPos, char const* szName)
	{
		g_kHomeRenewMgr.SetBoardNameTag(iBoard, iPos, UNI(szName));
	}

	bool IsMyhomeApt()
	{
		//PgHome* pkHome = g_pkWorld->GetHome();
		//if( !pkHome )
		//{
		//	return false;
		//}

		//PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		//if( !pkHomeUnit )
		//{
		//	return false;
		//}

		//SHOMEADDR kAddr;
		//pkHomeUnit->HomeAddr(kAddr);
		SMYHOME const& kHome = g_kHomeTownMgr.GetMyHomeInfo();
		return g_kHomeRenewMgr.IsMyhomeApt(kHome.siStreetNo);
	}

	void SendMyhomeInvite(std::wstring const& strName)
	{
		if( strName.empty() )
		{
			return;
		}
		BM::Stream kPacket(PT_C_M_REQ_MYHOME_POST_INVITATION_CARD);
		kPacket.Push(strName);
		NETWORK_SEND(kPacket);
	}

	void SendMyhomeInvitebyFriend(lwGUID kGuid)
	{
		SFriendItem	rkFriend;
		if( g_kFriendMgr.Friend_Find_ByGuid(kGuid(), rkFriend) )
		{
			SendMyhomeInvite(rkFriend.ExtendName());
		}
	}

	void SendMyhomeInvitebyGuild(lwGUID kGuid)
	{
		SGuildMemberInfo kMemberInfo;
		if( g_kGuildMgr.GetMemberByGuid(kGuid(), kMemberInfo) )
		{
			SendMyhomeInvite(kMemberInfo.kCharName);
		}
	}

	void SendMyhomeInvitebyParty(lwGUID kGuid)
	{
		SPartyMember const* kPartyName = NULL;
		if( g_kParty.GetMember(kGuid(), kPartyName) )
		{
			SendMyhomeInvite(kPartyName->kName);
		}
	}

	void SendMyhomeInvitebyCouple()
	{
		SCouple const &rkMyCoupleInfo = g_kCoupleMgr.GetMyInfo();
		if( rkMyCoupleInfo.CoupleGuid() != BM::GUID::NullData() )
		{
			SendMyhomeInvite(rkMyCoupleInfo.CharName() );
		}
	}

	void SendEnterHome(short sStreet, int iHouse)
	{
		BM::Stream kPacket(PT_C_M_REQ_MYHOME_ENTER);
		kPacket.Push(sStreet);
		kPacket.Push(iHouse);

		if( g_kExpedition.ExpeditionGuid().IsNotNull() )
		{
			std::wstring kMessage = TTW(720048) + std::wstring(L"\n") + TTW(201109);
			CallCommonMsgYesNoBox(kMessage, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
		}
		else if( g_kParty.PartyGuid().IsNotNull() )
		{
			std::wstring kMessage = TTW(201119) + std::wstring(L"\n") + TTW(201109);
			CallCommonMsgYesNoBox(kMessage, 96, 97, lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET, NULL);
		}
		else
		{
			NETWORK_SEND(kPacket);
		}
	}

	void SendMyhomeGobyFriend(lwGUID kGuid)
	{
		SFriendItem	rkFriend;
		if( g_kFriendMgr.Friend_Find_ByGuid(kGuid(), rkFriend) )
		{
			SendEnterHome(rkFriend.HomeAddr().StreetNo(), rkFriend.HomeAddr().HouseNo());
		}
	}

	bool IsHaveMyhomebyFriend(lwGUID kGuid)
	{
		SFriendItem	rkFriend;
		if( g_kFriendMgr.Friend_Find_ByGuid(kGuid(), rkFriend) )
		{
			if( rkFriend.HomeAddr().StreetNo() > 0 && rkFriend.HomeAddr().HouseNo() > 0 )
			{
				return true;
			}
		}

		return false;
	}

	void SendMyhomeGobyGuild(lwGUID kGuid)
	{
		SGuildMemberInfo kMemberInfo;
		if( g_kGuildMgr.GetMemberByGuid(kGuid(), kMemberInfo) )
		{
			SendEnterHome(kMemberInfo.kHomeAddr.StreetNo(), kMemberInfo.kHomeAddr.HouseNo());
		}
	}

	void SendMyhomeGobyParty(lwGUID kGuid)
	{
		SPartyMember const* kPartyMember = NULL;
		if( g_kParty.GetMember( kGuid(), kPartyMember) )
		{
			SendEnterHome(kPartyMember->kHomeAddr.StreetNo(), kPartyMember->kHomeAddr.HouseNo());
		}
	}

	void SendMyhomeGobyCouple(lwGUID kGuid)
	{
		SCouple const &rkMyCoupleInfo = g_kCoupleMgr.GetMyInfo();
		if( rkMyCoupleInfo.CoupleGuid() != BM::GUID::NullData() )
		{
			if( rkMyCoupleInfo.HomeAddr().StreetNo() > 0 && rkMyCoupleInfo.HomeAddr().HouseNo() > 0)
			{
			SendEnterHome(rkMyCoupleInfo.HomeAddr().StreetNo(), rkMyCoupleInfo.HomeAddr().HouseNo());
		}
	}
	}

	void EnterMyhomeApt()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer && !pkPlayer->HomeAddr().IsNull() )
		{
			if( g_kHomeRenewMgr.IsMyhomeApt(pkPlayer->HomeAddr().StreetNo()) )
			{
				SendEnterHome(pkPlayer->HomeAddr().StreetNo(), pkPlayer->HomeAddr().HouseNo());
				return;
			}
		}

		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201711, true);
		XUI::CXUI_Wnd* pkCommon = XUIMgr.Get(L"SFRM_MSG_COMMON");
		if( pkCommon )
		{
			RegistUIAction(pkCommon, L"CloseUI");
		}
	}

	bool GetMyhomeBuff(PgMyHome* pkHomeUnit, CONT_HOMEBUFF& kContBuff)
	{
		if( !pkHomeUnit )
		{
			return false;
		}
		PgInventory* pkHomeInv = pkHomeUnit->GetInven();
		if(!pkHomeInv)
		{
			return false;
		}
		
		CONT_HAVE_ITEM_NO kContHomeItem;
		pkHomeInv->GetItems(IT_HOME, kContHomeItem);
		CONT_HAVE_ITEM_NO::const_iterator Item_itor = kContHomeItem.begin();
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CONT_DEFEFFECT const *pkDefEffect = NULL;
		g_kTblDataMgr.GetContDef(pkDefEffect);

		if (NULL==pkDefEffect)
		{
			return false;
		}

		while(kContHomeItem.end() != Item_itor)
		{
			int const& iItemNo = (*Item_itor);
			CItemDef const* pkDef = kItemDefMgr.GetDef(iItemNo);
			if (NULL!=pkDef)
			{
				int const iEffectNo = pkDef->GetAbil(AT_EFFECTNUM1);
				CONT_DEFEFFECT::const_iterator Eff_itor = pkDefEffect->find(iEffectNo);
				if(pkDefEffect->end() != Eff_itor)
				{
					CONT_DEFEFFECT::mapped_type const& kElement = Eff_itor->second;
					wchar_t const* pcEffName=NULL;
					GetDefString(kElement.iName, pcEffName);
					bool bInsert = true;
					SHomeBuff kBuff;
					kBuff.iItemNo = iItemNo;
					kBuff.iLv = kElement.iName % 100;
					kBuff.iType = kElement.iName - kBuff.iLv;
					kBuff.iEffectNo = iEffectNo;
					kBuff.strName = pcEffName;

					std::pair<CONT_HOMEBUFF::iterator, bool> Result = kContBuff.insert(std::make_pair(kBuff.iType, kBuff));
					if( false == Result.second )
					{
						if( Result.first->second.iLv < kBuff.iLv )
						{
							Result.first->second = kBuff;
						}
					}
				}
			}
			++Item_itor;
		}

		return true;
	}

	void OnMyhomeBuffDisplay()
	{
		if(NULL==g_pkWorld)
		{
			return;
		}

		PgHome* pkHome = g_pkWorld->GetHome();
		if( !pkHome )
		{
			return;
		}

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		CONT_HOMEBUFF kBuff;
		if( !GetMyhomeBuff(pkHomeUnit, kBuff) )
		{
			return;
		}

		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"FRM_MYHOME_BUFF_DISPLAY");
		if( pkWnd )
		{
			XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_ITEM_SLOT"));
			if( pkBuild )
			{
				CONT_HOMEBUFF::iterator itr = kBuff.begin();
				int const MAX_SLOT = pkBuild->CountX() * pkBuild->CountY();
				for(int i = 0; i < MAX_SLOT; ++i)
				{
					BM::vstring vStr(L"FRM_ITEM_SLOT");
					vStr += i;

					XUI::CXUI_Icon* pkSlot = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(vStr));
					if( pkSlot )
					{
						SIconInfo kIconInfo;
						kIconInfo.iIconGroup = pkSlot->IconInfo().iIconGroup;
						if( itr != kBuff.end() )
						{
							kIconInfo.iIconKey = itr->second.iEffectNo;
							++itr;
						}
						pkSlot->SetIconInfo(kIconInfo);
						pkSlot->SetCustomData(&kIconInfo.iIconKey, sizeof(kIconInfo.iIconKey));
					}
				}
			}
			lwShowMyHomeSize();
		}
	}

	//집정보 보기
	void OnCallMyhomeDetailView(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		SHOMEADDR kAddr;
		pParent->GetCustomData(&kAddr, sizeof(kAddr));

		OnCallMyhomeMinimap(kAddr.StreetNo(), kAddr.HouseNo());
	}

	void OnCallMyhomeMinimap(short const siStreetNo, int const iHouseNo)
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"SFRM_MYHOME_POS");
		if( !pMainUI || pMainUI->IsClosed() )
		{
			pMainUI = XUIMgr.Call(L"SFRM_MYHOME_POS");
			if( !pMainUI )
			{
				return;
			}
		}

		//if( !g_pkWorld->IsHaveAttr(GATTR_FLAG_HOMETOWN) )
		//{
		//	RegistUIAction(pMainUI, L"CloseUI");
		//}

		XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(L"SFRM_TOWN_TEXT");
		if( !pTemp )
		{
			return;
		}
		SHOMEADDR kAddr;
		pTemp->GetCustomData(&kAddr, sizeof(kAddr));

		BM::vstring kStr(L"BTN_HOME_ITEM");
		kStr += kAddr.HouseNo();
		XUI::CXUI_Wnd* pWnd = pMainUI->GetControl(kStr);
		if( pWnd )
		{
			pWnd->SetTwinkle(false);
			pWnd->TwinkleStartTime(0);
		}

		kStr = L"BTN_HOME_ITEM";
		kStr += iHouseNo;
		pWnd = pMainUI->GetControl(kStr);
		if( pWnd )
		{
			pWnd->SetTwinkle(true);
			pWnd->TwinkleInterTime(100);
		}

		{
			for( int i=1; i<=MAX_MYHOME_BOARD_NUM; ++i )
			{
				BM::vstring strName(L"SFRM_ARROW");
				strName += i;
				XUI::CXUI_Wnd* pArrow = pMainUI->GetControl(strName);
				if( pArrow )
				{
					if( i == siStreetNo )
					{
						pArrow->Visible(true);
					}
					else
					{
						pArrow->Visible(false);
					}
				}
			}
		}
		wchar_t szTemp[MAX_PATH] = {0,};
		//swprintf_s(szTemp, TTW(201003).c_str(), siStreetNo);
		pTemp->Text(lwHomeUIUtil::GetTownName(siStreetNo));
		kAddr.StreetNo(siStreetNo);
		kAddr.HouseNo(iHouseNo);
		pTemp->SetCustomData(&kAddr, sizeof(kAddr));

		//XUI::CXUI_Wnd* pBackImg = pMainUI->GetControl(L"SFRM_TOWNMAP");
		//if( pBackImg )
		//{
		//	wchar_t szTemp[MAX_PATH]={0,};
		//	swprintf_s(szTemp, MAX_PATH, L"../Data/6_ui/myhome/mhMap%02d.tga", siStreetNo);
		//	pBackImg->DefaultImgName(szTemp);
		//}

		CONT_MYHOME	kHomeList;
		g_kHomeTownMgr.GetTownHouseCont( siStreetNo, kHomeList );

		int iFirstHouse = iHouseNo;

		CONT_MYHOME::iterator home_iter = kHomeList.begin();
		for(int i = 1; i <= BUILDING_MAX_HOUSE_COUNT; ++i)
		{
			BM::vstring vStr(L"BTN_HOME_ITEM");
			vStr += i;

			XUI::CXUI_Button* pMiniMapIcon = dynamic_cast<XUI::CXUI_Button*>(pMainUI->GetControl(vStr));
			if( pMiniMapIcon )
			{
				if( home_iter != kHomeList.end() && home_iter->first.HouseNo() == i)
				{
					CONT_MYHOME::mapped_type const& kHome = home_iter->second;

					if( iFirstHouse == 0 )
					{
						iFirstHouse = kHome.iHouseNo;
					}

					//NiScreenTexture::ScreenRect* pRect = NULL;
					//int iRectCount = 0;
					//{
					//	PgUISpriteObject *pkSpriteObj = (PgUISpriteObject *)pMiniMapIcon->BtnImg();
					//	if( pkSpriteObj )
					//	{
					//		NiScreenTexture *pkSprite = pkSpriteObj->GetTex();
					//		iRectCount = pkSprite->GetNumScreenRects();
					//		pRect = NiNew NiScreenTexture::ScreenRect[ iRectCount ];
					//		if( pRect )
					//		{
					//			for(int i = 0; i < iRectCount; ++i)
					//			{
					//				pRect[i] = pkSprite->GetScreenRect(i);
					//			}
					//		}
					//	}
					//}

					if( kHome.kName.empty() )
					{
						pMiniMapIcon->Text( TTW(201014) );
					}
					else
					{
						pMiniMapIcon->Text( kHome.kName );
					}
					XUI::CXUI_Image* pHomeIcon = dynamic_cast<XUI::CXUI_Image*>(pMiniMapIcon->GetControl(L"IMG_HOME"));
					if( pHomeIcon )
					{
						if( g_kPilotMan.IsMyPlayer( kHome.kOwnerGuid ) )
						{
							SUVInfo kUVInfo = pHomeIcon->UVInfo();
							kUVInfo.Index = 27;
							pHomeIcon->UVInfo(kUVInfo);
							std::wstring const& strName = pMiniMapIcon->BtnImgName();
							std::wstring const& strImg(L"../Data/6_ui/myhome/mhNIdBtMy.tga");
							if( strName != strImg )
							{
								pMiniMapIcon->ButtonChangeImage(strImg);
							}
						}
						else
						{
							if( MAS_IS_BIDDING == kHome.bAuctionState )
							{
								SUVInfo kUVInfo = pHomeIcon->UVInfo();
								kUVInfo.Index = 25;
								pHomeIcon->UVInfo(kUVInfo);
								std::wstring const& strName = pMiniMapIcon->BtnImgName();
								std::wstring const& strImg(L"../Data/6_ui/myhome/mhNIdBt.tga");
								if( strName != strImg )
								{
									pMiniMapIcon->ButtonChangeImage(strImg);
								}
							}
							else
							{
								if( PgHomeUIUtil::EHCR_CLOSE == PgHomeUIUtil::CheckToHomeVisitPossible( kHome ) )
								{
									SUVInfo kUVInfo = pHomeIcon->UVInfo();
									kUVInfo.Index = 26;
									pHomeIcon->UVInfo(kUVInfo);
									std::wstring const& strName = pMiniMapIcon->BtnImgName();
									std::wstring const& strImg(L"../Data/6_ui/myhome/mhNIdBt.tga");
									if( strName != strImg )
									{
										pMiniMapIcon->ButtonChangeImage(strImg);
									}
								}
								else
								{
									SUVInfo kUVInfo = pHomeIcon->UVInfo();
									kUVInfo.Index = 24;
									pHomeIcon->UVInfo(kUVInfo);
									std::wstring const& strName = pMiniMapIcon->BtnImgName();
									std::wstring const& strImg(L"../Data/6_ui/myhome/mhNIdBt.tga");
									if( strName != strImg )
									{
										pMiniMapIcon->ButtonChangeImage(strImg);
									}
								}
							}
						}
					}

					//{
					//	PgUISpriteObject *pkSpriteObj = (PgUISpriteObject *)pMiniMapIcon->BtnImg();
					//	if( pkSpriteObj )
					//	{
					//		if( pRect )
					//		{
					//		NiScreenTexture *pkSprite = pkSpriteObj->GetTex();
					//		for(int i = 0; i < iRectCount; ++i)
					//		{
					//			pkSprite->AddNewScreenRect(pRect[i].m_sPixTop, pRect[i].m_sPixLeft, pRect[i].m_usPixWidth, pRect[i].m_usPixHeight,
					//				pRect[i].m_usTexTop, pRect[i].m_usTexLeft, pRect[i].m_kColor);
					//		}
					//			NiDelete [] pRect;
					//			pRect = NULL;
					//		}
					//	}
					//}

					pMiniMapIcon->Visible(true);
					++home_iter;
				}
				else
				{
					pMiniMapIcon->Visible(false);
				}
			}
		}

		lwHomeUI::OnClickMinimapHomeItem(lwUIWnd(pMainUI), iFirstHouse);
	}

	void OnSendBuild_HomeList()
	{
		if( g_pkWorld )
		{
			const CONT_DEFGROUNDBUILDINGS* pCont = NULL;
			g_kTblDataMgr.GetContDef(pCont);
			if( pCont )
			{
				CONT_DEFGROUNDBUILDINGS::const_iterator itr = pCont->find(g_pkWorld->MapNo());
				if( itr != pCont->end())
				{
					BM::Stream kPacket(PT_C_M_REQ_HOMETOWN_INFO);
					kPacket.Push((BYTE)PgHomeUIUtil::ETLRT_OPEN_BUILDING_NAME_BOARD);
					NETWORK_SEND(kPacket);
				}
			}
		}
	}

	void SetBuildNameBoard()
	{
		if( g_pkWorld )
		{
			const CONT_DEFGROUNDBUILDINGS* pCont = NULL;
			g_kTblDataMgr.GetContDef(pCont);
			if( pCont )
			{
				CONT_DEFGROUNDBUILDINGS::const_iterator itr = pCont->find(g_pkWorld->MapNo());
				if( itr != pCont->end())
				{
					g_kHomeRenewMgr.SetHomeBoard();
				}
			}
		}
		
	}

	void SetBuildNameBoardforOptionChange()
	{
		SetBuildNameBoard();

		char const * szEventName = "SetBuildNameBoard";
		STimerEvent kEvent;
		kEvent.Set(0.5f, "SetBuildNameBoard()");
		g_kEventTimer.AddLocal(szEventName, kEvent);
	}

	void OnClickMinimapMyhomePopup(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pDropWnd = XUIMgr.Call(L"FRM_TOWNMAP_MYHOME_SELECT");
		if( !pDropWnd )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
		if( !pList )
		{
			return;
		}

		pList->DeleteAllItem();

		wchar_t szTemp[MAX_PATH] = {0,};
		int ItemHeight = 0;

		PgHomeUI::CONT_TOWNNO const& kTownNoList = g_kHomeTownMgr.TownNoList();
		PgHomeUI::CONT_TOWNNO::const_iterator townNo_itor = kTownNoList.begin();
		while( townNo_itor != kTownNoList.end() )
		{
			PgHomeUI::CONT_TOWNNO::value_type const& kTownNo = (*townNo_itor);

			XUI::SListItem* pListItem = pList->AddItem(L"");
			if( pListItem && pListItem->m_pWnd )
			{
				pListItem->m_pWnd->SetCustomData(&kTownNo, sizeof(kTownNo));
				//swprintf_s(szTemp, MAX_PATH, TTW(201003).c_str(), kTownNo);
				pListItem->m_pWnd->Text(lwHomeUIUtil::GetTownName(kTownNo));
				ItemHeight = pListItem->m_pWnd->Size().y;
			}
			++townNo_itor;
		}

		if( !pList->GetTotalItemCount() )
		{
			pDropWnd->Close();
		}

		XUI::CXUI_Wnd* pListBorder = pDropWnd->GetControl(L"SFRM_BORDER");
		if( !pListBorder )
		{
			return;
		}
		pDropWnd->Location(pParent->TotalLocation().x, pParent->TotalLocation().y + pParent->Size().y);
		pList->Size(POINT2(pList->Size().x, ItemHeight * pList->GetTotalItemCount()));
		pDropWnd->Size(pDropWnd->Size().x, pList->Size().y);
		pListBorder->Size(POINT2(pListBorder->Size().x, pList->Size().y + 4));
	}

	void OnClickMinimapMyhomePopupItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( !pSelf )
		{
			return;
		}
		short siStreetNo = 0;
		pSelf->GetCustomData(&siStreetNo, sizeof(siStreetNo));

		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"SFRM_MYHOME_POS");
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Wnd* pDropTitle = pMainUI->GetControl(L"SFRM_TOWN_TEXT");
		if( !pDropTitle )
		{
			return;
		}
		SHOMEADDR kAddr;
		pDropTitle->GetCustomData(&kAddr, sizeof(kAddr));
		kAddr.StreetNo(siStreetNo);
		pDropTitle->SetCustomData(&kAddr, sizeof(kAddr));

		OnCallMyhomeMinimap(siStreetNo);
	}

	void OnCallMyhomeInviteLetter()
	{
		if(!g_pkWorld)
		{
			return;
		}
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"SFRM_MYHOME_INVITE_LETTER");
		if( !pkWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pkBack = pkWnd->GetControl(L"SFRM_BACK");
		if( !pkBack )
		{
			return;
		}
		int iDay = 5;

		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			if( pkHomeUnit )
			{
				PLAYER_ABIL kInfo;
				pkHomeUnit->GetPlayerAbil(kInfo);
				if( kInfo.iHairStyle )
				{
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pkItemDef = kItemDefMgr.GetDef(kInfo.iHairStyle);
					if( pkItemDef )
					{
						int iTempNum = pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_4);
						if( iTempNum )
						{
							iDay = iTempNum;
						}
					}
				}

			}
		}
		
		BM::vstring strText(TTW(201717));
		strText.Replace( L"#MARK1#", iDay);
		pkBack->Text(strText);
	}

	void OnCallMyhomeSellCommonMessageBox(lwUIWnd kWnd)
	{// 퇴거 메세지 창을 부른다.
		kWnd.Close();

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(pkPlayer)
		{
			SHOMEADDR kAddr = pkPlayer->HomeAddr();
			if( !g_kHomeRenewMgr.IsMyhomeApt(kAddr.StreetNo() ) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201740, true);
				return;
			}
		}

		g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_SELL();
	}

	///////////////////////////////////////////////////
	// 마이홈 채팅
	///////////////////////////////////////////////////
	bool IsInsideHome()
	{
		if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_MYHOME))
		{
			return true;
		}

		return false;
	}

	void SetChatHeaderName()
	{
		XUI::CXUI_Wnd* pParent = XUIMgr.Get(_T("ChatBar"));
		if( !pParent )
		{
			return; 
		}

		XUI::CXUI_Wnd* pHead = pParent->GetControl(_T("BTN_TELL_TYPE"));
		if( !pHead )
		{
			return; 
		}

		pHead->Text(TTW(201684));

		DWORD const dwTwinkleTime = 60000;	//1분간 깜빡거리도록
		DWORD const dwTwinkleInter = 500;
		pHead->SetTwinkle(true);
		pHead->TwinkleTime(dwTwinkleTime);
		pHead->TwinkleInterTime(dwTwinkleInter);
	}

	bool IsMyRoomMasterChatRoom()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		BM::GUID kRoomGuid;
		if( FindMyhomeChatRoomGuid(kRoomGuid) )
		{
			SHomeChatRoom* pkChatRoom = g_kHomeRenewMgr.FindMyhomeChatRoom(kRoomGuid);
			if( pkChatRoom )
			{
				SHomeChatGuest* pkChatGuest = g_kHomeRenewMgr.FindMyhomeChatRoomMaster(*pkChatRoom);
				if( pkChatGuest )
				{
					if( pkChatGuest->kID == pkPlayer->GetID() )
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	// 마이홈 채팅 SFRM_MYHOME_CHAT_POPUP
	void OpenMyhomeChatPopup()
	{
		bool bBidding = lwHomeUI::IsHomeBidding();
		if( bBidding )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201799, true);
			return;
		}

		XUIMgr.Activate(L"SFRM_MYHOME_CHAT_POPUP");
	}

	void MyhomeChatPopupEnter(lwUIWnd kWnd)
	{
		PgMyHome* pkHomeUnit = g_kHomeRenewMgr.GetCurrentHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		kWnd.Close();

		std::wstring strPW;
		g_kHomeRenewMgr.SetPublicChatRoom(true);
		g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_ENTER(pkHomeUnit->GetID(), BM::GUID::NullData(), strPW);
	}

	void MyhomeChatPopupList(lwUIWnd kWnd)
	{
		PgMyHome* pkHomeUnit = g_kHomeRenewMgr.GetCurrentHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		kWnd.Close();

		g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_ROOM_LIST(pkHomeUnit->GetID());
	}

	// 채팅방 만들기 SFRM_MYHOME_CHAT_MAKE
	void MyhomeChatMakeMemberDropDown(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}
		
		XUI::CXUI_Wnd* pDropWnd = XUIMgr.Call(L"SFRM_MYHOME_CHAT_MAKE_DROPDOWN");
		if( !pDropWnd )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
		if( !pList )
		{
			return;
		}

		pList->DeleteAllItem();

		int ItemHeight = 0;
		for( int i=2; i<=10; ++i)
		{
			XUI::SListItem* pListItem = pList->AddItem(L"");
			if( pListItem && pListItem->m_pWnd )
			{
				pListItem->m_pWnd->SetCustomData(&i, sizeof(i));
				pListItem->m_pWnd->Text(BM::vstring(i));
				ItemHeight += pListItem->m_pWnd->Size().y;
			}
		}

		XUI::CXUI_Wnd* pListBorder = pDropWnd->GetControl(L"SFRM_BORDER");
		if( !pListBorder )
		{
			return;
		}
		pDropWnd->Location(pkWnd->TotalLocation().x, pkWnd->TotalLocation().y + pkWnd->Size().y);
		pList->Size(POINT2(pList->Size().x, ItemHeight));
		pDropWnd->Size(pDropWnd->Size().x, pList->Size().y);
		pListBorder->Size(POINT2(pListBorder->Size().x, pList->Size().y + 4));

	}

	void MyhomeChatMakeDropDownItem(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pSelf = kWnd.GetSelf();
		if( !pSelf )
		{
			return;
		}
		int iData = 0;
		pSelf->GetCustomData(&iData, sizeof(iData));

		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"SFRM_MYHOME_CHAT_MAKE");
		if( !pMainUI )
		{
			return;
		}

		XUI::CXUI_Wnd* pkShd = pMainUI->GetControl(L"SFRM_BODY_SHD");
		if( !pkShd )
		{
			return;
		}

		XUI::CXUI_Wnd* pkDrop = pkShd->GetControl(L"SFRM_TEXT2");
		if( !pkDrop )
		{
			return;
		}

		pkDrop->Text(BM::vstring(iData));
	}

	void MyhomeChatMakePublicOpen(lwUIWnd kWnd, bool bPublic)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}

		XUI::CXUI_Wnd* pkShd = pkWnd->GetControl(L"SFRM_BODY_SHD");
		if( !pkShd )
		{
			return;
		}

		XUI::CXUI_CheckButton* pkCBtn1 = dynamic_cast<XUI::CXUI_CheckButton*>(pkShd->GetControl(L"CBTN_BTN1"));
		XUI::CXUI_CheckButton* pkCBtn2 = dynamic_cast<XUI::CXUI_CheckButton*>(pkShd->GetControl(L"CBTN_BTN2"));
		if( !pkCBtn1 || !pkCBtn2 )
		{
			return;
		}

		pkCBtn1->Check(bPublic);
		pkCBtn2->Check(!bPublic);

		XUI::CXUI_Wnd* pkPWTitle = pkShd->GetControl(L"SFRM_TITLE4");
		XUI::CXUI_Wnd* pkPWText = pkShd->GetControl(L"SFRM_TEXT4");
		if( !pkPWTitle || !pkPWText )
		{
			return;
		}

		if( bPublic )
		{
			pkPWTitle->Visible(false);
			pkPWText->Visible(false);
			pkPWText->Text(L"");
		}
		else
		{
			pkPWTitle->Visible(true);
			pkPWText->Visible(true);
		}
	}

	void MyhomeChatMakeNotice(lwUIWnd kWnd)
	{
	}

	void MyhomeChatMakeOK(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}

		PgMyHome* pkHomeUnit = g_kHomeRenewMgr.GetCurrentHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		std::wstring strTitle;
		std::wstring strNotice;
		std::wstring strPW;
		std::wstring strNum;

		

		XUI::CXUI_Wnd* pkShd = pkWnd->GetControl(L"SFRM_BODY_SHD");
		XUI::CXUI_Wnd* pkShd2 = pkWnd->GetControl(L"SFRM_BODY_SHD2");
		if( !pkShd || !pkShd2 )
		{
			return;
		}

		XUI::CXUI_Wnd* pkRoomTitle = pkShd->GetControl(L"SFRM_TEXT1");
		if( pkRoomTitle )
		{
			XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pkRoomTitle->GetControl(L"EDT_TITLE"));
			if( pEdit )
			{
				strTitle = pEdit->EditText();
				if( strTitle.empty() )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201793, true);
					return;
				}
				if ( g_kClientFS.Filter( strTitle, false, FST_ALL ) )
				{
					pEdit->EditText(L"");
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50513, true);
					return;
				}
			}
		}

		XUI::CXUI_CheckButton* pkCBtnNotice = dynamic_cast<XUI::CXUI_CheckButton*>(pkShd2->GetControl(L"CBTN_NOTICE"));
		if( pkCBtnNotice )
		{
			if( pkCBtnNotice->Check() )
			{
				XUI::CXUI_Wnd* pkNotice = pkShd2->GetControl(L"SFRM_TEXT");
				if( pkNotice )
				{
					XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pkNotice->GetControl(L"EDT_NOTICE"));
					if( pEdit )
					{
						strNotice = pEdit->EditText();
						if( strNotice.empty() )
						{
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201795, true);
							return;
						}

						if ( g_kClientFS.Filter( strNotice, false, FST_ALL ) )
						{
							pEdit->EditText(L"");
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50513, true);
							return;
						}
					}
				}
			}
		}

		XUI::CXUI_CheckButton* pkCBtn1 = dynamic_cast<XUI::CXUI_CheckButton*>(pkShd->GetControl(L"CBTN_BTN1"));
		if( pkCBtn1 )
		{
			if( !pkCBtn1->Check() )
			{
				XUI::CXUI_Wnd* pkPW = pkShd->GetControl(L"SFRM_TEXT4");
				if( pkPW )
				{
					XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pkPW->GetControl(L"EDT_PW"));
					if( pEdit )
					{
						strPW = pEdit->EditText();
						if( strPW.empty() )
						{
							lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201794, true);
							return;
						}
					}
				}
			}
		}

		XUI::CXUI_Wnd* pkNum = pkShd->GetControl(L"SFRM_TEXT2");
		if( pkNum )
		{
			strNum = pkNum->Text();
		}

		int iNum = _wtoi(strNum.c_str());
		int iMake = 0;
		pkWnd->GetCustomData(&iMake, sizeof(iMake));
		if( 1 == iMake )// make
		{
			kWnd.Close();
			g_kHomeRenewMgr.SetPublicChatRoom(false);
			g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_ROOM_CREATE(pkHomeUnit->GetID(), strTitle, strNotice, strPW, iNum);
		}
		else
		{
			g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_MODIFY_ROOM(pkHomeUnit->GetID(), strTitle, strNotice, strPW, iNum);
		}
	}

	// 채팅방 리스트 SFRM_MYHOME_CHAT_ROOM_LIST
	void OpenMyhomeChatRoomList()
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ROOM_LIST");
		if( !pkWnd )
		{
			pkWnd = XUIMgr.Call(L"SFRM_MYHOME_CHAT_ROOM_LIST");
		}
		if( !pkWnd )
		{
			return;
		}

		XUI::CXUI_Wnd* pkShd = pkWnd->GetControl(L"SFRM_BODY_SHD");
		if( !pkShd )
		{
			return;
		}

		SetMyhomeChatRoomList(pkShd);
		SetMyhomeChatRoomListPageControl(pkShd->GetControl(L"FRM_PAGE"));
	}

	void SetMyhomeChatRoomList(XUI::CXUI_Wnd* pMainUI)
	{
		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pMainUI->GetControl(L"BLD_ITEM_SLOT"));
		if( !pBuild )
		{
			return;
		}

		CONT_MYHOMECHATROOM	kDisplay;
		g_kHomeRenewMgr.GetMyhomeChatRoomDisplayItem(kDisplay);

		if( kDisplay.empty() )
		{
			XUI::CXUI_Wnd* pkHideText = pMainUI->GetControl(L"FRM_HIDE_TEXT");
			if( pkHideText )
			{
				pkHideText->Visible(true);
			}
		}
		else
		{
			XUI::CXUI_Wnd* pkHideText = pMainUI->GetControl(L"FRM_HIDE_TEXT");
			if( pkHideText )
			{
				pkHideText->Visible(false);
			}
		}
		int iNowPage = g_kHomeRenewMgr.GetMyhomeChatRoomPage().Now();
		int iMaxSlot = g_kHomeRenewMgr.GetMyhomeChatRoomPage().GetMaxItemSlot();
		int iNum = iNowPage * iMaxSlot + 1;

		CONT_MYHOMECHATROOM::iterator itr = kDisplay.begin();
		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pSlot = pMainUI->GetControl(vStr);
			if( pSlot )
			{
				if( itr != kDisplay.end() )
				{
					CONT_MYHOMECHATROOM::mapped_type& kInfo = itr->second;
					SetMyhomeChatRoomSlot(pSlot, kInfo, iNum+i);
					++itr;
				}
				else
				{
					SetMyhomeChatRoomSlot(pSlot, SHomeChatRoom(), iNum+i);
				}
			}
		}
	}

	void SetMyhomeChatRoomSlot(XUI::CXUI_Wnd* pSlot, SHomeChatRoom& kChatRoom, int iNum)
	{
		if( !pSlot )
		{
			return;
		}

		if( BM::GUID::NullData() == kChatRoom.kRoomGuid )
		{
			pSlot->Visible(false);
		}
		else
		{
			pSlot->Visible(true);
		}

		pSlot->OwnerGuid(kChatRoom.kRoomGuid);

		XUI::CXUI_Wnd* pTemp = pSlot->GetControl(L"SFRM_ITEM_BG1");
		if( pTemp )
		{
			BM::vstring strText(iNum);
			pTemp->Text(strText);
		}

		pTemp = pSlot->GetControl(L"SFRM_ITEM_BG2");
		if( pTemp )
		{
			if( MAX_PUBLIC_CHAT_USER_COUNT == kChatRoom.kMaxUserCount )// 마이홈 전체 채팅
			{
				pTemp->Text(TTW(201772));
			}
			else
			{
				pTemp->Text(kChatRoom.kRoomTitle);
			}
		}

		pTemp = pSlot->GetControl(L"SFRM_ITEM_BG3");
		if( pTemp )
		{
			if( kChatRoom.kRoomPassword.empty() || MAX_PUBLIC_CHAT_USER_COUNT == kChatRoom.kMaxUserCount )
			{
				pTemp->Text(TTW(201751));
			}
			else
			{
				pTemp->Text(TTW(201752));
			}
		}

		pTemp = pSlot->GetControl(L"SFRM_ITEM_BG4");
		if( pTemp )
		{
			if( MAX_PUBLIC_CHAT_USER_COUNT == kChatRoom.kMaxUserCount )// 마이홈 전체 채팅
			{
				pTemp->Text(L"");
			}
			else
			{
				SHomeChatGuest* pkGuest = g_kHomeRenewMgr.FindMyhomeChatRoomMaster(kChatRoom);
				if( pkGuest )
				{
					pTemp->Text(pkGuest->kName);
				}
			}
		}

		pTemp = pSlot->GetControl(L"SFRM_ITEM_BG5");
		if( pTemp )
		{
			if( MAX_PUBLIC_CHAT_USER_COUNT == kChatRoom.kMaxUserCount )// 마이홈 전체 채팅
			{
				BM::vstring strText(kChatRoom.kContGuest.size());
				pTemp->Text(strText);
			}
			else
			{
				SHomeChatGuest* pkGuest = g_kHomeRenewMgr.FindMyhomeChatRoomMaster(kChatRoom);
				if( pkGuest )
				{
					BM::vstring strText(kChatRoom.kContGuest.size());
					strText += '/';
					strText += BM::vstring(kChatRoom.kMaxUserCount);
					pTemp->Text(strText);
				}
			}
		}
	}

	void MyhomeChatRoomListEnter(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}

		SHomeChatRoom* pkChatRoom = g_kHomeRenewMgr.FindMyhomeChatRoom(pkWnd->OwnerGuid() );
		if( pkChatRoom )
		{
			if( pkChatRoom->kRoomPassword.empty() )// 공개
			{
				PgMyHome* pkHome = g_kHomeRenewMgr.GetCurrentHomeUnit();
				if( !pkHome )
				{
					return;
				}
				std::wstring strPW;
				bool bPublic = false;
				if( MAX_PUBLIC_CHAT_USER_COUNT == pkChatRoom->kMaxUserCount )
				{
					bPublic = true;
				}
				g_kHomeRenewMgr.SetPublicChatRoom(bPublic);
				g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_ENTER(pkHome->GetID(), pkWnd->OwnerGuid(), strPW);
				return;
			}
		}

		// 선택된 리스트의 정보를 메세지 창으로 넘긴다.
		XUI::CXUI_Wnd* pkMsgWnd = XUIMgr.Call(L"FRM_MYHOME_CHAT_ROOM_LIST_MSG_BOX");
		if( pkMsgWnd )
		{
			pkMsgWnd->OwnerGuid(pkWnd->OwnerGuid());
			XUI::CXUI_Wnd* pkMessage = pkMsgWnd->GetControl(L"FRM_MESSAGE");
			if( pkMessage )
			{
				pkMessage->Text(TTW(201779));
			}
		}
	}

	void MyhomeChatRoomListMsgBoxOK(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}
		
		XUI::CXUI_Edit* pkEdit = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(L"EDT_PW"));
		if( !pkEdit )
		{
			return;
		}
		std::wstring strPW = pkEdit->EditText();
		
		SHomeChatRoom* pkChatRoom = g_kHomeRenewMgr.FindMyhomeChatRoom(pkWnd->OwnerGuid() );
		if( !pkChatRoom )
		{
			return;
		}

		if( strPW != pkChatRoom->kRoomPassword )
		{// 패스워드가 틀렸다.
			XUI::CXUI_Wnd* pkMessage = pkWnd->GetControl(L"FRM_MESSAGE");
			if( pkMessage )
			{
				pkMessage->Text(TTW(201780));
			}
			XUI::CXUI_Edit* pkEdit = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(L"EDT_PW"));
			if( pkEdit )
			{
				pkEdit->EditText(L"");
			}
			return;
		}

		PgMyHome* pkHome = g_kHomeRenewMgr.GetCurrentHomeUnit();
		if( !pkHome )
		{
			return;
		}
		// 저장된 정보로 채팅방에 들어간다.
		g_kHomeRenewMgr.SetPublicChatRoom(false);
		g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_ENTER(pkHome->GetID(), pkWnd->OwnerGuid(), strPW);

		kWnd.Close();
	}

	void SetMyhomeChatRoomListPageControl(XUI::CXUI_Wnd* pPageMainUI)
	{
		if( !pPageMainUI )
		{
			return;
		}

		lwHomeUIUtil::CommonPageControl(pPageMainUI, g_kHomeRenewMgr.GetMyhomeChatRoomPage());
	}

	void MyhomeChatRoomListPageFirst(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pkPage = pkWnd->Parent();
		if( !pkPage )
		{
			return;
		}
		XUI::CXUI_Wnd* pkShd = pkPage->Parent();
		if( !pkShd )
		{
			return;
		}
		
		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeChatRoomPage();
		int const NowPage = kPage.Now();
		if( NowPage == kPage.PageBegin() )
		{
			return;
		}

		SetMyhomeChatRoomList(pkShd);
		SetMyhomeChatRoomListPageControl(pkPage);
	}

	void MyhomeChatRoomListPageJumpPrev(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pkPage = pkWnd->Parent();
		if( !pkPage )
		{
			return;
		}
		XUI::CXUI_Wnd* pkShd = pkPage->Parent();
		if( !pkShd )
		{
			return;
		}
		
		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeChatRoomPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PagePrevJump() )
		{
			return;
		}

		SetMyhomeChatRoomList(pkShd);
		SetMyhomeChatRoomListPageControl(pkPage);
	}

	void MyhomeChatRoomListPagePrev(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pkPage = pkWnd->Parent();
		if( !pkPage )
		{
			return;
		}
		XUI::CXUI_Wnd* pkShd = pkPage->Parent();
		if( !pkShd )
		{
			return;
		}
		
		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeChatRoomPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PagePrev() )
		{
			return;
		}

		SetMyhomeChatRoomList(pkShd);
		SetMyhomeChatRoomListPageControl(pkPage);
	}

	void MyhomeChatRoomListPageNext(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pkPage = pkWnd->Parent();
		if( !pkPage )
		{
			return;
		}
		XUI::CXUI_Wnd* pkShd = pkPage->Parent();
		if( !pkShd )
		{
			return;
		}
		
		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeChatRoomPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PageNext() )
		{
			return;
		}

		SetMyhomeChatRoomList(pkShd);
		SetMyhomeChatRoomListPageControl(pkPage);
	}

	void MyhomeChatRoomListPageJumpNext(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pkPage = pkWnd->Parent();
		if( !pkPage )
		{
			return;
		}
		XUI::CXUI_Wnd* pkShd = pkPage->Parent();
		if( !pkShd )
		{
			return;
		}
		
		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeChatRoomPage();
		int const NowPage = kPage.Now() + 1;
		if( NowPage == kPage.PageNextJump() )
		{
			return;
		}

		SetMyhomeChatRoomList(pkShd);
		SetMyhomeChatRoomListPageControl(pkPage);
	}

	void MyhomeChatRoomListPageEnd(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pkPage = pkWnd->Parent();
		if( !pkPage )
		{
			return;
		}
		XUI::CXUI_Wnd* pkShd = pkPage->Parent();
		if( !pkShd )
		{
			return;
		}
		
		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeChatRoomPage();
		int const NowPage = kPage.Now();
		if( NowPage == kPage.PageEnd() )
		{
			return;
		}

		SetMyhomeChatRoomList(pkShd);
		SetMyhomeChatRoomListPageControl(pkPage);
	}

	void MyhomeChatRoomListPageNum(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pkPage = pkWnd->Parent();
		if( !pkPage )
		{
			return;
		}
		XUI::CXUI_Wnd* pkShd = pkPage->Parent();
		if( !pkShd )
		{
			return;
		}

		if( kWnd.GetCheckState() )
		{
			return;
		}

		int const iBuildIndex = pkWnd->BuildIndex();

		PgPage& kPage = g_kHomeRenewMgr.GetMyhomeChatRoomPage();
		int const NowPage = kPage.Now();
		int iNewPage = (NowPage / kPage.GetMaxViewPage()) * kPage.GetMaxViewPage() + iBuildIndex;
		if( NowPage == iNewPage )
		{
			return;
		}

		if( iNewPage >= kPage.Max() )
		{
			iNewPage = kPage.Max() - 1;
		}

		kPage.PageSet(iNewPage);
		SetMyhomeChatRoomList(pkShd);
		SetMyhomeChatRoomListPageControl(pkPage);
	}

	void MyhomeChatRoomListMake(lwUIWnd kWnd)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			if( pkPlayer->HomeAddr().IsNull() )// 홈 미소유자
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201781, true);
				return;
			}
		}

		kWnd.Close();

		XUI::CXUI_Wnd* pkMake = XUIMgr.Call(L"SFRM_MYHOME_CHAT_MAKE");
		if( pkMake )
		{
			int i = 1;// make
			pkMake->SetCustomData(&i, sizeof(i));
		}
	}

	// SFRM_MYHOME_CHAT_ROOM_CHAT
	void SetMyhomeChatRoomChatTitleBG(SHomeChatRoom& kChatRoom)
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ROOM_CHAT");
		if( !pkWnd )
		{
			return;
		}

		XUI::CXUI_Wnd* pkTitleBg = pkWnd->GetControl(L"SFRM_TITLE_BG");
		if( pkTitleBg )
		{
			// 제목
			XUI::CXUI_Wnd* pkRoomTitle = pkTitleBg->GetControl(L"FRM_ROOM_TITLE");
			if( pkRoomTitle )
			{
				pkRoomTitle->Text(kChatRoom.kRoomTitle);
			}
			// 공개/미공개
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( pkPlayer )
			{
				SHomeChatGuest* pkGuest = g_kHomeRenewMgr.FindMyhomeChatRoomGuest(kChatRoom, pkPlayer->GetID());
				if( pkGuest )
				{
					XUI::CXUI_Wnd* pkAuBox = pkTitleBg->GetControl(L"SFRM_AU_BOX");
					if( pkAuBox )
					{
						if( pkGuest->bRoomMaster )
						{
							pkAuBox->Visible(true);
							if( kChatRoom.kRoomPassword.empty() )
							{
								pkAuBox->Text(TTW(201751));
							}
							else
							{
								pkAuBox->Text(TTW(201752));
							}
						}
						else
						{
							pkAuBox->Visible(false);
						}
					}
				}
			}
		}
	}

	void OpenMyhomeChatRoomChat(SHomeChatRoom& kChatRoom)
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"SFRM_MYHOME_CHAT_ROOM_CHAT");
		if( !pkWnd )
		{
			return;
		}
		pkWnd->OwnerGuid(kChatRoom.kRoomGuid);
		
		SetMyhomeChatRoomChatTitleBG(kChatRoom);

		// 공지 세팅
		XUI::CXUI_Wnd* pkNotice = pkWnd->GetControl(L"SFRM_AU_BOX");
		if( pkNotice )
		{
			pkNotice->Text(kChatRoom.kRoomNoti);
		}
		// 내 정보 세팅
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}
		XUI::CXUI_Wnd* pkRight = pkWnd->GetControl(L"FRM_RIGHT_BG");
		if( !pkRight )
		{
			return;
		}
		XUI::CXUI_Image* pkMyClassImg = dynamic_cast<XUI::CXUI_Image*>(pkRight->GetControl(L"IMG_CLASS"));
		lwSetMiniClassIconIndex( pkMyClassImg, pkPlayer->GetAbil(AT_CLASS) );

		XUI::CXUI_Wnd* pkMyName = pkRight->GetControl(L"FRM_NAME");
		if( pkMyName )
		{
			BM::vstring strText(TTW(201766));
			strText.Replace(L"#MARK1#", pkPlayer->GetAbil(AT_LEVEL));
			strText.Replace(L"#MARK2#", pkPlayer->Name());
			pkMyName->Text(strText);
		}
		// 채팅참여자 세팅
		XUI::CXUI_Wnd* pkText2 = pkRight->GetControl(L"FRM_TEXT2");
		if( pkText2 )
		{
			BM::vstring strNum(kChatRoom.kContGuest.size());
			strNum += "/";
			strNum += kChatRoom.kMaxUserCount;
			BM::vstring strText(TTW(201769));
			strText.Replace(L"#MARK1#", strNum);
			pkText2->Text(strText);
		}
		XUI::CXUI_List* pkMemberList = dynamic_cast<XUI::CXUI_List*>(pkRight->GetControl(L"LST_MEMBER"));
		SetMyhomeChatMemberList(kChatRoom, pkMemberList);
	}

	void ModifyMyhomeChatRoomChat(SHomeChatRoom& kChatRoom)
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ROOM_CHAT");
		if( !pkWnd )
		{
			return;
		}
		pkWnd->OwnerGuid(kChatRoom.kRoomGuid);

		XUI::CXUI_Wnd* pkTitleBg = pkWnd->GetControl(L"SFRM_TITLE_BG");
		if( pkTitleBg )
		{
			// 제목
			XUI::CXUI_Wnd* pkRoomTitle = pkTitleBg->GetControl(L"FRM_ROOM_TITLE");
			if( pkRoomTitle )
			{
				pkRoomTitle->Text(kChatRoom.kRoomTitle);
			}
			// 공개/미공개
			SHomeChatGuest* pkGuest = g_kHomeRenewMgr.FindMyhomeChatRoomMaster(kChatRoom);
			if( pkGuest )
			{
				XUI::CXUI_Wnd* pkAuBox = pkTitleBg->GetControl(L"SFRM_AU_BOX");
				if( pkAuBox )
				{
					if( g_kPilotMan.IsMyPlayer( pkGuest->kID ) )
					{
						pkAuBox->Visible(true);
						if( kChatRoom.kRoomPassword.empty() )
						{
							pkAuBox->Text(TTW(201751));
						}
						else
						{
							pkAuBox->Text(TTW(201752));
						}
					}
					else
					{
						pkAuBox->Visible(false);
					}
				}
			}
		}
		// 공지 세팅
		XUI::CXUI_Wnd* pkNotice = pkWnd->GetControl(L"SFRM_AU_BOX");
		if( pkNotice )
		{
			pkNotice->Text(kChatRoom.kRoomNoti);
		}
		// 내 정보 세팅
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}
		XUI::CXUI_Wnd* pkRight = pkWnd->GetControl(L"FRM_RIGHT_BG");
		if( !pkRight )
		{
			return;
		}
		XUI::CXUI_Image* pkMyClassImg = dynamic_cast<XUI::CXUI_Image*>(pkRight->GetControl(L"IMG_CLASS"));
		lwSetMiniClassIconIndex( pkMyClassImg, pkPlayer->GetAbil(AT_CLASS) );

		XUI::CXUI_Wnd* pkMyName = pkRight->GetControl(L"FRM_NAME");
		if( pkMyName )
		{
			BM::vstring strText(TTW(201766));
			strText.Replace(L"#MARK1#", pkPlayer->GetAbil(AT_LEVEL));
			strText.Replace(L"#MARK2#", pkPlayer->Name());
			pkMyName->Text(strText);
		}
		// 채팅참여자 세팅
		XUI::CXUI_Wnd* pkText2 = pkRight->GetControl(L"FRM_TEXT2");
		if( pkText2 )
		{
			BM::vstring strNum(kChatRoom.kContGuest.size());
			strNum += "/";
			strNum += kChatRoom.kMaxUserCount;
			BM::vstring strText(TTW(201769));
			strText.Replace(L"#MARK1#", strNum);
			pkText2->Text(strText);
		}
		XUI::CXUI_List* pkMemberList = dynamic_cast<XUI::CXUI_List*>(pkRight->GetControl(L"LST_MEMBER"));
		SetMyhomeChatMemberList(kChatRoom, pkMemberList);
	}

	void MyhomeChatRoomChatChange(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkMake = XUIMgr.Call(L"SFRM_MYHOME_CHAT_MAKE");
		if( pkMake )
		{
			int i = 0;// modify
			pkMake->SetCustomData(&i, sizeof(i));

			BM::GUID kRoomGuid;
			if( FindMyhomeChatRoomGuid(kRoomGuid) )
			{
				SHomeChatRoom* pkRoom = g_kHomeRenewMgr.FindMyhomeChatRoom(kRoomGuid);
				if( pkRoom )
				{
					XUI::CXUI_Wnd* pkShd = pkMake->GetControl(L"SFRM_BODY_SHD");
					if( pkShd )
					{
						XUI::CXUI_CheckButton* pkBtn1 = dynamic_cast<XUI::CXUI_CheckButton*>(pkShd->GetControl(L"CBTN_BTN1"));//공개/비공개
						XUI::CXUI_CheckButton* pkBtn2 = dynamic_cast<XUI::CXUI_CheckButton*>(pkShd->GetControl(L"CBTN_BTN2"));
						if( pkBtn1 && pkBtn2 )
						{
							if( pkRoom->kRoomPassword.empty() )//공개
							{
								MyhomeChatMakePublicOpen(lwUIWnd(pkMake), true);
							}
							else
							{
								MyhomeChatMakePublicOpen(lwUIWnd(pkMake), false);
							}
						}
						XUI::CXUI_Wnd* pkTitle = pkShd->GetControl(L"SFRM_TEXT1");
						if( pkTitle )
						{
							XUI::CXUI_Edit* pkEdit = dynamic_cast<XUI::CXUI_Edit*>(pkTitle->GetControl(L"EDT_TITLE"));// 타이틀
							{
								pkEdit->EditText(pkRoom->kRoomTitle);
							}
						}
						XUI::CXUI_Wnd* pkPW = pkShd->GetControl(L"SFRM_TEXT4");
						if( pkPW )
						{
							XUI::CXUI_Edit* pkEdit = dynamic_cast<XUI::CXUI_Edit*>(pkPW->GetControl(L"EDT_PW"));//패스워드
							{
								pkEdit->EditText(pkRoom->kRoomPassword);
							}
						}
					}
					XUI::CXUI_Wnd* pkShd2 = pkMake->GetControl(L"SFRM_BODY_SHD2");
					if( pkShd2 )
					{
						XUI::CXUI_CheckButton* pkChkNotice = dynamic_cast<XUI::CXUI_CheckButton*>(pkShd2->GetControl(L"CBTN_NOTICE"));
						if( pkChkNotice )
						{
							if( pkRoom->kRoomNoti.empty() )
							{
								pkChkNotice->Check(false);
							}
							else
							{
								pkChkNotice->Check(true);
							}
						}

						XUI::CXUI_Wnd* pkNotice = pkShd2->GetControl(L"SFRM_TEXT");
						if( pkNotice )
						{
							XUI::CXUI_Edit* pkEdit = dynamic_cast<XUI::CXUI_Edit*>(pkNotice->GetControl(L"EDT_NOTICE"));
							{
								pkEdit->EditText(pkRoom->kRoomNoti);
							}
						}
					}
				}
			}
		}
	}

	void MyhomeChatEnablePrivateMsg(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}

		XUI::CXUI_CheckButton* pkCBtn1 = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_BTN1"));
		//XUI::CXUI_CheckButton* pkCBtn2 = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_BTN2"));
		if( !pkCBtn1 )
		{
			return;
		}

		bool bCheck1 = pkCBtn1->Check();
		//bool bCheck2 = !pkCBtn2->Check();
		bool bCheck2 = false;

		PgMyHome* pkHome = g_kHomeRenewMgr.GetCurrentHomeUnit();
		if( !pkHome )
		{
			return;
		}

		g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_MODIFY_GUEST(pkHome->GetID(), bCheck2, bCheck1);
	}

	// SFRM_MYHOME_CHAT_ALL_CHAT
	void OpenMyhomeChatAllChat(SHomeChatRoom& kChatRoom)
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"SFRM_MYHOME_CHAT_ALL_CHAT");
		if( !pkWnd )
		{
			return;
		}
		pkWnd->OwnerGuid(kChatRoom.kRoomGuid);
		// 내 정보 세팅
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}
		XUI::CXUI_Wnd* pkRight = pkWnd->GetControl(L"FRM_RIGHT_BG");
		if( !pkRight )
		{
			return;
		}
		{// 직업 아이콘 설정
			XUI::CXUI_Image* pkMyClassImg = dynamic_cast<XUI::CXUI_Image*>(pkRight->GetControl(L"IMG_CLASS"));
			lwSetMiniClassIconIndex( pkMyClassImg, pkPlayer->GetAbil(AT_CLASS) );
		}

		XUI::CXUI_Wnd* pkMyName = pkRight->GetControl(L"FRM_NAME");
		if( pkMyName )
		{
			BM::vstring strText(TTW(201766));
			strText.Replace(L"#MARK1#", pkPlayer->GetAbil(AT_LEVEL));
			strText.Replace(L"#MARK2#", pkPlayer->Name());
			pkMyName->Text(strText);
		}
		// 채팅참여자 세팅
		XUI::CXUI_Wnd* pkText2 = pkRight->GetControl(L"FRM_TEXT2");
		if( pkText2 )
		{
			BM::vstring strText(TTW(201769));
			strText.Replace(L"#MARK1#", kChatRoom.kContGuest.size());
			pkText2->Text(strText);
		}
		XUI::CXUI_List* pkMemberList = dynamic_cast<XUI::CXUI_List*>(pkRight->GetControl(L"LST_MEMBER"));
		SetMyhomeChatMemberList(kChatRoom, pkMemberList);
	}

	void SetMyhomeChatMemberList(SHomeChatRoom& kChatRoom)
	{
		bool bPublic = g_kHomeRenewMgr.IsPublicChatRoom();
		XUI::CXUI_Wnd* pkWnd;
		if( bPublic )
		{
			pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ALL_CHAT");
			if( !pkWnd )
			{
				pkWnd = XUIMgr.Call(L"SFRM_MYHOME_CHAT_ALL_CHAT");
			}
		}
		else
		{
			pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ROOM_CHAT");
			if( !pkWnd )
			{
				pkWnd = XUIMgr.Call(L"SFRM_MYHOME_CHAT_ROOM_CHAT");
			}
		}
		if( !pkWnd )
		{
			return;
		}
		pkWnd->OwnerGuid(kChatRoom.kRoomGuid);
		// 공지 세팅
		XUI::CXUI_Wnd* pkNotice = pkWnd->GetControl(L"SFRM_AU_BOX");
		if( pkNotice )
		{
			pkNotice->Text(kChatRoom.kRoomNoti);
		}
		// 내 정보 세팅
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}
		XUI::CXUI_Wnd* pkRight = pkWnd->GetControl(L"FRM_RIGHT_BG");
		if( !pkRight )
		{
			return;
		}
		{// 직업 아이콘 설정
			XUI::CXUI_Image* pkMyClassImg = dynamic_cast<XUI::CXUI_Image*>(pkRight->GetControl(L"IMG_CLASS"));
			lwSetMiniClassIconIndex( pkMyClassImg, pkPlayer->GetAbil(AT_CLASS) );
		}

		XUI::CXUI_Wnd* pkMyName = pkRight->GetControl(L"FRM_NAME");
		if( pkMyName )
		{
			BM::vstring strText(TTW(201766));
			strText.Replace(L"#MARK1#", pkPlayer->GetAbil(AT_LEVEL));
			strText.Replace(L"#MARK2#", pkPlayer->Name());
			pkMyName->Text(strText);
		}
		// 채팅참여자 세팅
		XUI::CXUI_Wnd* pkText2 = pkRight->GetControl(L"FRM_TEXT2");
		if( pkText2 )
		{
			if( bPublic )
			{
				BM::vstring strText(TTW(201769));
				strText.Replace(L"#MARK1#", kChatRoom.kContGuest.size());
				pkText2->Text(strText);
			}
			else
			{
				BM::vstring strNum(kChatRoom.kContGuest.size());
				strNum += "/";
				strNum += kChatRoom.kMaxUserCount;
				BM::vstring strText(TTW(201769));
				strText.Replace(L"#MARK1#", strNum);
				pkText2->Text(strText);
			}
		}

		XUI::CXUI_List* pkMemberList = dynamic_cast<XUI::CXUI_List*>(pkRight->GetControl(L"LST_MEMBER"));
		SetMyhomeChatMemberList(kChatRoom, pkMemberList);
	}

	void SetMyhomeChatMemberList(SHomeChatRoom& kChatRoom, XUI::CXUI_List* pkMemberList)
	{
		if( pkMemberList )
		{
			pkMemberList->ClearList();

			CONT_MYHOMECHATGUEST::iterator itr = kChatRoom.kContGuest.begin();
			for( ; itr != kChatRoom.kContGuest.end(); ++itr )
			{
				SHomeChatGuest& kChatGuest = itr->second;
				XUI::SListItem* pkListItem = pkMemberList->AddItem(L"");
				if( pkListItem && pkListItem->m_pWnd )
				{
					pkListItem->m_pWnd->OwnerGuid(kChatGuest.kID);

					XUI::CXUI_Wnd* pkRoomMaster = pkListItem->m_pWnd->GetControl(L"IMG_ROOM_MASTER_BG");
					if( pkRoomMaster )
					{
						if( kChatGuest.bRoomMaster )
						{
							pkRoomMaster->Visible(true);
						}
						else
						{
							pkRoomMaster->Visible(false);
						}
					}
					XUI::CXUI_Image* pkClassImg = dynamic_cast<XUI::CXUI_Image*>(pkListItem->m_pWnd->GetControl(L"IMG_ICON_BG"));
					if( pkClassImg )
					{
						SUVInfo kUVInfo = pkClassImg->UVInfo();
						kUVInfo.Index = kChatGuest.iClass;
						pkClassImg->UVInfo(kUVInfo);
					}
					XUI::CXUI_Wnd* pkName = pkListItem->m_pWnd->GetControl(L"FRM_NAME");
					if( pkName )
					{
						BM::vstring strName(kChatGuest.kName);
						if( kChatGuest.bNotStanding)
						{
							strName += TTW(201796);
						}
						pkName->Text(strName);
					}
				}
			}
		}
	}

	void AddMyhomeChatAllChatContent(SChatLog& kChatLog, bool bSystem)
	{
		//g_kHomeRenewMgr.AddHomeChatLog(kChatLog);

		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ALL_CHAT");
		if( !pkWnd )
		{
			pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ROOM_CHAT");
			if( !pkWnd )
			{
				return;
			}
		}
		XUI::CXUI_Wnd* pkShd = pkWnd->GetControl(L"SFRM_BODY_SHD");
		if( !pkShd )
		{
			return;
		}
		XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkShd->GetControl(L"LST_CHAT"));
		if( !pkList )
		{
			return;
		}

		XUI::SListItem* pkListItem = pkList->AddItem(L"");
		if( pkListItem && pkListItem->m_pWnd )
		{
			XUI::CXUI_Image* pkClassImg = dynamic_cast<XUI::CXUI_Image*>(pkListItem->m_pWnd->GetControl(L"IMG_CLASS_BG"));
			if( pkClassImg )
			{
				int iClass = 0;
				SHomeChatGuest* pkChatGuest = g_kHomeRenewMgr.FindMyhomeChatRoomGuest(kChatLog.kCharGUID);
				if( pkChatGuest )
				{
					iClass = pkChatGuest->iClass;
				}
				if( false == lwSetMiniClassIconIndex(pkClassImg, iClass) )
				{// 직업 아이콘 설정
					pkClassImg->Visible(false);
				}
			}
			XUI::CXUI_Image* pkGenderImg = dynamic_cast<XUI::CXUI_Image*>(pkListItem->m_pWnd->GetControl(L"IMG_GENDER_BG"));
			if( pkGenderImg )
			{
				int iGender = 0;
				SHomeChatGuest* pkChatGuest = g_kHomeRenewMgr.FindMyhomeChatRoomGuest(kChatLog.kCharGUID);
				if( pkChatGuest )
				{
					iGender = pkChatGuest->iGender;
				}
				if( iGender )
				{
					pkGenderImg->Visible(true);
					SUVInfo kUVInfo = pkGenderImg->UVInfo();
					kUVInfo.Index = iGender;
					pkGenderImg->UVInfo(kUVInfo);
				}
				else
				{
					pkGenderImg->Visible(false);
				}
			}
			int iItemHeight = 0;
			XUI::CXUI_Wnd* pkName = pkListItem->m_pWnd->GetControl(L"FRM_TEXT");
			if( pkName )
			{
				BM::vstring strText(PgChatMgrUtil::ColorHexToString(kChatLog.dwColor));
				if( !bSystem )
				{
					strText += kChatLog.kCharName;
					strText += " : ";
				}
				strText += kChatLog.kContents;
				pkName->Text(strText);
				int iMinHeight = 25;
				int iDefultY = 4;
				iItemHeight = pkName->TextRect().y;
				if( iItemHeight < iMinHeight )
				{
					iItemHeight = iMinHeight;
					XUI::CXUI_Wnd* pkText = pkListItem->m_pWnd->GetControl(L"FRM_TEXT");
					if( pkText )
					{
						POINT3I ptLoc = pkText->Location();
						ptLoc.y = iDefultY;
						pkText->Location(ptLoc);
					}
				}
			}

			POINT2 ptSize = pkListItem->m_pWnd->Size();
			ptSize.y = iItemHeight;
			pkListItem->m_pWnd->Size(ptSize);
		}

		while( MAX_CHAT_COUNT < pkList->GetTotalItemCount() )//갯수 초과 삭제
		{
			XUI::SListItem *pkFirstItem = pkList->FirstItem();//새로운 개체가 아래에서 붙는 방식이기 때문.
			if( pkFirstItem )
			{
				pkList->DeleteItem(pkFirstItem);
			}
			else
			{
				//pkFirstItem 없을 수는 없지만.
				assert(NULL);
				break;
			}
		}

		//Adjust List Scroll Position
		pkList->DisplayStartPos(pkList->GetTotalItemLen() - pkList->Height());
		pkList->AdjustMiddleBtnPos();
	}

	void MyhomeChatSend(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}
		XUI::CXUI_Wnd* pkShd2 = pkWnd->GetControl(L"SFRM_BODY_SHD2");
		if( !pkShd2 )
		{
			return;
		}
		XUI::CXUI_Edit_MultiLine* pkEdit = dynamic_cast<XUI::CXUI_Edit_MultiLine*>(pkShd2->GetControl(L"MEDT_COMMENT_WRITE"));
		if( !pkEdit )
		{
			return;
		}

		XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(FONT_CHAT);
		if( pFont )
		{
			pkEdit->SetEditFocus(true);
			std::wstring strText = pkEdit->XUI::CXUI_Edit::EditText();
			if( strText.empty() )// 비었을때
			{
				return;
			}

			while( strText[0] == 13 && strText[1] == 10)
			{
				if( strText.size() == 2)// 엔터 한번일때
				{
					pkEdit->EditText(L"");
					return;
				}
				else if( strText.size() > 2)// 앞쪽 엔터 제거
				{
					strText = strText.substr(2, strText.size() - 2);
				}
			}

			g_kChatMgrClient.InputChatLog_Add(strText);
			if( !g_kChatMgrClient.CheckSpamChat(strText) )
			{
				XUI::CXUI_Style_String	kText(XUI::PgFontDef(pFont, g_kHomeRenewMgr.GetChatFontColor()), strText);
				g_kChatMgrClient.SendChat_Message(CT_MYHOME_PUBLIC, kText);
			}
			pkEdit->EditText(L"");
		}
	}

	void MyhomeChatNotStanding(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd )
		{
			return;
		}

		XUI::CXUI_CheckButton* pkCBtn1 = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_BTN1"));
		//XUI::CXUI_CheckButton* pkCBtn2 = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_BTN2"));
		if( !pkCBtn1 )
		{
			return;
		}

		bool bCheck1 = !pkCBtn1->Check();
		//bool bCheck2 = pkCBtn2->Check();
		bool bCheck2 = false;

		PgMyHome* pkHome = g_kHomeRenewMgr.GetCurrentHomeUnit();
		if( !pkHome )
		{
			return;
		}

		g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_MODIFY_GUEST(pkHome->GetID(), bCheck2, bCheck1);
	}

	void MyhomeChatClean(lwUIWnd kWnd)
	{
		lwCallYesNoMsgBox(TTW(201778), BM::GUID::NullData(), MBT_MYHOME_CHAT_CLEAN);
	}

	void ExcuteMyhomeChatClean()
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ALL_CHAT");
		if( !pkWnd )
		{
			pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ROOM_CHAT");
			if( !pkWnd )
			{
				return;
			}
		}
		XUI::CXUI_Wnd* pkShd = pkWnd->GetControl(L"SFRM_BODY_SHD");
		if( !pkShd )
		{
			return;
		}
		XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkShd->GetControl(L"LST_CHAT"));
		if( !pkList )
		{
			return;
		}

		pkList->ClearList();
	}

	void MyhomeChatExit(lwUIWnd kWnd)
	{
		unsigned long ulTT = 201777;
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			SHomeChatGuest* pkGuest = g_kHomeRenewMgr.FindMyhomeChatRoomGuest(pkPlayer->GetID());
			if( pkGuest )
			{
				if( pkGuest->bRoomMaster )
				{
					ulTT = 201802;
					SHomeChatRoom* pkRoom = g_kHomeRenewMgr.FindMyhomeChatRoombyGuest(pkPlayer->GetID());
					if( pkRoom->kContGuest.size() == 1 )
					{
						ulTT = 201806;
					}
				}
			}
		}
		lwCallYesNoMsgBox(TTW(ulTT), BM::GUID::NullData(), MBT_MYHOME_CHAT_EXIT);
	}

	void ExcuteMyhomeChatExit()
	{
		PgMyHome* pkHome = g_kHomeRenewMgr.GetCurrentHomeUnit();
		if( pkHome )
		{
			g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_EXIT(pkHome->GetID());
		}
	}

	void SetMyhomeChatFontColor(unsigned int iColor)
	{
		g_kHomeRenewMgr.SetChatFontColor(iColor);
	}

	unsigned int GetMyhomeChatFontColor()
	{
		return g_kHomeRenewMgr.GetChatFontColor();
	}

	bool FindMyhomeChatRoomGuid(BM::GUID& kRoomGuid)
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ALL_CHAT");
		if( pkWnd )
		{
			kRoomGuid = pkWnd->OwnerGuid();
			return true;
		}

		pkWnd = XUIMgr.Get(L"SFRM_MYHOME_CHAT_ROOM_CHAT");
		if( !pkWnd )
		{
			return false;
		}

		kRoomGuid = pkWnd->OwnerGuid();
		return true;
	}

	void PopupMyhomeChatWhisper(lwGUID kGuid)
	{
		SHomeChatGuest* pkGuest = g_kHomeRenewMgr.FindMyhomeChatRoomGuest(kGuid());
		if( !pkGuest )
		{
			return;
		}
		BM::vstring strText(L"/MODE 4 ");
		strText += pkGuest->kName;
		g_kChatMgrClient.CheckChatCommand(strText);
	}

	void PopupMyhomeChatAddFriend(lwGUID kGuid)
	{
	}

	void PopupMyhomeChatInvitation(lwGUID kGuid)
	{
		SHomeChatGuest* pkGuest = g_kHomeRenewMgr.FindMyhomeChatRoomGuest(kGuid());
		if( !pkGuest )
		{
			return;
		}
		
		SendMyhomeInvite(pkGuest->kName);
	}

	void PopupMyhomeChatRoomMaster(lwGUID kGuid)
	{
		if( g_kPilotMan.IsMyPlayer(kGuid()) )
		{
			return;
		}

		PgMyHome* pkHome = g_kHomeRenewMgr.GetCurrentHomeUnit();
		if( !pkHome )
		{
			return;
		}

		g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_SET_ROOMMASTER(pkHome->GetID(), kGuid());
	}

	void PopupMyhomeChatRoomListWhisper(lwGUID kGuid)
	{
		SHomeChatRoom* pkRoom = g_kHomeRenewMgr.FindMyhomeChatRoom(kGuid());
		if( !pkRoom )
		{
			return;
		}

		SHomeChatGuest* pkGuest = g_kHomeRenewMgr.FindMyhomeChatRoomMaster(*pkRoom);
		if( !pkGuest )
		{
			return;
		}

		BM::vstring strText(L"/MODE 4 ");
		strText += pkGuest->kName;
		g_kChatMgrClient.CheckChatCommand(strText);
	}

	void PopupMyhomeChatRoomListAddFriend(lwGUID kGuid)
	{
		SHomeChatRoom* pkRoom = g_kHomeRenewMgr.FindMyhomeChatRoom(kGuid());
		if( !pkRoom )
		{
			return;
		}

		SHomeChatGuest* pkGuest = g_kHomeRenewMgr.FindMyhomeChatRoomMaster(*pkRoom);
		if( !pkGuest )
		{
			return;
		}

		SFriendItem kFriendItem;
		kFriendItem.CharGuid(pkGuest->kID);
		g_kFriendMgr.SendFriend_Command(FCT_ADD_BYGUID, kFriendItem);
	}

	void PopupMyhomeChatKickGuest(lwGUID kGuid)
	{
		PgMyHome* pkHomeUnit = g_kHomeRenewMgr.GetCurrentHomeUnit();
		if( !pkHomeUnit )
		{
			return;
		}

		g_kHomeRenewMgr.Send_PT_C_M_REQ_MYHOME_CHAT_KICK_GEUST(pkHomeUnit->GetID(), kGuid());
	}

	void CallMyhomeEmoticonUI()
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"FRM_MYHOME_EMOTICON");
		if( !pMainUI || pMainUI->IsClosed() )
		{
			pMainUI = XUIMgr.Call(L"FRM_MYHOME_EMOTICON");
			if( !pMainUI ){ return; }

			XUI::CXUI_Wnd* pSub = pMainUI->GetControl(L"FRM_SUB_NOVEL");
			if( pSub )
			{
				SetMyhomeEmotionPage(pSub, 0);
			}
		}
		else
		{
			pMainUI->Close();
		}
	}

	void SetMyhomeEmotionPage(XUI::CXUI_Wnd* pPage, int const iGroup)
	{
		if( !pPage ){ return; }

		XUI::CXUI_Wnd* pSubTypeText = pPage->GetControl(L"FRM_SUB_TYPE_TEXT");
		if( pSubTypeText )
		{
			::CONT_EMOTION_GROUP const* pContEmotiGroup = NULL;
			g_kTblDataMgr.GetContDef(pContEmotiGroup);	//맵 데프
			if( pContEmotiGroup )
			{
				CONT_EMOTION_GROUP::const_iterator def_iter = pContEmotiGroup->find(SEMOTIONGROUPKEY(ET_EMOTICON, iGroup));
				if( def_iter != pContEmotiGroup->end() )
				{
					wchar_t const* szName = NULL;
					if( GetDefString(def_iter->second, szName) )
					{
						pSubTypeText->Text(szName);
					}
					else
					{
						pSubTypeText->Text(L"");
					}
				}
			}
		}

		PgEmotiMgr::CONT_IDARRAY	kCont;
		if( g_kPgEmotiMgr.GetEmotiGroupItem(ET_EMOTICON, iGroup, kCont) )
		{
			PgEmotiMgr::CONT_IDARRAY::iterator iter = kCont.begin();
			for(int i = 0; i < MAX_EMOTI_ITEM_SLOT; ++i)
			{
				BM::vstring	vStr(L"FRM_ICON");
				vStr += i;
				XUI::CXUI_Wnd* pIcon = pPage->GetControl(vStr);
				if( pIcon )
				{
					pIcon->DefaultImgName(L"");
					if( iter != kCont.end() )
					{
						BM::vstring	TextStr(L"#");
						TextStr += (*iter);
						pIcon->Text(g_kChatMgrClient.ConvertUserCommand(TextStr));
						pIcon->SetCustomData(&(*iter), sizeof((*iter)));
						++iter;
					}
					else
					{
						pIcon->Text(L"");
						pIcon->ClearCustomData();
					}
				}
			}
		}
	}

	void MyhomeSelectEmotiItem(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( !pSelf ){ return; }

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent ){ return; }

		int const iEmotiID = lwUIWnd(pSelf).GetCustomData<int>();
		if( iEmotiID == 0 )
		{
			return;
		}

		lua_tinker::call<void, int>("AddMyhomeEmoticon", iEmotiID);
	}

	void Test_AddMyhomeChatPopupList(char const * szTitle, bool bNotCall)
	{
		g_kHomeRenewMgr.Test_AddMyhomeChatPopupList(static_cast<std::wstring>(BM::vstring(szTitle)));
		!bNotCall ? OpenMyhomeChatRoomList() : 0;
	}
};