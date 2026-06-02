#ifndef FREEDOM_DRAGONICA_SCENE_UI_PGUISCENE_H
#define FREEDOM_DRAGONICA_SCENE_UI_PGUISCENE_H

#include "PGIScene.h"
#include "XUI/XUI_Manager.h"
#include "XUI/XUI_Font.h"
#include "PgRenderer.h"
#include "PgUIDrawObject.h"
#include "PgDamageNumMan.h"
#include "PgWorld.h"
#include "PgStat.h"
#include "XUI/XUI_TextTable_Base.h"
#include "NiGeometryGroup.H"

// xxxxxxxxxxxxxxxxxxxxxxxxxxx todo
#include "PgUiModel.h"
#include "CreateUsingNiNew.inl"

class lwTrigger;
class PgActor;
class PgCameraMan;
class PgToolTipBalloon;
class PgMiniMapUI;
class PgWorldMapUI;
class PgWorldMapPopUpUI;
class PgMiniMapBase;
extern NxVec3 g_kPlayerLoc;//todo 삭제
extern int const NULL_ITEM_NUM;	// NULL_ITEM_NUM

std::wstring const LINK_EDT_CHAR= (_T("CH"));
std::wstring const LINK_EDT_ITEM= (_T("IT"));

typedef enum : BYTE
{
	LOADING_IMG_DEFAULT = 0,
	LOADING_IMG_FILED	= 1,
	LOADING_IMG_RESULT,
	LOADING_IMG_MISSION,
	LOADING_IMG_PVP,
	LOADING_IMG_EMBATTLE,
	LOADING_IMG_DRAGON,
	LOADING_IMG_HIDDEN,
	LOADING_IMG_CHAOS,
	//LOADING_IMG_MYHOME,
	LOADING_IMG_PetLD,
	LOADING_IMG_NEW_RACE,
	LOADING_IMG_MAX,
}SLoadingImgType;

typedef std::list< std::string > LOADINGIMG_ID_LIST;

typedef struct _tagMissionImgInfo
{
	int iID;
	std::string strPath;
	std::string strExtension;

	_tagMissionImgInfo()
	{
		iID = 0;
		strPath.clear();
		strExtension.clear();
	}
} SMissionImgInfo;

class PgCursour
{
public:
	PgCursour(XUI::CXUI_Renderer_Base* pRenderer)
	:m_bHide(false)	
	{
		m_pRenderer = pRenderer;		
	}
	virtual ~PgCursour(){}
	
	bool Draw(float fFrameTime);
public:
	XUI::CXUI_Renderer_Base *m_pRenderer;	
	bool m_bShowInfoText;
	bool HideCursor(E_CURSOR_STATE const eAtThisState);
	bool ShowCursor(E_CURSOR_STATE const eAtThisState);
private:	
	bool m_bHide;
};

#define	D3DFVF_UIRECT	(D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR)

struct	stFVF_UIRECT
{
    D3DXVECTOR4 p;
	D3DCOLOR	kDiffuse;
	D3DCOLOR	kSpecular;
    D3DXVECTOR2 t;

	stFVF_UIRECT() : p(0,0,0,0),kDiffuse(0),kSpecular(0),t(0,0)
	{
	}
};

struct	stRenderOption
{
	NiColorA m_kOutlineColor;
	bool m_bExtendOutline;
	float	m_fScale;
	POINT2F m_kScaleCenter;

	float	m_fRotateAngle;
	POINT2F m_kRotationCenter;

	NiColorA m_kSpecular;


	stRenderOption(const NiColorA &kOutlineColor = NiColorA(0,0,0,0),
		bool bExtendOutline = false,
		float	fScale=1.0f,
		float	fRotateAngle=0.0f,
		const NiColorA &kSpecular = NiColorA(0,0,0,0)
		, POINT2F kScaleCenter = POINT2F(0.0f,0.0f)
		, POINT2F kRotationCenter = POINT2F(0.0f,0.0f))
										:m_kOutlineColor(kOutlineColor),
										m_bExtendOutline(bExtendOutline),
										m_fScale(fScale),
										m_fRotateAngle(fRotateAngle),
										m_kSpecular(kSpecular)
										, m_kScaleCenter(kScaleCenter)
										, m_kRotationCenter(kRotationCenter)
	{
	};

	bool	IsEqual(const stRenderOption& kOption)
	{
		if(m_kOutlineColor != kOption.m_kOutlineColor)	return	false;
		if(m_bExtendOutline != kOption.m_bExtendOutline)	return	false;
		if(m_fScale != kOption.m_fScale)	return	false;
		if(m_fRotateAngle != kOption.m_fRotateAngle)	return	false;
		if(m_kSpecular != kOption.m_kSpecular)	return	false;
		if(m_kScaleCenter != kOption.m_kScaleCenter)	return false;
		if(m_kRotationCenter != kOption.m_kRotationCenter)	return false;
		return	true;
	}
};

class	PgScreenTextureRendererDataGeomGroup : public NiGeometryGroup
{
	public:
		
	virtual void Purge()
	{
	};

	virtual void RemoveObject(NiGeometryData* pkData)
	{
	}

    virtual void RemoveObject(NiSkinPartition::Partition* pkPartition)
	{
	};

    virtual void AddObject(NiScreenTexture* pkScreenTexture)
	{
	};
    virtual void RemoveObject(NiScreenTexture* pkScreenTexture)
	{
		NiGeometryBufferData* pkBuffData = 
			(NiGeometryBufferData*)pkScreenTexture->GetRendererData();
		if (!pkBuffData)
			return;

		NiGeometryGroup::RemoveObject(pkBuffData);

		NiDelete pkBuffData;
		pkScreenTexture->SetRendererData(NULL);
	};
    virtual void AddObject(
        NiGeometryData* pkData, 
        NiSkinInstance* pkSkinInstance, 
        NiSkinPartition::Partition* pkPartition)
	{
	};
    virtual NiVBChip* CreateChip(NiGeometryBufferData* pkBuffData, 
        unsigned int uiStream)
	{
		return	NULL;
	};
    virtual void ReleaseChip(NiGeometryBufferData* pkBuffData, 
        unsigned int uiStream)
	{
	};

    virtual bool IsDynamic()
	{
		return	true;
	}
};
class PgScreenTextureRendererData : public NiGeometryBufferData
{
public:

	PgScreenTextureRendererData();
	virtual ~PgScreenTextureRendererData();
	virtual bool ContainsVertexData(NiShaderDeclaration::ShaderParameter 
		eParameter) const;

	int	GetTotalVerts()	const	{	return	m_iTotalVerts;	}
	void	SetTotalVerts(int iVerts)	{	m_iTotalVerts = iVerts;	}
	stFVF_UIRECT	*GetVertexBuffer()	const	{	return	m_pkVertexBuffer;	}
	void	SetVertexBuffer(stFVF_UIRECT *pkNewBuff)
	{
		if(m_pkVertexBuffer)
		{
			delete []m_pkVertexBuffer;
		}

		m_pkVertexBuffer = pkNewBuff;
	}
	LPDIRECT3DVERTEXBUFFER9	GetD3DVertexBuffer()	{	return	m_lpVertexBuffer;	}
	void	SetD3DVertexBuffer(LPDIRECT3DVERTEXBUFFER9 lpVertexBuffer)
	{
		if(m_lpVertexBuffer)
		{
			m_lpVertexBuffer->Release();
		}

		m_lpVertexBuffer = lpVertexBuffer;
	}


	void	SetRenderOption(const stRenderOption&	kRenderOption)
	{
		m_kRenderOption = kRenderOption;
	}
	stRenderOption&	GetRenderOption()
	{
		return	m_kRenderOption;
	}

private:

	int	m_iTotalVerts;
	LPDIRECT3DVERTEXBUFFER9	m_lpVertexBuffer;
	stFVF_UIRECT	*m_pkVertexBuffer;
	stRenderOption	m_kRenderOption;

};


typedef struct tagUIActinByDist 
{
	float			m_fDist;
	POINT3			m_kOriginPoint;

	std::wstring	m_wstrScript;
	std::wstring	m_wstrUIName;

	bool			m_bRegistDelete;

	tagUIActinByDist()
	{
		Clear();
	}

	void Clear()
	{
		m_fDist = 0;
		m_kOriginPoint = POINT3(0.0f,0.0f,0.0f);
		m_wstrScript.clear();
		m_wstrUIName.clear();
		m_bRegistDelete = false;
	}

	virtual bool Update(const POINT3& rkPlayerPos)
	{
		if(m_bRegistDelete)
		{
			return true;
		}

		const POINT3 kDis = rkPlayerPos-m_kOriginPoint;
		if (POINT3::Distance(rkPlayerPos,m_kOriginPoint) >= m_fDist )
		{
			if (!m_wstrScript.empty())
			{
				lua_tinker::call<void, char const*>(MB(m_wstrScript), MB(m_wstrUIName));
			}
			return true;
		}

		return false;
	}

}SUIActinByDist;

typedef std::map<std::wstring const, SUIActinByDist> UIActinByDistMap;

class	PgMobileSuit;
//template<typename T>
class PgUIScene
	: public PgIScene
	, public XUI::CXUI_Renderer_Base
	, public CXUI_Resource_Manager_Base		// todo namespace
	, public XUI::CXUI_TextTable_Base
	, public XUI::CXUI_HotKeyModifier
{
	//friend struct CreateUsingNiNew< PgUIScene >;	
	friend	class	PgMobileSuit;

public:

	typedef	std::map<std::wstring,stUIOffscreen*> UIOffscreenMap;
	typedef	std::list<stUIOffscreen*> UIOffscreenList;

private:
	
	bool	m_bUseOffscreenDraw;
	UIOffscreenMap	m_kOffscreenMap;
	UIOffscreenList	m_kOffscreenDrawList;
		
public:
	PgUIScene(void);
	virtual ~PgUIScene(void);
	
	virtual	void Destroy()	{};
	bool Initialize();

	bool LoadUIPath(char const *pcPathXml);
	bool LoadXUIXML(std::wstring const &wstrFileName);
	bool Build();

	void Draw(PgRenderer *pkRenderer, float fFrameTime);//! 최상단 UI를 렌더링 한다.
	bool Update(float fAccumTime, float fFrameTime);//! 최상단 UI를 최신 상태로 유지한다.
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);//! XML 'UI' 노드를 파싱한다.
	virtual bool ProcessInput(PgInput *pkInput);//! 입력을 처리한다.
	virtual bool RenderText(const SRenderTextInfo &rRenderTextInfo);//! XUI override
	virtual bool RenderSprite(void* &pSprite, int &iScrIndex, const SRenderInfo &rRenderInfo, bool bInsertList = true);//! XUI override
	virtual bool RenderModel(char const *pcModelName, const POINT2 &ptWndSize, const POINT2 &ptWndPos, NiActorManager *pkActorManager, NiAVObject *pkModelRoot = 0);
	virtual	bool RenderStaticForm(void *&pkStaticForm, const SRenderInfo &rRenderInfo, bool bInsertList = true);

	static	void	Begin_RenderUIObject(PgRenderer *pkRenderer);
	static	void	End_RenderUIObject();

	static	void	Render_UIObject(PgRenderer *pkRenderer,NiObject *pkAVObject,DWORD const dwType = PgRenderer::E_PS_NONE,const stRenderOption &kRenderOption=stRenderOption());
	static	void	Render_ScreenTexture(PgRenderer *pkRenderer,NiScreenTexture* pkScreenTexture,const stRenderOption &kRenderOption=stRenderOption());
	static	void	Render_ScreenElements(PgRenderer *pkRenderer,NiScreenElements* spScreenElements);
	static	void	Render_DynamicTexture(HDC kDC,int iScreenWidth,int iScreenHeight,PgDynamicTexture *pkDTexture,NiScreenTexture* pkScreenTexture,const stRenderOption &kRenderOption=stRenderOption());

	static	void	Pack_ScreenTexture(PgRenderer *pkRenderer,NiScreenTexture* pkScreenTexture,const stRenderOption &kRenderOption=stRenderOption());

	void	EnableOffScreenRendering(bool bEnable);
	void	SetUpdateEveryOffscreen();
	
	PgMiniMapBase* GetMiniMapUI(std::string strMiniMapName);
	void UpdateMinimapQuest();
	void TerminateMiniMap(std::string strMiniMapName);
	void TerminateAllMiniMap();
	bool SetWorldDataForMiniMap(std::string strMiniMapName, PgWorld::ObjectContainer* pObjectContainter, PgWorld::TriggerContainer* pTriggerContainer, NiCameraPtr worldCamera, std::string& kMiniMapImage, POINT2 const& WndSize, POINT* pDrawHeight);
	bool SetWorldDataForAllMiniMap(PgWorld::ObjectContainer* pObjectContainter, PgWorld::TriggerContainer* pTriggerContainer, NiCameraPtr worldCamera, std::string& kMiniMapImage, POINT* pDrawHeight);
	void SetRefreshAllMiniMap();
	bool InitMiniMap(std::string const& strMiniMapName, const POINT2 &ptWndSize, const POINT2 &ptWndPos, int const InitType);
	bool RenderMiniMap(std::string const& strMiniMapName, const POINT2 &ptWndPos);
	bool RenderMiniMapUI(std::string const& strMiniMapName, XUI::CXUI_Wnd *pWnd);
	bool SetMiniMapTriggerIconUI(std::string strMiniMapName, XUI::CXUI_Wnd *pWnd, POINT2 const& ptWndPos);
	bool ShowMiniMap(std::string strMiniMapName, bool bShow);
	bool CloseMiniMap(std::string strMiniMapName);
	void RecreateMiniMap(std::string strMiniMapName, POINT2 const& WndSize);
	void RecreateAllMiniMap(bool bResetAll = false);
	bool SetWndSize(std::string strMiniMapName, POINT2 kWndSize);
	PgMiniMapBase* CloneMiniMap(std::string strSrcMiniMap, std::string strDestMiniMap, POINT2 kWndSize);
	bool RemoveMiniMapIcon(BM::GUID const & rkGuid, std::string const & strMiniMapName=std::string());

	// UI상에 3d 모델을 렌더링 관련
	bool InitRenderModel(std::string const &strRenderModel, const POINT2 &ptWndSize, const POINT2 &ptWndPos, bool bUpdate = true, bool bOrtho = false);
	void AddToDrawListRenderModel(std::string const &strRenderModel);
	PgUIModel* FindUIModel(std::string const &strRenderModel);	


	void RecreateOffscreen();
	PgWorldMapUI* GetWorldMapUI() { return m_pkWorldMapUI; }
	void SetWorldMapUI(PgWorldMapUI* pkWorldMapUI) { m_pkWorldMapUI = pkWorldMapUI; }
	PgWorldMapPopUpUI* GetWorldMapPopUpUI() { return m_pkWorldMapPopUpUI; }
	void SetWorldMapPopUpUI(PgWorldMapPopUpUI* pkWorldMapPopUpUI) { m_pkWorldMapPopUpUI = pkWorldMapPopUpUI; }

	void UpdateQuest();

	virtual bool VPrepareResource(std::wstring const &rKey);
	virtual void* VCreateResource(std::wstring const &rKey);	//! XUI override
	virtual void* VCreateStaticFormUIDrawObject(void *pkStaticForm);
	virtual bool VReleaseStaticFormUIDrawObject(SPRITE_HASH::mapped_type &pRsc);
	virtual void* VCreateIconResource(int const IconNo, RECT rectIcon,bool bGrayScale = false);
	virtual bool VReleaseRsc(SPRITE_HASH::mapped_type &pRsc);	//! XUI override
	virtual void* VCreateOffscreen(const POINT2& ptSize, int staticNum, DWORD bgColor, float fAlpha);		//! XUI override
	virtual bool VReleaseOffscreen(SPRITE_HASH::mapped_type &pRsc);
	virtual bool RenderSpriteToOffscreen(void* &pOffscreen, void* &pSprite, int &iScrIndex, const SRenderInfo &rRenderInfo);//! XUI override
	virtual bool InvalidateOffscreen(void* &pOffscreen);
	virtual bool InvalidateOffscreen(void* &pOffscreen, RECT& rectInvalidate);
	virtual bool EndRenderOffscreen(void* &pOffscreen, void** ppImages, int iNumImages, bool bClearOffscreen = true);
	virtual bool EndRenderOffscreen(void* &pOffscreen, std::list<std::pair<void*,bool>>& rkDrawList, bool bClearOffscreen = true);
	virtual bool RenderOffscreen(void* &pOffscreen);
	
	void ResetUI();

	// xxxxxxxxxxxx땜빵
	PgUIModel *GetRenderModel(char const *pcModelName);
	static bool CALLBACK ScriptCallback( std::string const &rScript, XUI::CXUI_Wnd *pWnd);//! XUI Script callbacker
	static bool CALLBACK CursorCallBackFunc(CXUI_Wnd *pkCursor, const POINT2& kPtPoint, const SIconInfo &kIconInfo, const eInputEventIndex kBtn1State, const eInputEventIndex kBtn2State);//! XUI Script callbacker
	static bool CALLBACK TextClickCallBackFunc(XUI::CXUI_Wnd* pWnd,int iButton,std::wstring const &kOriginalString,PgExtraDataPack const &kExtraDataPack,bool bIsDoubleClick);

	static DWORD CALLBACK HotKeyToRealKey(DWORD const& dwHotKey);
	static void CALLBACK UniqueExistUICallFailNotice(std::wstring const& kType);

	static	void	XUIManagerCallBackFunc(CXUI_Wnd *pkWnd,CXUI_Manager::CallBackEventType kEventType);

	virtual const TCHAR *GetTextW(unsigned long ulTextNo)const;

	void	AddToDrawList(PgUIDrawObject *pkDrawObject,bool bPushFront = false);
	void	RemoveFromDrawList(PgUIDrawObject *pkDrawObject);
	void	ClearDrawLIst();

	//아이콘매니저 기능
	bool AddIconRsc(int const IconNo, std::wstring const &wstrRscName, const SUVInfo &rkUvInfo);//리소스 추가.
	void RenderIcon(int const IconNo, const POINT2 &ptPos, bool const bIs48siz = false, unsigned short sW=40, unsigned short sH=40,bool bGrayScale = false);
	bool	GetIconInfo(int const iIconNo,
		bool const bIs48siz,
		unsigned short sW, 
		unsigned short sH,
		NiTexture *&pkTexture,
		float &fU,float &fV,
		float &fW,float &fH);


	void RenderIcon(int const IconNo, const SRenderInfo &rRenderInfo);

	void	DrawFillRect(PgRenderer *pkRenderer,int iDX,int iDY,int iWidth,int iHeight,const NiColorA &kColor);
	PgUIUVSpriteObject* GetIconTexture(int const iItemNo);//

	void RegistESCScript( char const *pcScriptName )	{	m_kEscKeyScript = std::string( pcScriptName );	}
	void ClearESCScript()	{	m_kEscKeyScript.clear();	}

	void RemoveModel(char const *pcModelName);

	std::string GetMissionMapPath(int iID);


	bool RegistUIAction(std::wstring const &rkName, SUIActinByDist &rkItem);
	bool RegistUIAction(std::wstring const &rkName, std::wstring const &rkScript, NiPoint3 const &pt3);
	bool RegistUIAction(std::wstring const &rkName, std::wstring const &rkScript, NiPoint3 const &pt3, float fDist);
	bool DelUIAction(std::wstring const &rkName);
	void DelAllUIAction();

	bool IsAlwaysMinimapDraw() {return m_bAlwaysMinimapDraw;}
protected:

	void Terminate();

	void	UpdateOffscreen(PgRenderer *pkRenderer,float fFrameTime);
	void	DrawOffscreen(PgRenderer *pkRenderer,float fFrameTime);
	void	ClearAllOffscreen();
	stUIOffscreen*	DrawUIToOffscreen(PgRenderer *pkRenderer,CXUI_Wnd *pkWnd);

protected:
	bool CallESCScript();

	DrawList m_kDrawList;
	UIActinByDistMap m_kUIDistActionList;

	typedef struct tagRenderObject
	{
		typedef enum 
		{
			OT_SPRITE = 1,
			OT_FONT = 2,
		} eOBJECT_TYPE;

		tagRenderObject(eOBJECT_TYPE InType, void *pInObj)
		{
			eType = InType;
			pObj = pInObj;
		}
		eOBJECT_TYPE eType;
		void *pObj;
	} RENDER_OBJECT;

	//typedef std::map<std::wstring, NiSourceTexture*> SourceTextureContainer;
#ifdef PG_USE_ELE_TEXTURE
	typedef std::list<PgUITexture *> SpriteContainer;
#else
	typedef std::list<NiScreenTexture *> SpriteContainer;
#endif
	typedef std::list<PgUISpriteObject *> SpriteObjContainer;
	typedef std::list<PgUIRenderedSpriteObject *> RenderedSpriteObjContainer;
	typedef std::list<Ni2DString *> TextContainer;
	typedef std::map<unsigned int, std::string> InputContainer;
	typedef std::list<NiScreenTexture *> RenderObjContainer;//기본 리소스는 맵으로 관리.
	typedef std::map<std::string, PgUIModel *> UIModelContainer;
	typedef std::map<std::string, PgMiniMapBase *> MiniMapContainer;

	//SourceTextureContainer m_kTextureContainer;
	SpriteContainer m_kSpriteContainer;
	SpriteObjContainer m_kSpriteObjContainer;
	RenderedSpriteObjContainer m_kRenderedSpriteObjContainer;
	TextContainer m_kTextContainer;
	InputContainer m_kInputContainer;
	RenderObjContainer m_kRenderContainer;
	UIModelContainer m_kUIModelContainer;
	MiniMapContainer m_kMiniMapContainer;
	PgWorldMapUI* m_pkWorldMapUI;
	PgWorldMapPopUpUI* m_pkWorldMapPopUpUI;

	PgRenderer *m_pkRenderer;
	PgCursour m_kCursor;

	typedef std::map< int, PgUISpriteObject* > CONT_IMG;
	typedef std::map< int, PgUISpritePoolObject::SprObjlist* > SpritePool;

	CONT_IMG m_mapImg;	
	PgUITextPoolObject::CONT_TEXT_POOL m_kTextPool;
	NiRenderTargetGroupPtr m_spRenderTargetGroup;
	NiRenderedTexturePtr m_spRenderTexture;
	NiScreenTexturePtr m_spScreenTexture;

	std::string m_kEscKeyScript;		// ESC키를 호출하는 스크립트가 등록이 되있으면 이함수를 호출한다.

	SpritePool m_kSpritePool;//번호로 매겨진 스프라이트들의 풀
	//	BM::TObjectPool< PgUITextObject > m_kTextObjPool;
	//	BM::TObjectPool< PgUIDrawObject* > m_kSpriteObjPool;

	typedef std::map<int, SMissionImgInfo> MissionImgCont;
	typedef MissionImgCont::iterator MissionImgContIt;
	MissionImgCont m_kMissionImgCont;

	bool ParseXmlForMission(char const* pcFileName);

	void	AddScreenTextureToSpriteCont(NiScreenTexture *pkScreenTexture);
	void	RemoveScreenTextureFromSpriteCont(NiScreenTexture *pkScreenTexture);

private:
	bool m_bOldKeyLog[NiInputKeyboard::KEY_TOTAL_COUNT];	// 이전 상태

	void	RenderImeInfo(PgRenderer *pkRenderer, float fFrameTime);	//	IME 정보를 렌더링 한다.
	void	RenderCandList(PgRenderer *pkRenderer, float fFrameTime,int iDX,int iDY);
	void	RenderCandListVertical(PgRenderer *pkRenderer, float fFrameTime,int iDX,int iDY,std::wstring const &kCompString);
	PgToolTipBalloon	*m_pImeNameTextBalloon;
	PgToolTipBalloon	*m_pImeCandidateTextBalloon;
	PgToolTipBalloon	*m_pImeCompReadTextBalloon;
	PgToolTipBalloon	*m_pImeCompStrTextBalloon;
	bool	m_bImeToolTipCursor;
	int	m_iImeToolTipCursorToggleTime;
	float m_fLastUpdateTime,m_fLastDrawTime;
	bool m_bInsideDraw;
	
	NiScreenTexturePtr	m_spFillRect;	//	채워진 사각형을 그릴때 사용된다.

	bool AddIconRsc(int const resNo);

private:
	typedef std::map<int, std::string> UILoadingImageMap;
	typedef std::map< SLoadingImgType, std::string > UIBasicLoadingImageMap;
	typedef std::map<int, std::string> UITriggerLoadingImageMap;
	UILoadingImageMap		m_kLoadingImgContainerTargetMap;
	UILoadingImageMap		m_kLoadingImgContainerSourceMap;
	UIBasicLoadingImageMap	m_kBasicLoadingImgContainer;
	UITriggerLoadingImageMap m_TriggerLoadingImgContainer;
	// Input Hooking
	bool (*m_pInputHookFunc)(int const,XUI::E_INPUT_EVENT_INDEX);	
	
	bool  m_bMouseMove;
	DWORD m_dwLastMouseMoveTime;
	bool m_bAlwaysMinimapDraw;

public:
	void InputHook( bool (*pInputHookFunc)(int const,XUI::E_INPUT_EVENT_INDEX) );

	bool ParseXmlForLoadingImg(char const* pcszFileName);
	char const* FindLoadingImg(int iMapIndex, EGndAttr const eMapAttr, int LoadingImageID = 0, bool bIsRidingPet = false, int iOldMapNo = 0);
	void GetLoadingImgeIDList(LOADINGIMG_ID_LIST& List);
};

extern	PgUIScene	g_kUIScene;

#endif // FREEDOM_DRAGONICA_SCENE_UI_PGUISCENE_H