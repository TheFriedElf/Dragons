#ifndef FREEDOM_DRAGONICA_CONTENTS_BALLOONEMOTICON_PGBALLOONEMOTICONINSTANCE_H
#define FREEDOM_DRAGONICA_CONTENTS_BALLOONEMOTICON_PGBALLOONEMOTICONINSTANCE_H

class PgBalloonEmoticonFrameInstance
{
public:
	bool Update(float fAccumTime,float fFrameTime, bool const bPause);
	void DrawImmediate(PgRenderer *pkRenderer, POINT2 const& PtTargetPos, POINT2 const& FrameSize, int const iU);

	explicit PgBalloonEmoticonFrameInstance(PgBalloonEmoticonFrameData const* pFrameInfo);
	~PgBalloonEmoticonFrameInstance(void);

protected:
	CLASS_DECLARATION_S(float, StartTime);
	float m_fPrevTime;
	PgBalloonEmoticonFrameData const* m_pFrameInfo;
};

class PgBalloonEmoticonInstance
{
public:
	bool Initialize(PgBalloonEmoticonData const* pEmoticonInfo);
	bool Update(float fAccumTime,float fFrameTime, bool const bPause);
	void DrawImmediate(PgRenderer *pkRenderer, PgActor* pActor);

	PgBalloonEmoticonInstance(void);
	~PgBalloonEmoticonInstance(void);

private:
	typedef std::list< PgBalloonEmoticonFrameInstance >	kFrameInstanceContainer;

	int m_iChangedFrameCount;
	PgBalloonEmoticonData const* m_pEmoticonInfo;
	kFrameInstanceContainer	m_kFrameInstance;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_BALLOONEMOTICON_PGBALLOONEMOTICONINSTANCE_H