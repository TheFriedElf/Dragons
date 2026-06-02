
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : AssetLoader.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/21 LeeJR First Created
//

#ifndef _SCENE_ASSETLOADER_H__
#define _SCENE_ASSETLOADER_H__

#include "AssetUtils.h"
#include "AssetManager.h"


namespace NewWare
{

namespace Scene
{

class DomainNodeManager;


class AssetLoader
{
private:
    struct PropertyData
    {
        PropertyData()
            : bUsePhysX(true), bIsTrigger(false), 
              bOnlyPlayerPhysX(false), bUseSharedStream(true), uiIndex(0) { /* Nothing */ };

        NiFixedString kEntityName;
        bool bUsePhysX;
        bool bIsTrigger;

        bool bOnlyPlayerPhysX;
        bool bUseSharedStream;
        NiFixedString kNIFPath;
        unsigned int const uiIndex;
    };


public:
    explicit AssetLoader( DomainNodeManager* pkDomainNodeManager );
    ~AssetLoader();


    bool Create( NiEntityStreaming* pkStream );
    void Destroy();


protected:
    void CreateEntity( NiEntityInterface* pkEntity, NiExternalAssetManager* pkAssetManager );

    bool LoadAmbienceSound( NiEntityInterface* pkEntity );
    void ReadPropertyData( NiEntityInterface* pkEntity, PropertyData& kPropData );
    void LoadEntityFromNIFData( PropertyData const& kPropData );
    void LoadEntityFromWidgetData( PropertyData const& kPropData, NiEntityInterface* pkEntity, 
                                   NiExternalAssetManager* pkAssetManager ); // case of Camera/Light widget etc.
    bool CheckEntityForAttributeData( NiEntityInterface* pkEntity );


private:
    void BuildDefaultCamera( NiEntityInterface* pkEntity, NiCamera* pkCamera );
    void BuildMinimapCamera( NiEntityInterface* pkEntity, NiCamera* pkCamera );
    void BuildLight( NiEntityInterface* pkEntity );

    bool BuildActorEntity( NiEntityInterface* pkEntity );

    void ApplyLightsToAffectedNodes();
    void ApplyLightToAffectedEntities( NiLight* pkLight, NiEntityComponentInterface* pkComponent );
    void AttachAffectedNodeToLight( NiLight* pkLight, NiNode* pkAffectedNode );


private:
    DomainNodeManager* m_pkDomainNodeManager;

    NiNodePtr m_spLightRoot;
    NiCameraPtr m_spMinimapCamera;

    AssetUtils::NIF::NiNodeMap m_mAffectedNodeFromLight;
    AssetUtils::GSA::NiEntityInterfaceList m_lstLightList;

    AssetManager m_kSharedAssets;
};


} //namespace Scene

} //namespace NewWare


#endif //_SCENE_ASSETLOADER_H__
