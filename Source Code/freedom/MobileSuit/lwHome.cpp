#include "StdAfx.h"
#include "lwHome.h"
#include "PgHome.h"
#include "PgPilotMan.h"

#include "Pghouse.h"
#include "PgPilot.h"

using namespace lua_tinker;

lwHome::lwHome(PgHome *pkHome)
{
	m_pkHome = pkHome;
}

bool lwHome::RegisterWrapper(lua_State *pkState)
{
	class_<lwHome>(pkState, "Home")
		.def(pkState, constructor<PgHome *>())
		.def(pkState, "IsNil", &lwHome::IsNil)
		.def(pkState, "AddWall", &lwHome::AddWall)
		.def(pkState, "RemoveWall", &lwHome::RemoveWall)
		.def(pkState, "AddFurniture", &lwHome::AddFurniture)
		.def(pkState, "RemoveAllFurniture", &lwHome::RemoveAllFurniture)
		.def(pkState, "SetArrangeMode", &lwHome::SetArrangeMode)		
		.def(pkState, "IsArrangeMode", &lwHome::IsArrangeMode)
		.def(pkState, "SetArrangingFurniture", &lwHome::SetArrangingFurniture)
		.def(pkState, "GetArrangingFurniture", &lwHome::GetArrangingFurniture)
		.def(pkState, "GetCenterPos", &lwHome::GetCenterPos)
		.def(pkState, "GetOwnerGuid", &lwHome::GetOwnerGuid)
		//.def(pkState, "SetOwnerGuid", &lwHome::SetOwnerGuid)
		.def(pkState, "GetHomeGuid", &lwHome::GetHomeGuid)
		.def(pkState, "IsMyHome", &lwHome::IsMyHome)
		.def(pkState, "SetRoomSize", &lwHome::SetRoomSize)
		.def(pkState, "SetMovingFurniture", &lwHome::SetMovingFurniture)
		.def(pkState, "RemoveFurniture", &lwHome::RemoveFurniture)
		.def(pkState, "IsHavePostbox", &lwHome::IsHavePostbox)
		.def(pkState, "IsHaveSafebox", &lwHome::IsHaveSafebox)
		.def(pkState, "RemoveAllFurnitureOnlyClient", &lwHome::RemoveAllFurnitureOnlyClient)
		;

	return true;
}

bool lwHome::IsNil()
{
	return (m_pkHome == 0 ? true : false);
}

PgHome *lwHome::operator()()
{
	return m_pkHome;
}

void lwHome::AddWall(lwPoint3 kPos, int iRotate)
{
	if (m_pkHome)
	{
		m_pkHome->AddWall(kPos(), iRotate);
	}
}

void lwHome::RemoveWall()
{
	if (m_pkHome)
	{
		m_pkHome->RemoveWall();
	}
}


lwFurniture lwHome::AddFurniture(int iFurnitureNo, lwPoint3 kPos, int iRotate)
{
	if (m_pkHome)
	{
		return lwFurniture(m_pkHome->AddFurniture(iFurnitureNo, kPos(), iRotate));
	}
	return lwFurniture(NULL);
}

void lwHome::RemoveAllFurniture()
{
	if (m_pkHome)
	{
		m_pkHome->UnsetAllFurniture();
	}
}

void lwHome::RemoveAllFurnitureOnlyClient()
{
	if (m_pkHome)
	{
		m_pkHome->RemoveAllFurniture();
	}
}

void lwHome::SetArrangeMode(bool bArrangeMode)
{
	if (m_pkHome)
		m_pkHome->SetArrangeMode(bArrangeMode);
}

bool lwHome::IsArrangeMode()
{
	if (m_pkHome)
		return m_pkHome->IsArrangeMode();

	return false;
}

void lwHome::SetArrangingFurniture(lwFurniture kFurniture)
{
	if (m_pkHome)
		m_pkHome->SetArrangingFurniture(kFurniture());
}

lwFurniture lwHome::GetArrangingFurniture()
{
	if (m_pkHome)
		return (lwFurniture)m_pkHome->GetArrangingFurniture();

	return lwFurniture(NULL);
}

lwPoint3 lwHome::GetCenterPos()
{
	if (m_pkHome)
		return lwPoint3(m_pkHome->GetCenterPos());

	return lwPoint3(0, 0, 0);
}

lwGUID lwHome::GetOwnerGuid()
{
	if (m_pkHome)
		return lwGUID(m_pkHome->GetOwnerGuid());

	return lwGUID(BM::GUID::NullData());
}

//void lwHome::SetOwnerGuid(lwGUID guid)
//{
//	if (m_pkHome)
//		m_pkHome->SetOwnerGuid(guid());
//}

lwGUID lwHome::GetHomeGuid()
{
	if( m_pkHome && m_pkHome->GetHomeUnit() )
	{
		return lwGUID(m_pkHome->GetHomeUnit()->GetID());
	}

	return lwGUID(BM::GUID::NullData());
}

bool lwHome::IsMyHome()
{
	if (m_pkHome)
	{
		return m_pkHome->IsMyHome();
	}

	return false;
}

bool lwHome::SetRoomSize(int iX, int iY, int iZ)
{
	if(!m_pkHome)
	{
		return false;
	}

	m_pkHome->SetRoomSize(POINT3I(iX, iY, iZ));

	return true;
}

void lwHome::SetMovingFurniture()
{
	if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_MYHOME))
	{
		if (m_pkHome)
		{
			if( !m_pkHome->IsUseAbleTimeofPickFurniture() )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 201511, true);
				return;
			}
			m_pkHome->SetArrangeMode(true);
			m_pkHome->SelectArrangingFurnitureByMenu();
			XUIMgr.Call(L"FRM_MYHOME_EDIT_MODE");
		}
	}
}

void lwHome::RemoveFurniture()
{
	if(g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_MYHOME))
	{
		if( !m_pkHome )
		{
			return;
		}

		m_pkHome->RemovePickedFurniture();
	}
}

bool lwHome::IsHavePostbox()
{
	if(m_pkHome)
	{
		return m_pkHome->IsHavePostbox();
	}

	return false;
}

bool lwHome::IsHaveSafebox()
{
	if(m_pkHome)
	{
		return m_pkHome->IsHaveSafebox();
	}

	return false;
}
