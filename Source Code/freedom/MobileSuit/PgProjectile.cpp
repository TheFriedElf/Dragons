#include "stdafx.h"
#include "Variant/Global.h"
#include "PgPostProcessMan.h"
#include "PgProjectile.h"
#include "PgRenderer.h"
#include "PgActor.h"
#include "PgNifMan.h"
#include "lwProjectile.h"
#include "PgProjectileMan.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgItemEx.h"
#include "PgMobileSuit.H"
#include "PgMath.H"
#include "PgTrail.H"
#include "PgSoundMan.h"
#include "PgWorld.h"
#include "lohengrin/PacketStruct4Map.h"
#include "PgNetwork.h"
#include "lwActionTargetList.H"
#include "PgCircleShadow.H"
#include "PgAction.h"
#include "PgParticleMan.h"
#include "PgPOTParticle.h"
#include "NewWare/Scene/ApplyTraversal.h"

const	float	g_fDefaultProjectileCollisionSphereRange = 30.0f;

PgProjectile::PgProjectile()
	:m_State(S_UNDEFINED),
	m_TargetType(TT_UNDEFINED),
	m_ScriptCallEvent(SCE_NONE),
	m_bDestroyMe(false),
	m_spNif(0),
	m_fAccumTime(0.0f),
	m_vVelocity(0,0,0),
	m_vTarget(0,0,0),
	m_fSpeed(0),
	m_fMass(0),
	m_iUID(-1),
	m_fAccelation(0),
	m_iParentActionNo(0),
	m_iParentActionInstanceID(0),
	m_kTextureFolder(""),
	m_MovingType(MT_STRAIGHTLINE),
	m_pkTrail(NULL),
	m_bAutoDirection(true),
	m_kTrailAttachNodeName("arrow_head"),
	m_bHide(false),
	m_dwParentActionTimeStamp(0),
	m_kParentGUID(BM::GUID::NullData()),
	m_bCollCheckByMe(false),
	m_bTargetListBroadCasted(false),
	m_iTrailTotalTime(0),
	m_iTrailBrightTime(0),
	m_fTrailExtendLength(0.0f),
	m_fTrailWidth(4.0f),
	m_bMultipleAttack(false),
	m_fLastCollideTime(0.0f),
	m_kTargetGuid(BM::GUID::NullData()),
	m_kHomingTime(0.0f),	//기본 0초
	m_kHomingDelayTime(0.0f),
	m_kStayTime(0.0f),
	m_kAccumDistance(0.0f),
	m_kClearTargetList(true),
	m_kPenetrationCount(0),
	m_bCollIsOnyTargetLoc(false),
	m_fCollisionCheckSec(0.5f),		// 기본 체크값이며 현재까지 유지되어오던 값이므로 0.5f 상수를 바꾸지 않길 바람
	m_bNoShadow(false),
	m_kAutoGround(0.0f),
	m_bIgnoreDefinedRange(false),
	m_fCollisionScale(1.0f)
{
}
PgProjectile*	PgProjectile::CreateClone()
{
	PgProjectile *pkNew = NiNew PgProjectile();
	pkNew->m_fOriginalAccelation = m_fOriginalAccelation;
	pkNew->m_fOriginalSpeed = m_fOriginalSpeed;
	pkNew->m_fSpeed = m_fOriginalSpeed;
	pkNew->m_fAccelation = m_fOriginalAccelation;
	pkNew->m_vVelocity = m_vVelocity;
	pkNew->m_vTarget = m_vTarget;
	pkNew->m_fMass = m_fMass;
	pkNew->m_fAccumTime = m_fAccumTime;
	pkNew->m_kScriptName = m_kScriptName;
	pkNew->m_TargetType = m_TargetType;
	pkNew->m_State = m_State;
	pkNew->m_ScriptCallEvent = m_ScriptCallEvent;;
	pkNew->m_kID = m_kID;
	pkNew->m_bDestroyMe = m_bDestroyMe;
	pkNew->m_spNif = static_cast<NiNode*>(m_spNif->Clone());
	pkNew->m_spNif->UpdateProperties();
	pkNew->m_spNif->UpdateEffects();
	pkNew->m_spNif->Update(0,true);
	pkNew->m_kTextureFolder = m_kTextureFolder;
	pkNew->m_kTrailTexturePath = m_kTrailTexturePath;
	pkNew->m_iTrailTotalTime = m_iTrailTotalTime;
	pkNew->m_iTrailBrightTime = m_iTrailBrightTime;
	pkNew->m_fTrailExtendLength = m_fTrailExtendLength;
	pkNew->SetAutoDirection(GetAutoDirection());
	pkNew->m_kTrailAttachNodeName = m_kTrailAttachNodeName;
	pkNew->m_CollisionSphereCont = m_CollisionSphereCont;
	pkNew->ResetCollisionSphereBeforePos();
	pkNew->m_fTrailWidth = m_fTrailWidth;
	pkNew->m_bMultipleAttack = m_bMultipleAttack;
	pkNew->HomingTime(HomingTime());
	pkNew->HomingDelayTime(HomingDelayTime());
	pkNew->StayTime(StayTime());
	pkNew->InitTrailNode();
	pkNew->ClearTargetList(true);//이놈은 무조건 기본으로 True다
	pkNew->SetMovingType(m_MovingType);
	pkNew->PenetrationCount(m_kPenetrationCount);
	pkNew->SetOptionInfo(m_kOptionInfo);
	pkNew->SetCollIsOnyTargetLoc(m_bCollIsOnyTargetLoc);
	pkNew->SetCollisionCheckSec(m_fCollisionCheckSec);
	pkNew->m_bNoShadow = m_bNoShadow;
	pkNew->m_kAutoGround = m_kAutoGround;
	pkNew->m_bIgnoreDefinedRange = m_bIgnoreDefinedRange;
	pkNew->m_fCollisionScale = m_fCollisionScale;

	if(false==m_bNoShadow)
	{
		PgCircleShadow::AttachNewShadowTo(pkNew->m_spNif,200.0f,0.4f);
	}

	NiTimeController::StartAnimations(pkNew->m_spNif);

	//	월드에 SpotLight 가 적용되지 않은 상태라면, 캐릭터의 GlowMap 을 제거한다.
	if(g_pkWorld)
	{
		PgRenderer::EnableGlowMap(pkNew->m_spNif,g_pkWorld->GetSpotLightOn());
	}
	return	pkNew;
}
void	PgProjectile::InitTrailNode()
{
	if(m_pkTrail)
	{
		return;
	}

	if(m_iTrailTotalTime>0)
	{
		m_pkTrail = g_spTrailNodeMan->StartNewTrail(NiDynamicCast(NiAVObject,m_spNif),
			m_kTrailAttachNodeName,
			m_kTrailTexturePath,
			m_iTrailTotalTime/1000.0f,
			m_iTrailBrightTime/1000.0f,
			m_fTrailWidth);
	}
}
void	PgProjectile::ResetCollisionSphereBeforePos()
{
	stCollisionSphere	*pkSphere = NULL;
	int	iTotalSphere = m_CollisionSphereCont.size();

	for(int i=0;i<iTotalSphere;++i)
	{
		pkSphere = &(m_CollisionSphereCont[i]);	
		pkSphere->m_kBeforePos = NiPoint3(0,0,0);
	}
}
void	PgProjectile::UpdateCollisionSphereBeforePos()
{
	stCollisionSphere	*pkSphere = NULL;
	int	iTotalSphere = m_CollisionSphereCont.size();

	for(int i=0;i<iTotalSphere;++i)
	{
		pkSphere = &(m_CollisionSphereCont[i]);	

		NiAVObject* pkTargetNode = m_spNif->GetObjectByName(pkSphere->m_kTargetNode.c_str());
		if(pkTargetNode == NULL)
		{
			pkTargetNode = m_spNif;
		}
		NiPoint3 const	&kTargetNodePos = pkTargetNode->GetWorldTranslate();

		pkSphere->m_kBeforePos = kTargetNodePos;
	}
}

void	PgProjectile::UpdatePos(float fAccumTime, float fFrameTime)
{

	/*NiPoint3	ptTarget;
	switch(m_TargetType)
	{
	case TT_OBJECT:
		{
			ptTarget = m_vTarget;
		}
		break;
	case TT_LOCATION:
		{
			ptTarget = m_vTarget;
		}
		break;
	}*/

	NiPoint3 kPastPos = m_spNif->GetWorldTranslate();

	bool const bIsTarget = !BM::GUID::IsNull(TargetGuid());

	if(MT_STAY==m_MovingType)
	{
		// 따로 처리
	}
	else if (bIsTarget)
	{
		if(HomingDelayTime() <= 0.0f)
		{
			PgActor* pkTarget = g_kPilotMan.FindActor(TargetGuid());
			if (pkTarget)
			{
				NiNode*	pkNode = NULL;
				if( (false == TargetEndNode().empty())
				 && (pkNode = (NiNode*)pkTarget->GetNIFRoot()->GetObjectByName(TargetEndNode().c_str())) )
				{
					m_vTarget = pkNode->GetWorldTranslate();
				}
				else
				{
					m_vTarget = pkTarget->GetPos();
				}
				m_TargetType = TT_OBJECT;
			}	
			if (MT_STRAIGHTLINE==GetMovingType())
			{
				SetMovingType(MT_HOMMING);
				if(g_pkWorld && m_spNif)
				{
					m_vFireStartPos = m_spNif->GetTranslate();
					m_fFireStartTime = g_pkWorld->GetAccumTime();
				}
			}
		}
		else
		{
			HomingDelayTime(HomingDelayTime() - fFrameTime);
		}
	}

	switch(m_MovingType)
	{
	case MT_STRAIGHTLINE:
		UpdatePos_StraightLineType(fAccumTime,fFrameTime);
		break;
	case MT_SIN_CURVELINE:
		UpdatePos_SinCurveLineType(fAccumTime,fFrameTime);
		break;
	case MT_BEZIER4_SPLINE:
		UpdatePos_Bezier4SplineType(fAccumTime,fFrameTime);
		break;
	case MT_HOMMING:
		UpdatePos_HommingType(fAccumTime,fFrameTime);
		break;
	case MT_STAY:
		UpdatePos_Stay(fAccumTime,fFrameTime);
		break;
	}

	if (/*m_MovingType!=MT_HOMMING ||*/ bIsTarget)	//유도가 있을 경우에는 검사필요
	{
		NiPoint3 kNewPos = m_spNif->GetWorldTranslate();
		NiPoint3 kPastPosTemp = kPastPos;
		kPastPosTemp.z = 0;
		kNewPos.z = 0;
		NiPoint3 kDist = kNewPos - kPastPosTemp;
		AccumDistance(AccumDistance()+kDist.Length());

		GET_DEF(CSkillDefMgr, kSkillDefMgr);

		CSkillDef const * pkDef = kSkillDefMgr.GetDef(m_iParentActionNo);

		if (pkDef)
		{
			if (pkDef->m_sRange < AccumDistance())
			{
				StartRemove();
			}
		}
	}

	//	Check whether this projcetile hit a phyx object or not
	//	and, if true, we destroy the projectile.
	
	NiPoint3 kNewPos = ProcessorAutoGround(m_spNif, kPastPos, fAccumTime);

	if(GetCollIsOnyTargetLoc())
	{
		NiPoint3 kMovePos = kNewPos - m_vTarget;
		float fMoveDistance = kMovePos.Length();
		if(fMoveDistance > 10)
		{
			// 아직 충돌한게 아님
			return;
		}

	}

	NiPoint3	vDir = kNewPos - kPastPos;
	float	fDistance =vDir.Unitize();
	if(g_pkWorld && fDistance > 0)
	{
		NxVec3 kNxDir(vDir.x, vDir.y, vDir.z);
		kNxDir.normalize();
		NxRaycastHit kHit;
		NxShape *pkShape = g_pkWorld->raycastClosestShape(NxRay(NxVec3(kPastPos.x,kPastPos.y,kPastPos.z),kNxDir),NX_STATIC_SHAPES, kHit, DEFAULT_ACTIVE_GRP, fDistance,NX_RAYCAST_SHAPE);
		if(pkShape)
		{
			OnArrivedAtTarget();
		}
		else
		{
			pkShape = g_pkWorld->raycastClosestShape(NxRay(NxVec3(kNewPos.x,kNewPos.y,kNewPos.z),-kNxDir),NX_STATIC_SHAPES, kHit, DEFAULT_ACTIVE_GRP, fDistance,NX_RAYCAST_SHAPE);
			if(pkShape)
			{
				OnArrivedAtTarget();
			}
		}
	}

	//_PgOutputDebugString("PgProjectile::UpdatePos fAccumTime:%f m_MovingType:%d Pos:%f,%f,%f\n", fAccumTime,m_MovingType,m_spNif->GetTranslate().x,m_spNif->GetTranslate().y,m_spNif->GetTranslate().z);
}
void	PgProjectile::InitNifData(NiAVObject *pkObject)
{
	if(NiIsKindOf(NiNode,pkObject))
	{
		NiNode	*pkNode = NiDynamicCast(NiNode,pkObject);
		if(pkNode)
		{
			int	iTotalChild = pkNode->GetArrayCount();
			NiAVObject	*pkChild = NULL;
			for(int i=0;i<iTotalChild;++i)
			{
				pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					InitNifData(pkChild);
				}
			}
		}
	}
	else if(NiIsKindOf(NiGeometry,pkObject))
	{
		NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkObject);
		if(pkGeom)
		{
			NiMaterialProperty	*pkMat = pkGeom->GetPropertyState()->GetMaterial();
			if(!pkMat)
			{
				pkMat = NiNew NiMaterialProperty();
				pkGeom->AttachProperty(pkMat);
			}

			NiAlphaProperty	*pkAlpha = NiNew NiAlphaProperty();
			pkGeom->AttachProperty(pkAlpha);
			pkGeom->UpdateProperties();

			pkAlpha->SetAlphaBlending(true);
			pkAlpha->SetAlphaTesting(true);
			pkAlpha->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
			pkAlpha->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
			pkAlpha->SetTestRef(10);
			pkAlpha->SetAlphaGroup(AG_EFFECT);

		}
	}
}

void	PgProjectile::SetAlpha(float	fAlpha)
{
	SetAlpha(fAlpha,(NiAVObject*)m_spNif);
}
void	PgProjectile::SetAlpha(float	fAlpha,NiAVObject *pkObject)
{
	if(NiIsKindOf(NiNode,pkObject))
	{
		NiNode	*pkNode = NiDynamicCast(NiNode,pkObject);
		if(pkNode)
		{
			int	iTotalChild = pkNode->GetArrayCount();
			NiAVObject	*pkChild = NULL;
			for(int i=0;i<iTotalChild;++i)
			{
				pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					SetAlpha(fAlpha,pkChild);
				}
			}
		}
	}
	else if(NiIsKindOf(NiGeometry,pkObject))
	{
		NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkObject);
		if(pkGeom)
		{
			NiMaterialProperty	*pkMat = pkGeom->GetPropertyState()->GetMaterial();
			pkMat->SetAlpha(fAlpha);

			NiAlphaProperty	*pkAlpha = pkGeom->GetPropertyState()->GetAlpha();
			pkAlpha->SetAlphaBlending(true);
		}
	}
}

void	PgProjectile::LookAtTarget(NiPoint3 &kStart,NiPoint3 &kTarget,float fAccumTime)
{
	if(!GetAutoDirection())
		return;

	NiPoint3	kWorldUp(0,0,1);

	//	타겟을 바라보도록 회전시킨다.
	NiPoint3 kDir = kStart - kTarget;
	if (kDir.SqrLength() < 1e-10f)
		return;

	kDir.Unitize();

	NiPoint3 kRight = kWorldUp.UnitCross(kDir);
    
	// will either be unit length or zero
	if (kRight.SqrLength() < 0.5f)
		return;

	NiPoint3 kUp = kDir.UnitCross(kRight);

	// will either be unit length or zero
	if (kUp.SqrLength() < 0.5f)
		return;

	// Directly create the rotation matrix using column vectors
	NiMatrix3 kRot = NiMatrix3(-kDir, kUp, kRight);

	m_spNif->SetRotate(kRot);
}
void	PgProjectile::UpdatePos_StraightLineType(float fAccumTime, float fFrameTime)
{
	float	fElapsedTime = fAccumTime - m_fFireStartTime;
	float	fTotalDistance = 0.0f;//(m_vFireStartPos-m_vTarget).Length();
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const * pkDef = kSkillDefMgr.GetDef(m_iParentActionNo);
	if(pkDef && 0 < pkDef->m_sRange && false==m_bIgnoreDefinedRange)
	{
		fTotalDistance = static_cast<float>(pkDef->m_sRange);
	}
	else
	{
		fTotalDistance = (m_vFireStartPos-m_vTarget).Length();
	}
	float	fCurDistance = m_fAccelation*fElapsedTime*fElapsedTime*0.5f+m_fSpeed*fElapsedTime;

	//_PgOutputDebugString("PgProjectile::UpdatePos_StraightLineType m_vFireStartPos(%f,%f,%f) m_vTarget(%f,%f,%f) fTotalDistance:%f fCurDistance:%f\n", m_vFireStartPos.x,m_vFireStartPos.y,m_vFireStartPos.z, m_vTarget.x,m_vTarget.y,m_vTarget.z, fTotalDistance,fCurDistance);

	NiPoint3	kDir = m_vTarget-m_vFireStartPos;
	kDir.Unitize();
	NiPoint3	kNewPos = m_vFireStartPos + (kDir * fCurDistance);

	if(fCurDistance>=fTotalDistance)
	{
		m_spNif->SetTranslate(m_vTarget);
		m_spNif->Update(fAccumTime);

		fCurDistance = fTotalDistance;
		OnArrivedAtTarget();	
		return;
	}

	m_spNif->SetTranslate(kNewPos);
	LookAtTarget(m_vFireStartPos,m_vTarget,fAccumTime);
	m_spNif->Update(fAccumTime);
}

void	PgProjectile::UpdatePos_SinCurveLineType(float fAccumTime, float fFrameTime)
{
	float	fElapsedTime = fAccumTime - m_fFireStartTime;
	bool	bArrivedTarget = false;
	NiPoint3	kNewPos = GetLoc_SinCurveLineType(m_vFireStartPos,m_vTarget,fElapsedTime,true,bArrivedTarget);

	if(bArrivedTarget)
	{
		m_spNif->SetTranslate(m_vTarget);
		m_spNif->Update(fAccumTime);

		OnArrivedAtTarget();	
		return;
	}

	NiPoint3	kNextPos = GetLoc_SinCurveLineType(m_vFireStartPos,m_vTarget,(float)(fElapsedTime+0.001),false,bArrivedTarget);

	m_spNif->SetTranslate(kNewPos);
	LookAtTarget(kNewPos,kNextPos,fAccumTime);
	m_spNif->Update(fAccumTime);
}

NiPoint3	PgProjectile::GetLoc_SinCurveLineType(NiPoint3 &kStart,NiPoint3 &kEnd,float fElapsedTime,bool bAdjustOverEndPos,bool &bArrivedEnd)
{
	bArrivedEnd = false;

	float	fTotalDistance = (kStart-kEnd).Length();
	float	fCurDistance = m_fAccelation*fElapsedTime*fElapsedTime+m_fSpeed*fElapsedTime;
	if(fCurDistance>=fTotalDistance)
	{
		bArrivedEnd = true;
		if(bAdjustOverEndPos)
		{
			fCurDistance = fTotalDistance;
		}
	}

	float	fRate = (fCurDistance/fTotalDistance);
	NiPoint3	kNewPos = (kEnd-kStart)*fRate;
	kNewPos.z = (float)((kEnd.z-kStart.z)*fRate + m_fSinPower*sin(fRate*180.0*NI_PI/180.0f));

	//	회전시키기
	NiPoint3	kRotateAxis = (kEnd-kStart);
	kRotateAxis.Unitize();
	NiMatrix3	kRotMat;
	kRotMat.MakeRotation(m_fSinRotateAngle,kRotateAxis);
	kNewPos = kRotMat * kNewPos;
	kNewPos = kStart + kNewPos;

	return	kNewPos;
}
NiPoint3	PgProjectile::GetLoc_HommingType(float fElapsedTime,bool bAdjustOverEndPos,bool &bArrivedEnd)
{
	bArrivedEnd = false;

	NiPoint3	kControlPoint[4];

	//kControlPoint[0] = GetWorldPos();
	kControlPoint[0] = m_vFireStartPos;
	kControlPoint[3] = m_vTarget;

	NiPoint3	vToTarget = m_vTarget-m_vFireStartPos;
	float		fDistance = vToTarget.Length();

	vToTarget.Unitize();

	NiPoint3	kStartVel = m_vVelocity;
	kStartVel.Unitize();

	NiPoint3	kReflectionVel = GetReflectionVec(-vToTarget,kStartVel);
	kReflectionVel.Unitize();


	kControlPoint[1] = m_vFireStartPos+kStartVel*fDistance*0.4f;
	kControlPoint[2] = m_vTarget+kReflectionVel*fDistance*0.25f;


	return	GetLoc_Bezier4SplineType(
		kControlPoint[0],
		kControlPoint[1],
		kControlPoint[2],
		kControlPoint[3],
		fElapsedTime,
		bAdjustOverEndPos,
		bArrivedEnd);
}

void	PgProjectile::UpdatePos_HommingType(float fAccumTime, float fFrameTime)
{
	float	fElapsedTime = fAccumTime - m_fFireStartTime;
	bool	bArrivedTarget = false;
	NiPoint3	kNewPos = GetLoc_HommingType(fElapsedTime,true,bArrivedTarget);

	if(bArrivedTarget)
	{
		m_spNif->SetTranslate(m_vTarget);
		m_spNif->Update(fAccumTime);

		OnArrivedAtTarget();	
		return;
	}

	NiPoint3	kNextPos = GetLoc_HommingType(fElapsedTime+0.001f,false,bArrivedTarget);

	m_spNif->SetTranslate(kNewPos);
	LookAtTarget(kNewPos,kNextPos,fAccumTime);
	m_spNif->Update(fAccumTime);
}

void	PgProjectile::UpdatePos_Bezier4SplineType(float fAccumTime, float fFrameTime)
{
	float	fElapsedTime = fAccumTime - m_fFireStartTime;
	bool	bArrivedTarget = false;
	NiPoint3	kNewPos = GetLoc_Bezier4SplineType(m_kBezierControlPoint[0],m_kBezierControlPoint[1],m_kBezierControlPoint[2],m_kBezierControlPoint[3],fElapsedTime,true,bArrivedTarget);

	if(bArrivedTarget)
	{
		m_spNif->SetTranslate(m_vTarget);
		m_spNif->Update(fAccumTime);

		OnArrivedAtTarget();	
		return;
	}

	NiPoint3	kNextPos = GetLoc_Bezier4SplineType(m_kBezierControlPoint[0],m_kBezierControlPoint[1],m_kBezierControlPoint[2],m_kBezierControlPoint[3],fElapsedTime+0.001f,false,bArrivedTarget);

	m_spNif->SetTranslate(kNewPos);
	LookAtTarget(kNewPos,kNextPos,fAccumTime);
	m_spNif->Update(fAccumTime);
}
NiPoint3	PgProjectile::GetLoc_Bezier4SplineType(NiPoint3 &p1,NiPoint3 &p2,NiPoint3 &p3,NiPoint3 &p4,float fElapsedTime,bool bAdjustOverEndPos,bool &bArrivedEnd)
{
	bArrivedEnd = false;

	float fTotalDistance = (p4 - p1).Length() ; //(p2-p1).Length()+(p3-p2).Length()+(p4-p2).Length();
	float fCurDistance =  m_fAccelation*fElapsedTime*fElapsedTime+m_fSpeed*fElapsedTime;
	
	if(fCurDistance>=fTotalDistance)
	{
		bArrivedEnd = true;
		if(bAdjustOverEndPos)
		{
			fCurDistance = fTotalDistance;
		}
	}

	float	fRate = fCurDistance / fTotalDistance;
	NiPoint3 kNewPos;

	Bezier4(p1,p2,p3,p4,fRate,kNewPos);

	return	kNewPos;
}

void	PgProjectile::UpdatePos_Stay(float const fAccumTime, float const fFrameTime)
{
	if(PgActor * pkActor = g_kPilotMan.FindActor(GetParentPilotGUID()))
	{
		NiNode*	pkNode = NULL;

		if( (false==FireNode().empty())
		 && (pkNode = (NiNode*)pkActor->GetNIFRoot()->GetObjectByName(FireNode().c_str())) )
		{
			m_vFireStartPos = pkNode->GetWorldTranslate();
		}

		if(PgActor * pkTargetActor = g_kPilotMan.FindActor(TargetGuid()))
		{
			pkActor = pkTargetActor;
		}

		if( (false==TargetEndNode().empty())
		 && (pkNode = (NiNode*)pkActor->GetNIFRoot()->GetObjectByName(TargetEndNode().c_str())) )
		{
			m_vTarget = pkNode->GetWorldTranslate();
		}
	}

	if((fAccumTime - m_fFireStartTime) > StayTime())
	{
		OnArrivedAtTarget();
		return;
	}

	m_spNif->SetTranslate(m_vFireStartPos);
	LookAtTarget(m_vFireStartPos,m_vTarget,fAccumTime);
	m_spNif->Update(fAccumTime);
}

void	PgProjectile::SetParam(char const*pParamName,char const*pParamValue)
{
	ParamMap::iterator itor = m_ParamMap.find(pParamName);
	if(itor != m_ParamMap.end())
	{
		itor->second = std::string(pParamValue);
		return;
	}	

	m_ParamMap.insert(std::make_pair(std::string(pParamName),std::string(pParamValue)));
}
char const*	PgProjectile::GetParam(char const*pParamName)
{
	ParamMap::iterator itor = m_ParamMap.find(pParamName);
	if(itor != m_ParamMap.end())
	{
		return	itor->second.c_str();
	}	
	return	NULL;
}
void	PgProjectile::SetMovingType(MovingType kMovingType)
{
	m_MovingType = kMovingType;
}
void	PgProjectile::SetParam_SinCurveLineType(float fSinPower,float fSinRotateAngle)
{
	m_fSinPower = fSinPower;
	m_fSinRotateAngle = fSinRotateAngle;
}
void	PgProjectile::SetParam_Bezier4SplineType(NiPoint3 const &p1,NiPoint3 const &p2,NiPoint3 const &p3,NiPoint3 const &p4)
{
	m_kBezierControlPoint[0] = p1;
	m_kBezierControlPoint[1] = p2;
	m_kBezierControlPoint[2] = p3;
	m_kBezierControlPoint[3] = p4;
}

void PgProjectile::AttachSound(char const* pcSoundID, float fMin, float fMax)
{
	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D,pcSoundID,0.0,fMin,fMax,m_spNif);
}
bool PgProjectile::Update(float fAccumTime, float fFrameTime)
{
	if(m_spNif)
	{
		m_spNif->Update(fAccumTime);
	}

	//_PgOutputDebugString("PgProjectile::Update fAccumTime:%f m_State:%d\n", fAccumTime,m_State);

	switch(m_State)
	{
	case S_REMOVING:

		if(CheckAllParticleRemoved(m_spNif))
		{
			m_bDestroyMe = true;
		}
		return	true;

	case S_DELAY_FIRING:
		{
			if(fAccumTime - m_fDelayFireStartTime>=m_fDelayFireTotalTime)
			{
				if(GetHide())	{SetHide(false);}
				Fire(m_bNoResetTargetLoc);
			}
		}
		break;
	case S_LOADING:
		{
			OnLoading();
		}
		break;
	case S_FLYING:
		{
			OnFlying();
			UpdatePos(fAccumTime,fFrameTime);
		}
		break;
	}

	OnUpdate();

	if(IsCollCheckByMe())
	{
		CollisionCheck();
	}

	if (/*!BM::GUID::IsNull(TargetGuid()) &&*/ 0 < HomingTime() && 0<m_fFireStartTime && fAccumTime - m_fFireStartTime > HomingTime() )//타겟이 있건없건 호밍 시간이 끝나면 없애야지
	{
		m_kActionTargetList.ApplyActionEffects();
		StartRemove();
	}

	return	true;
}

void	PgProjectile::CollisionCheck()
{
	if(!g_pkWorld) { return; }

	//	이미 타겟을 찾아 브로드캐스트 했다면, 다시 타겟을 찾을 필요가 없다.
	if(false == IsMultipleAttack())
	{
		if(GetTargetListBroadCasted()) { return; }
	}

	if(0 != m_fLastCollideTime /*&& !IsMultipleAttack()*/) // 멀티 타겟이면 사용하지 않음
	{
		if(g_pkWorld->GetAccumTime() - m_fLastCollideTime < GetCollisionCheckSec() )
		{//	이미 이전에 타겟을 한번 찾았다면, 그 다음에는 일정시간 후에 찾는다(연속으로 파파팍 때리는 것을 막기 위함)
			return;
		}
		else
		{
			UpdateCollisionSphereBeforePos();	//	0.5초 후에 다시 타겟을 찾을 때는, 이전 충돌구 위치를 무시한다.
		}
	}

	PgActionTargetList	kTargetList;
	float	fSphereRange = 0.0f;

	PgPilot* pkPilot = g_kPilotMan.FindPilot(GetParentPilotGUID());
	if(!pkPilot) return;

	stCollisionSphere* pkSphere = NULL;
	int	iTotalSphere = m_CollisionSphereCont.size();

	if(0 == iTotalSphere)	//	예외처리, 기본 충돌구를 넣어준다.
	{
		stCollisionSphere	kSphere;
		kSphere.m_fRange = g_fDefaultProjectileCollisionSphereRange;
		kSphere.m_kTargetNode = std::string("Scene Root");
		m_CollisionSphereCont.push_back(kSphere);

		iTotalSphere = m_CollisionSphereCont.size();
	}

	if(0 == iTotalSphere)
	{
		return;
	}

	PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
	if(pkActor)
	{
		if(pkActor->IsUnderMyControl())
		{
			pkPilot = g_kPilotMan.GetPlayerPilot();
		}
	}

	NiPoint3 kTempPos;

	//	충돌구와 충돌하는 타겟이 있는지 찾아본다.
	bool	bFound = false;
	for(int i=0;i<iTotalSphere;++i)
	{
		pkSphere = &(m_CollisionSphereCont[i]);

		fSphereRange = pkSphere->m_fRange * m_fCollisionScale;

		NiAVObject* pkTargetNode = m_spNif->GetObjectByName(pkSphere->m_kTargetNode.c_str());
		if(pkTargetNode == NULL)
		{
			pkTargetNode = m_spNif;
		}

		NiPoint3 const& kTargetNodePos = pkTargetNode->GetWorldTranslate();
		kTempPos = kTargetNodePos;

		//	현재 충돌구 위치에 있는 타겟들을 체크한다.
		if(PgAction::FindTargets
							(
								GetParentActionNo(),
								PgAction::TAT_SPHERE,
								kTargetNodePos,
								NiPoint3::ZERO,
								0,0,
								fSphereRange,
								&kTargetList,1,true,kTargetList,
								pkPilot
								,true
							)>0)
		{
			bFound = true;
			break;
		}

		if(!bFound)	//	이전 충돌구 위치와 현재 충돌구 위치 사이에 있는 녀석들을 체크한다.
		{

			if(pkSphere->m_kBeforePos != NiPoint3::ZERO && 
				pkSphere->m_kBeforePos != kTargetNodePos)
			{
				NiPoint3	kBarStart = pkSphere->m_kBeforePos;
				NiPoint3	kBarDir = kTargetNodePos - pkSphere->m_kBeforePos;
				float	fBarLength = kBarDir.Length();
				kBarDir.Unitize();

				if(PgAction::FindTargets
									(
										GetParentActionNo(),
										PgAction::TAT_BAR,
										kBarStart,
										kBarDir,
										fBarLength,fSphereRange,
										0,
										&kTargetList,1,true,kTargetList,
										pkPilot
										,true
									)>0)
				{
					bFound = true;
					break;
				}		
			}
		}
	}

	UpdateCollisionSphereBeforePos();

	if(bFound)
	{
		//	부딪힌 액터 구하기
		PgActionTargetInfo* pkInfo = &(*kTargetList.begin());
		if(!pkInfo)
		{
			return;
		}
		PgPilot* pkCollidePilot = g_kPilotMan.FindPilot(pkInfo->GetTargetPilotGUID());
		if(!pkCollidePilot)
		{
			return;
		}
		PgActor* pkCollideActor = dynamic_cast<PgActor*>(pkCollidePilot->GetWorldObject());
		if(!pkCollideActor)
		{
			return;
		}
		
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = (CSkillDef*) kSkillDefMgr.GetDef(GetParentActionNo());
		
		if(pkSkillDef &&
			0 == pkSkillDef->GetAbil(AT_PROJECTILE_IGNORE_DOT_TARGET))	// 0이 디폴트
		{// 발사 위치보다 뒤에있는 적이 맞을리 없다면
			NiPoint3	kCollidePos = pkCollideActor->GetHitObjectCenterPos();
			NiPoint3	kDir = kCollidePos - m_vFireStartPos;
			kDir.Unitize();

			NiPoint3	kTargetDir = (m_vTarget - m_vFireStartPos);
			kTargetDir.Unitize();

			if(0 > kDir.Dot(kTargetDir))
			{//	타겟 체크할 필요가 없다
				return;
			}
		}

		m_fLastCollideTime = g_pkWorld->GetAccumTime();

		//	타겟과 부딪혔다.
		//	현재 위치에서, 타겟리스트를 생성해야한다.
		int	iFoundTargets = OnCollision(pkCollideActor);
		
		if(0 == iFoundTargets)
		{//	타겟이 하나도 없을때
			if(m_kOptionInfo.bRemoveNoneTargetProjectile)
			{// 지우게 되어있으면 그냥 터진다
				OnTargetListModified();
			}
		}
		else
		{
			//	내가 발사한 발사체라면, 타겟리스트에 있는 모든 녀석들을 맞추어야한다.
			if(g_kPilotMan.IsMyPlayer(pkPilot->GetGuid())
			|| g_kPilotMan.IsMySummoned(pkPilot->GetUnit()))
			{
				if(pkSkillDef)
				{
					int iMaxTarget = pkSkillDef->GetAbil(AT_MAX_TARGETNUM);

					//관통 어빌을 사용하는 스킬 이면 관통 수를 더 해준다.
					if(0 < pkSkillDef->GetAbil(AT_PENETRATION_USE_TYPE))
					{
						//무조건 넣으면 안되고 스킬에서 사용하는것만 넣어줘야한다.
						int iPenetrationCount = pkPilot->GetAbil(AT_PENETRATION_COUNT);
						if(0 != iPenetrationCount)
						{
							iMaxTarget += iPenetrationCount;
						}
					}

					int iCount = 0;

					int	iTotalTarget = m_kActionTargetList.size();
					PgActionTargetInfo* pkTargetInfo = NULL;
					for(ActionTargetList::iterator itor = m_kActionTargetList.begin();itor != m_kActionTargetList.end();)
					{
						if(iCount < iMaxTarget)
						{
							++itor;
							++iCount;
							continue;
						}

						pkTargetInfo = &(*itor);
						itor = m_kActionTargetList.GetList().erase(itor);	//	최대치보다 많을 경우 제거 한다.
					}

					g_kProjectileMan.BroadCast_PAction(this,&m_kActionTargetList);
				}				
			}
			else
			{
				//	몬스터가 발사한 발사체라면, 나만이 유효한 타겟이다.
				//	타겟리스트에서 내가 아닌 다른 타겟을 제거한다.
				BM::GUID	kMyGUID;
				if(g_kPilotMan.GetPlayerPilotGuid(kMyGUID) == false)
				{
					return;
				}

				int	iTotalTarget = m_kActionTargetList.size();
				PgActionTargetInfo* pkTargetInfo = NULL;
				for(ActionTargetList::iterator itor = m_kActionTargetList.begin();itor != m_kActionTargetList.end();)
				{
					pkTargetInfo = &(*itor);

					if(pkTargetInfo->GetTargetPilotGUID() == kMyGUID
					|| g_kPilotMan.IsMySummoned(pkTargetInfo->GetTargetPilotGUID()) )
					{
						++itor;
						continue;
					}

					itor = m_kActionTargetList.GetList().erase(itor);	//	내가 아니면 제거한다.
				}

				//	타겟리스트에 남은 것이 있다면 브로드캐스트한다.(즉, 나만이 남아있을 것이다)
				if(0 < m_kActionTargetList.size())
				{
					g_kProjectileMan.BroadCast_PAction(this,&m_kActionTargetList);
				}
			}
		}
	}
}
int		PgProjectile::OnCollision(PgActor* pkCollideActor)	//	타겟과 충돌했다.
{
	//	충돌했을 때의 처리 및 타겟리스트 생성을 한다.
	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);
	int iTargetCount = lua_tinker::call<int,lwProjectile,lwActor,lwActionTargetList>((m_kScriptName+"_OnCollision").c_str(),lwProjectile(this),lwActor(pkCollideActor),lwActionTargetList(&m_kActionTargetList));
	NIMETRICS_ENDTIMER(a);
	return iTargetCount;
}
void	PgProjectile::OnUpdate()
{
	if(m_ScriptCallEvent&SCE_UPDATE)
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);
		bool bResult = lua_tinker::call<bool,lwProjectile>((m_kScriptName+"_OnUpdate").c_str(),lwProjectile(this));
		NIMETRICS_ENDTIMER(a);
		if (false == bResult)
		{
			StartRemove();
		}
	}
}

void PgProjectile::OnRemove()
{
	StartRemove();
}

void PgProjectile::Draw(PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime)
{
	if ( GetHide() )
	{
		return;
	}
	pkRenderer->CullingProcess_Deprecated(pkCamera, m_spNif, pkRenderer->GetVisibleArray_Deprecated());
    NiDrawVisibleArrayAppend( *pkRenderer->GetVisibleArray_Deprecated() );
}
void PgProjectile::DrawImmediate(PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime)
{

}

bool PgProjectile::ParseXml(TiXmlNode const* pkNode, void* pArg, bool bUTF8)
{
	int const iType = pkNode->Type();

	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			
			char const* pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "PROJECTILE") == 0)
			{
					TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
					while(pkAttr)
					{
						char const* pcAttrName = pkAttr->Name();
						char const* pcAttrValue = pkAttr->Value();

						if(strcmp(pcAttrName, "ID") == 0)
						{
							SetID(pcAttrValue);
						}
						else if(strcmp(pcAttrName,"AUTO_DIRECTION")==0)
						{
							SetAutoDirection(strcmp(pcAttrValue,"TRUE")==0);
						}
						else if(strcmp(pcAttrName,"MULTIPLE_ATTACK")==0)
						{
							SetMultipleAttack(strcmp(pcAttrValue,"TRUE")==0);
						}
						else if(strcmp(pcAttrName,"NO_SHADOW")==0)
						{
							m_bNoShadow = (strcmp(pcAttrValue,"TRUE")==0);
						}
						else if(strcmp(pcAttrName,"AUTO_GROUND_Z")==0)
						{
							m_kAutoGround = static_cast<float>(atof(pcAttrValue));
						}
						else
						{
							PG_ASSERT_LOG(!"invalid attribute");
						}

						pkAttr = pkAttr->Next();
					}

				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				TiXmlNode const* pkChildNode = pkNode->FirstChild();
				if(0 != pkChildNode)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "TEX_PATH") == 0)
			{
				m_kTextureFolder = pkElement->GetText();
			}
			else if(strcmp(pcTagName, "NIF_PATH") == 0)
			{
				float fScale = 1.0f;
				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "SCALE") == 0)
					{
						fScale = (float)atof(pcAttrValue);
					}
					if(strcmp(pcAttrName, "COLLISION_SCALE") == 0)
					{
						m_fCollisionScale = (float)atof(pcAttrValue);
					}
					else
					{
						PG_ASSERT_LOG(!"invalid attribute");
					}

					pkAttr = pkAttr->Next();
				}

				bool bPathChanged = false;
				if(0 < m_kTextureFolder.length())
				{
					// 이펙트의 텍스쳐 경로를 지정한다.
					bPathChanged = true;
					NiImageConverter::SetPlatformSpecificSubdirectory(m_kTextureFolder.c_str(), TEMPORARY_PLATFORM_SUB_DIR_SLOT);
				}
				m_spNif = (NiNode*)g_kNifMan.GetNif(pkElement->GetText());
				if (bPathChanged)
				{
					NiImageConverter::SetPlatformSpecificSubdirectory(NULL, TEMPORARY_PLATFORM_SUB_DIR_SLOT);

				}

				PG_ASSERT_LOG(m_spNif);
				if (NULL == m_spNif)
				{
					return false;
				}

				m_spNif->SetScale(fScale);		
				InitNifData(m_spNif);
			}
			else if(strcmp(pcTagName,"SCRIPT") == 0)
			{
				m_kScriptName = pkElement->GetText();
			}
			else if(strcmp(pcTagName,"TRAIL") == 0)
			{
				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				TiXmlNode const* pkChildNode = pkNode->FirstChild();
				if(0 != pkChildNode)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName,"TRAIL_TEXTURE") == 0)
			{
				m_kTrailTexturePath = pkElement->GetText();
			}
			else if(strcmp(pcTagName,"TRAIL_TOTAL_TIME") == 0)
			{
				m_iTrailTotalTime = atoi(pkElement->GetText());
			}
			else if(strcmp(pcTagName,"TRAIL_BRIGHT_TIME") == 0)
			{
				m_iTrailBrightTime = atoi(pkElement->GetText());
			}
			else if(strcmp(pcTagName,"TRAIL_EXTEND_LENGTH") == 0)
			{
				m_fTrailExtendLength = (float)atof(pkElement->GetText());
			}
			else if(strcmp(pcTagName,"TRAIL_ATTACH") == 0)
			{
				m_kTrailAttachNodeName = pkElement->GetText();
			}
			else if(strcmp(pcTagName,"TRAIL_WIDTH") == 0)
			{
				m_fTrailWidth = (float)atof(pkElement->GetText());
			}
			else if(strcmp(pcTagName, "PARAM") == 0)
			{
				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "SPEED") == 0)
					{
						m_fSpeed = m_fOriginalSpeed = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "ACCEL") == 0)
					{
						m_fAccelation = m_fOriginalAccelation = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "MASS") == 0)
					{
						m_fMass = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "HOMING_TIME") == 0)
					{
						HomingTime((float)atof(pcAttrValue));
						if(HomingTime()>0)
						{
							SetMovingType(MT_HOMMING);
						}
					}
					else if(strcmp(pcAttrName, "HOMING_DELAY_TIME") == 0)
					{
						HomingDelayTime(static_cast<float>(atof(pcAttrValue)));
						if(HomingDelayTime()>0)
						{
							SetMovingType(MT_STRAIGHTLINE);
						}

					}
					else if(strcmp(pcAttrName, "STAY_TIME") == 0)
					{
						StayTime((float)atof(pcAttrValue));
						if(StayTime()>0)
						{
							SetMovingType(MT_STAY);
						}
					}
					else if(strcmp(pcAttrName, "IGNORE_DEF_RANGE") == 0)
					{
						if(strcmp(pcAttrValue, "TRUE") == 0 || strcmp(pcAttrValue, "true") == 0)
						{
							m_bIgnoreDefinedRange = true;
						}
					}
					else
					{
						PG_ASSERT_LOG(!"invalid attribute");
					}

					pkAttr = pkAttr->Next();
				}				
			}
			else if(strcmp(pcTagName, "SCRIPT_CALL_EVENT") == 0)
			{
				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				TiXmlNode const* pkChildNode = pkNode->FirstChild();
				if(0 != pkChildNode)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "COLLISION_SPHERE") == 0)
			{
				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				TiXmlNode const* pkChildNode = pkNode->FirstChild();
				if(0 != pkChildNode)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "COLL_ITEM") == 0)
			{
				stCollisionSphere	kSphere;

				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "TARGET_NODE") == 0)
					{
						kSphere.m_kTargetNode = std::string(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "RANGE") == 0)
					{
						kSphere.m_fRange = (float)atof(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "COLL_IS_ONLY_TARGETLOC") == 0)
					{						
						m_bCollIsOnyTargetLoc = (strcmp(pcAttrValue,"TRUE")==0);
					}
					else if(strcmp(pcAttrName, "NOT_REMOVE_IF_NONE_TARGET") == 0)
					{// 발사체의 타겟이 없을때 삭제 하지 않음(디폴트는 삭제하게 되어있음)
						m_kOptionInfo.bRemoveNoneTargetProjectile = !(strcmp(pcAttrValue,"TRUE")==0);
					}
					else if(strcmp(pcAttrName, "CHECK_COLLISION_BY_SEC") == 0)
					{// 몇초 마다 충돌 체크를 할 것 인가( 설정 안하면 기본은 0.5초)
						SetCollisionCheckSec((float)atof(pcAttrValue));
					}
					else
					{
						PG_ASSERT_LOG(!"invalid attribute");
					}

					pkAttr = pkAttr->Next();
				}				

				m_CollisionSphereCont.push_back(kSphere);
			}
			else if(strcmp(pcTagName, "EVENT") == 0)
			{
				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					char const* pcAttrName = pkAttr->Name();
					char const* pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrValue, "LOADING_START") == 0)
					{
						m_ScriptCallEvent |= SCE_LOADING_START;
					}
					else if(strcmp(pcAttrValue, "LOADING") == 0)
					{
						m_ScriptCallEvent |= SCE_LOADING;
					}					
					else if(strcmp(pcAttrValue, "FLYING_START") == 0)
					{
						m_ScriptCallEvent |= SCE_FLYING_START;
					}					
					else if(strcmp(pcAttrValue, "FLYING") == 0)
					{
						m_ScriptCallEvent |= SCE_FLYING;
					}		
					else if(strcmp(pcAttrValue, "ARIVED_AT_TARGET") == 0)
					{
						m_ScriptCallEvent |= SCE_ARRIVED_AT_TARGET;
					}		
					else if(strcmp(pcAttrValue, "ANIMATION") == 0)
					{
						m_ScriptCallEvent |= SCE_ANIMATION;
					}		
					else if(strcmp(pcAttrValue, "LOST_TARGET") == 0)
					{
						m_ScriptCallEvent |= SCE_LOST_TARGET;
					}		
					else if(strcmp(pcAttrValue, "UPDATE") == 0)
					{
						m_ScriptCallEvent |= SCE_UPDATE;
					}	
					else if(strcmp(pcAttrValue, "COLLISION") == 0)
					{
						m_ScriptCallEvent |= SCE_COLLISION;
					}	
					else
					{
						PG_ASSERT_LOG(!"invalid attribute");
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
	};

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	TiXmlNode const* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}
	return	true;
}
void PgProjectile::Terminate()
{
	if(m_spNif && m_spNif->GetParent())
	{
//		try
//		{
			m_spNif->GetParent()->DetachChild(m_spNif);
//		}
//		catch(char *str)
//		{
//			//_PgOutputDebugString("Exception Raised : PgProjectile::Terminate() [%s]\n",str);
//		}
	}
	m_spNif = 0;

	if(g_spTrailNodeMan && m_pkTrail)
	{
		g_spTrailNodeMan->StopTrail(m_pkTrail,false);
	}
	m_pkTrail= NULL;

	m_kActionTargetList.ApplyActionEffects(true,true);
}
void	PgProjectile::SetParam(float fSpeed,float fAccel,float fMass)
{
	m_fSpeed = fSpeed;
	m_fAccelation = fAccel;
	m_fMass	= fMass;
}

void	PgProjectile::SetScale(float fScale)
{
	PG_ASSERT_LOG(fScale > 0.0f);
	m_spNif->SetScale(fScale);
}
float	PgProjectile::GetScale()
{
	return	m_spNif->GetScale();
}

NiPoint3 const& PgProjectile::GetNodeWorldPos(char const* strNode)const
{
	if( m_spNif )
	{
		NiAVObject* pkNode = m_spNif->GetObjectByName(strNode);
		if(pkNode)
		{
			return pkNode->GetWorldTranslate();
		}
	}
	return NiPoint3(0,0,0);
}

void	PgProjectile::SetTargetLoc(NiPoint3 const& vTargetLoc)
{
	m_TargetType = TT_LOCATION;

	m_vTarget = vTargetLoc;

	//_PgOutputDebugString("PgProjectile::SetTargetLoc m_vTarget:%f,%f,%f\n", m_vTarget.x,m_vTarget.y,m_vTarget.z);

	if(m_spNif->GetParent())
	{
		m_spNif->GetParent()->DetachChild(m_spNif);

		NiNodePtr	spArrowHead = (NiNode*)m_spNif->GetObjectByName("arrow_head");
		NiNodePtr	spArrowTail = (NiNode*)m_spNif->GetObjectByName("arrow_end");
		if(0 != spArrowHead && 0 != spArrowTail)
		{
			const NiTransform	&kTransform = m_spNif->GetWorldTransform();
			m_spNif->SetLocalTransform(kTransform);
		}
		else
		{
			m_spNif->SetTranslate(m_spNif->GetWorldTranslate());
		} 
	}
}
void	PgProjectile::SetVelocity(NiPoint3 const& kVel)
{
	m_vVelocity = kVel;
}
void	PgProjectile::Fire(bool bNoResetTargetLoc)
{
	if(!g_pkWorld)
	{
		return;
	}

	m_bNoResetTargetLoc = bNoResetTargetLoc;

	m_State = S_FLYING;

	//m_fLastCollideTime = 0.0f;

	m_vFireStartPos = m_spNif->GetTranslate();
	m_fFireStartTime = g_pkWorld->GetAccumTime();


	//_PgOutputDebugString("PgProjectile::Fire() m_vFireStartPos:%f,%f,%f m_fFireStartTime:%f\n", m_vFireStartPos.x,m_vFireStartPos.y,m_vFireStartPos.z,m_fFireStartTime);

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(GetParentActionNo());

	float	fSpeedRateUnit=1.0f,fSpeedRateMissionLevel=1.0f;

	PgPilot*	pkPilot = g_kPilotMan.FindPilot(m_kParentGUID);
	CUnit*		pkUnit = NULL;
	
	if(pkPilot)
	{
		pkUnit = pkPilot->GetUnit();
	}

	//	발사체 이동 속도 비율 어빌 적용
	if(pkSkillDef)
	{
		float fAddSpeedRateProjectile = 0.0f;

		if(pkPilot)
		{
			fSpeedRateUnit = (float)pkPilot->GetAbil(AT_PROJECTILE_SPEED_RATE)/ABILITY_RATE_VALUE_FLOAT;
			if(0 == fSpeedRateUnit)
			{
				fSpeedRateUnit = 1.0f;
			}

			if(pkUnit && pkUnit->IsUnitType(UT_MONSTER))
			{
				//	몬스터일 경우에는 미션 레벨에 따라 속도 조절을 해줘야한다.
				int	iLevel = g_pkWorld->GetDifficultyLevel();
				fSpeedRateMissionLevel = lua_tinker::call<float,int>("GetProjectileSpeedByMissionLevel",iLevel);
			}		
		}

	}

	SetSpeed(m_fSpeed * fSpeedRateUnit * fSpeedRateMissionLevel);

	//	클라이언트가 체크하는 발사체라면
	if(pkSkillDef && pkSkillDef->IsSkillAtt(SAT_CLIENT_CTRL_PROJECTILE))
	{
		//	발사한 주체가 몬스터 또는 나라면, 이 발사체의 충돌체크는 내가 한다.
		if(NULL == pkPilot)
		{
			StartRemove();
			return;
		}

		if(pkPilot->GetUnit())
		{
			CUnit*	pkUnit = pkPilot->GetUnit();
			bool	bIsUnderMyControl = false;
			PgActor* pkActor = dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
			if(pkActor)
			{
				bIsUnderMyControl = (pkActor->IsUnderMyControl());
			}
			
			m_bCollCheckByMe = pkUnit->IsUnitType(UT_MONSTER) || (bIsUnderMyControl) || g_kPilotMan.IsMySummoned(pkUnit);
			m_bCollCheckByMe |= (pkUnit->GetAbil(AT_CALLER_TYPE)&UT_MONSTER) && pkUnit->IsUnitType(UT_ENTITY);
		}


		bool	bMakeHorizontalAngle = (0 < m_kActionTargetList.size());

		//	타겟 리스트는 클리어한다.
		if(ClearTargetList())
		{
			m_kActionTargetList.clear();
		}

		if(false == m_bNoResetTargetLoc)
		{
			NiPoint3	kTargetPos = m_vTarget;
			if(bMakeHorizontalAngle)
			{
				kTargetPos.z = m_vFireStartPos.z;
			}

			//	타겟좌표를 바꾸자.
			NiPoint3	kTargetDir = (kTargetPos - m_vFireStartPos);
			kTargetDir.Unitize();

			float	fDistance = static_cast<float>(lwAction::GetSkillRangeEx(pkSkillDef,lwActor(dynamic_cast<PgActor*>(pkPilot->GetWorldObject()))));

			kTargetPos = m_vFireStartPos+kTargetDir*fDistance*1.1f;
			
			SetTargetLoc(kTargetPos);
		}
	}

	InitTrailNode();

	OnFlyingStart();
}
void	PgProjectile::DelayFire(float fDelayTime, bool bNoResetTargetLoc)	//	일정 시간 기다린 후에 자동으로 발사된다.
{
	if(!g_pkWorld)
	{
		return;
	}

	m_bNoResetTargetLoc = bNoResetTargetLoc;

	m_fDelayFireStartTime = g_pkWorld->GetAccumTime();
	m_fDelayFireTotalTime = fDelayTime;
	m_State = S_DELAY_FIRING;

	NiPoint3	kStartPos = m_spNif->GetWorldTranslate();
	LookAtTarget(kStartPos,m_vTarget,m_fDelayFireStartTime);
	m_spNif->Update(m_fDelayFireStartTime);
}
void	PgProjectile::ModifyTargetList(PgActionTargetList &kTargetList)
{
	PgActionTargetInfo* pkTargetInfo=NULL,*pkTargetInfo2=NULL;
	for(ActionTargetList::iterator itor = kTargetList.begin();itor != kTargetList.end(); )
	{

		pkTargetInfo = &(*itor);
		pkTargetInfo2 = m_kActionTargetList.GetTargetByGUID(pkTargetInfo->GetTargetPilotGUID());

		if(!pkTargetInfo2)
		{
			++itor;
			continue;
		}

		pkTargetInfo2->CopyFrom(*pkTargetInfo);

		if(false == m_kActionTargetList.IsActionEffectApplied())
		{
			itor = kTargetList.GetList().erase(itor);
		}
		else
		{
			++itor;
		}
	}

	m_kActionTargetList.SetTimeStamp(kTargetList.GetTimeStamp());
	m_kActionTargetList.SetEffectReal(true);
}
void	PgProjectile::SetTargetObjectList(PgActionTargetList &kTargetInfoList,bool bNoSetTargetLoc)	//	리스트의 첫번째에 있는 놈이 목표위치가 된다.
{
	if(0 == kTargetInfoList.size()) { return; }

	m_TargetType = TT_OBJECT;

	m_kActionTargetList.ApplyActionEffects(true,true);

	m_kActionTargetList = kTargetInfoList;

	g_kActionTargetTransferInfoMan.AddTransferInfo(
		GetParentPilotGUID(),
		GetParentActionNo(),
		GetParentActionInstanceID(),
		GetUID());
	
	PgActionTargetInfo* pkTargetInfo = &(*m_kActionTargetList.begin());

	PgPilot* pkPilot = g_kPilotMan.FindPilot(pkTargetInfo->GetTargetPilotGUID());
	if(!pkPilot) { return; }

	PgActor* pkActor = dynamic_cast<PgActor* >(pkPilot->GetWorldObject());
	if(!pkActor) { return; }

	if(false == bNoSetTargetLoc)
	{
		SetTargetLoc(pkActor->GetABVShapeWorldPos(pkTargetInfo->GetSphereIndex()));
	}

	if(m_spNif->GetParent())
	{
		m_spNif->GetParent()->DetachChild(m_spNif);

		NiNodePtr	spArrowHead = NiDynamicCast(NiNode,m_spNif->GetObjectByName("arrow_head"));
		NiNodePtr	spArrowTail = NiDynamicCast(NiNode,m_spNif->GetObjectByName("arrow_end"));
		if(0 != spArrowHead && 0 != spArrowTail)
		{
			NiTransform const& kTransform = m_spNif->GetWorldTransform();
			m_spNif->SetLocalTransform(kTransform);
		}
		else
		{
			m_spNif->SetTranslate(m_spNif->GetWorldTranslate());
		}
	}
}

void	PgProjectile::SetTargetObject(PgActionTargetInfo& kTargetInfo)
{
	PgActionTargetList	kNewList;

	kNewList.SetActionInfo(m_kParentGUID,m_iParentActionInstanceID,m_iParentActionNo,m_dwParentActionTimeStamp);

	kNewList.GetList().push_back(kTargetInfo);
	SetTargetObjectList(kNewList);
}

void	PgProjectile::LoadToPosition(NiPoint3 const& rkPos)
{
	if(!g_pkWorld)
	{
		return;
	}

	m_State = S_LOADING;
	m_TargetType  = TT_UNDEFINED;

	OnLoadingStart();

	g_pkWorld->RunObjectGroupFunc(OGT_EFFECT, WorldObjectGroupsUtil::AttachChild(m_spNif, true));
	m_spNif->SetTranslate(rkPos);
	m_spNif->Update(0);

	NiTimeController::StartAnimations(m_spNif, 0.0f);
}

void	PgProjectile::LoadToHelper(const PgActor* pkActor, char const* strHelperName)
{
	m_State = S_LOADING;
	m_TargetType  = TT_UNDEFINED;

	OnLoadingStart();

	if(strHelperName)
	{
		FireNode(strHelperName);
	}

	bool	bLoaded = false;
	NiNode*	pkHelper = (NiNode*)pkActor->GetNIFRoot()->GetObjectByName(strHelperName);
	if(pkHelper)
	{
		pkHelper->AttachChild(m_spNif, true);
		pkHelper->Update(0);
		bLoaded = true;
	}
	if(!bLoaded)
	{
		NxVec3 vGlobalPos = pkActor->GetPhysXActor()->getGlobalPosition();
		m_spNif->SetTranslate(NiPoint3(vGlobalPos.x,vGlobalPos.y,vGlobalPos.z));
		m_spNif->Update(0);
	}
	NiTimeController::StartAnimations(m_spNif, 0.0f);
}

void	PgProjectile::LoadToWeapon(const PgActor* pkActor)
{
	m_State = S_LOADING;
	m_TargetType  = TT_UNDEFINED;

	OnLoadingStart();

	bool	bLoaded = false;
	PgActor::PartsAttachInfo kPartsAttachInfo = pkActor->GetPartsAttachInfo();
	PgActor::PartsAttachInfo::iterator itr = kPartsAttachInfo.find(EQUIP_LIMIT_WEAPON);	//	5번이 무기
	if(itr != kPartsAttachInfo.end())
	{
		NiNode* pkSrcNode = itr->second->GetFirstAttachedObject();
		if(!pkSrcNode)
		{
			return;
		}
		NiNode* pkDummy = NiDynamicCast(NiNode,pkSrcNode->GetObjectByName(ATTACH_POINT_01));
		if(pkDummy)
		{
			pkDummy->AttachChild(m_spNif, true);
			pkDummy->Update(0);
			bLoaded = true;
		}
	}
	if(!bLoaded)
	{
		NxVec3 vGlobalPos = pkActor->GetPhysXActor()->getGlobalPosition();
		m_spNif->SetTranslate(NiPoint3(vGlobalPos.x,vGlobalPos.y,vGlobalPos.z));
		m_spNif->Update(0);
	}
	NiTimeController::StartAnimations(m_spNif, 0.0f);
}
float	PgProjectile::GetCurrentSpeed()
{
	return	m_fSpeed;
}
NiPoint3	PgProjectile::GetWorldPos()
{
	return	m_spNif->GetWorldTranslate();
}
void	PgProjectile::SetWorldPos(NiPoint3 const& kPos)
{
	m_spNif->SetTranslate(kPos);
	m_spNif->Update(0);
}
PgActionTargetInfo const* PgProjectile::GetActionTargetInfo(int const iIndex)
{
	if(m_kActionTargetList.size() <= iIndex) { return	NULL; }

	int iCount = 0;
	for(ActionTargetList::iterator itor = m_kActionTargetList.begin(); itor != m_kActionTargetList.end(); ++itor)
	{
		if(iCount == iIndex)
		{
			return	&(*itor);
		}

		++iCount;
	}
	return	NULL;
}
void	PgProjectile::StartRemove()
{
	m_State = S_REMOVING;
	
	//	파티클 노드가 있는지 찾아보자.
	if(!HasParticleNode(m_spNif))
	{
		//	없으면 바로 지운다.
		m_bDestroyMe = true;
		return;
	}

	//	있으면 파티클이 전부 없어질때까지 기다려야한다.
	//	이제부터 파티클 생성을 없앤다.
    NewWare::Scene::ApplyTraversal::Geometry::RunParticleGeneration( m_spNif, true );

	//	파티클이 아닌 오브젝트는 HIDE 시켜야한다.
	HideNoneParticleObject(m_spNif);

}
bool	PgProjectile::HasParticleNode(NiAVObject* pkObject)
{
	if(NiIsKindOf(NiNode, pkObject))
	{
		NiNode* pkNode = NiDynamicCast(NiNode, pkObject);
		int	iChildArrayCount = pkNode->GetArrayCount();
		NiAVObject* pkChild = NULL;
		for(int i=0;i<iChildArrayCount;++i)
		{
			pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				if(HasParticleNode(pkChild))
				{
					return	true;
				}
			}
		}
	}
	else if(NiIsKindOf(NiParticleSystem, pkObject))
	{
		return	true;
	}
	return	false;
}
bool	PgProjectile::CheckAllParticleRemoved(NiAVObject *pkObject)
{
	if(NiIsKindOf(NiNode, pkObject))
	{
		NiNode* pkNode = NiDynamicCast(NiNode, pkObject);
		int	iChildArrayCount = pkNode->GetArrayCount();
		NiAVObject* pkChild = NULL;
		for(int i=0;i<iChildArrayCount;++i)
		{
			pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				if(!CheckAllParticleRemoved(pkChild))
				{
					return	false;
				}
			}
		}
	}
	else if(NiIsKindOf(NiParticleSystem, pkObject))
	{
		NiParticleSystem* pkParticleSystem = NiDynamicCast(NiParticleSystem, pkObject);
		if(0 == pkParticleSystem->GetNumParticles())
		{
			return	true;
		}
		return	false;
	}
	return	true;
}

void	PgProjectile::HideNoneParticleObject(NiAVObject *pkObject)
{
	if(NiIsKindOf(NiNode, pkObject))
	{
		NiNode* pkNode = NiDynamicCast(NiNode, pkObject);
		int	iChildArrayCount = pkNode->GetArrayCount();
		NiAVObject* pkChild = NULL;
		for(int i=0;i<iChildArrayCount;++i)
		{
			pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				HideNoneParticleObject(pkChild);
			}
		}
	}
	else if(NiIsKindOf(NiGeometry, pkObject))
	{
		if(false == NiIsKindOf(NiParticleSystem, pkObject))
		{
			pkObject->SetAppCulled(true);
		}
	}
}

//	스크립트 호출
void	PgProjectile::OnLoadingStart()	//	장전시작
{
	if(m_ScriptCallEvent&SCE_LOADING_START)
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);
		bool bResult = lua_tinker::call<bool,lwProjectile>((m_kScriptName+"_OnLoadingStart").c_str(),lwProjectile(this));
		NIMETRICS_ENDTIMER(a);
		if (false == bResult)
		{
			StartRemove();
		}
	}
}
void	PgProjectile::OnLoading()	//	장전중
{
	if(m_ScriptCallEvent&SCE_LOADING)
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);
		bool bResult = lua_tinker::call<bool,lwProjectile>((m_kScriptName+"_OnLoading").c_str(),lwProjectile(this));
		NIMETRICS_ENDTIMER(a);
		if (false == bResult)
		{
			StartRemove();
		}
	}
}
void	PgProjectile::OnFlyingStart()	//	발사 시작
{
	if(m_ScriptCallEvent&SCE_FLYING_START)
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);
		bool bResult = lua_tinker::call<bool,lwProjectile>((m_kScriptName+"_OnFlyingStart").c_str(),lwProjectile(this));
		NIMETRICS_ENDTIMER(a);
		if (false == bResult)
		{
			StartRemove();
		}
	}
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const * pkDef = kSkillDefMgr.GetDef(m_iParentActionNo);
	
	if(0< pkDef->GetAbil(AT_POT_PARTICLE_ACTION))
	{
		PgActor* pkActor = g_kPilotMan.FindActor(m_kParentGUID);
		if(pkActor)
		{
			PgPOTParticleUtile::AttachParticle(pkActor->GetPOTParticleInfo(), PgPOTParticle::E_APOT_PROJELCTILE_ATK, m_spNif);
		}
	}
}
void	PgProjectile::OnFlying()	//	날아가는 중
{
	if(m_ScriptCallEvent&SCE_FLYING)
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);
		bool bResult = lua_tinker::call<bool,lwProjectile>((m_kScriptName+"_OnFlying").c_str(),lwProjectile(this));
		NIMETRICS_ENDTIMER(a);
		if (false == bResult)
		{
			StartRemove();
		}
	}
}
void	PgProjectile::OnArrivedAtTarget()	//	목표도달 완료
{
	m_State = S_ARRIVED_AT_TARGET;

	if(m_ScriptCallEvent&SCE_ARRIVED_AT_TARGET)
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);                                                                                                                        
		bool bResult = lua_tinker::call<bool,lwProjectile>((m_kScriptName+"_OnArrivedAtTarget").c_str(),lwProjectile(this));
		NIMETRICS_ENDTIMER(a);
		if (false == bResult)
		{
			StartRemove();
		}
	}
}
void	PgProjectile::OnTargetListModified()	//	서버로부터 타겟을 새로 받았다.
{
	NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
	NIMETRICS_STARTTIMER(a);
	bool bResult = lua_tinker::call<bool,lwProjectile>((m_kScriptName+"_OnTargetListModified").c_str(),lwProjectile(this));
	NIMETRICS_ENDTIMER(a);
	if (false == bResult)
	{
		StartRemove();
	}

}
void	PgProjectile::OnAnimation(char const* pkEventName)
{
	if(m_ScriptCallEvent&SCE_ANIMATION)
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);
		bool bResult = lua_tinker::call<bool,char const*,lwProjectile>((m_kScriptName+"_OnAnimationEvent").c_str(),pkEventName,lwProjectile(this));
		NIMETRICS_ENDTIMER(a);
		if (false == bResult)
		{
			StartRemove();
		}
	}
}
void	PgProjectile::OnLostTarget()	//	타겟이 사라졌다
{
	if(m_ScriptCallEvent&SCE_LOST_TARGET)
	{
		NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
		NIMETRICS_STARTTIMER(a);
		bool bResult = lua_tinker::call<bool,lwProjectile>((m_kScriptName+"_OnLostTarget").c_str(),lwProjectile(this));
		NIMETRICS_ENDTIMER(a);
		if (false == bResult)
		{
			StartRemove();
		}
	}
}

void	PgProjectile::SetRotate(const NiQuaternion& rkQuat)
{
	m_spNif->SetRotate(rkQuat);
}

NiPoint3 PgProjectile::GetDirection()
{
	NiPoint3 kPos = m_spNif->GetWorldTranslate();

	NiPoint3 kDir = kPos - m_vTarget;
	kDir.Unitize();

	return kDir;
}

NiPoint3 PgProjectile::GetUp()
{	
	
	NiPoint3 kUp = GetDirection().UnitCross(GetRight());
	kUp.Unitize();

	return kUp;

}

NiPoint3 PgProjectile::GetRight()
{
	NiPoint3	kWorldUp(0,0,1);
	NiPoint3 kRight = kWorldUp.UnitCross(GetDirection());
	kRight.Unitize();

	return kRight;
}

NiPoint3 PgProjectile::ProcessorAutoGround(NiNodePtr const NodePtr, NiPoint3& PastPos, float const AccumTime)
{
	NiPoint3 kGPos = NodePtr->GetWorldTranslate();
	if(0.0f==m_kAutoGround || NULL==g_pkWorld)
	{
		return kGPos;
	}

	if(g_pkWorld->GetPhysXScene() && g_pkWorld->GetPhysXScene()->GetPhysXScene())
	{
		NxRaycastHit kHit;
		NxRay kRay(NxVec3(kGPos.x, kGPos.y, kGPos.z+40.0f), NxVec3(0.0f, 0.0f, -1.0f));
		NxScene	*pkScene = g_pkWorld->GetPhysXScene()->GetPhysXScene();

		NxU32	iGroup = DEFAULT_ACTIVE_GRP ;
		NxShape *pkHitShape = g_pkWorld->raycastClosestShape(
			kRay, 
			NX_ALL_SHAPES, 
			kHit, iGroup, 200, 
			NX_RAYCAST_SHAPE|NX_RAYCAST_FACE_NORMAL|NX_RAYCAST_DISTANCE |  NX_RAYCAST_IMPACT);

		if(pkHitShape)
		{	
			const	NiTransform	&kTransform = NodePtr->GetLocalTransform();
			NodePtr->SetTranslate(kTransform.m_Translate.x,kTransform.m_Translate.y,kHit.worldImpact.z+2);
			
			kGPos.z = kHit.worldImpact.z+2;
			
			LookAtTarget(PastPos, kGPos, AccumTime);//회전은 여기서
			m_spNif->Update(AccumTime);
		}
	}

	return kGPos;
}
