#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_PORTALRENDERINGSYSTEM_PGPSROOME_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_PORTALRENDERINGSYSTEM_PGPSROOME_H
#include <NiNode.H>
#include <NiRoom.H>
#include "PgWorld.H"

class	PgPSRoom	:	public	NiRoom
{

    NiDeclareRTTI;

	friend class	PgPSRoomGroup;

public:

	typedef	std::vector<std::string> StringVector;

public:

	PgPSRoom();
	virtual	~PgPSRoom();

protected:

	void	UpdateSelective(NiCamera *pkCamera,float fTime,bool bUpdateControllers);


	void	AddPortalName(std::string const &kName)
	{
		m_kPortalNameCont.push_back(kName);
	}
	void	AddObjectName(std::string const &kName)
	{
		m_kObjectNameCont.push_back(kName);
	}

	bool	IsYourObject(std::string const &kName)
	{
		StringVector::iterator find_iter = std::find(m_kObjectNameCont.begin(), m_kObjectNameCont.end(), kName);
		return	(m_kObjectNameCont.end() != find_iter);
	}
	bool	IsYourPortal(std::string const &kName)
	{
		StringVector::iterator find_iter = std::find(m_kPortalNameCont.begin(), m_kPortalNameCont.end(), kName);
		return	(m_kPortalNameCont.end() != find_iter);
	}

	void	AssignObject(PgWorld::MapObjectMap &kObjectMap,bool bAssignEverything);

	NiNode*	GetStaticNodeRoot()	const	{	return	m_spStaticRoot;	}
	NiNode*	GetSelectiveNodeRoot()	const	{	return	m_spSelectiveRoot;	}

private:
	void	UpdateSelected(NiCamera *pkCamera,float fTime,NiNode *pkSelectiveNodeRoot);

private:

	NiNodePtr	m_spSelectiveRoot,m_spStaticRoot;

	PgWorld::MapObjectMap	m_kObjectCont;

	StringVector	m_kPortalNameCont;
	StringVector	m_kObjectNameCont;

	NiExtraDataPtr	m_spExtraData;

};

typedef NiPointer<PgPSRoom> PgPSRoomPtr;

#endif FREEDOM_DRAGONICA_RENDER_RENDERER_PORTALRENDERINGSYSTEM_PGPSROOME_H