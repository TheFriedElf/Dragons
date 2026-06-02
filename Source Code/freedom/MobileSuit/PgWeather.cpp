#include "stdafx.h"
#include "PgWeather.h"
#include "PgNifMan.h"
#include "PgRenderer.h"
#include "PgCameraMan.h"
#include "PgWorld.h"

void	PgRainDropArea::SetPointInfo(int iTotalPointInfo,stRainDropPointInfo *paPointInfos,NiPoint3 const &bBoxMin,NiPoint3 const &bBoxMax)
{
	float	fMinAreaSize = 500;

	float	fXDiff = bBoxMax.x - bBoxMin.x;
	float	fYDiff = bBoxMax.y - bBoxMin.y;
	float	fZDiff = bBoxMax.z - bBoxMin.z;

	m_ptBBox[0] = bBoxMin;
	m_ptBBox[1] = bBoxMax;

	if(fXDiff<fMinAreaSize)
	{

		m_iTotalPointInfo = 0;
		NiPoint3	*pVertex;
		for(int i=0;i<iTotalPointInfo;i++)
		{

			pVertex = &(paPointInfos+i)->m_ptVertex;

			if(pVertex->x >=bBoxMin.x && pVertex->x<bBoxMax.x && 
				pVertex->y >=bBoxMin.y && pVertex->y <bBoxMax.y &&
				pVertex->z >=bBoxMin.z && pVertex->z <bBoxMax.z)
			{
				m_iTotalPointInfo++;
			}
			
		}
		if(m_iTotalPointInfo==0) return;

		m_paPointInfos = new stRainDropPointInfo[m_iTotalPointInfo];
		int iCount = 0;
		for(int i=0;i<iTotalPointInfo;i++)
		{
			pVertex = &(paPointInfos+i)->m_ptVertex;

			if(pVertex->x >=bBoxMin.x && pVertex->x<bBoxMax.x && 
				pVertex->y >=bBoxMin.y && pVertex->y <bBoxMax.y &&
				pVertex->z >=bBoxMin.z && pVertex->z <bBoxMax.z)
			{
				*(m_paPointInfos+iCount) = *(paPointInfos+i);
				iCount++;
			}
			
		}
		return;
	}

	m_paChild = new PgRainDropArea[8];
	(m_paChild+0)->SetPointInfo(iTotalPointInfo,paPointInfos,NiPoint3(bBoxMin.x,bBoxMin.y+fYDiff/2,bBoxMin.z),NiPoint3(bBoxMin.x+fXDiff/2,bBoxMax.y,bBoxMin.z+fZDiff/2));
	(m_paChild+1)->SetPointInfo(iTotalPointInfo,paPointInfos,NiPoint3(bBoxMin.x+fXDiff/2,bBoxMin.y+fYDiff/2,bBoxMin.z),NiPoint3(bBoxMax.x,bBoxMax.y,bBoxMin.z+fZDiff/2));
	(m_paChild+2)->SetPointInfo(iTotalPointInfo,paPointInfos,NiPoint3(bBoxMin.x+fXDiff/2,bBoxMin.y,bBoxMin.z),NiPoint3(bBoxMax.x,bBoxMin.y+fYDiff/2,bBoxMin.z+fZDiff/2));
	(m_paChild+3)->SetPointInfo(iTotalPointInfo,paPointInfos,bBoxMin,NiPoint3(bBoxMin.x+fXDiff/2,bBoxMin.y+fYDiff/2,bBoxMin.z+fZDiff/2));

	(m_paChild+4)->SetPointInfo(iTotalPointInfo,paPointInfos,NiPoint3(bBoxMin.x,bBoxMin.y+fYDiff/2,bBoxMin.z+fZDiff/2),NiPoint3(bBoxMin.x+fXDiff/2,bBoxMax.y,bBoxMax.z));
	(m_paChild+5)->SetPointInfo(iTotalPointInfo,paPointInfos,NiPoint3(bBoxMin.x+fXDiff/2,bBoxMin.y+fYDiff/2,bBoxMin.z+fZDiff/2),bBoxMax);
	(m_paChild+6)->SetPointInfo(iTotalPointInfo,paPointInfos,NiPoint3(bBoxMin.x+fXDiff/2,bBoxMin.y,bBoxMin.z+fZDiff/2),NiPoint3(bBoxMax.x,bBoxMin.y+fYDiff/2,bBoxMax.z));
	(m_paChild+7)->SetPointInfo(iTotalPointInfo,paPointInfos,NiPoint3(bBoxMin.x,bBoxMin.y,bBoxMin.z+fZDiff/2),NiPoint3(bBoxMin.x+fXDiff/2,bBoxMin.y+fYDiff/2,bBoxMax.z));

}
void	PgRainDropArea::Draw(PgRenderer *pkRenderer,PgWorld *pkWorld,PgRainDropMan *pMan)
{
	//	카메라 뷰 프러스텀 체크를 하자.
	NiCameraPtr spCamera = pkWorld->m_kCameraMan.GetCamera();
	NiFrustum kFrustum = spCamera->GetViewFrustum();
	float fFarSaved = kFrustum.m_fFar;
	kFrustum.m_fFar = 1000.0f;
	spCamera->SetViewFrustum(kFrustum);

	g_kFrustum = kFrustum;

	NiFrustumPlanes kFP(*spCamera);

	kFrustum.m_fFar = fFarSaved;
	spCamera->SetViewFrustum(kFrustum);

	g_kFrustum = kFrustum;

	NiPlane	kPlane = kFP.GetPlane(NiFrustumPlanes::LEFT_PLANE);
	if(kPlane.WhichSide(m_ptBBox[0])==NiPlane::NEGATIVE_SIDE &&kPlane.WhichSide(m_ptBBox[1])==NiPlane::NEGATIVE_SIDE) return;
	kPlane = kFP.GetPlane(NiFrustumPlanes::RIGHT_PLANE);
	if(kPlane.WhichSide(m_ptBBox[0])==NiPlane::NEGATIVE_SIDE &&kPlane.WhichSide(m_ptBBox[1])==NiPlane::NEGATIVE_SIDE) return;
	kPlane = kFP.GetPlane(NiFrustumPlanes::NEAR_PLANE);
	if(kPlane.WhichSide(m_ptBBox[0])==NiPlane::NEGATIVE_SIDE &&kPlane.WhichSide(m_ptBBox[1])==NiPlane::NEGATIVE_SIDE) return;
	kPlane = kFP.GetPlane(NiFrustumPlanes::FAR_PLANE);
	if(kPlane.WhichSide(m_ptBBox[0])==NiPlane::NEGATIVE_SIDE &&kPlane.WhichSide(m_ptBBox[1])==NiPlane::NEGATIVE_SIDE) return;
	kPlane = kFP.GetPlane(NiFrustumPlanes::TOP_PLANE);
	if(kPlane.WhichSide(m_ptBBox[0])==NiPlane::NEGATIVE_SIDE &&kPlane.WhichSide(m_ptBBox[1])==NiPlane::NEGATIVE_SIDE) return;
	kPlane = kFP.GetPlane(NiFrustumPlanes::BOTTOM_PLANE);
	if(kPlane.WhichSide(m_ptBBox[0])==NiPlane::NEGATIVE_SIDE &&kPlane.WhichSide(m_ptBBox[1])==NiPlane::NEGATIVE_SIDE) return;


	for(int i=0;i<m_iTotalPointInfo;i++)
	{
		if(BM::Rand_Range(1))
		{
			pMan->Add_Rain_Drop((m_paPointInfos+i));
		}
	}

	if(m_paChild)
	{
		for(int i=0;i<8;i++)
		{
			(m_paChild+i)->Draw(pkRenderer,pkWorld,pMan);
		}
	}
	
}
void	PgRainDropArea::Init()
{
	m_iTotalPointInfo = 0;
	m_paPointInfos = 0;
	m_paChild = 0;
}

void	PgRainDropArea::Destroy()
{
	SAFE_DELETE_ARRAY(m_paPointInfos);
	SAFE_DELETE_ARRAY(m_paChild);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgRainDropMan
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void	PgRainDropMan::LoadFromFile(std::string const &kPath)
{
	FILE *fp = fopen(kPath.c_str(),"rb");
	if(!fp) return;

	int	iTotal;
	fread(&iTotal,sizeof(int),1,fp);


	if(iTotal>0)
	{
		stRainDropPointInfo	*pPointInfo = new stRainDropPointInfo[iTotal];

		NiPoint3	bBoxMin,bBoxMax;
		bBoxMin.x=bBoxMin.y=bBoxMin.z=99999;
		bBoxMax.x=bBoxMax.y=bBoxMax.z=-99999;

		for(int i=0;i<iTotal;i++)
		{
			fread(&(pPointInfo+i)->m_ptVertex.x,sizeof(float),1,fp);
			fread(&(pPointInfo+i)->m_ptVertex.y,sizeof(float),1,fp);
			fread(&(pPointInfo+i)->m_ptVertex.z,sizeof(float),1,fp);

			fread(&(pPointInfo+i)->m_ptNormal.x,sizeof(float),1,fp);
			fread(&(pPointInfo+i)->m_ptNormal.y,sizeof(float),1,fp);
			fread(&(pPointInfo+i)->m_ptNormal.z,sizeof(float),1,fp);

			if(bBoxMin.x>(pPointInfo+i)->m_ptVertex.x) bBoxMin.x = (pPointInfo+i)->m_ptVertex.x;
			if(bBoxMin.y>(pPointInfo+i)->m_ptVertex.y) bBoxMin.y = (pPointInfo+i)->m_ptVertex.y;
			if(bBoxMin.z>(pPointInfo+i)->m_ptVertex.z) bBoxMin.z = (pPointInfo+i)->m_ptVertex.z;

			if(bBoxMax.x<(pPointInfo+i)->m_ptVertex.x) bBoxMax.x = (pPointInfo+i)->m_ptVertex.x;
			if(bBoxMax.y<(pPointInfo+i)->m_ptVertex.y) bBoxMax.y = (pPointInfo+i)->m_ptVertex.y;
			if(bBoxMax.z<(pPointInfo+i)->m_ptVertex.z) bBoxMax.z = (pPointInfo+i)->m_ptVertex.z;
		}

		m_pTopNode = new PgRainDropArea();

		float	fLengthX = fabs(bBoxMax.x) + fabs(bBoxMin.x);
		float	fLengthY =   fabs(bBoxMax.y)+  fabs(bBoxMin.y);
		float	fLengthZ =   fabs(bBoxMax.z )+  fabs(bBoxMin.z);

		float	fMaxLength = fLengthX;
		if(fMaxLength<fLengthY) fMaxLength = fLengthY;
		if(fMaxLength<fLengthZ) fMaxLength = fLengthZ;

		bBoxMin.x = -fMaxLength/2;
		bBoxMax.x = fMaxLength/2;

		bBoxMin.y = -fMaxLength/2;
		bBoxMax.y = fMaxLength/2;

		bBoxMin.z = -fMaxLength/2;
		bBoxMax.z = fMaxLength/2;


		m_pTopNode->SetPointInfo(iTotal,pPointInfo,bBoxMin,bBoxMax);

		SAFE_DELETE_ARRAY(pPointInfo);
	}

	fclose(fp);
}
void	PgRainDropMan::Draw(PgRenderer *pkRenderer,PgWorld *pkWorld)
{

	int	iElapsedTime;
	unsigned	long	ulTime = BM::GetTime32();
	stNode	*pNode;
	NiGeometry	*pGeom;
	float	fQuadSize = 2.0f;
	NiPoint3	vQuad[4];

	PgCameraMan *pkCameraMan = &pkWorld->m_kCameraMan;
	NiCameraPtr	spCamera = pkCameraMan->GetCamera();

	NiPoint3	vCamRight = spCamera->GetWorldRightVector();
	NiPoint3	vCamUp = spCamera->GetWorldUpVector();

	vQuad[0] = vCamRight*fQuadSize+vCamUp*fQuadSize*2;
	vQuad[1] = vCamRight*fQuadSize;
	vQuad[2] = -vCamRight*fQuadSize+vCamUp*fQuadSize*2;
	vQuad[3] = -vCamRight*fQuadSize;

	float	fAlpha;
	pGeom = (NiGeometry*)m_spNiNode->GetObjectByName("Plane01");

	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end();)
	{

		pNode = *itor;

		iElapsedTime = ulTime - pNode->m_ulStartTime;
		if(iElapsedTime>300)
		{
			SAFE_DELETE(pNode);
			itor = m_NodeList.erase(itor);
			continue;
		}

		fAlpha = 1-iElapsedTime/300.0f;
		if(fAlpha>1) fAlpha = 1;
		if(fAlpha<0) fAlpha = 0;

		NiPoint3	*pVert = pGeom->GetVertices();
	
		memcpy(pVert,vQuad,sizeof(NiPoint3)*4);

		pGeom->GetPropertyState()->GetMaterial()->SetAlpha(fAlpha);
		pGeom->GetModelData()->MarkAsChanged(NiGeometryData::VERTEX_MASK);
		pGeom->SetRotate(NiQuaternion::IDENTITY);

		m_spNiNode->SetTranslate(pNode->m_pPoint->m_ptVertex);
		m_spNiNode->Update(0);

		pGeom->RenderImmediate(pkRenderer->GetRenderer());

		itor++;
	}

	m_iFrameCount++;
	if(m_iFrameCount%5 != 0) return;

	if(m_pTopNode)
		m_pTopNode->Draw(pkRenderer,pkWorld,this);
}
void	PgRainDropMan::Init()
{
	m_iFrameCount = 0;
	m_pTopNode = 0;
//	m_spModel = 0;

	m_spTex = g_kNifMan.GetTexture("../Data/5_Effect/9_Tex/RainDrop.dds");
	m_spNiNode = g_kNifMan.GetNif("../Data/6_UI/Quad.nif");

	NiGeometry	*pGeom = (NiGeometry*)m_spNiNode->GetObjectByName("Plane01");
	pGeom->GetPropertyState()->GetTexturing()->SetBaseTexture(m_spTex);
	pGeom->GetPropertyState()->GetAlpha()->SetAlphaTesting(true);
	pGeom->GetPropertyState()->GetAlpha()->SetTestMode(NiAlphaProperty::TEST_GREATER);
	pGeom->GetPropertyState()->GetAlpha()->SetTestRef(10);
	pGeom->SetConsistency(NiGeometryData::VOLATILE);

}
void	PgRainDropMan::Destroy()
{
	SAFE_DELETE(m_pTopNode);

	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end();)
	{
		SAFE_DELETE(*itor);
	}
	m_NodeList.clear();

	m_spTex = 0;
	m_spNiNode = 0;

}
void	PgRainDropMan::Add_Rain_Drop(stRainDropPointInfo *pDropPoint)
{
	stNode	*pNewNode = new stNode();
	pNewNode->m_ulStartTime = BM::GetTime32();
	pNewNode->m_pPoint = pDropPoint;

	m_NodeList.push_back(pNewNode);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgRain
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void	PgRain::Update(float fAccumTime, float fFrameTime)
{
}
void	PgRain::DrawRainDrops(PgRenderer *pkRenderer, PgWorld *pkWorld,float fFrameTime)
{
	if(m_pRainDropMan==0)
	{
		m_pRainDropMan = new PgRainDropMan();
		m_pRainDropMan->LoadFromFile("../RainDropData.Dat");
	}
	if(m_pRainDropMan)
		m_pRainDropMan->Draw(pkRenderer,pkWorld);
}

void	PgRain::DrawImmediate(PgRenderer *pkRenderer, PgWorld *pkWorld,float fFrameTime)
{
	if(m_spRainNif == 0)
	{
		LoadResource();
	}

	PgCameraMan *pkCameraMan = &pkWorld->m_kCameraMan;

	NiCameraPtr	spCamera = pkCameraMan->GetCamera();
	
	m_fFrameTime+=fFrameTime;

	NiPoint3	vCamVel = pkCameraMan->GetVelocity();
	NiPoint3	vCamVelNormalized = vCamVel;
	NiPoint3::UnitizeVector(vCamVelNormalized);

	NiPoint3	vCamUp = spCamera->GetWorldUpVector();
	NiPoint3	vAxis = vCamVelNormalized.Cross(vCamUp);
	NiPoint3	vCamDir = spCamera->GetWorldDirection();

	NiPoint3	vCamRight = spCamera->GetWorldRightVector();
	float	fRate = fabs(vCamRight.Dot(vCamVelNormalized));

	float	fDot = vAxis.Dot(vCamDir);

	float	fRotAngle = (float)(vCamVel.SqrLength()*3.141592/180.0f*0.05*fRate);
	if(fDot>0)
		m_fRainRotate += fRotAngle;
	else
		m_fRainRotate -= fRotAngle;

	NiQuaternion		quat(-m_fRainRotate,NiPoint3(0,0,1));

	NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("g_fRain_FrameTime"),sizeof(float), &m_fFrameTime);

	NiGeometry	*pPlane;

	m_spRainNif->SetRotate(quat);
	NiPoint3	vPos = pkCameraMan->GetCamera()->GetWorldLocation();
	vPos.z = 0;
	m_spRainNif->SetTranslate(vPos);
	m_spRainNif->Update(0);

	pPlane = (NiGeometry*)m_spRainNif->GetObjectByName("Plane02");
	pPlane->RenderImmediate(pkRenderer->GetRenderer());
	pPlane = (NiGeometry*)m_spRainNif->GetObjectByName("Plane03");
	pPlane->RenderImmediate(pkRenderer->GetRenderer());
	pPlane = (NiGeometry*)m_spRainNif->GetObjectByName("Plane04");
	pPlane->RenderImmediate(pkRenderer->GetRenderer());
	pPlane = (NiGeometry*)m_spRainNif->GetObjectByName("Plane05");
	pPlane->RenderImmediate(pkRenderer->GetRenderer());


}

void	PgRain::Init()
{
	m_spRainNif = 0;
	m_fRainRotate= 0;
	m_fFrameTime = 0;
	m_pRainDropMan = 0;
}

void	PgRain::Destroy()
{
	SAFE_DELETE(m_pRainDropMan);
	m_spRainNif = 0;
}

void	PgRain::LoadResource()
{
	NiShaderPtr spRainShader = NiShaderFactory::GetInstance()->RetrieveShader("RainShader", NiShader::DEFAULT_IMPLEMENTATION, true);
	NiSingleShaderMaterial* pkMaterial = NiSingleShaderMaterial::Create(spRainShader);

	NiTexturingProperty *pTexturing;
	NiSourceTexture	*pRainTex = g_kNifMan.GetTexture("../Data/5_Effect/9_Tex/Rain01.dds");
	NiSourceTexture	*pNoiseTex = g_kNifMan.GetTexture("../Data/5_Effect/9_Tex/Noise.dds");
	m_spRainNif = g_kNifMan.GetNif("../Data/5_Effect/0_Common/Rain.nif");

	NiGeometry	*pPlane = (NiGeometry*)m_spRainNif->GetObjectByName("Plane02");
	pPlane->SetConsistency(NiGeometryData::VOLATILE);
	pTexturing = pPlane->GetPropertyState()->GetTexturing();
	pTexturing->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(pRainTex,0));
	pTexturing->SetShaderMap(1,NiNew NiTexturingProperty::ShaderMap(pNoiseTex,0));
	pPlane->ApplyAndSetActiveMaterial(pkMaterial);

	pPlane = (NiGeometry*)m_spRainNif->GetObjectByName("Plane03");
	pPlane->SetConsistency(NiGeometryData::VOLATILE);
	pTexturing = pPlane->GetPropertyState()->GetTexturing();
	pTexturing->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(pRainTex,0));
	pTexturing->SetShaderMap(1,NiNew NiTexturingProperty::ShaderMap(pNoiseTex,0));
	pPlane->ApplyAndSetActiveMaterial(pkMaterial);

	pPlane = (NiGeometry*)m_spRainNif->GetObjectByName("Plane04");
	pPlane->SetConsistency(NiGeometryData::VOLATILE);
	pTexturing = pPlane->GetPropertyState()->GetTexturing();
	pTexturing->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(pRainTex,0));
	pTexturing->SetShaderMap(1,NiNew NiTexturingProperty::ShaderMap(pNoiseTex,0));
	pPlane->ApplyAndSetActiveMaterial(pkMaterial);

	pPlane = (NiGeometry*)m_spRainNif->GetObjectByName("Plane05");
	pPlane->SetConsistency(NiGeometryData::VOLATILE);
	pTexturing = pPlane->GetPropertyState()->GetTexturing();
	pTexturing->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(pRainTex,0));
	pTexturing->SetShaderMap(1,NiNew NiTexturingProperty::ShaderMap(pNoiseTex,0));
	pPlane->ApplyAndSetActiveMaterial(pkMaterial);


	
}
