#include "stdafx.h"
#include "Variant/PgStringUtil.h"
#include "Variant/PgQuestInfo.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgQuest.h"
#include "PgQuestUI.h"
#include "PgQuestDialog.h"
#include "PgQuestMan.h"
#include "PgOption.h"
#include "Pg2DString.h"
#include "PgChatMgrClient.h"
#include "PgNetwork.h"
#include "lwUI.h"
#include "lwUIQuest.h"

size_t const MAX_ICON_NUM	=5;
size_t const MAX_BUTTON_NUM	=2;

PgQUI_QuestDialog::PgQUI_QuestDialog()
	:m_bInfoDialog(false), m_iQuestID(0), m_iDialogID(0), m_kUserState()
{
}

bool	PgQUI_QuestDialog::OnRewardItemSelected_From_QuestInfo(int iRewardType, unsigned int iSelectedNum)//고를 수 있는 보상 아이템 선택했을 때
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(m_iQuestID);
	if( !pkQuestInfo )
	{
		return false;
	}

	SQuestDialog const* pkQuestDialog = NULL;
	bool bFindDialog = pkQuestInfo->GetDialog(m_iDialogID, pkQuestDialog);
	if( !bFindDialog )
	{
		return false;
	}

	if( QDT_Complete != pkQuestDialog->eType )//이 창이 완료 버튼이 아니면
	{
		NILOG(PGLOG_ERROR, "it is not a Complete Quest Dialog !!!"); assert(0);
		return false;
	}

	const SQuestReward& rkQuestReward = pkQuestInfo->m_kReward;
	int const iClassNo = pkPlayer->GetAbil(AT_CLASS);
	int const iGender = pkPlayer->GetAbil(AT_GENDER);
	ContRewardVec::value_type const *pkRewardSet1 = rkQuestReward.GetRewardSet1(iGender, iClassNo);
	ContRewardVec::value_type const *pkRewardSet2 = rkQuestReward.GetRewardSet2(iGender, iClassNo);
	size_t const iSelectCount1 = (pkRewardSet1)? pkRewardSet1->kItemList.size(): 0;
	size_t const iSelectCount2 = (pkRewardSet2)? pkRewardSet2->kItemList.size(): 0;

	if( 1 == iRewardType )
	{
		if( 1 > iSelectedNum
		||	iSelectCount1 < iSelectedNum )
		{
			PG_ASSERT_LOG(0 && "Invalid select reward 1");
			return false;
		}
		g_kQuestMan.SelectItem1(iSelectedNum);
	}
	else if( 2 == iRewardType )
	{
		if( 1 > iSelectedNum
		||	iSelectCount2 < iSelectedNum )
		{
			PG_ASSERT_LOG(0 && "Invalid select reward 2");
			return false;
		}
		g_kQuestMan.SelectItem2(iSelectedNum);
	}
	else
	{
		NILOG(PGLOG_ERROR, "Invalid reward type"); assert(0);
		return false;
	}

	return true;
}

void PgQUI_QuestDialog::OnGiveUpConfirmed()
{
	if( !m_iQuestID )
	{
		return;
	}
	g_kQuestMan.Send_Drop_Quest(m_iQuestID);

	//
	m_iQuestID = 0;

	XUIMgr.Close(WSTR_MY_QUEST_INFO);
}

void PgQUI_QuestDialog::Show(bool const bInfoDialog, int const iQuestID, int const iDialogID)
{
	if( (bInfoDialog && !iQuestID) || (!bInfoDialog && (!iQuestID || !iDialogID)) )
	{
#ifndef EXTERNAL_RELEASE
		_PgMessageBox("Error", "Invalid InfoDialog(QuestID, DialogID)");
#endif
		return;
	}
	m_bInfoDialog = bInfoDialog;
	m_iQuestID = iQuestID;
	m_iDialogID = iDialogID;

	//	XML 파일이 존재하는가?
	if( m_bInfoDialog )
	{
		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(m_iQuestID);
		if( pkQuestInfo )
		{
			const SQuestDialog* pDlg = NULL;
			bool bFindDialog = pkQuestInfo->GetInfoDialog(pDlg);
			if( !bFindDialog )
			{
				PG_ASSERT_LOG(bFindDialog);
			}

			m_iDialogID = pDlg->iDialogID;
		}
	}
	
	UpdateControl();
}

//>>Local Function
int const iMoneySep = 10000;
int const iOnePercent = 100;
bool BreakMoney_UInt(__int64 const iTotal, std::vector< unsigned int > &rkVec)
{
	__int64 iValue = iTotal;
	if(iTotal>iValue)
	{
		NILOG(PGLOG_ERROR, "Critical Error int64 -> unsigned int"); assert(0); return false;
	}

	if(rkVec.size())
	{
		rkVec.clear();
	}

	unsigned int iGold = (unsigned int)(iTotal/iMoneySep);
	unsigned int iSilver = (unsigned int)((iTotal - (static_cast<__int64>(iGold)*iMoneySep)) / iOnePercent);
	unsigned int iBronze = (unsigned int)((iTotal - (static_cast<__int64>(iGold)*iMoneySep)-(static_cast<__int64>(iSilver)*iOnePercent)) % iOnePercent);
	rkVec.push_back(iGold);//Gold
	rkVec.push_back(iSilver);//Silver
	rkVec.push_back(iBronze);//Copper
	return true;
}

bool BreakMoney_WStr(__int64 const iTotal, VEC_WSTRING &rkVec)
{
	std::vector< unsigned int > kVec;
	if(!BreakMoney_UInt(iTotal, kVec))
	{
		return false;
	}

	TCHAR szTemp[_MAX_PATH] = {0, };
	//99 >		__ __ 99		Step1
	//100 >		__ _1 00		Step2
	//10100>    _1 01 00		Step3
	if(0 != kVec[0])//Step3
	{
		_stprintf_s(szTemp, _MAX_PATH, _T("%2d/%02d/%02d"), kVec[0], kVec[1], kVec[2]);
	}
	else if(0 != kVec[1])//Step2
	{
		_stprintf_s(szTemp, _MAX_PATH, _T("%2d/%2d/%02d"), kVec[0], kVec[1], kVec[2]);
	}
	else//Step1
	{
		_stprintf_s(szTemp, _MAX_PATH, _T("%2d/%2d/%2d"), kVec[0], kVec[1], kVec[2]);
	}
	PgStringUtil::BreakSep(szTemp, rkVec, L"/");

	return true;
}

bool FormatMoney(__int64 const iTotal, std::wstring &rkOut)
{
	std::vector< unsigned int > kVec;
	bool const bBreakRet = BreakMoney_UInt(iTotal, kVec);
	if( !bBreakRet )
	{
		return false;
	}

	std::wstring kOutText;
	std::wstring kTemp;
	//99 >		__ __ 99		Step1
	//100 >		__ _1 00		Step2
	//10100>    _1 01 00		Step3
	if( kVec[0] )//Gold
	{
		if( FormatTTW(kTemp, 700051, kVec[0]) )
		{
			kOutText += kTemp;
		}
	}
	if( kVec[1] )//Silver
	{
		if( FormatTTW(kTemp, 700050, kVec[1]) )
		{
			kOutText += kTemp;
		}
	}
//	if( kVec[2] )//Bronze -> 0 쿠퍼는 찍히게.
	{
		if( FormatTTW(kTemp, 700049, kVec[2]) )
		{
			kOutText += kTemp;
		}
	}

	if( !kOutText.empty() )
	{
		rkOut = kOutText;
	}
	return true;
}

void SetYLoc(XUI::CXUI_Wnd* pkWnd, int &iYLoc, int const iTerm = 5)
{
	POINT3I kPosition = pkWnd->Location();
	kPosition.y = iYLoc;
	pkWnd->Location(kPosition);
	iYLoc += pkWnd->Size().y + iTerm;
}
//<<Local Function

bool PgQUI_QuestDialog::UpdateState(int const iQuestID)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	PgMyQuest const *pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{
		return false;
	}
	
	m_kUserState = SUserQuestState();
	ContUserQuestState kVec;
	if(pkMyQuest->GetQuestList(kVec))
	{
		for(unsigned int i=0; kVec.size() > i; ++i)
		{
			if(kVec[i].iQuestID == iQuestID)
			{
				m_kUserState = kVec[i];
				break;
			}
		}
	}

	if( !m_kUserState.iQuestID )
	{
		return false;
	}

	return true;
}

void PgQUI_QuestDialog::Clear()
{
	XUI::CXUI_List* pListWnd = dynamic_cast<XUI::CXUI_List*>(GetControl(_T("QUEST_LIST")));
	if( pListWnd )
	{
		pListWnd->ClearList();	//배경 초기화
	}

	XUI::CXUI_Wnd* pkGiveUpWnd = GetControl( PgQuestManUtil::kGiveupButtonName );
	if( pkGiveUpWnd )
	{
		pkGiveUpWnd->Visible(false);
	}

	XUI::CXUI_Wnd* pkRemoteForm = GetControl( PgQuestManUtil::kRemoteFormName );
	if( pkRemoteForm )
	{
		pkRemoteForm->Visible(false);
	}
}

//
void PgQUI_QuestDialog::UpdateControl()
{
	//INFO 다이얼로그만 하자
	if( !m_iQuestID
	||	!m_iDialogID )
	{
		PG_ASSERT_LOG(0 && "m_sQuestID == 0, m_sDialogID == 0\n");
		return;
	}

	UpdateState(m_iQuestID);

	PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(m_iQuestID);
	if( !pkQuestInfo )
	{
		return;
	}

	const SQuestDialog* pkQuestDialog = NULL;
	bool bFindDialog = pkQuestInfo->GetDialog(m_iDialogID, pkQuestDialog);
	if( !bFindDialog )
	{
		return;
	}

	switch(pkQuestDialog->eType)
	{
	case QDT_Wanted:
	case QDT_VillageBoard:
		{
			return;
		}break;
	default:
		{
		}break;
	}

	XUI::CXUI_List* pListWnd = dynamic_cast<XUI::CXUI_List*>(GetControl(_T("QUEST_LIST")));
	if( !pListWnd )
	{
		return;
	}

	//정보창 배경
	XUI::SListItem *pkNewItem = pListWnd->FirstItem();
	if( NULL == pkNewItem )
	{
		pkNewItem = pListWnd->AddItem(_T("BackGround"));
		if( !pkNewItem )
		{
			return;
		}
	}

	XUI::CXUI_Wnd *pFirstItem = pkNewItem->m_pWnd;
	if( !pFirstItem )
	{
		return;
	}

	if( QDT_Normal == pkQuestDialog->eType )
	{
		return;
	}

	int iYLoc = 0;//시작
 
	bool bUpdate = false;

	//
	const SQuestDialog* pkInfoDialog = pkQuestDialog;
	if( QDT_Info == pkQuestDialog->eType
	||	QDT_Prolog == pkQuestDialog->eType
	||	QDT_Complete == pkQuestDialog->eType )
	{
		pkQuestInfo->GetInfoDialog(pkInfoDialog);
	}
	bUpdate = UpdateShare(pkQuestInfo, pkInfoDialog, pFirstItem, iYLoc);
	bUpdate = UpdateTitle(pkQuestInfo, pkInfoDialog, pFirstItem, iYLoc);//Title
	bUpdate = UpdateDialog(pkQuestInfo, pkInfoDialog, pFirstItem, iYLoc);//Dialog Text
	bUpdate = UpdateObject(pkQuestInfo, pkInfoDialog, pFirstItem, iYLoc);//Quest Object's

	bUpdate = UpdateReward(pkQuestInfo, pkQuestDialog, pFirstItem, iYLoc);//Reward
	bUpdate = UpdateRewardIcon(1, pkQuestInfo, pkQuestDialog, pFirstItem, iYLoc);//아이템 목록 1
	bUpdate = UpdateRewardIcon(2, pkQuestInfo, pkQuestDialog, pFirstItem, iYLoc);//아이템 목록 2
	bUpdate = UpdateSkillIcon(pkQuestInfo, pkQuestDialog, pFirstItem, iYLoc);
	bUpdate = UpdateLevelIcon(pkQuestInfo, pFirstItem, iYLoc);
	bUpdate = UpdateButtons(pkQuestInfo, pkQuestDialog, pFirstItem, iYLoc);//Button

	//모든 리스트 아이템을 마쳤다
	pFirstItem->Size( pFirstItem->Size().x, iYLoc + 20 );
	pFirstItem->SetInvalidate(true);
	pListWnd->AdjustMiddleBtnPos();
}


bool PgQUI_QuestDialog::UpdateShare(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc)
{
	XUI::CXUI_Wnd* pkButton = pTopWnd->GetControl( std::wstring(L"BTN_SHARE_QUEST") );
	if( !pkButton )
	{
		return false;
	}

	if( QDT_Info == pkQuestDialog->eType
	&&	m_bInfoDialog )
	{
		if( pkQuestInfo->IsCanShare() )
		{
			SetYLoc(pkButton, iYLoc);
			int const iQuestID = pkQuestInfo->ID();
			pkButton->SetCustomData( &iQuestID, sizeof(iQuestID) );
			
			switch( pkQuestInfo->Type() )
			{
			case QT_Couple:
			case QT_SweetHeart:
				{
					pkButton->Text( TTW(401438) );
				}break;
			default:
				{
					pkButton->Text( TTW(401431) );
				}break;
			}
		}
		else
		{
			pkButton->ClearCustomData();
		}
		pkButton->Visible( pkQuestInfo->IsCanShare() );
	}
	else
	{
		pkButton->Visible(false);
	}
	return true;
}

bool PgQUI_QuestDialog::UpdateTitle(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc)
{
	XUI::CXUI_Wnd* pTitleWnd = pTopWnd->GetControl(_T("FRM_TITLE_BG"));
	if( !pTitleWnd )
	{
		return false;
	}

	if( QDT_Info == pkQuestDialog->eType )
	{
		BM::vstring kQuestTitle( TTW(pkQuestInfo->m_iTitleTextNo) );
		if( g_pkApp->VisibleClassNo() )
		{
			kQuestTitle += L" ";
			kQuestTitle += pkQuestInfo->ID();
		}
		pTitleWnd->Text( static_cast<std::wstring>( kQuestTitle ) );
		pTitleWnd->Visible(true);
		SetYLoc(pTitleWnd, iYLoc);
	}
	else
	{
		pTitleWnd->Visible(false);
	}
	return true;
}

bool PgQUI_QuestDialog::UpdateDialog(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc)
{
	if( !pkQuestInfo )
	{
		return false;
	}

	XUI::CXUI_List *pListWnd = dynamic_cast<XUI::CXUI_List *>(pTopWnd->GetControl(_T("LIST_INFO_CONTENTS")));
	if( !pListWnd )
	{
		return false;
	}

	XUI::SListItem *pkNewItem = pListWnd->FirstItem();
	if( !pkNewItem )
	{
		pkNewItem = pListWnd->AddItem(std::wstring(_T("")));
		if( !pkNewItem )
		{
			return false;
		}
	}

	if( pkNewItem
	&&	pkNewItem->m_pWnd
	&&	QDT_Info == pkQuestDialog->eType )
	{
		CXUI_Wnd *pDialogWnd = pkNewItem->m_pWnd;
		std::wstring kDialogText;
		ContQuestDialogText::const_iterator dialogtext_iter = pkQuestDialog->kDialogText.begin();
		while(pkQuestDialog->kDialogText.end() != dialogtext_iter)
		{
			kDialogText += TTW(dialogtext_iter->iTextNo);
			++dialogtext_iter;
		}

		Quest::lwQuestTextParser(kDialogText, pkQuestInfo);
		pDialogWnd->Text(kDialogText);
		CXUI_Style_String kStyleText = pDialogWnd->StyleText();
		const POINT kCalcSize = Pg2DString::CalculateOnlySize(kStyleText);
		pDialogWnd->Size( pDialogWnd->Size().x, kCalcSize.y+2 );
		pDialogWnd->SetInvalidate();

		const POINT2 kListSize(pListWnd->Size().x, kCalcSize.y+10);
		pListWnd->Size(kListSize);
		pListWnd->SetInvalidate();

		SetYLoc(pListWnd, iYLoc, 5);//리스트 위치를 조절 한다.

		pListWnd->AdjustMiddleBtnPos();
		pListWnd->AdjustMiddleBarSize();
		pListWnd->AdjustDownBtnPos();
	}
	else
	{
		pListWnd->Visible(false);
	}
	return true;
}

bool PgQUI_QuestDialog::UpdateObject(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc)
{
	XUI::CXUI_Wnd* pObjectBgWnd = pTopWnd->GetControl(_T("FRM_OBJECT_BG"));//
	if( !pObjectBgWnd )
	{
		return false;
	}

	XUI::CXUI_Wnd* pObjectFrmWnd = pTopWnd->GetControl(_T("FRM_OBJECT"));//
	if( !pObjectFrmWnd )
	{
		return false;
	}

	if( QDT_Complete == pkQuestDialog->eType
	||	QDT_Info == pkQuestDialog->eType
	||	QDT_Prolog == pkQuestDialog->eType )
	{
		//각 퀘스트 목표를 합쳐서 표시 한다.
		std::wstring kObjectDesc;
		int iCurIcon = 1;
		for(int iCur = 0; QUEST_PARAMNUM > iCur; ++iCur)
		{
			TCHAR szUIName[255] = {0, };
			_stprintf_s(szUIName, 255, _T("IMG_IconAllow%d"), iCurIcon);
			XUI::CXUI_Wnd *pIconWnd = pObjectFrmWnd->GetControl(szUIName);
			if( !pIconWnd )
			{continue;}

			int const iObjectCount = pkQuestInfo->GetParamEndCondition(iCur);
			if( iObjectCount )
			{
				int const iTTW = pkQuestInfo->GetObjectText(iCur);
				if( iTTW )
				{
					kObjectDesc += TTW(iTTW);//목표 설명을 합치고
					TCHAR szTemp[255] = {0, };
					if( QDT_Info == pkQuestDialog->eType
					&& 1 < iObjectCount )
					{
						_stprintf_s(szTemp, 255, _T(" [%d/%d]\n"), m_kUserState.byParam[iCur], iObjectCount);
					}
					else
					{
						_stprintf_s(szTemp, 255, _T("\n"));
					}
					kObjectDesc += szTemp;
					pIconWnd->Visible(true);

					++iCurIcon;
				}
			}
			else
			{
				pIconWnd->Visible(false);
			}
		}
		bool const bVisibleObject = (0 != kObjectDesc.size());//설명이 있나
		pObjectBgWnd->Visible(bVisibleObject);
		pObjectFrmWnd->Visible(bVisibleObject);

		if( bVisibleObject )
		{
			Quest::lwQuestTextParser(kObjectDesc, pkQuestInfo);
			SetYLoc(pObjectBgWnd, iYLoc);

			pObjectFrmWnd->Text(kObjectDesc);
			CXUI_Style_String kStyleText = pObjectFrmWnd->StyleText();
			const POINT kCalcSize = Pg2DString::CalculateOnlySize(kStyleText);
			pObjectFrmWnd->Size(pObjectFrmWnd->Size().x, kCalcSize.y);
			SetYLoc(pObjectFrmWnd, iYLoc);
		}
	}
	else
	{
		pObjectBgWnd->Visible(false);
		pObjectFrmWnd->Visible(false);
	}

	return true;
}

bool PgQUI_QuestDialog::UpdateReward(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	XUI::CXUI_Wnd* pRewardBgWnd = pTopWnd->GetControl(_T("FRM_REWARD_BG"));//
	if( !pRewardBgWnd )
	{
		return false;
	}

	XUI::CXUI_Wnd* pChangeClassBgWnd = pTopWnd->GetControl(_T("FRM_CHANGECLASS_BG"));//Change Class
	if( !pChangeClassBgWnd )
	{
		return false;
	}

	XUI::CXUI_Wnd* pExpFrmWnd = pTopWnd->GetControl(_T("FRM_EXP"));//Exp
	if( !pExpFrmWnd )
	{
		return false;
	}

	XUI::CXUI_Wnd* pGoldFrmWnd = pTopWnd->GetControl(_T("FRM_GOLD"));//Gold
	if( !pGoldFrmWnd )
	{
		return false;
	}

	if( QDT_Complete == pkQuestDialog->eType
	||	QDT_Info == pkQuestDialog->eType
	||	QDT_Prolog == pkQuestDialog->eType )
	{
		pRewardBgWnd->Visible(true);
		SetYLoc(pRewardBgWnd, iYLoc);

		if( QT_ClassChange == pkQuestInfo->Type() )//전직 직업이 설정 되 있으면
		{
			const SChangeClassInfo& rkChangeClass = pkQuestInfo->m_kReward.kChangeClass;
			int const iChangeClassNo = rkChangeClass.iChangeClassNo;
			int const iChangeClassMagicNumber = 30100;//매직넘버 ㅠㅠ;
			std::wstring kClassName = TTW(iChangeClassNo+iChangeClassMagicNumber);
			if( kClassName.size() )
			{
				//UV설정하고
				pChangeClassBgWnd->Text(kClassName);
				pChangeClassBgWnd->Visible(true);
				SetYLoc(pChangeClassBgWnd, iYLoc, 2);
			}
			else
			{
				PG_ASSERT_LOG(0 && "Can't find Class name");
			}
		}
		else
		{
			pChangeClassBgWnd->Visible(false);
		}

		CONT_DEF_QUEST_LEVEL_REWARD const *pDef_LevelReward = NULL;
		g_kTblDataMgr.GetContDef(pDef_LevelReward);

		int iTotalExp = PgQuestInfoUtil::GetQuestRewardExp(pkQuestInfo, pkPlayer->GetMyQuest());
		if( pDef_LevelReward )
		{
			CONT_DEF_QUEST_LEVEL_REWARD::const_iterator kitor_Reward = pDef_LevelReward->find( pkPlayer->GetAbil(AT_LEVEL) );
			if( pDef_LevelReward->end() != kitor_Reward )
			{
				float const fRate = pkQuestInfo->m_kReward.iLevelPer_Exp_1000 / 1000;
				iTotalExp += (*kitor_Reward).second.iExp * fRate;
			}
		}
		if( iTotalExp )
		{
			XUI::CXUI_Wnd* pWnd_Exp = pExpFrmWnd->GetControl(_T("FRM_EXP_NUM"));
			if( pWnd_Exp )
			{
				BM::vstring vstrLv(iTotalExp);
				pWnd_Exp->Text(vstrLv);

				POINT3I kPosition = pExpFrmWnd->Location();
				kPosition.y = iYLoc;
				pExpFrmWnd->Location(kPosition);
				pExpFrmWnd->Visible(true);
			}
		}
		else
		{
			pExpFrmWnd->Visible(false);
		}

		int iTotalMoney = pkQuestInfo->m_kReward.iMoney;
		if( pDef_LevelReward )
		{
			CONT_DEF_QUEST_LEVEL_REWARD::const_iterator kitor_Reward = pDef_LevelReward->find( pkPlayer->GetAbil(AT_LEVEL) );
			if( pDef_LevelReward->end() != kitor_Reward )
			{
				float const fRate = pkQuestInfo->m_kReward.iLevelPer_Money_1000 / 1000;
				iTotalMoney += (*kitor_Reward).second.iMoney * fRate;
			}
		}
		if( iTotalMoney )
		{
			XUI::CXUI_Wnd* pWnd_Gold = pGoldFrmWnd->GetControl(_T("FRM_GOLD_NUM"));
			XUI::CXUI_Wnd* pWnd_Silver = pGoldFrmWnd->GetControl(_T("FRM_SILVER_NUM"));
			XUI::CXUI_Wnd* pWnd_Bronze = pGoldFrmWnd->GetControl(_T("FRM_BRONZE_NUM"));
			if(pWnd_Gold && pWnd_Silver && pWnd_Bronze)//돈 업데이트
			{
				std::vector< std::wstring > kVec;
				BreakMoney_WStr(iTotalMoney, kVec);

				pWnd_Gold->Text(kVec[0]);
				pWnd_Silver->Text(kVec[1]);
				pWnd_Bronze->Text(kVec[2]);
			}

			pGoldFrmWnd->Visible(true);
			SetYLoc(pGoldFrmWnd, iYLoc, 2);
		}
		else
		{
			pGoldFrmWnd->Visible(false);
		}

		if(iTotalExp && (!iTotalMoney))
		{// 골드 보상이 없을 경우 예외처리
			SetYLoc(pExpFrmWnd, iYLoc, 2);
		}
	}
	else
	{
		pRewardBgWnd->Visible(false);
		pChangeClassBgWnd->Visible(false);
		pExpFrmWnd->Visible(false);
		pGoldFrmWnd->Visible(false);
	}
	return true;
}


bool PgQUI_QuestDialog::UpdateRewardIcon(int const iSetNo, PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd *pTopWnd, int &iYLoc)
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	const SQuestReward &rkReward = pkQuestInfo->m_kReward;
	int const iClassNo = pkPlayer->GetAbil(AT_CLASS);
	int const iGender = pkPlayer->GetAbil(AT_GENDER);
	
	bool bSelectiveItem = false;
	ContQuestItem kItemList;

	switch( iSetNo )
	{
	case 1:
		{
			ContRewardVec::value_type const *pkRewardSet1 = rkReward.GetRewardSet1(iGender, iClassNo);
			bSelectiveItem = (pkRewardSet1)? pkRewardSet1->bSelective: false;
			kItemList = (pkRewardSet1)? pkRewardSet1->kItemList: ContQuestItem();
		}break;
	case 2:
		{
			ContRewardVec::value_type const *pkRewardSet2 = rkReward.GetRewardSet2(iGender, iClassNo);
			bSelectiveItem = (pkRewardSet2)? pkRewardSet2->bSelective: false;
			kItemList = (pkRewardSet2)? pkRewardSet2->kItemList: ContQuestItem();
		}break;
	default:
		{
			PG_ASSERT_LOG(0 && iSetNo);
		}break;
	}

	TCHAR szTemp[255] = {0, };
	_stprintf_s(szTemp, _T("SELECT_ONE%d"), iSetNo);
	XUI::CXUI_Wnd *pkSelectTitle = pTopWnd->GetControl(szTemp);
	if( !pkSelectTitle )
	{
		return false;
	}

	_stprintf_s(szTemp, _T("IMG_OVER%d"), iSetNo);
	XUI::CXUI_Wnd* pkOverImg = pTopWnd->GetControl(szTemp);
	if( !pkOverImg )
	{
		return false;
	}
	
	pkOverImg->Visible(false);

	XUI::CXUI_Wnd *pkItemList[MAX_ICON_NUM] = {0, };
	XUI::CXUI_Wnd *pkItemBG[MAX_ICON_NUM] = {0, };
	XUI::CXUI_Wnd *pkItemBtn[MAX_ICON_NUM] = {0, };

	for(size_t iCur=0; MAX_ICON_NUM>iCur; ++iCur)
	{
		_stprintf_s(szTemp, WSTR_REWARD_ICON_FORMAT, iSetNo, iCur+1);
		pkItemList[iCur] = pTopWnd->GetControl(szTemp);
		_stprintf_s(szTemp, WSTR_REWARD_SFRM_FORMAT, iSetNo, iCur+1);
		pkItemBG[iCur] = pTopWnd->GetControl(szTemp);
		_stprintf_s(szTemp, WSTR_REWARD_BTN_FORMAT, iSetNo, iCur+1);
		pkItemBtn[iCur] = pTopWnd->GetControl(szTemp);
	}

	for(size_t iCur=0; MAX_ICON_NUM>iCur; ++iCur)
	{
		if( !pkItemList[iCur] )
		{
			return false;
		}

		if( !pkItemBG[iCur] )
		{
			return false;
		}

		if( !pkItemBtn[iCur] )
		{
			return false;
		}
		pkItemList[iCur]->Visible(false);
		pkItemBG[iCur]->Visible(false);
		pkItemBtn[iCur]->Visible(false);
	}

	if( kItemList.size() )
	{
		//400098: 아래의 보상 중에 선택 할 수 있습니다.
		//400090: 아래의 보상이 주어집니다.
		std::wstring kText = (bSelectiveItem)? TTW(400089): TTW(400090);

		pkSelectTitle->Text(kText);
		pkSelectTitle->Location(pkSelectTitle->Location().x, iYLoc);
		pkSelectTitle->Visible(true);
		iYLoc += pkSelectTitle->Size().y + 2;

		size_t const iItemCnt = kItemList.size();
		ContQuestItem::const_iterator item_iter = kItemList.begin();
		for(size_t iCur=0; iItemCnt>iCur; ++iCur, ++item_iter)
		{
			const ContQuestItem::value_type& rkElement = (*item_iter);
			if( MAX_ICON_NUM <= iCur ) break;

			if( rkElement.iItemNo && rkElement.iCount )
			{
				pkItemList[iCur]->Location(pkItemList[iCur]->Location().x, iYLoc+1);
				pkItemList[iCur]->SetCustomData(&rkElement, sizeof(rkElement));//
				if( 1 < rkElement.iCount )
				{
					pkItemList[iCur]->Text((std::wstring)BM::vstring((int)rkElement.iCount));
				}
				else
				{
					pkItemList[iCur]->Text(_T(""));
				}
				Quest::lwOnCallQuestRewardItemImage(pkItemList[iCur]);
				pkItemList[iCur]->Visible(true);

				pkItemBG[iCur]->Location(pkItemBG[iCur]->Location().x, iYLoc);
				pkItemBG[iCur]->Visible(true);

				bool const bComplete = (QDT_Complete == pkQuestDialog->eType);
				if( bComplete
				&&	bSelectiveItem )
				{
					pkItemBtn[iCur]->Location(pkItemBG[iCur]->Location().x, iYLoc);
					pkItemBtn[iCur]->SetCustomData(&rkElement, sizeof(rkElement));
					pkItemBtn[iCur]->Visible(true);
				}
			}
		}
		iYLoc += pkItemList[0]->Size().y + 5;
	}
	else
	{
		pkSelectTitle->Visible(false);
	}

	return true;
}

bool PgQUI_QuestDialog::UpdateSkillIcon(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc)
{
	XUI::CXUI_Wnd* pkTitleWnd = pTopWnd->GetControl( std::wstring(L"SELECT_ONE3") );
	XUI::CXUI_Icon* pkIconWnd = dynamic_cast< XUI::CXUI_Icon* >(pTopWnd->GetControl( std::wstring(L"REWARD_ITEM3_1") ));
	XUI::CXUI_Wnd* pkBGWnd = pTopWnd->GetControl( std::wstring(L"SFRM_REWARD_BG3_1") );
	if( !pkTitleWnd || !pkIconWnd || !pkBGWnd )
	{
		return false;
	}

	int const iSkillNo = pkQuestInfo->m_kReward.iSkillNo;
	bool const bHaveSkill = (0 != iSkillNo);

	if( bHaveSkill )
	{
		SetYLoc(pkTitleWnd, iYLoc, 2);
		pkBGWnd->Location( pkBGWnd->Location().x, iYLoc );
		pkIconWnd->Location( pkIconWnd->Location().x, iYLoc+1 );

		GET_DEF(CSkillDefMgr, kSkillDef);
		CSkillDef const* pkSkillDef = kSkillDef.GetDef(iSkillNo);
		if( pkSkillDef )
		{
			SIconInfo kIconInfo( pkIconWnd->IconInfo() );
			kIconInfo.iIconKey = iSkillNo;
			kIconInfo.iIconResNumber = pkSkillDef->RscNameNo();
			pkIconWnd->SetIconInfo( kIconInfo );
		}

		iYLoc += pkBGWnd->Size().x + 2;
	}
	pkTitleWnd->Visible( bHaveSkill );
	pkIconWnd->Visible( bHaveSkill );
	pkBGWnd->Visible( bHaveSkill );

	return true;
}
bool PgQUI_QuestDialog::UpdateLevelIcon(PgQuestInfo const* pkQuestInfo, XUI::CXUI_Wnd* pTopWnd, int& iYLoc)
{
	//레벨업 보상이 있을 경우, 맨 마지막 아이콘에 연결시켜 주자.
	const SQuestReward &rkReward = pkQuestInfo->m_kReward;
	XUI::CXUI_Wnd* pkRewardLevel = pTopWnd->GetControl(WSTR_REWARD_CHANGE_LEVEL);
	XUI::CXUI_Wnd* pkGetTitle = pTopWnd->GetControl(L"GET_ONE");
	if( pkRewardLevel && pkGetTitle)
	{
		int const iChangeLevel = pkQuestInfo->m_kReward.iChangeLevel;
		if( 0 < iChangeLevel )
		{
			pkGetTitle->Visible(true);
			pkRewardLevel->Visible(true);;
			pkRewardLevel->SetCustomData( &iChangeLevel, sizeof(iChangeLevel) );
			pkGetTitle->Location(pkGetTitle->Location().x, iYLoc);
			pkRewardLevel->Location(pkRewardLevel->Location().x, iYLoc+20);
			XUI::CXUI_Wnd* pkLevelText = pkRewardLevel->GetControl(L"FRM_LV_TEXT");
			if( pkLevelText )
			{
				BM::vstring vstrText(iChangeLevel);
				pkLevelText->Text( static_cast<std::wstring>(vstrText) );
			}
			iYLoc += (pkRewardLevel->Size().y + 5);
		}
		else
		{
			pkRewardLevel->Visible(false);
			pkGetTitle->Visible(false);
		}
	}
	return true;
}
#define MAX_QUEST_BUTTON	(2)
typedef std::vector< XUI::CXUI_Wnd* > ContpXuiVec;
bool PgQUI_QuestDialog::UpdateButtons(PgQuestInfo const* pkQuestInfo, const SQuestDialog* pkQuestDialog, XUI::CXUI_Wnd* pTopWnd, int& iYLoc)
{
	if( !pTopWnd
	||	!pkQuestInfo )
	{
		return false;
	}

	size_t const iButtonCount = pkQuestDialog->kSelectList.size();

	XUI::CXUI_Wnd* pButton[MAX_QUEST_BUTTON] = {0, };
	ContpXuiVec kHideVec;

	//특수 다이얼로그당 할당된 기본 버튼수는 2개
	bool bModeComplete = false;
	if( QDT_Complete == pkQuestDialog->eType )
	{
		pButton[0] = GetControl( PgQuestManUtil::kCompleteButtonName );
		pButton[1] = GetControl( PgQuestManUtil::kFunc2ButtonName );
		std::back_inserter(kHideVec) = GetControl( PgQuestManUtil::kAcceptButtonName );
		std::back_inserter(kHideVec) = GetControl( PgQuestManUtil::kRejectButtonName );
	}
	else if( QDT_Prolog == pkQuestDialog->eType )
	{
		pButton[0] = GetControl( PgQuestManUtil::kAcceptButtonName );
		pButton[1] = GetControl( PgQuestManUtil::kRejectButtonName );
		std::back_inserter(kHideVec) = GetControl( PgQuestManUtil::kCompleteButtonName );
		std::back_inserter(kHideVec) = GetControl( PgQuestManUtil::kFunc2ButtonName );
	}
	else if( QDT_Info == pkQuestDialog->eType )//아래 버튼들은 다른 폼이다
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
			if( pkMyQuest )
			{
				SUserQuestState const* pkQuestState = pkMyQuest->Get(m_iQuestID);
				if( pkQuestState )
				{
					bModeComplete = (pkQuestState->byQuestState == QS_End) && pkQuestInfo->IsCanRemoteComplete();
				}
			}
		}

		if( !bModeComplete )
		{
			pButton[0] = GetControl( PgQuestManUtil::kGiveupButtonName );
			pButton[1] = GetControl( PgQuestManUtil::kFunc2ButtonName );
		}
		else
		{
			pButton[0] = NULL;
			pButton[1] = NULL;
			std::back_inserter(kHideVec) = GetControl( PgQuestManUtil::kGiveupButtonName );
			std::back_inserter(kHideVec) = GetControl( PgQuestManUtil::kFunc2ButtonName );
		}
		std::back_inserter(kHideVec) = GetControl( PgQuestManUtil::kAcceptButtonName );
		std::back_inserter(kHideVec) = GetControl( PgQuestManUtil::kRejectButtonName );
		std::back_inserter(kHideVec) = GetControl( PgQuestManUtil::kCompleteButtonName );
		std::back_inserter(kHideVec) = GetControl( PgQuestManUtil::kFunc2ButtonName );

		//
		XUI::CXUI_Wnd* pkRemoteForm = GetControl( PgQuestManUtil::kRemoteFormName );
		if( pkRemoteForm )
		{
			pkRemoteForm->Visible(bModeComplete);
			XUI::CXUI_Wnd* pkGiveUpBtn = pkRemoteForm->GetControl( PgQuestManUtil::kRemoteGiveupBtnName );
			XUI::CXUI_Wnd* pkCompleteBtn = pkRemoteForm->GetControl( PgQuestManUtil::kRemoteCompleteBtnName );
			if( pkCompleteBtn )
			{
				if( bModeComplete )
				{
					pkCompleteBtn->SetCustomData( &m_iQuestID, sizeof(m_iQuestID) );
					pButton[0] = pkGiveUpBtn;
				}
				else
				{
					pkCompleteBtn->ClearCustomData();
				}
			}
		}
	}

	for(size_t iCur=0; MAX_QUEST_BUTTON>iCur; ++iCur)
	{
		XUI::CXUI_Wnd* pCurButton = pButton[iCur];
		if( pCurButton )
		{
			//설정 되있으면
			if( iButtonCount >= iCur+1 )
			{
				SQuestSelect const& kCurSelectInfo = pkQuestDialog->kSelectList.at(iCur);
				int const iNextDialogID = kCurSelectInfo.iSelectID;
				std::wstring kButtonText = TTW(kCurSelectInfo.iSelectTextNo);
				Quest::lwQuestTextParser(kButtonText);

				pCurButton->SetCustomData(&iNextDialogID, sizeof(iNextDialogID));
				if( !bModeComplete )
				{
					pCurButton->Text(kButtonText);
				}
				else
				{
					// nothing
				}
				pCurButton->Visible(true);
			}
			else
			{
				pCurButton->Visible(false);
			}
		}
	}

	ContpXuiVec::iterator wnd_iter = kHideVec.begin();
	while(kHideVec.end() !=  wnd_iter)
	{
		ContpXuiVec::value_type pkElement = (*wnd_iter);
		if( pkElement )
		{
			pkElement->Visible(false);
		}
		++wnd_iter;
	}

	iYLoc += 2;

	return true;
}
