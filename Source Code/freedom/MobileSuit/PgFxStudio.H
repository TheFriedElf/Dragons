#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXSTUDIO_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXSTUDIO_H

#ifdef SetBit
#undef SetBit
#endif

#include <NiMain.H>
#include "CreateUsingNiNew.inl"
#include "FxStudioRT.h"
#include "FxReference.H"
#include "PgFxBankFile.H"

class	PgFxStudio	:	public	NiObject
{
	friend	struct CreateUsingNiNew<PgFxStudio>;

protected:

	PgFxStudio();
	virtual	~PgFxStudio();

public:

	bool	Initialize();
	void	Terminate();

public:

	void	Update(float fFrameTime);
	void	DrawScreenElements(NiRenderer *pkRenderer,NiCamera *pkCamera);
public:	

	FxStudio::Manager*	GetFxStudioManager()	{	return	m_pkFxStudioManager;	}
	FxManager&	GetFxManager()	{	return	m_kFxManager;	}

	bool	LoadBank();

public:

	NiScreenElementsArray	m_kScreenElementsCont;
	NiCameraPtr	m_spCamera;
	PgFxBankFilePtr	m_spBankFile;

	FxManager	m_kFxManager;
	FxStudio::Manager	*m_pkFxStudioManager;

	FxAssetManagerPtr	m_spAssetManager;
	FxSoundManagerPtr	m_spSoundManager;
	
};

#define g_kFxStudio SINGLETON_CUSTOM(PgFxStudio,CreateUsingNiNew)
#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXSTUDIO_H