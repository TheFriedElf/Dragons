#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PGSCRIPTING_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PGSCRIPTING_H

#include "PgStat.h"

#define LW_CLASS(C, L)\
	class C;\
	class lw##L\
	{\
	protected:\
		C *m_pk##L;\
	public:\
		lw##L(void *obj) { m_pk##L = (C *)obj;}\
		static void RegisterWrapper(lua_State *pkState);\
		C *operator()() { return m_pk##L; }\
		bool IsNil() { return ((m_pk##L == NULL) ? true : false); }\
		void* GetObject() { return m_pk##L; }

#define LW_CLASS_CONSTRUCTOR(L, P)\
	lw##L(##P);

#define LW_CLASS_END\
	};\


#define LW_REG_WRAPPER(L)\
	lw##L::RegisterWrapper(pkState)


#define LW_REG_CLASS(N)\
	class_<lw##N>(pkState, #N)\
		.def(pkState, constructor<void *>())\
		.def(pkState, "IsNil", &lw##N::IsNil)\
		.def(pkState, "GetObject", &lw##N::GetObject)


#define LW_REG_METHOD(N, M)\
	.def(pkState, #M, &lw##N::M)

#define LW_REG_CONSTRUCTOR(N, P)\
	.def(pkState, constructor<##P>())

class PgScripting
{
	struct stDoFileInfo
	{
		std::string strFileName;
		float fAddTime;
	};

	typedef std::map<std::string, StatInfoF> ScriptStatContainer;
	typedef std::list<stDoFileInfo> ScriptDoFileQueue;
public:
	static bool Initialize();
	static bool Terminate();	
	static bool RegisterModules();
	static void Update();
	static void DoFile(char const *pcFileName);
	static void DoString(char const *pcCmd);
	static bool DoBuffer(char const *pcCmd, size_t const size);
	static void Reset();

protected:
	PgScripting();
	~PgScripting();

	static void doFile(char const *pcFileName);

protected:	
	static ScriptStatContainer m_kScriptStatContainer;
	static FrameStat m_kDoBufferStat;
	static PgStatGroup m_kDoBufferStatGroup;
	static StatInfoF m_kDoStringStat;
	static ScriptDoFileQueue m_kScriptDoFileQueue;
	static Loki::Mutex m_kDoFileQueueLock;
	static DWORD m_hMainThread;
	static bool m_bDoFileOptimize;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PGSCRIPTING_H