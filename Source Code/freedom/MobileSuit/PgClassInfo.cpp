#include "stdafx.h"
#include "PgClassInfo.h"

PgClassInfo	*g_pkClassInfo = 0;

PgClassInfo::PgClassInfo()
{
	m_iClassInfoCount = 0;
}
PgClassInfo::~PgClassInfo()
{
	int	iTotalInfo = m_vClassInfo.size();
	for(int i=0;i<iTotalInfo;i++)
	{
		delete m_vClassInfo[i];
	}

	m_vClassInfo.clear();
}

bool PgClassInfo::ParseXml(const TiXmlNode *pkNode, void *pArg )
{
	const int iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			assert(pkElement);
			
			const char *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "CLASS_INFO") == 0)
			{
				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "CLASS") == 0)
			{
				stClassInfo	*pNewInfo = new stClassInfo();
				m_vClassInfo.push_back(pNewInfo);

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					const char *pcAttrName = pkAttr->Name();
					const char *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "ID") == 0)
					{
						pNewInfo->m_iClassID = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName, "NAME") == 0)
					{
						pNewInfo->m_kClassName = pcAttrValue;
					}
					else
					{
						assert(!"invalid attribute");
					}

					pkAttr = pkAttr->Next();
				}
			}
			else if(strcmp(pcTagName, "CAN_LEARN_SKILL") == 0)
			{
				// 자식 노드들을 파싱한다.
				// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml(pkChildNode))
					{
						return false;
					}
				}				
			}
			else if(strcmp(pcTagName, "SKILL") == 0)
			{
				stClassInfo *pClassInfo = m_vClassInfo[m_vClassInfo.size()-1];
				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					const char *pcAttrName = pkAttr->Name();
					const char *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "ID") == 0)
					{
						pClassInfo->m_vCanLearnSkill.push_back(atoi(pcAttrValue));
					}
					else
					{
						assert(!"invalid attribute");
					}

					pkAttr = pkAttr->Next();
				}
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
				break;
			}
		}
	default:
		break;
	};

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}
	return	true;
}
