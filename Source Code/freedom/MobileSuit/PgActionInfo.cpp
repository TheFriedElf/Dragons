#include "stdafx.h"
#include "PgActionInfo.h"

PgActionInfo	*g_pkActionInfo = 0;

PgActionInfo::PgActionInfo()
{
}
PgActionInfo::~PgActionInfo()
{
	for(ActionInfoCont::iterator itor = m_ContActionInfo.begin(); itor != m_ContActionInfo.end(); itor++)
	{
		delete itor->second;
	}

	m_ContActionInfo.clear();
}

bool PgActionInfo::ParseXml(const TiXmlNode *pkNode, void *pArg )
{
	const int iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			assert(pkElement);
			
			const char *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "ACTION_INFO") == 0)
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
			else if(strcmp(pcTagName, "ACTION") == 0)
			{
				stActionInfo	*pNewInfo = new stActionInfo();

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					const char *pcAttrName = pkAttr->Name();
					const char *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "ID") == 0)
					{
						pNewInfo->m_iActionID = atoi(pcAttrValue);
					}
					else if(strcmp(pcAttrName,"XML_ID") == 0)
					{
						pNewInfo->m_kXML_ID = pcAttrValue;
					}
					else
					{
						assert(!"invalid attribute");
					}

					pkAttr = pkAttr->Next();
				}

				m_ContActionInfo[pNewInfo->m_iActionID] = pNewInfo;
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
