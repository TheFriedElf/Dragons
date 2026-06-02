#pragma once

typedef std::vector<int> IntVector;

struct	stClassInfo
{
	std::string m_kClassName;
	int	m_iClassID;

	IntVector	m_vCanLearnSkill;
};
class	PgClassInfo	:	public	PgIXmlObject
{

	typedef std::vector<stClassInfo*> ClassInfoVec;

	int	m_iClassInfoCount;
	ClassInfoVec		m_vClassInfo;

public:

	PgClassInfo();
	virtual	~PgClassInfo();

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0);
};

extern	PgClassInfo	*g_pkClassInfo;