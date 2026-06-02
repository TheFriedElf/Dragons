#include "StdAfx.h"
#include "PgNifParticle.h"
#include "PgMobileSuit.h"
#include "PgWorld.H"
#include "PgPIlotMan.H"
#include "PgParticleMan.H"
#include "PgRenderer.h"
#include "PgSoundMan.H"

#include "NewWare/Scene/ApplyTraversal.h"


NiImplementRTTI(PgNifParticle, PgParticle);

void	PgNifParticle::CheckFinish(float fAccumTime,float fFrameTime)
{
	if(IsFinished())
	{
		return;
	}

	if(GetNow()>GetEnd() && false == IsLoop())
	{
		SetFinished(true);
	}

	CheckAliveTime(fFrameTime);
}
bool PgNifParticle::Create(NiAVObject *pkParticleRoot,std::string const &kID,stParticleInfo const &kParticleInfo)
{
	if(!pkParticleRoot)
	{
		return	false;
	}

	pkParticleRoot->SetTranslate(0, 0, 0);	
	
	AttachChild(pkParticleRoot, true);

	UpdateProperties();
	UpdateNodeBound();
	UpdateEffects();
	NiAVObject::Update(0.0f, true);

    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( this, false );
	
	ApplyParticleInfo(kParticleInfo);
	SetID(kID.c_str());

	Analyze(this);

	return true;
}

void PgNifParticle::Analyze(NiObjectNET* pkRoot)
{
	if(pkRoot == 0)
	{
		return;
	}

	NiTimeController* pkController = pkRoot->GetControllers();

	// Gather all time controllers from this object
	while(pkController != 0)
	{
		PgNifParticle *pkParticle = NiDynamicCast(PgNifParticle, pkRoot);
		//float fBegin = pkController->GetBeginKeyTime();
		float fEnd = pkController->GetEndKeyTime();
		
		if(m_fEnd < fEnd)
		{
			m_fEnd = fEnd;
		}

		if (m_bLoop && pkController->GetCycleType() != NiTimeController::LOOP)
		{
			if (NiIsKindOf(NiInterpController, pkController) && GetName().Contains("colorshadow"))
			{
				pkController->SetCycleType(NiTimeController::LOOP);
				NILOG(PGLOG_WARNING, "%s particle %s controller is not loop\n", GetName(), pkController->GetRTTI()->GetName());
			}
		}
		pkController = pkController->GetNext();
	}

	if(NiIsKindOf(NiAVObject, pkRoot))
	{
		// NiProperties can have time controllers, so search them too
		NiAVObject* pkObj = (NiAVObject*) pkRoot;
		NiPropertyList* pkPropList = &(pkObj->GetPropertyList());
		NiTListIterator kIter = pkPropList->GetHeadPos();
		while(pkPropList != 0 && !pkPropList->IsEmpty() && kIter)
		{
			NiProperty* pkProperty = pkPropList->GetNext(kIter);
			if(pkProperty)
			{
				if (pkProperty->Type() == NiProperty::ALPHA)
				{
					NiAlphaProperty* pkAlpha = (NiAlphaProperty*)pkProperty;
					if (pkAlpha->GetAlphaBlending() && pkAlpha->GetDestBlendMode() == NiAlphaProperty::ALPHA_DESTALPHA)
					{
						pkAlpha->SetDestBlendMode(NiAlphaProperty::ALPHA_ONE);
						NILOG(PGLOG_WARNING, "[PgNifParticle] %s, %s effect has dest alpha property.\n", GetID().c_str(), pkRoot->GetName());
					}
				}
				Analyze(pkProperty);
			}
		}
	}

	if(NiIsKindOf(NiNode, pkRoot))
	{
		NiNode* pkNode = (NiNode*) pkRoot;
		// Search all of the children
		for(unsigned int ui = 0; ui < pkNode->GetArrayCount(); ui++)
		{
			NiAVObject* pkObj = pkNode->GetAt(ui);
			Analyze(pkObj);
		}
		// NiDynamicEffects can have time controllers, so search them too
		const NiDynamicEffectList* pkEffectList= &(pkNode->GetEffectList());

		NiTListIterator kIter = pkEffectList->GetHeadPos();
		while(pkEffectList != 0 && !pkEffectList->IsEmpty() && kIter)
		{
			NiDynamicEffect* pkEffect = pkEffectList->GetNext(kIter);
			if(pkEffect)
			{
				Analyze(pkEffect);
			}
		}
	}
}

NiObject* PgNifParticle::CreateClone(NiCloningProcess& kCloning)
{
    PgNifParticle* pkClone = NiNew PgNifParticle;
    CopyMembers(pkClone , kCloning);
    return pkClone;
}
void PgNifParticle::CopyMembers(PgNifParticle* pDest, NiCloningProcess& kCloning)
{
	PgParticle::CopyMembers(pDest,kCloning);

	pDest->m_fEnd = m_fEnd;
}
