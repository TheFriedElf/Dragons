#ifndef FREEDOM_DRAGONICA_UI_PGGUILDMARK_H
#define FREEDOM_DRAGONICA_UI_PGGUILDMARK_H

class PgGuildMark : public NiGeometry
{
	NiDeclareRTTI;

public:
	PgGuildMark();
	virtual ~PgGuildMark();

	virtual	void UpdateWorldBound();

	void Init();
	void Destroy();
	static void DestoryStatic();

	void Set( BYTE const cGuildMarkNo, BYTE const byEmporiaMark, float const fNameWidth);
	//void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	virtual void RenderImmediate(NiRenderer* pkRenderer);

	static float GetGuildIconWidth(); //아이콘 하나당 넓이 : 0.0~1.0 사이의 UV좌표 기준
	static BYTE GetGuildIconNumPerRow(); //아이콘 이미지에서 한 행당 아이콘 갯수

	//void NameWidth(float const fNewNameWidth);
protected:
	void Draw3DPos(PgRenderer* pkRenderer, NiCamera* pkCamera, NiPoint3 const &rkPos);
	NiBillboardNodePtr CreateQuad( NiTexture* pkTexture, float const fRowIcon, float const fCollIcon, bool const bLeft );
	NiTriShape*	CreateShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap);

	CLASS_DECLARATION_S(BYTE, GuildMarkNo);
	CLASS_DECLARATION_S(BYTE, EmporiaMark);
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
	static	float	m_fGuildIconWidth;
	static	BYTE m_byGuildIconNumPerRow;

	PgActor* m_pkActor;
	NiScreenTexturePtr	m_spScreenTexture;
	NiScreenTexturePtr	m_spScreenTexture_EMMark;
};

NiSmartPointer(PgGuildMark);
#endif // FREEDOM_DRAGONICA_UI_PGGUILDMARK_H