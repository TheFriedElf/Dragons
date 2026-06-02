#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_FURNITURE_PGFURNITURE_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_FURNITURE_PGFURNITURE_H

#include "PgIWorldObject.h"
#include "PgItemEx.h"

#define WALL_CELL_UNIT_SIZE		10
#define FLOOR_CELL_OBJECT_SIZE	4
#define WALL_CELL_OBJECT_SIZE	20

#define CELL_OBJECT_LENGTH		40	// 셀 오브젝트의 길이

const float ROTATION_ANGLE = NI_HALF_PI;
const int ROTATION_ANGLE2 = 90;

class PgFurniture : public PgIWorldObject
{
	NiDeclareRTTI;
public:
	PgFurniture();
	virtual ~PgFurniture();

	bool Initialize();
	void Release();

	//! Overriding
	virtual bool Update(float fAccumTime, float fFrameTime);
    virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) {};
	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit = false);
	virtual bool ParseXml(TiXmlNode const* pkNode, void* pArg = 0, bool bUTF8 = false);
	virtual void InitPhysX(NiPhysXScene *pkPhysXScene, int uiGroup);
	virtual void ReleasePhysX();

	//! Overriding
	virtual NiAVObject* GetNIFRoot() const;
	virtual NiObject* CreateClone(NiCloningProcess& kCloning);
	virtual void ProcessClone(NiCloningProcess& kCloning);

	//! 윗 방향을 변경한다.
	void SetNormal(NiPoint3 const &rkNormal);
	void IncRotate(float fRadian);
	void SetRotation(int iRotation);
	int GetRotation() { return m_iRotation; }

	bool SetPosition(NiPoint3 const &rkTranslate);

	void UpdateBound();

	void SetFurnitureType(EFurnitureType eFurnitureType) { m_eFurnitureType = eFurnitureType; }
	EFurnitureType GetFurnitureType() { return m_eFurnitureType; }
	int GetFurnitureNo() { if (m_pkItemDef) return m_pkItemDef->No(); return 0; }
	std::string& GetXMLPath() { return m_kXMLPath; }
	void SetXMLPath(char const* path) { m_kXMLPath = path; }

	static PgFurniture* GetFurnitureFromDef(int iFurnitureNo);
	static void DeleteFurniture(PgFurniture* pkFurniture);
	static bool	IsFurnitureMesh(NiAVObject* pkObject, PgFurniture*& rkFurniture);

	int GetFurnitureDetailType();
	bool SetPhysxTriggerFlag(bool bSet);

	void SetBeforeTran(NiTransform kTran) { m_kBeforeTran = kTran; }
	NiTransform GetBeforeTran() { return m_kBeforeTran; }

	void SetFurnitureColor(NiColor const& kColor);

	CItemDef *GetItemDef() { return m_pkItemDef; }

	char const* GetMeshPath()const;

	TexturePathContainer& GetTexturePathContainer() { return m_kTextureContainer;}

	void SetParentFurniture(PgFurniture* pkFurniture) { m_pkParentFurniture = pkFurniture; }
	PgFurniture* GetParentFurniture() { return m_pkParentFurniture; }
	void SetParentIndex(int iIndex) { m_iParentIndex = iIndex; }
	int GetParentIndex() { return m_iParentIndex; }

protected:
	void getMinMaxPoint(NiPoint3& rkMinPoint, NiPoint3& rkMaxPoint, NiAVObject* pkObject);
	void calcVertexMinMaxPoint(int iVertexCount, NiPoint3 const* pkData, NiPoint3& rkMinPoint, NiPoint3& rkMaxPoint);

	CItemDef *m_pkItemDef;
	EFurnitureType m_eFurnitureType;
	NiPoint3 m_kNormal;
	float m_fRotation;
	int m_iRotation;
	NiBoxBV m_kBound;
	NiPoint3 m_kCellSize;
	NiPoint3 m_kCellCenter;
	std::string m_kMeshPath;
	TexturePathContainer m_kTextureContainer;
	NiNodePtr m_kMeshRoot;
	std::string m_kXMLPath;

	PgFurniture* m_pkParentFurniture;
	int m_iParentIndex;

	NxActor *m_pkPhysXActor;
	NiPhysXKinematicSrc *m_pkPhysXSrc;
	NiPhysXTransformDest *m_pkPhysXDest;

	NiTransform m_kBeforeTran;
};

typedef std::list<PgFurniture *> FurnitureContainer;
typedef std::map<int, NiPoint3> FurnitureDummyPos;

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_FURNITURE_PGFURNITURE_H