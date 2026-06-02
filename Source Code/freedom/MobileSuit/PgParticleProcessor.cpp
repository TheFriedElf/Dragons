#include "stdafx.h"
#include "PgParticleProcessor.H"

NiImplementRTTI(PgParticleProcessor, NiObject);


void PgParticleProcessor::CopyMembers(PgParticleProcessor* pDest, NiCloningProcess& kCloning)
{
	NiObject::CopyMembers(pDest,kCloning);
}
