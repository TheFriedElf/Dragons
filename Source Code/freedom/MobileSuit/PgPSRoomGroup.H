#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_PORTALRENDERINGSYSTEM_PGPSROOMGROUP_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_PORTALRENDERINGSYSTEM_PGPSROOMGROUP_H
#include <NiNode.H>
#include <NiRoomGroup.H>
#include <NiRoom.H>
#include <NiPortal.H>
#include "PgPSRoom.H"
#include "PgWorld.H"
#include "PgRenderer.H"

#include "NewWare/Scene/ApplyTraversal.h"


class	PgPSRoomGroup	:	public	NiRoomGroup
{
public:

	typedef	std::map<NiPortalPtr,std::string> PortalMap;

public:

	PgPSRoomGroup()
	{
		Init();
	}

	void	Init();

	bool	AddEntity(NiEntityInterface *pkEntity);

	void	InitializeRooms(PgWorld::MapObjectMap &kObjectMap);

	virtual void OnVisible(NiCullingProcess& kCuller);

	void	UpdateSelective(NiCamera *pkCamera,float fAccumTime,bool bUpdateControllers = true);

public:

	void	SetShaderConstantUpdateOptimizeFlag(bool bUseOptimize)
	{
		NiTListIterator pkIter = m_kRooms.GetHeadPos();
		while (pkIter)
		{
			PgPSRoom* pkRoom = NiDynamicCast(PgPSRoom,m_kRooms.GetNext(pkIter));
			if(pkRoom)
			{
                NewWare::Scene::ApplyTraversal::Geometry::
                    SetShaderConstantUpdateOptimizeFlag( pkRoom->GetStaticNodeRoot(), bUseOptimize, true );
			}
		}
	}

private:

	void	AddRoomEntity(NiEntityInterface *pkEntity);
	void	AddPortalEntity(NiEntityInterface *pkEntity);

	void	AssginPortal();
	void	AssignObject(PgWorld::MapObjectMap &kObjectMap);

	NiRoom*	FindRoom(std::string const &kRoomName)
	{
		NiTListIterator pkIter = m_kRooms.GetHeadPos();
		while (pkIter)
		{
			NiRoom* pkRoom = m_kRooms.GetNext(pkIter);
			if(pkRoom && pkRoom->GetName() == kRoomName.c_str())
			{
				return	pkRoom;
			}
		}
		return 0;

	}

private:

	PgPSRoomPtr	m_spOutdoor;

	PortalMap	m_kPortalCont;

};

#endif // FREEDOM_DRAGONICA_RENDER_RENDERER_PORTALRENDERINGSYSTEM_PGPSROOMGROUP_H