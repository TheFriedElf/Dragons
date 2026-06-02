#ifndef FREEDOM_DRAGONICA_RENDER_EFFECT_PGWATER_H
#define FREEDOM_DRAGONICA_RENDER_EFFECT_PGWATER_H

#include "NiMain.h"
#include "PgRenderer.h"
class	PgWorld;

class	PgWater	:	public	NiMemObject
{

	int	m_iTexWidth,m_iTexHeight;

	struct	stWaterNode
	{
		NiRenderedTexturePtr m_spRT_UponWater;	//	물위 장면
	    NiRenderTargetGroupPtr m_spRTG_UponWater;
	
		D3DXMATRIX	m_matReflectMirrorView;
		NiGeometryPtr	m_spWaterNode;	

		//	물이 흐르는 속도 비율( 0 : 물이 흐르지 않음   0~:흐르는 속도 증가)
		float	m_fWaterSpeed;
		//	물결의 크기 비율
		float	m_fWaterWaveSize;
		//	반사비율(1.0 : 완전반사, 0.0 : 반사 없음)
		float	m_fWaterReflect;
		//	물결 높이 비율(0: 물결 높이 없음 0~ : 물결 높이 증가)
		float	m_fWaterWaveHeight;
		//	해조 농도(0: 해조 없음 1:해조만 보임)
		float	m_fWaterDetailDensity;
		stWaterNode()
		{
			m_spRT_UponWater=0;
			m_spRTG_UponWater = 0;
			m_fWaterSpeed = 0.7f;
			m_fWaterWaveSize = 0.05f;
			m_fWaterReflect = 0.3f;
			m_fWaterWaveHeight = 0.35f;
			m_fWaterDetailDensity = 0.2f;
		}
		~stWaterNode()
		{
			m_spRT_UponWater = 0;
			m_spRTG_UponWater = 0;
			m_spWaterNode = 0;
		}
	};

	typedef std::list<stWaterNode*> WaterNodeList;

	NiRenderedTexturePtr m_spRT_UnderWater;	//	물밑 장면
    NiRenderTargetGroupPtr m_spRTG_UnderWater;

	NiSourceTexturePtr	m_spWaterBump[2];
	NiSourceTexturePtr	m_spWaterDetail;

	WaterNodeList	m_WaterNodeList;

	float	m_fTime;

	bool	m_bNowUpdating;

public:

	PgWater()	{	Init();	}
	~PgWater()	{	Destroy();	}

	void	InitFromNode(NiNodePtr spWaterRootNode);
	void	UpdateScene(PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime,PgWorld *pWorld);
	void	RenderImmediate(PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime);

	void	SetWaterInfoFromXML(const TiXmlNode *pkNode);

private:

	stWaterNode*	CreateNewWaterNode(NiAVObjectPtr	spWaterNode,NiRenderer *pkRenderer);
	void	InitFromNodeRecursive(NiAVObjectPtr spWaterNode);
	void	UpdateWaterNode(stWaterNode *pWaterNode,PgRenderer *pkRenderer, NiCameraPtr spCamera,PgWorld *pWorld,float fFrameTime);

	void	Init();
	void	Destroy();

	stWaterNode*	GetWaterNode(char const *strNodeName);

};

#endif // FREEDOM_DRAGONICA_RENDER_EFFECT_PGWATER_H