#include "stdafx.h"
#include "Variant/Global.h"
#include "PgNetwork.h"
#include "PgMail.h"
#include "lwUI.h"
#include "xui/XUI_Manager.h"
#include "Lohengrin/GameTime.h"
#include "variant/PgEventView.h"
#include "PgPilotMan.h"
#include "PgCommandMgr.h"
#include "lwTrade.h"
#include "PgQuest.h"
#include "PgQuestUI.h"
#include "PgQuestDialog.h"

namespace Quest
{
	extern void SetCutedTextLimitLength(XUI::CXUI_Wnd *pkWnd, std::wstring const &rkText, std::wstring kTail, long const iCustomTargetWidth = 0);
}

const size_t TITLE_MAX_LEN = 15;		//DB와 상관없이 보여지는 길이제한
const size_t LETTER_TITLE_MAX_LEN = 21;
const int MAIL_BASE_TEXT_NO = 940;

PgMailMgr::PgMailMgr()
{
	Init();
}

PgMailMgr::~PgMailMgr()
{

}

void PgMailMgr::Init()
{
	Index(0i64);
	IsNewMailArrived(false);
	Page(0);
	ClearMailList();
	ReservePageDelta(0);
	MinIndex(0);
	TotalCount(0);
	NewMailCount(0);
	for ( int i = 0; i < INVEN_KIND; ++i)
	{
		m_iCheckCount[i] = 0;
	}
}

void PgMailMgr::ClearMailUI(XUI::CXUI_Wnd* pkWnd)
{
	if (pkWnd)
	{
		pkWnd->Text(L"");

		XUI::CXUI_Wnd* pkName = pkWnd->GetControl(L"FRM_NAME");
		pkName ? pkName->Text(L"") : 0;

		XUI::CXUI_CheckButton* pkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_CHECK"));
		if (pkBtn)
		{
			pkBtn->Check(false);
			pkBtn->Visible(false);
		}

		XUI::CXUI_Image* pkImg = dynamic_cast<XUI::CXUI_Image*>(pkWnd->GetControl(L"IMG_LETTER"));
		if (pkImg)
		{
			SUVInfo kUVInfo= pkImg->UVInfo();
			kUVInfo.Index = 1;
			pkImg->UVInfo(kUVInfo);
			pkImg->SetInvalidate();	
		}

		if(XUI::CXUI_Wnd* pkArrow = pkWnd->GetControl(L"IMG_ARROW_RETURN"))
		{
			pkArrow->Visible(false);
		}

		XUI::CXUI_Wnd* pkExpCard = pkWnd->GetControl(L"FRM_EXPCARD");
		if (pkExpCard)
		{
			pkExpCard->Visible(false);
		}

		XUI::CXUI_Icon* pkIcn = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(L"ICN_ITEM"));
		if (pkIcn)
		{
			SIconInfo kInfo(pkIcn->IconInfo().iIconGroup, pkWnd->BuildIndex());
			pkIcn->SetIconInfo(kInfo);
		}

		ClearText(pkWnd, L"FRM_DATE");
		ClearText(pkWnd, L"SFRM_TITLE");

		for(int k = 0; k < 3; ++k)
		{
			BM::vstring strCoin(L"FRM_COIN");
			strCoin+=k;
			ClearText(pkWnd, (std::wstring const &)strCoin);
			pkWnd->GetControl((std::wstring const &)strCoin)->Visible(false);
		}
	}
}

void PgMailMgr::InitUI(int const iIdx, XUI::CXUI_Wnd* pWnd)
{
	if (NULL==pWnd)
	{
		pWnd = XUIMgr.Get(L"SFRM_POST");
	}

	if (NULL==pWnd)
	{
		return;
	}

	BM::vstring strPost(L"FRM_POST");
	strPost+=iIdx;
	XUI::CXUI_Wnd* pPost = pWnd->GetControl((std::wstring const &)strPost);
	if (NULL==pPost)
	{
		return;
	}

	if (0==iIdx)
	{
		for (int i = 0; i < LETTER_PER_PAGE; ++i)
		{
			BM::vstring strName(L"FRM_LETTER");
			strName+=i;
			ClearMailUI(pPost->GetControl((std::wstring const &)strName));
		}
	}
	else if (1==iIdx)
	{
		ClearEditText(pPost, L"EDT_TO");

		BM::vstring kcost(TTW(918));
		ClearText(pPost, L"FRM_TEXT1", (std::wstring const &)kcost);
		ClearEditText(pPost, L"EDT_TITLE");
		ClearEditText(pPost, L"MEDT_CONTENTS");

		XUI::CXUI_Icon* pkIcn = dynamic_cast<XUI::CXUI_Icon*>(pPost->GetControl(L"ICN_ITEM"));
		if (pkIcn)
		{
			SIconInfo kInfo(pkIcn->IconInfo().iIconGroup, pkIcn->IconInfo().iIconKey);
			pkIcn->SetIconInfo(kInfo);
		}

		ClearText(pPost, L"FRM_TEXT4", TTW(922));

		XUI::CXUI_CheckButton* pkSend = dynamic_cast<XUI::CXUI_CheckButton*>(pPost->GetControl(L"CBTN_SEND_MONEY"));
		if (pkSend)
		{
			pkSend->Check(true);
			pkSend->ClickLock(true);
		}
		XUI::CXUI_CheckButton* pkReq = dynamic_cast<XUI::CXUI_CheckButton*>(pPost->GetControl(L"CBTN_REQ_MONEY"));
		if (pkReq)
		{
			pkReq->Check(false);
			pkReq->ClickLock(false);
		}
		for(int k = 0; k < 3; ++k)
		{
			BM::vstring strCoin(L"EDT_COIN");
			strCoin+=k;
			ClearEditText(pPost, (std::wstring const &)strCoin);
		}

		//SetSrcItem(SItemPos(0,0));
		SendItemPos(SItemPos(0,0));
		SendItemGuid(BM::GUID::NullData());
	}
	else if (2==iIdx)
	{
		ClearEditText(pPost, L"EDT_TO");
		ClearEditText(pPost, L"EDT_TITLE");
		ClearText(pPost, L"FRM_TEXT4", TTW(922));
		ClearText(pPost, L"FRM_COUNT");
		std::wstring kTemp;
		FormatTTW(kTemp, 926, 0);	
		ClearText(pPost, L"FRM_AVAILABLE", kTemp);

		lwCategoryBtnInit(lwUIWnd(pPost->GetControl(L"CBTN_CATEGORY0")));
		for ( int i = 0; i < INVEN_KIND; ++i)
		{
			m_iCheckCount[i] = 0;
		}
		for(int k = 0; k < 3; ++k)
		{
			BM::vstring strCoin(L"EDT_COIN");
			strCoin+=k;
			ClearEditText(pPost, (std::wstring const &)strCoin);
		}
	}
}

void PgMailMgr::ClearEditText(XUI::CXUI_Wnd* pkWnd, std::wstring const & kName)
{
	if (pkWnd)
	{
		XUI::CXUI_Edit* pkEdt = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(kName));
		if (pkEdt)
		{
			pkEdt->EditText(L"");
		}
	}
}

void PgMailMgr::ClearText(XUI::CXUI_Wnd* pkWnd, std::wstring const & kName, std::wstring const kText)
{
	if (pkWnd)
	{
		XUI::CXUI_Wnd* pkEdt = pkWnd->GetControl(kName);
		if (pkEdt)
		{
			pkEdt->Text(kText);
		}
	}
}

void PgMailMgr::ClearMailList()
{
	m_kMailList.clear();
	m_kNowList.clear();
}

bool PgMailMgr::FindMail_ByGuid(BM::GUID const& rkGuid, SRecvMailInfoEx& rkDest) const
{
	CONT_RECV_MAIL_EX_LIST::const_iterator it_mail = m_kMailList.begin();
	while (m_kMailList.end() != it_mail)
	{
		SRecvMailInfo const& rkMail = (*it_mail).second;

		if (rkGuid==rkMail.kMailGuid)
		{
			rkDest = rkMail;
			return true;
		}
		++it_mail;
	}

	return false;
}

bool PgMailMgr::CheckNewMail(CONT_RECV_MAIL_LIST const & rkCont)
{
	bool bNew = false;
	CONT_RECV_MAIL_LIST::const_iterator it = rkCont.begin();
	while (it!=rkCont.end())
	{
		SRecvMailInfo const & rkMail = (*it);
		if (PMMT_NOT_READ==rkMail.kMailState)
		{
			bNew = true;
			IsNewMailArrived(true);
			break;
		}
		++it;
	}

	return bNew;
}

bool PgMailMgr::RecvMail_Command(WORD const wPacketType, BM::Stream &rkPacket)
{
	switch(wPacketType)
	{
	case PT_M_C_POST_ANS_MAIL_SEND:
		{
			SPT_M_C_POST_ANS_MAIL_SEND kStruct;
			kStruct.ReadFromPacket(rkPacket);
			lwAddWarnDataTT(MAIL_BASE_TEXT_NO+kStruct.Result());
		}
		break;
	case PT_M_C_POST_ANS_MAIL_RECV:
		{
			SPT_M_C_POST_ANS_MAIL_RECV kStruct;
			kStruct.ReadFromPacket(rkPacket);
			if (PMRR_SUCCESS == kStruct.Result())
			{
				CONT_RECV_MAIL_LIST::const_iterator it = kStruct.Cont().begin();
				while (it!=kStruct.Cont().end())
				{
					SRecvMailInfoEx kInfoEx((*it));
					m_kMailList.insert(std::make_pair(kInfoEx.i64MailIndex, kInfoEx));
					++it;
				}
				CheckNewMail(kStruct.Cont());
				Page(Page()+ReservePageDelta());
				MakeMailList();
				TotalCount(kStruct.ucMailCount());
				NewMailCount(kStruct.ucNewMailCount());
				Update_NOTI_NEW_MAIL();
			}
			else
			{
				ReservePageDelta(0);
			}
		}
		break;
	case PT_M_C_POST_ANS_MAIL_MODIFY:
		{
			SPT_M_C_POST_ANS_MAIL_MODIFY kStruct;
			kStruct.ReadFromPacket(rkPacket);
			SRecvMailInfoEx kMail;
			EPostMailModifyResult const eResult = kStruct.Result();
			if (PMMR_SUCCESS == eResult)
			{
				if (FindMail_ByIndex(kStruct.MailIndex(), kMail))
				{
					if (!(kMail.kMailState & PMMT_READ))	//아직 안읽은 메일이면
					{
						NewMailCount(NewMailCount()-1);
					}
					EPostMailModifyType const kState = kStruct.ModifyType();
					if (kState&PMMT_DELETE /*|| (kState&PMMT_RETURN && !(kState&PMMT_READ))*/)
					{
						DeleteMail(kMail.i64MailIndex);	//지울땐 딴건 필요 없지
						TotalCount(__max(0,TotalCount()-1));
					}
					else //if (kState&PMMT_READ || kState&PMMT_GET_ANNEX || kState&PMMT_PAYMENT )
					{
						kMail.kMailState = kState;
						kMail.kLimitTime = kStruct.LimitTime();
						m_kMailList[kMail.i64MailIndex] = kMail;

						if(kState&PMMT_GET_ANNEX)	//아이템 획득인데
						{
							lwUIWnd kLetter = lwGetUIWnd("SFRM_LETTER");
							if(!kLetter.IsNil())
							{
								m_kNowList[kMail.i64MailIndex] = kMail;
								OpenMailAt(kLetter.GetCustomData<int>(), NULL);//밑에서 메일 리스트 업데이트 해 줄꺼라서
							}
						}
					}

					MakeMailList(NULL,false);

					Update_NOTI_NEW_MAIL();
				}
			}
			else
			{
#ifndef EXTERNAL_RELEASE
				BM::vstring kErr((int)eResult);
				switch(eResult)
				{
				case PMMR_CANT_RETURN:
					{
						kErr+=TTW(998);
					}break;
				default:
					{
						kErr+=TTW(eResult+970);
					}break;
				}

				Notice_Show(kErr, 1);
#else
				switch(eResult)
				{
				case PMMR_CANT_RETURN:
					{
						lwAddWarnDataTT(998);
					}break;
				default:
					{
						lwAddWarnDataTT((int)eResult+970);
					}break;
				}
#endif
				ReservePageDelta(0);
				//에러 메세지 출력
			}
			
		}
	    break;
	case PT_M_C_POST_NOTI_NEW_MAIL:
		{
			SPT_M_C_POST_NOTI_NEW_MAIL kStruct;
			kStruct.ReadFromPacket(rkPacket);
			SRecvMailInfoEx kInfoEx(kStruct.NewMail());
			m_kMailList.insert(std::make_pair(kInfoEx.i64MailIndex, kInfoEx));
			Index(kInfoEx.i64MailIndex);
			TotalCount(TotalCount()+1);
			NewMailCount(NewMailCount()+1);
		}break;
	case PT_M_C_POST_ANS_MAIL_MIN:
		{
			SPT_M_C_POST_ANS_MAIL_MIN kStruct;
			kStruct.ReadFromPacket(rkPacket);
			MinIndex(kStruct.MinIndex());
		}break;
	default:
	    break;
	}
	return true;
}

bool PgMailMgr::REQ_MAIL_SEND(XUI::CXUI_Wnd* pkWnd)
{
	if (NULL==pkWnd)
	{
		return false;
	}

	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(pkUnit)
	{
		if( pkUnit->GetAbil64(AT_MONEY) < SEND_MAIL_COST )
		{
			lwAddWarnDataTT(MAIL_BASE_TEXT_NO+PMSR_NOT_ENOUGH_MONEY);
			return false;
		}
	}

	SPT_C_M_POST_REQ_MAIL_SEND kStruct(m_kNpcGuid);

	XUI::CXUI_Edit* pkEdtTo = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(L"EDT_TO"));
	if (pkEdtTo)
	{
		std::wstring const & rkTo = pkEdtTo->EditText();
		if (rkTo == pkUnit->Name())
		{
			lwAddWarnDataTT(MAIL_BASE_TEXT_NO+PMSR_CANT_SEND_TO_SELF);
			return false;
		}
		kStruct.ToName( rkTo );
		if (kStruct.ToName().empty())
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", MAIL_BASE_TEXT_NO+PMSR_NOT_FOUND_RECVER, true);			
			return false;
		}
	}

	XUI::CXUI_Edit* pkEdtTitle = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(L"EDT_TITLE"));
	if(pkEdtTitle)
	{
		std::wstring kTitle = pkEdtTitle->EditText();
		if (kTitle.empty())
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 929, true);			
			return false;
		}
		else
		{
			if(!g_kUnicodeFilter.IsCorrect(UFFC_MAIL_SUBJECT, kTitle))
			{
				lwAddWarnDataTT(600037);
				return false;
			}
			g_kClientFS.Filter(kTitle, true);	//욕설 등이 있으면 제거			
		}
		kStruct.Title( kTitle );
	}

	XUI::CXUI_Edit* pkEdtContents = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(L"MEDT_CONTENTS"));
	if (pkEdtContents)
	{
		std::wstring kCont = pkEdtContents->EditText();
		if(!g_kUnicodeFilter.IsCorrect(UFFC_MAIL_CONTENTS, kCont))
		{
			lwAddWarnDataTT(600037);
			return false;
		}
		g_kClientFS.Filter(kCont, true);		//욕설 등이 있으면 제거		
		kStruct.Text(kCont);
	}

	kStruct.Money(0i64);
	
	XUI::CXUI_CheckButton *pkChbtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_REQ_MONEY"));
	if (pkChbtn)
	{
		bool const bReq = pkChbtn->Check();
		if (bReq && SendItemPos() == SItemPos::NullData())
		{
			lwAddWarnDataTT(MAIL_BASE_TEXT_NO+PMSR_PAYMENT_NEED_ITEM);
			return false;
		}
		
		kStruct.PayMent(bReq);
		for (int i = 0; i < 3; ++i)
		{
			kStruct.Money(kStruct.Money()*100i64);
			BM::vstring kStr(L"EDT_COIN");
			kStr+=i;
			XUI::CXUI_Edit* pkEdtGold = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl((std::wstring const &)kStr));
			if (pkEdtGold)
			{
				kStruct.Money(kStruct.Money()+_atoi64(MB(pkEdtGold->EditText().c_str())));
			}
		}
		if(bReq)
		{// 대금 청구에
			if(0 == kStruct.Money())
			{// 청구 금액이 입력되지 않았다면
				lwAddWarnDataTT(995);
				return false;
			}
		}
	}

	if (!kStruct.PayMent() && kStruct.Money() > pkUnit->GetAbil64(AT_MONEY)-SEND_MAIL_COST)	//보낼 돈이 적으면
	{
		lwAddWarnDataTT(985);
		return false;
	}

	kStruct.Pos(SendItemPos());

	BM::Stream kPacket;
	kStruct.WriteToPacket(kPacket);

	InitUI(1);

	NETWORK_SEND(kPacket);

	return bSendRet;
}

bool PgMailMgr::REQ_MASSMAIL_SEND(XUI::CXUI_Wnd* pkWnd)
{
	if (NULL==pkWnd)
	{
		return false;
	}

	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(pkUnit)
	{
		if( pkUnit->GetAbil64(AT_MONEY) < SEND_MAIL_COST*GetTotalCheckCount() )
		{
			lwAddWarnDataTT(MAIL_BASE_TEXT_NO+PMSR_NOT_ENOUGH_MONEY);
			return false;
		}
	}

	std::wstring kTo;
	std::wstring kTitle;
	XUI::CXUI_Edit* pkEdtTo = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(L"EDT_TO"));
	if (pkEdtTo)
	{
		kTo = pkEdtTo->EditText();
		if (kTo == pkUnit->Name())
		{
			lwAddWarnDataTT(MAIL_BASE_TEXT_NO+PMSR_CANT_SEND_TO_SELF);
			return false;
		}
		
		if (kTo.empty())
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", MAIL_BASE_TEXT_NO+PMSR_NOT_FOUND_RECVER, true);			
			return false;
		}
	}

	XUI::CXUI_Edit* pkEdtTitle = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl(L"EDT_TITLE"));
	if(pkEdtTitle)
	{
		kTitle =  pkEdtTitle->EditText();
		if (kTitle.empty())
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 929, true);			
			return false;
		}
		else if (kTitle.size() >= MAX_MAIL_TITLE_LEN)
		{
			kTitle = kTitle.substr(0,MAX_MAIL_TITLE_LEN-1);
		}
		if(!g_kUnicodeFilter.IsCorrect(UFFC_MAIL_SUBJECT, kTitle))
		{
			lwAddWarnDataTT(600037);
			return false;
		}
		g_kClientFS.Filter(kTitle, true);
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LST_INV"));
	if (NULL == pkList)
	{
		return false;
	}

	__int64 iMoney = 0i64;
	bool bReq = false;
	XUI::CXUI_CheckButton *pkChbtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_REQ_MONEY"));
	if (pkChbtn)
	{
		bReq = pkChbtn->Check();
		for (int i = 0; i < 3; ++i)
		{
			iMoney*=100i64;
			BM::vstring kStr(L"EDT_COIN");
			kStr+=i;
			XUI::CXUI_Edit* pkEdtGold = dynamic_cast<XUI::CXUI_Edit*>(pkWnd->GetControl((std::wstring const &)kStr));
			if (pkEdtGold)
			{
				iMoney+=_atoi64(MB(pkEdtGold->EditText().c_str()));
			}
		}
		if (!bReq)	//돈 보낼때 작으면
		{
			if(pkUnit)
			{
				if( pkUnit->GetAbil64(AT_MONEY) < (SEND_MAIL_COST+iMoney)*GetTotalCheckCount() )
				{
					lwAddWarnDataTT(MAIL_BASE_TEXT_NO+PMSR_NOT_ENOUGH_MONEY);
					return false;
				}
			}
		}

	}

	XUI::SListItem* pkItem = pkList->GetItemAt(0);
	if (pkItem)
	{
		TCHAR szBuf[255] = {0,};
		int iIndex = 1;
		for (int i = 0; i < INVEN_KIND; ++i)
		{
			BM::vstring kInvName(L"FRM_INVEN");
			kInvName+=i;
			XUI::CXUI_Wnd* pkInven = pkItem->m_pWnd->GetControl((std::wstring const &)kInvName);
			if (pkInven)
			{
				for (int k = 0; k < MAX_INVEN_COUNT; ++k)
				{
					BM::vstring kIconBgName(L"SFRM_ITEM");
					kIconBgName+=k;
					XUI::CXUI_Wnd* pkIconBG = pkInven->GetControl((std::wstring const &)kIconBgName);
					if (pkIconBG)
					{
						XUI::CXUI_CheckButton* pkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkIconBG->GetControl(L"CBTN_CHECK"));
						if (pkBtn && pkBtn->Check() && pkBtn->Visible())
						{
							SPT_C_M_POST_REQ_MAIL_SEND kStruct(m_kNpcGuid);
							wsprintfW(szBuf, L"(%d)", iIndex);
							size_t const wsCount = wcslen(szBuf);
							std::wstring kNewTitle = kTitle;
							if (kNewTitle.size() + wsCount>= MAX_MAIL_TITLE_LEN)
							{
								kNewTitle = kNewTitle.substr(0,MAX_MAIL_TITLE_LEN-1-wsCount);
							}

							kNewTitle+=szBuf;

							kStruct.Title(kNewTitle);
							kStruct.ToName(kTo);
							kStruct.PayMent(bReq);
							kStruct.Money(iMoney);
							kStruct.Pos(SItemPos(i+1,k));

							BM::Stream kPacket;
							kStruct.WriteToPacket(kPacket);
							NETWORK_SEND(kPacket)

							pkBtn->Check(false);
							++iIndex;
						}
					}
				}
			}
		}
	}

	InitUI(2);
	return true;
}

void PgMailMgr::REQ_MAIL_RECV(__int64 i64Index, int iCount)
{
	SPT_C_M_POST_REQ_MAIL_RECV kStruct;
	
	if (!m_kMailList.empty() && 0i64 == i64Index)
	{
		SRecvMailInfo const &kInfo = (*(m_kMailList.rbegin())).second;
		i64Index = kInfo.i64MailIndex;
	}

	Index(i64Index);

	kStruct.MailIndex(i64Index);
	kStruct.ReqCount(iCount);

	BM::Stream kPacket;
	kStruct.WriteToPacket(kPacket);

	NETWORK_SEND(kPacket)
}

void PgMailMgr::MakeMailList(XUI::CXUI_Wnd * pPost0, bool const bCheckReset)
{
	ReservePageDelta(0);
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(L"SFRM_POST");
	if (NULL==pWnd) 
	{ 
		if (!pPost0)
		{
			return; 
		}
		pWnd = pPost0->Parent();
	}

	if (NULL==pWnd) { return; }

	XUI::CXUI_Wnd* pPost[3] = {0,};
	for (int i = 0; i < 3; ++i)
	{
		BM::vstring kStr(L"FRM_POST");
		kStr+=i;
		pPost[i] = pWnd->GetControl((std::wstring const &)kStr);
		if (NULL!=pPost[i]) 
		{  
			pPost[i]->Visible(0==i);//0이 메일 목록이다.
		}		
	}
	
	//Page()는 0 BASE
	size_t const kSize = m_kMailList.size();
	int const iMaxPage = __min(Page(), kSize/LETTER_PER_PAGE);
	Page(__min(Page(), iMaxPage));	//마지막 페이지 넘지 말게
	int const iLeftCount = __min(kSize - LETTER_PER_PAGE*Page(), LETTER_PER_PAGE);
	if (0>=iLeftCount)
	{
		Page(__max(0, Page()-1));
	}

	//int const iLeftLetterNo = Page()*LETTER_PER_PAGE + iLeftCount;

	CONT_RECV_MAIL_EX_LIST::const_reverse_iterator mail_it = m_kMailList.rbegin();//한페이지에 8개. 최근순으로 정렬되어 있다고 가정
	for (int i = 0; i < Page()*LETTER_PER_PAGE ; ++i)
	{
		//출력해야 할 편지까지 돌자
		if (mail_it!=m_kMailList.rend())
		{
			++mail_it;
		}
	}

	m_kNowList.clear();	//8개를 유지하자
	
	for (int i = 0; i < LETTER_PER_PAGE; ++i)	//역순으로 넣자
	{
		BM::vstring kStr(L"FRM_LETTER");
		kStr+=i;
		XUI::CXUI_Wnd* pkLetter = pPost[0]->GetControl((std::wstring const &)kStr);

		if (mail_it!=m_kMailList.rend())
		{
			m_kNowList.insert(std::make_pair((*mail_it).first, (*mail_it).second));
			SRecvMailInfoEx const & rkMailInfo = (*mail_it).second;
			WriteToUI(rkMailInfo, pkLetter, bCheckReset);
			++mail_it;
		}
		else
		{
			ClearMailUI(pkLetter);	//없는놈은 클리어
		}
	}

	if (pPost[0] && pPost[0]->Visible())
	{
		XUI::CXUI_Wnd* pkPage = pPost[0]->GetControl(L"SFRM_PAGE");
		if (pkPage)
		{
			BM::vstring kStrPage(Page()+1);
			kStrPage+=L"/";

			kStrPage+=(__max(0,TotalCount()-1)/LETTER_PER_PAGE +1);
			pkPage->Text((std::wstring const &)kStrPage);
		}
	}
}

bool PgMailMgr::WriteToUI(SRecvMailInfoEx const& rkSrc, XUI::CXUI_Wnd* pkWnd, bool const bCheckReset)
{
	if (NULL==pkWnd) { return false; }
	XUI::CXUI_Wnd* pkImg = pkWnd->GetControl(L"IMG_LETTER");
	bool const bRead = rkSrc.kMailState&PMMT_READ;
	if (pkImg)
	{
		SUVInfo kUVInfo= pkImg->UVInfo();
		kUVInfo.Index = int(!bRead) + 1;
		pkImg->UVInfo(kUVInfo);
		pkImg->SetInvalidate();
	}

	PgBase_Item kItem;
	kItem.Clear();

	XUI::CXUI_CheckButton* pkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_CHECK"));
	if (pkBtn)
	{
		if(bCheckReset)
		{
			pkBtn->Check(false);
		}
		pkBtn->Visible(true);
	}

	std::wstring kReq;
	AttachHeadString(rkSrc, kReq);

	if(XUI::CXUI_Wnd* pkArrow = pkWnd->GetControl(L"IMG_ARROW_RETURN"))
	{
		pkArrow->Visible(rkSrc.kMailState&PMMT_RETURN);
	}

	XUI::PgFontDef kFontDef(g_kFontMgr.GetFont(L"Font_Text"));
	XUI::CXUI_Style_String kSText(kFontDef, kReq);
	size_t const kPreText = kSText.Length();
	
	if (kPreText + rkSrc.kMailTitle.size() > TITLE_MAX_LEN)
	{
		kReq+=(rkSrc.kMailTitle.substr(0, TITLE_MAX_LEN - 1 - kPreText) + L"...");
	}
	else
	{
		kReq+=rkSrc.kMailTitle;
	}
	ClearText(pkWnd, L"SFRM_TITLE", kReq);

	__int64 i64Money = rkSrc.i64Money;

	for (int i = 2; i >= 0; --i)
	{
		BM::vstring kStr(L"FRM_COIN");
		kStr+=i;
		int iMoney = 0;
		if (0==i)
		{
			iMoney = i64Money;
		}
		else
		{
			iMoney = i64Money%100i64;
		}

		XUI::CXUI_Wnd* pkCoin = pkWnd->GetControl(kStr);
		if (pkCoin)
		{
			if(rkSrc.kMailState&PMMT_GET_ANNEX)
			{// 돈을 이미 얻었으면 돈을 출력하지 않고
				pkCoin->Visible(false);
				pkCoin->Text(std::wstring());
			}
			else
			{// 돈이 존재 한다면, 돈을 출력 한다				
				BM::vstring kMoney;
				if(rkSrc.kMailState&PMMT_PAYMENT || rkSrc.bPamentType)
				{//빨간색 추가
					kMoney = L"{C=";
					kMoney += TTW(4006);
					kMoney += L"/}";
				}
				kMoney += iMoney;
				pkCoin->Visible(rkSrc.i64Money);
				pkCoin->Text(kMoney);
			}
		}

		i64Money/=100i64;
	}
	
	Quest::SetCutedTextLimitLength(pkWnd->GetControl(L"FRM_NAME"), rkSrc.kFromName, L"...");

	XUI::CXUI_Icon* pkIcn = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(L"ICN_ITEM"));
	if (pkIcn)
	{
		// 이미 얻은 아이템이라면, 아이콘을 감춘다
		pkIcn->Visible(!(0<rkSrc.kItem.ItemNo() && rkSrc.kMailState&PMMT_GET_ANNEX));

		XUI::CXUI_Wnd* pkExpCard = pkWnd->GetControl(L"FRM_EXPCARD");
		if (pkExpCard)
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(rkSrc.kItem.ItemNo());
			if(!pDef || pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) != UICT_EXPCARD )
			{
				pkExpCard->Visible(false);
			}
			else
			{
				pkExpCard->Visible(pkIcn->Visible());
			}			
		}
	}

	BM::vstring kDay(CalcRestDay(rkSrc.kLimitTime));
	kDay+=TTW(908);
	ClearText(pkWnd, L"FRM_DATE", (std::wstring const &)kDay);
	
	return !kItem.IsEmpty();
}

int PgMailMgr::CalcRestDay(BM::DBTIMESTAMP_EX const & rkLimit)
{
	__int64 i64GameTime = g_kEventView.GetLocalSecTime();
	__int64 i64MailTime = 0i64;
	CGameTime::AceTime2SecTime(rkLimit, i64MailTime);
	int iDeltaDay = __max(0i64,(i64MailTime-i64GameTime)/60i64/60i64/24i64);

	return iDeltaDay;
}

PgBase_Item const * PgMailMgr::GetItemAt(int const iAt) const
{
	if (iAt >= m_kNowList.size() || m_kNowList.empty() || iAt < 0)
	{
		return NULL;
	}

	CONT_RECV_MAIL_EX_LIST::const_reverse_iterator mail_it = m_kNowList.rbegin();//한페이지에 8개. 최근순으로 정렬되어 있다고 가정
	for (int i = 0; i < iAt; ++i)
	{
		++mail_it;
	}

	return &((*mail_it).second.m_kItem);
}

bool PgMailMgr::DeleteMail(__int64 const & rkIndex)
{
	CONT_RECV_MAIL_EX_LIST::iterator mail_it = m_kMailList.find(rkIndex);
	if (mail_it != m_kMailList.end())
	{
		m_kMailList.erase(mail_it);
		return true;
	}
	return false;
}

__int64 PgMailMgr::DeleteMailAt(int const iIndex)
{
	SRecvMailInfoEx kMail;
	if (!FindMail_ByAt(iIndex, kMail))
	{
		return -1i64;
	}

	if ( 0<kMail.kItem.ItemNo() || 0i64<kMail.i64Money)	
	{
		if (!(kMail.kMailState&PMMT_GET_ANNEX))	//아직 물건을 안받았는데
		{
			lwAddWarnDataTT(956);
			return -1i64;
		}
	}

	SPT_C_M_POST_REQ_MAIL_MODIFY kStruct(m_kNpcGuid);
	kStruct.MailGuid(kMail.kMailGuid);
	kStruct.MailModifyType(PMMT_DELETE);
	BM::Stream kPacket;
	kStruct.WriteToPacket(kPacket);

	NETWORK_SEND(kPacket)

	return kMail.i64MailIndex;
}

void PgMailMgr::NextPage()
{
	if (LETTER_PER_PAGE >= TotalCount())	//총 메일이 8통 이하면 이전 페이지가 없잖아?
	{
		ReservePageDelta(0);
		return;
	}

	size_t const kSize = m_kMailList.size();
	if(0>kSize)
	{
		return;
	}

	ReservePageDelta(1);

	CONT_RECV_MAIL_EX_LIST::const_iterator it = m_kMailList.begin();
	if(it==m_kMailList.end())
	{
		return;
	}

	__int64 const & i64Index = (*it).second.i64MailIndex;	//내가 갖고있는 메일 중에 제일 빠른 번호
	if (MinIndex() < i64Index)
	{
		size_t const kRestSize = kSize-(Page()+1)*LETTER_PER_PAGE;
		REQ_MAIL_RECV(i64Index,kRestSize-LETTER_PER_PAGE);
	}
	else
	{
		Page(Page()+ReservePageDelta());
		MakeMailList();
	}
}

void PgMailMgr::PrevPage()
{
	if (0>=Page())
	{
		return;
	}
	if (0<Page())
	{
		ReservePageDelta(-1);
		Page(Page()+ReservePageDelta());
		MakeMailList();
	}
}

bool PgMailMgr::FindMail_ByAt(int const iAt, SRecvMailInfoEx& rkDest)
{
	if (iAt >= m_kNowList.size() || m_kNowList.empty() || iAt < 0)
	{
		return false;
	}

	CONT_RECV_MAIL_EX_LIST::const_reverse_iterator mail_it = m_kNowList.rbegin();//한페이지에 8개. 최근순으로 정렬되어 있다고 가정
	for (int i = 0; i < iAt; ++i)
	{
		++mail_it;
	}

	rkDest = (*mail_it).second;

	return true;
}

bool PgMailMgr::FindMail_ByIndex(__int64 const i64Index, SRecvMailInfoEx& rkDest)
{
	CONT_RECV_MAIL_EX_LIST::iterator mail_it = m_kMailList.find(i64Index);
	if (m_kMailList.end()!=mail_it)
	{
		rkDest = (*mail_it).second;
		return true;
	}

	return false;
}

void PgMailMgr::OpenMailAt(int const iIndex, XUI::CXUI_Wnd* pkWnd)
{
	SRecvMailInfoEx kMail;
	if (!FindMail_ByAt(iIndex, kMail)) {	return; }

	lwUIWnd kLetter = lwActivateUI("SFRM_LETTER");
	if (kLetter.IsNil()) { return; }

	kLetter.SetCustomData<int>(iIndex);//번호 저장

	std::wstring kTitle;
	AttachHeadString(kMail, kTitle);
	XUI::PgFontDef kFontDef(g_kFontMgr.GetFont(L"Font_Text"));
	XUI::CXUI_Style_String kSText(kFontDef, kTitle);
	size_t const kPreText = kSText.Length();

	if (kPreText + kMail.kMailTitle.size() > LETTER_TITLE_MAX_LEN)
	{
		kTitle+=(kMail.kMailTitle.substr(0, LETTER_TITLE_MAX_LEN - 1 - kPreText) + L"...");
	}
	else
	{
		kTitle+=kMail.kMailTitle;
	}

	ClearText(kLetter.GetSelf(), L"SFRM_TITLE_TEXT", kTitle);
	Quest::SetCutedTextLimitLength(kLetter()->GetControl(L"FRM_FROM"), kMail.kFromName, L"...");
	ClearText(kLetter.GetSelf(), L"SFRM_SDW", kMail.kMailText);

	__int64 i64Money = kMail.i64Money;
	
	for (int i = 2; i >= 0; --i)
	{
		BM::vstring kStr(L"FRM_COIN");
		kStr+=i;
		int iMoney = i64Money%100i64;
		if ( 0 == i )
		{
			iMoney = i64Money;
		}
				 
		lwUIWnd kCoin = kLetter.GetSelf()->GetControl((std::wstring const &)kStr);
		if(!kCoin.IsNil())
		{
			if(kMail.kMailState&PMMT_GET_ANNEX)
			{// 이미 얻은 우편이라면 골드를 표시하지 않고				
				ClearText(kLetter.GetSelf(), (std::wstring const &)kStr, std::wstring());
				kCoin.Visible(false);
			}
			else
			{// 아직 얻지 않은 우편이라면 골드를 표시한다
				BM::vstring kMoney;
				kMoney+=iMoney;
				kLetter.GetSelf()->GetControl((std::wstring const &)kStr)->Visible(kMail.i64Money);
				ClearText(kLetter.GetSelf(), (std::wstring const &)kStr, (std::wstring const &)kMoney);				
			}
			i64Money/=100i64;
		}		
	}

	lwUIWnd kItemIcon = kLetter.GetControl("ICN_ITEM");
	if (!kItemIcon.IsNil())
	{
		kItemIcon.SetIconKey(iIndex);	//빌드인덱스로 맞추면 될듯
		//bool const bGray = 0<kMail.dwItemNo && kMail.kMailState&PMMT_GET_ANNEX;
		//kItemIcon.SetGrayScale(bGray);
		if(0<kMail.kItem.ItemNo() && kMail.kMailState&PMMT_GET_ANNEX)
		{
			kItemIcon.Visible(false);
		}
		else
		{
			kItemIcon.Visible(true);
		}

		lwUIWnd kExpCard = kLetter.GetControl("FRM_EXPCARD");
		if( !kExpCard.IsNil() )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(kMail.kItem.ItemNo());
			if(!pDef || pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) != UICT_EXPCARD )
			{
				kExpCard.Visible(false);
			}
			else
			{
				kExpCard.Visible(true);
			}			
		}
	}

	if (!(PMMT_READ&kMail.kMailState))	//않읽은 놈이면
	{
		SendModify(kMail, PMMT_READ);
	}

	//////////////////////////////////////////////////////////////////////////
	//UI
	//////////////////////////////////////////////////////////////////////////
	if (pkWnd)
	{
		kLetter.SetCustomData<int>(iIndex);
		XUI::CXUI_Wnd* pkImg = pkWnd->GetControl(L"IMG_LETTER");
		if (pkImg)
		{
			SUVInfo kUVInfo= pkImg->UVInfo();
			kUVInfo.Index = 1;
			pkImg->UVInfo(kUVInfo);
			pkImg->SetInvalidate();	
		}

		int const iRestDay = CalcRestDay(kMail.kLimitTime);
		if (REST_DAY<iRestDay)
		{
			BM::vstring kDay(REST_DAY);
			kDay+=TTW(908);
			ClearText(pkWnd, L"FRM_DATE", (std::wstring const &)kDay);
		}
	}
}

void PgMailMgr::SendModify(SRecvMailInfo const & rkMail, EPostMailModifyType eType)
{
	PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	PgInventory *pkInven = (pkPC)? pkPC->GetInven(): NULL;

	switch( eType )
	{
	case PMMT_GET_ANNEX:
		{
			if( !pkInven )
			{
				return;
			}

			CONT_RECV_MAIL_EX_LIST::const_iterator find_iter = m_kMailList.find( rkMail.i64MailIndex );
			if( m_kMailList.end() == find_iter )
			{
				return;
			}

			CONT_RECV_MAIL_EX_LIST::mapped_type const &rkElement = (*find_iter).second;
			if(!PgBase_Item::IsEmpty(&(rkElement.m_kItem)) && S_OK != pkInven->CanInsert(rkElement.m_kItem) )
			{
				lwAddWarnDataTT(400715);
				return;
			}
		}break;
	default:
		{
		}break;
	}

	SPT_C_M_POST_REQ_MAIL_MODIFY kStruct(m_kNpcGuid);
	kStruct.MailGuid(rkMail.kMailGuid);
	kStruct.MailModifyType(eType);
	BM::Stream kPacket;
	kStruct.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)
}

bool PgMailMgr::SetSrcItem(SItemPos const &rkItemPos)
{
	if(0==rkItemPos.x && 0==rkItemPos.y)	//0,0이면 애초에 필요없음
	{
		return false;
	}
	SendItemPos(SItemPos(0,0));
	SendItemGuid(BM::GUID::NullData());
	
	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(L"SFRM_POST");
	XUI::CXUI_Wnd* pRemove = NULL;
	if(pWnd)
	{
		XUI::CXUI_Wnd* pTemp = pWnd->GetControl(L"FRM_POST1");
		if(pTemp && pTemp->Visible())
		{
			pTemp = pTemp->GetControl(L"SFRM_SDW");
			if(pTemp)
			{
				pRemove = pTemp->GetControl(L"BTN_REMOVE");
				if(pRemove)
				{
					pRemove->Visible(false);
				}
			}
		}
	}
	if( KUIG_ITEM_MAIL == rkItemPos.x )
	{
		return false;
	}

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();

	PgBase_Item kItem;
	if(!PgTradeMgr::CheckItemToGiveOther(rkItemPos, pkPlayer, kItem))	//자기 자신이 클릭 되는건 괜찮다
	{
		lwAddWarnDataTT(MAIL_BASE_TEXT_NO+PMSR_CANT_SEND_ITEM);
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if (!pDef) { return false; }
	
	EItemCantModifyEventType const eAbil = (EItemCantModifyEventType)(pDef->GetAbil(AT_ATTRIBUTE));
	if (eAbil&(ICMET_Cant_SendMail|ICMET_Cant_PlayerTrade))
	{
		lwAddWarnDataTT(MAIL_BASE_TEXT_NO+PMSR_CANT_SEND_ITEM);
		return false;
	}

	SendItemPos(rkItemPos);
	SendItemGuid(kItem.Guid());

	if(pRemove)
	{
		pRemove->Visible(true);
	}
	return true;
}

void PgMailMgr::GetAnnex(XUI::CXUI_Wnd* pkWnd)
{
	if (!pkWnd) { return; }

	int kIndex = 0;
	pkWnd->GetCustomData(&kIndex, sizeof(kIndex));

	SRecvMailInfoEx kMail;

	if (FindMail_ByAt(kIndex, kMail))
	{
		if (0>=kMail.kItem.ItemNo() && 0>=kMail.i64Money)
		{
			return;
		}
		else if( kMail.kMailState&PMMT_GET_ANNEX )
		{
			lwAddWarnDataTT(955);
			return;
		}
		else if (kMail.bPamentType && !(kMail.kMailState & PMMT_RETURN))	//반환된 우편이면 그냥 받자
		{
			CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
			if(pkUnit)
			{
				if( pkUnit->GetAbil64(AT_MONEY) < kMail.i64Money )
				{
					lwAddWarnDataTT(968);
					return;
				}
				else
				{
					std::vector< unsigned int > kCont;
					if(BreakMoney_UInt(kMail.i64Money, kCont))
					{
						int const iItemNo = kMail.kItem.ItemNo();
						if(kCont.size() > 2
							&& iItemNo > 0)
						{
							std::wstring kOutText;
							std::wstring kItemName;
							::GetItemName(iItemNo, kItemName);
							::FormatTTW(kOutText, 997, kItemName.c_str(), kCont[0], kCont[1], kCont[2]);
							lwCallYesNoMsgBox(kOutText, lwGUID(pkUnit->GetID()), MBT_CONFIRM_PAYMENT_MAIL, kIndex);
						}
					}
				}
			}
		}
		else
		{
			if (!kMail.m_kItem.IsEmpty())
			{
				CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
				if (pkUnit)
				{
					PgInventory* pkInv = pkUnit->GetInven();
					if (pkInv && S_OK == pkInv->CanInsert(kMail.m_kItem))
					{
						SendModify(kMail, PMMT_GET_ANNEX);			
						return;
					}

					lwAddWarnDataTT(700037);
				}
			}
			else if (0<kMail.i64Money)
			{
				SendModify(kMail, PMMT_GET_ANNEX);
			}
		}
	}
}

void PgMailMgr::GetAnnex(int const iIndex, bool bOnlyNoPay)
{
	SRecvMailInfoEx kMail;
	if (FindMail_ByAt(iIndex, kMail))
	{
		if (0>=kMail.kItem.ItemNo() && 0i64>=kMail.i64Money)
		{
			return;
		}
		if (kMail.bPamentType)
		{
			if (bOnlyNoPay)	//대금청구 메일인데 아닌것들만 할려고 하면
			{
				return;
			}
		}
		SendModify(kMail, PMMT_GET_ANNEX);	
	}
}

void PgMailMgr::GetAnnexAll()
{
	lwUIWnd kWndParnt = lwGetUIWnd("SFRM_POST");
	if (kWndParnt.IsNil())
	{
		return; 
	}

	lwUIWnd kWnd = kWndParnt.GetControl("FRM_POST0");
	if (kWnd.IsNil() || !kWnd.IsVisible())
	{
		return; 
	}

	lwUIWnd kBtnAll = kWnd.GetControl("BTN_ALL");
	if (false == kBtnAll.IsNil())
	{
		kBtnAll.CheckState(false);
	}

	char szName[255] = {0,};
	for (int i = 0; i < LETTER_PER_PAGE; ++i)
	{
		sprintf(szName, "FRM_LETTER%d", i);
		lwUIWnd kLetter = kWnd.GetControl(szName);
		if (!kLetter.IsNil())
		{
			XUI::CXUI_CheckButton* pkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(kLetter.GetControl("CBTN_CHECK").GetSelf());
			if (pkBtn)
			{
				if (pkBtn->Check())
				{
					g_kMailMgr.GetAnnex(i, true);
				}
				pkBtn->Check(false);
			}
		}
	}
}

void PgMailMgr::ReturnMailAt(int const iAt)
{
	SRecvMailInfoEx kMail;
	if (FindMail_ByAt(iAt, kMail))
	{
		if ((0<kMail.kItem.ItemNo() || 0i64<kMail.i64Money) && !(kMail.kMailState&PMMT_GET_ANNEX) )
		{
			SendModify(kMail, PMMT_RETURN);
		}
	}
}

__int64 const PgMailMgr::GetLatestIndex() const
{
	if (!m_kMailList.empty())
	{
		SRecvMailInfoEx const& rkMail = (*m_kMailList.rbegin()).second;
		return rkMail.i64MailIndex;
	}
	return 0i64;
}

int PgMailMgr::AddCheckCount(int iType, int iValue)
{
	if (0 > iType || INVEN_KIND <=iType )
	{
		return 0;
	}
	m_iCheckCount[iType]+=iValue;

	m_iCheckCount[iType] = __min(MAX_INVEN_COUNT, m_iCheckCount[iType]);
	m_iCheckCount[iType] = __max(0, m_iCheckCount[iType]);

	return m_iCheckCount[iType];
}
int PgMailMgr::GetTotalCheckCount()
{
	int iTotal = 0;
	for (int i = 0; i < INVEN_KIND; ++i)
	{
		iTotal+=m_iCheckCount[i];
	}

	return iTotal;
}

void AttachHeadString(SRecvMailInfo const & rkMail, std::wstring& rkStr)
{
	if (rkMail.kMailState&PMMT_RETURN)	//반송된 우편
	{
		rkStr+=TTW(980);
	}
	if (rkMail.kMailState&PMMT_PAYMENT && rkMail.bPamentType)	//대금청구
	{
		rkStr+=TTW(906);
	}
}