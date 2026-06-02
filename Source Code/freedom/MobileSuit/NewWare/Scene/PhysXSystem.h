
#ifndef _PHYSXSYSTEM_H__
#define _PHYSXSYSTEM_H__

#define _ENABLE_PHYSXSYSTEM_STOPWATCH_


class PhysXSystem
{
public:
    typedef void (*InputDeviceHook)(void);


public:
    explicit PhysXSystem( DWORD dwMainThreadID = ::GetCurrentThreadId() );
    ~PhysXSystem();


    void Create( bool bUseThread );
    void Destroy();
    void Update( float fAccumTime, InputDeviceHook pfnHook );

    void SetUpdateSpeed( float fSpeed, float fTimeStep, unsigned int uiMaxIter, bool bFixed );
    float GetUpdateSpeed() const { return m_fUpdateSpeed; };


public:
    bool ThrowRay( NiPoint3 const& kStart, NiPoint3 const& kDir, float fDistance, NiPoint3& kHitPoint ) const;
    NxShape* ThrowRayObject( NiPoint3 const& kStart, NiPoint3 const& kDir, float fRange, 
                             unsigned int uiGroup, NiPoint3& kImpact, NiPoint3& kCenter ) const;


    unsigned int OverlapCheckAtSphere( NiPoint3 const& kStart, float fRange, unsigned int uiGroup, 
                                       unsigned int uiShapesSize, NxShape** ppkShapes ) const { 
        return OverlapSphereShapes( NxSphere(NxVec3(kStart.x, kStart.y, kStart.z), fRange), 
                                    NX_DYNAMIC_SHAPES, uiShapesSize, ppkShapes, NULL, uiGroup, NULL, true );
    }
    unsigned int OverlapCheckAtBox( NiPoint3 const& kStart, NiPoint3 const& kDir, 
                                    float fEdgeLength, float fRange, unsigned int uiGroup, 
                                    unsigned int uiShapesSize, NxShape** ppkShapes ) const;
    unsigned int OverlapCheckAtHitPoint( NiPoint3 const& kStart, NiPoint3 const& kDir, float fRadius, 
                                unsigned int uiGroup, unsigned int uiShapesSize, NxShape** ppkShapes ) const;


    NxShape* RaycastClosestShape( NxRay const& kWorldRay, NxShapesType eShapeType, NxRaycastHit& kHit, 
                    NxU32 uiGroups = 0xffffffff, NxReal fMaxDist = NX_MAX_F32, NxU32 uiHintFlags = 0xffffffff, 
                    NxGroupsMask const* pkGroupsMask = NULL, NxShape** ppkCache = NULL ) const;
    unsigned int RaycastAllShapes( NxRay const& kWorldRay, NxUserRaycastReport& kReport, 
                                   NxShapesType eShapesType, NxU32 uiGroups, NxReal fMaxDist, 
                                   NxU32 uiHintFlags, NxGroupsMask const* pkGroupsMask ) const;

    unsigned int OverlapSphereShapes( NxSphere const& kWorldSphere, NxShapesType eShapeType, NxU32 nbShapes, 
                    NxShape** ppkShapes, NxUserEntityReport<NxShape*>* pkCallback, NxU32 uiActiveGroups, 
                    NxGroupsMask const* pkGroupsMask, bool bAccurateCollision ) const;
    unsigned int OverlapCapsuleShapes( NxCapsule const& kWorldCapsule, NxShapesType eShapeType, NxU32 nbShapes, 
                    NxShape** ppkShapes, NxUserEntityReport<NxShape*>* pkCallback, 
                    NxU32 uiActiveGroups, NxGroupsMask const* pkGroupsMask, bool bAccurateCollision ) const;
    unsigned int OverlapOBBShapes( NxBox const& kWorldBox, NxShapesType eShapeType, NxU32 nbShapes, 
                    NxShape** ppkShapes, NxUserEntityReport<NxShape*>* pkCallback, 
                    NxU32 uiActiveGroups, NxGroupsMask const* pkGroupsMask, bool bAccurateCollision ) const;


public:
#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
    void DumpState( std::string const& strID, std::string& strDump );
#endif //#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_


protected:
    NxScene* GetSafePhysXScene() const
    {
        assert( m_spPhysXScene );
        NxScene* pkPxScene = m_spPhysXScene->GetPhysXScene();
        if ( NULL == pkPxScene ) { 
            assert( 0 );
            return NULL;
        }
        if ( m_dwMainThreadID != ::GetCurrentThreadId() ) { // it's allowed only to mainthread.
            assert( 0 );
            return NULL;
        }
        return pkPxScene;
    }

    void Lock();
    void Unlock();


private:
    DWORD m_dwMainThreadID;
    NiPhysXScenePtr m_spPhysXScene;

    float m_fGravity;
    float m_fUpdateSpeed;

#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
    struct StopWatch;
    StopWatch* m_pkStopWatch;
#endif //#ifdef _ENABLE_PHYSXSYSTEM_STOPWATCH_
};


#endif //_PHYSXSYSTEM_H__
