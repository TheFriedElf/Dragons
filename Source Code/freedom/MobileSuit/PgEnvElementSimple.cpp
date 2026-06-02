#include "stdafx.h"
#include "PgEnvElementSimple.H"
#include "PgParticleMan.H"
#include "PgParticle.H"
#include "PgWorld.H"
#include "PgRenderer.H"
#include "PgRendererUtil.H"
#include "PgEnvElementFactory.H"

PgEnvElementSimple::PgEnvElementSimple(std::string const kParticleID, float const fRadius)
:m_fBoundRadius(fRadius)
,m_bActivated(false)
,m_iWorldParticleSlotID(-1)
,m_kPreviousCameraPosition(NiPoint3::ZERO)
,m_kParticleID(kParticleID)
{}

PgEnvElementSimple::~PgEnvElementSimple()
{
	Deactivate();
	ReleaseParticle();
}

void PgEnvElementSimple::Update(NiCamera *pkCamera,float fAccumTime,float fFrameTime) 
{
	PgEnvElement::Update(pkCamera,fAccumTime,fFrameTime);

	UpdateParticlePosition(pkCamera);
	UpdateModifier(pkCamera);
}

void PgEnvElementSimple::UpdateParticlePosition(NiCamera *pkCamera)
{
	if(!m_spParticle
		|| NULL == pkCamera)
	{
		return;
	}
	m_spParticle->SetTranslate(pkCamera->GetWorldTranslate());
}

void PgEnvElementSimple::SetValue(float fIntensity,float fTransitTime,float fAccumTime)
{
	PgEnvElement::SetValue(fIntensity,fTransitTime,fAccumTime);

}

void PgEnvElementSimple::Activate()
{
	if(GetActivated())
	{
		return;
	}
	SetActivated(true);
	CreateParticle();
	AttachParticleToWorld();
}

void PgEnvElementSimple::Deactivate()
{
	if(!GetActivated())
	{
		return;
	}
	SetActivated(false);
	DetachParticleFromWorld();
}

void PgEnvElementSimple::AttachParticleToWorld()
{
	if(!g_pkWorld)
	{
		return;
	}

	m_iWorldParticleSlotID = g_pkWorld->AttachParticle(m_spParticle,NiPoint3::ZERO);
}

void PgEnvElementSimple::DetachParticleFromWorld()
{
	if(g_pkWorld)
	{
		g_pkWorld->DetachParticle(m_iWorldParticleSlotID);
	}
	m_iWorldParticleSlotID = -1;
}

void PgEnvElementSimple::SetIntensity(float fIntensity)
{
	PgEnvElement::SetIntensity(fIntensity);
	if(0 < GetIntensity())
	{
		Activate();
	}
	else
	{
		Deactivate();
	}
}

void PgEnvElementSimple::CreateParticle()
{
	if(m_spParticle
		|| m_kParticleID.empty()
		)
	{
		return;
	}
	ReleaseParticle();

	m_spParticle = NiDynamicCast(NiNode,g_kParticleMan.GetParticle(m_kParticleID.c_str()));
	if(!m_spParticle)
	{
		return;
	}

	PgRendererUtil::RemoveParticleSystemModifierRecursive<NiPSysAgeDeathModifier>(m_spParticle);
	PgRenderer::RunUpParticleSystem(m_spParticle);

	ModifyParticleRecursive(m_spParticle);
}

void PgEnvElementSimple::ReleaseParticle()
{
	m_spParticle = 0;
	m_kModifierInfoCont.clear();
}

void PgEnvElementSimple::ModifyParticleRecursive(NiAVObject *pkAVObject)
{
	if(!pkAVObject)
	{
		return;
	}

	NiNode *pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int const iTotalChild = pkNode->GetArrayCount();
		for(int i=0;i<iTotalChild;++i)
		{
			ModifyParticleRecursive(pkNode->GetAt(i));
		}
	}

	NiParticleSystem *pkParticleSystem = NiDynamicCast(NiParticleSystem,pkAVObject);
	if(pkParticleSystem)
	{
		ModifyParticleSystem(pkParticleSystem);
	}
}

void PgEnvElementSimple::ModifyParticleSystem(NiParticleSystem *pkParticleSystem)
{
	RemoveRedundantModifiers(pkParticleSystem);
	PgRendererUtil::RemoveController<NiPSysEmitterCtlr>(pkParticleSystem);

	PgPSysBoundedPositionModifier* pkBoundedPositionModifier = AppendBoundedPositionModifier(pkParticleSystem);
	PgPSysColorModifierDecorator *pkColorModifierDecorator = AppendColorModifierDecorator(pkParticleSystem);

	float fPositionBoundRadius = GetPositionBoundRadius(pkParticleSystem)/2;
	pkBoundedPositionModifier->SetBoundRadius(fPositionBoundRadius);
	AddModifierInfo(pkBoundedPositionModifier,pkColorModifierDecorator);
}

void PgEnvElementSimple::AddModifierInfo(PgPSysBoundedPositionModifier *pkPositionModifier, PgPSysColorModifierDecorator *pkColorModifierDecorator)
{
	m_kModifierInfoCont.push_back(stModifierInfo(pkPositionModifier,pkColorModifierDecorator));
}

PgPSysColorModifierDecorator* PgEnvElementSimple::AppendColorModifierDecorator(NiParticleSystem *pkParticleSystem)
{
	NiPSysColorModifier *pkColorModifier = PgRendererUtil::FindParticleSystemModifier<NiPSysColorModifier>(pkParticleSystem);
	PgPSysColorModifierDecorator *pkColorModifierDecorator = NiNew PgPSysColorModifierDecorator(
		"Color Modifier Decorator",
		pkColorModifier);

	if(pkColorModifier)
	{
		pkParticleSystem->RemoveModifier(pkColorModifier);
	}

	pkParticleSystem->AddModifier(pkColorModifierDecorator);

	return pkColorModifierDecorator;
}

float PgEnvElementSimple::GetPositionBoundRadius(NiParticleSystem *pkParticleSystem)
{
	NiPSysBoxEmitter *pkBoxEmitter = PgRendererUtil::FindParticleSystemModifier<NiPSysBoxEmitter>(pkParticleSystem);
	if(pkBoxEmitter)
	{
		return pkBoxEmitter->GetEmitterWidth();
	}
	return 0;
}

PgPSysBoundedPositionModifier* PgEnvElementSimple::AppendBoundedPositionModifier(NiParticleSystem *pkParticleSystem)
{
	PgPSysBoundedPositionModifier *pkBoundedPositionModifier = NiNew PgPSysBoundedPositionModifier("Bounded Position Modifier");
	pkParticleSystem->AddModifier(pkBoundedPositionModifier);
	return pkBoundedPositionModifier;
}

void PgEnvElementSimple::RemoveRedundantModifiers(NiParticleSystem *pkParticleSystem)
{
	PgRendererUtil::RemoveParticleSystemModifier<NiPSysAgeDeathModifier>(pkParticleSystem);
	PgRendererUtil::RemoveParticleSystemModifier<NiPSysPositionModifier>(pkParticleSystem);
}

void PgEnvElementSimple::UpdateModifier(NiCamera *pkCamera)
{

	int const iTotal = m_kModifierInfoCont.size();
	for(int i=0;i<iTotal;++i)
	{
		UpdateModifier(pkCamera,m_kModifierInfoCont[i]);
	}

}

void PgEnvElementSimple::UpdateModifier(NiCamera *pkCamera,stModifierInfo &kModifierInfo)
{
	kModifierInfo.m_spPositionModifier->SetBoundCenter( pkCamera->GetWorldTranslate() );
	kModifierInfo.m_spColorModifierDecorator->SetColor( NiColorA(1.0f,1.0f,1.0f,GetIntensity()) );
}
