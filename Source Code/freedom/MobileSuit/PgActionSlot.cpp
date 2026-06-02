#include "stdafx.h"
#include "PgXmlLoader.h"
#include "PgActionSlot.h"
#include "PgSoundMan.h"
#include "PgActor.h"

PgActionSlot::~PgActionSlot()
{
	//	leesg213 2006-11-23 추가
	//	오디오 소스 제거 루틴
	//m_kSoundContainer.clear();
}

bool PgActionSlot::GetDefaultAnimation(std::string const& rkSlotName, NiActorManager::SequenceID &rkSeqID_out)
{
	std::string SlotName(rkSlotName);
	LWR(SlotName);
	DefaultAnimationContainer::iterator itr = m_kDefaultAnimationContainer.find(SlotName);
	if(itr == m_kDefaultAnimationContainer.end())
	{
		return false;
	}

	rkSeqID_out = itr->second;
	return true;
}

bool PgActionSlot::GetAnimation(std::string const &rkSlotName, NiActorManager::SequenceID &rkSeqID_out,bool bNoRandom)const
{
	std::string SlotName(rkSlotName);
	LWR(SlotName);
	AnimationContainer::const_iterator itr = m_kAnimationContainer.find(SlotName);
	if(itr == m_kAnimationContainer.end())
	{
		//PG_ASSERT_LOG(!"ActorSlot : Unknown Slot Name \n");
		return false;
	}

	SequenceContainer const& kSeqContainer = itr->second;
	PG_ASSERT_LOG(kSeqContainer.size() > 0);
	int iTotalSeq = kSeqContainer.size();
	if (iTotalSeq == 0)
	{
		return false;
	}

	int RandAcc = 0;

	SequenceContainer::const_iterator seqitor = kSeqContainer.begin();
	if(bNoRandom)
	{
		//	랜덤 사용하지 않을 경우 컨테이너의 첫번째 것을 리턴한다.
		rkSeqID_out = (*seqitor).first;
		return true;
	}

	typedef std::list< int > CONT_RATE;
	CONT_RATE kContRate;

	while( seqitor != kSeqContainer.end() )
	{
		//랜덤값 합산
		int const iRate = (*seqitor).second.iRandom;
		RandAcc += iRate;
		kContRate.push_back(iRate);
		++seqitor;
	}

	if(RandAcc > 0)
	{
		size_t const iResultRate = BM::Rand_Index(RandAcc);

		size_t iAccRate = 0;
		
		int iRetIndex = 0;

		CONT_RATE::const_iterator rate_itor =  kContRate.begin();
		while(rate_itor != kContRate.end())
		{
			iAccRate += (*rate_itor);//누적값이 넘어 서는 순간. 간다.
			if( iAccRate > iResultRate )
			{
				break;
			}
			++iRetIndex;
			++rate_itor;
		}

		rkSeqID_out = kSeqContainer[iRetIndex].first;
		return true;
	}
	
	rkSeqID_out = kSeqContainer[0].first;
	return true;
	
	////////////////////////////////////////////////////////////////////////
/*
	seqitor = kSeqContainer.begin();
	while( seqitor != kSeqContainer.end() )
	{
		iEqualizedRandomNum = static_cast<int>(((*seqitor).second.iRandom/((float)RandAcc))*iMaxRandomNum);

		for(int i=0;i<iEqualizedRandomNum;i++)
		{	
			iRandomNum[i+iRandomNumIndex] = iIndex;
		}
		iRandomNumIndex+=iEqualizedRandomNum;

		++seqitor;
		iIndex++;
	}

	//	섞기
	int	iTemp,iRand;
	for(int i=0;i<iMaxRandomNum;i++)
	{
		iRand = BM::Rand_Index(iMaxRandomNum);
		iTemp = iRandomNum[iRand];
		iRandomNum[iRand] = iRandomNum[i];
		iRandomNum[i] = iTemp;
	}

	int	const iSelectedRandomIndex = BM::Rand_Index(iMaxRandomNum);
	int	const iSelectedSeqIndex = iRandomNum[iSelectedRandomIndex];

	int const ret = BM::Rand_Range(RandAcc);

	iIndex = 0;
	seqitor = kSeqContainer.begin();
	while( seqitor != kSeqContainer.end() )
	{

		if(iIndex == iSelectedSeqIndex)
		{
			rkSeqID_out = (*seqitor).first;
			return true;
		}
		
		++seqitor;
		iIndex++;
	}

	// TODO : 여러개의 Seq중에 랜덤 비율에 따라 선택되게 해야 함..
	rkSeqID_out = kSeqContainer[0].first;
	return true;
*/
}
/*
{
	AnimationContainer::iterator itr;
	if(bIgnoreCase)
	{
		bool	bFound = false;
		for(itr = m_kAnimationContainer.begin(); itr != m_kAnimationContainer.end(); itr++)
		{
			if(stricmp(itr->first.c_str(),rkSlotName.c_str()) == 0)
			{
				bFound = true;
				break;
			}
		}
		if(!bFound)
		{
			return	false;
		}
	}
	else
	{
		itr = m_kAnimationContainer.find(rkSlotName);
		if(itr == m_kAnimationContainer.end())
		{
		//PG_ASSERT_LOG(!"ActorSlot : Unknown Slot Name \n");
			return false;
		}
	}

	SequenceContainer kSeqContainer = itr->second;
	PG_ASSERT_LOG(kSeqContainer.size() > 0);
	int iTotalSeq = kSeqContainer.size();
	if (iTotalSeq == 0)
	{
		return false;
	}

	int RandAcc = 0;

	SequenceContainer::iterator seqitor = kSeqContainer.begin();
	if(bNoRandom)
	{
		//	랜덤 사용하지 않을 경우 컨테이너의 첫번째 것을 리턴한다.
		rkSeqID_out = (*seqitor).first;
		return true;
	}
	while( seqitor != kSeqContainer.end() )
	{
		//랜덤값 합산
		RandAcc += (*seqitor).second.iRandom;
		++seqitor;
	}

	int	iRandomNum[50];
	int	iIndex = 0,iRandomNumIndex=0;
	int	iMaxRandomNum = 50;
	int	iEqualizedRandomNum;

	memset(iRandomNum,0,sizeof(int)*iMaxRandomNum);

	seqitor = kSeqContainer.begin();
	while( seqitor != kSeqContainer.end() )
	{
		iEqualizedRandomNum = static_cast<int>(((*seqitor).second.iRandom/((float)RandAcc))*iMaxRandomNum);

		for(int i=0;i<iEqualizedRandomNum;i++)
		{	
			iRandomNum[i+iRandomNumIndex] = iIndex;
		}
		iRandomNumIndex+=iEqualizedRandomNum;

		++seqitor;
		iIndex++;
	}

	//	섞기
	int	iTemp,iRand;
	for(int i=0;i<iMaxRandomNum;i++)
	{
		iRand = BM::Rand_Index(iMaxRandomNum);
		iTemp = iRandomNum[iRand];
		iRandomNum[iRand] = iRandomNum[i];
		iRandomNum[i] = iTemp;
	}

	int	const iSelectedRandomIndex = BM::Rand_Index(iMaxRandomNum);
	int	const iSelectedSeqIndex = iRandomNum[iSelectedRandomIndex];

	int const ret = BM::Rand_Range(RandAcc);

	iIndex = 0;
	seqitor = kSeqContainer.begin();
	while( seqitor != kSeqContainer.end() )
	{

		if(iIndex == iSelectedSeqIndex)
		{
			rkSeqID_out = (*seqitor).first;
			return true;
		}
		
		++seqitor;
		iIndex++;
	}

	// TODO : 여러개의 Seq중에 랜덤 비율에 따라 선택되게 해야 함..
	rkSeqID_out = kSeqContainer[0].first;
	return true;
}
*/
bool PgActionSlot::GetAnimationInfo(std::string const& rkSlotName,int iSeqID, std::string const& rkInfoName, std::string& rkInfoOut)const
{
	std::string SlotName(rkSlotName);
	LWR(SlotName);
	AnimationContainer::const_iterator itr = m_kAnimationContainer.find(SlotName);
	if(itr == m_kAnimationContainer.end())
	{
		return false;
	}

	SequenceContainer const& kSeqContainer = itr->second;
	PG_ASSERT_LOG(kSeqContainer.size() > 0);
	if (kSeqContainer.size() == 0)
	{
		return false;
	}

	bool	bFound = false;
	SequenceContainer::const_iterator seqitor = kSeqContainer.begin();
	if(NiActorManager::INVALID_SEQUENCE_ID!=iSeqID && 0!=iSeqID)
	{
		for(;seqitor!=kSeqContainer.end();seqitor++)
		{
			if(seqitor->first == iSeqID)
			{
				bFound = true;
				break;
			}
		}
	}
	else
	{
		if(kSeqContainer.end() != seqitor)
		{
			AnimationInfoMap::const_iterator infoIter = (*seqitor).second.kInfoMap.find(rkInfoName);
			if((*seqitor).second.kInfoMap.end() != infoIter)
			{
				rkInfoOut = infoIter->second;
				return true;
			}
		}
	}
	
	if(!bFound) 
	{
		return false;
	}

	if (seqitor->second.kInfoMap.size() == 0)
	{
		return false;
	}

	AnimationInfoMap::const_iterator infoIter = seqitor->second.kInfoMap.find(rkInfoName);
	if (infoIter == seqitor->second.kInfoMap.end())
	{
		return false;
	}

	rkInfoOut = infoIter->second;
	return true;
}

bool PgActionSlot::GetSound(std::string const& rkSeqID, stSoundInfo &kSoundInfo_Out)const
{
	std::string SeqID(rkSeqID);
	LWR(SeqID);
	SoundContainer::const_iterator itr = m_kSoundContainer.find(SeqID);
	if(itr == m_kSoundContainer.end())
	{
		return false;
	}

	kSoundInfo_Out = itr->second;
	return true;
}

PgActionSlot *PgActionSlot::Clone()
{
	PgActionSlot* pkClone = new PgActionSlot;
	
	pkClone->m_kDefaultAnimationContainer.insert(m_kDefaultAnimationContainer.begin(), m_kDefaultAnimationContainer.end());
	pkClone->m_kAnimationContainer.insert(m_kAnimationContainer.begin(), m_kAnimationContainer.end());
	pkClone->m_kSoundContainer.insert(m_kSoundContainer.begin(), m_kSoundContainer.end());

	return pkClone;
}

bool PgActionSlot::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	NiActorManager *pkAM = 0;
	if(pArg)
	{
		pkAM = (NiActorManager *)pArg;
	}

	const TiXmlElement *pkElement = pkNode->FirstChildElement();

	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "ITEM") == 0)
		{
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			AnimationInfo animInfo;
			std::string SlotName;
			int iSequenceID = NiActorManager::INVALID_SEQUENCE_ID;
			int iDefaultSequenceID = NiActorManager::INVALID_SEQUENCE_ID;
			float fSpeed = 0.0f;
			char const* pcSoundPath = NULL;
			float fSoundVolume = 0.0f;
			float fSoundMinDist = 1000.0f;
			float fSoundMaxDist = 30000.0f;

			animInfo.iRandom = 0;
			
			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName,"NAME") == 0)
				{
					SlotName = pcAttrValue;
					LWR(SlotName);
				}
				else if(strcmp(pcAttrName,"ANIMATION") == 0)
				{
					iSequenceID = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName,"DEFAULT") == 0)
				{
					iDefaultSequenceID = atoi(pcAttrValue);
				}				
				else if(strcmp(pcAttrName,"SPEED") == 0)
				{
					if (pcAttrValue)
						animInfo.kInfoMap.insert(std::make_pair(STR_SPEED, pcAttrValue));
				}
				else if(strcmp(pcAttrName,"RANDOM") == 0)
				{
					animInfo.iRandom = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName,"SOUND") == 0)
				{
					pcSoundPath = pcAttrValue;
				}
				else if(strcmp(pcAttrName,STR_FIRE_START_NODE) == 0)
				{
					PG_ASSERT_LOG(pcAttrValue);
					if (pcAttrValue)
						animInfo.kInfoMap.insert(std::make_pair(STR_FIRE_START_NODE, pcAttrValue));
				}
				else if(strcmp(pcAttrName,STR_FIRE_EFFECT) == 0)
				{
					PG_ASSERT_LOG(pcAttrValue);
					if (pcAttrValue)
						animInfo.kInfoMap.insert(std::make_pair(STR_FIRE_EFFECT, pcAttrValue));
				}
				else if(strcmp(pcAttrName,STR_TARGET_EFFECT_NODE) == 0)
				{
					PG_ASSERT_LOG(pcAttrValue);
					if (pcAttrValue)
						animInfo.kInfoMap.insert(std::make_pair(STR_TARGET_EFFECT_NODE, pcAttrValue));
				}
				else if(strcmp(pcAttrName,STR_TARGET_EFFECT) == 0)
				{
					PG_ASSERT_LOG(pcAttrValue);
					if (pcAttrValue)
						animInfo.kInfoMap.insert(std::make_pair(STR_TARGET_EFFECT, pcAttrValue));
				}
				else if(strcmp(pcAttrName,STR_PROJECTILE_ID) == 0)
				{
					PG_ASSERT_LOG(pcAttrValue);
					if (pcAttrValue)
						animInfo.kInfoMap.insert(std::make_pair(STR_PROJECTILE_ID, pcAttrValue));
				}
				else if(STR_COMMON_EFFECT_ID.compare(pcAttrName) == 0
					|| STR_COMMON_EFFECT_NODE.compare(pcAttrName) == 0
					|| STR_COMMON_EFFECT_SCALE.compare(pcAttrName) == 0
					|| STR_COMMON_EFFECT_TARGET.compare(pcAttrName) == 0
					|| STR_COMMON_EFFECT_DETACH_SKIP.compare(pcAttrName) == 0)
				{
					PG_ASSERT_LOG(pcAttrValue);
					if (pcAttrValue)
						animInfo.kInfoMap.insert(std::make_pair(pcAttrName, pcAttrValue));
				}
				else if(strcmp(pcAttrName,"VOLUME") == 0)
				{
					sscanf_s(pcAttrValue, "%f, %f, %f", &fSoundVolume, &fSoundMinDist, &fSoundMaxDist);
				}
				else
				{
					if(strlen(pcAttrName)>0)
					{
						animInfo.kInfoMap.insert(std::make_pair(pcAttrName, pcAttrValue));
					}
				}
			
				pkAttr = pkAttr->Next();
			}

			if(SlotName.empty())
			{
				PG_ASSERT_LOG(!"slot item's name is null");
				return false;
			}

			AnimationContainer::iterator itr = m_kAnimationContainer.find(SlotName);
			if(itr == m_kAnimationContainer.end())
			{
				itr = m_kAnimationContainer.insert(std::make_pair(SlotName, SequenceContainer())).first;
			}

			// QUESTION 현재 안 쓰이고 있는 거 같다. -> PC쪽에 씁니다.
			if(iDefaultSequenceID != NiActorManager::INVALID_SEQUENCE_ID &&
				m_kDefaultAnimationContainer.find(SlotName) == m_kDefaultAnimationContainer.end())
			{
				// Default Animation Container에는 필요한 애들만 넣는다.
				m_kDefaultAnimationContainer.insert(std::make_pair(SlotName, iDefaultSequenceID));
			}

			itr->second.push_back(std::make_pair(iSequenceID, animInfo));

			// 사운드 설정이 있다면, 사운드 파일을 로드한다.
			if(pcSoundPath)
			{
				m_kSoundContainer.insert(std::make_pair(SlotName, stSoundInfo(std::string(pcSoundPath),fSoundVolume,fSoundMinDist,fSoundMaxDist)));
			}
		}
		else
		{
			PG_ASSERT_LOG(!"unknow tag");
		}

		pkElement = pkElement->NextSiblingElement();
	}

	return true;
}
PgActionSlot::AnimationContainer const& PgActionSlot::GetAnimationCont() const
{
	return m_kAnimationContainer;
}