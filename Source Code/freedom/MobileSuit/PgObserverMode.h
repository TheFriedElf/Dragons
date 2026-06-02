#ifndef FREEDOM_DRAGONICA_CONTENTS_PVP_PGOBSERVERMODE_H
#define FREEDOM_DRAGONICA_CONTENTS_PVP_PGOBSERVERMODE_H

typedef enum : BYTE
{
	OBMODE_NONE,
	OBMODE_SEARCHING,
	OBMODE_FOLLOWING,	// 따라가는중
	OBMODE_ENDING,
}EOBMode;

class PgObserverMode
{
public:
	PgObserverMode(void);
	virtual ~PgObserverMode();

	void Start( BM::GUID const &kTargetID );
	void End(void);
	void Failed( BM::GUID const &kTargetID );

	void Update( float const fAccumTime, float const fFrameTime );

	void SetTarget( BM::GUID const &kCharGuid );
	void AddTarget( BM::GUID const &kCharGuid );
	void RemoveTarget( BM::GUID const &kCharGuid );
	bool ToggleTarget();

protected:
	SET_GUID	m_kContTargetChar;
	BM::GUID	m_kNowTargetChar;
	EOBMode		m_kMode;
	DWORD		m_dwDelay;// 너무 자주 요청하지 않게.
};

#define g_kOBMode SINGLETON_STATIC( PgObserverMode )

#endif // FREEDOM_DRAGONICA_CONTENTS_PVP_PGOBSERVERMODE_H