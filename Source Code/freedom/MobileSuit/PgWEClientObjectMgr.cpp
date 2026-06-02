#include "stdafx.h"
#include "PgWEClientObjectMgr.h"

PgWEClientObjectClientMgr::PgWEClientObjectClientMgr()
{
}

PgWEClientObjectClientMgr::~PgWEClientObjectClientMgr()
{
	ClearWEClientObject();
}

void PgWEClientObjectClientMgr::ClearWEClientObject()
{
	ContWEObjectState::iterator iter = m_kContState.begin();
	while( m_kContState.end() != iter )
	{
		SAFE_DELETE( (*iter).second );
		++iter;
	}
	m_kContState.clear();
}

void PgWEClientObjectClientMgr::SyncFromServer(BM::Stream& rkPacket)
{
	size_t iCount = 0;
	rkPacket.Pop( iCount );

	PgWEClientObjectState kTempState;

	while( 0 < iCount )
	{
		kTempState.ReadFromPacket( rkPacket );
		UpdateClientObject( kTempState );
		--iCount;
	}
}

void PgWEClientObjectClientMgr::UpdateClientObject(PgWEClientObjectState const& rkState)
{
	ContWEObjectState::iterator iter = m_kContState.find( rkState.ID() );
	if( m_kContState.end() != iter )
	{
		PgWECOClient* pkWEClientObject = ((*iter).second);
		if( pkWEClientObject )
		{
			pkWEClientObject->Update(rkState);
		}
	}
	else
	{
		NILOG(PGLOG_WARNING, "[PgWEClientObjectClientMgr] unknown WEClientObject ID[%u]\n", rkState.ID());
#ifndef USE_INB
		_PgMessageBox(GetClientName(), "Can't Find WEClientOBject [%d]", rkState.ID());
#endif
	}
}

bool PgWEClientObjectClientMgr::ParseWEClientObjectList(TiXmlElement const* pkListElementNode)
{
	ClearWEClientObject();
	return _MyBaseType::ParseWEClientObjectList(pkListElementNode);
}


bool PgWEClientObjectClientMgr::ParseWEClientObject(TiXmlElement const* pkWEObjectNode)
{
	if( !pkWEObjectNode )
	{
		return false;
	}

	PgWECOClient* pkNewState = new PgWECOClient();
	if( pkNewState->Parse(pkWEObjectNode) )
	{
		std::pair<ContWEObjectState::iterator, bool> kRet = m_kContState.insert( std::make_pair(pkNewState->ID(), pkNewState) );
		if( kRet.second )
		{
			return true;
		}
		else
		{
			CAUTION_LOG(BM::LOG_LV1, __FL__ << L"Duplicate WorldEventClientObject ID[" << pkNewState->ID() << L"]");
		}
	}
	SAFE_DELETE(pkNewState);
	return false;
}