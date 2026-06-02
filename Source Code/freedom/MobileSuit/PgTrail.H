#ifndef FREEDOM_DRAGONICA_RENDER_EFFECT_PGTRAIL_H
#define FREEDOM_DRAGONICA_RENDER_EFFECT_PGTRAIL_H

#include "PgIWorldObject.h"

//	검이나 도끼등 무기를 휘둘렀을 때, 혹은 발사체가 이동할때, 그 궤적을 보여주는 클래스

#define	TRAIL2_INITIAL_VERTEX_COUNT	1000
#define MIN_TRAIL_UPDATE_DELAY	0.01
class	PgTrailNode	:	public	NiNode
{
public:

	struct	stTrailRibbonLine
	{
		NiPoint3	m_vStart,m_vEnd;
		float	m_fCreateTime;

		stTrailRibbonLine()
		{
			m_fCreateTime= 0;
		}
		stTrailRibbonLine(NiPoint3	const &vStart,NiPoint3 const &vEnd,float const &fCreateTime)
		{
			Set(vStart,vEnd,fCreateTime);
		}

		void	Set(NiPoint3	const &vStart,NiPoint3 const &vEnd,float const &fCreateTime)
		{
			m_vStart = vStart;
			m_vEnd = vEnd;
			m_fCreateTime = fCreateTime;
		}

		void	Add(stTrailRibbonLine *pkLine)
		{
			m_vStart += pkLine->m_vStart;
			m_vEnd += pkLine->m_vEnd;
			m_fCreateTime += pkLine->m_fCreateTime;
		}
		void	Subtract(stTrailRibbonLine *pkLine)
		{
			m_vStart -= pkLine->m_vStart;
			m_vEnd -= pkLine->m_vEnd;
			m_fCreateTime -= pkLine->m_fCreateTime;
		}
	};
	
	typedef	std::vector<stTrailRibbonLine*>	TrailRibbonLineVector;

	struct	stTrailRibbon
	{
		NiAVObjectPtr	m_spStartDummy,m_spEndDummy;
		NiPoint3	m_kPrevCenterPoint;

		TrailRibbonLineVector	m_vControlLines;
		TrailRibbonLineVector	m_vDrawLines;

		stTrailRibbon()
		{
			m_spStartDummy = 0;
			m_spEndDummy = 0;
			m_kPrevCenterPoint = NiPoint3(-1,-1,-1);
		}
		~stTrailRibbon()
		{
			m_spStartDummy = 0;
			m_spEndDummy = 0;

			ReleaseTrailRibbonLineVector(m_vControlLines);
			ReleaseTrailRibbonLineVector(m_vDrawLines);
		}

		static	void	ReleaseTrailRibbonLineVector(TrailRibbonLineVector &kLineVector);
	};

	typedef	std::vector<stTrailRibbon*>	TrailRibbonVector;

private:

	NiAVObjectPtr	m_spSrcObj;
	NiTriShapePtr	m_spGeom;

	bool	m_bRemoveReserved;	//	궤적이 모두 사라지면, 자동으로 제거되도록 설정한다.
	bool	m_bFinished;

	TrailRibbonVector	m_vTrailRibbons;

	float	m_fTotalExistTime;
	float	m_fBrightTime;
	float	m_fTrailWidth;
	std::string	m_kTexPath;

	float	m_fAccumTimeAdjust;

	float	m_fLastUpdateTime;

public:

	PgTrailNode();
	~PgTrailNode()	
	{	
		Destroy();	
	}

	void	Init(NiAVObject* pkSrcObj,std::string const &kTexPath,float fTotalExistTime,float fBrightTime);
	void	Init(NiAVObject* pkSrcObj,std::string const &kCenterObjName,std::string const &kTexPath,float fTotalExistTime,float fBrightTime,float fTrailWidth);

	void UpdateDownwardPass(float fTime, bool bUpdateControllers);	

	void	ReserveRemove()	{	m_bRemoveReserved	=	true;	}
	void	SetAccumTimeAdjust(float fAccumTimeAdjust)	{	m_fAccumTimeAdjust	=	fAccumTimeAdjust;	}

	bool	IsFinished()	{	return	m_bFinished;	}

private:

	void	Create(NiAVObjectPtr spSrcObj,std::string kTexPath,float fTotalExistTime,float fBrightTime);
	void	Destroy();

	void	InitializeTrailRibbons();
	void	InitializeTrailRibbons(std::string const &kCenterObjName);

	void	DestroyTrailRibbons();

	bool	UpdateRibbons(float	fTime);	//	return : 그릴것이 남지 않았을 때 false 리턴. 
	int	UpdateRibbon(float	fTime,stTrailRibbon	*pkRibbon);

	void	CreateGeom(int iVertexCount);	
	void	UpdateGeom(int iTotalTriangles,float fTime);

};
NiSmartPointer(PgTrailNode);

class	PgTrailNodeMan	:	public	NiNode
{
public:

	PgTrailNodeMan()	{	Create();	}
	~PgTrailNodeMan()	{	Destroy();	}

	PgTrailNode*	StartNewTrail(NiAVObject* pkSrcObj,std::string const &kTexPath,float fTotalExistTime,float fBrightTime);
	PgTrailNode*	StartNewTrail(NiAVObject* pkSrcObj,std::string const &kCenterObjName,std::string const &kTexPath,float fTotalExistTime,float fBrightTime,float fTrailWidth);
	void	StopTrail(PgTrailNode*	pkTrailNode,bool bRemoveImm);
	void UpdateDownwardPass(float fTime, bool bUpdateControllers);	

	//! pkRenderer를 이용해서 Draw
	virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

private:

	void	Create();
	void	Destroy();
};
NiSmartPointer(PgTrailNodeMan);

extern	PgTrailNodeManPtr	g_spTrailNodeMan;

#endif // FREEDOM_DRAGONICA_RENDER_EFFECT_PGTRAIL_H