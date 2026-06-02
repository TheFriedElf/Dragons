#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_BREAKABLEOBEJCT_PGBREAKABLEOBJECT_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_BREAKABLEOBEJCT_PGBREAKABLEOBJECT_H
#include "PgActor.H"
class	PgObject;

class	PgBreakableObjectGroup
{
public:

	typedef	std::vector<PgObject*> VBreakableObject;

private:

	BM::GUID	m_kGUID;
	VBreakableObject	m_vObjectCont;

public:

	PgBreakableObjectGroup()	{	Init();	}
	~PgBreakableObjectGroup()	{	Terminate();	}

	void	AddObject(PgObject *pkObject);
	void	SetGUID(BM::GUID const &kGUID)	{	m_kGUID = kGUID;	}
	BM::GUID &GetGUID()	{	return	m_kGUID;	}

	void	ClearAllObject();
	void	BreakObject(PgObject *pkBrokenObject);

private:

	void	RearrangeObjectPosition();

	void	Init();
	void	Terminate();
};

class	PgBreakableObjectGroupMan
{
public:

	typedef	std::map<BM::GUID,PgBreakableObjectGroup*> VBreakableObjectGroup;

private:

	VBreakableObjectGroup	m_vObjectGroupCont;

public:

	PgBreakableObjectGroupMan()	{	Init();	}
	virtual	~PgBreakableObjectGroupMan()	{	Terminate();	}


	void	Init();
	void	Terminate();

	PgBreakableObjectGroup*	AddNewGroup(BM::GUID const &kGroupGUID);
	PgBreakableObjectGroup*	GetGroup(BM::GUID const &kGroupGUID);
	void	DestroyGroup(BM::GUID const &kGroupGUID);
	void	ClearAllGroup();

};

extern	PgBreakableObjectGroupMan	g_kBreakableObjectGroupMan;
#endif// FREEDOM_DRAGONICA_RENDER_WORDOBJECT_BREAKABLEOBEJCT_PGBREAKABLEOBJECT_H