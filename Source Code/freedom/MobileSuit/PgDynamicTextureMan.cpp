#include "stdafx.h"
#include "PgDynamicTextureMan.h"

PgDynamicTextureMan	g_DynamicTextureMan;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgDynamicTexture
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgDynamicTexture::PgDynamicTexture()
:m_spCopyData(NULL),
m_bDirty(false),
m_spDynamicTexture(0),
m_iWidth(0),
m_iHeight(0)
{
}

PgDynamicTexture::PgDynamicTexture(unsigned int uiWidth, unsigned int uiHeight, NiDynamicTexture::FormatPrefs& kPrefs,bool bOnlySystemMem) 
: m_spCopyData(NULL),
m_iWidth(uiWidth),
m_iHeight(uiHeight),
m_bDirty(false),
m_spDynamicTexture(0)
{
	NILOG(PGLOG_LOG,"PgDynamicTexture::PgDynamicTexture() uiWidth : %d uiHeight : %d\n",uiWidth,uiHeight);

	PG_ASSERT_LOG(uiWidth > 1);
	PG_ASSERT_LOG(uiHeight > 1);

	m_iWidth = uiWidth;
	m_iHeight = uiHeight;

	NiPixelFormat	kPixelFormat(NiPixelFormat::BGRA4444);
	m_spCopyData = NiNew NiPixelData(uiWidth,uiHeight,kPixelFormat);

	if(bOnlySystemMem == false)
	{
		bool bBackup = NiSourceTexture::GetDestroyAppDataFlag();
		
		NiSourceTexture::SetDestroyAppDataFlag(false);
		m_spDynamicTexture = NiSourceTexture::Create(m_spCopyData,kPrefs);
		NiSourceTexture::SetDestroyAppDataFlag(bBackup);
		if(m_spDynamicTexture == 0) 
		{
			NILOG(PGLOG_LOG,"PgDynamicTexture::PgDynamicTexture() NiDynamicTexture::Create() Failed\n");
			return;
		}

		m_spDynamicTexture->SetStatic(false);
	}

	NILOG(PGLOG_LOG,"PgDynamicTexture::PgDynamicTexture() Create Success iWidth : %d iHeight : %d\n",uiWidth,uiHeight);
}

PgDynamicTexture::~PgDynamicTexture()
{
	m_spCopyData = 0;
	m_spDynamicTexture = 0;
	m_iWidth = m_iHeight = 0;

}
void*	PgDynamicTexture::Lock(int &iPitch,bool bClearMem)
{
	if(!m_spCopyData)
	{
		return	NULL;
	}

	iPitch = m_spCopyData->GetPixelStride()*m_iWidth;

	if(bClearMem)
	{
		::memset(m_spCopyData->GetPixels(),0,(iPitch*m_iHeight));
	}

	return	(void*)m_spCopyData->GetPixels();
}
void	PgDynamicTexture::UnLock()
{
	RecreateByCopyData();
}
BYTE*	PgDynamicTexture::GetCopyData()
{
	if(!m_spCopyData)
	{
		return	NULL;
	}
	return	(BYTE*)m_spCopyData->GetPixels();	
}

// IDEA : Dirty Flag를 두는게 낫지 않나?
void	PgDynamicTexture::RecreateByCopyData()
{
	//NILOG(PGLOG_LOG,"PgDynamicTexture::RecreateByCopyData() Start\n");
	if( m_spDynamicTexture == 0 || m_spCopyData == 0)
	{
		return;
	}

	NiDX9TextureData* pkTexData = (NiDX9TextureData*)m_spDynamicTexture->GetRendererData();	
	if(!pkTexData)
	{
		return;
	}

	m_spCopyData->MarkAsChanged();

	NiDX9SourceTextureData* pkSourceTexData = (NiDX9SourceTextureData *)pkTexData;
	pkSourceTexData->Update();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgDynamicTextureMan
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgDynamicTexture*	PgDynamicTextureMan::CreateDynamicTexture(unsigned int uiWidth, unsigned int uiHeight, NiDynamicTexture::FormatPrefs& kPrefs,bool bOnlySystemMem)
{
	PgDynamicTexture	*pNewTexture = NiNew PgDynamicTexture(uiWidth,uiHeight,kPrefs,bOnlySystemMem);
	if(bOnlySystemMem==false && pNewTexture->GetTexture() == 0) 
	{
		SAFE_DELETE_NI(pNewTexture);
		return NULL;
	}
	m_Textures.push_back(pNewTexture);

	return	pNewTexture;
}
void	PgDynamicTextureMan::ReleaseDynamicTexture(PgDynamicTexture *pTexture)
{
	for(CONT_DYNAMIC_TEXTURE::iterator itor = m_Textures.begin(); itor != m_Textures.end(); itor++)
	{
		if(pTexture == *itor)
		{
			SAFE_DELETE_NI(pTexture);
			m_Textures.erase(itor);
			return;
		}
	}
}
void	PgDynamicTextureMan::RecreateAllTextureByCopyData()
{
	for(CONT_DYNAMIC_TEXTURE::iterator itor = m_Textures.begin(); itor != m_Textures.end(); itor++)
	{
		(*itor)->RecreateByCopyData();
	}
}
void	PgDynamicTextureMan::Destroy()
{
	for(CONT_DYNAMIC_TEXTURE::iterator itor = m_Textures.begin(); itor != m_Textures.end(); itor++)
	{
		SAFE_DELETE_NI(*itor);
	}
	m_Textures.clear();

	m_bAlive = false;
}