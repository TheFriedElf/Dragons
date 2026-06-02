#include "stdafx.h"
#include "PgInput.h"

PgInput::PgInput()
{
	m_uiUKey = 0;
	m_pkPacket = 0;
	//m_kActionLoc = NiPoint3::ZERO;
}

bool PgInput::IsLocal() const
{
	if(GetUKey() > PgInput::UR_LOCAL_BEGIN
	&& GetUKey() < PgInput::UR_REMOTE_BEGIN)
	{
		return true;
	}
	return false;
}

bool PgInput::IsRemote() const
{
	if(GetUKey() > UR_REMOTE_BEGIN
	&& GetUKey() < UR_TEMP_BEGIN)
	{
		return true;
	}
	return false;
}

bool PgInput::IsExtendedSlot() const
{
	if(GetUKey() > UR_EXTENDED_BEGIN
		&& GetUKey() < UR_REMOTE_BEGIN)
	{
		return true;		
	}
	return false;
}


void PgInput::SetPilotGuid(BM::GUID const &kGuid)
{
	m_kPilotGuid = kGuid;
}

BM::GUID const &PgInput::GetPilotGuid()
{
	return m_kPilotGuid;
}

void PgInput::SetPressed(bool bPressed)
{
	m_bPressed = bPressed;
}

bool PgInput::GetPressed()
{
	return m_bPressed;
}

unsigned int PgInput::GetUKey()const
{
	return m_uiUKey;
}

void PgInput::SetUKey(unsigned int uiUKey)
{
	m_uiUKey = uiUKey;
}

BM::Stream *PgInput::GetPacket()
{
	return m_pkPacket;
}
	
void PgInput::SetPacket(BM::Stream *pkPacket)
{
	m_pkPacket = pkPacket;
}

//NiPoint3 &PgInput::GetActionLoc()
//{
//	return m_kActionLoc;
//}
//
//void PgInput::SetActionLoc(NiPoint3 &kLoc)
//{
//	m_kActionLoc = kLoc;
//}