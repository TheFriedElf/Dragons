#ifndef FREEDOM_DRAGONICA_CONTENTS_MYHOME_PGMYHOMEMARK_H
#define FREEDOM_DRAGONICA_CONTENTS_MYHOME_PGMYHOMEMARK_H

class PgMyhomeMark : public NiGeometry
{
	NiDeclareRTTI;

public:
	PgMyhomeMark();
	virtual ~PgMyhomeMark();

	virtual	void UpdateWorldBound();

	void Init();
	void Destroy();
	void SetNameLength(float fLen);
	void ReCreateQuad();
	static void DestoryStatic();

	virtual void RenderImmediate(NiRenderer* pkRenderer);

protected:
	void Draw3DPos(PgRenderer* pkRenderer, NiCamera* pkCamera, NiPoint3 const &rkPos);
	NiBillboardNodePtr CreateQuad( NiTexture* pkTexture, float const fRowIcon, float const fCollIcon, bool const bLeft );
	NiTriShape*	CreateShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap);

	CLASS_DECLARATION_S(bool, IsDraw);
	CLASS_DECLARATION_NO_SET(float, m_fQuadScale, QuadScale);
	CLASS_DECLARATION_S_NO_SET(float, NameWidth);
private:
	NiBillboardNodePtr m_spQuad;
	NiBillboardNodePtr m_spQuad_EMMark;
	static	NiMaterialPropertyPtr	m_spMaterialProperty;
	static	NiAlphaPropertyPtr	m_spAlphaProperty;
	static	NiZBufferPropertyPtr	m_spZBufferProperty;
	static	NiVertexColorPropertyPtr	m_spVertexColorProperty;

	PgActor* m_pkActor;
	NiScreenTexturePtr	m_spScreenTexture;
	NiScreenTexturePtr	m_spScreenTexture_EMMark;
};

NiSmartPointer(PgMyhomeMark);

#endif // FREEDOM_DRAGONICA_CONTENTS_MYHOME_PGMYHOMEMARK_H