#ifndef FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_ENVELEMENTSNOW_PGENVELEMENTSNOW_H
#define FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_ENVELEMENTSNOW_PGENVELEMENTSNOW_H
#include "PgEnvElementSimple.H"

class PgEnvElementSnow
	: public PgEnvElementSimple
{
public:
	PgEnvElementSnow();
	virtual ~PgEnvElementSnow();
	
	virtual	ENV_ELEMENT_TYPE	GetType()	const	{	return	GetTypeStatic();	}
	static	ENV_ELEMENT_TYPE	GetTypeStatic()	{	return	PgEnvElement::EET_SNOW;	}
};

#endif // FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_ENVELEMENTSNOW_PGENVELEMENTSNOW_H