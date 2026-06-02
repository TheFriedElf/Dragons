#include "StdAfx.h"
#include "lwRenderMan.h"
#include "PgRenderMan.h"
#include "PgRemoteManager.h"
#include "PgMobileSuit.h"
#include "ServerLib.h"
#include "PgScreenUpdateThread.h"
#include "PgAlphaAccumulator.h"
#include "PgRenderer.h"
#include "PgMovieMgr.h"
#include "PgMissionComplete.h"
#include "PgWorld.h"

lwRenderMan::lwRenderMan(void*)
{
}

bool lwRenderMan::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwRenderMan>(pkState, "RenderMan")
		.def(pkState, constructor<PgRenderMan *>())
		.def(pkState, "AddScene", &lwRenderMan::AddScene)
		.def(pkState, "CleanUpScene", &lwRenderMan::CleanUpScene)
		.def(pkState, "CreateScene", &lwRenderMan::CreateScene)
		.def(pkState, "AddSceneByNo", &lwRenderMan::AddSceneByNo)
		.def(pkState, "RemoveScene", &lwRenderMan::RemoveScene)
		.def(pkState, "RemoveAllScene", &lwRenderMan::RemoveAllScene)
		.def(pkState, "InvalidateSceneContainer", &lwRenderMan::InvalidateSceneContainer)
		//.def(pkState, "PreloadScene", &lwRenderMan::PreloadScene)
		.def(pkState, "FindScene", &lwRenderMan::FindScene)
		.def(pkState, "Draw", &lwRenderMan::Draw)
		.def(pkState, "SetGraphicOption", &lwRenderMan::SetGraphicOption)
		.def(pkState, "SetUseAlphaGroup", &lwRenderMan::SetUseAlphaGroup)
		.def(pkState, "SetFrameDebug", &lwRenderMan::SetFrameDebug)
		;

	return true;
}

void lwRenderMan::CleanUpScene(char const *pcID)
{
	g_kMissionComplete.CleanUp();	//씬이 바뀔 때 미션 결과창이 살아있으면 종료하자
	g_kBossComplete.CleanUp();
	g_kExpeditionComplete.CleanUp();
}

void *lwRenderMan::AddScene(char const *pcID)
{
	if ( g_kRenderMan.FindScene(pcID) == NULL )
	{
		return g_kRenderMan.AddScene(pcID, false);
	}
	return NULL;
}

void *lwRenderMan::CreateScene(char const *pcID)
{
	if (pcID == NULL)
	{
		return 0;
	}
	return NULL;
}

std::wstring ConvWorldID(int const nMapNo)
{
	TBL_DEF_MAP const *pMap = PgWorld::GetMapData(nMapNo);

	if(pMap)
	{
		std::wstring const kWorldID = pMap->strXMLPath;
		return kWorldID;
	}
	return _T("");
}

void *lwRenderMan::AddSceneByNo(int const iMapNo)
{
	if(iMapNo)
	{
		std::wstring kWorldID = ConvWorldID(iMapNo);
		return g_kRenderMan.AddScene(MB(kWorldID), true);
	}
	return NULL;
}

void lwRenderMan::RemoveAllScene(char const *pcExceptWorld)
{
	g_kRenderMan.RemoveAllScene(pcExceptWorld);
}

void lwRenderMan::InvalidateSceneContainer()
{
	g_kRenderMan.InvalidateSceneContainer();
}

void lwRenderMan::RemoveScene(char const *pcWorldID)
{
	if(pcWorldID)
	{
		g_kRenderMan.RemoveScene(pcWorldID);
	}
}

void* lwRenderMan::FindScene(char const *pcID)
{
	if(pcID)
	{
		return g_kRenderMan.FindScene(pcID);	
	}
	return NULL;
}

void lwRenderMan::Draw()
{
	if(!g_kScreenUpdateThread.IsRunning())
	{
		g_kRenderMan.Update(g_pkApp->GetAccumTime(), 0.0f);
		if(!g_kMovieMgr.IsPlay())
		{
			g_kRenderMan.Draw(g_pkApp->GetAccumTime(), 0.0f);
		}
		g_pkApp->IncFrameCount();
	}
}

void lwRenderMan::SetGraphicOption(char const* pcOptionName, int iOptionValue)
{
	g_kRenderMan.SetGraphicOption(pcOptionName, iOptionValue);
}

void lwRenderMan::SetUseAlphaGroup( bool bUse )
{
    PgAlphaAccumulator::SetUseAlphaGroup( bUse );
}

void lwRenderMan::SetFrameDebug( bool bDebug )
{
    PgAlphaAccumulator* pkAccumulator = NiDynamicCast(PgAlphaAccumulator, NiRenderer::GetRenderer()->GetSorter());
	if ( pkAccumulator )
		pkAccumulator->SetFrameDebug( bDebug );
}
