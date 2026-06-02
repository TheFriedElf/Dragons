#include "StdAfx.h"
#include "ServerLib.h"
#include "PgNetwork.h"
#include "lwUI.h"
#include "lwGUID.h"
#include "variant/PgCharacterCard.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "lwCharacterCard.h"
#include "PgCommandMgr.h"
#include "lwMtoMChat.h"
#include "PgChatMgrClient.h"
#include "PgHelpSystem.h"
#include "lwHelpSystem.h"

std::wstring const CHARACTER_CARD_UI_NAME = std::wstring(L"FRM_CHAR_CARD");
int const MAX_CLEAR_CARD_UI_BUILD = 7;
int const MIN_INPUT_LIMIT_AGE = 1;
int const MAX_INPUT_LIMIT_AGE = 120;
int const MAX_DROP_LIST_VIEW_COUNT = 7;
int const CHARACTER_CARD_CREATE_ITEMNO = 50400020;
int const CHARACTER_CARD_CHANGE_ITEMNO = 50400030;

extern char const* HELP_CONDITION_CHAR_CARD;

void lwCharacterCard::RegisterWrapper(lua_State* pkState)
{
	using namespace lua_tinker;

	def(pkState, "CallCharacterCardUI", lwCallCharacterCardUI);
	def(pkState, "GetMyCharacterCardInfo", lwGetMyCharacterCardInfo);
	def(pkState, "SetMyCharacterCard", lwSetMyCharacterCard);
	def(pkState, "CallCardDropList", lwCallCardDropList);
	def(pkState, "CallCardDropListByTextWndID", lwCallCardDropListByTextWndID);
	def(pkState, "SelectedDropItem", lwSelectedDropItem);
	def(pkState, "UpdateEditInputInfo", lwUpdateEditInputInfo);
	def(pkState, "SaveCardEditInfo", lwSaveCardEditInfo);
	def(pkState, "RecommendTarget", lwRecommendTarget);
	def(pkState, "RecommendTargetInUI", lwRecommendTargetInUI);
	def(pkState, "ChangeComment", lwChangeComment);
	def(pkState, "CheckRecommendLevel", lwCheckRecommendLevel);
	def(pkState, "CheckExistCharacterCard", lwCheckExistCharacterCard);
	def(pkState, "SendMatchUser", lwSendMatchUser);
	def(pkState, "MatchDefaultInfo", lwMatchDefaultInfo);	
	def(pkState, "DisplayPopularPoint", lwDisplayPopularPoint);		
	def(pkState, "DisplayToDayPopularPoint", lwDisplayTodayPopularPoint);
	def(pkState, "OnClickSelectMatchUser", lwOnClickSelectMatchUser);	
	def(pkState, "OnOverSelectMatchUser", lwOnOverSelectMatchUser);	
	def(pkState, "OnClickManToManTalk", lwOnClickManToManTalk);	
	def(pkState, "ChangeCardOpenState", lwChangeCardOpenState);
}

void lwCharacterCard::lwCallCharacterCardUI(lwGUID kGuid)
{
	BM::Stream	kPacket(PT_C_M_REQ_CHARACTER_CARD_INFO);
	kPacket.Push(kGuid.GetGUID());
	NETWORK_SEND(kPacket);
}

void lwCharacterCard::lwGetMyCharacterCardInfo()
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		if( true == pPlayer->IsCreateCard() )
		{
			lwCallCharacterCardUI(lwGUID(pPlayer->GetID()));
		}
		else
		{
			if( !g_kCharacterCardInfo.Name().size() )
			{
				g_kCharacterCardInfo.Name(pPlayer->Name());
				g_kCharacterCardInfo.OwnerGuid(pPlayer->GetID());
			}
		}
	}
}

void lwCharacterCard::lwSetMyCharacterCard()
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}

	CallCharacterCardUI(false, UICT_NONE, g_kCharacterCardInfo, 0, pPlayer->IsCreateCard());
}

void lwCharacterCard::lwCallCardDropList(lwUIWnd UISelf, char const* pDropListName)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}
	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}
	int const iBuildIndex = pSelf->BuildIndex();

	BM::vstring vText(L"FRM_TEXT");
	if( iBuildIndex > 3 )
	{
		vText += L"_EX";
	}
	vText += iBuildIndex;

	XUI::CXUI_Wnd* pTextWnd = pParent->GetControl(vText);
	if( !pTextWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pDropWnd = XUIMgr.Activate(UNI(pDropListName));
	if( !pDropWnd || pDropWnd->IsClosed() )
	{
		return;
	}
	
	XUI::CXUI_List* pDropList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
	if( !pDropList )
	{
		return;
	}
	XUI::CXUI_Wnd* pListBorder = pDropWnd->GetControl(L"SFRM_BORDER");
	if( !pListBorder )
	{
		return;
	}

	pDropWnd->Location(pTextWnd->TotalLocation().x, pTextWnd->TotalLocation().y + pTextWnd->Size().y);
	InitDropListItem(pDropList, static_cast<EDROP_LIST_INIT_TYPE>(iBuildIndex));
	pDropWnd->Size(POINT2(pDropWnd->Size().x, pDropList->Size().y));
	pListBorder->Size(POINT2(pListBorder->Size().x, pDropList->Size().y + 4));
}

void lwCharacterCard::lwCallCardDropListByTextWndID(lwUIWnd UISelf, char const* pTextWndID, char const* pDropListName, int const InitType)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}
	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}
	int const iBuildIndex = pSelf->BuildIndex();

	XUI::CXUI_Wnd* pTextWnd = pParent->GetControl(UNI(pTextWndID));
	if( !pTextWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pDropWnd = XUIMgr.Activate(UNI(pDropListName));
	if( !pDropWnd || pDropWnd->IsClosed() )
	{
		return;
	}
	
	XUI::CXUI_List* pDropList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
	if( !pDropList )
	{
		return;
	}
	XUI::CXUI_Wnd* pListBorder = pDropWnd->GetControl(L"SFRM_BORDER");
	if( !pListBorder )
	{
		return;
	}

	pDropWnd->Location(pTextWnd->TotalLocation().x, pTextWnd->TotalLocation().y + pTextWnd->Size().y);
	InitDropListItem(pDropList, static_cast<EDROP_LIST_INIT_TYPE>(InitType));
	pDropWnd->Size(POINT2(pDropWnd->Size().x, pDropList->Size().y));
	pListBorder->Size(POINT2(pListBorder->Size().x, pDropList->Size().y + 4));
}

void lwCharacterCard::lwSelectedDropItem(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	SDROP_ITEM_INFO	kInfo;
	if( pSelf->GetCustomDataSize() )
	{
		pSelf->GetCustomData(&kInfo, sizeof(SDROP_ITEM_INFO));
	}
	else
	{
		return;
	}

	bool bIsEquipUI = false;
	//얻어온 정보를 적용 시켜라
	XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(CHARACTER_CARD_UI_NAME);
	if( !pMainUI )
	{
		pMainUI = XUIMgr.Get(L"CharInfo");
		if( !pMainUI )
		{
			return;
		}

		pMainUI = pMainUI->GetControl(L"FRM_CHAR_CARD");
		if( !pMainUI )
		{
			return;
		}
		bIsEquipUI = true;
	}

	XUI::CXUI_Wnd* pEditUI = NULL;
	if( !bIsEquipUI )
	{
		pEditUI = pMainUI->GetControl(L"FRM_CHAR_CARD_EDIT");
		if( !pEditUI )
		{
			return;
		}
	}

	switch( kInfo.Type )
	{
	case ECARDUT_SEX:
		{
			std::wstring kFormName(L"FRM_SEX");
			if( bIsEquipUI )
			{
				kFormName = L"SFRM_SEX";
			}

			XUI::CXUI_Wnd* pMainText = pMainUI->GetControl(kFormName);
			if( !pMainText )
			{
				return;
			}
			pMainText->Text(TTW(600500 + kInfo.KeyValue));
			pMainText->SetCustomData(&kInfo.KeyValue, sizeof(kInfo.KeyValue));

			if( bIsEquipUI ){ break; }

			BM::vstring kTextWnd(L"FRM_TEXT");
			kTextWnd += static_cast<int>(kInfo.Type);
			XUI::CXUI_Wnd* pEditText = pEditUI->GetControl(kTextWnd);
			if( !pEditText )
			{
				return;
			}
			pEditText->Text(pMainText->Text());
		}break;
	case ECARDUT_LOC:
		{
			std::wstring kFormName(L"FRM_REGION");
			if( bIsEquipUI )
			{
				kFormName = L"SFRM_REGION";
			}

			XUI::CXUI_Wnd* pMainText = pMainUI->GetControl(kFormName);
			if( !pMainText )
			{
				return;
			}
			pMainText->Text(GetCardLocalString(kInfo.KeyValue));
			pMainText->SetCustomData(&kInfo.KeyValue, sizeof(kInfo.KeyValue));

			if( bIsEquipUI ){ break; }

			BM::vstring kTextWnd(L"FRM_TEXT");
			kTextWnd += static_cast<int>(kInfo.Type);
			XUI::CXUI_Wnd* pEditText = pEditUI->GetControl(kTextWnd);
			if( !pEditText )
			{
				return;
			}
			pEditText->Text(pMainText->Text());
		}break;
	case ECARDUT_BLOODTYPE:
		{
			XUI::CXUI_Wnd* pMainText = pMainUI->GetControl(L"FRM_BLOOD");
			BM::vstring kTextWnd(L"FRM_TEXT");
			kTextWnd += static_cast<int>(kInfo.Type);
			XUI::CXUI_Wnd* pEditText = pEditUI->GetControl(kTextWnd);
			if( !pMainText || !pEditText )
			{
				return;
			}
			pMainText->Text(GetCardKeyString(CAKT_BLOOD, kInfo.KeyValue));
			pMainText->SetCustomData(&kInfo.KeyValue, sizeof(kInfo.KeyValue));
			pEditText->Text(pMainText->Text());
		}break;
	case ECARDUT_CONSTELLATION:
		{
			XUI::CXUI_Wnd* pMainText = pMainUI->GetControl(L"FRM_CONSTELLATION");
			BM::vstring kTextWnd(L"FRM_TEXT_EX");
			kTextWnd += static_cast<int>(kInfo.Type);
			XUI::CXUI_Wnd* pEditText = pEditUI->GetControl(kTextWnd);
			if( !pMainText || !pEditText )
			{
				return;
			}
			pMainText->Text(GetCardKeyString(CAKT_CONSTELLATION, kInfo.KeyValue));
			pMainText->SetCustomData(&kInfo.KeyValue, sizeof(kInfo.KeyValue));
			pEditText->Text(pMainText->Text());
		}break;
	case ECARDUT_INTEREST:
		{
			XUI::CXUI_Wnd* pMainText = pMainUI->GetControl(L"FRM_INTEREST");
			BM::vstring kTextWnd(L"FRM_TEXT_EX");
			kTextWnd += static_cast<int>(kInfo.Type);
			XUI::CXUI_Wnd* pEditText = pEditUI->GetControl(kTextWnd);
			if( !pMainText || !pEditText )
			{
				return;
			}
			pMainText->Text(GetCardKeyString(CAKT_HOBBY, kInfo.KeyValue));
			pMainText->SetCustomData(&kInfo.KeyValue, sizeof(kInfo.KeyValue));
			pEditText->Text(pMainText->Text());
		}break;
	case ECARDUT_STYLE:
		{
			XUI::CXUI_Wnd* pMainText = pMainUI->GetControl(L"FRM_STYLE");
			BM::vstring kTextWnd(L"FRM_TEXT_EX");
			kTextWnd += static_cast<int>(kInfo.Type);
			XUI::CXUI_Wnd* pEditText = pEditUI->GetControl(kTextWnd);
			if( !pMainText || !pEditText )
			{
				return;
			}
			pMainText->Text(GetCardKeyString(CAKT_STYLE, kInfo.KeyValue));
			pMainText->SetCustomData(&kInfo.KeyValue, sizeof(kInfo.KeyValue));
			pEditText->Text(pMainText->Text());
		}break;
	default:
		{
			//pList->Close();
			return;
		}
	}
}

void lwCharacterCard::lwUpdateEditInputInfo(lwUIWnd UISelf, int const UpdateType)
{
	XUI::CXUI_Edit* pSelf = dynamic_cast<XUI::CXUI_Edit*>(UISelf.GetSelf());
	if( !pSelf )
	{
		return;
	}

	switch( UpdateType )
	{
	case ECARDUT_AGE:
		{
			int const iAge = _wtoi(pSelf->EditText().c_str());
			if( MIN_INPUT_LIMIT_AGE > iAge
			||  MAX_INPUT_LIMIT_AGE < iAge )
			{
				pSelf->EditText(L"");
				wchar_t szTemp[MAX_PATH] = {0,};
				swprintf_s(szTemp, MAX_PATH, TTW(51032).c_str(), MIN_INPUT_LIMIT_AGE, MAX_INPUT_LIMIT_AGE);
				lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(szTemp), true);
				return;
			}
		}break;
	case ECARDUT_COMMENT:
		{
			std::wstring kInputText = pSelf->EditText();
			if(g_kClientFS.Filter(kInputText, false)
				|| !g_kUnicodeFilter.IsCorrect(UFFC_CHAR_CARD_COMMENT, kInputText)
				)	//욕설 등이 있으면
			{
				pSelf->EditText(L"");
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 600037, true);
				return;
			}
		}break;
	default:
		{
			//pList->Close();
			return;
		}
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	if( pParent->ID().compare(L"SFRM_AGE") == 0 )
	{
		return;
	}

	XUI::CXUI_Wnd* pMainUI = pParent->Parent();
	if( !pMainUI )
	{
		return;
	}

	XUI::CXUI_Wnd* UpdateTargetUI = NULL;
	switch( UpdateType )
	{
	case ECARDUT_AGE:
		{
			UpdateTargetUI = pMainUI->GetControl(L"FRM_AGE");
		}break;
	case ECARDUT_COMMENT:
		{
			UpdateTargetUI = pMainUI->GetControl(L"FRM_COMMENT");
		}break;
	default:
		{
			//pList->Close();
			return;
		}
	}

	if( !UpdateTargetUI )
	{
		return;
	}
	UpdateTargetUI->Text(pSelf->EditText());
}

void lwCharacterCard::lwSaveCardEditInfo(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	XUI::CXUI_Wnd* pMainUI = pParent->Parent();
	if( !pMainUI )
	{
		return;
	}

	EUseItemCustomType Type = UICT_NONE;
	if( pMainUI->GetCustomDataSize() )
	{
		pMainUI->GetCustomData(&Type, sizeof(Type));
	}

	XUI::CXUI_Wnd* pSex = pMainUI->GetControl(L"FRM_SEX");
	XUI::CXUI_Wnd* pAge = pMainUI->GetControl(L"FRM_AGE");
	XUI::CXUI_Wnd* pRegion = pMainUI->GetControl(L"FRM_REGION");
	XUI::CXUI_Wnd* pBlood = pMainUI->GetControl(L"FRM_BLOOD");
	XUI::CXUI_Wnd* pConstellation = pMainUI->GetControl(L"FRM_CONSTELLATION");
	XUI::CXUI_Wnd* pInterest = pMainUI->GetControl(L"FRM_INTEREST");
	XUI::CXUI_Wnd* pStyle = pMainUI->GetControl(L"FRM_STYLE");
	XUI::CXUI_Wnd* pComment = pMainUI->GetControl(L"FRM_COMMENT");
	if( !pSex || !pAge || !pRegion || !pBlood || !pConstellation || !pInterest || !pStyle || !pComment )
	{
		return;
	}

	std::wstring kNoneInputData;
	if( pAge->Text().empty() )
	{
		kNoneInputData += TTW(51005);
	}
	if( pComment->Text().empty() )
	{
		if( !kNoneInputData.empty() ){ kNoneInputData += L", "; };
		kNoneInputData += TTW(51034);
	}
	if( 0 == pSex->GetCustomDataSize() )
	{
		if( !kNoneInputData.empty() ){ kNoneInputData += L", "; };
		kNoneInputData += TTW(51004);
	}
	if( 0 == pRegion->GetCustomDataSize() )
	{
		if( !kNoneInputData.empty() ){ kNoneInputData += L", "; };
		kNoneInputData += TTW(51006);
	}
	if( 0 == pStyle->GetCustomDataSize() )
	{
		if( !kNoneInputData.empty() ){ kNoneInputData += L", "; };
		kNoneInputData += TTW(51010);
	}
	if( 0 == pBlood->GetCustomDataSize() )
	{
		if( !kNoneInputData.empty() ){ kNoneInputData += L", "; };
		kNoneInputData += TTW(51007);
	}
	if( 0 == pInterest->GetCustomDataSize() )
	{
		if( !kNoneInputData.empty() ){ kNoneInputData += L", "; };
		kNoneInputData += TTW(51009);
	}
	if( 0 == pConstellation->GetCustomDataSize() )
	{
		if( !kNoneInputData.empty() ){ kNoneInputData += L", "; };
		kNoneInputData += TTW(51008);
	}
	if( !kNoneInputData.empty() )
	{
		kNoneInputData = TTW(51031) + L"\n" + kNoneInputData;
		lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(kNoneInputData), true);
		return;
	}

	BYTE const bAge = static_cast<BYTE>(_wtoi(pAge->Text().c_str()));
	std::wstring Comment = pComment->Text();
	int iLocal = lwUIWnd(pRegion).GetCustomData<int>();
	BYTE bSex = static_cast<BYTE>(lwUIWnd(pSex).GetCustomData<int>());
	BYTE bStyle = static_cast<BYTE>(lwUIWnd(pStyle).GetCustomData<int>());
	BYTE bBlood = static_cast<BYTE>(lwUIWnd(pBlood).GetCustomData<int>());
	BYTE bInterest = static_cast<BYTE>(lwUIWnd(pInterest).GetCustomData<int>());
	BYTE bConstellation = static_cast<BYTE>(lwUIWnd(pConstellation).GetCustomData<int>());

	SItemPos kItemPos;
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		PgInventory* pInv = pPlayer->GetInven();
		if( !pInv )
		{
			return;
		}

		int const iItemNo = CHARACTER_CARD_CREATE_ITEMNO + ((Type - UICT_CREATE_CARD) * 10);
		
		if( S_OK != pInv->GetFirstItem(iItemNo, kItemPos) )
		{
			return;
		}
	}

	BM::Stream	kPacket;
	int TTWID = 0;
	switch( Type )
	{
	case UICT_CREATE_CARD:
		{
			kPacket.Push(PT_C_M_REQ_CREATE_CHARACTER_CARD);
			TTWID = 51027;
		}break;
	case UICT_MODIFY_CARD:
		{
			kPacket.Push(PT_C_M_REQ_MODIFY_CHARACTER_CARD);
			TTWID = 51028;
		}break;
	default:
		{
			pMainUI->Close();
		}break;
	}
	kPacket.Push(kItemPos);
	kPacket.Push(bAge);				//나이
	kPacket.Push(bSex);				//성별
	kPacket.Push(iLocal);			//지역
	kPacket.Push(Comment);			//자기소개
	kPacket.Push(bConstellation);	//별자리
	kPacket.Push(bInterest);		//취미
	kPacket.Push(bBlood);			//혈액형
	kPacket.Push(bStyle);			//스타일
	lwCallCommonMsgYesNoBox(MB(TTW(TTWID)), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
	pMainUI->Close();
}

void lwCharacterCard::lwRecommendTarget(lwGUID kGuid)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		int const iLevel = pPlayer->GetAbil(AT_LEVEL);
		if( ENABLE_RECOMMEND_LEVEL > iLevel )
		{
			CheckResultError(E_NOT_ENOUGH_LEVEL);
			return;
		}

		if( !pPlayer->RecommendPoint() )
		{
			CheckResultError(E_NOT_ENOUGH_RP);
			return;			
		}

		PgPilot* pTarget = g_kPilotMan.FindPilot(kGuid.GetGUID());
		if( pTarget )
		{
			wchar_t szOutText[MAX_PATH] = {0,};
			swprintf_s(szOutText, MAX_PATH, TTW(51026).c_str(), pTarget->GetName().c_str(), pPlayer->RecommendPoint());

			BM::Stream	kPacket(PT_C_M_REQ_RECOMMEND_CHARACTER);
			kPacket.Push(kGuid.GetGUID());
			lwCallCommonMsgYesNoBox(MB(szOutText), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
		}
	}
}

void lwCharacterCard::lwRecommendTargetInUI(lwUIWnd UISelf)
{
	if( UISelf.IsNil() )
	{
		return;
	}

	lwUIWnd kParent = UISelf.GetParent();
	if( kParent.IsNil() )
	{
		return;
	}

	if( kParent.GetCustomDataSize() )
	{
		lwRecommendTarget(kParent.GetCustomDataAsGuid());
	}
}

void lwCharacterCard::lwChangeComment(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent )
	{
		return;
	}

	XUI::CXUI_Edit* pEdit = dynamic_cast<XUI::CXUI_Edit*>(pParent->GetControl(L"EDT_COMMENT"));
	if( !pEdit )
	{
		return;
	}

	std::wstring kInputText = pEdit->EditText();
	if( !kInputText.empty() )
	{
		if(g_kClientFS.Filter(kInputText, false)
			|| !g_kUnicodeFilter.IsCorrect(UFFC_CHAR_CARD_COMMENT, kInputText)
			)
		{//욕설 등이 있으면
			pEdit->EditText(L"");
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 600037, true);
			return;
		}
	}
	else
	{
		return;
	}

	BM::Stream	kPacket(PT_C_M_REQ_MODIFY_CHARACTER_CARD_COMMENT);
	kPacket.Push(kInputText);
	lwCallCommonMsgYesNoBox(MB(TTW(51029)), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
}

void lwCharacterCard::lwChangeCardOpenState(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf ){ return; }

	int Type = 0;
	pSelf->GetCustomData(&Type, sizeof(Type));

	switch( Type )
	{
	case ECARDVBT_HELP:
		{
			OnClickDetailHelpView(g_kHelpSystem.ChangeConditionNameToHelpID(std::string(HELP_CONDITION_CHAR_CARD), ""));
		}break;
	case ECARDVBT_RECOMMAND:
		{
			XUI::CXUI_Wnd* pkParent = pSelf->Parent();
			if( pkParent )
			{
				lwRecommendTarget(lwGUID(pkParent->OwnerGuid()));
			}
		}break;
	case ECARDVBT_HIDE:
		{
			BM::Stream kPacket(PT_C_M_REQ_MODIFY_CHARACTER_CARD_STATE);
			kPacket.Push(!g_kCharacterCardInfo.Enable());
			lwCallCommonMsgYesNoBox(MB(TTW(51030)), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
		}break;
	default:
		{}break;
	}
}

bool lwCharacterCard::lwCheckRecommendLevel()
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{//공통코드 만들고나서
		int const iLevel = pPlayer->GetAbil(AT_LEVEL);
		if( ENABLE_RECOMMEND_LEVEL <= iLevel )
		{
			return true;
		}
	}
	return false;
}

bool lwCharacterCard::lwCheckExistCharacterCard(lwGUID kGuid)
{
	PgPilot* pPilot = g_kPilotMan.FindPilot(kGuid.GetGUID());
	if( pPilot )
	{//어떤정보를 가져오나?
		PgPlayer* pPlayer = dynamic_cast<PgPlayer*>(pPilot->GetUnit());
		if( pPlayer )
		{
			return pPlayer->IsCreateCard();
		}
	}
	return false;
}

void lwCharacterCard::lwSendMatchUser(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
	if( !pMainUI )
	{
		return;
	}

	XUI::CXUI_Wnd* pOrtherSex = pMainUI->GetControl(L"SFRM_SEX");
	XUI::CXUI_Wnd* pOrtherAge = pMainUI->GetControl(L"SFRM_AGE");
	XUI::CXUI_Wnd* pOrtherRegion = pMainUI->GetControl(L"SFRM_REGION");
	if( !pOrtherSex || !pOrtherAge || !pOrtherRegion )
	{
		return;
	}
	
	XUI::CXUI_Edit* pOrtherEdit = dynamic_cast<XUI::CXUI_Edit*>(pOrtherAge->GetControl(L"EDT_AGE"));
	if( !pOrtherEdit )
	{
		return;
	}

	BYTE bSex = static_cast<BYTE>(lwUIWnd(pOrtherSex).GetCustomData<int>());
	BYTE bAge = static_cast<BYTE>(_wtoi(pOrtherEdit->EditText().c_str()) * 0.1f);
	int iLocal = (lwUIWnd(pOrtherRegion).GetCustomData<int>());

	BM::Stream	kPacket(PT_C_M_REQ_SEARCH_MATCH_CARD);
	kPacket.Push(bSex);
	kPacket.Push(bAge);
	kPacket.Push(iLocal);
	NETWORK_SEND(kPacket);
}

void lwCharacterCard::lwMatchDefaultInfo(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pOrtherSex = pSelf->GetControl(L"SFRM_SEX");
	XUI::CXUI_Wnd* pOrtherAge = pSelf->GetControl(L"SFRM_AGE");
	XUI::CXUI_Wnd* pOrtherRegion = pSelf->GetControl(L"SFRM_REGION");
	XUI::CXUI_Button* pBtnSex = dynamic_cast<XUI::CXUI_Button*>(pSelf->GetControl(L"BTN_DROP_SEX"));
	XUI::CXUI_Button* pBtnRegion = dynamic_cast<XUI::CXUI_Button*>(pSelf->GetControl(L"BTN_DROP_REGION"));
	if( !pOrtherSex || !pOrtherAge || !pOrtherRegion || !pBtnSex || !pBtnRegion )
	{
		return;
	}

	XUI::CXUI_Edit* pOrtherEdit = dynamic_cast<XUI::CXUI_Edit*>(pOrtherAge->GetControl(L"EDT_AGE"));
	if( !pOrtherEdit )
	{
		return;
	}

	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}

	if( pPlayer->IsCreateCard() )
	{
		int iSex = (UG_MALE == g_kCharacterCardInfo.Sex())?(UG_FEMALE):(UG_MALE);
		pOrtherSex->Text(TTW(600500 + iSex));
		pOrtherSex->SetCustomData(&iSex, sizeof(iSex));
		pOrtherEdit->EditText(BM::vstring(g_kCharacterCardInfo.Year()));
		pOrtherRegion->Text(GetCardLocalString(g_kCharacterCardInfo.Local()));
		pOrtherRegion->SetCustomData(&g_kCharacterCardInfo.Local(), sizeof(g_kCharacterCardInfo.Local()));
		pBtnSex->Visible(true);
		pOrtherEdit->Visible(true);
		pBtnRegion->Visible(true);
	}
	else
	{
		pOrtherSex->Text(L"");
		pOrtherEdit->EditText(L"");
		pOrtherRegion->Text(L"");
		pBtnSex->Visible(false);
		pOrtherEdit->Visible(false);
		pBtnRegion->Visible(false);
	}
}

void lwCharacterCard::lwDisplayPopularPoint(lwUIWnd UISelf)
{
	if( UISelf.IsNil() )
	{
		return;
	}
	UISelf.GetSelf()->Text(BM::vstring(g_kCharacterCardInfo.PopularPoint()));
}

void lwCharacterCard::lwDisplayTodayPopularPoint(lwUIWnd UISelf)
{
	if( UISelf.IsNil() )
	{
		return;
	}
	UISelf.GetSelf()->Text(BM::vstring(g_kCharacterCardInfo.TodayPopularPoint()));
}

void lwCharacterCard::lwOnClickSelectMatchUser(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf ){ return; }

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pSelf->Parent());
	if( !pList ){ return; }

	XUI::SListItem* pItem = pList->FirstItem();
	while( pItem && pItem->m_pWnd )
	{
		XUI::CXUI_Wnd* pUser = pItem->m_pWnd->GetControl(L"IMG_SELECT");
		if( pUser )
		{
			pUser->UVUpdate(1);
		}
		pItem = pList->NextItem(pItem);
	}
	XUI::CXUI_Wnd* pUser = pSelf->GetControl(L"IMG_SELECT");
	if( pUser )
	{
		pUser->UVUpdate(3);
	}

	XUI::CXUI_Wnd* pParent = pList->Parent();
	if( !pParent ){ return; }

	XUI::CXUI_Button* pReqTalk = dynamic_cast<XUI::CXUI_Button*>(pParent->GetControl(L"BTN_REQ_TALK"));
	if( pReqTalk )
	{
		pReqTalk->Disable(false);
		pReqTalk->OwnerGuid(pSelf->OwnerGuid());
	}
}

void lwCharacterCard::lwOnOverSelectMatchUser(lwUIWnd UISelf)
{
	XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
	if( !pSelf ){ return; }

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pSelf->Parent());
	if( !pList ){ return; }

	XUI::SListItem* pItem = pList->FirstItem();
	while( pItem && pItem->m_pWnd )
	{
		XUI::CXUI_Wnd* pUser = pItem->m_pWnd->GetControl(L"IMG_SELECT");
		if( pUser && 3 > pUser->UVInfo().Index )
		{
			pUser->UVUpdate(1);
		}
		pItem = pList->NextItem(pItem);
	}
	XUI::CXUI_Wnd* pUser = pSelf->GetControl(L"IMG_SELECT");
	if( pUser && 3 > pUser->UVInfo().Index )
	{
		pUser->UVUpdate(2);
	}
}

void lwCharacterCard::lwOnClickManToManTalk(lwUIWnd UISelf)
{
	XUI::CXUI_Button* pSelf = dynamic_cast<XUI::CXUI_Button*>(UISelf.GetSelf());
	if( !pSelf ){ return; };

	XUI::CXUI_Wnd* pParent = pSelf->Parent();
	if( !pParent ){ return; };

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pParent->GetControl(L"LST_MATCH_INFO"));
	if( !pList ){ return; }

	XUI::SListItem* pItem = pList->FirstItem();
	while( pItem && pItem->m_pWnd )
	{
		XUI::CXUI_Wnd* pUser = pItem->m_pWnd->GetControl(L"IMG_SELECT");
		if( pUser )
		{
			pUser->UVUpdate(1);
		}
		pItem = pList->NextItem(pItem);
	}

	pSelf->Disable(true);
	MToMChatNewDlg( pSelf->OwnerGuid(), 0 );
}

void lwCharacterCard::ReceivePacket_Command(WORD const wPacketType, BM::Stream& kPacket)
{
	switch( wPacketType )
	{
	case PT_M_C_NOTI_CREATE_CARD:
		{
			BM::GUID kGuid;
			kPacket.Pop(kGuid);
			PgPilot * pkPilot = g_kPilotMan.FindPilot(kGuid);
			if(pkPilot)
			{
				PgPlayer * pkPlayer = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());
				if(pkPlayer)
				{
					pkPlayer->IsCreateCard(true);
				}
			}
		}break;
	case PT_M_C_ANS_CREATE_CHARACTER_CARD:
		{
			HRESULT	kResult;
			kPacket.Pop(kResult);
			if( CheckResultError(kResult) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 51051, true);
				PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
				if( pPlayer )
				{
					pPlayer->IsCreateCard(true);
					lwCallCharacterCardUI(lwGUID(pPlayer->GetID()));
				}
			}
		}break;
	case PT_M_C_ANS_MODIFY_CHARACTER_CARD:
		{
			HRESULT	kResult;
			kPacket.Pop(kResult);
			if( CheckResultError(kResult) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 51052, true);
				PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
				if( pPlayer )
				{
					lwCallCharacterCardUI(lwGUID(pPlayer->GetID()));
				}
			}
		}break;
	case PT_M_C_ANS_MODIFY_CHARACTER_CARD_COMMENT:
		{
			HRESULT	kResult;
			kPacket.Pop(kResult);
			if( CheckResultError(kResult) )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 51053, true);
			}
		}break;
	case PT_M_C_ANS_RECOMMEND_CHARACTER:
		{
			ECharacterCardRecommendType kType;
			kPacket.Pop(kType);
			switch(kType)
			{
			case CCRT_RECOMMEND:
				{
					HRESULT	kResult = E_FAIL;
					std::wstring kTarget;
					int iRecommandPoint = 0;
					kPacket.Pop(kResult);
					kPacket.Pop(kTarget);
					kPacket.Pop(iRecommandPoint);
					if( CheckResultError(kResult) )
					{
						PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
						if( pPlayer )
						{
							wchar_t szResult[MAX_PATH] = {0,};
							swprintf_s(szResult, MAX_PATH, TTW(51054).c_str(), kTarget.c_str(), iRecommandPoint);
							lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(szResult), true);
						}
					}
				}break;
			case CCRT_POPULARER:
				{
					std::wstring kOwner;
					int iEffectNum = 0;
					kPacket.Pop(kOwner);
					kPacket.Pop(iEffectNum);

					BM::vstring vstrText(TTW(51057)); 
					vstrText.Replace( L"#Name#", kOwner);
					g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT), vstrText, true, EL_Notice4, true, false);

					lwActor kActor(g_kPilotMan.GetPlayerActor());
					if(0 == iEffectNum)
					{
						kActor.AttachParticleS(10000, "p_ef_star", "ef_CharacterCard_1step", 1);
					}
					else
					{
						kActor.AttachParticleS(10000, "p_ef_star", "ef_CharacterCard_2step", 1);
					}
				}break;
			}
		}break;
	case PT_M_C_ANS_MODIFY_CHARACTER_CARD_STATE:
		{
			HRESULT	kResult;
			kPacket.Pop(kResult);
			CheckResultError(kResult);
		}break;
	case PT_M_C_ANS_CHARACTER_CARD_INFO:
		{
			BM::GUID kGuid;
			HRESULT	kResult;
			PgCharacterCard	kCard;
			kPacket.Pop(kGuid);
			kPacket.Pop(kResult);
			kCard.ReadFromPacket(kPacket);

			if( g_kPilotMan.IsMyPlayer(kGuid) )
			{
				if( EC_OK == kResult )
				{
					g_kCharacterCardInfo = kCard;
				}
			}
			else
			{
				CallCharacterCardUI(false, UICT_NONE, kCard, 0, (EC_OK == kResult));
			}
		}break;
	case PT_M_C_ANS_SEARCH_MATCH_CARD:
		{
			CONT_MATCH_CARD_INFO kMatchInfo;
			PU::TLoadArray_M(kPacket, kMatchInfo);
			SetMatchListInfo(kMatchInfo);
		}break;
	default:
		{//정의되지 않은 타입~
		}break;
	}
}

bool lwCharacterCard::CheckResultError(HRESULT const Result)
{
	switch( Result )
	{
	case EC_OK:{ return true; } break;
	case E_NOT_FOUND_CARD:
	case E_NOT_ENOUGH_RP:
	case E_DISABLE_CARD:
	case E_NOT_ENOUGH_LEVEL:
	case E_BADSTRING:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 51061 + (Result - E_NOT_FOUND_CARD), true);
		}break;
	default:
		{
		}break;
	}
	return false;
}

void lwCharacterCard::CallCharacterCardUI(bool const bEdit, EUseItemCustomType const Type, PgCharacterCard const& kCardInfo, int const ResourceNo, bool const bExist)
{
	XUI::CXUI_Wnd* pTabUI = NULL;
	XUI::CXUI_Wnd* pMainUI = NULL;

	if( false == bEdit )
	{
		if( g_kPilotMan.IsMyPlayer(kCardInfo.OwnerGuid()) )
		{
			XUI::CXUI_Wnd* pEquip = XUIMgr.Get(L"CharInfo");
			if( !pEquip ){ return; }

			pTabUI = pEquip->GetControl(L"FRM_CHAR_CARD");
			if( !pTabUI ){ return; }

			XUI::CXUI_Wnd* pCover = pTabUI->GetControl(L"FRM_NOCARD");
			if( pCover )
			{
				pCover->Visible(bExist);
			}

			pMainUI = pTabUI->GetControl(L"FRM_CARD_INFO");
		}
		else
		{
			XUI::CXUI_Wnd* pEquip = XUIMgr.Get(L"SFRM_OTHER_CharInfo");
			if( !pEquip )
			{
				lwViewOtherEquip(kCardInfo.OwnerGuid());

				pEquip = XUIMgr.Get(L"SFRM_OTHER_CharInfo");
				if( !pEquip ){ return; }

				XUI::CXUI_Wnd* pkBtnBg = pEquip->GetControl(L"SFRM_TAB1");
				if( !pkBtnBg ){ return; }

				XUI::CXUI_CheckButton* pkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkBtnBg->GetControl(L"CBTN_TAB"));
				if( !pkBtn ){ return; }

				pkBtn->DoScript(SCRIPT_ON_L_BTN_DOWN);
			}

			pTabUI = pEquip->GetControl(L"FRM_CHAR_CARD");
			if( !pTabUI ){ return; }

			pMainUI = pTabUI->GetControl(L"FRM_CARD_INFO");
		}
	}
	else
	{
		pMainUI = XUIMgr.Call(CHARACTER_CARD_UI_NAME);
	}

	//캐릭터카드 정보가 없으면 표시하지 않음
	if(pTabUI)
	{
		XUI::CXUI_Wnd* pBg = pTabUI->GetControl(L"FRM_BG");
		if(pBg) 
		{
			XUI::CXUI_Wnd* pInfo = pBg->GetControl(L"FRM_MATCH_INFO_BG_SD");
			if(pInfo)
			{
				pInfo->Visible(bExist);				
			}
		}
		XUI::CXUI_Wnd* pBtn = pTabUI->GetControl(L"BTN_SEARCH");
		if(pBtn)
		{
			pBtn->Enable(bExist);
		}
	}


	if( !pMainUI && (false == bEdit && !pTabUI) )
	{
		return;
	}

	bool const bIsMyInfoView = (false == bEdit && g_kPilotMan.IsMyPlayer(kCardInfo.OwnerGuid()) )?(true):(false);

	if( false == bEdit )
	{
		pTabUI->OwnerGuid(kCardInfo.OwnerGuid());
		pMainUI->Visible(bExist);
		XUI::CXUI_Wnd* pCover = pTabUI->GetControl(L"FRM_NOCARD");
		if( pCover )
		{
			pCover->Visible(!bExist);
		}

		XUI::CXUI_Wnd* pButton = dynamic_cast<XUI::CXUI_Button*>(pTabUI->GetControl(L"BTN_ACTION"));
		if( pButton )
		{
			pButton->Visible(true);
			ECARD_VIEW_BTN_TYPE	Type;
			if( false == bExist )
			{
				pButton->Text(TTW(51036));
				Type = ECARDVBT_HELP;
				pButton->SetCustomData(&Type, sizeof(Type));
				return;
			}
			else
			{
				if( bIsMyInfoView )
				{
					pButton->Visible(false);
					pButton->Text(TTW(kCardInfo.Enable()?(51020):(51017)));
					Type = ECARDVBT_HIDE;
				}
				else
				{
					pButton->Text(TTW(51025));
					Type = ECARDVBT_RECOMMAND;
				}
			}
			pButton->SetCustomData(&Type, sizeof(Type));
		}
	}

	XUI::CXUI_Wnd* pMainBg = pMainUI->GetControl(L"FRM_BG");
	if( pMainBg )
	{
		int const BGResourceNo = (true == bEdit)?(ResourceNo):(kCardInfo.BGndNo());
		if (BGResourceNo < 0 || BGResourceNo >= 506504708)
		{
			pMainBg->DefaultImgName(L"../Data/6_ui/card/infoCdbg.tga");
			return;
		}
		else
		{
			const CONT_DEFRES* pContDefRes = NULL;
			g_kTblDataMgr.GetContDef(pContDefRes);
			CONT_DEFRES::const_iterator itrDest = pContDefRes->find(BGResourceNo);
			if(itrDest != pContDefRes->end())
			{
				pMainBg->DefaultImgName(itrDest->second.strXmlPath);
			}
		}
	}

	//여기는 상단을 채우고
	XUI::CXUI_Wnd* pName = pMainUI->GetControl(L"FRM_NAME");
	XUI::CXUI_Wnd* pSex = pMainUI->GetControl(L"FRM_SEX");
	XUI::CXUI_Wnd* pAge = pMainUI->GetControl(L"FRM_AGE");
	XUI::CXUI_Wnd* pRegion = pMainUI->GetControl(L"FRM_REGION");
	XUI::CXUI_Wnd* pTotalPopular = pMainUI->GetControl(L"FRM_TOTAL_POPULAR");
	XUI::CXUI_Wnd* pPopular = pMainUI->GetControl(L"FRM_POPULAR");
	XUI::CXUI_Wnd* pBlood = pMainUI->GetControl(L"FRM_BLOOD");
	XUI::CXUI_Wnd* pConstellation = pMainUI->GetControl(L"FRM_CONSTELLATION");
	XUI::CXUI_Wnd* pInterest = pMainUI->GetControl(L"FRM_INTEREST");
	XUI::CXUI_Wnd* pStyle = pMainUI->GetControl(L"FRM_STYLE");
	XUI::CXUI_Wnd* pComment = pMainUI->GetControl((true == bIsMyInfoView)?(L"EDT_COMMENT"):(L"FRM_COMMENT"));
	if( !pName || !pSex || !pAge || !pRegion || !pTotalPopular || !pPopular || !pBlood || !pConstellation || !pInterest || !pStyle || !pComment )
	{
		return;
	}


	pName->Text(kCardInfo.Name());
	pName->ClearCustomData();
	pSex->Text((UICT_CREATE_CARD == Type)?(TTW(51023)):(TTW(600500 + kCardInfo.Sex())));
	pSex->ClearCustomData();
	pAge->Text((UICT_CREATE_CARD == Type)?(L""):(BM::vstring(kCardInfo.Year())));
	pRegion->Text((UICT_CREATE_CARD == Type)?(TTW(51023)):(GetCardLocalString(kCardInfo.Local())));
	pRegion->ClearCustomData();
	pTotalPopular->Text(BM::vstring(kCardInfo.PopularPoint()));
	pTotalPopular->ClearCustomData();
	pPopular->Text(BM::vstring(kCardInfo.TodayPopularPoint()));
	pPopular->ClearCustomData();
	pBlood->Text((UICT_CREATE_CARD == Type)?(TTW(51023)):(GetCardKeyString(CAKT_BLOOD, kCardInfo.Blood())));
	pBlood->ClearCustomData();
	pConstellation->Text((UICT_CREATE_CARD == Type)?(TTW(51023)):(GetCardKeyString(CAKT_CONSTELLATION, kCardInfo.Constellation())));
	pConstellation->ClearCustomData();
	pInterest->Text((UICT_CREATE_CARD == Type)?(TTW(51023)):(GetCardKeyString(CAKT_HOBBY, kCardInfo.Hobby())));
	pInterest->ClearCustomData();
	pStyle->Text((UICT_CREATE_CARD == Type)?(TTW(51023)):(GetCardKeyString(CAKT_STYLE, kCardInfo.Style())));
	pStyle->ClearCustomData();
	if( bIsMyInfoView )
	{ 
		XUI::CXUI_Edit* pEditComment = dynamic_cast<XUI::CXUI_Edit*>(pComment);
		if( pEditComment )
		{
			pEditComment->EditText(kCardInfo.Comment());
		}
	}
	else
	{
		pComment->Text((UICT_CREATE_CARD == Type)?(L""):(kCardInfo.Comment()));
	}

	if( false == bEdit )
	{//장비창 동작은 여기서 갈리지
		if( g_kPilotMan.IsMyPlayer(kCardInfo.OwnerGuid()) )
		{
			XUI::CXUI_Wnd* pParent = pMainUI->Parent();
			if( !pParent )
			{
				return;
			}
			XUI::CXUI_Wnd* pPlus = pParent->GetControl(L"SFRM_PLUS_OPTION");
			if( pPlus )
			{
				SCHARACTER_CARD_ABIL kAbil;
				if( kCardInfo.GetCardAbil(kAbil) )	
				{
					std::wstring kText;
					if( MakeCardAbilString(kAbil.wAbilType, kAbil.iValue, kText) )
					{
						pPlus->Text(kText);
					}
				}
			}
		}
		return;
	}

	//여기는 하단을 컨트롤
	XUI::CXUI_Wnd* pRecommand = pMainUI->GetControl(L"BTN_RECOMMEND");
	XUI::CXUI_Wnd* pEditWnd = pMainUI->GetControl(L"FRM_CHAR_CARD_EDIT");
	XUI::CXUI_Wnd* pMainBG = pMainUI->GetControl(L"FRM_BG");
	if( !pRecommand || !pEditWnd || !pMainBG )
	{
		return;	
	}
	pEditWnd->Visible(bEdit);
	pRecommand->Visible(!pEditWnd->Visible());

	int iPlusHeight = pRecommand->Size().y;
	if( pEditWnd->Visible() )
	{//에디트 모드다
		ClearCardEditUI(pEditWnd);
		iPlusHeight = pEditWnd->Size().y;
		pMainUI->SetCustomData(&Type, sizeof(Type));
	}
	else
	{
		pMainUI->SetCustomData(&kCardInfo.OwnerGuid(), sizeof(kCardInfo.OwnerGuid()));
	}
	pMainUI->Size(pMainUI->Size().x, pMainBG->Size().y + iPlusHeight);
	pMainUI->Location(pMainUI->Location().x, (XUIMgr.GetResolutionSize().y - pMainUI->Size().y) * 0.5f);
}

void lwCharacterCard::ClearCardEditUI(XUI::CXUI_Wnd* pWnd)
{
	for(int i = 0; i < MAX_CLEAR_CARD_UI_BUILD; ++i)
	{
		BM::vstring vText(L"FRM_TEXT");
		if( i > 3 )
		{
			vText += L"_EX";
		}
		vText += i;

		XUI::CXUI_Wnd* pTextWnd = pWnd->GetControl(vText);
		if( !pTextWnd )
		{
			continue;
		}
		pTextWnd->Text(TTW(51004 + pTextWnd->BuildIndex()));
		pTextWnd->ClearCustomData();

		if( i == EDROPTYPE_AGE )
		{
			pTextWnd->Visible(false);
			XUI::CXUI_Wnd* pDropBtn = pWnd->GetControl(L"BTN_DROP1");
			if( pDropBtn )
			{
				pDropBtn->Visible(false);
			}
		}
	}

	XUI::CXUI_Edit* pEditWnd = dynamic_cast<XUI::CXUI_Edit*>(pWnd->GetControl(L"EDT_AGE"));
	if( pEditWnd )
	{
		pEditWnd->EditText(L"");
	}
	pEditWnd = dynamic_cast<XUI::CXUI_Edit*>(pWnd->GetControl(L"EDT_COMMENT"));
	if( pEditWnd )
	{
		pEditWnd->EditText(L"");
	}
}

void lwCharacterCard::InitDropListItem(XUI::CXUI_List* pList, EDROP_LIST_INIT_TYPE const Type)
{
	pList->DeleteAllItem();
	int iStart = 0;
	int iEnd = 0;
	switch( Type )
	{
	case EDROPTYPE_SEX:
		{
			for(int i = UG_MALE; i < UG_UNISEX; ++i)
			{
				XUI::SListItem* pItem = pList->AddItem(L"");
				if( pItem )
				{
					pItem->m_pWnd->Text(TTW(600500 + i));
					SDROP_ITEM_INFO	kInfo(Type, i);
					pItem->m_pWnd->SetCustomData(&kInfo, sizeof(kInfo));
				}
			}
		}break;
	case EDROPTYPE_LOC:
		{
			CONT_CARD_LOCAL const * pCardLocalDef = NULL;
			g_kTblDataMgr.GetContDef(pCardLocalDef);//지역정보
			if( pCardLocalDef )
			{
				CONT_CARD_LOCAL::const_iterator c_iter = pCardLocalDef->begin();
				while( c_iter != pCardLocalDef->end() )
				{
					XUI::SListItem* pItem = pList->AddItem(L"");
					if( pItem )
					{
						pItem->m_pWnd->Text(c_iter->second.kText);
						SDROP_ITEM_INFO	kInfo(Type, c_iter->second.iLocal);
						pItem->m_pWnd->SetCustomData(&kInfo, sizeof(kInfo));
					}
					++c_iter;
				}
			}
		}break;
	case EDROPTYPE_BLOODTYPE:
	case EDROPTYPE_CONSTELLATION:
	case EDROPTYPE_INTEREST:
	case EDROPTYPE_STYLE:
		{
			CONT_DEF_CARD_KEY_STRING const* pCardKeyDef = NULL;
			g_kTblDataMgr.GetContDef(pCardKeyDef);//지역정보
			if( pCardKeyDef )
			{
				ECARD_ABIL_KEY_TYPE const REAL_TYPE = static_cast<ECARD_ABIL_KEY_TYPE>((Type - EDROPTYPE_BLOODTYPE) + 1);
				TBL_DEF_CARD_KEY_STRING_KEY	kKey(REAL_TYPE, 1);
				CONT_DEF_CARD_KEY_STRING::const_iterator c_iter = pCardKeyDef->find(kKey);
				while( c_iter != pCardKeyDef->end() )
				{
					if( REAL_TYPE != c_iter->first.bKeyType )
					{
						break;
					}
					wchar_t const* pName = NULL;
					GetDefString(c_iter->second.iStringNo, pName);
					if( pName )
					{
						XUI::SListItem* pItem = pList->AddItem(L"");
						if( pItem )
						{
							pItem->m_pWnd->Text(pName);
							SDROP_ITEM_INFO	kInfo(Type, c_iter->first.bKeyValue);
							pItem->m_pWnd->SetCustomData(&kInfo, sizeof(kInfo));
						}
					}
					++c_iter;
				}
			}
		}break;
	default:
		{
			return;
		}
	}

	int const iCount = std::min(pList->GetTotalItemCount(), MAX_DROP_LIST_VIEW_COUNT);
	pList->Size(POINT2(pList->Size().x, iCount * 24));
}

bool lwCharacterCard::MakeCardAbilString(WORD wAbilType, int const iValue, std::wstring& kText)
{
	const TCHAR *pText = NULL;
	if(!GetAbilName(wAbilType, pText))
	{
		return false;
	}

	if( 0 == iValue )
	{
		return false;
	}

	switch(wAbilType)
	{
	case AT_R_CRITICAL_POWER:
		{
			return false;
		}break;//해당 어빌은 사용하지 않는다.
	case AT_R_PHY_DEFENCE:
	case AT_R_MAGIC_DEFENCE:
	case AT_R_PHY_ATTACK_MIN:
	case AT_R_PHY_ATTACK_MAX:
	case AT_R_MAGIC_ATTACK_MIN:
	case AT_R_MAGIC_ATTACK_MAX:
	case AT_R_JUMP_HEIGHT:
	case AT_R_RESIST_FIRE:
	case AT_R_RESIST_WATER:
	case AT_R_RESIST_TREE:
	case AT_R_RESIST_IRON:
	case AT_R_RESIST_EARTH:
	case AT_R_STR:
	case AT_R_INT:
	case AT_R_CON:
	case AT_R_DEX:
	case AT_R_MAX_HP:
	case AT_R_MAX_MP:
	case AT_R_HIT_SUCCESS_VALUE:
	case AT_R_MOVESPEED:
	case AT_R_BLOCK_SUCCESS_VALUE:
	case AT_R_DODGE_SUCCESS_VALUE:
	case AT_PHY_DMG_DEC:
	case AT_MAGIC_DMG_DEC:
	case AT_R_PHY_DMG_DEC:
	case AT_R_HP_RECOVERY:
	case AT_R_MP_RECOVERY:
	case AT_R_ATTACK_RANGE:
	case AT_R_ATTACK_SPEED:
	case AT_R_MAGIC_DMG_DEC:
	case AT_R_CASTING_SPEED:
	case AT_R_CRITICAL_SUCCESS_VALUE:
	case AT_I_PHY_DEFENCE_ADD_RATE:
	case AT_I_MAGIC_DEFENCE_ADD_RATE:
		{
			BM::vstring	kStr(pText);
			float fRate = iValue / (ABILITY_RATE_VALUE * 0.01f);
			kStr += L" +";
			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, _T("%.1f%%"), fRate);
			kStr += szTemp;
			kText = (std::wstring const&)kStr;
		}break;
	case AT_STR:
	case AT_INT:
	case AT_DEX:
	case AT_CON:
	case AT_MAX_HP:
	case AT_MAX_MP:
	case AT_PHY_DEFENCE:
	case AT_MAGIC_DEFENCE:
	case AT_PHY_ATTACK_MIN:// 최소 데미지
	case AT_PHY_ATTACK_MAX:// 최대 데미지
	case AT_MAGIC_ATTACK_MIN: //최소마법 공격력
	case AT_MAGIC_ATTACK_MAX://마법 최대 공격력
	case AT_MOVESPEED:
	case AT_ATTACK_SPEED:
	case AT_CRITICAL_SUCCESS_VALUE:
	case AT_HIT_SUCCESS_VALUE:
	case AT_DODGE_SUCCESS_VALUE:
	default:
		{
			BM::vstring	kStr(pText);
			kStr += L" +";
			kStr += iValue;
			kText = (std::wstring const&)kStr;
		}break;
	}
	return true;
}

void lwCharacterCard::SetMatchListInfo(CONT_MATCH_CARD_INFO& kList)
{
	XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"CharInfo");
	if( !pMainUI )
	{
		return;
	}

	XUI::CXUI_Wnd* pCharacterCard = pMainUI->GetControl(L"FRM_CHAR_CARD");
	if( !pCharacterCard )
	{
		return;
	}

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pCharacterCard->GetControl(L"LST_MATCH_INFO"));
	if( pList )
	{
		int const iMaxItemCount = pList->GetTotalItemCount();
		if( iMaxItemCount < kList.size() )
		{
			for(int i = iMaxItemCount; i < kList.size(); ++i)
			{
				pList->AddItem(L"");
			}
		}
		else if(iMaxItemCount > kList.size())
		{
			for(int i = iMaxItemCount; i > kList.size(); --i)
			{
				pList->DeleteItem(pList->FirstItem());
			}
		}

		if( kList.empty() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 51066, true);
			return;
		}

		std::sort( kList.begin(), kList.end(), lwCharacterCard::MatchListOrder);

		XUI::SListItem* pItem = pList->FirstItem();
		CONT_MATCH_CARD_INFO::const_iterator c_iter = kList.begin();
		while( c_iter != kList.end() )
		{
			if( pItem && pItem->m_pWnd )
			{
				XUI::CXUI_Wnd* pItemWnd = pItem->m_pWnd;

				XUI::CXUI_Wnd* pSex = pItemWnd->GetControl(L"ITEM_SEX");
				XUI::CXUI_Wnd* pName = pItemWnd->GetControl(L"ITEM_NAME");
				XUI::CXUI_Wnd* pAge = pItemWnd->GetControl(L"ITEM_AGE");
				XUI::CXUI_Wnd* pMatch = pItemWnd->GetControl(L"ITEM_MATCH");
				XUI::CXUI_Wnd* pSelect = pItemWnd->GetControl(L"IMG_SELECT");
				if( !pSex || !pName || !pAge || !pMatch || !pSelect )
				{
					return;
				}

				g_kChatMgrClient.Name2Guid_Add(CT_NORMAL, c_iter->kName, c_iter->kCharGuid);

				pItemWnd->OwnerGuid(c_iter->kCharGuid);
				pSex->Text(TTW(600500 + c_iter->bSex));
				pName->Text(c_iter->kName);
				pAge->Text(BM::vstring(c_iter->bAge));
				pMatch->Text(BM::vstring(c_iter->iMatchPoint));
				pItem = pList->NextItem(pItem);
				pSelect->UVUpdate(1);
			}
			++c_iter;
		}
	}
}

bool lwCharacterCard::MatchListOrder(SMATCH_CARD_INFO const& rhs, SMATCH_CARD_INFO const lhs)
{
	return ( rhs.iMatchPoint > lhs.iMatchPoint );
}

std::wstring const lwCharacterCard::GetCardLocalString(int const iKey)
{
	std::wstring kLocalStr;
	CONT_CARD_LOCAL const * pCardLocalDef = NULL;
	g_kTblDataMgr.GetContDef(pCardLocalDef);//지역정보
	if( pCardLocalDef )
	{
		CONT_CARD_LOCAL::const_iterator c_iter = pCardLocalDef->find(iKey);
		if( c_iter != pCardLocalDef->end() )
		{
			kLocalStr = c_iter->second.kText;
		}
	}
	return kLocalStr;
}

std::wstring const lwCharacterCard::GetCardKeyString(BYTE const KeyType, BYTE const KeyValue)
{
	std::wstring kKeyStr;
	CONT_DEF_CARD_KEY_STRING const* pCardKeyDef = NULL;
	g_kTblDataMgr.GetContDef(pCardKeyDef);//지역정보
	if( pCardKeyDef )
	{
		TBL_DEF_CARD_KEY_STRING_KEY	kKey(KeyType, KeyValue);
		CONT_DEF_CARD_KEY_STRING::const_iterator c_iter = pCardKeyDef->find(kKey);
		if( c_iter != pCardKeyDef->end() )
		{
			wchar_t const* pName = NULL;
			GetDefString(c_iter->second.iStringNo, pName);
			if( pName )
			{
				kKeyStr = pName;
			}
		}
	}
	return kKeyStr;
}