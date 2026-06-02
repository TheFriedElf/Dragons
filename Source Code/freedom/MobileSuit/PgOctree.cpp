#include "stdafx.h"
#include "PgOctree.h"
#include "PgRenderer.H"

PgBoundingBox::PgBoundingBox(const	NiBound &kBound)
{
	Set(kBound);
}
PgBoundingBox::PgBoundingBox(const	NiPoint3 &vCenter, float fRadius)
{
	Set(vCenter,fRadius);
}

PgBoundingBox::PgBoundingBox(const	NiPoint3 &vMin,const	NiPoint3 &vMax)
{
	Set(vMin,vMax);
}
void	PgBoundingBox::Set(const	NiBound &kBound)
{
	const	NiPoint3	&vCenter = kBound.GetCenter();
	float	fRadius	=	kBound.GetRadius();

	Set(vCenter,fRadius);
}
void	PgBoundingBox::Set(const	NiPoint3 &vCenter, float fRadius)
{
	m_vMin.x = vCenter.x - fRadius;
	m_vMin.y = vCenter.y - fRadius;
	m_vMin.z = vCenter.z - fRadius;

	m_vMax.x = vCenter.x + fRadius;
	m_vMax.y = vCenter.y + fRadius;
	m_vMax.z = vCenter.z + fRadius;
}
void	PgBoundingBox::Set(const	NiPoint3 &vMin,const		NiPoint3 &vMax)
{
	m_vMin = vMin;
	m_vMax = vMax;
}
bool	PgBoundingBox::IsInBox(const	NiPoint3	vCenter,float fRadius)
{
	if (vCenter.x < m_vMin.x && m_vMin.x - vCenter.x > fRadius)
		return false;

	if (vCenter.x > m_vMax.x && vCenter.x  - m_vMax.x > fRadius)
		return false;

	if (vCenter.y < m_vMin.y && m_vMin.y - vCenter.y > fRadius)
		return false;

	if (vCenter.y > m_vMax.y && vCenter.y  - m_vMax.y > fRadius)
		return false;

	if (vCenter.z < m_vMin.z && m_vMin.z - vCenter.z > fRadius)
		return false;

	if (vCenter.z > m_vMax.z && vCenter.z  - m_vMax.z > fRadius)
		return false;

	return true;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
PgSphereTree::PgSphereTree()
{	
	m_bTopNodeInit = false;
}
void	PgSphereTree::AddNode(NiNode	*pStartNode)
{
	AddGeometry(pStartNode);

	if(!m_bTopNodeInit)
	{
		m_kTopNode.m_kBound = pStartNode->GetWorldBound();
		m_bTopNodeInit = true;
	}
	else
	{
		m_kTopNode.m_kBound.Merge(&pStartNode->GetWorldBound());
	}
	
}
void	PgSphereTree::AddGeometry(NiAVObject	*pkObject)
{
	if(pkObject->GetAppCulled())
	{
		return;
	}

	NiNode	*pkNode = NiDynamicCast(NiNode,pkObject);
	if(pkNode)
	{
		int iArrayCount = pkNode->GetArrayCount();
		for(int i=0;i<iArrayCount;i++)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				AddGeometry(pkChild);
			}
		}
	}
	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkObject);
	if(pkGeom)
	{
		m_kTopNode.m_kObjList.push_back(pkGeom);
	}
}
int CountTreeNode(stTreeNode *pkNode)
{
	int	iCount = 0;
	for(int i=0;i<pkNode->m_iTotalChild;i++)
	{
		stTreeNode	*pkChild = *(pkNode->m_paChildArray+i);
		iCount += CountTreeNode(pkChild);
	}

	return	iCount+pkNode->m_iTotalChild;
}
void	PgSphereTree::GenerateTree(NiNode	*pStartNode)
{
	GenerateRecursive(pStartNode,0);
}
void	PgSphereTree::GenerateRecursive(NiAVObject *pkObject,int iLevel)
{
	if(!pkObject)
	{
		return;
	}

	if(pkObject->GetAppCulled())
	{
		return;
	}

	NiBound	const	&kBound = pkObject->GetWorldBound();

	float	fMinRadius = lua_tinker::call<float>("GetMinSphereRadius");
	float	fMaxRadius = kBound.GetRadius()/2.0f;
	if(fMaxRadius<fMinRadius)
	{
		return;
	}

	NiNode	kTempNode;

	NiNode	*pkNode = NiDynamicCast(NiNode,pkObject);
	if(!pkNode)
	{
		return;
	}

	int	iArrayCount = pkNode->GetArrayCount();
	if(iArrayCount <= 1)
	{
		return;
	}

	for(int i=0;i<iArrayCount;i++)
	{
		NiAVObject	*pkChild = pkNode->GetAt(i);
		if(pkChild)
		{
			kTempNode.AttachChild(pkChild, true);
		}
	}

	iArrayCount = kTempNode.GetArrayCount();
	for(int i=0;i<iArrayCount;i++)
	{
		NiAVObject	*pkChild = kTempNode.GetAt(i);
		if(pkChild)
		{
			NiNode	*pkAttachNode = FindAddNode(pkNode,pkChild,fMaxRadius);
			if(pkAttachNode == NULL)
			{
				pkAttachNode = NiNew NiNode();
				pkNode->AttachChild(pkAttachNode, true);
			}
			pkAttachNode->AttachChild(pkChild, true);
			pkNode->Update(0);
			pkNode->UpdateWorldBound();
		}
	}

	iArrayCount = pkNode->GetArrayCount();
	for(int i=0;i<iArrayCount;i++)
	{
		NiAVObject	*pkChild = pkNode->GetAt(i);
		if(pkChild)
		{
			GenerateRecursive(pkChild,iLevel+1);
		}
	}

}
void PgSphereTree::CullingProcess(PgRenderer *pkRenderer,NiCamera *pkCamera, NiVisibleArray *pkVisibleArray,stTreeNode *pkNode)
{
	if(!pkNode)
	{
		pkNode = &m_kTopNode;
		pkVisibleArray->RemoveAll();

		m_kPlanes.Set(pkCamera->GetViewFrustum(), pkCamera->GetWorldTransform());
		m_kPlanes.EnableAllPlanes();
	}

	if(m_kPlanes.IsAnyPlaneActive() == false)
	{
		for(int i=0;i<pkNode->m_iTotalObject;i++)
		{
			NiGeometry	*pkGeom = (NiGeometry*)(*(pkNode->m_paObjArray+i));
			if(pkGeom->GetAppCulled() == false)
			{
				pkVisibleArray->Add(*pkGeom);
			}
		}

		for(int i=0;i<pkNode->m_iTotalChild;i++)
		{
			stTreeNode	*pkChild = *(pkNode->m_paChildArray+i);
			CullingProcess(pkRenderer,pkCamera,pkVisibleArray,pkChild);
		}
		return;
	}
	

	unsigned int uiSaveActive = m_kPlanes.GetActivePlaneState();

 // Determine if the object is not visible by comparing its world
    // bound to each culling plane.
    unsigned int i;
    for (i = 0; i < NiFrustumPlanes::MAX_PLANES; i++)
    {
        if (m_kPlanes.IsPlaneActive(i))
        {
			int iSide = pkNode->m_kBound.WhichSide(
				m_kPlanes.GetPlane(i));

			if (iSide == NiPlane::NEGATIVE_SIDE)
			{
				// The object is not visible since it is on the negative
				// side of the plane.
				break;
			}
            if (iSide == NiPlane::POSITIVE_SIDE)
            {
                // The object is fully on the positive side of the plane,
                // so there is no need to compare child objects to this
                // plane.
                m_kPlanes.DisablePlane(i);
            }
		}
    }

	if(i == NiFrustumPlanes::MAX_PLANES)
	{
		for(int i=0;i<pkNode->m_iTotalObject;i++)
		{
			NiGeometry	*pkGeom = (NiGeometry*)(*(pkNode->m_paObjArray+i));
			CullingProcess(pkRenderer,pkVisibleArray,pkGeom);

		}


		for(int i=0;i<pkNode->m_iTotalChild;i++)
		{
			stTreeNode	*pkChild = *(pkNode->m_paChildArray+i);
			CullingProcess(pkRenderer,pkCamera,pkVisibleArray,pkChild);
		}
	}


	m_kPlanes.SetActivePlaneState(uiSaveActive);

}
void PgSphereTree::CullingProcess(PgRenderer *pkRenderer,NiVisibleArray *pkVisibleArray,NiAVObject *pkAVObject)
{
	if(pkAVObject->GetAppCulled())
	{
		return;
	}

	unsigned int uiSaveActive = m_kPlanes.GetActivePlaneState();

 // Determine if the object is not visible by comparing its world
    // bound to each culling plane.
    unsigned int i;
    for (i = 0; i < NiFrustumPlanes::MAX_PLANES; i++)
    {
        if (m_kPlanes.IsPlaneActive(i))
        {
			int iSide = pkAVObject->GetWorldBound().WhichSide(
				m_kPlanes.GetPlane(i));

			if (iSide == NiPlane::NEGATIVE_SIDE)
			{
				// The object is not visible since it is on the negative
				// side of the plane.
				break;
			}
            if (iSide == NiPlane::POSITIVE_SIDE)
            {
                // The object is fully on the positive side of the plane,
                // so there is no need to compare child objects to this
                // plane.
                m_kPlanes.DisablePlane(i);
            }
		}
    }

	if(i == NiFrustumPlanes::MAX_PLANES)
	{

		NiGeometry *pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
		if(pkGeom)
		{
			pkVisibleArray->Add(*pkGeom);
		}

	}

	m_kPlanes.SetActivePlaneState(uiSaveActive);
}

NiNode*	PgSphereTree::FindAddNode(NiNode *pkParentNode,NiAVObject *pkObject,float const fMaxRadius)
{
	NiBound	const	&kObjBound = pkObject->GetWorldBound();

	int	iArrayCount = pkParentNode->GetArrayCount();
	for(int i=0;i<iArrayCount;i++)
	{
		NiAVObject	*pkChild = pkParentNode->GetAt(i);
		if(pkChild)
		{
			NiBound	kBound = pkChild->GetWorldBound();
			kBound.Merge(&kObjBound);
			if(kBound.GetRadius()>fMaxRadius)
			{
				continue;
			}

			return	(NiNode*)pkChild;
		}
	}
	return	NULL;
}

void	PgSphereTree::SliceNodeWithPlane(NiNode *pTargetNode,NiPlane Plane,NiNode **pout_NewNode1,NiNode **pout_NewNode2)
{
	NiNode	*pParent = pTargetNode->GetParent();
	assert(pParent);
	pParent->DetachChild(pTargetNode);
	int	iTotalChild = pParent->GetArrayCount();
	for(int i=0;i<iTotalChild;i++)
	{
	}
}
