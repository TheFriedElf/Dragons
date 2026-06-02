#include "StdAfx.h"

#include "Themida/ThemidaSDK.h"
#include "XUI/XUI_Font.H"
#include "Variant/PgControlDefMgr.h"
#include "Variant/PgBattleSquare.h"
#include "Variant/PgStringUtil.h"
#include "Variant/Global.h"
#include "PgWorld.h"
#include "PgCameraMan.h"
#include "PgCameraModeFollow.h"
#include "PgMobileSuit.h"
#include "PgRenderer.h"
#include "PgXmlLoader.h"
#include "PgLocalManager.h"
#include "PgRemoteManager.h"
#include "PgSoundMan.h"
#include "PgTrigger.h"
#include "PgShineStone.h"
#include "PgInput.h"
//#include "PgActor.h"
#include "PgActorPet.h"
//#include "PgRidingObject.h"
#include "PgActorNpc.h"
#include "PgActorMonster.h"
#include "PgSelectStage.h"
//#include "PgMood.h"
//#include "PgSeasonMan.h"
//#include "PgMoodMan.h"
#include "PgRenderMan.h"
#include "PgScripting.h"
#include "PgPostProcessMan.h"
#include "PgDropBox.h"
#include "PgOption.h"
#include "PgCircleShadow.H"
#include "PgActorManager.H"
//#include "PgAlphaAccumulator.h"
#include "PgAVObjectAlphaProcessManager.h"
#include "PgBreakableObject.H"
#include "PgRope.H"
#include "PgEventScriptDataMan.H"
#include "PgEventScriptSystem.H"
#include "PgObject.H"
#include "NiOcclusionCuller.H"
// Lua Wrapper
#include "lwPacket.h"
#include "lwWorld.h"
#include "PgUIScene.h"
#include "PgOctree.h"
#include "PgPilot.h"
#include "PgClientParty.h"
#include "PgGuild.h"
#include "PgGuildMark.h"
#include "PgPartyBalloon.h"
#include "PgExpeditionBalloon.h"
#include "PgDuelTitle.h"
#include "PgQuestMan.h"
#include "PgWater.h"
#include "PgMath.h"
#include "PgPilotMan.h"
#include "PgSky.h"
#include "PgWorldActionMan.h"
#include "PgActionPool.h"
#include "PgWeather.h"
#include "PgPuppet.h"
#include "PgHome.h"
#include "lwHomeUI_Script.h"
#include "PgProjectileMan.h"
#include "PgWorkerThread.h"
#include "PgTrail.H"
#include "lwBase.H"
#include "PgNifMan.h"
#include "variant/PgNpc.H"
#include "PgStatusEffect.H"
#include "PgParticleMan.H"
#include "PgParticle.H"
#include "FreedomPool.H"
#include "PgMissionComplete.h"
#include "ServerLib.h"
#include "PgLinkedPlane.H"
#include "lwUI.h"
#include "PgHeadBuffIcon.H"
#include "PgPathData.H"
#include "PgEnergyGaugeBig.h"
#include "PgSpotLight.H"
#include "PgContentsBase.h"
#include "PgDirectionArrow.h"
#include "PgWorldObjectPool.H"
#include "PgHelpSystem.h"
#include "PgChatMgrClient.h"
#include "PgCoupleMgr.h"
#include "PgAction.h"
#include "PgActorUtil.h"
#include "PgObserverMode.h"
#include "PgWEClientObjectMgr.h"
#include "PgWorldEventClientMgr.H"
#include "PgPSRoomGroup.H"
#include "lwUICharInfo.h"
#include "PgTextBalloon.h"
#include "PgHouse.h"
#include "PgBattleSquare.h"
#include "Variant/PgWorldTimeCondition.h"
#include "PgWorldElementFog.H"
#include "PgSkillOptionMan.H"
#include "PgEnvironmentMan.H"
#include "PgEnvElementFactory.H"
#include "PgFxStudio.H"
#include "PgScreenBreak.h"
#include "PgChainAttack.h"
#include "PgSkillTargetMan.h"
#include "PgScreenEffect.H"
#include "PgCustomUI_Summmoner.h"
#include "Variant/PgNpcEvent.h"
#include "PgWorldMapUI.h"
#include "PgMinimap.h"
#include "PgDungeonMinimap.h"

#ifndef USE_INB
#include "PgZoneDrawing.h"
#endif//USE_INB

#include "NewWare/Scene/ApplyTraversal.h"
#include "NewWare/Scene/DomainNodeManager.h"
#include "NewWare/Scene/AssetUtils.h"
#include "NewWare/Renderer/DrawUtils.h"
#include "NewWare/Renderer/StreamPack.h"
#include "NewWare/Renderer/Kernel/RenderStateTagExtraData.h"

extern char	const *ACTIONNAME_RIDING;

namespace WorldObjectGroupsUtil
{
	AttachChild::AttachChild(NiAVObject* pkChildNode, bool const bFirstAvail)
		:m_pkChildNode(pkChildNode), m_bFirstAvail(bFirstAvail)
	{
	}
	AttachChild::AttachChild(AttachChild const& rhs)
		:m_pkChildNode(rhs.m_pkChildNode), m_bFirstAvail(rhs.m_bFirstAvail)
	{
	}
	bool AttachChild::operator() (NiNode* pkRootNode)
	{
		if( pkRootNode
		&&	m_pkChildNode )
		{
			pkRootNode->AttachChild(m_pkChildNode, m_bFirstAvail);
			return true;
		}
		return false;
	}

	//
	DetachChild::DetachChild(NiAVObject* pkChildNode)
		: m_pkChildNode(pkChildNode)
	{
	}
	DetachChild::DetachChild(DetachChild const& rhs)
		: m_pkChildNode(rhs.m_pkChildNode)
	{
	}
	bool DetachChild::operator() (NiNode* pkRootNode)
	{
		if( pkRootNode
		&&	m_pkChildNode )
		{
			pkRootNode->DetachChild(m_pkChildNode);
			return true;
		}
		return false;
	}
}

namespace PgWorldUtil
{
	bool SetTriggerEnable(PgWorld* pkWorld, std::string const& rkTriggerID, bool const bEnable)
	{
		if( !pkWorld )
		{
			return false;
		}

		PgTrigger* pkTrigger = pkWorld->GetTriggerByID( rkTriggerID );
		if( pkTrigger )
		{
			pkTrigger->Enable( bEnable );
		}
#ifndef USE_INB
		else
		{
			//_PgMessageBox(GetClientName(), "Can't find Trigger[%s] from sync the MapServer", rkTriggerID.c_str());
		}
#endif
		return true;
	}

	struct SWorldTextCompare
	{
		SWorldTextCompare(PgWorld::stWorldText const& rkOrg)
			: m_rkWorldText(rkOrg)
		{
		}

		bool operator ()(PgWorld::stWorldText const* prhs) const
		{
			if( prhs )
			{
				return m_rkWorldText == *prhs;
			}
			return false;
		}

	private:
		PgWorld::stWorldText const& m_rkWorldText;
	};
};

inline void SetPetCallerUnit(PgPlayer* pkPlayer, PgPlayer* pkCaller)
{
	if(pkPlayer == NULL || pkPlayer->IsCompleteInit() == false ) { return; }
	PgPilot* pkPetPilot = g_kPilotMan.FindPilot(pkPlayer->SelectedPetID());
	if(pkPetPilot == NULL) { return; }
	PgPet* pkPet = dynamic_cast<PgPet*>(pkPetPilot->GetUnit());
	if(pkPet == NULL) { return; }

	pkPet->SetCallerUnit(pkCaller);
}

using namespace XUI;
extern NiPoint3 g_kSpanTrn;
extern NiPoint3 g_kSpanRot;
extern NiPoint3 g_kAdjustTrn;
extern NiPoint3 g_kAdjustTargetTrn;
extern NiQuaternion g_kSpeed;
//extern PgRenderMan* g_pkRenderMan;
extern NiPoint3 g_kZoomInCameraPosition;
extern NiPoint3 g_kZoomOutCameraPosition;

#define MAX_FAR_PLANE 40000.0f
#define PG_WOG_NPC		9
#define PG_USE_NEW_FOG

NxVec3 g_kPlayerLoc(0,0,0);// TODO : UIScene에서 위치 표시할 때만 쓴다.

float g_fSimulationTime = 0.0f;
DWORD g_dwFrameCount = 0;
PgWorld	*g_pkWorld = NULL;
ACE_RW_Thread_Mutex g_kWorldLock;

bool	PgWorld::m_bNowLoading = false;

extern ControllerManager g_kControllerManager;

//extern std::string const CAMTRG_IN_MARK("_IN");
//extern std::string const CAMTRG_OUT_MARK("_OUT");
extern std::string const CAMTRG_OUT_TYPE("OUT");
extern std::string const CAMTRG_IN_TYPE("IN");
extern std::string const CAMTRG_SINGLE_TYPE("SINGLE");
extern std::string const CAMTRG_EMPTY_MARK("");
extern std::string const CAMTRG_TYPE_NAME("CAMERA_TYPE");
extern std::string const CAMTRG_GROUP("GROUP");
extern std::string const CAMTRG_CAMERA_ADJUST("CAMERA_ADJUST");
extern std::string const CAMTRG_WIDTH("WIDTH");
extern std::string const CAMTRG_ZOOM("ZOOM");
extern std::string const CAMTRG_UP("UP");
extern std::string const CAMTRG_TARGET_X("TARGET_X");
extern std::string const CAMTRG_TARGET_Y("TARGET_Y");
extern std::string const CAMTRG_TARGET_Z("TARGET_Z");

std::wstring const EFFECT_REMAIN_TIME_UI(L"LST_EFFECT_TIME");

extern void SaveServerElapsedTime();
bool lwViewPetEquip();	//주인의 GUID

#define PG_USE_NPC_FREEZE
//////////////////////////////////////////////////////////////////////////////////
//	struct	stWorldText
//////////////////////////////////////////////////////////////////////////////////

void printPhysXStat()
{
#ifndef EXTERNAL_RELEASE
	if (g_pkWorld == NULL || !g_pkWorld->GetPhysXScene())
		return;

	NxScene* pkScene2 = g_pkWorld->GetPhysXScene()->GetPhysXScene();
	if (pkScene2 == NULL)
		return;

	NxSceneStats stats;
	const NxSceneStats2* stats2;
	pkScene2->getStats(stats);
	stats2 = pkScene2->getStats2();

	NILOG(PGLOG_STAT, "\n\n--------------- %s world physx statistics ---------------\n", g_pkWorld->GetID().c_str());
	NILOG(PGLOG_STAT, "\t time %f, count %d, avg %f\n", g_fSimulationTime, g_dwFrameCount, g_fSimulationTime / (float)(g_dwFrameCount ? g_dwFrameCount : 1));
	NILOG(PGLOG_STAT, "\t Contacts(%d,%d), Pairs(%d,%d), DynamicACtorsInAwakeGroups(%d,%d) AxisConstraints(%d,%d)\n", stats.numContacts, stats.maxContacts, stats.numPairs, stats.maxPairs, stats.numDynamicActorsInAwakeGroups, stats.maxDynamicActorsInAwakeGroups, stats.numAxisConstraints, stats.maxAxisConstraints);
	NILOG(PGLOG_STAT, "\t SolverBodies(%d,%d), Actors(%d,%d), DynamicActors(%d,%d) StaticShpaes(%d,%d) DynamicShapes(%d,%d), Joints(%d,%d)\n", stats.numSolverBodies, stats.maxSolverBodies, stats.numActors, stats.maxActors, stats.numDynamicActors, stats.maxDynamicActors, stats.numStaticShapes, stats.maxStaticShapes, stats.numJoints, stats.maxJoints);

	if (stats2)
	{
		for (unsigned int i = 0; i < stats2->numStats; ++i)
		{
			if (stats2->stats[i].curValue != 0 && stats2->stats[i].maxValue != 0)
			{
				NILOG(PGLOG_STAT, "\t %s(%d,%d)\n", stats2->stats[i].name, stats2->stats[i].curValue, stats2->stats[i].maxValue);
			}
		}
	}

	g_kControllerManager.printStats();
	NILOG(PGLOG_STAT, "--------------- physx statistics end ---------------\n\n");
#endif
}

PgWorld::stWorldText::stWorldText(NiPoint3 const& kCenter,std::wstring const& kText,float fScale,const NiColorA &kColor,std::wstring const& kFontName)
{
	m_kPosition = kCenter;
	m_fScale = fScale;
	m_kColorA = kColor;
	m_pkText = NULL;

	CXUI_Font	*pFont = g_kFontMgr.GetFont(kFontName);
	if(!pFont)
	{
		return;
	}

	m_pkText = new Pg2DString(XUI::PgFontDef(pFont),_T(""));
	PG_ASSERT_LOG(m_pkText);
	if (m_pkText)
	{				
		m_pkText->SetText(pFont, kText);
	}
}

void	PgWorld::stWorldText::Draw(PgRenderer *pkRenderer,NiCameraPtr spCamera)
{
	if(m_pkText)
	{
		//m_pkText->Draw_3DPos(pkRenderer,m_kPosition,spCamera,NiColorA(0.f,0.f,0.f,1.f),NiColorA(0,0,0,0),false,m_fScale,true,XUI::XTF_ALIGN_CENTER);
		m_pkText->Draw_3DPos(pkRenderer,m_kPosition,spCamera,m_kColorA,NiColorA(0,0,0,0),false,m_fScale,true,XUI::XTF_ALIGN_CENTER);
	}
}
bool PgWorld::stWorldText::operator ==(stWorldText const& rhs) const
{
	return (m_kPosition == rhs.m_kPosition)
		&& ((m_pkText && rhs.m_pkText)? m_pkText->GetText() == rhs.m_pkText->GetText(): false)
		&& (m_fScale == rhs.m_fScale)
		&& (m_kColorA == rhs.m_kColorA);
}

PgWorld::stWorldText::~stWorldText()
{
	SAFE_DELETE(m_pkText);
}
//////////////////////////////////////////////////////////////////////////////////
//	class	PgWorld
//////////////////////////////////////////////////////////////////////////////////

PgWorld::PgWorld(void)
: PgIScene(10, 10),
m_spPhysXScene(0),	
//	m_pkSeasonMan(0),
//	m_pkMoodMan(0),
	m_fGravity(-490.0f),
	m_spFocusFilter(0),
	//m_fFocusFilterTimer(0.0f),
	//m_fTargetAlpha(0.0f),
	//m_fStartAlpha(0.0f),
	//m_fFocusFilterTimerStartTime(0.0f),
	//m_bFocusFilterDelete(false),
	m_kFogColor(0.0f, 0.0f, 0.0f),
	m_fFogAlpha(1.0f),
	m_bUseFogAtSkybox(true),
	m_bUseFogEndFar(true),
	m_eUnlockBidirection(UT_NONETYPE),
	m_bLoadGsa(false),
	m_kAttribute(GATTR_DEFAULT),
	m_fOriginalFogStart(0),
	m_fOriginalFogEnd(0),
	m_fOriginalFogDensity(0),
	m_fLastUpdateTime(0),
	m_fSavedUpdateTime(0),
	m_pkSpotLightMan(NULL),
	m_spExtraData(0),
	m_iDifficultyLevel(-1),
	m_iWorldParticleLastID(0),
	m_iSmallAreaIndex(PgSmallAreaInfo::ERROR_INDEX),
	m_dwMainThreadID(0),
	m_bForceSpot(false),
	m_dwSaveBlinkTime(0),
	m_kDynamicGndAttr(DGATTR_NONE),
    m_pkDomainNodeManager(new NewWare::Scene::DomainNodeManager), 
	m_iEnvironmentStateSetID(-1), 
    m_ePrevDrawMode(NewWare::Renderer::DrawBound::DRAWMODE_NONE),
	m_spMinimapCamera(0),
	m_bNotCulling(false),
	m_kMapNo(0)
	,m_bCallDungeonExistUI(false)
{
	m_pkHome = 0;
	m_pkWater = 0;
	m_pWorldActionMan = 0;
	m_BossGUID.Clear();
	m_Attacker_Monster_GUID.Clear();
	m_Monster_Target_GUID.Clear();
	m_pkFogProperty = 0;
	m_bUseCameraAdjust = false;
	m_bUseFogAdjust = false;
	m_bUseLODAdjust = false;
	m_bShowActorInfo = false;
	m_bShowWorldInfo = false;
	m_iTriggerIndexOrder = 0;
	IsActivateWorldMap(false);
	IsUpdate(true);
	m_bDrawBG = true;
	m_bDrawWorldText = true;
#ifndef EXTERNAL_RELEASE
	m_iDraw = 3;
#endif

	// LOD Default Value;
	m_afWorldObjectLODRange[0] = 1000.0f;
	m_afWorldObjectLODRange[1] = 2000.0f;
	m_afWorldObjectLODRange[2] = 10000.0f;

	m_afViewDistanceRange[0] = 1000.0f;
	m_afViewDistanceRange[1] = 2000.0f;
	m_afViewDistanceRange[2] = 10000.0f;

	m_afWorldQualityRange[0] = 2000.0f;
	m_afWorldQualityRange[1] = 4000.0f;
	m_afWorldQualityRange[2] = 10000.0f;

	memset(m_iUpdateSelectiveLODCounter,0,sizeof(m_iUpdateSelectiveLODCounter));

	m_bUseWorldObjectLOD = true;
	m_bUseWorldQualityRange = true;
	m_bUseViewDistanceRange = true;
	{
		BM::CAutoMutex kLock(g_kWorldLock, true);
		g_pkWorld = this; // TODO 여기 있으면 안된다. AddWorld가 되는 순간 바뀌어야 함. 추후 변경
	}

	m_pkAVObjectAlphaPM = NULL;
	m_pkAVObjectAlphaPM = NiNew PgAVObjectAlphaProcessManager;	

	m_kCullContainter.clear();
	m_kAppCulledToTrueMainTainList.clear();
	m_kAppCulledToFalseMainTainList.clear();

	if(m_pkAVObjectAlphaPM)
	{
		m_pkAVObjectAlphaPM->SetAppCulledToTrueMainTainList(&m_kAppCulledToTrueMainTainList);
		m_pkAVObjectAlphaPM->SetAppCulledToFalseMainTainList(&m_kAppCulledToFalseMainTainList);
	}

	m_pkDamageNumMan = NULL;

	m_pkDynamicNodeRoot = NULL;
	m_pkStaticNodeRoot = NULL;
	m_pkSelectiveNodeRoot = NULL;
	m_pkRoomGroupRoot = 0;

	m_fAccumTime = 0.0f;
	m_fFrameTime = 0.0f;
	m_fUpdateSpeed = 1.0f;
	g_fSimulationTime = 0.0f;
	g_dwFrameCount = 0;
	g_kEventScriptDataMan.Init();
	g_kEventScriptSystem.Init();

	g_kWorldObjectPool.Init();

	m_dwMainThreadID = ::GetCurrentThreadId();
	XUIMgr.Close(EFFECT_REMAIN_TIME_UI);

	if(g_bUseMTSGU)
	{
		m_spMultiThreadSceneGraphUpdater = NiNew PgMultiThreadSceneGraphUpdater();
	}

	CreateEnvironmentSet();

	m_pkOldMouseOverObject = NULL;
}
	
void	PgWorld::ApplyProjectionDecalTextureCoordinate(NiAVObject *pkAVObject,NiBound const &kBound,NiTexture *pkDecalTexture,NiCamera const &kCamera)
{
	NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int	const iArrayCount = pkNode->GetArrayCount();
		for(int i=0;i<iArrayCount;++i)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				ApplyProjectionDecalTextureCoordinate(pkChild,kBound,pkDecalTexture,kCamera);
			}
		}

		return;
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(!pkGeom)
	{
		return;
	}

	unsigned	short	const	usTotalVert = pkGeom->GetVertexCount();
	if(usTotalVert == 0)
	{
		return;
	}

	NiTexturingProperty	*pkTexturing = pkGeom->GetPropertyState()->GetTexturing();
	if(!pkTexturing)
	{
		return;
	}

	int	iTextureSetCount = pkGeom->GetTextureSets();

	NiPoint3	const	*pkVert = pkGeom->GetVertices();
	NiPoint2	*pkNewUVSet = NiNew NiPoint2[usTotalVert];

	if(!pkNewUVSet)
	{
		return;
	}

	pkTexturing->SetDecalMap(0,NiNew NiTexturingProperty::Map(pkDecalTexture,iTextureSetCount,NiTexturingProperty::CLAMP_S_CLAMP_T));

	NiPoint2	*pkUV = pkNewUVSet;
	for(unsigned short s = 0; s<usTotalVert;s++)
	{
		NiPoint3	kVertexWorldPos = pkGeom->GetWorldTransform()*(*pkVert);

		kCamera.WorldPtToScreenPt(kVertexWorldPos,pkUV->x,pkUV->y);

		pkUV->y = 1-pkUV->y;

		pkUV++;
		pkVert++;
	}

	pkGeom->AppendTextureSet(pkNewUVSet);
	NiDelete[] pkNewUVSet;

	pkGeom->GetModelData()->SetConsistency(NiGeometryData::MUTABLE);
	pkGeom->GetModelData()->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
}

void	PgWorld::UpdateProjectionDecal()
{
	NiRenderer	*pkRenderer = NiRenderer::GetRenderer();

	NiNode	*pkOccluder = NiDynamicCast(NiNode,GetSceneRoot()->GetObjectByName("ProjectionDecalTest"));
	if(!pkOccluder)
	{
		return;
	}

	pkRenderer->SetBackgroundColor(NiColorA(0,0,0,0));
	pkRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup, NiRenderer::CLEAR_ALL);	
	
	NiVisibleArray kArray;
	NiCullingProcess kCuller(&kArray);
	NiDrawScene(&m_kProjectionDecalCamera, pkOccluder, kCuller);

	pkRenderer->EndUsingRenderTargetGroup();
}	
void	PgWorld::ApplyProjectionDecal()
{
	NiNode	*pkOccluder = NiDynamicCast(NiNode,GetSceneRoot()->GetObjectByName("ProjectionDecalTest"));
	NiNode	*pkReceiver1 = NiDynamicCast(NiNode,GetSceneRoot()->GetObjectByName("IN_WINDPLAIN_P_B_ROAD1 03"));
	NiNode	*pkReceiver2 = NiDynamicCast(NiNode,GetSceneRoot()->GetObjectByName("Art_WoodFerry1002_01 02"));

	if(!pkOccluder || !pkReceiver1 || !pkReceiver2)
	{
		return;
	}

	float	fRadius = pkOccluder->GetWorldBound().GetRadius();
	if(fRadius == 0)
	{
		return;
	}


	NiRenderer	*pkRenderer = NiRenderer::GetRenderer();
	if(!pkRenderer)
	{
		return;
	}


	int	iTextureSize = std::min(static_cast<int>(fRadius*2*2.0f),512);

	NiTexture::FormatPrefs kFormat;
	kFormat.m_ePixelLayout= NiTexture::FormatPrefs::TRUE_COLOR_32;
	kFormat.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
	kFormat.m_eMipMapped = NiTexture::FormatPrefs::NO;

	m_spRenderedTexture = NiRenderedTexture::Create(iTextureSize, iTextureSize, pkRenderer, kFormat);
	if(!m_spRenderedTexture)
	{
		return;
	}

	NiBound	kBound = pkOccluder->GetWorldBound();
	kBound.SetRadius(fRadius);

	m_spRenderTargetGroup = NiRenderTargetGroup::Create(m_spRenderedTexture->GetBuffer(), pkRenderer, false, true);

	NiPoint3	kCamPos = pkOccluder->GetWorldBound().GetCenter()+NiPoint3(0,0,fRadius+300);
	m_kProjectionDecalCamera.SetTranslate(kCamPos);

	NiMatrix3 kCamRot = NiMatrix3(-NiPoint3::UNIT_Z, NiPoint3::UNIT_Y, NiPoint3::UNIT_X);
	m_kProjectionDecalCamera.SetRotate(kCamRot);

	NiFrustum	kFrustum = m_kProjectionDecalCamera.GetViewFrustum();
	kFrustum.m_bOrtho = true;
	kFrustum.m_fNear = 10;
	kFrustum.m_fFar = 300 + fRadius + 1000;
	kFrustum.m_fLeft = -fRadius;
	kFrustum.m_fRight = fRadius;
	kFrustum.m_fTop = fRadius;
	kFrustum.m_fBottom = -fRadius;
	m_kProjectionDecalCamera.SetViewFrustum(kFrustum);

	m_kProjectionDecalCamera.Update(0);

	pkRenderer->SetBackgroundColor(NiColorA(0,0,0,0));
	pkRenderer->BeginUsingRenderTargetGroup(m_spRenderTargetGroup, NiRenderer::CLEAR_ALL);	
	
	NiVisibleArray kArray;
	NiCullingProcess kCuller(&kArray);
	NiDrawScene(&m_kProjectionDecalCamera, pkOccluder, kCuller);

	pkRenderer->EndUsingRenderTargetGroup();

	ApplyProjectionDecalTextureCoordinate(pkReceiver1,kBound,m_spRenderedTexture,m_kProjectionDecalCamera);
	ApplyProjectionDecalTextureCoordinate(pkReceiver2,kBound,m_spRenderedTexture,m_kProjectionDecalCamera);
}

bool PgWorld::RemoveUnitFromAddUnitQueue(BM::GUID const& kGUID)
{
	_PgOutputDebugString("PgWorld::RemoveUnitFromAddUnitQueue() GUID:%s\n",MB(kGUID.str()));

	bool bRet = false;
	m_kAddUnitQueueLock.Lock();
	AddUnitQueue::iterator iter = m_kAddUnitQueue.begin();
	while (m_kAddUnitQueue.end() != iter)
	{
		if(iter->pkUnit && iter->pkUnit->GetID() == kGUID)
		{
			_PgOutputDebugString("PgWorld::RemoveUnitFromAddUnitQueue() GUID:%s Found!\n",MB(kGUID.str()));
			DeleteAddUnitInfo(iter->pkUnit, iter->pkPilot);
			m_kAddUnitQueue.erase(iter);
			bRet = true;
			break;
		}
		++iter;
	}
	m_kAddUnitQueueLock.Unlock();
	_PgOutputDebugString("PgWorld::RemoveUnitFromAddUnitQueue() GUID:%s Finished\n",MB(kGUID.str()));
	return bRet;
}

PgWorld::~PgWorld()
{
	printPhysXStat();
	if(g_pkWorld == this)
	{
		BM::CAutoMutex kLock(g_kWorldLock, true);
		g_pkWorld = NULL;
		g_kUIScene.TerminateAllMiniMap();
	}
	m_spExtraData = 0;

	ClearAllItemEquipInfo();

	// g_iUseAddUnitThread 2 인경우에는 Actor에서 처리한다.
	if (g_iUseAddUnitThread == 1)
	{
		m_kAddUnitQueueLock.Lock();
		AddUnitQueue::iterator iter = m_kAddUnitQueue.begin();
		while (m_kAddUnitQueue.end() != iter)
		{
			DeleteAddUnitInfo(iter->pkUnit, iter->pkPilot);
			++iter;
		}
		m_kAddUnitQueue.clear();
		m_kAddUnitQueueLock.Unlock();
	}

	TerminateSpotLightMan();

	// 백그라운드 작업이 끝나기를 기다린다.
	m_kPick.RemoveTarget();

	g_kEventScriptSystem.Terminate();
	g_kEventScriptDataMan.Terminate();

	// 아래 두줄 leesg213 2006-11-21 추가
	// m_pSeasonMan 을 메모리 해제 시켜주기 위한 코드임.
	//	SAFE_DELETE_NI(m_pkSeasonMan);
	SAFE_DELETE_NI(m_pkWater);
	SAFE_DELETE_NI(m_pWorldActionMan);
	SAFE_DELETE_NI(m_pkHome);

	//	발사체 매니저 클리어
	g_kProjectileMan.RemoveAll();

	//	파티클 모두 제거
	DetachAllParticle();

	g_kEnvironmentMan.ReleaseAllEnvStateSet();

	//	트레일 매니저 클리어
	if(g_spTrailNodeMan)
	{
		g_spTrailNodeMan = 0;
	}

	g_kLinkedPlaneGroupMan.ClearAll();

	ClearAllWorldText();
	ResetFocusFilter();

	g_kChainAttack.ClearAll();
	g_kScreenBreak.ClearAll();
	g_kScreenEffect.ClearAll();

	//	leesg213 2006-11-22 추가
	//	m_spPhysXScene 가 제거 되기 전에 오브젝트들을 삭제시켜줘야 한다.(그렇지 않으면 오브젝트 삭제과정에서 크래시발생)
	RemoveAllObject();
	SAFE_DELETE_NI(m_pkDamageNumMan);

	TriggerContainer::iterator trigger_itor = m_kTriggerContainer.begin();
	while(trigger_itor != m_kTriggerContainer.end())
	{
		SAFE_DELETE_NI(trigger_itor->second);
		++trigger_itor;
	}
	m_kTriggerContainer.clear();

	m_kBgSoundContainer.clear();

	//	ShineStone 내부 요소들 메모리 제거
	ShineStoneContainer::iterator stone_itor = m_kShineStoneContainer.begin();
	while(stone_itor != m_kShineStoneContainer.end())
	{
		PgShineStone* pkShineStone = stone_itor->second;
		pkShineStone->Terminate();
		SAFE_DELETE(pkShineStone);
		++stone_itor;
	}
	m_kShineStoneContainer.clear();

	//	DynamicEffect 영향 받는 노드 클리어
	PgRenderer::DetachAllAffectedNode(m_spLightRoot);

	m_kOptPolyCont.clear();	// 포인터만 가지고 있으므로.
	m_kOptimizeMidCont.clear();	// 포인터만 가지고 있으므로.
	m_kOptimizeLowCont.clear();	// 포인터만 가지고 있으므로.

	m_spTriggerRoot = 0;
	m_spSpawnRoot = 0;
	m_spPermissionRoot = 0;
	m_spPathRoot = 0;
	m_spSkyRoot = 0;
	m_spLadderRoot = 0;
	m_spRopeRoot = 0;
	m_spPhysRoot = 0;
	m_spLightRoot = 0;
	m_spCameraWalls = 0;
	m_spOptimization_0 = NULL;
	m_spOptimization_1 = NULL;
	m_spOptimization_2 = NULL;

	for (unsigned int i = 0; i < OGT_MAX; ++i)
	{
		m_aspObjectGroups[i] = NULL;
	}

	if(m_spPhysXScene)
	{
		LockPhysX(true);

		//m_spPhysXScene->FetchResults(NI_INFINITY, true);
		if (m_spPhysXScene->GetSnapshot())
		{
			m_spPhysXScene->ClearSceneFromSnapshot();
		}
		m_spPhysXScene->ReleaseSnapshot();
		m_spPhysXScene->ReleaseScene();

		LockPhysX(false);
	}
	m_spPhysXScene = 0;
	m_spCameraAM = 0;

	if(m_pkAVObjectAlphaPM)
	{
		m_pkAVObjectAlphaPM->Release();
		SAFE_DELETE_NI(m_pkAVObjectAlphaPM);
	}

	m_kCullContainter.clear();
	m_kAppCulledToTrueMainTainList.clear();
	m_kAppCulledToFalseMainTainList.clear();

	m_pkDynamicNodeRoot = NULL;
    m_kSharedAssets.RemoveAllAsset();
	m_pkStaticNodeRoot = NULL;
	m_pkRoomGroupRoot = 0;
	m_pkSelectiveNodeRoot = NULL;
	PG_STAT(g_kWorldStatGroup.PrintStatGroup());

	g_kWorldObjectPool.Terminate();
	g_kNifMan.Destroy();
	g_kParticleMan.ReleaseCacheBySourceType("FX");
	g_kParticleMan.ReleaseCacheBySourceType("NIF");
	
    delete m_pkDomainNodeManager;
	m_pkOldMouseOverObject = NULL;

	g_kWorldMapUI.Destroy();
}

NiNode* PgWorld::GetSceneRoot()const
{
	return m_spSceneRoot;
}

NiNode *PgWorld::GetLadderRoot()const
{
	return m_spLadderRoot;
}

NiNode *PgWorld::GetRopeRoot()const
{
	return m_spRopeRoot;
}

NiNode *PgWorld::GetPhysXRoot()const
{
	return m_spPhysRoot;
}

NiNode *PgWorld::GetCameraWallsRoot()const
{
	return m_spCameraWalls;
}

NiNode*	PgWorld::GetLightRoot()const
{
	return	m_spLightRoot;
}

NiNode *PgWorld::GetSpawnRoot()const
{
	return m_spSpawnRoot;
}

bool PgWorld::IsHaveAttr( const T_GNDATTR kCheckAttr )
{
	if ( GATTR_DEFAULT == kCheckAttr )
	{
		return !(GATTR_INSTANCE & m_kAttribute);
	}
	return m_kAttribute & kCheckAttr;
}

int	PgWorld::AttachParticle(NiAVObject *pkParticle,NiPoint3 const& kPos,float const fAliveTime)
{
	int	iSlotID = -1;

	if(m_kWorldParticleDisposedIDList.size() > 0)
	{
		IntList::iterator itor = m_kWorldParticleDisposedIDList.begin();
		iSlotID = *itor;
		m_kWorldParticleDisposedIDList.erase(itor);
	}
	else
	{
		iSlotID = ++m_iWorldParticleLastID;	
	}

	m_kWorldParticle.insert(std::make_pair(iSlotID,pkParticle));

	if(fAliveTime > 0)
	{
		if(NiIsKindOf(PgParticle, pkParticle))
		{
			PgParticle* pkPlayParticle = NiDynamicCast(PgParticle,pkParticle);
			pkPlayParticle->SetAliveTime(fAliveTime);
		}
	}

	if( pkParticle )
	{
		pkParticle->SetTranslate(kPos);
		RunObjectGroupFunc(OGT_EFFECT, WorldObjectGroupsUtil::AttachChild(pkParticle, true));
	}
	return	iSlotID;

}
void	PgWorld::DetachParticle(int iSlotID)
{
	AVObjectMap::iterator itor = m_kWorldParticle.find(iSlotID);
	if(itor == m_kWorldParticle.end())
	{
		return;
	}

	NiAVObject* 	pkAVObject = itor->second;
	if(pkAVObject)
	{
		RunObjectGroupFunc(OGT_EFFECT, WorldObjectGroupsUtil::DetachChild(pkAVObject));
	}	

	m_kWorldParticle.erase(itor);
	m_kWorldParticleDisposedIDList.push_back(iSlotID);
}
void	PgWorld::DetachAllParticle()
{
	for(AVObjectMap::iterator itor = m_kWorldParticle.begin(); itor != m_kWorldParticle.end();++itor)
	{
		NiAVObject* 	pkAVObject = itor->second;
		if(pkAVObject)
		{
			RunObjectGroupFunc(OGT_EFFECT, WorldObjectGroupsUtil::DetachChild(pkAVObject));
			pkAVObject = NULL;
		}	
	}
	m_kWorldParticle.clear();
	m_iWorldParticleLastID = 0;
}
void	PgWorld::ProcessParticle(float fFrameTime)
{
	for(AVObjectMap::iterator itor = m_kWorldParticle.begin(); itor != m_kWorldParticle.end() ;)
	{
		NiAVObject* pkAVObject = itor->second;
		int	iSlotID = itor->first;

		// 파티클 정리
		if(NiIsKindOf(PgParticle, pkAVObject))
		{
			PgParticle* pkParticle = NiDynamicCast(PgParticle,pkAVObject);
			pkParticle->Update(GetAccumTime(),GetFrameTime());
			if(pkParticle->IsFinished())
			{
				RunObjectGroupFunc(OGT_EFFECT, WorldObjectGroupsUtil::DetachChild(pkParticle));
				m_kWorldParticleDisposedIDList.push_back(iSlotID);
				itor = m_kWorldParticle.erase(itor);
				continue;
			}
		}

		++itor;
	}
}

void PgWorld::AttachSound(char const* pcSoundID, NiPoint3& kPos, float fVolume, float fMin, float fMax)
{
	NiAudioSource* pkAudioSource = g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, pcSoundID, fVolume, fMin, fMax, NULL, &kPos);
	if( pkAudioSource )
	{
		RunObjectGroupFunc(OGT_EFFECT, WorldObjectGroupsUtil::AttachChild(pkAudioSource));
	}
}

void PgWorld::DetachSound(char const* pcSoundID)
{
	g_kSoundMan.ResetAudioByID(pcSoundID);
}

//!	보스의 GUID 를 반환
const	BM::GUID&	PgWorld::GetBossGUID()
{	
	return	m_BossGUID;	
}
void	PgWorld::SetBossGUID(BM::GUID const& GUID)
{
	m_BossGUID = GUID;
}

//!	몬스터 타겟 설정
void	PgWorld::SetMonsterTarget(BM::GUID const& MonGUID,BM::GUID const& TargetGUID)
{
	m_Attacker_Monster_GUID = MonGUID;
	m_Monster_Target_GUID = TargetGUID;
}

bool	PgWorld::SetWorldDataForMiniMap(std::string strMiniMapName, POINT2 const& WndSize)
{
	g_kUIScene.SetWorldDataForMiniMap(strMiniMapName, m_kObjectContainer, &m_kTriggerContainer, m_spMinimapCamera, m_kMiniMapImage, WndSize, &m_kDrawHeight);	
	NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( m_spSceneRoot, false );
	return true;
}

bool	PgWorld::SetWorldDataForAllMiniMap()
{
	g_kUIScene.SetWorldDataForAllMiniMap(&m_kObjectContainer[WOCID_MAIN], &m_kTriggerContainer, m_spMinimapCamera, m_kMiniMapImage, &m_kDrawHeight);	
	NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( m_spSceneRoot, false );
	return true;
}

bool	PgWorld::SetMinimapCamera(const char* kCameraName)
{
	if( !kCameraName )
	{
		return false;
	}
	
	CONT_CAMERA::const_iterator c_it = m_kContMinimapCamera.find(kCameraName);
	if( m_kContMinimapCamera.end()!=c_it )
	{
		m_spMinimapCamera = (*c_it).second;
		return true;
	}
	return false;
}

void	PgWorld::SetUpdateSpeed(float fSpeed)
{
	if(fSpeed == 0)
	{
		fSpeed = 0.0001f;
	}
	m_fUpdateSpeed = fSpeed;

	float fTimeStep = 60;
	unsigned int iMaxIter = 1;
	bool bFixed = false;

#if !defined(USE_INB) && !defined(EXTERNAL_RELEASE)
	fTimeStep = (float)::GetPrivateProfileInt(TEXT("Debug"), TEXT("PhysX.TimeStep"), 60, g_pkApp->GetConfigFileName());
	iMaxIter = ::GetPrivateProfileInt(TEXT("Debug"), TEXT("PhysX.MaxIter"), 1, g_pkApp->GetConfigFileName());
	bFixed = (::GetPrivateProfileInt(TEXT("Debug"), TEXT("PhysX.FixedStep"), 0, g_pkApp->GetConfigFileName())) == 1;
#endif

	fTimeStep = fTimeStep*(1.0f/m_fUpdateSpeed);

	if (fTimeStep > 0.0f)
		fTimeStep = 1.0f / fTimeStep;


	m_spPhysXScene->SetTimestep(fTimeStep);
	m_spPhysXScene->SetDoFixedStep(bFixed);
	m_spPhysXScene->GetPhysXScene()->setTiming(fTimeStep, iMaxIter, bFixed ? NX_TIMESTEP_FIXED : NX_TIMESTEP_VARIABLE);

}
float	PgWorld::GetUpdateSpeed()
{
	return	m_fUpdateSpeed;
}

void PgWorld::SetSlowMotion(SlowMotionType const eType, float const fFrom, float const fTo, float const fTime)
{
	kSlowMotionInter.Setup(fFrom,fTo,fTime);
	SetUpdateSpeed(fFrom);
}

bool PgWorld::UpdateSlowMotion(float const fDeltaTime)
{
	if( kSlowMotionInter.IsUse() )
	{
		SetUpdateSpeed(kSlowMotionInter.Interpolate(fDeltaTime) ? 1 : kSlowMotionInter.GetValue());
		return false;
	}
	return true;
}

void	PgWorld::ReserveTransitActionToActors(char const* kActionName,EUnitType kUnitType)
{
	for(int i=0;i<MAX_OBJ_CONT;++i)
	{
		for(ObjectContainer::iterator itr = m_kObjectContainer[i].begin();
			itr != m_kObjectContainer[i].end();
			++itr)
		{
			PgIWorldObject* pkObject = itr->second;
			PgPilot* pkPilot = pkObject->GetPilot();
			if(!pkPilot)
			{
				continue;
			}

			if(NULL == pkPilot->GetUnit())
			{
				continue;
			}
			if(false == pkPilot->GetUnit()->IsUnitType(kUnitType))
			{
				continue;
			}

			if(0 == pkPilot->GetAbil(AT_HP) || pkPilot->GetUnit()->IsDead())
			{
				continue;
			}

			PgActor* pkActor = dynamic_cast<PgActor*>(pkObject);
			if(pkActor)
			{
				if(pkActor->GetAction() && pkActor->GetAction()->GetSkillDef())
				{
					std::wstring kName = pkActor->GetAction()->GetSkillDef()->GetActionName();
					std::wstring kSub = kName.substr(0, 5);
					if(kSub==L"a_die")
					{
						continue;
					}
				}
				pkActor->ReserveTransitAction(kActionName);
			}	
		}
	}
}
void	PgWorld::GetMonsterTarget(BM::GUID& MonGUID,BM::GUID& TargetGUID)
{
	MonGUID = m_Attacker_Monster_GUID;
	TargetGUID = m_Monster_Target_GUID;
}

void PgWorld::GetShowNPC(CONT_MINIMAP_ICON_INFO & rkContIconInfo)const
{
	PgWorld::ObjectContainer kContObj = m_kObjectContainer[WOCID_MAIN];
	PgWorld::ObjectContainer::const_iterator itObject = kContObj.begin();
	for(; itObject != kContObj.end(); ++itObject)
	{
		PgWorld::ObjectContainer::mapped_type pkObject = itObject->second;
		if( pkObject )
		{ // HIDE_MINIMAP 속성을 가진 NPC는 미니맵 하단에 표시되는 NPC 리스트에 넣지 않음.
			PgPilot * pPilot = pkObject->GetPilot();
			if( pPilot )
			{
				PgNpc * pNpc = dynamic_cast<PgNpc*>(pPilot->GetUnit());
				if( pNpc && pNpc->HideMiniMap() )
				{
					continue;
				}
			}
		}

		if( pkObject && pkObject->GetPilot() && pkObject->GetPilot()->GetWorldObject() )
		{
			if(pkObject->GetObjectID() == PgIXmlObject::ID_NPC)
			{
				SMINIMAP_ICON_INFO kInfo;
				kInfo.Type = PgAlwaysMiniMap::EIT_NPC;
				kInfo.kIconID = UNI(pkObject->GetID());
				kInfo.kIconName = pkObject->GetPilot()->GetName();
				rkContIconInfo.insert(rkContIconInfo.end(), kInfo);
			}
		}
	}
}

void	PgWorld::InitSmallAreaInfo(BM::Stream *pkPacket)
{
	m_kSmallAreaInfo.ReadFromPacket(*pkPacket);
	m_iSmallAreaIndex = PgSmallAreaInfo::ERROR_INDEX;

#ifndef USE_INB
	g_kZoneDrawer.SetZoneInfo(m_kSmallAreaInfo.GetCoordMin(), m_kSmallAreaInfo.GetCoordMax(), 
		m_kSmallAreaInfo.GetAreaSize(), m_kSmallAreaInfo.GetTotalCount());
#endif//USE_INB
}
void	PgWorld::UpdateSmallAreaIndex(NiPoint3 const &kPosition)
{
	if(m_kSmallAreaInfo.GetTotalSize() == 0)
	{
		return;
	}

	POINT3	kPos(kPosition.x,kPosition.y,kPosition.z);

	int const	iNewIndex = m_kSmallAreaInfo.GetAreaIndexFromPos(kPos);

	if(m_iSmallAreaIndex != iNewIndex)
	{
		m_iSmallAreaIndex = iNewIndex;

		_PgOutputDebugString("The Small Area Index has been changed to %d(%f,%f,%f)\n",iNewIndex,kPos.x,kPos.y,kPos.z);

		if(g_bSendSmallAreaInfo)
		{
			BM::Stream	kPacket(PT_C_M_NFY_POSITION);
			kPacket.Push(kPos);
			NETWORK_SEND(kPacket)
		}
	}
}
void	PgWorld::AddItemEquipInfo(BM::GUID const &kUnitGUID,int iItemNo,bool bSetToDefaultItem,PgItemEx *pkEquipItem)
{
	BM::CAutoMutex kMutex(m_kItemEquipInfoListMutex,true);

	m_kItemEquipInfoList.push_back(stItemEquipInfo(kUnitGUID,iItemNo,bSetToDefaultItem,pkEquipItem));
}
void	PgWorld::UpdateItemEquipInfo()
{
	BM::CAutoMutex kMutex(m_kItemEquipInfoListMutex,true);

	for(ItemEquipInfoList::iterator itor = m_kItemEquipInfoList.begin(); itor != m_kItemEquipInfoList.end(); ++itor)
	{
		stItemEquipInfo	const	&kInfo = *itor;

		PgActor	*pkActor = g_kPilotMan.FindActor(kInfo.m_kUnitGUID);
		if(!pkActor)
		{
			THREAD_DELETE_ITEM(kInfo.m_pkEquipItem);
			continue;
		}

		if(!pkActor->EquipItemProc(kInfo.m_iItemNo, kInfo.m_bSetToDefaultItem, kInfo.m_pkEquipItem))
		{
			THREAD_DELETE_ITEM(kInfo.m_pkEquipItem);
		}
	}

	m_kItemEquipInfoList.clear();

}
void	PgWorld::ClearAllItemEquipInfo()
{
	BM::CAutoMutex kMutex(m_kItemEquipInfoListMutex,true);

	for(ItemEquipInfoList::iterator itor = m_kItemEquipInfoList.begin(); itor != m_kItemEquipInfoList.end(); ++itor)
	{
		stItemEquipInfo	const	&kInfo = *itor;
		THREAD_DELETE_ITEM(kInfo.m_pkEquipItem);
	}

	m_kItemEquipInfoList.clear();
}

NiPoint3	PgWorld::ThrowRay(NiPoint3 const& kStart, NiPoint3 const& kDir, float const fDistance, int const iShapeType, unsigned int const uiGroup) const	//	레이를 쏴서 충돌지점을 구한다.
{
	if(!m_spPhysXScene) return NiPoint3(-1,-1,-1);
	PG_ASSERT_LOG(fDistance > 0.0f);

	NxVec3	vStart(kStart.x,kStart.y,kStart.z),vDir(kDir.x,kDir.y,kDir.z);

	NxRay kRay(vStart, vDir);
	NxRaycastHit kHit;
	
	NxShapesType eShapeType = NX_STATIC_SHAPES;
	switch(iShapeType)
	{
	case NxShapesType::NX_DYNAMIC_SHAPES	:	{ eShapeType = NxShapesType::NX_DYNAMIC_SHAPES;	} break;
	case NxShapesType::NX_ALL_SHAPES		:	{ eShapeType = NxShapesType::NX_ALL_SHAPES;		} break;
	default									:	{ eShapeType = NxShapesType::NX_STATIC_SHAPES;	} break;
	}
	
	NxShape* pkHitShape = PgWorld::raycastClosestShape(kRay, eShapeType, kHit, uiGroup, NiMax(fDistance, 0.1f), NX_RAYCAST_SHAPE | NX_RAYCAST_IMPACT);
	if(pkHitShape)
	{
		return	NiPoint3(kHit.worldImpact.x,kHit.worldImpact.y,kHit.worldImpact.z);
	}

	return	NiPoint3(-1,-1,-1);
}

void PgWorld::RayCheckObjectRange(PgWOPriorityQueue& kOut,int iActionNo,NiPoint3 const& vStart, unsigned int uiGroup, float fRange,EAttackedUnitPos kTargetPosType,bool bNoDuplication,PgActor* pkCasterActor )
{
	if(!m_spPhysXScene)
	{
		return;
	}

	int	nbMaxShapes = 100;
	NxShape* pkCollidedShapes[100];

	PG_STAT(PgStatTimerF timerA(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.RayCheck"), g_pkApp->GetFrameCount()));
	int iTotalShapes = overlapSphereShapes(NxSphere(NxVec3(vStart.x,vStart.y,vStart.z),fRange),
		NX_DYNAMIC_SHAPES,nbMaxShapes,pkCollidedShapes,NULL,uiGroup,NULL,true);

	NiPoint3 vShapePos;
	for(int i=0;i<iTotalShapes;++i)
	{
		NxShape* pkHitShape = pkCollidedShapes[i];
		if (pkHitShape && pkHitShape->userData)
		{
			PgActor* pkActor = (PgActor*)pkHitShape->userData;

			if(0 < iActionNo && false == PgAction::IsCorrectTargetWithTargetType(iActionNo,pkActor,pkCasterActor))
			{
				continue;
			}

			if (false == pkActor->GetCanHit() && pkActor->IsEnemy(pkCasterActor) )
			{
				continue;
			}

			bool bCorrectTarget = false;

			if (pkActor->IsDownState() && (kTargetPosType&EAttacked_Pos_ground))	{ bCorrectTarget = true; }
			if (pkActor->IsBlowUp() && (kTargetPosType&EAttacked_Pos_Blowup))		{ bCorrectTarget = true; }
			if (!pkActor->IsDownState() && !pkActor->IsBlowUp() && (kTargetPosType&EAttacked_Pos_Normal)) { bCorrectTarget = true; }

			if(!bCorrectTarget)
			{
				continue;
			}

			if (bNoDuplication && kOut.CheckExist(pkActor))
			{
				continue;
			}

			NiPhysXTypes::NxVec3ToNiPoint3(pkCollidedShapes[i]->getGlobalPosition(), vShapePos);
			NxVec3	vRayDir(vShapePos.x-vStart.x,vShapePos.y-vStart.y,vShapePos.z-vStart.z);
			float	fCheckDistance = vRayDir.magnitude();
			PG_ASSERT_LOG(fCheckDistance > 0.0f);

			if(0.0f < fCheckDistance)
			{//	중간에 장애물이 있는지 체크해보자.
				NxRaycastHit kHit;

				vRayDir.normalize();

				NxRay kRay(NxVec3(vStart.x, vStart.y, vStart.z), vRayDir);
				NxShape* pkCollShape = raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, 1, NiMax(fCheckDistance, 0.1f), NX_RAYCAST_SHAPE);

				if (pkCollShape && pkCollShape != pkCollidedShapes[i])
				{
					continue;
				}
			}

			char const* shapename = pkHitShape->getName();

			NiPoint3 kCenter;
			NiPhysXTypes::NxVec3ToNiPoint3(pkHitShape->getGlobalPosition(), kCenter);

			pkActor->SetHitPoint(kCenter);
			pkActor->SetHitObjectCenterPos(kCenter);
			pkActor->SetHitObjectABVIndex(pkActor->GetABVShapeIndex(shapename));

			NiPhysXTypes::NxVec3ToNiPoint3(pkHitShape->getGlobalPosition(), vShapePos);
			vRayDir = NxVec3(vShapePos.x-vStart.x,vShapePos.y-vStart.y,vShapePos.z-vStart.z);
			kOut.AddObj(pkActor,vRayDir.magnitude());

		}
	}
}
void PgWorld::RayCheckObjectFloorSphere(PgWOPriorityQueue& kOut,int iActionNo,NiPoint3 const& vStart,NiPoint3 const& vDir, unsigned int uiGroup, float fRadius,EAttackedUnitPos kTargetPosType,bool bNoDuplication,PgActor* pkCasterActor  )
{
	if(!m_spPhysXScene)
	{
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.RayCheck"), g_pkApp->GetFrameCount()));
	//	먼저 vStart 로부터 vDir 방향으로 레이를 쏴서 바닥과 충돌하는 위치를 찾아내자.
	NxRaycastHit kFloorHit;
	NxShape* pkFloorHitShape = raycastClosestShape(NxRay(NxVec3(vStart.x,vStart.y,vStart.z),NxVec3(vDir.x,vDir.y,vDir.z)), NX_STATIC_SHAPES, kFloorHit, 1, 1000, NX_RAYCAST_SHAPE);

	if ( pkFloorHitShape == NULL)
	{
		return;
	}

	int	nbMaxShapes = 100;
	NxShape* pkCollidedShapes[100];

	int iTotalShapes = overlapSphereShapes(NxSphere(kFloorHit.worldImpact,fRadius),
		NX_DYNAMIC_SHAPES,nbMaxShapes,pkCollidedShapes,NULL,uiGroup,NULL,true);

	int	iTotal = 0;
	NxShape* pkHitShape;

	NiPoint3	vVec,vShapePos;
	for(int i=0;i<iTotalShapes;++i)
	{
		pkHitShape = pkCollidedShapes[i];
		PgActor* pkActor = (PgActor*)pkHitShape->userData;

		if (pkActor)
		{
			if(0 < iActionNo && false == PgAction::IsCorrectTargetWithTargetType(iActionNo,pkActor,pkCasterActor))
			{
				continue;
			}

			if (false == pkActor->GetCanHit() && pkActor->IsEnemy(pkCasterActor) )
			{
				continue;
			}

			bool bCorrectTarget = false;

			if (pkActor->IsDownState() && (kTargetPosType&EAttacked_Pos_ground))	{ bCorrectTarget = true; }
			if (pkActor->IsBlowUp() && (kTargetPosType&EAttacked_Pos_Blowup))		{ bCorrectTarget = true; }
			if (!pkActor->IsDownState() && !pkActor->IsBlowUp() && (kTargetPosType&EAttacked_Pos_Normal)) { bCorrectTarget = true; }

			if(!bCorrectTarget)
			{
				continue;
			}


			if (bNoDuplication && kOut.CheckExist(pkActor))
			{
				continue;
			}

			NiPhysXTypes::NxVec3ToNiPoint3(pkHitShape->getGlobalPosition(), vShapePos);
			vVec = vShapePos - vStart;
			vVec.Unitize();

			//	중간에 장애물이 있는지 체크해보자.
			NxRaycastHit kHit;
			NxVec3 vRayDir(vShapePos.x-vStart.x,vShapePos.y-vStart.y,vShapePos.z-vStart.z);

			float fCheckDistance = vRayDir.magnitude();
			PG_ASSERT_LOG(fCheckDistance > 0.0f);
			vRayDir.normalize();
			NxRay kRay(NxVec3(vStart.x, vStart.y, vStart.z), vRayDir);
			NxShape* pkCollShape = raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, 1, NiMax(fCheckDistance, 0.1f), NX_RAYCAST_SHAPE);

			if(pkCollShape && pkCollShape != pkCollidedShapes[i])
			{
				continue;
			}

			if ( pkHitShape->userData == NULL)
			{
				continue;
			}

			char const* shapename = pkHitShape->getName();

			NiPoint3 kCenter;
			NiPhysXTypes::NxVec3ToNiPoint3(pkHitShape->getGlobalPosition(), kCenter);

			int	iABVShapeIndex = pkActor->GetABVShapeIndex(shapename);

			pkActor->SetHitPoint(kCenter);
			pkActor->SetHitObjectCenterPos(kCenter);
			pkActor->SetHitObjectABVIndex(iABVShapeIndex);

			kOut.AddObj(pkActor,fCheckDistance);
		}
	}
}
int	PgWorld::overlapSphereShapes(
								 const NxSphere& worldSphere, 
								 NxShapesType shapeType, 
								 NxU32 nbShapes, 
								 NxShape** shapes, 
								 NxUserEntityReport<NxShape*>* callback, 
								 NxU32 activeGroups, 
								 const NxGroupsMask* groupsMask, 
								 bool accurateCollision)
{
	if(!m_spPhysXScene || !m_spPhysXScene->GetPhysXScene())
	{
		return	0;
	}
	if(m_dwMainThreadID != ::GetCurrentThreadId())	//	it's allowed only to mainthread
	{
		return	0;
	}


	return m_spPhysXScene->GetPhysXScene()->overlapSphereShapes(worldSphere,shapeType,nbShapes,shapes,callback,activeGroups,groupsMask,accurateCollision);
}
NxU32 PgWorld::overlapCapsuleShapes(const NxCapsule& worldCapsule, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups, const NxGroupsMask* groupsMask, bool accurateCollision)
{
	if(!m_spPhysXScene || !m_spPhysXScene->GetPhysXScene())
	{
		return	0;
	}
	if(m_dwMainThreadID != ::GetCurrentThreadId())	//	it's allowed only to mainthread
	{
		return	0;
	}

	return	m_spPhysXScene->GetPhysXScene()->overlapCapsuleShapes(worldCapsule,shapeType,nbShapes,shapes,callback,activeGroups,groupsMask,accurateCollision);
}
NxU32 PgWorld::overlapOBBShapes(const NxBox& worldBox, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups, const NxGroupsMask* groupsMask, bool accurateCollision)
{
	if(!m_spPhysXScene || !m_spPhysXScene->GetPhysXScene())
	{
		return	0;
	}
	if(m_dwMainThreadID != ::GetCurrentThreadId())	//	it's allowed only to mainthread
	{
		return	0;
	}

	return	m_spPhysXScene->GetPhysXScene()->overlapOBBShapes(worldBox,shapeType,nbShapes,shapes,callback,activeGroups,groupsMask,accurateCollision);

}

NxShape* PgWorld::raycastClosestShape(const NxRay& worldRay, const NxShapesType shapeType, NxRaycastHit& hit, const NxU32 groups, const NxReal maxDist, const NxU32 hintFlags, const NxGroupsMask* groupsMask, NxShape** cache) const
{
	if(!m_spPhysXScene || !m_spPhysXScene->GetPhysXScene())
	{
		return	NULL;
	}
	if(m_dwMainThreadID != ::GetCurrentThreadId())	//	it's allowed only to mainthread
	{
		return	NULL;
	}

	return m_spPhysXScene->GetPhysXScene()->raycastClosestShape(worldRay,shapeType,hit,groups,maxDist,hintFlags,groupsMask,cache);
}
NxU32  PgWorld::raycastAllShapes(const NxRay &worldRay, NxUserRaycastReport &report, NxShapesType shapesType, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask *groupsMask) const
{
	if(!m_spPhysXScene || !m_spPhysXScene->GetPhysXScene())
	{
		return	0;
	}
	if(m_dwMainThreadID != ::GetCurrentThreadId())	//	it's allowed only to mainthread
	{
		return	0;
	}

	return m_spPhysXScene->GetPhysXScene()->raycastAllShapes(worldRay,report,shapesType,groups,maxDist,hintFlags,groupsMask);
}

void PgWorld::RayCheckObjectWidely(PgWOPriorityQueue& kOut,int iActionNo,NiPoint3 const& vStart,NiPoint3 const& vDir, unsigned int uiGroup, float fRange,float fHorizRange,EAttackedUnitPos kTargetPosType,bool bNoDuplication,PgActor* pkCasterActor )
{
	if(!m_spPhysXScene)
	{
		return;
	}

	PG_STAT(PgStatTimerF timerA(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.RayCheck"), g_pkApp->GetFrameCount()));
	int	nbMaxShapes = 100;
	NxShape* pkCollidedShapes[100];

	int iTotalShapes = overlapSphereShapes(NxSphere(NxVec3(vStart.x,vStart.y,vStart.z),fRange),
		NX_DYNAMIC_SHAPES,nbMaxShapes,pkCollidedShapes,NULL,uiGroup,NULL,true);

	int		iSelectedShapeIndex = -1;
	float	fSelectedShapeDot = -1;
	float	fSelectedShapeDistance = 99999.0f;
	float	fDot,fDistance;
	NiPoint3 vVec, vShapePos;
	for(int i=0; i<iTotalShapes; ++i)
	{
		NiPhysXTypes::NxVec3ToNiPoint3(pkCollidedShapes[i]->getGlobalPosition(), vShapePos);
		vVec = vShapePos - vStart;
		vVec.Unitize();
		fDot = vDir.Dot(vVec);
		float fAngle = acos(fDot)*180.0f/NI_PI;
		if(fAngle>fHorizRange)
		{
			continue;
		}
		if(NULL == pkCollidedShapes[i]->userData)
		{
			continue;
		}

		PgActor* pkActor = (PgActor*)pkCollidedShapes[i]->userData;
		if (NULL == pkActor)
		{
			continue;
		}

		if(0 < iActionNo && false == PgAction::IsCorrectTargetWithTargetType(iActionNo,pkActor,pkCasterActor))
		{
			continue;
		}

		if(!pkActor->GetActorDead()) // 타겟 타입이 OnlyDead이고 실제 죽은 경우 라면 true가 되어있음
		{
			if (false == pkActor->GetCanHit())
			{
				continue;
			}
		}

		bool bCorrectTarget = false;

		if (pkActor->IsDownState() && (kTargetPosType&EAttacked_Pos_ground))	{ bCorrectTarget = true; }
		if (pkActor->IsBlowUp() && (kTargetPosType&EAttacked_Pos_Blowup))		{ bCorrectTarget = true; }
		if (!pkActor->IsDownState() && !pkActor->IsBlowUp() && (kTargetPosType&EAttacked_Pos_Normal)) { bCorrectTarget = true; }

		if(!bCorrectTarget) 
		{
			continue;
		}


		if (bNoDuplication && kOut.CheckExist(pkActor))
		{
			continue;
		}

		fDistance = (vStart-vShapePos).Length();
		if(fDistance < fSelectedShapeDistance)
		{
			//	충돌한 물체가 vStart 보다 뒤에 있으면 안된다.
			NiPoint3 vShapeDir = vShapePos-vStart;
			vShapeDir.Unitize();
			if(0 > vShapeDir.Dot(vDir))
			{
				continue;
			}

			//	중간에 장애물이 있는지 체크해보자.
			NxRaycastHit kHit;
			NxVec3 vRayDir(vShapePos.x-vStart.x,vShapePos.y-vStart.y,vShapePos.z-vStart.z);

			float fCheckDistance = vRayDir.magnitude();
			PG_ASSERT_LOG(fCheckDistance > 0.0f);

			vRayDir.normalize();
			NxRay kRay(NxVec3(vStart.x, vStart.y, vStart.z), vRayDir);
			NxShape* pkCollShape = raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, 1, NiMax(fCheckDistance, 0.1f), NX_RAYCAST_SHAPE);

			if(pkCollShape && pkCollShape != pkCollidedShapes[i])
			{
				continue;
			}

			float fAngle = acos(fDot)*180.0f/NI_PI;
			if(fAngle>fHorizRange)
			{
				continue;
			}

			NxShape* pkHitShape = pkCollidedShapes[i];

			char const* shapename = pkHitShape->getName();

			NiPoint3 kCenter;
			NiPhysXTypes::NxVec3ToNiPoint3(pkHitShape->getGlobalPosition(), kCenter);

			int	iABVShapeIndex = pkActor->GetABVShapeIndex(shapename);

			pkActor->SetHitPoint(kCenter);
			pkActor->SetHitObjectCenterPos(kCenter);
			pkActor->SetHitObjectABVIndex(iABVShapeIndex);

			kOut.AddObj(pkActor,fDistance);
		}
	}
}

void PgWorld::RayCheckObjectBar(PgWOPriorityQueue& kOut,int iActionNo,NiPoint3 const& vStart,NiPoint3 const& vDir,unsigned int uiGroup, float fEdgeLength,float fRange,EAttackedUnitPos kTargetPosType,bool bNoDuplication,PgActor* pkCasterActor )
{
	if(!m_spPhysXScene)
	{
		return;
	}

	int	nbMaxShapes = 100;
	NxShape* pkCollidedShapes[100];

	NxVec3	kCenter;

	kCenter.x = vStart.x+vDir.x*fRange/2;
	kCenter.y = vStart.y+vDir.y*fRange/2;
	kCenter.z = vStart.z+vDir.z*fRange/2;

	NxVec3	kExtend;

	kExtend.x = fRange/2;
	kExtend.y = fEdgeLength/2;
	kExtend.z = fEdgeLength/2;

	NxMat33	vRot;

	NxVec3	vDirection(vDir.x,vDir.y,vDir.z);
	NxVec3	vRight,vUp;
	vRight = vDirection.cross(NxVec3(0,0,1));

	if(vRight.distance(NxVec3(0,0,0))<0.001)
	{
		vRight = vDirection.cross(NxVec3(0,1,0));
	}

	vUp = vDirection.cross(-vRight);

	vRot.setColumn(0,vDirection);
	vRot.setColumn(1,vUp);
	vRot.setColumn(2,vRight);

	NxBox kBar(kCenter,kExtend,vRot);

	PG_STAT(PgStatTimerF timerA(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.RayCheck"), g_pkApp->GetFrameCount()));

	int iTotalShapes = overlapOBBShapes(kBar,NX_DYNAMIC_SHAPES,nbMaxShapes,pkCollidedShapes,NULL,uiGroup,NULL,true);

	NiPlane	kTestPlane(vDir,vStart);

	NiPoint3 vShapePos,vMax;
	NxBounds3	kBounds;
	float	fRadius=0;
	for(int i=0;i<iTotalShapes;++i)
	{
		NxShape* pkHitShape = pkCollidedShapes[i];
		if (pkHitShape && pkHitShape->userData)
		{
			PgActor* pkActor = (PgActor*)pkHitShape->userData;

			if(0 < iActionNo && false == PgAction::IsCorrectTargetWithTargetType(iActionNo,pkActor,pkCasterActor))
			{
				continue;
			}

			if (false == pkActor->GetCanHit() && pkActor->IsEnemy(pkCasterActor))
			{
				continue;
			}

			bool bCorrectTarget = false;

			if (pkActor->IsDownState() && (kTargetPosType&EAttacked_Pos_ground))	{ bCorrectTarget = true; }
			if (pkActor->IsBlowUp() && (kTargetPosType&EAttacked_Pos_Blowup))		{ bCorrectTarget = true; }
			if (!pkActor->IsDownState() && !pkActor->IsBlowUp() && (kTargetPosType&EAttacked_Pos_Normal)) { bCorrectTarget = true; }

			if(!bCorrectTarget)
			{
				continue;
			}

			if (bNoDuplication && kOut.CheckExist(pkActor)) 
			{
				continue;
			}

			NiPhysXTypes::NxVec3ToNiPoint3(pkCollidedShapes[i]->getGlobalPosition(), vShapePos);

			NxVec3 vRayDir(vShapePos.x-vStart.x,vShapePos.y-vStart.y,vShapePos.z-vStart.z);
			float fCheckDistance = vRayDir.magnitude();
			PG_ASSERT_LOG(fCheckDistance > 0.0f);

			//	중간에 장애물이 있는지 체크해보자.
			NxRaycastHit kHit;

			vRayDir.normalize();
			NxRay kRay(NxVec3(vStart.x, vStart.y, vStart.z), vRayDir);
			NxShape* pkCollShape = raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, 1, NiMax(fCheckDistance, 0.1f), NX_RAYCAST_SHAPE);

			if (pkCollShape && pkCollShape != pkCollidedShapes[i])
			{
				continue;
			}

			char const* shapename = pkHitShape->getName();

			NiPoint3 kCenter;
			NiPhysXTypes::NxVec3ToNiPoint3(pkHitShape->getGlobalPosition(), kCenter);

			pkActor->SetHitPoint(kCenter);
			pkActor->SetHitObjectCenterPos(kCenter);
			pkActor->SetHitObjectABVIndex(pkActor->GetABVShapeIndex(shapename));

			NiPhysXTypes::NxVec3ToNiPoint3(pkHitShape->getGlobalPosition(), vShapePos);
			vRayDir = NxVec3(vShapePos.x-vStart.x,vShapePos.y-vStart.y,vShapePos.z-vStart.z);
			kOut.AddObj(pkActor,vRayDir.magnitude());

		}
	}
}
PgIWorldObject *PgWorld::RayCheckObject(NiPoint3 const& kStart, int iActionNo, NiPoint3 const& kDir, unsigned int uiObjectGroup, float fRange, PgIWorldObject *pkSelf)
{
	if(!m_spPhysXScene)
	{
		return NULL;
	}
	PG_ASSERT_LOG(fRange > 0.0f);

	PG_STAT(PgStatTimerF timerA(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.RayCheck"), g_pkApp->GetFrameCount()));
	NxRaycastHit kHit;
	NxRay kRay(NxVec3(kStart.x, kStart.y, kStart.z), NxVec3(kDir.x, kDir.y, kDir.z));
	NxShape* pkHitShape = raycastClosestShape(kRay, NX_DYNAMIC_SHAPES, kHit, 1 << (uiObjectGroup + 1), NiMax(fRange, 0.1f));

	if(pkHitShape && pkHitShape->userData)
	{
		char const* shapename = pkHitShape->getName();
		PgIWorldObject *pkWObj = (PgIWorldObject *)pkHitShape->userData;

		NiPoint3 kImpact,kCenter;
		NiPhysXTypes::NxVec3ToNiPoint3(kHit.worldImpact, kImpact);
		NiPhysXTypes::NxVec3ToNiPoint3(pkHitShape->getGlobalPosition(), kCenter);
		pkWObj->SetHitPoint(kImpact);
		pkWObj->SetHitObjectCenterPos(kCenter);
		pkWObj->SetHitObjectABVIndex(((PgActor*)pkWObj)->GetABVShapeIndex(shapename));

		return pkWObj;
	}

	return 0;
}

bool PgWorld::LoadNif(char const* pcNifPath)
{
	if (pcNifPath == NULL)
		return false;

	m_bLoadGsa = false;
	g_kParticleMan.ParseXml("effect.xml");
	NiStream kStream;

	if (!kStream.Load(pcNifPath))
	{
		NILOG(PGLOG_ERROR, "[PgWorld] Load %s file faild\n", pcNifPath);
		return NULL;
	}

	if (kStream.GetObjectCount() == 0)
	{
		NILOG(PGLOG_ERROR, "[PgWorld] Load %s file but no object\n", pcNifPath);
		return NULL;
	}

	NiAVObject *pkRoot = NiDynamicCast(NiAVObject, kStream.GetObjectAt(0));

	if(!pkRoot)
	{
		PG_ASSERT_LOG(!"no root node");
		return false;
	}
	m_spSceneRoot = (NiNode *)pkRoot;
	//m_spSceneRoot = NiNew NiNode();

	m_pkRoomGroupRoot = NiNew PgPSRoomGroup();
	m_spSceneRoot->AttachChild(m_pkRoomGroupRoot);

	NiNode	*pkStaticNodeRoot = NiNew NiNode();
	PG_ASSERT_LOG(pkStaticNodeRoot);
	pkStaticNodeRoot->SetWorldTranslate(NiPoint3(0, 0, 0));
	pkStaticNodeRoot->SetName("STATIC_NODE_ROOT");
	m_spSceneRoot->AttachChild(pkStaticNodeRoot, true);
	m_pkStaticNodeRoot = pkStaticNodeRoot;
	m_spSceneRoot->SetName("DYNAMIC_NODE_ROOT");
	m_pkDynamicNodeRoot = m_spSceneRoot;
	m_spSceneRoot->UpdateNodeBound();
	//	NiNode	*pkDynamicNodeRoot = NiNew NiNode();
	//	pkDynamicNodeRoot->SetName("DYNAMIC_NODE_ROOT");
	//	m_spSceneRoot->AttachChild(pkDynamicNodeRoot, true);

	//pkDynamicNodeRoot->AttachChild(pkRoot, true);


	bool bSelectiveUpdate;
	bool bRigid;
	m_spSceneRoot->SetSelectiveUpdateFlags(bSelectiveUpdate, false, bRigid);
	m_spSceneRoot->UpdateNodeBound();
	m_spSceneRoot->UpdateProperties();
	m_spSceneRoot->UpdateEffects();
	m_spSceneRoot->Update(0.0f);

	DefaultNodeSetting(m_spSceneRoot);

	// 광들
	m_spLightRoot = NiNew NiNode;
	{
		NiTPointerList<NiNodePtr> kStack;
		kStack.AddTail(m_spSceneRoot);
		while(!kStack.IsEmpty())
		{
			NiNodePtr spNode = kStack.RemoveTail();

			for(unsigned int i = 0; i < spNode->GetArrayCount(); ++i)
			{
				NiAVObject *pkObject = spNode->GetAt(i);

				if(NiIsKindOf(NiNode, pkObject))
				{
					kStack.AddTail((NiNode *)pkObject);
				}
				else if(NiIsKindOf(NiLight, pkObject))
				{
					pkObject->SetLocalTransform(pkObject->GetWorldTransform());
					m_spLightRoot->AttachChild(pkObject, true);
				}
			}
		}
	}
	m_spSceneRoot->AttachChild(m_spLightRoot, true);

	NewWare::Scene::ApplyTraversal::Property::SetAlphaGroup( m_spSceneRoot, AG_EFFECT );

	// 카메라를 추출한다. (렌즈 등을 max 설정으로 바로 사용한다.)
	NiCamera* pkToMainCam = 0;
	unsigned int cnt = kStream.GetObjectCount();
	for (unsigned int ui = 1; ui < cnt; ++ui)
	{
		NiObject *pkObject = kStream.GetObjectAt(ui);

		// 기본 카메라를 설정한다.
		if (NiIsKindOf(NiCamera, pkObject))
		{
			NiCameraPtr spCamera = (NiCamera *)kStream.GetObjectAt(ui);
			NiNode* pkCameraParent = spCamera->GetParent();
			spCamera->SetName(pkCameraParent->GetName());

			// 카메라를 카메라맨에 등록한다.
			m_kCameraMan.SetCamera(spCamera);
			m_kCameraMan.AddCamera((NiString)spCamera->GetName(), spCamera);
			m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_NONE, 0);

			// 카메라를 씬에 등록한다.
			NiPoint3 kLoc = spCamera->GetWorldTranslate();
			NiMatrix3 kRot = spCamera->GetWorldRotate();
			m_spSceneRoot->AttachChild(spCamera, true);
			m_spSceneRoot->DetachChild(pkCameraParent);
			spCamera->SetTranslate(kLoc);
			spCamera->SetRotate(kRot);

			pkToMainCam = spCamera;
		}
		// 물리 노드를 설정한다.
		else if(NiIsKindOf(NiPhysXScene, pkObject))
		{
			if (m_spPhysXScene)
			{
				m_spPhysXScene->ReleaseSnapshot();
				m_spPhysXScene->ClearSceneFromSnapshot();
				m_spPhysXScene->ReleaseScene();
				m_spPhysXScene = 0;
			}
			m_spPhysXScene = (NiPhysXScene *)pkObject;
		}
	}

	// 기본 카메라를 만든다.
	if (pkToMainCam)
	{
		NiCameraPtr spCamera = NiDynamicCast(NiCamera, pkToMainCam->CreateDeepCopy());
		char const* pcMainCamName = "Def_MainCamera";
		spCamera->SetName(pcMainCamName);

		// Culling 거리 조절
		NiFrustum kFrustum = spCamera->GetViewFrustum();
		kFrustum.m_fNear = 100.0f;
		kFrustum.m_fFar = MAX_FAR_PLANE;
		kFrustum.m_fLeft = -0.51428568f;
		kFrustum.m_fRight = 0.51428568f;
		kFrustum.m_fTop = 0.38571426f;
		kFrustum.m_fBottom = -0.38571426f;
		g_kFrustum = kFrustum;
		//spCamera->SetViewFrustum(kFrustum);

		// 카메라를 카메라맨에 등록한다.
		m_kCameraMan.SetCamera(spCamera);
		m_kCameraMan.AddCamera((NiString)pcMainCamName, spCamera);
		m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_NONE, 0);

		// 카메라를 씬에 등록한다.
		m_spSceneRoot->AttachChild(spCamera, true);
	}

	// 월드에 물리 시스템을 설정한다.
	if(!LoadPhysX())
	{
		PG_ASSERT_LOG(!"failed to loading physX");
		return false;
	}

	//if (!CheckRootNode(m_spSceneRoot))
	//{
	//	return false;
	//}

	m_spSceneRoot->UpdateNodeBound();
	m_spSceneRoot->UpdateProperties();
	m_spSceneRoot->UpdateEffects();
	m_spSceneRoot->Update(0.0f);
	return true;
}
bool	PgWorld::Update(float fAccumTime,float fFrameTime)
{
	UpdateSlowMotion(fFrameTime);

	m_fFrameTime	=	fFrameTime*m_fUpdateSpeed;
	m_fAccumTime	=	m_fAccumTime+m_fFrameTime;

	SaveServerElapsedTime();

	/*	float	fFPS = (float)g_pkApp->m_DebugInfo.m_iUPS;
	float	fSPF	=	1.0f/fFPS;

	if(m_fLastUpdateTime == 0)
	{
	m_fLastUpdateTime = m_fAccumTime;
	m_fSavedUpdateTime = 0.0f;
	}

	float	fElapsedTime = m_fAccumTime - m_fLastUpdateTime + m_fSavedUpdateTime;

	m_fLastUpdateTime = m_fAccumTime;
	if(fElapsedTime<fSPF)
	{
	m_fSavedUpdateTime=fElapsedTime;
	}
	else
	{*/

	if ( IsUpdate() )
	{
		UpdateScene(m_fAccumTime, m_fFrameTime);
	}

	UpdateEffectRemainTime();
	
	//	m_fSavedUpdateTime=0;
	//}

	return	true;
}

void PgWorld::processRemoveObjectList(unsigned int iProcessCount)
{
	unsigned int iCount = 0;
	if (iProcessCount == 0)
		iCount = m_kRemoveObjectContainer.size();
	else
		iCount = __min(iProcessCount, m_kRemoveObjectContainer.size());

	if(m_kRemoveObjectContainer.size()>0)
	{
		_PgOutputDebugString("PgWorld::processRemoveObjectList iProcessCount:%d iCount:%d Size : %d\n",iProcessCount,iCount,m_kRemoveObjectContainer.size());
	}

	RemoveObjectContainer::iterator itrE = m_kRemoveObjectContainer.begin();
	for (unsigned int i = 0 ; i < iCount && itrE != m_kRemoveObjectContainer.end(); ++i)
	{
		if (RemoveObject(*itrE))
		{
			_PgOutputDebugString("processRemoveObjectList %s success\n", MB(itrE->str()));
			itrE = m_kRemoveObjectContainer.erase(itrE);
		}
		else
		{
			if (g_kPilotMan.RemoveReservedPilot(*itrE))
			{
				_PgOutputDebugString("processRemoveObjectList %s from reserved list\n", MB(itrE->str()));
				itrE = m_kRemoveObjectContainer.erase(itrE);				
			}
			else
			{
				// QUESTION: RemoveObject도 안되고 RemoveReservedPilot도 안되면 넌 누구냐; -_-
				// 일단 다음 프레임에도 지우려고 시도를 하는게 좋을까?
				_PgOutputDebugString("processRemoveObjectList %s failed\n", MB(itrE->str()));
				itrE = m_kRemoveObjectContainer.erase(itrE); //++itrE;				
			}
		}		
	}
}
void PgWorld::processAddUnitQueue(unsigned int iProcessCount)
{
	unsigned int iCount = 0;
	if (iProcessCount == 0)
		iCount = m_kAddUnitQueue.size();
	else
		iCount = __min(iProcessCount, m_kAddUnitQueue.size());

	if (g_iUseAddUnitThread == 1 && iCount > 0)
	{
		_PgOutputDebugString("[PgWorld] %d frame, processAddUnitQueue Start(%d,%d)\n", g_pkApp->GetFrameCount(), iCount, m_kAddUnitQueue.size());
		m_kAddUnitQueueLock.Lock();
		AddUnitQueue::iterator iter = m_kAddUnitQueue.begin();
		for (unsigned int i = 0 ; i < iCount && iter != m_kAddUnitQueue.end(); ++i)
		{
			PG_ASSERT_LOG(iter->pkUnit);
			PG_ASSERT_LOG(iter->pkPilot);
			if (g_kPilotMan.IsReservedPilot(iter->pkUnit->GetID()))
			{
				PgIWorldObject* pObject = AddUnit(iter->pkUnit, iter->pkPilot,iter->fPacketReceiveTime);
				if (pObject != NULL)
				{
					g_kPilotMan.FlushReservedPilotPacket(iter->pkUnit->GetID());
					g_kPilotMan.RemoveReservedPilot(iter->pkUnit->GetID(), false);
					if (pObject->IsMyActor())
					{
						// 내가 world에 Add가 되면 Loading화면을 끈다. - 차후에 MapMoveCompelete에서 끄게 될지도.
						PgScripting::DoString("UI_MapMoveComplete()");
					}
				}
				else
				{
					DeleteAddUnitInfo(iter->pkUnit, iter->pkPilot);
					NILOG(PGLOG_ERROR, "[PgWorld] AddUnit(%s,%d) while update Failed\n", MB(iter->pkUnit->GetID().str()), iter->pkUnit->UnitType());
				}				
			}
			else
			{
				DeleteAddUnitInfo(iter->pkUnit, iter->pkPilot);
			}

			iter = m_kAddUnitQueue.erase(iter);
		}
		_PgOutputDebugString("processAddUnitQueue End(%d)\n", m_kAddUnitQueue.size());
		m_kAddUnitQueueLock.Unlock();
	}
}

bool PgWorld::UpdateScene(float fAccumTime, float fFrameTime)
{
#ifndef EXTERNAL_RELEASE
	if(!lua_tinker::call<bool>("UpdateWorld"))
		return true;
#endif

	PG_STAT(PgStatTimerF timerA(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.processList"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerB(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.UpdateScript"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerC(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.ObjectUpdate"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerD(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.EffectUpdate"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerE(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.FrontObject"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerF(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.RemoveEffect"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerG(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.PhysX"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerH(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.SceneRoot"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerH1(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.SceneRoot.Sel"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerH2(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.SceneRoot.Dyn"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerI(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.Particle"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerJ(g_kWorldStatGroup.GetStatInfo("PgWorld.UpdateScene.Last"), g_pkApp->GetFrameCount()));
#ifndef EXTERNAL_RELEASE
	if (m_iDraw == 1 || m_iDraw == 3)
	{
#endif


		/*static	int	iUpdate = 0;
		iUpdate++;
		if(iUpdate==5)
		{
		iUpdate = 0;
		}
		else
		return true;*/

		//if(lua_tinker::call<bool>("UpdateUnitQueue"))
		{
			// 대기큐에 있는 오브젝트를 삭제한다. 
			// 한프레임에 하나씩만 삭제를 하도록 한다.
			PG_STAT(timerA.Start());
			processRemoveObjectList(1);
			processUnfreezeObjectList(1);
			processAddUnitQueue(1);
			PG_STAT(timerA.Stop());
		}

		UpdateItemEquipInfo();

#ifndef EXTERNAL_RELEASE
		if(lua_tinker::call<bool>("UpdateWorldUpdateScript"))
#endif
		{
			PG_STAT(timerB.Start());
			if(!m_kUpdateScript.empty())
			{
				lua_tinker::call<bool, lwWorld, float>(m_kUpdateScript.c_str(), (lwWorld)this, fAccumTime);
			}
			PG_STAT(timerB.Stop());
		}

		//	이벤트 스크립트 시스템 처리
		g_kEventScriptSystem.Update(fAccumTime,fFrameTime);
		g_kEnvironmentMan.Update(m_kCameraMan.GetCamera(),fAccumTime,fFrameTime);

		PG_STAT(timerC.Start());
#ifndef EXTERNAL_RELEASE
		m_fObjectUpdateTime = NiGetCurrentTimeInSec();
#endif

#ifndef EXTERNAL_RELEASE
		if(lua_tinker::call<bool>("UpdateWorldObject"))
#endif
		{
			// 월드 오브젝트를 최신 상태로 갱신합니다.
			for(int i=0;i<MAX_OBJ_CONT;++i)
			{
				for(ObjectContainer::iterator itr = m_kObjectContainer[i].begin();
					itr != m_kObjectContainer[i].end();
					++itr)
				{
					itr->second->Update(fAccumTime, fFrameTime);
				}
			}

		}

		// Trigger 오브젝트를 최신 상태로 갱신합니다.
		TriggerContainer::iterator itr_trigger = m_kTriggerContainer.begin();
		while(itr_trigger != m_kTriggerContainer.end())
		{
			itr_trigger->second->Update(fAccumTime, fFrameTime);
			++itr_trigger;
		}
		//UpdateMapKfmObject(fAccumTime);
#ifndef EXTERNAL_RELEASE
		m_fObjectUpdateTime = NiGetCurrentTimeInSec() - m_fObjectUpdateTime;
#endif
		PG_STAT(timerC.Stop());

		PG_STAT(timerG.Start());

#ifndef EXTERNAL_RELEASE
		if(lua_tinker::call<bool>("UpdatePhysX"))
#endif
		{
			float fStartTime = NiGetCurrentTimeInSec();
			// Gamebryo --> PhysX
			m_spPhysXScene->UpdateSources(fAccumTime);

			LockPhysX(true);

			// PhysX를 시물레이션한다.
			m_spPhysXScene->Simulate(fAccumTime);

			// 시뮬레이션 결과를 받아온다.
			m_spPhysXScene->FetchResults(fAccumTime, true);

			// updateController는 FetchResult이후에 호출 해준다.
			g_kControllerManager.updateControllers();

			// PhysX --> Gamebryo 동기화
			m_spPhysXScene->UpdateDestinations(fAccumTime);

			LockPhysX(false);

			g_fSimulationTime += (NiGetCurrentTimeInSec() - fStartTime);
			++g_dwFrameCount;
		}

		PG_STAT(timerG.Stop());

		ProcessParticle(fFrameTime);

		PG_STAT(timerH.Start());
		float	fStartTime = NiGetCurrentTimeInSec();
#ifndef EXTERNAL_RELEASE
		if(lua_tinker::call<bool>("UpdateSelectiveRoot"))
#endif
		{
			NiNode	*pkSelective = GetSelectiveNodeRoot();
			if (pkSelective)
			{
				//if(lua_tinker::call<bool>("UpdateSelected"))
				//	pkSelective->UpdateSelected(fAccumTime);//lua_tinker::call<bool>("UpdateController"));
				//else
				//	pkSelective->Update(fAccumTime,lua_tinker::call<bool>("UpdateController"));
				PG_STAT(timerH1.Start());
				UpdateSelected(fAccumTime,m_kCameraMan.GetCamera(),pkSelective);
				PG_STAT(timerH1.Stop());
			}
		}

		NiNode	*pkDynamic = GetDynamicNodeRoot();
#ifndef EXTERNAL_RELEASE
		if(lua_tinker::call<bool>("UpdateDynamicRoot"))
#endif
		{
			if(m_spMultiThreadSceneGraphUpdater)
			{
				m_spMultiThreadSceneGraphUpdater->SetTime(fAccumTime,fFrameTime);
				for(int i=0;i<OGT_MAX;++i)
				{
					int	iChildCount = m_aspObjectGroups[i]->GetArrayCount();
					for(int j=0;j<iChildCount;++j)
					{
						NiAVObject	*pkAVObject = m_aspObjectGroups[i]->GetAt(j);
						if(pkAVObject)
						{
							m_spMultiThreadSceneGraphUpdater->UpdateObject(pkAVObject);
						}
					}
				}
				m_spMultiThreadSceneGraphUpdater->WaitFinish();
			}
			else
			{
				for(int i=0;i<OGT_MAX;++i)
				{
					int	iChildCount = m_aspObjectGroups[i]->GetArrayCount();
					for(int j=0;j<iChildCount;++j)
					{
						NiAVObject	*pkAVObject = m_aspObjectGroups[i]->GetAt(j);
						if(pkAVObject)
						{
							pkAVObject->Update(fAccumTime);
						}
					}
				}
			}
			//if (pkDynamic)
			//{
			//	PG_STAT(timerH2.Start());
			//	pkDynamic->Update(fAccumTime,true);
			//	PG_STAT(timerH2.Stop());
			//}

		}
#ifndef EXTERNAL_RELEASE
	}
#endif

#ifndef EXTERNAL_RELEASE
	if(lua_tinker::call<bool>("UpdateCamera"))
#endif
	{
		m_kCameraMan.Update(fFrameTime);
	}

#ifndef EXTERNAL_RELEASE
	if (m_iDraw == 1 || m_iDraw == 3)
	{
#endif

		if(GetPSRoomGroupRoot())
		{
			GetPSRoomGroupRoot()->UpdateSelective(m_kCameraMan.GetCamera(),fAccumTime);
		}

		//pkDynamic->UpdateNodeBound();
		//m_spSceneRoot->Update(fAccumTime);

		PG_STAT(timerH.Stop());


		PG_STAT(timerI.Start());
#ifndef EXTERNAL_RELEASE
		if(lua_tinker::call<bool>("UpdateParticlePos"))
#endif
		{
			// 월드 오브젝트 파티클 위치 맞추기
			for(int i=0;i<MAX_OBJ_CONT;++i)
			{
				for(ObjectContainer::iterator itr = m_kObjectContainer[i].begin();
					itr != m_kObjectContainer[i].end();
					++itr)
				{
					itr->second->UpdateParticlesTransform(fAccumTime, fFrameTime);
				}
			}
		}

		if(m_pkDamageNumMan)
		{
			m_pkDamageNumMan->Update(fAccumTime,fFrameTime);
		}
		g_kLinkedPlaneGroupMan.Update(fAccumTime,fFrameTime);
		g_kHeadBuffIconListMgr.Update(fAccumTime,fFrameTime);
		g_kFxStudio.Update(fFrameTime);

		// 분위기를 바꾼다.
		//if(m_pkMoodMan)
		//{
		//	m_pkMoodMan->UpdateMood(fAccumTime, fFrameTime);
		//}
		PG_STAT(timerI.Stop());

		PG_STAT(timerJ.Start());
		PgIWorldObject *pkWorldObject = g_kPilotMan.GetPlayerActor();
		if (pkWorldObject)
		{
			NiPoint3 kLoc = pkWorldObject->GetTranslate();
			g_kPlayerLoc.x = kLoc.x;
			g_kPlayerLoc.y = kLoc.y;
			g_kPlayerLoc.z = kLoc.z;
		}

		// 월드 액션 업데이트
		if(m_pWorldActionMan)
			m_pWorldActionMan->Update(fAccumTime,fFrameTime);

		//	프로젝틸 업데이트

#ifndef EXTERNAL_RELEASE
		if(lua_tinker::call<bool>("UpdateProjectile"))
#endif
		{
			g_kProjectileMan.Update(fAccumTime,fFrameTime);
		}

		// 홈
		if(m_pkHome)
		{
			m_pkHome->Update(fAccumTime, fFrameTime);
		}

		// 홈타운
		if(IsHaveAttr(GATTR_HOMETOWN))
		{
			BM::GUID kGuid;
			if( g_kPilotMan.GetPlayerPilotGuid(kGuid) )
			{
				PgPilot* pkPilot = g_kPilotMan.FindHouse(kGuid);
				if( g_pkWorld && pkPilot )
				{
					PgHouse* pkHouse = dynamic_cast<PgHouse*>(g_pkWorld->FindObject(pkPilot->GetGuid()));
					if( pkHouse )
					{
						pkHouse->UpdateHouse(fAccumTime, fFrameTime);
					}
				}
			}
		}

		UpdateFocusFilter(fAccumTime,fFrameTime);

		PG_STAT(timerJ.Stop());
#ifndef EXTERNAL_RELEASE
	}
#endif
#ifndef EXTERNAL_RELEASE

	if(lua_tinker::call<bool>("UpdateTemporaryInput"))
		processTemporaryInput();
#endif

	//미션 결과 업데이트
	g_kMissionComplete.Update(fAccumTime,fFrameTime);
	g_kBossComplete.Update(fAccumTime,fFrameTime);
	g_kExpeditionComplete.Update(fAccumTime,fFrameTime);
	g_kOBMode.Update( fAccumTime, fFrameTime );

	if ( PgContentsBase::ms_pkContents )
	{
		if ( !PgContentsBase::ms_pkContents->Update(fAccumTime,fFrameTime) )
		{
			SAFE_DELETE(PgContentsBase::ms_pkContents);
		}
	}

	NiCamera* pkCamera = m_kCameraMan.GetCamera();
#ifndef USE_INB
	g_kSkillTargetMan.Update(fAccumTime);
	g_kMonSkillTargetMan.Update(fAccumTime);
#endif//USE_INB


	//PgAVObjectAlphaProcessManager 이용한 알파처리
	//거리에 따라서 나타나거나 사라지는 오브젝트들을 알파처리 시켜서 나타나거나 사라지게 해줌
#ifndef EXTERNAL_RELEASE
	if(lua_tinker::call<bool>("UpdateAlphaProcess"))
#endif
	{
		CullAlphaProcessUpdate(fAccumTime,fFrameTime);

		if (GET_OPTION_WORLD_QUALITY == false)
		{
			cullObjectByRange(pkCamera, m_kOptimizeMidCont, m_afWorldQualityRange[0]);
			cullObjectByRange(pkCamera, m_kOptimizeLowCont, m_afWorldQualityRange[0]);
		}
		else
		{
			cullObjectByRange(pkCamera, m_kOptimizeMidCont, m_afWorldQualityRange[1]);
			cullObjectByRange(pkCamera, m_kOptimizeLowCont, m_afWorldQualityRange[0]);
		}
	}

	UpdateCurrentRemoteObjcet();
	//--------
	return true;
}

void PgWorld::Draw( PgRenderer* pkRenderer, float fFrameTime )
{
    PROFILE_FUNC();

#ifndef EXTERNAL_RELEASE
    if ( m_iDraw < 1 )
        return;
#endif

    using NewWare::Renderer::DrawWorkflow;
    using NewWare::Renderer::StreamPack;
    namespace OptionFlag = NewWare::OptionFlag;

    GetInstanceProxy(DrawWorkflow)->Execute( fFrameTime, 
        StreamPack(m_kBGColor, OptionFlag::EnableDumpScene(), GetAccumTime(), g_bUseProjectionShadow, 
                   m_kFocusFilterInfo.bRenderObject, m_kCameraMan.GetCamera()) );
}

void	PgWorld::CreateProjectionMap()
{
	int	iWidth = 1024,iHeight = 1024;
	NiTexture::FormatPrefs kPrefs;
	kPrefs.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;

	m_spProjectionTargetMap  = NiRenderedTexture::Create(iWidth,iHeight,NiRenderer::GetRenderer(),kPrefs);
	m_spProjectionTargetGroup = NiRenderTargetGroup::Create(m_spProjectionTargetMap->GetBuffer(), NiRenderer::GetRenderer(), true, true);

	m_spProjectionEffect = NiNew NiTextureEffect;
	m_spProjectionEffect->SetEffectTexture(m_spProjectionTargetMap);

}

void	PgWorld::AddWorldText(NiPoint3 const& kCenterPos,std::wstring const& kText,float fScale,const NiColorA &kColor,std::wstring const& kFontName)
{
	m_WorldTextCont.push_back(NiNew stWorldText(kCenterPos,kText,fScale,kColor,kFontName));
}
void	PgWorld::DelWorldText(NiPoint3 const& kCenterPos,std::wstring const& kText,float fScale,const NiColorA &kColor,std::wstring const& kFontName)
{
	stWorldText const kTemp(kCenterPos,kText,fScale,kColor,kFontName);
	WorldTextCont::iterator find_iter = std::find_if(m_WorldTextCont.begin(), m_WorldTextCont.end(), PgWorldUtil::SWorldTextCompare(kTemp));
	if( m_WorldTextCont.end() != find_iter )
	{
		if((*find_iter) != NULL)
		{
			delete (*find_iter);
		}
		m_WorldTextCont.erase(find_iter);
	}
}
void	PgWorld::DrawWorldText(PgRenderer *pkRenderer,NiCameraPtr spCamera)
{
	stWorldText	*pkWorldText;
	for(WorldTextCont::iterator itor = m_WorldTextCont.begin(); itor != m_WorldTextCont.end(); ++itor)
	{
		pkWorldText = *itor;
		pkWorldText->Draw(pkRenderer,spCamera);
	}
}
void	PgWorld::ClearAllWorldText()
{
	for(WorldTextCont::iterator itor = m_WorldTextCont.begin(); itor != m_WorldTextCont.end(); ++itor)
	{
		SAFE_DELETE_NI(*itor);
	}
	m_WorldTextCont.clear();
}

void	PgWorld::DestroyProjectionMap()
{
	m_spProjectionEffect = 0;
	m_spProjectionTargetMap = 0;
	m_spProjectionTargetGroup = 0;
}
//!	노드와 하위노드들에게 프로젝션 맵을 설정해준다.
void	PgWorld::ApplyProjectionMap(NiAVObject* 	pkObject)
{
	NiNode	*pkNode = NiDynamicCast(NiNode,pkObject);
	if(pkNode)
	{
		m_spProjectionEffect->AttachAffectedNode(pkNode);

		int iChild = pkNode->GetArrayCount();
		for(int i=0;i<iChild;++i)
		{
			ApplyProjectionMap(pkNode->GetAt(i));
		}
	}
}
void	PgWorld::DrawProjectionMap(PgRenderer *pkRenderer, NiCameraPtr spCamera,float fFrameTime)
{
	//	내 플레이어를 가져오자.
}

void PgWorld::DrawObjects(PgRenderer* pkRenderer, NiCamera* pkCamera, float const fFrameTime)
{
    bool bDrawObject = true;
    bool const bFilterEnable = (0 != m_kDrawActorFilter.size());

    for ( unsigned int ui = 0; ui < MAX_OBJ_CONT; ++ui )
    {
        for ( PgWorld::ObjectContainer::iterator iter = m_kObjectContainer[ui].begin(); 
              iter != m_kObjectContainer[ui].end(); ++iter )
        {
            if ( bFilterEnable )
            {
                if ( NiIsKindOf(PgActor, iter->second) )
                {
                    BM::GUID const& kActorGuid = iter->second->GetGuid();
                    bDrawObject = 
                        (m_kDrawActorFilter.end() != 
                        m_kDrawActorFilter.find(kActorGuid));
                }
                else
                {
                    bDrawObject = true; // Actor 가 아니면 그리자
                }
            }

            if ( !bFilterEnable || bDrawObject )
            {
                iter->second->Draw( pkRenderer, pkCamera, fFrameTime );
            }
        }
    }
}

void PgWorld::DrawImmediateActor(PgRenderer* pkRenderer, NiCamera* pkCamera, float const fFrameTime)
{
    bool bDrawObject = true;
    bool const bFilterEnable = (0 != m_kDrawActorFilter.size());

    for ( unsigned int ui = 0; ui < MAX_OBJ_CONT; ++ui )
    {
        for ( PgWorld::ObjectContainer::iterator iter = m_kObjectContainer[ui].begin(); 
              iter != m_kObjectContainer[ui].end(); ++iter )
        {
            if ( bFilterEnable )
            {
                if ( NiIsKindOf(PgActor, iter->second) )
                {
                    BM::GUID const& kActorGuid = iter->second->GetGuid();
                    bDrawObject = 
                        (m_kDrawActorFilter.end() != 
                         m_kDrawActorFilter.find(kActorGuid));
                }
                else
                {
                    bDrawObject = true; // Actor 가 아니면 그리자
                }
            }

            if ( !bFilterEnable || bDrawObject )
            {
                iter->second->DrawImmediate( pkRenderer, pkCamera, fFrameTime );
            }
        }
    }
}

void PgWorld::DrawNoZTest(PgRenderer* pkRenderer, NiCamera* pkCamera, float const fFrameTime)
{
	NewWare::Renderer::DrawUtils::EnableGlobalZBufferSafe( FALSE );

    for ( unsigned int ui = 0; ui < MAX_OBJ_CONT; ++ui )
    {
        for ( PgWorld::ObjectContainer::iterator iter = m_kObjectContainer[ui].begin(); 
              iter != m_kObjectContainer[ui].end(); ++iter )
        {
            iter->second->DrawNoZTest( pkRenderer, pkCamera, fFrameTime );
        }
    }

    NewWare::Renderer::DrawUtils::EnableGlobalZBuffer( TRUE );
}

void	PgWorld::InitSpotLightMan()
{
	if(m_pkSpotLightMan)
	{
		return;
	}

	m_pkSpotLightMan = PgSpotLight::Create(NiRenderer::GetRenderer(), 9);
	if(!m_pkSpotLightMan)
	{
		NILOG(PGLOG_LOG,"PgSpotLight::Create() Failed");
		return;
	}

	m_pkSpotLightMan->SetLightDirection(NiPoint3(0,0,-1)); 

	NiTextureEffect	*pkTextureEffect= m_pkSpotLightMan->GetTextureEffect();
	if(!pkTextureEffect)
	{
		return;
	}

	NiNode *pkTarget = (NiNode*)GetSceneRoot();
	if(pkTarget)
	{
		pkTextureEffect->AttachAffectedNode(pkTarget);
		pkTarget->UpdateEffects();
	}
	pkTextureEffect->SetSwitch(false);
}
void	PgWorld::TerminateSpotLightMan()
{
	SAFE_DELETE(m_pkSpotLightMan);
}
bool	PgWorld::GetSpotLightOn()
{
	if(!m_pkSpotLightMan)
	{
		return false;
	}

	NiTextureEffect	*pkTextureEffect= m_pkSpotLightMan->GetTextureEffect();

	if(!pkTextureEffect)
	{
		return	false;
	}

	return	pkTextureEffect->GetSwitch();
}

void	PgWorld::AddSpotLightCaster(NiAVObject* 	pkCaster)
{
	if(!m_pkSpotLightMan)
	{
		return;
	}

	NiNode	*pkCasterHead = m_pkSpotLightMan->GetCaster();
	if(!pkCasterHead)
	{
		return;
	}

	//	이미 있으면 리턴한다.
	int iTotal = pkCasterHead->GetArrayCount();
	for(int i=0;i<iTotal;++i)
	{
		NiAVObject* 	pkChild = pkCasterHead->GetAt(i);
		if(pkChild == pkCaster)
		{
			return;
		}
	}

	pkCasterHead->AttachChild(pkCaster, true);

}
void	PgWorld::RemoveSpotLightCaster(NiAVObject* 	pkCaster)
{
	if(!m_pkSpotLightMan)
	{
		return;
	}

	NiNode	*pkCasterHead = m_pkSpotLightMan->GetCaster();
	if(!pkCasterHead)
	{
		return;
	}

	pkCasterHead->DetachChild(pkCaster);
}
void	PgWorld::SetSpotLightBGColor(int const Red,int const Blue,int const Green)
{
	if(!m_pkSpotLightMan)
	{
		return;
	}

	m_pkSpotLightMan->SetBGColor(NiColor(Red/255.0f,Blue/255.0f,Green/255.0f));

}
void	PgWorld::TurnOnSpotLight(bool const bTurnOn)
{
	if(!m_pkSpotLightMan)
	{
		return;
	}
	NiTextureEffect	*pkTextureEffect= m_pkSpotLightMan->GetTextureEffect();
	if(!pkTextureEffect)
	{
		return;
	}

	pkTextureEffect->SetSwitch(bTurnOn);

	NiNode *pkTarget = (NiNode*)GetSceneRoot();
	if(pkTarget)
	{
		pkTarget->UpdateEffects();
	}

	if(m_spSceneRoot)
	{
		NiNode	*pkStaticRoot = GetStaticNodeRoot();
		if(pkStaticRoot)
		{
            NewWare::Scene::ApplyTraversal::Geometry::SetShaderConstantUpdateOptimizeFlag( pkStaticRoot, true, true );
		}
		if(GetPSRoomGroupRoot())
		{
			GetPSRoomGroupRoot()->SetShaderConstantUpdateOptimizeFlag(true);
		}
	}
}

void PgWorld::SetBgColor(NiColor const kColor)
{
	m_kBGColor = kColor;
}

void PgWorld::SetDrawMode( NewWare::Renderer::DrawBound::DrawMode const eDrawMode )
{
    using NewWare::Renderer::DrawWorkflow;

	if ( GetInstanceProxy(DrawWorkflow)->GetDrawMode() != eDrawMode )
	{
		m_ePrevDrawMode = GetInstanceProxy(DrawWorkflow)->GetDrawMode();
		GetInstanceProxy(DrawWorkflow)->Create( eDrawMode );
	}
}

void PgWorld::RestoreDrawMode()
{
	GetInstanceProxy(NewWare::Renderer::DrawWorkflow)->Create( m_ePrevDrawMode );
}

bool PgWorld::IsNotCulling()const
{
	if(0.f == m_kCameraAdjustInfo.fMinZoomDelta)
	if(1.f == m_kCameraAdjustInfo.fMaxZoomDelta)
	{
		return false;
	}
	/*if(m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW || m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW_DEPTH_PATH)
	{
		PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(m_kCameraMan.GetCameraMode());
		if( pkCameraMode && pkCameraMode->GetCameraZoom() <= 1.f)
		{
			return false;
		}
	}*/
	return m_bNotCulling;
}

void	PgWorld::UpdateSelected(float fTime,NiCamera *pkCamera,NiNode *pkSelectiveNodeRoot)
{
	// NOTE: When changing UpdateDownwardPass, UpdateSelectedDownwardPass,
	// or UpdateRigidDownwardPass, remember to make equivalent changes
	// the all of these functions.
	NiFrustumPlanes	kPlanes(*pkCamera);

	NiPoint3	const	&kCamPos = pkCamera->GetWorldTranslate();

	pkSelectiveNodeRoot->UpdateObjectControllers(fTime, pkSelectiveNodeRoot->GetSelectiveUpdatePropertyControllers());

	if (pkSelectiveNodeRoot->GetSelectiveUpdateTransforms())
		pkSelectiveNodeRoot->UpdateWorldData();

	NiBound kBound;

	pkSelectiveNodeRoot->SetWorldBound(kBound);

	float	fLowUpdateDistance = 1500;
	float	fDistanceFromCam = 0.0f;
	bool	bLowUpdate = g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_WORLD_QUALITY) != 1;
	bool	bUseUpdateLOD = false;

	NiPoint3	kPlayerPos;
	PgActor* pkPlayerActor = g_kPilotMan.GetPlayerActor();
	if(pkPlayerActor)
	{
		kPlayerPos = pkPlayerActor->GetPosition();
	}

	// To avoid having to call UpdateWorldBound and therefore making another
	// iteration through the node's children, the world bound is calculated
	// during this loop.
	int	const	iChildCount = pkSelectiveNodeRoot->GetArrayCount();
	for (int i = 0; i < iChildCount; ++i)
	{
		NiAVObject* pkChild = pkSelectiveNodeRoot->GetAt(i);
		if (pkChild)
		{
			bool	bVisible = true;
			unsigned int j = 0;

			bUseUpdateLOD = (pkChild->GetExtraData("USE_UPDATE_LOD")==NULL);

			NiBound	kChildBound = pkChild->GetWorldBound();
			fDistanceFromCam = (kChildBound.GetCenter()-kCamPos).Length() - kChildBound.GetRadius();

			if(bUseUpdateLOD)
			{
				if(fDistanceFromCam > 1000)
				{
					for (j = 0; j < NiFrustumPlanes::MAX_PLANES; ++j)
					{
						int iSide = kChildBound.WhichSide(
							kPlanes.GetPlane(j));

						if (iSide == NiPlane::NEGATIVE_SIDE)
						{
							// The object is not visible since it is on the negative
							// side of the plane.
							bVisible = false;
							break;
						}
					}		
				}
			}

			bool	bUpdate = false;
			if(bVisible)
			{
				if(bLowUpdate && bUseUpdateLOD)
				{
					if(fDistanceFromCam<fLowUpdateDistance)
					{
						bUpdate = true;
					}
				}
				else
				{
					bUpdate = true;
				}
			}

			bool	bPreviousUpdate = (pkChild->GetExtraData("UPDATED")!=NULL);

			if(bUpdate || kChildBound.GetRadius() == 0)
			{
				if(!bPreviousUpdate)	//	이전에 업데이트를 안했다면,
				{
                    NewWare::Scene::ApplyTraversal::Geometry::
                                SetShaderConstantUpdateOptimizeFlag( pkChild, false, true ); // 쉐이더 상수 업데이트 최적화 끄기

					if(m_spExtraData == 0)
					{
						m_spExtraData = NiNew NiExtraData();
					}
					pkChild->AddExtraData("UPDATED",m_spExtraData);
				}

				pkChild->DoSelectedUpdate(fTime);

				if (pkChild->IsVisualObject())
				{
					if (kBound.GetRadius() == 0.0f)
					{
						kBound = pkChild->GetWorldBound();
					}
					else
					{
						kBound.Merge(&pkChild->GetWorldBound());
					}
				}
			}
			else
			{
				if(bPreviousUpdate)	//	이전에 업데이트를 했다면
				{
					pkChild->RemoveExtraData("UPDATED");
                    NewWare::Scene::ApplyTraversal::Geometry::
                                SetShaderConstantUpdateOptimizeFlag( pkChild, true, true ); // 쉐이더 상수 업데이트 최적화 켜기
				}
			}
        }
    }

	pkSelectiveNodeRoot->SetWorldBound(kBound);

	if (pkSelectiveNodeRoot->GetParent())
		pkSelectiveNodeRoot->GetParent()->UpdateUpwardPass();
}

void PgWorld::DrawReflectionWorldScene( PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime )
{
    __asm nop
    __asm int 3
}

void	PgWorld::AdjustCameraAspectRatio(NiCamera *pkCamera)
{
	float	fWidth = NiRenderer::GetRenderer()->GetDefaultRenderTargetGroup()->GetWidth(0);
	float	fHeight = NiRenderer::GetRenderer()->GetDefaultRenderTargetGroup()->GetHeight(0);
	float	fRatio = fWidth / fHeight;

	pkCamera->AdjustAspectRatio(fRatio);
	pkCamera->Update(0);
}

void PgWorld::DrawFocusFilter( PgRenderer* pkRenderer )
{
	m_spFocusFilter->Draw(pkRenderer->GetRenderer());
}

bool PgWorld::ProcessInput(PgInput *pkInput)
{
	if(pkInput->IsRemote())
	{
		InputContainer::iterator itr = m_kInputContainer.find(pkInput->GetUKey());

		if(itr != m_kInputContainer.end())
		{
			lua_tinker::call<bool, lwWorld, lwPacket>(itr->second.c_str(), lwWorld(this), lwPacket(pkInput->GetPacket()));
		}
	}
	else if(pkInput->IsLocal())
	{
		InputContainer::iterator itr = m_kInputContainer.find(pkInput->GetUKey());

		if(itr != m_kInputContainer.end())
		{
			lua_tinker::call<bool, lwWorld, bool>(itr->second.c_str(), lwWorld(this), pkInput->GetPressed());
		}
		else
		{
			// 카메라 무빙
			switch(pkInput->GetUKey() - PgInput::UR_LOCAL_BEGIN)
			{
			case 1001:	// Actor Picking(DOWN)
				//{
				//}break;
				//case 1002:	// Actor Picking(UP)
				{
					if( g_kVendorBalloonMgr.ProcessInput(pkInput) )
					{
						break;
					}

					if( g_kPartyBalloonMgr.ProcessInput(pkInput) )
					{
						break;
					}
					
					if( g_kExpeditionBalloonMgr.ProcessInput(pkInput) )
					{
						break;
					}
					//static bool bOldClick = false, bCurClick = false;
					//bOldClick = bCurClick;
					//bCurClick = pkInput->GetPressed();
					//if( bOldClick
					//&&	!bCurClick )//업 이벤트 일때
					//{
					NiCamera* pkCamera = m_kCameraMan.GetCamera();
					if( pkCamera )
					{
						NiPick kPick;
						PgPickObjectMouseTester kTester(*pkCamera, kPick);
						PgIWorldObject *pkObject = PickObject(kTester);
						if( pkObject )
						{
							PgDropBox *pDropBox = dynamic_cast<PgDropBox*>(pkObject);
							if( pDropBox )
							{// 오브젝트가 드롭박스이면
								if( pDropBox->CheckPickup() )
								{// 인스턴트 아이템과 같은 마우스로 클릭해서 먹으면 안되는 아이템인지 체크한다.
									pkObject->Pickup();
								}
							}
							else
							{// 드롭박스가 아니면 상관 없다.
								if(!((g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) && (PgBossComplete::E_MSN_CPL_SELECT == g_kBossComplete.CompleteState())))
								{
									pkObject->Pickup();
								}
							}
						}
						//g_kMissionComplete.Pickup(1001);
						g_kBossComplete.Pickup(1001);

#ifndef EXTERNAL_RELEASE
						if(pkObject && lwKeyIsDown(NiInputKeyboard::KEY_LCONTROL,true))
						{
							BM::vstring vMsg("[DEV]Pickup Unit: ");
							vMsg += pkObject->GetID();
							Notice_Show( vMsg, EL_Normal );
							lua_tinker::call<void,lwGUID>("SetPickupActorGuid",lwGUID(pkObject->GetGuid()));
						}
#endif
					}
					//}
				}
				break;
			case NiInputKeyboard::KEY_SPACE:
				{
					static bool bOldSpace = false, bCurSpace = false;
					bOldSpace = bCurSpace;
					bCurSpace = pkInput->GetPressed();
					if( !g_kQuestMan.IsQuestDialog() //퀘스트 대화창 아니면서
						&&	bOldSpace
						&&	!bCurSpace ) //Space키가 Up 상태면
					{
						PgIWorldObject *pkObject = PickObjectByKeyboard();
						if(pkObject)
						{
							return pkObject->Pickup();
						}
					}
				}break;
#ifndef EXTERNAL_RELEASE
			case NiInputKeyboard::KEY_F8:
				{
					//if( pkInput->GetPressed() )
					//{
					//	lua_tinker::call<void>("Pick_PC");
					//}
				}break;
			case NiInputKeyboard::KEY_F7:
				{
					//if(pkInput->GetPressed())
					//{
					//	PgAlphaAccumulator::SetUseAlphaGroup(!PgAlphaAccumulator::GetUseAlphaGroup());
					//}
				}break;
			case NiInputKeyboard::KEY_F4:
				{
					if (pkInput->GetPressed())
					{
						printPhysXStat();
					}
				}break;
			case NiInputKeyboard::KEY_F2:
				{
					if( pkInput->GetPressed() )
					{
						g_bCameraDebug = !g_bCameraDebug;
					}
				}break;
			case NiInputKeyboard::KEY_F9:
				{
					if( pkInput->GetPressed() )
					{
						lua_tinker::call<void>("SelfReload");
						lua_tinker::call<void>("Reload");
					}
				}break;
#endif
			}

			if (m_pkHome)
			{
				m_pkHome->ProcessInput(pkInput);
			}

			if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_HOMETOWN))// 외부
			{
				BM::GUID kGuid;
				if( g_kPilotMan.GetPlayerPilotGuid(kGuid) )
				{
					PgPilot* pkPilot = g_kPilotMan.FindHouse(kGuid);
					if( pkPilot )
					{
						PgHouse* pkHouse = dynamic_cast<PgHouse*>(g_pkWorld->FindObject(pkPilot->GetGuid()));
						if( pkHouse )
						{
							pkHouse->ProcessInput(pkInput);
						}
					}
				}
			}
		}
	}
	else if(pkInput->Data())
	{
		NiCamera* pkCamera = m_kCameraMan.GetCamera();
		if( pkCamera )
		{
			PgIWorldObject *pkObject = PickObject(OGT_PLAYER);
			if( pkObject )
			{//마우스오버된 오브젝트가 존재하면
				if( m_pkOldMouseOverObject != pkObject)
				{//기존에 있는거랑 동일한지 확인하고 동기화
					bool bWorking = false;
					pkObject->MouseOver(bWorking);
					if(bWorking)
					{//스크립트가 동작했으면
						if(m_pkOldMouseOverObject)
						{//마우스아웃 액션
							//m_pkOldMouseOverObject->MouseOut();
						}
						m_pkOldMouseOverObject = pkObject;
					}
				}
			}
			else
			{//마우스오버된 오브젝트가 존재하지 않을때
				if(m_pkOldMouseOverObject)
				{//마우스아웃 액션
					//m_pkOldMouseOverObject->MouseOut();
					//m_pkOldMouseOverObject = NULL;
				}
			}
		}
	}

	return false;
}

bool	PgWorld::ApplyFogElement(PgWorldElementFog const &kFogElement)
{
#ifndef PG_USE_NEW_FOG		
	if(kFogElement.IsTheAttributeValid(PgWorldElementFog::AT_DEPTH))
	{
		if (m_pkFogProperty)
		{
			m_pkFogProperty->SetDepth(kFogElement.GetDepth());
		}
	}
	if(kFogElement.IsTheAttributeValid(PgWorldElementFog::AT_FAR_DISTANCE))
	{
		g_kFrustum.m_fFar = kFogElement.GetFarDistance();
	}
#else
	if(kFogElement.IsTheAttributeValid(PgWorldElementFog::AT_DENSITY))
	{
		m_fOriginalFogDensity = kFogElement.GetDensity();
		if (m_pkFogProperty)
		{
			m_pkFogProperty->SetFogDensity(m_fOriginalFogDensity);
		}
	}
	if(kFogElement.IsTheAttributeValid(PgWorldElementFog::AT_START_DISTANCE))
	{
		m_fOriginalFogStart = kFogElement.GetStartDistance();
		if (m_pkFogProperty)
		{
			m_pkFogProperty->SetFogStart(m_fOriginalFogStart);
		}
	}
	if(kFogElement.IsTheAttributeValid(PgWorldElementFog::AT_END_DISTANCE))
	{
		m_fOriginalFogEnd = kFogElement.GetEndDistance();
		if (m_pkFogProperty)
		{
			m_pkFogProperty->SetFogEnd(m_fOriginalFogEnd);
		}
	}
	if(kFogElement.IsTheAttributeValid(PgWorldElementFog::AT_FUNC))
	{
		if (m_pkFogProperty)
		{
			m_pkFogProperty->SetFogFunction(kFogElement.GetFogFunction());
		}
	}
#endif
	if(kFogElement.IsTheAttributeValid(PgWorldElementFog::AT_COLOR))
	{
		m_kFogColor = kFogElement.GetFogColor();
		if (m_pkFogProperty)
		{
			m_pkFogProperty->SetFogColor(m_kFogColor);
		}
	}
	if(kFogElement.IsTheAttributeValid(PgWorldElementFog::AT_ALPHA))
	{
		m_fFogAlpha = kFogElement.GetAlpha();
		if (m_pkFogProperty)
		{
			m_pkFogProperty->SetFogAlpha(m_fFogAlpha);
		}
	}
	if(kFogElement.IsTheAttributeValid(PgWorldElementFog::AT_APPLY_TO_SKYBOX))
	{
		m_bUseFogAtSkybox = kFogElement.GetApplyToSkyBox();
	}
	if(kFogElement.IsTheAttributeValid(PgWorldElementFog::AT_USE_FOG_ENDFAR))
	{
		m_bUseFogEndFar = kFogElement.GetUseFogEndFar();
	}

	return	true;
}
void PgWorld::UseFog(bool bUse, bool bUseAdjust)
{
	if (bUse && g_bUseFog)
	{
		if (m_pkFogProperty == NULL)
		{
			m_pkFogProperty = NiNew NiFogProperty;
			m_pkFogProperty->SetFog(true);
#ifdef PG_USE_NEW_FOG
			m_pkFogProperty->SetFogFunction(NiFogProperty::FOG_Z_LINEAR);
			m_pkFogProperty->SetFogStart(500);
			m_pkFogProperty->SetFogEnd(MAX_FAR_PLANE);
			m_pkFogProperty->SetFogDensity(0.05f);
#endif
			m_spSceneRoot->AttachProperty(m_pkFogProperty);
			if(m_spSkyRoot)
			{
				if(m_bUseFogAtSkybox)
				{
					NiFogProperty *pkProp = (NiFogProperty*)m_spSkyRoot->GetProperty(NiFogProperty::GetType());
					if(pkProp)
					{
						m_spSkyRoot->DetachProperty(pkProp);
					}
				}
				else
				{
					NiFogProperty *pkProp = (NiFogProperty*)m_spSkyRoot->GetProperty(NiFogProperty::GetType());
					if(!pkProp)
					{
						pkProp = (NiFogProperty*)m_pkFogProperty->Clone();
						m_spSkyRoot->AttachProperty(pkProp);
					}
					pkProp->SetFog(false);
				}
			}
			m_spSceneRoot->UpdateProperties();
            NewWare::Scene::ApplyTraversal::Property::DisableFogAtBillboardAndParticle( m_spSceneRoot );
		}
		else
		{
			m_pkFogProperty->SetFog(true);
			if(m_spSkyRoot)
			{
				if(m_bUseFogAtSkybox)
				{
					NiFogProperty *pkProp = (NiFogProperty*)m_spSkyRoot->GetProperty(NiFogProperty::GetType());
					if(pkProp)
					{
						m_spSkyRoot->DetachProperty(pkProp);
					}
				}
				else
				{
					NiFogProperty *pkProp = (NiFogProperty*)m_spSkyRoot->GetProperty(NiFogProperty::GetType());
					if(!pkProp)
					{
						pkProp = (NiFogProperty*)m_pkFogProperty->Clone();
						m_spSkyRoot->AttachProperty(pkProp);
					}
					pkProp->SetFog(false);
				}
			}
            NewWare::Scene::ApplyTraversal::Geometry::SetMaterialNeedsUpdate( m_spSceneRoot, true );
		}
		m_bUseFogAdjust = bUseAdjust;
	}
	else
	{
		if (m_pkFogProperty)
			m_pkFogProperty->SetFog(false);

		if (m_spSkyRoot)
		{
			NiFogProperty *pkProp = (NiFogProperty*)m_spSkyRoot->GetProperty(NiProperty::FOG);
			if (pkProp)
				pkProp->SetFog(false);
		}
		m_bUseFogAdjust = false;
        NewWare::Scene::ApplyTraversal::Geometry::SetMaterialNeedsUpdate( m_spSceneRoot, true );
	}
}

bool PgWorld::FindSpawnLoc(char const* pcSpawnName, NiPoint3 &rkSpawn_out)
{
	if (pcSpawnName == NULL)
		return false;

	NiAVObject *pkSpawn = m_spSpawnRoot->GetObjectByName(pcSpawnName);
	if(!pkSpawn)
	{
		NILOG(PGLOG_ERROR, "can't find spawn %s name in %s world\n", pcSpawnName, GetID().c_str());
		return false;
	}

	rkSpawn_out = pkSpawn->GetTranslate();
	rkSpawn_out = pkSpawn->GetWorldTranslate();
	return true;
}

bool PgWorld::FindTriggerLoc(char const* pcTriggerName, NiPoint3 &rkTrigger_out)
{
	if (pcTriggerName == NULL)
		return false;

	NiAVObject *pkTrigger = m_spTriggerRoot->GetObjectByName(pcTriggerName);
	if(!pkTrigger)
	{
		NILOG(PGLOG_ERROR, "can't find trigger %s name in %s world\n", pcTriggerName, GetID().c_str());
		return false;
	}

	rkTrigger_out = pkTrigger->GetTranslate();
	rkTrigger_out = pkTrigger->GetWorldTranslate();
	return true;
}

bool PgWorld::AddObject(BM::GUID const& rkGuid, PgIWorldObject *pkObject, NiPoint3 &rkLoc, ObjectGroupType kGroup)
{
	PG_ASSERT_LOG(pkObject);
	if (pkObject == NULL)
		return false;

	if(kGroup == OGT_NONE)
		return false;

	if (kGroup == OGT_PLAYER)
	{
		NILOG(PGLOG_LOG, "[PgWorld] AddPlayerObject %s(%d), (%f, %f, %f)\n", MB(rkGuid.str()), kGroup, rkLoc.x, rkLoc.y, rkLoc.z);
	}
	else if (kGroup == OGT_MONSTER)
	{
		NILOG(PGLOG_LOG, "[PgWorld] AddMonsterObject %s(%d), (%f, %f, %f)\n", MB(rkGuid.str()), kGroup, rkLoc.x, rkLoc.y, rkLoc.z);
	}
	else
	{
		NILOG(PGLOG_LOG, "[PgWorld] AddObject %s(%d), (%f, %f, %f)\n", MB(rkGuid.str()), kGroup, rkLoc.x, rkLoc.y, rkLoc.z);
	}

	PG_ASSERT_LOG(pkObject);

	WORLD_OBJECT_CONT_ID	kContID = WOCID_MAIN;

	if (kGroup == OGT_OBJECT)
	{
		kContID = WOCID_BEFORE;
	}

	ObjectContainer::iterator itor = m_kObjectContainer[kContID].find(rkGuid);
	if(itor != m_kObjectContainer[kContID].end())
	{
		//	중복 guid
		PgError1("PgWorld::AddObject : Guid 중복 - %s", MB(rkGuid.str()));
		PG_ASSERT_LOG(NULL);
		return	false;
	}

	if(!m_kObjectContainer[kContID].insert(std::make_pair(rkGuid, pkObject)).second)
	{
		SAFE_DELETE_NI(pkObject);
		// PG_ASSERT_LOG(!"failed to inserting new world object, maybe same GUID already exists");
		return false;
	}

	// !!! Character Controller를 사용하는 WorldObject는 모두 좌표를 조절해 주어야 한다.
	// !!! 클라는 Character Controller의 중점을 기준으로 쓰고, 서버는 바닥을 기준으로 캐릭터를 움직인다.
	bool bCheckFloor = true;
	if (kGroup == OGT_NPC)
	{
		if (pkObject->GetPilot() && pkObject->GetPilot()->GetUnit())
		{
			PgNpc* pkNpc = dynamic_cast<PgNpc*>(pkObject->GetPilot()->GetUnit());
			if (pkNpc && pkNpc->NpcType() == E_NPC_TYPE_CREATURE)
			{
				bCheckFloor = false;
			}
		}
	}

	if(kGroup == OGT_PLAYER || kGroup == OGT_MONSTER || kGroup == OGT_GROUNDBOX || kGroup == OGT_OBJECT || kGroup == OGT_ENTITY || kGroup == OGT_PET || (kGroup == OGT_NPC && bCheckFloor)
		|| kGroup == OGT_SIMILAR_PLAYER
		)
	{
		rkLoc = FindActorFloorPos(rkLoc,-1);
		if(kGroup == OGT_GROUNDBOX)
			rkLoc.z -= PG_CHARACTER_Z_ADJUST;

		if( (OGT_SIMILAR_PLAYER==kGroup || OGT_MONSTER==kGroup)
		&& pkObject->GetPilot()->GetAbil(AT_MONSTER_TYPE)==EMONTYPE_FLYING )
		{
			int iTargetHeight = pkObject->GetPilot()->GetAbil(AT_HEIGHT);
			rkLoc.z += iTargetHeight + 10.0f; // 살짝 위에서 내려오는 듯하게.
		}

		NILOG(PGLOG_LOG, "[PgWorld] AddObject %s, impactPoint(%f, %f, %f)\n", MB(rkGuid.str()), rkLoc.x, rkLoc.y, rkLoc.z);
	}

	//if(kGroup == 1)
	//{
	//	NxRay kRay(NxVec3(rkLoc.x, rkLoc.y, rkLoc.z + 150.0f), NxVec3(0, 0, -1.0f));
	//	NxRaycastHit kHit;
	//	if(m_spPhysXScene->GetPhysXScene()->raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, 1, 300.0f))
	//	{
	//		NiPhysXTypes::NxVec3ToNiPoint3(kHit.worldImpact, rkLoc);
	//		rkLoc.z += 25.0f;
	//	}
	//}
	//if ( kGroup == 2 )
	//{
	//	NxRay kRay(NxVec3(rkLoc.x, rkLoc.y, rkLoc.z + 50.0f), NxVec3(0, 0, -1.0f));
	//	NxRaycastHit kHit;
	//	if(m_spPhysXScene->GetPhysXScene()->raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, -1, 200.0f))
	//	{
	//		rkLoc.z = kHit.worldImpact.z;
	//		rkLoc.z += 26.0f;
	//		NILOG(PGLOG_LOG, "[PgWorld] AddObject %s(%d), z adjust %f\n", MB(rkGuid.str()), kGroup, rkLoc.z);
	//	}
	//}

	//if ( kGroup == 6 ) //! npc
	//{
	//	NxRay kRay(NxVec3(rkLoc.x, rkLoc.y, rkLoc.z + 50.0f), NxVec3(0, 0, -1.0f));
	//	NxRaycastHit kHit;
	//	if(m_spPhysXScene->GetPhysXScene()->raycastClosestShape(kRay, NX_STATIC_SHAPES, kHit, -1, 200.0f))
	//	{
	//		rkLoc.z = kHit.worldImpact.z;
	//		rkLoc.z += 26.0f;
	//		NILOG(PGLOG_LOG, "[PgWorld] AddObject %s(%d), z adjust %f\n", MB(rkGuid.str()), kGroup, rkLoc.z);
	//	}
	//}

	pkObject->SetGuid(rkGuid);
	pkObject->SetTranslate(rkLoc);
	pkObject->SetPathRoot(m_spPathRoot);
	pkObject->SetGroupNo(kGroup);
	pkObject->InitPhysX(m_spPhysXScene, kGroup);
	
#ifndef EXTERNAL_RELEASE
	if (NiIsKindOf(PgActor, pkObject))
	{
		if (kGroup == OGT_PLAYER)
			((PgActor*)pkObject)->SetTraceUpdate(true);
		else
			((PgActor*)pkObject)->SetTraceUpdate(false);
	}
#endif


	if(kGroup == OGT_PUPPET || kGroup == OGT_OBJECT)	//	퍼펫만 라이트를 받게 한다. 나머지 오브젝트들은 라이트를 받으면 안됨. leesg213
	{
		LightObjectRecurse(m_spLightRoot, pkObject,LT_ALL);
	}

    NewWare::Scene::ApplyTraversal::Property::DisableFogAtBillboardAndParticle( pkObject );

	PG_ASSERT_LOG(kGroup>=0 && kGroup<OGT_MAX);
	m_aspObjectGroups[kGroup]->AttachChild(pkObject, true);

	int iLODCount = pkObject->GetLODCount();
	if (iLODCount > 0 && GET_OPTION_WORLD_QUALITY == false)
	{
		//pkObject->SetLOD(1);
	}

	switch(kGroup)
	{
	case OGT_PLAYER:
	case OGT_SIMILAR_PLAYER:
		{
			using namespace PgActorUtil;

			bool bIsClassRank4 = false;
			//PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>(pkObject);

			if (pkObject->GetPilot())
			{
				switch( pkObject->GetPilot()->GetAbil(AT_CLASS) )
				{
				case UCLASS_DRAOON:
				case UCLASS_DESTROYER:
				case UCLASS_ARCHMAGE:
				case UCLASS_WARLORD:
				case UCLASS_SENTINEL:
				case UCLASS_LAUNCHER:
				case UCLASS_MANIAC:
				case UCLASS_SHADOW:
					{
						bIsClassRank4 = true;
					}break;
				}
			}
			UpdateColorShadow( *pkObject, this, false, bIsClassRank4 );
			/*
			char	*kParticleID[4]={
				"e_ef_colorshadow_battler",
				"e_ef_colorshadow_magician",
				"e_ef_colorshadow_archer",
				"e_ef_colorshadow_thief"
			};

			PG_ASSERT_LOG(pkObject->GetPilot());
			if( pkObject->GetPilot()
			&&	g_kHelpSystem.HelperGuid() != pkObject->GetGuid() )
			{
				int const iBaseClassIDMax = 5, iBaseClassIDMin = 0;
				int	iBaseClassID = pkObject->GetPilot()->GetBaseClassID();
				PG_ASSERT_LOG(iBaseClassID>iBaseClassIDMin);
				PG_ASSERT_LOG(iBaseClassID<iBaseClassIDMax);
				if( iBaseClassIDMin < iBaseClassID
				&&	iBaseClassIDMax > iBaseClassID )
				{
					NiAVObject *pkParticle = g_kParticleMan.GetParticle(kParticleID[NiClamp(iBaseClassID-1, 0, 3)], pkObject->GetEffectScale());
					if(pkParticle)
					{
						if(!pkObject->AttachTo(9812341,"char_root",pkParticle))
						{
							THREAD_DELETE_PARTICLE(pkParticle);
						}
					}
				}
			}
			*/
		}
		break;
	case OGT_NPC:
		{
			//PG_ASSERT_LOG(pkObject->GetPilot());
			//if(pkObject->GetPilot() && bCreature == false)
			//{
			//	NiAVObject *pkParticle = g_kParticleMan.GetParticle("e_ef_colorshadow_npc", pkObject->GetEffectScale());
			//	if(pkParticle)
			//	{
			//		pkObject->AttachTo(9812341,"char_root",pkParticle);
			//	}
			//}
		}
		break;
	}

	pkObject->SetSelectiveUpdate(true);
	pkObject->SetSelectiveUpdateTransforms(true);
	pkObject->SetSelectiveUpdatePropertyControllers(true);
	pkObject->SetSelectiveUpdateRigid(true);
	pkObject->UpdateNodeBound();
	pkObject->UpdateProperties();
	pkObject->UpdateEffects();
	((NiNode *)pkObject)->Update(0.0f);

    NewWare::Scene::AssetUtils::NIF::RecursivePrepack( pkObject, true );

	//	Init Script 가 있다면 실행해주자.
	PgActor* pkActor = dynamic_cast<PgActor* >(pkObject);
	if(pkActor)
	{

		if(pkObject->GetInitScript() != _T(""))
		{
			lua_tinker::call<void,lwActor>(MB(pkObject->GetInitScript()),lwActor((pkActor)));
		}

		//PgRenderer::SetSelectiveUpdateFlags(pkActor);

	}
    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkObject, false );
	return true;
}

void	PgWorld::AddWaveEffect(char const* strEntityID,char const* strStopEffectID,char const* strMoveEffectID)
{
	if(!GetSceneRoot())
	{
		return;
	}

	stWaveEffect	kWaveEffect;

	kWaveEffect.m_kEntityID = strEntityID;
	kWaveEffect.m_kStopEffectID = strStopEffectID;
	kWaveEffect.m_kMoveEffectID = strMoveEffectID;
	kWaveEffect.m_pkEntityNode = GetSceneRoot()->GetObjectByName(strEntityID);

	m_vWaveEffect.push_back(kWaveEffect);
}
void	PgWorld::UpdateWaveEffect(PgIWorldObject *pkWorldObject,bool bMoveEffect)
{
	PgActor* pkActor = NiDynamicCast(PgActor,pkWorldObject);
	if(!pkActor)
	{
		return;
	}

	NiPoint3	const& kPos = pkActor->GetPos();

	NiPick kPick;
	kPick.SetCoordinateType(NiPick::WORLD_COORDINATES);
	kPick.SetPickType(NiPick::FIND_FIRST);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetObserveAppCullFlag(false);
	kPick.ClearResultsArray();
	kPick.SetReturnNormal(true);

	NiPoint3	kOrigin = kPos + NiPoint3::UNIT_Z * 9.0f;

	NiPoint3	kLookDir = pkActor->GetLookingDir();

	int iTotalWave = m_vWaveEffect.size();
	for(int i=0;i<iTotalWave;++i)
	{
		stWaveEffect const& kWave = m_vWaveEffect[i];

		if(kWave.m_pkEntityNode == NULL)
		{
			continue;
		}

		kPick.ClearResultsArray();
		kPick.SetTarget(kWave.m_pkEntityNode);
		if(kPick.PickObjects(kOrigin, NiPoint3::UNIT_Z*-1) == false)
		{
			continue;
		}

		NiPick::Results &kResult = kPick.GetResults();
		if(kResult.GetEffectiveSize() == 0)
		{
			continue;
		}
		NiPick::Record* pkRecord =  kResult.GetAt(0);
		if(pkRecord && pkRecord->GetDistance()>33)
		{
			continue;
		}

		std::string	kEffectID;
		if(bMoveEffect)
		{
			kEffectID = kWave.m_kMoveEffectID;
		}
		else
		{
			kEffectID = kWave.m_kStopEffectID;
		}

		if(kEffectID.size() == 0)
		{
			continue;
		}

		PgParticle* pkParticle = g_kParticleMan.GetParticle(kEffectID.c_str());
		if(!pkParticle)
		{
			continue;
		}

		NiPoint3 const& kIntersectPos = pkRecord->GetIntersection();
		NiPoint3 const& kNormal = pkRecord->GetNormal();

		NiPoint3	kParticlePos = kIntersectPos;
		kParticlePos.z+=2;

		NxVec3	vUp(0,0,1);
		NxVec3	vNormal = NxVec3(kNormal.x,kNormal.y,kNormal.z);
		float	fTheta = acos(vUp.dot(vNormal));
		NxVec3	vRotationAxis = vNormal.cross(vUp);
		vRotationAxis.normalize();

		NiMatrix3	kmatRot;
		kmatRot.MakeRotation(fTheta,NiPoint3(vRotationAxis.x,vRotationAxis.y,vRotationAxis.z));

		if(bMoveEffect)
		{
			NiPoint3	vAxis;

			float	fCos = kLookDir.Dot(NiPoint3::UNIT_X);
			float	fAngle = acos(fCos);

			vAxis = NiPoint3::UNIT_X.Cross(kLookDir);
			vAxis.Unitize();

			if(vAxis.Length() < 0.5)
			{
				vAxis = NiPoint3::UNIT_Z;
			}

			NiQuaternion	kQuat(fAngle,vAxis);
			NiMatrix3	kmatRot2;

			kQuat.ToRotation(kmatRot2);
			
			NiMatrix3	kmatRot3;
			kmatRot3 = kmatRot*kmatRot2;

			kmatRot = kmatRot3;
		}

		pkParticle->SetRotate(kmatRot);
		AttachParticle(pkParticle,kParticlePos);
	}

}

PgRope*	PgWorld::AddRope(char const* strRopeName,NiPoint3 const& kStart,NiPoint3 const& kEnd,float fRange)
{
	if(strRopeName == NULL || strlen(strRopeName) == 0)
	{
		return	NULL;
	}

	NiNode	*pkStatic = GetStaticNodeRoot();
	if(pkStatic)
	{
		if(pkStatic->GetObjectByName(strRopeName))
		{
			PgError1("[PgWorld] %s rope is already exist!", strRopeName);
			return	NULL;
		}

		if(kStart == kEnd)
		{
			PgError1("[PgWorld] - %s rope - start position must be not equal to end position", strRopeName);
			return	NULL;
		}

		if(fRange<=0)
		{
			PgError1("[PgWorld] - %s rope - range must be larger than 0", strRopeName);
			return	NULL;
		}

		NiPoint3	kStartPos = kStart,kEndPos = kEnd;

		//float	fHeight = 100;
		//NiPoint3	kRayStart = kStart;
		//kRayStart.z+=50;

		//NiPoint3 kFloorPos = ThrowRay(kRayStart,-NiPoint3::UNIT_Z,1000);
		//if(kFloorPos != NiPoint3(-1,-1,-1))
		//{
		//	kStartPos = kFloorPos+NiPoint3::UNIT_Z*fHeight;
		//}

		//kRayStart = kEnd;
		//kRayStart.z+=50;

		//kFloorPos = ThrowRay(kRayStart,-NiPoint3::UNIT_Z,1000);
		//if(kFloorPos != NiPoint3(-1,-1,-1))
		//{
		//	kEndPos = kFloorPos+NiPoint3::UNIT_Z*fHeight;
		//}

		PgRope	*pkNewRope = NiNew PgRope();
		pkNewRope->SetName(strRopeName);
		pkNewRope->SetRopeData(kStartPos,kEndPos,fRange);

		pkStatic->AttachChild(pkNewRope, true);
		pkStatic->Update(0);

		return	pkNewRope;

	}

	return	NULL;
}
PgPuppet *PgWorld::AddPuppet_KFMPath(std::string const &kName,std::string const &kKFMPath, NiActorManager::SequenceID const &kActivateSeqID,NiTransform const &kTransform,bool bUsePhysX,bool bUseTrigger)
{

	std::string	kRelativePath = kKFMPath;

	int	iLen = kKFMPath.length();
	std::string	kDataString("DATA\\");

	if(iLen>kDataString.length())
	{
		int	iStartPos = 0;
		for(int i=iLen-kDataString.length();i>=0;i--)
		{
			if(stricmp(kKFMPath.substr(i,kDataString.length()).c_str(),kDataString.c_str()) == 0)
			{
				iStartPos = i+kDataString.length();
				break;
			}
		}

		kRelativePath = kKFMPath.substr(iStartPos,iLen-iStartPos);
	}

	kRelativePath = std::string("../Data/") + kRelativePath;

	PgPuppet *pkPuppet = PgPuppet::CreatePuppet(kName,kRelativePath,kActivateSeqID,kTransform,bUsePhysX,bUseTrigger);
	if( !pkPuppet )
	{
		_PgMessageBox(GetClientName(), "Create PgPuppet Failed [%s][%s]", kName.c_str(), kRelativePath.c_str());
		return	NULL;
	}
	pkPuppet->SaveTransform(kTransform);
	pkPuppet->SetHasActorXML(false);

	NiPoint3	kTranslate = kTransform.m_Translate;
	AddObject(BM::GUID::Create(), pkPuppet, kTranslate, OGT_PUPPET);

	NiAVObject	*pkAVObject = NiDynamicCast(NiAVObject,pkPuppet);
	if(pkAVObject)
	{
		pkAVObject->Update(0);
	}

	return	pkPuppet;
}
PgPuppet *PgWorld::AddPuppet_ActorXML(std::string const &kName,std::string const &kXmlPath, NiActorManager::SequenceID const &kActivateSeqID,NiTransform const &kTransform,bool bUsePhysX,bool bUseTrigger)
{

	std::string	kRelativePath = kXmlPath;

	int	iLen = kXmlPath.length();
	std::string	kXmlString("XML\\");

	if(iLen>kXmlString.length())
	{
		int	iStartPos = 0;
		for(int i=iLen-kXmlString.length();i>=0;i--)
		{
			if(stricmp(kXmlPath.substr(i,kXmlString.length()).c_str(),kXmlString.c_str()) == 0)
			{
				iStartPos = i+kXmlString.length();
				break;
			}
		}

		kRelativePath = kXmlPath.substr(iStartPos,iLen-iStartPos);
	}

	PgPuppet *pkPuppet = dynamic_cast< PgPuppet* >( PgXmlLoader::CreateObjectFromFile(kRelativePath.c_str()) );
	if( !pkPuppet )
	{
		_PgMessageBox(GetClientName(), "Create PgPuppet Failed [%s]", kName.c_str(), kRelativePath.c_str());
		return	NULL;
	}
	pkPuppet->SaveTransform(kTransform);
	pkPuppet->SetHasActorXML(true);

	pkPuppet->SetID(kName.c_str());
	pkPuppet->SetName(kName.c_str());

	pkPuppet->SetUseTrigger(bUseTrigger);
	pkPuppet->SetUsePhysX(bUsePhysX);
	//pkPuppet->InitPhysX(m_spPhysXScene,OGT_PUPPET);

	if(pkPuppet->GetActorManager())
	{
		pkPuppet->GetActorManager()->Reset();
		pkPuppet->GetActorManager()->SetTargetAnimation(kActivateSeqID);
	}

	NiPoint3	kTranslate = kTransform.m_Translate + pkPuppet->GetTranslate();
	AddObject(BM::GUID::Create(), pkPuppet, kTranslate, OGT_PUPPET);

	NiTransform kNewTransform(kTransform);
	kNewTransform.m_Translate = kTranslate;

	pkPuppet->SetLocalTransform(kNewTransform);

	NiAVObject	*pkAVObject = NiDynamicCast(NiAVObject,pkPuppet);
	if(pkAVObject)
	{
		pkAVObject->Update(0);
	}

	return	pkPuppet;
}
PgPuppet *PgWorld::AddPuppet(BM::GUID& rkGuid, char const* pcPuppetName, NiPoint3 kTranslate, NiQuaternion kRotate)
{
	PuppetContainer::iterator itr = m_kPuppetContainer.find(pcPuppetName);
	if(itr == m_kPuppetContainer.end())
	{
		return 0;
	}

	//PgPuppet *pkPuppet = (PgPuppet *)itr->second;
	PgPuppet *pkPuppet = dynamic_cast< PgPuppet* >( itr->second->Clone() );
	if( !pkPuppet )
	{
		return 0;
	}

	kRotate.FastNormalize();
	pkPuppet->SetRotate(kRotate);

	AddObject(BM::GUID::Create(), pkPuppet, kTranslate, OGT_PUPPET);
	//	AddObject(rkGuid, pkPuppet, kTranslate, 7);
	((NiNode *)pkPuppet)->Update(0.0f);
	return pkPuppet;
}

bool PgWorld::AddShineStoneToWorld(BM::GUID const& kStoneGuid, POINT3 const& ptPos)
{
	std::string strName = SHINESTONE_PREFIX_STRING;
//	std::string strID = MB(BM::vstring(rkPlayerStone.iID));
//	strName += strID;

//	ShineStoneContainer::iterator itr = m_kShineStoneContainer.find(strName.c_str());
//	if (itr == m_kShineStoneContainer.end())
//	{
//		return false;
//	}

//	PgShineStone* pkStone = (PgShineStone*)itr->second;
//	pkStone->PlayerStone(rkPlayerStone);
//	if (!pkStone)
//	{
//		return false;
//	}

	// Add to SceneRoot
	PgShineStone* pkStone = NiNew PgShineStone;

	pkStone->SetID(SHINESTONE_PREFIX_STRING);
	pkStone->SetGuid(kStoneGuid);
	pkStone->SetPos(ptPos);

	AddObject(kStoneGuid, pkStone, pkStone->GetPos(), OGT_SHINESTONE);

	m_kShineStoneContainer.insert(std::make_pair(pkStone->GetGuid(), pkStone));//스톤.

	// Add to PhysXScene
	NiPhysXScenePtr spPhysXObject = pkStone->GetPhysXObj();
	if (!spPhysXObject)
	{
		NILOG(PGLOG_ERROR, "Can't Get ShineStone PhysX Object \n");
		return false;
	}
	if (spPhysXObject->GetSnapshot())
	{
		NiPhysXSceneDesc* pkDesc = spPhysXObject->GetSnapshot();

		// 중복 네임이 있으면 피직스가 잘못 먹히기 때문에 강제로 이름을 바꾸어 준다.
		int iActorTotal = pkDesc->GetActorCount();
		for (int iActorCount=0 ; iActorCount<iActorTotal ; ++iActorCount)
		{
			NiPhysXActorDesc *pkActorDesc = pkDesc->GetActorAt(iActorCount);
			int iShapeTotal = pkActorDesc->GetActorShapes().GetSize();
			for (int iShapeCount=0 ; iShapeCount<iShapeTotal ; ++iShapeCount)
			{
				NiPhysXShapeDesc *pkShapeDesc =
					pkActorDesc->GetActorShapes().GetAt(iShapeCount);

				// Rename PhysX Object
				if (pkShapeDesc->GetMeshDesc())
				{//GUID 는 고유값이라.
//					NiString strDescName = pkStone->GetGuid();
//					strDescName += "_";
//					char szCount[256];
//					_itoa_s(iActorCount, szCount, 10);
//					strDescName += szCount;
//					strDescName += "_";
//					_itoa_s(iShapeCount, szCount, 10);
//					strDescName += szCount;
//					strDescName += "_";
//					//strDescName += strNifPath.c_str();
//					NiFixedString strDescName_ = strDescName.MakeExternalCopy();
					pkShapeDesc->GetMeshDesc()->SetName(MB(pkStone->GetGuid().str()));
				}
			}
		}
		NxMat34 kSlaveMat;
		NiMatrix3 kPhysXRotMat = NiMatrix3::IDENTITY;
		NiPoint3 kPhysXTranslation = pkStone->GetPos();
		NiPhysXTypes::NiTransformToNxMat34(kPhysXRotMat, kPhysXTranslation, kSlaveMat);

		spPhysXObject->SetSlaved(m_spPhysXScene, kSlaveMat);
		spPhysXObject->CreateSceneFromSnapshot(0);
	}
	else
	{
		NILOG(PGLOG_ERROR, "Can't Get SnapShot from ShineStone PhysX Object \n");
	}
	// Insert PhysX Kinetic Data(Trigger Data)
	for (unsigned int iSrcCount=0 ; iSrcCount<spPhysXObject->GetSourcesCount() ; ++iSrcCount)
	{
		NiPhysXSrc *pkPhysXSrc = spPhysXObject->GetSourceAt(iSrcCount);
		if(NiIsKindOf(NiPhysXRigidBodySrc, pkPhysXSrc))
		{
			NiPhysXRigidBodySrc *pkBodySrc = (NiPhysXRigidBodySrc *)pkPhysXSrc;
			NiAVObject *pkSrcObj = pkBodySrc->GetSource();
			pkSrcObj->SetName(MB(pkStone->GetGuid().str()));
		}
		m_spPhysXScene->AddSource(pkPhysXSrc);

		// Add to container
		NiAVObject *pkSrcObj = 0;
		NxActor *pkActor = 0;
		if (!FindObjFromPhysXSrc(pkPhysXSrc, &pkSrcObj, &pkActor, PG_PHYSX_GROUP_SHINESTONE))
		{
		}
		if(pkSrcObj && pkActor && pkSrcObj->GetName())
		{
			ShineStoneContainer::iterator itr = m_kShineStoneContainer.find(kStoneGuid);
			if(itr == m_kShineStoneContainer.end())
			{
				continue;
			}

			pkActor->userData = (void *)itr->second;
		}
	}
	return true;
}

bool PgWorld::RemoveObject(BM::GUID const& rkGuid, bool bPreserve)
{
	// 제거할 오브젝트를 찾는다.
	for(int i=0;i<MAX_OBJ_CONT;++i)
	{
		ObjectContainer::iterator itr = m_kObjectContainer[i].find(rkGuid);
		if(itr == m_kObjectContainer[i].end())
		{
			continue;
		}

		PgIWorldObject	*pkObject = itr->second;

		if(!bPreserve)
		{
			PgPilot	*pkPilot = pkObject->GetPilot();
			CUnit	*pkUnit = pkPilot ? pkPilot->GetUnit() : NULL;

#ifdef PG_USE_NPC_FREEZE
			if (pkPilot && pkUnit && pkUnit->UnitType() == UT_NPC)
			{
				// 월드의 NPC를 미리 로딩해 두기 때문에 아예 Unfreeze도 안한다.
				//FreezeObject(rkGuid);
				return true;
			}
#endif
			removePilot(itr);
		}

		// 컨테이너에서 제거한다.
		m_kObjectContainer[i].erase(itr);

		g_kUIScene.SetRefreshAllMiniMap();

		m_pkOldMouseOverObject = NULL;

		return true;
	}

	return	false;
}

inline void PgWorld::removePilot(ObjectContainer::iterator iter, bool bCheckWorld)
{
	// TODO 내꺼는 안 지우면 좋겠는데...
	//PgActor* actor = NiDynamicCast(PgActor, iter->second);
	//if (actor)
	//{
	//	if (actor->IsMyActor())
	//		return;
	//}

	BM::GUID guid = iter->first;

	if( guid == BM::GUID::NullData() && iter->second != NULL )
	{
		NILOG(PGLOG_WARNING, "[PgWorld] removePilot which has NULL GUID (%s)(%#X)\n", MB(iter->second->GetGuid().str()), iter->second);
	}

	BM::GUID kCoupleGuid, kPartyGuid, kGuildGuid;

	PgIWorldObject	*pkObject = iter->second;
	if( pkObject )
	{
		PgPilot* pkPilot = g_kPilotMan.FindPilot(guid);
		if( pkPilot )
		{
			CUnit* pkUnit = pkPilot->GetUnit();
			if( OGT_PLAYER == pkObject->GetGroupNo() )
			{
				PgPlayer* pkPlayer = dynamic_cast<PgPlayer*>( pkUnit );
				if( pkPlayer )
				{
					kCoupleGuid = pkPlayer->CoupleGuid();
					kPartyGuid = pkPlayer->PartyGuid();
					kGuildGuid = pkPlayer->GuildGuid();
					BM::GUID kSubPlayerGuid = pkPlayer->SubPlayerID();
					if( kSubPlayerGuid.IsNotNull() )
					{
						BM::GUID kTemp;
						PgPilotManUtil::RemoveConnectionWithSubPlayer(guid, kTemp);
						RemoveObject(kSubPlayerGuid);
					}

					SetPetCallerUnit(pkPlayer, NULL);
				}
			}
			else if( OGT_SIMILAR_PLAYER == pkObject->GetGroupNo() )
			{
				switch( pkUnit->UnitType() )
				{
				case UT_SUMMONED:
					{
						PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
						if(pkPlayer)
						{
							pkPlayer->DeleteSummonUnit(guid);
						}
					}break;
				case UT_SUB_PLAYER:
					{
						PgSubPlayer* pkSubPlayer = static_cast<PgSubPlayer*>( pkUnit );
						if(pkSubPlayer)
						{
							pkSubPlayer->Caller( BM::GUID::NullData() );
							pkSubPlayer->SetCallerUnit(NULL);
						}
					}break;
				}
			}
		}
		NiNode	*pkParent = pkObject->GetParent();

		bool bMyActor = pkObject->IsMyActor();	// SetPilot(0)를 해버리면 항상 false라서 먼저 한다.
		// 부모와의 연결을 제거한다.
		pkObject->BeforeCleanUp();
		pkObject->SetPilot(0);

		if (bMyActor)
		{
			NILOG(PGLOG_LOG, "[PgWorld] removePilot(my actor,%s,%#X)(%d,%d,%d)\n", MB(pkObject->GetGuid().str()), pkObject, bCheckWorld, g_pkWorld != this, pkObject->GetParent() == NULL);
			if ((bCheckWorld && g_pkWorld != this) || pkObject->GetParent() == NULL)
			{
				SAFE_DELETE_NI(pkObject);	// 내꺼는 그냥 지운다. TODO: 안지우고 보존하는 방법은?				
			}
			else
			{
				if(pkParent)
				{
					pkParent->DetachChild(pkObject);
				}
			}

			g_kUnitCustomUI.Clear();
		}
		else
		{
			if ((bCheckWorld && g_pkWorld != this) || pkParent == NULL)
			{
				THREAD_DELETE_ACTOR(pkObject);
			}
			else
			{
				THREAD_DELETE_ACTOR(pkParent->DetachChild(pkObject));
			}
		}
	}
	// 파일럿을 제거한다.
	g_kPilotMan.RemovePilot(guid);

	//
	//삭제되는 Unit의 
	if( g_kPilotMan.IsMyPlayer(kCoupleGuid) )
	{
		PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
		if( pkMyActor )
		{
			pkMyActor->UpdateName(); // 내하트는 경직
		}
	}
	if( BM::GUID::IsNotNull(kPartyGuid) )
	{
		g_kParty.RemovePartyName(kPartyGuid, guid);
	}
	if( BM::GUID::IsNotNull(kGuildGuid) )
	{
		g_kGuildMgr.DelGuildInfo(kGuildGuid, guid);
	}
}

void PgWorld::SetTriggerPhysX(std::string const& kID,  bool bOn)
{
	TriggerContainer::iterator iter = m_kTriggerContainer.find(kID);
	if(iter == m_kTriggerContainer.end())
	{
		return;
	}

	PgTrigger* pkTarget = iter->second;
	NxShape* const* pkShape = pkTarget->GetPhysXRigidBodySrc()->GetTarget()->getShapes();

	(*pkShape)->setFlag(NX_TRIGGER_ENABLE, !bOn);
	(*pkShape)->setFlag(NX_SF_DISABLE_COLLISION, bOn);
}

void PgWorld::FreezeObject(BM::GUID& rkGuid)
{
	// 제거할 오브젝트를 찾는다.

	ObjectContainer::iterator itr = m_kObjectContainer[WOCID_MAIN].find(rkGuid);
	if(itr == m_kObjectContainer[WOCID_MAIN].end())
	{
		return;
	}

	PgPilot *pkPilot = g_kPilotMan.FindPilot(rkGuid);
	if(!pkPilot)
	{
		return;
	}

	NILOG(PGLOG_LOG, "[PgWorld] FreezeObject %s \n", MB(rkGuid.str()));

	PgIWorldObject *pkObject = itr->second;

	// 파일럿을 얼린다.
	pkPilot->SetFreeze(true);

	PG_STAT(PgStatTimerF timerA(g_kWorldStatGroup.GetStatInfo("PgWorld.ReleasePhysX"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerB(g_kWorldStatGroup.GetStatInfo("PgWorld.ReleaseAllParticles"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerC(g_kWorldStatGroup.GetStatInfo("PgWorld.DetachChild"), g_pkApp->GetFrameCount()));
	PG_STAT(PgStatTimerF timerD(g_kWorldStatGroup.GetStatInfo("PgWorld.move"), g_pkApp->GetFrameCount()));

	// 물리 객체를 제거한다.
	//PG_STAT(timerA.Start());
	//pkObject->ReleasePhysX();
	//PG_STAT(timerA.Stop());

	PG_STAT(timerB.Start());
	pkObject->ReleaseAllParticles();
	PG_STAT(timerB.Stop());

	PG_STAT(timerC.Start());
	// 객체 그룹 트리에서 제거(객체 그룹 트리가 부모이다)
	NiAVObjectPtr spObject = pkObject->GetParent()->DetachChild(pkObject);
	PG_STAT(timerC.Stop());

	// 컨테이너에서 제거한다.
	PG_STAT(timerD.Start());
	m_kObjectContainer[WOCID_MAIN].erase(itr);

	// Frozen 컨테이너에 추가한다.
	m_kFrozenContainer.insert(std::make_pair(rkGuid, spObject));
	PG_STAT(timerD.Stop());

}

void PgWorld::UnfreezeObject(BM::GUID const& rkGuid)
{
	PgOutputPrint("UnfreezeObject\n");
	m_kUnfreezeObjectList.push_back(rkGuid);
}

void PgWorld::processUnfreezeObjectList(unsigned int iProcessCount)
{
	unsigned int iCount = 0;
	if (iProcessCount == 0)
		iCount = m_kUnfreezeObjectList.size();
	else
		iCount = __min(iProcessCount, m_kUnfreezeObjectList.size());

	UnfreezeObjectContainer::iterator itrE = m_kUnfreezeObjectList.begin();
	for (unsigned int i = 0 ; i < iCount && itrE != m_kUnfreezeObjectList.end(); ++i)
	{
		PG_STAT(PgStatTimerF timerA(g_kWorldStatGroup.GetStatInfo("PgWorld.InitPhysX"), g_pkApp->GetFrameCount()));
		PG_STAT(PgStatTimerF timerB(g_kWorldStatGroup.GetStatInfo("PgWorld.AttachChild"), g_pkApp->GetFrameCount()));
		PG_STAT(PgStatTimerF timerC(g_kWorldStatGroup.GetStatInfo("PgWorld.Insert"), g_pkApp->GetFrameCount()));	

		BM::GUID rkGuid = (*itrE);
		// Frozen 컨테이너에서 제거한다.
		AVObjectContainer::iterator itr = m_kFrozenContainer.find(rkGuid);
		if(itr == m_kFrozenContainer.end())
		{
			itrE = m_kUnfreezeObjectList.erase(itrE);
			continue;
		}
		NILOG(PGLOG_LOG, "[PgWorld] UnfreezeObject %s \n", MB(rkGuid.str()));

		NiAVObjectPtr spObject = itr->second;
		m_kFrozenContainer.erase(itr);

		PgActor* pkObject = NiDynamicCast(PgActor, spObject);
		if(pkObject)
		{
			PgPilot *pkPilot = g_kPilotMan.FindPilot(rkGuid);
			if(!pkPilot)
			{
				itrE = m_kUnfreezeObjectList.erase(itrE);
				continue;
			}

			// 원래 객체 컨테이너에 담고
			if(!m_kObjectContainer[WOCID_MAIN].insert(std::make_pair(rkGuid, pkObject)).second)
			{
				itrE = m_kUnfreezeObjectList.erase(itrE);
				continue;
			}

			// 파일럿 동결 해제
			pkPilot->SetFreeze(false);

			//PG_STAT(timerA.Start());
			//// 물리를 다시 생성
			//pkObject->InitPhysX(m_spPhysXScene, pkObject->GetGroupNo());
			//PG_STAT(timerA.Stop());

			PG_STAT(timerB.Start());
			// 객체 그룹 트리로
			m_aspObjectGroups[pkObject->GetGroupNo()]->AttachChild(pkObject, true);
			PG_STAT(timerB.Stop());

			PG_STAT(timerC.Start());
			pkObject->FindPathNormal();

			//	MovingVector 생성을 위해 방향을 지정해주자.
			NiPoint3	kMovingVector = pkObject->GetPathNormal().UnitCross(NiPoint3::UNIT_Z * (DIR_LEFT & DIR_LEFT ? -1.0f : 1.0f));
			pkObject->SetMovingDir(kMovingVector);

			//	정면을 보도록 한다.		
			pkObject->SetLookingDirection(DIR_DOWN, true);
			if((m_eUnlockBidirection & pkPilot->GetUnit()->UnitType()) == pkPilot->GetUnit()->UnitType())
			{
				pkObject->LockBidirection(false);
			}
			PG_STAT(timerC.Stop());
			itrE = m_kUnfreezeObjectList.erase(itrE);			
		}
		PgOutputPrint("processUnfreezeObject\n");
	}
}

void PgWorld::RemoveObjectOnNextUpdate(BM::GUID const &rkGuid)
{
	// 제거 대기큐에 등록한다.
	_PgOutputDebugString("RemoveObjectOnNextUpdate rkGuid:%s\n",MB(rkGuid.str()));

	//	이미 존재하는지 체크한다.
	RemoveObjectContainer::const_iterator itr = m_kRemoveObjectContainer.find(rkGuid);
	if(itr != m_kRemoveObjectContainer.end())
	{
		_PgOutputDebugString("RemoveObjectOnNextUpdate Already Exist In Container\n");
		return;
	}
	{
		BM::GUID kSubPlayer;
		if( PgPilotManUtil::RemoveConnectionWithSubPlayer( rkGuid, kSubPlayer ) ) 
		{// SubPlayer를 소유하고 있다면 참조를 제거하고
			RemoveObjectOnNextUpdate( kSubPlayer );
		}
	}
	m_kRemoveObjectContainer.insert(rkGuid);
}
//!	오브젝트가 실제로 월드에 존재하는지 체크한다(에러 체크용)
bool	PgWorld::CheckObjectExist(PgIWorldObject *pkObject)const
{
	for(int i=0;i<MAX_OBJ_CONT;++i)
	{
		for(ObjectContainer::const_iterator itr = m_kObjectContainer[i].begin(); itr != m_kObjectContainer[i].end(); ++itr)
		{
			if(itr->second == pkObject) return true;
		}
	}

	return	false;
}
//!	트리거가 실제로 월드에 존재하는지 체크한다(에러 체크용)
bool	PgWorld::CheckTriggerExist(PgTrigger *pkObject)
{
	TriggerContainer::iterator trigger_itr = m_kTriggerContainer.begin();
	while(trigger_itr != m_kTriggerContainer.end())
	{
		if(trigger_itr->second == pkObject)
		{
			return true;
		}
		++trigger_itr;
	}
	return	false;
}
//!	샤인스톤이 실제로 월드에 존재하는지 체크한다(에러 체크용)
bool	PgWorld::CheckShineStoneExist(PgShineStone *pkObject)
{
	ShineStoneContainer::iterator stone_itor = m_kShineStoneContainer.begin();
	while(stone_itor != m_kShineStoneContainer.end())
	{
		if(stone_itor->second->GetGuid() == pkObject->GetGuid()) 
		{
			return true;
		}
		++stone_itor;
	}
	return	false;
}

PgIWorldObject *PgWorld::FindObject(BM::GUID const& rkGuid)const
{
	// 오브젝트를 찾는다.
	for(int i=0;i<MAX_OBJ_CONT;++i)
	{
		ObjectContainer::const_iterator itr = m_kObjectContainer[i].find(rkGuid);
		if(itr == m_kObjectContainer[i].end())
		{
			continue;
		}

		return itr->second;
	}

	return	NULL;
}

PgIWorldObject *PgWorld::FindObjectByName(char const *pkObjectName)const
{
	// 오브젝트를 찾는다.
	for(int i=0;i<MAX_OBJ_CONT;++i)
	{
		ObjectContainer::const_iterator itr = m_kObjectContainer[i].begin();
		while(m_kObjectContainer[i].end() != itr)
		{
			if( NULL != (*itr).second->GetObjectByName(pkObjectName) )
			{
				return itr->second;
			}
			++itr;
		}
	}
	return	NULL;
}

void PgWorld::ClearAllObjects(bool bExceptNPC)
{
	m_kContBuilding.clear();
	m_kContEmporiaGateInfo.clear();
	m_kContHillLamp.clear();

	// 모든 오브젝트를 돌면서 지운다.
	for(int i=0;i<MAX_OBJ_CONT;++i)
	{
		ObjectContainer::iterator objIter = m_kObjectContainer[i].begin();
		while (objIter != m_kObjectContainer[i].end())
		{
			if (bExceptNPC && NiIsKindOf(PgActorNpc, objIter->second))
			{//같은 맵을 이동하는 경우이므로 NPC 살려둠
				++objIter;
				continue;
			}

			if( bExceptNPC && NiIsKindOf(PgObject, objIter->second))
			{//마찬가지로 같은 맵을 이동하는 경우 이동 오브젝트 살려둠.
				++objIter;
				continue;
			}

			if( bExceptNPC && NiIsKindOf(PgPuppet, objIter->second))
			{//마찬가지로 같은 맵을 이동하는 경우 이동 오브젝트 살려둠.
				++objIter;
				continue;
			}
			removePilot(objIter, false);
			m_kObjectContainer[i].erase(objIter++);
		}		
	}

	// FrozenContainer도 깡통으로 만든다.
	for(AVObjectContainer::iterator itr = m_kFrozenContainer.begin();
		itr != m_kFrozenContainer.end();
		++itr)
	{
		BM::GUID kGuid = itr->first;

		// 파일럿을 제거한다.
		g_kPilotMan.RemovePilot(kGuid);
	}
	m_kFrozenContainer.clear();

	if(m_pkAVObjectAlphaPM)
	{
		m_pkAVObjectAlphaPM->Release();
	}
}

PgIWorldObject* PgWorld::PickObject(ObjectGroupType iObjectGroupType)
{
	NiCamera* pkCamera = m_kCameraMan.GetCamera();
	if( pkCamera )
	{
		NiPick kPick;
		PgPickObjectMouseTypeChecker kTester(*pkCamera, kPick, iObjectGroupType);
		return PickObject(kTester);
	}
	return NULL;
}

PgIWorldObject *PgWorld::PickObjectByKeyboard()
{
	PgIWorldObject *pkPlayer = g_kPilotMan.GetPlayerActor();
	if( !pkPlayer )
	{
		return NULL;
	}

	PgPickObjectSpaceCheker kTester(*pkPlayer);
	PgIWorldObject *pkObject = NULL;
	PickObject(kTester);//아무것도 받지 않는다.

	if( kTester.m_pkClosestObject )
	{
		return kTester.m_pkClosestObject;
	}
	return NULL;
}

void PgWorld::RemoveLightObjectRecurse(NiAVObject* pkLightTree, NiNode* pkToBeLit,LIGHT_TYPE kLightType)
{
	if ( ((kLightType&LT_DIRECTIONAL || kLightType == LT_ALL) && NiIsKindOf(NiDirectionalLight, pkLightTree)) ||
		((kLightType&LT_AMBIENT || kLightType == LT_ALL) && NiIsKindOf(NiAmbientLight, pkLightTree)) ||
		((kLightType&LT_POINT || kLightType == LT_ALL) && NiIsKindOf(NiPointLight, pkLightTree)) )
	{
		((NiLight*)pkLightTree)-> DetachAffectedNode(pkToBeLit);            
	}
	else if (NiIsKindOf(NiNode, pkLightTree))
	{
		NiNode* pkNode = (NiNode*)pkLightTree;

		for (unsigned int i = 0; i < pkNode->GetArrayCount(); ++i)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);

			if (pkChild) 
			{
				RemoveLightObjectRecurse(pkChild, pkToBeLit,kLightType);
			}
		}
	}
}
void PgWorld::LightObjectRecurse(NiAVObject* pkLightTree, NiNode* pkToBeLit,LIGHT_TYPE kLightType)
{
	if ( (kLightType&LT_DIRECTIONAL || kLightType == LT_ALL) && NiIsKindOf(NiDirectionalLight, pkLightTree))
	{
		((NiLight*)pkLightTree)->AttachAffectedNode(pkToBeLit);            
	}
	else  if ((kLightType&LT_AMBIENT  || kLightType == LT_ALL) && NiIsKindOf(NiAmbientLight, pkLightTree))
	{
		((NiLight*)pkLightTree)->AttachAffectedNode(pkToBeLit);            
	}
	else if ((kLightType&LT_POINT || kLightType == LT_ALL) && NiIsKindOf(NiPointLight, pkLightTree))
	{
		NiPointLight	*pkLight = NiDynamicCast(NiPointLight,pkLightTree);
		float	fC = pkLight->GetConstantAttenuation();
		float	fL = pkLight->GetLinearAttenuation();
		float	fQ = pkLight->GetQuadraticAttenuation();

		float	fDistance = (pkToBeLit->GetWorldTranslate() - pkLight->GetWorldLocation()).Length();
		float	fAttu = (fC + fL*fDistance + fQ*fDistance*fDistance);
		if(fAttu>=0 && fAttu<100.0f)
		{
			pkLight->AttachAffectedNode(pkToBeLit);
		}
	}
	else if (NiIsKindOf(NiNode, pkLightTree))
	{
		NiNode* pkNode = (NiNode*)pkLightTree;

		for (unsigned int i = 0; i < pkNode->GetArrayCount(); ++i)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);

			if (pkChild) 
			{
				LightObjectRecurse(pkChild, pkToBeLit,kLightType);
			}
		}
	}
}

void PgWorld::SetOptimizationLevel(int iLevel)
{
	switch(iLevel)
	{
	case 0:
		{
			if (m_spOptimization_0) m_spOptimization_0->SetAppCulled(false);
			if (m_spOptimization_1) m_spOptimization_1->SetAppCulled(false);
			if (m_spOptimization_2) m_spOptimization_2->SetAppCulled(false);
		}break;
	case 1:
		{
			if (m_spOptimization_0) m_spOptimization_0->SetAppCulled(false);
			if (m_spOptimization_1) m_spOptimization_1->SetAppCulled(false);
			if (m_spOptimization_2) m_spOptimization_2->SetAppCulled(true);
		}break;
	case 2:
		{
			if (m_spOptimization_0) m_spOptimization_0->SetAppCulled(false);
			if (m_spOptimization_1) m_spOptimization_1->SetAppCulled(true);
			if (m_spOptimization_2) m_spOptimization_2->SetAppCulled(true);
		}break;
	default:
		{
			// 없는 레벨을 선택했음.
		}break;
	}
}

void PgWorld::cullObjectByRange(NiCamera* pkCamera, NodeContainer& rkContainer, float fRange)
{
	if (pkCamera == NULL)
		return;

	if (rkContainer.size() == 0)
		return;

	PG_ASSERT_LOG(fRange > 0.0f);
	NiPoint3 kCameraPos = pkCamera->GetWorldLocation();
	NiPoint3 kCameraLookAt = pkCamera->GetWorldDirection();
	kCameraLookAt.Unitize();
	kCameraLookAt.z = 0.0f; // 바닥과 평행하게 만든다.

	NiPoint3 kPlanePos = kCameraPos + kCameraLookAt * fRange;

	NiPlane kPlane(kCameraLookAt * -1.0f, kPlanePos);

	NodeContainer::iterator iter = rkContainer.begin();
	while (iter != rkContainer.end())
	{
		NiNode* pNode = iter->second;
		if (pNode != NULL)
		{
			//			float length = (pNode->GetWorldTranslate() - kCameraPos).Length();
			//			if (length < fRange)
			if (kPlane.WhichSide(pNode->GetWorldTranslate()) == NiPlane::POSITIVE_SIDE)
			{
				AddAlphaProcessObject(pNode, 1.0f, ALPHA_MANAGER_PROCESS_TYPE_ALPHA_ADD, ALPHA_MANAGER_PROCESS_TYPE_OBJECT_MAINTAIN, true);				
				//pNode->SetAppCulled(false); //AlphaProcess를 사용하지 않을때 위에 코드와 주석을 바꿔주면 됨.
			}
			else
			{
				AddAlphaProcessObject(pNode, 1.0f, ALPHA_MANAGER_PROCESS_TYPE_ALPHA_SUB, ALPHA_MANAGER_PROCESS_TYPE_OBJECT_MAINTAIN, true);				
				//pNode->SetAppCulled(true);
			}
		}
		++iter;
	}
}

void PgWorld::SetActiveLowPolygon(bool bActive, int iGlobalLOD)
{
	OptimizationPolyCont::iterator itrOptPoly = m_kOptPolyCont.begin();
	while( itrOptPoly != m_kOptPolyCont.end() )
	{
		itrOptPoly->second->SetLODActive(bActive);
		NiLODData* pLODData = itrOptPoly->second->GetLODData();
		NiLODNode* pLODNode = itrOptPoly->second;
		if (pLODData == NULL)
		{
			NILOG(PGLOG_WARNING, "%s node has no LODData\n", itrOptPoly->second->GetName());
		}
		if (pLODNode)
		{
			NILOG(PGLOG_CONSOLE, "%s node has lod data, translate %f, %f, %f\n", itrOptPoly->first.c_str(), pLODNode->GetWorldTranslate().x, pLODNode->GetWorldTranslate().y, pLODNode->GetWorldTranslate().z);
		}
		++itrOptPoly;
	}

	if (bActive)
		NiLODNode::SetGlobalLOD(iGlobalLOD);
}

void PgWorld::SetWorldLODData(float first, float second, float third)
{
	OptimizationPolyCont::iterator itrOptPoly = m_kOptPolyCont.begin();
	while (itrOptPoly != m_kOptPolyCont.end())
	{
		NiRangeLODData* pLODData = (NiRangeLODData*)itrOptPoly->second->GetLODData();
		if (pLODData)
		{
			switch(pLODData->GetNumRanges())
			{
			case 1:
				pLODData->SetRange(0, 0.0f, third);
				break;
			case 2:
				if (GET_OPTION_WORLD_QUALITY)
				{
					pLODData->SetRange(0, 0.0f, second);
					pLODData->SetRange(1, second, third);
				}
				else
				{
					pLODData->SetRange(0, 0.0f, first);
					pLODData->SetRange(1, first, third);
				}
				break;
			case 3:
				if (GET_OPTION_WORLD_QUALITY)
				{
					pLODData->SetRange(0, 0.0f, first);
					pLODData->SetRange(1, first, second);
					pLODData->SetRange(2, second, third);
				}
				else
				{
					pLODData->SetRange(0, 0.0f, 0.0f);
					pLODData->SetRange(1, 0.0f, second);
					pLODData->SetRange(2, second, third);
				}
				break;
			default:
				break;
			}

		}
		++itrOptPoly;
	}
}

//bool PgWorld::ChangeSeason(char const* pcSeasonName)
//{
//	if(m_pkSeasonMan)
//	{
//		return m_pkSeasonMan->ChangeSeason(pcSeasonName);
//	}
//	return false;
//}
//
//bool PgWorld::TransitMood(char const* pcMoodName)
//{
//	if(m_pkMoodMan)
//	{
//		return m_pkMoodMan->TransitMood(pcMoodName);
//	}
//	return false;
//}

// 아 Trigger 빼주기 싫은데 -_-...xxxxxxxxxxxxxxxx 나중에 바꾸자.
PgTrigger * PgWorld::GetTriggerByIDWithIgnoreCase(char const * kTriggerID)const
{
	if(!kTriggerID)
	{
		return NULL;
	}

	TriggerContainer::const_iterator itr = m_kTriggerContainer.begin();
	while(itr != m_kTriggerContainer.end())
	{
		std::string	const& kID = itr->first;
		if(stricmp(kID.c_str(),kTriggerID) == 0)
		{
			return itr->second;
		}
		++itr;
	}

	return	NULL;
}
PgTrigger * PgWorld::GetTriggerByID( std::string const& strTriggerID )const
{
	TriggerContainer::const_iterator itr = m_kTriggerContainer.find(strTriggerID);
	if(itr != m_kTriggerContainer.end())
	{
		return itr->second;
	}

	return NULL;
}
PgTrigger * PgWorld::GetTriggerByIndex(int const iIndex)const
{
	TriggerContainer::const_iterator itr = m_kTriggerContainer.begin();
	while(itr != m_kTriggerContainer.end())
	{
		if(itr->second->GetIndex() == iIndex)
		{
			return itr->second;
		}
		++itr;
	}

	return 0;
}

void PgWorld::GetTriggerByType(int const Type, CONT_TRIGGER_INFO & ContTriggerName)const
{
	TriggerContainer::const_iterator itr = m_kTriggerContainer.begin();
	while(itr != m_kTriggerContainer.end())
	{
		if( Type == itr->second->GetTriggerType() )
		{
			ContTriggerName.push_back( itr->second );
		}
		++itr;
	}
}

bool PgWorld::ParseXmlForShineStone(const TiXmlNode *pkNode, void *pArg)
{
	int const iType = pkNode->Type();

	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);

			char const* pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "STONE") == 0)
			{
				// Shine Stones.
				//PgShineStone* pkShineStone = (PgShineStone*)PgXmlLoader::CreateObject(pkNode);
				//if(!pkShineStone)
				//{
				//	PG_ASSERT_LOG(!"failed to parsing shinestone");
				//	return false;
				//}

				//m_kShineStoneContainer.insert(std::make_pair(pkShineStone->GetID(), pkShineStone));
			}
		}break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXmlForShineStone(pkNextNode))
		{
			return false;
		}
	}

	//// xml 파싱이 성공적으로 끝났다면,
	//if(strcmp(pkNode->Value(), "STONES") == 0)
	//{	
	//}

	return true;
}

bool PgWorld::ParseXmlForCompleteBG(const TiXmlNode *pkNode, void *pArg)
{
	int const iType = pkNode->Type();

	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);

			char const* pcTagName = pkElement->Value();

			int iID = -1;

			if(strcmp(pcTagName, "ITEM") == 0)
			{
				TiXmlAttribute *pkAttr = ((TiXmlElement*)pkNode)->FirstAttribute();

				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "ID") == 0)
					{
						iID = atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "PATH") == 0)
					{ 
						if (iID >= 0)
						{
							m_CompleteBGContainer.insert(std::make_pair(iID, std::string(pkAttr->Value())));
						}

						iID = -1;
					}
					pkAttr = pkAttr->Next();
				}
			}
		}break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXmlForCompleteBG(pkNextNode))
		{
			return false;
		}
	}

	//// xml 파싱이 성공적으로 끝났다면,
	//if(strcmp(pkNode->Value(), "STONES") == 0)
	//{	
	//}

	return true;
}

bool PgWorld::ParseXmlForNpc( TiXmlElement const * pkElement, BM::GUID &rkOutNpcGuid )
{
	if ( !pkElement )
	{
		return false;
	}

	TiXmlAttribute const *pkAttr = pkElement->FirstAttribute();
	unsigned int iNpcNo = 0;
	unsigned int iNpcKID = 0;
	char const* pcActorName = 0;
	char const* pcScriptName = 0;
	NiPoint3 kPos = NiPoint3(0,0,0);
	NiPoint3 kDir = NiPoint3(0,0,0);
	int iType = E_NPC_TYPE_FIXED;
	bool bHidden = false;
	bool bIgnoreCameraCulling = false;
	int iParam = 0;
	bool bHideMiniMap = false;
	int iMinimapIcon = 0;
	int GiveEffectNo = 0;

	while(pkAttr)
	{
		char const* pcAttrName = pkAttr->Name();
		char const* pcAttrValue = pkAttr->Value();

		if (_stricmp(pcAttrName, "NAME") == 0)
		{
			iNpcNo = (unsigned int)atoi(pcAttrValue);
		}
		else if (_stricmp(pcAttrName, "ACTOR") == 0)
		{
			pcActorName = pcAttrValue;
		}
		else if (_stricmp(pcAttrName, "SCRIPT") == 0)
		{
			pcScriptName = pcAttrValue;
		}
		else if (_stricmp(pcAttrName, "HIDDEN") == 0)
		{
			bHidden = (stricmp(pcAttrValue,"TRUE")==0);
		}
		else if (_stricmp(pcAttrName, "GUID") == 0)
		{
			rkOutNpcGuid.Set(pcAttrValue);
		}
		else if (_stricmp(pcAttrName, "KID") == 0)
		{
			iNpcKID = (unsigned int)atoi(pcAttrValue);
		}
		else if (_stricmp(pcAttrName, "LOCATION") == 0)
		{
			sscanf_s(pcAttrValue, "%f, %f, %f", &kPos.x, &kPos.y, &kPos.z);						
		}
		else if (_stricmp(pcAttrName, "DIRECTION") == 0)
		{
			sscanf_s(pcAttrValue, "%f, %f, %f", &kDir.x, &kDir.y, &kDir.z);						
		}
		else if (_stricmp(pcAttrName, "TYPE") == 0)
		{
			if (_stricmp(pcAttrValue, "FIXED") == 0)
				iType = E_NPC_TYPE_FIXED;
			else if (_stricmp(pcAttrValue, "DYNAMIC") == 0)
				iType = E_NPC_TYPE_DYNAMIC;
			else if (_stricmp(pcAttrValue, "CREATURE") == 0)
				iType = E_NPC_TYPE_CREATURE;
			else
			{
				NILOG( PGLOG_WARNING, "[PgWorld] %s NPC has unknown type(%s)\n", pcActorName, pcAttrValue );
			}
		}
		else if (_stricmp(pcAttrName, "IGNORE_CAMERA_CULLING") == 0)
		{
			bIgnoreCameraCulling = (stricmp(pcAttrValue,"TRUE")==0);
		}
		else if ( _stricmp(pcAttrName, "PARAM") == 0 )
		{
			iParam = ::atoi( pcAttrValue );
		}
		else if ( _stricmp(pcAttrName, "HIDE_MINIMAP") == 0 )
		{
			int iHideMiniMap = atoi( pcAttrValue );
			bHideMiniMap = (iHideMiniMap ? true : false);
		}
		else if ( _stricmp(pcAttrName, "MINIMAP_ICON") == 0 )
		{
			iMinimapIcon = ::atoi( pcAttrValue );
		}
		else if ( _stricmp(pcAttrName, "GIVE_EFFECTNO") == 0 )
		{
			GiveEffectNo = ::atoi( pcAttrValue );
		}

		pkAttr = pkAttr->Next();
	}

	ContSayItem kVec;
	TiXmlElement const *pkChild = pkElement->FirstChildElement("SAY");//
	while(pkChild)
	{
		//<SAY TYPE="IDLE" TTW="" UPTIME="" DELAY="" ACTION="" MIN_LEVEL="" MAX_LEVEL="" FACE="" COMPLETE_QUEST="" NOT_COMPLETE_QUEST=""/>
		SSayItem kItem;
		if( kItem.ParseXml(pkChild) )
		{
			kVec.push_back(kItem);
		}
		//
		pkChild = pkChild->NextSiblingElement();
	}

	PgPilot *pkPilot = addNpcToWorld(iNpcNo, iNpcKID, rkOutNpcGuid, pcActorName, pcScriptName, kPos, kDir, iType, bHidden, GiveEffectNo, bHideMiniMap );
	if( pkPilot )
	{
		if ( iParam )
		{
			pkPilot->SetAbil( AT_CUSTOMDATA1, iParam );
		}

		if( iMinimapIcon )
		{
			pkPilot->SetAbil( AT_DISPLAY_MINIMAP_EFFECT, iMinimapIcon );
		}

		PgActorNpc *pkActorNpc = dynamic_cast<PgActorNpc*>(pkPilot->GetWorldObject());
		if(pkActorNpc)
		{
			pkActorNpc->AddSayAction(kVec);
			pkActorNpc->SetIgnoreCameraCulling(bIgnoreCameraCulling);
		}

		return true;
	}
	return false;
}

bool PgWorld::ParseXmlForTrigger(TiXmlNode const* pkNode, PgTrigger *&pkTrigger)
{
	pkTrigger = dynamic_cast<PgTrigger*>(PgXmlLoader::CreateObject(pkNode));
	if( pkTrigger )
	{
		NiAVObjectPtr	spTriggerObj = m_spSceneRoot->GetObjectByName(pkTrigger->GetID().c_str());
		if (spTriggerObj)
		{
			pkTrigger->SetTriggerObject(spTriggerObj);
		}
		else
		{
			NILOG(PGLOG_WARNING, "[PgWorld] %s world %s tirgger has no object\n", GetID().c_str(), pkTrigger->GetID().c_str());
		}

		bool bNoHiddenPortal = true;
		if(spTriggerObj)
		{
			bNoHiddenPortal = (spTriggerObj->GetExtraData("Hidden_Portal")==NULL);
		}

		//	트리거 텍스트 추가
		if(pkTrigger->GetTriggerTitleTextID()>0)
		{
			if(spTriggerObj && bNoHiddenPortal)
			{

				bool bAddTitleText = pkTrigger->Enable();

#ifndef EXTERNAL_RELEASE
				if(g_pkApp->IsSingleMode())
				{
					if(false == lua_tinker::call<bool>("ShowTriggerTitleText"))
					{
						bAddTitleText = false;
					}
				}
#endif

				if(bAddTitleText)
				{
					NiPoint3	kTriggerPos = spTriggerObj->GetWorldTranslate();
					AddWorldText(kTriggerPos+pkTrigger->GetTitleTextAdjust(),
						TTW(pkTrigger->GetTriggerTitleTextID()),
						pkTrigger->GetTriggerTitleTextScale(),
						pkTrigger->GetTriggerTitleTextColor(),
						pkTrigger->GetTriggerTitleTextFontName());
				}
			}
		}

		// 이미 있는지 체크, 어떻게 처리할까..
		if(spTriggerObj && bNoHiddenPortal)
		{
			TriggerContainer::iterator iter = m_kTriggerContainer.find(pkTrigger->GetID());
			if (iter != m_kTriggerContainer.end())
			{
				PgError1("[PgWorld] %s trigger is already exist!", pkTrigger->GetID().c_str());
			}

			pkTrigger->Initialize();
			pkTrigger->SetIndex(m_iTriggerIndexOrder++);			
			m_kTriggerContainer.insert(std::make_pair(pkTrigger->GetID(), pkTrigger));
		}
		return true;
	}

	PG_ASSERT_LOG(!"failed to parsing trigger");
	return false;
}

bool PgWorld::ParseXmlForProgress(TiXmlElement const * pkElement)
{
	if ( !pkElement )
	{
		return false;
	}

	TiXmlAttribute const *pkAttr = pkElement->FirstAttribute();

	int iID = -1;
	PROGRESSINFO kProgress;
	while(pkAttr)
	{
		char const* pcAttrName = pkAttr->Name();
		char const* pcAttrValue = pkAttr->Value();

		if(_stricmp(pcAttrName, "VALUE") == 0)
		{
			iID = static_cast<int>( atoi(pcAttrValue) );
		}
		else if(_stricmp(pcAttrName, "X") == 0)
		{
			kProgress.ix = atoi(pcAttrValue);
		}
		else if(_stricmp(pcAttrName, "Y") == 0)
		{
			kProgress.iy = atoi(pcAttrValue);
		}
		else if(_stricmp(pcAttrName, "DIRECTION") == 0)
		{
			if(_stricmp(pcAttrValue, "LEFT") == 0)
			{
				kProgress.eDir = PROGRESSINFO::ED_LEFT;
			}
			else if(_stricmp(pcAttrValue, "RIGHT") == 0)
			{
				kProgress.eDir = PROGRESSINFO::ED_RIGHT;
			}
			else
			{
				PgError1("[PgWorld] %d Progress Direction is incorrect!", iID);
				return false;
			}
		}
		else if(_stricmp(pcAttrName, "TYPE") == 0)
		{
			if(_stricmp(pcAttrValue, "NONE") == 0)
			{
				kProgress.kType = PROGRESSINFO::ET_NONE;
			}
			else
			{
				kProgress.kType ^= PROGRESSINFO::ET_NONE;
				if(_stricmp(pcAttrValue, "START") == 0)
				{
					kProgress.kType = PROGRESSINFO::ET_START;
				}
				else if(_stricmp(pcAttrValue, "BOSS") == 0)
				{
					kProgress.kType = PROGRESSINFO::ET_BOSS;
				}
				else if(_stricmp(pcAttrValue, "MAIN_ROUTE") == 0)
				{
					kProgress.kType = PROGRESSINFO::ET_MAIN_ROUTE;
				}
				else if(_stricmp(pcAttrValue, "SUB_ROUTE") == 0)
				{
					kProgress.kType = PROGRESSINFO::ET_SUB_ROUTE;
				}
			}
		}

		pkAttr = pkAttr->Next();
	}
	if(iID == -1)
	{
		PgError("[PgWorld] Cannot find Progress id!");
		return false;
	}

	if(false == g_kProgressMap.AddProgressArea(iID, kProgress))
	{
		//MSG: 동일한 PROGRESS 번호가 존재함
		PgError1("[PgWorld] %d Progress is already exist!", iID);
		return false;
	}

	return true;
}

bool PgWorld::ParseXmlForProgressWay(TiXmlElement const * pkElement)
{
	if ( NULL == pkElement ) { return false; }

	TiXmlAttribute const *pkAttr = pkElement->FirstAttribute();
	PROGRESSINFO kProgress;

	int iFrom = 0;
	int iTo = 0;
	PROGRESS_WAY_INFO kNewElem;
	while(pkAttr)
	{
		char const* pcAttrName = pkAttr->Name();
		char const* pcAttrValue = pkAttr->Value();

		if(_stricmp(pcAttrName, "FROM") == 0)
		{
			kNewElem.iFrom = static_cast<int>( atoi(pcAttrValue) );
		}
		else if(_stricmp(pcAttrName, "TO") == 0)
		{
			kNewElem.iTo = static_cast<int>( atoi(pcAttrValue) );
		}
		else if(_stricmp(pcAttrName, "TYPE") == 0)
		{
			if(_stricmp(pcAttrValue, "MAIN_ROUTE") == 0)
			{
				kNewElem.kType = PROGRESS_WAY_INFO::ET_MAIN_ROUTE;
			}
			else if(_stricmp(pcAttrValue, "SUB_ROUTE") == 0)
			{
				kNewElem.kType = PROGRESS_WAY_INFO::ET_SUB_ROUTE;
			}
			else if(_stricmp(pcAttrValue, "CUTTED_ROUTE") == 0)
			{
				kNewElem.kType = PROGRESS_WAY_INFO::ET_CUTTED_ROUTE;
			}
			else
			{
				kNewElem.kType = PROGRESS_WAY_INFO::ET_NONE;
			}
		}

		pkAttr = pkAttr->Next();
	}

	if(kNewElem.kType != PROGRESS_WAY_INFO::ET_NONE)
	{
		if(g_kProgressMap.AddProgressWay(kNewElem) == false)
		{
			PgError("[PgWorld] %d Failed To Get Progress Way!");
			return false;
		}
	}

	return true;
}

bool PgWorld::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	g_kWorldEventClientMgr.ClearParsedWorldEvent();
	g_kProgressMap.Cleanup();
	
	ParseWorldXml(pkNode,pArg);		// 월드 XML 파싱후
	//CheckCameraTrigger();			// 카메라 트리거가 정상적인지 체크
	//CheckCamTrg();
	
	return	true;
}

void PgWorld::CheckCameraTrigger()
{// 카메라 트리거가 정상적인지 체크
#ifndef EXTERNAL_RELEASE
	{//카메라 트리거 체크 
		struct SCamTrgCheckInfo
		{
			int iInCnt;
			int iOutCnt;
			SCamTrgCheckInfo()
				:iInCnt(0)
				,iOutCnt(0)
			{}
		};

		typedef std::map<int, SCamTrgCheckInfo> CONT_CAMTRG_INFO;
		CONT_CAMTRG_INFO kContCheckInfo;

		TriggerContainer::const_iterator kTrg_itor = m_kTriggerContainer.begin();
		while(kTrg_itor != m_kTriggerContainer.end())
		{// 카메라 트리거 정보 뽑기
			PgTrigger* pkTrg = kTrg_itor->second;
			if(pkTrg)
			{
				std::string kCamType = CheckCamTrgType(pkTrg);
				if(!kCamType.empty())
				{// 카메라 타입 트리거이면
					int iGroup = static_cast<int>(pkTrg->GetCamTrgInfoGroup());
					CONT_CAMTRG_INFO::iterator kInfo_itor = kContCheckInfo.find(iGroup);
					if(kContCheckInfo.end() == kInfo_itor)
					{
						std::pair<CONT_CAMTRG_INFO::iterator, bool> kRet = kContCheckInfo.insert(std::make_pair(iGroup, SCamTrgCheckInfo()));
						kInfo_itor = kRet.first;
					}

					if(CAMTRG_IN_TYPE == kCamType)
					{// in과
						++(kInfo_itor->second.iInCnt);
					}
					else if(CAMTRG_OUT_TYPE == kCamType)
					{// out의 갯수를 센다
						++(kInfo_itor->second.iOutCnt);
					}
					else
					{// single

					}
				}
			}
			++kTrg_itor;
		}

		// 카메라 트리거 in, out 갯수 체크
		CONT_CAMTRG_INFO::const_iterator kInfo_itor = kContCheckInfo.begin();
		while(kInfo_itor != kContCheckInfo.end())
		{
			int const& iInCnt = kInfo_itor->second.iInCnt;
			int const& iOutCnt = kInfo_itor->second.iOutCnt;
			std::string const& kWorldID = this->GetID();
			if(iOutCnt < iInCnt)
			{// out 카메라 트리거가 in 보다 많다면 Error
				PgMessageBox("CameraTrigger", "Check MAP ID:%s\nGROUP:%d camera trigger. It need more \'OUT\' CAMERA_TYPE", kWorldID.c_str(), kInfo_itor->first);
			}

			if(iInCnt == 0)
			{//in 이 없는 경우
				PgMessageBox("CameraTrigger", "Check MAP ID:%s\nGROUP:%d camera trigger. None exist \'IN\' CAMERA_TYPE", kWorldID.c_str(), kInfo_itor->first);
			}
			++kInfo_itor;
		}
	}
#endif
}

TiXmlNode const* PgWorld::ParseWorldXml(TiXmlNode const *pkNode, void *pArg)
{
	int const iType = pkNode->Type();

	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);

			std::string kTagName(pkElement->Value());

			if( kTagName == "LOCAL" )
			{
				TiXmlNode const* pkFindLocalNode = PgXmlLocalUtil::FindInLocal(g_kLocal, pkElement, "PgWorld");
				if( pkFindLocalNode )
				{
					TiXmlNode const* pkResultNode = pkFindLocalNode->FirstChildElement();
					if( pkResultNode )
					{
						do
						{//! switch문 다음에 pkNode->NextSibling(); 호출하므로, 
						//! 여기서는 재귀호출만 해주면 된다.
							pkResultNode = ParseWorldXml( pkResultNode );
						}
						while(pkResultNode);
					}
				}
			}
			else if(kTagName == "TIME_EVENT")
			{
				m_TimeEventLoader.ReadFromXmlElement(pkElement);
			}
			else if(kTagName == "WORLD")
			{
				m_TimeEventLoader.ClearID();
				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "ID") == 0)
					{
						SetID(pcAttrValue);
					}
					else if (_stricmp(pcAttrName, "DESCRIPTION") == 0)
					{
						//북용 데이터
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}

				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					while((pkChildNode = ParseWorldXml(pkChildNode)));
				}
			}
			else if(kTagName == "NIFPATH")
			{
				// NIF를 로딩한다.
				if(!LoadNif(pkElement->GetText()))
				{
					PgError1("World '%s' Can't Load", pkElement->GetText());
					return NULL;
				}
			}
			else if (kTagName == "MINIMAP")
			{
				m_kMiniMapImage = pkElement->GetText();
				PG_ASSERT_LOG(m_kMiniMapImage.empty() == false);

				NiPoint3 minimapCameraPos(0,0,0);
				NiPoint3 minimapRotation(0,0,0);
				float orthoWidth = 0.0f;

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "Y") == 0)
					{
						m_kDrawHeight.x = atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "H") == 0)
					{
						m_kDrawHeight.y = atoi(pkAttr->Value());
					}

					pkAttr = pkAttr->Next();
				}

				if (m_spMinimapCamera == NULL)
				{
					m_spMinimapCamera = NiNew NiCamera;
					NiFrustum kFrustum = m_spMinimapCamera->GetViewFrustum();
					kFrustum.m_fNear = 10.0f;
					kFrustum.m_fFar = 100000.0f;
					if (orthoWidth > 0)
					{
						kFrustum.m_bOrtho = true;
						kFrustum.m_fLeft = -orthoWidth / 2;
						kFrustum.m_fRight = orthoWidth / 2;
						kFrustum.m_fTop = (float)((orthoWidth / 1.3333) / 2);
						kFrustum.m_fBottom = (float)(-(orthoWidth / 1.3333) / 2);
					}

					m_spMinimapCamera->SetViewFrustum(kFrustum);

					// 모델을 찍을 카메라를 설정한다.
					m_spMinimapCamera->SetTranslate(minimapCameraPos);
					m_spMinimapCamera->Update(0.0f);
					NiMatrix3 rot, rot2, rot3, rot4, rot5;
					rot.FromEulerAnglesXYZ(minimapRotation.x, minimapRotation.y, minimapRotation.z);
					rot2.FromEulerAnglesXZY(minimapRotation.x, minimapRotation.z, minimapRotation.y);
					rot3.FromEulerAnglesYXZ(minimapRotation.y, minimapRotation.x, minimapRotation.z);
					rot4.FromEulerAnglesYZX(minimapRotation.y, minimapRotation.z, minimapRotation.x);
					rot5.FromEulerAnglesZXY(minimapRotation.z, minimapRotation.x, minimapRotation.y);
					m_spMinimapCamera->SetRotate(rot);
					m_spMinimapCamera->Update(0.0f);
					//m_spMinimapCamera->UpdateWorldBound();
					//m_spMinimapCamera->UpdateWorldData();
				}
			}
			else if(kTagName == "PROGRESS_MAP")
			{
				PROGRESS_MAP_UI_INFO kMapInfo;

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				kMapInfo.wstrImgPath = UNI(pkElement->GetText());
				while(pkAttr)
				{
					if(  strcmp(pkAttr->Name(), "SHOW_NPCLIST") == 0)
					{
						kMapInfo.bShowNpcList = (atoi(pkAttr->Value()) == 1);
					}
					else if(strcmp(pkAttr->Name(), "W") == 0)
					{
						kMapInfo.iImgW = atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "H") == 0)
					{
						kMapInfo.iImgH = atoi(pkAttr->Value());
					}

					pkAttr = pkAttr->Next();
				}

				g_kProgressMap.SetMainUI(kMapInfo, false);
				//m_kProgressMinimap.Initialize(kMapInfo);
			}
			else if(kTagName == "GSAPATH")
			{
				std::string	kGsaPath;

				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode && pkChildNode->Type() == TiXmlNode::ELEMENT)
				{
					GetGsaPath(pkChildNode,kGsaPath);
				}
				else
				{
					kGsaPath = pkElement->GetText();
				}

				// GSA를 로딩한다.
				if ( LoadGsa(kGsaPath.c_str()) == false )
				{
					PgError1("World '%s' Can't Load", pkElement->GetText());
					return NULL;
				}
			}
			else if(kTagName == "BACKGROUND_COLOR")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				m_kBGColor = DEF_BG_COLOR;
				NiColor kColor = DEF_BG_COLOR;
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "RED") == 0)
					{
						kColor.r = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "GREEN") == 0)
					{
						kColor.g = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "BLUE") == 0)
					{
						kColor.b = (float)atof(pcAttrValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}
					pkAttr = pkAttr->Next();
				}
				m_kBGColor = kColor;
			}
			else if(kTagName == "CAMERA_MODE")
			{
				// CameraMode를 바꾼다.
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				int iCameraMode = 0;
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "MODE") == 0)
					{
						iCameraMode = (unsigned int)atoi(pcAttrValue);
					}
					pkAttr = pkAttr->Next();
				}

				PgActor* pkActor = g_kPilotMan.GetPlayerActor();
				if ((PgCameraMan::ECameraMode)iCameraMode != PgCameraMan::CMODE_FOLLOW || pkActor)
				{
					m_kCameraMan.SetCameraMode((PgCameraMan::ECameraMode)iCameraMode, pkActor);
				}
				//else
				//{
				//	if (m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW ||
				//		m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW_DEPTH_PATH)
				//	{
				//		PgCameraModeFollow* pkCam = (PgCameraModeFollow*)m_kCameraMan.GetCameraMode();

				//		if (iCameraMode == PgCameraMan::CMODE_FOLLOW)
				//			pkCam->SetCameraMoveType(PgCameraModeFollow::MOVETYPE_DEPTH_FOLLOW_ACTOR);
				//		else if (iCameraMode == PgCameraMan::CMODE_FOLLOW_DEPTH_PATH)
				//			pkCam->SetCameraMoveType(PgCameraModeFollow::MOVETYPE_DEPTH_FOLLOW_PATH);
				//	}
				//}

			}
			else if(kTagName == "FREE_DIRECTION")
			{
				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttr = pkAttr->Name();
					char const* pcValue = pkAttr->Value();
					bool bTrue = (strcmp(pcValue, "TRUE") == 0);

					if(strcmp(pcAttr, "PLAYER") == 0)
					{
						m_eUnlockBidirection = (bTrue ? m_eUnlockBidirection | UT_PLAYER : m_eUnlockBidirection & ~UT_PLAYER);
					}
					else if(strcmp(pcAttr, "MONSTER") == 0)
					{
						m_eUnlockBidirection = (bTrue ? m_eUnlockBidirection | UT_MONSTER : m_eUnlockBidirection & ~UT_MONSTER);
					}
					else if(strcmp(pcAttr, "NPC") == 0)
					{
						m_eUnlockBidirection = (bTrue ? m_eUnlockBidirection | UT_NPC : m_eUnlockBidirection & ~UT_NPC);
					}
					else if(strcmp(pcAttr, "BOSS") == 0)
					{
						m_eUnlockBidirection = (bTrue ? m_eUnlockBidirection | UT_BOSSMONSTER : m_eUnlockBidirection & ~UT_BOSSMONSTER);
					}
					else if(strcmp(pcAttr, "PET") == 0)
					{
						m_eUnlockBidirection = (bTrue ? m_eUnlockBidirection | UT_PET : m_eUnlockBidirection & ~UT_PET);
					}
					else if(strcmp(pcAttr, "ALL") == 0)
					{
						m_eUnlockBidirection = (bTrue ? 0xffffffff : 0x0);
					}
					pkAttr = pkAttr->Next();
				}
			}
			else if(kTagName == "WAVE_EFFECT")
			{
				std::string kEntityID,kStopEffectID,kMoveEffectID;

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttr = pkAttr->Name();
					char const* pcValue = pkAttr->Value();

					if(strcmp(pcAttr, "ENTITY_ID") == 0)
					{
						kEntityID = pcValue;
					}
					else if(strcmp(pcAttr, "STOP_EFFECT_ID") == 0)
					{
						kStopEffectID = pcValue;
					}
					else if(strcmp(pcAttr, "MOVE_EFFECT_ID") == 0)
					{
						kMoveEffectID = pcValue;
					}
					pkAttr = pkAttr->Next();
				}

				AddWaveEffect(kEntityID.c_str(),kStopEffectID.c_str(),kMoveEffectID.c_str());
			}
			else if(kTagName == "CAMERA_ADJUST")
			{
				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					float const fValue = static_cast<float>( ::atof(pkAttr->Value()) );
					if(0 == ::strcmp(pkAttr->Name(), "UP"))
					{
						m_kCameraAdjustInfo.kCameraAdjust.fCameraHeight = fValue;
					}
					else if(0 == ::strcmp(pkAttr->Name(), "WIDTH"))
					{
						m_kCameraAdjustInfo.kCameraAdjust.fCameraWidth = fValue;
					}
					else if(0 == ::strcmp(pkAttr->Name(), "ZOOM"))
					{
						m_kCameraAdjustInfo.kCameraAdjust.fDistanceFromTarget = fValue;
					}
					else if(0 == ::strcmp(pkAttr->Name(), "TARGET_X"))
					{
						m_kCameraAdjustInfo.kCameraAdjust.fLookAtWidth = fValue;
					}
					else if(0 == ::strcmp(pkAttr->Name(), "TARGET_Y"))
					{
						m_kCameraAdjustInfo.kCameraAdjust.fLookAtDepth = fValue;
					}
					else if(0 == ::strcmp(pkAttr->Name(), "TARGET_Z"))
					{
						m_kCameraAdjustInfo.kCameraAdjust.fLookAtHeight = fValue;
					}
					else if(0 == ::strcmp(pkAttr->Name(), "MIN_ZOOM"))
					{
						m_kCameraAdjustInfo.fMinZoomPos = fValue;
					}
					else if(0 == ::strcmp(pkAttr->Name(), "MAX_ZOOM"))
					{
						m_kCameraAdjustInfo.fMaxZoomPos = fValue;
					}
					else if(0 == ::strcmp(pkAttr->Name(), "WHEEL_MIN_ZOOM"))
					{
						m_kCameraAdjustInfo.fMinZoomDelta = fValue;
					}
					else if(0 == ::strcmp(pkAttr->Name(), "WHEEL_MAX_ZOOM"))
					{
						m_kCameraAdjustInfo.fMaxZoomDelta = fValue;
					}
					else if(0 == ::strcmp(pkAttr->Name(), "NOT_CULLING"))
					{
						m_bNotCulling = static_cast<bool>(fValue);
					}

					pkAttr = pkAttr->Next();
				}
			}
			else if(kTagName == "EVENT_SCRIPT_ON_ENTER")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(stricmp(pcAttrName, "ID") == 0)
					{
						m_kEventScriptOnEnter.m_iEventScriptIDOnEnter = PgStringUtil::SafeAtoi(pcAttrValue);
					}
					else if(stricmp(pcAttrName, "PARTY_BREAK_IN_ID") == 0)
					{
						m_kEventScriptOnEnter.m_iEventScriptIDOnEnterByPartyBreakIn = PgStringUtil::SafeAtoi(pcAttrValue);
					}
					else if(stricmp(pcAttrName, "LIMIT_QUEST") == 0)
					{
						std::wstring kTempStr = UNI(pcAttrValue);
						VEC_WSTRING kVec;
						PgStringUtil::BreakSep(kTempStr, kVec, L"/");
						VEC_WSTRING::const_iterator loop_iter = kVec.begin();
						while( kVec.end() != loop_iter )
						{
							int iQuestNo = PgStringUtil::SafeAtoi((*loop_iter));
							m_kEventScriptOnEnter.m_kContLimit_Quest.push_back(iQuestNo);
							++loop_iter;
						}
					}
					else if(stricmp(pcAttrName, "LIMIT_CLASS") == 0)
					{
						std::wstring kTempStr = UNI(pcAttrValue);
						VEC_WSTRING kVec;
						PgStringUtil::BreakSep(kTempStr, kVec, L"/");
						VEC_WSTRING::const_iterator loop_iter = kVec.begin();
						while( kVec.end() != loop_iter )
						{
							int iClassNo = PgStringUtil::SafeAtoi((*loop_iter));
							m_kEventScriptOnEnter.m_kContLimit_Class.push_back(iClassNo);
							++loop_iter;
						}
					}
					else if(stricmp(pcAttrName, "LIMIT_LEVEL_MIN") == 0)
					{
						m_kEventScriptOnEnter.m_iLimitLevelMin = PgStringUtil::SafeAtoi(pcAttrValue);
					}
					else if(stricmp(pcAttrName, "LIMIT_LEVEL_MAX") == 0)
					{
						m_kEventScriptOnEnter.m_iLimitLevelMax = PgStringUtil::SafeAtoi(pcAttrValue);
					}
					else if(stricmp(pcAttrName, "PLAY_ONCE") == 0)
					{
						bool bPlayOnce = PgStringUtil::SafeAtoi(pcAttrValue);
						m_kEventScriptOnEnter.m_bPlayOnce = bPlayOnce;
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}
			}
			else if(kTagName == "MAP_MOVE_COMPLETE_FADE")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(stricmp(pcAttrName, "FADE_COLOR") == 0)
					{
						m_kMapMoveCompleteFade.SetFadeColor(pcAttrValue);
					}
					else if(stricmp(pcAttrName, "FADE_TIME") == 0)
					{
						float const fValue = static_cast<float>( ::atof(pkAttr->Value()) );
						m_kMapMoveCompleteFade.fFadeTime = fValue;
					}
					else if(stricmp(pcAttrName, "FADE_IN") == 0)
					{
						m_kMapMoveCompleteFade.bFadeIn = (stricmp(pcAttrValue,"TRUE")==0);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}
			}

			else if(kTagName == "MAPVALUE")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "PLAYERKILL") == 0)//앞으로 사용안할 예정이다.
					{
					}
					else if(strcmp(pcAttrName, "TYPE") == 0)
					{
					}
					else if(strcmp(pcAttrName, "MAP_DISCRIPTION_TEXT_NUM") == 0)
					{
						m_iMapDisciptionNo = atoi(pcAttrValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}
			}
			else if(kTagName == "SCRIPT")
			{
				m_kScript = pkElement->GetText();

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "UPDATE") == 0 )
					{
						m_kUpdateScript = pkAttr->Value();
					}
					else if(strcmp(pkAttr->Name(), "INIT") == 0 )
					{
						m_kPostScript = pkAttr->Value();
					}
					else if( strcmp(pkAttr->Name(), "START") == 0 )
					{
						m_kStartScript = pkAttr->Value();
					}
					else if( strcmp(pkAttr->Name(), "READY") == 0 )
					{
						m_kReadyScript = pkAttr->Value();
					}
					//보스 죽을 경우 처리되는 것들
					else if( strcmp(pkAttr->Name(), "BOSS_DIE") == 0 )
					{
						m_kBossDie = pkAttr->Value();
					}
					else if( strcmp(pkAttr->Name(), "ADD_SCENE") == 0 )
					{
						m_kAddScript = pkAttr->Value();
					}
					else if( strcmp(pkAttr->Name(), "REMOVE_SCENE") == 0 )
					{
						m_kRemoveScript = pkAttr->Value();
					}
					else if( strcmp(pkAttr->Name(), "TERMINATE") == 0 )
					{
						m_kTerminateScript = pkAttr->Value();
					}

					pkAttr = pkAttr->Next();
				}
			}
			else if(kTagName == "BGSOUND")
			{
				char const* pcSndPath = 0;
				float fVolume = 1.0;

				// NIF 경로를 알아낸다.
				pcSndPath = pkElement->GetText();

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				int iID = 0;
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "VOLUME") == 0)
					{
						fVolume = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ID") == 0)
					{
						iID = atoi(pcAttrValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}
				
				if(NULL!=pcSndPath)
				{
					int const iTimeEventID = atoi( m_TimeEventLoader.GetID().c_str() );
					if(iTimeEventID == iID)
					{
						m_kBgSoundContainer.insert(std::make_pair(std::string(pcSndPath), fVolume));
					}
				}
				// 배경 음원을 로딩한다.
				//g_kSoundMan.LoadBgSound( pcSndPath, fVolume );
			}
			else if(kTagName == "BGSOUND_SUB")
			{
				char const* pcSndPath = 0;
				float fVolume = 1.0f;
				int iLoop = 0;

				// NIF 경로를 알아낸다.
				pcSndPath = pkElement->GetText();

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "VOLUME") == 0)
					{
						fVolume = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "LOOP") == 0)
					{
						iLoop = (int)atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ID") == 0)
					{
						if(0!=strcmp(m_TimeEventLoader.GetID().c_str(), pcAttrValue))
						{
							pcSndPath = NULL;
							break;
						}
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}

				// 배경 음원을 로딩한다.
				if(NULL!=pcSndPath)
				{
					g_kSoundMan.LoadBgSound( pcSndPath, fVolume, iLoop, false );
				}
			}
			else if(kTagName == "BLOOM")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					PgPostProcessMan *pPPM = g_kRenderMan.GetRenderer()->m_pkPostProcessMan;
					PgIPostProcessEffect *pEffect = pPPM->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);

					if(pEffect)
					{
						if(strcmp(pcAttrName, "BLUR") == 0)
						{
							pEffect->SetParam("BlurWidth",(float)atof(pcAttrValue));
						}
						else if(strcmp(pcAttrName, "BRIGHT") == 0)
						{
							pEffect->SetParam("Brightness",(float)atof(pcAttrValue));
						}
						else if(strcmp(pcAttrName, "SRC") == 0)
						{
							pEffect->SetParam("SceneIntensity",(float)atof(pcAttrValue));
						}
						else if(strcmp(pcAttrName, "GLOW") == 0)
						{
							pEffect->SetParam("GlowIntensity",(float)atof(pcAttrValue));
						}
						else if(strcmp(pcAttrName, "HIGHLIGHT") == 0)
						{
							pEffect->SetParam("HighlightIntensity",(float)atof(pcAttrValue));
						}
						else
						{
							PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
						}
					}

					pkAttr = pkAttr->Next();
				}
			}
			else if(kTagName == "FOG")
			{
				PgWorldElementFog	kFogElement;
				if(true == kFogElement.ReadFromXmlElement(pkNode))
				{
					if( false == m_TimeEventLoader.GetID().empty() )
					{ // 만약 타임이벤트에 해당하는 FOG가 존재하면 다른 FOG는 무시
						if( kFogElement.GetID() == m_TimeEventLoader.GetID() )
						{
							UseFog(true,false);
							ApplyFogElement(kFogElement);
						}
					}
					else
					{ // 타임이벤트가 없으면 디폴트 지정
						if( kFogElement.GetID().empty() )
						{
							UseFog(true,false);
							ApplyFogElement(kFogElement);
						}
					}
				}
			}	
			else if(kTagName == "WORLD_OBJECT_LOD")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "HIGH_END") == 0)
					{
						m_afWorldObjectLODRange[0] = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "MID_END") == 0)
					{
						m_afWorldObjectLODRange[1] = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "LOW_END") == 0)
					{
						m_afWorldObjectLODRange[2] = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "USE") == 0)
					{
						m_bUseWorldObjectLOD = atoi(pcAttrValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}				
			}
			else if(kTagName == "WORLD_QUALITY")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "HIGH_END") == 0)
					{
						m_afWorldQualityRange[0] = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "MID_END") == 0)
					{
						m_afWorldQualityRange[1] = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "LOW_END") == 0)
					{
						m_afWorldQualityRange[2] = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "USE") == 0)
					{
						m_bUseWorldQualityRange = atoi(pcAttrValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}				
			}
			else if(kTagName == "VIEW_DISTANCE")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "HIGH_END") == 0)
					{
						m_afViewDistanceRange[0] = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "MID_END") == 0)
					{
						m_afViewDistanceRange[1] = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "LOW_END") == 0)
					{
						m_afViewDistanceRange[2] = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "USE") == 0)
					{
						m_bUseViewDistanceRange = atoi(pcAttrValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}				
			}
			else if(kTagName == "ROPE")
			{
				std::string	kStartNode,kTargetNode,kName;
				float	fRange = 1.0f;
				

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "START") == 0)
					{
						kStartNode = pcAttrValue;
					}
					else if(strcmp(pcAttrName, "TARGET") == 0)
					{
						kTargetNode = pcAttrValue;
					}
					else if(strcmp(pcAttrName, "ID") == 0)
					{
						kName = pcAttrValue;
					}
					else if(strcmp(pcAttrName, "RANGE") == 0)
					{
						fRange = (float)atof(pcAttrValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}		
				NiAVObject* 	pkStartNode = GetSceneRoot()->GetObjectByName(kStartNode.c_str());
				NiAVObject* 	pkTargetNode = GetSceneRoot()->GetObjectByName(kTargetNode.c_str());
				if(pkStartNode && pkTargetNode)
				{
					AddRope(kName.c_str(),pkStartNode->GetWorldTranslate(),pkTargetNode->GetWorldTranslate(),fRange);
				}

			}
			else if(kTagName == "OBJECT")
			{
				int	iClassNo = -1;
				int	kObjAttr = EObj_Attr_None;
				int	iMoveSpeed = 100;
				BM::GUID kGuid;

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(stricmp(pcAttrName, "CLASS_NO") == 0)
					{
						iClassNo = atoi(pcAttrValue);
					}
					else if(stricmp(pcAttrName, "RIDABLE") == 0)
					{
						kObjAttr |= EObj_Attr_CanRide;
					}
					//else if(stricmp(pcAttrName, "TRAPLIKE") == 0)
					//{
					//	kObjAttr |= EObj_Attr_TrapLike;
					//}
					else if(stricmp(pcAttrName, "BREAKABLE") == 0)
					{
						kObjAttr |= EObj_Attr_Breakable;
					}
					else if(stricmp(pcAttrName, "GUID") == 0)
					{
						kGuid.Set(UNI(pcAttrValue));
					}
					else if(stricmp(pcAttrName,"SPEED") == 0)
					{
						iMoveSpeed = atoi(pcAttrValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}						

				PgPilot *pkPilot = g_kPilotMan.NewPilot(kGuid,iClassNo,0,"OBJECT");
				CUnit	*pkUnit = g_kTotalObjMgr.CreateUnit(UT_OBJECT, kGuid);
				if(pkPilot && pkUnit)
				{
					pkUnit->SetAbil(AT_CLASS, iClassNo);
					pkUnit->SetAbil(AT_GENDER, 0);

					SObjUnitGroupIndex	const kIndex;
					SObjUnitBaseInfo	kBaseInfo(kIndex);
					SObjUnitCreateInfo kInfo(kBaseInfo);

					kInfo.kGuid = kGuid;
					kInfo.iID = iClassNo;
					kInfo.dwAttribute = kObjAttr;

					pkUnit->Create(&kInfo);

					pkUnit->SetAbil(AT_MOVESPEED,iMoveSpeed);

					PgObject	*pkObject = dynamic_cast<PgObject*>(pkPilot->GetWorldObject());
					PG_ASSERT_LOG(pkObject);

					const TiXmlNode * pkChildNode = pkNode->FirstChild();
					if (pkChildNode != NULL)
					{
						pkObject->ParseFromWorldXML(pkChildNode);
					}

					if( NULL != g_pkWorld )
					{
						g_pkWorld->AddUnit(pkUnit,pkPilot);
					}

					if(g_iUseAddUnitThread)
					{
						g_kPilotMan.InsertPilot(kGuid,pkPilot);
					}

					pkObject->ReserveTransitAction("a_ridingobject_move");
					//g_pkWorld->AddObject(kGuid,pkPilot->GetWorldObject(),NiPoint3(0,0,0),OGT_OBJECT);
				}
			}
			else if(kTagName == "TRIGGER")
			{
				PgTrigger *pkTrigger = NULL;
				ParseXmlForTrigger( pkNode, pkTrigger );
			}
			else if(kTagName == "PROGRESS")
			{
				ParseXmlForProgress(pkElement);
			}
			else if(kTagName == "PROGRESS_WAY")
			{
				ParseXmlForProgressWay(pkElement);
			}
			else if(kTagName == "STONES")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();
					if(strcmp(pcAttrName, "MINIMAM") == 0)
					{
						// 강정욱. MIN, MAX를 저장해둔다.? but 서버만 필요한 정보일지도 모름.
						(float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "MAXIMAM") == 0)
					{
						(float)atof(pcAttrValue);
					}
					pkAttr = pkAttr->Next();
				}

				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXmlForShineStone(pkChildNode))
					{
						return NULL;
					}
				}
			}
			else if(kTagName == "INPUT")
			{
				TiXmlElement *pkChild = pkElement->FirstChildElement();

				while(pkChild)
				{
					char const* pcTagName = pkChild->Value();

					if(strcmp(pcTagName, "SLOT") == 0)
					{
						TiXmlAttribute *pkAttr = pkChild->FirstAttribute();
						unsigned int uiKey = 0;
						char const* pcScript = 0;

						while(pkAttr)
						{
							if(strcmp(pkAttr->Name(), "NAME") == 0)
							{
							}
							else if(strcmp(pkAttr->Name(), "KEY") == 0)
							{
								uiKey = atoi(pkAttr->Value());
							}
							else if(strcmp(pkAttr->Name(), "SCRIPT") == 0)
							{
								pcScript = pkAttr->Value();
							}
							else
							{
							}

							pkAttr = pkAttr->Next();
						}

						if(uiKey && pcScript)
						{
							m_kInputContainer.insert(std::make_pair(uiKey, std::string(pcScript)));
						}
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
					}

					pkChild = pkChild->NextSiblingElement();
				}
			}
			//else if(strcmp(pcTagName, "MOOD") == 0)
			//{
			//	m_pkMoodMan = (PgMoodMan *)PgXmlLoader::CreateObject(pkNode, m_spSceneRoot);

			//	if(!m_pkMoodMan)
			//	{
			//		PG_ASSERT_LOG(!"failed to parsing Mood");
			//		return false;
			//	}
			//}
			//else if(strcmp(pcTagName, "SEASON") == 0)
			//{
			//	m_pkSeasonMan = (PgSeasonMan *)PgXmlLoader::CreateObject(pkNode);
			//	if(!m_pkSeasonMan)
			//	{
			//		PG_ASSERT_LOG(!"failed to creating SeasonMan");
			//		return false;
			//	}
			//	
			//	if(!m_spSceneRoot)
			//	{
			//		PG_ASSERT_LOG(!"SeasonMan Init : Scene Root Node is null!");
			//		return false;
			//	}
			//	m_pkSeasonMan->SetRootNode(m_spSceneRoot);
			//}	
			else if(kTagName == "TRAP")
			{
				/*
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				char const* pcName = 0;
				char const* pcPath = pkElement->GetText();

				while(pkAttr)
				{
				if(strcmp(pkAttr->Name(), "NAME") == 0)
				{
				pcName = pkAttr->Value();
				}

				pkAttr = pkAttr->Next();
				}

				if(pcName && pcPath)
				{
				PgTrapDef *pkTrapDef = dynamic_cast<PgTrap *>(PgXmlLoader::CreateObjectFromFile(pcPath));

				if(pkTrap)
				{
				}
				}
				*/
			}

			// Parsing Season
			else if(kTagName == "CAMERA_KFMPATH")
			{
				NiString szName = pkElement->GetText();
				m_spCameraAM = PgActorManager::CreatePG(szName);
			}
			else if(kTagName == "WORLD_ACTION_SETTING")
			{
				m_pWorldActionMan = (PgWorldActionMan*)PgXmlLoader::CreateObject(pkNode);
				if(!m_pWorldActionMan)
				{
					PG_ASSERT_LOG(!"failed to creating WorldActionMan");
					return NULL;
				}
			}
			else if(kTagName == "WATER_SETTING")
			{
				if(m_pkWater)
				{
					m_pkWater->SetWaterInfoFromXML(pkNode);
				}
			}
			else if(kTagName == "PUPPET_SETTING")
			{
				TiXmlElement *pkChild = pkElement->FirstChildElement();
				while(pkChild)
				{
					if(strcmp(pkChild->Value(), "PUPPET") == 0)
					{
						char const* pcXmlPath  = pkChild->GetText();
						char const* pcName = 0;

						TiXmlAttribute *pkAttr = pkChild->FirstAttribute();
						while(pkAttr)
						{
							if(strcmp(pkAttr->Name(), "NAME") == 0)
							{
								pcName = pkAttr->Value();
							}

							pkAttr = pkAttr->Next();
						}

						if(pcXmlPath && pcName)
						{
							PgPuppet *pkPuppet = dynamic_cast< PgPuppet* >( PgXmlLoader::CreateObjectFromFile(pcXmlPath) );
							if( pkPuppet )
							{
								m_kPuppetContainer.insert(std::make_pair(pcName, pkPuppet));
							}
							else
							{
								_PgMessageBox(GetClientName(), "Create PgPuppet Failed Path[%s]", pcXmlPath);
							}
						}
					}

					pkChild = pkChild->NextSiblingElement();
				}
			}
			else if(kTagName == "SPOTLIGHT")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				int	iRed=0,iBlue=0,iGreen = 0;
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if (_stricmp(pcAttrName, "BGCOLOR") == 0)
					{
						sscanf_s(pcAttrValue, "%d, %d, %d", &iRed, &iBlue, &iGreen);
					}
					if (_stricmp(pcAttrName, "FORCE_SPOT") == 0)
					{
						if(_stricmp(pcAttrValue, "TRUE") == 0)
						{
							m_bForceSpot = true;
						}
						else
						{
							m_bForceSpot = false;
						}
					}

					pkAttr = pkAttr->Next();
				}

				InitSpotLightMan();
				TurnOnSpotLight(true);
				SetSpotLightBGColor(iRed,iBlue,iGreen);
			}
			else if(kTagName == "NPC")
			{
				BM::GUID kNpcGuid;
				ParseXmlForNpc( pkElement, kNpcGuid );
			}
			else if(kTagName == "COMPLETE_PATH")
			{
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					m_CompleteBGContainer.clear();
					if(!ParseXmlForCompleteBG(pkChildNode))
					{
						return NULL;
					}
				}
			}
			else if(kTagName == "DISABLE_RANDOM_ANI")
			{
				m_kDisableRandomAniObjNameContainer.clear();
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				while(pkChildNode)
				{
					int const iType = pkChildNode->Type();
					switch(iType)
					{
					case TiXmlNode::ELEMENT:
						{
							TiXmlElement *pkElement = (TiXmlElement *)pkChildNode;
							PG_ASSERT_LOG(pkElement);

							char const* pcName = pkElement->Value();

							if(strcmp(pcName, "ITEM") == 0)
							{
								TiXmlAttribute *pkAttr = pkElement->FirstAttribute();

								while(pkAttr)
								{
									if(strcmp(pkAttr->Name(), "OBJECT_NAME") == 0)
									{
										m_kDisableRandomAniObjNameContainer.push_back(pkAttr->Value());
									}
									pkAttr = pkAttr->Next();
								}
							}
						}break;
					}
					pkChildNode = pkChildNode->NextSibling();
				}
			}
			else if ( kTagName == "PVP" )
			{
				if ( m_spSceneRoot )
				{
					m_kContHillLamp.clear();
					TiXmlElement const* pkChildElement = pkNode->FirstChildElement();
					while(pkChildElement)
					{
						if ( !strcmp( pkChildElement->Value(), "HILL") )
						{
							size_t iIndex = 0;
							char const* szText = NULL;
							TiXmlAttribute const* pkAttr = pkChildElement->FirstAttribute();
							while ( pkAttr )
							{
								if ( !strcmp( pkAttr->Name(), "LAMP") )
								{
									szText = pkAttr->Value();
								}
								else if ( !strcmp( pkAttr->Name(), "NO") )
								{
									iIndex = (size_t)::atoi( pkAttr->Value());
								}
								pkAttr = pkAttr->Next();
							}

							NiFixedString strHillName(szText);
							NiAVObject* pkObj = m_spSceneRoot->GetObjectByName(strHillName);
							PG_ASSERT_LOG(pkObj);
							if ( pkObj )
							{
								m_kContHillLamp.insert( std::make_pair(iIndex, pkObj) );
							}
						}	
						pkChildElement = pkChildElement->NextSiblingElement();
					}
				}	
			}
			else if ( kTagName == "BUILDING" )
			{
				if ( m_spSceneRoot )
				{
					short sID = 0;
					std::pair<CONT_BUILDING::iterator, bool> kPair;
					kPair.second = false;

					NiAVObject *pkObject = NULL;
					float fAlpha = 0.3f;

					TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
					while ( pkAttr )
					{
						if ( !strcmp( pkAttr->Name(), "ID") )
						{
							sID = (short)::atoi( pkAttr->Value() );

							if ( !kPair.second )
							{
								kPair = m_kContBuilding.insert( std::make_pair(sID, PgBuilding()) );
							}
						}
						else if ( !strcmp( pkAttr->Name(), "OBJECT") )
						{
							NiFixedString strObjectName(pkAttr->Value());
							pkObject = m_spSceneRoot->GetObjectByName(strObjectName);	
						}
						else if ( !::strcmp( pkAttr->Name(), "ALPHA") )
						{
							fAlpha = static_cast<float>(::atof( pkAttr->Value() ));
						}
						pkAttr = pkAttr->Next();
					}

					if ( true == kPair.second )
					{
						PgBuilding &kElement = kPair.first->second;
						PgBoardBalloon *pkBoardBallon = NULL;

						TiXmlElement const* pkChildElement = pkNode->FirstChildElement();
						while(pkChildElement)
						{
							if ( !::strcmp( pkChildElement->Value(), "SIGNBOARD") )
							{
								std::string strTextureName;
								float fScale = 1.0f;
								NiPoint3 pt3AdjustPos;

								TiXmlAttribute const* pkAttr = pkChildElement->FirstAttribute();
								while ( pkAttr )
								{
									if ( !::strcmp( pkAttr->Name(), "TEXTURE") )
									{
										strTextureName = pkAttr->Value();
									}
									else if ( !::strcmp( pkAttr->Name(), "ADJUST") )
									{
										::sscanf_s( pkAttr->Value(), "%f, %f, %f"
											,	&pt3AdjustPos.x
											,	&pt3AdjustPos.y 
											,	&pt3AdjustPos.z);
									}
									else if ( !::strcmp( pkAttr->Name(), "SCALE") )
									{
										fScale = ::atof( pkAttr->Value() );
										if ( 0.01f > fScale )
										{
											fScale = 0.01;
										}
									}
									pkAttr = pkAttr->Next();
								}

								if ( true == g_kBalloonMan3D.CreateNode( pkBoardBallon ) )
								{
									if ( true == pkBoardBallon->Init( strTextureName, pt3AdjustPos, fScale ) )
									{
										TiXmlElement const* pkChildElement2 = pkChildElement->FirstChildElement();
										while ( pkChildElement2 )
										{
											if ( !::strcmp( pkChildElement2->Value(), "TEXT") )
											{
												int iSlot = 0;
												std::wstring wstrText;
												float fScale = 1.0f;
												NiColorA kTextColor( 1.0f, 1.0f, 1.0f, 1.0f );
												NiColorA kOutlineColor( 0.0f, 0.0f, 0.0f, 0.0f );
												bool bOutLine = false;
												NiPoint3 pt3AdjustPos( 0.0f, 0.0f, 0.0f );
												XUI::CXUI_Font *pkFont = NULL;
												int iTextLimitWidth = -1;

												pkAttr = pkChildElement2->FirstAttribute();
												while ( pkAttr )
												{
													if ( !::strcmp( pkAttr->Name(), "SLOT") )
													{
														iSlot = ::atoi( pkAttr->Value() );
													}
													else if ( !::strcmp( pkAttr->Name(), "ID") )
													{
														int const iTextID = ::atoi( pkAttr->Value() );
														wstrText = TTW(iTextID);
													}
													else if( !::strcmp( pkAttr->Name(), "FONT") )
													{
														pkFont = g_kFontMgr.GetFont( UNI(pkAttr->Value()) );
													}
													else if ( !::strcmp( pkAttr->Name(), "SCALE") )
													{
														fScale = ::atof( pkAttr->Value() );
													}
													else if ( !::strcmp( pkAttr->Name(), "COLOR") )
													{
														::sscanf_s( pkAttr->Value(), "%f, %f, %f, %f", &kTextColor.r, &kTextColor.g, &kTextColor.b, &kTextColor.a );
													}
													else if ( !::strcmp( pkAttr->Name(), "COLOR_LINE") )
													{
														bOutLine = true;
														::sscanf_s( pkAttr->Value(), "%f, %f, %f, %f", &kOutlineColor.r, &kOutlineColor.g, &kOutlineColor.b, &kOutlineColor.a );
													}
													else if ( !::strcmp( pkAttr->Name(), "ADJUST") )
													{
														::sscanf_s( pkAttr->Value()	, "%f, %f, %f"
																					,	&pt3AdjustPos.x
																					,	&pt3AdjustPos.y 
																					,	&pt3AdjustPos.z);
													}
													else if ( !::strcmp( pkAttr->Name(), "LIMIT_WIDTH") )
													{
														iTextLimitWidth = ::atoi( pkAttr->Value() );
													}

													pkAttr = pkAttr->Next();
												}

												if ( pkFont )
												{
													CXUI_Style_String kSrcStyleString( XUI::PgFontDef( pkFont, 0xFFFFFFFF ), wstrText );
													Pg2DString *pk2DString = new Pg2DString( kSrcStyleString, true, false, iTextLimitWidth, true );
													if ( FAILED(pkBoardBallon->AddText( iSlot, pk2DString, pt3AdjustPos, kTextColor, kOutlineColor, bOutLine, fScale )) )
													{
														SAFE_DELETE( pk2DString );
													}
												}
											}
											pkChildElement2 = pkChildElement2->NextSiblingElement();
										}
									}
								}
							}
							else if ( !::strcmp( pkChildElement->Value(), "TOGGLE") )
							{
								NiFixedString strObjectName;
								bool bToggleOnShow = false;
								TiXmlAttribute const* pkAttr = pkChildElement->FirstAttribute();
								while ( pkAttr )
								{
									if ( !::strcmp( pkAttr->Name(), "OBJECT") )
									{
										strObjectName = pkAttr->Value();
									}
									else if ( !::strcmp( pkAttr->Name(), "ON_SHOW") )
									{
										bToggleOnShow = (strcmp(pkAttr->Value(), "TRUE") == 0);
									}
									pkAttr = pkAttr->Next();
								}

								NiAVObject *pkObject = m_spSceneRoot->GetObjectByName(strObjectName);
								if ( pkObject )
								{
									kElement.AddToggleObject( pkObject, bToggleOnShow );
								}
								else
								{
									_PgMessageBox( "[BUILDING OBJECT]", "ToggleObject NotFound : %s", strObjectName );
								}
							}
							else if ( !::strcmp( pkChildElement->Value(), "TRIGGER") )
							{
								PgTrigger *pkTrigger = NULL;
								if ( ParseXmlForTrigger( pkChildElement, pkTrigger ) )
								{
									kElement.SetTrigger( pkTrigger );
								}
							}
							else if( !::strcmp( pkChildElement->Value(), "PROGRESS") )
							{ //월드 맵에서의 구간 위치 파악을 위한 트리거 (던전 미니맵 용)
							}

							else if( !::strcmp( pkChildElement->Value(), "NPC") )
							{
								BM::GUID kNpcGuid;
								if ( ParseXmlForNpc( pkChildElement, kNpcGuid ) )
								{
									kElement.NpcID( kNpcGuid );
								}
							}
							pkChildElement = pkChildElement->NextSiblingElement();
						}
						kElement.Init( pkObject, fAlpha, pkBoardBallon );
					}
					else
					{
						_PgMessageBox( "[BUILDING OBJECT]", "Overlapping ID[%d] : %d", sID );
					}
				}
			}
			else if ( kTagName == "WORLDEVENT_OBJECT_LIST" )
			{
				g_kWEClientObjectMgr.ParseWEClientObjectList(pkElement);
			}
			else if ( kTagName == "WORLD_EVENT" )
			{
				g_kWorldEventClientMgr.ParseWorldEvent(pkElement);
			}
			else if (kTagName == "DUNGEON_EXIT_UI")
			{
				m_bCallDungeonExistUI = true;
			}
			else
			{
				NILOG(PGLOG_WARNING, "[PgWorld] unknown %s node in %s world\n", kTagName.c_str(), GetID().c_str());
			}
		}

	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		return pkNextNode;
	}

	// xml 파싱이 성공적으로 끝났다면,
	if(strcmp(pkNode->Value(), "WORLD") == 0)
	{
		// All_Npc.xml에서 파싱한 해당 월드의 Npc 추가
		PgWorldNpcMgr::CONT_WORLD_NPC::const_iterator c_WorldNpcIter = g_kWorldNpcMgr.GetWorldNpc().find( GetID() );
		if( c_WorldNpcIter != g_kWorldNpcMgr.GetWorldNpc().end() )
		{
			PgWorldNpcMgr::CONT_WORLD_NPC::mapped_type kContNpc = c_WorldNpcIter->second;
			PgWorldNpcMgr::CONT_NPC::const_iterator c_NpcIter = kContNpc.begin();
			while( kContNpc.end() != c_NpcIter )
			{
				PgWorldNpcMgr::CONT_NPC::mapped_type const& kElement = c_NpcIter->second;

				PgPilot *pkPilot = addNpcToWorld(	kElement.uiNpcNo, kElement.uiNpcKID, kElement.kNpcGuid, kElement.kActorName.c_str(),
					kElement.kScriptName.c_str(), kElement.kPos, kElement.kDir, kElement.iType, kElement.bHidden, kElement.GiveEffectNo, kElement.bHideMiniMap	);
				if( pkPilot )
				{
					if( kElement.iParam )
					{
						pkPilot->SetAbil( AT_CUSTOMDATA1, kElement.iParam );
					}

					if( kElement.iMinimapIcon )
					{
						pkPilot->SetAbil( AT_DISPLAY_MINIMAP_EFFECT, kElement.iMinimapIcon );
					}

					PgActorNpc *pkActorNpc = dynamic_cast<PgActorNpc*>( pkPilot->GetWorldObject() );
					if( pkActorNpc )
					{
						pkActorNpc->AddSayAction( kElement.kVec );
						pkActorNpc->SetIgnoreCameraCulling( kElement.bIgnoreCameraCulling );
					}
				}

				++c_NpcIter;
			}
		}

		if (m_bUseWorldObjectLOD)
		{
			SetWorldLODData(m_afWorldObjectLODRange[0], m_afWorldObjectLODRange[1], m_afWorldObjectLODRange[2]);
			NiLODNode::SetGlobalLOD(-1);
		}
		else
			NiLODNode::SetGlobalLOD(0);

		//	그림자 붙이기
		//	PgCircleShadow::AttachCircleShadowRecursive(m_spSceneRoot, 500.0f, 0.7f, NULL, NULL);

		//bool bSelectiveUpdate;
		//bool bRigid;
		//m_spSceneRoot->SetSelectiveUpdateFlags(bSelectiveUpdate, false, bRigid);
		m_spSceneRoot->UpdateNodeBound();
		m_spSceneRoot->UpdateProperties();
		m_spSceneRoot->UpdateEffects();

		optimizeWorldNode();

		InitSpotLightMan();
		//TurnOnSpotLight(true);
		//SetSpotLightBGColor(0,0,0);

		/*	m_pOctree = NiNew PgOctree();
		m_pOctree->GenerateTreeFromNode(m_spSceneRoot);
		m_pOctree->UpdateNodeBound();
		m_pOctree->UpdateProperties();
		m_pOctree->UpdateEffects();
		m_pOctree->Update(0.0f);	////
		*/

		// 바로 그릴 수 있도록 준비한다.
        NewWare::Scene::ApplyTraversal::Property::DisableFogAtBillboardAndParticle( m_spSceneRoot );
        NewWare::Scene::AssetUtils::NIF::RecursivePrepack( m_spSceneRoot, true );
		//thread ((NiDX9Renderer*)NiRenderer::GetRenderer())->PerformPrecache();

		NiTimeController::StartAnimations(m_spSceneRoot, 0.0f);

		//PgPathData	kData;
		//kData.CreatePathDataFromPhysX(m_spPhysXScene->GetPhysXScene());

		// 트리거 정보를 설정한다.
		LoadTriggers();
		LoadShineStones();

		//! 대미지 넘버 보여주는 클래스
		if(!m_pkDamageNumMan)
		{
			m_pkDamageNumMan = NiNew PgDamageNumMan();
		}

		if(GetPSRoomGroupRoot())
		{
			GetPSRoomGroupRoot()->InitializeRooms(m_kMapObjectCont);
		}

		m_spSceneRoot->UpdateNodeBound();
		m_spSceneRoot->UpdateProperties();
		m_spSceneRoot->UpdateEffects();
		m_spSceneRoot->Update(0.0f);

		ApplyFogOption();
		ApplyTextureResOption();
		ApplyTextureFilteringOption();

        using NewWare::Renderer::DrawWorkflow;
        m_ePrevDrawMode = GetInstanceProxy(DrawWorkflow)->GetDrawMode();
        GetInstanceProxy(DrawWorkflow)->Create( NewWare::Renderer::DrawBound::DRAWMODE_NOTHREAD ); // 주의 DRAWMODE_THREAD는 쓰지 마세요!!!

		//NiOcclusionCuller	*pkCuller = (NiOcclusionCuller*)PgRenderer::GetPgRenderer()->GetCuller();
		//if(pkCuller)
		//{
		//	pkCuller->RemoveAllOcclusionGeometry();

		//	pkCuller->FindOcclusionGeometry(m_pkStaticNodeRoot,false);
		//	pkCuller->FindOcclusionGeometry(m_pkSelectiveNodeRoot,false);
		//}

		//m_kSphereTree.AddNode(m_pkStaticNodeRoot);
		//m_kSphereTree.AddNode(m_pkSelectiveNodeRoot);

		if(lua_tinker::call<bool>("UseSphereTree"))
		{
			PgSphereTree kSphereTree;
			kSphereTree.GenerateTree(GetStaticNodeRoot());
			kSphereTree.GenerateTree(GetSelectiveNodeRoot());
		}

		if(g_pkApp->IsSingleMode())
		{
			//	Call the method only when it is single mode.
			//	When it is not single mode, this method is called by net_basic.lua.
			RecursiveDetachGlowMapByGndAttr(m_spSceneRoot);
		}

		if(!m_kScript.empty())
		{
			PgScripting::DoFile(m_kScript.c_str());
		}

		if(!m_kPostScript.empty())
		{
			lua_tinker::call<bool, lwWorld>(m_kPostScript.c_str(), this);
		}

		//NiTimeController::StopAnimations(m_spSceneRoot);
		// 전체 씬을 초기화한다.
        NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( m_spSceneRoot, false );

		// BgSound
		size_t iBgSoundSize = m_kBgSoundContainer.size();
		if( 0 < iBgSoundSize )
		{
			int iIndex = BM::Rand_Range((iBgSoundSize - 1), 0);
			int iCount = 0;

			BgSoundContainer::iterator iter = m_kBgSoundContainer.begin();
			while( m_kBgSoundContainer.end() != iter )
			{
				if( iIndex == iCount )
				{
					g_kSoundMan.LoadBgSound( iter->first.c_str(), iter->second );
					break;
				}
				++iCount;
				++iter;
			}			
		}
	}

	return NULL;
}
void	PgWorld::ApplyFogOption()
{
	if(GET_OPTION_VIEW_DISTANCE == false)
	{
		if(m_pkFogProperty)
		{
			m_pkFogProperty->SetFogDensity(lua_tinker::call<float>("GetLowFogDensity"));
			m_pkFogProperty->SetFogStart(lua_tinker::call<float>("GetLowFogStart"));
			m_pkFogProperty->SetFogEnd(lua_tinker::call<float>("GetLowFogEnd"));
		}
	}
	else
	{
		if(m_pkFogProperty)
		{
			m_pkFogProperty->SetFogDensity(m_fOriginalFogDensity);
			m_pkFogProperty->SetFogStart(m_fOriginalFogStart);
			m_pkFogProperty->SetFogEnd(m_fOriginalFogEnd);
		}
	}

	if(m_spSceneRoot)
	{
		if(GetPSRoomGroupRoot())
		{
			GetPSRoomGroupRoot()->SetShaderConstantUpdateOptimizeFlag(true);
		}
        NewWare::Scene::ApplyTraversal::Geometry::SetShaderConstantUpdateOptimizeFlag( GetStaticNodeRoot(), true, true );
        NewWare::Scene::ApplyTraversal::Geometry::SetShaderConstantUpdateOptimizeFlag( m_spSkyRoot, true, true );
	}

    NewWare::Renderer::RefreshRenderStateBlockCaches();
}
void	PgWorld::ApplyTextureResOption()
{
	NiNode	*pkStaticRoot = GetStaticNodeRoot();
	NiNode	*pkSelectiveRoot = GetSelectiveNodeRoot();
	NiNode	*pkRoomGroupRoot = GetPSRoomGroupRoot();

	//	Apply Resolution
	if(g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_TEXTURE_RES) == 1)
	{
		//	High Resolution
		PgRenderer::ReloadTexture(pkRoomGroupRoot,0);
		PgRenderer::ReloadTexture(pkStaticRoot,0);
		PgRenderer::ReloadTexture(pkSelectiveRoot,0);
	}
	else
	{
		//	Low Resolution
		PgRenderer::ReloadTexture(pkRoomGroupRoot,1);
		PgRenderer::ReloadTexture(pkStaticRoot,1);
		PgRenderer::ReloadTexture(pkSelectiveRoot,1);
	}

	NewWare::Renderer::RefreshRenderStateBlockCaches();
}
void	PgWorld::ApplyTextureFilteringOption()
{
	NiNode	*pkStaticRoot = GetStaticNodeRoot();
	NiNode	*pkSelectiveRoot = GetSelectiveNodeRoot();
	NiNode	*pkRoomGroupRoot = GetPSRoomGroupRoot();

	//	Apply Filtering
	if(g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_TEXTURE_FILTERING) == 1)
	{
		//	High Filtering
        NewWare::Renderer::DrawUtils::SetBaseTextureFiltering( pkStaticRoot,    NiTexturingProperty::FILTER_TRILERP );
        NewWare::Renderer::DrawUtils::SetBaseTextureFiltering( pkSelectiveRoot, NiTexturingProperty::FILTER_TRILERP );
        NewWare::Renderer::DrawUtils::SetBaseTextureFiltering( pkRoomGroupRoot, NiTexturingProperty::FILTER_TRILERP );
	}
	else
	{
		//	Low Filtering
        NewWare::Renderer::DrawUtils::SetBaseTextureFiltering( pkStaticRoot,    NiTexturingProperty::FILTER_NEAREST );
        NewWare::Renderer::DrawUtils::SetBaseTextureFiltering( pkSelectiveRoot, NiTexturingProperty::FILTER_NEAREST );
        NewWare::Renderer::DrawUtils::SetBaseTextureFiltering( pkRoomGroupRoot, NiTexturingProperty::FILTER_NEAREST );
	}

	NewWare::Renderer::RefreshRenderStateBlockCaches();
}
void	PgWorld::ApplyWorldQualityOption()
{
	NiNode	*pkStaticRoot = GetStaticNodeRoot();
	NiNode	*pkSelectiveRoot = GetSelectiveNodeRoot();
	NiNode	*pkRoomGroupRoot = GetPSRoomGroupRoot();

	// Apply World Quality
	if(1 == g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_WORLD_QUALITY))
	{//High Quality
		PgRenderer::SetWorldQuality(pkStaticRoot, true);
		PgRenderer::SetWorldQuality(pkSelectiveRoot, true);
		PgRenderer::SetWorldQuality(pkRoomGroupRoot, true);
	}
	else
	{//Low Quality
		PgRenderer::SetWorldQuality(pkStaticRoot, false);
		PgRenderer::SetWorldQuality(pkSelectiveRoot, false);
		PgRenderer::SetWorldQuality(pkRoomGroupRoot, false);
	}

	NewWare::Renderer::RefreshRenderStateBlockCaches();
}


NiPoint3	PgWorld::FindActorFloorPos(NiPoint3 const& kStart,const NxU32 kGroup)	//	액터 놓을 위치를 계산한다.(떨어지지 않게)
{
	NiPoint3	kFloorPos = kStart;

	PG_ASSERT_LOG(m_spPhysXScene);
	PG_ASSERT_LOG(m_spPhysXScene->GetPhysXScene());

	if(!m_spPhysXScene)	return kFloorPos;

	NxScene *pkScene = m_spPhysXScene->GetPhysXScene();
	if(!pkScene) return kFloorPos;

	for(int k=0;k<2;++k)
	{
		for(int i=10;i<150;i+=10)
		{
			NxRay kRay(NxVec3(kStart.x, kStart.y, kStart.z+i), NxVec3(0, 0, -1.0f));
			NxRaycastHit kHit;

			if(raycastClosestShape(kRay, 
				NX_STATIC_SHAPES, 
				kHit, 
				kGroup, 
				150.0f+k*100.0f, 
				NX_RAYCAST_SHAPE | NX_RAYCAST_IMPACT))
			{
				NiPhysXTypes::NxVec3ToNiPoint3(kHit.worldImpact, kFloorPos);
				kFloorPos.z = kHit.worldImpact.z + PG_CHARACTER_Z_ADJUST;
				return	kFloorPos;
			}
		}
	}


	return	kFloorPos;
}

void PgWorld::MapMoveCompleteFade()
{
	if( IsMapMoveCompleteFade() )
	{
		if(m_kMapMoveCompleteFade.bFadeIn)
		{
			SetShowWorldFocusFilter(m_kMapMoveCompleteFade.dwFadeColor, 1.f, 0.f, m_kMapMoveCompleteFade.fFadeTime, false, false);
		}
		else
		{
			SetShowWorldFocusFilter(m_kMapMoveCompleteFade.dwFadeColor, 0.f, 1.f, m_kMapMoveCompleteFade.fFadeTime, false, false);
		}
	}
}

void PgWorld::SaveHideActor(int const iClassNo, bool const bHide)
{
	if(bHide)
	{
		m_kContSaveHideActor.insert(iClassNo);
	}
	else
	{
		m_kContSaveHideActor.erase(iClassNo);
	}
}

void PgWorld::SaveHideActor(EUnitType const eType, bool const bHide)
{
	if(bHide)
	{
		m_kContSaveHideByUnitType.insert(eType);
	}
	else
	{
		m_kContSaveHideByUnitType.erase(eType);
	}
}

bool PgWorld::IsSaveHideActor(PgActor * pkActor)const
{
	if(pkActor && pkActor->GetPilot())
	{
		return IsSaveHideActor( pkActor->GetPilot()->GetAbil(AT_CLASS) ) || IsSaveHideActor(pkActor->GetUnit()->UnitType());
	}
	return false;
}

bool PgWorld::IsSaveHideActor(int const iClassNo)const
{
	return m_kContSaveHideActor.end() != m_kContSaveHideActor.find(iClassNo);
}

bool PgWorld::IsSaveHideActor(EUnitType const eType)const
{
	return m_kContSaveHideByUnitType.end() != m_kContSaveHideByUnitType.find(eType);
}

void PgWorld::ClearSaveHideActor()
{
	m_kContSaveHideActor.clear();
}

void PgWorld::ResetFocusFilter()
{
	m_spFocusFilter = NULL;
	m_kFocusFilterInfo.pTexture1 = NULL;
	m_kFocusFilterInfo.pTexture2 = NULL;
	m_kFocusFilterInfo.fAlphaStart = 0.0f;
	m_kFocusFilterInfo.fAlphaEnd = 0.0f;
	m_kFocusFilterInfo.fStartTime = 0.0f;
	m_kFocusFilterInfo.fTotalTime = 0.0f;
	m_kFocusFilterInfo.fInterval = 0.0f;
	m_kFocusFilterInfo.bAutoTurnOff = true;
	m_kFocusFilterInfo.bRenderObject = true;
	m_kFocusFilterInfo.bAlphaEndIsMaxValueRenderObject = true;

	SetDrawWorldText(true);
	SetDrawBG(true);		
}

void PgWorld::SetShowWorldFocusFilter(bool bShow, char* pTextureFile, float fAlpha)
{
	ResetFocusFilter();
	if (bShow)
	{
		NiSourceTexture* tex = g_kNifMan.GetTexture(pTextureFile);
		if (tex == NULL)
			return;

		m_spFocusFilter = NiNew NiScreenTexture(tex);
		if (m_spFocusFilter != NULL)
		{
			PgRenderer* pkPgRenderer = g_kRenderMan.GetRenderer();
			NiRenderer *pkRenderer = NiRenderer::GetRenderer();
			PG_ASSERT_LOG(pkRenderer);

			unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
			unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

			bool bFSAA = GET_OPTION_ANTI_ALIAS;
			bool bGlow = GET_OPTION_GLOW_EFFECT;
			if (bFSAA || bGlow)
			{
				uiWidth = uiWidth * 2;
				uiHeight = uiHeight * 2;
			}

			m_spFocusFilter->SetClampMode(NiTexturingProperty::WRAP_S_WRAP_T);
			m_spFocusFilter->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
			m_spFocusFilter->AddNewScreenRect(0, 0, uiWidth, uiHeight, 0, 0, NiColorA(1, 1, 1, fAlpha));
			m_spFocusFilter->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
		}
		else
		{

		}
	}
}

void PgWorld::SetShowWorldFocusFilter(char* pTextureFile, float fAlphaStart, float fAlphaEnd, float fTime, bool bAutoTurnOff, bool const bRenderObject, bool const bAlphaEndIsMaxValueRenderObject)
{
	ResetFocusFilter();
	m_kFocusFilterInfo.fAlphaStart = fAlphaStart;
	m_kFocusFilterInfo.fAlphaEnd = fAlphaEnd;
	m_kFocusFilterInfo.fTotalTime = fTime;
	m_kFocusFilterInfo.fStartTime = NiGetCurrentTimeInSec();
	m_kFocusFilterInfo.bAutoTurnOff = bAutoTurnOff;
	m_kFocusFilterInfo.bRenderObject = bRenderObject;
	m_kFocusFilterInfo.bAlphaEndIsMaxValueRenderObject = bAlphaEndIsMaxValueRenderObject;

	NiSourceTexture* tex = g_kNifMan.GetTexture(pTextureFile);
	if (tex == NULL)
		return;

	m_spFocusFilter = NiNew NiScreenTexture(tex);
	if (m_spFocusFilter != NULL)
	{
		PgRenderer* pkPgRenderer = g_kRenderMan.GetRenderer();
		NiRenderer *pkRenderer = NiRenderer::GetRenderer();
		PG_ASSERT_LOG(pkRenderer);

		unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
		unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

		bool bFSAA = GET_OPTION_ANTI_ALIAS;
		bool bGlow = GET_OPTION_GLOW_EFFECT;
		if (bFSAA || bGlow)
		{
			uiWidth = uiWidth * 2;
			uiHeight = uiHeight * 2;
		}

		m_spFocusFilter->SetClampMode(NiTexturingProperty::WRAP_S_WRAP_T);
		m_spFocusFilter->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		m_spFocusFilter->AddNewScreenRect(0, 0, uiWidth, uiHeight, 0, 0, NiColorA(1, 1, 1, fAlphaStart));
		m_spFocusFilter->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	}
	else
	{

	}
}

void PgWorld::SetShowWorldFocusFilter(DWORD color, float fAlphaStart, float fAlphaEnd, float fTime, bool bAutoTurnOff, bool const bRenderObject, bool const bAlphaEndIsMaxValueRenderObject )
{
	ResetFocusFilter();
	m_kFocusFilterInfo.fAlphaStart = fAlphaStart;
	m_kFocusFilterInfo.fAlphaEnd = fAlphaEnd;
	m_kFocusFilterInfo.fTotalTime = fTime;
	m_kFocusFilterInfo.fStartTime = NiGetCurrentTimeInSec();
	m_kFocusFilterInfo.bAutoTurnOff = bAutoTurnOff;
	m_kFocusFilterInfo.bRenderObject = bRenderObject;
	m_kFocusFilterInfo.bAlphaEndIsMaxValueRenderObject = bAlphaEndIsMaxValueRenderObject;

	NiSourceTexture* tex = g_kNifMan.GetTexture("../Data/6_UI/whitebg.tga");
	//if (tex == NULL)
	//	return;

	m_spFocusFilter = NiNew NiScreenTexture(tex);
	if (m_spFocusFilter != NULL)
	{
		PgRenderer* pkPgRenderer = g_kRenderMan.GetRenderer();
		NiRenderer *pkRenderer = NiRenderer::GetRenderer();
		PG_ASSERT_LOG(pkRenderer);

		unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
		unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

		bool bFSAA = GET_OPTION_ANTI_ALIAS;
		bool bGlow = GET_OPTION_GLOW_EFFECT;
		if (bFSAA || bGlow)
		{
			uiWidth = uiWidth * 2;
			uiHeight = uiHeight * 2;
		}

		DWORD dw0x = 0x000000ff;		
		m_spFocusFilter->SetClampMode(NiTexturingProperty::WRAP_S_WRAP_T);
		m_spFocusFilter->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		m_spFocusFilter->AddNewScreenRect(0, 0, uiWidth, uiHeight, 0, 0, NiColorA((color>>16 & dw0x)/255.0f, (color>>8 & dw0x)/255.0f, (color & dw0x)/255.0f, fAlphaStart));
		m_spFocusFilter->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	}
	else
	{

	}
}

void PgWorld::SetShowWorldFocusFilter(char* pTextureFile1, char* pTextureFile2, float fAlpha1, float fAlpha2, float fTotalTime, float fInterval)
{
	ResetFocusFilter();
	m_kFocusFilterInfo.fTotalTime = fTotalTime;
	m_kFocusFilterInfo.fInterval = fInterval;
	m_kFocusFilterInfo.fStartTime = NiGetCurrentTimeInSec();

	NiSourceTexture* tex = g_kNifMan.GetTexture(pTextureFile1);
	if (tex == NULL)
		return;

	PgRenderer* pkPgRenderer = g_kRenderMan.GetRenderer();
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	PG_ASSERT_LOG(pkRenderer);

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	bool bFSAA = GET_OPTION_ANTI_ALIAS;
	bool bGlow = GET_OPTION_GLOW_EFFECT;
	if (bFSAA || bGlow)
	{
		uiWidth = uiWidth * 2;
		uiHeight = uiHeight * 2;
	}

	m_kFocusFilterInfo.pTexture1 = NiNew NiScreenTexture(tex);
	if (m_kFocusFilterInfo.pTexture1 != NULL)
	{		
		m_kFocusFilterInfo.pTexture1->SetClampMode(NiTexturingProperty::WRAP_S_WRAP_T);
		m_kFocusFilterInfo.pTexture1->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		m_kFocusFilterInfo.pTexture1->AddNewScreenRect(0, 0, uiWidth, uiHeight, 0, 0, NiColorA(1, 1, 1, fAlpha1));
		m_kFocusFilterInfo.pTexture1->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	}
	else
	{

	}

	tex = g_kNifMan.GetTexture(pTextureFile2);
	if (tex == NULL)
		return;

	m_kFocusFilterInfo.pTexture2 = NiNew NiScreenTexture(tex);
	if (m_kFocusFilterInfo.pTexture2 != NULL)
	{		
		m_kFocusFilterInfo.pTexture2->SetClampMode(NiTexturingProperty::WRAP_S_WRAP_T);
		m_kFocusFilterInfo.pTexture2->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		m_kFocusFilterInfo.pTexture2->AddNewScreenRect(0, 0, uiWidth, uiHeight, 0, 0, NiColorA(1, 1, 1, fAlpha2));
		m_kFocusFilterInfo.pTexture2->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	}
	else
	{

	}

	m_spFocusFilter = m_kFocusFilterInfo.pTexture1;
}

float	PgWorld::GetWorldFocusFilterAlpha()
{
	if(!m_spFocusFilter) return	0;

	if(m_spFocusFilter->GetNumScreenRects() == 0) return 0;

	return	m_spFocusFilter->GetScreenRect(0).m_kColor.a;
}

bool PgWorld::IsEndShowWorldFocusFilter()
{
	if (m_spFocusFilter)
	{
		if (m_kFocusFilterInfo.fStartTime != 0.0f && m_kFocusFilterInfo.fTotalTime != 0.0f)
		{
			float fCurrentTime = NiGetCurrentTimeInSec();

			if ( fCurrentTime < (m_kFocusFilterInfo.fStartTime + m_kFocusFilterInfo.fTotalTime) )
			{
				return false;
			}
		}
	}
	return true;
}
void PgWorld::SetShowWorldFocusFilter(DWORD color1, DWORD color2, float fAlpha1, float fAlpha2, float fTotalTime, float fInterval)
{
	ResetFocusFilter();
	m_kFocusFilterInfo.fTotalTime = fTotalTime;
	m_kFocusFilterInfo.fInterval = fInterval;
	m_kFocusFilterInfo.fStartTime = NiGetCurrentTimeInSec();

	NiSourceTexture* tex = g_kNifMan.GetTexture("../Data/6_UI/whitebg.tga");
	if (tex == NULL)
		return;

	PgRenderer* pkPgRenderer = g_kRenderMan.GetRenderer();
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	PG_ASSERT_LOG(pkRenderer);

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	bool bFSAA = GET_OPTION_ANTI_ALIAS;
	bool bGlow = GET_OPTION_GLOW_EFFECT;
	if (bFSAA || bGlow)
	{
		uiWidth = uiWidth * 2;
		uiHeight = uiHeight * 2;
	}

	m_kFocusFilterInfo.pTexture1 = NiNew NiScreenTexture(tex);
	if (m_kFocusFilterInfo.pTexture1 != NULL)
	{		
		DWORD dw0x = 0x000000ff;		
		m_kFocusFilterInfo.pTexture1->SetClampMode(NiTexturingProperty::WRAP_S_WRAP_T);
		m_kFocusFilterInfo.pTexture1->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		m_kFocusFilterInfo.pTexture1->AddNewScreenRect(0, 0, uiWidth, uiHeight, 0, 0, NiColorA((color1>>16 & dw0x)/255.0f, (color1>>8 & dw0x)/255.0f, (color1 & dw0x)/255.0f, fAlpha1));
		m_kFocusFilterInfo.pTexture1->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	}
	else
	{

	}

	m_kFocusFilterInfo.pTexture2 = NiNew NiScreenTexture(tex);
	if (m_kFocusFilterInfo.pTexture2 != NULL)
	{		
		DWORD dw0x = 0x000000ff;		
		m_kFocusFilterInfo.pTexture2->SetClampMode(NiTexturingProperty::WRAP_S_WRAP_T);
		m_kFocusFilterInfo.pTexture2->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		m_kFocusFilterInfo.pTexture2->AddNewScreenRect(0, 0, uiWidth, uiHeight, 0, 0, NiColorA((color2>>16 & dw0x)/255.0f, (color2>>8 & dw0x)/255.0f, (color2 & dw0x)/255.0f, fAlpha2));
		m_kFocusFilterInfo.pTexture2->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	}
	else
	{

	}

	m_spFocusFilter = m_kFocusFilterInfo.pTexture1;
}
void	PgWorld::UpdateFocusFilter(float fAccumTime,float fFrameTime)
{
	if(!GetFocusFilterActivated())
	{
		return;
	}

	float	const	fElapsedTime = CalcFocusFilterElapsedTime();

	ResetFocusFilterIfTimeIsOver(fElapsedTime);
	UpdateFocusFilterTextureSwap(fElapsedTime);
	UpdateFocusFilterAlphaTransition(fElapsedTime);
}
bool	PgWorld::GetFocusFilterActivated()	const
{
	if(!m_spFocusFilter)
	{
		return false;
	}
	if (m_kFocusFilterInfo.fStartTime == 0.0f || m_kFocusFilterInfo.fTotalTime == 0.0f)
	{
		return false;
	}

	return	true;
}
float	PgWorld::CalcFocusFilterElapsedTime()	const
{
	float fCurrentTime = NiGetCurrentTimeInSec();
	float	fElapsedTime = fCurrentTime - m_kFocusFilterInfo.fStartTime;

	return	fElapsedTime;
}
void	PgWorld::ResetFocusFilterIfTimeIsOver(float fElapsedTime)
{
	if (fElapsedTime > m_kFocusFilterInfo.fTotalTime)
	{
		if (m_kFocusFilterInfo.bAutoTurnOff)
		{
		 // 지우자.
			ResetFocusFilter();
		}
	}
}
void	PgWorld::UpdateFocusFilterTextureSwap(float fElapsedTime)
{
	if(m_kFocusFilterInfo.fInterval == 0.0f)
	{
		return;
	}

	int iCurFilter = ((int)(fElapsedTime / m_kFocusFilterInfo.fInterval)) % 2;
	if (iCurFilter == 0)
	{
		m_spFocusFilter = m_kFocusFilterInfo.pTexture1;
	}
	else
	{
		m_spFocusFilter = m_kFocusFilterInfo.pTexture2;
	}
}
void	PgWorld::UpdateFocusFilterAlphaTransition(float fElapsedTime)
{
	if(m_kFocusFilterInfo.fInterval != 0.0f)
	{
		return;
	}

	float	const	fAlpha = CalcFocusFilterAlpha(fElapsedTime);

	if(false == m_kFocusFilterInfo.bAlphaEndIsMaxValueRenderObject && 1.0f == fAlpha) //최대로 가릴 경우
	{
		SetDrawWorldText(false);
		SetDrawBG(false);
	}
	else if(false == m_kFocusFilterInfo.bAlphaEndIsMaxValueRenderObject && 1.0f != fAlpha) //최대치가 아닐 경우
	{
		SetDrawWorldText(true);
		SetDrawBG(true);
	}
	else
	{
		SetDrawWorldText(true);
		SetDrawBG(true);
	}

	SetFocusFilterAlpha(fAlpha);

}
float	PgWorld::CalcFocusFilterAlpha(float fElapsedTime)	const
{
	float fAlphaInc = (m_kFocusFilterInfo.fAlphaEnd - m_kFocusFilterInfo.fAlphaStart) / m_kFocusFilterInfo.fTotalTime;
	float fAlpha = m_kFocusFilterInfo.fAlphaStart + fAlphaInc * fElapsedTime;

	float const fMax = (m_kFocusFilterInfo.fAlphaEnd > m_kFocusFilterInfo.fAlphaStart) ? (m_kFocusFilterInfo.fAlphaEnd) : (m_kFocusFilterInfo.fAlphaStart);
	float const fMin = (m_kFocusFilterInfo.fAlphaEnd < m_kFocusFilterInfo.fAlphaStart) ? (m_kFocusFilterInfo.fAlphaEnd) : (m_kFocusFilterInfo.fAlphaStart);

	fAlpha = NiClamp(fAlpha, fMin, fMax);

	return	fAlpha;
}
void	PgWorld::SetFocusFilterAlpha(float fAlpha)
{
	if(!m_spFocusFilter)
	{
		return;
	}

	NiScreenTexture::ScreenRect& rect = m_spFocusFilter->GetScreenRect(0);
	if(rect.m_kColor.a == fAlpha)
	{
		return;
	}

	rect.m_kColor.a = fAlpha;
	m_spFocusFilter->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
}

PgPilot *PgWorld::CreatePilot(CUnit* pkUnit, bool& bNew)
{
	PG_ASSERT_LOG(pkUnit);
	if(!pkUnit)
	{
		return NULL;
	}

	int iClass = pkUnit->GetAbil(AT_CLASS);
	int const iGender = pkUnit->GetAbil(AT_GENDER);
	BM::GUID const& rkGuid = pkUnit->GetID();
	char const* pcObjectType = NULL;

	switch(pkUnit->UnitType())
	{
	case UT_BOSSMONSTER:
	case UT_MONSTER:
		{
			// 테스트
			//if (pkUnit->GetAbil(AT_MONSTER_TYPE) == EMONTYPE_FLYING)
			{
				pcObjectType = OBJ_TYPE_MONSTER;
			}
		}break;
	case UT_NPC:
		{
			pcObjectType = OBJ_TYPE_NPC;
		}break;
	case UT_PET:
		{
			pcObjectType = OBJ_TYPE_PET;
		}break;
	case UT_OBJECT:
		{
			pcObjectType = OBJ_TYPE_OBJECT;
		}break;
	case UT_GROUNDBOX:
		{
			PgGroundItemBox *pBox = dynamic_cast<PgGroundItemBox*>(pkUnit);
			if( pBox
				&&	g_pkWorld
				&&	g_pkWorld->IsMineItemBox(pBox) )
			{
				iClass += 10000;//내꺼는 10000 더한것
			}
		}break;
	case UT_MYHOME:
		{
			pcObjectType = OBJ_TYPE_MYHOME;
		}break;
	case UT_SUMMONED:
		{
			pcObjectType = OBJ_TYPE_SUMMONED;
		}break;
	case UT_SUB_PLAYER:
		{
			pcObjectType = OBJ_TYPE_SUB_PLAYER;
		}break;
	}
	PG_STAT(PgStatTimerF timerA(g_kWorldStatGroup.GetStatInfo("PgWorld.CreatePilot"), g_pkApp->GetFrameCount()));

	PG_STAT(timerA.Start());
	PG_MEM_LOG
		NILOG(PGLOG_LOG, "[PgWorld] CreatePilot %s - Start Create Pilot\n", MB(rkGuid.str()));
	PgPilot *pkPilot = g_kPilotMan.NewPilot(rkGuid, iClass, iGender, pcObjectType); //! 이미 있는 GUID면 return 0을 한다.
	NILOG(PGLOG_LOG, "[PgWorld] CreatePilot %s - End Create Pilot(%d)\n", MB(rkGuid.str()), pkPilot == NULL ? 0 : 1);
	PG_MEM_LOG
		PG_STAT(timerA.Stop());

	if (pkPilot == NULL)
		bNew = false;
	else
		bNew = true;

	//! 공중형 몬스터 체크
	if( (pkUnit->IsUnitType(UT_MONSTER) || pkUnit->IsUnitType(UT_SUMMONED))
	&& pkUnit->GetAbil(AT_MONSTER_TYPE) == EMONTYPE_FLYING && pkPilot && pkPilot->GetWorldObject())
	{
		PgActorMonster *pkMonster = dynamic_cast<PgActorMonster*>(pkPilot->GetWorldObject());
		if (pkMonster)
			pkMonster->SetMonsterType(EMONTYPE_FLYING);
	}

	return pkPilot;
}

extern int g_iEventNo;
extern SEventRaceData g_kRaceEventInfo;
PgIWorldObject* PgWorld::AddUnit(CUnit* pkUnit, PgPilot* pkPilot,float fPacketReceiveTime)
{
	if (pkUnit == NULL)
	{
		return NULL;
	}

	BM::GUID const kCallerGuid = pkUnit->Caller();
	bool const IsGuardianEntity = (pkUnit->IsUnitType(UT_ENTITY) && pkUnit->GetAbil(AT_CALLER_DELETE));
	//if( pkUnit->IsUnitType(UT_ENTITY) )
	if( IsGuardianEntity )
	{
		pkUnit->Caller(BM::GUID::NullData());
	}

	//소환체인경우
	if(pkUnit->IsUnitType(UT_SUMMONED) && g_kPilotMan.IsMyPlayer(pkUnit->Caller()))
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(pkPlayer)
		{
			pkPlayer->AddSummonUnit(pkUnit->GetID(), pkUnit->GetAbil(AT_CLASS), ESO_IGNORE_MAXCOUNT);
			pkUnit->Caller(BM::GUID::NullData());
		}
	}

	if(int const iType = pkUnit->GetAbil(AT_REG_ADDUNIT_GUID_TYPE))
	{//lua - TBL_REQ_ADD_UNIT_GUID에 정의됨
		lua_tinker::call<void,int,lwGUID>("RegAddUnitGuid", iType, lwGUID(pkUnit->GetID()));
	}

	bool bIsNew = true;
	BM::GUID const& rkGuid = pkUnit->GetID();
	if (pkPilot == NULL)
	{
		pkPilot = CreatePilot(pkUnit, bIsNew);
	}

	if (bIsNew && g_iUseAddUnitThread == 1)
	{
		g_kPilotMan.InsertPilot(rkGuid, pkPilot);
	}

	PG_STAT(PgStatTimerF timerB(g_kWorldStatGroup.GetStatInfo("PgWorld.AddUnit"), g_pkApp->GetFrameCount()));
	PG_STAT(timerB.Start());
	if(!pkPilot)
	{
		bIsNew = false;
		pkPilot = g_kPilotMan.FindPilot(rkGuid);
		if(!pkPilot)
		{
			// 파일럿을 생성하지 못했고, 찾아도 없다면 생성 실패
			NILOG(PGLOG_ERROR, "[PgWorld] AddUnit(%s)(%d) failed\n", MB(rkGuid.str()), pkUnit->UnitType());
			return NULL;
		}
		else
		{
			// 내 유닛일 수 있다... 체크 해주자.
			if( g_kPilotMan.IsMyPlayer(rkGuid) )
			{
				NILOG( PGLOG_LOG, "[PgWorld] My Unit Recv AddUnit!!\n" );

				// 내유닛이면.. 하면 안된다.
				return NULL;
			}

			// 이미 있는 Pilot이라면.. 그냥 위치만 바꾸긴 하는데, 가끔 RemoveObjectQueue에 들어가 있는 경우가 있다 그것을 체크해주자.
			RemoveObjectContainer::iterator itr = m_kRemoveObjectContainer.find(rkGuid);
			if(m_kRemoveObjectContainer.end() != itr)
			{
				NILOG(PGLOG_LOG, "[PgWorld] %s unit is already exist and waiting for remove.\n", MB(rkGuid.str()));
				m_kRemoveObjectContainer.erase(itr);
			}
		}
	}
	NILOG(PGLOG_LOG, "[PgWorld] AddUnit(%s)(%d,%s)(%d)\n", MB(rkGuid.str()), pkUnit->UnitType(), bIsNew ? "new" : "old", g_pkApp->GetFrameCount());

	POINT3 oldPos(0,0,0);
	PG_ASSERT_LOG(pkPilot->GetUnit());
	if (pkPilot->GetUnit() == NULL)
	{
		NILOG(PGLOG_LOG, "[PgWorld] AddUnit, this unit has no unit\n");
	}
	else
	{
		oldPos = pkPilot->GetUnit()->GetPos();
	}

	EUnitType const eUnitType = pkUnit->UnitType();
	pkPilot->SetUnit(pkUnit);
	POINT3 const newPos = pkPilot->GetUnit()->GetPos();
	NILOG(PGLOG_LOG, "[PgWorld] AddUnit(old(%f,%f,%f) new(%f,%f,%f))\n", oldPos.x, oldPos.y, oldPos.z, newPos.x, newPos.y, newPos.z);
	_PgOutputDebugString("[PgWorld] AddUnit %s (old(%f,%f,%f) new(%f,%f,%f))\n",MB(rkGuid.str()), oldPos.x, oldPos.y, oldPos.z, newPos.x, newPos.y, newPos.z);

	PgIWorldObject *pkWorldObject = pkPilot->GetWorldObject();
	PG_ASSERT_LOG(pkWorldObject);

	if (pkWorldObject == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgWorld] AddUnit failed(%s,%d) - no world object\n", MB(pkPilot->GetName()), eUnitType);
		return NULL;
	}

	if (bIsNew)
	{
		pkWorldObject->BeforeUse();

		POINT3 const kSpawnLoc = pkUnit->GetPos();
		NiPoint3 kPoint(kSpawnLoc.x, kSpawnLoc.y, kSpawnLoc.z);
		
		//! 0,0,0 으로 오면 오류라고 생각하고 spawn location을 찾는다. 
		if (kPoint.x == 0 && kPoint.y == 0 && kPoint.z == 0)
		{
			PG_ASSERT_LOG(!"Not Correct Location in AddUnit");
			FindSpawnLoc("char_spawn_1", kPoint);
		}

		ObjectGroupType	kGroup = OGT_NONE;
		switch( eUnitType )
		{
		case UT_PLAYER:		{	kGroup = OGT_PLAYER;	}break;
		case UT_MONSTER:
		case UT_BOSSMONSTER:{	kGroup = OGT_MONSTER;	}break;
		case UT_GROUNDBOX:	{	kGroup = OGT_GROUNDBOX;	}break;
		case UT_PET:		{	kGroup = OGT_PET;		}break;
		case UT_NPC:		{	kGroup = OGT_NPC;		}break;
		case UT_CUSTOM_UNIT:
		case UT_ENTITY:		{	kGroup = OGT_ENTITY;	}break;
		case UT_OBJECT:		{	kGroup = OGT_OBJECT;	}break;
		case UT_MYHOME:		{	kGroup = OGT_MYHOME;	}break;
		case UT_SUB_PLAYER: {	kGroup = OGT_SIMILAR_PLAYER;	}break;
		case UT_SUMMONED:	{	kGroup = OGT_SIMILAR_PLAYER;	}break;
		}
		PG_ASSERT_LOG(kGroup != OGT_NONE);

		// 그룹을 맞게 설정해주어야 한다. (일단은 Unit Type으로 한다)
		if(AddObject(rkGuid, pkWorldObject, kPoint, kGroup))
		{
			PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
			if(pkActor)
			{
				NiPoint3 const& rkDir = pkActor->GetLookingDir();
				if (rkDir == NiPoint3::ZERO)
				{
					pkActor->LockBidirection((m_eUnlockBidirection & eUnitType) != eUnitType);
					pkActor->FindPathNormal();

					BYTE byDirection = (DIR_NONE==pkUnit->FrontDirection()) ? DIR_DOWN : pkUnit->FrontDirection() ;
					if( pkActor->IsMyActor() && pkActor->IsLockBidirection() )//내 플레이어 일 경우에만. 4방향인 경우에만
					{
						byDirection = DIR_RIGHT;
					}
					pkActor->SetLookingDirection( byDirection, false );

					//	NiPoint3 kMovingVector = pkActor->GetPathNormal().UnitCross(NiPoint3::UNIT_Z);
					pkActor->SetMovingDir( pkActor->GetLookingDir() );// MovingVector도 Look으로 일단 만들어 둔다.
				}

				

				if(NULL != dynamic_cast<PgObject*>(pkActor))
				{//오브젝트일경우(파괴오브젝트)
					if(-1 == pkActor->GetAniSequenceID())
					{//초기애니메이션이 없으면
						//Idle 상태로 세팅한다
						pkActor->SetTargetAnimation(std::string("idle_01"), true, true);
					}
				}

				switch( pkUnit->GetAbil(AT_CREATE_SUMMON_DIR) )
				{
				case 1:
					{// 소환자(Caller)와 같은 방향
						PgActor* pkCallerActor = g_kPilotMan.FindActor(kCallerGuid);
						if( pkCallerActor )
						{
							NiPoint3 kDir = pkCallerActor->GetLookingDir();
							pkActor->ConcilDirection(kDir, true);
							pkActor->SetMovingDir(kDir);							// 이동하면서도 방향이 바뀌려면 이렇게 세팅이 필요
						}
					}break;
				default:
					{
						int const iRotateAngle = pkUnit->GetAbil(AT_MON_DEFAULT_ANGLE);
						if(255 != iRotateAngle)
						{//추가된 유닛이 회전값을 가지고 있으면 그 값에 맞게 회전시켜준다.
							//파괴오브젝트의 회전을 위해 만들어진 코드
							float fAngle = (static_cast<float>(iRotateAngle) / 180.0f) * NI_TWO_PI;
							NiMatrix3 kRotateAxZ;
							kRotateAxZ.FromEulerAnglesXYZ(0.0f, 0.0f, fAngle);
							NiQuaternion kRot;
							kRot.FromRotation(kRotateAxZ);
							pkActor->SetRotation(kRot, true);
						}
					}break;
				}
				// 이름을 생성한다.
				pkActor->UpdateName();
			}
		}
		else
		{
			NILOG(PGLOG_ERROR, "[PgWorld] AddUnit %s(%s,%d) - already in object container\n", MB(rkGuid.str()), MB(pkPilot->GetName()), eUnitType);
			return 0;
		}
	}
	else
	{
		if (pkPilot->IsFrozen())
		{
			UnfreezeObject(pkPilot->GetGuid());
		}

		bool bCheckFloor = true;
		switch(eUnitType)
		{
		case UT_NPC:	// NPC는 checkFloor를 type을 보고 결정한다.
			if (pkUnit)
			{
				PgNpc* pkNpc = dynamic_cast<PgNpc*>(pkUnit);
				if (pkNpc->NpcType() == E_NPC_TYPE_CREATURE)
				{
					bCheckFloor = false;
				}
			}
		case UT_PLAYER:
		case UT_MONSTER:
		case UT_BOSSMONSTER:
		case UT_ENTITY:
		case UT_SUMMONED:
		case UT_SUB_PLAYER:
		case UT_CUSTOM_UNIT:
			{
				PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
				PG_ASSERT_LOG(pkActor);

				if(pkActor)
				{
					PgEnergyGauge *pkEnergyGauge = pkActor->GetHPGaugeBar();
					if ( pkEnergyGauge )
					{
						pkEnergyGauge->Reset( pkPilot );
					}

					if(UT_ENTITY==eUnitType && ENTITY_GUARDIAN==pkUnit->GetAbil(AT_ENTITY_TYPE))
					{
						PgEntity* pkEntity = dynamic_cast<PgEntity*>(pkUnit);
						if( pkEntity && !pkEntity->IsEternalLife() )
						{
							float const fAliveTotalTime = pkEntity->LifeTime()/1000.f;
							pkActor->SetAliveTimeGauge(fAliveTotalTime);
							pkActor->SetAutoDeleteActorTimer(fAliveTotalTime);
						}
					}

					NiPoint3 const kPrevPos = pkActor->GetPosition();
					NiPoint3 rkLoc(pkUnit->GetPos().x,pkUnit->GetPos().y,pkUnit->GetPos().z);

					pkActor->SetPosition(FindActorFloorPos(rkLoc));

					_PgOutputDebugString("[SYNC_INFO]PgWorld::AddUnit _Exist_ GUID:%s Name:%s Pos:%f,%f,%f\n", MB(pkPilot->GetGuid().str()),MB(pkPilot->GetName()),rkLoc.x,rkLoc.y,rkLoc.z);

					//pkActor->m_pkController->setPosition(NxExtendedVec3(rkLoc.x, rkLoc.y, rkLoc.z));
					//pkActor->SetTranslate(rkLoc.x, rkLoc.y, rkLoc.z);
					NILOG(PGLOG_LOG, "[PgWorld] AddUnitPosChange(%f,%f,%f) before(%f,%f,%f)\n", rkLoc.x, rkLoc.y, rkLoc.z, kPrevPos.x, kPrevPos.y, kPrevPos.z);

					// 이름을 다시 생성해야 한다.
					pkActor->UpdateName();
				}
			}
			break;
		case UT_GROUNDBOX:
			{
				NxExtendedVec3 const prevPos =NxExtendedVec3(pkWorldObject->GetTranslate().x,pkWorldObject->GetTranslate().y,pkWorldObject->GetTranslate().z);

				NiPoint3	rkLoc(pkUnit->GetPos().x,pkUnit->GetPos().y,pkUnit->GetPos().z);


				rkLoc = FindActorFloorPos(rkLoc);
				rkLoc.z -= PG_CHARACTER_Z_ADJUST;

				pkWorldObject->SetTranslate(rkLoc);

				NILOG(PGLOG_LOG, "[PgWorld] AddUnitPosChange(%f,%f,%f) before(%f,%f,%f)\n", rkLoc.x, rkLoc.y, rkLoc.z, prevPos.x, prevPos.y, prevPos.z);

			}
			break;
		}
	}
	
	switch(eUnitType)
	{
	case UT_NPC:
		{
			PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
			PG_ASSERT_LOG(pkActor);
			if(pkActor)
			{
				pkActor->PopSavedQuestSimpleInfo();
				pkActor->DoReservedAction(PgActor::RA_IDLE);
				if (eUnitType == UT_NPC)
				{
					PgNpc* pkNpc = dynamic_cast<PgNpc*>(pkUnit);
					if (pkNpc && (pkNpc->NpcType() == E_NPC_TYPE_CREATURE))
					{
						pkActor->SetFreeMove(true);
					}
				}
			}
		}break;
	case UT_PLAYER:
		{
			PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkUnit);
			PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
			bool bMyCharacter = g_kPilotMan.IsMyPlayer(pkPilot->GetGuid());
			PG_ASSERT_LOG(pkPlayer);
			PG_ASSERT_LOG(pkActor);
			pkActor->ClearActionState();

			// MemberGuid가 Null이면 ReadPacket으로 Unit을 만든게 아니라, 임의로 만든 것이다.
			if(pkPlayer->GetMemberGUID() != BM::GUID::NullData())
			{
				PLAYER_ABIL kInfo;
				pkPlayer->GetPlayerAbil(kInfo);

				if (bMyCharacter == false)
					g_kSelectStage.EquipBaseItemFullSet(pkActor, kInfo);

				pkActor->AddToDefaultItem(EQUIP_LIMIT_HAIR_COLOR, kInfo.iHairColor);
				pkActor->AddToDefaultItem(EQUIP_LIMIT_FACE, kInfo.iFace);
				pkActor->AddToDefaultItem(EQUIP_LIMIT_HAIR, kInfo.iHairStyle);
				pkActor->AddToDefaultItem(EQUIP_LIMIT_SHIRTS, kInfo.iJacket);
				pkActor->AddToDefaultItem(EQUIP_LIMIT_PANTS, kInfo.iPants);
				pkActor->AddToDefaultItem(EQUIP_LIMIT_BOOTS, kInfo.iShoes);
				pkActor->AddToDefaultItem(EQUIP_LIMIT_GLOVE, kInfo.iGloves);
				pkActor->EquipAllItem();
				PgActorUtil::UpdateColorShadow(pkActor, true);
				
				BM::GUID kSubPlayerGUID = pkPlayer->SubPlayerID();
				if( kSubPlayerGUID.IsNotNull() )
				{// 각 파츠 얻어와서 동일하게 구성
					//PgActor* pkSubPlayerActor = g_kPilotMan.FindActor( kSubPlayerGUID );
					//if( pkSubPlayerActor )
					//{
					//	pkSubPlayerActor->CopyEquipItem(pkActor);
					//	pkSubPlayerActor->DoReservedAction(PgActor::RA_OPENING, true);
					//	pkSubPlayerActor->LockBidirection(false);//8방향 풀기. 안풀면 움직일 때 방향이 자꾸 바뀌어 이상하게 보임
					//}
					PgPilot* pkSubPlayerPilot = g_kPilotMan.FindPilot( kSubPlayerGUID );
					if(pkSubPlayerPilot)
					{
						PgSubPlayer* pkSubPlayer = dynamic_cast<PgSubPlayer*>( pkSubPlayerPilot->GetUnit() );
						if(pkSubPlayer)
						{
							pkSubPlayer->SetCallerUnit(pkPlayer);
						}
						
						PgActor* pkSubPlayerActor = dynamic_cast<PgActor*>( pkSubPlayerPilot->GetWorldObject() );
						if(pkSubPlayerActor)
						{// 각 파츠 얻어와서 동일하게 구성
							pkSubPlayerActor->EquipAllItem_SubPlayer(pkActor);
						}
					}
				}
			}

			if (bMyCharacter)
			{
				// 내 플레이어의 경우 카메라를 셋팅한다.
				g_kPilotMan.SetPlayerPilot(pkPilot->GetGuid());

				PgPlayer * pPlayer = dynamic_cast<PgPlayer *>(pkUnit);
				if( pPlayer )
				{	// 채팅 매니저에 채팅 차단 리스트 추가.
					CONT_BLOCKCHAT_LIST MyChatBlockList;
					pPlayer->GetChatBlockList(MyChatBlockList);

					g_kChatMgrClient.SetChatBlockList(MyChatBlockList);
				}

				m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_FOLLOW, pkActor);
				PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(m_kCameraMan.GetCameraMode());
				if (pkCameraMode)
				{
					pkCameraMode->SetCameraAdjustInfo(m_kCameraAdjustInfo);
				}

				g_kSkillOptionMan.LoadFromFile(pkPilot->GetGuid());
				//	스킬 트리 생성
				g_kSkillTree.CreateSkillTree( pkPlayer );
				lua_tinker::call<void>("InitBasicSkillSet");//기초스킬셋 설정
				g_kUIScene.RecreateAllMiniMap();

				//CustomUI생성
				if( pkPilot->GetBaseClassID() == UCLASS_SHAMAN )
				{
					g_kUnitCustomUI.Init(pkActor);
				}
				else
				{
					g_kUnitCustomUI.Clear();
				}
			}

			switch( GetAttr() )
			{
			case GATTR_BATTLESQUARE:
				{
					int const iTeamNo = g_kBattleSquareMng.GetTeam(pkPlayer->GetID());
					if( BST_NONE == iTeamNo )
					{
						int const iUnvisible = 1;
						pkPlayer->SetAbil(AT_HIDE, iUnvisible);
					}
				}break;
			case GATTR_RACE_GROUND:
				{
					if(g_iEventNo > 0 && g_kRaceEventInfo.MoveSpeed > 0)
					{
						if(NULL != pkPlayer)
						{
							pkPlayer->SetAbil(AT_C_MOVESPEED, g_kRaceEventInfo.MoveSpeed);
						}
					}
				}break;
			default:
				{
				}break;
			}

			//		if(pkPlayer->GetActionID() == ACTIONEFFECT_DIE)
			if ( pkPlayer->IsDead() )
			{
				PgAction	*pkDieAction = pkActor->ReserveTransitAction(ACTIONEFFECT_DIE);
				if(pkDieAction)
				{
					pkDieAction->SetParam(1,"DIE_IDLE");
				}
			}
			else
			{
				if ( !pkActor->IsMyActor() )
				{// 내 Actor 이면 AddUnit일때 Idle을 보내면 안된다.(맵로딩이 다 끝나지 않았는데 맞는다.)
					pkActor->DoReservedAction(PgActor::RA_IDLE);
				}
			}
			pkPlayer->SetCompleteInit(); //여기까지 정상 실행되어야 초기화가 완료된 것이다.
			SetPetCallerUnit(pkPlayer, pkPlayer);

			InitDungeonProgressPosFromPartyMaster(pkPilot);
		}break;
	//case UT_OBJECT:
	//{
	//	if( g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION) )
	//	{
	//		PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);

	//		if( pkActor )
	//		{
	//			if( pkActor->GetCanHit() )
	//			{
	//				std::wstring kActionID = _T("a_opening");
	//				pkActor->TransitAction(MB(kActionID));					
	//				pkActor->SetAnimSpeed(0.01f);
	//			}
	//		}
	//	}
	//}break;
	case UT_BOSSMONSTER:
	case UT_MONSTER:
		{
			PgMonster *pkMonster = dynamic_cast<PgMonster *>(pkUnit);
			PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
			PG_ASSERT_LOG(pkMonster);
			PG_ASSERT_LOG(pkActor);

			pkActor->DoReservedAction(PgActor::RA_IDLE,true);	//	일단 Idle 액션 시켜준다.

			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			std::wstring kActionID = kSkillDefMgr.GetActionName(pkMonster->ActionID());

			if(kActionID == _T("") || pkMonster->ActionID()==0) kActionID = UNI(ACTIONNAME_IDLE);

			// 몬스터는 이전에 하던 행동을 한다.
			_PgOutputDebugString("Monster Init Action From Server : %s\n", MB(kActionID));

			POINT3 kGoalPos = pkMonster->GoalPos();

			//	실제 AddUnit 패킷이 온 시간과, 로딩에 의해 발생된 실제 생성시간(즉 지금)간의 시간차를 고려해서 좌표를 보정한다.
			//  TODO : 로딩이 느릴경우 너무 멀리가는데, 이때 PhysX벽이나 오브젝트를 뚫고 들어가는 경우가 있다. 거리 계산시 그 부분을 체크 해야 함.
			//         혹은 AddUnit된 후에 Packet Flushing을 하는데, 이 부분을 고려해서 체크하는 것도 방법임.
			if(kActionID == _T("a_walk_left") ||
				kActionID == _T("a_walk_right") ||
				kActionID == _T("a_walk_up") ||
				kActionID == _T("a_walk_down"))
			{

				float fNow = NiGetCurrentTimeInSec();
				float const	fMoveSpeed = static_cast<float>(pkUnit->GetAbil(AT_MOVESPEED));

				float	fRecentActionPacketTime = g_kPilotMan.GetFirstPacketBufferedTime(pkPilot->GetGuid());

				if(fRecentActionPacketTime != -1 && fNow>fRecentActionPacketTime)
				{
					fNow = fRecentActionPacketTime;
				}

				float	fElapsedTime = fNow - fPacketReceiveTime;

				if(fPacketReceiveTime == 0)
				{
					fElapsedTime = 0;
				}

				NiPoint3	kStartPos(pkUnit->GetPos().x,pkUnit->GetPos().y,pkUnit->GetPos().z);
				NiPoint3	kTargetPos(kGoalPos.x,kGoalPos.y,kGoalPos.z);

				float	fTotalDistance = (kTargetPos - kStartPos).Length();
				float	fCurrDistance = fMoveSpeed*fElapsedTime;

				if(fCurrDistance<fTotalDistance)
				{
					pkActor->TransitAction(MB(kActionID));

					NiPoint3	kMovingVector = (kTargetPos-kStartPos);
					kMovingVector.Unitize();

					NiPoint3	kAdjustedStartPos = kStartPos + kMovingVector*fCurrDistance;

					_PgOutputDebugString("[SYNC_INFO]PgWorld::AddUnit Monster GUID:%s Init Position (%f,%f,%f)  Adjusted(%f,%f,%f) ElapsedTime:%f MoveSpeed:%f\n", MB(pkPilot->GetGuid().str()), kStartPos.x,kStartPos.y,kStartPos.z, kAdjustedStartPos.x,kAdjustedStartPos.y,kAdjustedStartPos.z, fElapsedTime,fMoveSpeed);

					pkActor->SetPosition(FindActorFloorPos(kAdjustedStartPos));
					pkActor->SetWalkingTargetLoc(kTargetPos, true, ACTIONNAME_IDLE, true);
				}
			}
			else
			{
				pkActor->TransitAction(MB(kActionID));
			}
			_PgOutputDebugString("[SYNC_INFO]PgWorld::AddUnit Monster Init Action GUID:%s Name:%s ActionID : %s TargetPos:%f,%f,%f\n", MB(pkPilot->GetGuid().str()),MB(pkPilot->GetName()),MB(kActionID),kGoalPos.x,kGoalPos.y,kGoalPos.z);
		}break;
	case UT_PET:
		{
			//라이딩펫은 주인을 참조해야 한다.
			PgPet* pkPet = dynamic_cast<PgPet*>(pkUnit);
			if(pkPet != NULL)
			{
				PgPilot* pkCallerPilot = g_kPilotMan.FindPilot( pkUnit->Caller() );
				PgPlayer* pkCallerUnit = NULL;
				if(pkCallerPilot != NULL)
				{
					pkCallerUnit = dynamic_cast<PgPlayer*>(pkCallerPilot->GetUnit());
				}
				if(pkCallerUnit != NULL)
				{
					if(pkCallerUnit->IsCompleteInit() == false)
					{
						pkCallerUnit = NULL;
					}
				}
				pkPet->SetCallerUnit(pkCallerUnit);
			}

			PgActorPet* pkActor = dynamic_cast<PgActorPet* >(pkWorldObject);
			if(pkActor)
			{
				GET_DEF(PgClassPetDefMgr, kClassDefMgr);
				PgClassPetDef kPetDef;
				if ( true == kClassDefMgr.GetDef( SClassKey(pkUnit->GetAbil(AT_CLASS), pkUnit->GetAbil(AT_LEVEL)), &kPetDef ))
				{
					pkActor->AddToDefaultItem(EQUIP_LIMIT_PET_HEAD, kPetDef.GetDefaultHair());
					pkActor->AddToDefaultItem(EQUIP_LIMIT_PET_BODY, kPetDef.GetDefaultBody());
					pkActor->AddToDefaultItem(EQUIP_LIMIT_FACE, kPetDef.GetDefaultFace());
				}

				if(pkActor->GetUnit())
				{
					if(pkActor->GetUnit()->GetAbil(AT_MOUNTED_PET_SKILL))
					{
						pkActor->ReserveTransitAction(ACTIONNAME_RIDING);
					}
					else
					{
						pkActor->DoReservedAction(PgActor::RA_OPENING, true);
					}
				}
				
				BM::GUID kPlayerGuid;
				g_kPilotMan.GetPlayerPilotGuid(kPlayerGuid);
				if(pkUnit->Caller() == kPlayerGuid)	//내 팻만 하자
				{
					pkActor->ResetSkillCoolTimeFromUnit();
					if ( PgActorPet::ms_pkMyPetInventory && PgActorPet::ms_pkMyPetInventory->OwnerGuid() == pkUnit->GetID() )
					{
						pkUnit->GetInven()->Swap( *PgActorPet::ms_pkMyPetInventory );
						pkUnit->InvenRefreshAbil();
					}
					SAFE_DELETE( PgActorPet::ms_pkMyPetInventory );
					SAFE_DELETE( PgActorPet::ms_pkTempPetInventory );

					XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"CharInfo");//UI가 열려 있으면 버튼 상태를 바꿔 줘야 함
					if(pkWnd)
					{
						lwCharInfo::ChangePetActor(pkUnit->GetID());
						lwCharInfo::SetPetInfoToUI(pkWnd->GetControl(L"FRM_PET_INFO"), pkUnit->GetID(), true);
					}
				}

				pkActor->EquipAllItem();	//기본아이템 장착
				pkActor->LockBidirection(false);//8방향 풀기. 안풀면 펫이 움직일 때 방향이 자꾸 바뀌어 이상하게 보임
			}
		}break;
	case UT_GROUNDBOX:
		{// 등장 연출
			PgDropBox *pkBox = dynamic_cast<PgDropBox *>(pkWorldObject);
			PG_ASSERT_LOG(pkBox);

			if( pkBox )
			{
				bool bAddedToActor = false;
				PgPilot* pkBoxPilot = pkBox->GetPilot();
				NiPoint3 kFromPos = pkBox->GetWorldTranslate();
				bool bByGodCMD = false;
				if( pkBoxPilot )
				{
					PgGroundItemBox	*pkBoxUnit = dynamic_cast<PgGroundItemBox*>(pkBoxPilot->GetUnit());
					if( pkBoxUnit )
					{
						BM::GUID kPlayerGuid;
						g_kPilotMan.GetPlayerPilotGuid(kPlayerGuid);
						BM::GUID const& kCaller = pkBoxUnit->Caller();//떨군 놈
						bByGodCMD = (kCaller==kPlayerGuid);
						if( BM::GUID::NullData() != kCaller && false==bByGodCMD )
						{
							PgPilot	*pkCallerPilot = g_kPilotMan.FindPilot(kCaller);
							if( pkCallerPilot )
							{
								PgActor* pkCallerActor = dynamic_cast<PgActor* >(pkCallerPilot->GetWorldObject());
								if( pkCallerActor )
								{
									kFromPos = pkCallerActor->GetPosition();//Pos 저장
									if(pkCallerPilot->GetUnit()->UnitType()==UT_PLAYER)
									{
										bByGodCMD = true;
									}

									if(false==bByGodCMD)
									{
										bAddedToActor = pkCallerActor->AddDropItem(pkBox);
									}

									//SItem kItem; // PgWorkerThread에서 미리 한다.
									//if ( S_OK == pkBoxUnit->PopItem(kItem, true) )
									//{
									//	pkBox->SetItemNum(kItem);
									//}
								}
							}
							else
							{
								//SItem kItem; // PgWorkerThread에서 미리 한다.
								//if ( S_OK == pkBoxUnit->PopItem(kItem, true) )
								//{
								//	pkBox->SetItemNum(kItem);
								//}
							}
						}
					}
				}

				if( !bAddedToActor )
				{
					pkBox->SetOfferer(BM::GUID::NullData(), kFromPos);
					/*if(false==bByGodCMD)
					{
					RemoveObjectOnNextUpdate(rkGuid); //자진 삭제
					pkBox->RemoveReserve(true);
					}*/
				}
			}
		}break;
	case UT_MYHOME:
		{
			PgMyHome *pkMyHome = dynamic_cast<PgMyHome *>(pkUnit);
			PG_ASSERT_LOG(pkMyHome);
			if( MYHOME_CLASS_INSIDE_NO==pkUnit->GetAbil(AT_CLASS) && GetHome() )
			{
				if( GetHome() )
				{
					GetHome()->SetHomeUnit(pkUnit);
					GetHome()->HomeUnitAddProcess();
					lwHomeUI::SendMyHomeInfoRequest();
				}
			}
			else
			{
				PgHouse* pkHouse = dynamic_cast<PgHouse* >(pkWorldObject);
				PG_ASSERT_LOG(pkHouse);//임시

				pkMyHome->SetAbil(AT_GENDER, 1);//임시다

				pkHouse->EquipAllExteria();
				//pkHouse->AddAllFurniture();
			}
		}break;
	case UT_SUB_PLAYER:
		{
			PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
			if(pkActor)
			{// 
				if( pkUnit->Caller().IsNotNull() )
				{
					PgPilot* pkCallerPilot = g_kPilotMan.FindPilot( pkUnit->Caller() );
					if(pkCallerPilot)
					{
						PgPlayer* pkPlayer = dynamic_cast<PgPlayer *>( pkCallerPilot->GetUnit() );
						if(pkPlayer)
						{
							pkPlayer->SubPlayerID( pkUnit->GetID() );
							PgSubPlayer* pkSubPlayer = dynamic_cast<PgSubPlayer* >(pkUnit);
							if(pkSubPlayer)
							{// 메인 캐릭터의 Unit을 지정
								pkSubPlayer->SetCallerUnit(pkPlayer);
							}
						}
						PgActor* pkCallerActor = dynamic_cast<PgActor*>( pkCallerPilot->GetWorldObject() );
						if(pkCallerActor)
						{// 각 파츠 얻어와서 동일하게 구성
							pkActor->EquipAllItem_SubPlayer(pkCallerActor);
						}
					}
				}
				pkActor->DoReservedAction(PgActor::RA_OPENING, true);
				pkActor->LockBidirection(false);//8방향 풀기. 안풀면 움직일 때 방향이 자꾸 바뀌어 이상하게 보임
			}
		}break;
	case UT_SUMMONED:
		{
			PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
			if(pkActor)
			{
				pkActor->DoReservedAction(PgActor::RA_OPENING, true);
				pkActor->LockBidirection(false);//8방향 풀기. 안풀면 펫이 움직일 때 방향이 자꾸 바뀌어 이상하게 보임
			}
		}break;
	case UT_CUSTOM_UNIT:
		{
			PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
			if(pkActor)
			{
				pkActor->ReserveTransitAction("a_pvp_flag_default");
			}
		}break;
	case UT_ENTITY:
		{
			PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
			if(pkActor)
			{
				if(pkUnit->ActionID())
				{
					pkActor->ReserveTransitAction( pkUnit->ActionID(), DIR_NONE );
				}
				else
				{
					int const iSkillNo = pkUnit->GetAbil(AT_MON_SKILL_01);
					int const iNotTrans = pkUnit->GetAbil(AT_MON_ENTITY_NOT_TRANS_SKILL1);
					if ( IsGuardianEntity )
					{
						PgAction *pkAction = pkActor->ReserveTransitAction( "a_opening", DIR_NONE );
					}
					else if ( iSkillNo && !iNotTrans )
					{
						PgAction *pkAction = pkActor->ReserveTransitAction( iSkillNo, 0 );
					}
				}
			}
		}
	default:
		{
		}break;
	}

	PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
	if(pkActor)
	{
		//	Scale 처리하자.
		float	fScale = pkUnit->GetAbil(AT_UNIT_SCALE)/ABILITY_RATE_VALUE_FLOAT;
		if(fScale == 0.0f)
		{
			fScale = 1.0f;
		}
		PG_ASSERT_LOG(pkActor->GetNIFRoot());
		if(pkActor->GetNIFRoot())
		{
			pkActor->GetNIFRoot()->SetScale(pkActor->GetScale()*fScale);
		}

		switch( eUnitType )
		{
		case UT_PLAYER:
			{// 플레이어만 경사각에서 미끄러질 필요가 있으므로 설정해주고
				pkActor->IgnoreSlide(false);
			}break;
		default:
			{// 나머지는 경사각 미끄러지기를 꺼주고
				pkActor->IgnoreSlide(true);
			}break;
		}
		
		switch(eUnitType)
		{
		case UT_PLAYER:
			{
				pkActor->SetObjectID(PgIXmlObject::ID_PC);
			}break;
		case UT_NPC:
			{
				pkActor->SetObjectID(PgIXmlObject::ID_NPC);
			}break;
		case UT_GROUNDBOX:
			{
				pkActor->SetObjectID(PgIXmlObject::ID_DROPBOX);
			}break;
		case UT_PET:
			{
				pkActor->SetObjectID(PgIXmlObject::ID_PET);
			}break;
		case UT_MONSTER:
			{
				pkActor->SetObjectID(PgIXmlObject::ID_MONSTER);
			}break;
		case UT_BOSSMONSTER:
			{
				pkActor->SetObjectID(PgIXmlObject::ID_BOSS);
			}break;
		case UT_ENTITY:
			{
				pkActor->SetObjectID(PgIXmlObject::ID_ENTITY);
			}break;
		case UT_SUB_PLAYER:
			{// 흠 필요할까?
				pkActor->SetObjectID(PgIXmlObject::ID_SUB_PLAYER);
			}break;
		case UT_CUSTOM_UNIT:
			{
				pkActor->SetObjectID(PgIXmlObject::ID_ENTITY);
			}break;
		default:
			break;
		}

		//XML에 등록된 아이템 여기서 장착하기, PgPilotMan::NewPilot()에서 하려고 했는데 라이딩팻과 칼키가 동시에 있는 상태에서 맵이동시 칼키가 보이지 않는 문제가 발생함. 원인은 아직 잘 모르겠음
		int const iCheckEquipUnitType = UT_MONSTER | UT_ENTITY | UT_SUMMONED;
		if(eUnitType&iCheckEquipUnitType)
		{
			if(pkActor->IsEquipItemList())
			{
				pkActor->EquipAllItem();
			}
		}
		if(IsSaveHideActor(pkActor))
		{
			pkActor->SetHide(true);
			pkActor->SetHideShadow(true);
			if(pkActor->GetUnit())
			{
				pkActor->GetUnit()->SetAbil(AT_EVENTSCRIPT_HIDE, true);
			}
		}
		if(g_kPilotMan.IsHideBalloon())
		{
			pkActor->SetHideBalloon(true);
		}
	}

	PG_STAT(timerB.Stop());
	return pkWorldObject;
} 
bool	PgWorld::FindPathNormal(NiPoint3 const& kPosition,NiPoint3 &kResult)
{
	NiPick	kPick;
	kPick.SetPickType(NiPick::FIND_FIRST);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	kPick.SetCoordinateType(NiPick::WORLD_COORDINATES);
	kPick.SetReturnNormal(true);

	static NiPoint3 akDirs[] =
	{
		NiPoint3(1.0f, 0.0f, 0.0f),
		NiPoint3(-1.0f, 0.0f, 0.0f),
		NiPoint3(0.0f, 1.0f, 0.0f),
		NiPoint3(0.0f, -1.0f, 0.0f),
	};

	kPick.SetTarget(m_spPathRoot);
	kPick.ClearResultsArray();

	NxVec3 ktLoc(kPosition.x,kPosition.y,kPosition.z);

	NiPoint3 kPickStart = NiPoint3(ktLoc.x, ktLoc.y, ktLoc.z);

	NiQuaternion	kQuat(90.0f * 3.141592f / 180.0f,NiPoint3::UNIT_Z);
	NiMatrix3	kAxisRot;

	kQuat.ToRotation(kAxisRot);

	for(int i = 0; i < 4; ++i)
	{
		NiPoint3 kDir = akDirs[i] * kAxisRot;
		kDir.Unitize();
		kPick.PickObjects(kPickStart, kDir, true);
	}

	NiPick::Results& rkResults = kPick.GetResults();

	if(rkResults.GetSize() == 0)
	{
		return false;
	}

	NiPick::Record *pkRecord = rkResults.GetAt(0);

	kResult = pkRecord->GetNormal();

	return	true;
}
void PgWorld::processTemporaryInput()
{
	if (!g_pkLocalManager || g_pkLocalManager->GetInputSystem() == NULL)
	{
		return;
	}

	NiInputKeyboard* pkKeyboard = g_pkLocalManager->GetInputSystem()->GetKeyboard();
	if (pkKeyboard == NULL)
	{
		return;
	}
#ifndef EXTERNAL_RELEASE
	if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPADENTER))
		m_iDraw = (m_iDraw + 1) % 4;
#endif

	//! Camera Adjust
	if (m_bUseCameraAdjust && 
		(m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW || m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW_DEPTH_PATH))
	{
		PgCameraModeFollow* pkCameraMode = dynamic_cast<PgCameraModeFollow*>(m_kCameraMan.GetCameraMode());
		if (pkCameraMode != NULL)
		{// 카메라 모드가 존재 하면
			
			NiPoint3 const& kvDir	= pkCameraMode->GetCamera()->GetWorldDirection();
			NiPoint3 const& kvRight = pkCameraMode->GetCamera()->GetWorldRightVector();
			//NiPoint3 const& kvUp	= pkCameraMode->GetCamera()->GetWorldUpVector();
			CameraInfo& kCamAdInfo  = pkCameraMode->GetCameraAdjustInfo().kCameraAdjust;
			
			float const fAmountMove = 5.0f;
			// 키입력시 카메라의

			if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD8))
			{// 높이값을 증가
				pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fCameraHeight += fAmountMove;
			}

			if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD2))
			{// 높이값을 감소
				pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fCameraHeight -= fAmountMove;
			}

			if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_ADD))
			{// Zoom In				
				NiPoint3 const kV = kvDir * fAmountMove;
				kCamAdInfo.fCameraWidth			+= kV.x;
				kCamAdInfo.fDistanceFromTarget	-= kV.y;
				kCamAdInfo.fCameraHeight		+= kV.z;
			}

			if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_SUBTRACT))
			{// Zoom Out				
				NiPoint3 const kV = kvDir * -fAmountMove;
				kCamAdInfo.fCameraWidth			+= kV.x;
				kCamAdInfo.fDistanceFromTarget	-= kV.y;
				kCamAdInfo.fCameraHeight		+= kV.z;				
			}

			if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD7))
			{// LookAt의 높이값 증가
				pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fLookAtHeight -= fAmountMove;
			}

			if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD9))
			{// LookAt의 높이값 감소
				pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fLookAtHeight += fAmountMove;
			}
			
			if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD4))
			{// 바라보는 방향을 기준으로 좌로 이동				
				NiPoint3 const kV = kvRight * -fAmountMove;
				kCamAdInfo.fCameraWidth			+= kV.x;
				kCamAdInfo.fDistanceFromTarget	-= kV.y;
				kCamAdInfo.fCameraHeight		+= kV.z;
				
			}
			if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD6))
			{// 바라보는 방향을 기준으로 우로 이동				
				NiPoint3 const kV = kvRight * fAmountMove;
				kCamAdInfo.fCameraWidth			+= kV.x;
				kCamAdInfo.fDistanceFromTarget	-= kV.y;
				kCamAdInfo.fCameraHeight		+= kV.z;
			}

			if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD1))
			{// 바라보는 방향을 기준으로 좌로 회전				
				NiPoint3 const kV = kvRight * -fAmountMove;
				kCamAdInfo.fLookAtWidth	 += kV.x;
				kCamAdInfo.fLookAtDepth	 += kV.y;
				kCamAdInfo.fLookAtHeight += kV.z;
			}
			if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD3))
			{// 바라보는 방향을 기준으로 우로 회전				
				NiPoint3 const kV = kvRight * fAmountMove;
				kCamAdInfo.fLookAtWidth	 += kV.x;
				kCamAdInfo.fLookAtDepth	 += kV.y;
				kCamAdInfo.fLookAtHeight += kV.z;
			}
			
			//if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD0))
			//{// LookAt의 깊이 값은 사용자에게 혼동만 줌
			//	//pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fLookAtDepth -= 5.0f;
			//	NiPoint3 kV = kvDir * 5.0f;
			//	kCamAdInfo.fLookAtWidth	 += kV.x;
			//	kCamAdInfo.fLookAtDepth	 += kV.y;
			//	kCamAdInfo.fLookAtHeight += kV.z;
			//}
			//if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_DECIMAL))
			//{
			//	//pkCameraMode->GetCameraAdjustInfo().kCameraAdjust.fLookAtDepth += 5.0f;
			//	NiPoint3 kV = kvDir * -5.0f;
			//	kCamAdInfo.fLookAtWidth	 += kV.x;
			//	kCamAdInfo.fLookAtDepth	 += kV.y;
			//	kCamAdInfo.fLookAtHeight += kV.z;
			//}
		}
	}

	//! Bloom
	PgPostProcessMan *pPostProcessMan = g_kRenderMan.GetRenderer()->m_pkPostProcessMan;
	PgIPostProcessEffect *pBloom = 0;
	if (pPostProcessMan)
	{
		pBloom = pPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
	}
	/*
	if (pBloom && pBloom->GetActive())
	{
	if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_0))
	pBloom->SetActive(false);

	if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_HOME))
	pBloom->SetParam("SceneIntensity",pBloom->GetParam("SceneIntensity") - 0.005f);

	if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_END))
	pBloom->SetParam("SceneIntensity",pBloom->GetParam("SceneIntensity") + 0.005f);

	if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_INSERT))
	pBloom->SetParam("GlowIntensity",pBloom->GetParam("GlowIntensity") - 0.005f);

	if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_PRIOR))
	pBloom->SetParam("GlowIntensity",pBloom->GetParam("GlowIntensity") + 0.005f);
	}*/

	// Fog
	if (m_pkFogProperty && m_bUseFogAdjust && m_bUseLODAdjust == false)
	{
		NiFogProperty* skyBoxFog = NULL;
		if (m_spSkyRoot)
			skyBoxFog = (NiFogProperty*)m_spSkyRoot->GetProperty(NiProperty::FOG);

#ifdef PG_USE_NEW_FOG
		bool bShiftDown = pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_LSHIFT) || pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_RSHIFT);
		_PgOutputDebugString("bShiftDown : %d\n",bShiftDown);

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_ADD))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_ADD))
			{
				m_kKeyPressedTIme[0] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[0] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[0]) / 1000), 1);
			}

			if (bShiftDown)
				m_pkFogProperty->SetFogStart(m_pkFogProperty->GetFogStart() + value);
			else
				m_pkFogProperty->SetFogEnd(m_pkFogProperty->GetFogEnd() + value);
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_SUBTRACT))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_SUBTRACT))
			{
				m_kKeyPressedTIme[1] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[1] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[1]) / 1000), 1);
			}

			if (bShiftDown)
				m_pkFogProperty->SetFogStart(m_pkFogProperty->GetFogStart() - value);
			else
				m_pkFogProperty->SetFogEnd(m_pkFogProperty->GetFogEnd() - value);
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD7))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD7))
			{
				m_kKeyPressedTIme[2] = BM::GetTime32();
				value = 0.002f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[2] > 200)
			{
				value = 0.005f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[2]) / 1000), 1);
			}

			if (bShiftDown)
				m_kFogColor.r -= value;
			else
				m_kFogColor.r += value;
		}
		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD8))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD8))
			{
				m_kKeyPressedTIme[3] = BM::GetTime32();
				value = 0.002f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[3] > 200)
			{
				value = 0.005f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[3]) / 1000), 1);
			}

			if (bShiftDown)
				m_kFogColor.g -= value;
			else
				m_kFogColor.g += value;
		}
		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD9))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD9))
			{
				m_kKeyPressedTIme[4] = BM::GetTime32();
				value = 0.002f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[4] > 200)
			{
				value = 0.005f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[4]) / 1000), 1);
			}

			if (bShiftDown)
				m_kFogColor.b -= value;
			else
				m_kFogColor.b += value;
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD0))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD0))
			{
				m_kKeyPressedTIme[5] = BM::GetTime32();
				value = 0.005f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[5] > 200)
			{
				value = 0.01f;
			}

			if (bShiftDown)
				m_pkFogProperty->SetFogDensity(m_pkFogProperty->GetFogDensity() - value);
			else
				m_pkFogProperty->SetFogDensity(m_pkFogProperty->GetFogDensity() + value);
		}
#else
		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_ADD))
			m_pkFogProperty->SetDepth(m_pkFogProperty->GetDepth() + 0.05f);

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_SUBTRACT))
			m_pkFogProperty->SetDepth(m_pkFogProperty->GetDepth() - 0.05f);

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD7))
			m_kFogColor.r += 0.05f;
		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD4))
			m_kFogColor.r -= 0.05f;
		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD8))
			m_kFogColor.g += 0.05f;
		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD5))
			m_kFogColor.g -= 0.05f;
		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD9))
			m_kFogColor.b += 0.05f;
		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD6))
			m_kFogColor.b -= 0.05f;

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD1))
			g_kFrustum.m_fFar += 100;

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD2))
			g_kFrustum.m_fFar += 100;
#endif

		if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_DIVIDE))
		{
			if (m_pkFogProperty->GetFogFunction() == NiFogProperty::FOG_RANGE_SQ)
				m_pkFogProperty->SetFogFunction(NiFogProperty::FOG_Z_LINEAR);
			else
				m_pkFogProperty->SetFogFunction(NiFogProperty::FOG_RANGE_SQ);
		}

		if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_DECIMAL))
		{
			m_bUseFogAtSkybox = !m_bUseFogAtSkybox;
			if (skyBoxFog)
			{
				skyBoxFog->SetFog(m_bUseFogAtSkybox);
			}
		}

		m_pkFogProperty->SetFogColor(m_kFogColor);
		if (skyBoxFog)
		{
			skyBoxFog->SetFogDensity(m_pkFogProperty->GetFogDensity());
			skyBoxFog->SetFogStart(m_pkFogProperty->GetFogStart());
			skyBoxFog->SetFogEnd(m_pkFogProperty->GetFogEnd());
			skyBoxFog->SetFogColor(m_pkFogProperty->GetFogColor());
			skyBoxFog->SetFogFunction(m_pkFogProperty->GetFogFunction());
			skyBoxFog->SetFogAlpha(m_pkFogProperty->GetFogAlpha());
		}

        NewWare::Scene::ApplyTraversal::Geometry::SetMaterialNeedsUpdate( m_spSceneRoot, true );
	}

	if (m_bUseLODAdjust && m_bUseFogAdjust == false)
	{
		bool bShiftDown = pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_LSHIFT) || pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_RSHIFT);
		bool bControlDown = pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_LCONTROL) || pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_RCONTROL);

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD1))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD1))
			{
				m_kKeyPressedTIme[0] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[0] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[0]) / 1000), 1);
			}

			if (bShiftDown)
				m_afWorldObjectLODRange[0] -= value;
			else
				m_afWorldObjectLODRange[0] += value;
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD2))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD2))
			{
				m_kKeyPressedTIme[1] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[1] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[1]) / 1000), 1);
			}

			if (bShiftDown)
				m_afWorldObjectLODRange[1] -= value;
			else
				m_afWorldObjectLODRange[1] += value;
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD3))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD3))
			{
				m_kKeyPressedTIme[2] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[2] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[2]) / 1000), 1);
			}

			if (bShiftDown)
				m_afWorldObjectLODRange[2] -= value;
			else
				m_afWorldObjectLODRange[2] += value;
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD4))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD4))
			{
				m_kKeyPressedTIme[3] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[3] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[3]) / 1000), 1);
			}

			if (bShiftDown)
				m_afWorldQualityRange[0] -= value;
			else
				m_afWorldQualityRange[0] += value;
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD5))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD5))
			{
				m_kKeyPressedTIme[4] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[4] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[4]) / 1000), 1);
			}

			if (bShiftDown)
				m_afWorldQualityRange[1] -= value;
			else
				m_afWorldQualityRange[1] += value;
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD6))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD6))
			{
				m_kKeyPressedTIme[5] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[5] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[5]) / 1000), 1);
			}

			if (bShiftDown)
				m_afWorldQualityRange[2] -= value;
			else
				m_afWorldQualityRange[2] += value;
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD7))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD7))
			{
				m_kKeyPressedTIme[6] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[6] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[6]) / 1000), 1);
			}

			if (bShiftDown)
				m_afViewDistanceRange[0] -= value;
			else
				m_afViewDistanceRange[0] += value;
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD8))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD8))
			{
				m_kKeyPressedTIme[7] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[7] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[7]) / 1000), 1);
			}

			if (bShiftDown)
				m_afViewDistanceRange[1] -= value;
			else
				m_afViewDistanceRange[1] += value;
		}

		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_NUMPAD9))
		{
			float value = 0.0f;
			if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_NUMPAD9))
			{
				m_kKeyPressedTIme[8] = BM::GetTime32();
				value = 5.0f;
			}
			else if (BM::GetTime32() - m_kKeyPressedTIme[8] > 200)
			{
				value = 20.0f * NiMin((int)((BM::GetTime32() - m_kKeyPressedTIme[8]) / 1000), 1);
			}

			if (bShiftDown)
				m_afViewDistanceRange[2] -= value;
			else
				m_afViewDistanceRange[2] += value;
		}
	}

	if(g_bUseSound)
	{
		if (pkKeyboard->KeyIsDown(NiInputKeyboard::KEY_F5))
		{
			g_kSoundMan.ResetAudioSource();
			g_kWeaponSoundMan.ResetWeaponSound();
		}	
	}

	if (pkKeyboard->KeyWasPressed(NiInputKeyboard::KEY_F4))
	{
		// 아이템 정보를 로그로 남긴다 - 아이템 사라지는 버그를 잡았다고 확신할 때 까지만.
		PgActor* pkActor = g_kPilotMan.GetPlayerActor();
		if (pkActor)
		{
			pkActor->PrintItemInfo();
		}
	}
}

void PgWorld::DeleteAddUnitInfo(CUnit* pkUnit, PgPilot* pkPilot)
{
	if (!pkUnit || !pkPilot) { return; }

	if (pkPilot)
	{
		PgIWorldObject* object = pkPilot->GetWorldObject();
		if (object)
		{
			object->SetPilot(0); // 부모와의 연결을 제거한다.
			object->BeforeCleanUp();
			THREAD_DELETE_ACTOR(object);
		}
		if (pkPilot->GetUnit() != pkUnit)
		{
			NILOG(PGLOG_LOG, "[PgPilot] %s unit(%#X) is destroying\n", MB(pkUnit->GetID().str()), pkUnit);
			g_kTotalObjMgr.UnRegistUnit(pkUnit);
			g_kTotalObjMgr.ReleaseUnit(pkUnit);
		}
		SAFE_DELETE_NI(pkPilot);
	}
	else
	{
		NILOG(PGLOG_LOG, "[PgPilot] %s unit(%#X) is destroying\n", MB(pkUnit->GetID().str()), pkUnit);
		g_kTotalObjMgr.UnRegistUnit(pkUnit);
		g_kTotalObjMgr.ReleaseUnit(pkUnit);
	}
}

void PgWorld::EnqueueAddUnitInfo(CUnit* pkUnit, PgPilot* pkPilot,float fPacketReceiveTime)
{
	if (pkUnit == NULL || pkPilot == NULL)
	{
		return;
	}

	_PgOutputDebugString("[PgWorld] %d frame, EnqueueAddUnit(%s)\n", g_pkApp->GetFrameCount(), MB(pkUnit->GetID().str()));
	m_kAddUnitQueueLock.Lock();

	for (AddUnitQueue::iterator iter = m_kAddUnitQueue.begin(); iter != m_kAddUnitQueue.end(); ++iter)
	{
		if (iter->pkUnit)
		{
			if (pkUnit->GetID() == iter->pkUnit->GetID())
			{
				NILOG(PGLOG_ERROR, "[PgWorld] EnqueueAddUnitInfo %s guid already in AddUnitQueue\n", MB(pkUnit->GetID().str()));
			}
		}
	}

	//if (pkUnit->UnitType() == UT_PLAYER)
	//{
	//	PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkUnit);
	//	PgActor* pkActor = dynamic_cast<PgActor* >(pkPilot->GetWorldObject());
	//	PG_ASSERT_LOG(pkPlayer);
	//	PG_ASSERT_LOG(pkActor);

	//	// MemberGuid가 Null이면 ReadPacket으로 Unit을 만든게 아니라, 임의로 만든 것이다.
	//	if(pkPlayer->GetMemberGUID() != BM::GUID::NullData())
	//	{
	//		PLAYER_ABIL kInfo;
	//		pkPlayer->GetPlayerAbil(kInfo);

	//		g_kSelectStage.EquipBaseItemFullSet(pkActor, kInfo);
	//		pkActor->AddToDefaultItem(EQUIP_LIMIT_HAIR_COLOR, kInfo.iHairColor);
	//		pkActor->AddToDefaultItem(EQUIP_LIMIT_FACE, kInfo.iFace);
	//		pkActor->AddToDefaultItem(EQUIP_LIMIT_HAIR, kInfo.iHairStyle);
	//		pkActor->AddToDefaultItem(EQUIP_LIMIT_SHIRTS, kInfo.iJacket);
	//		pkActor->AddToDefaultItem(EQUIP_LIMIT_PANTS, kInfo.iPants);
	//		pkActor->AddToDefaultItem(EQUIP_LIMIT_BOOTS, kInfo.iShoes);
	//		pkActor->AddToDefaultItem(EQUIP_LIMIT_GLOVE, kInfo.iGloves);
	//		pkActor->EquipAllItem();
	//	}
	//}
	AddUnitInfo info;
	info.pkUnit = pkUnit;
	info.pkPilot = pkPilot;
	info.fPacketReceiveTime = fPacketReceiveTime;
	m_kAddUnitQueue.push_back(info);
	m_kAddUnitQueueLock.Unlock();
	_PgOutputDebugString("EnqueuedAddUnit(%s)\n", MB(pkUnit->GetID().str()));
}
void PgWorld::RemoveAllObjectExceptMe()
{
	for(int i=0;i<MAX_OBJ_CONT;++i)
	{
		ObjectContainer::iterator itr = m_kObjectContainer[i].begin();
		PgIWorldObject *pkObject = NULL;
		while (m_kObjectContainer[i].end() != itr)
		{
			pkObject = itr->second;
			if(pkObject->IsMyActor() == false)
			{
				removePilot(itr);
				itr = m_kObjectContainer[i].erase(itr);
				continue;
			}
			++itr;
		}
	}
}
void PgWorld::RemoveAllObject(bool bExceptNPC)
{
	if (m_kRemoveObjectContainer.size() > 0)
	{
		RemoveObjectContainer::iterator itrE = m_kRemoveObjectContainer.begin();
		while (itrE != m_kRemoveObjectContainer.end())
		{
			NILOG(PGLOG_WARNING, "[PgWorld] Object : %s didn't removed\n", MB(itrE->str()));
			++itrE;
		}		
	}
	processRemoveObjectList();		// PilotMan에서 제거가 되어야 할게 있기 때문에 process를 해준다.
	ClearAllObjects(bExceptNPC);
	m_kRemoveObjectContainer.clear();
	m_kUnfreezeObjectList.clear();	// unfreeze된 것들은 ClearAllObject에서 처리가 된다.

	g_kBreakableObjectGroupMan.Terminate();
}

void PgWorld::RemoveAllMonster()
{
	for(int i=0;i<MAX_OBJ_CONT;++i)
	{
		ObjectContainer::const_iterator itr = m_kObjectContainer[i].begin();
		PgIWorldObject *pkObject = NULL;
		while (m_kObjectContainer[i].end() != itr)
		{
			pkObject = itr->second;
			if (pkObject->GetObjectID() == ID_MONSTER)
			{
				m_kRemoveObjectContainer.insert(pkObject->GetGuid());
			}
			++itr;
		}
	}
}

void PgWorld::RemoveAllMonster(int const iGrade)
{
	if(iGrade < EMGRADE_NORMAL || EMGRADE_MONSTERMAX < iGrade)
	{
		return;
	}

	PgIWorldObject * pkObject = NULL;
	PgActor * pkActor = NULL;
	CUnit * pkUnit = NULL;
	for(int i=0;i<MAX_OBJ_CONT;++i)
	{
		ObjectContainer::const_iterator itr = m_kObjectContainer[i].begin();
		while(m_kObjectContainer[i].end() != itr)
		{
			pkObject = itr->second;
			if(pkObject->GetObjectID() == ID_MONSTER)
			{
				if(pkActor = dynamic_cast<PgActor*>(pkObject))
				{
					if(pkUnit = pkActor->GetUnit())
					{
						if(pkUnit->GetAbil(AT_GRADE) == iGrade)
						{
							m_kRemoveObjectContainer.insert(pkObject->GetGuid());
						}
					}
				}
			}
			++itr;
		}
	}
}

// eType이 PgIXmlObject::MAX_XML_OBJECT_ID라면 모든 오브젝트를 다 준다.
bool PgWorld::GetContainer(const PgIXmlObject::XmlObjectID eType, ObjectContainer &pkContainer)
{
	pkContainer.clear();

	if (eType == PgIXmlObject::MAX_XML_OBJECT_ID)
	{
		pkContainer = m_kObjectContainer[WOCID_MAIN];
		return true;
	}

	ObjectContainer::const_iterator itr = m_kObjectContainer[WOCID_MAIN].begin();
	PgIWorldObject *pkObject = NULL;
	while (m_kObjectContainer[WOCID_MAIN].end() != itr)
	{
		pkObject = itr->second;
		if (pkObject->GetObjectID() == eType)
		{
			pkContainer.insert(std::make_pair(itr->first, pkObject));
		}
		++itr;
	}

	return true;
}

PgPilot* PgWorld::addNpcToWorld(unsigned int iNpcNo, unsigned iNpcKID, BM::GUID const &rkGuid, char const* pcActorName, char const* pcScriptName, NiPoint3 const& rkPos, NiPoint3 const& rkDir, int iType, bool bHidden, int GiveEffectNo, bool bHideMiniMap)
{
	if (rkGuid == BM::GUID::NullData())
		return false;

	const wchar_t *pText = NULL;

	if(!GetDefString(iNpcNo, pText))
	{
		NILOG(PGLOG_ERROR, "[PgWorld] %s npc Can't Get NpcName NameNo[%d]", MB(rkGuid.str()), iNpcNo);
		return false;
	}

	PgNpc *pkNpc = dynamic_cast<PgNpc*>(g_kTotalObjMgr.CreateUnit(UT_NPC, rkGuid));
	PG_ASSERT_LOG(pkNpc);
	if (pkNpc == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgWorld] %s(%s) npc can't create unit[%d]", MB(pText), MB(rkGuid.str()), iNpcNo);
		return false;
	}

	POINT3 ptLoc;
	ptLoc.x = rkPos.x;
	ptLoc.y = rkPos.y;
	ptLoc.z = rkPos.z;

	if (iType != E_NPC_TYPE_DYNAMIC)
		ptLoc.z += 30;	// 바닥에 빠지지 않도록 조금 올려주자

	pkNpc->Create(rkGuid, pText, UNI(pcActorName), UNI(pcScriptName), ptLoc, iNpcKID, GiveEffectNo, static_cast<ENpcType>(iType), bHideMiniMap );	// TODO: 나중에 CreateInfo로 바꿔야 할듯.

	bool bIsNew = true;
	PgPilot* pkPilot = CreatePilot(pkNpc, bIsNew);
	if (pkPilot)
	{
		PgIWorldObject *pkWorldObject = pkPilot->GetWorldObject();
		if (!pkWorldObject)
		{
			return false;
		}

		PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
		if (!pkActor)
		{
			return false;
		}

		pkActor->LookAt(rkDir);
		AddUnit(pkNpc, pkPilot);

		if ( 0 >= pkNpc->GetAbil(AT_HP) )
		{
			bHidden = true;
		}

		pkActor->SetHide(bHidden);
		pkActor->SetHideShadow(bHidden);
		pkActor->SetHideMiniMap(bHideMiniMap);
	}

	return pkPilot;
}

void PgWorld::ProcessNpcEvent()
{
	CUnit * pkUnit = g_kPilotMan.GetPlayerUnit();
	if(!pkUnit)
	{ 
		return; 
	}

	CONT_ACTIVATE_NPC_EVENT_ACTION kContAction;
	g_kNpcEventMgr.Check(MapNo(), pkUnit, kContAction);

	PgPilot * pkNpcPilot = NULL;
	PgActor * pkNpcActor = NULL;
	CONT_ACTIVATE_NPC_EVENT_ACTION::mapped_type pkAction = NULL;
	CONT_ACTIVATE_NPC_EVENT_ACTION::const_iterator activate_it = kContAction.begin();
	while(activate_it != kContAction.end())
	{
		if(pkAction = (*activate_it).second)
		{
			CONT_NPC_EVENT_ACTION::const_iterator action_it = pkAction->begin();
			while(action_it != pkAction->end())
			{
				if( pkNpcPilot = g_kPilotMan.FindPilot( (*action_it).GetGuid() ) )
				if( pkNpcActor = dynamic_cast<PgActor*>(pkNpcPilot->GetWorldObject()) )
				{
					bool const bHidden = (*action_it).IsHidden();
					pkNpcActor->SetHide(bHidden);
					pkNpcActor->SetHideShadow(bHidden);
				}
				++action_it;
			}
		}
		
		++activate_it;
	}
}


bool PgWorld::IsHiddenNpcEvent(BM::GUID const& rkNpcGuid, CUnit * pkUnit)const
{
	if( !pkUnit )
	{
		return false;
	}

	return g_kNpcEventMgr.IsHiddenNpc(MapNo(), rkNpcGuid, pkUnit);
}
/*
bool PgWorld::AddPetToWorld(BM::GUID const& rkGuid, BM::GUID const& rkMasterGuid, NiPoint3 const& rkPos, int iID)
{
if (rkGuid == BM::GUID::NullData() || rkMasterGuid == BM::GUID::NullData())
{
return false;
}

PgPet *pkPet = dynamic_cast<PgPet*>(g_kTotalObjMgr.CreateUnit(UT_PET, rkGuid));
PG_ASSERT_LOG(pkPet);
if (!pkPet)
{
NILOG(PGLOG_ERROR, "[PgWorld] %s pet can't create unit", MB(rkGuid.str()));
return false;
}

// Get Master
PgPilot *pkMasterPilot = g_kPilotMan.FindPilot(rkMasterGuid);
if (!pkMasterPilot)
{
NILOG(PGLOG_ERROR, "[PgWorld] %s pet's master is null", MB(rkMasterGuid.str()));
return false;
}
//POINT3 ptMasterLoc = pkMasterPilot->GetUnit()->GetPos();
//NiPoint3 kMasterLoc(ptMasterLoc.x, ptMasterLoc.y, ptMasterLoc.z);

NiPoint3 kLoc;
kLoc.x = rkPos.x;
kLoc.y = rkPos.y;
kLoc.z = rkPos.z + 30;
POINT3 ptLoc;
ptLoc.x = kLoc.x;
ptLoc.y = kLoc.y;
ptLoc.z = kLoc.z;

pkPet->Create(rkGuid, rkMasterGuid, ptLoc, iID);

//pkNpc->Create(rkGuid, pText, UNI(pcActorName), UNI(pcScriptName), ptLoc, iNpcKID);	// TODO: 나중에 CreateInfo로 바꿔야 할듯.
//pkNpc->NpcType((ENpcType)iType);

bool bIsNew = true;
PgPilot* pkPilot = CreatePilot(pkPet, bIsNew);
if (pkPilot)
{
PgIWorldObject *pkWorldObject = pkPilot->GetWorldObject();
if (!pkWorldObject)
{
return false;
}

PgActor* pkActor = dynamic_cast<PgActor* >(pkWorldObject);
if (!pkActor)
{
return false;
}

AddUnit(pkPet, pkPilot);
pkActor->SetPosition(kLoc);
BM::GUID kMasterGuid = rkMasterGuid;
pkActor->SetPetMaster(kMasterGuid);
}

return true;
}
*/

void PgWorld::optimizeWorldNode()
{

}

bool PgWorld::GetCompleteBG(CompleteBGContainer& pkContainer)
{
	if (m_CompleteBGContainer.empty())
	{
		return false;
	}
	pkContainer = m_CompleteBGContainer;
	return true;
}


void PgWorld::AddDrawActorFilter(BM::GUID const& rkActorGuid)
{
	m_kDrawActorFilter.insert(rkActorGuid);
}

bool PgWorld::IsDrawActor(BM::GUID const& rkActorGuid) const
{
	ContActorFilter::const_iterator actor_iter = m_kDrawActorFilter.find(rkActorGuid);
	return (m_kDrawActorFilter.end() != actor_iter);
}

void PgWorld::ClearDrawActorFilter()
{
	m_kDrawActorFilter.clear();
}

bool PgWorld::IsMineItemBox(PgGroundItemBox *pDropItemBox)
{
	PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
	if( pDropItemBox && pkPC )
	{
		//TODO: 7초 3초 20초 룰 적용 (총합 30초룰)
		bool const bMine = pDropItemBox->IsOwner(pkPC->GetID());
		return bMine;
	}
	return false;
}

unsigned int PgWorld::GetLockBidirection()
{
	return m_eUnlockBidirection;;
}

void PgWorld::CullAlphaProcessUpdate(float fAccumTime, float fFrameTime)
{
	if(m_pkAVObjectAlphaPM)
	{
		m_pkAVObjectAlphaPM->Update(fAccumTime, fFrameTime);

		//컬링 되어야 하는 오브젝트 리스트(Alpha가 0이 된 오브젝트)
		if(!m_kAppCulledToTrueMainTainList.empty())
		{
			CullNodeContainer::iterator itor = m_kAppCulledToTrueMainTainList.begin();
			while(itor != m_kAppCulledToTrueMainTainList.end())
			{
				m_kCullContainter[(*itor)] = ALPHA_PROCESS_CULL_TRUE;
				(*itor)->SetAppCulled(true);
				++itor;
			}
		}
		m_kAppCulledToTrueMainTainList.clear();

		//컬링 되지 말아야하는 오브젝트 리스트(알파가 1이 된 오브젝트)
		if(!m_kAppCulledToFalseMainTainList.empty())
		{
			CullNodeContainer::iterator itor = m_kAppCulledToFalseMainTainList.begin();
			while(itor != m_kAppCulledToFalseMainTainList.end())
			{
				m_kCullContainter[(*itor)] = ALPHA_PROCESS_CULL_FALSE;
				(*itor)->SetAppCulled(false);
				++itor;
			}
		}
		m_kAppCulledToFalseMainTainList.clear();
	}
}

void PgWorld::AddAlphaProcessObject(NiNode* pNode, float const fEndTime, BYTE const cAlphaProcessType, BYTE const cObjectProcessType, bool bOverride)
{
	//알파가 증가하는 것은 오브젝트가 나타나는 것이다.
	if(ALPHA_MANAGER_PROCESS_TYPE_ALPHA_ADD == cAlphaProcessType)
	{
		if(ALPHA_PROCESS_CULL_TRUE == m_kCullContainter[pNode]) //3 안보이는 상태
		{
			if(m_pkAVObjectAlphaPM)
			{
				m_pkAVObjectAlphaPM->AddAVObject(pNode, fEndTime, ALPHA_MANAGER_PROCESS_TYPE_ALPHA_ADD, ALPHA_MANAGER_PROCESS_TYPE_OBJECT_MAINTAIN, true);
			}
			m_kCullContainter[pNode] = ALPHA_PROCESS_CULL_ALPHA_ADD;
		}
		else
		{
			pNode->SetAppCulled(false);
		}
	}
	//알파가 감소하는 것은 오브젝트가 사라지는 것이다.
	else
	{
		if(ALPHA_PROCESS_CULL_FALSE == m_kCullContainter[pNode]) // 보이는 상태
		{
			if(m_pkAVObjectAlphaPM)
			{
				m_pkAVObjectAlphaPM->AddAVObject(pNode, fEndTime, ALPHA_MANAGER_PROCESS_TYPE_ALPHA_SUB, ALPHA_MANAGER_PROCESS_TYPE_OBJECT_MAINTAIN, true);
			}
			m_kCullContainter[pNode] = ALPHA_PROCESS_CULL_ALPHA_SUB;
		}
		else if(ALPHA_PROCESS_CULL_ALPHA_SUB == m_kCullContainter[pNode] || ALPHA_PROCESS_CULL_ALPHA_ADD == m_kCullContainter[pNode]) // Alpha 줄어드는 상태 // Alpha 늘어나는 상태
		{
			pNode->SetAppCulled(false);
		}
		else // 안보이는 상태
		{
			pNode->SetAppCulled(true);
		}
	}

}

bool PgWorld::FindDisableRandomAniObject(const NiFixedString &strObjectName)
{
	for(StringIter itor = m_kDisableRandomAniObjNameContainer.begin(); itor != m_kDisableRandomAniObjNameContainer.end(); ++itor)
	{
		if((*itor).Equals(strObjectName))
			return true;
	}

	return false;
}

bool PgWorld::DetachObjectByName(char const* szObjectName)
{
	//DisableRandomAni 리스트 안에 존재 하면 리스트에서 제거
	for(StringIter itor = m_kDisableRandomAniObjNameContainer.begin(); itor != m_kDisableRandomAniObjNameContainer.end(); ++itor)
	{
		if((*itor).Equals(szObjectName))
		{
			m_kDisableRandomAniObjNameContainer.erase(itor);
			break;
		}
	}

	NiAVObject* pkObject = m_spSceneRoot->GetObjectByName(szObjectName);
	if(pkObject && pkObject->GetParent())
	{
		pkObject->GetParent()->DetachChild(pkObject);
		return true;
	}

	return false;
}

PgAVObjectAlphaProcessManager* PgWorld::GetAVObjectAlphaProcessManager()
{
	return m_pkAVObjectAlphaPM;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
PgPickObjectMouseTester::PgPickObjectMouseTester(NiCamera& rkCamera, NiPick& rkNewPick)
:m_kPick(rkNewPick)
{
	POINT2 ptXUIPos = XUIMgr.MousePos();// 마우스의 위치를 받는다.
	rkCamera.WindowPointToRay(ptXUIPos.x, ptXUIPos.y, m_kOrgPt, m_kRayDir);// 카메라와 마우스의 위치로 Ray 정보를 구한다.

	// 레이 객체를 초기화한다.
	m_kPick.SetCoordinateType(NiPick::WORLD_COORDINATES);
	m_kPick.SetPickType(NiPick::FIND_FIRST);
	m_kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
};

bool PgPickObjectMouseTester::Test(PgIWorldObject* pkObject)
{
	if( !pkObject )
	{
		return false;//Pass
	}

	const ObjectGroupType iType = (ObjectGroupType)pkObject->GetGroupNo();
#ifndef EXTERNAL_RELEASE
	bool const bNotPass = (OGT_PLAYER == iType) || (OGT_NPC == iType) || (OGT_PET == iType) || (OGT_GROUNDBOX == iType) || (OGT_PUPPET == iType) || (OGT_ENTITY == iType) || (OGT_SIMILAR_PLAYER == iType) || (OGT_MONSTER == iType);
#else
	bool const bNotPass = (OGT_PLAYER == iType) || (OGT_NPC == iType) || (OGT_PET == iType) || (OGT_GROUNDBOX == iType) || (OGT_PUPPET == iType) || (OGT_ENTITY == iType) || (OGT_SIMILAR_PLAYER == iType) || (OGT_MONSTER == iType);
#endif
	if( !bNotPass )//위의 오브젝트 타입중 없으면
	{
		return false;//Pass
	}

	switch(iType)
	{
	case	OGT_PET:
	case	OGT_PLAYER:
	case	OGT_MONSTER:
	case	OGT_NPC:
	case	OGT_ENTITY:
	case	OGT_SIMILAR_PLAYER:
		{
			//	Actor 일경우, 보이지 않는 다면  Pick에 걸리지 않는다.
			PgActor* pkActor = dynamic_cast<PgActor*>(pkObject);
			if(pkActor && pkActor->CanSee() == false)
			{
				return	false;
			}
		}
		break;
#ifndef EXTERNAL_RELEASE
	//엘가맵에서 검출이 되지 않아야 되는데 검출이되서 테스트하는데 방해가됨
	case	OGT_PUPPET:
		{
			if(lwKeyIsDown(NiInputKeyboard::KEY_LCONTROL,true))
			{
				return false;
			}
		}break;
#endif
	}

	return PickTest(pkObject);
}

float PgPickObjectMouseTester::Distance(PgIWorldObject const* pkObject)const
{
	return (m_kOrgPt - pkObject->GetWorldTranslate()).Length();
}

bool PgPickObjectMouseTester::PickTest(PgIWorldObject* pkObject)
{
	if( !pkObject )
	{
		return false;
	}

	// 레이를 체크하고
	// 충돌이 있으면 반환한다.
	m_kPick.SetTarget(pkObject);
	bool const bPickRet = m_kPick.PickObjects(m_kOrgPt, m_kRayDir, true);
	if( !bPickRet )
	{
		return false;//픽킹 안됨
	}
	return true;//픽킹 됨
}

//
PgPickObjectMouseTypeChecker::PgPickObjectMouseTypeChecker(NiCamera& rkCamera, NiPick& rkNewPick, ObjectGroupType iObjectGroupType)
:PgPickObjectMouseTester(rkCamera, rkNewPick)
{
	m_iObjectGroupType = iObjectGroupType;
}

bool PgPickObjectMouseTypeChecker::Test(PgIWorldObject* pkObject)
{
	if( !pkObject )
	{
		return false;//Pass
	}

	if( m_iObjectGroupType != OGT_NONE
		&&	m_iObjectGroupType != pkObject->GetGroupNo() )
	{
		return false;//Pass
	}
	return PickTest(pkObject);
}

//
PgPickObjectSpaceCheker::PgPickObjectSpaceCheker(PgIWorldObject& rkPlayer)
: m_kPlayer(rkPlayer)
{
	m_pkClosestObject = NULL;
	m_fClosestDistance = 70.f;

	m_kPlayerPos = m_kPlayer.GetTranslate();
}

bool PgPickObjectSpaceCheker::Test(PgIWorldObject* pkObject)
{
	if( !pkObject )
	{
		return false;//Pass
	}

	const ObjectGroupType iType = (ObjectGroupType)pkObject->GetGroupNo();
	bool const bPassObject = (iType != OGT_GROUNDBOX) && (iType != OGT_NPC) && (iType != OGT_ENTITY);
	if( m_kPlayer.GetGuid() == pkObject->GetGuid()
		||	bPassObject )
	{
		return false;
	}

	switch(pkObject->GetGroupNo())
	{
	case OGT_GROUNDBOX:
		{
			PgDropBox* pkDropBox = dynamic_cast<PgDropBox*>(pkObject);
			if( pkDropBox
				&&	pkDropBox->GetHide() )
			{
				//드랍박스의 경우, PgDropBox::GetHide() 가 True 일 경우는 Pick 에 걸리지 않는다.
				return false;
			}
		}break;
	case	OGT_PLAYER:
	case	OGT_MONSTER:
	case	OGT_NPC:
	case	OGT_ENTITY:
		{
			//	Actor 일경우, 보이지 않는 다면  Pick에 걸리지 않는다.
			PgActor* pkActor = dynamic_cast<PgActor*>(pkObject);
			if(pkActor && pkActor->CanSee() == false)
			{
				return	false;
			}
		}
		break;

	default:
		{

		}break;
	}

	NiPoint3 const& kTargetPos = pkObject->GetTranslate();
	float fDistance = (m_kPlayerPos - kTargetPos).Length();
	if( fDistance < m_fClosestDistance )
	{
		m_pkClosestObject = pkObject;
		m_fClosestDistance = fDistance;
	}

	return false;//일단 무조건 false
}

void PgWorld::ShineStoneCountContReset()
{
	m_kShineStoneCountContainer.clear();
}

void PgWorld::CountShineStone(DWORD stoneID)
{
	ShineStoneCountCont::iterator	iter = m_kShineStoneCountContainer.find(stoneID);
	if(iter != m_kShineStoneCountContainer.end())
	{
		iter->second += 1;
	}
	else
	{
		m_kShineStoneCountContainer.insert(std::make_pair(stoneID, 1));
	}
}

int	 PgWorld::GetShineStoneCount(DWORD stoneID)
{
	ShineStoneCountCont::iterator	iter = m_kShineStoneCountContainer.find(stoneID);
	if(iter != m_kShineStoneCountContainer.end())
	{
		return iter->second;
	}

	return 0;
}

void PgWorld::OnAddScript()
{
	if(!m_kAddScript.empty())
	{
		lua_tinker::call<bool, lwWorld>(m_kAddScript.c_str(), (lwWorld)this);
	}
}

void PgWorld::OnRemoveScript()
{
	if(!m_kRemoveScript.empty())
	{
		lua_tinker::call<bool, lwWorld>(m_kRemoveScript.c_str(), (lwWorld)this);
	}

	g_kSoundMan.StopBgSound();
	g_kSoundMan.UnloadBgSound();
}

void PgWorld::OnTerminateScript()
{
	if(!m_kTerminateScript.empty())
	{
		lua_tinker::call<bool, lwWorld>(m_kTerminateScript.c_str(), (lwWorld)this);
	}
}

PgHome* PgWorld::CreateHome()
{
	if (m_pkHome)	// 이미 있으면 어떻게 할까?
		return m_pkHome;

	m_pkHome = NiNew PgHome(this);
	InitializeHome();

	return m_pkHome;
}

void PgWorld::InitializeHome()
{
	if (m_pkHome)
	{
		m_pkHome->Initialize();
		if (GetDynamicNodeRoot())
		{
			RunObjectGroupFunc(OGT_FURNITURE, WorldObjectGroupsUtil::AttachChild(m_pkHome->GetHomeRoot(), true));
		}
		//m_pkHome->SetLight();
		//m_pkHome->SetRoomSize(POINT3I(240, 240, 240));
		//m_pkHome->BuildWalls();
	}
}

void PgWorld::ReleaseHome()
{
	if (m_pkHome)
	{
		if(GetDynamicNodeRoot())
		{
			RunObjectGroupFunc(OGT_FURNITURE, WorldObjectGroupsUtil::DetachChild(m_pkHome->GetHomeRoot()));
		}
		m_pkHome->Terminate();
		m_pkHome = NULL;
	}
}

bool PgWorld::LightHillLamp( size_t const iIndex )
{
	bool bRet = false;
	ContPvPLamp::iterator lamp_itr;
	for ( lamp_itr=m_kContHillLamp.begin(); lamp_itr!=m_kContHillLamp.end(); ++lamp_itr )
	{
		if ( lamp_itr->first == iIndex )
		{
			lamp_itr->second->SetAppCulled(false);
			bRet = true;
		}
		else
		{
			lamp_itr->second->SetAppCulled(true);
		}
	}
	return bRet;
}

//NiNode*	PgWorld::GetObjectGroup(ObjectGroupType kType)
//{
//	if(kType >=OGT_MAX || kType<0)
//	{
//		return	NULL;
//	}
//	return m_aspObjectGroups[kType];
//}

void PgWorld::SetDirectionArrow( NiAVObject* pkDestObj )
{
	if(!g_pkWorld)
	{
		return;
	}
	// DestObj가 null이면 화살표를 없앤다.
	static BM::GUID g_guidDirArrow = BM::GUID::Create();
	PgIWorldObject* pkWorldObj = g_pkWorld->FindObject(g_guidDirArrow);
	PgDirectionArrow* pkDirArrow = 0;
	if (pkWorldObj)
	{
		pkDirArrow = dynamic_cast<PgDirectionArrow*>(pkWorldObj);
	}
	else
	{
		g_guidDirArrow.Generate();
		pkDirArrow = NiNew PgDirectionArrow;
		AddObject(g_guidDirArrow, pkDirArrow, g_kPilotMan.GetPlayerActor()->GetPos(), OGT_ENTITY);
	}

	if ( pkDirArrow )
	{
		if (!pkDestObj || pkDestObj == pkDirArrow->GetDestObject())
		{
			pkDirArrow->SetAppCulled(true);
		}
		else
		{
			pkDirArrow->SetAppCulled(false);
			pkDirArrow->SetDestObject(pkDestObj);
		}
	}
}

void PgCharacterLevel::SetCharacterLevel()
{
	PgPlayer *pkMyPilot = g_kPilotMan.GetPlayerUnit();
	if(pkMyPilot)
	{
		m_nCharacterLevel = pkMyPilot->GetAbil(AT_LEVEL);
	}
}
void PgCharacterLevel::SetCharacterLevel(int nLevel)
{
	m_nCharacterLevel = nLevel; 
}

TBL_DEF_MAP const* PgWorld::GetMapData(int const nMapNo)
{
	const CONT_DEFMAP* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);

	CONT_DEFMAP::const_iterator map_itor = pContDefMap->find(nMapNo);

	if(map_itor != pContDefMap->end())
	{
		return &(*map_itor).second;
	}

	return NULL;
}

bool PgWorld::BuildingControl( CONT_BUILDING::key_type nBuildNo, SEmporiaFunction const &kFunc, bool const bBuild, bool const bMessage )
{
	CONT_BUILDING::iterator building_itr = m_kContBuilding.find( nBuildNo );
	if ( building_itr!=m_kContBuilding.end() )
	{
		if ( bBuild )
		{
			if ( building_itr->second.Construct(kFunc) )
			{
				if ( bMessage )
				{
					int const iTTWNo = PgBuilding::GetTTWName(nBuildNo);
					if ( iTTWNo )
					{
						std::wstring wstrMsg;
						WstringFormat( wstrMsg, MAX_PATH, TTW(72020).c_str(), TTW(iTTWNo).c_str() );
						Notice_Show( wstrMsg, EL_Normal );
					}
				}
				return true;
			}
		}
		else
		{
			if ( building_itr->second.Destroy() )
			{
				if ( bMessage )
				{
					int const iTTWNo = PgBuilding::GetTTWName(nBuildNo);
					if ( iTTWNo )
					{
						std::wstring wstrMsg;
						WstringFormat( wstrMsg, MAX_PATH, TTW(72021).c_str(), TTW(iTTWNo).c_str() );
						Notice_Show( wstrMsg, EL_Normal );
					}
				}
				return true;
			}
		}
	}
	return false;
}

void PgWorld::Recv_PT_M_C_UNIT_POS_CHANGE( BM::Stream &kPacket )
{
	BYTE byType = MMET_None;
	BM::GUID kCharGuid;
	POINT3 pt3NewPos;
	kPacket.Pop( byType );
	kPacket.Pop( kCharGuid );
	kPacket.Pop( pt3NewPos );

	pt3NewPos.z += PG_CHARACTER_Z_ADJUST;
	bool setPositionResult = false;

	PgPilot *pkPilot = g_kPilotMan.FindPilot(kCharGuid);
	if(pkPilot)
	{
		PG_ASSERT_LOG(pkPilot->GetUnit());
		PG_ASSERT_LOG(pkPilot->GetWorldObject());

		PgActor* pkActor = dynamic_cast<PgActor* >(pkPilot->GetWorldObject());
		PG_ASSERT_LOG(pkActor);

		if (!pkActor)
		{
			return;
		}

		// 강제 이동 할 수 있는지 알아보고 이동을 시도.
		if (pkActor->GetForceSync())
		{
			pkPilot->GetUnit()->SetPos(pt3NewPos);
			NiPoint3 kPrevPos = pkActor->GetPosition();
			setPositionResult = pkActor->SetPosition( NiPoint3(pt3NewPos.x,pt3NewPos.y,pt3NewPos.z) );
			pkActor->ResetLastFloorPos();
			pkActor->SetMeetFloor(false);

			NILOG(PGLOG_LOG, "[PgWorld] Unit(%s)PosChange(%d)(%f,%f,%f) before(%f,%f,%f)\n", MB(kCharGuid.str()), setPositionResult, pt3NewPos.x, pt3NewPos.y, pt3NewPos.z - PG_CHARACTER_Z_ADJUST, kPrevPos.x, kPrevPos.y, kPrevPos.z);

			if (	pkActor->IsMyActor()
				&&	(m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW || m_kCameraMan.GetCameraModeE() == PgCameraMan::CMODE_FOLLOW_DEPTH_PATH)
				)
			{
				PgCameraModeFollow* cameraModeFollow = dynamic_cast<PgCameraModeFollow*>(m_kCameraMan.GetCameraMode());
				if (cameraModeFollow)
				{
					cameraModeFollow->RefreshCameraPosition();
				}

				g_kMapMoveCompleteEventMgr.Push(kCharGuid, byType);	// 이벤트 추가 하고
				g_kMapMoveCompleteEventMgr.Pop(kCharGuid);
			}
		}
	}
}

PgIWorldObject* PgWorld::PickObject(PgPickObjectNullChecker& kTester)
{
	// 오브젝트 전체를 돌면서
	float fMaxDistance = std::numeric_limits<float>::max();
	ObjectContainer::mapped_type pkElement = NULL;
	ObjectContainer::iterator iter = m_kObjectContainer[WOCID_MAIN].begin();
	for(; m_kObjectContainer[WOCID_MAIN].end() != iter; ++iter)
	{
		ObjectContainer::mapped_type& rkElement = (*iter).second;
		if( !rkElement )
		{
			continue;
		}

		// 그룹 번호가 None 이거나.. 인자로 넘어온 그룹넘버랑 같아야만 Pick을 한다.
		bool bTestRet = kTester.Test(rkElement);
		if(bTestRet && OGT_PUPPET == rkElement->GetGroupNo())
		{
			bTestRet = (false == rkElement->GetPickupScript().empty());
		}
#ifndef EXTERNAL_RELEASE
		if(bTestRet && lwKeyIsDown(NiInputKeyboard::KEY_LCONTROL,true))
		{
			lwGUID kGuid = lua_tinker::call<lwGUID>("GetPickupActorGuid");
			if(rkElement->GetGuid() == kGuid())
			{
				bTestRet = false;
			}
		}
#endif
		if( bTestRet )
		{
			float const fDistance = kTester.Distance(rkElement);
			if(fMaxDistance > fDistance && fDistance >= 0)
			{
				fMaxDistance = fDistance;
				pkElement = rkElement;
			}
		}
	}
	return pkElement;
}

void	PgWorld::CreateEnvironmentSet()
{
	g_kEnvironmentMan.ReleaseAllEnvStateSet();

	m_iEnvironmentStateSetID = g_kEnvironmentMan.AddEnvStateSet();
	PgEnvStateSet	*pkStateSet = g_kEnvironmentMan.GetEnvStateSet(m_iEnvironmentStateSetID);
	pkStateSet->AddEnvElement(g_kEnvElementFactory.CreateObject(PgEnvElement::EET_SNOW));
	pkStateSet->AddEnvElement(g_kEnvElementFactory.CreateObject(PgEnvElement::EET_SAKURA));
	pkStateSet->AddEnvElement(g_kEnvElementFactory.CreateObject(PgEnvElement::EET_SUNFLOWER));
	pkStateSet->AddEnvElement(g_kEnvElementFactory.CreateObject(PgEnvElement::EET_ICECREAM));

	g_kEnvironmentMan.SetEnvElementValue(m_iEnvironmentStateSetID,PgEnvElement::EET_SNOW,0,0,0);
}

bool	PgWorld::GetGsaPath(TiXmlNode const* pkNode,std::string &kGsaPath)
{
	int const iType = pkNode->Type();

	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);

			std::string kTagName(pkElement->Value());
			if(kTagName == "CONDITION")
			{
				std::string	kType;
				std::string	kText(pkElement->GetText());

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					std::string kAttrName(pkAttr->Name());
					std::string kAttrValue(pkAttr->Value());

					if(kAttrName == "TYPE" || kAttrName == "ID")
					{
						kType = kAttrValue;
					}
					
					pkAttr = pkAttr->Next();
				}

				if(kType == "DEFAULT")
				{
					kGsaPath = kText;
				}
				else if(kType == "TIME")
				{
					SYSTEMTIME	kNow;
					g_kEventView.GetLocalTime(&kNow);
#ifndef EXTERNAL_RELEASE
					if(g_pkApp->IsSingleMode())
					{
						::GetSystemTime(&kNow);
					}
#endif

					PgWorldTimeCondition	kTimeCondition;
					kTimeCondition.SetConditionAttributeName("FROM","TO");
					kTimeCondition.ReadFromXmlElement(pkElement);

					if( true == kTimeCondition.CheckTimeIsInsideDuration(kNow) )
					{
						kGsaPath = kText;
						return	true;
					}

				}
				else	//DEFAULT도 TIME도 아니면 ID라고 가정하자
				{
					if(m_TimeEventLoader.GetID()==kType)
					{
						kGsaPath = kText;
						return	true;
					}
				}
				
			}

		}
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(GetGsaPath(pkNextNode,kGsaPath))
		{
			return true;
		}
	}

	return	false;
}

bool PgWorld::CheckCamTrgCondition(PgTrigger* pkTrg)
{//touch 아닌값 알림
	if( !pkTrg ) 
	{
		return false;
	}

	if( PgTrigger::CT_TOUCH != pkTrg->GetConditionType() )
	{
		_PgMessageBox("Camera Trigger Parsing Error","%s is Not \'touch\' CONDITION TYPE",pkTrg->GetID().c_str() );
		return false;
	}
	return true;
}

std::string PgWorld::CheckCamTrgType(PgTrigger* pkTrg)
{//CAMERA_TYPE 이상한 값 알림

	if( !pkTrg ) 
	{
		return false;
	}

	char const* const pcCamType = pkTrg->GetParamFromParamMap(CAMTRG_TYPE_NAME.c_str());
	if( NULL == pcCamType ) 
	{
		//_PgMessageBox("Camera Trigger Parsing Error","%s none exist CAMERA_TYPE", pkTrg->GetID().c_str() );
		return std::string();
	}
	return pcCamType;
}

//bool PgWorld::CheckCamTrgMark(PgTrigger* pkTrg)
//{//MARK 이상한값 알림
//	
//	if( !pkTrg ) 
//	{
//		return false;
//	}
//
//	std::string kMark = pkTrg->GetID();
//	std::transform( kMark.begin(), kMark.end(), kMark.begin(), toupper );
//
//	std::string::size_type index = kMark.rfind(CAMTRG_IN_MARK);
//	if( std::string::npos == index )
//	{// _IN MARK가 아닐때
//
//		std::string::size_type index = kMark.rfind(CAMTRG_OUT_MARK);
//		if( std::string::npos == index )
//		{// _IN, _OUT MARK 모두 아닌 이상한 값 일때
//
//			_PgMessageBox("Camera Trigger Parsing Error","%s is Not suitable ID", kMark.c_str() );		
//			return false;
//		}
//
//		if( CAMTRG_IN_TYPE == CheckCamTrgType(pkTrg) )
//		{// _OUT MARK 인데 IN TYPE일경우 
//
//			_PgMessageBox("Camera Trigger Parsing Error","%s Not match MARK and CAMERA_TYPE",pkTrg->GetID().c_str() );
//			return false;
//		}
//	}
//	else if( CAMTRG_OUT_TYPE == CheckCamTrgType(pkTrg) )
//	{// _IN MARK 인데 OUT TYPE일경우 
//
//		_PgMessageBox("Camera Trigger Parsing Error","%s Not match MARK and CAMERA_TYPE",pkTrg->GetID().c_str() );
//		return false;
//	}
//	return true;
//}

void PgWorld::CheckCamTrg()
{
#ifndef USE_INB
#ifndef EXTERNAL_RELEASE

	//typedef std::vector<PgTrigger*> CONT_CAMTRG;
	//CONT_CAMTRG kContCamTrg;
	//
	//{
	//	TriggerContainer::const_iterator itor;
	//	for( itor=m_kTriggerContainer.begin(); itor != m_kTriggerContainer.end(); ++itor)
	//	{// 전체 트리거 중, 카메라 타입 트리거만 간추려 낸다.
	//		PgTrigger::TriggerType const& eTrgType = (*itor).second->GetTriggerType();
	//		if( eTrgType != PgTrigger::TRIGGER_TYPE_CAMERA ) { continue; }
	//		if( !CheckCamTrgCondition( (*itor).second ) )	 { continue; }
	//		//if( !CheckCamTrgMark( (*itor).second ) )		 { continue; }
	//		kContCamTrg.push_back( (*itor).second );
	//	}
	//}
	//	
	//CONT_CAMTRG::iterator itor;

	//for(itor = kContCamTrg.begin(); itor != kContCamTrg.end(); ++itor)
	//{
	//	if( NULL == (*itor) ) 	{ continue;	}
	//	std::string kMark1 = (*itor)->GetID();
	//	std::transform( kMark1.begin(), kMark1.end(), kMark1.begin(), toupper );
	//	{
	//		std::string::size_type index1 = kMark1.rfind(CAMTRG_IN_MARK);
	//		if( std::string::npos == index1 )
	//		{
	//			index1 = kMark1.rfind(CAMTRG_OUT_MARK);
	//		}
	//		kMark1.erase(index1, kMark1.size());
	//	}	

	//	char const* const pcCamType1 = (*itor)->GetParamFromParamMap(CAMTRG_TYPE_NAME.c_str());
	//	std::string kCamType1(pcCamType1);
	//	CONT_CAMTRG::iterator itor2;

	//	for(itor2= itor+1; itor2 != kContCamTrg.end(); ++itor2)
	//	{
	//		if( NULL == (*itor2) ) 	{ continue;	}
	//		std::string kMark2 = (*itor2)->GetID();
	//		std::transform( kMark2.begin(), kMark2.end(), kMark2.begin(), toupper );
	//		{				
	//			std::string::size_type index2 = kMark2.rfind(CAMTRG_IN_MARK);
	//			if( std::string::npos == index2 )
	//			{
	//				index2 = kMark2.rfind(CAMTRG_OUT_MARK);
	//			}
	//			kMark2.erase(index2, kMark2.size());
	//		}			
	//		
	//		std::string const kReusltMark1(kMark1);
	//		std::string const kReusltMark2(kMark2);
	//		if( kReusltMark1 == kReusltMark2 )
	//		{// MARK 가 pair 하고
	//			
	//			char const* const pcCamType2 = (*itor2)->GetParamFromParamMap(CAMTRG_TYPE_NAME.c_str());
	//			std::string kCamType2(pcCamType2);
	//			if( kCamType1 == kCamType2 )
	//			{// TYPE이 같으면 오류
	//				_PgMessageBox("Camera Trigger Parsing Error","%s and %s has same CAMERA_TYPE",(*itor)->GetID().c_str(),(*itor2)->GetID().c_str() );
	//			}
	//			// 정상적이든, TYPE가 같아 오류든 확인 했으니 제거함
	//			(*itor) = NULL;
	//			(*itor2) = NULL;
	//			break;
	//		}			
	//	}
	//}
	//for(itor = kContCamTrg.begin(); itor != kContCamTrg.end(); ++itor)
	//{// 최종적으로 pair를 이루지 않는 것들만 남으므로, 알림
	//	if( NULL == (*itor) ) 	{ continue;	}
	//	_PgMessageBox("Camera Trigger Parsing Error","Can't find %s 's pair ID", (*itor)->GetID().c_str() );
	//}

#endif
#endif
}

void PgWorld::AddEffectRemainTime(BM::GUID const& rkTargetGuid, int const iNum, unsigned long const ulEndTime, CEffectDef const* pkEffectDef)
{
	if(0>=iNum || 0>=ulEndTime)
	{
		return;
	}

	CEffectDef const* pkEffectDefNew = NULL;

	if(NULL==pkEffectDef)
	{
		GET_DEF(CEffectDefMgr, kEffectDefMgr);
		pkEffectDefNew = kEffectDefMgr.GetDef(iNum);
	}
	else
	{
		pkEffectDefNew = pkEffectDef;
	}

	if(!pkEffectDefNew 
		|| (pkEffectDef && 0==pkEffectDef->GetAbil(AT_SHOW_EFFECT_TIME) )
		)//특정 어빌이 없으면
	{
		return;
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iNum);
	if (!pSkillDef)
	{
		return;
	}

	BM::CAutoMutex kLock(g_kWorldLock, true);

	XUI::CXUI_List *pkWnd = dynamic_cast<XUI::CXUI_List*>(XUIMgr.Activate(EFFECT_REMAIN_TIME_UI));
	if(!pkWnd)
	{
		return;
	}

	if(m_kContEffectRemainTime.empty())
	{
		pkWnd->DeleteAllItem();
	}

	wchar_t const* pName = NULL;
	if(false==GetDefString(pSkillDef->NameNo(),pName))
	{
		return;
	}
	XUI::SListItem* pkItem = pkWnd->AddItem(pName);
	if(pkItem && pkItem->m_pWnd)
	{
		SEffectRemainTime kNewTime(pName, ulEndTime, pkItem);
		kNewTime.iDurationTime = pkEffectDefNew->GetDurationTime();
		kNewTime.eType = (EFFECT_BAR_BLINK_TYPE)pkEffectDefNew->GetAbil(AT_SHOW_EFFECT_TIME_ATTR);//(EFFECT_BAR_BLINK_TYPE)(BM::Rand_Index(3)+1);//임시
		if(EBT_NONE==kNewTime.eType)
		{
			kNewTime.eType = EBT_SCREEN_BLINK;
		}
		m_kContEffectRemainTime.insert(std::make_pair(ulEndTime, kNewTime));
		pkWnd->Size(POINT2(pkWnd->Size().x, pkItem->m_pWnd->Size().y*pkWnd->GetTotalItemCount()));//아이템 갯수
	}

	pkItem = pkWnd->FirstItem();
	ContEffectRemainTime::iterator time_it = m_kContEffectRemainTime.begin();
	while(m_kContEffectRemainTime.end() != time_it)
	{
		SEffectRemainTime &rkTime = (*time_it).second;
		if(pkItem)
		{
			pkItem->m_wstrKey = rkTime.szName;
			rkTime.pkListItem = pkItem;
			if(pkItem->m_pWnd)
			{
				XUI::CXUI_AniBar *pkBar = dynamic_cast<XUI::CXUI_AniBar*>(pkItem->m_pWnd->GetControl(L"BAR_TIME"));
				if(pkBar)
				{
					pkBar->Max(rkTime.iDurationTime);
				}
				pkItem->m_pWnd->SetCustomData(&rkTargetGuid, sizeof(rkTargetGuid));
			}
		}

		pkItem = pkWnd->NextItem(pkItem);

		++time_it;
	}
}

void PgWorld::UpdateEffectRemainTime()
{
	if(m_kContEffectRemainTime.empty())
	{
		return;
	}
	BM::CAutoMutex kLock(g_kWorldLock, true);
	DWORD const dwAccumTime = g_kEventView.GetServerElapsedTime();//BM::GetTime32();
	ContEffectRemainTime::iterator time_it = m_kContEffectRemainTime.begin();
	while(m_kContEffectRemainTime.end() != time_it)
	{
		SEffectRemainTime &rkTime = (*time_it).second;
		if(!rkTime.Update(dwAccumTime))
		{
			time_it = m_kContEffectRemainTime.erase(time_it);
		}
		else
		{
			++time_it;
		}
	}

	if(m_kContEffectRemainTime.empty())
	{
		XUIMgr.Close(EFFECT_REMAIN_TIME_UI);
	}
	else
	{
		XUI::CXUI_List *pkWnd = dynamic_cast<XUI::CXUI_List*>(XUIMgr.Get(EFFECT_REMAIN_TIME_UI));
		if(pkWnd)
		{
			SListItem const* pkItem = pkWnd->FirstItem();
			if(pkItem && pkItem->m_pWnd)
			{
				pkWnd->Size(POINT2(pkWnd->Size().x, pkItem->m_pWnd->Size().y*pkWnd->GetTotalItemCount()));//아이템 갯수만큼 크기 조절
			}
			int iCount = 0;
			if(pkWnd->GetCustomData(&iCount, sizeof(iCount)))
			{
				if(0<iCount && 1000<=(dwAccumTime - m_dwSaveBlinkTime))
				{
					m_dwSaveBlinkTime = dwAccumTime;
					SetShowWorldFocusFilter(0xff0000, 0.2f, 0.0f, 0.4f, true );
					--iCount;
					if(0<=iCount)
					{
						pkWnd->SetCustomData(&iCount, sizeof(iCount));
					}
				}
			}
		}
	}
}

void PgWorld::DeleteEffectRemainTime(BM::GUID const& rkGuid)
{
	if(true==rkGuid.IsNull())
	{
		return;
	}

	XUI::CXUI_List *pkWnd = dynamic_cast<XUI::CXUI_List*>(XUIMgr.Get(EFFECT_REMAIN_TIME_UI));
	if(pkWnd)
	{
		BM::CAutoMutex kLock(g_kWorldLock, true);
		SListItem* pkItem = pkWnd->FirstItem();
		bool bDeleted = false;
		while(NULL!=pkItem)
		{
			bDeleted = false;
			if(pkItem)
			{
				if(pkItem->m_pWnd)
				{
					BM::GUID kGuid;
					if(pkItem->m_pWnd->GetCustomData(&kGuid, sizeof(kGuid)))
					{
						if(rkGuid==kGuid)
						{
							bDeleted = true;
							pkItem = pkWnd->DeleteItem(pkItem);
						}
					}
				}
			}

			if(false==bDeleted)
			{
				pkItem = pkWnd->NextItem(pkItem);
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void PgWorld::tagEffectRemainTime::Set(wchar_t const* pkName, unsigned long const ulTime, XUI::SListItem* pkItem)
{
	ulEndTime = ulTime;
	szName = pkName;
	if(pkItem)
	{
		pkListItem = pkItem;
	}
}
bool PgWorld::tagEffectRemainTime::Update(unsigned long const ulAccumTime)
{
	int iRemainTime = static_cast<int>(ulEndTime) - static_cast<int>(ulAccumTime);
	if(0 <= iRemainTime)
	{
		MakeText(iRemainTime);
	}
	else
	{
		if(pkListItem && pkListItem->m_pWnd)
		{
			XUI::CXUI_List *pkWnd = dynamic_cast<XUI::CXUI_List*>(pkListItem->m_pWnd->Parent());
			if(pkWnd)
			{
				pkWnd->DeleteItem(pkListItem);
				pkListItem = NULL;
				return false;
			}
		}
	}
	return true;
}

void PgWorld::tagEffectRemainTime::MakeText(unsigned long const ulRemainTime)
{
	BM::vstring szText(szName);//보류
	szText+=L" ";
	unsigned long ulTimeArr[3] = {0,};
	ulTimeArr[0] = ulRemainTime/60000;			//분
	ulTimeArr[1] = (ulRemainTime - ulTimeArr[0]*60000)/1000;	//초
	ulTimeArr[2] = (ulRemainTime%1000)/10;	//밀리초

	if(0>=ulTimeArr[0] && 10>ulTimeArr[1])
	{
		szText += L"{C=0xFFFF0000/}";			//빨간색
	}
	else
	{
		szText += L"{C=0xFFFFF568/}";			//노란색
	}

	for(int i = 0; i < 3; ++i)
	{
		if(10>ulTimeArr[i])
		{
			szText+=L"0";
		}
		szText+=ulTimeArr[i];
		if(2>i)
		{
			szText+=L":";
		}
	}

	if(pkListItem && pkListItem->m_pWnd)
	{
		XUI::CXUI_AniBar *pkBar = dynamic_cast<XUI::CXUI_AniBar*>(pkListItem->m_pWnd->GetControl(L"BAR_TIME"));
		if(pkBar)
		{
			pkBar->Text((std::wstring const&)szText);
			pkBar->Max(iDurationTime);
			pkBar->Now(ulRemainTime);

			if(0>=ulTimeArr[0] && 3>=ulTimeArr[1])
			{
				if(EBT_BAR_TWINKLE&eType)
				{
					pkListItem->m_pWnd->TwinkleInterTime(300);
					pkListItem->m_pWnd->TwinkleTime(4000);
					pkListItem->m_pWnd->SetTwinkle(true);
				}
				if(EBT_SCREEN_BLINK&eType)
				{
					int iBlinkCount = 0;
					pkListItem->m_pWnd->Parent()->GetCustomData(&iBlinkCount, sizeof(iBlinkCount));//한번도 설정된 적이 없으면 return이 false일 수 있다
					iBlinkCount+=4;	//세번 더 깜빡이자
					pkListItem->m_pWnd->Parent()->SetCustomData(&iBlinkCount, sizeof(iBlinkCount));	//깜빡일 횟수 저장
				}
				eType = EBT_NONE;
			}
		}
	}
}

//트리거 아이디와 길드 정보를 바탕으로 엠포리아 포탈의 길드아이콘과 
//길드명을 업데이트 하는 함수
bool PgWorld::UpdatePortalObject( std::string const& kTriggerID, SEmporiaGuildInfo const &kGuildInfo )
{	
	//트리거의 아이디로 트리거를 구해 온다.
	PgTrigger *pkTrigger = GetTriggerByID( kTriggerID );
	if ( pkTrigger )
	{
		//트리거와 연결된 엠포리아 포탈을 가져온다.
		char const* pObjectName = pkTrigger->GetParamFromParamMap( "OBJECT" );
		if ( pObjectName )
		{
			std::pair<CONT_EMPORIA_GATE_INFO::iterator, bool> kPair = m_kContEmporiaGateInfo.insert( std::make_pair( std::string(pObjectName), kGuildInfo ) );
			if ( !kPair.second )
			{
				kPair.first->second = kGuildInfo;
			}
			return true;
		}
	}
	return false;
}

void PgWorld::AllNpcUpdateQuest()
{
	// 모든 NPC 퀘스트 마크 다시 업데이트
	ObjectContainer::iterator iter = m_kObjectContainer[WOCID_MAIN].begin();
	for(; m_kObjectContainer[WOCID_MAIN].end() != iter; ++iter)
	{
		ObjectContainer::mapped_type& rkElement = (*iter).second;
		if( !rkElement )
		{
			continue;
		}

		switch( rkElement->GetGroupNo() )
		{
		//case OGT_PLAYER:
		//case OGT_MONSTER:
		case OGT_NPC:
			{
				PgActor* pkActor = dynamic_cast<PgActor*>(rkElement);
				if( pkActor )
				{
					pkActor->PopSavedQuestSimpleInfo();
				}
			}break;
		default:
			{
			}break;
		}
	}
}

void PgWorld::AllMonsterUpdateName()
{
	// 모든 몬스터 이름 다시 업데이트
	// 모든 파괴 오브젝트 퀘스트 표시 다시 업데이트
	ObjectContainer::iterator main_iter = m_kObjectContainer[WOCID_MAIN].begin();
	for(; m_kObjectContainer[WOCID_MAIN].end() != main_iter; ++main_iter)
	{
		ObjectContainer::mapped_type& rkElement = (*main_iter).second;
		if( !rkElement )
		{
			continue;
		}
		switch( rkElement->GetGroupNo() )
		{
		case OGT_MONSTER:
			{
				PgActor* pkActor = dynamic_cast< PgActor* >(rkElement);
				if( pkActor )
					//&& pkActor->CanSee() == false)
				{
					pkActor->UpdateName();
				}
			}break;
		default:
			{
			}break;
		}
	}

	ObjectContainer::iterator before_iter = m_kObjectContainer[WOCID_BEFORE].begin();
	for(; m_kObjectContainer[WOCID_BEFORE].end() != before_iter; ++before_iter)
	{
		ObjectContainer::mapped_type& rkElement = (*before_iter).second;
		if( !rkElement )
		{
			continue;
		}

		switch( rkElement->GetGroupNo() )
		{
		case OGT_OBJECT:
			{
				PgActor* pkActor = dynamic_cast< PgActor* >(rkElement);
				if( pkActor )
					//&& pkActor->CanSee() == false)
				{
					pkActor->UpdateQuestDepend();
				}
			}break;
		default:
			{
			}break;
		}
	}

}
void PgWorld::AllPlayerUpdateName()
{
	// 모든 플레이어 이름 다시 업데이트
	ObjectContainer::iterator iter = m_kObjectContainer[WOCID_MAIN].begin();
	for(; m_kObjectContainer[WOCID_MAIN].end() != iter; ++iter)
	{
		ObjectContainer::mapped_type& rkElement = (*iter).second;
		if( !rkElement )
		{
			continue;
		}

		switch( rkElement->GetGroupNo() )
		{
		case OGT_PLAYER:
			//case OGT_MONSTER:
			//case OGT_NPC:
			{
				PgActor* pkActor = dynamic_cast<PgActor*>(rkElement);
				if( pkActor )
					//&& pkActor->CanSee() == false)
				{
					pkActor->UpdateName();
					pkActor->EquipAllItem();
				}
			}break;
		default:
			{
			}break;
		}
	}
}

void PgWorld::DynamicGndAttr(EDynamicGroundAttr const eNew)
{
	EDynamicGroundAttr const eOldGndDynamicAttr = DynamicGndAttr();
	m_kDynamicGndAttr = eNew;
}

void PgWorld::OnWorldEnvironmentStatusChanged(DWORD const dwNewValue,bool bChangeImmediately)
{
	PgEnvElement::ENV_ELEMENT_TYPE eType = PgEnvElement::ENV_ELEMENT_TYPE::EET_NONE;
	float fIntensity = 0.0f;
	if( SWorldEnvironmentStatusBitFlag::Is(dwNewValue, WEST_SNOW) )
	{
		eType = PgEnvElement::ENV_ELEMENT_TYPE::EET_SNOW;
		fIntensity = SWorldEnvironmentStatusBitFlag::Is(dwNewValue, WEST_SNOW) ? 1 : 0;
	}
	else if( SWorldEnvironmentStatusBitFlag::Is(dwNewValue, WEST_SAKURA) )
	{
		eType = PgEnvElement::ENV_ELEMENT_TYPE::EET_SAKURA;
		fIntensity = SWorldEnvironmentStatusBitFlag::Is(dwNewValue, WEST_SAKURA) ? 1 : 0;
	}
	else if( SWorldEnvironmentStatusBitFlag::Is(dwNewValue, WEST_SUNFLOWER) )
	{
		eType = PgEnvElement::ENV_ELEMENT_TYPE::EET_SUNFLOWER;
		fIntensity = SWorldEnvironmentStatusBitFlag::Is(dwNewValue, WEST_SUNFLOWER) ? 1 : 0;
	}
	else if( SWorldEnvironmentStatusBitFlag::Is(dwNewValue, WEST_ICECREAM) )
	{
		eType = PgEnvElement::ENV_ELEMENT_TYPE::EET_ICECREAM;
		fIntensity = SWorldEnvironmentStatusBitFlag::Is(dwNewValue, WEST_ICECREAM) ? 1 : 0;
	}

	float const fTransitTime = bChangeImmediately ? 0 : 3;
	g_kEnvironmentMan.SetEnvElementValue(m_iEnvironmentStateSetID,eType,fIntensity,fTransitTime,GetAccumTime());
}

void PgWorld::SetDrawBG(bool const bDrawBG)
{
#ifndef EXTERNAL_RELEASE
	lua_tinker::call<bool, bool>("SetDrawBG", bDrawBG);
#else
	m_bDrawBG = bDrawBG;
#endif
}

bool PgWorld::GetDrawBG()
{
#ifndef EXTERNAL_RELEASE
	return lua_tinker::call<bool>("DrawBG");
#else
	return m_bDrawBG;
#endif

}
void PgWorld::SetDrawWorldText(bool const bDrawWorldText)
{
#ifndef EXTERNAL_RELEASE
	lua_tinker::call<bool, bool>("SetDrawWorldText", bDrawWorldText);
#else
	m_bDrawWorldText = bDrawWorldText;
#endif
}

bool PgWorld::GetDrawWorldText()
{
#ifndef EXTERNAL_RELEASE
	return lua_tinker::call<bool>("DrawWorldText");
#else
	return m_bDrawWorldText;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void PgWorld::DrawEmporiaGate( PgRenderer* pkRenderer, float fFrameTime )
{
    PROFILE_FUNC();

    if ( m_kContEmporiaGateInfo.empty() || NULL==pkRenderer || NULL==pkRenderer->GetRenderer() )
        return;

	if (NULL==m_spSceneRoot)
	{
		return;
	}

    // 엠포리아 Gate Draw

    pkRenderer->SetBackgroundColor( NiColorA(0.0f, 0.0f, 0.0f, 0.0f) );

    bool const bIsInsideFrame = pkRenderer->GetRenderer()->GetInsideFrameState();
    if ( !bIsInsideFrame )
    {
        pkRenderer->BeginFrame();
    }

	bool bDirty = false;

    CONT_EMPORIA_GATE_INFO::iterator gate_itr = m_kContEmporiaGateInfo.begin();
    while ( gate_itr != m_kContEmporiaGateInfo.end() )
    {
        CONT_EMPORIA_GATE_INFO::mapped_type const &kGuildInfo = gate_itr->second;

        // 이름으로 포탈 오브젝트를 찾는다.
        NiAVObject* pkObject = m_spSceneRoot->GetObjectByName( gate_itr->first.c_str() );
        if ( pkObject )
        {
            NiAVObject* pkGuildLogo = pkObject->GetObjectByName("GuildLogoTag");
            if ( pkGuildLogo )
            {
                NiGeometry* pkGeom = NiDynamicCast(NiGeometry, pkGuildLogo);
                if ( pkGeom )
                {
                    // 모델데이터 가져옴
                    BYTE const nEmblemIndex = ( kGuildInfo.IsEmpty() ? 111 : kGuildInfo.nEmblem );
                    NiGeometryData* pkGeomData = pkGeom->GetModelData();
                    if ( pkGeomData )
                    {
                        float const fIconWidth = PgGuildMark::GetGuildIconWidth();
                        BYTE const byUIndex = nEmblemIndex % PgGuildMark::GetGuildIconNumPerRow(); 
                        BYTE const byVIndex = nEmblemIndex / 
							std::max((BYTE)1,PgGuildMark::GetGuildIconNumPerRow());//0나누기 방지
                        NiPoint2* pkUV = pkGeomData->GetTextures();

						if (pkUV)
						{
                        // U=1.0, V=1.0
                        pkUV->x = static_cast<float>(byUIndex) * fIconWidth + fIconWidth;
                        pkUV->y = static_cast<float>(byVIndex) * fIconWidth + fIconWidth;

                        // U=1.0, V=0.0
                        (pkUV+1)->x = static_cast<float>(byUIndex) * fIconWidth + fIconWidth;
                        (pkUV+1)->y = static_cast<float>(byVIndex) * fIconWidth;

                        // U=0.0, V=1.0
                        (pkUV+2)->x = static_cast<float>(byUIndex) * fIconWidth;
                        (pkUV+2)->y = static_cast<float>(byVIndex) * fIconWidth + fIconWidth;

                        // U=0.0, V=0.0
                        (pkUV+3)->x = static_cast<float>(byUIndex) * fIconWidth;
                        (pkUV+3)->y = static_cast<float>(byVIndex) * fIconWidth;
						}

                        //툴에서 뽑아낸 Nif의 모델데이터들은 일관성이 Static으로 되어있어 변경이 되지 않는다.
                        //변경빈도에 따라 Mutable이나 Volatile로 바꿔줘야 함.
                        pkGeomData->SetConsistency( NiGeometryData::MUTABLE );

                        //변경된 내용을 적용
                        pkGeomData->MarkAsChanged( NiGeometryData::TEXTURE_MASK );
                    }
                }
            }

            NiAVObject* pkGuildName = pkObject->GetObjectByName( "GuildNameTag" );
            if ( pkGuildName )
            {
                // 길드네임 태그가 있으면
                NiGeometry* pkGeom = NiDynamicCast(NiGeometry, pkGuildName);
                if ( pkGeom )
                {
                    NewWare::Renderer::SetRenderStateTagExtraDataNumber( *pkGeom, 
                        NewWare::Renderer::Kernel::RenderStateTagExtraData::NOT_USED_RENDERSTATE );


                    //지오메트리냐...
                    //모델 데이터 검사
                    NiGeometryData* pkModelData = pkGeom->GetModelData();

                    //모델데이터에 텍스쳐좌표가 제대로 들어가있는지 검사 없으면 만들어준다.
                    if(pkModelData && (NULL == pkModelData->GetTextures()))
                    {
                        NiPoint2* pkNewTextureUV = NiNew NiPoint2[4];
                        if(pkNewTextureUV)
                        {
                            // U=1.0, V=1.0
                            pkNewTextureUV->x =1.0f;
                            pkNewTextureUV->y =1.0f;

                            // U=1.0, V=0.0
                            (pkNewTextureUV+1)->x =1.0f;
                            (pkNewTextureUV+1)->y =0.0f;

                            // U=0.0, V=1.0
                            (pkNewTextureUV+2)->x =0.0f;
                            (pkNewTextureUV+2)->y =1.0f;

                            // U=0.0, V=0.0
                            (pkNewTextureUV+3)->x =0.0f;
                            (pkNewTextureUV+3)->y =0.0f;
                        }
                        pkModelData->SetData(pkModelData->GetVertexCount(), pkModelData->GetVertices(), pkModelData->GetNormals(),
                            pkModelData->GetColors(), pkNewTextureUV, 1, NiGeometryData::NBT_METHOD_NONE);
                        //변경일관성 설정
                        pkModelData->SetConsistency(NiGeometryData::MUTABLE);
                        //변경 내용 적용
                        pkModelData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
                    }					

                    //필요한 변수들
                    int const iGuildNameTagSize = 256; //길드이름 중앙 정렬을 위한 네임태그의 사이즈
                    XUI::CXUI_Font *pkFont = g_kFontMgr.GetFont(FONT_GUILDNAME);
					if (NULL==pkFont)
					{//폰트 없음
						return;
					}
                    Pg2DString kGuildName(XUI::PgFontDef(pkFont), _T(""));
                    kGuildName.SetText(pkFont, kGuildInfo.wstrName);
                    int const iStringPos = (iGuildNameTagSize - kGuildName.GetSize().x)/2; //길드이름 중앙정렬을 위한 계산

                    NiRenderTargetGroup* pkOldTarget = 0;
                    if( pkRenderer->IsRenderTargetGroupActive() )
                    {
                        pkOldTarget = const_cast<NiRenderTargetGroup*>(pkRenderer->GetRenderer()->GetCurrentRenderTargetGroup());
                        pkRenderer->EndUsingRenderTargetGroup();
                    }

                    //텍스쳐링 프로퍼티가 있나
                    NiProperty* pkProp = pkGuildName->GetProperty(NiProperty::TEXTURING);
                    if(pkProp)
                    {
                        NiTexturingProperty* pkTexProp = NiDynamicCast(NiTexturingProperty, pkProp);
                        if(pkTexProp)
                        {
                            NiRenderedTexture* pkRenderTex = NiDynamicCast(NiRenderedTexture, pkTexProp->GetBaseTexture());
                            if(pkRenderTex)
                            {
                                NiRenderTargetGroup* pkTextTexture = NiRenderTargetGroup::Create(pkRenderTex->GetBuffer(), g_kRenderMan.GetRenderer()->GetRenderer(),false);
								if( pkTextTexture )
								{
									pkRenderer->GetRenderer()->BeginUsingRenderTargetGroup(pkTextTexture, NiRenderer::CLEAR_ALL);
									pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
									kGuildName.Draw(pkRenderer, iStringPos,0,NiColorA(1.0f, 1.0f, 1.0f, 1.0f), NiColorA(0.5f, 1.0f, 0.5f, 1.0f), true, XUI::XTF_OUTLINE);
									pkRenderer->GetRenderer()->EndUsingRenderTargetGroup();
									NiDelete pkTextTexture;
								}
                            }
                        }

                        gate_itr = m_kContEmporiaGateInfo.erase( gate_itr );//지웠다
						bDirty = true;
                    }
                    else
                    {
                        //텍스쳐 프로퍼티 만들어서 삽입
                        NiRenderedTexture* pkBuffer = NiRenderedTexture::Create(iGuildNameTagSize, 32, pkRenderer->GetRenderer());
                        NiTexturingProperty* pkTexProp = NiNew NiTexturingProperty();
                        NiTexturingProperty::Map* pkNewMap = NiNew NiTexturingProperty::Map();
                        if( pkBuffer )
						{
							if( pkNewMap && pkTexProp )
							{
								pkNewMap->SetTexture(pkBuffer);
								pkTexProp->SetBaseMap(pkNewMap);
								pkGeom->AttachProperty(pkTexProp);
								pkGeom->UpdateProperties();
							}

							NiRenderTargetGroup* pkTextTexture = NiRenderTargetGroup::Create(pkBuffer->GetBuffer(), pkRenderer->GetRenderer(),false);
							if( pkTextTexture )
							{//렌더타겟 사용시작
								pkRenderer->GetRenderer()->BeginUsingRenderTargetGroup(pkTextTexture, NiRenderer::CLEAR_ALL);
								pkRenderer->GetRenderer()->SetScreenSpaceCameraData();
								kGuildName.Draw(pkRenderer, iStringPos,0,NiColorA(1.0f, 1.0f, 1.0f, 1.0f), NiColorA(0.5f, 1.0f, 0.5f, 1.0f), true, XUI::XTF_OUTLINE);
								pkRenderer->GetRenderer()->EndUsingRenderTargetGroup();
								NiDelete pkTextTexture;
							}
						}

                        ++gate_itr;// 증가했다
						bDirty = true;
                    }

                    // 이아래에서는 더이상 gate_itr 사용 금지(kGuildInfo도 마찬가지)
                    if(pkOldTarget)
                    {
                        pkRenderer->BeginUsingRenderTargetGroup(pkOldTarget, NiRenderer::CLEAR_NONE);
                    }
                }
            }
			else
			{
				PgError1("Cannot GetObjectByName m_kContEmporiaGateInfo[%s]", gate_itr->first.c_str());
				gate_itr = m_kContEmporiaGateInfo.erase( gate_itr );
				bDirty = true;
			}
        }
		if ( false == bDirty )
		{
			++gate_itr;
		}
    }

    if ( !bIsInsideFrame )
    {
        pkRenderer->EndFrame();
        pkRenderer->DisplayFrame();
    }

    pkRenderer->SetBackgroundColor( DEF_BG_COLORA );
}

void PgWorld::DrawDebugInfo( PgRenderer* pkRenderer, float fFrameTime )
{
    PROFILE_FUNC();

#ifndef EXTERNAL_RELEASE
    if ( m_pkFogProperty && m_bUseFogAdjust )
    {
        BM::vstring str;
#ifdef PG_USE_NEW_FOG
        str += _T("FOG : Start("); str += (double)m_pkFogProperty->GetFogStart();
        str += _T(") End(");       str += (double)m_pkFogProperty->GetFogEnd();
        str += _T(") Density(");   str += (double)m_pkFogProperty->GetFogDensity();
        str += _T(") Func(");      str += m_pkFogProperty->GetFogFunction() == NiFogProperty::FOG_Z_LINEAR ? _T("Z_LINEAR") : _T("RANGE_SQ");
#else
        str += _T("FOG : Depth("); str += (double)m_pkFogProperty->GetDepth();
        str += _T(") Far(");       str += (double)g_kFrustum.m_fFar;
#endif	
        str += _T(") COLOR(");     str += (double)m_pkFogProperty->GetFogColor().r;
        str += _T(", ");           str += (double)m_pkFogProperty->GetFogColor().g;
        str += _T(", ");           str += (double)m_pkFogProperty->GetFogColor().b; 
        str += _T(") ");
        str += m_bUseFogAtSkybox ? _T("SKYBOX ") : _T("");
        str += m_bUseFogEndFar ? _T("FAR ") : _T("");

        str += _T("\n key(-,+, 7,8,9,0,/)");

        SRenderTextInfo kRenderTextInfo;

        kRenderTextInfo.dwDiffuseColor = COLOR(255, 50, 0, 100);
        kRenderTextInfo.fAlpha = 1.0f;
        kRenderTextInfo.kLoc = POINT2( 10, 50 );
        SetRect(&kRenderTextInfo.rcDrawable, 0, 0, 0, 0);
        kRenderTextInfo.wstrText = (const wchar_t*)str;
        kRenderTextInfo.wstrFontKey = FONT_TEXT;

        static XUI::CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_TEXT);
        static CXUI_2DString* p2DString = 
            (CXUI_2DString*)g_kFontMgr.CreateNew2DString(PgFontDef(pFont, kRenderTextInfo.dwDiffuseColor), kRenderTextInfo.wstrText);
        if ( p2DString )
        {
            p2DString->SetText(PgFontDef(pFont, kRenderTextInfo.dwDiffuseColor),kRenderTextInfo.wstrText);
        }

        kRenderTextInfo.m_p2DString = p2DString;

        g_kUIScene.RenderText(kRenderTextInfo);
    }

    if ( m_bShowActorInfo )
    {
        int iCount[PgActor::MAX_NUM_INVISIBLE] = { 0, };
        int iTotal = 0;


        for(ObjectContainer::iterator itr = m_kObjectContainer[WOCID_MAIN].begin();
            itr != m_kObjectContainer[WOCID_MAIN].end();
            ++itr)
        {
            if (NiIsKindOf(PgActor, itr->second))
            {
                PgActor* actor = (PgActor*)itr->second;
                iCount[actor->GetInvisibleGrade()]++;
                ++iTotal;
            }
        }


        BM::vstring str;
        str += _T("Total(");    str += (int)m_kObjectContainer[WOCID_MAIN].size();
        str += _T(") Actors("); str += iTotal;

        for (int i = 0; i < PgActor::MAX_NUM_INVISIBLE; ++i)
        {
            str += _T(") G"); str += i;
            str += _T("(");   str += iCount[i];
        }
        str += _T(") Time("); str += (double)m_fObjectUpdateTime; str += _T(")");

        SRenderTextInfo kRenderTextInfo;

        kRenderTextInfo.dwDiffuseColor = COLOR(255, 50, 0, 100);
        kRenderTextInfo.fAlpha = 1.0f;
        kRenderTextInfo.kLoc = POINT2( 10, 70 );
        SetRect(&kRenderTextInfo.rcDrawable, 0, 0, 0, 0);
        kRenderTextInfo.wstrText = (const wchar_t*)str;
        kRenderTextInfo.wstrFontKey = FONT_TEXT;

        static XUI::CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_TEXT);
        static CXUI_2DString* p2DString = 
            (CXUI_2DString*)g_kFontMgr.CreateNew2DString(PgFontDef(pFont, kRenderTextInfo.dwDiffuseColor), kRenderTextInfo.wstrText);
        if ( p2DString )
        {
            p2DString->SetText(PgFontDef(pFont, kRenderTextInfo.dwDiffuseColor),kRenderTextInfo.wstrText);
        }

        kRenderTextInfo.m_p2DString = p2DString;

        g_kUIScene.RenderText(kRenderTextInfo);
    }

    if ( m_bShowWorldInfo )
    {
        BM::vstring str;
        str += _T("Map Number("); str += UNI(GetID());
        str += _T(") HIGH(");     str += m_afWorldObjectLODRange[0];
        str += _T(") MID(");      str += m_afWorldObjectLODRange[1];
        str += _T(") LOW(");      str += m_afWorldObjectLODRange[2]; str += _T(")");

        SRenderTextInfo kRenderTextInfo;
        kRenderTextInfo.dwDiffuseColor = COLOR(255, 50, 0, 100);
        kRenderTextInfo.fAlpha         = 1.0f;
        kRenderTextInfo.kLoc           = POINT2( 10, 90 );
        SetRect(&kRenderTextInfo.rcDrawable, 0, 0, 0, 0);
        kRenderTextInfo.wstrText = (const wchar_t*)str;
        kRenderTextInfo.wstrFontKey = FONT_TEXT;

        static XUI::CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_TEXT);
        static CXUI_2DString* p2DString = 
            (CXUI_2DString*)g_kFontMgr.CreateNew2DString(PgFontDef(pFont, kRenderTextInfo.dwDiffuseColor), kRenderTextInfo.wstrText);
        if ( p2DString )
        {
            p2DString->SetText(PgFontDef(pFont, kRenderTextInfo.dwDiffuseColor),kRenderTextInfo.wstrText);
        }

        kRenderTextInfo.m_p2DString = p2DString;

        g_kUIScene.RenderText( kRenderTextInfo );
    }

    if ( m_bUseLODAdjust )
    {
        BM::vstring str;
        if ( m_bUseWorldObjectLOD )
        {
            str += _T(" LOD : H("); str += (int)m_afWorldObjectLODRange[0];
            str += _T(") M(");      str += (int)m_afWorldObjectLODRange[1];
            str += _T(") L(");      str += (int)m_afWorldObjectLODRange[2]; str += _T(")");
        }
        if ( m_bUseWorldQualityRange )
        {
            str += _T(" QUAL : H("); str += (int)m_afWorldQualityRange[0];
            str += _T(") M(");       str += (int)m_afWorldQualityRange[1];
            str += _T(") L(");       str += (int)m_afWorldQualityRange[2]; str += _T(")");
        }
        if ( m_bUseViewDistanceRange )
        {
            str += _T(" VIEW : H("); str += (int)m_afViewDistanceRange[0];
            str += _T(") M(");       str += (int)m_afViewDistanceRange[1];
            str += _T(") L(");       str += (int)m_afViewDistanceRange[2]; str += _T(")");
        }

        SRenderTextInfo kRenderTextInfo;

        kRenderTextInfo.dwDiffuseColor = COLOR(255, 50, 0, 100);
        kRenderTextInfo.fAlpha = 1.0f;
        kRenderTextInfo.kLoc = POINT2( 10, 120 );
        SetRect(&kRenderTextInfo.rcDrawable, 0, 0, 0, 0);
        kRenderTextInfo.wstrText = (const wchar_t*)str;
        kRenderTextInfo.wstrFontKey = FONT_TEXT;

        static XUI::CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_TEXT);
        static CXUI_2DString* p2DString = 
            (CXUI_2DString*)g_kFontMgr.CreateNew2DString(PgFontDef(pFont, kRenderTextInfo.dwDiffuseColor), kRenderTextInfo.wstrText);
        if ( p2DString )
        {
            p2DString->SetText(PgFontDef(pFont, kRenderTextInfo.dwDiffuseColor),kRenderTextInfo.wstrText);
        }

        kRenderTextInfo.m_p2DString = p2DString;

        g_kUIScene.RenderText(kRenderTextInfo);
    }

#endif // #ifndef EXTERNAL_RELEASE
}

PgWorld::PgTimeEventLoader::PgTimeEventLoader()
{
	m_TimeCondition.SetConditionAttributeName("FROM", "TO");
	m_ID.clear();
}

void PgWorld::PgTimeEventLoader::ReadFromXmlElement(TiXmlElement const *Element)
{
	if(true==m_TimeCondition.ReadFromXmlElement(Element))
	{
		SYSTEMTIME Now;
		g_kEventView.GetLocalTime(&Now);
		if(true==m_TimeCondition.CheckTimeIsInsideDuration(Now))
		{
			SetID(FindID(Element));
		}

	}
}

std::string PgWorld::PgTimeEventLoader::FindID(TiXmlElement const *Element) const
{
	TiXmlAttribute const* pkAttr = Element->FirstAttribute();
	while(pkAttr)
	{
		char const* pcAttrName = pkAttr->Name();
		char const* pcAttrValue = pkAttr->Value();

		if(0==strcmp(pcAttrName, "ID"))
		{
			return std::string(pcAttrValue);
		}

		pkAttr = pkAttr->Next();
	}

	return std::string("");
}

std::string const& PgWorld::PgTimeEventLoader::GetID() const
{
	return m_ID; 
}

void PgWorld::PgTimeEventLoader::SetID(std::string const& ID)
{
	m_ID = ID;
}

bool PgWorldNpcMgr::ParseXmlForWorldNpc( TiXmlElement const * pkElement, SWorldNpc& rkOutWorldNpc )
{
	if( !pkElement )
	{
		return false;
	}

	TiXmlAttribute const *pkAttr = pkElement->FirstAttribute();
	while(pkAttr)
	{
		char const* pcAttrName = pkAttr->Name();
		char const* pcAttrValue = pkAttr->Value();

		if (_stricmp(pcAttrName, "NAME") == 0)
		{
			rkOutWorldNpc.uiNpcNo = (unsigned int)atoi(pcAttrValue);
		}
		else if (_stricmp(pcAttrName, "ACTOR") == 0)
		{
			rkOutWorldNpc.kActorName = pcAttrValue;
		}
		else if (_stricmp(pcAttrName, "SCRIPT") == 0)
		{
			rkOutWorldNpc.kScriptName = pcAttrValue;
		}
		else if (_stricmp(pcAttrName, "HIDDEN") == 0)
		{
			rkOutWorldNpc.bHidden = (stricmp(pcAttrValue,"TRUE")==0);
		}
		else if (_stricmp(pcAttrName, "GUID") == 0)
		{
			rkOutWorldNpc.kNpcGuid.Set(pcAttrValue);
		}
		else if (_stricmp(pcAttrName, "KID") == 0)
		{
			rkOutWorldNpc.uiNpcKID = (unsigned int)atoi(pcAttrValue);
		}
		else if (_stricmp(pcAttrName, "LOCATION") == 0)
		{
			sscanf_s(pcAttrValue, "%f, %f, %f", &rkOutWorldNpc.kPos.x, &rkOutWorldNpc.kPos.y, &rkOutWorldNpc.kPos.z);
		}
		else if (_stricmp(pcAttrName, "DIRECTION") == 0)
		{
			sscanf_s(pcAttrValue, "%f, %f, %f", &rkOutWorldNpc.kDir.x, &rkOutWorldNpc.kDir.y, &rkOutWorldNpc.kDir.z);						
		}
		else if (_stricmp(pcAttrName, "TYPE") == 0)
		{
			if (_stricmp(pcAttrValue, "FIXED") == 0)
				rkOutWorldNpc.iType = E_NPC_TYPE_FIXED;
			else if (_stricmp(pcAttrValue, "DYNAMIC") == 0)
				rkOutWorldNpc.iType = E_NPC_TYPE_DYNAMIC;
			else if (_stricmp(pcAttrValue, "CREATURE") == 0)
				rkOutWorldNpc.iType = E_NPC_TYPE_CREATURE;
			else
			{
				NILOG( PGLOG_WARNING, "[PgWorld] %s NPC has unknown type(%s)\n", pcAttrName, pcAttrValue );
			}
		}
		else if (_stricmp(pcAttrName, "IGNORE_CAMERA_CULLING") == 0)
		{
			rkOutWorldNpc.bIgnoreCameraCulling = (stricmp(pcAttrValue,"TRUE")==0);
		}
		else if ( _stricmp(pcAttrName, "PARAM") == 0 )
		{
			rkOutWorldNpc.iParam = ::atoi( pcAttrValue );
		}
		else if (_stricmp(pcAttrName, "DESCRIPTION") == 0)
		{
			rkOutWorldNpc.uiDesctibID = (unsigned int)atoi(pcAttrValue);
		}
		else if ( _stricmp(pcAttrName, "HIDE_MINIMAP") == 0 )
		{
			int iHideMiniMap = atoi( pcAttrValue );
			rkOutWorldNpc.bHideMiniMap = (iHideMiniMap ? true : false);
		}
		else if ( _stricmp(pcAttrName, "MINIMAP_ICON") == 0 )
		{
			rkOutWorldNpc.iMinimapIcon = ::atoi( pcAttrValue );
		}
		else if ( _stricmp(pcAttrName, "GIVE_EFFECTNO") == 0 )
		{
			rkOutWorldNpc.GiveEffectNo = ::atoi( pcAttrValue );
		}


		pkAttr = pkAttr->Next();
	}

	ContSayItem kVec;
	TiXmlElement const *pkChild = pkElement->FirstChildElement("SAY");
	while(pkChild)
	{
		//<SAY TYPE="IDLE" TTW="" UPTIME="" DELAY="" ACTION="" MIN_LEVEL="" MAX_LEVEL="" FACE="" COMPLETE_QUEST="" NOT_COMPLETE_QUEST=""/>
		SSayItem kItem;
		if( kItem.ParseXml(pkChild) )
		{
			kVec.push_back(kItem);
		}
		//
		pkChild = pkChild->NextSiblingElement();
	}

	rkOutWorldNpc.kVec.swap(kVec);

	std::wstring const* pkString = NULL;
	if( rkOutWorldNpc.uiNpcNo
	&&	::GetDefString(rkOutWorldNpc.uiNpcNo, pkString) )
	{
		g_kQuestMan.AddNpcName(rkOutWorldNpc.kNpcGuid, *pkString);
	}

	return true;
}

void PgWorldNpcMgr::LoadFile( const char* pcFilename )
{
	if( m_kContWorldNpc.size() )	{ m_kContWorldNpc.clear(); }
	if( m_kContNpc.size() )			{ m_kContNpc.clear(); }
	TiXmlDocument kXmlDoc(pcFilename);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcFilename)))
	{
		PgError1("Parse Failed [%s]", pcFilename);
		return;
	}
	else
	{
		TiXmlNode* pkRootNode = kXmlDoc.FirstChild("TOTALNPC");
		if( pkRootNode )
		{
			TiXmlNode* pkNextNode = pkRootNode->FirstChild();
			while( pkNextNode )
			{
				std::string kOutWorldId;
				int iMapNo = 0;
				ParseAllNpcXml( pkNextNode, kOutWorldId, iMapNo);
				if( !AddWorldNpc( kOutWorldId, m_kContNpc, iMapNo ) )
				{
					NILOG( PGLOG_WARNING, "[PgWorldNpcMgr::LoadFile] duplicate World ID =[%s]\n", kOutWorldId.c_str() );
				}

				pkNextNode = pkNextNode->NextSibling();
			}
		}
	}
}

TiXmlNode const* PgWorldNpcMgr::ParseAllNpcXml( TiXmlNode const *pkNode, std::string& rkOutWorldId, int& rkOutMapNo )
{
	int const iType = pkNode->Type();

	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);

			std::string kTagName(pkElement->Value());

			if( kTagName == "LOCAL" )
			{
				TiXmlNode const* pkFindLocalNode = PgXmlLocalUtil::FindInLocal(g_kLocal, pkElement, "PgWorldNpcMgr");
				if( pkFindLocalNode )
				{
					TiXmlNode const* pkResultNode = pkFindLocalNode->FirstChildElement();
					if( pkResultNode )
					{
						do
						{
							pkResultNode = ParseAllNpcXml( pkResultNode, rkOutWorldId, rkOutMapNo );
						}
						while(pkResultNode);
					}
				}
			}
			else if(kTagName == "WORLD")
			{
				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "ID") == 0)
					{
						rkOutWorldId = pcAttrValue;
					}
					else if(strcmp(pcAttrName, "MAPNO") == 0)
					{
						rkOutMapNo = atoi(pcAttrValue);
					}

					pkAttr = pkAttr->Next();
				}
				
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					while( ( pkChildNode = ParseAllNpcXml( pkChildNode, rkOutWorldId, rkOutMapNo ) ) );
				}
			}
			else if( kTagName == "NPC" )
			{
				SWorldNpc kWorldNpc;
				ParseXmlForWorldNpc( pkElement, kWorldNpc );
				if( !AddNpc( kWorldNpc ) )
				{
					NILOG( PGLOG_WARNING, "[PgWorldNpcMgr::AddNpc] duplicate Npc KID=[%d], world[%s]\n", kWorldNpc.uiNpcKID, rkOutWorldId.c_str() );
				}
			}
			else if( kTagName == "NPC_EVENT" )
			{
				int const iMapNo = rkOutMapNo;
				g_kNpcEventMgr.Parse(pkElement, iMapNo);
			}
			else
			{
				NILOG(PGLOG_WARNING, "[PgWorldNpcMgr] unknown %s node in %s world\n", kTagName.c_str(), rkOutWorldId.c_str() );
			}
		}
	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		return pkNextNode;
	}

	return NULL;
}

bool PgWorldNpcMgr::AddNpc( const SWorldNpc& rkWorldNpc )
{
	std::pair<CONT_NPC::iterator, bool> ret = m_kContNpc.insert( std::make_pair( rkWorldNpc.kNpcGuid, rkWorldNpc ) );
	if( !ret.second )
	{
		return false;
	}

	return true;
}

bool PgWorldNpcMgr::AddWorldNpc( const std::string& kWorldId, const CONT_NPC& kContNpc, int const iMapNo )
{
	std::pair<CONT_WORLD_NPC::iterator, bool> ret = m_kContWorldNpc.insert( std::make_pair( kWorldId, kContNpc ) );
	if( !ret.second )
	{
		return false;
	}

	if(0<iMapNo && false==kContNpc.empty())
	{
		m_kContWorldNpcByMapNo.insert( std::make_pair(iMapNo, kContNpc));
	}

	m_kContNpc.clear();
	return true;
}

SWorldNpc PgWorldNpcMgr::FindNpc(BM::GUID const& rkGuid, int const iMapNo) const
{
	if(0<iMapNo)
	{
		CONT_WORLD_NPC_BY_MAPNO::const_iterator it = m_kContWorldNpcByMapNo.find(iMapNo);
		if(m_kContWorldNpcByMapNo.end()!=it)
		{
			return FindNpc((*it).second, rkGuid);
		}
	}
	else
	{
		CONT_WORLD_NPC_BY_MAPNO::const_iterator it = m_kContWorldNpcByMapNo.begin();
		while(m_kContWorldNpcByMapNo.end()!=it)
		{
			SWorldNpc npc = FindNpc((*it).second, rkGuid);
			if(0<npc.uiNpcNo)
			{
				return npc;
			}
			++it;
		}
	}
	return SWorldNpc();
}

SWorldNpc PgWorldNpcMgr::FindNpc(CONT_NPC const& rkCont, BM::GUID const& rkGuid) const
{
	CONT_NPC::const_iterator npc_it = rkCont.find(rkGuid);
	if(rkCont.end()!=npc_it)
	{
		return (*npc_it).second;
	}

	return SWorldNpc();
}

//--------------------------------------------------------------------------
// tagSayItem
//--------------------------------------------------------------------------
tagSayItem::tagSayItem()
{
	Clear();
}

bool tagSayItem::ParseXml(TiXmlElement const *pkSayNode)
{
	if( !pkSayNode )
	{
		return false;
	}

	double dUpTime = 0.0, dDelay = 0.0;

	std::string const kType = pkSayNode->Attribute("TYPE");
	pkSayNode->Attribute("TTW", &iTTW);
	pkSayNode->Attribute("UPTIME", &dUpTime);
	pkSayNode->Attribute("DELAY", &dDelay);
	char const *szActionName = pkSayNode->Attribute("ACTION");
	char const *szSoundID = pkSayNode->Attribute("SOUND");
	char const *szFaceID = pkSayNode->Attribute("FACE");
	char const *szFaceAniID = pkSayNode->Attribute("FACE_ANI");
	pkSayNode->Attribute("MIN_LEVEL", &iMinLevel);
	pkSayNode->Attribute("MAX_LEVEL", &iMaxLevel);
	char const *szCompleteQuest = pkSayNode->Attribute("COMPLETE_QUEST");
	char const *szNotCompleteQuest = pkSayNode->Attribute("NOT_COMPLETE_QUEST");
	char const *szCamPos = pkSayNode->Attribute("CAM_POS");
	if( szCamPos )
	{
		VEC_STRING kVec;
		PgStringUtil::BreakSep(std::string(szCamPos), kVec, "/");
		if( 3 == kVec.size() )
		{
			kCamPos = POINT3( PgStringUtil::SafeAtof(kVec.at(0)), PgStringUtil::SafeAtof(kVec.at(1)), PgStringUtil::SafeAtof(kVec.at(2)) );
		}
	}

	ESayActionType iType = SAT_None;
	if(kType == kSAT_Idle)
	{
		iActionType = SAT_Idle;
	}
	else if(kType == kSAT_Click)
	{
		iActionType = SAT_Click;
	}
	else if(kType == kSAT_Talk)
	{
		iActionType = SAT_Talk;
	}
	else if(kType == kSAT_Warning)
	{
		iActionType = SAT_Warning;
	}
	else if(kType == kSAT_TextDialogs)
	{
		iActionType = SAT_TextDialogs;
	}
	else
	{
		PG_ASSERT_LOG(NULL);
		return false;
	}

	fUpTime		= (float)dUpTime;
	fDelay		= (float)dDelay;
	kActionName	= (szActionName)? szActionName: "";
	kSoundID	= (szSoundID)? szSoundID: "";
	kFaceID		= (szFaceID)? szFaceID: "";
	iFaceAniID	= (szFaceAniID)? atoi(szFaceAniID): 0;

	typedef std::vector< std::string > ContString;
	if( szCompleteQuest )
	{
		std::string kStr = szCompleteQuest;
		ContString kVec;
		PgStringUtil::BreakSep(kStr, kVec, "/");
		ContString::iterator iter = kVec.begin();
		while(kVec.end() != iter)
		{
			std::back_inserter(kCompleteQuest) = PgStringUtil::SafeAtoi(*iter);
			++iter;
		}
	}
	if( szNotCompleteQuest )
	{
		std::string kStr = szNotCompleteQuest;
		ContString kVec;
		PgStringUtil::BreakSep(kStr, kVec, "/");
		ContString::iterator iter = kVec.begin();
		while(kVec.end() != iter)
		{
			std::back_inserter(kNotCompleteQuest) = PgStringUtil::SafeAtoi(*iter);
			++iter;
		}
	}
	return true;
}

void tagSayItem::Clear()
{
	iActionType = 0;
	iTTW = 0;
	fUpTime = 0;
	fDelay = 0;
	kActionName.clear();
	kSoundID.clear();
	kFaceID.clear();
	iFaceAniID = 0;
	iMinLevel = 0;
	iMaxLevel = 0;
	kCompleteQuest.clear();
	kNotCompleteQuest.clear();
	kCamPos = POINT3(0.f, 0.f, 0.f);
}

bool tagSayItem::operator == (const tagSayItem &rkRight) const
{
	bool bRet = true;
	bRet = bRet && (this->iActionType == rkRight.iActionType);
	bRet = bRet && (this->fDelay == rkRight.fDelay);
	bRet = bRet && (this->fUpTime == rkRight.fUpTime);
	bRet = bRet && (this->iTTW == rkRight.iTTW);
	bRet = bRet && (this->kActionName == rkRight.kActionName);
	bRet = bRet && (this->kSoundID == rkRight.kSoundID);
	bRet = bRet && (this->kFaceID == rkRight.kFaceID);
	bRet = bRet && (this->iFaceAniID == rkRight.iFaceAniID);
	bRet = bRet && (this->iMinLevel == rkRight.iMinLevel);
	bRet = bRet && (this->iMaxLevel == rkRight.iMaxLevel);
	return bRet;
}
void PgWorld::SetCurrentRemoteObject(SREMOTE_OBJECT const kRemoteObject)
{
	PgIWorldObject* pkObject = FindObjectByName( kRemoteObject.kTargetObjectName.c_str() );
	if( NULL == pkObject )
	{
		_PgMessageBox(GetClientName(), "Not Found Object [%s]", kRemoteObject.kTargetObjectName.c_str());
		return;
	}
	PgPuppet *pkPuppet = dynamic_cast<PgPuppet*>(pkObject);
	if( NULL == pkPuppet)
	{
		_PgMessageBox(GetClientName(), "Object[%s] is not Puppet", kRemoteObject.kTargetObjectName.c_str());
		return;
	}
	if( true == IsHaveCurrentRemoteObject(kRemoteObject.kTargetObjectName) )
	{
		return;
	}

	CONT_VEC_CURRENT_REMOTE_OBJECT::value_type kObject;
	kObject.dwInitTime = BM::GetTime32();
	kObject.dwRandomDelayTime = kObject.dwInitTime;
	kObject.kObjectInfo = kRemoteObject;
	m_kContCurrentObject.push_back(kObject);
}
bool PgWorld::IsHaveCurrentRemoteObject(std::string const kObjectName)
{
	CONT_VEC_CURRENT_REMOTE_OBJECT::const_iterator itor_Object = m_kContCurrentObject.begin();
	while( m_kContCurrentObject.end() != itor_Object )
	{
		if( 0 == strcmp((*itor_Object).kObjectInfo.kTargetObjectName.c_str(), kObjectName.c_str()) )
		{
			return true;
		}
		++itor_Object;
	}
	return false;
}
bool PgWorld::IsHaveCurrentRemoteObject()
{
	return ( 0 != m_kContCurrentObject.size() ? true : false );
}
void PgWorld::PlayRemoteObject( std::string const kObjectName, std::string const &rkAniName )
{
	if( 0==rkAniName.size() || 0==kObjectName.size() )
	{
		return;
	}
	if( NULL == g_pkWorld )
	{
		return;
	}
	PgIWorldObject* pkObject = g_pkWorld->FindObjectByName( kObjectName.c_str() );
	if( NULL == pkObject )
	{
		_PgMessageBox(GetClientName(), "Not Found Object [%s]", kObjectName.c_str());
		return;
	}
	PgPuppet *pkPuppet = dynamic_cast<PgPuppet*>(pkObject);
	if( NULL == pkPuppet)
	{
		_PgMessageBox(GetClientName(), "Object[%s] is not Puppet", kObjectName.c_str());
		return;
	}
	if( false == pkPuppet->PlaySlot( rkAniName ) )
	{
		_PgMessageBox(GetClientName(), "Can't Play Object[%s] AniName[%s]", kObjectName.c_str(), rkAniName.c_str() );
	}
}
bool PgWorld::IsPlayRemoteObject( std::string const kObjectName )
{
	if( NULL == g_pkWorld )
	{
		return false;
	}
	PgIWorldObject* pkObject = g_pkWorld->FindObjectByName( kObjectName.c_str() );
	if( NULL == pkObject )
	{
		_PgMessageBox(GetClientName(), "Not Found Object [%s]", kObjectName.c_str());
		return false;
	}
	PgPuppet *pkPuppet = dynamic_cast<PgPuppet*>(pkObject);
	if( NULL == pkPuppet)
	{
		_PgMessageBox(GetClientName(), "Object[%s] is not Puppet", kObjectName.c_str());
		return false;
	}
	bool const bAniDone = pkPuppet->IsAnimationDone();
	return (false == bAniDone);
}
void PgWorld::UpdateCurrentRemoteObjcet()
{
	DWORD const dwCurrentTime = BM::GetTime32();
	CONT_VEC_CURRENT_REMOTE_OBJECT::iterator itor_Object = m_kContCurrentObject.begin();
	while( m_kContCurrentObject.end() != itor_Object )
	{
		if( false == (*itor_Object).bIsPlay )
		{
			if( 0 == (*itor_Object).kObjectInfo.dwStartAniTime 
				|| (*itor_Object).kObjectInfo.dwStartAniTime <= (dwCurrentTime-(*itor_Object).dwInitTime))
			{//시작시간이 0이거나 시작시간이 다 됐으면 플레이 시작
				PlayRemoteObject( (*itor_Object).kObjectInfo.kTargetObjectName, (*itor_Object).kObjectInfo.kStartAniID );
				(*itor_Object).bIsPlay = true;
			}
			else if( true == (*itor_Object).kObjectInfo.bUseRandomStart 
				&& 200 <= (dwCurrentTime-(*itor_Object).dwRandomDelayTime) )
			{//랜덤 시작일 경우, 최소 0.2초마다 갱신해 주도록
				if( 30 >= BM::Rand_Index(100) )
				{//확률은 30%
					PlayRemoteObject( (*itor_Object).kObjectInfo.kTargetObjectName, (*itor_Object).kObjectInfo.kStartAniID );
					(*itor_Object).bIsPlay = true;
				}
				else
				{//실패하면 시간 갱신
					(*itor_Object).dwRandomDelayTime = dwCurrentTime;
				}
			}

		}
		else
		{
			if( false == IsPlayRemoteObject( (*itor_Object).kObjectInfo.kTargetObjectName) )
			{
				if( 0 != (*itor_Object).kObjectInfo.kNextAniID.size() )
				{
					PlayRemoteObject( (*itor_Object).kObjectInfo.kTargetObjectName, (*itor_Object).kObjectInfo.kNextAniID );
				}
				m_kContCurrentObject.erase( itor_Object );
				continue;
			}
		}
		++itor_Object;
	}
}

void PgWorld::InitDungeonProgressPosFromPartyMaster(PgPilot* pkPilot)
{ //던전 난입시 파티장과의 진행 위치를 일치되도록 초기화
	if( NULL == g_pkWorld )
	{
		return;
	}
	if(NULL == pkPilot)
	{
		return;
	}
	if(PgClientPartyUtil::IsInParty() == false)
	{ //내가 파티에 속한 경우가 아니면 동기화 할 필요가 없다
		return;
	}


	PgPilot* pkPlayerPilot = g_kPilotMan.GetPlayerPilot();
	if(NULL == pkPlayerPilot)
	{ //내 유닛이 로딩되지 않았으면 종료
		return;
	}

	BM::GUID const kMasterGuid = g_kParty.MasterGuid();
	PgPilot* pkMasterPilot = g_kPilotMan.FindPilot(kMasterGuid);
	if(NULL == pkMasterPilot)
	{ //파티장 유닛이 로딩되지 않았으면 종료
		return;
	}

	if(pkPilot->GetGuid() != pkPlayerPilot->GetGuid() && pkPilot->GetGuid() != kMasterGuid)
	{ //추가된 유닛이 나 또는 파티장이 아니면 그냥 종료
		return;
	}

	PgPlayer* pkMaster = dynamic_cast<PgPlayer*>(pkMasterPilot->GetUnit());
	if(NULL == pkMaster)
	{
		return;
	}

	//파티장과 내가 모두 로딩이 완료 되었다면...
	if(pkPlayerPilot->GetGuid() == kMasterGuid)
	{ //내가 파티장이면
		pkMaster->SetAbil(AT_PARTY_MASTER_PROGRESS_POS, g_kProgressMap.GetCurPos());
		BM::Stream kPacket(PT_C_M_NFY_PROGRESS_POS, g_kProgressMap.GetCurPos());
		NETWORK_SEND(kPacket);
	}
	else if(pkMaster->GetAbil(AT_PARTY_MASTER_PROGRESS_POS) <= 0)
	{ //파티장이 아직 입장하지 않았다면 그냥 시작점으로 초기화
		pkPlayerPilot->SetAbil(AT_PARTY_MASTER_PROGRESS_POS, g_kProgressMap.GetCurPos());
		BM::Stream kPacket(PT_C_M_NFY_PROGRESS_POS, g_kProgressMap.GetCurPos());
		NETWORK_SEND(kPacket);
	}
	else
	{ //내가 파티원이면 파티장의 진행 위치를 얻어와서 초기화
		g_kProgressMap.SetCurPosPartyMember(pkMaster->GetAbil(AT_PARTY_MASTER_PROGRESS_POS));
		g_kProgressMap.SetCurPos(pkMaster->GetAbil(AT_PARTY_MASTER_PROGRESS_POS), true);
		//CurProgress(pkMaster->GetAbil(AT_PARTY_MASTER_PROGRESS_POS));
		XUI::CXUI_Wnd* pkMiniMap = XUIMgr.Get(L"SFRM_PROGRESS_MAP");
		if(NULL != pkMiniMap)
		{
			//lwUpdateDungeonMinimap(lwUIWnd(pkMiniMap));
		}
	}
}