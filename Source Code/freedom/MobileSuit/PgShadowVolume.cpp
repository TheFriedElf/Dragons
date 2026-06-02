#include "stdafx.h"
#include "PgShadowVolume.H"

PgShadowVolume::PgShadowVolume()
{
	m_spShadowGeom = 0;
	m_spQuad = 0;

	m_iActiveTriCount = 0;
	m_iActiveEdgeCount = 0;

	m_vTriangles.reserve(3000);
	m_vEdges.reserve(3000);

}
PgShadowVolume::~PgShadowVolume()
{
	m_spShadowGeom = 0;
	m_spQuad = 0;
}
void	PgShadowVolume::UpdateShadowVolume(NiVisibleArray	*pkVisibleArray,const	NiPoint3	&kLightDir)
{
	m_iActiveTriCount = 0;
	m_iActiveEdgeCount = 0;

	m_vTriangles.clear();
	m_vEdges.clear();

	//	라이트 방향에서 보이는 삼각형만 추려낸다.
	CullTriangleFromLightDir(pkVisibleArray,kLightDir,m_vTriangles,m_vEdges);

	//	처음 추려낸 삼각형 리스트와 합친다.
	CreateFinalGeometry(m_vTriangles,m_vEdges);

}
void	PgShadowVolume::SetupStencilBuffer(PgRenderer *pkRenderer)
{

	if(!m_spShadowGeom)
	{
		return;
	}

	NiPropertyState	*pkProp = m_spShadowGeom->GetPropertyState();
	if(!pkProp)
	{
		return;
	}

	NiZBufferProperty	*pkZBuffer = pkProp->GetZBuffer();
	if(!pkZBuffer)
	{
		pkZBuffer = NiNew NiZBufferProperty();
		m_spShadowGeom->AttachProperty(pkZBuffer);
	}
	
	pkZBuffer->SetZBufferTest(true);
	pkZBuffer->SetZBufferWrite(false);

	NiStencilProperty	*pkStencil = pkProp->GetStencil();
	if(!pkStencil)
	{
		pkStencil = NiNew NiStencilProperty();
		m_spShadowGeom->AttachProperty(pkStencil);
	}

	pkStencil->SetStencilOn(true);
	pkStencil->SetStencilFunction(NiStencilProperty::TEST_ALWAYS);
	pkStencil->SetStencilPassZFailAction(NiStencilProperty::ACTION_KEEP);
	pkStencil->SetStencilFailAction(NiStencilProperty::ACTION_KEEP);
	pkStencil->SetStencilMask(0xffffffff);
	pkStencil->SetStencilReference(0x1);

	m_spShadowGeom->UpdateProperties();

}
void	PgShadowVolume::RenderQuad(PgRenderer *pkRenderer,NiCamera*	pkCamera)
{
	if(!m_spQuad)
	{
		m_spQuad = NiNew NiScreenElements(NiNew NiScreenElementsData(false, false, 0));
		m_spQuad->Insert(4);
		m_spQuad->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
		m_spQuad->UpdateBound();
		m_spQuad->SetColors(0,NiColorA::BLACK,NiColorA::BLACK,NiColorA::BLACK,NiColorA::BLACK);

		NiStencilProperty	*pkStencil = NiNew NiStencilProperty();

		pkStencil->SetStencilOn(true);
		pkStencil->SetStencilFunction(NiStencilProperty::TEST_LESSEQUAL);
		pkStencil->SetStencilPassZFailAction(NiStencilProperty::ACTION_KEEP);
		pkStencil->SetStencilFailAction(NiStencilProperty::ACTION_KEEP);
		pkStencil->SetStencilMask(0xffffffff);
		pkStencil->SetStencilReference(0x1);
		m_spQuad->AttachProperty(pkStencil);
	}

	m_spQuad->RenderImmediate(pkRenderer->GetRenderer());

	NiDX9Renderer *pDX9Renderer = (NiDX9Renderer*)pkRenderer->GetRenderer();
	pDX9Renderer->GetRenderState()->SetRenderState(D3DRS_STENCILENABLE,false);
	pDX9Renderer->GetRenderState()->SetRenderState(D3DRS_CULLMODE,   D3DCULL_CW);
	
}

void	PgShadowVolume::RenderImmediate(PgRenderer *pkRenderer,NiCamera*	pkCamera,bool bFront)
{
	if(!m_spShadowGeom)
	{
		return;
	}
	NiPropertyState	*pkProp = m_spShadowGeom->GetPropertyState();

	NiAlphaProperty	*pkAlpha = pkProp->GetAlpha();
	if(!pkAlpha)
	{
		pkAlpha = NiNew NiAlphaProperty();
		m_spShadowGeom->AttachProperty(pkAlpha);
	}
	pkAlpha->SetAlphaBlending(true);
	pkAlpha->SetSrcBlendMode(NiAlphaProperty::ALPHA_ZERO);
	pkAlpha->SetDestBlendMode(NiAlphaProperty::ALPHA_ONE);

	NiStencilProperty	*pkStencil = pkProp->GetStencil();

	if(bFront)
	{
		pkStencil->SetStencilPassAction(NiStencilProperty::ACTION_INCREMENT);
		pkStencil->SetDrawMode(NiStencilProperty::DRAW_CCW);
	}
	else
	{
		pkStencil->SetStencilPassAction(NiStencilProperty::ACTION_DECREMENT);
		pkStencil->SetDrawMode(NiStencilProperty::DRAW_CW);
	}

	m_spShadowGeom->UpdateProperties();
	m_spShadowGeom->Update(0);

	pkRenderer->PartialRenderClick_Deprecated(m_spShadowGeom);
}

void	PgShadowVolume::CullTriangleFromLightDir(NiVisibleArray	*pkVisibleArray,const	NiPoint3	&kLightDir,VTri &kCulledTri,VEdge &kEdge)
{

	int	iTotalGeom = pkVisibleArray->GetCount();
	for(int s=0;s<iTotalGeom;s++)
	{
		NiTriBasedGeom	*pkGeom = NiDynamicCast(NiTriBasedGeom,&(pkVisibleArray->GetAt(s)));

		if(pkGeom)
		{
			const	NiTransform	&kTransform = pkGeom->GetWorldTransform();

			int	iTotalTri = pkGeom->GetTriangleCount();
			NiPoint3	*kPoint[3];
			stTri	kTri;
			for(int i=0;i<iTotalTri;i++)
			{
				pkGeom->GetModelTriangle(i,kPoint[0],kPoint[1],kPoint[2]);

				kTri.m_kPoint[0] = kTransform*(*kPoint[0]);
				kTri.m_kPoint[1] = kTransform*(*kPoint[1]);
				kTri.m_kPoint[2] = kTransform*(*kPoint[2]);

				NiPoint3	kVec[2];

				kVec[0] = kTri.m_kPoint[1] - kTri.m_kPoint[0];
				kVec[1] = kTri.m_kPoint[2] - kTri.m_kPoint[1];

				kVec[0].Unitize();
				kVec[1].Unitize();

				kTri.m_kNormal = kVec[0].Cross(kVec[1]);

				float	fDot = kTri.m_kNormal.Dot(kLightDir);
				if(fDot<=0) continue;

				kCulledTri.push_back(kTri);
				m_iActiveTriCount++;

				AddEdgeToEdgeList(kTri.m_kPoint[0],kTri.m_kPoint[1],kEdge,kLightDir);
				AddEdgeToEdgeList(kTri.m_kPoint[1],kTri.m_kPoint[2],kEdge,kLightDir);
				AddEdgeToEdgeList(kTri.m_kPoint[2],kTri.m_kPoint[0],kEdge,kLightDir);

			}
		}
	}
	
}

void	PgShadowVolume::CreateEdgeList(VTri &kCulledTri,VEdge &kEdge)
{
}
void	PgShadowVolume::CreateRectPoly( VEdge &kEdge,const	NiPoint3	&kLightDir)
{

	float	fMaxDistance = 1000.0f;
	int	iTotal = kEdge.size();
	NiPoint3	kAddVector = kLightDir*fMaxDistance;

	NiPoint3	kPoint[4];
	for(VEdge::iterator itor = kEdge.begin(); itor != kEdge.end(); itor++)
	{	
		stEdge	*pkEdge = &(*itor);

		kPoint[0] = pkEdge->m_kPoint[0];
		kPoint[1] = pkEdge->m_kPoint[1];
		kPoint[2] = kPoint[1]+kAddVector;
		kPoint[3] = kPoint[0]+kAddVector;

		pkEdge->m_kTriangle[0].m_kPoint[0] = kPoint[0];
		pkEdge->m_kTriangle[0].m_kPoint[1] = kPoint[2];
		pkEdge->m_kTriangle[0].m_kPoint[2] = kPoint[3];

		pkEdge->m_kTriangle[1].m_kPoint[0] = kPoint[0];
		pkEdge->m_kTriangle[1].m_kPoint[1] = kPoint[1];
		pkEdge->m_kTriangle[1].m_kPoint[2] = kPoint[2];
	}
}
void	PgShadowVolume::CreateFinalGeometry(VTri &kCulledTri,VEdge &kEdge)
{
	int	iTotalTriangle = m_iActiveTriCount + m_iActiveEdgeCount*2;
	int	iTotalVert = iTotalTriangle*3;

	NiTriShapeData	*pkModelData = NULL;
	NiPoint3	*pkVerts = NULL;
	
	if(m_spShadowGeom)
	{
		pkModelData = (NiTriShapeData*)m_spShadowGeom->GetModelData();
		if(pkModelData->GetVertexCount() < iTotalVert)
		{
			m_spShadowGeom = 0;

			pkModelData = NULL;
			pkVerts = NiNew NiPoint3[iTotalVert];
		}
		else
		{
			pkVerts = m_spShadowGeom->GetVertices();
		}
	}
	else
	{
		pkVerts = NiNew NiPoint3[iTotalVert];
	}

	//	먼저 Edge부터 Add하자.
	int	iTotalEdge = kEdge.size();
	NiPoint3	*pkVerts2 = pkVerts;
	for(int i=0;i<m_iActiveEdgeCount;i++)
	{
		stEdge	*pkEdge = &kEdge[i];

		*(pkVerts2++) = pkEdge->m_kTriangle[0].m_kPoint[0];
		*(pkVerts2++) = pkEdge->m_kTriangle[0].m_kPoint[1];
		*(pkVerts2++) = pkEdge->m_kTriangle[0].m_kPoint[2];

		*(pkVerts2++) = pkEdge->m_kTriangle[1].m_kPoint[0];
		*(pkVerts2++) = pkEdge->m_kTriangle[1].m_kPoint[1];
		*(pkVerts2++) = pkEdge->m_kTriangle[1].m_kPoint[2];
	}

	//	다음 컬링된 삼각형Add
	int	iTotalCulledTri = m_iActiveTriCount;
	for(int i=0;i<m_iActiveTriCount;i++)
	{
		stTri	*pkTri = &kCulledTri[i];

		*(pkVerts2++) = pkTri->m_kPoint[0];
		*(pkVerts2++) = pkTri->m_kPoint[1];
		*(pkVerts2++) = pkTri->m_kPoint[2];
	}


	if(pkModelData)
	{
	/*	if(pkModelData->GetTriangleCount() > iTotalTriangle)
		{
			pkModelData->SetActiveTriangleCount(iTotalTriangle);
			pkModelData->MarkAsChanged(NiTriBasedGeomData::TRIANGLE_COUNT_MASK);
		}*/
		pkModelData->MarkAsChanged(NiGeometryData::VERTEX_MASK);
	}
	else
	{
		int	iTotalIndex = iTotalTriangle*3;
		unsigned	short	*pkIndex = NiAlloc(unsigned short,iTotalIndex);
		for(int j=0;j<iTotalIndex;j++)
		{
			*(pkIndex+j) = j;
		}


		pkModelData = NiNew NiTriShapeData(iTotalVert,pkVerts,NULL,NULL,NULL,0,NiGeometryData::NBT_METHOD_NONE,iTotalTriangle,pkIndex);
		m_spShadowGeom = NiNew NiTriShape(pkModelData);

		NiMaterialProperty	*pkMat =NiNew NiMaterialProperty();
		pkMat->SetDiffuseColor(NiColor(0,0,0));
		pkMat->SetAmbientColor(NiColor::BLACK);
		pkMat->SetEmittance(NiColor::BLACK);
		pkMat->SetAlpha(1.0f);

		m_spShadowGeom->AttachProperty(pkMat);
		m_spShadowGeom->UpdateProperties();
		m_spShadowGeom->Update(0);
	}
	
}
void	PgShadowVolume::AddEdgeToEdgeList(NiPoint3 const &kP1,NiPoint3 const &kP2,VEdge &kEdgeList, const	NiPoint3	&kLightDir)
{

	//	기존에 있는 것인지 체크한다.
	int	iTotalEdge = kEdgeList.size();
	for(int i=0;i<m_iActiveEdgeCount;i++)
	{
		stEdge	*pkEdge = &kEdgeList[i];

		if( (pkEdge->m_kPoint[0] == kP1 && pkEdge->m_kPoint[1] == kP2) ||
			(pkEdge->m_kPoint[1] == kP1 && pkEdge->m_kPoint[0] == kP2) )
		{
			return;
		}
	}

	stEdge	kNewEdge;

	kNewEdge.m_kPoint[0] = kP1;
	kNewEdge.m_kPoint[1] = kP2;

	float	fMaxDistance = 100.0f;
	NiPoint3	kAddVector = kLightDir*fMaxDistance;

	NiPoint3	kPoint[4];
	stEdge	*pkEdge = &kNewEdge;

	kPoint[0] = pkEdge->m_kPoint[0];
	kPoint[1] = pkEdge->m_kPoint[1];
	kPoint[2] = kPoint[1]+kAddVector;
	kPoint[3] = kPoint[0]+kAddVector;

	pkEdge->m_kTriangle[0].m_kPoint[0] = kPoint[0];
	pkEdge->m_kTriangle[0].m_kPoint[1] = kPoint[2];
	pkEdge->m_kTriangle[0].m_kPoint[2] = kPoint[3];

	pkEdge->m_kTriangle[1].m_kPoint[0] = kPoint[0];
	pkEdge->m_kTriangle[1].m_kPoint[1] = kPoint[1];
	pkEdge->m_kTriangle[1].m_kPoint[2] = kPoint[2];

	kEdgeList.push_back(kNewEdge);
	m_iActiveEdgeCount++;
}