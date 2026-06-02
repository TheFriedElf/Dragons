#ifndef FREEDOM_DRAGONICA_CONTENTS_MARKET_PGVENDERBALLOON_H
#define FREEDOM_DRAGONICA_CONTENTS_MARKET_PGVENDERBALLOON_H

class PgVendorBalloon
{
public:
	PgVendorBalloon();
	virtual ~PgVendorBalloon();

	void Init();
	void Init(PgActor *pkPactor);
	void Destroy();

	bool SetNewTitle(std::wstring const &rkTitle, BM::GUID const &rkOwnerGuid = BM::GUID::NullData());

	void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	bool ProcessInput(PgInput *pkInput);

protected:
	void LoadResource();

protected:
	PgActor* m_pkActor;
	NiScreenTexturePtr	m_spScreenTexture;
	NiScreenTexturePtr	m_spIconTexture;

	std::wstring m_kOrgText;
	Pg2DString* m_pkTitle;

	BM::GUID m_kOwnerGuid;
	//CLASS_DECLARATION_S(bool, Updated);
	RECT m_kRect;//UI Box
};

//////////////////////////
class PgVendorBalloonMgr : public PgNodeMgr< PgVendorBalloon >
{
	typedef PgNodeMgr< PgVendorBalloon > nodemgr_type;
public:
	PgVendorBalloonMgr() {};

	virtual void Init();
	virtual bool ProcessInput(PgInput *pkInput);
	virtual void DrawImmediate(PgRenderer* pkRenderer, float fFrameTime);
};

#define g_kVendorBalloonMgr SINGLETON_STATIC(PgVendorBalloonMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_MARKET_PGVENDERBALLOON_H