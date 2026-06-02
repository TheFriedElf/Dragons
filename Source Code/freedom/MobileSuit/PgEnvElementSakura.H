#ifndef FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_ENVELEMENTSAKURA_PGENVELEMENTSAKURA_H
#define FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_ENVELEMENTSAKURA_PGENVELEMENTSAKURA_H
#include "PgEnvElementSimple.H"

class PgEnvElementSakura
	: public PgEnvElementSimple
{
public:
	PgEnvElementSakura();
	virtual ~PgEnvElementSakura() {};
	
	virtual	ENV_ELEMENT_TYPE	GetType()	const	{	return	GetTypeStatic();	}
	static	ENV_ELEMENT_TYPE	GetTypeStatic()	{	return	PgEnvElement::EET_SAKURA;	}
};



class PgEnvElementSunflower
	: public PgEnvElementSimple
{
public:
	PgEnvElementSunflower();
	virtual ~PgEnvElementSunflower() {};
	
	virtual	ENV_ELEMENT_TYPE	GetType()	const	{	return	GetTypeStatic();	}
	static	ENV_ELEMENT_TYPE	GetTypeStatic()	{	return	PgEnvElement::EET_SUNFLOWER;	}
};


class PgEnvElementIceCream
	: public PgEnvElementSimple
{
public:
	PgEnvElementIceCream();
	virtual ~PgEnvElementIceCream() {};
	
	virtual	ENV_ELEMENT_TYPE	GetType()	const	{	return	GetTypeStatic();	}
	static	ENV_ELEMENT_TYPE	GetTypeStatic()	{	return	PgEnvElement::EET_ICECREAM;	}

	//virtual void UpdateParticlePosition(NiCamera *pkCamera);
};


#endif // FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_ENVELEMENTSakura_PGENVELEMENTSAKURA_H