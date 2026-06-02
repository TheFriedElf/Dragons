#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_PGUIDATAMGR_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_PGUIDATAMGR_H

//#include "BM/TWRapper.h"

typedef enum
{
	UIDATA_EMPORIA_ADMIN,
	UIDATA_EXPCARD,
	UIDATA_COSTUME_MIX,
	UIDATA_USE_UNLOCK_ITEM,
}E_UIDATA_TYPE;

class PgUIData_Base
{
public:
	PgUIData_Base(void){}
	virtual ~PgUIData_Base(void){}

	virtual int GetType(void)const = 0;
};

class PgUIData_EmporiaAdmin
	:	public PgUIData_Base
{
public:
	PgUIData_EmporiaAdmin(void):m_bIsOwner(false),m_byGateState(EMPORIA_GATE_CLOSE){}
	virtual ~PgUIData_EmporiaAdmin(void){}

	virtual int GetType(void)const{return UIDATA_EMPORIA_ADMIN;}

	void ReadFromPacket( BM::Stream &kPacket );

public:
	bool				m_bIsOwner;
	SEmporiaKey			m_kKey;
	BYTE				m_byGateState;
	PgEmporiaFunction	m_kFunc;
};

class PgUIDataMgr
{
	typedef std::map< int, PgUIData_Base* >		CONT_DATA;
public:
	PgUIDataMgr(void);
	~PgUIDataMgr(void);

	void Clear();

	template< typename T >
	bool Add( T *& p, bool const bAutoDelete=true )
	{
		PgUIData_Base *pBase =  dynamic_cast<PgUIData_Base*>(p);
		if ( pBase )
		{
			std::pair<CONT_DATA::iterator, bool> kPair = m_kCont.insert( std::make_pair( p->GetType(), pBase ) );
			if ( !kPair.second )
			{
				SAFE_DELETE( kPair.first->second );
				kPair.first->second = pBase;
			}
			return true;
		}

		if ( true == bAutoDelete )
		{
			SAFE_DELETE(p);
		}

		return false;
	}

	bool Remove( int const iID );

	template< typename T >
	bool Get( int const iID, T *& pOut )
	{
		CONT_DATA::iterator itr = m_kCont.find( iID );
		if ( itr != m_kCont.end() )
		{
			pOut = dynamic_cast<T*>(itr->second);
			return ( NULL != pOut );
		}
		return false;
	}

private:
	CONT_DATA		m_kCont;
};

#define g_kUIDataMgr SINGLETON_STATIC(PgUIDataMgr)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_PGUIDATAMGR_H