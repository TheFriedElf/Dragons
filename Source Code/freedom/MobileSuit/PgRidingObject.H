#pragma once

#include "PgActor.H"

class	PgRidingObject	:	public	PgActor
{
	NiDeclareRTTI;

	typedef std::vector<NiPhysXKinematicSrc*> NiPhysXKinematicSrcCont;
	typedef std::vector<NiPhysXScenePtr> NiPhysXSceneCont;

	NiPhysXSceneCont	m_kPhysXSceneObjCont;
	NiPhysXKinematicSrcCont	m_vKinematicSrcCont;

	NiAVObject	*m_pkAttachedObject;

public:

	struct	MountedActor
	{
		BM::GUID m_kUnitGUID;

		MountedActor(BM::GUID const &kGUID)
			:m_kUnitGUID(kGUID)
		{
		}
	};

	typedef	std::map<BM::GUID,MountedActor> MountedActorCont;

	MountedActorCont	m_kMountedActorMap;

	NiPoint3	m_kPositionSaved;

public:

	PgRidingObject();

	virtual void ReleasePhysX();
	virtual bool BeforeCleanUp();
	
	//! PhysX를 초기화한다.
	virtual	void InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup);
	virtual void UpdatePhysX(float fAccumTime, float fFrameTime);

	virtual bool Update(float fAccumTime, float fFrameTime);
	virtual void UpdateDownwardPass(float fTime, bool bUpdateControllers);
	virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

public:

	void	SetAttachedObject(char const *pkObjectName);
	void	SetAttachedObject(NiAVObject *pkAttachedObject)	{	m_pkAttachedObject = pkAttachedObject;	}
	void	SetTransfromByAttachedObject();
	NiPoint3	const&	GetAttactedObjectPos();

	void	MountActor(PgActor *pkActor);
	void	DemountActor(PgActor *pkActor);
};
