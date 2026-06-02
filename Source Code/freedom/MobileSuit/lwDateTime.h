#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWDATETIME_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWDATETIME_H

class lwDateTime
{
public:
	
	lwDateTime();
	lwDateTime(BM::DBTIMESTAMP_EX & kDateTime);
	static bool RegisterWrapper(lua_State *pkState);

private:

	BM::DBTIMESTAMP_EX m_kDateTime;
};

#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWDATETIME_H