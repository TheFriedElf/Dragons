#include "StdAfx.h"
#include "PgResourceIcon.h"

PgResourceIcon::PgResourceIcon(void)
{
}

PgResourceIcon::~PgResourceIcon(void)
{
}

bool PgResourceIcon::Initialize()
{
	return true;
}

void PgResourceIcon::Terminate()
{
}

bool PgResourceIcon::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "RESOURCE_ICON") == 0)
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
			else if(strcmp(pcTagName, "IMG_LIST") == 0)
			{
				// 내부 들어가야 함.
				const TiXmlNode * pkChildNode = pkNode->FirstChild();
				if(pkChildNode != 0)
				{
					if(!ParseXml_IconImage(pkChildNode, pArg))
					{
						return false;
					}
				}
			}
			else if(strcmp(pcTagName, "ICON") == 0)
			{
				// 단일.
				SResourceIcon kRscIcon;
				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "ID") == 0)
					{
						kRscIcon.wstrID = UNI(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IMG_ID") == 0)
					{
						kRscIcon.wstrImageID = UNI(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "IDX") == 0)
					{
						kRscIcon.iIdx = (int)atoi(pkAttr->Value());
					}
					pkAttr = pkAttr->Next();
				}

				m_kIconCont.insert(std::make_pair(kRscIcon.wstrID, kRscIcon));
			}
			else
			{
				NILOG(PGLOG_WARNING, "[PgWorld] unknown %s node in %s world\n", pcTagName, GetID().c_str());
			}
		}

	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml(pkNextNode))
		{
			return false;
		}
	}

	// xml 파싱이 성공적으로 끝났다면,
	if(strcmp(pkNode->Value(), "RESOURCE_ICON") == 0)
	{
		// ?
	}

	return true;
}

bool PgResourceIcon::ParseXml_IconImage(const TiXmlNode *pkNode, void *pArg)
{
	int const iType = pkNode->Type();
	
	switch(iType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
			PG_ASSERT_LOG(pkElement);
			char const *pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "IMG") == 0)
			{
				//
				SIconImage kIconImage;
				kIconImage.wstrPath = UNI(pkElement->GetText());

				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
				while(pkAttr)
				{
					if(strcmp(pkAttr->Name(), "ID") == 0)
					{
						kIconImage.wstrID = UNI(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "WIDTH") == 0)
					{
						kIconImage.iWidth = (int)atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "HEIGHT") == 0)
					{
						kIconImage.iHeight = (int)atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "MAX_X") == 0)
					{
						kIconImage.iMaxX = (int)atoi(pkAttr->Value());
					}
					else if(strcmp(pkAttr->Name(), "MAX_Y") == 0)
					{
						kIconImage.iMaxY = (int)atoi(pkAttr->Value());
					}
					pkAttr = pkAttr->Next();
				}

				m_kIconImageCont.insert(std::make_pair(kIconImage.wstrID, kIconImage));
			}
		}
	default:
		break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pkNextNode = pkNode->NextSibling();
	if(pkNextNode)
	{
		if(!ParseXml_IconImage(pkNextNode, pArg))
		{
			return false;
		}
	}
	return true;
}

bool PgResourceIcon::GetIcon(SResourceIcon& rkRetIcon, std::wstring wstrID)
{
	IconCont::iterator find_itr = m_kIconCont.find(wstrID);
	if (find_itr == m_kIconCont.end())
	{
		// 물음표 띄우자?
		rkRetIcon.wstrID = L"NONE_ICON";
		rkRetIcon.wstrImageID = L"ICON_PATH_MON";
		rkRetIcon.iIdx = 43;
		return true;
	}

	rkRetIcon = (*find_itr).second;
	return true;
}

bool PgResourceIcon::GetIconImage(SIconImage& rkRetIconImage, std::wstring wstrID)
{
	IconImageCont::iterator find_itr = m_kIconImageCont.find(wstrID);
	if (find_itr == m_kIconImageCont.end())
	{
		return false;
	}

	rkRetIconImage = (*find_itr).second;
	return true;
}

bool PgResourceIcon::AddIconID(std::wstring wstrActorName, std::wstring wstrXMLFilePath, std::wstring wstrID)
{
	BM::CAutoMutex kLock(m_kIconLock);
	IconIDCont::iterator find_itr = m_kIconID_KeyPath.find(wstrXMLFilePath);
	if (find_itr == m_kIconID_KeyPath.end())
	{
		m_kIconID_KeyPath.insert(std::make_pair(wstrXMLFilePath, wstrID));
	}

	find_itr = m_kIconID_KeyName.find(wstrActorName);
	if (find_itr == m_kIconID_KeyName.end())
	{
		m_kIconID_KeyName.insert(std::make_pair(wstrActorName, wstrID));
	}

	return true;
}

bool PgResourceIcon::GetIconIDFromXMLPath(std::wstring wstrXMLFilePath, std::wstring& rkRetID)
{
	IconIDCont::iterator find_itr = m_kIconID_KeyPath.find(wstrXMLFilePath);
	if (find_itr != m_kIconID_KeyPath.end())
	{
		rkRetID = (*find_itr).second;
		return true;
	}

	UPR(wstrXMLFilePath);
	IconIDCont::iterator itr = m_kIconID_KeyPath.begin();
	while(itr != m_kIconID_KeyPath.end())
	{
		std::wstring rkXml = (*itr).first;
		UPR(rkXml);
		int iCur = rkXml.find(wstrXMLFilePath);
		if (iCur != -1)
		{
			rkRetID = (*itr).second;
			return true;
		}
		++itr;
	}

	return false;
}

bool PgResourceIcon::GetIconIDFromActorName(std::wstring wstrActorName, std::wstring& rkRetID)
{
	IconIDCont::iterator find_itr = m_kIconID_KeyName.find(wstrActorName);
	if (find_itr != m_kIconID_KeyName.end())
	{
		rkRetID = (*find_itr).second;
		return true;
	}

	UPR(wstrActorName);
	IconIDCont::iterator itr = m_kIconID_KeyName.begin();
	while(itr != m_kIconID_KeyName.end())
	{
		std::wstring rkName = (*itr).first;
		UPR(rkName);
		int iCur = rkName.find(wstrActorName);
		if (iCur != -1)
		{
			rkRetID = (*itr).second;
			return true;
		}
		++itr;
	}

	return false;
}
