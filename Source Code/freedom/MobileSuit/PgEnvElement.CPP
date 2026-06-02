#include "stdafx.h"
#include "PgEnvElement.H"

PgEnvElement::PgEnvElement()
:m_fIntensity(0.0f)
,m_fTransitStartIntensity(0.0f)
,m_fTargetIntensity(0.0f)
,m_fTransitStartTime(0.0f)
,m_fTransitTotalTime(0.0f)
{
}

PgEnvElement::~PgEnvElement()
{
}

void PgEnvElement::Update(NiCamera *pkCamera, float fAccumTime, float fFrameTime)
{
	UpdateIntensityTransition(fAccumTime,fFrameTime);
}

void PgEnvElement::SetValue(float fIntensity, float fTransitTime, float fAccumTime)
{
	m_fTransitStartIntensity = m_fIntensity;
	m_fTargetIntensity = fIntensity;
	m_fTransitStartTime = fAccumTime;
	m_fTransitTotalTime = fTransitTime;

	if(fTransitTime == 0)
	{
		SetIntensity(fIntensity);
		return;
	}
}

float PgEnvElement::GetIntensity() const
{
	return	m_fIntensity;
}

void PgEnvElement::SetIntensity(float fIntensity)
{
	m_fIntensity = fIntensity;
}

void PgEnvElement::UpdateIntensityTransition(float fAccumTime, float fFrameTime)
{
	if(m_fTargetIntensity == m_fIntensity)
	{
		return;
	}

	float	fElapsedTime = fAccumTime - m_fTransitStartTime;
	float	fRatio = fElapsedTime / m_fTransitTotalTime;
	if(fRatio>1)
	{
		fRatio = 1;
	}

	float	fNewIntensity = m_fTransitStartIntensity + (m_fTargetIntensity - m_fTransitStartIntensity)*fRatio;
	SetIntensity(fNewIntensity);
}