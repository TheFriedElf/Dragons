#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWSKILLTARGETMAN_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWSKILLTARGETMAN_H

//#ifndef USE_INB
class	PgSkillTargetMan;
class	lwGUID;
class	lwPoint3;
class	lwPilot;
class	lwSkillTargetMan
{

public:

	lwSkillTargetMan();

	static bool RegisterWrapper(lua_State *pkState);

	bool	IsNil();
	void SetDrawTargetMark(bool bDraw);

	void	EnableTargetAreaDisplay(bool bEnable);
	void	SetTargetAreaCircle(lwPoint3 kCenter,float fRange);
	void	SetTargetAreaTriangle(lwPoint3 kStart,lwPoint3 kDir,float fAngle,float fRange);
	void	SetTargetAreaRectangle(lwPoint3 kStart,lwPoint3	kDir,float fSideRange,float fRange);


};

//#endif//USE_INB
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWSKILLTARGETMAN_H