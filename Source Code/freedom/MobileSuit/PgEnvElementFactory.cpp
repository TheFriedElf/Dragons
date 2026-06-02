#include "stdafx.h"
#include "PgEnvElementFactory.H"

	
void	PgEnvElementFactory::ReleaseAllPrototype()
{
	m_kEnvElementPrototypeCont.clear();
}

void	PgEnvElementFactory::RegisterPrototype(PgEnvElement *pkEnvElement)
{
	assert(pkEnvElement);
	if(!pkEnvElement)
	{
		return;
	}

	PgEnvElement::ENV_ELEMENT_TYPE const &kEnvElementType = pkEnvElement->GetType();

	if(GetPrototype(kEnvElementType))
	{
		return;
	}

	m_kEnvElementPrototypeCont.insert(std::make_pair(kEnvElementType,pkEnvElement));
}
PgEnvElement*	PgEnvElementFactory::CreateEnvElement(PgEnvElement::ENV_ELEMENT_TYPE const &kEnvElementType)
{
	PgEnvElement	*pkPrototype = GetPrototype(kEnvElementType);
	if(!pkPrototype)
	{
		return	NULL;
	}

	return	pkPrototype->Clone();
}
PgEnvElement*	PgEnvElementFactory::GetPrototype(PgEnvElement::ENV_ELEMENT_TYPE const &kEnvElementType)
{

	EnvElementPrototypeMap::iterator itor = m_kEnvElementPrototypeCont.find(kEnvElementType);
	if(itor == m_kEnvElementPrototypeCont.end())
	{
		return	NULL;
	}

	PgEnvElement	*pkEnvElement = itor->second;
	return	pkEnvElement;
}