#include "StdAfx.h"
#include "PgCommandMgr.h"

bool PgCommandMgr::FindCommand(std::wstring const &InWord, std::wstring const* &pOut) const//커멘드를 찾아
{
	SCRIPT_HASH::const_iterator itrCommand = m_kCommand.begin();

	while(itrCommand != m_kCommand.end())
	{
		CONT_WORD const &rkContWord = itrCommand->first;
		for(CONT_WORD::const_iterator itr = rkContWord.begin(); itr != rkContWord.end(); ++itr)
		{
			std::wstring::size_type const pos = InWord.find(*itr);

			if( std::wstring::npos != pos)
			{
				pOut = &itrCommand->second;
				return true;
			}
		}
		++itrCommand;
	}
	return false;
}

bool PgCommandMgr::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	while(pkNode)
	{
		switch(iType)
		{
		case TiXmlNode::ELEMENT:
			{
				TiXmlElement *pkElement = (TiXmlElement *)pkNode;
				PG_ASSERT_LOG(pkElement);
				
				char const *pcTagName = pkElement->Value();

				if(strcmp(pcTagName, "LINE_COMMAND")==0)
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
				else if(strcmp(pcTagName, "TEXT") == 0)
				{
					const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					unsigned long ulTextID = 0;
					NiString kText;
					char const *strScript = 0;
					while(pkAttr)
					{
						char const *pcAttrName = pkAttr->Name();
						char const *pcAttrValue = pkAttr->Value();

						if(strcmp(pcAttrName, "ID") == 0)
						{
							ulTextID = (unsigned long)atol(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "INPUT") == 0)
						{
							kText = pcAttrValue;
						}
						else
						{
							PG_ASSERT_LOG(!"invalid attribute");
						}

						pkAttr = pkAttr->Next();
					}				

					strScript = pkElement->GetText();

					CONT_WORD kCONT_WORD;
					char const *pcDelim = ",";
					char *pcNextToken = 0;
					char *pcText = kText.MakeExternalCopy();
					char *pcToken = strtok_s(pcText, pcDelim, &pcNextToken);
					while(pcToken)
					{
						// 앞 뒤의 마지막 공백 문자를 없앤다.
						NiString kToken(pcToken);
						kToken.TrimLeft();
						kToken.TrimRight();
						if(kToken.Length())
						{
							kCONT_WORD.push_back(UNI((char const *)kToken));
						}
						pcToken = strtok_s(0, pcDelim, &pcNextToken);
					}

					NiFree(pcText);
					std::pair<SCRIPT_HASH::iterator, bool> ret = m_kCommand.insert(std::make_pair(kCONT_WORD, UNI(strScript)));
					if(!ret.second)
					{
						char str[100];
						sprintf_s(str, 100, "Same LineCommand Table ID [%u] Exist", ulTextID);
						//::MessageBox(0, UNI(str), _T("LineCommand Table Loading Error"), MB_OK);
						_PgMessageBox("LineCommand Table Loading Error", str );
					}
				}
			}
		}

		const TiXmlNode* pkNextNode = pkNode->NextSibling();
		pkNode = pkNextNode;
	}
	return	true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PgCilentFilterString::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int iXmlType = 0;	
	int const iType = pkNode->Type();	
	while(pkNode)
	{
		switch(iType)
		{
		case TiXmlNode::ELEMENT:
			{
				TiXmlElement const* pkElement = (TiXmlElement const*)pkNode;
				assert(pkElement);				
				char const *pcTagName = pkElement->Value();
				if(_stricmp(pcTagName, "BAD_WORD")==0)
				{// BadWord 필터링 방식
					iXmlType = 1;
					if( !BM::PgFilterString::ParseXml(pkNode, bUTF8) )
					{
						return false;
					}
					//TiXmlNode const* pkLastNode = pkNode->LastChild();
					//pkNode = pkLastNode;
					//continue;
				}
				else if(_stricmp(pcTagName, "BLOCK_NICK_NAME")==0)
				{// BlockNickName 필터링 방식
					iXmlType = 3;
					
					if( !BM::PgFilterString::ParseXml(pkNode, bUTF8) )
					{
						return false;
					}
					TiXmlNode const* pkLastNode = pkNode->LastChild();
					pkNode = pkLastNode;
					continue;
				}
				else if(_stricmp(pcTagName, "REGEX")==0)
				{// RegEX 필터링 방식을 얻어온다
					TiXmlNode const* pkChildNode = pkNode->FirstChild();
					pkNode = pkChildNode;
					continue;
				}	
				else if(_stricmp(pcTagName, "TEXT") == 0)
				{
					const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
					unsigned long Lv=0;
					char const	*strText=0;
					while(pkAttr)
					{

						char const *pcAttrName = pkAttr->Name();
						char const *pcAttrValue = pkAttr->Value();

						if(_stricmp(pcAttrName, "LV") == 0)
						{
							Lv = (unsigned long)atol(pcAttrValue);
						}
						else if(_stricmp(pcAttrName, "Text") == 0)
						{
							strText=pcAttrValue;
						}
						else
						{
							assert(!"invalid attribute");
						}

						pkAttr = pkAttr->Next();
					}

					std::wstring uniText;
					if (bUTF8)
					{
						uniText = BM::vstring::ConvToUnicodeFromUTF8(strText);
					}
					else
					{
						uniText = UNI(strText);
					}

					switch(iXmlType)
					{
					case 1:
						{
							if(m_kBadWordPattern.empty())
							{
								try
								{// 정규 표현식이 올바르지 않으면 예외를 던지므로 try 사용
									m_kBadWordPattern = uniText;	
									std::wregex kTemp(m_kBadWordPattern);
									std::regex_constants::syntax_option_type eFlag = std::regex_constants::icase
																					   | std::regex_constants::nosubs;
									m_kBadWordRegex.assign(m_kBadWordPattern, eFlag);
								}
								catch(...)
								{// 올바르지 않은 정규 표현식이 들어왔다면
									MessageBox(NULL,L"BadWord pattern is invalid regex", L"BadWord parsing error", MB_ICONERROR);
								}
							}
							else
							{// 패턴이 이미 등록 되었다면 알려준다
								std::wstring kTemp = L"BadWord Pattern already inputed\nCant use this pattern:";
								kTemp += uniText;
								MessageBox(NULL, kTemp.c_str(), L"ERROR", MB_ICONERROR);
							}
						}break;	
					case 3:
						{
							if(m_kBlockNickNamePattern.empty())
							{
								try
								{// 정규 표현식이 올바르지 않으면 예외를 던지므로 try 사용
									m_kBlockNickNamePattern = uniText;
									std::wregex kTemp(m_kBlockNickNamePattern);
									std::regex_constants::syntax_option_type eFlag = std::regex_constants::icase
																					   | std::regex_constants::nosubs;
									m_kBlockNickNameRegex.assign(m_kBlockNickNamePattern, eFlag);
								}
								catch(...)
								{// 올바르지 않은 정규 표현식이 들어왔다면
									MessageBox(NULL,L"BlockNickName pattern is invalid regex", L"BlockNickName parsing error", MB_ICONERROR);
								}
							}
							else
							{// 패턴이 이미 등록 되었다면 알려준다
								std::wstring kTemp = L"BlockNickName Pattern already inputed\nCant use this pattern:";
								kTemp += uniText;
								MessageBox(NULL, kTemp.c_str(), L"ERROR", MB_ICONERROR);
							}
						}break;
					default:
						{
						}break;
					}
				}
				else				
				{// 이외의 다른 요소들은 동일하게 동작하게 한다
					if(!BM::PgFilterString::ParseXml(pkNode, bUTF8))
					{
						return false;
					}
				}
			}
		}		
		pkNode = pkNode->NextSibling();
	}

	
	return true;
}

bool PgCilentFilterString::Filter(std::wstring &kStr, bool const bIsConvert, EFilterSourceType const eFilterType)const
{
	//기존의 필터
	if( true == BM::PgFilterString::Filter(kStr, bIsConvert, eFilterType) )
	{
		return true;
	}
	//RegEx 방식 필터
	if(!kStr.empty())
	{
		std::wstring kCopyStr = kStr;
		bool bRetBad = false;
		if(eFilterType & FST_BADWORD)
		{// BadWord
			bRetBad = RegExFilter(kCopyStr, m_kBadWordRegex);
		}
		bool bRetNick = false;
		if(eFilterType & FST_NICKNAME)
		{// BlockNickName
			bRetNick = RegExFilter(kCopyStr, m_kBlockNickNameRegex);
		}

		if(bRetBad || bRetNick)
		{// 필터에 걸렸고,
			if(bIsConvert)
			{//  변환해야 된다면 글자를 바꾼다.
				kStr = kCopyStr;
			}
			return true;
		}
	}
	return false;
}

bool PgCilentFilterString::RegExFilter(std::wstring& kStr, std::wregex const& kRegEx) const
{
	static std::wsregex_iterator const itor_regex_end;
	bool bRet = false;
	try
	{
		std::wsregex_iterator itor_regex_start(kStr.begin(),kStr.end(), kRegEx);
		while (itor_regex_start != itor_regex_end)
		{
			size_t const& BeginIndex = itor_regex_start->position();
			size_t const& StrLength = itor_regex_start->length();
			// 시작index : BeginIndex
			// 문자길이  : StrLength
			// 분리된문장: std::string kStr(kInputStr, BeginIndex, StrLength);

			{//문장 교체
				std::wstring kTempReplace = GetGoodString(StrLength);
				kStr.replace(BeginIndex, StrLength, kTempReplace.c_str());
				bRet = true;
			}
			++itor_regex_start;
		}
		return bRet;
	}
	catch(...)
	{
		return false;
	}
}
