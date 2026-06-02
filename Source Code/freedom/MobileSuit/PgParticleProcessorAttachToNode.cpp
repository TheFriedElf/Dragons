#include "stdafx.h"
#include "PgParticleProcessorAttachToNode.H"
#include "PgParticle.H"

NiImplementRTTI(PgParticleProcessorAttachToNode, PgParticleProcessor);

bool	PgParticleProcessorAttachToNode::DoProcess(PgParticle *pkParticle,float fAccumTime,float fFrameTime)
{
	if(!pkParticle || !m_spAttachNode)
	{
		return	false;
	}

	NiTransform	kTransform = m_spAttachNode->GetWorldTransform();

	if(pkParticle->GetNoFollowParentRotation())
	{
		kTransform.m_Rotate = NiMatrix3::IDENTITY;
	}
	
	kTransform.m_fScale *= pkParticle->GetOriginalScale();
	pkParticle->SetLocalTransform(kTransform);

	if(pkParticle->IsLoop())
	{
		// 왜 루프일때만?
		//	Loop 이면서, Attach 라는 것은, 즉 Attach  되는 타겟 노드에 긴 시간동안 붙어있게 된다는 것, 따라서 이런 이펙트에 대해서는 Attach 되는 타겟의 색상을 적용시켜준다(상태이상 등에 의해 색이 변할때..)
		NiColorA	kColor = m_spAttachNode->GetColor();
		pkParticle->SetColorLocal(pkParticle->GetColorLocal() * kColor);
	}

	return	true;

}
NiImplementCreateClone(PgParticleProcessorAttachToNode);
void PgParticleProcessorAttachToNode::CopyMembers(PgParticleProcessorAttachToNode* pDest, NiCloningProcess& kCloning)
{
	PgParticleProcessor::CopyMembers(pDest,kCloning);

	pDest->m_spAttachNode = m_spAttachNode;
}
