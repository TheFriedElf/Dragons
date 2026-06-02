#ifndef FREEDOM_DRAGONICA_CONTENTS_WORLDEVENTSYSTEM_PGWORLDEVENTCLIENTMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_WORLDEVENTSYSTEM_PGWORLDEVENTCLIENTMGR_H
#include "Variant/PgWorldEvent.h"
class PgWorldEventState;
class PgWorldEventBaseMgr;
class	PgTrigger;
class	PgActor;

class PgWorldEventClientMgr : private PgWorldEventBaseMgr
{
	typedef std::map< WORD, PgWorldEventState > ContWorldEventState;
public:
	PgWorldEventClientMgr();
	~PgWorldEventClientMgr();

	void SyncFromServer(BM::Stream& rkPacket);
	void AddWorldEvent(PgWorldEventState const& rkState);

	bool ParseWorldEvent(TiXmlElement const* pkRoot);
	bool GetWorldEvent(WORD const& rkWorldEventID, PgWorldEvent const* &pkOut) const;
	void ClearParsedWorldEvent();

	void	OnPhysXTrigger(PgTrigger *pkTrigger,PgActor *pkActor,WorldEventCondition::EConditionType kConditionType);

protected:
	void ClearWorldEvent();

private:
	ContWorldEventState m_kEventState;
};

namespace WorldEventClientMgrUtil
{
	void Net_PT_C_M_CHECK_WORLDEVENT_CONDITION(PgActor* pkActor, WORD const wEventID);
};

#define g_kWorldEventClientMgr SINGLETON_CUSTOM(PgWorldEventClientMgr, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_CONTENTS_WORLDEVENTSYSTEM_PGWORLDEVENTCLIENTMGR_H