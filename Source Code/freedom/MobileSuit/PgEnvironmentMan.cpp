#include "stdafx.h"
#include "PgEnvironmentMan.H"
#include "PgEnvElementFactory.H"
#include "PgEnvElementSnow.H"
#include "PgEnvElementSakura.H"

void	PgEnvironmentMan::SetEnvElementValue(int iEnvStateSetID,PgEnvElement::ENV_ELEMENT_TYPE kEnvElementType,float fIntensity, float fTransitTime,float fAccumTime)
{
	PgEnvStateSet	*pkStateSet = GetEnvStateSet(iEnvStateSetID);
	if(!pkStateSet)
	{
		return;
	}

	pkStateSet->SetEnvElementValue(kEnvElementType,fIntensity,fTransitTime,fAccumTime);

}
bool	PgEnvironmentMan::SetActiveEnvStateSet(int iEnvStateSetID,float fTransitTime,float fAccumTime)
{
	if(m_iActivatedStateSetID == iEnvStateSetID)
	{
		return	true;
	}

	PgEnvStateSet	*pkStateSet = GetEnvStateSet(iEnvStateSetID);
	if(!pkStateSet)
	{
		return false;
	}

	PgEnvStateSet	*pkPrev = GetEnvStateSet(m_iActivatedStateSetID);

	if(pkStateSet->SetActive(pkPrev,fTransitTime,fAccumTime))
	{
		m_iActivatedStateSetID = iEnvStateSetID;
		return	true;
	}

	return	false;
}
int		PgEnvironmentMan::AddEnvStateSet()
{
	PgEnvStateSet	*pkNew = new PgEnvStateSet(GenerateNextStateSetID());
	m_kEnvStateSetStack.push_back(pkNew);

	return	pkNew->GetStateSetID();
}
int	PgEnvironmentMan::GenerateNextStateSetID()
{
	static	int	iStateSetIDGenerator = 0;

	return	iStateSetIDGenerator++;
}
bool	PgEnvironmentMan::ReleaseEnvStateSet(int iEnvStateSetID)
{
	if(m_iActivatedStateSetID == iEnvStateSetID)
	{
		return	false;
	}

	for(EnvStateSetCont::iterator itor = m_kEnvStateSetStack.begin(); itor != m_kEnvStateSetStack.end(); itor++)
	{
		PgEnvStateSet	*pkSet = *itor;
		if(pkSet->GetStateSetID() == iEnvStateSetID)
		{
			SAFE_DELETE(pkSet);
			m_kEnvStateSetStack.erase(itor);
			return	true;
		}
	}

	return	true;
}
void	PgEnvironmentMan::ReleaseAllEnvStateSet()
{
	m_iActivatedStateSetID = -1;
	for(EnvStateSetCont::iterator itor = m_kEnvStateSetStack.begin(); itor != m_kEnvStateSetStack.end(); itor++)
	{
		PgEnvStateSet	*pkSet = *itor;
		SAFE_DELETE(pkSet);
	}

	m_kEnvStateSetStack.clear();

}
void	PgEnvironmentMan::Update(NiCamera *pkCamera,float fAccumTime,float fFrameTime)
{
	for(EnvStateSetCont::iterator itor = m_kEnvStateSetStack.begin(); itor != m_kEnvStateSetStack.end(); itor++)
	{
		PgEnvStateSet	*pkSet = *itor;
		if(pkSet)
		{
			pkSet->Update(pkCamera,fAccumTime,fFrameTime);
		}
	}

}
void	PgEnvironmentMan::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	for(EnvStateSetCont::iterator itor = m_kEnvStateSetStack.begin(); itor != m_kEnvStateSetStack.end(); itor++)
	{
		PgEnvStateSet	*pkSet = *itor;
		if(pkSet)
		{
			pkSet->DrawImmediate(pkRenderer,pkCamera,fFrameTime);
		}
	}
}
void	PgEnvironmentMan::Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	for(EnvStateSetCont::iterator itor = m_kEnvStateSetStack.begin(); itor != m_kEnvStateSetStack.end(); itor++)
	{
		PgEnvStateSet	*pkSet = *itor;
		if(pkSet)
		{
			pkSet->Draw(pkRenderer,pkCamera,fFrameTime);
		}
	}
}

void	PgEnvironmentMan::Init()
{
	m_iActivatedStateSetID = INVALID_ENVSTATESET_ID;

	REGISTER_ENVELEMENT_TO_FACTORY(PgEnvElementSnow);
	REGISTER_ENVELEMENT_TO_FACTORY(PgEnvElementSakura);
	REGISTER_ENVELEMENT_TO_FACTORY(PgEnvElementSunflower);
	REGISTER_ENVELEMENT_TO_FACTORY(PgEnvElementIceCream);
}

void	PgEnvironmentMan::Terminate()
{
	ReleaseAllEnvStateSet();
}

PgEnvStateSet*	PgEnvironmentMan::GetEnvStateSet(int iEnvStateSetID)
{
	for(EnvStateSetCont::iterator itor = m_kEnvStateSetStack.begin(); itor != m_kEnvStateSetStack.end(); itor++)
	{
		PgEnvStateSet	*pkSet = *itor;
		if(pkSet->GetStateSetID() == iEnvStateSetID)
		{
			return	pkSet;
		}
	}

	return	NULL;
}