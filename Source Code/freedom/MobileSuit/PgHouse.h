#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_HOUSE_PGHOUSE_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_HOUSE_PGHOUSE_H
#include "PgIWorldObject.h"
#include "PgWall.h"

int const DefaultExteriaCount = 4;

class PgBase_Item;
//홈의 외관을 월드에서 출력하기 위해서
class PgHouse
	: public PgIWorldObject
{
	NiDeclareRTTI;
public:
	typedef std::map<EEquipPos, PgFurniture*> ExteriaMap;

	PgHouse() { Init(); }
	virtual ~PgHouse();
	virtual void Terminate();
	void Init();
	void AddAllFurniture();	//외부 아이템 한번에 장착(AddUnit될 때)
	void EquipAllExteria();	//외관 아이템 한번에 장착(AddUnit될 때)
	//void MakeTitle();		//문패에 이름 쓰기
	void RemoveFurniture(PgBase_Item const& rkBaseItem);	//아이템 중간 제거
	bool EquipExteria(SMYHOME const& rkBaseItem);//외관 아이템 장착

	PgFurniture* AddFurniture(PgBase_Item const& rkBaseItem);

	//! PgIWorldObject 재정의
	//virtual bool Update(float fAccumTime, float fFrameTime);

	//! PgIWorldObject 재정의
    virtual void Draw(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) { /* hook-method */ };

	//! Action
	virtual bool ProcessAction(PgAction *pkAction,bool bInvalidateDirection=false,bool bForceToTransit = false);

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

    void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime) { /* hook-method */ };
	bool BuildWalls();

	void SetArrangeMode(bool bArrangeMode);
	BM::GUID GetOwnerGuid();
	bool IsMyHouse();
	void CancelLocateArrangingFurniture();
	void ClearArrangingFurniture();
	void AttachFurniture(BM::GUID const& kGuid);
	void SetArrangingFurniture();
	bool LocateArrangingFurniture();
	bool IsArrangeMode();
	bool ProcessInput(PgInput *pkInput);
	bool SetPickFurniture();
	bool HomeEquipRequest(PgFurniture* pkFurniture);
	void Send_PT_C_M_REQ_HOME_EQUIP(BM::GUID const& kUserGuid, SItemPos kItemPos, POINT3 kPos, SHORT kDir, BM::GUID const& kParentFurnitureGuid, BYTE byPosIndex);
	bool HomeItemModifyRequest(PgFurniture* pkFurniture);
	void Send_PT_C_M_REQ_HOME_ITEM_MODIFY(BM::GUID const& kUserGuid, SItemPos kItemPos, POINT3 kPos, SHORT kDir, BM::GUID const& kParentFurnitureGuid, BYTE byPosIndex);
	bool SelectArrangingFurnitureByMenu();
	void RemovePickedFurniture();
	bool HomeUnEquipRequest(PgFurniture* pkFurniture);
	void Send_PT_C_M_REQ_HOME_UNEQUIP(BM::GUID const& kGuid, SItemPos kItemPos);
	bool UnlocateArrangingFurniture(PgFurniture* pkFurniture);
	void AddFurniturebyInven(int iItemNo, BM::GUID const& kGuid);

	//패킷 전송용
	//void Net_PT_C_M_REQ_HOME_EQUIP(SItemPos const& rkItemPos, POINT3 const& rkPos, short siDir, BM::GUID const& rkParentGuid = BM::GUID::NullData(), BYTE btLinkIdx = 0);
	void Net_PT_C_M_REQ_HOME_UNEQUIP(SItemPos const& rkItemPos);
	void RecvHouse_Command(WORD const wPacketType, BM::Stream &rkPacket);

	void Send_PT_C_M_REQ_HOME_USE_HOME_STYLEITEM(SItemPos kItemPos, DWORD dwClientTime, BM::GUID const& kHomeGuid);
	bool HomeItemUseStyleItem(SItemPos const& rkItemPos);

	bool EquipExteria(int const iNo, NiPoint3 const& rkPos, float const fDir = 0.0f);//외관 아이템 장착

	void UpdateHouse(float fAccumTime,float fFrameTime);
	bool SnapArrangingFurniture(float fAccumTime);
	bool PickCell(NiPoint3& kCellPos);
private:
	PgWall			m_kWall;	//외부 아이템 장착을 위해서
	ExteriaMap		m_kExteriaMap;	//집 외관용
	PgFurniture* AddFurniture(int iFurnitureNo, NiPoint3& kPos, int iRotate, BM::GUID const& rkGuid, bool bLocate = true);
		
	NiNodePtr m_spWallRoot;
	PgPuppet* m_kNamePlate;

	bool m_bArrangeMode;
	PgFurniture* m_pkArrangingFurniture;
	PgFurniture* m_pkPickedFurniture;

	static EEquipPos const EquipPosArray[DefaultExteriaCount];
	static int const DefaultExteriaArray[DefaultExteriaCount];
};

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_HOUSE_PGHOUSE_H