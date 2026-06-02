

#include "stdafx.h"
#include "PgSkillTargetMan.h"
#include "PgNifMan.h"
#include "PgActor.h"
#include "PgRenderer.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgActionPool.H"
#include "PgAction.H"
#include "lwAction.h"
#include "variant/Global.h"

#include "NewWare/Renderer/DrawUtils.h"

//#ifndef USE_INB

PgSkillTargetMan	g_kSkillTargetMan;
unsigned int const PgSkillTargetMan::MAX_VERTS=512;

PgSkillTargetMan::PgSkillTargetMan() : 
m_pkMaterial(NULL)
//m_pkCubeVertexArray(NULL), m_pkSphereVertexArray(NULL), m_pkConeVertexArray(NULL),
//, m_spCubeLine(NULL), m_spSphereLine(NULL), m_spConeLine(NULL),	
//m_pkCubeConnect(NULL), m_pkSphereConnect(NULL), m_pkConeConnect(NULL)
{
}

void PgSkillTargetMan::SetAllConnectivity(NiBool* pkConnect, bool const bSetVal)
{//pkTarget의 내용을 일괄적으로 bSetVal로 채운다.
	assert(NULL!=pkConnect);
	memset(pkConnect, bSetVal, sizeof(NiBool)*MAX_VERTS);
}

void PgSkillTargetMan::SetRangeConnectivity(NiBool* pkConnect, size_t const iStartPos, size_t const iDestCount, bool const bSetVal)
{//pkTarget에서 iStartPos ~ iStartPos+iDestCount 범위의 내용을 일괄적으로 bSetVal로 채운다.
	assert(NULL!=pkConnect);
	assert(MAX_VERTS>iStartPos);//iStartPos가 최대범위보다는 작아야 함
	assert(MAX_VERTS>(iStartPos+iDestCount)); //iStartPos+iDestCount가 최대범위보다는 작아야 함
	memset(pkConnect+iStartPos, bSetVal, sizeof(NiBool)*iDestCount);
}

void PgSkillTargetMan::SetLineColor(NiColor const& kLineColor)
{
	m_pkMaterial->SetAmbientColor(kLineColor);
	m_pkMaterial->SetDiffuseColor(kLineColor);
	m_pkMaterial->SetSpecularColor(kLineColor);
	m_pkMaterial->SetEmittance(kLineColor);
	m_pkMaterial->SetShineness(0.0f);
}

void PgSkillTargetMan::SetTargetCone(SElem* pkElem, NiPoint3 const& kPos, NiPoint3 const& kDir, float const fRange,float const fHeight, float const fBaseRadius)
{
	if(!pkElem) 
	{
		return;
	}
	BM::CAutoMutex kLock(m_kLock);
	SetAllConnectivity(pkElem->m_pkConeConnect, false);
	int const iLongs = 10;
	int const iVCPL=3; // Vertex Count Per Loop;
	NiPoint3 kPrevPoint(0.0f, 0.0f, 0.0f);
	NiPoint3 kBase(kPos.x, kPos.y, kPos.z+fHeight);

	for(int i=0;i<=iLongs;++i)
	{
		SetRangeConnectivity(pkElem->m_pkConeConnect, i*iVCPL, 2, true);

		double dLng = 2 * PI * static_cast<double> (i - 1) / iLongs;
		double dX = cos(dLng);
		double dY = sin(dLng);

		pkElem->m_pkConeVertexArray[i*iVCPL] = NiPoint3(dX, dY, 0.0f);
		pkElem->m_pkConeVertexArray[i*iVCPL] *= fBaseRadius;
		pkElem->m_pkConeVertexArray[i*iVCPL].z = fHeight;

		if(0==i)
		{
			dLng = 2 * PI * static_cast<double>(iLongs - 1) / iLongs;
			dX = cos(dLng);
			dY = sin(dLng);
			kPrevPoint.x = dX, kPrevPoint.y = dY, kPrevPoint.z =0.0f;
			kPrevPoint *= fBaseRadius;
			kPrevPoint.z = fHeight;
		}
		pkElem->m_pkConeVertexArray[i*iVCPL+1] = kPrevPoint;
		pkElem->m_pkConeVertexArray[i*iVCPL+2] = NiPoint3(0.0f, 0.0f, 0.0f);
		kPrevPoint = pkElem->m_pkConeVertexArray[i*iVCPL];
	}
	NiPoint3 kFinalPos = kPos;
	kFinalPos += kDir*fRange;

	pkElem->m_spConeLine->UpdateProperties();
	pkElem->m_spConeLine->SetTranslate(kFinalPos);
}

void PgSkillTargetMan::SetTargetSphere(SElem* pkElem, NiPoint3 const& kPos, float const fRange , int ILats, int iLongs) 
{
	if(!pkElem) 
	{
		return;
	}
	BM::CAutoMutex kLock(m_kLock);
	SetAllConnectivity(pkElem->m_pkSphereConnect, false);

	NiPoint3 kPrevPoint(0.0f, 0.0f, 0.0f);
	int const iVCPL = 3; //Vertex Count Per Loop

	for(int i = 0; i <= ILats; i++) 
	{
		double dLat0 = PI * (-0.5 + static_cast<double> (i - 1) / ILats);
		double dZ0  = sin(dLat0);
		double dZr0 =  cos(dLat0);

		double lat1 = PI * (-0.5 + static_cast<double> (i)/ ILats);
		double dZ1 = sin(lat1);
		double dZr1 = cos(lat1);

		for(int j = 0; j <= iLongs; j++) 
		{
			SetRangeConnectivity(pkElem->m_pkSphereConnect, (i*iLongs*iVCPL)+(j*iVCPL), 2, true);

			double dLng = 2 * PI * static_cast<double> (j - 1) / iLongs;
			double dX = cos(dLng);
			double dY = sin(dLng);

			pkElem->m_pkSphereVertexArray[(i*iLongs*iVCPL)+(j*iVCPL)  ] = NiPoint3(dX*dZr0, dY*dZr0, dZ0);
			pkElem->m_pkSphereVertexArray[(i*iLongs*iVCPL)+(j*iVCPL)+1] = NiPoint3(dX*dZr1, dY*dZr1, dZ1);
			if((iLongs-1)==j)
			{
				pkElem->m_pkSphereVertexArray[(i*iLongs*iVCPL)+2] = pkElem->m_pkSphereVertexArray[(i*iLongs*iVCPL)+(j*iVCPL)+1];
			}
			pkElem->m_pkSphereVertexArray[(i*iLongs*iVCPL)+(j*iVCPL)+2] = kPrevPoint;
			kPrevPoint = pkElem->m_pkSphereVertexArray[(i*iLongs*iVCPL)+(j*iVCPL)+1];
		}
	}

	pkElem->m_spSphereLine->UpdateProperties();
	pkElem->m_spSphereLine->SetTranslate(kPos);
	pkElem->m_spSphereLine->SetScale(fRange);
}

void PgSkillTargetMan::SetTargetCube(SElem* pkElem, NiPoint3 const& kPos, NiPoint3 const& kDir, float const fSideRange, float const fRange, float const fHeight)
{
	if(!pkElem) 
	{
		return;
	}
	NiPoint3 kUp(0.0f,0.0f,1.0f);
	{
		NxVec3	vDirection(kDir.x,kDir.y,kDir.z);
		NxVec3	vRight,vUp;
		vRight = vDirection.cross(NxVec3(0.0f, 0.0f, 1.0f));
		if(vRight.distance(NxVec3(0.0f,0.0f,0.0f))<0.001f)
		{
			vRight = vDirection.cross(NxVec3(0.0f, 1.0f, 0.0f));
		}
		vUp = vDirection.cross(-vRight);
		kUp.x = vUp.x;
		kUp.y = vUp.y;
		kUp.z = vUp.z;
	}

	BM::CAutoMutex kLock(m_kLock);
	//SettingVertexPos
	NiPoint3	kVector1, kVector2, kVector3, kVector4;
	NiMatrix3	kRotateMatrix;
	NiQuaternion kRotateQuat3(NxMath::degToRad(-90.0f),kUp);
	kRotateQuat3.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector3);
	kVector3*=fSideRange/2;

	NiQuaternion kRotateQuat4(NxMath::degToRad(90.0f),kUp);
	kRotateQuat4.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector4);
	kVector4*=fSideRange/2;

	NiQuaternion kRotateQuat(NxMath::degToRad(-90.0f),kUp);
	kRotateQuat.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector1);
	kVector1*=fSideRange/2;
	kVector1+=kDir*fRange;

	NiQuaternion kRotateQuat2(NxMath::degToRad(90.0f),kUp);
	kRotateQuat2.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector2);
	kVector2*=fSideRange/2;
	kVector2+=kDir*fRange;

	//Set Vertex of LineStrip

	SetAllConnectivity(pkElem->m_pkCubeConnect, false);
	SetRangeConnectivity(pkElem->m_pkCubeConnect, 0, 7, true);
	pkElem->m_pkCubeVertexArray[0] = kVector3; pkElem->m_pkCubeVertexArray[0].z = fHeight;
	pkElem->m_pkCubeVertexArray[1] = kVector1; pkElem->m_pkCubeVertexArray[1].z = fHeight;
	pkElem->m_pkCubeVertexArray[2] = kVector2; pkElem->m_pkCubeVertexArray[2].z = fHeight;
	pkElem->m_pkCubeVertexArray[3] = kVector4; pkElem->m_pkCubeVertexArray[3].z = fHeight;
	pkElem->m_pkCubeVertexArray[4] = kVector3; pkElem->m_pkCubeVertexArray[4].z = fHeight;
	pkElem->m_pkCubeVertexArray[5] = kVector3;
	pkElem->m_pkCubeVertexArray[6] = kVector1;
	pkElem->m_pkCubeVertexArray[7] = kVector1; pkElem->m_pkCubeVertexArray[7].z = fHeight;

	SetRangeConnectivity(pkElem->m_pkCubeConnect, 8, 7, true);
	pkElem->m_pkCubeVertexArray[8] = kVector4;
	pkElem->m_pkCubeVertexArray[9] = kVector2;
	pkElem->m_pkCubeVertexArray[10] = kVector2; pkElem->m_pkCubeVertexArray[10].z = fHeight;
	pkElem->m_pkCubeVertexArray[11] = kVector4; pkElem->m_pkCubeVertexArray[11].z = fHeight;
	pkElem->m_pkCubeVertexArray[12] = kVector4;
	pkElem->m_pkCubeVertexArray[13] = kVector3;
	pkElem->m_pkCubeVertexArray[14] = kVector1;
	pkElem->m_pkCubeVertexArray[15] = kVector2;

	pkElem->m_spCubeLine->UpdateProperties();
	pkElem->m_spCubeLine->SetTranslate(kPos);
}

void PgSkillTargetMan::SetTargetListUpdateInterval(BM::GUID const kGUID, int const iInterval)
{
	SElem* pkElem = GetUnitElem(kGUID);
	if(pkElem)
	{
		BM::CAutoMutex kLock(m_kLock);
		pkElem->m_iUpdateTargetInterval = iInterval;
	}
}

void PgSkillTargetMan::Update(float fAccumTime)
{
	m_fSavedAccumTime = fAccumTime;
}

void PgSkillTargetMan::DrawTargetArea(PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime)
{
	if(m_bDrawTargetArea)
	{
		BM::CAutoMutex kLock(m_kLock);
		CONT_ELEM::iterator kItor = m_kContElem.begin();
		while(m_kContElem.end() != kItor)
		{
			SElem& rkElem = kItor->second;
			//NiNodePtr spAreaNif = NULL;
			NiLinesPtr spLines= NULL;

			switch(rkElem.m_TargetAreaType)
			{
			case TAT_CIRCLE:
				{
					//spAreaNif = m_spAreaCircleNif;
					spLines = rkElem.m_spSphereLine;

				}break;			
			case TAT_TRIANGLE:
				{
					//spAreaNif = m_spAreaTriNif;
					spLines= rkElem.m_spConeLine;
				}break;
			case TAT_RECTANGLE:
				{
					//spAreaNif = m_spAreaRectNif;
					spLines = rkElem.m_spCubeLine;
				}break;
			}

			//if(NULL != spAreaNif)
			//{
			//	//spAreaNif->Update(m_fSavedAccumTime);
			//	//pkRenderer->PartialRenderClick_Deprecated(spAreaNif);
			//}
			if(NULL != spLines)
			{			
				spLines->Update(m_fSavedAccumTime);
				NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), spLines );
				SetAllConnectivity(rkElem.m_pkCubeConnect, false);
				SetAllConnectivity(rkElem.m_pkSphereConnect, false);
				SetAllConnectivity(rkElem.m_pkConeConnect, false);
			}
			++kItor;
		}
	}
}
void PgSkillTargetMan::DrawImmediate(PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime)
{
	BM::CAutoMutex kLock(m_kLock);
	
	for(CONT_ELEM::iterator kItor = m_kContElem.begin(); m_kContElem.end() != kItor; ++kItor)
	{
		SElem& rkElem = kItor->second;	

		if(m_bDrawTargetArea)
		{
			PgActionTargetInfo* pkTargetInfo = NULL;
			PgPilot* pkPilot = NULL;
			PgActor* pkActor = NULL;
			
			pkActor = g_kPilotMan.FindActor(kItor->first);
			if(pkActor)
			{//공격범위 처리
				SetLineColor(NiColor::NiColor(1.0f, 1.0f, 1.0f));
				PgAction* pkCurAction = pkActor->GetAction();
				if(pkCurAction)
				{
					int const iCurActionNo = pkCurAction->GetActionNo();
					if(iCurActionNo != rkElem.iActionNo)
					{
						continue;
					}
					
					switch(rkElem.iAreaType)
					{
					case PgAction::TAT_SPHERE :
						{
							rkElem.m_TargetAreaType = TAT_CIRCLE;
						}break;
					default:
						{
							rkElem.m_TargetAreaType = TAT_RECTANGLE;
						}break;
					}

					switch(rkElem.m_TargetAreaType)
					{
					case TAT_RECTANGLE:
						{
							SetTargetCube(&rkElem, rkElem.kStartPos, rkElem.kDir, rkElem.fWideAngle, rkElem.fSkillRange, 30.0f);
						}break;
					case TAT_CIRCLE:
						{
							//SetTargetAreaCircle(kStart, fEffectRadius);
							SetTargetSphere(&rkElem, rkElem.kStartPos, rkElem.fEffectRadius);
						}break;
					case TAT_TRIANGLE:
						{
							//SetTargetAreaTriangle(rkElem.kStartPos, rkElem.kDir, rkElem.fWideAngle, rkElem.fSkillRange);
						}
					default:
						{
						}break;
					}
					DrawTargetArea(pkRenderer, pkCamera, fFrameTime);
				}
			}
		}

		if(m_bDrawTargetMark)
		{
			if(!m_spTargetingNif)
			{
				m_spTargetingNif = g_kNifMan.GetNif("../Data/5_Effect/0_Common/ef_select_01.nif");
				NiTimeController::StartAnimations(m_spTargetingNif);

				if(NULL == m_spTargetingNif)
				{
					NILOG(PGLOG_ERROR, "[PgSkillTargetMan] Create TargetingNif is Failed\n");
					return;
				}
			}

			if(!m_spSelectEnemyNif)
			{
				m_spSelectEnemyNif = g_kNifMan.GetNif("../Data/5_Effect/0_Common/ef_select_02.nif");
				NiTimeController::StartAnimations(m_spSelectEnemyNif);

				if(NULL == m_spSelectEnemyNif)
				{
					NILOG(PGLOG_ERROR, "[PgSkillTargetMan] Create SelectEnemyNif is Failed\n");
					return;
				}
			}
		}
	}
}

void PgSkillTargetMan::EnableTargetAreaDisplay(bool const bEnable)
{
	m_bDrawTargetArea = bEnable;
}
void PgSkillTargetMan::SetTargetAreaCircle(SElem* pkElem, NiPoint3 const& kCenter, float const fRange)
{
	if(!pkElem)
	{
		return;
	}
	BM::CAutoMutex kLock(m_kLock);
	NiTriStripsPtr	kTriStripGeom = NULL;
	if(!pkElem->m_spAreaCircleNif)
	{
		pkElem->m_spAreaCircleNif = g_kNifMan.GetNif_DeepCopy("../Data/5_Effect/0_Common/ef_area_circle.nif");
		if(!pkElem->m_spAreaCircleNif) return;

		kTriStripGeom = (NiTriStrips*)( pkElem->m_spAreaCircleNif->GetObjectByName("Plane09") );
		kTriStripGeom->SetConsistency(NiGeometryData::VOLATILE);

		NiGeometry* pkGeom = (NiGeometry*)( pkElem->m_spAreaCircleNif->GetObjectByName("Plane09") );
		if(pkGeom)
		{
			NiMaterialProperty	*m_pkLineMarterial = pkGeom->GetPropertyState()->GetMaterial();
			if(m_pkLineMarterial)
			{
				m_pkLineMarterial->SetAlpha(1.0f);
			}
		}
	}
	else
	{
		kTriStripGeom = (NiTriStrips*)( pkElem->m_spAreaCircleNif->GetObjectByName("Plane09") );
	}

	if(!kTriStripGeom) return;

	pkElem->m_TargetAreaType = TAT_CIRCLE;

	NiPoint3* pkVert = kTriStripGeom->GetVertices();
	NiPoint3 kVector1(-fRange,fRange,0),kVector2(-fRange,-fRange,0) ,kVector3(fRange,fRange,0),kVector4(fRange,-fRange,0);

	*pkVert = kVector1;pkVert->z+=0.1f;	pkVert++;
	*pkVert = kVector2;pkVert->z+=0.1f;	pkVert++;
	*pkVert = kVector3;pkVert->z+=0.1f;	pkVert++;
	*pkVert = kVector4;pkVert->z+=0.1f;	

	kTriStripGeom->GetModelData()->MarkAsChanged(NiGeometryData::VERTEX_MASK);
	pkElem->m_spAreaCircleNif->SetTranslate(kCenter);
	pkElem->m_spAreaCircleNif->UpdateWorldBound();
}
void	PgSkillTargetMan::SetTargetAreaTriangle(SElem* pkElem, NiPoint3 const& kStart, NiPoint3 const& kDir, float const fAngle, float const fRange)
{
	if(!pkElem)
	{
		return;
	}
	BM::CAutoMutex kLock(m_kLock);
	NiTriStripsPtr	kTriStripGeom = NULL;
	if(!pkElem->m_spAreaTriNif)
	{
		pkElem->m_spAreaTriNif = g_kNifMan.GetNif_DeepCopy("../Data/5_Effect/0_Common/ef_area_tri.nif");
		if(!pkElem->m_spAreaTriNif) return;

		kTriStripGeom = (NiTriStrips*)(pkElem->m_spAreaTriNif->GetObjectByName("Plane09"));
		kTriStripGeom->SetConsistency(NiGeometryData::VOLATILE);

		NiGeometry* pkGeom = (NiGeometry*)(pkElem->m_spAreaTriNif->GetObjectByName("Plane09"));
		if(pkGeom)
		{
			NiMaterialProperty* m_pkLineMarterial = pkGeom->GetPropertyState()->GetMaterial();
			if(m_pkLineMarterial)
			{
				m_pkLineMarterial->SetAlpha(1.0f);
			}
		}
	}
	else
	{
		kTriStripGeom = (NiTriStrips*)(pkElem->m_spAreaTriNif->GetObjectByName("Plane09"));
	}

	if(!kTriStripGeom) return;
	
	NiPoint3 kUp(0.0f,0.0f,1.0f);
	{
		NxVec3	vDirection(kDir.x,kDir.y,kDir.z);
		NxVec3	vRight,vUp;
		vRight = vDirection.cross(NxVec3(0.0f, 0.0f, 1.0f));
		if(vRight.distance(NxVec3(0.0f,0.0f,0.0f))<0.001f)
		{
			vRight = vDirection.cross(NxVec3(0.0f, 1.0f, 0.0f));
		}
		vUp = vDirection.cross(-vRight);
		kUp.x = vUp.x;
		kUp.y = vUp.y;
		kUp.z = vUp.z;
	}

	pkElem->m_TargetAreaType = TAT_TRIANGLE;

	NiPoint3	kVector1,kVector2,kVector3,kVector4;
	NiMatrix3	kRotateMatrix;
	NiQuaternion kRotateQuat(NxMath::degToRad(fAngle),kUp);
	kRotateQuat.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector1);
	kVector1*=fRange;

	NiQuaternion kRotateQuat2(NxMath::degToRad(-fAngle),kUp);
	kRotateQuat2.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector2);
	kVector2*=fRange;

	float const fSideRange = (NiPoint3(kVector1.x,kVector1.y,0) - NiPoint3(kVector2.x,kVector2.y,0)).Length()/2;

	NiQuaternion kRotateQuat3(NxMath::degToRad(-90.0f),kUp);
	kRotateQuat3.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector3);
	kVector3*=fSideRange;

	NiQuaternion kRotateQuat4(NxMath::degToRad(90.0f),kUp);
	kRotateQuat4.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector4);
	kVector4*=fSideRange;

	NiPoint3* pkVert = kTriStripGeom->GetVertices();

	pkVert->x = kVector1.x;	pkVert->y = kVector1.y;	pkVert->z=kVector1.z+0.1f; pkVert++;
	pkVert->x = kVector4.x;	pkVert->y = kVector4.y;	pkVert->z=kVector4.z+0.1f; pkVert++;
	pkVert->x = kVector2.x;	pkVert->y = kVector2.y;	pkVert->z=kVector2.z+0.1f; pkVert++;
	pkVert->x = kVector3.x;	pkVert->y = kVector3.y;	pkVert->z=kVector3.z+0.1f; pkVert++;

	kTriStripGeom->GetModelData()->MarkAsChanged(NiGeometryData::VERTEX_MASK);
	pkElem->m_spAreaTriNif->SetTranslate(kStart);
	pkElem->m_spAreaTriNif->UpdateWorldBound();
}

void	PgSkillTargetMan::SetTargetAreaRectangle(SElem* pkElem, NiPoint3 const& kStart,NiPoint3 const& kDir,float const fSideRange, float const fRange)
{
	if(!pkElem)
	{
		return;
	}
	BM::CAutoMutex kLock(m_kLock);
	NiTriStripsPtr	kTriStripGeom = NULL;
	if(!pkElem->m_spAreaRectNif)
	{
		pkElem->m_spAreaRectNif = g_kNifMan.GetNif_DeepCopy("../Data/5_Effect/0_Common/ef_area_box.nif");
		if(!pkElem->m_spAreaRectNif) return;

		kTriStripGeom = (NiTriStrips*)(pkElem->m_spAreaRectNif->GetObjectByName("Plane09"));
		kTriStripGeom->SetConsistency(NiGeometryData::VOLATILE);

		NiGeometry* pkGeom = (NiGeometry*)(pkElem->m_spAreaRectNif->GetObjectByName("Plane09"));
		if(pkGeom)
		{
			NiMaterialProperty	*m_pkLineMarterial = pkGeom->GetPropertyState()->GetMaterial();
			if(m_pkLineMarterial)
			{
				m_pkLineMarterial->SetAlpha(1.0f);
			}
		}
	}
	else
	{
		kTriStripGeom = (NiTriStrips*)(pkElem->m_spAreaRectNif->GetObjectByName("Plane09"));
	}

	if(!kTriStripGeom) return;
	
	NiPoint3 kUp(0.0f,0.0f,1.0f);
	{
		NxVec3	vDirection(kDir.x,kDir.y,kDir.z);
		NxVec3	vRight,vUp;
		vRight = vDirection.cross(NxVec3(0.0f, 0.0f, 1.0f));
		if(vRight.distance(NxVec3(0.0f,0.0f,0.0f))<0.001f)
		{
			vRight = vDirection.cross(NxVec3(0.0f, 1.0f, 0.0f));
		}
		vUp = vDirection.cross(-vRight);
		kUp.x = vUp.x;
		kUp.y = vUp.y;
		kUp.z = vUp.z;
	}

	pkElem->m_TargetAreaType = TAT_RECTANGLE;

	NiPoint3	kVector1, kVector2, kVector3, kVector4;
	NiMatrix3	kRotateMatrix;

	NiQuaternion kRotateQuat3(NxMath::degToRad(-90.0f),kUp);
	kRotateQuat3.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector3);
	kVector3*=fSideRange/2;

	NiQuaternion kRotateQuat4(NxMath::degToRad(90.0f),kUp);
	kRotateQuat4.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector4);
	kVector4*=fSideRange/2;

	NiQuaternion kRotateQuat(NxMath::degToRad(-90.0f),kUp);
	kRotateQuat.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector1);
	kVector1*=fSideRange/2;
	kVector1+=kDir*fRange;

	NiQuaternion kRotateQuat2(NxMath::degToRad(90.0f),kUp);
	kRotateQuat2.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector2);
	kVector2*=fSideRange/2;
	kVector2+=kDir*fRange;

	NiPoint3* pkVert = kTriStripGeom->GetVertices();

	pkVert->x = kVector2.x;	pkVert->y = kVector2.y;	pkVert->z=kVector2.z+0.1f; pkVert++;
	pkVert->x = kVector4.x;	pkVert->y = kVector4.y;	pkVert->z=kVector4.z+0.1f; pkVert++;
	pkVert->x = kVector1.x;	pkVert->y = kVector1.y;	pkVert->z=kVector1.z+0.1f; pkVert++;
	pkVert->x = kVector3.x;	pkVert->y = kVector3.y;	pkVert->z=kVector3.z+0.1f; pkVert++;

	kTriStripGeom->GetModelData()->MarkAsChanged(NiGeometryData::VERTEX_MASK);
	pkElem->m_spAreaRectNif->SetTranslate(kStart);
	pkElem->m_spAreaRectNif->UpdateWorldBound();
}

void PgSkillTargetMan::Init()
{
	m_spTargetingNif = NULL;
	m_bDrawTargetMark = false;
	m_bDrawTargetArea = false;

	m_pkMaterial = NiNew NiMaterialProperty();
	m_pkMaterial->SetAmbientColor(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetDiffuseColor(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetSpecularColor(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetEmittance(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetShineness(0.0f);
	m_fSavedAccumTime = NULL;
	
}

void PgSkillTargetMan::Destroy()
{
	BM::CAutoMutex kLock(m_kLock);
	CONT_ELEM::iterator kItor = m_kContElem.begin();
	while(m_kContElem.end() != kItor)
	{
		SElem& rkElem = kItor->second;

		rkElem.m_spAreaCircleNif = NULL;
		rkElem.m_spAreaTriNif = NULL;
		rkElem.m_spAreaRectNif = NULL;

		//draw cube stuff
		rkElem.m_spCubeLine = NULL;
		if(NULL != rkElem.m_pkCubeVertexArray)
		{
			NiDelete[] rkElem.m_pkCubeVertexArray;
			rkElem.m_pkCubeVertexArray = NULL;
		}
		if(NULL != rkElem.m_pkCubeConnect)
		{
			NiFree(rkElem.m_pkCubeConnect);
			rkElem.m_pkCubeConnect = NULL;
		}
		//draw sphere stuff
		rkElem.m_spSphereLine = NULL;
		if(NULL != rkElem.m_pkSphereVertexArray)
		{
			NiDelete[] rkElem.m_pkSphereVertexArray;
			rkElem.m_pkSphereVertexArray = NULL;
		}
		if(NULL != rkElem.m_pkSphereConnect)
		{
			NiFree(rkElem.m_pkSphereConnect);
			rkElem.m_pkSphereConnect = NULL;
		}

		//draw cone stuff
		rkElem.m_spConeLine = NULL;
		if(NULL != rkElem.m_pkConeVertexArray)
		{
			NiDelete[] rkElem.m_pkConeVertexArray;
			rkElem.m_pkConeVertexArray = NULL;
		}
		if(NULL != rkElem.m_pkConeConnect)
		{
			NiFree(rkElem.m_pkConeConnect);
			rkElem.m_pkConeConnect = NULL;
		}

		++kItor;
	}
	
	m_spTargetingNif = NULL;
	m_spSelectEnemyNif = NULL;

	if(NULL != m_pkMaterial)
	{
		NiDelete m_pkMaterial;
		m_pkMaterial = NULL;
	}
}
//#endif//USE_INB


PgSkillTargetMan::SElem* PgSkillTargetMan::AddUnit(BM::GUID const& kGUID)
{
	BM::CAutoMutex kLock(m_kLock);
	SElem kEmpty;
	std::pair<CONT_ELEM::iterator, bool> kRet = m_kContElem.insert( std::make_pair(kGUID, kEmpty) );
	if(!kRet.second)
	{
		return NULL;
	}
	
	SElem& rkElem = kRet.first->second;

	rkElem.m_ulLastTargetUpdateTime = 0;
	rkElem.m_iUpdateTargetInterval = 0;

	rkElem.m_spAreaCircleNif = NULL;
	rkElem.m_spAreaTriNif = NULL;
	rkElem.m_spAreaRectNif = NULL;

	//draw cube stuff
	rkElem.m_pkCubeVertexArray = NiNew NiPoint3[MAX_VERTS];
	rkElem.m_pkCubeConnect = (NiBool*)NiMalloc(sizeof(NiBool)*MAX_VERTS);
	rkElem.m_spCubeLine = NiNew NiLines(MAX_VERTS, rkElem.m_pkCubeVertexArray, NULL, NULL, 0, NiGeometryData::NBT_METHOD_NONE, rkElem.m_pkCubeConnect);
	rkElem.m_spCubeLine->IncRefCount();

	rkElem.m_spCubeLine->AttachProperty(m_pkMaterial);
	rkElem.m_spCubeLine->Update(0.0f);
	rkElem.m_spCubeLine->UpdateEffects();
	rkElem.m_spCubeLine->UpdateProperties();
	rkElem.m_spCubeLine->UpdateNodeBound();	

	//draw sphere stuff
	rkElem.m_pkSphereVertexArray = NiNew NiPoint3[MAX_VERTS];
	rkElem.m_pkSphereConnect = (NiBool*)NiMalloc(sizeof(NiBool)*MAX_VERTS);
	rkElem.m_spSphereLine = NiNew NiLines(MAX_VERTS, rkElem.m_pkSphereVertexArray, NULL, NULL, 0, NiGeometryData::NBT_METHOD_NONE, rkElem.m_pkSphereConnect);
	rkElem.m_spSphereLine->IncRefCount();

	rkElem.m_spSphereLine->AttachProperty(m_pkMaterial);
	rkElem.m_spSphereLine->Update(0.0f);
	rkElem.m_spSphereLine->UpdateEffects();
	rkElem.m_spSphereLine->UpdateProperties();
	rkElem.m_spSphereLine->UpdateNodeBound();

	//draw cone stuff
	rkElem.m_pkConeVertexArray = NiNew NiPoint3[MAX_VERTS];
	rkElem.m_pkConeConnect = (NiBool*)NiMalloc(sizeof(NiBool)*MAX_VERTS);
	rkElem.m_spConeLine = NiNew NiLines(MAX_VERTS, rkElem.m_pkConeVertexArray, NULL, NULL, 0, NiGeometryData::NBT_METHOD_NONE, rkElem.m_pkConeConnect);
	rkElem.m_spConeLine->IncRefCount();

	rkElem.m_spConeLine->AttachProperty(m_pkMaterial);
	rkElem.m_spConeLine->Update(0.0f);
	rkElem.m_spConeLine->UpdateEffects();
	rkElem.m_spConeLine->UpdateProperties();
	rkElem.m_spConeLine->UpdateNodeBound();

	return &rkElem;
}
void PgSkillTargetMan::EditSkillRangeParam(BM::GUID const& kGUID, int const iActionNo, int const iAreaType, NiPoint3 const& kDir, NiPoint3 const& kStartPos, float const fSkillRange, float const fWideAngle, float const fEffectRadius)
{
	BM::CAutoMutex kLock(m_kLock);
	CONT_ELEM::iterator kItor = m_kContElem.find(kGUID);
	SElem* pkElem = NULL;
	if( m_kContElem.end() == kItor )
	{
		pkElem = AddUnit(kGUID);
	}
	else
	{
		pkElem = &kItor->second;
	}
	
	pkElem->iActionNo = iActionNo;
	pkElem->iAreaType = iAreaType;
	pkElem->kDir = kDir;
	pkElem->kStartPos = kStartPos;
	pkElem->fSkillRange = fSkillRange;
	pkElem->fWideAngle = fWideAngle;
	pkElem->fEffectRadius = fEffectRadius;
}
void PgSkillTargetMan::DeleteUnit(BM::GUID const& kGUID)
{
	BM::CAutoMutex kLock(m_kLock);
	m_kContElem.erase(kGUID);
}
PgSkillTargetMan::SElem* PgSkillTargetMan::GetUnitElem(BM::GUID const& kGUID)
{
	CONT_ELEM::iterator kItor = m_kContElem.find(kGUID);
	if( m_kContElem.end() == kItor)
	{
		return NULL;
	}
	return &(kItor->second);
}

///////////////////////////////////////////////////////////////////////////////////////

PgMonSkillTargetMan	g_kMonSkillTargetMan;
unsigned int const PgMonSkillTargetMan::MAX_VERTS=512;

PgMonSkillTargetMan::PgMonSkillTargetMan() : 
m_pkCubeVertexArray(NULL), m_pkSphereVertexArray(NULL), m_pkConeVertexArray(NULL),
m_pkMaterial(NULL), m_spCubeLine(NULL), m_spSphereLine(NULL), m_spConeLine(NULL),	
m_pkCubeConnect(NULL), m_pkSphereConnect(NULL), m_pkConeConnect(NULL)
{
}

void PgMonSkillTargetMan::SetAllConnectivity(NiBool* pkConnect, bool const bSetVal)
{//pkTarget의 내용을 일괄적으로 bSetVal로 채운다.
	assert(NULL!=pkConnect);
	memset(pkConnect, bSetVal, sizeof(NiBool)*MAX_VERTS);
}

void PgMonSkillTargetMan::SetRangeConnectivity(NiBool* pkConnect, size_t const iStartPos, size_t const iDestCount, bool const bSetVal)
{//pkTarget에서 iStartPos ~ iStartPos+iDestCount 범위의 내용을 일괄적으로 bSetVal로 채운다.
	assert(NULL!=pkConnect);
	assert(MAX_VERTS>iStartPos);//iStartPos가 최대범위보다는 작아야 함
	assert(MAX_VERTS>(iStartPos+iDestCount)); //iStartPos+iDestCount가 최대범위보다는 작아야 함
	memset(pkConnect+iStartPos, bSetVal, sizeof(NiBool)*iDestCount);
}

void PgMonSkillTargetMan::SetLineColor(NiColor const& kLineColor)
{
	m_pkMaterial->SetAmbientColor(kLineColor);
	m_pkMaterial->SetDiffuseColor(kLineColor);
	m_pkMaterial->SetSpecularColor(kLineColor);
	m_pkMaterial->SetEmittance(kLineColor);
	m_pkMaterial->SetShineness(0.0f);
}

void PgMonSkillTargetMan::SetTargetCone(NiPoint3 const& kPos, NiPoint3 const& kDir, float const fRange,float const fHeight, float const fBaseRadius)
{
	SetAllConnectivity(m_pkConeConnect, false);
	int const iLongs = 10;
	int const iVCPL=3; // Vertex Count Per Loop;
	NiPoint3 kPrevPoint(0.0f, 0.0f, 0.0f);
	NiPoint3 kBase(kPos.x, kPos.y, kPos.z+fHeight);

	for(int i=0;i<=iLongs;++i)
	{
		SetRangeConnectivity(m_pkConeConnect, i*iVCPL, 2, true);

		double dLng = 2 * PI * static_cast<double> (i - 1) / iLongs;
		double dX = cos(dLng);
		double dY = sin(dLng);

		m_pkConeVertexArray[i*iVCPL] = NiPoint3(dX, dY, 0.0f);
		m_pkConeVertexArray[i*iVCPL] *= fBaseRadius;
		m_pkConeVertexArray[i*iVCPL].z = fHeight;

		if(0==i)
		{
			dLng = 2 * PI * static_cast<double>(iLongs - 1) / iLongs;
			dX = cos(dLng);
			dY = sin(dLng);
			kPrevPoint.x = dX, kPrevPoint.y = dY, kPrevPoint.z =0.0f;
			kPrevPoint *= fBaseRadius;
			kPrevPoint.z = fHeight;
		}
		m_pkConeVertexArray[i*iVCPL+1] = kPrevPoint;
		m_pkConeVertexArray[i*iVCPL+2] = NiPoint3(0.0f, 0.0f, 0.0f);
		kPrevPoint = m_pkConeVertexArray[i*iVCPL];
	}
	NiPoint3 kFinalPos = kPos;
	kFinalPos += kDir*fRange;

	m_spConeLine->UpdateProperties();
	m_spConeLine->SetTranslate(kFinalPos);
}

void PgMonSkillTargetMan::SetTargetSphere(NiPoint3 const& kPos, float const fRange , int ILats, int iLongs) 
{
	SetAllConnectivity(m_pkSphereConnect, false);

	NiPoint3 kPrevPoint(0.0f, 0.0f, 0.0f);
	int const iVCPL = 3; //Vertex Count Per Loop

	for(int i = 0; i <= ILats; i++) 
	{
		double dLat0 = PI * (-0.5 + static_cast<double> (i - 1) / ILats);
		double dZ0  = sin(dLat0);
		double dZr0 =  cos(dLat0);

		double lat1 = PI * (-0.5 + static_cast<double> (i)/ ILats);
		double dZ1 = sin(lat1);
		double dZr1 = cos(lat1);

		for(int j = 0; j <= iLongs; j++) 
		{
			SetRangeConnectivity(m_pkSphereConnect, (i*iLongs*iVCPL)+(j*iVCPL), 2, true);

			double dLng = 2 * PI * static_cast<double> (j - 1) / iLongs;
			double dX = cos(dLng);
			double dY = sin(dLng);

			m_pkSphereVertexArray[(i*iLongs*iVCPL)+(j*iVCPL)  ] = NiPoint3(dX*dZr0, dY*dZr0, dZ0);
			m_pkSphereVertexArray[(i*iLongs*iVCPL)+(j*iVCPL)+1] = NiPoint3(dX*dZr1, dY*dZr1, dZ1);
			if((iLongs-1)==j)
			{
				m_pkSphereVertexArray[(i*iLongs*iVCPL)+2] = m_pkSphereVertexArray[(i*iLongs*iVCPL)+(j*iVCPL)+1];
			}
			m_pkSphereVertexArray[(i*iLongs*iVCPL)+(j*iVCPL)+2] = kPrevPoint;
			kPrevPoint = m_pkSphereVertexArray[(i*iLongs*iVCPL)+(j*iVCPL)+1];
		}
	}

	m_spSphereLine->UpdateProperties();
	m_spSphereLine->SetTranslate(kPos);
	m_spSphereLine->SetScale(fRange);
}

void PgMonSkillTargetMan::SetTargetCube(NiPoint3 const& kPos, NiPoint3 const& kDir, float const fSideRange, float const fRange, float const fHeight)
{
	//SettingVertexPos
	NiPoint3	kVector1, kVector2, kVector3, kVector4;
	NiMatrix3	kRotateMatrix;

	NiQuaternion kRotateQuat3(-90.0f*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat3.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector3);
	kVector3*=fSideRange/2;

	NiQuaternion kRotateQuat4(90.0f*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat4.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector4);
	kVector4*=fSideRange/2;

	NiQuaternion kRotateQuat(-90.0f*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector1);
	kVector1*=fSideRange/2;
	kVector1+=kDir*fRange;

	NiQuaternion kRotateQuat2(90.0f*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat2.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector2);
	kVector2*=fSideRange/2;
	kVector2+=kDir*fRange;

	//Set Vertex of LineStrip

	SetAllConnectivity(m_pkCubeConnect, false);
	SetRangeConnectivity(m_pkCubeConnect, 0, 7, true);
	m_pkCubeVertexArray[0] = kVector3; m_pkCubeVertexArray[0].z = fHeight;
	m_pkCubeVertexArray[1] = kVector1; m_pkCubeVertexArray[1].z = fHeight;
	m_pkCubeVertexArray[2] = kVector2; m_pkCubeVertexArray[2].z = fHeight;
	m_pkCubeVertexArray[3] = kVector4; m_pkCubeVertexArray[3].z = fHeight;
	m_pkCubeVertexArray[4] = kVector3; m_pkCubeVertexArray[4].z = fHeight;
	m_pkCubeVertexArray[5] = kVector3;
	m_pkCubeVertexArray[6] = kVector1;
	m_pkCubeVertexArray[7] = kVector1; m_pkCubeVertexArray[7].z = fHeight;

	SetRangeConnectivity(m_pkCubeConnect, 8, 7, true);
	m_pkCubeVertexArray[8] = kVector4;
	m_pkCubeVertexArray[9] = kVector2;
	m_pkCubeVertexArray[10] = kVector2; m_pkCubeVertexArray[10].z = fHeight;
	m_pkCubeVertexArray[11] = kVector4; m_pkCubeVertexArray[11].z = fHeight;
	m_pkCubeVertexArray[12] = kVector4;
	m_pkCubeVertexArray[13] = kVector3;
	m_pkCubeVertexArray[14] = kVector1;
	m_pkCubeVertexArray[15] = kVector2;

	m_spCubeLine->UpdateProperties();
	m_spCubeLine->SetTranslate(kPos);
}

void PgMonSkillTargetMan::SetTargetListUpdateInterval(int const iInterval)
{
	m_iUpdateTargetInterval = iInterval;
}

void PgMonSkillTargetMan::Update(float fAccumTime)
{
	m_fSavedAccumTime = fAccumTime;
}

void PgMonSkillTargetMan::DrawTargetArea(PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime)
{
	if(m_bDrawTargetArea)
	{
		//NiNodePtr spAreaNif = NULL;
		NiLinesPtr spLines= NULL;

		switch(m_TargetAreaType)
		{
		case TAT_CIRCLE:
			{
				//spAreaNif = m_spAreaCircleNif;
				spLines = m_spSphereLine;

			}break;			
		case TAT_TRIANGLE:
			{
				//spAreaNif = m_spAreaTriNif;
				spLines= m_spConeLine;
			}break;
		case TAT_RECTANGLE:
			{
				//spAreaNif = m_spAreaRectNif;
				spLines = m_spCubeLine;
			}break;
		}

		//if(NULL != spAreaNif)
		//{
		//	//spAreaNif->Update(m_fSavedAccumTime);
		//	//pkRenderer->PartialRenderClick_Deprecated(spAreaNif);
		//}
		if(NULL != spLines)
		{			
			spLines->Update(m_fSavedAccumTime);
            NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), spLines );
			SetAllConnectivity(m_pkCubeConnect, false);
			SetAllConnectivity(m_pkSphereConnect, false);
			SetAllConnectivity(m_pkConeConnect, false);
		}
	}
}
void PgMonSkillTargetMan::DrawImmediate(PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime)
{
	if(m_bDrawTargetMark || m_bDrawTargetArea)
	{
		UpdateTargetList();
	}

	if(m_bDrawTargetArea)
	{
		PgActionTargetInfo* pkTargetInfo = NULL;
		PgPilot* pkPilot = NULL;
		PgActor* pkActor = NULL;

		for(ActionTargetList::iterator itor2 = m_kAreaInEnemyList.begin(); itor2 != m_kAreaInEnemyList.end(); ++itor2)
		{
			pkTargetInfo = &(*itor2);

			pkPilot = g_kPilotMan.FindPilot(pkTargetInfo->GetTargetPilotGUID());
			if(!pkPilot) continue;

			pkActor =  dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			if(!pkActor) continue;

			if(pkActor)
			{
				PgAction const* pkCurAction = pkActor->GetAction();
				if(!pkCurAction) continue;

				CSkillDef const* pkSkillDef = pkCurAction->GetSkillDef();
				if(!pkSkillDef)
				{
					continue;
				}

				//1차 공격범위 그리기
				SetLineColor(NiColor::NiColor(1.0f, 0.0f, 0.0f));
				float const fRange = PgAdjustSkillFunc::GetAttackRange(pkPilot->GetUnit(), pkSkillDef);
				int const i1stAttacAreaType = pkSkillDef->GetAbil(AT_1ST_AREA_TYPE);
				float const fAreaParam = static_cast<float>(pkSkillDef->GetAbil(AT_1ST_AREA_PARAM2));
				switch(i1stAttacAreaType)
				{
				case ESArea_Sphere:
					{	
						m_TargetAreaType = TAT_CIRCLE;
						//SetTargetAreaCircle(pkActor->GetPos(), fAreaParam);
						SetTargetSphere(pkActor->GetPos(), fAreaParam);
						DrawTargetArea(pkRenderer, pkCamera, fFrameTime);
					}break;
				case ESArea_Cube:
					{
						m_TargetAreaType = TAT_RECTANGLE;						
						//SetTargetAreaRectangle(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam, fRange);
						SetTargetCube(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam, fRange, 30.0f);
						DrawTargetArea(pkRenderer, pkCamera, fFrameTime);
					}break;
				case ESArea_Cone:
					{
						m_TargetAreaType = TAT_TRIANGLE;
						//SetTargetAreaTriangle(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam, fRange);
						SetTargetCone(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam, 30.0f, fRange);
						DrawTargetArea(pkRenderer, pkCamera, fFrameTime);
					}break;
				default :
					{
						float const fAreaParam1 = static_cast<float>(pkSkillDef->GetAbil(AT_2ND_AREA_PARAM1));
						float const fAreaParam2 = static_cast<float>(pkSkillDef->GetAbil(AT_2ND_AREA_PARAM2));
						if(0.0f != fAreaParam1 && 0.0f != fAreaParam2)
						{
							SetTargetAreaRectangle(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam1, fAreaParam2);
							DrawTargetArea(pkRenderer, pkCamera, fFrameTime);
						}
					}break;
				}
				//2차 공격범위 그리기

				SetLineColor(NiColor::NiColor(0.0f, 0.0f, 1.0f));
				int const i2ndAttackParam1 = static_cast<int>(pkSkillDef->GetAbil(AT_2ND_AREA_PARAM1));
				if(0 != i2ndAttackParam1)
				{
					int const i2ndAttackAreaType = static_cast<int>(pkSkillDef->GetAbil(AT_1ST_AREA_TYPE));
					float const fAreaParam = static_cast<float>(pkSkillDef->GetAbil(AT_2ND_AREA_PARAM2));

					switch(i2ndAttackAreaType)
					{
					case ESArea_Sphere:
						{
							m_TargetAreaType = TAT_CIRCLE;
							//SetTargetAreaCircle(pkActor->GetPos(), fAreaParam);
							SetTargetSphere(pkActor->GetPos(), fAreaParam);
							DrawTargetArea(pkRenderer, pkCamera, fFrameTime);
						}break;
					case ESArea_Cube:
						{
							m_TargetAreaType = TAT_RECTANGLE;
							//SetTargetAreaRectangle(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam, fRange);
							SetTargetCube(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam, fRange, 30.0f);
							DrawTargetArea(pkRenderer, pkCamera, fFrameTime);
						}break;
					case ESArea_Cone:
						{
							m_TargetAreaType = TAT_TRIANGLE;
							//SetTargetAreaTriangle(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam, fRange);
							SetTargetCone(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam, 30.0f, fRange);
							DrawTargetArea(pkRenderer, pkCamera, fFrameTime);
						}break;
					default :
						{
							float const fAreaParam1 = static_cast<float>(pkSkillDef->GetAbil(AT_2ND_AREA_PARAM1));
							float const fAreaParam2 = static_cast<float>(pkSkillDef->GetAbil(AT_2ND_AREA_PARAM2));
							if(0.0f != fAreaParam1 && 0.0f != fAreaParam2)
							{
								//SetTargetAreaRectangle(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam1, fAreaParam2);
								SetTargetCube(pkActor->GetPos(), pkActor->GetLookingDir(), fAreaParam1, fAreaParam2, 30.0f);
								DrawTargetArea(pkRenderer, pkCamera, fFrameTime);
							}
						}break;
					}
				}
			}
		}
	}

	if(m_bDrawTargetMark)
	{
		if(!m_spTargetingNif)
		{
			m_spTargetingNif = g_kNifMan.GetNif("../Data/5_Effect/0_Common/ef_select_01.nif");
			NiTimeController::StartAnimations(m_spTargetingNif);

			if(NULL == m_spTargetingNif)
			{
				NILOG(PGLOG_ERROR, "[PgMonSkillTargetMan] Create TargetingNif is Failed\n");
				return;
			}
		}

		if(!m_spSelectEnemyNif)
		{
			m_spSelectEnemyNif = g_kNifMan.GetNif("../Data/5_Effect/0_Common/ef_select_02.nif");
			NiTimeController::StartAnimations(m_spSelectEnemyNif);

			if(NULL == m_spSelectEnemyNif)
			{
				NILOG(PGLOG_ERROR, "[PgMonSkillTargetMan] Create SelectEnemyNif is Failed\n");
				return;
			}
		}

		PgActionTargetInfo* pkTargetInfo = NULL;
		PgPilot* pkPilot = NULL;
		PgActor* pkActor = NULL;

		//for(ActionTargetList::iterator itor = m_kTargetList.begin(); itor != m_kTargetList.end(); ++itor)
		//{
		//	pkTargetInfo = &(*itor);

		//	pkPilot = g_kPilotMan.FindPilot(pkTargetInfo->GetTargetPilotGUID());
		//	if(!pkPilot) continue;

		//	pkActor =  dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
		//	if(!pkActor) continue;

		//	m_spTargetingNif->SetTranslate(pkActor->GetABVShapeWorldPos(pkTargetInfo->GetSphereIndex()));
		//	m_spTargetingNif->Update(m_fSavedAccumTime);

		//	pkRenderer->PartialRenderClick_Deprecated(m_spTargetingNif);
		//}

		for(ActionTargetList::iterator itor2 = m_kAreaInEnemyList.begin(); itor2 != m_kAreaInEnemyList.end(); ++itor2)
		{
			pkTargetInfo = &(*itor2);

			pkPilot = g_kPilotMan.FindPilot(pkTargetInfo->GetTargetPilotGUID());
			if(!pkPilot) continue;

			pkActor =  dynamic_cast<PgActor*>(pkPilot->GetWorldObject());
			if(!pkActor) continue;

			//NiAVObject *pkDummy = pkActor->GetNIFRoot()->GetObjectByName(ATTACH_POINT_STAR);
			NiAVObject *pkDummy = pkActor->GetNodePointStar();

			if(!pkDummy)
			{
				pkDummy = pkActor->GetCharRoot()->GetObjectByName("char_root");
			}

			if(pkDummy)
			{
				m_spSelectEnemyNif->SetTranslate(pkDummy->GetWorldTranslate());
				m_spSelectEnemyNif->Update(m_fSavedAccumTime);
			}

			pkRenderer->PartialRenderClick_Deprecated(m_spSelectEnemyNif);		
		}

	}
}
bool PgMonSkillTargetMan::CreateDefaultAttackAction()
{
	PgPilot* pkMyPilot = g_kPilotMan.GetPlayerPilot();
	if(!pkMyPilot) return false;

	PgActor* pkActor = dynamic_cast<PgActor *>(pkMyPilot->GetWorldObject());
	if(!pkActor) return false;

	char const* strDefaultActionID = pkMyPilot->FindActionID(ACTIONKEY_ATTACK);
	if(!strDefaultActionID) return false;

	if(m_pkDefaultAttackAction)
	{
		if(m_pkDefaultAttackAction->GetID() == strDefaultActionID) return true;	//	이미 존재함.
		g_kActionPool.ReleaseAction(m_pkDefaultAttackAction);
		m_pkDefaultAttackAction= NULL;
	}

	m_pkDefaultAttackAction = g_kActionPool.CreateAction(strDefaultActionID);
	return	true;
}
void PgMonSkillTargetMan::EnableTargetAreaDisplay(bool const bEnable)
{
	m_bDrawTargetArea = bEnable;
}
void PgMonSkillTargetMan::SetTargetAreaCircle(NiPoint3 const& kCenter, float const fRange)
{
	NiTriStripsPtr	kTriStripGeom = NULL;
	if(!m_spAreaCircleNif)
	{
		m_spAreaCircleNif = g_kNifMan.GetNif_DeepCopy("../Data/5_Effect/0_Common/ef_area_circle.nif");
		if(!m_spAreaCircleNif) return;

		kTriStripGeom = (NiTriStrips*)m_spAreaCircleNif->GetObjectByName("Plane09");
		kTriStripGeom->SetConsistency(NiGeometryData::VOLATILE);

		NiGeometry* pkGeom = (NiGeometry*)m_spAreaCircleNif->GetObjectByName("Plane09");
		if(pkGeom)
		{
			NiMaterialProperty	*m_pkLineMarterial = pkGeom->GetPropertyState()->GetMaterial();
			if(m_pkLineMarterial)
			{
				m_pkLineMarterial->SetAlpha(1.0f);
			}
		}
	}
	else
	{
		kTriStripGeom = (NiTriStrips*)m_spAreaCircleNif->GetObjectByName("Plane09");
	}

	if(!kTriStripGeom) return;

	m_TargetAreaType = TAT_CIRCLE;

	NiPoint3* pkVert = kTriStripGeom->GetVertices();
	NiPoint3 kVector1(-fRange,fRange,0),kVector2(-fRange,-fRange,0) ,kVector3(fRange,fRange,0),kVector4(fRange,-fRange,0);

	*pkVert = kVector1;pkVert->z+=0.1f;	pkVert++;
	*pkVert = kVector2;pkVert->z+=0.1f;	pkVert++;
	*pkVert = kVector3;pkVert->z+=0.1f;	pkVert++;
	*pkVert = kVector4;pkVert->z+=0.1f;	

	kTriStripGeom->GetModelData()->MarkAsChanged(NiGeometryData::VERTEX_MASK);
	m_spAreaCircleNif->SetTranslate(kCenter);
	m_spAreaCircleNif->UpdateWorldBound();
}
void	PgMonSkillTargetMan::SetTargetAreaTriangle(NiPoint3 const& kStart, NiPoint3 const& kDir, float const fAngle, float const fRange)
{
	NiTriStripsPtr	kTriStripGeom = NULL;
	if(!m_spAreaTriNif)
	{
		m_spAreaTriNif = g_kNifMan.GetNif_DeepCopy("../Data/5_Effect/0_Common/ef_area_tri.nif");
		if(!m_spAreaTriNif) return;

		kTriStripGeom = (NiTriStrips*)m_spAreaTriNif->GetObjectByName("Plane09");
		kTriStripGeom->SetConsistency(NiGeometryData::VOLATILE);

		NiGeometry* pkGeom = (NiGeometry*)m_spAreaTriNif->GetObjectByName("Plane09");
		if(pkGeom)
		{
			NiMaterialProperty* m_pkLineMarterial = pkGeom->GetPropertyState()->GetMaterial();
			if(m_pkLineMarterial)
			{
				m_pkLineMarterial->SetAlpha(1.0f);
			}
		}
	}
	else
	{
		kTriStripGeom = (NiTriStrips*)m_spAreaTriNif->GetObjectByName("Plane09");
	}

	if(!kTriStripGeom) return;

	m_TargetAreaType = TAT_TRIANGLE;

	NiPoint3	kVector1,kVector2,kVector3,kVector4;
	NiMatrix3	kRotateMatrix;
	NiQuaternion kRotateQuat(fAngle*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector1);
	kVector1*=fRange;

	NiQuaternion kRotateQuat2(-fAngle*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat2.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector2);
	kVector2*=fRange;

	float const fSideRange = (NiPoint3(kVector1.x,kVector1.y,0) - NiPoint3(kVector2.x,kVector2.y,0)).Length()/2;

	NiQuaternion kRotateQuat3(-90.0f*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat3.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector3);
	kVector3*=fSideRange;

	NiQuaternion kRotateQuat4(90.0f*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat4.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector4);
	kVector4*=fSideRange;

	NiPoint3* pkVert = kTriStripGeom->GetVertices();

	pkVert->x = kVector1.x;	pkVert->y = kVector1.y;	pkVert->z=kVector1.z+0.1f; pkVert++;
	pkVert->x = kVector4.x;	pkVert->y = kVector4.y;	pkVert->z=kVector4.z+0.1f; pkVert++;
	pkVert->x = kVector2.x;	pkVert->y = kVector2.y;	pkVert->z=kVector2.z+0.1f; pkVert++;
	pkVert->x = kVector3.x;	pkVert->y = kVector3.y;	pkVert->z=kVector3.z+0.1f; pkVert++;

	kTriStripGeom->GetModelData()->MarkAsChanged(NiGeometryData::VERTEX_MASK);
	m_spAreaTriNif->SetTranslate(kStart);
	m_spAreaTriNif->UpdateWorldBound();
}

void	PgMonSkillTargetMan::SetTargetAreaRectangle(NiPoint3 const& kStart,NiPoint3 const& kDir,float const fSideRange, float const fRange)
{
	NiTriStripsPtr	kTriStripGeom = NULL;
	if(!m_spAreaRectNif)
	{
		m_spAreaRectNif = g_kNifMan.GetNif_DeepCopy("../Data/5_Effect/0_Common/ef_area_box.nif");
		if(!m_spAreaRectNif) return;

		kTriStripGeom = (NiTriStrips*)m_spAreaRectNif->GetObjectByName("Plane09");
		kTriStripGeom->SetConsistency(NiGeometryData::VOLATILE);

		NiGeometry* pkGeom = (NiGeometry*)m_spAreaRectNif->GetObjectByName("Plane09");
		if(pkGeom)
		{
			NiMaterialProperty	*m_pkLineMarterial = pkGeom->GetPropertyState()->GetMaterial();
			if(m_pkLineMarterial)
			{
				m_pkLineMarterial->SetAlpha(1.0f);
			}
		}
	}
	else
	{
		kTriStripGeom = (NiTriStrips*)m_spAreaRectNif->GetObjectByName("Plane09");
	}

	if(!kTriStripGeom) return;

	m_TargetAreaType = TAT_RECTANGLE;

	NiPoint3	kVector1, kVector2, kVector3, kVector4;
	NiMatrix3	kRotateMatrix;

	NiQuaternion kRotateQuat3(-90.0f*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat3.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector3);
	kVector3*=fSideRange/2;

	NiQuaternion kRotateQuat4(90.0f*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat4.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector4);
	kVector4*=fSideRange/2;

	NiQuaternion kRotateQuat(-90.0f*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector1);
	kVector1*=fSideRange/2;
	kVector1+=kDir*fRange;

	NiQuaternion kRotateQuat2(90.0f*3.141592f/180.0f,NiPoint3(0,0,1));
	kRotateQuat2.ToRotation(kRotateMatrix);
	NiMatrix3::TransformVertices(kRotateMatrix,NiPoint3(0,0,0),1,&kDir,&kVector2);
	kVector2*=fSideRange/2;
	kVector2+=kDir*fRange;

	NiPoint3* pkVert = kTriStripGeom->GetVertices();

	pkVert->x = kVector2.x;	pkVert->y = kVector2.y;	pkVert->z=kVector2.z+0.1f; pkVert++;
	pkVert->x = kVector4.x;	pkVert->y = kVector4.y;	pkVert->z=kVector4.z+0.1f; pkVert++;
	pkVert->x = kVector1.x;	pkVert->y = kVector1.y;	pkVert->z=kVector1.z+0.1f; pkVert++;
	pkVert->x = kVector3.x;	pkVert->y = kVector3.y;	pkVert->z=kVector3.z+0.1f; pkVert++;

	kTriStripGeom->GetModelData()->MarkAsChanged(NiGeometryData::VERTEX_MASK);
	m_spAreaRectNif->SetTranslate(kStart);
	m_spAreaRectNif->UpdateWorldBound();
}

void PgMonSkillTargetMan::UpdateTargetList()
{
	if(0 == m_iUpdateTargetInterval || 0 == m_ulLastTargetUpdateTime || BM::GetTime32()-m_ulLastTargetUpdateTime> (DWORD)m_iUpdateTargetInterval)
	{
		m_ulLastTargetUpdateTime = BM::GetTime32();
	}	
	else
	{
		return;
	}

//	m_kTargetList.clear();
	m_kAreaInEnemyList.clear();

	PgActor* pkActor = g_kPilotMan.GetPlayerActor();
	if(!pkActor) return;

	PgAction* pkCurAction = pkActor->GetAction();
	if(!pkCurAction) return;

	CSkillDef const* pkSkillDef = pkCurAction->GetSkillDef();
	if(!pkSkillDef)
	{
		return;
	}

	float const fMaxRange = 256.0f;
	int const iMaxTarget = 50;
	if(pkCurAction->GetTargetList() && pkCurAction->GetTargetList()->size())
	{
//		m_kTargetList = *(pkCurAction->GetTargetList());
		PgAction::FindTargets(pkCurAction->GetActionNo(), PgAction::TAT_SPHERE, pkActor->GetPos(), pkActor->GetLookingDir(), fMaxRange, 0.0f, fMaxRange, NULL, iMaxTarget, true, m_kAreaInEnemyList, NULL, false);

		return;
	}
	else
	{
		if(EST_ACTIVE != pkSkillDef->GetType())
		{
			if(CreateDefaultAttackAction())
			{
				PG_ASSERT_LOG(m_pkDefaultAttackAction);
				//m_pkDefaultAttackAction->FindTargetFSM(pkActor,m_pkDefaultAttackAction,m_kTargetList);
				PgAction::FindTargets(m_pkDefaultAttackAction->GetActionNo(), PgAction::TAT_SPHERE, pkActor->GetPos(), pkActor->GetLookingDir(), fMaxRange, 0.0f, fMaxRange, NULL, iMaxTarget, true, m_kAreaInEnemyList, NULL, false);
			}
		}
		else
		{
			PG_ASSERT_LOG(pkCurAction);
			//pkCurAction->FindTargetFSM(pkActor,m_pkDefaultAttackAction,m_kTargetList);
			PgAction::FindTargets(pkCurAction->GetActionNo(), PgAction::TAT_SPHERE, pkActor->GetPos(), pkActor->GetLookingDir(), fMaxRange, 0.0f, fMaxRange, NULL, iMaxTarget, true, m_kAreaInEnemyList, NULL, false);
		}
	}
}

void PgMonSkillTargetMan::ClearTargetList()
{
//	m_kTargetList.clear();
	m_kAreaInEnemyList.clear();
}

void PgMonSkillTargetMan::Init()
{
	m_bDrawTargetMark = false;
	m_ulLastTargetUpdateTime = 0;
	m_iUpdateTargetInterval = 0;

	m_spTargetingNif = NULL;
	m_fSavedAccumTime = NULL;

	m_pkDefaultAttackAction = NULL;

	m_spAreaCircleNif = NULL;
	m_spAreaTriNif = NULL;
	m_spAreaRectNif = NULL;

	m_bDrawTargetArea = false;

	//draw cube stuff
	m_pkCubeVertexArray = NiNew NiPoint3[MAX_VERTS];
	m_pkCubeConnect = (NiBool*)NiMalloc(sizeof(NiBool)*MAX_VERTS);
	m_spCubeLine = NiNew NiLines(MAX_VERTS, m_pkCubeVertexArray, NULL, NULL, 0, NiGeometryData::NBT_METHOD_NONE, m_pkCubeConnect);
	m_spCubeLine->IncRefCount();

	m_pkMaterial = NiNew NiMaterialProperty();
	m_pkMaterial->SetAmbientColor(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetDiffuseColor(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetSpecularColor(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetEmittance(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetShineness(0.0f);

	m_spCubeLine->AttachProperty(m_pkMaterial);
	m_spCubeLine->Update(0.0f);
	m_spCubeLine->UpdateEffects();
	m_spCubeLine->UpdateProperties();
	m_spCubeLine->UpdateNodeBound();	

	//draw sphere stuff
	m_pkSphereVertexArray = NiNew NiPoint3[MAX_VERTS];
	m_pkSphereConnect = (NiBool*)NiMalloc(sizeof(NiBool)*MAX_VERTS);
	m_spSphereLine = NiNew NiLines(MAX_VERTS, m_pkSphereVertexArray, NULL, NULL, 0, NiGeometryData::NBT_METHOD_NONE, m_pkSphereConnect);
	m_spSphereLine->IncRefCount();

	m_spSphereLine->AttachProperty(m_pkMaterial);
	m_spSphereLine->Update(0.0f);
	m_spSphereLine->UpdateEffects();
	m_spSphereLine->UpdateProperties();
	m_spSphereLine->UpdateNodeBound();

	//draw cone stuff
	m_pkConeVertexArray = NiNew NiPoint3[MAX_VERTS];
	m_pkConeConnect = (NiBool*)NiMalloc(sizeof(NiBool)*MAX_VERTS);
	m_spConeLine = NiNew NiLines(MAX_VERTS, m_pkConeVertexArray, NULL, NULL, 0, NiGeometryData::NBT_METHOD_NONE, m_pkConeConnect);
	m_spConeLine->IncRefCount();

	m_spConeLine->AttachProperty(m_pkMaterial);
	m_spConeLine->Update(0.0f);
	m_spConeLine->UpdateEffects();
	m_spConeLine->UpdateProperties();
	m_spConeLine->UpdateNodeBound();
}

void PgMonSkillTargetMan::Destroy()
{
	if(m_pkDefaultAttackAction)
	{
		g_kActionPool.ReleaseAction(m_pkDefaultAttackAction);
		m_pkDefaultAttackAction = NULL;
	}	

	m_spTargetingNif = NULL;
	m_spSelectEnemyNif = NULL;

	m_spAreaCircleNif = NULL;
	m_spAreaTriNif = NULL;
	m_spAreaRectNif = NULL;

	ClearTargetList();

	//draw cube stuff
	m_spCubeLine = NULL;
	if(NULL != m_pkCubeVertexArray)
	{
		NiDelete[] m_pkCubeVertexArray;
		m_pkCubeVertexArray = NULL;
	}
	if(NULL != m_pkCubeConnect)
	{
		NiFree(m_pkCubeConnect);
		m_pkCubeConnect = NULL;
	}
	//draw sphere stuff
	m_spSphereLine = NULL;
	if(NULL != m_pkSphereVertexArray)
	{
		NiDelete[] m_pkSphereVertexArray;
		m_pkSphereVertexArray = NULL;
	}
	if(NULL != m_pkSphereConnect)
	{
		NiFree(m_pkSphereConnect);
		m_pkSphereConnect = NULL;
	}
	
	//draw cone stuff
	m_spConeLine = NULL;
	if(NULL != m_pkConeVertexArray)
	{
		NiDelete[] m_pkConeVertexArray;
		m_pkConeVertexArray = NULL;
	}
	if(NULL != m_pkConeConnect)
	{
		NiFree(m_pkConeConnect);
		m_pkConeConnect = NULL;
	}
	
	if(NULL != m_pkMaterial)
	{
		NiDelete m_pkMaterial;
		m_pkMaterial = NULL;
	}
}