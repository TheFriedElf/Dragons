#ifndef FREEDOM_DRAGONICA_RENDER_EFFECT_PGSKY_H
#define FREEDOM_DRAGONICA_RENDER_EFFECT_PGSKY_H

#include "NiMain.h"

class	PgRenderer;

#define MAX_SKY_CLOUD_TEX	1

class	PgSky	:	public	NiMemObject
{


	NiSourceTexturePtr	m_spCloudTex[MAX_SKY_CLOUD_TEX];

	NiNodePtr	m_spSkyDome;
	NiGeometry*	m_pkSkyDomeGeom;

	float	m_fMovement;
public:

	PgSky()	{	Init();	}
	virtual	~PgSky()	{	Destroy();	}

	void	DrawImmediate(NiCameraPtr	spCamera,PgRenderer *pkRenderer,float fFrameTime);

private:

	void	Init();
	void	Destroy();

	void	Load_Res();
};

#endif // FREEDOM_DRAGONICA_RENDER_EFFECT_PGSKY_H