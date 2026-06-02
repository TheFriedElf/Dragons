#include "StdAfx.h"
#include "PgMobileSuit.H"
#include "PgParticle.h"
#include "PgParticleProcessor.H"
#include "PgParticleProcessorSound.H"
#include "PgParticleProcessorAutoGround.H"
#include "PgParticleProcessorSpriteAnimation.h"

NiImplementRTTI(PgParticle, NiNode);

PgParticle::PgParticle()
:m_bLoop(false),
m_bZTest(false),
m_bUseAppAccumTime(false),
m_bNoFollowParentRotation(false),
m_fOriginalScale(1),
m_bFinished(false),
m_fNow(0),
m_fStartTime(-1),
m_bUseAliveTime(false),
m_fTotalAliveTime(0)
{
	m_kParticleProcessorCont.resize(PgParticleProcessor::PPID_MAX);
	m_kOriginalRotate.MakeIdentity();
}

PgParticle::~PgParticle()
{
}

bool PgParticle::Update(float fAccumTime, float fFrameTime)
{

	UpdateNowTime(fAccumTime);

	CheckFinish(fAccumTime,fFrameTime);

	if(DoParticleProcessor(fAccumTime,fFrameTime))
	{
		NiAVObject::Update(fAccumTime,false);
	}

	return true;
}
bool	PgParticle::DoParticleProcessor(float fAccumTime,float fFrameTime)
{
	bool	bNeedUpdateAVObject = false;

	SetColorLocal(NiColorA::WHITE);

	for(int i=0;i<PgParticleProcessor::PPID_MAX;++i)
	{
		PgParticleProcessor	*pkProcessor = m_kParticleProcessorCont[i];
		if(!pkProcessor)
		{
			continue;
		}

		bNeedUpdateAVObject |= pkProcessor->DoProcess(this,fAccumTime,fFrameTime);

		if(pkProcessor->GetFinished())
		{
			m_kParticleProcessorCont[i] = NULL;
		}
	}

	return	bNeedUpdateAVObject;

}
void PgParticle::CheckFinish(float fAccumTime,float fFrameTime)
{
}

void PgParticle::UpdateDownwardPass(float fTime, bool bUpdateControllers)
{
	if(GetUseAppAccumTime())
	{
		fTime = g_pkApp->GetAccumTime();
	}

	NiNode::UpdateDownwardPass(fTime, bUpdateControllers);
}


void PgParticle::ApplyParticleInfo(stParticleInfo const &kParticleInfo)
{
	SetName(kParticleInfo.m_kPath.c_str());
	SetScale(kParticleInfo.m_fScale);
	SetOriginalScale(kParticleInfo.m_fScale);
	SetLoop(kParticleInfo.m_bLoop);
	SetZTest(kParticleInfo.m_bZTest);
	SetUseAppAccumTime(kParticleInfo.m_bUseAppAccumTime);
	SetNoFollowParentRotation(kParticleInfo.m_bNoFollowParentRotation);

	SetAutoGround(kParticleInfo.m_bAutoGround);
	SetSound(kParticleInfo.m_fSoundTime,kParticleInfo.m_kSoundID);
	SetSprite(kParticleInfo.m_kSpriteXML);
	SetAliveTime(kParticleInfo.m_fAliveTime);
}
void PgParticle::SetSound(float fSoundTime,std::string const &kSoundID)
{
	if(fSoundTime>=0 && kSoundID.empty() == false)
	{
		SetParticleProcessor(NiNew PgParticleProcessorSound(fSoundTime,kSoundID));
	}
	else
	{
		RemoveParticleProcessor(PgParticleProcessor::PPID_SOUND);
	}
}

void PgParticle::SetSprite(std::string const& kSpriteXml)
{
	if(!kSpriteXml.empty())
	{
		SetParticleProcessor(NiNew PgParticleProcessorSpriteAnimation(this));

		TiXmlDocument kXmlDoc(kSpriteXml.c_str());
		if( !PgXmlLoader::LoadFile(kXmlDoc, UNI(kSpriteXml.c_str())) )
		{
			return;
		}

		PgParticleProcessorSpriteAnimation* pkProcessor = dynamic_cast<PgParticleProcessorSpriteAnimation*>(GetParticleProcessor(PgParticleProcessor::PPID_SPRITE_ANIMATION));
		if(pkProcessor)
		{
			TiXmlNode const* pkXmlNode = kXmlDoc.FirstChildElement();
			pkProcessor->ParseXml(pkXmlNode);
		}

	}
	else
	{
		RemoveParticleProcessor(PgParticleProcessor::PPID_SPRITE_ANIMATION);
	}
}
void PgParticle::SetAutoGround(bool bAutoGround)	
{	
	if(bAutoGround)
	{
		SetParticleProcessor(NiNew PgParticleProcessorAutoGround());
	}
	else
	{
		RemoveParticleProcessor(PgParticleProcessor::PPID_AUTO_GROUND);
	}
}

void PgParticle::ApplyOptions(OPTION kValidOptions,float fScale, bool bLoop, bool bZTest, float fSoundTime,bool bAutoGround,bool bUseAppAccumTime, bool bNoFollowParentRotation)
{

	if(kValidOptions&O_SCALE)
	{
		float	fFinalScale = GetScale()*fScale;
		SetScale(fFinalScale);
		SetOriginalScale(fFinalScale);
	}
	if(kValidOptions&O_LOOP)
	{
		SetLoop(bLoop);
	}
	if(kValidOptions&O_ZTEST)
	{
		SetZTest(bZTest);
	}
	if(kValidOptions&O_SOUNDTIME)
	{
	}
	if(kValidOptions&O_AUTOGROUND)
	{
		SetAutoGround(bAutoGround);
	}
	if(kValidOptions&O_USEAPPACCUMTIME)
	{
		SetUseAppAccumTime(bUseAppAccumTime);
	}
	if(kValidOptions&O_NOFOLLOWPARENTROTATION)
	{
		SetNoFollowParentRotation(bNoFollowParentRotation);
	}

}

void PgParticle::CopyMembers(PgParticle* pDest, NiCloningProcess& kCloning)
{
	NiNode::CopyMembers(pDest,kCloning);

	pDest->SetID(GetID());

	pDest->m_fNow = 0.0f;
	pDest->m_bLoop = m_bLoop;
	pDest->m_bZTest = m_bZTest;
	pDest->m_bUseAppAccumTime = m_bUseAppAccumTime;
	pDest->m_bNoFollowParentRotation = m_bNoFollowParentRotation;
	pDest->m_fOriginalScale = m_fOriginalScale;

	for(int i=0;i<PgParticleProcessor::PPID_MAX;++i)
	{
		PgParticleProcessor	*pkProcessor = m_kParticleProcessorCont[i];
		if(!pkProcessor)
		{
			continue;
		}

		NiCloningProcess	kProcessorClonProcess;
		pDest->SetParticleProcessor(NiDynamicCast(PgParticleProcessor,pkProcessor->Clone(kProcessorClonProcess)));
	}
}
void PgParticle::UpdateNowTime(float fAccumTime)
{
	if(m_fStartTime == -1)
	{
		m_fStartTime = fAccumTime;
	}

	float	fElapsedTime = fAccumTime - m_fStartTime;
	SetNow(fElapsedTime);
}

bool PgParticle::IsLoop()
{
	return m_bLoop;
}

bool PgParticle::GetZTest()
{
	return m_bZTest;
}

void PgParticle::SetLoop(bool bLoop)
{
	m_bLoop = bLoop;
}

void PgParticle::SetZTest(bool bZTest)
{
	m_bZTest = bZTest;
}

void PgParticle::SetAliveTime(float const fTotalAliveTime)
{
	if(fTotalAliveTime)
	{
		m_bUseAliveTime = true;
		m_fTotalAliveTime = fTotalAliveTime;
	}
}

void PgParticle::CheckAliveTime(float const fFrameTime)
{
	if(UseAliveTime())
	{
		m_fTotalAliveTime -= fFrameTime;
		if(m_fTotalAliveTime <= 0)
		{
			SetFinished(true);
		}
	}
}
void PgParticle::SetPlaySpeed(float const fPlaySpeed)
{// 파티클의 플레이 속도를 조정한다 (2.0f == 2배)
	PgParticleUtil::SetParticlePlaySpeed( this, fPlaySpeed );
}

namespace PgParticleUtil
{
	void SetParticlePlaySpeed(NiAVObject* pkAVObj, float const fPlaySpeed)
	{// 
		if( !pkAVObj )
		{
			return;
		}

		NiPropertyList const& rkPropList = pkAVObj->GetPropertyList();
		NiTListIterator kPos = rkPropList.GetHeadPos();
		while( kPos )
		{// 프로퍼티(알파, 텍스쳐 애니 등)의
			NiProperty* pkProperty = rkPropList.GetNext(kPos);
			if( pkProperty )
			{	
				NiTimeController* pkControl = pkProperty->GetControllers();
				if( pkControl )
				{
					for(; pkControl; pkControl = pkControl->GetNext() )
					{// 컨트롤러를 찾아 시간 값을 증가 시켜줄수 있게 설정하고
						pkControl->SetFrequency( fPlaySpeed );
					}
				}
			}
		}
		NiTimeController* pkControl = pkAVObj->GetControllers();
		if( pkControl )
		{
			for(; pkControl; pkControl = pkControl->GetNext() )
			{// 자신 하위의 컨트롤러도 시간 값을 증가 시켜줄수 있게 설정하고
				pkControl->SetFrequency( fPlaySpeed );
			}
		}
		
		NiNode* pkNode = NiDynamicCast(NiNode, pkAVObj);
		if(pkNode)
		{// 나머지 자식 노드들에게도 똑같이 적용해준다
			for (unsigned int i = 0; i < pkNode->GetChildCount(); ++i)
			{
				NiAVObject* pkChild = pkNode->GetAt(i);
				if (pkChild)
				{
					SetParticlePlaySpeed( pkChild, fPlaySpeed );
				}
			}
		}
	}
}