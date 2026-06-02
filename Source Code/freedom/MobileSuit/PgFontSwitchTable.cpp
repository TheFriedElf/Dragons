#include "StdAfx.h"
#include "variant/PgStringUtil.h"
#include "PgFontSwitchTable.h"

PgFontSwitchTable::PgFontSwitchTable(void)
{
	ClearAllData();
}

PgFontSwitchTable::~PgFontSwitchTable(void)
{
	ClearAllData();
}

bool PgFontSwitchTable::ParseXML(char const* strXMLPath)
{
	if( strXMLPath )
	{
		TiXmlDocument	TiDoc;
		if( !PgXmlLoader::LoadFile(TiDoc, UNI(strXMLPath)) )
		{
			return false;
		}

		TiXmlElement* TiElem = TiDoc.FirstChildElement();
		if( TiElem )
		{
			TiXmlNode* pNode = dynamic_cast<TiXmlNode*>(TiElem);
			assert(pNode);
			if( ExplorerNode(pNode) )
			{
				return true;
			}
		}
	}
	
	return false;
}

bool PgFontSwitchTable::ExplorerNode(TiXmlNode const* pNode, bool const bSysFont)
{
	if( pNode )
	{
		int const iType = pNode->Type();
		while( pNode )
		{
			switch(iType)
			{
			case TiXmlNode::ELEMENT:
				{
					TiXmlElement *pkElement = (TiXmlElement *)pNode;
					assert(pkElement);
					char const* pcTagName = pkElement->Value();

					if(strcmp(pcTagName, "FONT_SWITCH_TABLE") == 0)
					{
						TiXmlNode const* pkChildNode = pNode->FirstChild();
						if(pkChildNode != NULL)
						{
							if(!ExplorerNode(pkChildNode))
							{
								return false;
							}
						}
					}
					if(strcmp(pcTagName, "SYS_FONT_SWITCH_TABLE") == 0)
					{
						TiXmlNode const* pkChildNode = pNode->FirstChild();
						if(pkChildNode != NULL)
						{
							if(!ExplorerNode(pkChildNode, true))
							{
								return false;
							}
						}
					}
					else if(strcmp(pcTagName, "ATTR") == 0)
					{
						bool bRegist = true;
						const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
						std::string kKey;
						std::string kSysName;
						std::string kTransKey;
						while(pkAttr)//
						{
							const char *pcAttrName	= pkAttr->Name();
							const char *pcAttrValue	= pkAttr->Value();

							if( strcmp(pcAttrName, "LOCAL") == 0)
							{
								if( pcAttrValue )
								{
									std::string kLocal(pcAttrValue);
									VEC_STRING kVec;
									PgStringUtil::BreakSep(kLocal, kVec, "/");
									if( !kVec.empty() )
									{
										bool bFoundRegion = false;
										VEC_STRING::const_iterator iter = kVec.begin();
										while( kVec.end() != iter )
										{
											int const iRegionCode = PgStringUtil::SafeAtoi( (*iter) );
											if( 0 == iRegionCode )
											{
												bFoundRegion = true;
												break;
											}
											else
											{
												bFoundRegion = g_kLocal.IsServiceRegion(iRegionCode) || bFoundRegion;
											}
											++iter;
										}
										bRegist = bFoundRegion;
									}
								}
							}
							else if(strcmp(pcAttrName, "KEY") == 0)
							{
								kKey = pcAttrValue;
							}
							else if(strcmp(pcAttrName, "NAME") == 0)
							{
								kSysName = pcAttrValue;
							}
							else if(strcmp(pcAttrName, "TRANS_KEY") == 0)
							{
								kTransKey = pcAttrValue;
							}
							pkAttr = pkAttr->Next();
						}

						if( bRegist )
						{
							if( bSysFont )
							{
								std::pair<kSysFontSwitchTable::iterator, bool> bRet = m_kSysCont.insert(std::make_pair(S_SYS_EMO_KEY(atoi(kKey.c_str()), kSysName), atoi(kTransKey.c_str())));
								if( !bRet.second )
								{
									PgError1("SysFontSwitchTable Overlap No[%s]", kKey.c_str());
								}
							}
							else
							{
								std::pair<kFontSwitchTable::iterator, bool> bRet = m_kCont.insert(std::make_pair(atoi(kKey.c_str()), atoi(kTransKey.c_str())));
								if( !bRet.second )
								{
									PgError1("FontSwitchTable Overlap No[%s]", kKey.c_str());
								}
							}
						}
					}
				}break;
			}
			const TiXmlNode* pNextNode = pNode->NextSibling();
			pNode = pNextNode;
		}
	}

	return true;
}

void PgFontSwitchTable::ClearAllData()
{
	m_kCont.clear();	
}

bool PgFontSwitchTable::Trans_key_value(int const Key, int& TransKey)
{
	kFontSwitchTable::iterator	iter = m_kCont.find(Key);
	if( iter != m_kCont.end() )
	{
		TransKey = iter->second;
		return true;
	}

	return false;
}

std::wstring PgFontSwitchTable::Trans_Key_SysFontString(S_SYS_EMO_KEY const& Key)
{
	kSysFontSwitchTable::iterator	iter = m_kSysCont.find(Key);
	if( iter != m_kSysCont.end() )
	{
		kSysFontSwitchTable::mapped_type const& kTransKey = iter->second;
		
		BM::vstring kString(L"{T=");
		kString += XUI::XUI_SAVE_FONT;
		kString += _T("/C=");
		kString += XUI::XUI_SAVE_COLOR;
		kString += _T("/}");
		kString += _T("{C=0xFFFFFFFF/T=SysEmoFont/}");
		kString += static_cast<wchar_t>(kTransKey);
		kString += _T("{T=");
		kString += XUI::XUI_RESTORE_FONT;
		kString += _T("/C=");
		kString += XUI::XUI_RESTORE_COLOR;
		kString += _T("/}");

		return kString.operator const std::wstring &();
	}
	return std::wstring(L"");
}

std::wstring PgFontSwitchTable::Trans_Key_SysFontString(int const Key)
{
	return Trans_Key_SysFontString(S_SYS_EMO_KEY(Key));
}

std::wstring PgFontSwitchTable::Trans_Key_SysFontString(std::string const& Key)
{
	kSysFontSwitchTable::iterator	iter = m_kSysCont.begin();
	while( iter != m_kSysCont.end() )
	{
		kSysFontSwitchTable::key_type const& kKey = iter->first;
		kSysFontSwitchTable::mapped_type const& kTransKey = iter->second;

		if( kKey == Key )
		{
			BM::vstring kString(L"{T=");
			kString += XUI::XUI_SAVE_FONT;
			kString += _T("/C=");
			kString += XUI::XUI_SAVE_COLOR;
			kString += _T("/}");
			kString += _T("{C=0xFFFFFFFF/T=SysEmoFont/}");
			kString += static_cast<wchar_t>(kTransKey);
			kString += _T("{T=");
			kString += XUI::XUI_RESTORE_FONT;
			kString += _T("/C=");
			kString += XUI::XUI_RESTORE_COLOR;
			kString += _T("/}");
			return kString.operator const std::wstring &();
		}
		++iter;
	}
	return std::wstring(L"");
}