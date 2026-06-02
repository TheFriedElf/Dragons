#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ITEM_PGBASEITEMSET_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ITEM_PGBASEITEMSET_H
class PgBaseItemSet : public NiMemObject
{
public:
	class PgBaseItem
	{
	public:
		PgBaseItem(std::string &rkIconPath, int iClassNo = 0, int iSetNo = 0);
		~PgBaseItem();

		void AddItem(int iItemNo);
		int GetItemNo(int iPos);
		int GetNbItemNo();
		int GetSetNo();
		int GetClassNo();
		std::string &GetIconPath();

	private:
		int m_iClassNo;
		int m_iSetNo;
		std::vector<int> m_kItemNoContainer;
		std::string m_kIconPath;
	};
	typedef std::vector<PgBaseItem *> BaseItemList;

	// GenderLimit - 남자 [1], 여자 [2], 공용 [3] 이다.
	PgBaseItemSet(int iItemType, int iGenderLimit = 3, int iSetGroup = 0, int iNbExposedSlot = 3);
	~PgBaseItemSet();

	//! BaseItem List에 아이템을 추가한다.(Add Range를 넣을까?)
	void AddItem(PgBaseItem *pkItem);

	//! Index에 해당하는 BaseItem을 반환한다.
	PgBaseItemSet::PgBaseItem *GetItem(int iPos);
	PgBaseItemSet::PgBaseItem *GetItemType(int iType);
	PgBaseItemSet::PgBaseItem *GetItemSet(int iClassNo, int iSetNo);

	//! 노출되어 있는 슬롯의 개수를 설정한다.
	void SetExposedSlot(int iNbExposedSlot);

	//! 아이템의 개수를 반환한다.
	int GetSize();

	//! 아이템 타입을 반환한다.
	int GetType();

	//! 아이템 성별제한을 반환한다.
	int GetGenderLimit();

	//! 아이템 셋트 번호를 반환한다.
	int GetSetGroup();
	
	//! UI에 노출되어 있는 슬롯 중에, 원하는 슬롯의 아이템 번호를 반환한다.
	PgBaseItem *GetBaseItem(int iExposedSlot);

	//! 다음 아이템을 첫 번쨰 슬롯으로 한다.
	void NextSlot();
	
	//! 이전 아이템을 첫 번쨰 슬롯으로 한다.
	void PrevSlot();

	//!
	void SetFirstExposedSlot(int iFirstExposedSlot);

protected:
	BaseItemList m_kBaseItemList;

	int m_iSetGroup;
	int m_iFirstExposedSlot;
	int m_iItemType;
	int m_iGenderLimit;
	int m_iNbExposedSlot;
};
#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ITEM_PGBASEITEMSET_H