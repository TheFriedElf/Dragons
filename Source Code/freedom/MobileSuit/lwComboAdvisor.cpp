#include <stdafx.h>
#include "lwComboAdvisor.H"
#include "PgComboAdvisor.H"

using namespace lua_tinker;


lwComboAdvisor	lwGetComboAdvisor()
{
	return	lwComboAdvisor();
}


void lwComboAdvisor::RegisterWrapper(lua_State *pkState)
{

	def(pkState, "GetComboAdvisor", &lwGetComboAdvisor);

	// 등록한다.
	class_<lwComboAdvisor>(pkState, "ComboAdvisor")
		.def(pkState, "OnNewActionEnter", &lwComboAdvisor::OnNewActionEnter)
		.def(pkState, "ResetComboAdvisor", &lwComboAdvisor::ResetComboAdvisor)
		.def(pkState, "AddNextAction", &lwComboAdvisor::AddNextAction)
		.def(pkState, "ClearNextAction", &lwComboAdvisor::ClearNextAction)
		;

}
void	lwComboAdvisor::AddNextAction(char const* strActionID)
{
	g_kComboAdvisor.AddNextAction(strActionID);
}
void	lwComboAdvisor::ClearNextAction()
{
	g_kComboAdvisor.ClearNextAction();
}

void	lwComboAdvisor::OnNewActionEnter(char const* strActionID)
{
	g_kComboAdvisor.OnNewActionEnter(strActionID, false);
}
void	lwComboAdvisor::ResetComboAdvisor()
{
	g_kComboAdvisor.ResetComboAdvisor();
}
