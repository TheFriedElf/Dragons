#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_OBEJCT_PGOBJECT_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_OBEJCT_PGOBJECT_H

#include "PgActor.H"
#include "variant/PgObjectUnit.H"

class PgQuestMarkHelp;

class	PgObject	:	public	PgActor
{

	NiDeclareRTTI;

public:

	struct	stBreakableObjectInfo
	{
		BM::GUID m_kGroupGUID;
		int	m_iVLoc;
		float	m_fHeight;

		stBreakableObjectInfo():m_iVLoc(0),m_fHeight(0)
		{
		};
	};

	struct	MountedActor
	{
		BM::GUID m_kUnitGUID;

		MountedActor(BM::GUID const &kGUID)
			:m_kUnitGUID(kGUID)
		{
		}
	};
	typedef	std::map<BM::GUID,MountedActor> MountedActorCont;

	struct	stWayPoint
	{
		NiPoint3	m_kPos;
		float	m_fTime;

		stWayPoint(NiPoint3 const &kPos,float fTime)
			:m_kPos(kPos),m_fTime(fTime)
		{
		}
	};

	struct	stRidingObjectInfo
	{
		typedef std::vector<stWayPoint> WayPointCont;

		WayPointCont	m_kWayPoint;
		float	m_fTimeForRoundingWayPoint;	//	한번 끝까지 갔다가 제자리로 돌아오는데 걸리는 시간

		NiAVObject	*m_pkAttachedObject;

		MountedActorCont	m_kMountedActorMap;
		NiPoint3	m_kPositionSaved;

		stRidingObjectInfo():m_pkAttachedObject(NULL),m_fTimeForRoundingWayPoint(0)
		{
		};
	};

private:

	stBreakableObjectInfo	m_kBreakableObjectInfo;
	stRidingObjectInfo		m_kRidingObjectInfo;

	EObjUnit_Attr	m_kAttribute;
	int	m_iMoveSpeed;

	bool	m_bIsWorldEventObject;
	int	m_iPrevWorldEventStateID;
	

public:

	PgObject();

protected:

	virtual void	DoLoadingFinishWork();

public:
	bool	HasAttribute(EObjUnit_Attr const &kAttribute);

	virtual	bool BeforeUse();
	virtual bool BeforeCleanUp();

	virtual	PgIWorldObject*	CreateCopy();
	
	//! PhysX를 초기화한다.
	virtual	void InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup);
	virtual void UpdatePhysX(float fAccumTime, float fFrameTime);

	virtual bool Update(float fAccumTime, float fFrameTime);
	virtual void UpdateDownwardPass(float fTime, bool bUpdateControllers);
	virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	virtual	void	SetWorldEventStateID(int iNewID,__int64 iChangeTime,bool bSetImmediate);

	//	For Breakble Attribute
	void	Break();

	BM::GUID const	&GetParentGroupGUID();
	int	GetVerticalLocation();

	float	GetHeight();

	//	For Ridable Attribute
	void	SetAttachedObject(char const *pkObjectName);
	void	SetAttachedObject(NiAVObject *pkAttachedObject)	
	{	
		m_kRidingObjectInfo.m_pkAttachedObject = pkAttachedObject;	
	}
	void	SetTransfromByAttachedObject();
	NiPoint3	const&	GetAttactedObjectPos();

	bool	MountActor(PgActor *pkActor);
	void	DemountActor(PgActor *pkActor);
	int	GetMoveSpeed()	{	return	m_iMoveSpeed;	}

	//	엘리베이터 등의 RidingObject 의 경우
	//	WayPoint 를 따라 움직일 경우
	//	움직이기 시작한 후 fElapsedTime 이 흘렀을 때의 위치를 구한다.
	NiPoint3	GetRidingObjectPosition(float fElapsedTime);


	//!	액터의 스페큘러를 켠다
	virtual	void	TurnOnSpecular();
	//!	액터의 스페큘러를 원상복구시킨다.
	virtual	void	RestoreSpecular();

	virtual void UpdateQuestDepend();

public:

	void	ParseFromWorldXML(const TiXmlNode *pkNode);

private:

	void	CalcTimeForRoundingWayPoint();
	NiPoint3	GetRidingObjectPosition_WorldEventObject(float fElapsedTime);

	bool	GetWorldEventObject()	const	{	return	m_bIsWorldEventObject;	}
	void	SetWorldEventObject(bool bIsWorldEventObject)	{	m_bIsWorldEventObject = bIsWorldEventObject;	}

	int	GetPrevWorldEventStateID()	const	{	return	m_iPrevWorldEventStateID;	}
	void	SetPrevWorldEventStateID(int iStateID)	{	m_iPrevWorldEventStateID = iStateID;	}	

	PgQuestMarkHelp* m_pkQuestMarkHelp;
};
#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_OBEJCT_PGOBJECT_H