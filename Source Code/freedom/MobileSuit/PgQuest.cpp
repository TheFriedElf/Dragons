#include "stdafx.h"
#include "Variant/Constant.h"
#include "Variant/PgMyQuest.h"
#include "Variant/PgPlayer.h"
#include "Variant/PgQuestInfo.h"
#include "PgIXmlObject.h"
#include "lwUI.h"
#include "PgXmlLoader.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "zlib.h"
#include "FreedomPool.h"
#include "PgNetwork.h"
#include "PgQuestUI.h"
#include "PgQuestIngList.h"
#include "PgQuestDialog.h"
#include "PgQuestSelectiveList.h"
#include "PgQuestMan.h"
#include "PgUIScene.h"
#include "lwUIQuest.h"
#include "PgDailyQuestUI.h"
#include "PgHelpSystem.h"

#include "PgQuest.h"

size_t const iMaxMiniListCount = 10;
std::wstring const WSTR_QUEST_ING_ITEM_HEAD = _T("FRM_QUEST_ING_ITEM_HEAD");
std::wstring const WSTR_QUEST_ING_ITEM_FORMAT = _T("FRM_QUEST_ING_ITEM_");
std::wstring const WSTR_SFRM_QUEST_QUICK_INFO = _T("SFRM_QUEST_QUICK_INFO");
std::wstring const WSTR_SAVE_FILE_NAME( L"QUEST.DAT" );

//Local Function
void _TextOrTTW(char const* szTTW, char const* szText, std::wstring &rkText)
{
	std::wstring kTemp;
	if(NULL != szTTW && 0 != atol(szTTW))
	{
		kTemp = TTW(atol(szTTW));
	}
	else
	{
		if(NULL != szText)
		{
			kTemp = UNI(szText);
		}
		else
		{
			PG_ASSERT_LOG(0);
		}
	}
	if(0 == kTemp.size())
	{
		PG_ASSERT_LOG(0);
	}

	//break space "\n " --> "\n"
	unsigned int i = 0;
	TCHAR cTemp1 = 0;//kTemp[i-1]
	TCHAR cTemp2 = 0;//kTemp[i-2]
	while(kTemp.size() > i)
	{
		if(cTemp2==_T('\\') && cTemp1==_T('n') && kTemp[i]==_T(' '))
		{
			//nothing to do
		}
		else
		{
			rkText = rkText + kTemp[i];
		}
		cTemp2 = cTemp1;
		cTemp1 = kTemp[i];
		++i;
	}
	rkText = rkText + _T("");
}


void Net_PT_M_C_ANS_NPC_QUEST(BM::Stream *pkPacket)
{
	g_kQuestMan.ClearNPCQuestInfo();//퀘스트 정보 초기화

	int iCount = 0;
	pkPacket->Pop(iCount);//몇명분의 NPC 정보인가

	for(int i=0; iCount>i; ++i)
	{
		BM::GUID kObjectGuid;
		ContSimpleQuest kVec;

		pkPacket->Pop(kObjectGuid);
		pkPacket->Pop(kVec);//Pop

		ContSimpleQuest::iterator iter = kVec.begin();
		for(; kVec.end() != iter; ++iter)
		{
			if( QS_None != iter->cState )
			{
				SNPCQuestInfo kInfo(kObjectGuid, *iter);
				g_kQuestMan.SaveNPCQuestInfo(kInfo);
			}
		}

		PgPilot *pkPilot = g_kPilotMan.FindPilot(kObjectGuid);
		if( pkPilot )
		{
			PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			if( pkActor )
			{
				pkActor->PopSavedQuestSimpleInfo();
			}
		}
	}

	//퀘스트 정보를 미리 읽어 놓자
	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if(pkPC)
	{
		PgMyQuest const *pkMyQuest = pkPC->GetMyQuest();
		if( pkMyQuest )
		{
			ContUserQuestState kVec;
			pkMyQuest->GetQuestList(kVec);
			ContUserQuestState::iterator kQuest_iter = kVec.begin();
			for(; kVec.end() != kQuest_iter; ++kQuest_iter)
			{
				const PgQuestInfo* pkQuestInfo = g_kQuestMan.GetQuest(kQuest_iter->iQuestID);
			}
		}
	}

	// MiniMap을 위한 UpdateQuest
	g_kUIScene.UpdateQuest();
}

void Net_PT_M_C_SHOWDIALOG(BM::Stream *pkPacket)
{
	BM::GUID kSessionGuid;
	BM::GUID kGuid;
	BYTE cDialogType = 0;
	int iQuestID = 0;
	int iDialogID = 0;

	pkPacket->Pop( kSessionGuid );
	pkPacket->Pop( kGuid );
	pkPacket->Pop( cDialogType );
	pkPacket->Pop( iQuestID );
	pkPacket->Pop( iDialogID );

	g_kQuestMan.TalkSessionGuid( kSessionGuid );

	PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
	if( pkQuestInfo )
	{
		switch( pkQuestInfo->Type() )
		{
		case QT_Day:
		case QT_GuildTactics:
		case QT_Random:
		case QT_RandomTactics:
		case QT_Wanted:
			{
				DailyQuestUI::AnsShowQuestDialog(kGuid, cDialogType, iQuestID, pkQuestInfo->Type(), iDialogID);
				return;
			}break;
		}
	}

	switch( cDialogType )
	{
	case QSDT_ErrorDialog:
		{
			XUIMgr.Close(PgQuestManUtil::kWantedQuestWndName);
			XUIMgr.Close(PgQuestManUtil::kWantedQuestCompleteWndName);
			XUIMgr.Close(PgQuestManUtil::kBoardQuestWndName);

			g_kQuestMan.CallQuestDialog(kGuid, iQuestID, iDialogID);
		}break;
	case QSDT_CompleteDialog:
		{
			g_kQuestMan.CallQuestDialog(kGuid, iQuestID, iDialogID);
		}break;
	case QSDT_BeginDialog:
	case QSDT_NormalDialog:
		{
			g_kQuestMan.CallQuestDialog(kGuid, iQuestID, iDialogID);
		}break;
	case QSDT_SelectDialog:
		{
			XUI::CXUI_Wnd* pkSelectWnd = XUIMgr.Get(WSTR_SELECTIVE_QUEST_LIST);
			if( !pkSelectWnd )
			{
				pkSelectWnd = XUIMgr.Call(WSTR_SELECTIVE_QUEST_LIST);
				if( !pkSelectWnd )
				{
					return;
				}
			}
			
			PgQUI_SelectiveQuestList* pSelectiveQuestList = dynamic_cast<PgQUI_SelectiveQuestList*>(pkSelectWnd);
			if(!pSelectiveQuestList)
			{
				return;
			}
			pSelectiveQuestList->Show(kGuid);
		}break;
	//case QSDT_BalloonTip:
	//	{
	//		//Say Action 하자
	//	}break;
	default:
		{
		}break;
	}
}


//////////////////////////////////////////////////////////////////////////////////
//
PgQuestMiniListMng::tagMiniState::tagMiniState(SUserQuestState const &rkState)
{
	iOldChangedParam = iCurChangedParam = 0;
	UserState(rkState);
	m_kNew = true;
}
void PgQuestMiniListMng::tagMiniState::UserState(SUserQuestState const &rkUserState)
{
	size_t iCur = 0;
	while(QUEST_PARAMNUM > iCur)
	{
		if( m_kUserState.byParam[iCur] != rkUserState.byParam[iCur] )
		{
			iOldChangedParam = iCurChangedParam;
			iCurChangedParam = iCur;
			break;
		}
		++iCur;
	}
	m_kUserState = rkUserState;
}
//
PgQuestMiniListMng::tagCompareHelper::tagCompareHelper(SUserQuestState const &rkLeft)
	: m_kLeft(rkLeft)
{}
bool PgQuestMiniListMng::tagCompareHelper::operator() (SUserQuestState const &rkRight) const
{
	return m_kLeft.iQuestID == rkRight.iQuestID;
}
bool PgQuestMiniListMng::tagCompareHelper::operator() (pair_type const &rkRight) const
{
	return m_kLeft.iQuestID == rkRight.second.ID();
}
//
PgQuestMiniListMng::PgQuestMiniListMng()
{
	Destroy();
}
PgQuestMiniListMng::~PgQuestMiniListMng()
{
}
void PgQuestMiniListMng::Destroy()
{
	m_iPrevMiniListCount = 0;
	m_iCurMiniListCount = 0;
	m_kFirstRun = true;
	m_kIng.clear();
}
void PgQuestMiniListMng::Save()
{
	PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	std::wstring kSaveFilePath;
	if( !PgHelpSystem::ConvertGuidToSavePathName(pkPlayer->GetID(), kSaveFilePath) )
	{
		return;
	}
	kSaveFilePath += WSTR_SAVE_FILE_NAME;

	BM::Stream kFileData;

	kFileData.Push( m_kIng.size() );
	ContMiniState::const_iterator iter = m_kIng.begin();
	while( m_kIng.end() != iter )
	{
		ContMiniState::key_type const& rkKey = (*iter).first;
		ContMiniState::mapped_type const& rkElement = (*iter).second;

		kFileData.Push(rkKey);
		kFileData.Push(rkElement.ID());

		++iter;
	}

	PgFormSnapedWnd::WriteToPacketSnapInfo(Quest::iQuestMiniListSnapGroupID, kFileData);

	BM::EncSave(kSaveFilePath, kFileData.Data());
}
void PgQuestMiniListMng::Load()
{
	if( !FirstRun() )//첫 실행일 때만
	{
		return;
	}

	m_kFirstRun = false;

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return ;
	}

	PgMyQuest const *pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{
		return;
	}

	std::wstring kSaveFilePath;
	if( !PgHelpSystem::ConvertGuidToSavePathName(pkPlayer->GetID(), kSaveFilePath) )
	{
		return;
	}
	kSaveFilePath += WSTR_SAVE_FILE_NAME;

	BM::Stream kFileData;
	BM::DecLoad(kSaveFilePath, kFileData.Data());
	kFileData.PosAdjust();

	size_t iIngCount = 0;
	if( !kFileData.Pop( iIngCount ) )
	{
		return;
	}

	for( size_t iCur = 0; iIngCount > iCur; ++iCur )
	{
		size_t iKey = 0;
		int iQuestID = 0;

		kFileData.Pop( iKey );
		kFileData.Pop( iQuestID );

		SUserQuestState const* pkQuestState = pkMyQuest->Get(iQuestID);
		if( pkQuestState )
		{
			std::pair<ContMiniState::iterator, bool> kRet = m_kIng.insert( std::make_pair(iKey, ContMiniState::mapped_type(*pkQuestState)) );
		}
	}
	Update();
	PgFormSnapedWnd::ReadFromPacketSnapInfo(Quest::iQuestMiniListSnapGroupID, kFileData);
}
bool PgQuestMiniListMng::Add(int const iQuestID)
{
	PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{
		return false;
	}

	SUserQuestState const* pkUserQuestState = pkMyQuest->Get(iQuestID);
	if( !pkUserQuestState )
	{
		return false;
	}

	if( iMaxMiniListCount <= m_kIng.size() )
	{
		return false;
	}

	SUserQuestState const kTemp(iQuestID, QS_None);
	ContMiniState::iterator find_iter = std::find_if(m_kIng.begin(), m_kIng.end(), SCompareHelper(kTemp));
	if( m_kIng.end() != find_iter )
	{
		return false;
	}

	size_t iEmptySlotNo = 0;
	if( !GetEmptySlotNo(iEmptySlotNo) )
	{
		return false;
	}

	std::pair<ContMiniState::iterator, bool> kRet = m_kIng.insert( std::make_pair(iEmptySlotNo, ContMiniState::mapped_type(*pkUserQuestState)) );
	Update();
	return kRet.second;
}
bool PgQuestMiniListMng::Del(int const iQuestID)
{
	SUserQuestState const kTemp(iQuestID, QS_None);
	ContMiniState::iterator find_iter = std::find_if(m_kIng.begin(), m_kIng.end(), SCompareHelper(kTemp));
	if( m_kIng.end() != find_iter )
	{
		CloseUI((*find_iter).first);
		m_kIng.erase(find_iter);
	}
	Update();
	return true;
}
bool PgQuestMiniListMng::IsInList(int const iQuestID) const
{
	SUserQuestState const kTemp(iQuestID, QS_None);
	return m_kIng.end() != std::find_if(m_kIng.begin(), m_kIng.end(), SCompareHelper(kTemp));
}
size_t PgQuestMiniListMng::Count() const
{
	return m_kIng.size();
}
bool PgQuestMiniListMng::Update()
{
	if( g_kQuestMan.IsQuestDialog() )
	{
		return false; // 수량 업데이트를 미룬다
	}

	m_iPrevMiniListCount = m_iCurMiniListCount;
	m_iCurMiniListCount = Count();

	if( m_iPrevMiniListCount != m_iCurMiniListCount )
	{
		Visible(0 != m_iCurMiniListCount);
		return true;
	}
	UpdateAll();
	return false;
}
void PgQuestMiniListMng::Visible(bool const bVisible)
{
	if( g_kQuestMan.IsQuestDialog() )
	{
		return;
	}

	bool const bOldVisibleStatus = m_bMiniQuestStatus;
	m_bMiniQuestStatus = bVisible && (0 != m_iCurMiniListCount);
	if( m_bMiniQuestStatus )
	{
		UpdateAll();
		//if( bOldVisibleStatus != m_bMiniQuestStatus )
		{
			Quest::lwLoadMiniIngQuestSnapInfo();
		}
	}
	else
	{
		if( 0 < m_iCurMiniListCount )
		{
			Quest::lwSaveMiniIngQuestSnapInfo();
		}
		CloseAll();
	}

	// 퀘스트 List UI에 체크박스 상태를 업데이트 한다
	XUI::CXUI_Wnd* pkMyListWnd = XUIMgr.Get(_T("FORM_MY_QUEST_LIST"));
	if( pkMyListWnd )
	{
		XUI::CXUI_Wnd* pkMyListSubWnd = pkMyListWnd->GetControl(_T("SFRM_MY_QUEST_LIST"));
		if( pkMyListSubWnd )
		{
			XUI::CXUI_Wnd* pkSubListWnd = pkMyListSubWnd->GetControl(_T("SFRM_INFO"));
			if( pkSubListWnd )
			{
				XUI::CXUI_CheckButton* pkCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkSubListWnd->GetControl(_T("CHKBTN_CHK")));
				if( pkCheckBtn )
				{
					pkCheckBtn->Check(m_bMiniQuestStatus);
				}
			}
		}
	}
}
bool PgQuestMiniListMng::GetEmptySlotNo(size_t& riOut) const
{
	size_t iEmptySlot = 1;
	while( iMaxMiniListCount >= iEmptySlot )
	{
		if( m_kIng.end() == m_kIng.find(iEmptySlot) )
		{
			riOut = iEmptySlot;
			return true;
		}
		++iEmptySlot;
	}
	return false;
}
void PgQuestMiniListMng::UpdateAll()
{
	if( g_kQuestMan.IsQuestDialog() )
	{
		return;
	}

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	PgMyQuest const *pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{
		return;
	}

	//ContUserQuestState kVec;
	//pkMyQuest->GetQuestList(kVec);

	// 업데이트 / 삭제 된것 검사
	ContMiniState::iterator ing_iter = m_kIng.begin();
	while( m_kIng.end() != ing_iter )
	{
		ContMiniState::key_type const &rkKey = ing_iter->first;
		ContMiniState::mapped_type &rkElement = ing_iter->second;

		SUserQuestState const* pkUserQuestState = pkMyQuest->Get(rkElement.ID());
		if( !pkUserQuestState )
		{
			CloseUI(rkKey);
			ing_iter = m_kIng.erase(ing_iter);
			continue;
		}

		rkElement.UserState( *pkUserQuestState );
		UpdateUI(rkKey, rkElement); // Update UI
		rkElement.New(false);

		++ing_iter;
	}

	PgFormSnapedHeadWnd *pkHeadSnapWnd = dynamic_cast<PgFormSnapedHeadWnd *>( XUIMgr.Get(WSTR_QUEST_ING_ITEM_HEAD) );
	if( pkHeadSnapWnd )
	{
		pkHeadSnapWnd->ReAlignSnapChild(SnapPT_Bottom);
	}
}
void PgQuestMiniListMng::CloseAll()
{
	size_t iCur = iMaxMiniListCount;
	while(iCur) // 1 ~ 10
	{
		CloseUI(iCur);
		--iCur;
	}
	XUIMgr.Close(WSTR_QUEST_ING_ITEM_HEAD);
}
void PgQuestMiniListMng::CloseUI(size_t const iNo)
{
	XUIMgr.Close( GetUIName(iNo) );
}
std::wstring PgQuestMiniListMng::GetUIName(size_t const iNo)
{
	BM::vstring kTemp = BM::vstring(WSTR_QUEST_ING_ITEM_FORMAT) << iNo;
	return std::wstring(kTemp);
}
void PgQuestMiniListMng::UpdateUI(size_t const iNo, SMiniState const &rkState)
{
	if( !IsInList(rkState.ID()) )
	{
		CloseUI(iNo);
		return;
	}

	// Head가 있는지 확인
	if( NULL == XUIMgr.Get(WSTR_QUEST_ING_ITEM_HEAD) )
	{
		XUIMgr.Call(WSTR_QUEST_ING_ITEM_HEAD);
	}

	// 본편
	std::wstring const kWndName = GetUIName(iNo);
	XUI::CXUI_Wnd *pkWnd = XUIMgr.Get( kWndName );
	if( !pkWnd )
	{
		pkWnd = XUIMgr.Call( kWndName );
		if( !pkWnd )
		{
			return;
		}
	}

	PgFormSnapedSubWnd *pkSnapedSubWnd = dynamic_cast<PgFormSnapedSubWnd *>(pkWnd);
	if( pkSnapedSubWnd
	&&	rkState.New() )
	{
		pkSnapedSubWnd->RegistHeadSnapChild();
	}

	PgQuestInfo const *pkQuestInfo = g_kQuestMan.GetQuest(rkState.ID());
	if( !pkQuestInfo )
	{
		return;
	}

	int const iQuestID = pkQuestInfo->ID();
	pkWnd->SetCustomData(&iQuestID, sizeof(iQuestID));

	XUI::CXUI_Wnd *pkTitleWnd = pkWnd->GetControl( std::wstring(_T("FRM_TITLE")) );
	if( pkTitleWnd )
	{
		int const iNormalTitleUV = 1;
		int const iScenarioTitleUV = 2;
		int const iCoupleTitleUV = 4;
		switch( pkQuestInfo->Type() )
		{
		case QT_Scenario:
			{
				pkTitleWnd->UVUpdate(iScenarioTitleUV);
				pkTitleWnd->OutLineColor(0xFFffab78);
			}break;
		case QT_Couple:
		case QT_SweetHeart:
			{
				pkTitleWnd->UVUpdate(iCoupleTitleUV);
				pkTitleWnd->OutLineColor(0xFFffab78);
			}break;
		case QT_Normal:
		case QT_Event_Normal:
		default:
			{
				pkTitleWnd->UVUpdate(iNormalTitleUV);
				pkTitleWnd->OutLineColor(0xFFaed9e6);
			}break;
		}

		int const iDefaultTextX = 0, iAlignCenterTextX = 73;
		std::wstring kQuestTitle = TTW(pkQuestInfo->m_iTitleTextNo);
		switch( rkState.State() )
		{
		case QS_End:
			{
				pkTitleWnd->FontColor(COLOR_GREEN);//완료는 색깔 바꿈
				pkTitleWnd->OutLineColor(COLOR_BLACK);
				pkTitleWnd->FontFlag( (pkTitleWnd->FontFlag() & ~XTF_ALIGN_CENTER) );
				pkTitleWnd->TextPos( POINT2(iDefaultTextX, pkTitleWnd->TextPos().y) );
				kQuestTitle = _T("[") + TTW(20001) + _T("] ") + kQuestTitle;
			}break;
		case QS_Failed:
			{
				pkTitleWnd->FontColor(0xFFC0C0C0);//실패도 색깔 바꿈
				pkTitleWnd->OutLineColor(COLOR_BLACK);
				pkTitleWnd->FontFlag( (pkTitleWnd->FontFlag() & ~XTF_ALIGN_CENTER) );
				pkTitleWnd->TextPos( POINT2(iDefaultTextX, pkTitleWnd->TextPos().y) );
				kQuestTitle = _T("[") + TTW(20002) + _T("] ") + kQuestTitle;
			}break;
		default:
			{
				pkTitleWnd->FontFlag( (pkTitleWnd->FontFlag() | XTF_ALIGN_CENTER) );
				pkTitleWnd->TextPos( POINT2(iAlignCenterTextX, pkTitleWnd->TextPos().y) );
				pkTitleWnd->FontColor(COLOR_BLACK);
			}break;
		}
		Quest::SetCutedTextLimitLength(pkTitleWnd, kQuestTitle, _T("..."));
	}

	int iChangedParam = rkState.iCurChangedParam;
	int iSubTTW = pkQuestInfo->GetObjectText(rkState.iCurChangedParam);
	if( !iSubTTW )
	{
		iChangedParam = 0;
		iSubTTW = pkQuestInfo->GetObjectText(iChangedParam);
		if( !iSubTTW )
		{
			return;
		}
	}

#define USE_RESIZE_END_QUEST (1)
#ifdef USE_RESIZE_END_QUEST
	bool const bEndQuest = QS_End == rkState.State();
	int const iDefaultSizeY = 40;
	int const iMinimumSizeY = 20;
	POINT2 kSize = pkWnd->Size();
	kSize.y = (bEndQuest)? iMinimumSizeY: iDefaultSizeY;
	pkWnd->Size( kSize );
	pkWnd->SetInvalidate();
#endif

	XUI::CXUI_Wnd *pkCountWnd = pkWnd->GetControl( std::wstring(_T("FRM_RIGHT_COUNT")) );
	if( pkCountWnd )
	{
		int const iMaxCount = pkQuestInfo->GetParamEndCondition(iChangedParam);
		int const iCurCount = rkState.GetParamCondition(iChangedParam);
		TCHAR szTemp[1024] = {0, };
		swprintf_s(szTemp, _T("{C=0xFFFFF200/}%d{C=0xFFFFFFFF/}/%d"), iCurCount, iMaxCount);
		pkCountWnd->Text( std::wstring(szTemp) );
		Quest::SetCutedTextLimitLength(pkCountWnd, std::wstring(szTemp), _T("..."));
#ifdef USE_RESIZE_END_QUEST
		pkCountWnd->Visible( !bEndQuest );
#endif
	}

	XUI::CXUI_Wnd *pkSubWnd = pkWnd->GetControl( std::wstring(_T("FRM_SUB_ITEM")) );
	if( pkSubWnd )
	{
		size_t iRemainCount = 0;
		for(size_t iCur = 0; QUEST_PARAMNUM > iCur; ++iCur)
		{
			if( pkQuestInfo->GetObjectText(iChangedParam) )
			{
				if( pkQuestInfo->GetParamEndCondition(iCur) > rkState.GetParamCondition(iCur) )
				{
					++iRemainCount;
				}
			}
		}

		TCHAR szTemp[512] = {0, };
		swprintf_s(szTemp, _T("(%u) "), iRemainCount);

		std::wstring kSubTitle = szTemp + TTW(iSubTTW);

		int const iDefaultGap = 2;
		int const iTextWidth = (pkCountWnd)? pkCountWnd->TextRect().x + iDefaultGap: iDefaultGap;
		int const iLimitWidth = pkWnd->Width() - iTextWidth;
		Quest::SetCutedTextLimitLength(pkSubWnd, kSubTitle, _T("..."), iLimitWidth);
#ifdef USE_RESIZE_END_QUEST
		pkSubWnd->Visible( !bEndQuest );
#endif
	}
}

void UpdateQuestMiniIngToolTip(int const iQuestID, CXUI_Wnd const* pkTopWnd)
{
	if( !iQuestID )
	{
		CXUI_Wnd* pkToolTipWnd = XUIMgr.Get( WSTR_SFRM_QUEST_QUICK_INFO );
		if( pkToolTipWnd )
		{
			pkToolTipWnd->SetCustomData(&iQuestID, sizeof(iQuestID));
		}
	}

	if( !iQuestID )
	{
		return;
	}

	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	PgMyQuest const *pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{
		return;
	}

	PgQuestInfo const *pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
	if( !pkQuestInfo )
	{
		return;
	}

	SUserQuestState const *pkState = pkMyQuest->Get(iQuestID);
	if( !pkState )
	{
		return;
	}


	
	PgMiniIngToolTip kToolTip;

	std::wstring kTemp;
	size_t iCount = 0;
	for( size_t iCur = 0; QUEST_PARAMNUM > iCur; ++iCur )
	{
		int const iMaxCount = pkQuestInfo->GetParamEndCondition(iCur);
		int const iCurCount = pkState->byParam[iCur];
		int const iEndConditionTTW = pkQuestInfo->GetObjectText(iCur);
		if( iEndConditionTTW
		&&	iMaxCount )
		{
			kToolTip.Add(TTW(iEndConditionTTW), iCurCount, iMaxCount);
			++iCount;
		}
	}

	kToolTip.Call(pkTopWnd, pkQuestInfo);
}



//
PgMiniIngToolTip::PgMiniIngToolTip()
	: m_kMsg(), m_iCount(0)
{
}
PgMiniIngToolTip::~PgMiniIngToolTip()
{
}
void PgMiniIngToolTip::Add(std::wstring const& rkDesc, int const iCur, int const iMax)
{
	if( !m_kMsg.empty() )
	{
		m_kMsg += _T("\n");
	}
	TCHAR szTemp[1024] = {0, };
	if( iMax > iCur )
	{
		swprintf_s(szTemp, 1024, _T("{C=0xFFFFFFFF/}%s {C=0xFFFFF200/}%d{C=0xFFFFFFFF/}/%d"), rkDesc.c_str(), iCur, iMax);
	}
	else
	{
		swprintf_s(szTemp, 1024, _T("{C=0xFF00FF00/}%s {C=0xFFFFF200/}%d{C=0xFFFFFFFF/}/%d"), rkDesc.c_str(), iCur, iMax);
	}
	m_kMsg += szTemp;
	++m_iCount;
}
void PgMiniIngToolTip::Add(std::wstring const& rkDesc)
{
	m_kMsg += rkDesc;
	++m_iCount;
}
bool PgMiniIngToolTip::Call(XUI::CXUI_Wnd const* pkTopWnd, PgQuestInfo const* pkQuestInfo)
{
	if( NULL != pkQuestInfo )
	{
		if( m_kMsg.empty() )
		{
			return false;
		}
	}

	XUI::CXUI_Wnd* pkToolTipWnd = XUIMgr.Get( WSTR_SFRM_QUEST_QUICK_INFO );
	if( !pkToolTipWnd )
	{
		pkToolTipWnd = XUIMgr.Call( WSTR_SFRM_QUEST_QUICK_INFO );
		if( !pkToolTipWnd )
		{
			return false;
		}
	}

	int const iQuestID = (NULL != pkQuestInfo)? pkQuestInfo->ID(): 0;
	pkToolTipWnd->Text( m_kMsg );
	pkToolTipWnd->SetCustomData(&iQuestID, sizeof(iQuestID));

	POINT2 const kTargetSize = pkToolTipWnd->TextRect() + POINT2(24, 8);
	pkToolTipWnd->Size(kTargetSize);
	pkToolTipWnd->TextPos( POINT2(kTargetSize.x / 2, pkToolTipWnd->TextPos().y) );

	if( pkTopWnd )
	{
		POINT3I const &rkTopWndPos = pkTopWnd->Location();
		int const iLimitX = XUIMgr.GetResolutionSize().x - (kTargetSize.x + pkTopWnd->Size().x);
		int const iLimitY = 540;
		if( rkTopWndPos.x < iLimitX && rkTopWndPos.y < iLimitY ) //좌상
		{
			pkToolTipWnd->Location( POINT2(rkTopWndPos.x + pkTopWnd->Size().x, rkTopWndPos.y) );
		}
		else if( rkTopWndPos.x < iLimitX && rkTopWndPos.y > iLimitY ) //좌하
		{
			pkToolTipWnd->Location( POINT2(rkTopWndPos.x + pkTopWnd->Size().x, rkTopWndPos.y) );
		}
		else if( rkTopWndPos.x >= iLimitX && rkTopWndPos.y < iLimitY ) //우상
		{
			pkToolTipWnd->Location( POINT2(rkTopWndPos.x - kTargetSize.x, rkTopWndPos.y) );
		}
		else if( rkTopWndPos.x >= iLimitX && rkTopWndPos.y > iLimitY ) //우하
		{
			pkToolTipWnd->Location( POINT2(rkTopWndPos.x - kTargetSize.x, rkTopWndPos.y) );
		}
	}

	DWORD const dwAliveTime = 4000;
	pkToolTipWnd->CalledTime(BM::GetTime32());
	pkToolTipWnd->AliveTime(dwAliveTime);
	pkToolTipWnd->SetInvalidate();

	return true;
}