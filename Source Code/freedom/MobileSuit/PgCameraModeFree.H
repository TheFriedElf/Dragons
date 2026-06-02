#ifndef FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEFREE_H
#define FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEFREE_H
#include "PgICameraMode.H"

class	PgInput;
class PgCameraModeFree : public PgICameraMode
{
	enum	MoveDir
	{
		MD_NONE=0,
		MD_RIGHT=(1<<0),
		MD_LEFT=(1<<1),
		MD_UP=(1<<2),
		MD_DOWN=(1<<3),
		MD_FRONT=(1<<4),
		MD_BACK=(1<<5)
	};
	enum	RotateAxis
	{
		RA_NONE=0,
		RA_X=(1<<0),
		RA_Y=(1<<1),
		RA_Z=(1<<2)
	};

	float	m_fRotateSpeed;
	float	m_fRotateAngle[3];
	int	m_kRotateAxis;

	float	m_fMaxMoveSpeed,m_fMaxMoveSpeedReal;
	float	m_fMoveSpeed;
	float	m_fMoveSpeedAccelation;

	int	m_kMoveDir;

	NiPoint3	m_kLastMoveDir;

	bool	m_bRMouseDown;

	float	m_fFOV;	//	In Degree

	NiPoint2	m_ptPrevMousePos;	

public:

	PgCameraModeFree(NiCamera *pkCamera);
	virtual	~PgCameraModeFree();

	bool Update(float fFrameTime);

public:

	bool	Input(PgInput *pkInput);	//	return  true : 내부에서 인풋을 사용했음. false : 인풋을 사용하지 않았음.

public:

	void	SetRotateSpeed(float fSpeed);
	void	SetMoveSpeed(float fSpeed);
	void	SetMoveAccelSpeed(float fSpeed);
	void	SetFOV(float fFOV);	//	fFOV is In Degree

private:

	void	MoveToDir(MoveDir kDir);
	void	Rotate(RotateAxis kAxis,float fAngle);
	void	Stop(MoveDir kDir);
};
#endif // FREEDOM_DRAGONICA_RENDER_CAMERA_PGCAMERAMODEFREE_H