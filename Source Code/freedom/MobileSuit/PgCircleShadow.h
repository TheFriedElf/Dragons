#ifndef FREEDOM_DRAGONICA_RENDER_EFFECT_PGCIRCLESHADOW_H
#define FREEDOM_DRAGONICA_RENDER_EFFECT_PGCIRCLESHADOW_H

#include "NiGeometry.H"

class	 PgCircleShadow	:	public	NiGeometry
{
	NiDeclareRTTI;

protected:
	NiAVObjectPtr	m_spShadowGeom;

	float		m_fAlpha;
	float		m_fScale;
	float		m_fMaxShadowDistance;
	float		m_fShadowScale;
	
	NiMatrix3	m_matRot;

	NiAVObject*	m_pSrcObject;

	NxRaycastHit	*m_pkBottomRayHit;


public:
	PgCircleShadow();
	virtual	~PgCircleShadow()	{	Destroy();	}

	virtual void RenderImmediate(NiRenderer* pkRenderer);

	void SetMaxShadowDistance( float const fValue = 200.0f )	{	m_fMaxShadowDistance = fValue;	}
	void SetShadowScale( float const fValue = 1.0f )			{	m_fShadowScale = fValue;	}
	void SetPositionSourceObject(NiAVObject* pkSourceObject);
	void SetShadowNif(NiAVObject* pkNewNif);
	void SetBottomRayHit(NxRaycastHit* pkBottomRayHit)
	{
		m_pkBottomRayHit = pkBottomRayHit;
	}

	NiPoint3 const& GetShadowPos()	
	{
		return m_spShadowGeom->GetWorldTranslate();
	}

	void SetTexture(char const* strTexturePath);

public:

	static void DetachCircleShadowRecursive(NiAVObject* pkObject);

	static int AttachCircleShadowRecursive(NiAVObject* pkTargetNode,
		float const	fMaxShadowDistance=200.0f,
		float const	fShadowScale=1.0f,
		NiAVObject const* pkShadowNif=0,
		NiAVObject* pSrcObject=NULL,
		NxRaycastHit* pkBottomRayHit = 0);

	static	PgCircleShadow*	AttachNewShadowTo(NiNode* pkTargetNode,
		float const	fMaxShadowDistance=200.0f,
		float const	fShadowScale=1.0f,
		NiAVObject const* pkShadowNif=0,
		NiAVObject* pSrcObject=NULL
		,NxRaycastHit* pkBottomRayHit = 0);

	static	void	ShowShadowRecursive(NiAVObject* pkTargetNode, bool const bShow);
	static	void	AddOnlyShadowToVisibleArrayRecursive(
		NiAVObject* pkTargetNode,
		PgRenderer* pkRenderer,
		NiCamera* pkCamera);

protected:

	virtual void Init();
	virtual void Destroy();

	void LoadNif();

	virtual	void UpdateWorldBound();

private:

	static	PgCircleShadow*	GetShadowChild(NiAVObject* pkTargetNode);

	bool	UpdateWorldBoundEx();

};

NiSmartPointer(PgCircleShadow);

#endif // FREEDOM_DRAGONICA_RENDER_EFFECT_PGCIRCLESHADOW_H