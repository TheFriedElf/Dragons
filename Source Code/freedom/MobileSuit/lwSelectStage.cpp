#include "stdafx.h"
#include "lwSelectStage.h"
#include "PgSelectStage.h"
#include "PgBaseItemSet.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgUIScene.h"
#include "XUI/XUI_Edit.h"
#include "PgWorld.h"
#include "PgNifMan.H"
#include "PgSoundMan.h"
#include "PgNetwork.h"
#include "lohengrin/PgRealmManager.h"
#include "PgCommandMgr.h"
#include "lwPlayTime.h"

#define MAX_LEAF_COUNT 221
#define MAX_LEAF_LOOP 30
#define LEAF_TRANS_TIME 0.01f
#define LEAF_X_VALUE 17
#define LEAF_Y_VALUE 13

extern void CutByRealmNameAndFlag(std::wstring const& kString, std::wstring& RealmName, bool& bIsNew);
float const UI_STAR_CHANGE_DELAY_TIEM = 1.8f;
int const iMAX_CARD_NUM = 4;

lwSelectStage::lwSelectStage(PgSelectStage *pkSelectStage)
	: m_bIsEffect(false)
	, m_asEffect(NULL)
	, m_bIsChangeStar(false)
	, m_fLeafStartTime(0.0f)
	, m_iLeafCount(0)
	, m_eLeafState(LAS_PADEIN)
{
	m_pkSelectStage = pkSelectStage;
}

bool lwSelectStage::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	class_<lwSelectStage>(pkState, "SelectStage")
		.def(pkState, "AddBaseItemSet", &lwSelectStage::AddBaseItemSet)
		.def(pkState, "AddBaseItemSetFromTableData", &lwSelectStage::AddBaseItemSetFromTableData)
		.def(pkState, "AddNewActor", &lwSelectStage::AddNewActor)
		.def(pkState, "GetBaseItemSet", &lwSelectStage::GetBaseItemSet)
		.def(pkState, "GetBaseItemSet_Detail", &lwSelectStage::GetBaseItemSet_Detail)
		.def(pkState, "GetBaseItemNo", &lwSelectStage::GetBaseItemNo)
		.def(pkState, "GetBaseItemIconPath", &lwSelectStage::GetBaseItemIconPath)
		.def(pkState, "GetSpawnSlot", &lwSelectStage::GetSpawnSlot)
		.def(pkState, "RemoveSpawnSlot", &lwSelectStage::RemoveSpawnSlot)
		.def(pkState, "GetSpawnActor", &lwSelectStage::GetSpawnActor)		
		.def(pkState, "SetSelectedActor", &lwSelectStage::SetSelectedActor)
		.def(pkState, "GetSelectedActor", &lwSelectStage::GetSelectedActor)
		.def(pkState, "SetNewActor", &lwSelectStage::SetNewActor)
		.def(pkState, "GetNewActor", &lwSelectStage::GetNewActor)
		.def(pkState, "ClearNewActor", &lwSelectStage::ClearNewActor)
		.def(pkState, "SetFirstExposedSlot", &lwSelectStage::SetFirstExposedSlot)
		.def(pkState, "NextSlot", &lwSelectStage::NextSlot)
		.def(pkState, "PrevSlot", &lwSelectStage::PrevSlot)
		
		.def(pkState, "EquipBaseItem", &lwSelectStage::EquipBaseItem)
		.def(pkState, "EquipBaseItemBySeq", &lwSelectStage::EquipBaseItemBySeq)
		.def(pkState, "EquipBaseItemSet", &lwSelectStage::EquipBaseItemSet)
		.def(pkState, "EquipBaseItemSetByPos", &lwSelectStage::EquipBaseItemSetByPos)
		
		
		.def(pkState, "SetFiveElement", &lwSelectStage::SetFiveElement)
		.def(pkState, "GetFiveElement", &lwSelectStage::GetFiveElement)
		.def(pkState, "SetDefaultFiveElement", &lwSelectStage::SetDefaultFiveElement)

		.def(pkState, "ResetSpawnSlot", &lwSelectStage::ResetSpawnSlot)
		.def(pkState, "GetSpawnSlotCount", &lwSelectStage::GetSpawnSlotCount)	
		.def(pkState, "GetDeleteSlotCount", &lwSelectStage::GetDeleteSlotCount)
		.def(pkState, "OnCallCharList", &lwSelectStage::OnCallCharList)
		.def(pkState, "OnCallDeleteWait", &lwSelectStage::OnCallDeleteWait)
		.def(pkState, "Send_PT_C_S_REQ_CREATE_CHARACTER", &lwSelectStage::Send_PT_C_S_REQ_CREATE_CHARACTER)
		.def(pkState, "OnCardClick", &lwSelectStage::OnCardClick)
		.def(pkState, "OnCreateDefault", &lwSelectStage::OnCreateDefault)
		.def(pkState, "InitElementSelecter", &lwSelectStage::InitElementSelecter)
		.def(pkState, "UpdateElement", &lwSelectStage::UpdateElement)
		.def(pkState, "SetElemSlot", &lwSelectStage::SetElemSlot)
		.def(pkState, "GetRealmName", &lwSelectStage::GetRealmName)

		.def(pkState, "InitRenderModel", &lwSelectStage::InitRenderModel)
		.def(pkState, "DrawRenderModel", &lwSelectStage::DrawRenderModel)
		.def(pkState, "AddActor", &lwSelectStage::AddActor)
		.def(pkState, "LeafAnimation", &lwSelectStage::LeafAnimation)
		.def(pkState, "SetLeafState", &lwSelectStage::SetLeafState)
		.def(pkState, "GetLeafState", &lwSelectStage::GetLeafState)

		.def(pkState, "CallDeleteCharacterUI", &lwSelectStage::CallDeleteCharacterUI)
		.def(pkState, "DeleteCharacterIfAble", &lwSelectStage::DeleteCharacterIfAble)
		.def(pkState, "GetIsActiveDeleteConfirmEditBox", &lwSelectStage::GetIsActiveDeleteConfirmEditBox)
		.def(pkState, "GetIsSuitableInputForDeleteCharacter", &lwSelectStage::GetIsSuitableInputForDeleteCharacter)
		.def(pkState, "CallDeleteCharacterIfAble", &lwSelectStage::CallDeleteCharacterIfAble)

		.def(pkState, "OnClickCombineCharacter", &lwSelectStage::OnClickCombineCharacter)
		.def(pkState, "OnClickCombineOK", &lwSelectStage::OnClickCombineOK)
		.def(pkState, "OnClickDeleteCharacter", &lwSelectStage::OnClickDeleteCharacter)
		.def(pkState, "OnInputCombineChangeName", &lwSelectStage::OnInputCombineChangeName)
		.def(pkState, "CheckNameConflict", &lwSelectStage::CheckNameConflict)
		
		.def(pkState, "SetServerInfo", &lwSelectStage::SetServerInfo)
		.def(pkState, "SetCharInfo", &lwSelectStage::SetCharInfo)
		
		.def(pkState, "CallSelectStyle_Hair", &lwSelectStage::CallSelectStyle_Hair)
		.def(pkState, "CallSelectStyle_Color", &lwSelectStage::CallSelectStyle_Color)
		.def(pkState, "CallSelectStyle_Face", &lwSelectStage::CallSelectStyle_Face)

		.def(pkState, "ClearTestParts", &lwSelectStage::ClearTestParts)
		.def(pkState, "GetSelectedSlot", &lwSelectStage::GetSelectedSlot)
		.def(pkState, "ReqEnableExtendSlot", &lwSelectStage::ReqEnableExtendSlot)
		.def(pkState, "GetEnableExtendSlot", &lwSelectStage::GetEnableExtendSlot)
		.def(pkState, "IsCreateNewRace", &lwSelectStage::IsCreateNewRace)
		.def(pkState, "GetActorPosToScreen", &lwSelectStage::GetActorPosToScreen)
		;

	return true;
}

void lwSelectStage::GetRealmName(lwUIWnd UISelf)
{
	PgRealm	Realm;
	if( S_OK == g_kRealmMgr.GetRealm(g_kNetwork.NowRealmNo(), Realm) )
	{
		std::wstring wStr;
		bool bTemp = false;

		CutByRealmNameAndFlag(Realm.Name(), wStr, bTemp);
		wStr += TTW(400437);
		lwWString lwstrName(wStr);
		UISelf.SetStaticTextW(lwstrName);
	}
}

void lwSelectStage::SetServerInfo()
{
	XUI::CXUI_Wnd* pMain = XUIMgr.Get(L"SFRM_SERVER_INFO");
	if(!pMain)
	{
		pMain = XUIMgr.Activate(L"SFRM_SERVER_INFO");
		if( !pMain )
		{
			return;
		}
	}
	//서버명
	XUI::CXUI_Wnd* pServer = pMain->GetControl(L"FRM_NAME");
	if(pServer)
	{
		GetRealmName(lwUIWnd(pServer));
	}
	//채널
	XUI::CXUI_Wnd* pChennel= pMain->GetControl(L"FRM_CHENNEL");
	if(pChennel)
	{
		int const iChennel = lwGetCurrentChennelName();
		if(iChennel)
		{
			BM::vstring kText(TTW(791504));
			kText.Replace(L"#CHENNEL#", iChennel);
			pChennel->Text(static_cast<std::wstring>(kText));
		}
	}
	//마지막 접속시간
	XUI::CXUI_Wnd* pConnectTime= pMain->GetControl(L"FRM_LAST_CONNECT_INFO");
	if(pConnectTime)
	{
		BM::DBTIMESTAMP_EX kLastLogOutTime = g_klwPlayTime.GetLastLogOutTime();
		BM::vstring kText;
		BM::vstring kHour;
		if( kLastLogOutTime.IsNull() )
		{//접속기록 없음
			kText = TTW(791518);
		}

		switch(g_kLocal.ServiceRegion())
		{
		case LOCAL_MGR::NC_CHINA:
			{
				kText = TTW(791522);
				kHour = kLastLogOutTime.hour;
			}break;
		default:
			{
				kText = TTW(791506);
				kHour = (kLastLogOutTime.hour % 12);
			}break;
		}
		kText.Replace(L"#YEAR#", kLastLogOutTime.year );
		kText.Replace(L"#MONTH#", kLastLogOutTime.month);
		kText.Replace(L"#DAY#", kLastLogOutTime.day);
		kText.Replace(L"#HOUR#",  kHour);
		BM::vstring vStrSec(kLastLogOutTime.minute, L"%02u");
		kText.Replace(L"#MIN#", vStrSec);
		if( 0 != (kLastLogOutTime.hour/12) )
		{//PM
			kText.Replace(L"#ISAFTER#", TTW(3542));
		}
		else
		{//AM
			kText.Replace(L"#ISAFTER#", TTW(3541));
		}
		pConnectTime->Text(static_cast<std::wstring>(kText));
	}
}

void lwSelectStage::SetCharInfo()
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->SetCharInfo();
}

void lwSelectStage::OnCardClick(int iSelect)
{
	SetSelectedActor(iSelect);
	//m_pkSelectStage->OnCallCharList();
}

void lwSelectStage::OnCreateDefault()
{
	if(NULL == m_pkSelectStage) { return; }

	SetSelectedActor(m_pkSelectStage->GetAllCharacterCount() - 1);
}

PgSelectStage *lwSelectStage::operator()()
{
	return m_pkSelectStage;
}

lwBaseItemSet lwSelectStage::AddBaseItemSet(int iItemPos, int iItemCnt, int iExposedSlot)
{
	if(NULL == m_pkSelectStage) { return lwBaseItemSet(NULL); }

	return lwBaseItemSet(m_pkSelectStage->AddBaseItemSet(iItemPos, iItemCnt, iExposedSlot));
}

void lwSelectStage::AddBaseItemSetFromTableData()
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->AddBaseItemSetFromTableData();
}

lwActor lwSelectStage::AddNewActor(int const iClass, int const iGender, char const *pcSpawnLoc)
{
	if(NULL == m_pkSelectStage) { return lwActor(NULL); }

	return lwActor(m_pkSelectStage->AddNewActor(iClass, iGender, pcSpawnLoc));
}

int lwSelectStage::GetSpawnSlot(lwGUID kGuid)
{
	if(NULL == m_pkSelectStage) { return 0; }

	return m_pkSelectStage->GetSpawnSlot(kGuid());
}

void lwSelectStage::RemoveSpawnSlot(lwGUID kGuid)
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->RemoveSpawnSlot(kGuid());
}

lwGUID lwSelectStage::GetSpawnActor(int iSpawnSlot)
{
	if(NULL == m_pkSelectStage) { return lwGUID(BM::GUID::NullData()); }

	return lwGUID(m_pkSelectStage->GetSpawnActor(iSpawnSlot));
}

void lwSelectStage::SetSelectedActor(int iSelect)
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->SetSelectedActor(iSelect);
}

lwGUID lwSelectStage::GetSelectedActor()
{
	if(NULL == m_pkSelectStage) { return lwGUID(BM::GUID::NullData()); }

	return lwGUID(m_pkSelectStage->GetSelectedActor());
}

void lwSelectStage::SetNewActor(lwActor kActor)
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->SetNewActor(kActor());
}

lwActor lwSelectStage::GetNewActor()
{
	if(NULL == m_pkSelectStage) { return lwActor(NULL); }

	return lwActor(m_pkSelectStage->GetNewActor());
}

void lwSelectStage::ClearNewActor()
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->SetNewActor(NULL);
}

void lwSelectStage::SetFirstExposedSlot(int iType, int iGender, int iFirstExposedSlot)
{
	if(NULL == m_pkSelectStage) { return; }

	PgBaseItemSet *pkBaseItemSet = m_pkSelectStage->GetBaseItemSet(iType, iGender);
	if(!pkBaseItemSet)
	{
		return;
	}
	pkBaseItemSet->SetFirstExposedSlot(iFirstExposedSlot);
}

void lwSelectStage::NextSlot(int iType, bool bCheckGender)
{
	if(NULL == m_pkSelectStage) { return; }

	int iGender = 3;
	if(bCheckGender)
	{
		iGender = m_pkSelectStage->GetNewActor()->GetPilot()->GetAbil(AT_GENDER);
	}
	
	int const iClass = m_pkSelectStage->GetNewActor()->GetPilot()->GetBaseClassID();
	int const iSetNo = m_pkSelectStage->GetBaseItemSetNo(iClass);
	
	PgBaseItemSet *pkBaseItemSet = m_pkSelectStage->GetBaseItemSets(iType, iSetNo, iGender);
	if(!pkBaseItemSet)
	{
		return;
	}
	pkBaseItemSet->NextSlot();
}

void lwSelectStage::PrevSlot(int iType, bool bCheckGender)
{
	if(NULL == m_pkSelectStage) { return; }

	int iGender = 3;
	if(bCheckGender)
	{
		iGender = m_pkSelectStage->GetNewActor()->GetPilot()->GetAbil(AT_GENDER);
	}

	int const iClass = m_pkSelectStage->GetNewActor()->GetPilot()->GetBaseClassID();
	int const iSetNo = m_pkSelectStage->GetBaseItemSetNo(iClass);

	PgBaseItemSet *pkBaseItemSet = m_pkSelectStage->GetBaseItemSets(iType, iSetNo, iGender);
	if(!pkBaseItemSet)
	{
		return;
	}
	pkBaseItemSet->PrevSlot();
}

void lwSelectStage::NextSetSlot(int iSet)
{
	if(NULL == m_pkSelectStage) { return; }

	PgBaseItemSet *pkBaseItemSet = m_pkSelectStage->GetBaseItemSets(iSet);
	if(!pkBaseItemSet)
	{
		return;
	}
	pkBaseItemSet->NextSlot();
}

void lwSelectStage::PrevSetSlot(int iSet)
{
	if(NULL == m_pkSelectStage) { return; }

	PgBaseItemSet *pkBaseItemSet = m_pkSelectStage->GetBaseItemSets(iSet);
	if(!pkBaseItemSet)
	{
		return;
	}

	pkBaseItemSet->PrevSlot();
}

int lwSelectStage::GetBaseItemNo(int iType, int iExposedPos, int iSetItemPos)
{
	if(NULL == m_pkSelectStage) { return -1; }

	PgBaseItemSet *pkBaseItemSet = m_pkSelectStage->GetBaseItemSet(iType);
	if(!pkBaseItemSet)
	{
		return -1;
	}

	PgBaseItemSet::PgBaseItem *pkBaseItem = pkBaseItemSet->GetBaseItem(iExposedPos);
	return pkBaseItem->GetItemNo(iSetItemPos);
}

char const *lwSelectStage::GetBaseItemIconPath(int iType, int iExposedPos)
{
	if(!m_pkSelectStage || !m_pkSelectStage->GetNewActor() || !m_pkSelectStage->GetNewActor()->GetPilot())
	{
		return	0;
	}

	int iGender = m_pkSelectStage->GetNewActor()->GetPilot()->GetAbil(AT_GENDER);
	PgBaseItemSet *pkBaseItemSet = m_pkSelectStage->GetBaseItemSet(iType, iGender);
	if(!pkBaseItemSet)
	{
		return 0;
	}

	PgBaseItemSet::PgBaseItem *pkBaseItem = pkBaseItemSet->GetBaseItem(iExposedPos);
	if(!pkBaseItem)
	{
		return 0;
	}

	return pkBaseItem->GetIconPath().c_str();
}

char const *lwSelectStage::GetBaseItemSetIconPath(int iSetGroup, int iExposedPos)
{
	if(NULL == m_pkSelectStage) { return NULL; }

	PgBaseItemSet *pkBaseItemSet = m_pkSelectStage->GetBaseItemSets(iSetGroup);
	if(!pkBaseItemSet)
	{
		return 0;
	}

	PgBaseItemSet::PgBaseItem *pkBaseItem = pkBaseItemSet->GetBaseItem(iExposedPos);
	if( !pkBaseItem )
	{
		return 0;
	}
	return pkBaseItem->GetIconPath().c_str();
}

char const *lwSelectStage::GetBaseItemSetIconPath(int iType, int iSetNo, int iGender, int iExposedPos)
{
	if(NULL == m_pkSelectStage) { return NULL; }

	PgBaseItemSet *pkBaseItemSet = m_pkSelectStage->GetBaseItemSets(iType, iSetNo, iGender);
	if(!pkBaseItemSet)
	{
		return 0;
	}

	PgBaseItemSet::PgBaseItem *pkBaseItem = pkBaseItemSet->GetBaseItem(iExposedPos);
	if( !pkBaseItem )
	{
		return 0;
	}
	return pkBaseItem->GetIconPath().c_str();
}

lwBaseItemSet lwSelectStage::GetBaseItemSet(int iType)
{
	if(NULL == m_pkSelectStage) { return lwBaseItemSet(NULL); }

	return lwBaseItemSet(m_pkSelectStage->GetBaseItemSet(iType));
}

lwBaseItemSet lwSelectStage::GetBaseItemSet_Detail(int const iType, int const iSetNo, int const iGender)
{
	if(NULL == m_pkSelectStage) { return lwBaseItemSet(NULL); }

	return lwBaseItemSet(m_pkSelectStage->GetBaseItemSets(iType, iSetNo, iGender));
}

int lwSelectStage::EquipBaseItem(int iType, int iExposedSlot)
{
	if(NULL == m_pkSelectStage) { return 0; }

	int const iClass = m_pkSelectStage->GetNewActor()->GetPilot()->GetBaseClassID();
	int const iSetNo = m_pkSelectStage->GetBaseItemSetNo(iClass);

	return m_pkSelectStage->EquipBaseItem(iType, iSetNo, iExposedSlot);
}

int lwSelectStage::EquipBaseItemBySeq(int iType, int iSeq)
{
	if(NULL == m_pkSelectStage) { return 0; }

	if(NULL == m_pkSelectStage->GetNewActor() || NULL == m_pkSelectStage->GetNewActor()->GetPilot())
	{
		return 0;
	}
	int const iClass = m_pkSelectStage->GetNewActor()->GetPilot()->GetBaseClassID();
	int const iSetNo = m_pkSelectStage->GetBaseItemSetNo(iClass);
	
	return m_pkSelectStage->EquipBaseItemBySeq(iType, iSetNo, iSeq);
}

void lwSelectStage::EquipBaseItemSet(int iClass, int iSet)
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->EquipBaseItemSet(iClass, iSet);
}
void lwSelectStage::EquipBaseItemSetByPos(int iSet, int iExposedPos)
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->EquipBaseItemSetByPos(iSet, iExposedPos);
}

bool lwSelectStage::SetFiveElement(int Element)
{
	if (m_pkSelectStage)
	{
		if( m_pkSelectStage->GetFiveElement() == Element )
		{
			return false;
		}
		m_pkSelectStage->SetFiveElement(Element);
	}

	return true;
}

void lwSelectStage::SetDefaultFiveElement()
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->SetDefaultFiveElement();
}

void lwSelectStage::ResetSpawnSlot()
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->ResetSpawnSlot();
}

int lwSelectStage::GetSpawnSlotCount()
{
	if(NULL == m_pkSelectStage) { return 0; }

	return m_pkSelectStage->GetSpawnSlotCount();
}

int lwSelectStage::GetDeleteSlotCount()
{
	if(NULL == m_pkSelectStage) { return 0; }

	return m_pkSelectStage->GetDeleteSlotCount();
}

void lwSelectStage::OnCallCharList()
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->OnCallCharList();
}

void lwSelectStage::OnCallDeleteWait()
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->OnCallDeleteWait();
}

void lwSelectStage::OnClickCombineCharacter(lwUIWnd UISelf)
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->OnClickCombineCharacter(UISelf.GetSelf());
}

void lwSelectStage::OnInputCombineChangeName(lwUIWnd UISelf)
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->OnInputCombineChangeName(UISelf.GetSelf());
}

void lwSelectStage::OnClickCombineOK(lwUIWnd UISelf)
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->OnClickCombineOK(UISelf.GetSelf());
}

void lwSelectStage::OnClickDeleteCharacter(lwUIWnd UISelf)
{
	if(NULL == m_pkSelectStage) { return; }

	m_pkSelectStage->OnClickDeleteCharacter(UISelf.GetSelf());
}

bool lwSelectStage::CheckNameConflict(lwGUID kGuid)
{
	if(NULL == m_pkSelectStage) { return false; }

	return m_pkSelectStage->CheckNameConflict(kGuid.GetGUID());
}

bool lwSelectStage::Send_PT_C_S_REQ_CREATE_CHARACTER()
{
	if(NULL == m_pkSelectStage) { return false; }

	PgActor *pkActor = m_pkSelectStage->GetNewActor();
	if(!pkActor)
	{
		return false;
	}

	PgPilot *pkPilot = g_kPilotMan.FindPilot(pkActor->GetPilotGuid());
	if(!pkPilot)
	{
		return false;
	}

	SReqCreateCharacter kReqCreateCharacter;
	
	std::wstring wstrCharName;
	if(S_OK != GetCraeteUserName(wstrCharName))
	{
		::Notice_Show( TTW(600013), 0);//이름이 길다.
		return false;
	}

	if(g_kClientFS.Filter(wstrCharName, false, FST_ALL)
		|| !g_kUnicodeFilter.IsCorrect(UFFC_CHARACTER_NAME, wstrCharName)
		)	//욕설 등이 있으면
	{
		::Notice_Show( TTW(600037), 0);
		return false;
	}

	std::wstring::size_type kGM = wstrCharName.find(L"GM");
	if(std::wstring::npos != kGM)
	{
		std::wstring::size_type kLen = wstrCharName.size();
		if(	2 <= wstrCharName.size() 
		&& (0==kGM || kGM==(kLen-2)))	//제일 처음이나 제일 뒤에 GM이 붙어있을 경우
		{
			::Notice_Show( TTW(600037), 0);
			return false;
		}
	}

	if(wstrCharName.size() > _countof(kReqCreateCharacter.szName))
	{
		::Notice_Show( TTW(600013), 0);//이름이 짧다.
		return false;
	}
	else
	{
		ModifyNameForNationCode(wstrCharName);
		_tcscpy(kReqCreateCharacter.szName, wstrCharName.c_str());
	}

	if(!pkPilot->GetAbil(AT_FIVE_ELEMENT_TYPE_AT_BODY))
	{
		::Notice_Show( TTW(600054), 0);//주속성이 없다..
		return false;
	}

	kReqCreateCharacter.byGender			= pkPilot->GetAbil(AT_GENDER);
	kReqCreateCharacter.iClass				= pkPilot->GetAbil(AT_CLASS);
	kReqCreateCharacter.byFiveElement_Body	= pkPilot->GetAbil(AT_FIVE_ELEMENT_TYPE_AT_BODY);
	kReqCreateCharacter.iDefaultHair		= pkActor->GetDefaultItem(EQUIP_LIMIT_HAIR);
	kReqCreateCharacter.iDefaultHairColor	= pkActor->GetDefaultItem(EQUIP_LIMIT_HAIR_COLOR);
	kReqCreateCharacter.iDefaultFace		= pkActor->GetDefaultItem(EQUIP_LIMIT_FACE);
	kReqCreateCharacter.iDefaultShirts		= pkActor->GetDefaultItem(EQUIP_LIMIT_SHIRTS);
	kReqCreateCharacter.iDefaultGlove		= pkActor->GetDefaultItem(EQUIP_LIMIT_GLOVE);
	kReqCreateCharacter.iDefaultPants		= pkActor->GetDefaultItem(EQUIP_LIMIT_PANTS);
	kReqCreateCharacter.iDefaultBoots		= pkActor->GetDefaultItem(EQUIP_LIMIT_BOOTS);

	if(g_kPilotMan.JumpingCharEvent().bUse)
	{
		kReqCreateCharacter.kJumpingCharEvent.iEventNo = g_kPilotMan.JumpingCharEvent().iNo;
		kReqCreateCharacter.kJumpingCharEvent.iClass = g_kPilotMan.JumpingCharEvent().iClass;
	}

	BM::Stream kPacket(PT_C_S_REQ_CREATE_CHARACTER);
	kReqCreateCharacter.WriteToPacket(kPacket);

	NETWORK_SEND(kPacket)
	
	return true;
}

HRESULT lwSelectStage::GetCraeteUserName(std::wstring &kStrName)
{
	XUI::CXUI_Wnd* pParentWnd = XUIMgr.Get(UNI("FRM_BACK_IMAGE"));
	XUI::CXUI_Wnd* pEditWnd = pParentWnd->GetControl(_T("editUserName"));

	XUI::CXUI_Edit* pkEditCtrl = dynamic_cast<XUI::CXUI_Edit*>(pEditWnd);

	if(pkEditCtrl)
	{
		kStrName = pkEditCtrl->EditText();
		if(kStrName.size())
		{
			return S_OK;
		}
	}
	return E_FAIL;	
}

void lwSelectStage::InitElementSelecter(lwUIWnd UISelf)
{
	if( UISelf.IsNil() )
	{
		return;
	}

	POINT2 WndSize = UISelf.GetSize()();
	POINT2 WndCenter(WndSize.x / 2, WndSize.y / 2);
	m_kElementCont.clear();
	for(int i = 1; i <= 5; ++i)
	{
		char const* const szFormName = "FRM_ELEMENT%d";
		char szTemp[MAX_PATH] = {0,};

		sprintf(szTemp, szFormName, i);
		lwUIWnd pChild = UISelf.GetControl(szTemp);
		if( pChild.IsNil() )
		{
			return;
		}

		POINT2 ChildSize = pChild.GetSize()();
		POINT2 ChildCenter;

		float fRadian = (NI_TWO_PI / 5) * (i - 1) - NI_HALF_PI;
		if( fRadian < 0.0f )
		{
			fRadian += NI_TWO_PI;
		}
		ChildCenter.x = cosf(fRadian) * DEF_STAR_HARF_LEN + WndCenter.x;
		ChildCenter.y = sinf(fRadian) * DEF_STAR_HARF_LEN + WndCenter.y;

		pChild.SetLocation(lwPoint2(ChildCenter.x - (ChildSize.x / 2), ChildCenter.y - (ChildSize.y / 2)));
		m_kElementCont.insert(std::make_pair(i, fRadian));
	}
}

void lwSelectStage::UpdateElement(lwUIWnd UISelf)
{
	if(!g_pkWorld)
	{
		return;
	}
	float const fZero = NI_HALF_PI * 3;
	int const MAX_ELEMENT = 5;

	kElementCont::iterator	iter = m_kElementCont.find(m_iSetElem);
	if( iter != m_kElementCont.end() )
	{
		float fTime = g_pkWorld->GetAccumTime();
		if( fTime < m_fEndTime )
		{
			fTime -= m_fStartTime;
			float fMovePercent = sinf((fTime / m_fEffectTime) * NI_HALF_PI);

			float fTotalMoveRadian = 0.0f;
			if( m_fStartRadian > NI_HALF_PI )
			{
				fTotalMoveRadian = abs(fZero - m_fStartRadian);
			}
			else
			{
				fTotalMoveRadian = m_fStartRadian + NI_HALF_PI;
			}

			float fResultRadian = fTotalMoveRadian * fMovePercent;

			if((iter->second > fZero )
			|| (iter->second < NI_HALF_PI ))
			{
				fResultRadian *= -1;
			}

			POINT2 WndSize = UISelf.GetSize()();
			POINT2 WndCenter(WndSize.x / 2, WndSize.y / 2);
			for(int i = 1; i <= MAX_ELEMENT; ++i)
			{
				char const* const szFormName = "FRM_ELEMENT%d";
				char szTemp[MAX_PATH] = {0,};

				sprintf(szTemp, szFormName, i);
				lwUIWnd pChild = UISelf.GetControl(szTemp);
				if( pChild.IsNil() )
				{
					return;
				}

				POINT2 ChildSize = pChild.GetSize()();
				POINT2 ChildCenter;


				kElementCont::iterator	iter = m_kElementCont.find(i);
				kElementCont::iterator	st_iter = m_kStartPosCont.find(i);
				if( iter == m_kElementCont.end() || st_iter == m_kStartPosCont.end() )
				{
					continue; 
				}
				iter->second = st_iter->second + fResultRadian;
				if( iter->second < 0 )
				{
					iter->second += NI_TWO_PI;
				}
				else if( iter->second > NI_TWO_PI )
				{
					iter->second -= NI_TWO_PI;
				}

				ChildCenter.x = cosf(iter->second) * DEF_STAR_HARF_LEN + WndCenter.x;
				ChildCenter.y = sinf(iter->second) * DEF_STAR_HARF_LEN + WndCenter.y;

				pChild.SetLocation(lwPoint2(ChildCenter.x - (ChildSize.x / 2), ChildCenter.y - (ChildSize.y / 2)));
			}
			m_bIsEffect = true;
			return;
		}

		if( m_bIsEffect )
		{	
			lwUIWnd pEffect = UISelf.GetControl("FRM_EFFECT");
			if( !pEffect.IsNil() )
			{
				char const* pUIModelName = "Select_Effect";
				char const* pUIModelNIFName = "ef_Create_cha_01";

				PgUIModel* pkModel = g_kUIScene.FindUIModel(pUIModelName);
				if( pkModel )
				{
					m_asEffect = g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, "Make_Character", 0.0f,0.0f,0.0f);
					pEffect.Visible(true);
					pkModel->ResetNIFAnimation(pUIModelNIFName);
					pkModel->SetNIFEnableUpdate(pUIModelNIFName, true);
				}
			}
			m_bIsEffect = false;
			m_bIsChangeStar = true;
			m_fAccumTime = g_pkWorld->GetAccumTime();
		}

		if( m_bIsChangeStar )
		{
			if( g_pkWorld->GetAccumTime() - m_fAccumTime > UI_STAR_CHANGE_DELAY_TIEM )
			{
				m_bIsChangeStar = false;

				lwUIWnd pStarGlow = UISelf.GetControl("FRM_ELEMENT_GLOW");
				if( !pStarGlow.IsNil() )
				{
					lua_tinker::call<void, lwUIWnd, int>("SetElemStarColor", pStarGlow, g_kSelectStage.GetFiveElement() );
				}
				lwUIWnd pSelectEff = UISelf.GetControl("FRM_SELECT_EFFECT");
				if( !pSelectEff.IsNil() )
				{
					pSelectEff.Visible(true);
				}
			}
		}
	}
}

void lwSelectStage::InitRenderModel(char const* szRenderModelName, lwUIWnd UISelf, bool bUpdate, bool bOrtho)
{
	// Init
	if (!szRenderModelName)
	{
		return;
	}
	PgUIModel* pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if( !pkModel )
	{
		g_kUIScene.InitRenderModel(szRenderModelName, UISelf.GetSize()(), UISelf.GetTotalLocation()(), bUpdate, bOrtho);// Point()
		pkModel = g_kUIScene.FindUIModel(szRenderModelName);
		if( pkModel )
		{
			pkModel->SetCameraZoomMinMax(0, DEF_CHAR_MODEL_LEN + 1);
		}	
	}
}

void lwSelectStage::AddActor(char const* szRenderModelName, char const* szActorName)
{
	if(NULL == m_pkSelectStage) { return; }

	int const DEF_TRANS = 25;

	PgActor *pkActor = m_pkSelectStage->GetNewActor();
	if(!pkActor)
	{
		return;
	}

	PgUIModel* pkModel = g_kUIScene.FindUIModel(szRenderModelName);
	if( pkModel )
	{
		pkModel->DeleteActor(szActorName);
		pkModel->AddActor(szActorName, pkActor->GetActorManager(), false, true);
		NiActorManager* pkAM = pkModel->GetActor(szActorName);
		if(pkAM)
		{
			pkAM->GetNIFRoot()->SetTranslate(NiPoint3(0, 0, -DEF_TRANS));
		}
		pkModel->SetActorEnableUpdate(szActorName, true);
		pkModel->CameraZoomSubstitution(DEF_CHAR_MODEL_LEN);
	}	
}

void lwSelectStage::DrawRenderModel(char const* szRenderModelName, lwUIWnd UISelf)
{
	if (!szRenderModelName)
	{
		return;
	}	

	PgUIModel *pkUIModel = g_kUIScene.FindUIModel(szRenderModelName);
	if(pkUIModel)
	{
		POINT2 ptPos = UISelf.GetTotalLocation()();
		ptPos.y += UISelf.GetSize()().y / 2;
		pkUIModel->RenderFrame(NiRenderer::GetRenderer(), ptPos);
	}
}

void lwSelectStage::SetElemSlot(int Slot)
{ 
	m_iSetElem = Slot;
	if( g_pkWorld )
	{
		m_fStartTime = g_pkWorld->GetAccumTime();
		m_fEffectTime = 1.0f;
		m_fEndTime = m_fStartTime + m_fEffectTime;

		kElementCont::iterator	iter = m_kElementCont.find(m_iSetElem);
		if( iter != m_kElementCont.end() )
		{
			m_kStartPosCont.clear();
			m_kStartPosCont.insert(m_kElementCont.begin(), m_kElementCont.end());
			m_fStartRadian = iter->second;
			m_bIsEffect = false;
			g_kSoundMan.ResetAudioByID("Make_Character");

			lwUIWnd pTemp = lwGetUIWnd("CharacterRotate");
			if(pTemp.IsNil())
			{
				return;
			}
			lwUIWnd pSelectEff = pTemp.GetControl("FRM_SELECT_EFFECT");
			if( !pSelectEff.IsNil() )
			{
				pSelectEff.Visible(false);
			}
		}
	}
}

int lwSelectStage::GetFiveElement()
{
	if(NULL == m_pkSelectStage) { return 0; }

	return m_pkSelectStage->GetFiveElement();
}

int lwSelectStage::LeafAnimation(lwUIWnd UIParent)
{
	if(!g_pkWorld)
	{
		return m_eLeafState;
	}
	XUI::CXUI_Wnd* pUIParent = UIParent.GetSelf();

	float fLEAF_TRANS_TIME = LEAF_TRANS_TIME;
	fLEAF_TRANS_TIME-=(static_cast<float>(XUIMgr.GetResolutionSize().x - EXV_DEFAULT_SCREEN_WIDTH)*0.00001f);//일단 가로방향만
	
	switch(m_eLeafState)
	{
	case LAS_PADEIN:
		{
			if( (g_pkWorld->GetAccumTime() - m_fLeafStartTime) > fLEAF_TRANS_TIME )
			{
				int iMAX_LEAF_LOOP = MAX_LEAF_LOOP;
				if(pUIParent)
				{
					XUI::CXUI_Wnd* pkBG = pUIParent->GetControl(L"SFRM_ITEM_BG");
					if(pkBG)
					{
						XUI::CXUI_Builder* pkBld = dynamic_cast<XUI::CXUI_Builder*>(pkBG->GetControl(L"BLD_LEAF"));
						if(pkBld)
						{
							iMAX_LEAF_LOOP = pkBld->CountX() + pkBld->CountY();
						}
					}
				}
				
				m_fLeafStartTime = g_pkWorld->GetAccumTime();
				if( m_iLeafCount >= iMAX_LEAF_LOOP )
				{
					m_eLeafState = LAS_CHANGE;
					break;
				}
				++m_iLeafCount;

				if( SetLeafVisible(pUIParent, true) )
				{

				}
			}
		}break;
	case LAS_CHANGE:
		{
			if(lua_tinker::call<bool>("LeafChangeWorkingTime"))
			{
				m_eLeafState = LAS_PADEOUT;
			}
		}break;
	case LAS_PADEOUT:
		{
			if( (g_pkWorld->GetAccumTime() - m_fLeafStartTime) > fLEAF_TRANS_TIME )
			{
				m_fLeafStartTime = g_pkWorld->GetAccumTime();
				if( m_iLeafCount <= 0 )
				{
					m_eLeafState = LAS_CLOSE;
					break;
				}
				--m_iLeafCount;

				if( SetLeafVisible(pUIParent, false) )
				{

				}
			}
		}break;
	case LAS_CLOSE:
		{
			UIParent.Close();
			m_eLeafState = LAS_PADEIN;
		}break;
	}

	return m_eLeafState;
}

void lwSelectStage::SetLeafState(int State)
{
	m_eLeafState = (ELeafAniState)State;
	switch(m_eLeafState)
	{
	case LAS_PADEIN:
		{
			m_iLeafCount = 0;
		}break;
	case LAS_PADEOUT:
		{
			int iMAX_LEAF_LOOP = MAX_LEAF_LOOP;
			XUI::CXUI_Wnd *pUIParent = XUIMgr.Get(L"SFRM_CHAR_SELECT_LEAF");
			if(pUIParent)
			{
				XUI::CXUI_Wnd* pkBG = pUIParent->GetControl(L"SFRM_ITEM_BG");
				if(pkBG)
				{
					XUI::CXUI_Builder* pkBld = dynamic_cast<XUI::CXUI_Builder*>(pkBG->GetControl(L"BLD_LEAF"));
					if(pkBld)
					{
						iMAX_LEAF_LOOP = pkBld->CountX() + pkBld->CountY();
					}
				}
			}
			m_iLeafCount = iMAX_LEAF_LOOP;
		}break;
	}
}

int lwSelectStage::GetLeafState()
{
	return m_eLeafState;
}

bool lwSelectStage::SetLeafVisible(XUI::CXUI_Wnd*& pWnd, bool bIsVisible)
{
	BM::vstring	kString(L"SFRM_ITEM_BG");
	XUI::CXUI_Wnd* pItemBg = pWnd->GetControl((std::wstring const&)kString);
	if( pItemBg )
	{ 
		int iLEAF_X_VALUE = LEAF_X_VALUE;
		int iLEAF_Y_VALUE = LEAF_Y_VALUE;

		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pItemBg->GetControl(L"BLD_LEAF"));
		if(pkBuild)
		{
			iLEAF_X_VALUE = pkBuild->CountX();
			iLEAF_Y_VALUE = pkBuild->CountY();
		}
		int iStartX = (m_iLeafCount > iLEAF_Y_VALUE)?(m_iLeafCount - iLEAF_Y_VALUE):(0);
		int iStartY = (m_iLeafCount > iLEAF_Y_VALUE)?(iLEAF_Y_VALUE - 1):(m_iLeafCount);
		for(int i = 0, j = iStartY - i; j >= 0; --j, ++i)
		{
			if( (i + iStartX) == iLEAF_X_VALUE)
			{
				break;
			}

			kString = L"SFRM_ICON";
			kString += (i + iStartX) + j * iLEAF_X_VALUE;

			XUI::CXUI_Wnd* pLeaf = pItemBg->GetControl((std::wstring const&)kString);
			if( pLeaf )
			{
				pLeaf->Visible(bIsVisible);
			}
		}
	}
	return true;
}

void lwSelectStage::CallDeleteCharacterUI()
{// 캐릭터 삭제 UI를 호출
	// 선택된 캐릭터 GUID를 얻어옴
	if(NULL == m_pkSelectStage) { return; }

	BM::GUID const kSelectedCharaterGUID = g_kSelectStage.GetSelectedActor();
	m_pkSelectStage->SetDeleteCharacterInfo(kSelectedCharaterGUID);
}

void lwSelectStage::DeleteCharacterIfAble(lwUIWnd UISelf)
{// 캐릭터 삭제가 가능하다면 캐릭터를 삭제
	if( UISelf.IsNil() )
	{
		return;
	}

	lwUIWnd Parent = UISelf.GetParent();
	if( Parent.IsNil() )
	{
		return;
	}

	if( false == GetIsSuitableInputForDeleteCharacter() )
	{// 캐릭터 삭제에 적당한 문장이 입력되지 않았다면 종료
		return;
	}

	// 적당한 문장이 입력 되었다면 선택된 캐릭터의 GUID로 캐릭터 삭제 패킷을 만들어 보낸다.
	BM::GUID kGuid = Parent.GetCustomDataAsGuid().GetGUID();
	BM::Stream kPacket(PT_C_S_REQ_DELETE_CHARACTER, kGuid);
	NETWORK_SEND( kPacket );
}

void lwSelectStage::CallDeleteCharacterIfAble(lwUIWnd UISelf)
{
	if( UISelf.IsNil() )
	{
		return;
	}

	if( false == GetIsSuitableInputForDeleteCharacter() )
	{// 캐릭터 삭제에 적당한 문장이 입력되지 않았다면 종료
		lwCallUI("WrongInputDeleteConfirm");
		lwCloseUI("DeleteConfirm");
	}
	else
	{
		lwUIWnd kParent = UISelf.GetParent();
		if( false == kParent.IsNil() )
		{
			XUI::CXUI_Wnd* pDlg = XUIMgr.Call(L"AskOneMoreDeleteConfirm", true);
			if( pDlg )
			{
				BM::GUID const kSelectedCharaterGUID = kParent.GetCustomDataAsGuid().GetGUID();
				pDlg->SetCustomData(&kSelectedCharaterGUID, sizeof(kSelectedCharaterGUID));
			}
		}
	}
}

bool lwSelectStage::GetIsActiveDeleteConfirmEditBox()
{// edit 박스에 입력된 문장이 있어 활성화 되었는지 확인

	XUI::CXUI_Wnd* pParentWnd = XUIMgr.Get( _T("DeleteConfirm") );
	if( NULL == pParentWnd ) 
	{
		return false;
	}

	XUI::CXUI_Edit* const pkEdt = dynamic_cast<XUI::CXUI_Edit *>( pParentWnd->GetControl(_T("EDITBOX_CERTIFICATE_STRING")) );
	if( NULL == pkEdt ) 
	{
		return false;
	}

	std::wstring const& kEditText = pkEdt->EditText();
	
	if( false == kEditText.empty() )
	{// edit 박스가 비어있지 않다면 true
		return true;
	}

	//비어 있다면 false
	return false;
}

bool lwSelectStage::GetIsSuitableInputForDeleteCharacter()
{// edit 박스에 캐릭터 삭제에 적당한 문장이 입력되었는가 확인

	// 캐릭터 삭제시 입력해야 하는 문장을 TextTable으로 부터 얻어옴
	static unsigned int const TEXTTABLE_ID = 41503;
	static std::wstring const SUITABLE_DELETE_STR = TTW(TEXTTABLE_ID);
	
	XUI::CXUI_Wnd* pParentWnd = XUIMgr.Get( _T("DeleteConfirm") );
	if( NULL == pParentWnd ) 
	{
		return false;
	}

	XUI::CXUI_Edit* const pkEdt = dynamic_cast<XUI::CXUI_Edit *>( pParentWnd->GetControl(_T("EDITBOX_CERTIFICATE_STRING")) );
	if( NULL == pkEdt ) 
	{
		return false;
	}

	std::wstring const& kEditText = pkEdt->EditText();
	if( SUITABLE_DELETE_STR == kEditText )
	{// edit 박스의 문장이 삭제 확인 문장과 같다면, true
		return true;
	}

	// 아니면 false
	return false;
}
bool lwSelectStage::ModifyNameForNationCode(std::wstring& kStrName)
{
	if(kStrName.empty()) 
	{
		return false;
	}	
	switch(g_kLocal.ServiceRegion())
	{
	case LOCAL_MGR::NC_EU:
	//case LOCAL_MGR::NC_USA:
		{// 유럽에서는 캐릭터 이름을
			BM::vstring::ConvToLWR(kStrName);
			std::wstring::iterator itor = kStrName.begin();
			wchar_t wcRevise = (L'A' - L'a');
			while(itor != kStrName.end())
			{// 첫 영문자는 대문자로, 나머지는 소문자로 함
				wchar_t& wCh = (*itor);
				if(wCh >= L'a' && wCh <= L'z')
				{
					wCh += wcRevise;
					return true;					
				}
				++itor;
			}
		}break;
	default:
		{
		}break;
	}
	return false;
}

void lwSelectStage::CallSelectStyle_Hair(lwUIWnd UISelf)
{
	if( UISelf.IsNil() )
	{
		return;
	}

	if(NULL==m_pkSelectStage)
	{
		return;
	}

	if(NULL==m_pkSelectStage->GetNewActor())
	{
		return;
	}

	if(NULL==m_pkSelectStage->GetNewActor()->GetPilot())
	{
		return;
	}

	int const iType = 2;
	int const iClass = m_pkSelectStage->GetNewActor()->GetPilot()->GetBaseClassID();
	int const iSetNo = m_pkSelectStage->GetBaseItemSetNo(iClass);
	int const iGender = m_pkSelectStage->GetNewActor()->GetPilot()->GetAbil(AT_GENDER);

	int iCount = 0;
	while(iCount < 3)
	{
		BM::vstring kBuildText(L"FRM_HAIR#NUM#");
		kBuildText.Replace(L"#NUM#", iCount);
		lwUIWnd kHair = UISelf.GetControl(static_cast<std::string>(kBuildText).c_str());
		if(!kHair.IsNil() )
		{
			lwUIWnd kImg = kHair.GetControl("FRM_IMG");
			if(!kImg.IsNil())
			{				
				char *pPath = NULL;
				pPath = const_cast<char *>(GetBaseItemSetIconPath(iType, iSetNo, iGender, iCount));
				if(pPath)
				{
					kImg.ChangeImage(pPath, false);
				}
			}
		}
		++iCount;
	}
}

void lwSelectStage::CallSelectStyle_Color(lwUIWnd UISelf)
{
	if( UISelf.IsNil() )
	{
		return;
	}

	if(NULL==m_pkSelectStage)
	{
		return;
	}

	if(NULL==m_pkSelectStage->GetNewActor())
	{
		return;
	}

	if(NULL==m_pkSelectStage->GetNewActor()->GetPilot())
	{
		return;
	}

	int const iType = 0;
	int const iClass = m_pkSelectStage->GetNewActor()->GetPilot()->GetBaseClassID();
	int const iSetNo = m_pkSelectStage->GetBaseItemSetNo(iClass);
	int const iGender = 3;

	int iCount = 0;
	while(iCount < 5)
	{
		BM::vstring kBuildText(L"FRM_COLOR#NUM#");
		kBuildText.Replace(L"#NUM#", iCount);
		lwUIWnd kHair = UISelf.GetControl(static_cast<std::string>(kBuildText).c_str());
		if(!kHair.IsNil() )
		{
			lwUIWnd kImg = kHair.GetControl("FRM_IMG");
			if(!kImg.IsNil())
			{				
				char *pPath = NULL;
				//pPath = const_cast<char *>(GetBaseItemIconPath(iType, iCount));
				pPath = const_cast<char *>(GetBaseItemSetIconPath(iType, iSetNo, iGender, iCount));
				if(pPath)
				{
					kImg.ChangeImage(pPath, false);
				}
			}
		}
		++iCount;
	}
}

void lwSelectStage::CallSelectStyle_Face(lwUIWnd UISelf)
{
	if( UISelf.IsNil() )
	{
		return;
	}

	if(NULL==m_pkSelectStage)
	{
		return;
	}

	if(NULL==m_pkSelectStage->GetNewActor())
	{
		return;
	}

	if(NULL==m_pkSelectStage->GetNewActor()->GetPilot())
	{
		return;
	}

	int const iType = 4;
	int const iClass = m_pkSelectStage->GetNewActor()->GetPilot()->GetBaseClassID();
	int const iSetNo = m_pkSelectStage->GetBaseItemSetNo(iClass);
	int const iGender = m_pkSelectStage->GetNewActor()->GetPilot()->GetAbil(AT_GENDER);

	int iCount = 0;
	while(iCount < 3)
	{
		BM::vstring kBuildText(L"FRM_FACE#NUM#");
		kBuildText.Replace(L"#NUM#", iCount);
		lwUIWnd kHair = UISelf.GetControl(static_cast<std::string>(kBuildText).c_str());
		if(!kHair.IsNil() )
		{
			lwUIWnd kImg = kHair.GetControl("FRM_IMG");
			if(!kImg.IsNil())
			{				
				char *pPath = NULL;
				pPath = const_cast<char *>(GetBaseItemSetIconPath(iType, iSetNo, iGender, iCount));
				if(pPath)
				{
					kImg.ChangeImage(pPath, false);
				}
			}
		}
		++iCount;
	}
}

void lwSelectStage::ClearTestParts()
{
	if(NULL==m_pkSelectStage)
	{
		return;
	}

	if(NULL==m_pkSelectStage->GetNewActor())
	{
		return;
	}

	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT_CASH, EEquipPos::EQUIP_POS_HELMET, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT_CASH, EEquipPos::EQUIP_POS_SHIRTS, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT_CASH, EEquipPos::EQUIP_POS_PANTS, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT_CASH, EEquipPos::EQUIP_POS_SHOULDER, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT_CASH, EEquipPos::EQUIP_POS_GLOVE, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT_CASH, EEquipPos::EQUIP_POS_BOOTS, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT_CASH, EEquipPos::EQUIP_POS_WEAPON, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT_CASH, EEquipPos::EQUIP_POS_SHEILD, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT_CASH, EEquipPos::EQUIP_POS_KICKBALL, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT_CASH, EEquipPos::EQUIP_POS_CLOAK, 0);

	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT, EEquipPos::EQUIP_POS_HELMET, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT, EEquipPos::EQUIP_POS_SHIRTS, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT, EEquipPos::EQUIP_POS_PANTS, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT, EEquipPos::EQUIP_POS_SHOULDER, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT, EEquipPos::EQUIP_POS_GLOVE, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT, EEquipPos::EQUIP_POS_BOOTS, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT, EEquipPos::EQUIP_POS_WEAPON, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT, EEquipPos::EQUIP_POS_SHEILD, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT, EEquipPos::EQUIP_POS_KICKBALL, 0);
	m_pkSelectStage->GetNewActor()->UnequipItem(IT_FIT, EEquipPos::EQUIP_POS_CLOAK, 0);
}

int lwSelectStage::GetSelectedSlot()
{
	if(NULL == m_pkSelectStage) { return 0; }

	return m_pkSelectStage->GetSelectedSlot();
}


int lwSelectStage::GetEnableExtendSlot()
{
	if(NULL == m_pkSelectStage) { return 0; }

	return m_pkSelectStage->ExtendSlot();
}

void lwSelectStage::ReqEnableExtendSlot()
{
	BM::Stream kPacket(PT_C_S_REQ_FIND_CHARACTOR_EXTEND_SLOT);
	NETWORK_SEND_TO_SWITCH(kPacket);
}

bool lwSelectStage::IsCreateNewRace() const
{
	if(NULL == m_pkSelectStage) { return false; }

	return m_pkSelectStage->IsCreateNewRace();
}

lwPoint2 lwSelectStage::GetActorPosToScreen(int const iSlot)
{
	lwPoint2 kPos(0,0);
	if(NULL == m_pkSelectStage) { return kPos; }

	if( !g_pkWorld )
	{
		return kPos;
	}
	NiCameraPtr	spCamera = g_pkWorld->GetCameraMan()->GetCamera();
	if(!spCamera) 
	{
		return kPos;
	}
	NiPoint3 kSpawnLoc = m_pkSelectStage->GetSpawnPoint(iSlot);
	float fBx = 0.0f;
	float fBy = 0.0f;
	if(false == spCamera->WorldPtToScreenPt(kSpawnLoc,fBx,fBy)) 
	{
		return kPos;
	}
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	int iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);
	int const iCenterX = (int)(iScreenWidth * fBx);
	int const iCenterY = (int)(iScreenHeight * (1-fBy));
	kPos = lwPoint2(iCenterX, iCenterY);
	return kPos;
}