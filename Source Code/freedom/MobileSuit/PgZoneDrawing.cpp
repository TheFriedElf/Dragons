

#include "stdafx.h"
#include "PgZoneDrawing.h"
#include "PgRenderer.h"
#include "PgPilotMan.h"

#include "NewWare/Renderer/DrawUtils.h"

#ifndef USE_INB
PgZoneDrawing	g_kZoneDrawer;

PgZoneDrawing::PgZoneDrawing()
:m_pkZoneVertexArray(NULL), m_pkMaterial(NULL), m_pkZoneLine(NULL), m_pkConnect(NULL), m_bZoneDraw(false), m_bZoneWholeDraw(m_bZoneWholeDraw)
{
}

PgZoneDrawing::~PgZoneDrawing()
{
}

void PgZoneDrawing::Init()
{
	m_pkZoneVertexArray = NiNew NiPoint3[16];
	m_pkConnect = (NiBool*)NiMalloc(sizeof(NiBool)*16);
	m_pkZoneLine = NiNew NiLines(16, m_pkZoneVertexArray, NULL, NULL, 0, NiGeometryData::NBT_METHOD_NONE, m_pkConnect);
	m_pkZoneLine->IncRefCount();

	m_pkMaterial = NiNew NiMaterialProperty();
	m_pkMaterial->SetAmbientColor(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetDiffuseColor(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetSpecularColor(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetEmittance(NiColor::NiColor(1.0f, 1.0f, 1.0f));
	m_pkMaterial->SetShineness(0.0f);

	m_pkZoneLine->AttachProperty(m_pkMaterial);
	m_pkZoneLine->Update(0.0f);
	m_pkZoneLine->UpdateEffects();
	m_pkZoneLine->UpdateProperties();
	m_pkZoneLine->UpdateNodeBound();	

	//SetAllConnectivity(m_pkCubeConnect, false);
	//SetRangeConnectivity(m_pkCubeConnect, 0, 7, true);
	//¼ø¼­
	//3   2
	//0   1
}

void PgZoneDrawing::SetZoneInfo(POINT3I const& kMinPos, POINT3I const& kMaxPos, POINT3I const& kZoneSize, POINT3I const& kZoneCount)
{
	m_kMinPos = kMinPos; 
	m_kMaxPos = kMaxPos;
	m_kZoneSize = kZoneSize;
	m_kZoneCount = kZoneCount;
	NiPoint3 kPoint0(0.0f, 0.0f, 0.0f), kPoint1(m_kZoneSize.x, 0.0f, 0.0f), kPoint2(m_kZoneSize.x, m_kZoneSize.y, 0.0f), kPoint3(0.0f, m_kZoneSize.y, 0.0f);
	//NiPoint3 kPoint0(0.0f, 0.0f, 0.0f), kPoint1(20.0f, 0.0f, 0.0f), kPoint2(20.0f, 20.0f, 0.0f), kPoint3(0.0f, 20.0f, 0.0f);

	memset(m_pkConnect, false, sizeof(NiBool)*16);
	memset(m_pkConnect, true, sizeof(NiBool)*7);
	m_pkZoneVertexArray[0] = kPoint0; 
	m_pkZoneVertexArray[1] = kPoint1; 
	m_pkZoneVertexArray[2] = kPoint2; 
	m_pkZoneVertexArray[3] = kPoint3; 
	m_pkZoneVertexArray[4] = kPoint0; 
	m_pkZoneVertexArray[5] = kPoint0; m_pkZoneVertexArray[5].z = m_kZoneSize.z;
	m_pkZoneVertexArray[6] = kPoint1; m_pkZoneVertexArray[6].z = m_kZoneSize.z;
	m_pkZoneVertexArray[7] = kPoint1; 

	memset(m_pkConnect+8, true, sizeof(NiBool)*7);
	m_pkZoneVertexArray[8]  = kPoint3; m_pkZoneVertexArray[8].z = m_kZoneSize.z;
	m_pkZoneVertexArray[9]  = kPoint0; m_pkZoneVertexArray[9].z = m_kZoneSize.z;
	m_pkZoneVertexArray[10] = kPoint1; m_pkZoneVertexArray[10].z = m_kZoneSize.z;
	m_pkZoneVertexArray[11] = kPoint2; m_pkZoneVertexArray[11].z = m_kZoneSize.z;
	m_pkZoneVertexArray[12] = kPoint3; m_pkZoneVertexArray[12].z = m_kZoneSize.z;
	m_pkZoneVertexArray[13] = kPoint3;
	m_pkZoneVertexArray[14] = kPoint2;
	m_pkZoneVertexArray[15] = kPoint2; m_pkZoneVertexArray[15].z = m_kZoneSize.z;
}

void PgZoneDrawing::Destroy()
{
	if(NULL != m_pkZoneVertexArray)
	{
		NiDelete[] m_pkZoneVertexArray;
        m_pkZoneVertexArray = NULL;
	}
	if(NULL != m_pkZoneLine)
	{
		m_pkZoneLine = NULL;
	}
	if(NULL != m_pkConnect)
	{
		NiFree(m_pkConnect);
        m_pkConnect = NULL;
	}
	if(NULL != m_pkMaterial)
	{
		NiDelete m_pkMaterial;
        m_pkMaterial = NULL;
	}
}

void PgZoneDrawing::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{	
	if(false != m_bZoneDraw)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		for(int iX=0; m_kZoneCount.x>iX; ++iX)
		{
			for(int iY=0; m_kZoneCount.x>iY; ++iY)
			{
				for(int iZ=0; m_kZoneCount.x>iZ; ++iZ)
				{
					m_pkZoneLine->SetTranslate(m_kMinPos.x+m_kZoneSize.x*iX, 
						m_kMinPos.y+m_kZoneSize.y*iY, 
						m_kMinPos.z+m_kZoneSize.z*iZ);
					//pkPlayer->GetPos()
					//m_pkZoneLine->SetTranslate(pkPlayer->GetPos().x,pkPlayer->GetPos().y,pkPlayer->GetPos().z);
					m_pkZoneLine->Update(fFrameTime);
					if(false == m_bZoneWholeDraw)
					{
						pkRenderer->PartialRenderClick_Deprecated(m_pkZoneLine);
					}
					else
					{
                        NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), m_pkZoneLine );
					}
				}
			}
		}
	}
}

#endif//USE_INB