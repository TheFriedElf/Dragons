#include "stdafx.h"
#include "Variant/PgStringUtil.h"
#include "Variant/Global.h"
#include "PgRealmQuestEvent.h"

#include "NiAVObject.h"
#include "PgWorld.h"
#include "PgActor.h"
#include "PgActorNpc.h"
#include "PgChatMgrClient.h"

namespace RealmQuestEventUtil
{
	void ParseError(TiXmlElement const* pkNode)
	{
		if( pkNode )
		{
#ifndef USE_INB
			TiXmlDocument const* pkDocu = pkNode->GetDocument();
			std::string const kDocuName( ((pkDocu)? pkDocu->Value(): std::string()) );
			BM::vstring kTempStr(BM::vstring(L"Wrong XML Element[")<<pkNode->Value()<<L"] Row["<<pkNode->Row()<<L"] Col["<<pkNode->Column()<<L"] File["<<kDocuName<<L"]");
			std::wstring const& rkStrTemp = kTempStr;
			//::MessageBox(NULL, rkStrTemp.c_str(), GetClientNameW(), MB_OK);
			_PgMessageBox(GetClientName(), MB(rkStrTemp));
#endif
		}
	}
};

//
PgRealmQuestNpcEvent::PgRealmQuestNpcEvent()
	: m_iCount(0), m_kNpcGuid(), m_kContNodeName(), m_iNoticeTextTableID(0)
{
}
PgRealmQuestNpcEvent::~PgRealmQuestNpcEvent()
{
}

bool PgRealmQuestNpcEvent::ParseXml(TiXmlElement const* pkRoot)
{
	if( !pkRoot )
	{
		return false;
	}

	m_iCount = static_cast< size_t >(PgStringUtil::SafeAtoi(pkRoot->Attribute("COUNT")));
	m_kNpcGuid = BM::GUID(PgStringUtil::SafeUni(pkRoot->Attribute("GUID")));

	bool bRet = true;
	TiXmlElement const* pkSub = pkRoot->FirstChildElement();
	while( pkSub )
	{
		char const* szName = pkSub->Value();
		char const* szText = pkSub->GetText();

		if( 0 == strcmp("NODE", szName) )
		{
			if( szText )
			{
				m_kContNodeName.push_back( std::string(szText) );
			}
			else
			{
				RealmQuestEventUtil::ParseError(pkSub);
				bRet = false;
			}
		}
		else if( 0 == strcmp("NOTICE", szName) )
		{
			m_iNoticeTextTableID = PgStringUtil::SafeAtoi(szText);
		}
		else
		{
			RealmQuestEventUtil::ParseError(pkSub);
			bRet = false;
		}
		pkSub = pkSub->NextSiblingElement();
	}

	if( 0 == m_iCount
	||	BM::GUID::IsNull(m_kNpcGuid) )
	{
		return false;
	}
	return bRet;
}


//
PgRealmQuestEvent::PgRealmQuestEvent()
	: m_kRealmQuestInfo(), m_kContNpcEvent()
	, m_kDescTextID(0), m_kRewardTextID(0), m_kTitleTextID(0)
	, m_kBuffFailTextID(0), m_kBuffMinuteTextID(0), m_kBuffSecTextID(0), m_kBuffNowTextID(0)
{
}
PgRealmQuestEvent::PgRealmQuestEvent(PgRealmQuestEvent const& rhs)
	: m_kRealmQuestInfo(rhs.m_kRealmQuestInfo), m_kContNpcEvent(rhs.m_kContNpcEvent)
	, m_kDescTextID(rhs.m_kDescTextID), m_kRewardTextID(rhs.m_kRewardTextID), m_kTitleTextID(rhs.m_kTitleTextID)
	, m_kBuffFailTextID(rhs.m_kBuffFailTextID), m_kBuffMinuteTextID(rhs.m_kBuffMinuteTextID), m_kBuffSecTextID(rhs.m_kBuffSecTextID), m_kBuffNowTextID(rhs.m_kBuffNowTextID)
{
}
PgRealmQuestEvent::~PgRealmQuestEvent()
{
}

bool PgRealmQuestEvent::ParseXml(TiXmlElement const* pkRoot)
{
	if( !pkRoot )
	{
		return false;
	}

	m_kRealmQuestInfo.kRealmQuestID = PgStringUtil::SafeUni(pkRoot->Attribute("ID"));

	TiXmlElement const* pkSub = pkRoot->FirstChildElement();
	return ParseXmlImp(pkSub, false);
}

bool PgRealmQuestEvent::ParseXmlImp(TiXmlElement const* pkSub, bool const bUseLocal)
{
	bool bRet = true;

 	while( pkSub )
	{
		char const* szName = pkSub->Value();
		char const* szValue = pkSub->GetText();

		if( 0 == strcmp("LOCAL", szName) )
		{
			TiXmlNode const* pkFindLocalNode = PgXmlLocalUtil::FindInLocal(g_kLocal, pkSub, "PgRealmQuestEvent");
			if( pkFindLocalNode )
			{
				TiXmlElement const* pkResultEle = pkFindLocalNode->FirstChildElement();
				if( pkResultEle )
				{
					do
					{//! switch문 다음에 pkNode->NextSibling(); 호출하므로, 
						//! 여기서는 재귀호출만 해주면 된다.
						;
					}
					while(ParseXmlImp( pkResultEle, true ));
				}
			}
		}
		else if( 0 == strcmp("NPC", szName) )
		{
			CONT_REALM_QUEST_NPC_EVENT::value_type kNpcEvent;
			if( !kNpcEvent.ParseXml(pkSub) )
			{
				bRet = false;
			}
			else
			{
				m_kContNpcEvent.push_back( kNpcEvent );
			}
		}
		else if( 0 == strcmp("QUEST_DESC_TEXT", szName) )			
		{ 
			m_kDescTextID = PgStringUtil::SafeAtoi(szValue); 
		}
		else if( 0 == strcmp("QUEST_REWARD_TEXT", szName) )			{ m_kRewardTextID = PgStringUtil::SafeAtoi(szValue); }
		else if( 0 == strcmp("QUEST_TITLE_TEXT", szName) )			{ m_kTitleTextID = PgStringUtil::SafeAtoi(szValue); }
		else if( 0 == strcmp("BUFF_FAIL", szName) )					{ m_kBuffFailTextID = PgStringUtil::SafeAtoi(szValue); }
		else if( 0 == strcmp("BUFF_MINUTE", szName) )				{ m_kBuffMinuteTextID = PgStringUtil::SafeAtoi(szValue); }
		else if( 0 == strcmp("BUFF_SEC", szName) )					{ m_kBuffSecTextID = PgStringUtil::SafeAtoi(szValue); }
		else if( 0 == strcmp("BUFF_NOW", szName) )					{ m_kBuffNowTextID = PgStringUtil::SafeAtoi(szValue); }
		else
		{
			RealmQuestEventUtil::ParseError(pkSub);
			bRet = false;
		}

		pkSub = pkSub->NextSiblingElement();
		if (NULL==pkSub && true==bUseLocal)
		{
			bRet = false;
		}
	}

	if( m_kRealmQuestInfo.kRealmQuestID.empty() )
	{
		RealmQuestEventUtil::ParseError(pkSub);
		return false;
	}
	return bRet;
}
void PgRealmQuestEvent::UpdateCount(SRealmQuestInfo const& rkRealmQuestInfo)
{
	if( !g_pkWorld )
	{
		return ;
	}

	CONT_REALM_QUEST_NPC_EVENT::const_iterator iter = m_kContNpcEvent.begin();
	while( m_kContNpcEvent.end() != iter )
	{
		CONT_REALM_QUEST_NPC_EVENT::value_type const& rkRealmQuestEvent = (*iter);
		bool const bOldVisible = m_kRealmQuestInfo.iCurCount >= rkRealmQuestEvent.Count();
		bool const bVisible = rkRealmQuestInfo.iCurCount >= rkRealmQuestEvent.Count();
		PgIWorldObject* pkNpcObject = g_pkWorld->FindObject(rkRealmQuestEvent.NpcGuid());
		if( pkNpcObject )
		{
			PgActorNpc* pkNpcActor = NiDynamicCast(PgActorNpc, pkNpcObject);
			if( pkNpcActor )
			{
				PgRealmQuestNpcEvent::CONT_NODE_NAME::const_iterator node_iter = rkRealmQuestEvent.ContNodeName().begin();
				while( rkRealmQuestEvent.ContNodeName().end() != node_iter )
				{
					PgRealmQuestNpcEvent::CONT_NODE_NAME::value_type const& rkNodeName = (*node_iter);
					pkNpcActor->SetNodeHide(rkNodeName.c_str(), !bVisible);
					//if( false == bOldVisible
					//&&	true == bVisible )
					//{
					//	float const fStartAlpha = 0.f;
					//	float const fEndAlpha = 1.f;
					//	float const fChangedAlphaSec = 1.f;
					//	pkNpcActor->SetNodeAlphaChange(rkNodeName.c_str(), fStartAlpha, fEndAlpha, fChangedAlphaSec);
					//}
					++node_iter;
				}
			}
		}

		if( 0 != rkRealmQuestEvent.NoticeTextTableID()
		&&	false == bOldVisible
		&&	true == bVisible )
		{
			std::wstring const kNoticeMessage = TTW(rkRealmQuestEvent.NoticeTextTableID());
			if( !kNoticeMessage.empty() )
			{
				int const iNoticeLevel = 3;
				g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT_SYSTEM), kNoticeMessage, true, iNoticeLevel);
			}
		}
		++iter;
	}

	m_kRealmQuestInfo = rkRealmQuestInfo;
}
bool PgRealmQuestEvent::IsLikedNpc(BM::GUID const& rkNpcGuid) const
{
	CONT_REALM_QUEST_NPC_EVENT::const_iterator iter = m_kContNpcEvent.begin();
	while( m_kContNpcEvent.end() != iter )
	{
		if( (*iter).NpcGuid() == rkNpcGuid )
		{
			return true;
		}
		++iter;
	}
	return false;
}