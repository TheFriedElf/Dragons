#include "StdAfx.h"
#include "PgCameraModeMovie.h"
#include "PgMobileSuit.h"

static NiCamera *GetCamera(NiNode *pkNode)
{
	for(unsigned int i = 0;
		i < pkNode->GetArrayCount();
		++i)
	{
		NiAVObject *pkChild = pkNode->GetAt(i);

		if(NiIsKindOf(NiNode, pkChild))
		{
			NiCamera *pkCamera = GetCamera(NiDynamicCast(NiNode, pkChild));
			if(pkCamera != 0)
			{
				return pkCamera;
			}
		}
		else if(NiIsKindOf(NiCamera, pkChild))
		{
			return NiDynamicCast(NiCamera, pkChild);
		}
	}

	return 0;
}

PgCameraModeMovie::PgCameraModeMovie(NiCamera *pkCamera, NiActorManagerPtr spAM)
: PgICameraMode(pkCamera), m_spAM(spAM), m_kSeqID(NiActorManager::INVALID_SEQUENCE_ID)//에니 선택 안됨
  , m_bAMUpdate(true)
{
	NiNode *pkNIFRoot = NiDynamicCast(NiNode, m_spAM->GetNIFRoot());
	if(pkNIFRoot)
	{
		m_pkMovieCamera = NiDynamicCast(NiCamera, pkNIFRoot->GetObjectByName("object0"));
		if(!m_pkMovieCamera)
		{
			m_pkMovieCamera = ::GetCamera(pkNIFRoot);
		}

		if(!m_pkMovieCamera)
		{
			NiMessageBox("Can't Find Camera", __FILE__);
			return;
		}

		m_kOldFrustum = g_kFrustum;
		g_kFrustum = m_pkMovieCamera->GetViewFrustum();
		g_kFrustum.m_fFar = 100000.0f;
	}
}

PgCameraModeMovie::~PgCameraModeMovie()
{
	g_kFrustum = m_kOldFrustum;
}

bool PgCameraModeMovie::Update(float fFrameTime)
{
	if(NULL==m_pkMovieCamera)	{return true;}
	if(g_pkWorld 
		&& m_bAMUpdate == true
		)
	{
		float fAccumTime = g_pkWorld->GetAccumTime();
		m_spAM->Update(fAccumTime);
		m_spAM->GetNIFRoot()->Update(fAccumTime);
	}

	m_pkCamera->SetViewFrustum(m_pkMovieCamera->GetViewFrustum());
	m_pkCamera->SetTranslate(m_pkMovieCamera->GetWorldTranslate());
	m_pkCamera->SetRotate(m_pkMovieCamera->GetWorldRotate());

	if( IsAnimationDone() )	//루프 카메라는 안잡힘
	{
		if( !m_szNextScript.empty() )
		{
			NIMETRICS_EVAL(NiMetricsClockTimer a("PgMobileSuit.lua_call"));
			NIMETRICS_STARTTIMER(a);			
			lua_tinker::dostring(*lua_wrapper_user(g_kLuaTinker), m_szNextScript.c_str());
			NIMETRICS_ENDTIMER(a);
		}
	}

	return true;
}

bool PgCameraModeMovie::SetTargetAnimation(int const &iNum, char const* szScript)
{
	if( m_spAM != 0 && m_pkCamera != 0)
	{
		// m_spAM->ChangeNIFRoot(m_pkCamera);

		m_kSeqID = iNum;

		//새로 애니 지정하기 전에 초기화 해줌
		m_spAM->Reset();
		if( szScript != 0 )
		{
			SetNextScript(szScript);
		}

		return m_spAM->SetTargetAnimation(iNum);
		
	}

	//실패시
	m_kSeqID = NiActorManager::INVALID_SEQUENCE_ID;

	return false;
}

bool PgCameraModeMovie::IsAnimationDone()
{
	assert(m_spAM);

	NiControllerSequence *pkSequence = m_spAM->GetSequence(m_kSeqID);

	if(!pkSequence || pkSequence->GetCycleType() == NiTimeController::LOOP)
	{
		return false;
	}

	float fTime = m_spAM->GetNextEventTime(NiActorManager::END_OF_SEQUENCE, m_kSeqID);

	if(m_spAM->GetCurAnimation() == m_kSeqID
	&& fTime == NiActorManager::INVALID_TIME)
	{
		return true;
	}

	return false;
}

void PgCameraModeMovie::SetNextScript(char const *szScript)
{
	m_szNextScript = szScript;
}

void PgCameraModeMovie::SetTranslate(NiPoint3 const &kTrn)
{
	NiNode *pkNIFRoot = NiDynamicCast(NiNode, m_spAM->GetNIFRoot());
	if(pkNIFRoot)
	{
		pkNIFRoot->SetTranslate(kTrn);
	}
}

void PgCameraModeMovie::SetAMUpdate(bool bUpdate)
{
	m_bAMUpdate = bUpdate;
}

