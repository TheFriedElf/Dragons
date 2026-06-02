#include "stdafx.h"
#include "PgFilterString.h"

bool PgFilterString::ParseXml(const TiXmlNode *pkNode, void *pArg)
{
	bool bIsBadWord = true;
	const int iType = pkNode->Type();
	
	while(pkNode)
	{
		switch(iType)
		{
		case TiXmlNode::ELEMENT:
			{
				TiXmlElement *pkElement = (TiXmlElement *)pkNode;
				assert(pkElement);
				
				const char *pcTagName = pkElement->Value();

				if(strcmp(pcTagName, "BAD_WORD")==0)
				{
					bIsBadWord = true;
					// 자식 노드들을 파싱한다.
					// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
					const TiXmlNode * pkChildNode = pkNode->FirstChild();
					
					pkNode = pkChildNode;
					continue;
//					if(pkChildNode != 0)
//					{
//						if(!ParseXml(pkChildNode))
//						{
//							return false;
//						}
//					}
				}
				else if(strcmp(pcTagName, "GOOD_WORD")==0)
				{
					bIsBadWord = false;
					// 자식 노드들을 파싱한다.
					// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
					const TiXmlNode * pkChildNode = pkNode->FirstChild();
					pkNode = pkChildNode;
					continue;
//					if(pkChildNode != 0)
//					{
//						if(!ParseXml(pkChildNode))
//						{
//							return false;
//						}
//					}
				}
				else if(strcmp(pcTagName, "TEXT") == 0)
				{
					const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					unsigned long Lv=0;
					const char	*strText=0;
					while(pkAttr)
					{

						const char *pcAttrName = pkAttr->Name();
						const char *pcAttrValue = pkAttr->Value();

						if(strcmp(pcAttrName, "LV") == 0)
						{
							Lv = (unsigned long)atol(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "Text") == 0)
						{
							strText=pcAttrValue;
						}
						else
						{
							assert(!"invalid attribute");
						}

						pkAttr = pkAttr->Next();
					}

					if(bIsBadWord)
					{
						RegBadString(UNI(strText), Lv);
					}
					else
					{
						RegGoodString(UNI(strText));
					}
	//				return true;
				}
			}
		}

		const TiXmlNode* pkNextNode = pkNode->NextSibling();
		pkNode = pkNextNode;
//		if(pkNextNode)
//		{
//		}
	}
	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
//	const TiXmlNode* pkNextNode = pkNode->NextSibling();
//	if(pkNextNode)
//	{
//		pkNode = pkNextNode;
//		if(!ParseXml(pkNextNode))
//		{
//			return false;
//		}
//	}

	return	true;
}



bool PgFilterString::RegGoodString(const std::wstring &str)
{
	CONT_GOOD_WORD::_Pairib ret = m_mapGoodWord.insert(std::make_pair(str.size(), WORD_ARRAY()));
	ret.first->second.push_back(str);
	return true;
}

const std::wstring& PgFilterString::GetGoodString(const size_t size)const
{
	CONT_GOOD_WORD::const_iterator itor = m_mapGoodWord.find(size);

	if(itor != m_mapGoodWord.end())
	{
		const size_t total_count = itor->second.size();
		if(total_count)
		{
			return itor->second.at(rand()%total_count);
		}
	}
	static const std::wstring NullString;
	return NullString;
}

bool PgFilterString::RegBadString(const std::wstring &str, const int Lv)
{
	STRING_INFO kInfo;
	if(StringAnalysis(str, kInfo))
	{
		TOTAL_CONT::_Pairib ret = m_mapTotal.insert(std::make_pair(str.at(0), STRING_LIST()));
		
		if( 1573 == Lv )
		{
			int x= 0;
			x = 1;
		}
		ret.first->second.push_back(kInfo);
		return true;
	}
	return false;
}

bool PgFilterString::StringAnalysis(const std::wstring &str, STRING_INFO &rInfo)const
{
	if(!str.size()){return false;}

	rInfo.wstrOrg = str;
	std::wstring::const_iterator itor = str.begin();
	while(str.end() != itor)
	{
		CONT_STRING_ANALYSIS::_Pairib ret = rInfo.m_mapAnalysis.insert(std::make_pair(*itor,0));
		
		++ret.first->second;
		++itor;
	}
	return true;
}

bool PgFilterString::Filter(std::wstring &str, const bool bIsConvert)const
{
	STRING_INFO kInfo;
	if(StringAnalysis(str, kInfo))
	{
		if(FilterDetail(kInfo, bIsConvert))
		{
			str = kInfo.wstrOrg;
			return true;
		}
	}
	return false;
}

bool PgFilterString::FilterDetail(STRING_INFO &rkInput, const bool bIsConvert)const
{
	bool bRet = false;
	TOTAL_CONT::const_iterator total_itor = m_mapTotal.begin();

	while(total_itor != m_mapTotal.end())
	{
		CONT_STRING_ANALYSIS::iterator string_itor = rkInput.m_mapAnalysis.find((*total_itor).first);
		if(string_itor != rkInput.m_mapAnalysis.end())//욕설의 첫글자를 내포하고 있다면
		{
			const STRING_LIST &lstBadWord = (*total_itor).second;

			STRING_LIST::const_reverse_iterator word_itor = lstBadWord.rbegin();
			while(lstBadWord.rend() != word_itor)
			{
				std::wstring::size_type pos = 0;
				do
				{
					const std::wstring &rBadWord = (*word_itor).wstrOrg;

					pos = rkInput.wstrOrg.find(rBadWord, pos);
					
					if(std::wstring::npos != pos)
					{
						if(bIsConvert)
						{
							rkInput.wstrOrg.replace(pos, rBadWord.size(), GetGoodString(rBadWord.size()) );
						}
						bRet = true;//필터가 걸렸다
						pos += rBadWord.size();
					}
				}while(std::wstring::npos != pos);

				++word_itor;
			}
		}

		++total_itor;
	}
	return bRet;
}