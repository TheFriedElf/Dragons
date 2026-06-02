#ifndef FREEDOM_DRAGONICA_CONTENTS_BALLOONEMOTICON_PGBALLOONEMOTICONMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_BALLOONEMOTICON_PGBALLOONEMOTICONMGR_H

#include "PgBalloonEmoticonData.h"
#include "PgBalloonEmoticonInstance.h"

namespace lwBalloonEmoticonUtil
{
	void RegisterWrapper(lua_State *pkState);
	void lwBalloonEmoticonSwitch(bool const bPause);
};

class PgBalloonEmoticonMgr
{
public:
	bool Initialize();
	void Update(float fAccumTime,float fFrameTime);
	void DrawImmediate(PgRenderer *pkRenderer);
	void Terminate();

	bool AddNewBalloonEmoticon(PgActor* pActor, int const ID);
	void ClearInstance();
	bool Reload();

	bool IsAlive() const { return m_bAlive; };

	PgBalloonEmoticonMgr(void);
	~PgBalloonEmoticonMgr(void);

protected:
	bool ParseXML(TiXmlElement const* pkElement);
	bool ParseEmoticonAttribute(TiXmlAttribute const* pkAttr, PgBalloonEmoticonData& EmoticonInfo);
	bool ParseFrameAttribute(TiXmlAttribute const* pkAttr, PgBalloonEmoticonFrameData& FrameInfo);
	void EraseInstance();

private:
	typedef std::map< int, PgBalloonEmoticonData >				kDataContainer;
	typedef std::map< BM::GUID, PgBalloonEmoticonInstance* >	kInstanceContainer;
	typedef std::list< PgBalloonEmoticonInstance* >				kEraseContainer;

	CLASS_DECLARATION_S(bool, Pause);

	kDataContainer		m_kDataContainer;
	kInstanceContainer	m_kInstanceContainer;
	kEraseContainer		m_kEraseContainer;
	bool		m_bAlive;
};

#define g_kBalloonEmoticonMgr SINGLETON_STATIC(PgBalloonEmoticonMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_BALLOONEMOTICON_PGBALLOONEMOTICONMGR_H