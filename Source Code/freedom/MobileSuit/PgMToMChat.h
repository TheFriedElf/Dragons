#ifndef FREEDOM_DRAGONICA_CONTENTS_CHAT_1_1_PGMTOMCHAT_H
#define FREEDOM_DRAGONICA_CONTENTS_CHAT_1_1_PGMTOMCHAT_H

extern int const MAX_MToM_CHAT_LIST;
extern int const MAX_DLG_COUNT;
extern float const FLASH_UPDATE_TICK_TIME;
extern wchar_t const* const szDefault_SmallFormName;
extern wchar_t const* const szDefault_BigFormName;
extern wchar_t const* const szDefault_QuestionName;

typedef enum
{
	MTM_MSG_DEFAULT			= 1,
	MTM_MSG_EMPORIABATTLE	= 3,
	MTM_MSG_EM_ADMERCENARY	= 4,
}E_MTM_MSG_TYPE;// 이미지의 UV Index에 맞게 잘 설정 해야 한다.

typedef struct tagMtoMChatData
{
	tagMtoMChatData()
		:	kName(_T("")), kContants(_T("")), dwColor(0)
//		,	kType(MTM_MSG_DEFAULT)
	{
	};

	std::wstring	kName;
	std::wstring	kContants;
	DWORD			dwColor;
//	BYTE			kType;
}SMtoMChatData;

typedef struct tagMtoMChatIDType
{
	tagMtoMChatIDType()
		:	kType(MTM_MSG_DEFAULT)
	{}

	tagMtoMChatIDType( BM::GUID const &_kGuid, BYTE const _kType )
		:	kGuid(_kGuid)
		,	kType(_kType)
	{}

	BM::GUID	kGuid;
	BYTE		kType;
}SMtoMChatIDType;
typedef std::list< SMtoMChatIDType > kMToMChatUserContainer;

class PgMToMChat
{
public:
	PgMToMChat();
	explicit PgMToMChat( BM::GUID const& Guid, std::wstring const& kName, BYTE const kType );
	virtual ~PgMToMChat();

	void Init( BM::GUID const& Guid, std::wstring const& kName, BYTE const kType );
	void AddChatMsg( BYTE const kType, SMtoMChatData const& ChatData, std::wstring const& kID, bool const bIsOpen );
	void DrawAllChatMsg(std::wstring const& kID);
	void Show();
	void Hide();

private:
	typedef std::list< SMtoMChatData > kChatContainer;

	kChatContainer	m_kChatContainer;

	CLASS_DECLARATION_S(bool, ArriveNewChat);
	CLASS_DECLARATION_S(bool, IsDlgOpen);
	CLASS_DECLARATION_S(BYTE, Type);
	CLASS_DECLARATION_S_NO_SET(std::wstring, MToMName);
	CLASS_DECLARATION_S_NO_SET(BM::GUID, MToMGuid);
};

typedef std::set< std::wstring > kIDContainer;

class PgMToMChatMgr
{
public:
	PgMToMChatMgr();
	virtual ~PgMToMChatMgr();
	
	void Add(BM::GUID const& Guid, std::wstring const& kName, BYTE const kType, SMtoMChatData const& ChatData, bool const bIsOpen = false );
	void Add(BM::GUID const& Guid, std::wstring const& kName, BYTE const kType );
	void Add(BM::GUID const& Guid, SMtoMChatData const& ChatData);
	void Del(BM::GUID const& Guid);

	void AddAllList(BM::GUID const& Guid);
	bool IsChatDlgState(BM::GUID const& Guid);
	bool SetChatDlgState(BM::GUID const& Guid, bool const IsState);
	bool IsArriveNew(BM::GUID const& Guid);
	bool SetArriveNew(BM::GUID const& Guid, bool const IsNew);
//	bool FlashUpdate(BM::GUID const& Guid, int& NextUV);
	bool GetName(BM::GUID const& Guid, std::wstring& Name)const;
	bool GetChatDlgList(kMToMChatUserContainer& kCont)const;

	bool AddID(BM::GUID const& Guid, std::wstring& wstrID);
	bool DelID(BM::GUID const& Guid);
	bool FindID(BM::GUID const& Guid, std::wstring& wstrID);
	void GetIDList(bool const bIsOpen, kIDContainer& kList);

	void Clear();
	
private:
	typedef std::map< BM::GUID, PgMToMChat > kMToMContainer;
	typedef std::set< std::wstring > kIDContainer;

	typedef struct tagNameData
	{
		bool bIsOpen;
		kIDContainer::iterator	iter;
	}SNameData;

	typedef std::map< BM::GUID, SNameData > kDlgIDContainer;
	
// 	typedef struct tagFlashUpdateData
// 	{
// 		tagFlashUpdateData() : bIsLight(false), fTime(0.0f){};
// 		bool bIsLight;
// 		float fTime;
// 	}SFlashUpdateData;

//	typedef std::map< BM::GUID, SFlashUpdateData > kFlashTimeContainer;

//	kFlashTimeContainer	m_kTimeCont;
	kMToMContainer	m_kMToMCont;
	kIDContainer	m_kBigIDCont;
	kIDContainer	m_kSmallIDCont;
	kDlgIDContainer	m_kGuidToDlgIDCont;
};

#define g_kMToMMgr SINGLETON_STATIC(PgMToMChatMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_CHAT_1_1_PGMTOMCHAT_H