#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWCOMBOADVISOR_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWCOMBOADVISOR_H

class lwComboAdvisor
{
public:
	static void RegisterWrapper(lua_State *pkState);

public:


	void	OnNewActionEnter(char const* strActionID);
	void	ResetComboAdvisor();
	void	AddNextAction(char const* strActionID);
	void	ClearNextAction();

};

#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWCOMBOADVISOR_H