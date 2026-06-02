#pragma once


class ShaderHelper;
class CWorldMan;

class CMobileSuit : public NiApplication
{
public:
	//// Constructor & Destructor
	//
	CMobileSuit();
	~CMobileSuit();

	//// Overrides NiApplication
	//
	virtual bool Initialize();
	virtual void Terminate();
	virtual void OnIdle();
	virtual bool CreateRenderer();
	virtual bool CreateScene();

protected:
	//// Execution Parameters
	//
	bool m_bDebugMode;

	//// World
	//
	CWorldMan *m_pkWorldMan;

	//// Turret, Camera Controller
	//
	NiTurret m_kTurret;
	NiNodePtr m_spTrnNode;
	NiNodePtr m_spRotNode;

	//// Actor
	//
	NiActorManagerPtr m_spActorMgr;


	//// Roots
	//
	NiNodePtr m_spPathsRoot;
	NiNodePtr m_spCamerasRoot;
	NiNodePtr m_spBlocksRoot;

	//// Shader
	//
	ShaderHelper *m_pkShaderHelper;
};

extern NiApplication *g_pkApp;