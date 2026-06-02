#include "stdafx.h"
#include "PgParticleProcessorAutoGround.H"
#include "PgParticle.H"
#include "PgActor.H"

NiImplementRTTI(PgParticleProcessorAutoGround, PgParticleProcessor);

bool	PgParticleProcessorAutoGround::DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime)
{
	if(!g_pkWorld)
	{
		return	false;
	}

	const	NiTransform	&kTransform = pkParticle->GetLocalTransform();

	NxRay kRay(NxVec3(kTransform.m_Translate.x,kTransform.m_Translate.y,kTransform.m_Translate.z+40), NxVec3(0.0f, 0.0f, -1.0f));
	NxRaycastHit kHit;
	if(g_pkWorld->GetPhysXScene() && g_pkWorld->GetPhysXScene()->GetPhysXScene())
	{
		NxScene	*pkScene = g_pkWorld->GetPhysXScene()->GetPhysXScene();

		NxU32	iGroup = DEFAULT_ACTIVE_GRP ;
		NxShape *pkHitShape = g_pkWorld->raycastClosestShape(
			kRay, 
			NX_ALL_SHAPES, 
			kHit, iGroup, 200, 
			NX_RAYCAST_SHAPE|NX_RAYCAST_FACE_NORMAL|NX_RAYCAST_DISTANCE |  NX_RAYCAST_IMPACT);

		if(pkHitShape)
		{
			NxVec3	vUp(0,0,1);
			NxVec3	vNormal = kHit.worldNormal;
			float	fTheta = acos(vUp.dot(vNormal));
			NxVec3	vRotationAxis = vNormal.cross(vUp);
			vRotationAxis.normalize();

			NiMatrix3	kmatRot;
			kmatRot.MakeRotation(fTheta,NiPoint3(vRotationAxis.x,vRotationAxis.y,vRotationAxis.z));
			kmatRot = pkParticle->GetOriginalRotate() * kmatRot;
			pkParticle->SetTranslate(kTransform.m_Translate.x,kTransform.m_Translate.y,kHit.worldImpact.z+2);
			pkParticle->SetRotate(kmatRot);

			return	true;
		}
	}

	return	false;
}

NiImplementCreateClone(PgParticleProcessorAutoGround);

void PgParticleProcessorAutoGround::CopyMembers(PgParticleProcessorAutoGround* pDest, NiCloningProcess& kCloning)
{
	PgParticleProcessor::CopyMembers(pDest,kCloning);
}
