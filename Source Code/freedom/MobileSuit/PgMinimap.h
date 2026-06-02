#ifndef FREEDOM_DRAGONICA_CONTENTS_MINIMAP_PGMINIMAP_H
#define FREEDOM_DRAGONICA_CONTENTS_MINIMAP_PGMINIMAP_H

#include "PgResourceIcon.h"
#include "PgQuestMan.h"
#include "Pg2DString.h"
#include "PgUIDrawObject.h"
#include "PgMinimapType.h"

extern bool g_bAbleSetTeleMove;
extern int GetEntityPointByTriggerID( char const * pTriggerID );
extern int GetEntityLevelByTriggerID( char const * pTriggerID );
extern int GetEntityLinkByTriggerID( char const * pTriggerID );
extern int GetPointByLevel( int const Level );
extern int GetLevelByPoint( int const Point );

typedef enum : BYTE
{//미리 정의해 두자
	EMUT_ALWAYS_VIEW	= 0,
	EMUT_FULLSIZE_VIEW	= 1,
	EMUT_BOOK_INFO		= 2,
	EMUT_MYHOME_TOWN	= 3,
}E_MINIMAP_USE_TYPE;

typedef enum : int
{
	ICONTYPE_NONE						= 0,
	ICONTYPE_ME							= 1,
	ICONTYPE_MYPET						= 2,
	ICONTYPE_PARTY						= 3,
	ICONTYPE_COUPLE						= 4,
	ICONTYPE_GUILD						= 5,
	ICONTYPE_NPC						= 6,
	ICONTYPE_MONSTER					= 7,
	ICONTYPE_PARTY_LEFT_ARROW			= 8,
	ICONTYPE_PARTY_RIGHT_ARROW			= 9,
	ICONTYPE_GUILD_LEFT_ARROW			= 10,
	ICONTYPE_GUILD_RIGHT_ARROW			= 11,
	ICONTYPE_NPC_LEFT_ARROW				= 12,
	ICONTYPE_NPC_RIGHT_ARROW			= 13,
	ICONTYPE_PORTAL						= 14,
	ICONTYPE_MISSION					= 15,
	ICONTYPE_JUMP						= 16,
	ICONTYPE_TELEJUMP					= 17,
	ICONTYPE_REPAIR						= 18,
	ICONTYPE_BANK						= 19,
	ICONTYPE_SHOP						= 20,
	ICONTYPE_QUEST						= 21,
	ICONTYPE_C_STONE					= 22,
	ICONTYPE_UNKNOWN_POINT				= 23,
	ICONTYPE_QUEST_END					= 24,	// ?
	ICONTYPE_QUEST_BEGIN				= 25,	// !
	ICONTYPE_QUEST_ING					= 26,	// ? (회색)
	ICONTYPE_QUEST_NOTYET				= 27,	// ! (회색)
	ICONTYPE_QUEST_END_STORY			= 28,	// ? (빨강)
	ICONTYPE_QUEST_BEGIN_STORY			= 29,	// ! (빨강)
	ICONTYPE_QUEST_END_LOOP				= 30,	// ? (파랑)
	ICONTYPE_QUEST_BEGIN_LOOP			= 31,	// ! (파랑)
	ICONTYPE_QUEST_END_GUILD			= 32,	// ? (녹색)
	ICONTYPE_QUEST_BEGIN_GUILD			= 33,	// ! (녹색)
	ICONTYPE_EMPORIA_MAIN_RED			= 34,
	ICONTYPE_EMPORIA_MAIN_BLUE			= 35,
	ICONTYPE_EMPORIA_SUB_RED			= 36,
	ICONTYPE_EMPORIA_SUB_BLUE			= 37,
	ICONTYPE_QUEST_END_WEEKLY			= 38,	// ? (보라)
	ICONTYPE_QUEST_BEGIN_WEEKLY			= 39,	// ! (보라)
	ICONTYPE_QUEST_END_COUPLE			= 40,	// ? (핑크)
	ICONTYPE_QUEST_BEGIN_COUPLE			= 41,	// ! (핑크)
	ICONTYPE_HOME_OPEN					= 42,
	ICONTYPE_HOME_AUCTION				= 43,
	ICONTYPE_HOME_CLOSE					= 44,
	ICONTYPE_HOME_MYHOUSE				= 45,
	ICONTYPE_BATTLESQUARE_ITEM			= 46,
	ICONTYPE_MISSION_EASY				= 47,
	ICONTYPE_TELEPORT					= 48,
	ICONTYPE_SELECT_OBJECT				= 49,
	ICONTYPE_QUEST_CARD_END				= 50,
	ICONTYPE_QUEST_CARD_BEGIN			= 51,
	ICONTYPE_TEAM_RED					= 52,
	ICONTYPE_TEAM_BLUE					= 53,
	ICONTYPE_TEAM_RED_MEDAL				= 54,
	ICONTYPE_TEAM_BLUE_MEDAL			= 55,
	ICONTYPE_TEAM_GUARDIAN_RED			= 56,
	ICONTYPE_TEAM_GUARDIAN_BLUE			= 57,
	ICONTYPE_STRONGHOLD_RED_LV1			= 58,
	ICONTYPE_STRONGHOLD_RED_LV2			= 59,
	ICONTYPE_STRONGHOLD_RED_LV3			= 60,
	ICONTYPE_STRONGHOLD_RED_LV4			= 61,
	ICONTYPE_STRONGHOLD_RED_LV5			= 62,
	ICONTYPE_STRONGHOLD_BLUE_LV1		= 63,
	ICONTYPE_STRONGHOLD_BLUE_LV2		= 64,
	ICONTYPE_STRONGHOLD_BLUE_LV3		= 65,
	ICONTYPE_STRONGHOLD_BLUE_LV4		= 66,
	ICONTYPE_STRONGHOLD_BLUE_LV5		= 67,
	ICONTYPE_BOMBMAN_RED				= 68,
	ICONTYPE_BOMBMAN_BLUE				= 69,
	ICONTYPE_LOVE_FENCE					= 70,
	ICONTYPE_DESTROYED_LOVE_FENCE		= 71,
	ICONTYPE_BEAR_RED					= 72,
	ICONTYPE_BEAR_BLUE					= 73,
	ICONTYPE_QUEST_END_CHANGECLASS		= 80,	// ?
	ICONTYPE_QUEST_BEGIN_CHANGECLASS	= 81,	// !
	ICONTYPE_QUEST_ING_CHANGECLASS		= 82,	// ? (회색)
	ICONTYPE_QUEST_NOTYET_CHANGECLASS	= 83,	// ! (회색)
	ICONTYPE_BOUNTY_EXCHANGE = 84, //Bounty NPC
	ICONTYPE_QUEST_MON = 85, // Monster Quest Icon
} IconType;

typedef enum eMapIconTexType
{
	EMITT_NONE = -1,
	EMITT_SMALL = 0,
	EMITT_LARGE = 1,
	EMITT_MYACTOR,
	EMITT_CORE,
	EMITT_SUB,
	EMITT_MYPARTY,
	EMITT_SELECT_ARROW,
}EMapIconTexType;

typedef struct tagMapIconTexInfo
{
	tagMapIconTexInfo()
		: IconTexture(NULL)
		, W(0), H(0), U(0), V(0)
	{};

	void operator = (tagMapIconTexInfo const& rhs)
	{
		IconTexture = rhs.IconTexture;
		W = rhs.W;
		H = rhs.H;
		U = rhs.U;
		V = rhs.V;
	};

	NiScreenElementsPtr	IconTexture;
	int W;
	int H;
	int U;
	int V;
}SMapIconTexInfo;

typedef struct tagMapIconTypeToIndex
{
	tagMapIconTypeToIndex()
		: Type(ICONTYPE_NONE)
		, Index(-1)
		, TexType(EMITT_NONE)
	{};
	tagMapIconTypeToIndex(IconType Type)
		: Type(Type)
		, Index(-1)
		, TexType(EMITT_NONE)
	{};

	bool operator == ( tagMapIconTypeToIndex const& rhs) const
	{
		return ( Type == rhs.Type )?(true):(false);
	}

	bool operator < ( tagMapIconTypeToIndex const& rhs) const
	{
		return ( Type < rhs.Type )?(true):(false);
	}

	IconType Type;
	short Index;
	EMapIconTexType	TexType;
	SMapIconTexInfo TexInfo;
	POINT2 IconPos;
}SIconTypeToIndex;

typedef struct tagMiniMapAniIcon
{
	tagMiniMapAniIcon() 
		: m_kUVInfo(0,0)
		, m_kTickTime(0.0f)
		, m_kTime(0.0f)
		, m_kNowFrame(0)
		, m_kMaxFrame(0)
	{
	};

	tagMiniMapAniIcon(float const TickTime, int const iMaxFrame, POINT2 const UVInfo)
		: m_kTime(0.0f)
		, m_kNowFrame(0)
	{
		m_kUVInfo = UVInfo;
		m_kTickTime = TickTime;
		m_kMaxFrame = iMaxFrame;
	}
	
	void CalcFrame()
	{
		++m_kNowFrame;
		if( m_kNowFrame >= m_kMaxFrame ){ m_kNowFrame = 0; }
	}

	bool NexFrame(float const fAccumTime)
	{
		if( (fAccumTime - m_kTime) > m_kTickTime )
		{
			m_kTime = fAccumTime;
			CalcFrame();
			return true;
		}
		return false;
	}

	CLASS_DECLARATION_S(POINT2,	UVInfo);
	CLASS_DECLARATION_S(float, TickTime);
private:
	CLASS_DECLARATION_S_NO_SET(float, Time);
	CLASS_DECLARATION_S_NO_SET(int,	NowFrame);
	CLASS_DECLARATION_S_NO_SET(int,	MaxFrame);
}SMiniMapAniIcon;

typedef struct tagQuestMiniMapKey
{
	tagQuestMiniMapKey()
		: kGuid()
		, kOrder(PgQuestManUtil::EQMDO_HIGH)
	{};
	tagQuestMiniMapKey(BM::GUID const& Guid)
		: kGuid(Guid)
		, kOrder(PgQuestManUtil::EQMDO_HIGH)
	{};
	tagQuestMiniMapKey(BM::GUID const& Guid, PgQuestManUtil::EQuestMarkDrawOrder const Order)
		: kGuid(Guid)
		, kOrder(Order)
	{};

	bool operator==(tagQuestMiniMapKey const& rhs) const
	{
		return (kGuid == rhs.kGuid)?(true):(false);
	}

	bool operator<(tagQuestMiniMapKey const& rhs) const
	{
		if(kOrder < rhs.kOrder)
		{
			return true;
		}
		else
		{
			if( rhs.kOrder < kOrder )
			{
				return false;
			}
		}

		return (kGuid < rhs.kGuid)?(true):(false);
	}

	BM::GUID	kGuid;
	PgQuestManUtil::EQuestMarkDrawOrder kOrder;

}SQuestMiniMapKey;

typedef struct tagQuestMiniMapInfo
{
	tagQuestMiniMapInfo()
		: iQuestID(0)
		, pkActor(NULL)
		, eState(QS_None)
	{};
	int iQuestID;
	PgActor* pkActor;
	EQuestState eState;
}SQuestMiniMapInfo;

typedef struct tagMiniMapRenderText
{
	tagMiniMapRenderText() : pText(NULL) {};
	~tagMiniMapRenderText(){ SAFE_DELETE(pText); };

	bool SetStr(std::wstring const& Text, std::wstring const& Font, DWORD const dwColor)
	{
		XUI::CXUI_Font* pkFont = g_kFontMgr.GetFont(Font);
		PG_ASSERT_LOG(pkFont);
		if( !pkFont ){ return false; }
		XUI::PgFontDef kFontDef(pkFont, dwColor);
		if( !pText )
		{
			pText = new Pg2DString(kFontDef, Text);
		}
		else
		{
			pText->SetText(kFontDef, Text);
		}

		return true;
	}
	
	void SetAttr(POINT2 const& Loc, NiColorA const& TextColor, NiColorA const& ShadowColor, bool const bAlways)
	{
		kLoc = Loc;
		kTextColor = TextColor;
		kShadowColor = ShadowColor;
		this->bAlways = bAlways;
	}

	void Render(PgRenderer* pkRenderer)
	{
		if( pText )
		{
			pText->Draw(pkRenderer, kLoc.x, kLoc.y, kTextColor, kShadowColor, true);
		}
	}

	POINT const GetSize() const { return pText->GetSize(); };
	bool const IsShow() const { return bShow; }
	bool const IsAlways() const { return bAlways; }
	void SetShow(bool bShow) { this->bShow = bShow; }
	bool const IsTextExist() { return pText != NULL; }

private:
	Pg2DString*	pText;
	POINT2		kLoc;
	NiColorA	kTextColor;
	NiColorA	kShadowColor;
	bool		bShow;
	bool		bAlways;
}SMiniMapRenderText;

typedef struct tagMiniMapIconInfo
{
	bool bDirty;
	POINT2 ptLastPos;
	NiPoint3 ptLastWorldPos;
	float fLastRotationRadian; // redundant?

	BM::GUID kObjectGuid;
	BM::GUID kLinkedObjectGuid;
	IconType kIconType;

	// for ani icon
	bool bIsAniIcon;
	EMapIconTexType kIconTexType;
	POINT2 kIconSize;
	POINT2 kLastIconTexturePos;

	// for icon delete
	NiScreenElements* pIconTexture;
	int iIconPolygonIndex;
	
	PgQuestManUtil::EQuestMarkDrawOrder kQuestMarkDrawOrder;
	// need something more?


	tagMiniMapIconInfo::tagMiniMapIconInfo()
		: bDirty(false)
		, fLastRotationRadian(0.f)
		, kIconType(ICONTYPE_NONE)
		, bIsAniIcon(false)
		, kIconTexType(EMITT_NONE)
		, pIconTexture(NULL)
		, iIconPolygonIndex(-1)
		, kQuestMarkDrawOrder(PgQuestManUtil::EQMDO_HIGH)
	{
		ptLastWorldPos.x = 0.f;
		ptLastWorldPos.y = 0.f;
		ptLastWorldPos.z = 0.f;
	}

} SMiniMapIconInfo;

typedef struct tagMiniMapIconUI
{
	tagMiniMapIconUI(int const _type=0)
		: iType(_type)
		, iIdx(0), iTTNo(0), iTargetIdx(0), iUVIndex(0)
		, W(0), H(0), iTransX(0), iTransY(0)
		, bUpdate(false)
	{};

	int iType; //PgTrigger::TriggerType
	int iIdx;
	int iTTNo;
	int iTargetIdx;
	int iUVIndex;
	NiPoint3 ViewPosition;
	int W, H;
	int iTransX, iTransY;
	bool bUpdate;
}SMiniMapIconUI;

typedef std::vector<SMiniMapAniIcon>	MiniMapAniIconContainer;
typedef std::map< int, MiniMapAniIconContainer > MiniMapAniIconCont;
typedef std::map<BM::GUID, SQuestOrderByResult>	MiniMapQuestCont;
typedef std::map< SQuestMiniMapKey, SQuestMiniMapInfo >	MiniMapQuestSortCont;
typedef std::map< UINT, SMapIconTexInfo >	kMapIconTexContainer;
typedef std::map< int, SIconTypeToIndex >		kMapIconToIdxContainer;
typedef std::map< int, SMiniMapRenderText >	kMapTextContainer;
typedef std::map< BM::GUID, SMiniMapIconInfo > MiniMapIconCont;
typedef std::map< BM::GUID, SMiniMapIconUI > MiniMapIconUICont;

namespace PgMiniMapUtil
{
	void UpdateStrongholdLink();
	void CallMiniMapIconInfoUI();
	void CloseMiniMapIconInfoUI();
	void Send_PT_C_M_REQ_ALARM_MINIMAP(POINT2 const& WndPos, POINT2 const& MousePos); // 미니맵 알람을 요청함
	void Recv_PT_M_C_NFY_ALARM_MINIMAP(BM::Stream & Packet);	// 우리 팀원이 미니맵 알람을 동작시킴
	void Recv_PT_M_C_NFY_TRIGGER_ONENTER(BM::Stream & Packet); // 상대 팀원이 진지 안으로 들어옴
	void Recv_PT_M_C_NFY_TRIGGER_ONLEAVE(BM::Stream & Packet); // 상대 팀원이 진지 밖으로 나감

	void ProcessMsg(unsigned short const usType, BM::Stream& rkPacket);
};

class PgHouse;
class PgDropBox;
class PgMiniMapBase
	: public PgUIDrawObject
{
public:
	typedef struct tagInitInfoBase
	{
		tagInitInfoBase() : kUISize(), pkCamera(NULL), kImgPath()
		{}
		POINT2 kUISize;
		NiCamera* pkCamera;
		std::string kImgPath;
	}S_INIT_INFO_BASE;
	bool BaseInitialize(S_INIT_INFO_BASE& kInitInfo);	//미니맵 설정
	void RenderFrame(NiRenderer* pkRenderer, POINT2 const& ptWndSize);	//화면에 그려질 텍스쳐를 설정
	virtual void RenderFrameUI(XUI::CXUI_Wnd * pkWnd);	//화면에 그려질 UI 설정
	virtual void SetTriggerIconUI(XUI::CXUI_Wnd * pkWnd, POINT2 const& ptWndPos);	//화면에 표시할 UI
	void Zoom(float const fZoom);
	void ShowMiniMap(bool bShow);
	
	virtual bool UpdateQuest() { return true; }
	virtual PgUITexture* GetTex();
	virtual void RefreshZoomMiniMap();
	virtual void Draw(PgRenderer* pkRenderer);	//실제 그리기
	virtual void Update(float const fAccumTime, float const fFrameTime);
	virtual void OnMouseOver(POINT2 const& ptWndPos, POINT2 const& ptMouse) const{};
	virtual void OnMouseClick(POINT2 const& ptWndPos, POINT2 const& ptMouse) const{};
	virtual E_MINIMAP_USE_TYPE const GetType() const = 0;
	virtual void Close();  // reserved;
	virtual void Terminate();

	virtual ~PgMiniMapBase(void);

	void SetRefreshAllMiniMapIconInfo(bool bRefresh)
	{
		m_bRefreshAllMiniMapIconInfo = bRefresh;
	}

public:	
	static void ReleaseAll();
	bool ParseMiniMapIconXml();	//아이콘 타입별 텍스쳐 정보 파싱하기
	void RemoveMiniMapIcon(BM::GUID const & rkGuid);

protected:
	typedef enum : BYTE
	{
		ERCR_OUT	= 0,
		ERCR_PART	= 1,
		ERCR_ALL	= 2,
	}E_RECT_CHECK_RESULT;

	PgMiniMapBase();//상속용도로만 쓰자.

	virtual void CustomizeRenderFrame(NiRenderer* pkRenderer, NiPoint2 const& kScreenSize, POINT2 const& ptWndPos) = 0;
	virtual POINT2 const IconDrawPosition(POINT2 const& ptWndPos, NiPoint3 const& ScreenPos) const;

	//아이콘이 얼마나 그려지는지 체크 및 영역 반환
	E_RECT_CHECK_RESULT const GetIconRect(RECT& rtIconRect, POINT2 const& ptWndPos, POINT2 const& ScreenPixel, int const iIconSize) const;
	E_RECT_CHECK_RESULT const GetIconRect(RECT& rtIconRect, POINT2 const& ptWndPos, POINT2 const& ScreenPixel, POINT2 const iIconSize) const;
	POINT2 const GetIconSize(EMapIconTexType const Type) const;	//아이콘의 기본 사이즈 반환(가로=세로)

	bool AddMiniMapEffectIcon(PgActor* pkActor, POINT2 const& ptWndPos);		//이펙트용
	bool AddMiniMapActorIcon(PgActor* pkActor, POINT2 const& ptWndPos);			//액터용
	bool AddMiniMapTriggerIcon(PgTrigger* pkTrigger, POINT2 const& ptWndPos);	//트리거
	bool AddMiniMapBrokenObjectIcon(PgPilot* pkPilot, POINT2 const& ptWndPos);	//오브젝트(지금은 파괴오브젝트만 들어옴)
	bool AddMiniMapMyHomeIcon(PgHouse* pkHouse, PgMyHome* pkHome, POINT2 const& ptWndPos);		//마이홈
	bool AddMiniMapDropBoxItemIcon(PgDropBox* pDropBox, POINT2 const& ptWndPos);	//드롭박스
	bool AddMiniMapQuestIcon(BM::GUID const& kGuid, NiPoint3 const& WorldPt, EQuestState const eState, POINT2 const& ptWndPos);	//퀘스트
	bool AddMiniMapIcon(BM::GUID const& kGuid, NiPoint3 const& kWorldPt, IconType const eIconType, POINT2 const& ptWndPos, float fRadian = 0.0f);	//아이콘 찾아서 그리기

	bool CreateScreenTexture(std::string const& kMiniMapImage);	//화면에 그릴 미니맵을 만든다
	bool GetMiniMapIcon(IconType const Type, POINT2& IconPos, EMapIconTexType& kType, POINT2& IconSize, NiScreenElements*& pIconTexture);	//아이콘 타입으로 텍스쳐 타입 및 사이즈 얻기
	void AddMapIconTex(char const* pPath, SMapIconTexInfo& rkInfo);		//아이콘 텍스쳐 등록
	bool ViewPtToScreenPt(NiPoint3 const& WorldPt, NiPoint3& ScreenPt) const;	//3D->2D 좌표 변환
	IconType const GetTriggerType(PgTrigger* pkTrigger) const;
	void ArrangeScreenBoundary();

	void ClearRemoveMiniMapIcon();
	void DoRemoveMiniMapIcon();
	void RefreshMiniMapIcon(BM::GUID const& rkGuid);

protected:
	//최종적으로 뿌리기 설정하는 곳
	bool addMiniMapIcon(POINT2 const& ptWndPos, POINT2 const& screenPixel, POINT2 const& iconTexturePos, SMiniMapIconInfo& kMiniMapIconInfo, float fAlpha = 1.0f, EMapIconTexType TexType = EMITT_SMALL, float fDir = 0.0f);

	CLASS_DECLARATION_S(POINT2, WindowPos);
	CLASS_DECLARATION_S_NO_SET(bool, ShowMiniMap);
	CLASS_DECLARATION_NO_SET(bool, m_bInitialized, Initialized);	//초기화 되었나
	CLASS_DECLARATION_NO_SET(float, m_fZoomFactor, ZoomFactor);	//줌
	CLASS_DECLARATION_S_NO_SET(POINT2, WndSize);	//출력UI사이즈
	CLASS_DECLARATION_S_NO_SET(NiPoint2, MiniMapSize);

protected:
	mutable Loki::Mutex m_kMutex;

	// 위치변환용 카메라(나중에 4x4행렬로 바꿔보자고 함)
	NiCameraPtr m_spCamera;	

	float m_fScreenImageRatio;	//이미지 비율
	POINT2 m_kMiniMapStartPt;	//미니맵 출력 시작 위치
	POINT2 m_kMiniMapEndPt;	//미니맵 출력 종료 위치
	NiPoint2 m_kScreenCenter;	//출력 중점
	int	m_iDrawGap;	//출력 종회비에 의한 차
	bool m_bRefreshAllMiniMapIconInfo;

	NiScreenElementsPtr m_spMiniMapScreenTexture;	//화면 출력 텍스쳐
	MiniMapAniIconCont m_kAniIconCont;	//아이콘 텍스쳐 에니메이션 컨트롤

	NiTexturePtr m_spMiniMapImage;	//미니맵 이미지
	MiniMapIconCont m_kMiniMapIconCont;
	VEC_GUID m_kRemoveMiniMapIconCont; //제거할 미니맵 아이콘

	static kMapIconTexContainer ms_kMapIconTexCont;	//아이콘 텍스쳐
	static kMapIconToIdxContainer ms_kMapIconTypeToIdxCont;	//아이콘 타입별 텍스쳐 정보
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PgAlwaysMiniMap
	: public PgMiniMapBase
{
public:
	typedef struct tagInitInfo : public PgMiniMapBase::S_INIT_INFO_BASE
	{
		tagInitInfo() : pkObjectCont(NULL), pkTriggerCont(NULL), pkHeightInfo(NULL), PgMiniMapBase::S_INIT_INFO_BASE()
		{}
		PgWorld::ObjectContainer* pkObjectCont;
		PgWorld::TriggerContainer* pkTriggerCont;
		POINT* pkHeightInfo;
	}S_INIT_INFO;

	typedef enum : BYTE
	{
		EIT_PLAYER				= 1,
		EIT_MONSTER				= 2,
		EIT_NPC					= 3,
		EIT_PARTY				= 4,
		EIT_COUPLE				= 5,
		EIT_GUILD				= 6,
		EIT_TRIGGER				= 7,
		EIT_BREAKOBJ			= 8,
		EIT_QUEST				= 9,
		EIT_BATTLESQUARE		= 10,
        EIT_GUARDIAN    		= 11,
        EIT_ITEM    		    = 12,
		EIT_END,
	}E_ICON_TYPE;

	bool Initialize(S_INIT_INFO& kInitInfo);
	virtual bool UpdateQuest();

	virtual void RefreshZoomMiniMap();
	virtual void Draw(PgRenderer* pkRenderer);
	virtual void Update(float const fAccumTime, float const fFrameTime);
	virtual void OnMouseOver(POINT2 const& ptWndPos, POINT2 const& ptMouse, char const* wndName) const;
	virtual void OnMouseClick(POINT2 const& ptWndPos, POINT2 const& ptMouse) const;
	virtual E_MINIMAP_USE_TYPE const GetType() const { return EMUT_ALWAYS_VIEW; }
	virtual void Terminate();

	void OnClickIconInfoListItem();
	bool GetViewFlag(E_ICON_TYPE const Type) const { return m_bShowFlag[Type]; }
	void SetViewFlag(E_ICON_TYPE const Type, bool const bView) { m_bShowFlag[Type] = bView; }

	POINT2 ActorToMinimapScreenPos( PgActor* pkActor, POINT2 const ptWndPos );
	bool GetMinimapIconInfo(CONT_MINIMAP_ICON_INFO& Cont);
	void SelectObject(std::wstring const& kObjectID);
	SMINIMAP_ICON_INFO const& SelectObject() const { return m_kSelectObject; }

	PgAlwaysMiniMap(void);
	virtual ~PgAlwaysMiniMap(void);

	static void DrawText(bool const bDraw) { m_bDrawText = bDraw; }
	static bool DrawText() { return m_bDrawText; }
	static void MapTeleMoveUseItem(bool const bUseItem) { m_bMapTeleMoveUseItem = bUseItem; }

	PgIWorldObject * GetObjectContainerByGUID(BM::GUID const & PlayerGuid) const;

	void UpdateStrongholdLink();
protected:
	virtual void CustomizeRenderFrame(NiRenderer* pkRenderer, NiPoint2 const& kScreenSize, POINT2 const& ptWndPos);
	virtual void RenderFrameUI(XUI::CXUI_Wnd * pkWnd);
	virtual void SetTriggerIconUI(XUI::CXUI_Wnd * pkWnd, const POINT2 &ptWndPos);
	virtual POINT2 const IconDrawPosition(POINT2 const& ptWndPos, NiPoint3 const& ScreenPos) const;

	bool AddMiniMapSelectObjectIconArrow(NiAVObject* pkObject, POINT2 const& ptWndPos);

	bool CheckQuestNotifyMarkingUnit(PgActor* pkActor, MiniMapQuestSortCont& kSortQuestCont);
	bool CheckQuestNotifyMarkingTrigger(PgTrigger* pkTrigger);
	NiAVObject* GetPickObject(SMINIMAP_ICON_INFO const& kObject);

	NiAVObject* OnOverPickActor(std::wstring const& kActorID) const;
	NiAVObject* OnOverPickTrigger(std::wstring const& kTriggerID) const;
	PgActor* OnOverPickActor(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType) const;
	std::wstring const OnOverPickParty(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType) const;
	PgTrigger* OnOverPickTrigger(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType) const;
	
	bool CheckMiniMapIconPicking(NiPoint3 const& kWorldPos, POINT2 const& ptWndPos, POINT const& pt, POINT2 const TexSize) const;
	bool AddDrawText(NiPoint3 const& kWorldPt, int const iTextNo, POINT2 const& ptWndPos, IconType const eIconType);
	PgQuestManUtil::EQuestMarkDrawOrder const GetQuestStateToDrawOrder(EQuestState const eState);

	bool CreateTriggerIconUI();
	void ClearTriggerIconUI();

protected:
	static BM::GUID const m_kSelectArrowGUID;
	mutable bool m_bIsShowToolTip;
	bool m_bShowFlag[EIT_END];
	SMINIMAP_ICON_INFO m_kSelectObject;
	CLASS_DECLARATION_S(bool, FixedSize);

protected:
	POINT	m_kDrawHeight;

	NiPoint3 m_kActorLastPos;
	PgWorld::ObjectContainer* m_pObjectContainer;
	PgWorld::TriggerContainer* m_pTriggerContainer;

	kMapTextContainer m_kMapRenderTextCont;	//화면 출력 텍스트
	MiniMapQuestCont m_kQuestInfoCont; //퀘스트 목록
	MiniMapIconUICont m_kContTriggerIconUI;

	static bool	m_bDrawText;	//텍스트 출력할건가
	static bool m_bMapTeleMoveUseItem;	//미니맵 이동 가능 플래그
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PgBookMiniMap
	: public PgMiniMapBase
{
public:
	virtual E_MINIMAP_USE_TYPE const GetType() const { return EMUT_BOOK_INFO; };

	PgBookMiniMap(void);
	virtual ~PgBookMiniMap(void);

protected:
	virtual void CustomizeRenderFrame(NiRenderer* pkRenderer, POINT2 const& ptWndSize);

	virtual bool ParseWorldMinimapInfo(TiXmlDocument const* pkDoc);

protected:

protected:

};

#endif // FREEDOM_DRAGONICA_CONTENTS_MINIMAP_PGMINIMAP_H