#include "stdafx.h"
#include "lwHelpObject.H"

using namespace lua_tinker;

void lwHelpObject::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(HelpObject)
		LW_REG_METHOD(HelpObject, GetUIType)
		LW_REG_METHOD(HelpObject, GetID)
		LW_REG_METHOD(HelpObject, GetTitleTTID)
		LW_REG_METHOD(HelpObject, GetMessageTTID)
		LW_REG_METHOD(HelpObject, GetHelperMsgTTID)
		LW_REG_METHOD(HelpObject, GetRelatedUIName)
		LW_REG_METHOD(HelpObject, GetCallEventID)
		LW_REG_METHOD(HelpObject, SetDoNotActivateAgain)
		LW_REG_METHOD(HelpObject, GetDoNotActivateAgain)
		;
}
int lwHelpObject::GetUIType()
{
	return	m_pkHelpObject->GetUIType();
}
int	lwHelpObject::GetID()
{
	return	m_pkHelpObject->GetID();
}
int	lwHelpObject::GetTitleTTID()
{
	return	m_pkHelpObject->GetTitleTTID();
}
int	lwHelpObject::GetMessageTTID()
{
	return	m_pkHelpObject->GetMessageTTID();
}
int	lwHelpObject::GetHelperMsgTTID()
{
	return	m_pkHelpObject->GetHelperMsgTTID();
}
char const*	lwHelpObject::GetRelatedUIName()
{
	return	m_pkHelpObject->GetRelatedUIName().c_str();
}
int lwHelpObject::GetCallEventID()
{
	return  m_pkHelpObject->GetCallEventID();
}
void	lwHelpObject::SetDoNotActivateAgain(bool bDoNotActivateAgain)
{
	m_pkHelpObject->SetDoNotActivateAgain(bDoNotActivateAgain);
}
bool	lwHelpObject::GetDoNotActivateAgain()
{
	return	m_pkHelpObject->GetDoNotActivateAgain();
}