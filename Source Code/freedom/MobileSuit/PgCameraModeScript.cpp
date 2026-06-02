#include "stdafx.h"
#include "PgCameraModeScript.H"

PgCameraModeScript::PgCameraModeScript(NiCamera *pkCamera):PgICameraMode(pkCamera)
{
}

//! 파괴자
PgCameraModeScript::~PgCameraModeScript(void)
{
}
//! 카메라의 위치를 갱신한다.
bool PgCameraModeScript::Update(float fFrameTime)
{
	lua_tinker::call<void>(m_kScriptName.c_str());
	return	true;
}

//!	카메라를 컨트롤할 루아 스크립트를 설정한다.
void	PgCameraModeScript::SetControlScript(std::string const kScriptName)
{
	m_kScriptName = kScriptName;
}