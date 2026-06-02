#include "StdAfx.h"
#include "PgParticleMan.h"
#include "PgParticleCache.H"
#include "PgXmlLoader.h"
#include "PgResourceMonitor.h"
#include "PgMobileSuit.h"
#include "PgSoundMan.h"
#include "PgRenderer.H"
#include "PgWorkerThread.h"
#include "PgWorld.h"
#include "PgActor.h"
#include "PgPilotMan.h"
#include "PgPilot.h"

#include "NewWare/Scene/ApplyTraversal.h"


PgParticleMan::PgParticleMan()
{
	m_spParticleCache = NiNew PgParticleCache();
}

PgParticleMan::~PgParticleMan()
{
	Terminate();
}
void	PgParticleMan::Terminate()
{
	m_spParticleCache = 0;
}

PgParticle *PgParticleMan::GetParticle(char const *pcEffectID,PgParticle::OPTION kValidOptions, float fScale, bool bLoop, bool bZTest, float fSoundTime,bool bAutoGround,bool bUseAppAccumTime, bool bNoFollowParentRotation)
{
	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(pcEffectID, PgResourceMonitor::RESOURCE_TYPE_PARTICLE));

	PgParticlePtr spParticleOriginal = m_spParticleCache->GetParticle(pcEffectID);

	if (spParticleOriginal == NULL)
	{
		return NULL;
	}

	PgParticle *pkParticle = NiDynamicCast(PgParticle, spParticleOriginal->Clone());
	
	if (!pkParticle)
	{
		PG_ASSERT_LOG(!"failed to clonning effect");
		return NULL;
	}

	pkParticle->ApplyOptions(kValidOptions,fScale,bLoop, bZTest, fSoundTime,bAutoGround,bUseAppAccumTime,bNoFollowParentRotation);
	
    NewWare::Scene::ApplyTraversal::Property::SetAlphaGroup( pkParticle, AG_EFFECT );
	
	pkParticle->UpdateProperties();
	pkParticle->UpdateEffects();
	pkParticle->UpdateNodeBound();
	((NiNode *)pkParticle)->Update(0.0f);	

	NiTimeController::StartAnimations(pkParticle, 0.0f);

    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkParticle, false );

	return pkParticle;
}


bool PgParticleMan::ParseXml(char const *pcXmlPath, void *pArg)
{
	TiXmlDocument kXmlDoc(pcXmlPath);

	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
	{
		PgError1("Parse Failed [%s]", pcXmlPath);
		return false;
	}

	// Root 'EFFECT'
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();
	PG_ASSERT_LOG(strcmp(pkElement->Value(), "EFFECT") == 0);

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "ITEM") == 0)
		{
			char const *pcID = 0;
			char const *pcPath = pkElement->GetText();
			char const *pcSound = 0;
			std::string kSourceType = "NIF";
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			float fScale = 1.0f;
			float fSoundTime = -1;
			float fAliveTime = 0.0f;
			bool bLoop = false;
			bool bZTest = true;
			bool bAutoGround = false;
			bool bUseAppAccumTime = false;
			bool bPreCache = false;
			bool bNoFollowParentRotation = false;
			bool bNoUseCacheAndDeepCopy = false;
			std::string kSpriteXML;

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();


				if(strcmp(pcAttrName, "ID") == 0)
				{
					pcID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "SCALE") == 0)
				{
					fScale = (float)atof(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "LOOP") == 0)
				{
					bLoop = strcmp(pcAttrValue, "true") == 0;
				}
				else if(strcmp(pcAttrName, "AUTO_GROUND") == 0)
				{
					bAutoGround = strcmp(pcAttrValue, "true") == 0;
				}
				else if(strcmp(pcAttrName, "ALIVE_TIME") == 0)
				{
					fAliveTime = (float)atof(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "Z_TEST") == 0)
				{
					bZTest = strcmp(pcAttrValue, "true") == 0;
				}
				else if(strcmp(pcAttrName, "USE_APP_ACCUMTIME") == 0)
				{
					bUseAppAccumTime = strcmp(pcAttrValue, "true") == 0;
				}
				else if(strcmp(pcAttrName, "SOUND") == 0)
				{
					pcSound = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "SOUNDTIME") == 0)
				{
					fSoundTime = (float)atof(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "PRECACHE") == 0)
				{
					bPreCache = stricmp(pcAttrValue, "true") == 0;
				}
				else if(strcmp(pcAttrName, "NO_USE_CACHE") == 0)
				{
					bNoUseCacheAndDeepCopy = stricmp(pcAttrValue, "true") == 0;
				}
				else if (strcmp(pcAttrName, "USAGE") == 0)
				{
					// TODO: Check Usage
				}
				else if(strcmp(pcAttrName,"SOURCE_TYPE") == 0)
				{
					kSourceType = pcAttrValue;
				}
				else if (strcmp(pcAttrName,"NO_FOLLOW_PARENT_ROTATION")==0)
				{
					bNoFollowParentRotation = strcmp(pcAttrValue, "true") == 0;
				}
				else if(0 == strcmp(pcAttrName,"SPRITE_XML"))
				{
					kSpriteXML = pcAttrValue;
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			pcPath = pkElement->GetText();

			if(!pcID || !pcPath)
			{
				PgXmlError(pkElement, "Not Enough Effect Data");
				
			}
			else
			{
				NiString strID(pcID);
				strID.ToLower();

				m_spParticleCache->AddParticleInfo(
					(char const*)strID,
					PgParticle::stParticleInfo(
					pcPath,
					fScale,
					bLoop,
					bZTest,
					fSoundTime,
					bAutoGround,
					bUseAppAccumTime,
					bNoFollowParentRotation,
					pcSound ? pcSound : "",
					kSourceType,
					kSpriteXML,
					bNoUseCacheAndDeepCopy,
					fAliveTime));

				// 미리 로딩하도록 한다.	
				if(bPreCache)
				{
					m_spParticleCache->CacheParticle(pcID);
				}
			}
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
		}

		pkElement = pkElement->NextSiblingElement();
	}
	
	return true;
}


void PgParticleMan::ChangeParticleGeneration(NiAVObject *pkObject, bool bGenerate)
{
	if(NiIsKindOf(NiNode, pkObject))
	{
		NiNode	*pkNode = NiDynamicCast(NiNode, pkObject);
		int	iChildArrayCount = pkNode->GetArrayCount();
		NiAVObject	*pkChild = NULL;
		for(int i=0;i<iChildArrayCount;i++)
		{
			pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				ChangeParticleGeneration(pkChild, bGenerate);
			}
		}
	}
	else if(NiIsKindOf(NiParticleSystem, pkObject))
	{
		NiParticleSystem	*pkParticleSystem = NiDynamicCast(NiParticleSystem, pkObject);
		
		int	iCount = pkParticleSystem->GetModifierCount();

		NiTimeController *pkTimeController = pkParticleSystem->GetControllers();
		while(pkTimeController)
		{
			if(NiIsKindOf(NiPSysModifierCtlr,pkTimeController))
			{
				NiPSysModifierCtlr *pkModifier = NiDynamicCast(NiPSysModifierCtlr, pkTimeController);
				pkModifier->SetActive(bGenerate);
			}

			pkTimeController = pkTimeController->GetNext();
		}
	}
}

void PgParticleMan::GetAllParticleNode(const NiNode *pkRootNode, NiObjectList &rkNodeList)
{
	NiTPointerList<NiNodePtr> kStack;
	kStack.AddTail((NiNode*)pkRootNode);
	while(!kStack.IsEmpty())
	{
		NiNodePtr spNode = kStack.RemoveTail();
		for(unsigned int i = 0; i < spNode->GetArrayCount(); ++i)
		{
			NiAVObject *pkObject = spNode->GetAt(i);
			if (!pkObject)
			{
				continue;
			}
			if(NiIsKindOf(NiNode, pkObject))
			{
				kStack.AddTail((NiNode *)pkObject);
			}
			else if(NiIsKindOf(NiParticleSystem, pkObject))
			{
				rkNodeList.AddTail(pkObject);
			}
		}
	}
}

NiQuaternion PgParticleMan::GetBetweenQuaternion(NiPoint3 const &rkSrc, NiPoint3 const &rkDest)
{
	// Dot
	float fDotValue = rkSrc.Dot(rkDest);
	if (fDotValue < -1.0f) fDotValue = -1.0f;
	else if (fDotValue > 1.0f) fDotValue = 1.0f;
	float fRadian = acos(fDotValue);
	float fAngle = fRadian * 180.0f / NI_PI;
	// Cross
	NiPoint3 ptAxisVec;
	ptAxisVec = (rkSrc).UnitCross(rkDest);
	if (ptAxisVec == NiPoint3::ZERO)
	{
		ptAxisVec = NiPoint3::UNIT_Z;
	}
	//Quaternion
	NiQuaternion kQuat;
	NiMatrix3 kMat;
	kQuat = NiQuaternion(fRadian, ptAxisVec);
	kQuat.Normalize();

	return kQuat;
}

bool PgParticleMan::DoClientWork(WorkData& rkWorkData)
{
	return true;
}


void	PgParticleMan::ReleaseCacheBySourceType(std::string const &kSourceType)
{
	m_spParticleCache->ReleaseCacheBySourceType(kSourceType);
}