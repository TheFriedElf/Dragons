#ifndef FREEDOM_DRAGONICA_UI_PGEFFECTCOUNTDOWN_H
#define FREEDOM_DRAGONICA_UI_PGEFFECTCOUNTDOWN_H

class PgEffectCountDown : public NiGeometry
{
	NiDeclareRTTI;
public:
	PgEffectCountDown();
	virtual ~PgEffectCountDown();

	void Init(void);
	void Set(WORD wWin);
	void Cleanup(void);
	static void DestoryStatic();

	virtual void UpdateWorldBound();
	virtual void RenderImmediate(NiRenderer* pkRenderer);
	void Draw3DPos(PgRenderer* pkRenderer, NiCamera* pkCamera, NiPoint3 const &rkPos);

	void	SetAlpha( float const fAlpha );

protected:
	WORD m_wCountDown;
	NiBillboardNodePtr CreateCountDown_Quad( NiPoint3 vQuadPos, NiTexture* pkTexture );
	NiTriShape*	CreateNumShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap);
	void ApplyDefaultMaterial(NiTriShape *pkShape, NiTexture *pkBaseMap);

private:
	static	NiMaterialPropertyPtr	m_spMaterialProperty;
	static	NiAlphaPropertyPtr	m_spAlphaProperty;
	static	NiZBufferPropertyPtr	m_spZBufferProperty;
	static	NiVertexColorPropertyPtr	m_spVertexColorProperty;

	NiBillboardNodePtr m_spCountDownQuad;
	NiScreenTexturePtr m_spCountDownTexture;
};

NiSmartPointer(PgEffectCountDown);
#endif
