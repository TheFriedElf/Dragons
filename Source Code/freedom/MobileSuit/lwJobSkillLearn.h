#ifndef FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWJOBSKILLLEARN_H
#define FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWJOBSKILLLEARN_H

#include "Variant/PgJobSkill.h"

namespace lwJobSkillLearn
{
	void RegisterWrapper(lua_State *pkState);
	void lwCallJobSkillLearnUI();
	void ReturnResult(EJOBSKILL_LEARN_RET const& eRet);
}

namespace lwJobSkill_NfySaveIdx
{
	void RegisterWrapper(lua_State *pkState);
	bool CheckNewSaveIdx(int const iJobSkill, int const iBeforeExp, int const iCurrentExp);
	void lwUpdateUI();
	void ShowNfySaveIdx(int const iSaveIdx);
	void ClearContSaveIdx();
}

#endif // FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_LWJOBSKILLLEARN_H