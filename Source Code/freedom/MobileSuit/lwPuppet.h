#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PUPPET_LWPUPPET_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PUPPET_LWPUPPET_H

#include "lwGUID.h"

class PgPuppet;

class lwPuppet
{
public:
	lwPuppet(PgPuppet *pkPuppet);
public:
	~lwPuppet(void);

	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//! PgPuppet로 바꿔준다.
	PgPuppet *lwPuppet::operator()();

	//! Puppet이 Nil인가!
	bool IsNil();

	//! 엑션을 구동한다.
	bool TransitAction(char const *pcActionName);

	//! 엑션을 구동한다.
	bool PlayAnimation(int iAnimationID);

	//! GUID를 반환한다.
	lwGUID GetGuid();

	//! GUID를 설정한다.
	void SetGuid(lwGUID kGuid);

	//! 지정한 파티클을 붙인다.
	bool AttachParticle(int iSlot, char const *pcTarget, char const *pcParticle);

	//! 현재 애니메이션 시퀀스 아이디를 반환한다
	int GetCurAnimation();

	//! Reload Nif
	void ReloadNif();

	int	GetWorldEventStateID();
	void	SetWorldEventStateID(int iNewID,bool bSetImmediate);


protected:
	PgPuppet *m_pkPuppet;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PUPPET_LWPUPPET_H