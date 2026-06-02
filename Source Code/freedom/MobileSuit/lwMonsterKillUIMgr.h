#ifndef FREEDOM_DRAGONICA_CONTENTS_MONSTERKILLCOUNT_LWMONSTERKILLUIMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_MONSTERKILLCOUNT_LWMONSTERKILLUIMGR_H

#include "lwUI.h"
#include "PgUIDrawObject.h"

class PgMonsterKillUIMgr
{
public:
	PgMonsterKillUIMgr();
	~PgMonsterKillUIMgr();

	void Update(int const iNum);
	void UpdateUI(bool const bEndRewardEffect);
	void Clear();
private:
	int m_iKillNow;
	int m_iKillMax;
	int m_iRewardKillNow;
	int m_iRewardKillMax;
	int m_iRewardNo;
	bool m_bIsNowRewardEffect;
	float m_fEffectEndTime;
};

#define g_kMonKillUIMgr SINGLETON_STATIC(PgMonsterKillUIMgr)

extern void lwDrawMonsterKillUI();


class PgFormAnimatedMoveWnd : public CXUI_Wnd
{
public:
	PgFormAnimatedMoveWnd();
	~PgFormAnimatedMoveWnd();

	virtual CXUI_Wnd* VCreate()const	{ return new PgFormAnimatedMoveWnd; }

	virtual bool VDisplay();

	void StartAni();
private:
	bool m_bPlayMoveSound;
	bool m_bCanAni;
	float m_fStartAni;
};

typedef struct tagRouletteItemDiffer
{
	tagRouletteItemDiffer(int const ItemNo, CItemDefMgr const& rkItemDefMgr)
		: iItemNo(ItemNo), kItemDefMgr(rkItemDefMgr)
	{
	}

	int const iItemNo;
	CItemDefMgr const& kItemDefMgr;
} SRouletteItemDiffer;

typedef struct tagRouletteItem
{
	tagRouletteItem()
		: iItemNo(0), iCount(0), pkUVSprite(0), iImgIndex(-1)
	{
	}

	tagRouletteItem(int const ItemNo, int const Count, PgUIUVSpriteObject *UVSprite)
		: iItemNo(ItemNo), iCount(Count), pkUVSprite(UVSprite), iImgIndex(-1)
	{
	}

	bool operator == (SRouletteItemDiffer const &rkItem)
	{
		if( iItemNo == rkItem.iItemNo )
		{
			return true;
		}

		CItemDef const* pkLeftDef = rkItem.kItemDefMgr.GetDef(iItemNo);
		CItemDef const* pkRightDef = rkItem.kItemDefMgr.GetDef(rkItem.iItemNo);
		if( !pkLeftDef
		||	!pkRightDef )
		{
			return true;
		}
		return pkLeftDef->ResNo() == pkRightDef->ResNo();
	}

	int iItemNo;
	int iCount;
	PgUIUVSpriteObject *pkUVSprite;
	int iImgIndex;
} SRouletteItem;
typedef std::vector< SRouletteItem > ContRouletteItem;


class PgFormRouletteWnd : public CXUI_Wnd
{
public:
	PgFormRouletteWnd();
	~PgFormRouletteWnd();

	virtual CXUI_Wnd* VCreate()const	{ return new PgFormRouletteWnd; }
	virtual CXUI_Wnd* VClone();

	virtual bool VDisplay();

	void Clear();
	bool SetRoulette(int const iItemBagGroupNo, PgBase_Item const &rkResultItem, size_t iTempCount = 32);
	//bool SetRouletteBagItem(int const iItemNo, PgBase_Item const &rkResultItem, size_t iTempCount = 32);
	void SetStopKey();
	void SetStopKeyState();
	bool GetCompleteState();

private:
	float m_fTime;
	float m_fChangeTime;
	ContRouletteItem m_kTempRoulette;
	bool m_bResultSound;
	PgBase_Item m_kResultItem;

	SRouletteItem *m_pkCurItem;
	size_t m_iCurItem;
	bool m_bStopKey;
	bool m_bComplete;
};	

#endif // FREEDOM_DRAGONICA_CONTENTS_MONSTERKILLCOUNT_LWMONSTERKILLUIMGR_H