#ifndef FREEDOM_DRAGONICA_SCENE_HOME_PGHOME_H
#define FREEDOM_DRAGONICA_SCENE_HOME_PGHOME_H

#include "PgWall.h"

int const DefaultTileCount = 2;

class PgFurniture;

typedef struct tagTypeContainer
{
	int iType;
	int iPos;
}STypeContainer;

class PgWorld;
class PgRenderer;

class PgHome : public NiMemObject
{
	typedef std::list<PgWall*> WallContainer;
public:
	PgHome(PgWorld* pkWorld);
	~PgHome();

	void Initialize();
	void Terminate();
	// 집을 구축한다.
	void Build();

	// 배치모드로 돌입한다.
	void SetArrangeMode(bool bArrangeMode);
	bool SelectArrangingFurniture();// 마우스 클릭하면 ArrangingFurniture 가 된다.
	void ClearArrangingFurniture();// ArrangingFurniture 삭제
	void SetArrangingFurniture(bool bForce = false);// ArrangingFurniture 놓일 위치에 놓는다.
	
	// 배치할 가구를 설정한다.
	void SetArrangingFurniture(PgFurniture* pkFurniture);

	// 배치모드인가?
	bool IsArrangeMode();

	// 갱신
	void Update(float fAccumTime,float fFrameTime);
	void Draw(PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime);
	bool ProcessInput(PgInput *pkInput);

	// 배치할 가구를 근접한 벽에 붙인다.
	bool SnapArrangingFurniture(float fAccumTime);

	// 배치 중인 가구를 반환한다.
	PgFurniture* GetArrangingFurniture();

	// 벽들을 구성한다.
	void BuildWalls();

	// 점이 방안(벽)에 있는지 검사한다.
	bool IsInside(const NiPoint3 &rkPt);

	NiPoint3 GetCenterPos() { return m_kCenterPos; }
	NiNodePtr GetHomeRoot() { return m_spRoot; }

	// 아이템 번호를 참조해서 파일에서 오브젝트를 생성한다.
	PgFurniture* AddFurniture(int iFurnitureNo, NiPoint3& kPos, int iRotate = 0, BM::GUID const& kGuid = BM::GUID::NullData(), bool bLocate = true, 
		BM::GUID const& kParentGuid = BM::GUID::NullData(), BYTE byLinkIndex = 0 );

	void SetRoomSize(POINT3I kRoomSize) { m_kRoomSize = kRoomSize; }
	void SetHomeOrigin(NiPoint3& rkOrigin);
	void SetLight();

	BM::GUID GetOwnerGuid();
	BM::GUID GetHomeGuid();
	//void SetOwnerGuid(BM::GUID& rkGuid) { m_kOwnerGuid = rkGuid; }

	PgWall* GetWall(PgWall::WallType eWallType);

	void AddWall(NiPoint3& rkPos, int iRotate);
	void RemoveWall();
	void RemoveAllFurniture();

	bool LocateArrangingFurniture();
	bool UnlocateArrangingFurniture(PgFurniture* pkFurniture);
	
	void SetHomeUnit(CUnit* pkUnit);
	PgMyHome* GetHomeUnit();
	bool HomeUnitAddProcess();
	
	void ReceivePacket_Command(WORD const wPacketType, BM::Stream& kPacket);

	bool HomeEquipRequest(PgFurniture* pkFurniture, BM::GUID const& kParentGuid = BM::GUID::NullData(), BYTE byPos = 0);
	bool HomeItemModifyRequest(PgFurniture* pkFurniture, BM::GUID const& kParentGuid = BM::GUID::NullData(), BYTE byPos = 0);
	bool HomeUnEquipRequest(PgFurniture* pkFurniture);

	void AddFurniturebyGuid(BM::GUID const& kGuid);
	PgWall::WallType GetWallTypetoFurniture(EFurnitureType eFurnitureType);
	void RemoveFurniturebyServer(BM::GUID const& kGuid, int iItemNo = 0);

	bool AddFurniturebyInven(int iItemNo, BM::GUID const& kGuid);
	bool RemoveArrangingFurniture();

	bool SelectArrangingFurnitureByMenu();
	void RemovePickedFurniture();

	bool IsMyHome();
	void CancelLocateArrangingFurniture();
	void HideAllCellTiles(bool bHide);

	bool HomeItemUseStyleItem(SItemPos const& rkItemPos);

	bool PickCell(PgWall* pkWall, NiPoint3& kCellPos);
	PgFurniture* GetFurniture(BM::GUID const& kGuid);

	void HomeItemRequest();
	void SetCharacterRegenPos(PgActor* pkActor);

	bool IsHaveSetPropPlace();// 장식용 아이템을 놓을 자리가 있나
	void SetMyhomeCameraPos(int iPos);

	bool IsHavePostbox();
	bool IsHaveSafebox();

	bool IsUseAbleTimeofPickFurniture();

	typedef struct tagInvenFurniture
	{
		BM::GUID kGuid;
		SItemPos kPos;
	}SInvenFurniture;
	typedef std::queue<SInvenFurniture> ContInvenFurniture;

	void UnsetAllFurniture();// 모두장착해제해서 인벤으로 돌린다.
	void ReleaseCellsOccupancy();

	void SetGrade(int iGrade) { m_iGrade = iGrade; }
	int const GetGrade() { return m_iGrade; }

	// 메이드 npc 처리
	bool IsMaidItem(int iItemNo);
	bool SendHomeMaidEquip(int iItemNo, BM::GUID const& kGuid);
	void SetHideMaidNpc(int iItemNo, bool bHide, NiPoint3 kPos);
	void SetMaidColor(int iItemNo, const NiColor &kColor);
	void AddMaidNpc(int iItemNo, NiPoint3 kPos);
	bool SendHomeMaidUnEquip(BM::GUID const& kNpcGuid);
	void RemoveMaidNpc(int iItemNo);
	bool IsExistMaidNpc();
	bool IsUseAbleTimeMaidNPC(BM::GUID const& kNpcGuid);
	bool SetMaidNpcMoving(BM::GUID const& kNpcGuid);


protected:
	void Send_PT_C_M_REQ_HOME_EQUIP(BM::GUID const& kUserGuid, SItemPos kItemPos, POINT3 kPos, SHORT kDir, BM::GUID const& kParentFurnitureGuid, BYTE byPosIndex);
	void Send_PT_C_M_REQ_HOME_ITEM_MODIFY(BM::GUID const& kUserGuid, SItemPos kItemPos, POINT3 kPos, SHORT kDir, BM::GUID const& kParentFurnitureGuid, BYTE byPosIndex);
	void Send_PT_C_M_REQ_HOME_UNEQUIP(BM::GUID const& kGuid, SItemPos kItemPos);
	void Send_PT_C_M_REQ_HOME_USE_HOME_STYLEITEM(SItemPos kItemPos, DWORD dwClientTime, BM::GUID const& kHomeGuid);

	bool SetPickFurniture(bool const bOnlyMyActor=true);

	void SetAllWallTile();
	bool SetWallTile(int iItemNo);
	void ReleaseWallTile();
	void RemoveFurnitreProcess();

	bool CheckErrorMsg(HRESULT kResult, bool bShowMessageBox);

protected:
	PgFurniture* pickFurniture(NiPoint3& kStartPt, NiPoint3& kRayDir);

	PgWorld *m_pkWorld;
	NiNodePtr m_spRoot;
	NiNodePtr m_spWallRoot;
	// 배치 모드 여부
	bool m_bArrangeMode;
	POINT3I m_kRoomSize;
	POINT2 m_kLastMousePos;
	NiPoint3 m_kCenterPos;
	//BM::GUID m_kOwnerGuid;

	// 배치 중인 가구
	PgFurniture *m_pkArrangingFurniture;
	WallContainer m_kWallContainer;
	FurnitureContainer m_kHomeFurnitures;	// Wall에 속하지 않는 Furnitures;

	PgMyHome* m_pkHomeUnit;

	PgFurniture* m_pkPickedFurniture;

	PgFurniture* m_pkFloorTile;
	PgFurniture* m_pkWallTile;
	PgFurniture* m_pkWallCornerTile;

	int m_iGrade;

	FurnitureContainer m_kRemoveFurniture;// 가구제거를 한번에 여러게 보내면 서버가 처리를 못하므로.

	ContInvenFurniture m_kUnsetFurniture;
	bool m_bAnitAlias;
	static EEquipPos const TilePosArray[DefaultTileCount];
	static int const DefaultTileArray[DefaultTileCount];

///////////////////////////////////////////////////// 1차 개발용 /////////////////////////////////////////////////////
//public:
//	typedef std::map<int, STypeContainer> TypeContainer;
//
//	bool ParseXml(char const* pcXmlPath);
//	PgFurniture* AddFurnitureFirst(int iFurnitureNo);
//	void RemoveFurnitureFirst(int iFurnitureNo);
//
//protected:
//	TypeContainer m_kTypeContainer;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};

#endif // FREEDOM_DRAGONICA_SCENE_HOME_PGHOME_H