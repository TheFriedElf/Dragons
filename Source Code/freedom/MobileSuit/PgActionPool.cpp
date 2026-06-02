#include "StdAfx.h"
#include "PgActionPool.h"
#include "PgAction.h"
#include "PgXmlLoader.h"
#include "PgResourceMonitor.h"

PgActionPool::PgActionPool()
{
	m_kElementPool.Init(50,100);
}

PgActionPool::~PgActionPool()
{
}

void PgActionPool::Destroy()
{
	BM::CAutoMutex kLock(m_kMutex);

	Container::iterator itr = m_kContainer.begin();
	while(itr != m_kContainer.end())
	{
		PG_ASSERT_LOG(itr->second);

		PgAction	*pkAction = itr->second;

		PgXmlLoader::ReleaseXmlDocumentInCacheByID(pkAction->GetID().c_str());
		pkAction->DeleteParamPacket();
		pkAction->Clear();
		m_kElementPool.Delete(pkAction);
		++itr;
	}

	m_kContainer.clear();
	m_kElementPool.Terminate();
}

PgAction* PgActionPool::CreateAction(char const *pcActionID, bool bWarning)
{
	BM::CAutoMutex kLock(m_kMutex);

	PgAction *pkAction = NULL;

	if (pcActionID && strlen(pcActionID) > 0)
	{
		char	strActionID[100] = { 0, };
		strncpy_s(strActionID, 100, pcActionID, 100);

		Container::const_iterator itr = m_kContainer.find(strActionID);
		
		if(itr == m_kContainer.end())
		{
			pkAction = dynamic_cast<PgAction *>(PgXmlLoader::CreateObject(strActionID, NULL, NULL, NULL, 1));
			if(!pkAction) 
			{
				return 0;
			}
			
			itr = m_kContainer.insert(std::make_pair(strActionID, pkAction)).first;

			if(pkAction->GetID() != (std::string)pcActionID)
			{
				NILOG(PGLOG_WARNING, "Action Name isn't equal\n");
				return 0;
			}
		}
		pkAction = m_kElementPool.New();
		//pkAction->Clear();
		pkAction->Init(itr->second);
		PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(pcActionID, PgResourceMonitor::RESOURCE_TYPE_ACTION));
	}
	else
	{
		if (bWarning)
		{
			NILOG(PGLOG_WARNING, "[PgActionPool] action id is null or length is zero\n");
		}

		pkAction = m_kElementPool.New();
		pkAction->Clear();
	}

	return pkAction;
}

void PgActionPool::ReleaseAction(PgAction*& pkAction)
{
	BM::CAutoMutex kLock(m_kMutex);

	if(pkAction)
	{
		pkAction->GetTargetList()->ApplyActionEffects(true, false, 0.0f, true);
		pkAction->DeleteParamPacket();
		pkAction->Clear();
		m_kElementPool.Delete(pkAction);
	}
}

void PgActionPool::LoadActionBeforeUse()
{// 액션 사용시 랙을 줄이기 위해 일부 액션을 미리 읽어 놓는다
	BM::CAutoMutex kLock(m_kMutex);
	{
		std::string kPath("../XML/PrepareActionList.xml");
		TiXmlDocument kXmlDoc( kPath.c_str() );
		if(!PgXmlLoader::LoadFile( kXmlDoc, UNI(kPath.c_str())) )
		{
			PgError1("Parse Failed [%s]", kPath.c_str());
			return;
		}
		
		TiXmlElement const* pkElement = kXmlDoc.FirstChildElement();
		pkElement = pkElement->FirstChildElement();
		while(pkElement)
		{
			std::string kActionID;
			char const *pcTagName = pkElement->Value();
			
			if(strcmp(pcTagName, "ACTION") == 0)
			{
				char const *pcID = 0;
				//char const *pcText = pkElement->GetText();
				TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();

				while(pkAttr)
				{
					char const *pcAttrName = pkAttr->Name();
					char const *pcAttrValue = pkAttr->Value();

					if(strcmp(pcAttrName, "ID") == 0)
					{
						if(pcAttrValue)
						{
							kActionID = pcAttrValue;
						}
					}
					pkAttr = pkAttr->Next();
				}
				//pcPath = pkElement->GetText(); <ACTION>요기부분을 GetText()로 얻어올수 있음</ACTION>
				
				PgAction* pkAction = dynamic_cast<PgAction *>( PgXmlLoader::CreateObject(kActionID.c_str()) );
				if(pkAction) 
				{
					Container::iterator itr = m_kContainer.insert(std::make_pair(kActionID, pkAction)).first;
					if(pkAction->GetID() != kActionID)
					{
						NILOG(PGLOG_WARNING, "Action Name isn't equal\n");
					}
				}
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
			}
			pkElement = pkElement->NextSiblingElement();
		}
	}
}