#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONTARGETLIST_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONTARGETLIST_H

#include "PgScripting.h"
#include "lwPoint3.h"

class	lwActionTargetInfo;
class	lwGUID;
class	lwAction;

LW_CLASS(PgActionTargetList, ActionTargetList)

	~lwActionTargetList();
	int		size();
	void	clear();
	lwActionTargetInfo	GetTargetInfo(int iIndex);
	void	AddTarget(lwActionTargetInfo kActionTargetInfo);
	void	DeleteTargetInfo(int iIndex);
	void	DeleteTargetInfoGUID(lwGUID kGUID);
	bool	IsActionEffectApplied();
	void	SetActionEffectApplied(bool bApplied);
	void	ApplyActionEffects(bool bOnlyDieEffect,bool bNoShowDamageNum, float fRandomPosRange);	//	모든 타겟들의 액션 이펙트를 적용한다.
	void	ApplyActionEffectsTarget(lwGUID kTargetGUID);	//	kTargetGUID 의 액션 이펙트만을 적용한다.
	void	ApplyOnlyDamage(int iDivide,bool bApplyEffects, float fRandomPosRange);
	void	Release();
	void	CopyFromActionGUIDCont(lwAction kAction);
	void	CopyToActionGUIDCont(lwAction kAction);
	void	DeleteInActionGUIDCont(lwAction kAction);
	void	AddToActionGUIDCont(lwAction kAction);
	void	SwapPosition(int iIndex,int iIndex2);	//	iIndex 와 iIndex2 에 들어있는 타겟들의 리스트상에서의 위치를 스왑한다.

LW_CLASS_END;
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_ACTION_LWACTIONTARGETLIST_H