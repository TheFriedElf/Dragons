#include "StdAfx.h"
#include "PgMobileSuit.h"
#include "PgMath.h"
#include "PgRenderer.h"
#include "PgUIScene.h"
#include "PgBalloonEmoticonData.h"
#include "PgBalloonEmoticonInstance.h"

PgBalloonEmoticonFrameInstance::PgBalloonEmoticonFrameInstance(PgBalloonEmoticonFrameData const* pFrameInfo)
	: m_kStartTime(0.0f)
	, m_fPrevTime(0.0f)
	, m_pFrameInfo(pFrameInfo)
{
}

PgBalloonEmoticonFrameInstance::~PgBalloonEmoticonFrameInstance(void)
{
	m_pFrameInfo = NULL;
}

bool PgBalloonEmoticonFrameInstance::Update(float fAccumTime,float fFrameTime, bool const bPause)
{
	if( !m_pFrameInfo )					{ return false; }
	if( !m_pFrameInfo->SrcTexture() )	{ return false; }

	m_fPrevTime = fAccumTime;

	if( bPause )
	{
		m_kStartTime += (fAccumTime - m_fPrevTime);
	}
	else
	{
		if( (m_fPrevTime - m_kStartTime) > m_pFrameInfo->DelayTime() )
		{
			return false;
		}
	}
	return true; 
}

void PgBalloonEmoticonFrameInstance::DrawImmediate(PgRenderer *pkRenderer, POINT2 const& PtTargetPos, POINT2 const& FrameSize, int const iU)
{
	if( !pkRenderer )					{ return; }
	if( !m_pFrameInfo )					{ return; }
	if( !m_pFrameInfo->SrcTexture() )	{ return; }

	short const StartX = m_pFrameInfo->FrameIdx() % iU * FrameSize.x;
	short const StartY = m_pFrameInfo->FrameIdx() / iU * FrameSize.y;

	m_pFrameInfo->SrcTexture()->RemoveAllScreenRects();
	m_pFrameInfo->SrcTexture()->AddNewScreenRect((short)PtTargetPos.y, (short)PtTargetPos.x, (short)FrameSize.x, (short)FrameSize.y, StartY, StartX);
	m_pFrameInfo->SrcTexture()->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	PgUIScene::Render_UIObject(pkRenderer, m_pFrameInfo->SrcTexture());
}


PgBalloonEmoticonInstance::PgBalloonEmoticonInstance(void)
	: m_iChangedFrameCount(0)
	, m_pEmoticonInfo(NULL)
{
}

PgBalloonEmoticonInstance::~PgBalloonEmoticonInstance(void)
{
	m_pEmoticonInfo = NULL;
}

bool PgBalloonEmoticonInstance::Initialize(PgBalloonEmoticonData const* pEmoticonInfo)
{
	if( !pEmoticonInfo )
	{ 
		return false; 
	}

	m_pEmoticonInfo = pEmoticonInfo;

	int const TotalFrame = m_pEmoticonInfo->GetTotalFrame();
	if( 0 == TotalFrame )
	{
		return false;
	}

	for(int i = 0; i < TotalFrame; ++i)
	{
		PgBalloonEmoticonFrameInstance FrameInstance(m_pEmoticonInfo->GetFrame(i));
		m_kFrameInstance.push_back(FrameInstance);
	}

	kFrameInstanceContainer::iterator iter = m_kFrameInstance.begin();
	if( iter != m_kFrameInstance.end() )
	{
		iter->StartTime(g_pkApp->GetAccumTime());
	}
	return true;
}

bool PgBalloonEmoticonInstance::Update(float fAccumTime,float fFrameTime, bool const bPause)
{ 
	if( !m_pEmoticonInfo )
	{ 
		return false; 
	}

	kFrameInstanceContainer::iterator iter = m_kFrameInstance.begin();
	if( iter == m_kFrameInstance.end() )
	{
		return false;
	}

	if( !iter->Update(fAccumTime, fFrameTime, bPause) )
	{
		int const iLoopCount = ++m_iChangedFrameCount / m_kFrameInstance.size();
		if( 1 <= iLoopCount)
		{
			if( 0 == m_pEmoticonInfo->LoopType() )
			{
				return false;
			}
			else
			{
				if( -1 != m_pEmoticonInfo->LoopType() && iLoopCount >= m_pEmoticonInfo->LoopType() )
				{
					return false;
				}
			}
		}
		m_kFrameInstance.push_back((*iter));
		m_kFrameInstance.pop_front();
		iter = m_kFrameInstance.begin();
		if( iter != m_kFrameInstance.end() )
		{
			iter->StartTime(fAccumTime);
		}
	}

	return true; 
}

void PgBalloonEmoticonInstance::DrawImmediate(PgRenderer *pkRenderer, PgActor* pActor) 
{ 
	if( !pkRenderer )
	{
		return;
	}
	if( !pActor || !pActor->GetPilot() )
	{ 
		return; 
	}
	if( !m_pEmoticonInfo )
	{ 
		return; 
	}

	kFrameInstanceContainer::iterator iter = m_kFrameInstance.begin();
	if( iter != m_kFrameInstance.end() )
	{
		int const iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
		int const iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

		PgRenderer *pkPgRenderer = PgRenderer::GetPgRenderer();
		if( !pkPgRenderer )	{ return; }

		NiCamera *pkCamera = pkPgRenderer->GetCameraData();
		if( !pkCamera )		{ return; }

		//NiNodePtr pkDummy = (NiNode*)pActor->GetObjectByName(ATTACH_POINT_STAR);
		NiNodePtr pkDummy = pActor->GetNodePointStar();
		if( !pkDummy )		{ return; }

		NiPoint3 const kActorPos = pkDummy->GetWorldTranslate();
		NiPoint3 const kLengthPos = kActorPos - pkCamera->GetTranslate();
		if( 1000.0f < kLengthPos.Length() )
		{
			return;
		}

		//NiPoint3 const kStartPos =
		NiPoint3 const kStartPos = kActorPos + NiPoint3(0.f, 0.f, 20.f);
		float fX = 0.f, fY = 0.f;
		bool const bResult = pkCamera->WorldPtToScreenPt(kStartPos, fX, fY);
		if( !bResult )		{ return; }

		POINT2 TargetPT(iScreenWidth * fX - m_pEmoticonInfo->FrameSize().x / 2, iScreenHeight * ( 1 - fY) - m_pEmoticonInfo->FrameSize().y);

		NiPoint2 kClippedAdjust;
		bool const bRet = Get_Clipped_Region(NiPoint2(TargetPT.x, TargetPT.y), NiPoint2(m_pEmoticonInfo->FrameSize().x, m_pEmoticonInfo->FrameSize().y), 
											 kClippedAdjust, NiPoint2(0.0f, 0.0f), NiPoint2(iScreenWidth, iScreenHeight));
		if( bRet )
		{
			iter->DrawImmediate(pkRenderer, TargetPT, m_pEmoticonInfo->FrameSize(), m_pEmoticonInfo->U());
		}
	}
}
