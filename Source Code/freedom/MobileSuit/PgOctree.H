#ifndef FREEDOM_DRAGONICA_SCENE_WORLD_PGOCTREE_H
#define FREEDOM_DRAGONICA_SCENE_WORLD_PGOCTREE_H
#include "NiMain.h"

class	PgBoundingBox
{
	NiPoint3	m_vMin,m_vMax;

public:

	PgBoundingBox()
	{
	}
	PgBoundingBox(const	NiBound &kBound);
	PgBoundingBox(const	NiPoint3 &vCenter, float fRadius);
	PgBoundingBox(const	NiPoint3 &vMin,const		NiPoint3 &vMax);

	void	Set(const	NiBound &kBound);
	void	Set(const	NiPoint3 &vCenter, float fRadius);
	void	Set(const	NiPoint3 &vMin,const		NiPoint3 &vMax);

	NiPoint3&	GetMin()	{	return	m_vMin;	}
	NiPoint3&	GetMax()	{	return	m_vMax;	}

	bool	IsInBox(const	NiPoint3	vCenter,float fRadius);

};

typedef	std::list<NiAVObject*> AVObjectList;

struct	stTreeNode;
typedef	std::list<stTreeNode*> NodeList;

struct	stTreeNode
{
	NiBound	m_kBound;

	NodeList	m_kChildList;
	AVObjectList	m_kObjList;

	int	m_iTotalChild;
	int	m_iTotalObject;

	stTreeNode	**m_paChildArray;
	NiAVObject	**m_paObjArray;

	stTreeNode():m_paChildArray(NULL),m_paObjArray(NULL),m_iTotalChild(0),m_iTotalObject(0)
	{
	};

	virtual	~stTreeNode()
	{
		for(NodeList::iterator itor = m_kChildList.begin(); itor != m_kChildList.end(); itor++)
		{
			stTreeNode *pkChild = *itor;
			SAFE_DELETE(pkChild);
		}	

		m_kChildList.clear();
	}
};

class	PgSphereTree
{
	
	stTreeNode	m_kTopNode;
	bool	m_bTopNodeInit;

	NiFrustumPlanes	m_kPlanes;

public:

	PgSphereTree();

	void	AddNode(NiNode	*pStartNode);
	void	GenerateTree(NiNode	*pStartNode);

	void CullingProcess(PgRenderer *pkRenderer,NiCamera *pkCamera, NiVisibleArray *pkVisibleArray,stTreeNode *pkNode = NULL);

private:

	void	AddGeometry(NiAVObject	*pkNode);

	void CullingProcess(PgRenderer *pkRenderer,NiVisibleArray *pkVisibleArray,NiAVObject *pkAVObject);

	void	GenerateRecursive(NiAVObject *pkObject,int iLevel);

	void	SliceNodeWithPlane(NiNode *pTargetNode,NiPlane Plane,NiNode **pout_NewNode1,NiNode **pout_NewNode2);

	void	Destroy();
	NiNode*	FindAddNode(NiNode *pkParentNode,NiAVObject *pkObject,float const fMaxRadius);
};

#endif // FREEDOM_DRAGONICA_SCENE_WORLD_PGOCTREE_H