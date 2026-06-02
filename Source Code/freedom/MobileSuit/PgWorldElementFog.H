#ifndef FREEDOM_DRAGONICA_SCENE_WORLD_WORLDELEMENT_PGWORLDELEMENTFOG_H
#define FREEDOM_DRAGONICA_SCENE_WORLD_WORLDELEMENT_PGWORLDELEMENTFOG_H

#include <NiMain.H>
#include "Variant/PgWorldTimeCondition.h"


class	PgWorldElementFog	:	public	NiObject
{

public:

	enum	ATTRIBUTE_TYPE
	{
		AT_CONDITION=0,
		AT_DEPTH,
		AT_FAR_DISTANCE,
		AT_DENSITY,
		AT_START_DISTANCE,
		AT_END_DISTANCE,
		AT_FUNC,
		AT_COLOR,
		AT_ALPHA,
		AT_APPLY_TO_SKYBOX,
		AT_USE_FOG_ENDFAR
	};

public:

	PgWorldElementFog();
	bool	ReadFromXmlElement(TiXmlNode const *pkFogNode);

	bool	IsConditionCorrect()	const;

	float	GetDepth()	const	{	return	m_fDepth;	}
	float	GetFarDistance()	const	{	return	m_fFarDistance;	}
	float	GetDensity()	const	{	return	m_fDensity;	}
	float	GetStartDistance()	const	{	return	m_fStartDistance;	}
	float	GetEndDistance()	const	{	return	m_fEndDistance;	}
	float	GetAlpha()	const	{	return	m_fAlpha;	}
	bool	GetApplyToSkyBox()	const	{	return	m_bApplyToSkyBox;	}
	bool	GetUseFogEndFar()	const	{	return	m_bUseFogEndFar;	}
	std::string const& GetID() const	{	return	m_ID;	}
	NiColor	const&	GetFogColor()	const	{	return	m_kColor;	}
	NiFogProperty::FogFunction	const&	GetFogFunction()	const	{	return	m_kFogFunction;	}

	bool	IsTheAttributeValid(ATTRIBUTE_TYPE const &kAttributeType)	const
	{
		return	(0 != (m_dwValidAttribute&(1<<kAttributeType)));
	}

private:

	void	SetAttributeValid(ATTRIBUTE_TYPE const &kAttributeType)
	{
		m_dwValidAttribute |= (1<<kAttributeType);
	}

private:

	DWORD	m_dwValidAttribute;

	PgWorldTimeCondition	m_kTimeCondition;

	float	m_fDepth;
	float	m_fFarDistance;
	float	m_fDensity;
	float	m_fStartDistance;
	float	m_fEndDistance;
	NiFogProperty::FogFunction	m_kFogFunction;
	NiColor	m_kColor;
	float	m_fAlpha;
	bool	m_bApplyToSkyBox;
	bool	m_bUseFogEndFar;
	std::string	m_ID;
};

#endif // FREEDOM_DRAGONICA_SCENE_WORLD_WORLDELEMENT_PGWORLDELEMENTFOG_H