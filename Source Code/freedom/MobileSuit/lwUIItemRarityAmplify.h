#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMRARITYAMPLIFY_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMRARITYAMPLIFY_H
#include "PgUIModel.h"
#include "lwUIToolTip.h"

namespace lwUIItemRarityAmplify
{
	extern void RegisterWrapper(lua_State *pkState);
	extern void CallItemAmplifySelectUI();
	extern void CallItemAmplifyUI();

	extern __int64 GetAmplifyNeedMoney();
	extern void OnDisplay();
	extern void OnTick(POINT2 kPoint);
	extern void DisplaySrcIcon();
	extern void DisplayNeedItemIcon(int const iNeedIndex, POINT2 kPoint);
	extern void UseInsureItem(bool bUse);
	extern bool IsUseInsureItem();
	extern void Clear();
	extern void Start();
	extern bool Cancle();
	extern bool Check();
};

typedef enum eAmplifyItemType
{
	AIT_TARGET_ITEM = 0,
	AIT_SOUL,
	AIT_NEED_ITEM,
	AIT_INSUR_ITEM,
}EAmplifyItemType;

class PgUIItemRarityAmplify
{
public:
	PgUIItemRarityAmplify();
	~PgUIItemRarityAmplify() {}
public:
	void Clear();
	void Start();
	void Recv(BM::Stream &rkPacket);
	bool Cancle();

	__int64 GetAmplifyNeedMoney();
	void OnDisplay();
	void OnTick(POINT2 kPoint);
	void DisplaySrcIcon();
	void DisplayNeedItemIcon(int const iNeedIndex, POINT2 kPointLocation);
	void UseInsureItem(bool bUse);
	bool UseInsureItem()const;
	void CheckInsureItem(bool const bUse);

	void SetItem(EKindUIIconGroup const kType, SIconInfo const & rkInfo);
	SItemPos const& GetSrcItemPos() const { return m_kTargetItemPos; }

	bool Check();

	bool IsUseableItem(PgBase_Item const& rkItem, bool & rbGrayVisible)const;

private:
	SDefBasicOptionAmp const * GetBasicOptionAmp()const;
	void Send_ReqAmplify();
	bool SetSrcItem(const SItemPos &rkItemPos);

	size_t GetHaveSoulCount();
	int GetNeedSoulCount();

	int GetHaveAmplifyCount(int& iOutAmplifyItemNo);
	int GetNeedAmplifyCount();

	__int64 GetHaveMoney();

	int GetHaveInsureItem(int& iOutItemNo, SItemPos &rkOutItemPos);

	void UpdateExplainText();
	void InitUIModel();
	void ProcessEffect(bool bRun);

	
	void RecentResult( HRESULT hResult );
	HRESULT const RecentResult() { return m_hResult; }

	int GetEquipPos(CItemDefMgr const &rkItemDefMgr, PgBase_Item &rkItem);
	bool GetChangeEnchantInfo(CONT_ABILS& rkOutContAbils, PgBase_Item& rkPrevItem, PgBase_Item& rkResultItem, int const iLevelLimit);
	bool GetEnchantAbils(CONT_ABILS &rkOutContAbil, int const iIndex, int const iEquipPos, SEnchantInfo& rkPrevEnchantInfo, SEnchantInfo& rkResultEnchantInfo, SBasicOptionAmpKey const& kPrevAmpKey, SBasicOptionAmpKey const& kResultAmpKey);

	void NoticeShow_SuccessInfo(PgBase_Item const& kItem, CItemDef const* pDef, CONT_ABILS const& kContAbils);
	bool FindCraftOption(PgBase_Item const& rkItem, CItemDef const* pkItemDef)const;
private:
	PgBase_Item m_kTargetItem;
	SItemPos m_kTargetItemPos;

	int m_iInsureItemNo;
	SItemPos m_kInsureItemPos;

	int m_iNeedItemNo;

	PgBase_Item m_kSoulItem;
	SItemPos m_kSoulItemPos;

	bool m_bUseInsureItem;
	bool m_bProcessStart;
	float m_fStartTime;

	PgUIModel	*m_pkWndUIModel;		//회오리 이펙트용
	PgUIModel	*m_pkWndUIModel_Result;	//결과 이펙트용

	HRESULT m_hResult;
};

#define g_kItemRarityAmplify SINGLETON_STATIC(PgUIItemRarityAmplify)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMRARITYAMPLIFY_H