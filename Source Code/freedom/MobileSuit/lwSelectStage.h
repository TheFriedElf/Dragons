#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWSELECTSTAGE_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWSELECTSTAGE_H

#include "lwBaseItemSet.h"
#include "lwActor.h"
#include "lwUI.h"

int const DEF_STAR_HARF_LEN = 215;
int const DEF_CHAR_MODEL_LEN = 330;

class PgSelectStage;

class lwSelectStage
{
public:
	//! 생성자
	lwSelectStage(PgSelectStage *pkSelectStage);

	//! Wrapper Class로 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//! 베이스 아이템 셋트를 추가한다.
	lwBaseItemSet AddBaseItemSet(int iItemPos, int iItemCnt, int iExposedSlot);

	void AddBaseItemSetFromTableData();	

	//! 새로운 액터를 만든다.
	lwActor AddNewActor(int const iClass, int const iGender, char const *pcSpawnLoc);

	//! GUID에 해당하는 스폰 슬롯을 반환한다.
	int GetSpawnSlot(lwGUID kGuid);

	//! SpawnSlot을 초기화한다.
	void ResetSpawnSlot();

	//! Spwan Slot을 지운다.
	void RemoveSpawnSlot(lwGUID kGuid);

	//! SpawnSlot을 주면 그에 해당하는 Guid을 준다.
	lwGUID GetSpawnActor(int iSpawnSlot);

	//! 등록된 스폰 슬롯 카운트를 얻어온다.
	int GetSpawnSlotCount();
	
	//! 삭제 대기 등록된 스폰 슬롯 카운트를 얻어온다.
	int GetDeleteSlotCount();

	//! 선택한 캐릭터의 Guid를 설정한다.
	void SetSelectedActor(int iSelect);

	//! 선택한 캐릭터 Guid를 반환한다.
	lwGUID GetSelectedActor();

	//! 새로 만든 캐릭터 Guid를 설정한다.
	void SetNewActor(lwActor kActor);


	//! 새로 만든 캐릭터 Guid를 반환한다.
	lwActor GetNewActor();

	//! 새로 만든 캐릭터를 초기화한다.
	void lwSelectStage::ClearNewActor();

	//! 타입과 위치에 해당하는 아이템 번호를 반환한다.
	int GetBaseItemNo(int iType, int iExposedPos, int iSetItemPos);

	//! Type에 해당하는 ItemSet를 준다.
	lwBaseItemSet GetBaseItemSet(int iType);
	lwBaseItemSet GetBaseItemSet_Detail(int const iType, int const iSetNo, int const iGender);

	//! 아이템을 착용시킨다.
	int EquipBaseItem(int iType, int iExposedSlot);
	int EquipBaseItemBySeq(int iType, int iSeq);
	bool SetFiveElement(int Element);
	void SetDefaultFiveElement();

	//! 아이템 셋트를 착용시킨다.
	void EquipBaseItemSet(int iClass, int iSet);

	void EquipBaseItemSetByPos(int iSet, int iExposedPos);

	//! 타입과 위치에 해당하는 아이템의 아이콘 패스를 반환한다.
	char const *GetBaseItemIconPath(int iType, int iExposedPos);

	//! SetNo와 위치를 주면 셋트 아이템의 아이콘 패스를 반환한다.
	char const *GetBaseItemSetIconPath(int iSetNo, int iExposedPos);

	//! SetNo와 위치를 주면 셋트 아이템의 아이콘 패스를 반환한다.
	char const *GetBaseItemSetIconPath(int iType, int iSetNo, int iGender, int iExposedPos);

	void SetFirstExposedSlot(int iType, int iGender, int iFirstExposedSlot);

	//! 지정한 타입의 기본 슬롯을 하나 증가시킨다.
	void NextSlot(int iType, bool bCheckGender);

	//! 지정한 타입의 기본 슬롯을 하나 감소시킨다.
	void PrevSlot(int iType, bool bCheckGender);

	//! 지정한 타입의 기본 슬롯을 하나 증가시킨다.
	void NextSetSlot(int iSet);

	//! 지정한 타입의 기본 슬롯을 하나 감소시킨다.
	void PrevSetSlot(int iSet);

	//! 
	void OnCallCharList();
	void OnCallDeleteWait();

	void OnClickCombineCharacter(lwUIWnd UISelf);
	void OnInputCombineChangeName(lwUIWnd UISelf);
	void OnClickCombineOK(lwUIWnd UISelf);
	void OnClickDeleteCharacter(lwUIWnd UISelf);
	bool CheckNameConflict(lwGUID kGuid);

	void InitElementSelecter(lwUIWnd UISelf);
	void UpdateElement(lwUIWnd UISelf);

	//
	void CallSelectStyle_Hair(lwUIWnd UISelf);
	void CallSelectStyle_Color(lwUIWnd UISelf);
	void CallSelectStyle_Face(lwUIWnd UISelf);
	//

	//! 형변환 Operator
	PgSelectStage *operator()();

	bool Send_PT_C_S_REQ_CREATE_CHARACTER();
	HRESULT GetCraeteUserName(std::wstring &kStrName);
	void OnCardClick(int iSelect);
	void OnCreateDefault();

	typedef std::map< int, float > kElementCont;

	void SetElemSlot(int Slot);
	void InitRenderModel(char const* szRenderModelName, lwUIWnd UISelf, bool bUpdate, bool bOrtho = false);
	void DrawRenderModel(char const* szRenderModelName, lwUIWnd UISelf);
	void AddActor(char const* szRenderModelName, char const* szActorName);
	int GetFiveElement();
	void GetElemColor();
	void GetRealmName(lwUIWnd UISelf);
	int LeafAnimation(lwUIWnd UIParent);
	void SetLeafState(int State);
	int GetLeafState();
	bool SetLeafVisible(XUI::CXUI_Wnd*& pWnd, bool bIsVisible);
	
	// 캐릭터 삭제 UI를 호출
	void CallDeleteCharacterUI();

	// 캐릭터 삭제가 가능하다면 캐릭터를 삭제
	void DeleteCharacterIfAble(lwUIWnd UISelf);
	void CallDeleteCharacterIfAble(lwUIWnd UISelf);

	// edit 박스에 입력된 문장이 있어 활성화 되었는지 확인
	bool GetIsActiveDeleteConfirmEditBox();

	// edit 박스에 캐릭터 삭제에 적당한 문장이 입력되었는가 확인
	bool GetIsSuitableInputForDeleteCharacter();
	
	// 서비스 국가에 맞는 캐릭터 이름 수정
	bool ModifyNameForNationCode(std::wstring& kStrName);

	void SetServerInfo();

	void SetCharInfo();

	void ClearTestParts();

	int GetSelectedSlot();

	int GetEnableExtendSlot();
	void ReqEnableExtendSlot();
	bool IsCreateNewRace() const;

	lwPoint2 GetActorPosToScreen(int const iSlot);
protected:
	PgSelectStage *m_pkSelectStage;
	kElementCont	m_kElementCont;
	kElementCont	m_kStartPosCont;
	NiAudioSource*	m_asEffect;
	int	m_iSetElem;
	float m_fStartTime;
	float m_fEndTime;
	float m_fStartRadian;
	float m_fEffectTime;
	bool m_bIsEffect;
	bool m_bIsChangeStar;
	float m_fAccumTime;

	// 나뭇잎
	typedef enum
	{
		LAS_PADEIN = 0,
		LAS_CHANGE,
		LAS_PADEOUT,
		LAS_CLOSE,
	}ELeafAniState;

	float m_fLeafStartTime;
	int m_iLeafCount;
	ELeafAniState m_eLeafState;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWSELECTSTAGE_H