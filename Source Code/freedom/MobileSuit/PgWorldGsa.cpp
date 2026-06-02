
#include "stdafx.h"
#include "PgWater.h"
#include "PgHome.h"
#include "PgWorld.h"
#include "PgTrail.H"
#include "PgPuppet.h"
#include "PgNifMan.h"
#include "PgStat.h"
#include "PgMobileSuit.h"
#include "PgOption.h"
#include "PgNiEntityStreamingAscii.h"
#include "PgRenderer.H"
#include "PgNiFile.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgEventCamera.h"
#include "PgSoundMan.h"
#include "PgPSRoomGroup.H"
#include "PgPhysXUtil.H"
#include "PgRendererUtil.H"

#include "NewWare/Scene/AssetUtils.h"
#include "NewWare/Scene/ApplyTraversal.h"

char const* COMPONENT_ANIMATION_OBJECT			= "Animation Object";
char const* COMPONENT_PHASE						= "Phase";

char const* COMPONENT_HIDE_OBJECT				= "Hide Object";
char const* COMPONENT_HIDE_ALPHAVALUE			= "Alpha Value";
char const* COMPONENT_HIDE_ALPHAE_ENABLE		= "Alpha Enable";

char const *COMPONENT_GLOWMAP					= "GlowMap";
char const *COMPONENT_GLOWMAP_ENABLE			= "Enable";

char const *COMPONENT_USE_UPDATE_LOD			= "Use Update Lod";
char const *COMPONENT_USE_UPDATE_LOD_SET		= "Value";

char const *COMPONENT_HIDDEN_PORTAL				= "Hidden_Portal";
char const *COMPONENT_HIDDEN_PORTAL_LEVEL		= "Level";

char const *COMPONENT_EVENT_OBJECT				= "EventObject";
char const *COMPONENT_EVENT_OBJECT_NAME			= "ObjectName";

char const *COMPONENT_OCCLUSION_CULLING			= "OcclusionCulling";
char const *COMPONENT_OCCLUSION_CULLING_GROUP	= "GroupNumber";

char const* COMPONENT_ACTOR			= "Actor";
char const* COMPONENT_ENVSOUND		= "Sound Object";
char const* COMPONENT_SHAREDSTREAM	= "Shared Stream";

char const* COMPONENT_ACTORXML			= "Actor Xml";
char const* COMPONENT_ACTORXML_PATH			= "Actor Xml Path";

char const* COMPONENT_ONLY_PLAYER_PHYSX			= "PhysX_PlayerOnly";
char const* COMPONENT_IGNORE_PORTAL_SYSTEM			= "IgnorePortalSystem";

int const	iDefaultNodeTotal = 15;
char	const	* szDefaultNodeName[15][2] =
{
	{"Path", "paths"},
	{"PhysX", "physx"},
    {"CharacterSpawn", "char_spawns"},
	{"CameraWalls", "camera_walls"},
	{"SkyBox", "sky_boxes"},
    {"Trigger", "triggers"},
    {"Trap", "traps"},
    {"Water", "waters"},
	{"Ladder", "ladders"},
	{"Rope", "ropes"},
	{"Room", "rooms"},
	{"Optimization0", "optimize_0"},
	{"Optimization1", "optimize_1"},
	{"Optimization2", "optimize_2"},
	{"PermissionArea", "permission_area"},
};

// Declare Nif Cont
typedef struct tagNifObject
{
	tagNifObject()
	{
		pkObject = 0;
		spPhysXObject = 0;
	}
	NiAVObject* pkObject;			// Original NI Object
	NiPhysXScenePtr spPhysXObject;	// Original Size PhysX Obj
} SNifObject;
typedef std::map< std::string, SNifObject > NifObjectCont;			// Path, Nif

//#define PG_LOADGSA_OUT_TESTNIF

int SearchNode(NiNode* pkTargetNode, const NiFixedString &kName, bool bRecursive = false)
{
	int iCount = 0;
	int iChildTotal = pkTargetNode->GetArrayCount();

	for (int i=0 ; i<iChildTotal ; i++)
	{
		NiAVObject* pkObject = pkTargetNode->GetAt(i);
		if (!pkObject)
			continue;

		if (bRecursive && NiIsKindOf(NiNode, pkObject))
		{
			NiNode* pkRecursiveNode = NiDynamicCast(NiNode, pkObject);
			iCount += SearchNode(pkRecursiveNode, kName, bRecursive);
		}

		if (pkObject->GetName().Equals(kName))
		{
			iCount += 1;
		}
	}

	return iCount;
}

// Geometry를 찾는 Stack 재귀 함수, 깊이 탐색
void PgWorld::GetAllGeometries(NiNode const* pkNode, NiObjectList& kGeomerties)
{
	if (NULL == pkNode)
	{
		return;
	}

	for(unsigned int i = 0; i < pkNode->GetArrayCount(); ++i)
	{	
		NiAVObject* pkChild = pkNode->GetAt(i);
		if (!pkChild)
		{
			continue;
		}

		if(NiIsKindOf(NiNode, pkChild))
		{
			GetAllGeometries(NiDynamicCast(NiNode, pkChild), kGeomerties);
			//kQueue.AddHead(pkChild);
		}
		else if(NiIsKindOf(NiGeometry, pkChild))
		{
			kGeomerties.AddTail(pkChild);
		}
	}
}


void SetGeometryShader(NiAVObject *pkAVObject)
{
	if(NiIsKindOf(NiNode,pkAVObject))
	{

		NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
		if(pkNode)
		{
			int iChildCount = pkNode->GetArrayCount();
			for(int i=0;i<iChildCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					SetGeometryShader(pkChild);
				}
			}
		}
	}
	if(NiIsKindOf(NiGeometry,pkAVObject))
	{
		NiGeometry *pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
		if(pkGeom)
		{
			NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, 
				NiRenderer::GetRenderer());

			const NiMaterial* pkTempMat = pkGeom->GetActiveMaterial();
			pkGeom->ApplyAndSetActiveMaterial(pkRenderer->GetDefaultMaterial());
			NiD3DShaderInterface* pkShader = NiDynamicCast(NiD3DShaderInterface, 
				pkGeom->GetShaderFromMaterial());
			pkGeom->SetActiveMaterial(pkTempMat);

			pkGeom->SetShader(pkShader);
		}
	}
}
void PgWorld::SetPostFixTextureToTexture(std::string const &rkPostfixTextureName, NiTexturingProperty::Map *pkMap)
{
	if(!pkMap)
	{
		return;
	}

	NiSourceTexture* pkSrc = NiDynamicCast(NiSourceTexture, pkMap->GetTexture());
	if(!pkSrc)
	{
		return;
	}

	std::string strTexture = pkSrc->GetFilename();
	std::string checkPath;
	strTexture.insert(strTexture.rfind("."), rkPostfixTextureName);
	checkPath = PG_SET_MAPTEXTURE_DIR + strTexture;
	if (PgNiFile::CheckFileExist(checkPath.c_str()) == false)
	{
		return;
	}			
	
	NiSourceTexture* pTexture = g_kNifMan.GetTexture(strTexture);
	if(!pTexture)
	{
		return;
	}

	pkMap->SetTexture(pTexture);

}
void PgWorld::SetPostFixTextureToNode(std::string const &rkPostfixTextureName, NiNode *pkObjectNode)
{
	pkObjectNode->UpdateProperties();

	NiObjectList kGeometries;
	GetAllGeometries(pkObjectNode, kGeometries);
	while(!kGeometries.IsEmpty())
	{
		NiGeometry *pkGeo = NiDynamicCast(NiGeometry, kGeometries.GetTail());
		kGeometries.RemoveTail();
		if (!pkGeo || !pkGeo->GetPropertyState() || !pkGeo->GetPropertyState()->GetTexturing())
		{
			continue;
		}

		NiTexturingProperty* pkTextureProp = pkGeo->GetPropertyState()->GetTexturing();

		if(pkTextureProp->GetBaseMap())
		{
			SetPostFixTextureToTexture(rkPostfixTextureName,pkTextureProp->GetBaseMap());
		}
		if(pkTextureProp->GetGlowMap())
		{
			SetPostFixTextureToTexture(rkPostfixTextureName,pkTextureProp->GetGlowMap());
		}
		
	}
}


// 게임이 정상적으로 돌아갈수 있는 필요한 노드가 있는지 체크 한다.
void PgWorld::CreateDefaultNode(NodeContainer &kNodeCont,bool bFindBase)
{
	// Create default node
	// 앞에 것은 맵툴에서 정해지는 Type
	// 뒤에 것은 쓰이는 노드 이름.

	for (int i=0 ; i<iDefaultNodeTotal ; i++)
	{
		// Base에서 추가 되지 않았을 경우에만 노드 추가를 한다.
		if (szDefaultNodeName[i][1] &&
			SearchNode(m_spSceneRoot, szDefaultNodeName[i][1], false) <= 0)
		{
			NiNode* pkDefaultNode = NiNew NiNode();
			pkDefaultNode->SetName(szDefaultNodeName[i][1]);
			m_spSceneRoot->AttachChild(pkDefaultNode, true);

			kNodeCont.insert(std::make_pair(szDefaultNodeName[i][0], pkDefaultNode));
		}
	}

	NiNode	*pkStaticNodeRoot = NiNew NiNode();
	PG_ASSERT_LOG(pkStaticNodeRoot);
	pkStaticNodeRoot->SetName("STATIC_NODE_ROOT");
    m_spSceneRoot->AttachChild( pkStaticNodeRoot, true ); // m_kSceneSpacePartition 활성화때 주석처리해야 함!
	m_pkStaticNodeRoot = pkStaticNodeRoot;

	m_pkRoomGroupRoot = NiNew PgPSRoomGroup();
	m_spSceneRoot->AttachChild(m_pkRoomGroupRoot,true);

	NiNode	*pkSelectiveNodeRoot = NiNew NiNode();
	if (bFindBase)
	{
		NiNodePtr spTemp = m_spSceneRoot;
		m_spSceneRoot = pkSelectiveNodeRoot;
		pkSelectiveNodeRoot = spTemp;
		pkSelectiveNodeRoot->SetName("SELECTIVE_NODE_ROOT");
		m_spSceneRoot->AttachChild(pkSelectiveNodeRoot, true);
		m_pkSelectiveNodeRoot = pkSelectiveNodeRoot;
	}
	else
	{
		PG_ASSERT_LOG(pkSelectiveNodeRoot);
		pkSelectiveNodeRoot->SetName("SELECTIVE_NODE_ROOT");
		m_spSceneRoot->AttachChild(pkSelectiveNodeRoot, true);
		m_pkSelectiveNodeRoot = pkSelectiveNodeRoot;
	}

	pkSelectiveNodeRoot->SetSelectiveUpdate(true);
	pkSelectiveNodeRoot->SetSelectiveUpdateTransforms(true);
	pkSelectiveNodeRoot->SetSelectiveUpdatePropertyControllers(true);
	pkSelectiveNodeRoot->SetSelectiveUpdateRigid(true);

	NiNode	*pkDynamicNodeRoot = NiNew NiNode();
	PG_ASSERT_LOG(pkDynamicNodeRoot);
	pkDynamicNodeRoot->SetName("DYNAMIC_NODE_ROOT");
	m_spSceneRoot->AttachChild(pkDynamicNodeRoot, true);
	m_pkDynamicNodeRoot = pkDynamicNodeRoot;

	// GSA맵은 Physx가 하나도 없을 경우가 있기 때문에.
	// Nodecheck를 무시하기 위하여 Physx 하나를 넣어준다.
	NiNode* pkDummyParent = ((NiNode*)m_spSceneRoot->GetObjectByName("physx"));
	PG_ASSERT_LOG(pkDummyParent);
	if (pkDummyParent && pkDummyParent->GetArrayCount() <= 0)
	{
		NiAVObject *pkDummyNode = NiNew NiNode();
		pkDummyParent->AttachChild(pkDummyNode, true);
	}

	DefaultNodeSetting(m_spSceneRoot);
}

bool PgWorld::CheckRootNode(NiNode *pkRootNode)
{
	NiNode *pkTargetNode = m_spSceneRoot;

	bool bIsError = false;
	bool bIsWarning = false;
	std::wstring wstrNodeError = UNI(std::string(GetID()));
	std::wstring wstrNodeWarning = wstrNodeError;

	int iNodeTotal = 3;
	wchar_t *szNodeName[3] =
	{
		{L"paths"},
		{L"char_spawns"},
		{L"physx"},
	};

	std::wstring wstrNodeName;
	for (int i=0 ; i<iNodeTotal ; i++)
	{
		wstrNodeName = szNodeName[i];

		NiNodePtr pkCheckNode = (NiNode *)pkTargetNode->GetObjectByName(MB(wstrNodeName));
		if (!pkCheckNode)
		{
			wstrNodeError += wstrNodeName;
			wstrNodeError += _T(" wasn't exist. ");
			bIsError = true;
		}
		else
		{
			if (pkCheckNode->GetArrayCount() <= 0)
			{
				wstrNodeError += wstrNodeName;
				wstrNodeError += _T(" hasn't child node. ");
				bIsError = true;
			}
			int iSearchCount = 0;
			if ( (iSearchCount = SearchNode(pkRootNode, MB(wstrNodeName))) > 1 )
			{
				wstrNodeError += wstrNodeName;
				wstrNodeError += _T("Root has ");
				wstrNodeError += wstrNodeName;
				wstrNodeError += _T(" node more 1 : ");
				wstrNodeError += iSearchCount;
				wstrNodeError += _T(". ");
				bIsError = true;
			}
		}
	}

	if (bIsWarning)
	{
	}
	if (bIsError)
	{
		char pkNodeError[1024];
		size_t CharactersConverted = 0;
		wcstombs_s(&CharactersConverted, pkNodeError, 
			wstrNodeError.length()+1, wstrNodeError.c_str(), 
				   _TRUNCATE);
		OutputDebugString(wstrNodeError.c_str());
		NILOG(PGLOG_ERROR, pkNodeError);
#ifndef EXTERNAL_RELEASE
		NiMessageBox(pkNodeError, "NiNodeCheck Error");
#endif
	}

	return true;
}

// LoadGsa, LoadNif 에서 사용
bool PgWorld::DefaultNodeSetting(NiNode *pkRootNode)
{
	// 정보성 노드들은 숨긴다.
	m_spPathRoot = (NiNode *)pkRootNode->GetObjectByName("paths");
	if(m_spPathRoot)
	{
		m_spPathRoot->SetAppCulled(true);
	}

	m_spSpawnRoot = (NiNode *)pkRootNode->GetObjectByName("char_spawns");
	if(m_spSpawnRoot)
	{
		m_spSpawnRoot->SetAppCulled(true);
	}

	m_spPermissionRoot = (NiNode*)pkRootNode->GetObjectByName("permission_area");
	if(m_spPermissionRoot)
	{
		m_spPermissionRoot->SetAppCulled(true);
	}

	m_spSkyRoot = (NiNode *)pkRootNode->GetObjectByName("sky_boxes");
	if(m_spSkyRoot)
	{
		//m_spSkyRoot->SetAppCulled(true);
	}

	m_spPhysRoot = (NiNode *)pkRootNode->GetObjectByName("physx");
	if(m_spPhysRoot)
	{
		m_spPhysRoot->SetAppCulled(true);
	}

	m_spTriggerRoot = (NiNode *)pkRootNode->GetObjectByName("triggers");
	if(m_spTriggerRoot)
	{
		//m_spTriggerRoot->SetAppCulled(true);
	}

	//NiNodePtr	spWaterRoot = (NiNode *)pkRootNode->GetObjectByName("waters");
	//if(spWaterRoot)
	//{
	//	m_pkWater = NiNew PgWater();
	//	m_pkWater->InitFromNode(spWaterRoot);
	//}

	NiNode *pkCameraWalls = (NiNode *)pkRootNode->GetObjectByName("camera_walls");
	if(pkCameraWalls)
	{
		m_spCameraWalls = pkCameraWalls;
		m_spCameraWalls->SetAppCulled(true);
	}

	m_spOptimization_0 = (NiNode *)pkRootNode->GetObjectByName("optimize_0");
	if (m_spOptimization_0)
	{
		m_spOptimization_0->SetAppCulled(false);
	}
	m_spOptimization_1 = (NiNode *)pkRootNode->GetObjectByName("optimize_1");
	if (m_spOptimization_1)
	{
		m_spOptimization_1->SetAppCulled(false);
	}
	m_spOptimization_2 = (NiNode *)pkRootNode->GetObjectByName("optimize_2");
	if (m_spOptimization_2)
	{
		m_spOptimization_2->SetAppCulled(false);
	}

	// TODO: pkDynamicNodeRoot가 NULL이면 어쩔껀가..
	NiNode	*pkDynamicNodeRoot = GetDynamicNodeRoot();

	// Object 그룹 노드를 생성한다.
	for(int i = 0; i < OGT_MAX; i++)
	{
		m_aspObjectGroups[i] = NiNew NiNode;
		char strName[128] = {0, };
		sprintf_s(strName, "object_group_%d", i);
		m_aspObjectGroups[i]->SetName(strName);
		pkDynamicNodeRoot->AttachChild(m_aspObjectGroups[i], true);

		// 그룹 노드를 초기화한다. (todo : selective update)
		m_aspObjectGroups[i]->UpdateNodeBound();
		m_aspObjectGroups[i]->UpdateProperties();
		m_aspObjectGroups[i]->UpdateEffects();
		m_aspObjectGroups[i]->Update(0.0f);
	}

#ifndef PG_LOADGSA_OUT_TESTNIF
	// 사운드 리스너를 신그래프에 등록한다.
	NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if(pkSS && pkSS->GetHWnd())
	{
		NiMilesListener *pkListener = pkSS->GetListener();
		pkListener->SetName("sound_listener");
		RunObjectGroupFunc(OGT_EFFECT, WorldObjectGroupsUtil::AttachChild(pkListener, true));
	}
#endif

	m_spLadderRoot = (NiNode *)pkRootNode->GetObjectByName("ladders");
	//// Max에서 하이드 되어서 나오기 때문에 따로 하이드 시킬 필요가 없다.
	//if(m_spLadderRoot)
	//{
	//	m_spLadderRoot->SetAppCulled(true);
	//}

	m_spRopeRoot = (NiNode *)pkRootNode->GetObjectByName("ropes");

	RunObjectGroupFunc(OGT_EFFECT, WorldObjectGroupsUtil::AttachChild(NiNew PgTrailNodeMan, true));

	return true;
}

void PgWorld::InsertPhysXDefObject(NiPhysXSceneDesc* pkDesc, const std::string& rkName)
{
	for (unsigned int iActor =0 ; iActor <pkDesc->GetActorCount() ; iActor++)
	{
		NiPhysXActorDesc *pkActorDesc = pkDesc->GetActorAt(iActor);
		if (!pkActorDesc)
		{
			continue;
		}

		for (unsigned int iShapeDesc = 0 ;
			iShapeDesc < pkActorDesc->GetActorShapes().GetSize() ;
			iShapeDesc++)
		{
			NiPhysXShapeDesc *pkShapeDesc = pkActorDesc->GetActorShapes().GetAt(iShapeDesc);
			if (pkShapeDesc && pkShapeDesc->GetMeshDesc())
			{
				char szUnique[1024];
				sprintf_s(szUnique, "%s_%f_%d_%d", rkName.c_str(), 1.0f, iActor, iShapeDesc);
				std::string strUnique = szUnique;
				pkShapeDesc->GetMeshDesc()->SetName(strUnique.c_str());
				if (pkShapeDesc->GetType() == NX_SHAPE_MESH)
				{
					NxTriangleMeshShapeDesc kTriMeshShapeDesc;
					pkShapeDesc->GetMeshDesc()->ToTriMeshDesc(kTriMeshShapeDesc, true);
					NxTriangleMesh *pkTriMesh = kTriMeshShapeDesc.meshData;
					if(!pkTriMesh) continue;
					NxTriangleMeshDesc kTriMeshDesc;
					pkTriMesh->saveToDesc(kTriMeshDesc);
				}
			}
		}
	}
}

void PgWorld::MakePhysXScaleObject(NiPhysXSceneDesc* pkDesc, float fScale, const std::string& rkName)
{
	NiPhysXManager* pkManager = NiPhysXManager::GetPhysXManager();
	if (!pkManager)
	{
		return;
	}

	for (unsigned int iActor =0 ; iActor <pkDesc->GetActorCount() ; iActor++)
	{
		NiPhysXActorDesc *pkActorDesc = pkDesc->GetActorAt(iActor);
		if (!pkActorDesc)
		{
			continue;
		}

		// 정확한 위치 계산을 위한 Matrix.
		NxMat34 kMultMat, kMultMatInv;
		kMultMat = pkActorDesc->GetPose(0);
		kMultMat.getInverse(kMultMatInv);

		for (unsigned int iShapeDesc = 0 ;
			iShapeDesc < pkActorDesc->GetActorShapes().GetSize() ;
			iShapeDesc++)
		{
			NiPhysXShapeDesc *pkShapeDesc = pkActorDesc->GetActorShapes().GetAt(iShapeDesc);

			if (pkShapeDesc && pkShapeDesc->GetMeshDesc() &&
				(pkShapeDesc->GetType() == NX_SHAPE_CONVEX || pkShapeDesc->GetType() == NX_SHAPE_MESH))
			{
				char szUnique[1024];
				// 바뀐 스케일로 이미 만들어 졌는지 찾아보고 있으면 그걸 쓰고 아니면 1짜리를 변환해서 쓰자.
				sprintf_s(szUnique, "%s_%f_%d_%d", rkName.c_str(), fScale, iActor, iShapeDesc);
				std::string strUnique = szUnique;
				pkShapeDesc->GetMeshDesc()->SetName(strUnique.c_str());
				if (pkManager)
				{
					NiPhysXTriangleMesh* pkNewMesh = 0;
					pkNewMesh = pkManager->GetTriangleMesh(strUnique.c_str());
					if (pkNewMesh)
					{
						NxTriangleMeshShapeDesc kTriMeshShapeDesc;
						pkShapeDesc->GetMeshDesc()->ToTriMeshDesc(kTriMeshShapeDesc, true);
						continue;
					}
				}

				// 1짜리 기본을 찾아서 스케일을 바꾸어 주자.
				sprintf_s(szUnique, "%s_%f_%d_%d", rkName.c_str(), 1.0f, iActor, iShapeDesc);
				std::string strDefault = szUnique;

				NiPhysXTriangleMesh* pkOrgMesh = pkManager->GetTriangleMesh(strDefault.c_str());
				if (!pkOrgMesh)
				{
					continue;
				}

				NxTriangleMesh* pkOrgTriMesh = pkOrgMesh->GetMesh();
				NxTriangleMeshDesc kTriMeshDesc;
				pkOrgTriMesh->saveToDesc(kTriMeshDesc);
				// for Insert to hash
				unsigned short usNumNxVerts = kTriMeshDesc.numVertices;
				NxVec3* pkNxVerts = (NxVec3*)kTriMeshDesc.points;
				NxVec3* pkNxCloneVerts = NiAlloc(NxVec3, usNumNxVerts);
				for (unsigned short us = 0; us < usNumNxVerts; us++)
				{
					NxVec3 kVec = kMultMat * pkNxVerts[us];
					pkNxCloneVerts[us] = kMultMatInv * (kVec * fScale);
				}
				kTriMeshDesc.points = pkNxCloneVerts;

				// ReCook Shape!!!!
				NxInitCooking();
				NiPhysXMemStream kMemStream;
				NIVERIFY(NxCookTriangleMesh(kTriMeshDesc, kMemStream));
				pkShapeDesc->GetMeshDesc()->SetData(kMemStream.GetSize(), (unsigned char *)kMemStream.GetBuffer());
				NxTriangleMeshShapeDesc kTriMeshShapeDesc;
				pkShapeDesc->GetMeshDesc()->ToTriMeshDesc(kTriMeshShapeDesc, true);

				NxCloseCooking();
				kMemStream.Reset();
				NiFree(pkNxCloneVerts);
			}
			else if (pkShapeDesc && pkShapeDesc->GetMeshDesc())
			{
				NILOG(PGLOG_LOG, "%s isn't NX_SHAPE_MESH \n", pkShapeDesc->GetMeshDesc()->GetName());
			}
		}
	}
}

void PgWorld::SetScaleToPhysXObject(NiPhysXSceneDesc *pkDesc, float fScale, NxVec3 kOffset)
{
	float	fStartTime = NiGetCurrentTimeInSec();

	NiPhysXManager* pkManager = NiPhysXManager::GetPhysXManager();
	if (!pkManager)
	{
		return;
	}

	int iActorTotal = pkDesc->GetActorCount();
	for (int iActorCount =0 ; iActorCount <iActorTotal ; iActorCount++)
	{
		NiPhysXActorDesc *pkActorDesc = pkDesc->GetActorAt(iActorCount);
		if (pkActorDesc == NULL)
			continue;

		// 정확한 위치 계산을 위한 Matrix.
		NxMat34 kMultMat, kMultMatInv;
		kMultMat = pkActorDesc->GetPose(0);
		kMultMat.getInverse(kMultMatInv);
		//_PgOutputDebugString(" actor has t(%f, %f, %f) M(%f, %f, %f, %f, %f, %f, %f, %f, %f)\n", kMultMat.t.x,  kMultMat.t.y, kMultMat.t.z, 
		//	kMultMat.M.getRow(0).x, kMultMat.M.getRow(0).y, kMultMat.M.getRow(0).z, 
		//	kMultMat.M.getRow(1).x, kMultMat.M.getRow(1).y, kMultMat.M.getRow(1).z, 
		//	kMultMat.M.getRow(2).x, kMultMat.M.getRow(2).y, kMultMat.M.getRow(2).z);

		//_PgOutputDebugString(" actor has inverse t(%f, %f, %f) M(%f, %f, %f, %f, %f, %f, %f, %f, %f)\n", kMultMatInv.t.x,  kMultMatInv.t.y, kMultMatInv.t.z, 
		//	kMultMatInv.M.getRow(0).x, kMultMatInv.M.getRow(0).y, kMultMatInv.M.getRow(0).z, 
		//	kMultMatInv.M.getRow(1).x, kMultMatInv.M.getRow(1).y, kMultMatInv.M.getRow(1).z, 
		//	kMultMatInv.M.getRow(2).x, kMultMatInv.M.getRow(2).y, kMultMatInv.M.getRow(2).z);

		int iShapeDescTotal = pkActorDesc->GetActorShapes().GetSize();
		for (int iShapeDescCount = 0 ;
			iShapeDescCount < iShapeDescTotal ;
			iShapeDescCount++)
		{
			NiPhysXShapeDesc *pkShapeDesc =
				pkActorDesc->GetActorShapes().GetAt(iShapeDescCount);

			if (pkShapeDesc && pkShapeDesc->GetMeshDesc() && (pkShapeDesc->GetType() == NX_SHAPE_CONVEX || pkShapeDesc->GetType() == NX_SHAPE_MESH))
			{
				NxTriangleMeshShapeDesc kTriMeshShapeDesc;
				size_t kSize = 0;
				unsigned char *pucData = 0;
				pkShapeDesc->GetMeshDesc()->GetData(kSize, &pucData);
				if (!kSize)
				{
					continue;
				}
				pkShapeDesc->GetMeshDesc()->ToTriMeshDesc(kTriMeshShapeDesc, true);

				NxTriangleMesh *pkTriMesh = kTriMeshShapeDesc.meshData;
				if(!pkTriMesh)
				{
					continue;
				}

				NxTriangleMeshDesc kTriMeshDesc;
				pkTriMesh->saveToDesc(kTriMeshDesc);

				unsigned short usNumNxVerts = kTriMeshDesc.numVertices;
				//unsigned short usMaxNumTris = kTriMeshDesc.numTriangles;
				//unsigned short pointStrideBytes = kTriMeshDesc.pointStrideBytes;
				//unsigned short triangleStrideBytes = kTriMeshDesc.triangleStrideBytes;
				NxVec3* pkNxVerts = (NxVec3*)kTriMeshDesc.points;
				//NxU32* pkNxTriData = (NxU32*)kTriMeshDesc.triangles;
				NxVec3* pkNxCloneVerts = NiAlloc(NxVec3, usNumNxVerts);

				for (unsigned short us = 0; us < usNumNxVerts; us++)
				{
					NxVec3 kVec = kMultMat * pkNxVerts[us];
					pkNxCloneVerts[us] = kMultMatInv * (kVec*fScale);
					//_PgOutputDebugString(" vertex %d o(%f, %f, %f) t(%f, %f, %f) s(%f, %f, %f) c(%f, %f, %f)\n", us, 
					//	pkNxVerts[us].x,  pkNxVerts[us].y, pkNxVerts[us].z, 
					//	kVec.x, kVec.y, kVec.z,
					//	kVec.x * fScale, kVec.y * fScale, kVec.z * fScale,
					//	pkNxCloneVerts[us].x,  pkNxCloneVerts[us].y, pkNxCloneVerts[us].z);
				}
				kTriMeshDesc.points = pkNxCloneVerts;

				// ReCook Triangle
				NxInitCooking();
				NiPhysXMemStream kMemStream;
				NIVERIFY(NxCookTriangleMesh(kTriMeshDesc, kMemStream));
				NxCloseCooking();
				kMemStream.Reset();

				if(pucData)
				{
					NiFree(pucData);
				}

				std::string kClonedName((char const*)pkShapeDesc->GetMeshDesc()->GetName());
				kClonedName += "_Clonned";

				pkShapeDesc->GetMeshDesc()->SetData(kMemStream.GetSize(), (unsigned char *)kMemStream.GetBuffer());
				pkShapeDesc->GetMeshDesc()->SetName(kClonedName.c_str());

		        NiFree(pkNxCloneVerts);
			}
			else if (pkShapeDesc && pkShapeDesc->GetMeshDesc())
			{
				NILOG(PGLOG_LOG, "%s isn't NX_SHAPE_MESH \n", pkShapeDesc->GetMeshDesc()->GetName());
			}
		}
	}


#ifdef PG_PERFORMCHECK_SET_PHYSX_SCALE
	dwTotalTime += (BM::GetTime32() - dwTime);
	dwTotalCount++;
	if (dwTotalCount % 1000 == 0)
	{
		NILOG(PGLOG_LOG, "MapLoading ( %d, %d, %f )\n",	dwTotalTime, dwTotalCount, (float)dwTotalTime / (float)dwTotalCount);
	}
#endif
}

#define PG_USE_LOADGSA_BASE
bool PgWorld::LoadBaseObject(NiScene* pkScene)
{
	PG_ASSERT_LOG(pkScene);
	if (pkScene == NULL)
		return false;

	bool bFindBase = false;

	// BaseCase
	NiPoint3 kBaseTranslation = NiPoint3(0,0,0);
	NiMatrix3 kBaseRotMat;
	float fBaseScale = 1.0f;

	int iTot = pkScene->GetEntityCount();
	for (int i=0 ; i<iTot ; i++)
	{
		NiEntityInterface *pkEntity = pkScene->GetEntityAt(i);
		if (pkEntity->GetPGProperty().Equals("BaseObject"))
		{
			NiFixedString kPath;
			pkEntity->GetPropertyData("NIF File Path", kPath, 0);
			pkEntity->GetPropertyData("Translation", kBaseTranslation);
			pkEntity->GetPropertyData("Rotation", kBaseRotMat);
			pkEntity->GetPropertyData("Scale", fBaseScale);

			if (!kPath.Exists())
			{
				NILOG(PGLOG_ERROR, "[PgWorld] LoadBaseObject path is not exist\n");
				continue;
			}
			// Load Base Nif
			NiStream kStream;
			if (!kStream.Load(kPath))
			{
				NILOG(PGLOG_ERROR, "[PgWorld] Load %s gsa file faild\n", kPath);
				continue;
			}

			if (kStream.GetObjectCount() == 0)
			{
				NILOG(PGLOG_ERROR, "[PgWorld] Load %s gsa file but no object\n", kPath);
				continue;
			}

			bFindBase = true;
			m_spSceneRoot = NiDynamicCast(NiNode, kStream.GetObjectAt(0));
			unsigned int cnt = kStream.GetObjectCount();
			for (unsigned int ui = 0; ui < cnt; ui++)
			{
				NiObject *pkObject = kStream.GetObjectAt(ui);

				// BaseNode들의 Transform을 바꾸어 준다.
				if(NiIsKindOf(NiNode, pkObject))
				{
					NiNode* pkNode = (NiNode*)pkObject;
					pkNode->Update(0);
					if (pkNode)
					{
						for (unsigned int uj=0 ; uj<pkNode->GetArrayCount() ; uj++)
						{
							NiAVObject *pkAVObj = (NiAVObject*)pkNode->GetAt(uj);
							if (pkAVObj)
							{
								float fAppScale = fBaseScale * pkAVObj->GetWorldScale();
								NiMatrix3 fAppRot = kBaseRotMat * pkAVObj->GetWorldRotate();
								NiPoint3 fAppTrans = kBaseTranslation + pkAVObj->GetWorldTranslate();
								pkAVObj->SetScale(fAppScale);
								pkAVObj->SetRotate(fAppRot);
								pkAVObj->SetTranslate(fAppTrans);
							}
						}
					}
				}

				// 물리 노드를 설정한다.
				if(NiIsKindOf(NiPhysXScene, pkObject))
				{
					if (m_spPhysXScene)
					{
						m_spPhysXScene->ReleaseSnapshot();
						m_spPhysXScene->ClearSceneFromSnapshot();
						m_spPhysXScene->ReleaseScene();
						m_spPhysXScene = 0;
					}
					NiTransform kTransform;
					kTransform.m_fScale = fBaseScale;
					kTransform.m_Rotate = kBaseRotMat;
					kTransform.m_Translate = kBaseTranslation;
					m_spPhysXScene = (NiPhysXScene *)pkObject;
					m_spPhysXScene->CreateSceneFromSnapshot(0);
					m_spPhysXScene->SetSceneXform(kTransform);
				}
			}

			break;
		}
	}
	// End of Base

	return bFindBase;
}
void PgWorld::AddHideObject(NiEntityComponentInterface const * _pkCompo, NiAVObject* _pkHideObject)
{
	if(_pkHideObject)
	{
		SHideObject sHideObject;
		sHideObject.fAlphaValue = 1.0f;
		sHideObject.bAlphaEnable = false;

		if(_pkCompo->GetPropertyData(COMPONENT_HIDE_ALPHAE_ENABLE, sHideObject.bAlphaEnable))
		{
			if(sHideObject.bAlphaEnable)
			{
				if(_pkCompo->GetPropertyData(COMPONENT_HIDE_ALPHAVALUE, sHideObject.fAlphaValue) )
				{
					m_kHideObjectContainer.insert(std::make_pair(_pkHideObject, sHideObject));
				}
			}
		}
	}
}
bool PgWorld::IsHideObject(NiAVObject* _pkParentNode, float& _fAlphaValue)
{
	HideObjectContainer::iterator itr = m_kHideObjectContainer.find(_pkParentNode);
	if( itr != m_kHideObjectContainer.end() )
	{
		_fAlphaValue = itr->second.fAlphaValue	;
		return true;
	}

	_fAlphaValue = 0;
	return false;
}
void PgWorld::ClearHideObject()
{
	m_kHideObjectContainer.clear();
}
void PgWorld::RecursiveDetachGlowMapByGndAttr(NiAVObject *pkObject)
{
	bool	bIsChaosMap = false;

	if(g_pkApp->IsSingleMode())
	{
		if(!g_pkWorld)
		{
			return;
		}
		bIsChaosMap = g_pkWorld->GetSpotLightOn();
	}
	else
	{
		bIsChaosMap = IsHaveAttr(GATTR_CHAOS_F);
	}

	if(bIsChaosMap) // 카오스 맵일 경우에는 글로우맵을 무조건 킨다.
	{
		return;
	}

	if(!pkObject)
	{
		return;
	}

	if ( pkObject->GetExtraData("DETACH_GLOWMAP") != NULL )
	{
		PgRenderer::EnableGlowMap( pkObject, false );
		return;
	}

	NiNode	*pkNode = NiDynamicCast(NiNode,pkObject);
	if(pkNode)
	{
			int const	iArrayCount = pkNode->GetArrayCount();
			for(int i=0;i<iArrayCount;i++)
			{
				NiAVObject	*pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					RecursiveDetachGlowMapByGndAttr(pkChild);
				}
			}
		}
	}

void PgWorld::SetDetachGlowMap(NiEntityComponentInterface* _pkCompo, NiAVObject *_pkObject)//!/ 맵로딩시 해당 엔터티의 글로우맵을 떼어낸다.
{
	if ( NULL == _pkObject || NULL == _pkCompo )
        return;

    bool bGlowMapEnable = false;
    if ( _pkCompo->GetPropertyData(COMPONENT_GLOWMAP_ENABLE, bGlowMapEnable) == false )
    {
        bGlowMapEnable = false;
    }
    if ( false == bGlowMapEnable )
    {
        // 글로우맵 컴포넌트의 Enable 프로퍼티 항목이 없을 경우에도 글로우맵을 비활성 시킴.
        _pkObject->AddExtraData( "DETACH_GLOWMAP", NiNew NiIntegerExtraData(0) );
    }
}

void PgWorld::SetUseUpdateLodForLoadTime(NiEntityComponentInterface* _pkCompo, NiAVObject *_pkObject)
{
	if(_pkObject && _pkCompo)
	{
		int nUseUpdateLod = 0;

		if(_pkCompo->GetPropertyData(COMPONENT_USE_UPDATE_LOD_SET, nUseUpdateLod))
		{
			if(nUseUpdateLod == 0)
			{
				NiIntegerExtraData* pkData = NiNew NiIntegerExtraData(0);
				_pkObject->AddExtraData("USE_UPDATE_LOD", pkData);
			}
		}
	}
}
void PgWorld::ApplyComponentOcclusionCulling(NiEntityComponentInterface* _pkCompo, NiAVObject *_pkObject)
{
	if(_pkObject && _pkCompo)
	{
		int nOcclusionCullingGroup = 0;

		if(_pkCompo->GetPropertyData(COMPONENT_OCCLUSION_CULLING_GROUP, nOcclusionCullingGroup))
		{
			NiIntegerExtraData* pkData = NiNew NiIntegerExtraData(nOcclusionCullingGroup);
			_pkObject->AddExtraData("OCCLUSION_CULLING_GROUP", pkData);
		}
	}
}
void PgWorld::ApplyComponentHiddenPortal(NiEntityComponentInterface const * _pkCompo, NiAVObject *_pkObject)
{
	if(g_pkApp->IsSingleMode())
		return;

	if(_pkObject && _pkCompo)
	{
		if(g_kLocal.ServiceRegion() == LOCAL_MGR::NC_USA || g_kLocal.ServiceRegion() == LOCAL_MGR::NC_THAILAND || g_kLocal.ServiceRegion() == LOCAL_MGR::NC_INDONESIA || g_kLocal.ServiceRegion() == LOCAL_MGR::NC_PHILIPPINES || g_kLocal.ServiceRegion() == LOCAL_MGR::NC_RUSSIA)// 싱가폴 막아놓기
		{
			int nLevel = 0;
			if(_pkCompo->GetPropertyData(COMPONENT_HIDDEN_PORTAL_LEVEL, nLevel)) 
			{
				if(nLevel != 0)
				{
					_pkObject->SetAppCulled(true);
					NiIntegerExtraData* pkData = NiNew NiIntegerExtraData(0);
					_pkObject->AddExtraData("Hidden_Portal", pkData);
				}
			}
		}
		else
		{
			int nLevel = 0;
			if(_pkCompo->GetPropertyData(COMPONENT_HIDDEN_PORTAL_LEVEL, nLevel))
			{
				int nMyLevel = g_kChaLevel.GetCharacterLevel();
				if(nMyLevel < nLevel)
				{
					_pkObject->SetAppCulled(true);
					NiIntegerExtraData* pkData = NiNew NiIntegerExtraData(0);
					_pkObject->AddExtraData("Hidden_Portal", pkData);
				}
			}
		}
	}
}
HRESULT ApplyComponent_AnimationObject(NiEntityInterface const * pkEntity, NiEntityComponentInterface* pkCompo, NiNode* pkSceneRootNode, NiNode* pkChildNode)
{
	float fPhaseTime = 0.0f;
	if (pkCompo->GetPropertyData(COMPONENT_PHASE, fPhaseTime))
	{
		if(fPhaseTime == -1001)
		{
			PgWorld::SetAniStop(pkChildNode);
		}
		else
		{
			bool bRandomTime = (fPhaseTime == -1000.0f);
			PgWorld::SetTimeToAniObj(pkChildNode, fPhaseTime, bRandomTime);
		}

	}
	else if(0)
	{
	}
	return S_OK;
}

HRESULT ApplyComponent_EventObject(NiEntityComponentInterface const * _pkCompo, NiAVObject *_pkObject)
{
	NiFixedString kString;
	if(_pkCompo->GetPropertyData(COMPONENT_EVENT_OBJECT_NAME, kString, 0))
	{
		std::string strName = kString;
		NiNode* pkNode = NiDynamicCast(NiNode, _pkObject);
		if(pkNode)
		{
			g_kEventObject.InsertEventObject(strName, pkNode);
		}
	}
	return S_OK;
}

HRESULT ApplyComponent( NiEntityInterface const* pkEntity, NiNode* pkSceneRootNode, NiNode* pkChildNode )
{
    if ( pkEntity == NULL || pkSceneRootNode == NULL || pkChildNode == NULL )
        return E_FAIL;

    bool bFoundGlowMapCompo = false;

    for ( unsigned int iCompo = 0; iCompo < pkEntity->GetComponentCount(); ++iCompo )
    {
        NiEntityComponentInterface* pkCompo = pkEntity->GetComponentAt( iCompo );
        if ( !pkCompo )
            continue;

        if ( pkCompo->GetName() == COMPONENT_ANIMATION_OBJECT )
        {
            ApplyComponent_AnimationObject( pkEntity, pkCompo, pkSceneRootNode, pkChildNode );
        }
        else if ( pkCompo->GetName() == COMPONENT_HIDE_OBJECT )
        {
            if ( g_pkWorld )
                g_pkWorld->AddHideObject( pkCompo, pkChildNode );
        }
        else if ( pkCompo->GetName() == COMPONENT_GLOWMAP )
        {
            if ( g_pkWorld )
                g_pkWorld->SetDetachGlowMap( pkCompo, pkChildNode );
            bFoundGlowMapCompo = true;
        }
        else if ( pkCompo->GetName() == COMPONENT_USE_UPDATE_LOD )
        {
            if ( g_pkWorld )
                g_pkWorld->SetUseUpdateLodForLoadTime( pkCompo, pkChildNode );
        }
        else if ( pkCompo->GetName() == COMPONENT_OCCLUSION_CULLING )
        {
            if ( g_pkWorld )
                g_pkWorld->ApplyComponentOcclusionCulling( pkCompo, pkChildNode );
        }
        else if ( pkCompo->GetName() == COMPONENT_HIDDEN_PORTAL )
        {
            if ( g_pkWorld )
                g_pkWorld->ApplyComponentHiddenPortal( pkCompo, pkChildNode );
        }
        else if ( pkCompo->GetName() == COMPONENT_EVENT_OBJECT )
        {
            ApplyComponent_EventObject( pkCompo, pkChildNode );
        }
    }

	if ( false == bFoundGlowMapCompo && pkChildNode )
	{
		// 글로우맵 컴포넌트가 없을 경우(씬디자이너에서 추가하지 않았을 경우)에는 글로우맵을 비활성 시킴.
		pkChildNode->AddExtraData( "DETACH_GLOWMAP", NiNew NiIntegerExtraData(0) );
	}
    return S_OK;
}

void PgWorld::loadOtherEntities(NiEntityInterface *pkEntity, NiEntityInterfaceContainer &kMapLight)
{
	NiObject* pkSceneRootPointer = NULL;
	NiAVObject* pkSceneRoot = NULL;

	if (pkEntity->GetPropertyData("Scene Root Pointer", pkSceneRootPointer, 0))
	{
		pkSceneRoot = NiDynamicCast(NiAVObject, pkSceneRootPointer);
		pkSceneRoot->SetName(pkEntity->GetName());
	}

	if (pkSceneRoot)
	{
		bool bIsAfterAttach = false;

		// 기본 카메라를 설정한다.
		if (NiIsKindOf(NiCamera, pkSceneRoot) &&
			pkEntity->GetPGProperty().Equals("MainCamera"))
		{
			NiCameraPtr spCamera = (NiCamera *)pkSceneRoot;
			//spCamera->SetName("main_camera");

			// Culling 거리 조절
			NiFrustum kFrustum = spCamera->GetViewFrustum();
			kFrustum.m_fNear = 100.0f;
			kFrustum.m_fFar = 20000.0f;
			kFrustum.m_fLeft = -0.51428568f;
			kFrustum.m_fRight = 0.51428568f;
			kFrustum.m_fTop = 0.38571426f;
			kFrustum.m_fBottom = -0.38571426f;
			spCamera->SetViewFrustum(kFrustum);

			g_kFrustum = kFrustum;

			// 카메라를 카메라맨에 등록한다.
			m_kCameraMan.SetCamera(spCamera);
			m_kCameraMan.AddCamera((NiString)(pkEntity->GetName()), spCamera);
			m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_NONE, 0);

			// 카메라를 씬에 등록한다.
			NiPoint3 kLoc = spCamera->GetWorldTranslate();
			NiMatrix3 kRot = spCamera->GetWorldRotate();
			spCamera->SetTranslate(kLoc);
			spCamera->SetRotate(kRot);
		}
		// 미니맵용 카메라를 설정.
		else if (NiIsKindOf(NiCamera, pkSceneRoot) &&
			pkEntity->GetPGProperty().Equals("MinimapCamera"))
		{
			if (m_spMinimapCamera)
			{
				m_spMinimapCamera = 0;
			}

			m_spMinimapCamera = (NiCamera *)pkSceneRoot;
			m_spMinimapCamera->SetName("minimap_camera");
			NiFrustum kFrustum = m_spMinimapCamera->GetViewFrustum();
			//kFrustum.m_bOrtho = true;
			kFrustum.m_fNear = 1.0f;
			kFrustum.m_fFar = 100000.0f;
			m_spMinimapCamera->SetViewFrustum(kFrustum);
		}
		// 라이트를 찾아서 'Entity로딩 후 라이트 적용'을 위해 맵에 저장해둔다.
		else if (NiIsKindOf(NiLight, pkSceneRoot))
		{
			kMapLight.insert(std::make_pair(pkEntity->GetName(), pkEntity));
			bIsAfterAttach = true;
		}

		if (!bIsAfterAttach)
		{
			m_spSceneRoot->AttachChild(pkSceneRoot, true);
		}
	}
}

void PgWorld::lightSetting(bool bFindBase, NodeContainer &kMapObjForLight, NiEntityInterfaceContainer &kMapLight)
{
	// Affected로 설정 된 물체들에게 라이트를 적용시켜 준다.
	//	SAFE_DELETE_NI(m_spLightRoot);	//	A smart pointer should not be deleted by using NiDelete
	m_spLightRoot = NiNew NiNode;
	m_spLightRoot->SetName("lights");

	// BaseObject 가 있으면 기본 LoadNif 라이트 세팅을 이용한다.
#ifdef PG_USE_LOADGSA_BASE
	if (bFindBase)
	{
		NiTPointerList<NiNodePtr> kStack;
		kStack.AddTail((NiNode*)m_spSceneRoot);

		while(!kStack.IsEmpty())
		{
			NiNodePtr spNode = kStack.RemoveTail();
			for(unsigned int index = 0; index < spNode->GetArrayCount(); ++index)
			{
				NiAVObject *pkObject = spNode->GetAt(index);
				if (!pkObject)
				{
					continue;
				}
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
#endif
	{
		NiEntityInterfaceContainer::iterator light_itr = kMapLight.begin();
		while(light_itr != kMapLight.end())
		{
			NiEntityInterface *pkEntity = light_itr->second;
			NiObject* pkSceneRootPointer = NULL;
			NiAVObject* pkSceneRoot = NULL;

			if (pkEntity->GetPropertyData("Scene Root Pointer", pkSceneRootPointer, 0))
			{
				pkSceneRoot = NiDynamicCast(NiAVObject, pkSceneRootPointer);
			}

			if (NiIsKindOf(NiLight, pkSceneRoot))
			{
				NiLight *pkLight = NiDynamicCast(NiLight, pkSceneRoot);
				pkLight->SetName(pkEntity->GetName());

				unsigned int iComponentCount = pkEntity->GetComponentCount();
				for (unsigned ucc = 0 ; ucc < iComponentCount ; ucc++)
				{
					NiEntityComponentInterface *pkComponentInter = pkEntity->GetComponentAt(ucc);

					if (pkComponentInter->GetClassName().Equals("NiLightComponent"))
					{
						NiLightComponent *pkLightComponent = (NiLightComponent *)pkComponentInter;
						int iAffectedCount = pkLightComponent->GetAffectedEntitiesCount();
						for (int iAffected=0 ; iAffected<iAffectedCount ; iAffected++)
						{
							NiEntityInterface* kAffectedEntity
								= pkLightComponent->GetAffectedEntityAt(iAffected);
							if (kAffectedEntity)
							{
								std::string strEntityName = kAffectedEntity->GetName();
								NodeContainer::iterator itor;
								itor = kMapObjForLight.find(strEntityName);
								if (itor != kMapObjForLight.end())
								{
									NiNode *pkAffectedNode = itor->second;
									pkLight->AttachAffectedNode(pkAffectedNode);
								}
							}
						}
					}

				}
				// insert
				m_spLightRoot->AttachChild(pkLight, true);
			}
			++light_itr;
		}
	}
	// End of insert affected object
	m_spSceneRoot->AttachChild(m_spLightRoot, true);
	//SetGeometryShader((NiAVObject*)m_spSceneRoot);
}

HRESULT PgWorld::AttachChildNode(NiEntityInterface const* pkEntity, NiNode* pkSceneRootNode, NiNode* pkChildNode,PhysXSceneVec const &kPhysXSceneCont)
{
	if (pkEntity == NULL || pkSceneRootNode == NULL || pkChildNode == NULL)
	{
		return E_FAIL;
	}
	bool	bNoPortalSystem = true;
	pkEntity->GetPropertyData(COMPONENT_IGNORE_PORTAL_SYSTEM, bNoPortalSystem);

	// PhysX가 없고 일반 Object 타입이면 Optimization 적용 하는 물체.
	if (pkEntity->GetPGProperty().Equals("Object") || pkEntity->GetPGProperty().Equals("Trap"))
	{
		bool	bHasTimeController = PgRenderer::HasTimeController(pkChildNode);
		bool	bHasTextureEffect = (PgRendererUtil::FindObjectByType<NiTextureEffect>(pkChildNode) != NULL);

		if((bHasTextureEffect || bHasTimeController) && GetSelectiveNodeRoot())
		{
			m_kMapObjectCont.insert(std::make_pair((char const*)pkEntity->GetName(),stMapObject(pkChildNode,kPhysXSceneCont,true,bNoPortalSystem)));
			GetSelectiveNodeRoot()->AttachChild(pkChildNode, true);
			return S_OK;
		}

		m_kMapObjectCont.insert(std::make_pair((char const*)pkEntity->GetName(),stMapObject(pkChildNode,kPhysXSceneCont,false,bNoPortalSystem)));
		GetStaticNodeRoot()->AttachChild(pkChildNode, true);
		return S_OK;
	}


	// Ladder의 처리를 해주자. 'ladder_obj'
	if (pkEntity->GetPGProperty().Equals("Ladder"))
	{
		NiNode* pkLadderObjNode = NiDynamicCast(NiNode, pkChildNode->GetObjectByName("ladder_obj"));
		if (pkLadderObjNode)
		{
			pkChildNode->Update(0);
			pkLadderObjNode = NiDynamicCast(NiNode, pkChildNode->GetObjectByName("ladder_obj"));

			// 지정된 노드만(ladder_obj) ladders안에 넣고 나머지는 SceneRoot에 넣는다.
			NiNode* pkLadder = NiDynamicCast(NiNode, pkSceneRootNode->GetObjectByName("ladders"));
		
			NiObjectList kGeometries;
			GetAllGeometries(pkLadderObjNode, kGeometries);
			while(!kGeometries.IsEmpty())
			{
				NiGeometry *pkGeo = NiDynamicCast(NiGeometry, kGeometries.GetTail());
				kGeometries.RemoveTail();
				pkGeo->SetTranslate(pkGeo->GetWorldTranslate());
				pkGeo->SetScale(pkGeo->GetWorldScale());
				pkGeo->SetRotate(pkGeo->GetWorldRotate());
				pkLadder->AttachChild(pkGeo, true);
			}
			m_kMapObjectCont.insert(std::make_pair((char const*)pkEntity->GetName(),stMapObject(pkChildNode,kPhysXSceneCont,false,bNoPortalSystem)));
			GetStaticNodeRoot()->AttachChild(pkChildNode, true);
			return S_OK;
		}
	}

	return E_FAIL;
}

// bRandomTime이 true이면 총 시간중에 랜덤으로 시작한다.
void PgWorld::SetTimeToAniObj(NiObjectNET* pkObj, float &rfTime, bool bRandomTime)
{
	if(!g_pkWorld)
	{
		return;
	}
	if(g_pkWorld->FindDisableRandomAniObject(pkObj->GetName()))
		return;

    NiTimeController* pkControl = pkObj->GetControllers();
    for (/**/; pkControl; pkControl = pkControl->GetNext())
    {
		if (bRandomTime && (int)pkControl->GetEndKeyTime() != 0.0f)
		{
			rfTime = (float)BM::Rand_Index((int)(pkControl->GetEndKeyTime() * 100.0f));
			rfTime *= 0.01f;
			// RandomTime이 한번 정해지면 다음 재귀(하위)를 할 때는 랜덤이 되지 말라고 false를.
			bRandomTime = false;
		}
		pkControl->SetPhase(rfTime);
    }

    if (NiIsKindOf(NiAVObject, pkObj))
    {
        NiAVObject* pkAVObj = (NiAVObject*) pkObj;

        // recurse on properties
        NiTListIterator kPos = pkAVObj->GetPropertyList().GetHeadPos();
        while (kPos)
        {
            NiProperty* pProperty = pkAVObj->GetPropertyList().GetNext(kPos);
            if (pProperty && pProperty->GetControllers())
			{
                SetTimeToAniObj(pProperty, rfTime, bRandomTime);
			}
        }
    }

    if (NiIsKindOf(NiNode, pkObj))
 	{
        NiNode* pkNode = (NiNode*) pkObj;
        // recurse on children
        for (unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
        {
            NiAVObject* pkChild = pkNode->GetAt(i);
            if (pkChild)
			{
                SetTimeToAniObj(pkChild, rfTime, bRandomTime);
			}
        }
    }
}

void PgWorld::IncTimeToAniObj(NiObjectNET* pkObj, unsigned __int64 iMilliSec)
{
	if(g_pkWorld)
	{
		if(g_pkWorld->FindDisableRandomAniObject(pkObj->GetName()))
		{
			return;
		}
	}

	if (0 > iMilliSec)
	{
		return;
	}

    NiTimeController* pkControl = pkObj->GetControllers();
    for (/**/; pkControl; pkControl = pkControl->GetNext())
    {
		if (pkControl == NULL)
		{
			continue;
		}

		if (0 > pkControl->GetPhase())
		{
			continue;
		}

		unsigned __int64 iPhase = (unsigned __int64)(pkControl->GetPhase() * 1000.0f);
		iPhase += iMilliSec;
		if (0 > iPhase)
		{
			continue;
		}

		unsigned __int64 iEndKeyTime = (unsigned __int64)(pkControl->GetEndKeyTime() * 1000.0f);	// sec -> millisec
		if (iEndKeyTime)
		{
			unsigned __int64 iResult = iPhase % iEndKeyTime;
			float fResult = iResult * 0.001f;		// millisec -> sec
			pkControl->SetPhase(fResult);
		}
    }

    if (NiIsKindOf(NiAVObject, pkObj))
    {
        NiAVObject* pkAVObj = (NiAVObject*) pkObj;
        // recurse on properties
        NiTListIterator kPos = pkAVObj->GetPropertyList().GetHeadPos();
        while (kPos)
        {
            NiProperty* pProperty = pkAVObj->GetPropertyList().GetNext(kPos);
            if (pProperty && pProperty->GetControllers())
			{
                IncTimeToAniObj(pProperty, iMilliSec);
			}
        }
    }

    if (NiIsKindOf(NiNode, pkObj))
    {
        NiNode* pkNode = (NiNode*)pkObj;
        // recurse on children
        for (unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
        {
            NiAVObject* pkChild = pkNode->GetAt(i);
            if (pkChild)
			{
                IncTimeToAniObj(pkChild, iMilliSec);
			}
        }
    }
}

void PgWorld::SetAniCycleType(NiObjectNET* pkObj, NiTimeController::CycleType eType)
{
    NiTimeController* pkControl = pkObj->GetControllers();
    for (/**/; pkControl; pkControl = pkControl->GetNext())
    {
		// Set
		pkControl->SetCycleType(eType);
    }

    if (NiIsKindOf(NiAVObject, pkObj))
    {
        NiAVObject* pkAVObj = (NiAVObject*) pkObj;

        // recurse on properties
        NiTListIterator kPos = pkAVObj->GetPropertyList().GetHeadPos();
        while (kPos)
        {
            NiProperty* pkProperty = pkAVObj->GetPropertyList().GetNext(kPos);
            if (pkProperty && pkProperty->GetControllers())
                SetAniCycleType(pkProperty, eType);
        }
    }

    if (NiIsKindOf(NiNode, pkObj))
    {
        NiNode* pkNode = (NiNode*) pkObj;

        // recurse on children
        for (unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
        {
            NiAVObject* pkChild;

            pkChild = pkNode->GetAt(i);
            if (pkChild)
                SetAniCycleType(pkChild, eType);
        }
    }
}

void PgWorld::SetAniStop(NiObjectNET* pkObj)
{
	NiTimeController* pkControl = pkObj->GetControllers();
	for (/**/; pkControl; pkControl = pkControl->GetNext())
	{
		// Set
		//pkControl->Stop();
		pkObj->RemoveController(pkControl);
	}

	if (NiIsKindOf(NiAVObject, pkObj))
	{
		NiAVObject* pkAVObj = (NiAVObject*) pkObj;

		// recurse on properties
		NiTListIterator kPos = pkAVObj->GetPropertyList().GetHeadPos();
		while (kPos)
		{
			NiProperty* pkProperty = pkAVObj->GetPropertyList().GetNext(kPos);
			if (pkProperty && pkProperty->GetControllers())
				SetAniStop(pkProperty);
		}
	}

	if (NiIsKindOf(NiNode, pkObj))
	{
		NiNode* pkNode = (NiNode*) pkObj;

		// recurse on children
		for (unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
		{
			NiAVObject* pkChild;

			pkChild = pkNode->GetAt(i);
			if (pkChild)
				SetAniStop(pkChild);
		}
	}
}

void PgWorld::SetAniType(NiObjectNET* pkObj, NiTimeController::AnimType eType)
{
    NiTimeController* pkControl = pkObj->GetControllers();
    for (/**/; pkControl; pkControl = pkControl->GetNext())
    {
		// Set
		pkControl->SetAnimType(eType);
    }

    if (NiIsKindOf(NiAVObject, pkObj))
    {
        NiAVObject* pkAVObj = (NiAVObject*) pkObj;

        // recurse on properties
        NiTListIterator kPos = pkAVObj->GetPropertyList().GetHeadPos();
        while (kPos)
        {
            NiProperty* pkProperty = pkAVObj->GetPropertyList().GetNext(kPos);
            if (pkProperty && pkProperty->GetControllers())
                SetAniType(pkProperty, eType);
        }
    }

    if (NiIsKindOf(NiNode, pkObj))
    {
        NiNode* pkNode = (NiNode*) pkObj;

        // recurse on children
        for (unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
        {
            NiAVObject* pkChild;

            pkChild = pkNode->GetAt(i);
            if (pkChild)
                SetAniType(pkChild, eType);
        }
    }
}


bool ChangePhysXName(NiPhysXSceneDesc* pkDesc, float fScale, const std::string& rkNifPath)
{
	bool bChange = false;
	int iActorTotal = pkDesc->GetActorCount();
	for (int iActorCount=0 ; iActorCount<iActorTotal ; ++iActorCount)
	{
		NiPhysXActorDesc *pkActorDesc = pkDesc->GetActorAt(iActorCount);
		// 정확한 위치 계산을 위한 Matrix.
		NxMat34 kMultMat;
		kMultMat = pkActorDesc->GetPose(0);

		int iShapeTotal = pkActorDesc->GetActorShapes().GetSize();
		for (int iShapeCount=0 ; iShapeCount<iShapeTotal ; iShapeCount++)
		{
			NiPhysXShapeDesc *pkShapeDesc =
				pkActorDesc->GetActorShapes().GetAt(iShapeCount);

			// Rename PhysX Object
			if (pkShapeDesc->GetMeshDesc())
			{
				std::string strMeshName = pkShapeDesc->GetMeshDesc()->GetName();

				char szUnique[1024];
				sprintf_s(szUnique, "%s_%f_%d_%d", rkNifPath.c_str(), fScale, iActorCount, iShapeCount);
				std::string strUnique = szUnique;
				//std::string strUnique = MB(BM::GUID::Create().str());				

				NiString strDescName = strUnique.c_str();
				NiFixedString strDescName_ = strDescName.MakeExternalCopy();
				//pkShapeDesc->GetMeshDesc()->SetName(strDescName_);
				bChange = true;
			}
		}
	}

	return bChange;
}


void PgWorld::CreatePhysXScene()
{
	// Create Root PhysX Scene
	if (!m_spPhysXScene)
	{
		m_spPhysXScene = NiNew NiPhysXScene();
		NxSceneDesc kSceneDesc;
		kSceneDesc.gravity = NxVec3(0.0f, 0.0f, m_fGravity);
		kSceneDesc.simType = NX_SIMULATION_SW;
		if (lua_tinker::call<bool>("UsePhysXThread"))
		{
			//kSceneDesc.backgroundThreadCount = PgComputerInfo::GetCPUInfo().iNumProcess;
			kSceneDesc.internalThreadCount = PgComputerInfo::GetCPUInfo().iNumProcess;
			kSceneDesc.threadMask = 0xfffffffe;
			kSceneDesc.flags |= NX_SF_ENABLE_MULTITHREAD;
		}
		NxScene* pkNxScene = NiPhysXManager::GetPhysXManager()->m_pkPhysXSDK->createScene(kSceneDesc);
		PG_ASSERT_LOG(pkNxScene);
		m_spPhysXScene->SetPhysXScene(pkNxScene);
	}
}
bool	PgWorld::LoadActorEntity(NiEntityInterface *pkEntity,NodeContainer &kMapObjForLight)
{
	NiActorComponent* pkActorComponent = 
        (NiActorComponent*)NewWare::Scene::AssetUtils::GSA::GetComponentByName( pkEntity, COMPONENT_ACTOR );
	if ( pkActorComponent )
	{
		bool const	bUsePhysX = pkEntity->GetPGUsePhysX();
		bool const	bIsTrigger = pkEntity->GetPGProperty().Equals("Trigger") || pkEntity->GetPGProperty().Equals("Trap");

		NiTransform	kTransform;
		pkEntity->GetPropertyData("Translation", kTransform.m_Translate);
		pkEntity->GetPropertyData("Rotation", kTransform.m_Rotate);
		pkEntity->GetPropertyData("Scale", kTransform.m_fScale);

		NiFixedString	kKFMFilePath;
		pkEntity->GetPropertyData("KFM File Path",kKFMFilePath);
		unsigned	int	uiActiveSeq = 0;
		pkEntity->GetPropertyData("Active Sequence ID",uiActiveSeq);


		NiFixedString	kActorXMLPath;
		PgPuppet* pkPuppet=NULL;
		if(pkEntity->GetPropertyData(COMPONENT_ACTORXML_PATH,kActorXMLPath))
		{
			pkPuppet = AddPuppet_ActorXML((char const*)pkEntity->GetName(),(char const*)kActorXMLPath,uiActiveSeq,kTransform,bUsePhysX,bIsTrigger);
		}
		else
		{
			pkPuppet = AddPuppet_KFMPath((char const*)pkEntity->GetName(),(char const*)kKFMFilePath,uiActiveSeq,kTransform,bUsePhysX,bIsTrigger);					
		}
		if(!pkPuppet)
		{
			return	false;
		}

		NiNode* pkKfmNode = NiDynamicCast(NiNode, pkPuppet->GetAt(0));
		if(pkKfmNode)
		{
			kMapObjForLight.insert(std::make_pair(pkEntity->GetName(), pkKfmNode));

			NiFixedString kPostfixTexture = pkEntity->GetPGPostfixTexture();
			if (kPostfixTexture.Exists() && kPostfixTexture.GetLength() > 0)
			{
				std::string strPostfixTexture = kPostfixTexture;
				SetPostFixTextureToNode(strPostfixTexture, pkKfmNode);
			}	
		}

		return	true;
	}
	return	false;
}
bool	PgWorld::LoadEnvSoundEntity(NiEntityInterface *pkEntity)
{
	NiEntityComponentInterface* pkEnvSoundComponent = 
                NewWare::Scene::AssetUtils::GSA::GetComponentByName(pkEntity, COMPONENT_ENVSOUND);
	if(NULL != pkEnvSoundComponent)
	{
		NiFixedString kMediaPath = "";
		float fVolume = 0.0f, fAffectRange=0.0f, fAffectAtten=0.0f, fProbability=0.0f;
		bool bRandom=false, bMixBGSound=false;
		NiPoint3 kSoundPos(0.0f, 0.0f, 0.0f);

		pkEntity->GetPropertyData("Translation", kSoundPos);
		pkEnvSoundComponent->GetPropertyData("Media Path", kMediaPath);
		pkEnvSoundComponent->GetPropertyData("Volume", fVolume);
		pkEnvSoundComponent->GetPropertyData("Affect Range", fAffectRange);
		pkEnvSoundComponent->GetPropertyData("Affect Attenuation", fAffectAtten);
		pkEnvSoundComponent->GetPropertyData("Random Play", bRandom);
		pkEnvSoundComponent->GetPropertyData("Random Probability", fProbability);
		pkEnvSoundComponent->GetPropertyData("Mix BGSound", bMixBGSound);

		//맵툴에서 저장된 사운드의 절대 경로를 클라이언트 기준의 리소스경로로 다시 맞춘다.
		//맞추는 방법
		//	1.환경사운드 리소스는 무조건 게임폴더\Sound\에 들어간다.
		//	2.GSA에서 읽은 리소스 절대경로에서 \\sound\\문자열을 찾는다.
		//	3.GSA에서 읽은 리소스 절대경로에서 2에서 찾은 위치 앞의 내용은 모두 지운다.
		//	4.3에서 구해진 결과에 ..\을 앞부분에 추가하여 실행파일 한단계 위의 경로임을 명시한다.
		std::string kRelativePath = static_cast<char const*>(kMediaPath);
		kRelativePath.erase(0, kRelativePath.rfind("\\Sound\\"));
		kRelativePath = ".." + kRelativePath;
		kMediaPath = kRelativePath.c_str();

		//char const* pcPath = kMediaPath;
		//g_kSoundMan.m_pkEnvSoundTemp = g_kSoundMan.PlayAudioSourceByPath(NiAudioSource::TYPE_3D, pcPath, 0.001f, fAffectAtten, fAffectRange, NULL, &kSoundPos);
		g_kSoundMan.AddAndPlayEnvSound(kMediaPath, fVolume, fAffectAtten, fAffectRange, kSoundPos, bRandom, fProbability, bMixBGSound);
		//사운드 오브젝트는 화면에 렌더링 하지 않는다.
		pkEntity->SetHidden(true);
		return	true;
	}	

	return	false;
}
void PgWorld::loadNifObject(NiEntityInterface const *pkEntity, NiNodePtr const &spNifRoot, bool bUseLOD, NiStream const &kUniqueStream, NodeContainer &kNodeCont, NodeContainer &kMapObjForLight, bool bLoadGsa2)
{
	if (pkEntity == NULL)
		return;

	NiFixedString kEntityName = pkEntity->GetName();
	bool bUsePhysX = pkEntity->GetPGUsePhysX();
	bool bIsTrigger = pkEntity->GetPGProperty().Equals("Trigger") || pkEntity->GetPGProperty().Equals("Trap");

	NiNode *pkSceneRoot = m_spSceneRoot;

	// Transform
	NiPoint3 kTranslation;
	NiMatrix3 kRotMat;
	float fScale;
	pkEntity->GetPropertyData("Translation", kTranslation);
	pkEntity->GetPropertyData("Rotation", kRotMat);
	pkEntity->GetPropertyData("Scale", fScale);
	NiPoint3 kObjTransl = kTranslation;
	NiMatrix3 kObjRotMat = kRotMat;
	float kObjRootScale = fScale;

	// 각 속성이 있는 노드는 지정된 노드속에 넣어준다.
	std::string strProperty = pkEntity->GetPGProperty();
	NodeContainer::iterator kNodeItr =  kNodeCont.find(strProperty);
	if (kNodeItr != kNodeCont.end())
	{
		pkSceneRoot = kNodeItr->second;
		kObjTransl = kTranslation - pkSceneRoot->GetTranslate();
		kObjRootScale = fScale * (1 / pkSceneRoot->GetScale());
		kObjRotMat = pkSceneRoot->GetRotate().Inverse() * kRotMat;
	}
	PG_ASSERT_LOG(pkSceneRoot);

	NiNode *pkChildRootNode = 0;
	if (bIsTrigger || bUseLOD || bLoadGsa2)
	{
		pkChildRootNode = spNifRoot;
	}
	else
	{
		pkChildRootNode = NiDynamicCast(NiNode, spNifRoot->Clone());
	}
	kMapObjForLight.insert(std::make_pair(kEntityName, pkChildRootNode));
	pkChildRootNode->SetScale(kObjRootScale);
	pkChildRootNode->SetRotate(kObjRotMat);
	pkChildRootNode->SetTranslate(kObjTransl);
	pkChildRootNode->SetName(kEntityName);
	pkChildRootNode->Update(0);

	// 하이드 해준다.
	if (pkEntity->GetHidden())
	{
		pkChildRootNode->SetAppCulled(true);
	}

	// 텍스쳐 변경
	NiFixedString kPostfixTexture = pkEntity->GetPGPostfixTexture();
	if (kPostfixTexture.Exists() && kPostfixTexture.GetLength() > 0)
	{
		std::string strPostfixTexture = kPostfixTexture;
		SetPostFixTextureToNode(strPostfixTexture, pkChildRootNode);
	}

	// Random Ani
	if (pkEntity->GetPGRandomAni() && 0)
	{
		float fTime = 0;
		SetTimeToAniObj(pkChildRootNode, fTime, true);
	}

	// PG Property
	NiFixedString kAlphaGroup = pkEntity->GetPGAlphaGroup();
	if (kAlphaGroup.GetLength() > 0)
	{
		int iAlphaGroup = atoi(kAlphaGroup);
		if ( iAlphaGroup >= -5 && iAlphaGroup <= 5 )
            NewWare::Scene::ApplyTraversal::Property::SetAlphaGroup( spNifRoot, iAlphaGroup );
	}

#ifdef PG_USE_LOADGSA_BASE
	// Transform을 적용하면 이상하게 먹힌다.
	//kPhysXTranslation = kPhysXTranslation - kBaseTranslation;
	//kPhysXRotMat = kBaseRotMat.Inverse() * kPhysXRotMat;
	//fPhysXScale = fPhysXScale * (1 / fBaseScale);
#endif
	if (bUsePhysX)
	{
		createPhysXObject(pkEntity, kUniqueStream);
	}

	// 컴퍼넌트 세팅 적용.
	ApplyComponent(pkEntity, m_spSceneRoot, pkChildRootNode);

	// 예외 Attach하는 경우 (예: 사다리, Optimization 적용 할 물체)
	if (AttachChildNode(pkEntity, m_spSceneRoot, pkChildRootNode,PhysXSceneVec()) == E_FAIL)
	{
		pkSceneRoot->AttachChild(pkChildRootNode, true);
	}

	if (bUseLOD)
	{
		m_kOptPolyCont.insert(std::make_pair(kEntityName, NiDynamicCast(NiLODNode, spNifRoot)));
	}

	std::string strOpt = pkEntity->GetPGOptimization();
	//if (bUsePhysX == false)
	{
		if (strOpt == "1")
		{
			m_kOptimizeMidCont.insert(std::make_pair(kEntityName, pkChildRootNode));
			m_kCullContainter.insert(std::make_pair(pkChildRootNode, ALPHA_PROCESS_CULL_FALSE));
		}
		else if (strOpt == "2")
		{
			m_kOptimizeLowCont.insert(std::make_pair(kEntityName, pkChildRootNode));
			m_kCullContainter.insert(std::make_pair(pkChildRootNode, ALPHA_PROCESS_CULL_FALSE));
		}
	}
}
void PgWorld::createPhysXObject(NiEntityInterface const *pkEntity, NiStream const &kUniqueStream)
{
	if (pkEntity == NULL)
		return;

	NiFixedString kPath;
	NiPoint3 kPhysXTranslation;
	NiMatrix3 kPhysXRotMat;
	float fPhysXScale;

	pkEntity->GetPropertyData("NIF File Path", kPath);
	pkEntity->GetPropertyData("Translation", kPhysXTranslation);
	pkEntity->GetPropertyData("Rotation", kPhysXRotMat);
	pkEntity->GetPropertyData("Scale", fPhysXScale);

	unsigned int cnt = kUniqueStream.GetObjectCount();
	for (unsigned int ui = 0 ; ui < cnt ; ui++)
	{
		NiObject *pkObject = kUniqueStream.GetObjectAt(ui);
		if(NiIsKindOf(NiPhysXScene, pkObject))
		{
			NiPhysXScenePtr spPhysXObject = NiDynamicCast(NiPhysXScene, pkObject);
			NiPhysXScenePtr spPhysXOrgObject = spPhysXObject;

			// 피직스 씬 루트에 종속 시킨다.
			// 그럼으로써 충돌 등 효과를 본다.
			if (spPhysXObject && spPhysXObject->GetSnapshot())
			{
				// 스케일이 1이 아닐경우. ReCook을 한다.
				if (fPhysXScale != 1.0f)
				{
					//SetScaleToPhysXObject(pkDesc, fPhysXScale);
					NiCloningProcess kCloning;
					kCloning.m_eCopyType = NiObjectNET::COPY_UNIQUE;
					spPhysXObject = (NiPhysXScene*)spPhysXOrgObject->Clone(kCloning);
					NiPhysXSceneDesc* pkCloneDesc = spPhysXObject->GetSnapshot();
					MakePhysXScaleObject(pkCloneDesc, fPhysXScale, std::string(kPath));
				}

				NiPhysXSceneDesc *pkDesc = spPhysXObject->GetSnapshot();
				// 중복 네임이 있으면 피직스가 잘못 먹히기 때문에
				// 강제로 이름을 바꾸어 준다.
				int iActorTotal = pkDesc->GetActorCount();
				for (int iActorCount=0 ; iActorCount<iActorTotal ; ++iActorCount)
				{
					NiPhysXActorDesc *pkActorDesc = pkDesc->GetActorAt(iActorCount);
					int iShapeTotal = pkActorDesc->GetActorShapes().GetSize();
					for (int iShapeCount=0 ; iShapeCount<iShapeTotal ; iShapeCount++)
					{
						NiPhysXShapeDesc *pkShapeDesc =
							pkActorDesc->GetActorShapes().GetAt(iShapeCount);

						// Rename PhysX Object
						if (pkShapeDesc->GetMeshDesc())
						{
							NiString strDescName = pkEntity->GetName();
							strDescName += "_";
							char szCount[256];
							_itoa_s(iActorCount, szCount, 10);
							strDescName += szCount;
							strDescName += "_";
							_itoa_s(iShapeCount, szCount, 10);
							strDescName += szCount;
							strDescName += "_";
							_itoa_s((int)(fPhysXScale*10000), szCount, 10);
							strDescName += szCount;
							strDescName += "_";
							strDescName += kPath;
							NiFixedString strDescName_ = strDescName.MakeExternalCopy();
							pkShapeDesc->GetMeshDesc()->SetName(strDescName_);
						}
					}
				}

				NxMat34 kSlaveMat;
				NiPhysXTypes::NiTransformToNxMat34(kPhysXRotMat, kPhysXTranslation, kSlaveMat);
				spPhysXObject->SetSlaved(m_spPhysXScene, kSlaveMat);
				spPhysXObject->CreateSceneFromSnapshot(0);
			}

			// Insert PhysX Kinetic Data(Trigger Data)
			for (unsigned int iSrcCount=0 ; iSrcCount<spPhysXObject->GetSourcesCount() ; iSrcCount++)
			{
				NiPhysXSrc *pkPhysXSrc = spPhysXObject->GetSourceAt(iSrcCount);
				if(NiIsKindOf(NiPhysXRigidBodySrc, pkPhysXSrc))
				{
					NiPhysXRigidBodySrc *pkBodySrc = (NiPhysXRigidBodySrc *)pkPhysXSrc;
					NiAVObject *pkSrcObj = pkBodySrc->GetSource();
					pkSrcObj->SetName(pkEntity->GetName());
				}
				m_spPhysXScene->AddSource(pkPhysXSrc);
			}

			/*
			// Destination - 현재 쓰지 않음.
			for (unsigned int iDstCount=0 ; iDstCount<pkPhysXObject->GetDestinationsCount() ; iDstCount++)
			{
			NiPhysXDest *pkPhysXDst = pkPhysXObject->GetDestinationAt(iDstCount);
			NiAVObject *pkSrcObj = 0;

			if(NiIsKindOf(NiPhysXRigidBodyDest, pkPhysXDst))
			{
			NiPhysXRigidBodyDest *pkDst = (NiPhysXRigidBodyDest *)pkPhysXDst;
			//pkSrcObj = pkDst->GetSource();
			//pkSrcObj->SetName(pkEntity->GetName());
			}
			m_spPhysXScene->AddDestination(pkPhysXDst);
			}
			*/
		}
	}
}

void PgWorld::ClearContMinimapCamera()
{
	CONT_CAMERA::iterator it = m_kContMinimapCamera.begin();
	while(m_kContMinimapCamera.end() != it)
	{
		(*it).second = 0;
		++it;
	}
	m_kContMinimapCamera.clear();
}

bool PgWorld::LoadGsa(char const *pcGsaPath)
{
	NILOG(PGLOG_MINOR, "PgWorld:LoadGsa Start\n");
	
	g_kSoundMan.ReleaseEnvSound();
	ClearContMinimapCamera();

	unsigned int	uiVSVersion = 0;
	NiDX9Renderer* pkRenderer = NiDynamicCast(NiDX9Renderer, 
		NiRenderer::GetRenderer());
	if(pkRenderer)
	{
		uiVSVersion = pkRenderer->GetVertexShaderVersion();
	}

	m_bLoadGsa = true;
	ClearHideObject();
	g_kEventObject.ClearEventObject();

	NiEntityStreaming* pkEntityStreaming = NiFactories::GetStreamingFactory()->GetPersistent("PACK_GSA");
	PG_ASSERT_LOG(pkEntityStreaming);
	if (pkEntityStreaming == NULL)
		return false;

	NiDefaultErrorHandler kDefaultErrorHandler;
	pkEntityStreaming->SetErrorHandler(&kDefaultErrorHandler);
	
	if (!pkEntityStreaming->Load(pcGsaPath))
	{
		PG_ASSERT_LOG(!"gsa loading failed");
		return false;
	}

	m_spSceneRoot = 0;
	m_spPhysXScene = 0;
	NiScene *pkScene = pkEntityStreaming->GetSceneAt(0);
	if(!pkScene)
	{
		PG_ASSERT_LOG(!"gsa loading failed <pkScene is NULL>");
		return false;
	}

	bool bFindBase = false;
#ifdef PG_USE_LOADGSA_BASE
	bFindBase = LoadBaseObject(pkScene);
#endif
	if (!bFindBase)
	{
		NiAVObject *pkRoot = NiNew NiNode;
		pkRoot->SetName("Scene Root");
		m_spSceneRoot = (NiNode *)pkRoot;
	}

	// Create default node
	// 앞에 것은 맵툴에서 정해지는 Type
	// 뒤에 것은 쓰이는 노드 이름.

	NodeContainer kNodeCont;
	CreateDefaultNode(kNodeCont,bFindBase);
	CreatePhysXScene();

	////////////////////////////////////////////////////////////////////
	// 하나의 Entity는 하나의 Nif와 동일
	NodeContainer kMapObjForLight;
	NiEntityInterfaceContainer kMapLight;
	NiExternalAssetManagerPtr pkAssetManager = NiNew NiExternalAssetManager(NiFactories::GetAssetFactory());	

	int iTotal = pkScene->GetEntityCount();
	NILOG(PGLOG_MINOR, "PgWorld:Iterating Entity %d total\n", iTotal);
	for (int i=0 ; i<iTotal ; i++)
	{
#ifdef PG_USE_LOADGSA_PERFORMCHECK
		PG_STAT(timerB.Start());
#endif
		NiNode* pkAssetNode = NULL;
		NiStream kUniqueStream;
		NiEntityInterface *pkEntity = pkScene->GetEntityAt(i);
		if (!pkEntity)
		{
			PG_ASSERT_LOG(!"entity is null");
			NILOG(PGLOG_MINOR, "PgWorld:Iterating Entity %d entity is null\n", i);
			continue;
		}

#ifdef PG_USE_LOADGSA_BASE
		if (pkEntity->GetPGProperty().Equals("BaseObject"))
		{
			continue;
		}
#endif
		NiFixedString kEntityName = pkEntity->GetName();
		std::string strEntityName = kEntityName;

		// PG Property
		bool const bUsePhysX = pkEntity->GetPGUsePhysX();
		bool const bIsTrigger = (pkEntity->GetPGProperty().Equals("Trigger") || pkEntity->GetPGProperty().Equals("Trap"));
		bool bOnlyPlayerPhysx = false;
		bool bUseSharedStream = true; //SharedStream을 사용할 것인가.
		if(!pkEntity->GetPropertyData(COMPONENT_ONLY_PLAYER_PHYSX, bOnlyPlayerPhysx))
		{
			bOnlyPlayerPhysx = false;
		}
		NiEntityComponentInterface* pkSharedStreamCompo = 
            NewWare::Scene::AssetUtils::GSA::GetComponentByName( pkEntity, COMPONENT_SHAREDSTREAM );
		if(pkSharedStreamCompo)
		{
			pkSharedStreamCompo->GetPropertyData("Use Shared Stream", bUseSharedStream);
		}

		unsigned int uiIndex = 0;
		NiFixedString kPath;
		pkEntity->GetPropertyData("NIF File Path", kPath, uiIndex);
#ifdef PG_USE_LOADGSA_PERFORMCHECK
		PG_STAT(timerB.Stop());
#endif

		//int const iComponentCount = pkEntity->GetComponentCount();

		//	Is this an environment sound entity?
		if(LoadEnvSoundEntity(pkEntity))
		{
			continue;
		}

		if (kPath.Exists())
		{
			std::string strPath = kPath;
			NiNodePtr spNifRoot = 0;

			bool bShareStream = false;

			// 만약 트리거가 있다면 Share를 하지 않고 그냥 로드 한다.
			if (bUsePhysX || bIsTrigger || !bUseSharedStream)
			{
				//if (!kUniqueStream.Load(kPath))
                if ( NewWare::Scene::AssetUtils::NIF::Load(kUniqueStream, kPath) == NULL )
				{
					PgError3("[PgWorld] Load %s nif file faild(UsePhysX:%d, Trigger:%d)\n", kPath, bUsePhysX, bIsTrigger);
					continue;
				}
			}

			// 트리거이거나 Component상에 SharedStream을 사용치 않도로고 명시한 경우 Share를 하지 않는다.
			if (bIsTrigger || !bUseSharedStream)
			{
				//pkAssetNode = &kUniqueStream;
                pkAssetNode = static_cast<NiNode*>(kUniqueStream.GetObjectAt(0));
			}
			else
			{
				bShareStream = true;
				std::string strPath = kPath;
                pkAssetNode = m_kSharedAssets.GetAsset( strPath );
			}

			if (bShareStream && pkAssetNode == NULL)
			{
				NILOG(PGLOG_ERROR, "[PgWorld] Load %s file faild\n", strPath.c_str());
				continue;
			}

			//if (pkShareStream->GetObjectCount() == 0)
			//{
			//	NILOG(PGLOG_ERROR, "[PgWorld] Load %s file but no object\n", strPath.c_str());
			//	continue;
			//}

			//NiNodePtr spDefRoot = 0;
			//spDefRoot = NiDynamicCast(NiNode, pkShareStream->GetObjectAt(0));

			//if(!spDefRoot)
			//{
			//	PG_ASSERT_LOG(!"no root node");
			//	return false;
			//}
            NiNode* spDefRoot = pkAssetNode;

			bool bUseLOD = false;
			if ( !bUsePhysX )
			{
                NiNode* pkLowRoot = m_kSharedAssets.GetAssetAsLOD( strPath, 
                                                    NewWare::Scene::AssetManager::LOD_POSTFIX_LOW );
				if ( pkLowRoot )
				{
                    pkLowRoot->SetName( (strEntityName + NewWare::Scene::AssetManager::LOD_POSTFIX_LOW).c_str() );
                    bUseLOD = true;
				}

                NiNode* pkMidRoot = m_kSharedAssets.GetAssetAsLOD( strPath, 
                                                    NewWare::Scene::AssetManager::LOD_POSTFIX_MIDDLE );
				if ( pkMidRoot )
				{
                    pkMidRoot->SetName( (strEntityName + NewWare::Scene::AssetManager::LOD_POSTFIX_MIDDLE).c_str() );
                    bUseLOD = true;
				}

				if ( bUseLOD == false )
				{
					spNifRoot = spDefRoot;
				}
				else
				{
					spDefRoot->SetName((strEntityName + "_Def").c_str());			

					int iLODCount = 0;
					NiLODNode* pLODNode = NiNew NiLODNode();
					NiRangeLODData* pRangeLODData = NiNew NiRangeLODData();

					pLODNode->SetAt(0, (NiAVObject*)spDefRoot->Clone());
					iLODCount++;					
					if (pkMidRoot)
					{
						pLODNode->SetAt(iLODCount, (NiAVObject*)pkMidRoot->Clone());
						pRangeLODData->SetRange(0, 0.0f, 600.0f);
						if (pkLowRoot)
							pRangeLODData->SetRange(1, 600.0f, 1000.0f);
						else
							pRangeLODData->SetRange(1, 600.0f, 20000.0f);
						iLODCount++;
					}
					else
					{
						pRangeLODData->SetRange(0, 0.0f, 1000.0f);
					}

					if (pkLowRoot)
					{
						pLODNode->SetAt(iLODCount, (NiAVObject*)pkLowRoot->Clone());
						pRangeLODData->SetRange(iLODCount, 1000, 20000.0f);
						iLODCount++;
					}

					pLODNode->SetLODData(pRangeLODData);
					spNifRoot = pLODNode;
				}
			}
			else
			{
				spNifRoot = spDefRoot;
			}

			if (spNifRoot)
			{
				NiNode *pkSceneRoot = m_spSceneRoot;
				
				// Transform
				NiPoint3 kTranslation;
				NiMatrix3 kRotMat;
				float fScale;
				pkEntity->GetPropertyData("Translation", kTranslation);
				pkEntity->GetPropertyData("Rotation", kRotMat);
				pkEntity->GetPropertyData("Scale", fScale);
				NiPoint3 kObjTransl = kTranslation;
				NiMatrix3 kObjRotMat = kRotMat;
				float kObjRootScale = fScale;

				// PG Property
				NiFixedString kAlphaGroup = pkEntity->GetPGAlphaGroup();
				if (kAlphaGroup.GetLength() > 0)
				{
					int iAlphaGroup = atoi(kAlphaGroup);
					if ( iAlphaGroup >= -5 && iAlphaGroup <= 5 )
						NewWare::Scene::ApplyTraversal::Property::SetAlphaGroup( spNifRoot, iAlphaGroup );
				}
				
				// 각 속성이 있는 노드는 지정된 노드속에 넣어준다.
				std::string strProperty = pkEntity->GetPGProperty();
				NodeContainer::iterator kNodeItr =  kNodeCont.find(strProperty);
				if (kNodeItr != kNodeCont.end())
				{
					pkSceneRoot = kNodeItr->second;
					kObjTransl = kTranslation - pkSceneRoot->GetTranslate();
					kObjRootScale = fScale * (1 / pkSceneRoot->GetScale());
					kObjRotMat = pkSceneRoot->GetRotate().Inverse() * kRotMat;
				}
				PG_ASSERT_LOG(pkSceneRoot);

				NiNode *pkChildRootNode = 0;
				if (bIsTrigger || bUseLOD)
				{
					pkChildRootNode = spNifRoot;
				}
				else
				{
					pkChildRootNode = NiDynamicCast(NiNode, spNifRoot->Clone());
				}
				kMapObjForLight.insert(std::make_pair(kEntityName, pkChildRootNode));
				pkChildRootNode->SetScale(kObjRootScale);
				pkChildRootNode->SetRotate(kObjRotMat);
				pkChildRootNode->SetTranslate(kObjTransl);
				pkChildRootNode->SetName(kEntityName);
				pkChildRootNode->Update(0);

				// 하이드 해준다.
				if (pkEntity->GetHidden())
				{
					pkChildRootNode->SetAppCulled(true);
				}

				// 텍스쳐 변경
				NiFixedString kPostfixTexture = pkEntity->GetPGPostfixTexture();
				if (kPostfixTexture.Exists() && kPostfixTexture.GetLength() > 0)
				{
					std::string strPostfixTexture = kPostfixTexture;
					SetPostFixTextureToNode(strPostfixTexture, pkChildRootNode);
				}

				// Random Ani
				if (pkEntity->GetPGRandomAni() && 0)
				{
					float fTime = 0;
					SetTimeToAniObj(pkChildRootNode, fTime, true);
				}

				// Physx Data를 찾아서 넣어주자.
				NiPoint3 kPhysXTranslation = kTranslation;
				NiMatrix3 kPhysXRotMat = kRotMat;
				float fPhysXScale = fScale;

				PhysXSceneVec	kPhysXSceneCont;
				if (bUsePhysX)
				{
					unsigned int cnt = kUniqueStream.GetObjectCount();
					for (unsigned int ui = 0 ; ui < cnt ; ui++)
					{
						NiObject *pkObject = kUniqueStream.GetObjectAt(ui);
						if(NiIsKindOf(NiPhysXScene, pkObject))
						{
							NiPhysXScene *pkPhysXObject = NiDynamicCast(NiPhysXScene, pkObject);

							// 피직스 씬 루트에 종속 시킨다.
							// 그럼으로써 충돌 등 효과를 본다.
							if (pkPhysXObject->GetSnapshot())
							{

								PgPhysXUtil::MakeStaticMeshNameUnique(pkPhysXObject,(char const*)kPath,(char const*)pkEntity->GetName(),fPhysXScale);
								PgPhysXUtil::ChangeScale(pkPhysXObject,fPhysXScale);

								NxMat34 kSlaveMat;
								NiPhysXTypes::NiTransformToNxMat34(kPhysXRotMat, kPhysXTranslation, kSlaveMat);
								pkPhysXObject->SetSlaved(m_spPhysXScene, kSlaveMat);
								pkPhysXObject->CreateSceneFromSnapshot(0);
								kPhysXSceneCont.push_back(pkPhysXObject);
							}

							// Insert PhysX Kinetic Data(Trigger Data)
							for (unsigned int iSrcCount=0 ; iSrcCount<pkPhysXObject->GetSourcesCount() ; iSrcCount++)
							{
								NiPhysXSrc *pkPhysXSrc = pkPhysXObject->GetSourceAt(iSrcCount);
								if(NiIsKindOf(NiPhysXRigidBodySrc, pkPhysXSrc))
								{
									NiPhysXRigidBodySrc *pkBodySrc = (NiPhysXRigidBodySrc *)pkPhysXSrc;
									NiAVObject *pkSrcObj = pkBodySrc->GetSource();
									pkSrcObj->SetName(pkEntity->GetName());
								}
								m_spPhysXScene->AddSource(pkPhysXSrc);
							}

							if(bOnlyPlayerPhysx)
							{
								SetPhysXGroup(pkPhysXObject, PG_PHYSX_GROUP_PLAYER_WALL);
							}
						}
					}
				}
#ifdef PG_USE_LOADGSA_PERFORMCHECK
				PG_STAT(timerE.Stop());
#endif

				// 컴퍼넌트 세팅 적용.
				ApplyComponent(pkEntity, m_spSceneRoot, pkChildRootNode);

				// 예외 Attach하는 경우 (예: 사다리, Optimization 적용 할 물체)
				if (AttachChildNode(pkEntity, m_spSceneRoot, pkChildRootNode,kPhysXSceneCont) == E_FAIL)
				{
					pkSceneRoot->AttachChild(pkChildRootNode, true);
				}

				if (bUseLOD)
				{
					m_kOptPolyCont.insert(std::make_pair(kEntityName, NiDynamicCast(NiLODNode, spNifRoot)));
				}

				std::string strOpt = pkEntity->GetPGOptimization();
				//if (bUsePhysX == false)
				{
					if (strOpt == "1")
					{
						m_kOptimizeMidCont.insert(std::make_pair(kEntityName, pkChildRootNode));
						m_kCullContainter.insert(std::make_pair(pkChildRootNode, ALPHA_PROCESS_CULL_FALSE));
					}
					else if (strOpt == "2")
					{
						m_kOptimizeLowCont.insert(std::make_pair(kEntityName, pkChildRootNode));
						m_kCullContainter.insert(std::make_pair(pkChildRootNode, ALPHA_PROCESS_CULL_FALSE));
					}
				}
			}
#ifdef PG_USE_LOADGSA_DIFF_PERF
			PG_STAT(timerA.Stop());
#endif
		}
		// case Camera, etc
		else
		{
			//	Is this a portal system entity?
			if(GetPSRoomGroupRoot()->AddEntity(pkEntity))
			{
				continue;
			}
			
			if(LoadActorEntity(pkEntity,kMapObjForLight))
			{
				continue;
			}

			pkEntity->Update(pkEntity->GetMasterEntity(), 0, &kDefaultErrorHandler, pkAssetManager);
			NiObject* pkSceneRootPointer = NULL;
			NiAVObject* pkSceneRoot = NULL;

			if (pkEntity->GetPropertyData("Scene Root Pointer", pkSceneRootPointer, uiIndex))
			{
				pkSceneRoot = NiDynamicCast(NiAVObject, pkSceneRootPointer);
			}

			if (pkSceneRoot)
			{
				bool bIsAfterAttach = false;

				// 기본 카메라를 설정한다.
				if (NiIsKindOf(NiCamera, pkSceneRoot) &&
					pkEntity->GetPGProperty().Equals("MainCamera"))
				{
					NiCameraPtr spCamera = (NiCamera *)pkSceneRoot;
					//spCamera->SetName("main_camera");
					
					// Culling 거리 조절
					NiFrustum kFrustum = spCamera->GetViewFrustum();
					kFrustum.m_fNear = 100.0f;
					kFrustum.m_fFar = 20000.0f;
					kFrustum.m_fLeft = -0.51428568f;
					kFrustum.m_fRight = 0.51428568f;
					kFrustum.m_fTop = 0.38571426f;
					kFrustum.m_fBottom = -0.38571426f;
					spCamera->SetViewFrustum(kFrustum);

					g_kFrustum = kFrustum;

					// 카메라를 카메라맨에 등록한다.
					m_kCameraMan.SetCamera(spCamera);
					m_kCameraMan.AddCamera((NiString)(pkEntity->GetName()), spCamera);
					m_kCameraMan.SetCameraMode(PgCameraMan::CMODE_NONE, 0);
					
					// 카메라를 씬에 등록한다.
					NiPoint3 kLoc = spCamera->GetWorldTranslate();
					NiMatrix3 kRot = spCamera->GetWorldRotate();
					spCamera->SetTranslate(kLoc);
					spCamera->SetRotate(kRot);
				}
				// 미니맵용 카메라를 설정.
				else if (NiIsKindOf(NiCamera, pkSceneRoot) &&
						pkEntity->GetPGProperty().Equals("MinimapCamera"))
				{
					if (m_spMinimapCamera)
					{
						m_spMinimapCamera = 0;
					}

					m_spMinimapCamera = (NiCamera *)pkSceneRoot;
					m_spMinimapCamera->SetName("minimap_camera");
					NiFrustum kFrustum = m_spMinimapCamera->GetViewFrustum();
					//kFrustum.m_bOrtho = true;
					kFrustum.m_fNear = 1.0f;
					kFrustum.m_fFar = 100000.0f;
					m_spMinimapCamera->SetViewFrustum(kFrustum);

					m_kContMinimapCamera.insert(std::make_pair(pkEntity->GetName(),m_spMinimapCamera));
					m_kContMinimapCamera[DEFAULT_MINIMAPCAMERA] = m_spMinimapCamera;
				}
				// 라이트를 찾아서 'Entity로딩 후 라이트 적용'을 위해 맵에 저장해둔다.
				else if (NiIsKindOf(NiLight, pkSceneRoot))
				{
					kMapLight.insert(std::make_pair(pkEntity->GetName(), pkEntity));
					bIsAfterAttach = true;
				}


				//{//!/
				//	unsigned int unIndex = 0;
				//	NiFixedString kKfmPath;
				//	pkEntity->GetPropertyData("KFM File Path", kKfmPath, unIndex);
				//	if(kKfmPath.Exists())
				//	{
				//		NiFixedString kEntityName = pkEntity->GetName();
				//		std::string strEntityName = kEntityName;
				//		std::string strEntityPath = kKfmPath;
				//		NiActorManager* pMapActor = GetSettedKfmObject(strEntityName, strEntityPath);
				//		
				//		//NiAVObject* pkNifRoot = pMapActor->GetNIFRoot();
				//		//pkNifRoot->SetTranslate(pkSceneRoot->GetTranslate());
				//		//pkNifRoot->SetRotate(pkSceneRoot->GetRotate());
				//		//pkNifRoot->SetWorldTranslate(pkSceneRoot->GetWorldTranslate());
				//		//pkNifRoot->SetWorldRotate(pkSceneRoot->GetWorldRotate());
				//		bool bRtn = pMapActor->ChangeNIFRoot(pkSceneRoot);
				//		//pkEntity->SetPropertyData("Scene Root Pointer", pkNifRoot, 0);

				//		//pkSceneRoot = pkNifRoot;

				//		pkSceneRoot->UpdateProperties();
				//		pkSceneRoot->Update(0.0f);

				//		unsigned int unSequenceID;
				//		pkEntity->GetPropertyData("Active Sequence ID", unSequenceID, unIndex);

				//		NiControllerSequence* pControllerSequence = pMapActor->GetSequence(unSequenceID);
				//		if(pControllerSequence)
				//		{
				//			pControllerSequence->SetCycleType(NiTimeController::LOOP);
				//			pMapActor->SetTargetAnimation(unSequenceID);
				//		}

				//		pMapActor->Update(0.0f);
				//	}
				//}
				
				if (!bIsAfterAttach)
				{
					m_spSceneRoot->AttachChild(pkSceneRoot, true);
				}
			}
		}
	}
	pkAssetManager = 0;

    //m_kSceneSpacePartition.Build( m_spSceneRoot, m_pkStaticNodeRoot ); // "physx" 이름규칙 때문에 버그 발생해서 임시 주석처리함 - Adrian

#ifdef PG_USE_LOADGSA_PERFORMCHECK
	PG_STAT(timerF.Start());
#endif
	if (m_spPhysXScene)
	{
		m_spPhysXScene->UpdateSources(0);
	}

	// Transform이 변경 되었기 때문에 Update
	m_spSceneRoot->UpdateNodeBound();
	m_spSceneRoot->UpdateProperties();
	m_spSceneRoot->UpdateEffects();
	m_spSceneRoot->Update(0.0f);

#ifndef EXTERNAL_RELEASE
    //NewWare::Scene::OutputDebugSceneWorldBound( m_pkStaticNodeRoot );
#endif //#ifndef EXTERNAL_RELEASE

	// Affected로 설정 된 물체들에게 라이트를 적용시켜 준다.
	//	SAFE_DELETE_NI(m_spLightRoot);	//	A smart pointer should not be deleted by using NiDelete
	m_spLightRoot = NiNew NiNode;
	m_spLightRoot->SetName("lights");

	// BaseObject 가 있으면 기본 LoadNif 라이트 세팅을 이용한다.
#ifdef PG_USE_LOADGSA_BASE
	if (bFindBase)
	{
		NiTPointerList<NiNodePtr> kStack;
		kStack.AddTail((NiNode*)m_spSceneRoot);

		while(!kStack.IsEmpty())
		{
			NiNodePtr spNode = kStack.RemoveTail();
			for(unsigned int index = 0; index < spNode->GetArrayCount(); ++index)
			{
				NiAVObject *pkObject = spNode->GetAt(index);
				if (!pkObject)
				{
					continue;
				}
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
#endif
	{
		NiEntityInterfaceContainer::iterator light_itr = kMapLight.begin();
		while(light_itr != kMapLight.end())
		{
			NiEntityInterface *pkEntity = light_itr->second;
			NiObject* pkSceneRootPointer = NULL;
			NiAVObject* pkSceneRoot = NULL;

			if (pkEntity->GetPropertyData("Scene Root Pointer", pkSceneRootPointer, 0))
			{
				pkSceneRoot = NiDynamicCast(NiAVObject, pkSceneRootPointer);
			}

			if (NiIsKindOf(NiLight, pkSceneRoot))
			{
				NiLight *pkLight = NiDynamicCast(NiLight, pkSceneRoot);
				pkLight->SetName(pkEntity->GetName());

				unsigned int iComponentCount = pkEntity->GetComponentCount();
				for (unsigned ucc = 0 ; ucc < iComponentCount ; ucc++)
				{
					NiEntityComponentInterface *pkComponentInter = pkEntity->GetComponentAt(ucc);

					if (pkComponentInter->GetClassName().Equals("NiLightComponent"))
					{
						NiLightComponent *pkLightComponent = (NiLightComponent *)pkComponentInter;
						int iAffectedCount = pkLightComponent->GetAffectedEntitiesCount();
						for (int iAffected=0 ; iAffected<iAffectedCount ; iAffected++)
						{
							NiEntityInterface* kAffectedEntity
								= pkLightComponent->GetAffectedEntityAt(iAffected);
							if (kAffectedEntity)
							{
								std::string strEntityName = kAffectedEntity->GetName();
								NodeContainer::iterator itor;

								if(strEntityName == "Van_testswich1001_01 01")
								{
									int ppp=0;
								}

								itor = kMapObjForLight.find(strEntityName);
								if (itor != kMapObjForLight.end())
								{
									NiNode *pkAffectedNode = itor->second;
									//	When the type of pkLight is not Ambient Light 
									//	and the version of vertex shader that the video card supports is less than 3.0,
									//	the maximum number of light that could affect each object is limited to MAX_LIGHT_FOR_OBJECT

									if(uiVSVersion>=D3DVS_VERSION(3,0) ||
										pkLight->GetEffectType() == NiDynamicEffect::AMBIENT_LIGHT ||
										PgRenderer::CountLight(pkAffectedNode)<MAX_LIGHT_FOR_OBJECT)
									{
										pkLight->AttachAffectedNode(pkAffectedNode);
									}
								}
							}
						}
					}
				}
				// insert
				if(pkLight->GetAffectedNodeList().GetSize()>0)
				{
					m_spLightRoot->AttachChild(pkLight, true);
				}
			}
			++light_itr;
		}
	}
	// End of insert affected object
	m_spSceneRoot->AttachChild(m_spLightRoot, true);
	//SetGeometryShader((NiAVObject*)m_spSceneRoot);

	for (unsigned int iError = 0; iError < kDefaultErrorHandler.GetErrorCount(); iError++)
	{
		NILOG(PGLOG_WARNING, "[PgWorld] LoadGsa %d error, %s,%s,%s,%s\n", iError,kDefaultErrorHandler.GetEntityName(iError), kDefaultErrorHandler.GetPropertyName(iError), kDefaultErrorHandler.GetErrorMessage(iError), kDefaultErrorHandler.GetErrorDescription(iError));
	}

	// 월드에 물리 시스템을 설정한다.
	if(!LoadPhysX(true))
	{
		PG_ASSERT_LOG(!"failed to loading physX");
		return false;
	}

	for (unsigned int iErrorCount = 0; iErrorCount < kDefaultErrorHandler.GetErrorCount(); iErrorCount++)
	{
		NILOG(PGLOG_MINOR, "PgWorld:LoadGsa Error %d, %s,%s,%s,%s", iErrorCount, kDefaultErrorHandler.GetEntityName(iErrorCount), kDefaultErrorHandler.GetPropertyName(iErrorCount), kDefaultErrorHandler.GetErrorMessage(iErrorCount), kDefaultErrorHandler.GetErrorDescription(iErrorCount));
	}

#ifdef PG_LOADGSA_OUT_TESTNIF
	// Nif Output for Test
	NiStream kOutStream;
	kOutStream.InsertObject(m_spSceneRoot);
	//kOutStream.InsertObject(m_spPhysXScene);
	kOutStream.Save("D:/asdf.nif");
#endif

	bool	bSelectiveUpdate =true;
	bool	bRigid = true;

	GetSelectiveNodeRoot()->SetSelectiveUpdateFlags(bSelectiveUpdate,true,bRigid);

    NewWare::Scene::ApplyTraversal::Geometry::SetShaderConstantUpdateOptimizeFlag( GetStaticNodeRoot(), true, true );
	if(GetPSRoomGroupRoot())
	{
		GetPSRoomGroupRoot()->SetShaderConstantUpdateOptimizeFlag(true);
	}

	if (!CheckRootNode(m_spSceneRoot))
	{
		return false;
	}
	
#ifdef PG_USE_LOADGSA_PERFORMCHECK
	PG_STAT(timerF.Stop());
	PG_STAT(g_kLoadGsaStatGroup.PrintStatGroup());
#endif

#ifdef PG_USE_LOADGSA_DIFF_PERF
	PG_STAT(g_kLoadGsaStatGroup.PrintStatGroup());
#endif
	m_spSceneRoot->UpdateNodeBound();
	m_spSceneRoot->UpdateProperties();
	m_spSceneRoot->UpdateEffects();
	m_spSceneRoot->Update(0.0f);

	return true;
}
