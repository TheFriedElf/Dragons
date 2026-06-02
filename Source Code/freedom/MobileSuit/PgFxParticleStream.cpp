#include "stdafx.H"
#include "PgFxParticleStream.H"
#include "PgFxParticle.H"
#include "PgFxStudio.H"
#include "PgWorld.H"
#include "PgCameraMan.H"

NiImplementRTTI(PgFxParticleStream, PgParticleStream);

PgParticlePtr	PgFxParticleStream::Load(std::string const &kID,PgParticle::stParticleInfo const &kParticleInfo)
{
	FxObjectPtr	spFxObject = NiNew FxObject(g_kFxStudio.GetFxManager(),*g_kFxStudio.GetFxStudioManager(),kParticleInfo.m_kPath.c_str(),GetWorldCamera());
	PgFxParticlePtr	spParticle = NiNew PgFxParticle();

	if(!spParticle->Create(spFxObject,kID,kParticleInfo))
	{
		return	NULL;
	}

	return	NiDynamicCast(PgParticle,spParticle);
}
NiCamera*	PgFxParticleStream::GetWorldCamera()
{
	if(!g_pkWorld)
	{
		return	NULL;
	}

	PgCameraMan*	pkCameraMan = g_pkWorld->GetCameraMan();
	if(!pkCameraMan)
	{
		return	NULL;
	}

	return	pkCameraMan->GetCamera();
}