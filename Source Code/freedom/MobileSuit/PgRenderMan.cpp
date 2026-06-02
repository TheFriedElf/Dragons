#include "StdAfx.h"
#include "PgRenderMan.h"
#include "PgRenderer.h"
#include "PgIScene.h"
#include "PgXmlLoader.h"

#include "PgPostProcessMan.h"
#include "PgPostProcessMRT.h"

#include "PgRemoteManager.h"
#include "PgLocalManager.h"

#include "PgWorld.h"
#include "PgActor.h"
#include "PgMobileSuit.h"
#include "PgDynamicTextureMan.h"
#include "PgOption.h"
#include "PgStat.h"
#include "BrightBloom.H"

#include "PgEventTimer.h"
#include "lwguid.h"
#include "PgClientParty.h"

#include "PgFSAA2.H"

#include "PgUIScene.h"


//PgRenderMan *g_pkRenderMan = 0;
PgRenderer *PgRenderMan::m_pkRenderer = 0;

PgRenderMan::PgRenderMan()
{
	m_pkRenderer = NULL;
#ifdef PG_SCENE_PRELOAD
	for (unsigned int uiT = 0; uiT < MAX_THREAD_NUM; uiT++)
	{
		m_apkPreloadThreads[uiT] = NULL;
	}
#endif
}

PgRenderMan::~PgRenderMan()
{
	Terminate();
}

void PgRenderMan::Terminate()
{
	SAFE_DELETE_NI(m_pkRenderer);

	// Delete All Scene
	for(SceneContainer::iterator itr = m_kSceneContainer.begin();
		itr != m_kSceneContainer.end();
		itr++)
	{
		itr->second->Destroy(); // 내부적으로 지운다.
		//		NiDelete itr->second;
	}
	m_kSceneContainer.clear();
	m_kSortedSceneContainer.clear();
}

bool PgRenderMan::SetRenderer(NiRenderer *pkRenderer)
{
	m_pkRenderer = PgRenderer::Create(pkRenderer);
	if(!m_pkRenderer)
	{
		PG_ASSERT_LOG(!"PgRenderer creatation failed");
		return false;
	}
	// 바탕색을 검은색으로
	m_pkRenderer->SetBackgroundColor(NiColor::BLACK);
	return true;
}

bool PgRenderMan::SetupPostProcessShaders()
{
	if (GetRenderer() == NULL)
		return false;

	SetGraphicOption(STR_OPTION_VIEW_DISTANCE,(g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_VIEW_DISTANCE) == 1));
	SetGraphicOption(STR_OPTION_ANTI_ALIAS,(g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_ANTI_ALIAS) == 1));
	SetGraphicOption(STR_OPTION_GLOW,(g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_GLOW) == 1));

	return true;
}

PgRenderer *PgRenderMan::GetRenderer()
{
	return m_pkRenderer;
}

PgIScene *PgRenderMan::AddScene(char const *pcID, bool const bIsFromFile)
{
	PG_ASSERT_LOG(pcID);
	if (pcID == NULL)
		return NULL;

	PG_ASSERT_LOG(pcID[0] != '\0');
	if (pcID[0] == '\0')
		return NULL;

	//! 먼저 찾아본다.
	PgIScene* pkScene = NULL;

#ifdef PG_SCENE_PRELOAD
	SceneContainer::iterator itr = m_kSceneContainer.find(pcID);

	if (itr != m_kSceneContainer.end())
		return itr->second;

	itr = m_kAddSceneContainer.find(pcID);

	if (itr != m_kAddSceneContainer.end())
		return itr->second;

	itr = m_kRemoveSceneContainer.find(pcID);

	if (itr != m_kRemoveSceneContainer.end())
	{
		pkScene = itr->second;
		m_kRemoveSceneContainer.erase(itr);
	}

	itr = m_kCachedSceneContainer.find(pcID);
	if (pkScene == NULL && itr != m_kCachedSceneContainer.end())
	{
		pkScene = itr->second;
	}

	//! Preloading중인지 찾아본다.
	int iThread = findPreloadingScene(pcID);

	if (pkScene == NULL && iThread >= 0 && iThread < MAX_THREAD_NUM
		&& m_apkPreloadThreads[iThread] != NULL)
	{
		//! Preloading이 끝날때까지 기다린다.
		bool waitForThreads = true;		
		while (waitForThreads)
		{
			Sleep(1);
			if (m_apkPreloadThreads[iThread]->WaitForCompletion())
				continue;

			waitForThreads = false;
		}

		//! Preloading이 끝났으니 찾아본다.
		SceneContainer::iterator itr = m_kCachedSceneContainer.find(pcID);

		if (itr != m_kCachedSceneContainer.end())
			pkScene = itr->second;
		else
			//! Loading은 끝났는데 kAddSceneContainter에 안들어있다고?
			PG_ASSERT_LOG(false);
	}

	if (pkScene == NULL)
	{
		//! 어디에도 없으니 만든다.
#else
	{
#endif
		PG_MEM_LOG
		NILOG(PGLOG_LOG, "[PgRenderMan] AddScene - Start Create %s Scene\n", pcID);
		if(bIsFromFile)
		{
			pkScene = dynamic_cast<PgIScene *>(PgXmlLoader::CreateObjectFromFile(pcID, this));
		}
		else
		{
			pkScene = dynamic_cast<PgIScene *>(PgXmlLoader::CreateObject(pcID, this));
		}

		if(!pkScene)
		{
			PgError1("[PgRenderMan] Scene '%s' Create Failed", pcID);
			return 0;
		}		
		NILOG(PGLOG_LOG, "[PgRenderMan] AddScene - End Create %s(%#X) Scene\n", pcID, pkScene);
		PG_MEM_LOG
	}

	if (pkScene != NULL && !(m_kAddSceneContainer.insert(std::make_pair(pcID, pkScene)).second))
	{
		PgError1("[PgRenderMan] Scene '%s' : Can't Insert Container", pcID);
		SAFE_DELETE_NI(pkScene);
		return 0;
	}

	return pkScene;
}

#ifdef PG_SCENE_PRELOAD
void PgRenderMan::PreloadScene(char const *pcID)
{
	if (pcID == NULL)
		return;

	//! 먼저 찾아본다.
	SceneContainer::iterator itr = m_kSceneContainer.find(pcID);

	if (itr != m_kSceneContainer.end())
		return;

	itr = m_kAddSceneContainer.find(pcID);

	if (itr != m_kAddSceneContainer.end())
		return;

	itr = m_kRemoveSceneContainer.find(pcID);

	if (itr != m_kRemoveSceneContainer.end())
	{
		m_kCachedSceneContainer.insert(std::make_pair(pcID, itr->second));
		m_kRemoveSceneContainer.erase(itr);
		return;
	}

	itr = m_kCachedSceneContainer.find(pcID);
	if (itr != m_kCachedSceneContainer.end())
		return;

	//! Preloading중인지 찾아본다.
	int iThread = findPreloadingScene(pcID);

	if (iThread >= 0 && iThread < MAX_THREAD_NUM && m_apkPreloadThreads[iThread] != NULL)
		return;

	for (unsigned int uiT = 0; uiT < MAX_THREAD_NUM; uiT++)
	{
		if (m_apkPreloadThreads[uiT] != NULL)
			continue;

		m_akPreloadTProcs[uiT].m_szSceneName[0] = '\0';
		m_akPreloadTProcs[uiT].m_bDeleteAtLoadingComplete = false;
		SAFE_STRNCPY(m_akPreloadTProcs[uiT].m_szSceneName, pcID);
		m_apkPreloadThreads[uiT] = NiThread::Create(&m_akPreloadTProcs[uiT]);
		m_apkPreloadThreads[uiT]->SetPriority(NiThread::BELOW_NORMAL);
		m_apkPreloadThreads[uiT]->Resume();
	}

	return;
}
#endif

void PgRenderMan::RemoveScene(char const *pcID)
{
	if (pcID == NULL)
		return;

	NILOG(PGLOG_LOG, "[PgRenderMan] RemoveScene - Reserve remove %s Scene\n", pcID);

#ifdef PG_SCENE_PRELOAD
	int iThread = findPreloadingScene(pcID);

	if (iThread >= 0 && iThread < MAX_THREAD_NUM)
	{
		// TODO : 생성중에 그냥 Delete하면 어떻게 될까 두려움;;
		// TODO : 미묘한 차이로 m_bDeleteAtLoadingComplete 가 true가 되기 전에 loading이 끝날 수 있음.
		//        Critical Section으로 보호해야 하나..
		if (m_apkPreloadThreads[iThread]->WaitForCompletion())
		{
			m_akPreloadTProcs[iThread].m_bDeleteAtLoadingComplete = true;
			return;
		}
	}
#endif

	if(m_kAddSceneContainer.size())
	{
		SceneContainer::iterator itr = m_kAddSceneContainer.find(pcID);
		if (itr != m_kAddSceneContainer.end())
		{
			NILOG(PGLOG_ERROR, "[PgRenderMan] RemoveScene - %s Scene removed from addWaitScene\n", pcID);
		}
	}

	SceneContainer::iterator itr = m_kSceneContainer.find(pcID);

	if(itr == m_kSceneContainer.end())
	{
		PgError1("[PgRenderMan] try to remove unknown scene : %s", pcID);
		return;
	}	

	if (itr->second)
	{
		itr->second->OnRemoveScript();
		PgIInputManager::RemoveInputObserver(itr->second);
	}

	m_kRemoveSceneContainer.insert(std::make_pair(itr->first, itr->second));
}

void PgRenderMan::RemoveAllScene(char const *pcExceptWorldID)
{
	NILOG(PGLOG_LOG, "[PgRenderMan] RemoveAllScene %s %s\n", pcExceptWorldID ? "except" : "", pcExceptWorldID ? pcExceptWorldID : "");
	SceneContainer::iterator scene_itr = m_kSceneContainer.begin();
	
	NILOG(PGLOG_LOG, "[PgRenderMan]	  RemoveAllScene currently %d scene exist\n", m_kSceneContainer.size());
	while(m_kSceneContainer.end() != scene_itr)
	{
		const SceneContainer::key_type& rkKey = scene_itr->first;

		if(NULL==pcExceptWorldID || rkKey != pcExceptWorldID)
		{
			NILOG(PGLOG_LOG, "[PgRenderMan]   RemoveAllScene %s scene removed\n", rkKey.c_str());
			RemoveScene(rkKey.c_str());
		}

		++scene_itr;
	}
}

void PgRenderMan::InvalidateSceneContainer()
{
	if(m_kRemoveSceneContainer.size())
	{
		for(SceneContainer::iterator itr = m_kRemoveSceneContainer.begin();
			itr != m_kRemoveSceneContainer.end();
			++itr)
		{
			PG_MEM_LOG
			NILOG(PGLOG_LOG, "[PgRenderMan] %s scene(%#X) remove start\n", itr->first.c_str(), itr->second);

			m_kSceneContainer.erase(itr->first);
			m_kSortedSceneContainer.remove(itr->second);
			if (itr->second)
				itr->second->OnTerminateScript();
			itr->second->Destroy();
			NILOG(PGLOG_LOG, "[PgRenderMan] %s scene remove end\n", itr->first.c_str());
			PG_MEM_LOG
//			NiDelete itr->second;
		}

		m_kRemoveSceneContainer.clear();
		m_kSortedSceneContainer.sort(PgIScene::CompareRenderPriority);
		NILOG(PGLOG_LOG, "[PgRenderMan] After Remove %d scene remains\n", m_kSortedSceneContainer.size());
	}

	if(m_kAddSceneContainer.size())
	{
		for(SceneContainer::iterator itr = m_kAddSceneContainer.begin();
			itr != m_kAddSceneContainer.end();
			++itr)
		{
			NILOG(PGLOG_LOG, "[PgRenderMan] %s scene(%#X) added\n", itr->first.c_str(), itr->second);
			PgIInputManager::AddInputObserver(itr->second);
			m_kSceneContainer.insert(std::make_pair(itr->first, itr->second));
			m_kSortedSceneContainer.push_back(itr->second);
			if (itr->second)
				itr->second->OnAddScript();

			PgWorld* pkWorld = dynamic_cast<PgWorld*>(itr->second);//EventTimer에 현재 SceneID 변경
			if( pkWorld )
			{
				g_kEventTimer.ChangeScene(pkWorld->GetID().c_str());
				g_kParty.ClearCache();
				g_kStatusEffectMan.ClearReserveEffectAll();
			}
		}

		m_kAddSceneContainer.clear();
		m_kSortedSceneContainer.sort(PgIScene::CompareRenderPriority);
		NILOG(PGLOG_LOG, "[PgRenderMan] After Add %d scene remains\n", m_kSortedSceneContainer.size());
	}

	PgIInputManager::InvalidateInputObserverContainer();
}

PgIScene *PgRenderMan::FindScene(char const *pcID)
{
	if (pcID == NULL)
		return NULL;

	SceneContainer::iterator itr = m_kSceneContainer.find(pcID);
	
	if(itr == m_kSceneContainer.end())
	{
		NILOG(PGLOG_WARNING, "unknown scene : %s", pcID);
		return NULL;
	}

	return itr->second;
}

void PgRenderMan::Draw( float fAccumTime, float fFrameTime )
{
    PROFILE_FUNC();

    if ( g_pkWorld && g_pkWorld->IsActivateWorldMap() == false )
    {
        g_pkWorld->Draw( m_pkRenderer, fFrameTime );
    }
    else
    {
        m_pkRenderer->BeginFrame();
        g_kUIScene.Draw( m_pkRenderer, fFrameTime );
        m_pkRenderer->EndFrame();
        m_pkRenderer->DisplayFrame();
    }
}

bool PgRenderMan::Update(float fAccumTime, float fFrameTime)
{
	PgIScene *pScene;
	//PG_ASSERT_LOG(m_kSortedSceneContainer.size() >= 2);

	// 우선순위에 따라서 갱신한다.
	for(SortedSceneContainer::iterator itr = m_kSortedSceneContainer.begin();
		itr != m_kSortedSceneContainer.end();
		++itr)
	{		
		pScene = *itr;		
		PG_STAT(PgStatTimerF timerD(g_kRenderManStatGroup.GetStatInfo(std::string("RenderMan.Update.") + (*itr)->GetID()), g_pkApp->GetFrameCount()));
		if((NULL==pScene) || (!pScene->Update(fAccumTime, fFrameTime)))
		{
			PG_ASSERT_LOG(!"scene updating failed");
			return false;
		}
	}

	InvalidateSceneContainer();
#ifdef PG_SCENE_PRELOAD
	for (unsigned int uiT = 0; uiT < MAX_THREAD_NUM; uiT++)
	{
		if (m_apkPreloadThreads[uiT] != NULL && m_apkPreloadThreads[uiT]->GetStatus() == NiThread::COMPLETE)
		{
			SAFE_DELETE_NI(m_apkPreloadThreads[uiT]);
		}
	}
#endif

	return true;
}
#ifdef PG_SCENE_PRELOAD
int PgRenderMan::findPreloadingScene(char const* pcID)
{
	int iThread = -1;
	for (unsigned int uiT = 0; uiT < MAX_THREAD_NUM; uiT++)
	{
		if (m_apkPreloadThreads[uiT] == NULL)
			continue;

		if (strnicmp(m_akPreloadTProcs[uiT].m_szSceneName, pcID, MAX_PATH) != 0)
			continue;

		iThread = uiT;
	}
	return iThread;
}

unsigned int PgRenderMan::PreloadTProc::ThreadProcedure(void* pvArg)
{
	PgIScene *pkScene = dynamic_cast<PgIScene *>(PgXmlLoader::CreateObject(m_szSceneName, g_pkRenderMan));
	if(!pkScene)
	{
		PgError1("Scene '%s' Create Failed", m_szSceneName);
		return 0;
	}

	if (m_bDeleteAtLoadingComplete)
	{
		SAFE_DELETE_NI(pkScene);
		return 0;
	}

	if(!(g_kRenderMan.m_kCachedSceneContainer.insert(std::make_pair(m_szSceneName, pkScene)).second))
	{
		PgError1("Scene '%s' : Can't Insert Container", m_szSceneName);
		SAFE_DELETE_NI(pkScene);
		return 0;
	}
	return 0;
}
#endif

void PgRenderMan::SetGraphicOption(char const* pcOptionName, int iOptionValue)
{
	if (pcOptionName == NULL)
		return;


	if (_strnicmp(pcOptionName,STR_OPTION_WORLD_QUALITY,strlen(STR_OPTION_WORLD_QUALITY)) == 0)
	{
		if(g_pkWorld)
		{
			g_pkWorld->ApplyWorldQualityOption();
		}
	}
	if (_strnicmp(pcOptionName,STR_OPTION_VIEW_DISTANCE,strlen(STR_OPTION_VIEW_DISTANCE)) == 0)
	{
		if( g_pkWorld )
		{
			g_pkWorld->ApplyFogOption();
		}
	}
	if (_strnicmp(pcOptionName,STR_OPTION_TEXTURE_RES,strlen(STR_OPTION_TEXTURE_RES)) == 0)
	{
		if( g_pkWorld )
		{
			g_pkWorld->ApplyTextureResOption();
		}
	}
	if (_strnicmp(pcOptionName,STR_OPTION_TEXTURE_FILTERING,strlen(STR_OPTION_TEXTURE_FILTERING)) == 0)
	{
		if( g_pkWorld )
		{
			g_pkWorld->ApplyTextureFilteringOption();
		}
	}
	
	if (_strnicmp(pcOptionName, STR_OPTION_ANTI_ALIAS, strlen(STR_OPTION_ANTI_ALIAS)) == 0)
	{
		if(PgFSAA2::GetUseShaderFSAA())
		{
			if (m_pkRenderer && m_pkRenderer->m_pkPostProcessMan)
			{
				bool bBloom = (g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_GLOW) == 1);
				bool bFSAA = ((iOptionValue == 1) && (bBloom == 0));	//	Bloom 이 켜져 있을 때에는 Bloom 에서 FSAA 를 처리한다.

				PgIPostProcessEffect	*pkFSAA = m_pkRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::FSAA);
				BrightBloom	*pkBloom = (BrightBloom*)m_pkRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);

				if(pkBloom)
				{
					pkBloom->SetAntiAlias((iOptionValue == 1));
				}

				if(bFSAA)
				{
					if(!pkFSAA)
					{
						m_pkRenderer->m_pkPostProcessMan->AddEffect(PgIPostProcessEffect::FSAA);
						pkFSAA = m_pkRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::FSAA);
					}

					if(!pkFSAA)
					{
						return;
					}

					pkFSAA->SetActive(true);
				}
				else
				{
					if(pkFSAA)
					{
						pkFSAA->SetActive(false);
					}
				}
			}
		}
		else
		{
			NiDX9Renderer* pkDX9Renderer = NiDynamicCast(NiDX9Renderer, 
				NiRenderer::GetRenderer());

			pkDX9Renderer->GetD3DDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS,iOptionValue == 1);
		}
	}

	if (_strnicmp(pcOptionName, STR_OPTION_GLOW, strlen(STR_OPTION_GLOW)) == 0)
	{
		if(PgFSAA2::GetUseShaderFSAA())
		{		
			if(!m_pkRenderer || !m_pkRenderer->m_pkPostProcessMan)
			{
				return;
			}

			bool bBloom = (iOptionValue == 1);

			BrightBloom	*pkBloom = (BrightBloom*)m_pkRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
			PgIPostProcessEffect	*pkFSAA = m_pkRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::FSAA);

			bool	bFSAA = (g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_ANTI_ALIAS) == 1);

			if(bBloom)
			{
				if(!pkBloom)
				{
					m_pkRenderer->m_pkPostProcessMan->AddEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
					pkBloom = (BrightBloom*)m_pkRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
				}

				if(!pkBloom)
				{
					return;
				}

				pkBloom->SetActive(true);
				pkBloom->SetAntiAlias(bFSAA);

				if(pkFSAA)
				{
					pkFSAA->SetActive(false);
				}
			}
			else
			{
				if(pkBloom)
				{
					pkBloom->SetActive(false);
				}

				if(bFSAA)
				{
					if(!pkFSAA)
					{
						m_pkRenderer->m_pkPostProcessMan->AddEffect(PgIPostProcessEffect::FSAA);
						pkFSAA = m_pkRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::FSAA);
					}

					if(!pkFSAA)
					{
						return;
					}

					pkFSAA->SetActive(true);
				}
				else
				{
					if(pkFSAA)
					{
						pkFSAA->SetActive(false);
					}
				}
			}
			
		}
		else
		{
			if(!m_pkRenderer || !m_pkRenderer->m_pkPostProcessMan)
			{
				return;
			}
			bool bBloom = (iOptionValue == 1);

			BrightBloom	*pkBloom = (BrightBloom*)m_pkRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
			if(bBloom)
			{
				if(!pkBloom)
				{
					m_pkRenderer->m_pkPostProcessMan->AddEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
					pkBloom = (BrightBloom*)m_pkRenderer->m_pkPostProcessMan->GetEffect(PgIPostProcessEffect::BRIGHT_BLOOM);
				}

				if(!pkBloom)
				{
					return;
				}

				pkBloom->SetActive(true);
			}
			else
			{
				if(pkBloom)
				{
					pkBloom->SetActive(false);
				}
			}
		}
	}
}