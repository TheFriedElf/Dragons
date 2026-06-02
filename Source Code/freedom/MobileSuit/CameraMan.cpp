#include "StdAfx.h"
#include "CameraMan.h"

CCameraMan::CCameraMan(void)
{
}

CCameraMan::~CCameraMan(void)
{
}

NiCameraPtr CCameraMan::GetCamera()
{
	return m_spCamera;
}

void CCameraMan::SetCamera(NiCameraPtr spCamera)
{
	m_spCamera = spCamera;
}

void CCameraMan::SetCamera(NiString& rkCameraName)
{
	NiCameraPtr spCamera;

	if(m_kContainer.GetAt(rkCameraName, spCamera))
	{
		m_spCamera = spCamera;
	}
}

void CCameraMan::AddCamera(NiString& rkCameraName, NiCameraPtr spCamera)
{
	m_kContainer.SetAt(rkCameraName, spCamera);
}

void CCameraMan::DeleteCamera(NiString& rkCameraName)
{
	m_kContainer.RemoveAt(rkCameraName);
}

void CCameraMan::Update(float fTime)
{
	// use camera mode strategy updating
}
