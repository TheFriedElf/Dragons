#ifndef FREEDOM_DRAGONICA_SCENE_WORLD_PGBUILDING_H
#define FREEDOM_DRAGONICA_SCENE_WORLD_PGBUILDING_H


class PgTrigger;
class PgBoardBalloon;

class PgBuilding
{
public:
	typedef std::pair<NiAVObject*,bool>		CONT_OBJECT_INFO;	//second: ToggleOnShow
	typedef std::vector<CONT_OBJECT_INFO>	CONT_OBJECT_LIST;

public:
	PgBuilding(void);
	~PgBuilding(void);

	static int GetTTWName( short nFuncNo );

	void Init( NiAVObject *pkObject, float const fDisableAlpha, PgBoardBalloon * pkBoardBallon );
	void AddToggleObject( NiAVObject *pkObject, bool const bToggleOnShow );
	void SetTrigger( PgTrigger *pkTrigger );
	
	bool Construct( SEmporiaFunction const &kFunc );
	bool Destroy();

	void SetDisableAlpha( float const fAlpha ){m_fDisableAlpha;}
	void SetChangeAlpha( NiAVObject* pkAVObject, float const fAlpha );

	CLASS_DECLARATION_S( BM::GUID, NpcID );

private:
	bool				m_bConstructed;
	CONT_OBJECT_LIST	m_kContObjectList;
	PgTrigger			*m_pkTrigger;

	float				m_fDisableAlpha;
	NiAVObject			*m_pkObject;

	BM::GUID			m_kBoardBalloonID;

private:
};

#endif // FREEDOM_DRAGONICA_SCENE_WORLD_PGBUILDING_H