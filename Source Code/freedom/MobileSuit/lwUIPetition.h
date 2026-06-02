#ifndef	FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIPETITION_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIPETITION_H
#include "PgPetitionMgr.h"

class lwUIPetition
{
private:

protected:
	PgPetitionMgr* m_pkPetitionMgr; 

public:
	lwUIPetition(PgPetitionMgr* pkPetitionMgr);
	~lwUIPetition();

	static bool RegisterWrapper(lua_State *pkState);

	bool Send_Petition_To_Gm();
	bool Remainder_Petition();
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIPETITION_H