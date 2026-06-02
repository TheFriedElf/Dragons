#ifndef FREEDOM_DRAGONICA_SCENE_WORLD_PGPATHDATA_H
#define FREEDOM_DRAGONICA_SCENE_WORLD_PGPATHDATA_H

#include <NiMain.H>
#include "PgWorld.H"
#include <list>
#include <vector>

struct	stTriangle
{
	NiPoint3	m_vPoints[3];
	NiPoint3	m_vNormal;

	stTriangle()	{};
	stTriangle(NiPoint3 const &kP1,
			NiPoint3 const &kP2,
			NiPoint3 const &kP3)
	{
		m_vPoints[0]=kP1;
		m_vPoints[1]=kP2;
		m_vPoints[2]=kP3;
	};
	stTriangle(NiPoint3 const &kP1,
			NiPoint3 const &kP2,
			NiPoint3 const &kP3,
			NiPoint3 const &kNormal)
	{
		m_vPoints[0]=kP1;
		m_vPoints[1]=kP2;
		m_vPoints[2]=kP3;
		m_vNormal = kNormal;
	};
	stTriangle(const NxVec3 &kP1,
			const NxVec3 &kP2,
			const NxVec3 &kP3)
	{
		m_vPoints[0]=NiPoint3(kP1.x,kP1.y,kP1.z);
		m_vPoints[1]=NiPoint3(kP2.x,kP2.y,kP2.z);
		m_vPoints[2]=NiPoint3(kP3.x,kP3.y,kP3.z);
	};
	stTriangle(const NxVec3 &kP1,
			const NxVec3 &kP2,
			const NxVec3 &kP3,
			const NxVec3 &kNormal)
	{
		m_vPoints[0]=NiPoint3(kP1.x,kP1.y,kP1.z);
		m_vPoints[1]=NiPoint3(kP2.x,kP2.y,kP2.z);
		m_vPoints[2]=NiPoint3(kP3.x,kP3.y,kP3.z);
		m_vNormal = NiPoint3(kNormal.x,kNormal.y,kNormal.z);
	};
};

typedef	std::list<stTriangle> TriPointCont;
typedef	std::vector<stTriangle> TriPointVec;

class	PgPathTriangle
{
public:
	struct	stPortal
	{
		NiPoint3	m_vPoints[2];	//	포털을 구성하는 두 점
		PgPathTriangle	*m_pNTri;	//	이웃 삼각형

		stPortal():m_pNTri(0)
		{
		}
		stPortal(NiPoint3 const &kP1,NiPoint3 const &kP2,PgPathTriangle *pkNTri)
		{
			m_vPoints[0] = kP1;
			m_vPoints[1] = kP2;
			m_pNTri = pkNTri;
		}
		~stPortal()
		{
			SAFE_DELETE(m_pNTri);
		}
	};

	typedef	std::list<stPortal*> ContPortal;

private:

	ContPortal	m_PortalCont;

	stTriangle	m_kPoints;	//	삼각형의 세점
	unsigned int	m_uiID;

public:

	PgPathTriangle(
		const stTriangle &kPoint,
		unsigned	int	uiID);

	virtual	~PgPathTriangle();

	const	stTriangle&	GetPoints()	{	return	m_kPoints;	}
	const	ContPortal&	GetPortalCont()	{	return	m_PortalCont;	}

public:

	bool	CanConnectToThisTriangle(const stTriangle &kPoint);
	void	ConnectToThisTriangle(PgPathTriangle *pkTriangle);
};

//	연결 되어 있는 삼각형 집합
class	PgPathGroup
{
	unsigned	int	m_uiGroupID;
	unsigned int	m_uiTriCount;
	unsigned	int	m_uiTriangleID;

	PgPathTriangle	*m_pkFirstTri;

public:

	PgPathGroup(unsigned int uiGroupID);
	virtual	~PgPathGroup();

	void	ClearAll();

public:

	bool	CanConnectToThisGroup(const stTriangle &kPoint);
	void	ConnectToThisGroup(const stTriangle &kPoint);

private:

	PgPathTriangle*	GetConnetableTriangle(const stTriangle &kPoint,PgPathTriangle *pkTriangle);

};


typedef	std::list <PgPathGroup*> ContPathGroup;

class	PgPathData
{
	ContPathGroup	m_PathGroupCont;
	unsigned	int	m_uiGroupID;

public:


	PgPathData();
	virtual	~PgPathData();

	bool	CreatePathDataFromPhysX(NxScene *pkScene);
	void	ClearData();


private:

	void	AddTriangle(const stTriangle &kPoint);

	PgPathGroup*	GetCorrectGroup(const stTriangle &kPoint);
	PgPathGroup*	CreateNewGroup();
};

#endif // FREEDOM_DRAGONICA_SCENE_WORLD_PGPATHDATA_H