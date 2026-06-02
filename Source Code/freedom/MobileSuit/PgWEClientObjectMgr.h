#ifndef FREEDOM_DRAGONICA_CONTENTS_WORLDEVENTSYSTEM_PGWECLIENTOBJECTMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_WORLDEVENTSYSTEM_PGWECLIENTOBJECTMGR_H

#include "Variant/PgWorldEventObject.h"
#include "PgWEObjectState.H"

typedef PgWEObjectState PgWECOClient;

//
class PgWEClientObjectClientMgr : private PgWEClientObjectBaseMgr< PgWECOClient* >
{
	typedef PgWEClientObjectBaseMgr< PgWECOClient* > _MyBaseType;
public:
	PgWEClientObjectClientMgr();
	virtual ~PgWEClientObjectClientMgr();

	void ClearWEClientObject();

	void SyncFromServer(BM::Stream& rkPacket);
	void UpdateClientObject(PgWEClientObjectState const& rkState);
	bool ParseWEClientObjectList(TiXmlElement const* pkListElementNode);

protected:
	virtual bool ParseWEClientObject(TiXmlElement const* pkWEObjectNode);
};
#define g_kWEClientObjectMgr SINGLETON_CUSTOM(PgWEClientObjectClientMgr, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_CONTENTS_WORLDEVENTSYSTEM_PGWECLIENTOBJECTMGR_H