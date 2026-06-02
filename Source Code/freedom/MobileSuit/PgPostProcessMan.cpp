#include "StdAfx.h"
#include "PgPostProcessMan.h"
#include "PgIPostProcessEffect.h"
#include "PgRenderer.h"
#include "HDRScene.h"
#include "BrightBloom.h"
#include "PgDepthOfField.H"
#include "PgFSAA2.H"
#include "PgRadialBlur.H"

PgPostProcessMan::~PgPostProcessMan()
{
	RemoveAllEffects();
}

bool PgPostProcessMan::AddEffect(PgIPostProcessEffect* pkEffect)
{
	assert(pkEffect);

	bool bSuccess = pkEffect->CreateEffect();
	if (bSuccess)
	{
		m_kEffects.Add(pkEffect);
	}

	return bSuccess;
}
bool	PgPostProcessMan::AddEffect(PgIPostProcessEffect::EFFECT_TYPE type)
{
	PgIPostProcessEffect *pNewEffect = 0;
	switch(type)
	{
	case PgIPostProcessEffect::HDR:
		{
			pNewEffect = (PgIPostProcessEffect*)NiNew HDRScene(true);
		}
		break;
	case PgIPostProcessEffect::DEPTH_OF_FIELD:
		{
			pNewEffect = (PgIPostProcessEffect*)NiNew PgDepthOfField(true);
		}
		break;
	case PgIPostProcessEffect::BRIGHT_BLOOM:
		{
			pNewEffect = (PgIPostProcessEffect*)NiNew BrightBloom(true);
		}
		break;

	case PgIPostProcessEffect::FSAA:
		{
			pNewEffect = (PgIPostProcessEffect*)NiNew PgFSAA2(true);
		}
		break;
	case PgIPostProcessEffect::RADIAL_BLUR:
		{
			pNewEffect = (PgIPostProcessEffect*)NiNew PgRadialBlur(true);
		}
		break;
	}

	if(pNewEffect)
	{
		if(!AddEffect(pNewEffect))
		{
			NiDelete pNewEffect;
			pNewEffect = NULL;
			return	false;
		}
	}
	else
		return	false;

	return true;
}

unsigned int PgPostProcessMan::GetNumEffects() const
{
	return m_kEffects.GetSize();
}

PgIPostProcessEffect* PgPostProcessMan::GetEffectAt(unsigned int uiIndex) const
{
	if (uiIndex < m_kEffects.GetSize())
	{
		return m_kEffects.GetAt(uiIndex);
	}

	return 0;
}

PgIPostProcessEffect* PgPostProcessMan::GetEffect(PgIPostProcessEffect::EFFECT_TYPE type) const
{
	PgIPostProcessEffect* pkEffect;
	for (unsigned int ui = 0; ui < m_kEffects.GetSize(); ui++)
	{
		pkEffect = m_kEffects.GetAt(ui);
		if(pkEffect->GetType() == type) return pkEffect;
	}
	return 0;
}

bool PgPostProcessMan::GetEffectActivated(PgIPostProcessEffect::EFFECT_TYPE type) const
{
	PgIPostProcessEffect* pkEffect = NULL;
	pkEffect = GetEffect(type);
	if (pkEffect == NULL)
		return false;

	return pkEffect->GetActive();
}

void PgPostProcessMan::RemoveEffectAt(unsigned int uiIndex)
{
	if (uiIndex < m_kEffects.GetSize())
	{
		PgIPostProcessEffect* pkEffect = m_kEffects.GetAt(uiIndex);
		m_kEffects.OrderedRemoveAt(uiIndex);
		pkEffect->CleanupEffect();
		SAFE_DELETE_NI(pkEffect);
	}
}

void PgPostProcessMan::RemoveAllEffects()
{
	for (unsigned int ui = 0; ui < m_kEffects.GetSize(); ui++)
	{
		PgIPostProcessEffect* pkEffect = m_kEffects.GetAt(ui);
		pkEffect->CleanupEffect();
		SAFE_DELETE_NI(pkEffect);
	}
	m_kEffects.RemoveAll();
}

void PgPostProcessMan::OnRenderFrameStart( float fTime, PgRenderer* pkRenderer, unsigned int uiClearMode )
{
	bool bActiveEffect = false;
	for ( unsigned int ui = 0; ui < m_kEffects.GetSize(); ++ui )
	{
		if ( m_kEffects.GetAt(ui)->GetActive() )
		{
			NiRenderTargetGroup* pkRenderTarget = m_kEffects.GetAt(ui)->GetRenderTargetGroup();
			assert( pkRenderTarget );
			pkRenderer->BeginUsingRenderTargetGroup( pkRenderTarget, uiClearMode );
			bActiveEffect = true;
			m_pkCurrRenderTargetGroup = pkRenderTarget;
			m_pkCurrRenderTargetTexture = m_kEffects.GetAt(ui)->GetTargetTexture();
			break;
		}
	}

	if ( !bActiveEffect )
	{
		m_pkCurrRenderTargetGroup = pkRenderer->GetDefaultRenderTargetGroup();
		m_pkCurrRenderTargetTexture = 0;
		pkRenderer->BeginUsingDefaultRenderTargetGroup( uiClearMode );
	}
}

void PgPostProcessMan::OnRenderFrameEnd( float fTime, PgRenderer* pkRenderer )
{
	NiRenderTargetGroup* pkDefaultRenderTarget = pkRenderer->GetDefaultRenderTargetGroup();

	for (unsigned int ui = 0; ui < m_kEffects.GetSize(); ui++)
	{
		if (m_kEffects.GetAt(ui)->GetActive())
		{
			if (pkRenderer->IsRenderTargetGroupActive())
				pkRenderer->EndUsingRenderTargetGroup();

			NiRenderTargetGroup* pkRenderTarget = pkDefaultRenderTarget;
			NiRenderedTexture* pkTargetTexture = 0; // FB is not a texture

			unsigned int uiNext;
			for (uiNext = ui + 1; uiNext < m_kEffects.GetSize(); uiNext++)
			{
				if (m_kEffects.GetAt(uiNext)->GetActive())
				{
					break;
				}
			}
			if (uiNext < m_kEffects.GetSize())
			{
				pkRenderTarget = m_kEffects.GetAt(uiNext)->
					GetRenderTargetGroup();
				assert(pkRenderTarget);

				pkTargetTexture = m_kEffects.GetAt(uiNext)->
					GetTargetTexture();
			}
			m_kEffects.GetAt(ui)->RenderFrame(fTime, pkRenderer, pkRenderTarget,
				pkTargetTexture);
		}
	}

	if (pkRenderer->IsRenderTargetGroupActive())
	{
		pkRenderer->EndUsingRenderTargetGroup();
	}
}

NiRenderTargetGroup* PgPostProcessMan::GetFirstRenderTargetGroup()
{
	return m_pkCurrRenderTargetGroup;
}

NiRenderedTexture* PgPostProcessMan::GetFirstRenderedTexture()
{
	return m_pkCurrRenderTargetTexture;
}