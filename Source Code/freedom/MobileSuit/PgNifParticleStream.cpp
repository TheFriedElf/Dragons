#include "stdafx.H"
#include "PgNifParticleStream.H"
#include "PgNifParticle.H"

NiImplementRTTI(PgNifParticleStream, PgParticleStream);

PgParticlePtr	PgNifParticleStream::Load(std::string const &kID,PgParticle::stParticleInfo const &kParticleInfo)
{

	NiAVObjectPtr	spNifSource = LoadNif(kParticleInfo.m_kPath);
	if(!spNifSource)
	{
		return	NULL;
	}

	PgNifParticlePtr	spParticle = NiNew PgNifParticle();

	if(!spParticle->Create(spNifSource,kID,kParticleInfo))
	{
		return	NULL;
	}

	return	NiDynamicCast(PgParticle,spParticle);
}
NiAVObjectPtr	PgNifParticleStream::LoadNif(std::string const &kPath)
{
	NiStream kStream;

	if (!kStream.Load(kPath.c_str()))
	{
		NILOG(PGLOG_ERROR, "[PgParticle] Load %s nif file failed\n", kPath.c_str());
		return NULL;
	}

	if (kStream.GetObjectCount() == 0)
	{
		NILOG(PGLOG_ERROR, "[PgParticle] Load %s nif file has no object\n", kPath.c_str());
		return NULL;
	}

	NiNode *pkRoot = (NiNode *)kStream.GetObjectAt(0);
	PG_ASSERT_LOG(pkRoot);

	pkRoot->UpdateProperties();
	pkRoot->UpdateEffects();
	pkRoot->UpdateNodeBound();
	pkRoot->Update(0);

	NiAVObject *pkParticle = pkRoot->GetObjectByName("char_root");
	
	if(!pkParticle)
	{
		pkParticle = pkRoot->GetObjectByName("Dummy01");
		
		if(!pkParticle)
		{
			pkParticle = pkRoot->GetObjectByName("Scene Root");
			pkParticle->SetName("Scene Root_01");
		}
	}

	PG_ASSERT_LOG(pkParticle);
	if (pkParticle == NULL)
	{
		return NULL;
	}

	pkParticle->SetTranslate(0, 0, 0);	

	return	pkParticle;
}