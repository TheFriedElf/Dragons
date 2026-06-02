#include "Stdafx.H"
#include "PgSea.H"
#include "PgNifMan.H"
#include "PgActor.H"
#include "PgPilot.H"
#include "PgPilotMan.H"

void	PgSea::Init()
{
	m_spSeaNode = g_kNifMan.GetNif("../Data/5_Effect/0_Common/SeaPlane.Nif");

	unsigned	int	uiWidth = 1024;
	unsigned	int	uiHeight = 1024;

	NiRenderer	*pkRenderer = NiRenderer::GetRenderer();

	m_spRT_UnderWater = NiRenderedTexture::Create(uiWidth, uiHeight, pkRenderer);
	m_spRT_UponWater = NiRenderedTexture::Create(uiWidth, uiHeight, pkRenderer);

	m_spRTG_UnderWater = NiRenderTargetGroup::Create(m_spRT_UnderWater->GetBuffer(), pkRenderer, true, true);;
	m_spRTG_UponWater = NiRenderTargetGroup::Create(m_spRT_UponWater->GetBuffer(), pkRenderer, true, true);;


	NiGeometry *pGeom = (NiGeometry*)m_spSeaNode->GetObjectByName("Plane01");

	pGeom->GetPropertyState()->GetTexturing()->SetShaderMap(0,NiNew NiTexturingProperty::ShaderMap(m_spRT_UnderWater,0));
	pGeom->GetPropertyState()->GetTexturing()->SetShaderMap(1,NiNew NiTexturingProperty::ShaderMap(m_spRT_UponWater,0));
}
void	PgSea::Destroy()
{
	m_spSeaNode = 0;

	m_spRT_UnderWater = 0;
	m_spRT_UponWater = 0;

	m_spRTG_UnderWater = 0;
	m_spRTG_UponWater = 0;
}
void	PgSea::Render(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{

	PgPilot *pPilot = g_kPilotMan.GetPlayerPilot();
	if(!pPilot) return;

	PgActor	*pActor = pPilot->GetActor();
	if(!pActor) return;

	NxExtendedVec3 vPos = pActor->m_pkController->getPosition();

	m_spSeaNode->SetTranslate(vPos.x,vPos.y,vPos.z);
	m_spSeaNode->Update(0);
	

	NiGeometry *pGeom = (NiGeometry*)m_spSeaNode->GetObjectByName("Plane01");

	pkRenderer->GetRenderer()->EndUsingRenderTargetGroup();

	const NiRenderTargetGroup	*pRTG_Backup = pkRenderer->GetRenderer()->GetCurrentRenderTargetGroup();

	//	먼저 물밑 장면을 렌더링 한다.

	//	카메라를 그냥 놔두고 클리핑 평면을 물의 높이에 해당하는 평면으로 설정한다.
	//	클리핑 평면
	NiPlane	ClipPlane(NiPoint3(0,0,1),NiPoint3(vPos.x,vPos.y,vPos.z));

	float	fPlane[4]={0,0,1,ClipPlane.GetConstant()};

	NiDX9Renderer	*pDX9Renderer = (NiDX9Renderer*)pkRenderer->GetRenderer();
	pDX9Renderer->GetD3DDevice()->SetRenderState(D3DRS_CLIPPLANEENABLE,D3DCLIPPLANE0);
	pDX9Renderer->GetD3DDevice()->SetClipPlane(0,fPlane);

	//	렌더 타겟 설정
//	pkRenderer->BeginUsingRenderTargetGroup(m_spRT_UnderWater,NiRenderer::CLEAR_ALL);




	pDX9Renderer->GetD3DDevice()->SetRenderState(D3DRS_CLIPPLANEENABLE,0);
	pkRenderer->BeginUsingRenderTargetGroup((NiRenderTargetGroup*)pRTG_Backup,NiRenderer::CLEAR_NONE);

	pGeom->RenderImmediate(pkRenderer->GetRenderer());
}
