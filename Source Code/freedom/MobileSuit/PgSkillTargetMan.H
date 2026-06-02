#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILL_PGSKILLTARGETMAN_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILL_PGSKILLTARGETMAN_H

//#ifndef USE_INB

#include <NiMain.H>
#include <list>
#include "PgActionTargetList.h"

class	PgActor;
class	PgRenderer;
class	PgPilot;
class	PgMobileSuit;
class	PgAction;

class	PgSkillTargetMan	
{
	friend	class	PgMobileSuit;
	
public:
	enum	TargetAreaType
	{
		TAT_CIRCLE=0,
		TAT_TRIANGLE,
		TAT_RECTANGLE
	};
	
private:
	struct SElem
	{
//		PgActionTargetList	m_kTargetList;

		NiPoint3			*m_pkCubeVertexArray;
		NiPoint3			*m_pkSphereVertexArray;
		NiPoint3			*m_pkConeVertexArray;
		NiLinesPtr			m_spCubeLine;
		NiLinesPtr			m_spSphereLine;
		NiLinesPtr			m_spConeLine;
		NiBool				*m_pkCubeConnect;
		NiBool				*m_pkSphereConnect;
		NiBool				*m_pkConeConnect;

		unsigned long m_ulLastTargetUpdateTime;
		int	m_iUpdateTargetInterval;

		TargetAreaType	m_TargetAreaType;
		NiNodePtr		m_spAreaCircleNif, m_spAreaTriNif, m_spAreaRectNif;
		
		NiPoint3 kDir;
		int iActionNo;
		int iAreaType;
		NiPoint3 kStartPos;
		float fSkillRange;
		float fWideAngle; 
		float fEffectRadius;

		SElem():
			m_pkCubeVertexArray(NULL)
			,m_pkSphereVertexArray(NULL)
			,m_pkConeVertexArray(NULL)
			,m_spCubeLine(NULL)
			,m_spSphereLine(NULL)
			,m_spConeLine(NULL)
			,m_pkCubeConnect(NULL)
			,m_pkSphereConnect(NULL)
			,m_pkConeConnect(NULL)
			,iActionNo(0)
			,iAreaType(0)
			,fSkillRange(0.0f)
			,fWideAngle(0.0f) 
			,fEffectRadius(0.0f)
		{}
	};

	typedef std::map<BM::GUID,SElem> CONT_ELEM;

public:
	PgSkillTargetMan();
	virtual	~PgSkillTargetMan()	{ Destroy(); }

	void	SetTargetListUpdateInterval(BM::GUID const kGUID, int const iInterval);
	
	void	Update(float fAccumTime);
	void	DrawTargetArea(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	void	DrawImmediate(PgRenderer *pkRendere1r, NiCamera *pkCamera, float fFrameTime);
	
	void	EnableTargetAreaDisplay(bool const bEnable);
	void	SetTargetAreaCircle(SElem* pkElem, NiPoint3 const &kCenter,float const fRange);
	void	SetTargetAreaTriangle(SElem* pkElem, NiPoint3 const &kStart,NiPoint3 const &kDir,float const fAngle,float const fRange);
	void	SetTargetAreaRectangle(SElem* pkElem, NiPoint3 const &kStart,NiPoint3 const &kDir,float const fSideRange,float const fRange);

	void	SetDrawTargetMark(bool const bDraw) { m_bDrawTargetMark = bDraw; }

	//공격영역 렌더링 코드
	void SetTargetCone(SElem* pkElem, NiPoint3 const& kPos, NiPoint3 const& kDir, float const fRange,float const fHeight, float const fBaseRadius);
	void SetTargetSphere(SElem* pkElem, NiPoint3 const& kPos, float const fRange , int ILats=10, int iLongs=10) ;
	void SetTargetCube(SElem* pkElem, NiPoint3 const& kPos, NiPoint3 const& kDir, float const fSideRange, float const fRange, float const fHeight);
	
	SElem* AddUnit(BM::GUID const& kGUID);
	void EditSkillRangeParam(BM::GUID const& kGUID, int const iActionNo, int const iAreaType, NiPoint3 const& kDir, NiPoint3 const& kStartPos, float const fSkillRange, float const fWideAngle, float const fEffectRadius);
	void DeleteUnit(BM::GUID const& kGUID);
protected:
	void	Init();//공격영역을 그리는데 쓸 NiLines를 할당하기 위해 Ni*가 초기화 된 이후에 호출해야 함.
	void	Destroy();

	//공격영역 렌더링시 선 연결성 지정함수
	void SetAllConnectivity(NiBool* pkConncet, bool const bSetVal);
	void SetRangeConnectivity(NiBool* pkConncet, size_t const iStartPos, size_t const iDestCount, bool const bSetVal);
	//공격영역 선 색상 지정 함수
	void SetLineColor(NiColor const& kLineColor);


private:
	SElem*	GetUnitElem(BM::GUID const& kGUID);

private:
	static unsigned int const MAX_VERTS;
	NiMaterialProperty	*m_pkMaterial;
	bool			m_bDrawTargetArea;
	bool	m_bDrawTargetMark;
	NiNodePtr			m_spTargetingNif;
	NiNodePtr			m_spSelectEnemyNif;
	float	m_fSavedAccumTime;
	
	CONT_ELEM m_kContElem;
	Loki::Mutex m_kLock;
};
extern	PgSkillTargetMan	g_kSkillTargetMan;


class	PgMonSkillTargetMan	
{
	friend	class	PgMobileSuit;

public:

	enum	TargetAreaType
	{
		TAT_CIRCLE=0,
		TAT_TRIANGLE,
		TAT_RECTANGLE
	};

private:

	static unsigned int const MAX_VERTS;

	PgActionTargetList	m_kAreaInEnemyList;
	PgActionTargetList	m_kTargetList;
	NiNodePtr			m_spTargetingNif;
	NiNodePtr			m_spSelectEnemyNif;
	
	NiPoint3				*m_pkCubeVertexArray, *m_pkSphereVertexArray, *m_pkConeVertexArray;
	NiMaterialProperty	*m_pkMaterial;
	NiLinesPtr			m_spCubeLine, m_spSphereLine, m_spConeLine;	 
	NiBool				*m_pkCubeConnect, *m_pkSphereConnect, *m_pkConeConnect;

	unsigned long m_ulLastTargetUpdateTime;
	int	m_iUpdateTargetInterval;

	float	m_fSavedAccumTime;
	bool	m_bDrawTargetMark;

	PgAction	*m_pkDefaultAttackAction;	//	기본 공격 액션

	bool			m_bDrawTargetArea;
	TargetAreaType	m_TargetAreaType;
	NiNodePtr		m_spAreaCircleNif, m_spAreaTriNif, m_spAreaRectNif;

public:

	PgMonSkillTargetMan();
	virtual	~PgMonSkillTargetMan()	{ Destroy(); }

	void	SetTargetListUpdateInterval(int const iInterval);
	
	void	Update(float fAccumTime);
	void	DrawTargetArea(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
	void	DrawImmediate(PgRenderer *pkRendere1r, NiCamera *pkCamera, float fFrameTime);
	void	UpdateTargetList();
	
	void	EnableTargetAreaDisplay(bool const bEnable);
	void	SetTargetAreaCircle(NiPoint3 const &kCenter,float const fRange);
	void	SetTargetAreaTriangle(NiPoint3 const &kStart,NiPoint3 const &kDir,float const fAngle,float const fRange);
	void	SetTargetAreaRectangle(NiPoint3 const &kStart,NiPoint3 const &kDir,float const fSideRange,float const fRange);

	void	SetDrawTargetMark(bool const bDraw) { m_bDrawTargetMark = bDraw; }

	//공격영역 렌더링 코드
	void SetTargetCone(NiPoint3 const& kPos, NiPoint3 const& kDir, float const fRange,float const fHeight, float const fBaseRadius);
	void SetTargetSphere(NiPoint3 const& kPos, float const fRange , int ILats=10, int iLongs=10) ;
	void SetTargetCube(NiPoint3 const& kPos, NiPoint3 const& kDir, float const fSideRange, float const fRange, float const fHeight);

protected:

	void	Init();//공격영역을 그리는데 쓸 NiLines를 할당하기 위해 Ni*가 초기화 된 이후에 호출해야 함.
	void	Destroy();

	//공격영역 렌더링시 선 연결성 지정함수
	void SetAllConnectivity(NiBool* pkConncet, bool const bSetVal);
	void SetRangeConnectivity(NiBool* pkConncet, size_t const iStartPos, size_t const iDestCount, bool const bSetVal);
	//공격영역 선 색상 지정 함수
	void SetLineColor(NiColor const& kLineColor);


private:
	void	ClearTargetList();

	bool	CreateDefaultAttackAction();

};

extern	PgMonSkillTargetMan	g_kMonSkillTargetMan;
//#endif//USE_INB

#endif // FREEDOM_DRAGONICA_CONTENTS_SKILL_PGSKILLTARGETMAN_H