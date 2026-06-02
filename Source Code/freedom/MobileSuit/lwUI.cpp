#include "StdAfx.h"
#include "Themida/ThemidaSDK.h"
#include "Variant/PgBattleSquare.h"
#include "lwUI.h"
#include "PgNetwork.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgWorld.h"
#include "ServerLib.h"
#include "PgMobileSuit.h"
#include "PgXmlLoader.h"
#include "PgItemEx.h"
#include "PgDropBox.h"
#include "PgAction.h"
#include "Variant/PgPlayer.h"
#include "Variant/constant.h"
#include "PgQuest.h"
#include "PgRemoteManager.h"
#include "PgNifMan.h"
#include "Variant/PgClassDefMgr.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "PgRenderMan.h"
#include "PgSkillTree.h"
#include "Variant/Inventory.H"
#include "PgOption.h"
#include "lwUIMission.h"
#include "PgMissionComplete.h"
#include "PgScreenBreak.H"
#include "PgSoundMan.h"
#include "PgGuild.h"
#include "lwTrade.h"
#include "PgWorldMapPopUpUI.h"
#include "PgAMPool.h"
#include "Variant/ItemMakingDefMgr.h"
#include "Variant/ItemBagMgr.h"
#include "lwCouple.h"
#include "lwPacket.h"
#include "PgMail.h"
#include "PgMarket.h"
#include "lwUIQuest.h"
#include "PgInputSystem.H"
#include "PgSystemInventory.h"
#include "PgFriendMgr.h"
#include "PgHelpSystem.h"
#include "PgQuestUI.h"
#include "lwMonsterKillUIMgr.h"
#include "lwMissionScoreUIMgr.h"
#include "lwMToMChat.h"
#include "lohengrin/packetstruct.h"
#include "PgEventTimer.h"
#include "PgChatMgrClient.h"
#include "PgTextTypistWnd.h"
#include "PgMoveAnimateWnd.h"
#include "lwUILogin.h"
#include "Variant/Global.h"
#include "PgMyActorViewMgr.h"
#include "lwUICharInfo.h"
#include "PgMovieMgr.h"
#include "lwWorld.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
#include "lwStyleString.h"
#include "PgEnergyGuage.H"
#include "PgUISound.h"
#include "lwUIMemTransCard.h"
#include "PgMacroCheck.h"
#include "lwUIGemStore.h"
#include "PgCashShop.h"
#include "lwUISealProcess.h"
#include <ShellAPI.h>
#include "PgCommandMgr.h"
#include "PgCmdLineParse.h"
#include "lwUIItemPlusUpgrade.h"
#include "lwUIItemRarityUpgrade.h"
#include "lwMarket.h"
#include "PgCoupleMgr.h"
#include "lwUIFireLove.h"
#include "lwCashItem.h"
#include "lwUIPet.h"
#include "PgHomeTown.h"
#include "PgMiniMap.h"
#include "PgBattleSquare.h"
#include "lwPlayTime.h"
#include "lwHomeUI_Script.h"
#include "lwCashShop.h"
#include "PgFontSwitchTable.h"
#include "NewWare/Scene/ApplyTraversal.h"
#include "PgContentsBase.h"
#include "PgEventScriptSystem.h"
#include "PgHomeRenew.h"
#include "PgVendor.h"
#include "lwVendor.h"
#include "lwJobSkillLearn.h"
#include "lwJobSkillView.h"
#include "PgSelectStage.h"
#include "Variant/PgJobSkillTool.h"
#include "lwUICSGacha.h"
#include "lwUICostumeMix.h"
#include "PgHome.h"
#include "lwSoulTransfer.h"
#include "PgCustomUI_Summmoner.h"
#include "lwActionTargetList.h"
#include "PgTrigger.h"
#include "PgWorldEventClientMgr.h"
#include "lwDefenceMode.h"
#include "PgPvPGame.h"
#include "CustomUtility.inl"
#include "Variant/PortalAccessInfo.h"
#include "lwMinimap_Script.h"
#include "PgConstellation.h"
#include "PgDungeonMinimap.h"

extern lwWString lwGetTT(int iTextTableNo);
extern void lwChangeShineStoneCount(lwUIWnd klwWnd, int iItemNo);
extern int lwChangeQuickSlotViewPage(int const iValue);
extern int lwGetQuickSlotViewPage();
extern void OnMsgBoxClose(lwUIWnd Self);
extern bool CheckEnchantBundle(PgBase_Item const& kItem);
extern void lwClearPostSendItem();
extern bool lwIsQuickSkillSlotActive(int const iIconKey);
extern void lwOnClickStrategySkill(int const iType);
extern void lwPartyStateCloseBtnDown(lwUIWnd kWnd);
extern int lwGetEnterDefenceMinLevel(int const iMissionNo, int const iMissionLv);
extern int lwGetEnterDefenceMaxLevel(int const iMissionNo, int const iMissionLv);
bool lwIsPlayerPlayTime();
lwPoint2 lwGetDefaultLocation(int const x, int const y);
extern lwWString lwMissionClearQuestText(int const iLevel);
extern bool lwIsMissionComplatedQuest(int const iLevel);
lwAction lwCustomUISummoner_DoAction(char const* szActionName);
void lwCustomUISummoner_TargetList(lwActionTargetList kout_FoundTargets);
void lwCustomUISummoner_MenuUpdatePos();
lwUIWnd lwGetFocusedEdit();
void lwShowActorUnitType(EUnitType const eType, bool const bShow, bool const bEventScript);
void lwSendCheckWorldEvent(lwActor kActor, lwWString kLimitQuestID, lwWString kEventScriptID);
extern void lwDoQuickUseItem(int const iItemNo);
int lwGetDefence7RelayItem();
int lwGetDefence7PointCopyItem();
extern int lwGetDefence7_RelayPoint(int const iStage);
void lwWorld_Dungeon_Portal(lwTrigger kTrigger, lwActor kActor);
void lwOnEnter_DungeonParty(lwUIWnd kWnd);
void lwOnSort_DungeonPartyList(lwUIWnd kWnd, int const MenuType);

extern bool lwUseWebLinkage();

namespace lwGuild
{
    extern void EmporiaGiveUp();
}

BM::GUID g_LastAttackTarget;
PgPilot *g_pkBoss = NULL;	//보스 파일럿 정보
EInvType	g_EquipInvViewType;

typedef std::map<int, std::wstring> BarInfoMap;
typedef BarInfoMap::const_iterator	BarInfoMapItor;
BarInfoMap g_BarInfoMap;
TCHAR g_szSuperGirlURL[512]={0,};

#if !defined(USE_INB) && !defined(EXTERNAL_RELEASE)
void AddTestPacket(lwPacket kPacket)
{
	if( g_pkRemoteManager )
	{
		g_pkRemoteManager->AddPacket( *kPacket() );
	}
}
#endif


lwUIWnd::lwUIWnd(XUI::CXUI_Wnd *pWnd, bool bAssert)
{
	if (bAssert)
	{
		PG_WARNING_LOG(pWnd);
	}
	self = pWnd;
}

bool lwUIWnd::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

#if !defined(USE_INB) && !defined(EXTERNAL_RELEASE)
	def(pkState, "AddTestPacket", &AddTestPacket);
#endif

	class_<lwUIWnd>(pkState, "UIWnd")
		.def(pkState, constructor<XUI::CXUI_Wnd *>())

		.def(pkState, "GetLocation", &lwUIWnd::GetLocation)
		.def(pkState, "SetLocation", &lwUIWnd::SetLocation)
		.def(pkState, "SetLocation3", &lwUIWnd::SetLocation3)
		.def(pkState, "GetLocation3", &lwUIWnd::GetLocation3)
		.def(pkState, "SetPriority", &lwUIWnd::SetPriority)
		.def(pkState, "GetPriority", &lwUIWnd::GetPriority)

		.def(pkState, "GetAlignX", &lwUIWnd::GetAlignX)
		.def(pkState, "GetAlignY", &lwUIWnd::GetAlignY)
		.def(pkState, "SetAlignX", &lwUIWnd::SetAlignX)
		.def(pkState, "SetAlignY", &lwUIWnd::SetAlignY)
		.def(pkState, "VAlign", &lwUIWnd::VAlign)

		.def(pkState, "Close", &lwUIWnd::Close)
		.def(pkState, "CloseParent", &lwUIWnd::CloseParent)
		.def(pkState, "ClearOutside", &lwUIWnd::ClearOutside)
		.def(pkState, "SetLastTargetInfo", &lwUIWnd::SetLastTargetInfo)
		.def(pkState, "SetPilotBar", &lwUIWnd::SetPilotBar)
		
		.def(pkState, "CallIconToolTip", &lwUIWnd::CallIconToolTip)
		.def(pkState, "CallImageIconToolTip", &lwUIWnd::CallImageIconToolTip)
		.def(pkState, "DrawIcon", &lwUIWnd::DrawIcon)
		.def(pkState, "DrawImageIcon", &lwUIWnd::DrawImageIcon)
		.def(pkState, "DrawCursorIcon", &lwUIWnd::DrawCursorIcon)
		.def(pkState, "DrawCoolTimeIcon", &lwUIWnd::DrawCoolTimeIcon)

		.def(pkState, "DrawMoney", &lwUIWnd::DrawMoney)
		.def(pkState, "DrawEmoticonMoney", &lwUIWnd::DrawEmoticonMoney)
		.def(pkState, "DrawMoneyByValue", &lwUIWnd::DrawMoneyByValue)

		.def(pkState, "SetCursorState", &lwUIWnd::SetCursorState)
		.def(pkState, "GetCursorState", &lwUIWnd::GetCursorState)
		.def(pkState, "CursorToThis", &lwUIWnd::CursorToThis)
		
		.def(pkState, "GetContCount", &lwUIWnd::GetContCount)
		.def(pkState, "GetContAt", &lwUIWnd::GetContAt)

//		.def(pkState, "UVIndex", &lwUIWnd::UVIndex)
		.def(pkState, "IconGrp", &lwUIWnd::IconGrp)
		.def(pkState, "IconKey", &lwUIWnd::IconKey)

		.def(pkState, "SetIconKeyAndResNo", &lwUIWnd::SetIconKeyAndResNo)
		.def(pkState, "SetIconKey", &lwUIWnd::SetIconKey)
		.def(pkState, "SetIconResNo", &lwUIWnd::SetIconResNo)
		.def(pkState, "SetAlpha", &lwUIWnd::SetAlpha)
		.def(pkState, "GetAlpha", &lwUIWnd::GetAlpha)
		.def(pkState, "SetMaxAlpha", &lwUIWnd::SetMaxAlpha)
		.def(pkState, "GetMaxAlpha", &lwUIWnd::GetMaxAlpha)
		.def(pkState, "BarNow", &lwUIWnd::BarNow)
		.def(pkState, "BarMax", &lwUIWnd::BarMax)
		.def(pkState, "GetBarNow", &lwUIWnd::GetBarNow)
		.def(pkState, "GetBarMax", &lwUIWnd::GetBarMax)
		.def(pkState, "GetBarCurrent", &lwUIWnd::GetBarCurrent)
		.def(pkState, "SetBarReverse", &lwUIWnd::SetBarReverse)

		.def(pkState, "IconToCursor", &lwUIWnd::IconToCursor)
		.def(pkState, "CursorToIcon", &lwUIWnd::CursorToIcon)
		.def(pkState, "SetPilotLv", &lwUIWnd::SetPilotLv)
		.def(pkState, "SetBuildIconKey", &lwUIWnd::lwSetBuildIconKey)
		.def(pkState, "SetBuildIChannelBtn", &lwUIWnd::lwSetBuildIChannelBtn)
		.def(pkState, "DeleteIcon", &lwUIWnd::DeleteIcon)
		.def(pkState, "CursorToIconFast", &lwUIWnd::CursorToIconFast)
		.def(pkState, "IconDoAction", &lwUIWnd::IconDoAction)
		.def(pkState, "OnOk", &lwUIWnd::OnOk)
		.def(pkState, "OnCancel", &lwUIWnd::OnCancel)
		.def(pkState, "MsgBoxOwnerGuid", &lwUIWnd::MsgBoxOwnerGuid)
		.def(pkState, "MsgBoxType", &lwUIWnd::MsgBoxType)
		.def(pkState, "AdjustToOwnerPos", &lwUIWnd::AdjustToOwnerPos)
		.def(pkState, "SetLocationByWorldNode", &lwUIWnd::SetLocationByWorldNode)
		.def(pkState, "RemoveInfoUI", &lwUIWnd::RemoveInfoUI)
		.def(pkState, "GetOwnerGuid", &lwUIWnd::GetOwnerGuid)
		.def(pkState, "SetOwnerGuid", &lwUIWnd::SetOwnerGuid)
		.def(pkState, "GetControl", &lwUIWnd::GetControl)

		.def(pkState, "UpWheal", &lwUIWnd::UpWheal)
		.def(pkState, "DownWheal", &lwUIWnd::DownWheal)

		.def(pkState, "GetParent", &lwUIWnd::GetParent)
		.def(pkState, "SetParent", &lwUIWnd::SetParent)
		.def(pkState, "GetTopParent", &lwUIWnd::GetTopParent)
		.def(pkState, "IsStaticIcon", &lwUIWnd::IsStaticIcon)
		.def(pkState, "IsIcon", &lwUIWnd::IsIcon)
		.def(pkState, "SetStaticIcon", &lwUIWnd::SetStaticIcon)
		.def(pkState, "SetUVIndex", &lwUIWnd::SetUVIndex)
		.def(pkState, "GetUVIndex", &lwUIWnd::GetUVIndex)
		.def(pkState, "SetMaxUVIndex", &lwUIWnd::SetMaxUVIndex)
		.def(pkState, "GetMaxUVIndex", &lwUIWnd::GetMaxUVIndex)
		.def(pkState, "Visible", &lwUIWnd::Visible)
		.def(pkState, "SetInvalidate", &lwUIWnd::SetInvalidate)
		.def(pkState, "IsVisible", &lwUIWnd::IsVisible)
		.def(pkState, "SetEditFocus", &lwUIWnd::SetEditFocus)
		.def(pkState, "GetEditLimitLength", &lwUIWnd::GetEditLimitLength)
		.def(pkState, "SetEditLimitLength", &lwUIWnd::SetEditLimitLength)
		.def(pkState, "SetEditText", &lwUIWnd::SetEditText)
		.def(pkState, "SetEditTextW", &lwUIWnd::SetEditTextW)
		.def(pkState, "GetEditText", &lwUIWnd::GetEditText)
		.def(pkState, "GetEditText_TextBlockApplied", &lwUIWnd::GetEditText_TextBlockApplied)
		.def(pkState, "SetStaticText", &lwUIWnd::SetStaticText)
		.def(pkState, "SetStaticTextW", &lwUIWnd::SetStaticTextW)
		.def(pkState, "GetStaticText", &lwUIWnd::GetStaticText)
		.def(pkState, "SetFontColor", &lwUIWnd::SetFontColor)
		.def(pkState, "SetFontColorRGBA", &lwUIWnd::SetFontColorRGBA)
		.def(pkState, "SetFontFlag", &lwUIWnd::SetFontFlag)
		.def(pkState, "GetFontFlag", &lwUIWnd::GetFontFlag)
		.def(pkState, "TempTreeTest", &lwUIWnd::TempTreeTest)
		.def(pkState, "SetOwnerState", &lwUIWnd::SetOwnerState)
		.def(pkState, "GetOwnerState", &lwUIWnd::GetOwnerState)
		.def(pkState, "IsNil", &lwUIWnd::IsNil)
		.def(pkState, "GetSelf", &lwUIWnd::GetSelf)
		.def(pkState, "RefreshMessageLog", &lwUIWnd::RefreshMessageLog)
		.def(pkState, "CheckState", &lwUIWnd::CheckState)
		.def(pkState, "GetCheckState", &lwUIWnd::GetCheckState)
		.def(pkState, "LockClick", &lwUIWnd::LockClick)
		.def(pkState, "Disable", &lwUIWnd::Disable)
		.def(pkState, "IsDisable", &lwUIWnd::IsDisable)
		.def(pkState, "SetButtonTextColor", &lwUIWnd::SetButtonTextColor)
		.def(pkState, "ExploreTypingHistory", &lwUIWnd::ExploreTypingHistory)
		.def(pkState, "ResetTypingHistory", &lwUIWnd::ResetTypingHistory)
		.def(pkState, "InitMiniMap", &lwUIWnd::InitMiniMap)
		.def(pkState, "DrawMiniMap", &lwUIWnd::DrawMiniMap)
		.def(pkState, "DrawMiniMapUI", &lwUIWnd::DrawMiniMapUI)
		.def(pkState, "SetMiniMapTriggerIconUI", &lwUIWnd::SetMiniMapTriggerIconUI)
		.def(pkState, "SetShowMiniMap", &lwUIWnd::ShowMiniMap)
		.def(pkState, "CloseMiniMap", &lwUIWnd::CloseMiniMap)
		.def(pkState, "GetZoomMiniMap", &lwUIWnd::GetZoomMiniMap)
		.def(pkState, "ZoomMiniMap", &lwUIWnd::ZoomMiniMap)
		.def(pkState, "ZoomMiniMapFixedFactor", &lwUIWnd::ZoomMiniMapFixedFactor)
		.def(pkState, "MouseOverMiniMap", &lwUIWnd::MouseOverMiniMap)
		.def(pkState, "MouseClickMiniMap", &lwUIWnd::MouseClickMiniMap)
		.def(pkState, "SetWndSize", &lwUIWnd::SetWndSize)
		.def(pkState, "CloneMiniMap", &lwUIWnd::CloneMiniMap)
		//.def(pkState, "ScrollMiniMap", &lwUIWnd::ScrollMiniMap)
		.def(pkState, "InitWorldMap", &lwUIWnd::InitWorldMap)
		.def(pkState, "TermWorldMap", &lwUIWnd::TermWorldMap)
		.def(pkState, "DrawWorldMap", &lwUIWnd::DrawWorldMap)
		.def(pkState, "PulseWorldMap", &lwUIWnd::PulseWorldMap)		
		.def(pkState, "InSideWorldMap", &lwUIWnd::InSideWorldMap)
		.def(pkState, "OutSideWorldMap", &lwUIWnd::OutSideWorldMap)
		.def(pkState, "MouseClickWorldMap", &lwUIWnd::MouseClickWorldMap)
		.def(pkState, "MouseOverWorldMap", &lwUIWnd::MouseOverWorldMap)
		.def(pkState, "InitWorldMapPopUp", &lwUIWnd::InitWorldMapPopUp)
		.def(pkState, "TermWorldMapPopUp", &lwUIWnd::TermWorldMapPopUp)
		.def(pkState, "DrawPetModel", &lwUIWnd::DrawPetModel)
		.def(pkState, "TurnPetModel", &lwUIWnd::TurnPetModel)
		.def(pkState, "SetLocationToMyActor", &lwUIWnd::SetLocationToMyActor)
		
		.def(pkState, "SetCustomDataAsBool", &lwUIWnd::SetCustomData<bool>)
		.def(pkState, "GetCustomDataAsBool", &lwUIWnd::GetCustomData<bool>)
		.def(pkState, "SetCustomDataAsBYTE", &lwUIWnd::SetCustomData<BYTE>)
		.def(pkState, "GetCustomDataAsBYTE", &lwUIWnd::GetCustomData<BYTE>)
		.def(pkState, "SetCustomDataAsShort", &lwUIWnd::SetCustomData<short>)
		.def(pkState, "GetCustomDataAsShort", &lwUIWnd::GetCustomData<short>)
		.def(pkState, "SetCustomDataAsInt", &lwUIWnd::SetCustomData<int>)
		.def(pkState, "GetCustomDataAsInt", &lwUIWnd::GetCustomData<int>)
		.def(pkState, "SetCustomDataAsSize_t", &lwUIWnd::SetCustomData<size_t>)
		.def(pkState, "GetCustomDataAsSize_t", &lwUIWnd::GetCustomData<size_t>)
		.def(pkState, "SetCustomDataAsFloat", &lwUIWnd::SetCustomData<float>)
		.def(pkState, "GetCustomDataAsFloat", &lwUIWnd::GetCustomData<float>)
		.def(pkState, "SetCustomDataAsGuid",&lwUIWnd::SetCustomDataAsGuid)
		.def(pkState, "GetCustomDataAsGuid",&lwUIWnd::GetCustomDataAsGuid)
		.def(pkState, "SetCustomDataAsStr",&lwUIWnd::SetCustomDataAsStr)
		.def(pkState, "GetCustomDataAsStr",&lwUIWnd::GetCustomDataAsStr)
		.def(pkState, "SetCustomDataAsPacket", &lwUIWnd::SetCustomDataAsPacket)
		.def(pkState, "GetCustomDataAsPacket", &lwUIWnd::GetCustomDataAsPacket)
		.def(pkState, "ClearCustomData", &lwUIWnd::ClearCustomData)
		.def(pkState, "GetCustomDataSize", &lwUIWnd::GetCustomDataSize)
		.def(pkState, "CopyCustomData", &lwUIWnd::CopyCustomData)
		.def(pkState, "GetUniqueUIType", &lwUIWnd::GetUniqueUIType)
		
		.def(pkState, "IsFocus", &lwUIWnd::IsFocus)
		.def(pkState, "IsEditFocus", &lwUIWnd::IsEditFocus)
		.def(pkState, "Effect_Open", &lwUIWnd::Effect_Open)
		.def(pkState, "Effect_Stop", &lwUIWnd::Effect_Stop)
		.def(pkState, "Effect_Clear", &lwUIWnd::Effect_Clear)
		.def(pkState, "GetWidth", &lwUIWnd::GetWidth)
		.def(pkState, "GetHeight", &lwUIWnd::GetHeight)
		
		// RenderModel
		.def(pkState, "InitRenderModel", &lwUIWnd::InitRenderModel)
		.def(pkState, "DrawRenderModel", &lwUIWnd::DrawRenderModel)
		.def(pkState, "ClearRenderModel", &lwUIWnd::ClearRenderModel)
		.def(pkState, "DelRenderModel", &lwUIWnd::DelRenderModel)
		.def(pkState, "AddToDrawListRenderModel", &lwUIWnd::AddToDrawListRenderModel)
		.def(pkState, "RenderModelCameraZoom", &lwUIWnd::RenderModelCameraZoom)
		.def(pkState, "RenderModelCameraZoomSubstitution", &lwUIWnd::RenderModelCameraZoomSubstitution)
		.def(pkState, "RenderModelOrthoCameraZoom", &lwUIWnd::RenderModelOrthoCameraZoom)
		.def(pkState, "RenderModelOrthoCameraZoomSubstitution", &lwUIWnd::RenderModelOrthoCameraZoomSubstitution)
		.def(pkState, "RenderModelOrthoCameraZoomSubstitutionOldVer", &lwUIWnd::RenderModelOrthoCameraZoomSubstitutionOldVer)
		.def(pkState, "SetRenderModelCameraZoomMinMax", &lwUIWnd::SetRenderModelCameraZoomMinMax)
		.def(pkState, "RotateRenderModel", &lwUIWnd::RotateRenderModel)
		.def(pkState, "SetAutoRotateRenderModel", &lwUIWnd::SetAutoRotateRenderModel)
		.def(pkState, "AddRenderModelActorByGuid", &lwUIWnd::AddRenderModelActorByGuid)
		.def(pkState, "AddRenderModelActorByID", &lwUIWnd::AddRenderModelActorByID)
		.def(pkState, "AddRenderModelActor", &lwUIWnd::AddRenderModelActor)
		.def(pkState, "AddRenderModelActorByPath", &lwUIWnd::AddRenderModelActorByPath)
		.def(pkState, "SetRenderModelActorTranslate", &lwUIWnd::SetRenderModelActorTranslate)
		.def(pkState, "SetRenderModelActorRotate", &lwUIWnd::SetRenderModelActorRotate)
		.def(pkState, "AddRenderModelNIFByID", &lwUIWnd::AddRenderModelNIFByID)
		.def(pkState, "AddRenderModelNIFByPath", &lwUIWnd::AddRenderModelNIFByPath)
		.def(pkState, "SetRenderModelNIFTranslate", &lwUIWnd::SetRenderModelNIFTranslate)
		.def(pkState, "SetRenderModelNIFRotate", &lwUIWnd::SetRenderModelNIFRotate)
		.def(pkState, "SetRenderModelActorTargetAnimation", &lwUIWnd::SetRenderModelActorTargetAnimation)
		.def(pkState, "IsRenderModelActorAnimationDone", &lwUIWnd::IsRenderModelActorAnimationDone)
		.def(pkState, "ResetRenderModelNIFAnimation", &lwUIWnd::ResetRenderModelNIFAnimation)
		.def(pkState, "SetRenderModelEnableUpdate", &lwUIWnd::SetRenderModelEnableUpdate)
		.def(pkState, "UpdateOrthoZoom", &lwUIWnd::UpdateOrthoZoom)
		.def(pkState, "UpdateOrthoZoomOldVer", &lwUIWnd::UpdateOrthoZoomOldVer)
		.def(pkState, "SetCameraByName", &lwUIWnd::SetCameraByName)
		.def(pkState, "SetRecursiveCameraByName", &lwUIWnd::SetRecursiveCameraByName)		
		.def(pkState, "SetRenderModelActorEnableUpdate", &lwUIWnd::SetRenderModelActorEnableUpdate)	
		.def(pkState, "SetRenderModelNIFEnableUpdate", &lwUIWnd::SetRenderModelNIFEnableUpdate)

		// 해상도		
		//.def(pkState, "GetResolutionWidth", &lwUIWnd::GetResolutionWidth)
		//.def(pkState, "GetResolutionHeight", &lwUIWnd::GetResolutionHeight)
		.def(pkState, "GetResolutionSize", &lwUIWnd::GetResolutionSize)
		.def(pkState, "GetResolutionModeCount", &lwUIWnd::GetResolutionModeCount)	// 디바이스가 지원하는 해상도 개수
		.def(pkState, "GetResolutionGap", &lwUIWnd::GetResolutionGap)
		.def(pkState, "IsWideResolution", &lwUIWnd::IsWideResolution)
				
		//	List
		.def(pkState, "AddNewListItem", &lwUIWnd::AddNewListItem)
		.def(pkState, "AddNewListItemChar", &lwUIWnd::AddNewListItemChar)
		.def(pkState, "ClearAllListItem", &lwUIWnd::ClearAllListItem)
		.def(pkState, "GetListItemCount", &lwUIWnd::GetListItemCount)
		.def(pkState, "ListDeleteItem", &lwUIWnd::ListDeleteItem)
		.def(pkState, "ListFindItem", &lwUIWnd::ListFindItem)
		.def(pkState, "ListFindItemW", &lwUIWnd::ListFindItemW)
		.def(pkState, "ListFirstItem", &lwUIWnd::ListFirstItem)
		.def(pkState, "ListNextItem", &lwUIWnd::ListNextItem)
		.def(pkState, "ElementID", &lwUIWnd::ElementID)
		.def(pkState, "MoveMiddleBtnEndPos", &lwUIWnd::MoveMiddleBtnEndPos)
		.def(pkState, "AdjustMiddleBtnPos", &lwUIWnd::AdjustMiddleBtnPos)
		.def(pkState, "SetDisplayStartPos", &lwUIWnd::SetDisplayStartPos)

		//	Tree
		.def(pkState, "AddNewTreeItem", &lwUIWnd::AddNewTreeItem)
		.def(pkState, "AddNewTreeItemChar", &lwUIWnd::AddNewTreeItemChar)
		.def(pkState, "TreeFirstItem", &lwUIWnd::TreeFirstItem)
		.def(pkState, "TreeNextItem", &lwUIWnd::TreeNextItem)
		.def(pkState, "TreeDeleteItem", &lwUIWnd::TreeDeleteItem)
		.def(pkState, "ClearAllTreeItem", &lwUIWnd::ClearAllTreeItem)
		.def(pkState, "GetTreeItemCount", &lwUIWnd::GetTreeItemCount)
		.def(pkState, "IsExpandTree", &lwUIWnd::IsExpandTree)
		.def(pkState, "ExpandTree", &lwUIWnd::ExpandTree)
		.def(pkState, "CollapseTree", &lwUIWnd::CollapseTree)

		.def(pkState, "GetID", &lwUIWnd::GetID)
		.def(pkState, "SetID", &lwUIWnd::SetID)
		.def(pkState, "RefreshCalledTime", &lwUIWnd::RefreshCalledTime)
		.def(pkState, "RefreshLastTickTime", &lwUIWnd::RefreshLastTickTime)
		.def(pkState, "SetAliveTime", &lwUIWnd::SetAliveTime)
		.def(pkState, "GetAliveTime", &lwUIWnd::GetAliveTime)
		.def(pkState, "GetSize", &lwUIWnd::GetSize)
		.def(pkState, "SetSize", &lwUIWnd::SetSize)
		.def(pkState, "GetImgSize", &lwUIWnd::GetImgSize)
		.def(pkState, "SetImgSize", &lwUIWnd::SetImgSize)
	
		.def(pkState, "GetScale", &lwUIWnd::GetScale)
		.def(pkState, "SetScale", &lwUIWnd::SetScale)
		.def(pkState, "GetScaleCenter", &lwUIWnd::GetScaleCenter)
		.def(pkState, "SetScaleCenter", &lwUIWnd::SetScaleCenter)
	
		.def(pkState, "GetRotationDeg", &lwUIWnd::GetRotationDeg)
		.def(pkState, "SetRotationDeg", &lwUIWnd::SetRotationDeg)
		.def(pkState, "GetRotationCenter", &lwUIWnd::GetRotationCenter)
		.def(pkState, "SetRotationCenter", &lwUIWnd::SetRotationCenter)

		.def(pkState, "RemoveAllControls", &lwUIWnd::RemoveAllControls)
		.def(pkState, "SetCanDrag", &lwUIWnd::SetCanDrag)
		.def(pkState, "GetCanDrag", &lwUIWnd::GetCanDrag)
		//	Ani Bar
		.def(pkState, "SetStartTime", &lwUIWnd::SetStartTime)
		.def(pkState, "SetCloseTime", &lwUIWnd::SetCloseTime)
		.def(pkState, "DisplayTimeBar", &lwUIWnd::DisplayTimeBar)
		.def(pkState, "DisplayTimeBarMsg", &lwUIWnd::DisplayTimeBarMsg)
		.def(pkState, "DisplayPerBarMsg", &lwUIWnd::DisplayPerBarMsg)
	
		.def(pkState, "GetTotalLocation", &lwUIWnd::GetTotalLocation)
		.def(pkState, "ChangeImage", &lwUIWnd::ChangeImage)
		.def(pkState, "VOnCall", &lwUIWnd::VOnCall)
		.def(pkState, "IsMouseDown", &lwUIWnd::IsMouseDown)

		.def(pkState, "SetColor", &lwUIWnd::SetColor)

		.def(pkState, "SetGrayScale", &lwUIWnd::SetGrayScale)
		.def(pkState, "IsGrayScale", &lwUIWnd::IsGrayScale)
		.def(pkState, "GetSizeScale", &lwUIWnd::GetSizeScale)
		.def(pkState, "SetSizeScale", &lwUIWnd::SetSizeScale)

		// Only Button
		.def(pkState, "ButtonChangeImage", &lwUIWnd::ButtonChangeImage)
		.def(pkState, "SetClose", &lwUIWnd::SetClose)
		.def(pkState, "GetClose", &lwUIWnd::GetClose)

		//게이지 바 텍스트 출력
		.def(pkState, "SetPilotBarInfo", &lwUIWnd::SetPilotBarInfo)
		//Static Form
		.def(pkState, "SetStaticNum", &lwUIWnd::SetStaticNum)
		.def(pkState, "GetStaticNum", &lwUIWnd::GetStaticNum)

		//Scroll
		.def(pkState, "SetScrollCur", &lwUIWnd::SetScrollCur)
		.def(pkState, "GetScrollCur", &lwUIWnd::GetScrollCur)
		.def(pkState, "GetScrollMax", &lwUIWnd::GetScrollMax)

		.def(pkState, "SetTextPos", &lwUIWnd::SetTextPos)
		.def(pkState, "GetTextPos", &lwUIWnd::GetTextPos)
		.def(pkState, "GetTextSize", &lwUIWnd::GetTextSize)

		.def(pkState, "GetBuildIndex", &lwUIWnd::GetBuildIndex)
		.def(pkState, "SetBuildIndex", &lwUIWnd::SetBuildIndex)

		.def(pkState, "SetEnable", &lwUIWnd::SetEnable)
		.def(pkState, "GetEnable", &lwUIWnd::GetEnable)
		
		.def(pkState, "IsTwinkle", &lwUIWnd::IsTwinkle)
		.def(pkState, "OffTwinkle", &lwUIWnd::OffTwinkle)
		.def(pkState, "OffTwinkleSelf", &lwUIWnd::OffTwinkleSelf)
		.def(pkState, "NowTwinkleOn", &lwUIWnd::NowTwinkleOn)
		.def(pkState, "SetTwinkle", &lwUIWnd::SetTwinkle)
		.def(pkState, "ClearTwinkleInfo", &lwUIWnd::ClearTwinkleInfo)

		.def(pkState, "GetLiveTime", &lwUIWnd::GetLiveTime)
		.def(pkState, "RegistScript", &lwUIWnd::RegistScript)
		.def(pkState, "PtInUI", &lwUIWnd::PtInUI)
		.def(pkState, "FindTopParent", &lwUIWnd::FindTopParent)
		.def(pkState, "QuickSlotIcon_OnDisplay", &lwUIWnd::QuickSlotIcon_OnDisplay)
		.def(pkState, "SetIconGroup", &lwUIWnd::SetIconGroup)
		.def(pkState, "ConnectedInfo", &ConnectedInfo)

		//PgFormSnapedHeadWnd
		.def(pkState, "ReLocationSnapChild", &lwUIWnd::ReLocationSnapChild)

		//
		.def(pkState, "CallCashItemToolTip", &lwUIWnd::CallCashItemToolTip)

		//PgStepByStepWnd
		.def(pkState, "IsFinishedTextDraw", &lwUIWnd::IsFinishedTextDraw)
		.def(pkState, "SkipTDrawInterval", &lwUIWnd::SkipTDrawInterval)

		//PgFormRouletteWnd
		.def(pkState, "ClearRouletteInfo", &lwUIWnd::ClearRouletteInfo)
		.def(pkState, "SetRouletteStop", &lwUIWnd::SetRouletteStop)
		.def(pkState, "StopMissionResult", &lwUIWnd::StopMissionResult)		

		//PgMoveAnimateWnd
		.def(pkState, "InitMoveState", &lwUIWnd::InitMoveState)
		.def(pkState, "GetMovePercent", &lwUIWnd::GetMovePercent)
		.def(pkState, "IsMoveComplate", &lwUIWnd::IsMoveComplate)
		.def(pkState, "IsMovePause", &lwUIWnd::IsMovePause)
		.def(pkState, "SetModifyPoint1", &lwUIWnd::SetModifyPoint1)
		.def(pkState, "SetModifyPoint2", &lwUIWnd::SetModifyPoint2)
		.def(pkState, "SetStartPoint", &lwUIWnd::SetStartPoint)
		.def(pkState, "SetEndPoint", &lwUIWnd::SetEndPoint)
		.def(pkState, "GetModifyPoint1", &lwUIWnd::GetModifyPoint1)
		.def(pkState, "GetModifyPoint2", &lwUIWnd::GetModifyPoint2)
		.def(pkState, "GetStartPoint", &lwUIWnd::GetStartPoint)
		.def(pkState, "GetEndPoint", &lwUIWnd::GetEndPoint)
		.def(pkState, "SetMoveTime", &lwUIWnd::SetMoveTime)
		.def(pkState, "GetMoveTime", &lwUIWnd::GetMoveTime)
		.def(pkState, "SwapMovePoint", &lwUIWnd::lwSwapMovePoint)
		.def(pkState, "SetPauseTime", &lwUIWnd::lwSetPauseTime)
		.def(pkState, "GetPauseTime", &lwUIWnd::lwGetPauseTime)
		.def(pkState, "SetupMove", &lwUIWnd::lwSetupMove)
		

		.def(pkState, "SetState", &lwUIWnd::SetState)
		.def(pkState, "GetState", &lwUIWnd::GetState)

		//List2
		.def(pkState, "SetSelect", &lwUIWnd::SetSelect)
		.def(pkState, "GetSelectGuid", &lwUIWnd::GetSelectGuid)
		.def(pkState, "SetDisplayStartItem", &lwUIWnd::SetDisplayStartItem)

		.def(pkState, "GetBuildCount", &lwUIWnd::GetBuildCount)
		.def(pkState, "GetBuildNumberCount", &lwUIWnd::GetBuildNumberCount)
		.def(pkState, "GetBuildGab", &lwUIWnd::GetBuildGab)

		.def(pkState, "IsClickLock", &lwUIWnd::IsClickLock)
		.def(pkState, "SetClickLock", &lwUIWnd::SetClickLock)
		
		//ON_TICK
		.def(pkState, "SetTickInterval", &lwUIWnd::SetTickInterval)
		.def(pkState, "GetTickInterval", &lwUIWnd::GetTickInterval)

		.def(pkState, "SetModal", &lwUIWnd::SetModal)
		.def(pkState, "GetDefaultImgName", &lwUIWnd::GetDefaultImgName)
		;
	def(pkState, "GlobalLBtnUp", lwGlobalLBtnUp);
	def(pkState, "GetScreenSize", lwGetScreenSize);
	def(pkState, "GetDefaultLocation", lwGetDefaultLocation);
	def(pkState, "GetCursorPos", lwGetCursorPos);
	def(pkState, "ActivateUI", lwActivateUI);
	def(pkState, "ActivateUIPriority", lwActivateUIPriority);
	def(pkState, "PickWnd", lwPickWnd);
	def(pkState, "AddChildUI", lwAddChildUI);
	def(pkState, "CheckCalledUniqueUIType", lwCheckCalledUniqueUIType);
	def(pkState, "IsExistUniqueType", lwIsExistUniqueType);
	def(pkState, "CallUI", lwCallUI);
	def(pkState, "CloneAutoCallUI", lwCloneAutoCallUI);
	def(pkState, "CreateControl", lwCreateControl);

//	def(pkState, "CallToolTip", lwCallToolTipByNo);
	def(pkState, "CallToolTipText", lwCallToolTipByText);
	def(pkState, "CallMutableToolTipText", lwCallMutableToolTipByText);
	def(pkState, "CallToolTipStatus", lwCallToolTipByStatus);
	
	def(pkState, "CloseUI", lwCloseUI);
	def(pkState, "CloseAllUI", lwCloseAllUI);
	def(pkState, "ReleaseAllControl", lwReleaseAllControl);

	def(pkState, "QuickInvDoAction", lwQuickInvDoAction);
	def(pkState, "DoQuickUseItem", lwDoQuickUseItem);

	def(pkState, "TryLogin", lwTryLogin);
	def(pkState, "GetAccountID", lwGetAccountID);
	def(pkState, "ClearAutoLogin", lwClearAutoLogin);
	def(pkState, "SetEditFocus", lwSetEditFocus);
	def(pkState, "SendScriptEvent", lwSendScriptEvent);
	def(pkState, "ClearEditFocus", lwClearEditFocus);
	def(pkState, "ExitApp", ExitApp);
	def(pkState, "RecvOtherItemChange", lwRecvOtherItemChange);
	def(pkState, "ChangeMapInfo", lwChangeMapInfo);
	def(pkState, "RecvPickupItem", lwRecvPickupItem);
	def(pkState, "SetDisConnectFlush", lwSetDisConnectFlush);

	//아이콘 집기가 되어야 하는데.
	def(pkState, "SetLastAttackTarget", lwSetLastAttackTarget);
	def(pkState, "SetCoolTime", lwSetCoolTime);
	def(pkState, "ExitClient", lwExitClient);

	def(pkState, "AddWarnData", lwAddWarnData);
	def(pkState, "AddWarnDataTT", lwAddWarnDataTT);

	def(pkState, "AddWarnDataStr", lwAddWarnDataStr);
	
	def(pkState, "GetPetInfo", lwGetPetInfo);
	//def(pkState, "TempOhterSet", lwTempOhterSet);
	//def(pkState, "NpcSet", lwNpcSet);

	def(pkState, "CallMessageBox", lwCallMessageBox);
	def(pkState, "CallInputBox", lwCallInputBox);
	def(pkState, "CallYesNoMsgBox", lwCallYesNoMsgBox);
	def(pkState, "CallCommonMsgYesNoBox", &lwCallCommonMsgYesNoBox);
	def(pkState, "CallPetUI", lwCallPetUI);
	//def(pkState, "CallCharUI", lwCallCharUI);
	def(pkState, "EnableUIOffscreenRendering", lwEnableUIOffscreenRendering);

	def(pkState, "DisplayPetText", lwDisplayPetText);
	def(pkState, "IdlePetText", lwIdlePetText);
	
	def(pkState, "TempQuestTest", lwTempQuestTest);
	def(pkState, "BossHPBar", lwBossHPBar);
	

	def(pkState, "SetBossGUIDForHPBar", lwSetBossGUIDForHPBar);
	def(pkState, "DisplayBossHPBar", lwDisplayBossHPBar);

	def(pkState, "CloseToolTip", lwCloseToolTip);

	def(pkState, "UIOff", lwUIOff);
	def(pkState, "UIOn", lwUIOn);
	def(pkState, "BlockGlobalHotKey", lwBlockGlobalHotKey);
	def(pkState, "IsVisible", lwIsVisible);
	def(pkState, "GetUIWnd", lwGetUIWnd);
	def(pkState, "IsUIWnd", lwIsUIWnd);

	def(pkState, "CompNum", lwCompNum);
	def(pkState, "ExitShop", lwExitShop);

	//버프 표시
	def(pkState, "SetBuffWnd", lwSetBuffWnd);
	//def(pkState, "AddBuff", lwAddBuff);
	def(pkState, "DeleteBuff", lwDeleteBuff);
	def(pkState, "UpdateBuffWnd", lwUpdateBuffWnd);
	def(pkState, "ChangeShineStoneCount", lwChangeShineStoneCount);
	def(pkState, "GetBuffEffectNo", lwGetBuffEffectNo);	

	def(pkState, "GetTT", lwGetTT);
	def(pkState, "KeyWasReleased", lwKeyWasReleased);
	def(pkState, "KeyIsDown", lwKeyIsDown);
	def(pkState, "ReleaseAllKey", lwReleaseAllKey);

	def(pkState, "GetDropRes", lwGetDropRes);
	def(pkState, "GetItemResData", lwGetItemResData);
	def(pkState, "RegUIColor", lwRegUIColor);
	def(pkState, "GetUIColor", lwGetUIColor);
	def(pkState, "ChangeUIColorSet", lwChangeUIColorSet);

	def(pkState, "DisconnectLoginServer", lwDisconnectLoginServer);
	def(pkState, "GetCurrentChennelName", lwGetCurrentChennelName);

	def(pkState, "GetDefString", lwGetDefString);
	
	//>>Option
	def(pkState, "Config_Save", lwConfig_Save);//모든 Saved 설정을 로컬 Config.xml 파일에 저장합니다.
	def(pkState, "Config_DefaultConfig", lwConfig_DefaultConfig);//Current Config설정 부분을 Default설정으로 돌립니다. (저장 전)
	def(pkState, "Config_DefaultKeySet", lwConfig_DefaultKeySet);//Current Keyset설정 부분을 Default설정으로 돌립니다. (저장 전)
	def(pkState, "Config_GetDefaultValue", lwConfig_GetDefaultValue);//char const *szHeadKey, char const *szKey 로 int 형 Default config 값을 얻습니다.
	def(pkState, "Config_GetDefaultText", lwConfig_GetDefaultText);//char const *szHeadKey, char const *szKey 로 char* 형 Default config 값을 얻습니다.
	def(pkState, "Config_ApplyConfig", lwConfig_ApplyConfig);//Config Current 설정을 Saved 설정에 저장합니다.
	def(pkState, "Config_ApplyKeySet", lwConfig_ApplyKeySet);//Keyset Current 설정을 Saved 설정에 저장합니다.
	def(pkState, "Config_ApplyGraphics", lwConfig_ApplyGraphics);//Keyset Current 설정을 Saved 설정에 저장합니다.
	def(pkState, "Config_CancelConfig", lwConfig_CancelConfig);//Config 설정을 Saved 상태로 돌립니다.
	def(pkState, "Config_CancelKeySet", lwConfig_CancelKeySet);//Keyset 설정을 Saved 상태로 돌립니다.
	def(pkState, "Config_GetValue", lwConfig_GetValue);//char const *szHeadKey, char const *szKey 로 int 형 config 값을 얻습니다.
	def(pkState, "Config_GetText", lwConfig_GetText);//char* 분류, char* 키 로 char* 형 config 값을 얻습니다.
	def(pkState, "Config_SetValue", lwConfig_SetValue);//int 형 config 값을 설정합니다.
	def(pkState, "Config_SetText", lwConfig_SetText);//char* 형 config 값을 설정합니다.
	def(pkState, "SysConfig_SetValue", lwSysConfig_SetValue);//int 형 Saved config 값을 설정합니다.(주의!! 시스템 사용만 권장)
	def(pkState, "SysConfig_SetText", lwSysConfig_SetText);//char* 형 Saved config 값을 설정합니다.(주의!! 시스템 사용만 권장)
	def(pkState, "UpdateEtcConfig", lwUpdateEtcConfig);
	def(pkState, "UseLevelRank", &lwUseLevelRank);
	//<<Option

	//>>KeySet
	def(pkState, "UKeyToKey", lwUKeyToKey);
	//<<KeySet

	//>>
	def(pkState, "GetMapNameW", lwGetMapNameW);//맵 이름을 얻습니다. return lwWstring
	def(pkState, "GetMapKey", lwGetMapKey);//맵 키를 얻습니다.
	def(pkState, "GetMapAttrByNo", lwGetMapAttrByNo);
	//<<

	def(pkState, "RecommendMissionQuestCount", &lwRecommendMissionQuestCount);
	def(pkState, "RecommendMissionAchievementCount", &lwRecommendMissionAchievementCount);
	def(pkState, "GetMissionRequiredItemNo", &lwGetMissionRequiredItemNo);
	def(pkState, "IsEventMission", &lwGetEventMission);
	def(pkState, "MissionUISetQuest", &lwMissionUISetQuest); // 미션UI에 보유 퀘스트 표시
	def(pkState, "Recv_PT_M_C_ANS_MISSION_INFO", &lwRecv_PT_M_C_ANS_MISSION_INFO);
	def(pkState, "IsAutoLevel", &lwIsAutoLevel);	
	def(pkState, "IsHaveArcadeMode", &lwIsHaveArcadeMode);
	def(pkState, "SetMissionGray", &lwSetMissionGray);	
	def(pkState, "GetMissionMapCount", &lwGetMissionMapCount);
	def(pkState, "GetMissionDifficultyLevel", &lwGetMissionDifficultyLevel);
	def(pkState, "GetMissionNo", &lwGetMissionNo);
	def(pkState, "SetMissionNo", &lwSetMissionNo);
	def(pkState, "GetDefence7MinLevel", &lwGetDefence7MinLevel);
	def(pkState, "GetDefence7MaxLevel", &lwGetDefence7MaxLevel);
	def(pkState, "Defence7EnterLevelCheck", &lwDefence7EnterLevelCheck);
	def(pkState, "IsMissionComplatedQuest", &lwIsMissionComplatedQuest);
	def(pkState, "MissionClearQuestText", &lwMissionClearQuestText);
	def(pkState, "IsMissionLastStage", &lwIsMissionLastStage);
    def(pkState, "IsOpenDefenseModeTable", &lwIsOpenDefenseModeTable);
	def(pkState, "GetMissionImgPath", &lwGetMissionImgPath);
	def(pkState, "HiddenRewordItemView", &lwHiddenRewordItemView);
	def(pkState, "HiddenRewordItemView2", &lwHiddenRewordItemView2);
	def(pkState, "HiddenRewordItemViewIcon", &lwHiddenRewordItemViewIcon);	
	def(pkState, "GetMissionKey", &lwGetMissionKey);
	def(pkState, "SendMemo", &lwSendMemo);
	def(pkState, "SendTakeCoupon", &lwSendTakeCoupon);
	def(pkState, "SendReqRankInfo", &lwSendReqRankInfo);
	def(pkState, "MissionRankingRewardItem", &lwMissionRankingRewardItem);	
	def(pkState, "SetMissionCountMap", &lwSetMissionCountMap);
	def(pkState, "SetMissionDifficultyLevel", &lwSetMissionDifficultyLevel);
	def(pkState, "SetScenarioMissionMapCount", &lwSetScenarioMissionMapCount);
	def(pkState, "CalcMissionCount", &lwCalcMissionCount);
	def(pkState, "SetMissionSelectMapName", &lwSetMissionSelectMapName);
	def(pkState, "Send_PT_C_N_REQ_MISSION_RESTART", &lwSend_PT_C_N_REQ_MISSION_RESTART);
	def(pkState, "CallMissionRetryUI", &lwCallMissionRetryUI);
	def(pkState, "CallMissionRebirthUI", &lwCallMissionRebirthUI);
	def(pkState, "Send_REQ_DEFENCE_INFALLIBLE_SELECTION", &lwSend_REQ_DEFENCE_INFALLIBLE_SELECTION);
	def(pkState, "DisplayIcon_InfallibleSelection", &lwDisplayIcon_InfallibleSelection);
	def(pkState, "Enter_DefenceMission", &lwEnter_DefenceMission);
	def(pkState, "Set_DefenceSelectUI", &lwSet_DefenceSelectUI);
	def(pkState, "GetDefenceResultStage", &lwGetDefenceResultStage);
	def(pkState, "DisplayIcon_DefenceSelectUI", &lwDisplayIcon_DefenceSelectUI);
	def(pkState, "SetMission_Defence_Hp", &lwSetMission_Defence_Hp);
	def(pkState, "GetDefence7RelayItem", &lwGetDefence7RelayItem);
	def(pkState, "GetDefence7PointCopyItem", &lwGetDefence7PointCopyItem);
	def(pkState, "GetDefence7RelayPoint", &lwGetDefence7_RelayPoint);
	def(pkState, "CallRankInfo", &lwCallRankInfo);	
	def(pkState, "CallRankTop", &lwCallRankTop);		
	def(pkState, "CallDefenceMissionReward", &lwCallDefenceMissionReward);
	def(pkState, "UpdateDefenceMissionGuardian", &lwUpdateDefenceMissionGuardian);
	def(pkState, "UpdateDefenceMissionSkill", &lwUpdateDefenceMissionSkill);
	def(pkState, "StrategyDefenceResultUI", &lwStrategyDefenceResultUI);
	def(pkState, "DisplayStrategicPoint", &lwDisplayStrategicPoint);	
	def(pkState, "StrategyItemDoAction", &lwStrategyItemDoAction);
	def(pkState, "MissionMgrInit", &lwMissionMgrInit);
	def(pkState, "GetDefenceGuardianNeedPos", &lwGetDefenceGuardianNeedPos);
	def(pkState, "ViewGuardianDetectRangeEffect", &lwViewGuardianDetectRangeEffect);
	def(pkState, "ClearGuardianDetectRangeEffect", &lwClearGuardianDetectRangeEffect);
	def(pkState, "OnDefenceGuardianMouseOver", &lwOnDefenceGuardianMouseOver);
	def(pkState, "OnDefenceGuardianMouseOut", &lwOnDefenceGuardianMouseOut);
	def(pkState, "IsGuardianInstall", &lwIsGuardianInstall);
	def(pkState, "IsGuardianInstallStr", &lwIsGuardianInstallStr);
	def(pkState, "SetGuardianInstallLoc", &lwSetGuardianInstallLoc);
	def(pkState, "CallGuardianUI", &lwCallGuardianUI);	
	def(pkState, "CallInstallGuardian", &lwCallInstallGuardian);
	def(pkState, "CallUpgradeGuardian", &lwCallUpgradeGuardian);
	def(pkState, "CallRemoveGuardian", &lwCallRemoveGuardian);
	def(pkState, "CallDefenceSkillLearn", &lwCallDefenceSkillLearn);
	def(pkState, "OnCallDefenceLearnSkill", &lwOnCallDefenceLearnSkill);
	def(pkState, "OnEnterGuardianInstallTrigger", &lwOnEnterGuardianInstallTrigger);
	def(pkState, "OnLeaveGuardianInstallTrigger", &lwOnLeaveGuardianInstallTrigger);
	def(pkState, "CheckScenarioMissionEnter", &CheckScenarioMissionEnter);
	def(pkState, "CheckHaveMissionEnterItem", &lwCheckHaveMissionEnterItem);

	def(pkState, "CallQuickSkillToolTip", &CallQuickSkillToolTip);
	
	def(pkState, "ChangeInvViewGroup", &lwChangeInvViewGroup);
	def(pkState, "SetInvViewSize", &lwSetInvViewSize);
	def(pkState, "ExtendInvSafeSlot", &lwExtendInvSafeSlot);
	
	def(pkState, "CashShopInvViewSize", &lwCashShopInvViewSize);
	
	def(pkState, "ChangeShopViewGroup", &lwChangeShopViewGroup);
	def(pkState, "ChangeShopViewSubOrder", &lwChangeShopViewSubOrder);
	def(pkState, "CalcValue", &lwCalcValue);
	def(pkState, "SetCalcDigit", &lwSetCalcDigit);
	def(pkState, "DoCalcuratorAction", &lwDoCalcuratorAction);
	def(pkState, "DoSMSAction", &lwDoSMSAction);
	
    def(pkState, "CallEmporiaShop", &lwCallEmporiaShop);
    def(pkState, "DoBuyEmporiaShop", &lwDoBuyEmporiaShop);

	def(pkState, "CallJobSkillShop", &lwCallJobSkillShop);
	def(pkState, "SetFilter_JobSkillShop", &lwSetFilter_JobSkillShop);
	def(pkState, "SelectFilter_JobSkillShop", &lwSelectFilter_JobSkillShop);
	def(pkState, "LearnFilter_JobSkillShop", &lwLearnFilter_JobSkillShop);

	def(pkState, "GetInvViewGroup", &lwGetInvViewGroup);

	def(pkState, "DrawMissionRewardItem", &lwDrawMissionRewardItem);
	def(pkState, "OnCallMissionRewardItemToolTip", &lwOnCallMissionRewardItemToolTip);

	def(pkState, "CMP_STATE_CHANGE", &lwCMP_STATE_CHANGE);
	def(pkState, "Get_CMP_STATE", &lwGet_CMP_STATE);
	def(pkState, "SendSpendMoneyAction", &lwSendSpendMoneyAction);
	def(pkState, "AddNewScreenBreak", &lwAddNewScreenBreak);

	def(pkState, "ChangeQuickSlotViewPage", &lwChangeQuickSlotViewPage);
	def(pkState, "GetQuickSlotViewPage", &lwGetQuickSlotViewPage);

	//DrawModel
	def(pkState, "DrawModel", &lwDrawModel);
	
	//Sound
	def(pkState, "SetBGMVolume", &lwSetBGMVolume);
	def(pkState, "SetEffectVolume", &lwSetEffectVolume);

	def(pkState, "CannotConnectLogin", &lwCannotConnectLogin);
	
	//Trade
	def(pkState, "NewTradeStart", &lwNewTradeStart);
	def(pkState, "OnTradeConfirmButton", &lwOnTradeConfirmButton);
	def(pkState, "GetTradeGuid", &lwGetTradeGuid);
	def(pkState, "SetTradeGuid", &lwSetTradeGuid);
	def(pkState, "ClearTradeInfo", &lwClearTradeInfo);

	// Safe Inven
	def(pkState, "CallInvSafeUI", &PgSafeInventory::CallInvSafeUI);
	def(pkState, "CloseInvSafeUI", &PgSafeInventory::CloseInvSafeUI);
	def(pkState, "SendCloseSafe", &PgSafeInventory::SendCloseSafe);

	// share safe inven
	def(pkState, "CallInvShareSafeUI", &PgSafeInventory::CallInvShareSafeUI);
	def(pkState, "CloseInvShareSafeUI", &PgSafeInventory::CloseInvShareSafeUI);
	def(pkState, "SendCloseShareSafe", &PgSafeInventory::SendCloseShareSafe);

	// Loading Image
	def(pkState, "LoadingImage", &lwLoadingImage);

	def(pkState, "ViewOtherEquip", &lwViewOtherEquip);
	def(pkState, "ViewPetEquip", &lwViewPetEquip);
	def(pkState, "RegistUIAction", &lwRegistUIAction);

	def(pkState, "RefreshRepairAlarmUI", &lwRefreshRepairAlarmUI);

	def(pkState, "GetCoinChangerResNo", &lwGetCoinChangerResNo);
	def(pkState, "SelectCoinItem", &lwSelectCoinItem);
	def(pkState, "SendBuyCoinItem", &lwSendBuyCoinItem);
	def(pkState, "SendCoinMaking", &lwSendCoinMaking);

	def(pkState, "GetReqInvWearableUpdate", &lwGetReqInvWearableUpdate);
	def(pkState, "SetReqInvWearableUpdate", &lwSetReqInvWearableUpdate);

	def(pkState, "IsOpenGroundByNo", &lwIsOpenGroundByNo);
	def(pkState, "CallMapInfo", &lwCallMapInfo);

	def(pkState, "GetAlphaFromImgAtCursor", &lwGetAlphaFromImgAtCursor);
	def(pkState, "HideUI", &lwHideUI);
	def(pkState, "HidePartUI", &lwHidePartUI);

	//MainUI
	def(pkState, "DrawPlayerLv", &lwDrawPlayerLv);
	def(pkState, "ShowMiniMap", &lwShowMiniMap);

	////MissionQuest
	//def(pkState, "MQCard_Select", &lwMQCard_Select);
	//def(pkState, "MQCard_TimeText", &lwMQCard_TimeText);
	//def(pkState, "MQCard_QuestInfo", &lwMQCard_QuestInfo);
	//def(pkState, "MQCard_Update", &lwMQCard_Update);
	//def(pkState, "MQCard_ForwardID", &lwMQCard_ForwardID);
	//def(pkState, "MQCard_Scale", &lwMQCard_Scale);

	//우편
	def(pkState, "CanAddToMail", &lwCanAddToMail);
	def(pkState, "CategoryBtnInit", &lwCategoryBtnInit);
	def(pkState, "REQ_MAIL_SEND", &lwREQ_MAIL_SEND);
	def(pkState, "REQ_MAIL_RECV", &lwREQ_MAIL_RECV);
	def(pkState, "InitMailUI", &lwInitMailUI);
	def(pkState, "GetNowMailCount", &lwGetNowMailCount);
	def(pkState, "PrevMailPage", &lwPrevMailPage);
	def(pkState, "NextMailPage", &lwNextMailPage);
	def(pkState, "MakeMailList", &lwMakeMailList);
	def(pkState, "OpenMail", &lwOpenMail);
	def(pkState, "DeleteMail", &lwDeleteMail);
	def(pkState, "DisplayMailSrcIcon", &lwDisplayMailSrcIcon);
	def(pkState, "GetAnnex", &lwGetAnnex);
	def(pkState, "GetAnnexAll", &lwGetAnnexAll);
	def(pkState, "DrawItemInPost", &lwDrawItemInPost);
	def(pkState, "OnCallItemInPostToolTip", &lwOnCallItemInPostToolTip);
//	def(pkState, "ReturnMail", &lwReturnMail);
//	def(pkState, "ReturnMailAt", &lwReturnMailAt);
	def(pkState, "ReplyMail", &lwReplyMail);
	def(pkState, "CheckMassItem", &lwCheckMassItem);
	def(pkState, "REQ_MASSMAIL_SEND", &lwREQ_MASSMAIL_SEND);
	def(pkState, "CheckedMailCount", &lwCheckedMailCount);
	def(pkState, "CallPostUI", &CallPostUI);
	def(pkState, "ClosePostUI", &ClosePostUI);
	def(pkState, "ClearPostSendItem", &lwClearPostSendItem);

	def(pkState, "SetMissionLevelingText", &lwSetMissionLevelingText);
	def(pkState, "GetMissionLevelingText", &lwGetMissionLevelingText);	
	def(pkState, "GetEnterDefenceMinLevel", &lwGetEnterDefenceMinLevel);
	def(pkState, "GetEnterDefenceMaxLevel", &lwGetEnterDefenceMaxLevel);
	def(pkState, "GetMissionLevelArea", &lwGetMissionLevelArea);

	def(pkState, "ItemSealProcessIconDraw", &lwItemSealProcessIconDraw);
	def(pkState, "ItemSealProcessIconToolTip", &lwItemSealProcessIconToolTip);
	def(pkState, "ItemSealProcessListSelect", &lwItemSealProcessListSelect);
	def(pkState, "ItemSealProcessSendPacket", &lwItemSealProcessSendPacket);
	def(pkState, "ItemSealProcessStop", &lwItemSealProcessStop);
	def(pkState, "ItemSealProcessWatchingDelayedPacket", &lwItemSealProcessWatchingDelayedPacket);	
	def(pkState, "ItemSealProcessSelectAll", &lwItemSealProcessSelectAll);
	
	def(pkState, "FranYesNoMsgBox", &lwFranYesNoMsgBox);
	def(pkState, "ItemSelfProcessIconToolTip", &lwItemSelfProcessIconToolTip);

	def(pkState, "DrawMonsterKillUI", &lwDrawMonsterKillUI);

	def(pkState, "UpdateMissionScoreUI", &lwUpdateMissionScoreUI);
	def(pkState, "DrawMissionScoreUI", &lwDrawMissionScoreUI);
	def(pkState, "MissionScoreTypeClear", &lwMissionScoreTypeClear);	
	def(pkState, "UpdateMissionScoreStart", &lwUpdateMissionScoreStart);	

	//>>	1:1
	def(pkState, "MToMChatClose", &lwMToMChatClose);
	def(pkState, "MToMChatDlgStateChange", &lwMToMChatDlgStateChange);
	def(pkState, "MToMMiniFlash", &lwMToMMiniFlash);
	def(pkState, "MToMRefreshUI", &lwMToMRefreshUI);
	def(pkState, "MToMChatNewDlg", &lwMToMChatNewDlg);
	def(pkState, "MToMSendChatLog", &lwMToMSendChatLog);
	def(pkState, "MToMMiniToolTip", &lwMToMMiniToolTip);
	def(pkState, "MToMMapMoveReCheck", &lwMToMMapMoveReCheck);
	//<<	1:1

	def(pkState, "CallChatWindow", &lwCallChatWindow);
	def(pkState, "ReCallQuickSlotEx", &lwReCallQuickSlotEx);
	def(pkState, "CallQuickSlotEx", &lwCallQuickSlotEx);

	
	def(pkState, "RecvCouponWord", &lwRecvCouponWord);	//이벤트 쿠폰
	def(pkState, "SendEventCoupon", &lwSendEventCoupon);	//이벤트 쿠폰
	def(pkState, "SendEventNameCoupon", &lwSendEventNameCoupon);	//특정인 쿠폰

	def(pkState, "SetLocalName", &lwSetLocalName);
	def(pkState, "GetLocalName", &lwGetLocalName);

	def(pkState, "PreCheckItemMaking", &SoulCraft::PreCheckItemMaking);
	def(pkState, "UpdateTradeUnselingScroll", &SoulCraft::UpdateTradeUnselingScroll);
	def(pkState, "SendTradeUnsealingScroll", &SoulCraft::SendTradeUnsealingScroll);
	def(pkState, "GetCanMaxTradeMaking", &SoulCraft::GetCanMaxTradeMaking);
	def(pkState, "SetSimpleItemIcon", &SoulCraft::SetSimpleItemIcon);
	def(pkState, "CallSimpleItemToolTip", &SoulCraft::CallSimpleItemToolTip);

	def(pkState, "AddSoulStoneTradeItem", &SoulStoneTrade::AddSoulStoneTradeItem);
	def(pkState, "UpdateSoulStoneTrade", &SoulStoneTrade::UpdateSoulStoneTrade);
	def(pkState, "SetSoulStoneTradeLocation", &SoulStoneTrade::SetSoulStoneTradeLocation);
	def(pkState, "IncSoulStoneTradeItemWnd", &SoulStoneTrade::IncSoulStoneTradeItemWnd);
	def(pkState, "CallSoulStoneTradeToolTip", &SoulStoneTrade::CallSoulStoneTradeToolTip);
	def(pkState, "SendSoulStoneTrade", &SoulStoneTrade::SendSoulStoneTrade);

	def(pkState, "AddItem_ConvertItem", &Item_Convert::AddItem_ConvertItem);
	def(pkState, "UpdateItem_Convert", &Item_Convert::UpdateItem_Convert);
	def(pkState, "SetItem_ConvertLocation", &Item_Convert::SetItem_ConvertLocation);
	def(pkState, "IncItem_ConvertItemWnd", &Item_Convert::IncItem_ConvertItemWnd);
	def(pkState, "CallItem_ConvertToolTip", &Item_Convert::CallItem_ConvertToolTip);
	def(pkState, "SendItem_Convert", &Item_Convert::SendItem_Convert);

	def(pkState, "ChangeCashBoxViewGroup", &lwChangeCashBoxViewGroup);
	def(pkState, "ChangeGuildInvViewGroup", &lwChangeGuildInvViewGroup);
	def(pkState, "SetGuildInvViewSize", &lwSetInvViewSize);

	def(pkState, "ExpandCashBox", &lwExpandCashBox);
	def(pkState, "DrawInvRemainTime", &lwDrawInvRemainTime);

	def(pkState, "GetImgSizeFromPath", &lwGetImgSizeFromPath);
	def(pkState, "GetMiniMapPath", &lwGetMiniMapPath);
	def(pkState, "GetMapNoFromNpcName", &lwGetMapNoFromNpcName);

	def(pkState, "PlayMovie", &lwPlayMovie);
	def(pkState, "IsPlayMovie", &lwIsPlayMovie);

	def(pkState, "DoQuickUseItem", &UIItemUtil::DoQuickUseItem);

	def(pkState, "CopyPlayerNameToClipBoard", &lwCopyPlayerNameToClipBoard);
	def(pkState, "CSNotifyCheck", &lwCSNotifyCheck);
	
	def(pkState, "ParticleTextureChange", &lwParticleTextureChange);

	def(pkState, "SendOrderTimer", &UIMacroCheck::lwUpdateMacroTimer);
	def(pkState, "SendOrderPassward", &UIMacroCheck::lwSendMacroPassword);
	def(pkState, "CloseOrderUI", &UIMacroCheck::lwCloseMacroUI);
	def(pkState, "IsOnMacroCheck", &UIMacroCheck::lwIsOnMacroCheck);
	def(pkState, "CallMacroCheckUI_IfNotComplete", &UIMacroCheck::lwCallMacroCheckUI_IfNotComplete);
	
	def(pkState, "DisPlayMissionUpdateScore", &lwDisPlayMissionUpdateScore);
	def(pkState, "OnSelectGadaCoinBox", &lwOnSelectGadaCoinBox);	
	def(pkState, "GetMissionType", &lwGetMissionType);	
	def(pkState, "GetMissionPlayCount", &lwGetMissionPlayCount);
	def(pkState, "GetMissionExpUI", &lwGetMissionExpUI);	

	def(pkState, "GetExtendSlotView", &lwGetExtendSlotView);
	def(pkState, "SetExtendSlotView", &lwSetExtendSlotView);
	
	//locale값 전역변수 대신 사용
	lua_tinker::table LOCALE(pkState, "LOCALE");
	LOCALE.set("NC_NOT_SET", LOCAL_MGR::NC_NOT_SET);
	LOCALE.set("NC_KOREA",  LOCAL_MGR::NC_KOREA);
	LOCALE.set("NC_CHINA",  LOCAL_MGR::NC_CHINA);
	LOCALE.set("NC_TAIWAN",  LOCAL_MGR::NC_TAIWAN);
	LOCALE.set("NC_THAILAND",  LOCAL_MGR::NC_THAILAND);
	LOCALE.set("NC_HONGKONG",  LOCAL_MGR::NC_HONGKONG);
	LOCALE.set("NC_USA",  LOCAL_MGR::NC_USA);
	LOCALE.set("NC_JAPAN",  LOCAL_MGR::NC_JAPAN);
	LOCALE.set("NC_EU",  LOCAL_MGR::NC_EU);
	LOCALE.set("NC_SINGAPORE",  LOCAL_MGR::NC_SINGAPORE);	
	LOCALE.set("NC_FRANCE",  LOCAL_MGR::NC_FRANCE);
	LOCALE.set("NC_GERMANY",  LOCAL_MGR::NC_GERMANY);	
	LOCALE.set("NC_MALAYSIA",  LOCAL_MGR::NC_MALAYSIA);
	LOCALE.set("NC_PHILIPPINES",  LOCAL_MGR::NC_PHILIPPINES);
	LOCALE.set("NC_AUSTRALIA",  LOCAL_MGR::NC_AUSTRALIA);
	LOCALE.set("NC_INDONESIA",  LOCAL_MGR::NC_INDONESIA);
	LOCALE.set("NC_VIETNAM",  LOCAL_MGR::NC_VIETNAM);
	LOCALE.set("NC_NEW_ZEALAND",  LOCAL_MGR::NC_NEW_ZEALAND);
	LOCALE.set("NC_LAOS",  LOCAL_MGR::NC_LAOS);
	LOCALE.set("NC_CAMBODIA",  LOCAL_MGR::NC_CAMBODIA);
	LOCALE.set("NC_MACAO",  LOCAL_MGR::NC_MACAO);
	LOCALE.set("NC_CANADA",  LOCAL_MGR::NC_CANADA);
	LOCALE.set("NC_MEXICO",  LOCAL_MGR::NC_MEXICO);
	LOCALE.set("NC_DEVELOP",  LOCAL_MGR::NC_DEVELOP);
	LOCALE.set("NC_ENGLAND",  LOCAL_MGR::NC_ENGLAND);	
	LOCALE.set("NC_RUSSIA",  LOCAL_MGR::NC_RUSSIA);

	def(pkState, "GetLocale", &lwGetLocale);
	def(pkState, "GetLocalTime", &lwGetLocalTime);

	def(pkState, "SendReqGemStore", &UIGemStore::lwSendReqGemStore);	
	def(pkState, "SendReqDefGemStore", &UIGemStore::lwSendReqDefGemStore);	
	def(pkState, "IsNotEmptyDefGemStore", &UIGemStore::lwIsNotEmptyDefGemStore);
	def(pkState, "ClickGemStoreListItem", &UIGemStore::lwClickGemStoreListItem);	
	def(pkState, "ClearGemStoreData", &UIGemStore::lwClearGemStoreData);
	def(pkState, "GemStoreDrawIconImage", &UIGemStore::lwGemStoreDrawIconImage);
	def(pkState, "GemStoreDrawIconTooltip", &UIGemStore::lwGemStoreDrawIconTooltip);
	def(pkState, "SendReqGemTrade", &UIGemStore::lwSendReqGemTrade);
	def(pkState, "SendReqGemTradeTree", &UIGemStore::lwSendReqGemTradeTree);
	def(pkState, "SetGemStoreItemTree", &UIGemStore::SetItemTree);
	def(pkState, "ItemTreeClear", &UIGemStore::ItemTreeClear);
	def(pkState, "IsGemStoreItemTree", &UIGemStore::IsItemTreeSlot);

	def(pkState, "OnClickGemStoreBeginPage", &UIGemStore::OnClickGemStoreBeginPage);
	def(pkState, "OnClickGemStoreEndPage", &UIGemStore::OnClickGemStoreEndPage);
	def(pkState, "OnClickGemStoreJumpPrevPage", &UIGemStore::OnClickGemStoreJumpPrevPage);
	def(pkState, "OnClickGemStoreJumpNextPage", &UIGemStore::OnClickGemStoreJumpNextPage);
	def(pkState, "OnClickGemStorePage", &UIGemStore::OnClickGemStorePage);
	def(pkState, "GetGemStoreTitleMenu", &UIGemStore::lwGetGemStoreTitleMenu);

	def(pkState, "SetUISizeForTextY", &lwSetUISizeForTextY);

	def(pkState, "GetKeynoToName", &lwGetKeynoToName);		
	def(pkState, "OpenURL", &lwOpenURL);		
	def(pkState, "IgnoreChannelUI", &lwIgnoreChannelUI);

	def(pkState, "OnClickCustomTypeItem", &UIItemUtil::OnClickCustomTypeItem);
	def(pkState, "OnDrawCustomTypeItemIcon", &UIItemUtil::OnDrawCustomTypeItemIcon);
	def(pkState, "OnCallCustomItemTooltip", &UIItemUtil::OnCallCustomItemTooltip);
	def(pkState, "OnOKUseCustomTypeItem", &UIItemUtil::OnOKUseCustomTypeItem);
	def(pkState, "SendEmotion", &SendEmotion);
	
	def(pkState, "RemoveBuffEffect", &lwRemoveBuffEffect);
	def(pkState, "SetBlockNickFilterProcessType", &SetBlockNickFilterProcessType);
	def(pkState, "GetBlockNickFilterProcessType", &GetBlockNickFilterProcessType);
	def(pkState, "SetBadWordFilterProcessType", &SetBadWordFilterProcessType);	
	def(pkState, "GetBadWordFilterProcessType", &GetBadWordFilterProcessType);

	def(pkState, "OnMsgBoxClose", &OnMsgBoxClose);

	def(pkState, "GetUKeynoToKeyStr", &GetUKeynoToKeyStr);
	def(pkState, "GetCoupleName", &lwGetCoupleName);
	def(pkState, "SendFireOfLoveSMS", &lwSendFireOfLoveSMS);
	
	def(pkState, "ChangeAllListChkBtn", &lwChangeAllListChkBtn);
	def(pkState, "AttatchParticleToActorNode", &AttatchParticleToActorNode);
	def(pkState, "AttatchParticleToActorPos", &AttatchParticleToActorPos);
	
	def(pkState, "DrawIconToItemNo", &lwDrawIconToItemNo);

	def(pkState, "DelRenderModelNIF", &lwDelRenderModelNIF);

	
	def(pkState, "OnMouseUpRepairItemUseIcon", &lwOnMouseUpRepairItemUseIcon);
	def(pkState, "ClickRepairItemUseButton", &lwClickRepairItemUseButton);
	def(pkState, "OnMouseUpEnchantItemUseIcon", &lwOnMouseUpEnchantItemUseIcon);
	def(pkState, "ClickEnchantItemUseButton", &lwClickEnchantItemUseButton);
	def(pkState, "MakeForceInchantUIText", &lwMakeForceInchantUIText);

	
	def(pkState, "OnCallCenterMiniMap", &lwOnCallCenterMiniMap);
	def(pkState, "OnCloseCenterMiniMap", &lwOnCloseCenterMiniMap);

	def(pkState, "OnCallBattleSquareMiniMap", &lwOnCallBattleSquareMiniMap);

	def(pkState, "UpdateCheckDrawLimitBreakBar", &lwUpdateCheckDrawLimitBreakBar);
	def(pkState, "MoveVCenterUI_TextBase", &lwMoveVCenterUI_TextBase);

	def(pkState, "GetShopGoodsViewSize", &lwGetShopGoodsViewSize);

	def(pkState, "IsMapFillRestExp", &lwIsMapFillRestExp);
	def(pkState, "ShowRestExpBar", &lwShowRestExpBar);
	def(pkState, "GetRestExpToString", &lwGetRestExpToString);
	def(pkState, "Notice_Show", &lwNotice_Show);

	def(pkState, "CallToolTipRepair", lwCallToolTipRepair);

	def(pkState, "UseHiddenItem", lwUseHiddenItem);

	def(pkState, "CallExtendInvMessageBox", lwCallExtendInvMessageBox);
	def(pkState, "ChangeIndexToSysEmoFont", lwChangeIndexToSysEmoFont);
	def(pkState, "IsQuickSkillSlotActive", lwIsQuickSkillSlotActive);

	def(pkState, "PartyStateCloseBtnDown", lwPartyStateCloseBtnDown);
	def(pkState, "IsPlayerPlayTime", lwIsPlayerPlayTime);

	def(pkState, "TogleOffAllJobSkillQuicIcon", lwTogleOffAllJobSkillQuicIcon);
	def(pkState, "SetJobSkillTogleState", lwSetJobSkillTogleState);	
	def(pkState, "CallSafeAdditionUI", lwCallSafeAdditionUI);
	
	def(pkState, "CustomUISummoner_DoAction", lwCustomUISummoner_DoAction);
	def(pkState, "CustomUISummoner_TargetList", lwCustomUISummoner_TargetList);
	def(pkState, "CustomUISummoner_MenuUpdatePos", lwCustomUISummoner_MenuUpdatePos);
	def(pkState, "GetFocusedEdit", lwGetFocusedEdit);
	def(pkState, "lwIsWideResolution", lwIsWideResolution);

	def(pkState, "ShowActorUnitType", lwShowActorUnitType);
	def(pkState, "SetMiniClassIconIndex", lwSetMiniClassIconIndex);

	def(pkState, "SendCheckWorldEvent", lwSendCheckWorldEvent);

	def(pkState, "CallPremiumToolTip", lwCallPremiumToolTip);
	def(pkState, "CallEventScheduleToolTip", lwCallEventScheduleToolTip);
	def(pkState, "CallChannelPartyListToolTip", lwCallChannelPartyListToolTip);
	def(pkState, "SendReqEventStore", &UIGemStore::lwSendReqEventStore);
	def(pkState, "SetGemStoreType", &UIGemStore::lwSetGemStoreType);
	def(pkState, "GetGemStoreType", &UIGemStore::lwGetGemStoreType);

	def(pkState, "GetNextIngameNoticeShowTimeInSec", lwGetNextIngameNoticeShowTimeInSec);

	def(pkState, "ReqExitInstanceDungeon", lwReqExitInstanceDungeon);
	def(pkState, "CheckReservedClassPromotionAction", lwCheckReservedClassPromotionAction);

	//던전 입구
	def(pkState, "World_Dungeon_Portal", lwWorld_Dungeon_Portal);
	def(pkState, "OnEnter_DungeonParty", lwOnEnter_DungeonParty);
	def(pkState, "OnSort_DungeonPartyList", lwOnSort_DungeonPartyList);
	def(pkState, "CheckReservedClassPromotionAction", lwCheckReservedClassPromotionAction);

	//미션 
	def(pkState, "GetMissionCashItemNo", lwGetMissionCashItemNo);
	
	def(pkState, "ParseXml_BasicCombo", lwParseXml_BasicCombo);

	return true;
}

int lwGetMissionCashItemNo()
{
	int const iStage = lua_tinker::call< int >("GetDefenceStageNo");
	CONT_MISSION_DEFENCE_STAGE_BAG::mapped_type kElement;
	const CONT_MISSION_DEFENCE_STAGE_BAG *pkStage;
	g_kTblDataMgr.GetContDef(pkStage);
	if( !pkStage )
	{
		return 0;
	}
	else
	{
		int const iPartyMemberCount = std::max<int>( 1, g_kParty.MemberCount() );

		CONT_MISSION_DEFENCE_STAGE_BAG::key_type kKey(lwGetMissionNo(), iPartyMemberCount, iStage);

		CONT_MISSION_DEFENCE_STAGE_BAG::const_iterator stage_itr = pkStage->find(kKey);		
		if( stage_itr == pkStage->end() )
		{
			return 0;
		}
		else
		{
			kElement = (*stage_itr).second;
			
			SMISSION_DEFENCE_STAGE kValue = kElement.kCont.back();
			int iItemNo = kValue.iDirection_Item;
			return iItemNo;
		}
	}
}


lwWString lwChangeIndexToSysEmoFont(int const Index)
{
	return lwWString(g_kEmoFontMgr.Trans_Key_SysFontString(Index));
}

int lwUIWnd::IconGrp()
{
	CXUI_Icon* pkIconWnd = dynamic_cast<CXUI_Icon*>(self);
	if(pkIconWnd)
	{
		return pkIconWnd->IconInfo().iIconGroup;
	}
	return 0;
}

int lwUIWnd::IconKey()
{
	CXUI_Icon* pkIconWnd = dynamic_cast<CXUI_Icon*>(self);
	if(pkIconWnd)
	{
		return pkIconWnd->IconInfo().iIconKey;
	}
	return 0;
}

bool	lwUIWnd::IsStaticIcon()
{
	CXUI_Icon	*pIcon = dynamic_cast<CXUI_Icon*>(self);
	if(!pIcon) return false;

	return	pIcon->IsStaticIcon();
}
bool	lwUIWnd::IsIcon()
{
	CXUI_Icon	*pIcon = dynamic_cast<CXUI_Icon*>(self);
	if(!pIcon) return false;

	return	true;
}

void	lwUIWnd::SetStaticIcon(bool bStatic)
{
	CXUI_Icon	*pIcon = dynamic_cast<CXUI_Icon*>(self);
	if(!pIcon) return;

	pIcon->IsStaticIcon(bStatic);
}

lwPoint2 lwUIWnd::GetLocation()
{
	if (self)
	{
		return self->Location();
	}	

	NILOG(PGLOG_WARNING, "[GetLocation] Can't find self\n");
	return lwPoint2(0,0);
}

lwPoint3 lwUIWnd::GetLocation3()
{
	if (self)
	{
		return lwPoint3((float)self->Location().x, (float)self->Location().y, (float)self->Location().z);
	}	

	NILOG(PGLOG_WARNING, "[GetLocation3] Can't find self\n");
	return lwPoint3(0,0,0);
}

void lwUIWnd::SetPriority(int const iPriority)
{
	self ? self->Priority(iPriority) : 0;
}

bool lwUIWnd::GetPriority()const
{
	return self ? self->Priority() : 0;
}
//
int lwUIWnd::GetAlignX()
{
	int iAlignX = 0;
	if(self)
	{
		iAlignX = self->AlignX();
	}
	return iAlignX;
}

int lwUIWnd::GetAlignY()
{
	int iAlignY= 0;
	if(self)
	{
		iAlignY = self->AlignY();
	}
	return iAlignY;
}
void lwUIWnd::SetAlignX(int const AlignX)
{
	if(self)
	{
		self->AlignX(AlignX);
	}
}
void lwUIWnd::SetAlignY(int const AlignY)
{
	if(self)
	{
		self->AlignY(AlignY);
	}
}

void lwUIWnd::VAlign()
{
	if(self)
	{
		self->VAlign();
	}
}

void lwUIWnd::SetLocationToMyActor(lwPoint2 pt,char const* strTargetDummy)
{
	if(!g_pkWorld)
	{
		return;
	}
	if (!self)
	{	
		NILOG(PGLOG_WARNING, "[SetLocationToMyActor] Can't find self\n");
		return;
	}
	PgActor	*pkMyActor = g_kPilotMan.GetPlayerActor();
	if(!pkMyActor) 
	{
		if(strcmp(g_pkWorld->GetID().c_str(), "w_char_sel"))
		{
			return;
		}
		else
		{
			pkMyActor = g_kSelectStage.GetNewActor();
			if(NULL==pkMyActor) 
			{
				return;
			}
		}
	}

	NiAVObjectPtr	spTargetDummy = pkMyActor->GetNIFRoot()->GetObjectByName(strTargetDummy);
	if(spTargetDummy == 0) return;

	PgWorld	*pkWorld = pkMyActor->GetWorld();
	if(!pkWorld) return;

	NiCameraPtr	spCamera = pkWorld->m_kCameraMan.GetCamera();
	if(!spCamera) return;

	NiPoint3	kDummyLoc = spTargetDummy->GetWorldTranslate();

	g_pkWorld->AdjustCameraAspectRatio( spCamera );

	float	fBx,fBy;
	if(spCamera->WorldPtToScreenPt(kDummyLoc,fBx,fBy)==false) return;

	NiRenderer	*pkRenderer = NiRenderer::GetRenderer();

	int	iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int	iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	POINT2	ptTargetLoc;

	ptTargetLoc.x = (LONG)(pt.GetX()+iScreenWidth*fBx);
	ptTargetLoc.y = (LONG)(pt.GetY()+iScreenHeight*(1-fBy));

	self->Location(ptTargetLoc);
}

void lwUIWnd::SetLocation(lwPoint2 pt)
{
	if (!self)
	{	
		NILOG(PGLOG_WARNING, "[SetLocation] Can't find self\n");
		return;
	}
	self->Location(pt());
}

void lwUIWnd::SetLocation3(lwPoint3 pt)
{
	if (!self)
	{	
		NILOG(PGLOG_WARNING, "[SetLocation3] Can't find self\n");
		return;
	}
	POINT3I pt3;
	pt3.x = (int)pt().x;
	pt3.y = (int)pt().y;
	pt3.z = (int)pt().z;
	self->Location(pt3);
}

lwPoint2 lwUIWnd::GetTotalLocation()
{
	if (!self)
	{	
		NILOG(PGLOG_WARNING, "[GetTotalLocation] Can't find self\n");
		return lwPoint2(0,0);
	}
	return self->TotalLocation();
}

void lwUIWnd::SetClose(bool bValue)
{
	if (!self)
	{	
		NILOG(PGLOG_WARNING, "[SetClose] Can't find self\n");
		return;
	}
	self->IsClosed(bValue);
}
bool lwUIWnd::GetClose()
{
	if (!self)
	{	
		NILOG(PGLOG_WARNING, "[GetClose] Can't find self\n");
		return false;
	}
	return self->IsClosed();
}

void lwUIWnd::SetTextPos(lwPoint2 kPoint)
{
	if (self)
	{
		self->TextPos(kPoint());
	}
}

lwPoint2 lwUIWnd::GetTextPos()
{ 
	if (self)
	{
		return lwPoint2(self->TextPos());
	}
	return lwPoint2(0,0);
}

lwPoint2 lwUIWnd::GetTextSize()
{
	if( self )
	{
		XUI::CXUI_Style_String kStyleString = self->StyleText();
		return lwPoint2( static_cast<POINT2>(Pg2DString::CalculateOnlySize(kStyleString)) );
	}
	return lwPoint2(0,0);
}

POINT2 CalculateOnlySize(XUI::CXUI_Style_String const& kString)
{
	return static_cast<POINT2>(Pg2DString::CalculateOnlySize(kString));
}

void lwGlobalLBtnUp()
{
/*	XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));

	std::wstring wstrGrp, wstrKey;

	self->GetValue(XUI::VT_ICON_GRP, wstrGrp);
	self->GetValue(XUI::VT_ICON_KEY, wstrKey);//읽어서 작성.

	if(pCursor)
	{
		if( wstrGrp != _T("3") )
		{
			pCursor->IconInfo().x = (BM::vstring)wstrGrp;
			pCursor->IconInfo().y = (BM::vstring)wstrKey;
		}
	}
*/
}

lwPoint2 lwGetScreenSize()
{
	return XUIMgr.GetResolutionSize();
}

// 실제 화면사이즈 중심에서 1024*768 시작 위치를 구함
// +--------------+
// |  +--------+  |
// |  |1024*768|  |
// |  +--------+  |
// +--------------+
POINT2 GetDefaultLocation(POINT2 const & kPos)
{
	POINT2 const & rkScreenSize = XUIMgr.GetResolutionSize();
	return POINT2(rkScreenSize.x/2 - 512 + kPos.x, rkScreenSize.y/2 - 384 + kPos.y);
}

lwPoint2 lwGetDefaultLocation(int const x, int const y)
{
	return GetDefaultLocation(POINT2(x,y));
}

void CalcAbosolute(POINT2 & rkPos, bool const bAbsoluteX, bool const bAbsoluteY)
{
	const POINT2 ptGab(XUIMgr.GetResolutionSize().x - XUI::EXV_DEFAULT_SCREEN_WIDTH, XUIMgr.GetResolutionSize().y - XUI::EXV_DEFAULT_SCREEN_HEIGHT);
	if( false==bAbsoluteX )
	{
		rkPos.x += ptGab.x;
	}
	if( false==bAbsoluteY )
	{
		rkPos.y += ptGab.y;
	}
}
void CalcAbosolute(NiPoint3 & rkPos, bool const bAbsoluteX, bool const bAbsoluteY)
{
	const POINT2 ptGab(XUIMgr.GetResolutionSize().x - XUI::EXV_DEFAULT_SCREEN_WIDTH, XUIMgr.GetResolutionSize().y - XUI::EXV_DEFAULT_SCREEN_HEIGHT);
	if( false==bAbsoluteX )
	{
		rkPos.x += ptGab.x;
	}
	if( false==bAbsoluteY )
	{
		rkPos.y += ptGab.y;
	}
}

lwPoint2 lwGetCursorPos()
{
	return (lwPoint2)(POINT2)XUIMgr.MousePos();
}

lwUIListItem	lwUIWnd::AddNewListItem(lwWString kText)
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		XUI::SListItem *pItem = pList->AddItem(kText());
		if(pItem)
		{
			pItem->m_pWnd->Text(kText());
			return lwUIListItem(pItem);
		}
	}
	return	lwUIListItem(NULL);
}

lwUIListItem	lwUIWnd::AddNewListItemChar(char* szControlId)
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		XUI::SListItem *pItem = pList->AddItem(UNI(szControlId));
		if(pItem)
		{
			pItem->m_pWnd->Text(UNI(szControlId));
			return lwUIListItem(pItem);
		}
	}
	return	lwUIListItem(NULL);
}
void	lwUIWnd::ClearAllListItem()
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		pList->ClearList();
	}
	else
	{
		XUI::CXUI_List2 *pList2 = dynamic_cast<XUI::CXUI_List2*>(self);
		if ( pList2 )
		{
			pList2->ClearList();
		}
	}
}

lwUIListItem	lwUIWnd::ListFirstItem()
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		return	lwUIListItem(pList->FirstItem());
	}
	return	lwUIListItem(NULL);
}

lwUIListItem	lwUIWnd::ListNextItem(lwUIListItem kItem)
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		return	lwUIListItem(pList->NextItem(kItem()));
	}
	return	lwUIListItem(NULL);
}

void	lwUIWnd::ListDeleteItem(lwUIListItem item)
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		pList->DeleteItem(item());
	}
}
lwUIListItem	lwUIWnd::ListFindItem(char *szItemName)
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		return lwUIListItem(pList->FindItem(UNI(szItemName)));
	}
	return	lwUIListItem(NULL);
}
lwUIListItem	lwUIWnd::ListFindItemW(lwWString kItemName)
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		return lwUIListItem(pList->FindItem(kItemName()));
	}
	return	lwUIListItem(NULL);
}
int	lwUIWnd::GetListItemCount()
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		return pList->GetTotalItemCount();
	}
	return 0;
}

void lwUIWnd::ElementID(char const* szName)
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		return pList->ElementID(std::wstring(UNI(szName)));
	}
	return;
}

void lwUIWnd::AdjustMiddleBtnPos()
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if(pList)
	{
		pList->AdjustMiddleBtnPos();
	}
	else
	{
		XUI::CXUI_HScroll *pkScroll = dynamic_cast<XUI::CXUI_HScroll*>(self);
		if ( pkScroll )
		{
			pkScroll->AdjustMiddleBtnPos();
		}
	}
	return;
}

void lwUIWnd::MoveMiddleBtnEndPos()
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if( pList )
	{
		SetDisplayStartPos(pList->GetTotalItemLen() - pList->Height());
	}
}

void lwUIWnd::SetDisplayStartPos(int const iValue)
{
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(self);
	if( pList )
	{
		pList->DisplayStartPos( iValue );
		pList->AdjustMiddleBtnPos();
	}
}

lwUIListItem	lwUIWnd::AddNewTreeItem(lwWString kText)
{
	XUI::CXUI_Tree *pTree = dynamic_cast<XUI::CXUI_Tree*>(self);
	if(pTree)
	{
		XUI::SListItem* pAddedItem = pTree->AddItem(_T(""));
		if(pAddedItem)
		{
			pAddedItem->m_pWnd->Text(kText());
			return lwUIListItem(pAddedItem);
		}
	}
	return	lwUIListItem(NULL);
}
lwUIListItem lwUIWnd::AddNewTreeItemChar(char* szControlId)
{
	XUI::CXUI_Tree *pTree = dynamic_cast<XUI::CXUI_Tree*>(self);
	if(pTree)
	{
		XUI::SListItem* pAddedItem = pTree->AddItem(_T(""));
		if(pAddedItem)
		{
			pAddedItem->m_pWnd->Text(UNI(szControlId));
			return lwUIListItem(pAddedItem);
		}
	}
	return	lwUIListItem(NULL);
}
lwUIListItem	lwUIWnd::TreeFirstItem()
{
	XUI::CXUI_Tree *pTree = dynamic_cast<XUI::CXUI_Tree*>(self);
	if(pTree)
	{
		return	lwUIListItem(pTree->FirstItem());
	}
	return	lwUIListItem(NULL);
}
lwUIListItem	lwUIWnd::TreeNextItem(lwUIListItem item)
{
	XUI::CXUI_Tree *pTree = dynamic_cast<XUI::CXUI_Tree*>(self);
	if(pTree)
	{
		return	lwUIListItem(pTree->NextItem(item()));
	}
	return	lwUIListItem(NULL);
}
void	lwUIWnd::TreeDeleteItem(lwUIListItem item)
{
	XUI::CXUI_Tree *pTree = dynamic_cast<XUI::CXUI_Tree*>(self);
	if(pTree)
	{
		pTree->DeleteItem(item());
	}
}

void	lwUIWnd::ClearAllTreeItem()
{
	XUI::CXUI_Tree *pTree = dynamic_cast<XUI::CXUI_Tree*>(self);
	if(pTree)
	{
		pTree->DeleteAllItem();
	}
}
int	lwUIWnd::GetTreeItemCount()
{
	XUI::CXUI_Tree *pTree = dynamic_cast<XUI::CXUI_Tree*>(self);
	if(pTree)
	{
		return pTree->GetItemCount();
	}
	PG_ASSERT_LOG(0);
	return	0;
}

bool lwUIWnd::IsExpandTree()
{
	XUI::CXUI_Tree *pTree= dynamic_cast<XUI::CXUI_Tree*>(self);
	assert(pTree);
	if(pTree)
	{
		return pTree->CheckState();
	}
	return false;
}

void	lwUIWnd::ExpandTree()
{
	XUI::CXUI_Tree *pTree = dynamic_cast<XUI::CXUI_Tree*>(self);
	if(pTree)
	{
		pTree->CheckState(true);
	}
}
void	lwUIWnd::CollapseTree()
{
	XUI::CXUI_Tree *pTree = dynamic_cast<XUI::CXUI_Tree*>(self);
	if(pTree)
	{
		pTree->CheckState(false);
	}
}

int		lwUIWnd::GetWidth()
{
	return self->Width();
}

int		lwUIWnd::GetHeight()
{
	return self->Height();
}


bool lwUIWnd::IsNil()
{
	return (self == 0);
}

bool lwUIWnd::IsFocus()
{
	if (!self)
	{
		return false;
	}
	return self->IsFocus();
}

bool lwUIWnd::IsEditFocus()
{
	if( !self )
	{
		return false;
	}
	XUI::CXUI_Wnd*	pEdit = XUIMgr.GetFocusedEdit();
	if( !pEdit )
	{
		return false;
	}
	return (self == pEdit)?(true):(false);
}

bool lwUIWnd::Close()
{
	if (!self)
	{
		return false;
	}
	XUIMgr.Close( self->ID() );
	g_kUIScene.DelUIAction(self->ID());
	return true;
}

bool lwUIWnd::CloseParent()
{
	if (!self)
	{
		return false;
	}
	XUI::CXUI_Wnd* pParent = self->Parent();
	if (pParent)
	{
		pParent->Close();
		g_kUIScene.DelUIAction(pParent->ID());
	}
	else
	{
		return false;
	}
	
	return true;
}

bool lwUIWnd::ClearOutside()
{
	if (!self)
	{
		return false;
	}

	self->ClearOutside();
	return true;
}

bool lwUIWnd::SetLastTargetInfo(lwUIWnd UIParent)
{
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);

	PgPilot const *pkPilot = g_kPilotMan.FindPilot(g_LastAttackTarget);
	if(pkPilot && 0<pkPilot->GetAbil(AT_HP) )
	{
		int const iHP = pkPilot->GetAbil(AT_HP);
		int const iMaxHP = pkPilot->GetAbil(AT_C_MAX_HP);

		if (pWnd)
		{
			pWnd->Now(iHP);
			pWnd->Max(iMaxHP);
		}
		else
		{
			NILOG(PGLOG_WARNING, "[SetLastTargetInfo] Can't find self\n");
			return false;
		}

		if(pkPilot->GetUnit())
		{
			pWnd->Text(pkPilot->GetUnit()->Name());
		}
		return true;
	}
	else
	{
		if ( UIParent.IsNil() == false )
		{
			UIParent.Close();
		}
	}

	return false;
}

void lwUIWnd::AdjustToOwnerPos()
{
	XUI::CXUI_Form* pWnd = dynamic_cast<XUI::CXUI_Form*>(self);
	bool bWillClose = false;
	if(pWnd)
	{
		BM::GUID kOwnerGuid = pWnd->OwnerGuid();
		
		PgPilot *pkPilot = g_kPilotMan.FindPilot(kOwnerGuid);
		if(pkPilot)
		{
			PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			NiCamera *pkCamera = pkActor->GetWorld()->m_kCameraMan.GetCamera();
			
			POINT2 ptSrc = XUIMgr.GetResolutionSize();

			float fX = 0.f, fY = 0.f;
			if(pkCamera->WorldPtToScreenPt(pkActor->GetWorldTranslate(), fX, fY))
			{
				POINT2 kPos((LONG)(ptSrc.x * fX), (LONG)(ptSrc.y * (1-fY)));
				pWnd->Location(kPos);//위치 변경
			}
		}
		else
		{
			bWillClose = true;
		}
	}
	else
	{
		NILOG(PGLOG_WARNING, "[AdjustToOwnerPos] Can't find self\n");
		return;
	}

	if(bWillClose)
	{
		pWnd->Close();//찾아보고 없으면 자동 닫음.
	}
}

void lwUIWnd::SetLocationByWorldNode(char* const pkNodeName, lwPoint2 kMovePt)
{
	XUI::CXUI_Form* pWnd = dynamic_cast<XUI::CXUI_Form*>(self);
	bool bWillClose = false;
	if(pWnd && g_pkWorld)
	{
		NiPoint3 kSpawnLoc;
		NiAVObject *pkNode = g_pkWorld->GetSceneRoot()->GetObjectByName(pkNodeName);
		if(!pkNode)
		{
			return;
		}
		kSpawnLoc = pkNode->GetWorldTranslate();
		NiCamera *pkCamera = g_pkWorld->GetCameraMan()->GetCamera();
		POINT2 ptSrc = XUIMgr.GetResolutionSize();
		float fX = 0.f, fY = 0.f;
		if(pkCamera->WorldPtToScreenPt(kSpawnLoc, fX, fY))
		{
			POINT2 kPos((LONG)(ptSrc.x * fX), (LONG)(ptSrc.y * (1-fY)));
			pWnd->Location( POINT2( kPos.x+kMovePt.GetX(), kPos.y+kMovePt.GetY() ) );//위치 변경
		}
		else
		{
			bWillClose = true;
		}
	}
	else
	{
		NILOG(PGLOG_WARNING, "[SetLocationByWorldNode] Can't find self\n");
		return;
	}

	if(bWillClose)
	{
		pWnd->Close();//찾아보고 없으면 자동 닫음.
	}
}

void lwUIWnd::RemoveInfoUI()
{
	XUI::CXUI_Form* pWnd = dynamic_cast<XUI::CXUI_Form*>(self);
	bool bWillClose = false;
	if(pWnd)
	{
		BM::GUID kOwnerGuid = pWnd->OwnerGuid();
		
		PgPilot *pkPilot = g_kPilotMan.FindPilot(kOwnerGuid);
		if(pkPilot)
		{
			PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			//pkActor->InfoUI(NULL);
		}
	}
	else
	{
		NILOG(PGLOG_WARNING, "[RemoveInfoUI] Can't find self\n");
	}
}

lwGUID lwUIWnd::GetOwnerGuid()
{
	if (NULL == self)
	{
		NILOG(PGLOG_WARNING, "[GetOwnerGuid] Can't find self\n");
		return lwGUID("");
	}
	return lwGUID(self->OwnerGuid());
}
void lwUIWnd::SetOwnerGuid(lwGUID lwkGuid)
{
	if (NULL == self)
	{
		NILOG(PGLOG_WARNING, "[SetOwnerGuid] Can't find self\n");
		return;
	}
	self->OwnerGuid(lwkGuid());
}

void lwUIWnd::SetOwnerState(int const iState)
{
	if (NULL == self)
	{
		NILOG(PGLOG_WARNING, "[GetOwnerState] Can't find self\n");
		return;
	}
	self->OwnerState(iState);
}

int lwUIWnd::GetOwnerState()
{
	if (NULL == self)
	{
		NILOG(PGLOG_WARNING, "[GetOwnerState] Can't find self\n");
		return 0;
	}
	return self->OwnerState();
}

void lwUIWnd::CursorToThis()
{
//	self->CursorToThis();
}

void lwUIWnd::SetCursorState(int iState)
{
	XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
	if(pCursor)
	{
		pCursor->CursorState((E_CURSOR_STATE)iState);
	}
}

const E_CURSOR_STATE lwUIWnd::GetCursorState() const
{
	XUI::CXUI_Cursor *pCursor = dynamic_cast<XUI::CXUI_Cursor*>(XUIMgr.Get(WSTR_XUI_CURSOR));
	if(pCursor)
	{
		return pCursor->CursorState();
	}

	return CS_NONE;
}

void lwUIWnd::DrawMoney( EMonetaryUnit const kMU )
{
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(pkUnit)
	{
		__int64 iMoney = 0;
		switch(kMU)
		{
			case EMoney_Cash:
				{
					iMoney = pkUnit->GetAbil64(AT_CASH);
				}break;
			case EMoney_BonusCash:
				{
					iMoney = pkUnit->GetAbil64(AT_BONUS_CASH);
				}break;
			case EMoeny_GuildMoney:
				{
					PgInventory *pkInv = g_kGuildMgr.GetInven();
					if( pkInv )
					{
						iMoney = pkInv->Money();
					}
				}break;
			default:
				{
					iMoney = pkUnit->GetAbil64(AT_MONEY);
				}break;
		}	
		DrawMoneyByValue( kMU, iMoney );
	}
}

void lwUIWnd::DrawEmoticonMoney()
{
	if (NULL == self)
	{
		NILOG(PGLOG_WARNING, "[DrawMoneyByValue] Can't find self\n");
		return;
	}

	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(pkUnit)
	{
		__int64 iMoney = pkUnit->GetAbil64(AT_MONEY);
		self->Text(GetMoneyString(iMoney));
	}
}

void lwUIWnd::DrawMoneyByValue( EMonetaryUnit const kMU, __int64 iMoney)
{
	if (NULL == self)
	{
		NILOG(PGLOG_WARNING, "[DrawMoneyByValue] Can't find self\n");
		return;
	}

	BM::vstring vstrLv( GetMoneyMU( kMU, iMoney )) ;
	self->Text(vstrLv);
}

bool lwUIWnd::IsClickLock(void)
{
	XUI::CXUI_CheckButton *pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(self);
	if ( pkChkBtn )
	{
		return pkChkBtn->ClickLock();
	}
	return false;
}

void lwUIWnd::SetClickLock( bool const bLock )
{
	XUI::CXUI_CheckButton *pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(self);
	if ( pkChkBtn )
	{
		pkChkBtn->ClickLock( bLock );
	}
}

void lwUIWnd::SetState( int const iState )
{
	if ( self )
	{
		self->SetState( static_cast<XUI::E_State>(iState) );
	}
}

int lwUIWnd::GetState()
{
	if ( self )
	{
		return static_cast<int>(self->GetState());
	}
	return XUI::ST_NONE;
}

bool lwUIWnd::SetPilotBar(int iType)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(pkPlayer)
	{
		CXUI_AniBar	*pWnd = dynamic_cast<CXUI_AniBar*>(self);
		if(!pWnd)
		{
			NILOG(PGLOG_WARNING, "[SetPilotBar] Can't find self\n");
			return false;
		}

		bool bClearText = false;//텍스트 사용 안함

		__int64 iNow = 0;
		__int64 iMax = 0;
		switch(iType)
		{
		case 1:
			{
				iNow = pkPlayer->GetAbil(AT_HP);// 100;
				iMax = pkPlayer->GetAbil(AT_C_MAX_HP);// 100;

				int const iTickRecveryValue = pkPlayer->GetAbil(AT_HP_RECOVERY_TICK_ITEM_AMOUNT);
				CXUI_AniBar* pkTick = dynamic_cast<CXUI_AniBar*>(self->Parent()->GetControl(_T("HP_TICK")));
				if( pkTick )
				{
					pkTick->Max(iMax);
					pkTick->Now(std::min(iMax, iNow + iTickRecveryValue));
				}

				if (pWnd->Max()==iMax && pWnd->Now() > iNow)
				{
					pWnd->TwinkleTime(1000);
					pWnd->TwinkleInterTime(100);
					pWnd->SetTwinkle(true);
				}
			}break;
		case 2:
			{
				iNow = pkPlayer->GetAbil(AT_MP);// 100;
				iMax = pkPlayer->GetAbil(AT_C_MAX_MP);// 100;

				int const iTickRecveryValue = pkPlayer->GetAbil(AT_MP_RECOVERY_TICK_ITEM_AMOUNT);
				CXUI_AniBar* pkTick = dynamic_cast<CXUI_AniBar*>(self->Parent()->GetControl(_T("MP_TICK")));
				if( pkTick )
				{
					pkTick->Max(iMax);
					pkTick->Now(std::min(iMax, iNow + iTickRecveryValue));
				}
			}break;
		case 4: 
			{
				int const iLv = pkPlayer->GetAbil(AT_LEVEL);// 100;
				int const iClass = pkPlayer->GetAbil(AT_CLASS);
				__int64 const i64Exp = pkPlayer->GetAbil64(AT_EXPERIENCE);

				GET_DEF(PgClassDefMgr, kClassDefMgr);
				__int64 const i64Begin = kClassDefMgr.GetAbil64(SClassKey(iClass, iLv), AT_EXPERIENCE);
				__int64 const i64NextEXP = kClassDefMgr.GetAbil64(SClassKey(iClass, iLv+1), AT_EXPERIENCE);

				//	PG_ASSERT_LOG( iNextEXP > 0);
				//	PG_ASSERT_LOG( iBegin > 0);
				if(i64NextEXP > 0 
					&& i64Begin > 0)
				{
					iNow = std::max((__int64)0, (i64Exp - i64Begin));// 100;
					iMax = std::max((__int64)0, (i64NextEXP - i64Begin));
				}
				else
				{
					iNow = i64Exp;
					iMax = i64NextEXP;
				}
			}break;
		case 5:
			{// 휴식 경험치 표현
				int const iLv = pkPlayer->GetAbil(AT_LEVEL);// 100;
				int const iClass = pkPlayer->GetAbil(AT_CLASS);
				__int64 const i64Exp = pkPlayer->GetAbil64(AT_EXPERIENCE);
				__int64 const i64RestExp = pkPlayer->GetAbil64(AT_REST_EXP_ADD_MAX)/* + i64Exp*/;

				GET_DEF(PgClassDefMgr, kClassDefMgr);
				__int64 const i64Begin = kClassDefMgr.GetAbil64(SClassKey(iClass, iLv), AT_EXPERIENCE);
				__int64 const i64NextEXP = kClassDefMgr.GetAbil64(SClassKey(iClass, iLv+1), AT_EXPERIENCE);

				if(i64NextEXP > 0 
					&& i64Begin > 0)
				{
					iNow = std::max((__int64)0, (i64RestExp - i64Begin));// 100;
					iMax = std::max((__int64)0, (i64NextEXP - i64Begin));
				}
				else
				{
					iNow = i64RestExp;
					iMax = i64NextEXP;
				}
				iNow = std::min(iNow, iMax);
			}break;
		case 9: // Fran
			{
				// Fran = Level + 1
				int iLv = pkPlayer->GetAbil(AT_LEVEL) + 1;// 100;
				int const iClass = pkPlayer->GetAbil(AT_CLASS);
				__int64 iFran  = pkPlayer->GetAbil64(AT_FRAN);

				GET_DEF(PgClassDefMgr, kClassDefMgr);
				__int64 iBegin = kClassDefMgr.GetAbil64(SClassKey(iClass, iLv), AT_EXPERIENCE);
				__int64 iNextFran = kClassDefMgr.GetAbil64(SClassKey(iClass, iLv+1), AT_EXPERIENCE);

				if( iLv >= PgClassDefMgr::s_sMaximumLevel )
				{
					iNextFran = iBegin;
				}

				if(iNextFran > 0 
					&& iBegin > 0)
				{
					iNow = __max(0, iFran);// 100;
					iMax = __max(0, iNextFran);
				}
				else
				{
					iNow = iFran;
					iMax = iNextFran;
				}
			}break;
		case 10:	//펫 마나
			{
				PgPet* pkPet = GetMySelectedPet();
				if(pkPet)
				{
					bool const b1stType = (EPET_TYPE_1 == static_cast<EPetType>(pkPet->GetPetType()));	//1차펫이냐 2차 펫이냐
					if(b1stType)
					{
						bClearText = true;
						break;
					}
					iNow = pkPet->GetAbil(AT_MP);// 100;
					iMax = pkPet->GetAbil(AT_C_MAX_MP);// 
				}
			}break;
		case 11:	//펫 경험치
			{
				BM::GUID const& rkGuid = pkPlayer->SelectedPetID();
				if(rkGuid.IsNotNull())
				{
					PgPilot const* pkPilot = g_kPilotMan.FindPilot(rkGuid);
					if(pkPilot)
					{
						int iLv = pkPilot->GetAbil(AT_LEVEL);// 100;
						int const iClass = pkPilot->GetAbil(AT_CLASS);
						__int64 iExp = pkPilot->GetAbil64(AT_EXPERIENCE);


						GET_DEF(PgClassPetDefMgr, kClassDefMgr);
						if(iLv>=kClassDefMgr.GetLastLv(iClass))//최대 레벨에 도달하면
						{
							break;
						}
						PgClassPetDef kPetDef;
						__int64 iBegin = 0i64;
						__int64 iNextEXP = 0i64;
						if ( true == kClassDefMgr.GetDef( SClassKey(iClass, iLv), &kPetDef ) )
						{
							if(EPET_TYPE_1==kPetDef.GetPetType())
							{
								bClearText = true;
								break;
							}
							iBegin = kPetDef.GetAbil64(AT_EXPERIENCE);
						}

						if ( true == kClassDefMgr.GetDef( SClassKey(iClass, iLv+1), &kPetDef ) )
						{
							if(EPET_TYPE_1==kPetDef.GetPetType())
							{
								bClearText = true;
								break;
							}
							iNextEXP = kPetDef.GetAbil64(AT_EXPERIENCE);
						}

						if(iNextEXP > 0 && iBegin > 0)
						{
							iNow = std::max((__int64)0, (iExp - iBegin));// 100;
							iMax = std::max((__int64)0, (iNextEXP - iBegin));
						}
						else
						{
							iNow = iExp;
							iMax = iNextEXP;
						}
					}
				}
			}break;
		}//end switch(iType)

		if( USE_AMOUNT_BASE_VALUE < iMax )
		{
			pWnd->Now( TranslateExp(iNow) );
			pWnd->Max( TranslateExp(iMax) );
		}
		else
		{
			pWnd->Now( static_cast<int>(iNow) );
			pWnd->Max( static_cast<int>(iMax) );
		}
		
		float const Rate = ((iMax)?((float)iNow/(float)iMax):0);
		TCHAR szInfo[255] = {0, };
		if( 4 == iType || 9 == iType || 11 == iType )//경험치 바 일때만
		{
			//_stprintf_s(szInfo, 255, _T("LV %d  %d/%d %.2f%%"), pkPilot->GetAbil(AT_LEVEL), iTempNow, iTempMax, Rate*100.0f);	//wchar_t는 부동소숫점 연산이 안됨
#ifndef EXTERNAL_RELEASE
			if(11 == iType)
			{
				_stprintf_s(szInfo, 255, _T("%I64d/%I64d %.2f%%"), iNow, iMax, Rate*100.0f);	//wchar_t는 부동소숫점 연산이 안됨
			}
			else
#endif
			{
				_stprintf_s(szInfo, 255, _T("%.2f%%"), Rate*100.0f);	//wchar_t는 부동소숫점 연산이 안됨
			}
		}
		else if( 3 == iType || 6 == iType || 7 == iType || 8 == iType)	//CP는 출력 안함, 펫 미니창도 표시 안함
		{
			//
		}
		else 
		{
			_stprintf_s(szInfo, 255, _T("%I64d/%I64d %.2f%%"), iNow, iMax, Rate*100.0f);	//wchar_t는 부동소숫점 연산이 안됨
		}

		std::wstring wstrInfo = szInfo;
		if(true==bClearText)
		{
			wstrInfo.clear();
		}
		std::pair<BarInfoMap::iterator, bool> ret = g_BarInfoMap.insert(std::make_pair(iType, wstrInfo));
		if (!ret.second)
		{
			ret.first->second = wstrInfo;
		}
	}
	return false;
}

int lwUIWnd::TranslateExp(__int64 iExp)
{
	float fExp = static_cast<float>(iExp)/ABILITY_FLOAT_MULTIFLY;
	return static_cast<int>(fExp);
}

void lwUIWnd::SetTickInterval(int const iTickInterval)
{
	if (NULL == self)
	{
		NILOG(PGLOG_WARNING, "[SetTickInterval] Can't find self\n");
		return;
	}
	self->TickInterval(iTickInterval);
}

DWORD lwUIWnd::GetTickInterval()
{
	if (NULL == self)
	{
		NILOG(PGLOG_WARNING, "[GetTickInterval] Can't find self\n");
		return 0;
	}
	return self->TickInterval();
}

bool lwClearEditFocus()
{
	XUIMgr.ClearEditFocus();
	return true;
}

bool lwSetEditFocus(char const *pszParent, char const *pszControl)
{
	return XUIMgr.SetEditFocus( UNI( pszParent ), UNI(pszControl) );
}

void lwSendScriptEvent(char const *pszParent, char const *pszControl, char const *pszScriptKey )
{
	XUIMgr.SendScriptEvent( UNI( pszParent ), UNI(pszControl), UNI(pszScriptKey) );
}

void ExitApp()
{//종료지.
	PostQuitMessage( 0 );
}

lwUIWnd _lwActivateUI(std::wstring const &kID, bool const bModal, std::wstring const& kNewID)
{
	XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( kID, bModal, kNewID);
	if (NULL == pkWnd)
	{
		NILOG(PGLOG_WARNING, "[lwActivateUI] Can't find ID=%s\n", MB(kID));
	}
	return lwUIWnd(pkWnd, false);
}

lwUIWnd lwActivateUI( char const *pszText , bool const bModal, char const* pszNewID)
{
	std::wstring	wstrNewID;
	if( pszNewID )
	{
		wstrNewID = UNI(pszNewID);
	}
	return _lwActivateUI( std::wstring( UNI(pszText) ), bModal, wstrNewID );
}

lwUIWnd lwActivateUIPriority(char const *pszText, int const iPriority)
{
	XUI::CXUI_Wnd *pkWnd = XUIMgr.Activate( std::wstring( UNI(pszText) ) );
	if (NULL == pkWnd)
	{
		NILOG(PGLOG_WARNING, "[lwActivateUIPriority] Can't find ID=%s\n", pszText);
	}
	else
	{
		pkWnd->Priority(iPriority);
	}
	return lwUIWnd(pkWnd, false);
}

bool lwCheckCalledUniqueUIType(char const* pszTypeID)
{
	if( pszTypeID )
	{
		std::wstring kTypeID = UNI(pszTypeID);
		if( !kTypeID.empty() )
		{
			return XUIMgr.CheckExistUniqueLiveType(kTypeID);
		}
	}
	return false;
}

bool lwIsExistUniqueType(char const* pszTypeID)
{		
	if( pszTypeID )
	{
		std::wstring kTypeID = UNI(pszTypeID);
		if( !kTypeID.empty() )
		{
			return XUIMgr.IsExistUniqueLiveType(kTypeID);
		}
	}
	return false;
}

lwUIWnd lwCloneAutoCallUI(char const *pszText, bool bModal)
{
	if( !pszText )
	{
		return lwUIWnd(NULL, false);
	}

	size_t iCount = 0;
	char szTemp[512] = {0, };
	CXUI_Wnd *pkTempWnd = NULL;
	do
	{
		sprintf_s(szTemp, "%s%u", pszText, iCount);
		pkTempWnd = XUIMgr.Get( std::wstring(UNI(szTemp)) );
		++iCount;
	} while( pkTempWnd && !pkTempWnd->IsClosed() );

	return lwCallUI(pszText, bModal, szTemp);
}

bool lwPickWnd(char const *pszText)
{
	return pszText ? XUIMgr.PickWnd( std::wstring( UNI(pszText) ) ) : false;
}

lwUIWnd lwAddChildUI(lwUIWnd kParent, char const *pszElementID, char const *pszNewID, bool const bRemove)
{
	XUI::CXUI_Wnd *pkWnd = NULL;
	if(false==kParent.IsNil() && NULL!=pszElementID && NULL!=pszNewID)
	{
		pkWnd = XUIMgr.AddChildUI( kParent(), std::wstring( UNI(pszElementID) ), std::wstring( UNI(pszNewID) ), bRemove );
	}
	return lwUIWnd(pkWnd, false);
}

lwUIWnd lwCallUI(char const *pszText, bool bModal, char const *pszNewID)
{
	if( !pszText )
	{
		return lwUIWnd(NULL, false);
	}

	std::wstring const kID = UNI(pszText);
	std::wstring const kNewID = (pszNewID)? UNI(pszNewID): std::wstring();
	XUI::CXUI_Wnd* pWnd = NULL;
	if( XUIMgr.IsActivate(kID, pWnd) )
	{
		PG_ASSERT_LOG(pWnd);
		pWnd->SetInvalidate();
		pWnd->Close();
	}
	else
	{
		XUI::CXUI_Wnd* pWnd = XUIMgr.Call(kID, bModal, kNewID);
		if( !pWnd )
		{
			NILOG(PGLOG_ERROR, "Can't find ID='%s'", pszText);
		}
		else
		{
			pWnd->SetInvalidate();
		}
		return lwUIWnd(pWnd, pWnd != NULL);
	}

	NILOG(PGLOG_WARNING, "[lwCallUI] Can't find ID=%s\n", pszText);
	return lwUIWnd(0, false);
}

bool lwCloseUI( char *pszText ) 
{
	bool const bRet = XUIMgr.Close( UNI(pszText) );
	g_kUIScene.DelUIAction(UNI(pszText));
	return bRet;
}

void lwCloseAllUI( bool bNotCursor ) 
{
	NILOG(PGLOG_MINOR, "[lwUI] CloseAllUI\n");
	Quest::lwSaveMiniIngQuestSnapInfo();
	//
	XUIMgr.CloseAll();
	g_kUIScene.DelAllUIAction();
	if ( bNotCursor )
	{
		XUIMgr.Call(WSTR_XUI_CURSOR);
	}
}

void lwReleaseAllControl() 
{
	NILOG(PGLOG_MINOR, "[lwUI] ReleaseAllUI\n");
	// WorldMap, WorldMapPopUp의 릴리즈.
	// 아래의 경우는 따로 XUI포인터를 관리하기 때문에. XUI가 릴리즈 되는 시점에 해제 해줘야 한다.
	g_kWorldMapUI.ReleaseControl();
	if (g_kUIScene.GetWorldMapPopUpUI())
	{
		g_kUIScene.GetWorldMapPopUpUI()->ReleaseControl();
	}

	// XUI Release All Control
	XUIMgr.ReleaseAllControl();
}

//void lwRecvOtherItemChange(lwGUID kGuid, int SrcGrp, int SrcKey, int TgtGrp, int TgtKey)
void lwRecvOtherItemChange(lwPacket &Packet)
{
	/*
	BM::Stream& rkPacket = *(Packet());
	PgBase_Item kSrcItem, kTgtItem;

	lwGUID kGuid = Packet.PopGuid();
	
	SItemPos kSrcItemPos, kTgtItemPos;
	rkPacket.Pop(kSrcItemPos);
	rkPacket.Pop(kSrcItem);
	rkPacket.Pop(kTgtItemPos);
	rkPacket.Pop(kTgtItem);

	PgPilot *pkPilot = g_kPilotMan.FindPilot(kGuid());
	if(!pkPilot || !pkPilot->GetUnit())
	{
		return;
	}

	PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
	if(!pkActor)
	{
		return;
	}

	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*> pkPilot->GetUnit();
	if (kSrcItemPos.group == KUIG_EQUIP)
	{
		PgBase_Item const* pkSrcItem = pkPlayer->GetInven()->GetItem(kSrcItemPos);
		if (!PgBase_Item::IsEmpty(pkSrcItem))
		{
			pkPlayer->GetInven()->EquipItem(kSrcItemPos, &kSrcItem);
			pkActor->UnequipItem(kSrcItemPos.key);
		}
	}
	if (kTgtItemPos.group == KUIG_EQUIP)
	{
		PgBase_Item const* pkTargetItem = pkPlayer->GetInven()->GetItem(kTgtItemPos);
		if (!PgBase_Item::IsEmpty(pkTargetItem))
		{
			pkPlayer->GetInven()->EquipItem(kTgtItemPos, &kTgtItem);
			pkActor->EquipItemByPos(kTgtItemPos.key);
		}
	}
	CHANGE_INVEN_ABIL(pkPlayer, pkPlayer->GetInven())
*/
}

void	lwUIWnd::RemoveAllControls()
{
	if (self)
	{
		self->RemoveAllControls();
	}
}
int	lwUIWnd::GetContCount()
{
	if (self)
	{
		return	self->GetContCount();
	}
	return 0;
}
lwUIWnd	lwUIWnd::GetContAt(int iIndex)
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetContAt] Can't find self\n");
		return lwUIWnd(NULL);
	}
	XUI::CXUI_Wnd* pWnd = self->GetContAt(iIndex);
	if (pWnd == NULL)
	{
		NILOG(PGLOG_WARNING, "[lwUIWnd::GetContAt] Can't find %d of %s\n", iIndex, MB(self->ID().c_str()));
	}
	return	lwUIWnd(pWnd, false);
}

XUI::CXUI_Wnd* lwUIWnd::GetSelf()
{
	return self;
}

bool lwUIWnd::IsFinishedTextDraw()
{
	if( !self )
	{
		NILOG(PGLOG_WARNING, "[IsFinishedTextDraw] Can't find self\n");
		return false;
	}

	PgTextTypistWnd* pWnd = dynamic_cast<PgTextTypistWnd*>(self);
	return ( pWnd )?(pWnd->DrawFinish()):(false);
}

void lwUIWnd::SkipTDrawInterval()
{
	if( !self )
	{
		NILOG(PGLOG_WARNING, "[SkipTDrawInterval] Can't find self\n");
		return;
	}

	PgTextTypistWnd* pWnd = dynamic_cast<PgTextTypistWnd*>(self);
	if( pWnd )
	{
		pWnd->SkipDrawInterval();
	}
}

void lwChangeMapInfo(int mapNo)
{
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("MapInfo"));

	if(pWnd)
	{
		std::wstring wstrText = pWnd->Text();
		
		const CONT_DEFMAP* pContDefMap = NULL;
		g_kTblDataMgr.GetContDef(pContDefMap);

		CONT_DEFMAP::const_iterator itor = pContDefMap->find(mapNo);
		if(itor!=pContDefMap->end())
		{
			const wchar_t *pText = NULL;
			if(GetDefString((*itor).second.NameNo, pText))
			{
				pWnd->Text(pText);
			}
		}
	}
}


void lwSetLastAttackTarget(lwGUID guid)
{
	g_LastAttackTarget = guid();
}

void lwUIWnd::SetPilotLv()
{
	XUI::CXUI_Wnd* pWnd = self;

	if (NULL == pWnd)
	{
		NILOG(PGLOG_WARNING, "[SetPilotLv] Can't find self\n");
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(pkPlayer)
	{
		int iLv = pkPlayer->GetAbil(AT_LEVEL);

		BM::vstring vstrLv(TTW(224));
		vstrLv += L".";
		vstrLv += iLv;
		pWnd->Text(vstrLv);//pkPilot->GetName() );
	}
}

float g_fCastStartTime	= 0;//시전시작시간
float g_fCastFinTime	= 0;//시전종료시간
float g_fSkillFinTime	= 0;//스킬 사용 한계
char	g_strGaugeText[256];

int g_iAccValue=0;//쌓인 값 --> 스킬 쓸때 이걸 빼줘버려

float g_fAccTime=0;//누적시간
int const GAUGE_LENGTH = 1000;// 게이지가 표현 할 수 있는 최대 숫자.

void SkillOnTick(float const fAccumTime, float const fFrameTime)
{//전체 * 지난시간누적/풀시간
	if(!g_fSkillFinTime && !g_iAccValue)
	{//보일 필요 없음.
		XUIMgr.Close(_T("DelayBar"));
		return;
	}
		
	XUIMgr.Call(_T("DelayBar"));

	//딜레이 바를 부른다.
	//각종 값을 셋팅 해둔다.
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("DelayBar"));

	if(g_fCastFinTime)//캐스팅 말반 시간 까지는 그냥 찬다.
	{
		if(g_fCastFinTime >= g_fCastStartTime+g_fAccTime)
		{
			g_fAccTime += fFrameTime;
			g_fAccTime = __min(g_fCastFinTime-g_fCastStartTime, g_fAccTime);

			g_iAccValue = (int)((GAUGE_LENGTH*g_fAccTime)/(g_fCastFinTime-g_fCastStartTime));

			if(g_fCastFinTime <= g_fCastStartTime+g_fAccTime)
			{
				//쌓는게 역전했거나 같아졌다면
				g_fCastFinTime = 0;
				g_fCastStartTime = 0;
				g_fAccTime = 0;
			}
		}
	}

	if( g_fSkillFinTime <= fAccumTime)
	{
		//스킬 끝.
		g_fSkillFinTime=0;
		g_iAccValue=0;
	}

	XUI::CXUI_Wnd *pControlWnd = pWnd->GetControl(_T("Guage"));
	if (pControlWnd)
	{
		CXUI_AniBar* pkBarWnd = dynamic_cast<CXUI_AniBar*>(pControlWnd);
		if(pkBarWnd)
		{
			pkBarWnd->Now(g_iAccValue);
			pkBarWnd->Max(GAUGE_LENGTH);
			pkBarWnd->Text(UNI(g_strGaugeText));
		}
	}
	else
	{
		NILOG(PGLOG_WARNING, "[SkillOnTick] Can't find Guage\n");
	}
}

void lwSetCoolTime(float value,char const *strText)
{
	if(!g_pkWorld)
	{
		return;
	}
	g_fCastStartTime = g_pkWorld->GetAccumTime();
	g_fCastFinTime = g_fCastStartTime + value;
	g_fSkillFinTime = g_fCastFinTime + 0.2f;
	strcpy_s(g_strGaugeText, 255, strText);
}

void lwSendPickupItem(int const iKey)
{
/*
	BM::GUID kGuid;
	if(g_kPilotMan.GetPlayerPilot(kGuid))
	{
		PgPilot *pkPilot = g_kPilotMan.FindPilot(kGuid);
		if(pkPilot)
		{
			PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
			if(!pkPlayer){return;}

			// QUESTION: 코드가 이상하다. pilot의 GetWorldObject가 DropBox?
			PgDropBox *pkDropBox = dynamic_cast<PgDropBox *>(pkPilot->GetWorldObject());
			if(pkDropBox)
			{
				if(!pkDropBox->GetItem(iKey))
				{
					return;
				}

				int iKeyPos = 0;
				SItemPos kTargetPos(2, iKeyPos);
				SItemPos kSourcePos(4, iKey);

				PgBase_Item *pkItem = (SItem*) pkPlayer->GetInven()->GetItem(kTargetPos);

				while(!PgBase_Item::IsEmpty(pkItem) && kTargetPos.y < 8)
				{
					++kTargetPos.y;
					pkItem = (SItem*) pkPlayer->GetInven()->GetItem(kTargetPos);
				}

				if(!PgBase_Item::IsEmpty(pkItem))
				{
					NiMessageBox("Your inventory is full!", "");
					return;
				}

				BM::Stream kPacket(12364);	// PT_C_M_REQ_PICKUPITEM
				char iItemCount = 1;
				kPacket.Push(pkDropBox->GetGuid());
				kPacket.Push(iItemCount);
				kPacket.Push(kSourcePos);
				kPacket.Push(kTargetPos);

				NETWORK_SEND(kPacket)
			}
		}
	}	
*/
}

void lwRecvPickupItem(lwPacket kPacket)
{
/*
	BM::Stream *pkPacket = kPacket();
	
	SGroundBoxInfo tempInfo;
	pkPacket->Pop(tempInfo);
	if (tempInfo.eRet != GITEM_ANS_OK)
	{
		if(tempInfo.eRet == GITEM_ANS_WRONG_OWNER) // 주인이 아니다
		{
			OutputDebugString(UNI("lwRecvPickupItem 주인이 아님 에러코드 91\n"));		
		}
		else if(tempInfo.eRet == GITEM_ANS_INVALID) 
		{
			OutputDebugString(UNI("lwRecvPickupItem GroundBox 가 Invalid 하다 (없어졌다) 에러코드 92\n"));		
		}
		else if(tempInfo.eRet == GITEM_ANS_TOOFAR) 
		{
			OutputDebugString(UNI("lwRecvPickupItem Player의 거리가 너무 멀다 에러코드 93\n"));		
		}
		else if(tempInfo.eRet == GITEM_ANS_FULLINVEN) 
		{
			OutputDebugString(UNI("lwRecvPickupItem Inventory가 가득 찼다 에러코드 94\n"));		
		}
		else if(tempInfo.eRet == GITEM_ANS_WRONGPOS) 
		{
			OutputDebugString(UNI("lwRecvPickupItem 잘못된 Position 이다 에러코드 95\n"));		
		}
		else if(tempInfo.eRet == GITEM_ANS_INVALID_SLOT) 
		{
			OutputDebugString(UNI("lwRecvPickupItem 잘못된 Slot (이미 아이템이 들어 있다) 에러코드 96\n"));		
		}
		else if(tempInfo.eRet == GITEM_ANS_GIVE_TO_PARTY) 
		{
			OutputDebugString(UNI("lwRecvPickupItem 얻은 아이템을 파티멤버에게 주었다! 에러코드 97\n"));		
		}
		else
		{
			TCHAR szError[255] = {0,};
			_stprintf_s(szError, 255, _T("lwRecvPickupItem 알수 없는 에러. 에러코드 %d \n"), tempInfo.eRet);
			OutputDebugString(szError);
		}

		return;
	}

	BM::GUID kBoxGuid = tempInfo.kGBoxGuid;
	OutputDebugString((kBoxGuid.str().c_str()));

	char cCount;
	pkPacket->Pop(cCount);
	TCHAR szCount[255] = {0,};//"드롭박스 아이템 갯수 : ";
	_stprintf_s(szCount, 255, _T("드롭박스 아이템 갯수 : %d \n"), (int)cCount);

	OutputDebugString(szCount);

	BM::GUID kGuid;
	if(!g_kPilotMan.GetPlayerPilot(kGuid))
	{
		return;
	}

	PgPilot *pkPilot = g_kPilotMan.FindPilot(kGuid);
	if(!pkPilot)
	{
		return;
	}

	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*> pkPilot->GetUnit();
	PgDropBox* pBox = dynamic_cast<PgDropBox *>(pkPilot->GetWorldObject());
	OutputDebugString(pBox->GetGuid().str().c_str());
	for(int iItemIdx=0; iItemIdx<cCount; ++iItemIdx)
	{
		SItemPos kFrom;
		SItemPos kTo;
		pkPacket->Pop(kFrom);
		pkPacket->Pop(kTo);

		PgBase_Item kItemOut;
		
		if (pBox == NULL)
		{
			continue;
		}
		else if(pBox->PopItem(kFrom.y, kItemOut) == ERROR_SUCCESS)
		{
			pkPlayer->GetInven()->AddNewItem(kItemOut, kTo);
			pBox->TransitAction("open");
			pBox->ChaseOwner();
			OutputDebugString(UNI("pBox->ChaseOwner()\n"));
		}
	}
*/
}

void lwExitClient()
{
	::PostQuitMessage(0);
}


//펫 정보 화면에 출력
void lwGetPetInfo()
{
/*	PgPilot *pkPetPilot = 0;
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	BM::GUID kPetGuid;
	
	if(!pkPlayer)
	{
		return;
	}

	kPetGuid = pkPlayer->PetGuid();
	pkPetPilot = g_kPilotMan.FindPilot(kPetGuid);
	if(!pkPetPilot)
	{
		return;
	}

	PgPet *pkPet = dynamic_cast<PgPet *>(pkPetPilot->GetUnit());
	if(!pkPet)
	{
		return;
	}

	XUI::CXUI_Wnd* form;
	if(XUIMgr.IsActivate(_T("CharInfo"), form))
	{
		//XUI::CXUI_Wnd *form = XUIMgr.Get(_T("CharInfo"));
		XUI::CXUI_Wnd *petform = form->GetControl(_T("Pet"));

		if(petform)
		{
			std::wstring szHungryState[] = 
			{
				TTW(21001),
				TTW(21002),
				TTW(21003),
				TTW(21004),
				TTW(21005),
			};

			std::wstring szLoyaltyState[] = {
				TTW(21011),
				TTW(21012),
				TTW(21013),
				TTW(21014),
				TTW(21015),
			};

			std::wstring szBloodType[] = {
				TTW(40),
				TTW(41),
				TTW(42),
				TTW(43)
			};

			TCHAR szBuf[1024];
			int iValue = 0;
			
			XUI::CXUI_Wnd* pWnd = 0;
			
			pWnd = petform->GetControl(_T("LEVEL_VALUE"));
			wsprintfW(szBuf, _T("Lv.%d"), pkPet->GetAbil(AT_LEVEL));
			pWnd->Text(szBuf);

			pWnd = petform->GetControl(_T("NAME_VALUE"));
			wsprintfW(szBuf, TTW(38).c_str(), pkPet->Name(), 2);
			pWnd->Text(szBuf);

			pWnd = petform->GetControl(_T("BLOOD_VALUE"));
			wsprintfW(szBuf, _T("%s"), szBloodType[NiClamp(pkPet->GetAbil(AT_BLOOD), 0, 3)].c_str());
			pWnd->Text(szBuf);

//			pWnd = petform->GetControl(_T("ATTACK_VALUE"));	//	leesg213 주석처리 AT_PHY_POWER 라는 어빌이 없음 ;;
//			wsprintfW(szBuf, _T("%d"), g_PetDef.GetAbil(pkPet->GetAbil(AT_LEVEL), AT_PHY_POWER));
//			pWnd->Text(szBuf);

			pWnd = petform->GetControl(_T("AGE_VALUE"));
			wsprintfW(szBuf, TTW(39).c_str(), pkPet->GetAbil(AT_AGE));
			pWnd->Text(szBuf);

			pWnd = petform->GetControl(_T("HUNGER_VALUE"));
			iValue = NiClamp(pkPet->GetAbil(AT_HUNGRY), 0, 99);

			int hungry_index = (int)(iValue/20);
			hungry_index = __min(hungry_index, 4);
			hungry_index = __max(hungry_index, 0);

			wsprintfW(szBuf, _T("%s"), szHungryState[hungry_index].c_str());
			pWnd->Text(szBuf);

			pWnd = petform->GetControl(_T("LOYAL_VALUE"));
			iValue = NiClamp(pkPet->GetAbil(AT_LOYALTY), 0, 99);

			int loyalty_index = (int)(iValue/20);
			loyalty_index = __min(loyalty_index, 4);
			loyalty_index = __max(loyalty_index, 0);

			wsprintfW(szBuf, _T("%s"), szLoyaltyState[loyalty_index].c_str());
			pWnd->Text(szBuf);
		}
	}
	
	XUI::CXUI_Wnd *pkMiniPetInfo = NULL;
	if(XUIMgr.IsActivate(_T("FRM_PetInfo"), pkMiniPetInfo))
	{
		TCHAR szBuf[1024];
		//XUI::CXUI_Wnd *pkMiniPetInfo = XUIMgr.Get(_T("FRM_PetInfo"));
		XUI::CXUI_Wnd *pkWnd = 0;
		
		pkWnd = pkMiniPetInfo->GetControl(_T("NAME_VALUE"));
		wsprintfW(szBuf, _T("%s"), pkPet->Name());
		pkWnd->Text(szBuf);

		pkWnd = pkMiniPetInfo->GetControl(_T("AGE_VALUE"));
		wsprintfW(szBuf, TTW(39).c_str(), pkPet->GetAbil(AT_AGE));
		pkWnd->Text(szBuf);

		pkWnd = pkMiniPetInfo->GetControl(_T("LEVEL_VALUE"));
		wsprintfW(szBuf, _T("Lv.%d"), pkPet->GetAbil(AT_LEVEL));
		pkWnd->Text(szBuf);
	}
*/
}

void lwCallPetUI()
{
	XUI::CXUI_Wnd *form = XUIMgr.Get(_T("CharInfo"));
	if( form == NULL )
	{
		return;
	}

	XUI::CXUI_CheckButton *pBtn = (XUI::CXUI_CheckButton*)form->GetControl(_T("CharMenu"));

	if( pBtn->Check() )
	{
		pBtn->ClickLock(false);
		pBtn->Check(false);
	}

	pBtn = (XUI::CXUI_CheckButton*)form->GetControl(_T("PetMenu"));
	if( !pBtn->Check() )
	{
		pBtn->Check(true);
	}
	pBtn->ClickLock(true);
}

//void lwCallCharUI()
//{
//	XUI::CXUI_Wnd *form = XUIMgr.Get(_T("CharInfo"));
//	if( form == NULL )
//		return;
///*
//	XUI::CXUI_CheckButton *pBtn = (XUI::CXUI_CheckButton*)form->GetControl(_T("PetMenu"));
//
//	if( pBtn->Check() )
//	{
//		pBtn->ClickLock(false);
//		pBtn->Check(false);
//	}
//
//	pBtn = (XUI::CXUI_CheckButton*)form->GetControl(_T("CharMenu"));
//	if( !pBtn->Check() )
//	{
//		pBtn->Check(true);
//	}
//	pBtn->ClickLock(true);
//	*/
//}

void lwCallMessageBox(std::wstring const &wstrMessage, std::wstring const &wstrTitle, DWORD const dwMessageBoxFlag)
{
	XUI::CXUI_MsgBox* pMsgBox = dynamic_cast<XUI::CXUI_MsgBox*>(XUIMgr.Call(_T("OK_CANCEL_BOX"), true));
	
	if(pMsgBox)
	{
		XUI::CXUI_Wnd *pkFrm = pMsgBox->GetControl(_T("FRM_MESSAGE"));
		if(NULL != pkFrm)
		{
			pkFrm->Text(wstrMessage);
		}
		//pMsgBox->Text(wstrMessage);
		XUI::PgMessageBoxInfo Info = pMsgBox->BoxInfo();
	}
//	PgMessageBoxInfo *pData = (PgMessageBoxInfo*)pMsgDlg->CustomData();
//	pData->iBoxType = MBT_PARTY_JOIN;
}

void lwCallInputBox(std::wstring const &rkMessage, int const iCommandType, bool const bModal, int iLimitLength)
{
	if( !iLimitLength )
	{
		return;
	}

	XUI::CXUI_Wnd* pkInputBox = XUIMgr.Call(_T("SFRM_COMMON_EDIT"), bModal);
	if( pkInputBox )
	{
		pkInputBox->SetCustomData(&iCommandType, sizeof(iCommandType));

		XUI::CXUI_Wnd* pkMessageWnd = pkInputBox->GetControl(_T("FRM_MESSAGE"));
		if( pkMessageWnd )
		{
			pkMessageWnd->Text(rkMessage);
		}
		//

		XUI::CXUI_Edit* pkEditWnd = dynamic_cast<XUI::CXUI_Edit*>(pkInputBox->GetControl(_T("EDT_CHARNAME")));
		if( pkEditWnd )
		{
			pkEditWnd->LimitLength(iLimitLength);
			XUIMgr.ClearEditFocus();
			pkEditWnd->SetEditFocus(false);//이게 마지막
		}

		XUI::CXUI_Wnd* pkLineImg1 = pkInputBox->GetControl(_T("IMG_LINE1"));
		XUI::CXUI_Wnd* pkLineImg2 = pkInputBox->GetControl(_T("IMG_LINE2"));
		if( pkMessageWnd && pkLineImg1 && pkLineImg2 )
		{
			int const iBaseY = 14;
			int const iYSize = Pg2DString::CalculateOnlySize((CXUI_Style_String)pkMessageWnd->StyleText()).y+2;
			POINT3I const kLoc1 = pkLineImg1->Location();
			POINT3I const kLoc2 = pkLineImg2->Location();

			pkLineImg1->Location(kLoc1.x, iBaseY + iYSize);
			pkLineImg2->Location(kLoc2.x, iBaseY + iYSize + 1);
		}
	}
}

void	lwUIWnd::SetCanDrag(bool bCanDrag)
{ 
	if (self)
	{
		self->CanDrag(bCanDrag);
	}
	else
	{
		NILOG(PGLOG_WARNING, "[SetCanDrag] Can't find self\n");
	}
}

bool	lwUIWnd::GetCanDrag()
{
	if (self)
	{
		return	self->CanDrag();
	}
	return false;
}

void lwUIWnd::OnOk()
{
	if( !self )
	{
		return;
	}
	XUI::CXUI_Wnd * pParent = self->Parent();
	XUI::CXUI_MsgBox* pMsgBox = dynamic_cast<XUI::CXUI_MsgBox*>(pParent);
	if(pMsgBox)
	{
		XUI::PgMessageBoxInfo Info = pMsgBox->BoxInfo();
		switch(Info.iBoxType)
		{
		case MBT_PARTY_JOIN:
			{
				if( PgClientPartyUtil::IsCanPartyArea(true) )//파티가 가능한 지역인가?
				{
					XUI::CXUI_Wnd* pkMain= XUIMgr.Get(_T("FRM_CASH_SHOP"));
					if( pkMain )
					{
						if( true == pkMain->Visible() )
						{
							PgClientPartyUtil::Net_C_N_ANS_JOIN_PARTY(false, Info.kOrderGuid);

							SChatLog kChatLog(CT_EVENT);
							g_kChatMgrClient.AddMessage(401366, kChatLog, true);
						}
					}
					else
					{
					PgClientPartyUtil::Net_C_N_ANS_JOIN_PARTY(true, Info.kOrderGuid);
				}
				}
			}break;
		case MBT_PARTYFIND_JOIN:
			{
				//if( PgClientPartyUtil::IsCanPartyArea(true) )//파티가 가능한 지역인가?
				//{
					PgClientPartyUtil::Net_C_N_ANS_JOIN_PARTYFIND(true, Info.kOrderGuid);
				//}				
			}break;
		case MBT_PARTY_EXIT:
			{
				if( PgClientPartyUtil::IsCanPartyArea(false) )//파티가 가능한 지역인가?
				{
					PgClientPartyUtil::Send_Request_Leave_Party();
				}
			}break;
		case MBT_GUILD_JOIN:
			{
				g_kGuildMgr.ReqJoinAnswer(Info.kOrderGuid, true);
			}break;
		case MBT_CONFIRM_EXIT:
			{
				DisconnectFromServer(true);
			}break;
		case MBT_CONFIRM_BUY_SKILL:
			{
				int iSkillNo = 0;
				if( pMsgBox->GetCustomData(&iSkillNo, sizeof(iSkillNo)) )
				{
					g_kGuildMgr.ReqGuildAddSkill(iSkillNo);
				}
			}break;
		case MBT_CONFIRM_SWEETHEART_QUEST:
			{
				int iQuestID = 0;
				if( pMsgBox->GetCustomData(&iQuestID, sizeof(iQuestID)) )
				{
					lwCouple::Send_AnsSweetHeartQuest(Info.kOrderGuid, true, iQuestID);
				}
			}break;
		case MBT_CONFIRM_SWEETHEART_COMPLETE:
			{
				int iQuestID = 0;
				if( pMsgBox->GetCustomData(&iQuestID, sizeof(iQuestID)) )
				{
					lwCouple::Send_AnsSweetHeartComplete(Info.kOrderGuid, true, iQuestID);
				}
			}break;
		case MBT_CONFIRM_COUPLE:
			{
				lwCouple::Send_AnsCouple(Info.kOrderGuid, true, false);
			}break;
		case MBT_CONFIRM_INSTANCE_COUPLE:
			{
				lwCouple::Send_AnsCouple(Info.kOrderGuid, true, true);
			}break;
		case MBT_CONFIRM_BREAK_COUPLE:
			{
				lwCouple::Send_BreakCouple();
			}break;
		case MBT_CONFIRM_WARP_COUPLE:
			{
				lwCouple::Send_WarpCouple();
			}break;
		case MBT_CONFIRM_PAYMENT_MAIL:
			{
				int iIndex = 0;
				pParent->GetCustomData(&iIndex, sizeof(iIndex));
				g_kMailMgr.GetAnnex(iIndex);
			}break;
		case MBT_CONFIRM_GET_ANNEX_ALL:
			{
				g_kMailMgr.GetAnnexAll();
			}break;
		case MBT_CONFIRM_DELETE_MAIL:
			{
				int iCustomData = 0;
				pParent->GetCustomData(&iCustomData, sizeof(iCustomData));
				g_kMailMgr.DeleteMailAt(iCustomData);
				lwCloseUI("SFRM_LETTER");
			}break;
		case MBT_CONFIRM_RETURN_MAIL_ALL:
			{
				lwReturnMail();
			}break;
		case MBT_CONFIRM_RETURN_MAIL:
			{
				int iCustomData = 0;
				pParent->GetCustomData(&iCustomData, sizeof(iCustomData));
				g_kMailMgr.ReturnMailAt(iCustomData);
				lwCloseUI("SFRM_LETTER");
			}break;
		case MBT_MARKET_REQUEST_BUY:
			{
				//g_kMarketMgr.RequestToMarketArticleBuy();
			}break;
		case MBT_MARKET_REQUEST_DELETE:
			{
				//g_kMarketMgr.RequestToMyRecordDelete();
			}break;
		case MBT_MARKET_REQUEST_REGIST:
			{
				//g_kMarketMgr.RequestToMyRegistArticle();
			}break;
		case MBT_MARKET_REQUEST_CANCEL:
			{
				//g_kMarketMgr.RequestToMyRemoveArticle();
			}break;
		case MBT_CONFIRM_GUILD_OWNER_CHANGE:
			{
				g_kGuildMgr.ChangeOwner(Info.kOrderGuid);
			}break;
		case MBT_CONFIRM_DROP_SYSTEM_INVEN:
			{
				int iSetNo = 0;
				if( pMsgBox->GetCustomData(&iSetNo, sizeof(iSetNo)) )
				{
					g_kSystemInven.SendDropSysInven( (size_t)iSetNo );
				}
			}break;
		case MBT_FRAN_CHANGE:
			{
				__int64 iExp = GetNowFran();
				if( iExp )
				{
					__int64 const iAbsVal = static_cast<__int64>((iExp * FRAN_REWORD_MONEY)/100);

					CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
					if(pkUnit)
					{
						if( pkUnit->GetAbil64(AT_MONEY) < iAbsVal )
						{
							lwAddWarnDataTT(700036);
							break;
						}
						BM::Stream kPacket(PT_C_M_REQ_REWORD_FRAN_EXP);
						kPacket.Push( Info.kOrderGuid );
						NETWORK_SEND(kPacket)
					}
				}
				else
				{
					lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(550010)), true);
				}
			}break;
		case MBT_FRIEND_ADD:
			{
				SFriendItem kFriendItem;
				if( g_kFriendMgr.Friend_Find_ByGuid(Info.kOrderGuid, kFriendItem) )
				{
					g_kFriendMgr.SendFriend_Command(FCT_ADD_ACCEPT, kFriendItem);
				}
				else
				{
					NILOG(PGLOG_ERROR, "Can't find friend\n"); assert(0);
				}
			}break;
		case MBT_CONFIRM_GUILD_KICK_USER:
			{
				BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_M_Kick);
				kPacket.Push(Info.kOrderGuid);
				NETWORK_SEND(kPacket)
			}break;
		case MBT_CONFIRM_GUILD_LEAVE:
			{
				BM::Stream kPacket(PT_C_N_REQ_GUILD_COMMAND, (BYTE)GC_Leave);
				NETWORK_SEND(kPacket)
			}break;
		case MBT_CONFIRM_ENTERTUTORIAL:
			{
				bool const bPassTutorial = false;
				if (LoginUtil::CanShowCharacterMovie())
				{
					g_kMovieMgr.SetMovieFinishedCallBack(&LoginUtil::SendSelectCharacterAfterMovieFinished);
					LoginUtil::g_kSelectedCharacterGUID = Info.kOrderGuid;
					g_kMovieMgr.PlayOpeningMoveFromGUID(Info.kOrderGuid);
				}
				else
				{
					LoginUtil::SendSelectCharacter(Info.kOrderGuid, bPassTutorial);
				}
			}break;
		case MBT_MODIFY_TO_NOMALMARKET:
		case MBT_COMMON_YESNO_TO_PACKET:
		case MBT_STATIC_CASHITEM_BUY_TO_PACKET:
		case MBT_INTERACTIVE_EMOTION_REQUEST_ACCEPT:
		case MBT_NONE_CASH_GACHA_ROULLET:
			{
				lwUIWnd	UIMsgBox(pMsgBox);
				lwPacket kPacket = UIMsgBox.GetCustomDataAsPacket();
				if( kPacket.IsNil() )
				{
					break;
				}

				PACKET_ID_TYPE wType = 0;
				kPacket()->Pop(wType);
				kPacket()->RdPos(0);//패킷 타입만 뽑자

				NETWORK_SEND(*kPacket())

				if( MBT_STATIC_CASHITEM_BUY_TO_PACKET == Info.iBoxType )
				{
					lua_tinker::call<void>("OnCloseStaticCashItemBuy");
				}
			}break;
		case MBT_COMMON_YESNO_PARTYMEMBER:
			{
				BM::GUID &rkCharGuid = Info.kOrderGuid;

				if( BM::GUID::NullData() != rkCharGuid )
				{
					int iItemNo = 98000130;
					SItemPos kItemPos;
					PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
					if(!pkPlayer){return;}
					PgInventory *pInv = pkPlayer->GetInven();
					if(pInv)
					{
						if (S_OK == pInv->GetFirstItem(iItemNo, kItemPos))
						{
							GET_DEF(CItemDefMgr, kItemDefMgr);
							const CItemDef *pkDef = kItemDefMgr.GetDef(iItemNo);
							if( pkDef )
							{
								if( UICT_MOVETOPARTYMEMBER == pkDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
								{
									SPT_C_M_REQ_MOVETOPARTYMEMBER kStruct;
									kStruct.kMemberGuid = rkCharGuid;
									kStruct.kItemPos = kItemPos;
									BM::Stream kPacket;
									kStruct.WriteToPacket(kPacket);
									NETWORK_SEND(kPacket)
								}
							}
						}
						else
						{
							g_kChatMgrClient.LogMsgBox( 401301 );
						}
					}
				}
			}break;
		case MBT_COMMON_YESNO_PARTYMASTERGROUND:
			{
				__int64 i64Price = g_kParty.ToPartyMasterWarpCost();
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pkPlayer ) { return; }
				__int64 i64HaveMoney = pkPlayer->GetAbil64(AT_MONEY);
				if( i64HaveMoney < i64Price )	// 소지금액이 부족함.
				{
					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddMessage(968, kChatLog, true);
					return;
				}
				BM::GUID& rkCharGuid = Info.kOrderGuid;

				if( BM::GUID::NullData() != rkCharGuid )
				{
					SPT_C_M_REQ_MOVETOPARTYMASTERGROUND kStruct;
					kStruct.kMemberGuid = rkCharGuid;
					BM::Stream kPacket;
					kStruct.WriteToPacket(kPacket);
					kPacket.Push(g_kParty.PartyMasterTransTowerGuid());
					kPacket.Push(g_kParty.PartyMasterTransTowerKey());
					NETWORK_SEND(kPacket);
				}
			}break;
		case MBT_ITEM_ACTION_YESNO_TO_PACKET:
			{
				lwUIWnd	UIMsgBox(pMsgBox);
				lwPacket kPacket = UIMsgBox.GetCustomDataAsPacket();
				if( kPacket.IsNil() )
				{
					break;
				}
				kPacket.PushInt(lwGetServerElapsedTime32());
				kPacket.PushGuid(PgSafeInventory::kSafeInventoryNpc);

				NETWORK_SEND(*kPacket())
			}break;
		case MBT_CHANGE_GUILD_MARK1:
			{
				PgGuildMgrUtil::SendReqChangeGuildMark(pMsgBox);
			}break;
		case MBT_USER_PORTAL:
			{// 기억전송 카드 사용 확인시
				g_kMemTransCard.SendUseMemPortalReq();
			}break;
		case MBT_CONFIRM_SEAL:
			{// 아이템 봉인 확인을 했다면
				g_kSealProcess.SendPacket();
				lwCloseUI("SFRM_SEAL_PROCESS");
				g_kSealProcess.CallProcessingNoticeUI();
			}break;
		case MBT_CONFIRM_MONSTERCARD:
			{
				lwUIWnd	UIMsgBox(pMsgBox);
				lwPacket kPacket = UIMsgBox.GetCustomDataAsPacket();
				if( kPacket.IsNil() )
				{
					break;
				}
				NETWORK_SEND(*kPacket());
				lwCloseUI("SFRM_MONSTER_CARD");
			}break;
		case MBT_PLUS_INSURE_OKCANCEL:
			{
				lwUIItemPlusUpgrade	PlusUpgrade(NULL);
				PlusUpgrade.CallComfirmMessageBox();
			}break;
		case MBT_RARITY_INSURE_OKCANCEL:
			{
				lwUIItemRarityUpgrade RarityUpgrade(NULL);
				RarityUpgrade.CallComfirmMessageBox();
			}break;
		case MBT_CONFIRM_ACCEPT_SHAREQUEST:
			{
				int iQuestID = 0;
				if( pMsgBox->GetCustomData(&iQuestID, sizeof(iQuestID)) )
				{
					Quest::SendAnsShareQuest(Info.kOrderGuid, iQuestID, true);
				}
			}break;
		case MBT_ACCEPT_HOMEAUCTION_CANCEL:
			{
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if(!pkPlayer)
				{
					return;
				}
				PgInventory* pkInv = pkPlayer->GetInven();
				if(!pkInv)
				{
					return;
				}
				__int64 const& i64MyMoney = pkInv->Money();
				__int64 const iNeedGold = g_kHomeTownMgr.MyHomeFirstBiddingCost()/2;
				if(iNeedGold > i64MyMoney)
				{// 경매 취소시, 경매등록에 사용한 금액의 50%가 필요하지만 부족하다면
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 968, true);
					return;
				}
				lwUIWnd	UIMsgBox(pMsgBox);
				lwPacket kPacket = UIMsgBox.GetCustomDataAsPacket();
				if( kPacket.IsNil() )
				{
					break;
				}
				NETWORK_SEND(*kPacket())
				g_kHomeTownMgr.MyHomeFirstBiddingCost(0);
			}break;
		case MBT_CONFIRM_EXITGAME_AND_WEBPAGE:
			{
				g_klwPlayTime.RunInternetBrowser();
			}break;
		case MBT_CONFIRM_LEAVE_BATTLESQUARE:
			{
				BM::Stream kPacket(PT_C_M_REQ_BS_EXIT);
				NETWORK_SEND(kPacket);
			}break;
		case MBT_COMMON_YESNO_MYHOM_REPAIR:
			{
				lwHomeUI::OpenMyHomeRepairService();
			}break;
		case MBT_COMMON_YESNO_MYHOM_ENCHANT:
			{
				lwHomeUI::OpenMyHomeEnchantService();
			}break;
		case MBT_COMMON_YESNO_MYHOM_SOUL_GOLD:
			{
				lwHomeUI::OpenMyHomeSideJobSoulGold();
			}break;
		case MBT_COMMON_YESNO_MYHOM_SOUL_COUNT:
			{
				lwHomeUI::OpenMyHomeSideJobSoulCount();
			}break;
		case MBT_COMMON_YESNO_MYHOM_SOCKET_GOLD:
			{
				lwHomeUI::OpenMyHomeSideJobSocketGold();
			}break;
		case MBT_COMMON_YESNO_MYHOM_SOCKET_RATE:
			{
				lwHomeUI::OpenMyHomeSideJobSocketRate();
			}break;
		case MBT_COMMON_YESNO_MYHOM_ENCHANT_RATE:
			{
				lwHomeUI::OpenMyHomeSideJobEnchantRate();
			}break;
		case MBT_CONFIRM_CASHSHOP_BASKET_REG:
			{
				lwUIWnd	UIMsgBox(pMsgBox);
				lwPacket kPacket = UIMsgBox.GetCustomDataAsPacket();
				if( kPacket.IsNil() )
				{
					break;
				}

				bool bReg = kPacket.PopBool();
				int kArticleNo = kPacket.PopInt();
				if( bReg )
				{
					lwCashShop::RegShoppingBasketItem(kArticleNo);
				}
				else
				{
					int kSlotIDX = kPacket.PopInt();
					lwCashShop::UnRegShoppingBasketItem(kSlotIDX, kArticleNo);
				}

				XUI::CXUI_Wnd* pWnd = XUIMgr.Get(L"FRM_CASH_SHOP");
				if(!pWnd){ return; }
				XUI::CXUI_Wnd* pTmp = pWnd->GetControl(L"SFRM_INFO");
				if(!pTmp){ return; }
				pTmp = pTmp->GetControl(L"SFRM_CASH_TITLE");
				if(!pTmp){ return; }
				pTmp = pTmp->GetControl(L"BTN_TITLE_TAB2");
				if(!pTmp){ return; }
				
				lua_tinker::call<void,lwUIWnd,int>("SelectCashInvType", lwUIWnd(pTmp), 2);
			}break;
		case MBT_PARTY_LEAVE:
			{
				lua_tinker::call<void,char const*,lwGUID>("OnClick_Community_PartyPopup","LEAVE",lwGUID(Info.kOrderGuid));
			}break;
		case MBT_PARTY_KICKOUT:
			{
				lua_tinker::call<void,char const*,lwGUID>("OnClick_Community_PartyPopup","KICKOUT",lwGUID(Info.kOrderGuid));
			}break;
		case MBT__COMMON_YESNO_RARITY_BUILDUP:
			{
				lua_tinker::call<void>("Start_RarityBuildUp");
			}break;
		case MBT__COMMON_YESNO_RARITY_AMPLIFY:
			{
				lua_tinker::call<void>("Start_RarityAmplify");
			}break;
		case MBT_SKILLSET_SAVE:
			{
				XUIMgr.Close( L"SFRM_SKILLSET" );
			}break;
		case MBT_STRATEGYSKILL_CHANGE:
			{
				static BM::GUID const TYPE1("00000000-0000-0000-0000-000000000000");
				static BM::GUID const TYPE2("00000000-0000-0000-0000-000000000001");				

				if(TYPE2 == Info.kOrderGuid)
				{
					lwOnClickStrategySkill(ESTT_SECOND);
				}
				else
				{
					lwOnClickStrategySkill(ESTT_BASIC);
				}
			}break;
		case MBT_EXIT_SUPER_GROUND:
			{
				if( false == g_kEventScriptSystem.IsNowActivate() )
				{
					BM::Stream kPacket(PT_C_M_REQ_EXIT_SUPER_GROUND);
					NETWORK_SEND(kPacket);
				}
			}break;
		case MBT_EXIT_INSTANCE_DUNGEON:
			{
				if( false == g_kEventScriptSystem.IsNowActivate() )
				{
					Net_RecentMapMove();
				}
			}break;
		case MBT_MYHOME_CHAT_CLEAN:
			{
				lwHomeRenew::ExcuteMyhomeChatClean();
			}break;
		case MBT_MYHOME_CHAT_EXIT:
			{
				lwHomeRenew::ExcuteMyhomeChatExit();
			}break;		
		case MBT_CONFIRM_GUILD_APPLICATION_CANCEL:
			{//길드가입 신청을 취소하겠다.
				lwUIWnd	UIMsgBox(pMsgBox);
				lwPacket kPacket = UIMsgBox.GetCustomDataAsPacket();
				if( kPacket.IsNil() )
				{
					break;
				}
				NETWORK_SEND(*kPacket());
				g_kGuildMgr.InitApplicantState();
				g_kGuildMgr.WrapperUpdateEntranceOpenGuildUI();
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage(kChatLog, TTW(400488), true);
			}break;
		case MBT_CONFIRM_GUILD_ENTRANCE_FEE:
			{// 길드가입 비용을 지불하겠다.
				lwUIWnd	UIMsgBox(pMsgBox);
				lwPacket kPacket = UIMsgBox.GetCustomDataAsPacket();
				if( kPacket.IsNil() )
				{
					break;
				}
				
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if(!pkPlayer){return;}
				__int64 i64Money = pkPlayer->GetAbil64(AT_MONEY);
				if( i64Money < g_kGuildMgr.GetGuildApplicationState().i64GuildEntranceFee )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403078, true);
					return ;
				}
				NETWORK_SEND(*kPacket());
			}break;
		case MBT_DEFENCE_GUARDIAN_INSTALL:
			{
				int iGuardinaNo = 0;
				if( pMsgBox->GetCustomData(&iGuardinaNo, sizeof(iGuardinaNo)) )
				{
					g_kMissionMgr.GurdianDoAction(iGuardinaNo);
				}
			}break;
		case MBT_DEFENCE_GUARDIAN_UPGRADE:
			{
				int iGuardinaNo = 0;
				if( pMsgBox->GetCustomData(&iGuardinaNo, sizeof(iGuardinaNo)) )
				{
					g_kMissionMgr.GurdianUpgradeDoAction(iGuardinaNo);
				}
			}break;
		case MBT_DEFENCE_GUARDIAN_REMOVE:
			{
				g_kMissionMgr.GurdianRemoveDoAction();
			}break;
		case MBT_CONFIRM_ENTER_VENDOR:
			{
				g_kVendorMgr.RequestVendorEnter();
			}break;
		case MBT_CONFIRM_LEAVE_VENDOR:
			{
				g_kVendorMgr.RequestVendorExit();
			}break;
        case MBT_EMPORIABATTLETHROW:
            {
                lwGuild::EmporiaGiveUp();
            }break;
		case MBT_NONE_COSTUME_ROULLET:
			{
				lwUIWnd	UIMsgBox(pMsgBox);
				lwPacket kPacket = UIMsgBox.GetCustomDataAsPacket();
				if( kPacket.IsNil() )
				{
					break;
				}
				NETWORK_SEND(*kPacket())
			}break;
		case MBT_CONFIRM_SOUL_TRANSITION_HAVE_RARITY:
			{
				if( !lwSoulTransfer::lwUseInsuranceItem() )
				{
					lwCallCommonMsgYesNoBox(MB(TTW(799877)), lwPacket(), true, MBT_CONFIRM_SOUL_TRANSITION_NOT_INSURANCE);
				}
				else
				{
					if( lwSoulTransfer::lwIsEndSoulTransProgress() )
					{
						lwSoulTransfer::lwStartSoulTransProgress(4.0f);
					}
				}
			}break;
		case MBT_CONFIRM_SOUL_TRANSITION_NOT_INSURANCE:
			{
				if( lwSoulTransfer::lwIsEndSoulTransProgress() )
				{
					lwSoulTransfer::lwStartSoulTransProgress(4.0f);
				}
			}break;
		case MBT_ENCHANT_SHIFT_START:
			{
				g_kEnchantShift.DoStartMixing();
			}break;
		case MBT_ENCHANT_SHIFT_DESTORY_INSURANCE:
			{
				g_kEnchantShift.CheckDestoryInurance(false);
				lua_tinker::call<void>("StartMixing");
			}break;
		case MBT_EXTRACT_ELEMENT:
			{
				int iItemPos_y = 0;
				if( pMsgBox->GetCustomData(&iItemPos_y, sizeof(iItemPos_y)) )
				{
					SItemPos kItemPos(IT_ETC, iItemPos_y);
					BM::Stream kPacket(PT_C_M_REQ_ELEMENT_EXTRACT);
					kPacket.Push(kItemPos);
					NETWORK_SEND(kPacket);
				}
			}break;
		case MBT_EXPEDITION_DESTROY:
			{
				PgClientExpeditionUtil::Send_C_N_REQ_DISPERSE_EXPEDITION();
			}break;
		case MBT_EXPEDITION_LEAVE:
			{
				PgClientExpeditionUtil::Send_C_N_REQ_LEAVE_EXPEDITION();
			}break;
		case MBT_EXPEDITION_JOIN:
			{
				if( PgClientPartyUtil::IsCanPartyArea(true) ) //파티가 가능한 지역인가?
				{
					PgClientExpeditionUtil::Send_C_N_ANS_JOIN_EXPEDITION(true, Info.kOrderGuid);
				}
			}break;
		case MBT_EXPEDITION_INVITE:
			{
				if( PgClientPartyUtil::IsCanPartyArea(true) ) //파티가 가능한 지역인가?
				{
					PgClientExpeditionUtil::Send_C_N_ANS_INVITE_EXPEDITION(true, Info.kOrderGuid);
				}
			}break;
		case MBT_EXPEDITION_COMPLETE_EXIT:
			{ // 원정대 결과창에서 나가기 버튼을 눌렀을 때
				g_kExpeditionComplete.SetExitButtonDown(true);
				lwCMP_STATE_CHANGE(14);
			}break;
		case MBT_CONFIRM_LEAVE_EXPEDITION_LOBBY:
			{
				PgActor * pActor = g_kPilotMan.GetPlayerActor();
				if( pActor )
				{
					if( g_kExpedition.IsExpeditionMaster(pActor->GetGuid()) )		// 원정대장이면
					{
						PgClientExpeditionUtil::Send_C_N_REQ_DISPERSE_EXPEDITION();	// 해산요청
					}
					else
					{
						PgClientExpeditionUtil::Send_C_N_REQ_LEAVE_EXPEDITION();	// 아니면 탈퇴요청
					}
					pActor->SetLeaveExpedition(true);								// 그리고 포탈 동작
					PgTrigger * pTrigger = pActor->GetCurrentTrigger();
					if( pTrigger && pActor->GetIsInTrigger() )
					{
						pTrigger->OnAction(pActor);
					}
				}
			}break;
		case MBT_EXPEDITION_CONFIRM_ENTER:
			{
				BM::Stream Packet(PT_C_M_REQ_NPC_ENTER_EXPEDITION);
				Packet.Push(Info.kOrderGuid);
				NETWORK_SEND(Packet);
			}break;
		case MBT_EXPEDITION_KICK_MEMBER:
			{
				PgClientExpeditionUtil::Send_C_N_REQ_KICKOUT_EXPEDITION(Info.kOrderGuid);
			}break;
		case MBT_EXPEDITION_PVPMODE_ENTER:
			{
				PgActor * pActor = g_kPilotMan.GetPlayerActor();
				if( pActor )
				{
					if( g_kExpedition.IsExpeditionMaster(pActor->GetGuid()) )		// 원정대장이면
					{
						PgClientExpeditionUtil::Send_C_N_REQ_DISPERSE_EXPEDITION();	// 해산요청
					}
					else
					{
						PgClientExpeditionUtil::Send_C_N_REQ_LEAVE_EXPEDITION();	// 아니면 탈퇴요청
					}
					if( UIMacroCheck::lwIsOnMacroCheck() )
					{
						break;
					}
					else
					{
						lua_tinker::call<void>("OnClickPvP");
					}
				}
			}break;
		case MBT_CHANGE_ANTIQUE:
			{
				XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"SFRM_COLLECT_ANTIQUE");
				if( pWnd )
				{
					lwDefenceMode::lwClickChangeAntiqueButton(lwUIWnd(pWnd), true);
				}
			}break;
		case MBT_EXCHANGE_ACCUMPOINT_TO_STRATEGICPOINT:
			{
				BM::Stream Packet(PT_C_M_REQ_EXCHANGE_ACCUMPOINT_TO_STRATEGICPOINT);
				NETWORK_SEND(Packet);
			}break;
		case MBT_EXCHANGE_GEMSTORE:
			{
				XUI::CXUI_Wnd * pWnd = NULL;
				for( int FormCount = 0; FormCount < ExchangeFormMaxSize; ++FormCount )
				{
					pWnd = XUIMgr.Get( szExchangeUI[FormCount].szFrom );
					if( pWnd )
					{
						lwDefenceMode::lwClickChangeItemButton(lwUIWnd(pWnd), true, szExchangeUI[FormCount].Menu);
						break;
					}
				}
			}break;
		case MBT_KILL_MONSTER_BY_POINT_COPY:
			{
				BM::Stream Packet(PT_C_M_REQ_KILL_MONSTER_BY_ACCUMPOINT);
				NETWORK_SEND(Packet);
			}break;
		case MBT_CREATE_JUMPING_CHAR_EVENT:
			{
				int const ijumpingCreateCount = g_kPilotMan.GetJumpingCreateCharCount();
				g_kPilotMan.UseJumpingEvent(true);

				if( 1==ijumpingCreateCount )
				{
					//첫번째 캐릭터의 Class로 세팅
					g_kPilotMan.SetJumpingSelectIdx(1);
					lua_tinker::call<void, bool>("Send_PT_C_S_REQ_CREATE_CHARACTER", false);
				}
				else
				{
					if( XUI::CXUI_Wnd * pkTopWnd = XUIMgr.Call(L"FRM_JUMPING_CLASS_MENU", true) )
					{
						if( XUI::CXUI_List *pkMenuListWnd = dynamic_cast<XUI::CXUI_List*>(pkTopWnd->GetControl(L"LIST_COMMAND")) )
						{
							pkMenuListWnd->ClearList();

							int iIndex = 1;
							while(iIndex <= ijumpingCreateCount)
							{
								lwWString kText(TTW(600103));
								kText.ReplaceStr("#CLASS#", MB(TTW(30000+g_kPilotMan.GetCreateJumpingCharClass(iIndex))) );
								kText.ReplaceInt("#LEVEL#", g_kPilotMan.GetCreateJumpingCharLevel(iIndex));
								lua_tinker::call<void, lwUIWnd, lwWString, int, lwGUID, bool>("AddJumpingClassMenuItem", pkMenuListWnd, kText, iIndex, lwGUID(NULL), false);
								++iIndex;
							}

							lua_tinker::call<void, lwUIWnd, lwWString, int, lwGUID, bool>("AddJumpingClassMenuItem", pkMenuListWnd, lwWString(TTW(400534)), iIndex, lwGUID(NULL), false);	//취소
						}
					}
				}
			}break;
		case MBT_WM_CLOSE:
			{
				g_bWM_CLOSE = true;
				if(g_pkApp)
				{
					NiAppWindow* pkWin = g_pkApp->GetAppWindow();
					if(pkWin)
					{
						PostMessage(pkWin->GetWindowReference(), WM_CLOSE, 0, 0);
					}
					//g_pkApp->QuitApplication();
				}
			}break;
		case MBT_CONFIRM_CONSUME_ITEM_PARTY_BREAK_IN:
			{
				int ChannelNo = -1;
				pMsgBox->GetCustomData(&ChannelNo, sizeof(ChannelNo));
				BM::Stream kPacket(PT_C_M_REQ_INDUN_PARTY_ENTER);
				kPacket.Push((short)ChannelNo);
				kPacket.Push(Info.kOrderGuid);
				NETWORK_SEND(kPacket)
			}break;
		default:
			{
			}break;
		}
	}
//	PgMessageBoxInfo *pData = (PgMessageBoxInfo*)self->CustomData();
//	PgClientParty::Send_Request_Join_Party(pData->kOrderGuid);
}

void lwUIWnd::OnCancel()
{
	if (NULL == self)
	{
		return;
	}
	XUI::CXUI_Wnd * pParent = self->Parent();
	XUI::CXUI_MsgBox* pMsgBox = dynamic_cast<XUI::CXUI_MsgBox*>(pParent);
	
	if(pMsgBox)
	{
		XUI::PgMessageBoxInfo Info = pMsgBox->BoxInfo();
		switch(Info.iBoxType)
		{
		case MBT_PARTY_JOIN:
			{
				PgClientPartyUtil::Net_C_N_ANS_JOIN_PARTY(false, Info.kOrderGuid);
			}break;
		case MBT_PARTYFIND_JOIN:
			{
				PgClientPartyUtil::Net_C_N_ANS_JOIN_PARTYFIND(false, Info.kOrderGuid);
			}break;
		case MBT_GUILD_JOIN:
			{
				g_kGuildMgr.ReqJoinAnswer(Info.kOrderGuid, false);
			}break;
		case MBT_CONFIRM_SWEETHEART_QUEST:
			{
				lwCouple::Send_AnsSweetHeartQuest(Info.kOrderGuid, false, 0);
			}break;
		case MBT_CONFIRM_SWEETHEART_COMPLETE:
			{
				lwCouple::Send_AnsSweetHeartComplete(Info.kOrderGuid, false, 0);
			}break;
		case MBT_CONFIRM_COUPLE:
			{
				lwCouple::Send_AnsCouple(Info.kOrderGuid, false, false);
			}break;
		case MBT_CONFIRM_INSTANCE_COUPLE:
			{
				lwCouple::Send_AnsCouple(Info.kOrderGuid, false, true);
			}break;
		case MBT_FRIEND_ADD:
			{
				SFriendItem kFriendItem;
				if( g_kFriendMgr.Friend_Find_ByGuid(Info.kOrderGuid, kFriendItem) )
				{
					g_kFriendMgr.SendFriend_Command(FCT_ADD_REJECT, kFriendItem);
				}
				else
				{
					NILOG(PGLOG_ERROR, "Can't find friend\n"); assert(0);
				}
			}break;
		case MBT_CONFIRM_ENTERTUTORIAL:
			{
				bool const bPassTutorial = true;
				LoginUtil::SendSelectCharacter(Info.kOrderGuid, bPassTutorial);

				// 헬프 옵션을 끈다.
				int const iValue = 1;
				g_kGlobalOption.SetConfig(XML_ELEMENT_ETC, std::string("OFF_HELP"), iValue, NULL);
				g_kGlobalOption.ApplyConfig();
				g_kGlobalOption.Save();
			}break;
		case MBT_CHANGE_GUILD_MARK1:
			{
				g_kChatMgrClient.LogMsgBox(400856);
			}break;
		case MBT_COMMON_YESNO_TO_PACKET:
			{
				size_t const iCustomSize = self->GetCustomDataSize();
				if( iCustomSize )
				{
					std::wstring kText;
					kText.resize(iCustomSize / sizeof(std::wstring::value_type));
					self->GetCustomData(&kText.at(0), iCustomSize);
					lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(kText), true);
				}
			}break;
		case MBT_STATIC_CASHITEM_BUY_TO_PACKET:
			{
				if( MBT_STATIC_CASHITEM_BUY_TO_PACKET == Info.iBoxType )
				{
					lua_tinker::call<void>("OnCloseStaticCashItemBuy");
				}
			}break;
		case MBT_MODIFY_TO_NOMALMARKET:
			{
				lwMarket::lwRevertMarketGrade();
			}break;
		case MBT_CONFIRM_ACCEPT_SHAREQUEST:
			{
				int iQuestID = 0;
				if( pMsgBox->GetCustomData(&iQuestID, sizeof(iQuestID)) )
				{
					Quest::SendAnsShareQuest(Info.kOrderGuid, iQuestID, false);
				}
			}break;
		case MBT_CONFIRM_GUILD_ENTRANCE_FEE:
			{//길드가입 비용 지불하기를 거절한다.(가입신청 취소)
				BM::Stream kPacket(PT_C_N_REQ_GUILD_ENTRANCE_CANCEL);
				NETWORK_SEND(kPacket);

				g_kGuildMgr.InitApplicantState();
				g_kGuildMgr.WrapperUpdateEntranceOpenGuildUI();
				SChatLog kChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage(kChatLog, TTW(400488), true);
			}break;
		case MBT_INTERACTIVE_EMOTION_REQUEST_ACCEPT:
			{
				lwUIWnd	UIMsgBox(pMsgBox);
				lwPacket klwPacket = UIMsgBox.GetCustomDataAsPacket();
				if( klwPacket.IsNil() )
				{
					break;
				}
				unsigned short usPacketType = klwPacket.PopShort();
				bool bTemp =klwPacket.PopBool();
				int iActionNo = klwPacket.PopInt();
				BM::GUID kReqGuid = (klwPacket.PopGuid())();
				BM::GUID kTargetGuid = (klwPacket.PopGuid())();

				BM::Stream kPacket(PT_C_M_ANS_INTERACTIVE_EMOTION_REQUEST);
				kPacket.Push(false);
				kPacket.Push(iActionNo);
				kPacket.Push(kReqGuid);
				kPacket.Push(kTargetGuid);
				
				NETWORK_SEND(kPacket);
			}break;
		case MBT_NONE_CASH_GACHA_ROULLET:
			{
				lua_tinker::call<void, bool>("Roulette_StartBtnDisable", false);
			}break;
		case MBT_EXPEDITION_JOIN:
			{
				PgClientExpeditionUtil::Send_C_N_ANS_JOIN_EXPEDITION(false, Info.kOrderGuid);
			}break;
		case MBT_EXPEDITION_INVITE:
			{
				PgClientExpeditionUtil::Send_C_N_ANS_INVITE_EXPEDITION(false, Info.kOrderGuid);
			}break;
		case MBT_CREATE_JUMPING_CHAR_EVENT:
			{
				lua_tinker::call<void, bool>("Send_PT_C_S_REQ_CREATE_CHARACTER", true);
			}break;
		case MBT_EXPEDITION_COMPLETE_EXIT:
		case MBT_CONFIRM_LEAVE_EXPEDITION_LOBBY:
		case MBT_EXPEDITION_CONFIRM_ENTER:
		case MBT_EXPEDITION_KICK_MEMBER:
		case MBT_EXPEDITION_PVPMODE_ENTER:
		case MBT_CHANGE_ANTIQUE:
		case MBT_EXCHANGE_ACCUMPOINT_TO_STRATEGICPOINT:
		case MBT_EXCHANGE_GEMSTORE:
		case MBT_KILL_MONSTER_BY_POINT_COPY:
		case MBT_EXPEDITION_DESTROY:
		case MBT_EXPEDITION_LEAVE:
		case MBT_CONFIRM_EXITGAME_AND_WEBPAGE:
		case MBT_CONFIRM_SEAL:		// 아이템 봉인 취소
		case MBT_USER_PORTAL:		// 기억전송 카드 사용 캔슬		
		case MBT_CONFIRM_EXIT:
		case MBT_CONFIRM_BREAK_COUPLE:
		case MBT_FRAN_CHANGE:
		case MBT_COMMON_YESNO_PARTYMEMBER:
		case MBT_CONFIRM_MONSTERCARD:
		case MBT_ACCEPT_HOMEAUCTION_CANCEL:
		case MBT_CONFIRM_GUILD_APPLICATION_CANCEL:		
		case MBT_CONFIRM_CONSUME_ITEM_PARTY_BREAK_IN:
		default:
			{
			}break;
		}
	}
}

void lwUIWnd::MsgBoxOwnerGuid(lwGUID const kGuid)
{
	XUI::CXUI_MsgBox* pMsgBox = dynamic_cast<XUI::CXUI_MsgBox*>(self);
	if(pMsgBox)
	{
		XUI::PgMessageBoxInfo Info = pMsgBox->BoxInfo();
		Info.kOrderGuid = kGuid.GetGUID();
		pMsgBox->BoxInfo(Info);
	}
}

void lwUIWnd::MsgBoxType(int const iType)
{
	XUI::CXUI_MsgBox* pMsgBox = dynamic_cast<XUI::CXUI_MsgBox*>(self);
	if(pMsgBox)
	{
		XUI::PgMessageBoxInfo Info = pMsgBox->BoxInfo();
		Info.iBoxType = iType;
		pMsgBox->BoxInfo(Info);
	}
}

void	lwUIWnd::SetIconKeyAndResNo(int iIconKey, int iIconResNo)
{
	CXUI_Icon* pkIconWnd = dynamic_cast<CXUI_Icon*>(self);
	if(pkIconWnd)
	{
		SIconInfo kIconInfo = pkIconWnd->IconInfo();

		kIconInfo.iIconKey = iIconKey;
		kIconInfo.iIconResNumber = iIconResNo;
		//NILOG(PGLOG_CONSOLE, "[lwUIWnd] SetIconKeyAndResNo, %#x %d, %d\n", self, iIconKey, iIconResNo);
		pkIconWnd->SetIconInfo(kIconInfo);
	}
}

void	lwUIWnd::SetIconKey(int iValue)
{
	CXUI_Icon* pkIconWnd = dynamic_cast<CXUI_Icon*>(self);
	if(pkIconWnd)
	{
		SIconInfo kIconInfo = pkIconWnd->IconInfo();

		kIconInfo.iIconKey = iValue;
		//NILOG(PGLOG_CONSOLE, "[lwUIWnd] SetIconKeyAndResNo, %#x %d\n", self, iValue);
		pkIconWnd->SetIconInfo(kIconInfo);
	}
}

void	lwUIWnd::SetIconResNo(int iValue)
{
	CXUI_Icon* pkIconWnd = dynamic_cast<CXUI_Icon*>(self);
	if(pkIconWnd)
	{
		SIconInfo kIconInfo = pkIconWnd->IconInfo();

		kIconInfo.iIconResNumber = iValue;
		//NILOG(PGLOG_CONSOLE, "[lwUIWnd] SetIconKeyAndResNo, %#x %d\n", self, iValue);
		pkIconWnd->SetIconInfo(kIconInfo);
	}
}

void	lwUIWnd::SetAlpha(float fValue)
{
	if(self)
	{
		self->Alpha(fValue);
	}
}

float	lwUIWnd::GetAlpha()
{
	if(self)
	{
		return self->Alpha();
	}
	return 0.0f;
}

void	lwUIWnd::SetMaxAlpha(float fValue)
{
	if(self)
	{
		self->AlphaMax(fValue);
	}
}

float	lwUIWnd::GetMaxAlpha()
{
	if(self)
	{
		return self->AlphaMax();
	}
	return 1.0f;
}

void	lwUIWnd::BarNow(int iValue)
{
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);
	if( pWnd )
	{
		pWnd->Now(iValue);
	}
}

void	lwUIWnd::BarMax(int iValue)
{
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);
	if( pWnd )
	{
		pWnd->Max(iValue);
	}
}

int	lwUIWnd::GetBarNow()
{
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);
	if( pWnd )
	{
		return pWnd->Now();
	}
	return 0;
}
int	lwUIWnd::GetBarMax()
{
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);
	if( pWnd )
	{
		return pWnd->Max();
	}
	return 0;
}

int lwUIWnd::GetBarCurrent()
{
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);
	if( pWnd )
	{
		return pWnd->Current();
	}
	return 0;	
}

void lwUIWnd::SetBarReverse( bool const bReverse )
{
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);
	if( pWnd )
	{
		pWnd->IsReverse( bReverse );
	}
}

void	lwUIWnd::SetUVIndex(int iIndex)
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[SetUVIndex] Can't find self\n");
		return;
	}
	SUVInfo kUVInfo= self->UVInfo();
	if(kUVInfo.Index != iIndex)
	{
		kUVInfo.Index = iIndex;
		self->UVInfo(kUVInfo);
		self->SetInvalidate();	
	}
}

int lwUIWnd::GetUVIndex()
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetUVIndex] Can't find self\n");
		return 0;
	}
	SUVInfo kUVInfo= self->UVInfo();
	return (int)kUVInfo.Index;
}

void lwUIWnd::SetMaxUVIndex(int const iU, int const iV)
{
	if(!self)
	{
		NILOG(PGLOG_WARNING, "[GetMaxUVIndex] Can't find self\n");
		return;
	}
	SUVInfo kUVInfo = self->UVInfo();
	kUVInfo.U = iU;
	kUVInfo.V = iV;
	self->UVInfo(kUVInfo);
}

int lwUIWnd::GetMaxUVIndex()
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetMaxUVIndex] Can't find self\n");
		return 0;
	}
	SUVInfo const &rkUVInfo = self->UVInfo();
	return rkUVInfo.U * rkUVInfo.V;
}

lwUIWnd lwUIWnd::GetControl(char const* szControlId)
{
	if (!self || !szControlId)
	{
		NILOG(PGLOG_WARNING, "[GetControl] Can't find self ID = %s\n", szControlId);
		return lwUIWnd(NULL, false);
	}

	XUI::CXUI_Wnd * pControl = self->GetControl(UNI(szControlId));
	if(pControl)
	{
		return lwUIWnd(pControl);
	}
	NILOG(PGLOG_WARNING, "[lwUIWnd::GetControl] Can't find ID=%s in %s\n", szControlId, MB(self->ID().c_str()));
	return lwUIWnd(NULL, false);
}

void lwUIWnd::UpWheal()
{
	if( !self )
	{
		NILOG(PGLOG_WARNING, "[GetControl] self is NULL\n");
		return;
	}
	self->UpWheal();
}

void lwUIWnd::DownWheal()
{
	if( !self )
	{
		NILOG(PGLOG_WARNING, "[GetControl] self is NULL\n");
		return;
	}
	self->DownWheal();
}

void	lwUIWnd::SetParent(lwUIWnd kParentWnd)
{
	if (self)
	{
		if (false == kParentWnd.IsNil())
		{
			kParentWnd()->AddControl(self);
		}
	}
	NILOG(PGLOG_WARNING, "[SetParent] Can't find self\n");
}

void lwUIWnd::SetInvalidate(bool bValue)
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[SetInvalidate] Can't find self\n");
		return;
	}
	self->SetInvalidate(bValue);
}
void lwUIWnd::Visible(bool bIsVisible)
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[Visible] Can't find self\n");
		return;
	}
	bool bUpdate = self->Visible() != bIsVisible;
	self->Visible(bIsVisible);
}
bool	lwUIWnd::IsVisible()
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[IsVisible] Can't find self\n");
		return false;
	}
	return	self->Visible();
}

lwUIWnd lwUIWnd::GetParent()
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetParent] Can't find self\n");
		return lwUIWnd(NULL);
	}
	return lwUIWnd(self->Parent());
}

lwUIWnd lwUIWnd::GetTopParent()
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetParent] Can't find self\n");
		return lwUIWnd(NULL);
	}

	return GetParentRecursive(self);
}

lwUIWnd lwUIWnd::GetParentRecursive(lwUIWnd kWnd)
{
	if( kWnd.GetParent().IsNil() == false )
	{
		return GetParentRecursive(kWnd.GetParent() );
	}

	return kWnd;
}

bool lwUIWnd::SetEditFocus(bool const bFocus)
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[SetEditFocus] Can't find self\n");
		return true;
	}
	return self->SetEditFocus(bFocus);
}

bool lwUIWnd::SetEditLimitLength(int const iLimitLength)
{
	CXUI_Edit* pkEditWnd = dynamic_cast<CXUI_Edit*>(self);
	if(pkEditWnd)
	{
//		return pkEditWnd->SetLimitLength(iLimitLength);
	}
	return false;
}

int lwUIWnd::GetEditLimitLength()
{
	CXUI_Edit* pkEditWnd = dynamic_cast<CXUI_Edit*>(self);
	if(pkEditWnd)
	{
		return pkEditWnd->LimitLength();
	}
	return 0;
}

bool lwUIWnd::SetEditText(char const *pszValue)
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[SetEditFocus] Can't find self Text = %s\n", pszValue);
		return false;
	}
	return SetEditTextW(lwWString(pszValue));
}

bool lwUIWnd::SetEditTextW(lwWString lwkEditText)
{
	CXUI_Edit* pkEditWnd = dynamic_cast<CXUI_Edit*>(self);
	if(pkEditWnd)
	{
		pkEditWnd->EditText(lwkEditText());
		return false;
	}
	return false;
}

bool lwUIWnd::SetStaticText(char const *pszValue)
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[SetStaticText] Can't find self text = %s\n", pszValue);
		return false;
	}
	if(pszValue)
	{
		self->Text(UNI(pszValue));
		//self->SetInvalidate();
		return true;
	}
	return false;
}
bool lwUIWnd::SetStaticTextW(lwWString wText)
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[SetStaticTextW] Can't find self text = %s\n", wText.GetStr());
		return false;
	}
	self->Text(wText());
	//self->SetInvalidate();
	return true;
}

lwWString lwUIWnd::GetStaticText()
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetStaticText] Can't find self\n");
		return lwWString("");
	}
	std::wstring const wstrValue = self->Text();
	return lwWString( wstrValue);
}

lwStyleString lwUIWnd::GetEditText_TextBlockApplied()
{
	CXUI_Edit* pkEditWnd = dynamic_cast<CXUI_Edit*>(self);

	if(pkEditWnd)
	{
		XUI::CXUI_Style_String kStyleString;
		if(pkEditWnd->GetEditText_TextBlockApplied(kStyleString))
		{
			return	lwStyleString(kStyleString);
		}
	}
	return lwStyleString();
}

lwWString lwUIWnd::GetEditText()
{
	CXUI_Edit* pkEditWnd = dynamic_cast<CXUI_Edit*>(self);

	if(pkEditWnd)
	{
		std::wstring const wstrText = pkEditWnd->EditText();
		return lwWString(wstrText.c_str(), wstrText.size());
	}
	return lwWString(std::wstring(_T("")));
}

void	lwUIWnd::SetFontFlag(int iFlag)
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[SetFontFlag] Can't find self\n");
		return;
	}
	self->FontFlag(iFlag);
}

int		lwUIWnd::GetFontFlag()
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetFontFlag] Can't find self\n");
		return 0;
	}
	return	self->FontFlag();
}

void	lwUIWnd::SetFontColor(unsigned int iColor)
{
	if (self)
	{
		self->FontColor((DWORD)iColor);
		return;
	}
	NILOG(PGLOG_WARNING, "[SetFontColor] Can't find self\n");
}

void	lwUIWnd::SetFontColorRGBA(int R,int G,int B,int A)
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[SetFontColorRGBA] Can't find self\n");
		return;
	}
	DWORD	dwColor = A<<24 | R<<16 | G<<8 | B;
	self->FontColor(dwColor);
}

template<typename T>
void	lwUIWnd::SetCustomData(T const kValue)
{
	if(self)
	{
		self->SetCustomData(&kValue, sizeof(T));
	}
}

template<typename T>
T	lwUIWnd::GetCustomData()const
{
	T kValue;
	if(self && self->GetCustomData(&kValue, sizeof(T)))
	{
		return kValue;
	}
	return T();
}

void lwUIWnd::SetCustomDataAsGuid( lwGUID kGuid )
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[SetCustomDataAsGuid] Can't find self\n");
		return;
	}
	self->SetCustomData(&(kGuid()),sizeof(kGuid()));
}

lwGUID lwUIWnd::GetCustomDataAsGuid()
{
	BM::GUID kTemp;
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetCustomDataAsGuid] Can't find self\n");
	}
	else
	{
		self->GetCustomData(&kTemp, sizeof(kTemp));
	}
	return lwGUID(kTemp);
}

void lwUIWnd::SetCustomDataAsSItem(PgBase_Item const & kItem)
{
	if (self)
	{
		BM::Stream kPacket;
		kItem.WriteToPacket(kPacket);
		self->SetCustomData(kPacket.Data());
	}
}

PgBase_Item lwUIWnd::GetCustomDataAsSItem()
{
	PgBase_Item kTemp;
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetCustomDataAsSItem] Can't find self\n");
	}
	else
	{
		BM::Stream kCustomData;
		self->GetCustomData(kCustomData.Data());
		kCustomData.PosAdjust();

		kTemp.ReadFromPacket(kCustomData);
	}
	return kTemp;
}

void lwUIWnd::SetCustomDataAsStr(char const* szStr)
{
	if( self 
	&&	szStr )
	{
		self->SetCustomData(UNI(szStr));
	}
}

lwWString lwUIWnd::GetCustomDataAsStr()
{
	if( self )
	{
		std::wstring kTemp;
		self->GetCustomData(kTemp);
		return lwWString(kTemp);
	}
	return lwWString(_T(""));
}

void lwUIWnd::SetCustomDataAsPacket(lwPacket kPacket)
{
	BM::Stream *pkPacket = kPacket();
	if ( self && pkPacket )
	{
		if(pkPacket->Data().size())
		{
			self->SetCustomData( &(pkPacket->Data().at(0)), pkPacket->Data().size() );
		}
	}
}

lwPacket lwUIWnd::GetCustomDataAsPacket()
{
	if ( self )
	{
		size_t const iCustomSize = self->GetCustomDataSize();
		if( iCustomSize )
		{
			lwPacket kPacket = lwNewPacket(0);
			kPacket()->Data().resize(iCustomSize);
			if ( self->GetCustomData( &(kPacket()->Data().at(0)), iCustomSize) )
			{
				kPacket()->PosAdjust();
				return kPacket;
			}
			kPacket.Release();
		}
	}
	return lwPacket(NULL);
}

void lwUIWnd::ClearCustomData()
{
	if ( self )
	{
		self->ClearCustomData();
	}
}

size_t lwUIWnd::GetCustomDataSize()const
{
	if ( self )
	{
		return self->GetCustomDataSize();
	}
	return 0;
}

void lwUIWnd::CopyCustomData( lwUIWnd kWnd )const
{
	if ( self && kWnd() )
	{
		std::vector<char> kVec;
		self->GetCustomData( kVec );
		kWnd()->SetCustomData( kVec );
	}
}

lwWString lwUIWnd::GetUniqueUIType() const
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetID] Can't find self\n");
		return lwWString("");
	}
	return	lwWString(self->UniqueExistType());
}

lwWString	lwUIWnd::GetID()
{
	if (!self)
	{
		NILOG(PGLOG_WARNING, "[GetID] Can't find self\n");
		return lwWString("");
	}
	return	lwWString(self->ID());
}

void	lwUIWnd::SetID(lwWString kNewID)
{
	std::wstring kNewIDStr = kNewID();
	self->ID(kNewIDStr);
}

bool lwUIWnd::InitMiniMap(char const* szMiniMapName, int const Type)
{
	if(!g_pkWorld)
	{
		return false;
	}
	if (!szMiniMapName)
	{
		return false;
	}

	if( (g_pkWorld->MapNo() == 9010502)
	||	(g_pkWorld->MapNo() == 9020201)
	||	(g_pkWorld->MapNo() == 9020601)
	||	(g_pkWorld->MapNo() == 9030401)
	||	(g_pkWorld->MapNo() == 9010701) )
	{
		return false;
	}

	if( (g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME))
	||	(g_pkWorld->IsHaveAttr(GATTR_FLAG_CHAOS_F)) 
	||	(g_pkWorld->IsHaveAttr(GATTR_FLAG_STATIC_DUNGEON)))
	{
		return false;
	}

	//	----------추가
	int iRealType = Type;
	if( Type == EMUT_FULLSIZE_VIEW )
	{
		iRealType = EMUT_ALWAYS_VIEW;
		self->Size(self->Width(), g_pkWorld->GetMiniMapHeight());
		XUI::CXUI_Wnd*	pParentWnd = self->Parent();
		if( pParentWnd )
		{
			pParentWnd->Size(pParentWnd->Width(), self->Height() + 6);
			XUI::CXUI_Wnd*	pTextWnd = pParentWnd->GetControl(L"FRM_CLICK_DESCRIPTION");
			if( pTextWnd )
			{
				pTextWnd->Location(pTextWnd->Location().x, pParentWnd->Height() - pTextWnd->Height());
			}
		}
	}
	g_kUIScene.InitMiniMap(szMiniMapName, self->Size(), self->TotalLocation(), iRealType);

	return true;
}

void lwUIWnd::DrawMiniMap(char const* szMiniMapName)
{
	if (!szMiniMapName)
	{
		return;
	}
	
	g_kUIScene.RenderMiniMap(szMiniMapName, self->TotalLocation());
}

void lwUIWnd::DrawMiniMapUI(char const* szMiniMapName, lwUIWnd kDummyWnd)
{
	if (!szMiniMapName)
	{
		return;
	}

	g_kUIScene.RenderMiniMapUI(szMiniMapName, kDummyWnd());	
}

void lwUIWnd::SetMiniMapTriggerIconUI(char const* szMiniMapName, lwUIWnd kDummyWnd)
{
	if (!szMiniMapName)
	{
		return;
	}

	g_kUIScene.SetMiniMapTriggerIconUI(szMiniMapName, kDummyWnd(), self->TotalLocation());
}

void lwUIWnd::ShowMiniMap(char const* szMiniMapName, bool bShow)
{
	if (!szMiniMapName)
	{
		return;
	}
	
	g_kUIScene.ShowMiniMap(szMiniMapName, bShow);
}

void lwUIWnd::CloseMiniMap(char const* szMiniMapName)
{
	if (!szMiniMapName)
	{
		return;
	}
	
	g_kUIScene.CloseMiniMap(szMiniMapName);

}

float lwUIWnd::GetZoomMiniMap(char const* szMiniMapName)
{
	if (!szMiniMapName)
	{
		return 0.0f;
	}
	PgMiniMapBase* pkMiniMap = g_kUIScene.GetMiniMapUI(szMiniMapName);
	if (pkMiniMap)
	{
		return pkMiniMap->ZoomFactor();
	}
	return 0.0f;
}

void lwUIWnd::ZoomMiniMap(char const* szMiniMapName, float fZoomFactor)
{
	if (!szMiniMapName)
	{
		return;
	}
	PgMiniMapBase* pkMiniMap = g_kUIScene.GetMiniMapUI(szMiniMapName);
	if (pkMiniMap)
	{
		pkMiniMap->Zoom(fZoomFactor);
	}
}

void lwUIWnd::ZoomMiniMapFixedFactor(char const* szMiniMapName, float fZoomFactor)
{
	if (!szMiniMapName)
	{
		return;
	}

	PgMiniMapBase* pkMiniMap = g_kUIScene.GetMiniMapUI(szMiniMapName);
	if (pkMiniMap)
	{
		float fFixedFactor = fZoomFactor - pkMiniMap->ZoomFactor();
		pkMiniMap->Zoom(fFixedFactor);

		PgAlwaysMiniMap* pkAlwaysMap = dynamic_cast<PgAlwaysMiniMap*>(pkMiniMap);
		if( pkAlwaysMap )
		{
			pkAlwaysMap->FixedSize(true);
		}
	}
}

void lwUIWnd::MouseOverMiniMap(char const* szMiniMapName, lwPoint2 &pt, char const* wndName)
{
	if( self->IsMouseOver() )
	{
		if( !szMiniMapName )
		{
			return;
		}
		PgAlwaysMiniMap* pkAlwaysMap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI(szMiniMapName));
		if( pkAlwaysMap )
		{
			pkAlwaysMap->OnMouseOver(self->TotalLocation(), pt(), wndName);
		}
	}
}

void lwUIWnd::MouseClickMiniMap(char const* szMiniMapName, lwPoint2 &pt)
{
	if( !szMiniMapName )
	{
		return;
	}
	PgAlwaysMiniMap* pkAlwaysMap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI(szMiniMapName));
	if( pkAlwaysMap )
	{
		pkAlwaysMap->OnMouseClick(self->TotalLocation(), pt());
	}
}

bool lwUIWnd::GetMiniMapOptionShowNPC(char const* szMiniMapName)
{
	if (!szMiniMapName)
	{
		return false;
	}

	PgAlwaysMiniMap* pkAlwaysMiniMap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI(szMiniMapName));
	if( !pkAlwaysMiniMap )
	{
		return false;
	}

	return pkAlwaysMiniMap->GetViewFlag(PgAlwaysMiniMap::EIT_NPC);
}

bool lwUIWnd::GetMiniMapOptionShowPartyMemeber(char const* szMiniMapName)
{
	if (!szMiniMapName)
	{
		return false;
	}

	PgAlwaysMiniMap* pkAlwaysMiniMap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI(szMiniMapName));
	if( !pkAlwaysMiniMap )
	{
		return false;
	}

	return pkAlwaysMiniMap->GetViewFlag(PgAlwaysMiniMap::EIT_PARTY);
}

bool lwUIWnd::GetMiniMapOptionShowGuildMemeber(char const* szMiniMapName)
{
	if (!szMiniMapName)
	{
		return false;
	}

	PgAlwaysMiniMap* pkAlwaysMiniMap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI(szMiniMapName));
	if( !pkAlwaysMiniMap )
	{
		return false;
	}

	return pkAlwaysMiniMap->GetViewFlag(PgAlwaysMiniMap::EIT_GUILD);
}

void lwUIWnd::SetMiniMapOptionShowNPC(char const* szMiniMapName, bool bShow)
{
	if (!szMiniMapName)
	{
		return;
	}

	PgAlwaysMiniMap* pkAlwaysMiniMap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI(szMiniMapName));
	if( pkAlwaysMiniMap )
	{
		pkAlwaysMiniMap->SetViewFlag(PgAlwaysMiniMap::EIT_NPC, bShow);
	}
}

void lwUIWnd::SetMiniMapOptionShowPartyMember(char const* szMiniMapName, bool bShow)
{
	if (!szMiniMapName)
	{
		return;
	}

	PgAlwaysMiniMap* pkAlwaysMiniMap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI(szMiniMapName));
	if( pkAlwaysMiniMap )
	{
		pkAlwaysMiniMap->SetViewFlag(PgAlwaysMiniMap::EIT_PARTY, bShow);
	}
}

void lwUIWnd::SetMiniMapOptionShowGuildMemeber(char const* szMiniMapName, bool bShow)
{
	if (!szMiniMapName)
	{
		return;
	}

	PgAlwaysMiniMap* pkAlwaysMiniMap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI(szMiniMapName));
	if( pkAlwaysMiniMap )
	{
		pkAlwaysMiniMap->SetViewFlag(PgAlwaysMiniMap::EIT_GUILD, bShow);
	}
}

void lwUIWnd::SetWndSize(char const* szMiniMapName, lwPoint2 pt)
{
	if (!szMiniMapName)
	{
		return;
	}

	//PgMiniMapUI* minimap = g_kUIScene.GetMiniMapUI(szMiniMapName);
	//if (minimap)
	//{
	//	minimap->SetWndSize(pt());
	//}
}

void lwUIWnd::CloneMiniMap(char const* szSrcMiniMap, char const* szDestMiniMap)
{
	if (!szSrcMiniMap || !szDestMiniMap)
	{
		return;
	}

	g_kUIScene.CloneMiniMap(szSrcMiniMap, szDestMiniMap, self->Size());
}


void lwUIWnd::InitWorldMap()
{
	if (g_kUIScene.GetWorldMapUI())
	{
		XUI::CXUI_Form* pkSelfForm = dynamic_cast<XUI::CXUI_Form*>(GetSelf());
		if (pkSelfForm)
		{
			g_kUIScene.GetWorldMapUI()->Initialize(pkSelfForm);
		}
	}
}

void lwUIWnd::TermWorldMap()
{
	if (g_kUIScene.GetWorldMapUI())
	{
		g_kUIScene.GetWorldMapUI()->Terminate();
	}
}

void lwUIWnd::DrawWorldMap()
{
	if (g_kUIScene.GetWorldMapUI())
	{
		g_kUIScene.GetWorldMapUI()->RenderWorldMap();
	}
}

void lwUIWnd::PulseWorldMap()
{
	if (g_kUIScene.GetWorldMapUI())
	{
		g_kUIScene.GetWorldMapUI()->PulseWorldMap();
	}
}

void lwUIWnd::InSideWorldMap()
{
	if (g_kUIScene.GetWorldMapUI())
	{
		g_kUIScene.GetWorldMapUI()->InSideWorldMap();
	}
}

void lwUIWnd::OutSideWorldMap()
{
	if (g_kUIScene.GetWorldMapUI())
	{
		g_kUIScene.GetWorldMapUI()->OutSideWorldMap();
	}
}

void lwUIWnd::MouseOverWorldMap(lwPoint2 &pt)
{
	if (g_kUIScene.GetWorldMapUI())
	{
		g_kUIScene.GetWorldMapUI()->MouseOverWorldMap(pt());
	}
}

void lwUIWnd::MouseClickWorldMap(lwPoint2 &pt)
{
	if (g_kUIScene.GetWorldMapUI())
	{
		g_kUIScene.GetWorldMapUI()->MouseClickWorldMap(pt());
	}
}

void lwUIWnd::InitWorldMapPopUp(lwUIWnd kIconState, lwUIWnd kFormState)
{
	if (!g_kUIScene.GetWorldMapPopUpUI())
	{
		PgWorldMapPopUpUI* pkPU = NiNew PgWorldMapPopUpUI();
		g_kUIScene.SetWorldMapPopUpUI(pkPU);
	}
	if (g_kUIScene.GetWorldMapPopUpUI())
	{
		XUI::CXUI_Form* pkSelfForm = dynamic_cast<XUI::CXUI_Form*>(GetSelf());
		if (pkSelfForm)
		{
			g_kUIScene.GetWorldMapPopUpUI()->Initialize(pkSelfForm, kIconState, kFormState);
		}
	}
}

void lwUIWnd::TermWorldMapPopUp()
{
	if (g_kUIScene.GetWorldMapUI())
	{
		g_kUIScene.GetWorldMapPopUpUI()->Terminate();
	}
}

bool lwDrawModel(lwUIWnd kWnd, char const* szGuid, float fRotate)
{
	BM::GUID kGuid(szGuid);//darby
	PgPilot* pkPilot = g_kPilotMan.FindPilot(kGuid);
	CXUI_Wnd* pkWnd = kWnd();
	if(pkPilot && pkWnd)
	{
		PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
		if(pkActor)
		{
			const std::string& kActorID = pkActor->GetID();
			bool bRet = g_kUIScene.RenderModel(kActorID.c_str(), pkWnd->Size(), pkWnd->TotalLocation(), pkActor->GetActorManager(), pkActor->GetUIModelUpdate());
			if(bRet)
			{
				PgUIModel *pkUIModel = g_kUIScene.GetRenderModel(kActorID.c_str());
				if(pkUIModel)
				{
					//pkUIModel->SetTargetAnimation(1000001);
					//fRotate += 0.5f;
					//if(360.f < fRotate) fRotate = 0.f;
//					pkUIModel->Rotate(fRotate);
				}
			}
			return bRet;
		}
	}
	return false;
}

void lwUIWnd::DrawPetModel()
{
	PgPilot *pkPetPilot = 0;

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return;
	}
/*
	BM::GUID kPetGuid = pkPlayer->PetGuid();
	pkPetPilot = g_kPilotMan.FindPilot(kPetGuid);
	if(!pkPetPilot)
	{
		return;
	}

	PgActor *pkPetActor = dynamic_cast<PgActor *>(pkPetPilot->GetWorldObject());
	if(!pkPetActor)
	{
		return;
	}

	std::string kPetID(pkPetActor->GetID());
	g_kUIScene.RenderModel(kPetID.c_str(), self->Size(), self->TotalLocation(), pkPetActor->GetActorManager(), pkPetActor->GetUIModelUpdate());
*/
}

void lwUIWnd::TurnPetModel(float fDegree)
{
	PgPilot *pkPetPilot = 0;

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	BM::GUID kPetGuid;
	
	if(!pkPlayer)
	{
		return;
	}
/*
	kPetGuid = pkPlayer->PetGuid();
	pkPetPilot = g_kPilotMan.FindPilot(kPetGuid);
	if(!pkPetPilot)
	{
		return;
	}

	PgUIModel *pkUIModel = g_kUIScene.GetRenderModel(pkPetPilot->GetWorldObject()->GetID().c_str());
	if(!pkUIModel)
	{
		return;
	}

*/
}

void lwUIWnd::InitRenderModel(char const* szRenderModelName, bool bUpdate, bool bOrtho)
{
	if (!szRenderModelName)
	{
		return;
	}
	
	g_kUIScene.InitRenderModel(szRenderModelName, self->Size(), self->TotalLocation(), bUpdate, bOrtho);
}

void lwUIWnd::DrawRenderModel(char const* szRenderModelName)
{
	PgUIModel *pkUIModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkUIModel)
	{
		pkUIModel->RenderFrame(NiRenderer::GetRenderer(), self->TotalLocation());
	}
}

void lwUIWnd::ClearRenderModel(char const* szRenderModelName)
{
	PgUIModel *pkUIModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkUIModel)
	{
		pkUIModel->Clear();
	}
}

void lwUIWnd::DelRenderModel(char const* szRenderModelName)
{
	g_kUIScene.RemoveModel(szRenderModelName);
}

void lwUIWnd::AddToDrawListRenderModel(char const* szRenderModelName)
{
	g_kUIScene.AddToDrawListRenderModel(szRenderModelName);
}

void lwUIWnd::RenderModelCameraZoom(char const* szRenderModelName, float fCameraZoomDistance)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->CameraZoom(fCameraZoomDistance);
	}
}

void lwUIWnd::RenderModelCameraZoomSubstitution(char const* szRenderModelName, float fCameraZoomDistance)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->CameraZoomSubstitution(fCameraZoomDistance);
	}
}

void lwUIWnd::RenderModelOrthoCameraZoom(char const* szRenderModelName, float fZoomDistance)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->SetOrthoZoom(pkModel->GetOrthoZoom() + fZoomDistance);
	}
}

void lwUIWnd::RenderModelOrthoCameraZoomSubstitution(char const* szRenderModelName, float fZoomDistance)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->SetOrthoZoom(fZoomDistance);
	}
}

void lwUIWnd::RenderModelOrthoCameraZoomSubstitutionOldVer(char const* szRenderModelName, float fZoomDistance)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->SetOrthoZoomOld(fZoomDistance);
	}
}

void lwUIWnd::SetRenderModelCameraZoomMinMax(char const* szRenderModelName, float fMin, float fMax)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->SetCameraZoomMinMax(fMin, fMax);
	}

}

void lwUIWnd::RotateRenderModel(char const* szRenderModelName, float fXAxis, float fYAxis, float fZAxis)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->WorldRotate(fXAxis, fYAxis, fZAxis);
	}
}

void lwUIWnd::SetAutoRotateRenderModel(char const* szRenderModelName, bool bAutoRotate, float fXAxis, float fYAxis, float fZAxis)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->SetAutoRotate(bAutoRotate, fXAxis, fYAxis, fZAxis);
	}
}

void lwUIWnd::AddRenderModelActorByGuid(char const* szRenderModelName, char const* szActorName, char const* szGuid, bool bClone, bool bCameraReset)
{
	BM::GUID kGuid(szGuid);//darby
	PgPilot* pkPilot = g_kPilotMan.FindPilot(kGuid);
	PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
	if(pkActor)
	{
		PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
		if(pkModel)
		{
			pkModel->AddActor(szActorName, pkActor->GetActorManager(), true, true);
		}
	}
}

void lwUIWnd::AddRenderModelActorByID(char const* szRenderModelName, char const* szActorName, char const* szID, bool bClone, bool bCameraReset)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->AddActorByID(szActorName, szID);
	}
}

void lwUIWnd::AddRenderModelActorByPath(char const* szRenderModelName, char const* szActorName, char const* szPath, bool bClone, bool bCameraReset)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->AddActor(szActorName, g_kAMPool.LoadActorManager(szPath, PgIXmlObject::ID_NONE, false), false, true);
	}
}

void lwUIWnd::SetRenderModelActorTranslate(char const* szRenderModelName, char const* szActorName, float fx, float fy, float fz)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		NiActorManager* pkAM = pkModel->GetActor(szActorName);
		if(pkAM)
		{
			pkAM->GetNIFRoot()->SetTranslate(NiPoint3(fx, fy, fz));
		}
	}
}
void lwUIWnd::SetRenderModelActorRotate(char const* szRenderModelName, char const* szActorName, float fXAxis, float fYAxis, float fZAxis)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		NiActorManager* pkAM = pkModel->GetActor(szActorName);
		if(pkAM)
		{
			pkModel->LocalRotate(pkAM->GetNIFRoot(), fXAxis, fYAxis, fZAxis);
		}
	}
}

void lwUIWnd::AddRenderModelActor(char const* szRenderModelName, char const* szActorName, lwActor kActor, bool bClone, bool bCameraReset)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->AddPgActor(szActorName, kActor(), bClone, bCameraReset);
	}
}

void lwUIWnd::AddRenderModelNIFByID(char const* szRenderModelName, char const* szNIFName, char const* szID, bool bClone, bool bCameraReset)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		TiXmlDocument* pkXmlDoc = PgXmlLoader::GetXmlDocumentByID(szID);
		if (pkXmlDoc == NULL)
			return ;

		char const* NifPath = PgXmlLoader::GetXmlTagValue(pkXmlDoc->FirstChild(), "NIF_PATH");
		if (NifPath)
		{
			pkModel->AddNIF(szNIFName, g_kNifMan.GetNif(NifPath), false, true);
		}
	}
}

void lwUIWnd::AddRenderModelNIFByPath(char const* szRenderModelName, char const* szNIFName, char const* szPath, bool bClone, bool bCameraReset)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->AddNIF(szNIFName, g_kNifMan.GetNif(szPath), false, bCameraReset);
	}
}

void lwUIWnd::SetRenderModelNIFTranslate(char const* szRenderModelName, char const* szNIFName, float fx, float fy, float fz)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		NiNode* pkNode = pkModel->GetNIF(szNIFName);
		if(pkNode)
		{
			pkNode->SetTranslate(NiPoint3(fx, fy, fz));
		}
	}
}
void lwUIWnd::SetRenderModelNIFRotate(char const* szRenderModelName, char const* szNIFName, float fXAxis, float fYAxis, float fZAxis)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		NiNode* pkNode = pkModel->GetNIF(szNIFName);
		if(pkNode)
		{
			pkModel->LocalRotate(pkNode, fXAxis, fYAxis, fZAxis);
		}
	}
}

void lwUIWnd::ResetRenderModelNIFAnimation(char const* szRenderModelName, char const* szNIFName)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->ResetNIFAnimation(szNIFName);
	}
}

void lwUIWnd::SetRenderModelEnableUpdate(char const* szRenderModelName, bool bUpdate)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->SetEnableUpdate(bUpdate);
	}
}

void lwUIWnd::SetRenderModelActorTargetAnimation(char const* szRenderModelName, char const* szActorName, int iID)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);

	if(pkModel)
	{
		pkModel->SetTargetAnimation(szActorName, iID);
	}
}
void lwUIWnd::UpdateOrthoZoom(char const* szRenderModelName, float const fScale, int const iX, int const iY)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);	
	if(pkModel)
	{
		pkModel->SetOrthoZoom( fScale, iX, iY );
	}	
}

void lwUIWnd::UpdateOrthoZoomOldVer(char const* szRenderModelName, float const fScale, int const iX, int const iY)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);	
	if(pkModel)
	{
		pkModel->SetOrthoZoomOld( fScale, iX, iY );
	}	
}


bool lwUIWnd::IsRenderModelActorAnimationDone(char const* szRenderModelName, char const* szActorName)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);

	if(pkModel)
	{
		return pkModel->IsAnimationDone(szActorName);
	}

	return false;
}

bool lwUIWnd::SetCameraByName(char const* szRenderModelName, char const* szNIFName, char const *pcCameraName)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);

	if(pkModel)
	{
		return pkModel->SetCameraByName(szNIFName, pcCameraName);
	}

	return false;
}

bool lwUIWnd::SetRecursiveCameraByName(char const* szRenderModelName, char const* szNIFName, char const *pcCameraName)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);

	if(pkModel)
	{
		return pkModel->SetRecursiveCameraByName(szNIFName, pcCameraName);
	}

	return false;
}

void lwUIWnd::SetRenderModelActorEnableUpdate(char const* szRenderModelName, char const* szActorName, bool bEnable)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);

	if(pkModel)
	{
		return pkModel->SetActorEnableUpdate(szActorName, bEnable);
	}

}
void lwUIWnd::SetRenderModelNIFEnableUpdate(char const* szRenderModelName, char const* szNIFName, bool bEnable)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);

	if(pkModel)
	{
		return pkModel->SetNIFEnableUpdate(szNIFName, bEnable);
	}
}


lwPoint2 lwUIWnd::GetResolutionSize( const unsigned int iIndex )
{
	const NiDX9SystemDesc* pkDesc = NiDX9Renderer::GetSystemDesc();
	if( !pkDesc )
	{
		return lwPoint2( 0, 0 );
	}

	NiDX9Renderer* pkRenderer = NiDynamicCast( NiDX9Renderer, NiRenderer::GetRenderer());
	if( !pkRenderer )
	{
		return lwPoint2( 0, 0 );
	}

	const NiDX9AdapterDesc* pkAdapterDesc = pkDesc->GetAdapter( pkRenderer->GetAdapter());
	if( !pkAdapterDesc )
	{
		return lwPoint2( 0, 0 );
	}

	const NiDX9AdapterDesc::ModeDesc *pkModeDesc = pkAdapterDesc->GetMode( iIndex );
	if( !pkModeDesc )
	{
		return lwPoint2( 0, 0 );
	}

	if( 32 != pkModeDesc->m_uiBPP  )
	{
		return lwPoint2( 0, 0 );
	}

	return lwPoint2( pkModeDesc->m_uiWidth, pkModeDesc->m_uiHeight );	
}

bool lwUIWnd::IsWideResolution( const unsigned int iWidth, const unsigned int iHeight )
{
	return lwIsWideResolution(iWidth, iHeight);
}

int lwUIWnd::GetResolutionModeCount()
{
	const NiDX9SystemDesc* pkDesc = NiDX9Renderer::GetSystemDesc();
	if( !pkDesc )
	{
		return 0;
	}

	NiDX9Renderer* pkRenderer = NiDynamicCast( NiDX9Renderer, NiRenderer::GetRenderer());
	if( !pkRenderer )
	{
		return 0;
	}

	const NiDX9AdapterDesc* pkAdapterDesc = pkDesc->GetAdapter( pkRenderer->GetAdapter());
	if( !pkAdapterDesc )
	{
		return 0;
	}

	unsigned int uiModeNum = pkAdapterDesc->GetModeCount();
	
	return uiModeNum;
}

// 기본 해상도와의 차이
lwPoint2 lwUIWnd::GetResolutionGap()
{
	return lwPoint2( XUIMgr.GetResolutionSize().x - XUI::EXV_DEFAULT_SCREEN_WIDTH, XUIMgr.GetResolutionSize().y - XUI::EXV_DEFAULT_SCREEN_HEIGHT );
}


static float g_fStartTime = (float)BM::GetTime32();
static float g_fDisplayTime = -1;
void lwDisplayPetText( lwWString Text, int iTime )	//문자열, 천분초
{
	g_fDisplayTime = static_cast<float>(iTime);
	if( g_fDisplayTime > 0.0f )
	{
		g_fStartTime = (float)BM::GetTime32();
	}
	lwCallUI("FRM_PetText");
	XUI::CXUI_Wnd *form = XUIMgr.Get(_T("FRM_PetText"));
	if( form != NULL )
		form->Text(Text());
}

void lwIdlePetText()
{
	if( g_fDisplayTime > 0.0f )
	{
		if(((float)BM::GetTime32() - g_fStartTime) > g_fDisplayTime ) //지속시간
		{
			XUI::CXUI_Wnd *form = XUIMgr.Get(_T("FRM_PetText"));
			if( form != NULL )
			{
				form->Text(_T(" "));
				lwCloseUI("FRM_PetText");
			}
			g_fDisplayTime = -1;
		}
	}
}

void lwUIWnd::TempTreeTest()
{
	XUI::CXUI_Tree* pWnd = dynamic_cast<XUI::CXUI_Tree*>(self);

	pWnd->AddItem(_T("Key1"), NULL)->m_pWnd->Text(_T("1"));
	pWnd->AddItem(_T("Key1"), NULL)->m_pWnd->Text(_T("2"));
	pWnd->AddItem(_T("Key1"), NULL)->m_pWnd->Text(_T("3"));
	pWnd->AddItem(_T("Key1"), NULL)->m_pWnd->Text(_T("4"));
	pWnd->AddItem(_T("Key1"), NULL)->m_pWnd->Text(_T("5"));
	pWnd->AddItem(_T("Key1"), NULL)->m_pWnd->Text(_T("6"));
	pWnd->AddItem(_T("Key1"), NULL)->m_pWnd->Text(_T("7"));
	pWnd->AddItem(_T("Key1"), NULL)->m_pWnd->Text(_T("8"));
	pWnd->AddItem(_T("Key1"), NULL)->m_pWnd->Text(_T("9"));
	pWnd->AddItem(_T("Key1"), NULL)->m_pWnd->Text(_T("a"));
}

void lwTempQuestTest()
{

}
lwUIWnd	lwCreateControl(char const* szControlID)
{
	XUI::CXUI_Wnd * pControl = XUIMgr.Create(std::wstring(UNI(szControlID)),true);
	return	lwUIWnd(pControl);
}
void lwBossHPBar()
{
	static int iTime = 0;
	if( iTime == 0 )
	{
		lwCallUI("FRM_BAR_BOSS");
	}
	XUI::CXUI_Wnd* pForm = XUIMgr.Get(_T("FRM_BAR_BOSS"));
	if (!pForm)
	{
		NILOG(PGLOG_WARNING, "[lwBossHPBar] Can't find FRM_BAR_BOSS\n");
		return;
	}

	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(pForm->GetControl(_T("BAR_BOSS_HP")));
	if( pWnd == NULL )
		return;

	if( iTime == 0 )
	{
		iTime = BM::GetTime32();
		BM::vstring vstrTime(iTime);
		pWnd->StartTime( iTime );
		pWnd->FillTime(10000);
		pWnd->Now( 9900 );
		pWnd->Max( 10000 );
	}
	BM::vstring vstcur((int)pWnd->UVInfo().Index);//pWnd->Current();
	vstcur+=_T(" Current()");
	vstcur+=pWnd->Current();
	pWnd->Text(vstcur);

	XUI::CXUI_Wnd* pImg = pForm->GetControl(_T("BG"));

	if( pImg == NULL ){return;}

	int iUV = pWnd->UVInfo().Index-1;
	if( iUV <= 0 )
	{
		pImg->Visible(false);
	}
	else
	{
		SUVInfo kUVInfo = pImg->UVInfo();
		kUVInfo.Index = iUV;
		pImg->UVInfo(kUVInfo);
		pImg->Visible(true);
	}
}

void lwExitShop()
{
	BM::Stream kPacket(PT_C_M_REQ_STORE_CLOSE);
	NETWORK_SEND(kPacket);
}

void lwSetBossGUIDForHPBar(lwGUID bossguid)
{
	g_pkBoss = NULL;
	PgActor *pkActor = g_kPilotMan.GetPlayerActor();
	if (pkActor == NULL || pkActor->GetWorld() == NULL)
	{
		return;
	}

	g_pkBoss = g_kPilotMan.FindPilot(bossguid());
	if( !g_pkBoss )
	{
		return;
	}

	lwCallUI("FRM_BAR_BOSS");
	XUI::CXUI_Wnd* pForm = XUIMgr.Get(_T("FRM_BAR_BOSS"));
	if( pForm == NULL )
	{
		return;
	}

	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(pForm->GetControl(_T("BAR_BOSS_HP")));
	if( pWnd == NULL )
	{
		return;
	}

	int iTime = BM::GetTime32();
//	BM::vstring vstrTime = iTime;

	int iHP = g_pkBoss->GetAbil(AT_HP);
	int iMax = g_pkBoss->GetAbil(AT_C_MAX_HP);

	BM::vstring vstrHP(iHP);
	BM::vstring vstrMax(iMax);	//몬스터에게 max정보가 없어서 처음 들어온 hp값을 max로 셋팅

	pWnd->StartTime( iTime );
	pWnd->FillTime(10000);
	pWnd->Now( iHP );
	pWnd->Max( iMax );
}

void lwDisplayBossHPBar()
{
	if( g_pkBoss == NULL || g_pkBoss->GetUnit() == NULL )
	{
		lwCloseUI("FRM_BAR_BOSS");
		return;
	}

	XUI::CXUI_Wnd* pForm = XUIMgr.Get(_T("FRM_BAR_BOSS"));
	if( pForm == NULL )
	{
		return;
	}

	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(pForm->GetControl(_T("BAR_BOSS_HP")));
	if( pWnd == NULL )
	{
		return;
	}

	int iHP = g_pkBoss->GetAbil(AT_HP);
	int iMax = g_pkBoss->GetAbil(AT_C_MAX_HP);
	
	BM::vstring kPercent(_T(" ? "));
	if(iMax != 0)
	{
		TCHAR akPercent[10] = {0};
		_stprintf_s(akPercent, 10, _T("%.1f"), iHP/(double)iMax * 100.0f);
		kPercent = akPercent;
	}

	pWnd->Now(iHP);

	XUI::CXUI_Image* pImg = dynamic_cast<XUI::CXUI_Image*>(pForm->GetControl(_T("BG")));

	if( pImg == NULL ){return;}

	int iUV = pWnd->UVInfo().Index-1;
	if( iUV <= 0 )
	{
		pImg->Visible(false);
	}
	else
	{
		SUVInfo kUVInfo = pImg->UVInfo();
		kUVInfo.Index = iUV;
		pImg->UVInfo(kUVInfo);
		pImg->Visible(true);
	}

	BM::vstring vstcur;
	vstcur = g_pkBoss->GetName();
	vstcur+=_T(" ");
	vstcur+=kPercent;
	vstcur+=_T("% (");
	vstcur+=iHP;
	vstcur+=_T("/");
	vstcur+=iMax;
	vstcur+=_T(")");
	pWnd->Text( vstcur );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	lwUIListItem
//////////////////////////////////////////////////////////////////////////////////////////////////////
//! 생성자
lwUIListItem::lwUIListItem(XUI::SListItem *pItem)
{
	m_pListItem = pItem;
}

//! Wrapper를 등록한다.
bool lwUIListItem::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwUIListItem>(pkState, "UIListItem")
		.def(pkState, constructor<XUI::SListItem*>())
		.def(pkState, "IsNil", &lwUIListItem::IsNil)
		.def(pkState, "GetWnd", &lwUIListItem::GetWnd)
		;
	return true;
}

bool	lwUIListItem::IsNil()
{
	return	m_pListItem == NULL;
}

lwUIWnd	lwUIListItem::GetWnd()
{
	if( m_pListItem )
	{
		return	lwUIWnd(m_pListItem->m_pWnd);
	}
	return lwUIWnd(NULL);
}
XUI::SListItem	* lwUIListItem::operator()()
{
	return	m_pListItem;
}

void lwUIWnd::SetAliveTime(int Value)
{
	if (self)
	{
		self->AliveTime(Value);
	}
	else
	{
		NILOG(PGLOG_WARNING, "[SetAliveTime] Can't find self\n");
	}
}
int lwUIWnd::GetAliveTime()
{
	if (self)
	{
		return self->AliveTime();
	}
	else
	{
		NILOG(PGLOG_WARNING, "[GetAliveTime] Can't find self\n");
		return 0;
	}	
}

void lwUIWnd::RefreshCalledTime()
{
	if (self)
	{
		self->RefreshCalledTime();
	}
	else
	{
		NILOG(PGLOG_WARNING, "[RefreshCalledTime] Can't find self\n");
	}
}

void lwUIWnd::RefreshLastTickTime()
{
	if (self)
	{
		self->RefreshLastTickTime();
	}
	else
	{
		NILOG(PGLOG_WARNING, "[RefreshLastTickTime] Can't find self\n");
	}
}

lwPoint2 lwUIWnd::GetSize()
{	
	if (self)
	{
		lwPoint2 size = self->Size();
		return size;
	}
	return lwPoint2(0,0);
}

void lwUIWnd::SetSize(lwPoint2 pt)
{
	if (self)
	{
		self->Size(pt());
	}
	else
	{
		NILOG(PGLOG_WARNING, "[SetSize] Can't find self\n");
	}
}

lwPoint2 lwUIWnd::GetImgSize()
{
	if (self)
	{
		lwPoint2 size = self->ImgSize();
		return size;
	}
	return lwPoint2(0,0);
}

void lwUIWnd::SetImgSize(lwPoint2 pt)
{
	if (self)
	{
		self->ImgSize(pt());
	}
	else
	{
		NILOG(PGLOG_WARNING, "[GetLocation] Can't find self\n");
	}
}
float lwUIWnd::GetScale()
{
	if (self)
	{
		return self->Scale();
	}
	return 0.0f;
}

void lwUIWnd::SetScale(float fScale)
{
	if (self)
	{
		self->Scale(fScale);
	}
}

lwPoint2F lwUIWnd::GetScaleCenter()
{	
	if (self)
	{
		return self->ScaleCenter();
	}
	return lwPoint2F(0.0f, 0.0f);;
}

void lwUIWnd::SetScaleCenter(lwPoint2F kCenter)
{
	if (self)
	{
		return self->ScaleCenter(kCenter());
	}
}

float lwUIWnd::GetRotationDeg()
{
	if (self)
	{
		return self->RotationDeg();
	}
	return 0.0f;
}

void lwUIWnd::SetRotationDeg(float fDeg)
{
	if (self)
	{
		while(360.0f < fDeg)
		{
			fDeg-=360.0f;
		}
		self->RotationDeg(fDeg);
	}
}

lwPoint2F lwUIWnd::GetRotationCenter()
{	
	if (self)
	{
		return self->RotationCenter();
	}
	return lwPoint2F(0.0f, 0.0f);;
}

void lwUIWnd::SetRotationCenter(lwPoint2F kCenter)
{
	if (self)
	{
		return self->RotationCenter(kCenter());
	}
}

void lwUIOff(bool const bAdd_LastContainer)
{
	OutputDebugString(UNI("UIOff\n"));
	int iNum = XUIMgr.HideAllUI(bAdd_LastContainer);
}

void lwUIOn()
{
	int iNum = XUIMgr.ShowAllUI();
	XUI::CXUI_Wnd *pkForm = XUIMgr.Get(std::wstring(_T("FormHero")));
	if (pkForm)
	{
		XUI::CXUI_Wnd *pkMphp = pkForm->GetControl(std::wstring(_T("SFRM_HPMP_BG")));
		if (pkMphp)
		{
			XUI::CXUI_Wnd *pkLV = pkMphp->GetControl(std::wstring(_T("FRM_LV")));
			if (pkLV)
			{
				lwUIWnd	lwText(pkLV->GetControl(_T("FRM_LV_TEXT")));
				lwDrawPlayerLv(lwText);
			}
		}
	}
}

void lwBlockGlobalHotKey( bool bBlocking )
{
	XUIMgr.BlockGlobalScript( bBlocking );
}

bool lwIsVisible(char *pszText)
{
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(UNI(pszText));
	if( pWnd != NULL && pWnd->Visible() )
	{
		return true;
	}

	return false;
}

lwUIWnd lwGetUIWnd(char const *pszText)
{
	if( NULL==pszText )
	{
		return lwUIWnd(NULL, true);
	}

	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(UNI(pszText));
	if(pWnd)
	{
		return lwUIWnd(pWnd);
	}

	NILOG(PGLOG_WARNING, "[lwGetUIWnd] Can't find ID=%s\n", pszText);
	return lwUIWnd(NULL, false);
}

bool lwIsUIWnd(char const *pszText)
{
	XUI::CXUI_Wnd *pkWnd = NULL;
	return XUIMgr.IsActivate( UNI(pszText), pkWnd );
}

void lwUIWnd::SetStartTime(int nMax)
{
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);

	if(!pWnd)	//다이나믹 캐스트 실패
	{
		return;
	}

	int iTime = (int)(g_pkApp->GetAccumTime()*1000.0f);

	pWnd->StartTime(iTime);
	pWnd->CloseTime(0);
	pWnd->Now(0);
	pWnd->Current(0);
	pWnd->Max(nMax);
}

// 현재시간부터 몇초후에 멈출것인지 셋팅한다.(남은시간보다 이값이 크면 자동으로 조절해준다.)
void lwUIWnd::SetCloseTime( int const nAfterStopTime )
{
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);

	if(!pWnd)	//다이나믹 캐스트 실패
	{
		NILOG(PGLOG_WARNING, "[SetCloseTime] Can't find pWnd\n");
		return;
	}

	int const iNow = (int)(g_pkApp->GetAccumTime()*1000.0f);
	int const iTime = iNow - pWnd->StartTime();
	int const iRemainTime = pWnd->Max() - iTime;
	if ( iRemainTime <= nAfterStopTime )
	{
		pWnd->CloseTime(iTime+iRemainTime);
	}
	else
	{
		pWnd->CloseTime(iTime+nAfterStopTime);
	}
}

void lwUIWnd::DisplayTimeBar( bool bText )
{
	if ( bText )
	{
		char szVal[5] = {0,};
		sprintf_s( szVal, sizeof(szVal), "%%.2f" );
		DisplayTimeBarMsg( szVal, true );
	}
	else
	{
		DisplayTimeBarMsg( NULL, true );
	}
}

void lwUIWnd::DisplayTimeBarMsg( char const* _Format, bool bReal )
{
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);
	if( pWnd )
	{
		char szVal[256] = {0,};
		int const iNow = (int)(g_pkApp->GetAccumTime()*1000.0f);
		int const iMaxTime = pWnd->Max();
		int iTime = iNow - pWnd->StartTime();
		iTime = __min( iTime, iMaxTime );

		if ( !pWnd->CloseTime() )
		{
			if( iTime >= iMaxTime )
			{
				if ( !pWnd->IsScriptRun() )
				{
					pWnd->DoScript(SCRIPT_ON_BAR_FULL);	//클로즈 되서 애니바 자체에서 DoScript가 실행되지 않으므로
					pWnd->IsScriptRun(true);			//강제로 실행

					if ( pWnd->BarType() == 1 )
					{
						GetParent().Close();
						return;
					}
				}
			}
		}
		else
		{
			if ( (DWORD)iTime >= pWnd->CloseTime() )
			{
				pWnd->DoScript(SCRIPT_ON_BAR_CLOSE);	// 강제로 타임바를 중간에 멈춘다.
				pWnd->IsScriptRun(true);
				GetParent().Close();
				return;
			}
		}

		pWnd->Now( iTime );

		if ( _Format )
		{
			std::string cutStr(_Format);
			cutStr = cutStr.substr(0, 255);
			int const iPrintTime = (pWnd->BarType() == 1) ? (iMaxTime - iTime) : iTime;
			if ( bReal )
			{
				sprintf_s(szVal, 256, cutStr.c_str(), iPrintTime/1000.0f );
			}
			else
			{
				sprintf_s(szVal, 256, cutStr.c_str(), iPrintTime/1000 + ( (iPrintTime%1000) ? 1 : 0 ) );
			}
		}
		pWnd->Text( UNI(szVal) );
	}
}

void lwUIWnd::DisplayPerBarMsg(char const* _Format, int iIncSpeedByMilSec, bool isFloat)
{// ANI_BAR의 게이지 증가 메세지를 백분율 단위로 보여준다
	XUI::CXUI_AniBar* pWnd = dynamic_cast<XUI::CXUI_AniBar*>(self);
	if( pWnd )
	{
		char szVal[50] = {0,};
		int const iNow = (int)(g_pkApp->GetAccumTime()*1000.0f);
		int const iMaxTime = pWnd->Max();
		if(0 == iMaxTime)
		{// 목표 시간이 0 이면 0을 표시한다.
			sprintf_s(szVal, 50, _Format, 0);
		}
		else
		{
			int iTime = iNow - pWnd->StartTime();
			iTime = __min( iTime, iMaxTime );

			if ( !pWnd->CloseTime() )
			{
				if( iTime >= iMaxTime )
				{
					if ( !pWnd->IsScriptRun() )
					{
						pWnd->DoScript(SCRIPT_ON_BAR_FULL);	//클로즈 되서 애니바 자체에서 DoScript가 실행되지 않으므로
						pWnd->IsScriptRun(true);			//강제로 실행

						if ( pWnd->BarType() == 1 )
						{
							GetParent().Close();
							return;
						}
					}
				}
			}
			else
			{
				if ( (DWORD)iTime >= pWnd->CloseTime() )
				{
					pWnd->DoScript(SCRIPT_ON_BAR_CLOSE);	// 강제로 타임바를 중간에 멈춘다.
					pWnd->IsScriptRun(true);
					GetParent().Close();
					return;
				}
			}

			pWnd->Now( iTime );


			if ( _Format )
			{
				int const iPrintTime = (pWnd->BarType() == 1) ? (iMaxTime - iTime) : iTime;
				if(isFloat)
				{// _Format이 실수 형태 일때
					float fIncSpeedByMilSec = iIncSpeedByMilSec;
					float fPercentage = (iPrintTime/fIncSpeedByMilSec)/iMaxTime*fIncSpeedByMilSec*100.0f;
					sprintf_s(szVal, 50, _Format, fPercentage );
				}
				else
				{// _Format이 정수 형태 일때
					float fPercentage = iPrintTime/iIncSpeedByMilSec;
					int iPercentage = (fPercentage/iMaxTime)*iIncSpeedByMilSec*100;
					sprintf_s(szVal, 50, _Format, iPercentage );
				}
			}
		}
		pWnd->Text( UNI(szVal) );
	}
}

void lwUIWnd::ChangeImage( char *pFileName, bool const bAniBar )
{
	if(!pFileName)
	{
		return;
	}


	if (self)
	{
		if ( true == bAniBar )
		{
			XUI::CXUI_AniBar *pkBar = dynamic_cast<XUI::CXUI_AniBar*>(self);
			if ( pkBar )
			{
				pkBar->ChangeBarImage( UNI(pFileName) );
			}
			else
			{
				NILOG(PGLOG_WARNING, "[ChangeImage] Not AniBar ImgPath=%s\n", pFileName );
			}
		}
		else
		{
			self->DefaultImgName(UNI(pFileName));
		}
	}
	else
	{
		NILOG(PGLOG_WARNING, "[ChangeImage] Can't find self ImgPath = %s\n", pFileName);
	}
}

void lwUIWnd::ButtonChangeImage(char *szFileName, int iWidth, int iHeight)
{
	XUI::CXUI_Button* pWnd = dynamic_cast<XUI::CXUI_Button*>(self);
	if(pWnd)
	{//!!주의!! 버튼크기는 변경되어도 Control 크기까지 변경되지 않는다. 이는 다른 SetValue같은 걸로 조절해야 한다.
		std::wstring kFileName = UNI(szFileName);
		pWnd->ButtonChangeImage(kFileName, iWidth, iHeight);
	}
	else
	{
		NILOG(PGLOG_WARNING, "[ButtonChangeImage] Can't find self ID = %s\n", szFileName);
	}
	PG_ASSERT_LOG(0);//Control이 Button이 아니다
}

void lwUIWnd::VOnCall()
{
	if (self)
	{
		self->VOnCall();
	}
	else
	{
		NILOG(PGLOG_WARNING, "[VOnCall] Can't find self\n");
	}
}

bool lwUIWnd::IsMouseDown()
{
	if (self)
	{
		return self->IsMouseDown();
	}
	NILOG(PGLOG_WARNING, "[IsMouseDown] Can't find self\n");
	return false;
}
void lwUIWnd::SetGrayScale(bool bGrayScale)
{
	if (self)
	{
		self->GrayScale(bGrayScale);
	}
}
bool lwUIWnd::IsGrayScale()
{
	if (self)
	{
		return self->GrayScale();
	}

	return false;
}

float lwUIWnd::GetSizeScale()
{
	if( self )
	{
		return self->Scale();
	}
	return 1.0f;
}

void lwUIWnd::SetSizeScale(float const fScale, bool const bIsCenterTie)
{
	if( self )
	{
		self->Scale(fScale);
		if( bIsCenterTie )
		{
			XUI::CXUI_Wnd* pParent = self->Parent();
			if( pParent == NULL )
			{
				return;
			}

			POINT2 PtSize((self->Size().x * fScale), (self->Size().y * fScale));
			self->Location(1+(pParent->Size().x - PtSize.x) / 2, (pParent->Size().y - PtSize.y) / 2);
		}
	}
}

bool lwUIWnd::SetColor(unsigned int iColor)
{
	if (self)
	{
		return self->SetColor((DWORD)iColor);
	}
	NILOG(PGLOG_WARNING, "[SetColor] Can't find self\n");
	return false;
}

bool lwUIWnd::SetPilotBarInfo(int iType)
{
	BarInfoMapItor it = g_BarInfoMap.find(iType);
	if (it == g_BarInfoMap.end())
	{
		return false;
	}

	XUI::CXUI_Wnd* pWnd = self;

	if (pWnd == NULL)
	{
		return false;
	}

	pWnd->Text(it->second );

	return true;
}

void lwUIWnd::SetStaticNum(int iNum)
{
	XUI::CXUI_StaticForm* pWnd = dynamic_cast<XUI::CXUI_StaticForm*>(self);
	if (pWnd == NULL)
	{
		return;
	}
	
	pWnd->StaticNum(iNum);	//번호를 새로 설정하고
	pWnd->VInit();			//config에서 번호에 지정된 이미지를 부름
}

int lwUIWnd::GetStaticNum()
{
	XUI::CXUI_StaticForm* pWnd = dynamic_cast<XUI::CXUI_StaticForm*>(self);
	if (pWnd == NULL)
	{
		return 0;
	}

	return pWnd->StaticNum();
}

//Scroll
void lwUIWnd::SetScrollCur(int iCur)
{
	XUI::CXUI_HScroll* pWnd = dynamic_cast<XUI::CXUI_HScroll*>(self);
	if(pWnd)
	{
		pWnd->CurValue(iCur);
	}
}
int lwUIWnd::GetScrollCur()
{
	XUI::CXUI_HScroll* pWnd = dynamic_cast<XUI::CXUI_HScroll*>(self);
	if(pWnd)
	{
		return pWnd->CurValue();
	}
	return 0;
}
int lwUIWnd::GetScrollMax()
{
	XUI::CXUI_HScroll* pWnd = dynamic_cast<XUI::CXUI_HScroll*>(self);
	if(pWnd)
	{
		return (int)pWnd->MaxValue();
	}
	return 0;
}  

int lwUIWnd::GetBuildIndex()
{
	if (self)
	{
		return self->BuildIndex();
	}

	return 0;
}

void lwUIWnd::SetBuildIndex(int iBuildIndex)
{
	if (self)
	{
		self->BuildIndex(iBuildIndex);
	}
}

lwPoint2 const lwUIWnd::GetBuildCount()	//CXUI_Builder의 카운트를 반환
{
	lwPoint2 kIndex(0,0);
	if (self)
	{
		if(XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(self))
		{
			kIndex.SetX(pkBuilder->CountX());
			kIndex.SetY(pkBuilder->CountY());
		}
	}
	return kIndex;
}

int const lwUIWnd::GetBuildNumberCount()
{
	if (self)
	{
		if(XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(self))
		{
			return pkBuilder->CountX() * pkBuilder->CountY();
		}
	}
	return 0;
}

lwPoint2 const lwUIWnd::GetBuildGab()
{
	lwPoint2 kGab(0,0);
	if (self)
	{
		if(XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(self))
		{
			kGab.SetX(pkBuilder->GabX());
			kGab.SetY(pkBuilder->GabY());
		}
	}
	return kGab;
}

bool lwUIWnd::ConvNiKey(int& iNiKey)
{
	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_FRANCE:
		{
			switch(iNiKey)
			{// 프랑스 키보드 
			case NiInputKeyboard::KEY_GRAVE:
				{
					iNiKey = NiInputKeyboard::KEY_APOSTROPHE;
				}return true;
			case NiInputKeyboard::KEY_MINUS:
				{// minus 위치의 기능은 LBracket
					iNiKey = NiInputKeyboard::KEY_LBRACKET;
				}return true;
			case NiInputKeyboard::KEY_LBRACKET:
				{// LBracke 위치의 기능은 
					iNiKey = NiInputKeyboard::KEY_RBRACKET;
				}return true;
			case NiInputKeyboard::KEY_RBRACKET:
				{// RBracke 위치의 기능은 
					iNiKey = NiInputKeyboard::KEY_SEMICOLON;
				}return true;
			case NiInputKeyboard::KEY_SEMICOLON:
				{
					iNiKey = NiInputKeyboard::KEY_M;
				}return true;
			case NiInputKeyboard::KEY_APOSTROPHE:
				{
					iNiKey = NiInputKeyboard::KEY_GRAVE;
				}return true;
			case NiInputKeyboard::KEY_M:
				{
					iNiKey = NiInputKeyboard::KEY_COMMA;
				}return true;
				
			case NiInputKeyboard::KEY_COMMA:
				{
					iNiKey = NiInputKeyboard::KEY_PERIOD;
				}return true;
			case NiInputKeyboard::KEY_PERIOD:
				{
					iNiKey = NiInputKeyboard::KEY_SLASH;
				}return true;
			//case NiInputKeyboard::KEY_SLASH:
			//	{//nikey에 없는 값
			//		if(self)
			//		{
			//			self->Enable(false);
			//			self->GrayScale(true);
			//		}
			//	}return false;
			}return false;
		}return false;
	case LOCAL_MGR::NC_GERMANY:
		{
			switch(iNiKey)
			{// 독일 키보드
			case NiInputKeyboard::KEY_GRAVE:
				{
					iNiKey = NiInputKeyboard::KEY_BACKSLASH;
				}return true;
			case NiInputKeyboard::KEY_MINUS:
				{
					iNiKey = NiInputKeyboard::KEY_LBRACKET;
				}return true;
			case NiInputKeyboard::KEY_EQUALS:
				{
					iNiKey = NiInputKeyboard::KEY_RBRACKET;
				}return true;

			case NiInputKeyboard::KEY_LBRACKET:
				{
					iNiKey = NiInputKeyboard::KEY_SEMICOLON;
				}return true;
			case NiInputKeyboard::KEY_RBRACKET:
				{
					iNiKey = NiInputKeyboard::KEY_EQUALS;
				}return true;

			case NiInputKeyboard::KEY_SEMICOLON:
				{
					iNiKey = NiInputKeyboard::KEY_GRAVE;
				}return true;

			case NiInputKeyboard::KEY_SLASH:
				{
					iNiKey = NiInputKeyboard::KEY_MINUS;
				}return true;
			}return false;
		}return false;
	case LOCAL_MGR::NC_EU:
	case LOCAL_MGR::NC_USA:
		{
			switch(iNiKey)
			{// 유럽 영어 키보드(영국식 영어 키보드)	
			case NiInputKeyboard::KEY_APOSTROPHE:
				{
					iNiKey = NiInputKeyboard::KEY_GRAVE;
				}return true;
			}return false;
		}return false;
	default:
		{
			return false;
		}break;
	}
	return false;
}

void lwUIWnd::lwSetPauseTime(int const iValue)
{
	PgMoveAnimateWnd* pWnd = dynamic_cast<PgMoveAnimateWnd*>(self);
	pWnd ? pWnd->PauseTime(iValue/1000.f) : 0;
}

float lwUIWnd::lwGetPauseTime()const
{
	PgMoveAnimateWnd* pWnd = dynamic_cast<PgMoveAnimateWnd*>(self);
	return pWnd ? pWnd->PauseTime() : 0.f;
}

void lwUIWnd::lwSetupMove(lwPoint2 const kStart, lwPoint2 const kEnd, float const fTotalMoveTime)
{
	PgMoveAnimateWnd* pkMoveTarget = dynamic_cast<PgMoveAnimateWnd*>(self);
	if( pkMoveTarget )
	{
		NiPoint3 kStartLoc(kStart.GetX(), kStart.GetY(), pkMoveTarget->StartPoint().z);
		NiPoint3 kEndLoc(kEnd.GetX(), kEnd.GetY(), pkMoveTarget->EndPoint().z);
		pkMoveTarget->Setup(kStartLoc, kEndLoc, fTotalMoveTime);
	}
}

float lwCompNum(float fLeft, float fRight)
{
	float fResult = fLeft - fRight;

	if(fResult == 0.0f)
	{
		return 0.0f;
	}
	else if(fResult > 0.0f)	//왼쪽이 더 크면
	{
		return 1.0f;
	}
	else if(fResult < 0.0f)	//오른쪽이 더 크면
	{
		return -1.0f;
	}

	return 0.0f;
}

int MakeCharInfoStr(int iOrigin, int iAdd, char* sz, unsigned int iSize)
{
	if(sz == NULL)
	{
		return 0;
	}

	int iDelta = iAdd - iOrigin;
	if( iDelta != 0 )
	{
		if( iDelta > 0 )
		{
			//sprintf_s( sz, iSize, "{C=0xFF4D3413/T=Font_Text/}%d ( %d + {C=0xFF00FF00/T=Font_Text/}%d {C=0xFF4D3413/T=Font_Text/})", iAdd, iOrigin, iDelta);
			sprintf_s( sz, iSize, "{C=0xFF4D3413/}%d (%d+{C=0xFF008e21/}%d{C=0xFF4D3413/})", iAdd, iOrigin, iDelta);
		}
		else
		{
			//sprintf_s( sz, iSize, "{C=0xFF4D3413/T=Font_Text/}%d ( %d - {C=0xFFFF0000/T=Font_Text/}%d {C=0xFF4D3413/T=Font_Text/})", iAdd, iOrigin, abs(iDelta));
			sprintf_s( sz, iSize, "{C=0xFF4D3413/}%d (%d-{C=0xFFFF0000/}%d{C=0xFF4D3413/})", iAdd, iOrigin, abs(iDelta));
		}
	}
	else
	{
		sprintf_s( sz, iSize, "{C=0xFF4D3413/}%d", iOrigin);
	}
	return iDelta;
}

lwWString lwGetTT(int iTextTableNo)
{
	lwWString lwStr = TTW(iTextTableNo);
	return lwStr;
}

bool lwKeyWasReleased(int iKeyNum)
{
	if(!g_pkLocalManager)
	{
		return	false;
	}

	NiInputKeyboard	*pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
	return	pkKeyboard->KeyWasReleased((NiInputKeyboard::KeyCode)iKeyNum);
}
void lwReleaseAllKey()
{
	if(!g_pkLocalManager)
	{
		return;
	}

	PgInputSystem *pkInputSystem = NiDynamicCast(PgInputSystem,g_pkLocalManager->GetInputSystem());

	if(pkInputSystem)
	{
		pkInputSystem->ReleaseAllKey();
	}
}
bool lwKeyIsDown(int iKeyNum, bool bIsNotUKey)
{
	if(!g_pkLocalManager)
	{
		return	false;
	}

	if(XUIMgr.GetFocusedEdit())
	{
		return	false;
	}

	if(bIsNotUKey == false)
	{
		iKeyNum = g_kGlobalOption.GetUKeyToKey(iKeyNum);
	}

	NiInputKeyboard	*pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
	return	pkKeyboard->KeyIsDown((NiInputKeyboard::KeyCode)iKeyNum);
}

lwWString lwGetDropRes(int iNum)
{
	if (iNum < 0 || iNum >= 506504708)
	{
		return lwWString("");
	}
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iNum);
	if (pkItemDef == NULL)
	{
		return lwWString("");
	}
	int const iResNo = pkItemDef->GetAbil(AT_DROPITEMRES);

	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);

	CONT_DEFRES::const_iterator itrDest = pContDefRes->find(iResNo);
	if(itrDest == pContDefRes->end())
	{
		// 아이템 테이블에서 못 찾으면 패스!
		return lwWString("");
	}

	return lwWString(MB(itrDest->second.strXmlPath));
}
void	lwEnableUIOffscreenRendering(bool bEnable)
{
	g_kUIScene.EnableOffScreenRendering(bEnable);
}
bool lwGetItemResData(int iItemNo, TBL_DEF_RES &rkDefRes)
{
	if (iItemNo < 0 || iItemNo >= 506504708)
	{
		return false;
	}
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if (pkItemDef == NULL)
	{
		return false;
	}
	//const int iResNo = pkItemDef->GetAbil(AT_DROPITEMRES);

	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);

	CONT_DEFRES::const_iterator itrDest = pContDefRes->find(iItemNo);
	if(itrDest == pContDefRes->end())
	{
		// 아이템 테이블에서 못 찾으면 패스!
		return false;
	}

	rkDefRes = itrDest->second;

	return true;
}

bool lwRegUIColor(int iIdx, int iColor)
{
	return XUIMgr.AddColor(iIdx, (DWORD)iColor);
}
int lwGetUIColor(int iIdx)
{
	return static_cast<int>(XUIMgr.GetColor(iIdx));
}
void lwChangeUIColorSet()
{
	XUIMgr.SetColorSet();
}

bool AddItemToInvenList(XUI::CXUI_Wnd* pWnd, PgBase_Item* kItem, SItemPos* kPos)
{
	if ( pWnd == NULL )
	{
		return true;
	}

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pWnd);

	if (pList == NULL || kItem == NULL)
	{
		return false;
	}

	std::wstring szName;
	if(MakeItemName(kItem->ItemNo(), kItem->EnchantInfo(), szName) )//이름 디스플레이
	{
		XUI::SListItem *pItem = pList->AddItem(szName);
		XUI::CXUI_Wnd* pName = pItem->m_pWnd->GetControl(_T("SFRM_ITEM_NAME"));
		if (pName)
		{
			pName->Text(szName);
		}
		if (!pItem)
		{
			return false;
		}
		if (!pItem->m_pWnd)
		{
			return false;
		}

		XUI::CXUI_Wnd* pBG = pItem->m_pWnd->GetControl(_T("SFRM_ICON_BG"));
		if (!pBG)
		{
			return false;
		}
		
		XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pBG->GetControl(_T("InvIcon")));
		if (pIcon)
		{
			SIconInfo kIconInfo = pIcon->IconInfo();

			kIconInfo.iIconKey = kPos->y;
			pIcon->SetIconInfo(kIconInfo);
		}
	}
	else
	{
		return false;
	}

	return true;
}

//>>Config
void lwUpdateEtcConfig()
{
	PgActor::SetVisiblePCName( 0 != g_kGlobalOption.GetValue("ETC", "SHOW_NAME_PC") );
	PgActor::SetVisibleNPCName( 0 != g_kGlobalOption.GetValue("ETC", "SHOW_NAME_NPC") );
	PgActor::SetAutoGetItemMoney( 0 != g_kGlobalOption.GetValue("ETC", "AUTOITEM_MONEY") );
	PgActor::SetAutoGetItemEquip( 0 != g_kGlobalOption.GetValue("ETC", "AUTOITEM_EQUIP") );
	PgActor::SetAutoGetItemConsume( 0 != g_kGlobalOption.GetValue("ETC", "AUTOITEM_CONSUME") );
	PgActor::SetAutoGetItemETC( 0 != g_kGlobalOption.GetValue("ETC", "AUTOITEM_ETC") );
	PgEnergyGauge::ms_bDrawEnergyGaugeBar = (0 != g_kGlobalOption.GetValue( "ETC", "BATTLEOP_DISPLAY_HPBAR" ));
	PgEnergyGauge::ms_bDrawPetMPGaugeBar = (0 != g_kGlobalOption.GetValue( "ETC", "DISPLAY_PET_MPBAR" ));
	g_kGlobalOption.SetDisplayHelmet( g_kGlobalOption.IsDisplayHelmet() );
	if( g_pkWorld )
	{
		g_pkWorld->AllNpcUpdateQuest();
		//g_kGlobalOption.ShowNavigation(0 != g_kGlobalOption.GetValue( "ETC", "NAVIGATION_VISIBLE" ));
	}
	g_kUIScene.UpdateMinimapQuest();
}

void lwConfig_DefaultConfig()
{
	g_kGlobalOption.DefaultConfig();
}
void lwConfig_DefaultKeySet()
{
	g_kGlobalOption.DefaultKeySet();
}
void lwConfig_Save(bool bSaveToServer)
{
	g_kGlobalOption.Save(bSaveToServer);
}
int lwConfig_GetDefaultValue(char const *szHead, char const *szKey)
{
	if(NULL==szHead || NULL==szKey)
	{
		NILOG(PGLOG_ERROR, "Head key or Key is NULL"); 
		PG_ASSERT_LOG(0);
		return 0;
	}
	return g_kGlobalOption.GetDefaultValue(szHead, szKey);
}
char const* lwConfig_GetDefaultText(char const *szHead, char const *szKey)
{
	if(NULL==szHead || NULL==szKey)
	{
		NILOG(PGLOG_ERROR, "Head key or Key is NULL"); 
		PG_ASSERT_LOG(0);
		return "";
	}
	return g_kGlobalOption.GetDefaultText(szHead, szKey);
}
void lwConfig_ApplyConfig()
{
	g_kGlobalOption.ApplyConfig();
	g_pkApp->OnConfigChanged();
}
void lwConfig_CancelConfig()
{
	g_kGlobalOption.CancelConfig();
}
void lwConfig_ApplyKeySet()
{
	g_kGlobalOption.ApplyKeySet();
}
void lwConfig_ApplyGraphics()
{
	//g_kGlobalOption.ApplyGraphics();
}
void lwConfig_CancelKeySet()
{
	g_kGlobalOption.CancelKeySet();
}
int lwConfig_GetValue(char const *szHead, char const *szKey)
{
	if(NULL==szHead || NULL==szKey)
	{
		NILOG(PGLOG_ERROR, "Head key or Key is NULL"); 
		PG_ASSERT_LOG(0);
		return 0;
	}
	return g_kGlobalOption.GetValue(szHead, szKey);
}
char const* lwConfig_GetText(char const *szHead, char const *szKey)
{
	if(NULL==szHead || NULL==szKey)
	{
		NILOG(PGLOG_ERROR, "Head key or Key is NULL"); 
		PG_ASSERT_LOG(0);
		return "";
	}
	return g_kGlobalOption.GetText(szHead, szKey);
}
void lwConfig_SetValue(char const *szHead, char const *szKey, int iValue)
{
	if(NULL==szHead || NULL==szKey)
	{
		NILOG(PGLOG_ERROR, "Head key or Key is NULL"); 
		PG_ASSERT_LOG(0);
		return ;
	}
	g_kGlobalOption.SetConfig(szHead, szKey, iValue, NULL);
}
void lwConfig_SetText(char const *szHead, char const *szKey, char const *szText)
{
	if(NULL==szHead || NULL==szKey)
	{
		NILOG(PGLOG_ERROR, "Head key or Key is NULL"); 
		PG_ASSERT_LOG(0);
		return ;
	}
	g_kGlobalOption.SetConfig(szHead, szKey, NULL, szText);
}
void lwSysConfig_SetValue(char const *szHead, char const *szKey, int iValue)
{
	if(NULL==szHead || NULL==szKey)
	{
		NILOG(PGLOG_ERROR, "Head key or Key is NULL"); 
		PG_ASSERT_LOG(0);
		return ;
	}
	g_kGlobalOption.SysSetConfig(szHead, szKey, iValue, NULL);
}
void lwSysConfig_SetText(char const *szHead, char const *szKey, char const *szText)
{
	if(NULL==szHead || NULL==szKey)
	{
		NILOG(PGLOG_ERROR, "Head key or Key is NULL"); 
		PG_ASSERT_LOG(0);
		return ;
	}
	g_kGlobalOption.SysSetConfig(szHead, szKey, NULL, szText);
}

bool lwUseLevelRank()
{
	return g_kGlobalOption.GetUseLevelRank();
}

//<<Config

//>>KeySet
int lwUKeyToKey(int iUkey)
{
	return g_kGlobalOption.GetUKeyToKey(iUkey);
}
//<<KeySet

char const* lwGetDefString(int iDef)
{
	if (iDef <= 0)	//1부터 시작함
	{
		return NULL;
	}

	const wchar_t *pText = 0;
	if(!GetDefString(iDef, pText))
	{
		return NULL;
	}

	static char pszNameText[255] = {0,}; 
	pszNameText[0] = '\0';
	strcpy_s(pszNameText, sizeof(pszNameText), MB(pText));

	return pszNameText;
}

lwWString lwGetMapNameW(unsigned int iMapNo)
{
	return GetMapName(iMapNo);
}

int lwGetMapKey(int const iMapNo)
{
	const TBL_DEF_MAP *pkDefMap = PgWorld::GetMapData(iMapNo);

	if ( pkDefMap )
	{
		return pkDefMap->iKey;
	}
	return 0;
}

int lwGetMapAttrByNo( int const iMapNo )
{
	const TBL_DEF_MAP *pkDefMap = PgWorld::GetMapData(iMapNo);

	if ( pkDefMap )
	{
		return pkDefMap->iAttr;
	}
	return GATTR_DEFAULT;
}

//void lwTempOhterSet(lwActor &Actor)
//{
//	PgActor* pkActor = Actor();
//	if(!pkActor) {return ;}
//	//if(!Actor()->InfoUI())
//	PgPilot *pkPilot = pkActor->GetPilot();
//	if(!pkPilot) {return ;}
//
//	BM::GUID kGuid = pkPilot->GetGuid();
//	XUI::CXUI_Wnd *pkWnd = NULL;
//	XUI::CXUI_Form *pkForm = NULL;
//	if(!XUIMgr.IsActivate(_T("OTHER_INFO"), pkWnd))
//	{
//		pkForm = dynamic_cast<XUI::CXUI_Form*>(XUIMgr.Call(_T("OTHER_INFO"), false));
//	}
//	else
//	{
//		pkForm = dynamic_cast<XUI::CXUI_Form*>(pkWnd);
//	}
//	
//	if(pkForm)
//	{
//		std::wstring kName = pkPilot->GetName();
//		PgStringUtil::EraseStr(kName, _T("\n"), kName);//케리지 리턴 삭제
//		pkForm->Text(kName);
//		//Actor()->InfoUI(form);
//		pkForm->OwnerGuid(kGuid);
//		lwUIWnd kWnd(pkForm);
//		kWnd.AdjustToOwnerPos();
//	}
//}

//void lwNpcSet(lwActor &Actor, char const* szUIID)
//{
//	if( !szUIID )
//	{return;}
//
//	PgActor* pkActor = Actor();
//	if( !pkActor )
//	{return;}
//
//	PgPilot *pkPilot = pkActor->GetPilot();
//	if( !pkPilot )
//	{return;}
//
//	std::wstring const kUIID = UNI(szUIID);
//
//	BM::GUID kGuid = pkPilot->GetGuid();
//
//	XUI::CXUI_Wnd *pkWnd = NULL;
//	if( !XUIMgr.IsActivate(kUIID, pkWnd) )
//	{
//		XUI::CXUI_Form *pkForm = dynamic_cast<XUI::CXUI_Form*>(XUIMgr.Call(kUIID, false));
//		if( pkForm )
//		{
//			std::wstring kName = pkPilot->GetName();
//			PgStringUtil::EraseStr(kName, _T("\n"), kName);//케리지 리턴 삭제
//			pkForm->Text(kName);
//
//			pkForm->OwnerGuid(kGuid);
//
//			lwUIWnd kWnd(pkForm);
//			kWnd.AdjustToOwnerPos();
//		}
//	}
//}

void lwDrawMissionRewardItem(lwUIWnd wnd)
{
	if (wnd.IsNil()) { return; }

	XUI::CXUI_Image *pkWnd = dynamic_cast<XUI::CXUI_Image*>( wnd() );

	if (pkWnd == NULL) { return; }
	PgBase_Item const & kItem = wnd.GetCustomDataAsSItem();

	PgUISpriteObject *pkSprite = g_kUIScene.GetIconTexture(kItem.ItemNo());

	pkWnd->Visible(0<kItem.ItemNo());

	if (!pkSprite) 
	{ 
		pkWnd->DefaultImgTexture(NULL);
		pkWnd->SetInvalidate();
		return; 
	}

	PgUIUVSpriteObject *pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);

	if(!pkUVSprite) { return; }

	pkWnd->DefaultImgTexture(pkUVSprite);

	SUVInfo &rkUV = pkUVSprite->GetUVInfo();
	pkWnd->UVInfo(rkUV);
	POINT2 kPoint(40*rkUV.U, 40*rkUV.V);//이미지 원본 사이즈 복구
	pkWnd->ImgSize(kPoint);
	pkWnd->SetInvalidate();
}

void lwOnCallMissionRewardItemToolTip(lwUIWnd kControl, bool bIgnoreEmpty)
{
	if (kControl.IsNil()) {	return; }
	XUI::CXUI_Wnd *pkWnd = kControl();
	if(pkWnd)
	{
		XUI::CXUI_Wnd* pHighestWnd = pkWnd;
		while( pHighestWnd->Parent() )
		{
			pHighestWnd = pHighestWnd->Parent();
		}

		PgBase_Item kItem = kControl.GetCustomDataAsSItem();
		if(!kItem.IsEmpty())
		{
			if( pHighestWnd->ID() == std::wstring(_T("SFRM_COIN_CHANGER")) )
			{
				if( kItem.EnchantInfo().UseTime() )
				{
					kItem.SetUseTime(kItem.EnchantInfo().TimeType(), kItem.EnchantInfo().UseTime());
				}
			}
			int iCloseToolTipItemNo = 90000000;
			if( kItem.ItemNo() != iCloseToolTipItemNo ) // "꽝" 아이템 툴팁을 없애준다
			{
				CallToolTip_SItem(&kItem,kControl.GetLocation());
			}
		}
		else if( bIgnoreEmpty )
		{
			std::wstring wstrText;
			std::wstring wstrLank;
			if(MakeToolTipText(kItem, wstrText, TBL_SHOP_IN_GAME::NullData(), wstrLank))
			{
				lwCallToolTipByText(kItem.ItemNo(), wstrText, kControl.GetLocation());
			}
		}
	}
	else
	{
		NILOG(PGLOG_ERROR, "kControl is NULL"); 
		assert(0);
	}
}

void lwCMP_STATE_CHANGE(int i)
{
	if(!g_pkWorld)
	{
		return;
	}

	if (PgMissionComplete::E_MSN_CPL_NONE != g_kMissionComplete.CompleteState())
	{
		g_kMissionComplete.TimeStamp(g_pkWorld->GetAccumTime());
		/*if (i == 0)
		{
			g_kMissionComplete.Start();
		}
		else*/
		{
			g_kMissionComplete.CompleteState((PgMissionComplete::E_MSN_CPL_STATE)i);
		}
	}
	else if (PgBossComplete::E_MSN_CPL_NONE != g_kBossComplete.CompleteState())
	{
		g_kBossComplete.TimeStamp(g_pkWorld->GetAccumTime());
		if (i == 0)
		{
			g_kBossComplete.Start();
		}
		else
		{
			g_kBossComplete.CompleteState((PgBossComplete::E_MSN_CPL_STATE)i);
		}
	}
	else if (PgExpeditionComplete::E_MSN_CPL_NONE != g_kExpeditionComplete.CompleteState())
	{
		g_kExpeditionComplete.TimeStamp(g_pkWorld->GetAccumTime());
		if (i == 0)
		{
			g_kExpeditionComplete.Start();
		}
		else
		{
			g_kExpeditionComplete.CompleteState((PgExpeditionComplete::E_MSN_CPL_STATE)i);
		}
	}
}

int lwGet_CMP_STATE()
{
	return (int)g_kMissionComplete.CompleteState();
}

bool lwSendSpendMoneyAction(const ESpendMoneyType kType, int iCustomData)
{
	if(0==iCustomData)
	{
		PgUtilAction_Revive kAction(kType,g_kPilotMan.GetPlayerPilot());
		return kAction.DoAction(true);
	}
	else	//코인으로 부활할 때의 동작
	{
		PgUtilAction_Revive kAction(ESMT_REVIVE_BY_INSURANCE, g_kPilotMan.GetPlayerPilot());
		return kAction.DoAction(true);
	}
	return true;
}

void lwAddNewScreenBreak(float fScreenX,float fScreenY)
{
	g_kScreenBreak.AddNewBreak(fScreenX,fScreenY);
}

//Sound
void lwSetBGMVolume(float fVolume, bool bConfig)
{
	g_kSoundMan.SetBGMVolume(fVolume, bConfig);
}
void lwSetEffectVolume(float fVolume, bool bConfig)
{
	g_kSoundMan.SetEffectVolume(fVolume, bConfig);
}

int lwChangeQuickSlotViewPage(int const iValue)
{
	if(!iValue){return 0;}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return 0;}
	
	PgQuickInventory *pkQInv = pkPlayer->GetQInven();

	if (pkQInv)
	{
		char cViewPage = 0;
		if(0 < iValue)
		{
			cViewPage = pkQInv->IncViewPage();
		}
		else
		{
			cViewPage = pkQInv->DecViewPage();
		}

		Send_PT_C_M_REQ_REGQUICKSLOT_VIEWPAGE(cViewPage);
		return cViewPage+1;
	}

	return 0;
}

int lwGetQuickSlotViewPage()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return 0;}

	PgQuickInventory *pkQInv = pkPlayer->GetQInven();

	if (pkQInv)
	{
		return pkQInv->ViewPage() + 1;
	}

	return 0;
}

void RefreshCharStateUI()
{
	lwUIWnd klwWnd = lwGetUIWnd("CharInfo");
	CUnit* pkUnit = g_kPilotMan.GetPlayerUnit();
	if (!pkUnit) { return; }
	if(!klwWnd.IsNil())
	{
		lwUIWnd  wndChar = klwWnd.GetControl("Charactor");
		lwCharInfo::lwSetCharInfoToUI(wndChar, pkUnit->GetID()); //캐릭 정보창 리셋.
		lwCharInfo::lwSetCharAbilInfo(klwWnd, 0, "L", false);
		lwCharInfo::lwSetCharAbilInfo(klwWnd, 0, "R", false);
	}

	klwWnd = lwGetUIWnd("SFRM_CHAR_INFO_DETAIL");
	if(!klwWnd.IsNil())
	{
		lwCharInfo::lwSetCharInfoToUI(klwWnd, pkUnit->GetID()); //캐릭 정보창 리셋.
	}
}

void lwNewTradeStart(lwGUID kTradeGuid, lwGUID kOtherGuid )
{
	g_kTradeMgr.NewTradeStart(kTradeGuid(), kOtherGuid());	
}

void lwOnTradeConfirmButton(bool bValue)
{
	g_kTradeMgr.OnConfirmButton(bValue);
}

lwGUID lwGetTradeGuid()
{
	return lwGUID(g_kTradeMgr.GetTradeGuid());
}

void lwSetTradeGuid(lwGUID kTradeGuid)
{
	g_kTradeMgr.SetTradeGuid(kTradeGuid());
}

void lwClearTradeInfo()
{
	g_kTradeMgr.Clear();
}


void lwUIWnd::SetEnable(bool bValue)
{
	if (self)
	{
		self->Enable(bValue);
	}
}

bool lwUIWnd::GetEnable()
{
	if (self)
	{
		return self->Enable();
	}

	return false;
}

bool lwUIWnd::IsTwinkle()
{
	if (self)
	{
		return self->IsTwinkle();
	}
	return false;
}

void lwUIWnd::OffTwinkle()
{
	if (self)
	{
		self->OffTwinkle();
	}
}

void lwUIWnd::OffTwinkleSelf()
{
	if (self)
	{
		self->SetTwinkle(false);
	}
}

bool lwUIWnd::NowTwinkleOn()
{
	if (self)
	{
		return self->NowTwinkleOn();
	}

	return false;
}

void lwUIWnd::SetTwinkle(unsigned int uiTwinkleTime, unsigned int uiInterTime)
{
	if( self )
	{
		self->TwinkleTime(uiTwinkleTime);
		if(0 < uiInterTime)
		{
			self->TwinkleInterTime(uiInterTime);
		}
		self->SetTwinkle(true);
	}
}

void lwUIWnd::ClearTwinkleInfo()
{
	if( self )
	{
		self->SetTwinkle(false);
		self->TwinkleStartTime(0);
		self->TwinkleInterTime(0);
		self->TwinkleTime(0);
	}
}

DWORD lwUIWnd::GetLiveTime(bool bSec)
{
	if ( self )
	{
		if ( bSec )
		{
			return (BM::GetTime32() - (self->CalledTime())) / 1000;
		}
		return BM::GetTime32() - (self->CalledTime());
	}
	return 0;
}

void lwUIWnd::RegistScript(char* szID, char* szValue)
{
	if (!self || !szID || !szValue)
	{
		return;
	}

	self->RegistScript(std::wstring(UNI(szID)), std::wstring(UNI(szValue)));
}

bool lwUIWnd::PtInUI(lwPoint2 pt)
{
	if ( self)
	{
		RECT rt;
		SetRect(&rt, self->TotalLocation().x, self->TotalLocation().y, 
			self->TotalLocation().x + self->Size().x, self->TotalLocation().y + self->Size().y);
		POINT kPoint;
		kPoint.x = pt.GetX();
		kPoint.y = pt.GetY();
		return (FALSE != PtInRect(&rt, kPoint));
	}

	return false;
}

lwUIWnd lwUIWnd::FindTopParent()
{
	if ( !self)
	{
		return lwUIWnd(NULL);
	}

	XUI::CXUI_Wnd *pkNow = self;
 
	while(pkNow) 
	{
		if (!pkNow->Parent())
		{
			break;
		}
		pkNow = pkNow->Parent();
	}

	return lwUIWnd(pkNow);
}

void lwUIWnd::ReLocationSnapChild()
{
	if( !self )
	{
		return;
	}

	PgFormSnapedHeadWnd *pkSelfWnd = dynamic_cast<PgFormSnapedHeadWnd *>(self);
	if( !pkSelfWnd )
	{
		return;
	}

	pkSelfWnd->ReLocationSnapChild();
}

void lwUIWnd::ClearRouletteInfo()
{
	PgFormRouletteWnd *pkRouletteWnd = dynamic_cast<PgFormRouletteWnd *>(self);
	if( pkRouletteWnd )
	{
		pkRouletteWnd->Clear();
	}
}

void lwUIWnd::SetRouletteStop()
{
	PgFormRouletteWnd *pkRouletteWnd = dynamic_cast<PgFormRouletteWnd *>(self);
	if( pkRouletteWnd )
	{
		pkRouletteWnd->SetStopKey();
	}
}

void lwUIWnd::StopMissionResult()
{
	g_kMissionComplete.StopMissionResult();
}

void lwUIWnd::InitMoveState()
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		pkMoveWnd->MovePercent(0.0f);
		pkMoveWnd->Location(pkMoveWnd->StartPoint().x, pkMoveWnd->StartPoint().y);
		pkMoveWnd->StartTime(g_pkApp->GetAccumTime());
		pkMoveWnd->IsPause(false);
		pkMoveWnd->StartPauseTime(0.0f);
		pkMoveWnd->WatingPauseTime(0.0f);
	}
}

float lwUIWnd::GetMovePercent()
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		return pkMoveWnd->MovePercent();
	}
	return 0.0f;
}

void lwUIWnd::SetMoveTime(float const fTime)
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		pkMoveWnd->TotalMoveTime(fTime);
	}
}

int lwUIWnd::GetMoveTime()const
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	return pkMoveWnd ? static_cast<int>(pkMoveWnd->TotalMoveTime()*1000) : 0;
}

bool lwUIWnd::IsMoveComplate()
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	return pkMoveWnd ? pkMoveWnd->IsMoveComplate() : false;
}

bool lwUIWnd::IsMovePause()
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		return pkMoveWnd->IsPause();
	}
	return false;
}

void lwUIWnd::SetModifyPoint1(lwPoint3 Pt)
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		pkMoveWnd->ModifyPoint1(Pt());
	}
}

void lwUIWnd::SetModifyPoint2(lwPoint3 Pt)
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		pkMoveWnd->ModifyPoint2(Pt());
	}
}

void lwUIWnd::SetStartPoint(lwPoint3 Pt)
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		pkMoveWnd->StartPoint(Pt());
	}
}

void lwUIWnd::SetEndPoint(lwPoint3 Pt)
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		pkMoveWnd->EndPoint(Pt());
	}
}

lwPoint3 lwUIWnd::GetModifyPoint1()
{
	NiPoint3 Pt;
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		Pt = pkMoveWnd->ModifyPoint1();
	}
	return lwPoint3(Pt);
}
lwPoint3 lwUIWnd::GetModifyPoint2()
{
	NiPoint3 Pt;
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		Pt = pkMoveWnd->ModifyPoint1();
	}
	return lwPoint3(Pt);
}
lwPoint3 lwUIWnd::GetStartPoint()
{
	NiPoint3 Pt;
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		Pt = pkMoveWnd->StartPoint();
	}
	return lwPoint3(Pt);
}
lwPoint3 lwUIWnd::GetEndPoint()
{
	NiPoint3 Pt;
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		Pt = pkMoveWnd->EndPoint();
	}
	return lwPoint3(Pt);
}

void lwUIWnd::lwSwapMovePoint()
{
	PgMoveAnimateWnd *pkMoveWnd = dynamic_cast<PgMoveAnimateWnd *>(self);
	if( pkMoveWnd )
	{
		pkMoveWnd->SwapMovePoint();
	}
}

bool lwUIWnd::SetSelect( lwGUID kGuid )
{
	XUI::CXUI_List2 *pkList = dynamic_cast<XUI::CXUI_List2*>(self);
	if ( pkList )
	{
		return pkList->SetSelect( kGuid() );
	}
	return false;
}

lwGUID lwUIWnd::GetSelectGuid()const
{
	XUI::CXUI_List2 *pkList = dynamic_cast<XUI::CXUI_List2*>(self);
	if ( pkList )
	{
		return lwGUID(pkList->GetSelecteGuid());
	}
	return lwGUID(BM::GUID::NullData());
}

void lwUIWnd::SetDisplayStartItem( size_t const iIndex )
{
	XUI::CXUI_List2 *pkList = dynamic_cast<XUI::CXUI_List2*>(self);
	if ( pkList )
	{
		pkList->SetDisplayStartItem( iIndex );
	}
}

void lwUIWnd::ConvertGuildInvTypeToIconGroup( const EInvType eInvType, EKindUIIconGroup& rkOutIconGroup )
{// 인벤타입 --> KUIG
	const int iBaseGuildIconGroup = static_cast<int>(KUIG_GUILD_INVENTORY1) - 1;
	rkOutIconGroup = static_cast<EKindUIIconGroup>(eInvType + iBaseGuildIconGroup);
}

void lwUIWnd::ConvertIconGroupToGuildInvType( const EKindUIIconGroup eIconGroup, EInvType& rkOutInvType )
{ // KUIG --> 인벤타입
	const int iBaseGuildIconGroup = static_cast<int>(KUIG_GUILD_INVENTORY1) - 1;
	rkOutInvType = static_cast<EInvType>( eIconGroup - iBaseGuildIconGroup );
}

void lwUIWnd::ConvertItemPosToGuildInvType( SItemPos& rkOutItemPos )
{// SItemPos 변환
	const int iBaseGuildIconGroup = static_cast<int>(KUIG_GUILD_INVENTORY1) - 1;
	rkOutItemPos.x = ( rkOutItemPos.x - static_cast<BYTE>(iBaseGuildIconGroup) );
}


//////////////////////////////////////////////////////////

void lwLoadingImage(int iMapIndex, EGndAttr const eMapType, int LoadingImageID, bool bIsRidingPet, int iOldMapNo)
{
	//lwCallUI("FRM_LOADING_IMG");
	lwUIWnd lwWnd = lwGetUIWnd("FRM_LOADING_IMG");
	if(lwWnd.IsNil())
	{
		return;
	}

	{//기존 로딩화면을 초기화한다.
		LOADINGIMG_ID_LIST	kList;
		g_kUIScene.GetLoadingImgeIDList(kList);
		LOADINGIMG_ID_LIST::iterator	iter = kList.begin();
		while( iter != kList.end() )
		{
			lwUIWnd LoadingWnd = lwWnd.GetControl(iter->c_str());
			if( !LoadingWnd.IsNil() )
			{
				LoadingWnd.Visible(false);
			}
			iter = kList.erase(iter);
		}
	}

	char const* pImageName = g_kUIScene.FindLoadingImg(iMapIndex, eMapType, LoadingImageID, bIsRidingPet, iOldMapNo);// 특별한 로딩화면이 필요한 경우의 데이타
	if( pImageName == NULL )
	{
		return;
	}
	lwUIWnd loadingWnd = lwWnd.GetControl(pImageName);
	if(loadingWnd.IsNil())
	{
		return;
	}

	loadingWnd.Visible(true);
}

void RegistUIAction(CXUI_Wnd* pkTopWnd, std::wstring const& rkScript)
{
	if( pkTopWnd )
	{
		PgActor *pkActor = g_kPilotMan.GetPlayerActor();
		if( pkActor )
		{
			g_kUIScene.RegistUIAction( pkTopWnd->ID(), rkScript, pkActor->GetTranslate());
		}
	}
}

void lwRegistUIAction(lwUIWnd kWnd, char const* szScript)
{
	if( szScript )
	{
		RegistUIAction(kWnd(), std::wstring(UNI(szScript)));
	}
}

//번호는 연결되는 이름이 없는 것, 현재 타입에 연결되는 이름을 찾는다.
char const g_szEqupName[31][12] = {"HAIR_COLOR", "HAIR", "FACE", "SHOULDER", "BACK",
									"GLASS", "WEAPON", "SHIELD", "NECKLACE", "EARING", 
									"L_RING", "BELT", "STONE","13", "14", 
									"15", "16", "17", "18", "19",
									"HEAD", "JACKET", "PANTS", "FOOT", "GLOVE", 
									"25", "26", "KICKBALL", "28", "29", "R_RING"};

extern int const MINIMUM_ENDURANCE_VALUE = 5;
bool lwRefreshRepairAlarmUI()
{
	T_GNDATTR const kGndAttr = g_pkWorld ? g_pkWorld->GetAttr() : 0;
	if( (GATTR_FLAG_EMPORIABATTLE&kGndAttr) )
	{
		return false;
	}

	PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
	if(!pPilot){return false;}

	PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pPilot->GetUnit());
	if(!pkPlayer){return false;}
	PgInventory *pkInv = pkPlayer->GetInven();
	if (!pkInv)
	{
		return false;
	}
/*	static int iEquipArr[9] = {EQUIP_POS_CLOAK, EQUIP_POS_WEAPON, EQUIP_POS_SHEILD,
		EQUIP_POS_HELMET, EQUIP_POS_SHOULDER, EQUIP_POS_SHIRTS, EQUIP_POS_GLOVE, EQUIP_POS_PANTS, 
		EQUIP_POS_BOOTS};*/

	int i = 0;

	XUI::CXUI_Wnd* pkWnd = NULL;
	XUI::CXUI_Wnd* pkChild = NULL;
	XUI::CXUI_Wnd* pkMan = NULL;
	XUI::CXUI_Wnd* pkEquip = NULL;
	
	float const fLimit = 0.2f;
	char szClass[255] = {0,};
	char szEquip[255] = {0,};
	int iCloseUI = 0;

	BaseItemMap kMap;
	FindRepairInfo(kMap, pkInv);
	/*GET_DEF(CItemDefMgr, kItemDefMgr);
	for (i = 0;  i< iEquipArrSize; ++i)
	{
		PgBase_Item kItem;;
		if(S_OK == pkInv->GetItem(SItemPos(IT_FIT, iEquipArr[i]), kItem))
		{
			CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if (pDef)
			{
				if (MINIMUM_ENDURANCE_VALUE >= kItem.Count())
				{
					kMap.insert(std::make_pair(i, iEquipArr[i]));//필요한 것들만 모으자
				}
			}
		}
	}*/

	if (kMap.empty())
	{
		lwCloseUI("FRM_REPAIR_ALARM");
	}
	else
	{
		pkWnd = lwActivateUI("FRM_REPAIR_ALARM").GetSelf();
		if (pkWnd)
		{
			pkWnd->TwinkleTime(UINT_MAX);
			pkWnd->SetTwinkle(true);
			pkWnd->TwinkleStartTime(BM::GetTime32());
			int const iBase = pPilot->GetBaseClassID();
			BM::vstring vStr("FRM_");
			switch( iBase )
			{
			case UCLASS_FIGHTER:
				{
					vStr+="FIGHTER";
				}break;
			case UCLASS_MAGICIAN:
				{
					vStr+="MAGICIAN";
				}break;
			case UCLASS_ARCHER:
				{
					vStr+="ARCHER";
				}break;
			case UCLASS_THIEF:
				{
					vStr+="THIFE";
				}break;
			case UCLASS_SHAMAN:
				{
					vStr+="SHAMAN";
				}break;
			case UCLASS_DOUBLE_FIGHTER:
				{
					vStr+="DOUBLE_FIGHTER";
				}break;
			}
			pkChild = pkWnd->GetControl( vStr );
			pkMan = pkWnd->GetControl(_T("FRM_MAN"));
			for (i = 0; i < iEquipArrSize; ++i)
			{
				BaseItemMap::const_iterator it = kMap.find(i);
				bool const bFound = kMap.end() != it;

				if (   iEquipArr[i] == EQUIP_POS_CLOAK 
					|| iEquipArr[i] == EQUIP_POS_WEAPON 
					|| iEquipArr[i] == EQUIP_POS_SHEILD  
					|| iEquipArr[i] == EQUIP_POS_KICKBALL)
				{
					if(pkChild)
					{
						_snprintf(szClass, 254, "IMG_%s", g_szEqupName[__max(0,iEquipArr[i])]);
						pkEquip = pkChild->GetControl(UNI(szClass));
						if (pkEquip)
						{
							SUVInfo kUVInfo= pkEquip->UVInfo();
							kUVInfo.Index = int(bFound) +1;
							pkEquip->UVInfo(kUVInfo);
							pkEquip->TwinkleTime(UINT_MAX);
							pkEquip->SetTwinkle(bFound);
							if(bFound)
							{
								pkEquip->TwinkleInterTime(500);
								pkEquip->TwinkleStartTime(pkWnd->TwinkleStartTime());
							}
						}
					}
				}
				else
				{
					if (pkMan)
					{
						_snprintf(szEquip, 254, "IMG_%s", g_szEqupName[__max(0,iEquipArr[i])]);
						pkEquip = pkMan->GetControl(UNI(szEquip));
						if (pkEquip)
						{
							BYTE byIsAccessory = 0;
							pkEquip->GetCustomData(&byIsAccessory, sizeof(BYTE));
							if(byIsAccessory > 0) //악세서리는 텍스처 인덱스로 사용
							{
								pkEquip->Visible(true);
								SUVInfo kUVInfo= pkEquip->UVInfo();
								kUVInfo.Index = int(bFound) +1;
								pkEquip->UVInfo(kUVInfo);
							}
							else
							{
								pkEquip->Visible(bFound);
							}
							pkEquip->TwinkleTime(UINT_MAX);
							pkEquip->SetTwinkle(bFound);
							if(bFound)
							{
								pkEquip->TwinkleInterTime(500);
								pkEquip->TwinkleStartTime(pkWnd->TwinkleStartTime());
							}
						}
					}
				}
			}
			pkWnd->SetInvalidate();

			g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_REPAIR), 0);
		}
	}	

	return true;
}	

bool lwIsOpenGroundByNo(int iNo)
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if ( pkPlayer )
	{
		return SUCCEEDED(pkPlayer->IsOpenWorldMap( iNo ));
	}
	return false;
}

int lwGetAlphaFromImgAtCursor(char const* szPath, lwUIWnd kWnd)
{
	if (kWnd.IsNil()) {return 0;}
	const XUI::CXUI_Wnd *pkWnd = kWnd.GetSelf();

	POINT2 kPt;
	kPt.x = XUIMgr.MousePos().x-pkWnd->TotalLocation().x;
	kPt.y = XUIMgr.MousePos().y-pkWnd->TotalLocation().y;

	if(kPt.x < 0 || kPt.y < 0 || kPt.x >= pkWnd->Size().x || kPt.y >= pkWnd->Size().y)
	{
		return 0;
	}

	return g_kWorldMapUI.GetAlphaFromImg(szPath, kPt.x, kPt.y);
}

extern int g_bToggleUIDraw;

void lwHideUI(bool bHide)
{
	if (bHide)
		g_bToggleUIDraw = 0;
	else
		g_bToggleUIDraw = 3;
}

void lwHidePartUI(char const* szHideFromID, bool const bHide)
{
	if( bHide )
	{
		XUI::ContWndIDList	kList;
		kList.push_back(UNI(szHideFromID));
		XUIMgr.HidePartUI(UNI(szHideFromID), kList); 
	}
	else
	{
		XUIMgr.ShowPartUI(UNI(szHideFromID));
	}
}

void lwCallMapInfo(int iNo)
{
	if (0>=iNo)
	{
		return;
	}
	g_kWorldMapUI.ClickedMapNo(iNo);
	XUIMgr.Call(_T("FRM_WORLDMAP_INFORMATION"));
}

void lwDrawPlayerLv(lwUIWnd UISelf)
{
	if(!UISelf.IsNil())
	{
		PgPilot	*pPilot = g_kPilotMan.GetPlayerPilot();
		if(pPilot != NULL)
		{
			BM::vstring vstrText;
			int iLv = pPilot->GetAbil(AT_BATTLE_LEVEL);
			if ( !iLv )
			{
				iLv	= pPilot->GetAbil(AT_LEVEL);
			}
			else
			{
				vstrText = L"{C=0xFFFF0000/}";
			}

			vstrText += iLv;
			UISelf.SetStaticTextW(lwWString(vstrText.operator wchar_t const*()));
		}
	}
}

void lwShowMiniMap(lwUIWnd UISelf)
{
	lwUIWnd	MapWnd		= lwGetUIWnd("FRM_MINIMAP");
	lwUIWnd	MapMiniWnd	= lwGetUIWnd("MiniMapMini");
	lwUIWnd	GamingWnd	= lwGetUIWnd("FRM_GAMING_RANDOM");

	if(MapWnd.IsNil() && MapMiniWnd.IsNil() && GamingWnd.IsNil())
	{
		UISelf.Visible(false);
	}
}

void ConverterValueToString(int iAbilType, int iAbilValue, std::wstring& wstrText)
{
	switch(iAbilType)
	{
	case AT_SKILL_ATT:
		{
			if(SAT_MAINTENANCE_CAST & iAbilValue)
			{
				wstrText += UNI("Maintenance");
			}
			if(SAT_CLIENT_CTRL_PROJECTILE & iAbilValue)
			{
				wstrText += UNI("Projectile ");
			}
			if(SAT_PHYSICS & iAbilValue)
			{
				wstrText += UNI("Physics ");
			}
			if(SAT_MAGIC & iAbilValue)
			{
				wstrText += UNI("Magic ");
			}
			if(SAT_DEFAULT & iAbilValue)
			{
				wstrText += UNI("Default Skill ");
			}
		}break;
	case AT_CASTTYPE:
		{
			if(E_SCAST_INSTANT & iAbilValue)
			{
				wstrText += UNI("INSTANT ");
			}
			if(E_SCAST_INSTANT & iAbilValue)
			{
				wstrText += UNI("CASTSHOT ");
			}

			if(2 == iAbilValue)
			{
				TCHAR szValue[32] = {0,};
				_itow_s(iAbilValue, szValue, 32, 10);
				wstrText += szValue;
			}
		}break;
	case AT_SKILL_KIND:
		{
			TCHAR szValue[32] = {0,};
			_itow_s(iAbilValue, szValue, 32, 10);
			wstrText += szValue;
		}break;
	default:
		{
			TCHAR szValue[32] = {0,};
			_itow_s(iAbilValue, szValue, 32, 10);
			wstrText += szValue;
		}break;
	}

}
void AddStringTypeAndValue(std::wstring& wstr, const wchar_t * wcharType, int iValue, bool bUseNULL = true)
{
	if(bUseNULL)
	{
		if(0 == iValue)
		{
			return;
		}
	}

	TCHAR szBuff[64] = {0,};

	wstr += wcharType;
	wstr += UNI(" : ");
	_itow_s(iValue, szBuff, 64, 10);
	wstr += szBuff;
	wstr += UNI("\n");
}

void AddClassLimitText(__int64 i64ClassLimit, std::wstring &wstrText)
{
	wstrText += UNI("ClassLimit : ");

	if(UCLIMIT_ALL == i64ClassLimit)//모든 직업
	{
		wstrText += TTW(30253);
		wstrText += UNI("\n");
		return;
	}
	else
	{
		for(int i = 1; i <= 36; ++i)
		{
			if(1i64<<i & i64ClassLimit)
			{
				wstrText += TTW(30000+i);
				wstrText += UNI(" ");
			}
		}

		if(0 == i64ClassLimit)
		{
			wstrText += UNI("UnDefinition");
		}

		wstrText += UNI("\n");
	}
}

void AddWeaponLimitText(int iWeaponLimit, std::wstring &wstrText)
{
	wstrText += UNI("WeaponLimit : ");

	for(int i = 1; i <= 8; ++i)
	{
		if(1<<(i-1) & iWeaponLimit)
		{
			wstrText += TTW(2000+i);
			wstrText += UNI(" ");
		}
	}

	if(0 == iWeaponLimit)
	{
		wstrText += UNI("UnDefinition");
	}

	wstrText += UNI("\n");
}

void AddTargetTypeText(int iTargetType, std::wstring &wstrText)
{
	wstrText += UNI("TargetType : ");

	if(iTargetType & ESTARGET_SELF)
	{
		wstrText += UNI("Me");
		wstrText += UNI(" ");
	}

	if(iTargetType & ESTARGET_ENEMY)
	{
		wstrText += UNI("Enemy");
		wstrText += UNI(" ");
	}

	if(iTargetType & ESTARGET_PARTY)
	{
		wstrText += TTW(400105); //파티
		wstrText += UNI(" ");
	}

	if(iTargetType & ESTARGET_LOCATION)
	{
		wstrText += UNI("Position");
		wstrText += UNI(" ");
	}

	if(iTargetType & ESTARGET_HIDDEN)
	{
		wstrText += UNI("Hidden");
		wstrText += UNI(" ");
	}

	if(iTargetType & ESTARGET_DEAD)
	{
		wstrText += UNI("Dead");
		wstrText += UNI(" ");
	}

	if(iTargetType & ESTARGET_ALLIES)
	{
		wstrText += UNI("Allies");
		wstrText += UNI(" ");
	}

	if(iTargetType & ESTARGET_COUPLE)
	{
		wstrText += TTW(450025); //커플
		wstrText += UNI(" ");
	}

	if(iTargetType & ESTARGET_GUILD)
	{
		wstrText += TTW(400106); //길드
		wstrText += UNI(" ");
	}

	if(0 == iTargetType)
	{
		wstrText += UNI("UnDefinition");
	}
	
	wstrText += UNI("\n");
}

void MakeSkillDefaultAbilToString(const CSkillDef* pkSkillDef, std::wstring& wstr, bool bUseNULL)
{
	TCHAR szBuff[64] = {0,};

	//Level
	AddStringTypeAndValue(wstr, TTW(46).c_str(), pkSkillDef->m_byLv, bUseNULL);
	//ActionName
	wstr += UNI("ActionName : ");
	wstr += pkSkillDef->GetActionName();
	wstr += UNI("\n");

	//Type
	wstr += TTW(53); //Skill Type
	wstr += UNI(" : ");
	switch(pkSkillDef->GetType())
	{
	case EST_GENERAL :
		{
			wstr += TTW(49); //GENERAL
		}break;
	case EST_ACTIVE :
		{
			wstr += TTW(50); //ACTIVE
		}break;
	case EST_PASSIVE :
		{
			wstr += TTW(51); //PASSIVE
		}break;
	case EST_TOGGLE :
		{
			wstr += TTW(52); //TOGGLE
		}break;
	default:
		{
			wstr += UNI("UnDefinition");
		}break;
	}
	wstr += UNI("\n");

	AddClassLimitText(pkSkillDef->m_i64ClassLimit, wstr);
	AddWeaponLimitText(pkSkillDef->m_iWeaponLimit, wstr);
	AddTargetTypeText(pkSkillDef->GetTargetType(), wstr);

	AddStringTypeAndValue(wstr, UNI("LevelLimit"),		pkSkillDef->m_sLevelLimit,		bUseNULL);
	AddStringTypeAndValue(wstr, UNI("StateLimit"),		pkSkillDef->m_iStateLimit,		bUseNULL);
	AddStringTypeAndValue(wstr, UNI("ParentSkill"),		pkSkillDef->GetParentSkill(),	bUseNULL);	
	AddStringTypeAndValue(wstr, UNI("SkillRange"),		pkSkillDef->m_sRange,			bUseNULL);
	AddStringTypeAndValue(wstr, TTW(47).c_str(),		pkSkillDef->m_sCastTime,		bUseNULL);
	AddStringTypeAndValue(wstr, TTW(48).c_str(),		pkSkillDef->m_sCoolTime,		bUseNULL);
	AddStringTypeAndValue(wstr, UNI("AnimationTime"),	pkSkillDef->m_iAnimationTime,	bUseNULL);
	AddStringTypeAndValue(wstr, TTW(60).c_str(),		pkSkillDef->m_sMP,				bUseNULL);
	AddStringTypeAndValue(wstr, TTW(61).c_str(),		pkSkillDef->m_sHP,				bUseNULL);
	AddStringTypeAndValue(wstr, UNI("EffectID"),		pkSkillDef->GetEffectNo(),		bUseNULL);
}

void MakeEffectDefaultAbilToString(const CEffectDef* pkEffectDef, std::wstring& wstr, bool bUseNULL)
{
	TCHAR szBuff[64] = {0,};

	//ActionName
	wstr += UNI("ActionName : ");
	wstr += pkEffectDef->GetActionName();
	wstr += UNI("\n");

	//Type
	wstr += UNI("EffectType : ");
	switch(pkEffectDef->GetType())
	{
	case EFFECT_TYPE_BLESSED :
		{
			wstr += UNI("BLESSED");			
		}break;
	case EFFECT_TYPE_CURSED :
		{
			wstr += UNI("CURSED");
		}break;
	case EFFECT_TYPE_PASSIVE :
		{
			wstr += UNI("PASSIVE");
		}break;
	case EFFECT_TYPE_NORMAL :
		{
			wstr += UNI("NORMAL");
		}break;
	case EFFECT_TYPE_PENALTY :
		{
			wstr += UNI("PENALTY");
		}break;
	case EFFECT_TYPE_ITEM :
		{
			wstr += UNI("ITEM");
		}break;
	case EFFECT_TYPE_TEMP :
		{
			wstr += UNI("TEMP");
		}break;
	default:
		{
			wstr += UNI("UnDefinition");
		}break;
	}
	wstr += UNI("(");
	TCHAR szType[32] = {0,};
	_itow_s(pkEffectDef->GetType(), szType, 32, 10);
	wstr += szType;
	wstr += UNI(")");
	wstr += UNI("\n");

	AddStringTypeAndValue(wstr, UNI("Interval"),		pkEffectDef->GetInterval(),		bUseNULL);
	AddStringTypeAndValue(wstr, UNI("DurationTime"),	pkEffectDef->GetDurationTime(),	bUseNULL);
	AddStringTypeAndValue(wstr, UNI("Toggle"),			pkEffectDef->GetToggle(),		bUseNULL);	
}

void MakeAbilObjectToString(const CAbilObject* pkAbilObject, std::wstring& wstr, bool bUsedDefAbilType)
{
	if(pkAbilObject)
	{
		SAbilIterator itor;
		pkAbilObject->FirstAbil(&itor);

		while(pkAbilObject->NextAbil(&itor))
		{
			
			//AbilType
			TCHAR szType[32] = {0,};
			_itow_s(itor.wType, szType, 32, 10);
			wstr += szType;

			//DefAbilType
			if(bUsedDefAbilType)
			{				
				const TCHAR *pText = NULL;
				GetAbilName(itor.wType, pText);
				if(pText)
				{
					wstr += UNI("(");
					if(pText)
					{
						wstr += pText;
					}
					wstr += UNI(")");
				}
			}

			wstr += UNI(" : ");

			//AbilValue
			ConverterValueToString(itor.wType, itor.iValue, wstr);
			
			wstr += UNI("\n");
		}
	}
}

void lwItemSelfProcessIconToolTip(lwUIWnd UISelf)
{
	if (UISelf.IsNil()) {	return; }
	XUI::CXUI_Wnd*	pWnd = UISelf.GetSelf();

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}

	SItemPos	rkPos;
	pWnd->GetCustomData(&rkPos, sizeof(rkPos));

	PgBase_Item kItem;
	if(S_OK == pkPlayer->GetInven()->GetItem(rkPos, kItem))
	{
		CallToolTip_SItem(&kItem, UISelf.GetTotalLocation());
	}
}

__int64 GetNowFran()
{
	__int64 iNow = 0;
	__int64 iMax = 0;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(pkPlayer)
	{

		// Fran = Level + 1
		int iLv = pkPlayer->GetAbil(AT_LEVEL) + 1;// 100;
		int const iClass = pkPlayer->GetAbil(AT_CLASS);
		__int64 iFran  = pkPlayer->GetAbil64(AT_FRAN);

		GET_DEF(PgClassDefMgr, kClassDefMgr);
		__int64 iBegin = kClassDefMgr.GetAbil64(SClassKey(iClass, iLv), AT_EXPERIENCE);
		__int64 iNextFran = kClassDefMgr.GetAbil64(SClassKey(iClass, iLv+1), AT_EXPERIENCE);

		if(iNextFran > 0 
			&& iBegin > 0)
		{
			iNow = __max(0, iFran);// 100;
			iMax = __max(0, iNextFran);
		}
		else
		{
			iNow = iFran;
			iMax = iNextFran;
		}
	}
	return iNow;
}

void lwUIWnd::ConnectedInfo(int const iMapNo, int const iTickTime)
{
	if (g_pkApp == NULL)
		return;

	static float fTime = g_pkApp->GetAccumTime() + (float)iTickTime;
	static bool bIsShow = false;

	if( g_pkApp->GetAccumTime() > fTime )
	{
		fTime = g_pkApp->GetAccumTime() + (float)iTickTime;
		bIsShow = !bIsShow;

		std::wstring wStr;
		if( bIsShow )
		{
			wStr = GetMapName(iMapNo);
		}
		else
		{
			short const ChannelNo = g_kNetwork.NowChannelNo();
			if( EXPEDITION_CHANNEL_NO == ChannelNo )
			{ // 원정대 채널
				wStr = TTW(710037);
			}
			else
			{ // 비원정대 채널
				wchar_t szTemp[MAX_PATH] = {0,};
				swprintf_s(szTemp, MAX_PATH, TTW(460001).c_str(), ChannelNo);
				wStr = szTemp;
			}
		}

		lwWString	lwStr(wStr);
		this->SetStaticTextW(lwStr);
	}
}

void lwUIWnd::SetModal(bool bModal)
{
	if (self)
	{
		self->IsModal(bModal);
	}
}

lwWString lwUIWnd::GetDefaultImgName()
{
	if(self)
	{
		return lwWString(MB(self->DefaultImgName()));
	}

	return lwWString("");
}

void lwReCallQuickSlotEx()
{
	if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME) )
	{
		return;
	}

	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("QuickInv"));
	if( !pWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pForm = pWnd->GetControl(_T("FRM_BG"));
	if( pForm )
	{
		XUI::CXUI_Wnd* pOpen = pForm->GetControl(_T("BTN_OPEN_EX_SLOT"));		
		XUI::CXUI_Wnd* pClose = pForm->GetControl(_T("BTN_CLOSE_EX_SLOT"));		
		if( !pOpen || !pClose )
		{
			return;
		}

		if( lwGetExtendSlotView() )
		{
			lwCallQuickSlotEx(false);
		}
		pOpen->Visible(!lwGetExtendSlotView());
		pClose->Visible(lwGetExtendSlotView());
	}
}

void lwCallQuickSlotEx(bool const bChange)
{
	bool bCloseExSlot = false;

	if( g_pkWorld
	&&	g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME) )
	{
		return;
	}

	XUI::CXUI_Wnd* QuickSlot = XUIMgr.Get(L"QuickInv");
	if( !QuickSlot )
	{
		return;
	}

	XUI::CXUI_Wnd* SlotBG = QuickSlot->GetControl(L"FRM_BG");
	if( !SlotBG )
	{
		return;
	}

	XUI::CXUI_Wnd* pClose = SlotBG->GetControl(L"BTN_CLOSE_EX_SLOT");
	XUI::CXUI_Wnd* pOpen = SlotBG->GetControl(L"BTN_OPEN_EX_SLOT");
	if( !pClose || !pOpen )
	{
		return;
	}

	XUI::CXUI_Wnd* pWnd = XUIMgr.Call(_T("QuickInvEx"));
	if( pWnd )
	{
		if( bChange )
		{
			lwSetExtendSlotView(!lwGetExtendSlotView());
			pWnd->Visible(lwGetExtendSlotView());
		}
		else
		{
			pWnd->Visible(lwGetExtendSlotView());
		}
	}
	pOpen->Visible(!pWnd->Visible());
	pClose->Visible(pWnd->Visible());

	bool bChangePos = true;
	if( !g_kChatMgrClient.CheckChatOut() )
	{	
		int iGabY = XUIMgr.GetResolutionSize().y - XUI::EXV_DEFAULT_SCREEN_HEIGHT;

		XUI::CXUI_Wnd*	pChatOut = XUIMgr.Activate(L"ChatOut");
		if( !pChatOut)
		{
			return;
		}
		POINT3I kChatOutPos;
		kChatOutPos.x = kChatLoc[ECS_COMMON].iX;
		kChatOutPos.y = (false == pWnd->Visible())?( kChatLoc[ECS_COMMON].iY + iGabY ):( kChatLoc[ECS_COMMON].iY + iGabY - pWnd->Size().y );
		if(pChatOut)
		{
			POINT3I kCurPos = pChatOut->Location();
			kCurPos.y += pChatOut->Size().y;

			int iDist = POINT3I::Distance(QuickSlot->TotalLocation(), kCurPos);
			int const ATTACH_LIMIT = 100;
			if(ATTACH_LIMIT<iDist)
			{
				kChatOutPos.x = kCurPos.x;
				kChatOutPos.y = kCurPos.y;
				bChangePos = false;
			}
		}

		ChatInitUtil::ResizeChatWindow(kChatOutPos.x, kChatOutPos.y,
		kChatLoc[ECS_COMMON].iWidth, kChatLoc[ECS_COMMON].iHeight, bChangePos);
	}
}

void lwCallChatWindow(int const iSetTap)
{
	if( g_kChatMgrClient.CheckChatOut() )
	{
		return;
	}

	lua_tinker::call<void, int>("ChatSetPreset", 1);
	//lua_tinker::call<void, lwUIWnd, int>("SetSysoutOption", lwUIWnd(XUIMgr.Get(_T("SFRM_CHAT_OPTION"))), 2);

	int const iChatStation = g_kChatMgrClient.ChatStation();

	bool bInitComplate = false;
	const int iGabY = XUIMgr.GetResolutionSize().y - XUI::EXV_DEFAULT_SCREEN_HEIGHT;

	if( (iChatStation == ECS_COMMON) )
	{
		XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("QuickInvEx"));
		if( pWnd && !pWnd->IsClosed() )
		{
			ChatInitUtil::ResizeChatWindow(kChatLoc[iChatStation].iX, kChatLoc[iChatStation].iY + iGabY - pWnd->Size().y, kChatLoc[iChatStation].iWidth, kChatLoc[iChatStation].iHeight);
			bInitComplate = true;
		}
	}

	if( !bInitComplate )
	{
		POINT2 kAdd(kChatLoc[iChatStation].iX, kChatLoc[iChatStation].iY + iGabY);
		if(iChatStation==ECS_PVPLOBBY || iChatStation==ECS_CASHSHOP)
		{
			POINT2 kDelta = XUIMgr.GetResolutionSize() - POINT2(EXV_DEFAULT_SCREEN_WIDTH, EXV_DEFAULT_SCREEN_HEIGHT);
			kDelta.x/=2;
			kDelta.y/=2;
			
			kAdd.x = kChatLoc[iChatStation].iX + kDelta.x;
			kAdd.y = kChatLoc[iChatStation].iY + kDelta.y;
		}
		XUI::CXUI_Wnd* pChatOut = XUIMgr.Get(_T("ChatOut"));
		XUI::CXUI_Wnd* pSysChatOut = XUIMgr.Get(_T("SysChatOut"));
		if(pChatOut && pSysChatOut )
		{
			pChatOut->CanDrag(ECS_COMMON==iChatStation);
			if( (ECS_COMMON != iChatStation) )
			{
				if(NULL==XUIMgr.Get(L"FRM_CASH_SHOP"))
				{
					g_kChatMgrClient.SetSavedPos(pChatOut->Location());
					g_kChatMgrClient.SetSystemSavedPos(pSysChatOut->Location());
				}
			}
		}
		ChatInitUtil::ResizeChatWindow(kAdd.x, kAdd.y, kChatLoc[iChatStation].iWidth, kChatLoc[iChatStation].iHeight);
	}

	XUI::CXUI_Wnd* pChatOut = XUIMgr.Get(_T("ChatOut"));
	if( !pChatOut )
	{
		return;
	}

	if( iChatStation == ECS_COMMON )
	{
		g_kChatMgrClient.ApplyChatOutSavedPos(pChatOut);
		lua_tinker::call<void, int>("ChangeChatOutSize", 0);
	}

	XUI::CXUI_Wnd* pSysChatOut = XUIMgr.Get(_T("SysChatOut"));
	if( !pSysChatOut )
	{
		return;
	}

	POINT3I ChatPos = pChatOut->Location();

	g_kChatMgrClient.SetSystemSavedPos(POINT2(ChatPos.x, ChatPos.y - pSysChatOut->Size().y));
	g_kChatMgrClient.ApplySystemChatOutSavedPos(pSysChatOut);

	XUI::CXUI_Wnd* pChatBar = XUIMgr.Get(_T("ChatBar"));
	if( !pChatBar || !pChatOut )
	{
		return;
	}


	XUI::CXUI_Button* pTemp = dynamic_cast<XUI::CXUI_Button*>(pChatBar->GetControl(L"BTN_TELL_TYPE"));
	if( pTemp )
	{
		pTemp->Disable(((ECS_CASHSHOP == iChatStation) || (ECS_PVPLOBBY == iChatStation))?(true):(false));
	}


	XUI::CXUI_Wnd* pBtnParent = pChatOut->GetControl(L"CHAT_BG");
	if( pBtnParent )
	{
		XUI::CXUI_Button* pBtnOption = dynamic_cast<XUI::CXUI_Button*>(pBtnParent->GetControl(L"BTN_FILTER_OPTION"));
		XUI::CXUI_Button* pEmoticon = dynamic_cast<XUI::CXUI_Button*>(pBtnParent->GetControl(L"BTN_EMOTICON"));
		XUI::CXUI_Button* pEmotion = dynamic_cast<XUI::CXUI_Button*>(pBtnParent->GetControl(L"BTN_EMOTION"));
		
		XUI::CXUI_Wnd* pMainBg = pBtnParent->GetControl(L"SFRM_CHATOUT_BG");
		if( !pMainBg || !pBtnOption || !pEmoticon || !pEmotion )
		{ 
			return; 
		}
		pBtnOption->Disable((ECS_COMMON != iChatStation)?(true):(false));
		pEmoticon->Disable((ECS_COMMON != iChatStation)?(true):(false));
		pEmotion->Disable((ECS_COMMON != iChatStation)?(true):(false));


		XUI::CXUI_CheckButton* pBtnNormal = dynamic_cast<XUI::CXUI_CheckButton*>(pBtnParent->GetControl(L"BTN_CHATFILTER_CHAT"));
		XUI::CXUI_CheckButton* pBtnTrade = dynamic_cast<XUI::CXUI_CheckButton*>(pBtnParent->GetControl(L"BTN_CHATFILTER_TRADE"));
		XUI::CXUI_CheckButton* pBtnGuild = dynamic_cast<XUI::CXUI_CheckButton*>(pBtnParent->GetControl(L"BTN_CHATFILTER_GUILD"));
		XUI::CXUI_CheckButton* pBtnParty = dynamic_cast<XUI::CXUI_CheckButton*>(pBtnParent->GetControl(L"BTN_CHATFILTER_PARTY"));
		if( !pBtnGuild || !pBtnTrade || !pBtnNormal || !pBtnParty )
		{ 
			return; 
		}

		int iOldChatMode = 1;
		if( true == pBtnNormal->Check() )
		{
			iOldChatMode = 1;
		}
		else if( true == pBtnGuild->Check() )
		{
			iOldChatMode = 2;
		}
		else if( true == pBtnParty->Check() )
		{
			iOldChatMode = 3;
		}
	
		else if( true == pBtnTrade->Check() )
		{
			iOldChatMode = 4;
		}

		g_kChatMgrClient.OldChatMode(iOldChatMode);

		//pBtnParty->Visible((ECS_CASHSHOP != iChatStation)?(true):(false));
		//pBtnGuild->Visible((ECS_CASHSHOP != iChatStation)?(true):(false));
		pBtnTrade->Visible(ECS_PVPLOBBY != iChatStation?(true):(false));

		XUI::CXUI_Wnd* pBtnBgParent = pBtnParent->GetControl(L"FRM_COLOR_BG");
		if( pBtnBgParent )
		{
			for(int i = 0; i < 4; ++i)
			{
				BM::vstring	vStr(L"IMG_BTN_BG");
				vStr += i;
				XUI::CXUI_Wnd* pWnd = pBtnBgParent->GetControl(vStr);
				if( pWnd && i != 0)
				{
					if( 3 > i )
					{
						//pWnd->Visible((ECS_CASHSHOP != iChatStation)?(true):(false));
					}
					else
					{
						pWnd->Visible(ECS_PVPLOBBY  != iChatStation?(true):(false));
					}
				}
			}
		}
	}
	lua_tinker::call<void, lwUIWnd, int>("ChatMode_FilterSet", lwUIWnd(pBtnParent), iSetTap);

	std::wstring command = _T("/s");
	switch(iSetTap)
	{
	case 1:
		{
			command = _T("/s");
		}break;
	case 2:
		{
			command = _T("/g");
		}break;
	case 3:
		{
			command = _T("/p");
		}break;
	case 4:
		{
			command = _T("/v");
		}break;
	default:
		{
			command = _T("/s");
		}
	}
}

void lwRecvCouponWord(lwWString kStr)
{
	if(kStr.IsNil() || kStr.Length() < MIN_COUPON_KEY_LEN || MAX_COUPON_KEY_LEN < kStr.Length())
	{
		lwAddWarnDataTT(1971);
		return;
	}

	XUI::CXUI_Wnd *pkWnd = XUIMgr.Get(L"SFRM_EVENT_COUPON");
	if( !pkWnd )
	{
		return;
	}

	XUI::CXUI_Edit *pkEdit = dynamic_cast<XUI::CXUI_Edit *>(pkWnd->GetControl(L"EDT_COUPON"));
	if( !pkEdit )
	{
		return;
	}

	XUI::CXUI_Wnd* pAlert = XUIMgr.Call(_T("SFRM_TRY_EVENT_COUPON"));
	if( pAlert )
	{
		pAlert->SetCustomData(pkEdit->EditText().c_str(), sizeof(std::wstring::value_type)*pkEdit->EditText().size());
		pkWnd->Close();
	}
}

void lwSendEventCoupon()
{
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("SFRM_TRY_EVENT_COUPON"));
	if( !pWnd )
	{
		return;
	}

	std::wstring kText;
	size_t const iCustomSize = pWnd->GetCustomDataSize() / sizeof(std::wstring::value_type);
	if( !iCustomSize )
	{
		return;
	}

	kText.resize(iCustomSize);
	if( iCustomSize == kText.size() )
	{
		bool const bSuccess = pWnd->GetCustomData(&kText.at(0), sizeof(std::wstring::value_type) * iCustomSize);
		if( !bSuccess )
		{
			return;
		}
	}

	if(lwSendTakeCoupon(MB(kText)))
	{
		pWnd->Close();
	}
}

void lwSendEventNameCoupon()
{
	BM::Stream kPacket(PT_C_M_TRY_TAKE_COUPON);
	kPacket.Push((std::wstring)(_T("")));

	NETWORK_SEND(kPacket)
}

void CallMegaPhoneMsgUI(SMS_HEADER const& kHeader, XUI::PgExtraDataPackInfo const &rkExtraDataPackInfo)//BYTE const eSMSType, std::wstring const& kName, std::wstring const& kContants)
{
	std::wstring	kFormName;
	
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(kHeader.iUseItemNo);	

	switch( kHeader.eCType )
	{
	case CARD_POLORING:	{ kFormName = _T("");						}break;
	case CARD_LOVE:		{ kFormName = _T("FRM_EVT_SMS_MSG");		}break;
	case CARD_FIRE:		
		{ 			
			kFormName = _T("FRM_EVT_FIRELOVE_MSG");
		}break;
	case MARRY_TALK:
		{
			lua_tinker::call<void, int>("SweetHeartNPC_Talk", kHeader.iUseItemNo);
			return;
		}break;
	default:			{ kFormName = _T("FRM_MEGAPHONE_MSG");		}break;
	}

	XUI::CXUI_Wnd* pkMega = XUIMgr.Get(kFormName);

	bool bIsCall = false;
	if( !pkMega )
	{
		bIsCall = true;
		pkMega = XUIMgr.Call(kFormName);
	}

	if( !pkMega )
	{  
		return;
	}

	XUI::CXUI_Wnd*	pkMegaText = pkMega->GetControl(L"FRM_TEXT");
	XUI::CXUI_Wnd*	pkChMega = pkMega->GetControl(L"FRM_CH_MEGAPHONE");
	XUI::CXUI_Wnd*	pkSvMega = pkMega->GetControl(L"FRM_SV_MEGAPHONE");

	if( CARD_FIRE != kHeader.eCType
		&& !pkMegaText )
	{
		return;
	}
	if( CARD_NONE == kHeader.eCType )
	{
		if( !pkChMega || !pkSvMega )
		{
			return;
		}
	}

	std::wstring	kTotalText = kHeader.kSenderName + L" : " + kHeader.kMsg;
	EChatType	ChatType = CT_NONE;
	bool bIsUpdate = false;

	switch( kHeader.eCType )
	{
	case CARD_POLORING:	{ }break;
	case CARD_LOVE:		
		{
			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(2111).c_str(), kHeader.kSenderName.c_str(), kHeader.kRecverName.c_str());
			pkMega->Text(szTemp);
			pkMegaText->Text(kHeader.kMsg);
			bIsUpdate = true;
			ChatType = CT_LOVE_SMS;
		}break;
	case CARD_FIRE:
		{
			int const iDurationTime = pkItemDef->GetAbil(AT_DURATION_TIME);
			//지속 시간 설정
			g_kFireLove.PushDataAndSetToUI(pkMega, PgFireLove::SFireLoveSMSInfo(kHeader.kMsg, kHeader.kRecverName, iDurationTime));
			return;
		}break;
	default:
		{ 
			switch( kHeader.eType )
			{
			case SMS_IN_CHANNEL:
				{
					if( bIsCall || !pkSvMega->Visible() )
					{
						pkChMega->Visible(true);
						pkSvMega->Visible(false);
						pkMegaText->Text(kTotalText);
						bIsUpdate = true;
					}
					ChatType = CT_MEGAPHONE_CH;
				}break;
			case SMS_IN_REALM:
				{
					pkChMega->Visible(false);
					pkSvMega->Visible(true);
					ChatType = CT_MEGAPHONE_SV;
					pkMegaText->Text(kTotalText);
					bIsUpdate = true;
				}break;
			}
		}break;
	}
	
	SChatLog	ChatLog(ChatType);
	ChatLog.kCharName = kHeader.kSenderName;
	ChatLog.kExtraDataPackInfo = rkExtraDataPackInfo;
	XUI::PgExtraDataPack	kCharNameData;
	kCharNameData.AddExtraData(LINK_EDT_CHAR, ChatLog.kCharGUID.str());
	ChatLog.kExtraDataPackInfo.AddExtraDataPack(kCharNameData);

	g_kChatMgrClient.AddLogMessage(ChatLog, kHeader.kMsg);

	std::string kSceneID;
	bool const bFindScendID = g_kRenderMan.GetFirstTypeID<PgWorld>(kSceneID);
	if( bIsUpdate && bFindScendID )
	{
		BM::vstring vStr(_T("CloseMegaPhoneUI"));
		vStr += (int)kHeader.eCType;
		std::wstring kStr = _T("CloseUI(\"");
		kStr += kFormName;
		kStr += _T("\")");

		STimerEvent kEvent;
		kEvent.Set(6.0f, MB(kStr));
		g_kEventTimer.AddLocal(MB(vStr), kEvent);
	}
}

void lwSetLocalName(char const* szName)
{
	if(!szName)
	{
		return;
	}
	std::wstring strName = UNI(szName);
	UPR(strName);
	XUIMgr.LocalName(strName);
}

lwWString lwGetLocalName()
{
	std::wstring strName = XUIMgr.LocalName();
	return lwWString(strName);
}

namespace SoulCraft
{
	int const iOneScrollNeedSoul = 3;
	int const iSoulUnsealingMakingNo = 9001;
	int const iOneTimeMaxMakingCount = 10; // 한번에 최대 10개씩만 교환 가능
	//int const iSoulItemNo = 79000030;

	void SetSimpleItemIcon(lwUIWnd kWnd, int const iItemNo)
	{
		XUI::CXUI_Wnd *pkImageControl = kWnd();
		if(NULL == pkImageControl)
		{
			NILOG(PGLOG_ERROR, "RewardItemIcon is NULL"); 
			assert(0); 
			return;
		}

		XUI::CXUI_Image *pkImageWnd = dynamic_cast<XUI::CXUI_Image*>(pkImageControl);
		if( !pkImageWnd )
		{
			return;
		}

		if( iItemNo )
		{
			PgUISpriteObject *pkSprite = g_kUIScene.GetIconTexture(iItemNo);
			PgUIUVSpriteObject *pkUVSprite = (PgUIUVSpriteObject*)pkSprite;
			if(!pkUVSprite)
			{
				NILOG(PGLOG_ERROR, "Is Texture is NULL"); 
				assert(0); 
				return;
			}
			
			pkImageWnd->DefaultImgTexture(pkUVSprite);
			const SUVInfo &rkUV = pkUVSprite->GetUVInfo();
			pkImageWnd->UVInfo(rkUV);
			POINT2 kPoint(Quest::iIconWidth*rkUV.U, Quest::iIconHeight*rkUV.V);//이미지 원본 사이즈 복구
			pkImageWnd->ImgSize(kPoint);
		}
		else
		{
			pkImageWnd->DefaultImgName(std::wstring());
		}
	}

	void CallSimpleItemToolTip(lwUIWnd kWnd, int const iItemNo)
	{
		XUI::CXUI_Wnd *pkWnd = kWnd();
		if( !pkWnd )
		{
			NILOG(PGLOG_ERROR, "kControl is NULL"); 
			assert(0);
		}
		CallToolTip_ItemNo(iItemNo, kWnd.GetLocation());
	}

	int GetCanMaxTradeMaking(int const iMakingNo, int const iLimitCount)
	{
		int const iMinimumCount = 1;
		int const iMaximumCount = INT_MAX - 1;
		GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
		SDefItemMakingData kMakingInfo;
		if( !kItemMakingDefMgr.GetDefItemMakingInfo(kMakingInfo, iMakingNo) )
		{
			return iMinimumCount;
		}

		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
		if( !pkPlayer )
		{
			return iMinimumCount;
		}

		PgInventory *pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return iMinimumCount;
		}

		typedef std::map< int, size_t > ContItemCount;
		ContItemCount kHaveItemCount;

		int iCanCount = iMaximumCount;
		for( size_t iCur = 0; MAX_SUCCESS_RATE_ARRAY > iCur; ++iCur )
		{
			int const iNeedItemNo = kMakingInfo.kNeedElements.aElement[iCur];
			int const iNeedItemcount = kMakingInfo.kNeedCount.aCount[iCur];

			if( iNeedItemNo )
			{
				ContItemCount::iterator find_iter = kHaveItemCount.find(iNeedItemNo);
				if( kHaveItemCount.end() == find_iter )
				{
					size_t const iHaveTotalCount = pkInv->GetTotalCount(iNeedItemNo, false);
					std::pair<ContItemCount::iterator, bool> kRet = kHaveItemCount.insert( std::make_pair(iNeedItemNo, iHaveTotalCount) );
					find_iter = kRet.first;
				}

				size_t &iCurHaveCount = (*find_iter).second;
				int const iCanMakeCountMax = static_cast< int >(iCurHaveCount/iNeedItemcount);
				iCanCount = __min(iCanCount, iCanMakeCountMax);
				//중복된 아이템이 다른 갯수로 필요하면? --;
				iCurHaveCount -= iCanMakeCountMax * iNeedItemcount;
			}
		}

		return std::min(iLimitCount, iCanCount);
		//return iCanCount;
	}

	void UpdateTradeUnselingScroll(lwUIWnd kTopWnd, int const iCount)
	{
		CXUI_Wnd* pkTopWnd = kTopWnd();
		if( !pkTopWnd )
		{
			return;
		}

		//if( 0 == GetCanMaxTradeMaking(iSoulUnsealingMakingNo) )
		//{
		//	lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 400450, true);
		//	pkTopWnd->Close();
		//}

		CXUI_Wnd* pkSoulCountWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_COUL_COUNT")) );
		CXUI_Wnd* pkScrollCountWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_SCROLL_COUNT")) );
		CXUI_Wnd* pkIncBtn = pkTopWnd->GetControl( std::wstring(_T("BTN_INC_COUNT")) );
		CXUI_Wnd* pkDecBtn = pkTopWnd->GetControl( std::wstring(_T("BTN_DEC_COUNT")) );
		CXUI_Wnd* pkOnBtn = pkTopWnd->GetControl( std::wstring(_T("BTN_OK")) );

		if( pkSoulCountWnd )	{ pkSoulCountWnd->Text( BM::vstring(iCount * iOneScrollNeedSoul) ); }
		if( pkScrollCountWnd )	{ pkScrollCountWnd->Text( BM::vstring(iCount) ); }
		if( pkIncBtn )			{ pkIncBtn->SetCustomData(&iCount, sizeof(iCount)); }
		if( pkDecBtn )			{ pkDecBtn->SetCustomData(&iCount, sizeof(iCount)); }
		if( pkOnBtn )			{ pkOnBtn->SetCustomData(&iCount, sizeof(iCount)); }
	}

	bool PreCheckItemMaking(int const iMakingNo, int const iTestCount)
	{
		int const iMaxCount = GetCanMaxTradeMaking(iMakingNo);
		if( iMaxCount < iTestCount )
		{
			return false;
		}
		return true;
	}

	bool CreateMakingOrder(SDefItemMakingData const& rkMakingInfo, SReqItemMaking &rkOutOrder, int const iCount, bool const bAllUseItem)
	{
		rkOutOrder.Clear();

		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
		if( !pkPlayer )
		{
			return false;
		}

		PgInventory *pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return false;
		}

		rkOutOrder.iMakingNo = rkMakingInfo.iNo;
		rkOutOrder.iMakingCount = iCount; // 몇개 교환?
	//	rkOutOrder.bAllUseItem = bAllUseItem;

	//	SReqItemMaking kReqItemMaking;
	//	kReqItemMaking.iMakingNo = iSoulUnsealingMakingNo;
	//	kReqItemMaking.iMakingCount = iCount; // 몇개 교환?
	/*
		// 전체 필요 아이템/수량
		for( size_t iCur = 0; MAX_SUCCESS_RATE_ARRAY > iCur; ++iCur )
		{
			int const iNeedItemNo = rkMakingInfo.kNeedElements.aElement[iCur];
			int const iNeedItemcount = rkMakingInfo.kNeedCount.aCount[iCur] * iCount; // 기본 필요수량 * 반복 횟수

			if( iNeedItemNo )
			{
				int const iCurItemTotalCount = (int)pkInv->GetTotalCount(iNeedItemNo, false);
				if( iNeedItemcount > iCurItemTotalCount )
				{
					lwAddWarnDataTT(400450);
					return false;
				}

				int iFindCount = iNeedItemcount;
				SItemPos kFindPos;
				if( S_OK == pkInv->GetFirstItem(iNeedItemNo, kFindPos, false) )
				{
					PgBase_Item kFindItem;
					if( S_OK == pkInv->GetItem(kFindPos, kFindItem) )
					{
						//찾은 아이템
						CONT_ITEM_MAKING_ITEMSLOT::value_type kNewSlotItem;
						kNewSlotItem.iItemCount = kFindItem.Count();//__min(iFindCount, kFindItem.Count());
						kNewSlotItem.kItemPos = kFindPos;
						rkOutOrder.kItemSlotCont.push_back(kNewSlotItem);

						iFindCount -= kFindItem.Count();

						if( 0 == iFindCount )
						{
							continue;
						}

						//모자른 분량 체워 넣기
						while( 0 < iFindCount && S_OK == pkInv->GetNextItem(iNeedItemNo, kFindPos) )
						{
							if( S_OK == pkInv->GetItem(kFindPos, kFindItem) )
							{
								kNewSlotItem.iItemCount = kFindItem.Count();//__min(iFindCount, kFindItem.Count());
								kNewSlotItem.kItemPos = kFindPos;
								rkOutOrder.kItemSlotCont.push_back(kNewSlotItem);

								iFindCount -= kFindItem.Count();

								if( 0 == iFindCount )
								{
									break; // break while()
								}
							}
						}
					}
				}

				if( 0 < iFindCount )
				{
					lwAddWarnDataTT(400450);
					return false;
				}
			}
		}
	*/
		return true;
	}

	void SendTradeUnsealingScroll(int const iCount)
	{
		if( 0 >= iCount )
		{
			return;
		}

		GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
		SDefItemMakingData kMakingInfo;
		if( !kItemMakingDefMgr.GetDefItemMakingInfo(kMakingInfo, iSoulUnsealingMakingNo) )
		{
			return;
		}

		SReqItemMaking kReqItemMaking;
		if( !CreateMakingOrder(kMakingInfo, kReqItemMaking, iCount) )
		{
			return;
		}

		BM::Stream kPacket(PT_C_M_REQ_ITEM_MAKING);
		kPacket.Push(1);	// 
		kReqItemMaking.WriteToPacket(kPacket);
		kPacket.Push(SItemPos());
		kPacket.Push(SItemPos());

		NETWORK_SEND(kPacket)
	}
}

namespace SoulStoneTrade
{
	typedef struct tagTrade
	{
		tagTrade()
			:gidSerial(BM::GUID::NullData()), iItemNo(0), iMaterialNo(0), iMaterialNeedsCount(0)
		{
		}

		tagTrade(BM::GUID Serial, int const ItemNo, int const Material, int const MaterialNeedsCount)
			:gidSerial(Serial), iItemNo(ItemNo), iMaterialNo(Material), iMaterialNeedsCount(MaterialNeedsCount)
		{
		}

		tagTrade(tagTrade const& rhs)
			:gidSerial(rhs.gidSerial), iItemNo(rhs.iItemNo), iMaterialNo(rhs.iMaterialNo), iMaterialNeedsCount(rhs.iMaterialNeedsCount)
		{
		}

		tagTrade operator = (tagTrade const& rhs)
		{
			gidSerial = rhs.gidSerial;
			iItemNo = rhs.iItemNo;
			iMaterialNo = rhs.iMaterialNo;
			iMaterialNeedsCount = rhs.iMaterialNeedsCount;
			return *this;
		}

		bool operator == (tagTrade const& rhs)const
		{
			return (gidSerial == rhs.gidSerial) && (iItemNo == rhs.iItemNo) && (iMaterialNo == rhs.iMaterialNo) && (iMaterialNeedsCount == rhs.iMaterialNeedsCount);
		}

		bool Empty()const
		{
			return (gidSerial.IsNull()) || (0 == iItemNo) || (0 == iMaterialNo) || (0 == iMaterialNeedsCount);
		}

		BM::GUID gidSerial;
		int iItemNo;
		int iMaterialNo;
		int iMaterialNeedsCount;
	} STrade;

	typedef std::vector< STrade > ContTrade;
	typedef std::map< int, ContTrade > ContTradeTable;

	std::wstring const kSoulStoneTradeWndName(L"FRM_TRADE_SOULSTONE");
	int const iSoulStoneItemNo = 60000960;
	int const iDefaulTadetLocationNo = 1;
	ContTradeTable kTradeTable;	

	//
	void AddSoulStoneTradeItem(char* pszSerial, int const iLocation, int iItemNo, int iMaterial, int iMaterialNeedsCount)
	{
		ContTradeTable::iterator find_iter = kTradeTable.find(iLocation);
		if( kTradeTable.end() == find_iter )
		{
			std::pair<ContTradeTable::iterator, bool> kRet = kTradeTable.insert( std::make_pair(iLocation, ContTradeTable::mapped_type()) );
			if( kRet.second )
			{
				find_iter = kRet.first;
			}
			else
			{
				return;
			}
		}

		STrade kNewTrade(BM::GUID(pszSerial), iItemNo, iMaterial, iMaterialNeedsCount);
		ContTradeTable::mapped_type &rkElement = (*find_iter).second;
		if( rkElement.end() == std::find(rkElement.begin(), rkElement.end(), kNewTrade)
		&&	!kNewTrade.Empty() )
		{
			std::back_inserter(rkElement) = kNewTrade;
		}
	}


	void InitTradeTable()
	{
		lua_tinker::call<void>("InitSoulStoneTradeTable");
	}


	void SetTradeTopWndCustomData(CXUI_Wnd* pkWnd, int const iCurTradeLocation, bool const bReset)
	{
		if( !pkWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		kCustomData.Push( iCurTradeLocation );
		kCustomData.Push( bReset );
		pkWnd->SetCustomData( kCustomData.Data() );
	}
	void GetTradeTopWndCustomData(CXUI_Wnd* pkWnd, int& iCurTradeLocation, bool& bReset)
	{
		if( !pkWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		if( !pkWnd->GetCustomData(kCustomData.Data()) )
		{
			return;
		}
		kCustomData.PosAdjust();
		kCustomData.Pop( iCurTradeLocation );
		kCustomData.Pop( bReset );
	}

	void SetSoulStoneTradeLocation(int const iTradeLocation)
	{
		CXUI_Wnd* pkTopWnd = XUIMgr.Get(kSoulStoneTradeWndName);
		if( !pkTopWnd )
		{
			return;
		}

		if( kTradeTable.end() == kTradeTable.find(iTradeLocation) )
		{
			return;
		}

		int iOldTradeLocation = 0;
		bool bReset = false;
		GetTradeTopWndCustomData(pkTopWnd, iOldTradeLocation, bReset);
		SetTradeTopWndCustomData(pkTopWnd, iTradeLocation, iOldTradeLocation != iTradeLocation);
	}

	void SetTradeItemCustomData(CXUI_Wnd* pkItemWnd, STrade const& rkTradeInfo, int const iCount, bool const bReset)
	{
		if( !pkItemWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		kCustomData.Push( rkTradeInfo );
		kCustomData.Push( iCount );
		kCustomData.Push( bReset );
		pkItemWnd->SetCustomData( kCustomData.Data() );
	}
	void GetTradeItemCustomData(CXUI_Wnd* pkItemWnd, STrade& rkTradeInfo, int& iCount, bool& bReset)
	{
		if( !pkItemWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		if( !pkItemWnd->GetCustomData(kCustomData.Data()) )
		{
			return;
		}
		kCustomData.PosAdjust();
		kCustomData.Pop( rkTradeInfo );
		kCustomData.Pop( iCount );
		kCustomData.Pop( bReset );
	}
	int GetCanMaxTradeMaking(STrade const & rkTradeInfo, int const iLimitCount)
	{
		int const iMinimumCount = 1;
		int const iMaximumCount = INT_MAX - 1;
		
		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
		if( !pkPlayer )
		{
			return iMinimumCount;
		}

		PgInventory *pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return iMinimumCount;
		}

		typedef std::map< int, size_t > ContItemCount;
		ContItemCount kHaveItemCount;

		int iCanCount = iMaximumCount;

		int const iNeedItemNo = rkTradeInfo.iMaterialNo;
		int const iNeedItemcount = rkTradeInfo.iMaterialNeedsCount;

		if( iNeedItemNo )
		{
			size_t const iHaveTotalCount = pkInv->GetTotalCount(iNeedItemNo, false);

			int const iCanMakeCountMax = static_cast< int >(iHaveTotalCount/iNeedItemcount);
			iCanCount = __min(iCanCount, iCanMakeCountMax);
		}

		return std::min(iLimitCount, iCanCount);
		//return iCanCount;
	}
	int UpdateTradeItemWnd(CXUI_Wnd* pkItemWnd, STrade const& rkTradeInfo, bool const bReset)
	{
		if( !pkItemWnd )
		{
			return 0;
		}

		int const iNeedItemNo = rkTradeInfo.iMaterialNo;
		int const iNeedItemCount = rkTradeInfo.iMaterialNeedsCount;

		TCHAR const* szNeedItemName = NULL;
		if( !GetItemName(iNeedItemNo, szNeedItemName) ) // 재료 아이템 이름
		{
			return 0;
		}

		std::wstring kNeedItemText;
		if( !FormatTTW(kNeedItemText, 400459, szNeedItemName, iNeedItemCount) )
		{
			return 0;
		}

		int const iMaxCount = GetCanMaxTradeMaking(rkTradeInfo);

		int iCount = 0;
		STrade kCurTradeInfo;
		bool bCurReset = false;
		GetTradeItemCustomData(pkItemWnd, kCurTradeInfo, iCount, bCurReset);

		iCount = (bReset || bCurReset)? 0: std::min(iCount, iMaxCount); // 제작가능한 최대 갯수로 제한

		pkItemWnd->Text( kNeedItemText ); // 영혼석 몇개 필요?
		CXUI_Wnd* pkIconWnd = pkItemWnd->GetControl( std::wstring(_T("ICON_Item")) );
		if( pkIconWnd ) // 아이템 이미지 설정
		{
			SoulCraft::SetSimpleItemIcon(pkIconWnd, rkTradeInfo.iItemNo);
		}
		CXUI_Wnd* pkResultItemWnd = pkItemWnd->GetControl( std::wstring(_T("SFRM_RESULT_ITEM")) );
		if( pkResultItemWnd ) // 결과 아이템이 뭐?
		{
			TCHAR const* szResultItemName = NULL;
			if( GetItemName(rkTradeInfo.iItemNo, szResultItemName) )
			{
				Quest::SetCutedTextLimitLength(pkResultItemWnd, std::wstring(szResultItemName), std::wstring(_T("...")));
			}
		}
		CXUI_Wnd* pkMakeCountWnd = pkItemWnd->GetControl( std::wstring(_T("SFRM_COUNT")) );
		if( pkMakeCountWnd ) // 몇개!
		{
			BM::vstring const kCountStr(iCount);
			pkMakeCountWnd->Text( kCountStr );
		}

		SetTradeItemCustomData(pkItemWnd, rkTradeInfo, iCount, false);
		return iCount * iNeedItemCount;
	}
	void IncSoulStoneTradeItemWnd(lwUIWnd kItemWnd, int const iIncVal)
	{
		CXUI_Wnd* pkTopWnd = kItemWnd();
		if( !pkTopWnd )
		{
			return;
		}

		STrade kTradeInfo;
		int iCount = 0;
		bool bReset = false;
		GetTradeItemCustomData(pkTopWnd, kTradeInfo, iCount, bReset);
		int const iMaxCount = GetCanMaxTradeMaking(kTradeInfo);
		if( 0 < iIncVal )
		{
			if( iMaxCount < (iCount+iIncVal) )
			{
				iCount = 0;
			}
			else
			{
				iCount = std::min(iCount+iIncVal, iMaxCount);
			}
		}
		else
		{
			if( 0 > (iCount+iIncVal) )
			{
				iCount = iMaxCount;
			}
			else
			{
				iCount = std::max(iCount+iIncVal, 0);
			}
		}
		SetTradeItemCustomData(pkTopWnd, kTradeInfo, iCount, false);
	}
	bool CreateMakingOrder(STrade const & rkInfo, SReqSoulStoneTrade &rkOutOrder, int const iCount, bool const bAllUseItem)
	{
		rkOutOrder.Clear();

		PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(g_kPilotMan.GetPlayerUnit());
		if( !pkPlayer )
		{
			return false;
		}

		PgInventory *pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return false;
		}

		rkOutOrder.gidSerial = rkInfo.gidSerial;
		rkOutOrder.iItemNo = rkInfo.iItemNo;
		rkOutOrder.iMaterialNo = rkInfo.iMaterialNo;
		rkOutOrder.iMaterialNeedsCount = rkInfo.iMaterialNeedsCount;
		rkOutOrder.iTradeCount = iCount;

		return true;
	}
	void SendSoulStoneTrade(lwUIWnd kItemWnd)
	{
		CXUI_Wnd* pkTopWnd = kItemWnd();
		if( !pkTopWnd )
		{
			return;
		}

		STrade kTradeInfo;
		int iCount = 0;
		bool bReset = false;
		GetTradeItemCustomData(kItemWnd(), kTradeInfo, iCount, bReset);
		if( 0 == iCount
		||	kTradeInfo.Empty() )
		{
			return;
		}
		SetTradeItemCustomData(kItemWnd(), kTradeInfo, iCount, true); // 다음번 업데이트시 0으로

		SReqSoulStoneTrade kReqSoulStoneTrade;
		if( !CreateMakingOrder(kTradeInfo, kReqSoulStoneTrade, iCount) )
		{
			return;
		}

		BM::Stream kPacket(PT_C_M_REQ_SOULSTONE_TRADE);
		kPacket.Push(1);
		kReqSoulStoneTrade.WriteToPacket(kPacket);
		kPacket.Push(SItemPos());
		kPacket.Push(SItemPos());

		NETWORK_SEND(kPacket)
	}
	void CallSoulStoneTradeToolTip(lwUIWnd kIconWnd)
	{
		CXUI_Wnd* pkIconWnd = kIconWnd();
		if( pkIconWnd )
		{
			STrade kTradeInfo;
			int iCount = 0;
			bool bReset = false;
			GetTradeItemCustomData(pkIconWnd->Parent(), kTradeInfo, iCount, bReset);
			if( !kTradeInfo.Empty() )
			{
				CallToolTip_ItemNo(kTradeInfo.iItemNo, kIconWnd.GetLocation());
			}
		}
	}

	void UpdateSoulStoneTrade()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgInventory const* pkInventory = pkPlayer->GetInven();
		if( !pkInventory )
		{
			return;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pkSoulStoneItemDef = kItemDefMgr.GetDef(iSoulStoneItemNo);
		if( !pkSoulStoneItemDef )
		{
			return;
		}

		CXUI_Wnd* pkTopWnd = XUIMgr.Get(kSoulStoneTradeWndName);
		if( !pkTopWnd )
		{
			pkTopWnd = XUIMgr.Call(kSoulStoneTradeWndName, true);
			if( !pkTopWnd )
			{
				return;
			}
		}

		InitTradeTable();

		int iTradeLocation = 0;
		bool bReset = false;
		GetTradeTopWndCustomData(pkTopWnd, iTradeLocation, bReset);
		if( 0 == iTradeLocation )
		{
			iTradeLocation = iDefaulTadetLocationNo;
			bReset = true;
		}

		//
		int iReservePayCount = 0;
		ContTradeTable::const_iterator trade_iter = kTradeTable.find( iTradeLocation );
		CXUI_List* pkList = dynamic_cast<CXUI_List*>(pkTopWnd->GetControl( std::wstring(_T("LIST_TRADE_LIST")) ));
		if( pkList
		&&	kTradeTable.end() != trade_iter )
		{
			ContTradeTable::mapped_type const& rkItemList = (*trade_iter).second;
			ContTrade::const_iterator item_iter = rkItemList.begin();

			// 있을 때
			// 없을 때
			SListItem* pkListItem = pkList->FirstItem();
			while( rkItemList.end() != item_iter )
			{
				SListItem* pkCurListItem = (pkListItem)? pkListItem: pkList->AddItem(std::wstring());
				if( pkCurListItem
				&&	pkCurListItem->m_pWnd )
				{
					int const iNeedCount = UpdateTradeItemWnd(pkCurListItem->m_pWnd, (*item_iter), bReset);
					iReservePayCount += iNeedCount;
				}

				pkListItem = (pkListItem)? pkList->NextItem(pkListItem): NULL;
				++item_iter;
			}

			// 남았을 때
			while( pkListItem )
			{
				pkListItem = pkList->DeleteItem(pkListItem);
			}
		}

		//
		CXUI_Wnd* pkLocalNameWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_LOCATION")) );
		if( pkLocalNameWnd )
		{
			int const iTextID = lua_tinker::call<int>("GetSoulTradeLocationText", iTradeLocation);
			if( iTextID )
			{
				pkLocalNameWnd->Text( TTW(iTextID) );
			}
		}

		CXUI_Wnd* pkSoulStoneNameWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_RESULT_ITEM")) );
		if( pkSoulStoneNameWnd )
		{
			std::wstring const *pkItemName = NULL;
			if( GetDefString(pkSoulStoneItemDef->NameNo(), pkItemName) )
			{
				pkSoulStoneNameWnd->Text( *pkItemName );
			}
		}

		size_t const iHaveNeedItemCount = pkInventory->GetTotalCount(iSoulStoneItemNo);
		CXUI_Wnd* pkSoulStonePayWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_COUNT_PAY")) );
		if( pkSoulStonePayWnd )
		{
			DWORD const dwOverCountColor = 0xFFFF3413;
			DWORD const dwCanCountColor = 0xFF4D3413;
			BM::vstring const kPayCount( -iReservePayCount );
			pkSoulStonePayWnd->Text( kPayCount );
			pkSoulStonePayWnd->FontColor( (iReservePayCount > iHaveNeedItemCount)? dwOverCountColor: dwCanCountColor);
		}

		CXUI_Wnd* pkSoulStoneCountWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_COUNT")) );
		if( pkSoulStoneCountWnd )
		{
			BM::vstring const kSoulStoneCount( iHaveNeedItemCount );
			pkSoulStoneCountWnd->Text( kSoulStoneCount );
		}
		SetTradeTopWndCustomData(pkTopWnd, iTradeLocation, false);
	}
}

bool IsShareRentalInv(int const iInvType)
{
	switch( iInvType )
	{
	case IT_SHARE_RENTAL_SAFE1:
	case IT_SHARE_RENTAL_SAFE2:
	case IT_SHARE_RENTAL_SAFE3:
	case IT_SHARE_RENTAL_SAFE4:
		{
			return true;
		}break;
	}
	return false;
}

void lwSetGuildInvViewSize(const BYTE byInvType)
{
	const int iExtendLineItemNameNo = 49000560;
	const int iExtendTabItemNameNo = 98009670;

	XUI::CXUI_Wnd* pkParent = XUIMgr.Get(L"SFRM_GUILD_INVENTORY");
	if( !pkParent )
	{
		return ;
	}

	XUI::CXUI_Wnd* pkLineExtendGuide = pkParent->GetControl(L"SFRM_LINE_EXTEND_GUIDE");
	if( !pkLineExtendGuide )
	{
		return ;
	}

	XUI::CXUI_Wnd* pkTabExtendGuide = pkParent->GetControl(L"SFRM_TAB_EXTEND_GUIDE");
	if( !pkTabExtendGuide )
	{
		return ;
	}

	PgInventory* pkInv = g_kGuildMgr.GetInven();
	if( !pkInv )
	{
		return ;
	}

	const BYTE byTotalInvSize = pkInv->GetMaxIDX(static_cast<EInvType>(byInvType));
	const std::wstring *pkItemName = NULL;

	if( 0 == byTotalInvSize )
	{// 탭 확장 안한것
		if(GetDefString(iExtendTabItemNameNo, pkItemName))
		{
			BM::vstring vStr(TTW(401094));
			vStr.Replace(L"#ITEM#", *pkItemName);
			pkTabExtendGuide->Text(vStr);
		}

		pkTabExtendGuide->Visible(true);
		pkLineExtendGuide->Visible(false);
	}
	else
	{
		if( byTotalInvSize >= MAX_GUILD_IDX_EXTEND )
		{// 라인 최대 확장
			pkTabExtendGuide->Visible(false);
			pkLineExtendGuide->Visible(false);
		}
		else
		{// 라인확장을 덜 했다.
			XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkParent->GetControl(L"BLD_INV_ICON"));
			if( pkBuilder )
			{				
				const int iRemainLine = ( MAX_GUILD_IDX_EXTEND - byTotalInvSize ) / pkBuilder->CountX(); // 더 확장 할 수 있는 라인의 수
				const int iExtendLine = ( MAX_GUILD_IDX_EXTEND - BASE_GUILD_IDX ) / pkBuilder->CountX() - iRemainLine; // 확장한 라인 수

				const int iIconHeight = 40;
				const int iIconGab = pkBuilder->GabY() - iIconHeight; // 갭

				const int iGuideHeight = ( iIconHeight * iRemainLine ) + ( iRemainLine - 1 ) * iIconGab;
				pkLineExtendGuide->Size( pkLineExtendGuide->Size().x, iGuideHeight );

				POINT2 kLocation = pkLineExtendGuide->Location();
				kLocation.y = 174 + ( iExtendLine * 40 ) + ( iIconGab * iExtendLine );
				pkLineExtendGuide->Location( kLocation );

				POINT2 kTextPos = pkLineExtendGuide->TextPos();
				kTextPos.y = ( iGuideHeight / 2 ) - 16;
				pkLineExtendGuide->TextPos( kTextPos );

				if(GetDefString(iExtendLineItemNameNo, pkItemName))
				{
					BM::vstring vStr(TTW(401094));
					vStr.Replace(L"#ITEM#", *pkItemName);
					pkLineExtendGuide->Text(vStr);
				}

				pkLineExtendGuide->Visible(true);
				pkTabExtendGuide->Visible(false);
			}
		}
	}
	
	lwChangeGuildInvViewGroup(byInvType);
}

void lwChangeGuildInvViewGroup(const BYTE byInvType)
{
	XUI::CXUI_Wnd* pkParent = XUIMgr.Get(L"SFRM_GUILD_INVENTORY");
	if( !pkParent )
	{
		return ;
	}

	const BYTE byBuildIndex = (byInvType - 1);

	for(int i = 0; i < 4; ++i)
	{
		BM::vstring	vStr(_T("CBTN_BAG"));
		vStr += i;

		XUI::CXUI_CheckButton*	pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkParent->GetControl(vStr));
		if( !pCheckBtn )
		{
			return;
		}
		
		if( i == byBuildIndex )
		{
			pCheckBtn->Check(true);
			pCheckBtn->ClickLock(true);
		}
		else
		{
			pCheckBtn->VLoseFocus();
			pCheckBtn->ClickLock(false);
			pCheckBtn->Check(false);
		}
	}

	EKindUIIconGroup eIconGrp = KUIG_NONE;	
	lwUIWnd::ConvertGuildInvTypeToIconGroup(static_cast<EInvType>(byInvType), eIconGrp);
	g_kGuildMgr.SetGuildInvView(static_cast<int>(eIconGrp));
}

void lwChangeCashBoxViewGroup(lwUIWnd UISelf, int const iInvType)
{
	XUI::CXUI_Wnd* pParent = UISelf.GetSelf();
	XUI::CXUI_CheckButton* pViewCheckBtn = NULL;

	for(int i = 0; i < 4; ++i)
	{
		BM::vstring	vStr(_T("CBTN_BAG"));
		vStr += i;

		XUI::CXUI_CheckButton*	pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vStr));
		if( !pCheckBtn )
		{
			return;
		}
		
		if( i == iInvType || i == (iInvType - 4))
		{
			pViewCheckBtn = pCheckBtn;
		}
		
		pCheckBtn->VLoseFocus();	// 마우스 우클릭으로 인해 탭이 전환 된 경우 필요.
		pCheckBtn->ClickLock(false);
		pCheckBtn->Check(false);
	}

	pViewCheckBtn->Check(true);
	pViewCheckBtn->ClickLock(true);

	int iRealInvType = 0;
	switch(iInvType)
	{
	case 0:
		{
			iRealInvType = IT_SAFE;
		}break;
	case 1:
	case 2:
	case 3:
		{
			iRealInvType = IT_FIT_CASH + iInvType;
		}break;
	case 4:
	case 5:
	case 6:
	case 7:
		{
			iRealInvType = IT_SHARE_RENTAL_SAFE1 + (iInvType - 4);
		}break;
	}

	InitCashBoxViewUI(pParent, iRealInvType);
	lwSetInvSafeViewSize(pParent, iRealInvType);
}

void InitCashBoxViewUI(XUI::CXUI_Wnd* pParent, int const iInvType)
{
	if( !pParent )
	{
		return;
	}

	pParent->SetCustomData(&iInvType, sizeof(iInvType));

	PgPlayer*	pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv )
	{
		return;
	}

	XUI::CXUI_Wnd* pTimeText = pParent->GetControl(_T("FRM_TIME"));
	XUI::CXUI_Wnd* pIntercepter = pParent->GetControl(_T("SFRM_INTERCEPTER"));
	XUI::CXUI_Wnd* pTimeOverText = pParent->GetControl(_T("FRM_INTERCEPT_TEXT"));
	XUI::CXUI_Button* pExpandBtn = dynamic_cast<XUI::CXUI_Button*>(pParent->GetControl(_T("BTN_EXPAND")));
	if( !pIntercepter || !pTimeText || !pTimeOverText || !pExpandBtn)
	{
		return;
	}

	pExpandBtn->SetCustomData(&iInvType, sizeof(iInvType));

	pTimeText->Visible(true);
	pExpandBtn->Disable(true);
	pIntercepter->Visible(false);
	pTimeOverText->Visible(false);
	pTimeOverText->Text( TTW( IsShareRentalInv(iInvType) ? 2925 : 2902 ));
	lwDrawInvRemainTime(lwUIWnd(pTimeText), false);

	bool const bShareRentalPremium = IsShareRentalPremium(iInvType, pPlayer);
	if(false == bShareRentalPremium)
	{
		if( IT_SAFE != iInvType && S_OK != pInv->CheckEnableUseRentalSafe((EInvType)iInvType) )
		{
			pTimeText->Visible(false);
			pExpandBtn->Disable(false);
			pIntercepter->Visible(true);
			pTimeOverText->Visible(true);
		}
	}
	else
	{
		pTimeText->Visible(false);
		pTimeOverText->Visible(true);
		pTimeOverText->Text( TTW(64004) );
	}

	if( lwUseBigSizeInv( static_cast<EInvType>(iInvType) ) )
	{
		pParent = pParent->GetControl(L"SFRM_ITEM_BIG");
	}
	else
	{
		pParent = pParent->GetControl(L"SFRM_ITEM_BG");
	}

	if( !pParent )
	{	
		return;
	}

	XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_INV_ICON"));
	if( pkBuilder )
	{
		int const MAX_SLOT_COUNT = pkBuilder->CountX() * pkBuilder->CountY();
		for(int i = 0; i < MAX_SLOT_COUNT; ++i)
		{
			BM::vstring	vStr(_T("ICON_ITEM"));
			vStr += i;
			
			XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pParent->GetControl(vStr));
			if( !pIcon )
			{
				return;
			}

			SIconInfo Info = pIcon->IconInfo();
			Info.iIconGroup = iInvType;
			pIcon->SetIconInfo(Info);
		}
	}
	pParent->Parent()->Invalidate();
}

void UpdateCashBoxViewGroup()
{
	lwUIWnd UIParent(XUIMgr.Get(_T("SFRM_INV_SAFE")));
	if( UIParent.IsNil() )
	{
		return;
	}

	InitCashBoxViewUI(UIParent.GetSelf(), UIParent.GetCustomData<int>());
}

void lwDrawInvRemainTime(lwUIWnd UISelf, bool const bIsCheckGoldBox)
{
	PgPlayer*	pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv )
	{
		return;
	}

	int iInvType = UISelf.GetParent().GetCustomData<int>();

	if( iInvType != IT_SAFE )
	{
		__int64 iRentalTime = pInv->GetRentalSafeUseTime((EInvType)iInvType);
		if( bIsCheckGoldBox && iRentalTime <= 0 )
		{
			InitCashBoxViewUI(UISelf.GetParent().GetSelf(), UISelf.GetParent().GetCustomData<int>());
			return;
		}

		SYSTEMTIME	Time;
		iRentalTime = __max(0, iRentalTime);

		Time.wDay = ((iRentalTime / DEF_SEC_TIME) / DEF_MIN_TIME) / DEF_HOUR_TIME;
		Time.wHour = ((iRentalTime / DEF_SEC_TIME) / DEF_MIN_TIME) % DEF_HOUR_TIME;
		Time.wMinute = (iRentalTime / DEF_SEC_TIME) % DEF_MIN_TIME;

		wchar_t	szTemp[MAX_PATH] = {0,};
		swprintf_s(szTemp, MAX_PATH, TTW(2904).c_str(), Time.wDay, Time.wHour, Time.wMinute);
		UISelf.GetSelf()->Text(szTemp);
	}
	else
	{
		UISelf.GetSelf()->Text(TTW(2908));
	}
}

void lwExpandCashBox(lwUIWnd UISelf)
{
	if( UISelf.IsNil() )
	{
		return;
	}

	int const iInvType = UISelf.GetCustomData<int>();

	PgPlayer*	pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv )
	{
		return;
	}

	int UictType = UICT_NONE;
	int NotExistTTNo = 0;
	int UseItemTTNo = 0;
	switch( iInvType )
	{
	case KUIG_RENTAL_SAFE1:
	case KUIG_RENTAL_SAFE2:
	case KUIG_RENTAL_SAFE3:
		{
			UictType = UICT_RENTALSAFE;
			NotExistTTNo = 2906;
			UseItemTTNo = 2907;
		}break;
	case KUIG_SHARE_RENTAL_SAFE1:
		{
			UictType = UICT_SHARE_RENTALSAFE_GOLD;
			NotExistTTNo = 2921;
			UseItemTTNo = 2922;
			
			ContHaveItemNoCount kItemCont;
			if( false == UIItemUtil::SearchEqualTypeItemList( UictType, kItemCont ) ) // 골드로 구매한 계정공유금고1 확장 아이템이 있는지 선 체크
			{// 없다면 캐시아이템으로 확장
				UictType = UICT_SHARE_RENTALSAFE_CASH;
			}
		}break;
	case KUIG_SHARE_RENTAL_SAFE2:
	case KUIG_SHARE_RENTAL_SAFE3:
	case KUIG_SHARE_RENTAL_SAFE4:
		{
			UictType = UICT_SHARE_RENTALSAFE_CASH;
			NotExistTTNo = 2921;
			UseItemTTNo = 2922;
		}break;
	default:
		return;
	}

	ContHaveItemNoCount	kItemCont;
	if( UIItemUtil::SearchEqualTypeItemList(UictType, kItemCont) )
	{
		if( 1< kItemCont.size() )
		{
			UIItemUtil::CONT_CUSTOM_PARAM	kParam;
			UIItemUtil::CONT_CUSTOM_PARAM_STR	kParamStr;
			std::pair<UIItemUtil::CONT_CUSTOM_PARAM::iterator, bool> Result = kParam.insert(std::make_pair(std::wstring(L"INV_TYPE"), UISelf.GetParent().GetCustomData<int>()));
			if( Result.second )
			{
				UIItemUtil::CallCommonUseCustomTypeItems(kItemCont, UIItemUtil::ECIUT_CUSTOM_DEFINED, kParam, kParamStr);
			}
		}
		else
		{
			ContHaveItemNoCount::iterator item_iter = kItemCont.begin();
			if( item_iter != kItemCont.end() )
			{
				ContHaveItemNoCount::key_type const& kKey = item_iter->first;

				SItemPos	kRentalPos;
				if( S_OK == pInv->GetFirstItem(kKey, kRentalPos) )
				{
					SPT_C_M_REQ_RENTALSAFE_EXTEND	Data;
					Data.kInvType = UISelf.GetParent().GetCustomData<int>();
					Data.kItemPos = kRentalPos;

					BM::Stream	kPacket;
					Data.WriteToPacket(kPacket);
					lwCallCommonMsgYesNoBox(MB(TTW(UseItemTTNo)), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
				}				
			}
		}
	}
	else
	{
		lua_tinker::call<void, int, int>("OnCallStaticCashItemBuy", 14, iInvType);
		//lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", NotExistTTNo, true);
	}		
}

void lwCallCommonMsgYesNoBox(char const* szText, lwPacket kPacket, bool bIsModal, int const iBoxType, char const* pCancelMsg, bool bEscIsOncancel)
{
	CallCommonMsgYesNoBox(UNI(szText), 96, 97, kPacket, bIsModal, iBoxType, pCancelMsg, bEscIsOncancel);
}

void CallCommonMsgYesNoBox(std::wstring kText, int const TTID_OK, int const TTID_CANCEL, lwPacket kPacket, bool bIsModal, int const iBoxType, char const* pCancelMsg, bool bEscIsOncancel)
{
	std::wstring wstrFormName;
	if( bEscIsOncancel )
	{// 디폴트: ESC 눌렀을때 Oncancel() 호출
		wstrFormName = _T("OK_CANCEL_BOX");
	}
	else
	{// ESC 눌렀을 때 창만 닫는다.
		wstrFormName = _T("OK_CANCEL_BOX_CLOSE");
	}

	BM::vstring vID(wstrFormName);
	vID += L"_";
	vID += BM::vstring(iBoxType);
	XUI::CXUI_MsgBox* pMsgBox = dynamic_cast<XUI::CXUI_MsgBox*>(XUIMgr.Call(wstrFormName, bIsModal, vID));
	if( pMsgBox )
	{
		XUI::CXUI_Wnd *pkFrm = pMsgBox->GetControl(_T("FRM_MESSAGE"));
		if( pkFrm )
		{
			pkFrm->Text(kText);

			XUI::CXUI_Style_String kStyleString = pkFrm->StyleText();
			POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
			int const iFormSizeY = 108;
			pkFrm->TextPos( POINT2(pkFrm->TextPos().x, (iFormSizeY - kTextSize.y) / 2) );
		}

		XUI::PgMessageBoxInfo Info = pMsgBox->BoxInfo();
		Info.iBoxType = iBoxType;
		Info.kOrderGuid = BM::GUID().NullData();
		
		int iExternInt = 0;
		pMsgBox->BoxInfo(Info);
		lwUIWnd	UIMsgBox(pMsgBox);
		UIMsgBox.SetCustomDataAsPacket(kPacket);

		XUI::CXUI_Wnd *pkOK = pMsgBox->GetControl(_T("BTN_OK"));
		XUI::CXUI_Wnd *pkCancel = pMsgBox->GetControl(_T("BTN_CANCEL"));
		if( pkOK )		{ pkOK->Text(TTW(TTID_OK)); }
		if( pkCancel )	{ pkCancel->Text(TTW(TTID_CANCEL)); }

		if( pCancelMsg )
		{
			lwUIWnd(pkCancel).SetCustomDataAsStr(pCancelMsg);
		}
	}
}

std::wstring const GetMapName(int const iMapNo)
{
	const wchar_t *pText = NULL;
	// 예외 그라운드 이름 얻기
	switch (iMapNo)
	{
	case PvP_Lobby_GroundNo_AnterRoom:
		{
			return TTW(113);
		}break;
	case PvP_Lobby_GroundNo_Exercise:
		{
			return TTW(114);
		}break;
	case PvP_Lobby_GroundNo_Ranking:
		{
			return TTW(115);
		}break;
	default:
		{
			CONT_DEFMAP const * pkContDefMap = NULL;
			g_kTblDataMgr.GetContDef(pkContDefMap);

			CONT_DEFMAP::const_iterator iter = pkContDefMap->find(iMapNo);
			if( pkContDefMap->end() == iter )
			{
				GetDefString(iMapNo, pText);
			}
			else
			{
				TBL_DEF_MAP const& rkDefMap = (*iter).second;
				GetDefString(rkDefMap.NameNo, pText);
			}
		}break;
	}

	return (pText == NULL) ? std::wstring() : std::wstring(pText);
}


lwPoint2 const lwGetImgSizeFromPath(char const* szPath)
{
	lwPoint2 kPt(0,0);
	if(szPath)
	{
		NiSourceTexture const* pkTex = g_kNifMan.GetTexture(szPath);
		if(pkTex)
		{
			kPt.SetX(pkTex->GetWidth());
			kPt.SetY(pkTex->GetHeight());
		}
	}
	return kPt;
}

lwWString lwGetMiniMapPath(int const iMapNo)
{
	EGndAttr const eAttr = static_cast<EGndAttr>(lwGetMapAttrByNo(iMapNo));
	if(!(eAttr & GATTR_INSTANCE) && !(eAttr & GATTR_STATIC_DUNGEON))	//인던이　아니면
	{
		std::wstring kMapPath;
		if(g_kWorldMapUI.GetMiniMapPath(iMapNo, kMapPath))
		{
			return lwWString(kMapPath);
		}
	}
	return lwWString("");
}

int lwGetMapNoFromNpcName(lwWString rkString)
{
	return g_kWorldMapUI.GetNpcMapNo(rkString.GetWString());
}

int lwPlayMovie(char const *pszText)
{
	if(false == g_kMovieMgr.IsPlay())
	{
		return g_kMovieMgr.Play(pszText);
	}
	return false;
}

bool lwIsPlayMovie()
{
	return g_kMovieMgr.IsPlay();
}

bool lwGetExtendSlotView()
{
	return (0 == g_kGlobalOption.GetValue(STR_SAVE_INFO, STR_SHOW_EXTEND_QUICK_SLOT))?(false):(true);
}

void lwSetExtendSlotView(bool const bShow)
{
	if(!g_kGlobalOption.SetConfig(STR_SAVE_INFO, STR_SHOW_EXTEND_QUICK_SLOT, (true == bShow)?(1):(0), NULL))
	{
		assert(0);
	}
	g_kGlobalOption.ApplyConfig();
	g_kGlobalOption.Save();
}

inline bool GetFindPlayerName(BM::GUID const& kPlayerGuid, std::wstring & kPlayerName)
{
	PgPilot const* pkPlayerPilot = g_kPilotMan.FindPilot(kPlayerGuid);
	if( !pkPlayerPilot )
	{
		return false;
	}
	
	kPlayerName = pkPlayerPilot->GetName();
	return true;
}

inline bool GetPartyPlayerName(BM::GUID const& kPlayerGuid, std::wstring & kPlayerName)
{
	SPartyMember const* pkMember = NULL;
	if( !g_kParty.GetMember(kPlayerGuid, pkMember) )
	{
		return false;
	}

	kPlayerName = pkMember->kName;
	return true;
}

bool lwCopyPlayerNameToClipBoard(lwGUID const kPlayerGuid) 
{	
	if( kPlayerGuid.IsNil() )
	{
		return false;
	}

	BM::GUID const kGuid = kPlayerGuid.GetGUID();
	std::wstring kPlayerName;
	if( false==GetFindPlayerName(kGuid, kPlayerName) )//블록이 없음!!
	if( false==GetPartyPlayerName(kGuid, kPlayerName) )
	{
		return false;
	}

	//PlayerName의메모리 공간을 계산 하고
	std::wstring::size_type const size = (kPlayerName.length()+1) * sizeof(std::wstring::value_type);	
	
	//메모리를 할당한다.
	HGLOBAL const hGlobal = GlobalAlloc(GHND, size);

	if ( !hGlobal )
	{//할당이 실패하면 return false
		GlobalFree(hGlobal);
		return false;
	}
	
	//PlayerName을 hGlobal의 메모리 공간에 복사 한다
	wchar_t* pcPlayerName = static_cast<wchar_t*>(GlobalLock(hGlobal));
	::memcpy(pcPlayerName, kPlayerName.c_str(), size);

	::GlobalUnlock(hGlobal);

	if ( ! ::OpenClipboard(NULL) ) 
	{//Clipboard 열기가 실패하면, 메모리 해제후 return false
		GlobalFree(hGlobal);
		return false;
	}

	//Clipboard를 비우고
	::EmptyClipboard();
	
	//유니코드 문자열인 PlayerName을 저장한다
	::SetClipboardData(CF_UNICODETEXT, hGlobal);
	
	//hGlobal 메모리를 해제하고
	::GlobalFree(hGlobal);

	//Clipboard를 닫고
	::CloseClipboard();
	
	//성공적 종료
	return true;
}

void lwCSNotifyCheck()
{
	if( !g_pkWorld ){ return; }
	if( !g_pkWorld->IsHaveAttr(GATTR_DEFAULT) ){ return; }

	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		BM::Stream	kPacket(PT_C_M_CS_REQ_LAST_RECVED_GIFT);
		NETWORK_SEND(kPacket);
	}
}

bool lwParticleTextureChange(char const* pcParticle, float const fScale, char const* pcNode, int const GeoNo, char const* pcTexPath)
{
	NiAVObject *pkParticle = g_kParticleMan.GetParticle( pcParticle,PgParticle::O_SCALE, fScale );
	if(!pkParticle)
		return false;

	NiNode* pNode = NiDynamicCast(NiNode, pkParticle->GetObjectByName(pcNode));
	if( !pNode )
		return false;

	NiSourceTexturePtr spChangeTargetTex = g_kNifMan.GetTexture(pcTexPath);
	if( !spChangeTargetTex )
		return false;

	NiGeometry* pkTexGeom = NiDynamicCast(NiGeometry, pNode->GetAt(GeoNo));
	if( !pkTexGeom )
		return false;

	NiPropertyStatePtr pkPropertyState = pkTexGeom->GetPropertyState();
	if( !pkPropertyState )
		return false;

	NiTexturingProperty* pkTexProperty = pkPropertyState->GetTexturing();
	if( !pkTexProperty )
		return false;

	pkTexProperty->SetBaseTexture( spChangeTargetTex );
	pkParticle->UpdateProperties();
	pkParticle->UpdateEffects();
	pkParticle->Update(0.0f);

    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkParticle, false );
	return true;
}

void UIMacroCheck::RecvMC_Command(WORD const wPacketType, BM::Stream& rkPacket)
{
	switch( wPacketType )
	{
	case PT_M_C_NOTI_MACRO_INPUT_PASSWORD:
		{
			wchar_t wcKey = 0;
			int iIteration = 0;
			short sType = 0;
			int	iLimitTime =0;
			rkPacket.Pop(wcKey);
			rkPacket.Pop(iIteration);
			rkPacket.Pop(sType);
			rkPacket.Pop(iLimitTime);
						
			PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
			if(pkMyPlayer) 
			{// 매크로 패킷이 먼저 오므로, 매크로 기다림 세팅을 서버보다 먼저 해준다.
				pkMyPlayer->MacroWaitAns(true);
			}
			g_kMacroChk.RememberCheckInfo(wcKey, iIteration, sType, iLimitTime);
			CallMacroCheckUI(wcKey, iIteration, sType);
		}break;
	case PT_M_C_ANS_MACRO_INPUT_PASSWORD:
		{
			XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("FRM_CALL_ORDER"));
			if( pWnd )
			{
				pWnd->Close();
			}
		}break;
	}
}

void UIMacroCheck::CallMacroCheckUI(wchar_t const wcKey, int const iIteration, short const sType)
{
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("FRM_CALL_ORDER"));
	if( !pWnd || pWnd->IsClosed() )
	{// 매크로 UI를 불러 와
		pWnd = XUIMgr.Call(_T("FRM_CALL_ORDER"));
		if( !pWnd )
		{
			return;
		}
	}
	// UI의 위치를 캐릭터를 가리지 않는 범위 내에서 랜덤하게 설정한 후
	pWnd->AlignX(BM::Rand_Range(100));
	pWnd->AlignY(BM::Rand_Range(25));

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(_T("LIST_TEXT")));
	XUI::CXUI_Wnd* pPassCode = pWnd->GetControl(_T("FRM_PASSCODE"));
	XUI::CXUI_Wnd* pTime = pWnd->GetControl(_T("FRM_TIME"));
	XUI::CXUI_Wnd* pNfy = pWnd->GetControl(_T("FRM_NFY_TEXT"));
	if( !pList || !pPassCode || !pTime || !pNfy)
	{
		return;	
	}
	std::wstring const kPassCode = MakePassCode(wcKey, iIteration);
	pPassCode->Text(kPassCode);
	
	{// %s에 해당하는 문자가 몇개 있는지 찾으라는 알림 문장을 만들고
		std::wstring kNfyText=TTW(790200);	
		std::wstring kKeyStrs;
		kKeyStrs+=wcKey;
		size_t const NullStrSize = 1;
		size_t const BufSize = kNfyText.size()+kKeyStrs.size()+NullStrSize;
		wchar_t* pBuff= new wchar_t[BufSize];
		::memset(pBuff,NULL, sizeof(wchar_t)*BufSize);
		::wsprintfW(pBuff, kNfyText.c_str(), kKeyStrs.c_str());
		kNfyText = pBuff;
		SAFE_DELETE_ARRAY(pBuff);
		pNfy->Text(kNfyText);
	}

	POINT2 const DefaultSize(pList->Size().x - 5, pList->Size().y - 5);

	XUI::SListItem*	pItem = NULL;
	if( 0 == pList->GetTotalItemCount() )
	{
		pItem = pList->AddItem(_T(""));
	}
	else
	{
		pItem = pList->FirstItem();
	}

	if( !pItem )
	{
		return;
	}
	XUI::CXUI_Wnd* pText = pItem->m_pWnd;
	pText->Size(DefaultSize);

	std::wstring wstrSoundID = _T("Macro_Warning");
	switch( sType )
	{// 만약 한번 이상 틀렸을 경우 소리를 재생하고
	case 1:{ wstrSoundID = _T("Macro_Fail01"); }break;
	case 2:{ wstrSoundID = _T("Macro_Fail02"); }break;
	default:{}break;
	}
	g_kUISound.PlaySoundByID(wstrSoundID);
	
	// NPC가 전하는 말을 출력 한다
	std::wstring kText = TTW(2404 + sType);
	pText->Text(kText);
	XUI::CXUI_Style_String	kString = pText->StyleText();
	POINT2 kTextSize(Pg2DString::CalculateOnlySize(kString));
	static bool bFirstTime = true;
	if( kTextSize.y > DefaultSize.y )
	{
		if(bFirstTime)
		{// 처음 뜰때는 텍스트에 대한 크기가 조정이 적절하게 되나
			bFirstTime = false;
			kTextSize.y += 10;
		}
		else
		{// 두번째 뜰때는 크기 조정이 맞지 않아 하드코딩으로 수정함
			kTextSize.y += 30;
		}
		pText->Size(kTextSize);
	}
	else
	{
		pText->TextPos(POINT2(pText->TextPos().x, (DefaultSize.y - kTextSize.y) * 0.5f));
	}
	pText->SetInvalidate();
	
	__int64 i64Time = g_kEventView.GetLocalSecTime();
	pTime->SetCustomData(&i64Time, sizeof(i64Time));
	XUIMgr.ClearEditFocus();
}

std::wstring const UIMacroCheck::PassCodeColorInput(std::wstring const& kStr)
{
	std::wstring ResultStr(_T(""));

	BYTE const MAX_COLOR = 8;
	DWORD const DEF_COLOR[MAX_COLOR] = { 0xFFFFFF00, 0xFFF40101, 0xFF12FF00, 0xFF402100, 0xFFC60087, 0xFF00FFFF, 0xFF0042FF, 0xFFFFFFFF, };

	for(int i = 0; i < kStr.size(); ++i)
	{
		wchar_t szTemp[MAX_PATH] = {0,};
		swprintf_s(szTemp, MAX_PATH, _T("{C=%x/}"), DEF_COLOR[BM::Rand_Index(MAX_COLOR)]);
		ResultStr += szTemp;
		ResultStr += kStr.at(i);
	}
	return ResultStr;
}

void UIMacroCheck::lwUpdateMacroTimer(lwUIWnd UISelf)
{
	int const TOTAL_DELAY_TIME = g_kMacroChk.GetCurLimitTime();

	XUI::CXUI_Wnd* pWnd = UISelf.GetSelf();
	if( !pWnd )
	{
		return;
	}

	__int64 i64Time = 0;
	pWnd->GetCustomData(&i64Time, sizeof(i64Time));
	
	__int64 iNowTime = g_kEventView.GetLocalSecTime();
	int iTick = static_cast<int>(iNowTime - i64Time);

	int Min = (TOTAL_DELAY_TIME - iTick) / 60;
	int Sec = (TOTAL_DELAY_TIME - iTick) % 60;

	BM::vstring vStr(Min);
	vStr += _T(":");
	wchar_t szTemp[MAX_PATH] = {0,};
	swprintf_s(szTemp, MAX_PATH, _T("%02d"), Sec);
	vStr += szTemp;

	pWnd->Text((std::wstring const&)vStr);

	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if(pkMyPlayer) 
	{// 플레이어가 존재하고
		if(g_kMacroChk.IsComplete() || !pkMyPlayer->IsAlive())
		{// 매크로가 완료 되었거나
		 // 플레이어가 죽었다면 매크로 방지 UI를 닫는다
			lwCloseUI("FRM_CALL_ORDER");
		}
	}
}

void UIMacroCheck::lwSendMacroPassword(int const iAnswer)
{
	BM::Stream	kPacket(PT_C_M_REQ_MACRO_INPUT_PASSWORD);
	kPacket.Push(iAnswer);
	NETWORK_SEND(kPacket);
	//if(2 == g_kMacroChk.GetType())
	//{// 두번 틀린 상태에서 보낸 답은 틀리건 맞건 더이상 기억할 필요가 없다.
	//	g_kMacroChk.Complete();
	//}
}

void UIMacroCheck::lwCloseMacroUI(lwUIWnd UISelf)
{//  매크로 방지 UI의 내용이 캐릭터 사라지게 하는것이 아니게 됨에 따라, 사용자 입력 잠금 해제 부분을 주석처리
//	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MacroCheck, false));
//	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MacroCheck, true));
}

std::wstring const UIMacroCheck::MakePassCode(wchar_t const wcKey, int const iIteration)
{// wcKey가 iIteration 만큼 포함되어있는, 총 10개 문자로된 매크로 passcode를 만든다
	BYTE const MAX_COLOR = 8;
	DWORD const DEF_COLOR[MAX_COLOR] = { 0xFFFFFF00, 0xFFF40101, 0xFF12FF00, 0xFF402100, 0xFFC60087, 0xFF00FFFF, 0xFF0042FF, 0xFFFFFFFF, };
	int const PASSCODE_MAX = 10;

	wchar_t const wEngStartCh = L'A';
	wchar_t const wEngEndCh = L'Z';
	wchar_t const wNumStartCh = L'2';
	wchar_t const wNumEndCh = L'9';

	
	std::wstring kRandomWch;	
	{// 먼저 랜덤문자 집합을 만들고
		for(wchar_t wCh=wEngStartCh; wCh<=wEngEndCh; ++wCh)
		{
			kRandomWch.push_back(wCh);
		}
		for(wchar_t wCh=wNumStartCh; wCh<=wNumEndCh; ++wCh)
		{
			kRandomWch.push_back(wCh);
		}
	}
	
	{//wcKey의 값을 랜덤문자 집합에서 제거한후
		size_t const EraseIndex = kRandomWch.rfind(wcKey);
		if(EraseIndex == std::wstring::npos)
		{
			_PgMessageBox("UIMacroCheck::MakePassCode","Undefined pass key!");
			return std::wstring(L"Error");
		}
		kRandomWch.replace(EraseIndex,1,L"");
	}

	// 정해진 pass문자를 iIteration 만큼 삽입하고
	std::wstring kPassCode(iIteration, wcKey);

	int const iUntil = PASSCODE_MAX-iIteration;
	for(int i=0; i<iUntil; ++i)
	{// 영어 대문자, 숫자가 pass문자와 합해서 총 10개 문자가 될때까지 무작위로 넣고
		size_t const RandIndex = BM::Rand_Index(kRandomWch.size());
		std::wstring::value_type const& wCh = kRandomWch.at(RandIndex);
		kPassCode.push_back(wCh);
	}

	for(int i=0; i<kPassCode.size(); ++i)
	{// 문자들을 마구 섞은후
		size_t sel = BM::Rand_Index(kPassCode.size());
		std::wstring::value_type const origin = kPassCode.at(i);
		std::wstring::value_type const target = kPassCode.at(sel);		
		kPassCode.at(i) = target;
		kPassCode.at(sel) = origin;
	}
	
	std::wstring kResult;
	for(int i = 0; i < kPassCode.size(); ++i)
	{// 색을 랜덤하게 지정한다
		wchar_t szTemp[MAX_PATH] = {0,};
		swprintf_s(szTemp, MAX_PATH, _T("{C=%x/}"), DEF_COLOR[BM::Rand_Index(MAX_COLOR)]);		
		kResult += szTemp;
		kResult += kPassCode.at(i);
	}
	return kResult;
}

bool UIMacroCheck::lwIsOnMacroCheck()
{
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("FRM_CALL_ORDER"));
	if( !pWnd || pWnd->IsClosed() )
	{// 메크로 체크 UI가 없다면
		return false;
	}
	::Notice_Show(TTW(790206), EL_Warning);
	return true;
}

void UIMacroCheck::lwCallMacroCheckUI_IfNotComplete(int iElapsTimeMilSec)
{
	if(!g_kMacroChk.IsComplete())
	{// 하지만 매크로 체크가 진행중이고
		if( NULL != g_pkWorld
			&& GATTR_FLAG_VILLAGE != g_pkWorld->GetAttr() )
		{
			// 마을이 아니면 무조건 띄운다
			int CurLimitTime = g_kMacroChk.GetLimitTime() - static_cast<int>((iElapsTimeMilSec/1000));				
			g_kMacroChk.SetCurLimitTime(CurLimitTime);
			CallMacroCheckUI(g_kMacroChk.GetKey(), g_kMacroChk.GetIteration(), g_kMacroChk.GetType());
		}
	}
}

int lwGetLocale()
{	
	return g_kLocal.ServiceRegion();
	//switch(g_kLocal.NationCode())	
	//{	//case NC_NOT_SET:	//	 0x00000000,//NOTSET일 경우는 무조건 에러처리합니다.	
	//case NC_KOREA:		//	 0x00000001,	
	//case NC_CHINA:		//	 0x00000002,	
	//case NC_TAIWAN:		//	 0x00000004,		
	//case NC_THAI:		//	 0x00000008,
	//case NC_HONGKONG:	//	 0x00000010,	
	//case NC_USA:		//	 0x00000020,	
	//case NC_JAPAN:		//	 0x00000040,	
	//case NC_EU:			//	 0x00000100,//유럽	//}
}

DWORD lwGetLocalTime()
{
	return BM::GetTime32();
}

void UIGemStore::lwSendReqGemStore(lwGUID lwkGuid)
{// 보석상점 물품 리스트 요청
	g_kGemStore.SendReqGemStoreInfo(lwkGuid.GetGUID());
}

void UIGemStore::lwSendReqEventStore(lwGUID lwkGuid)
{// 보석상점 물품 리스트 요청
	g_kGemStore.SendReqEventStoreInfo(lwkGuid.GetGUID());
}

void UIGemStore::lwSetGemStoreType(int const iType)
{
	g_kGemStore.SetGemStoreType(static_cast<E_GEMSTORE_TYPE>(iType));
}

int UIGemStore::lwGetGemStoreType()
{
	return g_kGemStore.GetGemStoreType();
}

void UIGemStore::lwClickGemStoreListItem(lwUIWnd kListItem)
{
	XUI::CXUI_Wnd* pSelf = kListItem.GetSelf();
	if( !pSelf ){ return; }

	int iCPItemNo = 0;
	if( pSelf->GetCustomDataSize() )
	{
		pSelf->GetCustomData(&iCPItemNo, sizeof(iCPItemNo));
	}

	if( iCPItemNo == 0 )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent ){ return; }

	XUI::CXUI_Wnd* pSelectImg = pParent->GetControl(L"IMG_SELECT");
	if( !pSelectImg ){ return; }

	int const iNewBuildIndex = pSelf->BuildIndex();
	int const iOldBuildIndex = lwUIWnd(pSelectImg).GetCustomData<int>();

	if( iNewBuildIndex == iOldBuildIndex 
		&& pSelectImg->Visible() )
	{
		pSelectImg->Visible(false);
		pSelectImg->ClearCustomData();

		XUI::CXUI_Wnd* pkIconWnd = kListItem.GetSelf();
		if( !pkIconWnd ){ return; }

		XUI::CXUI_Wnd* pMainWnd = pkIconWnd->Parent();
		if(!pMainWnd){ return; }

		g_kGemStore.ClearNeedItemImg(pMainWnd);
		g_kGemStore.ResetCursor(kListItem);
	}
	else
	{
		POINT3I ptPos = pSelf->Location();
		pSelectImg->Location(ptPos.x, ptPos.y);
		pSelectImg->Visible(true);
		
		pSelectImg->SetCustomData(&iNewBuildIndex, sizeof(iNewBuildIndex));

		if( g_kGemStore.IsOtherMode() || g_kGemStore.IsEventMode())
		{
			g_kGemStore.ClickSlotItem(kListItem);
		}
		else
		{
			g_kGemStore.ClickDefSlotItem(kListItem);
			SetItemTree(g_kGemStore.GetSelectedItemNo(), g_kGemStore.GetOrderIndex());
		
		}
	}
}

void UIGemStore::ItemTreeClear()
{
	g_kGemStore.ItemTreeClear();
}

bool UIGemStore::IsItemTreeSlot(int const iSelectedItemNo, int const iOrderIndex)
{
	return g_kGemStore.IsItemTreeSlot(iSelectedItemNo, iOrderIndex);
}

void UIGemStore::SetItemTree(int const iSelectedItemNo, int const iOrderIndex)
{
	g_kGemStore.SetItemTreeSlot(iSelectedItemNo, iOrderIndex);
}

void UIGemStore::lwClearGemStoreData()
{
	g_kGemStore.Clear();
}
void UIGemStore::lwGemStoreDrawIconImage(lwUIWnd kSelf)
{
	g_kGemStore.DrawIconImage(kSelf);
}

void UIGemStore::lwGemStoreDrawIconTooltip(lwUIWnd kSelf)
{
	g_kGemStore.DrawItemTooltip(kSelf);
}

bool UIGemStore::lwSendReqGemTrade()
{
	return g_kGemStore.SendReqGemTrade();
}

bool UIGemStore::lwSendReqGemTradeTree()
{
	return g_kGemStore.SendReqGemTradeTree();
}

void UIGemStore::OnClickGemStoreBeginPage(lwUIWnd kSelf)
{
	g_kGemStore.BeginPage(kSelf);
}

void UIGemStore::OnClickGemStoreEndPage(lwUIWnd kSelf)
{
	g_kGemStore.EndPage(kSelf);
}

void UIGemStore::OnClickGemStoreJumpPrevPage(lwUIWnd kSelf)
{
	g_kGemStore.JumpPrevPage(kSelf);
}

void UIGemStore::OnClickGemStoreJumpNextPage(lwUIWnd kSelf)
{
	g_kGemStore.JumpNextPage(kSelf);
}

void UIGemStore::OnClickGemStorePage(lwUIWnd kSelf)
{
	g_kGemStore.Page(kSelf);
}

int UIGemStore::lwGetGemStoreTitleMenu()
{
	return g_kGemStore.GetGemStoreTitleMenu();
}

void CallGetItemAlam(std::wstring const& wstrStr, int const iItemNo, int const iMilliSec, const int iLocationY)
{// 얻은 아이템을 UI로 표현한다
 // itemNo의 아이템을 아이콘으로 표현하고 wstrStr의 문장으로 UI를 채워, iMilliSec초 동안 유지한다
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Activate(_T("FRM_GET_ITEM"));
	if(!pkWnd)
	{
		return;
	}

	static int iOriginalY = pkWnd->Location().y;

	if( iLocationY )
	{
		pkWnd->Location( pkWnd->Location().x, iLocationY );
	}
	else
	{
		pkWnd->Location( pkWnd->Location().x, iOriginalY );
	}

	// 얻은 아이템을 보여준다
	XUI::CXUI_Icon* pkIconWnd = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(_T("ICON")));
	if(pkIconWnd)
	{
		pkIconWnd->SetCustomData(&iItemNo, sizeof(iItemNo));
	}
	XUI::CXUI_Wnd* pkTextWnd = pkWnd->GetControl(_T("FRM_MSG"));
	if(pkTextWnd)
	{// 아이템 획득 표시
		pkTextWnd->Text(wstrStr);
	}
	pkWnd->AliveTime(iMilliSec);
}

void CallGetItemAlam2(std::wstring const& wstrStr, int const iItemNo, int const iMilliSec)
{
	XUI::CXUI_Wnd* pkMain = XUIMgr.Activate(L"FRM_GET_ITEM2");
	if(!pkMain)
	{
		return;
	}
	//pkMain->AlignX(iAlignX);
	//pkMain->AlignY(iAlignY);

	PgBase_Item kItem;
	kItem.ItemNo(iItemNo);

	XUI::CXUI_Wnd* pIcon = pkMain->GetControl(L"ICN_ITEM");
	if (!pIcon)
	{
		return;
	}	
	lwUIWnd(pIcon).SetCustomDataAsSItem(kItem);
	PgUIUtil::DrawIconToItemNo(pIcon, kItem.ItemNo(), 1.5f);

	const wchar_t *pText = NULL;
	std::wstring wstrText;
	GetDefString(kItem.ItemNo(), pText);

	BM::vstring vStr(wstrStr);
	vStr.Replace(L"%s", pText);

	pkMain->Text(static_cast<std::wstring>(vStr).c_str());
}

void lwSetUISizeForTextY(lwUIWnd kWnd)
{// 소유한 Text에 맞게 사이즈를 조정한다
	XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
	if(!pkWnd)
	{
		return;
	}
	XUI::CXUI_Style_String kStyleString = pkWnd->StyleText();
	POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
	POINT2 kSize(pkWnd->Width(), kTextSize.y);
	pkWnd->Size(kSize);
	pkWnd->SetInvalidate();
}

char const* lwGetKeynoToName(int const iKeyNo)
{// NiKey 값에 해당하는 키보드 문자를 넘겨준다

	if(!iKeyNo)
	{
		return "";
	}

	lwWString lwStr(TTW(420000+iKeyNo));
	return lwStr.GetStr();
}

void lwOpenURL(char const *szText)
{
	BM::vstring szURL(szText);
	
	HINSTANCE hInstance = ShellExecute(NULL, _T("open") , (NULL==szText ? g_szSuperGirlURL : UNI(szText)) , NULL, NULL, SW_SHOWNORMAL);
}

void SendEmotion(int const iID)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		::CONT_EMOTION const * pContEmotion = NULL;
		g_kTblDataMgr.GetContDef(pContEmotion);	//맵 데프
		if( !pContEmotion )
		{
			return;
		}

		::CONT_EMOTION::const_iterator c_iter = pContEmotion->find(SEMOTIONKEY(ET_EMOTION, iID));
		if( c_iter == pContEmotion->end() )
		{
			return;
		}

		PgBitArray<MAX_DB_EMOTICON_SIZE> kCont;
		if( pPlayer->GetContEmotion(ET_EMOTION, kCont) )
		{
			if( c_iter->second == 0 || kCont.Get(iID) )
			{
				if(CheckCanEmotion(iID))
				{
					SendEmotionPacket(g_kPilotMan.PlayerActor(), ET_EMOTION, iID);
				}				
			}
		}
	}
}

namespace Item_Convert
{
	typedef struct tagTrade
	{
		tagTrade()
			:iItemNo(0), iMakingNo(0)
		{
		}

		tagTrade(int const ItemNo, int const MakingNo)
			:iItemNo(ItemNo), iMakingNo(MakingNo)
		{
		}

		tagTrade(tagTrade const& rhs)
			:iItemNo(rhs.iItemNo), iMakingNo(rhs.iMakingNo)
		{
		}

		tagTrade operator = (tagTrade const& rhs)
		{
			iItemNo = rhs.iItemNo;
			iMakingNo = rhs.iMakingNo;
			return *this;
		}

		bool operator == (tagTrade const& rhs)const
		{
			return (iItemNo == rhs.iItemNo) && (iMakingNo == rhs.iMakingNo);
		}

		bool Empty()const
		{
			return (0 == iItemNo) || (0 == iMakingNo);
		}

		int iItemNo;
		int iMakingNo;
	} STrade;
	typedef std::vector< STrade > ContTrade;
	typedef std::map< int, ContTrade > ContTradeTable;


	//
	std::wstring const kItem_ConvertWndName(L"FRM_ITEM_CONVERT");
	int const iConvertItemNo = 60000960;
	int const iDefaulTadetLocationNo = 1;
	ContTradeTable kTradeTable;

	//
	void AddItem_ConvertItem(int const iLocation, int iItemNo, int iMakingNo)
	{
		ContTradeTable::iterator find_iter = kTradeTable.find(iLocation);
		if( kTradeTable.end() == find_iter )
		{
			std::pair<ContTradeTable::iterator, bool> kRet = kTradeTable.insert( std::make_pair(iLocation, ContTradeTable::mapped_type()) );
			if( kRet.second )
			{
				find_iter = kRet.first;
			}
			else
			{
				return;
			}
		}

		STrade kNewTrade(iItemNo, iMakingNo);
		ContTradeTable::mapped_type &rkElement = (*find_iter).second;
		if( rkElement.end() == std::find(rkElement.begin(), rkElement.end(), kNewTrade)
		&&	!kNewTrade.Empty() )
		{
			std::back_inserter(rkElement) = kNewTrade;
		}
	}


	void InitTradeTable()
	{
		lua_tinker::call<void>("InitItem_ConvertTable");
	}


	void SetTradeTopWndCustomData(CXUI_Wnd* pkWnd, int const iCurTradeLocation, bool const bReset)
	{
		if( !pkWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		kCustomData.Push( iCurTradeLocation );
		kCustomData.Push( bReset );
		pkWnd->SetCustomData( kCustomData.Data() );
	}
	void GetTradeTopWndCustomData(CXUI_Wnd* pkWnd, int& iCurTradeLocation, bool& bReset)
	{
		if( !pkWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		if( !pkWnd->GetCustomData(kCustomData.Data()) )
		{
			return;
		}
		kCustomData.PosAdjust();
		kCustomData.Pop( iCurTradeLocation );
		kCustomData.Pop( bReset );
	}

	void SetItem_ConvertLocation(int const iTradeLocation)
	{
		CXUI_Wnd* pkTopWnd = XUIMgr.Get(kItem_ConvertWndName);
		if( !pkTopWnd )
		{
			return;
		}

		if( kTradeTable.end() == kTradeTable.find(iTradeLocation) )
		{
			return;
		}

		int iOldTradeLocation = 0;
		bool bReset = false;
		GetTradeTopWndCustomData(pkTopWnd, iOldTradeLocation, bReset);
		SetTradeTopWndCustomData(pkTopWnd, iTradeLocation, iOldTradeLocation != iTradeLocation);
	}

	void SetTradeItemCustomData(CXUI_Wnd* pkItemWnd, STrade const& rkTradeInfo, int const iCount, bool const bReset)
	{
		if( !pkItemWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		kCustomData.Push( rkTradeInfo );
		kCustomData.Push( iCount );
		kCustomData.Push( bReset );
		pkItemWnd->SetCustomData( kCustomData.Data() );
	}
	void GetTradeItemCustomData(CXUI_Wnd* pkItemWnd, STrade& rkTradeInfo, int& iCount, bool& bReset)
	{
		if( !pkItemWnd )
		{
			return;
		}

		BM::Stream kCustomData;
		if( !pkItemWnd->GetCustomData(kCustomData.Data()) )
		{
			return;
		}
		kCustomData.PosAdjust();
		kCustomData.Pop( rkTradeInfo );
		kCustomData.Pop( iCount );
		kCustomData.Pop( bReset );
	}
	int UpdateTradeItemWnd(CXUI_Wnd* pkItemWnd, STrade const& rkTradeInfo, bool const bReset)
	{
		if( !pkItemWnd )
		{
			return 0;
		}

		GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
		SDefItemMakingData kMakingInfo;
		if( !kItemMakingDefMgr.GetDefItemMakingInfo(kMakingInfo, rkTradeInfo.iMakingNo) )
		{
			return 0;
		}

		int const iNeedItemNo = kMakingInfo.kNeedElements.aElement[0];
		int const iNeedItemCount = kMakingInfo.kNeedCount.aCount[0];

		TCHAR const* szNeedItemName = NULL;
		if( !GetItemName(iNeedItemNo, szNeedItemName) ) // 재료 아이템 이름
		{
			return 0;
		}

		std::wstring kNeedItemText;
		if( !FormatTTW(kNeedItemText, 400459, szNeedItemName, iNeedItemCount) )
		{
			return 0;
		}

		int const iMaxCount = SoulCraft::GetCanMaxTradeMaking(rkTradeInfo.iMakingNo);

		int iCount = 0;
		STrade kCurTradeInfo;
		bool bCurReset = false;
		GetTradeItemCustomData(pkItemWnd, kCurTradeInfo, iCount, bCurReset);

		iCount = (bReset || bCurReset)? 0: std::min(iCount, iMaxCount); // 제작가능한 최대 갯수로 제한

		pkItemWnd->Text( kNeedItemText ); // 영혼석 몇개 필요?
		CXUI_Wnd* pkIconWnd = pkItemWnd->GetControl( std::wstring(_T("ICON_Item")) );
		if( pkIconWnd ) // 아이템 이미지 설정
		{
			SoulCraft::SetSimpleItemIcon(pkIconWnd, rkTradeInfo.iItemNo);
		}
		CXUI_Wnd* pkResultItemWnd = pkItemWnd->GetControl( std::wstring(_T("SFRM_RESULT_ITEM")) );
		if( pkResultItemWnd ) // 결과 아이템이 뭐?
		{
			TCHAR const* szResultItemName = NULL;
			if( GetItemName(rkTradeInfo.iItemNo, szResultItemName) )
			{
				Quest::SetCutedTextLimitLength(pkResultItemWnd, std::wstring(szResultItemName), std::wstring(_T("...")));
			}
		}
		CXUI_Wnd* pkMakeCountWnd = pkItemWnd->GetControl( std::wstring(_T("SFRM_COUNT")) );
		if( pkMakeCountWnd ) // 몇개!
		{
			BM::vstring const kCountStr(iCount);
			pkMakeCountWnd->Text( kCountStr );
		}

		SetTradeItemCustomData(pkItemWnd, rkTradeInfo, iCount, false);
		return iCount * iNeedItemCount;
	}
	void IncItem_ConvertItemWnd(lwUIWnd kItemWnd, int const iIncVal)
	{
		CXUI_Wnd* pkTopWnd = kItemWnd();
		if( !pkTopWnd )
		{
			return;
		}

		STrade kTradeInfo;
		int iCount = 0;
		bool bReset = false;
		GetTradeItemCustomData(pkTopWnd, kTradeInfo, iCount, bReset);
		int const iMaxCount = SoulCraft::GetCanMaxTradeMaking(kTradeInfo.iMakingNo);
		if( 0 < iIncVal )
		{
			if( iMaxCount < (iCount+iIncVal) )
			{
				iCount = 0;
			}
			else
			{
				iCount = std::min(iCount+iIncVal, iMaxCount);
			}
		}
		else
		{
			if( 0 > (iCount+iIncVal) )
			{
				iCount = iMaxCount;
			}
			else
			{
				iCount = std::max(iCount+iIncVal, 0);
			}
		}
		SetTradeItemCustomData(pkTopWnd, kTradeInfo, iCount, false);
	}
	void SendItem_Convert(lwUIWnd kItemWnd)
	{
		CXUI_Wnd* pkTopWnd = kItemWnd();
		if( !pkTopWnd )
		{
			return;
		}

		STrade kTradeInfo;
		int iCount = 0;
		bool bReset = false;
		GetTradeItemCustomData(kItemWnd(), kTradeInfo, iCount, bReset);
		if( 0 == iCount
		||	kTradeInfo.Empty() )
		{
			return;
		}
		SetTradeItemCustomData(kItemWnd(), kTradeInfo, iCount, true); // 다음번 업데이트시 0으로
		
		GET_DEF(CItemMakingDefMgr, kItemMakingDefMgr);
		SDefItemMakingData kMakingInfo;
		if( !kItemMakingDefMgr.GetDefItemMakingInfo(kMakingInfo, kTradeInfo.iMakingNo) )
		{
			return;
		}

		SReqItemMaking kReqItemMaking;
		if( !SoulCraft::CreateMakingOrder(kMakingInfo, kReqItemMaking, iCount) )
		{
			return;
		}


		BM::Stream kPacket(PT_C_M_REQ_ITEM_MAKING);
		kPacket.Push(1);
		kReqItemMaking.WriteToPacket(kPacket);
		kPacket.Push(SItemPos());
		kPacket.Push(SItemPos());

		NETWORK_SEND(kPacket)
	}
	void CallItem_ConvertToolTip(lwUIWnd kIconWnd)
	{
		CXUI_Wnd* pkIconWnd = kIconWnd();
		if( pkIconWnd )
		{
			STrade kTradeInfo;
			int iCount = 0;
			bool bReset = false;
			GetTradeItemCustomData(pkIconWnd->Parent(), kTradeInfo, iCount, bReset);
			if( !kTradeInfo.Empty() )
			{
				CallToolTip_ItemNo(kTradeInfo.iItemNo, kIconWnd.GetLocation());
			}
		}
	}

	void UpdateItem_Convert()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgInventory const* pkInventory = pkPlayer->GetInven();
		if( !pkInventory )
		{
			return;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pkSoulStoneItemDef = kItemDefMgr.GetDef(iConvertItemNo);
		if( !pkSoulStoneItemDef )
		{
			return;
		}

		CXUI_Wnd* pkTopWnd = XUIMgr.Get(kItem_ConvertWndName);
		if( !pkTopWnd )
		{
			pkTopWnd = XUIMgr.Call(kItem_ConvertWndName);
			if( !pkTopWnd )
			{
				return;
			}
		}

		InitTradeTable();

		int iTradeLocation = 0;
		bool bReset = false;
		GetTradeTopWndCustomData(pkTopWnd, iTradeLocation, bReset);
		if( 0 == iTradeLocation )
		{
			iTradeLocation = iDefaulTadetLocationNo;
			bReset = true;
		}

		//
		int iReservePayCount = 0;
		ContTradeTable::const_iterator trade_iter = kTradeTable.find( iTradeLocation );
		CXUI_List* pkList = dynamic_cast<CXUI_List*>(pkTopWnd->GetControl( std::wstring(_T("LIST_TRADE_LIST")) ));
		if( pkList
		&&	kTradeTable.end() != trade_iter )
		{
			ContTradeTable::mapped_type const& rkItemList = (*trade_iter).second;
			ContTrade::const_iterator item_iter = rkItemList.begin();

			// 있을 때
			// 없을 때
			SListItem* pkListItem = pkList->FirstItem();
			while( rkItemList.end() != item_iter )
			{
				SListItem* pkCurListItem = (pkListItem)? pkListItem: pkList->AddItem(std::wstring());
				if( pkCurListItem
				&&	pkCurListItem->m_pWnd )
				{
					int const iNeedCount = UpdateTradeItemWnd(pkCurListItem->m_pWnd, (*item_iter), bReset);
					iReservePayCount += iNeedCount;
				}

				pkListItem = (pkListItem)? pkList->NextItem(pkListItem): NULL;
				++item_iter;
			}

			// 남았을 때
			while( pkListItem )
			{
				pkListItem = pkList->DeleteItem(pkListItem);
			}
		}

		//
		CXUI_Wnd* pkLocalNameWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_LOCATION")) );
		if( pkLocalNameWnd )
		{
			int const iTextID = lua_tinker::call<int>("GetSoulTradeLocationText", iTradeLocation);
			if( iTextID )
			{
				pkLocalNameWnd->Text( TTW(iTextID) );
			}
		}

		CXUI_Wnd* pkSoulStoneNameWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_RESULT_ITEM")) );
		if( pkSoulStoneNameWnd )
		{
			std::wstring const *pkItemName = NULL;
			if( GetDefString(pkSoulStoneItemDef->NameNo(), pkItemName) )
			{
				pkSoulStoneNameWnd->Text( *pkItemName );
			}
		}

		size_t const iHaveNeedItemCount = pkInventory->GetTotalCount(iConvertItemNo);
		CXUI_Wnd* pkSoulStonePayWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_COUNT_PAY")) );
		if( pkSoulStonePayWnd )
		{
			DWORD const dwOverCountColor = 0xFFFF3413;
			DWORD const dwCanCountColor = 0xFF4D3413;
			BM::vstring const kPayCount( -iReservePayCount );
			pkSoulStonePayWnd->Text( kPayCount );
			pkSoulStonePayWnd->FontColor( (iReservePayCount > iHaveNeedItemCount)? dwOverCountColor: dwCanCountColor);
		}

		CXUI_Wnd* pkSoulStoneCountWnd = pkTopWnd->GetControl( std::wstring(_T("SFRM_COUNT")) );
		if( pkSoulStoneCountWnd )
		{
			BM::vstring const kSoulStoneCount( iHaveNeedItemCount );
			pkSoulStoneCountWnd->Text( kSoulStoneCount );
		}
		SetTradeTopWndCustomData(pkTopWnd, iTradeLocation, false);
	}
}

namespace XUIListUtil
{
	void SetMaxItemCount(XUI::CXUI_List* pList, int const iMax)
	{
		if( pList )
		{
			int const iListCount = pList->GetTotalItemCount();
			if( iListCount < iMax )
			{
				for(int i = iListCount; i < iMax; ++i )
				{
					pList->AddItem(L"");
				}
			}
			else if( iListCount > iMax )
			{
				for(int i = iListCount; i > iMax; --i)
				{
					pList->DeleteItem(pList->FirstItem());
				}
			}			
		}
	}
};

namespace PgUIUtil
{
	void DrawIconToItemNo(XUI::CXUI_Wnd* pWnd, int const iItemNo, float const fScale)
	{
		if( !pWnd )
		{
			return;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		const CItemDef* pItemDef = kItemDefMgr.GetDef(iItemNo);

		PgUISpriteObject* pkSprite = g_kUIScene.GetIconTexture(iItemNo);
		if( !pkSprite )
		{
			pWnd->DefaultImgTexture(NULL);
			pWnd->SetInvalidate();
			return;
		}

		PgUIUVSpriteObject* pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);
		if( !pkUVSprite ){ return; }

		pWnd->DefaultImgTexture(pkUVSprite);
		SUVInfo& rkUV = pkUVSprite->GetUVInfo();
		pWnd->UVInfo(rkUV);
		POINT2	kPoint(40*rkUV.U, 40*rkUV.V);
		pWnd->ImgSize(kPoint);
		if( fScale != 1.0f )
		{
			POINT3I	kLocation = pWnd->Location();
			pWnd->VScale(fScale);
			pWnd->Location(kLocation);
		}
		pWnd->SetInvalidate();		
	}

	void ResizeFormSize(XUI::CXUI_Wnd* pWnd, int const ExtendSizeX, int const ExtendSizeY, bool const bChange)
	{
		if( !pWnd )
		{
			return;
		}

		POINT2	kSize = pWnd->Size();
		if( bChange )
		{
			kSize.x = (ExtendSizeX)?(ExtendSizeX):(kSize.x);
			kSize.y = (ExtendSizeY)?(ExtendSizeY):(kSize.y);
		}
		else
		{
			kSize.x += ExtendSizeX;
			kSize.y += ExtendSizeY;
		}
		pWnd->Size(kSize);
	}

	void MoveFormLocation(XUI::CXUI_Wnd* pWnd, int const MoveLocX, int const MoveLocY, bool const bChange)
	{
		if( !pWnd )
		{
			return;
		}

		POINT3I kLoc = pWnd->Location();
		if( bChange )
		{
			kLoc.x = (MoveLocX)?(MoveLocX):(kLoc.x);
			kLoc.y = (MoveLocY)?(MoveLocY):(kLoc.y);
		}
		else
		{
			kLoc.x += MoveLocX;
			kLoc.y += MoveLocY;
		}
		pWnd->Location(kLoc);
	}
};

bool lwIgnoreChannelUI(bool const bSet, bool const bIgnore)
{
	static bool bIgnoreChannelUI = false;
	if(bSet)
	{
		bIgnoreChannelUI = bIgnore;
	}
	return bIgnoreChannelUI;
}

void lwRemoveBuffEffect(lwUIWnd kWnd)
{
	if(kWnd.IsNil())
	{
		return;
	}
	int const iEffectNo = kWnd.GetCustomData<int>();
	{
		BM::GUID kPlayerGUID;
		if(true == g_kPilotMan.GetPlayerPilotGuid(kPlayerGUID))
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkPlayer)
			{
				return;
			}
			CEffect* pkEffect = pkPlayer->GetEffect(iEffectNo);
			if(!pkEffect)
			{
				return;
			}
			if(0 < pkEffect->GetAbil(AT_IGNORE_REQ_USER_DEL_EFFECT))
			{// 유저가 삭제 할수 없는 이펙트 속성 일때는 삭제 불능
				return;
			}
			switch(pkEffect->GetAbil(AT_TYPE))
			{
			case EFFECT_TYPE_BLESSED:
			case EFFECT_TYPE_ITEM:
				{
				}break;
			default:
				{// 패시브, 패널티, 디버프 타입은 삭제 불능
					return;
				}break;
			}
			BM::Stream kPacket(PT_C_M_REQ_DELETE_BUFFEFFECT);
			kPacket.Push(kPlayerGUID);
			kPacket.Push(iEffectNo);
			{// 정말 지울것인지 물어본다
				lwCloseToolTip();
				lwCallCommonMsgYesNoBox(MB(TTW(790030)), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
			}
		}
	}
}

void SetBlockNickFilterProcessType(int iType)
{
	typedef BM::PgFilterString::EFilterProcessType EFilterType;
	switch(iType)
	{
	case EFilterType::E_SIMPLE_FILTER:	// 1
	case EFilterType::E_DETAIL_FILTER:	// 2
		{
			g_kClientFS.SetBlockNickFilterProcessType(static_cast<EFilterType>(iType));
		}break;
	default:
		{
			return;
		}break;
	}
}

int GetBlockNickFilterProcessType()
{
	return g_kClientFS.GetBlockNickFilterProcessType();
}

void SetBadWordFilterProcessType(int iType)
{
	switch(iType)
	{
	case BM::PgFilterString::E_SIMPLE_FILTER:	// 1
	case BM::PgFilterString::E_DETAIL_FILTER:	// 2
		{
			g_kClientFS.SetBadWordFilterProcessType(static_cast<BM::PgFilterString::EFilterProcessType>(iType));
		}break;
	default:
		{
			return;
		}break;
	}
}

int GetBadWordFilterProcessType()
{
	return g_kClientFS.GetBadWordFilterProcessType();
}

void OnMsgBoxClose(lwUIWnd Self)
{
	if( Self.IsNil() ){ return; }

	XUI::CXUI_Wnd* pkWnd = dynamic_cast<XUI::CXUI_Wnd*>(Self.GetSelf());
	if( !pkWnd ){ return; }
	
	int iMsgNo = 0;
	pkWnd->GetCustomData(&iMsgNo, sizeof(iMsgNo));

	if(	g_kCmdLineParse.IsUseForceAccount()
	&&	g_iMsgNoAnswerServer == iMsgNo)//종료 안내 메세지 라면.
	{
		lwExitClient();	
	}
}

lwWString GetUKeynoToKeyStr(int const iUKeyNo)
{
	std::wstring kKeyWStr;
	if(true == g_kGlobalOption.GetUKeyToKeyStr(iUKeyNo, kKeyWStr))
	{
		std::string kKetStr;
		if(!kKeyWStr.empty())
		{
			int const KeyNo = ::_wtoi(kKeyWStr.c_str());
			kKetStr = lwGetKeynoToName(KeyNo);
		}
		return lwWString(kKetStr.c_str());
	}
	return lwWString(L"");
}

char const* lwGetCoupleName()
{
	lwWString lwStr(g_kCoupleMgr.GetMyInfo().CharName());
	return lwStr.GetStr();
}

int lwChangeAllListChkBtn(char const* pcParent, char const* pcList, char const* pcChkBtn, bool bCheckState)
{// list의 모든 체크 버튼의 체크 상태를 수정하고자 할때 사용
	if(!pcParent
		|| !pcList
		|| !pcChkBtn
		)
	{
		return 0;
	}

	int iCnt = 0;
	XUI::CXUI_Wnd* pkParent =  XUIMgr.Get(UNI(pcParent));
	if(!pkParent)
	{
		return iCnt;
	}
	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkParent->GetControl(UNI(pcList)));
	if(!pkList)
	{
		return iCnt;
	}
	
	XUI::SListItem* pkListItem = pkList->FirstItem();
	while(pkListItem)
	{
		XUI::CXUI_Wnd* pWnd = pkListItem->m_pWnd;
		if(pWnd)
		{
			XUI::CXUI_CheckButton* pkChkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pWnd->GetControl(UNI(pcChkBtn)));
			if(pkChkBtn
				&& bCheckState != pkChkBtn->Check()
				)
			{// 상태가 같지 않은것들만 수정한다
				pkChkBtn->Check(bCheckState);
				++iCnt;
			}
		}
		pkListItem = pkList->NextItem(pkListItem);
	}
	return iCnt;
}

bool AttatchParticleToActorNode(char* pcNpcActorName,int iSlot, char const* pcTargetName, char const* pcParticle, float fScale)
{// NPC의 Node(TargetName)에 파티클을 붙인다
	PgParticle *pkParticle = g_kParticleMan.GetParticle(pcParticle, PgParticle::O_SCALE,fScale);
	if (!pkParticle)
	{
		return false;
	}

	PgActor* pkNpc = g_kPilotMan.FindNpcActor(UNI(pcNpcActorName));
	if(pkNpc)
	{
		pkNpc->AttachTo(iSlot, pcTargetName, pkParticle);
		return true;
	}
	return false;
}

bool AttatchParticleToActorPos(char* pcNpcActorName,int iSlot, lwPoint3 kPos, char const* pcParticle, float fScale)
{// NPC의 위치를 기준으로 kPos를 더해 파티클을 붙인다
	PgParticle *pkParticle = g_kParticleMan.GetParticle(pcParticle, PgParticle::O_SCALE,fScale);
	if (!pkParticle)
	{
		return false;
	}

	PgActor* pkNpc = g_kPilotMan.FindNpcActor(UNI(pcNpcActorName));
	if(pkNpc)
	{
		NiPoint3 kTempPos(kPos.GetX(), kPos.GetY(), kPos.GetZ());
		kTempPos+=pkNpc->GetPos();
		pkNpc->AttachToPoint(iSlot, kTempPos, pkParticle);
		return true;
	}
	return false;
}

void lwDrawIconToItemNo(lwUIWnd kWnd, int iItemNo, float fScale, lwPoint2F kScaleCenter)
{
	XUI::CXUI_Wnd *pkWnd = kWnd.GetSelf();
	if(!pkWnd)
	{
		return;
	}
	pkWnd->ScaleCenter(kScaleCenter());
	PgUIUtil::DrawIconToItemNo(pkWnd, iItemNo, fScale);
}

void lwDelRenderModelNIF(char const* szRenderModelName, char const* szNIFName)
{
	PgUIModel *pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkModel)
	{
		pkModel->DeleteNIF(szNIFName);
	}
}

void CallRepairItemUseUI(SItemPos const& kItemPos, PgBase_Item const& kItem)
{
	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Call(L"SFRM_REPAIR_USE_ITEM");
	if( !pkMainUI )
	{
		return;
	}
	
	XUI::CXUI_Icon* pkMaterial = dynamic_cast<XUI::CXUI_Icon*>(pkMainUI->GetControl(L"ICN_MATERIAL"));
	if( pkMaterial )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( pDef )
		{
			BM::Stream kPacket;
			kPacket.Push(pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1));
			kPacket.Push(pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2));
			pkMainUI->SetCustomData(&kPacket.Data().at(0), kPacket.Data().size());

			pkMaterial->SetCustomData(&kItem.ItemNo(), sizeof(kItem.ItemNo()));
			pkMaterial->OwnerGuid(kItem.Guid());
		}
	}
}

void lwOnMouseUpRepairItemUseIcon(lwUIWnd kSelf)
{
	if( !kSelf.IsNil() && kSelf.CursorToIcon() )
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();

		SItemPos	kItemPos;
		pSelf->GetCustomData(&kItemPos, sizeof(kItemPos));

		if( kItemPos == SItemPos::NullData() )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return;
		}

		PgBase_Item	kItem;
		if( S_OK != pkInv->GetItem(kItemPos, kItem) )
		{
			pSelf->ClearCustomData();
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
			return;
		}

		if(!IsCanRepair( kItem.ItemNo() ) )
		{
			pSelf->ClearCustomData();
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1219, true);
			return;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if(!pDef)
		{//아이템이 디비에 없다
			pSelf->ClearCustomData();
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 460034, true);
			return;
		}

		if( kItem.Count() == pDef->GetAbil(AT_DEFAULT_AMOUNT) )
		{
			pSelf->ClearCustomData();
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1220, true);
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		lwPacket kPacket = lwUIWnd(pParent).GetCustomDataAsPacket();

		int const iLvMin = kPacket.PopInt();
		int const iLvMax = kPacket.PopInt();
		int const iLvNow = pDef->GetAbil(AT_LEVELLIMIT);

		if( iLvMin > iLvNow || iLvMax < iLvNow )
		{
			pSelf->ClearCustomData();
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 22027, true);
			return;
		}
	}	
}

void lwClickRepairItemUseButton(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf ){ return; }

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent ){ return; }

	XUI::CXUI_Wnd* pTargetForm = pParent->GetControl(L"ICN_TARGET");
	XUI::CXUI_Wnd* pDestItem = pParent->GetControl(L"ICN_MATERIAL");
	if( !pTargetForm || !pDestItem )
	{
		return;
	}

	SItemPos kTargetPos;
	pTargetForm->GetCustomData(&kTargetPos, sizeof(kTargetPos));
	if(kTargetPos == SItemPos::NullData() )
	{//등록된게 없다
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
		return;
	}

	DWORD kMaterialNo = 0;
	pDestItem->GetCustomData(&kMaterialNo, sizeof(kMaterialNo));
	if(kMaterialNo == 0 )
	{//등록된게 없다
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	SItemPos kMaterialPos;
	if( S_OK != pkInv->GetFirstItem(kMaterialNo, kMaterialPos) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1288, true);
		return;
	}

	PgBase_Item	kMaterialItem;
	if( S_OK != pkInv->GetItem(kMaterialPos, kMaterialItem) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1288, true);
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pMaterialDef = kItemDefMgr.GetDef(kMaterialItem.ItemNo());
	if(!pMaterialDef)
	{//아이템이 디비에 없다
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1288, true);
		return;
	}

	if( UICT_REPAIR_ITEM != pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1289, true);
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_USE_REPAIR_ITEM);
	kPacket.Push(kMaterialPos);
	kPacket.Push(kTargetPos);
	NETWORK_SEND(kPacket);

	kSelf.CloseParent();
}

void lwOnMouseUpEnchantItemUseIcon(lwUIWnd kSelf)
{
	if( !kSelf.IsNil() && kSelf.CursorToIcon() )
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if(!pSelf) { return; }

		SItemPos	kItemPos;
		pSelf->GetCustomData(&kItemPos, sizeof(kItemPos));

		if( kItemPos == SItemPos::NullData() )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return;
		}

		PgBase_Item	kItem;
		if( S_OK != pkInv->GetItem(kItemPos, kItem) )
		{
			pSelf->ClearCustomData();
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2851, true);
			return;
		}

		if( LOCAL_MGR::NC_JAPAN == g_kLocal.ServiceRegion() 
			&& false == CheckEnchantBundle(kItem) )
		{//일본의 경우 캐시 아이템은 인챈트 관련 작업 불가
			pSelf->ClearCustomData();
			lwAddWarnDataTT(59002);
			return ;
		}

		{
			XUI::CXUI_Wnd* pParent = pSelf->Parent();
			if(!pParent) { return; }

			DWORD kMaterialNo = 0;
			pParent->GetCustomData(&kMaterialNo, sizeof(kMaterialNo));
			if(kMaterialNo == 0 )
			{//등록된게 없다
				return;
			}

			int iErrorMsg = 0;
			if(!IsUsableItemForceInchant(kMaterialNo, &kItem, iErrorMsg))
			{
				pSelf->ClearCustomData();
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", iErrorMsg, true);
			}



		}

	}
}

bool IsUsableItemForceInchant(int iMtrlItemNo, PgBase_Item const* pkTargetItem, int& riErrorMsgOut)
{
	riErrorMsgOut = 0;
	if(iMtrlItemNo == 0 || pkTargetItem == NULL)
	{
		return false;
	}
	//업그레이드 아이템
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pMaterialDef = kItemDefMgr.GetDef(iMtrlItemNo);
	if(!pMaterialDef)
	{//아이템이 디비에 없다
		return false;
	}

	if(!CheckEnchantBundle(*pkTargetItem))
	{
		riErrorMsgOut = 1248;
		return false;
	}

	//업그레이드 대상 아이템
	CItemDef const *pTargetItemDef = kItemDefMgr.GetDef(pkTargetItem->ItemNo());
	if(!pTargetItemDef) { return false; } //대상 아이템이 없다

	if(pMaterialDef->GetAbil(AT_EQUIP_LIMIT))
	{ //대상아이템 종류 제한 체크
		if( !(pMaterialDef->GetAbil(AT_EQUIP_LIMIT) & pTargetItemDef->GetAbil(AT_EQUIP_LIMIT) ) )
		{
			riErrorMsgOut = 3200;
			return false;
		}
	}

	int iCustomValue1 = pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
	int iCustomValue2 = pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
	int iCustomValue3 = pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_3);
	int iCustomValue4 = pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_4);

	int iRequireLvMin = iCustomValue3;
	int iRequireLvMax = (iCustomValue4 == 0) ? (iCustomValue1 - 1) : std::max(iCustomValue3, iCustomValue4);

	if( pkTargetItem->EnchantInfo().PlusLv() < iRequireLvMin ||
		pkTargetItem->EnchantInfo().PlusLv() > iRequireLvMax )
	{ //대상아이템 레벨 제한 체크
		riErrorMsgOut = 3201;
		return false;
	}

	return true;
}

void lwClickEnchantItemUseButton(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf ){ return; }

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent ){ return; }

	DWORD kMaterialNo = 0;
	pParent->GetCustomData(&kMaterialNo, sizeof(kMaterialNo));
	if(kMaterialNo == 0 )
	{//등록된게 없다
		return;
	}

	XUI::CXUI_Wnd* pTargetForm = pParent->GetControl(L"ICN_TARGET");
	if( !pTargetForm ){ return; }

	SItemPos kTargetPos;
	pTargetForm->GetCustomData(&kTargetPos, sizeof(kTargetPos));

	if(kTargetPos == SItemPos::NullData() )
	{//등록된게 없다
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403076, true);
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	PgInventory* pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	SItemPos kMaterialPos;
	if( S_OK != pkInv->GetFirstItem(kMaterialNo, kMaterialPos) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1290, true);
		return;
	}

	PgBase_Item	kMaterialItem;
	if( S_OK != pkInv->GetItem(kMaterialPos, kMaterialItem) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1290, true);
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pMaterialDef = kItemDefMgr.GetDef(kMaterialItem.ItemNo());
	if(!pMaterialDef)
	{//아이템이 디비에 없다
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1290, true);
		return;
	}

	if( UICT_FORCED_ENCHANT != pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 1291, true);
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_USE_ENCHANT_ITEM);
	kPacket.Push(kMaterialPos);
	kPacket.Push(kTargetPos);
	NETWORK_SEND(kPacket);

	kSelf.CloseParent();
}

void lwMakeForceInchantUIText(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pkSelf = kSelf.GetSelf();
	if(!pkSelf) { return; }

	std::wstring wstrText = TTW(1284);

	DWORD dwMaterialNo = 0;
	pkSelf->GetCustomData(&dwMaterialNo, sizeof(dwMaterialNo));
	if(dwMaterialNo > 0 )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pMaterialDef = kItemDefMgr.GetDef(dwMaterialNo);
		if(pMaterialDef && pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) == UICT_FORCED_ENCHANT)
		{
			int iCustomValue1 = pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			int iCustomValue2 = pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
			int iCustomValue3 = pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_3);
			int iCustomValue4 = pMaterialDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_4);

			int iRequireLvMin = iCustomValue3;
			int iRequireLvMax = (iCustomValue4 == 0) ? (iCustomValue1 - 1) : std::max(iCustomValue3, iCustomValue4);
			int iResultLvMin = iCustomValue1;
			int iResultLvMax = std::max(iCustomValue1, iCustomValue2);


			wstrText += L"\n";
			wstrText += L"{C=0xFF0046FF/}";
			wstrText += TTW(40904);
			int const iEquipPos = pMaterialDef->GetAbil(AT_EQUIP_LIMIT);
			std::wstring wstrEquipType;
			GetEqiupPosStringMonsterCard(iEquipPos, wstrEquipType);
			if(!wstrEquipType.empty())
			{
				wstrText += wstrEquipType;
				wstrText += L" ";
			}

			wchar_t szTemp[MAX_PATH];
			if(iRequireLvMin == iRequireLvMax)
			{
				swprintf_s(szTemp, MAX_PATH, L"+%d", iRequireLvMin); //+xx
			}
			else
			{
				swprintf_s(szTemp, MAX_PATH, L"+%d ~ +%d", iRequireLvMin, iRequireLvMax); //+xx ~ +xx
			}
			wstrText += szTemp;

			wstrText += L"\n";
			wstrText += TTW(40905);
			if(iResultLvMin == iResultLvMax)
			{
				swprintf_s(szTemp, MAX_PATH, L"+%d", iResultLvMin); //+xx
			}
			else
			{
				swprintf_s(szTemp, MAX_PATH, L"+%d ~ +%d", iResultLvMin, iResultLvMax); //+xx ~ +xx
			}
			wstrText += szTemp;
		}

	}

	XUI::CXUI_Wnd* pkBgWnd = pkSelf->GetControl(L"SFRM_BG");
	if(pkBgWnd)
	{
		XUI::CXUI_Wnd* pkTextWnd = pkBgWnd->GetControl(L"FRM_HELP_TEXT");
		if(pkTextWnd)
		{
			pkTextWnd->Text(wstrText);
		}
	}
}

void lwOnCallCenterMiniMap(lwGUID kGuid, int const iAliveTime)
{
	if(!g_pkWorld)
	{
		return;
	}
	T_GNDATTR const kGndAttr = g_pkWorld->GetAttr();
	if( (GATTR_FLAG_MISSION&kGndAttr) && true == lua_tinker::call<bool>("IsDefenceMode") )
	{//디펜스 모드에서는 미니맵 사용할수 없음.
		::Notice_Show( TTW(98), EL_Warning );
		return;
	}

	//if(true == lwCallDungeonMinimap())
	if(true == g_kProgressMap.CallUI())
	{ //던전용 미니맵
		return;
		//::Notice_Show( TTW(665), EL_Warning );
	}
	else if(GATTR_CONSTELLATION_BOSS & kGndAttr)
	{
		if(g_pkWorld->IsExistMiniMapData() == false)
		{
			::Notice_Show( TTW(665), EL_Warning );
			return;
		}
	}

	if(GATTR_BOSS == kGndAttr) 
	{//던전 미니맵 정보가 없는 보스방에서는 미니맵 사용할수 없음.
		::Notice_Show( TTW(665), EL_Warning );
		return;
	}
	if(GATTR_ELEMENT_GROUND_BOSS == kGndAttr)
	{//정령던전 보스방 입장시 미니맵 사용할수 없음.
		::Notice_Show( TTW(665), EL_Warning );
		return;
	}
	
	if(NULL == g_pkWorld->MapNo())
	{
		return;
	}
	if(9010801 == g_pkWorld->MapNo())
	{//망자의 탑은 미니맵 대신 층별 보스클리어 정보를 출력한다.미니맵을 막아야 한다.
		::Notice_Show( TTW(665), EL_Warning );
		return;
	}

	XUI::CXUI_Wnd* pkMiniMap = XUIMgr.Get(L"SFRM_BIG_MAP");
	if( !pkMiniMap || pkMiniMap->IsClosed() )
	{
		pkMiniMap = XUIMgr.Call(L"SFRM_BIG_MAP");
	}
	else if( kGuid.IsNil() )
	{
		pkMiniMap->Close();
		return;
	}

	bool const bIsEmpriaBattle = g_pkWorld->IsHaveAttr(GATTR_FLAG_EMPORIABATTLE);
	bool bIsPvPTypeKTH = false;
	bool bIsPvPTypeLove = false;
	PgPvPGame *pkGame = dynamic_cast<PgPvPGame*>(PgContentsBase::ms_pkContents);
	if ( pkGame )
	{
		if( pkGame->IsKTHMode() )
		{
			bIsPvPTypeKTH = true;
		}
		if( pkGame->IsLoveMode() )
		{
			bIsPvPTypeLove = true;
		}
	}

	if( pkMiniMap )
	{
		pkMiniMap->IsAliveTimeIgnoreMouseOver(true);
		pkMiniMap->AliveTime(iAliveTime);
		bool const bTeleMoveItemUse = !kGuid.IsNil();
		PgAlwaysMiniMap::MapTeleMoveUseItem(bTeleMoveItemUse);
		pkMiniMap->OwnerGuid(kGuid.GetGUID());

		bool bVisibleTab = true;
		bool bVisibleClick = true;
		//지역, PVP 타입에 따라 미니맵 보이고 안보이고 설정 =시작=
		if( bIsEmpriaBattle
			|| bIsPvPTypeKTH
			|| bIsPvPTypeLove )
		{
			bVisibleTab = false;
			bVisibleClick = false;
		}
		//지역, PVP 타입에 따라 미니맵 보이고 안보이고 설정 =끝=

		XUI::CXUI_Wnd* pkTabDescription = pkMiniMap->GetControl(L"FRM_TAB_DESCRIPTION");
		XUI::CXUI_Wnd* pkDescription = pkMiniMap->GetControl(L"FRM_CLICK_DESCRIPTION");
		XUI::CXUI_Wnd* pkCloseBt = pkMiniMap->GetControl(L"BTN_CLOSE");

		if( bIsEmpriaBattle )
		{
			pkDescription ? pkDescription->Visible(true) : 0;
			pkCloseBt ? pkCloseBt->Visible(false) : 0;
		}
		else
		{
			pkDescription ? pkDescription->Visible(bVisibleClick) : 0;
			if( bIsPvPTypeKTH
				|| bIsPvPTypeLove )
			{// 점령전, 러브러브전이면 클로즈 버튼 숨김
				pkCloseBt ? pkCloseBt->Visible(false) : 0;
			}
			else
			{
				pkCloseBt ? pkCloseBt->Visible(true) : 0;
			}
		}
		pkTabDescription ? pkTabDescription->Visible(bVisibleTab) : 0;

		if( pkDescription )
		{
			if( bIsEmpriaBattle )
			{
				PgPlayer*	pPlayer = g_kPilotMan.GetPlayerUnit();
				if( pPlayer )
				{
					bool bIsCoreBattle = WAR_TYPE_DESTROYCORE == PgContentsBase::ms_pkContents->GetType();
					if( TEAM_ATTACKER == pPlayer->GetAbil(AT_TEAM) )
					{
						BM::vstring vStr(bIsCoreBattle?TTW(76020):L"");
						if( bIsCoreBattle )
						{
							vStr.Replace(L"#SYSEMOTICON#", g_kEmoFontMgr.Trans_Key_SysFontString("EMPORIA_CORE_BLUE"));
						}
						pkDescription->Text(vStr);
					}
					else
					{
						BM::vstring vStr(bIsCoreBattle?TTW(76021):L"");
						if( bIsCoreBattle )
						{
							vStr.Replace(L"#SYSEMOTICON#", g_kEmoFontMgr.Trans_Key_SysFontString("EMPORIA_CORE_RED"));
						}
						pkDescription->Text(vStr);
					}
				}
			}
			else
			{
				if( bTeleMoveItemUse )
				{
					pkDescription->Text(TTW(1940));
				}
				else
				{
					pkDescription->Text(TTW(650));
				}
			}
			
			XUI::CXUI_Style_String kStyleString = pkDescription->StyleText();
			POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
			pkDescription->Location(POINT2(pkDescription->Location().x, pkMiniMap->Size().y - kTextSize.y));
		}
	}
}

void lwOnCloseCenterMiniMap(lwUIWnd UISelf)
{
	if( !UISelf.IsNil() )
	{
		UISelf.ClearCustomData();
	}
	//배틀스퀘어에서 올라간 맵을 원상태로 되돌림
	lwPoint2 MapPos = UISelf.GetLocation();
	MapPos.SetY(48);
	UISelf.SetLocation(MapPos);
	PgAlwaysMiniMap::MapTeleMoveUseItem(false);
}

void lwOnCallBattleSquareMiniMap()
{
	XUI::CXUI_Wnd* pkMiniMap = XUIMgr.Get(L"SFRM_BIG_MAP");
	if( !pkMiniMap || pkMiniMap->IsClosed() )
	{
		pkMiniMap = XUIMgr.Call(L"SFRM_BIG_MAP");
	}

	if( pkMiniMap )
	{
		POINT2 MapPos = pkMiniMap->Location();
		MapPos.y = 0;
		pkMiniMap->Location(POINT2(MapPos.x, MapPos.y));

		pkMiniMap->IsAliveTimeIgnoreMouseOver(true);
		pkMiniMap->AliveTime(0);
		PgAlwaysMiniMap::MapTeleMoveUseItem(false);
		
		XUI::CXUI_Wnd* pkTabDescription = pkMiniMap->GetControl(L"FRM_TAB_DESCRIPTION");
		if( pkTabDescription )
		{
			pkTabDescription->Visible(false);
		}
		pkMiniMap->OwnerGuid(BM::GUID(L""));
		XUI::CXUI_Wnd* pkDescription = pkMiniMap->GetControl(L"FRM_CLICK_DESCRIPTION");
		if( pkDescription )
		{
			pkDescription->Text(TTW(799230));
			
			XUI::CXUI_Style_String kStyleString = pkDescription->StyleText();
			POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
			pkDescription->Location(POINT2(pkDescription->Location().x, pkMiniMap->Size().y - kTextSize.y));
		}
	}
	//배틀스퀘어 정보
	XUI::CXUI_Wnd* pkInfo = XUIMgr.Get(L"FRM_BS_INFO");
	if( !pkInfo || pkInfo->IsClosed() )
	{
		pkInfo = XUIMgr.Call(L"FRM_BS_INFO");
	}
	if(pkInfo)
	{
		const int PosY =  pkMiniMap->Location().y + pkMiniMap->Size().y+1;
		pkInfo->Location(POINT2(pkInfo->Location().x, PosY));
	}
	//내 포인트
	XUI::CXUI_Wnd* pkMyPoint = XUIMgr.Get(L"FRM_BS_MY_POINT");
	if( !pkMyPoint || pkMyPoint->IsClosed() )
	{
		pkMyPoint = XUIMgr.Call(L"FRM_BS_MY_POINT");
	}
	if( pkMyPoint )
	{
		const int PosY =  pkInfo->Location().y + pkInfo->Size().y-1;
		pkMyPoint->Location(POINT2(pkMyPoint->Location().x, PosY));
	}
}
		
void OnClickMiniMapTeleMoveToTarget(POINT3 const& kPos)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		PgInventory* pkInv = pkPlayer->GetInven();
		if( pkInv )
		{
			XUI::CXUI_Wnd* pkMiniMapUI = XUIMgr.Get(L"SFRM_BIG_MAP");
			if( pkMiniMapUI )
			{
				BM::GUID const& kItemGuid = pkMiniMapUI->OwnerGuid();
				PgBase_Item kItem;
				SItemPos	kItemPos;
				if( kItemGuid.IsNotNull() && S_OK == pkInv->GetItem(kItemGuid, kItem, kItemPos) )
				{
					BM::Stream kPacket(PT_C_M_REQ_USE_TELEPORT_ITEM);
					kPacket.Push(kItemPos);
					kPacket.Push(lwGetServerElapsedTime32());
					kPacket.Push(POINT3(kPos.x, kPos.y, kPos.z));
					NETWORK_SEND(kPacket);
					pkMiniMapUI->Close();
				}
			}
		}
	}
}

void lwUpdateCheckDrawLimitBreakBar()
{
	XUI::CXUI_Wnd* pTemp = XUIMgr.Get(L"FRM_CASH_SHOP");
	if( NULL != pTemp )
	{
		return;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	if( 0 == pkPlayer->GetAbil(AT_ENABLE_AWAKE_SKILL) )
	{//각성기 활성화 되지 않음.
		return;
	}

	XUI::CXUI_Wnd* pkGauge = XUIMgr.Get(L"FRM_SKILL_AWAKE_BAR");
	if( !pkGauge )
	{
		pkGauge = XUIMgr.Call(L"FRM_SKILL_AWAKE_BAR");
		if( !pkGauge )
		{
			return;
		}
	}

	if( PgMissionComplete::E_MSN_CPL_NONE != g_kMissionComplete.CompleteState()
		|| PgBossComplete::E_MSN_CPL_NONE != g_kBossComplete.CompleteState()
		|| PgExpeditionComplete::E_MSN_CPL_NONE != g_kExpeditionComplete.CompleteState())
	{
		if( pkGauge ){ pkGauge->Close(); }
		return;
	}

	int const NowAwakeValue = pkPlayer->GetAbil(AT_AWAKE_VALUE);
	int GaugeState = pkPlayer->GetAbil(AT_AWAKE_STATE);

	if( 0 == NowAwakeValue )
	{
		pkGauge->Close();
		return;
	}
	
	if(NowAwakeValue >= AWAKE_VALUE_MAX
		&& EAS_NORMAL_GUAGE == GaugeState
		)
	{// 서버에서 올때까지 기다리면 느리므로 클라에서 먼저 세팅해준다
		pkPlayer->SetAbil(AT_AWAKE_STATE, EAS_MAX_GUAGE);
		GaugeState = EAS_MAX_GUAGE;
		PgActorUtil::ExpressAwakeMaxState(g_kPilotMan.GetPlayerActor());
	}

	XUI::CXUI_AniBar* pkAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pkGauge->GetControl(L"ANIBAR_SKILLPOINT"));
	XUI::CXUI_Wnd* pkBurn = pkGauge->GetControl(L"FRM_BURN_IMG");
	XUI::CXUI_Wnd* pkMaxBorder = pkGauge->GetControl(L"FRM_BAR_MAX_BD");
	XUI::CXUI_Wnd* pkMax = pkGauge->GetControl(L"FRM_BAR_MAX");
	if( pkAniBar && pkBurn && pkMaxBorder && pkMax )
	{
		pkAniBar->Max(AWAKE_VALUE_MAX);
		if( EAS_PENALTY_GUAGE == GaugeState )
		{
			if( !pkBurn->Visible() )
			{
				pkBurn->Visible(true);
			}
		}
		else
		{
			pkBurn->Visible(false);
		}

		switch( GaugeState )
		{
		case EAS_NORMAL_GUAGE:// 노말
			{
				bool const bAwakeChargeUI = lua_tinker::call<bool>("GetAwakeChargeState");
				if(bAwakeChargeUI)
				{
					pkAniBar->UVUpdate((1 == pkAniBar->UVInfo().Index)?(2):(1));
				}
				else
				{
					pkAniBar->UVUpdate(1);
				}
				
				pkMax->Visible(false);
			}break;
		case EAS_MAX_GUAGE:// 만땅
			{
				pkAniBar->UVUpdate(2);
				pkMax->Visible(true);
			}break;
		case EAS_PENALTY_GUAGE:// 소모
			{
				POINT2 kPos;
				kPos.x = pkAniBar->Location().x + pkAniBar->Size().x * (pkAniBar->Now() / static_cast<float>(pkAniBar->Max())) - (pkBurn->Size().x * 0.5f);
				kPos.y = pkBurn->Location().y;
				pkBurn->Location(kPos);
				pkAniBar->UVUpdate((1 == pkAniBar->UVInfo().Index)?(2):(1));
				pkMax->Visible(false);
			}break;
		default:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790413, true);
				pkGauge->Close();
			}return;
		}
		pkMaxBorder->Visible(pkMax->Visible());
		pkAniBar->Now(NowAwakeValue);
	}
}

void lwMoveVCenterUI_TextBase(lwUIWnd UIParent, lwUIWnd UISelf, int iX)
{
	if( UIParent.IsNil() || UISelf.IsNil() )
	{
		return;
	}

	lwPoint2 kParentSize = UIParent.GetSize();
	lwPoint2 kSelfTextSize = UISelf.GetTextSize();

	int iCenterX1 = kParentSize.GetX() / 2;
	int iCenterX2 = (kSelfTextSize.GetX() - iX) / 2;
	lwPoint2 kTempSize(iCenterX1-iCenterX2, UISelf.GetLocation().GetY());

	if(kTempSize.GetX() > 0)
	{	
		UISelf.SetLocation(kTempSize);
		UISelf.SetInvalidate();
	}
}

extern PgStore g_kViewStore;//지금 보고있는 
extern int g_iShopViewGrp;
int lwGetShopGoodsViewSize()
{
	return g_kViewStore.GetGoodsViewSize();
}

bool lwIsMapFillRestExp(int const iMapNo)
{
	if(0 < iMapNo)
	{
		GET_DEF(PgDefMapMgr, kDefMap);

		// 휴식 경험치를 주는 간격(초단위)
		int const iInterBnsRestExp = kDefMap.GetAbil(iMapNo, AT_BONUS_EXP_INTERVAL_INVILLAGE);

		// 얻게되는 휴식 경험치 (만분율값)
		int const iEarnRestExpRate = kDefMap.GetAbil(iMapNo, AT_BONUS_EXP_RATE_INVILLAGE);
		if(iEarnRestExpRate
			&& iInterBnsRestExp
			)
		{
			return true;
		}
	}
	return false;
}
bool lwShowRestExpBar()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}
	__int64 const i64Exp = pkPlayer->GetAbil64(AT_EXPERIENCE);
	__int64 const i64RestExp = pkPlayer->GetAbil64(AT_REST_EXP_ADD_MAX)/*+i64Exp*/;
	bool const bOn = (i64Exp < i64RestExp);
	lua_tinker::call<void, bool>("SetRestXpMode", bOn);
	return bOn;
}

lwWString lwGetRestExpToString()
{	
	lwWString kStr("0");
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(pkPlayer)
	{
		__int64 const i64Exp = pkPlayer->GetAbil64(AT_EXPERIENCE);
		__int64 const i64RestExp = pkPlayer->GetAbil64(AT_REST_EXP_ADD_MAX);
		__int64 const i64PureRestExp = i64RestExp-i64Exp;
		if(0 < i64PureRestExp)
		{
			__int64 i64Max = 0;
			int const iLv = pkPlayer->GetAbil(AT_LEVEL);
			int const iClass = pkPlayer->GetAbil(AT_CLASS);
			GET_DEF(PgClassDefMgr, kClassDefMgr);
			__int64 const i64Begin = kClassDefMgr.GetAbil64(SClassKey(iClass, iLv), AT_EXPERIENCE);
			__int64 const i64NextEXP = kClassDefMgr.GetAbil64(SClassKey(iClass, iLv+1), AT_EXPERIENCE);
			if(0 < i64NextEXP)
			{
				i64Max = std::max(static_cast<__int64>(0), (i64NextEXP - i64Begin));
			}
			else
			{
				i64Max = i64NextEXP;
			}
			float const fRate = ((i64Max)?(static_cast<float>(i64PureRestExp)/static_cast<float>(i64Max)):0);
			wchar_t szBuf[MAX_PATH] = {0, };
			_stprintf_s(szBuf, MAX_PATH, _T("%.2f"), fRate*100.0f);
			kStr = std::wstring(szBuf);
		}
	}
	return kStr;
}

void lwNotice_Show(lwWString klwStr, int const iLevel, bool const bCheckEqualMessage)
{
	::Notice_Show(klwStr(), iLevel, bCheckEqualMessage);
}

void lwUseHiddenItem(lwUIWnd lwFrm)
{
	XUI::CXUI_Form *pkForm = (XUI::CXUI_Form*)lwFrm();
	if( !pkForm )
	{
	}
	

	XUI::CXUI_Form* pkMsgWnd = (XUI::CXUI_Form*)pkForm->GetControl(std::wstring(_T("FRM_MESSAGE")));
	if( !pkMsgWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pkBtnWnd = pkForm->GetControl(std::wstring(_T("BTN_OK")));
	if( !pkBtnWnd )
	{
		return;
	}

	int iItemNo = 0;
	SItemPos ItemInvPos;

	pkMsgWnd->GetCustomData(&iItemNo, sizeof(iItemNo));
	pkBtnWnd->GetCustomData(&ItemInvPos, sizeof(ItemInvPos));

	if( !iItemNo )
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(iItemNo);
	if( !pItemDef )
	{
		return;
	}

	int const iType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	int const iCurLevel = pkPlayer->GetAbil(AT_LEVEL);
	int const iMinLv = pItemDef->GetAbil(AT_LEVELLIMIT);
	int const iMaxLv = pItemDef->GetAbil(AT_MAX_LEVELLIMIT);
	if( ((iMinLv > 0) && (iCurLevel < iMinLv))
	 || ((iMaxLv > 0) && (iCurLevel > iMaxLv)) )
	{
		lwAddWarnDataTT(22027);
		return;
	}

	PgPilot *pkPilot =g_kPilotMan.GetPlayerPilot();
	if (pkPilot)
	{
		PgActor	*pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
		if(pkActor)
		{
			PgAction *pkAction = pkActor->GetAction();
			if( pkAction )
			{
				lua_tinker::call<void, char const*, lwActor, BYTE, BYTE>("World_ScrollPortal_Action","Hidden" , lwActor(pkActor), ItemInvPos.x, ItemInvPos.y);
			}					
		}
	}
}


void TimeToString(DWORD const dwTimeSec, std::wstring & kOutText )
{
	int iSec  = dwTimeSec;
	int iMin  = iSec  / 60;
	int iHour = iMin  / 60;
	int iDay  = iHour / 24;
	iSec  = iSec % 60;
	iMin  = iMin % 60;
	iHour = iHour % 24;

	wchar_t szTemp[MAX_PATH] = {0,};
	if(iDay > 0)
	{
		swprintf_s(szTemp, MAX_PATH, L"%d%s ", iDay, TTW(174).c_str());
	}

	if(iHour > 0)
	{
		int iLen = wcslen(szTemp);
		swprintf_s(szTemp+iLen, MAX_PATH-iLen, L"%d%s ", iHour, TTW(175).c_str());
	}

	if(iMin > 0)
	{
		int iLen = wcslen(szTemp);
		swprintf_s(szTemp+iLen, MAX_PATH-iLen, L"%d%s ", iMin, TTW(176).c_str());
	}

	if(iSec > 0)
	{
		int iLen = wcslen(szTemp);
		swprintf_s(szTemp+iLen, MAX_PATH-iLen, L"%d%s ", iSec, TTW(177).c_str());
	}

	kOutText = szTemp;
}

bool lwIsQuickSkillSlotActive(int const iIconKey)
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}

	SQuickInvInfo kQuickInvInfo;
	PgQuickInventory* pkQInv = pkPlayer->GetQInven();
	HRESULT const hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(iIconKey), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다
	int const iItemNo = kQuickInvInfo.ID();
	if(iItemNo)
	{
		return true;
	}
	return false;
}

void UIGemStore::lwSendReqDefGemStore(lwGUID lwkGuid, int const iMenu, bool const bCheckClass)
{// 보석상점 물품 리스트 요청
	g_kGemStore.SendReqDefGemStoreInfo(lwkGuid.GetGUID(), iMenu, bCheckClass);
}

bool UIGemStore::lwIsNotEmptyDefGemStore(lwGUID lwkGuid, int const iMenu)
{// 보석상점 물품 리스트 요청
	CONT_DEFGEMSTORE const * pkDef = NULL;
	g_kTblDataMgr.GetContDef(pkDef);
	if(NULL != pkDef)
	{
		BM::GUID const& kNpcGuid = lwkGuid.GetGUID();
		CONT_DEFGEMSTORE::const_iterator iter = pkDef->find(kNpcGuid);
		if(iter != pkDef->end())
		{
			CONT_DEFGEMSTORE::mapped_type const& kStore = (*iter).second;
			CONT_DEFGEMSTORE_ARTICLE::const_iterator c_it = kStore.kContArticles.begin();
			while(c_it!=kStore.kContArticles.end())
			{
				if((*c_it).first.iMenu==iMenu)
				{
					return true;
				}
				++c_it;
			}
		}
	}

	return false;
}

bool lwIsPlayerPlayTime()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( pkPlayer )
	{
		return pkPlayer->IsUse();
	}
	return false;
}

//////////직업 기술
void GetJobSkillText_ToolType(int const iItemNo, BM::vstring& kOutput)//도구 타입별 텍스트
{
	EJobSkillToolType const eToolType = static_cast<EJobSkillToolType>(JobSkillToolUtil::GetToolType(iItemNo));
	switch(eToolType)
	{//도구 타입별 문구(도끼, 곡괭이 ...)
	case JSTT_AXE:
	case JSTT_PICKAX:
	case JSTT_HOE:
	case JSTT_ROD:
	case JSTT_ALL:
		{
			kOutput = TTW(799665);
		}break;
	case JSTT_CHAINSOW:
	case JSTT_DRILL:
	case JSTT_CUTTER:
	case JSTT_NET:
	case JSTT_ALL_SUB:
		{
			kOutput = TTW(799666);
		}break;
	}
}
bool GetJobSkillText_ToolType(int const iGatherType, int const iSkillNo, BM::vstring& rkTextOut)//도구 타입별 텍스트
{
	bool bResult = false;
	EJobSkillType const eJobSkillType = JobSkill_Util::GetJobSkillType(iSkillNo);
	switch(eJobSkillType)
	{//도구 타입별 문구(도끼, 곡괭이 ...)
	case JST_1ST_MAIN:
		{
			rkTextOut = TTW(799665);
			bResult = true;
		}break;
	case JST_1ST_SUB:
		{
			rkTextOut = TTW(799666);
			bResult = true;
		}break;
	}
	return bResult;
}

void GetJobSkillText_GatherType(int const iSkillNo, BM::vstring& kOutput)//획득 타입별 텍스트
{
	CONT_DEF_JOBSKILL_SKILL const* pkContDefJobSkill;
	g_kTblDataMgr.GetContDef(pkContDefJobSkill);
	if(!pkContDefJobSkill)
	{
		return;
	}
	CONT_DEF_JOBSKILL_SKILL::const_iterator iter_Skill = pkContDefJobSkill->find(iSkillNo);
	if( iter_Skill == pkContDefJobSkill->end() )
	{
		return;
	}
	CONT_DEF_JOBSKILL_SKILL::mapped_type kSkill = (*iter_Skill).second;
	switch(kSkill.eJobSkill_Type)
	{//기술 타입별 문구(1차 : [채집기술 ] / 2차 : [가공 기술] / 3차 : [생산 기술]
	case JST_1ST_MAIN:
	case JST_1ST_SUB:
		{
			kOutput = TTW(799502);
		}break;
	case JST_2ND_MAIN:
	case JST_2ND_SUB:
		{
			kOutput = TTW(799503);
		}break;
	case JST_3RD_MAIN:
		{
			kOutput = TTW(799504);
		}break;
	}
}

void GetJobSkillText_MaterialType(int const iItemNo, BM::vstring& kOutput)//가구장치 타입별 텍스트
{
	eJobSkillMaterialType const eToolType =JobSkill_Util::GetJobSkillMaterialType(iItemNo);
	switch(eToolType)
	{
	case JSMRT_WOOD:
		{//목재
			kOutput = TTW(799670);
		}break;
	case JSMRT_METAL:
		{//금속
			kOutput = TTW(799671);
		}break;
	case JSMRT_JEWEL:
		{//보석
			kOutput = TTW(799672);
		}break;
	case JSMRT_HERB:
		{//약초
			kOutput = TTW(799673);
		}break;
	case JSMRT_VEGETABLE:
		{//채소
			kOutput = TTW(799674);
		}break;
	case JSMRT_FISH:
		{//생선
			kOutput = TTW(799675);
		}break;
	}
}

void NoticeAddExpertnessMsg(PgPlayer const & rkPlayer, int const iSkillNo, int const iAddExpertness)//숙련도 오름 메세지
{
	if( !JobSkillUtil::IsJobSkill(iSkillNo) )
	{
		return;
	}
	int const iCurExpertness = rkPlayer.JobSkillExpertness().Get(iSkillNo);
	if( 0 < iCurExpertness
		&& iAddExpertness > iCurExpertness)
	{
		int iPoint = (iAddExpertness/10000) - (iCurExpertness/10000);
		if( 1 <= iPoint)
		{
			BM::vstring kNotice(TTW(25021));
			BM::vstring kGatherType;
			GetJobSkillText_GatherType(iSkillNo, kGatherType);
			wchar_t const *pSkillName = NULL;
			if(::GetDefString(iSkillNo, pSkillName) )
			{
				kNotice.Replace(L"#SKILLTYPE#", kGatherType);
				kNotice.Replace(L"#SKILLNAME#", BM::vstring(pSkillName));
				kNotice.Replace(L"#EXP#", iPoint);
				::Notice_Show(static_cast<std::wstring>(kNotice), EL_Normal, true);
			}
		}
	}
	lwJobSkill_NfySaveIdx::ClearContSaveIdx();
	if( lwJobSkill_NfySaveIdx::CheckNewSaveIdx( iSkillNo, iCurExpertness, iAddExpertness ) )
	{
		lwJobSkill_NfySaveIdx::lwUpdateUI();
	}
}

void CallJobSkillGatherTimer(int const iSkillNo, DWORD const dwRunTurnTime)
{
	if( 0 == iSkillNo 
		|| 0 == dwRunTurnTime)
	{
		return;
	}
	if( !JobSkillUtil::IsJobSkill(iSkillNo) )
	{
		return;
	}

	PgPlayer *pPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pPlayer)
	{
		return;
	}
	
	PgJobSkillExpertness const &rkExpertness = pPlayer->JobSkillExpertness();
	int const iExpertness = rkExpertness.Get(iSkillNo);
	int const iMaxExhaustion = JobSkillExpertnessUtil::GetMaxExhaustion_1ST(pPlayer->GetPremium(), rkExpertness.GetAllSkillExpertness());
	int const iCurExhaustion = rkExpertness.CurExhaustion();

	lua_tinker::call<void, int, int, int, int>("CallJobSkillGatherTimer", dwRunTurnTime/1000, iExpertness/10000, iCurExhaustion, iMaxExhaustion);
}

void lwTogleOffAllJobSkillQuicIcon()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	SQuickInvInfo kQuickInvInfo;
	PgQuickInventory* pkQInv = pkPlayer->GetQInven();
	if (!pkQInv)
	{
		return;
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);	
	{
		CXUI_Wnd* pkQuickInvEx = XUIMgr.Get(L"QuickInvEx");
		if(pkQuickInvEx)
		{
			XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkQuickInvEx->GetControl(L"BLD_QUICK"));
			if(pkBuilder)
			{
				int const iMax = pkBuilder->CountX()*pkBuilder->CountY();

				for(int i=0; i < iMax; i++)
				{
					BM::vstring vSlotName(L"QUICK");
					vSlotName+=i;
					
					CXUI_Icon*  pkQuickSlot = dynamic_cast<CXUI_Icon*>(pkQuickInvEx->GetControl(vSlotName));
					if(pkQuickSlot)
					{
						SIconInfo const& kIconInfo = pkQuickSlot->IconInfo();

						HRESULT const  hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(kIconInfo.iIconKey), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다
						if(S_OK == hRet)
						{
							CSkillDef const* pkSkillDef =kSkillDefMgr.GetDef(kQuickInvInfo.ID());
							if(pkSkillDef
								&&
								0 < pkSkillDef->GetAbil(AT_JOBSKILL_TOOL_TYPE)
								)
							{
								CXUI_Wnd* pkStroke =  pkQuickSlot->GetControl(L"FRM_TOGGLE_IMG");
								if(pkStroke)
								{
									pkStroke->Visible(false);
								}
							}
						}
					}
				}
			}
		}
	}
	
	{
		CXUI_Wnd* pkQuickInv =XUIMgr.Get(L"QuickInv");
		if(pkQuickInv)
		{
			CXUI_Wnd* pkBg = pkQuickInv->GetControl(L"FRM_BG");
			if(pkBg)
			{
				CXUI_Wnd* pkIcon = pkBg->GetControl(L"FRM_ICON");
				if(pkIcon)
				{
					XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkIcon->GetControl(L"BLD_QUICK"));
					if(pkBuilder)
					{
						int const iMax = pkBuilder->CountX()*pkBuilder->CountY();
						for(int i=0; i < iMax; i++)
						{
							BM::vstring vSlotName(L"QUICK");
							vSlotName+=i;
							CXUI_Icon* pkQuickSlot = dynamic_cast<CXUI_Icon*>(pkIcon->GetControl(vSlotName));
							if(pkQuickSlot)
							{
								SIconInfo const& kIconInfo = pkQuickSlot->IconInfo();
								HRESULT const  hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(kIconInfo.iIconKey), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다
								if(S_OK == hRet)
								{
									CSkillDef const* pkSkillDef =kSkillDefMgr.GetDef(kQuickInvInfo.ID());
									if(pkSkillDef
										&&	0 < pkSkillDef->GetAbil(AT_JOBSKILL_TOOL_TYPE)
										)
									{
										CXUI_Wnd* pkStroke =  pkQuickSlot->GetControl(L"FRM_TOGGLE_IMG");
										if(pkStroke)
										{
											pkStroke->Visible(false);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void lwSetJobSkillTogleState(int const iSkillNo, bool const bToggleOn)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	SQuickInvInfo kQuickInvInfo;
	PgQuickInventory* pkQInv = pkPlayer->GetQInven();
	if (!pkQInv)
	{
		return;
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);	
	{
		CXUI_Wnd* pkQuickInvEx = XUIMgr.Get(L"QuickInvEx");
		if(pkQuickInvEx)
		{
			XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkQuickInvEx->GetControl(L"BLD_QUICK"));
			if(pkBuilder)
			{
				int const iMax = pkBuilder->CountX()*pkBuilder->CountY();

				for(int i=0; i < iMax; i++)
				{
					BM::vstring vSlotName(L"QUICK");
					vSlotName+=i;
					
					CXUI_Icon*  pkQuickSlot = dynamic_cast<CXUI_Icon*>(pkQuickInvEx->GetControl(vSlotName));
					if(pkQuickSlot)
					{
						SIconInfo const& kIconInfo = pkQuickSlot->IconInfo();
						HRESULT const  hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(kIconInfo.iIconKey), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다
						if(S_OK == hRet)
						{	
							if(iSkillNo == kQuickInvInfo.ID())
							{
								CXUI_Wnd* pkStroke =  pkQuickSlot->GetControl(L"FRM_TOGGLE_IMG");
								if(pkStroke)
								{
									pkStroke->Visible(bToggleOn);
								}
							}
						}
					}
				}
			}
		}
	}
	
	{
		CXUI_Wnd* pkQuickInv =XUIMgr.Get(L"QuickInv");
		if(pkQuickInv)
		{
			CXUI_Wnd* pkBg = pkQuickInv->GetControl(L"FRM_BG");
			if(pkBg)
			{
				CXUI_Wnd* pkIcon = pkBg->GetControl(L"FRM_ICON");
				if(pkIcon)
				{
					XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkIcon->GetControl(L"BLD_QUICK"));
					if(pkBuilder)
					{
						int const iMax = pkBuilder->CountX()*pkBuilder->CountY();
						for(int i=0; i < iMax; i++)
						{
							BM::vstring vSlotName(L"QUICK");
							vSlotName+=i;
							CXUI_Icon* pkQuickSlot = dynamic_cast<CXUI_Icon*>(pkIcon->GetControl(vSlotName));
							if(pkQuickSlot)
							{
								SIconInfo const& kIconInfo = pkQuickSlot->IconInfo();
								HRESULT const  hRet = pkQInv->GetItem(pkQInv->AdjustSlotIDX(kIconInfo.iIconKey), kQuickInvInfo);// 퀵인벤에서는 위치만 가져온다
								if(S_OK == hRet)
								{
									
									if(iSkillNo == kQuickInvInfo.ID())
									{
										CXUI_Wnd* pkStroke =  pkQuickSlot->GetControl(L"FRM_TOGGLE_IMG");
										if(pkStroke)
										{
											pkStroke->Visible(bToggleOn);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void lwCallSafeAdditionUI(char const* szID, bool bOnlyChangePos)
{
	if(NULL==szID)		{return;}
	XUI::CXUI_Wnd* pkSafe = NULL;
	std::wstring const kID = UNI(szID);
	if(true==bOnlyChangePos)
	{
		pkSafe = XUIMgr.Get(kID);
	}
	else
	{
		if(XUIMgr.IsActivate(kID, pkSafe))
		{
			pkSafe->SetInvalidate();
			pkSafe->Close();
			pkSafe = NULL;
		}
		else
		{
			pkSafe = XUIMgr.Call(kID);
		}
	}
		
	if(NULL==pkSafe)	{return;}

	XUI::CXUI_Wnd* pkInv = XUIMgr.Get(L"Inv");
	if(NULL==pkInv)		{return;}

	POINT3I kInvLoc = pkInv->Location();
	POINT2 const kInvSize = pkInv->Size();
	kInvLoc.y+=kInvSize.y;

	pkSafe->Location(kInvLoc);

	CoverSafeAdditionUI(pkSafe);
}

void CoverSafeAdditionUI(XUI::CXUI_Wnd* pkWnd)
{
	if(NULL==pkWnd)	{return;}
	XUI::CXUI_Wnd* pkItemBG = pkWnd->GetControl(L"SFRM_ITEM_BG");
	if(NULL==pkItemBG)	{return;}
	
	XUI::CXUI_Wnd* pkItemSdw = pkItemBG->GetControl(L"SFRM_SDW");
	if(NULL==pkItemSdw)	{return;}

	XUI::CXUI_Builder* pkBld = dynamic_cast<XUI::CXUI_Builder*>(pkItemSdw->GetControl(L"BLD_ICON_BG"));
	if(NULL==pkBld)	{return;}

	int const ITEM_PER_LINE = pkBld->CountX();

	int iOpenCount = 0;
	int iTextNo = 265;
	bool bTimeEnd = true;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(pkPlayer)
	{
		PgInventory* pkInv = pkPlayer->GetInven();
		if( pkInv )
		{
			PgBase_Item kItem;
			if(S_OK==pkInv->GetItem(PgItem_PetInfo::ms_kPetItemEquipPos, kItem) )	//유닛은 없는데 아이템은 있으면
			{
				if(false==kItem.IsUseTimeOut())
				{
					PgItem_PetInfo *pkPetInfo = NULL;
					if(kItem.GetExtInfo(pkPetInfo) && pkPetInfo)
					{
						GET_DEF(PgClassPetDefMgr, kClassDefMgr);
						PgClassPetDef kPetDef;
						kClassDefMgr.GetDef( pkPetInfo->ClassKey(), &kPetDef);

						int const iAbil = kPetDef.GetAbil(AT_ADDITIONAL_INVEN_SIZE);
						iOpenCount = std::min(int(MAX_SAFE_ADD_IDX), iAbil * ITEM_PER_LINE);
					}
					bTimeEnd = false;
				}
				//else
				//{
				//	iTextNo = 268;//추가 인벤토리 아이템의 사용기간이 만료되어 더 이상 이용할 수 없습니다.
				//}
			}
			if( S_OK == pkInv->GetItem(g_RingRItemEquipPos, kItem) )	// 오른쪽 반지에 추가 인벤토리 기능이 있는지 검사.
			{
				if( false == kItem.IsUseTimeOut() )
				{
					GET_DEF(CItemDefMgr, ItemDefMgr);
					CItemDef const * pItemDef = ItemDefMgr.GetDef(kItem.ItemNo());
					if( pItemDef )
					{
						int const Abil = pItemDef->GetAbil(AT_ADDITIONAL_INVEN_SIZE);

						if( Abil > 0 )
						{
							int tempCount = std::min(int(MAX_SAFE_ADD_IDX), Abil * ITEM_PER_LINE);
							if( tempCount > iOpenCount )	// 추가 인벤 확장 기능이 여러개 있을 경우에는 가장 큰 것만 적용.
							{
								iOpenCount = tempCount;
							}
							bTimeEnd = false;
						}
					}
				}
			}
			if( S_OK == pkInv->GetItem(g_RingLItemEquipPos, kItem) )	// 왼쪽 반지에 추가 인벤토리 기능이 있는지 검사.
			{
				if( false == kItem.IsUseTimeOut() )
				{
					GET_DEF(CItemDefMgr, ItemDefMgr);
					CItemDef const * pItemDef = ItemDefMgr.GetDef(kItem.ItemNo());
					if( pItemDef )
					{
						int const Abil = pItemDef->GetAbil(AT_ADDITIONAL_INVEN_SIZE);
						
						if( Abil > 0 )
						{
							int tempCount = std::min(int(MAX_SAFE_ADD_IDX), Abil * ITEM_PER_LINE);
							if( tempCount > iOpenCount )	// 추가 인벤 확장 기능이 여러개 있을 경우에는 가장 큰 것만 적용.
							{
								iOpenCount = tempCount;
							}
							bTimeEnd = false;
						}
					}
				}
			}
			if( bTimeEnd )
			{
				iTextNo = 268;//추가 인벤토리 아이템의 사용기간이 만료되어 더 이상 이용할 수 없습니다.
			}
		}
	}

	XUI::CXUI_Wnd* pkExtend = pkWnd->GetControl(L"SFRM_EXTENDINV_GUIDE");
	if(NULL==pkExtend)	{return;}

	pkExtend->Visible(iOpenCount!=MAX_SAFE_ADD_IDX);
	if(false==pkExtend->Visible())
	{
		return;
	}

	pkExtend->Text(TTW(iTextNo));

	int iExtendSizeY = pkItemBG->Size().y;	//기본 크기

	if(0<iOpenCount)
	{
		int iOpenLine = iOpenCount / ITEM_PER_LINE;
		if(0< (iOpenCount % ITEM_PER_LINE))	
		{
			++iOpenLine;
		}

		iExtendSizeY -= (iOpenLine*44 + 6);	//아래위 여백이 6
	}

	POINT2 kSize = pkExtend->Size();
	kSize.y = iExtendSizeY;
	pkExtend->Size(kSize);

	POINT3I kLoc = pkItemBG->Location();
	kLoc.y+=(pkItemBG->Size().y-iExtendSizeY);
	pkExtend->Location(kLoc);
	lua_tinker::call<void, lwUIWnd>("UI_T_Y_ToCenter", lwUIWnd(pkExtend));	//y축 맞춤
}

bool CheckSafeAddition(PgBase_Item const &kItem, BM::Stream &kPacket)
{
	PgItem_PetInfo *pkPetInfo = NULL;
	if( true==kItem.GetExtInfo( pkPetInfo ) && false==kItem.IsUseTimeOut() )
	{
		GET_DEF(PgClassPetDefMgr, kClassDefMgr);
		PgClassPetDef kPetDef;
		kClassDefMgr.GetDef( pkPetInfo->ClassKey(), &kPetDef);

		int const iAbil = kPetDef.GetAbil(AT_ADDITIONAL_INVEN_SIZE);
		if(0<iAbil)
		{
			lwCloseToolTip();
			lwCallCommonMsgYesNoBox(MB(TTW(269)), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
			return true;
		}
	}

	return false;
}

bool CheckSafeAdditionItem(PgBase_Item const & Item, BM::Stream & Packet)
{
	if( false == Item.IsUseTimeOut() )
	{
		GET_DEF(CItemDefMgr, ItemDefMgr);
		CItemDef const * pItemDef = ItemDefMgr.GetDef(Item.ItemNo());
		if( pItemDef )
		{
			int const Abil = pItemDef->GetAbil(AT_ADDITIONAL_INVEN_SIZE);
			if( 0 < Abil )
			{
				lwCloseToolTip();
				lwCallCommonMsgYesNoBox(MB(TTW(269)), lwPacket(&Packet), true, MBT_COMMON_YESNO_TO_PACKET);
				return true;
			}
		}
	}
	
	return false;
}

PgBase_Item CompairToWarnForSafeAddition(const SItemPos &kCasterPos, const SItemPos &kTargetPos)
{
	if(!(kCasterPos.x==KUIG_FIT && kCasterPos.y==EQUIP_POS_PET) &&
		!(kTargetPos.x==KUIG_FIT && kTargetPos.y==EQUIP_POS_PET))
	{
		return PgBase_Item::NullData();
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(NULL==pkPlayer)	{return PgBase_Item::NullData();}
	
	PgInventory* pkInv = pkPlayer->GetInven();
	if(NULL==pkInv)		{return PgBase_Item::NullData();}

	PgBase_Item kItemCaster;
	PgBase_Item kItemTarget;

	HRESULT castRet = pkInv->GetItem(kCasterPos.x==KUIG_FIT ? kCasterPos : kTargetPos, kItemCaster);
	HRESULT targetRet = pkInv->GetItem(kCasterPos.x==KUIG_FIT ? kTargetPos : kCasterPos, kItemTarget);//kTargetPos가 없으면 인벤에서 빼는것

	PgItem_PetInfo *pkPetInfoCast = NULL;
	PgItem_PetInfo *pkPetInfoTarget = NULL;

	kItemCaster.GetExtInfo(pkPetInfoCast);
	kItemTarget.GetExtInfo(pkPetInfoTarget);

	int iAbilCast = 0;
	int iAbilTarget = 0;

	GET_DEF(PgClassPetDefMgr, kClassDefMgr);
	PgClassPetDef kPetDef;

	if(pkPetInfoCast)
	{
		kClassDefMgr.GetDef( pkPetInfoCast->ClassKey(), &kPetDef);
		iAbilCast = kPetDef.GetAbil(AT_ADDITIONAL_INVEN_SIZE);
	}

	if(pkPetInfoTarget)
	{
		kClassDefMgr.GetDef( pkPetInfoTarget->ClassKey(), &kPetDef);
		iAbilTarget = kPetDef.GetAbil(AT_ADDITIONAL_INVEN_SIZE);
	}

	if(iAbilCast > iAbilTarget)
	{
		return kItemCaster;
	}
	
	return PgBase_Item::NullData();
}

PgBase_Item CompairToWarnForSafeAdditionItem(SItemPos const & CasterPos, SItemPos const & TargetPos)
{
	if( ( !(CasterPos.x == KUIG_FIT && CasterPos.y == EQUIP_POS_RING_L) &&
		!(TargetPos.x == KUIG_FIT && TargetPos.y == EQUIP_POS_RING_L) ) &&
		( !(CasterPos.x == KUIG_FIT && CasterPos.y == EQUIP_POS_RING_R) &&
		!(TargetPos.x == KUIG_FIT && TargetPos.y == EQUIP_POS_RING_R) ) )
	{
		return PgBase_Item::NullData();
	}

	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer ) { return PgBase_Item::NullData(); }
	
	PgInventory * pInv = pPlayer->GetInven();
	if( NULL == pInv) { return PgBase_Item::NullData(); }

	PgBase_Item ItemCaster;
	PgBase_Item ItemTarget;

	HRESULT castRet = pInv->GetItem( CasterPos.x == KUIG_FIT ? CasterPos : TargetPos, ItemCaster);
	HRESULT targetRet = pInv->GetItem( CasterPos.x == KUIG_FIT ? TargetPos : CasterPos, ItemTarget); //kTargetPos가 없으면 인벤에서 빼는것

	int AbilCast = 0;
	int AbilTarget = 0;

	GET_DEF(CItemDefMgr, ItemDefMgr);
	CItemDef const * pItemCastDef = ItemDefMgr.GetDef(ItemCaster.ItemNo());
	if( pItemCastDef )
	{
		AbilCast = pItemCastDef->GetAbil(AT_ADDITIONAL_INVEN_SIZE);
	}

	CItemDef const * pItemTargetDef = ItemDefMgr.GetDef(ItemTarget.ItemNo());
	if( pItemTargetDef )
	{
		AbilTarget = pItemTargetDef->GetAbil(AT_ADDITIONAL_INVEN_SIZE);
	}


	if( AbilCast > AbilTarget)
	{
		return ItemCaster;
	}
	
	return PgBase_Item::NullData();
}

void lwShowMyHomeSize()
{
	if(!g_pkWorld)
	{
		return;
	}
	BM::vstring kstrMyHomeSize(TTW(201819));
	PgHome* pkHome = g_pkWorld->GetHome();
	if(pkHome)
	{
		PgMyHome* pkMyHome = pkHome->GetHomeUnit();
		if(pkMyHome)
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			PLAYER_ABIL kInfo;
			pkMyHome->GetPlayerAbil(kInfo);
			CItemDef const* pDefStyle = kItemDefMgr.GetDef(kInfo.iHairStyle);
			if( pDefStyle )
			{
				int const iMyHomeSize_X = pDefStyle->GetAbil(AT_MYHOME_ROOM_X);
				int const iMyHomeSize_Y = pDefStyle->GetAbil(AT_MYHOME_ROOM_Y);
				kstrMyHomeSize.Replace(L"#SIZEX#", iMyHomeSize_X);
				kstrMyHomeSize.Replace(L"#SIZEY#", iMyHomeSize_Y);
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		return;
	}

	XUI::CXUI_Wnd* pkMyHomeSizeWnd = XUIMgr.Call(L"TOOL_TIP_MYHOME_SIZE");
	if(!pkMyHomeSizeWnd)
	{
		return;
	}

	int const TT_OL_SIZE_HALF = 12;
	int const TT_OL_SIZE = TT_OL_SIZE_HALF * 2;

	pkMyHomeSizeWnd->Size(iTOOLTIP_X_SIZE_MAX, pkMyHomeSizeWnd->Size().y);
	pkMyHomeSizeWnd->Text(kstrMyHomeSize);
	XUI::CXUI_Style_String	kStyleString = pkMyHomeSizeWnd->StyleText();
	POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleString, (iTOOLTIP_X_SIZE_MAX - TT_OL_SIZE), true));

	//텍스트 출력 위치
	POINT2 TextPos;
	TextPos.x = TT_OL_SIZE_HALF;
	// 텍스트 가운데 정렬
	TextPos.x += kTextSize.x / 2;
	pkMyHomeSizeWnd->FontFlag( pkMyHomeSizeWnd->FontFlag() | XUI::XTF_ALIGN_CENTER );
	TextPos.y = TT_OL_SIZE_HALF;
	pkMyHomeSizeWnd->TextPos(TextPos);
	//전체 사이즈 조정
	pkMyHomeSizeWnd->Size(kTextSize.x + TT_OL_SIZE, kTextSize.y + TT_OL_SIZE);
	pkMyHomeSizeWnd->Invalidate(true);

	//위치 설정
	XUI::CXUI_Wnd* pkMyHomeBuffWnd = XUIMgr.Get(L"FRM_MYHOME_BUFF_DISPLAY");
	if(!pkMyHomeBuffWnd)
	{
		return;
	}
	POINT3I const kBuffPos = pkMyHomeBuffWnd->Location();
	int const iSettingY = kBuffPos.y - pkMyHomeSizeWnd->Size().y;
	pkMyHomeSizeWnd->Location(pkMyHomeSizeWnd->Location().x, iSettingY);
}

void MakeEquipTypeText(EEquipType const eEquipType, std::wstring &wstrTextOut)
{
	switch(eEquipType)
	{
	case EEQUIP_WEAPON:
		{
			wstrTextOut =  TTW(3311);
		}break;
	case EEQUIP_ARMOR:
		{
			wstrTextOut =  TTW(3312);
		}break;
	case EEQUIP_ACC:
		{
			wstrTextOut =  TTW(3313);
		}break;
	}
}

bool MakeElementTypeText(int const iItemNo, std::wstring &wstrTextOut)
{
	GET_DEF(CItemDefMgr, kItemDefMgr)
	const CItemDef *pDef = kItemDefMgr.GetDef(iItemNo);
	if(!pDef)
	{
		return false;
	}
	if( 0 != pDef->GetAbil(AT_EXTRACT_ELEMENT_FIRE) )
	{
		wstrTextOut = TTW(792002);
		return true;
	}
	else if( 0 != pDef->GetAbil(AT_EXTRACT_ELEMENT_ICE) )
	{
		wstrTextOut = TTW(792003);
		return true;
	}
	else if( 0 != pDef->GetAbil(AT_EXTRACT_ELEMENT_NATURE) )
	{
		wstrTextOut = TTW(792004);
		return true;
	}
	else if( 0 != pDef->GetAbil(AT_EXTRACT_ELEMENT_CURSE) )
	{
		wstrTextOut = TTW(792005);
		return true;
	}
	else if( 0 != pDef->GetAbil(AT_EXTRACT_ELEMENT_DESTROY) )
	{
		wstrTextOut = TTW(792006);
		return true;
	}
	else if( 0 != pDef->GetAbil(AT_EXTRACT_ELEMENT_RANDOM) )
	{
		wstrTextOut = TTW(792011);
		return true;
	}
	return false;
}

lwAction lwCustomUISummoner_DoAction(char const* szActionName)
{
	return lwAction(g_kUnitCustomUI.DoAction(szActionName));
}

void lwCustomUISummoner_TargetList(lwActionTargetList kout_FoundTargets)
{
	PgActionTargetList * pkFoundTargetList = kout_FoundTargets();
	if(pkFoundTargetList)
	{
		g_kUnitCustomUI.FindTargets( *pkFoundTargetList );
	}
}

void lwCustomUISummoner_MenuUpdatePos()
{
	g_kUnitCustomUI.MenuUpdatePos();
}

lwUIWnd lwGetFocusedEdit()
{
	return lwUIWnd(XUIMgr.GetFocusedEdit());
}

bool lwIsWideResolution( const unsigned int iWidth, const unsigned int iHeight )
{
	unsigned int iDivisor = std::max< unsigned int >( iWidth, iHeight );	// 제수
	unsigned int iDividend = std::min< unsigned int >( iWidth, iHeight );	// 피제수

	unsigned int iGCM = 0; // 최대 공약수
	unsigned int iRemainder = ( iDivisor % iDividend ); // 나머지

	if( 0 == iRemainder )
	{// 큰 수를 작은수로 나누었을 때 나머지가 0이면 작은수가 최대공약수임.
		iGCM = iDividend;
	}
	else // 그렇지 않으면 나머지가 0이 될 때까지 다음과 같이 계산
	{
		while( 1 )
		{
			iDivisor = iDividend;	// 제수는 피제수로 치환
			iDividend = iRemainder;	// 피제수는 나머지로 치환

			iRemainder = ( iDivisor % iDividend );
			if( 0 == iRemainder )
			{
				iGCM = iDividend;
				break;
			}
		}
	}

	// iWidth, iHeight를 각각 최대 공약수로 나눈다.(비율구하기)
	float fWidthRatio = static_cast<float>(iWidth) / iGCM;
	float fHeightRation = static_cast<float>(iHeight) / iGCM;

	// 와이드인지 아닌지 판별 1.6( 16:10 기준 )으로 판별 한다.
	const float fWideRatio = 1.6f;	
	if( fWideRatio <= ( fWidthRatio / fHeightRation ) )
	{
		return true; // 와이드임.
	}
	else
	{
		return false;
	}
}

void lwShowActorUnitType(EUnitType const eType, bool const bShow, bool const bEventScript)
{
	UNIT_PTR_ARRAY kAddUnitArray;
    if( g_kPilotMan.FindUnit(eType, kAddUnitArray) )
    {
        UNIT_PTR_ARRAY::const_iterator c_it = kAddUnitArray.begin();
        while(c_it != kAddUnitArray.end())
        {
            if( c_it->pkUnit )
            {
				if(bEventScript)
				{
					c_it->pkUnit->SetAbil(AT_EVENTSCRIPT_HIDE, false==bShow);
				}

                PgActor* pkActor = g_kPilotMan.FindActor(c_it->pkUnit->GetID());
                if( pkActor )
                {
                    pkActor->SetHide(false==bShow);
                    pkActor->SetHideShadow(false==bShow);
                }
            }
            ++c_it;
        }
    }
}

bool lwSetMiniClassIconIndex(lwUIWnd kWnd, int const iClassNo)
{// 작은 직업 마크 index 설정 하는 함수
	if( kWnd.IsNil() 
		|| 0 >= iClassNo
		) 
	{
		return false;
	}
	XUI::CXUI_Wnd* pkImg = kWnd();
	if( !pkImg )
	{
		return false;
	}
	
	SUVInfo kUVInfo= pkImg->UVInfo();
	kUVInfo.Index = iClassNo;
	if(50 < kUVInfo.Index) 
	{// 신종족 직업 아이콘 나올수 있게 수정
		kUVInfo.Index -= 22;
	}
	pkImg->Visible(true);
	pkImg->UVInfo(kUVInfo);
	pkImg->SetInvalidate();
	return true;
}

void lwSendCheckWorldEvent(lwActor kActor, lwWString kLimitQuestID, lwWString kWorldEventScriptID)
{
	PgActor* pActor = kActor();
	if( !pActor )
	{
		return;
	}
	PgPlayer* pPlayer = dynamic_cast<PgPlayer*>(pActor->GetUnit());
	if( !pPlayer )
	{
		return;
	}
	if( !kWorldEventScriptID.Length() )
	{
		return;
	}
	std::wstring wstrWorldEventScriptID = kWorldEventScriptID.GetWString();
	VEC_WSTRING kVecWorldEventScriptID;
	PgStringUtil::BreakSep(wstrWorldEventScriptID, kVecWorldEventScriptID, _T("/"));

	VEC_WSTRING::const_iterator iter_WorldEvent = kVecWorldEventScriptID.begin();
	if( kLimitQuestID.Length() )
	{//퀘스트 제한이 있을때만
		std::wstring wstrLimitQuestID = kLimitQuestID.GetWString();
		VEC_WSTRING kVecLimitQuestID;
		PgStringUtil::BreakSep(wstrLimitQuestID, kVecLimitQuestID, _T("/"));

		VEC_WSTRING::const_iterator iter_LimitQuest = kVecLimitQuestID.begin();

		while( kVecLimitQuestID.end() != iter_LimitQuest 
			&& kVecWorldEventScriptID.end() != iter_WorldEvent )
		{
			short const sLimitQuestID = static_cast<short>( PgStringUtil::SafeAtoi(*iter_LimitQuest) );
			SUserQuestState const *pQuestState = pPlayer->GetQuestState( sLimitQuestID );
			if(pQuestState)
			{
				if( QS_Ing == pQuestState->byQuestState )
				{
					break;
				}
			}
			++iter_WorldEvent;
			++iter_LimitQuest;
		}
	}

	if( kVecWorldEventScriptID.end() == iter_WorldEvent )
	{
		return;
	}

	int const iWorldEventID = PgStringUtil::SafeAtoi(*iter_WorldEvent);

	WorldEventClientMgrUtil::Net_PT_C_M_CHECK_WORLDEVENT_CONDITION(pActor, iWorldEventID );
}

int GetCommonClassEquipItemType(__int64 const iClassLimit)
{
	switch(iClassLimit)
	{
	case UCLIMIT_ALL:
		{
			return ECCE_ALL_CLASS;
		}break;
	case UCLIMIT_ALL_HUMAN:
		{
			return ECCE_ALL_HUMAN_CLASS;
		}break;
	case UCLIMIT_ALL_DRAGONIAN:
		{
			return ECCE_ALL_DRAGONIAN_CLASS;
		}break;
	}
	return ECCE_NOT_COMMON;
}

//namespace SkillLinkage
//{
//	void lwCallLinkageUI()
//	{
//		PgPilot* pkPilot = g_kPilotMan.GetPlayerPilot();
//		if(!pkPilot)
//		{
//			return;
//		}
//		int const iMaxUnitGuageCnt = pkPilot->GetAbil(AT_SKILL_LINKAGE_POINT_MAX); // 캐릭터가 가진 연계포인트 최대 개수
//		if(!iMaxUnitGuageCnt)
//		{
//			return;
//		}
//		XUIMgr.Call(L"FRM_WV_GUAGE");
//		lwUpdateLinkageUI();
//	}
//
//	void lwUpdateLinkageUI()
//	{
//		PgPilot* pkPilot = g_kPilotMan.GetPlayerPilot();
//		if(!pkPilot)
//		{
//			return;
//		}
//		XUI::CXUI_Wnd* pkMain = XUIMgr.Get(L"FRM_WV_GUAGE");
//		if(!pkMain)
//		{
//			return;
//		}
//		if(!g_pkWorld)
//		{
//			return;
//		}
//		PgCameraMan* pkCamMan = g_pkWorld->GetCameraMan();
//		if(!pkCamMan)
//		{
//			return;
//		}
//		NiCameraPtr spCam = pkCamMan->GetCamera();
//		if(!spCam)
//		{
//			return;
//		}
//		PgActor* pkActor = g_kPilotMan.GetPlayerActor();
//		if(!pkActor)
//		{
//			return;
//		}
//		NiAVObject* pkNode = pkActor->GetObjectByName(ATTACH_POINT_PC_TO_PET);
//		if(!pkNode)
//		{
//			return;
//		}
//		NiPoint3 const kStarPos = pkNode->GetWorldTranslate();
//		float fX = 0.f, fY = 0.f;
//		if( !spCam->WorldPtToScreenPt(kStarPos, fX, fY) )
//		{
//			return;
//		}
//
//		int const iScreenWidth = XUIMgr.GetResolutionSize().x;
//		int const iScreenHeight = XUIMgr.GetResolutionSize().y;
//
//		POINT2 pt2(iScreenWidth*fX,iScreenHeight*(1-fY));
//		pt2.x -= pkMain->Size().x / 2;
//		pt2.y += pkMain->Size().y / 2;
//		pkMain->Location(pt2);
//		//
//		XUI::CXUI_Builder* pkBld = static_cast<XUI::CXUI_Builder*>( pkMain->GetControl(L"BLD_CNT") );
//		if(!pkBld)
//		{
//			return;
//		}
//		XUI::CXUI_Wnd* pkGuageUIUnit = pkMain->GetControl(L"FRM_GUAGE0");
//		if(!pkGuageUIUnit)
//		{
//			return;
//		}
//
//		int const iGuageUIWidth = pkGuageUIUnit->Width();
//		//int const iGuageUIHight = pkGuageUIUnit->Height();
//		int const iMaxUICntOnLine = pkBld->CountX();				// 한 줄에 표현 가능한 UI 개수
//		int const iMaxUICnt =  pkBld->CountX()*pkBld->CountY();		// 최대 표현 가능한 UI 개수 
//		int const iMaxUnitGuageCnt = pkPilot->GetAbil(AT_SKILL_LINKAGE_POINT_MAX); // 캐릭터가 가진 연계포인트 최대 개수
//
//		int const iCurLinkagePoint = pkPilot->GetAbil(AT_SKILL_LINKAGE_POINT);
//
//		// 첫 UI의 X값을 계산해서
//		int iFirstX = iGuageUIWidth*( iMaxUICntOnLine - std::min(iMaxUICntOnLine,iMaxUnitGuageCnt) );
//		iFirstX = iFirstX/2;
//
//		for(int i=0; i < iMaxUICnt; ++i)
//		{
//			BM::vstring vStr("FRM_GUAGE"); vStr+=i;
//			XUI::CXUI_Wnd* pkWnd = pkMain->GetControl(vStr);
//			if(pkWnd)
//			{// 보여줄 게이지 UI갯수만큼만 보여주고
//				bool const bVisible = iMaxUnitGuageCnt > i;
//				pkWnd->Visible( bVisible );
//				if(bVisible)
//				{
//					POINT2 ptOrig = pkWnd->Location();
//					int iCnt = i%iMaxUICntOnLine;
//					if(0 < i
//						&& (0 == i%iMaxUICntOnLine)
//						)
//					{// 다음 행을 보여 줄때는 첫 UI의 X값을 다시 계산해서
//						iFirstX = iGuageUIWidth*(iMaxUICntOnLine-(iMaxUnitGuageCnt%iMaxUICntOnLine) );
//						iFirstX = iFirstX/2;
//					}
//					{// 위치 값을 세팅하고
//						pkWnd->Location(iFirstX+(iCnt*iGuageUIWidth) , ptOrig.y);
//					}
//					{// 현재 연계 포인트 만큼 게이지 UI를 표시해 준다
//						vStr="IMG_WV";
//						XUI::CXUI_Wnd* pkGuage = pkWnd->GetControl(vStr);
//						if(pkGuage)
//						{
//							pkGuage->Visible(i < iCurLinkagePoint);
//						}
//					}
//				}
//			}
//		}
//	}
//}
int lwGetDefence7RelayItem()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return 0;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return 0;
	}

	ContHaveItemNoCount kContItemNo;
	if( S_OK != pkInv->GetItems(UICT_MISSION_DEFENCE7_ITEM, kContItemNo) )
	{
		return 0;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	ContHaveItemNoCount::const_iterator c_it = kContItemNo.begin();
	while(c_it != kContItemNo.end())
	{
		if(CItemDef const* pkItemDef = kItemDefMgr.GetDef( (*c_it).first ))
		{
			if(UICT_DEFENCE7_RELAY == pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2))
			{
				return pkItemDef->No();
			}
		}
		++c_it;
	}

	return 0;
}

int lwGetDefence7PointCopyItem()
{
	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return 0;
	}

	PgInventory * pInven = pPlayer->GetInven();
	if( !pInven )
	{
		return 0;
	}

	ContHaveItemNoCount ContItemNo;
	if( S_OK != pInven->GetItems(UICT_MISSION_DEFENCE7_ITEM, ContItemNo) )
	{
		return 0;
	}

	GET_DEF(CItemDefMgr, ItemDefMgr);
	ContHaveItemNoCount::const_iterator item_iter = ContItemNo.begin();
	while( item_iter != ContItemNo.end() )
	{
		if( CItemDef const * pItemDef = ItemDefMgr.GetDef( (*item_iter).first ) )
		{
			if( UICT_DEFENCE7_POINT_COPY == pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2) )
			{
				return pItemDef->No();
			}
		}
		++item_iter;
	}

	return 0;
}

int lwGetNextIngameNoticeShowTimeInSec(void)
{ //정시까지 몇초가 남았는지를 리턴 (한국 심의등급 표시에 사용)
	SYSTEMTIME kCurTime;
	g_kEventView.GetLocalTime(&kCurTime);
	int iRemainSec = 3600 - (kCurTime.wMinute * 60 + kCurTime.wSecond);
	return iRemainSec;
}

void lwReqExitInstanceDungeon()
{// 그라운드 attribute가 1인맵에서 나가기 요청
	if( false == g_kEventScriptSystem.IsNowActivate() )
	{
		lwCallCommonMsgYesNoBox(MB(TTW(402013)), lwPacket(), false, MBT_EXIT_INSTANCE_DUNGEON);
	}
	else
	{
		SChatLog kChatLog(CT_EVENT_SYSTEM);
		g_kChatMgrClient.AddMessage(799223, kChatLog, true);
	}
}

bool lwCheckReservedClassPromotionAction()
{	
	PgActor* pkActor = g_kPilotMan.GetPlayerActor();
	if(!pkActor)
	{
		return false;
	}

	PgAction* pkAction = pkActor->GetAction();
	if(!pkAction)
	{
		return false;
	}

	if( 100009801 == pkAction->GetActionNo()
		|| 100009901 == pkAction->GetActionNo()
		)
	{
		return true;
	}
	if( pkAction->GetNextActionName() == "a_class_promotion_human"
		||  pkAction->GetNextActionName() == "a_class_promotion_dragonian"
		)
	{
		return true;
	}

	return false;
}

void UpdateLowHPWarnning(int iBeforeHP, int iCurHP)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer) { return; }

	eHPSTATE eCurState = GetHPState(iCurHP, pkPlayer->GetAbil(AT_C_MAX_HP));
	eHPSTATE eBeforeState = GetHPState(iBeforeHP, pkPlayer->GetAbil(AT_C_MAX_HP));
	if(eCurState == eBeforeState) { return; }

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_WARNNING_LOW_HEALTH");
	if(pkWnd)
	{
		if(eCurState == HPSTATE_DIE || eCurState == HPSTATE_NORMAL)
		{ //연출 중에 사망하거나 20% 이상으로 회복되면 연출 종료
			pkWnd->Close();
			return;
		}
	}
	else if(eCurState == HPSTATE_DIE) { return; }

	switch(eCurState)
	{
	case HPSTATE_DIE:
	case HPSTATE_NORMAL:
		{
			return;
		}break;
	case HPSTATE_LOW: //HP가 10~20% 이내일때
		{
			lua_tinker::call<void, int, float, float>("SetLowHPWarnAlpha", -1, 0.4f, 0.2f);
		}break;
	case HPSTATE_VERYLOW: //HP가 0~10% 이내일때
		{
			lua_tinker::call<void, int, float, float>("SetLowHPWarnAlpha", -1, 0.33f, 0.13f);
		}break;
	}
	if(!pkWnd)
	{
		lwCallUI("SFRM_WARNNING_LOW_HEALTH");
	}

}

eHPSTATE GetHPState(int iHP, int iMaxHP)
{
	if(iHP == 0)
	{
		return HPSTATE_DIE;
	}

	float fRateHP = iHP * 100.f / iMaxHP;
	eHPSTATE eState = HPSTATE_NORMAL;
	if(fRateHP < 10.f)
	{
		eState = HPSTATE_VERYLOW;
	}
	else if(fRateHP < 20.f)
	{
		eState = HPSTATE_LOW;
	}
	return eState;
}

void AdjustLowHPWarnningByResolution(void)
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_WARNNING_LOW_HEALTH");
	if(pkWnd)
	{
		pkWnd->Size(XUIMgr.GetResolutionSize());
		pkWnd->Location(POINT2(0, 0));
	}
}

//던전 입장 UI
int const MAX_IMPORTANT_MONSTER_COUNT = 5;
void GetDungeonImportantMonster(PgTrigger * pkTrigger, VEC_INT & rkOut)
{
	if( !pkTrigger )
	{
		return;
	}

	for(int i=0; i<MAX_IMPORTANT_MONSTER_COUNT; ++i)
	{
		BM::vstring vStr("IMPORTANT_MONSTER_");
		vStr += i;
		int const iMonsterNo = PgStringUtil::SafeAtoi(pkTrigger->GetParamFromParamMap( vStr.operator const std::string().c_str() ));
		if(0 == iMonsterNo)
		{
			break;
		}
		rkOut.push_back(iMonsterNo);
	}

	int const iMapNo = pkTrigger->Param();

	GET_DEF(PgMonsterBag, kMonsterBag);
	GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
	SMonsterControl kGetMon;
	kMonsterBag.GetMonster(iMapNo, kGetMon);

	typedef std::set<TBL_TRIPLE_KEY_INT> SET_KEY;
	SET_KEY kContMonster;
	CMonsterDef const* pkDefMonster = NULL;
	for (int i=0 ; i<MAX_MONSTERBAG_ELEMENT; ++i)
	{
		CONT_SET_DATA const& rkMonData = kGetMon.kSetMonster[i];
		CONT_SET_DATA::const_iterator mon_itr = rkMonData.begin();
		while (rkMonData.end() != mon_itr)
		{
			int const iMonsterNo = (*mon_itr);
			pkDefMonster = kMonsterDefMgr.GetDef(iMonsterNo);
			if(pkDefMonster)
			{
				int const iGrade = pkDefMonster->GetAbil(AT_GRADE);
				if(iGrade == EMGRADE_ELITE || iGrade == EMGRADE_BOSS)	//정예몹 이상
				{
					kContMonster.insert(TBL_TRIPLE_KEY_INT(iGrade, pkDefMonster->GetAbil(AT_LEVEL), iMonsterNo));
				}
			}
			++mon_itr;
		}
	}

	for(SET_KEY::const_reverse_iterator c_it=kContMonster.rbegin(); c_it!=kContMonster.rend(); ++c_it)
	{
		rkOut.push_back((*c_it).kTrdKey);
	}
	
	VEC_INT::iterator new_end = std::unique(rkOut.begin(), rkOut.end());
	rkOut.erase(new_end, rkOut.end());
}

BM::vstring GetQuestStateTTW(BYTE const byState)
{
	BM::vstring vStr;
	if((byState&(GWQT_Ing|GWQT_IngAny)) != 0)
	{
		vStr = TTW(20000);
	}
	if((byState&GWQT_Ended) != 0)
	{
		if(vStr.size()>0)
		{
			vStr += L"/";
		}
		vStr += TTW(20001);
	}
	return vStr;
}

void lwWorld_Dungeon_Portal(lwTrigger kTrigger, lwActor kActor)
{
	PgTrigger * pkTrigger = kTrigger.GetSelf();
	PgActor * pkActor = kActor();
	if(!pkTrigger || !pkActor || !pkActor->GetUnit())
	{
		return;
	}

	const CONT_DEFMAP* pkContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pkContDefMap);
	if(!pkContDefMap)
	{
		return;
	}

	int const iMapNo = pkTrigger->Param();
	CONT_DEFMAP::const_iterator map_it = pkContDefMap->find(iMapNo);
	if(map_it==pkContDefMap->end())
	{
		return;
	}

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"FRM_DUNGEON_ENTER");
	if( !pkWnd )
	{
		return;
	}

	BM::GUID kOwner;
	kOwner.Generate();
	pkWnd->OwnerGuid(kOwner);

	if( 0 == PgStringUtil::SafeStrcmp("TRUE", pkTrigger->GetParamFromParamMap("SHOW_PARTY_UI")) )
	{
		lua_tinker::call<void,lwUIWnd,int>("OnCall_DungeonEnterParty", lwUIWnd(pkWnd), iMapNo);
	}

	SPortalAccessInfo kAccessInfo;
	pkTrigger->IsAccessInfo(0, kAccessInfo);

	BM::vstring vStr;

	//Title
	const wchar_t *pName = NULL;
	GetDefString(iMapNo, pName);
	SET_XUI_CONTROL_TEXT(pkWnd, L"FRM_TXT_TITLE", pName);
	
	//Level
	vStr = TTW(401252);
	vStr.Replace(L"#COLOR#", TTW(kAccessInfo.kLevel.bOk ? 401259 : 401260));
	BM::vstring vLevel;
	if(0==kAccessInfo.kLevel.iMin && (0==kAccessInfo.kLevel.iMax || 100<=kAccessInfo.kLevel.iMax))
	{
		vLevel = TTW(401262);
	}
	else
	{
		vLevel = TTW(401261);
		vLevel.Replace(L"#MIN#", kAccessInfo.kLevel.iMin>0 ? BM::vstring(kAccessInfo.kLevel.iMin) : L"");
		vLevel.Replace(L"#MAX#", kAccessInfo.kLevel.iMax>0 && kAccessInfo.kLevel.iMax<100 ? BM::vstring(kAccessInfo.kLevel.iMax) : L"");
	}
	vStr += vLevel;
	SET_XUI_CONTROL_TEXT(pkWnd, L"FRM_TXT_LEVEL", vStr);
	
	//Recommend	
	vStr = TTW(401253);
	vStr.Replace(L"#LEVEL#", (*map_it).second.byRecommend_Level);
	vStr.Replace(L"#PARTY#", (*map_it).second.byRecommend_Party);
	SET_XUI_CONTROL_TEXT(pkWnd, L"FRM_TXT_PARTY", vStr);
	
	//Quest and Item
	vStr = "";
	BM::vstring vQuest;
	if( !kAccessInfo.kContQuest.empty() )
	{
		bool bAndAppend = false;
		for(SPortalAccessInfo::CONT_AND_QUEST::const_iterator and_it=kAccessInfo.kContQuest.begin(); and_it!=kAccessInfo.kContQuest.end(); ++and_it)
		{
			if(bAndAppend)
			{
				vQuest += ' ';
				vQuest += TTW(401268);
				vQuest += L"\n";
			}

			bool bOrAppend = false;
			for(SPortalAccessInfo::CONT_OR_QUEST::const_iterator or_it=(*and_it).second.begin(); or_it!=(*and_it).second.end(); ++or_it)
			{
				PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest((*or_it).kData.sQuestNo);
				if( !pkQuestInfo )
				{
					continue;
				}

				if( !pkQuestInfo->m_kLimit.CheckClass(pkActor->GetUnit()->GetAbil(AT_CLASS)) )
				{
					continue;
				}

				if(bOrAppend)
				{
					vQuest += ' ';
					vQuest += TTW(401267);
					vQuest += L"\n";
				}

				BM::vstring vTitle( TTW(pkQuestInfo->m_iTitleTextNo) );
#ifndef USE_INB
				if( g_pkApp->VisibleClassNo() )
				{
					vTitle += L" (";
					vTitle += (*or_it).kData.sQuestNo;
					vTitle += L")";
				}
#endif
				BM::vstring vTmp( TTW(vQuest.size()>0 ? 401255 : 401254) );
				vTmp.Replace(L"#COLOR#", TTW((*or_it).bOk ? 401259 : 401260));
				vTmp.Replace(L"#LEVEL#", pkQuestInfo->m_kLimit.iMinLevel);
				vTmp.Replace(L"#TITLE#", vTitle);
				vTmp.Replace(L"#TYPE#", GetQuestStateTTW((*or_it).kData.byState));
				vQuest += vTmp;
				bOrAppend = true;
			}
			bAndAppend = true;
		}		
		vQuest += L"\n";
		vQuest += L"\n";
	}
	vStr += vQuest;

	BM::vstring vItem;
	if( !kAccessInfo.kContItem.empty() )
	{
		bool bAndAppend = false;
		GET_DEF(CItemDefMgr, kItemDefMgr);
		for(SPortalAccessInfo::CONT_AND_ITEM::const_iterator and_it=kAccessInfo.kContItem.begin(); and_it!=kAccessInfo.kContItem.end(); ++and_it)
		{
			if(bAndAppend)
			{
				vItem += ' ';
				vItem += TTW(401268);
				vItem += L"\n";
			}

			bool bOrAppend = false;
			for(SPortalAccessInfo::CONT_OR_ITEM::const_iterator or_it=(*and_it).second.begin(); or_it!=(*and_it).second.end(); ++or_it)
			{
				CItemDef const* pkDefItem = kItemDefMgr.GetDef((*or_it).kData.iItemNo);
				if( !pkDefItem )
				{
					continue;
				}

				if(bOrAppend)
				{
					vItem += ' ';
					vItem += TTW(401267);
					vItem += L"\n";
				}

				std::wstring const* pkItemName = &TTW(40140);
				GetDefString(pkDefItem->NameNo(), pkItemName);

				BM::vstring vTmp( TTW(vItem.size()>0 ? 401257 : 401256) );
				vTmp.Replace(L"#COLOR#", TTW((*or_it).bOk ? 401259 : 401260));
				vTmp.Replace(L"#NAME#", *pkItemName);

				BM::vstring const vConsume(TTW((*or_it).kData.bConsume ? 3360 : 3361));
				BM::vstring vType;
				if( IT_SAFE==(*or_it).kData.kInvType )
				{
					vType = TTW(401263);
					vType += ' ';
					vType += vConsume;
				}
				else if( IT_CASH_SAFE==(*or_it).kData.kInvType )
				{
					vType = TTW(401264);
					vType += ' ';
					vType += vConsume;
				}
				else if( IT_FIT==(*or_it).kData.kInvType )
				{
					vType = TTW(401265);
				}
				else
				{
					vType = TTW(401266);
					vType += ' ';
					vType += vConsume;
				}
				vType.Replace(L"#COUNT#", (*or_it).kData.iItemCount);

				vTmp.Replace(L"#TYPE#", vType);
				vItem += vTmp;
				bOrAppend = true;
			}
			bAndAppend = true;
		}
	}
	vStr += vItem;
	SET_XUI_CONTROL_TEXT(pkWnd, L"FRM_TXT_QUEST", vStr);

	//Monster
	XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_MON"));
	int const iBuildCount = pkBuild ? pkBuild->CountX() : 0;
	if(iBuildCount > 0)
	{
		GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
		CMonsterDef const* pkDefMonster = NULL;

		VEC_INT kContMon;
		GetDungeonImportantMonster(pkTrigger, kContMon);
		PgPilotMan::stClassInfo kClassInfo;
		VEC_INT::const_iterator c_it = kContMon.begin();

		for(int i=0; i<iBuildCount; ++i)
		{
			vStr = (L"ICN_MON");
			vStr += i;

			CXUI_Icon* pkIcon = dynamic_cast<CXUI_Icon*>(pkWnd->GetControl(vStr));
			if( pkIcon )
			{
				bool bVisible = false;

				if(c_it != kContMon.end())
				{
					int const iMonsterNo = (*c_it);
					++c_it;

					pkDefMonster = kMonsterDefMgr.GetDef(iMonsterNo);
					if(pkDefMonster)
					{
						int const iIconNo = pkDefMonster->GetAbil(AT_DEF_RES_NO);
						if(iIconNo)
						{
							bVisible = true;
							pkIcon->SetCustomData(&iIconNo, sizeof(iIconNo));
							
							SIconInfo kIconInfo = pkIcon->IconInfo();
							kIconInfo.iIconKey = iMonsterNo;
							pkIcon->SetIconInfo(kIconInfo);
						}
					}
				}

				pkIcon->Visible(bVisible);
			}
		}
	}
	
	//Description
	if(XUI::CXUI_Wnd * pkTmpWnd = pkWnd->GetControl(L"SFRM_DESCRIPTION"))
	{
		pkTmpWnd->Visible( (*map_it).second.iDescriptionTextNo );
		pkTmpWnd->Text( TTW((*map_it).second.iDescriptionTextNo) );

		XUI::CXUI_Style_String	kStyleString = pkTmpWnd->StyleText();
		POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleString));

		POINT2 kPos( pkTmpWnd->Size() );
		kPos.y = kTextSize.y + 15;

		pkTmpWnd->Size(kPos);
		pkTmpWnd->Invalidate(true);
	}

	if( XUI::CXUI_Wnd * pkBtn = pkWnd->GetControl(L"BTN_OK") )
	{
		pkBtn->SetCustomData( UNI(pkTrigger->GetID()) );
	}
}

void lwOnEnter_DungeonParty(lwUIWnd kWnd)
{
	if(kWnd.IsNil())
	{
		return;
	}
	
	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(kWnd.GetSelf()->GetControl(L"LST_PARTY"));
	if(!pkList)
	{
		return;
	}

	XUI::SListItem * pkListItem = pkList->FirstItem();
	while( pkListItem )
	{
		XUI::CXUI_Wnd * pWnd = pkListItem->m_pWnd;
		pWnd = pWnd ? pWnd->GetControl(L"SFRM_ITEM") : NULL;
		pWnd = pWnd ? pWnd->GetControl(L"BTN_SELECT") : NULL;
		pWnd = pWnd ? pWnd->GetControl(L"IMG_SELECT") : NULL;
		if(pWnd && pWnd->Visible())
		{
			//아이템의 GetCustomData에서 패킷을 뽑아내고 해당 하는 채널과 파트장에게 요청을 한다.
			BM::Stream kReadPacket;
			pkListItem->m_pWnd->GetCustomData(kReadPacket);

			short sChannel = 0;
			SIndunPartyInfo kPartyInfo;
			kReadPacket.Pop(sChannel);
			kPartyInfo.ReadFromPacket(kReadPacket);

			BM::Stream kPacket(PT_C_M_REQ_INDUN_PARTY_ENTER);
			kPacket.Push(sChannel);
			kPacket.Push(kPartyInfo.kMasterGuid);
			NETWORK_SEND(kPacket);
			return;
		}
		pkListItem = pkList->NextItem(pkListItem);
	}
}


void UI_AddDungeonPartyList(XUI::CXUI_List * pkList, short const sChannel, SIndunPartyInfo const& rkInfo)
{
	if(!pkList)
	{
		return;
	}

	if(rkInfo.iNowUser >= rkInfo.iMaxUser)
	{
		return;
	}

	if( rkInfo.byPartyState )
	{
		return;
	}

	if(XUI::SListItem * pkItem = pkList->AddItem(rkInfo.kPartyGuid.str()))
	{
		XUI::CXUI_Wnd * pkBody = pkItem->m_pWnd->GetControl(L"SFRM_ITEM");
		if(pkBody)
		{
			BM::Stream kPacket;
			kPacket.Push(sChannel);
			rkInfo.WriteToPacket(kPacket);
			pkItem->m_pWnd->SetCustomData(kPacket);

			if(XUI::CXUI_Wnd * pkTitle = pkBody->GetControl(L"SFRM_TITLE"))
			{
				BM::vstring vStr;
				if(g_kNetwork.NowChannelNo() != sChannel)
				{
					BM::vstring vChannel(TTW(401273));
					vChannel.Replace(L"#NO#", sChannel);
					vStr += vChannel;
				}

				CONT_DEFMAP const * pContDefMap = NULL;
				g_kTblDataMgr.GetContDef(pContDefMap);
				if( NULL != pContDefMap )
				{
					CONT_DEFMAP::const_iterator map_iter = pContDefMap->find(rkInfo.iMasterMapNo);
					if( pContDefMap->end() != map_iter )
					{
						if( GATTR_MISSION == map_iter->second.iAttr )
						{
							vStr += L"[";
							vStr += TTW(420059);
							vStr += L"]";
						}
						else if( GATTR_CHAOS_MISSION == map_iter->second.iAttr )
						{
							vStr += L"[";
							vStr += TTW(420060);
							vStr += L"]";
						}
					}
				}
				vStr += rkInfo.kPartyTitle;
				Quest::SetCutedTextLimitLength(pkTitle, vStr, std::wstring(_T("..")));
			}
			if(XUI::CXUI_Wnd * pkUser = pkBody->GetControl(L"SFRM_USER"))
			{
				BM::vstring vStr(TTW(401272));
				vStr.Replace(L"#NOW#", rkInfo.iNowUser);
				vStr.Replace(L"#MAX#", rkInfo.iMaxUser);
				pkUser->Text(vStr);
			}
		}
	}
}

void lwOnSort_DungeonPartyList(lwUIWnd kWnd, int const MenuType)
{ // Menu Type : 1-Default, 2-Constellation
	struct _SParty
	{
		_SParty() : sChannel(0) {}
		short sChannel;
		SIndunPartyInfo kInfo;

		static bool Sort(_SParty const& rhs, _SParty const lhs)
		{
			if( rhs.kInfo.iNowUser < lhs.kInfo.iNowUser )	{return true;}
			if( rhs.kInfo.iNowUser > lhs.kInfo.iNowUser )	{return false;}

			if( rhs.kInfo.kPartyGuid < lhs.kInfo.kPartyGuid )	{return true;}
			if( rhs.kInfo.kPartyGuid > lhs.kInfo.kPartyGuid )	{return false;}

			return false;
		}
	};
	typedef std::vector<_SParty> CONT_PARTY;

	if( kWnd.IsNil() || NULL==kWnd.GetSelf()->Parent())
	{
		return;
	}

	//ASC(순차), DESC(역순) 정렬하기.
	bool const bDesc = kWnd.GetCustomData<int>() == 0;

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(kWnd.GetSelf()->Parent()->GetControl(L"LST_PARTY"));
	if(!pkList)
	{
		return;
	}

	CONT_PARTY kContParty;
	_SParty kParty;

	XUI::SListItem * pkListItem = pkList->FirstItem();
	while( pkListItem )
	{
		if(pkListItem->m_pWnd)
		{
			BM::Stream kPacket;
			pkListItem->m_pWnd->GetCustomData(kPacket);

			kPacket.Pop(kParty.sChannel);
			kParty.kInfo.ReadFromPacket(kPacket);

			kContParty.push_back(kParty);
		}

		pkListItem = pkList->NextItem(pkListItem);
	}

	std::sort(kContParty.begin(), kContParty.end(), _SParty::Sort);

	pkList->ClearList();
	if(bDesc)
	{
		CONT_PARTY::const_iterator party_it = kContParty.begin();
		while(party_it != kContParty.end())
		{
			if( 1 == MenuType )
			{
				UI_AddDungeonPartyList(pkList, (*party_it).sChannel, (*party_it).kInfo);
			}
			else if( 2 == MenuType )
			{
				g_kConstellationEnterUIMgr.AddItem_DungeonPartyList(pkList, (*party_it).sChannel, (*party_it).kInfo);
			}
			++party_it;
		}
	}
	else
	{
		CONT_PARTY::reverse_iterator party_it = kContParty.rbegin();
		while(party_it != kContParty.rend())
		{
			if( 1 == MenuType )
			{
				UI_AddDungeonPartyList(pkList, (*party_it).sChannel, (*party_it).kInfo);
			}
			else if( 2 == MenuType )
			{
				g_kConstellationEnterUIMgr.AddItem_DungeonPartyList(pkList, (*party_it).sChannel, (*party_it).kInfo);
			}
			++party_it;
		}
	}
}

void OnRefresh_DungeonPartyList(BM::GUID const& rkGuid, CONT_INDUN_PARTY_CHANNEL const& rkList)
{
	XUI::CXUI_Wnd * pkWnd = XUIMgr.Get(L"SFRM_DUNGEON_PARTY");
	if(!pkWnd || pkWnd->OwnerGuid() != rkGuid)
	{
		return;
	}
	
	if(XUI::CXUI_Wnd * pkLoading = pkWnd->GetControl(L"FRM_LOADING"))
	{
		pkLoading->Visible(false);
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LST_PARTY"));
	if(!pkList)
	{
		return;
	}

	CONT_INDUN_PARTY_CHANNEL::const_iterator channel_it = rkList.begin();
	while(channel_it != rkList.end())
	{
		CONT_INDUN_PARTY::const_iterator party_it = channel_it->second.begin();
		while(party_it != channel_it->second.end())
		{
			UI_AddDungeonPartyList(pkList, (*channel_it).first, (*party_it));
			++party_it;
		}
		++channel_it;
	}
}

void SetTextToFitWindowSize(XUI::CXUI_Wnd * pWnd, std::wstring & TextContent, int const ContentID, int const margin)
{// 윈도우에 텍스트를 삽입할 때 윈도우 넓이만큼 글자를 잘라서 세팅해주는 함수
	if( NULL == pWnd )
	{
		return;
	}

	int TextMaxLength = pWnd->Size().x - margin;
	if( 0 > TextMaxLength )
	{
		return;
	}

	int TextWidth = 0;

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(pWnd->Font());
	if( pFont )
	{
		TextWidth = pFont->CalcWidth(TextContent);
	}

	if( 0 == TextWidth )
	{
		return;
	}

	if( TextMaxLength < TextWidth )
	{
		while( TextMaxLength < TextWidth )
		{
			int endpos = TextContent.size() - 2;
			if( endpos > 0 )
			{
				TextContent = TextContent.substr(0, endpos);
				TextWidth = pFont->CalcWidth(TextContent);
			}
			else
			{
				break;
			}
		}
		TextContent += _T("...");
	}

	if( ContentID && g_pkApp->VisibleClassNo() )
	{
		BM::vstring vStr(ContentID);
		TextContent += L"(";
		TextContent += (std::wstring)vStr;
		TextContent += L")";
	}
	pWnd->Text(TextContent);
}

void lwParseXml_BasicCombo()
{
	g_kSkillTree.ParseXml_BasicCombo();
}

bool MakeWeaponTypeName(int const iWeaponType, std::wstring &wstrText)
{
	switch(iWeaponType)
	{
	case PgItemEx::IT_SWORD:		{ wstrText = TTW(2001); }break;		//IT_SWORD = 1,
	case PgItemEx::IT_BTS:			{ wstrText = TTW(2002); }break;		//IT_BTS = 2,
	case PgItemEx::IT_STF:			{ wstrText = TTW(2003); }break;		//IT_STF = 3,
	case PgItemEx::IT_SPR:			{ wstrText = TTW(2004); }break;		//IT_SPR = 4,
	case PgItemEx::IT_BOW:			{ wstrText = TTW(2005); }break;		//IT_BOW = 5,
	case PgItemEx::IT_CROSSBOW:		{ wstrText = TTW(2006); }break;		//IT_CROSSBOW = 6,
	case PgItemEx::IT_CLAW:			{ wstrText = TTW(2007); }break;		//IT_CLAW = 7,
	case PgItemEx::IT_KAT:			{ wstrText = TTW(2008); }break;		//IT_KAT = 8,
	case PgItemEx::IT_GUN_STAFF:	{ wstrText = TTW(2011); }break;		//IT_GUN_STAFF = 11,
	case PgItemEx::IT_GLOVE:		{ wstrText = TTW(2012); }break;		//IT_GLOVE = 12,
	}
	return !wstrText.empty();
}

void MakeClassLimitText(__int64 i64ClassLimit, std::wstring &wstrText)
{
	switch( GetCommonClassEquipItemType( i64ClassLimit ) )
	{
	case ECCE_ALL_CLASS:
		{
			wstrText += TTW(30000+253);
		}break;
	case ECCE_ALL_HUMAN_CLASS:
		{
			wstrText += TTW(30000+254);
		}break;
	case ECCE_ALL_DRAGONIAN_CLASS:
		{
			wstrText += TTW(30000+255);
		}break;
	case ECCE_NOT_COMMON:
		{
			for(int i = 1; i <= UCLASS_MAX; ++i)
			{
				if(1i64<<i & i64ClassLimit)
				{
					wstrText += TTW(30000+i);
					break;
				}
			}
		}break;
	}
}

void CallNpcList(XUI::CXUI_Wnd* pkMinimap)
{
	if(NULL == g_pkWorld || NULL == pkMinimap)
	{
		return;
	}

	CONT_MINIMAP_ICON_INFO kContIconInfo;
	g_pkWorld->GetShowNPC(kContIconInfo);

	lwMinimap::UpdateNpcListUI(pkMinimap, kContIconInfo);
}
