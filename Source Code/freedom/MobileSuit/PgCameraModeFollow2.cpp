#include "stdafx.h"
#include "PgCameraModeFollow2.H"
#include "PgInterpolator.h"
#include "PgDamper.h"
#include "PgWorld.h"
#include "PgActor.h"
#include "PgPilotMan.h"


PgCameraModeFollow2::PgCameraModeFollow2(NiCamera *pkCamera,PgActor *pkActor) : PgICameraMode(pkCamera)
{
	m_pkActor = pkActor;
}
PgCameraModeFollow2::~PgCameraModeFollow2()
{
	m_pkActor = NULL;
}

bool PgCameraModeFollow2::Update(float fFrameTime)
{
	return	false;
}

bool	PgCameraModeFollow2::Input(PgInput *pkInput)	//	return  true : 내부에서 인풋을 사용했음. false : 인풋을 사용하지 않았음.
{
	return	false;
}