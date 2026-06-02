#ifndef FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTDATA_H
#define FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTDATA_H

#include "PgEventScriptCmd.H"

typedef	std::vector<PgEventScriptCmd*> EventScriptCmdCont;
class	PgEventScriptData
{
	EventScriptCmdCont	m_kEventScriptCmdCont;

	int	m_iEventID;
	int	m_iInstanceCount;	//	이 데이터 객체를 사용하는 인스턴스 갯수

	static	unsigned	long	m_ulDataFileVersion;	//	데이터 파일 버전

public:

	PgEventScriptData();

	virtual	~PgEventScriptData();

	bool	LoadFromFile(int const iEventID);
	bool	Reload();

	int	GetEventID()	const	{	return	m_iEventID;	}

	void	IncInstanceCount()	{	m_iInstanceCount++;	}
	void	DecInstanceCount()	{	if(m_iInstanceCount>0) m_iInstanceCount--;	}
	int	GetInstanceCount()	const	{	return	m_iInstanceCount;	}

	int	GetCmdCount()	const	{	return	m_kEventScriptCmdCont.size();	}
	PgEventScriptCmd*	GetCmd(int iIndex)	const	{	return	m_kEventScriptCmdCont[iIndex];	}

private:

	void	SetEventID(int iEventID)	{	m_iEventID = iEventID;	}

	bool	LoadFromXml(char const *pkDataFilePath);

	bool	LoadFromBinary(char const *pkDataFilePath);
	void	SaveToBinary(char const *pkDataFilePath)	const;

	void	ReleaseAll();

	bool	CheckLoadFromXML()	const;	//	XML 로부터 로딩해야한다면 true, Binary 로부터 로딩해야 한다면 false

	PgEventScriptCmd*	AddNewCmd(PgEventScriptCmd *pkCmd);

};

#endif // FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTDATA_H