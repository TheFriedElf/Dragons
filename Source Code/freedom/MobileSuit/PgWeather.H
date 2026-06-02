#ifndef FREEDOM_DRAGONICA_RENDER_EFFECT_PGWEATHER_H
#define FREEDOM_DRAGONICA_RENDER_EFFECT_PGWEATHER_H
#include "NiMain.h"
class PgRenderer;
class PgWorld;

struct	stRainDropPointInfo
{
	NiPoint3	m_ptVertex,m_ptNormal;
};

class PgRainDropMan;
class	PgRainDropArea
{

	int	m_iTotalPointInfo;
	stRainDropPointInfo	*m_paPointInfos;

	NiPoint3	m_ptBBox[2];	//	0:Min 1:Max

	PgRainDropArea	*m_paChild;

public:

	PgRainDropArea()	{	Init();	}
	~PgRainDropArea()	{	Destroy();	}


	void	SetPointInfo(int iTotalPointInfo,stRainDropPointInfo *paPointInfos,NiPoint3 const &bBoxMin,NiPoint3 const &bBoxMax);
	void	Draw(PgRenderer *pkRenderer,PgWorld *pkWorld,PgRainDropMan *pMan);

private:

	void	Init();
	void	Destroy();
};
class	PgRainDropMan
{
	PgRainDropArea	*m_pTopNode;

	NiSourceTexturePtr	m_spTex;
	NiAVObjectPtr	m_spNiNode;

	int	m_iFrameCount;

	struct	stNode
	{
		unsigned	long	m_ulStartTime;
		stRainDropPointInfo	*m_pPoint;
	};
	typedef std::list<stNode*> NodeList;

	NodeList	m_NodeList;
	
public:

	PgRainDropMan()
	{
		Init();
	}
	~PgRainDropMan()
	{
		Destroy();
	}

	void	LoadFromFile(std::string const &kPath);

	void	Draw(PgRenderer *pkRenderer,PgWorld *pkWorld);

	void	Add_Rain_Drop(stRainDropPointInfo *pDropPoint);

private:

	void	Init();
	void	Destroy();

};

class	PgRain		
{
	NiNodePtr	m_spRainNif;
	float	m_fRainRotate;
	float	m_fFrameTime;

	PgRainDropMan	*m_pRainDropMan;

	NiScreenTexturePtr	m_spScreenTexture;
public:

	PgRain()	{	Init();	}
	~PgRain()	{	Destroy();	}

	void	Update(float fAccumTime, float fFrameTime);
	void	DrawRainDrops(PgRenderer *pkRenderer, PgWorld *pkWorld,float fFrameTime);
	void	DrawImmediate(PgRenderer *pkRenderer, PgWorld *pkWorld,float fFrameTime);

private:

	void	Init();
	void	Destroy();

	void	LoadResource();


};

#endif // FREEDOM_DRAGONICA_RENDER_EFFECT_PGWEATHER_H