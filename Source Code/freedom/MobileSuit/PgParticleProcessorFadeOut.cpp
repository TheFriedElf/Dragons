#include "stdafx.h"
#include "PgParticleProcessorFadeOut.H"
#include "PgParticle.H"

NiImplementRTTI(PgParticleProcessorFadeOut, PgParticleProcessor);

bool	PgParticleProcessorFadeOut::DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime)
{
	if(!pkParticle)
	{
		return	false;
	}

	UpdateElapsedTime(fAccumTime);

	float	fAlpha = 0;
	
	if(GetFadeOutTime()>0)
	{
		fAlpha = GetElapsedTime()/GetFadeOutTime();
		if(fAlpha>1)
		{
			fAlpha = 1;
		}

		fAlpha = 1-fAlpha;
	}

	NiColorA	kColor = pkParticle->GetColorLocal();
	kColor.a = kColor.a * fAlpha;
	pkParticle->SetColorLocal(kColor);

	if(fAlpha==0)
	{
		SetFinished(true);
		pkParticle->SetFinished(true);
	}

	return	false;

}
NiImplementCreateClone(PgParticleProcessorFadeOut);

void PgParticleProcessorFadeOut::CopyMembers(PgParticleProcessorFadeOut* pDest, NiCloningProcess& kCloning)
{
	PgParticleProcessor::CopyMembers(pDest,kCloning);

	pDest->SetFadeOutTime(GetFadeOutTime());
}
