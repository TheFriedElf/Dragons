#ifndef FREEDOM_DRAGONICA_CONTENTS_PGELEMENTMARK_H
#define FREEDOM_DRAGONICA_CONTENTS_PGELEMENTMARK_H

std::string const STR_DEFAULT_ELEMENT_MARK_IMG = "../Data/6_UI/icon/iconAttribute.tga";

class PgElementMark : public NiGeometry
{
	NiDeclareRTTI;

public:

	PgElementMark();
	virtual ~PgElementMark();
	void Destroy();
	static void DestoryStatic();

	virtual	void UpdateWorldBound();

	void Init(std::string const& rkPath, NiPoint2 const& rkIconSize);
	void Set( BYTE const kMarkNo, float const fNameWidth );
	virtual void RenderImmediate(NiRenderer* pkRenderer);

	BYTE GetMarkNo() const;

private:
	void Draw3DPos(PgRenderer* pkRenderer, NiCamera* pkCamera, NiPoint3 const &rkPos);
	NiBillboardNodePtr CreateQuad( NiTexture* pkTexture, float const fRowIcon, float const fCollIcon, bool const bLeft );
	NiTriShape*	CreateShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap);

	NiScreenTexturePtr	m_spScreenTexture;

	NiBillboardNodePtr					m_spQuad;
	static	NiMaterialPropertyPtr		m_spMaterialProperty;
	static	NiAlphaPropertyPtr			m_spAlphaProperty;
	static	NiZBufferPropertyPtr		m_spZBufferProperty;
	static	NiVertexColorPropertyPtr	m_spVertexColorProperty;
	static	float	m_fIconWidth;
	static	BYTE m_byIconNumPerRow;

	BYTE m_kMarkNo;
	float m_kNameWidth;
	float m_fQuadScale;

	NiPoint2 m_kIconSize;
};
 
NiSmartPointer(PgElementMark);
#endif //FREEDOM_DRAGONICA_CONTENTS_PGELEMENTMARK_H