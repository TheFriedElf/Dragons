#include "StdAfx.h"
#include "PgFxParticle.h"
#include "FxReference/FxObject.H"

NiImplementRTTI(PgFxParticle, PgParticle);

void	PgFxParticle::CheckFinish(float fAccumTime,float fFrameTime)
{
	if(IsFinished())
	{
		return;
	}
	if(m_spFxObject)
	{
		if(m_spFxObject->GetDestroyed())
		{
			SetFinished(true);
		}
	}
}
bool PgFxParticle::Create(NiAVObject *pkParticleRoot,std::string const &kID,PgParticle::stParticleInfo const &kParticleInfo)
{
	if(!pkParticleRoot)
	{
		return	false;
	}
	m_spFxObject = NiDynamicCast(FxObject,pkParticleRoot);
	if(!m_spFxObject)
	{
		return	false;
	}

	m_spFxObject->GetFxInstance().SetAutoUpdate(false);

	m_spFxObject->SetTranslate(0, 0, 0);	
	
	AttachChild(m_spFxObject, true);

	UpdateProperties();
	UpdateNodeBound();
	UpdateEffects();
	NiAVObject::Update(0.0f, true);
	
	ApplyParticleInfo(kParticleInfo);
	SetID(kID.c_str());

	return true;
}
NiObject* PgFxParticle::CreateClone(NiCloningProcess& kCloning)
{
	SetFxObjectCamera();

    PgFxParticle* pkClone = NiNew PgFxParticle;
    CopyMembers(pkClone , kCloning);
    return pkClone;
}
void	PgFxParticle::SetFxObjectCamera()
{
	if(!g_pkWorld)
	{
		return;
	}
	PgCameraMan	*pkCameraMan = g_pkWorld->GetCameraMan();
	if(!pkCameraMan)
	{
		return;
	}

	if(!m_spFxObject)
	{
		return;
	}

	m_spFxObject->SetCamera(pkCameraMan->GetCamera());
}
void PgFxParticle::CopyMembers(PgFxParticle* pDest, NiCloningProcess& kCloning)
{
	PgParticle::CopyMembers(pDest,kCloning);
}
void PgFxParticle::ProcessClone(NiCloningProcess& kCloning)
{
	PgParticle::ProcessClone(kCloning);

    NiObject* pkClone = NULL;
    bool bCloned = kCloning.m_pkCloneMap->GetAt(this, pkClone);
    assert(bCloned);
    PgFxParticle* pkDest = (PgFxParticle*)pkClone;

    bCloned = kCloning.m_pkCloneMap->GetAt(m_spFxObject, pkClone);

    if (bCloned)
    {
        pkDest->m_spFxObject = (FxObject*) pkClone;
		pkDest->m_spFxObject->GetFxInstance().SetAutoUpdate(true);
    }
    else
    {
        pkDest->m_spFxObject = m_spFxObject;
    }
}
