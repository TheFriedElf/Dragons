#include "StdAfx.h"
#include "PgMobileSuit.h"
#include "lwUI.h"
#include "PgRenderMan.h"
#include "PgEventTimer.h"
#include "PgPilotMan.h"
#include "PgAchieveNfyMgr.h"
#include "PgChatMgrClient.h"
#include "PgCommandMgr.h"
#include "Variant/PgStringUtil.h"
#include "PgMoveAnimateWnd.h"
#include "PgOption.h"

extern bool GetDefString(int const iTextNo, wchar_t const *&pString);
extern void MakeEquipAddOption(PgBase_Item const& kItem, CItemDef const* pDef, bool const bOrtherActor, std::wstring& wstrText, bool const bColor);

namespace Quest
{
	extern void SetCutedTextLimitLength(XUI::CXUI_Wnd *pkWnd, std::wstring const &rkText, std::wstring kTail, long const iCustomTargetWidth = 0);
}

namespace PgChatMgrUtil
{
	extern DWORD ChatInputColor(EChatType const Type);
}

float const ACH_NFY_CLOSE_INTERVAL_TIME = 20.0f;
float const ACH_NM_CLOSE_INTERVAL_TIME = 6.0f;
int const MAX_LIST_ITEM = 3;
int const MAX_FLASH_COUNT = 6;
int const MAX_ACHIEVESTATE_LEVELLIMIT = 5;

inline bool IsShowAchieveOK()
{
	if(!g_pkWorld) return false;
	if(g_pkWorld->IsHaveAttr(GATTR_FLAG_EMPORIABATTLE)) return false;
	if(XUIMgr.Get(L"FRM_ACHIEVEMENT_POPUP")) return false;
	if(XUIMgr.Get(L"FRM_EMBATTLE_RESULT")) return false;
	if(XUIMgr.Get(L"FRM_LOADING_IMG")) return false;
	if(XUIMgr.Get(L"FRM_SELECTWND_OUTLINE")) return false;
	return true;
}

inline bool IsShowAchieveStatusOK()
{
	if( g_kLocal.ServiceRegion() != LOCAL_MGR::NC_DEVELOP)
	{// 미개방 컨텐츠. 업적 알림 #NOT_OPEN#
		return false;
	}

	if(!g_pkWorld) return false;
	static const int GATTR_FLAG = GATTR_FLAG_HIDDEN_F | GATTR_FLAG_CHAOS_F | GATTR_FLAG_STATIC_DUNGEON | GATTR_FLAG_BATTLESQUARE | GATTR_FLAG_HARDCORE_DUNGEON | GATTR_FLAG_SUPER | GATTR_FLAG_EMPORIABATTLE;
	T_GNDATTR const kAttr = g_pkWorld->GetAttr();
	if(kAttr & GATTR_FLAG) return false;
	if(XUIMgr.Get(L"FRM_LOADING_IMG")) return false;
	if(XUIMgr.Get(L"FRM_SELECTWND_OUTLINE")) return false;
	if(XUIMgr.Get(L"FRM_CASH_SHOP")) return false;
	if(XUIMgr.Get(L"FRM_EMBATTLE_RESULT")) return false;
	if(g_kGlobalOption.GetValue(XML_ELEMENT_ETC, std::string("DISPLAY_ACHIEVE_STATUS"))) return false;

	return true;
}

namespace lwAchieveNfyMgr
{
	typedef struct tagAchieveSortKey
	{
		tagAchieveSortKey(int Idx=0, float Rate=0.f)
			: iIdx(Idx), fRate(Rate)
		{};
		bool operator<(tagAchieveSortKey const& rhs) const
		{
			return (fRate > rhs.fRate);
		}

		int iIdx;
		float fRate;
	}SAchieveSortKey;

	typedef std::vector<TBL_DEF_ACHIEVEMENTS> CONT_ACHIEVE;
	typedef std::map<SAchieveSortKey, TBL_DEF_ACHIEVEMENTS> CONT_ACHIEVESORT;	

	void GetAchieveStatuesList(lwAchieveNfyMgr::CONT_ACHIEVE & kContMain, lwAchieveNfyMgr::CONT_ACHIEVESORT & kContSub);
	void lwCheckAchieveStatusNfy();
	void lwOpenRecommandAchieveInfo();
	void lwSettingXmlLoad();
	void lwSwitchAchieveStatus(lwUIWnd kWnd, bool const bBig);
	void lwAchieveStatusInit();
	bool IsAchieveStatusFirstLogin();
	void lwClose_FRM_ACHIEVEMENT_POPUP();

	CONT_ACHIEVESORT m_kContRecommand;
	bool m_bFirstLogin = true;
}

void lwAchieveNfyMgr::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	
	def(pkState, "OnClickAddonButton", &lwAchieveNfyMgr::lwOnClickAddonButton);
	def(pkState, "UpdateSlideTextViewUI", &lwAchieveNfyMgr::lwUpdateSlideTextViewUI);
	def(pkState, "SendAchieveGreetMessage", &lwAchieveNfyMgr::lwSendAchieveGreetMessage);

	def(pkState, "OpenAchievementInfo", &lwAchieveNfyMgr::lwOpenAchievementInfo);
	def(pkState, "OpenRecommandAchieveInfo", &lwAchieveNfyMgr::lwOpenRecommandAchieveInfo);
	def(pkState, "AchieveStatusInit", &lwAchieveNfyMgr::lwAchieveStatusInit);
	def(pkState, "CheckAchieveStatusNfy", &lwAchieveNfyMgr::lwCheckAchieveStatusNfy);
	def(pkState, "SettingXmlLoad", &lwAchieveNfyMgr::lwSettingXmlLoad);
	def(pkState, "SwitchAchieveStatus", &lwAchieveNfyMgr::lwSwitchAchieveStatus);
	def(pkState, "Close_FRM_ACHIEVEMENT_POPUP", &lwAchieveNfyMgr::lwClose_FRM_ACHIEVEMENT_POPUP);
}

void lwAchieveNfyMgr::lwCheckNewAchieveNfy()
{
	CONT_DEF_ACHIEVEMENTS const * pContDefAch = NULL;
	g_kTblDataMgr.GetContDef(pContDefAch);	//업적 데프
	if( !pContDefAch ){ return; }

	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkMyPlayer ){ return; }

	PgAchievements* pkAchiv = pkMyPlayer->GetAchievements();
	if( !pkAchiv ){ return; }

	int const iLevel = pkMyPlayer->GetAbil(AT_LEVEL);

	m_kContRecommand.clear();
	CONT_DEF_ACHIEVEMENTS::const_iterator ach_it = pContDefAch->begin();
	while(pContDefAch->end() != ach_it)
	{
		bool const bIsComplet = pkAchiv->IsComplete(ach_it->second.iSaveIdx);
		if( (iLevel==ach_it->second.iLevel) && !bIsComplet )
		{
			PgAchieveValueCtrl kValue((*ach_it).second.iSaveIdx, *pkMyPlayer);
			int const iMax = kValue.Max();
			int const iNow = kValue.Now();
			float const fRate = kValue.Rate() / 100.f;

			if( iNow<iMax )
			{
				SAchieveSortKey	Key(ach_it->second.iIdx, fRate);
				std::pair<CONT_ACHIEVESORT::iterator, bool> Rst = m_kContRecommand.insert(std::make_pair(Key, ach_it->second));
				if( !Rst.second )
				{
					CAUTION_LOG(BM::LOG_LV1, __FL__ << L"Overlapping Achievement: [" << Rst.first->second.iIdx <<L"]");
					//assert(0);//중복
				}
			}
		}
		++ach_it;
	}

	if( !m_kContRecommand.empty()
	&& (g_pkWorld && !g_pkWorld->IsHaveAttr(GATTR_FLAG_MYHOME)) )
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Activate(L"BTN_ACHIEVEMENT_MINI");
		if( pkWnd )
		{
			DWORD const dwAliveTime = g_kAchieveNfyMgr.GetIconAliveTime();
			pkWnd->AliveTime(dwAliveTime);
			pkWnd->SetInvalidate();
		}
	}
}

void lwAchieveNfyMgr::GetAchieveStatuesList(lwAchieveNfyMgr::CONT_ACHIEVE & kContMain, lwAchieveNfyMgr::CONT_ACHIEVESORT & kContSub)
{
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkMyPlayer || !pkMyPlayer->GetAchievements() ){ return; }

	PgAchievements * const pkAchiv = pkMyPlayer->GetAchievements();
	int const iLevel = pkMyPlayer->GetAbil(AT_LEVEL);
	float const fCheckProgress = g_kAchieveNfyMgr.GetCheckProgress();

	CONT_DEF_ACHIEVEMENTS const * pContDefAch = NULL;
	g_kTblDataMgr.GetContDef(pContDefAch);	//업적 데프
	if( !pContDefAch ){ return; }	

	GET_DEF(CItemDefMgr, kItemDefMgr);

	kContMain.clear();
	kContSub.clear();
	for(CONT_DEF_ACHIEVEMENTS::const_iterator ach_it=pContDefAch->begin(); pContDefAch->end()!=ach_it; ++ach_it)
	{
		bool const bIsComplet = pkAchiv->IsComplete(ach_it->second.iSaveIdx);
		if( bIsComplet )
		{
			continue;
		}

		CItemDef const *pDef = kItemDefMgr.GetDef(ach_it->second.iItemNo);
		if( !pDef ){ continue; }

		if( IG_ARTIFACT > pDef->GetAbil(AT_GRADE) )
		{
			continue;
		}

		PgAchieveValueCtrl kValue((*ach_it).second.iSaveIdx, *pkMyPlayer);
		int const iMax = kValue.Max();
		int const iNow = kValue.Now();
		float const fRate = kValue.Rate() / 100.f;

		if( iNow>=iMax )
		{
			continue;
		}

		SAchieveSortKey	Key(ach_it->second.iSaveIdx, fRate);
		if( iLevel<=ach_it->second.iLevel && ach_it->second.iLevel<iLevel+MAX_ACHIEVESTATE_LEVELLIMIT )
		{
			kContMain.push_back(ach_it->second);
		}

		if( fCheckProgress<=fRate )
		{
			kContSub.insert(std::make_pair(Key, ach_it->second));
		}
	}
}

void SetMoveTextWidth(PgMoveAnimateWnd *pkWnd, long const iCustomTargetWidth=0)
{
	if( !pkWnd )
	{
		return;
	}

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(pkWnd->Font());
	if( !pFont )
	{
		return;
	}

	long const iLimitWidth = (iCustomTargetWidth ? iCustomTargetWidth : pkWnd->Size().x);

	XUI::CXUI_Style_String const & kStyleString = pkWnd->StyleText();
	POINT kSize = Pg2DString::CalculateOnlySize(kStyleString);

	int const iEndX = (kSize.x>iLimitWidth) ? -kSize.x : 0;//(kSize.x>iLimitWidth) ? -(kSize.x-iLimitWidth) : 0;
	int const iEndY = pkWnd->EndPoint().y;
	int const iEndZ = pkWnd->EndPoint().z;

	NiPoint3 kEndPoint(iEndX, iEndY, iEndZ);
	pkWnd->EndPoint(kEndPoint);
}

void lwAchieveNfyMgr::lwSwitchAchieveStatus(lwUIWnd kWnd, bool const bBig)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	if( !pkWnd ){ return; }
	
	XUI::CXUI_Wnd* pkTmp = NULL;
	int iBig = 0;
	int iSmall = 0;

	pkTmp = pkWnd->GetControl(L"BTN_SWITCH_CLOSE");
	pkTmp ? pkTmp->Visible(bBig) : 0;

	pkTmp = pkWnd->GetControl(L"BTN_SWITCH_OPEN");
	pkTmp ? pkTmp->Visible(!bBig) : 0;	

	pkTmp = pkWnd->GetControl(L"FRM_BIG");
	if( pkTmp )
	{
		pkTmp->Visible(bBig);
		iBig = pkTmp->Size().y;
	}

	pkTmp = pkWnd->GetControl(L"FRM_SMALL");
	if( pkTmp )
	{
		pkTmp->Visible(!bBig);
		iSmall = pkTmp->Size().y;
	}

	POINT2 kSize = pkWnd->Size();
	kSize.y = bBig ? iBig : iSmall;
	pkWnd->Size(kSize);
}


bool lwAchieveNfyMgr::IsAchieveStatusFirstLogin()
{ 
	return m_bFirstLogin; 
}

void lwAchieveNfyMgr::lwAchieveStatusInit()
{
	m_bFirstLogin = true;
}

void lwAchieveNfyMgr::lwCheckAchieveStatusNfy()
{
	static ACE_RW_Thread_Mutex kMutex;
	BM::CAutoMutex kLock(kMutex, true);

	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkMyPlayer ){ return; }

	PgAchievements* pkAchiv = pkMyPlayer->GetAchievements();
	if( !pkAchiv ){ return; }

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"FMA_ACHIEVEMENT_STATUS");
	if( pkWnd ){ return; }

	CONT_ACHIEVE kContMain;
	CONT_ACHIEVESORT kContSub;
	GetAchieveStatuesList(kContMain, kContSub);

	std::random_shuffle(kContMain.begin(), kContMain.end());

	CONT_ACHIEVE::const_iterator kMain_it = kContMain.begin();
	if( kMain_it==kContMain.end() ){ return; }
	CONT_ACHIEVESORT::mapped_type const & kMainAchieve = *kMain_it;

	pkWnd = XUIMgr.Call(L"FMA_ACHIEVEMENT_STATUS");
	if( !pkWnd ){ return; }	

	m_bFirstLogin = false;

	XUI::CXUI_Wnd* pkTitle = pkWnd->GetControl(L"FRM_TITLE");
	if( pkTitle )
	{
		const wchar_t *pTitle = NULL;
		std::wstring kTitle;
		if(GetDefString(kMainAchieve.iDiscriptionNo, pTitle))
		{
			kTitle = pTitle;
		}

		Quest::SetCutedTextLimitLength(pkTitle, kTitle, L"..");
	}

	XUI::CXUI_AniBar* pkAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pkWnd->GetControl(L"ANB_PERCENT"));
	if( pkAniBar )
	{
		PgAchieveValueCtrl kValue(kMainAchieve.iSaveIdx, *pkMyPlayer);
		int const iMax = kValue.Max();
		int const iNow = kValue.Now();

		BM::vstring vStr;
		vStr = iNow;
		vStr += L"{C=0xFFFFFFFF/}";
		vStr += L"/";
		vStr += iMax;

		pkAniBar->Max(iMax);
		pkAniBar->Now(iNow);
		pkAniBar->Text(vStr);
	}

	BM::vstring vstrAbilText;
	PgBase_Item kItem;
	if(S_OK == CreateSItem(kMainAchieve.iItemNo, 1, 0, kItem))
	{
		std::wstring wstrText;
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(kMainAchieve.iItemNo);

		MakeEquipAddOption(kItem, pDef, false, wstrText, false);
		vstrAbilText = wstrText;
	}

	XUI::CXUI_Wnd* pkDiscr = pkWnd->GetControl(L"FRM_DISCRIPT");
	if( pkDiscr )
	{
		PgMoveAnimateWnd* pkAbil = dynamic_cast<PgMoveAnimateWnd*>(pkDiscr->GetControl(L"FMA_ABILS"));
		if( pkAbil )
		{
			vstrAbilText.Replace( L"\n", L", " );
			std::wstring kAbilText(vstrAbilText);
			PgStringUtil::RTrim<std::wstring>(kAbilText, L", ");
			pkAbil->Text(kAbilText);
			SetMoveTextWidth(pkAbil, pkDiscr->Size().x);
		}
	}

	XUI::CXUI_Wnd* pkImg = pkWnd->GetControl(L"IMG_ITEM");
	if( pkImg )
	{
		if(kItem.ItemNo())
		{
			lwUIWnd(pkImg).SetCustomDataAsSItem(kItem);
			pkImg->SetInvalidate();
			pkImg->Visible(true);
		}
		else
		{
			pkImg->Visible(false);
		}
	}

	XUI::CXUI_Wnd* pkBig = pkWnd->GetControl(L"FRM_BIG");
	if( pkBig )
	{
		XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkBig->GetControl(L"BLD_ICON_SOON"));
		if( pkBuilder )
		{
			int const MAX_SLOT_COUNT = pkBuilder->CountX() * pkBuilder->CountY();
			PgBase_Item kSlotItem;
			CONT_ACHIEVESORT::const_iterator kSub_it = kContSub.begin();
			for(int i=0; i<MAX_SLOT_COUNT; ++i)
			{
				BM::vstring vStr(L"FRM_ICON_SOON");
				vStr += i;

				XUI::CXUI_Wnd* pkSlot = pkBig->GetControl(vStr);
				if( !pkSlot ){ continue; }

				XUI::CXUI_Wnd* pkItem = pkSlot->GetControl(L"IMG_ITEM");
				XUI::CXUI_Wnd* pkPercent = pkSlot->GetControl(L"FRM_PERCENT");
				if(!pkItem || !pkPercent){ continue; }

				if( kSub_it != kContSub.end() )
				{
					CONT_ACHIEVESORT::mapped_type const & kSubAchieve = kSub_it->second;
					if(S_OK == CreateSItem(kSubAchieve.iItemNo, 1, 0, kSlotItem))
					{
						lwUIWnd(pkItem).SetCustomDataAsSItem(kSlotItem);
					}
					else
					{
						pkItem->ClearCustomData();
					}

					PgAchieveValueCtrl kValue(kSubAchieve.iSaveIdx, *pkMyPlayer);

					BM::vstring vPerStr;
					vPerStr = kValue.Rate();
					vPerStr += L"%";
					pkPercent->Text(vPerStr);

					++kSub_it;
				}
				else
				{
					pkItem->ClearCustomData();
					pkPercent->Text(L"");
				}
			}
		}
	}
	
	if( kContSub.empty() )
	{
		lwSwitchAchieveStatus(lwUIWnd(pkWnd), false);

		XUI::CXUI_Wnd* pkTmp = pkWnd->GetControl(L"BTN_SWITCH_OPEN");
		pkTmp ? pkTmp->Visible(false) : 0;
	}
	else
	{
		static bool bOldBigView = true;
		XUI::CXUI_Wnd* pkOpen = pkWnd->GetControl(L"BTN_SWITCH_OPEN");
		XUI::CXUI_Wnd* pkClose = pkWnd->GetControl(L"BTN_SWITCH_CLOSE");		
		if(pkOpen && pkClose)
		{
			if(pkOpen->Visible() || pkClose->Visible())
			{
				XUI::CXUI_Wnd* pkBig = pkWnd->GetControl(L"FRM_BIG");
				bOldBigView = pkBig ? pkBig->Visible() : true;
			}
		}

		lwSwitchAchieveStatus(lwUIWnd(pkWnd), bOldBigView);
	}

	PgMoveAnimateWnd* pkMoveFrm = dynamic_cast<PgMoveAnimateWnd*>(pkWnd);
	if(pkMoveFrm)
	{
		pkMoveFrm->PauseTime(g_kAchieveNfyMgr.GetStatusAliveTime()/1000.f);
	}
}

void lwAchieveNfyMgr::lwOpenRecommandAchieveInfo()
{
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkMyPlayer ){ return; }
	PgAchievements* pkAchieve = pkMyPlayer->GetAchievements();
	if( !pkAchieve ){ return; }

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Activate(L"SFRM_NEW_ACHIEVEMENTS");
	if( !pkWnd ){ return; }

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"ACHIEVEMENT_LIST"));
	if( !pkList ){ return; }

	pkList->ClearList();

	if( m_kContRecommand.empty() )
	{
		pkWnd->Close();
		return;
	}

	CONT_ACHIEVESORT::const_iterator c_iter = m_kContRecommand.begin();
	while( m_kContRecommand.end() != c_iter )
	{
		XUI::SListItem* pkItem = pkList->AddItem(BM::vstring(c_iter->second.iSaveIdx));
		if( !pkItem ){ return; }

		XUI::CXUI_Wnd* pkItemWnd = pkItem->m_pWnd;
		if( !pkItemWnd ){ return; }


		int const iIndex = c_iter->second.iIdx;
		TBL_DEF_ACHIEVEMENTS const& rkValue = c_iter->second;

		bool const bIsComplet = pkAchieve->IsComplete(rkValue.iSaveIdx);
		bool const bHasItem = pkAchieve->CheckHasItem(rkValue.iSaveIdx);

		pkItemWnd->UVUpdate((bIsComplet)?(1):(2));
		pkItemWnd->SetCustomData(&iIndex, sizeof(iIndex));
		XUI::CXUI_Wnd* pkTitle = pkItemWnd->GetControl(L"SFRM_TITLE");
		if( pkTitle )
		{
			const wchar_t *pTitle = NULL;
			std::wstring kTitle;
			if(0 < iIndex && GetDefString(rkValue.iTitleNo, pTitle))
			{
				kTitle = pTitle;
			}
			pkTitle->Text(kTitle);
			pkTitle->GrayScale(!bIsComplet);
		}

		XUI::CXUI_Wnd* pkDiscr = pkItemWnd->GetControl(L"FRM_DISCRIPT");
		if( pkDiscr )
		{
			const wchar_t *pDisc = NULL;
			std::wstring kDisc;
			if(0 < iIndex && GetDefString(rkValue.iDiscriptionNo, pDisc))
			{
				kDisc = pDisc;
			}
			Quest::SetCutedTextLimitLength(pkDiscr, kDisc, L"..");
		}

		XUI::CXUI_Wnd* pkImg = pkItemWnd->GetControl(L"IMG_ITEM");
		if( pkImg )
		{
			PgBase_Item kItem;
			if(S_OK == CreateSItem(rkValue.iItemNo, 1, 0, kItem))
			{
				lwUIWnd(pkImg).SetCustomDataAsSItem(kItem);
				pkImg->GrayScale(!bIsComplet);
				pkImg->SetInvalidate();
				pkImg->Visible(true);
			}
			else
			{
				pkImg->Visible(false);
			}
		}

		XUI::CXUI_AniBar* pkAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pkItemWnd->GetControl(L"ANB_PERCENT"));
		if( pkAniBar )
		{
			PgAchieveValueCtrl kValue(rkValue.iSaveIdx, *pkMyPlayer);
			int const iMax = kValue.Max();
			int const iNow = kValue.Now();

			BM::vstring vStr;
			vStr = iNow;
			vStr += L"{C=0xFFFFFFFF/}";
			vStr += L"/";
			vStr += iMax;

			pkAniBar->Max(iMax);
			pkAniBar->Now(iNow);
			pkAniBar->Text(vStr);
		}

		++c_iter;
	}
}

void lwAchieveNfyMgr::lwSettingXmlLoad()
{
	g_kAchieveNfyMgr.ParseXml("Setting.xml");
}

void lwAchieveNfyMgr::AddNewSlideText(PgSlideTextMgr::E_ADD_TEXT_TYPE const Type, std::wstring const& kText, BM::Stream const& kAddon)
{
	PgSlideTextMgr::S_SLIDE_TEXT_INFO kInfo;
	kInfo.Type = Type;
	kInfo.kText = kText;
	kInfo.kAddon = kAddon;
	g_kSlideTextMgr.Push(kInfo);

	XUI::CXUI_Wnd* pkMainUI = XUIMgr.Get(L"FRM_ACHIEVE_NEWS");
	if( pkMainUI && !pkMainUI->IsClosed() )
	{
		return;
	}
	else
	{
		pkMainUI = XUIMgr.Call(L"FRM_ACHIEVE_NEWS");
		if( !pkMainUI )
		{
			return;
		}
	}

	SetSlideTextViewUI(pkMainUI);
}

void lwAchieveNfyMgr::SetSlideTextViewUI(XUI::CXUI_Wnd* pkMainUI)
{
	PgSlideTextMgr::S_SLIDE_TEXT_INFO kInfo;
	if( g_kSlideTextMgr.Pop(kInfo) )
	{
		BM::Stream kPacket;
		kPacket.Push( kInfo.Type );
		kPacket.Push( kInfo.kAddon );

		pkMainUI->ClearCustomData();
		lwUIWnd(pkMainUI).SetCustomDataAsPacket(lwPacket(&kPacket));
		XUI::CXUI_Wnd* pkBorder = pkMainUI->GetControl(L"FRM_BORDER");
		if( pkBorder )
		{
			XUI::CXUI_Wnd* pkSlideUI = pkBorder->GetControl(L"FMA_FIRST_ACHIEVEMENT");
			if( pkSlideUI )
			{
				pkSlideUI->Text(kInfo.kText);
				pkSlideUI->VOnCall();
				return;
			}
		}
	}
	pkMainUI->Close();
}

void lwAchieveNfyMgr::lwUpdateSlideTextViewUI(lwUIWnd kParent)
{
	XUI::CXUI_Wnd* pkMainUI = kParent.GetSelf();
	if( !pkMainUI )
	{
		return;
	}
	SetSlideTextViewUI(pkMainUI);
}

void lwAchieveNfyMgr::lwOnClickAddonButton(lwUIWnd kParent)
{
	XUI::CXUI_Wnd* pkMainUI = kParent.GetSelf();
	if( !pkMainUI )
	{
		return;
	}

	lwPacket kPacket = lwUIWnd(pkMainUI).GetCustomDataAsPacket();
	int const TextType = kPacket.PopInt();

	switch( TextType )
	{
	case PgSlideTextMgr::EATT_ACHIEVE_FIRST_NOTIFY:
		{
			std::wstring kUserName = kPacket.PopWString().GetWString();

			XUI::CXUI_Wnd* pMsgBox = XUIMgr.Get(L"FRM_ACHIEVE_GREET_MSG");
			if( !pMsgBox || pMsgBox->IsClosed() )
			{
				pMsgBox = XUIMgr.Call(L"FRM_ACHIEVE_GREET_MSG");
				if( !pMsgBox )
				{
					return;
				}
			}

			lwUIWnd(pMsgBox).SetCustomDataAsStr(MB(kUserName));
			std::wstring kText = TTW(2804) + kUserName;
			pMsgBox->Text(kText);
			
			XUI::CXUI_Edit* pkEdit = dynamic_cast<XUI::CXUI_Edit*>(pMsgBox->GetControl(L"EDT_GREET_MSG"));
			if( pkEdit )
			{
				pkEdit->EditText(L"");
				XUIMgr.ClearEditFocus();
				pkEdit->SetEditFocus(true);
			}
		}break;
	case PgSlideTextMgr::EATT_ITEM_RARE_NOTIFY:
		{
		}break;
	default:
		{
		}break;
	}
}

void lwAchieveNfyMgr::CallFirstAchieveUserNfy(int const iAchievementNo, std::wstring const& kUserName)
{
	// 메달 정보를 얻어 온다
	int const iAchieveTypeNo = g_kAchieveNfyMgr.IdxToType(iAchievementNo);

	TBL_DEF_ACHIEVEMENTS	kAchievementInfo;
	if( !GetAchievementInfo(iAchieveTypeNo, iAchievementNo, kAchievementInfo) )
	{
		// 얻는데 실패했다
		return;
	}

	std::wstring kAchieveStr = GetAchievementString(iAchievementNo, kAchievementInfo.iTitleNo);

	std::wstring iAchievementName;
	BM::vstring kString(TTW(2802));
	kString.Replace(L"#NAME#", kUserName);
	kString.Replace(L"#ACHIEVE#", kAchieveStr);
	BM::Stream kPacket;
	kPacket.Push( kUserName );

	if( g_pkWorld )
	{//특정 지역과 캐시샵안이면 막는다.
		int const NoViewerFlag = GATTR_FLAG_MYHOME|GATTR_FLAG_PVP|GATTR_FLAG_BATTLESQUARE|GATTR_FLAG_HARDCORE_DUNGEON|GATTR_FLAG_BOSS|GATTR_FLAG_EMPORIABATTLE|GATTR_FLAG_EMPORIA;
		if( g_pkWorld->IsHaveAttr(NoViewerFlag) || ECS_COMMON != g_kChatMgrClient.ChatStation() || NULL != XUIMgr.Get(L"FRM_PVP_LOBBY") )
		{
			return;
		}
	}

	AddNewSlideText(PgSlideTextMgr::EATT_ACHIEVE_FIRST_NOTIFY, (std::wstring)kString, kPacket);
}

void lwAchieveNfyMgr::CallGenericNfy(std::wstring const& Content, CONT_GENERIC_NOTICE const& ContNotice)
{
	BM::vstring vStr(Content);

	std::wstring UserName;

	CONT_GENERIC_NOTICE::const_iterator iter = ContNotice.begin();
	while( ContNotice.end() != iter )
	{
		if( L"#NAME#" == iter->first )
		{
			UserName = iter->second;
		}
		vStr.Replace(iter->first, iter->second);
		++iter;
	}

	BM::Stream kPacket;
	kPacket.Push( UserName );

	if( g_pkWorld )
	{//특정 지역과 캐시샵안이면 막는다.
		int const NoViewerFlag = GATTR_FLAG_MYHOME|GATTR_FLAG_PVP|GATTR_FLAG_BATTLESQUARE|GATTR_FLAG_HARDCORE_DUNGEON|GATTR_FLAG_BOSS|GATTR_FLAG_EMPORIABATTLE|GATTR_FLAG_EMPORIA;
		if( g_pkWorld->IsHaveAttr(NoViewerFlag) || ECS_COMMON != g_kChatMgrClient.ChatStation() || NULL != XUIMgr.Get(L"FRM_PVP_LOBBY") )
		{
			return;
		}
	}

	AddNewSlideText(PgSlideTextMgr::EATT_ACHIEVE_FIRST_NOTIFY, (std::wstring)vStr, kPacket);
}

void lwAchieveNfyMgr::lwSendAchieveGreetMessage(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	std::wstring kUserName;
	if( pSelf->GetCustomDataSize() )
	{
		kUserName = kSelf.GetCustomDataAsStr().GetWString();
	}
	else
	{
		return;
	}

	std::wstring kGreetMsg;
	XUI::CXUI_Edit* pkEdit = dynamic_cast<XUI::CXUI_Edit*>(pSelf->GetControl(L"EDT_GREET_MSG"));
	if( pkEdit )
	{
		kGreetMsg = pkEdit->Text();
		if( g_kClientFS.Filter(kGreetMsg, false, FST_ALL) 
		|| !g_kUnicodeFilter.IsCorrect(UFFC_CHAT, kGreetMsg) )
		{
			int const iTTWBadWord = 10014;
			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddMessage(iTTWBadWord, kChatLog, true);
			return;
		}
	}

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(FONT_CHAT);
	if( pFont )
	{
		XUI::CXUI_Style_String	kText(XUI::PgFontDef(pFont, PgChatMgrUtil::ChatInputColor(CT_WHISPER_BYNAME)), kGreetMsg);
		g_kChatMgrClient.SendChat_Message(CT_WHISPER_BYNAME, kText, kUserName);
	}

	pSelf->Close();
}

void lwAchieveNfyMgr::AddAchievementNfyMsg(DWORD const dwAchievementNum)
{
	if( (AT_ACHIEVEMENT_DATA_MIN > dwAchievementNum) || (dwAchievementNum > AT_ACHIEVEMENT_DATA_MAX) )
	{
		return;
	}

	// 메달 정보를 얻어 온다
	TBL_DEF_ACHIEVEMENTS	kAchievementInfo;
	if( !GetAchievementInfo(dwAchievementNum, kAchievementInfo) )
	{
		// 얻는데 실패했다
		return;
	}

	XUI::CXUI_List* pAchievementWnd = dynamic_cast<XUI::CXUI_List*>(XUIMgr.Activate(_T("LST_ACHIEVEMENT")));
	if ( !pAchievementWnd )
	{
		return;
	}

	// 메달제목을 세팅
	XUI::SListItem*	pkItem = pAchievementWnd->AddItem(_T(""));
	if( pkItem && pkItem->m_pWnd )
	{
		XUI::CXUI_Wnd* pkTemp = pkItem->m_pWnd->GetControl(_T("FRM_ALERT"));
		if( pkTemp )
		{
			int iCount = 0;
			pkTemp->SetCustomData(&iCount, sizeof(iCount));
		}
		pkTemp = pkItem->m_pWnd->GetControl(_T("FRM_TEXT"));
		if( pkTemp )
		{
			float fTime = g_pkApp->GetAccumTime();
			pkTemp->SetCustomData(&fTime, sizeof(fTime));
			pkTemp->Text(GetAchievementString(kAchievementInfo.iIdx, kAchievementInfo.iTitleNo));
		}
		pkTemp = pkItem->m_pWnd->GetControl(_T("FRM_INTERSEPT"));
		if( pkTemp )
		{
			pkTemp->SetCustomData(&dwAchievementNum, sizeof(dwAchievementNum));
		}
	}
}

void lwAchieveNfyMgr::ShowAchieve(DWORD const iSaveIdx)
{
	if( lwCheckReservedClassPromotionAction() )
	{// 전직 연출 중이면 업적 획득 pass 하고
		return;
	}

	// 메달 정보를 얻어 온다
	DWORD AchieveType = 0;
	DWORD AchieveIdx = 0;
	if( !g_kAchieveNfyMgr.SIdxToIdxAndType(iSaveIdx, AchieveType, AchieveIdx) )
	{
		return;
	}

	TBL_DEF_ACHIEVEMENTS	kAchievementInfo;
	if( !GetAchievementInfo(AchieveType, AchieveIdx, kAchievementInfo) )
	{
		// 얻는데 실패했다
		return;
	}

	XUI::CXUI_Wnd* pAchieve = XUIMgr.Call(_T("FRM_ACHIEVEMENT_POPUP"));
	if( !pAchieve )
	{
		return;
	}

	XUI::CXUI_Icon* pMedalIcon = dynamic_cast<XUI::CXUI_Icon*>(pAchieve->GetControl(_T("ICON_MEDAL")));
	XUI::CXUI_Wnd* pMedalText = pAchieve->GetControl(_T("FRM_TEXT"));
	if( !pMedalIcon || !pMedalText )
	{
		return;
	}

	//메달 정보를 세팅한다
	pMedalIcon->SetCustomData(&kAchievementInfo.iItemNo, sizeof(kAchievementInfo.iItemNo));
	std::wstring wstrText = GetAchievementString(kAchievementInfo.iIdx, kAchievementInfo.iTitleNo);
	wstrText += L"{T=FTcts_13/C=0xFFFFE9AF/}";

	wchar_t const* pDescript = NULL;
	if( GetDefString( kAchievementInfo.iDiscriptionNo, pDescript ) )
	{
		wstrText += L"\n";
		wstrText += pDescript;
	}
	pMedalText->Text(wstrText);
	XUI::CXUI_Style_String	kStyle = pMedalText->StyleText();
	POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStyle));
	POINT2 const kTextPos(pMedalText->TextPos().x, 32 - (kTextSize.y / 2));
	pMedalText->TextPos(kTextPos);
	pMedalText->Text(wstrText);

	std::string kSceneID;
	bool const bFindScendID = g_kRenderMan.GetFirstTypeID<PgWorld>(kSceneID);
	if( bFindScendID )
	{
		char const * szEventName = "CloseAchieve";
		STimerEvent kEvent;
		kEvent.Set(ACH_NM_CLOSE_INTERVAL_TIME, "Close_FRM_ACHIEVEMENT_POPUP()");
		g_kEventTimer.AddLocal(szEventName, kEvent);
	}
}

void lwAchieveNfyMgr::lwOpenAchievementInfo(lwUIWnd kWnd)
{
	if( kWnd.IsNil() ){ return; }

	DWORD const dwAchievementNum = static_cast<DWORD>(kWnd.GetCustomData<int>());

	kWnd.Close();

	// 메달 정보를 얻어 온다
	TBL_DEF_ACHIEVEMENTS	kAchievementInfo;
	if( !GetAchievementInfo(dwAchievementNum, kAchievementInfo, false) )
	{
		// 얻는데 실패했다
		return;
	}

	//메달 메인
	XUI::CXUI_Wnd* pCallWnd = XUIMgr.Call(_T("FRM_ACHIEVEMENT_INFO"));
	if( !pCallWnd )
	{
		return;
	}

	//메달 아이콘/메달 텍스트
	XUI::CXUI_Icon* pMedalIcon = dynamic_cast<XUI::CXUI_Icon*>(pCallWnd->GetControl(_T("ICON_MEDAL")));
	XUI::CXUI_Wnd* pMedalText = pCallWnd->GetControl(_T("FRM_TEXT"));
	if( !pMedalIcon || !pMedalText )
	{
		return;
	}
	
	// 메달 정보를 세팅
	const wchar_t *pTitle = NULL;
	std::wstring kTitle;

	pCallWnd->Text(GetAchievementString(kAchievementInfo.iIdx, kAchievementInfo.iTitleNo));
	pMedalIcon->SetCustomData(&kAchievementInfo.iItemNo, sizeof(kAchievementInfo.iItemNo));
	pMedalText->Text(GetAchievementString(kAchievementInfo.iIdx, kAchievementInfo.iDiscriptionNo));

	std::string kSceneID;
	bool const bFindScendID = g_kRenderMan.GetFirstTypeID<PgWorld>(kSceneID);
	if( bFindScendID )
	{
		char const * szEventName = "CloseAchievementInfo";
		STimerEvent kEvent;
		kEvent.Set(ACH_NM_CLOSE_INTERVAL_TIME, "CloseUI(\"FRM_ACHIEVEMENT_INFO\")");
		g_kEventTimer.AddLocal(szEventName, kEvent);
	}
}

std::wstring const lwAchieveNfyMgr::GetAchievementString(int const iIndex, int const iTitleNo)
{
	const wchar_t *pTitle = NULL;
	if( 0 < iIndex && GetDefString(iTitleNo, pTitle) )
	{
		return std::wstring(pTitle);
	}
	return std::wstring();
}

bool lwAchieveNfyMgr::GetAchievementInfo(DWORD const dwAchievementNum, TBL_DEF_ACHIEVEMENTS& kAchievementInfo, bool const bIsInfoCall)
{
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkMyPlayer) {return false;}

	CONT_DEF_CONT_ACHIEVEMENTS const * pContDefAch = NULL;
	g_kTblDataMgr.GetContDef(pContDefAch);
	if( !pContDefAch || !pContDefAch->size() ){ return false; }

	CONT_DEF_CONT_ACHIEVEMENTS::const_iterator ach_it = pContDefAch->find(dwAchievementNum);
	if(pContDefAch->end() != ach_it)
	{
		if( GetNearAchievementInfo(pkMyPlayer->GetAbil(static_cast<WORD>(dwAchievementNum)), ach_it->second, kAchievementInfo, bIsInfoCall) )
		{
			return true;
		}
	}
	return false;
}

bool lwAchieveNfyMgr::GetAchievementInfo(DWORD const dwAchieveType, DWORD const dwAchieveIdx, TBL_DEF_ACHIEVEMENTS& AchievementInfo)
{
	CONT_DEF_CONT_ACHIEVEMENTS const * pContDefAch = NULL;
	g_kTblDataMgr.GetContDef(pContDefAch);
	if( !pContDefAch || !pContDefAch->size() ){ return false; }

	CONT_DEF_CONT_ACHIEVEMENTS::const_iterator ach_it = pContDefAch->find(dwAchieveType);
	if(pContDefAch->end() != ach_it)
	{
		CONT_DEF_ACHIEVEMENTS::const_iterator sub_ach_it = ach_it->second.begin();
		while( ach_it->second.end() != sub_ach_it )
		{
			if( sub_ach_it->second.iIdx == dwAchieveIdx )
			{
				AchievementInfo = sub_ach_it->second;
				return true;
			}
			++sub_ach_it;
		}
	}
	return false;
}

bool lwAchieveNfyMgr::GetNearAchievementInfo(int const iAchievementCount, CONT_DEF_ACHIEVEMENTS const& AchievementCont, TBL_DEF_ACHIEVEMENTS& kAchievementInfo, bool const bIsInfoCall)
{
	if( !AchievementCont.size() )
	{
		return false;
	}

	if( iAchievementCount <= 0 )
	{
		return false;
	}
	
	CONT_DEF_ACHIEVEMENTS::const_iterator	ach_it = AchievementCont.begin();
	while( ach_it != AchievementCont.end() )
	{
		if( bIsInfoCall )
		{
			if( (ach_it->first + 1) == iAchievementCount )
			{
				++ach_it;
				if( ach_it != AchievementCont.end() )
				{
					kAchievementInfo = ach_it->second;
					return true;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			int const CalcValue = ach_it->first - iAchievementCount;
			if( CalcValue >= 0 || (AchievementCont.size() == 1) )
			{
				kAchievementInfo = ach_it->second;
				return true;
			}
		}
		++ach_it;
	}
	return false;
}

void lwAchieveNfyMgr::lwClose_FRM_ACHIEVEMENT_POPUP()
{
	if(XUI::CXUI_Wnd* kWnd = XUIMgr.Get(L"FRM_ACHIEVEMENT_POPUP"))
	{
		kWnd->Close();
		g_kAchieveNfyMgr.ContShowAchieve_PopFront();
	}
}

PgAchieveNfyMgr::PgAchieveNfyMgr(void)
	: m_fCheckTime(1800.f)
	, m_fCheckProgress(0.5f)
	, m_iStatusAliveTime(30000)
	, m_iIconAliveTime(60000)
{
}

PgAchieveNfyMgr::~PgAchieveNfyMgr(void)
{
}

void PgAchieveNfyMgr::Update(float fAccumTime)
{
	static float fOldAccumTime = fAccumTime;
	if( IsShowAchieveStatusOK() &&
		((true == lwAchieveNfyMgr::IsAchieveStatusFirstLogin()) ||
		(fAccumTime-fOldAccumTime >= m_fCheckTime)) )
	{
		lwAchieveNfyMgr::lwCheckAchieveStatusNfy();
		fOldAccumTime = fAccumTime;
	}

	if(!m_ContShowAchieve.empty())
	{
		if( IsShowAchieveOK() )
		{
			lwAchieveNfyMgr::ShowAchieve(m_ContShowAchieve[0]);
		}
	}
}

void PgAchieveNfyMgr::ContShowAchieve_PopFront()
{
	if( !m_ContShowAchieve.empty() )
	{
		m_ContShowAchieve.pop_front();
	}
}

void PgAchieveNfyMgr::AddShowAchieve(DWORD const iSaveIdx)
{
	m_ContShowAchieve.push_back(iSaveIdx);
}

void PgAchieveNfyMgr::ClearShowAchievement()
{
	m_ContShowAchieve.clear();
}

bool PgAchieveNfyMgr::ParseXml(char const* pcXmlPath)
{
	TiXmlDocument kXmlDoc(pcXmlPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
	{
		if( kXmlDoc.Error() )
		{
			CAUTION_LOG(BM::LOG_LV1, __FL__ << L"Can't open file: " << pcXmlPath << L"Error Msg["<<kXmlDoc.ErrorDesc()<<L"]");
		}
		else
		{
			CAUTION_LOG(BM::LOG_LV1, __FL__ << L"Can't open file: " << pcXmlPath);
		}

		return false;
	}

	TiXmlElement const* pkElement = kXmlDoc.FirstChildElement();
	while( pkElement )
	{
		char const* pcTagName = pkElement->Value();

		if( strcmp(pcTagName, "ACHIEVEMENT_STATUS") == 0 )
		{
			TiXmlElement const* pkSubElem = pkElement->FirstChildElement();
			
			while( pkSubElem )
			{
				char const* pcTagName = pkSubElem->Value();
				char const* pcValue = pkSubElem->GetText();

				if( strcmp(pcTagName, "CHECK_TIME") == 0 )
				{
					int const iTemp = PgStringUtil::SafeAtoi(pcValue);

					if( 0 < iTemp )
					{
						m_fCheckTime = static_cast<float>(iTemp);
					}
				}
				else if( strcmp(pcTagName, "CHECK_PROGRESS") == 0 )
				{
					int const iTemp = PgStringUtil::SafeAtoi(pcValue);

					if( 0 <= iTemp )
					{
						m_fCheckProgress = iTemp/100.f;
					}
				}
				else if( strcmp(pcTagName, "STATUS_ALIVE_TIME") == 0 )
				{
					int const iTemp = PgStringUtil::SafeAtoi(pcValue);

					if( 0 < iTemp )
					{
						m_iStatusAliveTime = iTemp*1000;
					}
				}
				else if( strcmp(pcTagName, "ICON_ALIVE_TIME") == 0 )
				{
					int const iTemp = PgStringUtil::SafeAtoi(pcValue);

					if( 0 < iTemp )
					{
						m_iIconAliveTime = iTemp*1000;
					}
				}

				pkSubElem = pkSubElem->NextSiblingElement();
			}
		}
		
		pkElement = pkElement->NextSiblingElement();
	}

	return true;
}

bool PgAchieveNfyMgr::OnBuild()
{
	CONT_DEF_CONT_ACHIEVEMENTS const * pContDefAch = NULL;
	g_kTblDataMgr.GetContDef(pContDefAch);
	if( !pContDefAch || !pContDefAch->size() ){ return false; }

	CONT_DEF_CONT_ACHIEVEMENTS::const_iterator ach_it = pContDefAch->begin();
	while( ach_it != pContDefAch->end() )
	{
		CONT_DEF_ACHIEVEMENTS::const_iterator	sub_ach_it = ach_it->second.begin();
		while( sub_ach_it != ach_it->second.end() )
		{
			std::pair<CONT_CACHE_IDX_TO_TYPE::iterator, bool> TypeRst = m_kReverseCacheIdxToType.insert(std::make_pair(sub_ach_it->second.iIdx, ach_it->first));
			if( !TypeRst.second )
			{

			}
			std::pair<CONT_CACHE_SIDX_TO_IDX::iterator, bool> IdxRst = m_kReverseCacheSIdxToIdx.insert(std::make_pair(sub_ach_it->second.iSaveIdx, sub_ach_it->second.iIdx));
			if( !IdxRst.second )
			{

			}
			++sub_ach_it;
		}
		++ach_it;
	}
	return true;
}

int PgAchieveNfyMgr::IdxToType(DWORD const Idx)
{
	CONT_CACHE_IDX_TO_TYPE::iterator iter = m_kReverseCacheIdxToType.find(Idx);
	if( iter != m_kReverseCacheIdxToType.end() )
	{
		return iter->second;
	}
	return 0;
}

int PgAchieveNfyMgr::SIdxToIdx(DWORD const SIdx)
{
	CONT_CACHE_SIDX_TO_IDX::iterator iter = m_kReverseCacheSIdxToIdx.find(SIdx);
	if( iter != m_kReverseCacheSIdxToIdx.end() )
	{
		return iter->second;
	}
	return 0;
}

int PgAchieveNfyMgr::SIdxToType(DWORD const SIdx)
{
	CONT_CACHE_SIDX_TO_IDX::iterator iter = m_kReverseCacheSIdxToIdx.find(SIdx);
	if( iter != m_kReverseCacheSIdxToIdx.end() )
	{
		CONT_CACHE_IDX_TO_TYPE::iterator sub_iter = m_kReverseCacheIdxToType.find(iter->second);
		if( sub_iter != m_kReverseCacheIdxToType.end() )
		{
			return sub_iter->second;
		}
	}
	return 0;
}

bool PgAchieveNfyMgr::SIdxToIdxAndType(DWORD const SIdx, DWORD& Type, DWORD& Idx)
{
	CONT_CACHE_SIDX_TO_IDX::iterator iter = m_kReverseCacheSIdxToIdx.find(SIdx);
	if( iter != m_kReverseCacheSIdxToIdx.end() )
	{
		CONT_CACHE_IDX_TO_TYPE::iterator sub_iter = m_kReverseCacheIdxToType.find(iter->second);
		if( sub_iter != m_kReverseCacheIdxToType.end() )
		{
			Idx = iter->second;
			Type = sub_iter->second;
			return true;
		}
	}
	return false;
}





class PgAchieveValueCtrl_LongTimeNoSee
{
	PgAchieveValueCtrl_LongTimeNoSee(){}
	~PgAchieveValueCtrl_LongTimeNoSee(){}
public:
	static int GetValue(WORD const,CUnit const * pkUnit)
	{
		int const iLogOutTime = pkUnit->GetAbil(AT_ACHIEVEMENT_LONGTIMENOSEE);
		if(0 != iLogOutTime)
		{
			BM::PgPackedTime kLogoutTime;
			kLogoutTime.SetTime(iLogOutTime);

			__int64 i64LogoutTime = 0;
			CGameTime::DBTimeEx2SecTime(static_cast<BM::DBTIMESTAMP_EX>(kLogoutTime),i64LogoutTime);
			__int64 const i64CurTime = g_kEventView.GetLocalSecTime();

			return static_cast<int>((i64CurTime - i64LogoutTime)/(24*60*60));
		}
		return 0;
	}
};

class PgAchieveValueCtrl_Age2Minute
{
	PgAchieveValueCtrl_Age2Minute(){}
	~PgAchieveValueCtrl_Age2Minute(){}
public:
	static int GetValue(WORD const,CUnit const * pkUnit)
	{
		PgPlayer const * pkPlayer = dynamic_cast<PgPlayer const *>(pkUnit);
		if(!pkPlayer)
		{
			return 0;
		}

		__int64 i64SecTime = 0;
		CGameTime::DBTimeEx2SecTime(static_cast<BM::DBTIMESTAMP_EX>(pkPlayer->BirthDate()), i64SecTime);

		__int64 const i64CurSecTime = g_kEventView.GetLocalSecTime();

		return static_cast<int>((i64CurSecTime - i64SecTime)/60i64);// 분단위 체크
	}
};

class PgAchieveValueCtrl_CoupleTime
{
	PgAchieveValueCtrl_CoupleTime(){}
	~PgAchieveValueCtrl_CoupleTime(){}
public:
	static int GetValue(WORD const,CUnit const * pkUnit)
	{
		int const iStartTime = pkUnit->GetAbil(AT_ACHIEVEMENT_COUPLE_START_DATE);
		if(0 != iStartTime)
		{
			BM::PgPackedTime kStartTime;
			kStartTime.SetTime(iStartTime);

			__int64 i64CoupleTime = 0;
			CGameTime::DBTimeEx2SecTime(static_cast<BM::DBTIMESTAMP_EX>(kStartTime),i64CoupleTime);
			__int64 const i64CurTime = g_kEventView.GetLocalSecTime();

			return static_cast<int>((i64CurTime - i64CoupleTime)/(24*60*60));
		}
		return 0;
	}
};

class PgAchieveValueCtrl_TexTime
{
	PgAchieveValueCtrl_TexTime(){}
	~PgAchieveValueCtrl_TexTime(){}
public:
	static int GetValue(WORD const,CUnit const * pkUnit)
	{
		return static_cast<int>(pkUnit->GetAbil(AT_ACHIEVEMENT_TEXTIME)/(24*60*60));
	}
};

int PgAchieveValueCtrl::Max()const
{
	switch(m_iType)
	{
	case AT_ACHIEVEMENT_DUNGEON_ANYONE_DIE:
	case AT_ACHIEVEMENT_DUNGEON_TIMEATTACK:
	case AT_ACHIEVEMENT_EMPORIA_VICTORY:
	case AT_ACHIEVEMENT_LONGTIMENOSEE:
	case AT_ACHIEVEMENT_ITEM:
		{
			return 1;
		}break;
	case AT_ACHIEVEMENT_CHEKCPLAYTIME:
		{
			return m_iValue / 1000;
		}break;
	case AT_ACHIEVEMENT_RELATION_CLAER:
		{
			CONT_DEF_ACHIEVEMENTS const * pContDefAch = NULL;
			g_kTblDataMgr.GetContDef(pContDefAch);	//업적 데프
			if(!pContDefAch){ return 0; }
			
			int iCount = 0;
			CONT_DEF_ACHIEVEMENTS::const_iterator ach_it = pContDefAch->begin();
			while(pContDefAch->end() != ach_it)
			{
				if( m_iSaveIdx != ach_it->second.iSaveIdx && m_iGroupNo == ach_it->second.iGroupNo)
				{
					++iCount;
				}
				++ach_it;
			};
			return iCount;
		}break;
	}
	return std::max<int>(m_iValue,1);
}

int PgAchieveValueCtrl::Now()const
{
	if(m_bComplete)
	{
		return Max();
	}

	int iNow = 0;
	switch(m_iType)
	{
	case AT_ACHIEVEMENT_LEVEL:
		{
			iNow = m_rkPlayer.GetAbil(AT_LEVEL);
		}break;
	case AT_ACHIEVEMENT_DUNGEON_ANYONE_DIE:
	case AT_ACHIEVEMENT_DUNGEON_TIMEATTACK:
	case AT_ACHIEVEMENT_LONGTIMENOSEE:
		{
			iNow = 0;
		}break;
	case AT_ACHIEVEMENT_CRAZYLEVELUP:
		{
			float const fNow = PgAchieveValueCtrl_Age2Minute::GetValue(m_iType, &m_rkPlayer);
			float const fRate = fNow / Max();
			if(fRate>1.f && !m_bComplete)
			{
				return 0;
			}
			iNow = static_cast<int>(fNow);
		}break;
	case AT_ACHIEVEMENT_COUPLE_START_DATE:
		{
			iNow = PgAchieveValueCtrl_CoupleTime::GetValue(m_iType, &m_rkPlayer);
		}break;
	case AT_ACHIEVEMENT_TEXTIME:
		{
			iNow = PgAchieveValueCtrl_TexTime::GetValue(m_iType, &m_rkPlayer);
		}break;
	case AT_ACHIEVEMENT_CHEKCPLAYTIME:
		{
			iNow = m_rkPlayer.GetAbil(m_iType);
			iNow /= 1000;
		}break;
	case AT_ACHIEVEMENT_RELATION_CLAER:
		{
			PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
			if(!pkMyPlayer){ return 0; }
			PgAchievements * pkAchiv = pkMyPlayer->GetAchievements();
			if(!pkAchiv){ return 0; }

			CONT_DEF_ACHIEVEMENTS const * pContDefAch = NULL;
			g_kTblDataMgr.GetContDef(pContDefAch);	//업적 데프
			if(!pContDefAch){ return 0; }
			
			CONT_DEF_ACHIEVEMENTS::const_iterator ach_it = pContDefAch->begin();
			while(pContDefAch->end() != ach_it)
			{
				if( m_iSaveIdx != ach_it->second.iSaveIdx && m_iGroupNo == ach_it->second.iGroupNo)
				{
					if(pkAchiv->IsComplete(ach_it->second.iSaveIdx))
					{
						++iNow;
					}
				}
				++ach_it;
			};
		}break;
	default:
		{
			iNow = m_rkPlayer.GetAbil(m_iType);
		}break;
	}

	return std::min<int>(iNow,Max());
}

int PgAchieveValueCtrl::Rate()const
{
	return static_cast<int>( Now()/static_cast<float>(Max())*100 );
}

PgAchieveValueCtrl::PgAchieveValueCtrl(int const iSaveIdx, PgPlayer & pkPlayer)
	: m_iSaveIdx(iSaveIdx), m_rkPlayer(pkPlayer), m_iType(0), m_iValue(0), m_bComplete(false)
{
	CONT_DEF_ACHIEVEMENTS_SAVEIDX const * pContDefAch = NULL;
	g_kTblDataMgr.GetContDef(pContDefAch);

	if(pContDefAch)
	{
		CONT_DEF_ACHIEVEMENTS_SAVEIDX::const_iterator c_iter = pContDefAch->find(m_iSaveIdx);
		if(c_iter != pContDefAch->end())
		{
			m_iValue = c_iter->second.iValue;
			m_iType = c_iter->second.iType;
			m_iGroupNo = c_iter->second.iGroupNo;
			m_bComplete = m_rkPlayer.GetAchievements() ? m_rkPlayer.GetAchievements()->IsComplete( m_iSaveIdx ) : false;
		}
	}
}
