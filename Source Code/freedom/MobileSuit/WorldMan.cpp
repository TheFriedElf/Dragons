#include "StdAfx.h"
#include "WorldMan.h"

extern NiApplication *g_pkApp;

CResourcePool<NiString, NiFontPtr> CWorldMan::ms_kFontPool;
NiString2DPtr CWorldMan::ms_spStrFrameRate;

CWorldMan::CWorldMan()
{
}

CWorldMan::~CWorldMan()
{
	ms_spStrFrameRate = NULL;
}

CWorldMan *CWorldMan::Create()
{
	CWorldMan *pkWorld = NiNew CWorldMan;

	pkWorld->m_spAlphaSorter = NiNew NiAlphaAccumulator;
	pkWorld->m_pkCuller = NiNew NiCullingProcess(&pkWorld->m_kVisibleScene);

	NiFontPtr spFont = NiFont::Create(NiRenderer::GetRenderer(), "../Font/verdana10.NFF");
	assert(spFont);
	ms_kFontPool.Add(NiString("sysinfo"), spFont);

	spFont = NiFont::Create(NiRenderer::GetRenderer(), "../Font/±¼¸²9.NFF");
	assert(spFont);
	ms_kFontPool.Add(NiString("chat"), spFont);

ms_spStrFrameRate = NiNew NiString2D(ms_kFontPool.Get(NiString("sysinfo")), NiFontString::COLORED,
		10, "", NiColorA(1.0f, 0.0f, 0.0f, 1.0f), 1, 1);

	return pkWorld;
}

bool CWorldMan::Load(const char *pcWorldName)
{
	NiStream kStream;

	if (!kStream.Load(g_pkApp->ConvertMediaFilename(pcWorldName)))
	{
		assert(false);
		return false;
	}

	NiAVObject *pRoot = NiDynamicCast(NiAVObject, kStream.GetObjectAt(0));
	if(!pRoot)
	{
		assert(false);
		return false;
	}
	
	m_spSceneRoot = (NiNode *)pRoot;
	
	// Extra Geos
	m_spPathRoot = (NiNode *)pRoot->GetObjectByName("paths");
	if(m_spPathRoot != NULL)
	{
		m_spPathRoot->SetAppCulled(true);
	}
	m_spSpawnRoot = (NiNode *)pRoot->GetObjectByName("char_spawns");
	if(m_spSpawnRoot != NULL)
	{
		m_spSpawnRoot->SetAppCulled(true);
	}

	m_spSceneRoot->UpdateProperties();
	m_spSceneRoot->UpdateEffects();
	RecursivePrepack(m_spSceneRoot);

	((NiDX9Renderer*)NiRenderer::GetRenderer())->PerformPrecache();

	unsigned int cnt = kStream.GetObjectCount();
	for (unsigned int ui = 1; ui < cnt; ui++)
    {
        if (NiIsKindOf(NiCamera, kStream.GetObjectAt(ui)))
        {
            NiCameraPtr spCamera = (NiCamera *)kStream.GetObjectAt(ui);
			
			m_kCameraMan.SetCamera(spCamera);
			m_kCameraMan.AddCamera((NiString)spCamera->GetName(), spCamera);
		}
    }

	SetTurretControl();
	return true;
}

void CWorldMan::RecursivePrepack(NiAVObject* pkObject)
{
    if (pkObject->GetAppCulled())
    {
        return;
    }

    NiRenderer* pkRenderer = NiRenderer::GetRenderer();
    assert(pkRenderer);

    bool bDynamic = false;
    if (!(pkRenderer->GetFlags() & NiRenderer::CAPS_HARDWARESKINNING))
    {
        bDynamic = true;
    }

    if (NiIsKindOf(NiGeometry, pkObject))
    {
        NiGeometry* pkGeom = (NiGeometry*)pkObject;

        if (NiIsKindOf(NiParticles, pkObject))
            bDynamic = true;

        // Search for morpher controllers
        NiTimeController* pkController;

        for (pkController = pkObject->GetControllers(); 
            pkController != NULL; pkController = pkController->GetNext())
        {
            if (NiIsKindOf(NiGeomMorpherController, pkController))
                bDynamic = true;
        }

        NiGeometryData::Consistency eFlags = bDynamic ? 
            NiGeometryData::VOLATILE : NiGeometryData::STATIC;
        pkGeom->SetConsistency(eFlags);

        pkGeom->GetModelData()->SetCompressFlags(NiGeometryData::COMPRESS_ALL);
        pkGeom->GetModelData()->SetKeepFlags(NiGeometryData::KEEP_XYZ |
            NiGeometryData::KEEP_NORM | NiGeometryData::KEEP_INDICES);
       
        pkRenderer->PrecacheGeometry(pkGeom, 0, 0);
    }
    
    if (NiIsKindOf(NiNode, pkObject))
    {
        NiNode* pkNode = (NiNode*)pkObject;
        for (unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
        {
            NiAVObject* pkChild = pkNode->GetAt(i);
            if (pkChild)
                RecursivePrepack(pkChild);
        }
    }
}

void CWorldMan::Draw(float fTime)
{
	NiRenderer* pkRenderer = NiRenderer::GetRenderer();
	NiCameraPtr spCamera = m_kCameraMan.GetCamera();
	
	pkRenderer->BeginFrame();
	pkRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_ALL);

	pkRenderer->SetCameraData(spCamera->GetWorldTranslate(),
        spCamera->GetWorldDirection(), spCamera->GetWorldUpVector(),
        spCamera->GetWorldRightVector(), spCamera->GetViewFrustum(),
        spCamera->GetViewPort());
	

	m_spAlphaSorter->StartAccumulating(spCamera);
	pkRenderer->SetSorter(m_spAlphaSorter);

	m_kVisibleScene.RemoveAll();
	m_pkCuller->Process(spCamera, m_spSceneRoot, &m_kVisibleScene);
	NiDrawVisibleArrayAppend(m_kVisibleScene);

	pkRenderer->GetSorter()->FinishAccumulating();
	pkRenderer->SetSorter(NULL);

	if(ms_spStrFrameRate)
	{
		ms_spStrFrameRate->sprintf("%.0f", 1/g_pkApp->GetFrameTime());
		ms_spStrFrameRate->Draw(pkRenderer);
	}

	pkRenderer->EndUsingRenderTargetGroup();
	pkRenderer->EndFrame();
    pkRenderer->DisplayFrame();
}

void CWorldMan::Update(float fTime)
{
	// Updating camera turret
	if(m_kTurret.Read())
	{
		m_spTrnNode->Update(fTime);
	}

	m_spSceneRoot->Update(fTime);
}

void CWorldMan::SetTurretControl()
{   
	NiCameraPtr spCamera = m_kCameraMan.GetCamera();

    spCamera->Update(0.0f);
    m_spTrnNode = NiNew NiNode();
    m_spTrnNode->SetTranslate(spCamera->GetWorldTranslate());
    spCamera->SetTranslate(NiPoint3::ZERO);
    m_spRotNode = NiNew NiNode();
    m_spTrnNode->AttachChild(m_spRotNode);
    m_spRotNode->SetRotate(spCamera->GetWorldRotate());
    spCamera->SetRotate(NiMatrix3::IDENTITY);
    m_spRotNode->AttachChild(spCamera);
    m_spTrnNode->Update(0.0f);
    
    float fTrnSpeed = 1.0f;
    float fRotSpeed = 0.005f;

    m_kTurret.SetStandardTrn(fTrnSpeed, m_spTrnNode);
    m_kTurret.SetStandardRot(fRotSpeed, m_spTrnNode, m_spRotNode);

    NiMatrix3 kRot;
    kRot.SetCol(0, 1.0f, 0.0f, 0.0f);
    kRot.SetCol(1, 0.0f, 0.0f, 1.0f);
    kRot.SetCol(2, 0.0f, -1.0f, 0.0f);
    m_kTurret.SetAxes(kRot);
    
    m_kTurret.SetTrnButtonsKB(0,
        NiInputKeyboard::KEY_A, NiInputKeyboard::KEY_D);
    m_kTurret.SetTrnButtonsKB(1,
        NiInputKeyboard::KEY_S, NiInputKeyboard::KEY_W);
    m_kTurret.SetTrnButtonsKB(2,
        NiInputKeyboard::KEY_Q, NiInputKeyboard::KEY_E);

    m_kTurret.SetRotButtonsKB(1,
        NiInputKeyboard::KEY_J, NiInputKeyboard::KEY_L);
    m_kTurret.SetRotButtonsKB(2,
        NiInputKeyboard::KEY_K, NiInputKeyboard::KEY_I);
}