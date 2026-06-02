#include "stdafx.h"
#include "PgCircleShadow.h"
#include "PgNifMan.h"
#include "PgRenderer.h"
#include "PgMobileSuit.h"
#include "PgRenderMan.h"
#include "PgActor.h"
#include "PgParticleMan.h"
#include "PgWorld.H"
#include "NiGeometry.H"
#include "NiGeometryData.H"

#include "NewWare/Renderer/DrawUtils.h"
#include "NewWare/Scene/ApplyTraversal.h"


NiImplementRTTI(PgCircleShadow, NiGeometry);

const	char*	g_strCircleShadowGeomPath = "../Data/5_Effect/0_Common/CircleShadowGeom.nif";
const	char*	g_strCircleShadowTexturePath = "../Data/5_effect/9_Tex/CircleShadow.dds";
const	char*	g_strLadderShadowTexturePath = "../Data/5_effect/9_Tex/LadderShadow.dds";

const	char*	g_strCircleShadowNodeName="circle_shadow";
const	char*	g_strLadderShadowNodeName="ladder_shadow";

PgCircleShadow::PgCircleShadow():
NiGeometry(NiNew NiTriShapeData(0,0,0,0,0,0,NiGeometryData::NBT_METHOD_NONE,0,0)),
m_pkBottomRayHit(0)
{	
	Init();	
}

void	PgCircleShadow::Init()
{
	m_spShadowGeom = 0;
	m_fScale = 0.0f;
	SetPositionSourceObject(NULL);

	SetMaxShadowDistance();
	SetShadowScale();

	m_matRot.MakeIdentity();
	LoadNif();

	NiAlphaPropertyPtr	spAlpha = NiNew NiAlphaProperty();
	spAlpha->SetAlphaBlending(true);
	spAlpha->SetAlphaGroup(AG_SHADOW);
	AttachProperty(spAlpha);
}
bool	PgCircleShadow::UpdateWorldBoundEx()
{
	if(!g_bDrawCircleShadow)
	{
		return	true;
	}

	NiAVObject*	pkParent = GetParent();
	if(m_pSrcObject)
	{
		pkParent = m_pSrcObject;
	}
	if(g_pkWorld && pkParent && m_spShadowGeom)
	{

		const	NiPoint3	&kParentPos = pkParent->GetWorldTranslate();
		const	NiTransform	&kTransform = GetLocalTransform();

		NxRay kRay(NxVec3(kParentPos.x,kParentPos.y,kParentPos.z+40), NxVec3(0.0f, 0.0f, -1.0f));
		NxRaycastHit kHit;

		NiPhysXScene *pkPhysXScene = g_pkWorld->GetPhysXScene();
		if(!pkPhysXScene)
		{
			return false;
		}
		NxScene	*pkScene = pkPhysXScene->GetPhysXScene();
		if(!pkScene)
		{
			return false;
		}

		NxU32	iGroup = DEFAULT_ACTIVE_GRP ;
		NxShape *pkHitShape = 0;

		if(m_pkBottomRayHit)
		{
			pkHitShape = m_pkBottomRayHit->shape;
			kHit = *m_pkBottomRayHit;
		}
		else
		{
			pkHitShape = g_pkWorld->raycastClosestShape(
						kRay, NX_ALL_SHAPES, kHit, iGroup, m_fMaxShadowDistance, 
						NX_RAYCAST_SHAPE|NX_RAYCAST_FACE_NORMAL|NX_RAYCAST_DISTANCE);
		}

		if(pkHitShape)
		{
			NxVec3	vUp(0,0,1);
			NxVec3	vNormal = kHit.worldNormal;
			float	fTheta = acos(vUp.dot(vNormal));
			float	fAdditionalScale = NiMin(1.0f, fabs(kHit.distance / m_fMaxShadowDistance));
			NxVec3	vRotationAxis = vNormal.cross(vUp);
			vRotationAxis.normalize();

			m_fScale = m_fShadowScale + ( fAdditionalScale * m_fShadowScale );

			m_matRot.MakeRotation(fTheta,NiPoint3(vRotationAxis.x,vRotationAxis.y,vRotationAxis.z));

			m_spShadowGeom->SetScale(m_fScale);
			m_spShadowGeom->SetTranslate(kHit.worldImpact.x,kHit.worldImpact.y,kHit.worldImpact.z+2);
			m_spShadowGeom->SetRotate(m_matRot);

			m_spShadowGeom->Update(0,false);

			m_kWorldBound = m_spShadowGeom->GetWorldBound();

		}
	}

	return	true;
}

void PgCircleShadow::UpdateWorldBound()
{
	UpdateWorldBoundEx();
}

void	PgCircleShadow::SetPositionSourceObject(NiAVObject *pkSourceObject)
{
	m_pSrcObject = pkSourceObject;
}

void	PgCircleShadow::Destroy()
{
	m_spShadowGeom = 0;
}

void PgCircleShadow::RenderImmediate(NiRenderer* pkRenderer)
{
	if(g_bDrawCircleShadow == false)
	{
		return;
	}

	if(m_spShadowGeom && !g_kRenderMan.GetRenderer())
	{
		return;
	}
    NewWare::Renderer::DrawUtils::DrawImmediate( g_kRenderMan.GetRenderer()->GetRenderer(), m_spShadowGeom );
}

void	PgCircleShadow::AddOnlyShadowToVisibleArrayRecursive(
	NiAVObject *pkTargetNode,
	PgRenderer *pkRenderer,
	NiCamera *pkCamera)
{
	
	NiNode*	pkNode = NiDynamicCast(NiNode,pkTargetNode);
	if(pkNode)
	{
		const	int	iTotalChild = pkNode->GetArrayCount();
		if(iTotalChild>0)
		{
			for(int i=0;i<iTotalChild;i++)
			{
				NiAVObject*	pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					AddOnlyShadowToVisibleArrayRecursive(pkChild,pkRenderer,pkCamera);
				}
			}
		}
	}

	PgCircleShadow	*pkShadow = NiDynamicCast(PgCircleShadow,pkTargetNode);
	if(pkShadow && pkShadow->GetAppCulled() == false)
	{
		pkRenderer->CullingProcess_Deprecated(pkCamera, pkTargetNode, pkRenderer->GetVisibleArray_Deprecated());
        NiDrawVisibleArrayAppend( *pkRenderer->GetVisibleArray_Deprecated() );
	}

}
void	PgCircleShadow::DetachCircleShadowRecursive(NiAVObject *pkObject)
{

	NiNode*	pkNode = NiDynamicCast(NiNode,pkObject);
	if(pkNode)
	{
		int	const iTotalChild = pkNode->GetArrayCount();
		for(int i=0;i<iTotalChild;i++)
		{
			NiAVObject*	pkChild = pkNode->GetAt(i);
			PgCircleShadow	*pkShadowNode = NiDynamicCast(PgCircleShadow,pkChild);
			if(pkShadowNode)
			{
				pkNode->DetachChild(pkShadowNode);
				continue;
			}

			if(pkChild)
			{
				DetachCircleShadowRecursive(pkChild);
			}
		}
	}

}
int	PgCircleShadow::AttachCircleShadowRecursive(NiAVObject* pkTargetNode,
	float const fMaxShadowDistance,
	float const fShadowScale,
	NiAVObject const *pkShadowNif,
	NiAVObject	*pSrcObject,
	NxRaycastHit *pkBottomRayHit)
{
	int	iShadowCount = 0;

	NiNode*	pkNode = NiDynamicCast(NiNode,pkTargetNode);
	if(pkNode)
	{
		if(pSrcObject)
		{
			AttachNewShadowTo(pkNode,fMaxShadowDistance,
				fShadowScale,pkShadowNif,pSrcObject,pkBottomRayHit);

			++iShadowCount;

			return iShadowCount;
		}
		int	const iTotalChild = pkNode->GetArrayCount();
		if(iTotalChild>0)
		{
			for(int i=0;i<iTotalChild;i++)
			{
				NiAVObject*	pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					iShadowCount+=AttachCircleShadowRecursive(pkChild,
						fMaxShadowDistance,
						fShadowScale,
						pkShadowNif,
						pSrcObject,
						pkBottomRayHit);
				}
			}
		}
		if(pkNode->GetName() == g_strCircleShadowNodeName || 
			pkNode->GetName() == g_strLadderShadowNodeName)
		{
			const	float	fParentScale= pkNode->GetScale();

			PgCircleShadow *pkShadow = AttachNewShadowTo(pkNode,fMaxShadowDistance,
				fShadowScale*fParentScale,pkShadowNif,pSrcObject,pkBottomRayHit);

			if(pkNode->GetName() == g_strLadderShadowNodeName)
			{
				pkShadow->SetTexture(g_strLadderShadowTexturePath);
			}

			++iShadowCount;
		}
	}

	return	iShadowCount;
}

void	PgCircleShadow::SetTexture(char const* strTexturePath)
{
	if(m_spShadowGeom == 0) return;

	NiSourceTexture	*pTex = g_kNifMan.GetTexture(strTexturePath);
	if(!pTex)
	{
		return;
	}
	
	NiGeometry *pGeom = NiDynamicCast(NiGeometry,m_spShadowGeom->GetObjectByName("Plane01"));

	if(pGeom)
	{
		pGeom->GetPropertyState()->GetTexturing()->SetBaseTexture(pTex);
	}

	m_spShadowGeom->UpdateProperties();
}

void	PgCircleShadow::ShowShadowRecursive(NiAVObject *pkTargetNode,bool const bShow)
{
	NiNode*	pkNode = NiDynamicCast(NiNode,pkTargetNode);
	if(pkNode)
	{
		const	int	iTotalChild = pkNode->GetArrayCount();
		if(iTotalChild>0)
		{
			for(int i=0;i<iTotalChild;i++)
			{
				NiAVObject*	pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					ShowShadowRecursive(pkChild,bShow);
				}
			}
		}
	}


	PgCircleShadow	*pkShadow = NiDynamicCast(PgCircleShadow,pkTargetNode);
	if(pkShadow)
	{
		pkShadow->SetAppCulled(bShow == false);
	}
}

PgCircleShadow*	PgCircleShadow::AttachNewShadowTo(NiNode *pkTargetNode,
	float const fMaxShadowDistance,
	float const fShadowScale,
	NiAVObject const* pkShadowNif,
	NiAVObject	*pSrcObject,
	NxRaycastHit *pkBottomRayHit)
{
	PgCircleShadow	*pkNewShadow = GetShadowChild(NiDynamicCast(NiAVObject,pkTargetNode));
	if(pkNewShadow == NULL)
	{
		pkNewShadow = NiNew PgCircleShadow();
		pkTargetNode->AttachChild(pkNewShadow, true);
	}

	pkNewShadow->SetBottomRayHit(pkBottomRayHit);
	pkNewShadow->SetMaxShadowDistance(fMaxShadowDistance);
	pkNewShadow->SetShadowScale(fShadowScale);
	pkNewShadow->SetPositionSourceObject(pSrcObject);
	if(pkShadowNif)
	{
		pkNewShadow->SetShadowNif(const_cast<NiAVObject*>(pkShadowNif));
	}

	pkTargetNode->UpdateNodeBound();
	pkTargetNode->UpdateProperties();
	pkTargetNode->UpdateEffects();

	return	pkNewShadow;
}
PgCircleShadow*	PgCircleShadow::GetShadowChild(NiAVObject* pkTargetNode)
{
	NiNode*	pkNode = NiDynamicCast(NiNode,pkTargetNode);
	if(pkNode)
	{
		int	const iTotalChild = pkNode->GetArrayCount();
		if(iTotalChild>0)
		{
			for(int i=0;i<iTotalChild;i++)
			{
				NiAVObject*	pkChild = pkNode->GetAt(i);
				if(pkChild)
				{
					PgCircleShadow	*pkShadow = NiDynamicCast(PgCircleShadow,pkChild);
					if(pkShadow)
					{
						return	pkShadow;
					}
				}
			}
		}
	}

	return	NULL;
}
void	PgCircleShadow::LoadNif()
{
	NiNodePtr spShadowGeom = g_kNifMan.GetNif(g_strCircleShadowGeomPath);
	if(spShadowGeom == 0) return;

	NiSourceTexture	*pTex = g_kNifMan.GetTexture(g_strCircleShadowTexturePath);
	
	NiGeometry *pGeom = NiDynamicCast(NiGeometry,spShadowGeom->GetObjectByName("Plane01"));

	if(pGeom)
	{
		pGeom->GetModelData()->SetConsistency(NiGeometryData::STATIC);
		pGeom->GetPropertyState()->GetTexturing()->SetBaseTexture(pTex);
		pGeom->GetPropertyState()->GetZBuffer()->SetZBufferWrite(false);

		NiAlphaProperty	*pkAlpha = pGeom->GetPropertyState()->GetAlpha();
		if(pkAlpha)
		{
			pkAlpha->SetAlphaGroup(AG_SHADOW);
		}
	}


	spShadowGeom->UpdateProperties();
	SetShadowNif(NiDynamicCast(NiAVObject,spShadowGeom));
}
void	PgCircleShadow::SetShadowNif(NiAVObject* pkNewNif)
{
	if(pkNewNif == 0)
	{
		return;
	}

    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkNewNif, false );
	m_spShadowGeom = pkNewNif;
}
