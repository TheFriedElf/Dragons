#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PROJECTILE_LWPROJECTILE_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PROJECTILE_LWPROJECTILE_H
#include "lwGUID.h"
#include "lwPoint3.h"
#include "lwActor.h"
#include "PgProjectile.h"

class	lwActionTargetInfo;
class	lwActionResult;
class	lwActionTargetList;
class	lwProjectileOptionInfo;

class	lwProjectile
{
public:

	lwProjectile(PgProjectile *pkProjectile);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	char const*	GetID();
	int	GetUID();
	int	GetState();

	int	GetParentActionNo();
	int	GetParentActionInstanceID();
	int	GetParentActionTimeStamp();
	lwGUID GetParentPilotGuid();
	void SetParentPilotGuid(lwGUID kGuid);

	void	SetParamValue(char const*pParamName,char const*pParamValue);
	char const*	GetParamValue(char const*pParamName);

	void	SetParam(float fSpeed,float fAccel,float fMass);
	void	SetTargetObject(lwActionTargetInfo kActionTargetInfo);
	void	SetTargetObjectList(lwActionTargetList kActionTargetInfoList);	//	리스트의 첫번째에 있는 놈이 목표위치가 된다.
	void	SetTargetLoc(lwPoint3	vTargetLoc);
	void	SetTargetGuidFromServer(lwGUID kGuid);
	void	SetTargetEndNode(char const* pcEndNode);
	char const* GetTargetEndNode()const;
	lwPoint3 GetNodeWorldPos(char const* pcNode)const;
	void	SetFireNode(char const* pcEndNode);
	char const* GetFireNode()const;
	void	Fire(bool bNoResetTargetLoc = false);
	void	DelayFire(float fDelayTime, bool bNoResetTargetLoc = false);	//	일정 시간 기다린 후에 자동으로 발사된다.
	void	LoadToWeapon(lwActor kActor);
	void	LoadToHelper(lwActor kActor,char *strTargetHelper);
	void	LoadToPosition(lwPoint3 kPos);
	void 	OnRemove();

	float	GetCurrentSpeed();

	void	SetMultipleAttack(bool bEnable);
	bool	GetMultipleAttack();

	void	SetAlpha(float fAlpha);
	void	SetScale(float fScale);
	float	GetScale();

	lwActionTargetInfo	GetActionTargetInfo(int iIndex);
	lwPoint3	GetTargetLoc();
	lwPoint3	GetFireStartPos();
	int	GetActionTargetCount();
	lwActionTargetList	GetActionTargetList();

	float	GetOriginalSpeed();
	float	GetOriginalAccelation();


	lwPoint3	GetWorldPos();
	void	SetWorldPos(lwPoint3 kPos);

	void	SetVelocity(lwPoint3 kVel);
	void	SetSpeed(float fSpeed);
	float	GetSpeed() const;

	void	SetMovingType(int kMovingType);
	int	GetMovingType();

	void	SetParam_SinCurveLineType(float fSinPower,float fSinRotateAngle);
	void	SetParam_Bezier4SplineType(lwPoint3 p1,lwPoint3 p2,lwPoint3 p3,lwPoint3 p4);


	void	SetParentActionInfo(int iActionNo,int iActionInstanceID,int iTimeStamp);

	PgProjectile *operator()();

	bool	IsNil();
	void	SetHide(bool bHide);
	bool	GetHide();
	void	AttachSound(char const *pcSoundID, float fMin, float fMax);
	void	SetRotate(lwQuaternion kQuat);

	lwPoint3 GetDirection();
	lwPoint3 GetUp();
	lwPoint3 GetRight();

	int  GetPenetrationCount() const;
	void SetPenetrationCount(int const iCount);
	
	lwProjectileOptionInfo GetOptionInfo();
	bool SetOptionInfo(lwProjectileOptionInfo kOptionInfo);

	void SetCollisionCheckSec(float const fCheckTime);
	float GetCollisionCheckSec()  const;
	float GetHomingTime()const;
	float GetHomingDelayTime()const;
private:

	PgProjectile	*m_pkProjectile;
	
};

class lwProjectileOptionInfo
{
public:
	lwProjectileOptionInfo();
	lwProjectileOptionInfo(PgProjectile::SOptionInfo* const pkProjectileOption);
	~lwProjectileOptionInfo();
	
	static bool RegisterWrapper(lua_State *pkState);
	
	void SetAutoRemoveIfNoneTarget(bool bAutoRemove);
	bool GetAutoRemoveIfNoneTarget();
	
	PgProjectile::SOptionInfo* GetSelf() const;

	bool IsNil();
private:
	PgProjectile::SOptionInfo* m_pkOption;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PROJECTILE_LWPROJECTILE_H