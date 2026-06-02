#include "stdafx.h"
#include "PgPSysBoundedPositionModifier.H"

NiImplementRTTI(PgPSysBoundedPositionModifier, NiPSysPositionModifier);


PgPSysBoundedPositionModifier::PgPSysBoundedPositionModifier(const char *pcName)
:NiPSysPositionModifier(pcName)
{
}

void PgPSysBoundedPositionModifier::Update(float fTime, NiPSysData* pkData)
{
	if(NULL == pkData)
	{
		return;
	}
	NiPSysPositionModifier::Update(fTime,pkData);
	AdjustParticlePositionToBound(pkData);
}

void	PgPSysBoundedPositionModifier::AdjustParticlePositionToBound(NiPSysData* pkData)
{
	if(NULL == pkData)
	{
		return;
	}
    for (unsigned short us = 0; us < pkData->GetNumParticles(); us++)
    {
		AdjustParticlePositionToBound(pkData,us);
    }
}

void	PgPSysBoundedPositionModifier::AdjustParticlePositionToBound(NiPSysData* pkData,unsigned int uiParticleIndex)
{
	if(NULL == pkData)
	{
		return;
	}
    NiParticleInfo* pkParticles = pkData->GetParticleInfo();
	NiParticleInfo* pkCurrentParticle = &pkParticles[uiParticleIndex];

	NiPoint3	kVertexPosition = pkData->GetVertices()[uiParticleIndex];

	NiBound	const	&kWorldBound = GetWorldBound();

	NiPoint3	vVertexToBoundCenter = kVertexPosition - kWorldBound.GetCenter();
	float	fDVertexToBoundCenter = vVertexToBoundCenter.Length();

	if(fDVertexToBoundCenter<kWorldBound.GetRadius())
	{
		return;
	}

	NiPoint3	kNewVertexPosition = CropVertexPosition(kVertexPosition);

	pkData->GetVertices()[uiParticleIndex] = kNewVertexPosition;
}