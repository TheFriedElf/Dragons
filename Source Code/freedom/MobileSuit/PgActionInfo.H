#pragma once

typedef std::vector<int> IntVector;

struct	stActionInfo
{
	int	m_iActionID;
	std::string m_kXML_ID;
};
class	PgActionInfo	:	public	PgIXmlObject
{

	typedef std::map<int,stActionInfo*> ActionInfoCont;

	ActionInfoCont		m_ContActionInfo;

public:

	PgActionInfo();
	virtual	~PgActionInfo();

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0);
};

extern	PgActionInfo	*g_pkActionInfo;