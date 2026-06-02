#ifndef FREEDOM_DRAGONICA_CONTENTS_MYHOME_PGHOMERENEW_H
#define FREEDOM_DRAGONICA_CONTENTS_MYHOME_PGHOMERENEW_H

#include "PgPage.h"
#include "Lohengrin/packetstruct.h"
#include "PgChatMgrClient.h"

namespace NewWare { namespace Renderer { class TextBoardTextures; } }

typedef struct tagHOME_BOARD_KEY
{
	tagHOME_BOARD_KEY() : sFirst(0), sSecond(0)
	{
	};

	tagHOME_BOARD_KEY(short _sFirst, short _sSecond) : sFirst(_sFirst), sSecond(_sSecond)
	{
	};

	bool operator<(tagHOME_BOARD_KEY const& rhs) const;

	short sFirst;
	int sSecond;
}SHomeBoardKey;
typedef std::map<SHomeBoardKey, std::wstring> ContHomeBoardMap;
typedef std::vector< SChatLog > ContHomeChatLog;

class lwUIWnd;
class lwGUID;
class PgHomeRenew
{
public:
	PgHomeRenew();
	~PgHomeRenew();

	void Build();
	void InitHomeBoard();
	void SetHomeBoard();
	void Update(float fAccumTime, float fFrameTime);
    void Draw( PgRenderer* pkRenderer, float fFrameTime );
	
	void SetNameforTownBoard(NiAVObject* pkMeshRoot, std::string const& rkNodeName, std::wstring const& rkOwnerName, NiColorA const& rkTextColor);
	void SetBoardNameTag(int iBoardNo, int iPos, std::wstring const& strName);

	void SetMyhomeEnterPos(int iPos) { m_iMyhomeEnterPos = iPos; }
	int GetMyhomeEnterPos() { return m_iMyhomeEnterPos; }

	void SetMyhomeEnterPageMaxItem(int iCount);
	PgPage& GetMyhomeEnterPage() { return m_kMyhomEnterPage; }
	void SetMyhomeEnterList(CONT_MYHOME const& kHomeList);
	void GetMyhomeEnterDisplayItem(CONT_MYHOME& kHomeList);

	PgPage& GetMyhomeVisitLogPage() { return m_kMyhomeVisitLogPage; }
	void GetMyhomeVisitLogDisplayItem(CONT_HOMEVISITLOG& kList);

	bool IsMyhomeApt(int iBuildingNum);
	DWORD GetVisitlogAddTime() { return m_dwVisitlogAddTime; }

	bool CheckErrorMsg(HRESULT kResult, bool bShowMessageBox = true);
	bool ProcessMsg(unsigned short const usType, BM::Stream& rkPacket);

	PgPage& GetMyhomeChatRoomPage() { return m_kMyhomeChatRoomPage; }
	void SetMyhomeChatRoomPageMaxItem(int iCount);
	
	void SetMyhomeChatRoomList(CONT_MYHOMECHATROOM const& kChatRoomList);
	void GetMyhomeChatRoomDisplayItem(CONT_MYHOMECHATROOM& kChatRoom);
	void ModifyMyhomeChatRoom(SHomeChatRoom const& kChatRoom);
	
	bool ModifyMyhomeChatRoomGuest(BM::GUID const& kGuid, SHomeChatGuest const& _kGuest);
	bool AddMyhomeChatRoomGuest(BM::GUID const& kRoomGuid, SHomeChatGuest const& kGuest);
	bool DeleteMyhomeChatRoomGuest(BM::GUID const& kRoomGuid, BM::GUID const& kGuestID);

	SHomeChatRoom* FindMyhomeChatRoom(BM::GUID const& kRoomGuid);
	SHomeChatRoom* FindMyhomeChatRoombyGuest(BM::GUID const& kGuestGuid);
	SHomeChatGuest* FindMyhomeChatRoomMaster(SHomeChatRoom& kChatRoom);
	SHomeChatGuest* FindMyhomeChatRoomGuest(SHomeChatRoom& kChatRoom, BM::GUID const& kGuestGuid);
	SHomeChatGuest* FindMyhomeChatRoomGuest(BM::GUID const& kGuestGuid);

	ContHomeChatLog& GetHomeChatLog() { return m_kHomeChatLog; }
	void AddHomeChatLog(SChatLog& kChatLog);

	void SetChatFontColor(unsigned int iColor) { m_iChatFontColor = iColor; }
	unsigned int GetChatFontColor() { return m_iChatFontColor; }

	void SetPublicChatRoom( bool bPublic ) { m_bPublicChatRoom = bPublic; }
	bool IsPublicChatRoom() { return m_bPublicChatRoom; }

	void SetPublicChstRoomInfo( SHomeChatRoom const& kInfo ) { m_kPublicChatRoomInfo = kInfo; }
	SHomeChatRoom* GetPublicChstRoomInfo() { return& m_kPublicChatRoomInfo; }

	PgMyHome* PgHomeRenew::GetCurrentHomeUnit();

	void Send_PT_C_M_REQ_MYHOME_BUY(bool const bReqPrice, short sNum);
	void Send_PT_C_M_REQ_MYHOME_EXIT(BM::GUID const& rkGuid, bool bPrevPos);
	void Send_PT_C_M_REQ_EXCHANGE_LOGCOUNTTOEXP();
	void Send_PT_C_M_REQ_HOME_VISITLOG_ADD(BM::GUID const& rkGuid, std::wstring const& strText, CONT_ITEMPOS const& kPos, bool const bPrivate = false);
	void Send_PT_C_M_REQ_HOME_VISITLOG_LIST(BM::GUID const& rkHomeGuid);
	void Send_PT_C_M_REQ_HOME_VISITLOG_DELETE(BM::GUID const& rkHomeGuid, BM::GUID const& rkLogGuid);
	void Send_PT_C_M_REQ_MYHOME_SELL();

	void Recv_PT_M_C_ANS_MYHOME_BUY(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_MYHOME_EXIT(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_EXCHANGE_LOGCOUNTTOEXP(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_HOME_VISITLOG_ADD(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_HOME_VISITLOG_LIST(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_HOME_VISITLOG_DELETE(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_MYHOME_SELL(BM::Stream& rkPacket);

	// 마이홈 채팅
	void Send_PT_C_M_REQ_MYHOME_CHAT_ENTER(BM::GUID const& rkHomeUnitGuid, BM::GUID const& rkHomeGuid, std::wstring const& strPW);// 채팅룸 참가 요청
	void Send_PT_C_M_REQ_MYHOME_CHAT_EXIT(BM::GUID const& rkHomeUnitGuid);// 채팅룸 나가기 
	void Send_PT_C_M_REQ_MYHOME_CHAT_MODIFY_ROOM(BM::GUID const& rkHomeUnitGuid, std::wstring const& strTitle, 
		std::wstring const& strNotice, std::wstring const& strPW, int iNum);// 채팅룸 정보 수정 요청
	void Send_PT_C_M_REQ_MYHOME_CHAT_MODIFY_GUEST(BM::GUID const& rkHomeUnitGuid, bool bEnablePrivateMsg, bool bNotStanding);// 채팅 참가자 정보 변경 ( 자신 정보 변경 )
	void Send_PT_C_M_REQ_MYHOME_CHAT_ROOM_LIST(BM::GUID const& rkHomeUnitGuid);// 채팅룸 리스트 요청
	void Send_PT_C_M_REQ_MYHOME_CHAT_ROOM_CREATE(BM::GUID const& rkHomeUnitGuid, std::wstring const& strTitle, 
		std::wstring const& strNotice, std::wstring const& strPW, int iNum);// 채팅룸 생성 요청
	void Send_PT_C_M_REQ_MYHOME_CHAT_SET_ROOMMASTER(BM::GUID const& kHomeUnitGuid, BM::GUID const& kTargetGuid);
	void Send_PT_C_M_REQ_MYHOME_CHAT_KICK_GEUST(BM::GUID const& kHomeUnitGuid, BM::GUID const& kTargetGuid);

	void Recv_PT_M_C_ANS_MYHOME_CHAT_ENTER(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_MYHOME_CHAT_EXIT(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_MYHOME_CHAT_MODIFY_ROOM(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_MYHOME_CHAT_MODIFY_GUEST(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_MYHOME_CHAT_ROOM_LIST(BM::Stream& rkPacket);
	void Recv_PT_M_C_NOTI_MYHOME_CHAT_ROOM_ENTER(BM::Stream& rkPacket);
	void Recv_PT_M_C_NOTI_MYHOME_CHAT_ROOM_EXIT(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_MYHOME_CHAT_ROOM_CREATE(BM::Stream& rkPacket);
	void Recv_PT_M_C_NOTI_MYHOME_CHAT_ROOM_MODIFY(BM::Stream& rkPacket);
	void Recv_PT_M_C_NOTI_MYHOME_CHAT_GUEST_MODIFY(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_MYHOME_CHAT_SET_ROOMMASTER(BM::Stream& rkPacket);
	void Recv_PT_M_C_NOTI_MYHOME_CHAT_RESULT(BM::Stream& rkPacket);
	void Recv_PT_M_C_ANS_MYHOME_CHAT_KICK_GEUST(BM::Stream& rkPacket);
	void Recv_PT_M_C_NOTI_MYHOME_CHAT_KICK_GEUST(BM::Stream& rkPacket);

	// Test
	void Test_AddMyhomeChatPopupList(std::wstring const & rkTitle);

protected:
	ContHomeBoardMap m_kHomeBoard;
	float m_fDrawBoardDelay;
	int m_iMyhomeEnterPos;
	DWORD m_dwVisitlogAddTime;

	PgPage m_kMyhomEnterPage;
	CONT_MYHOME m_kMyhomeEnter;
	
	CONT_HOMEVISITLOG m_kMyhomeVisitLog;
	PgPage m_kMyhomeVisitLogPage;

	PgPage m_kMyhomeChatRoomPage;
	CONT_MYHOMECHATROOM m_kMyhomeChatRoom;

	SHomeChatRoom m_kPublicChatRoomInfo;

	ContHomeChatLog m_kHomeChatLog;

	int m_iChatFontColor;
	bool m_bPublicChatRoom;

    NewWare::Renderer::TextBoardTextures* m_pkTextBoardTextures;
};

#define g_kHomeRenewMgr SINGLETON_STATIC(PgHomeRenew)

namespace lwHomeRenew
{
	typedef struct tagHomeBuff
	{
		int iItemNo;
		int iType;
		int iLv;
		int iEffectNo;
		std::wstring strName;
	}SHomeBuff;
	typedef std::map<int,SHomeBuff> CONT_HOMEBUFF;

	void RegisterWrapper(lua_State *pkState);
	void DoHomeRenew();
	void OnRecvHomeInvite();
	void SetBoardNameTag(int iBoard, int iPos, char const* szName);
	bool IsMyhomeApt();
	// 마이 홈APT 입장
	void EnterMyhomeApt();
	// 마이홈 입장 창
	void OnMyhomeEnterMoveOther(lwUIWnd kWnd);
	void OnMyhomeEnterMoveMine(lwUIWnd kWnd);
	void SetMyHomeEnterPageControl(XUI::CXUI_Wnd* pPageMainUI);
	void OnClickMyhomeEnterJumpPrevPage(lwUIWnd kSelf);
	void OnClickMyhomeEnterJumpNextPage(lwUIWnd kSelf);
	void OnClickMyhomeEnterPage(lwUIWnd kSelf);
	// 방문자 리스트 창
	void OnMyhomeVisitListCall();
	void OpenMyhomeVisitListWnd();
	void OnMyhomeVisitListDelete(lwUIWnd kWnd);
	void OnMyhomeVisitListAddFrient(lwUIWnd kWnd);
	void OnMyhomeVisitListUseCountBtn();
	void SetMyhomeVisitLogSlotList(XUI::CXUI_Wnd* pMainUI);
	void SetMyhomeVisitLogPageControl(XUI::CXUI_Wnd* pPageMainUI);
	void OnClickMyhomeVisitLogJumpPrevPage(lwUIWnd kSelf);
	void OnClickMyhomeVisitLogJumpNextPage(lwUIWnd kSelf);
	void OnClickMyhomeVisitLogPage(lwUIWnd kSelf);
	// 방명록 카운트수 사용하기 창
	void OnSendExchangeLogCountToExp();
	// 방명록 쓰기 창
	void SendMyhomeVisitComment(lwUIWnd kWnd);
	bool ReserveMyhomeVisitComment();
	bool OnCallMyhomeVisitComment(SMYHOME const& kHome);
	// 마이홈 구입
	void OpenMyhomeBuy();
	void AnsOpenMyhomeBuy(int const iGold);
	void SendMyhomeBuy(lwUIWnd kWnd);
	// 마이홈에서 나가기
	void SendMyhomeExit(bool bPrevPos);
	// 마이홈에서 소켓 하기
	void OnClickMyHomesSocket();

	void OnSendMyhomeEnter(int iPos);
	void OpenMyhomeEnterWnd();
	// 마이홈 아파트 부활
	void OnMyhomeAptRelive(lwUIWnd kWnd);
	bool ReserveMyhomeAptRelive();
	bool OnCallMyhomeAptRelive(SMYHOME const& kHome);
	bool SendMyhomeAptRelive_Packet();
	// 마이홈 들어가기
	void SendEnterHome(short sStreet, int iHouse);
	// 초대장 발급
	void SendMyhomeInvite(std::wstring const& strName);
	void SendMyhomeInvitebyFriend(lwGUID kGuid);
	void SendMyhomeInvitebyGuild(lwGUID kGuid);
	void SendMyhomeInvitebyParty(lwGUID kGuid);
	void SendMyhomeInvitebyCouple();
	// 커뮤니티 창에서 마이홈 가기
	void SendMyhomeGobyFriend(lwGUID kGuid);
	void SendMyhomeGobyGuild(lwGUID kGuid);
	void SendMyhomeGobyParty(lwGUID kGuid);
	void SendMyhomeGobyCouple(lwGUID kGuid);
	bool IsHaveMyhomebyFriend(lwGUID kGuid);
	// 마이홈 버프 아이콘 창
	void OnMyhomeBuffDisplay();
	bool GetMyhomeBuff(PgMyHome* pkHomeUnit, CONT_HOMEBUFF& kBuff);
	// 마이홈 경매 게시판 미니맵
	void OnCallMyhomeDetailView(lwUIWnd kSelf);
	void OnCallMyhomeMinimap(short const siStreetNo, int const iHouseNo=0);
	void OnClickMinimapMyhomePopup(lwUIWnd kSelf);
	void OnClickMinimapMyhomePopupItem(lwUIWnd kSelf);
	// 마이홈 게시판 정보 요청
	void OnSendBuild_HomeList();
	void SetBuildNameBoard();
	void SetBuildNameBoardforOptionChange();
	// 초대장 수정
	void OnCallMyhomeInviteLetter();
	// 마이홈 퇴거
	void OnCallMyhomeSellCommonMessageBox(lwUIWnd kWnd);

	///////////////////////////////////////////////////
	// 마이홈 채팅
	///////////////////////////////////////////////////
	bool IsInsideHome();
	void SetChatHeaderName();
	bool IsMyRoomMasterChatRoom();
	// 마이홈 채팅 SFRM_MYHOME_CHAT_POPUP
	void OpenMyhomeChatPopup();
	void MyhomeChatPopupEnter(lwUIWnd kWnd);
	void MyhomeChatPopupList(lwUIWnd kWnd);
	// 채팅방 만들기 SFRM_MYHOME_CHAT_MAKE
	void MyhomeChatMakeMemberDropDown(lwUIWnd kWnd);
	void MyhomeChatMakeDropDownItem(lwUIWnd kWnd);
	void MyhomeChatMakePublicOpen(lwUIWnd kWnd, bool bPublic);
	void MyhomeChatMakeNotice(lwUIWnd kWnd);
	void MyhomeChatMakeOK(lwUIWnd kWnd);
	// 채팅방 리스트 SFRM_MYHOME_CHAT_ROOM_LIST
	void OpenMyhomeChatRoomList();
	void SetMyhomeChatRoomList(XUI::CXUI_Wnd* pMainUI);
	void SetMyhomeChatRoomSlot(XUI::CXUI_Wnd* pSlot, SHomeChatRoom& kChatRoom, int iNum);
	void MyhomeChatRoomListEnter(lwUIWnd kWnd);
	void MyhomeChatRoomListMsgBoxOK(lwUIWnd kWnd);
	void SetMyhomeChatRoomListPageControl(XUI::CXUI_Wnd* pPageMainUI);
	void MyhomeChatRoomListPageFirst(lwUIWnd kWnd);
	void MyhomeChatRoomListPageJumpPrev(lwUIWnd kWnd);
	void MyhomeChatRoomListPagePrev(lwUIWnd kWnd);
	void MyhomeChatRoomListPageNext(lwUIWnd kWnd);
	void MyhomeChatRoomListPageJumpNext(lwUIWnd kWnd);
	void MyhomeChatRoomListPageEnd(lwUIWnd kWnd);
	void MyhomeChatRoomListPageNum(lwUIWnd kWnd);
	void MyhomeChatRoomListMake(lwUIWnd kWnd);
	// SFRM_MYHOME_CHAT_ROOM_CHAT
	void OpenMyhomeChatRoomChat(SHomeChatRoom& kChatRoom);
	void SetMyhomeChatRoomChatTitleBG(SHomeChatRoom& kChatRoom);
	void ModifyMyhomeChatRoomChat(SHomeChatRoom& kChatRoom);
	void MyhomeChatRoomChatChange(lwUIWnd kWnd);
	// SFRM_MYHOME_CHAT_ALL_CHAT
	void OpenMyhomeChatAllChat(SHomeChatRoom& kChatRoom);
	// 채팅창 공통
	void MyhomeChatEnablePrivateMsg(lwUIWnd kWnd);
	void AddMyhomeChatAllChatContent(SChatLog& kChatLog, bool bSystem = false);
	void SetMyhomeChatMemberList(SHomeChatRoom& kChatRoom);
	void SetMyhomeChatMemberList(SHomeChatRoom& kChatRoom, XUI::CXUI_List* pkMemberList);
	void MyhomeChatSend(lwUIWnd kWnd);
	void MyhomeChatExit(lwUIWnd kWnd);
	void ExcuteMyhomeChatExit();
	void MyhomeChatNotStanding(lwUIWnd kWnd);
	void MyhomeChatClean(lwUIWnd kWnd);
	void ExcuteMyhomeChatClean();
	void SetMyhomeChatFontColor(unsigned int iColor);
	unsigned int GetMyhomeChatFontColor();
	bool FindMyhomeChatRoomGuid(BM::GUID& kRoomGuid);
	// 팝업 커맨드
	void PopupMyhomeChatWhisper(lwGUID kGuid);
	void PopupMyhomeChatAddFriend(lwGUID kGuid);
	void PopupMyhomeChatInvitation(lwGUID kGuid);
	void PopupMyhomeChatRoomMaster(lwGUID kGuid);
	void PopupMyhomeChatRoomListWhisper(lwGUID kGuid);
	void PopupMyhomeChatRoomListAddFriend(lwGUID kGuid);
	void PopupMyhomeChatKickGuest(lwGUID kGuid);
	// 이모티콘
	void CallMyhomeEmoticonUI();
	void SetMyhomeEmotionPage(XUI::CXUI_Wnd* pPage, int const iGroup);
	void MyhomeSelectEmotiItem(lwUIWnd UISelf);
}

#endif // FREEDOM_DRAGONICA_CONTENTS_MYHOME_PGHOMERENEW_H