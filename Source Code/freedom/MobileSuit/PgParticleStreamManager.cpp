#include "stdafx.h"
#include "PgParticleStreamManager.H"
#include "PgNifParticleStream.H"
#include "PgFxParticleStream.H"

void	PgParticleStreamManager::RegisterStreams()
{
	RegisterStream(NiNew PgNifParticleStream());
	RegisterStream(NiNew PgFxParticleStream());
}

PgParticlePtr	PgParticleStreamManager::LoadParticle(std::string const &kStreamID,std::string const &kParticleID,PgParticle::stParticleInfo const &kParticleInfo)
{

	PgParticleStream	*pkStream = FindStream(kStreamID);
	if(!pkStream)
	{
		return	NULL;
	}

	return	pkStream->Load(kParticleID,kParticleInfo);
}

void	PgParticleStreamManager::RegisterStream(PgParticleStream *pkStream)
{
	if(FindStream(pkStream->GetStreamID()))
	{
		return;
	}

	m_kParticleStreamCont.insert(std::make_pair(pkStream->GetStreamID(),pkStream));
}

PgParticleStream*	PgParticleStreamManager::FindStream(std::string const &kStreamID)
{

	ParticleStreamMap::iterator itor = m_kParticleStreamCont.find(kStreamID);
	if(itor == m_kParticleStreamCont.end())
	{
		return	NULL;
	}


	PgParticleStream	*pkStream = itor->second;
	return	pkStream;
}
