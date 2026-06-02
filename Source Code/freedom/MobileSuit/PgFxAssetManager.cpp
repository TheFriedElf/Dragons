#include "stdafx.h"
#include "PgFxAssetManager.H"
#include "PgNifMan.H"

NiSourceTexturePtr PgFxAssetManager::GetSourceTexture(char const* szFileName)
{
	if( !szFileName || !szFileName[0])
		return 0;

	std::string strNormalizedFileName = this->ConvertMediaFileName(std::string(szFileName));

	return	g_kNifMan.GetTexture(strNormalizedFileName);;
}
NiFont*			 PgFxAssetManager::GetFont(char const* szFileName)
{
	if( !szFileName || !szFileName[0])
		return 0;

	std::string strNormalizedFileName = this->ConvertMediaFileName(std::string(szFileName));
	
	FontMap::iterator entryIter = m_Fonts.find(strNormalizedFileName);

	if( entryIter != m_Fonts.end() && 0 == NiStricmp(entryIter->first.c_str(), strNormalizedFileName.c_str()) )
	{
		return entryIter->second;
	}

	NiRenderer* pkRenderer = NiRenderer::GetRenderer();

	NiFont* pkFont = NiFont::Create(pkRenderer, strNormalizedFileName.c_str());			

	m_Fonts.insert(entryIter, std::pair<std::string, NiFontPtr>(strNormalizedFileName, pkFont));
	
	return pkFont;
}
NiNodePtr	PgFxAssetManager::GetNif(char const* szFileName)
{
	if( !szFileName || !szFileName[0])
		return 0;

	std::string strNormalizedFileName = this->ConvertMediaFileName(std::string(szFileName));

	return	g_kNifMan.GetNif(strNormalizedFileName.c_str());
}
// Be sure to always clone the objects from this stream.
NiStream*		 PgFxAssetManager::GetStream(char const* szFileName)
{
	return	NULL;
}

void PgFxAssetManager::ReleaseNextFrame(NiObject* pkObject)
{
	if( pkObject )
		m_ReleaseQueue.push_back(pkObject);
}

void PgFxAssetManager::ReleaseFrameDelayedAssets()
{
	m_ReleaseQueue.clear();
}

std::string PgFxAssetManager::ConvertMediaFileName(std::string const &kFileName)
{
	BM::vstring strConv("../Data/5_Effect/");

	strConv += kFileName;
	return	strConv;
}