#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWZONEDRAWING_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWZONEDRAWING_H

class	PgSkillTargetMan;
class	lwGUID;
class	lwPoint3;
class	lwPilot;

class	lwZoneDrawing
{
public:
	lwZoneDrawing();

	static bool RegisterWrapper(lua_State *pkState);

	bool	IsNil();
	void	SetZoneDraw(bool bDraw);
	void	SetWholeDraw(bool bOpt);
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWZONEDRAWING_H