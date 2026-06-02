#include "stdafx.h"

#include "lwUI.h"
#include "PgMail.h"
#include "PgPilotMan.h"
#include "PgUIScene.h"
#include "PgNetwork.h"
#include "lwTrade.h"
#include "lwCashItem.h"

void CallPostUI(lwGUID kNpcGuid)
{
	g_kMailMgr.NpcGuid(kNpcGuid());

	std::wstring const kPostUIName( _T("SFRM_POST") );
	CXUI_Wnd* pkTopWnd = XUIMgr.Activate( kPostUIName );
	if( pkTopWnd )
	{
		RegistUIAction(pkTopWnd);
	}
}

void ClosePostUI()
{
	g_kMailMgr.NpcGuid( BM::GUID::NullData() );
}

int lwCheckedMailCount(lwUIWnd kWnd)
{
	int iCount = 0;
	if (kWnd.IsNil())
	{
		return iCount;
	}

	XUI::CXUI_Wnd* pkPost0 = kWnd.GetControl("FRM_POST0").GetSelf();
	if (pkPost0 && pkPost0->Visible())
	{
		for (int i = 0; i < LETTER_PER_PAGE; ++i)
		{
			BM::vstring kName(L"FRM_LETTER");
			kName+=i;
			XUI::CXUI_Wnd* pkLetter = pkPost0->GetControl(kName);
			if (pkLetter)
			{
				XUI::CXUI_CheckButton* pkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkLetter->GetControl(L"CBTN_CHECK"));
				if (pkBtn)
				{
					iCount+=(int)pkBtn->Check();
				}
			}
		}
	}

	return iCount;
}

void CalcMassMailPostage()
{
	int const iCheckCount = g_kMailMgr.GetTotalCheckCount();

	lwUIWnd kPost = lwGetUIWnd("SFRM_POST");
	if (!kPost.IsNil())
	{
		lwUIWnd kPost2 = kPost.GetControl("FRM_POST2");
		if (!kPost2.IsNil())
		{
			__int64 i64Money = iCheckCount*SEND_MAIL_COST;
			for (int i = 2; i >= 0; --i)
			{
				BM::vstring kStr(L"FRM_POSTAGE");
				kStr+=i;
				int const iMoney = i64Money%100i64;
				BM::vstring kMoney(iMoney);

				XUI::CXUI_Wnd* pkCoin = kPost2.GetSelf()->GetControl(kStr);
				if (pkCoin)
				{
					pkCoin->Text(kMoney);
				}

				i64Money/=100i64;
			}

			lwUIWnd kCount = kPost2.GetControl("FRM_COUNT");
			if (!kCount.IsNil())
			{
				std::wstring kTemp;
				FormatTTW(kTemp, 925, iCheckCount);
				kCount.SetStaticTextW(lwWString(kTemp));
			}
		}
	}
}

bool lwREQ_MAIL_SEND(lwUIWnd kWnd)
{
	return g_kMailMgr.REQ_MAIL_SEND(kWnd.GetSelf());
}

bool lwCanAddToMail(lwUIWnd kWnd)//한단계 상위를 넣자. 그래야 매번 검사하는 항목이 줄어들지
{
	if (kWnd.IsNil())
	{
		return false;
	}

	XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
	int const iCustomData = kWnd.GetCustomData<int>();
	g_kMailMgr.AddCheckCount(iCustomData-1,-MAX_INVEN_COUNT);	//초기화

	GET_DEF(CItemDefMgr, kItemDefMgr);
	int iCount = 0;

	for (int i = 0; i < MAX_INVEN_COUNT; ++i)
	{
		BM::vstring kName(L"SFRM_ITEM");
		kName+=i;
		XUI::CXUI_Wnd* pkSForm = pkWnd->GetControl((std::wstring const &)kName);
		if (pkSForm)
		{
			XUI::CXUI_Image* pkIcon = dynamic_cast<XUI::CXUI_Image*>(pkSForm->GetControl(L"IMG_INV"));
			if (pkIcon)
			{
				XUI::CXUI_CheckButton* pkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkSForm->GetControl(L"CBTN_CHECK"));
				if (pkBtn)
				{
					pkBtn->Visible(false);

					int iItemNo = 0;

					PgBase_Item kItem;
					if(PgTradeMgr::CheckItemToGiveOther(SItemPos(iCustomData, pkIcon->BuildIndex()), g_kPilotMan.GetPlayerUnit(), kItem))
					{
						iItemNo = kItem.ItemNo();
						CItemDef const* pItemDef = kItemDefMgr.GetDef(iItemNo);
						if (pItemDef)
						{
							int const iAttr = pItemDef->GetAbil(AT_ATTRIBUTE);
							bool const bCanSend = !(iAttr & (ICMET_Cant_SendMail|ICMET_Cant_PlayerTrade));
							int const iCanSend = (int)bCanSend;
							iCount+=iCanSend;
							pkBtn->Visible(bCanSend);
							if (!bCanSend)
							{
								pkBtn->Check(false);
							}
							g_kMailMgr.AddCheckCount(iCustomData-1, bCanSend && pkBtn->Check());
						}
					}
					SetUIAddResToImage(pkIcon, iItemNo);
					lwDrawItemInPost(lwUIWnd(pkIcon), iItemNo);
					lwAccumlationExpCard::CheckUsingExpCardInfo(pkIcon, kItem, false);
				}
			}
		}
	}

	std::wstring kTemp;
	if (kWnd.IsVisible())
	{
		FormatTTW(kTemp, 926, iCount);
		XUI::CXUI_Wnd* pkAvali = kWnd.GetParent().GetParent().GetParent().GetControl("FRM_AVAILABLE").GetSelf();
		if (pkAvali)
		{
			pkAvali->Text(kTemp);
		}
	}

	FormatTTW(kTemp, 925, g_kMailMgr.GetTotalCheckCount());
	kWnd.GetParent().GetParent().GetParent().GetControl("FRM_COUNT").SetStaticTextW(lwWString(kTemp));

	CalcMassMailPostage(); // 매번 새로 계산

	kWnd.SetInvalidate();

	return true;
}

void lwCategoryBtnInit(lwUIWnd kWnd)
{
	if (kWnd.IsNil())
	{
		return;
	}

	XUI::CXUI_Wnd* pkParent = kWnd.GetParent().GetSelf();
	if (NULL == pkParent)
	{
		return;
	}

	int const iBuildIdx = kWnd.GetBuildIndex();
	for (int i = 0; i < 4; ++i)
	{
		BM::vstring kString(L"CBTN_CATEGORY");
		kString+=i;
		XUI::CXUI_CheckButton* pkCheck = dynamic_cast<XUI::CXUI_CheckButton*>(pkParent->GetControl((std::wstring const &)kString));
		if (pkCheck)
		{
			pkCheck->Check(iBuildIdx==i);
			pkCheck->ClickLock(iBuildIdx==i);
		}
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkParent->GetControl(L"LST_INV"));
	if (pkList)
	{
		XUI::SListItem *pItem = pkList->GetItemAt(0);
		if (pItem)
		{
			for (int i = 0; i < INVEN_KIND; ++i)
			{
				bool const bThis = iBuildIdx==i;
				BM::vstring kString(L"FRM_INVEN");
				kString+=i;
				XUI::CXUI_Wnd* pkChild = pItem->m_pWnd->GetControl((std::wstring const &)kString);
				if (pkChild)
				{
					pkChild->Visible(bThis);
					if (bThis)
					{
						lwCanAddToMail(pkChild);
					}
				}
			}
		}
	}
	kWnd.GetParent().GetControl("SFRM_LIST_BG").Visible(false);
}

void lwREQ_MAIL_RECV()
{
	g_kMailMgr.Init();
	g_kMailMgr.REQ_MAIL_RECV();

	SPT_C_M_POST_REQ_MAIL_MIN kStruct;
	BM::Stream kPacket;
	kStruct.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)
}

void lwInitMailUI(lwUIWnd kWnd)
{
	g_kMailMgr.Page(0);
	for (int i = 0; i < 3; ++i)
	{
		g_kMailMgr.InitUI(i, kWnd.GetSelf());
	}

	SPT_C_M_POST_REQ_MAIL_MIN kStruct;
	BM::Stream kPacket;
	kStruct.WriteToPacket(kPacket);
	NETWORK_SEND(kPacket)

	if (0==g_kMailMgr.Index() || g_kMailMgr.Index() > g_kMailMgr.GetLatestIndex())//서버에서 보낸거랑 내가 갖고있는거랑 비교
	{
		g_kMailMgr.REQ_MAIL_RECV(g_kMailMgr.Index());
	}
}

int lwGetNowMailCount()
{
	return g_kMailMgr.GetNowMailCount();
}

void lwPrevMailPage()
{
	g_kMailMgr.PrevPage();
}

void lwNextMailPage()
{
	g_kMailMgr.NextPage();
}

void lwDeleteMail()
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
	int iCount = 0;
	__int64 i64LastIndex = 0;
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
					i64LastIndex = g_kMailMgr.DeleteMailAt(i);
					if (0<=i64LastIndex)
					{
						++iCount;
					}
				}
				pkBtn->Check(false);
			}
		}
	}

	if (0<iCount)
	{
		if (LETTER_PER_PAGE <= g_kMailMgr.GetMailCount() - iCount)
		{
			//g_kMailMgr.MakeMailList();
		}
		else
		{
			g_kMailMgr.REQ_MAIL_RECV(i64LastIndex, -LETTER_PER_PAGE);
		}	
	}
}

void lwOpenMail(lwUIWnd kWnd)
{
	if (kWnd.IsNil()) { return; }

	int const iIndex = kWnd.GetBuildIndex();
	
	g_kMailMgr.OpenMailAt(iIndex, kWnd.GetSelf());	
}

void lwMakeMailList(lwUIWnd kWnd)
{
	g_kMailMgr.MakeMailList(kWnd.GetSelf());
}

void lwDisplayMailSrcIcon(lwUIWnd kWnd)
{
	if (kWnd.IsNil()) { return; }

	XUI::CXUI_Wnd *pWnd = kWnd.GetSelf();
	
	int const iNull = 0;
	SItemPos const & kItemPos = g_kMailMgr.SendItemPos();
	BM::GUID const & rkGuid = g_kMailMgr.SendItemGuid();
	PgBase_Item kItem;

	if (kItemPos != SItemPos::NullData() && !BM::GUID::IsNull(rkGuid))
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer){return;}

		PgInventory *pInv = pkPlayer->GetInven();
		if(!pInv){return;}

		POINT2 const & rPT = kWnd.GetTotalLocation()();

		if(S_OK == pInv->GetItem(kItemPos, kItem))
		{
			if(kItem.Guid() == rkGuid)
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
				if(pItemDef)
				{
					if( kItem.Count() && pItemDef->IsAmountItem())//소비형일 경우
					{
						SRenderTextInfo kRenderTextInfo;
						kRenderTextInfo.wstrText = (const wchar_t*)BM::vstring(kItem.Count());
						kRenderTextInfo.kLoc = rPT;
						kRenderTextInfo.wstrFontKey = FONT_TEXT;
						kRenderTextInfo.dwTextFlag|=XUI::XTF_OUTLINE;
						g_kUIScene.RenderText(kRenderTextInfo);
					}

					kWnd.DrawIconDetail_AddRes(pItemDef->ResNo(), rPT);
					g_kUIScene.RenderIcon( pItemDef->ResNo(), rPT);	
				}
				kWnd.SetCustomData<int>(kItem.ItemNo());
			}
		}
	}

	if( kItem.IsEmpty() )
	{
		kWnd.SetCustomData<int>(0);
	}

	lwAccumlationExpCard::CheckUsingExpCardInfo(pWnd, kItem, false);
}

void lwGetAnnex(lwUIWnd kWnd)
{
	if (kWnd.IsNil()) { return; }

	g_kMailMgr.GetAnnex(kWnd.GetSelf());
}

void lwGetAnnexAll()
{
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(pkUnit)
	{
		lwCallYesNoMsgBox(TTW(961), lwGUID(pkUnit->GetID()), MBT_CONFIRM_GET_ANNEX_ALL);
	}
}

void lwDrawItemInPost(lwUIWnd wnd, int iItemNo)
{
	if (wnd.IsNil()) { return; }

	XUI::CXUI_Image *pkWnd = dynamic_cast<XUI::CXUI_Image*>( wnd() );

	if (pkWnd == NULL) { return; }

	PgUISpriteObject *pkSprite = NULL;

	int const iNum = wnd.GetCustomData<int>();
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}
	
	PgBase_Item kItem;
	int iItemCount = 0;
	if(S_OK == pkPlayer->GetInven()->GetItem(SItemPos(iNum, wnd.GetBuildIndex()), kItem))
	{
		iItemCount = kItem.Count();
		if (0>=iItemNo)
		{	
			iItemNo = kItem.ItemNo();
		}
	}
	
	BM::vstring kCount;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
	if(pItemDef && pItemDef->IsAmountItem())//소비형일 경우
	{
		kCount = iItemCount;
	}

	wnd.SetStaticTextW(lwWString((std::wstring const &)kCount));

	pkSprite = g_kUIScene.GetIconTexture(iItemNo);

	if (!pkSprite) 
	{ 
		pkWnd->DefaultImgTexture(NULL);
		pkWnd->SetInvalidate();
		return; 
	}

	PgUIUVSpriteObject *pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);

	if(!pkUVSprite) { return; }

	pkWnd->DefaultImgTexture(pkUVSprite);

	SUVInfo &rkUV = pkUVSprite->GetUVInfo();
	pkWnd->UVInfo(rkUV);
	POINT2 kPoint(40*rkUV.U, 40*rkUV.V);//이미지 원본 사이즈 복구
	pkWnd->ImgSize(kPoint);	
	pkWnd->SetInvalidate();
}

void lwOnCallItemInPostToolTip(lwUIWnd kControl)
{
	if (kControl.IsNil()) {	return; }

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return;}

	PgBase_Item kItem;
	if(S_OK == pkPlayer->GetInven()->GetItem(SItemPos(kControl.GetCustomData<int>(), kControl.GetBuildIndex()), kItem))
	{
		CallToolTip_SItem(&kItem, kControl.GetTotalLocation());
	}
}

void lwReturnMail()
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
				SRecvMailInfoEx kMail;
				if (pkBtn->Check() )
				{
					g_kMailMgr.ReturnMailAt(i);
				}
				pkBtn->Check(false);
			}
		}
	}
}

void lwReturnMailAt(int const iAt)
{
	g_kMailMgr.ReturnMailAt(iAt);
}

void lwReplyMail(lwUIWnd kWnd)
{
	if (kWnd.IsNil())
	{
		return;
	}

	int const iAt = kWnd.GetCustomData<int>();

	SRecvMailInfoEx kMail;
	if (!g_kMailMgr.FindMail_ByAt(iAt, kMail)) {	return; }

	XUI::CXUI_Wnd* pkPost = lwActivateUI("SFRM_POST").GetSelf();
	if (!pkPost)
	{
		return;
	}

	XUI::CXUI_Wnd* pkLetterTo = NULL;

	for (int i = 0; i < 3; ++i)
	{
		BM::vstring kBtnName(L"CBTN_POST");
		kBtnName+=i;
		BM::vstring kPostName(L"FRM_POST");
		kPostName+=i;
		bool const bThis = 1==i;
		XUI::CXUI_CheckButton* pkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkPost->GetControl((std::wstring const &)kBtnName));
		
		if (pkBtn)
		{
			pkBtn->ClickLock(bThis);
			pkBtn->Check(bThis);
		}

		XUI::CXUI_Wnd* pkPost1 = pkPost->GetControl((std::wstring const &)kPostName);
		if (pkPost1)
		{
			pkPost1->Visible(bThis);
			if (bThis)
			{
				pkLetterTo = pkPost1;
			}
		}
	}

	if (pkLetterTo)
	{
		lwUIWnd UIEditTo = pkLetterTo->GetControl(L"EDT_TO");
		if( UIEditTo.IsNil() )
		{
			return ;
		}		

		lwUIWnd UIEditTitle = pkLetterTo->GetControl(L"EDT_TITLE");
		if( UIEditTitle.IsNil() )
		{
			return ;
		}

		std::wstring kTitle = TTW(930);
		kTitle += kMail.kMailTitle;
		if(MAX_MAIL_TITLE_LEN<kTitle.size())
		{
			kTitle = kTitle.substr(0,MAX_MAIL_TITLE_LEN);
		}
		
		UIEditTitle.SetEditTextW( kTitle );
		UIEditTo.SetEditTextW( kWnd.GetControl("FRM_FROM").GetStaticText().GetWString() );
	}

	kWnd.Close();
}

void lwCheckMassItem(lwUIWnd kWnd)
{
	if (kWnd.IsNil())
	{
		return;
	}

	lwUIWnd kParent = kWnd.GetParent();
	lwUIWnd kImg = kParent.GetControl("IMG_INV");
	if (!kImg.IsNil())
	{
		int const iCustomData = kImg.GetCustomData<int>();
		g_kMailMgr.AddCheckCount(iCustomData-1, kWnd.GetCheckState() ? -1 : 1);	//한단계 이전 상태를 확인해야 하기 때문에
	}

	CalcMassMailPostage();
}

bool lwREQ_MASSMAIL_SEND(lwUIWnd kWnd)
{
	return g_kMailMgr.REQ_MASSMAIL_SEND(kWnd.GetSelf());
}

void Recv_NOTI_NEW_MAIL()
{
	int const kCount = g_kMailMgr.TotalCount();
	lwUIWnd kMap = lwGetUIWnd("FRM_MINIMAP");
	if (!kMap.IsNil())
	{
		lwUIWnd kSub = kMap.GetControl("SFRM_SUB_MENU");
		if (!kSub.IsNil())
		{
			lwUIWnd kBtn = kSub.GetControl("IMG_LETTER");
			if (!kBtn.IsNil())
			{
				kBtn.Visible(false);
			}

			lwUIWnd kAlarm = kSub.GetControl("IMG_LETTER_ALARM");
			if (!kAlarm.IsNil())
			{
				kAlarm.Visible(true);
				kAlarm.ChangeImage( "../Data/6_ui/main/mnLT02.tga", false );
			}
		}
	}
}

void Update_NOTI_NEW_MAIL()
{
	int const kCount = g_kMailMgr.TotalCount();
	int const kNotReadCount = g_kMailMgr.NewMailCount();
	const float fRate = (float)kCount/(float)MAX_MAIL_COUNT;
	lwUIWnd kMap = lwGetUIWnd("FRM_MINIMAP");
	if (!kMap.IsNil())
	{
		lwUIWnd kSub = kMap.GetControl("SFRM_SUB_MENU");
		if (!kSub.IsNil())
		{
			lwUIWnd kAlarm = kSub.GetControl("IMG_LETTER_ALARM");
			if (!kAlarm.IsNil())
			{
				int iCustomData = 2;
				char szPath[255] = "../Data/6_ui/main/mnLT02.tga";
				if (kNotReadCount)
				{
					if (fRate<1.0f && fRate >= 0.75f)
					{
						iCustomData = 3;
					}
					else if (fRate>=1.0f)
					{
						iCustomData = 4;
					}
				}

				sprintf_s(szPath, 254, "../Data/6_ui/main/mnLT0%d.tga", iCustomData);
				lwUIWnd kBtn = kSub.GetControl("IMG_LETTER");
				if (!kBtn.IsNil())
				{
					kBtn.Visible(!kNotReadCount && fRate<0.75f);
				}
				kAlarm.Visible(!kBtn.IsVisible());
				kAlarm.ChangeImage( szPath, false );
				kAlarm.SetCustomData<int>(980+iCustomData);
			}
		}
	}
}

void lwClearPostSendItem()
{
	const SItemPos kItemPos(KUIG_ITEM_MAIL,0);
	g_kMailMgr.SetSrcItem(kItemPos);
}