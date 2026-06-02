#include "stdafx.h"
#include "PgCameraModeFree.H"
#include "PgInput.H"
#include "NiMain.H"

PgCameraModeFree::PgCameraModeFree(NiCamera *pkCamera):PgICameraMode(pkCamera)
{

	m_kMoveDir = MD_NONE;
	m_fMaxMoveSpeed = 0;
	m_fMaxMoveSpeedReal = 300;
	m_fMoveSpeed = 0;
	m_fMoveSpeedAccelation = 100;

	m_kRotateAxis = RA_NONE;

	m_kLastMoveDir = NiPoint3(0,0,0);

	m_fRotateAngle[0]=m_fRotateAngle[1] = 0;

	m_fRotateSpeed = 1.0f;

	m_bRMouseDown = false;

	SetFOV((float)61.9);

}
PgCameraModeFree::~PgCameraModeFree()
{
}
void	PgCameraModeFree::SetFOV(float fFOV)	//	fFOV is In Degree
{
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	assert(pkRenderer);

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	float	fAspect = uiWidth/(float)uiHeight;

	m_fFOV = fFOV;

    NiFrustum kNewFrustum;
    kNewFrustum = m_pkCamera->GetViewFrustum();

	kNewFrustum.m_fRight = NiSin((m_fFOV / 2.0f) * NI_TWO_PI / 360.0f);
    kNewFrustum.m_fLeft = - kNewFrustum.m_fRight;
    kNewFrustum.m_fTop = kNewFrustum.m_fRight / fAspect;
    kNewFrustum.m_fBottom = -kNewFrustum.m_fTop;
    m_pkCamera->SetViewFrustum(kNewFrustum);

	g_kFrustum = kNewFrustum;
}

bool	PgCameraModeFree::Update(float fFrameTime)
{
	bool	bUpdated = false;
	float	fMoveSpeed;

	if(m_fMoveSpeed!=m_fMaxMoveSpeed && m_fMoveSpeedAccelation !=0)
	{
		if(m_fMoveSpeed<m_fMaxMoveSpeed)
		{
			m_fMoveSpeed+=fabs(m_fMoveSpeedAccelation)*fFrameTime;
			if(m_fMoveSpeed>m_fMaxMoveSpeed)
				m_fMoveSpeed = m_fMaxMoveSpeed;
		}
		else
		{
			m_fMoveSpeed-=fabs(m_fMoveSpeedAccelation)*fFrameTime;
			if(m_fMoveSpeed<m_fMaxMoveSpeed)
				m_fMoveSpeed = m_fMaxMoveSpeed;
		}
	}

	fMoveSpeed = m_fMoveSpeed*fFrameTime;

	NiPoint3	vMove(0,0,0);
	NiPoint3	vCameraRight = m_pkCamera->GetWorldRightVector();
	NiPoint3	vCameraUp = m_pkCamera->GetWorldUpVector();
	NiPoint3	vCameraDir = m_pkCamera->GetWorldDirection();

	if(m_fMoveSpeed>0)
	{
		if(m_kMoveDir&MD_RIGHT)	vMove+=vCameraRight;
		if(m_kMoveDir&MD_LEFT)	vMove+=-vCameraRight;
		if(m_kMoveDir&MD_UP)	vMove+=NiPoint3(0,0,1);
		if(m_kMoveDir&MD_DOWN)	vMove+=-NiPoint3(0,0,1);
		if(m_kMoveDir&MD_FRONT)	vMove+=vCameraDir;
		if(m_kMoveDir&MD_BACK)	vMove+=-vCameraDir;
		if(m_kMoveDir == MD_NONE) 
		{
			vMove = m_kLastMoveDir;
		}
		else
			m_kLastMoveDir = vMove;

		vMove = vMove*fMoveSpeed;
		m_pkCamera->SetTranslate(m_pkCamera->GetTranslate()+vMove);

		bUpdated = true;
	}

	NiMatrix3	kRotationMatrix;
	NiQuaternion kQuat;
	if(m_kRotateAxis&RA_X && fabs(m_fRotateAngle[0]) >0 )
	{
		kQuat.FromAngleAxis(m_fRotateAngle[0]*m_fRotateSpeed,-NiPoint3(0,0,1));
		kQuat.ToRotation(kRotationMatrix);

		NiPoint3 kNewCamDir = kRotationMatrix * vCameraDir;
		NiPoint3 kNewCamUp = kRotationMatrix * vCameraUp;
		kNewCamDir.Unitize();
		kNewCamUp.Unitize();
		NiPoint3	kNewCamRight = kNewCamDir.Cross(kNewCamUp);
		kNewCamRight.Unitize();

		NiMatrix3 kCamRot = NiMatrix3(kNewCamDir, kNewCamUp, kNewCamRight);
		m_pkCamera->SetRotate(kCamRot);

		vCameraRight= kNewCamRight;
		vCameraDir = kNewCamDir;
		vCameraUp = kNewCamUp;

		m_fRotateAngle[0] = m_fRotateAngle[0]/2.0f;

		if(fabs(m_fRotateAngle[0])<0.01) m_fRotateAngle[0] = 0;

		bUpdated = true;
	}
	if(m_kRotateAxis&RA_Y && fabs(m_fRotateAngle[1])>0)
	{
		kQuat.FromAngleAxis(m_fRotateAngle[1]*m_fRotateSpeed,-vCameraRight);
		kQuat.ToRotation(kRotationMatrix);

		NiPoint3 kNewCamDir = kRotationMatrix * vCameraDir;
		kNewCamDir.Unitize();
		NiPoint3	kNewCamUp = vCameraRight.Cross(kNewCamDir);
		kNewCamUp.Unitize();

		NiMatrix3 kCamRot = NiMatrix3(kNewCamDir, kNewCamUp, vCameraRight);
		m_pkCamera->SetRotate(kCamRot);

		m_fRotateAngle[1] = m_fRotateAngle[1]/2.0f;

		if(fabs(m_fRotateAngle[1])<0.01) m_fRotateAngle[1] = 0;
		bUpdated = true;
	}

	if(m_fRotateAngle[0] == 0 && m_fRotateAngle[1] == 0) 	m_kRotateAxis = RA_NONE;

	return	bUpdated;
}
bool	PgCameraModeFree::Input(PgInput *pkInput)
{
	if(!pkInput) return false;

	if(pkInput->Data()->GetDeviceID() != 1)
	{
		int const value = pkInput->Data()->GetDataValue();

		int xui_value = value;
		XUI::E_INPUT_EVENT_INDEX xui_event_type = XUI::IEI_NONE;

		POINT3I pt3Pos;

		unsigned int uiAppData = pkInput->Data()->GetAppData();

		NiPoint3	vCameraRight = m_pkCamera->GetWorldRightVector();
		NiPoint3	vCameraUp = m_pkCamera->GetWorldUpVector();
		NiPoint3	vCameraDir = m_pkCamera->GetWorldDirection();

		switch(uiAppData)
		{
			case NiAction::MOUSE_AXIS_X:
				{

					float	fDeltaValue = value - m_ptPrevMousePos.x;
					if(m_ptPrevMousePos.x == 0)
					{
						fDeltaValue = 0;
					}
					if(g_bUseDirectInput)
					{
						fDeltaValue = value;
					}

					m_ptPrevMousePos.x = value;

					float	fRotateAngle = (float)(fDeltaValue*3.141592/180.0f);

					if(m_bRMouseDown)
						Rotate(RA_X,fRotateAngle);
				}
				return	true;
			case NiAction::MOUSE_AXIS_Y:
				{

					float	fDeltaValue = value - m_ptPrevMousePos.y;
					if(m_ptPrevMousePos.y == 0)
					{
						fDeltaValue = 0;
					}
					if(g_bUseDirectInput)
					{
						fDeltaValue = value;
					}

					m_ptPrevMousePos.y = value;

					float	fRotateAngle = (float)(fDeltaValue*3.141592/180.0f);

					if(m_bRMouseDown)
						Rotate(RA_Y,fRotateAngle);
				}
				return	true;
			case NiAction::MOUSE_AXIS_Z:
				break;
			case NiAction::MOUSE_BUTTON_LEFT:
				break;
			case NiAction::MOUSE_BUTTON_RIGHT:
				m_bRMouseDown = (value != 0);
				return	true;
			case NiAction::MOUSE_BUTTON_MIDDLE:
				break;
		};
		
	}
	else
	{
		switch(pkInput->GetUKey()-1000)
		{
		case NiInputKeyboard::KEY_NUMPAD8:
			if(pkInput->GetPressed())
				MoveToDir(MD_FRONT);
			else
				Stop(MD_FRONT);
			return	true;
		case NiInputKeyboard::KEY_NUMPAD2:
			if(pkInput->GetPressed())
				MoveToDir(MD_BACK);
			else
				Stop(MD_BACK);
			return	true;
		case NiInputKeyboard::KEY_NUMPAD4:
			if(pkInput->GetPressed())
				MoveToDir(MD_LEFT);
			else
				Stop(MD_LEFT);
			return	true;
		case NiInputKeyboard::KEY_NUMPAD6:
			if(pkInput->GetPressed())
				MoveToDir(MD_RIGHT);
			else
				Stop(MD_RIGHT);
			return	true;
		case NiInputKeyboard::KEY_NUMPAD9:
			if(pkInput->GetPressed())
				MoveToDir(MD_UP);
			else
				Stop(MD_UP);
			return	true;
		case NiInputKeyboard::KEY_NUMPAD3:
			if(pkInput->GetPressed())
				MoveToDir(MD_DOWN);
			else
				Stop(MD_DOWN);
			return	true;
		};
	}

	return	false;
}
void	PgCameraModeFree::Rotate(RotateAxis kAxis,float fAngle)
{
	m_kRotateAxis |= (int)kAxis;
	switch(kAxis)
	{
	case RA_X:	m_fRotateAngle[0]	=	fAngle;	break;
	case RA_Y:	m_fRotateAngle[1]	=	fAngle;	break;
	}
}
void	PgCameraModeFree::SetRotateSpeed(float fSpeed)
{
	m_fRotateSpeed = fSpeed;
}
void	PgCameraModeFree::SetMoveSpeed(float fSpeed)
{
	m_fMaxMoveSpeedReal = fSpeed;
}
void	PgCameraModeFree::SetMoveAccelSpeed(float fSpeed)
{
	m_fMoveSpeedAccelation =fSpeed;
}

void	PgCameraModeFree::MoveToDir(MoveDir kDir)
{
	m_kMoveDir |= kDir;
	m_fMaxMoveSpeed = m_fMaxMoveSpeedReal;
}
void	PgCameraModeFree::Stop(MoveDir kDir)
{
	m_kMoveDir &= ~kDir;
	if(m_kMoveDir == MD_NONE)	m_fMaxMoveSpeed = 0;
}

