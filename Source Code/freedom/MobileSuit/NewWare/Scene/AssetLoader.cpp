
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : AssetLoader.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/21 LeeJR First Created
//

#include "stdafx.h"
#include "AssetLoader.h"

#include "DomainNodeManager.h"

#include "../Renderer/DrawUtils.h"

#include "../../PgSoundMan.h"
#include "../../PgPSRoomGroup.h"


namespace NewWare
{

namespace Scene
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

namespace { 
    char const* const COMPONENT_NAME_AMBIENCE_SOUND = "Sound Object";
    char const* const COMPONENT_NAME_ACTOR = "Actor";
    char const* const COMPONENT_NAME_SHARED_STREAM = "Shared Stream";

    char const* const COMPONENT_NAME_ACTORXML_PATH = "Actor Xml Path";

    char const* const COMPONENT_NAME_PHYSX_PLAYERONLY = "PhysX_PlayerOnly";
} //namespace

//-----------------------------------------------------------------------------------

AssetLoader::AssetLoader( DomainNodeManager* pkDomainNodeManager )
    : m_pkDomainNodeManager(pkDomainNodeManager)
{
    assert( m_pkDomainNodeManager );
}

//-----------------------------------------------------------------------------------

AssetLoader::~AssetLoader()
{
    Destroy();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

bool AssetLoader::Create( NiEntityStreaming* pkStream )
{
    if ( NULL == pkStream )
    {
        assert( 0 );
        return false;
    }

    NiScene* pkScene = pkStream->GetSceneAt( 0 );
    if ( NULL == pkScene )
    {
        assert( 0 );
        return false;
    }

    Destroy();

    NiExternalAssetManagerPtr spAssetManager = 
                                    NiNew NiExternalAssetManager( NiFactories::GetAssetFactory() );
    assert( spAssetManager );

    for ( unsigned int ui = 0; ui < pkScene->GetEntityCount(); ++ui )
	{
        NiEntityInterface* pkEntity = pkScene->GetEntityAt( ui );
        if ( NULL != pkEntity )
            CreateEntity( pkEntity, spAssetManager );
	}
    return true;
}

//-----------------------------------------------------------------------------------

void AssetLoader::Destroy()
{
    m_spLightRoot = 0;
    m_spMinimapCamera = 0;

    m_mAffectedNodeFromLight.clear();
    m_lstLightList.clear();

    m_kSharedAssets.RemoveAllAsset();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void AssetLoader::CreateEntity( NiEntityInterface* pkEntity, NiExternalAssetManager* pkAssetManager )
{
    assert( pkEntity && pkAssetManager );

    if ( LoadAmbienceSound(pkEntity) )
        return;

    PropertyData kPropData;
    ReadPropertyData( pkEntity, kPropData );

	if ( kPropData.kNIFPath.Exists() )
        LoadEntityFromNIFData( kPropData );
	else
        LoadEntityFromWidgetData( kPropData, pkEntity, pkAssetManager );
}

//-----------------------------------------------------------------------------------

bool AssetLoader::LoadAmbienceSound( NiEntityInterface* pkEntity )
{
    assert( pkEntity );

    NiEntityComponentInterface* pkComponent = 
                    AssetUtils::GSA::GetComponentByName( pkEntity, COMPONENT_NAME_AMBIENCE_SOUND );
    if ( NULL == pkComponent )
        return false;

    NiPoint3 kSoundPos = NiPoint3::ZERO;
    NiFixedString kMediaPath = "";
    float fVolume = 0.0f;
    float fAffectRange = 0.0f;
    float fAffectAtten = 0.0f;
    bool bRandom = false, bMixBGSound = false;
    float fProbability = 0.0f;

    pkEntity->GetPropertyData( "Translation", kSoundPos );
    pkEntity->SetHidden( true );

    pkComponent->GetPropertyData( "Media Path", kMediaPath );
    pkComponent->GetPropertyData( "Volume", fVolume );
    pkComponent->GetPropertyData( "Affect Range", fAffectRange );
    pkComponent->GetPropertyData( "Affect Attenuation", fAffectAtten );
    pkComponent->GetPropertyData( "Random Play", bRandom );
    pkComponent->GetPropertyData( "Random Probability", fProbability );
	pkComponent->GetPropertyData( "Mix BGSound", bMixBGSound );

    // 맵툴에서 저장된 사운드의 절대 경로를 클라이언트 기준의 리소스경로로 다시 맞춘다.
    // <맞추는 방법>
    //	1. 환경사운드 리소스는 무조건 게임폴더\Sound\에 들어간다.
    //	2. GSA에서 읽은 리소스 절대경로에서 \\sound\\문자열을 찾는다.
    //	3. GSA에서 읽은 리소스 절대경로에서 2에서 찾은 위치 앞의 내용은 모두 지운다.
    //	4. 3에서 구해진 결과에 ..\을 앞부분에 추가하여 실행파일 한단계 위의 경로임을 명시한다.
    std::string strRelativePath = static_cast<char const*>(kMediaPath);
    strRelativePath.erase( 0, strRelativePath.rfind("\\Sound\\") );
    strRelativePath = ".." + strRelativePath;
    kMediaPath = strRelativePath.c_str();

    g_kSoundMan.AddAndPlayEnvSound( kMediaPath, fVolume, fAffectAtten, fAffectRange, kSoundPos, 
                                    bRandom, fProbability, bMixBGSound );
    return true;
}

//-----------------------------------------------------------------------------------

void AssetLoader::ReadPropertyData( NiEntityInterface* pkEntity, PropertyData& kPropData )
{
    assert( pkEntity );

    kPropData.kEntityName = pkEntity->GetName();
    kPropData.bUsePhysX   = pkEntity->GetPGUsePhysX();
    kPropData.bIsTrigger  = (pkEntity->GetPGProperty().Equals("Trigger") || pkEntity->GetPGProperty().Equals("Trap"));

    if ( pkEntity->GetPropertyData(COMPONENT_NAME_PHYSX_PLAYERONLY, kPropData.bOnlyPlayerPhysX) == false )
    {
        kPropData.bOnlyPlayerPhysX = false;
    }

    NiEntityComponentInterface* pkSharedStreamCompo = 
                        AssetUtils::GSA::GetComponentByName( pkEntity, COMPONENT_NAME_SHARED_STREAM );
    if ( pkSharedStreamCompo )
    {
        pkSharedStreamCompo->GetPropertyData( "Use Shared Stream", kPropData.bUseSharedStream );
    }

    pkEntity->GetPropertyData( "NIF File Path", kPropData.kNIFPath, kPropData.uiIndex );
}

//-----------------------------------------------------------------------------------

void AssetLoader::LoadEntityFromNIFData( PropertyData const& kPropData )
{
#if 0
    std::string strPath = kPropData.kNIFPath;
    NiNodePtr spNifRoot = 0;

    NiStream kUniqueStream;
    NiStream* pkShareStream = NULL;
    bool bShareStream = false;

    // 만약 트리거가 있다면 Share를 하지 않고 그냥 로드 한다.
    if (kPropData.bUsePhysX || kPropData.bIsTrigger || !kPropData.bUseSharedStream)
    {
        if (!kUniqueStream.Load(kPropData.kNIFPath))
        {
            PgError3( "[PgWorld] Load %s nif file faild(UsePhysX:%d, Trigger:%d)\n", 
                      kPropData.kNIFPath, kPropData.bUsePhysX, kPropData.bIsTrigger );
            return;
        }
    }

    // 트리거이거나 Component상에 SharedStream을 사용치 않도로고 명시한 경우 Share를 하지 않는다.
    if (kPropData.bIsTrigger || !kPropData.bUseSharedStream)
    {
        pkShareStream = &kUniqueStream;
    }
    else
    {
        bShareStream = true;
        std::string strPath = kPropData.kNIFPath;
        pkShareStream = m_kSharedAssets.GetAsset( strPath );
    }

    if (bShareStream && pkShareStream == NULL)
    {
        NILOG(PGLOG_ERROR, "[PgWorld] Load %s file faild\n", strPath.c_str());
        continue;
    }

    if (pkShareStream->GetObjectCount() == 0)
    {
        NILOG(PGLOG_ERROR, "[PgWorld] Load %s file but no object\n", strPath.c_str());
        continue;
    }

    NiNodePtr spDefRoot = 0;
    spDefRoot = NiDynamicCast(NiNode, pkShareStream->GetObjectAt(0));

    if(!spDefRoot)
    {
        PG_ASSERT_LOG(!"no root node");
        return false;
    }

    bool bUseLOD = false;
    if (!kPropData.bUsePhysX)
    {
        NiNodePtr pkLowRoot = NULL;
        NiNodePtr pkMidRoot = NULL;
        NiStream* pkLowStream = m_kSharedAssets.GetAssetAsLOD( strPath, AssetManager::LOD_POSTFIX_LOW );
        NiStream* pkMidStream = m_kSharedAssets.GetAssetAsLOD( strPath, AssetManager::LOD_POSTFIX_MIDDLE );

        if (pkLowStream && pkLowStream->GetObjectCount() > 0)
        {
            pkLowRoot = NiDynamicCast(NiNode, pkLowStream->GetObjectAt(0));
            if (pkLowRoot)
            {
                pkLowRoot->SetName((strEntityName + AssetManager::LOD_POSTFIX_LOW).c_str());
                bUseLOD = true;
            }					
        }

        if (pkMidStream && pkMidStream->GetObjectCount() > 0)
        {
            pkMidRoot = NiDynamicCast(NiNode, pkMidStream->GetObjectAt(0));
            if (pkMidStream)
            {
                pkMidRoot->SetName((strEntityName + AssetManager::LOD_POSTFIX_MIDDLE).c_str());
                bUseLOD = true;
            }
        }

        if (bUseLOD == false)
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
            if (iAlphaGroup >= -5 && iAlphaGroup <= 5)
                Scene::ApplyTraversal::Property::SetAlphaGroup( spNifRoot, iAlphaGroup );
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
        if (kPropData.bIsTrigger || bUseLOD)
        {
            pkChildRootNode = spNifRoot;
        }
        else
        {
            pkChildRootNode = NiDynamicCast(NiNode, spNifRoot->Clone());
        }
        m_mAffectedNodeFromLight.insert(std::make_pair(kEntityName, pkChildRootNode));
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
            SetTextureToNode(strPostfixTexture, pkChildRootNode);
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
        if (kPropData.bUsePhysX)
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
                        NiPhysXSceneDesc *pkDesc = pkPhysXObject->GetSnapshot();
                        // 스케일이 1이 아닐경우. ReCook을 한다.
                        if (fPhysXScale != 1.0f)
                        {
                            SetScaleToPhysXObject(pkDesc, fPhysXScale);
                        }
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
                                    _itoa_s((int)(fScale*10000), szCount, 10);
                                    strDescName += szCount;
                                    strDescName += "_";
                                    strDescName += kPropData.kNIFPath;
                                    NiFixedString strDescName_ = strDescName.MakeExternalCopy();
                                    pkShapeDesc->GetMeshDesc()->SetName(strDescName_);
                                }
                            }
                        }

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

                    if ( kPropData.bOnlyPlayerPhysX )
                    {
                        SetPhysXGroup(pkPhysXObject, PG_PHYSX_GROUP_PLAYER_WALL);
                    }
                }
            }
        }

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
        //if (kPropData.bUsePhysX == false)
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
#endif
}

//-----------------------------------------------------------------------------------

void AssetLoader::LoadEntityFromWidgetData( PropertyData const& kPropData, NiEntityInterface* pkEntity, 
                                            NiExternalAssetManager* pkAssetManager )
{
    assert( pkEntity && pkAssetManager && m_pkDomainNodeManager );

    if ( CheckEntityForAttributeData(pkEntity) )
        return;

    pkEntity->Update( pkEntity->GetMasterEntity(), 0, &AssetUtils::GSA::GetDefaultErrorHandler(), pkAssetManager );

    NiObject* pkSceneRoot = NULL;
    if ( pkEntity->GetPropertyData("Scene Root Pointer", pkSceneRoot, kPropData.uiIndex) == false )
        return;

    if ( NiIsKindOf(NiCamera, pkSceneRoot) )
    {
        NiCamera* pkCamera = static_cast<NiCamera*>(pkSceneRoot);

        if ( pkEntity->GetPGProperty().Equals("MainCamera") )
            BuildDefaultCamera( pkEntity, pkCamera );
        else if ( pkEntity->GetPGProperty().Equals("MinimapCamera") )
            BuildMinimapCamera( pkEntity, pkCamera );

        m_pkDomainNodeManager->AttatchToSceneRoot( pkCamera, true, false );
    }
    else if ( NiIsKindOf(NiLight, pkSceneRoot) )
    {
        BuildLight( pkEntity );
    }
    else
    {
        NiAVObject* pkAVObject = NiDynamicCast(NiAVObject, pkSceneRoot);
        if ( pkAVObject )
            m_pkDomainNodeManager->AttatchToSceneRoot( pkAVObject, true, false );
    }
}

//-----------------------------------------------------------------------------------

bool AssetLoader::CheckEntityForAttributeData( NiEntityInterface* pkEntity )
{
    assert( m_pkDomainNodeManager && pkEntity );

    // Is this a portal system entity?
    if ( m_pkDomainNodeManager->GetRoomGroupNodeRoot()->AddEntity(pkEntity) )
        return true;

    if ( BuildActorEntity(pkEntity) )
        return true;

    return false;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void AssetLoader::BuildDefaultCamera( NiEntityInterface* pkEntity, NiCamera* pkCamera )
{
    assert( pkEntity && pkCamera );
	if(!g_pkWorld)
	{
		return;
	}
    NiCameraPtr spCamera = pkCamera;
    //spCamera->SetName( "main_camera" );

    // Culling 거리 조절
    NiFrustum kFrustum = spCamera->GetViewFrustum();
    kFrustum.m_fNear   = 100.0f;
    kFrustum.m_fFar    = 20000.0f;
    kFrustum.m_fLeft   = -0.51428568f;
    kFrustum.m_fRight  =  0.51428568f;
    kFrustum.m_fTop    =  0.38571426f;
    kFrustum.m_fBottom = -0.38571426f;
    spCamera->SetViewFrustum( kFrustum );

    g_kFrustum = kFrustum;

    // 카메라를 카메라맨에 등록한다.
    g_pkWorld->GetCameraMan()->SetCamera( spCamera );
    g_pkWorld->GetCameraMan()->AddCamera( (NiString)(pkEntity->GetName()), spCamera );
    g_pkWorld->GetCameraMan()->SetCameraMode( PgCameraMan::CMODE_NONE, 0 );

    // 카메라를 씬에 등록한다.
    NiPoint3 kLoc  = spCamera->GetWorldTranslate();
    NiMatrix3 kRot = spCamera->GetWorldRotate();
    spCamera->SetTranslate( kLoc );
    spCamera->SetRotate( kRot );
}

//-----------------------------------------------------------------------------------

void AssetLoader::BuildMinimapCamera( NiEntityInterface* pkEntity, NiCamera* pkCamera )
{
    assert( pkEntity && pkCamera );

    m_spMinimapCamera = pkCamera;
    m_spMinimapCamera->SetName( "minimap_camera" );

    NiFrustum kFrustum = m_spMinimapCamera->GetViewFrustum();
    //kFrustum.m_bOrtho = true;
    kFrustum.m_fNear = 1.0f;
    kFrustum.m_fFar = 100000.0f;
    m_spMinimapCamera->SetViewFrustum( kFrustum );
}

//-----------------------------------------------------------------------------------

void AssetLoader::BuildLight( NiEntityInterface* pkEntity )
{
    assert( pkEntity );

    // 라이트는 찾아서 'Entity로딩 후 라이트 적용'을 위해 컨테이너에 저장해 둠.
    m_lstLightList.push_back( pkEntity );
}

//-----------------------------------------------------------------------------------

bool AssetLoader::BuildActorEntity( NiEntityInterface* pkEntity )
{
    assert( pkEntity );
	if(!g_pkWorld)
	{
		return false;
	}
    NiActorComponent* pkActorComponent = 
        static_cast<NiActorComponent*>(AssetUtils::GSA::GetComponentByName(pkEntity, COMPONENT_NAME_ACTOR));
    if ( pkActorComponent )
    {
        bool const bUsePhysX  = pkEntity->GetPGUsePhysX();
        bool const bIsTrigger = pkEntity->GetPGProperty().Equals("Trigger") || pkEntity->GetPGProperty().Equals("Trap");

        NiTransform	kTransform;
        pkEntity->GetPropertyData( "Translation", kTransform.m_Translate );
        pkEntity->GetPropertyData( "Rotation", kTransform.m_Rotate );
        pkEntity->GetPropertyData( "Scale", kTransform.m_fScale );

        NiFixedString kKFMFilePath;
        pkEntity->GetPropertyData( "KFM File Path", kKFMFilePath );
        unsigned int uiActiveSeq = 0;
        pkEntity->GetPropertyData( "Active Sequence ID", uiActiveSeq );

        NiFixedString kActorXMLPath;
        PgPuppet* pkPuppet = NULL;
        if ( pkEntity->GetPropertyData(COMPONENT_NAME_ACTORXML_PATH, kActorXMLPath) )
        {
            pkPuppet = g_pkWorld->AddPuppet_ActorXML( (char const*)pkEntity->GetName(), 
                                    (char const*)kActorXMLPath, uiActiveSeq, kTransform, bUsePhysX, bIsTrigger );
        }
        else
        {
            pkPuppet = g_pkWorld->AddPuppet_KFMPath( (char const*)pkEntity->GetName(), 
                                    (char const*)kKFMFilePath, uiActiveSeq, kTransform, bUsePhysX, bIsTrigger );
        }
        return	true;
    }
    return	false;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

void AssetLoader::ApplyLightsToAffectedNodes()
{
    for ( AssetUtils::GSA::NiEntityInterfaceListCIter citer = m_lstLightList.begin(); 
          citer != m_lstLightList.end(); ++citer )
    {
        NiEntityInterface* pkEntity = (*citer);

        NiObject* pkSceneRoot = NULL;
        if ( pkEntity->GetPropertyData("Scene Root Pointer", pkSceneRoot, 0) == false )
            continue;

        if ( NiIsKindOf(NiLight, pkSceneRoot) )
        {
            NiLight* pkLight = static_cast<NiLight*>(pkSceneRoot);
            pkLight->SetName( pkEntity->GetName() );

            for ( unsigned int ui = 0; ui < pkEntity->GetComponentCount(); ++ui )
            {
                ApplyLightToAffectedEntities( pkLight, pkEntity->GetComponentAt(ui) );
            }

            if ( pkLight->GetAffectedNodeList().GetSize() > 0 )
            {
                m_spLightRoot->AttachChild( pkLight, true );
            }
        }
    }
    m_lstLightList.clear();
}

//-----------------------------------------------------------------------------------

void AssetLoader::ApplyLightToAffectedEntities( NiLight* pkLight, NiEntityComponentInterface* pkComponent )
{
    assert( pkLight && pkComponent );

    if ( pkComponent->GetClassName().Equals("NiLightComponent") )
    {
        NiLightComponent* pkLightComponent = static_cast<NiLightComponent*>(pkComponent);
        for ( unsigned int ui = 0; ui < pkLightComponent->GetAffectedEntitiesCount(); ++ui )
        {
            NiEntityInterface* kAffectedEntity = pkLightComponent->GetAffectedEntityAt( ui );
            if ( kAffectedEntity )
            {
                AssetUtils::NIF::NiNodeMapCIter citer = 
                            m_mAffectedNodeFromLight.find( (char const*)kAffectedEntity->GetName() );
                if ( citer != m_mAffectedNodeFromLight.end() )
                    AttachAffectedNodeToLight( pkLight, citer->second );
            }
        }
    }
}

//-----------------------------------------------------------------------------------

void AssetLoader::AttachAffectedNodeToLight( NiLight* pkLight, NiNode* pkAffectedNode )
{
    assert( pkLight && pkAffectedNode );

    // When the type of pkLight is not Ambient Light 
    //  and the version of vertex shader that the video card supports is less than 3.0, 
    //  the maximum number of light that could affect each object is limited to MAX_LIGHT_FOR_OBJECT
    if ( ((NiDX9Renderer*)NiRenderer::GetRenderer())->GetVertexShaderVersion() >= D3DVS_VERSION(3,0) || 
         pkLight->GetEffectType() == NiDynamicEffect::AMBIENT_LIGHT || 
         Renderer::DrawUtils::GetLightCountByAffectedObject(pkAffectedNode) < MAX_LIGHT_FOR_OBJECT )
    {
        pkLight->AttachAffectedNode( pkAffectedNode );
    }
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Scene

} //namespace NewWare
