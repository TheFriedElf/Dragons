#ifndef FREEDOM_DRAGONICA_CONTENTS_WORLDACTION_PGWORLDACTIONMAN_H
#define FREEDOM_DRAGONICA_CONTENTS_WORLDACTION_PGWORLDACTIONMAN_H

#include "NiMain.h"
#include "PgIXmlObject.h"

class	PgWorldActionObject;

struct	stWorldActionXML
{
	int	m_iWorldActionID;
	std::string	m_ScriptName;
	DWORD m_dwDuration;
};
class PgWorldActionMan	:	public	PgIXmlObject
{
	typedef std::list<PgWorldActionObject*> WorldActionObjectList;

	WorldActionObjectList	m_ObjList;

	int	m_iTotalWorldActionXML;
	stWorldActionXML	*m_paWorldActionXML;

public:

	PgWorldActionMan()	{	Init();	}
	virtual	~PgWorldActionMan()	{	Destroy();	}

	virtual	bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	PgWorldActionObject *CreateNewWorldActionObject(int iWorldActionID);	

	void Update(float fFrameTime,float fAccumTime);

	std::string	GetWorldActionScript(int iWorldActionID);
	DWORD GetWorldActionDuration(int iWorldActionID);

private:

	void	Init();
	void	Destroy();

	int	m_iCounter;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_WORLDACTION_PGWORLDACTIONMAN_H