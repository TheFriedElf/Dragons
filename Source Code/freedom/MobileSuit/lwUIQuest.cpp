#include "StdAfx.h"
#include "Variant/PgPlayer.h"
#include "Variant/constant.h"
#include "Variant/PgQuestInfo.h"
#include "lwUI.h"
#include "lwUIQuest.h"
#include "PgNetwork.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgActor.h"
#include "PgPilot.h"
#include "PgWorld.h"
#include "ServerLib.h"
#include "PgMobileSuit.h"
#include "PgXmlLoader.h"
#include "PgAction.h"
#include "PgQuest.h"
#include "PgQuestUI.h"
#include "PgQuestIngList.h"
#include "PgQuestSelectiveList.h"
#include "PgQuestDialog.h"
#include "PgQuestIngList.h"
#include "PgQuestMan.h"
#include "lwGuid.h"
#include "PgRenderMan.h"
#include "PgGuild.h"
#include "PgSoundMan.h"
#include "PgChatMgrClient.h"
#include "PgClientParty.h"
#include "lwPartyUI.h"
#include "PgDailyQuestUI.h"

namespace EventTaiwanEffectQuest
{
	std::wstring const kEventUIName(L"SFRM_EVENT_TW_QUESTEFFECT");
	void CallUI(lwGUID kNpcGuid)
	{
		if( BM::GUID::IsNull(kNpcGuid()) )
		{
			return;
		}

		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgActor* pkActor = g_kPilotMan.GetPlayerActor();
		if( !pkActor )
		{
			return;
		}

		std::wstring const kCountUIName(L"FRM_COUNT");
		CXUI_Wnd* pkTopWnd = XUIMgr.Get(kEventUIName);
		if( !pkTopWnd )
		{
			pkTopWnd = XUIMgr.Call(kEventUIName);
			if( !pkTopWnd )
			{
				return;
			}
			g_kUIScene.RegistUIAction(kEventUIName, WSTR_CLOSEUI, pkActor->GetTranslate());
		}

		CXUI_Wnd* pkCountUI = pkTopWnd->GetControl(kCountUIName);
		if( pkCountUI )
		{
			int const iFormatTTW = 700506;
			std::wstring kTempStr;
			FormatTTW(kTempStr, iFormatTTW, GetEnableCount(pkPlayer->GetMyQuest()));
			pkCountUI->Text( kTempStr );
		}

		CXUI_Wnd* pkBuffImg = pkTopWnd->GetControl( std::wstring(L"ICN_BUFF") );
		if( pkBuffImg )
		{
			pkBuffImg->GrayScale( 0 == GetEnableCount(pkPlayer->GetMyQuest()) );
		}

		BM::GUID kTempGuid(kNpcGuid());
		pkTopWnd->SetCustomData( &kTempGuid, sizeof(kTempGuid) );
	}

	void CallEffectToolTip(lwUIWnd kSelf)
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iEventEffectNo);
		if(pkSkillDef)
		{
			PgSkillTree::stTreeNode kTempNode(iEventEffectNo, iEventEffectNo, pkSkillDef, true);
			CallQuickSkillToolTip(&kTempNode, kSelf.GetLocation(), iEventEffectNo, true);
		}
	}

	void UpdateUI()
	{
		CXUI_Wnd* pkTopWnd = XUIMgr.Get(kEventUIName);
		if( !pkTopWnd )
		{
			return;
		}

		BM::GUID kNpcGuid;
		pkTopWnd->GetCustomData( &kNpcGuid, sizeof(kNpcGuid) );
		CallUI(kNpcGuid);
	}

	void ReqSend(lwUIWnd kTopWnd)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		if( !kTopWnd() )
		{
			return;
		}

		BM::GUID kNpcGuid;
		kTopWnd()->GetCustomData( &kNpcGuid, sizeof(kNpcGuid) );

		if( BM::GUID::IsNull(kNpcGuid)
		||	0 == GetEnableCount(pkPlayer->GetMyQuest()) )
		{
			g_kChatMgrClient.AddMessage(iNotHaveQuestMsg, SChatLog(CT_EVENT), true);
			return;
		}

		if( 0 != pkPlayer->GetAbil(AT_ADD_EXP_PER) )
		{
			g_kChatMgrClient.AddMessage(iCantDupExpPotionMsg, SChatLog(CT_EVENT), true);
			return;
		}

		if( NULL != pkPlayer->GetEffect(iEventEffectNo) )
		{
			g_kChatMgrClient.AddMessage(iCantDupRewardEffectMsg, SChatLog(CT_EVENT), true);
			return;
		}

		BM::Stream kPacket(PT_C_M_REQ_EVENT_TW_USE_EFFECTQUEST);
		kPacket.Push( kNpcGuid );
		NETWORK_SEND(kPacket);
	}

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "Call_EventTaiwanQuestEffect", CallUI);
		def(pkState, "Send_EventTaiwanQuestEffect", ReqSend);
		def(pkState, "ToolTip_EventTaiwanQuestEffect", CallEffectToolTip);
	}
};

namespace RealmQuest
{
	void SendReqRealmQuestInfo(char const* szRealmQuestID)
	{
		if( !szRealmQuestID )
		{
			return;
		}

		BM::Stream kPacket(PT_C_N_REQ_REALM_QUEST_INFO);
		kPacket.Push( std::wstring(UNI(szRealmQuestID)) );
		NETWORK_SEND(kPacket);
	}

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "SendReqRealmQuestInfo", SendReqRealmQuestInfo);
	}
};

namespace Quest
{
	int const iQuestMiniListSnapGroupID = 1;

	bool RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		//Quest
		def(pkState, "OnCallQuestRewardItemImage", lwOnCallQuestRewardItemImage);
		def(pkState, "OnCallQuestRewardItemToolTip", lwOnCallQuestRewardItemToolTip);
		def(pkState, "OnCallQuestRewardLevelToolTip", lwOnCallQuestRewardLevelToolTip);
		def(pkState, "OnQuestItemSelected_From_MyQuestList", lwOnQuestItemSelected_From_MyQuestList);
		def(pkState, "OnQuestItemSelected_From_SelectiveQuestList", lwOnQuestItemSelected_From_SelectiveQuestList);
		def(pkState, "OnItemSelected_From_QuestInfo", lwOnItemSelected_From_QuestInfo);
		def(pkState, "OnGiveUpQuestConfirmed", lwOnGiveUpQuestConfirmed);
		def(pkState, "OnCheckBoxClick_From_MyQuestList", lwOnCheckBoxClick_From_MyQuestList);
		def(pkState, "OnShowMiniQuestList", lwOnShowMiniQuestList);
		def(pkState, "Get_MiniQuestList_ShowCount", lwGet_MiniQuestList_ShowCount);
		def(pkState, "OnRewardItemSelected_From_QuestInfo", lwOnRewardItemSelected_From_QuestInfo);
		def(pkState, "Show_IngQuestInterface", lwShow_IngQuestInterface);
		def(pkState, "IsCanShowIngQuestList", lwIsCanShowIngQuestList);
		def(pkState, "SaveMiniIngQuestSnapInfo", lwSaveMiniIngQuestSnapInfo);
		def(pkState, "LoadMiniIngQuestSnapInfo", lwLoadMiniIngQuestSnapInfo);
		def(pkState, "SetCutedTextLimitLength", lwSetCutedTextLimitLength);
		def(pkState, "ReqQuestShareToParty", ReqShareQuest);
		def(pkState, "ReqQuestRemoteComplete", SendReqRemoteCompleteQuest);
		return true;
	}



	void lwQuestTextParser(std::wstring &szText, const PgQuestInfo* pkQuestInfo)
	{
		if( szText.empty() )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer){return;}

		typedef std::list<std::wstring> RegTextList;//예약어 목록

		RegTextList kRegTextList;
		kRegTextList.push_back( _T("%class%") );			//클래스
		kRegTextList.push_back( _T("%name%") );				//캐릭터 이름
		kRegTextList.push_back( _T("%level%") );			//캐릭터 레벨
		kRegTextList.push_back( _T("%tempguildname%") );	//임시 길드 이름
		kRegTextList.push_back( _T("%guildname%") );
		if( pkQuestInfo )
		{
			kRegTextList.push_back( _T("%param1%"));
			kRegTextList.push_back( _T("%param2%"));
			kRegTextList.push_back( _T("%param3%"));
			kRegTextList.push_back( _T("%param4%"));
			kRegTextList.push_back( _T("%param5%"));
		}

		//
		std::wstring szTemp;
		int const iClass = pkPlayer->GetAbil(AT_CLASS);
		int const iMinClass = 1;
		int const iMaxClass = 58;
		if( iMinClass <= iClass
		&&	iMaxClass >= iClass)
		{
			szTemp = TTW(30000 + iClass);
		}
		else
		{
			szTemp = _T("NONE Class");
		}

		char szLevel[10] = {0, };
		sprintf_s(szLevel, 10, "%d", pkPlayer->GetAbil(AT_LEVEL) );

		RegTextList kRealTextList;//
		kRealTextList.push_back( szTemp );				//클래스
		kRealTextList.push_back( pkPlayer->Name() );	//이름
		kRealTextList.push_back( UNI(szLevel) );		//레벨
		kRealTextList.push_back( g_kGuildMgr.TempGuildName() );
		kRealTextList.push_back( g_kGuildMgr.GuildName() );
		if( pkQuestInfo )
		{
			BM::vstring kTempStr;
			kTempStr = pkQuestInfo->GetParamEndCondition(0);	kRealTextList.push_back( (std::wstring)kTempStr );
			kTempStr = pkQuestInfo->GetParamEndCondition(1);	kRealTextList.push_back( (std::wstring)kTempStr );
			kTempStr = pkQuestInfo->GetParamEndCondition(2);	kRealTextList.push_back( (std::wstring)kTempStr );
			kTempStr = pkQuestInfo->GetParamEndCondition(3);	kRealTextList.push_back( (std::wstring)kTempStr );
			kTempStr = pkQuestInfo->GetParamEndCondition(4);	kRealTextList.push_back( (std::wstring)kTempStr );
		}

		if( kRegTextList.size() != kRealTextList.size() )
		{
			PG_ASSERT_LOG(0 && "Invalid count of reserved Quest Text Word");
			return ;
		}

		RegTextList::iterator iter = kRegTextList.begin();
		RegTextList::iterator real_iter = kRealTextList.begin();
		for( ; iter != kRegTextList.end(); ++iter, ++real_iter )
		{
			std::wstring::size_type pos = szText.find((*iter), 0);	//0번째부터 예약어 검사

			while( pos != std::wstring::npos )	//루프를 돌아야 전부 바뀜
			{
				szText.replace((int)pos, (*iter).size(), (*real_iter) );
				pos = szText.find((*iter), 0);	//0번째부터 예약어 검사
			}
		}
	}

	void lwOnQuestItemSelected_From_MyQuestList(lwUIWnd UIWnd)
	{
		PgQUI_IngQuestList *pIngQuestListForm = (PgQUI_IngQuestList*)XUIMgr.Get(WSTR_FORM_MY_QUEST_LIST);
		if( !pIngQuestListForm )
		{return;}

		pIngQuestListForm->OnQuestItemSelected_From_MyQuestList(UIWnd.GetCustomData<int>());
	}

	void lwOnQuestItemSelected_From_SelectiveQuestList(int const iQuestID, lwGUID kGuid, bool const bRemoteAccept)
	{
		BM::Stream kPacket( PT_C_M_REQ_INTROQUEST );
		kPacket.Push( iQuestID );
		kPacket.Push( kGuid() );
		kPacket.Push( bRemoteAccept );
		NETWORK_SEND(kPacket)
	}

	void lwOnItemSelected_From_QuestInfo(lwUIWnd kQuestInfoWnd, int const iSelectionID)
	{
		if( !kQuestInfoWnd() )
		{
			return;
		}

		PgQUI_QuestDialog* pQuestInfoDialog = dynamic_cast< PgQUI_QuestDialog* >(kQuestInfoWnd());

		int const iQuestID = g_kQuestMan.NowQuestID();
		int const iDialogID = g_kQuestMan.NowDialogID();

		// 퀘스트 정보를 가져와서, 선택 아이템이 있는데 선택을 안했으면
		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
		if( !pkQuestInfo )
		{
			return;
		}

		const SQuestDialog* pkQuestDialog = NULL;
		bool const bFindDialog = pkQuestInfo->GetDialog(iDialogID, pkQuestDialog);
		if( !bFindDialog )
		{
			return;
		}

		bool const bCanNext = g_kQuestMan.CanNextDialog(pkQuestInfo, pkQuestDialog, iSelectionID);
		if( !bCanNext )
		{
			return; // 넘어가면 안됨
		}

		// 무조건 보낸다
		g_kQuestMan.Send_Dialog_Select(iQuestID, iDialogID, iSelectionID, g_kQuestMan.SelectItem1(), g_kQuestMan.SelectItem2());

		if( QRDID_REJECT_Start <= iSelectionID
		&&  QRDID_REJECT_End >= iSelectionID )//거부
		{
			PgActor* pkActor = g_kPilotMan.GetPlayerActor();
			if( pkActor )
			{
				pkActor->PlayNewSound(NiAudioSource::TYPE_3D, "QUEST_Deny", 0.0f);
			}
		}
	}

	void lwOnGiveUpQuestConfirmed()
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("FORM_MY_QUEST_LIST"));
		if( !pkWnd )
		{
			return;
		}

		PgQUI_QuestDialog* pDlg = dynamic_cast<PgQUI_QuestDialog*>(pkWnd->GetControl(WSTR_MY_QUEST_INFO));
		if( !pDlg )
		{return;}

		pDlg->OnGiveUpConfirmed();
		g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_AMBIENT, "QUEST_GiveUp", 0.0f);

		XUIMgr.Close(_T("SFRM_NEW_QUEST"));//퀘스트 알림목록 창 닫기
	
	}

	void lwOnCheckBoxClick_From_MyQuestList(lwUIWnd UIWnd)
	{
		PgQUI_IngQuestList *pIngQuestListForm= (PgQUI_IngQuestList*)XUIMgr.Get(WSTR_FORM_MY_QUEST_LIST);
		if( !pIngQuestListForm )
		{return;}

		pIngQuestListForm->OnCheckBoxClick_From_MyQuestList(UIWnd());
	}

	void lwOnShowMiniQuestList()
	{
		bool const bVisible = g_kQuestMan.MiniQuestStatus();
		g_kQuestMan.ShowMiniQuest(bVisible);
	}

	unsigned int lwGet_MiniQuestList_ShowCount()
	{
		return (int)g_kQuestMan.GetMiniQuestCount();;
	}

	void lwOnRewardItemSelected_From_QuestInfo(lwUIWnd UIWnd, int iRewardType, int iSelectNum)
	{
		XUI::CXUI_Wnd* pkFullQuestWnd = XUIMgr.Get( PgQuestManUtil::kFullQuestWndName );
		if( !pkFullQuestWnd )
		{return;}

		PgQUI_QuestDialog* pDlg = dynamic_cast<PgQUI_QuestDialog*>(pkFullQuestWnd->GetControl(_T("FRM_QUEST_REQ")));
		if(!pDlg) return;

		bool bRet = pDlg->OnRewardItemSelected_From_QuestInfo(iRewardType, iSelectNum);
		if(bRet)//무엇이 선택되었는지 표시한다.
		{
			wchar_t szTemp[255] = {0, };
			CXUI_Wnd *pkWnd = UIWnd();
			if( !pkWnd )
			{return;}

			CXUI_Wnd* pkParent = pkWnd->Parent();
			if( !pkParent )
			{return;}

			_stprintf_s(szTemp, WSTR_IMG_OVER_FORMAT, iRewardType);
			CXUI_Wnd *pkOver = pkParent->GetControl(szTemp);
			if( !pkOver )
			{return;}

			_stprintf_s(szTemp, WSTR_REWARD_SFRM_FORMAT, iRewardType, iSelectNum);
			CXUI_Wnd* pkSelectedWnd = pkParent->GetControl(szTemp);
			if( !pkSelectedWnd )
			{return;}

			const POINT3I& kPos = pkSelectedWnd->Location();
			pkOver->Location(kPos.x, kPos.y);
			pkOver->UVUpdate(3);//Selected
			pkOver->Visible(true);
			pkOver->SetInvalidate(true);
		}
	}

	//	진행중인 퀘스트 목록 인터페이스를 보여주도록 설정한다.
	void lwShow_IngQuestInterface(int const iSelectedQuestID)
	{
		PgQUI_IngQuestList *pIngQuestListForm = dynamic_cast<PgQUI_IngQuestList *>( XUIMgr.Get(WSTR_FORM_MY_QUEST_LIST) );
		if( !pIngQuestListForm || !pIngQuestListForm->Visible() || iSelectedQuestID )
		{
			if( !pIngQuestListForm )
			{
				pIngQuestListForm = dynamic_cast<PgQUI_IngQuestList *>( XUIMgr.Call(WSTR_FORM_MY_QUEST_LIST) );
			}

			if( pIngQuestListForm )
			{
				pIngQuestListForm->Show(iSelectedQuestID);
			}
		}
		else
		{
			XUIMgr.Close( WSTR_FORM_MY_QUEST_LIST );
		}
	}

	bool lwIsCanShowIngQuestList(int const iQuestID)
	{
		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
		if( pkQuestInfo )
		{
			return QT_Soul != pkQuestInfo->Type();
		}
		return false;
	}

	void lwOnCallQuestRewardItemImage(lwUIWnd IconControl)
	{
		XUI::CXUI_Wnd *pkImageControl = IconControl();
		if(NULL == pkImageControl)
		{
			NILOG(PGLOG_ERROR, "RewardItemIcon is NULL"); 
			assert(0); 
			return;
		}

		SQuestItem kQuestRewardItem;
		pkImageControl->GetCustomData(&kQuestRewardItem, sizeof(kQuestRewardItem));

		if( kQuestRewardItem.iItemNo )
		{
			PgUISpriteObject *pkSprite = g_kUIScene.GetIconTexture(kQuestRewardItem.iItemNo);
			PgUIUVSpriteObject *pkUVSprite = (PgUIUVSpriteObject*)pkSprite;
			if(!pkUVSprite)
			{
				NILOG(PGLOG_ERROR, "Is Texture is NULL"); 
				assert(0); 
				return;
			}
			/*XUI::CXUI_Image *pkImageWnd = dynamic_cast<XUI::CXUI_Image*>(pkImageControl);*/
			pkImageControl->DefaultImgTexture(pkUVSprite);
			const SUVInfo &rkUV = pkUVSprite->GetUVInfo();
			pkImageControl->UVInfo(rkUV);
			POINT2 kPoint(iIconWidth*rkUV.U, iIconHeight*rkUV.V);//이미지 원본 사이즈 복구
			pkImageControl->ImgSize(kPoint);
		}
	}

	void lwOnCallQuestRewardItemToolTip(lwUIWnd kControl)
	{
		XUI::CXUI_Wnd *pkWnd = kControl();
		if(pkWnd)
		{
			SQuestItem kQuestRewardItem;
			pkWnd->GetCustomData(&kQuestRewardItem, sizeof(kQuestRewardItem));
			CallToolTip_ItemNo(kQuestRewardItem.iItemNo, kControl.GetLocation(), kQuestRewardItem.bSeal, true, false, kQuestRewardItem.iTimeType, kQuestRewardItem.iTimeValue);
		}
		else
		{
			NILOG(PGLOG_ERROR, "kControl is NULL"); 
			assert(0);
		}
	}

	void lwOnCallQuestRewardLevelToolTip(lwUIWnd kControl)
	{
		XUI::CXUI_Wnd *pkWnd = kControl();
		if(pkWnd)
		{
			int iRewardLevel = 0;
			pkWnd->GetCustomData(&iRewardLevel, sizeof(iRewardLevel));
			if( 0 != iRewardLevel )
			{
				lwPoint2 pt( pkWnd->Location().x, pkWnd->Location().y );
				BM::vstring kText(TTW(460098));
				kText.Replace( L"#LEVEL#", iRewardLevel);
				lwCallToolTipByText(0, static_cast<std::wstring>(kText), pt, "ToolTip2" );
			}
		}
	}


	void lwSaveMiniIngQuestSnapInfo()
	{
		PgFormSnapedWnd::SaveSnapInfo(iQuestMiniListSnapGroupID);
	}

	void lwLoadMiniIngQuestSnapInfo()
	{
		PgFormSnapedWnd::LoadSnapInfo(iQuestMiniListSnapGroupID);
	}

	void lwSetCutedTextLimitLength(lwUIWnd UIWnd, lwWString kText, lwWString kTail, long const iCustomTargetWidth )
	{
		CXUI_Wnd *pkWnd = UIWnd();
		if( NULL == pkWnd )
		{
			return;
		}
		std::wstring kOriginText = kText();
		std::wstring kCutText= kTail();

		SetCutedTextLimitLength(pkWnd, kOriginText, kCutText, iCustomTargetWidth);
	}

	void SetCutedTextLimitLength(XUI::CXUI_Wnd *pkWnd, std::wstring const &rkText, std::wstring kTail, long const iCustomTargetWidth)
	{
		if( !pkWnd )
		{
			return;
		}
		long const iLimitWidth = (iCustomTargetWidth)? iCustomTargetWidth: pkWnd->Size().x;

		XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(pkWnd->Font());
		if( !pFont )
		{
			return;
		}

		XUI::CXUI_Style_String kTempStyle( PgFontDef(pFont, pkWnd->FontColor(), pkWnd->FontFlag()), rkText );
		POINT kSize = Pg2DString::CalculateOnlySize(kTempStyle);
		if( kSize.x < iLimitWidth )
		{
			pkWnd->Text(rkText);
			return;
		}

		int iTextLength = rkText.size() - 1;
		std::wstring kTextStr;
		while(iTextLength >= 0)
		{
			kTextStr = rkText.substr(0, iTextLength) + kTail; //끝 부터 자른다.
			kTempStyle = XUI::CXUI_Style_String( PgFontDef(pFont, pkWnd->FontColor(), pkWnd->FontFlag()), kTextStr );
			kSize = Pg2DString::CalculateOnlySize(kTempStyle);
			if( kSize.x < iLimitWidth )
			{
				pkWnd->Text(kTextStr);
				return;
			}
			--iTextLength;
		}
		pkWnd->Text(L"");
	}

	//
	void ReqShareQuest(lwUIWnd kBtn)
	{
		XUI::CXUI_Wnd* pkButton = kBtn();
		if( !pkButton )
		{
			return;
		}

		int iQuestID = 0;
		if( !pkButton->GetCustomData(&iQuestID, sizeof(iQuestID)) )
		{
			return;
		}

		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
		if( !pkQuestInfo )
		{
			return;
		}

		if( !pkQuestInfo->IsCanShare() )
		{
			g_kChatMgrClient.LogMsgBox(401404); // 공유 할 수 없는 퀘스트
			return;
		}

		if( !PgClientPartyUtil::lwHaveIParty() )
		{
			g_kChatMgrClient.LogMsgBox(401407); // 파티 상태에서만 공유 가능
			return;
		}

		if( 1 >= g_kParty.MemberCount() )
		{
			g_kChatMgrClient.LogMsgBox(401422); // 공유할 파티원이 없습니다.
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
		if( !pkMyQuest )
		{
			return;
		}

		if( !pkMyQuest->IsIngQuest(iQuestID) )
		{
			g_kChatMgrClient.LogMsgBox(401406); // 진행 중인 퀘스트만 공유 가능
			return;
		}

		SUserQuestState const* pkState = pkMyQuest->Get(iQuestID);
		if( QS_Failed == pkState->byQuestState )
		{
			g_kChatMgrClient.LogMsgBox(401404); // 공유 할 수 없는 퀘스트
		}

		BM::Stream kPacket(PT_C_M_REQ_SHAREQUEST);
		kPacket.Push( iQuestID );
		NETWORK_SEND( kPacket );
	}
	typedef std::set< int > ContShareQuestID;
	ContShareQuestID kRecvShareQuestID;
	void ClearRecvShareQuest()
	{
		kRecvShareQuestID.clear();
	}
	void RecvShareQuest(BM::GUID const& rkOrgGuid, int const iShareQuestID)
	{
		if( kRecvShareQuestID.end() != kRecvShareQuestID.find(iShareQuestID) )
		{
			return;
		}

		kRecvShareQuestID.insert( iShareQuestID );

		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iShareQuestID);
		if( !pkQuestInfo )
		{
			return;
		}

		SPartyMember const* pkMember = NULL;
		if( !g_kParty.GetMember(rkOrgGuid, pkMember) )
		{
			g_kChatMgrClient.LogMsgBox(401405); // 퀘스트 공유자가 사라져 공유 받을 수 없어
			return;
		}

		std::wstring kTemp;
		if( FormatTTW(kTemp, 401430, TTW(pkQuestInfo->m_iTitleTextNo).c_str(), pkMember->kName.c_str()) )
		{
			CallYesNoMsgBox(kTemp, rkOrgGuid, MBT_CONFIRM_ACCEPT_SHAREQUEST, iShareQuestID);
		}
	}
	void SendAnsShareQuest(BM::GUID const& rkOrgGuid, int const iShareQuestID, bool const bSayYes)
	{
		if( kRecvShareQuestID.end() == kRecvShareQuestID.find(iShareQuestID) )
		{
			return;
		}
		kRecvShareQuestID.erase(iShareQuestID);

		BM::Stream kPacket(PT_C_M_REQ_ACCEPT_SHAREQUEST);
		kPacket.Push( bSayYes );
		kPacket.Push( iShareQuestID );
		kPacket.Push( rkOrgGuid );
		NETWORK_SEND( kPacket );
	}

	//
	void SendReqRemoteCompleteQuest(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd *pkWnd = kWnd();
		if( !pkWnd )
		{
			return;
		}

		int iQuestID = 0;
		if( !pkWnd->GetCustomData(&iQuestID, sizeof(iQuestID)) )
		{
			return;
		}

		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
		if( !pkQuestInfo )
		{
			return;
		}

		if( !pkQuestInfo->IsCanRemoteComplete() )
		{
			return;
		}

		switch( pkQuestInfo->Type() )
		{
		case QT_Couple:
		case QT_SweetHeart:
		case QT_Random:
		case QT_RandomTactics:
		case QT_Wanted:
			{
				if( NULL != g_pkWorld
					&& 0 != (g_pkWorld->GetAttr() & GATTR_FLAG_MISSION) )
				{
					g_kChatMgrClient.AddMessage(RandomQuestUI::iCantCompleteInMission, SChatLog(CT_EVENT_SYSTEM), true);
					return;
				}
			}break;
		default:
			{
			}break;
		}

		BM::Stream kPacket(PT_C_M_REQ_COMPLETE_QUEST);
		kPacket.Push( iQuestID );
		NETWORK_SEND(kPacket);
	}
};