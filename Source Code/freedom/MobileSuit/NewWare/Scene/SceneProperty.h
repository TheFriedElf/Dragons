
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : SceneProperty.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/21 LeeJR First Created
//

#ifndef _SCENE_SCENEPROPERTY_H__
#define _SCENE_SCENEPROPERTY_H__


namespace NewWare
{

namespace Scene
{


class SceneProperty
{
private:
    enum { AFFECTEDOBJECT_DEFAULT_COUNT = 8 };

    typedef std::vector<NiAVObjectPtr>         AffectedObjectList;
    typedef AffectedObjectList::iterator       AffectedObjectListIter;
    typedef AffectedObjectList::const_iterator AffectedObjectListCIter;


public:
    struct FogChannel
    {
        struct OriginalValue
        {
            OriginalValue() : fFogStart(0.0f), fFogEnd(0.0f), fFogDensity(0.0f) { /**/ };

            float fFogStart;
            float fFogEnd;
            float fFogDensity;
        };

        FogChannel();

        void Switch( NiNode* pkSceneRoot, bool bTurnOn, bool bAdjustable )
        {
            assert( pkSceneRoot );
            if ( m_bForceTurnOn && bTurnOn )
                TurnOn( pkSceneRoot, bAdjustable );
            else
                TurnOff( pkSceneRoot );
        }
        void TurnOn( NiNode* pkSceneRoot, bool bAdjustable );
        void TurnOff( NiNode* pkSceneRoot );

        bool m_bForceTurnOn;
        bool m_bDebugAdjustable;
        NiFogProperty* m_pkProperty;
        AffectedObjectList m_vAffectedObjectList;

        bool m_bApplyToAffectedObject;
        bool m_bUseEndFar;

        OriginalValue m_kOriginValue;
    };


public:
    SceneProperty();
    ~SceneProperty();


    void SwitchFogChannel( bool bTurnOn, bool bAdjustable ) 
        { assert( m_spSceneRoot ); m_kFog.Switch( m_spSceneRoot, bTurnOn, bAdjustable ); };
    void AddAffectedObjectToFogChannel( NiAVObject* pkAVObject ) 
        { assert( pkAVObject ); m_kFog.m_vAffectedObjectList.push_back( pkAVObject ); };
    bool RemoveAffectedObjectFromFogChannel( NiAVObject* pkAVObject );
    bool RemoveAllAffectedObjectFromFogChannel() { m_kFog.m_vAffectedObjectList.clear(); };
    void SetFogChannelApplyToAffectedObject( bool bApply ) { m_kFog.m_bApplyToAffectedObject = bApply; };
    bool GetFogChannelApplyToAffectedObject() const { return m_kFog.m_bApplyToAffectedObject; };
    void SetFogChannelUseEndFar( bool bUse ) { m_kFog.m_bUseEndFar = bUse; };
    bool GetFogChannelUseEndFar() const { return m_kFog.m_bUseEndFar; };


private:
    FogChannel m_kFog;

    NiNodePtr m_spSceneRoot;
};


} //namespace Scene

} //namespace NewWare


#endif //_SCENE_SCENEPROPERTY_H__
