#ifndef FREEDOM_DRAGONICA_SCENE_HOME_PGWALL_H
#define FREEDOM_DRAGONICA_SCENE_HOME_PGWALL_H

#include "PgFurniture.h"

#define CELL_CURSOR_OK	"../Data/3_World/97_Myhome/09_Texture/cursor_t01.dds"
#define CELL_CURSOR_NO	"../Data/3_World/97_Myhome/09_Texture/cursor_t02.dds"

class PgRenderer;

class PgWall : public NiMemObject
{
public:
	typedef enum {
		WALL_TYPE_NONE = -1,
		WALL_TYPE_FLOOR,
		WALL_TYPE_WALL,
		MAX_WALL_TYPE,
	} WallType;

	typedef struct _stCell
	{
		bool m_bOccupied;
		PgFurniture* m_pkOccupiedObject;
		NiAVObjectPtr m_spCellTile;
		NiAVObjectPtr m_spBaseCell;

		_stCell()
		{
			m_bOccupied = false;
			m_pkOccupiedObject = NULL;
			m_spCellTile = NULL;
			m_spBaseCell = NULL;
		}

		void Clear()
		{
			m_bOccupied = false;
			m_pkOccupiedObject = NULL;
			m_spCellTile = NULL;
			m_spBaseCell = NULL;
		}
	} PgCell;
public:
	// 생성자
	PgWall();
	~PgWall();

	void Terminate();
	void SetWallInfo(WallType eWallType, NiPoint3& rkLeftTop, NiPoint3& rkRightBottom);
	void UpdateBound();
	bool IsInside(NiPoint3 const &rkPt);
	bool IsValidWall() { return (m_eWallType > WALL_TYPE_NONE && m_eWallType < MAX_WALL_TYPE); }

	void SetCellObject(NiAVObjectPtr spCellObject, bool bDefault = false);
	void SetCellObject(PgFurniture* pCellObject, bool bDefault = false);
	bool BuildWallCells();
	bool BuildGardenCells();
	bool ChangeWallCell(NiAVObjectPtr spCellObject);
	bool ChangeWallCell(PgFurniture* pCellObject);
	bool ChangeWallTextures(NiFixedString strTextureName, NiFixedString strNewTexture);
	bool AttachFurniture(PgFurniture* pkAttachment, NiPoint3 const & rkPos, int iRotate, BM::GUID const& kGuid);
	bool MoveFurniture(PgFurniture* pkAttachment, NiPoint3 const & rkPos, int iRotate, bool bForce = false);
	void RemoveAllFurniture();
	void RemovePhysxWalls();
	void RemoveFurniture(PgFurniture* pkFurniture); // parameter로 뭘 받을 것인가?
	void RemoveFurniture(BM::GUID const& kGuid); // parameter로 뭘 받을 것인가?
	PgFurniture* PickFurniture(NiPoint3& kStartPt, NiPoint3& kRayDir);
	NiAVObject* PickCell(NiPoint3& kStartPt, NiPoint3& kRayDir, NiPoint3& kCellPosition);
	void ShowCellTile(int iCellX, int iCellY, bool bShow, bool bOccupied);
	void ShowCellTile(PgFurniture* pkFurniture);
	void HideAllCellTiles(bool bHide);

	void SetWallOrigin(NiPoint3& rkPoint);
	NiAVObject* GetWallRoot() { return m_kWallRoot; }
	WallType GetWallType() { return m_eWallType; }	
	bool IsFloor() { return m_eWallType == WALL_TYPE_FLOOR; }
	bool IsWall() { return m_eWallType == WALL_TYPE_WALL; };	

	virtual bool Update(float fAccumTime, float fFrameTime);
	virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

	static bool	IsCellMesh(NiAVObject* pkObject, NiAVObject*& rkFurniture);

	NiPoint3 GetNormal();
	bool LocateFurniture(PgFurniture* pkFurniture);// 체크없이 오브젝트를 배치한다.
	//bool LocateArrangingFurniture(PgFurniture* pkFurniture);// 체크후 오브젝트를 배치한다.
	bool UnlocateArrangingFurniture(PgFurniture* pkFurniture);

	void ClearAllCells();
	void AddCellBoundUnit(int iPos, NiPoint3 const& kOffset, NiQuaternion const& kRot, int iKind = 1);
	void SetCellsBound(int iColCellNum, int iColCellNum2, float fZOffset = 0);
	void SetGardenCellsBound(float fZOffset = 0);

	bool CheckCellsBound(PgFurniture* pkFurniture);
	bool CheckOtherFurniture(PgFurniture* pkFurniture);
	PgFurniture* CheckParentFurniture(PgFurniture* pkFurniture, NiPoint3 const& kPos, int& iDummyPos);

	void SetPhysxNum( int iNum ) { m_iPhysxNum = iNum; }
	int GetPhysxNum() { return m_iPhysxNum; }

	PgFurniture* GetFurniture(BM::GUID const& kGuid);

	PgFurniture* GetDoorTile() { return m_pkDoorTile; }
	bool LocateObjectAttachFurniture(PgFurniture* pkFurniture);

	bool GetChildList(PgFurniture* pkFurniture, FurnitureContainer& contList);
	bool GetCanUseDummyPos(PgFurniture* pkFurniture, FurnitureDummyPos& contPos);
	NiAVObject* GetDummyNode(PgFurniture* pkFurniture, int iIndex );

	bool IsHaveSetPropPlace();// 장식용 아이템을 놓을 자리가 있나
	bool FindUnderCellofFurniture(PgFurniture* pkFurniture, NiPoint3 kRayDir, int iStart, int iEnd, int& iPos);
	void ReleaseCellsOccupancy();
protected:	
	static NiNodePtr getCellTile();
	void removeFurniture(PgFurniture* pkFurniture);
	NiPoint3 getWallCenter();
	int whichSide(NiPoint3 const& rkPos);		// WALL_TYPE_WALL에서만 쓰임.
#ifndef _DEBUG
	void attachArrowToPickPoint(NiPoint3 const& rkPos, NiPoint3 const& rkNormal);
	void removeAllArrow();
#else
#define attachArrowToPickPoint(x)	__noop(x);
#define removeAllArrow(x)			__noop(x);

#endif

	// 메쉬의 한 점에서 가까운 셀 위치를 얻어낸다.
	NiPoint3 findCellPosition(NiAVObject* pkCellMesh, NiPoint3 const& rkPos);
	// 벽의 왼쪽-아래 모서리
	NiPoint3 m_kLeftTop;
	
	// 벽의 오른쪽-위 모서리
	NiPoint3 m_kRightBottom;
	NiPoint2 m_kTotalCellCount;
	PgCell* m_pkArrCells;
	PgCell* m_pkCellsBound1;
	PgCell* m_pkCellsBound2;

	int m_iCellsBound1Count;
	int m_iCellsBound2Count;
		
	// 벽면 보는 방향
	NiPoint3 m_kNormal;
	
	// 벽의 BV
	NiBoxBV m_kBound;

	WallType m_eWallType;
	NiAVObjectPtr m_kCellObject;
	NiAVObjectPtr m_kDefaultCellObject;
	//PgFurniture* m_kCellObject;
	//PgFurniture* m_kDefaultCellObject;

	int m_iPhysxNum;
#ifndef _DEBUG
	NiTObjectArray<NiAVObjectPtr> m_kArrowArray;	
#endif
	NiNodePtr m_kWallRoot;
	FurnitureContainer m_kWallAttachments;
	FurnitureContainer m_kWalls;

	PgFurniture* m_pkDoorTile;
};

#endif // FREEDOM_DRAGONICA_SCENE_HOME_PGWALL_H