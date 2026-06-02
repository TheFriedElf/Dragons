#ifndef FREEDOM_DRAGONICA_CONTENTS_CHAT_PGCHATMGRCLIENT_H
#define FREEDOM_DRAGONICA_CONTENTS_CHAT_PGCHATMGRCLIENT_H

#include <list>
#include <map>

#include "CreateUsingNiNew.inl"
#include "PgIXmlObject.h"
#include "XUI/XUI_Manager.h"

#define	MAX_WHISPER 5

namespace PgChatMgrUtil
{
	class CChatLogList
	{
	public:
		CChatLogList()
		{
			Clear();
		}

		~CChatLogList()
		{
		}

		bool CmpFilterFlag(DWORD const dwInFlag) const
		{
			if( ChatFilterFlag() & dwInFlag )
			{
				return true;
			}
			return false;
		}

		DWORD ApplyFlag(DWORD const dwInFlag)
		{
			m_kChatFilterFlag |= dwInFlag;
			return m_kChatFilterFlag;
		}
		
		DWORD DisApplyFlag(DWORD const dwInFlag)
		{
			m_kChatFilterFlag &= (~dwInFlag);
			return m_kChatFilterFlag;
		}

	protected:
		void Clear()
		{
			ChatFilterFlag(0);
			m_kWndName.clear();
		}

		CLASS_DECLARATION_S(DWORD, ChatFilterFlag);
		CLASS_DECLARATION_S(std::wstring, WndName);
	};

	void ChatFilterClear();
	void ChatFilterAdd(int const iFilterSet, BYTE const cChatMode, std::wstring const &rkXuiListID);
	void ChatFilterDel(int const iFilterSet, BYTE const cChatMode, std::wstring const &rkXuiListID);
	void NoticeLevelColor_Add(int const iLevel, DWORD const dwFontColor, DWORD const dwOutlineColor, bool const bShowBG);
	void NoticeLevelColor_Clear();

	std::wstring const ColorHexToString(DWORD const dwColor);
};

typedef enum eChatStation
{
	ECS_COMMON		= 0,
	ECS_PVPLOBBY	= 1,
	ECS_CASHSHOP	= 2,
	ECS_MYHOME		= 3,
	ECS_END,
}EChatStation;

typedef enum eSpamChkState
{
	ESCS_ChainInput		= 1,
	ESCS_LooseChainInput= 2,
	ESCS_MacroInput		= 4, 
	ESCS_SameInput		= 8,
	ESCS_MAX_FLAG = ESCS_ChainInput|ESCS_LooseChainInput|ESCS_MacroInput|ESCS_SameInput,
}ESpamChkState;

namespace ChatInitUtil
{
	typedef struct tagChatLoc
	{
		int iX, iY, iWidth, iHeight;
	}SCHAT_LOC;
	
	void ResizeChatWindow(int const iX, int const iY, int const iWidth, int const iHeight, bool bChangeLoc = true);
};

extern ChatInitUtil::SCHAT_LOC	kChatLoc[ECS_END];

typedef struct tagChatLog//채팅 로그
{
	tagChatLog()
	{
		Clear();
	}

	tagChatLog(const EChatType eChatType)
	{
		Set(eChatType);
	}

	void Set(const EChatType eChatType)
	{
		Clear();
		ChatType(eChatType);
	}
	
	DWORD ChatFlag()const 
	{
		return (1<<ChatType());
	}

	void Clear();

	bool IsDelayLog() const
	{
		return IMEPT_NONE != DelayCause();
	}

	//
	float fTime;
	BM::GUID	kCharGUID;
	std::wstring kCharName;
	std::wstring kContents;
	DWORD		 dwColor;
	XUI::PgExtraDataPackInfo	kExtraDataPackInfo;
	bool bMine;
	CLASS_DECLARATION_S(EChatType, ChatType);
	CLASS_DECLARATION_S(EItemModifyParentEventType, DelayCause);
} SChatLog;

typedef enum eEventMessage
{
	EM_FreindFailed					= 700013,
	EM_FriendAdder					= 700014,
	EM_FriendRecver					= 700015,
	EM_FriendAdded					= 700016,
	EM_DoNot_Keyboard_Warior		= 700017,
	EM_Invalid_Command				= 700020,
	EM_Whisper_InputError			= 700021,
	EM_Invalid_EditGroup			= 700022,
	EM_Invalid_EditGroup_Friend		= 700023,
	EM_Is_Not_Empty					= 700024,
	EM_Is_Login						= 700025,
	EM_Is_Logout					= 700026,
	EM_Duplicate_Group				= 700029,
	EM_Limit_Group					= 700030,
	EM_Cant_Add_Me					= 700039,
	EM_FriendRecver_Req				= 700133,
}EEventMessage;

typedef enum eBallonState
{
	BS_Not_Open,
	BS_Normal_Open,
	BS_Party_Open,
	BS_Guild_Open,
};

typedef enum eNotifyConnectInfo: BYTE
{
	NCI_NONE	= 0,
	NCI_FRIEND	= 1,
	NCI_GUILD	= 2,
	NCI_COUPLE	= 3,
	NIC_END
}ENotifyConnectInfo;

typedef struct tagNotifyConnectInfo
{
	tagNotifyConnectInfo() 
		:	eNotifyInfo(NCI_NONE),
			kOutString(_T("")),
			kName(_T("")),
			kUVInfo(SUVInfo()),
			nClassNo(0),
			kGuid(BM::GUID())
	{
	}

	tagNotifyConnectInfo(ENotifyConnectInfo const NfyInfo, std::wstring const& rkOutString, std::wstring const& rkName, int ClassNo, SUVInfo& rkUVInfo, BM::GUID const& rkGuid)
		:	eNotifyInfo(NfyInfo),
			kOutString(rkOutString),
			kName(rkName),
			kUVInfo(rkUVInfo),
			nClassNo(ClassNo),
			kGuid(rkGuid)
	{
	}

	tagNotifyConnectInfo(tagNotifyConnectInfo const& rhs)
		:	eNotifyInfo(rhs.eNotifyInfo),
			kOutString(rhs.kOutString),
			kName(rhs.kName),
			kUVInfo(rhs.kUVInfo),
			nClassNo(rhs.nClassNo),
			kGuid(rhs.kGuid)
	{
	}

	ENotifyConnectInfo	eNotifyInfo;
	std::wstring		kOutString;
	std::wstring		kName;
	SUVInfo				kUVInfo;
	int					nClassNo;
	BM::GUID			kGuid;
} SNotifyConnectInfo;

typedef struct tagTextColorMap
{
	tagTextColorMap()
		:	eChatType(CT_NONE),
			TextColor(0xFFFFFFFF)
	{
	}

	tagTextColorMap(EChatType ChatType, unsigned int iColor)
		:	eChatType(ChatType),
			TextColor(iColor)
	{
	}

	tagTextColorMap(int ChatType, unsigned int iColor)
		:	eChatType(static_cast<EChatType>(ChatType)),
			TextColor(iColor)
	{
	}

	tagTextColorMap(tagTextColorMap const& rhs)
		:	eChatType(rhs.eChatType),
			TextColor(rhs.TextColor)
	{
	}

	EChatType			eChatType;
	unsigned int		TextColor;
} STextColorMap;

typedef struct tagNoticeLevelColor
{
	tagNoticeLevelColor()
	{
		dwFontColor = dwOutlineColor = 0xFF000000;
	}

	tagNoticeLevelColor(DWORD const dwFont, DWORD const dwOutline, bool const bBG)
	{
		dwFontColor = dwFont;
		dwOutlineColor = dwOutline;
		bShowBG = bBG;
	}

	tagNoticeLevelColor(const tagNoticeLevelColor& rhs)
	{
		dwFontColor = rhs.dwFontColor;
		dwOutlineColor = rhs.dwOutlineColor;
		bShowBG = rhs.bShowBG;
	}

	bool operator == (const tagNoticeLevelColor& rkRight) const
	{
		bool bRet = true;
		bRet &= dwFontColor == rkRight.dwFontColor;
		bRet &= dwOutlineColor == rkRight.dwOutlineColor;
		bRet &= bShowBG == rkRight.bShowBG;
		return bRet;
	}

	DWORD dwFontColor;
	DWORD dwOutlineColor;
	bool bShowBG;
} SNoticeLevelColor;

//
class PgChatMgrClient : public PgIXmlObject
{
	typedef std::vector< SChatLog > ContDelayLog;
	typedef std::list< SChatLog > ContDelayList;
	typedef std::map< int, ContDelayLog > ContDelayLogMap;
public:

	typedef struct tagChatCommand//채팅형 판단 커맨드
	{
		std::wstring kChatCommand;
		EChatType eChatType;
		unsigned int iMinCount;
		std::string kLuaScript;
		tagChatCommand()
		{
			eChatType = CT_NONE;
			iMinCount = 0;
			kLuaScript.clear();
		}
	} SChatCommand;

	typedef struct tagNoticeWaitItem : public SNoticeLevelColor
	{
		tagNoticeWaitItem()
			:tagNoticeLevelColor()
		{
		}

		tagNoticeWaitItem(std::wstring const &_kContents, const SNoticeLevelColor& rkColor)
			:kContents(_kContents), tagNoticeLevelColor(rkColor)
		{}

		bool operator == (const tagNoticeWaitItem& rhs)const
		{
			bool bRet = true;
			bRet &= SNoticeLevelColor::operator ==(rhs);
			bRet &= (kContents==rhs.kContents);
			return bRet;
		}
		std::wstring	kContents;
	}SNoticeWaitItem;
	typedef std::queue< SNoticeWaitItem >	QueueNoticeWait;

	typedef struct tagName2Guid//Name->Guid, Guid->Name
	{
		tagName2Guid(EChatType const Type, std::wstring const &rkCharName, BM::GUID const &rkCharGuid)
			: eType(Type), kCharName(rkCharName), kCharGuid(rkCharGuid)
		{
		}

		tagName2Guid()
		{
			eType = CT_NONE;
			kCharName.clear();
			kCharGuid.Clear();
		}

		EChatType eType;
		std::wstring kCharName;
		BM::GUID kCharGuid;
	} SName2Guid;

	typedef std::list< SFriendItem > ContListFriend;
	typedef std::list< SName2Guid > ContName2GuidList;
	typedef std::map< std::wstring, ContName2GuidList::iterator > ContName2Guid;
	typedef std::map< BM::GUID, ContName2GuidList::iterator > ContGuid2Name;
	typedef std::map< std::wstring, SChatCommand > ContChatCommand;
	typedef std::list< std::wstring > ContWhisper;
	typedef std::list<SNotifyConnectInfo> ContNotifyConnectInfoList;
	typedef std::list<STextColorMap> ContTextColorList;

	typedef struct tagInputChatLog//
	{
		float fInputTime;
		std::wstring kChat;
		float fTermTime;
		tagInputChatLog(std::wstring const &rkChat);

		bool operator == (std::wstring const &rkRight)
		{
			return (kChat == rkRight);
		}
	} SInputChatLog;
	typedef std::list< SInputChatLog > ContInputLog;
	typedef std::list<std::wstring> ContInputList;


	//Member Function
public:
	PgChatMgrClient();
	virtual ~PgChatMgrClient();

	void Clear();

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	bool RegChatCommand(std::wstring const &rkCommand, const EChatType eChatType, const unsigned int iMinCount, const std::string& rkLuaScript);

	//일반 채팅 관련
	bool SendChat(std::wstring const &rkChat, std::wstring const &rkName);//전체 채팅 메니져 (이모션, /파티 등등 처리)
	bool SendMToMChat(std::wstring& rkChat, DWORD& Color, std::wstring const& wstrName);
	bool RecvChat(BM::Stream* pkPacket);

	//채팅 행위(이모션) 관련
	bool SendChat_InputNow(bool bClear=false);
	bool RecvChat_InputNow(BM::Stream* pkPacket);
	bool Self_InputNow(bool const bPopup, int const iChatMode = CT_NORMAL);

	void Character_ChatBallon_Set(BM::GUID &rkCharGuid, std::wstring const &rkWString, int const iChatType = CT_NORMAL);//캐릭터 말풍선 삽입

	//Chat Log
	void InputChatLog_Add(std::wstring const &rkChat);//채팅 입력 Log
	bool InputChatLog_Get(std::wstring &rkChat, bool bPrev=false);
	void InputChatLog_Reset();
	bool CheckSpamChat(std::wstring const &rkChat); // 도배 판별

	//Chat Command
	bool Command_UserFunction(std::wstring const& rkChat);
	bool Command_EmotiAction(std::wstring const& rkChat);
	bool Command_God(std::wstring const &rkChat);
	bool Command_Chat(std::wstring const &rkChat, int &iChatMode, std::wstring &rkName, std::wstring &rkContents);//채팅 커맨드
	bool Command_Emotion(std::wstring const &rkChat);//이모션 필터링(감정)
	bool Command_Filter(std::wstring &rkChat);//욕 필터링

	//커맨드가 포함된 금지어 필터링(2010. 07. 14 조현건)
	bool TransEmoticonFilter(std::wstring &rkChat);
	bool SeperateCommandString(std::wstring const& rkOrgText, std::wstring &rkNormalText, std::map<size_t, std::wstring> &kContCommand);//커맨드와 일반 채팅을 분리(맵에 커맨드 저장)
	bool SeperateUserCommand(std::wstring const& rkCmdText, std::wstring::size_type kNextPos, std::wstring &rkNormalText, std::map<size_t, std::wstring> &kContCommand);//커맨드타입에 따라 분리
	
	//채팅 기능 제한
	void SetBlockSpamChat_ChainInput(float const fWatchT, float const fBlockTime);
	void SetBlockSpamChat_LooseChainInput(float const fWatchT, int const iCnt, float const fBlockTime);
	void SetBlockSpamChat_MacroInput(int const iCnt, float const fRangeTime, float const fBlockTime);
	void SetBlockSpamChat_SameInput(float const fWatchT, int iCnt, float const fBlockTime);

	void SetMaxBlockTime(float const fMaxBlockTime);

	//Name 2 Guid
	void Name2Guid_Add(EChatType const eType, std::wstring const &rkCharName, BM::GUID const &rkCharGuid);
	bool Name2Guid_Find_ByName(std::wstring const &rkCharName, BM::GUID &rkCharGuid) const;
	bool Name2Guid_Find_ByGuid(BM::GUID const &rkCharGuid, std::wstring &rkCharName) const;
	void Name2Guid_Clear();
	bool Name2Guid_Remove(BM::GUID const &rkCharGuid);
	bool Name2Guid_Remove(std::wstring const &rkCharName);
	void Name2Guid_Remove(EChatType const eType);

	//Log 추가 함수
	void ShowNoticeUI(std::wstring const kContents, int iLevel = 0, bool bUpper = true, bool const bCheckEqualMessage = true);
	void AddMessage(int const iTextTable, SChatLog &rkChatLog, bool bNotice = false, int iLevel = 0, bool bUpper=true, bool const bCheckEqualMessage = false);
	void AddLogMessage(SChatLog &rkChatLog, std::wstring const &rkLogMessage, bool bNotice = false, int iLevel = 0, bool bUpper=true, bool const bCheckEqualMessage = false, bool const IsMonster = false);
	void LogMsgBox(int const iTTW);
	void LogMsgBox(std::wstring const &rkMsg);

	void ShowNotice(bool bUpper = true, bool const bCheckEqualMessage = true);
	bool CheckChatOut();
	bool SetChatOutFocus();
	void ClearNotice();
	void DisableNotice();

	std::wstring const& GetFinalWisper() { BM::CAutoMutex kLock(m_kMutex); return (*m_kWhisperList.rbegin()); }
	void SetChatControl(EChatType const Type, std::wstring const& kName, std::wstring  const& Contants);
	bool SendChatDlg(std::wstring const& Text);

	bool SetWhisperDlg(XUI::CXUI_Wnd* pkWnd);
	std::wstring SetWhisperByTab(void);
	std::wstring ConvertUserCommand(std::wstring const &kString,std::wstring const & kFont = _T("EmoticonFont")); // # 계열 커멘드(#1,#2,#R...) 를 파싱하여 XUI의 StyleString 에 맞게 치환한다.

	bool CheckSysCommand(std::wstring const& kText);
	bool CheckChatCommand(std::wstring const& kText);
	bool SendChat_Check(std::wstring const& kText, bool const bCheckSpamChat=false);
	bool SendChat_CheckSS(XUI::CXUI_Style_String const& kText, bool const bCheckSpamChat=false);
	bool SendChat_Message(int const iChatType, XUI::CXUI_Style_String const& kText, std::wstring const& kName = L"");
	bool SendChat_MuteCheck();

	void Update();

	void AddDelayLog(SChatLog const& rkLog); // 특수 이벤트 상황(아이템 메시지 지연에 사용)
	void FlushDelayLog(EItemModifyParentEventType const eCause);
	
	void ClearBlockTime();	// 채팅 금지 시간 초기화
	
	//채팅가능 인식 모드
	void ChatAram(const std::wstring & strAramSound, const bool bShowNow = false);
	void ClearChatBalloon(); 

	void SetChatOutDefaultPos(POINT2 const& rkPos);
	void SetSysChatOutDefaultPos(POINT2 const& rkPos);
	void SetSavedPos(POINT2 const& rkPos);
	void SetSystemSavedPos(POINT2 const& rkPos);
	void ApplyChatOutSavedPos(XUI::CXUI_Wnd* pkWnd);
	void ApplySystemChatOutSavedPos(XUI::CXUI_Wnd* pkWnd);
	POINT2 const& GetChatOutDefaultPos() const;
	POINT2 const& GetSysChatOutDefaultPos() const;

	bool TogglingConsecutiveChat(void);

	void ToggleConsecutiveChatUI(void);

	// 접속 알림.
	void ShowNotifyConnectInfo(ENotifyConnectInfo const NfyInfo, std::wstring const& kName, int nClassNo, SUVInfo& kUVInfo, BM::GUID const& rkGuid);
	void ShowNotifyConnectInfoUI(void);
	void PopNotifyList(void);

	BM::GUID& GetNotifyGuid(void);

	void CheckSnapPos(POINT2 const& rkPos);

	void UseFontColor(unsigned int iColor);
	bool CheckChatTag(std::wstring const & Text);	// 유저가 채팅 Tag를 사용했는지 검사.

	// 채팅 차단 관련 함수.
	void SetChatBlockList(CONT_BLOCKCHAT_LIST & BlockList);
	void GetChatBlockList(CONT_BLOCKCHAT_LIST & BlockList);
	void Regist_ChatBlockUser(std::wstring & Name);
	void UnRegist_ChatBlockUser(std::wstring & Name);
	void Modify_ChatBlockMode(std::wstring & Name, BYTE BlockMode);
	void UpdateChatBlockList(XUI::CXUI_Wnd * pWnd);
	bool CheckChatBlockList(std::wstring & CharName);
	bool AddChatBlockList(std::wstring & CharName, BYTE BlockType);
	void RemoveChatBlockList(std::wstring & CharName);
	bool ModifyChatBlockList(std::wstring & CharName, BYTE BlockType, BYTE & ModifyBlockType);
	bool CheckChatBlockCharacter(std::wstring & CharName, EChatBlockType BlockType);

	bool CheckChattingOption(SChatLog const & ChatLog, PgChatMgrUtil::CChatLogList const & ChatLogList);

	void LoveModeMessageDelay(std::wstring & Message, int nLevel);

	bool IsItemLinkContainData(XUI::PgExtraDataPackInfo const &rkExtraDataPackInfo) const;

protected:
	void InsertWhisper(std::wstring const& kName, bool const bIsSort = true);
	SChatCommand const* GetCommandType(std::wstring const& Cmd);
	SChatCommand const* GetCommandTypeToMode(EChatType const Mode);

	void ShowNotice(int const iIndex, std::wstring const &rkContents, const SNoticeLevelColor& rkColor, DWORD const dwCallTime, int const iY = 0, bool const bCheckEqualMessage = true);
	void RecvChatLog_Add(const SChatLog &rkChatLog, bool const bCheckEqualMessage = false, bool const IsMonster = false);//도착한 Log // 귓속말, 메시지 지연일 경우는 bCheckEqualMessage 체크 하지 않음
	bool GetUserCommandConvertedString(std::wstring const &kStringSrc,std::wstring &kConvertedString, std::wstring const & kFont);	
	ContName2GuidList::iterator Name2Guid_Remove(ContName2GuidList::iterator iter);	
	void IncBlockChatTime(float const fIncTime);

	CLASS_DECLARATION(bool, m_bShowNoticeAble,IsShowNoticeAble);//공지가 가능한 상태인지?(ex 맵로딩중에는 공지가 와도 안보이니...
	CLASS_DECLARATION(int, m_iChatMode, ChatMode);//현재 채팅 상태정보
	CLASS_DECLARATION(bool, m_bActive, Active);//Application이 작동중인지? (종료단계에서 assert(0) 나온다)

	//도배로 인한 채팅 금지 시간
	CLASS_DECLARATION(float, m_fStartBlockTime, StartBlockTime);	// 도배 금지 시작 시간
	CLASS_DECLARATION(float, m_fBlockTime, BlockTime);				// 도배로 인한 총 채팅 금지 시간
	CLASS_DECLARATION(float, m_fMaxBlockTime, MaxBlockTime);		// 도배로 인한 최대 채팅 금지 시간	
	
	//도배 검사 flag값
	CLASS_DECLARATION(int, m_iSpamChkFlag, SpamChkFlag);

	//도배 판단 기준
	CLASS_DECLARATION(size_t, m_stMaxInputLog, MaxInputLog);	// 입력한 채팅로그를 몇개까지 저장 할 것 인가
	//SetBlockSpamChat_ChainInput - T 초 이내 채팅 입력하면 도배 판단
	CLASS_DECLARATION(float, m_fChainWatchT, ChainWatchT);			// T
	CLASS_DECLARATION(float, m_fChainBlockTime, ChainBlockTime);	// 채팅 금지 시간

	//SetBlockSpamChat_LooseChainInput - T초 이내 X번 채팅 입력 하면 도배 판단
	CLASS_DECLARATION(float, m_fLChainWatchT,LChainWatchT);			// T
	CLASS_DECLARATION(unsigned int, m_iLChainCnt, LChainCnt);		// X
	CLASS_DECLARATION(float, m_fLChainBlockTime, LChainBlockTime);	// 채팅 금지 시간

	//SetBlockSpamChat_MacroInput - X번을 일정시간 만큼(오차 R초) 입력하면 도배 판단
	CLASS_DECLARATION(unsigned int, m_iMaroCnt, MacroCnt);			// T
	CLASS_DECLARATION(float, m_fMacroRangeTime, MacroRangeTime);	// R
	CLASS_DECLARATION(float, m_fMacroBlockTime, MacroBlockTime);	// 채팅 금지 시간

	//SetBlockSpamChat_SameInput - T초이내 같은 글자를 연속으로 X번 입력하면 도배
	CLASS_DECLARATION(float, m_fSameWatchT, SameWatchT);			// T
	CLASS_DECLARATION(unsigned int, m_iSameCnt, SameCnt);			// X
	CLASS_DECLARATION(float, m_fSameBlockTime, SameBlockTime);		// 채팅 금지 시간
	
	CLASS_DECLARATION(unsigned int, m_iFontColor, FontColor);
	CLASS_DECLARATION(bool, m_bIsUseColor, UseColor);
	
	CLASS_DECLARATION( UINT64, m_iOldChatTime, OldChatTime);//채팅모드 인식기능 시간

	CLASS_DECLARATION(int, m_iOldChatMode, OldChatMode);

	CLASS_DECLARATION(bool, m_bToggleConsecutiveChat, ToggleConsecutiveChat);
	CLASS_DECLARATION(bool, m_bExistWhisperList, ExistWhisperList);
	CLASS_DECLARATION(bool, m_bSysChatOutSnap, SysChatOutSnap);
	CLASS_DECLARATION(bool, m_bSysOutHide, SysOutHide);

	CLASS_DECLARATION(float, m_TradeModeChatTime, TradeModeChatTime);	// 거래 채팅 대기 시간(30초에 한번만 가능).

	void ChatStation(EChatStation eStation);
	EChatStation ChatStation() const;
	EChatStation PastChatStation() const;

private:
	bool Command_UserUse(std::wstring const &rkCommand); // 유저가 사용할수 있는 커맨드
	POINT2 m_kSavedPos;
	POINT2 m_kSystemSavedPos;
	EChatStation m_kChatStation;
	EChatStation m_kPastChatStation;

	//Member Variable
protected:

	//입력채팅 로그
	ContInputLog							m_kLog_Input;//도배 방지 사용
	ContInputList							m_kLog_InputChatForBlock;//도배 방지용 입력 히스토리
	ContInputList							m_kLog_InputChat;//키보드 입력 히스토리
	ContInputList::reverse_iterator			m_kLog_InputChatIter;//키보드 입력 히스토리 커서
	int										m_nLog_InputChat_Old;

	//사용자 Guid 2 Name
	ContName2GuidList		m_kCache_Name2Guid;
	ContName2Guid			m_kLog_Name2Guid; // 이름을 Guid로
	ContGuid2Name			m_kLog_Guid2Name; // Guid를 이름으로
	ContWhisper				m_kWhisperList;
	std::wstring			m_kWhisperTarget;

	//채팅 커맨드
	ContChatCommand			m_kChatCommand;//채팅 커맨드 리스트 (/길드, /p, /w, /s, ... 등등)
	eBallonState			m_kBallonState;

	// 공지사항 대기 리스트
	QueueNoticeWait		m_kNoticeWaitList;

	DWORD m_dwChatModeFilter;

	ContDelayList	m_kChatLog;			//Update용도
	ContDelayLogMap m_kDelayLog;		// 지연된 메시지 저장소

	ContNotifyConnectInfoList	m_kNotifyList;	// 친구,길드,연인 접속알림 창 메세지

	ContTextColorList m_kTextColorList;	// 채팅 타입별 텍스트 칼라.

	CONT_BLOCKCHAT_LIST m_MyChatBlockList;	// 채팅 차단 리스트.

private:
	mutable Loki::Mutex m_kMutex;
};

void Notice_Show(std::wstring const &rkString, int const iLevel, bool const bCheckEqualMessage);
void Notice_Show_ByDefStringNo(int const iDefStringNo, int const iLevel);
void Notice_Show_ByTextTableNo(int const iTextTableNo, int const iLevel, bool const bCheckEqualMessage = false);

extern void MakeStringForDate( std::wstring &wstr, SYSTEMTIME const &kTime );
extern void MakeStringForTime( std::wstring &wstr, SYSTEMTIME const &kTime, bool const bMSEC=false );
extern void WstringFormat( std::wstring& rkOutMsg, size_t const iSize, const wchar_t* szMsg, ... );

extern void MakeStringForDateTT( std::wstring &wstr, SYSTEMTIME const &kTime, int const iTT );
extern void MakeStringForTimeTT( std::wstring &wstr, SYSTEMTIME const &kTime, int const iTT );

extern void Announce_Msg(std::wstring const &rkStr, EChatType const eChatType, ENoticeLevel const eNoticeLevel=EL_Normal, bool const bNotice=true, bool const bUpper=true);
#define g_kChatMgrClient SINGLETON_CUSTOM(PgChatMgrClient, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_CONTENTS_CHAT_PGCHATMGRCLIENT_H