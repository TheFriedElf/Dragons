#include "StdAfx.h"
#include "PgParticleMan.h"
#include "PgPilotMan.h"
#include "PgParticle.h"
#include "PgDirectionArrow.h"
#include "PgParticleProcessorRotateToTarget.H"

PgDirectionArrow::PgDirectionArrow(void)
{
	m_iDirArrowSlot = 454326;
	m_pkDestObject = 0;
	m_pkDirArrow = 0;

	m_pkDirArrow = (PgParticle*)g_kParticleMan.GetParticle("e_ef_direction_arrow", PgParticle::O_SCALE,2.0f);
	PG_ASSERT_LOG(m_pkDirArrow);
	if(m_pkDirArrow)
	{
		g_kPilotMan.GetPlayerActor()->AttachTo(m_iDirArrowSlot, "char_root", m_pkDirArrow);
	}
}

PgDirectionArrow::~PgDirectionArrow(void)
{
	if(m_pkDirArrow && g_kPilotMan.GetPlayerActor())
	{
		//NiAVObject* pkCharRoot = g_kPilotMan.GetPlayerActor()->GetObjectByName("char_root");
		//if ((NiNode*)pkCharRoot)
		{
			//NiNode* pkRootNode = (NiNode*)pkCharRoot;
			//pkRootNode->DetachChild(m_pkDirArrow);
		}
	}
	//g_kPilotMan.GetPlayerActor()->DetachFrom(m_iDirArrowSlot);
}

bool PgDirectionArrow::Initialize()
{
	return true;
}

void PgDirectionArrow::Terminate()
{
}

//! PgIWorldObject 재정의
bool PgDirectionArrow::Update(float fAccumTime, float fFrameTime)
{
	PgIWorldObject::Update(fAccumTime, fFrameTime);

	/*
	NiPoint3 const &rkDirArrowPos = m_pkDirArrow->GetTranslate();
	static NiPoint3 g_ptPos = NiPoint3(0,0,50);
	g_ptPos.z += 0.5f;
	if (g_ptPos.z > 80)
	{
		g_ptPos.z = 50;
	}
	m_pkDirArrow->SetTranslate(g_ptPos);
	*/

	return true;
}

void PgDirectionArrow::SetDestObject(NiAVObject* pkDestObject)
{
	PG_ASSERT_LOG(m_pkDestObject);	
	// 똑같은 것을 눌렀다면 때주자.
	if (m_pkDestObject != NULL && m_pkDestObject == pkDestObject)
	{
		g_kPilotMan.GetPlayerActor()->DetachFrom(m_iDirArrowSlot);
		m_pkDestObject = NULL;
		m_pkDirArrow = NULL;
	}
	else if (pkDestObject)
	{
		m_pkDirArrow = (PgParticle*)g_kParticleMan.GetParticle("e_ef_direction_arrow",PgParticle::O_SCALE, 2.0f);
		PG_ASSERT_LOG(m_pkDirArrow);
		if(m_pkDirArrow)
		{
			g_kPilotMan.GetPlayerActor()->AttachTo(m_iDirArrowSlot, "char_root", m_pkDirArrow);
			m_pkDestObject = pkDestObject;
			m_pkDirArrow->SetParticleProcessor(NiNew PgParticleProcessorRotateToTarget(pkDestObject,NiPoint3(1,0,0)));
		}
	}
}
