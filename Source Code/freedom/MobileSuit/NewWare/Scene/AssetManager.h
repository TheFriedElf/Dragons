
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : AssetManager.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/22 LeeJR First Created
//

#ifndef _SCENE_ASSETMANAGER_H__
#define _SCENE_ASSETMANAGER_H__


namespace NewWare
{

namespace Scene
{


class AssetManager
{
private:
    typedef std::map< std::string, NiNodePtr > AssetList;
    typedef AssetList::value_type              AssetListValueType;
    typedef AssetList::iterator                AssetListIter;
    typedef AssetList::const_iterator          AssetListCIter;


public:
    static char const* const LOD_POSTFIX_LOW;
    static char const* const LOD_POSTFIX_MIDDLE;


public:
    AssetManager();
    ~AssetManager();


    NiNode* GetAsset( std::string const& strPath );
    NiNode* GetAssetAsLOD( std::string const& strPath, std::string const& strPostFix );
    NiNode* FindAsset( std::string const& strPath ) const;
    bool RemoveAsset( std::string const& strPath );
    void RemoveAllAsset();

    NiStream* GetStream() const { m_bLockStream = true; return m_pkStream; };
    void ReleaseStream() { assert( m_bLockStream ); m_bLockStream = false; };


protected:
    NiNode* CreateAsset( std::string const& strName, std::string const& strPath );

    void AddAsset( std::string const& strPath, NiNode* pkAssetNode )
    {
        assert( pkAssetNode );
        m_mAssetList.insert( AssetListValueType(strPath, pkAssetNode) );
    }


private:
    mutable bool m_bLockStream;
    NiStream* m_pkStream;

    AssetList m_mAssetList;
};


} //namespace Scene

} //namespace NewWare


#endif //_SCENE_ASSETMANAGER_H__
