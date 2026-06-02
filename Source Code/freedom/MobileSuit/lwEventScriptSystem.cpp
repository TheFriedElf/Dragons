#include "stdafx.h"
#include "lwEventScriptSystem.H"
#include "PgEventScriptDataMan.H"

using namespace lua_tinker;

lwEventScriptSystem	lwGetEventScriptSystem()
{
	return	lwEventScriptSystem(&g_kEventScriptSystem);
}

void lwEventScriptSystem::RegisterWrapper(lua_State *pkState)
{
	def(pkState, "GetEventScriptSystem", &lwGetEventScriptSystem);

	LW_REG_CLASS(EventScriptSystem)
		LW_REG_METHOD(EventScriptSystem, ActivateEvent)
		LW_REG_METHOD(EventScriptSystem, DeactivateEvent)
		LW_REG_METHOD(EventScriptSystem, DeactivateAll)
		LW_REG_METHOD(EventScriptSystem, IsNowActivated)
		LW_REG_METHOD(EventScriptSystem, IsNowActivate)
		LW_REG_METHOD(EventScriptSystem, IsNowUpdate)
		LW_REG_METHOD(EventScriptSystem, ReloadAll)
		LW_REG_METHOD(EventScriptSystem, Reload)
		;
}
bool	lwEventScriptSystem::ActivateEvent(int iEventID,float fAccumTime,float fFrameTime)	//	이미 실행중인 이벤트를 다시 실행하는것은 안된다.
{
	return	m_pkEventScriptSystem->ActivateEvent(iEventID);
}
bool	lwEventScriptSystem::DeactivateEvent(int iEventID)
{
	return	m_pkEventScriptSystem->DeactivateEvent(iEventID);
}
bool	lwEventScriptSystem::DeactivateAll()
{
	return	m_pkEventScriptSystem->DeactivateAll();
}
bool	lwEventScriptSystem::IsNowActivated(int iEventID)
{
	return	m_pkEventScriptSystem->IsNowActivated(iEventID);
}
bool	lwEventScriptSystem::IsNowActivate()
{
	return	m_pkEventScriptSystem->IsNowActivate();
}
bool	lwEventScriptSystem::IsNowUpdate()
{
	return	m_pkEventScriptSystem->IsNowUpdate();
}
void	lwEventScriptSystem::ReloadAll()
{
	g_kEventScriptDataMan.ReloadAll();
}
void	lwEventScriptSystem::Reload(int iEventID)
{
	g_kEventScriptDataMan.Reload(iEventID);
}