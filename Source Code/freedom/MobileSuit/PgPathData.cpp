#include "stdafx.h"
#include "PgPathData.H"

//////////////////////////////////////////////////////////////////////////////////
//	class	PgPathTriangle
//////////////////////////////////////////////////////////////////////////////////
PgPathTriangle::PgPathTriangle(const stTriangle &kPoint,unsigned	int	uiID)
	:m_uiID(uiID),
	m_kPoints(kPoint)
{
//	m_vFaceNormal 
}

PgPathTriangle::~PgPathTriangle()
{
	for(ContPortal::iterator itor = m_PortalCont.begin();itor!=m_PortalCont.end();itor++)
	{
		stPortal *pkPortal = *itor;
		SAFE_DELETE(pkPortal);
	}
	m_PortalCont.clear();
}

bool	PgPathTriangle::CanConnectToThisTriangle(const stTriangle &kPoint)
{

	//	같은 점이 2개 있으면, 연결 가능하다.
	
	int	iSamePointCount=0;
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			if( (m_kPoints.m_vPoints[i] - kPoint.m_vPoints[j]).Length() < 5)
			{
				iSamePointCount++;
				break;
			}
		}
	}

	return	(iSamePointCount==2);
}
void	PgPathTriangle::ConnectToThisTriangle(PgPathTriangle *pkTriangle)
{
	NiPoint3	kPortalPoint[2];
	int	iSamePointCount=0;
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			if( (m_kPoints.m_vPoints[i]-pkTriangle->GetPoints().m_vPoints[j]).Length() < 5 )
			{
				kPortalPoint[iSamePointCount++] = m_kPoints.m_vPoints[i];
				break;
			}
		}
	}

	if(iSamePointCount!=2)
	{
		return;
	}

	m_PortalCont.push_back(new stPortal(kPortalPoint[0],kPortalPoint[1],pkTriangle));
}

//////////////////////////////////////////////////////////////////////////////////
//	class	PgPathGroup
//////////////////////////////////////////////////////////////////////////////////
PgPathGroup::PgPathGroup(unsigned int uiGroupID) 
:	m_uiGroupID(uiGroupID),
	m_uiTriCount(0),
	m_uiTriangleID(0),
	m_pkFirstTri(0)
{
}
PgPathGroup::~PgPathGroup()
{
	ClearAll();
}
void	PgPathGroup::ClearAll()
{
	SAFE_DELETE(m_pkFirstTri);
	m_uiGroupID=-1;
	m_uiTriCount=0;
	m_uiTriangleID=0;
}

bool	PgPathGroup::CanConnectToThisGroup(const stTriangle &kPoint)
{
	return	(GetConnetableTriangle(kPoint,m_pkFirstTri)!=NULL);
}
void	PgPathGroup::ConnectToThisGroup(const stTriangle &kPoint)
{
	PgPathTriangle	*pkTargetTriangle = GetConnetableTriangle(kPoint,m_pkFirstTri);

	m_uiTriCount++;

	PgPathTriangle	*pkNewTriangle = new PgPathTriangle(kPoint,m_uiTriangleID++);
	if(m_pkFirstTri == NULL)
	{
		m_pkFirstTri = pkNewTriangle;
		return;
	}

	pkTargetTriangle->ConnectToThisTriangle(pkNewTriangle);
}

PgPathTriangle*	PgPathGroup::GetConnetableTriangle(const stTriangle &kPoint,PgPathTriangle *pkTriangle)
{
	if(!pkTriangle)
	{
		return	NULL;
	}

	if(pkTriangle->CanConnectToThisTriangle(kPoint))
	{
		return	pkTriangle;
	}


	const	PgPathTriangle::ContPortal	&kPortalCont = pkTriangle->GetPortalCont();
	for(PgPathTriangle::ContPortal::const_iterator itor = kPortalCont.begin();
		itor != kPortalCont.end(); itor++)
	{
		PgPathTriangle::stPortal *pkPortal = *itor;
		if(!pkPortal->m_pNTri)
		{
			continue;
		}

		PgPathTriangle *pkTriangle = GetConnetableTriangle(kPoint,pkPortal->m_pNTri);
		if(pkTriangle)
		{
			return	pkTriangle;
		}
	}

	return	NULL;
}

//////////////////////////////////////////////////////////////////////////////////
//	class	PgPathData
//////////////////////////////////////////////////////////////////////////////////

PgPathData::PgPathData()
:m_uiGroupID(0)
{
}
PgPathData::~PgPathData()
{
	ClearData();
}
void	PgPathData::ClearData()
{
	for(ContPathGroup::iterator itor = m_PathGroupCont.begin(); itor != m_PathGroupCont.end(); itor++)
	{
		PgPathGroup	*pkGroup = (*itor);
		SAFE_DELETE(pkGroup);
	}
	m_PathGroupCont.clear();
	m_uiGroupID = 0;
}
bool	PgPathData::CreatePathDataFromPhysX(NxScene *pkScene)
{

	const	NxDebugRenderable	*pkDebugRenderable = pkScene->getDebugRenderable();
	if(!pkDebugRenderable)
	{
		return	false;
	}

	NxU32	nbActors = pkScene->getNbActors();

	NxActor	**ppkActors = pkScene->getActors();
	for(NxU32 i=0;i<nbActors;i++)
	{
		NxActor	*pkActor = *ppkActors;

		NxU32	nbShapes = pkActor->getNbShapes();
		for(NxU32 j=0;j<nbShapes;j++)
		{
			NxShape	*pkShape = *(pkActor->getShapes()+j);
			if(pkShape)
			{
				NxShapeType	kType = pkShape->getType();
				if(kType == NX_SHAPE_MESH)
				{
					const	NxTriangleMeshShape	*pkTriangleShape = pkShape->isTriangleMesh();
					if(pkTriangleShape)
					{
						NxTriangleMeshShapeDesc	kDesc;
						pkTriangleShape->saveToDesc(kDesc);

						int	iTotalTriangle = kDesc.meshData->getCount(0,NX_ARRAY_TRIANGLES);

						for(int k=0;k<iTotalTriangle;k++)
						{
							NxTriangle	kTriangle;
							pkTriangleShape->getTriangle(kTriangle,NULL,NULL,k);

							NxVec3	kNormal;
							kTriangle.normal(kNormal);

							AddTriangle(stTriangle(kTriangle.verts[0],kTriangle.verts[1],kTriangle.verts[2],kNormal));

						}

					}
				}
				
			}
		}

		ppkActors++;
	}

	return	true;
}

void	PgPathData::AddTriangle(const stTriangle &kPoint)
{
	PgPathGroup	*pkGroup = GetCorrectGroup(kPoint);
	if(!pkGroup)
	{
		pkGroup = CreateNewGroup();
		m_PathGroupCont.push_back(pkGroup);
	}

	pkGroup->ConnectToThisGroup(kPoint);
}
PgPathGroup*	PgPathData::GetCorrectGroup(const stTriangle &kPoint)
{
	
	for(ContPathGroup::iterator itor = m_PathGroupCont.begin(); itor != m_PathGroupCont.end(); itor++)
	{
		PgPathGroup	*pkGroup = (*itor);
		if(pkGroup->CanConnectToThisGroup(kPoint))
		{
			return	pkGroup;
		}
	}
	return	NULL;
}
PgPathGroup*	PgPathData::CreateNewGroup()
{
	return	new PgPathGroup(m_uiGroupID++);
}