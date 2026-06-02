#include "stdafx.h"
#include "PgPSysColorModifierDecorator.h"

NiImplementRTTI(PgPSysColorModifierDecorator, NiPSysModifier);

//---------------------------------------------------------------------------
PgPSysColorModifierDecorator::PgPSysColorModifierDecorator(char const* pcName,NiPSysColorModifier* pkColorModifier) 
: m_spColorModifier(pkColorModifier),m_kColor(NiColorA::WHITE)
{
}
//---------------------------------------------------------------------------
PgPSysColorModifierDecorator::PgPSysColorModifierDecorator() : m_spColorModifier(NULL),m_kColor(NiColorA::WHITE)
{
}
//---------------------------------------------------------------------------
void PgPSysColorModifierDecorator::Update(float fTime, NiPSysData* pkData)
{
	if(m_spColorModifier)
	{
		m_spColorModifier->Update(fTime,pkData);
	}

	ApplyColor(pkData);
}
void	PgPSysColorModifierDecorator::ApplyColor(NiPSysData* pkData)
{
    NiParticleInfo* pkCurrentParticle = pkData->GetParticleInfo();
    NiColorA* pkColor = pkData->GetColors();
    if (!pkColor)
	{
        return;
	}

    unsigned short usNumParticles = pkData->GetNumParticles();

    for (unsigned short us = 0; us < usNumParticles;
        us++, pkCurrentParticle++)
    {
        pkColor[us] *= GetColor();
    }
}

//---------------------------------------------------------------------------
// Cloning
//---------------------------------------------------------------------------
NiImplementCreateClone(PgPSysColorModifierDecorator);
//---------------------------------------------------------------------------
void PgPSysColorModifierDecorator::CopyMembers(PgPSysColorModifierDecorator* pkDest,
    NiCloningProcess& kCloning)
{
    NiPSysModifier::CopyMembers(pkDest, kCloning);

    pkDest->SetColor(GetColor());
}
