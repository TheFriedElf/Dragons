#ifndef FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTCMD_H
#define FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTCMD_H

class	PgEventScriptCmd
{
public:
	
	typedef	std::map<std::string,std::string>	ParamMap;

private:

	std::string	m_kType;
	ParamMap	m_kAttributes;

	ParamMap	*m_pkCustomParamMap;	//	Only Reference, Do not delete this object from memory.

	int	m_iEventID;	//	it's the event id which have this command
	int	m_iCommandNumber;	//	the number of this command in order of sequence

public:

	static	PgEventScriptCmd*	Create(TiXmlAttribute const *pkAttribute,int iEventID,int iCommandNumber);
	static	PgEventScriptCmd*	Create(FILE *fp,int iEventID,int iCommandNumber);

	PgEventScriptCmd(int iEventID,int iCommandNumber);
	virtual	~PgEventScriptCmd();

	virtual	bool	Update(float fAccumTime,float fFrameTime,ParamMap &kCustomParamMap);

public:

	std::string	const&	GetType()	{	return	m_kType;	}
	void	SetType(std::string const &kType)	{	m_kType = kType;	}

	int	GetEventID()	const	{	return	m_iEventID;	}
	int	GetCommandNumber()	const	{	return	m_iCommandNumber;	}

public:

	virtual	bool	LoadFromXMLAttribute(TiXmlAttribute const *pkAttribute);

	virtual	bool	LoadFromBinary(FILE *fp);
	virtual	void	SaveToBinary(FILE *fp);

public:

	std::string const&	GetAttr(std::string const &kAttrName);
	int	GetAttrInt(std::string const &kAttrName);
	float	GetAttrFloat(std::string const &kAttrName);

	std::string const&	GetParam(std::string const &kParamName);
	int	GetParamInt(std::string const &kParamName);
	float	GetParamFloat(std::string const &kParamName);

	void	SetParam(std::string const &kParamName,std::string const &kParamValue);
	void	SetParamInt(std::string const &kParamName,int iParamValue);
	void	SetParamFloat(std::string const &kParamName,float fParamValue);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTCMD_H