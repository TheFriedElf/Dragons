#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ITEM_PGITEMMAN_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ITEM_PGITEMMAN_H

class PgItemEx;

#define PG_USE_ITEM_REUSE

typedef struct tagSetItemEffectInfo
{
	std::wstring m_strAttachNode;
	std::wstring m_strEffectID;

}SItemEffectInfo;

struct SPOTParticleInfo
{// 세트 아이템 파티클 정보(공격시만 파티클 붙이기)
	std::string kAttachNode;
	std::string kAttachNodePos;
	std::string kEffectID;
	std::string kProjectileEffectID;
	std::string kEventKey;
	int iAttachPointOfTime;
	float fScale;
	bool bDetachWhenActionEnd;	//액션 종료시 파티클 제거

	SPOTParticleInfo()
		:iAttachPointOfTime(0)
		,fScale(1.0f)
		,bDetachWhenActionEnd(false)
	{}

	bool operator==(SPOTParticleInfo const& rhs) const
	{
		return (rhs.kAttachNode == kAttachNode)
			&& (rhs.kAttachNodePos == kAttachNodePos)
			&& (rhs.kEffectID == kEffectID)
			&& (rhs.kProjectileEffectID == kProjectileEffectID)
			&& (rhs.kEventKey == kEventKey)
			&& (rhs.iAttachPointOfTime == iAttachPointOfTime)
			&& (rhs.fScale == fScale)
			&& (rhs.bDetachWhenActionEnd == bDetachWhenActionEnd);
	}
};

struct SItemChangeInfo
{
	EEquipPos eEquipPos;
	int iItemNo;
	EInvType eInvType;

	SItemChangeInfo()
		:eEquipPos(EQUIP_POS_NONE)
		,iItemNo(0)
		,eInvType(IT_NONE)
	{}
	SItemChangeInfo(EEquipPos const eEquip_in, int const iItemNo_in, EInvType eInv)
		:eEquipPos(eEquip_in)
		,iItemNo(iItemNo_in)
		,eInvType(eInv)
	{}
};

typedef std::vector<SItemChangeInfo> CONT_ITEM_CHANGE_INFO;
typedef std::map<int, CONT_ITEM_CHANGE_INFO> CONT_ITEMSET_CHANGE_EQUIP_INFO;

typedef std::map<int, SPOTParticleInfo>	CONT_SETITEM_POT_PARTICLE_INFO;			// 세트 아이템 파티클 정보(공격시만 파티클 붙이기)
typedef std::map<int, SPOTParticleInfo>	CONT_POT_PARTICLE_INFO;

typedef struct tagSetItemEffectIndexInfo
{
	typedef std::vector<SItemEffectInfo> CONT_SET_EFFECT_INFO;

	CONT_SET_EFFECT_INFO m_kEffect; //SetItem에 붙어야할 이펙트들을 저장하고 있음.
}SSetItemEffectIndexInfo;

class PgItemMan
{
	struct stItemCacheInfo
	{
		stItemCacheInfo(int iItemDefNo, int iUserGender, int iClassNo, PgItemEx* pItem)
		{
			PG_ASSERT_LOG(pItem);
			this->iItemDefNo = iItemDefNo;
			this->iUserGender = iUserGender;
			this->iClassNo = iClassNo;
			this->pItem = pItem;
		};
		int iItemDefNo;
		int iUserGender;
		int iClassNo;
		PgItemEx* pItem;
	};

	typedef std::vector<stItemCacheInfo> ItemCacheContainer;
	//세트 이펙트 번호별 정보
	typedef std::unordered_map<int, int>				CONST_SET_ITEM_EFFECT_INDEX; //second 의 값은 CONT_SET_EFFECT_INDEX의 key값으로 쓰임
	typedef std::map<int, SSetItemEffectIndexInfo>	CONT_SET_EFFECT_INDEX;

public:
	PgItemMan();
	~PgItemMan();

	//! 새 아이템 인스턴스를 생성한다.
	PgItemEx *GetItem(int const iItemDefNo, int const iUserGender, int const iClassNo);
	PgItemEx *GetItem(CItemDef const *pkItemDef, int const iUserGender, int const iClassNo);
	PgItemEx* FindItemInCache(int const iItemDefNo, int const iUserGender, int const iClassNo);
	void PrepareItem(int const iItemDefNo);
	void CacheItem(PgItemEx *pkItem);
	void ClearAllCache();

	void ParseXml_SetItemEffectTable(std::wstring const &strTable1, std::wstring const &strTable2);
	SSetItemEffectIndexInfo const *GetSetItemEffectInfo(int const iSetItemNo) const;

	bool GetSetItemParticleInfo(int const iSetItemNo, SPOTParticleInfo &kResult) const;

	void ParseXML_ItemPOTParticleInfo(std::wstring const &kXmlFileName);
	bool GetItemPOTParticleInfo(int const iItemNo, SPOTParticleInfo &kResult) const;

	bool GetChangeItemInfo(int const iSetNo, CONT_ITEM_CHANGE_INFO& rkResult) const;
protected:
	Loki::Mutex m_kItemCacheLock;
	ItemCacheContainer m_kItemCacheContainer;

	CONT_SET_EFFECT_INDEX		m_kSetEffectInfo;
	CONST_SET_ITEM_EFFECT_INDEX m_kSetItemEffectInfo;

	CONT_SETITEM_POT_PARTICLE_INFO		m_kContSetParticleInfo;
	CONT_POT_PARTICLE_INFO				m_kContItemParticleInfo;

	CONT_ITEMSET_CHANGE_EQUIP_INFO m_kContChangeItem;
};

#define g_kItemMan SINGLETON_STATIC(PgItemMan)

#endif //FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ITEM_PGITEMMAN_H