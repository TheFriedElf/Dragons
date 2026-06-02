#include "stdafx.h"
#include "PgXmlLoader.h"
#include "PgAnimationSet.h"


bool PgAnimationSet::GetAnimation(std::string kSlotName, NiActorManager::SequenceID& rkSeqID)
{
	Container::iterator itr = m_kContainer.find(kSlotName);
	if(itr == m_kContainer.end())
	{
		assert(!"Animation Set : Unknown Slot Name \n");
		return false;
	}

	SequenceContainer kSeqContainer = itr->second;

	// TODO : 여러개의 Seq중에 랜덤 비율에 따라 선택되게 해야 함..
	rkSeqID = kSeqContainer[0].first;
	
	return true;
}

bool PgAnimationSet::ParseXml(const TiXmlNode *pkNode, void *pArg)
{
	const int iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			const TiXmlElement *pkElement = pkNode->FirstChildElement();

			while(pkElement)
			{
				const char *pcTagName = pkElement->Value();

				if(strcmp(pcTagName, "SLOT") == 0)
				{
					const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					const char *pcAttrName = pkAttr->Name();
					SequenceContainer kSeqContainer;
					
					if(strcmp(pcAttrName,"NAME") == 0)
					{
						const char *pcSlotName = pkAttr->Value();
						SequenceContainer kSeqContainer;
						int iSequenceID;
						int iRandom = 100;
						
						const TiXmlElement *pkChildElement = pkElement->FirstChildElement();
						while(pkChildElement)
						{
							pcTagName = pkChildElement->Value();
							
							if(strcmp(pcTagName, "ITEM") == 0)
							{
								const TiXmlAttribute *pkAttr = pkChildElement->FirstAttribute();				
								while(pkAttr)
								{
									const char *pcAttrName = pkAttr->Name();
									const char *pcAttrValue = pkAttr->Value();
									
									if(strcmp(pcAttrName,"SEQ") == 0)
									{
										iSequenceID = atoi(pcAttrValue);
									}
									else if(strcmp(pcAttrName,"RANDOM") == 0)
									{
										iRandom = atoi(pcAttrValue);
									}
									else
									{
										assert(!"unknow attribute");
									}
									pkAttr = pkAttr->Next();
								}
								kSeqContainer.push_back(std::make_pair(iSequenceID, iRandom));
								pkChildElement = pkChildElement->NextSiblingElement();
							}
						}
						m_kContainer.insert(std::make_pair(pcSlotName, kSeqContainer));
					}
					else
					{
						assert(!"unknow attribute");
					}
				}
				else
				{
					assert(!"unknow tag");
				}

				pkElement = pkElement->NextSiblingElement();
			}
		}
		break;
	default:
		break;
	}

	return true;
}