#include "stdafx.h"
#include "PgParticleProcessorFadeIn.H"
#include "PgParticle.H"

NiImplementRTTI(PgParticleProcessorFadeIn, PgParticleProcessor);

bool	PgParticleProcessorFadeIn::DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime)
{
	if(!pkParticle)
	{
		return	false;
	}

	float	fAlpha = 1;
	
	if(GetFadeInTime()>0)
	{
		fAlpha = pkParticle->GetNow()/GetFadeInTime();
		if(fAlpha>1)
		{
			fAlpha = 1;
		}
	}

	NiColorA	kColor = pkParticle->GetColorLocal();
	kColor.a = kColor.a * fAlpha;
	pkParticle->SetColorLocal(kColor);

	if(fAlpha==1)
	{
		SetFinished(true);
	}

	return	false;

}
NiImplementCreateClone(PgParticleProcessorFadeIn);

void PgParticleProcessorFadeIn::CopyMembers(PgParticleProcessorFadeIn* pDest, NiCloningProcess& kCloning)
{
	PgParticleProcessor::CopyMembers(pDest,kCloning);

	pDest->SetFadeInTime(GetFadeInTime());
}
