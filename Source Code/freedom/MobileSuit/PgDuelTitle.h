#ifndef FREEDOM_DRAGONICA_UI_PGDUELTITLE_H
#define FREEDOM_DRAGONICA_UI_PGDUELTITLE_H

class PgDuelTitle : public NiGeometry
{
	NiDeclareRTTI;
public:
	PgDuelTitle();
	virtual ~PgDuelTitle();

	void Init(void);
	void Set(WORD wWin);
	void Cleanup(void);
	static void DestoryStatic();

	virtual void UpdateWorldBound();
	virtual void RenderImmediate(NiRenderer* pkRenderer);
	void Draw3DPos(PgRenderer* pkRenderer, NiCamera* pkCamera, NiPoint3 const &rkPos);

protected:
	WORD m_wWins;
	NiBillboardNodePtr CreateWinnerMark_Quad( NiPoint3 vQuadPos, float fWidth, float fHeight, NiTexture* pkTexture );
	NiBillboardNodePtr CreateWinCount_Quad( NiPoint3 vQuadPos, NiTexture* pkTexture );
	NiTriShape*	CreateShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap);
	NiTriShape*	CreateNumShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap);
	void ApplyDefaultMaterial(NiTriShape *pkShape, NiTexture *pkBaseMap);

private:
	static	NiMaterialPropertyPtr	m_spMaterialProperty;
	static	NiAlphaPropertyPtr	m_spAlphaProperty;
	static	NiZBufferPropertyPtr	m_spZBufferProperty;
	static	NiVertexColorPropertyPtr	m_spVertexColorProperty;

	NiBillboardNodePtr m_spWinnerMarkQuad;
	NiBillboardNodePtr m_spWinCountQuad;
	NiBillboardNodePtr m_spChampionMarkQuad;
	NiScreenTexturePtr m_spWinnerMarkTexture;
	NiScreenTexturePtr m_spWinCountTexture;
	NiScreenTexturePtr m_spChampionMarkTexture;

};

NiSmartPointer(PgDuelTitle);
#endif
