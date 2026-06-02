
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : OcclusionTimeController.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/12/08 LeeJR First Created
//

#ifndef _RENDERER_OCULLER_HW_OCCLUSIONTIMECONTROLLER_H__
#define _RENDERER_OCULLER_HW_OCCLUSIONTIMECONTROLLER_H__

#include <NiTimeController.h>
#include <NiTransform.h>
#include <NiTriShape.h>
#include <NiDebug.h>


namespace NewWare
{

namespace Renderer
{

namespace OCuller
{

namespace HW
{

namespace OcclusionFlag
{
    enum
    {
        OCCLUDER = (1 << 0), 
        OCCLUDEE = (1 << 1), 
    };
} //namespace OcclusionType


class OcclusionTimeController : public NiTimeController
{
    NiDeclareRTTI;

private:
    static const float OCCLUDER_LIMIT_SIZE;
public:
    static char const* const OCCLUDER_NAME;


public:
    OcclusionTimeController( bool bIsOccluder, NiSingleShaderMaterial* pkDepthMaterial );
    virtual ~OcclusionTimeController();


    virtual void Update( float fTime );
    virtual void SetTarget( NiObjectNET* pkTarget );


    void ResolveOcclusionType( NiCamera const* pkCamera );

    NiTriShape* GetProxy() const { return m_spProxy; };

    bool IsOccluder() const { return m_bIsOccluder; };
    bool IsMutable() const { return m_bMutable; };
    bool IsSkinned() const { return m_bIsSkinned; };


protected:
    virtual bool TargetIsRequiredType() const { return NiIsKindOf(NiGeometry, m_pkTarget); };


private:
    bool CreateProxy();
    void UpdateTransform();
    void UpdateSkinnedTransform();

    bool LessEqualThresholdSize() const
    {
        if ( m_spProxy->GetWorldBound().GetRadius() <= OCCLUDER_LIMIT_SIZE )
            return true;
        return false;
    }


private:
    NiTriShapePtr m_spProxy;
    NiTransform m_kTransform;
    bool m_bIsOccluder;
    bool m_bMutable;
    bool m_bIsSkinned;

    NiSingleShaderMaterial* m_pkDepthMaterial;
};


/////////////////////////////////////////////////////////////////////////////////////////////
//

inline OcclusionTimeController* FindOcclusionTimeController( NiAVObject const* pkAVObject )
{
    assert( pkAVObject );

    if ( pkAVObject->IsVisualObject() && !pkAVObject->GetAppCulled() )
    {
        return NiGetController(OcclusionTimeController, pkAVObject);
    }
    return NULL;
}

//-----------------------------------------------------------------------------------

inline bool IsOccluder( NiTimeController const* pkController )
{
    assert( pkTimeCtrler );

    OcclusionTimeController* pkTimeCtrler = NiDynamicCast(OcclusionTimeController, pkController);
    if ( pkTimeCtrler )
        pkTimeCtrler->IsOccluder();
    return false;
}
inline bool IsOccluder( NiAVObject const* pkAVObject )
{
    assert( pkAVObject );

    OcclusionTimeController* pkTimeCtrler = FindOcclusionTimeController( pkAVObject );
    if ( pkTimeCtrler )
        pkTimeCtrler->IsOccluder();
    return false;
}

//-----------------------------------------------------------------------------------

inline bool ResolveOcclusionType( NiAVObject* pkAVObject, NiCamera const* pkCamera )
{
    assert( pkAVObject && pkCamera );

    OcclusionTimeController* pkTimeCtrler = FindOcclusionTimeController( pkAVObject );
    if ( pkTimeCtrler )
    {
        pkTimeCtrler->ResolveOcclusionType( pkCamera );
        return true;
    }
    return false;
}
inline bool ResolveOcclusionType( NiGeometry& kGeometry, NiCamera const* pkCamera )
{
    assert( pkCamera );

    if ( kGeometry.IsVisualObject() && !kGeometry.GetAppCulled() )
    {
        OcclusionTimeController* pkTimeCtrler = 
            ((OcclusionTimeController*)kGeometry.GetController( &OcclusionTimeController::ms_RTTI) );
        if ( pkTimeCtrler )
        {
            pkTimeCtrler->ResolveOcclusionType( pkCamera );
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------------

inline NiTriShape* GetOccludeeProxy( NiAVObject const* pkAVObject )
{
    assert( pkAVObject );

    OcclusionTimeController* pkTimeCtrler = FindOcclusionTimeController( pkAVObject );
    if ( pkTimeCtrler )
        return pkTimeCtrler->GetProxy();
    return NULL;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace HW

} //namespace OCuller

} //namespace Renderer

} //namespace NewWare


#endif //_RENDERER_OCULLER_HW_OCCLUSIONTIMECONTROLLER_H__
