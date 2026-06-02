#include "StdAfx.h"
#include "PgMobileSuit.h"
#include "PgFurniture.h"
#include "PgInterpolator.h"
#include "PgAMPool.h"
#include "PgNifMan.h"

extern ControllerManager g_kControllerManager;

NiImplementRTTI(PgFurniture, PgIWorldObject);

PgFurniture::PgFurniture()
{
	m_kNormal = NiPoint3::UNIT_Z;
	m_kCellSize = NiPoint3::ZERO;
	m_kCellCenter = NiPoint3::ZERO;
	m_fRotation = 0.0f;
	m_iRotation = 0;
	m_kMeshRoot = NULL;
	m_eFurnitureType = FURNITURE_TYPE_NONE;
	m_pkItemDef = NULL;

	m_pkPhysXActor = 0;
	m_pkPhysXSrc = 0;
	m_pkPhysXDest = 0;

	m_pkParentFurniture = NULL;
	m_iParentIndex = 1;
}

PgFurniture::~PgFurniture()
{
	m_kTextureContainer.clear();
	m_kMeshRoot = NULL;
	m_pkItemDef = NULL;
}

bool PgFurniture::Initialize()
{
	if (m_kMeshPath.empty() && GetActorManager() == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgFurniture] %s furniture has no mesh\n", m_kID);
		return false;
	}

	if (m_kMeshPath.empty() == false && m_kMeshRoot == NULL)
	{
		m_kMeshRoot = g_kNifMan.GetNif(m_kMeshPath);
	}

	if (m_kMeshRoot == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgFurniture] can't load %s furniture, %s nif\n", m_kID.c_str(), m_kMeshPath.c_str());
		return false;
	}

	AttachChild(m_kMeshRoot, true);

	bool bRet = TexturePair::ApplyTexture(m_kMeshRoot, m_kTextureContainer);
	
	UpdateBound();

	// PhysX는 나중에 업데이트 한다.
	NiAVObject* pkRoot = (NiAVObject*)this;
	if (pkRoot)
		pkRoot->IncRefCount();	// 이걸 안하면 Detach를 할 때 삭제가 되어 버린다.

	return bRet;
}

void PgFurniture::Release()
{
	ReleasePhysX();
}
NiAVObject* PgFurniture::GetNIFRoot() const
{
	return (NiAVObject*)this;
}

bool PgFurniture::Update(float fAccumTime, float fFrameTime)
{
	PgIWorldObject::Update(fAccumTime, fFrameTime);
	//if (m_kMeshRoot)
	//	m_kMeshRoot->Update(fAccumTime);
	return true;
}

void PgFurniture::InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup)
{
	PG_ASSERT_LOG(pkPhysXScene);

//	NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
//	if (pkPhysXScene == NULL || pkPhysXManager == NULL)
//		return;
//
	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		GetWorld()->LockPhysX(true);
	}
//
//	NxScene* pkNxScene = pkPhysXScene->GetPhysXScene();
//#ifdef PG_USE_CAPSULE_CONTROLLER
//
//	NxCapsuleControllerDesc kCtrlDesc;
//	float fHeight = PG_CHARACTER_CAPSULE_HEIGHT;
//	float fRadius = PG_CHARACTER_CAPSULE_RADIUS;
//	NiPoint3 kLoc = GetTranslate();
//	kCtrlDesc.position.x = kLoc.x;
//	kCtrlDesc.position.y = kLoc.y;
//	kCtrlDesc.position.z = kLoc.z;
//	kCtrlDesc.radius = fRadius;
//	kCtrlDesc.height = fHeight;
//	kCtrlDesc.upDirection = NX_Z;
//	kCtrlDesc.slopeLimit = cosf(NxMath::degToRad(PG_LIMITED_ANGLE + 10.0f));
//	kCtrlDesc.skinWidth = 0.1f;
//	kCtrlDesc.stepOffset = PG_CHARACTER_CAPSULE_RADIUS * 2.0f;
//#else
//	NxBoxControllerDesc kCtrlDesc;
//	float fHeight = PG_CHARACTER_CAPSULE_HEIGHT + 10.0f;
//	float fRadius = PG_CHARACTER_CAPSULE_RADIUS - 5.0f;
//	NiPoint3 kLoc = GetTranslate();
//	kCtrlDesc.position.x = kLoc.x;
//	kCtrlDesc.position.y = kLoc.y;
//	kCtrlDesc.position.z = kLoc.z;
//	kCtrlDesc.extents = NxVec3(7.5f, 25.0f , 7.5f);
//	kCtrlDesc.upDirection = NX_Z;
//	kCtrlDesc.slopeLimit = cosf(NxMath::degToRad(30.0f));
//	kCtrlDesc.skinWidth = 0.1f;
//	kCtrlDesc.stepOffset = fRadius;
//#endif
//
//#ifdef PG_USE_CAPSULE_CONTROLLER
//	m_pkController = (NxCapsuleController *)g_kControllerManager.createController(pkNxScene, kCtrlDesc);
//#else
//	m_pkController = (NxBoxController *)g_kControllerManager.createController(pkNxScene, kCtrlDesc);
//#endif
//	m_pkController->setInteraction(NXIF_INTERACTION_EXCLUDE);
//	m_pkController->setPosition(NxExtendedVec3(kLoc.x, kLoc.y, kLoc.z));
//
//	NxMat33 kMat;
//	NiPhysXTypes::NiQuaternionToNxMat33(NiQuaternion(NI_HALF_PI, NiPoint3::UNIT_X), kMat);
//
//	m_pkPhysXActor = m_pkController->getActor();
//	m_pkPhysXActor->setCMassOffsetLocalOrientation(kMat);
//	m_pkPhysXActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
//	
//	//SetRotation(NiQuaternion::IDENTITY);
//
//	NxShape *pkShape = m_pkPhysXActor->getShapes()[0];
//	pkShape->setLocalOrientation(kMat);
//	pkShape->setLocalPosition(NxVec3(0, 0, 0));
//	pkShape->setFlag(NX_SF_DISABLE_COLLISION, true); 
//	pkShape->setFlag(NX_SF_DISABLE_RAYCASTING, true);
//
//	//GetNIFRoot()->SetTranslate(NiPoint3(0, 0, -PG_CHARACTER_Z_ADJUST));
//	
//		
//	// Gamebryo --> PhysX 동기자를 생성한다.
//	m_pkPhysXSrc = NiNew NiPhysXKinematicSrc(this, m_pkPhysXActor);
//	m_pkPhysXSrc->SetActive(false);
//	m_pkPhysXSrc->SetInterpolate(false);
//	pkPhysXScene->AddSource(m_pkPhysXSrc);
//
//	// PhysX --> Gamebryo 동기자를 생성한다.
//	m_pkPhysXDest = NiNew NiPhysXTransformDest(this, m_pkPhysXActor, 0);
//	m_pkPhysXDest->SetActive(true);
//	m_pkPhysXDest->SetInterpolate(false);
//	pkPhysXScene->AddDestination(m_pkPhysXDest);

	NiStream kStream;
	//if(kStream.Load("E:/work/Dragonica_Exe/SFreedom_Dev/Data/3_World/97_Myhome/02_Modeling/01_Object/OB001_0101.nif"))
	if(kStream.Load(m_kMeshPath.c_str()))
	{
		//int iCount = m_kMeshRoot->GetChildCount();
		int iCount = kStream.GetObjectCount();
		for(int i=1;i<iCount;i++)
		{
			//NiAVObject* pkObject = m_kMeshRoot->GetAt(i);
			NiObject *pkObject = kStream.GetObjectAt(i);
			if(NiIsKindOf(NiPhysXScene,pkObject))
			{
				NiPhysXScenePtr spPhysXSceneObj = NiDynamicCast(NiPhysXScene,pkObject);

				if(spPhysXSceneObj->GetSnapshot())
				{
					NiPhysXSceneDesc* pkDesc = spPhysXSceneObj->GetSnapshot();

					// 중복 네임이 있으면 피직스가 잘못 먹히기 때문에 강제로 이름을 바꾸어 준다.
					int iActorTotal = pkDesc->GetActorCount();
					for (int iActorCount=0 ; iActorCount<iActorTotal ; ++iActorCount)
					{
						NiPhysXActorDesc *pkActorDesc = pkDesc->GetActorAt(iActorCount);
						int iShapeTotal = pkActorDesc->GetActorShapes().GetSize();
						for (int iShapeCount=0 ; iShapeCount<iShapeTotal ; iShapeCount++)
						{
							NiPhysXShapeDesc *pkShapeDesc =
								pkActorDesc->GetActorShapes().GetAt(iShapeCount);

							// Rename PhysX Object
							if (pkShapeDesc->GetMeshDesc())
							{
								BM::GUID kGuid("123");
								kGuid.Generate();
								NiString strDescName = MB(kGuid.str());
								//NiString strDescName = GetID().c_str();
								strDescName += "_";
								char szCount[256];
								_itoa_s(iActorCount, szCount, 10);
								strDescName += szCount;
								strDescName += "_";
								_itoa_s(iShapeCount, szCount, 10);
								strDescName += szCount;
								strDescName += "_";
								//strDescName += strPath.c_str();
								NiFixedString strDescName_ = strDescName.MakeExternalCopy();
								pkShapeDesc->GetMeshDesc()->SetName(strDescName_);
							}
						}
					}
				}

				NxMat34 kSlaveMat;
				NiMatrix3 kPhysXRotMat;
				kPhysXRotMat.MakeIdentity();
				NiPoint3 kPhysXTranslation = GetTranslate();
				NiPhysXTypes::NiTransformToNxMat34(kPhysXRotMat, kPhysXTranslation, kSlaveMat);

				spPhysXSceneObj->SetSlaved(pkPhysXScene, kSlaveMat);
				spPhysXSceneObj->CreateSceneFromSnapshot(0);

				unsigned	int	iSourceNum = spPhysXSceneObj->GetSourcesCount();
				for (unsigned int iSrcCount=0 ; iSrcCount< iSourceNum ; iSrcCount++)
				{
					NiPhysXSrc *pkPhysXSrc = spPhysXSceneObj->GetSourceAt(iSrcCount);

					NiPhysXKinematicSrc *pkPhysXKinematicSrcOrg = NiDynamicCast(NiPhysXKinematicSrc,pkPhysXSrc);
					if(!pkPhysXKinematicSrcOrg)
					{
						_PgOutputDebugString("PgObject::InitPhysX Name[%s] No pkPhysXKinematicSrcOrg\n",GetID().c_str());
						continue;
					}
					NiAVObject	*pkGBSource = GetObjectByName(pkPhysXKinematicSrcOrg->GetSource()->GetName());
					if(!pkGBSource)
					{
						_PgOutputDebugString("PgObject::InitPhysX Name[%s] No pkGBSource\n",GetID().c_str());
						continue;
					}

					//NxActor	*pkTarget = pkPhysXKinematicSrcOrg->GetTarget();
					m_pkPhysXActor = pkPhysXKinematicSrcOrg->GetTarget();
					if(!m_pkPhysXActor)
					{
						_PgOutputDebugString("PgObject::InitPhysX Name[%s] No m_pkPhysXActor\n",GetID().c_str());
						continue;
					}

					NxShape *const *pkShapes = m_pkPhysXActor->getShapes();
					int	iNumShapes = m_pkPhysXActor->getNbShapes();

					for(int k=0;k<iNumShapes;k++)
					{

						NxShape	*pkShape = *pkShapes;

						pkShape->setGroup(uiGroup+1);
						pkShape->userData = this;

						pkShapes++;
					}

					m_pkPhysXActor->setGroup(uiGroup+1);
					m_pkPhysXActor->userData = this;

					// Gamebryo --> PhysX 동기자를 생성한다.
					m_pkPhysXSrc = NiNew NiPhysXKinematicSrc(pkGBSource, m_pkPhysXActor);
					m_pkPhysXSrc->SetActive(true);
					m_pkPhysXSrc->SetInterpolate(false);
					pkPhysXScene->AddSource(m_pkPhysXSrc);

					//// Gamebryo --> PhysX 동기자를 생성한다.
					//m_pkPhysXSrc = NiNew NiPhysXKinematicSrc(this, m_pkPhysXActor);
					//m_pkPhysXSrc->SetActive(false);
					//m_pkPhysXSrc->SetInterpolate(false);
					//pkPhysXScene->AddSource(m_pkPhysXSrc);

					// PhysX --> Gamebryo 동기자를 생성한다.
					//m_pkPhysXDest = NiNew NiPhysXTransformDest(pkGBSource, m_pkPhysXActor, 0);
					//m_pkPhysXDest->SetActive(true);
					//m_pkPhysXDest->SetInterpolate(false);
					//m_pkPhysXDest->SetOptimizeSleep(false);
					//pkPhysXScene->AddDestination(m_pkPhysXDest);
				}
			}
		}
	}

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		GetWorld()->LockPhysX(false);
	}
	m_pkPhysXScene = pkPhysXScene;
}

void PgFurniture::ReleasePhysX()
{
	if(m_pkPhysXScene && m_pkPhysXActor)
	{
		PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.ReleasePhysX"), g_pkApp->GetFrameCount()));
		NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
		if (GetWorld())
		{
			PG_STAT(PgStatTimerF timerA(g_kMobileSuitStatGroup.GetStatInfo("PhysX.WaitSDKLock"), g_pkApp->GetFrameCount()));
			PG_STAT(timerA.Start());
			GetWorld()->LockPhysX(true);
			PG_STAT(timerA.Stop());
		}

		m_pkPhysXScene->DeleteDestination(m_pkPhysXDest);
		m_pkPhysXScene->DeleteSource(m_pkPhysXSrc);
		//if (m_pkPhysXScene->GetPhysXScene())
		//	m_pkPhysXScene->GetPhysXScene()->releaseActor(*m_pkPhysXActor);	// dukguru assert : double deletion
		
		//for(int i = 0; i < PG_MAX_NB_ABV_SHAPES && m_apkPhysXCollisionActors[i]; ++i)
		//{
		//	m_pkPhysXScene->DeleteSource(m_apkPhysXCollisionSrcs[i]);
		//	if (m_pkPhysXScene->GetPhysXScene())
		//	{
		//		m_pkPhysXScene->GetPhysXScene()->releaseActor(*m_apkPhysXCollisionActors[i]);
		//	}
		//	m_apkPhysXCollisionActors[i] = 0;
		//}
		
		//if(m_pkController)
		//{
		//	g_kControllerManager.releaseController(*m_pkController);
		//}

		if(m_pkPhysXActor && m_pkPhysXScene->GetPhysXScene())
		{
			m_pkPhysXScene->GetPhysXScene()->releaseActor(*m_pkPhysXActor);
		}

		if (GetWorld())
		{
			GetWorld()->LockPhysX(false);
		}
	}

	m_pkPhysXScene = 0;
	m_pkPhysXActor = 0;
	//m_pkController = 0;
}

bool PgFurniture::ProcessAction(PgAction *pkAction,bool bInvalidateDirection,bool bForceToTransit)
{
	return true;
}

NiObject* PgFurniture::CreateClone(NiCloningProcess& kCloning)
{
    PgFurniture* pkClone = NiNew PgFurniture;
    CopyMembers(pkClone , kCloning);
    return pkClone;
}

void PgFurniture::ProcessClone(NiCloningProcess& kCloning)
{
	PgIWorldObject::ProcessClone(kCloning);

	NiObject *pkClone = 0;
	
	bool bCloned = kCloning.m_pkCloneMap->GetAt(this, pkClone);
	PgFurniture *pkDest = 0;

	if(bCloned)
	{
		pkDest = (PgFurniture *) pkClone;
	}
	else
	{
		pkDest = this;
	}

	pkDest->m_pkItemDef = this->m_pkItemDef;
	pkDest->m_eFurnitureType = this->m_eFurnitureType;
	pkDest->m_kNormal = this->m_kNormal;
	pkDest->m_fRotation = this->m_fRotation;
	pkDest->m_iRotation = this->m_iRotation;
	pkDest->m_kBound = this->m_kBound;
	pkDest->m_kCellSize = this->m_kCellSize;
	pkDest->m_kCellCenter = this->m_kCellCenter;
	pkDest->m_kMeshPath = this->m_kMeshPath;
	pkDest->m_kTextureContainer = this->m_kTextureContainer;
	pkDest->m_kMeshRoot = this->m_kMeshRoot;
	pkDest->m_kXMLPath = this->m_kXMLPath;

	// Copy Members
	/// 이거 고치다 만거임, 지우지 말것
	//PG_ASSERT_LOG(m_spAM);
	//if (m_spAM)
	//{
	//	pkDest->m_spAM = m_spAM->Clone();
	//	pkDest->AttachChild(pkDest->m_spAM->GetNIFRoot());
	//}
}

void PgFurniture::SetNormal(NiPoint3 const &rkNormal)
{
	//NiQuaternion kNormalQuat(NiACos(NiPoint3::UNIT_Z.Dot(rkNormal)), NiPoint3::UNIT_Z.UnitCross(rkNormal));
	//NiQuaternion kRotateQuat(m_fRotation, NiPoint3::UNIT_Z);
	//SetRotate(kNormalQuat * kRotateQuat);
	
	m_kNormal = rkNormal;
}

void PgFurniture::IncRotate(float fAngle)
{
	m_fRotation += fAngle;
	SetNormal(m_kNormal);
}

void PgFurniture::SetRotation(int iRotation)
{
	if(FURNITURE_TYPE_OBJECT != GetFurnitureType() && FURNITURE_TYPE_OBJECT_ATTACH != GetFurnitureType())
	{
		return;
	}
	//m_fRotation = iRotation * ROTATION_ANGLE + (NI_HALF_PI*0.5f);
	m_fRotation = iRotation * ROTATION_ANGLE;
	m_iRotation = iRotation;

	NiTransform kTrn = GetWorldTransform();
	NiQuaternion kRot = NiQuaternion::IDENTITY;
	NiQuaternion kNormalQuat(NiACos(NiPoint3::UNIT_Z.Dot(m_kNormal)), NiPoint3::UNIT_Z.UnitCross(m_kNormal));
	NiQuaternion kRotateQuat(m_fRotation, NiPoint3::UNIT_Z);
	kRot = kNormalQuat * kRotateQuat;
	kRot.ToRotation(kTrn.m_Rotate);
	//NiQuaternion kNormalQuat(NiACos(NiPoint3::UNIT_Z.Dot(m_kNormal)), NiPoint3::UNIT_Z.UnitCross(m_kNormal));
	//NiQuaternion kRotateQuat(m_fRotation, NiPoint3::UNIT_Z);
	//SetRotate(kNormalQuat * kRotateQuat);
	//NiTransform kTrn = GetLocalTransform();



	//NiTransform kTrn;
	//kTrn.MakeIdentity();

	//NiQuaternion kRot = NiQuaternion::IDENTITY;
	//kRot.FromAngleAxis(ROTATION_ANGLE*m_iRotation, m_kNormal);
	//kRot.ToRotation(kTrn.m_Rotate);

	//if(m_pkPhysXActor)
	//{
	//	NxQuat kNxQuat;
	//	NiPhysXTypes::NiQuaternionToNxQuat(kRot, kNxQuat);
	//	m_pkPhysXActor->setGlobalOrientationQuat(kNxQuat);
	//}

	SetLocalFromWorldTransform(kTrn);
}

bool PgFurniture::SetPosition(NiPoint3 const &rkTranslate)
{
	//if(!m_pkController)
	//{
	//	return false;
	//}

	/// Comment : PhysX동기자가 다음 업데이트 시에 Gamebryo Object와 좌표를 맞춰 주기 때문에 
	///	그 프레임에 GetTranslate()을 하면 좌표가 한 프레임 어긋난다. 때문에 AVObject::SetTranslate()을 같이 해준다. 

	//m_pkController->setPosition(NxExtendedVec3(rkTranslate.x, rkTranslate.y, rkTranslate.z));

	if(m_pkPhysXActor)
	{
		NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();

		if (GetWorld() && g_iUseAddUnitThread == 1)
		{
			GetWorld()->LockPhysX(true);
		}

		//m_pkPhysXActor->setGlobalPosition(NxVec3(rkTranslate.x, rkTranslate.y, rkTranslate.z));
		m_pkPhysXActor->moveGlobalPosition(NxVec3(rkTranslate.x, rkTranslate.y, rkTranslate.z));

		//NxShape *pkShape = m_pkPhysXActor->getShapes()[0];
		//pkShape->setGlobalPosition(NxVec3(rkTranslate.x, rkTranslate.y, rkTranslate.z));

		//for(int i = 0; GetABVShape(i)->IsValid() && i < PG_MAX_NB_ABV_SHAPES; i++)
		//{
		//	if(m_apkPhysXCollisionActors[i])
		//	{
		//		PgIWorldObjectBase::ABVShape *pkABVShape = GetABVShape(i);
		//		NiAVObject *pkTarget = GetObjectByName(GetABVShape(i)->m_kTo);
		//		if(pkTarget)
		//		{
		//			NxVec3 kTargetPos;
		//			NiAVObject *pkTarget = GetObjectByName(GetABVShape(i)->m_kTo);
		//			NiPhysXTypes::NiPoint3ToNxVec3(pkTarget->GetWorldTranslate(), kTargetPos);
		//			m_apkPhysXCollisionActors[i]->setGlobalPosition(kTargetPos);
		//		}
		//	}
		//}

		if (GetWorld() && g_iUseAddUnitThread == 1)
		{
			GetWorld()->LockPhysX(false);
		}

	}

	return true;
}

bool PgFurniture::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	TiXmlElement *pkFirstElement = (TiXmlElement *)pkNode;
	PG_ASSERT_LOG(pkFirstElement);
	if (pkFirstElement == NULL)
		return false;

	if (pArg == NULL)
		return false;

	m_pkItemDef = (CItemDef*)pArg;
	m_eFurnitureType = (EFurnitureType)m_pkItemDef->GetAbil(AT_FURNITURE_TYPE);
	if (m_eFurnitureType <= FURNITURE_TYPE_NONE || m_eFurnitureType > FURNITURE_TYPE_ETC)
	{
		m_eFurnitureType = FURNITURE_TYPE_NONE;
		return false;
	}

	const char *pcFirstTagName = pkFirstElement->Value();

	if(strcmp(pcFirstTagName, "FURNITURE") == 0)
	{
		TiXmlAttribute* pkAttr = pkFirstElement->FirstAttribute();
		while(pkAttr)
		{
			const char *pcAttrName = pkAttr->Name();
			const char *pcAttrValue = pkAttr->Value();

			if (strnicmp(pcAttrName, "NAME", strlen("NAME")) == 0)
			{
				m_kID = pcAttrValue;
			}
			else if (strnicmp(pcAttrName, "ATTACH_TO", strlen("ATTACH_TO")) == 0)
			{
			}
			else if (strnicmp(pcAttrName, "TYPE", strlen("TYPE")) == 0)
			{
			}
			else
			{
				PgError1("PgFurniture : Unknown attribute - %s", pcAttrName);
			}

			pkAttr = pkAttr->Next();
		}
	}
	else
	{
		PgError1("%s xml has no FURNITURE Attribute!", pkNode->GetDocument()->Value());
		return false;
	}

	TiXmlElement *pkElement = pkFirstElement->FirstChildElement();

	while(pkElement)
	{
		const char *pcTagName = pkElement->Value();
		if (strcmp(pcTagName, "MESH") == 0)
		{
			m_kMeshPath = pkElement->GetText();
			TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
			if(pkAttr)
			{
				const char *pcAttrName = pkAttr->Name();
				const char *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "SCALE") == 0)
				{
				}
			}
		}
		else if(strcmp(pcTagName, "TEXTURE") == 0)
		{
			// TODO : 텍스쳐풀에서 가져와야 함
			const char *pcSrc = 0;
			const char *pcDest = 0;

			TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				const char *pcAttrName = pkAttr->Name();
				const char *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "SRC") == 0)
				{
					pcSrc = pcAttrValue;
					pcDest = pkElement->GetText();
				}
				else
				{
					PgError1("PgItemEx - Unknown tag : %s", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			if(pcSrc && pcDest)
			{
				NiString kSrc(pcSrc);
				NiString kDest(pcDest);

				kSrc.ToLower();
				kDest.ToLower();

				TexturePair kTexturePair(kSrc, kDest);
				kTexturePair.m_pkDestTexture = g_kNifMan.GetTexture(kTexturePair.m_kDestTexture);
				m_kTextureContainer.push_back(kTexturePair);
			}
		}
		else
		{
			PgError1("PgFurniture - Unknown tag : %s", pcTagName);
			break;
		}

		pkElement = pkElement->NextSiblingElement();
	}

	return Initialize();
}

void PgFurniture::UpdateBound()
{
	NiAVObject* pkRoot = m_kMeshRoot;

	NiPoint3 kMinPoint = NiPoint3::ZERO;
	NiPoint3 kMaxPoint = NiPoint3::ZERO;

	getMinMaxPoint(kMinPoint, kMaxPoint, pkRoot);

	if (kMinPoint == NiPoint3::ZERO || kMaxPoint == NiPoint3::ZERO)
		return;

	if (kMinPoint.x == kMaxPoint.x || kMinPoint.y == kMaxPoint.y || kMinPoint.z == kMaxPoint.z)
	{
		NILOG(PGLOG_WARNING, "[PgFurniture] %s furniture has some error(%f,%f,%f)(%f,%f,%f)\n", GetID().c_str(), kMinPoint.x, kMinPoint.y, kMinPoint.z, kMaxPoint.x, kMaxPoint.y, kMaxPoint.z);
		return;
	}

	if ((kMinPoint.x > 0 && kMinPoint.y > 0 && kMinPoint.z > 0) || (kMaxPoint.x < 0 && kMaxPoint.y < 0 && kMaxPoint.z < 0))
	{
		NILOG(PGLOG_WARNING, "[PgFurniture] %s furniture has odd align(%f,%f,%f)(%f,%f,%f)\n", GetID().c_str(), kMinPoint.x, kMinPoint.y, kMinPoint.z, kMaxPoint.x, kMaxPoint.y, kMaxPoint.z);
	}

	if (kMinPoint.z < 0)
	{
		NILOG(PGLOG_WARNING, "[PgFurniture] %s furniture has odd z(%f,%f,%f)(%f,%f,%f)\n", GetID().c_str(), kMinPoint.x, kMinPoint.y, kMinPoint.z, kMaxPoint.x, kMaxPoint.y, kMaxPoint.z);
	}

	//calc cell size
	NiPoint3 kCenter = PgInterpolator::Lerp(kMinPoint, kMaxPoint, 0.5f);

	m_kCellCenter.x = (int)(ceil(NiAbs(kMinPoint.x) / WALL_CELL_UNIT_SIZE));
	m_kCellCenter.y = (int)(ceil(NiAbs(kMinPoint.y) / WALL_CELL_UNIT_SIZE));
	m_kCellCenter.z = (int)(ceil(NiAbs(kMinPoint.z) / WALL_CELL_UNIT_SIZE));

	m_kCellSize.x = m_kCellCenter.x + (int)(ceil(NiAbs(kMaxPoint.x) / WALL_CELL_UNIT_SIZE));
	m_kCellSize.y = m_kCellCenter.x + (int)(ceil(NiAbs(kMaxPoint.y) / WALL_CELL_UNIT_SIZE));
	m_kCellSize.z = m_kCellCenter.x + (int)(ceil(NiAbs(kMaxPoint.z) / WALL_CELL_UNIT_SIZE));
	
	if (NiSign(kMinPoint.x) == NiSign(kMaxPoint.x))
	{
		m_kCellSize.x -= (m_kCellCenter.x * -2);
		m_kCellCenter.x += (m_kCellSize.x / 2);
	}

	if (NiSign(kMinPoint.y) == NiSign(kMaxPoint.y))
	{
		m_kCellSize.y -= (m_kCellCenter.y * -2);
		m_kCellCenter.y += (m_kCellSize.y / 2);
	}


}

void PgFurniture::getMinMaxPoint(NiPoint3& rkMinPoint, NiPoint3& rkMaxPoint, NiAVObject* pkObject)
{
	if (pkObject == NULL)
		return;

	if (NiIsKindOf(NiTriBasedGeom, pkObject))
	{
		NiTriBasedGeom* pkGeometry = (NiTriBasedGeom*)pkObject;
		NiPoint3 *pVerticies = pkGeometry->GetVertices();
		if (pVerticies == NULL)
			return;

		unsigned int vertexCount = pkGeometry->GetVertexCount();
		NiGeometryData *pGeometryData = pkGeometry->GetModelData();
		calcVertexMinMaxPoint(vertexCount, pVerticies, rkMinPoint, rkMaxPoint);
	}
	else if(NiIsKindOf(NiNode, pkObject))
	{
		NiNode* pkNode = (NiNode*)pkObject;
		for(unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				getMinMaxPoint(rkMinPoint, rkMaxPoint, pkChild);
			}
		}
	}
}

void PgFurniture::calcVertexMinMaxPoint(int iVertexCount, const NiPoint3* pkData, NiPoint3& rkMinPoint, NiPoint3& rkMaxPoint)
{
	if (iVertexCount <= 0)
		return;

	NiPoint3 kMin = pkData[0];
	NiPoint3 kMax = kMin;
	int i;
	for (i = 1; i < iVertexCount; i++) 
	{
		if (kMin.x > pkData[i].x)
			kMin.x = pkData[i].x;
		if (kMin.y > pkData[i].y)
			kMin.y = pkData[i].y;
		if (kMin.z > pkData[i].z)
			kMin.z = pkData[i].z;
		if (kMax.x < pkData[i].x)
			kMax.x = pkData[i].x;
		if (kMax.y < pkData[i].y)
			kMax.y = pkData[i].y;
		if (kMax.z < pkData[i].z)
			kMax.z = pkData[i].z;
	}

	rkMinPoint = kMin;
	rkMaxPoint = kMax;
}

PgFurniture* PgFurniture::GetFurnitureFromDef(int iFurnitureNo)
{
	PgFurniture* pkFurniture = NULL;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(iFurnitureNo);
	if(!pkItemDef)
	{
		NILOG(PGLOG_ERROR, "[PgFurniture] GetFurnitureFromDef, Can't Find %d Item from defmgr\n", iFurnitureNo);
		return NULL;
	}

	int iFurnitureType = pkItemDef->GetAbil(AT_FURNITURE_TYPE);
	if (iFurnitureType == 0)
	{
		NILOG(PGLOG_ERROR, "[PgFurniture] GetFurnitureFromDef, %d Item is not furniture\n", iFurnitureNo);
		return NULL;
	}

	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);

	CONT_DEFRES::const_iterator itrDest = pContDefRes->find(pkItemDef->ResNo());
	if(itrDest == pContDefRes->end())
	{
		NILOG(PGLOG_ERROR, "[PgFurniture] GetFurnitureFromDef, can't find %d Item's res\n", iFurnitureNo);
		return NULL;
	}

	// 아이템을 생성한다.
	std::string xmlPath = MB(itrDest->second.strXmlPath);

	PG_WARNING_LOG(xmlPath.length() == 0);

	if(xmlPath.length() <= 1)	//	경로 길이가 너무 작으면 비정상적인 경로로 가정하고 리턴한다.
	{
		return	NULL;
	}

	pkFurniture = NiNew PgFurniture();
	pkFurniture->SetFurnitureType((EFurnitureType)iFurnitureType);
	pkFurniture->SetXMLPath(xmlPath.c_str());

	pkFurniture = static_cast<PgFurniture *>(PgXmlLoader::CreateObjectFromFile(xmlPath.c_str(), (void *)pkItemDef, pkFurniture));
	PG_ASSERT_LOG(pkFurniture != NULL);
	if (pkFurniture == NULL)
	{
		NILOG(PGLOG_ERROR, "[PgFurniture] GetFurnitureFromDef, can't create %d item, paht(%s)\n", iFurnitureNo, xmlPath.c_str());
	}

	return pkFurniture;
}

void PgFurniture::DeleteFurniture(PgFurniture* pkFurniture)
{
	if (pkFurniture == NULL)
		return;

	pkFurniture->Release();
	NiDelete pkFurniture;
}

bool PgFurniture::IsFurnitureMesh(NiAVObject* pkObject, PgFurniture*& rkFurniture)
{
	if (pkObject == NULL)
		return false;

	NiNode* pkParent = (NiNode*)pkObject;
	while (pkParent)
	{
		if (NiIsKindOf(PgFurniture, pkParent))
		{
			PgFurniture* pkFurniture = NiDynamicCast(PgFurniture, pkParent);
			CItemDef* pItemDef = pkFurniture->GetItemDef();
			if( pItemDef && pItemDef->EquipPos() != EQUIP_POS_NONE )
			{
				return false;
			}
			else if( pkFurniture->GetFurnitureType() != FURNITURE_TYPE_WALL && pkFurniture->GetFurnitureType() != FURNITURE_TYPE_FLOOR )
			{
				rkFurniture = pkFurniture;
				return true;
			}
		}

		pkParent = pkParent->GetParent();
	}

	return false;
}

int PgFurniture::GetFurnitureDetailType()
{
	//if( m_pkItemDef )
	//{
	//	return m_pkItemDef->GetAbil(AT_FURNITURE_DETAIL_TYPE);
	//}

	return 0;
}

bool PgFurniture::SetPhysxTriggerFlag(bool bSet)
{
	NxShape *const *pkShapes = m_pkPhysXActor->getShapes();
	if( !pkShapes )
	{
		return false;
	}

	int	iNumShapes = m_pkPhysXActor->getNbShapes();

	for(int k=0;k<iNumShapes;k++)
	{

		NxShape	*pkShape = *pkShapes;
		pkShape->setFlag(NX_TRIGGER_ENABLE, bSet);

		pkShapes++;
	}
	
	return true;
}

void PgFurniture::SetFurnitureColor(const NiColor& kColor)
{
	NiColorA kGeomColor = GetColorLocal();
	kGeomColor.r = kColor.r;
	kGeomColor.g = kColor.g;
	kGeomColor.b = kColor.b;

	SetColorLocal(kGeomColor);
}

char const *PgFurniture::GetMeshPath()const
{
	return m_kMeshPath.c_str();
}