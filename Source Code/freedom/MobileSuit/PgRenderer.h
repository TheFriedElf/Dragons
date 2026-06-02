#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_PGRENDERER_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_PGRENDERER_H

class PgPostProcessMan;

struct	stSameTextureCounter
{
	typedef std::map<std::string,int> SameTextureCounterCont;

	int	m_iGeomCounter;
	SameTextureCounterCont	m_kCont;

	stSameTextureCounter()
		:m_iGeomCounter(0)
	{
	};
	void	AddTexture(std::string kTextureName)
	{
		SameTextureCounterCont::iterator itor = m_kCont.find(kTextureName);
		if(itor != m_kCont.end())
		{
			itor->second = itor->second+1;
		}
		else
		{
			m_kCont.insert(std::make_pair(kTextureName,1));
		}
		m_iGeomCounter++;
	}
	void	OutputResult()
	{

		char	msg[200];
		_snprintf_s(msg,sizeof(msg),_TRUNCATE,"SameTextureCounterResult TotalTexture : %d TotalGeom : %d\n",m_kCont.size(),m_iGeomCounter);
		OutputDebugString(UNI(msg));
		int	iIndex = 0;
		for(SameTextureCounterCont::iterator itor = m_kCont.begin(); itor != m_kCont.end(); itor++)
		{
			char	msg[200];
			_snprintf_s(msg,sizeof(msg),_TRUNCATE,"[%3d] Count : %3d Name:%s\n",iIndex++,itor->second,UNI(itor->first));
			OutputDebugString(UNI(msg));
		}

	}
};

class PgRenderer : public NiMemObject
{
protected:
	//! 생성자, 외부에서 호출할 수 없다.
	PgRenderer(void);

public:
	typedef enum ePS_TYPE
	{
		E_PS_NONE = 0x00,	
		E_PS_GRAY = 0x01,
		E_PS_HIGHLIGHT = 0x02, 	
	}E_PS_TYPE;
	//! 파괴자
	~PgRenderer(void);

	//! 새 인스턴스를 반환한다.
	static PgRenderer *Create(NiRenderer *pkRenderer);
	
	//! 선택된 게임 브리오 렌더러(NiRenderer)를 반환한다.
    NiRenderer* GetRenderer() const { assert( m_spRenderer ); return m_spRenderer; };


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// Deprecated ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    private:
    NiCullingProcess m_kCuller_Deprecated;
    NiVisibleArray m_kVisibleScene_Deprecated;
    public:
    NiVisibleArray* GetVisibleArray_Deprecated();
	void PartialRenderClick_Deprecated(NiAVObject *pkObject);
    void CullingProcess_Deprecated(NiCamera *pkCamera, NiAVObject *pkObject, NiVisibleArray *pkVisibleArray,bool bReset=true);
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// Deprecated ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////


	bool BeginFrame();
	bool EndFrame();
	bool DisplayFrame();
	void SetCameraData(NiCamera *pkCamera);
	NiCamera*	GetCameraData()	{	return	m_pkCamera;	}

	NiRenderTargetGroup *GetDefaultRenderTargetGroup();
	void SetDefaultRenderTargetGroup(NiRenderTargetGroup *pkGroup);	//	pkGroup 을 0으로 설정할 경우, NiRenderer의 DefaultRenderTargetGroup 으로 설정된다. 

	bool BeginUsingDefaultRenderTargetGroup(unsigned int uiClearMode);
	bool BeginUsingRenderTargetGroup(NiRenderTargetGroup *pkTarget, unsigned int uiClearMode);
	bool EndUsingRenderTargetGroup();
	bool IsRenderTargetGroupActive();

	void GetBackgroundColor(NiColorA& kColor);
	void SetBackgroundColor(const NiColor& kColor);
	void SetBackgroundColor(const NiColorA& kColor);

    NiAccumulator* GetSorter() const { return m_spRenderer->GetSorter(); };
    void SetSorter( NiAccumulator* pkAccum ) { return m_spRenderer->SetSorter( pkAccum ); };

	void SetScreenSpaceCameraData(const NiRect<float>* pkPort = 0);

	DWORD GetPixelShaderVersion() { return m_dwPixelShaderVersion; }
	DWORD GetVertexShaderVersion() { return m_dwVertexShaderVersion; }
	void SetPixelShaderVersion(DWORD version) { m_dwPixelShaderVersion = version; }
	void SetVertexShaderVersion(DWORD version) { m_dwVertexShaderVersion = version; }	

	LPDIRECT3DPIXELSHADER9	GetGrayScalePS()	{	return	m_pGrayScalePS;	}

	PgPostProcessMan* m_pkPostProcessMan;

	NiCamera *m_pkCamera;

	static	void	EnableGlowMap(NiAVObject* pkAVObject,bool bEnable);

	static	void	SetWorldQuality(NiAVObject *pkObject, bool bFullOption);

	//static	void	SetSelectiveUpdateFlags(NiAVObject *pkAVObject);
	//static	void	SetSelectiveUpdateFlags(NiAVObject *pkAVObject,bool bSelectiveUpdate,bool bSelectiveUpdateTransforms,bool bSelectiveUpdateProperty,bool bSelectiveUpdateRigid);
	static	bool	HasTimeController(NiAVObject *pkAVObject);
	static	void	ReloadTexture(NiAVObject *pkAVObject,int iMipMapSkipLevel);
	static	int		GetBonesPerPartition()	{	return	m_iBonesPerPartition;	}
	static	void	SetBonesPerPartition(int iBonesPerPartition)	{	m_iBonesPerPartition = iBonesPerPartition; }
	static	int		CountNode(NiAVObject *pkAVObject);	///	pkAVObject 의 자식들의 갯수를 세어준다.
	static	void	ChangeShader(NiAVObject *pkAVObject,char const *strShaderName);
	static	void	EnableAlphaBlending(NiAVObject *pkAVObject,bool bEnable);
	static	void	CountSameTexture(NiAVObject *pkAVObject,stSameTextureCounter &kCounter);
	static	void	SetZBufferWrite(NiAVObject *pkAVObject,bool bZBufferWrite);
	static	void	SetAlphaBlendMode(NiAVObject *pkAVObject,NiAlphaProperty::AlphaFunction kSrcBlend,NiAlphaProperty::AlphaFunction kDestBlend);
	static	void	RestartAnimation(NiAVObject *pkAVObject,float fCurrentAccumTime);
	static	void	PrecacheGeometry(NiAVObject *pkAVObject);
	static	int		CountLight(NiAVObject *pkAffectedObject);	//	pkAffectedObject 에 적용되고 있는 포인트 라이트 갯수를 리턴한다.
	static	void	EnableMipMap(NiAVObject *pkAVObject,bool bEnable);
	static	void	AddToVisibleArray(NiVisibleArray &rkVisibleArray,NiAVObject *pkAVObject);
	static	void	DetachAllAffectedNode(NiAVObject *pkLightRoot);
	static	void	VerifyNodes(NiAVObject *pkAVObject);
	static	bool	HasGeomMorphController(NiGeometry *pkGeom);
	static	bool	HasTextureTransformController(NiGeometry *pkGeom);
    static	void	RunUpParticleSystem(NiAVObject *pkAVObject);

    static NiPropertyState* CopyPropertyState(NiPropertyState *pkSource);

    static PgRenderer* GetPgRenderer() { return ms_pkPgRenderer; };

	NiFrustumPlanes	const	&GetFrustumPlanes()	{	return	m_kFrustumPlanes;	}


protected:
	static	PgRenderer*	ms_pkPgRenderer;

	NiRendererPtr m_spRenderer;

	NiRenderTargetGroupPtr	m_spDefaultRenderTargetGroup;
	DWORD m_dwVertexShaderVersion;
	DWORD m_dwPixelShaderVersion;

	LPDIRECT3DPIXELSHADER9	m_pGrayScalePS;

	NiFrustumPlanes	m_kFrustumPlanes;

	static	int	m_iBonesPerPartition;
};

#endif // FREEDOM_DRAGONICA_RENDER_RENDERER_PGRENDERER_H