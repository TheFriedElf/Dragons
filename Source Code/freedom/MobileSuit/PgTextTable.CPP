#include "stdafx.h"
#include "Variant/PgQuestInfo.h"
#include "PgTextTable.h"
#include "PgMobileSuit.h"
#include "ServerLib.h"

class PgTextReplacerClient : public PgQuestInfoUtil::PgTextReplacer
{
public:
	static std::wstring Parse(size_t const iTextID, std::wstring const& rkOrgString)
	{
		PgTextReplacerClient kReplacer(iTextID, rkOrgString);
		return kReplacer.m_kResult;
	}

private:
	PgTextReplacerClient(size_t const iTextID, std::wstring const& rkOrgText)
		: PgTextReplacer(rkOrgText), m_iTextID(iTextID)
	{
		PgTextReplacer::Parse();
	}
	virtual ~PgTextReplacerClient()
	{
	}

	virtual void AddParsedError(std::wstring const& rkArg, std::wstring const& rkErrorMsg)
	{
#ifndef USE_INB
		_PgError("Text Replace Error", "TextID %u, Replace Error: %s\n%s", m_iTextID, MB(rkArg), MB(rkErrorMsg));
#endif
	}

	virtual void ConvertText(int const iNo, std::wstring &rkOutputText) // TextTableNo -> Text
	{
		rkOutputText = TTW(iNo);
	}

	size_t const m_iTextID;
};

PgTextTable	*g_pkTT=0;

void PgTextTable::WordWrap( std::wstring &wstrWord )
{
	std::wstring::iterator itr = std::find( wstrWord.begin(), wstrWord.end(), L'\\' );
	if ( itr != wstrWord.end() )
	{
		std::wstring::iterator itr_begin = itr;
		if ( (++itr != wstrWord.end()) && (L'n' == *itr) )
		{
			wstrWord.replace( itr_begin, ++itr, 1, L'\n' );
			WordWrap( wstrWord );	//재귀호출
		}
	}
}

//! Node를 파싱한다.
bool PgTextTable::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
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

				if(strcmp(pcTagName, XML_ELEMENT_TEXT_TABLE)==0)
				{
					// 자식 노드들을 파싱한다.
					// 첫 자식만 여기서 걸어주면, 나머지는 NextSibling에 의해서 자동으로 파싱된다.
					const TiXmlNode * pkChildNode = pkNode->FirstChild();
					if(pkChildNode != 0)
					{
						if(!ParseXml(pkChildNode, pArg, bUTF8))
						{
							return false;
						}
					}
				}
				else if(strcmp(pcTagName, XML_ELEMENT_QUEST_TEXT_TABLE) == 0)
				{
					const TiXmlNode * pkChildNode = pkNode->FirstChild();
					if(pkChildNode != 0)
					{
						if(!ParseXml(pkChildNode, pArg, bUTF8))
						{
							return false;
						}
					}
				}
				else if(strcmp(pcTagName, XML_ELEMENT_MAP_TEXT_TABLE) == 0)
				{
					const TiXmlNode * pkChildNode = pkNode->FirstChild();
					if(pkChildNode != 0)
					{
						if(!ParseXml(pkChildNode, pArg, bUTF8))
						{
							return false;
						}
					}
				}
				else if(strcmp(pcTagName, XML_ELEMENT_TEXT) == 0)
				{
					const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					unsigned long ulTextID=0;
					char const	*strText=0;
					while(pkAttr)
					{

						char const *pcAttrName = pkAttr->Name();
						char const *pcAttrValue = pkAttr->Value();

						if(strcmp(pcAttrName, XML_ATTR_TextID) == 0)
						{
							ulTextID = (unsigned long)atol(pcAttrValue);
						}
						else if(strcmp(pcAttrName, XML_ATTR_TextTEXT) == 0)
						{
							strText=pcAttrValue;
						}
						else
						{
							assert(!"invalid attribute");
						}

						pkAttr = pkAttr->Next();
					}				

					//	같은 아이디로 이미 텍스트가 존재하는지 체크한다.					
					std::wstring kText;

					if (bUTF8)
					{
						kText = BM::vstring::ConvToUnicodeFromUTF8(strText);
					}
					else
					{
						kText = UNI(strText);
					}

					WordWrap( kText );
					kText = PgTextReplacerClient::Parse( ulTextID, kText );

					if( ulTextID
					&&	!kText.empty() )
					{
						std::pair<TextMap::iterator, bool> kRet = m_TextMap.insert( std::make_pair(ulTextID, kText) );
						if( !kRet.second )//중복 되었으면
						{
							//	같은 아이디가 존재하면 안되는데, 존재한다 -,-;
							char szStr[100] = {0, };
							_snprintf_s(szStr, sizeof(szStr),_TRUNCATE, "Same Text Table ID [%u] Exist", ulTextID);
							PgError(szStr);
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

void PgTextTable::Init()
{
}

void PgTextTable::Destroy()
{
}

void PgTextTable::Clear()
{
	m_TextMap.clear();
}

std::wstring const &PgTextTable::GetTextW(unsigned long ulIndex)const
{
	TextMap::const_iterator itor = m_TextMap.find(ulIndex);
	if(itor == m_TextMap.end())
	{
#ifndef EXTERNAL_RELEASE
		if (ulIndex != 0)
		{
			PgError1("%d TextTable is not exist", ulIndex);
		}
#endif	
		if (ulIndex != 0)
		{
			NILOG(PGLOG_ERROR, "%d TextTable is not exist", ulIndex);
		}
		static std::wstring const NullString = _T("");
		return NullString;
	}
	
#ifndef USE_INB
	if( g_pkApp->VisibleClassNo() )
	{
		return *PgDefStringDebugUtil::GetDebugTextString(ulIndex, itor->second);
	}
#endif
	return itor->second;
}

bool FormatTTW(std::wstring& rkOut, int const iTTW, ...)
{
	std::wstring kTTW = TTW(iTTW);
	if( kTTW.empty() )
	{
		return false;
	}

	TCHAR szTemp[MAX_PATH] = {0, };
	va_list vargs;
	va_start( vargs, iTTW );

	_vstprintf_s(szTemp, MAX_PATH, kTTW.c_str(), vargs );

	va_end(vargs);
	rkOut = szTemp;

	return true;
}



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Format 된 메시지를 원하는 문자로 변환하기 위한 함수

void TextChange_Item(std::wstring &strText, CItemDef const *pkItemDef)
{
	wchar_t const *pkItemName = NULL;
	GetDefString(pkItemDef->NameNo(), pkItemName);
	const TCHAR* pszInit[][2] = {	{_T("%ITEM_NAME%"), pkItemName },
									{ NULL, NULL } };

	VEC_TextChangeInfo kChangeInfo;
	int iIndex = 0;
	while (pszInit[iIndex][0] != NULL)
	{
		kChangeInfo.push_back(STextChangeInfo(pszInit[iIndex][0], pszInit[iIndex][1]));

		++iIndex;
	}

	VEC_TextChangeInfo::const_iterator itor = kChangeInfo.begin();
	std::wstring::size_type kFindPos;
	while (itor != kChangeInfo.end())
	{
		while ((kFindPos = strText.find(itor->wFrom)) != std::wstring::npos)
		{
			// Finding....
			strText.replace(kFindPos, itor->wFrom.size(), itor->wTo);
		}
		++itor;
	}
}