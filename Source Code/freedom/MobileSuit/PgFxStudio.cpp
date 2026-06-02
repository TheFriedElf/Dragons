#include "stdafx.h"
#include "PgFxStudio.H"
#include "PgFxAssetManager.H"
#include "PgFxSoundManager.H"
#include "PgNiFile.H"
#include "PgFxAllocator.H"

PgFxAllocator	g_FxAllocator;

PgFxStudio::PgFxStudio()
:m_pkFxStudioManager(NULL)
{
}
PgFxStudio::~PgFxStudio()
{
}
bool	PgFxStudio::Initialize()
{
	m_kFxManager.InitializeStatics();

	m_spCamera = NiNew NiCamera();
	m_kFxManager.SetCamera(m_spCamera);


	m_pkFxStudioManager = FxStudio::CreateManager();
	if(!m_pkFxStudioManager)
	{
		_PgMessageBox("Error","FxStudio Create Manager Failed");
		return	false;
	}

	m_spAssetManager = NiNew PgFxAssetManager();
	m_spSoundManager = NiNew PgFxSoundManager();

	m_kFxManager.SetFxManager(m_pkFxStudioManager);
	m_kFxManager.SetScreenElementsRenderArray(&m_kScreenElementsCont);
	m_kFxManager.SetAssetManager(m_spAssetManager);
	m_kFxManager.SetSoundManager(m_spSoundManager);

	return	LoadBank();
}
bool	PgFxStudio::LoadBank()
{
	char const* const szFxBankFileName = "../Data/5_Effect/GeneralEffect.fxb";

	PgFxBankFilePtr	spBank = NiNew PgFxBankFile();
	spBank->LoadFile(szFxBankFileName);

	if( spBank->IsLoaded() )
	{
		if ( !m_pkFxStudioManager->LoadBank(spBank->GetData(), spBank->GetDataSize()) )
		{
			NILOG( "Failed to load FxBank file \"%s\"\n", szFxBankFileName );
			return false;
		}
	}
	else
	{
		return false;
	}

	m_spBankFile = spBank;

	return	true;
}
void	PgFxStudio::Update(float fFrameTime)
{
	float fPrimaryMotor, fSecondaryMotor;
	m_kFxManager.GetRumbleValues(fPrimaryMotor, fSecondaryMotor);

	if(m_pkFxStudioManager)
	{
		m_pkFxStudioManager->Update(fFrameTime);
	}

	if(m_spAssetManager)
	{
		((PgFxAssetManager*)(FxAssetManager*)m_spAssetManager)->ReleaseFrameDelayedAssets();
	}
}
void	PgFxStudio::DrawScreenElements(NiRenderer *pkRenderer,NiCamera *pkCamera)
{
	pkRenderer->SetScreenSpaceCameraData();

	for(unsigned int ui=0;ui<m_kScreenElementsCont.GetSize();++ui)
	{
		NiScreenElements *pkScreenElement = m_kScreenElementsCont.GetAt(ui);
		if(!pkScreenElement)
		{
			continue;
		}

		pkScreenElement->RenderImmediate(pkRenderer);
	}

	pkRenderer->SetCameraData(pkCamera);
}
void	PgFxStudio::Terminate()
{
	m_kFxManager.SetCamera(NULL);
	m_kFxManager.SetAssetManager(NULL);
	m_kFxManager.SetSoundManager(NULL);
	m_kFxManager.SetFxManager(NULL);
	m_kFxManager.SetScreenElementsRenderArray(NULL);
	m_kScreenElementsCont.RemoveAll();
	if( m_pkFxStudioManager )
	{
		FxStudio::ReleaseManager(m_pkFxStudioManager);
		m_pkFxStudioManager = NULL;
	}

	m_spBankFile = 0;
	m_spAssetManager = 0;
	m_spSoundManager = 0;
	m_spCamera = 0;

	g_FxAllocator.LogReport();
	m_kFxManager.TerminateStatics();
}