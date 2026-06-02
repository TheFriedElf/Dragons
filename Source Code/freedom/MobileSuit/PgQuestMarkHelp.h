#ifndef FREEDOM_DRAGONICA_CONTENTS_QUEST_HELPER_PGQUESTMARKHELP_H
#define FREEDOM_DRAGONICA_CONTENTS_QUEST_HELPER_PGQUESTMARKHELP_H
//
class PgQuestMarkHelp
{
public:

	PgQuestMarkHelp();
	~PgQuestMarkHelp();

	void Clear();

	void Update(NiPoint3 const &rkWorldPos, bool const bHide) { m_pkActorPos = rkWorldPos, m_bHide = bHide; }

	void SetHelpText(std::wstring const &rkHelpText, std::string const& rkBGTexture, float const fTextAddX = 20.f);
	//void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

private:
	NiScreenTexturePtr m_pkHelpBGTexture;
	Pg2DString *m_pkHelpText;

	NiPoint3 m_pkActorPos;
	bool m_bHide;
	float m_fTextAddX;
};

class PgQuestMarkMng : public PgNodeMgr< PgQuestMarkHelp >
{
public:
	PgQuestMarkMng();
	virtual ~PgQuestMarkMng();

	void DrawImmediate(PgRenderer* pkRenderer, float fFrameTime);
protected:
	float m_fAccumTime;
	bool m_bVisible;
};

#define g_kQuestMarkHelpMng SINGLETON_STATIC(PgQuestMarkMng)

#endif // FREEDOM_DRAGONICA_CONTENTS_QUEST_HELPER_PGQUESTMARKHELP_H