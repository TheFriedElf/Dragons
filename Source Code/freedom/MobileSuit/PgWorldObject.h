#pragma once

#include <NiNode.h>

class PgIWorldObject : public NiNode, public PgIXmlObject
{
public:
	//! PgIWorldObject 소멸자
	virtual ~PgIWorldObject() = 0

	//! WorldObject를 fFrameTime(AccumTime)에 대한 시각으로 갱신
	virtual bool Update(float fAccumTime, float fFrameTime) = 0;

	//! pkRenderer를 이용해서 Draw
	virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) = 0;

	//! 오브젝트에 애니메이션을 지정해준다. 단 즉시 플레이 할 수도 있고 그렇지 않을 수도 있다.
	bool SetTargetAnimation(NiActorManager::SequenceID kSequenceID, bool bActivate = true) = 0;

	//! 현재 설정된 애니메이션을 활성화 한다.
	bool ActivateAnimation() = 0;


protected:
	NiNode *m_spRootNode;
	
	// animation
	NiActorManagerPtr m_spAM;
	NiActorManager::SequenceID m_kCurrentSequenceID;
	
	// render
	NiWireframePropertyPtr m_spWireframe;
	NiBoneLODController *m_pkBoneLOD;
};