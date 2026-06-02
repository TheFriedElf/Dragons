#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUI_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUI_H

#include "PgScripting.h"
#include "lwGUID.h"
#include "lwWString.h"
#include "lwPacket.h"
#include "lwPoint2.h"
#include "lwPoint2F.h"
#include "lwPoint3.h"
#include "lwActor.h"
#include "lwUnit.h"
#include "XUI/XUI_Manager.h"
#include "PgSkillTree.h"
#include "PgUtilAction.h"

typedef enum eToolTipCategory
{
	TTC_None		= 0,
	TTC_Community	= 11,
} EToolTipCategory;

typedef enum E_KCIP_USE_CONTENTS_TYPE
{
	EKUCT_DEFAULT		= 0,
	EKUCT_CRAFT			= 1,
	EKUCT_MON_CARD_T2,
};

typedef enum E_IMG_ICON_TYPE
{
	EIIT_CUSTOM_INV_POS	= 0,
};

typedef enum E_TOOLTIP_FLAG
{
	TTF_NONE			= 0x0,
	TTF_NOT_AMONUT		= 0x1, //수량나누기 표시 안함
	TTF_NOT_METHOD		= 0x2, //아이템사용 방법 메시지 표시 안함
};

typedef enum eCommonClassEquipItem
{// 모든직업 장착하는 아이템류인가 ( GetCommonClassEquipItemType 함수 참고 )
	ECCE_NOT_COMMON			= 0,	// 모든직업 장착하는 아이템이 아님
	ECCE_ALL_HUMAN_CLASS	= 1,	// 모든 인간이 장착가능한 아이템
	ECCE_ALL_DRAGONIAN_CLASS= 2,	// 모든 용족이 장착가능한 아이템
	ECCE_ALL_CLASS = 255,
};

struct SToolTipFlag
{
public:
	SToolTipFlag(E_TOOLTIP_FLAG kFlag=TTF_NONE):m_Flag(kFlag){}
	~SToolTipFlag(){}

	bool Enable(E_TOOLTIP_FLAG kFlag) const
	{
		return (m_Flag&kFlag);
	}
private:
	E_TOOLTIP_FLAG m_Flag;
};

int const iEquipArrSize = 15;
int const iEquipArr[iEquipArrSize] = {	EQUIP_POS_CLOAK, EQUIP_POS_WEAPON, EQUIP_POS_SHEILD,EQUIP_POS_HELMET,
										EQUIP_POS_SHOULDER, EQUIP_POS_SHIRTS, EQUIP_POS_GLOVE, EQUIP_POS_PANTS,
										EQUIP_POS_BOOTS, EQUIP_POS_KICKBALL, EQUIP_POS_RING_L, EQUIP_POS_RING_R,
										EQUIP_POS_EARRING, EQUIP_POS_NECKLACE, EQUIP_POS_BELT };
typedef std::map<int, PgBase_Item> BaseItemMap;

class	lwUIListItem;
class	PgWorld;
class	PgQuestInfo;
class	lwStyleString;

class lwUIWnd
{
	friend class lwUIAniBar;
protected:
	typedef enum EICON_ACTION_RESULT
	{
		EIAR_OK,
		EIAR_FAIL,
		EIAR_UICT_DONE,
	};

public:
	lwUIWnd(XUI::CXUI_Wnd *pWnd = NULL, bool bAssert = true);
	
	static bool RegisterWrapper(lua_State *pkState);

	// wrappers
	bool Close();
	bool CloseParent();
	bool ClearOutside();
	bool ClickInv();
	bool SetLastTargetInfo(lwUIWnd UIParent);
	bool SetPilotBar(int iType);
	
	void IconToCursor();
	bool CursorToIcon( bool bIsRBtnClicked = false );
	void DrawIconDetail(int const IconGrp, int const IconKey, const POINT2 &ptPos, const SIconInfo& rkIconInfo, bool const bDrawCount);
	void DrawIconDetail_sub( SSimpleItemInfo const &kItemInfo, POINT2 const &ptPos );
	void DrawIconDetail_sub( int const iIconGrp, PgBase_Item const* pkItem, const POINT2 &ptPos );
	
	void CallIconToolTip();
	void CallImageIconToolTip(int const iType);
	void CompEquipToolTip(PgPlayer* pkPlayer, DWORD const& iItemNo);
	void DrawIcon();
	void DrawImageIcon(int const iType);
	void DrawCursorIcon();
	void DrawCoolTimeIcon();
	void DrawMoney( EMonetaryUnit const kMU );
	void DrawEmoticonMoney();
	void DrawMoneyByValue( EMonetaryUnit const kMU, __int64 iMoney );

	void SetCursorState(int iState);
	const XUI::E_CURSOR_STATE GetCursorState() const;
	void CursorToThis();

	bool	IsStaticIcon();
	void	SetStaticIcon(bool bStatic);
	
	bool	IsIcon();

	void	RemoveAllControls();

	int	GetContCount();
	lwUIWnd	GetContAt(int iIndex);

	void SetPilotLv();
	
	EICON_ACTION_RESULT IconDoAction();

	void CursorToIconFast(int bIsDblClicked);
	void SetLicence();
	void lwSetBuildIconKey();
	void lwSetBuildIChannelBtn();
	void DeleteIcon();

	void OnOk();
	void OnCancel();
	void MsgBoxOwnerGuid(lwGUID const kGuid);
	void MsgBoxType(int const iType);

	void	SetCanDrag(bool bCanDrag);
	bool	GetCanDrag();

	void SetInvalidate(bool bValue = true);
	void Visible(bool bIsVisible);
	bool	IsVisible();

	bool SetEditFocus(bool bFocus);

	bool SetEditLimitLength(int const iLimitLength);
	int GetEditLimitLength();
	bool SetEditText(char const *pszValue);
	bool SetEditTextW(lwWString lwkEditText);
	bool SetStaticText(char const *pszValue);
	bool SetStaticTextW(lwWString wText);
	lwWString GetStaticText();
	lwWString GetEditText();
	lwStyleString GetEditText_TextBlockApplied();
	void SetTextPos(lwPoint2 kPoint);
	lwPoint2 GetTextPos();
	lwPoint2 GetTextSize();

	void	SetFontColor(unsigned int iColor);
	void	SetFontColorRGBA(int R,int G,int B,int A);

	void	SetFontFlag(int iFlag);
	int		GetFontFlag();

	template<typename T>
	void	SetCustomData(T const kValue);
	template<typename T>
	T		GetCustomData()const;
	void	SetCustomDataAsGuid(lwGUID kGuid);
	lwGUID	GetCustomDataAsGuid();
	void	    SetCustomDataAsSItem(PgBase_Item const & kItem);
	PgBase_Item	GetCustomDataAsSItem();
	void	  SetCustomDataAsStr(char const* szStr);
	lwWString GetCustomDataAsStr();
	void	 SetCustomDataAsPacket(lwPacket kPacket);
	lwPacket GetCustomDataAsPacket();
	void ClearCustomData();
	size_t GetCustomDataSize()const;

	void CopyCustomData( lwUIWnd kWnd )const;

	lwWString GetUniqueUIType() const;

	lwWString	GetID();
	void	SetID(lwWString kNewID);

	void RemoveInfoUI();
	void AdjustToOwnerPos();//Form 전용
	lwGUID GetOwnerGuid();
	void SetOwnerGuid(lwGUID lwkGuid);
	void SetOwnerState(int const iState);
	int GetOwnerState();

	lwUIWnd GetParent();
	lwUIWnd GetControl(char const* szControlId);
	lwUIWnd GetTopParent();
	lwUIWnd GetParentRecursive(lwUIWnd kWnd);

	void UpWheal();
	void DownWheal();

	void	SetParent(lwUIWnd kParentWnd);

	int IconGrp();
	int IconKey();
	void	SetUVIndex(int iIndex);
	int	GetUVIndex();
	void SetMaxUVIndex(int const iU, int const iV);
	int GetMaxUVIndex();

	void	SetIconKeyAndResNo(int iIconKey, int iIconResNo);
	void	SetIconKey(int iValue);
	void	SetIconResNo(int iValue);
	void	SetAlpha(float fValue);
	float	GetAlpha();
	void	SetMaxAlpha(float fValue);
	float	GetMaxAlpha();

	void	BarNow(int iValue);
	void	BarMax(int iValue);
	int	GetBarNow();
	int	GetBarMax();
	int GetBarCurrent();
	void SetBarReverse( bool const bReverse );
	int	GetWidth();
	int	GetHeight();

	bool IsNil();
//	void UVIndex(int const iValue);

	bool IsFocus();//Focus?
	bool IsEditFocus();

	void TempTreeTest();

	XUI::CXUI_Wnd* GetSelf();
	void RefreshMessageLog();
	void CheckState(bool bIsCheck);
	bool GetCheckState();
	void LockClick(bool bLock);
	void Disable(bool bDisable);
	bool IsDisable();
	void ExploreTypingHistory(bool const bIsPrev);
	void ResetTypingHistory();
	void SetButtonTextColor(int iState, float fColor);

	bool InitMiniMap(char const* szMiniMapName, int const Type);
	void DrawMiniMap(char const* szMiniMapName);
	void DrawMiniMapUI(char const* szMiniMapName, lwUIWnd kDummyWnd);
	void SetMiniMapTriggerIconUI(char const* szMiniMapName, lwUIWnd kDummyWnd);
	void ShowMiniMap(char const* szMiniMapName, bool bShow);
	void CloseMiniMap(char const* szMiniMapName);
	float GetZoomMiniMap(char const* szMiniMapName);
	void ZoomMiniMap(char const* szMiniMapName, float fZoomFactor);
	void ZoomMiniMapFixedFactor(char const* szMiniMapName, float fZoomFactor);
	void MouseOverMiniMap(char const* szMiniMapName, lwPoint2 &pt, char const* wndName = NULL);
	void MouseClickMiniMap(char const* szMiniMapName, lwPoint2 &pt);	
	bool GetMiniMapOptionShowNPC(char const* szMiniMapName);
	bool GetMiniMapOptionShowPartyMemeber(char const* szMiniMapName);
	bool GetMiniMapOptionShowGuildMemeber(char const* szMiniMapName);
	void SetMiniMapOptionShowNPC(char const* szMiniMapName, bool bShow);
	void SetMiniMapOptionShowPartyMember(char const* szMiniMapName, bool bShow);
	void SetMiniMapOptionShowGuildMemeber(char const* szMiniMapName, bool bShow);	
	void SetWndSize(char const* szMiniMapName, lwPoint2 pt);
	void CloneMiniMap(char const* szSrcMiniMap, char const* szDestMiniMap);

	void InitWorldMap();
	void TermWorldMap();
	void DrawWorldMap();
	void PulseWorldMap();	
	void InSideWorldMap();
	void OutSideWorldMap();
	void MouseOverWorldMap(lwPoint2 &pt);
	void MouseClickWorldMap(lwPoint2 &pt);

	void InitWorldMapPopUp(lwUIWnd kIconState, lwUIWnd kFormState);
	void TermWorldMapPopUp();

	void DrawPetModel();
	void TurnPetModel(float fDegree);

	void InitRenderModel(char const* szRenderModelName, bool bUpdate = true, bool bOrtho = false);
	void DrawRenderModel(char const* szRenderModelName);
	void ClearRenderModel(char const* szRenderModelName);
	void DelRenderModel(char const* szRenderModelName);
	void AddToDrawListRenderModel(char const* szRenderModelName);
	void RenderModelCameraZoom(char const* szRenderModelName, float fCameraZoomDistance);
	void RenderModelCameraZoomSubstitution(char const* szRenderModelName, float fCameraZoomDistance);
	void RenderModelOrthoCameraZoom(char const* szRenderModelName, float fZoomDistance);
	void RenderModelOrthoCameraZoomSubstitution(char const* szRenderModelName, float fZoomDistance);
	void RenderModelOrthoCameraZoomSubstitutionOldVer(char const* szRenderModelName, float fZoomDistance);
	void SetRenderModelCameraZoomMinMax(char const* szRenderModelName, float fMin, float fMax);
	void RotateRenderModel(char const* szRenderModelName, float fXAxis, float fYAxis, float fZAxis);
	void SetAutoRotateRenderModel(char const* szRenderModelName, bool bAutoRotate, float fXAxis = 0.0f, float fYAxis = 0.0f, float fZAxis = 0.0f);
	void UpdateOrthoZoom(char const* szRenderModelName, float const fScale, int const iX, int const iY);
	void UpdateOrthoZoomOldVer(char const* szRenderModelName, float const fScale, int const iX, int const iY);

	void AddRenderModelActorByGuid(char const* szRenderModelName, char const* szActorName, char const* szGuid, bool bClone = true, bool bCameraReset = true);
	void AddRenderModelActorByID(char const* szRenderModelName, char const* szActorName, char const* szID, bool bClone = true, bool bCameraReset = true);
	void AddRenderModelActorByPath(char const* szRenderModelName, char const* szActorName, char const* szPath, bool bClone = true, bool bCameraReset = true);
	void SetRenderModelActorTranslate(char const* szRenderModelName, char const* szActorName, float fx, float fy, float fz);
	void SetRenderModelActorRotate(char const* szRenderModelName, char const* szActorName, float fXAxis = 0.0f, float fYAxis = 0.0f, float fZAxis = 0.0f);
	void AddRenderModelActor(char const* szRenderModelName, char const* szActorName, lwActor kActor, bool bClone, bool bCameraReset);

	void AddRenderModelNIFByID(char const* szRenderModelName, char const* szActorName, char const* szID, bool bClone = true, bool bCameraReset = true);
	void AddRenderModelNIFByPath(char const* szRenderModelName, char const* szActorName, char const* szPath, bool bClone = true, bool bCameraReset = true);
	void SetRenderModelNIFTranslate(char const* szRenderModelName, char const* szActorName, float fx, float fy, float fz);
	void SetRenderModelNIFRotate(char const* szRenderModelName, char const* szActorName, float fXAxis = 0.0f, float fYAxis = 0.0f, float fZAxis = 0.0f);

	void ResetRenderModelNIFAnimation(char const* szRenderModelName, char const* szNIFName);

	void SetRenderModelEnableUpdate(char const* szRenderModelName, bool bUpdate);

	void SetRenderModelActorTargetAnimation(char const* szRenderModelName, char const* szActorName, int iID);
	bool IsRenderModelActorAnimationDone(char const* szRenderModelName, char const* szActorName);
	bool SetCameraByName(char const* szRenderModelName, char const* szNIFName, char const *pcCameraName);
	bool SetRecursiveCameraByName(char const* szRenderModelName, char const* szNIFName, char const *pcCameraName);
	void SetRenderModelActorEnableUpdate(char const* szRenderModelName, char const* szActorName, bool bEnable);
	void SetRenderModelNIFEnableUpdate(char const* szRenderModelName, char const* szNIFName, bool bEnable);
	void ConnectedInfo(int const iMapNo, int const iTickTime);
	
	lwPoint2 GetResolutionSize( const unsigned int iIndex );	
	int	GetResolutionModeCount();
	bool IsWideResolution( const unsigned int iWidth, const unsigned int iHeight );
	lwPoint2 GetResolutionGap();

	XUI::CXUI_Wnd*operator ()()
	{
		return GetSelf();
	}

	void Effect_Open()
	{
		self->SetState( XUI::ST_OPENING );
	}

	void Effect_Stop()
	{
		self->SetState( XUI::ST_STOP );
	}

	void Effect_Clear()
	{
		self->SetState( XUI::ST_NONE );
	}

	//	For List Control
	lwUIListItem	AddNewListItem(lwWString kText);
	lwUIListItem	AddNewListItemChar(char* szControlId);
	void	ClearAllListItem();
	int	GetListItemCount();
	lwUIListItem	ListFindItem(char *szItemName);
	lwUIListItem	ListFindItemW(lwWString kItemName);
	void	ListDeleteItem(lwUIListItem item);
	lwUIListItem	ListFirstItem();
	lwUIListItem	ListNextItem(lwUIListItem kItem);
	void ElementID(char const* szName);
	void AdjustMiddleBtnPos();
	void MoveMiddleBtnEndPos();
	void SetDisplayStartPos(int const iValue);

	//	For Tree Control
	lwUIListItem AddNewTreeItem(lwWString kText);
	lwUIListItem AddNewTreeItemChar(char* szControlId);
	lwUIListItem TreeFirstItem();
	lwUIListItem TreeNextItem(lwUIListItem item);
	void	TreeDeleteItem(lwUIListItem item);

	void	ClearAllTreeItem();
	int	GetTreeItemCount();
	bool	IsExpandTree();
	void	ExpandTree();
	void	CollapseTree();

	void RefreshCalledTime();
	void RefreshLastTickTime();
	void SetAliveTime(int Value);
	int  GetAliveTime();

	lwPoint2 GetLocation();
	lwPoint3 GetLocation3();
	void SetLocation(lwPoint2 pt);
	void SetLocation3(lwPoint3 pt);
	void SetLocationToMyActor(lwPoint2 pt,char const* strTargetDummy);
	lwPoint2 GetTotalLocation();
	void SetPriority(int const iPriority);
	bool GetPriority()const;

	int GetAlignX();
	int GetAlignY();
	void SetAlignX(int const AlignX);
	void SetAlignY(int const AlignY);
	void VAlign();

	lwPoint2 GetSize();
	void SetSize(lwPoint2 pt);

	lwPoint2 GetImgSize();
	void SetImgSize(lwPoint2 pt);

	//	For Ani Bar
	void SetStartTime(int nMax);
	void SetCloseTime( int const nAfterStopTime );
	void DisplayTimeBar( bool bText );
	void DisplayTimeBarMsg( char const* _Format, bool bReal );
	void DisplayPerBarMsg(char const* _Format, int iIncSpeedByMilSec, bool isFloat);
	
	void ChangeImage( char *pFileName, bool const bAniBar );
	void VOnCall();
	bool IsMouseDown();
	bool SetColor(unsigned int iColor);
	void SetGrayScale(bool bGrayScale);
	bool IsGrayScale();
	float GetSizeScale();
	void SetSizeScale(float const fScale, bool const bIsCenterTie);
	
	float GetScale();
	void SetScale(float fScale);
	lwPoint2F GetScaleCenter();
	void SetScaleCenter(lwPoint2F kCenter);

	float GetRotationDeg();
	void SetRotationDeg(float fDeg);
	lwPoint2F GetRotationCenter();
	void SetRotationCenter(lwPoint2F kCenter);

	void ButtonChangeImage(char *szFileName, int iWidth=0, int iHeight=0);

	bool SetPilotBarInfo(int iType);
	int TranslateExp(__int64 iExp);

	// For Static Form
	void SetStaticNum(int iNum);
	int GetStaticNum();

	//Scroll
	void SetScrollCur(int iCur);
	int GetScrollCur();
	int GetScrollMax();

	void SetClose(bool bValue);
	bool GetClose();

	int GetBuildIndex();
	void SetBuildIndex(int iBuildIndex);

	void SetEnable(bool bValue);
	bool GetEnable();

	bool IsTwinkle();
	void OffTwinkle();
	void OffTwinkleSelf();
	bool NowTwinkleOn();
	void SetTwinkle(unsigned int uiTwinkleTime, unsigned int uiInterTime);
	void ClearTwinkleInfo();


	DWORD GetLiveTime(bool bSec);

	void RegistScript(char* szID, char* szValue);
	bool PtInUI(lwPoint2 pt);
	lwUIWnd FindTopParent();
	void QuickSlotIcon_OnDisplay();
	void SetIconGroup(int iGroup);

	void ReLocationSnapChild();
	
	void CallCashItemToolTip(bool const bGift = false);

	void ClearRouletteInfo();
	void SetRouletteStop();
	void StopMissionResult();

	bool IsFinishedTextDraw();
	void SkipTDrawInterval();

	void SetModal(bool bModal);
	lwWString GetDefaultImgName();

	//List2용
	bool SetSelect( lwGUID kGuid );
	lwGUID GetSelectGuid()const;
	void SetDisplayStartItem( size_t const iIndex );

	lwPoint2 const GetBuildCount();	//CXUI_Builder의 카운트를 반환
	int const GetBuildNumberCount();
	lwPoint2 const GetBuildGab();

	//ON_TICK
	DWORD GetTickInterval();
	void SetTickInterval(int const iTickInterval);

	// 길드금고 전용 ----------------------
	static void ConvertGuildInvTypeToIconGroup( const EInvType eInvType, EKindUIIconGroup& rkOutIconGroup ); // 인벤타입 --> KUIG
	static void ConvertIconGroupToGuildInvType( const EKindUIIconGroup eIconGroup, EInvType& rkOutInvType ); // KUIG --> 인벤타입
	static void ConvertItemPosToGuildInvType( SItemPos& rkOutItemPos ); // SItemPos 변환

	//MoveWindow전용-----------------------
	void InitMoveState();
	float GetMovePercent();
	void SetModifyPoint1(lwPoint3 Pt);
	void SetModifyPoint2(lwPoint3 Pt);
	void SetStartPoint(lwPoint3 Pt);
	void SetEndPoint(lwPoint3 Pt);
	void SetMoveTime(float const fTime);
	int GetMoveTime()const;
	lwPoint3 GetModifyPoint1();
	lwPoint3 GetModifyPoint2();
	lwPoint3 GetStartPoint();
	lwPoint3 GetEndPoint();
	bool IsMoveComplate();
	bool IsMovePause();
	void lwSwapMovePoint();
	void lwSetPauseTime(int const fValue);
	float lwGetPauseTime()const;
	void lwSetupMove(lwPoint2 const kStart, lwPoint2 const kEnd, float const fTotalMoveTime);

	bool IsClickLock(void);
	void SetClickLock( bool const bLock );

	void SetState( int const iState );
	int GetState();

	void SetLocationByWorldNode(char* const pkNodeName, lwPoint2 kMovePt);
	//-------------------------------------
	void DrawIconDetail_AddRes(int const iResNo, POINT2 const& ptPos, unsigned short sW = 40, unsigned short sH = 40 );
	//기본 아이콘이 참조하는 부가 아이콘을 화면에 그린다. (AddResNo1, AddResNo2, AddResNo3)

protected:
	void GetSkillCoolTimeInfo(lwActor klwActor, XUI::CXUI_Icon* pParent, XUI::CXUI_Wnd* pToggleImg, float& fRemainTime, float& fTotalTime);
	void GetItemSkillCoolTimeInfo(lwActor klwActor, int const iSkillNo, XUI::CXUI_Icon* pParent, float& fRemainTime, float& fTotalTime);
	bool ConvNiKey(int& iNiKey);	

	bool AutoMoveItemToInven(PgPlayer* pkPlayer, SItemPos const& rkItemPos, EInvType const& eInvType);
protected:
	XUI::CXUI_Wnd *self;
};


class	lwUIListItem
{
public:

	//! 생성자
	lwUIListItem(XUI::SListItem *pItem);

	//! Wrapper를 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	bool	IsNil();

	lwUIWnd	GetWnd();

	XUI::SListItem	* operator()();
private:

	XUI::SListItem	*m_pListItem;

};

extern void ExitApp();

extern	lwUIWnd	lwCreateControl(char const*pszControlID);

extern void lwSendScriptEvent(char const *pszParent, char const *pszControl, char const *pszScriptKey );
extern bool lwSetEditFocus(char const *pszParent, char const *pszControl);
extern bool lwTryLogin(lwWString lwID, lwWString lwPW, bool bSave = false, bool bCheck = false);
extern bool lwClearAutoLogin();
extern lwWString lwGetAccountID();
extern void lwSetDisConnectFlush();

extern bool lwClearEditFocus();
extern void lwChangeMapInfo(int mapNo);
extern void lwRecvPickupItem(lwPacket kPacket);
extern void lwSendPickupItem(int const uiKey);
extern void lwSetLastAttackTarget(lwGUID guid);

extern lwUIWnd lwActivateUI( char const *pszText, bool const bModal = false, char const* pszNewID = NULL );
extern lwUIWnd lwActivateUIPriority( char const *pszText, int const iPriority);
extern lwUIWnd _lwActivateUI(std::wstring const  &kID, bool const bModal = false, std::wstring const& pszNewID = _T("") );
extern bool lwCheckCalledUniqueUIType(char const* pszTypeID);
extern bool lwIsExistUniqueType(char const* pszTypeID);

extern void lwCallToolTipByText( int iIconNo, lwWString strText, lwPoint2 &pt, char const* wndName = NULL, int const iCategory = 0, char const* szEx = NULL);
extern void lwCallMutableToolTipByText(lwWString strText, lwPoint2& pt, int iIconNo = 0, char const* wndName = NULL, bool bDrawTextCenter = true);
extern void lwCallToolTipByStatus( WORD wAbilType, lwPoint2 &pt, char const* wndName = NULL, int const iCategory = 0, char const* szEx = NULL);

extern void MakeToopTipIcon_Common(XUI::CXUI_Wnd* pMainWnd, int iIconNo);
extern bool MakeToolTipText_Status( WORD wAbilType, std::wstring &wstrTextOut, CUnit* pkUnit = NULL, int* pkiOutResult  = NULL );
extern void MakeToolTipText_MonsterCard(PgBase_Item const &kOrgItem, lwPoint2 &pt, SToolTipFlag const &kFlag=SToolTipFlag());
extern void MakeToolTipText_SkillExtend(PgBase_Item const &kOrgItem, lwPoint2 &pt, SToolTipFlag const &kFlag=SToolTipFlag());
extern void MakeToolTipText_SuperGirl(PgBase_Item const &kOrgItem, lwPoint2 &pt, TBL_SHOP_IN_GAME const &kShopItemInfo, SToolTipFlag const &kFlag=SToolTipFlag());
extern bool MakeToolTipText(PgBase_Item const &kOrgItem, std::wstring &wstrTextOut, TBL_SHOP_IN_GAME const &kShopItemInfo, std::wstring& wstrLank, bool bHideRarity = false, bool bOrtherActor = false, SToolTipFlag const &kFlag=SToolTipFlag());
extern void CallToolTip_ItemNo(int const iItemNo, lwPoint2 &rPT, bool const bForceSeal = false, bool bHideRarity = false, bool bOrtherActor = false, int const iTimeType = 0, int const iTimeValue = 0);
extern void CallToolTip_SItem(PgBase_Item const* pItem, lwPoint2 &pt, TBL_SHOP_IN_GAME const &kShopItemInfo = TBL_SHOP_IN_GAME::NullData(), char const* pkWndName = NULL, const wchar_t* pkHeadStr = NULL, bool bOrtherActor = false, SToolTipFlag const &kFlag=SToolTipFlag(), bool const bShowExtend = true);
extern void ToolTipComp_SItem(PgBase_Item const &kItem, lwPoint2 kPoint, SToolTipFlag const &kFlag=SToolTipFlag());
extern bool MakeSkillToolTipText( PgSkillTree::stTreeNode *pTreeNode, std::wstring &wstrTextOut,bool bNextLevel, std::wstring& wstrType);
extern bool MakeSkillToolTipText(int iSkillNo, std::wstring &wstrTextOut,/*bool bNextLevel,*/ std::wstring& wstrType);
extern bool MakeComboSkillToolTip(int iSkillNo, std::wstring &wstrTextOut, std::wstring& wstrType);
extern bool MakeSkillLimitClassText(const	CSkillDef *pkSkillDef,std::wstring &kOut);
extern void CallSkillToolTip(PgSkillTree::stTreeNode *pTreeNode, lwPoint2 &pt);
extern void CallSkillToolTip(int iSkillNo, lwPoint2 &pt);
extern void CallComboSkillToolTip(int iSkillNo, lwPoint2 &pt);
extern void MakeComboDamageText( int iSkillNo, std::wstring &wstrTextOut );
extern void CallSkillAbilToolTip(int iSkillNo, lwPoint2 &pt); //Debug용 EXTERNAL_RELEASE에선 사용되지 않음
extern bool MakeMonsterToolTipText(int iIconKey, std::wstring &wstrTextOut, std::wstring & wstrLank);
extern void CallMonsterToolTip(int iIconKey, lwPoint2 &pt);		// iIconKey 는 몬스터 NO
extern void CallGuardianToolTip(int iIconKey, lwPoint2 &pt);		// iIconKey 는 가디언 NO
extern void CallStratagySkillToolTip(int const iGuardianNo, int const iSkillNo, lwPoint2 &pt);

extern void MakeToolTipTextExtraction(CItemDef const *pDef,std::wstring &wstrText,PgBase_Item const &kItem);
extern bool MakeToolTipTextConstellation(CONT_DEFGEMSTORE const * pDef, DWORD const & MaterialItemNo, int const Menu);
//직업스킬
extern void CallJobSkillToolTip(int iSkillNo, lwPoint2 &pt);
extern void MakeToolTipText_JobSkill_Tool(PgBase_Item const &kItem, std::wstring &wstrTextOut, std::wstring &wstrType, TBL_SHOP_IN_GAME const &kShopItemInfo = TBL_SHOP_IN_GAME::NullData());
extern void MakeToolTipText_JobSkill_SaveIdx(PgBase_Item const &kItem, std::wstring &wstrTextOut, std::wstring &wstrType, TBL_SHOP_IN_GAME const &kShopItemInfo = TBL_SHOP_IN_GAME::NullData(), SToolTipFlag const &kFlag=SToolTipFlag());
extern void MakeToolTipText_JobSkill_Item(PgBase_Item const &kItem, std::wstring &wstrTextOut, std::wstring &wstrType, TBL_SHOP_IN_GAME const &kShopItemInfo = TBL_SHOP_IN_GAME::NullData(), SToolTipFlag const &kFlag=SToolTipFlag());
//영력보관구슬
extern void MakeToolTipText_SoulTransition(PgBase_Item const &kItem, std::wstring &wstrTextOut, std::wstring &wstrType, TBL_SHOP_IN_GAME const &kShopItemInfo = TBL_SHOP_IN_GAME::NullData(), SToolTipFlag const &kFlag=SToolTipFlag());
extern void MakeAbilStringOnlyType(PgBase_Item const &kItem, CItemDef const* pkItemDef, WORD const wType, int iValue, std::wstring &wstrText);

extern bool MakeToolTip_ItemExtend(PgBase_Item const &kOrgItem, std::wstring &wstrTextOut, TBL_SHOP_IN_GAME const &kShopItemInfo, std::wstring& wstrLank, bool bHideRarity = false, bool bOrtherActor = false, SToolTipFlag const &kFlag=SToolTipFlag());

//펫용
extern void MakeToolTipText_Pet(PgBase_Item const &kOrgItem, std::wstring &wstrTextOut, std::wstring& wstrLank);
extern void CallSkillToolTip_Pet(int const iSkillNo, lwPoint2 &pt);
extern bool MakeSkillToolTipText_Pet(int const iSkillNo, std::wstring &wstrTextOut, std::wstring &wstrType);

extern void MakeItemColorForToolTip(PgBase_Item const &kOrgItem, std::wstring& wstrText, std::wstring& wstrRank);	//툴팁용 이름색상 결정

//프리미엄서비스
extern void lwCallPremiumToolTip(lwPoint2 &pt);

//커뮤니티 이벤트용
extern void MakeClassLimitText(__int64 i64HumanClassLimit, __int64 i64DragonClassLimit, std::wstring &wstrText);
extern void lwCallEventScheduleToolTip(int const EventNo, lwPoint2 &pt);
extern void lwCallChannelPartyListToolTip(lwPacket packet, lwPoint2 &pt);

//즉석 가공
extern void AddToolTip_Manufacture(CItemDef const *pDef, std::wstring &wstrTextOut);

//인챈트 점프 아이템 (인챈트 레벨 제한. 레벨 제한 없을 경우 기존 툴팁 생성)
extern void AddToolTip_InchantForceUpgrade(PgBase_Item const &kOrgItem, std::wstring &wstrTextOut);

typedef struct tagEnchantAbil
{
	tagEnchantAbil()
		:iPos(0), iGroup(0), iLevel(0), wType(0), iValue(0), iIdx(0)
	{
	}
	
	explicit tagEnchantAbil(int const iInPos, int const iInGroup, int const iInLevel, WORD const _type, int const _value, int const _idx)
		:iPos(iInPos), iGroup(iInGroup), iLevel(iInLevel), wType(_type), iValue(_value), iIdx(_idx)
	{
	}
	
	int	iPos;
	int iGroup;
	int iLevel;
	WORD wType;
	int iValue;
	int iIdx;
}SEnchantAbil;

typedef std::vector< SEnchantAbil > CONT_ENCHANT_ABIL;
extern void GetDefaultOption(PgBase_Item const &kItem, CONT_ENCHANT_ABIL & kEnchantAbil, bool const bRareType = false, SEnchantInfo const * pkEnchantInfo = NULL);

extern void MakeSetItemOptionString(PgBase_Item const &kItem, CItemDef const* pkItemDef, int const iAbilNum, std::wstring &wstrText, bool const bColor=true);
extern void MakeMonsterCardOptionString(PgBase_Item const &kItem, CItemDef const* pkItemDef, int const iAbilNum, std::wstring &wstrText, bool const bAddTitle = true, PgBase_Item const * const pkEquipItem = NULL);
extern int GetMonsterCardItemNo(int const iOrderIndex, int const iCardNo);
extern lwUIWnd lwCallUI(char const *pszText, bool bModal = false, char const *pszNewID = NULL);
extern lwUIWnd lwCloneAutoCallUI(char const *pszText, bool bModal = false);

extern void lwCloseAllUI(bool bNotCursor = false );
extern void lwReleaseAllControl();
extern bool lwCloseUI(char *pszText );
extern void lwRecvOtherItemChange(lwPacket &Packet);

extern void lwAddChatBaloon(lwPacket Packet);
extern void lwDelChatBaloon(lwPacket Packet);
extern void lwAddChatData(lwPacket Packet);
extern void lwSetCoolTime(float value,char const *strText);
extern void lwExitClient();

extern void lwAddWarnDataStr(lwWString Str, int const iLevel, bool const bCheckEqualMessage = false);	//공지 및 정보사항창
extern void lwAddWarnData(int const iDefStringNo);							//공지 및 정보사항창
extern void lwAddWarnDataTT(int const iTextTableNo, bool const bCheckEqualMessage = false);		//공지 및 정보사항창

extern void lwQuickInvDoAction(int const iSlotNum);

extern void lwSendSelectInfo(char *pszText);
extern void lwSelectItem( char *pszText);

extern void lwGetQuestInfoFromList(XUI::CXUI_Wnd* pListWnd = NULL);
extern void lwSelectQuest(XUI::CXUI_Wnd* pItem);

extern void lwGetPetInfo();
//void lwTempOhterSet(lwActor &Actor);
//void lwNpcSet(lwActor &Actor, char const* szUIID);

extern void Send_PT_C_M_REQ_ITEM_CHANGE(const SItemPos &kCasterPos, const SItemPos &kTargetPos);
extern void Send_PT_C_M_REQ_ITEM_CHANGE_GUILD(const SItemPos &kCasterPos, const SItemPos &kTargetPos, const bool bCasterIsGuildInv, const bool bTargetIsGuildInv);
extern void Send_PT_C_M_REQ_ITEM_CHANGE_YES_NO(int const TTWID, const SItemPos &kCasterPos, const SItemPos &kTargetPos);
extern void RegistUIAction(XUI::CXUI_Wnd* pkTopWnd, std::wstring const& rkScript = WSTR_CLOSEUI);

namespace PgGuildInventory
{
	extern BM::GUID kGuildInventoryNpc;
	void SetGuildInventoryNpc(lwGUID kNpcGuid);
	void ClearGuildInventoryNpc();
}

namespace PgSafeInventory
{
	extern BM::GUID kSafeInventoryNpc;
	void CallInvSafeUI(lwGUID kNpcGuid);
	void CloseInvSafeUI();
	void SendCloseSafe();
	void CallInvShareSafeUI(lwGUID kNpcGuid);
	void CloseInvShareSafeUI();
	void SendCloseShareSafe();
}

typedef enum eMsgBoxType
{
	MBT_NONE						= 0,
	MBT_PARTY_JOIN					= 1,
	MBT_PARTY_EXIT					= 2,
	MBT_GUILD_JOIN					= 3,
	MBT_CONFIRM_EXIT				= 4,
	MBT_CONFIRM_BUY_SKILL			= 5,
	MBT_CONFIRM_COUPLE				= 6,
	MBT_CONFIRM_BREAK_COUPLE		= 7,
	MBT_CONFIRM_INSTANCE_COUPLE		= 8,
	MBT_CONFIRM_WARP_COUPLE			= 9,
	MBT_PARTYFIND_JOIN				= 10,
	MBT_CONFIRM_PAYMENT_MAIL		= 11,
	MBT_CONFIRM_GET_ANNEX_ALL		= 12,
	MBT_CONFIRM_RETURN_MAIL_ALL		= 13,
	MBT_CONFIRM_RETURN_MAIL			= 14,
	MBT_CONFIRM_DELETE_MAIL			= 15,
	MBT_MARKET_REQUEST_BUY			= 16,
	MBT_MARKET_REQUEST_DELETE		= 17,
	MBT_MARKET_REQUEST_REGIST		= 18,
	MBT_MARKET_REQUEST_CANCEL		= 19,
	MBT_CONFIRM_GUILD_OWNER_CHANGE	= 20,
	MBT_CONFIRM_DROP_SYSTEM_INVEN	= 21,
	MBT_FRAN_CHANGE					= 22,
	MBT_FRIEND_ADD					= 23,
	MBT_CONFIRM_GUILD_KICK_USER		= 24,
	MBT_CONFIRM_GUILD_LEAVE			= 25,
	MBT_CONFIRM_ENTERTUTORIAL		= 26,
	MBT_COMMON_YESNO_TO_PACKET		= 27,
	MBT_COMMON_YESNO_PARTYMEMBER	= 28,
	MBT_ITEM_ACTION_YESNO_TO_PACKET	= 29,
	MBT_CHANGE_GUILD_MARK1			= 30,
	MBT_USER_PORTAL					= 31,
	MBT_CONFIRM_SEAL				= 32,
	MBT_CONFIRM_MONSTERCARD			= 33,
	MBT_PLUS_INSURE_OKCANCEL		= 34,
	MBT_RARITY_INSURE_OKCANCEL		= 35,
	MBT_MODIFY_TO_NOMALMARKET		= 36,
	MBT_CONFIRM_SWEETHEART_QUEST	= 37,
	MBT_CONFIRM_SWEETHEART_COMPLETE	= 38,
	MBT_CONFIRM_ACCEPT_SHAREQUEST	= 39,
	MBT_ACCEPT_HOMEAUCTION_CANCEL	= 40,
	MBT_CONFIRM_LEAVE_BATTLESQUARE	= 41,
	MBT_CONFIRM_EXITGAME_AND_WEBPAGE = 42,
	MBT_COMMON_YESNO_MYHOM_REPAIR	= 43,
	MBT_COMMON_YESNO_MYHOM_ENCHANT	= 44,
	MBT_CONFIRM_CASHSHOP_BASKET_REG	= 45,
	MBT_COMMON_YESNO_MYHOM_SOUL_GOLD	= 46,
	MBT_COMMON_YESNO_MYHOM_SOUL_COUNT	= 47,
	MBT_COMMON_YESNO_MYHOM_SOCKET_GOLD	= 48,
	MBT_COMMON_YESNO_MYHOM_SOCKET_RATE	= 49,
	MBT_COMMON_YESNO_MYHOM_ENCHANT_RATE	= 50,
	MBT_STATIC_CASHITEM_BUY_TO_PACKET = 51,
	MBT_PARTY_LEAVE					= 52,
	MBT_PARTY_KICKOUT				= 53,
	MBT__COMMON_YESNO_RARITY_BUILDUP	= 54,
	MBT__COMMON_YESNO_RARITY_AMPLIFY	= 55,
	MBT_SKILLSET_SAVE				= 56,
	MBT_EXIT_SUPER_GROUND			= 57,
	MBT_MYHOME_CHAT_CLEAN			= 58,
	MBT_MYHOME_CHAT_EXIT			= 59,
	MBT_STRATEGYSKILL_CHANGE		= 60,
	MBT_CONFIRM_GUILD_APPLICATION_CANCEL = 61,
	MBT_CONFIRM_GUILD_ENTRANCE_FEE = 62,
	MBT_DEFENCE_GUARDIAN_INSTALL	= 63,
	MBT_DEFENCE_GUARDIAN_UPGRADE	= 64,
	MBT_CONFIRM_ENTER_VENDOR= 65,
	MBT_CONFIRM_LEAVE_VENDOR= 66,
    MBT_EMPORIABATTLETHROW          = 67,
	MBT_DEFENCE_GUARDIAN_REMOVE		= 68,
	MBT_BUY_JOBSKILL				= 69,
	MBT_INTERACTIVE_EMOTION_REQUEST_ACCEPT	= 70,
	MBT_DEL_JOBSKILL				= 71,
	MBT_NONE_COSTUME_ROULLET		= 72,
	MBT_NONE_CASH_GACHA_ROULLET		= 73,
	MBT_CONFIRM_SOUL_TRANSITION_HAVE_RARITY 		= 74,
	MBT_ENCHANT_SHIFT_START			= 75,
	MBT_CONFIRM_SOUL_TRANSITION_NOT_INSURANCE		= 76,
	MBT_ENCHANT_SHIFT_DESTORY_INSURANCE				= 77,
	MBT_EXTRACT_ELEMENT				= 78,
	MBT_COMMON_YESNO_PARTYMASTERGROUND				= 79,
	MBT_EXPEDITION_DESTROY			= 80,
	MBT_EXPEDITION_LEAVE			= 81,
	MBT_EXPEDITION_JOIN				= 82,
	MBT_EXPEDITION_INVITE			= 83,
	MBT_EXPEDITION_COMPLETE_EXIT	= 84,
	MBT_CONFIRM_LEAVE_EXPEDITION_LOBBY = 85,
	MBT_EXPEDITION_CONFIRM_ENTER	= 86,
	MBT_EXPEDITION_KICK_MEMBER		= 87,
	MBT_EXPEDITION_PVPMODE_ENTER	= 88,
	MBT_CHANGE_ANTIQUE				= 89,
	MBT_EXCHANGE_ACCUMPOINT_TO_STRATEGICPOINT = 90,
	MBT_EXCHANGE_GEMSTORE			= 91,
	MBT_KILL_MONSTER_BY_POINT_COPY	= 92,
	MBT_CREATE_JUMPING_CHAR_EVENT	= 93,
	MBT_WM_CLOSE					= 94,
	MBT_EXIT_INSTANCE_DUNGEON		= 95,
	MBT_CONFIRM_CONSUME_ITEM_PARTY_BREAK_IN = 98,
}EMsgBoxType;

extern bool DisconnectFromServer(bool bDisConnectByUser);

extern void lwCallMessageBox(std::wstring const &wstrMessage, std::wstring const &wstrTitle, DWORD const dwMessageBoxFlag);
extern void lwCallInputBox(std::wstring const &rkMessage, int const iCommandType, bool const bModal, int iLimitLength);
extern void lwCallYesNoMsgBox(lwWString kMessage, lwGUID kOrderGuid, int const iBoxType, int const iExternInt = 0, bool bUICloseRegistAction = false);
extern void CallYesNoMsgBox(std::wstring const &wstrMessage, BM::GUID const &rkOrderGuid, const EMsgBoxType iBoxType, int const iExternInt = 0, bool bUICloseRegistAction = false);
extern void CallYesNoMsgBoxLimit(std::wstring const &wstrMessage, BM::GUID const &rkOrderGuid, const EMsgBoxType iBoxType, std::wstring const &kName = std::wstring());

extern void lwGetPetInfo();

extern void lwCallPetUI();
//extern void lwCallCharUI();

extern void lwDisplayPetText( lwWString pcText, int iTime );	//문자열, 천분초
extern void lwIdlePetText();	//시간체크

extern void lwTempQuestTest();
	
extern void lwCheckForMiniQuestMenu( XUI::CXUI_Wnd* pWnd );
extern void lwGiveUpQuest();

extern	void	lwEnableUIOffscreenRendering(bool bEnable);

extern void lwBossHPBar();

extern void lwExitShop();






extern void lwSetBossGUIDForHPBar(lwGUID bossguid);
extern void lwDisplayBossHPBar();

extern lwPoint2 lwGetScreenSize();
extern lwPoint2 lwGetCursorPos();
extern void lwCloseToolTip(char const* wndName = NULL, int const iTargetCategoly = 0);
extern void lwUIOff(bool const bAdd_LastContainer=false);
extern void lwUIOn();
extern void lwBlockGlobalHotKey( bool bBlocking );
extern bool lwIsVisible(char *pszText);
extern lwUIWnd lwGetUIWnd(char const *pszText);
extern bool lwIsUIWnd(char const *pszText);
extern float lwCompNum(float fLeft, float fRight);	// 두 숫자 비교. return left - right. XML에서 >, < 기호를 사용못하기 때문에
extern void lwSetBuffWnd(lwUIWnd UIWnd);
extern void lwAddBuff(int const iEffectKey, int const iEffectID, int const iValue, bool const bNewAdd = true, DWORD const dwElapsedTime = 0, DWORD const dwStartTime = 0, DWORD const dwEndTime = 0);
extern void lwDeleteBuff(int iEffectID);
extern void lwUpdateBuffWnd();
extern int	lwGetBuffEffectNo(int const index); //godcmd에서 필요한 함수 나에게 걸려있는 버프아이콘 순서에 해당되는 EffectID를 얻어온다.
extern void lwUpdateBuffEndTime(int const iEffectID, DWORD const dwEndTime);
extern void lwGlobalLBtnUp();
extern void lwGlobalLBtnUp();
extern bool lwKeyWasReleased(int iKeyNum);
extern bool lwKeyIsDown(int iKeyNum,bool bIsNotUKey);
extern void lwReleaseAllKey();
extern bool MakeItemName(int const iItemNo, const SEnchantInfo &rkEnchant, std::wstring &wstrName);
extern lwWString lwGetDropRes(int iNum);
extern bool lwGetItemResData(int iItemNo, TBL_DEF_RES &rkDefRes);
extern bool lwRegUIColor(int iIdx, int iColor);
extern int lwGetUIColor(int iIdx);
extern void lwChangeUIColorSet();
extern POINT2 CalculateOnlySize(XUI::CXUI_Style_String const& kString);

//>>Option
extern void lwConfig_DefaultConfig();
extern void lwConfig_DefaultKeySet();
extern int lwConfig_GetDefaultValue(char const *szHead, char const *szKey);
extern char const* lwConfig_GetDefaultText(char const *szHead, char const *szKey);
extern void lwConfig_ApplyConfig();
extern void lwConfig_CancelConfig();
extern void lwConfig_ApplyKeySet();
extern void lwConfig_CancelKeySet();
extern void lwConfig_ApplyGraphics();
extern void lwConfig_Save(bool bSaveToServer);
extern int lwConfig_GetValue(char const *szHead, char const *szKey);
extern char const* lwConfig_GetText(char const *szHead, char const *szKey);
extern void lwConfig_SetValue(char const *szHead, char const *szKey, int iValue);
extern void lwConfig_SetText(char const *szHead, char const *szKey, char const *szText);
extern void lwSysConfig_SetValue(char const *szHead, char const *szKey, int iValue);
extern void lwSysConfig_SetText(char const *szHead, char const *szKey, char const *szText);
extern void lwUpdateEtcConfig();
extern bool lwUseLevelRank();
//<<Option
//>>KeySet
int lwUKeyToKey(int iUkey);//Ukey를 Key번호로 바꾼다. lua에서 KeyNo를 가지고 GetKeynoToName()를 통해 이름(설명)을 얻을 수 있다.
//<<KetSet

extern bool lwDisconnectLoginServer();

extern int const lwGetCurrentChennelName();
extern bool AddItemToInvenList(XUI::CXUI_Wnd* pWnd, PgBase_Item* kItem, SItemPos* kPos);
extern char const* lwGetDefString(int iDef);
extern std::wstring const GetMapName(int const iMapNo);
extern lwWString lwGetMapNameW(unsigned int iMapNo);
extern int lwGetMapKey(int const iMapNo);
extern int lwGetMapAttrByNo(int const iMapNo);
extern __int64 GetMoneyMU( EMonetaryUnit const kMU, __int64 const i64Money );
extern std::wstring const GetMoneyString( __int64 const iMoney, bool const bFilterZero=true, bool const bUseIcon = true );

extern void CallQuickSkillToolTip(PgSkillTree::stTreeNode* pTreeNode, lwPoint2& pt, int const iSkillNo = 0, bool const bNotShowZeroValue = false, bool const bUseOverSkillLevel = false);
extern bool MakeQuickSkillToolTipText( PgSkillTree::stTreeNode* pTreeNode, std::wstring& wstrTextOut, std::wstring& wstrType, int const iSkillNo = 0, bool const bNotShowZeroValue = false, bool const bUseOverSkillLevel = false);
extern void lwChangeInvViewGroup(int iGrp);
extern void lwSetInvViewSize(lwUIWnd UIParent);
extern void lwSetInvSafeViewSize(XUI::CXUI_Wnd* pTopUI, int const iInvType);
extern bool lwUseBigSizeInv( const EInvType kInvType );
extern void lwCashShopInvViewSize(lwUIWnd Parent, bool const bIsSafe);
extern void lwExtendInvSafeSlot(lwUIWnd kWnd, int const Idx);
extern int SetInvIconVisible(XUI::CXUI_Wnd* pParent, std::wstring const& kBuildName, int const iMaxCount, int const iMaxSlotCount);
extern int SetLeadInvExtendInvIconVisible(XUI::CXUI_Wnd* pParent, std::wstring const& kBuildName, int const iInvType, int const iUsingSlot, int const iExtendSlot);
extern void lwCallExtendInvMessageBox(lwUIWnd kSelf);
extern lwWString lwChangeIndexToSysEmoFont(int const Index);
extern int lwGetInvViewGroup();
extern void lwChangeShopViewGroup(int iGrp);
extern void lwChangeShopViewSubOrder(char const* szOrder);
extern void lwCalcValue(int iValue);
extern void lwSetCalcDigit();
extern void lwDoCalcuratorAction();
extern void lwDoSMSAction(lwUIWnd Self);
extern int lwGetMissionCashItemNo();

extern void lwCallEmporiaShop(lwUIWnd kWnd);
extern void lwDoBuyEmporiaShop(lwUIWnd kWnd);

extern void lwCallJobSkillShop(lwUIWnd kWnd);
extern void lwSetFilter_JobSkillShop(lwUIWnd kWnd);
extern void lwSelectFilter_JobSkillShop(lwUIWnd kWnd);
extern void lwLearnFilter_JobSkillShop(bool const bCheck, int const iMovePage);

extern void Send_PT_C_M_REQ_REGQUICKSLOT(size_t const slot_idx, SQuickInvInfo const &kQuickInvInfo);
extern void Send_PT_C_M_REQ_REGQUICKSLOT_VIEWPAGE(char const cViewPage);

//>>MissionUI
extern void lwDrawMissionRewardItem(lwUIWnd wnd);
extern void lwOnCallMissionRewardItemToolTip(lwUIWnd kControl, bool bIgnoreEmpty = false);

extern void lwCMP_STATE_CHANGE(int i);
extern void lwAddNewScreenBreak(float fScreenX,float fScreenY);

void lwSetBGMVolume(float fVolume, bool bConfig);
void lwSetEffectVolume(float fVolume, bool bConfig);

extern int lwGet_CMP_STATE();
extern bool lwSendSpendMoneyAction(const ESpendMoneyType kType, int iCustomData = 0);

extern void RefreshCharStateUI();	//캐릭터 정보창 리프레쉬

extern bool lwDrawModel(lwUIWnd kWnd, char const* szGuid, float fRotate);

extern void lwCannotConnectLogin();



extern void lwNewTradeStart(lwGUID kTradeGuid, lwGUID kOtherGuid );
extern void lwOnTradeConfirmButton(bool bValue);
extern lwGUID lwGetTradeGuid();
extern void lwSetTradeGuid(lwGUID kTradeGuid);
extern void lwClearTradeInfo();
extern void lwLoadingImage(int iMapIndex, EGndAttr const eMapType, int LoadingImageID = 0, bool bIsRidingPet = false, int iOldMapNo = 0);// 로딩 이미지를 부른다
extern bool lwViewOtherEquip(lwGUID kGuid);
extern bool lwViewOtherDetailInfo();
extern bool lwViewPetEquip();	//주인의 GUID
extern void lwRegistUIAction(lwUIWnd kWnd, char const* szScript);

extern bool lwRefreshRepairAlarmUI();

extern void lwSelectCoinItem(lwUIWnd kWnd);
extern void CallCoinChanger();
extern void lwSendBuyCoinItem(lwUIWnd kWnd);
extern int lwGetCoinChangerResNo(lwUIWnd kWnd);
extern void lwSendCoinMaking(int iNo);

extern bool lwGetReqInvWearableUpdate();
extern void lwSetReqInvWearableUpdate(bool bValue);
extern bool lwIsOpenGroundByNo(int iNo);
extern int lwGetAlphaFromImgAtCursor(char const* szPath, lwUIWnd kWnd);
extern DWORD SetGradeColor(E_ITEM_GRADE const eItemLv, bool bIsCash, std::wstring& wstrName);
extern void lwCallMapInfo(int iNo);
extern void lwHideUI(bool bHide);
extern void lwHidePartUI(char const* szHideFromID, bool const bHide);

extern void lwDrawPlayerLv(lwUIWnd UISelf);
extern void lwShowMiniMap(lwUIWnd UISelf);

extern bool lwCanAddToMail(lwUIWnd kWnd);

//Debug용 Abil : Value 출력 함수 bUsedDefAbilType = DefAbilType를 참조하여 무슨 어빌인지 출력할 것인가?
//출력 예시 -> AbilType(DefAbil) : AbilValue
extern void MakeAbilObjectToString(const CAbilObject* pkAbilObject, std::wstring& wstr, bool bUsedDefAbilType = true);
extern void MakeSkillDefaultAbilToString(const CSkillDef* pkSkillDef, std::wstring& wstr, bool bUseNULL = true);
extern void MakeEffectDefaultAbilToString(const CEffectDef* pkEffectDef, std::wstring& wstr, bool bUseNULL= true);

extern bool lwCanAddToMail(lwUIWnd kWnd);
extern void lwCategoryBtnInit(lwUIWnd kWnd);

extern bool lwREQ_MAIL_SEND(lwUIWnd kWnd);
extern void lwREQ_MAIL_RECV();
extern void lwInitMailUI(lwUIWnd kWnd);
extern int lwGetNowMailCount();
extern void lwPrevMailPage();
extern void lwNextMailPage();
extern void lwMakeMailList(lwUIWnd kWnd);
extern void lwOpenMail(lwUIWnd kWnd);
extern void lwDeleteMail();
extern void lwDisplayMailSrcIcon(lwUIWnd kWnd);
extern void lwGetAnnex(lwUIWnd kWnd);
extern void lwGetAnnexAll();
extern void lwDrawItemInPost(lwUIWnd wnd, int iItemNo = 0);
extern void lwOnCallItemInPostToolTip(lwUIWnd kControl);
extern void lwReturnMail();
extern void lwReturnMailAt(int const iAt);
extern void lwReplyMail(lwUIWnd kWnd);
extern void lwCheckMassItem(lwUIWnd kWnd);
extern bool lwREQ_MASSMAIL_SEND(lwUIWnd kWnd);
extern int lwCheckedMailCount(lwUIWnd kWnd);
extern void CallPostUI(lwGUID kNpcGuid);
extern void ClosePostUI();

extern void lwItemSelfProcessIconToolTip(lwUIWnd Self);

extern void lwFranYesNoMsgBox(lwGUID kNpcGuid);
extern __int64 GetNowFran();
extern void ConnectedInfo(lwUIWnd UISelf, int const iMapNo, int const iTickTime);

extern void CallToolTip_Cash(XUI::CXUI_Wnd* pkWnd, POINT3I &rPT);
extern void CallToolTip_GiftInCashShop(XUI::CXUI_Wnd* pkWnd, POINT3I &pt, SCASHGIFTINFO const& kCashShopGiftInfo);

extern void lwCallChatWindow(int const iSetTap);
extern void lwReCallQuickSlotEx();
extern void lwCallQuickSlotEx(bool const bChange);

extern void lwRecvCouponWord(lwWString kStr);
extern void lwSendEventCoupon();
extern void lwSendEventNameCoupon();

extern void CallMegaPhoneMsgUI(SMS_HEADER const& kHeader, XUI::PgExtraDataPackInfo const &kExtraDataPackInfo);

extern void lwSetLocalName(char const* szName);
extern lwWString lwGetLocalName();

extern void lwChangeCashBoxViewGroup(lwUIWnd UISelf, int const iInvType = 0);
extern void lwChangeGuildInvViewGroup(const BYTE byInvType);
extern void lwSetGuildInvViewSize(const BYTE byInvType);

extern void lwExpandCashBox(lwUIWnd UISelf);
extern void lwDrawInvRemainTime(lwUIWnd UISelf, bool const bIsCheckGoldBox = true);
extern void lwCallCommonMsgYesNoBox(char const* szText, lwPacket kPacket, bool bIsModal, int const iBoxType, char const* pCancelMsg = NULL, bool bEscIsOncancel = true);
extern void CallCommonMsgYesNoBox(std::wstring kText, int const TTID_OK, int const TTID_CANCEL, lwPacket kPacket, bool bIsModal, int const iBoxType, char const* pCancelMsg, bool bEscIsOncancel = true);
extern void UpdateCashBoxViewGroup();
void InitCashBoxViewUI(XUI::CXUI_Wnd* pParent, int const iInvType);
extern lwPoint2 const lwGetImgSizeFromPath(char const* szPath);
extern lwWString lwGetMiniMapPath(int const iMapNo);
extern int lwGetMapNoFromNpcName(lwWString rkString);
extern int lwPlayMovie(char const *pszText);
extern bool lwIsPlayMovie();
extern bool lwGetExtendSlotView();
extern void lwSetExtendSlotView(bool const bShow);
extern void lwOpenURL(char const *szText);
extern void SendEmotion(int const iID);

namespace SoulCraft
{
	extern int const iSoulUnsealingMakingNo;
	extern int const iOneTimeMaxMakingCount;

	bool PreCheckItemMaking(int const iMakingNo, int const iTestCount);
	bool CreateMakingOrder(SDefItemMakingData const& rkMakingInfo, SReqItemMaking &rkOutOrder, int const iCount, bool const bAllUseItem = false);
	int GetCanMaxTradeMaking(int const iMakingNo, int const iLimitCount = 10);
	
	void SetSimpleItemIcon(lwUIWnd kWnd, int const iItemNo);
	void CallSimpleItemToolTip(lwUIWnd kWnd, int const iItemNo);

	void SendTradeUnsealingScroll(int const iCount);
	void UpdateTradeUnselingScroll(lwUIWnd kTopWnd, int const iCount);
};

namespace SoulStoneTrade
{
	struct tagTrade;
	extern std::wstring const kSoulStoneTradeWndName;

	void AddSoulStoneTradeItem(char* pszSerial, int const iLocation, int iItemNo, int iMaterial, int iMaterialNeedsCount);
	void UpdateSoulStoneTrade();
	void SetSoulStoneTradeLocation(int const iTradeLocation);
	void IncSoulStoneTradeItemWnd(lwUIWnd kItemWnd, int const iIncVal);
	void CallSoulStoneTradeToolTip(lwUIWnd kIconWnd);
	void SendSoulStoneTrade(lwUIWnd kItemWnd);
	int GetCanMaxTradeMaking(tagTrade const & rkTradeInfo, int const iLimitCount = 10);
	bool CreateMakingOrder(tagTrade const & rkInfo, SReqSoulStoneTrade &rkOutOrder, int const iCount, bool const bAllUseItem = false);
};

namespace Item_Convert
{
	extern std::wstring const kItem_ConvertWndName;

	void AddItem_ConvertItem(int const iLocation, int iItemNo, int iMakingNo);
	void UpdateItem_Convert();
	void SetItem_ConvertLocation(int const iTradeLocation);
	void IncItem_ConvertItemWnd(lwUIWnd kItemWnd, int const iIncVal);
	void CallItem_ConvertToolTip(lwUIWnd kIconWnd);
	void SendItem_Convert(lwUIWnd kItemWnd);
}

namespace UIItemUtil
{
	bool DoQuickUseItem(int const iItemNo);

	enum E_CUSTOM_ITEM_USE_TYPE
	{
		ECIUT_NORMAL_CONSUME	= 0,
		ECIUT_CUSTOM_DEFINED	= 1,
	};

	enum E_INSURANCE_CALL_UI_TYPE
	{
		EICUT_NONE				= 0,
		EICUT_PLUS_UPGRADE		= 1,
		EICUT_RARITY_UPGRADE	= 2,
		EICUT_EQ_MAKING			= 3,
		EICUT_ENCHANT_SHIFT		= 4,
	};

	typedef std::map< std::wstring, int > CONT_CUSTOM_PARAM;
	typedef std::map< std::wstring, std::wstring > CONT_CUSTOM_PARAM_STR;

	struct SCustomItemUseInfo
	{
		BM::Stream MakeCustomData()
		{
			BM::Stream kPacket;
			kPacket.Push(eType);
			kPacket.Push(iItemNo);
			PU::TWriteTable_AA(kPacket, ContParam);
			PU::TWriteTable_AA(kPacket, ContParamStr);
			return kPacket;
			
		}
		void ReadCustomData(BM::Stream& kPacket)
		{
			kPacket.Pop(eType);
			kPacket.Pop(iItemNo);
			PU::TLoadTable_AA(kPacket, ContParam);
			PU::TLoadTable_AA(kPacket, ContParamStr);
		}

		bool IsEmpty()
		{
			return (0 == iItemNo);
		}

		E_CUSTOM_ITEM_USE_TYPE	eType;
		int						iItemNo;
		CONT_CUSTOM_PARAM		ContParam;
		CONT_CUSTOM_PARAM_STR	ContParamStr;
	};

	bool SearchEqualTypeItemList(int const iTargetType, ContHaveItemNoCount& kList);
	void CallCommonUseCustomTypeItems(ContHaveItemNoCount const& kList, E_CUSTOM_ITEM_USE_TYPE const ec_Type, CONT_CUSTOM_PARAM const& kParam, CONT_CUSTOM_PARAM_STR const& kParamStr);
	void OnClickCustomTypeItem(lwUIWnd UISelf);
	void OnDrawCustomTypeItemIcon(lwUIWnd UISelf);
	void OnCallCustomItemTooltip(lwUIWnd UISelf);
	void OnOKUseCustomTypeItem(lwUIWnd UISelf);
	
	struct SUICT_Info
	{
		EUseItemCustomType eType;	// 5019 어빌타입
		int iCustomVal1;			// 5020 어빌타입
		int iCustomVal2;			// 5023 어빌타입
		int iCustomVal3;			// 5024 어빌타입
		SUICT_Info(): eType(UICT_NONE),iCustomVal1(0),iCustomVal2(0),iCustomVal3(0)	{}
	};
	// 아이템 번호로 UICT 정보를 얻어온다(User Item Custom Type)
	bool GetUICTInfo(int const iItemNo, SUICT_Info& kResult);
}

extern bool MakeAbilNameString2(WORD const Type, std::wstring &wstrText);
extern bool IsItemMoveEnable(bool const bWarn = true);

extern bool lwCopyPlayerNameToClipBoard(lwGUID const kPlayerGuid);
extern void lwCSNotifyCheck();

extern bool lwParticleTextureChange(char const* pcParticle, float const fScale, char const* pcNode, int const GeoNo, char const* pcTexPath);

namespace UIMacroCheck
{
	void RecvMC_Command(WORD const wPacketType, BM::Stream& rkPacket);
	void CallMacroCheckUI(wchar_t const wcKey, int const iIteration, short const sType);
	std::wstring const MakePassCode(wchar_t const wcKey, int const iIteration);
	std::wstring const PassCodeColorInput(std::wstring const& kStr);
	void lwUpdateMacroTimer(lwUIWnd UISelf);
	void lwSendMacroPassword(int const iAnswer);
	void lwCloseMacroUI(lwUIWnd UISelf);
	bool lwIsOnMacroCheck();
	void lwCallMacroCheckUI_IfNotComplete(int iElapsTimeMilSec);
};
extern DWORD lwGetLocalTime();
extern int lwGetLocale();
namespace UIGemStore
{
	extern void lwSendReqGemStore(lwGUID lwkGuid);
	extern void lwClickGemStoreListItem(lwUIWnd kListItem);
	extern void lwClearGemStoreData();
	extern void lwGemStoreDrawIconImage(lwUIWnd kSelf);
	extern void lwGemStoreDrawIconTooltip(lwUIWnd kSelf);
	extern bool lwSendReqGemTrade();
	extern bool lwSendReqGemTradeTree();

	extern void OnClickGemStoreBeginPage(lwUIWnd kSelf);
	extern void OnClickGemStoreEndPage(lwUIWnd kSelf);
	extern void OnClickGemStoreJumpPrevPage(lwUIWnd kSelf);
	extern void OnClickGemStoreJumpNextPage(lwUIWnd kSelf);
	extern void OnClickGemStorePage(lwUIWnd kSelf);

	extern void lwSendReqDefGemStore(lwGUID lwkGuid, int const iMenu, bool const bCheckClass = false);	
	extern bool lwIsNotEmptyDefGemStore(lwGUID lwkGuid, int const iMenu);
	extern int lwGetGemStoreTitleMenu();
	extern void lwSendReqEventStore(lwGUID lwkGuid);
	extern void lwSetGemStoreType(int const iType);
	extern int lwGetGemStoreType();

	extern void SetItemTree(int const iSelectedItemNo, int const iOrderIndex);
	extern void ItemTreeClear();
	extern bool IsItemTreeSlot(int const iSelectedItemNo, int const iOrderIndex);
	
}
extern void CallGetItemAlam(std::wstring const& wstrStr, int const iItemNo, int const iMilliSec = 5000, const int iLocationY = 0);
extern void CallGetItemAlam2(std::wstring const& wstrStr, int const iItemNo, int const iMilliSec = 5000);

extern void lwSetUISizeForTextY(lwUIWnd kWnd);

extern char const* lwGetKeynoToName(int const iKeyNo);

namespace XUIListUtil
{
	void SetMaxItemCount(XUI::CXUI_List* pList, int const iMax);
};

namespace PgUIUtil
{
	void DrawIconToItemNo(XUI::CXUI_Wnd* pWnd, int const iItemNo, float const fScale = 1.0f);
	void ResizeFormSize(XUI::CXUI_Wnd* pWnd, int const ExtendSizeX, int const ExtendSizeY = 0, bool const bChange = false);
	void MoveFormLocation(XUI::CXUI_Wnd* pWnd, int const MoveLocX, int const MoveLocY = 0, bool const bChange = false);
};

extern bool lwIgnoreChannelUI(bool const bSet=false, bool const bIgnore=false);
extern void lwRemoveBuffEffect(lwUIWnd kWnd);

extern void SetBlockNickFilterProcessType(int iType);
extern int GetBlockNickFilterProcessType();
extern void SetBadWordFilterProcessType(int iType);
extern int GetBadWordFilterProcessType();

extern lwWString GetUKeynoToKeyStr(int const iUKeyNo);
extern char const* lwGetCoupleName();
extern lwWString GetUKeynoToKeyStr(int const iUKeyNo);

extern void lwSendFireOfLoveSMS(lwUIWnd kWnd);

extern int lwChangeAllListChkBtn(char const* pcParent, char const* pcList, char const* pcChkBtn, bool bCheckState);
extern bool AttatchParticleToActorNode(char* pcNpcActorName,int iSlot, char const* pcTargetName, char const* pcParticle, float fScale);
extern bool AttatchParticleToActorPos(char* pcNpcActorName,int iSlot, lwPoint3 kPos, char const* pcParticle, float fScale);

extern void lwDrawIconToItemNo(lwUIWnd kWnd, int iItemNo, float fScale, lwPoint2F kScaleCenter);

extern void lwDelRenderModelNIF(char const* szRenderModelName, char const* szNIFName);

extern void CallRepairItemUseUI(SItemPos const& kItemPos, PgBase_Item const& kItem);
extern void lwOnMouseUpRepairItemUseIcon(lwUIWnd kSelf);
extern void lwClickRepairItemUseButton(lwUIWnd kSelf);
extern void lwOnMouseUpEnchantItemUseIcon(lwUIWnd kSelf);
extern bool IsUsableItemForceInchant(int iMtrlItemNo, PgBase_Item const* pkTargetItem, int& riErrorMsgOut);
extern void lwClickEnchantItemUseButton(lwUIWnd kSelf);
extern void lwMakeForceInchantUIText(lwUIWnd kSelf);

extern void lwOnCallCenterMiniMap(lwGUID kGuid, int const iAliveTime = 0);
extern void lwOnCloseCenterMiniMap(lwUIWnd UISelf);
extern void OnClickMiniMapTeleMoveToTarget(POINT3 const& kPos);

extern void lwOnCallBattleSquareMiniMap();
extern void lwUpdateCheckDrawLimitBreakBar();
extern void lwMoveVCenterUI_TextBase(lwUIWnd UIParent, lwUIWnd UISelf, int iX);

extern int lwGetShopGoodsViewSize();

extern bool lwIsMapFillRestExp(int const iMapNo);
extern bool lwShowRestExpBar();

extern size_t FindRepairInfo(BaseItemMap &rkMap, PgInventory *pkInv = NULL);

extern void lwCallToolTipRepair(lwPoint2 &pt);

extern lwWString lwGetRestExpToString();
extern void lwNotice_Show(lwWString klwStr, int const iLevel, bool const bCheckEqualMessage);
extern void lwUseHiddenItem(lwUIWnd lwFrm);
extern void TimeToString(DWORD const dwTimeSec, std::wstring & kOutText );
extern POINT2 GetDefaultLocation(POINT2 const & kPos);
void CalcAbosolute(POINT2 & rkPos, bool const bAbsoluteX=false, bool const bAbsoluteY=false);
void CalcAbosolute(NiPoint3 & rkPos, bool const bAbsoluteX=false, bool const bAbsoluteY=false);

extern void GetJobSkillText_GatherType(int const iSkillNo, BM::vstring& rkText);//스킬 타입별 텍스트
extern void GetJobSkillText_ToolType(int const iItemNo, BM::vstring& rkText);//도구 타입별 텍스트
extern bool GetJobSkillText_ToolType(int const iGatherType, int const iSkillNo, BM::vstring& rkTextOut);//도구 타입별 텍스트
extern void GetJobSkillText_MaterialType(int const iItemNo, BM::vstring& kOutput);//가구장치 재료별 텍스트
extern void NoticeAddExpertnessMsg(PgPlayer const & rkPlayer, int const iSkillNo, int const iAddExpertness);//숙련도 오름 메세지
extern void CallJobSkillGatherTimer(int const iSkillNo, DWORD const dwRunTurnTime);//채집 프로그래스 UI

extern void UseJobSkill(int const iJobSkillNo);
extern void lwSetJobSkillTogleState(int const iSkillNo, bool const bToggleOn);
extern void lwTogleOffAllJobSkillQuicIcon();

bool lwPickWnd(char const *pszText);
lwUIWnd lwAddChildUI(lwUIWnd kParent, char const *pszElementID, char const *pszNewID, bool const bRemove);

extern void lwCallSafeAdditionUI(char const* szID, bool bOnlyChangePos);
extern void CoverSafeAdditionUI(XUI::CXUI_Wnd* pkWnd);
extern bool CheckSafeAddition(PgBase_Item const &kItem, BM::Stream &kPacket);		// 펫에 추가 인벤.
extern bool CheckSafeAdditionItem(PgBase_Item const & Item, BM::Stream & Packet);	// 일반 아이템에 추가 인벤.
extern PgBase_Item CompairToWarnForSafeAddition(const SItemPos &kCasterPos, const SItemPos &kTargetPos);		// 펫에 추가 인벤.
extern PgBase_Item CompairToWarnForSafeAdditionItem(SItemPos const & CasterPos, SItemPos const & TargetPos);	// 일반 아이템에 추가 인벤.

extern void lwShowMyHomeSize();

extern void UseItemSkill(int const iItemSkill);

extern void GetEqiupPosString(DWORD const dwEquipLimit, bool const bIsPet, std::wstring& wstrText);

extern void MakeEquipTypeText(EEquipType const eEquipType, std::wstring &wstrTextOut);	//장착타입별 문구 삽입(무기/방어구/악세사리)

extern void GetEqiupPosStringMonsterCard(DWORD const dwEquipLimit, std::wstring& wstrText);

extern bool MakeElementTypeText(int const iItemNo, std::wstring &wstrTextOut);	//원소 추출 타입별 문구 삽입

extern bool TryReserveActionToMyActor(int const iSkillNo);

extern bool lwIsWideResolution( const unsigned int iWidth, const unsigned int iHeight );

extern bool lwSetMiniClassIconIndex(lwUIWnd kWnd, int const iClassNo);

extern int GetCommonClassEquipItemType(__int64 const iClassLimit);


extern bool SetUIAddResToImage(XUI::CXUI_Image* pItemImg, int iItemNo);

extern void lwParseXml_BasicCombo();

extern bool MakeWeaponTypeName(int const iWeaponType, std::wstring &wstrText);

extern void MakeClassLimitText(__int64 i64ClassLimit, std::wstring &wstrText);

int lwGetNextIngameNoticeShowTimeInSec(void);
typedef enum eHPSTATE
{
	HPSTATE_NORMAL = 0,
	HPSTATE_LOW,
	HPSTATE_VERYLOW,
	HPSTATE_DIE,
};
void UpdateLowHPWarnning(int iBeforeHP, int iCurHP);
eHPSTATE GetHPState(int iHP, int iMaxHP);
void AdjustLowHPWarnningByResolution(void);

//namespace SkillLinkage
//{
//	extern void lwCallLinkageUI();
//	extern void lwUpdateLinkageUI();
//}
extern void lwReqExitInstanceDungeon();

extern bool lwCheckReservedClassPromotionAction();

extern void SetTextToFitWindowSize(XUI::CXUI_Wnd * pWnd, std::wstring & TextContent, int const ContentID = 0, int const margin = 20);
void OnRefresh_DungeonPartyList(BM::GUID const& rkGuid, CONT_INDUN_PARTY_CHANNEL const& rkList);

void CallNpcList(XUI::CXUI_Wnd* pkMinimap);

namespace TooltipUtil
{
	template<typename T>
	bool MakeValueColorStr(T Value, BM::vstring& rOut, wchar_t* pwcFormat = NULL)	// -값이면 빨강, +값이면 초록, 0이면 흰색
	{
		BM::vstring vApllyColor;
		BM::vstring vRevertColor;
		rOut = BM::vstring();
		if(0 < Value)
		{
			vApllyColor = ::TTW(1548);	// 녹색
			vRevertColor = ::TTW(1554);	// 흰색
		}
		else if(0 > Value)
		{
			vApllyColor = ::TTW(89999);	// 빨강
			vRevertColor = ::TTW(1554); // 흰색
		}
		if( 0 != vApllyColor.size() )
		{
			rOut = vApllyColor;
		}
		if(NULL == pwcFormat)
		{
			rOut += Value;
		}
		else
		{
			rOut += BM::vstring(Value, pwcFormat);
		}
		if(  0 != vRevertColor.size() )
		{
			rOut += vRevertColor;
		}
		return true;
	}

	BM::vstring GetValueColor(float const fValue, int const iAbilType = 0);	// 어빌값에 해당하는 글자색
};

#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUI_H