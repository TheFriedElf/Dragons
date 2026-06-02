#include "StdAfx.h"
#include "Pilot.h"
#include "WorldMan.h"
#include "Actor.h"

CPilot::CPilot(CWorldMan *pkWorldMan, CActor *pkActor)
{
	m_pkWorldMan = pkWorldMan;
	m_pkActor = pkActor;
	m_bToLeft = false;
	m_pkActor->SetAnimation(6);
}

CPilot::~CPilot(void)
{
}

void CPilot::Update(float fTime)
{
	assert(m_pkActor);

	if(FindPathNormal())
	{
		ConcilDirection();
	}

	//// xxxxxxxxxxxxxxxx to physics
	NiPoint3 kDir = m_pkActor->GetPathNormal().UnitCross(NiPoint3::UNIT_Z);
	m_pkActor->SetTranslate(m_pkActor->GetTranslate() + kDir * fTime * 160.0f);
	////

	m_pkActor->Update(fTime);
}

bool CPilot::FindPathNormal()
{
	assert(m_pkWorldMan);
	assert(m_pkWorldMan->GetPathRoot());
	assert(m_pkActor);

	static NiPoint3 akDirs[] =
	{
		NiPoint3(1.0f, 0.0f, 0.0f),
		NiPoint3(-1.0f, 0.0f, 0.0f),
		NiPoint3(0.0f, 1.0f, 0.0f),
		NiPoint3(0.0f, -1.0f, 0.0f),
	};
	
	static NiPick kPick;

	NiNode *pkPath = m_pkWorldMan->GetPathRoot();

	kPick.ClearResultsArray();
	kPick.SetTarget(pkPath);
	kPick.SetPickType(NiPick::FIND_FIRST);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);
	// kPick.SetSortType(NiPick::SORT);
	kPick.SetReturnNormal(true);
	
	NiPoint3 kPickStart = m_pkActor->GetWorldTranslate() + NiPoint3(0, 0, 30.0f);
	for(int i = 0; i < 4; i++)
	{
		kPick.PickObjects(kPickStart, akDirs[i], true);
	}

	NiPick::Results& rkResults = kPick.GetResults();
	if(rkResults.GetSize() == 0)
	{
		return false;
	}

	NiPick::Record *pkRecord = rkResults.GetAt(0);
	if(pkRecord->GetDistance() > 100)
	{
		return false;
	}

	NiPoint3 kPathNormal = pkRecord->GetNormal();
	kPathNormal.Unitize();
	
	m_pkActor->SetPathNormal(kPathNormal);

	return true;
}

void CPilot::ConcilDirection()
{
	assert(m_pkActor);

	NiPoint3 kDir = m_pkActor->GetPathNormal().UnitCross(NiPoint3::UNIT_Z);
	float fAngle = NiACos(kDir.Dot(NiPoint3::UNIT_Y));

	m_pkActor->SetRotate(NiQuaternion(fAngle, NiPoint3::UNIT_Z));
}