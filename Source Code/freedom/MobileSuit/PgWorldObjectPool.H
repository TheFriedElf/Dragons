#ifndef FREEDOM_DRAGONICA_RENDER_WORLDOBJECT_PGWORLDOBJECTPOOL_H
#define FREEDOM_DRAGONICA_RENDER_WORLDOBJECT_PGWORLDOBJECTPOOL_H

class PgIWorldObject;

class PgWorldObjectPool
{
public:

	typedef	std::list<PgIWorldObject*> ObjectList;
	
	struct	stPoolData
	{
		std::string	m_kObjectID;
		PgIXmlObject::XmlObjectID	m_kObjectTypeID;
		std::string m_kObjectTypeStr;

		ObjectList	m_kObjectList;

		stPoolData()
		{
		}
		stPoolData(std::string const& kObjectID,
			PgIXmlObject::XmlObjectID kObjectTypeID,
			std::string const& kObjectTypeStr):
					m_kObjectID(kObjectID),
					m_kObjectTypeID(kObjectTypeID),
					m_kObjectTypeStr(kObjectTypeStr)
		{
		}
	};

	typedef std::list<stPoolData*> PoolDataList;
	typedef	std::map<std::string,PoolDataList> PoolDataCont;

private:

	PoolDataCont	m_kPoolDataCont;
	Loki::Mutex m_kPoolLock;

public:


	PgWorldObjectPool() {};
	virtual	~PgWorldObjectPool()	{	Terminate();	}

	void	Init();
	void	Terminate();

public:

	PgIWorldObject*	CreateObject(std::string const& kObjectID,
		PgIXmlObject::XmlObjectID kObjectTypeID,
		char const* pcObjectTypeStr);

private:

	stPoolData*	GetPoolData(std::string const& kObjectID,
		PgIXmlObject::XmlObjectID kObjectTypeID,
		std::string const& kObjectTypeStr) const;

	stPoolData*	AddPoolData(std::string const& kObjectID,
		PgIXmlObject::XmlObjectID kObjectTypeID,
		std::string const& kObjectTypeStr);


};

extern	PgWorldObjectPool	g_kWorldObjectPool;
#endif // FREEDOM_DRAGONICA_RENDER_WORLDOBJECT_PGWORLDOBJECTPOOL_H