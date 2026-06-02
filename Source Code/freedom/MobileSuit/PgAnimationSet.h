#pragma once

#include "PgIXmlObject.h"

class PgAnimationSet : public PgIXmlObject
{
	typedef std::vector<std::pair<int, int> > SequenceContainer;
	typedef std::map<std::string, SequenceContainer> Container;

public:
	bool ParseXml(const TiXmlNode *kNode, void *pArg = 0);
	bool GetAnimation(std::string kSlotName, NiActorManager::SequenceID& rkSeqID);

private:
	
	Container m_kContainer;
};