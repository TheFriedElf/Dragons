#include "stdafx.h"
#include "lwBase.h"
#include "lwRenderMan.h"
#include "lwPilotMan.h"
#include "lwParticleMan.h"
#include "lwFriendMgr.h"
#include "lwUIPetition.h"
#include "PgMovieMgr.h"
#include "PgScripting.h"
#include "PgMobileSuit.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgCameraMan.h"
#include "PgParticleMan.h"
#include "PgXmlLoader.h"
#include "ServerLib.h"
#include "PgError.H"
#include "PgPostProcessMan.H"
#include "PgDepthOfField.H"
#include "PgRenderMan.H"
#include "PgRenderer.H"
#include "PgScreenUpdateThread.h"
#include "PgChainAttack.H"
#include "PgScreenBreak.H"
#include "PgSoundMan.h"
#include "PgPilot.H"
#include "PgActor.H"
#include "BrightBloom.H"
#include "PgPetitionMgr.h"
#include "PgOption.H"
#include "PgWorkerThread.h"
#include "variant/Global.h"
#include "lwActionResult.h"
#include "lwUNIT_PTR_ARRAY.h"
#include "PgMath.H"
#include "lwPlayTime.h"
#include "PgChatWindow.H"
#include "lwUI.h"
#include "PgEventCamera.h"
#include "lwChatMgrClient.h"
#include "PgEnergyGaugeBig.h"
#include "PgMobileSuit.h"
#include "Utility/MiniDump.h"
#include "Utility/ExceptionHandler.h"
#include "Utility/ErrorReportFile.h"
#include "NiOcclusionCuller.H"
#include "PgEmporiaMgr.h"
#include "Variant/constant.h"
#include "PgWorldEventClientMgr.H"
#include "lwTrigger.H"
#include "Variant/PgWorldEvent.h"
#include "PgMiniMap.h"
#include "PgSkillOptionMan.H"
#include "PgSkillOptionCommandKey.H"
#include "PgEnvironmentMan.H"
#include "PgEnvElementSnow.H"
#include "PgFxStudio.H"
#include "PgFxParticleStream.H"
#include "PgMemStatusManager.H"

#include "NewWare/Renderer/DrawUtils.h"


extern NiQuaternion g_kSpeed;
extern QuakeCameraInfo g_kQuakeCameraInfo;

extern unsigned int g_iUseAddUnitThread;
extern bool g_bCameraHeightAdjust;
extern bool g_bCameraSmoothMove;
extern bool g_bUseCameraWall;
namespace PgDropBoxUtil
{
	extern float Bias(float x, float biasAmt);
}


void	lwClearMemStatus()
{
	g_kMemStatusManager.ClearData();
}
void	lwActivateMemStatusMan(bool bActive)
{
	g_kMemStatusManager.SetActive(bActive);
}
void	lwRecreateRenderer(bool bFullScreen, bool bBorderless, unsigned int uiWidth,unsigned int uiHeight,unsigned int uiRefreshRate,bool bVSync)
{
	g_pkApp->RecreateRenderer(bFullScreen,bBorderless,uiWidth,uiHeight,uiRefreshRate,bVSync);
}
void	lwReloadFxBank()
{
	g_kParticleMan.ReleaseCacheBySourceType(PgFxParticleStream().GetStreamID());
	g_kFxStudio.LoadBank();
}
void	lwActivateSnow(bool bActivate,bool bChangeImmediately)
{
	if(!g_pkWorld)
	{
		return;
	}
	
	DWORD	dwState = 0;
	if(bActivate)
	{
		dwState = (1<<1);
	}

	g_pkWorld->OnWorldEnvironmentStatusChanged(dwState,bChangeImmediately);
}

void	lwCreateChatDlg()
{
	if( g_pkApp->GetFullscreen() )
	{
		lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(TTW(102)), true);
	}
	else
	{
		XUI::CXUI_Wnd*	pChatWnd = XUIMgr.Get(L"ChatOut");
		XUI::CXUI_Wnd*	pChatBar = XUIMgr.Get(L"ChatBar");
		XUI::CXUI_Wnd*	pSysOut	 = XUIMgr.Get(L"SysChatOut");
		XUI::CXUI_Wnd*	pCatTT = XUIMgr.Get(L"FRM_CHAT_TOOLTIP");
		if( pCatTT )	{ pCatTT->Close(); }
		if( pChatWnd )	{ pChatWnd->Close(); }
		if( pChatBar )	{ pChatBar->Close(); }
		if( pSysOut )	{ pSysOut->Close();	}

		g_kChatWindow.InitWindow(g_pkApp->GetWindowReference());

		int const iScrX = ::GetSystemMetrics(SM_CXSCREEN);
		int const iScrY = ::GetSystemMetrics(SM_CYSCREEN);

		RECT	rcWinP, rcWinC;
		GetWindowRect(g_pkApp->GetWindowReference(), &rcWinP);
		GetWindowRect(g_kChatWindow.GethWnd(), &rcWinC);

		int iTPx = iScrX / 2 - ((rcWinP.right - rcWinP.left) + (rcWinC.right - rcWinC.left)) / 2;
		int iTPy = iScrY / 2 - (rcWinP.bottom - rcWinP.top) / 2;
		iTPx = __max(0, iTPx);
		iTPy = __max(0, iTPy);

		SetWindowPos(g_pkApp->GetWindowReference(), NULL, iTPx, iTPy, 0, 0, SWP_NOSIZE);
		SetWindowPos(g_kChatWindow.GethWnd(), NULL, iTPx + (rcWinP.right - rcWinP.left), iTPy, 0, 0, SWP_NOSIZE);
	}
}
int	lwGetRandomItemNo(int	iClass,int	iGender,int	iPartsType)
{

	static	CItemDefMgr::CONT_DEF	kItemCont;

	if(kItemCont.size() == 0)
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		kItemDefMgr.GetCont(kItemCont);
	}

	static	std::vector<int> kItemNoCont[40][2];

	if(kItemNoCont[iPartsType][iGender].size() == 0)
	{

		for(CItemDefMgr::CONT_DEF::iterator itor = kItemCont.begin(); itor != kItemCont.end(); ++itor)
		{
			CItemDef	*pkDef = itor->second;
			if(pkDef)
			{
				if(pkDef->GetAbil(AT_EQUIP_LIMIT) != (1<<iPartsType))
				{
					continue;
				}

				if(pkDef->CanEquip() == false)
				{
					continue;
				}

				if(pkDef->m_byGender != iGender && pkDef->m_byGender != 3)
				{
					continue;
				}

				kItemNoCont[iPartsType][iGender].push_back(itor->first);
			}
		}
	}

	int	iCount = kItemNoCont[iPartsType][iGender].size();
	if(iCount == 0)
	{
		return	0;
	}

	int	iSelected = kItemNoCont[iPartsType][iGender].at(BM::Rand_Index(iCount));

	return	iSelected;
}
void lwToggleFullScreen(bool bFullScreen)
{
	g_pkApp->RecreateRenderer(bFullScreen, (g_kGlobalOption.GetCurrentGraphicOption("BORDERLESS") == 1), XUIMgr.GetResolutionSize().x, XUIMgr.GetResolutionSize().y, 0, (g_kGlobalOption.GetCurrentGraphicOption("REFRESH_RATE_SYNC") == 1));
}

void lwODS(char const *pcText, bool bAssert=false, int const iLevel=0)
{
	if (pcText == NULL)
		return;

	if (_strnicmp(pcText, "nil", strlen("nil")) == 0)
		bAssert = true;

#ifndef EXTERNAL_RELEASE
	_PgOutputDebugStringLevel(pcText,iLevel);
	if ( 2 == g_iUseDebugConsole )
	{
		WriteToConsole("[ODS]%s",pcText);
	}
#endif
	NILOG(PGLOG_LOG, "[ODS] %s", pcText);
	if (bAssert)
	{
		PG_ASSERT_LOG(0);
	}
}

void lwWriteToConsole(char const *pcText)
{
#ifndef EXTERNAL_RELEASE
	WriteToConsole(pcText);
#endif
}

void lwMessageBox(char const *pcText, char const *pcCaption)
{
	if (pcText == NULL)
		return;

	lwODS(pcText);
	NILOG(PGLOG_ERROR, "[MessageBox] %s", pcText);
#ifndef EXTERNAL_RELEASE
	bool bShowMessageBox = (::GetPrivateProfileInt(TEXT("DEBUG"), TEXT("ShowMessageBox"), 1, g_pkApp->GetConfigFileName()) == 1);
	if (bShowMessageBox)
	{
		std::string strText = pcText;
		strText += "\n Save Dump File? It takes 1~2 minutes to save dump file.";
		int ret = ::MessageBoxA(NULL, strText.c_str(), pcCaption, MB_OKCANCEL);
		if (ret == IDOK && MiniDumper::gpDumper)
		{
			MiniDumper::gpDumper->SetMiniDumpType(MiniDumpWithFullMemory);
			PG_TRY_BLOCK
			::RaiseException(1,  0, 0, NULL);
			PG_CATCH_BLOCK_NO_EXIT
			MiniDumper::gpDumper->SetMiniDumpType(MiniDumpNormal);
		}
	}
#endif
}
void lwMessageBoxW(lwWString wText, lwWString wCaption)
{
	if (wText.IsNil())
		return;

	OutputDebugString(wText().c_str());
	NILOG(PGLOG_ERROR, "[MessageBox] %s", MB(wText()));
#ifndef EXTERNAL_RELEASE
	::MessageBox(NULL,wText().c_str(),wCaption().c_str(),MB_OK);
#endif
}

void lwSetBreak()
{
#ifndef EXTERNAL_RELEASE
 	__asm int 3;
#endif
}
bool lwCheckNil(bool bIsNil)
{
	if(!bIsNil)
	{
		return false;
	}
	PG_ASSERT_LOG(!bIsNil);
#ifndef EXTERNAL_RELEASE
 	__asm int 3;
#endif
	return true;
}

void lwSleep(int iSec)
{
	::Sleep(iSec*1000);
}

lwPilotMan lwGetPilotMan()
{
	return (lwPilotMan)&g_kPilotMan;
}

lwParticleMan lwGetParticleMan()
{
	return (lwParticleMan)&g_kParticleMan;
}

lwMovieMgr lwGetMovieMgr()
{
	return (lwMovieMgr)&g_kMovieMgr;
}

lwChatMgrClient lwGetChatMgrClient()
{
	return (lwChatMgrClient)&g_kChatMgrClient;
}

lwFriendMgr lwGetFriendMgr()
{
	return (lwFriendMgr)&g_kFriendMgr;
}

lwUIPetition lwGetPetitionUI()
{
	return (lwUIPetition)&g_kPetitionMgr;
}

void	lwSetOcclusionCulling(bool bEnable)
{
	NiOcclusionCuller::SetOcclusionCulling(bEnable);
}
void lwSetClassDefaultCamera(int iClass)
{
	PgCameraModeFollow::SetDefaultCamera(iClass);
}

bool lwSetCameraSetting(lwPoint3 kTrn, lwPoint3 kRot)
{
	PgCameraModeFollow::GetCameraInfo().kCurrentPos.fDistanceFromTarget = kTrn().y;
	PgCameraModeFollow::GetCameraInfo().kCurrentPos.fCameraHeight = kTrn().z;
	PgCameraModeFollow::GetCameraInfo().kCurrentPos.fLookAtHeight = kRot().z;

	return true;
}

bool lwSetCameraSpeed(float fA, float fB, float fC, float fD)
{
	g_kSpeed = NiQuaternion(fD, fA, fB, fC);

	return true;
}

bool lwSetCameraSpeedW(float w)
{
	g_kSpeed.SetW(w);
	return true;
}

void lwSetUseCameraWall(bool bUse)
{
	g_bUseCameraWall = bUse;
}

lwPoint3 lwGetCameraTrn(bool bNow)
{
	if(bNow && g_pkWorld)
	{
		PgICameraMode* pkMode = g_pkWorld->GetCameraMan()->GetCameraMode();
		if(pkMode)
		{
			NiCamera const *pkCamera = pkMode->GetCamera();
			if(pkCamera)
			{
				return lwPoint3(pkCamera->GetWorldLocation());
			}
		}
	}
	return lwPoint3(0, 
		PgCameraModeFollow::GetCameraInfo().kCurrentPos.fDistanceFromTarget,
		PgCameraModeFollow::GetCameraInfo().kCurrentPos.fCameraHeight);
}

lwPoint3 lwGetCameraRot()
{
	return lwPoint3(0, 0, PgCameraModeFollow::GetCameraInfo().kCurrentPos.fLookAtHeight);
}

void lwUseCameraHeightAdjust(bool bUse)
{
	g_bCameraHeightAdjust = bUse;
}

void lwUseCameraSmoothMove(bool bUse)
{
	g_bCameraSmoothMove = bUse;
}

bool lwQuakeCamera(float fDuring, float fFactor, int iType, float fFrequency, int iCount)
{
	// frequency의 부호를 bPlus의 부호로 쓰자... argument가 6개가 안된다;;
	PG_ASSERT_LOG(fDuring > 0.0f && fDuring <= 10.0f);
	if (fDuring <= 0.0f)
		return false;

	PG_ASSERT_LOG(g_pkWorld);
	if (g_pkWorld == NULL)
		return false;

	g_pkWorld->m_kCameraMan.ClearQuake();

	g_kQuakeCameraInfo.fQuakeDuring = fDuring;
	g_kQuakeCameraInfo.fQuakeFactor = fFactor;
	g_kQuakeCameraInfo.fQuakeTime = 0.0f;
	g_kQuakeCameraInfo.iQuakeType = iType;
	g_kQuakeCameraInfo.fFrequency = NiAbs(fFrequency);
	g_kQuakeCameraInfo.iCycleCount = iCount;
	g_kQuakeCameraInfo.bPlus = (fFrequency >= 0);

	if (g_kQuakeCameraInfo.fFrequency == 0.0f)
		g_kQuakeCameraInfo.fFrequency = 2.0f;

	if (g_kQuakeCameraInfo.iCycleCount == 0)
		g_kQuakeCameraInfo.iCycleCount = 1;

	return true;
}

void lwClearQuake()
{
	if(g_pkWorld)
	{
		g_pkWorld->m_kCameraMan.ClearQuake();
	}
}

void *lwCreateObject(char const *pcObjectID)
{
	PgIXmlObject *pkObject = PgXmlLoader::CreateObject(pcObjectID);
	return pkObject;
}

int lwAND(int a, int b)
{
	return a&b;
}
void lwTogglePE(int iEffectID)
{
	PgPostProcessMan *pPostProcessMan = g_kRenderMan.GetRenderer()->m_pkPostProcessMan;
	if(pPostProcessMan->GetEffect((PgIPostProcessEffect::EFFECT_TYPE)iEffectID))
	{
		pPostProcessMan->GetEffect((PgIPostProcessEffect::EFFECT_TYPE)iEffectID)->SetActive(!pPostProcessMan->GetEffect((PgIPostProcessEffect::EFFECT_TYPE)iEffectID)->GetActive());
	}
}
void lwDoFile(char const *pcFileName)
{
	//디버그 , 릴리즈에서 따로 쓰도록.
	if (pcFileName == NULL)
		return;

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	PgScripting::DoFile(pcFileName);
}

lwWString	lwGetTextW(int iTextTableID)
{
	return	lwWString(TTW(iTextTableID));
}

lwPilot	lwGetMyPilot()
{
	return	lwPilot(g_kPilotMan.GetPlayerPilot());
}
int	lwATOI(char const *str)
{
	return	atoi(str);
}

char const *lwGetActorPath(int iClass)
{
	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);

	CONT_DEFRES::const_iterator itrDest = pContDefRes->find(iClass);
	if(itrDest == pContDefRes->end())
	{
	
		return 0;
	}
	return lwWString(itrDest->second.strXmlPath).GetStr();
}

int lwGetPetResNoFromLevel(int iPetNo,int iLevel)
{
//	return g_PetDef.GetAbil(SClassKey(iPetNo,iLevel),AT_NAMENO);
	return 0;
}

int lwGetPetResNoFromGrade(int iPetNo,int iGrade)
{
	if ( iGrade == 0 )
	{
		return 0 ;
	}

	int iLevel = 0 ;
	if ( iGrade == 1 )
	{
		iLevel = 1 ;
	}
	else
	{
		iLevel = ( iGrade - 1 ) * 10 ;
	}

	return lwGetPetResNoFromLevel(iPetNo,iLevel);
}

float lwGetFrustumLeft()
{
	return g_kFrustum.m_fLeft;
}
float lwGetFrustumRight()
{
	return g_kFrustum.m_fRight;
}
float lwGetFrustumTop()
{
	return g_kFrustum.m_fTop;
}
float lwGetFrustumBottom()
{
	return g_kFrustum.m_fBottom;
}
float	lwGetAccumTime()
{
	return	g_pkApp->GetAccumTime();
}

float	lwGetCurrentTimeInSec()
{
	return NiGetCurrentTimeInSec();
}

float lwGetFrameTime()
{
	return g_pkApp->GetFrameTime();
}

float lwGetWorldFrameTime()
{
	if(g_pkWorld)
	{
		return g_pkWorld->GetFrameTime();
	}
	return 0.0f;
}

float lwGetWorldAccumTime()
{
	if(g_pkWorld)
	{
		return g_pkWorld->GetAccumTime();
	}
	return 0.0f;
}


void lwSetFrustum(float fLeft, float fRight, float fTop, float fBottom)
{
	g_kFrustum.m_fLeft = fLeft;
	g_kFrustum.m_fRight = fRight;
	g_kFrustum.m_fTop = fTop;
	g_kFrustum.m_fBottom = fBottom;
}

float lwGetFrustumFar()
{
	return g_kFrustum.m_fFar;
}

void lwSetFrustumFar(float const fFar)
{
	g_kFrustum.m_fFar = fFar;
}

void	lwInfoLog(int iType,char *strText)
{
	_PgOutputDebugString("InfoLog[%d] %s\n",iType,strText);
}
void lwSetShaderConstant(char const *var_name,float fValue)
{
	NiShaderFactory::UpdateGlobalShaderConstant(var_name,sizeof(float), &fValue);
}
float	lwGetShaderConstant(char const *var_name)
{
	float	*fValue;
	unsigned	int	uiDataSize=sizeof(float);
	NiShaderFactory::RetrieveGlobalShaderConstant(var_name,uiDataSize,(void const*&)fValue);
	return	*fValue;
}
void	lwDOF(char const *var_name,float fValue)
{
	PgDepthOfField	*pkDOF = (PgDepthOfField*)g_kRenderMan.GetRenderer()->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::DEPTH_OF_FIELD);
	if(pkDOF)
	{
		pkDOF->SetParam(var_name,fValue);
	}
}
void	lwbl_gauss_dev(float fValue)
{
	BrightBloom	*pkBloom= (BrightBloom*)g_kRenderMan.GetRenderer()->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
	if(pkBloom)
	{
		pkBloom->SetGaussBlurDev(fValue);
	}
}
void	lwbl_gauss_mul(float fValue)
{
	BrightBloom	*pkBloom= (BrightBloom*)g_kRenderMan.GetRenderer()->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
	if(pkBloom)
	{
		pkBloom->SetGaussBlurMul(fValue);
	}
}
void	lwbl_si(float fValue)
{
	BrightBloom	*pkBloom= (BrightBloom*)g_kRenderMan.GetRenderer()->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
	if(pkBloom)
	{
		pkBloom->SetSceneIntensity(fValue);
	}
}
void	lwbl_bi(float fValue)
{
	BrightBloom	*pkBloom= (BrightBloom*)g_kRenderMan.GetRenderer()->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
	if(pkBloom)
	{
		pkBloom->SetBloomIntensity(fValue);
	}
}
void	lwbl_hi(float fValue)
{
	BrightBloom	*pkBloom= (BrightBloom*)g_kRenderMan.GetRenderer()->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
	if(pkBloom)
	{
		pkBloom->SetHighlightIntensity(fValue);
	}
}
lwWString lwGetItemName(int iNum)
{
	const wchar_t *pName = NULL;
	GetItemName(iNum, pName);

	if ( pName != NULL )
	{
		return lwWString(pName);
	}
/*
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(iNum);
	if(pItemDef != NULL)
	{
		const wchar_t *pName = NULL;
		bool bResult = GetDefString(pItemDef->NameNo(), pName);
		if(bResult)
		{
			return lwWString(pName);
		}
		else
		{
			return lwWString("");
		}
	}*/
	return lwWString("");
}

bool lwDoString(char* str)
{
	if (str == NULL)
		return false;

	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NILOG(PGLOG_LOG, "DoString(%s)\n", str);
	NIMETRICS_STARTTIMER(a);
	lua_tinker::dostring(*lua_wrapper_user(g_kLuaTinker), str);
	NIMETRICS_ENDTIMER(a);

	return true;
}
void lwAddNewBreak(float fX,float fY)
{
	g_kScreenBreak.AddNewBreak(fX,fY);
}

void lwStartScreenUpdate()
{
	PG_START_SCREEN_UPDATE_THREAD;
}

void lwEndScreenUpdate()
{
	PG_END_SCREEN_UPDATE_THREAD;
}
void lwFPSMethod(int iMethod)
{
	g_pkApp->m_DebugInfo.m_iFPSMethod = iMethod;
}
void	lwUPS(int iUPS)
{
	g_pkApp->m_DebugInfo.m_iUPS = iUPS;
}
void lwSetAddUnitThread(unsigned int iSet)
{
#ifndef EXTERNAL_RELEASE
	g_iUseAddUnitThread = iSet;
#endif
}
void lwSetShowActor(char const* szMiniMapName, int iMode)
{
#ifndef EXTERNAL_RELEASE
	PgMiniMapBase* pkMiniMap = NULL;
	if (szMiniMapName == NULL)
		return;

	if (pkMiniMap = g_kUIScene.GetMiniMapUI(szMiniMapName))
	{
		PgAlwaysMiniMap* pkAlwaysMiniMap = dynamic_cast<PgAlwaysMiniMap*>(pkMiniMap);
		if( pkAlwaysMiniMap )
		{
			pkAlwaysMiniMap->SetViewFlag(PgAlwaysMiniMap::EIT_PLAYER, static_cast<bool>(iMode));
		}
	}
#endif
}
void lwSetVariableActorUpdate(bool bUse)
{
	g_bUseVariableActorUpdate = bUse;
}
bool lwGetVariableActorUpdate()
{
	return g_bUseVariableActorUpdate;
}
void lwPlayAction(int iActionNo)
{
	PgActor	*pkActor = g_kPilotMan.GetPlayerActor();
	if(pkActor)
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		std::wstring kActionName = kSkillDefMgr.GetActionName(iActionNo);
		pkActor->ReserveTransitAction(MB(kActionName));
	}
}

void lwPlayBgSound( int iNo )
{
	g_kSoundMan.PlayBgSound( iNo );
}

void lwStopBgSound()
{
	g_kSoundMan.StopBgSound();
}

void lwLoadBgSound(char const* pcPath, float fVolume)
{
	g_kSoundMan.LoadBgSound(pcPath, fVolume);
}

bool lwPlayBgSoundByPath( char const* pcPath, float fVolume, int const iLoopCount )
{
	return g_kSoundMan.PlayBgSound( pcPath, fVolume, iLoopCount );
}

void lwRegistESCScript( char const* pcScript )
{
	g_kUIScene.RegistESCScript( pcScript );
}

void lwClearESCScript()
{
	g_kUIScene.ClearESCScript();
}

DWORD lwtimeGetTime()
{
	return BM::GetTime32();
}

void lwSetTeleMove(bool bAble)
{
	g_bAbleSetTeleMove = bAble;
}

void lwEventCamera(char const* pcName, bool bPlay)
{
	if(bPlay)
	{
		g_kEventObject.StartAnimation(pcName);
	}
	else
	{
		g_kEventObject.StopAnimation(pcName);
	}
}

void lwClearAllWorks()
{
	g_kAddUnitThread.ClearAllWorks();
	g_kEquipThread.ClearAllWorks();
	g_kLoadWorkerThread.ClearAllWorks();
	g_kMemoryWorkerThread.ClearAllWorks();
}

int lwCS_CheckDamage(lwUnit kCaster, lwUnit kTarget, int iDamage, bool bPhysical, lwActionResult kResult, int iSkillPRate)
{
	return CS_CheckDamage(kCaster(), kTarget(), iDamage, bPhysical, kResult(), iSkillPRate);
}

bool lwCS_GetDmgResult(int iSkillNo, lwUnit kCaster, lwUnit kTarget, int const iDmgRate, int const iDmg, lwActionResult kResult)
{
	return CS_GetDmgResult(iSkillNo, kCaster(),  kTarget(), iDmgRate, iDmg, kResult());
}

bool lwCS_GetSkillResultDefault(int const iSkillNo, lwUnit kCaster, lwUNIT_PTR_ARRAY kTargetArray, lwActionResultVector kResultVector)
{
	return CS_GetSkillResultDefault(iSkillNo, kCaster(), (*kTargetArray()), kResultVector());
}

int lwCS_GetSkillPower(lwUnit kCaster, int iSkillNum)
{
	return 0;
}

bool lwCS_GetSkillResult103201201(int const iSkillNo, lwUnit kCaster, lwUNIT_PTR_ARRAY kTargetArray, lwActionResultVector kResultVector)
{
	return CS_GetSkillResult103201201(iSkillNo, kCaster(), (*kTargetArray()), kResultVector());
}

bool lwCS_GetSkillResult106000101(int const iSkillNo, lwUnit kCaster, lwUNIT_PTR_ARRAY kTargetArray, lwActionResultVector kResultVector)
{
	return CS_GetSkillResult106000101(iSkillNo, kCaster(), (*kTargetArray()), kResultVector());
}

bool lwCS_GetSkillResult105300501(int const iSkillNo, lwUnit kCaster, lwUNIT_PTR_ARRAY kTargetArray, lwActionResultVector kResultVector)
{
	return CS_GetSkillResult105300501(iSkillNo, kCaster(), (*kTargetArray()), kResultVector());
}



void	lwSetRendererToStandard()
{
	NiRenderer	*pkRenderer = NiRenderer::GetRenderer();
	NiDX9Renderer	*pkDX9Renderer = NiDynamicCast(NiDX9Renderer,pkRenderer);
	if(pkDX9Renderer)
	{
		pkDX9Renderer->SetDefaultMaterial(pkDX9Renderer->GetInitialDefaultMaterial());
	}
}


void	lwSetRendererToLegacy()
{
	NiRenderer	*pkRenderer = NiRenderer::GetRenderer();
	NiDX9Renderer	*pkDX9Renderer = NiDynamicCast(NiDX9Renderer,pkRenderer);
	if(pkDX9Renderer)
	{
		pkDX9Renderer->UseLegacyPipelineAsDefaultMaterial();
	}
}

/*
int lwCS_GetReflectDamage(lwPilot kCaster, lwUnit kTarget, int iDamage)
{
	CUnit* pkCaster = kCaster()->GetUnit();
	if (pkCaster != NULL)
	{
		return CS_GetReflectDamage(pkCaster, kTarget(), iDamage);
	}
	return 0;
}

int lwCS_CheckDamage(lwPilot kCaster, lwUnit kTarget, int iDamage, bool bPhysical)
{
	CUnit* pkCaster = kCaster()->GetUnit();
	if (pkCaster != NULL)
	{
		return CS_CheckDamage(pkCaster, kTarget(), iDamage, bPhysical);
	}
	return 0;
}

bool lwCS_GetDmgResult(int iSkillNo, lwPilot kCaster, lwUnit kTarget, int const iDmgRate, int const iDmg, lwActionResult kResult)
{
	CUnit* pkCaster = kCaster()->GetUnit();
	if (pkCaster != NULL)
	{
		return CS_GetDmgResult(iSkillNo, pkCaster,  kTarget(), iDmgRate, iDmg, kResult());
	}
	return 0;
}

bool lwCS_GetSkillResultDefault(int const iSkillNo, lwPilot kCaster, lwUNIT_PTR_ARRAY kTargetArray, lwActionResultVector kResultVector)
{
	CUnit* pkCaster = kCaster()->GetUnit();
	if (pkCaster != NULL)
	{
		return CS_GetSkillResultDefault(iSkillNo, pkCaster, (*kTargetArray()), kResultVector());
	}
	return 0;
}

int lwCS_GetSkillPower(lwPilot kCaster, int iSkillNum)
{
	CUnit* pkCaster = kCaster()->GetUnit();
	if (pkCaster != NULL)
	{
		return CS_GetSkillPower(pkCaster, iSkillNum);
	}
	return 0;
}
*/
lwPoint3	lwBezier4(lwPoint3 p1,lwPoint3 p2,lwPoint3 p3,lwPoint3 p4,float fRate)
{

	NiPoint3	kResult;

	Bezier4(p1(),p2(),p3(),p4(),fRate,kResult);

	return	lwPoint3(kResult);
}
void	lwOnWorldEvent_PhysXTriggerEnter(lwTrigger kTrigger,lwActor kActor)
{
	g_kWorldEventClientMgr.OnPhysXTrigger(kTrigger.GetSelf(),kActor(),WorldEventCondition::CT_Enter);
}
void	lwOnWorldEvent_PhysXTriggerAction(lwTrigger kTrigger,lwActor kActor)
{
	g_kWorldEventClientMgr.OnPhysXTrigger(kTrigger.GetSelf(),kActor(),WorldEventCondition::CT_Action);
}

void lwSetEreaseBackGroundMessage(bool bEnable)
{
	if(g_pkApp)
	{
		g_pkApp->m_bUsedEraseBackgroundMessage = bEnable;
	}
}

void lwCallMapMove( bool const bStart )
{
	g_kEmporiaMgr.Clear();
	PgWorld::SetNowLoading(bStart);

	g_kChatMgrClient.IsShowNoticeAble( !bStart );
	PgEnergyGaugeBig::Show( !bStart );
	if ( bStart )
	{
		g_kChatMgrClient.DisableNotice();
	}
	else
	{
		g_kChatMgrClient.ShowNotice();
	}
}

void lwSetSkillOption(DWORD dwSkillID,bool bEnableCommandKey)
{
	BM::GUID	kPlayerGuid;
	g_kPilotMan.GetPlayerPilotGuid(kPlayerGuid);

	PgSkillOptionCommandKey *pkSkillOptionCommandKey = NiDynamicCast(PgSkillOptionCommandKey,g_kSkillOptionMan.GetSkillOption(dwSkillID,PgSkillOptionCommandKey::GetTypeS()));
	if(!pkSkillOptionCommandKey)
	{
		pkSkillOptionCommandKey = NiDynamicCast(PgSkillOptionCommandKey,g_kSkillOptionMan.AddSkillOption(dwSkillID,PgSkillOptionCommandKey::GetTypeS()));
	}

	if(pkSkillOptionCommandKey)
	{
		pkSkillOptionCommandKey->SetEnableCommandKey(bEnableCommandKey);
		g_kSkillOptionMan.SaveToFile(kPlayerGuid);
	}
}

bool lwGetSkillOption(DWORD dwSkillID)
{
	PgSkillOptionCommandKey *pkSkillOptionCommandKey = NiDynamicCast(PgSkillOptionCommandKey,g_kSkillOptionMan.GetSkillOption(dwSkillID,PgSkillOptionCommandKey::GetTypeS()));
	if( pkSkillOptionCommandKey )
	{
		return pkSkillOptionCommandKey->GetEnableCommandKey();
	}
	lwSetSkillOption(dwSkillID, true);
	return true;
}

void lwCallTimerToolTip(lwUIWnd UIWnd)
{
	g_klwPlayTime.CallTimerToolTip(UIWnd);
}

void lwCallPlayTime(char *pszName)
{
	g_klwPlayTime.Activate(pszName);
}

bool lwIsGroundItemBox_Pickup()
{
	return g_klwPlayTime.IsPickUpItem();
}

void lwSetShowTriangle(bool bShow)
{
	g_bShowTriangleCount = bShow;
}

int lwRandom(int iMax )
{
	if ( 0 == iMax )
	{
		iMax = SHRT_MAX;
	}
	else
	{
		iMax = __min(iMax, SHRT_MAX);
	}
	return BM::Rand_Range(iMax);	//루아에서는 INT_MAX 처리가 안됨. 오버플로우로 음수가 나옴.
}

void lwSetFilteringMode( int iMode )
{
	if(!g_pkWorld)
	{
		return;
	}
    NewWare::Renderer::DrawUtils::SetBaseTextureFiltering( g_pkWorld->GetSceneRoot(), 
                                                             (NiTexturingProperty::FilterMode)iMode );
}

void lwShowSkillAbil(int iSkillNo)
{
	std::wstring wstrText;
	wstrText += UNI("SkillNo : ");
	TCHAR szNo[32] = {0,};
	_itow_s(iSkillNo, szNo, 32, 10);
	wstrText += szNo;
	wstrText += UNI("\n");

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	const CSkillDef	*pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	if(pkSkillDef)
	{
		MakeSkillDefaultAbilToString(pkSkillDef, wstrText);
		MakeAbilObjectToString(pkSkillDef, wstrText);
	}

	//::MessageBox(NULL, wstrText.c_str(), UNI("SkillAbilList"), MB_OK);
	_PgMessageBox("SkillAbilList", "%s", MB(wstrText));
}

void lwShowEffectAbil(int iEffectNo)
{
	std::wstring wstrText;
	wstrText += UNI("EffectID :");
	TCHAR szNo[32] = {0,};
	_itow_s(iEffectNo, szNo, 32, 10);
	wstrText += szNo;
	wstrText += UNI("\n");

	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	const CEffectDef *pkEffectDef = kEffectDefMgr.GetDef(iEffectNo);
	if(pkEffectDef)
	{
		MakeEffectDefaultAbilToString(pkEffectDef,wstrText);
		MakeAbilObjectToString(pkEffectDef, wstrText);
	}

	//::MessageBox(NULL, wstrText.c_str(), UNI("EffectAbilList"), MB_OK);
	_PgMessageBox("EffectAbilList", "%s", MB(wstrText));
}

void lwShowSkillEffectAbil(int iNo)
{
	std::wstring wstrText;
	wstrText += UNI("SkillNo : ");
	TCHAR szNo[32] = {0,};
	_itow_s(iNo, szNo, 32, 10);
	wstrText += szNo;
	wstrText += UNI("\n");

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	const CSkillDef	*pkSkillDef = kSkillDefMgr.GetDef(iNo);
	if(pkSkillDef)
	{
		MakeSkillDefaultAbilToString(pkSkillDef, wstrText);
		MakeAbilObjectToString(pkSkillDef, wstrText);
		int iEffectNo = pkSkillDef->GetEffectNo();
		wstrText += UNI("\n\nEffectID :");
		_itow_s(iEffectNo, szNo, 32, 10);
		wstrText += szNo;
		wstrText += UNI("\n");

		GET_DEF(CEffectDefMgr, kEffectDefMgr);
		const CEffectDef *pkEffectDef = kEffectDefMgr.GetDef(iEffectNo);
		if( pkEffectDef )
		{
			MakeEffectDefaultAbilToString(pkEffectDef,wstrText);
			MakeAbilObjectToString(pkEffectDef, wstrText);
		}
	}
	//::MessageBox(NULL, wstrText.c_str(), UNI("AbilList"), MB_OK);
	_PgMessageBox("AbilList", "%s", MB(wstrText));
}

float	lwGetItemPickUpLimitDistance()
{
	return	Item_PickUp_Distance;
}
#ifndef EXTERNAL_RELEASE
void lwSetDebugLevel(int newDebugLevelValue)
{
	g_iOutPutDebugLevel = newDebugLevelValue;
}

int lwGetCurrentDebugLevel()
{
	return g_iOutPutDebugLevel;
}

void lwUseDebugConsole(bool const bUse)
{
	if(bUse)
	{
		if(0==g_iUseDebugConsole)
		{
			g_iUseDebugConsole = 2;
			AllocConsole();
		}
	}
	else
	{
		if(0!=g_iUseDebugConsole)
		{
			g_iUseDebugConsole = 0;
			FreeConsole();
		}
	}
}
#endif

#ifndef USE_INB
void lwSetVisibleClassNo(bool bVisible)
{
	g_pkApp->VisibleClassNo(bVisible);
}
void lwSetTextDebug(bool const bTextDebug, bool const bQTextDebug)
{
	bool bResetUI = (g_pkApp->VisibleTextDebug() != bTextDebug);
	bResetUI = (g_pkApp->VisibleQTextDebug() != bTextDebug) || bResetUI;

	g_pkApp->VisibleTextDebug(bTextDebug);
	g_pkApp->VisibleQTextDebug(bQTextDebug);

	if( bResetUI )
	{
		g_kUIScene.ResetUI();
	}
}
#endif

float lwGetTotalElementalResistRate(lwUnit attacker, lwUnit defender, int const iAbil)
{
	return CalcTotalElementalResistRate(attacker(), defender(), iAbil);
}

void lwCharacterRevisionNo()
{
	PgPlayer const * pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pkPlayer )
	{
		return;
	}

	PgMyQuest const * pkMyQuest = pkPlayer->GetMyQuest();
	if( NULL == pkMyQuest )
	{
		return;
	}

	int iMigration = pkMyQuest->MigrationRevision();
	if( 0 == iMigration )
	{
		return;
	}

	BM::vstring Message(TTW(99510));
	Message.Replace(L"$NAME$", pkPlayer->Name());
	Message.Replace(L"$VALUE$", iMigration);

	SChatLog kChatLog(CT_EVENT);
	g_kChatMgrClient.AddLogMessage(kChatLog, std::wstring(Message), true);
}

bool lwIsSingleMode()
{
	if(g_pkApp)
	{
		return g_pkApp->IsSingleMode();
	}
	return true;
}
void lwSetSingleMode(bool const bSingleMode)
{
	if(g_pkApp)
	{
		return g_pkApp->SetSingleMode(bSingleMode);
	}
}

float NewCalcValueToRate(int const iSuccessValueType, int iAbilValue, int iFinalSuccessRateType,int iAddFinalSuccessRate, int const iCasterLv, int const iTargetLv)
{// 회피,명중,블록,크리티컬 수치 -> 율 변환 공식
	return PgSkillHelpFunc::CalcSuccessValueToRate(iSuccessValueType, iAbilValue, iFinalSuccessRateType, iAddFinalSuccessRate, iCasterLv, iTargetLv);
}

bool lwBase::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "ClearMemStatus", lwClearMemStatus);
	def(pkState, "ActivateMemStatusMan", lwActivateMemStatusMan);
	def(pkState, "ReloadFxBank", lwReloadFxBank);
	def(pkState, "ActivateSnow", lwActivateSnow);
	def(pkState, "GetItemPickUpLimitDistance", lwGetItemPickUpLimitDistance);
	def(pkState, "CreateChatDlg", lwCreateChatDlg);
	def(pkState, "DoFile", lwDoFile);
	def(pkState, "MessageBox", lwMessageBox);
	def(pkState, "MessageBoxW", lwMessageBoxW);
	def(pkState, "ODS", lwODS);
	def(pkState, "InfoLog", lwInfoLog);
	def(pkState, "WriteToConsole", lwWriteToConsole);
	def(pkState, "SetBreak", lwSetBreak);
	def(pkState, "CheckNil", lwCheckNil);
	def(pkState, "Sleep", lwSleep);
	def(pkState, "GetPilotMan", lwGetPilotMan);
	def(pkState, "GetChatMgrClient", lwGetChatMgrClient);
	def(pkState, "GetFriendMgr", lwGetFriendMgr);
	def(pkState, "GetPetitionUI", lwGetPetitionUI);
	def(pkState, "GetParticleMan", lwGetParticleMan);
	def(pkState, "GetMovieMgr", lwGetMovieMgr);
	def(pkState, "SetClassDefaultCamera", lwSetClassDefaultCamera);
	def(pkState, "SetCameraSetting", lwSetCameraSetting);
	def(pkState, "SetCameraSpeed", lwSetCameraSpeed);	
	def(pkState, "SetCameraSpeedW", lwSetCameraSpeedW);
	def(pkState, "SetUseCameraWall", lwSetUseCameraWall);
	def(pkState, "GetCameraTrn", lwGetCameraTrn);
	def(pkState, "GetCameraRot", lwGetCameraRot);
	def(pkState, "UseCameraHeightAdjust", lwUseCameraHeightAdjust);
	def(pkState, "UseCameraSmoothMove", lwUseCameraSmoothMove);
	def(pkState, "QuakeCamera", lwQuakeCamera);
	def(pkState, "ClearQuake", lwClearQuake);
	def(pkState, "CreateObject", lwCreateObject);
	def(pkState, "AND", lwAND);
	def(pkState, "atoi", lwATOI);
	def(pkState, "GetAccumTime", lwGetAccumTime);
	def(pkState, "GetCurrentTimeInSec", lwGetCurrentTimeInSec);
	def(pkState, "GetFrameTime", lwGetFrameTime);
	def(pkState, "GetWorldAccumTime", lwGetWorldAccumTime);
	def(pkState, "GetWorldFrameTime", lwGetWorldFrameTime);	

	def(pkState, "ToggleFullScreen", lwToggleFullScreen);
	def(pkState, "RecreateRenderer", lwRecreateRenderer);
	def(pkState, "SetOcclusionCulling", lwSetOcclusionCulling);

	def(pkState, "SetRendererToStandard", lwSetRendererToStandard);
	def(pkState, "SetRendererToLegacy", lwSetRendererToLegacy);

	def(pkState, "SetFilteringMode", lwSetFilteringMode);
	def(pkState, "GetRandomItemNo", lwGetRandomItemNo);
	
	def(pkState, "GetTextW",lwGetTextW);
	def(pkState, "GetMyPilot",lwGetMyPilot);
	def(pkState, "GetActorPath", lwGetActorPath);
	def(pkState, "GetPetResNoFromLevel", lwGetPetResNoFromLevel);
	def(pkState, "GetPetResNoFromGrade", lwGetPetResNoFromGrade);
	
	def(pkState, "GetFrustumLeft", lwGetFrustumLeft);
	def(pkState, "GetFrustumRight", lwGetFrustumRight);
	def(pkState, "GetFrustumTop", lwGetFrustumTop);
	def(pkState, "GetFrustumBottom", lwGetFrustumBottom);
	def(pkState, "SetFrustum", lwSetFrustum);
	def(pkState, "GetFrustumFar", lwGetFrustumFar);
	def(pkState, "SetFrustumFar", lwSetFrustumFar);
	
	def(pkState, "GetItemName", lwGetItemName);
	def(pkState, "DoString", lwDoString);
	def(pkState, "SetShaderConstant", lwSetShaderConstant);
	def(pkState, "GetShaderConstant", lwGetShaderConstant);

	def(pkState, "dof",lwDOF);
	def(pkState, "TogglePE",lwTogglePE);
	def(pkState, "StartScreenUpdate", lwStartScreenUpdate);
	def(pkState, "EndScreenUpdate", lwEndScreenUpdate);
	def(pkState, "SetAddUnitThread", lwSetAddUnitThread);
	def(pkState, "SetShowActor", lwSetShowActor);
	def(pkState, "SetVariableActorUpdate", lwSetVariableActorUpdate);
	def(pkState, "GetVariableActorUpdate", lwGetVariableActorUpdate);

	def(pkState, "SetSkillOption", lwSetSkillOption);
	def(pkState, "GetSkillOption", lwGetSkillOption);
	
	def(pkState, "bl_gauss_dev",lwbl_gauss_dev);
	def(pkState, "bl_gauss_mul",lwbl_gauss_mul);
	def(pkState, "bl_si",lwbl_si);
	def(pkState, "bl_bi",lwbl_bi);
	def(pkState, "bl_hi",lwbl_hi);


	def(pkState, "FPSMethod", lwFPSMethod);
	def(pkState, "UPS", lwUPS);
	def(pkState, "AddNewBreak", lwAddNewBreak);

	def(pkState, "PlayAction", lwPlayAction);
	def(pkState, "Bezier4", lwBezier4);

	def(pkState, "PlayBgSound", lwPlayBgSound);
	def(pkState, "StopBgSound", lwStopBgSound);
	def(pkState, "LoadBgSound", lwLoadBgSound);
	def(pkState, "PlayBgSoundByPath", lwPlayBgSoundByPath);

	def(pkState, "RegistESCScript", lwRegistESCScript);
	def(pkState, "ClearESCScript", lwClearESCScript);

	def(pkState, "timeGetTime", lwtimeGetTime);
	def(pkState, "SetTeleMove", lwSetTeleMove);
	def(pkState, "ClearAllWorks", lwClearAllWorks);
	def(pkState, "CharacterRevisionNo", lwCharacterRevisionNo);

	// Builder Command
	def(pkState, "set_builder", lwSetTeleMove);
	def(pkState, "add_item", lwSetTeleMove);
	def(pkState, "allserver_notice", lwSetTeleMove);

	def(pkState, "EventCamera", lwEventCamera);

	// 전투공식 (서버/클라이언트 공통 함수)
	def(pkState, "CS_CheckDamage", &lwCS_CheckDamage);
	def(pkState, "CS_GetDmgResult", &lwCS_GetDmgResult);
	def(pkState, "CS_GetSkillResultDefault", &lwCS_GetSkillResultDefault);
	def(pkState, "CS_GetSkillPower", &lwCS_GetSkillPower);

	def(pkState, "CS_GetSkillResult103201201", &lwCS_GetSkillResult103201201);
	def(pkState, "CS_GetSkillResult106000101", &lwCS_GetSkillResult106000101);
	def(pkState, "CS_GetSkillResult105300501", &lwCS_GetSkillResult105300501);

	def(pkState, "SetEreaseBackGroundMessage", &lwSetEreaseBackGroundMessage);

	def(pkState, "OnWorldEvent_PhysXTriggerAction", &lwOnWorldEvent_PhysXTriggerAction);
	def(pkState, "OnWorldEvent_PhysXTriggerEnter", &lwOnWorldEvent_PhysXTriggerEnter);
	
	def(pkState, "CallMapMove", &lwCallMapMove);
	def(pkState, "CallTimerToolTip", &lwCallTimerToolTip);
	def(pkState, "CallPlayTime", &lwCallPlayTime);
	def(pkState, "IsGroundItemBox_Pickup", &lwIsGroundItemBox_Pickup);

	def(pkState, "SetShowTriangle", &lwSetShowTriangle);
	def(pkState, "ShowSkillAbil", &lwShowSkillAbil);
	def(pkState, "ShowEffectAbil", &lwShowEffectAbil);
	def(pkState, "ShowSkillEffectAbil", &lwShowSkillEffectAbil);
	def(pkState, "IsSingleMode", &lwIsSingleMode);
	def(pkState, "SetSingleMode", &lwSetSingleMode);

	def(pkState, "Random", &lwRandom);
	def(pkState, "Bias", &PgDropBoxUtil::Bias);

	//속성저항률
	def(pkState, "GetTotalElementalResistRate", &lwGetTotalElementalResistRate);

	//투사 룰렛
	//도둑 2/4타 공격

#ifndef EXTERNAL_RELEASE
	def(pkState,"SetDebugLevel", &lwSetDebugLevel);
	def(pkState,"GetCurrentDebugLevel", &lwGetCurrentDebugLevel);	
	def(pkState,"UseDebugConsole", &lwUseDebugConsole);
#endif
#ifndef USE_INB
	def(pkState,"SetVisibleClassNo", &lwSetVisibleClassNo);
	def(pkState,"SetTextDebug", &lwSetTextDebug);
#endif
	def(pkState,"NewCalcValueToRate", &NewCalcValueToRate);

	return true;
}


