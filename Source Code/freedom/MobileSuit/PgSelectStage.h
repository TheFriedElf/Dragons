#ifndef FREEDOM_DRAGONICA_SCENE_WORLD_PGSELECTSTAGE_H
#define FREEDOM_DRAGONICA_SCENE_WORLD_PGSELECTSTAGE_H

class PgBaseItemSet;
class PgActor;
class PgWorld;

typedef std::map< int, TBL_DEF_CHARACTER_BASEWEAR >	CONTMAP_DEFCHARACTER_BASEWEAR;// first key : TBL_DEF_CHARACTER_BASEWEAR::iWearNo

typedef struct tagSelectCharKey
{
	tagSelectCharKey(BM::GUID const &kCharacterGuid, BM::PgPackedTime const &kBirthDate, short const nLevel, short const nSlot )
	{
		CharacterGuid(kCharacterGuid);
		BirthDate(kBirthDate);
		Level(nLevel);
		Slot(nSlot);
	}

	CLASS_DECLARATION_S(BM::PgPackedTime, BirthDate);// pkPlayer->BirthDate();
	CLASS_DECLARATION_S(BM::GUID, CharacterGuid);
	CLASS_DECLARATION_S(short, Level);
	CLASS_DECLARATION_S(int, Slot);
	
	bool operator < (tagSelectCharKey const &rhs)const
	{
		if(Slot() < rhs.Slot()){ return true; }
		if(rhs.Slot() < Slot()){ return false; }

		if(BirthDate() < rhs.BirthDate()){ return true; }
		if(rhs.BirthDate() < BirthDate() ){ return false; }

		if(CharacterGuid() < rhs.CharacterGuid()){ return true; }
		if(rhs.CharacterGuid() < CharacterGuid()){ return false; }

		return false;
	}
}SSelectCharKey;

typedef std::map<int, SSelectCharKey > CONT_SPAWN_SLOT_MAP;

class PgSelectStage
{
public:
	typedef std::vector< PgBaseItemSet* > CONT_BASE_ITEM;

	PgSelectStage();
	~PgSelectStage();

	void Terminate();
	void Clear();
	
	PgBaseItemSet *AddBaseItemSet(int iType, int iSetCnt = 5, int iNbExposedSlot = 3);//! BaesItem Set를 추가한다.
	void AddBaseItemSetFromTableData();//! Table 데이타로 부터 BaseItem Set를 추가한다.

	//! Table 데이타로 부터 BaseItem을 Default아이템으로 입힌다.
	void EquipBaseItemSet(int iClass, int iSet);
	void EquipBaseItemSet(int iClass, int iSet, PgActor* pkActor, bool bDefault = true);
	void EquipBaseItemFullSet(PgActor* pkActor, PLAYER_ABIL& rkPlayerAbil);
	void PrepareBaseItemFullSet();

	//! 단일 아이템을 Default아이템으로 입힌다.
	int EquipBaseItem(int const iType, int const iSetNo, int iExposedPos);
	int EquipBaseItemBySeq(int iType, int iSetNo, int iSeq);
	int EquipBaseItemBySeq(int iType, int iSetNo, int iSeq, PgActor* pkActor, bool bDefault = true, int iFindThisItemFirst = 0);
	
	int EquipBaseItemSetByPos(int iSet, int iExposedPos);

	HRESULT SetFiveElement(int Element);
	HRESULT SetDefaultFiveElement();
	int GetFiveElement();

	PgBaseItemSet *GetBaseItemSetByPos(int iPos);//! BaseItem Set을 가져온다.
	PgBaseItemSet *GetBaseItemSet(int iType, int iGender = 3);//! BaseItem Set을 가져온다.
	PgBaseItemSet *GetBaseItemSets(int iSetGroup);//! Set아이템의 BaseItemSet들을 가져온다.
	PgBaseItemSet *GetBaseItemSets(int iType, int iSetGroup, int iGender);//! WearType, SetNo, Gender에 맞는 BaseItemSet들을 가져온다.

	int GetBaseItemSetNo(int const iClassNo);

	void ResetSpawnSlot();//! SpawnSlot을 리셋한다.
	void RemoveSpawnSlot(BM::GUID const &rkGuid);//! SpawnSlot에 등록된 Guid를 지운다.
	int PgSelectStage::GetSpawnSlot(BM::GUID const &rkGuid)const;//! Guid를 주면 그에 해당하는 Spawn Slot을 준다.
	NiPoint3 GetSpawnPoint(int const iSlot);//! Guid를 주면 그에 해당하는 Spwan Point이름을 준다.
	BM::GUID GetSpawnActor(int iSpawnSlot);//! 선택가능한 캐릭터 컨테이너중에서 SpawnSlot을 주면 그에 해당하는 Guid을 준다.
	BM::GUID GetDeleteActor(int iSpawnSlot);//! 삭제 대기중인 캐릭터 컨테이너중에서 SpawnSlot을 주면 그에 해당하는 Guid을 준다.
	bool AddToSlot(SSelectCharKey const &kKey, bool const bDeleteWait);//! FirstEmptySlot에 캐릭터 Guid를 등록!
	PgActor *AddNewActor(int const iClass, int const iGender, char const *pcSpawnLoc);//! 새로운 액터를 추가한다.

	bool SelectDefaultCharacter();//처음 선택 되어있을 캐릭 선택
	void SetSelectedActor(int iSelect);//! 선택한 캐릭터를 저장한다.

	BM::GUID &GetSelectedActor();//! 선택한 캐릭터를 반환한다.
	void SetNewActor(PgActor *pkActor);//! 새로 만든 캐릭터 Guid를 설정한다.
	PgActor *GetNewActor();//! 새로 만든 캐릭터 Guid를 반환한다.
	int GetSpawnSlotCount();//! 등록된 스폰 슬롯 카운트를 얻어온다.
	int GetDeleteSlotCount();//! 삭제 대기 등록된 스폰 슬롯 카운트를 얻어온다.
	void OnCallCharList();
	void OnCallDeleteWait();

	void OnClickCombineCharacter(XUI::CXUI_Wnd* pSelf);
	void OnInputCombineChangeName(XUI::CXUI_Wnd* pSelf);
	void OnClickCombineOK(XUI::CXUI_Wnd* pSelf);
	void OnClickDeleteCharacter(XUI::CXUI_Wnd* pSelf);
	bool OnDeleteWaitCharacter(BM::GUID const& kGuid);
	void SetDeleteCharacterInfo(BM::GUID const& kGuid);
	bool MoveSlotDelToSpawn(BM::GUID const& kGuid);
	bool CheckDeleteWaitSlot(BM::GUID const& kGuid);
	bool CheckNameConflict(BM::GUID const& kGuid);

	void SetCharInfoToCard(XUI::CXUI_Wnd* pWnd, BM::GUID const & rkGuid, wchar_t const* kName = NULL);

	int const GetAllCharacterCount() { return m_kContSpawnSlotMap.size(); };
	int GetNotNewbiePlayerCount() const;

	bool SelectCharactorByGuid(BM::GUID const &rkGuid);
	void SetCharInfo();
	BM::PgPackedTime GetLastLogOutTime();

	int GetSelectedSlot();

	bool IsCreateNewRace() const;

	bool DrawSpawnCharacter(BM::GUID const& kGuid);
protected:	
	
	CONT_BASE_ITEM m_kContBaseItem;
	CONT_SPAWN_SLOT_MAP	m_kContSpawnSlotMap;
	CONT_SPAWN_SLOT_MAP m_kContDeleteSlotMap;

	PgActor *m_pkNewActor;
	BM::GUID m_kSelectedActorGuid;

	int m_iSelectUISelectedSlot;
	int m_iSelectUIDrawStartSlot;

	CLASS_DECLARATION_S(int, ExtendSlot);
};

#define g_kSelectStage SINGLETON_CUSTOM(PgSelectStage, Loki::CreateUsingNew)

#endif // FREEDOM_DRAGONICA_SCENE_WORLD_PGSELECTSTAGE_H