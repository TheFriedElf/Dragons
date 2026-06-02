#include "stdafx.h"
#include "PgSelectStage.h"
#include "PgWorld.h"
#include "PgActor.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgBaseItemSet.h"
#include "PgItemMan.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "lwPilot.h"
#include "lwActor.h"
#include "Variant/PgTotalObjectMgr.h"
#include "Variant/PgPlayer.h"
#include "Variant/PgControlDefMgr.h"
#include "lwUI.h"
#include "PgNifMan.h"
#include "PgUIScene.h"
#include "PgSoundMan.h"
#include "lwUIQuest.h"
#include "PgCommandMgr.h"
#include "Lohengrin/PgRealmManager.h"

int const BASE_ITEM_SET_HUMAN = 10000;
int const BASE_ITEM_SET_DRAGON = 20000;

PgSelectStage::PgSelectStage() 
:	m_pkNewActor(NULL)
,	m_iSelectUISelectedSlot(0)
,	m_iSelectUIDrawStartSlot(0)

{
	m_kContBaseItem.clear();
	m_kContSpawnSlotMap.clear();
	m_kContDeleteSlotMap.clear();

	m_kSelectedActorGuid = BM::GUID::NullData();
}

PgSelectStage::~PgSelectStage()
{
	Terminate();
}

void PgSelectStage::Terminate()
{
	CONT_BASE_ITEM::iterator iter = m_kContBaseItem.begin();
	while (iter != m_kContBaseItem.end())
	{
		SAFE_DELETE_NI(*iter);
		++iter;
	}
	m_kContBaseItem.clear();
	m_kContSpawnSlotMap.clear();
	m_kContDeleteSlotMap.clear();
	m_pkNewActor = NULL;
	m_kSelectedActorGuid = BM::GUID::NullData();
}

void PgSelectStage::Clear()
{
	m_kContSpawnSlotMap.clear();
	m_kContDeleteSlotMap.clear();
	m_pkNewActor = NULL;
	m_kSelectedActorGuid = BM::GUID::NullData();
}

void PgSelectStage::SetSelectedActor(int iSelect)
{
	m_iSelectUISelectedSlot = iSelect;
	OnCallCharList();
	lua_tinker::call<void>("OnSelectUI");
}

BM::GUID &PgSelectStage::GetSelectedActor()
{
	return m_kSelectedActorGuid;
}

void PgSelectStage::SetNewActor(PgActor *pkActor)
{
	PG_ASSERT_LOG(pkActor);
	m_pkNewActor = pkActor;
}

PgActor *PgSelectStage::GetNewActor()
{
	return m_pkNewActor;
}

int PgSelectStage::EquipBaseItem(int const iType, int const iSetNo, int iExposedPos)
{
	if(!m_pkNewActor || !m_pkNewActor->GetPilot()) 
	{
		return 0;
	}

	int iGender = m_pkNewActor->GetPilot()->GetAbil(AT_GENDER);
	PgBaseItemSet *pkBaseItemSet = NULL;
	if( 0 == iType)
	{//헤어컬러는 Gender = 3;
		iGender = 3;
	}
	pkBaseItemSet = GetBaseItemSets(iType, iSetNo, iGender);
	if(!pkBaseItemSet) { return 0; }

	PgBaseItemSet::PgBaseItem *pkItem = pkBaseItemSet->GetBaseItem(iExposedPos);
	if(!pkItem) { return 0; }

	int iItemNo = pkItem->GetItemNo(0);
	if(!iItemNo) { return 0; }

	if(iType == 0)
	{
		// item type이 0이면 머리 색깔이다.
		m_pkNewActor->SetItemColor(EQUIP_LIMIT_HAIR, iItemNo);
	}
	else
	{
		m_pkNewActor->AddEquipItem(iItemNo, true, PgItemEx::LOAD_TYPE_INSTANT);
	}

	return iItemNo;
}

int PgSelectStage::EquipBaseItemSetByPos(int iSet, int iExposedPos)
{
	if(!m_pkNewActor || !m_pkNewActor->GetPilot()) 
	{
		return 0;
	}

	int iGender = m_pkNewActor->GetPilot()->GetAbil(AT_GENDER);

	PgBaseItemSet *pkBaseItemSet = GetBaseItemSets(iSet);
	if(!pkBaseItemSet) { return 0; }

	PgBaseItemSet::PgBaseItem *pkItem = pkBaseItemSet->GetBaseItem(iExposedPos);
	if(!pkItem) { return 0; }

	int iItemNo = pkItem->GetItemNo(0);
	if(!iItemNo) { return 0; }

	m_pkNewActor->AddEquipItem(iItemNo, true, PgItemEx::LOAD_TYPE_INSTANT);

	return iItemNo;
}

int PgSelectStage::EquipBaseItemBySeq(int const  iType, int const  iSetNo, int const  iSeq)
{
	return EquipBaseItemBySeq(iType, iSetNo, iSeq, m_pkNewActor);
}

int PgSelectStage::EquipBaseItemBySeq(int const  iType, int const  iSetNo, int const  iSeq, PgActor* pkActor, bool bDefault, int iFindThisItemFirst)
{
	if(!pkActor || !pkActor->GetPilot()) 
	{
		return 0;
	}

	int const iClass = pkActor->GetPilot()->GetBaseClassID();
	if (iClass < 0)
	{
		return 0;
	}

	int iGender = pkActor->GetPilot()->GetAbil(AT_GENDER);
	if( 0 == iType)
	{//머리색깔은 성별공용
		iGender = 3;
	}
	int iItemNo = 0;
	if ((iType != 0 && (iFindThisItemFirst == 0 || g_kItemMan.FindItemInCache(iFindThisItemFirst, iGender, iClass) == NULL)) ||
		(iType == 0 && iFindThisItemFirst == 0))
	{
		PgBaseItemSet *pkBaseItemSet = GetBaseItemSets(iType, iSetNo, iGender);
		if(!pkBaseItemSet) { return 0; }

		PgBaseItemSet::PgBaseItem *pkItem = pkBaseItemSet->GetItem(iSeq);
		if(!pkItem) { return 0; }

		iItemNo = pkItem->GetItemNo(0);
	}
	else
	{
		iItemNo = iFindThisItemFirst;
		bDefault = true;
	}

	if(!iItemNo) { return 0; }

	if(iType == 0)
	{
		// item type이 0이면 머리 색깔이다.
		pkActor->SetItemColor(EQUIP_LIMIT_HAIR, iItemNo, bDefault);
	}
	else
	{
		pkActor->AddEquipItem(iItemNo, bDefault, PgItemEx::LOAD_TYPE_INSTANT);
	}
	return iItemNo;
}
HRESULT PgSelectStage::SetFiveElement(int Element)
{
	PgActor* pkActor = m_pkNewActor;
	if(!pkActor || !pkActor->GetPilot()) 
	{
		return 0;
	}

	pkActor->GetPilot()->SetAbil(AT_FIVE_ELEMENT_TYPE_AT_BODY, Element);

	return S_OK;
}

int PgSelectStage::GetFiveElement()
{
	PgActor* pkActor = m_pkNewActor;
	if(!pkActor || !pkActor->GetPilot()) 
	{
		return 0;
	}

	return pkActor->GetPilot()->GetAbil(AT_FIVE_ELEMENT_TYPE_AT_BODY);
}

HRESULT PgSelectStage::SetDefaultFiveElement()
{
	PgActor* pkActor = m_pkNewActor;
	if(!pkActor || !pkActor->GetPilot()) 
	{
		return 0;
	}

	SYSTEMTIME st = {0, };
	::GetLocalTime(&st);

	int const defaultElement = st.wDay % 5 + 1;

	pkActor->GetPilot()->SetAbil(AT_FIVE_ELEMENT_TYPE_AT_BODY, defaultElement);

	return S_OK;
}

void PgSelectStage::EquipBaseItemSet(int iClass, int iSet)
{
	EquipBaseItemSet(iClass, iSet, m_pkNewActor);
}

void PgSelectStage::EquipBaseItemSet(int iClass, int iSet, PgActor* pkActor, bool bDefault)
{
	if (pkActor == NULL)
	{
		return;
	}

	PgBaseItemSet *pkBaseItemSet = GetBaseItemSets(iSet);
	if(!pkBaseItemSet)
	{
		return;
	}
	// 상의, 하의, 장갑, 신발, [기본 무기]를 입힌다.
	PgBaseItemSet::PgBaseItem *pkItems = pkBaseItemSet->GetItemSet(iClass, iSet);
	if(!pkItems)
	{
		return;
	}

	int const iNbItems = pkItems->GetNbItemNo();
	for(int iIndex = 0; iIndex < iNbItems; ++iIndex)
	{
		pkActor->AddEquipItem(pkItems->GetItemNo(iIndex), bDefault, PgItemEx::LOAD_TYPE_INSTANT);
	}
}

// rkPlayerAbil에 있는 것이 현재 ItemCache에 들어있다면 입고, 그게 아니라면 Default 아이템을 입는다.
void PgSelectStage::EquipBaseItemFullSet(PgActor* pkActor, PLAYER_ABIL& rkPlayerAbil)
{
	return;
	if (pkActor == NULL || pkActor->GetPilot() == NULL)
	{
		return;
	}

	if (pkActor->EquipDefaultItem() == true)
		return;

	int iClass = pkActor->GetPilot()->GetBaseClassID();
	if (iClass < 0)
	{
		return;
	}
	int iGender = pkActor->GetPilot()->GetAbil(AT_GENDER);

	PgBaseItemSet *pkBaseItemSet = GetBaseItemSets(true);
	if(!pkBaseItemSet)
	{
		return;
	}

	PgBaseItemSet::PgBaseItem *pkItems = pkBaseItemSet->GetItemSet(iClass, 1);
	if(!pkItems)
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	int const iSetNo = GetBaseItemSetNo(iClass);
	EquipBaseItemBySeq(4, iSetNo, 1, pkActor, true, rkPlayerAbil.iFace); // face
	EquipBaseItemBySeq(2, iSetNo, 1, pkActor, true, rkPlayerAbil.iHairStyle); // hair
	EquipBaseItemBySeq(0, iSetNo, 1, pkActor, true, rkPlayerAbil.iHairColor); // hair color	

	int iNbItems = pkItems->GetNbItemNo();
	for(int iIndex = 0; iIndex < iNbItems; ++iIndex)
	{
		int iItemNo = pkItems->GetItemNo(iIndex);
		int iFindFirstItemNo = 0;
		if (iItemNo == 0)
			continue;

		CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);
		if (pkItemDef)
		{
			switch(pkItemDef->GetAbil(AT_EQUIP_LIMIT))
			{
			case EQUIP_LIMIT_SHIRTS:
				iFindFirstItemNo = rkPlayerAbil.iJacket;
				break;
			case EQUIP_LIMIT_PANTS:
				iFindFirstItemNo = rkPlayerAbil.iPants;
				break;
			case EQUIP_LIMIT_BOOTS:
				iFindFirstItemNo = rkPlayerAbil.iShoes;
				break;
			case EQUIP_LIMIT_GLOVE:
				iFindFirstItemNo = rkPlayerAbil.iGloves;
				break;
			default:
				break;
			}
		}
		else
		{	// 없으면 안된다;
		}

		bool bDefault = false;

		if (iFindFirstItemNo != 0 && g_kItemMan.FindItemInCache(iFindFirstItemNo, iGender, iClass) != NULL)
		{
			iItemNo = iFindFirstItemNo;
			bDefault = true;
		}
		pkActor->AddEquipItem(iItemNo, bDefault, PgItemEx::LOAD_TYPE_INSTANT);
	}
	pkActor->EquipDefaultItem(true);
}

void PgSelectStage::PrepareBaseItemFullSet()
{
	PgBaseItemSet *pkBaseItemSet = GetBaseItemSets(true);
	if(!pkBaseItemSet)
	{
		return;
	}

	int const iSetCount = pkBaseItemSet->GetSize();

	for (int i = 0; i < iSetCount; ++i)
	{
		PgBaseItemSet::PgBaseItem* pkItems = pkBaseItemSet->GetItem(i);

		if (pkItems)
		{
			int const iItemCount = pkItems->GetNbItemNo();
			for (int j = 0; j < iItemCount; ++j)
			{
				g_kItemMan.PrepareItem(pkItems->GetItemNo(j));
			}
		}
	}
}

void PgSelectStage::AddBaseItemSetFromTableData()
{
	{//테이블 초기화(괄호 빼지 말 것.)
		CONT_BASE_ITEM kTemp;
		m_kContBaseItem.swap(kTemp);
	}
	const CONT_DEFCHARACTER_BASEWEAR* pkContBaseWear = NULL;
	g_kTblDataMgr.GetContDef(pkContBaseWear);

	GET_DEF(CItemDefMgr, kItemDefMgr);
	for(CONT_DEFCHARACTER_BASEWEAR::const_iterator itr = pkContBaseWear->begin();
		itr != pkContBaseWear->end();
		++itr)
	{
		int iItemNo = itr->first;
		const TBL_DEF_CHARACTER_BASEWEAR *pkBaseWear = &itr->second;
		std::wstring kIconPath(pkBaseWear->strIconPath);
		CItemDef const *pkItemDef = kItemDefMgr.GetDef(iItemNo);

		int iItemType = pkBaseWear->iWearType;
		int const iClassNo = pkBaseWear->iClassNo;
		int iGenderLimit = 3;

		if(pkItemDef)
		{
			iItemType = pkItemDef->GetAbil(AT_EQUIP_LIMIT);
			iGenderLimit = pkItemDef->GetAbil(AT_GENDERLIMIT);
		}
		
		//_PgOutputDebugString("ItemNo : %d\t ItemType : %d\t ClassLimit : %d\t GenderLimit : %d\n", iItemNo, iItemType, iClassNo, iGenderLimit);

		PgBaseItemSet *pkBaseItemSet = 0;
		PgBaseItemSet::PgBaseItem *pkBaseItem = 0;
		if(pkBaseWear->iClassNo != 0)
		{
			// Set No가 있으면, 셋트 아이템이므로 SetNo와 Class No로 BaseItem을 찾아서 추가한다.
			pkBaseItemSet = GetBaseItemSets(pkBaseWear->iSetNo);
			if(!pkBaseItemSet)
			{
				pkBaseItemSet = NiNew PgBaseItemSet(iItemType, iGenderLimit, pkBaseWear->iSetNo);
				m_kContBaseItem.push_back(pkBaseItemSet);
			}

			pkBaseItem = pkBaseItemSet->GetItemSet(iClassNo, pkBaseWear->iSetNo);
		}
		else
		{
			pkBaseItemSet = GetBaseItemSets(iItemType, pkBaseWear->iSetNo, iGenderLimit);
			if(!pkBaseItemSet)
			{
				pkBaseItemSet = NiNew PgBaseItemSet(iItemType, iGenderLimit, pkBaseWear->iSetNo);
				m_kContBaseItem.push_back(pkBaseItemSet);
			}
			
			pkBaseItem = new PgBaseItemSet::PgBaseItem(std::string(MB(kIconPath)), iClassNo, pkBaseWear->iSetNo);
			pkBaseItemSet->AddItem(pkBaseItem);

		}

		if(!pkBaseItem)
		{
			// SetItem은 모두 Icon Path를 가지고 있어야 한다.
			pkBaseItem = new PgBaseItemSet::PgBaseItem(std::string(MB(kIconPath)), iClassNo, pkBaseWear->iSetNo);
			pkBaseItemSet->AddItem(pkBaseItem);
		}
		
		pkBaseItem->AddItem(iItemNo);
	}
}

PgBaseItemSet *PgSelectStage::AddBaseItemSet(int iType, int iSetCnt, int iNbExposedSlot)
{
	PgBaseItemSet *pkBaseItemSet = NiNew PgBaseItemSet(iType, iSetCnt, iNbExposedSlot);
	m_kContBaseItem.push_back(pkBaseItemSet);
	return pkBaseItemSet;
}

PgBaseItemSet *PgSelectStage::GetBaseItemSetByPos(int iPos)
{
	if((int)m_kContBaseItem.size() <= iPos)
	{
		return NULL;
	}
	return m_kContBaseItem.at(iPos);
}

PgBaseItemSet *PgSelectStage::GetBaseItemSet(int iType, int iGender)
{
	CONT_BASE_ITEM::iterator itr = m_kContBaseItem.begin();
	while(itr != m_kContBaseItem.end())
	{
		PgBaseItemSet *pkItemSet = *itr;
		if(pkItemSet->GetType() == iType)
		{
			// 공용 아이템이 아니면 Gender를 체크하자.
			if(iGender == 3)
			{
				return pkItemSet;
			}
			else if((pkItemSet->GetGenderLimit() & iGender) == iGender)
			{
				return pkItemSet;
			}
		}
		++itr;
	}

	return 0;
}

PgBaseItemSet *PgSelectStage::GetBaseItemSets(int iSetGroup)
{
	// 현재는 Set로 가지는 ItemSet는 단 하나 뿐이다. (상의, 하의, 장갑, 신발 세트)
	CONT_BASE_ITEM::iterator itr = m_kContBaseItem.begin();
	while(itr != m_kContBaseItem.end())
	{
		if((*itr)->GetSetGroup() == iSetGroup)
		{
			return *itr;
		}
		++itr;
	}

	return 0;
}

PgBaseItemSet *PgSelectStage::GetBaseItemSets(int iType, int iSetGroup, int iGender)
{
	// 현재는 Set로 가지는 ItemSet는 단 하나 뿐이다. (상의, 하의, 장갑, 신발 세트)
	CONT_BASE_ITEM::iterator itr = m_kContBaseItem.begin();
	while(itr != m_kContBaseItem.end())
	{
		if((*itr)->GetType() == iType
		&&	(*itr)->GetSetGroup() == iSetGroup
		&& (*itr)->GetGenderLimit() == iGender )
		{
			return *itr;
		}
		++itr;
	}

	return 0;
}

int PgSelectStage::GetSpawnSlot(BM::GUID const &rkGuid)const
{
	CONT_SPAWN_SLOT_MAP::const_iterator itr = m_kContSpawnSlotMap.begin();

	while( m_kContSpawnSlotMap.end() != itr)
	{
		if( (*itr).second.CharacterGuid() == rkGuid)
		{
			return ((*itr).second.Slot()+1);
		}
		++itr;
	}
	
	return -1;
}

void PgSelectStage::RemoveSpawnSlot(BM::GUID const &rkGuid)
{
	CONT_SPAWN_SLOT_MAP::iterator itr = m_kContSpawnSlotMap.begin();

	while(itr != m_kContSpawnSlotMap.end())
	{
		if((*itr).second.CharacterGuid() == rkGuid)
		{
			m_kContSpawnSlotMap.erase(itr++);
			OnCallCharList();
			return;
		}
		++itr;
	}
}

NiPoint3 PgSelectStage::GetSpawnPoint(int const iSlot)
{
	BM::vstring kObjectName(L"char_root_select_#IDX#");

	NiPoint3 kPoint3(0, 0, 0);
	if(g_pkWorld)
	{
		kObjectName.Replace(L"#IDX#", BM::vstring(iSlot, L"%02d"));
		std::string strObjectName = BM::vstring::ConvToMultiByte(kObjectName);
		NiAVObject *pkNode = g_pkWorld->GetSceneRoot()->GetObjectByName(strObjectName.c_str());
		//NiAVObject *pkNode = g_pkWorld->GetSceneRoot()->GetObjectByName("cha_test 02");
		if(!pkNode)
		{
			return NiPoint3(0, 0, 0);
		}
		return pkNode->GetWorldTranslate();
	}
	return kPoint3;
}

BM::GUID PgSelectStage::GetSpawnActor(int iSpawnSlot)
{
	CONT_SPAWN_SLOT_MAP::iterator itr = m_kContSpawnSlotMap.find(iSpawnSlot-1);
	if(itr != m_kContSpawnSlotMap.end())
	{
		return (*itr).second.CharacterGuid();
	}

	return BM::GUID::NullData();
}

BM::GUID PgSelectStage::GetDeleteActor(int iSpawnSlot)
{
	CONT_SPAWN_SLOT_MAP::iterator itr = m_kContDeleteSlotMap.find(iSpawnSlot-1);
	if(itr != m_kContDeleteSlotMap.end())
	{
		return (*itr).second.CharacterGuid();
	}

	return BM::GUID::NullData();
}

bool PgSelectStage::SelectDefaultCharacter()
{
	CONT_SPAWN_SLOT_MAP::const_iterator iter_Slot = m_kContSpawnSlotMap.begin();
	if( m_kContSpawnSlotMap.end() == iter_Slot)
	{
		m_iSelectUISelectedSlot = 0;
		m_iSelectUIDrawStartSlot = 0;
	}
	else
	{
		int const iFirstSlot = (*iter_Slot).second.Slot();
		m_iSelectUISelectedSlot = iFirstSlot;
		m_iSelectUIDrawStartSlot = iFirstSlot;
	}

	SetSelectedActor(m_iSelectUISelectedSlot);
	return true;
}

bool PgSelectStage::AddToSlot(SSelectCharKey const &kKey, bool const bDeleteWait)
{
	if( bDeleteWait )
	{
		std::pair<CONT_SPAWN_SLOT_MAP::iterator, bool> kResult = m_kContDeleteSlotMap.insert( std::make_pair(kKey.Slot(), kKey) );
		if( false == kResult.second )
		{
			return false;
		}
	}
	else
	{
		std::pair<CONT_SPAWN_SLOT_MAP::iterator, bool> kResult = m_kContSpawnSlotMap.insert( std::make_pair(kKey.Slot(), kKey) );
		if( false == kResult.second )
		{
			return false;
		}
	}

	return true;
}

void PgSelectStage::ResetSpawnSlot()
{
	if(!g_pkWorld)
	{
		return;
	}
	CONT_SPAWN_SLOT_MAP::iterator itr = m_kContSpawnSlotMap.begin();
	while(itr != m_kContSpawnSlotMap.end())
	{
		BM::GUID kGuid;
		kGuid = (*itr).second.CharacterGuid();
		g_pkWorld->RemoveObject(kGuid);
		++itr;
	}

	CONT_SPAWN_SLOT_MAP kTemp;
	kTemp.swap(m_kContSpawnSlotMap);
}

PgActor *PgSelectStage::AddNewActor(int const iClass, int const iGender, char const *pcSpawnLoc)
{
	if(g_pkWorld == NULL)
	{
		PG_ASSERT_LOG(NULL);
		return NULL;
	}

	BM::GUID kTempGuid = BM::GUID::Create();
	CUnit *pkUnit = g_kTotalObjMgr.CreateUnit(UT_PLAYER, kTempGuid);
	
	PgPlayer *pkPlayer = dynamic_cast<PgPlayer *>(pkUnit);
	SPlayerDBData kDBInfo; 
	SPlayerBasicInfo kBasicInfo;
	SPlayerBinaryData kBinaryData;
	
	kDBInfo.guidCharacter = kTempGuid;
	kDBInfo.byGender = iGender;
	kDBInfo.iClass = iClass;
	kDBInfo.wLv = 1;
	kDBInfo.byFiveElementBody = 0;
	kDBInfo.iHP = 10;

	kBasicInfo.iMaxHP = kDBInfo.iHP;
	pkPlayer->Create( BM::GUID::Create(), kDBInfo, kBasicInfo, kBinaryData);

	NiPoint3 kSpawnLoc;
	if(!g_pkWorld->FindSpawnLoc(pcSpawnLoc, kSpawnLoc))
	{//스폰이름으로 못찾으면 노드로 재검색해보자.
		NiAVObject *pkNode = g_pkWorld->GetSceneRoot()->GetObjectByName(pcSpawnLoc);
		if(!pkNode)
		{
			return 0;
		}
		kSpawnLoc = pkNode->GetWorldTranslate();
	}
	pkUnit->SetPos(POINT3(kSpawnLoc.x, kSpawnLoc.y, kSpawnLoc.z));

	PgActor *pkActor = dynamic_cast<PgActor *>(g_pkWorld->AddUnit(pkUnit));
	
	return pkActor;
}

int PgSelectStage::GetSpawnSlotCount()
{
	return (int)m_kContSpawnSlotMap.size();
}

int PgSelectStage::GetDeleteSlotCount()
{
	return (int)m_kContDeleteSlotMap.size();
}


void PgSelectStage::OnCallCharList()
{
	CONT_SPAWN_SLOT_MAP::iterator itr = m_kContSpawnSlotMap.find(m_iSelectUISelectedSlot);

	int index = 0;

	if( itr != m_kContSpawnSlotMap.end() )
	{
		m_kSelectedActorGuid = (*itr).second.CharacterGuid();
	}
}

void PgSelectStage::SetCharInfo()
{
	XUI::CXUI_Wnd * pWnd = XUIMgr.Activate(L"SFRM_CHARACTOR_INFO");
	if( !pWnd )
	{
		return;
	}

	PgPilot* pkPilot = g_kPilotMan.FindPilot(m_kSelectedActorGuid);
	PgPlayer *pkPlayer = NULL;
	if (pkPilot != NULL)
	{
		pkPlayer = dynamic_cast< PgPlayer* >(pkPilot->GetUnit());
	}

	//캐릭터 아이디
	XUI::CXUI_Wnd *pkID= pWnd->GetControl(std::wstring(_T("FRM_ID_TEXT")));
	if( pkID )
	{
		if(pkPlayer)
		{
			pkID->Text(pkPlayer->Name());
		}
		else
		{
			pkID->Text(L"");
		}
	}
	//캐릭터 레벨
	XUI::CXUI_Wnd *pkLevel= pWnd->GetControl(std::wstring(_T("FRM_LEVEL_TEXT")));
	if( pkLevel )
	{
		BM::vstring kText(TTW(791500));
		if(pkPlayer)
		{
			BM::vstring kLevel(pkPlayer->GetAbil(AT_LEVEL));
			kText.Replace(L"#LEVEL#", kLevel);
		}
		else
		{
			kText.Replace(L"#LEVEL#", L" ");
		}
		pkLevel->Text(static_cast<std::wstring>(kText));
	}
	//직업
	XUI::CXUI_Wnd *pkClass= pWnd->GetControl(std::wstring(_T("FRM_CLASS_TEXT")));
	if( pkClass )
	{
		BM::vstring kText(TTW(791513));
		if(pkPlayer)
		{
			int const iClass = pkPlayer->GetAbil(AT_CLASS);
			kText.Replace(L"#CLASS#", TTW(30000+iClass));
		}
		else
		{
			kText.Replace(L"#CLASS#", L" ");
		}
		pkClass->Text(static_cast<std::wstring>(kText));
	}
	//성별
	XUI::CXUI_Wnd *pkGender= pWnd->GetControl(std::wstring(_T("FRM_SEX_TEXT")));
	if( pkGender )
	{
		BM::vstring kText(TTW(791514));
		if(pkPlayer)
		{
			int const iGender = pkPlayer->GetAbil(AT_GENDER);
			if(1 == iGender)
			{
				kText.Replace(L"#SEX#", TTW(600501));
			}
			else
			{
				kText.Replace(L"#SEX#", TTW(600502));
			}
		}
		else
		{
			kText.Replace(L"#SEX#", L" ");
		}
		pkGender->Text(static_cast<std::wstring>(kText));	
	}
	//서버
	XUI::CXUI_Wnd *pkRealm= pWnd->GetControl(std::wstring(_T("FRM_SERVER_TEXT")));
	if( pkRealm )
	{
		PgRealm	Realm;
		if( S_OK == g_kRealmMgr.GetRealm(g_kNetwork.NowRealmNo(), Realm) )
		{
			BM::vstring kRealm(TTW(791501));
			kRealm.Replace(L"#SERVER#", Realm.Name());
			pkRealm->Text(static_cast<std::wstring>(kRealm));
		}
	}

	//맵이름
	XUI::CXUI_Wnd *pkMapText = pWnd->GetControl(std::wstring(_T("FRM_MAP_TEXT")));
	if( pkMapText )
	{//너무 길면 말 줄임표를 쓴다.
		if(pkPlayer)
		{
			int const iMap = pkPlayer->GetRecentMapNo(GATTR_DEFAULT);
			if( iMap )
			{//맵 이름
				BM::vstring kMapName(TTW(791502));
				kMapName.Replace(L"#LOCATION#", UNI(lwGetMapNameW(iMap).GetStr()) );
				int const iWidth = pkMapText->Width();
				Quest::SetCutedTextLimitLength(pkMapText, static_cast<std::wstring>(kMapName), _T("..."), iWidth);
			}
			else
			{
				pkMapText->Text(L"");
			}
		}
		else
		{
			pkMapText->Text(L"");
		}
	}

	XUI::CXUI_Wnd *pkBirthText = pWnd->GetControl(std::wstring(_T("FRM_PLAY_TIME_TEXT")));
	if (pkBirthText)
	{//플레이타임.
		__int64 i64Hour = 0;
		__int64 i64Min = 0;
		if(pkPlayer)
		{
			__int64 i64TotalPlayTime = pkPlayer->GetTotalPlayTimeSec();
			if( 0 < i64TotalPlayTime)
			{
				__int64 iTotalPlayTimeMin = i64TotalPlayTime / 60;
				i64Hour = iTotalPlayTimeMin / 60;
				if( 60 < iTotalPlayTimeMin )
				{
					i64Min = iTotalPlayTimeMin % 60;
				}
				else
				{
					i64Min = iTotalPlayTimeMin;
				}
			}
		}

		BM::vstring kBirthText(TTW(791503));
		kBirthText.Replace(L"#HOUR#", i64Hour);
		kBirthText.Replace(L"#MIN#", i64Min);
		pkBirthText->Text(static_cast<std::wstring>(kBirthText));
	}

	XUI::CXUI_Wnd *pkImgClass = pWnd->GetControl(std::wstring(_T("IMG_CLASS")));
	if (pkImgClass)
	{
		if(pkPlayer)
		{
			pkImgClass->Visible(true);
			int const iClass = pkPlayer->GetAbil(AT_CLASS);
			lwSetMiniClassIconIndex(pkImgClass, iClass);
		}
		else
		{
			pkImgClass->Visible(false);
		}
	}
	
	XUI::CXUI_Wnd *pkExp= pWnd->GetControl(std::wstring(_T("FRM_EXP")));
	XUI::CXUI_Wnd *pkExpInfo= pWnd->GetControl(std::wstring(_T("FRM_INFO")));
	if (pkExp)
	{//경험치 보너스 표시
		if( !pkPlayer || !lwUseLevelRank() )
		{
			pkExp->Visible(false);
			pkExpInfo->Visible(false);
		}
		else
		{
			size_t iLevelRank = 0;

			CONT_SPAWN_SLOT_MAP::iterator slot_itr = m_kContSpawnSlotMap.find(m_iSelectUISelectedSlot);
			if( m_kContSpawnSlotMap.end() != slot_itr )
			{
				CONT_SPAWN_SLOT_MAP::iterator temp_slot_itr = m_kContSpawnSlotMap.begin();
				while( m_kContSpawnSlotMap.end() != temp_slot_itr)
				{
					if( (*temp_slot_itr).second.Level() > (*slot_itr).second.Level())
					{
						++iLevelRank;
					}
					++temp_slot_itr;
				}
			}
			if( 0 == iLevelRank )
			{
				pkExp->Visible(false);
			}
			else
			{
				pkExp->Visible(true);
				pkExp->UVUpdate( iLevelRank );
			}
		}
	}

	if(pkPlayer)
	{
		bool bNewRace = false;
		__int64 i64TotalPlayTime = pkPlayer->GetTotalPlayTimeSec();
		int const iClassNo = pkPlayer->GetAbil(AT_CLASS);
		if( UCLASS_SHAMAN == iClassNo ||
			UCLASS_DOUBLE_FIGHTER == iClassNo )
		{// 신종족이고
			if( !i64TotalPlayTime )
			{//  플레이타임 0이고
				int const iMap = pkPlayer->GetRecentMapNo(GATTR_DEFAULT);
				if( !iMap )
				{//최근 맵정보 없으면...!!
					//완전 처음 들어오는 거다 (신종족 최초 진입 로딩 이미지를 돌리자)
					bNewRace = true;
				}
			}
		}
		lua_tinker::call<void, bool>("SetNewRaceLoadingImg", bNewRace);
	}
}

void PgSelectStage::OnCallDeleteWait()
{	
	if( m_kContDeleteSlotMap.empty() )
	{
		XUI::CXUI_List* pParent = dynamic_cast<XUI::CXUI_List*>(XUIMgr.Get(L"LST_REALM_COMBINE_CHAR_INFO"));
		if( !pParent )
		{
			return;
		}
		pParent->DeleteAllItem();
		pParent->Close();

		lua_tinker::call<void>("OnSelectUI");
		return;
	}
	
	XUI::CXUI_List* pParent = dynamic_cast<XUI::CXUI_List*>(XUIMgr.Activate(L"LST_REALM_COMBINE_CHAR_INFO"));
	if( !pParent )
	{
		return;
	}

	int const iListCount = pParent->GetTotalItemCount();
	if( iListCount < m_kContDeleteSlotMap.size() )
	{
		for(int i = iListCount; i < m_kContDeleteSlotMap.size(); ++i)
		{
			pParent->AddItem(L"");
		}
	}
	else if( iListCount > m_kContDeleteSlotMap.size() )
	{
		for(int i = iListCount; i > m_kContDeleteSlotMap.size(); --i)
		{
			pParent->DeleteItem(pParent->FirstItem());
		}		
	}

	XUI::SListItem* pItem = pParent->FirstItem();

	CONT_SPAWN_SLOT_MAP::iterator	c_iter = m_kContDeleteSlotMap.begin();
	while( c_iter != m_kContDeleteSlotMap.end() )
	{
		if( pItem && pItem->m_pWnd )
		{
			BM::GUID const kCharGuid = (*c_iter).second.CharacterGuid();
			SetCharInfoToCard(pItem->m_pWnd, kCharGuid);
			pItem->m_pWnd->SetCustomData(&kCharGuid, sizeof(kCharGuid));

			PgActor *pActor = g_kPilotMan.FindActor(kCharGuid);
			if( pActor )
			{
				CUnit* pUnit = pActor->GetUnit();
				if( pUnit )
				{
					PgPlayer* pPlayer = dynamic_cast<PgPlayer*>(pUnit);
					if(pPlayer)
					{
						BYTE const kState = pPlayer->GetDBPlayerState();
						bool const bChangeName = (( kState & CIDBS_NeedRename) == CIDBS_NeedRename)?(true):(false);;
						XUI::CXUI_Wnd* pChangeName = pItem->m_pWnd->GetControl(L"BTN_CHANGE_NAME");
						XUI::CXUI_Wnd* pCombine = pItem->m_pWnd->GetControl(L"BTN_COMBINE");
						XUI::CXUI_Wnd* pDelete = pItem->m_pWnd->GetControl(L"BTN_DELETE");
						XUI::CXUI_Wnd* pTextWnd = pItem->m_pWnd->GetControl(L"FRM_CARD_DISABLER");
						if( pChangeName && pCombine && pDelete && pTextWnd)
						{
							pChangeName->Visible(bChangeName);
							pCombine->Visible(!bChangeName);
							pDelete->Visible(!bChangeName);
							if( bChangeName )
							{
								pTextWnd->Text( TTW(50507) );
							}
							else
							{
								pTextWnd->Text( TTW(50506) );
							}
						}
					}
				}
			}

			pItem = pParent->NextItem(pItem);
		}
		++c_iter;
	}

}

void PgSelectStage::SetCharInfoToCard(XUI::CXUI_Wnd* pWnd, BM::GUID const & rkGuid, wchar_t const* pName)
{
	if(!pWnd)
	{
		return;
	}

	pWnd->Visible(true);

	PgPilot* pkPilot = g_kPilotMan.FindPilot(rkGuid);
	if (pkPilot == NULL)
	{
		return;
	}

	PgPlayer const *pkPlayer = dynamic_cast< PgPlayer const * >(pkPilot->GetUnit());

	if(pkPlayer)
	{
		int const iClass = pkPlayer->GetAbil(AT_CLASS);
		BM::vstring const OPTION(_T("{C=0xFF333333/T=Font_Text/}"));
		BM::vstring kText((pName == NULL)?(pkPlayer->Name()):(pName));	//이름
		kText+=L"\n";
		kText+=OPTION;
		kText+=TTW(224);
		kText+=pkPlayer->GetAbil(AT_LEVEL);
		kText+=L"\n";
		kText+=TTW(30000+iClass);
		kText+=L"\n";
		
		XUI::CXUI_Wnd *pkWndMapText = pWnd->GetControl(std::wstring(_T("FRM_MAP_TEXT")));
		if( NULL != pkWndMapText )
		{//너무 길면 말 줄임표를 쓴다.
			int const iMap = pkPlayer->GetRecentMapNo(GATTR_DEFAULT);
			if( iMap )
			{//맵 이름 
				BM::vstring kMapName(OPTION);
				kMapName+=UNI(lwGetMapNameW(iMap).GetStr());			

				int const iWidth = pkWndMapText->Width();
				Quest::SetCutedTextLimitLength(pkWndMapText, static_cast<std::wstring>(kMapName), _T("..."), iWidth);
			}
			else
			{
				pkWndMapText->Text(L"");
			}
		}
		XUI::CXUI_Wnd *pkBirthText = pWnd->GetControl(std::wstring(_T("FRM_BIRTH_TEXT")));
		if (pkBirthText)
		{//생성날짜.
			BM::PgPackedTime const &kBirth = pkPlayer->BirthDate();
			BM::vstring kBirthText((int)kBirth.Year()+BM::PgPackedTime::BASE_YEAR);
			kBirthText+=L"/";
			kBirthText+=kBirth.Month();
			kBirthText+=L"/";
			kBirthText+=kBirth.Day();
			pkBirthText->Text((std::wstring)(kBirthText));
		}

		std::wstring wstrImg = _T("../Data/6_ui/chaMake/chaCd01.tga");

		switch(pkPilot->GetBaseClassID())
		{
		case 1:	{wstrImg = _T("../Data/6_ui/chaMake/chaCd01.tga");}break;//전사계열
		case 2:	{wstrImg = _T("../Data/6_ui/chaMake/chaCd02.tga");}break;//법사계열
		case 3:	{wstrImg = _T("../Data/6_ui/chaMake/chaCd03.tga");}break;//궁수계열
		case 4:	{wstrImg = _T("../Data/6_ui/chaMake/chaCd04.tga");}break;//도둑계열
		}
		pWnd->DefaultImgName(wstrImg);
		pWnd->Text((std::wstring)(kText));

		XUI::CXUI_Wnd *pkClass = pWnd->GetControl(std::wstring(_T("IMG_CLASS")));
		lwSetMiniClassIconIndex(pkClass, iClass);

		XUI::CXUI_Wnd *pkCard = pWnd->Parent();
		if (pkCard)
		{
			XUI::CXUI_Wnd *pkEmpty = pkCard->GetControl(std::wstring(_T("FRM_CHAR_ITEM_EMPTY")));
			if (pkEmpty)
			{
				pkEmpty->Visible(false);
			}

			XUI::CXUI_Wnd *pkDisabler = pkCard->GetControl(std::wstring(_T("FRM_CARD_DISABLER")));
			if( pkDisabler )
			{
				pkDisabler->SetCustomData(&rkGuid, sizeof(rkGuid));

				BYTE const kState = pkPlayer->GetDBPlayerState();
				pkDisabler->Visible((kState & CIDBS_NeedRename) == CIDBS_NeedRename);
			}
		}
	}
}

void PgSelectStage::OnClickCombineCharacter(XUI::CXUI_Wnd* pSelf)
{
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	BM::GUID kGuid;
	if( !pParent->GetCustomDataSize() )
	{
		return;
	}
	pParent->GetCustomData(&kGuid, sizeof(kGuid));

	PgPilot* pkPilot = g_kPilotMan.FindPilot(kGuid);
	if( !pkPilot )
	{
		return;
	}

	CUnit*	pUnit = pkPilot->GetUnit();
	if( !pUnit )
	{
		return;
	}

	PgPlayer* pPlayer = dynamic_cast<PgPlayer*>(pUnit);
	if( !pPlayer )
	{
		return;
	}

	// 뒷 배경의 다른 창이 선택되지 않게 하기 위해 보이지 않는 창을 만듬
	XUI::CXUI_Wnd const* const pkBarrierWnd = XUIMgr.Call( _T("BarrierForDeleteConfirm") );
	if( NULL == pkBarrierWnd ) 
	{
		return;
	}

	XUI::CXUI_Wnd* pRealmRequest = XUIMgr.Call(L"SFRM_COMBINE_REQUEST");
	if( !pRealmRequest )
	{
		return;
	}

	XUI::CXUI_Wnd* pBefore = pRealmRequest->GetControl(L"FRM_CARD_BEFORE");
	XUI::CXUI_Wnd* pAfter = pRealmRequest->GetControl(L"FRM_CARD_AFTER");
	XUI::CXUI_Wnd* pEditBg = pRealmRequest->GetControl(L"FRM_BG_SM");

	XUI::CXUI_Button* pRenameOk = dynamic_cast<XUI::CXUI_Button*>(pRealmRequest->GetControl(L"BTN_RENAME_OK"));
	XUI::CXUI_Button* pOk = dynamic_cast<XUI::CXUI_Button*>(pRealmRequest->GetControl(L"BTN_OK"));
	XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pRealmRequest->GetControl(L"EDT_NAME"));
	if( !pBefore || !pAfter || !pEditBg || !pEdit || !pRenameOk || !pOk )
	{
		return;
	}

	SetCharInfoToCard(pBefore, kGuid);
	pOk->OwnerGuid(kGuid);
	BYTE const kState = pPlayer->GetDBPlayerState();
	if( (kState & CIDBS_NeedRename) == CIDBS_NeedRename )
	{
		pRenameOk->Disable(false);
		pRenameOk->Visible(true);
		pEditBg->Visible(true);
		pEdit->Visible(true);
		pEdit->EditText(L"");
		SetCharInfoToCard(pAfter, kGuid, L" ");
	}
	else
	{
		pRenameOk->Disable(true);
		pRenameOk->Visible(false);
		pEditBg->Visible(false);
		pEdit->Visible(false);
		pOk->Disable(false);
		SetCharInfoToCard(pAfter, kGuid);
		pOk->SetCustomData(pUnit->Name().c_str(), sizeof(std::wstring::value_type)*pUnit->Name().size());
	}
}

void PgSelectStage::OnInputCombineChangeName(XUI::CXUI_Wnd* pSelf)
{
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pSelf);
	if( !pEdit )
	{
		return;
	}

	std::wstring kInputText = pEdit->EditText();
	
	if( kInputText.empty() )
	{
		pEdit->EditText(L"");
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50520, true);
		return;
	}

	if(g_kClientFS.Filter(kInputText, false, FST_ALL)
		|| !g_kUnicodeFilter.IsCorrect(UFFC_CHARACTER_NAME, kInputText)
		)
	{//욕설 등이 있으면
		pEdit->EditText(L"");
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 600037, true);
		return;
	}

	// 캐릭터명 중복 확인 요청
	BM::Stream kPacket( PT_C_N_REQ_CHECK_CHARACTERNAME_OVERLAP );
	kPacket.Push( kInputText );
	NETWORK_SEND( kPacket );

	XUI::CXUI_Wnd* pParent = pEdit->Parent();
	if( !pParent )
	{
		return;
	}
	
	XUI::CXUI_Wnd* pOk = pParent->GetControl(L"BTN_OK");
	if( !pOk )
	{
		return;
	}

	lwUIWnd(pOk).SetCustomDataAsStr(MB(pEdit->EditText()));
}

void PgSelectStage::OnClickCombineOK(XUI::CXUI_Wnd* pSelf)
{
	if( !pSelf )
	{
		return;
	}

	BM::GUID const kGuid = pSelf->OwnerGuid();
	std::wstring kName = lwUIWnd(pSelf).GetCustomDataAsStr().GetWString();

	BM::Stream kPacket(PT_C_N_REQ_REALM_MERGE);
	kPacket.Push(kGuid);
	kPacket.Push(kName);
	NETWORK_SEND(kPacket);
	pSelf->Parent()->Close();
}

void PgSelectStage::OnClickDeleteCharacter(XUI::CXUI_Wnd* pSelf)
{
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	BM::GUID kGuid;
	if( pParent->GetCustomDataSize() )
	{
		pParent->GetCustomData(&kGuid, sizeof(kGuid));
		SetDeleteCharacterInfo(kGuid);
	}
}

bool PgSelectStage::OnDeleteWaitCharacter(BM::GUID const& kGuid)
{
	CONT_SPAWN_SLOT_MAP::iterator itr = m_kContDeleteSlotMap.begin();
	while( itr != m_kContDeleteSlotMap.end() )
	{
		if( kGuid == (*itr).second.CharacterGuid() )
		{
			m_kContDeleteSlotMap.erase(itr);
			OnCallDeleteWait();
			return true;
		}
		++itr;
	}
	return false;
}

void PgSelectStage::SetDeleteCharacterInfo(BM::GUID const& kGuid)
{

	// 캐릭터 삭제 UI 호출시, 뒷 배경의 다른 창이 선택되지 않게 하기 위해 보이지 않는 창을 만듬
	XUI::CXUI_Wnd const* const pkBarrierWnd = XUIMgr.Call( _T("BarrierForDeleteConfirm") );
	if( NULL == pkBarrierWnd ) 
	{
		return;
	}
	
	// 캐릭터 삭제 UI를 xml으로 부터 부름
	XUI::CXUI_Wnd* pParentWnd = XUIMgr.Call( _T("DeleteConfirm") );	
	if( NULL == pParentWnd ) 
	{
		return;
	}
	pParentWnd->SetCustomData(&kGuid, sizeof(kGuid));
	
	// 캐릭터 정보를 표시할 창을 찾음
	XUI::CXUI_Wnd* const pChildWnd = pParentWnd->GetControl( _T("SFRM_CHARACTER_INFO_BOX") );
	if( NULL == pChildWnd ) 
	{
		return;
	}
	
	// 선택된 캐릭터 GUID를 얻어옴
	PgPilot const* const pkPilot = g_kPilotMan.FindPilot( kGuid );	
	if( NULL == pkPilot ) 
	{
		return;
	}

	PgPlayer const* const pkPlayer = dynamic_cast< PgPlayer const* >(pkPilot->GetUnit());
	if( NULL == pkPlayer ) 
	{
		return;
	}
	
	// 캐릭터 클래스, 레벨 이름을 얻어와 캐릭터 정보 창에 표시
	int const iClass = pkPlayer->GetAbil(AT_CLASS);
	BM::vstring kText(pkPlayer->Name());
	kText+=_T( "\n" );	
	kText+=pkPlayer->GetAbil( AT_LEVEL );
	kText+=TTW( 224 );
	kText+=_T( " " );
	kText+=TTW( 30000 + iClass );	
	pChildWnd->Text( std::wstring(kText) );
	
	// 삭제 UI가 호출 될때마다 삭제확인 edit 박스의 문장을 지움
	XUI::CXUI_Edit* const pkEdt = dynamic_cast<XUI::CXUI_Edit *>( pParentWnd->GetControl(_T("EDITBOX_CERTIFICATE_STRING")) );
	if( NULL == pkEdt ) 
	{
		return;
	}
	pkEdt->EditText( _T("") );
	
	// 삭제 확인 버튼이 최초 호출시에는 비활성화
	XUI::CXUI_Button* const pkButton = dynamic_cast<XUI::CXUI_Button*>(pParentWnd->GetControl(_T("BTN_DO_DELETE")));
	if( NULL == pkButton ) 
	{
		return;
	}
	pkButton->Enable( false );
}

bool PgSelectStage::MoveSlotDelToSpawn(BM::GUID const& kGuid)
{
	CONT_SPAWN_SLOT_MAP::iterator itr = m_kContDeleteSlotMap.begin();
	while( itr != m_kContDeleteSlotMap.end() )
	{
		if( kGuid == (*itr).second.CharacterGuid() )
		{
			break;
		}
		++itr;
	}

	if( itr != m_kContDeleteSlotMap.end() )
	{
		int iSlot = 1;
		while( BM::GUID::NullData() != GetSpawnActor(iSlot) )
		{
			++iSlot;
		}
		(*itr).second.Slot( iSlot-1 );
		std::pair<CONT_SPAWN_SLOT_MAP::iterator, bool> kResult = m_kContSpawnSlotMap.insert( std::make_pair((*itr).second.Slot(), (*itr).second) );
		if( kResult.second )
		{//렐름 통합되면서 수정된 슬롯 번호 저장
			DrawSpawnCharacter( (*itr).second.CharacterGuid() );
			BM::Stream kPacket(PT_C_S_REQ_SAVE_CHARACTOR_SLOT);
			kPacket.Push((*itr).second.CharacterGuid());
			kPacket.Push(iSlot);
			NETWORK_SEND_TO_SWITCH(kPacket);

			m_kContDeleteSlotMap.erase(itr++);
		}
	}

	OnCallDeleteWait();
	OnCallCharList();
	return false;
}

bool PgSelectStage::DrawSpawnCharacter( BM::GUID const &rkGuid)
{
	if( NULL == g_pkWorld )
	{
		return false;
	}
	PgActor* pActor = dynamic_cast< PgActor* >( g_pkWorld->FindObject(rkGuid) );
	if( !pActor )
	{
		return false;
	}
	int const iSpawnSlot = GetSpawnSlot(rkGuid);
	if( -1 == iSpawnSlot )
	{
		return false;
	}
	lua_tinker::call<void, int>("DrawSpawnCharacter", iSpawnSlot);
	NiPoint3 kSpawnLoc = GetSpawnPoint(iSpawnSlot);
	kSpawnLoc = g_pkWorld->FindActorFloorPos(kSpawnLoc,-1);

	pActor->SetTranslate( kSpawnLoc );

	return true;
}
bool PgSelectStage::CheckDeleteWaitSlot(BM::GUID const& kGuid)
{
	CONT_SPAWN_SLOT_MAP::iterator itr = m_kContDeleteSlotMap.begin();
	while( itr != m_kContDeleteSlotMap.end() )
	{
		if( kGuid == (*itr).second.CharacterGuid() )
		{
			return true;
		}
		++itr;
	}
	return false;
}

bool PgSelectStage::CheckNameConflict(BM::GUID const& kGuid)
{
	PgPilot* pkPilot = g_kPilotMan.FindPilot(kGuid);
	if( !pkPilot )
	{
		return true;
	}

	CUnit*	pUnit = pkPilot->GetUnit();
	if( !pUnit )
	{
		return true;
	}

	PgPlayer* pPlayer = dynamic_cast<PgPlayer*>(pUnit);
	if( !pPlayer )
	{
		return true;
	}

	BYTE const kState = pPlayer->GetDBPlayerState();
	return ( (kState & CIDBS_NeedRename) == CIDBS_NeedRename );
}


int PgSelectStage::GetNotNewbiePlayerCount() const
{
	int iCount = 0;
	CONT_SPAWN_SLOT_MAP::const_iterator iter = m_kContSpawnSlotMap.begin();
	while( m_kContSpawnSlotMap.end() != iter )
	{
		CONT_SPAWN_SLOT_MAP::mapped_type const &rkCharKey = (*iter).second;
		PgPilot const* pkPilot = g_kPilotMan.FindPilot(rkCharKey.CharacterGuid());
		if( pkPilot )
		{
			PgPlayer const *pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
			if( pkPlayer )
			{
				if( !PgPlayerUtil::IsNewbiePlayer(pkPlayer) )
				{
					++iCount;
				}
			}
		}
		++iter;
	}

	return iCount;
}

bool PgSelectStage::SelectCharactorByGuid(BM::GUID const &rkGuid)
{
	int iSelect = 0;
	CONT_SPAWN_SLOT_MAP::const_iterator iter = m_kContSpawnSlotMap.begin();
	while( m_kContSpawnSlotMap.end() != iter )
	{
		if( (*iter).second.CharacterGuid() == rkGuid )
		{
			SetSelectedActor(iSelect);
			return true;
		}
		++iSelect;
		++iter;
	}
	return false;
}


int PgSelectStage::GetBaseItemSetNo(int const iClassNo)
{
	int iSetNo = 0;
	switch(iClassNo)
	{
	case UCLASS_FIGHTER:
	case UCLASS_MAGICIAN:
	case UCLASS_ARCHER:
	case UCLASS_THIEF:
		{
			iSetNo = BASE_ITEM_SET_HUMAN;
		}break;
	case UCLASS_SHAMAN:
	case UCLASS_DOUBLE_FIGHTER:
		{
			iSetNo = BASE_ITEM_SET_DRAGON;
		}break;
	}

	return iSetNo;
}

int PgSelectStage::GetSelectedSlot()
{
	return m_iSelectUISelectedSlot;
}

bool PgSelectStage::IsCreateNewRace() const
{
	bool bResult = false;
	CONT_SPAWN_SLOT_MAP::const_iterator iter = m_kContSpawnSlotMap.begin();
	while( m_kContSpawnSlotMap.end() != iter )
	{
		CONT_SPAWN_SLOT_MAP::mapped_type kElement = (*iter).second;
		PgPilot *pPilot = g_kPilotMan.FindPilot(kElement.CharacterGuid());
		if(pPilot)
		{
			if( 20 <= kElement.Level() )
			{//20레벨 이상 캐릭터가 있어야지만 신종족 선택 가능
				bResult = true;
				break;
			}
			else if(pPilot->GetBaseClassID() == UCLASS_SHAMAN 
				|| pPilot->GetBaseClassID() == UCLASS_DOUBLE_FIGHTER )
			{
				bResult = true;
				break;
			}
		}
		++iter;
	}
	return bResult;
}