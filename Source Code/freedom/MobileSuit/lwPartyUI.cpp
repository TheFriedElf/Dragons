#include "StdAfx.h"
#include "lwUI.h"
#include "lwPartyUI.h"
#include "PgNetwork.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgWorld.h"
#include "ServerLib.h"
#include "PgMobileSuit.h"
#include "PgXmlLoader.h"
#include "PgItemEx.h"
#include "PgDropBox.h"
#include "PgAction.h"
#include "Variant/PgPlayer.h"
#include "Variant/constant.h"
#include "PgQuest.h"
#include "lwUIQuest.h"
#include "PgRemoteManager.h"
#include "PgNifMan.h"

#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "PgRenderMan.h"
#include "PgHelpSystem.h"
#include "pgchatMgrClient.h"
#include "Lohengrin/PacketStruct4Map.h"

#include "PgCmdLineParse.h"

std::wstring const WSTR_ORG_OK_CANCEL_BOX = _T("OK_CANCEL_BOX");
//std::wstring const WSTR_TRG_OK_CANCEL_BOX = _T("OK_CANCEL_BOX%u");

extern bool FormatMoney(__int64 const iTotal, std::wstring &rkOut);

void lwCallYesNoMsgBox(lwWString kMessage, lwGUID kOrderGuid, int const iBoxType, int const iExternInt, bool bUICloseRegistAction)
{
	CallYesNoMsgBox(kMessage(), kOrderGuid(), (EMsgBoxType)iBoxType, iExternInt, bUICloseRegistAction);
}

void CallYesNoMsgBox(std::wstring const &wstrMessage, BM::GUID const &rkOrderGuid, const EMsgBoxType iBoxType, int const iExternInt, bool bUICloseRegistAction)
{
	if(	MBT_CONFIRM_EXIT == iBoxType
	&&	g_kCmdLineParse.IsUseForceAccount())
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 66, true);
		return;
	}

	BM::vstring kStrMsgBox;	
	switch( iBoxType )
	{
	case MBT_CONFIRM_ENTERTUTORIAL:
		{
			kStrMsgBox = WSTR_ORG_OK_CANCEL_BOX + L"_CLOSE";
		}break;
	default:
		{
			kStrMsgBox = WSTR_ORG_OK_CANCEL_BOX;
		}
	}

	size_t iCount = 0;
	BM::vstring kStrTemp;
	CXUI_Wnd *pkTempWnd = NULL;
	do
	{
		kStrTemp = kStrMsgBox + iCount;		
		pkTempWnd = XUIMgr.Get(kStrTemp);
		++iCount;
	} while( pkTempWnd );

	XUI::CXUI_MsgBox* pMsgBox = dynamic_cast<XUI::CXUI_MsgBox*>(XUIMgr.Call(kStrMsgBox, true, kStrTemp));
	if( pMsgBox )
	{
		XUI::CXUI_Wnd *pkFrm = pMsgBox->GetControl(_T("FRM_MESSAGE"));
		if( pkFrm )
		{
			pkFrm->Text(wstrMessage);

			XUI::CXUI_Style_String kStyleString = pkFrm->StyleText();
			POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
			int const iFormSizeY = 108;
			pkFrm->TextPos( POINT2(pkFrm->TextPos().x, (iFormSizeY - kTextSize.y) / 2) );
		}

		XUI::PgMessageBoxInfo Info = pMsgBox->BoxInfo();
		Info.iBoxType = iBoxType;
		Info.kOrderGuid = rkOrderGuid;
		//Info.kExtGuid = rkExtendGuid;
		
		pMsgBox->BoxInfo(Info);
		pMsgBox->SetCustomData(&iExternInt, sizeof(iExternInt));

		int iOk = 96, iCancel = 97;
		switch(iBoxType)
		{
		case MBT_CONFIRM_EXITGAME_AND_WEBPAGE:
			{
				pMsgBox->Priority(10);
				pMsgBox->SetEditFocus(true); 
			}
		case MBT_CONFIRM_INSTANCE_COUPLE:
		case MBT_CONFIRM_COUPLE:case MBT_PARTY_JOIN:
		case MBT_GUILD_JOIN:
		case MBT_CONFIRM_SWEETHEART_QUEST:
		case MBT_CONFIRM_SWEETHEART_COMPLETE:
		case MBT_CONFIRM_ACCEPT_SHAREQUEST:
			{
				iOk = 400536, iCancel = 400537;
			}break;
		case MBT_CONFIRM_ENTERTUTORIAL:
			{
				iOk = 252, iCancel = 400647;
			}break;
		default:
			{
			}break;
		}

		XUI::CXUI_Wnd *pkOK = pMsgBox->GetControl(_T("BTN_OK"));
		XUI::CXUI_Wnd *pkCancel = pMsgBox->GetControl(_T("BTN_CANCEL"));
		if( pkOK )		{ pkOK->Text(TTW(iOk)); }
		if( pkCancel )	{ pkCancel->Text(TTW(iCancel)); }
		if(	bUICloseRegistAction ) { RegistUIAction(pMsgBox); }
	}
}

void CallYesNoMsgBoxLimit(std::wstring const &wstrMessage, BM::GUID const &rkOrderGuid, const EMsgBoxType iBoxType, std::wstring const &kName)
{
	if(!g_pkWorld)
	{
		return;
	}
	size_t iCount = 0;
	TCHAR szTemp[512] = {0, };
	CXUI_Wnd *pkTempWnd = NULL;
	do
	{
		std::wstring const WSTR_TRG_OK_CANCEL_BOXLIMIT = _T("OK_CANCEL_BOXLIMIT%u");
		_stprintf_s(szTemp, WSTR_TRG_OK_CANCEL_BOXLIMIT.c_str(), iCount);
		pkTempWnd = XUIMgr.Get( std::wstring(szTemp) );
		++iCount;
	} while( pkTempWnd );

	XUI::CXUI_MsgBox* pMsgBox = dynamic_cast<XUI::CXUI_MsgBox*>(XUIMgr.Call(_T("OK_CANCEL_BOXLIMIT"), true, std::wstring(szTemp)));
	if( pMsgBox )
	{
		XUI::CXUI_Wnd *pkFrm = pMsgBox->GetControl(_T("FRM_MESSAGE"));
		if( pkFrm )
		{
			pkFrm->Text(wstrMessage);

			XUI::CXUI_Style_String kStyleString = pkFrm->StyleText();
			POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
			int const iFormSizeY = 108;
			pkFrm->TextPos( POINT2(pkFrm->TextPos().x, (iFormSizeY - kTextSize.y) / 2) );
		}

		XUI::PgMessageBoxInfo Info = pMsgBox->BoxInfo();
		Info.iBoxType = iBoxType;
		Info.kOrderGuid = rkOrderGuid;
		
		pMsgBox->BoxInfo(Info);
		pMsgBox->SetCustomData(kName.c_str(), sizeof(std::wstring::value_type)*kName.size());

		int iOk = 96, iCancel = 97, iRefuse = 799367;
// 		switch(iBoxType)
// 		{
// 		default:
// 			{
//				iOk = 96, iCancel = 97;
//			}break;
		//		}


		XUI::CXUI_Wnd *pkOK = pMsgBox->GetControl(_T("BTN_OK"));
		XUI::CXUI_Wnd *pkCancel = pMsgBox->GetControl(_T("BTN_CANCEL"));
		if( pkOK )		{ pkOK->Text(TTW(iOk)); }
		if( pkCancel )	{ pkCancel->Text(TTW(iCancel)); }
		XUI::CXUI_Wnd *pkRefuse = pMsgBox->GetControl(_T("BTN_REFUSE"));
		if( pkRefuse )	{ pkRefuse->Text(TTW(iRefuse));	}
		pkRefuse->Visible(false);
		
		if( (0 != ((GATTR_MISSION | GATTR_CHAOS_MISSION) & g_pkWorld->GetAttr())) 
			&& (!g_kParty.PartyState()) )
		{
			lua_tinker::call<void, lwUIWnd, lwUIWnd, lwUIWnd>("PartyJoinWndButtonPosModify", lwUIWnd(pkOK), lwUIWnd(pkCancel), lwUIWnd(pkRefuse));
		}
	}
}

void lwFranYesNoMsgBox(lwGUID kNpcGuid)
{
	__int64 iNow = GetNowFran();

	if( iNow )
	{
		std::wstring kFormatMoney;
		__int64 const iAbsVal = static_cast<__int64>((iNow * FRAN_REWORD_MONEY)/100);
		if( !FormatMoney(iAbsVal, kFormatMoney) )
		{
			kFormatMoney = (std::wstring)BM::vstring((int)iAbsVal);
		}

		std::wstring kLog;
		bool const bRet = FormatTTW(kLog, 550008, iNow, iNow, kFormatMoney.c_str());
		if( !bRet )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(pkPlayer)
		{
			CallYesNoMsgBox(kLog, kNpcGuid(), MBT_FRAN_CHANGE, 0, true);
		}		
	}
	else
	{
		lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(550010)), true);
	}
}

namespace PgClientPartyUtil
{
	void Refresh_Part_People_FindWnd()
	{
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_COMMUNITY"));
		if( !pkTopWnd )
		{
			return;
		}

		XUI::CXUI_Wnd* pkTopFom = pkTopWnd->GetControl(_T("FRM_PARTY"));
		if( !pkTopFom )
		{
			return;
		}

		XUI::CXUI_Button* pkButtonWnd = dynamic_cast<XUI::CXUI_Button*>(pkTopFom->GetControl(_T("BTN_PARTY_PEOPLE_FIND")));
		if( !pkButtonWnd )
		{
			return;
		}

		if( IsInParty() )
		{
			pkButtonWnd->Text(TTW(401207));
		}
		else
		{
			pkButtonWnd->Text(TTW(401208));
		}

		XUI::CXUI_Button* pkButtonCreateWnd = dynamic_cast<XUI::CXUI_Button*>(pkTopFom->GetControl(_T("BTN_PARTY_CREATE")));
		if( !pkButtonCreateWnd )
		{
			return;
		}

		if( IsInParty() )
		{
			pkButtonCreateWnd->Text(TTW(400565));
		}
		else
		{
			pkButtonCreateWnd->Text(TTW(400576));
		}
	}

	void RefreshPartyInfoWnd(lwUIWnd lwPartyFrm)//'P' 누르면 뜨는 파티창
	{//lwPartyFrm == "FRM_PARTY"
		XUI::CXUI_Form *pkForm = (XUI::CXUI_Form*)lwPartyFrm();
		if( !pkForm )
		{
			XUI::CXUI_Form *pkTopFrm = (XUI::CXUI_Form*)XUIMgr.Get(std::wstring(_T("SFRM_COMMUNITY")));
			if( !pkTopFrm )
			{
				return;
			}
			pkForm = (XUI::CXUI_Form*)pkTopFrm->GetControl(std::wstring(_T("FRM_PARTY")));
			if( !pkForm )
			{
				return;
			}
		}

		XUI::CXUI_Wnd* pkTitleWnd = pkForm->GetControl(_T("SFRM_TITLE"));
		if( pkTitleWnd )
		{
			if( GUID_NULL == g_kParty.PartyGuid() )
			{
				pkTitleWnd->Text(_T(""));
			}
			else
			{
				pkTitleWnd->Text(g_kParty.PartyName());
			}
		}

		XUI::CXUI_List *pkListWnd = (XUI::CXUI_List*)pkForm->GetControl(std::wstring(_T("LST_PARTY_LIST")));
		if( !pkListWnd )
		{
			return;
		}

		//std::wstring MasterName = g_kParty.Get_Party_Master_Name();
		BM::GUID const &rkMasterGuid = g_kParty.MasterGuid();

		ContPartyMember kPartyList;
		g_kParty.GetPartyMemberList(kPartyList);

		XUI::SListItem* pItem = pkListWnd->FirstItem();
		ContPartyMember::const_iterator member_itor = kPartyList.begin();
		int iCustomCount = 1;
		while( kPartyList.end() != member_itor )
		{
			const SPartyMember *pkMember = (*member_itor);
			if( !pItem )
			{
				pItem = pkListWnd->AddItem(_T(""),0);
			}

			if(	pItem
			&&	pkMember )
			{
				XUI::CXUI_Form *pForm = (XUI::CXUI_Form*)pItem->m_pWnd;
				if(pForm)
				{
					pForm->OwnerGuid(pkMember->kCharGuid);//캐릭GUID
					pForm->SetCustomData(&iCustomCount, sizeof(iCustomCount));

					XUI::CXUI_Form *pkNameForm = (XUI::CXUI_Form*)pForm->GetControl(std::wstring(_T("FRM_NAME")));
					if( pkNameForm )
					{
						pkNameForm->Text(pkMember->kName);//사람이름

						//DWORD const dwFontFlag = pkNameForm->FontFlag();
						//if( pkMember->kCharGuid == rkMasterGuid )//파티장이다.
						//{
						//	pkNameForm->FontFlag(dwFontFlag|XTF_BOLD);//Bold
						//}
						//else
						//{
						//	pkNameForm->FontFlag(dwFontFlag&(XTF_FULL_FLAG^XTF_BOLD));//Bold 업음
						//}
					}

					XUI::CXUI_Wnd *pImg = pForm->GetControl(std::wstring(_T("IMG_CLASS")));
					lwSetMiniClassIconIndex(pImg, pkMember->iClass);

					XUI::CXUI_Wnd *pkLevel = pForm->GetControl(std::wstring(_T("FRM_LEVEL")));
					if( pkLevel )
					{
						BM::vstring kLevel(pkMember->sLevel);
						pkLevel->Text(kLevel);
					}

					XUI::CXUI_Wnd *pkLocation = pForm->GetControl(std::wstring(_T("FRM_LOCATION")));
					if( pkLocation )
					{
						int iCustom = 0;
						std::wstring kMapName = GetMapName(pkMember->GroundNo());
						Quest::SetCutedTextLimitLength(pkLocation,kMapName, _T("..."));
					}
					XUI::CXUI_Wnd* pMyHomewnd = pForm->GetControl(L"BTN_MYHOME_GO");
					if( pMyHomewnd )
					{
						XUI::CXUI_Button* pkMyhomeGo = dynamic_cast<XUI::CXUI_Button*>(pMyHomewnd);
						if( pkMyhomeGo )
						{
							pkMyhomeGo->Disable( pkMember->kHomeAddr.IsNull() );
						}
					}
				}
			}
			++iCustomCount;
			++member_itor;
			pItem = pkListWnd->NextItem(pItem);
		}
		while(pItem)
		{
			pItem = pkListWnd->DeleteItem(pItem);
		}
		return;
	}

	void RefreshPartyStateWnd()//미니 파티창 갱신
	{
		if ( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_NOPARTY) )
		{
			return;
		}

		PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
		if( !pkPC )
		{return;}

		XUI::CXUI_Form *pPartyStateWnd = dynamic_cast<XUI::CXUI_Form*>(XUIMgr.Get(_T("FRM_PARTY_STATE")));
		if( !pPartyStateWnd )
		{return;}

		/*XUI::CXUI_List *pList = dynamic_cast<XUI::CXUI_List*>(pPartyStateWnd->GetControl(_T("MEMBER_LIST")));
		if( !pList )
		{return;}*/

		BM::GUID const &rkMasterGuid = g_kParty.MasterGuid();

		ContPartyMember kPartyList;
		g_kParty.GetPartyMemberList(kPartyList);

		//XUI::SListItem* pItem = pList->FirstItem();
		bool bNoParty = BM::GUID::NullData() == rkMasterGuid;
		XUI::CXUI_Button* pkSwitchBtn = dynamic_cast<XUI::CXUI_Button*>(pPartyStateWnd->GetControl(L"BTN_SWITCH"));
		if ( pkSwitchBtn )
		{
			pkSwitchBtn->Visible(!bNoParty);
		}

		if( !bNoParty )
		{
			SPartyMember const* pkMyMemberInfo = NULL;
			if( !g_kParty.GetMember(pkPC->GetID(), pkMyMemberInfo) )
			{
				return;
			}

			bool const bIamMaster = IsPartyMaster(pkPC->GetID());

			ContPartyMember::const_iterator member_itor = kPartyList.begin();
			//while(member_itor != kPartyList.end() && iCount<PV_MAX_MEMBER_CNT)
			for(int iCount=0; iCount<PV_MAX_MEMBER_CNT; ++iCount)
			{
				BM::vstring kWndName(L"PARTY_MEMBER_STATE");
				kWndName+=iCount;
				XUI::CXUI_Wnd* pkWnd = pPartyStateWnd->GetControl(kWndName);
				if( !pkWnd ){ continue; }

				if ( member_itor != kPartyList.end() )
				{
					if( iCount == (PV_MAX_MEMBER_CNT - 1) )
					{//파티 참여 허용 버튼은 마지막 파티원 자리에 나타나기 때문에 4번째 파티원이 있으면 꺼준다.
						XUI::CXUI_Wnd * BreakInWnd = pPartyStateWnd->GetControl(L"FRM_BREAK_IN");
						if( BreakInWnd )
						{
							BreakInWnd->Visible(false);
						}
					}

					SPartyMember *pkMember = (*member_itor);
					++member_itor;

					bool const bMyGuid = (pkMember->kCharGuid == pkPC->GetID());
					if ( pkMember )
					{
						std::wstring kTempName = pkMember->kName;
						DWORD dwNameColor = 0xffffffff;

						XUI::CXUI_Wnd* pMasterWnd = pkWnd->GetControl(_T("FRM_MASTER"));
						if( pMasterWnd )
						{
							if( pkMember->kCharGuid == rkMasterGuid )//마스터 이면
							{
								pMasterWnd->Visible(true);
							}
							else
							{
								pMasterWnd->Visible(false);
							}
						}

						if( bMyGuid )//내 윈도우면
						{
							dwNameColor = 0xffffff00;//색상만 노랑
						}

						pkWnd->Visible(true);
						SUVInfo kUVInfo= pkWnd->UVInfo();
						if(kUVInfo.Index != 1)
						{
							kUVInfo.Index = 1;
							pkWnd->UVInfo(kUVInfo);
							pkWnd->SetInvalidate();	
						}
						pkWnd->FontColor(dwNameColor);
						pkWnd->Text(kTempName);//사람이름
						pkWnd->OwnerGuid(pkMember->kCharGuid);//캐릭GUID

						CXUI_Wnd* pLevelFrm = pkWnd->GetControl(_T("FRM_LEVEL"));
						if( pLevelFrm )
						{
							BM::vstring kLevel(TTW(224));
							kLevel+=L". ";
							kLevel+=(int)pkMember->sLevel;
							pLevelFrm->Text(kLevel);
						}

						CXUI_AniBar* pHpBar = dynamic_cast<CXUI_AniBar*>(pkWnd->GetControl(_T("BAR_MEMBER_HP")));
						if( pHpBar )
						{
							pHpBar->Now(pkMember->sHP);
							pHpBar->Max(PMCA_MAX_PERCENT);
						}

						CXUI_AniBar* pMpBar = dynamic_cast<CXUI_AniBar*>(pkWnd->GetControl(_T("BAR_MEMBER_MP")));
						if( pMpBar )
						{
							pMpBar->Now(pkMember->sMP);
							pMpBar->Max(PMCA_MAX_PERCENT);
						}

						XUI::CXUI_Wnd *pImg = pkWnd->GetControl(_T("IMG_CLASS"));
						if(pImg)
						{
							pImg->Visible(true);
							lwSetMiniClassIconIndex(pImg, pkMember->iClass);
						}


						XUI::CXUI_Wnd* pWalkImg = pkWnd->GetControl(_T("IMG_WALK"));
						if( pWalkImg )
						{
							if( pkMember->GroundNo() != pkMyMemberInfo->GroundNo() )
							{
								pWalkImg->Visible(true);
							}
							else
							{
								pWalkImg->Visible(false);
							}
						}

						XUI::CXUI_Wnd* pBtnClose = pkWnd->GetControl(_T("BTN_CLOSE"));
						if( pBtnClose )
						{
							if(bMyGuid)
							{
								pBtnClose->Visible(true);
							}
							else
							{
								pBtnClose->Visible(false);
							}
						}
					}

				}
				else
				{
					pkWnd->Visible(true);
					
					if( iCount == (PV_MAX_MEMBER_CNT - 1) )
					{
						bool const EnablePartyBreakIn = lwEnablePartyBreakIn();
						XUI::CXUI_Wnd * BreakInWnd = pPartyStateWnd->GetControl(L"FRM_BREAK_IN");
						if( BreakInWnd )
						{
							BreakInWnd->Visible(EnablePartyBreakIn);
							XUI::CXUI_CheckButton * pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(BreakInWnd->GetControl(L"CHK_BREAK_IN"));
							if( pCheckBtn )
							{
								if( EnablePartyBreakIn )
								{
									BYTE PartyState = g_kParty.Option().GetOptionState();
									pCheckBtn->Check( !(PartyState & EPartyRefuse::EPR_CLIENT) );
								}
							}
						}

						if( EnablePartyBreakIn )
						{
							pkWnd->Visible(false);
						}
					}

					pkWnd->OwnerGuid(BM::GUID::NullData());
					SUVInfo kUVInfo= pkWnd->UVInfo();
					if(kUVInfo.Index != 2)
					{
						kUVInfo.Index = 2;
						pkWnd->UVInfo(kUVInfo);
						pkWnd->SetInvalidate();	
					}
					pkWnd->Text(L" ");

					XUI::CXUI_Wnd* pMasterWnd = pkWnd->GetControl(_T("FRM_MASTER"));
					if ( pMasterWnd )
					{
						pMasterWnd->Visible(false);
					}
					CXUI_Wnd* pLevelFrm = pkWnd->GetControl(_T("FRM_LEVEL"));
					if( pLevelFrm )
					{
						pLevelFrm->Text(L" ");
					}

					CXUI_AniBar* pHpBar = dynamic_cast<CXUI_AniBar*>(pkWnd->GetControl(_T("BAR_MEMBER_HP")));
					if( pHpBar )
					{
						pHpBar->Now(0);
						pHpBar->Max(0);
					}

					CXUI_AniBar* pMpBar = dynamic_cast<CXUI_AniBar*>(pkWnd->GetControl(_T("BAR_MEMBER_MP")));
					if( pMpBar )
					{
						pMpBar->Now(0);
						pMpBar->Max(0);
					}

					XUI::CXUI_Wnd *pImg = pkWnd->GetControl(_T("IMG_CLASS"));
					if(pImg)
					{
						pImg->Visible(false);
					}

					XUI::CXUI_Wnd* pWalkImg = pkWnd->GetControl(_T("IMG_WALK"));
					if( pWalkImg )
					{
						pWalkImg->Visible(false);
					}

					XUI::CXUI_Wnd* pBtnClose = pkWnd->GetControl(_T("BTN_CLOSE"));
					if( pBtnClose )
					{
						pBtnClose->Visible(false);
					}
				}
			}
		}
		else	//네비게이션 보여야 됨
		{
			for(int iCount=0; iCount<PV_MAX_MEMBER_CNT; ++iCount)
			{
				BM::vstring kWndName(L"PARTY_MEMBER_STATE");
				kWndName+=iCount;
				XUI::CXUI_Wnd* pkWnd = pPartyStateWnd->GetControl(kWndName);
				if ( pkWnd )
				{
					pkWnd->Visible(false);
					pkWnd->OwnerGuid(BM::GUID::NullData());
				}
			}
			XUI::CXUI_Wnd * BreakInWnd = pPartyStateWnd->GetControl(L"FRM_BREAK_IN");
			if( BreakInWnd )
			{
				BreakInWnd->Visible(false);
			}
		}
		return;
	}

	int GetPartyMemberLocation(lwGUID kCharGuid)
	{
		SPartyMember const* pkMember = NULL;
		if( !g_kParty.GetMember(kCharGuid(), pkMember) )
		{
			return 0;
		}

		return pkMember->GroundNo();
	}

	int GetPartyOptionExp()
	{
		const SPartyOption& rkOption = g_kParty.Option();
		return rkOption.GetOptionExp();
	}

	int GetPartyOptionItem()
	{
		const SPartyOption& rkOption = g_kParty.Option();
		return rkOption.GetOptionItem();
	}

	int GetPartyOptionPublicTitle()
	{
		const SPartyOption& rkOption = g_kParty.Option();
		return rkOption.GetOptionPublicTitle();
	}

	int GetOptionAttribute()
	{
		const SPartyOption& rkOption = g_kParty.Option();
		return rkOption.GetOptionAttribute();
	}

	int GetOptionContinent()
	{
		const SPartyOption& rkOption = g_kParty.Option();
		return rkOption.GetOptionContinent();
	}

	int GetOptionArea_NameNo()
	{
		const SPartyOption& rkOption = g_kParty.Option();
		return rkOption.GetOptionArea_NameNo();
	}

	lwWString GetPartyOptionWStr()
	{
		std::wstring kTemp;
		bool const bRet = PartyOptionStr(g_kParty.Option(), kTemp, false);
		return lwWString(kTemp);
	}

	bool GetPartyOptionState()
	{
		const SPartyOption& rkOption = g_kParty.Option();
		return rkOption.GetOptionState();
	}

	void SetPartyOptionNew(int const iAttribute, int const iContinent, int const iArea, int const MaxMember, lwWString kPartySubName)
	{
		g_kParty.TempPartyOption = g_kParty.Option();

		g_kParty.TempPartyOption.SetOptionAttribute(iAttribute);
		g_kParty.TempPartyOption.SetOptionContinent(iContinent);
		g_kParty.TempPartyOption.SetOptionArea_NameNo(iArea);
		g_kParty.TempPartyOption.SetOptionMaxMember(MaxMember);
		g_kParty.TempPartyOption.PartySubName(kPartySubName());
	}


	void SetPartyOption(int const iOptionExp, int const iOptionItem, int const iOptionPublicTitle, int const iLevel, bool const bState)
	{
		//SPartyOption kOption = g_kParty.Option();
		SPartyOption kOption = g_kParty.TempPartyOption;
		kOption.SetOptionExp((EPartyOptionExp)iOptionExp);
		kOption.SetOptionItem((EPartyOptionItem)iOptionItem);
		kOption.SetOptionPublicTitle((EPartyOptionPublicTitle)iOptionPublicTitle);
		kOption.SetOptionLevel(iLevel);
		kOption.SetOptionState(bState);
	#ifndef EXTERNAL_RELEASE
		EPartyOptionExp eExp = kOption.GetOptionExp();
		EPartyOptionItem eItem = kOption.GetOptionItem();
		EPartyOptionPublicTitle ePublicTitle = kOption.GetOptionPublicTitle();
		int eLevel = kOption.GetOptionLevel();
	#endif
		g_kParty.Option(kOption);
	}

	/*
	void lwSetPartyOption(int const iOptionExp, int const iOptionItem)
	{
		SPartyOption kOption = g_kParty.Option();
		kOption.SetOptionExp((EPartyOptionExp)iOptionExp);
		kOption.SetOptionItem((EPartyOptionItem)iOptionItem);
	#ifndef EXTERNAL_RELEASE
		EPartyOptionExp eExp = kOption.GetOptionExp();
		EPartyOptionItem eItem = kOption.GetOptionItem();
	#endif
		g_kParty.Option(kOption);
	}
	*/


	void RefreshPartyPeopleFindWnd(lwUIWnd lwPartyFrm)
	{//lwPartyFrm == "SFRM_PARTY_PEOPLE_FIND"
		if(!g_pkWorld)
		{
			return;
		}
		XUI::CXUI_Form *pkForm = (XUI::CXUI_Form*)lwPartyFrm();
		if( !pkForm )
		{
			XUI::CXUI_Form *pkTopFrm = (XUI::CXUI_Form*)XUIMgr.Get(std::wstring(_T("SFRM_PARTY_PEOPLE_FIND")));
			if( !pkTopFrm )
			{
				return;
			}
			pkForm = (XUI::CXUI_Form*)pkTopFrm->GetControl(std::wstring(_T("FRM_ITEM")));
			if( !pkForm )
			{
				return;
			}
		}



		XUI::CXUI_List *pkListWnd = (XUI::CXUI_List*)pkForm->GetControl(std::wstring(_T("LST_PARTY_PEOPLE_LIST")));
		if( !pkListWnd )
		{
			return;
		}


		PgWorld::ObjectContainer kContainer;
		g_pkWorld->GetContainer(PgIXmlObject::ID_PC, kContainer);
		if (kContainer.empty())
		{
			return;
		}

		XUI::SListItem* pItem = pkListWnd->FirstItem();
		int iCustomCount = 1;

		PgWorld::ObjectContainer::const_iterator itr = kContainer.begin();
		while (kContainer.end() != itr)
		{
			if( !pItem )
			{
				pItem = pkListWnd->AddItem(_T(""),0);
			}

			PgIWorldObject* pkPlayer = itr->second;

			if( !pkPlayer ) 
			{
				++itr;
				continue;
			}
			
			PgPilot* pkPilot = pkPlayer->GetPilot();

			if( !pkPilot ) 
			{
				++itr;
				continue;
			}

			PgPlayer* pkPC = dynamic_cast<PgPlayer*>(pkPilot->GetUnit());

			if( !pkPC )
			{
				++itr;
				continue;
			}
			
			if( pkPC->HaveParty() )
			{
				++itr;
				continue;
			}

			if( pkPC->GetID() == g_kHelpSystem.HelperGuid() )
			{
				++itr;
				continue;
			}

			if(	pItem )
			{
				XUI::CXUI_Form *pForm = (XUI::CXUI_Form*)pItem->m_pWnd;

				if(pForm)
				{
					BM::GUID const &rkCharGuid = pkPilot->GetGuid();

					if( !rkCharGuid ) continue;

					pForm->OwnerGuid(rkCharGuid);//캐릭GUID
					pForm->SetCustomData(&iCustomCount, sizeof(iCustomCount));

					XUI::CXUI_Form *pkNameForm = (XUI::CXUI_Form*)pForm->GetControl(std::wstring(_T("FRM_NAME")));
					if( pkNameForm )
					{
						pkNameForm->Text(pkPilot->GetName());//사람이름
					}

					XUI::CXUI_Wnd *pImg = pForm->GetControl(std::wstring(_T("IMG_CLASS")));
					lwSetMiniClassIconIndex(pImg, pkPilot->GetAbil(AT_CLASS));
					
					XUI::CXUI_Wnd *pkLevel = pForm->GetControl(std::wstring(_T("FRM_LEVEL")));
					if( pkLevel )
					{
						BM::vstring kLevel(pkPilot->GetAbil(AT_LEVEL));
						pkLevel->Text(kLevel);
					}

					++iCustomCount;

					g_kChatMgrClient.Name2Guid_Add( CT_NORMAL, pkPilot->GetName(), rkCharGuid );
				}
			}		
			++itr;
			pItem = pkListWnd->NextItem(pItem);

			if( iCustomCount >= PV_MAX_LIST_CNT ) break;
		}
		while(pItem)
		{
			pItem = pkListWnd->DeleteItem(pItem);
		}
	}

	int SetPartyOptionArea(int const iAttribute, int const iContinent, bool bAll)
	{
		CONT_ATTRIBUTE_LIST::iterator iter = g_kParty.m_kPartyListAttribute.find(iContinent);
		if( g_kParty.m_kPartyListAttribute.end() != iter )
		{
			CONT_ATTRIBUTE_LIST::mapped_type const kAreaNameNoList = iter->second;
			CONT_ATTRIBUTE_LIST::mapped_type::const_iterator iter_AreaNameNoList = kAreaNameNoList.begin();
			while(kAreaNameNoList.end() != iter_AreaNameNoList)
			{
				bool bRet = false;
				if( true == bAll )
				{
					bRet = (0 == iAttribute);
				}
				if( (iAttribute == iter_AreaNameNoList->iAttribute) || bRet )
				{
					int iValue = iter_AreaNameNoList->iArea_NameNo;

					lua_tinker::call<void, int>("OnSelectParty_Area", iValue);

					return iValue;
				}
				++iter_AreaNameNoList;
			}
		}
		return 0;		
	}

	void RefreshPartyTitleWnd()
	{
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_PARTY_OPTION"));
		if( !pkTopWnd )
		{
			return;
		}

		XUIMgr.ClearEditFocus();

		XUI::CXUI_Edit* pkEditWnd1 = dynamic_cast<XUI::CXUI_Edit*>(pkTopWnd->GetControl(_T("EDT_CHARNAME_2")));
		if( !pkEditWnd1 )
		{
			return;
		}

		pkEditWnd1->SetEditFocus(false);

		if( pkEditWnd1 )
		{
			if( GUID_NULL == g_kParty.PartyGuid() )
			{
				pkEditWnd1->EditText(_T(""));
			}
			else
			{
				pkEditWnd1->EditText(g_kParty.Option().PartySubName());
			}
		}

		XUI::CXUI_Edit* pkEditWnd = dynamic_cast<XUI::CXUI_Edit*>(pkTopWnd->GetControl(_T("EDT_CHARNAME_1")));
		if( !pkEditWnd )
		{
			return;
		}
		pkEditWnd->SetEditFocus(false);

		if( pkEditWnd )
		{
			if( GUID_NULL == g_kParty.PartyGuid() )
			{
				pkEditWnd->EditText(_T(""));
			}
			else
			{
				pkEditWnd->EditText(g_kParty.PartyName());
			}
		}


		XUI::CXUI_Wnd *pkSubWnd = pkTopWnd->GetControl(std::wstring(_T("FRM_ITEM")));

		if( !pkSubWnd ) return;

		std::wstring szWndStr1 = _T("");
		std::wstring szWndStr2 = _T("");
		std::wstring szWndStr3 = _T("");
		std::wstring szWndStr4 = _T("");
		int iLevel = g_kParty.Option().GetOptionLevel();

		if( !IsInParty() )
		{
			szWndStr1 = _T("CHK_OPTION4");
			szWndStr2 = _T("CHK_OPTION5");

			szWndStr3 = _T("CHK_OPTION1");
			szWndStr4 = _T("CHK_OPTION2");
		}
		else
		{
			if( g_kParty.Option().GetOptionPublicTitle() == POT_Public )
			{
				szWndStr1 = _T("CHK_OPTION4");
				szWndStr2 = _T("CHK_OPTION5");
			}
			else
			{
				szWndStr1 = _T("CHK_OPTION5");
				szWndStr2 = _T("CHK_OPTION4");
			}

			if( g_kParty.Option().GetOptionItem() == POI_Order )
			{
				szWndStr3 = _T("CHK_OPTION1");
				szWndStr4 = _T("CHK_OPTION2");
			}
			else
			{
				szWndStr3 = _T("CHK_OPTION2");
				szWndStr4 = _T("CHK_OPTION1");
			}
		}	


		XUI::CXUI_CheckButton* pkCheckWnd1 = dynamic_cast<XUI::CXUI_CheckButton*>(pkSubWnd->GetControl(szWndStr1));
		XUI::CXUI_CheckButton* pkCheckWnd2 = dynamic_cast<XUI::CXUI_CheckButton*>(pkSubWnd->GetControl(szWndStr2));
		XUI::CXUI_CheckButton* pkCheckWnd3 = dynamic_cast<XUI::CXUI_CheckButton*>(pkSubWnd->GetControl(szWndStr3));
		XUI::CXUI_CheckButton* pkCheckWnd4 = dynamic_cast<XUI::CXUI_CheckButton*>(pkSubWnd->GetControl(szWndStr4));

		XUI::CXUI_CheckButton* pkCheckFieldWnd = dynamic_cast<XUI::CXUI_CheckButton*>(pkSubWnd->GetControl(_T("BTN_FIELD")));
		XUI::CXUI_CheckButton* pkCheckMissionWnd = dynamic_cast<XUI::CXUI_CheckButton*>(pkSubWnd->GetControl(_T("BTN_MISSION")));
		XUI::CXUI_CheckButton* pkCheckIndunWnd = dynamic_cast<XUI::CXUI_CheckButton*>(pkSubWnd->GetControl(_T("BTN_INDUN")));
		XUI::CXUI_CheckButton* pkCheckHiddenWnd = dynamic_cast<XUI::CXUI_CheckButton*>(pkSubWnd->GetControl(_T("BTN_HIDDEN")));
		XUI::CXUI_CheckButton* pkCheckChaosWnd = dynamic_cast<XUI::CXUI_CheckButton*>(pkSubWnd->GetControl(_T("BTN_CHAOS")));

		XUI::CXUI_Wnd* pkContinentWnd = pkSubWnd->GetControl(_T("SFRM_BG_INPUT_CONTINENT"));
		XUI::CXUI_Wnd* pkAreaWnd = pkSubWnd->GetControl(_T("SFRM_BG_INPUT_AREA"));
		XUI::CXUI_Wnd* pkLevelWnd = pkSubWnd->GetControl(_T("SFRM_BG_INPUT_LEVEL"));

		if( pkCheckWnd1 )
		{
			pkCheckWnd1->Check(true);
		}
		
		if( pkCheckWnd2 )
		{
			pkCheckWnd2->Check(false);
		}

		if( pkCheckWnd3 )
		{
			pkCheckWnd3->Check(false);
		}

		if( pkCheckWnd4 )
		{
			pkCheckWnd4->Check(true);
		}

		int iAttribute = static_cast<int>(g_kParty.Option().GetOptionAttribute());
		if( !IsInParty() )
		{
			iAttribute = static_cast<int>(POA_Field);
		}		

		if( pkCheckFieldWnd )
		{
			pkCheckFieldWnd->Check(false);
		}
		if( pkCheckMissionWnd )
		{
			pkCheckMissionWnd->Check(false);
		}
		if( pkCheckIndunWnd )
		{
			pkCheckIndunWnd->Check(false);
		}
		if( pkCheckHiddenWnd )
		{
			pkCheckHiddenWnd->Check(false);
		}
		if( pkCheckChaosWnd )
		{
			pkCheckChaosWnd->Check(false);
		}

		int iContinent = static_cast<int>(g_kParty.Option().GetOptionContinent());
		if( !IsInParty() )
		{
			iContinent = static_cast<int>(POC_Area1);
		}
		if( pkContinentWnd )
		{			
			lua_tinker::call<void, int>("OnSelectParty_Continent", iContinent);
		}

		lua_tinker::call<void, int>("SetPartyAttribute", iAttribute);

		switch( iAttribute )
		{
		case POA_All:
			{
			}//break;
		case POA_Field:
			{
				if( pkCheckFieldWnd )
				{
					pkCheckFieldWnd->Check(true);
				}
			}break;
		case POA_Mission:
			{
				if( pkCheckMissionWnd )
				{
					pkCheckMissionWnd->Check(true);
				}
			}break;
		case POA_Indun:
			{
				if( pkCheckIndunWnd )
				{
					pkCheckIndunWnd->Check(true);
				}
			}break;
		case POA_Hidden:
			{
				if( pkCheckHiddenWnd )
				{
					pkCheckHiddenWnd->Check(true);
				}
			}break;
		case POA_Chaos:
			{
				if( pkCheckChaosWnd )
				{
					pkCheckChaosWnd->Check(true);
				}
			}break;
		default:
			{
			}break;
		}

		if( pkAreaWnd )
		{
			if( !IsInParty() )
			{
				SetPartyOptionArea(iAttribute, iContinent);
			}
			else
			{
				int iAreaText = static_cast<int>(g_kParty.Option().GetOptionArea_NameNo());
				if( 0 < iAreaText )
				{
					lua_tinker::call<void, int>("OnSelectParty_Area", iAreaText);
				}
				else
				{
					SetPartyOptionArea(iAttribute, iContinent);
				}
			}
		}		
		
		if( pkLevelWnd )
		{
			lua_tinker::call<void, int>("OnSelectParty_Level", iLevel);

			/*BM::vstring vStr(L"");
			vStr = TTW(401329);
			vStr.Replace(L"#LEVEL#", iLevel);
			pkLevelWnd->Text(vStr);*/
		}
	}

	lwWString GetClassName(lwGUID kGuid)
	{
		BM::GUID const &rkCharGuid = kGuid();

		std::wstring kMessage;
		const PgPilot* pkPilot = g_kPilotMan.FindPilot(rkCharGuid);
		if( pkPilot )
		{
			std::wstring const &rkCharName = TTW(pkPilot->GetAbil(AT_CLASS)+30000);
			kMessage = rkCharName;
		}

		return lwWString(kMessage);
	}

	void SearchPeopleUpdate(CONT_SEARCH_UNIT_INFO& unit_itr)
	{
		XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(_T("SFRM_PROPLE_SEARCH"));
		if( !pkTopWnd )
		{
			return;
		}

		XUI::CXUI_Wnd* pkTopFom = pkTopWnd->GetControl(_T("FRM_ITEM"));
		if( !pkTopFom )
		{
			return;
		}

		XUI::CXUI_List *pkListWnd = (XUI::CXUI_List*)pkTopFom->GetControl(std::wstring(_T("LST_SEARCH_PEOPLE_LIST")));
		if( !pkListWnd )
		{
			return;
		}

		pkListWnd->ClearList();

		int iCustomCount = 0;
		CONT_SEARCH_UNIT_INFO::iterator iter = unit_itr.begin();
		while( iter != unit_itr.end() )
		{		
			XUI::SListItem* pkNewItem = pkListWnd->AddItem(iter->Name);
			if( pkNewItem )
			{
				XUI::CXUI_Wnd* pkNewItemWnd = pkNewItem->m_pWnd;
				if( pkNewItemWnd )
				{
					pkNewItemWnd->OwnerGuid( iter->kCharGuid );

					XUI::CXUI_Wnd* pkLocationWnd = pkNewItemWnd->GetControl(_T("FRM_LOCATION"));
					XUI::CXUI_Wnd* pkCharNameWnd = pkNewItemWnd->GetControl(_T("FRM_NAME"));				
					XUI::CXUI_Wnd* pkLevelWnd = pkNewItemWnd->GetControl(_T("FRM_LEVEL"));

					if( pkLocationWnd )
					{
						std::wstring const &rkMapName = (iter->iGndNo) ? GetMapName(iter->iGndNo) : std::wstring();
						pkLocationWnd->Text( rkMapName );
					}
					if( pkCharNameWnd )
					{
						pkCharNameWnd->Text(iter->Name);
					}
					if( pkLevelWnd )
					{
						BM::vstring kLevel(iter->iLevel);
						pkLevelWnd->Text(kLevel);
					}
				}
			}

			if( ++iCustomCount >= PV_MAX_LIST_CNT )
			{
				break;
			}
			++iter;
		}
	}
	bool lwHaveIParty()
	{// 플레이어가 파티를 가지고 있는가? 
		PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
		if( !pkPC )
		{// 플레이어를 찾을수 없다면
			_PgMessageBox("PgClientPartyUtil::HaveIParty()", "Can\'t find PlayerUnit");
			return false;
		}
		if(pkPC->HaveParty())
		{// 파티가 있다면
			return true;
		}
		else
		{// 파티가 없다면
			return false;
		}
	}
	
	bool lwEnablePartyBreakIn()
	{
		int MapNo = 0;
		if( NULL == g_pkWorld )
		{
			return false;
		}

		MapNo = g_pkWorld->MapNo();

		if( PV_MAX_MEMBER_CNT <= g_kParty.MemberCount() )
		{
			return false;
		}
		
		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( pPlayer )
		{
			if( false == PgClientPartyUtil::IsPartyMaster(pPlayer->GetID()) )
			{// 파티장이 아니면 상관 없음.
				return false;
			}
		}

		if( PgClientExpeditionUtil::IsInExpedition() )
		{// 원정대일 경우에도 패스
			return false;
		}

		CONT_DEFMAP const * pContDefMap = NULL;
		g_kTblDataMgr.GetContDef(pContDefMap);	//맵 데프
		if( NULL == pContDefMap )
		{
			return false;
		}

		CONT_DEFMAP::const_iterator map_iter = pContDefMap->find(MapNo);	//맵번호로 맵 데프에서 찾아서
		if( pContDefMap->end() == map_iter )
		{
			return false;
		}

		if( 0 == map_iter->second.byPartyBreakIn )
		{
			return false;
		}

		return true;
	}
	
	void lwSend_PT_C_M_REQ_JOIN_PARTY_REFUSE(bool const Refuse)
	{
		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( false == PgClientPartyUtil::IsPartyMaster(pPlayer->GetID()) )
		{
			return;
		}

		BM::Stream Packet(PT_C_M_REQ_JOIN_PARTY_REFUSE);
		Packet.Push(Refuse);
		NETWORK_SEND(Packet);
	}
}

void PgClientPartyUtil::lwCheckCanKick(lwGUID lwCharGuid, char const* ReqType)
{
	BM::GUID const & CharGuid = lwCharGuid();
	if( BM::GUID::NullData() == CharGuid )
	{
		return;
	}

	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return;
	}

	if( NULL == ReqType )
	{
		return;
	}

	BM::vstring const Type(ReqType);

	if( ReqType == "KICKOUT" )
	{// 추방일 경우에는 자신이 파티장인지 검사한다.
		if( false == PgClientPartyUtil::IsPartyMaster( pPlayer->GetID() ) )
		{
			std::wstring Temp = TTW(700064);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Temp, true);
			return;
		}
	}

	BM::Stream Packet(PT_C_M_REQ_CHECK_CAN_KICK);
	Packet.Push(Type);
	Packet.Push(CharGuid);
	NETWORK_SEND(Packet);
}

bool PgClientPartyUtil::lwCheckCanKick_Leave()
{
	if( NULL == g_pkWorld )
	{
		return false;
	}

	if( true == g_pkWorld->IsHaveAttr(GATTR_FLAG_ENABLE_KICK) )
	{
		return false;
	}
	return true;
}

void lwPartyStateCloseBtnDown(lwUIWnd kWnd)
{
	if( kWnd.IsNil() ){ return; }
	if( !PgClientPartyUtil::IsInParty() ){ return; }
	
	BM::GUID const & kOwner = kWnd.GetOwnerGuid()();
	if( kOwner.IsNull() ){ return; }

	if( !PgClientPartyUtil::IsInPartyMemberGuid(kOwner) ){ return; }

	SPartyMember const* kPartyName = NULL;
	if( !g_kParty.GetMember(kOwner, kPartyName) ){ return; }
	
	BM::GUID kMyGuid;
	if( !g_kPilotMan.GetPlayerPilotGuid(kMyGuid) ){ return; }

	bool const bMine = (kMyGuid == kOwner);
	bool const bIamMaster = PgClientPartyUtil::IsPartyMaster(kMyGuid);
	
	if( bMine )
	{
		CallYesNoMsgBox(TTW(22004), kOwner, MBT_PARTY_LEAVE);
		return;
	}
	
	if( bIamMaster )
	{
		BM::vstring vStr( TTW(22005) );
		vStr.Replace(L"#PLAYER#", kPartyName->kName);

		CallYesNoMsgBox(vStr, kOwner, MBT_PARTY_KICKOUT);
		return;
	}
}