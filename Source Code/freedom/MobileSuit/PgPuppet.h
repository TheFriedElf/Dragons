#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PUPPET_PGPUPPET_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PUPPET_PGPUPPET_H

#include "PgIWorldObject.h"

class PgAction;

class PgPuppet : public PgIWorldObject
{
	NiDeclareRTTI;

	typedef std::vector< NiAVObject* > ContNiAVObject;
	typedef std::vector< NiFixedString > ContNiFixedString;
public:
	PgPuppet();
	virtual ~PgPuppet();

	static	PgPuppet*	CreatePuppet(std::string const &kName,std::string const &kKFMPath, NiActorManager::SequenceID const &kActivateSeqID,NiTransform const &kTransform,bool bUsePhysX,bool bUseTrigger);

	//! Overriding
	virtual bool Update(float fAccumTime, float fFrameTime);
	virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	virtual void DrawNoZTest(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	virtual void InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup);
	virtual void ReleasePhysX();
	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit = false);
	virtual bool BeforeCleanUp();
	
	virtual	void	SetWorldEventStateID(int iNewID,__int64 iChangeTime,bool bSetImmediate);

	virtual	void	DoLoadingFinishWork();

	//! 엑션을 구동한다. (열기, 닫기, 제거 등)
	bool TransitAction(char const *pcAction);
	bool	PlaySlot(std::string const &kSlot);
	bool	PlayAnimation(int iAnimationID);
	//최초 로드할 때 static 피직스를 제대로 적용하기 위해 PgWorldGsa.cpp의 AddPuppet시 이 함수를
	//사용하여 변환을 저장해두고 피직스 변환시 사용
	void SaveTransform(NiTransform const& kTransform)
	{
		m_kPhysXTransform = kTransform;
	}

	//! Overriding
	virtual NiObject* CreateClone(NiCloningProcess& kCloning);
	virtual void ProcessClone(NiCloningProcess& kCloning);
	int GetCurAnimation();
	void ReloadNif();

	//ActorXML을 가지고 있는 퍼펫인가...
	void	SetHasActorXML(bool const& bHasActorXML) { m_bHasActorXML = bHasActorXML; }
	bool	IsAnimationDone()	const;

	void	SetUsePhysX(bool bUse)	{	m_bUsePhysX = bUse;	}
	void	SetUseTrigger(bool bUse)	{	m_bUseTrigger = bUse;	}
private:

	bool	GetUsePhysX()	const	{	return	m_bUsePhysX;	}
	bool	GetUseTrigger()	const	{	return	m_bUseTrigger;	}

	void	TransitToWorldEventState(int iFrom,int iTo,bool bSetImmediate);
	void	UpdateWorldEventStateTransition();

	NiTransform m_kPhysXTransform;

protected:
	ContNiAVObject m_kRenamedPhysXObject;
	ContNiFixedString m_kOriginalNameOfRenamedPhysXObject;

	NiAudioSourcePtr m_pkCurrentAudioSource;	//! 재생 중인 사운드 소스
	bool	m_bUsePhysX;
	bool	m_bUseTrigger;
	bool	m_bLoadingFinish;
	bool	m_bHasActorXML;

	NiActorManager::SequenceID m_kSeqID;

	bool	m_bNowWorldEventStateTransit;

	std::vector<NiPhysXKinematicSrcPtr> m_kPhysXSrcCont;
	std::vector<NiPhysXScenePtr> m_kPhysXSceneCont;

};

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_PUPPET_PGPUPPET_H