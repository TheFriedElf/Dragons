#include "stdafx.h"
#include "PgBreakableObject.H"
#include "PgWorld.H"
#include "PgAction.H"
#include "PgPilot.H"
#include "PgObject.H"

PgBreakableObjectGroupMan	g_kBreakableObjectGroupMan;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgBreakableObjectGroup
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void	PgBreakableObjectGroup::AddObject(PgObject *pkObject)
{

	VBreakableObject::iterator itor = m_vObjectCont.begin();
	for(;itor!=m_vObjectCont.end();itor++)
	{
		PgObject	*pkPrevObject = *itor;

		if(pkPrevObject->GetVerticalLocation()>pkObject->GetVerticalLocation())
		{
			break;
		}
	}

	m_vObjectCont.insert(itor,pkObject);

	RearrangeObjectPosition();	//	오브젝트들의 좌표를 조정한다.
}
void	PgBreakableObjectGroup::RearrangeObjectPosition()
{
	if(!g_pkWorld)
	{
		return;
	}
	int const iTotal = m_vObjectCont.size();
	if(iTotal == 0)
	{
		return;
	}

	PgObject	*pkBottomObj = m_vObjectCont[0];
	NiPoint3	kPosition = pkBottomObj->GetPosition();

	NiPoint3 kBottomPos = g_pkWorld->ThrowRay(kPosition,NiPoint3(0,0,-1),100000);
	if ( kBottomPos==NiPoint3(-1,-1,-1))
	{
		kBottomPos = kPosition;
	}

	kBottomPos.z+=PG_CHARACTER_Z_ADJUST;

	for(int i=0;i<iTotal;i++)
	{
		PgObject	*pkObject = m_vObjectCont[i];

		NiPoint3	kPos = pkObject->GetPosition();

		if(kPos != kBottomPos)
		{
			kPos.x = kBottomPos.x;
			kPos.y = kBottomPos.y;
			pkObject->SetPosition(kPos);

			if(kPos.z>kBottomPos.z)
			{
				PgAction *pkAction = pkObject->ReserveTransitAction("a_breakable_object_pulldown");
				pkAction->SetParamAsPoint(0,kBottomPos);
			}
			else
			{
				pkObject->ReserveTransitAction(ACTIONNAME_IDLE);
				pkObject->SetPosition(kBottomPos);
			}
		}

		float	fScale = 1.0f;
		NiActorManager	*pkAM = pkObject->GetActorManager();

		if(pkAM)
		{
			NiAVObject	*pkNifRoot = pkAM->GetNIFRoot();
			if(pkNifRoot)
			{
				fScale = pkNifRoot->GetScale();
			}
		}

		float	fHeight = pkObject->GetHeight();

		kBottomPos.z+=fHeight*fScale;
	}

}
void	PgBreakableObjectGroup::BreakObject(PgObject *pkBrokenObject)
{
	if(m_vObjectCont.size() == 0)
	{
		return;
	}
	for(VBreakableObject::iterator itor = m_vObjectCont.begin();itor!=m_vObjectCont.end();itor++)
	{
		PgObject	*pkObject = *itor;
		if(pkObject == pkBrokenObject)
		{
			m_vObjectCont.erase(itor);
			break;
		}
	}

	RearrangeObjectPosition();
}

void	PgBreakableObjectGroup::Init()
{
}
void	PgBreakableObjectGroup::Terminate()
{
	ClearAllObject();
}
void	PgBreakableObjectGroup::ClearAllObject()
{
	m_vObjectCont.clear();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgBreakableObjectGroupMan
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void	PgBreakableObjectGroupMan::Init()
{
}
void	PgBreakableObjectGroupMan::Terminate()
{
	ClearAllGroup();
}

PgBreakableObjectGroup*	PgBreakableObjectGroupMan::AddNewGroup(BM::GUID const &kGroupGUID)
{
	PgBreakableObjectGroup	*pkNewGroup = NULL;

	//	이미 있으면 기존 것을 리턴한다.
	pkNewGroup = GetGroup(kGroupGUID);
	if(pkNewGroup)
	{
		return	pkNewGroup;
	}

	pkNewGroup = new PgBreakableObjectGroup();
	pkNewGroup->SetGUID(kGroupGUID);

	m_vObjectGroupCont.insert(std::make_pair(kGroupGUID,pkNewGroup));

	return	pkNewGroup;
}
PgBreakableObjectGroup*	PgBreakableObjectGroupMan::GetGroup(BM::GUID const &kGroupGUID)
{

	VBreakableObjectGroup::iterator itor = m_vObjectGroupCont.find(kGroupGUID);
	if(itor == m_vObjectGroupCont.end())
	{
		return	NULL;
	}

	PgBreakableObjectGroup	*pkFound = itor->second;

	return	pkFound;
}
void	PgBreakableObjectGroupMan::DestroyGroup(BM::GUID const &kGroupGUID)
{
	VBreakableObjectGroup::iterator itor = m_vObjectGroupCont.find(kGroupGUID);
	if(itor == m_vObjectGroupCont.end())
	{
		return;
	}

	PgBreakableObjectGroup	*pkFound = itor->second;
	SAFE_DELETE(pkFound);

	m_vObjectGroupCont.erase(itor);
}
void	PgBreakableObjectGroupMan::ClearAllGroup()
{
	for(VBreakableObjectGroup::iterator itor = m_vObjectGroupCont.begin(); itor != m_vObjectGroupCont.end(); itor++)
	{
		PgBreakableObjectGroup	*pkGroup = itor->second;
		SAFE_DELETE(pkGroup);
	}

	m_vObjectGroupCont.clear();
}