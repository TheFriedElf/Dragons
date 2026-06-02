#ifndef FREEDOM_DRAGONICA_UI_PGACHIEVEMENTTITLE_H
#define FREEDOM_DRAGONICA_UI_PGACHIEVEMENTTITLE_H

class PgAchievementTitle
	: public NiGeometry
{
	NiDeclareRTTI;
public:
	PgAchievementTitle(void);
	virtual ~PgAchievementTitle(void);

	void Init(void);
	void Set(XUI::CXUI_Font* pFont, std::wstring const& kAchievementText, DWORD const BgColor = 0xFFFFFFFF, DWORD const FontColor = 0xFFFFFFFF);
	void Destroy(void);
	static void DestroyStatic(void);
	virtual void UpdateWorldBound();
	virtual void RenderImmediate(NiRenderer* pkRenderer);

	void	SetAlpha( float const fAlpha ){m_kTextColor.a = fAlpha;}
	POINT Get2DStringSize();

protected:
	void Draw3DPos(PgRenderer* pkRenderer, NiCamera* pkCamera, NiPoint3 const &rkPos);
	NiBillboardNodePtr CreateQuad( NiTexture* pkTexture, int const TxtWidth, int const TxtHeight);
	NiTriShape*	CreateShape(NiPoint3 *pkVerts, NiColorA *pkColors, NiPoint2 *pkUVs, NiTexture *pkBaseMap);

private:
	NiScreenTexturePtr m_spBgTexture;
	NiBillboardNodePtr m_spBgTextureBill;

	static	NiMaterialPropertyPtr	m_spMaterialProperty;
	static	NiAlphaPropertyPtr	m_spAlphaProperty;
	static	NiZBufferPropertyPtr	m_spZBufferProperty;
	static	NiVertexColorPropertyPtr	m_spVertexColorProperty;

	bool m_bAlive;
	NiColorA m_kBgColor;
	std::wstring m_wstrAchievementText;
	NiColorA m_kTextColor;
	Pg2DString* m_pk2DString;

	CLASS_DECLARATION(bool, m_bDrawShadow, DrawShadow);
	CLASS_DECLARATION(bool, m_bUseBgColor, UseBgColor);
	CLASS_DECLARATION_NO_SET(float, m_fQuadScale, QuadScale);
};

NiSmartPointer(PgAchievementTitle);
#endif // FREEDOM_DRAGONICA_UI_PGACHIEVEMENTTITLE_H