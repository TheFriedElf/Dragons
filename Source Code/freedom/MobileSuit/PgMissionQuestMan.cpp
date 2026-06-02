#include "stdafx.h"
//#include "PgMissionQuestMan.h"
//
//PgMissionQuestMan::PgMissionQuestMan()
//{
//}
//
//PgMissionQuestMan::~PgMissionQuestMan()
//{
//}
//
//bool PgMissionQuestMan::Parse(std::wstring const &wstrFileName)
//{
//	TiXmlDocument kXmlDoc(MB(wstrFileName));
//
//	if( !PgXmlLoader::LoadFile(kXmlDoc, wstrFileName) )
//	{
//		return false;
//	}
//
//	TiXmlNode*	pNode = kXmlDoc.FirstChild();
//	if( pNode != NULL )
//	{
//		ParseXml(pNode);
//	}
//
//	return	true;
//}
//
//bool PgMissionQuestMan::ParseXml(TiXmlNode const* pkNode)
//{
//	const int iType = pkNode->Type();
//	
//	switch(iType)
//	{
//	case TiXmlNode::ELEMENT:
//		{
//			TiXmlElement *pkElement = (TiXmlElement *)pkNode;
//			PG_ASSERT_LOG(pkElement);
//			const char *pcTagName = pkElement->Value();
//
//			if(strcmp(pcTagName, "MQCARD_ILLUST_PATH") == 0)
//			{
//				const TiXmlNode * pkChildNode = pkNode->FirstChild();
//				if(pkChildNode != 0)
//				{
//					if(!ParseXml(pkChildNode))
//					{
//						return false;
//					}
//				}
//			}
//			else if(strcmp(pcTagName, "BG_LIST") == 0)
//			{
//				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
//				std::wstring wstrFilePath = L"";
//				int iGrade = 0;
//
//				while(pkAttr)
//				{
//					if(strcmp(pkAttr->Name(), "GRADE") == 0)
//					{
//						iGrade = atoi(pkAttr->Value());
//					}
//					else if(strcmp(pkAttr->Name(), "PATH") == 0)
//					{
//						wstrFilePath = UNI(pkAttr->Value());
//					}
//
//					pkAttr = pkAttr->Next();
//				}
//
//				m_MissionBg.insert(std::make_pair(iGrade, wstrFilePath));
//			}
//			else if(strcmp(pcTagName, "ILLUST_LIST") == 0)
//			{
//				// 단일.
//				const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
//				std::wstring wstrFilePath = L"";
//				int	iMissionKey = 0;
//
//				while(pkAttr)
//				{
//					if(strcmp(pkAttr->Name(), "ID") == 0)
//					{
//						iMissionKey = atoi(pkAttr->Value());
//					}
//					else if(strcmp(pkAttr->Name(), "PATH") == 0)
//					{
//						wstrFilePath = UNI(pkAttr->Value());
//					}
//
//					pkAttr = pkAttr->Next();
//				}
//
//				m_MissionIllust.insert(std::make_pair(iMissionKey, wstrFilePath));
//			}
//			else
//			{
//				//	알수 없는 노드
//			}
//		}
//	default:
//		break;
//	}
//
//	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
//	const TiXmlNode* pkNextNode = pkNode->NextSibling();
//	if(pkNextNode)
//	{
//		if(!ParseXml(pkNextNode))
//		{
//			return false;
//		}
//	}
//
//	// xml 파싱이 성공적으로 끝났다면,
//	if(strcmp(pkNode->Value(), "MQCARD_ILLUST_PATH") == 0)
//	{
//		// ?
//	}
//
//	return true;
//}
//
//int PgMissionQuestMan::GetMissionKey(int const iItemNo) const
//{
//	ContMissionQuest::const_iterator	iter = m_kMissionQuest.begin();
//	while( iter != m_kMissionQuest.end() )
//	{
//		if( iter->first.iItemNo == iItemNo )
//		{
//			return iter->first.iMissionKey;
//		}
//
//		++iter;
//	}
//
//	return 0xFFFF;
//}
//
//std::wstring const PgMissionQuestMan::GetMQCardIllustDirToCardID(int const iCardID) const
//{
//	int const iMissionKey = GetMissionKey(iCardID);
//	if( iMissionKey != 0xFFFF )
//	{
//		IllustPathContainer::const_iterator	iter = m_MissionIllust.find(iMissionKey);
//		if( iter != m_MissionIllust.end() )
//		{
//			return iter->second;
//		}
//	}
//
//	return	L"";
//}
//
//std::wstring const PgMissionQuestMan::GetMQCardIllustDirToQuestID(int const iQuestID) const
//{
//	ContMissionQuest::const_iterator	iter = m_kMissionQuest.begin();
//	while( iter != m_kMissionQuest.end() )
//	{
//		if( iter->second.iQuestID == iQuestID )
//		{
//			IllustPathContainer::const_iterator	Path_iter = m_MissionIllust.find(iter->first.iMissionKey);
//			if( Path_iter != m_MissionIllust.end() )
//			{
//				return Path_iter->second;
//			}
//		}
//	}
//
//	return	L"";
//}
//
//std::wstring const PgMissionQuestMan::GetMQCardGradeBg(int const iGrade) const
//{
//	IllustPathContainer::const_iterator	iter = m_MissionBg.find(iGrade);
//	if( iter != m_MissionBg.end() )
//	{
//		return iter->second;
//	}
//
//	return L"";
//}