#ifndef FREEDOM_DRAGONICA_RENDER_EFFECT_PGLINKEDPLANE_H
#define FREEDOM_DRAGONICA_RENDER_EFFECT_PGLINKEDPLANE_H

#include "NiNode.H"
#include "PgRenderer.H"

class PgLinkedPlaneGroup;
class PgLinkedPlane	
{
public:
	PgLinkedPlane(int const iInstanceID,
		PgLinkedPlaneGroup* pkParentGroup,
		NiTexturingPropertyPtr spTexturingProp,
		NiAlphaPropertyPtr spAlphaProp,
		NiVertexColorPropertyPtr spVertexColorProp,
		NiZBufferPropertyPtr spZBufferProp,
		NiStencilPropertyPtr spStencilProp,
		NiMaterialPropertyPtr spMaterialProp)
	{
		Create(iInstanceID,pkParentGroup,spTexturingProp,spAlphaProp,spVertexColorProp,
			spZBufferProp,spStencilProp,spMaterialProp);
	}

	~PgLinkedPlane() { Destroy(); }

	void	Update(float fAccumTime, float fFrameTime);
	void	DrawImmediate(PgRenderer* pkRenderer);

	int	const GetInstanceID() { return m_iInstanceID;	}

	void	SetStartPos(NiPoint3 const& kPos);
	void	SetEndPos(NiPoint3 const& kPos);

	void	UpdateProperties();

	void	SetWidth(float const fWidth);
	float	GetWidth();

	void	SetEnable(bool const bEnable);
	bool	GetEnable();

private:

	void	Create(int const iInstanceID,
		PgLinkedPlaneGroup *pkParentGroup,
		NiTexturingPropertyPtr spTexturingProp,
		NiAlphaPropertyPtr spAlphaProp,
		NiVertexColorPropertyPtr spVertexColorProp,
		NiZBufferPropertyPtr spZBufferProp,
		NiStencilPropertyPtr spStencilProp,
		NiMaterialPropertyPtr spMaterialProp
		);

	void	Destroy();

	void	UpdateTextureCoordinates();
	void	UpdateWorldBound();

private:
	int	m_iInstanceID;

	float m_fLastTextureCoordinateUpdateTime;
	float m_fWidth;
	float m_fTileSegmentLength;

	NiPoint3 m_vStart,m_vEnd;
		
	NiTriStripsPtr	m_spGeom;

	PgLinkedPlaneGroup* m_pkParentGroup;

	bool m_bEnable;


};

class	PgLinkedPlaneGroup
{
public:

	PgLinkedPlaneGroup(int const iInstanceID)
	{
		Create(iInstanceID);
	}
	~PgLinkedPlaneGroup()
	{
		Destroy();
	}

	void	Update(float fAccumTime, float fFrameTime);
	void	DrawImmediate(PgRenderer* pkRenderer);

	void	SetTexture(char const* strPath);

	PgLinkedPlane*	AddNewPlane(int const iAlphaTestRef);
	PgLinkedPlane*	GetPlane(int const iInstanceID);
	void	ReleasePlane(int const iInstanceID);
	void	ReleaseAllPlane();

	int	const GetInstanceID() { return m_iInstanceID; }

	float	GetTextureCoordinateUpdateDelay() { return m_fTextureCoordinateUpdateDelay; }
	void	SetTextureCoordinateUpdateDelay(float const fDelay);

private:
	void	Destroy();
	void	Create(int const iInstanceID);

private:
	typedef	std::vector<PgLinkedPlane*> PlaneCont;

	PlaneCont	m_PlaneCont;

	int	m_iInstanceID;
	int	m_iPlaneInstanceIDCounter;

	float m_fTextureCoordinateUpdateDelay;

	NiTexturingPropertyPtr		m_spTexturingProp;
	NiAlphaPropertyPtr			m_spAlphaProp;
	NiVertexColorPropertyPtr	m_spVertexColorProp;
	NiZBufferPropertyPtr		m_spZBufferProp;
	NiStencilPropertyPtr		m_spStencilProp;
	NiMaterialPropertyPtr		m_spMaterialProp;
};

class PgMobileSuit;
class PgLinkedPlaneGroupMan
{
public:
	typedef std::vector<PgLinkedPlaneGroup*> PlaneGroupCont;

private:

	PlaneGroupCont	m_PlaneGroupCont;

public:

	PgLinkedPlaneGroupMan()		{ Create(); }
	~PgLinkedPlaneGroupMan()	{ Destroy(); }

	PgLinkedPlaneGroup*	CreateNewLinkedPlaneGroup();
	PgLinkedPlaneGroup*	GetLinkedPlaneGroup(int const iInstanceID);
	void	ReleaseLinkedPlaneGroup(int const iInstanceID);

	void	Update(float fAccumTime,float fFrameTime);
	void	DrawImmediate(PgRenderer* pkRenderer);

	void	ClearAll();

private:
	void	Create();
	void	Destroy();

private:
	friend class PgMobileSuit;

	int	m_iPlaneGroupInstanceIDCounter;
};

extern	PgLinkedPlaneGroupMan g_kLinkedPlaneGroupMan;

#endif // FREEDOM_DRAGONICA_RENDER_EFFECT_PGLINKEDPLANE_H