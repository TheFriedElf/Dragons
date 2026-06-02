#pragma once

#include "PgIXmlObject.h"

class PgActorSlot : public PgIXmlObject
{
	typedef std::vector<std::pair<NiActorManager::SequenceID, int> > SequenceContainer;
	typedef std::map<std::string, SequenceContainer> AnimationContainer;
	typedef std::map<NiActorManager::SequenceID, NiAudioSourcePtr> SoundContainer;	// leesg213 2006-11-24, NiAudioSource* -->NiAudioSourcePtr 로 바꿈(스마트 포인터 사용)

public:
	~PgActorSlot();

	bool ParseXml(const TiXmlNode *kNode, void *pArg = 0);
	bool GetAnimation(std::string &rkSlotName, NiActorManager::SequenceID &rkSeqID_out);
	bool GetSound(NiActorManager::SequenceID, NiAudioSource *&rpkSound_out);
	
private:
	AnimationContainer m_kAnimationContainer;
	SoundContainer m_kSoundContainer;
};