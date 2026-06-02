#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUITREASURECHEST_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUITREASURECHEST_H

#include "PgScripting.h"
#include "lwPacket.h"

namespace lwTreasureChest
{
	typedef enum tagANITYPE
	{
		TC_ANI_IDLE = 0,
		TC_ANI_SUCCESSED,
		TC_ANI_FAILED,
	}EANITYPE;

	void RegisterWrapper(lua_State *pkState);
	bool Init(PgBase_Item const* pkItem);
	void SendReqOpenChest(void);

	void InitChestCamera(const char* szRenderModelName, const char* szRenderActorName);
	float BeginOpenAnimation(void);
	float BeginFailAnimation(void);
	void ResetChest(lwUIWnd kTopWnd);
	bool TryOpen(PgBase_Item const* pkItem, SItemPos const & rkItemInvPos);
	bool InsertKey(PgBase_Item const* pkItem, SItemPos const & rkItemInvPos);
	void RemoveKey(void);
	void SetRewardItem(int const iItemNo);
	void DrawIcon(lwUIWnd kIconWnd);
	void SetChestAni(EANITYPE eAniType);
	float OpenChest(void);
	bool IsAvailableChest(void);

}


class PgTreasureChestMgr
{
public:
	PgTreasureChestMgr();
	~PgTreasureChestMgr();

	void Clear(void);
	bool RegisterChest(int iItemNo, const SItemPos& kChestPos);
	bool RegisterKey(int iItemNo, const SItemPos& kKeyPos);
	void UnregisterKey(void);
	bool GetChestAndKeyItem(PgBase_Item& kChest, PgBase_Item& kKey);
	bool TestMatching(void);
	void Send_PT_M_C_REQ_TREASURE_CHEST(void);
	void Recv_PT_M_C_ANS_TREASURE_CHEST(BM::Stream* pkPacket);
	void Send_PT_C_M_REQ_TREASURE_CHEST_INFO(void);
	void Recv_PT_M_C_ANS_TREASURE_CHEST_INFO(BM::Stream* pkPacket);

protected:
	SItemPos m_kChestPos, m_kKeyPos;
	int m_iChestItemNo, m_iKeyItemNo;
	CONT_TREASURE_CHEST m_kContTableData;
};

namespace TreasureChestUtil
{
	PgInventory* GetInventory(void);
}

#define g_kTreasureChestMgr SINGLETON_STATIC(PgTreasureChestMgr)

#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUITREASURECHEST_H