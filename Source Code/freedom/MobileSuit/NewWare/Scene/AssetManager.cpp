
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : AssetManager.cpp
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/22 LeeJR First Created
//

#include "stdafx.h"
#include "AssetManager.h"

#include "AssetUtils.h"

#include "../../PgNiFile.h"


namespace NewWare
{

namespace Scene
{


/////////////////////////////////////////////////////////////////////////////////////////////
//

char const* const AssetManager::LOD_POSTFIX_LOW = "_low";
char const* const AssetManager::LOD_POSTFIX_MIDDLE = "_mid";

//-----------------------------------------------------------------------------------

AssetManager::AssetManager()
    : m_bLockStream(false), 
      m_pkStream(NiNew NiStream)
{
    assert( m_pkStream );
}

//-----------------------------------------------------------------------------------

AssetManager::~AssetManager()
{
    RemoveAllAsset();

    NiDelete m_pkStream;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

NiNode* AssetManager::GetAsset( std::string const& strPath )
{
    if ( strPath.empty() )
    {
        assert( 0 );
        return NULL;
    }

    std::string strName = strPath;
    UPR( strName );

    NiNode* pkAssetNode = FindAsset( strName );
    if ( NULL != pkAssetNode )
        return pkAssetNode;
    return CreateAsset( strName, strPath );
}

//-----------------------------------------------------------------------------------

NiNode* AssetManager::GetAssetAsLOD( std::string const& strPath, std::string const& strPostFix )
{
    assert( strPath.empty() == false && strPostFix.empty() == false );

    unsigned int const ui = strPath.rfind( "." );
    if ( -1 != ui )
    {
        std::string strFileName = strPath;
        strFileName.insert( ui, strPostFix );

        if ( PgNiFile::CheckFileExist(strFileName.c_str()) )
            return GetAsset( strFileName );
    }
    return NULL;
}

//-----------------------------------------------------------------------------------

NiNode* AssetManager::FindAsset( std::string const& strPath ) const
{
    AssetListCIter citer = m_mAssetList.find( strPath );
    if ( citer != m_mAssetList.end() )
        return citer->second;
    return NULL;
}

//-----------------------------------------------------------------------------------

bool AssetManager::RemoveAsset( std::string const& strPath )
{
    AssetListIter iter = m_mAssetList.find( strPath );
    if ( iter != m_mAssetList.end() )
    {
        m_mAssetList.erase( iter );
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------

void AssetManager::RemoveAllAsset()
{
    m_mAssetList.clear();
}

//
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
//

NiNode* AssetManager::CreateAsset( std::string const& strName, std::string const& strPath )
{
    assert( strName.empty() == false && strPath.empty() == false && m_pkStream );
    assert( false == m_bLockStream );

    NiNode* pkAssetNode = AssetUtils::NIF::Load( *m_pkStream, strPath.c_str() );
    if ( NULL == pkAssetNode )
        return NULL;

    pkAssetNode->UpdateNodeBound();
    pkAssetNode->UpdateProperties();
    pkAssetNode->UpdateEffects();
    pkAssetNode->Update( 0.0f );

    AddAsset( strName, pkAssetNode );
    return pkAssetNode;
}

//
/////////////////////////////////////////////////////////////////////////////////////////////


} //namespace Scene

} //namespace NewWare
