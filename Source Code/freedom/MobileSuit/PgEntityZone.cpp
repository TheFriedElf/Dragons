#include "StdAfx.h"
#include "PgParticleMan.h"
#include "PgParticle.h"
#include "PgEntityZone.h"
#include "PgAMPool.h"
#include "PgRenderer.H"

NiImplementRTTI(PgEntityZone, PgIWorldObject);

PgEntityZone::PgEntityZone()
:	m_pkParticle(NULL)
{
	m_kNormal = NiPoint3::UNIT_Z;
	m_fRotation = 0.0f;
}

PgEntityZone::~PgEntityZone()
{
}

bool PgEntityZone::Update(float fAccumTime, float fFrameTime)
{
	PgIWorldObject::Update(fAccumTime, fFrameTime);

	return true;
}

void PgEntityZone::InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup)
{
	AttachTo( 1, GetName(), (NiAVObject*)m_pkParticle );
}

void PgEntityZone::ReleasePhysX()
{
}

void PgEntityZone::DrawImmediate( PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime )
{
	// Attach ³à¼®µé Á¤¸®
	PgParticle* pkParticle;
	for ( AttachSlot::iterator itr = m_kAttachSlot.begin(); itr != m_kAttachSlot.end(); ++itr )
	{
		pkParticle = NiDynamicCast(PgParticle, itr->second);
		if ( pkParticle )
		{
			pkParticle->SetAppCulled(false);
			pkRenderer->PartialRenderClick_Deprecated( pkParticle );
			pkParticle->SetAppCulled(true);
		}
	}
}

bool PgEntityZone::ProcessAction(PgAction *pkAction,bool bInvalidateDirection,bool bForceToTransit)
{
	return true;
}

NiObject* PgEntityZone::CreateClone(NiCloningProcess& kCloning)
{
	PgEntityZone* pkClone = NiNew PgEntityZone;
	CopyMembers(pkClone , kCloning);
	return pkClone;
}

void PgEntityZone::ProcessClone(NiCloningProcess& kCloning)
{
	PgIWorldObject::ProcessClone(kCloning);

	NiObject *pkClone = 0;

	bool bCloned = kCloning.m_pkCloneMap->GetAt(this, pkClone);
	PgEntityZone *pkDest = 0;

	if(bCloned)
	{
		pkDest = (PgEntityZone *) pkClone;
	}
	else
	{
		pkDest = this;
	}
}

void PgEntityZone::SetNormal(NiPoint3 const &rkNormal)
{
	NiQuaternion kNormalQuat(NiACos(NiPoint3::UNIT_Z.Dot(rkNormal)), NiPoint3::UNIT_Z.UnitCross(rkNormal));
	NiQuaternion kRotateQuat(m_fRotation, NiPoint3::UNIT_Z);
	SetRotate(kNormalQuat * kRotateQuat);

	m_kNormal = rkNormal;
}

void PgEntityZone::IncRotate(float fAngle)
{
	m_fRotation += fAngle;
	SetNormal(m_kNormal);
}

bool PgEntityZone::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();

	TiXmlElement *pkElement = NULL;
	TiXmlAttribute* pkAttr = NULL;
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);

			char const *pcTagName = pkElement->Value();

			if( !strcmp(pcTagName, "ENTITY") )
			{
				pkAttr = pkElement->FirstAttribute();
				while( pkAttr )
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if( !strcmp(pcAttrName, "ID") )
					{
						SetName(pcAttrValue);
					}

					pkAttr = pkAttr->Next();
				}

				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if ( NULL != pkChildNode && !this->ParseXml(pkChildNode) )
				{
					return false;
				}
			}
			else if( !strcmp(pcTagName, "ITEM") )
			{
				eEntityType eType = ENTITY_NONE;
				pkAttr = pkElement->FirstAttribute();
				while( pkAttr )
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if( !strcmp(pcAttrName, "TYPE") )
					{
						if ( !strcmp(pcAttrValue,"PARTICLE") )
						{
							eType = ENTITY_PARTICLE;
						}
					}
					else if( !strcmp(pcAttrName, "ID") )
					{
						switch( eType )
						{
						case ENTITY_PARTICLE:
							{
								m_pkParticle = g_kParticleMan.GetParticle( pcAttrValue );
							}break;
						default:
							break;
						}
					}
					pkAttr = pkAttr->Next();
				}

			}
		}break;
	default:
		return false;
	}
	return true;
}