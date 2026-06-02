#include "stdafx.h"
#include "PgMobileSuit.h"
#include "PgRenderer.h"
#include "PgAMPool.h"
#include "PgWorld.h"
#include "PgRewardBox.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgSoundMan.h"
#include "PgActorManager.H"

float const ROT_DEGREE = 0.7f;
float const PARTICLE_ROTATE_CYCLE = 4.4f;
float const PARTICLE_ROTATE_SOUND_DELAY = 2.0f;

PgRewardBox::PgRewardBox() : m_pkNIFRoot(NULL), m_spObject(0)
{
	Init();
	m_spAM = g_kAMPool.LoadActorManager("../Data/4_Item/9_Etc/MissionMapCube/MissionMapCube.kfm", PgIXmlObject::ID_NPC);
	if (m_spAM)
	{
		m_pkNIFRoot = NiDynamicCast(NiAVObject, m_spAM->GetNIFRoot());
		PG_ASSERT_LOG(m_pkNIFRoot);

		if (m_pkNIFRoot)
		{
			//m_pkNIFRoot->SetScale(1.1f);
			m_pkNIFRoot->UpdateNodeBound();
			m_pkNIFRoot->UpdateProperties();
			m_pkNIFRoot->UpdateEffects();
			m_pkNIFRoot->Update(0);
		}
	}

	UpDownVec(NiPoint3::UNIT_Z);
}

PgRewardBox::~PgRewardBox()
{
	Destroy();
}

void PgRewardBox::Init()
{
	BoxState(E_REWORD_BOX_NONE);
	m_iNumber = 0;
	m_wstrOwner.clear();
	TimeStamp(0.0f);
	StartTime(0.0f);
	RotDegree(0.0f);
	SetRotate(NiMatrix3::IDENTITY);
	UpDown(false);
	if (m_spAM)
	{
		m_spAM->SetTargetAnimation(1000000);
	}
	UpDownVec(NiPoint3::UNIT_Z);
}

void PgRewardBox::Destroy()
{
	m_spObject = NULL;
	m_pkNIFRoot = NULL;	
	m_spAM = NULL;
}

void PgRewardBox::FaceToCamera(NiPoint3 const& rkRightVec)
{// 카메라 업벡터가 입력된 후에 계산할 것!!!
	//float const kfDot = UpDownVec().Dot(NiPoint3::UNIT_Z);

	//NiMatrix3 kRot;
	//kRot.MakeRotation(-kfDot, kRightVec);
	//SetRotate(kRot);

	NiPoint3 kLookingDir = rkRightVec;// - GetTranslate();
	kLookingDir.Unitize();
	NiPoint3 kBase = kLookingDir;
	kBase.z = 0;
	kBase.Unitize();
	float const kfDot = NiACos(kBase.Dot(kLookingDir));

	NiMatrix3 kRot;
	kRot.MakeRotation(-kfDot, rkRightVec);
	SetRotate(kRot);
}

void PgRewardBox::SetTranslate(NiPoint3 const &kTrn)
{
	if (m_pkNIFRoot)
	{	
		m_pkNIFRoot->SetTranslate(kTrn);
	}
}

NiPoint3 PgRewardBox::GetTranslate() const
{
	if (m_pkNIFRoot)
	{	
		return m_pkNIFRoot->GetWorldTranslate();
	}

	return NiPoint3(0,0,0);
}

void PgRewardBox::SetRotate(NiMatrix3 const& kRot)
{
	if (m_pkNIFRoot)
	{	
		NiNode *pkTarget = NiDynamicCast(NiNode, m_pkNIFRoot->GetObjectByName("cube"));
		if (pkTarget)
		{
			pkTarget->SetRotate(kRot);
		}
	}
}

NiMatrix3 PgRewardBox::GetRotate() const
{
	if (m_pkNIFRoot)
	{	
		return m_pkNIFRoot->GetWorldRotate();
	}

	return NiMatrix3();
}

void PgRewardBox::BoxState(E_REWORD_BOX_STATE const eState)
{
	if(!g_pkWorld || !m_spAM || !m_pkNIFRoot)
	{
		return;
	}

	if (m_kBoxState != eState)
	{
		m_kBoxState = eState;
		TimeStamp(g_pkWorld->GetAccumTime());	// 스테이트가 바뀔 때 마다 시간 새로 저장

		switch(BoxState())
		{
		case E_REWORD_BOX_NONE:
			{
				m_pkNIFRoot->SetAppCulled( false );
				if (!m_spObject)
				{
					m_spObject = g_kParticleMan.GetParticle("e_ef_RewardBox_Select",PgParticle::O_SCALE, 0.7f);
				}

				if (m_spObject)
				{
					NiNode *pkTarget = NiDynamicCast(NiNode, m_pkNIFRoot->GetObjectByName("cube"));
					if (pkTarget)
					{
						pkTarget->DetachChild(m_spObject);
					}
				}
			}break;
		case E_REWORD_BOX_DROP:
			{
				m_spAM->SetTargetAnimation(1000000);
				StartTime(g_pkWorld->GetAccumTime());		//시작 시간 저장
			}break;
		case E_REWORD_BOX_IDLE:
			{
				m_spAM->SetTargetAnimation(1000000);
			}break;
		case E_REWORD_BOX_OVER:
			{
				m_spAM->SetTargetAnimation(1000000);
			}break;
		case E_REWORD_BOX_SELECT:
			{
				if (!m_spObject)
				{
					m_spObject = g_kParticleMan.GetParticle("e_ef_RewardBox_Select",PgParticle::O_SCALE, 0.7f);
				}
				if (m_spObject)
				{
					NiNode *pkTarget = NiDynamicCast(NiNode, m_pkNIFRoot->GetObjectByName("cube"));
					if (pkTarget)
					{
						pkTarget->AttachChild(m_spObject, true);
						TimeStamp(g_pkWorld->GetAccumTime()+PARTICLE_ROTATE_SOUND_DELAY-PARTICLE_ROTATE_CYCLE);
					}
				}

				m_spAM->SetTargetAnimation(1000000);
			}break;
		case E_REWORD_BOX_OPEN:
			{
				NiNode *pkTarget = NiDynamicCast(NiNode, m_pkNIFRoot->GetObjectByName("cube"));
				if (pkTarget)
				{
					if (m_spObject)
					{
						pkTarget->DetachChild(m_spObject);
					}
				}

				m_spAM->SetTargetAnimation(1000001);
			}break;
		case E_REWORD_BOX_DISABLE:
			{
				m_pkNIFRoot->SetAppCulled( true );
// 				if (m_pkNIFRoot)
// 				{
// 					NiAlphaProperty* pkProPerty = RecursiveProcessAlpha(m_pkNIFRoot);
// 					if (pkProPerty)
// 					{
// 						// 약간 회색으로 알파주자 반투명하게
// 					}
// 				}
// 					
// 				m_spAM->SetTargetAnimation(1000001);
			}break;
		}
	}
}

bool PgRewardBox::Picked(NiPoint3 const& rkOrgPt, NiPoint3 const& rkRayDir)	// 이놈이 마우스에 걸렸는지
{
	if (!m_pkNIFRoot)
	{
		return false;
	}
	NiPick kPick;
	kPick.SetCoordinateType(NiPick::WORLD_COORDINATES);
	kPick.SetPickType(NiPick::FIND_ALL);
	kPick.SetIntersectType(NiPick::TRIANGLE_INTERSECT);

	// 레이를 체크하고
	// 충돌이 있으면 반환한다.
	kPick.SetTarget(m_pkNIFRoot->GetObjectByName("center"));
	return kPick.PickObjects(rkOrgPt, rkRayDir, true);
}

NiAlphaProperty* PgRewardBox::RecursiveProcessAlpha(NiAVObject* pkObject)
{
	if (NiIsKindOf(NiGeometry, pkObject))
	{
		NiAlphaProperty *pkProp = NiDynamicCast(NiAlphaProperty, pkObject->GetProperty(NiProperty::ALPHA));
		if (pkProp)
		{
			return pkProp;
		}
	}

	if(NiIsKindOf(NiNode, pkObject))
	{
		NiNode* pkNode = NiDynamicCast(NiNode, pkObject);
		for(unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				RecursiveProcessAlpha(pkChild);
			}
		}
	}

	return NULL;
}

bool PgRewardBox::Update(float const fAccumTime, float const fFrameTime)
{
	if (!m_spAM || !m_pkNIFRoot || BoxState() == E_REWORD_BOX_NONE)
	{
		return false;
	}

	float const fFlowTime = fAccumTime - TimeStamp();

	switch(BoxState())
	{
	case E_REWORD_BOX_DROP:
	case E_REWORD_BOX_IDLE:
	case E_REWORD_BOX_OVER:
	case E_REWORD_BOX_SELECT:
		{
			if (UpDown())
			{
				float const fAdd = NiSin((fAccumTime-StartTime())/NI_PI*13.0f)*0.2f;
				NiPoint3 ptBox = GetTranslate();
				ptBox+=fAdd*UpDownVec();
				SetTranslate(ptBox);
			}
			
			if (BoxState() != E_REWORD_BOX_OVER) //안좋아 안좋아.. 다른 방법이 있을텐데.....
			{
				RotDegree(RotDegree() + ROT_DEGREE*fFrameTime);
				NiMatrix3 kRot = GetRotate();
				kRot.MakeRotation(RotDegree(), NiPoint3::UNIT_Z);	//로컬 축으로 도니까
				SetRotate(kRot);
			}

			if (BoxState() == E_REWORD_BOX_SELECT)
			{
				if (fFlowTime>=PARTICLE_ROTATE_CYCLE && g_kPilotMan.GetPlayerPilot())
				{
					g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D,"MSC_Rotate_Particle", 0.0f,80,180,g_kPilotMan.GetPlayerPilot()->GetWorldObject());

					TimeStamp(fAccumTime);
				}
			}
			
		}break;
	case E_REWORD_BOX_OPEN:
	    break;
	case E_REWORD_BOX_DISABLE:
		break;
	}

	m_spAM->Update(fAccumTime);
	m_pkNIFRoot->Update(fAccumTime);
	return true;
}

void PgRewardBox::Draw(PgRenderer *pkRenderer, float const fFrameTime)
{
	if (!m_pkNIFRoot || pkRenderer == NULL)
	{
		return;
	}
	
 	pkRenderer->PartialRenderClick_Deprecated(m_pkNIFRoot);
}