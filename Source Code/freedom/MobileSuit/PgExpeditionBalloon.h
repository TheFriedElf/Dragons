#ifndef FREEDOM_DRAGONICA_UI_PGEXPEDITIONBALLON_H
#define FREEDOM_DRAGONICA_UI_PGEXPEDITIONBALLON_H

extern unsigned int MakeTextureScreenRect(NiScreenTexturePtr& rkTexture, NiPoint2 kTargetPos, NiPoint2 kSize, NiPoint2 const kTexPos, NiPoint2 const kLeftTop, NiPoint2 const kRightBottom);

class PgExpeditionBalloon
{
public:
	PgExpeditionBalloon();
	virtual ~PgExpeditionBalloon();

	void Init();
	void Init(PgActor *pkPactor);
	void Destroy();

	bool SetNewTitle(std::wstring const &rkTitle, BM::GUID const &rkExpeditionGuid = BM::GUID::NullData(), bool const rkTitlePublic = true);

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

	BM::GUID m_kExpeditionGuid;
	//CLASS_DECLARATION_S(bool, Updated);
	RECT m_kRect;//UI Box

//	NiPoint3 m_kActorPos;

	bool m_bMaxMember;
};

//////////////////////////
class PgExpeditionBalloonMgr : public PgNodeMgr< PgExpeditionBalloon >
{
	typedef PgNodeMgr< PgExpeditionBalloon > nodemgr_type;
public:
	PgExpeditionBalloonMgr() {};

	virtual void Init();
	virtual bool ProcessInput(PgInput *pkInput);
	virtual void DrawImmediate(PgRenderer* pkRenderer, float fFrameTime);
};

#define g_kExpeditionBalloonMgr SINGLETON_STATIC(PgExpeditionBalloonMgr)
#endif // FREEDOM_DRAGONICA_UI_PGEXPEDITIONBALLON_H