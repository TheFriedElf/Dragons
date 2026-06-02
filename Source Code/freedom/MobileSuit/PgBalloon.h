#ifndef FREEDOM_DRAGONICA_UI_PGBALLOON_H
#define FREEDOM_DRAGONICA_UI_PGBALLOON_H

typedef enum
{
	BALLOON_TEXT_ACTOR,
	BALLOON_TOOLTIP,
	BALLOON_BOARD,
}E_BALLOON_TYPE;

class PgBalloon
{// 순수 가상 class
public:
	typedef struct tagBalloonTextInfo
	{
		tagBalloonTextInfo()
			:	pk2DString(NULL)
			,	kTextAdjustPos(0.0f, 0.0f, 0.0f)
			,	bOutLine(false)
			,	fScale(1.0f)
		{}

		~tagBalloonTextInfo()
		{
		}

		void Release()
		{
			SAFE_DELETE(pk2DString);
		}

		NiColorA	kTextColor;
		NiColorA	kOutLineColor;
		NiPoint3	kTextAdjustPos;
		bool		bOutLine;
		float		fScale;
		Pg2DString	*pk2DString;
	}SBalloonTextInfo;
	typedef std::map< int, SBalloonTextInfo >		CONT_TEXT;

public:
	PgBalloon(void);
	virtual ~PgBalloon(void);

	virtual E_BALLOON_TYPE GetType(void)const = 0;
	virtual void DrawImmediate( PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime ) = 0;
	virtual bool Is3D(void)const = 0;

	virtual void SetExistTime( int const iExistTime ){SetShow(false);}
	
	void SetEnable(bool bEnable){m_bEnable = bEnable;}
	bool IsEnable(void)const{return	m_bEnable;}

	void SetShow(bool bShow){m_bShow = bShow;}
	bool IsShow(void)const{return m_bShow;}

	BM::GUID const &GetID(void)const{return m_kID;}

	HRESULT AddText( int const iTextID, Pg2DString *pk2DString, NiPoint3 const &kAdjustPos, NiColorA const &kTextColor, NiColorA const &kOutLineColor, bool const bOutLine, float const fScale );
	HRESULT RemoveText( int const iTextID );
	Pg2DString * GetText( int const iTextID )const;

protected:
	bool IsDrawImmediate(void)const;
	void DrawImmediateText( PgRenderer *pkRender, NiCamera* pCamera, NiPoint3 const &kBasePos );
	
private:
	BM::GUID		m_kID;
	bool			m_bEnable;
	bool			m_bShow;

	CONT_TEXT		m_kContText;

protected:
	NiScreenTexturePtr	m_spScreenTexture;
};

class PgBoardBalloon
	:	public PgBalloon
{
public:
	PgBoardBalloon(void);
	virtual ~PgBoardBalloon(void);

	virtual E_BALLOON_TYPE GetType(void)const{return BALLOON_BOARD;}
	virtual void DrawImmediate( PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime );
	virtual bool Is3D(void)const{return true;}

	bool Init( std::string const &strTexutre, NiPoint3 const &pt3AdjustPos, float const fScale );

	void SetPos( NiPoint3 const &ptPos ){m_ptPos = m_ptAdjustPos + ptPos;}

	static void DestoryStatic();

protected:
	NiBillboardNodePtr CreateQuad();
	NiTriShape*	CreateShape( unsigned short usVertices, NiPoint3 *pkVertexs, NiColorA *pkColors, NiPoint2 *pkTextureUV, unsigned short usTriangles, unsigned short *pusTriangleList, NiTexture *pkTexture );

protected:
	static	NiMaterialPropertyPtr		m_spMaterialProperty;
	static	NiAlphaPropertyPtr			m_spAlphaProperty;
	static	NiZBufferPropertyPtr		m_spZBufferProperty;
	static	NiVertexColorPropertyPtr	m_spVertexColorProperty;

	NiPoint3			m_ptPos;
	NiPoint3			m_ptAdjustPos;
	NiBillboardNodePtr	m_spQuad;
	float				m_fScale;
};
#endif // FREEDOM_DRAGONICA_UI_PGBALLOON_H