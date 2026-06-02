#ifndef FREEDOM_DRAGONICA_CONTENTS_INVENTORY_PGINVENTORYUI_H
#define FREEDOM_DRAGONICA_CONTENTS_INVENTORY_PGINVENTORYUI_H

#include "Variant/Inventory.H"

typedef enum eItemPosDiff
{
	IPD_NONE      = 0,           //변동 없음.
	IPD_DECREASE  = 1,           //갯수가 줄어들었다.(같은 종류 아이템)
	IPD_INCREASE  = 2,           //갯수가 늘어났다.  (같은 종류 아이템)
	IPD_EMPTY     = 3,           //없어졌다.(빈 공간)
	IPD_NEW       = 4,           //새로 생겼다.
	IPD_CHANGE    = 5,           //다른 아이템으로 바뀌었다.

	IPD_ERROR     = 100,         //다른 아이템으로 바뀌었다.
}EITEM_POS_DIFF_RET;

class PgInventoryUI
{
public:
	PgInventoryUI();
	~PgInventoryUI();

	void Init(PgInventory* m_kOrgInv);
	void Clear();
	void ModifyCompInvType(EInvType const eInvType, PgInventory* m_kOrgInv);             //비교인벤 동기화(인벤 타입 별)
	EITEM_POS_DIFF_RET CheckDiff(SItemPos const &rPos, PgBase_Item const& kOrgItem);  //위치에 아이템 비교
	bool CheckDiff_InvType(EInvType const eInvType);
	
protected:

private:
	PgInventory m_kCompInv;		//비교하기 위한 인벤토리
};
#define	g_kInvUIMgr	SINGLETON_STATIC(PgInventoryUI)

#endif // FREEDOM_DRAGONICA_CONTENTS_INVENTORY_PGINVENTORYUI_H