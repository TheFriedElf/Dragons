#ifndef FREEDOM_DRAGONICA_UI_PGMARKBALLOON_H
#define FREEDOM_DRAGONICA_UI_PGMARKBALLOON_H

#include "PgNodeMng.h"

class PgActor;
class Pg2DString;

enum eMarkType
{
	T_MARK_NONE				= 0x00,
	T_MARK_BATTLE_ENEMY		= 0x01,
	T_MARK_BATTLE_MVP		= 0x02,
};
typedef WORD MARK_TYPE;

class PgMarkBalloon
{
	static std::string const ms_strMARK_Enemy;
	static std::string const ms_strMARK_MVP;

public:
	
public:
	PgMarkBalloon();
	virtual ~PgMarkBalloon();

	void	Init();
	void	Init(PgActor *pActor);
	void	Destroy();

	void	SetNewMarkBalloon( MARK_TYPE const eMarkType );
	void	Release();

	void	DrawImmediate( PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime = 0.f);

	void	SetShow( bool bShow ){m_bShow=bShow;}
	MARK_TYPE GetType()const{return m_eMarkType;}
	BM::GUID const &GetGuid()const;

private:
	void	LoadMarkResource( MARK_TYPE const eMarkType );

private:
	PgActor*			m_pkActor;
	NiScreenTexturePtr	m_spScreenTexture;
	MARK_TYPE			m_eMarkType;
	Pg2DString*			m_pText;
	bool				m_bShow;
	int					m_iTextHeight;
};

typedef struct tagMarkPoint
{
	tagMarkPoint()
		:	kType(T_MARK_NONE)
		,	iPoint(0)
	{}

	MARK_TYPE	kType;
	size_t		iPoint;

	bool IsDrawMark()const
	{
		switch ( kType )
		{
		case T_MARK_BATTLE_ENEMY:
			{
				return iPoint > 0;
			}break;
		case T_MARK_BATTLE_MVP:
			{
				return iPoint > 0;
			}break;
		}
		return false;
	}
}SMarkPoint;

class	PgMarkBalloonMan : public PgNodeMgr< PgMarkBalloon >
{
	friend	class	PgMobileSuit;
private:
	typedef std::map<BM::GUID, SMarkPoint> CONT_MARK_POINT;

public:
	PgMarkBalloonMan();
	virtual ~PgMarkBalloonMan();

	void Init();
	void Clear();

	void SetType( MARK_TYPE const kType );
	void AddType( MARK_TYPE const kType );
	void RemoveType( MARK_TYPE const kType );

	void AddActor( PgActor *pkActor );
	void AddMarkPoint( BM::GUID const &kGuid, MARK_TYPE const kMarkType, size_t iPoint=1 );
	void RemoveMarkPoint( BM::GUID const &kGuid, MARK_TYPE const kMarkType );

protected:
	
private:
	MARK_TYPE		m_kMarkType;
	CONT_MARK_POINT	m_kContMarkPoint;

};

#define g_kMarkBalloonMan SINGLETON_STATIC(PgMarkBalloonMan)
#endif // FREEDOM_DRAGONICA_UI_PGMARKBALLOON_H