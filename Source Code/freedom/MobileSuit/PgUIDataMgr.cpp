#include "stdafx.h"
#include "PgUIDataMgr.h"

void PgUIData_EmporiaAdmin::ReadFromPacket( BM::Stream &kPacket )
{
	kPacket.Pop( m_kKey );
	kPacket.Pop( m_byGateState );
	kPacket.Pop( m_bIsOwner );
	if ( true == m_bIsOwner )
	{
		m_kFunc.ReadFromPacket( kPacket );
	}
}

PgUIDataMgr::PgUIDataMgr(void)
{

}

PgUIDataMgr::~PgUIDataMgr(void)
{
	Clear();
}

void PgUIDataMgr::Clear(void)
{
	CONT_DATA::iterator itr = m_kCont.begin();
	for ( ; itr!=m_kCont.end() ; ++itr )
	{
		SAFE_DELETE( itr->second );
	}
	m_kCont.clear();
}

bool PgUIDataMgr::Remove( int const iID )
{
	CONT_DATA::iterator itr = m_kCont.find( iID );
	if ( itr != m_kCont.end() )
	{
		SAFE_DELETE( itr->second );
		m_kCont.erase( itr );
		return  true;
	}
	return false;
}
