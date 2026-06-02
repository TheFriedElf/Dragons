#include "stdafx.h"
#include "lwLinkedPlane.H"
#include "lwPoint3.H"

using namespace lua_tinker;


///////////////////////////////////////////////////////////////////////////////
//	class	lwLinkedPlane
///////////////////////////////////////////////////////////////////////////////

void lwLinkedPlane::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(LinkedPlane)
		LW_REG_METHOD(LinkedPlane, GetInstanceID)
		LW_REG_METHOD(LinkedPlane, SetStartPos)
		LW_REG_METHOD(LinkedPlane, SetEndPos)
		LW_REG_METHOD(LinkedPlane, SetWidth)
		LW_REG_METHOD(LinkedPlane, GetWidth)
		LW_REG_METHOD(LinkedPlane, SetEnable)
		LW_REG_METHOD(LinkedPlane, GetEnable)
		;
}

int	lwLinkedPlane::GetInstanceID()
{
	if(m_pkLinkedPlane)
		return	m_pkLinkedPlane->GetInstanceID();

	return 0;
}
void lwLinkedPlane::SetStartPos(lwPoint3 kPos)
{
	if(m_pkLinkedPlane)
		m_pkLinkedPlane->SetStartPos(kPos());
}
void lwLinkedPlane::SetEndPos(lwPoint3 kPos)
{
	if(m_pkLinkedPlane)
		m_pkLinkedPlane->SetEndPos(kPos());
}
void lwLinkedPlane::SetWidth(float fWidth)
{
	if(m_pkLinkedPlane)
		m_pkLinkedPlane->SetWidth(fWidth);
}
float lwLinkedPlane::GetWidth()
{
	if(m_pkLinkedPlane)
		return	m_pkLinkedPlane->GetWidth();

	return 0.0f;
}
void lwLinkedPlane::SetEnable(bool bEnable)
{
	if(m_pkLinkedPlane)
		m_pkLinkedPlane->SetEnable(bEnable);
}
bool lwLinkedPlane::GetEnable()
{
	if(m_pkLinkedPlane)
		return	m_pkLinkedPlane->GetEnable();

	return false;
}

///////////////////////////////////////////////////////////////////////////////
//	class	lwLinkedPlaneGroup
///////////////////////////////////////////////////////////////////////////////

void lwLinkedPlaneGroup::RegisterWrapper(lua_State *pkState)
{
	LW_REG_CLASS(LinkedPlaneGroup)
		LW_REG_METHOD(LinkedPlaneGroup, SetTexture)
		LW_REG_METHOD(LinkedPlaneGroup, AddNewPlane)
		LW_REG_METHOD(LinkedPlaneGroup, GetPlane)
		LW_REG_METHOD(LinkedPlaneGroup, ReleasePlane)
		LW_REG_METHOD(LinkedPlaneGroup, ReleaseAllPlane)
		LW_REG_METHOD(LinkedPlaneGroup, GetInstanceID)
		LW_REG_METHOD(LinkedPlaneGroup, SetTextureCoordinateUpdateDelay)
		LW_REG_METHOD(LinkedPlaneGroup, GetTextureCoordinateUpdateDelay)
		;
}
void lwLinkedPlaneGroup::SetTexture(char const *strPath)
{
	if(m_pkLinkedPlaneGroup)
		m_pkLinkedPlaneGroup->SetTexture(strPath);
}
lwLinkedPlane lwLinkedPlaneGroup::AddNewPlane(int const iAlphaTestRef)
{
	if(m_pkLinkedPlaneGroup)
		return	lwLinkedPlane(m_pkLinkedPlaneGroup->AddNewPlane(iAlphaTestRef));

	return lwLinkedPlane(NULL);
}
lwLinkedPlane	lwLinkedPlaneGroup::GetPlane(int iInstanceID)
{
	if(m_pkLinkedPlaneGroup)
		return	lwLinkedPlane(m_pkLinkedPlaneGroup->GetPlane(iInstanceID));

	return lwLinkedPlane(NULL);
}
void	lwLinkedPlaneGroup::ReleasePlane(int iInstanceID)
{
	if(m_pkLinkedPlaneGroup)
		m_pkLinkedPlaneGroup->ReleasePlane(iInstanceID);
}
void	lwLinkedPlaneGroup::ReleaseAllPlane()
{
	if(m_pkLinkedPlaneGroup)
		m_pkLinkedPlaneGroup->ReleaseAllPlane();
}
int	lwLinkedPlaneGroup::GetInstanceID()
{
	if(m_pkLinkedPlaneGroup)
		return	m_pkLinkedPlaneGroup->GetInstanceID();

	return 0;
}
void lwLinkedPlaneGroup::SetTextureCoordinateUpdateDelay(float fDelay)
{
	if(m_pkLinkedPlaneGroup)
		m_pkLinkedPlaneGroup->SetTextureCoordinateUpdateDelay(fDelay);
}
float lwLinkedPlaneGroup::GetTextureCoordinateUpdateDelay()
{
	if(m_pkLinkedPlaneGroup)
		return	m_pkLinkedPlaneGroup->GetTextureCoordinateUpdateDelay();

	return 0.0f;
}

///////////////////////////////////////////////////////////////////////////////
//	class	lwLinkedPlaneGroupMan
///////////////////////////////////////////////////////////////////////////////
lwLinkedPlaneGroupMan	lwGetLinkedPlaneGroupMan()
{
	return	lwLinkedPlaneGroupMan(&g_kLinkedPlaneGroupMan);
}

void lwLinkedPlaneGroupMan::RegisterWrapper(lua_State *pkState)
{
	def(pkState, "GetLinkedPlaneGroupMan", &lwGetLinkedPlaneGroupMan);

	LW_REG_CLASS(LinkedPlaneGroupMan)
		LW_REG_METHOD(LinkedPlaneGroupMan, CreateNewLinkedPlaneGroup)
		LW_REG_METHOD(LinkedPlaneGroupMan, GetLinkedPlaneGroup)
		LW_REG_METHOD(LinkedPlaneGroupMan, ReleaseLinkedPlaneGroup)
		;
}
lwLinkedPlaneGroup lwLinkedPlaneGroupMan::CreateNewLinkedPlaneGroup()
{
	if(m_pkLinkedPlaneGroupMan)
		return	lwLinkedPlaneGroup(m_pkLinkedPlaneGroupMan->CreateNewLinkedPlaneGroup());

	return lwLinkedPlaneGroup(NULL);
}
lwLinkedPlaneGroup	lwLinkedPlaneGroupMan::GetLinkedPlaneGroup(int iInstanceID)
{
	if(m_pkLinkedPlaneGroupMan)
	return	lwLinkedPlaneGroup(m_pkLinkedPlaneGroupMan->GetLinkedPlaneGroup(iInstanceID));

	return lwLinkedPlaneGroup(NULL);
}
void	lwLinkedPlaneGroupMan::ReleaseLinkedPlaneGroup(int iInstanceID)
{
	if(m_pkLinkedPlaneGroupMan)
		m_pkLinkedPlaneGroupMan->ReleaseLinkedPlaneGroup(iInstanceID);
}