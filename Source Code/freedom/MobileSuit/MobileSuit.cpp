// MobileSuit.cpp : Application 클레스입니다.
//
// 동작 : 
//			- 렌더러, 사운드, 피직 등 서브 시스템을 초기화
//			- Gamebryo의 리소스 환경을 설정

#include "stdafx.h"
#include "MobileSuit.h"
#include "ShaderHelper.h"
#include "WorldMan.h"

NiApplication *g_pkApp;
LuaStateOwner g_luaScript;

// Instancing application
NiApplication* NiApplication::Create()
{
	SetMediaPath("../data/");

	g_pkApp = NiNew CMobileSuit;

	return g_pkApp;
}

CMobileSuit::CMobileSuit()
	: NiApplication("Freedom - MobileSuit", 1024, 768)
{
	#if defined(_DEBUG)
		m_bDebugMode = true;
	#else
		m_bDebugMode = false;
	#endif

	m_fVTMemoryMax = 150000.0f;
	m_fVTPerformanceMax = 360.0f;
}

CMobileSuit::~CMobileSuit()
{
	SAFE_DELETE_NI(m_pkShaderHelper);
}

bool CMobileSuit::Initialize()
{
	if(!NiApplication::Initialize())
		return false;
	
	EnableFrameRate(true);

	//// Scripting
	//
	LuaRemoteDebuggingServer::RegisterState("State", g_luaScript);
	LuaRemoteDebuggingServer::Initialize();
	// TODO : call init script

	//// WorldMan
	//
	m_pkWorldMan = CWorldMan::Create();
	m_pkWorldMan->Load("test1.nif");

	return true;
}

void CMobileSuit::Terminate()
{
	m_spTrnNode = 0;
	m_spActorMgr = 0;

	NiApplication::Terminate();
}

bool CMobileSuit::CreateRenderer()
{
    const char* pcWorkingDir = ConvertMediaFilename("Shaders\\Generated");
    NiMaterial::SetDefaultWorkingDirectory(pcWorkingDir);

    NiWindowRef pWnd;
    if (m_bFullscreen)
        pWnd = m_pkAppWindow->GetWindowReference();
    else
        pWnd = m_pkAppWindow->GetRenderWindowReference();

    char acErrorStr[512];

    m_spRenderer = NiDX9Select::CreateRenderer(
        m_pkAppWindow->GetWindowReference(), 
        m_pkAppWindow->GetRenderWindowReference(), m_bRendererDialog, 
        m_uiBitDepth, m_pkAppWindow->GetWidth(), 
        m_pkAppWindow->GetHeight(), m_bStencil,
        m_bMultiThread, m_bRefRast, m_bSWVertex, m_bNVPerfHUD,
        m_bFullscreen);

    if (m_spRenderer == NULL)
    {
        NiStrcpy(acErrorStr, 512, "DX9 Renderer Creation Failed");
        return false;
    }
    else
    {
        m_spRenderer->SetBackgroundColor(NiColor(0.5f, 0.5f, 0.5f));
    }

	//// Shader
	//
	m_pkShaderHelper = NiNew ShaderHelper;
	if(m_pkShaderHelper != NULL)
	{
		char* apcProgramDirectories[1] = 
        {
			"../Data/Shaders/DX9/"
        };

        char* apcShaderDirectories[1] =
        {
			"../Data/Shaders/"
        };

		if(!m_pkShaderHelper->SetupShaderSystem(apcProgramDirectories, 1, apcShaderDirectories, 1))
		{
			assert("test");
			return false;
		}
	}

    return true;
}

bool CMobileSuit::CreateScene()
{

	m_spScene = NiNew NiNode;
/*
	m_spActorMgr = NiActorManager::Create(ConvertMediaFilename("m_pal.kfm"));

	if (!m_spActorMgr)
	{
		NiMessageBox("couldn't load 'data/m_pal.kfm'", "MobileSuit");
		return false;
	}
*/
	/*
	m_spScene->AttachChild(m_spActorMgr->GetNIFRoot());
	m_spActorMgr->GetNIFRoot()->SetTranslate(0, 100, 0);
	m_spActorMgr->ActivateSequence(0, 1);
	*/

	return true;
}

void CMobileSuit::OnIdle()
{
	if (!MeasureTime())
		return;

	NiApplication::UpdateInput();

	ResetFrameTimings();
    BeginUpdate();
	m_pkWorldMan->Update(m_fFrameTime);
    EndUpdate();

    BeginRender();
    m_pkWorldMan->Draw(m_fFrameTime);
    EndRender();

    UpdateMetrics();
}