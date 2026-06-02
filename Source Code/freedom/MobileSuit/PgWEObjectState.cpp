#include "stdafx.h"
#include "PgWEObjectState.H"
#include "PgError.H"
#include "PgWorld.H"
#include "PgPuppet.H"
#include "PgObject.H"
#include "PgPilotMan.h"

namespace NiAVObjectUtil
{
	NiAVObject* FindLinearByName(NiNode* pkRootNode, char const* szName)
	{
		if( !pkRootNode || !szName )
		{
			return NULL;
		}

		int iChildCount = pkRootNode->GetArrayCount();
		for(int i=0; i<iChildCount; ++i)
		{
			NiAVObject* pkChild = pkRootNode->GetAt(i);
			if( pkChild )
			{
				if( pkChild->GetName().Equals(szName) )
				{
					return pkChild;
				}
			}
		}
		return NULL;
	}

	//
	FindPuppetNode::FindPuppetNode(std::string const& rkObjectName, NiAVObject*& pkFindReturn)
		: m_rkObjectName(rkObjectName), m_pkFindReturn(pkFindReturn)
	{
	}
	FindPuppetNode::FindPuppetNode(FindPuppetNode const& rhs)
		: m_rkObjectName(rhs.m_rkObjectName), m_pkFindReturn(rhs.m_pkFindReturn)
	{
	}
	bool FindPuppetNode::operator() (NiNode* pkRootNode)
	{
		if( !pkRootNode )
		{
			return false;
		}

		m_pkFindReturn = FindLinearByName(pkRootNode, m_rkObjectName.c_str());
		if( !m_pkFindReturn )
		{
			return false;
		}
		return true;
	}
};

PgWEObjectState::PgWEObjectState()
	: PgWEClientObjectState(), m_kObjectType(OT_UNKOWN), m_kObjectGUID(), m_kObjectName()
{
}

PgWEObjectState::PgWEObjectState(PgWEObjectState const& rhs)
	: PgWEClientObjectState(rhs), m_kObjectType(rhs.m_kObjectType), m_kObjectGUID(rhs.m_kObjectGUID), m_kObjectName(rhs.m_kObjectName)
{
}

PgWEObjectState::~PgWEObjectState()
{
}

bool PgWEObjectState::SetStatus(int const iNewState)
{
	return	PgWEClientObjectState::SetStatus(iNewState);
}
void PgWEObjectState::OnStateChange()
{
	if(!g_pkWorld)
	{
		_PgMessageBox(GetClientName(), "g_pkWorld is NULL, [%s]", m_kObjectName.c_str());
		return;
	}

	switch(m_kObjectType)
	{
	case OT_PUPPET:
		{
			NiAVObject *pkPuppetNode = NULL;
			if( !g_pkWorld->RunObjectGroupFunc(OGT_PUPPET, NiAVObjectUtil::FindPuppetNode(m_kObjectName, pkPuppetNode)) )
			{
				_PgMessageBox(GetClientName(), "pkPuppetNode is NULL, [%s]", m_kObjectName.c_str());
				return;
			}

			PgPuppet	*pkPuppet = NiDynamicCast(PgPuppet,pkPuppetNode);
			if( !pkPuppet )
			{
				_PgMessageBox(GetClientName(), "Node[%s] is not Puppet", m_kObjectName.c_str());
				return;
			}

			pkPuppet->SetWorldEventStateID(State(),SyncTime(),false);

		}break;
	case OT_RIDABLE_OBJECT:
		{
			if(!g_pkWorld)
			{
				return;
			}
			NiAVObject	*pkAVObject = g_pkWorld->FindObject(m_kObjectGUID);
			if(!pkAVObject)
			{
				_PgMessageBox(GetClientName(), "Can't find Ridable Node[%s]", MB(m_kObjectGUID.str()));
				return;
			}

			PgObject	*pkObject = NiDynamicCast(PgObject,pkAVObject);
			if(!pkObject)
			{
				_PgMessageBox(GetClientName(), "Node[%s] is not PgObject", MB(m_kObjectGUID.str()));
				return;
			}

			pkObject->SetWorldEventStateID(State(),SyncTime(),false);

		}break;
	case OT_CAMERA:
		{
		}break;
	case OT_MAPOBJECT:
		{
			if(!g_pkWorld)
			{
				return;
			}
			NiNode* pkRootNode = g_pkWorld->GetSceneRoot();
			if( !pkRootNode )
			{
				return;
			}

			NiAVObject* pkObject = pkRootNode->GetObjectByName(m_kObjectName.c_str());
			if( !pkObject )
			{
				_PgMessageBox(GetClientName(), "Can't find Node[%s]", m_kObjectName.c_str());
				return;
			}

			pkObject->SetAppCulled( !Show() );

		}break;
	case OT_NPC:
		{
			if( m_kObjectGUID.IsNull() )
			{
				_PgMessageBox(GetClientName(), "m_kObjectGUID is NULL");
				return;
			}
			PgActor *pActor = g_kPilotMan.FindActor(m_kObjectGUID);
			if( !pActor )
			{
				_PgMessageBox(GetClientName(), "Can't find m_kObjectGUID[%s]", m_kObjectGUID.str() );
				return;
			}
			pActor->SetHide( !Show() );
		}break;
	}
}
bool PgWEObjectState::Parse(TiXmlElement const* pkNode)
{
	bool	bResult = PgWEClientObjectState::Parse(pkNode);
	if(!bResult)
	{
		return	false;
	}

	TiXmlAttribute const* pkAttribute = pkNode->FirstAttribute();
	while( pkAttribute )
	{
		std::string const szAttrName(pkAttribute->Name());
		std::string const szAttrValue(pkAttribute->Value());

		if(szAttrName == "TYPE")
		{
			if(szAttrValue == "PUPPET")
			{
				m_kObjectType = OT_PUPPET;
			}
			else if(szAttrValue == "RIDABLE_OBJECT")
			{
				m_kObjectType = OT_RIDABLE_OBJECT;
			}
			else if(szAttrValue == "CAMERA")
			{
				m_kObjectType = OT_CAMERA;
			}
			else if(szAttrValue == "MAPOBJECT")
			{
				m_kObjectType = OT_MAPOBJECT;
			}
			else if(szAttrValue == "NPC")
			{
				m_kObjectType = OT_NPC;
			}
			else
			{
				PgXmlError1(pkNode, "XmlParse: Unknown World Event Object Type '%s'", szAttrValue.c_str());
				break;
			}
		}
		else if(szAttrName == "GUID")
		{
			m_kObjectGUID = BM::GUID(szAttrValue);
		}
		else if(szAttrName == "NAME")
		{
			m_kObjectName = szAttrValue;
		}
		//else
		//{
		//	PgXmlError1(pkNode, "XmlParse: Unknown Attribute '%s'", szAttrName.c_str());
		//	break;
		//}

		pkAttribute = pkAttribute->Next();
	}

	ValidateXmlData();

	return true;

}
void	PgWEObjectState::ValidateXmlData()
{
	if(m_kObjectType == OT_UNKOWN)
	{
		_PgMessageBox("","[WorldEventObject Parse Error] A valid object type must be specified .");
		return;
	}

	if(m_kObjectType == OT_PUPPET)
	{
		if(m_kObjectName.empty())
		{
			_PgMessageBox("","[WorldEventObject Parse Error] An object name must be specified for an PUPPET type object");
			return;
		}
	}

	if(m_kObjectType == OT_RIDABLE_OBJECT)
	{
		if(m_kObjectGUID == BM::GUID::NullData())
		{
			_PgMessageBox("","[WorldEventObject Parse Error] An object GUID must be specified for an RIDABLE_OBJECT type object");
			return;
		}
	}

}