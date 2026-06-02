#include "stdafx.h"
#include "PgEnvStateSet.H"

void	PgEnvStateSet::SetEnvElementValue(PgEnvElement::ENV_ELEMENT_TYPE kEnvElementType,float fIntensity, float fTransitTime,float fAccumTime)
{
	PgEnvElement	*pkElement = GetEnvElement(kEnvElementType);
	if(!pkElement)
	{
		return;
	}

	pkElement->SetValue(fIntensity,fTransitTime,fAccumTime);
}
bool	PgEnvStateSet::SetActive(PgEnvStateSet* pkPrevActiveSet, float fTransitTime,float fAccumTime)
{


	return	true;
}

void	PgEnvStateSet::AddEnvElement(PgEnvElement *pkElement)
{
	if(!pkElement)
	{
		return;
	}

	PgEnvElement::ENV_ELEMENT_TYPE	kType = pkElement->GetType();

	//	동일한 타입의 Element 가 이미 존재한다면, 기존 것은 삭제해버린다.
	int	iTotal =m_kEnvElementVec.size();
	for(int i=0;i<iTotal;i++)
	{
		if(m_kEnvElementVec[i]->GetType() == kType)
		{
			m_kEnvElementVec[i] = pkElement;
			return;
		}
	}

	m_kEnvElementVec.push_back(pkElement);
		
}
PgEnvElement*	PgEnvStateSet::GetEnvElement(PgEnvElement::ENV_ELEMENT_TYPE kElementTypeID)	const
{
	int	iTotal =m_kEnvElementVec.size();
	for(int i=0;i<iTotal;i++)
	{
		if(m_kEnvElementVec[i]->GetType() == kElementTypeID)
		{
			return m_kEnvElementVec[i];
		}
	}

	return	NULL;
}

void	PgEnvStateSet::Update(NiCamera *pkCamera,float fAccumTime,float fFrameTime)
{
	int	iTotal =m_kEnvElementVec.size();
	for(int i=0;i<iTotal;i++)
	{
		m_kEnvElementVec[i]->Update(pkCamera,fAccumTime,fFrameTime);
	}
}
void	PgEnvStateSet::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	int	iTotal =m_kEnvElementVec.size();
	for(int i=0;i<iTotal;i++)
	{
		m_kEnvElementVec[i]->DrawImmediate(pkRenderer,pkCamera,fFrameTime);
	}
}
void	PgEnvStateSet::Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	int	iTotal =m_kEnvElementVec.size();
	for(int i=0;i<iTotal;i++)
	{
		m_kEnvElementVec[i]->Draw(pkRenderer,pkCamera,fFrameTime);
	}
}

void	PgEnvStateSet::Init()
{
}
void	PgEnvStateSet::Terminate()
{
	m_kEnvElementVec.clear();
}