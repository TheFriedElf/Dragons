#ifndef FREEDOM_DRAGONICA_XML_PGIXMLOBJECT_H
#define FREEDOM_DRAGONICA_XML_PGIXMLOBJECT_H

class PgIXmlObject// : public NiRefObject
{
protected:
	PgIXmlObject();

public:
	//! 일단 필요한 ID만 Open해서 쓴다.
	typedef enum
	{
		ID_NONE,
		////! --- Interfaces ---
		//ID_IACTION,
		//ID_ISCENE,
		////! --- Actors(WorldObjects) --- 
		//ID_ACTOR,
		ID_PC,		// ACTOR의 하위 분류인데 따로 넣어야 하나..
		ID_NPC,
		ID_PET,
		ID_MONSTER,
		ID_BOSS,
		////! --- Action & Animation --- 
		//ID_ACTION,
		//ID_ACTIONINFO,
		//ID_ACTIONSLOT,
		//ID_ACTORSLOT, // 없어진거라고 들었는데?
		//ID_ANIMATIONSET,
		////! --- WorldObjects ---
		ID_PUPPET,
		ID_FURNITURE,
		//ID_DAMAGE_NUM_MANAGER,
		//ID_COMBO_LIST,
		//ID_CLASSINFO,
		ID_DROPBOX,
		ID_ENTITY,
		//ID_EFFECT,
		//ID_PARTICLE,
		//ID_PUPPET,
		//ID_SCREENBREAK,
		////! --- etc ---
		ID_ITEM,
		//ID_PILOT,
		//ID_PROJECTILE,
		//ID_QUEST_XML,
		//ID_SKILL_TREE_CON_POSITION_INFO,
		//ID_TEXT_TABLE,
		//ID_TRIGGER,
		////! --- Managers ---
		//ID_REMOTE_MANAGER,
		//ID_SEASON_MANAGER,
		//ID_LOCAL_MANAGER,
		//ID_MOOD_MANAGER,
		//ID_WORLD_ACTION_MANAGER,
		ID_SUB_PLAYER,
		ID_CUSTOM_UNIT,
		MAX_XML_OBJECT_ID,
	} XmlObjectID;

	virtual ~PgIXmlObject(void);

	//! Node를 파싱한다.
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false) = 0;

	//! 객체의 이름을 반환한다.
	std::string const &GetID()const;
	XmlObjectID GetObjectID() { return m_eID; }		

	//! 객체의 이름을 설정한다.
	void SetID(char const *pcID);
	void SetObjectID(XmlObjectID eID) { m_eID = eID; }

protected:	
	std::string m_kID;
	XmlObjectID m_eID;
};
#endif // FREEDOM_DRAGONICA_XML_PGIXMLOBJECT_H