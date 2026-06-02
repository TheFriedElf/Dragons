#include "StdAfx.h"
#include "lwFurniture.h"
#include "PgFurniture.h"

using namespace lua_tinker;

lwFurniture lwNewFurniture(char const *pcFurnitureID)
{
	PgFurniture *pkFurniture = dynamic_cast<PgFurniture *>(PgXmlLoader::CreateObject(pcFurnitureID));
	return lwFurniture((PgFurniture *)pkFurniture);
}

lwFurniture lwNewFurnitureFromFile(char const *pcXmlPath)
{
	PgFurniture *pkFurniture = dynamic_cast<PgFurniture *>(PgXmlLoader::CreateObjectFromFile(pcXmlPath));
	return lwFurniture((PgFurniture *)pkFurniture);
}

lwFurniture lwToFurniture(PgIWorldObject *pkObject)
{
	return (lwFurniture)((PgFurniture *)pkObject);
}

lwFurniture::lwFurniture(PgFurniture *pkFurniture)
{
	m_pkFurniture = pkFurniture;
}

bool lwFurniture::RegisterWrapper(lua_State *pkState)
{
	def(pkState, "NewFurnitureFromFile", lwNewFurnitureFromFile);
	def(pkState, "NewFurniture", lwNewFurniture);
	def(pkState, "ToFurniture", lwToFurniture);

	class_<lwFurniture>(pkState, "Furniture")
		.def(pkState, constructor<PgFurniture *>())
		.def(pkState, "IsNil", &lwFurniture::IsNil)
		.def(pkState, "IncRotate", &lwFurniture::IncRotate)
		;

	return true;
}

bool lwFurniture::IsNil()
{
	return (m_pkFurniture == 0 ? true : false);
}

PgFurniture *lwFurniture::operator()()
{
	return m_pkFurniture;
}

void lwFurniture::IncRotate(float fAngle)
{
	m_pkFurniture->IncRotate(fAngle);
}