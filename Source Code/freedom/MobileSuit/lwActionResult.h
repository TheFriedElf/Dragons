#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONRESULT_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONRESULT_H
#include "PgScripting.h"
#include "lwGUID.h"
#include "Variant\PgActionResult.h"

LW_CLASS(PgActionResult, ActionResult)
	void SetMissed(bool bMissed);
	bool GetMissed();
	void SetValue(int const iValue);
	int GetValue();
	void AddEffect(int const iEffect);
	int GetEffectNum();
	int GetEffect(int const iEffectIndex);
	void Init();
	void SetBlocked(BYTE const byBlocked);
	BYTE GetBlocked();
	void SetComic(bool const bComic);
	bool GetComic();
	void SetDead(bool const bDead);
	bool GetDead();
	void SetInvalid(bool const bInvalid);
	bool GetInvalid();
	void SetDodged(bool const bDodged);
	bool GetDodged();
	void SetCritical(bool const bCritical);
	bool GetCritical();
	void SetRemainHP(int const iHP);
	bool IsMissed(); // GetMissed()와의 차이 : IsMissed() = GetMissed() or GetBlocked() or GetDodged() / 즉 어떤 이유로든 안맞았으면 true를 리턴하는 함수이다.
	void SetAbil(WORD wAbil, int iValue);
	int GetAbil(WORD wAbil);
LW_CLASS_END;

LW_CLASS(PgActionResultVector, ActionResultVector)
	lwActionResult GetResult(lwGUID kGuid, bool bMake);
LW_CLASS_END;

#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONRESULT_H