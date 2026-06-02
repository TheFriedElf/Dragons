#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORMANAGER_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORMANAGER_H
#include "NiActorManager.H"


class	PgActorManager	:	public	NiActorManager
{
	
	static	bool	m_bUseNewClone;

	bool	m_bSlightClonned;
	bool	m_bNoUnloadSequnce;

	NiNodePtr	m_spSourceNIFRoot;

public:

	virtual	~PgActorManager();

    virtual	NiActorManager* Clone(NiCloningProcess* pkCloningProcess = NULL);

    virtual	bool SetTargetAnimation(SequenceID eSequenceID);

    virtual	NiControllerSequence* GetSequence(SequenceID eSequenceID);
    virtual	bool ChangeNIFRoot(NiAVObject* pkNIFRoot);

public:

	void	SetSourceNIFRoot(NiNode *pkSourceNIFRoot)	{	m_spSourceNIFRoot = pkSourceNIFRoot;	}
	NiNode*	GetSourceNIFRoot()	{	return	m_spSourceNIFRoot;	}

private:

	void	SetSlightClonned(bool bSlightClonned)	{	m_bSlightClonned = bSlightClonned;	}
	bool	GetSlightClonned()	{	return	m_bSlightClonned;	}

public:

    static NiActorManager* CreatePG(char const* pcKFMFilename,
        bool bCumulativeAnimations = false, bool bLoadFilesFromDisk = true,
        NiStream* pkStream = NULL);

    static NiActorManager* CreatePG(NiKFMTool* pkKFMTool,
        char const* pcKFMFilePath, bool bCumulativeAnimations = false,
        bool bLoadFilesFromDisk = true, NiStream* pkStream = NULL);

	static	void	SetUseNewClone(bool bUse)	{	m_bUseNewClone = bUse;	}
	static	bool	GetUseNewClone()	{	return	m_bUseNewClone;	}

protected:

    PgActorManager(NiKFMTool* pkKFMTool, bool bCumulativeAnimations);

    bool Initialize(NiStream& kStream);

private:

    NiActorManager* CloneEx(NiCloningProcess* pkCloningProcess);

    bool ChangeSequence(SequenceID eSequenceID,NiControllerSequence* pkSequence,bool bUnloadSequnce);

	void	UnloadAllSequence();

};

NiSmartPointer(PgActorManager);

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTOR_PGACTORMANAGER_H