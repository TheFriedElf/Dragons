#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONPOOL_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONPOOL_H
#include "BM/ObjectPool.h"
#include "CreateUsingNiNew.inl"

class PgAction;
class	PgMobileSuit;
class PgActionPool
{
	friend struct Loki::CreateUsingNew< PgActionPool >;
	friend	class	PgMobileSuit;
	friend void lwInitActionXML();

	typedef std::map<std::string, PgAction* > Container;

protected:
	PgActionPool();
	~PgActionPool();

	void Destroy();

public:
	PgAction* CreateAction(char const *pcActionID, bool bWarning = true);
	void ReleaseAction(PgAction*& pkAction);
	
	void LoadActionBeforeUse();
protected:
	Container m_kContainer;

	BM::TObjectPool< PgAction, CreateArrayUsingNiNew > m_kElementPool;

	mutable Loki::Mutex m_kMutex;
};

#define g_kActionPool SINGLETON_CUSTOM(PgActionPool, Loki::CreateUsingNew)
#endif //FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONPOOL_H