#include "StdAfx.h"
#include "PgWorld.h"
#include "lwDropBox.h"
#include "PgDropBox.h"
#include "PgXmlLoader.h"
#include "lwPilot.h"

using namespace lua_tinker;

lwDropBox lwNewDropBox(char const *pcDropBoxID)
{
	PgDropBox *pkDropBox = dynamic_cast<PgDropBox *>(PgXmlLoader::CreateObject(pcDropBoxID));
	return lwDropBox(dynamic_cast<PgDropBox *>(pkDropBox));
}

lwDropBox lwNewDropBoxFromFile(char const *pcFileName)
{
	PgDropBox *pkDropBox = dynamic_cast<PgDropBox *>(PgXmlLoader::CreateObjectFromFile(pcFileName));
	return lwDropBox(dynamic_cast<PgDropBox *>(pkDropBox));
}

lwDropBox lwToDropBox(PgIWorldObject *pkObject)
{
	return (lwDropBox)(dynamic_cast<PgDropBox *>(pkObject));
}

lwDropBox lwPilotToDropBox(lwPilot *pkPilot)
{
	PgPilot* pkRealPilot = (*pkPilot)();
	
	return (lwDropBox)(dynamic_cast<PgDropBox *>(pkRealPilot->GetWorldObject()));
}


lwDropBox::lwDropBox(PgDropBox *pkDropBox)
{
	m_pkDropBox = pkDropBox;
}

bool lwDropBox::RegisterWrapper(lua_State *pkState)
{
	def(pkState, "NewDropBox", &lwNewDropBox);
	def(pkState, "NewDropBoxFromFile", &lwNewDropBoxFromFile);
	def(pkState, "ToDropBox", &lwToDropBox);
	def(pkState, "PilotToDropBox", &lwPilotToDropBox);

	class_<lwDropBox>(pkState, "DropBox")
		.def(pkState, constructor<PgDropBox *>())
		.def(pkState, "SetGuid", &lwDropBox::SetGuid)
		.def(pkState, "GetGuid", &lwDropBox::GetGuid)
		.def(pkState, "SetOwnerGuid", &lwDropBox::SetOwnerGuid)
		.def(pkState, "GetOwnerGuid", &lwDropBox::GetOwnerGuid)
		.def(pkState, "IsNil", &lwDropBox::IsNil)
		.def(pkState, "AddItems", &lwDropBox::AddItems)
		.def(pkState, "TransitAction", &lwDropBox::TransitAction)
		.def(pkState, "ItemCount", &lwDropBox::ItemCount)
		.def(pkState, "SetNameColor", &lwDropBox::SetNameColor)
		.def(pkState, "SetItemNum", &lwDropBox::SetItemNum)
		.def(pkState, "IsMine", &lwDropBox::IsMine)
		.def(pkState, "SetOfferer", &lwDropBox::SetOfferer)
		.def(pkState, "IsMoney", &lwDropBox::IsMoney)
		.def(pkState, "GetAbil", &lwDropBox::GetAbil)
		.def(pkState, "GetPos", &lwDropBox::GetPos)
		;

	return true;
}

PgDropBox *lwDropBox::operator()()
{
	return m_pkDropBox;
}
lwPoint3 lwDropBox::GetPos()
{
	return	lwPoint3(m_pkDropBox->GetWorldTranslate());
}

void lwDropBox::SetOwnerGuid(lwGUID kGuid)
{
	m_pkDropBox->SetOwnerGuid(kGuid());
}

lwGUID lwDropBox::GetOwnerGuid()
{
	return lwGUID(m_pkDropBox->GetOwnerGuid());
}

void lwDropBox::SetGuid(lwGUID kGuid)
{
	m_pkDropBox->SetGuid(kGuid());
}

lwGUID lwDropBox::GetGuid()
{
	return lwGUID(m_pkDropBox->GetGuid());
}

bool lwDropBox::IsNil()
{
	return m_pkDropBox == 0;
}

bool lwDropBox::AddItems(lwPacket kPacket)
{
	BM::Stream *pkPacket = kPacket();
	
	char cCount;
	pkPacket->Pop(cCount);

	NILOG(PGLOG_LOG, "[lwDropBox] AddItems, %d item added\n", cCount);
		
	for (char iItemIdx=0; iItemIdx < cCount; ++iItemIdx)
	{
		short sBoxPos;
		pkPacket->Pop(sBoxPos);			// Box Position

		PgBase_Item kItem;
		kItem.ReadFromPacket( *pkPacket );

		// Add Item to DropBox!
		m_pkDropBox->AddItem(kItem);
	}

	return true;
}

bool lwDropBox::TransitAction(char const *pcActionName)
{
	return m_pkDropBox->TransitAction(pcActionName);
}

int lwDropBox::ItemCount()
{
	return m_pkDropBox->ItemCount();
}

void lwDropBox::SetNameColor(int iColor)
{
	m_pkDropBox->SetNameColor((DWORD)iColor);
}

void lwDropBox::SetItemNum(int iNum)
{
	m_pkDropBox->SetItemNum(iNum);
}

bool lwDropBox::IsMine()
{
	if( !m_pkDropBox )
	{
		return false;
	}

	PgPilot* pkPilot = m_pkDropBox->GetPilot();
	if( !pkPilot )
	{
		return false;
	}

	PgGroundItemBox* pkItemBox = dynamic_cast<PgGroundItemBox*>(pkPilot->GetUnit());
	if( g_pkWorld )
	{
		return g_pkWorld->IsMineItemBox(pkItemBox);
	}
	return false;
}

bool lwDropBox::IsMoney()
{
	return m_pkDropBox->IsMoney();
}

int lwDropBox::GetAbil(WORD const wAbil)
{
	return m_pkDropBox->GetAbil(wAbil);
}

bool lwDropBox::SetOfferer(lwGUID kGuid, lwPoint3 kPos)
{
	if( !m_pkDropBox )
	{
		return false;
	}

	return m_pkDropBox->SetOfferer(kGuid(), NiPoint3(kPos.GetX(), kPos.GetY(), kPos.GetZ()));
}