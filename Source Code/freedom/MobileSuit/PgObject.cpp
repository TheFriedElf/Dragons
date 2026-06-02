#include "stdafx.h"
#include "PgObject.H"
#include "variant/PgObjectUnit.H"
#include "PgPilot.H"
#include "PgBreakableObject.H"
#include "PgWorld.H"
#include "PgPilotMan.H"
#include "PgQuestMan.h"
#include "PgNetwork.h"
#include "PgQuestMarkHelp.h"
#include "PgPhysXUtil.H"

NiImplementRTTI(PgObject, PgActor);

PgObject::PgObject() : 
m_kAttribute(EObj_Attr_None),
m_iMoveSpeed(0),
m_bIsWorldEventObject(false),
m_iPrevWorldEventStateID(-1),
m_pkQuestMarkHelp(NULL)
{
}
bool	PgObject::HasAttribute(EObjUnit_Attr const &kAttribute)
{
	return	(m_kAttribute&kAttribute) != 0;
}

bool PgObject::BeforeUse()
{
	PgObjectUnit	*pkObjUnit = NULL;
	if(GetPilot())
	{
		pkObjUnit = (PgObjectUnit*)(GetPilot()->GetUnit());
		PG_ASSERT_LOG(pkObjUnit);
	}

	if(pkObjUnit)
	{
		m_kAttribute = static_cast<EObjUnit_Attr>(pkObjUnit->Attributes());
		m_iMoveSpeed = pkObjUnit->GetAbil(AT_MOVESPEED);
	}

	if(HasAttribute(EObj_Attr_Breakable))
	{
		if(pkObjUnit)
		{
			SObjUnitGroupIndex	const &kGroupInfo =pkObjUnit->GetGroupIndex() ;

			m_kBreakableObjectInfo.m_kGroupGUID = kGroupInfo.kGroupID;
			m_kBreakableObjectInfo.m_iVLoc = kGroupInfo.iIndex;
			m_kBreakableObjectInfo.m_fHeight = pkObjUnit->GetHeight() ;
		}
	}
	if(HasAttribute(EObj_Attr_CanRide))
	{
		CalcTimeForRoundingWayPoint();
	}

	return	PgActor::BeforeUse();
}
bool PgObject::BeforeCleanUp()
{
	if(g_pkWorld)
	{
		g_pkWorld->RemoveLightObjectRecurse(g_pkWorld->GetLightRoot(),this,PgWorld::LT_ALL);
	}

	if(HasAttribute(EObj_Attr_Breakable))
	{
		PgBreakableObjectGroup	*pkGroup = g_kBreakableObjectGroupMan.GetGroup(GetParentGroupGUID());
		if(pkGroup)
		{
			pkGroup->BreakObject(this);
		}
	}

	m_kRidingObjectInfo.m_kMountedActorMap.clear();
	m_kRidingObjectInfo.m_pkAttachedObject = NULL;
	g_kQuestMarkHelpMng.DestroyNode(m_pkQuestMarkHelp);

	return	PgActor::BeforeCleanUp();
}
PgIWorldObject*	PgObject::CreateCopy()
{
	PgObject	*pkNewObject = NiNew PgObject();
	PgActor::CreateCopyEx(pkNewObject);
	return	pkNewObject;
}
void	PgObject::Break()
{
	if(HasAttribute(EObj_Attr_Breakable))
	{
		ReleaseABVShapes();

		PgBreakableObjectGroup	*pkGroup = g_kBreakableObjectGroupMan.GetGroup(GetParentGroupGUID());
		if(!pkGroup)
		{
			return;
		}

		pkGroup->BreakObject(this);
	}
}
void	PgObject::DoLoadingFinishWork()
{

	const	bool	bLoadingComplete = m_bLoadingComplete;

	PgActor::DoLoadingFinishWork();

	if(!bLoadingComplete && m_bLoadingComplete)
	{
		if(HasAttribute(EObj_Attr_Breakable))
		{
			PgBreakableObjectGroup	*pkGroup = g_kBreakableObjectGroupMan.GetGroup(GetParentGroupGUID());
			if(!pkGroup)
			{
				pkGroup = g_kBreakableObjectGroupMan.AddNewGroup(GetParentGroupGUID());
			}

			pkGroup->AddObject(this);
		}
	}
}
BM::GUID const	&PgObject::GetParentGroupGUID()	
{	
	return	m_kBreakableObjectInfo.m_kGroupGUID;
}
int	PgObject::GetVerticalLocation()	
{	
	return	m_kBreakableObjectInfo.m_iVLoc;
}
float	PgObject::GetHeight()
{
	return	m_kBreakableObjectInfo.m_fHeight;
}

void	PgObject::SetAttachedObject(char const *pkObjectName)
{
	if(!GetWorld())
	{
		return;
	}

	NiNode	*pkSceneRoot = GetWorld()->GetSceneRoot();
	if(!pkSceneRoot)
	{
		return;
	}

	NiAVObject	*pkObject = pkSceneRoot->GetObjectByName(pkObjectName);
	if(pkObject)
	{
		pkObject->SetAppCulled(true);
		SetAttachedObject(pkObject);
	}
}
NiPoint3	const&	PgObject::GetAttactedObjectPos()
{
	if(!m_kRidingObjectInfo.m_pkAttachedObject)
	{
		static	NiPoint3	kInvalidPos(-1,-1,-1);
		return	kInvalidPos;
	}

	return	m_kRidingObjectInfo.m_pkAttachedObject->GetWorldTranslate();
}

void	PgObject::SetTransfromByAttachedObject()
{
	if(!m_kRidingObjectInfo.m_pkAttachedObject)
	{
		return;
	}

	NiPoint3	const &kPosition = m_kRidingObjectInfo.m_pkAttachedObject->GetWorldTranslate();
	SetPosition(kPosition);
}
bool	PgObject::MountActor(PgActor *pkActor)
{
	if(HasAttribute(EObj_Attr_CanRide) == false)
	{
		return false;
	}

	if(!pkActor)
	{
		return false;
	}

	pkActor->SetMeetFloor(true);

	BM::GUID kPilotGuid = pkActor->GetPilotGuid();
	MountedActorCont::iterator itor = m_kRidingObjectInfo.m_kMountedActorMap.find(kPilotGuid);
	if(itor != m_kRidingObjectInfo.m_kMountedActorMap.end())
	{
		return false;
	}

	m_kRidingObjectInfo.m_kMountedActorMap.insert(std::make_pair(kPilotGuid,MountedActor(kPilotGuid)));

	return	true;

}
void	PgObject::DemountActor(PgActor *pkActor)
{
	if(HasAttribute(EObj_Attr_CanRide) == false)
	{
		return;
	}

	if(!pkActor)
	{
		return;
	}

	BM::GUID kPilotGuid = pkActor->GetPilotGuid();
	MountedActorCont::iterator itor = m_kRidingObjectInfo.m_kMountedActorMap.find(kPilotGuid);
	if(itor != m_kRidingObjectInfo.m_kMountedActorMap.end())
	{
		m_kRidingObjectInfo.m_kMountedActorMap.erase(itor);
		return;
	}
}
void	PgObject::SetWorldEventStateID(int iNewID,__int64 iChangeTime,bool bSetImmediate)
{

	SetPrevWorldEventStateID(GetWorldEventStateID());

	if(iNewID >= 0 )
	{
		SetWorldEventObject(true);
	}

	PgIWorldObject::SetWorldEventStateID(iNewID,iChangeTime,bSetImmediate);
}
NiPoint3	PgObject::GetRidingObjectPosition_WorldEventObject(float fElapsedTime)
{
	if(GetWorldEventStateID()<0 || GetWorldEventStateID()>=m_kRidingObjectInfo.m_kWayPoint.size())
	{
		return	GetPosition();
	}

	float	fMoveSpeed = GetMoveSpeed();
	if(fMoveSpeed == 0)
	{
		return	GetPosition();
	}

	float	fRemainTime = fElapsedTime - GetWorldEventStateChangeTime()/1000.0f;

	if(GetPrevWorldEventStateID() == -1
		|| (GetPrevWorldEventStateID() == GetWorldEventStateID()) )
	{
		return	m_kRidingObjectInfo.m_kWayPoint[GetWorldEventStateID()].m_kPos;
	}

	if(GetWorldEventStateID() > GetPrevWorldEventStateID())	//	정방향
	{
		for(int i=GetPrevWorldEventStateID();i<GetWorldEventStateID();i++)
		{
			stWayPoint const	&kWayPoint = m_kRidingObjectInfo.m_kWayPoint[i];

			fRemainTime -= kWayPoint.m_fTime;
			if(fRemainTime<=0)
			{
				return	kWayPoint.m_kPos;
			}

			stWayPoint &kWayPointNext = m_kRidingObjectInfo.m_kWayPoint[i+1];

			float	fDistance = (kWayPoint.m_kPos - kWayPointNext.m_kPos).Length();
			if(fDistance == 0) continue;

			float	fMoveTime = (3*fDistance) / (2*fMoveSpeed);

			fRemainTime -= fMoveTime;
			if(fRemainTime<0)
			{
				fRemainTime = -fRemainTime;

				float	fPassedTime = fMoveTime - fRemainTime;

				float	fA = -(2/(fMoveTime*fMoveTime*fMoveTime));
				float	fB = 3/(fMoveTime*fMoveTime);

				float	fDistRate = fA*(fPassedTime*fPassedTime*fPassedTime) + fB*(fPassedTime*fPassedTime);
				if(fDistRate>1)
				{
					fDistRate = 1;
				}

				NiPoint3	kPos = kWayPoint.m_kPos + (kWayPointNext.m_kPos - kWayPoint.m_kPos)*fDistRate;
				return	kPos;
			}

		}
	}
	else	//	역방향
	{
		for(int i=GetPrevWorldEventStateID();i>GetWorldEventStateID();i--)
		{
			stWayPoint const	&kWayPoint = m_kRidingObjectInfo.m_kWayPoint[i];

			fRemainTime -= kWayPoint.m_fTime;
			if(fRemainTime<=0)
			{
				return	kWayPoint.m_kPos;
			}

			stWayPoint &kWayPointNext = m_kRidingObjectInfo.m_kWayPoint[i-1];

			float	fDistance = (kWayPoint.m_kPos - kWayPointNext.m_kPos).Length();
			if(fDistance == 0) continue;

			float	fMoveTime = (3*fDistance) / (2*fMoveSpeed);

			fRemainTime -= fMoveTime;
			if(fRemainTime<0)
			{
				fRemainTime = -fRemainTime;

				float	fPassedTime = fMoveTime - fRemainTime;

				float	fA = -(2/(fMoveTime*fMoveTime*fMoveTime));
				float	fB = 3/(fMoveTime*fMoveTime);

				float	fDistRate = fA*(fPassedTime*fPassedTime*fPassedTime) + fB*(fPassedTime*fPassedTime);
				if(fDistRate>1)
				{
					fDistRate = 1;
				}

				NiPoint3	kPos = kWayPoint.m_kPos + (kWayPointNext.m_kPos - kWayPoint.m_kPos)*fDistRate;
				return	kPos;
			}
		}
	}

	SetPrevWorldEventStateID(GetWorldEventStateID());	//	translation completed.

	return	GetPosition();
}

//	when this object is an riding object, and moving along the waypoints,
//	this function calculates the position of the object when the time passed for fElapsedTime from the start.
NiPoint3	PgObject::GetRidingObjectPosition(float fElapsedTime)
{
	if(GetWorldEventObject())
	{
		return	GetRidingObjectPosition_WorldEventObject(fElapsedTime);
	}

	float	fMoveSpeed = GetMoveSpeed();
	if(fMoveSpeed == 0)
	{
		return	GetPosition();
	}

	int	iRoundingCount = fElapsedTime/m_kRidingObjectInfo.m_fTimeForRoundingWayPoint;
	float	fRemainTime = fElapsedTime - iRoundingCount*m_kRidingObjectInfo.m_fTimeForRoundingWayPoint;

	int	iTotalPoint = m_kRidingObjectInfo.m_kWayPoint.size();
	//	정방향 진행
	for(int i=0;i<iTotalPoint-1;i++)
	{
		stWayPoint &kWayPoint = m_kRidingObjectInfo.m_kWayPoint[i];

		fRemainTime -= kWayPoint.m_fTime;

		if(fRemainTime<0)
		{
			return	kWayPoint.m_kPos;
		}

		stWayPoint &kWayPointNext = m_kRidingObjectInfo.m_kWayPoint[i+1];

		float	fDistance = (kWayPoint.m_kPos - kWayPointNext.m_kPos).Length();
		if(fDistance == 0) continue;

		float	fMoveTime = (3*fDistance) / (2*fMoveSpeed);

		fRemainTime -= fMoveTime;
		if(fRemainTime<0)
		{
			fRemainTime = -fRemainTime;

			float	fPassedTime = fMoveTime - fRemainTime;

			float	fA = -(2/(fMoveTime*fMoveTime*fMoveTime));
			float	fB = 3/(fMoveTime*fMoveTime);

			float	fDistRate = fA*(fPassedTime*fPassedTime*fPassedTime) + fB*(fPassedTime*fPassedTime);
			if(fDistRate>1)
			{
				fDistRate = 1;
			}

			NiPoint3	kPos = kWayPoint.m_kPos + (kWayPointNext.m_kPos - kWayPoint.m_kPos)*fDistRate;
			return	kPos;
		}
	}

	//	역방향 진행
	for(int i=iTotalPoint-1;i>0;i--)
	{
		stWayPoint &kWayPoint = m_kRidingObjectInfo.m_kWayPoint[i];

		fRemainTime -= kWayPoint.m_fTime;

		if(fRemainTime<0)
		{
			return	kWayPoint.m_kPos;
		}

		stWayPoint &kWayPointPrev = m_kRidingObjectInfo.m_kWayPoint[i-1];

		float	fDistance = (kWayPoint.m_kPos - kWayPointPrev.m_kPos).Length();
		if(fDistance == 0) continue;

		float	fMoveTime = (3*fDistance) / (2*fMoveSpeed);

		fRemainTime -= fMoveTime;
		if(fRemainTime<0)
		{
			fRemainTime = -fRemainTime;
			float	fPassedTime = fMoveTime - fRemainTime;

			float	fA = -(2/(fMoveTime*fMoveTime*fMoveTime));
			float	fB = 3/(fMoveTime*fMoveTime);

			float	fDistRate = fA*(fPassedTime*fPassedTime*fPassedTime) + fB*(fPassedTime*fPassedTime);
			if(fDistRate>1)
			{
				fDistRate = 1;
			}

			NiPoint3	kPos = kWayPoint.m_kPos + (kWayPointPrev.m_kPos - kWayPoint.m_kPos)*fDistRate;
			return	kPos;
		}
	}

	return	GetPosition();	//	Error 연산오류?
}
void	PgObject::CalcTimeForRoundingWayPoint()	//	this function calculates the time for rounding whole waypoints once.
{
	if(!GetPilot())
	{
		return;
	}

	float	fMoveSpeed = GetPilot()->GetAbil(AT_MOVESPEED);

	float	fTime = 0;
	int	iTotalPoint = m_kRidingObjectInfo.m_kWayPoint.size();

	for(int i=0;i<iTotalPoint-1;i++)
	{
		stWayPoint &kWayPoint = m_kRidingObjectInfo.m_kWayPoint[i];

		fTime += kWayPoint.m_fTime;

		stWayPoint &kWayPointNext = m_kRidingObjectInfo.m_kWayPoint[i+1];

		float	fDistance = (kWayPoint.m_kPos - kWayPointNext.m_kPos).Length();
		if(fDistance == 0) continue;

		float	fMoveTime = (3*fDistance) / (2*fMoveSpeed);
		fTime += fMoveTime;
	}

	for(int i=1;i<iTotalPoint;i++)
	{
		stWayPoint &kWayPoint = m_kRidingObjectInfo.m_kWayPoint[i];

		fTime += kWayPoint.m_fTime;

		stWayPoint &kWayPointPrev = m_kRidingObjectInfo.m_kWayPoint[i-1];

		float	fDistance = (kWayPoint.m_kPos - kWayPointPrev.m_kPos).Length();
		if(fDistance == 0) continue;

		float	fMoveTime = (3*fDistance) / (2*fMoveSpeed);
		fTime += fMoveTime;
	}

	m_kRidingObjectInfo.m_fTimeForRoundingWayPoint = fTime;
}
void	PgObject::ParseFromWorldXML(const TiXmlNode *pkNode)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			assert(pkElement);
			
			char const *pcTagName = pkElement->Value();

			if (stricmp(pcTagName, "WayPoint") == 0)
			{
				NiPoint3	kPoint;
				float	fTime=0;

				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();
				
					if(stricmp(pcAttrName, "POINT") == 0)
					{
						sscanf_s(pcAttrValue,"%f,%f,%f",&kPoint.x,&kPoint.y,&kPoint.z);
					}
					else if(strcmp(pcAttrName, "TIME") == 0)
					{
						fTime = atof(pcAttrValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}

				m_kRidingObjectInfo.m_kWayPoint.push_back(stWayPoint(kPoint,fTime));
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
				break;
			}
		}

	default:
		break;
	}

	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		ParseFromWorldXML(pkNextNode);
	}

}
void PgObject::Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{


	//SetTranslate(m_kPositionSaved);
	//SetWorldTranslate(m_kPositionSaved);
	//NiNode::Update(g_pkWorld->GetAccumTime());

	PgActor::Draw(pkRenderer,pkCamera,fFrameTime);

	//SetTranslate(kMovedPosition);
	//SetWorldTranslate(kMovedPosition);
	//NiNode::Update(g_pkWorld->GetAccumTime());

}
bool PgObject::Update(float fAccumTime, float fFrameTime)
{
	m_kRidingObjectInfo.m_kPositionSaved = GetWorldTranslate();
	
	bool bResult =	PgActor::Update(fAccumTime,fFrameTime);

	if(HasAttribute(EObj_Attr_CanRide))
	{
		SetTranslate(GetPosition());

		UpdateDownwardPass(fAccumTime,false);

		NiPoint3	kMovedPosition = GetWorldTranslate();
		NiPoint3	kPositionSaved = m_kRidingObjectInfo.m_kPositionSaved;

		NxVec3	kMoveAmount = NxVec3(kMovedPosition.x - kPositionSaved.x,
									kMovedPosition.y - kPositionSaved.y,
									kMovedPosition.z - kPositionSaved.z);

		for(MountedActorCont::iterator itor = m_kRidingObjectInfo.m_kMountedActorMap.begin(); 
			itor != m_kRidingObjectInfo.m_kMountedActorMap.end(); )
		{
			MountedActor &kMountedActor = itor->second;

			PgActor *pkMountedActor = g_kPilotMan.FindActor(kMountedActor.m_kUnitGUID);
			if(!pkMountedActor)
			{
				itor = m_kRidingObjectInfo.m_kMountedActorMap.erase(itor);
				continue;
			}

			pkMountedActor->MoveActorAbsolute(kMoveAmount);

			itor++;
		}

		UpdateRidingInfo(fAccumTime, fFrameTime);

	}

	if( m_pkQuestMarkHelp )
	{
		NiNodePtr pkDummy = (NiNode*)GetObjectByName(ATTACH_POINT_STAR);
		if( pkDummy )
		{
			bool const bHaveHideAbil = ((GetPilot())? GetPilot()->IsHide(): false);
			m_pkQuestMarkHelp->Update(pkDummy->GetWorldTranslate(), (IsHide() || bHaveHideAbil));
		}
	}

	return	bResult;
}
void PgObject::UpdatePhysX(float fAccumTime, float fFrameTime)
{
}

//!	액터의 스페큘러를 켠다
void	PgObject::TurnOnSpecular()
{
	for(SpecularEnableContainer::iterator itor = m_kSpecularContainer.begin(); itor != m_kSpecularContainer.end(); ++itor)
	{
		NiSpecularPropertyPtr spSpecular = itor->first;
		if(spSpecular)
		{
			spSpecular->SetSpecular(true);
		}
		
	}

	SetDefaultMaterialNeedsUpdateFlag(false);	//	쉐이더를 업데이트 시켜야한다.

}
//!	액터의 스페큘러를 원상복구시킨다.
void	PgObject::RestoreSpecular()
{
	for(SpecularEnableContainer::iterator itor = m_kSpecularContainer.begin(); itor != m_kSpecularContainer.end(); ++itor)
	{
		NiSpecularPropertyPtr spSpecular = itor->first;
		if(spSpecular)
		{
			bool	const	bTurnOn = itor->second;

			spSpecular->SetSpecular(bTurnOn);
		}
	}
	SetDefaultMaterialNeedsUpdateFlag(false);	//	쉐이더를 업데이트 시켜야한다.
}

void PgObject::UpdateDownwardPass(float fTime, bool bUpdateControllers)
{
	if(HasAttribute(EObj_Attr_CanRide))
	{
		NiNode::UpdateDownwardPass(fTime,bUpdateControllers);
		return;
	}

	return	PgActor::UpdateDownwardPass(fTime,bUpdateControllers);
}
void PgObject::InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup)
{

	//SetAttachedObject("SD_SkyCastle_Elevator1001_01 01");

	PgActor::InitPhysX(pkPhysXScene,uiGroup);

	if(HasAttribute(EObj_Attr_CanRide) == false)
	{
		return;
	}

	InitRidingInfo(pkPhysXScene, uiGroup);	
}


void PgObject::UpdateQuestDepend()
{
	int const iHelpTextID = 0;
	std::string const kHelpTexture( "../Data/6_UI/quest/qsTxtBg.tga" );

	PgPilot* pkPilot = GetPilot();
	if( !pkPilot )
	{
		return;
	}

	CUnit* pkUnit = pkPilot->GetUnit();
	if( !pkUnit )
	{
		return;
	}
	
	if( !pkUnit->IsInUnitType(UT_OBJECT) )
	{
		return;
	}
	
	bool const bIsDependMonster = g_kQuestMan.IsDependIngQuestMonster(pkPilot->GetAbil(AT_CLASS), g_kNowGroundKey.GroundNo());
	if( bIsDependMonster )
	{
		if( !m_pkQuestMarkHelp )
		{
			m_pkQuestMarkHelp = g_kQuestMarkHelpMng.CreateNode();
		}

		int const iHelpText = 450269;
		std::wstring const kHelpText = TTW(iHelpText);
		if( m_pkQuestMarkHelp
		&&	!kHelpText.empty() )
		{
			m_pkQuestMarkHelp->SetHelpText(kHelpText, kHelpTexture, 0.f);
		}
	}
	else
	{
		g_kQuestMarkHelpMng.DestroyNode(m_pkQuestMarkHelp);
	}
}