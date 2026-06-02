
#include "stdafx.h"
#include "PhysXSystem.h"


#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
struct PhysXSystem::StopWatch
{
    StopWatch() : fSimulationTime(0.0f), dwSimulationCount(0), fStartTime(0.0f) { /* Nothing */ };

    void Start()
    {
        fStartTime = NiGetCurrentTimeInSec();
    }
    void Stop()
    {
        fSimulationTime += (NiGetCurrentTimeInSec() - fStartTime);
        ++dwSimulationCount;
    }

    float fSimulationTime;
    DWORD dwSimulationCount;

    float fStartTime;
};
#endif //#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_



PhysXSystem::PhysXSystem( DWORD dwMainThreadID )
    : m_dwMainThreadID(dwMainThreadID), 
      m_fGravity(-490.0f), 
      m_fUpdateSpeed(1.0f) 
#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
      , m_pkStopWatch(new StopWatch)
#endif //#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
{
    /* Nothing */
}

PhysXSystem::~PhysXSystem()
{
    Destroy();

#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
    delete m_pkStopWatch;
#endif //#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
}


void PhysXSystem::Create( bool bUseThread )
{
    assert( 0 == m_spPhysXScene );

    m_spPhysXScene = NiNew NiPhysXScene;

    NxSceneDesc kSceneDesc;
    kSceneDesc.gravity = NxVec3(0.0f, 0.0f, m_fGravity);
    kSceneDesc.simType = NX_SIMULATION_SW;
    if ( bUseThread )
    {
        //kSceneDesc.backgroundThreadCount = PgComputerInfo::GetCPUInfo().iNumProcess;
        kSceneDesc.internalThreadCount = PgComputerInfo::GetCPUInfo().iNumProcess;
        kSceneDesc.threadMask = 0xfffffffe;
        kSceneDesc.flags |= NX_SF_ENABLE_MULTITHREAD;
    }

    NxScene* pkNxScene = NiPhysXManager::GetPhysXManager()->m_pkPhysXSDK->createScene( kSceneDesc );
    assert( pkNxScene );
    m_spPhysXScene->SetPhysXScene( pkNxScene );
}

void PhysXSystem::Destroy()
{
    if ( m_spPhysXScene )
    {
        Lock();

        //m_spPhysXScene->FetchResults( NI_INFINITY, true );
        if ( m_spPhysXScene->GetSnapshot() )
            m_spPhysXScene->ClearSceneFromSnapshot();
        m_spPhysXScene->ReleaseSnapshot();
        m_spPhysXScene->ReleaseScene();

        Unlock();
    }
    m_spPhysXScene = 0;
}



void PhysXSystem::Update( float fAccumTime, InputDeviceHook pfnHook )
{
    assert( m_spPhysXScene && pfnHook );

#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
    m_pkStopWatch->Start();
#endif //#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_

    m_spPhysXScene->UpdateSources( fAccumTime, false ); // Gamebryo -> PhysX sync.

    Lock();
        m_spPhysXScene->Simulate( fAccumTime );
        m_spPhysXScene->FetchResults( fAccumTime, true );

        pfnHook();

        m_spPhysXScene->UpdateDestinations( fAccumTime, false ); // PhysX -> Gamebryo sync.
    Unlock();

#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
    m_pkStopWatch->Stop();
#endif //#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
}



void PhysXSystem::SetUpdateSpeed( float fSpeed, float fTimeStep, unsigned int uiMaxIter, bool bFixed )
{
    if ( 0.0f == fSpeed )
        fSpeed = 0.0001f;
    m_fUpdateSpeed = fSpeed;

    fTimeStep = fTimeStep * (1.0f/m_fUpdateSpeed);
    if ( fTimeStep > 0.0f )
        fTimeStep = 1.0f / fTimeStep;

    m_spPhysXScene->SetTimestep( fTimeStep );
    m_spPhysXScene->SetDoFixedStep( bFixed );
    m_spPhysXScene->GetPhysXScene()->setTiming( fTimeStep, uiMaxIter, 
                                                bFixed? NX_TIMESTEP_FIXED: NX_TIMESTEP_VARIABLE );
}



bool PhysXSystem::ThrowRay( NiPoint3 const& kStart, NiPoint3 const& kDir, float fDistance, NiPoint3& kHitPoint ) const
{
    assert( m_spPhysXScene && 0.0f < fDistance );

    NxRaycastHit kHit;
    NxShape* pkHitShape = 
        RaycastClosestShape( NxRay(NxVec3(kStart.x, kStart.y, kStart.z), NxVec3(kDir.x, kDir.y, kDir.z)), 
                             NX_STATIC_SHAPES, kHit, -1, NiMax(fDistance, 0.1f), NX_RAYCAST_SHAPE|NX_RAYCAST_IMPACT );
    if ( pkHitShape )
    {
        kHitPoint = NiPoint3( kHit.worldImpact.x, kHit.worldImpact.y, kHit.worldImpact.z );
        return true;
    }
    return false;
}

NxShape* PhysXSystem::ThrowRayObject( NiPoint3 const& kStart, NiPoint3 const& kDir, float fRange, 
                                      unsigned int uiGroup, NiPoint3& kImpact, NiPoint3& kCenter ) const 
{
    assert( 0.0f < fRange );

    NxRaycastHit kHit;
    NxShape* pkHitShape = 
        RaycastClosestShape(NxRay(NxVec3(kStart.x, kStart.y, kStart.z), NxVec3(kDir.x, kDir.y, kDir.z)), 
                            NX_DYNAMIC_SHAPES, kHit, 1 << (uiGroup+1), NiMax(fRange, 0.1f) );
    if ( NULL == pkHitShape || NULL == pkHitShape->userData )
        return NULL;

    NiPhysXTypes::NxVec3ToNiPoint3( kHit.worldImpact, kImpact );
    NiPhysXTypes::NxVec3ToNiPoint3( pkHitShape->getGlobalPosition(), kCenter );
    return pkHitShape;
}



unsigned int PhysXSystem::OverlapCheckAtBox( NiPoint3 const& kStart, NiPoint3 const& kDir, 
                                             float fEdgeLength, float fRange, unsigned int uiGroup, 
                                             unsigned int uiShapesSize, NxShape** ppkShapes ) const
{
    NxVec3 kCenter( kStart.x+kDir.x*fRange/2, kStart.y+kDir.y*fRange/2, kStart.z+kDir.z*fRange/2 );
    NxVec3 kExtend( fRange/2, fEdgeLength/2, fEdgeLength/2 );

    NxVec3 kDirection( kDir.x, kDir.y, kDir.z );
    NxVec3 kRight = kDirection.cross( NxVec3(0.0f, 0.0f, 1.0f) );
    if ( kRight.distance(NxVec3(0.0f, 0.0f, 0.0f)) < 0.001f )
    {
        kRight = kDirection.cross( NxVec3(0.0f, 1.0f, 0.0f) );
    }
    NxVec3 kUp = kDirection.cross( -kRight );

    NxMat33	kRot;
    kRot.setRow( 0, kDirection );
    kRot.setRow( 1, kUp );
    kRot.setRow( 2, kRight );

    return OverlapOBBShapes( NxBox(kCenter, kExtend, kRot), NX_DYNAMIC_SHAPES, 
                             uiShapesSize, ppkShapes, NULL, uiGroup, NULL, true );
}

unsigned int PhysXSystem::OverlapCheckAtHitPoint( NiPoint3 const& kStart, NiPoint3 const& kDir, 
                                                  float fRadius, unsigned int uiGroup, 
                                                  unsigned int uiShapesSize, NxShape** ppkShapes ) const
{
    NxRaycastHit kHit;
    if ( RaycastClosestShape(NxRay(NxVec3(kStart.x, kStart.y, kStart.z), NxVec3(kDir.x, kDir.y, kDir.z)), 
                             NX_STATIC_SHAPES, kHit, 1, 1000.0f, NX_RAYCAST_SHAPE) == NULL )
        return 0;

    return OverlapSphereShapes( NxSphere(kHit.worldImpact, fRadius), NX_DYNAMIC_SHAPES, uiShapesSize, 
                                ppkShapes, NULL, uiGroup, NULL, true );
}



NxShape* PhysXSystem::RaycastClosestShape( NxRay const& kWorldRay, NxShapesType eShapeType, NxRaycastHit& kHit, 
                                           NxU32 uiGroups, NxReal fMaxDist, NxU32 uiHintFlags, 
                                           NxGroupsMask const* pkGroupsMask, NxShape** ppkCache ) const
{
    NxScene* pkPxScene = GetSafePhysXScene();
    if ( pkPxScene )
        return pkPxScene->raycastClosestShape( kWorldRay, eShapeType, kHit, uiGroups, 
                                               fMaxDist, uiHintFlags, pkGroupsMask, ppkCache );
    return NULL;
}

unsigned int PhysXSystem::RaycastAllShapes( NxRay const& kWorldRay, NxUserRaycastReport& kReport, 
                                            NxShapesType eShapesType, NxU32 uiGroups, NxReal fMaxDist, 
                                            NxU32 uiHintFlags, NxGroupsMask const* pkGroupsMask ) const
{
    NxScene* pkPxScene = GetSafePhysXScene();
    if ( pkPxScene )
        return pkPxScene->raycastAllShapes( kWorldRay, kReport, eShapesType, uiGroups, fMaxDist, 
                                            uiHintFlags, pkGroupsMask );
    return 0;
}

unsigned int PhysXSystem::OverlapSphereShapes( NxSphere const& kWorldSphere, NxShapesType eShapeType, NxU32 nbShapes, 
                                        NxShape** ppkShapes, NxUserEntityReport<NxShape*>* pkCallback, 
                                        NxU32 uiActiveGroups, NxGroupsMask const* pkGroupsMask, bool bAccurateCollision ) const
{
    NxScene* pkPxScene = GetSafePhysXScene();
    if ( pkPxScene )
        return pkPxScene->overlapSphereShapes( kWorldSphere, eShapeType, nbShapes, ppkShapes, pkCallback, 
                                               uiActiveGroups, pkGroupsMask, bAccurateCollision );
    return 0;
}

unsigned int PhysXSystem::OverlapCapsuleShapes( NxCapsule const& kWorldCapsule, NxShapesType eShapeType, NxU32 nbShapes, 
                                        NxShape** ppkShapes, NxUserEntityReport<NxShape*>* pkCallback, 
                                        NxU32 uiActiveGroups, NxGroupsMask const* pkGroupsMask, bool bAccurateCollision ) const
{
    NxScene* pkPxScene = GetSafePhysXScene();
    if ( pkPxScene )
        return pkPxScene->overlapCapsuleShapes( kWorldCapsule, eShapeType, nbShapes, ppkShapes, pkCallback, 
                                                uiActiveGroups, pkGroupsMask, bAccurateCollision );
    return 0;
}

unsigned int PhysXSystem::OverlapOBBShapes( NxBox const& kWorldBox, NxShapesType eShapeType, NxU32 nbShapes, 
                                        NxShape** ppkShapes, NxUserEntityReport<NxShape*>* pkCallback, 
                                        NxU32 uiActiveGroups, NxGroupsMask const* pkGroupsMask, bool bAccurateCollision ) const
{
    NxScene* pkPxScene = GetSafePhysXScene();
    if ( pkPxScene )
        return pkPxScene->overlapOBBShapes( kWorldBox, eShapeType, nbShapes, ppkShapes, pkCallback, 
                                             uiActiveGroups, pkGroupsMask, bAccurateCollision );
    return 0;
}


void PhysXSystem::Lock()
{
    assert( m_spPhysXScene );

    NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
    pkPhysXManager->WaitSDKLock();

    while ( m_spPhysXScene->GetInSimFetch() )
    {
        NiSleep( 1 );
    }
}

void PhysXSystem::Unlock()
{
    assert( m_spPhysXScene );

    NiPhysXManager::GetPhysXManager()->ReleaseSDKLock();
}



#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
void PhysXSystem::DumpState( std::string const& strID, std::string& strDump )
{
    assert( m_spPhysXScene && m_pkStopWatch );

    NxScene* pkScene = m_spPhysXScene->GetPhysXScene();
    assert( pkScene );

    NxSceneStats kStats;
    NxSceneStats2 const* pkStats2;
    pkScene->getStats( kStats );
    pkStats2 = pkScene->getStats2();

    NILOG( PGLOG_STAT, "\n\n--------------- %s Physx statistics ---------------\n", strID.c_str());
    //NILOG( PGLOG_STAT, "\t time %f, count %d, avg %f\n", m_pkStopWatch->fSimulationTime, 
    //                                                     m_pkStopWatch->dwSimulationCount, 
    //                                                     m_pkStopWatch->fSimulationTime / 
    //                                                     (float)(m_pkStopWatch->dwSimulationCount? m_pkStopWatch->dwSimulationCount: 1) );
    //NILOG( PGLOG_STAT, "\t Contacts(%d,%d), Pairs(%d,%d), DynamicACtorsInAwakeGroups(%d,%d) AxisConstraints(%d,%d)\n", 
    //                                                     kStats.numContacts, kStats.maxContacts, kStats.numPairs, 
    //                                                     kStats.maxPairs, kStats.numDynamicActorsInAwakeGroups, 
    //                                                     kStats.maxDynamicActorsInAwakeGroups, kStats.numAxisConstraints, kStats.maxAxisConstraints );
    //NILOG( PGLOG_STAT, "\t SolverBodies(%d,%d), Actors(%d,%d), DynamicActors(%d,%d) StaticShpaes(%d,%d) DynamicShapes(%d,%d), Joints(%d,%d)\n", 
    //                                                     kStats.numSolverBodies, kStats.maxSolverBodies, kStats.numActors, 
    //                                                     kStats.maxActors, kStats.numDynamicActors, kStats.maxDynamicActors, 
    //                                                     kStats.numStaticShapes, kStats.maxStaticShapes, kStats.numJoints, kStats.maxJoints );

    if ( pkStats2 )
    {
        for ( unsigned int ui = 0; ui < pkStats2->numStats; ++ui )
        {
            if ( pkStats2->stats[ui].curValue != 0 && pkStats2->stats[ui].maxValue != 0 )
            {
                //NILOG( PGLOG_STAT, "\t %s(%d,%d)\n", 
                //    pkStats2->stats[ui].name, pkStats2->stats[ui].curValue, pkStats2->stats[ui].maxValue );
            }
        }
    }

    NILOG(PGLOG_STAT, "--------------- physx statistics end ---------------\n\n");
}
#endif //#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
