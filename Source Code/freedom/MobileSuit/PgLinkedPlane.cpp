#include "stdafx.h"
#include "PgLinkedPlane.H"
#include "PgNifMan.H"
#include "PgMobileSuit.H"
#include "PgWorld.H"
#include "NiGeometryData.H"

PgLinkedPlaneGroupMan g_kLinkedPlaneGroupMan;//체인 라이트닝에서 씀.

///////////////////////////////////////////////////////////////////////////////
//	class	PgLinkedPlane
///////////////////////////////////////////////////////////////////////////////

void PgLinkedPlane::Update(float fAccumTime,float fFrameTime)
{
	if(!m_spGeom)
	{
		return;
	}

	if(!m_bEnable)
	{
		return;
	}

	UpdateTextureCoordinates();

	m_spGeom->UpdateNodeBound();
	m_spGeom->Update(fAccumTime);
}
void PgLinkedPlane::DrawImmediate(PgRenderer *pkRenderer)
{
	if(!m_spGeom)
	{
		return;
	}
	if(!m_bEnable)
	{
		return;
	}

	pkRenderer->PartialRenderClick_Deprecated(m_spGeom);
}
void PgLinkedPlane::Create(int const iInstanceID,
	PgLinkedPlaneGroup *pkParentGroup,
	NiTexturingPropertyPtr spTexturingProp,
	NiAlphaPropertyPtr spAlphaProp,
	NiVertexColorPropertyPtr spVertexColorProp,
	NiZBufferPropertyPtr spZBufferProp,
	NiStencilPropertyPtr spStencilProp,
	NiMaterialPropertyPtr spMaterialProp
	)
{
	m_iInstanceID = iInstanceID;
	m_pkParentGroup = pkParentGroup;

	m_fWidth = 10.0f;

	m_bEnable = true;

	unsigned short usVertices = 4 * 2; //사각형 2개

	NiPoint3* pkVerts = NiNew NiPoint3[usVertices];
	memset(pkVerts,0,sizeof(NiPoint3) * usVertices);

	NiColorA* pkColors = NiNew NiColorA[usVertices];
	memset(pkColors,0xff,sizeof(NiColorA) * usVertices);

	NiPoint2* pkTextures = NiNew NiPoint2[usVertices];
	memset(pkTextures,0,sizeof(NiPoint2) * usVertices);

	unsigned short* pusStripLengths = NiAlloc(unsigned short, 2);
	pusStripLengths[0] = 4;
	pusStripLengths[1] = 4;

	unsigned short* pusStripList = NiAlloc(unsigned short, usVertices);
	unsigned short kList[] = {0, 1, 2, 3, 4, 5, 6, 7};
	memcpy(pusStripList, kList, sizeof(unsigned short) * (usVertices));

	unsigned short usTriangles = 4;
	unsigned short usStrips = 2;

	NiTriStripsDataPtr spGeomData = NiNew NiTriStripsData
		(usVertices,pkVerts,NULL,pkColors,pkTextures,1,NiGeometryData::NBT_METHOD_NONE,
		usTriangles,usStrips,pusStripLengths,pusStripList);
	spGeomData->SetConsistency(NiGeometryData::VOLATILE);

	m_spGeom = NiNew NiTriStrips(spGeomData);

	m_spGeom->AttachProperty(spTexturingProp);
	m_spGeom->AttachProperty(spAlphaProp);
	m_spGeom->AttachProperty(spVertexColorProp);
	m_spGeom->AttachProperty(spZBufferProp);
	m_spGeom->AttachProperty(spStencilProp);
	m_spGeom->AttachProperty(spMaterialProp);

	m_spGeom->UpdateProperties();
	m_spGeom->UpdateEffects();
	m_spGeom->Update(0);

	m_fLastTextureCoordinateUpdateTime = -1;
	UpdateTextureCoordinates();
}
void PgLinkedPlane::UpdateWorldBound()
{
	if(!m_spGeom)
	{
		return;
	}

	NiPoint3 kMin(99999.0f, 99999.0f, 99999.0f), kMax(-99999.0f, -99999.0f, -99999.0f);

	NiGeometryDataPtr	spGeomData = m_spGeom->GetModelData();

	int	iTotalVert = spGeomData->GetVertexCount();

	NiPoint3* pkVerts = spGeomData->GetVertices();

	for(int i = 0;i < iTotalVert; ++i)
	{
		if(pkVerts->x<kMin.x) kMin.x = pkVerts->x;
		if(pkVerts->y<kMin.y) kMin.y = pkVerts->y;
		if(pkVerts->z<kMin.z) kMin.z = pkVerts->z;

		if(pkVerts->x>kMax.x) kMax.x = pkVerts->x;
		if(pkVerts->y>kMax.y) kMax.y = pkVerts->y;
		if(pkVerts->z>kMax.z) kMax.z = pkVerts->z;

		++pkVerts;
	}

	float fLen = (kMax-kMin).Length();
	NiPoint3 kCenter = kMin + (kMax-kMin) / 2;

	NiBound	kBound;
	kBound.SetCenterAndRadius(kCenter, fLen / 2);

	spGeomData->SetBound(kBound);
}
void PgLinkedPlane::SetStartPos(NiPoint3 const& kPos)
{
	if(!m_spGeom)
	{
		return;
	}

	m_vStart = kPos;
}
void PgLinkedPlane::SetEndPos(NiPoint3 const& kPos)
{
	if(!m_spGeom)
	{
		return;
	}

	m_vEnd = kPos;

	NiGeometryDataPtr spGeomData = m_spGeom->GetModelData();
	NiPoint3* pkVerts = spGeomData->GetVertices();

	if(!g_pkWorld)
	{
		return;
	}
	
	NiPoint3 kDir = (m_vEnd-m_vStart);
	kDir.Unitize();

	NiCameraPtr spCamera = g_pkWorld->m_kCameraMan.GetCamera();
	if(!spCamera)
	{
		return;
	}

	NiPoint3 kCamDir = spCamera->GetWorldDirection();
	NiPoint3 kUp = kCamDir.Cross(kDir);
	NiPoint3 kHoriz = kUp.Cross(kDir);

	//	1,3,5,7 가 뒷쪽 버텍스이다.

	//     0_____________________________1
	//     |                             |
	//     |  4__________________________|__5
	//     | /                           | /
	//     |/.(Start)              (End).|/
	//     /                             /
	//    /|                            /|
	//  6/_|__________________________7/ |
	//     |                             |
	//     |                             |
	//     2_____________________________3

	*(pkVerts+0) = m_vStart+kUp*m_fWidth;
	*(pkVerts+2) = m_vStart-kUp*m_fWidth;
	*(pkVerts+4) = m_vStart+kHoriz*m_fWidth;
	*(pkVerts+6) = m_vStart-kHoriz*m_fWidth;

	*(pkVerts+1) = m_vEnd+kUp*m_fWidth;
	*(pkVerts+3) = m_vEnd-kUp*m_fWidth;
	*(pkVerts+5) = m_vEnd+kHoriz*m_fWidth;
	*(pkVerts+7) = m_vEnd-kHoriz*m_fWidth;

	spGeomData->MarkAsChanged(NiGeometryData::VERTEX_MASK);
	UpdateWorldBound();
}
void PgLinkedPlane::UpdateTextureCoordinates()
{
	if(!g_pkWorld)
	{
		return;
	}

	float fTime = g_pkWorld->GetAccumTime();

	if(-1 != m_fLastTextureCoordinateUpdateTime)
	{
		if(fTime - m_fLastTextureCoordinateUpdateTime < m_pkParentGroup->GetTextureCoordinateUpdateDelay())
		{
			return;
		}
	}

	m_fLastTextureCoordinateUpdateTime = fTime;

	if(!m_spGeom)
	{
		return;
	}

	NiGeometryDataPtr spGeomData = m_spGeom->GetModelData();

	NiPoint2* pkTextures = spGeomData->GetTextures();

	float const fTexSegWidth = 0.25f;

	int	const iTotalSeg = (int)(1.0f / 0.25f);
	int	const iSelectedSeg = BM::Rand_Index(iTotalSeg);

	*(pkTextures+0) = NiPoint2(iSelectedSeg*fTexSegWidth,0);
	*(pkTextures+1) = NiPoint2(iSelectedSeg*fTexSegWidth,1);
	*(pkTextures+2) = NiPoint2((iSelectedSeg+1)*fTexSegWidth,0);
	*(pkTextures+3) = NiPoint2((iSelectedSeg+1)*fTexSegWidth,1);

	*(pkTextures+4) = NiPoint2(iSelectedSeg*fTexSegWidth,0);
	*(pkTextures+5) = NiPoint2(iSelectedSeg*fTexSegWidth,1);
	*(pkTextures+6) = NiPoint2((iSelectedSeg+1)*fTexSegWidth,0);
	*(pkTextures+7) = NiPoint2((iSelectedSeg+1)*fTexSegWidth,1);

	spGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);

}
void PgLinkedPlane::Destroy()
{
	m_spGeom = NULL;
}

void PgLinkedPlane::UpdateProperties()
{
	if(m_spGeom)
	{
		m_spGeom->UpdateProperties();
	}	
}

void PgLinkedPlane::SetWidth(float const fWidth)
{
	m_fWidth = fWidth;

	SetStartPos(m_vStart);
	SetEndPos(m_vEnd);
}
float PgLinkedPlane::GetWidth()
{
	return m_fWidth;
}

void PgLinkedPlane::SetEnable(bool const bEnable)
{
	m_bEnable = bEnable;
}

bool PgLinkedPlane::GetEnable()
{
	return m_bEnable;
}

///////////////////////////////////////////////////////////////////////////////
//	class	PgLinkedPlaneGroup
///////////////////////////////////////////////////////////////////////////////
void PgLinkedPlaneGroup::Update(float fAccumTime,float fFrameTime)
{
	int	iCount = m_PlaneCont.size();
	for(int i = 0;i < iCount; ++i)
	{
		m_PlaneCont[i]->Update(fAccumTime,fFrameTime);
	}
}

void PgLinkedPlaneGroup::DrawImmediate(PgRenderer *pkRenderer)
{
	int	iCount = m_PlaneCont.size();
	for(int i = 0; i < iCount; ++i)
	{
		m_PlaneCont[i]->DrawImmediate(pkRenderer);
	}
}

void PgLinkedPlaneGroup::SetTexture(char const* strPath)
{
	NiSourceTexturePtr spNewTexture = g_kNifMan.GetTexture(std::string(strPath));
	 
	if(!spNewTexture)
	{
		return;
	}

	//	Plane 들의 텍스쳐를 새것으로 교체해주어야한다.
	m_spTexturingProp->SetBaseTexture(spNewTexture);

	int	iCount = m_PlaneCont.size();
	for(int i = 0;i < iCount; ++i)
	{
		m_PlaneCont[i]->UpdateProperties();
	}
}
void PgLinkedPlaneGroup::SetTextureCoordinateUpdateDelay(float const fDelay)
{
	m_fTextureCoordinateUpdateDelay = fDelay;
}

PgLinkedPlane*	PgLinkedPlaneGroup::GetPlane(int const iInstanceID)
{
	int	const iCount = m_PlaneCont.size();
	for(int i = 0; i < iCount; ++i)
	{
		if(iInstanceID == m_PlaneCont[i]->GetInstanceID())
		{
			return m_PlaneCont[i];
		}
	}

	return	NULL;
}

PgLinkedPlane* PgLinkedPlaneGroup::AddNewPlane(int const iAlphaTestRef)
{
	if( 0 != iAlphaTestRef )
	{
		m_spAlphaProp->SetTestRef( static_cast<unsigned char>(iAlphaTestRef) );
	}
	PgLinkedPlane* pkNewPlane = new PgLinkedPlane(
		m_iPlaneInstanceIDCounter++,
		this,
		m_spTexturingProp,
		m_spAlphaProp,
		m_spVertexColorProp,
		m_spZBufferProp,
		m_spStencilProp,m_spMaterialProp
		);
	m_PlaneCont.push_back(pkNewPlane);

	return pkNewPlane;
}

void PgLinkedPlaneGroup::ReleaseAllPlane()
{
	int	iCount = m_PlaneCont.size();
	for(int i = 0; i < iCount; ++i)
	{
		SAFE_DELETE(m_PlaneCont[i]);
	}
	m_PlaneCont.clear();
	m_iPlaneInstanceIDCounter = 0;
}

void PgLinkedPlaneGroup::ReleasePlane(int const iInstanceID)
{
	for(PlaneCont::iterator itor = m_PlaneCont.begin(); itor != m_PlaneCont.end(); ++itor)
	{
		if(iInstanceID == (*itor)->GetInstanceID())
		{
			SAFE_DELETE((*itor));
			m_PlaneCont.erase(itor);
			return;
		}
	}
}

void PgLinkedPlaneGroup::Destroy()
{
	ReleaseAllPlane();

	m_spTexturingProp = 0;
	m_spAlphaProp = 0;
	m_spVertexColorProp = 0;
	m_spZBufferProp = 0;
	m_spStencilProp = 0;
}
void PgLinkedPlaneGroup::Create(int const iInstanceID)
{
	m_iPlaneInstanceIDCounter = 0;
	m_iInstanceID = iInstanceID;

	m_fTextureCoordinateUpdateDelay = 0.05f;

	m_spTexturingProp = NiNew NiTexturingProperty();
	m_spAlphaProp = NiNew NiAlphaProperty();
	m_spVertexColorProp = NiNew NiVertexColorProperty();
	m_spZBufferProp = NiNew NiZBufferProperty();
	m_spStencilProp = NiNew NiStencilProperty();

	m_spAlphaProp->SetAlphaBlending(true);
	m_spAlphaProp->SetAlphaTesting(true);
	m_spAlphaProp->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
	m_spAlphaProp->SetDestBlendMode(NiAlphaProperty::ALPHA_ONE);
	m_spAlphaProp->SetTestMode(NiAlphaProperty::TEST_GREATER);

	m_spVertexColorProp->SetLightingMode(NiVertexColorProperty::LIGHTING_E_A_D);
	m_spVertexColorProp->SetSourceMode(NiVertexColorProperty::SOURCE_IGNORE);

	m_spZBufferProp->SetZBufferTest(true);
	m_spZBufferProp->SetZBufferWrite(true);

	m_spStencilProp->SetDrawMode(NiStencilProperty::DRAW_BOTH);
	
	m_spMaterialProp = NiNew NiMaterialProperty();
	m_spMaterialProp->SetAlpha(1.f);
	m_spMaterialProp->SetAmbientColor(NiColor(1.f, 1.f, 1.f));
	m_spMaterialProp->SetDiffuseColor(NiColor(1.f ,1.f, 1.f));
	m_spMaterialProp->SetEmittance(NiColor(1.f, 1.f, 1.f));
	m_spMaterialProp->SetShineness(10.f);
	m_spMaterialProp->SetSpecularColor(NiColor(1.f, 1.f, 1.f));

	m_spTexturingProp->SetBaseFilterMode(NiTexturingProperty::FILTER_TRILERP);
}

///////////////////////////////////////////////////////////////////////////////
//	class	PgLinkedPlaneGroupMan
///////////////////////////////////////////////////////////////////////////////

PgLinkedPlaneGroup*	PgLinkedPlaneGroupMan::CreateNewLinkedPlaneGroup()
{
	PgLinkedPlaneGroup* pkNewGroup = new PgLinkedPlaneGroup(m_iPlaneGroupInstanceIDCounter++);
	m_PlaneGroupCont.push_back(pkNewGroup);
	return pkNewGroup;
}

PgLinkedPlaneGroup*	PgLinkedPlaneGroupMan::GetLinkedPlaneGroup(int const iInstanceID)
{
	for(PlaneGroupCont::iterator itor = m_PlaneGroupCont.begin();itor != m_PlaneGroupCont.end(); ++itor)
	{
		if(iInstanceID == (*itor)->GetInstanceID())
		{
			return *itor;
		}
	}
	return	NULL;
}
void PgLinkedPlaneGroupMan::ReleaseLinkedPlaneGroup(int const iInstanceID)
{
	for(PlaneGroupCont::iterator itor = m_PlaneGroupCont.begin();itor != m_PlaneGroupCont.end(); ++itor)
	{
		if(iInstanceID == (*itor)->GetInstanceID())
		{
			SAFE_DELETE((*itor));
			m_PlaneGroupCont.erase(itor);
			return;
		}
	}
}
void PgLinkedPlaneGroupMan::ClearAll()
{
	int const iCount = m_PlaneGroupCont.size();

	for(int i = 0; i < iCount; ++i)
	{
		SAFE_DELETE(m_PlaneGroupCont[i]);
	}

	m_PlaneGroupCont.clear();
	m_iPlaneGroupInstanceIDCounter = 0;
}
void PgLinkedPlaneGroupMan::Destroy()
{
	ClearAll();
}

void PgLinkedPlaneGroupMan::Update(float fAccumTime, float fFrameTime)
{
	int const iCount = m_PlaneGroupCont.size();

	for(int i = 0; i < iCount; ++i)
	{
		m_PlaneGroupCont[i]->Update(fAccumTime,fFrameTime);
	}
}
void PgLinkedPlaneGroupMan::DrawImmediate(PgRenderer* pkRenderer)
{
	int const iCount = m_PlaneGroupCont.size();

	for(int i = 0; i < iCount; ++i)
	{
		m_PlaneGroupCont[i]->DrawImmediate(pkRenderer);
	}
}

void PgLinkedPlaneGroupMan::Create()
{
	m_iPlaneGroupInstanceIDCounter = 0;
}
