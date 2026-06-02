#pragma once

#include <NiFont.h>
#include "ResourcePool.h"
#include "CameraMan.h"

#define MAX_SPAWN_LOCS		10	// Maximum Spawnning Locations

class CWorldMan : public NiMemObject
{
public:
	//// Constructor & Destructor
	//
	CWorldMan();
	~CWorldMan();

	//// Instancing
	//
	static CWorldMan *Create();
	static void Destroy();

	//// Initialization
	//
	bool Initialize();

	//// Load
	//
	bool Load(const char *pcWorldName);
	void RecursivePrepack(NiAVObject* pkObject);

	//// Intervals
	//
    void Update(float fTime);
	void Draw(float fTime);

	//// Camera
	//
	void SetTurretControl();

	//// Get
	//
	NiNodePtr GetSkyRoot()
	{
		return m_spSkyRoot;
	}

	NiNodePtr GetPathRoot()
	{
		return m_spPathRoot;
	}
	
	NiNodePtr GetSpawnRoot()
	{
		return m_spSpawnRoot;
	}

	NiNodePtr GetSceneRoot()
	{
		return m_spSceneRoot;
	}

protected:
	//// Root Nodes
	//
	NiNodePtr m_spSceneRoot;
	NiNodePtr m_spPathRoot;
	NiNodePtr m_spSpawnRoot;
	NiNodePtr m_spSkyRoot;

	//// Camera
	//
	CCameraMan m_kCameraMan;
	NiTurret m_kTurret;
	NiNodePtr m_spTrnNode;
	NiNodePtr m_spRotNode;

	//// Culling
	//
	NiVisibleArray m_kVisibleSky;
	NiVisibleArray m_kVisibleScene;
	NiAlphaAccumulatorPtr m_spAlphaSorter;
	NiCullingProcess* m_pkCuller;

	//// Font
	//
	static CResourcePool<NiString, NiFontPtr> ms_kFontPool;
	static NiString2DPtr ms_spStrFrameRate;
};