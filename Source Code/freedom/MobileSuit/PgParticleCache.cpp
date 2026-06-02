#include "StdAfx.h"
#include "PgParticleCache.h"
#include "PgParticleStreamManager.H"
#include "PgNifParticleStream.H"
#include "PgFxParticleStream.H"

NiImplementRTTI(PgParticleCache, NiObject);

PgParticleCache::PgParticleCache()
{
	m_spParticleStreamManager = NiNew PgParticleStreamManager();
	m_spParticleStreamManager->RegisterStreams();
}
void	PgParticleCache::ReleaseCacheBySourceType(std::string const &kSourceType)
{
	BM::CAutoMutex kLock(m_kCacheContLock);

	for(Container::iterator itor = m_kCacheContainer.begin(); itor != m_kCacheContainer.end();)
	{
		std::string const &kParticleID = itor->first;

		PgParticle::stParticleInfo const *pkParticleInfo = FindParticleInfo(kParticleID);
		if(pkParticleInfo && pkParticleInfo->m_kSourceType == kSourceType)
		{
			itor = m_kCacheContainer.erase(itor);
			continue;
		}

		++itor;
	}

}
PgParticlePtr	PgParticleCache::CacheParticle(char const *pcEffectID)
{
	if (pcEffectID == NULL)
	{
		return NULL;
	}

	NiString strEffectID(pcEffectID);
	strEffectID.ToLower();

	PgParticlePtr spParticle = CreateParticle(strEffectID);

	PgParticle::stParticleInfo const* pkParticleInfo = FindParticleInfo(pcEffectID);
	
	if(pkParticleInfo)
	{
		if(false == pkParticleInfo->m_bNoUseCacheAndDeepCopy)
		{
			if(!AddParticleToCacheContainer(strEffectID,spParticle))
			{
				return	NULL;
			}
		}
	}
	else
	{
		if(!AddParticleToCacheContainer(strEffectID,spParticle))
		{
			return	NULL;
		}
	}

	return	spParticle;
}
PgParticlePtr	PgParticleCache::GetParticle(char const *pcEffectID)
{
	if (pcEffectID == NULL)
	{
		return NULL;
	}

	NiString strEffectID(pcEffectID);
	strEffectID.ToLower();

	PgParticlePtr spParticle = FindParticleInCacheContainer(strEffectID);

	PgParticle::stParticleInfo const* pkParticleInfo = FindParticleInfo(pcEffectID);
	if(pkParticleInfo)
	{
		if(pkParticleInfo->m_bNoUseCacheAndDeepCopy)
		{
			spParticle = NULL;
		}
	}

	if(!spParticle)
	{
		spParticle = CacheParticle(strEffectID);
	}

	return	spParticle;
}
PgParticlePtr	PgParticleCache::FindParticleInCacheContainer(char const *pcEffectID)
{
	PgParticlePtr spParticle = NULL;

	BM::CAutoMutex kLock(m_kCacheContLock);

	Container::iterator itr = m_kCacheContainer.find(pcEffectID);
	if(itr != m_kCacheContainer.end())
	{
		spParticle = itr->second;
	}

	return	spParticle;
}
bool	PgParticleCache::AddParticleToCacheContainer(char const *pcEffectID,PgParticlePtr spParticle)
{
	BM::CAutoMutex kLock(m_kCacheContLock);

	std::pair<Container::iterator, bool> ret = m_kCacheContainer.insert(std::make_pair((char const*)pcEffectID, spParticle));

	return	ret.second;
}
PgParticlePtr	PgParticleCache::CreateParticle(char const *pcEffectID)
{
	PgParticle::stParticleInfo const *pkParticleInfo = FindParticleInfo(pcEffectID);
	if(NULL == pkParticleInfo)
	{
		PgError1("Incorrect Effect Name : [%s]", pcEffectID);
		return 0;
	}

	std::string	const	&kParticlePath = pkParticleInfo->m_kPath;

	PgParticlePtr	spNewParticle = m_spParticleStreamManager->LoadParticle(pkParticleInfo->m_kSourceType,pcEffectID,*pkParticleInfo);
	if(!spNewParticle)
	{
		PgError1("[PgParticleCache] Can't Load Effect [%s]", kParticlePath.c_str());
		return NULL;
	}

	return	spNewParticle;
}
