#include "stdafx.h"
#include "lwZoneDrawing.h"
#include "PgZoneDrawing.h"
#include "lwGUID.h"
#include "lwPilot.h"
#include "lwPoint3.h"

#ifndef USE_INB

lwZoneDrawing	GetZoneDrawer()
{
	return	lwZoneDrawing();
}
lwZoneDrawing::lwZoneDrawing()
{
}

bool lwZoneDrawing::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "GetZoneDrawer", &GetZoneDrawer);
	class_<lwZoneDrawing>(pkState, "ZoneDrawer")
		.def(pkState, constructor<void>())
		.def(pkState, "SetZoneDraw", &lwZoneDrawing::SetZoneDraw)
		.def(pkState, "SetWholeDraw", &lwZoneDrawing::SetWholeDraw)
		;

	return true;
}

void lwZoneDrawing::SetZoneDraw(bool bDraw)
{
	g_kZoneDrawer.SetDraw(bDraw);
}

void lwZoneDrawing::SetWholeDraw(bool bOpt)
{
	g_kZoneDrawer.SetWholeDraw(bOpt);
}

#endif//USE_INB