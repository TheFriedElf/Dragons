#ifndef FREEDOM_DRAGONICA_RENDER_SHADOW_SHADOWVOLUME_PGSHADOWVOLUME_H
#define FREEDOM_DRAGONICA_RENDER_SHADOW_SHADOWVOLUME_PGSHADOWVOLUME_H
#include "NiMain.h"
#include "PgRenderer.H"

class	PgShadowVolume	:	public	NiNode
{
	struct	stTri
	{
		NiPoint3	m_kPoint[3];
		NiPoint3	m_kNormal;
	};

	struct	stEdge
	{
		NiPoint3	m_kPoint[2];
		stTri	m_kTriangle[2];
	};

	NiTriShapePtr	m_spShadowGeom;

	typedef	std::vector<stTri> VTri;
	typedef	std::vector<stEdge> VEdge;

public:

	VTri	m_vTriangles;
	VEdge	m_vEdges;

	int	m_iActiveTriCount;
	int	m_iActiveEdgeCount;

	NiScreenElementsPtr	m_spQuad;


	PgShadowVolume();
	virtual	~PgShadowVolume();

public:

	void	UpdateShadowVolume(NiVisibleArray	*pkVisibleArray,const	NiPoint3	&kLightDir);

public:


	void	SetupStencilBuffer(PgRenderer *pkRenderer);
	void	RenderImmediate(PgRenderer *pkRenderer,NiCamera*	pkCamera,bool bFront);
	void	RenderQuad(PgRenderer *pkRenderer,NiCamera*	pkCamera);

private:

	//	라이트 방향에서 보이는 삼각형만 추려낸다.
	void	CullTriangleFromLightDir(NiVisibleArray	*pkVisibleArray,const	NiPoint3	&kLightDir,VTri &kCulledTri,VEdge &kEdge);

	//	추려낸 삼각형의	엣지들중에 중복되는 것들을 제외하고, 엣지 리스트를 만든다.
	void	CreateEdgeList(VTri &kCulledTri,VEdge &kEdge);

	//	각 엣지 리스트로 사각폴리곤을 생성한다.
	void	CreateRectPoly( VEdge &kEdge,const	NiPoint3	&kLightDir);

	//	처음 추려낸 삼각형 리스트와 합친다.
	void	CreateFinalGeometry(VTri &kCulledTri,VEdge &kEdge);

private:

	void	AddEdgeToEdgeList(NiPoint3 const &kP1,NiPoint3 const &kP2,VEdge &kEdgeList,const	NiPoint3	&kLightDir);
};

#endif // FREEDOM_DRAGONICA_RENDER_SHADOW_SHADOWVOLUME_PGSHADOWVOLUME_H