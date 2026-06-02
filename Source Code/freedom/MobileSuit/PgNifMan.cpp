#include "stdafx.h"
#include "PgNifMan.h"
#include <D3D9.h>
#include <D3Dx9tex.h>
#include "PgResourceMonitor.h"
#include "PgRenderer.H"
#include "PgNiFile.h"

#include "NewWare/Scene/ApplyTraversal.h"


PgNifMan::PgNifMan() 
{
	Init();
}

PgNifMan::~PgNifMan()
{
	Destroy();
}

void	PgNifMan::Init()
{
}

void	PgNifMan::Destroy()
{
	DeleteAllNif();
	DeleteAllTexture();
}

SNifScene PgNifMan::GetNewNifScene(const std::string& NifFilePath, NiObjectNET::CopyType kCopyType)
{
	SNifScene kNifScene;

	NiStream kStream;
	if(kStream.Load(NifFilePath.c_str()) == false)
	{	//	로딩 실패 메세지
		NILOG(PGLOG_ERROR, "[PgNifMan] Can't Load %s file\n", NifFilePath.c_str());
		return kNifScene;
	}

	if (kStream.GetObjectCount() == 0)
	{
		NILOG(PGLOG_ERROR, "[PgNifMan] Load %s file but no object\n", NifFilePath.c_str());
		return kNifScene;
	}

	NiNodePtr spSourceNif = (NiNode*)kStream.GetObjectAt(0);
	if (spSourceNif == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgNifMan] Load %s file but no object\n", NifFilePath.c_str());
		return kNifScene;
	}

	spSourceNif->SetDefaultCopyType(kCopyType);
	spSourceNif->UpdateNodeBound();
	spSourceNif->UpdateProperties();
	spSourceNif->UpdateEffects();
	spSourceNif->Update(0.0f);
	kNifScene.spNifNode = spSourceNif;

	unsigned int cnt = kStream.GetObjectCount();
	for (unsigned int ui = 0 ; ui < cnt ; ui++)
	{
		NiObject *pkObject = kStream.GetObjectAt(ui);
		if(NiIsKindOf(NiPhysXScene, pkObject))
		{
			NiPhysXScenePtr spPhysXObject = NiDynamicCast(NiPhysXScene, pkObject);
			kNifScene.spPhysXScene = spPhysXObject;
			break;
		}
	}

	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(NifFilePath, PgResourceMonitor::RESOURCE_TYPE_NIF));
	return kNifScene;
}

NiNodePtr PgNifMan::findNif(const std::string& NifFilePath)
{
	if (NifFilePath.empty())
	{
		return NULL;
	}

	std::string uprPath = NifFilePath;
	UPR(uprPath);
	NiNodePtr spSourceNif = NULL;

	m_kNifLock.Lock();
	NifContainer::iterator itr = m_NifContainer.find(uprPath);

	if (itr == m_NifContainer.end())
	{
		NiStream kStream;
		if(kStream.Load(NifFilePath.c_str()) == false)
		{
			NILOG(PGLOG_ERROR, "[PgNifMan] Can't Load %s file\n", NifFilePath.c_str());
			m_kNifLock.Unlock();
			return NULL;
		}

		if (kStream.GetObjectCount() == 0)
		{
			NILOG(PGLOG_ERROR, "[PgNifMan] Load %s file but no object\n", NifFilePath.c_str());
			m_kNifLock.Unlock();
			return NULL;
		}

		spSourceNif = NiDynamicCast(NiNode,kStream.GetObjectAt(0));
		if (spSourceNif == NULL)
		{
			NILOG(PGLOG_ERROR, "[PgNifMan] Load %s file but no object\n", NifFilePath.c_str());
			m_kNifLock.Unlock();
			return NULL;
		}

		spSourceNif->SetDefaultCopyType(NiObjectNET::COPY_EXACT);
		spSourceNif->UpdateNodeBound();
		spSourceNif->UpdateProperties();
		spSourceNif->UpdateEffects();
		spSourceNif->Update(0.0f);

        NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( spSourceNif, false );

		m_NifContainer.insert(std::make_pair(uprPath, spSourceNif));
		m_kNifLock.Unlock();
		return spSourceNif;
	}

	spSourceNif = itr->second;
	m_kNifLock.Unlock();
	return spSourceNif;
}

NiNodePtr PgNifMan::GetNif(const std::string& NifFilePath, NiObjectNET::CopyType kCopyType)	//	Nif 파일을 로딩한 후, 클론 한 오브젝트를 리턴한다.
{
	NiNodePtr	spSourceNif = findNif(NifFilePath);
	PG_ASSERT_LOG(spSourceNif);
	if (spSourceNif == NULL)
		return NULL;

	NiNodePtr	spClone = NiDynamicCast(NiNode,spSourceNif->Clone());
	PG_ASSERT_LOG(spClone);
	if (spClone == NULL)
		return NULL;

	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(NifFilePath, PgResourceMonitor::RESOURCE_TYPE_NIF));

	spClone->UpdateNodeBound();
	spClone->UpdateProperties();
	spClone->UpdateEffects();
	spClone->Update(0.0f);

    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( spClone, false );

	return spClone;
}

NiNodePtr PgNifMan::GetNif_DeepCopy(const std::string& NifFilePath)	//	Nif 파일을 로딩한 후, 클론 한 오브젝트를 리턴한다.(이때에는 DeepCopy를 사용한다)
{
	NiNodePtr	spSourceNif = findNif(NifFilePath);
	PG_ASSERT_LOG(spSourceNif);
	if (spSourceNif == NULL)
		return NULL;

	NiNodePtr	spClone = NiDynamicCast(NiNode, spSourceNif->CreateDeepCopy());
	PG_ASSERT_LOG(spClone);
	if (spClone == NULL)
		return NULL;

	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(NifFilePath, PgResourceMonitor::RESOURCE_TYPE_NIF));
	
	spClone->UpdateNodeBound();
	spClone->UpdateProperties();
	spClone->UpdateEffects();
	spClone->Update(0.0f);

    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( spClone, false );

	return spClone;
}

bool	PgNifMan::DeleteAllNif()	//	컨테이너 클리어
{
	BM::CAutoMutex kLock(m_kNifLock);

#ifndef EXTERNAL_RELEASE
	// 혹시 문제가 있는 파일이 있을까봐 하나씩 지운다.
	NifContainer::iterator itr = m_NifContainer.begin();
	
	while (itr != m_NifContainer.end())
	{
		std::string nifName = (*itr).first;
		if ((*itr).second->GetRefCount() > 1)
		{
			_PgOutputDebugString("[PgNifMan] DeleteAllNif : %s nif has %d reference count\n", (*itr).first.c_str(), (*itr).second->GetRefCount());
		}
		(*itr).second = NULL;
		++itr;
	}
#endif
	m_NifContainer.clear();
	return true;
}

bool	PgNifMan::DeleteAllTexture()	//	컨테이너 클리어
{
	BM::CAutoMutex kLock(m_kTextureLock);
#ifndef EXTERNAL_RELEASE
	// 혹시 문제가 있는 파일이 있을까봐 하나씩 지운다.
	TextureContainer::iterator itr = m_TextureContainer.begin();
	
	while (itr != m_TextureContainer.end())
	{
		std::string texName = (*itr).first;
		if ((*itr).second->GetRefCount() > 1)
		{
			_PgOutputDebugString("[PgNifMan] DeleteAllTexture : %s nif has %d reference count\n", (*itr).first.c_str(), (*itr).second->GetRefCount());
		}
		(*itr).second = NULL;
		++itr;
	}
#endif
	m_TextureContainer.clear();
	return true;
}

NiSourceTexturePtr PgNifMan::findTexture(const std::string& TexturePath)
{
	if (TexturePath.empty())
		return NULL;

	std::string uprPath = TexturePath;
	UPR(uprPath);
	NiSourceTexturePtr spSourceTexture = NULL;

	m_kTextureLock.Lock();
	TextureContainer::iterator itr = m_TextureContainer.find(uprPath);	

	if (itr == m_TextureContainer.end())
	{
		if (PgNiFile::CheckFileExist(TexturePath.c_str()) == false)
		{
			NILOG(PGLOG_ERROR, "[PgNifMan] %s textures doesn't exist\n", TexturePath.c_str());
			// 그냥 아래로 흘러가게 해서 8*8의 보라색 Texture가 생기게 한다.
		}

		spSourceTexture = NiSourceTexture::Create(TexturePath.c_str());
		if (spSourceTexture == NULL)
		{
			NILOG(PGLOG_ERROR, "[PgNifMan] Can't Load %s file\n", TexturePath.c_str());
			m_kTextureLock.Unlock();
			return NULL;
		}

#ifndef EXTERNAL_RELEASE
		unsigned int width = spSourceTexture->GetWidth();
		unsigned int height = spSourceTexture->GetHeight();
		if (width <= 128 || height <= 128)
		{
			NILOG(PGLOG_WARNING, "[PgNifMan] %s texture size is small(%dx%d)\n", TexturePath.c_str(), width, height);
		}
#endif
		m_TextureContainer.insert(std::make_pair(uprPath, spSourceTexture));
		m_kTextureLock.Unlock();
		return spSourceTexture;
	}
	spSourceTexture = itr->second;	
	m_kTextureLock.Unlock();
	return spSourceTexture;
}

NiSourceTexturePtr PgNifMan::GetTexture(const std::string& TexturePath)
{
	if (TexturePath.empty())
	{
		return NULL;
	}

	NILOG(PGLOG_MEMIO, "[PgNifMan] Get %s texture \n", TexturePath.c_str());
	NiSourceTexturePtr	spSourceTexture = findTexture(TexturePath);
	PG_ASSERT_LOG(spSourceTexture);
	if (spSourceTexture == NULL)
		return NULL;

	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(TexturePath, PgResourceMonitor::RESOURCE_TYPE_TEXTURE));

	return spSourceTexture;
}