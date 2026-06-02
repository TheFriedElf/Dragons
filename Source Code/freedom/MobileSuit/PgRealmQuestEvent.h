#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_REALMQUEST_PGREALMQUESTEVENT_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_REALMQUEST_PGREALMQUESTEVENT_H

//
class PgRealmQuestNpcEvent
{
public:
	typedef std::list< std::string > CONT_NODE_NAME;

public:
	PgRealmQuestNpcEvent();
	~PgRealmQuestNpcEvent();

	bool ParseXml(TiXmlElement const* pkRoot);

private:
	CLASS_DECLARATION_NO_SET(size_t, m_iCount, Count);
	CLASS_DECLARATION_NO_SET(BM::GUID, m_kNpcGuid, NpcGuid);
	CLASS_DECLARATION_NO_SET(CONT_NODE_NAME, m_kContNodeName, ContNodeName);
	CLASS_DECLARATION_NO_SET(int, m_iNoticeTextTableID, NoticeTextTableID);
};
typedef std::list< PgRealmQuestNpcEvent > CONT_REALM_QUEST_NPC_EVENT;

//
class PgRealmQuestEvent
{
public:
	PgRealmQuestEvent();
	PgRealmQuestEvent(PgRealmQuestEvent const& rhs);
	~PgRealmQuestEvent();

	std::wstring const& RealmQuestID() const		{ return m_kRealmQuestInfo.kRealmQuestID; }
	bool ParseXml(TiXmlElement const* pkRoot);
	void UpdateCount(SRealmQuestInfo const& rkRealmQuestInfo);
	bool IsLikedNpc(BM::GUID const& rkNpcGuid) const;

private:
	CLASS_DECLARATION_S_NO_SET(int, DescTextID);
	CLASS_DECLARATION_S_NO_SET(int, RewardTextID);
	CLASS_DECLARATION_S_NO_SET(int, TitleTextID);
	CLASS_DECLARATION_S_NO_SET(int, BuffFailTextID);
	CLASS_DECLARATION_S_NO_SET(int, BuffMinuteTextID);
	CLASS_DECLARATION_S_NO_SET(int, BuffSecTextID);
	CLASS_DECLARATION_S_NO_SET(int, BuffNowTextID);
	CLASS_DECLARATION_S_NO_SET(SRealmQuestInfo, RealmQuestInfo);
	CONT_REALM_QUEST_NPC_EVENT m_kContNpcEvent;

	bool ParseXmlImp(TiXmlElement const* pkSub, bool const bUseLocal);
};
typedef std::map< std::wstring , PgRealmQuestEvent > CONT_REALM_QUEST_EVENT;

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_REALMQUEST_PGREALMQUESTEVENT_H