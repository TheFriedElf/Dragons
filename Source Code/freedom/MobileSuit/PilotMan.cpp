#include "StdAfx.h"
#include "MobileSuit.h"
#include "PilotMan.h"
#include "WorldMan.h"
#include "Pilot.h"
#include "Actor.h"

CPilotMan::CPilotMan()
{
}

CPilotMan::~CPilotMan(void)
{
}

void CPilotMan::SetWorldMan(CWorldMan *pkWorldMan)
{
	m_pkWorldMan = pkWorldMan;
}

CWorldMan *CPilotMan::GetWorldMan()
{
	return m_pkWorldMan;
}

void CPilotMan::Update(float fTime)
{
	Container::iterator itor = m_kContainer.begin();
	while(itor != m_kContainer.end())
	{
		itor->second->Update(fTime);
		itor++;
	}
}

void CPilotMan::Draw(NiCamera *pkCamera)
{
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	
	pkRenderer->SetCameraData(pkCamera);
	m_kVisibleActor.RemoveAll();

	/*
	NiTMapIterator kPos = m_kContainer.GetFirstPos();
	while(kPos)
	{
		BM::GUID kGUID;
		CActor *pkActor;
		m_kContainer.GetNext(kPos, kGUID, pkActor);

		if(pkActor != NULL)
		{
			pkActor->Draw(pkRenderer, pkCamera, pkWM->GetCuller(), m_kVisibleActor);
		}
	}
	*/

	Container::iterator itor = m_kContainer.begin();
	while(itor != m_kContainer.end())
	{
		itor->second->GetActor()->Draw(pkRenderer, pkCamera, m_pkWorldMan->GetCuller(), m_kVisibleActor);
		itor++;
	}

	NiDrawVisibleArrayAppend(m_kVisibleActor);
}

bool CPilotMan::AddPilot(BM::GUID& rkGUID, char *pcKFMPath, NiPoint3 &rkLoc, CPilot **ppkPilot_out)
{
	NiActorManager *pkAM = NiActorManager::Create(g_pkApp->ConvertMediaFilename(pcKFMPath));
	assert(pkAM);
	
	CActor *pkActor = CActor::Create(pkAM);
	assert(pkActor);

	CPilot *pkPilot = NiNew CPilot(m_pkWorldMan, pkActor);
	assert(pkPilot);

	/*
	m_kContainer.SetAt(rkGUID, pkActor);
	*/
	m_kContainer.insert(std::make_pair(rkGUID, pkPilot));

	pkActor->SetTranslate(rkLoc);
	pkActor->Update(0.1f);

	*ppkPilot_out = pkPilot;

	return true;
}

CPilot *CPilotMan::GetPilot(BM::GUID& rkGUID)
{
	Container::iterator itor = m_kContainer.find(rkGUID);

	if(itor != m_kContainer.end())
	{
		return itor->second;
	}

	return NULL;
}