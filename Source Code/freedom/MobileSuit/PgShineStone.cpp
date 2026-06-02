#include "StdAfx.h"
#include "PgActor.h"
#include "PgWorld.h"
#include "PgMobileSuit.h"
#include "PgPilotMan.h"
#include "Variant/Constant.h"
#include "PgNetwork.h"
#include "PgCircleShadow.h"
#include "lwUI.h"
#include "Variant/Inventory.h"
#include "lwActor.h"
#include "PgParticleMan.h"
#include "PgRenderer.h"
#include "PgNifMan.h"
#include "PgMath.h"
#include "PgSoundMan.h"
#include "PgShineStone.h"

#include "NewWare/Scene/ApplyTraversal.h"


int const PG_SHINESTONE_FIRST_IDX = 78;
float const CHASE_TIME = 0.4f;
float const GET_TIME = CHASE_TIME*0.99f;
float const MAX_SHINESTONE_SHADOW_DISTANCE = 10000.0f;

NiImplementRTTI(PgShineStone, PgIWorldObject);

PgShineStone::PgShineStone()
{
	m_iShineStoneTitleTextID = 0;

	m_ptTitleTextAdjust = NiPoint3(0,0,0);
	m_kShineStoneTitleTextScale = 1.0;
	m_kShineStoneTitleTextColor = NiColorA(1,1,1,1);

	m_iIndex = -1;
	m_kPos = NiPoint3(0,0,0);
	m_fRate = 0;
	m_pkDetailGeo = 0;
	
	// ShineStone Nif	// 깨지기 전
	std::string strNifPath = "../Data/3_World/Palettes/ShineStone/ShineStone.nif";
	SNifScene kNifScene = g_kNifMan.GetNewNifScene(strNifPath);
	m_spShineStone = kNifScene.spNifNode;
	m_spShineStonePhysX = kNifScene.spPhysXScene;
	if (m_spShineStonePhysX)
	{
	}
	else
	{
		NILOG(PGLOG_ERROR, "Can't Get ShineStonePhysX\n");
	}

	AttachChild(kNifScene.spNifNode, true);
	SetPhysXObj(kNifScene.spPhysXScene);
	GetWorld()->GetAllGeometries(m_spShineStone, m_kShineStoneGeoList);

	// Detail			// 깨진후.
	std::string strDetailNifPath = "../Data/5_Effect/4_UI/ef_shinestone_01.nif";
	m_spDetailStone = g_kNifMan.GetNif_DeepCopy(strDetailNifPath);
	AttachChild(m_spDetailStone, true);

	// Particle은 따로 땐다.
	NiObjectList kNodeList;
	PgParticleMan::GetAllParticleNode(m_spDetailStone, kNodeList);
	NiTListIterator itr = kNodeList.GetHeadPos();
	
	m_spParticleNode = NiNew NiNode();
	while(kNodeList.GetSize() > 0)
	{
		NiParticleSystemPtr pkPar = NiDynamicCast(NiParticleSystem, kNodeList.Get(itr));
		if (pkPar)
		{
			m_spParticleNode->AttachChild(pkPar->GetParent(), true);
		}
		break;
	}
	AttachChild(m_spParticleNode, true);

	NewWare::Scene::ApplyTraversal::Property::SetAlphaGroup( this, AG_EFFECT );

	Initialize();
}

PgShineStone::~PgShineStone()
{
	Terminate();
}

void PgShineStone::SetPos(POINT3 const &ptPos)
{
	m_kPos.x = ptPos.x;
	m_kPos.y = ptPos.y;
	m_kPos.z = ptPos.z;
}

bool PgShineStone::Initialize()
{
	m_spShineStone->SetAppCulled(false);
	m_spDetailStone->SetScale(1);
	m_spDetailStone->SetAppCulled(true);

	Enable(true);
	m_eStoneState = E_STONE_NONE;
	m_fSinProgress = 0.0f;
	m_bChaseOwner = false;
	m_fChaseStartTime = 0.0f;

	m_bHideShadow = false;

	SetTranslate(m_kPos);
	SetScale(1.0f);

	m_iItemNo = 0;

	NiTimeController::StartAnimations(m_spShineStone, 0.0f);
	NiTimeController::StartAnimations(m_spDetailStone, 0.0f);

	PgParticleMan::ChangeParticleGeneration(m_spParticleNode, true);

	PgCircleShadow::AttachCircleShadowRecursive(NiDynamicCast(NiAVObject,m_spShineStone),
		MAX_SHINESTONE_SHADOW_DISTANCE,
		1.0f,NULL,NiDynamicCast(NiAVObject,m_spShineStone));

	return true;
}

void PgShineStone::Terminate()
{
	m_spShineStone = 0;
	m_spDetailStone = 0;
	m_spPhysXObject = 0;
	m_spParticleNode = 0;
}

std::string &PgShineStone::GetConditionAction()
{
	return m_kConditionAction;
}

void GetAllBillboardNode(const NiNode *pkNode, NiObjectList &kBillboardNode)
{
	for(unsigned int i = 0;
		i < pkNode->GetArrayCount();
		i++)
	{	
		NiAVObject *pkChild = pkNode->GetAt(i);
		if (!pkChild)
		{
			continue;
		}

		if(NiIsKindOf(NiBillboardNode, pkChild))
		{
			kBillboardNode.AddTail(pkChild);
		}
		if(NiIsKindOf(NiNode, pkChild))
		{
			GetAllBillboardNode(NiDynamicCast(NiNode, pkChild), kBillboardNode);
		}
	}
}

bool PgShineStone::OnEnter(PgActor *pkActor)
{
	if(!g_pkWorld)
	{
		return	false;
	}

	if(!Enable())	{return false;}
	Enable(false);

	m_bChaseOwner = true;
	m_fChaseStartTime = g_pkWorld->GetAccumTime();

	// Play Particle
	{
		char const *pcTarget = "Scene Root";
		char const *pcParticle = "ef_touch_shinestone";
		int iSlot = 111;
		NiAVObject *pkParticle = g_kParticleMan.GetParticle(pcParticle,PgParticle::O_SCALE, 5.0);
		if(!pkParticle)
			return false;
		if(!AttachTo(iSlot, pcTarget, (NiAVObject *)pkParticle))
		{
			THREAD_DELETE_PARTICLE(pkParticle);
			return	false;
		}
	}

	// 지우지 않고, ENABLE_TRIGGER flag만 끈다. (onTrigger에서)
	//m_spShineStonePhysX->ClearSceneFromSnapshot();

	// Get ShineStone.
	bool bSingleMode = 
#ifndef EXTERNAL_RELEASE
		g_pkApp->IsSingleMode();
#else
		false;
#endif
	if(!bSingleMode)
	{
		// Packet to server
		BM::Stream kPacket(PT_C_M_REQ_TRIGGER);
		kPacket.Push((int)QOT_ShineStone);
		kPacket.Push(GetGuid());
//		kPacket.Push((int)TRIGGER_ACTION_COLLISION);
//		kPacket.Push((BYTE)PlayerStone().iID);
		NETWORK_SEND(kPacket)
	}
	else
	{
		m_iItemNo = 0;
		ChaseOwner();
		// Test Change Model
		{
			ReserveGetShineStone(78 + BM::Rand_Index(5));
		}
	}

	return true;
}

bool PgShineStone::OnUpdate(PgActor *pkActor)
{
	if(!Enable())	{return false;}

	return false;
}

bool PgShineStone::OnLeave(PgActor *pkActor)
{
	if(!Enable())	{return false;}

	return false;
}

bool PgShineStone::OnAction(PgActor *pkActor)
{
	if(!Enable())	{return false;}

	return true;
}

bool PgShineStone::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			assert(pkElement);
			
			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "STONE") == 0)
			{
				char const *pcScript = 0;

				TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "TYPE") == 0)
					{
						if(strcmp(pcAttrValue, "touch") == 0)
						{
							m_eConditionType = CT_TOUCH;
						}
						else if(strcmp(pcAttrValue, "action") == 0)
						{
							m_eConditionType = CT_ACTION;
						}
						else if(strcmp(pcAttrValue, "script") == 0)
						{
							m_eConditionType = CT_SCRIPT;
						}
						else
						{
							PgXmlError1(pkElement, "Incorrect ShineStone Type [%s]", pcAttrValue);
						}
					}
					// ID, RATE, POS를 저장해둔다.
					else if(strcmp(pcAttrName, "ID") == 0)
					{
						m_iIndex = atoi(pcAttrValue);
						std::string strID = SHINESTONE_PREFIX_STRING;
						strID += pcAttrValue;
						SetID(strID.c_str());
					}
					else if(strcmp(pcAttrName, "RATE") == 0)
					{
						m_fRate =  (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "POSX") == 0)
					{
						m_kPos.x = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "POSY") == 0)
					{
						m_kPos.y = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "POSZ") == 0)
					{
						m_kPos.z = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ITEMBAGNO") == 0)
					{
						atoi(pcAttrValue);
					}
					else
					{
						PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
					}
					
					pkAttr = pkAttr->Next();
				}
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
				break;
			}
		}

	default:
		break;
	}

	/*if(!m_pCircleShadow)
	{
		m_pCircleShadow = NiNew PgCircleShadow();
		m_pCircleShadow->SetMaxShadowDistance(MAX_SHINESTONE_SHADOW_DISTANCE);
	}*/

	return true;
}

void PgShineStone::UpdateDownwardPass(float fTime, bool bUpdateControllers)
{
	if(GetWorldTranslate() != NiPoint3::ZERO && ( m_eStoneState == E_STONE_IDLE || m_eStoneState == E_STONE_NONE) && PgRenderer::GetPgRenderer())
	{
		NiCamera	*pkCam = PgRenderer::GetPgRenderer()->GetCameraData();
		if(pkCam)
		{
		
			NiFrustumPlanes	const &kPlanes = PgRenderer::GetPgRenderer()->GetFrustumPlanes();

			NiBound	kBound;
			kBound.SetCenterAndRadius(GetWorldTranslate(),100);

			bool	bVisible = true;
			unsigned int i = 0;
			for (i = 0; i < NiFrustumPlanes::MAX_PLANES; i++)
			{

				int iSide = kBound.WhichSide(
					kPlanes.GetPlane(i));

				if (iSide == NiPlane::NEGATIVE_SIDE)
				{
					// The object is not visible since it is on the negative
					// side of the plane.
					SetAppCulled(true);
					return;
				}
			}			
		}
	}

	SetAppCulled(false);
	NiNode::UpdateDownwardPass(fTime,bUpdateControllers);
}

//#define PG_USE_SHINESTONE_EAT_PATTERN1
//#define PG_USE_SHINESTONE_EAT_PATTERN2
#define PG_USE_SHINESTONE_EAT_PATTERN3
//#define PG_USE_SHINESTONE_EAT_PATTERN4
bool PgShineStone::Update(float fAccumTime, float fFrameTime)
{
	PgIWorldObject::Update(fAccumTime, fFrameTime);
	NiActorManager *pkAM = GetActorManager();
	if (pkAM)
		pkAM->Update(fAccumTime);

#ifdef PG_USE_SHINESTONE_EAT_PATTERN2
	// 위로 약간 올라가는 모션.
	if (m_eStoneState == E_STONE_EAT && m_bChaseOwner && m_fChaseStartTime > 0)
	{
		int const iLength = 40;			// 수치 만큼 위로 더올라간다
		int const iRotate = 360 * 5;	// 수치 만큼 더 돌게 한다.
		NiPoint3 const ptBasePos = m_kPos;
		//NiPoint3 ptStonePos = GetWorldTranslate();
		float fDeltaTime = fAccumTime - m_fChaseStartTime;

		if (fDeltaTime < CHASE_TIME)
		{
			// Translate
			float fPercent = fDeltaTime / CHASE_TIME;	// 0 ~ 1
			float fDeltaPos = iLength * fPercent;
			NiPoint3 ptResultPos = ptBasePos + NiPoint3(0,0,fDeltaPos);
			SetTranslate(ptResultPos);

			// Rotate
			NiMatrix3 kMat = GetRotate();
			NiQuaternion kQuat;
			kQuat.FromRotation(kMat);

			float fDeltaRotate = iRotate * fPercent;
			//float fDeltaRotate = 17.0f;
			kQuat.FromAngleAxisZ(fDeltaRotate);
			NiMatrix3 kResultMat;
			kQuat.ToRotation(kResultMat);

			SetRotate(kResultMat);

			_PgOutputDebugString(":: %f, %f %f \n", fPercent, fDeltaPos, fDeltaRotate);
		}
		else
		{
			SetAppCulled(true);
			m_fChaseStartTime = 0;
		}
	}
#endif

#ifdef PG_USE_SHINESTONE_EAT_PATTERN4
	if (m_eStoneState == E_STONE_EAT && m_bChaseOwner && m_fChaseStartTime > 0)
	{
		if (!g_kPilotMan.GetPlayerActor())
		{
			return false;
		}

		float const fUpLength = 50.0f;
		NiPoint3 const ptBasePos = m_kPos;
		NiPoint3 const ptBaseUpPos = (ptBasePos + NiPoint3(0,0,fUpLength));
		NiPoint3 const ptPlayerPos = g_kPilotMan.GetPlayerActor()->GetTranslate();
		NiPoint3 const ptPlayerUpPos = (ptPlayerPos + NiPoint3(0,0,fUpLength));
		NiPoint3 const ptStonePos = GetTranslate();

		float const fDeltaTime = fAccumTime - m_fChaseStartTime;
		float const fTotalTime = 3.0f;
		float fPercent = fDeltaTime / fTotalTime;
		
		NiPoint3 kResultPos;
		Bezier4(ptBasePos, ptBaseUpPos, ptPlayerUpPos, ptPlayerPos, fPercent, kResultPos);

		SetTranslate(kResultPos);
	}
#endif

#ifdef PG_USE_SHINESTONE_EAT_PATTERN3
	// 빠르게->느리게  올라간다음. 캐릭터 따라 들어감. (확정)
	if (m_eStoneState == E_STONE_EAT && m_bChaseOwner && m_fChaseStartTime > 0)
	{
		if (!g_kPilotMan.GetPlayerActor())
		{
			return false;
		}

		float const fUpTime = 0.4f;
		float const fUpLengthRatio = 8.0f;
		float const fUpLength = fUpLengthRatio * fUpLengthRatio;
		float const fWaitTime = 0.0f;
		float const fChaseTime = 1.5f;
		float const fChaseMinSpeed = 5.0f;
		float const fDeltaTime = fAccumTime - m_fChaseStartTime;
		NiPoint3 const ptBasePos = m_kPos;
		NiPoint3 const ptBaseUpPos = (ptBasePos + NiPoint3(0,0,fUpLength));
		NiPoint3 const ptPlayerPos = g_kPilotMan.GetPlayerActor()->GetTranslate();
		NiPoint3 const ptStonePos = GetTranslate();


		if (m_eStoneEatState == E_EAT_UP)
		{
			int iMoveUpType = 1;
			float fDeltaPos = 0;
			float fPercent = fDeltaTime / fUpTime;
			
			if (iMoveUpType == 0)
			{
				// 조금씩 올라간다.
				fDeltaPos = fUpLength * fPercent;
				//fDeltaPos += fUpLength_2 * (1 - fPercent);
			}    
			else
	 		{
				// 조금씩 올라간다. 빠르다가 천천히

				float fExponent = 7;		// 홀수만 가능. 수가 높을수록 빠르게 올라가는 시간이 빠르다.
				float fRatio = ((NiPow(-1 + fPercent, fExponent)) + 1);
				fRatio = fRatio * fUpLength;
				fDeltaPos += fRatio;
			}

			// Translate
			NiPoint3 ptResultPos = ptBasePos + NiPoint3(0,0,fDeltaPos);
			SetTranslate(ptResultPos);

			if (fDeltaTime > fUpTime)
			{
				m_eStoneEatState = E_EAT_UP_WAIT;
				m_fChaseStartTime = fAccumTime;
			}
		}
		else if (m_eStoneEatState == E_EAT_UP_WAIT)
		{
			if (fDeltaTime > fWaitTime)
			{
				m_eStoneEatState = E_EAT_CHASE;
				m_fChaseStartTime = fAccumTime;
			}
			
		}
		else if (m_eStoneEatState == E_EAT_CHASE)
		{
			// 캐릭터를 따라간다.
			float fPercent = fDeltaTime / fChaseTime;

			NiPoint3 ptDeltaPos = ptPlayerPos - ptStonePos;
			NiPoint3 ptNormDeltaPos = ptDeltaPos;
			ptNormDeltaPos.Unitize();

			bool bEatenStone = false;
			// 한프레임 가야할 거리 계산.
			NiPoint3 ptBaseDeltaPos = ptPlayerPos - ptBaseUpPos;
			float fDeltaLength = (ptBaseDeltaPos.Length() + 50) * fPercent;
			if (fDeltaLength > ptDeltaPos.Length())
			{
				fDeltaLength = ptDeltaPos.Length();
				bEatenStone = true;
			}
			if (fDeltaLength < fChaseMinSpeed)
			{
				fDeltaLength = fChaseMinSpeed;
			}

			// 이동.
			NiPoint3 ptResultPos = ptStonePos + (ptNormDeltaPos * fDeltaLength);
			SetTranslate(ptResultPos);

			// 스케일
			float fLengthPer = 1.0f;
			if(ptBaseDeltaPos.Length()!=0)	//	0으로 나누기 막기
			{
				fLengthPer = ((ptResultPos - ptBaseUpPos).Length() / ptBaseDeltaPos.Length());	// 0 ~ 1
			}
			float fScale = 1.0f - (0.7f * fLengthPer);
			if (GetScale() > fScale)
			{
				m_spDetailStone->SetScale(fScale);
			}

			// 일정 시간이 지나거나. 캐릭이랑 겹치면 먹었음으로 판단.
			if (fDeltaTime > fChaseTime || ptDeltaPos.Length() < 10.0f || bEatenStone)
			{
				m_eStoneEatState = E_EAT_END;
				m_bHideShadow = true;
			}
		}
		else if (m_eStoneEatState == E_EAT_END)
		{
			// 클라에서 먹은것을 완료.
			m_fChaseStartTime = 0;

			m_spDetailStone->SetScale(1);
			m_spDetailStone->SetAppCulled(true);
			PgParticleMan::ChangeParticleGeneration(m_spParticleNode, false);
		}
	}
#endif

#ifdef PG_USE_SHINESTONE_EAT_PATTERN1
	if (m_eStoneState == E_STONE_EAT && m_bChaseOwner && m_fChaseStartTime > 0)
	{
		NiPoint3 ptPos = GetWorldTranslate();
		
		NiPoint3 ptOwnerPos;
		//if (m_pkOwnerPilot)
		{
			//ptOwnerPos = g_kPilotMan.GetPlayerActor()->GetTranslate();
			ptOwnerPos = GetTranslate();
			float fDeltaTime = fAccumTime - m_fChaseStartTime;
			if (fDeltaTime < CHASE_TIME)
			{
				if (fDeltaTime < CHASE_TIME*0.8f)
				{
					m_fSinProgress+=(fFrameTime/CHASE_TIME);
				}
				else
				{
					m_fSinProgress+=(fFrameTime*0.3f/CHASE_TIME);
				}
				
				if (m_fSinProgress > 0.5f)
				{
/*					NiGeometry *pkGeo = NiDynamicCast(NiGeometry, m_pkBoxActor);
					if(pkGeo)
					{	//알파 빼기
						pkGeo->GetPropertyState()->GetMaterial()->SetAlpha(1.5f - m_fSinProgress);
					}*/
					// 사이즈 줄이기
					float const fSize = GetScale() - m_fSinProgress*0.4f;
					SetScale(__max(fSize, 0));
					_PgOutputDebugString("1아이템 줄일때 사이즈 : %f m_fSinProgress : %f \n", fSize, m_fSinProgress);
				}
				else
				{
					// 사이즈 늘리기
					float const fSize = 1.0f + m_fSinProgress;
					SetScale( fSize );
					_PgOutputDebugString("2아이템 늘릴때 사이즈 : %f m_fSinProgress : %f \n", fSize, m_fSinProgress);
				}
				if (fDeltaTime > GET_TIME)
				{
					//자신 삭제
//					GetWorld()->RemoveObjectOnNextUpdate(GetGuid());
				}
				else
				{
					NiPoint3 ptDelta = (ptOwnerPos - ptPos);	//남은 거리
					NiPoint3::UnitizeVector(ptDelta);
					NiPoint3 ptNormalDelta;
//					ptDelta.UnitizeVector(ptNormalDelta);
					ptNormalDelta = ptDelta*(fDeltaTime/CHASE_TIME)*50.0f;
					float fAddZ = NiSin(m_fSinProgress*NI_PI);
					if (ptDelta.SqrLength() <= ptNormalDelta.SqrLength() )
					{
						ptPos.x = ptOwnerPos.x;
						ptPos.y = ptOwnerPos.y;
						//자신 삭제
						//GetWorld()->RemoveObjectOnNextUpdate(GetGuid());
					}
					else
					{
						ptPos.x += ptNormalDelta.x;
						ptPos.y += ptNormalDelta.y;
						//_PgOutputDebugString("ptNormalDelta X : %f Y : %f Z : %f \n", ptNormalDelta.x,ptNormalDelta.y, ptNormalDelta.z);
					}
					ptPos.z = m_kptOriginPos.z+fAddZ*100.0f;
					//_PgOutputDebugString("fAddZ*20 : %f  ptPos.z : %f \n", fAddZ*70.0f, ptPos.z );
					SetTranslate(ptPos);
				}
			}
			else
			{
				SetAppCulled(true);
			}
		}
	}
	else
	{
		m_kptOriginPos = GetWorldTranslate();
	}
#endif


	return true;
}

void PgShineStone::ReserveGetShineStone(int iIndex)
{
	int iStoneIndex = iIndex - PG_SHINESTONE_FIRST_IDX;	// 78부터 스톤이미지임.414
	// Sound
	{
		std::string strSoundName = "GetShineStone_";
		strSoundName += MB((std::wstring)(BM::vstring(iStoneIndex)));

		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D,strSoundName.c_str(),0.0f, 0.0f, 0.0f, this);
	}

	// Texture Change
	// Find Billboard Node
	// And get child from billboard node
	if (!m_pkDetailGeo)
	{
		NiObjectList kList;
		GetAllBillboardNode(m_spDetailStone, kList);
		if(kList.IsEmpty())
		{
			return;
		}
		m_pkBillboardNode = NiDynamicCast(NiBillboardNode, kList.GetTail());
		// Get NiGeometry
		if(!NiIsKindOf(NiGeometry, m_pkBillboardNode->GetAt(0)))
		{
			return;
		}
		
		m_pkDetailGeo = NiDynamicCast(NiGeometry, m_pkBillboardNode->GetAt(0));
	}
	if (!m_pkDetailGeo ||
		!m_pkDetailGeo->GetModelData() ||
		!m_pkDetailGeo->GetModelData()->GetTextures() ||
		m_pkDetailGeo->GetModelData()->GetVertexCount() < 4)
	{
		return;
	}

	// Set UV
	float fX = ((iStoneIndex) % 4) * 0.25f;
	float fY = (int)((iStoneIndex) / 4) * 0.5f;
	m_pkDetailGeo->GetModelData()->GetTextures()[0] = NiPoint2(fX, fY);
	m_pkDetailGeo->GetModelData()->GetTextures()[1] = NiPoint2(fX, fY + 0.5f);
	m_pkDetailGeo->GetModelData()->GetTextures()[2] = NiPoint2(fX + 0.25f, fY);
	m_pkDetailGeo->GetModelData()->GetTextures()[3] = NiPoint2(fX + 0.25f, fY + 0.5f);

	_PgOutputDebugString(":: Set Stone UV Index:%d  x:%f y:%f \n", iStoneIndex, fX, fY);
}

void PgShineStone::ChaseOwner()
{
	if(!g_pkWorld)
	{
		return;
	}
	// process when Recv Packet
	m_bChaseOwner = true;
	m_fChaseStartTime = g_pkWorld->GetAccumTime();
	m_eStoneState = E_STONE_EAT;
	m_eStoneEatState = E_EAT_UP;
	m_fSinProgress = 0;

	TBL_DEF_RES kDefRes;
	_PgOutputDebugString("!! GetStoneItem No :%d\n", m_iItemNo);

	m_spShineStone->SetAppCulled(true);
	m_spDetailStone->SetAppCulled(false);
	m_spDetailStone->SetAppCulled(false);
	if (lwGetItemResData(m_iItemNo, kDefRes))
	{
		ReserveGetShineStone(kDefRes.UVIndex);
	}
}

void PgShineStone::DrawImmediate( PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime )
{
	PgParticle* pkParticle;
	for ( AttachSlot::iterator itr = m_kAttachSlot.begin(); itr != m_kAttachSlot.end(); ++itr )
	{
		pkParticle = NiDynamicCast(PgParticle,itr->second);
		if ( pkParticle && pkParticle->GetZTest() == true )
		{
			pkParticle->SetAppCulled(false);
			pkRenderer->PartialRenderClick_Deprecated(pkParticle);
			pkParticle->SetAppCulled(true);
		}
	}
}

void lwChangeShineStoneCount(lwUIWnd klwWnd, int iItemNo)
{
	XUI::CXUI_Wnd *pkWnd = klwWnd();
	if (pkWnd == NULL)
	{
		return;
	}

	if( g_pkWorld && !g_pkWorld->IsHaveAttr(GATTR_VILLAGE) )
	{
		size_t PlayerCnt = 0;//g_pkWorld->GetShineStoneCount(iItemNo);
		size_t MapCnt = g_pkWorld->m_kShineStoneContainer.size();
//		PgWorld::ShineStoneContainer::iterator iter = g_pkWorld->m_kShineStoneContainer.begin();
//		while( iter != g_pkWorld->m_kShineStoneContainer.end() )
//		{
//			if( iter->second->PlayerStone().iItemNo == iItemNo )
//			{
//				++MapCnt;
//			}
//			++iter;
//		}


		wchar_t	szTemp[MAX_PATH] = {0,};
		swprintf(szTemp, MAX_PATH, TTW(400424).c_str(), PlayerCnt, MapCnt);
		klwWnd.SetStaticText(MB(szTemp));
		if( MapCnt )
		{
			klwWnd.SetGrayScale(false);
		}
		else
		{
			klwWnd.SetGrayScale(true);
		}
	}
	else
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer){ return; }
		PgInventory *pkInv = pkPlayer->GetInven();
		if (!pkInv) { return; }

		size_t const PlayerCnt = pkInv->GetTotalCount(iItemNo);
		klwWnd.SetStaticText(MB(BM::vstring((int)PlayerCnt)));
		klwWnd.SetGrayScale(false);
	}
}