#include "stdafx.h"
#include "PgIWorldObjectBase.h"

#include "PgIWorldObject.h"
#include "PgAMPool.h"
#include "PgScripting.h"
#include "PgParticleMan.h"
#include "PgParticle.h"
#include "PgActionSlot.h"
#include "PgWorld.H"
#include "PgMobileSuit.h"
#include "PgNifMan.h"
#include "PgRenderer.H"
#include "PgActor.h"

#include "NewWare/Scene/ApplyTraversal.h"


PgIWorldObjectBase::PgIWorldObjectBase() :
	m_spAM(0),
	m_spSubAM(0),
	m_bUseSubActorManager(false),
	m_pkActionSlot(0),
	m_fEffectScale(1.0f),
	m_fScale(1.0f),
	m_pkSubIWorldObjectBase(0)
{
	m_strDamageBlinkID = "se_damage_blink";
}

PgIWorldObjectBase::~PgIWorldObjectBase()
{
	if (m_spAM)
	{
		NiAVObject* pkRoot = m_spAM->GetNIFRoot();
		NiNode* pkRootNode = NiDynamicCast(NiNode, pkRoot);

		if (pkRoot && pkRootNode)
		{
			CameraAnimationInfoMap::iterator iter = m_kCameraAniMap.begin();
			while (iter != m_kCameraAniMap.end())
			{
				NiAVObject* pkObject = pkRootNode->GetObjectByName((*iter).first.c_str());
				if (pkObject != NULL)
				{
					pkRootNode->DetachChild(pkObject);
				}
				++iter;
			}
		}
	}

#ifdef PG_USE_ACTORMANAGER_REUSE
	g_kAMPool.ReturnActorManager(m_spAM);
	g_kAMPool.ReturnActorManager(m_spSubAM);
#else
	m_spAM = 0;
	m_spSubAM = 0;
#endif
	SAFE_DELETE_NI(m_pkActionSlot);
	SAFE_DELETE_NI(m_pkSubIWorldObjectBase);
}

PgIWorldObjectBase *PgIWorldObjectBase::GetSubIWorldObjectBase()
{
	return m_pkSubIWorldObjectBase;
}

void PgIWorldObjectBase::RemoveSubIWorldObjectBase()
{
	SAFE_DELETE_NI(m_pkSubIWorldObjectBase);
}

void PgIWorldObjectBase::SetSubIWorldObjectBase(PgIWorldObjectBase *pkSub)
{
	if(!pkSub || pkSub->GetSubIWorldObjectBase())
	{
		return;
	}

	m_pkSubIWorldObjectBase = pkSub;
}

PgActionSlot *PgIWorldObjectBase::GetActionSlot() const
{
	if(m_pkSubIWorldObjectBase)
	{
		return m_pkSubIWorldObjectBase->GetActionSlot();
	}
	return m_pkActionSlot;
}

NiActorManager *PgIWorldObjectBase::GetActorManager() const
{
	if(m_pkSubIWorldObjectBase)
	{
		return m_pkSubIWorldObjectBase->GetActorManager();
	}

	if (m_bUseSubActorManager && m_spSubAM)
		return m_spSubAM;

	return m_spAM;
}

NiActorManager *PgIWorldObjectBase::GetSubActorManager() const
{
	return m_spSubAM;
}

NiAVObject *PgIWorldObjectBase::GetNIFRoot() const
{
	if(m_pkSubIWorldObjectBase)
	{
		return m_pkSubIWorldObjectBase->GetNIFRoot();
	}

	PG_ASSERT_LOG(m_spAM);
	if (m_bUseSubActorManager && m_spSubAM)
		return m_spSubAM->GetNIFRoot();

	if (m_spAM)
		return m_spAM->GetNIFRoot();

	return NULL;
}
NiAVObject*	PgIWorldObjectBase::GetMainNIFRoot()	const
{
	if (m_spAM)
	{
		return m_spAM->GetNIFRoot();
	}

	return	NULL;
}
const std::wstring &PgIWorldObjectBase::GetUpdateScript() const
{
	if(m_pkSubIWorldObjectBase)
	{
		return m_pkSubIWorldObjectBase->GetUpdateScript();
	}

	return m_kUpdateScript;
}

const std::wstring &PgIWorldObjectBase::GetEventScript() const
{
	if(m_pkSubIWorldObjectBase)
	{
		return m_pkSubIWorldObjectBase->GetEventScript();
	}

	return m_kEventScript;
}

const std::wstring &PgIWorldObjectBase::GetPickupScript() const
{
	if(m_pkSubIWorldObjectBase)
	{
		return m_pkSubIWorldObjectBase->GetPickupScript();
	}

	return m_kPickupScript;
}

const std::wstring &PgIWorldObjectBase::GetMouseOverScript() const
{
	if(m_pkSubIWorldObjectBase)
	{
		return m_pkSubIWorldObjectBase->GetMouseOverScript();
	}

	return m_kMouseOverScript;
}

const std::wstring &PgIWorldObjectBase::GetMouseOutScript() const
{
	if(m_pkSubIWorldObjectBase)
	{
		return m_pkSubIWorldObjectBase->GetMouseOutScript();
	}

	return m_kMouseOutScript;
}

const std::wstring &PgIWorldObjectBase::GetInitScript() const
{
	if(m_pkSubIWorldObjectBase)
	{
		return m_pkSubIWorldObjectBase->GetInitScript();
	}

	return m_kInitScript;
}
float PgIWorldObjectBase::GetEffectScale() const
{
	if(m_pkSubIWorldObjectBase)
	{
		return m_pkSubIWorldObjectBase->GetEffectScale();
	}

	return m_fEffectScale;
}

void PgIWorldObjectBase::SetPickupScript(std::wstring kScriptName)
{
	m_kPickupScript = kScriptName;
}

void PgIWorldObjectBase::SetMouseOverScript(std::wstring kScriptName)
{
	m_kMouseOverScript = kScriptName;
}

void PgIWorldObjectBase::SetMouseOutScript(std::wstring kScriptName)
{
	m_kMouseOutScript = kScriptName;
}

void PgIWorldObjectBase::SetUpdateScript(std::wstring kScriptName)
{
	m_kUpdateScript = kScriptName;
}

PgIWorldObjectBase::ABVShape* PgIWorldObjectBase::GetABVShape(int iIndex)
{	
	assert(iIndex>=0 && iIndex<PG_MAX_NB_ABV_SHAPES);
	return &m_kABVShapes[iIndex];	
}
PgIWorldObjectBase*	PgIWorldObjectBase::CreateCopy(PgIWorldObject *pkWorldObject,bool bBeSubBase)
{
	PgIWorldObjectBase *pkNewBase = NiNew PgIWorldObjectBase();

	if(!bBeSubBase)
	{
		pkWorldObject->SetWorldObjectBase(pkNewBase);
	}
	else
	{
		pkWorldObject->SetSubIWorldObjectBase(pkNewBase);
	}

	pkNewBase->m_fScale = m_fScale;

	if(m_spAM)
	{
		std::string kKFMPath = m_spAM->GetKFMFileName();

		NiActorManagerPtr spAM = g_kAMPool.LoadActorManager(kKFMPath.c_str(), PgIXmlObject::ID_PC);

		if (spAM == NULL)
		{
			PgError1("[PgIWorldObjectBase] can't load %s", kKFMPath.c_str());
			return false;
		}

		NiQuaternion kQuat;
		m_spAM->GetNIFRoot()->GetRotate(kQuat);
		pkNewBase->SetActorManager(spAM, pkWorldObject, m_spAM->GetNIFRoot()->GetScale(), false, kQuat);
	}
	if(m_spSubAM)
	{
		std::string kKFMPath = m_spSubAM->GetKFMFileName();
		NiActorManagerPtr spAM = g_kAMPool.LoadActorManager(kKFMPath.c_str(), PgIXmlObject::ID_PC);

		if (spAM == NULL)
		{
			PgError1("[PgIWorldObjectBase] can't load %s", kKFMPath.c_str());
			return false;
		}

		NiQuaternion kQuat;
		m_spSubAM->GetNIFRoot()->GetRotate(kQuat);
		pkNewBase->SetActorManager(spAM, pkWorldObject, m_spSubAM->GetNIFRoot()->GetScale(), true, kQuat);
	}	

	pkNewBase->m_kUpdateScript = m_kUpdateScript;
	pkNewBase->m_kEventScript = m_kEventScript;
	pkNewBase->m_kPickupScript = m_kPickupScript;
	pkNewBase->m_kMouseOverScript = m_kMouseOverScript;
	pkNewBase->m_kMouseOutScript = m_kMouseOutScript;
	pkNewBase->m_kInitScript = m_kInitScript;
	pkNewBase->m_pkActionSlot = m_pkActionSlot != NULL ? m_pkActionSlot->Clone() : NULL;
	pkNewBase->m_fEffectScale = m_fEffectScale;
	pkNewBase->SetDamageBlinkID(GetDamageBlinkID().c_str());

	for(int i=0;i<PG_MAX_NB_ABV_SHAPES;i++)
	{
		pkNewBase->m_kABVShapes[i].m_eType = m_kABVShapes[i].m_eType;
		pkNewBase->m_kABVShapes[i].m_kTo = m_kABVShapes[i].m_kTo;
		pkNewBase->m_kABVShapes[i].kCapsuleDesc.height = m_kABVShapes[i].kCapsuleDesc.height;
		pkNewBase->m_kABVShapes[i].kCapsuleDesc.radius = m_kABVShapes[i].kCapsuleDesc.radius;
		pkNewBase->m_kABVShapes[i].kSphereDesc.radius = m_kABVShapes[i].kSphereDesc.radius;
		pkNewBase->m_kABVShapes[i].kBoxDesc.dimensions.x = m_kABVShapes[i].kBoxDesc.dimensions.x;
		pkNewBase->m_kABVShapes[i].kBoxDesc.dimensions.y = m_kABVShapes[i].kBoxDesc.dimensions.y;
		pkNewBase->m_kABVShapes[i].kBoxDesc.dimensions.z = m_kABVShapes[i].kBoxDesc.dimensions.z;
		pkNewBase->m_kABVShapes[i].m_kMat = m_kABVShapes[i].m_kMat;

	}

	//	"ATTACH"
	pkNewBase->m_kEffectAttachInfoList = m_kEffectAttachInfoList;
	AttachEffects(pkWorldObject);

	pkNewBase->m_wstrIconID = m_wstrIconID;
	pkNewBase->m_kCameraAniMap = m_kCameraAniMap;
	pkNewBase->GetActorManager()->Update(0.0f);

	return	pkNewBase;
}
PgIWorldObjectBase *PgIWorldObjectBase::Clone()
{
	// TODO : PgIWorldObjectBase에서 AM을 클론해버리면, 너무 느리기 때문에 PC같은 경우 문제가 있다.
	PgIWorldObjectBase *pkCloned = NiNew PgIWorldObjectBase;
	if(m_spAM)
	{
		pkCloned->m_spAM = m_spAM->Clone();
	}

	if(m_spSubAM)
	{
		pkCloned->m_spSubAM = m_spSubAM->Clone();
	}

	if(m_pkActionSlot)
	{
		pkCloned->m_pkActionSlot = m_pkActionSlot->Clone();
	}

	pkCloned->SetDamageBlinkID( m_strDamageBlinkID.c_str() );
	return pkCloned;
}

void PgIWorldObjectBase::SetUseSubActorManager(bool bUse)
{
	if (m_bUseSubActorManager == bUse)
		return;

	if (bUse)
	{
		PG_ASSERT_LOG(m_spSubAM);
		if (m_spSubAM == NULL)
		{
			m_bUseSubActorManager = false;
			return;
		}

		NiActorManager *pkAM = GetActorManager();
		PG_ASSERT_LOG(pkAM);
		if (pkAM)
		{
			pkAM->GetNIFRoot()->SetAppCulled(true);

			if(m_spSubAM->GetNIFRoot())
			{
				//	SubAM 의 Scale 을 AM 과 동일하게 맞춰준다.
				m_spSubAM->GetNIFRoot()->SetScale(pkAM->GetNIFRoot()->GetScale());
			}
		}

		//	SubAM 의 Translate 를 AM 과 동일하게 맞춰준다.
		m_spSubAM->GetNIFRoot()->SetTranslate(NiPoint3(0,0,-PG_CHARACTER_Z_ADJUST));
		m_spSubAM->GetNIFRoot()->SetAppCulled(false);


		if(m_spSubAM->GetNIFRoot()->GetParent())
		{
			m_spSubAM->GetNIFRoot()->GetParent()->Update(0);
		}
		else
			m_spSubAM->GetNIFRoot()->Update(0);

		m_bUseSubActorManager = true;
	}
	else
	{
		if (m_spSubAM)
			m_spSubAM->GetNIFRoot()->SetAppCulled(true);
		m_bUseSubActorManager = false;

		NiActorManager *pkAM = GetActorManager();
		PG_ASSERT_LOG(pkAM);
		if (pkAM)
			pkAM->GetNIFRoot()->SetAppCulled(false);
	}
}

void	PgIWorldObjectBase::ParseWorldEventStateInfo(const TiXmlNode *pkNode)
{
	if(!pkNode)
	{
		return;
	}

	int const iType = pkNode->Type();
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			
			std::string kTagName(pkElement->Value());

			if(kTagName == "STATE")
			{
				const TiXmlAttribute *pAttr = pkElement->FirstAttribute(); 

				int	iID=-1;
				std::string	kSlotID;

				while(pAttr)
				{
					std::string const pcAttrName(pAttr->Name());
					std::string const pcAttrValue(pAttr->Value());


					if(pcAttrName == "ID")
					{
						iID = atoi(pcAttrValue.c_str());
					}
					else if(pcAttrName == "SLOT")
					{
						kSlotID = pcAttrValue;
					}

					pAttr = pAttr->Next();
				}

				LWR(kSlotID);
				m_kWorldEventStateInfo.m_kStateCont.push_back(stWorldEventStateInfo::stState(iID,kSlotID));
			}
			else if(kTagName == "TRANSIT")
			{
				const TiXmlAttribute *pAttr = pkElement->FirstAttribute(); 

				int	iFromID=-1,iToID=-1;
				std::string	kSlotID;

				while(pAttr)
				{
					std::string const pcAttrName(pAttr->Name());
					std::string const pcAttrValue(pAttr->Value());


					if(pcAttrName == "FROM")
					{
						iFromID = atoi(pcAttrValue.c_str());
					}
					else if(pcAttrName == "TO")
					{
						iToID = atoi(pcAttrValue.c_str());
					}
					else if(pcAttrName == "SLOT")
					{
						kSlotID = pcAttrValue;
					}

					pAttr = pAttr->Next();
				}

				LWR(kSlotID);
				m_kWorldEventStateInfo.m_kTransitCont.push_back(stWorldEventStateInfo::stTransit(iFromID,iToID,kSlotID));

			}
		}
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		ParseWorldEventStateInfo(pkNextNode);
	}
}
bool PgIWorldObjectBase::ParseXml(const TiXmlNode *pkNode, PgIWorldObject *pkWorldObject, bool bBeSubBase)
{
	if(!pkWorldObject)
	{
		return false;
	}

	int const iType = pkNode->Type();
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			
			std::string kTagName(pkElement->Value());

			PgActor::EQUIP_ITEM_CONT kEquipItemList;

			if(kTagName == "WORLDOBJECT")
			{
				// this를 WorldObject의 Base로 설정
				if(!bBeSubBase)
				{
					pkWorldObject->SetWorldObjectBase(this);
				}
				else
				{
					pkWorldObject->SetSubIWorldObjectBase(this);
				}

				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml(pkChildNode, pkWorldObject, bBeSubBase))
					{
						return false;
					}
				}
			}
			else if(kTagName == "WORLD_EVENT_STATE_INFO")
			{
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				ParseWorldEventStateInfo(pkChildNode);
			}
			else if(kTagName == "KFMPATH" || kTagName == "SUB_KFMPATH")
			{
				bool bSubAM = false;
				if (kTagName == "SUB_KFMPATH")
				{
					bSubAM = true;
				}

				float fScale = 1.0f;
				NiQuaternion kQuat = NiQuaternion::IDENTITY;
				std::string pcKFMPath = pkElement->GetText();
				PG_ASSERT_LOG(!pcKFMPath.empty());

				TiXmlAttribute const* pAttr = pkElement->FirstAttribute(); 

				while(pAttr)
				{
					char const *pcAttrName = pAttr->Name();
					char const *pcAttrValue = pAttr->Value();

					if(strcmp(pcAttrName, "SCALE") == 0)
					{
						fScale = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ROT_X") == 0)
					{
						kQuat.m_fX = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ROT_Y") == 0)
					{
						kQuat.m_fY = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ROT_Z") == 0)
					{
						kQuat.m_fZ = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ROT_ANGLE") == 0)
					{
						kQuat.m_fW = (float)atof(pcAttrValue);
					}
					else
					{
						NILOG(PGLOG_ERROR, "[PgIWorldObjectBase] Invalid attribute %s\n", pcAttrName);
					}

					pAttr = pAttr->Next();
				}

				if (g_iUseAddUnitThread != 2)
				{
					m_fScale = fScale;
					// KFM 경로를 알아낸다.
					NiActorManagerPtr spAM = g_kAMPool.LoadActorManager(pcKFMPath.c_str(), PgIXmlObject::ID_PC);

					if (spAM == NULL)
					{
						PgError1("[PgIWorldObjectBase] can't load %s", pcKFMPath.c_str());
						return false;
					}

					SetActorManager(spAM, pkWorldObject, fScale, bSubAM, kQuat);
				}
			}
			else if(kTagName == "SCRIPT")
			{
				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				if(pkAttr)
				{
					char const *pcName = pkAttr->Name();
					char const *pcValue = pkAttr->Value();

					if(strcmp(pcName, "SRC") == 0)
					{
						PgScripting::DoFile(pcValue);
					}
				}

				TiXmlElement *pkChildElement = pkElement->FirstChildElement();
				while(pkChildElement)
				{
					char const *pcTagName = pkChildElement->Value();
					if(strcmp(pcTagName, "ITEM") == 0)
					{
						pkAttr = pkChildElement->FirstAttribute();

						std::string kScriptName;

						while(pkAttr)
						{
							char const *pcName = pkAttr->Name();
							char const *pcValue = pkAttr->Value();

							if(strcmp(pcName, "NAME") == 0)
							{
								kScriptName = std::string(pcValue);
							}
							else if(strcmp(pcName, "TYPE") == 0)
							{
								if(!kScriptName.empty())
								{
									if(strcmp(pcValue, "COMMON") == 0)
									{
										m_kUpdateScript = UNI(kScriptName);
									}
									else if(strcmp(pcValue, "EVENT") == 0)
									{
										m_kEventScript = UNI(kScriptName);
									}
									else if(strcmp(pcValue, "PICKUP") == 0)
									{
										m_kPickupScript = UNI(kScriptName);
									}
									else if(strcmp(pcValue, "MOUSEOVER") == 0)
									{
										m_kMouseOverScript = UNI(kScriptName);
									}
									else if(strcmp(pcValue, "MOUSEOUT") == 0)
									{
										m_kMouseOutScript = UNI(kScriptName);
									}
									else if(strcmp(pcValue, "INIT") == 0)
									{
										m_kInitScript = UNI(kScriptName);
									}
									else
									{
										PgError1("Unknown Script Type : %s", pcValue);
									}
								}
							}
							pkAttr = pkAttr->Next();
						}	// end while
					}
					else
					{
						PgError1("Unknown Tag : %s", pcTagName);
					}

					pkChildElement = pkChildElement->NextSiblingElement();
				}
			}
			else if(kTagName == "SLOT")
			{
				m_pkActionSlot = new PgActionSlot;
				PG_ASSERT_LOG(m_pkActionSlot);
				
				if(m_pkActionSlot == NULL || m_pkActionSlot->ParseXml(pkNode, GetActorManager()) == false)
				{
					SAFE_DELETE(m_pkActionSlot);
					PG_ASSERT_LOG(!"faild to loading Actor's Slot");
					return false;
				}
			}
			else if(kTagName == "EFFECT")
			{
				TiXmlAttribute const* pAttr = pkElement->FirstAttribute(); 
				
				while(pAttr)
				{
					char const *pcAttrName = pAttr->Name();
					char const *pcAttrValue = pAttr->Value();

					if(strcmp(pcAttrName, "SCALE") == 0)
					{
						m_fEffectScale = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "DAMAGEBLINK") == 0)
					{
						SetDamageBlinkID( pAttr->Value() );
					}
					else
					{
						NILOG(PGLOG_ERROR, "[PgIWorldObjectBase] Invalid attribute %s\n", pcAttrName);
					}

					pAttr = pAttr->Next();
				}
			}
			else if(kTagName == "ABV")
			{
				unsigned int i = 0;
				TiXmlElement *pkShapeElement = pkElement->FirstChildElement();

				while(pkShapeElement && i < PG_MAX_NB_ABV_SHAPES)
				{
					char const *pcShapeType = pkShapeElement->Value();

					if(strcmp(pcShapeType, "CAPSULE") == 0)
					{	
						m_kABVShapes[i].m_eType = ABVShape::ST_CAPSULE;
					}
					else if(strcmp(pcShapeType, "SPHERE") == 0)
					{
						m_kABVShapes[i].m_eType = ABVShape::ST_SPHERE;
					}
					else if(strcmp(pcShapeType, "BOX") == 0)
					{
						m_kABVShapes[i].m_eType = ABVShape::ST_BOX;
					}

					TiXmlAttribute const* pkAttr = pkShapeElement->FirstAttribute();
					while(pkAttr)
					{
						char const *pcAttrName = pkAttr->Name();
						char const *pcAttrValue = pkAttr->Value();

						if(strcmp(pcAttrName, "TO") == 0)
						{
							m_kABVShapes[i].m_kTo = pcAttrValue;
						}
						else if(strcmp(pcAttrName, "SIZE") == 0)
						{
							if(m_kABVShapes[i].m_eType == ABVShape::ST_CAPSULE)
							{
								sscanf_s(pcAttrValue, "%f, %f", 
									&(m_kABVShapes[i].kCapsuleDesc.height), 
									&(m_kABVShapes[i].kCapsuleDesc.radius));

								m_kABVShapes[i].kCapsuleDesc.height *= m_fScale;
								m_kABVShapes[i].kCapsuleDesc.radius *= m_fScale;
							}
							else if(m_kABVShapes[i].m_eType == ABVShape::ST_SPHERE)
							{
								sscanf_s(pcAttrValue, "%f",
									&(m_kABVShapes[i].kSphereDesc.radius));

								m_kABVShapes[i].kSphereDesc.radius *= m_fScale;
							}
							else if(m_kABVShapes[i].m_eType == ABVShape::ST_BOX)
							{
								sscanf_s(pcAttrValue, "%f, %f, %f", 
									&(m_kABVShapes[i].kBoxDesc.dimensions.x), 
									&(m_kABVShapes[i].kBoxDesc.dimensions.y), 
									&(m_kABVShapes[i].kBoxDesc.dimensions.z));

								m_kABVShapes[i].kBoxDesc.dimensions.x *= m_fScale;
								m_kABVShapes[i].kBoxDesc.dimensions.y *= m_fScale;
								m_kABVShapes[i].kBoxDesc.dimensions.z *= m_fScale;
							}
						}
						else if(strcmp(pcAttrName, "ROTATE") == 0)
						{
							float fX, fY, fZ, fAngle;
							sscanf_s(pcAttrValue, "%f, %f, %f, %f",
								&fX, &fY, &fZ, &fAngle);
							NiPhysXTypes::NiQuaternionToNxMat33(NiQuaternion(fAngle / 180.0f * NI_PI, NiPoint3(fX, fY, fZ)),
								m_kABVShapes[i].m_kMat.M);
						}
						else if(strcmp(pcAttrName, "LOCATE") == 0)
						{
							sscanf_s(pcAttrValue, "%f, %f, %f",
								&(m_kABVShapes[i].m_kMat.t.x), &(m_kABVShapes[i].m_kMat.t.y), &(m_kABVShapes[i].m_kMat.t.z));

						}
						pkAttr = pkAttr->Next();
					}

					i++;
					pkShapeElement = pkShapeElement->NextSiblingElement();
				}
			}
			else if(kTagName == "ATTACH")
			{
				int iSlot = 0;
				char const *pcEffect = 0;
				char const *pcTo = 0;
				float fScale = 0.0f;

				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "EFFECT") == 0)
					{
						pcEffect = pcAttrValue;
					}
					else if(strcmp(pcAttrName, "TO") == 0)
					{
						pcTo = pcAttrValue;
					}
					else if(strcmp(pcAttrName, "SLOT") == 0)
					{
						iSlot = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "SCALE") == 0)
					{
						fScale = atof(pcAttrValue);
					}

					pkAttr = pkAttr->Next();
				}

				if(iSlot && pcEffect && pcTo)
				{
					stEffectAttachInfo kEffectAttachInfo(std::string(pcEffect),std::string(pcTo),iSlot);
					m_kEffectAttachInfoList.push_back(kEffectAttachInfo);

					PgParticle *pkParticle = g_kParticleMan.GetParticle(pcEffect, PgParticle::O_SCALE,0.0f>=fScale?m_fEffectScale:fScale);
					if(!pkWorldObject->AttachTo(iSlot, pcTo, pkParticle))
					{
						THREAD_DELETE_PARTICLE(pkParticle);
					}
					PgActor* pkActor = dynamic_cast<PgActor*>(pkWorldObject);
					if(pkActor)
					{
						pkActor->AddTransformEffectAttachInfo(PgActor::E_NONE_INIT, kEffectAttachInfo);
					}
				}
			}
			else if(kTagName == "ICON")
			{
				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					if(strcmp(pcAttrName, "ID") == 0)
					{
						m_wstrIconID = UNI(pkAttr->Value());
					}
					pkAttr = pkAttr->Next();
				}
			}
			else if(kTagName == "CAMERA")
			{
				TiXmlAttribute const* pAttr = pkElement->FirstAttribute(); 
				char const* pcCameraName = NULL;

				while(pAttr)
				{
					char const *pcAttrName = pAttr->Name();
					char const *pcAttrValue = pAttr->Value();

					if(strcmp(pcAttrName, "NAME") == 0)
					{
						pcCameraName = pcAttrValue;
					}
					pAttr = pAttr->Next();
				}				

				m_kCameraAniMap.insert(std::make_pair(std::string(pcCameraName), stCameraAnimationInfo(pcCameraName, pkElement->GetText())));

			}
			else if ( kTagName == "OWNER" )
			{
				break;
			}
			else if(kTagName == "EQUIP_ITEM")
			{
				int iEquipPos = 0;
				int iItemNo = 0;

				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "EQUIP_POS") == 0)
					{
						iEquipPos = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ITEM_NO") == 0)
					{
						iItemNo = atoi(pcAttrValue);
					}

					pkAttr = pkAttr->Next();
				}

				if(iEquipPos && iItemNo)
				{
					kEquipItemList.insert(std::make_pair(iEquipPos,iItemNo));
				}
			}
			else if(kTagName == "TYPE")
			{
				break;
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", kTagName.c_str());
				break;
			}

			if(!kEquipItemList.empty())
			{
				if(PgActor * pkActor = dynamic_cast<PgActor*>(pkWorldObject))
				{
					pkActor->SetAdjustedItem(kEquipItemList);
					//pkActor->EquipAllItem(); Pilot이 아직 생성되지 않아서 없음
				}
			}
		}

	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode, pkWorldObject, bBeSubBase))
		{
			return false;
		}
	}

	// 모든 파싱이 끝났다면 Actor를 초기화한다.
	if(strcmp(pkNode->Value(), "WORLDOBJECT") == 0)
	{
		if (GetActorManager())
		{
			NiAVObject* pkRoot = GetActorManager()->GetNIFRoot();
			NiNode* pkRootNode = NiDynamicCast(NiNode, pkRoot);

			if (pkRoot && pkRootNode)
			{
				CameraAnimationInfoMap::iterator iter = m_kCameraAniMap.begin();
				while (iter != m_kCameraAniMap.end())
				{
					if (pkRootNode->GetObjectByName((*iter).first.c_str()) != NULL)
					{
						NILOG(PGLOG_ERROR, "[PgIWorldBaseObjectBase] %s object has %s node already!\n", m_spAM->GetKFMFileName(), (*iter).first.c_str());
					}
					else
					{
						NiNodePtr spCamera = g_kNifMan.GetNif((*iter).second.m_kCameraAniFilePath);
						if (spCamera)
						{
							pkRootNode->AttachChild(spCamera, true);
						}
					}
					++iter;
				}
			}
			GetActorManager()->Update(0.0f);
		}
	}

	return true;
}

unsigned int PgIWorldObjectBase::GetLODCount()
{
	NiActorManager* pkAM = GetActorManager();
	if (pkAM)
	{
		NiBoneLODController* pkLODController = pkAM->GetBoneLODController();
		if (pkLODController)
		{
			return pkLODController->GetNumberOfBoneLODs();
		}
	}

	return 0;
}

bool PgIWorldObjectBase::SetLOD(int iLODLevel)
{
	NiActorManager* pkAM = GetActorManager();
	if (pkAM)
	{
		NiBoneLODController* pkLODController = pkAM->GetBoneLODController();
		if (pkLODController)
		{
			return pkLODController->SetBoneLOD(iLODLevel);
		}
	}
	return false;
}

int PgIWorldObjectBase::GetCurrentLOD()
{
	NiActorManager* pkAM = GetActorManager();
	if (pkAM)
	{
		NiBoneLODController* pkLODController = pkAM->GetBoneLODController();
		if (pkLODController)
		{
			return pkLODController->GetBoneLOD();
		}
	}
	return 0;
}
void	PgIWorldObjectBase::ChangeActorManager(std::string const &kKFMPath,PgIWorldObject *pkWorldObject)
{
	if(!pkWorldObject)
	{
		return;
	}
	NiActorManagerPtr spAM = g_kAMPool.LoadActorManager(kKFMPath.c_str(), PgIXmlObject::ID_PC);
	if(!spAM)
	{
		return;
	}

	pkWorldObject->UnregisterEventCallback();
	pkWorldObject->ReleaseAllParticles();
	pkWorldObject->NfyActorManagerChanged(true);

	float	fScale = 1.0f;
	NiQuaternion	kQuat;

	NiAVObject	*pkCurrentRoot = m_spAM->GetNIFRoot();
	if(pkCurrentRoot)
	{
		NiTransform const &kTransform = pkCurrentRoot->GetLocalTransform();

		fScale = kTransform.m_fScale;
		kQuat.FromRotation(kTransform.m_Rotate);

		NiAVObject	*pkNewRoot = spAM->GetNIFRoot();
		if(pkNewRoot)
		{
			pkNewRoot->SetTranslate(kTransform.m_Translate);
		}
	}

	//g_kAMPool.ReturnActorManager(m_spAM);
	//m_spAM = 0;
	//pkWorldObject->DetachChild(pkCurrentRoot);
	pkCurrentRoot->SetAppCulled(true);	//	현재 NifRoot 는 Hide시킨다.

	SetActorManager(spAM,pkWorldObject,fScale,false,kQuat);


	pkWorldObject->UpdateNodeBound();
	pkWorldObject->UpdateProperties();
	pkWorldObject->UpdateEffects();
	pkWorldObject->NiAVObject::Update(0,true);

	pkWorldObject->ResetABVToNewAM();
	pkWorldObject->NfyActorManagerChanged(false);

	AttachEffects(pkWorldObject);
	
}
void*	PgIWorldObjectBase_Terminate_CallBack(NiAVObject *pkObject)
{
	__asm	int	3;
}
void*	PgIWorldObjectBase_Detach_CallBack(NiAVObject *pkObject)
{
	__asm	int	3;
}

bool PgIWorldObjectBase::SetActorManager(NiActorManagerPtr spAM, PgIWorldObject *pkWorldObject, float fScale, bool bSubAM, const NiQuaternion &rkQuat)
{
	PG_ASSERT_LOG(spAM);
	PG_ASSERT_LOG(pkWorldObject);
	if (spAM == NULL || pkWorldObject == NULL || fScale <= 0.0f)
	{
		return false;
	}


	NiNodePtr spNifRoot = (NiNode *)spAM->GetNIFRoot();

	spAM->Update(0);

	PG_ASSERT_LOG(spNifRoot);
	if (spNifRoot) 
	{
		spNifRoot->SetScale(fScale);
		spNifRoot->SetRotate(rkQuat);
	}

	if (bSubAM)
	{
		m_spSubAM = spAM;
		pkWorldObject->RegisterSubEventCallback();
	}
	else
	{
		m_spAM = spAM;
		// EventCallback Event를 등록한다.
		pkWorldObject->RegisterEventCallback();
	}

	pkWorldObject->AttachChild(spNifRoot, true);

	spNifRoot->SetSelectiveUpdate(true);
	spNifRoot->SetSelectiveUpdateTransforms(true);
	spNifRoot->SetSelectiveUpdatePropertyControllers(true);
	spNifRoot->SetSelectiveUpdateRigid(true);
	spNifRoot->UpdateNodeBound();
	spNifRoot->UpdateProperties();
	spNifRoot->UpdateEffects();
	spNifRoot->Update(0.0f);
	NewWare::Scene::ApplyTraversal::Property::SetAlphaGroup( spNifRoot, AG_EFFECT );

	if (bSubAM)
		spNifRoot->SetAppCulled(true);

	pkWorldObject->NfyActorManagerLoadingComplete();

	return true;
}

void PgIWorldObjectBase::AttachEffects(PgIWorldObject *pkWorldObject)
{
	if(pkWorldObject)
	{
		for(EffectAttachInfoList::iterator itor = m_kEffectAttachInfoList.begin(); itor != m_kEffectAttachInfoList.end(); ++itor)
		{
			stEffectAttachInfo& kInfo = (*itor);
			PgParticle	*pkParticle = g_kParticleMan.GetParticle(kInfo.m_kEffectID.c_str(),PgParticle::O_SCALE,m_fEffectScale);

			if(!pkWorldObject->AttachTo(kInfo.m_iSlot, kInfo.m_kTo.c_str(),pkParticle))
			{	
				THREAD_DELETE_PARTICLE(pkParticle);
			}
		}
	}
}
void PgIWorldObjectBase::DetachEffects(PgIWorldObject *pkWorldObject)
{
	if(pkWorldObject)
	{
		for(EffectAttachInfoList::iterator itor = m_kEffectAttachInfoList.begin(); itor != m_kEffectAttachInfoList.end(); ++itor)
		{
			stEffectAttachInfo& kInfo = (*itor);
			pkWorldObject->DetachFrom(kInfo.m_iSlot, true);
		}
	}
}