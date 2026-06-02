#include "stdafx.h"
#include "PgNifMan.h"
#include "PgTrail.h"
#include "PgRenderer.h"
#include "PgMath.h"
#include <math.h>
#include "PgMobileSuit.H"
#include "PgWorld.H"
#include "PgCameraMan.H"
#include "FreedomPool.H"


void	PgTrailNode::stTrailRibbon::ReleaseTrailRibbonLineVector(TrailRibbonLineVector &kLineVector)
{
	for(size_t i=0;i<kLineVector.size();i++)
	{
		stTrailRibbonLine	*pkLine = kLineVector[i];
		g_kTrailRibbonLinePool.Delete(pkLine);
	}
	kLineVector.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgTrailNode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgTrailNode::PgTrailNode()
:m_spSrcObj(0),
m_spGeom(0),
m_bRemoveReserved(false),
m_bFinished(false),
m_fTotalExistTime(0.0f),
m_fBrightTime(0.0f),
m_fAccumTimeAdjust(0.0f),
m_fTrailWidth(0.0f),
m_fLastUpdateTime(0.0f)
{
};
void	PgTrailNode::Init(NiAVObject* pkSrcObj,std::string const &kTexPath,float fTotalExistTime,float fBrightTime)
{
	PG_ASSERT_LOG(pkSrcObj);

	m_spSrcObj = pkSrcObj;
	m_spGeom = 0;
	m_bRemoveReserved = false;
	m_fAccumTimeAdjust = 0;
	m_bFinished = false;
	m_fTrailWidth = 0;

	m_fTotalExistTime = fTotalExistTime;
	m_fBrightTime = fBrightTime;
	m_kTexPath = kTexPath;

	InitializeTrailRibbons();

	CreateGeom(TRAIL2_INITIAL_VERTEX_COUNT);
}
void	PgTrailNode::Init(NiAVObject* pkSrcObj,std::string const &kCenterObjName,std::string const &kTexPath,float fTotalExistTime,float fBrightTime,float fTrailWidth)
{
	PG_ASSERT_LOG(pkSrcObj);

	m_spSrcObj = pkSrcObj;
	m_spGeom = 0;
	m_bRemoveReserved = false;
	m_fAccumTimeAdjust = 0;
	m_bFinished = false;

	m_fTotalExistTime = fTotalExistTime;
	m_fBrightTime = fBrightTime;
	m_kTexPath = kTexPath;
	m_fTrailWidth = fTrailWidth;

	InitializeTrailRibbons(kCenterObjName);

	CreateGeom(TRAIL2_INITIAL_VERTEX_COUNT);
}

void PgTrailNode::UpdateDownwardPass(float fTime, bool bUpdateControllers)
{
	//fTime += m_fAccumTimeAdjust;

	NiNode::UpdateDownwardPass(fTime,bUpdateControllers);

	if(m_bFinished) return;

	if(!UpdateRibbons(fTime))
	{
		if(m_bRemoveReserved)
		{
			m_bFinished = true;
		}
	}
}
bool	PgTrailNode::UpdateRibbons(float	fTime)
{
	float	fElapsedTime = fTime - m_fLastUpdateTime;
	if(fElapsedTime < MIN_TRAIL_UPDATE_DELAY)
	{
		return	true;
	}
	m_fLastUpdateTime = fTime;
	
	int	iTotalVertex = 0;
	int	iTotalTriangles = 0;
	for(int i=0;i<static_cast<int>(m_vTrailRibbons.size());i++)
	{
		int iVertex =UpdateRibbon(fTime,m_vTrailRibbons[i]);

		iTotalVertex+=iVertex;
		iTotalTriangles+=iVertex-2;
	}

	if(iTotalVertex>0)
	{
		if(iTotalVertex>m_spGeom->GetModelData()->GetVertexCount())	//	버택스 갯수가 초과되었다면,
		{
			CreateGeom(iTotalVertex+100);	//	갯수를 늘린다.
		}

		UpdateGeom(iTotalTriangles,fTime);	
		return	true;
	}	
	else
	{
		NiTriShapeData	*pkShapeData = (NiTriShapeData*)m_spGeom->GetModelData();
		pkShapeData->SetActiveTriangleCount(0);
	}

	return	false;
}
void	PgTrailNode::UpdateGeom(int iTotalTriangles,float fTime)
{
	if(iTotalTriangles ==0) return;

	//	m_spGeom 의 버택스,인덱스,컬러,텍스쳐좌표등을 재설정한다.
	stTrailRibbon	*pkRibbon;
	NiTriShapeData	*pkShapeData = (NiTriShapeData*)m_spGeom->GetModelData();
	NiPoint3	*pkVertex = pkShapeData->GetVertices();
	NiColorA	*pkColor = pkShapeData->GetColors();
	NiPoint2	*pkTexture = pkShapeData->GetTextures();
	unsigned	short	*pusIndex = NiAlloc(unsigned short, iTotalTriangles * 3);
	unsigned	short	*pusIndex2 = pusIndex;
	int	iVertexCount = 0;
	float	fAlpha;
	NiPoint3	kMin(99999,99999,99999),kMax(-99999,-99999,-99999);
	for(int i=0;i<static_cast<int>(m_vTrailRibbons.size());i++)
	{
		pkRibbon = m_vTrailRibbons[i];

		int	iTotalLine = pkRibbon->m_vDrawLines.size();
		if(iTotalLine == 0) continue;

		for(int j=0;j<iTotalLine;j++)
		{
			*(pkVertex++) = pkRibbon->m_vDrawLines[j]->m_vEnd;
			*(pkVertex++) = pkRibbon->m_vDrawLines[j]->m_vStart;


			if((pkVertex-2)->x<kMin.x) kMin.x = (pkVertex-2)->x;
			if((pkVertex-2)->y<kMin.y) kMin.y = (pkVertex-2)->y;
			if((pkVertex-2)->z<kMin.z) kMin.z = (pkVertex-2)->z;

			if((pkVertex-1)->x<kMin.x) kMin.x = (pkVertex-1)->x;
			if((pkVertex-1)->y<kMin.y) kMin.y = (pkVertex-1)->y;
			if((pkVertex-1)->z<kMin.z) kMin.z = (pkVertex-1)->z;

			if((pkVertex-2)->x>kMax.x) kMax.x = (pkVertex-2)->x;
			if((pkVertex-2)->y>kMax.y) kMax.y = (pkVertex-2)->y;
			if((pkVertex-2)->z>kMax.z) kMax.z = (pkVertex-2)->z;

			if((pkVertex-1)->x>kMax.x) kMax.x = (pkVertex-1)->x;
			if((pkVertex-1)->y>kMax.y) kMax.y = (pkVertex-1)->y;
			if((pkVertex-1)->z>kMax.z) kMax.z = (pkVertex-1)->z;

			float	fFinishTime = pkRibbon->m_vDrawLines[j]->m_fCreateTime + m_fTotalExistTime;
			float	fBrightFinishTime = pkRibbon->m_vDrawLines[j]->m_fCreateTime+m_fBrightTime;

			if(fTime<fBrightFinishTime)
				fAlpha = 1;
			else
			{
				fAlpha = 1-(fTime-fBrightFinishTime)/(fFinishTime-fBrightFinishTime);
			}

			if(fAlpha<0) fAlpha = 0;
			if(fAlpha>1) fAlpha = 1;

			pkTexture->x = (j/((float)iTotalLine-1));	pkTexture->y = 1;	pkTexture++;
			pkTexture->x = (j/((float)iTotalLine-1));	pkTexture->y = 0;	pkTexture++;

			*(pkColor++) = NiColorA(1,1,1,fAlpha);
			*(pkColor++) = NiColorA(1,1,1,fAlpha);

		}
		int nTriangles = (iTotalLine*2)-2;
		for(int j=0;j<nTriangles;j++)	
		{	
			*(pusIndex2++) = iVertexCount+j;	
			*(pusIndex2++) = iVertexCount+j+1;	
			*(pusIndex2++) = iVertexCount+j+2;	
		}

		iVertexCount+=iTotalLine*2;
	}

	pkShapeData->Replace(iTotalTriangles,pusIndex);
	pkShapeData->SetActiveTriangleCount(iTotalTriangles);
	pkShapeData->MarkAsChanged(NiGeometryData::VERTEX_MASK | NiGeometryData::COLOR_MASK);

	NiBound	kBound;
	NiPoint3	kCenter = kMin+(kMax-kMin)/2.0f;
	kBound.SetCenterAndRadius(kCenter,(kMax-kCenter).Length());
	pkShapeData->SetBound(kBound);
	m_kWorldBound = kBound;
	//PgOutputPrint2(" -------- %f, %f, %f, %f --------- \n", kBound.GetCenter().x, kBound.GetCenter().y, kBound.GetCenter().z, kBound.GetRadius());
}
int	PgTrailNode::UpdateRibbon(float	fTime,stTrailRibbon	*pkRibbon)
{

	if(m_bRemoveReserved == false)
	{
		//	최신 컨트롤 라인 추가

		if(pkRibbon->m_spStartDummy == pkRibbon->m_spEndDummy)
		{
			const	float	fTrailWidth = m_fTrailWidth;

			const	NiPoint3	&kCenterPos = pkRibbon->m_spStartDummy->GetWorldTranslate();

			NiPoint3	kNewPoint[2];
			NiPoint3	kVec = kCenterPos -pkRibbon->m_kPrevCenterPoint;
			if(pkRibbon->m_kPrevCenterPoint == NiPoint3(-1,-1,-1) || kVec == NiPoint3::ZERO)
			{
				kNewPoint[0] = NiPoint3(kCenterPos.x,kCenterPos.y,kCenterPos.z+fTrailWidth/2);
				kNewPoint[1] = NiPoint3(kCenterPos.x,kCenterPos.y,kCenterPos.z-fTrailWidth/2);
			}
			else
			{
				kVec.Unitize();
				
				if(g_pkWorld)
				{
					NiCameraPtr spCamera = g_pkWorld->GetCameraMan()->GetCamera();
					if(spCamera)
					{
						const	NiPoint3&	kCamDir = spCamera->GetWorldDirection();

						NiPoint3	kLineDir = kVec.Cross(kCamDir);
						kLineDir.Unitize();
				
						kNewPoint[0] = kCenterPos + kLineDir*fTrailWidth/2;
						kNewPoint[1] = kCenterPos - kLineDir*fTrailWidth/2;

					}
				}
			}

			pkRibbon->m_kPrevCenterPoint = kCenterPos;
			stTrailRibbonLine *pkLine = g_kTrailRibbonLinePool.New();
			pkLine->Set(kNewPoint[0],kNewPoint[1],fTime);
			pkRibbon->m_vControlLines.push_back(pkLine);

		}
		else
		{
			NiPoint3	vStart = pkRibbon->m_spStartDummy->GetWorldTranslate();
			NiPoint3	vEnd	=	pkRibbon->m_spEndDummy->GetWorldTranslate();
			stTrailRibbonLine *pkLine = g_kTrailRibbonLinePool.New();
			pkLine->Set(vStart,vEnd,fTime);
			pkRibbon->m_vControlLines.push_back(pkLine);
		}
	}

	int	iTotalControlLine = pkRibbon->m_vControlLines.size();

	//	시간이 다 된 컨트롤 라인을 지운다.
	float	fElapsedTime = 0.0f;
	bool	bOverTime = true;
	for(int i=0;i<iTotalControlLine;i++)
	{
		fElapsedTime = fTime - pkRibbon->m_vControlLines[i]->m_fCreateTime;
		if(fElapsedTime<m_fTotalExistTime)
		{
			bOverTime = false;

			int iCount = 0;
			for(TrailRibbonLineVector::iterator itor = pkRibbon->m_vControlLines.begin(); itor != pkRibbon->m_vControlLines.end();)
			{
				if(iCount >= i-1) break;

				stTrailRibbonLine	*pkLine = *itor;
				g_kTrailRibbonLinePool.Delete(pkLine);
				itor = pkRibbon->m_vControlLines.erase(itor);
				iCount++;
				
			}
			break;
		}
	}

	if(bOverTime)
		stTrailRibbon::ReleaseTrailRibbonLineVector(pkRibbon->m_vControlLines);

	iTotalControlLine = pkRibbon->m_vControlLines.size();

	//	남은 컨트롤 라인 갯수가 최소한 2개 이상이어야 리본을 그릴수가 있다.
	if(iTotalControlLine<2) return 0;

	stTrailRibbon::ReleaseTrailRibbonLineVector(pkRibbon->m_vDrawLines);

	//	컨트롤 라인들을 보간하여 곡선 라인을 생성하자.
	stTrailRibbonLine	kControlLine[4];
	float	fLength,fLength2;
	int	iDivide;
	for(int i=1;i<iTotalControlLine;i++)
	{
		if(i==1)
		{
			kControlLine[0] = *pkRibbon->m_vControlLines[0];
			kControlLine[0].Add(pkRibbon->m_vControlLines[0]);
			kControlLine[0].Subtract(pkRibbon->m_vControlLines[1]);
		}
		else
			kControlLine[0] = *pkRibbon->m_vControlLines[i-2];

		kControlLine[1] = *pkRibbon->m_vControlLines[i-1];
		kControlLine[2] = *pkRibbon->m_vControlLines[i];

		if(i+1>=iTotalControlLine)
		{
			kControlLine[3] = *pkRibbon->m_vControlLines[i];
			kControlLine[3].Add(pkRibbon->m_vControlLines[i]);
			kControlLine[3].Subtract(pkRibbon->m_vControlLines[i-1]);
		}
		else
			kControlLine[3] = *pkRibbon->m_vControlLines[i+1];


		fLength = (kControlLine[2].m_vStart - kControlLine[1].m_vStart).Length();
		fLength2 = (kControlLine[2].m_vEnd - kControlLine[1].m_vEnd).Length();

		if(fLength2>fLength) fLength = fLength2;

		//	거리차에 따라서 분할 갯수를 결정하자.
		iDivide = (int)(fLength/2.0f);

		if(iDivide==0) iDivide=1;
		if(iDivide>10) iDivide = 10;

		NiPoint3	kNewStart,kNewEnd;
		float	fMu,fCreateTime;
		for(int j=0;j<iDivide;j++)
		{
			fMu = (j/((float)iDivide));
			Catmull_Rom_Splines(kControlLine[0].m_vStart,kControlLine[1].m_vStart,kControlLine[2].m_vStart,kControlLine[3].m_vStart,fMu,kNewStart);
			Catmull_Rom_Splines(kControlLine[0].m_vEnd,kControlLine[1].m_vEnd,kControlLine[2].m_vEnd,kControlLine[3].m_vEnd,fMu,kNewEnd);

			//Bezier4(kControlLine[0].m_vStart,kControlLine[1].m_vStart,kControlLine[2].m_vStart,kControlLine[3].m_vStart,fMu,kNewStart);
			//Bezier4(kControlLine[0].m_vEnd,kControlLine[1].m_vEnd,kControlLine[2].m_vEnd,kControlLine[3].m_vEnd,fMu,kNewEnd);

			fCreateTime = kControlLine[1].m_fCreateTime+(kControlLine[2].m_fCreateTime-kControlLine[1].m_fCreateTime)*fMu;

			stTrailRibbonLine *pkLine = g_kTrailRibbonLinePool.New();
			pkLine->Set(kNewStart,kNewEnd,fCreateTime);
			pkRibbon->m_vDrawLines.push_back(pkLine);
		}
	}

	//	최신 라인도 추가해주자.
	stTrailRibbonLine *pkLine = g_kTrailRibbonLinePool.New();
	pkLine->Set(
		pkRibbon->m_vControlLines[iTotalControlLine-1]->m_vStart
		,pkRibbon->m_vControlLines[iTotalControlLine-1]->m_vEnd
		,pkRibbon->m_vControlLines[iTotalControlLine-1]->m_fCreateTime);

	pkRibbon->m_vDrawLines.push_back(pkLine);


	return	pkRibbon->m_vDrawLines.size()*2;	//	리본을 그리는데 필요한 버텍스 갯수를 리턴한다.
}
void	PgTrailNode::InitializeTrailRibbons(std::string const &kCenterObjName)
{
	PG_ASSERT_LOG(m_spSrcObj);

	stTrailRibbon	*pkRibbon = NULL;

	NiAVObjectPtr	spCenterObj = m_spSrcObj->GetObjectByName(kCenterObjName.c_str());
	if(spCenterObj == 0)
	{
		spCenterObj = m_spSrcObj;
	}

	pkRibbon = new stTrailRibbon();
	pkRibbon->m_spStartDummy = spCenterObj;
	pkRibbon->m_spEndDummy = spCenterObj;
	m_vTrailRibbons.push_back(pkRibbon);

}
void	PgTrailNode::InitializeTrailRibbons()
{
	PG_ASSERT_LOG(m_spSrcObj);

	stTrailRibbon	*pkRibbon = NULL;

	if(m_spSrcObj->GetObjectByName("Bip01 Head") && m_spSrcObj->GetObjectByName("char_root"))
	{
		pkRibbon = new stTrailRibbon();
		pkRibbon->m_spStartDummy = m_spSrcObj->GetObjectByName("char_root");
		pkRibbon->m_spEndDummy = m_spSrcObj->GetObjectByName("Bip01 Head");
		m_vTrailRibbons.push_back(pkRibbon);
	}
	else
	{
		if(m_spSrcObj->GetObjectByName("Trail_Head") && m_spSrcObj->GetObjectByName("Trail_End"))
		{
			pkRibbon = new stTrailRibbon();
			pkRibbon->m_spStartDummy = m_spSrcObj->GetObjectByName("Trail_Head");
			pkRibbon->m_spEndDummy = m_spSrcObj->GetObjectByName("Trail_End");
			m_vTrailRibbons.push_back(pkRibbon);
		}
		if(m_spSrcObj->GetObjectByName("Bip01 Head") && m_spSrcObj->GetObjectByName("Trail_Tail"))
		{
			pkRibbon = new stTrailRibbon();
			pkRibbon->m_spStartDummy = m_spSrcObj->GetObjectByName("Trail_Head");
			pkRibbon->m_spEndDummy = m_spSrcObj->GetObjectByName("Trail_Tail");
			m_vTrailRibbons.push_back(pkRibbon);
		}
	}

}
void	PgTrailNode::DestroyTrailRibbons()
{
	for(int i=0;i<static_cast<int>(m_vTrailRibbons.size());i++)
	{
		SAFE_DELETE(m_vTrailRibbons[i]);
	}
	m_vTrailRibbons.clear();
}
void	PgTrailNode::Create(NiAVObjectPtr spSrcObj,std::string kTexPath,float fTotalExistTime,float fBrightTime)
{
	PG_ASSERT_LOG(spSrcObj);

	m_spSrcObj = spSrcObj;
	m_spGeom = 0;
	m_bRemoveReserved = false;
	m_fAccumTimeAdjust = 0;
	m_bFinished = false;

	m_fTotalExistTime = fTotalExistTime;
	m_fBrightTime = fBrightTime;
	m_kTexPath = kTexPath;

	InitializeTrailRibbons();

	CreateGeom(TRAIL2_INITIAL_VERTEX_COUNT);
}
void	PgTrailNode::CreateGeom(int iVertexCount)
{
	if(m_spGeom)
	{
		DetachChild(m_spGeom);
		m_spGeom = 0;
	}

	int	nTriangles = iVertexCount-2;
	NiPoint3*pkVertex = NiNew NiPoint3[iVertexCount];
	NiColorA* pkColor = NiNew NiColorA[iVertexCount];
	NiPoint2* pkTexture = NiNew NiPoint2[iVertexCount];
	unsigned	short	*pusTriangleList = NiAlloc(unsigned short, nTriangles * 3);
	for(int i=0;i<nTriangles;++i)	
	{	
		*(pusTriangleList+i*3+0) = i;	
		*(pusTriangleList+i*3+1) = i+1;	
		*(pusTriangleList+i*3+2) = i+2;	
	}

	NiTriShapeDynamicData	*pkModelData = NiNew NiTriShapeDynamicData(
		iVertexCount,pkVertex,NULL,pkColor,pkTexture,1,NiGeometryData::NBT_METHOD_NONE,
		nTriangles,pusTriangleList,iVertexCount,nTriangles);

	m_spGeom = NiNew NiTriShape(pkModelData);
	m_spGeom->SetConsistency(NiGeometryData::VOLATILE);

    NiMaterialPropertyPtr spPlaneMaterial = NiNew NiMaterialProperty();
    spPlaneMaterial->SetAmbientColor(NiColor(0.5880f,0.5880f,0.5880f));
    spPlaneMaterial->SetDiffuseColor(NiColor(0.5880f,0.5880f,0.5880f));
	spPlaneMaterial->SetEmittance(NiColor(1,1,1));
	spPlaneMaterial->SetSpecularColor(NiColor(0.9000f,0.9000f,0.9000f));

	NiAlphaPropertyPtr	spAlphaProperty	=	NiNew	NiAlphaProperty();
	spAlphaProperty->SetAlphaBlending(true);
	spAlphaProperty->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
	spAlphaProperty->SetDestBlendMode(NiAlphaProperty::ALPHA_ONE);
	spAlphaProperty->SetAlphaTesting(true);
	spAlphaProperty->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);
	spAlphaProperty->SetTestRef(10);
	spAlphaProperty->SetAlphaGroup(AG_EFFECT);

	NiZBufferPropertyPtr spZBufferProperty	=	NiNew	NiZBufferProperty();
	spZBufferProperty->SetZBufferWrite(false);

	NiStencilPropertyPtr	spStencil =	NiNew	NiStencilProperty();
	spStencil->SetDrawMode(NiStencilProperty::DRAW_BOTH);

	NiTexturingPropertyPtr	spTexturing = NiNew NiTexturingProperty();
	spTexturing->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
	NiSourceTexture	*pTrailTex = g_kNifMan.GetTexture(m_kTexPath);
	spTexturing->SetBaseTexture(pTrailTex);

	NiVertexColorPropertyPtr	spVertexColor = NiNew NiVertexColorProperty();
	spVertexColor->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
	spVertexColor->SetSourceMode(NiVertexColorProperty::SOURCE_EMISSIVE);
	
    m_spGeom->AttachProperty(spAlphaProperty);
    m_spGeom->AttachProperty(spPlaneMaterial);
    m_spGeom->AttachProperty(spStencil);
    m_spGeom->AttachProperty(spVertexColor);
    m_spGeom->AttachProperty(spTexturing);
	m_spGeom->AttachProperty(spZBufferProperty);

	m_spGeom->UpdateProperties();
	m_spGeom->UpdateNodeBound();
	m_spGeom->Update(0);

	m_spGeom->SetActiveTriangleCount(0);

	AttachChild(m_spGeom, true);
}
void	PgTrailNode::Destroy()
{
	m_spGeom = 0;
	m_spSrcObj = 0;

	DestroyTrailRibbons();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgTrailNodeMan
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgTrailNodeManPtr	g_spTrailNodeMan = 0;

PgTrailNode*	PgTrailNodeMan::StartNewTrail(NiAVObject* pkSrcObj,
											  std::string const &kTexPath,
											  float fTotalExistTime,
											  float fBrightTime)
{
	if(!pkSrcObj)
		return	NULL;

	PgTrailNodePtr	spNewNode = NiNew PgTrailNode();
	spNewNode->Init(pkSrcObj,kTexPath,fTotalExistTime,fBrightTime);
	AttachChild(spNewNode, true);

	return	spNewNode;
}
PgTrailNode*	PgTrailNodeMan::StartNewTrail(NiAVObject* pkSrcObj,
											  std::string const &kCenterObjName,
											  std::string const &kTexPath,
											  float fTotalExistTime,
											  float fBrightTime,
											  float fTrailWidth)
{
	if(!pkSrcObj)
		return	NULL;

	PgTrailNodePtr	spNewNode = NiNew PgTrailNode();
	spNewNode->Init(pkSrcObj,kCenterObjName,kTexPath,fTotalExistTime,fBrightTime,fTrailWidth);
	AttachChild(spNewNode, true);

	return	spNewNode;
}
void	PgTrailNodeMan::StopTrail(PgTrailNode*	pkTrailNode,bool bRemoveImm)
{
	if(bRemoveImm)
		DetachChild(pkTrailNode);
	else
	{
		pkTrailNode->ReserveRemove();
	}
}
void	PgTrailNodeMan::Create()
{
	g_spTrailNodeMan = this;
}
void	PgTrailNodeMan::Destroy()
{
}
void PgTrailNodeMan::UpdateDownwardPass(float fTime, bool bUpdateControllers)
{
	NiNode::UpdateDownwardPass(fTime,bUpdateControllers);

	int	iTotalChild = NiNode::GetArrayCount();
	PgTrailNode	*pkObj;
	for(int i=0;i<iTotalChild;i++)
	{
		pkObj = (PgTrailNode*)GetAt(i);
		if(pkObj && pkObj->IsFinished())
		{
			DetachChildAt(i);
		}
	}
}

void PgTrailNodeMan::Draw(PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime)
{

}