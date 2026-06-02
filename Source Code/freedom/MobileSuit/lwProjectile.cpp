#include "stdafx.h"
#include "lwProjectile.h"
#include "lwActionTargetInfo.H"
#include "lwActionResult.H"
#include "lwActionTargetList.H"
#include "lwQuaternion.h"

lwProjectile::lwProjectile(PgProjectile *pkProjectile)
{
	m_pkProjectile = pkProjectile;
}

//! 스크립팅 시스템에 등록한다.
bool lwProjectile::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwProjectile>(pkState, "Projectile")
		.def(pkState, constructor<PgProjectile*>())
		.def(pkState, "GetID", &lwProjectile::GetID)
		.def(pkState, "GetState", &lwProjectile::GetState)
		.def(pkState, "SetParam", &lwProjectile::SetParam)
		.def(pkState, "SetTargetObject", &lwProjectile::SetTargetObject)
		.def(pkState, "SetTargetObjectList", &lwProjectile::SetTargetObjectList)
		.def(pkState, "SetTargetLoc", &lwProjectile::SetTargetLoc)
		.def(pkState, "SetTargetGuidFromServer", &lwProjectile::SetTargetGuidFromServer)
		.def(pkState, "SetTargetEndNode", &lwProjectile::SetTargetEndNode)
		.def(pkState, "GetTargetEndNode", &lwProjectile::GetTargetEndNode)
		.def(pkState, "GetNodeWorldPos", &lwProjectile::GetNodeWorldPos)
		.def(pkState, "SetFireNode", &lwProjectile::SetFireNode)
		.def(pkState, "GetFireNode", &lwProjectile::GetFireNode)
		.def(pkState, "Fire", &lwProjectile::Fire)
		.def(pkState, "DelayFire", &lwProjectile::DelayFire)
		.def(pkState, "LoadToWeapon", &lwProjectile::LoadToWeapon)
		.def(pkState, "LoadToHelper", &lwProjectile::LoadToHelper)
		.def(pkState, "LoadToPosition", &lwProjectile::LoadToPosition)
		.def(pkState, "GetWorldPos", &lwProjectile::GetWorldPos)
		.def(pkState, "SetWorldPos", &lwProjectile::SetWorldPos)
		.def(pkState, "GetUID", &lwProjectile::GetUID)
		.def(pkState, "IsNil", &lwProjectile::IsNil)
		.def(pkState, "GetActionTargetInfo", &lwProjectile::GetActionTargetInfo)
		.def(pkState, "GetActionTargetCount", &lwProjectile::GetActionTargetCount)
		.def(pkState, "GetActionTargetList", &lwProjectile::GetActionTargetList)
		.def(pkState, "GetTargetLoc", &lwProjectile::GetTargetLoc)
		.def(pkState, "GetParentActionNo", &lwProjectile::GetParentActionNo)
		.def(pkState, "GetParentActionInstanceID", &lwProjectile::GetParentActionInstanceID)
		.def(pkState, "GetParentActionTimeStamp", &lwProjectile::GetParentActionTimeStamp)
		.def(pkState, "GetParentPilotGuid", &lwProjectile::GetParentPilotGuid)
		.def(pkState, "SetParentPilotGuid", &lwProjectile::SetParentPilotGuid)		
		.def(pkState, "SetParamValue", &lwProjectile::SetParamValue)
		.def(pkState, "GetParamValue", &lwProjectile::GetParamValue)
		.def(pkState, "SetVelocity", &lwProjectile::SetVelocity)
		.def(pkState, "SetSpeed", &lwProjectile::SetSpeed)
		.def(pkState, "GetSpeed", &lwProjectile::GetSpeed)
		.def(pkState, "SetMovingType", &lwProjectile::SetMovingType)
		.def(pkState, "GetMovingType", &lwProjectile::GetMovingType)
		.def(pkState, "SetParam_SinCurveLineType", &lwProjectile::SetParam_SinCurveLineType)
		.def(pkState, "SetParam_Bezier4SplineType", &lwProjectile::SetParam_Bezier4SplineType)
		.def(pkState, "SetParentActionInfo", &lwProjectile::SetParentActionInfo)
		.def(pkState, "GetCurrentSpeed", &lwProjectile::GetCurrentSpeed)
		.def(pkState, "SetHide", &lwProjectile::SetHide)
		.def(pkState, "GetHide", &lwProjectile::GetHide)
		.def(pkState, "SetAlpha", &lwProjectile::SetAlpha)
		.def(pkState, "SetScale", &lwProjectile::SetScale)
		.def(pkState, "GetFireStartPos", &lwProjectile::GetFireStartPos)
		.def(pkState, "GetOriginalSpeed", &lwProjectile::GetOriginalSpeed)
		.def(pkState, "GetOriginalAccelation", &lwProjectile::GetOriginalAccelation)
		.def(pkState, "AttachSound", &lwProjectile::AttachSound)
		.def(pkState, "SetMultipleAttack", &lwProjectile::SetMultipleAttack)
		.def(pkState, "GetMultipleAttack", &lwProjectile::GetMultipleAttack)
		.def(pkState, "SetRotate", &lwProjectile::SetRotate)
		.def(pkState, "GetScale", &lwProjectile::GetScale)
		.def(pkState, "GetDirection", &lwProjectile::GetDirection)
		.def(pkState, "GetUp", &lwProjectile::GetUp)
		.def(pkState, "GetRight", &lwProjectile::GetRight)
		.def(pkState, "GetRight", &lwProjectile::GetRight)
		.def(pkState, "GetPenetrationCount", &lwProjectile::GetPenetrationCount)
		.def(pkState, "SetPenetrationCount", &lwProjectile::SetPenetrationCount)
		.def(pkState, "GetOptionInfo", &lwProjectile::GetOptionInfo)
		.def(pkState, "SetOptionInfo", &lwProjectile::SetOptionInfo)
		.def(pkState, "SetCollisionCheckSec", &lwProjectile::SetCollisionCheckSec)
		.def(pkState, "GetCollisionCheckSec", &lwProjectile::GetCollisionCheckSec)
		.def(pkState, "GetHomingTime", &lwProjectile::GetHomingTime)
		.def(pkState, "GetHomingDelayTime", &lwProjectile::GetHomingDelayTime)
		.def(pkState, "OnRemove", &lwProjectile::OnRemove)
		
		;

	return true;
}
void	lwProjectile::DelayFire(float fDelayTime, bool bNoResetTargetLoc)
{
	m_pkProjectile->DelayFire(fDelayTime, bNoResetTargetLoc);
}

float	lwProjectile::GetOriginalSpeed()
{
	return	m_pkProjectile->GetOriginalSpeed();
}
float	lwProjectile::GetOriginalAccelation()
{
	return	m_pkProjectile->GetOriginalAccelation();
}

void	lwProjectile::SetScale(float fScale)
{
	m_pkProjectile->SetScale(fScale);
}
float	lwProjectile::GetScale()
{
	return	m_pkProjectile->GetScale();
}
lwPoint3	lwProjectile::GetFireStartPos()
{
	return	lwPoint3(m_pkProjectile->GetFireStartPos());
}
int	lwProjectile::GetParentActionNo()
{
	return	m_pkProjectile->GetParentActionNo();
}
int	lwProjectile::GetParentActionInstanceID()
{
	return	m_pkProjectile->GetParentActionInstanceID();
}
int	lwProjectile::GetParentActionTimeStamp()
{
	return	m_pkProjectile->GetParentActionTimeStamp();
}

lwGUID lwProjectile::GetParentPilotGuid()
{
	return	lwGUID(m_pkProjectile->GetParentPilotGUID());
}

void lwProjectile::SetParentPilotGuid(lwGUID kGuid)
{
	m_pkProjectile->SetParentPilot(kGuid());
}

void	lwProjectile::SetParentActionInfo(int iActionNo,int iActionInstanceID,int iTimeStamp)
{
	m_pkProjectile->SetParentActionInfo(iActionNo,iActionInstanceID,iTimeStamp);
}

int	lwProjectile::GetUID()
{
	return	m_pkProjectile->GetUID();
}
void	lwProjectile::SetAlpha(float fAlpha)
{
	m_pkProjectile->SetAlpha(fAlpha);
}	
void	lwProjectile::SetMultipleAttack(bool bEnable)
{
	m_pkProjectile->SetMultipleAttack(bEnable);
}
bool	lwProjectile::GetMultipleAttack()
{
	return	m_pkProjectile->IsMultipleAttack();
}

float	lwProjectile::GetCurrentSpeed()
{
	return	m_pkProjectile->GetCurrentSpeed();
}
void	lwProjectile::SetParamValue(char const*pParamName,char const*pParamValue)
{
	m_pkProjectile->SetParam(pParamName,pParamValue); 
}
char const*	lwProjectile::GetParamValue(char const*pParamName)
{
	return	m_pkProjectile->GetParam(pParamName);
}

char const*	lwProjectile::GetID()
{
	return	m_pkProjectile->GetID().c_str();
}
int	lwProjectile::GetState()
{
	return	(int)m_pkProjectile->GetState();
}
bool	lwProjectile::IsNil()
{
	return	m_pkProjectile==NULL;
}
lwActionTargetInfo	lwProjectile::GetActionTargetInfo(int iIndex)
{
	return	lwActionTargetInfo((PgActionTargetInfo*)m_pkProjectile->GetActionTargetInfo(iIndex));
}
lwPoint3	lwProjectile::GetTargetLoc()
{
	return	lwPoint3(m_pkProjectile->GetTargetLoc());
}
int	lwProjectile::GetActionTargetCount()
{
	return	m_pkProjectile->GetActionTargetCount();
}
lwActionTargetList	lwProjectile::GetActionTargetList()
{
	return	lwActionTargetList((PgActionTargetList*)&m_pkProjectile->GetActionTargetList());
}

lwPoint3	lwProjectile::GetWorldPos()
{
	return	lwPoint3(m_pkProjectile->GetWorldPos());
}
void	lwProjectile::SetWorldPos(lwPoint3 kPos)
{
	m_pkProjectile->SetWorldPos(kPos());
}
void	lwProjectile::SetVelocity(lwPoint3 kVel)
{
	m_pkProjectile->SetVelocity(kVel());
}
void lwProjectile::SetSpeed(float fSpeed)
{
	m_pkProjectile->SetSpeed(fSpeed);
}
float lwProjectile::GetSpeed() const
{
	if(m_pkProjectile)
	{
		return m_pkProjectile->GetSpeed();
	}

	return 0.0f;	
}
void	lwProjectile::SetMovingType(int kMovingType)
{
	m_pkProjectile->SetMovingType((PgProjectile::MovingType)kMovingType);
}
int	lwProjectile::GetMovingType()
{
	return	m_pkProjectile->GetMovingType();
}
void	lwProjectile::SetParam_SinCurveLineType(float fSinPower,float fSinRotateAngle)
{
	m_pkProjectile->SetParam_SinCurveLineType(fSinPower,fSinRotateAngle);
}
void	lwProjectile::SetParam_Bezier4SplineType(lwPoint3 p1,lwPoint3 p2,lwPoint3 p3,lwPoint3 p4)
{
	m_pkProjectile->SetParam_Bezier4SplineType(p1(),p2(),p3(),p4());
}

void	lwProjectile::SetParam(float fSpeed,float fAccel,float fMass)
{
	m_pkProjectile->SetParam(fSpeed,fAccel,fMass);
}
void	lwProjectile::SetTargetObject(lwActionTargetInfo kActionTargetInfo)
{
	m_pkProjectile->SetTargetObject(*kActionTargetInfo());
}
void	lwProjectile::SetTargetObjectList(lwActionTargetList kActionTargetInfoList)	//	리스트의 첫번째에 있는 놈이 목표위치가 된다.
{
	m_pkProjectile->SetTargetObjectList(*kActionTargetInfoList());
}

void	lwProjectile::Fire(bool bNoResetTargetLoc)
{
	m_pkProjectile->Fire(bNoResetTargetLoc);
}
void	lwProjectile::SetTargetLoc(lwPoint3	vTargetLoc)
{
	m_pkProjectile->SetTargetLoc(vTargetLoc());
}
void	lwProjectile::LoadToHelper(lwActor kActor,char *strTargetHelper)
{
	m_pkProjectile->LoadToHelper(kActor(),strTargetHelper);
}

void	lwProjectile::LoadToWeapon(lwActor kActor)
{
	m_pkProjectile->LoadToWeapon(kActor());
}

void	lwProjectile::LoadToPosition(lwPoint3 kPos)
{
	m_pkProjectile->LoadToPosition(kPos());
}

PgProjectile *lwProjectile::operator()()
{
	return	m_pkProjectile;
}

void	lwProjectile::SetHide(bool bHide)
{
	m_pkProjectile->SetHide(bHide);
}

bool	lwProjectile::GetHide()
{
	return m_pkProjectile->GetHide();
}

void	lwProjectile::AttachSound(char const *pcSoundID, float fMin, float fMax)
{
	m_pkProjectile->AttachSound(pcSoundID, fMin, fMax);
}

void	lwProjectile::SetRotate(lwQuaternion kQuat)
{
	if(m_pkProjectile)
	{
		m_pkProjectile->SetRotate(kQuat());
	}
}

void	lwProjectile::SetTargetGuidFromServer(lwGUID kGuid)
{
	if (m_pkProjectile && !kGuid.IsNil())
	{
		m_pkProjectile->TargetGuid(kGuid.GetGUID());
	}
}

void	lwProjectile::SetTargetEndNode(char const* pcEndNode)
{
	if(m_pkProjectile && pcEndNode)
	{
		m_pkProjectile->TargetEndNode(pcEndNode);
	}
}

char const* lwProjectile::GetTargetEndNode()const
{
	if(m_pkProjectile)
	{
		return m_pkProjectile->TargetEndNode().c_str();
	}
	return NULL;
}

lwPoint3 lwProjectile::GetNodeWorldPos(char const* pcNode)const
{
	if(m_pkProjectile)
	{
		return m_pkProjectile->GetNodeWorldPos(pcNode);
	}
	return lwPoint3(0,0,0);
}

void	lwProjectile::SetFireNode(char const* pcNode)
{
	if(m_pkProjectile && pcNode)
	{
		m_pkProjectile->FireNode(pcNode);
	}
}

char const* lwProjectile::GetFireNode()const
{
	if(m_pkProjectile)
	{
		return m_pkProjectile->FireNode().c_str();
	}
	return "";
}

lwPoint3 lwProjectile::GetDirection()
{
	if(m_pkProjectile)
	{
		return lwPoint3(m_pkProjectile->GetDirection());
	}

	return lwPoint3(0.0f, 0.0f, 0.0f);

}
lwPoint3 lwProjectile::GetUp()
{
	if(m_pkProjectile)
	{
		return lwPoint3(m_pkProjectile->GetUp());
	}

	return lwPoint3(0.0f, 0.0f, 0.0f);
}
lwPoint3 lwProjectile::GetRight()
{
	if(m_pkProjectile)
	{
		return lwPoint3(m_pkProjectile->GetRight());
	}

	return lwPoint3(0.0f, 0.0f, 0.0f);
}

int lwProjectile::GetPenetrationCount() const
{
	if(m_pkProjectile)
	{
		return m_pkProjectile->PenetrationCount();
	}

	return 0;
}

void lwProjectile::SetPenetrationCount(int const iCount)
{
	if(m_pkProjectile)
	{
		m_pkProjectile->PenetrationCount(iCount);
	}
}

lwProjectileOptionInfo lwProjectile::GetOptionInfo()
{
	if(m_pkProjectile)
	{
		return lwProjectileOptionInfo(&m_pkProjectile->m_kOptionInfo);
	}
	return lwProjectileOptionInfo();
}

bool lwProjectile::SetOptionInfo(lwProjectileOptionInfo kOptionInfo)
{
	if(m_pkProjectile)
	{
		if(kOptionInfo.GetSelf())
		{
			m_pkProjectile->SetOptionInfo(*kOptionInfo.GetSelf());
			return true;
		}
	}
	return false;
}

void lwProjectile::SetCollisionCheckSec(float const fCheckTime)
{
	if(m_pkProjectile)
	{
		m_pkProjectile->SetCollisionCheckSec(fCheckTime);
	}
}

float lwProjectile:: GetCollisionCheckSec()  const
{
	if(m_pkProjectile)
	{
		return m_pkProjectile->GetCollisionCheckSec();
	}
	return 0.0f;
}

float lwProjectile::GetHomingTime()const
{
	return m_pkProjectile ? m_pkProjectile->HomingTime() : 0.0f;
}

float lwProjectile::GetHomingDelayTime()const
{
	return m_pkProjectile ? m_pkProjectile->HomingDelayTime() : 0.0f;
}

void lwProjectile::OnRemove()
{
	if(m_pkProjectile)
	{
		m_pkProjectile->OnRemove();
	}
}

// lwProjectileOptionInfo ----------------------------------------------------------------------------------------------------------------
lwProjectileOptionInfo::lwProjectileOptionInfo(PgProjectile::SOptionInfo* const pkProjectileOption)
{
	m_pkOption = pkProjectileOption;
}

lwProjectileOptionInfo::lwProjectileOptionInfo()
:m_pkOption(NULL)
{
}

lwProjectileOptionInfo::~lwProjectileOptionInfo()
{	
}

bool lwProjectileOptionInfo::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwProjectileOptionInfo>(pkState, "ProjectileOption")
		.def(pkState, constructor<PgProjectile::SOptionInfo*>())
		.def(pkState, "IsNil", &lwProjectileOptionInfo::IsNil)
		.def(pkState, "SetAutoRemoveIfNoneTarget", &lwProjectileOptionInfo::SetAutoRemoveIfNoneTarget)
		.def(pkState, "GetAutoRemoveIfNoneTarget", &lwProjectileOptionInfo::GetAutoRemoveIfNoneTarget)		
		;
	return true;	
}

PgProjectile::SOptionInfo* lwProjectileOptionInfo::GetSelf() const
{
	if(m_pkOption)
	{
		return m_pkOption;
	}
	return NULL;
}

bool lwProjectileOptionInfo::IsNil()
{
	return	m_pkOption==NULL;
}

void lwProjectileOptionInfo::SetAutoRemoveIfNoneTarget(bool bAutoRemove)
{
	if(m_pkOption)
	{
		m_pkOption->bRemoveNoneTargetProjectile = bAutoRemove;
	}
}

bool lwProjectileOptionInfo::GetAutoRemoveIfNoneTarget()
{
	if(m_pkOption)
	{
		return m_pkOption->bRemoveNoneTargetProjectile;
	}
	return false;
}