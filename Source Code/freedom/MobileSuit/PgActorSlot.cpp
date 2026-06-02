#include "stdafx.h"
#include "PgXmlLoader.h"
#include "PgActorSlot.h"
#include "PgSoundMan.h"
#include "PgActor.h"

PgActorSlot::~PgActorSlot()
{
	//	leesg213 2006-11-23 추가
	//	오디오 소스 제거 루틴
	m_kSoundContainer.clear();
}

bool PgActorSlot::GetAnimation(std::string &rkSlotName, NiActorManager::SequenceID &rkSeqID_out)
{
	AnimationContainer::iterator itr = m_kAnimationContainer.find(rkSlotName);
	if(itr == m_kAnimationContainer.end())
	{
		//assert(!"ActorSlot : Unknown Slot Name \n");
		return false;
	}

	SequenceContainer kSeqContainer = itr->second;

	int RandAcc = 0;

	SequenceContainer::iterator seqitor = kSeqContainer.begin();
	while( seqitor != kSeqContainer.end() )
	{
		//랜덤값 합산
		RandAcc += (*seqitor).second;
		++seqitor;
	}

	int ret = rand()%(RandAcc+1);

	RandAcc = 0;
	seqitor = kSeqContainer.begin();
	while( seqitor != kSeqContainer.end() )
	{
		//랜덤값 합산
		RandAcc += (*seqitor).second;
		if( ret <= RandAcc )
		{
			rkSeqID_out = (*seqitor).first;
			return true;
		}
		++seqitor;
	}

	// TODO : 여러개의 Seq중에 랜덤 비율에 따라 선택되게 해야 함..
	rkSeqID_out = kSeqContainer[0].first;
	return true;
}

bool PgActorSlot::GetSound(NiActorManager::SequenceID kSeqID, NiAudioSource *&rpkSound_out)
{

	SoundContainer::iterator itr = m_kSoundContainer.find(kSeqID);
	
	if(itr == m_kSoundContainer.end())
	{
		return false;
	}

	rpkSound_out = itr->second;

	return true;
}


bool PgActorSlot::ParseXml(const TiXmlNode *pkNode, void *pArg)
{
	PgActor *pkActor = (PgActor *)pArg;
	const TiXmlElement *pkElement = pkNode->FirstChildElement();

	while(pkElement)
	{
		const char *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "ITEM") == 0)
		{
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			const char *pcSlotName = 0;
			int iSequenceID = -1;
			int iRandom = 100;
			float fSpeed = 0.0f;
			const char *pcSndPath = 0;
			float fSndVolume = 0.0f;
			float fSndDistMin = 1000.0f;
			float fSndDistMax = 30000.0f;
			
			while(pkAttr)
			{
				const char *pcAttrName = pkAttr->Name();
				const char *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName,"NAME") == 0)
				{
					pcSlotName = pcAttrValue;
				}
				else if(strcmp(pcAttrName,"ANIMATION") == 0)
				{
					iSequenceID = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName,"SPEED") == 0)
				{
					fSpeed = (float)atof(pcAttrValue);
				}
				else if(strcmp(pcAttrName,"RANDOM") == 0)
				{
					iRandom = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName,"SOUND") == 0)
				{
					pcSndPath = pcAttrValue;
				}
				else if(strcmp(pcAttrName,"VOLUME") == 0)
				{
					sscanf_s(pcAttrValue, "%f, %f, %f", &fSndVolume, &fSndDistMin, &fSndDistMax);
				}
			
				pkAttr = pkAttr->Next();
			}

			if(!pcSlotName)
			{
				assert(!"slot item's name is null");
				return false;
			}

			AnimationContainer::iterator itr = m_kAnimationContainer.find(pcSlotName);
			if(itr == m_kAnimationContainer.end())
			{
				itr = m_kAnimationContainer.insert(std::make_pair(pcSlotName, SequenceContainer())).first;
			}

			itr->second.push_back(std::make_pair(iSequenceID, iRandom));

			// 속도 설정이 있으면, 속도를 변경하자
			if(fSpeed)
			{
				NiControllerSequence *pkSeq = pkActor->m_spAM->GetSequence(iSequenceID);
				assert(pkSeq);
				
				pkSeq->SetFrequency(fSpeed);
			}

			// 사운드 설정이 있다면, 사운드 파일을 로드한다.
			if(pcSndPath)
			{
				// 주의, 사운드 로딩의 에러 처리는 여기서 하지 말것!
				NiAudioSource *pkAudioSource = g_kSoundMan.LoadAudioSource(NiAudioSource::TYPE_3D, pcSndPath, fSndVolume, fSndDistMin, fSndDistMax);
				
				if(pkAudioSource)
				{
					m_kSoundContainer.insert(std::make_pair(iSequenceID, pkAudioSource));
				}
			}
		}
		else
		{
			assert(!"unknow tag");
		}

		pkElement = pkElement->NextSiblingElement();
	}

	return true;
}