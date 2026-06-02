#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXSOUNDMANAGER_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXSOUNDMANAGER_H

#include "FxReference/FxSoundManager.h"

class	PgFxSoundManager	:	public FxSoundManager
{

public:

	virtual void* PlayGameSound(char const* szSound, const NiPoint3& pos, bool bLoop, NiPoint2* pFallOff, const NiPoint3& vel = NiPoint3(0,0,0));

	// Stops a sound.
	virtual void StopGameSound(void* hSound);

	// Pauses a sound.
	virtual void PauseGameSound(void* hSound, bool bPaused);
};

NiSmartPointer(PgFxSoundManager);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXSOUNDMANAGER_H