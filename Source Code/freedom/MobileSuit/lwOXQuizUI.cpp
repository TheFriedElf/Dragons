#include "StdAfx.h"
#include "PgMobileSuit.h"
#include "PgNetwork.h"
#include "Pg2DString.h"
#include "lwUI.h"
#include "lwOXQuizUI.h"

int const DFT_CENTER_YSIZE = 103;
int const DFT_FRAME_SIZE = 30;
float const DFT_INVITATION_TIME = 30.0f;
float const DFT_SOLVE_TIME = 10.0f;
float const DFT_FLASH_TIME = 0.3f;

void lwOXQuizUI::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "SelectOXAnswer", &lwOXQuizUI::lwSelectOXAnswer);
	def(pkState, "UpdateOXLight", &lwOXQuizUI::lwUpdateOXLight);
	def(pkState, "TickUpdateTimer", &lwOXQuizUI::lwTickUpdateTimer);
	def(pkState, "RequestOXAnswer", &lwOXQuizUI::lwRequestOXAnswer);
}

void lwOXQuizUI::lwSelectOXAnswer(lwUIWnd kWnd, bool const bOK)
{
	EOX_DLG_STATE State = OXDS_START;
	XUI::CXUI_Wnd* pWnd = kWnd.GetSelf();
	if( !pWnd )
	{
		return;
	}

	pWnd->GetCustomData(&State, sizeof(EOX_DLG_STATE));
	BM::GUID kGuid = pWnd->OwnerGuid();

	switch( State )
	{
	case OXDS_START:
		{
			if( bOK )
			{
				BM::Stream kPacket(PT_C_M_REQ_OXQUIZ_ENTER);
				kPacket.Push(kGuid);
				NETWORK_SEND(kPacket);
			}
			pWnd->Close();
		}break;
	case OXDS_QUEST:
		{
			SetOXSwitchLight(pWnd, bOK);
		}break;
	}
}

void lwOXQuizUI::lwUpdateOXLight(lwUIWnd kWnd)
{
	if( kWnd.IsNil() )
	{
		return;
	}

	float fTime = kWnd.GetCustomData<float>();
	if( fTime > g_pkApp->GetAccumTime() )
	{
		return;
	}
	
	fTime = g_pkApp->GetAccumTime() + DFT_FLASH_TIME;
	kWnd.SetCustomData<float>(fTime);
	int iIndex = kWnd.GetUVIndex();
	if( iIndex == 2 )
	{
		kWnd.SetUVIndex(iIndex - 1);
	}
	else
	{
		kWnd.SetUVIndex(iIndex + 1);
	}
}

bool lwOXQuizUI::lwTickUpdateTimer(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pWnd = kWnd.GetSelf();
	if( !pWnd )
	{
		return false;
	}

	XUI::CXUI_Wnd*	pParent = pWnd;
	do
	{
		pParent = pParent->Parent();
	}while( pParent->Parent() );

	EOX_DLG_STATE State = OXDS_START;
	pParent->GetCustomData(&State, sizeof(EOX_DLG_STATE));

	if( OXDS_NOTICE == State )
	{
		return false;
	}
	
	XUI::CXUI_Wnd* pText = pWnd->GetControl(_T("FRM_TEXT"));
	XUI::CXUI_Wnd* pOne = pWnd->GetControl(_T("FRM_ONE"));
	XUI::CXUI_Wnd* pTen = pWnd->GetControl(_T("FRM_TEN"));
	if( !pText || !pOne || !pTen )
	{
		return false;
	}

	float fEndTime = 0.0f;
	pText->GetCustomData(&fEndTime, sizeof(fEndTime));
	float const fNowTime = g_pkApp->GetAccumTime();

	float fResultTime = fEndTime - fNowTime;
	int const iOne = static_cast<int>(fResultTime) % 10;
	int const iTen = static_cast<int>(fResultTime) / 10;

	pTen->Visible((0 < iTen)?(true):(false));
	pOne->Visible((iTen <= 0 && iOne <= 0)?(false):(true));
	if( false == pOne->Visible() )
	{
		return true;
	}

	pTen->UVUpdate(iTen + 1);
	pOne->UVUpdate(iOne + 1);

	return false;
}

void lwOXQuizUI::lwRequestOXAnswer(lwUIWnd kWnd)
{
	EOX_DLG_STATE State = OXDS_START;
	XUI::CXUI_Wnd* pWnd = kWnd.GetSelf();
	if( !pWnd )
	{
		return;
	}

	pWnd->GetCustomData(&State, sizeof(EOX_DLG_STATE));
	BM::GUID kGuid = pWnd->OwnerGuid();

	switch( State )
	{
	case OXDS_START:
		{
		}break;
	case OXDS_QUEST:
		{
			int const iOXResult = GetOXSwitchLight(pWnd);
			BM::Stream kPacket(PT_C_M_REQ_OXQUIZ_ANS_QUIZ);
			kPacket.Push(kGuid);
			kPacket.Push(iOXResult);
			NETWORK_SEND(kPacket);
			pWnd->Close();
		}break;
	}
}

void lwOXQuizUI::InitOXDialogUI(XUI::CXUI_Wnd* pkWnd)
{
	if( !pkWnd )
	{
		return;
	}
	pkWnd->OwnerGuid(BM::GUID());
	XUI::CXUI_Wnd* pOBg = pkWnd->GetControl(_T("FRM_O_BG"));
	XUI::CXUI_Wnd* pXBg = pkWnd->GetControl(_T("FRM_X_BG"));
	XUI::CXUI_Wnd* pO = pkWnd->GetControl(_T("FRM_O"));
	XUI::CXUI_Wnd* pX = pkWnd->GetControl(_T("FRM_X"));
	if( !pOBg || !pXBg || !pO || !pX )
	{
		return;
	}
	pOBg->Visible(false);
	pXBg->Visible(false);
	pO->Visible(false);
	pX->Visible(false);

	XUI::CXUI_Wnd* pText = pkWnd->GetControl(_T("FRM_MDL"));
	XUI::CXUI_Wnd* pBottom = pkWnd->GetControl(_T("FRM_BTM"));
	if( !pText || !pBottom )
	{
		return;
	}
	pText->Text(_T(""));
	
	XUI::CXUI_Button* pOkBtn = dynamic_cast<XUI::CXUI_Button*>(pBottom->GetControl(_T("FRM_O_OK")));
	XUI::CXUI_Button* pNoBtn = dynamic_cast<XUI::CXUI_Button*>(pBottom->GetControl(_T("FRM_O_NO")));
	XUI::CXUI_Button* pOk = dynamic_cast<XUI::CXUI_Button*>(pBottom->GetControl(_T("FRM_OK")));
	SetTimeLimit(pBottom, 0.0f);
	if( !pOkBtn || !pNoBtn || !pOk )
	{
		return;
	}
	pOkBtn->Text(_T(""));
	pNoBtn->Text(_T(""));
	pOk->Visible(false);
}

void lwOXQuizUI::CallOXInvitation(BM::Stream& rkPacket)
{
	EOX_DLG_STATE State = OXDS_START;

	BM::GUID kGuid;
	rkPacket.Pop(kGuid);
	if( kGuid.IsNull() )
	{
		return;
	}

	XUI::CXUI_Wnd*	pWnd = XUIMgr.Activate(_T("FRM_OX_QUIZ"));

	InitOXDialogUI(pWnd);
	pWnd->OwnerGuid(kGuid);
	pWnd->SetCustomData(&State, sizeof(EOX_DLG_STATE));

	XUI::CXUI_Wnd* pText = pWnd->GetControl(_T("FRM_MDL"));
	XUI::CXUI_Wnd* pBottom = pWnd->GetControl(_T("FRM_BTM"));
	if( !pText || !pBottom )
	{
		return;
	}
	SetMiddleText(pText, TTW(2301));
	pBottom->Location(POINT2(pBottom->Location().x, pText->Location().y + pText->Size().y));

	XUI::CXUI_Button* pOkBtn = dynamic_cast<XUI::CXUI_Button*>(pBottom->GetControl(_T("FRM_O_OK")));
	XUI::CXUI_Button* pNoBtn = dynamic_cast<XUI::CXUI_Button*>(pBottom->GetControl(_T("FRM_O_NO")));
	if( !pOkBtn || !pNoBtn )
	{
		return;
	}
	pOkBtn->Text(TTW(400533) + TTW(2200));
	pNoBtn->Text(TTW(400534) + TTW(2201));
	pOkBtn->Visible(true);
	pNoBtn->Visible(true);

	pWnd->Size(POINT2(pWnd->Size().x, pBottom->Location().y + pBottom->Size().y));

	SetTimeLimit(pBottom, DFT_INVITATION_TIME);
}

void lwOXQuizUI::CallOXQuestionDialog(BM::Stream& rkPacket)
{
	EOX_DLG_STATE State = OXDS_QUEST;

	BM::GUID kGuid;
	std::wstring kQuizText;
	int iCountTime = 0;

	rkPacket.Pop(kGuid);
	rkPacket.Pop(kQuizText);
	rkPacket.Pop(iCountTime);

	XUI::CXUI_Wnd*	pWnd = XUIMgr.Activate(_T("FRM_OX_QUIZ"));

	InitOXDialogUI(pWnd);
	pWnd->OwnerGuid(kGuid);
	pWnd->SetCustomData(&State, sizeof(OXDS_QUEST));

	XUI::CXUI_Wnd* pText = pWnd->GetControl(_T("FRM_MDL"));
	XUI::CXUI_Wnd* pBottom = pWnd->GetControl(_T("FRM_BTM"));
	XUI::CXUI_Wnd* pOBg = pWnd->GetControl(_T("FRM_O_BG"));
	XUI::CXUI_Wnd* pXBg = pWnd->GetControl(_T("FRM_X_BG"));
	if( !pText || !pBottom || !pOBg || !pXBg )
	{
		return;
	}
	SetMiddleText(pText, kQuizText);
	pBottom->Location(POINT2(pBottom->Location().x, pText->Location().y + pText->Size().y));
	pOBg->Visible(true);
	pXBg->Visible(true);

	XUI::CXUI_Button* pOkBtn = dynamic_cast<XUI::CXUI_Button*>(pBottom->GetControl(_T("FRM_O_OK")));
	XUI::CXUI_Button* pNoBtn = dynamic_cast<XUI::CXUI_Button*>(pBottom->GetControl(_T("FRM_O_NO")));
	if( !pOkBtn || !pNoBtn )
	{
		return;
	}
	pOkBtn->Text(TTW(2302) + TTW(2200));
	pNoBtn->Text(TTW(2303) + TTW(2201));
	pOkBtn->Visible(true);
	pNoBtn->Visible(true);

	pWnd->Size(POINT2(pWnd->Size().x, pBottom->Location().y + pBottom->Size().y));

	SetTimeLimit(pBottom, static_cast<float>(iCountTime));
}

void lwOXQuizUI::CallOXNoticeDialog(std::wstring const& kText, std::wstring const& kBtnText)
{
	EOX_DLG_STATE State = OXDS_NOTICE;

	XUI::CXUI_Wnd*	pWnd = XUIMgr.Activate(_T("FRM_OX_QUIZ"));

	InitOXDialogUI(pWnd);
	pWnd->SetCustomData(&State, sizeof(OXDS_QUEST));
	XUI::CXUI_Wnd* pText = pWnd->GetControl(_T("FRM_MDL"));
	XUI::CXUI_Wnd* pBottom = pWnd->GetControl(_T("FRM_BTM"));
	if( !pText || !pBottom )
	{
		return;
	}
	SetMiddleText(pText, kText);
	pBottom->Location(POINT2(pBottom->Location().x, pText->Location().y + pText->Size().y));

	XUI::CXUI_Button* pOkBtn = dynamic_cast<XUI::CXUI_Button*>(pBottom->GetControl(_T("FRM_O_OK")));
	XUI::CXUI_Button* pNoBtn = dynamic_cast<XUI::CXUI_Button*>(pBottom->GetControl(_T("FRM_O_NO")));
	XUI::CXUI_Button* pOk = dynamic_cast<XUI::CXUI_Button*>(pBottom->GetControl(_T("FRM_OK")));
	if( !pOkBtn || !pNoBtn || !pOk )
	{
		return;
	}
	pOkBtn->Visible(false);
	pNoBtn->Visible(false);
	pOk->Visible(true);
	pOk->Text(kBtnText + TTW(2200));
	pWnd->Size(POINT2(pWnd->Size().x, pBottom->Location().y + pBottom->Size().y));
}

void lwOXQuizUI::SetOXSwitchLight(XUI::CXUI_Wnd* pkWnd, bool const bIsOK)
{
	if( !pkWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pOFrm = pkWnd->GetControl(_T("FRM_O"));
	XUI::CXUI_Wnd* pXFrm = pkWnd->GetControl(_T("FRM_X"));
	if( !pOFrm || !pXFrm )
	{
		return;
	}
	pOFrm->Visible(bIsOK);
	pXFrm->Visible(!bIsOK);
}

int lwOXQuizUI::GetOXSwitchLight(XUI::CXUI_Wnd* pkWnd)
{
	if( !pkWnd )
	{
		return 0;
	}

	XUI::CXUI_Wnd* pOFrm = pkWnd->GetControl(_T("FRM_O"));
	XUI::CXUI_Wnd* pXFrm = pkWnd->GetControl(_T("FRM_X"));
	if( !pOFrm || !pXFrm )
	{
		return 0;
	}

	if( pOFrm->Visible() )
	{
		return 1;
	}
	else if( pXFrm->Visible() )
	{
		return 2;
	}
	else
	{
		return 0;
	}
}

void lwOXQuizUI::SetMiddleText(XUI::CXUI_Wnd* pMiddle, std::wstring const& kText)
{
	POINT2 kDefSize = pMiddle->Size();
	POINT2 kStrPos = pMiddle->TextPos();
	pMiddle->Size(POINT2(kDefSize.x - (DFT_FRAME_SIZE * 2), kDefSize.y));
	pMiddle->TextPos(POINT2(kStrPos.x - DFT_FRAME_SIZE, kStrPos.y));
	pMiddle->Text(kText);
	XUI::CXUI_Style_String kStyleString = pMiddle->StyleText();
	POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
	pMiddle->Size(POINT2(kDefSize.x, kTextSize.y));
	pMiddle->ImgSize(pMiddle->Size());
	pMiddle->TextPos(kStrPos);
	//pMiddle->Text(kText);
	pMiddle->SetInvalidate();
}

void lwOXQuizUI::SetTimeLimit(XUI::CXUI_Wnd* pParent, float fLimitTime)
{
	if( !pParent )
	{
		return;
	}

	int const iTen = static_cast<int>(fLimitTime) / 10;
	int const iOne = static_cast<int>(fLimitTime) % 10;

	XUI::CXUI_Wnd* pTen = pParent->GetControl(_T("FRM_TEN"));
	XUI::CXUI_Wnd* pOne = pParent->GetControl(_T("FRM_ONE"));
	XUI::CXUI_Wnd* pText = pParent->GetControl(_T("FRM_TEXT"));
	
	if( !pTen || !pOne || !pText )
	{
		return;
	}

	pTen->Visible((iTen)?(true):(false));
	pOne->Visible((iTen == 0 && iOne == 0)?(false):(true));
	pText->Visible(pOne->Visible());

	if( !pText->Visible() )
	{
		return;
	}

	pTen->UVUpdate(iTen + 1);
	pOne->UVUpdate(iOne + 1);
	float fEndTime = g_pkApp->GetAccumTime() + fLimitTime;
	pText->SetCustomData(&fEndTime, sizeof(fEndTime));
}

void lwOXQuizUI::RecvOXQuiz_Command(WORD const wPacketType, BM::Stream& rkPacket)
{
	switch( wPacketType )
	{
	case PT_M_C_ANS_OXQUIZ_ENTER:
		{
			EOXQUIZ_EVENT_RESULT iErrorType = OXQES_SUCCESS;
			rkPacket.Pop(iErrorType);
			if( OxQuizResult(iErrorType) )
			{
				CallOXNoticeDialog(TTW(2310), TTW(400533));
			}
		}break;
	case PT_M_C_ANS_OXQUIZ_EXIT:
		{
			//lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2309, true);
		}break;
	case PT_M_C_REQ_OXQUIZ_REQ_QUIZ:
		{
			CallOXQuestionDialog(rkPacket);
		}break;
	case PT_M_C_ANS_OXQUIZ_ANS_QUIZ:
		{
			EOXQUIZ_EVENT_RESULT iErrorType = OXQES_SUCCESS;
			rkPacket.Pop(iErrorType);
			OxQuizResult(iErrorType);
		}break;
	case PT_M_C_NOTI_OXQUIZ_OPEN:
		{
			CallOXInvitation(rkPacket);
		}break;
	case PT_M_C_NOTI_OXQUIZ_LOCK:
		{
			BM::GUID	kGuid;
			int			iTotal;
			rkPacket.Pop(kGuid);
			rkPacket.Pop(iTotal);
			wchar_t szTemp[MAX_PATH] = {0,};
			swprintf_s(szTemp, MAX_PATH, TTW(2308).c_str(), iTotal);
			CallOXNoticeDialog(szTemp, TTW(400533));
		}break;
	case PT_M_C_NOTI_OXQUIZ_RESULT:
		{
			bool bResult = false;
			int iLiveUser = 0;
			int iDeadUser = 0;
			std::wstring kDiscription;

			rkPacket.Pop(bResult);
			rkPacket.Pop(iLiveUser);
			rkPacket.Pop(iDeadUser);
			rkPacket.Pop(kDiscription);

			std::wstring kText;
			int iBtnNo = 400533;
			if( true == bResult )
			{//정답이다
				wchar_t szText[MAX_PATH] = {0, };
				swprintf_s(szText, MAX_PATH, TTW(2305).c_str(), iLiveUser);
				kText = szText;
			}
			else
			{//오답이다
				kText = TTW(2304);
				iBtnNo = 400542;
			}

			kText += _T("\n");
			kText += _T("\n");
			kText += kDiscription;

			CallOXNoticeDialog(kText, TTW(iBtnNo));
		}break;
	case PT_M_C_NOTI_OXQUIZ_CLOSE:
		{
			BM::GUID	kGuid;
			rkPacket.Pop(kGuid);
			CallOXNoticeDialog(TTW(2307), TTW(400533));
		}break;
	default:
		{
			//None Def Packet Type
		}break;
	}
}

bool lwOXQuizUI::OxQuizResult(EOXQUIZ_EVENT_RESULT const iErrorType)
{
	switch( iErrorType )
	{
	case OXQES_SUCCESS:
		{
			return true;
		}
	//OXQES_NOT_FOUND_EVENT,
	//OXQES_NOT_OPENED,
	//OXQES_ALREADY_JOINED,
	//OXQES_NOT_QUIZ_TIME,
	//OXQES_NOT_JOINED_EVENT,
	default:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 2319 + iErrorType, true);
		}break;
	}
	return false;
}
