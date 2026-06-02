
#include "stdafx.h"
#include "lwSkillTargetMan.h"
#include "PgSkillTargetMan.h"
#include "lwGUID.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "lwPilot.h"
#include "lwPoint3.h"
//#ifndef USE_INB
lwSkillTargetMan	GetSkillTargetMan()
{
	return	lwSkillTargetMan();
}
lwSkillTargetMan::lwSkillTargetMan()
{
}

bool lwSkillTargetMan::RegisterWrapper(lua_State *pkState)
{

	using namespace lua_tinker;

	def(pkState, "GetSkillTargetMan", &GetSkillTargetMan);


	class_<lwSkillTargetMan>(pkState, "SkillTargetMan")
		.def(pkState, constructor<void>())
		.def(pkState, "EnableTargetAreaDisplay", &lwSkillTargetMan::EnableTargetAreaDisplay)
		.def(pkState, "SetTargetAreaCircle", &lwSkillTargetMan::SetTargetAreaCircle)
		.def(pkState, "SetTargetAreaTriangle", &lwSkillTargetMan::SetTargetAreaTriangle)
		.def(pkState, "SetTargetAreaRectangle", &lwSkillTargetMan::SetTargetAreaRectangle)
		.def(pkState, "SetDrawTargetMark", &lwSkillTargetMan::SetDrawTargetMark)
		;
	return true;
}
void lwSkillTargetMan::SetDrawTargetMark(bool bDraw)
{
	g_kSkillTargetMan.SetDrawTargetMark(bDraw);
	g_kMonSkillTargetMan.SetDrawTargetMark(bDraw);
}
void	lwSkillTargetMan::EnableTargetAreaDisplay(bool bEnable)
{
	g_kSkillTargetMan.EnableTargetAreaDisplay(bEnable);
	g_kMonSkillTargetMan.EnableTargetAreaDisplay(bEnable);
}
void	lwSkillTargetMan::SetTargetAreaCircle(lwPoint3 kCenter,float fRange)
{
	//g_kSkillTargetMan.SetTargetAreaCircle(kCenter(),fRange);
	g_kMonSkillTargetMan.SetTargetAreaCircle(kCenter(),fRange);
}
void	lwSkillTargetMan::SetTargetAreaTriangle(lwPoint3 kStart,lwPoint3 kDir,float fAngle,float fRange)
{
	//g_kSkillTargetMan.SetTargetAreaTriangle(kStart(),kDir(),fAngle,fRange);
	g_kMonSkillTargetMan.SetTargetAreaTriangle(kStart(),kDir(),fAngle,fRange);
}
void	lwSkillTargetMan::SetTargetAreaRectangle(lwPoint3 kStart,lwPoint3 kDir,float fSideRange,float fRange)
{
	//g_kSkillTargetMan.SetTargetAreaRectangle(kStart(),kDir(),fSideRange,fRange);
	g_kMonSkillTargetMan.SetTargetAreaRectangle(kStart(),kDir(),fSideRange,fRange);
}
//#endif//#ifndef USE_INB
