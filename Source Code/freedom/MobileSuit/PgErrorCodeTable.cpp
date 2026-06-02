#include "stdafx.h"
#include "PgErrorCodeTable.h"

PgErrorCodeTable	*g_pkErrorCT = NULL;
unsigned long ulTextID=0;
char const	*strText=0;

//! Node를 파싱한다.
bool PgErrorCodeTable::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();

	while(pkNode)
	{
		switch(iType)
		{
		case TiXmlNode::ELEMENT:
			{
				TiXmlElement *pkElement = (TiXmlElement *)pkNode;
				assert(pkElement);

				char const *pcTagName = pkElement->Value();

				if(strcmp(pcTagName, "ERROR")==0)
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
				else if(strcmp(pcTagName, "ERRNO") == 0)
				{
					const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					char const *pcPath = pkElement->GetText();
					ulTextID = 0;	//일단 초기화
					strText = NULL;
					while(pkAttr)
					{

						char const *pcAttrName = pkAttr->Name();
						char const *pcAttrValue = pkAttr->Value();

						if(strcmp(pcAttrName, "value") == 0)
						{
							ulTextID = (unsigned long)atol(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "name") == 0)
						{
							strText=pcAttrValue;
						}
						else
						{
							PgError(pcAttrName);
							assert(!"invalid attribute");
						}

						pkAttr = pkAttr->Next();
					}				
					const TiXmlNode * pkChildNode = pkNode->FirstChild();
					if(pkChildNode != 0)
					{
						if(!ParseXml(pkChildNode))
						{
							return false;
						}
					}
				}
				else if(strcmp(pcTagName, "DESCRIPTION") == 0)
				{
					//	같은 아이디로 이미 텍스트가 존재하는지 체크한다.
					TextMap::iterator itor = m_TextMap.find(ulTextID);
					if(itor != m_TextMap.end())
					{
						//	같은 아이디가 존재하면 안되는데, 존재한다 -,-;
						char szStr[100] = {0, };
						_snprintf_s(szStr, sizeof(szStr),_TRUNCATE, "Same Text Table ID [%u] Exist", ulTextID);
						PgError(szStr);
					}

					if(/*ulTextID !=0 && */strText != 0) //값이 0으로 시작한다
					{
						char const *pcPath = pkElement->GetText();
						if (pcPath != NULL)
						{
							m_TextMap[ulTextID] = std::wstring(UNI(pcPath));
						}
					}
				}
			}
		}

		const TiXmlNode* pkNextNode = pkNode->NextSibling();
		pkNode = pkNextNode;
	}
	return	true;
}

void	PgErrorCodeTable::Init()
{
	m_TextMap.clear();
}

void	PgErrorCodeTable::Destroy()
{
}

std::wstring const &PgErrorCodeTable::GetTextW(unsigned long ulIndex)const
{
	TextMap::const_iterator itor = m_TextMap.find(ulIndex);
	if(itor == m_TextMap.end())
	{
		char temp[100];
		_snprintf_s(temp,sizeof(temp),_TRUNCATE,"GetTextW(%u) Failed.",ulIndex);
		//		MessageBox(0,UNI(temp),_T("TextTable Error"),MB_OK);
		assert(0 && temp);
		static std::wstring const NullString = _T("");
		return NullString;
	}
	return itor->second;
}