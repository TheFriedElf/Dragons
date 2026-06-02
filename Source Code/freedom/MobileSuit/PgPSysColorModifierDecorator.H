#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_MODIFIER_PGPSYSCOLORMIDIFIERDECORATOR_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_MODIFIER_PGPSYSCOLORMIDIFIERDECORATOR_H

#include "NiPSysModifier.h"
#include <NiPSysColorModifier.H>
#include <NiColorData.h>

class NIPARTICLE_ENTRY PgPSysColorModifierDecorator : public NiPSysModifier
{
    NiDeclareRTTI;
    NiDeclareClone(PgPSysColorModifierDecorator);

public:
    PgPSysColorModifierDecorator(char const* pcName, NiPSysColorModifier* pkColorModifier);

    virtual void Update(float fTime, NiPSysData* pkData);

	void	SetColor(NiColorA const &kColor)	{	m_kColor = kColor;	}
	NiColorA	const&	GetColor()	const{	return	m_kColor;	}

private:

	void	ApplyColor(NiPSysData* pkData);

protected:
    // For cloning and streaming only.
    PgPSysColorModifierDecorator();

	NiColorA	m_kColor;

	NiPSysColorModifierPtr	m_spColorModifier;

};

NiSmartPointer(PgPSysColorModifierDecorator);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_MODIFIER_PGPSYSCOLORMIDIFIERDECORATOR_H