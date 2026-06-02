#ifndef FREEDOM_DRAGONICA_UI_PGPARTYBALLON_H
#define FREEDOM_DRAGONICA_UI_PGPARTYBALLON_H

class PgPartyBalloon
{
public:
	PgPartyBalloon();
	virtual ~PgPartyBalloon();

	void Init();
	void Init(PgActor *pkPactor);
	void Destroy();

	bool SetNewTitle(std::wstring const &rkTitle, BM::GUID const &rkPartyGuid = BM::GUID::NullData(), bool const rkTitlePublic = true);

//	void Update(NiPoint3 const &rkWorldPos) { m_kActorPos = rkWorldPos; }
	void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	bool ProcessInput(PgInput *pkInput);

	void SetMaxMember(bool bMember);
	bool GetMaxMember();

	bool GetTitlePublic();

protected:
	void LoadResource();

protected:
	PgActor* m_pkActor;
	NiScreenTexturePtr	m_spScreenTexture;
	NiScreenTexturePtr	m_spIconTexture;

	std::wstring m_kOrgText;
	Pg2DString* m_pkTitle;
	bool m_bTitlePublic;

	BM::GUID m_kPartyGuid;
	//CLASS_DECLARATION_S(bool, Updated);
	RECT m_kRect;//UI Box

//	NiPoint3 m_kActorPos;

	bool m_bMaxMember;
};

//////////////////////////
class PgPartyBalloonMgr : public PgNodeMgr< PgPartyBalloon >
{
	typedef PgNodeMgr< PgPartyBalloon > nodemgr_type;
public:
	PgPartyBalloonMgr() {};

	virtual void Init();
	virtual bool ProcessInput(PgInput *pkInput);
	virtual void DrawImmediate(PgRenderer* pkRenderer, float fFrameTime);
};

#define g_kPartyBalloonMgr SINGLETON_STATIC(PgPartyBalloonMgr)
#endif // FREEDOM_DRAGONICA_UI_PGPARTYBALLON_H