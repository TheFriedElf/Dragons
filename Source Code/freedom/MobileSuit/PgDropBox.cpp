#include "StdAfx.h"
#include "PgMobileSuit.h"
#include "PgDropBox.h"
#include "PgRenderer.h"
#include "PgActionSlot.h"
#include "PgAMPool.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgWorld.h"
#include "FreedomPool.h"
#include "PgCircleShadow.h"
#include "PgParticleMan.h"
#include "PgItemMan.h"
#include "PgWorkerThread.h"
#include "PgSoundMan.h"
#include "PgTrail.H"

#include "NewWare/Scene/ApplyTraversal.h"


float const CHASE_TIME = 0.7f;
float const GET_TIME = CHASE_TIME*0.99f;
int const MAX_BOXITEM = 8;
extern ControllerManager g_kControllerManager;

float PgDropBox::ms_fGravity = -50.0f;				// 아이템 점프시에만 적용되는 중력
PgDropBox::DropTransformList PgDropBox::ms_kDropTransformList;
bool PgDropBox::ms_bLoaded = false;

NiImplementRTTI(PgDropBox, PgIWorldObject);

PgDropBox::PgDropBox()
:	m_strOwnerEffect_TargetName("char_root")
{
	m_pkCurrentAudioSource = 0;
	m_kItemArray.reserve(MAX_BOXITEM);
	m_pNameText = NULL;
	m_kNameColor = NiColorA::WHITE;
	m_bChaseOwner = false;
	m_fChaseStartTime = 0;
	m_fSinProgress = 0.0f;
	m_kItemArray.clear();
	m_kOffererGuid.Clear();
	m_kOffererPos = NiPoint3(0.f,0.f,0.f);
	m_kOwnerGuid.Clear();

	m_bLove_BaseItem = false;

	m_eBoxState = E_BOX_NONE;
//	m_fJumpAccumHeight = 0.0f;
	m_bJump = false;
	m_fJumpTime = 0;
	m_bFloor = false;
	m_fInitialVelocity = 0.0f;

#ifdef PG_USE_DROPBOX_EAT_PATTERN1
	m_pkOwnerPilot = NULL;
	m_pkOwnerActor = NULL;
#endif
	PastPos(NiPoint3(0,0,0));

	// PhysX 관련
	m_pkPhysXActor = 0;
	m_pkPhysXSrc = 0;
	m_pkPhysXDest = 0;

	m_pkController = 0;

	for (unsigned int i = 0; i < PG_MAX_NB_ABV_SHAPES; i++)
	{
		m_apkPhysXCollisionActors[i] = 0;
		m_apkPhysXCollisionSrcs[i] = 0;
	}


	NiObjectList kNodeList;
	RemoveReserve(false);
	IsGetDropItemRes(false);
	ItemNum(0);
	RotDegree(0.0f);
	IsMine(false);
	PickupSound("Com_Item_Pickup");
	
	m_pkAM = 0;
	m_pkItemEX = NULL;

	m_pkTrail = NULL;
	m_bMeshLoaded = false;

	m_kCreateTime = 0.f;

	m_kBottomRayHit.worldImpact.set(NxVec3(0.0f, 0.0f, 0.0f));
	m_bContactGround = false;

	m_kRemainTime = 0;
	m_fLifeTime = 0.f;
}

PgDropBox::~PgDropBox()
{
	SAFE_DELETE(m_pNameText);
	THREAD_DELETE_ITEM(m_pkItemEX);

	StopTrail();

	if ( m_strOwnerEffect.size() )
	{
		PgActor *pkActor = g_kPilotMan.FindActor(m_kOwnerGuid);
		if(pkActor)
		{
			PgParticle *pkParticle = g_kParticleMan.GetParticle( m_strOwnerEffect.c_str(), PgParticle::O_SCALE, pkActor->GetEffectScale());
			if( pkParticle )
			{
				if( !pkActor->AttachTo( 20091218, m_strOwnerEffect_TargetName.c_str(), (NiAVObject *)pkParticle) )
				{
					THREAD_DELETE_PARTICLE(pkParticle);
				}
			}
		}
	}

	if( !m_ParticleName.empty() )
	{
		DetachFrom(20111124);
	}
}
void	PgDropBox::StopTrail()
{
	if( m_pkTrail )
	{
		if ( g_spTrailNodeMan )
		{
			g_spTrailNodeMan->StopTrail(m_pkTrail,false);
		}
		
		m_pkTrail= NULL;
	}
}
void	PgDropBox::StartTrail()
{
	StopTrail();

	if(m_pkTrail)
	{
		return;
	}


	m_pkTrail = g_spTrailNodeMan->StartNewTrail(NiDynamicCast(NiAVObject,this),
		"Scene Root",
		"../Data/5_Effect/9_Tex/Trail_arc.dds",
		0.5f,
		0.25,
		10.0);

}

namespace PgDropBoxUtil
{
	float fJumpedBias = 0.5f;
	float Bias(float x, float biasAmt)
	{
		float fLastEmponent = 0;
		fLastEmponent = NiLog(biasAmt) * -1.4427f;
		return NiPow(x, fLastEmponent);
	}

	inline float RemapVal(float val, float A, float B, float C, float D) {return C + (D-C) * (val - A) / (B - A); };
}

#define PG_USE_DROPBOX_EAT_PATTERN2
//#define PG_USE_DROPBOX_EAT_PATTERN1
bool PgDropBox::Update(float fAccumTime, float fFrameTime)
{
	if (RemoveReserve())
	{
		NILOG(PGLOG_WARNING, "[PgDropBox] RemoveReserve is True \n");
		_PgOutputDebugString("[PgDropBox] RemoveReserve is True \n");
		return false;
	}
	
	if((PgGroundItemBox::ms_GROUNDITEMBOX_DURATION_TIME + 1000) < fAccumTime - m_kCreateTime)	//최대 생존시간보다 1초이상이면 박스 강제 삭제
	{
		RemoveNextUpdate();
		return false;
	}
	PgIWorldObject::Update(fAccumTime, fFrameTime);
	NiActorManager *pkAM = GetActorManager();
	if (pkAM)
	{
		pkAM->Update(fAccumTime);
	}

	if (m_eBoxState == E_BOX_JUMP)
	{
		if(m_bJump)
		{
			// 점프 중이라면 점프 높이를 계산하자.
			// h = (v0 * (t+ delta t) + g(t + delta t)^2) - v0*t + gt^2 
			//   = (v0 * (JumpTime + FrameTime) + g(JumpTime + FrameTime)^2) - v0*t + gt^2
			//   = ...
			//   = v0 + (g * (2 * JumpTime + FrameTime))
			float const fJumpHeight = m_fInitialVelocity + (ms_fGravity * (2.0f * m_fJumpTime + fFrameTime));
			m_fJumpTime += fFrameTime;
//			m_fJumpAccumHeight += fJumpHeight * fFrameTime;

			float const fJumpProgress = PgDropBoxUtil::RemapVal((m_fInitialVelocity-fJumpHeight), 0, m_fInitialVelocity*2.f, 0.f, 1.f);//Jump Process 0.0 ~ 1.0f
			float const fBiasedProgress = PgDropBoxUtil::Bias(fJumpProgress, PgDropBoxUtil::fJumpedBias);

			NiPoint3 ptBox = GetWorldTranslate();

			ptBox.x = PgDropBoxUtil::RemapVal(fBiasedProgress, 0.f, 1.f, m_kOffererPos.x, m_kptOriginPos.x);
			ptBox.y = PgDropBoxUtil::RemapVal(fBiasedProgress, 0.f, 1.f, m_kOffererPos.y, m_kptOriginPos.y);
			ptBox.z += fJumpHeight;

			if( (ptBox.z <= m_kptOriginPos.z+5) || (1.0f <= fJumpProgress) || (1.0f <= fBiasedProgress) )//! 서버에서 지정한 Z좌표보다 아래면
			{
				ptBox.z = m_kptOriginPos.z+5;
				m_eBoxState = E_BOX_IDLE;
				StopTrail();
				CreateABVShapes();
			}
			SetPosition(ptBox);
		}
	}
	else if (m_eBoxState == E_BOX_IDLE)
	{
		NiPoint3 ptHeight = NiPoint3(0,0,m_kNowDropTransform.m_fUpDown);
		NiPoint3 ptBox = m_kptOriginPos + ptHeight + m_kNowDropTransform.m_kPos;;

		if(m_kNowDropTransform.m_fRotSpeed)
		{
			RotDegree((RotDegree()+m_kNowDropTransform.m_fRotSpeed*fFrameTime));
			NiMatrix3 kRot;
			kRot.MakeRotation(RotDegree(), NiPoint3::UNIT_Z);	//로컬 축으로 도니까
			NiQuaternion kQuat;
			kQuat.FromRotation(kRot);

			SetRotation(kQuat);
		}
		
		float const fAdd = NiSin(m_fSinProgress/NI_PI*m_kNowDropTransform.m_fUpDownSpeed)*ptHeight.z;
		ptBox.z+=fAdd;
		m_fSinProgress+=fFrameTime;
		SetPosition(ptBox);

		if(false==m_bContactGround)
		{
			m_bContactGround = true;
			AttachCircleShadow(ptBox);
		}
	}
#ifdef PG_USE_DROPBOX_EAT_PATTERN2
	// 200709018 강정욱
	else if (m_eBoxState == E_BOX_EAT && m_bChaseOwner && m_fChaseStartTime > 0 && !GetHide())
	{
		NiPoint3 ptPos = GetWorldTranslate();
		
		NiPoint3 ptOwnerPos;
		PgPilot* pkOwnerPilot = g_kPilotMan.FindPilot(m_kOwnerGuid);
		if (pkOwnerPilot)
		{
			PgActor* pkOwnerActor = dynamic_cast<PgActor *>(pkOwnerPilot->GetWorldObject());
			if (!pkOwnerActor)
			{
				NILOG(PGLOG_ERROR, "[PgDropBox::Update] can't find owner(%s) for dropbox(%s)\n", MB(m_kOwnerGuid.str().c_str()), MB(GetGuid().str().c_str()));
				RemoveNextUpdate();
				return false;
			}
			ptOwnerPos = pkOwnerActor->GetTranslate();
			float const fDeltaTime = fAccumTime - m_fChaseStartTime;
			float const fMinScale = 0.1f;
			if (fDeltaTime < CHASE_TIME || GetScale() <= fMinScale)
			{
				if (fDeltaTime < CHASE_TIME*0.8f)
				{
					m_fSinProgress+=(fFrameTime/CHASE_TIME);
				}
				else
				{
					m_fSinProgress+=(fFrameTime*0.3f/CHASE_TIME);
				}
				
				float const fHalfPer = 0.4f;	// 전체 시간에서 몇프로 까지 첫번째 동작을 하는지.
				float const fPercent = fDeltaTime / CHASE_TIME;
				float const fMaxScale = 1.8f;

				float const fJumpProgress = PgDropBoxUtil::RemapVal(fDeltaTime, 0, CHASE_TIME, 0.f, 1.f);//Process 0.0 ~ 1.0f
				float const fBiasedProgress = PgDropBoxUtil::Bias(fJumpProgress, PgDropBoxUtil::fJumpedBias);

				if (fPercent < fHalfPer)
				{
					float const fPartPer = fPercent / fHalfPer;
					float const fScale = ((fMaxScale - 1) * fPartPer) + 1;
					SetScale(fScale);
				}
				else
				{
					float const fPartPer = ((fPercent-fHalfPer) / (1-fHalfPer));
					float const fScalePer = (fMaxScale - fMinScale) * fPartPer;
					float const fScale = fMaxScale - fScalePer;
					SetScale(fScale);
				}

				if (fDeltaTime > GET_TIME)
				{
					//자신 삭제
					RemoveNextUpdate();
					return false;
				}
				else
				{
					NiPoint3 ptDelta = (ptOwnerPos - ptPos);	//남은 거리
					NiPoint3::UnitizeVector(ptDelta);
					NiPoint3 ptNormalDelta;
//					ptDelta.UnitizeVector(ptNormalDelta);
					ptNormalDelta = ptDelta*(fDeltaTime/CHASE_TIME)*50.0f;
					float const fAddZ = PgDropBoxUtil::Bias(NiSin(m_fSinProgress*NI_PI), 0.5f);
					if (ptDelta.SqrLength() <= ptNormalDelta.SqrLength() )
					{
						//ptPos.x = ptOwnerPos.x;
						//ptPos.y = ptOwnerPos.y;
						ptPos.x = PgDropBoxUtil::RemapVal(fBiasedProgress, 0.f, 1.f, m_kptOriginPos.x, ptOwnerPos.x);
						ptPos.y = PgDropBoxUtil::RemapVal(fBiasedProgress, 0.f, 1.f, m_kptOriginPos.y, ptOwnerPos.y);
						//자신 삭제
						//GetWorld()->RemoveObjectOnNextUpdate(GetGuid());
					}
					else
					{
						ptPos.x = PgDropBoxUtil::RemapVal(fBiasedProgress, 0.f, 1.f, m_kptOriginPos.x, ptOwnerPos.x);
						ptPos.y = PgDropBoxUtil::RemapVal(fBiasedProgress, 0.f, 1.f, m_kptOriginPos.y, ptOwnerPos.y);
						//ptPos.x += ptNormalDelta.x;
						//ptPos.y += ptNormalDelta.y;
						//_PgOutputDebugString("ptNormalDelta X : %f Y : %f Z : %f \n", ptNormalDelta.x,ptNormalDelta.y, ptNormalDelta.z);
					}
					ptPos.z = m_kptOriginPos.z+fAddZ*100.0f;
					//_PgOutputDebugString("fAddZ*20 : %f  ptPos.z : %f \n", fAddZ*70.0f, ptPos.z );
					SetPosition(ptPos);
				}
			}
			else
			{
				RemoveNextUpdate();
				return false;
			}
		}
		else
		{
			RemoveNextUpdate();
			return false;
		}
	}
#endif
#ifdef PG_USE_DROPBOX_EAT_PATTERN1
	// 승봉씨가 만든 DropBox먹는 패턴.
	else if (m_eBoxState == E_BOX_EAT && m_bChaseOwner && m_fChaseStartTime > 0)
	{
		NiPoint3 ptPos = GetWorldTranslate();
		
		NiPoint3 ptOwnerPos;
		if (m_pkOwnerPilot)
		{
			if (!m_pkOwnerActor)
			{
				GetWorld()->RemoveObjectOnNextUpdate(GetGuid()); //자진 삭제
				return false;
			}
			ptOwnerPos = m_pkOwnerActor->GetTranslate();
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
					SetPosition(ptPos);
				}
			}
			else
			{
				GetWorld()->RemoveObjectOnNextUpdate(GetGuid());
			}
		}
	}
#endif
	else
	{
		m_kptOriginPos = GetWorldTranslate();
		NILOG(PGLOG_WARNING, "m_eBoxState = %d m_bChaseOwner = %d m_fChaseStartTime = %f\n", m_eBoxState, m_bChaseOwner, m_fChaseStartTime);
		//_PgOutputDebugString("m_eBoxState = %d m_bChaseOwner = %d m_fChaseStartTime = %f\n", m_eBoxState, m_bChaseOwner, m_fChaseStartTime);
		//GetWorld()->RemoveObjectOnNextUpdate(GetGuid());
	}
	return true;
}

bool PgDropBox::ProcessAction(PgAction *pkAction,bool bInvalidateDirection,bool bForceToTransit)
{
	return true;
}

void PgDropBox::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	if (GetHide() == true || m_eBoxState == E_BOX_EAT)
	{
		return;
	}

	PgBase_Item * pItem = this->GetItem(0);
	if( !pItem )
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(pItem->ItemNo());
	if( !pDef )
	{
		return;
	}

	int const Type = pDef->GetAbil(AT_INSTANCE_ITEM);
	if ( !Type )
	{// 인스턴트 아이템이면 이름을 그리지 않는다.
		if (m_pNameText == NULL)
		{
			XUI::CXUI_Font	*pFont = g_kFontMgr.GetFont(FONT_NAME);
			if(pFont)
			{
				m_pNameText = new Pg2DString(XUI::PgFontDef(pFont),_T(""));
				PG_ASSERT_LOG(m_pNameText);
				if (m_pNameText)
				{
					m_pNameText->SetText(pFont, m_kName);
				}
			}
		}

//	NiActorManager *pkAM = GetActorManager();
//	if (pkAM == NULL)
//		return;

		if(m_pNameText)
		{
			NiPoint3	vCamDir = pkCamera->GetWorldDirection();
			NiPoint3	vCamRight = pkCamera->GetWorldRightVector();
			NiPoint3	vCamUp = pkCamera->GetWorldUpVector();
			const NiBound& kBound = GetWorldBound();
			NiPoint3	spPos(m_kBottomRayHit.worldImpact.x, m_kBottomRayHit.worldImpact.y, m_kBottomRayHit.worldImpact.z);
			spPos.z += 37.f;

			float		fOutlineThickness = 0.5f;
			NiColorA	kShadowColor;
			kShadowColor = m_kNameColor * 0.2f;
			kShadowColor.a = 1.f;
			m_pNameText->Draw_3DPos(pkRenderer,spPos+vCamUp*fOutlineThickness+vCamRight*fOutlineThickness,pkCamera,kShadowColor,NiColorA(0,0,0,0),false,1.0f,true);
			m_pNameText->Draw_3DPos(pkRenderer,spPos,pkCamera,m_kNameColor);
		}
	}
}

void PgDropBox::SetRotation(const NiQuaternion &kQuat)
{
	if(m_pkPhysXActor)
	{
		NxQuat kNxQuat;
		NiPhysXTypes::NiQuaternionToNxQuat(kQuat, kNxQuat);
		m_pkPhysXActor->setGlobalOrientationQuat(kNxQuat);
	}

	SetRotate(kQuat);
}

bool PgDropBox::SetPosition(NiPoint3 const &rkTranslate)
{
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.SetPosition"), g_pkApp->GetFrameCount()));
	if(!m_pkController)
	{
		PG_ASSERT_LOG(!"Character Controller is null!");
		NILOG(PGLOG_ERROR, "[PgActor] SetPosition, %s(%s) actor has no Character Controller\n", GetPilot() ? MB(GetPilot()->GetName()) : "", MB(GetGuid().str()));
	}

	/// Comment : PhysX동기자가 다음 업데이트 시에 Gamebryo Object와 좌표를 맞춰 주기 때문에 
	///	그 프레임에 GetTranslate()을 하면 좌표가 한 프레임 어긋난다. 때문에 AVObject::SetTranslate()을 같이 해준다. 

	m_pkController->setPosition(NxExtendedVec3(rkTranslate.x, rkTranslate.y, rkTranslate.z));
	SetTranslate(rkTranslate);
	SetWorldTranslate(rkTranslate);

	if(m_pkPhysXActor)
	{
		NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();

		if (GetWorld() && g_iUseAddUnitThread == 1)
		{
			GetWorld()->LockPhysX(true);
		}

		m_pkPhysXActor->setGlobalPosition(NxVec3(rkTranslate.x, rkTranslate.y, rkTranslate.z));

		NxShape *pkShape = m_pkPhysXActor->getShapes()[0];
		pkShape->setGlobalPosition(NxVec3(rkTranslate.x, rkTranslate.y, rkTranslate.z));

		for(int i = 0; GetABVShape(i)->IsValid() && i < PG_MAX_NB_ABV_SHAPES; i++)
		{
			if(m_apkPhysXCollisionActors[i])
			{
				PgIWorldObjectBase::ABVShape *pkABVShape = GetABVShape(i);
				NiAVObject *pkTarget = GetObjectByName(GetABVShape(i)->m_kTo);
				if(pkTarget)
				{
					//NiPoint3	kTargetPos = pkTarget->GetWorldTranslate()+NiPoint3(pkABVShape->m_kMat.t.x,pkABVShape->m_kMat.t.y,pkABVShape->m_kMat.t.z);
					//m_apkPhysXCollisionActors[i]->setGlobalPosition(NxVec3(rkTranslate.x, rkTranslate.y, rkTranslate.z));

					NxVec3 kTargetPos;
					NiAVObject *pkTarget = GetObjectByName(GetABVShape(i)->m_kTo);
					NiPhysXTypes::NiPoint3ToNxVec3(pkTarget->GetWorldTranslate(), kTargetPos);
					m_apkPhysXCollisionActors[i]->setGlobalPosition(kTargetPos);
				}
			}
		}

		if (GetWorld() && g_iUseAddUnitThread == 1)
		{
			GetWorld()->LockPhysX(false);
		}

	}

	/// Comment ; NxCapsuleController::setPosition()가 bool을 리턴하는데, 메뉴얼(2.6.2)에 보면 현재는 항상 true를 리턴한다고 되어 있다.
	return true;
}
void PgDropBox::CreateABVShapes()
{
	ReleaseABVShapes();


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

		bool	bHasNoValid = true;

		for(int i = 0; GetABVShape(i)->IsValid() && i < PG_MAX_NB_ABV_SHAPES; i++)
		{
			

			NxShapeDesc *kShapeDesc = GetABVShape(i)->GetPhysXShapeDesc();
			kShapeDesc->group = OGT_GROUNDBOX + 1;
			kShapeDesc->userData = this;
			kShapeDesc->name = "Scene Root";

			NxBodyDesc kBodyDesc;
			NxActorDesc kActorDesc;
			kActorDesc.shapes.push_back(kShapeDesc);
			kActorDesc.body = &kBodyDesc;
			kActorDesc.density = 0.1f;
			
			NiAVObject *pkTarget = GetObjectByName(GetABVShape(i)->m_kTo);
			if(pkTarget)
			{
				m_apkPhysXCollisionActors[i] = m_pkPhysXScene->GetPhysXScene()->createActor(kActorDesc);
				m_apkPhysXCollisionActors[i]->raiseBodyFlag(NX_BF_KINEMATIC);
				//m_apkPhysXCollisionSrcs[i] = NiNew NiPhysXDynamicSrc(pkTarget, m_apkPhysXCollisionActors[i], 0, 0);
				m_apkPhysXCollisionSrcs[i] = NiNew NiPhysXKinematicSrc(pkTarget, m_apkPhysXCollisionActors[i]);
				m_apkPhysXCollisionSrcs[i]->SetActive(true);
				m_pkPhysXScene->AddSource(m_apkPhysXCollisionSrcs[i]);
				
				m_apkPhysXCollisionActors[i]->raiseBodyFlag(NX_BF_FROZEN_POS);
				m_apkPhysXCollisionActors[i]->raiseBodyFlag(NX_BF_FROZEN_ROT);
				m_apkPhysXCollisionActors[i]->setGroup(OGT_GROUNDBOX + 1);
				m_apkPhysXCollisionActors[i]->userData = this;

				bHasNoValid = false;

				//if(i < PG_MAX_NB_ABV_SHAPES - 1)
				//{
				//	m_apkPhysXCollisionActors[i+1] = 0;
				//	m_apkPhysXCollisionSrcs[i+1] = 0;
				//}
			}
			else
			{
				PgError2("Actor [%s] ABV TargetNode [%s] Not Found.",GetID().c_str(),GetABVShape(i)->m_kTo);
			}
		}

		if(bHasNoValid)	//	유요한 것이 하나도 없을 경우 디폴트로 하나 만들어서 넣어준다.
		{

			GetABVShape(0)->m_eType = PgIWorldObjectBase::ABVShape::ST_SPHERE;
			GetABVShape(0)->kSphereDesc.radius = 20.0f * GetScale();

			NxShapeDesc *kShapeDesc = GetABVShape(0)->GetPhysXShapeDesc();

			kShapeDesc->group = OGT_GROUNDBOX + 1;
			kShapeDesc->userData = this;
			kShapeDesc->name = "Scene Root";

			NxBodyDesc kBodyDesc;
			NxActorDesc kActorDesc;
			kActorDesc.shapes.push_back(kShapeDesc);
			kActorDesc.body = &kBodyDesc;
			kActorDesc.density = 0.1f;
			
			m_apkPhysXCollisionActors[0] = m_pkPhysXScene->GetPhysXScene()->createActor(kActorDesc);
			m_apkPhysXCollisionActors[0]->raiseBodyFlag(NX_BF_KINEMATIC);
			m_apkPhysXCollisionSrcs[0] = NiNew NiPhysXKinematicSrc(this, m_apkPhysXCollisionActors[0]);
			m_apkPhysXCollisionSrcs[0]->SetActive(true);
			m_pkPhysXScene->AddSource(m_apkPhysXCollisionSrcs[0]);
			
			m_apkPhysXCollisionActors[0]->raiseBodyFlag(NX_BF_FROZEN_POS);
			m_apkPhysXCollisionActors[0]->raiseBodyFlag(NX_BF_FROZEN_ROT);
			m_apkPhysXCollisionActors[0]->setGroup(OGT_GROUNDBOX + 1);
			m_apkPhysXCollisionActors[0]->userData = this;

		}

		if (GetWorld())
		{
			GetWorld()->LockPhysX(false);
		}
	}
	

}

void PgDropBox::ReleaseABVShapes()
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

		for(int i = 0; i < PG_MAX_NB_ABV_SHAPES && m_apkPhysXCollisionActors[i]; ++i)
		{
			if(GetABVShape(i))
			{
				GetABVShape(i)->m_eType = PgIWorldObjectBase::ABVShape::ST_NONE;
			}

			m_pkPhysXScene->DeleteSource(m_apkPhysXCollisionSrcs[i]);
			if (m_pkPhysXScene->GetPhysXScene())
				m_pkPhysXScene->GetPhysXScene()->releaseActor(*m_apkPhysXCollisionActors[i]);
			m_apkPhysXCollisionActors[i] = 0;
		}
		
		if (GetWorld())
		{
			GetWorld()->LockPhysX(false);
		}
	}
}
void PgDropBox::InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup)
{
	PG_ASSERT_LOG(pkPhysXScene);

	NiPhysXManager* pkPhysXManager = NiPhysXManager::GetPhysXManager();
	if (pkPhysXScene == NULL || pkPhysXManager == NULL)
		return;
	PG_STAT(PgStatTimerF timerA(g_kActorStatGroup.GetStatInfo("PgActor.InitPhysX"), g_pkApp->GetFrameCount()));

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		PG_STAT(PgStatTimerF timerA(g_kMobileSuitStatGroup.GetStatInfo("PhysX.WaitSDKLock"), g_pkApp->GetFrameCount()));
		PG_STAT(timerA.Start());
		GetWorld()->LockPhysX(true);
		PG_STAT(timerA.Stop());
	}

	NxScene* pkNxScene = pkPhysXScene->GetPhysXScene();
#ifdef PG_USE_CAPSULE_CONTROLLER

	NxCapsuleControllerDesc kCtrlDesc;
	float fHeight = PG_CHARACTER_CAPSULE_HEIGHT;
	float fRadius = PG_CHARACTER_CAPSULE_RADIUS;
	NiPoint3 kLoc = GetTranslate();
	kCtrlDesc.position.x = kLoc.x;
	kCtrlDesc.position.y = kLoc.y;
	kCtrlDesc.position.z = kLoc.z;
	kCtrlDesc.radius = fRadius;
	kCtrlDesc.height = fHeight;
	kCtrlDesc.upDirection = NX_Z;
	kCtrlDesc.slopeLimit = cosf(NxMath::degToRad(PG_LIMITED_ANGLE + 10.0f));
	kCtrlDesc.skinWidth = 0.1f;
	kCtrlDesc.stepOffset = PG_CHARACTER_CAPSULE_RADIUS * 2.0f;
#else
	NxBoxControllerDesc kCtrlDesc;
	float fHeight = PG_CHARACTER_CAPSULE_HEIGHT + 10.0f;
	float fRadius = PG_CHARACTER_CAPSULE_RADIUS - 5.0f;
	NiPoint3 kLoc = GetTranslate();
	kCtrlDesc.position.x = kLoc.x;
	kCtrlDesc.position.y = kLoc.y;
	kCtrlDesc.position.z = kLoc.z;
	kCtrlDesc.extents = NxVec3(7.5f, 25.0f , 7.5f);
	kCtrlDesc.upDirection = NX_Z;
	kCtrlDesc.slopeLimit = cosf(NxMath::degToRad(30.0f));
	kCtrlDesc.skinWidth = 0.1f;
	kCtrlDesc.stepOffset = fRadius;
#endif

#ifdef PG_USE_CAPSULE_CONTROLLER
	m_pkController = (NxCapsuleController *)g_kControllerManager.createController(pkNxScene, kCtrlDesc);
#else
	m_pkController = (NxBoxController *)g_kControllerManager.createController(pkNxScene, kCtrlDesc);
#endif
	m_pkController->setInteraction(NXIF_INTERACTION_EXCLUDE);
	m_pkController->setPosition(NxExtendedVec3(kLoc.x, kLoc.y, kLoc.z));

	NxMat33 kMat;
	NiPhysXTypes::NiQuaternionToNxMat33(NiQuaternion(NI_HALF_PI, NiPoint3::UNIT_X), kMat);

	m_pkPhysXActor = m_pkController->getActor();
	m_pkPhysXActor->setCMassOffsetLocalOrientation(kMat);
	m_pkPhysXActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
	
	SetRotation(NiQuaternion::IDENTITY);

	NxShape *pkShape = m_pkPhysXActor->getShapes()[0];
	pkShape->setLocalOrientation(kMat);
	pkShape->setLocalPosition(NxVec3(0, 0, 0));
	pkShape->setFlag(NX_SF_DISABLE_COLLISION, true); 
	pkShape->setFlag(NX_SF_DISABLE_RAYCASTING, true);

	//GetNIFRoot()->SetTranslate(NiPoint3(0, 0, -PG_CHARACTER_Z_ADJUST));
	
		
	// Gamebryo --> PhysX 동기자를 생성한다.
	m_pkPhysXSrc = NiNew NiPhysXKinematicSrc(this, m_pkPhysXActor);
	m_pkPhysXSrc->SetActive(false);
	m_pkPhysXSrc->SetInterpolate(false);
	pkPhysXScene->AddSource(m_pkPhysXSrc);

	// PhysX --> Gamebryo 동기자를 생성한다.
	m_pkPhysXDest = NiNew NiPhysXTransformDest(this, m_pkPhysXActor, 0);
	m_pkPhysXDest->SetActive(true);
	m_pkPhysXDest->SetInterpolate(false);
	pkPhysXScene->AddDestination(m_pkPhysXDest);

	if (GetWorld() && g_iUseAddUnitThread == 1)
	{
		GetWorld()->LockPhysX(false);
	}
	m_pkPhysXScene = pkPhysXScene;

}

void PgDropBox::ReleasePhysX()
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
		
		for(int i = 0; i < PG_MAX_NB_ABV_SHAPES && m_apkPhysXCollisionActors[i]; ++i)
		{
			m_pkPhysXScene->DeleteSource(m_apkPhysXCollisionSrcs[i]);
			if (m_pkPhysXScene->GetPhysXScene())
				m_pkPhysXScene->GetPhysXScene()->releaseActor(*m_apkPhysXCollisionActors[i]);
			m_apkPhysXCollisionActors[i] = 0;
		}
		
		if(m_pkController)
		{
			g_kControllerManager.releaseController(*m_pkController);
		}

		//if(m_pkPhysXActor && m_pkPhysXScene->GetPhysXScene())
		//{
		//	m_pkPhysXScene->GetPhysXScene()->releaseActor(*m_pkPhysXActor);
		//}

		if (GetWorld())
		{
			GetWorld()->LockPhysX(false);
		}
	}

	m_pkPhysXScene = 0;
	m_pkPhysXActor = 0;
	m_pkController = 0;
}

HRESULT PgDropBox::AddItem(PgBase_Item const &rkItem)
{
	m_kItemArray.push_back(rkItem);
	//_PgOutputDebugString("AddItem Num = %d m_kItemArray.size() = %d\n", rkItem.ItemNo(), m_kItemArray.size());
	return S_OK;
}

size_t PgDropBox::GetCount()const
{
	return m_kItemArray.size();
}

HRESULT PgDropBox::PopItem(BM::GUID const &rkItemGuid, PgBase_Item &rkOutItem)
{
	CONT_ITEM_ARRAY::iterator itor = m_kItemArray.begin();

	while(itor != m_kItemArray.end())
	{
		if((*itor).Guid() == rkItemGuid)
		{
			rkOutItem = (*itor);
			return S_OK;
		}
		++itor;
	}

	return E_FAIL;
}

HRESULT PgDropBox::PopItem(size_t const szPos, PgBase_Item &rkOutItem)
{
	PG_ASSERT_LOG(m_kItemArray.size() > szPos);
	if(m_kItemArray.size() <= szPos || m_kItemArray.size() > MAX_BOXITEM)
	{
		//_PgOutputDebugString("m_kItemArray.size() = %d\n", m_kItemArray.size());
		return E_FAIL;
	}

	rkOutItem = m_kItemArray.at(szPos);
	BM::GUID const &kGuid = rkOutItem.Guid();

	CONT_ITEM_ARRAY::iterator itor = m_kItemArray.begin();

	while(itor != m_kItemArray.end())
	{
		if((*itor).Guid() == kGuid)
		{
			m_kItemArray.erase(itor);
			return ERROR_SUCCESS;
		}
		++itor;
	}
	
	return E_FAIL;
}


PgBase_Item *PgDropBox::GetItem(size_t szPos)
{
	PG_ASSERT_LOG(szPos < GetCount());
	if(szPos >= GetCount())
	{
		return 0;
	}

	return &m_kItemArray.at(szPos);
}

int PgDropBox::ItemCount()
{
	return m_kItemArray.size();
}

bool	PgDropBox::IsMoney()
{
	int const iAbil = GetAbil();
	switch(iAbil)
	{
	case 0:
	case 400001:
	case 400005:
	case 400008:
	case 410001:
	case 410005:
	case 410008:
	case 400010:
	case 400011:
	case 410010:
	case 410011:
		return	true;
	}
	return	false;
}
bool	PgDropBox::IsEquip()
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(ItemNum());
	if(!pkItemDef) return	false;

	return pkItemDef->IsType(ITEM_TYPE_EQUIP);
}
bool	PgDropBox::IsConsume()
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(ItemNum());
	if(!pkItemDef) return	false;

	return pkItemDef->IsType(ITEM_TYPE_CONSUME);
}
bool	PgDropBox::IsETC()
{
	if(IsMoney()) return	false;

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(ItemNum());
	if(!pkItemDef) return	true;

	if(pkItemDef->IsType(ITEM_TYPE_EQUIP) || pkItemDef->IsType(ITEM_TYPE_CONSUME)) return	false;

	return	true;
}

bool PgDropBox::CheckPickup()
{
	PgBase_Item * pItem = this->GetItem(0);
	if( !pItem )
	{
		return false;
	}
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(pItem->ItemNo());
	if( !pDef )
	{
		return false;
	}
	if( 0 != pDef->GetAbil(AT_INSTANCE_ITEM) )
	{// 인스턴트 아이템은 픽업 금지
		return false;
	}

	return true;
}

bool PgDropBox::TransitAction(char const *pcAction)
{
	NiActorManager *pkAM = GetActorManager();
	PgActionSlot *pkActionSlot = GetActionSlot();
	if (pkAM == NULL || pkActionSlot == NULL)
		return false;

	// 엑션에 지정된 에니를 플레이한다
	NiActorManager::SequenceID kSeqID;
	if(!pkActionSlot->GetAnimation(std::string(pcAction), kSeqID))
	{
		return false;
	}
	pkAM->SetTargetAnimation(kSeqID);

	// 예전에 플레이한 사운드를 정지한다.
	/*
	if(m_pkCurrentAudioSource)
	{
		if(m_pkCurrentAudioSource->GetStatus() == NiAudioSource::PLAYING)
		{
			m_pkCurrentAudioSource->Stop();
			m_pkCurrentAudioSource = 0;
		}
	}
	*/
	
	// 엑션에 지정된 사운드를 플레이한다.

	PgActionSlot::stSoundInfo kSoundInfo;
	if(pkActionSlot->GetSound(std::string(pcAction), kSoundInfo))
	{
		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D,kSoundInfo.m_kSoundID.c_str(),kSoundInfo.m_fVolume,kSoundInfo.m_fMinDist,kSoundInfo.m_fMaxDist,this);
	}

	return true;
}

void PgDropBox::SetOwnerGuid(BM::GUID const &rkGuid)
{
	m_kOwnerGuid = rkGuid;

#ifdef PG_USE_DROPBOX_EAT_PATTERN1
	if (m_kOwnerGuid == BM::GUID::NullData() )
	{
		return;
	}

	m_pkOwnerPilot = g_kPilotMan.FindPilot(m_kOwnerGuid);
	if (m_pkOwnerPilot)
	{
		m_pkOwnerActor = dynamic_cast<PgActor *>(m_pkOwnerPilot->GetWorldObject());
	}
#endif
}

BM::GUID& PgDropBox::GetOwnerGuid()
{
	return m_kOwnerGuid;
}

bool PgDropBox::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();

	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);

			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "DROPBOX") == 0)
			{
				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "WORLDOBJECT") == 0)
			{
				PgIWorldObjectBase *pkNewBase = NiNew PgIWorldObjectBase;
				if(pkNewBase->ParseXml(pkNode, this))
				{
					m_kNowDropTransform.m_fScale = pkNewBase->GetScale();
					SetObjectID(PgIXmlObject::ID_DROPBOX);
				}
				else
				{
					SAFE_DELETE_NI(pkNewBase);
					return false;
				}
			}
			else if(strcmp(pcTagName, "DROP_TRANSFORM") == 0)
			{
				TiXmlAttribute const* pAttr = pkElement->FirstAttribute(); 

				while(pAttr)
				{
					char const *pcAttrName = pAttr->Name();
					char const *pcAttrValue = pAttr->Value();
					if(strcmp(pcAttrName, "DROP_SOUND") == 0)
					{
						m_kNowDropTransform.m_strDropSoundName = pcAttrValue;
					}
					else if(strcmp(pcAttrName, "CATCH_SOUND") == 0)
					{
						m_kNowDropTransform.m_strCatchSoundName = pcAttrValue;
					}
					else if(strcmp(pcAttrName, "EFFECT") == 0)
					{
						m_kNowDropTransform.m_wstrEffectName = UNI(pcAttrValue);
					}

					pAttr = pAttr->Next();
				}
			}
			else if( 0 == ::strcmp(pcTagName, "PARTICLE") )
			{
				TiXmlAttribute const* pAttr = pkElement->FirstAttribute();

				while( pAttr )
				{
					char const * pAttrName = pAttr->Name();
					char const * pAttrValue = pAttr->Value();
					if( 0 == ::strcmp(pAttrName, "NAME") )
					{
						m_ParticleName = pAttrValue;
					}
					else if( 0 == ::strcmp(pAttrName, "NODE") )
					{
						m_ParticleNode = pAttrValue;
					}

					pAttr = pAttr->Next();
				}
			}
			else if( 0 == ::strcmp(pcTagName, "OWNER") )
			{
				TiXmlAttribute const* pAttr = pkElement->FirstAttribute(); 
				while(pAttr)
				{
					char const *pcAttrName = pAttr->Name();
					char const *pcAttrValue = pAttr->Value();
					if( 0 == ::strcmp(pcAttrName, "EFFECT") )
					{
						m_strOwnerEffect = pcAttrValue;
					}
					else if ( 0 == ::strcmp(pcAttrName, "EFFECT_TARGET") )
					{
						m_strOwnerEffect_TargetName = pcAttrValue;
					}

					pAttr = pAttr->Next();
				}
			}
			else if( 0 == ::strcmp(pcTagName, "TYPE") )
			{
				TiXmlAttribute const* pAttr = pkElement->FirstAttribute();

				while( pAttr )
				{
					char const * pcAttrName = pAttr->Name();
					char const * pcAttrValue = pAttr->Value();
					if( 0 == ::strcmp(pcAttrName, "LOVE_BASEITEM") )
					{
						m_bLove_BaseItem = atoi(pcAttrValue) ? true : false;
					}

					pAttr = pAttr->Next();
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


	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}

	// 모든 파싱이 끝났다면 DropBox를 초기화한다.
	if(strcmp(pkNode->Value(), "DROPBOX") == 0)
	{
//		m_kNowDropTransform.m_fScale = GetScale();
		InitDropBox();
		NiAVObject::UpdateProperties();
		NiAVObject::UpdateEffects();
		NiAVObject::Update(0.0f);
	}

	return true;
}

void PgDropBox::SetName(std::wstring const &wName)
{
	m_kName = wName;
}

void PgDropBox::SetItemNum(int const iNum)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(iNum);

	if (pDef)
	{
		std::wstring wstrName;
		PgBase_Item tempItem;
		MakeItemName(iNum, tempItem.EnchantInfo(), wstrName);
		SetName(wstrName);
		ItemNum(iNum);
		SetOriginMesh();
	}
}

void PgDropBox::SetItemNum(PgBase_Item const &rkItem)
{
	if (rkItem.IsEmpty())
	{
		m_kName = _T("");
		return;
	}

	std::wstring wstrName;
	MakeItemName(rkItem.ItemNo(), rkItem.EnchantInfo(), wstrName);
	SetName(wstrName);
	m_kItemArray.push_back(rkItem);
	ItemNum(rkItem.ItemNo());
	SetOriginMesh();

	E_ITEM_GRADE const eItemGrade = GetItemGrade(rkItem);
	bool const bIsCash = CheckIsCashItem(rkItem);

	DWORD dwColor = 0xFFFFFFFF;//흰색.
	if(	IG_CURSE != eItemGrade	//저주 아니고
	&&	IG_SEAL != eItemGrade )	//봉인 아니고.
	{
		dwColor = SetGradeColor(eItemGrade, bIsCash, wstrName);
	}

	SetNameColor(dwColor);
}

//bool PgDropBox::CalcIsMine()
//{
//	PgPilot* pkPilot = GetPilot();
//	if( !pkPilot )
//	{
//		return false;
//	}
//
//	PgGroundItemBox* pkItemBox = dynamic_cast<PgGroundItemBox*>(pkPilot->GetUnit());
//
//	bool bMine = false;
//
//	if( g_pkWorld )
//	{
//		bMine = g_pkWorld->IsMineItemBox(pkItemBox);
//	}
//
//	return bMine;
//}

void PgDropBox::SetOriginMesh()
{
	if (m_bMeshLoaded)
	{
		m_bMeshLoaded = true;
	}
	int const iAbil = GetAbil();
	int const iAbil2 = GetAbil(AT_BEAR_ITEM);
	if (420000 == iAbil || 430000 == iAbil || 400020 == iAbil
		|| iAbil2 ) // 이 라인은 러브모드 곰 아이템
	//if (410002==iAbil || 410004==iAbil || 400002==iAbil || 400004==iAbil)	//임시
	{
		LoadTransformXml("Object/DropTransform.xml");

		int const iType = GetAbil(AT_DROP_TRANSFORM_TYPE);
		DropTransformList::iterator trns_it = ms_kDropTransformList.find(iType);
		if(ms_kDropTransformList.end() != trns_it)
		{
			m_kNowDropTransform = (*trns_it).second;
		}
		else
		{
			PG_ASSERT_LOG(0);
		}

		if (ItemNum() <= 0)
		{
			ItemNum(0);
		}

		m_pkItemEX = g_kItemMan.GetItem(ItemNum(), 1, 0);
		if (m_pkItemEX)
		{
			NiNode* pMeshRoot = m_pkItemEX->GetMeshRoot();
			if (pMeshRoot)
			{
				int const iCount = GetArrayCount();
				for (int i = 0; i < iCount; ++i)
				{
					DetachChildAt(i);
					NiAVObject::UpdateProperties();
					NiAVObject::UpdateEffects();
					NiAVObject::Update(0.0f);
				}
				char const* pkName = m_pkItemEX->GetTargetPoint();
				if (pkName)
				{
					NiAVObject* pTarget = pMeshRoot->GetObjectByName(pkName);
					PG_ASSERT_LOG(pTarget);
					if(pTarget)
					{
						pTarget->SetTranslate(0,0,0);
					}
				}
				
				pMeshRoot->SetScale(m_kNowDropTransform.m_fScale);
				//pMeshRoot->SetTranslate(pMeshRoot->GetTranslate()+m_kNowDropTransform.m_kPos);
				pMeshRoot->SetRotate(m_kNowDropTransform.m_kQuat);

				AttachChild(pMeshRoot, true);
				
				InitDropBox();
				NiAVObject::UpdateProperties();
				NiAVObject::UpdateEffects();
				NiAVObject::Update(0.0f);
			}
			m_pkAM = m_pkItemEX->GetActorManager();
		}
	}
	
	{
		if (!IsMine())
		{
			//알파를 빼기 위해 블랜딩 프로퍼티를 더한다
			{
				NiAlphaProperty *pkAlphaProp = NiNew NiAlphaProperty;
				pkAlphaProp->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);
				pkAlphaProp->SetTestRef(10);
				pkAlphaProp->SetAlphaTesting(true);
				pkAlphaProp->SetAlphaBlending(true);
				pkAlphaProp->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
				pkAlphaProp->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
				AttachProperty(pkAlphaProp);
				UpdateProperties();
			}
			NiNode *pkRoot = NiDynamicCast(NiNode, this);
			if (pkRoot)
			{
				SetTotalAlpha(pkRoot, 0.5f);
				pkRoot->UpdateProperties();
				pkRoot->UpdateEffects();
				pkRoot->Update(0.0f);
			}
		}
	}
	m_bMeshLoaded = true;
}

bool PgDropBox::SetTotalAlpha(NiNode *pkRoot, float const fAlpha)
{
	if (!pkRoot){ return false; }

	unsigned int uiArrayCount = pkRoot->GetArrayCount();
	for (unsigned int i = 0; i < uiArrayCount; i++)
	{
		NiAVObject* pkChild = pkRoot->GetAt(i);
		if(!pkChild)
		{
			continue;
		}
		if(NiIsKindOf(NiGeometry, pkChild))
		{
			NiGeometry *pkGeometry = NiDynamicCast(NiGeometry ,pkChild);
			if (pkGeometry)
			{
				NiMaterialProperty *pkMaterialProp = pkGeometry->GetPropertyState()->GetMaterial();
				NiAlphaProperty *pkAlpha = NiDynamicCast(NiAlphaProperty, pkGeometry->GetProperty(NiProperty::ALPHA));
				if (pkAlpha)
				{
					pkAlpha->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);
					pkAlpha->SetTestRef(10);
					pkAlpha->SetAlphaTesting(true);
				}
				if (pkMaterialProp)
				{
					pkMaterialProp->SetAlpha(fAlpha);
				}
			}
		}
		SetTotalAlpha(NiDynamicCast(NiNode,pkChild), fAlpha);
	}

	return true;
}

void PgDropBox::SetNameColor(DWORD dwColor)
{
	DWORD dw0x = 0x000000ff;
	m_kNameColor.b = (dwColor & dw0x)/255.0f;
	m_kNameColor.g = (dwColor>>8 & dw0x)/255.0f;
	m_kNameColor.r = (dwColor>>16 & dw0x)/255.0f;
	m_kNameColor.a = 1.0f;
}

void PgDropBox::ChaseOwner()
{
	//StartTrail();
	if(!g_pkWorld)
	{
		return;
	}

	m_bChaseOwner = true;
	m_fChaseStartTime = g_pkWorld->GetAccumTime();
	m_eBoxState = E_BOX_EAT;
	m_fSinProgress = 0;

    NewWare::Scene::ApplyTraversal::Geometry::HideParticleSystems( this, true );

	bool bResult = TransitAction("open");
	//PgParticleMan::ChangeParticleGeneration(m_spParticleNode, false);

	ReleaseABVShapes();

	std::string kSound = "Catch_Weapon";
	if(m_kNowDropTransform.m_strCatchSoundName.empty())
	{
		int const iAbil = GetAbil();
		switch(iAbil)
		{
		case 0:
		case 400001:
		case 400005:
		case 400008:
		case 410001:
		case 410005:
		case 410008:
		case 400010:	//돈주머니
		case 400011:
		case 410010:
		case 410011:
			kSound = "Catch_Coin";
			break;
			/*case 400002:
			case 410002:
			case 400004:
			case 410004:
			kSound = _T("Catch_Weapon");
			break;*/
		case 400003:
		case 400007:
		case 400009:
		case 410003:
		case 410007:
		case 410009:
		case 400012:	//마력탄
		case 410012:
			kSound = "Catch_ETC";
			break;
		case 400006:
		case 410006:
			kSound = "Catch_UseItem";
			break;
		default:
			break;
		}
	}
	else
	{
		kSound = m_kNowDropTransform.m_strCatchSoundName;
	}

	PickupSound(kSound);
	PgCircleShadow::DetachCircleShadowRecursive(this);
}
void	PgDropBox::SetHide(bool bHide)
{
	if(bHide)
	{
		m_eBoxState = E_BOX_HIDE;
	}
	SetAppCulled(bHide);
}
bool	PgDropBox::GetHide()
{
	return	(m_eBoxState == E_BOX_HIDE);
}


void PgDropBox::StartJump(float fHeight)
{
//	m_fJumpAccumHeight = 0.0f;
	m_bFloor = false;
	m_bJump = true;
	m_fJumpTime = 0.0f;
	m_fInitialVelocity = NiSqrt(2.0f * -ms_fGravity * fHeight);
	m_eBoxState = E_BOX_JUMP;

    NewWare::Scene::ApplyTraversal::Geometry::HideParticleSystems( this, false );

	bool bResult = TransitAction("drop");

	m_kptOriginPos = GetWorldTranslate() + m_kNowDropTransform.m_kOriginPos;

	SetPosition(m_kOffererPos);


	//StartTrail();
}

bool PgDropBox::SetOfferer(BM::GUID const &rkGuid, NiPoint3 const &rkOffererPos, float const fJumpHeight)
{
	m_kptOriginPos = GetWorldTranslate() + m_kNowDropTransform.m_kOriginPos;
	m_kOffererGuid = rkGuid;
	m_kOffererPos = rkOffererPos + NiPoint3(0.f, 0.f, 7.f);

	int const iCur = BM::Rand_Index(37);
	float fFinalJumpHeight = 3.7f + (iCur * 0.1f);
	if(fJumpHeight > 0)
	{
		fFinalJumpHeight = fJumpHeight;
	}
	StartJump(fFinalJumpHeight);

	m_kCreateTime = g_pkApp->GetAccumTime();

	std::string kSound = "Drop_Weapon";
	if(m_kNowDropTransform.m_strDropSoundName.empty())
	{
		int const iAbil = GetAbil();
		switch(iAbil)
		{
		case 0:
		case 400001:
		case 400005:
		case 400008:
		case 410001:
		case 410005:
		case 410008:
		case 400010:
		case 400011:
		case 410010:
		case 410011:
			{
				SetOriginMesh();
				kSound = "Drop_Coin";
			}break;
			/*case 400002:
			case 410002:
			case 400004:
			case 410004:
			kSound = _T("Drop_Weapon");
			break;*/
		case 400003:
		case 400007:
		case 400009:
		case 410003:
		case 410007:
		case 410009:
		case 400012:	//마력탄
		case 410012:
			{
				kSound = "Drop_ETC";
			}break;
		case 400006:
		case 410006:
			{
				kSound = "Drop_UseItem";
			}break;
		default:
			{
			}break;
		}
	}
	else
	{
		kSound = m_kNowDropTransform.m_strDropSoundName;
	}
	
	PlaySound(kSound.c_str());

	if(IsMine() && !m_kNowDropTransform.m_wstrEffectName.empty())
	{
		NiAVObject *pkParticle = g_kParticleMan.GetParticle(MB(m_kNowDropTransform.m_wstrEffectName),PgParticle::O_SCALE, 1.0f );
		AttachChild(pkParticle, true);
	}

	return false;
}

BM::GUID& PgDropBox::GetOffererGuid()
{
	return m_kOffererGuid;
}

bool PgDropBox::LoadTransformXml(char const* szFile)
{
	if (ms_bLoaded == true)
	{
		return false;
	}

	PgDropBox::ms_kDropTransformList.clear();
	if (NULL == szFile || !PgDropBox::ms_kDropTransformList.empty())//!!!!!!!XML수정이 완료되면 주석 풀어야 함
	{
		return false;
	}
	
	TiXmlDocument *pkXmlDoc = PgXmlLoader::GetXmlDocumentInCacheByPath(szFile);

	if (pkXmlDoc == NULL)
	{
		return false;
	}

	TiXmlNode *pkRootNode = pkXmlDoc->FirstChild();
	if (!pkRootNode)
	{
		return false;
	}

	ParseTransformXml(pkRootNode);

	ms_bLoaded = true;
	return true;
}

bool PgDropBox::ParseTransformXml(const TiXmlNode *pkNode, void *pArg)
{
	int const iType = pkNode->Type();

	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);

			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "DROP_TRANSFORM") == 0)
			{
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseTransformXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "ITEM") == 0)
			{
				TiXmlAttribute* pkAttr = pkElement->FirstAttribute();
				SDropTransform kSrt;
				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if (strcmp(pcAttrName, "ID") == 0)
					{
						kSrt.m_iID = atoi(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "EFF_NAME") == 0)
					{
						kSrt.m_wstrEffectName = UNI(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "ROT_SPEED") == 0)
					{
						kSrt.m_fRotSpeed = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "SCALE") == 0)
					{
						kSrt.m_fScale = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "ROT_X") == 0)
					{
						kSrt.m_kQuat.m_fX = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "ROT_Y") == 0)
					{
						kSrt.m_kQuat.m_fY = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "ROT_Z") == 0)
					{
						kSrt.m_kQuat.m_fZ = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "ROT_ANGLE") == 0)
					{
						kSrt.m_kQuat.m_fW = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "POS_X") == 0)
					{
						kSrt.m_kPos.x = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "POS_Y") == 0)
					{
						kSrt.m_kPos.y = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "POS_Z") == 0)
					{
						kSrt.m_kPos.z = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "ORIGIN_X") == 0)
					{
						kSrt.m_kOriginPos.x = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "ORIGIN_Y") == 0)
					{
						kSrt.m_kOriginPos.y = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "ORIGIN_Z") == 0)
					{
						kSrt.m_kOriginPos.z = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "UPDOWN") == 0)
					{
						kSrt.m_fUpDown = (float)atof(pcAttrValue);
					}
					else if (strcmp(pcAttrName, "UPDOWN_SPEED") == 0)
					{
						kSrt.m_fUpDownSpeed = (float)atof(pcAttrValue);
					}

					else
					{
						PgError1("PgItemEx : Unknown attribute - %s", pcAttrName);
					}

					pkAttr = pkAttr->Next();
				}

				kSrt.m_kQuat.Normalize();
				ms_kDropTransformList.insert(std::make_pair(kSrt.m_iID, kSrt));
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
				break;
			}
		}break;
	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseTransformXml(pkNextNode))
		{
			return false;
		}
	}

	return true;
}

int const PgDropBox::GetAbil(WORD const wAbil)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pkItemDef = kItemDefMgr.GetDef(ItemNum());
	if(pkItemDef)
	{
		int iRet = pkItemDef->GetAbil(wAbil);
		if (0==iRet)
		{
			if (AT_DROPITEMRES==wAbil)
			{
				iRet = 400009;
			}
		}
		return iRet;
	}

	return 0;
}

NiActorManager *PgDropBox::GetActorManager() const
{
	if(PgIWorldObject::GetActorManager())
	{
		return PgIWorldObject::GetActorManager();
	}
	
	return m_pkAM;

}

void PgDropBox::InitDropBox()
{// 드롭 박스를 초기화 하는데
	NiActorManager *pkAM = GetActorManager();
	if ( !pkAM )
	{
		return;
	}
	pkAM->Update(0.0f);
	
	if( m_ParticleName.empty()		// 파티클 ID
		|| m_ParticleNode.empty()	// 파티클이 붙을 노드
		)
	{//를 확인하고
		return;
	}
	// 파티클을 얻어와
	PgParticle * pParticle = g_kParticleMan.GetParticle(m_ParticleName.c_str(), PgParticle::O_SCALE, 1.0f );
	if( !pParticle )
	{
		return;
	}
	NiAVObject* pRoot = GetNIFRoot();
	if( !pRoot )
	{// 최상위 노드의
		return;
	}
	// 붙여야할 노드를 찾아서
	NiNode* pNodeRet = NiDynamicCast( NiNode, pRoot->GetObjectByName(m_ParticleNode.c_str()) );
	if( !pNodeRet )
	{
		return;
	}
	// 파티클을 그 노드에 붙여준다.
	pNodeRet->AttachChild( NiDynamicCast(NiAVObject, pParticle) , true);
}

bool PgDropBox::PlaySound(char const *szPath)
{ 
	if (!szPath)
	{
		return false;
	}
	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D,szPath,0.0f,80,180,this);

	return false;
}

void PgDropBox::AttachCircleShadow(NiPoint3 const& rkPos)
{
	if(!GetWorld()) {return;}

	NxRay kRay(NxVec3(rkPos.x, rkPos.y, rkPos.z+20.0f), NxVec3(0.0f, 0.0f, -1.0f));
	NxShape *pkHitShape = GetWorld()->raycastClosestShape(kRay, NX_STATIC_SHAPES, m_kBottomRayHit, DEFAULT_ACTIVE_GRP, 10000.0f, NX_RAYCAST_SHAPE|NX_RAYCAST_FACE_NORMAL|NX_RAYCAST_DISTANCE|NX_RAYCAST_IMPACT);
	if(!pkHitShape)	{return;}
	
	if(PgCircleShadow::AttachCircleShadowRecursive(this,200.0f,0.5f,0,0,&m_kBottomRayHit) == 0)
	{
		int iRet = PgCircleShadow::AttachCircleShadowRecursive(this,200.0f,0.5f,0,this,&m_kBottomRayHit);
	}

	m_kptOriginPos.z = m_kBottomRayHit.worldImpact.z;
}

void PgDropBox::RemoveNextUpdate()
{
	RemoveReserve(true);
	GetWorld()->RemoveObjectOnNextUpdate(GetGuid());
}