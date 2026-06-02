#ifndef FREEDOM_DRAGONICA_SCENE_PGISCENE_H
#define FREEDOM_DRAGONICA_SCENE_PGISCENE_H

#include "PgIXmlObject.h"
#include "PgIInputObserver.h"

class PgRenderer;

class PgIScene
	: public PgIXmlObject
	, public PgIInputObserver
{
protected:
	PgIScene(unsigned int uiRenderPriority, unsigned int uiInputPriority);

public:
	typedef enum {
		PG_SCENE_STATE_NONE = -1,
		PG_SCENE_STATE_INSTANCED,
		PG_SCENE_STATE_LOADED,
		PG_SCENE_STATE_INITIALIZED,
		PG_SCENE_STATE_ADDED,		// 이 2개의 State는 RenderMan에서 세팅한다. 
		PG_SCENE_STATE_REMOVED,		// 이 2개의 State는 RenderMan에서 세팅한다.
		PG_SCENE_STATE_TERMINATED,
		MAX_SCENE_STATE,
	} SceneState;

	virtual ~PgIScene();
	unsigned int GetRenderPriority();//! 렌더링 우선순위를 반환한다.
	virtual bool Update(float fAccumTime, float fFrameTime) = 0;//! Scene을 최신 상태로 유지한다.
	virtual void Draw(PgRenderer *pkRenderer, float fFrameTime) = 0;//! Scene을 렌더링 한다.
	static bool CompareRenderPriority(PgIScene *left, PgIScene *right);//! 우선순위를 비교한다.
	
	SceneState GetSceneState() { return m_eSceneState; }
	void SetSceneState(SceneState eState) { m_eSceneState = eState; }

	//! 각각 Scene이 Init/Add/Remove 될 때 불리게 된다.
	virtual void OnAddScript() {}
	virtual void OnRemoveScript() {}
	virtual void OnTerminateScript() {}

	virtual void Destroy(){NiDelete this;}
protected:
	//! Scene
	NiNodePtr m_spSceneRoot;

protected:
	unsigned int m_uiRenderPriority;
	SceneState m_eSceneState;
};

#endif // FREEDOM_DRAGONICA_SCENE_PGISCENE_H