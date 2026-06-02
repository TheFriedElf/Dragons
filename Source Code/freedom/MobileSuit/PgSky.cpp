#include "stdafx.h"
#include "PgSky.h"
#include "PgNifMan.h"
#include "PgRenderer.h"

#include "NewWare/Scene/ApplyTraversal.h"


void	PgSky::DrawImmediate(NiCameraPtr	spCamera,PgRenderer *pkRenderer, float fFrameTime)
{

	//	스카이돔을 카메라 위치로 이동시킨다.
	m_spSkyDome->SetTranslate(spCamera->GetWorldLocation());


	m_spSkyDome->Update(0);

	m_fMovement+=fFrameTime;
	NiShaderFactory::UpdateGlobalShaderConstant(NiFixedString("g_fCloud_Move_Factor"),sizeof(float), &m_fMovement);


	m_pkSkyDomeGeom->RenderImmediate(pkRenderer->GetRenderer());
}

void	PgSky::Init()
{
	for(int i=0;i<MAX_SKY_CLOUD_TEX;i++)
		m_spCloudTex[i] = 0;

	m_spSkyDome = 0;
	m_pkSkyDomeGeom = 0;

	m_fMovement= ((float)BM::Rand_Index(100)/100.0f);

	Load_Res();
		
}
void	PgSky::Destroy()
{
	for(int i=0;i<MAX_SKY_CLOUD_TEX;i++)
		m_spCloudTex[i] = 0;

	m_spSkyDome = 0;
	m_pkSkyDomeGeom = 0;
}

void	PgSky::Load_Res()
{
	m_spSkyDome = g_kNifMan.GetNif("../Data/5_Effect/0_Common/SkyDome.Nif");
	m_pkSkyDomeGeom = (NiGeometry*)m_spSkyDome->GetObjectByName("Sphere01");

	NiPoint2	*pUV = m_pkSkyDomeGeom->GetTextures();

	NiShaderPtr spCloudSky = NiShaderFactory::GetInstance()->RetrieveShader("CloudSky", NiShader::DEFAULT_IMPLEMENTATION, true);
	NiSingleShaderMaterial* pkMaterial = NiSingleShaderMaterial::Create(spCloudSky);
	m_pkSkyDomeGeom->ApplyAndSetActiveMaterial(pkMaterial);

	NiTexturingProperty *pTexturing = m_pkSkyDomeGeom->GetPropertyState()->GetTexturing();
	pTexturing->SetBaseMap(0);

	char	strCloudTexName[512];
	for(int i=0;i<MAX_SKY_CLOUD_TEX;i++)
	{
		sprintf_s(strCloudTexName,512,"../Data/5_Effect/9_Tex/CLOUD_LEVEL%d.dds",i);
		m_spCloudTex[i] = g_kNifMan.GetTexture(strCloudTexName);

		pTexturing->SetShaderMap(i, NiNew NiTexturingProperty::ShaderMap(m_spCloudTex[i], 0));
	}

	pTexturing->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
	pTexturing->SetBaseFilterMode(NiTexturingProperty::FILTER_NEAREST);

	m_spSkyDome->UpdateEffects();
    m_spSkyDome->UpdateProperties();
    m_spSkyDome->Update(0.0f);

    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( m_spSkyDome, false );
}
