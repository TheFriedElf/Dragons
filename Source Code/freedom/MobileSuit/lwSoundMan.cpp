#include "stdafx.h"
#include "lwSoundMan.h"
#include "PgSoundMan.h"

lwSoundMan 	GetSoundMan()
{
	return	lwSoundMan();
}
lwSoundMan::lwSoundMan()
{
}

bool lwSoundMan::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "GetSoundMan", &GetSoundMan);


	class_<lwSoundMan>(pkState, "SoundMan")
		.def(pkState, constructor<void>())
		.def(pkState, "SetEnvSoundDebugRenderingOn", &lwSoundMan::SetEnvSoundDebugRenderingOn)
		.def(pkState, "SetEnvSoundDebugRenderingOff", &lwSoundMan::SetEnvSoundDebugRenderingOff)
		;

	return true;
}

void lwSoundMan::SetEnvSoundDebugRenderingOn()
{
	g_kSoundMan.SetEvnSoundDebugRendering(true);
}
void lwSoundMan::SetEnvSoundDebugRenderingOff()
{
	g_kSoundMan.SetEvnSoundDebugRendering(false);
}