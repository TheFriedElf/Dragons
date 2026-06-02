#include "StdAfx.h"
#include "PgIPostProcessEffect.h"

PgIPostProcessEffect::PgIPostProcessEffect(bool bActive) : m_bActive(bActive)
{
}

PgIPostProcessEffect::~PgIPostProcessEffect()
{
}

bool PgIPostProcessEffect::GetActive() const
{
    return m_bActive;
}

void PgIPostProcessEffect::SetActive(bool bActive)
{
    m_bActive = bActive;
}
void	PgIPostProcessEffect::SetParam(char const *ParamName,float fValue)
{
}
float PgIPostProcessEffect::GetParam(char const *ParamName)
{
	return 0;
}
