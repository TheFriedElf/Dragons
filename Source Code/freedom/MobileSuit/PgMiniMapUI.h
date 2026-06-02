#pragma once

#include "PgActor.h"
#include "PgUIDrawObject.h"
#include "PgIWorldObject.h"
#include "PgWorld.h"
#include "PgQuestMan.h"
#include "Pg2DString.h"

#define MINIMAP_ZOOMFACTOR_MAX		(0.8f)
#define MINIMAP_ZOOMFACTOR_MIN		(0.0f)

extern bool g_bAbleSetTeleMove;

typedef enum
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
	ICONTYPE_EMPORIA_MAIN_RED			= 34,	// ? (파랑)
	ICONTYPE_EMPORIA_MAIN_BLUE			= 35,	// ! (파랑)
	ICONTYPE_EMPORIA_SUB_RED			= 36,	// ? (녹색)
	ICONTYPE_EMPORIA_SUB_BLUE			= 37,	// ! (녹색)
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
	ICONTYPE_MAX_NUM,
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
		W = W;
		H = H;
		U = U;
		V = V;
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
		, Index(0)
		, TexType(EMITT_NONE)
	{};
	tagMapIconTypeToIndex(IconType Type)
		: Type(Type)
		, Index(0)
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
}SIconTypeToIndex;

typedef struct tagMiniMapIcon
{
	tagMiniMapIcon()
	{
		viewPosition = NiPoint3::ZERO;
		pData = NULL;
		iconType = ICONTYPE_NONE;
	}

	POINT2 screenPixel;
	NiPoint3 viewPosition;
	void* pData;
	IconType iconType;
} MiniMapIconData;

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
		: pkActor(NULL)
		, eState(QS_None)
	{};
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
	
	void SetAttr(POINT2 const& Loc, NiColorA const& TextColor, NiColorA const& ShadowColor, bool const Always = false)
	{
		kLoc = Loc;
		kTextColor = TextColor;
		kShadowColor = ShadowColor;
		bAlways	= Always;
	}

	void Render(PgRenderer* pkRenderer)
	{
		if( pText )
		{
			pText->Draw(pkRenderer, kLoc.x, kLoc.y, kTextColor, kShadowColor, true);
		}
	}

	POINT const GetSize() const { return pText->GetSize(); };
	bool const IsAlways() const { return bAlways; }

private:
	Pg2DString*	pText;
	POINT2		kLoc;
	NiColorA	kTextColor;
	NiColorA	kShadowColor;
	bool		bAlways;
}SMiniMapRenderText;

//typedef struct tagMiniMapQuest
//{
//	tagMiniMapQuest()
//	{
//		pkActor = 0;
//	}
//
//	PgActor* pkActor;
//	EQuestState eState;
//} SMiniMapQuest;

//typedef std::map<std::string, PgTrigger *> TriggerContainer;
//typedef std::map<BM::GUID, PgIWorldObject *> ObjectContainer;
typedef std::vector<SMiniMapAniIcon>	MiniMapAniIconContainer;
typedef std::vector<MiniMapIconData>	MiniMapIconContainer;
typedef std::map<BM::GUID, EQuestState>	MiniMapQuestCont;
typedef std::map< int, MiniMapAniIconContainer > MiniMapAniIconCont;
typedef std::map< SQuestMiniMapKey, SQuestMiniMapInfo >	MiniMapQuestSortCont;
typedef std::map< UINT, SMapIconTexInfo >	kMapIconTexContainer;
typedef std::set< SIconTypeToIndex >		kMapIconToIdxContainer;
typedef std::map< int, SMiniMapRenderText >	kMapTextContainer;

class PgRenderer;

class PgMiniMapUI 
	: public PgUIDrawObject
{
public:
	PgMiniMapUI(const POINT2 &ptWndSize);
	virtual ~PgMiniMapUI();

	void Terminate();

	//! Screen Element를 초기화
	bool Initialize(NiAVObjectPtr pkModelRoot, PgWorld::ObjectContainer* pObjectContainter, 
        PgWorld::TriggerContainer* pTriggerContainer, NiCameraPtr pkWorldCamera, 
        std::string& kMiniMapImage, POINT* pDrawHeight);

	//! 렌더링
	void RenderFrame(NiRenderer *pkRenderer, const POINT2 &pt);

	//! 카메라를 가져온다.
	NiCameraPtr GetCamera();

	//! 모델을 업데이트 한다.
	bool UpdateModel(NiCameraPtr pkWorldCamera);

	//! UpdateQuest
	bool UpdateQuest();

	//! WorldObject를 fFrameTime(AccumTime)에 대한 시각으로 갱신
	bool Update(float fAccumTime, float fFrameTime);

	//! pkRenderer를 이용해서 Draw
	void Draw(PgRenderer *pkRenderer);
	virtual PgUITexture* GetTex();
	void SetAlwayType(bool Type) { m_bAlwaysMinimap = Type; }
	bool const GetAlwayType() { return m_bAlwaysMinimap; }

	//!
	POINT2 GetWndSize();

	//! 모델 루트를 반환한다.
	NiAVObjectPtr GetModelRoot();

	NiScreenElementsPtr GetMiniMapScreenTexture();
	NiScreenTexturePtr GetModelScreenTexture();
	NiRenderTargetGroupPtr GetRenderTargetGroup();
	NiRenderedTexturePtr GetRenderTexture();
	NiTexturePtr GetMiniMapImage();
	PgWorld::ObjectContainer* GetObjectContainer();
	PgWorld::TriggerContainer* GetTriggerContainer();
	MiniMapIconContainer GetMapIcons();
	MiniMapIconContainer GetMapLeftIcons();
	MiniMapIconContainer GetMapRightIcons();
	kMapIconTexContainer& GetMapIconTexCont();

	//! 모델의 바운드를 찾는다.
	void GetWorldBounds(NiAVObject *pkObject, NiBound &kBound);

	//! 노드의 바운드를 찾는다.
	void GetWorldBoundsNode(NiNode *pkObject, NiBound &kBound);
	void RenderImmediate(NiRenderer* pkRenderer, NiAVObject *pkObject);

	bool GetInitialized() { return m_bInitialized; }
	void GetMiniMapOptions(bool& bShowNPC, bool& bShowPartyMemeber, bool& bShowGuildMemeber)
	{
		bShowNPC = m_bShowNPC;
		bShowPartyMemeber= m_bShowPartyMember;
		bShowGuildMemeber= m_bShowGuildMember;
	}
	void SetMiniMapOptionShowNPC(bool bShow) { m_bShowNPC = bShow; }
	void SetMiniMapOptionShowPartyMemeber(bool bShow) { m_bShowPartyMember = bShow; }
	void SetMiniMapOptionShowGuildMemeber(bool bShow) { m_bShowGuildMember = bShow; }

	void RefreshZoomMiniMap();
	void ZoomInMiniMap(int zoomLevel);	
	void ZoomOutMiniMap(int zoomLevel);
	void ScrollMiniMap(NiPoint2 direction);

	PgTrigger* GetTriggerInMiniMap(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType);
	PgActor* GetObjectInMiniMap(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType);
	void MouseOverMiniMap(const POINT2 &ptWndPos, const POINT2 &pt, char const* wndName);
	void MouseClickMiniMap(const POINT2 &ptWndPos, const POINT2 &pt);
	void CloneFromSrc(PgMiniMapUI* pkSrcMiniMap);
	std::wstring GetObjectInMiniMapParty(const POINT2 &ptWndPos, const POINT2 &pt, EMapIconTexType TexType);

	void SetWndSize(POINT2 kWndSize);
	std::string& GetMiniMapImageName();
	float GetZoomFactor() { return m_fZoomFactor; }
	void SetZoomFactor(float fZoomFactor);

	void SetShowActor(int iMode)
	{ 
		if (iMode < 0 || iMode > 3)
			m_iShowActor = 0;
		else
			m_iShowActor= iMode;
	}

	static void ShowText(bool bIsShow){ m_bIsShowText = bIsShow; };

private:
	void CheckAddIconToMyHome(PgPilot* pkPilot, POINT2 const& ptWndPos);
	void copyTextureToBuffer(NiTexture* pTexture);
	void arrangeScreenBoundary();
	bool addPositionToMiniMap(NiPoint3 const& WorldPt, POINT2 const& ptWndPos, IconType eIconType = ICONTYPE_PARTY);
	bool addOtherWorldActorToMiniMap(PgActor* actor, const POINT2 &ptWndPos, IconType eIconType = ICONTYPE_PARTY);
	bool addActorToMiniMap(PgActor* actor, const POINT2 &ptWndPos, IconType eIconType = ICONTYPE_NONE);
	bool addTriggerToMiniMap(PgTrigger* actor, const POINT2 &ptWndPos, IconType eIconType = ICONTYPE_NONE);
	//bool addQuestToMiniMap(const SMiniMapQuest& rkQuest, const POINT2 &ptWndPos, IconType eIconType = ICONTYPE_NONE);
	bool addQuestToMiniMap(PgActor* pkActor, const EQuestState eState, const POINT2 &ptWndPos, IconType eIconType = ICONTYPE_NONE);
	bool GetIconRect(RECT& rtIconRect, const POINT2& ptWndPos, const POINT2& screenPixel, EMapIconTexType TexType);
	void addMiniMapIcon(const POINT2& ptWndPos, const POINT2& screenPixel, const POINT2& iconTexturePos, float fAlpha = 1.0f, EMapIconTexType TexType = EMITT_SMALL, float fDir = 0.0f);
	IconType getIconType(PgActor* actor);
	IconType getIconType(PgTrigger* trigger);
	bool getIconTexturePosByType(IconType iconType, POINT2& rkOutPoint);
	bool viewPtToScreenPt(NiPoint3 worldPt, NiPoint3& screenPt);
	void updateCameraPosition();

	int m_iShowActor;	// 0: original, 1: show all(pc/mon) 2: show mon, 3: show pc
	bool m_bInitialized;
	bool m_bShowNPC;
	bool m_bShowPartyMember;
	bool m_bShowGuildMember;
	bool m_bShowTrigger;
	bool m_bShowQuest;
	NiAVObjectPtr m_spRoot;
	NiCameraPtr m_spCamera;
	NiScreenElementsPtr m_spMiniMapScreenTexture;
	NiScreenTexturePtr m_spModelScreenTexture;
	NiRenderTargetGroupPtr m_spRenderTargetGroup;
	NiRenderedTexturePtr m_spRenderTexture;	
	NiTexturePtr m_spMiniMapImage;
	PgWorld::ObjectContainer* m_pObjectContainer;
	PgWorld::TriggerContainer* m_pTriggerContainer;

	NiPoint3 m_kWorldCenter;
	float m_fWorldRadius;
	float m_fZoomFactor;
	float m_fScreenImageRatio;
	POINT2 m_kWndSize;	
	NiPoint2 m_kScreenCenter;
	bool m_bAlwaysMinimap;
	bool m_bUseMiniMapImage;
	MiniMapIconContainer m_kMapIcons;
	MiniMapIconContainer m_kMapLeftIcons;
	MiniMapIconContainer m_kMapRightIcons;
	POINT*	m_pkDrawHeight;
	NiPoint2 m_kDrawHeight;
	int	m_iMiniMapDrawGap;

	MiniMapAniIconCont	m_kAniIconCont;

	// 미니맵을 파일로 로드 할 때.. 파일 이름(보통 TGA)
	std::string m_kMiniMapImageName;

	// Quest Container
	MiniMapQuestCont m_kQuestInfoCont;

	bool m_bIsShowToolTip;
	std::string m_strToolTipID;

	kMapIconTexContainer		m_kMapIconTexCont;
	kMapTextContainer			m_kMapRenderTextCont;
	static kMapIconToIdxContainer	m_kMapIconTypeToIdxCont;
	void AddMapIconTex(char const* pPath, SMapIconTexInfo& rkInfo);
	bool ParseMiniMapXml();

	static bool m_bIsShowText;

	PgTextObjectPtr	m_kTextObject;
};
