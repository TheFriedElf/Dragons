#include "stdafx.h"
#include "lwCameraModeEvent.H"
#include "PgEventScriptDataMan.H"
#include "lwGUID.H"
#include "lwPoint3.H"

using namespace lua_tinker;

void lwCameraModeEvent::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(CameraModeEvent)
		LW_REG_METHOD(CameraModeEvent, StartMoveCamera)
		LW_REG_METHOD(CameraModeEvent, SetTargetActor)
		LW_REG_METHOD(CameraModeEvent, SetTargetPos)
		LW_REG_METHOD(CameraModeEvent, GetState)
		;	
}

void	lwCameraModeEvent::StartMoveCamera(float fMoveTime,float fCamDistance,float fCamMoveUpDistance,float fHorizRotAngle,float fVertRotAngle)
{
	if(!m_pkCameraModeEvent)
	{
		return;
	}

	m_pkCameraModeEvent->StartMoveCamera(fMoveTime,fCamDistance,fCamMoveUpDistance,fHorizRotAngle,fVertRotAngle);
}

void	lwCameraModeEvent::SetTargetActor(lwGUID kTargetActor)
{
	if(!m_pkCameraModeEvent)
	{
		return;
	}

	m_pkCameraModeEvent->SetTargetActor(kTargetActor());
}
void	lwCameraModeEvent::SetTargetPos(lwPoint3 kTargetPos)
{
	if(!m_pkCameraModeEvent)
	{
		return;
	}
	m_pkCameraModeEvent->SetTargetPos(kTargetPos());
}
int		lwCameraModeEvent::GetState()
{
	if(!m_pkCameraModeEvent)
	{
		return 0;
	}
	return	m_pkCameraModeEvent->GetState();
}
