#include "stdafx.h"
#include "PgPSRoomGroup.H"
#include "NiRoomComponent.H"
#include "NiPlaneComponent.H"
#include "NiPortalComponent.H"
#include "NiBoxComponent.H"
#include "PgPSRoom.H"

void	PgPSRoomGroup::Init()
{
	SetName("ROOM_GROUP_ROOT");

	m_spOutdoor = NiNew PgPSRoom();
	m_spOutdoor->SetName("__OutDoor_Reserved__");
	AttachRoom(m_spOutdoor);
}
void	PgPSRoomGroup::UpdateSelective(NiCamera *pkCamera,float fAccumTime,bool bUpdateControllers)
{
    NIMETRICS_MAIN_INCREMENTUPDATES();

    if (bUpdateControllers)
        UpdateObjectControllers(fAccumTime);

    UpdateWorldData();
	UpdateColorData();

    m_kWorldBound.SetRadius(0.0f);

    // To avoid having to call UpdateWorldBound and therefore making another
    // iteration through the node's children, the world bound is calculated
    // during this loop.
    for (unsigned int i = 0; i < m_kChildren.GetSize(); i++)
    {
		NiAVObject* pkChild = m_kChildren.GetAt(i);
        if (pkChild)
        {
			PgPSRoom	*pkRoom = NiDynamicCast(PgPSRoom,pkChild);  
			if(pkRoom)
			{
				pkRoom->UpdateSelective(pkCamera,fAccumTime,bUpdateControllers);
			}
			else
			{
	            pkChild->UpdateDownwardPass(fAccumTime, bUpdateControllers);
			}

            if (pkChild->IsVisualObject())
            {
                if (m_kWorldBound.GetRadius() == 0.0f)
                {
                    m_kWorldBound = pkChild->GetWorldBound();
                }
                else
                {
                    m_kWorldBound.Merge(&pkChild->GetWorldBound());
                }
            }
        }
    }

    if (m_pkParent)
        m_pkParent->UpdateUpwardPass();
}

void PgPSRoomGroup::OnVisible(NiCullingProcess& kCuller)
{
    NIMETRICS_PORTAL_SCOPETIMER(ROOMGROUP_ONVISIBLE);

	if(m_spLastRoom == m_spOutdoor)
	{
		m_spLastRoom = 0;
	}

    // Locate the room which contains the camera (or NULL if outside).
    const NiCamera* pkCamera = kCuller.GetCamera();
    NiRoom* pkRoom = WhichRoom(pkCamera->GetWorldLocation(), m_spLastRoom);
    if (pkRoom)
    {
        // The camera is in pkRoom.  Remember this room for a fast lookup
        // in the next call to WhichRoom.
        m_spLastRoom = pkRoom;

        // Compute the potentially visible set of the room.  Use the fixture
        // culler to obtain a unique list of objects, and then append that
        // list to the input culler.
        m_kFixtureCuller.Process(pkCamera, pkRoom, kCuller.GetVisibleSet());
    }
    else if (m_spShell)
    {
        // The camera is outside the room group, process the shell.
        m_spLastRoom = pkRoom;
        m_spShell->Cull(kCuller);
    }
    else
    {
        // The camera is outside the room group, but there is no shell to
        // process.  Do no change the m_spLastRoom in this case.  When there
        // is no shell, we just continue processing the last room.  This
        // behavior limits the issues with rooms that do not quite overlap.
        if (m_spLastRoom)
            m_spLastRoom->Cull(kCuller);
    }
}
bool	PgPSRoomGroup::AddEntity(NiEntityInterface *pkEntity)
{

	if(pkEntity->GetPGProperty() == "PS_ROOM")
	{
		AddRoomEntity(pkEntity);
		return	true;
	}
	if(pkEntity->GetPGProperty() == "PS_PORTAL")
	{
		AddPortalEntity(pkEntity);
		return	true;
	}

	return	false;
}
void	PgPSRoomGroup::AddRoomEntity(NiEntityInterface *pkEntity)
{
	int	const	iComponentCount = pkEntity->GetComponentCount();

	NiBoxComponent	*pkBox  = 0;
	NiRoomComponent	*pkRoom = 0;
	NiTransformationComponent	*pkTransform = 0;

	for(int i =0;i<iComponentCount; ++i)
	{
		NiEntityComponentInterface *pkComponent = pkEntity->GetComponentAt(i);
		if(!pkComponent)
		{
			continue;
		}

		if(pkComponent->GetName() == "Box")
		{
			pkBox = (NiBoxComponent*)pkComponent;
			continue;
		}
		if(pkComponent->GetName() == "Transformation")
		{
			pkTransform = (NiTransformationComponent*)pkComponent;
			continue;
		}
		if(pkComponent->GetName() == "PS_Room")
		{
			pkRoom = (NiRoomComponent*)pkComponent;
			continue;
		}
	}

	if(!pkBox || !pkRoom || !pkTransform)
	{
		return;
	}

	PgPSRoomPtr	spNewRoom = NiNew PgPSRoom();
	spNewRoom->SetName(pkEntity->GetName());

	//	8점 만들기
	NiPoint3	kPoint[8]={
		NiPoint3(-pkBox->GetSize().x/2,-pkBox->GetSize().y/2,pkBox->GetSize().z/2),
		NiPoint3(pkBox->GetSize().x/2,-pkBox->GetSize().y/2,pkBox->GetSize().z/2),
		NiPoint3(pkBox->GetSize().x/2,-pkBox->GetSize().y/2,-pkBox->GetSize().z/2),
		NiPoint3(-pkBox->GetSize().x/2,-pkBox->GetSize().y/2,-pkBox->GetSize().z/2),

		NiPoint3(-pkBox->GetSize().x/2,pkBox->GetSize().y/2,pkBox->GetSize().z/2),
		NiPoint3(pkBox->GetSize().x/2,pkBox->GetSize().y/2,pkBox->GetSize().z/2),
		NiPoint3(pkBox->GetSize().x/2,pkBox->GetSize().y/2,-pkBox->GetSize().z/2),
		NiPoint3(-pkBox->GetSize().x/2,pkBox->GetSize().y/2,-pkBox->GetSize().z/2),
	};

	//	Transform
	NiTransform	kTransform;
	kTransform.m_Rotate = pkTransform->GetRotation();
	kTransform.m_fScale = pkTransform->GetScale();
	kTransform.m_Translate = pkTransform->GetTranslation();

	for(int i=0;i<8;++i)
	{
		kPoint[i] = kTransform*kPoint[i];
	}

	//	6 평면 만들기
	NiPlane	kPlane[6];

    NiPoint3 v1 = kPoint[0] - kPoint[1];
    NiPoint3 v2 = kPoint[1] - kPoint[2];
    NiPoint3 kNormal = v1.Cross(v2);
	kNormal.Unitize();

    kPlane[0] = NiPlane(kNormal, kPoint[0]);

    v1 = kPoint[5] - kPoint[4];
    v2 = kPoint[4] - kPoint[7];
    kNormal = v1.Cross(v2);
	kNormal.Unitize();

    kPlane[1] = NiPlane(kNormal, kPoint[7]);

    v1 = kPoint[0] - kPoint[4];
    v2 = kPoint[4] - kPoint[5];
    kNormal = v1.Cross(v2);
	kNormal.Unitize();

    kPlane[2] = NiPlane(kNormal, kPoint[0]);

    v1 = kPoint[2] - kPoint[6];
    v2 = kPoint[6] - kPoint[7];
    kNormal = v1.Cross(v2);
	kNormal.Unitize();

    kPlane[3] = NiPlane(kNormal, kPoint[2]);

    v1 = kPoint[1] - kPoint[5];
    v2 = kPoint[5] - kPoint[6];
    kNormal = v1.Cross(v2);
	kNormal.Unitize();

    kPlane[4] = NiPlane(kNormal, kPoint[1]);

    v1 = kPoint[4] - kPoint[0];
    v2 = kPoint[0] - kPoint[3];
    kNormal = v1.Cross(v2);
	kNormal.Unitize();

    kPlane[5] = NiPlane(kNormal, kPoint[0]);
	NiRoom::WallArray	&kWall = spNewRoom->GetWallArray();
	
	for(int i=0;i<6;++i)
	{
		NiRoom::Wall *pkWall = NiNew NiRoom::Wall();
		pkWall->m_kModelPlane = kPlane[i];
		kWall.Add(pkWall);
	}


	//	Fixture Object Name 등록
	NiTPrimitiveArray<NiEntityInterface*>	const &kObjects = pkRoom->GetObjects();
	int	iTotal = kObjects.GetSize();
	for(int i=0;i<iTotal;++i)
	{
		spNewRoom->AddObjectName((char const *)kObjects.GetAt(i)->GetName());
	}
	
	//	Portal Name 등록
	NiTPrimitiveArray<NiEntityInterface*>	const &kPortals = pkRoom->GetPortals();
	iTotal = kPortals.GetSize();
	for(int i=0;i<iTotal;++i)
	{
		spNewRoom->AddPortalName((char const *)kPortals.GetAt(i)->GetName());
	}

	AttachRoom(spNewRoom);
}
void	PgPSRoomGroup::AddPortalEntity(NiEntityInterface *pkEntity)
{
	int	const	iComponentCount = pkEntity->GetComponentCount();

	NiPlaneComponent	*pkPlane = 0;
	NiTransformationComponent *pkTransform = 0;
	NiPortalComponent	*pkPortal = 0;

	for(int i =0;i<iComponentCount; ++i)
	{
		NiEntityComponentInterface *pkComponent = pkEntity->GetComponentAt(i);
		if(!pkComponent)
		{
			continue;
		}

		if(pkComponent->GetName() == "Plane")
		{
			pkPlane = (NiPlaneComponent*)pkComponent;
			continue;
		}
		if(pkComponent->GetName() == "Transformation")
		{
			pkTransform = (NiTransformationComponent*)pkComponent;
			continue;
		}
		if(pkComponent->GetName() == "PS_Portal")
		{
			pkPortal = (NiPortalComponent*)pkComponent;
			continue;
		}
	}

	if(!pkPlane || !pkTransform || !pkPortal)
	{
		return;
	}

	NiPoint3	kVertex[4]={
		NiPoint3(-pkPlane->GetSize().x/2,0,pkPlane->GetSize().y/2),
		NiPoint3(pkPlane->GetSize().x/2,0,pkPlane->GetSize().y/2),
		NiPoint3(-pkPlane->GetSize().x/2,0,-pkPlane->GetSize().y/2),
		NiPoint3(pkPlane->GetSize().x/2,0,-pkPlane->GetSize().y/2),
	};
	NiPortal	*pkNewPortal = NiNew NiPortal(4,kVertex);
	pkNewPortal->SetName(pkEntity->GetName());

	NiTransform	kTransform;
	kTransform.m_fScale = pkTransform->GetScale();
	kTransform.m_Rotate = pkTransform->GetRotation();
	kTransform.m_Translate = pkTransform->GetTranslation();

	pkNewPortal->SetLocalTransform(kTransform);


	std::string	kTargetRoomName;
	if(pkPortal->GetTargetRoom())
	{
		kTargetRoomName = (char const*)pkPortal->GetTargetRoom()->GetName();
	}
	m_kPortalCont.insert(std::make_pair(pkNewPortal,kTargetRoomName));

}
void	PgPSRoomGroup::InitializeRooms(PgWorld::MapObjectMap &kObjectMap)
{
	AssginPortal();
	AssignObject(kObjectMap);
}
void	PgPSRoomGroup::AssginPortal()
{

	NiRoom	*pkTargetRoom = 0;
	for(PortalMap::iterator itor = m_kPortalCont.begin(); itor != m_kPortalCont.end(); ++itor)
	{
		NiPortal	*pkPortal = itor->first;
		std::string	const	&kTargetRoomName = itor->second;

		if(kTargetRoomName.empty())
		{
			pkTargetRoom = m_spOutdoor;
		}
		else
		{
			pkTargetRoom = FindRoom(kTargetRoomName);
		}
		if(!pkTargetRoom)
		{
			continue;
		}

		pkPortal->SetAdjoiner(pkTargetRoom);

		NiTListIterator pkIter = m_kRooms.GetHeadPos();
		while (pkIter)
		{
			PgPSRoom* pkRoom = NiDynamicCast(PgPSRoom,m_kRooms.GetNext(pkIter));
			if(!pkRoom)
			{
				continue;
			}

	
			if(pkRoom->IsYourPortal((char const*)pkPortal->GetName()) == false)
			{
				continue;
			}

			pkRoom->AttachOutgoingPortal(pkPortal);
			break;
		}
	}
}
void	PgPSRoomGroup::AssignObject(PgWorld::MapObjectMap &kObjectMap)
{
	NiTListIterator pkIter = m_kRooms.GetHeadPos();
	while (pkIter)
	{
		PgPSRoom* pkRoom = NiDynamicCast(PgPSRoom,m_kRooms.GetNext(pkIter));
		if(!pkRoom || pkRoom == m_spOutdoor)
		{
			continue;
		}


		pkRoom->AssignObject(kObjectMap,false);

	}

	m_spOutdoor->AssignObject(kObjectMap,true);
}

