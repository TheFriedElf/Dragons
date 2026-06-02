#include "StdAfx.h"
#include "PgIXmlObject.h"

PgIXmlObject::PgIXmlObject()
{
	m_eID = ID_NONE;
}

PgIXmlObject::~PgIXmlObject()
{
}

std::string const &PgIXmlObject::GetID()const
{
	return m_kID;
}

void PgIXmlObject::SetID(char const *pcID)
{
	m_kID = pcID;
}