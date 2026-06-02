#include "StdAfx.h"
#include "Variant/constant.h"
#include "Variant/PgPlayer.h"
#include "Variant/PgQuickInventory.h"

#include "lwSkillSet.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgMobileSuit.h"
#include "PgChatMgrClient.h"

namespace lwSkillSet
{
	//함수정의
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "SkillSetInit", lwSkillSetInit);
		def(pkState, "SkillSetClose", lwSkillSetClose);
		def(pkState, "SkillSetSave", lwSkillSetSave);
		def(pkState, "GetClassLevel", lwGetClassLevel);
		def(pkState, "ChangeSaveState", lwChangeSaveState);
		def(pkState, "ClearBasicSkillSet", lwClearBasicSkillSet);
		def(pkState, "SetBasicSkillSet", lwSetBasicSkillSet);
		def(pkState, "ApplyBasicSkillSetToSkillSet", lwApplyBasicSkillSetToSkillSet);
		def(pkState, "CheckUseSkillSetWnd", lwCheckUseSkillSetWnd);
		def(pkState, "CheckUseSkillSetNo", lwCheckUseSkillSetNo);
		def(pkState, "CallHelpBasicSkillSet", lwCallHelpBasicSkillSet);
		def(pkState, "SetQuickInvBasicSkillSet", lwSetQuickInvBasicSkillSet);
		def(pkState, "GetHelpSkillSetNo", lwGetHelpSkillSetNo);
		def(pkState, "CallHelpSkillSetToolTip", lwCallHelpSkillSetToolTip);
	}

	inline void SetSkillUI(CONT_USER_SKILLSET::mapped_type const * pkSkillSet, XUI::CXUI_Wnd * pkSkillFrm, BYTE const bySetNo, bool const bCloseSet, BYTE const byLimit)
	{
		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkSkillFrm->GetControl(L"BLD_SKILLICON"));
		if( !pkBuild ){ return; }
		int const iMaxSlot = pkBuild->CountX();

		wchar_t wszTemp[MAX_PATH] = {0,};
		for(int i=0; i<iMaxSlot; ++i)
		{
			BM::vstring vStr(L"FRM_SKILLSET_SKILLICON");
			vStr += i;
			XUI::CXUI_Wnd* pkMainFrm = pkSkillFrm->GetControl(vStr);
			if( !pkMainFrm ){ continue; }

			pkMainFrm->Visible( i<byLimit );

			XUI::CXUI_Wnd* pkDelay = pkMainFrm->GetControl(L"FRM_DELAY");
			XUI::CXUI_Wnd* pkIconWnd = pkMainFrm->GetControl(L"SFRM_SETICON_BG");
			if( !pkDelay || !pkIconWnd ){ continue; }

			pkDelay = pkDelay->GetControl(L"IMG_TIME");
			XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pkMainFrm->GetControl(L"ICN_SKILL"));
			if( !pkDelay || !pkIcon ){ continue; }

			if( !bCloseSet )
			{
				int iDelay = 0;
				SIconInfo kSkillData;
				if( pkSkillSet )
				{ 
					iDelay = pkSkillSet->byDelay[i];
					kSkillData.iIconGroup = pkSkillSet->byType[i];
					kSkillData.iIconKey = pkSkillSet->iSkillNo[i];
				}

				pkDelay->SetCustomData(&iDelay, sizeof(iDelay));
				pkIcon->SetCustomData(&kSkillData, sizeof(kSkillData));

				if(iDelay > 0)
				{
					::swprintf_s( wszTemp, MAX_PATH, L"%0.1f", static_cast<float>(iDelay)/10 );//0.0~1.0초까지(1~10)
				}
				else
				{
					::swprintf_s( wszTemp, MAX_PATH, L"0");
				}
				pkDelay->Text(wszTemp);
				pkIcon->Visible( true );
			}
			else
			{
				pkDelay->ClearCustomData();
				pkIcon->ClearCustomData();
				pkDelay->Text(L"");
				pkIcon->Visible( false );
			}
		}
	}

	void GetSkillSetOpenList(PgPlayer const * pkPlayer, VEC_INT & kContOpen)
	{
		if( !pkPlayer ){ return; }

		const CONT_DEFSKILLSET *pkContDefMap = NULL;
		g_kTblDataMgr.GetContDef(pkContDefMap);
		if( !pkContDefMap ){ return; }

		int iSkillSetUseCnt = pkPlayer->GetAbil(AT_SKILLSET_USE_COUNT);
		int const iMyClass = lwGetClassLevel(pkPlayer->GetAbil(AT_CLASS));
		int const iMyLevel = pkPlayer->GetAbil(AT_LEVEL);

		VEC_INT kContUseTmp;
		for(CONT_DEFSKILLSET::const_iterator c_iter=pkContDefMap->begin();c_iter!=pkContDefMap->end();++c_iter)
		{
			BYTE const bySetNo = c_iter->second.bySetNo;
			BYTE const byCondType = c_iter->second.byConditionType;
			BYTE const byCondValue = c_iter->second.byConditionValue;

			switch(byCondType)
			{
			case SSCT_CLASS:
				{
					if(iMyClass >= byCondValue)
					{
						kContOpen.push_back(bySetNo);
					}
				}break;
			case SSCT_ITEM:
				{
					if(0 < iSkillSetUseCnt)
					{
						kContOpen.push_back(bySetNo);
					}
					--iSkillSetUseCnt;
				}break;
			case SSCT_LEVEL:
				{
					if(iMyLevel < byCondValue)
					{
						kContOpen.push_back(bySetNo);
					}
					else
					{
						kContUseTmp.push_back(bySetNo);
					}
				}break;
			}
		}

		for(VEC_INT::const_iterator c_iter=kContUseTmp.begin();0<iSkillSetUseCnt && c_iter!=kContUseTmp.end();++c_iter)
		{
			kContOpen.push_back(*c_iter);
			--iSkillSetUseCnt;
		}
	}

	void lwSkillSetInit(lwUIWnd kWnd)
	{
		PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
		if(!pkMyPlayer || !pkMyActor){ return; }

		const CONT_DEFSKILLSET *pkContDefMap = NULL;
		g_kTblDataMgr.GetContDef(pkContDefMap);
		if( !pkContDefMap ){ return; }

		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd ){ return; }

		XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LIST_SKILLSET"));
		if( !pkListWnd ){ return; }

		pkListWnd->DeleteAllItem();

		VEC_INT kContSetOpen;
		GetSkillSetOpenList(pkMyPlayer, kContSetOpen);

		for(CONT_DEFSKILLSET::const_iterator c_iter=pkContDefMap->begin();c_iter!=pkContDefMap->end();++c_iter)
		{
			XUI::SListItem* pkItem = pkListWnd->AddItem(BM::vstring(c_iter->first));
			if( !pkItem || !pkItem->m_pWnd ){ continue; }

			XUI::CXUI_Wnd* pkSkillFrm = pkItem->m_pWnd;
			XUI::CXUI_Wnd* pkClose = pkSkillFrm->GetControl(L"IMG_SKILLSET_CLOSE");
			if( !pkClose ){ continue; }

			BYTE const bySetNo = c_iter->second.bySetNo;
			BYTE const byCondType = c_iter->second.byConditionType;
			BYTE const byCondValue = c_iter->second.byConditionValue;
			int  const iResNo = c_iter->second.iResNo;

			bool bCloseSet = true;
			VEC_INT::const_iterator open_it = std::find(kContSetOpen.begin(),kContSetOpen.end(),bySetNo);
			if(open_it != kContSetOpen.end())
			{
				bCloseSet = false;
			}

			BM::vstring vCloseText;
			if( bCloseSet )
			{
				if(SSCT_CLASS == byCondType)
				{
					vCloseText = TTW(401603);
					vCloseText.Replace(L"#CLASS#",byCondValue);
				}
				else if(SSCT_ITEM == byCondType)
				{
					vCloseText = TTW(401604);
				}
				else if(SSCT_LEVEL == byCondType)
				{
					vCloseText = TTW(401617);
					vCloseText.Replace(L"#LEVEL#",byCondValue-1);
				}
			}
			pkClose->Text(vCloseText);
			pkClose->Visible(bCloseSet);

			XUI::CXUI_Icon* pkSetIcon = dynamic_cast<XUI::CXUI_Icon*>(pkSkillFrm->GetControl(L"ICN_SKILLSET"));
			if( pkSetIcon )
			{
				SIconInfo kIconInfo(KUIG_SKILLSET_SETICON,bySetNo);
				kIconInfo.iIconResNumber = iResNo;
				pkSetIcon->SetIconInfo(kIconInfo);
				pkSetIcon->GrayScale( bCloseSet );
			}

			CONT_USER_SKILLSET::mapped_type const *pkSkillSet = pkMyActor->SkillSetAction().find(bySetNo);
			SetSkillUI(pkSkillSet, pkSkillFrm, bySetNo, bCloseSet, c_iter->second.byLimit);
		}

		XUI::CXUI_Wnd* pkSkillTree = XUIMgr.Get(L"SFRM_SKILL_WINDOW");
		if(pkSkillTree)
		{
			POINT2 kTreeLoc  = pkSkillTree->Location();
			POINT2 kTreeSize = pkSkillTree->Size();
			POINT2 kWndSize  = pkWnd->Size();

			int iLocX = 0;
			int iLocY = kTreeLoc.y + 10;

			if(kTreeLoc.x+kTreeSize.x+kWndSize.x > XUIMgr.GetResolutionSize().x)
			{
				iLocX = kTreeLoc.x - kWndSize.x;
				iLocX -= 10;
			}
			else
			{
				iLocX = kTreeLoc.x+kTreeSize.x;
			}

			pkWnd->Location(iLocX,iLocY);
		}

		XUI::CXUI_Button *pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_SAVE"));
		if(pkBtn)
		{ 
			pkBtn->Disable(true);
		}
	}

	void lwSkillSetSave()
	{
		PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
		PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkMyActor || !pkMyPlayer ){ return; }

		const CONT_DEFSKILLSET *pkContDefMap = NULL;
		g_kTblDataMgr.GetContDef(pkContDefMap);
		if( !pkContDefMap ){ return; }

		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_SKILLSET");
		if( !pkWnd ){ return; }

		XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LIST_SKILLSET"));
		if( !pkListWnd ){ return; }

		CONT_USER_SKILLSET kSkillSet;
		for( XUI::SListItem* pkBegin=pkListWnd->FirstItem();NULL!=pkBegin;
			pkBegin=pkListWnd->NextItem(pkBegin) )
		{
			BM::vstring kTmp(pkBegin->m_wstrKey);
			int const iKey = kTmp;

			XUI::CXUI_Wnd* pkSkillFrm = pkBegin->m_pWnd;
			CONT_DEFSKILLSET::const_iterator c_iter = pkContDefMap->find(iKey);
			if( c_iter!=pkContDefMap->end() && pkSkillFrm )
			{
				bool bInsert = false;
				CONT_USER_SKILLSET::mapped_type element;
				element.bySetNo = c_iter->second.bySetNo;
				for(int i=0; (i<c_iter->second.byLimit) && (c_iter->second.byLimit<MAX_SKILLSET_GROUP); ++i)
				{
					BM::vstring vStr(L"FRM_SKILLSET_SKILLICON");
					vStr += i;
					XUI::CXUI_Wnd* pkMainFrm = pkSkillFrm->GetControl(vStr);
					if( !pkMainFrm ){ continue; }

					XUI::CXUI_Wnd* pkDelay = pkMainFrm->GetControl(L"FRM_DELAY");
					XUI::CXUI_Wnd* pkIconWnd = pkMainFrm->GetControl(L"SFRM_SETICON_BG");
					if( !pkDelay || !pkIconWnd ){ continue; }

					pkDelay = pkDelay->GetControl(L"IMG_TIME");
					XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pkMainFrm->GetControl(L"ICN_SKILL"));
					if( !pkDelay || !pkIcon ){ continue; }

					int iDelay = 0;
					SIconInfo kDataIcon;

					pkDelay->GetCustomData(&iDelay, sizeof(iDelay));
					pkIcon->GetCustomData(&kDataIcon, sizeof(kDataIcon));

					element.byDelay[i] = static_cast<BYTE>(iDelay);
					element.byType[i] = static_cast<BYTE>(kDataIcon.iIconGroup);
					element.iSkillNo[i] = kDataIcon.iIconKey;
					if(0!=element.byDelay[i] || 0!=element.iSkillNo[i])
					{
						bInsert = true;
					}
				}

				if( bInsert )
				{
					kSkillSet.insert(std::make_pair(element.bySetNo, element));
				}
			}
		}

		//Save
		pkMyActor->SkillSetAction().ContSkillSet(kSkillSet);
		pkMyPlayer->GetMySkill()->ContSkillSet(kSkillSet);

		//Send Server
		BM::Stream kPacket(PT_C_T_REQ_SAVE_SKILLSET);
		PgSaveSkillSetPacket kSavePacket(kSkillSet);
		kSavePacket.WriteToPacket(kPacket);
		NETWORK_SEND(kPacket);
	}

	int lwGetClassLevel(int const iClass)
	{
		__int64 const TypeID = INT64_1 << iClass;
		if(0 != (UCLIMIT_ONE_CLASS&TypeID))
		{
			return 1;
		}
		else if(0 != (UCLIMIT_TWO_CLASS&TypeID))
		{
			return 2;
		}
		else if(0 != (UCLIMIT_THREE_CLASS&TypeID))
		{
			return 3;
		}
		else if(0 != (UCLIMIT_FOUR_CLASS&TypeID))
		{
			return 4;
		}

		return 0;
	}

	void lwSkillSetDoAction(BYTE const iSetNo)
	{
		PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
		if(!pkMyPlayer || !pkMyActor){ return; }

		switch( pkMyActor->SkillSetAction().CheckCanSkillSetDoAction(pkMyPlayer,iSetNo) )
		{
		case ESSR_NOT_CLASS:
		case ESSR_NOT_USE_ITEM:
			{
				return;
			}break;
		}

		DWORD dwMaxRemainTime = 0;
		if( !pkMyActor->SkillSetAction().GetMaxCoolTime(pkMyActor, iSetNo, dwMaxRemainTime) )
		{
			return;
		}
		if( dwMaxRemainTime > 0)
		{
			g_kChatMgrClient.ShowNoticeUI(TTW(235), 2, true, true);
			return;
		}

		pkMyActor->SkillSetAction().ReserveAction(iSetNo);
		pkMyActor->SkillSetAction().DoReservedAction(pkMyActor);
	}

	void lwCallSkillSetToolTip(BYTE const iSetNo)
	{
		PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
		if( !pkMyActor ){ return; }

		PgSkillSetAction::CONT_SKILLTEXT kContSkillText;
		switch( pkMyActor->SkillSetAction().CheckCanSkillSetDoAction(g_kPilotMan.GetPlayerUnit(),iSetNo) )
		{
		case ESSR_NOT_CLASS:
		case ESSR_NOT_USE_ITEM:
			{
			}break;
		default:
			{
				pkMyActor->SkillSetAction().GetSkillText(iSetNo, kContSkillText);
			}break;
		}

		BM::vstring vStr;
		//SkillSet 이름
		vStr += TTW(401605);//글꼴
		vStr += TTW(401600);
		vStr += iSetNo;

		//SkillSet에 포함된 Skill
		vStr += TTW(401606);//글꼴
		if(!kContSkillText.empty())
		{
			vStr += L"\n";
		}
		for(PgSkillSetAction::CONT_SKILLTEXT::const_iterator c_iter=kContSkillText.begin();c_iter!=kContSkillText.end();++c_iter)
		{
			vStr += L"\n";
			vStr += *c_iter;
		}
		std::wstring const & rkText = vStr;

		lwPoint2 pt(XUIMgr.MousePos().x+3,XUIMgr.MousePos().y+3);
		lwCallToolTipByText(0, rkText, pt, "ToolTip_Skill", 0, NULL);
	}

	void lwChangeSaveState(bool const bEnable)
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_SKILLSET");
		if( !pkWnd ){ return; }

		XUI::CXUI_Button *pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_SAVE"));
		if( !pkBtn ){ return; }

		pkBtn->Disable(!bEnable);
	}

	bool lwGetSaveState()
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_SKILLSET");
		if( !pkWnd ){ return false; }

		XUI::CXUI_Button *pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_SAVE"));
		if( !pkBtn ){ return false; }

		return pkBtn->Enable();
	}

	void lwSetMinTime(lwUIWnd kWnd, int const iSkillNo)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd();
		if( !pkWnd ){ return; }

		XUI::CXUI_Wnd* pkParent = pkWnd->Parent();
		if( !pkParent ){ return; }

		int const iBuildIdx = pkParent->BuildIndex()+1;
		BM::vstring vStr(L"FRM_SKILLSET_SKILLICON");
		vStr += iBuildIdx;

		XUI::CXUI_Wnd* pkMain = pkParent->Parent();
		if( !pkMain ){ return; }

		XUI::CXUI_Wnd* pkFrm = pkMain->GetControl(vStr);
		if( !pkFrm ){ return; }


		XUI::CXUI_Wnd* pkDelay = pkFrm->GetControl(L"FRM_DELAY");
		if( !pkDelay ){ return; }

		XUI::CXUI_Wnd* pkUp = pkDelay->GetControl(L"BTN_UP");
		XUI::CXUI_Wnd* pkDown = pkDelay->GetControl(L"BTN_DOWN");
		XUI::CXUI_Wnd* pkText = pkDelay->GetControl(L"IMG_TIME");
		if( !pkUp || !pkDown || !pkText ){ return; }

		int iDelayTIme = 0;
		if(JUMPSKILLNO==iSkillNo)
		{
			iDelayTIme = 3;//최소 3초 딜레이
		}
		pkUp->SetCustomData(&iDelayTIme,sizeof(iDelayTIme));
		pkDown->SetCustomData(&iDelayTIme,sizeof(iDelayTIme));

		int iOldDelayTIme = 0;
		pkText->GetCustomData(&iOldDelayTIme, sizeof(iOldDelayTIme));
		if(iOldDelayTIme < iDelayTIme)
		{
			pkText->SetCustomData(&iDelayTIme, sizeof(iDelayTIme));

			wchar_t szText[MAX_PATH] = {0,};
			swprintf_s(szText, MAX_PATH, L"%0.1f", iDelayTIme/10.f);
			pkText->Text(szText);
		}
	}

	void lwSkillSetClose()
	{
		if(lwGetSaveState())
		{
			CallYesNoMsgBox(TTW(401616), BM::GUID(), MBT_SKILLSET_SAVE);
		}
		else
		{
			XUIMgr.Close( L"SFRM_SKILLSET" );
		}
	}

	void lwCallSkillSetToolTip_Basic(BYTE const iSetNo)
	{
		PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
		if( !pkMyActor ){ return; }
		PgSkillSetAction::CONT_SKILLTEXT kContSkillText;
		switch( pkMyActor->SkillSetAction().CheckCanSkillSetDoAction(g_kPilotMan.GetPlayerUnit(),iSetNo) )
		{
		case ESSR_NOT_CLASS:
			{
			}break;
		default:
			{
				pkMyActor->SkillSetAction().GetSkillText(iSetNo, kContSkillText);
			}break;
		}

		BM::vstring vStr;
		//SkillSet 이름
		vStr += TTW(401605);//글꼴
		vStr += TTW(799402);
		vStr += (iSetNo-30);

		//SkillSet에 포함된 Skill
		vStr += TTW(401606);//글꼴
		if(!kContSkillText.empty())
		{
			vStr += L"\n";
		}
		CONT_USER_SKILLSET::mapped_type const * pkSkillSet = pkMyActor->SkillSetAction().find(iSetNo);
		if( pkSkillSet )
		{
			int iSkillCount = 0;
			int iReplayCount = 1;
			bool bReplaySkill = false;
			for(PgSkillSetAction::CONT_SKILLTEXT::const_iterator c_iter=kContSkillText.begin();c_iter!=kContSkillText.end();++c_iter)
			{
				int const iSkill = pkSkillSet->iSkillNo[iSkillCount];
				if( JUMPSKILLNO != iSkill)
				{
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pSkillDef;
					PgSkillTree::stTreeNode	*pkNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iSkill));
					if( !pkNode )
					{
						pSkillDef = kSkillDefMgr.GetDef(iSkill);
						int const iNameNo = pSkillDef->NameNo();
						pkNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iNameNo));
						if( !pkNode )
						{
							++iSkillCount;
							continue;
						}
					}
					pSkillDef = pkNode->GetSkillDef();
					if(!pSkillDef)
					{
						++iSkillCount;
						continue;
					}
					if( pkNode->IsLearned() 
						&& false == pkNode->IsTemporaryLevelChanged())
					{
						if( 0 != pSkillDef->m_iEffectID)
						{
							if(pSkillDef->m_iEffectID >= iSkill )
							{
								vStr +=TTW(799406);
							}
							else
							{
								vStr +=TTW(799407);
							}
						}
						else
						{
							vStr +=TTW(799406);
						}
					}
					else
					{
						vStr +=TTW(799407);
					}
				}
				else
				{
					vStr +=TTW(799406);
				}
				if( MAX_SKILLSET_GROUP > iSkillCount+1)
				{
					if( iSkill == pkSkillSet->iSkillNo[iSkillCount+1] )
					{
						++iReplayCount;
						bReplaySkill = true;
					}
					else
					{
						vStr += *c_iter;
						if( true == bReplaySkill )
						{
							wchar_t szTemp[MAX_PATH] = {0,};
							swprintf_s(szTemp, MAX_PATH, TTW(799408).c_str(), iReplayCount);
							vStr += szTemp;

							iReplayCount = 1;
							bReplaySkill = false;
						}
						vStr += L"\n";
					}
				}
				else
				{
					vStr += *c_iter;
					vStr += L"\n";
				}
				++iSkillCount;
			}
		}
		std::wstring const & rkText = vStr;

		lwPoint2 pt(XUIMgr.MousePos().x+3,XUIMgr.MousePos().y+3);
		lwCallToolTipByText(0, rkText, pt, "ToolTip_Skill", 0, NULL);
	}

	void lwClearBasicSkillSet()
	{
		PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
		if(!pkMyActor)
		{ 
			return; 
		}
		CONT_USER_SKILLSET kBasicSkillSet;
		pkMyActor->SkillSetAction().ContBasicSkillSet(kBasicSkillSet);
	}

	bool lwSetBasicSkillSet(int const iSetNo, int const iSkillNo, int const iCheckSkillNo, int const iDelay, int const iCount)
	{
		PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
		if(!pkMyPlayer || !pkMyActor)
		{ 
			return false; 
		}

		CONT_USER_SKILLSET kBasicSkillSet;
		kBasicSkillSet = pkMyActor->SkillSetAction().GetBasicSkillSet();

		bool bNewElement = true;
		CONT_USER_SKILLSET::mapped_type element;
		CONT_USER_SKILLSET::iterator iter_SkillSet = kBasicSkillSet.find( iSetNo );
		if( kBasicSkillSet.end() != iter_SkillSet )
		{
			element = (*iter_SkillSet).second;
			bNewElement = false;
		}

		int iSetCount = 0;
		while( 0 != element.iSkillNo[iSetCount] )
		{
			++iSetCount;
			if (MAX_SKILLSET_GROUP == iSetCount )
			{
				break;
			}
		}

		if( 0 == element.bySetNo)
		{
			element.bySetNo = iSetNo;
		}

		for(int i = 0; i < iCount; ++i)
		{
			element.iSkillNo[iSetCount] = iSkillNo;
			element.iCheckSkillNo[iSetCount] = iCheckSkillNo;
			if( 0 == i)
			{
				element.byDelay[iSetCount] = (iDelay);
			}
			else
			{
				element.byDelay[iSetCount] = 0;
			}
			PgSkillTree::stTreeNode	*pkNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iSkillNo));
			if(pkNode)
			{
				CSkillDef const* pkSkillDef = pkNode->GetSkillDef();
				if(pkSkillDef)
				{
					element.byType[iSetCount] = pkSkillDef->GetType();
				}
			}
			++iSetCount;

			if (MAX_SKILLSET_GROUP == iSetCount )
			{
				break;
			}
		}

		if( false == bNewElement )
		{
			(*iter_SkillSet).second = element;
		}
		else
		{
			kBasicSkillSet.insert(std::make_pair(element.bySetNo, element));
		}

		pkMyActor->SkillSetAction().ContBasicSkillSet(kBasicSkillSet);

		return true;
	}
	void lwApplyBasicSkillSetToSkillSet()
	{		
		PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
		if(!pkMyPlayer || !pkMyActor)
		{ 
			return; 
		}
		pkMyActor->SkillSetAction().ApplyBasicSkillSetToSkillSet();
	}

	bool lwCheckUseSkillSetWnd( lwUIWnd kWnd, bool const bCheckAll )
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if(!pkWnd)
		{
			return false;
		}
		XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pkWnd);
		if(!pIcon)
		{
			return false;
		}	
		SIconInfo const kIcon = pIcon->IconInfo();

		PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();		
		if(!pkMyPlayer)
		{ 
			return false; 
		}
		PgMySkill * pkMySkill = pkMyPlayer->GetMySkill();
		if( !pkMySkill )
		{
			return false;
		}
		PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
		if(!pkMyActor)
		{ 
			pIcon->GrayScale(true);
			return false; 
		}

		CONT_USER_SKILLSET kSkillSet;
		kSkillSet = pkMyActor->SkillSetAction().GetSkillSet();

		CONT_USER_SKILLSET::mapped_type element;
		CONT_USER_SKILLSET::iterator iter_SkillSet = kSkillSet.find( kIcon.iIconKey );
		if( kSkillSet.end() != iter_SkillSet )
		{
			element = (*iter_SkillSet).second;
			for(int iSkillCount = 0; iSkillCount < MAX_SKILLSET_GROUP; ++iSkillCount)
			{
				int iCheckSkillNo = 0;
				if( 0 != element.iCheckSkillNo[iSkillCount] )
				{
					iCheckSkillNo = element.iCheckSkillNo[iSkillCount];
				}
				else
				{
					iCheckSkillNo = element.iSkillNo[iSkillCount];
				}
				if( 0 == iCheckSkillNo)
				{
					break;
				}
				else if( JUMPSKILLNO == iCheckSkillNo)
				{
					continue;
				}
				CSkillDef const* pSkillDef = NULL;
				PgSkillTree::stTreeNode *pkNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iCheckSkillNo));
				if( !pkNode )
				{
					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					pSkillDef = kSkillDefMgr.GetDef(iCheckSkillNo);
					int const iNameNo = pSkillDef->NameNo();
					pkNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iNameNo));
					if( !pkNode )
					{
						pIcon->GrayScale(true);
						return false;
					}
				}
				else
				{
					pSkillDef = pkNode->GetSkillDef();
				}
				if(!pSkillDef)
				{
					pIcon->GrayScale(true);
					return false;
				}
				if(bCheckAll)	//true면 스킬세트 전부 획득했는지 검사
				{
					if( false == pkMyPlayer->GetMySkill()->IsExist(iCheckSkillNo, true) )
					{
						return false;
					}
				}
				else	//하나라도 획득했는지 검사
				{
					if( true == pkMyPlayer->GetMySkill()->IsExist(iCheckSkillNo, true) )
					{
						return true;
					}
				}
			}
		}
		else
		{
			return false;
		}
		pIcon->GrayScale(!bCheckAll);
		return bCheckAll;
	}

	bool lwCheckUseSkillSetNo( int const iSetNo, bool const bCheckAll )	//스킬획득 여부 리턴
	{		
		PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();		
		if(!pkMyPlayer)
		{ 
			return false; 
		}
		PgMySkill * pkMySkill = pkMyPlayer->GetMySkill();
		if( !pkMySkill )
		{
			return false;
		}
		PgActor *pkMyActor = g_kPilotMan.GetPlayerActor();
		if(!pkMyActor)
		{ 
			return false; 
		}

		CONT_USER_SKILLSET kSkillSet;
		kSkillSet = pkMyActor->SkillSetAction().GetSkillSet();

		CONT_USER_SKILLSET::mapped_type element;
		CONT_USER_SKILLSET::iterator iter_SkillSet = kSkillSet.find( iSetNo );
		if( kSkillSet.end() != iter_SkillSet )
		{
			element = (*iter_SkillSet).second;
			for(int iSkillCount = 0; iSkillCount < MAX_SKILLSET_GROUP; ++iSkillCount)
			{
				int iCheckSkillNo = 0;
				if( 0 != element.iCheckSkillNo[iSkillCount] )
				{
					iCheckSkillNo = element.iCheckSkillNo[iSkillCount];
				}
				else
				{
					iCheckSkillNo = element.iSkillNo[iSkillCount];
				}

				if( 0 == iCheckSkillNo)
				{
					break;
				}
				else if( JUMPSKILLNO == iCheckSkillNo)
				{
					continue;
				}

				if(bCheckAll)	//true면 스킬세트 전부 획득했는지 검사
				{
					if( false == pkMyPlayer->GetMySkill()->IsExist(iCheckSkillNo, true) )
					{
						return false;
					}
				}
				else	//하나라도 획득했는지 검사
				{
					if( true == pkMyPlayer->GetMySkill()->IsExist(iCheckSkillNo, true) )
					{
						return true;
					}
				}
			}
		}
		else
		{
			return false;
		}
		return bCheckAll;
	}

	bool lwCallHelpBasicSkillSet( int const iSetNo, bool const bCheckAll )	//기초 스킬 세트 알림
	{
		if( false == lwCheckUseSkillSetNo(iSetNo, bCheckAll) )
		{
			return false;
		}
		if(bCheckAll)
		{
			bool bLearned = false;
			bLearned = lua_tinker::call<bool, int>("GetLearnBasicSkill", iSetNo);
			if( bLearned )
			{
				return false;
			}
		}
		else
		{
			bool bReady = false;
			bReady = lua_tinker::call<bool, int>("GetReadyBasicSkill", iSetNo);
			if( bReady )
			{
				return false;
			}
		}
		int const iSet = iSetNo - 30;
		XUI::CXUI_Wnd* pHelpUI = XUIMgr.Call(L"FRM_HELP_BASIC_SKILLSET");
		if(!pHelpUI)
		{
			return false;
		}
		XUI::CXUI_Wnd* pTitle = pHelpUI->GetControl(L"FRM_HELP_TITLE");
		if(pTitle)
		{
			wchar_t szTemp[MAX_PATH] = {0,};
			if(bCheckAll)
			{
				swprintf_s(szTemp, MAX_PATH, TTW(799403).c_str(), iSet);
			}
			else
			{
				swprintf_s(szTemp, MAX_PATH, TTW(799411).c_str() );
			}
			pTitle->Text(szTemp);
		}
		XUI::CXUI_Wnd* pContents = pHelpUI->GetControl(L"FRM_HELP_CONTENTS");
		if(pContents)
		{
			wchar_t szTemp[MAX_PATH] = {0,};
			if(bCheckAll)
			{
				swprintf_s(szTemp, MAX_PATH, TTW(799409).c_str(), iSet, iSet);
			}
			else
			{
				swprintf_s(szTemp, MAX_PATH, TTW(799412).c_str() );
			}
			pContents->Text(szTemp);
		}
		XUI::CXUI_Wnd* pAutoSkillSetBtn = pHelpUI->GetControl(L"FRM_HELP_AUTOSET");
		if(pAutoSkillSetBtn)
		{
			pAutoSkillSetBtn->Enable(bCheckAll);
			pAutoSkillSetBtn->Visible(bCheckAll);
		}
		XUI::CXUI_Wnd* pCallSkillSetBtn = pHelpUI->GetControl(L"FRM_HELP_CALL_SKILLSET");
		if(pCallSkillSetBtn)
		{
			pCallSkillSetBtn->Enable(!bCheckAll);
			pCallSkillSetBtn->Visible(!bCheckAll);
		}
		XUI::CXUI_Wnd* pSkillSet = pHelpUI->GetControl(L"SFRM_SKILLSET");
		if(pSkillSet)
		{
			XUI::CXUI_Wnd* pSetIcon = pSkillSet->GetControl(L"ICN_SKILLSET");
			if(pSetIcon)
			{
				XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pSetIcon);
				if(pIcon)
				{
					SIconInfo kIconInfo = pIcon->IconInfo();
					kIconInfo.iIconKey = iSetNo;
					pIcon->SetIconInfo(kIconInfo);
					if(bCheckAll)
					{
						lua_tinker::call<void, int, bool>("SetLearnBasicSkill", iSetNo, true);
					}
					else
					{
						lua_tinker::call<void, int, bool>("SetReadyBasicSkill", iSetNo, true);
					}
					return true;
				}
			}
		}
		return false;
	}

	void lwSetQuickInvBasicSkillSet(int const iQuickInvSlot, int const iSetNo )
	{
		XUI::CXUI_Wnd* pHelpUI = XUIMgr.Activate(L"FRM_HELP_BASIC_SKILLSET");
		if(!pHelpUI)
		{
			return;
		}
		XUI::CXUI_Wnd* pSkillSet = pHelpUI->GetControl(L"SFRM_SKILLSET");
		if(!pSkillSet)
		{
			return;
		}
		XUI::CXUI_Wnd* pSetIcon = pSkillSet->GetControl(L"ICN_SKILLSET");
		if(!pSetIcon)
		{
			return;
		}
		XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pSetIcon);
		if(!pIcon)
		{
			return;
		}
		SIconInfo kIconInfo = pIcon->IconInfo();
		if( 0 == kIconInfo.iIconKey )
		{
			return;
		}

		PgPlayer *pkMyPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkMyPlayer)
		{ 
			return; 
		}
		PgQuickInventory* pkQInv = pkMyPlayer->GetQInven();
		if(pkQInv)
		{
			SQuickInvInfo kNewQuickInvInfo;
			kNewQuickInvInfo.Set(kIconInfo.iIconGroup, kIconInfo.iIconKey);
			Send_PT_C_M_REQ_REGQUICKSLOT( pkQInv->AdjustSlotIDX(iQuickInvSlot), kNewQuickInvInfo);
		}
	}

	int lwGetHelpSkillSetNo(lwUIWnd* UISelf)
	{		
		XUI::CXUI_Wnd* pHelpUI = XUIMgr.Activate(L"FRM_HELP_BASIC_SKILLSET");
		if(!pHelpUI)
		{
			return 0;
		}
		XUI::CXUI_Wnd* pSkillSet = pHelpUI->GetControl(L"SFRM_SKILLSET");
		if(!pSkillSet)
		{
			return 0;
		}
		XUI::CXUI_Wnd* pSetIcon = pSkillSet->GetControl(L"ICN_SKILLSET");
		if(!pSetIcon)
		{
			return 0;
		}
		XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pSetIcon);
		if(!pIcon)
		{
			return 0;
		}
		SIconInfo kIconInfo = pIcon->IconInfo();
		return kIconInfo.iIconKey;
	}

	void lwCallHelpSkillSetToolTip(int const iSetNo, lwPoint2& pt)
	{		
		wchar_t szTemp[MAX_PATH] = {0,};
		if(31 == iSetNo)
		{
			swprintf_s(szTemp, MAX_PATH, TTW(799404).c_str(), "A");
		}
		else if(32 == iSetNo)
		{
			swprintf_s(szTemp, MAX_PATH, TTW(799404).c_str(), "S");
		}
		lwCallMutableToolTipByText(szTemp, pt);
	}
}