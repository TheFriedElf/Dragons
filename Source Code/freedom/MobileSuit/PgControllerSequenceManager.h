#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGCONTROLLERSEQUENCEMANAGER_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGCONTROLLERSEQUENCEMANAGER_H

#include "NiActorManager.H"

class	PgControllerSequenceManager	
{
public:

	struct	stFileBuf
	{
		int	m_iLength;
		BYTE	*m_pkBuf;

		stFileBuf():m_pkBuf(NULL),m_iLength(0)
		{
		}
		~stFileBuf()
		{
			if(m_pkBuf)
				delete []m_pkBuf;
			m_pkBuf= NULL;
		}
	};

	typedef	std::map<std::string,stFileBuf*> IntSeqMap;

private:

	IntSeqMap	m_kConSeqMap;


public:

	PgControllerSequenceManager();
	virtual	~PgControllerSequenceManager();


	void	Init();
	void	Terminate();

public:

	NiControllerSequencePtr	CreateNewControllerSequence(NiKFMTool *pkKFMTool,NiActorManager::SequenceID kSequenceID);
	NiControllerSequencePtr	CreateNewControllerSequence(std::string const &kKFFilename,int iAnimIndex);

private:

	stFileBuf*	Find(std::string const &kKFFilename) const;

};

extern	PgControllerSequenceManager	g_kControllerSequenceManager;

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGCONTROLLERSEQUENCEMANAGER_H