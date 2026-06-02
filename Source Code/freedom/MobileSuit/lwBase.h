#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWBASE_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWBASE_H

class lwBase
{
public:
	static bool RegisterWrapper(lua_State *pkState);	
};

extern void lwDoFile(char const* pcFileName);
extern char const *lwGetActorPath(int iClass);
extern	int	lwGetRandomItemNo(int	iClass,int	iGender,int	iPartsType);
extern void lwClearQuake();
extern bool lwCheckNil(bool bIsNil);
extern void lwSetBreak();
extern bool lwQuakeCamera(float fDuring, float fFactor, int iType, float fFrequency, int iCount);
extern float lwGetAccumTime();
extern bool lwIsSingleMode();
extern void lwSetSingleMode(bool const bSingleMode);
extern void lwUseCameraHeightAdjust(bool bUse);
extern float NewCalcValueToRate(int const iSuccessValueType, int iAbilValue, int iFinalSuccessRateType,int iAddFinalSuccessRate, int const iCasterLv, int const iTargetLv);
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWBASE_H