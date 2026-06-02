#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_PGRENDERMAN_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_PGRENDERMAN_H

#include "ResourcePool.h"

//#define PG_SCENE_PRELOAD

class PgRenderer;
class PgIScene;

class PgRenderMan 
//	: public NiMemObject
{
	friend struct ::Loki::CreateStatic< PgRenderMan >;

	typedef std::map<std::string, PgIScene*> SceneContainer;
	typedef std::list<PgIScene*> SortedSceneContainer;

public:
	PgRenderMan();
	~PgRenderMan();
	void Terminate();

	bool SetRenderer(NiRenderer *pkRenderer);
	
	//! Shader관련 setup
	bool SetupPostProcessShaders();
	//! 렌더러를 반환한다.
	static PgRenderer *GetRenderer();

	//! 새 Scene을 생성해서 등록한다.
	PgIScene *AddScene(char const *pcID, bool const bIsFromFile = false);

	//! 등록된 Scene을 삭제한다.
	void RemoveScene(char const *pcWorldID);
	void RemoveAllScene(char const *pcExceptWorldID = 0);

	//! 대기 큐에 등록된 Scene을 처리한다.
	void InvalidateSceneContainer();
	
	//! 등록된 Scene에서 찾아서 반환한다.
	PgIScene *FindScene(char const *pcID);

	//! 등록된 Scene들을 렌더링 한다.
	void Draw(float fAccumTime, float fFrameTime);

	//! 등록된 Scene들을 최신의 상태로 유지한다.
	bool Update(float fAccumTime, float fFrameTime);

	void SetGraphicOption(char const* pcOptionName, int iOptionValue);

	int	GetSceneCount()	const	{	return	m_kSceneContainer.size();	}

#ifdef PG_SCENE_PRELOAD
	//! 새 Scene을 미리 생성한다.
	void PreloadScene(char const *pcID);
#endif	

	template<class _T>
	inline bool GetFirstTypeID(std::string& rkOut) const
	{
		SortedSceneContainer::const_iterator iter = m_kSortedSceneContainer.begin();
		while(m_kSortedSceneContainer.end() != iter)
		{
			const SortedSceneContainer::value_type pkElement = (*iter);
			const _T* pkTemp = dynamic_cast<_T*>(pkElement);
			if( pkTemp )
			{
				rkOut = pkTemp->GetID();
				return true;
			}
			++iter;
		}
		return false;
	};

protected:


#ifdef PG_SCENE_PRELOAD
	int findPreloadingScene(char const* pcID);

	class PreloadTProc : public NiThreadProcedure
	{
	public:
		PreloadTProc()
		{
			m_szSceneName[0] = '\0';
			m_bDeleteAtLoadingComplete = false;
		}

		virtual unsigned int ThreadProcedure(void* pvArg);

		static NiFastCriticalSection ms_kCriticalSection;
		char m_szSceneName[MAX_PATH];
		bool m_bDeleteAtLoadingComplete;
	};

	PreloadTProc m_akPreloadTProcs[MAX_THREAD_NUM];
	NiThread* m_apkPreloadThreads[MAX_THREAD_NUM];
	NiFastCriticalSection m_kAddSceneCriticalSection;
#endif

	static PgRenderer *m_pkRenderer;

	SceneContainer m_kSceneContainer;
	SceneContainer m_kAddSceneContainer;
	SceneContainer m_kRemoveSceneContainer;
#ifdef PG_SCENE_PRELOAD
	SceneContainer m_kCachedSceneContainer;
#endif

	SortedSceneContainer m_kSortedSceneContainer;
};

#define g_kRenderMan SINGLETON_STATIC(PgRenderMan)

#endif // FREEDOM_DRAGONICA_RENDER_RENDERER_PGRENDERMAN_H