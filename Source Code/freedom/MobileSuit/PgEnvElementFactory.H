#ifndef FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVELEMENTFACTORY_H
#define FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVELEMENTFACTORY_H

#include <NiMain.H>
#include "Loki/Factory.H"
#include "CreateUsingNiNew.inl"

class	PgEnvElement;

typedef Loki::Factory<PgEnvElement,PgEnvElement::ENV_ELEMENT_TYPE> PgEnvElementFactory;
#define g_kEnvElementFactory SINGLETON_STATIC(PgEnvElementFactory)

#define REGISTER_ENVELEMENT_TO_FACTORY(classname) \
class	classname##Creator	\
	{	\
	public:	\
		PgEnvElement*	operator()()	\
		{	\
			return	NiNew	classname;	\
		};	\
	};	\
	g_kEnvElementFactory.Register(##classname##::GetTypeStatic(),classname##Creator());	

#endif // FREEDOM_DRAGONICA_RENDER_ENVIRONMENT_PGENVELEMENTFACTORY_H