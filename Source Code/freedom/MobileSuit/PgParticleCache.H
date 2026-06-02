#ifndef FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLECACHE_H
#define FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLECACHE_H

#include <NiMain.H>
#include "PgParticle.H"
 
NiSmartPointer(PgParticleStreamManager);

class PgParticleCache : public NiObject
{

	NiDeclareRTTI;

public:


	typedef std::map<std::string, PgParticlePtr> Container;
	typedef std::map<std::string, PgParticle::stParticleInfo> ParticleInfoContainer;

public:

	PgParticleCache();

	PgParticlePtr GetParticle(char const *pcEffectID);

public:

	void	AddParticleInfo(std::string const &kParticleID,PgParticle::stParticleInfo const &kParticleInfo)
	{
		m_kParticleInfoContainer.insert(std::make_pair(kParticleID,kParticleInfo));
	}
	PgParticlePtr	CacheParticle(char const *pcEffectID);

	void	ReleaseCacheBySourceType(std::string const &kSourceType);

	PgParticle::stParticleInfo	const*	FindParticleInfo(std::string const &kParticleID)
	{
		ParticleInfoContainer::iterator itor = m_kParticleInfoContainer.find(kParticleID);

		if(itor == m_kParticleInfoContainer.end())
		{
			return NULL;
		}

		return	&itor->second;
	}

private:

	PgParticlePtr	CreateParticle(char const *pcEffectID);
	PgParticlePtr	FindParticleInCacheContainer(char const *pcEffectID);
	bool	AddParticleToCacheContainer(char const *pcEffectID,PgParticlePtr pkParticle);
	

protected:

	Container m_kCacheContainer;
	Loki::Mutex m_kCacheContLock;

	ParticleInfoContainer m_kParticleInfoContainer;

	PgParticleStreamManagerPtr	m_spParticleStreamManager;
};

NiSmartPointer(PgParticleCache);

#endif // FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLECACHE_H