#include "stdafx.h"
#include "PgConstellation.h"
#include "PgClientParty.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgWorld.h"
#include "PgQuestMan.h"
#include "lwUIQuest.h"
#include "PgChatMgrClient.h"
#include "PgMissionComplete.h"
#include "PgUISound.h"

#include "Variant/PgSimpleTime.h"
#include "Variant/PgEventView.h"
#include "Variant/Inventory.h"
#include "Variant/Global.h"

#include "XUI/XUI_Manager.h"
#include "PgPilot.h"

extern int GetNowGroundNo();
extern bool IsDefenceMode7();

// lwConstellationEnterUIUtil
namespace lwConstellationEnterUIUtil
{
	typedef enum eWNDINIT_TYPE
	{
		EWIT_ALL			= 0,
		EWIT_NOT_OPTION		= 0x01,
	}E_WNDINIT_TYPE;

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "ReqEnterConstellation", lwReqEnterConstellation);
		def(pkState, "ReqEnterConstellation_Boss", lwReqEnterConstellation_Boss);
		def(pkState, "IsInDay_Constellation", lwIsInDay);
		def(pkState, "ConstellationEnable", lwEnable);
		def(pkState, "UpdateConstellationInfo", lwUpdateConstellationInfo);
		def(pkState, "ReqConstellationPartyList", lwReqConstellationPartyList);
		def(pkState, "FakeUpdateConstellationPartyList", lwFakeUpdateConstellationPartyList);
		def(pkState, "OnEnter_ConstellationParty", lwOnEnter_ConstellationParty);
		def(pkState, "InitConstellationCashItem", lwInitCashItem);
		def(pkState, "Init_ConstellationUI", lwInitConstellationUI);
		def(pkState, "CheckCondition_CashItem", lwCheckCondition_CashItem);
		def(pkState, "UseConstellationCashItem", lwUseConstellationCashItem);
		def(pkState, "GetMinConstellationDungeon", lwGetMinConstellationDungeon);
		def(pkState, "GetMaxConstellationDungeon", lwGetMaxConstellationDungeon);
		def(pkState, "ReqConstellationMission", lwReqConstellationMission);
		def(pkState, "UpdateResult", lwConstellationUpdate);
		def(pkState, "RequestAddReward", lwReqAddReward);
		def(pkState, "ConstellationResultClose", lwConstellationResultClose);
		def(pkState, "DifficultyName",lwDifficultyName);
		def(pkState, "DifficultyStory",lwDifficultyStory);
		def(pkState, "GetConstellationPartyListToolTip",lwGetConstellationPartyListToolTip);
		def(pkState, "GetDifficultyLegend",lwDifficultyLegend);
	}
	bool lwDifficultyLegend(int const Position)
	{
		int const GroundNo = GetNowGroundNo();
		int const MaxDifficultNameCount = 4;
		CONT_CONSTELLATION::const_iterator iter = g_kConstellationEnterUIMgr.m_ContEnter.find(GroundNo);
		if(g_kConstellationEnterUIMgr.m_ContEnter.end() == iter )
		{
			return false;
		}

		ConstellationKey KeyLegend(Position, MaxDifficultNameCount);
		CONT_CONSTELLATION_ENTER::const_iterator sub_iter_Legend = iter->second.find(KeyLegend);
		if( iter->second.end() == sub_iter_Legend )
		{
			return false;
		}
		return true;
		
	}
	void lwDifficultyStory(int const Position, int Difficulty)
	{
		int const GroundNo = GetNowGroundNo();
		int const MaxDifficultNameCount = 4;
		CONT_CONSTELLATION::const_iterator iter = g_kConstellationEnterUIMgr.m_ContEnter.find(GroundNo);
		if(g_kConstellationEnterUIMgr.m_ContEnter.end() == iter )
		{
			return;
		}

		ConstellationKey Key(Position, Difficulty);
		CONT_CONSTELLATION_ENTER::const_iterator sub_iter = iter->second.find(Key);
		if( iter->second.end() == sub_iter )
		{
			return;
		}


		XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"SFRM_CONSTELLATION_STORY");
		if(NULL == pWnd)
		{
			return;
		}
		
		g_kConstellationEnterUIMgr.UpdateConstellationInfo_Story( pWnd, Difficulty, sub_iter->second.Description );

	}
	void lwDifficultyName(int const Position, int Difficulty)
	{
		int const GroundNo = GetNowGroundNo();
		int const MaxDifficultNameCount = 4;
		CONT_CONSTELLATION::const_iterator iter = g_kConstellationEnterUIMgr.m_ContEnter.find(GroundNo);
		if(g_kConstellationEnterUIMgr.m_ContEnter.end() == iter )
		{
			return;
		}

		ConstellationKey Key(Position, Difficulty);
		CONT_CONSTELLATION_ENTER::const_iterator sub_iter = iter->second.find(Key);
		if( iter->second.end() == sub_iter )
		{
			return;
		}

		ConstellationKey KeyLegend(Position, MaxDifficultNameCount);
		CONT_CONSTELLATION_ENTER::const_iterator sub_iter_Legend = iter->second.find(KeyLegend);
		XUI::CXUI_Wnd * pWnd;
		if( iter->second.end() == sub_iter_Legend )
		{
			pWnd = XUIMgr.Get(L"FRM_BTNSET_DIFFICULTY");
		}
		else
		{
			pWnd = XUIMgr.Get(L"FRM_BTNSET_DIFFICULTY_LEGEND");
		}

		if(NULL == pWnd)
		{
			return;
		}
		XUI::CXUI_Wnd * pDiffcultyName = pWnd->GetControl(L"FRM_DUNGEON_NAME_DIFFICULTY");
		if( NULL != pDiffcultyName )
		{
			g_kConstellationEnterUIMgr.UpdateConstellationInfo_Name( pDiffcultyName, Difficulty, sub_iter->second.Name );
		}

		XUI::CXUI_Wnd * pLevel = pWnd->GetControl(L"FRM_LEVEL");
		if(NULL != pLevel )
		{// 입장 최소, 최대 레벨
			g_kConstellationEnterUIMgr.UpdateConstellationInfo_Level( pLevel, sub_iter->second.Condition.MinLevel, sub_iter->second.Condition.MaxLevel );
		}

		int DifficultyName = 1;
		for(int i=0;i<MaxDifficultNameCount;++i)
		{
			BM::vstring vStrLevelCount(L"FRM_LEVEL");
			vStrLevelCount += i;
			XUI::CXUI_Wnd* vStrLevel = pWnd->GetControl(vStrLevelCount);
			
			CONT_CONSTELLATION::const_iterator iter = g_kConstellationEnterUIMgr.m_ContEnter.find(GroundNo);
			if(g_kConstellationEnterUIMgr.m_ContEnter.end() == iter )
			{
				return;
			}

			if(1<=i)
			{
				DifficultyName +=1;
			}
				
			ConstellationKey Key(Position, DifficultyName);
			CONT_CONSTELLATION_ENTER::const_iterator sub_iter = iter->second.find(Key);
			if( iter->second.end() == sub_iter )
			{
				return;
			}
		
			if(vStrLevel != NULL)
			{
				g_kConstellationEnterUIMgr.UpdateConstellationInfo_Level( vStrLevel, sub_iter->second.Condition.MinLevel, sub_iter->second.Condition.MaxLevel );
			}
		}

	}

	void lwConstellationUpdate()
	{
		g_kConstellationEnterUIMgr.ConstellationResultUpdate();
	}

	void ResultWndInit(E_WNDINIT_TYPE const eType=EWIT_ALL)
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"FRM_CONSTELLATION_RESULT");
		if( !pkWnd ){ return; }

		bool const bVisible = false;
		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SLOT"));
		if( pBuild )
		{ 
			int const iMaxSlot = pBuild->CountX() * pBuild->CountY();
			for(int i=0; i<iMaxSlot; ++i)
			{
				BM::vstring vStr(L"FRM_C_RESULT_SLOT");
				vStr += i;

				XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
				if( pSlot )
				{
					XUI::CXUI_Wnd* pTmp = NULL;
					//pTmp = pSlot->GetControl(L"IMG_SELECT");
					//pTmp ? pTmp->Visible(bVisible) : 0;

					pTmp = pSlot->GetControl(L"IMG_SELECT_ANI");
					pTmp ? pTmp->Visible(bVisible) : 0;
				}
			}
		}

		pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SECLECT_ICON"));
		if(pBuild)
		{
			int const iMax = pBuild->CountX() * pBuild->CountY();
			for(int i=0; i < iMax; i++)
			{
				BM::vstring vStr(L"ICON_SECLECT_ITEM");
				vStr += i;
				XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(vStr));
				if( pIcon )
				{
					pIcon->ClearCustomData();
					SIconInfo kIconInfo = pIcon->IconInfo();
					kIconInfo.iIconCount = 0;
					pIcon->SetIconInfo(kIconInfo);
					pIcon->GrayScale(false);
				}
			}
		}
		pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_MY_ICON"));
		if(pBuild)
		{
			int const iMax = pBuild->CountX() * pBuild->CountY();
			for(int i=0; i < iMax; i++)
			{
				BM::vstring vStr(L"ICON_MY_ITEM");
				vStr += i;
				XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(vStr));
				if( pIcon )
				{
					pIcon->ClearCustomData();
					SIconInfo kIconInfo = pIcon->IconInfo();
					kIconInfo.iIconCount = 0;
					pIcon->SetIconInfo(kIconInfo);
				}
			}
		}
		pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_PARTY_ICON1"));
		if(pBuild)
		{
			int const iMax = pBuild->CountX() * pBuild->CountY();
			for(int i=0; i < iMax; i++)
			{
				BM::vstring vStr(L"ICON_ITEM_PARTY_MEMBER0");
				vStr += i;
				XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(vStr));
				if( pIcon )
				{
					pIcon->ClearCustomData();
					SIconInfo kIconInfo = pIcon->IconInfo();
					kIconInfo.iIconCount = 0;
					pIcon->SetIconInfo(kIconInfo);
				}
			}
		}
		pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_PARTY_ICON2"));
		if(pBuild)
		{
			int const iMax = pBuild->CountX() * pBuild->CountY();
			for(int i=0; i < iMax; i++)
			{
				BM::vstring vStr(L"ICON_ITEM_PARTY_MEMBER1");
				vStr += i;
				XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(vStr));
				if( pIcon )
				{
					pIcon->ClearCustomData();
					SIconInfo kIconInfo = pIcon->IconInfo();
					kIconInfo.iIconCount = 0;
					pIcon->SetIconInfo(kIconInfo);
				}
			}
		}
		pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_PARTY_ICON3"));
		if(pBuild)
		{
			int const iMax = pBuild->CountX() * pBuild->CountY();
			for(int i=0; i < iMax; i++)
			{
				BM::vstring vStr(L"ICON_ITEM_PARTY_MEMBER2");
				vStr += i;
				XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(vStr));
				if( pIcon )
				{
					pIcon->ClearCustomData();
					SIconInfo kIconInfo = pIcon->IconInfo();
					kIconInfo.iIconCount = 0;
					pIcon->SetIconInfo(kIconInfo);
				}
			}
		}
		XUI::CXUI_Wnd* pText = pkWnd->GetControl(_T("FRM_MY_ID"));
		pText->Text(std::wstring());

		pText = pkWnd->GetControl(_T("FRM_MY_PARTY_MEMBER0"));
		pText->Text(std::wstring());

		pText = pkWnd->GetControl(_T("FRM_MY_PARTY_MEMBER1"));
		pText->Text(std::wstring());

		pText = pkWnd->GetControl(_T("FRM_MY_PARTY_MEMBER2"));
		pText->Text(std::wstring());

		XUI::CXUI_Button* pkExitBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(_T("BTN_CANCLE")));
		if(pkExitBtn)
		{
			pkExitBtn->Disable(false);
		}


	}
	void lwConstellationResultClose()
	{
		bool const bLoop = false;
		lwConstellationEnterUIUtil::E_WNDINIT_TYPE const bInitType = bLoop ? lwConstellationEnterUIUtil::EWIT_NOT_OPTION : lwConstellationEnterUIUtil::EWIT_ALL;
		lwConstellationEnterUIUtil::ResultWndInit(bInitType);
	}



	bool ProcessMsg(unsigned short const usType, BM::Stream& Packet)
	{
		switch( usType )
		{
		default:
			{
				return g_kConstellationEnterUIMgr.ProcessMsg(usType, Packet);
			}break;
		} 
		return true;
	}

	void lwReqEnterConstellation(int const Position, int const Difficulty)
	{
		g_kConstellationEnterUIMgr.ReqEnterConstellation(Position, Difficulty);
	}
	
	void lwReqEnterConstellation_Boss()
	{
		g_kConstellationEnterUIMgr.ReqEnterConstellation_Boss();
	}

	bool lwIsInDay(int const Position)
	{
		return g_kConstellationEnterUIMgr.IsInDay(Position);
	}

	bool lwEnable(int const Position, int const Difficulty)
	{
		return g_kConstellationEnterUIMgr.Enable(Position, Difficulty);
	}

	void lwUpdateConstellationInfo(int const Position, int const Difficulty)
	{
		g_kConstellationEnterUIMgr.UpdateConstellationInfo(Position, Difficulty);
	}

	void lwReqConstellationPartyList(lwUIWnd lwWnd)
	{
		XUI::CXUI_Wnd * pWnd = lwWnd();
		if( NULL == pWnd )
		{
			return;
		}

		g_kConstellationEnterUIMgr.ReqConstellationPartyList(pWnd);
	}

	void lwFakeUpdateConstellationPartyList(lwUIWnd lwWnd)
	{
		XUI::CXUI_Wnd * pWnd = lwWnd();
		if( NULL == pWnd )
		{
			return;
		}

		g_kConstellationEnterUIMgr.FakeUpdateConstellationPartyList(pWnd);
	}

	void lwOnEnter_ConstellationParty(lwUIWnd lwWnd)
	{
		XUI::CXUI_Wnd * pWnd = lwWnd();
		if( NULL == pWnd )
		{
			return;
		}

		g_kConstellationEnterUIMgr.OnEnter_ConstellationParty(pWnd);
	}

	void lwInitCashItem()
	{
		g_kConstellationEnterUIMgr.InitCashItem();
	}

	void lwInitConstellationUI(lwUIWnd lwWnd)
	{
		XUI::CXUI_Wnd * pWnd = lwWnd();
		if( NULL == pWnd )
		{
			return;
		}

		g_kConstellationEnterUIMgr.InitConstellationUI(pWnd);
	}

	bool lwCheckCondition_CashItem()
	{
		return g_kConstellationEnterUIMgr.CheckCondition_CashItem();
	}

	bool lwUseConstellationCashItem(lwUIWnd lwWnd, bool const bUse)
	{
		return g_kConstellationEnterUIMgr.UseConstellationCashItem(lwWnd(), bUse);
	}

	int lwGetMinConstellationDungeon()
	{
		return MIN_CONSTELLATION_DUNGEON;
	}

	int lwGetMaxConstellationDungeon()
	{
		return MAX_CONSTELLATION_DUNGEON;
	}

	void lwReqConstellationMission()
	{
		g_kConstellationEnterUIMgr.ReqConstellationMission();
	}

	bool lwReqAddReward(void)
	{
		if(!g_kConstellationEnterUIMgr.EnoughConsumeItem() || g_kConstellationEnterUIMgr.RewardCurrentChoice() >= PgConstellationEnterUIMgr::MAX_CHOICE) { return false; }
		if(PgConstellationEnterUIMgr::ES_BONUS_PROCESS_READY != g_kConstellationEnterUIMgr.GetState()) { return false; }

		NETWORK_SEND(BM::Stream(PT_C_M_REQ_CONSTELLATION_CASH_REWARD));
		return true;
	}

	lwWString lwGetConstellationPartyListToolTip(lwGUID lwGuid)
	{
		BM::GUID guid = lwGuid();
		std::wstring TooltipStr;
		if( g_kConstellationEnterUIMgr.GetConstellationPartyListToolTip(guid, TooltipStr) )
		{
			return lwWString(TooltipStr);
		}
		return lwWString("");
	}
}

using namespace lwConstellationEnterUIUtil;

// PgConstellationEnterUIMgr
PgConstellationEnterUIMgr::PgConstellationEnterUIMgr()
: m_bConstellationFreePass(false),
  m_NeedItemNo(0),
  m_NeedQuestNo(0),
  m_CashItemNo(0)
{
	Clear();
}

PgConstellationEnterUIMgr::~PgConstellationEnterUIMgr()
{
}

void PgConstellationEnterUIMgr::ConstellationClear(bool const bClearState)
{
	if(bClearState)
	{
		m_eState = ES_NONE;
		m_eProcessState = EPS_INIT;

		for(int i = 0; i < MAX_PLAYER; i++)
		{
			memset(&m_kArrRewardInfo[i], 0, sizeof(REWARDINFO));
		}
		m_iRewardPlayer = 0;
		m_iMyRewardIndex = 0;


		CONT_CONSTELLATION_BONUS_ITEM::const_iterator iterBonus = m_ContBonusItem.find(m_Mission.ConstellationKey.WorldGndNo);
		if( iterBonus != m_ContBonusItem.end() )
		{ //UI에 올릴 목적으로 대표 아이템 번호 하나만 저장해놓음
			if(iterBonus->second.VecItemNo.empty())
			{
				m_iNeedRewardConsumeItemNoAtUI = 0;
			}
			else
			{
				m_iNeedRewardConsumeItemNoAtUI = iterBonus->second.VecItemNo.front();
			}
			m_iMaxRewardBonusCount = MAX_CHOICE;
		}
		else
		{
		}

		EnoughConsumeItem(GetPlayerRewardConsumItemCount(m_iNeedRewardConsumeItemNo, m_iNeedRewardConsumeItemCount, m_iHaveRewardConsumeItemCount));
		if( !EnoughConsumeItem() )
		{
			m_iNeedRewardConsumeItemNo = 0;
			m_iNeedRewardConsumeItemCount = 0;
			m_iHaveRewardConsumeItemCount = 0;
		}
	}

}


bool PgConstellationEnterUIMgr::GetPlayerRewardConsumItemCount(int& riOutNeedItemNo, size_t& riOutNeedItemCount, size_t& riOutHaveItemCount)
{
	CONT_CONSTELLATION_BONUS_ITEM::const_iterator iterBonus = m_ContBonusItem.find(m_Mission.ConstellationKey.WorldGndNo);
	if( iterBonus == m_ContBonusItem.end() ) { return false; }

	PgPlayer *pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer ) { return false; }

	PgInventory * pkInv = pPlayer->GetInven();
	if( NULL == pkInv ) { return false; }

	riOutNeedItemNo = 0;
	riOutNeedItemCount = 1;
	riOutHaveItemCount = 1;

	if( 1 < RewardCurrentChoice() )
	{
		// 캐시 아이템 필요갯수 체크
		if(iterBonus->second.VecItemCount.size() < (RewardCurrentChoice() - 1))
		{
			return false;
		}
		else
		{
			try
			{
				riOutNeedItemCount = iterBonus->second.VecItemCount.at(RewardCurrentChoice() - 1);
			}
			catch(const std::out_of_range& exception)
			{
				return false;
			}
		}
	}

	bool bHaveCashItem = false;
	// 필요한 갯수만큼 소지하고 있는 캐시 아이템이 있는지 체크
	VEC_INT::const_reverse_iterator item_iter = iterBonus->second.VecItemNo.rbegin();
	while( iterBonus->second.VecItemNo.rend() != item_iter )
	{
		riOutNeedItemNo = *item_iter;
		riOutHaveItemCount = pkInv->GetInvTotalCount(riOutNeedItemNo);
		if( riOutHaveItemCount >= riOutNeedItemCount )
		{ //캐쉬 아이템 검증
			bHaveCashItem = true;
			break;
		}
		++item_iter;
	}

	return bHaveCashItem;
}

void PgConstellationEnterUIMgr::UpdateRewardBounsConsumeInfo(void)
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"FRM_CONSTELLATION_RESULT");
	if(!pkWnd) { return; }

	XUI::CXUI_Wnd* pIcon = pkWnd->GetControl(L"ICON_BONUS_ITEM");
	XUI::CXUI_Wnd* pkWndArrow = pkWnd->GetControl(L"FRM_ARROW");
	XUI::CXUI_Wnd* pIconNeed = pkWnd->GetControl(L"FRM_NEED");
	XUI::CXUI_Wnd* pIconHave = pkWnd->GetControl(L"FRM_HAVE");
	XUI::CXUI_Wnd* pTooltip = pkWnd->GetControl(L"FRM_HELP_TOOLTIP");
	XUI::CXUI_Button* pkExitBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(_T("BTN_CANCLE")));

	bool bVisible = m_iNeedRewardConsumeItemNoAtUI != 0 && EnoughConsumeItem() && RewardCurrentChoice() < MAX_CHOICE && GetState() == ES_BONUS_PROCESS_READY;
	if(pIcon)
	{
		pIcon->GrayScale(!bVisible);
		pIcon->SetCustomData(&m_iNeedRewardConsumeItemNoAtUI, sizeof(int));
	}

	if(pkWndArrow)
	{
		pkWndArrow->Visible(bVisible);
	}

	if(pkExitBtn)
	{
		pkExitBtn->Disable(GetState() != ES_BONUS_PROCESS_READY);
	}

	if(pIconNeed)
	{
		BM::vstring vStrItemCount(TTW(798821).c_str());
		vStrItemCount += TTW(3308);
		vStrItemCount += m_iNeedRewardConsumeItemCount;
		vStrItemCount += TTW(3308);
		vStrItemCount += TTW(1709);
		pIconNeed->Text(vStrItemCount);
	}

	if(pIconHave)
	{
		BM::vstring vStrItemHaveCount(TTW(798822).c_str());
		vStrItemHaveCount += TTW(3308);
		vStrItemHaveCount += m_iHaveRewardConsumeItemCount;
		vStrItemHaveCount += TTW(3308);
		vStrItemHaveCount += TTW(1709);
		pIconHave->Text(vStrItemHaveCount);
	}

}

bool PgConstellationEnterUIMgr::ConstellationResultOpenningUpdate(float fCurTime)
{
	static const float fTimeLeftStep = 0.7f;
	static const float fTimeRightStep = 0.15f;
	static float fTimeNext = 0;
	static size_t iRightStepCurIndex = 0;
	static PgItemBag::BagElementCont kContBagElem;

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"FRM_CONSTELLATION_RESULT");
	if( !pkWnd )
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(_T("FRM_CONSTELLATION_RESULT"));
		if( !pkWnd )
		{
			return false;
		}
	}

	switch(m_eAppStep)
	{
	case EAS_INIT:
		{
			GET_DEF(CItemBagMgr, kItemBagMgr);
			PgItemBag kItemBag;
			PgPilot* pkMyPilot = g_kPilotMan.GetPlayerPilot();
			if(!pkMyPilot || E_FAIL == kItemBagMgr.GetItemBag(m_kArrRewardInfo[m_iMyRewardIndex].iAddChoiceRewardBagNo, pkMyPilot->GetAbil(AT_LEVEL), kItemBag))
			{
				return false;
			}
			kContBagElem.clear();
			kContBagElem = kItemBag.GetElements();

			fTimeNext = fCurTime + fTimeRightStep;
			iRightStepCurIndex = 0;
			bool bInit = lua_tinker::call<bool, lwUIWnd>("InitConstellationResultSlide", lwUIWnd(pkWnd));
			if(!bInit) { return false; }

			m_eAppStep++;
		}break;
	case EAS_APPEAR_WINDOW:
		{
			bool bInit = lua_tinker::call<bool, lwUIWnd, float>("UpdateConstellationResultSlide", lwUIWnd(pkWnd), fCurTime);
			if(bInit)
			{
				m_eAppStep++;
			}
		}break;
	case EAS_REWARD_ITEMBAG:
		{
			if(fCurTime >= fTimeNext)
			{
				BM::vstring vstrIconName(L"ICON_SECLECT_ITEM");
				vstrIconName += iRightStepCurIndex;
				XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(vstrIconName));
				if(pkIcon)
				{
					try
					{
						pkIcon->SetCustomData(&kContBagElem.at(iRightStepCurIndex).iItemNo, sizeof(int));

						SIconInfo kIconInfo = pkIcon->IconInfo();
						kIconInfo.iIconCount = kContBagElem.at(iRightStepCurIndex).nCount;
						pkIcon->SetIconInfo(kIconInfo);
					}
					catch(std::out_of_range& exception)
					{
						return false;
					}
					lwPlaySoundByID("roulette_tick");
				}

				if(iRightStepCurIndex++ < 9)
				{
					fTimeNext = fCurTime + fTimeRightStep;
				}
				else
				{
					fTimeNext = fCurTime + fTimeLeftStep;
					m_eAppStep++;
				}
			}
		}break;
	case EAS_ESSENTIAL_REWARD:
		{ //필수 보상 아이콘 나타남
			if(fCurTime >= fTimeNext)
			{
				XUI::CXUI_Wnd* pkIcon = pkWnd->GetControl(_T("ICON_MY_ITEM0"));
				if(!pkIcon) { return false; }
				pkIcon->SetCustomData(&m_kArrRewardInfo[m_iMyRewardIndex].iRewardItemNo, sizeof(size_t));
				lwPlaySoundByID("inchant-change-steppass");

				fTimeNext = fCurTime + fTimeLeftStep;
				m_eAppStep++;
			}
		}break;
/*	case EAS_ADDITION_REWARD:
		{ //랜덤 보상 아이콘 나타남
			if(fCurTime >= fTimeNext)
			{
				XUI::CXUI_Wnd* pkIcon = pkWnd->GetControl(_T("ICON_MY_ITEM1"));
				if(!pkIcon) { return false; }
				pkIcon->SetCustomData(&m_kArrRewardInfo[m_iMyRewardIndex].iRandRewardItemNo, sizeof(size_t));
				lwPlaySoundByID("inchant-change-steppass");

				fTimeNext = fCurTime + fTimeLeftStep;
				m_eAppStep++;
			}
		}break;*/
	default:
		{
			SetState(ES_BONUS_PROCESSING);
			//SetState(ES_BONUS_PROCESS_READY);
		}break;
	}

	return true;
}


void PgConstellationEnterUIMgr::ConstellationResultUpdate()
{
	static const int PERCENT_MAX = 100;
	static const float fConstSendTime = 0.6f;
	static const float fConstAniMaxTime = fConstSendTime+3.5f;
	static const float fConstAniSleepTime = 0.05f;
	static float fOldTime = 0.f;
	static float fOldAniTime = 0.f;
	static float fFastAniTime = 0.f;
	float fNowTime = g_pkApp->GetAccumTime();
	enum
	{
		SLOT_INDEX_MIN = 0,
		SLOT_INDEX_MAX = 8,
		SLOT_INDEX_NONE = 1000 //이 값은 슬롯 인덱스가 선택되지 않음을 의미하는 상수로 사용
	};
	static size_t iPrevSlotIndex = SLOT_INDEX_NONE;

	switch(GetState())
	{
	case ES_STEP_1:
		{
			if(!ConstellationResultOpenningUpdate(fNowTime))
			{ //...
			}
		}break;
	case ES_BONUS_PROCESSING:
		{
			//상태변화
			switch(GetProcessState())
			{
			case EPS_INIT:
				{
					fOldTime = g_pkApp->GetAccumTime();
					fOldAniTime = g_pkApp->GetAccumTime();
					fFastAniTime = 0.f;
					iPrevSlotIndex = SLOT_INDEX_NONE;
					SetProcessState(EPS_NORMAL);
				}break;
			case EPS_NORMAL:
				{
					if(fNowTime-fOldTime > fConstSendTime )
					{
						SetProcessState( EPS_SENDING );
					}
				}break;
			case EPS_SENDING:
				{
					SetProcessState( EPS_RECV_WAIT );
				}break;
			case EPS_RECV:
				{
					int const iRate = ((fNowTime-fOldTime)/fConstAniMaxTime)*100;
					if(iRate >= 95)			{	fFastAniTime += 0.0001f;	}
					else if(iRate >= 90)	{	fFastAniTime  = 0.32f;		}
					else if(iRate >= 70)	{	fFastAniTime  = 0.16f;		}
					else if(iRate >= 50)	{	fFastAniTime  = 0.08f;		}
					else if(iRate >= 30)	{	fFastAniTime  = 0.02f;		}
					else					{	fFastAniTime  = 0.0f;		}
				}break;
			case EPS_FINISH:
				{
					XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"FRM_CONSTELLATION_RESULT");
					if( !pkWnd ) { break; }

					BM::vstring vStrIcon(L"ICON_SECLECT_ITEM");
					vStrIcon += RewardCurrentStop();
					XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(vStrIcon));
					if( pkIcon )
					{
						int iIconNo = 0;
						pkIcon->GetCustomData(&iIconNo, sizeof(int));
						if(iIconNo > 0)
						{
							BM::vstring vStrRecvIcon(L"ICON_MY_ITEM");
							vStrRecvIcon += RewardCurrentChoice() % 4;
							XUI::CXUI_Icon* pkRecvIcon = dynamic_cast<XUI::CXUI_Icon*>(pkWnd->GetControl(vStrRecvIcon));
							if(pkRecvIcon)
							{
								pkRecvIcon->SetCustomData(&iIconNo, sizeof(int));

								SIconInfo kIconInfo = pkRecvIcon->IconInfo();
								kIconInfo.iIconCount = pkIcon->IconInfo().iIconCount;
								pkRecvIcon->SetIconInfo(kIconInfo);
							}
						}
					}

					lwPlaySoundByID("inchant-change-success");
					SetState(ES_RESET_OK);
				}break;
			}

			//애니메이션
			switch(GetProcessState())
			{
			case EPS_RECV_WAIT:
				{
					SetProcessState( EPS_RECV );
				}break;
			case EPS_RECV:
				{ //보너스 연출 돌리는 중
					XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"FRM_CONSTELLATION_RESULT");
					if( !pkWnd )
					{
						SetProcessState( EPS_FINISH );
						break;
					}
					int const iPercent = std::min<int>(PERCENT_MAX,((fNowTime-fOldTime)/fConstSendTime)*100);
					size_t iChoiceSlotIndex = BM::Rand_Range(SLOT_INDEX_MIN, SLOT_INDEX_MAX);
					bool bUpdate = false;
					if( (PERCENT_MAX <= iPercent) &&
						(fNowTime-fOldAniTime > fConstAniSleepTime+fFastAniTime) )
					{
						fOldAniTime = fNowTime;
						bUpdate = true;
					}
					if( fNowTime - fOldTime > fConstAniMaxTime + fConstAniSleepTime )
					{ //1회 연출 끝. 다음 단계로
						iChoiceSlotIndex = RewardCurrentStop();
						SetRewardBonusChosenSlot(pkWnd, iChoiceSlotIndex);
						bUpdate = true;
						SetProcessState( EPS_FINISH );
					}
					if(bUpdate)
					{
						if(iPrevSlotIndex != SLOT_INDEX_NONE)
						{
							SetRewardBonusMixSlot(pkWnd, iPrevSlotIndex, false); //이전 칸 off
						}
						SetRewardBonusMixSlot(pkWnd, iChoiceSlotIndex, true); //현재 칸 on
						lwPlaySoundByID("inchant-change-ing");
						iPrevSlotIndex = iChoiceSlotIndex;
					}
				}break;
			}
		}break;
		default:
		{
		}break;
	}

	CONT_PARTY_MEMBER_REWARD::iterator iter =  m_PartyMemberReward.begin();
	if( m_PartyMemberReward.end() != iter )
	{
		if( g_pkApp && ((g_pkApp->GetAccumTime() - iter->AccumTime) >= 4.5) )
		{
			XUI::CXUI_Wnd * pWnd = XUIMgr.Get(_T("FRM_CONSTELLATION_RESULT"));
			if( pWnd )
			{
				XUI::CXUI_Wnd * IconWnd = pWnd->GetControl(iter->FormName);
				if( IconWnd )
				{
					IconWnd->SetCustomData(&iter->ItemNo, sizeof(iter->ItemNo) );
				}
			}
			m_PartyMemberReward.erase(iter);
		}
	}
}

//결산 보너스 룰렛 연출 애니 on/off
void PgConstellationEnterUIMgr::SetRewardBonusMixSlot(XUI::CXUI_Wnd* pkWnd, size_t iSlotIndex, bool bVisible)
{
	if(!pkWnd) { return; }

	BM::vstring vStr(L"FRM_C_RESULT_SLOT");
	vStr += iSlotIndex;
	XUI::CXUI_Wnd* pkSlot = pkWnd->GetControl(vStr);
	if(!pkSlot) { return; }

	pkSlot->Visible(bVisible);

	XUI::CXUI_Wnd* pkSlotImg = pkSlot->GetControl(L"IMG_SELECT_ANI");
	if(pkSlotImg)
	{
		pkSlotImg->Visible(bVisible);
	}

}

void PgConstellationEnterUIMgr::SetRewardBonusChosenSlot(XUI::CXUI_Wnd* pkWnd, size_t iSlotIndex)
{
	if(!pkWnd) { return; }

	BM::vstring vStr(L"ICON_SECLECT_ITEM");
	vStr += iSlotIndex;
	XUI::CXUI_Wnd* pkSlot = pkWnd->GetControl(vStr);
	if(!pkSlot) { return; }

	pkSlot->GrayScale(true);
}


void PgConstellationEnterUIMgr::SetState(PgConstellationEnterUIMgr::E_STATE const eState)
{
	E_STATE const eOldState = m_eState;
	m_eState = eState;
	
	switch(m_eState)
	{
	case ES_OPEN:
		{
			ConstellationClear(true);
			m_eAppStep = EAS_INIT;
			lwConstellationEnterUIUtil::ResultWndInit();
			SetProcessState(EPS_INIT);
			SetState(ES_STEP_1);
			CONT_REWARD_CHOSEN_SLOT_LIST kContTemp;
		}break;
	case ES_STEP_1:
		{
		}break;
	case ES_BONUS_PROCESS_READY:
		{
			EnoughConsumeItem(GetPlayerRewardConsumItemCount(m_iNeedRewardConsumeItemNo, m_iNeedRewardConsumeItemCount, m_iHaveRewardConsumeItemCount));
			UpdateRewardBounsConsumeInfo();
			//lwSetReqInvWearableUpdate(true);
			//SetState(ES_PROCESSING);
		}break;
	case ES_BONUS_PROCESSING:
		{
			UpdateRewardBounsConsumeInfo();
			SetProcessState(EPS_INIT);
		}break;
	case ES_RESET:
		{
		}break;
	case ES_RESET_OK:
		{
			NETWORK_SEND(BM::Stream(PT_C_M_REQ_CONSTELLATION_CASH_REWARD_COMPLETE));
			SetState(ES_BONUS_PROCESS_READY);
		}break;
	}
}
void PgConstellationEnterUIMgr::SetProcessState(E_PROCESS_STATE const eState)
{
	m_eProcessState = eState;
}

bool PgConstellationEnterUIMgr::ConstellationIsReset()const
{
	return ES_RESET==m_eState;
}

void PgConstellationEnterUIMgr::Clear()
{
	m_ContEnter.clear();
	m_Mission.Clear();
	m_TempCont.clear();
	m_ContBonusItem.clear();
	m_EntranceCashItem.clear();
	m_PartyMemberReward.clear();
}

void PgConstellationEnterUIMgr::LoadFile(const char* pcFilename)
{
	TiXmlDocument XmlDoc(pcFilename);
	if(!PgXmlLoader::LoadFile(XmlDoc, UNI(pcFilename)))
	{
		PgError1("Parse Failed [%s]", pcFilename);
		return;
	}

	m_ContEnter.clear();

	TiXmlNode* pRootNode = XmlDoc.FirstChild("CONSTELLATION");
	if( pRootNode )
	{
		CONT_DEFITEM const* pDefItem = NULL;
		g_kTblDataMgr.GetContDef(pDefItem);
		CONT_DEFMONSTER const* pDefMonster = NULL;
		g_kTblDataMgr.GetContDef(pDefMonster);
		CONT_DEF_QUEST_REWARD const* pDefQuestReward = NULL;
		g_kTblDataMgr.GetContDef(pDefQuestReward);
		
		if( pDefItem && pDefMonster && pDefQuestReward )
		{
			TiXmlNode* pNextNode = pRootNode->FirstChild();
			while( pNextNode )
			{
				int OutGroundNo = 0;
				ParseXml( pNextNode, pDefItem, pDefMonster, pDefQuestReward, OutGroundNo );
				pNextNode = pNextNode->NextSibling();
			}
		}
	}
}

TiXmlNode const* PgConstellationEnterUIMgr::ParseXml(TiXmlNode const* pNode
													, CONT_DEFITEM const* pDefItem
													, CONT_DEFMONSTER const* pDefMonster
													, CONT_DEF_QUEST_REWARD const* pDefQuestReward
													, int & rOutGroundNo)
{
	if( NULL == pNode || NULL == pDefItem || NULL == pDefMonster || NULL == pDefQuestReward )
	{
		return NULL;
	}

	int const NodeType = pNode->Type();
	switch(NodeType)
	{
	case TiXmlNode::ELEMENT:
		{
			TiXmlElement *pElement = (TiXmlElement *)pNode;

			std::string TagName(pElement->Value());

			if( TagName == "CHECK_BONUS_CASH_ITEM" )
			{
				Constellation::CONT_CONSTELLATION_BONUS_ITEM::mapped_type kTemp;
				
				kTemp.ParseXml( pElement, pDefItem);

				m_ContBonusItem.insert( std::make_pair(kTemp.WorldMapNo, kTemp) );
			}
			else if( TagName == "CHECK_ENTRANCE_CASH_ITEM" )
			{
				Constellation::CONT_CONSTELLATION_ENTRANCE_CASH_ITEM::mapped_type element;
				element.ParseXml( pElement );
				
				m_EntranceCashItem.insert( std::make_pair(element.WorldMapNo, element) );
			}
			else if( TagName == "WORLD" )
			{
				TiXmlAttribute* pAttr = pElement->FirstAttribute();
				while( pAttr )
				{
					char const* pcAttrName = pAttr->Name();
					char const* pcAttrValue = pAttr->Value();

					if( strcmp( pcAttrName, "MAPNO" ) == 0 )
					{
						rOutGroundNo = atoi(pcAttrValue);
					}
					pAttr = pAttr->Next();
				}

				const TiXmlNode* pChildNode = pNode->FirstChild();
				if( NULL != pChildNode )
				{
					while( ( pChildNode = ParseXml( pChildNode, pDefItem, pDefMonster, pDefQuestReward, rOutGroundNo) ) );
				}
			}
			else if( TagName == "LOCAL" )
			{
				TiXmlNode const* pFindLocalNode = PgXmlLocalUtil::FindInLocal(g_kLocal, pElement);
				if( pFindLocalNode )
				{
					TiXmlElement const* pResultNode = pFindLocalNode->FirstChildElement();
					if( pResultNode )
					{
						ParseXml( pResultNode, pDefItem, pDefMonster, pDefQuestReward, rOutGroundNo );
					}
				}
			}
			else if( TagName == "DUNGEON" )
			{
				Constellation::CONT_CONSTELLATION::mapped_type NewElement;
				Constellation::CONT_CONSTELLATION_ENTER::mapped_type SubElement;

				if( SubElement.ParseXml(pElement, pDefItem, pDefMonster, pDefQuestReward) )
				{
					Constellation::CONT_CONSTELLATION_ENTER::key_type Key(SubElement.Position, SubElement.Difficulty);
					Constellation::CONT_CONSTELLATION::iterator iter = m_ContEnter.find(rOutGroundNo);
					if( iter == m_ContEnter.end() )
					{
						NewElement.insert( std::make_pair(Key, SubElement) );
						m_ContEnter.insert( std::make_pair(rOutGroundNo, NewElement) );
					}
					else
					{
						iter->second.insert( std::make_pair(Key, SubElement) );
					}
				}
			}
		}break;
	default:
		{
		}break;
	}

	// 같은 층의 다음 노드를 재귀적으로 파싱한다.
	const TiXmlNode* pNextNode = pNode->NextSibling();
	if(pNextNode)
	{
		return pNextNode;
	}

	return NULL;
}

void PgConstellationEnterUIMgr::CallConstellationUI()
{
	if( m_ContEnter.empty() )
	{
		return;
	}

	XUI::CXUI_Wnd * pTopWnd = XUIMgr.Call(L"FRM_CONSTELLATION_DUNGEON");
	if( NULL == pTopWnd )
	{
		return;
	}
}

void PgConstellationEnterUIMgr::InitConstellationUI(XUI::CXUI_Wnd * pWnd)
{
	if( NULL == pWnd )
	{
		return;
	}

	int SelectWnd = 0;

	for( int index = MIN_CONSTELLATION_DUNGEON; index < MAX_CONSTELLATION_DUNGEON; ++index )
	{
		if( IsInDay(index) )
		{
			SelectWnd = index;
			int const difficulty = IsInDifficulty(index);
			UpdateConstellationInfo(pWnd, index, difficulty);
			lua_tinker::call<void, int, int>("SetSelectedConstellationDungeon", index, difficulty); // lua script에 선택된 position, difficulty값 세팅
			break;
		}
	}

	/*for( int index = MIN_CONSTELLATION_DUNGEON; index < MAX_CONSTELLATION_DUNGEON; ++index )
	{
		BM::vstring vStr(L"FRM_CONSTELLATION_");
		vStr += index;
		XUI::CXUI_Wnd * pSelectedWnd = pWnd->GetControl((std::wstring)vStr);
		if( pSelectedWnd )
		{
			XUI::CXUI_Wnd * pSubWnd = pSelectedWnd->GetControl(L"FRM_SELECT");
			if( pSubWnd )
			{
				if( SelectWnd == index )
				{
					pSubWnd->AlphaMax(1);
				}
				else
				{
					pSubWnd->AlphaMax(0);
				}
			}
		}
	}*/
}

void PgConstellationEnterUIMgr::ReqEnterConstellation(int const Position, int const Difficulty)
{
	ConstellationKey Key(Position, Difficulty);
	DWORD dwRet = CheckCondition(Key);
	if( ECE_OK == dwRet )
	{
		BM::Stream Packet(PT_C_M_REQ_ENTER_CONSTELLATION);
		Packet.Push(Position);
		Packet.Push(Difficulty);
		Packet.Push(CheckCondition_CashItem());
		NETWORK_SEND(Packet);
	}
	else
	{// 입장 조건에 맞지 않다.
		ConditionErrorMsg(dwRet);
	}
}

void PgConstellationEnterUIMgr::ReqEnterConstellation_Boss()
{
	if( g_pkWorld && g_pkWorld->IsHaveAttr(GKIND_CONSTELLATION) )
	{// 이미 별자리 던전 내부일 경우에만 보스방에 입장요청 할 수 있다.
		BM::Stream Packet(PT_C_M_REQ_ENTER_CONSTELLATION_BOSS);
		m_Mission.ConstellationKey.WriteToPacket(Packet);
		NETWORK_SEND(Packet);
	}
}

DWORD PgConstellationEnterUIMgr::CheckCondition(ConstellationKey const& Key)
{
	DWORD dwRet = ECE_OK;

	if( NULL == g_pkWorld )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	CONT_CONSTELLATION::const_iterator iter = m_ContEnter.find(g_pkWorld->MapNo());
	if( iter == m_ContEnter.end() )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}
	
	CONT_CONSTELLATION_ENTER::const_iterator sub_iter = iter->second.find(Key);
	if( sub_iter == iter->second.end() )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	if( PgClientPartyUtil::IsInParty() )
	{// 파티에 속해있다면 파티장만 검사하도록 한다.
		BM::GUID myGuid;
		if( false == g_kPilotMan.GetPlayerPilotGuid( myGuid ) )
		{
			return dwRet |= ECE_ERROR_SYSTEM;
		}

		if( false == PgClientPartyUtil::IsPartyMaster( myGuid ) )
		{
			return dwRet |= ECE_ERROR_PARTYMASTER;
		}
	}

	if( false == m_bConstellationFreePass )
	{// GMCMD
		if( CheckCondition_CashItem() )
		{// 캐쉬아이템 체크
			dwRet = ECE_OK;
		}
		else
		{
			dwRet |= CheckCondition_DayofWeek(sub_iter->second);	// 요일 체크
			dwRet |= CheckCondition_Level(sub_iter->second);		// 레벨 체크
			dwRet |= CheckCondition_Item(sub_iter->second);			// 아이템 체크
			dwRet |= CheckCondition_Quest(sub_iter->second);		// 퀘스트 체크
		}
	}

	return dwRet;
}

void PgConstellationEnterUIMgr::ConditionErrorMsg(DWORD const dwRet)
{
	if( (dwRet & ECE_ERROR_LV_MIN)			//최소 레벨 제한에 걸렸을 때
		|| (dwRet & ECE_ERROR_LV_MAX) )		//최대 레벨 제한에 걸렸을 때
	{
		std::wstring const& Message = TTW(798900);
		
		lwAddWarnDataStr(lwWString(Message), 1, false);
	}
	else if( (dwRet & ECE_ERROR_NO_PARTY)		//파티가 없을 때
		|| (dwRet & ECE_ERROR_PARTYMASTER) )	//파티장이 아닐 때
	{
		std::wstring const& Message = TTW(798901);
		
		lwAddWarnDataStr(lwWString(Message), 1, false);
	}
	else if( dwRet & ECE_ERROR_QUEST )		//퀘스트 필요 조건에 걸렸을 때
	{
		PgQuestInfo const* pQuestInfo = g_kQuestMan.GetQuest( m_NeedQuestNo );
		if( pQuestInfo )
		{
			BM::vstring Message(TTW(798903));
			Message.Replace(L"#QUEST#", TTW(pQuestInfo->m_iTitleTextNo));
			
			lwAddWarnDataStr(lwWString((std::wstring)Message), 1, false);
		}
	}
	else if( dwRet & ECE_ERROR_ITEM )		//아이템 필요 조건에 걸렸을 때
	{
		std::wstring ItemName;
		BM::vstring Message(TTW(798902));
		if( GetItemName(m_NeedItemNo, ItemName) )
		{
			Message.Replace(L"#ITEM#", ItemName);

			lwAddWarnDataStr(lwWString((std::wstring)Message), 1, false);
		}
	}
	else if( dwRet & ECE_ERROR_CASH_ITEM )	// 캐시 아이템을 사용하려고 했는데 없을 떄
	{
		std::wstring ItemName;
		BM::vstring Message(TTW(798902));
		if( GetItemName(m_CashItemNo, ItemName) )
		{
			Message.Replace(L"#ITEM#", ItemName);

			lwAddWarnDataStr(lwWString((std::wstring)Message), 1, false);
		}
	}
	else
	{
		//ECE_ERROR_DAY				//요일에 걸렸을 때
		//ECE_ERROR_SYSTEM			//시스템 내부적 에러
	}
}

void PgConstellationEnterUIMgr::InitCashItem()
{
	m_CashItemNo = 0;
}

bool PgConstellationEnterUIMgr::UseConstellationCashItem(XUI::CXUI_Wnd * pWnd, bool const bUse)
{
	if( NULL == pWnd )
	{
		return false;
	}

	int FirstItemNo = 0;
	m_CashItemNo = CheckCashItemCount(FirstItemNo);
	
	if( 0 == m_CashItemNo )
	{
		return false;
	}

	XUI::CXUI_Wnd * deregWnd = pWnd->GetControl(L"BTN_DEREG");
	XUI::CXUI_Wnd * regWnd = pWnd->GetControl(L"BTN_REG");
	XUI::CXUI_Wnd * pCashItemWnd = pWnd->GetControl(L"SFRM_CASH_KEY_ITEM");

	if( NULL == deregWnd
		|| NULL == regWnd
		|| NULL == pCashItemWnd )
	{
		return false;
	}

	XUI::CXUI_Wnd * pIcon = pCashItemWnd->GetControl(L"ICN_ITEM");
	if( NULL == pIcon )
	{
		return false;
	}

	deregWnd->Visible(bUse);
	regWnd->Visible(!bUse);
	pIcon->GrayScale(!bUse);

	if( bUse )
	{
		pIcon->GetCustomData( &m_CashItemNo, sizeof(m_CashItemNo) );
		
		std::wstring ItemName;
		BM::vstring Message(TTW(798904));
		if( GetItemName(m_CashItemNo, ItemName) )
		{
			Message.Replace(L"#ITEM#", ItemName);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
		}
	}
	else
	{
		InitCashItem();
	}
	
	return true;
}

bool PgConstellationEnterUIMgr::CheckCondition_CashItem()
{
	return (m_CashItemNo > 0) ? true : false;
}

DWORD PgConstellationEnterUIMgr::CheckCondition_DayofWeek(CONT_CONSTELLATION_ENTER::mapped_type const& MissionInfo)
{// 요일 체크
	DWORD dwRet = ECE_OK;

	SYSTEMTIME sysTime;
	g_kEventView.GetLocalTime(&sysTime);

	SDAYOFWEEK dayOfWeek(EDOW_NONE);
	dayOfWeek.SetDayofWeek(MissionInfo.Condition.DayofWeek);

	if( false == dayOfWeek.IsInDay(static_cast<EDAYOFWEEK>(eDayOfWeekIdx[sysTime.wDayOfWeek])) )
	{
		dwRet |= ECE_ERROR_DAY;
	}
	
	return dwRet;
}

DWORD PgConstellationEnterUIMgr::CheckCondition_Level(CONT_CONSTELLATION_ENTER::mapped_type const& MissionInfo)
{// 레벨 체크
	DWORD dwRet = ECE_OK;

	int const LimitMaxLevel = MissionInfo.Condition.MaxLevel;
	int const LimitMinLevel = MissionInfo.Condition.MinLevel;

	if( 0 == LimitMaxLevel && 0 == LimitMinLevel )
	{// 레벨체크를 할 필요가 없다.
		return ECE_OK;
	}

	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	int const MyLevel = pPlayer->GetAbil(AT_LEVEL);

	if( LimitMaxLevel && LimitMaxLevel < MyLevel )
	{// 최대레벨  제한에 걸린다.
		dwRet |= ECE_ERROR_LV_MAX;
	}
	if( LimitMinLevel && LimitMinLevel > MyLevel )
	{// 최소레벨 제한에 걸린다.
		dwRet |= ECE_ERROR_LV_MIN;
	}

	return dwRet;
}

DWORD PgConstellationEnterUIMgr::CheckCondition_Item(CONT_CONSTELLATION_ENTER::mapped_type const& MissionInfo)
{// 아이템 체크
	DWORD dwRet = ECE_OK;

	int const NeedItemNo = MissionInfo.Condition.ItemNo;
	int NeedItemCount = MissionInfo.Condition.ItemCount;

	if( 0 == NeedItemNo )
	{// 체크할 아이템이 없다.
		return ECE_OK;
	}

	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	if( PgClientPartyUtil::IsInParty() )
	{// 파티에 속해 있는가?
		if( 3 <= g_kParty.MemberCount() )
		{
			NeedItemCount = 2;
		}
	}

	CONT_DEFITEM const* pDefItem = NULL;
	g_kTblDataMgr.GetContDef(pDefItem);
	if( !pDefItem )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	CONT_DEFITEM::const_iterator def_item = pDefItem->find( NeedItemNo );
	if( pDefItem->end() == def_item )
	{// 존재하지 않는 아이템이다.
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	PgInventory * pInven = pPlayer->GetInven();
	if( NULL == pInven )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	int ItemCount = (int)pInven->GetTotalCount( NeedItemNo );
	if( NeedItemCount > ItemCount )
	{
		m_NeedItemNo = NeedItemNo;
		dwRet |= ECE_ERROR_ITEM;
	}

	return dwRet;
}

DWORD PgConstellationEnterUIMgr::CheckCondition_Quest(CONT_CONSTELLATION_ENTER::mapped_type const& MissionInfo)
{// 퀘스트 체크
	DWORD dwRet = ECE_OK;

	int const NeedQuestNo = MissionInfo.Condition.QuestNo;

	if( 0 == NeedQuestNo )
	{// 체크할 퀘스트가 없다.
		return ECE_OK;
	}

	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	CONT_DEF_QUEST_REWARD const* pDefQuestReward = NULL;
	g_kTblDataMgr.GetContDef(pDefQuestReward);
	if( NULL == pDefQuestReward )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	CONT_DEF_QUEST_REWARD::const_iterator quest_iter = pDefQuestReward->find(NeedQuestNo);
	if( pDefQuestReward->end() == quest_iter )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	PgMyQuest const* pMyQuest = pPlayer->GetMyQuest();
	if( NULL == pMyQuest )
	{
		return dwRet |= ECE_ERROR_SYSTEM;
	}

	if( (false == pMyQuest->IsEndedQuest(NeedQuestNo)) && (false == pMyQuest->IsIngQuest(NeedQuestNo)) )
	{
		m_NeedQuestNo = NeedQuestNo;
		return dwRet |= ECE_ERROR_QUEST;
	}

	return dwRet;
}

bool PgConstellationEnterUIMgr::ProcessMsg(unsigned short const usType, BM::Stream& Packet)
{
	switch( usType )
	{
	case PT_M_C_NFY_ENTER_CONSTELLATION:
		{
			Recv_PT_M_C_NFY_ENTER_CONSTELLATION(Packet);
		}break;
	case PT_M_C_NFY_CONSTELLATION_MISSION:
		{
			Recv_PT_M_C_NFY_CONSTELLATION_MISSION(Packet);
		}break;
	case PT_M_C_NFY_CONSTELLATION_MODIFY_MISSION:
		{
			Recv_PT_M_C_NFY_CONSTELLATION_MODIFY_MISSION(Packet);
		}break;
	case PT_M_C_NFY_CONSTELLATION_MISSION_FAIL:
		{
			Recv_PT_M_C_NFY_CONSTELLATION_MISSION_FAIL(Packet);
		}break;
	case PT_M_C_NFY_CONSTELLATION_RESULT_WAIT:
		{
			Recv_PT_M_C_NFY_CONSTELLATION_RESULT_WAIT(Packet);
		}break;
	case PT_M_C_NFY_CONSTELLATION_RESULT:
		{
			Recv_PT_M_C_NFY_CONSTELLATION_RESULT(Packet);
		}break;
	case PT_M_C_NFY_CONSTELLATION_CASH_REWARD:
		{
			Recv_PT_M_C_NFY_CONSTELLATION_CASH_REWARD(Packet);
		}break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

void PgConstellationEnterUIMgr::Recv_PT_M_C_NFY_ENTER_CONSTELLATION(BM::Stream& Packet)
{
	DWORD dwRet = 0;
	int GroundNo = 0;
	int Position = 0;
	int Difficulty = 0;

	Packet.Pop(dwRet);
	Packet.Pop(GroundNo);
	Packet.Pop(Position);
	Packet.Pop(Difficulty);

	int MinLevel = 0;
	int MaxLevel = 0;
	CONT_CONSTELLATION::const_iterator iter = m_ContEnter.find(GroundNo);
	if( m_ContEnter.end() != iter )
	{
		ConstellationKey Key(Position, Difficulty);
		CONT_CONSTELLATION_ENTER::const_iterator sub_iter = iter->second.find(Key);
		if( iter->second.end() != sub_iter )
		{
			MinLevel = sub_iter->second.Condition.MinLevel;
			MaxLevel = sub_iter->second.Condition.MaxLevel;
		}
	}

	lua_tinker::call<void>("SetSelectedConstellationDungeon", Position, Difficulty);
	lua_tinker::call<void>("SetConstellationInfo", MinLevel, MaxLevel);

	lua_tinker::call<void>("CallConstellationUI");

	ConditionErrorMsg((ECE_RESULT)dwRet);
}

void PgConstellationEnterUIMgr::Recv_PT_M_C_NFY_CONSTELLATION_MISSION(BM::Stream& Packet)
{
	m_Mission.Clear();
	bool bBossMap = false;
	Packet.Pop( bBossMap );
	PU::TLoadArray_A(Packet, m_Mission.DungeonMission);
	PU::TLoadArray_A(Packet, m_Mission.NormalMapMission);
	if( bBossMap )
	{
		PU::TLoadArray_A(Packet, m_Mission.BossMapMission);
	}
	m_Mission.ConstellationKey.ReadFromPacket(Packet);

	if( m_Mission.DungeonMission.empty() && m_Mission.NormalMapMission.empty() && m_Mission.BossMapMission.empty() )
	{// 모두 비어 있으면 
		XUIMgr.Close(L"SFRM_CONSTELLATION_MISSION");
		return;
	}
	AddMission();
}

void PgConstellationEnterUIMgr::Recv_PT_M_C_NFY_CONSTELLATION_MODIFY_MISSION(BM::Stream& Packet)
{
	std::wstring MissionType;
	int MissionNo = 0;
	BYTE State = 0;
	Packet.Pop(MissionType);
	Packet.Pop(MissionNo);
	Packet.Pop(State);

	if( MissionType == MISSION_DUNGEON )
	{
		CONT_MAP_MISSION_EX::iterator iter = m_Mission.DungeonMission.begin();
		while( m_Mission.DungeonMission.end() != iter )
		{
			if( iter->Mission.MissionNo == MissionNo )
			{
				iter->State = State;
				NoticeMissionState(TTW(iter->Mission.MissionTextNo), iter->State);
				break;
			}
			++iter;
		}
	}
	else if( MissionType == MISSION_NORMAL )
	{
		CONT_MAP_MISSION_EX::iterator iter = m_Mission.NormalMapMission.begin();
		while( m_Mission.NormalMapMission.end() != iter )
		{
			if( iter->Mission.MissionNo == MissionNo )
			{
				iter->State = State;
				NoticeMissionState(TTW(iter->Mission.MissionTextNo), iter->State);
				break;
			}
			++iter;
		}
	}
	else if( MissionType == MISSION_BOSS )
	{
		CONT_MAP_MISSION_EX::iterator iter = m_Mission.BossMapMission.begin();
		while( m_Mission.BossMapMission.end() != iter )
		{
			if( iter->Mission.MissionNo == MissionNo )
			{
				iter->State = State;
				NoticeMissionState(TTW(iter->Mission.MissionTextNo), iter->State);
				break;
			}
			++iter;
		}
	}
	
	AddMission();
}

void PgConstellationEnterUIMgr::NoticeMissionState(std::wstring const& MissionText, BYTE const State)
{
	std::wstring Message = L"'";
	Message += MissionText;
	Message += L"' ";

	switch( State )
	{
	case Constellation::MissionState::MS_FAIL:
		{
			Message += TTW(20002);
			lwAddWarnDataStr(lwWString(Message), 1, false);
		}break;
	case Constellation::MissionState::MS_CLEAR:
		{
			Message += TTW(20001);

			lwAddWarnDataStr(lwWString(Message), 7, false);
		}break;
	}
}

void PgConstellationEnterUIMgr::Recv_PT_M_C_NFY_CONSTELLATION_MISSION_FAIL(BM::Stream& Packet)
{
	if( g_pkWorld && (false == g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION)) )
	{
		XUIMgr.Call(L"FRM_CONSTELLATION_FAILED");
	}
}

void PgConstellationEnterUIMgr::Recv_PT_M_C_NFY_CONSTELLATION_RESULT_WAIT(BM::Stream& Packet)
{
	AllClearMission();
	if( g_pkWorld && (false == g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION)) )
	{
		XUIMgr.Call(L"FRM_CONSTELLATION_CLEAR");
	}
}

void PgConstellationEnterUIMgr::Recv_PT_M_C_NFY_CONSTELLATION_RESULT(BM::Stream& Packet)
{
	std::wstring wstrName;
	int iRandRewardItemNo = 0;
	int iRewardItemNo = 0;
	int iAddChoiceRewardBagNo = 0;
	RewardCurrentChoice(1);

	std::wstring kCancleText(TTW(400338));
	if( IsDefenceMode7() )
	{
		if(false==g_kMissionComplete.GetMission_ResultC().IsLastStage())
		{
			kCancleText = TTW(400533);
			lua_tinker::call<void,int>("UI_SetConstellationResultCountTimeSec", g_kMissionComplete.GetUICloseTime2()-2);
		}
	}

	XUI::CXUI_Wnd* pWnd = XUIMgr.Call(_T("FRM_CONSTELLATION_RESULT"));
	if(pWnd==NULL)
	{
		return;
	}
	
	lua_tinker::call<void>("CloseConstellationUI");

	XUI::CXUI_Wnd* pCancleWnd = pWnd->GetControl(L"BTN_CANCLE");
	if(pCancleWnd)
	{
		pCancleWnd->Text(kCancleText);
	}

	g_kConstellationEnterUIMgr.SetState(PgConstellationEnterUIMgr::ES_OPEN);

	Packet.Pop(m_iRewardPlayer);//참여인원
	
	lua_tinker::call< void >("Constellation__ClearRecord"); //전체 지움

	int iIdx = 0;
	int iCountIdx = 0;
	int iItemIndex = 0;
	for(int i=0; i < m_iRewardPlayer; ++i)
	{
		if(i >= MAX_PLAYER) //혹시 몰라서 방어코드
		{
			continue;
		}
		Packet.Pop(m_kArrRewardInfo[i].wstrName); //파티원 이름.
		Packet.Pop(m_kArrRewardInfo[i].iRewardItemNo); //기본 지급 보상 아이템 번호
		Packet.Pop(m_kArrRewardInfo[i].iAddChoiceRewardBagNo); //아이템백 번호
		Packet.Pop(iItemIndex); //이 값은 현재 사용 안 함
		Packet.Pop(m_kArrRewardInfo[i].iRandRewardItemNo); //랜덤 지급 보상 아이템 번호

		PgPilot* pkPilot = g_kPilotMan.FindPilot(m_kArrRewardInfo[i].wstrName);
		if(pkPilot == NULL)
		{
			continue;
		}

		if(g_kPilotMan.IsMyPlayer(pkPilot->GetGuid()))
		{ //루프 내 한번만 실행
			m_iMyRewardIndex = i;
			RewardCurrentStop(iItemIndex);

			XUI::CXUI_Wnd* pkID = pWnd->GetControl(_T("FRM_MY_ID"));
			if(pkID != NULL)
			{
				pkID->Text(m_kArrRewardInfo[i].wstrName);
			}
		}
		else
		{
			BM::vstring vStrPartyNum(L"FRM_MY_PARTY_MEMBER");
			vStrPartyNum += iCountIdx;
			XUI::CXUI_Wnd* pkPartyNumID = pWnd->GetControl(vStrPartyNum); //컨트롤 아이디 수정 필요!!!!
			if(pkPartyNumID != NULL) 
			{
				pkPartyNumID->Text(m_kArrRewardInfo[i].wstrName);
			}
			//if(NULL != pkPartyNumID->Text(m_kArrRewardInfo[i].wstrName)) 아니면 case 1 2 3 switch(i)
			BM::vstring strPartyIcon(_T("ICON_ITEM_PARTY_MEMBER"));
			strPartyIcon += iCountIdx;
			strPartyIcon += 0;
			XUI::CXUI_Wnd* pkPartyIcon = pWnd->GetControl(strPartyIcon);
			if(pkPartyIcon != NULL)
			{
				pkPartyIcon->SetCustomData(&m_kArrRewardInfo[i].iRandRewardItemNo, sizeof(m_kArrRewardInfo[i].iRandRewardItemNo));
			}
			iCountIdx++;
		}
	}
}
void PgConstellationEnterUIMgr::Recv_PT_M_C_NFY_CONSTELLATION_CASH_REWARD(BM::Stream& Packet)
{
	std::wstring strName;
	int iIndexItemNo = 0;
	int iItemNo = 0;
	const int iMaxItemSlot = 3;
	static const int MAX_PLAYER = 3;

	XUI::CXUI_Wnd* pWnd = XUIMgr.Get(_T("FRM_CONSTELLATION_RESULT"));
	if(pWnd==NULL)
	{
		return;
	}

	int iCurChoice;
	Packet.Pop(strName); //이름 파티원
	Packet.Pop(iCurChoice); //추가 지급 보상: 몇번째 선택인가?
	Packet.Pop(iIndexItemNo); //추가 지급 보상: 최종 지급될 아이템백 인덱스 번호
	Packet.Pop(iItemNo); //추가 지급 보상: 최종 지급될 아이템 번호
	if(iItemNo == 0)
	{
		return;
	}

	PgPilot* pkMyPilot = g_kPilotMan.GetPlayerPilot();
	if(pkMyPilot == NULL) { return; }

	if(pkMyPilot->GetName() == strName)
	{ //내 정보인가?
		if(GetState() != ES_BONUS_PROCESS_READY)
		{ //결과창이 현재 대기 상태가 아니면 연출 패킷이 와도 무시함
			return;
		}
		RewardCurrentChoice(iCurChoice);
		RewardCurrentStop(iIndexItemNo);
		SetState(ES_BONUS_PROCESSING); //돌려라
	}
	else
	{ //파티원의 정보인가?
		int iPlayerIdx = -1;
		for(int iPlayer = 0; iPlayer < MAX_PLAYER; iPlayer++)
		{
			BM::vstring vStrPlayerNameID(L"FRM_MY_PARTY_MEMBER");
			vStrPlayerNameID += iPlayer;
			XUI::CXUI_Wnd* vStrPlayerNameWnd = pWnd->GetControl(vStrPlayerNameID);
			if(vStrPlayerNameWnd == NULL) { continue; }

			if(vStrPlayerNameWnd->Text() == strName)
			{
				iPlayerIdx = iPlayer;
				break;
			}
		}
		if(iPlayerIdx == -1) { return; } //해당 플레이어의 UI 인덱스를 찾을 수 없음!

		BM::vstring vStrPlayerIconID(L"ICON_ITEM_PARTY_MEMBER");
		vStrPlayerIconID += iPlayerIdx;
		vStrPlayerIconID += (iCurChoice - 1) % 3;
		
		if( g_pkApp )
		{
			SPartyMemberReward element;
			element.FormName = (std::wstring)vStrPlayerIconID;
			element.AccumTime = g_pkApp->GetAccumTime();
			element.ItemNo = iItemNo;

			m_PartyMemberReward.push_back(element);
		}
	}

}

void PgConstellationEnterUIMgr::AddMission()
{
	XUI::CXUI_Wnd * pWnd = XUIMgr.Activate(L"SFRM_CONSTELLATION_MISSION");
	if( NULL == pWnd )
	{
		return;
	}

	XUI::CXUI_List * pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_MISSION"));
	if( NULL == pList )
	{
		return;
	}

	pList->ClearList();	// 리스트 초기화
	
	AddDungeonMission(pList);
	AddNormalMission(pList);
	AddBossMission(pList);

	int const TotalCount = pList->GetTotalItemCount();
	POINT2 size = pWnd->Size();

	XUI::SListItem * pItem = pList->FirstItem();
	if( NULL == pItem )
	{
		return;
	}

	XUI::CXUI_Wnd * pItemWnd = pItem->m_pWnd;
	if( NULL == pItemWnd )
	{
		return;
	}

	size.y = (TotalCount * pItemWnd->Size().y) + 5;
	pWnd->Size(size);
}

void PgConstellationEnterUIMgr::AddDungeonMission(XUI::CXUI_List * pList)
{// 던전미션 등록
	if( NULL == pList )
	{
		return;
	}
	if( m_Mission.DungeonMission.empty() )
	{
		return;
	}
	CONT_MAP_MISSION_EX::const_iterator iter = m_Mission.DungeonMission.begin();
	while( m_Mission.DungeonMission.end() != iter )
	{
		SetMissionState(pList, iter->Mission.MissionTextNo, iter->State);
		++iter;
	}
}

void PgConstellationEnterUIMgr::AddNormalMission(XUI::CXUI_List * pList)
{// 일반맵 미션 등록
	if( NULL == pList )
	{
		return;
	}
	if( m_Mission.NormalMapMission.empty() )
	{
		return;
	}
	CONT_MAP_MISSION_EX::const_iterator iter = m_Mission.NormalMapMission.begin();
	while( m_Mission.NormalMapMission.end() != iter )
	{
		SetMissionState(pList, iter->Mission.MissionTextNo, iter->State);
		++iter;
	}
}

void PgConstellationEnterUIMgr::AddBossMission(XUI::CXUI_List * pList)
{// 보스맵 미션 등록
	if( NULL == pList )
	{
		return;
	}
	if( m_Mission.BossMapMission.empty() )
	{
		return;
	}
	CONT_MAP_MISSION_EX::const_iterator iter = m_Mission.BossMapMission.begin();
	while( m_Mission.BossMapMission.end() != iter )
	{
		SetMissionState(pList, iter->Mission.MissionTextNo, iter->State);
		++iter;
	}
}

void PgConstellationEnterUIMgr::SetMissionState(XUI::CXUI_List * pList, int const MissionTextNo, BYTE const MissionState)
{
	if( NULL == pList )
	{
		return;
	}

	XUI::SListItem* pItem = pList->AddItem(L""); // 리스트에 등록시킨다.
	if( NULL == pItem )
	{
		return;
	}

	XUI::CXUI_Wnd* pItemWnd = pItem->m_pWnd;
	if( NULL == pItemWnd )
	{
		return;
	}

	pItemWnd->Text(TTW(MissionTextNo));

	XUI::CXUI_Wnd * pClear = pItemWnd->GetControl(L"FRM_CLEAR");
	if( NULL == pClear )
	{
		return;
	}

	switch( MissionState )
	{
	case MS_NONE:
		{
			pClear->UVUpdate( 1 );
		}break;
	case MS_CLEAR:
		{
			pClear->UVUpdate( 2 );
		}break;
	case MS_FAIL:
		{
			pClear->UVUpdate( 3 );
		}break;
	}
}

void PgConstellationEnterUIMgr::AllClearMission()
{
	XUI::CXUI_Wnd * pWnd = XUIMgr.Activate(L"SFRM_CONSTELLATION_MISSION");
	if( NULL == pWnd )
	{
		return;
	}

	XUI::CXUI_List * pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_MISSION"));
	if( NULL == pList )
	{
		return;
	}

	XUI::SListItem * pListItem = pList->FirstItem();
	while( pListItem )
	{
		XUI::CXUI_Wnd * pItemWnd = pListItem->m_pWnd;
		if( pItemWnd )
		{
			XUI::CXUI_Wnd * pClear = pItemWnd->GetControl(L"FRM_CLEAR");
			if( pClear )
			{
				SUVInfo const& info = pClear->UVInfo();
				if( 1 == info.Index )
				{// 현재 상태가 완료되지 않은 미션이라면 성공메시지를 띄워준다.
					NoticeMissionState(pItemWnd->Text(), 2);
				}

				pClear->UVUpdate( 2 );
			}
		}
		pListItem = pList->NextItem(pListItem);
	}
}

bool PgConstellationEnterUIMgr::IsInDay(int const Position)
{
	int const GroundNo = GetNowGroundNo();
	CONT_CONSTELLATION::const_iterator iter = m_ContEnter.find(GroundNo);
	if( m_ContEnter.end() == iter )
	{
		return false;
	}

	ConstellationKey Key(Position, 1); // 입장일 제한은 난이도에 관계없이 같다는 전제로
	CONT_CONSTELLATION_ENTER::const_iterator sub_iter = iter->second.find(Key);
	if( iter->second.end() == sub_iter )
	{
		return false;
	}

	SYSTEMTIME sysTime;
	g_kEventView.GetLocalTime(&sysTime);

	SDAYOFWEEK dayOfWeek(EDOW_NONE);
	dayOfWeek.SetDayofWeek(sub_iter->second.Condition.DayofWeek);

	return dayOfWeek.IsInDay(static_cast<EDAYOFWEEK>(eDayOfWeekIdx[sysTime.wDayOfWeek]));
}

bool PgConstellationEnterUIMgr::Enable(int const Position, int const Difficulty)
{
	int const GroundNo = GetNowGroundNo();
	CONT_CONSTELLATION::const_iterator iter = m_ContEnter.find(GroundNo);
	if( m_ContEnter.end() == iter )
	{
		return false;
	}

	ConstellationKey Key(Position, Difficulty);
	CONT_CONSTELLATION_ENTER::const_iterator sub_iter = iter->second.find(Key);
	if( iter->second.end() == sub_iter )
	{
		return false;
	}

	return sub_iter->second.Enable;
}

int PgConstellationEnterUIMgr::IsInDifficulty(int const Position)
{
	int const GroundNo = GetNowGroundNo();
	CONT_CONSTELLATION::const_iterator iter = m_ContEnter.find(GroundNo);
	if( m_ContEnter.end() == iter )
	{
		return 1;
	}

	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return 1;
	}

	int const MyLevel = pPlayer->GetAbil(AT_LEVEL);

	int difficulty = 0;
	for( difficulty = MAX_CONSTELLATION_DIFFICULTY - 1; difficulty >= MIN_CONSTELLATION_DIFFICULTY; --difficulty )
	{
		ConstellationKey Key(Position, difficulty);
		CONT_CONSTELLATION_ENTER::const_iterator sub_iter = iter->second.find(Key);
		if( iter->second.end() == sub_iter )
		{
			continue;
		}
		
		int const MaxLevel = sub_iter->second.Condition.MaxLevel;
		int const MinLevel = sub_iter->second.Condition.MinLevel;

		if( (MyLevel <= MaxLevel)
			&& (MyLevel >= MinLevel) )
		{
			return difficulty;
		}
	}
	return 1;
}

void PgConstellationEnterUIMgr::GetString_Difficulty(int const Difficulty, BM::vstring & OutStr)
{
	switch( Difficulty )
	{
	case 1:
		{
			OutStr = TTW(798813);
		}break;
	case 2:
		{
			OutStr = TTW(798814);
		}break;
	case 3:
		{
			OutStr = TTW(798815);
		}break;
	}
}

void PgConstellationEnterUIMgr::GetString_Position(int const Position, BM::vstring & OutStr)
{
	int const ConstellationTextBase = 798800;
	switch( Position )
	{
	case 1:
		{
			OutStr = TTW(ConstellationTextBase);
		}break;
	case 2:
		{
			OutStr = TTW(ConstellationTextBase+1);
		}break;
	case 3:
		{
			OutStr = TTW(ConstellationTextBase+2);
		}break;
	case 4:
		{
			OutStr = TTW(ConstellationTextBase+3);
		}break;
	case 5:
		{
			OutStr = TTW(ConstellationTextBase+4);
		}break;
	case 6:
		{
			OutStr = TTW(ConstellationTextBase+5);
		}break;
	case 7:
		{
			OutStr = TTW(ConstellationTextBase+6);
		}break;
	case 8:
		{
			OutStr = TTW(ConstellationTextBase+7);
		}break;
	case 9:
		{
			OutStr = TTW(ConstellationTextBase+8);
		}break;
	case 10:
		{
			OutStr = TTW(ConstellationTextBase+9);
		}break;
	case 11:
		{
			OutStr = TTW(ConstellationTextBase+10);
		}break;
	}
}

void PgConstellationEnterUIMgr::GetString_DayofWeek(VEC_WSTRING const& DayofWeek, BM::vstring & OutStr)
{
	int TotalCount = DayofWeek.size();
	std::vector<std::wstring>::const_iterator day_iter;
	for( day_iter = DayofWeek.begin(); day_iter != DayofWeek.end(); ++day_iter)
	{
		bool bInsert = false;
		if((*day_iter) == L"MON")
		{
			OutStr += TTW(11011);
			bInsert = true;
		}
		else if((*day_iter) == L"TUE")
		{
			OutStr += TTW(11012);
			bInsert = true;
		}
		else if((*day_iter) == L"WED")
		{
			OutStr += TTW(11013);
			bInsert = true;
		}
		else if((*day_iter) == L"THU")
		{
			OutStr += TTW(11014);
			bInsert = true;
		}
		else if((*day_iter) == L"FRI")
		{
			OutStr += TTW(11015);
			bInsert = true;
		}
		else if((*day_iter) == L"SAT")
		{
			OutStr += TTW(11016);
			bInsert = true;
		}
		else if((*day_iter) == L"SUN")
		{
			OutStr += TTW(11010);
			bInsert = true;
		}

		if( bInsert )
		{
			--TotalCount;
			if( TotalCount > 0 )
			{
				OutStr += "/";
			}
		}
	}
}

void PgConstellationEnterUIMgr::UpdateConstellationInfo(int const Position, int const Difficulty)
{
	XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"FRM_CONSTELLATION_DUNGEON");
	if( NULL == pWnd )
	{
		return;
	}

	UpdateConstellationInfo(pWnd, Position, Difficulty);
}

void PgConstellationEnterUIMgr::UpdateConstellationInfo(XUI::CXUI_Wnd * pWnd, int const Position, int const Difficulty)
{
	if( NULL == pWnd )
	{
		return;
	}

	int const GroundNo = GetNowGroundNo();
	CONT_CONSTELLATION::const_iterator iter = m_ContEnter.find(GroundNo);
	if( m_ContEnter.end() == iter )
	{
		return;
	}

	ConstellationKey Key(Position, Difficulty);
	CONT_CONSTELLATION_ENTER::const_iterator sub_iter = iter->second.find(Key);
	if( iter->second.end() == sub_iter )
	{
		return;
	}

	XUI::CXUI_Wnd * pName = pWnd->GetControl(L"FRM_DUNGEON_NAME");
	if( pName )
	{
		UpdateConstellationInfo_Name( pName, Difficulty, sub_iter->second.Name );
	}

	XUI::CXUI_Wnd * pInfo = pWnd->GetControl(L"SFRM_DUNGEON_INFO");
	if( NULL == pInfo )
	{
		return;
	}

	if( sub_iter->second.Enable )
	{
		pInfo->Text(TTW(401251));
	}
	else
	{
		pInfo->Text(L"");
	}

	XUI::CXUI_Wnd * pBox = pInfo->GetControl(L"SFRM_BOX_INFO");
	if( pBox )
	{
		pBox->Visible(sub_iter->second.Enable);
	}

	XUI::CXUI_Wnd * pText = pInfo->GetControl(L"FRM_TEXT_ITEM");
	if( pText )
	{
		pText->Visible(sub_iter->second.Enable);
	}
	pText = pInfo->GetControl(L"FRM_TEXT_MONSTER");
	if( pText )
	{
		pText->Visible(sub_iter->second.Enable);
	}
	pText = pInfo->GetControl(L"FRM_NEED_ITEM_INFO");
	if( pText )
	{
		pText->Visible(sub_iter->second.Enable);
	}
	pText = pInfo->GetControl(L"FRM_TEXT_NOT_OPEN");
	if( pText )
	{
		pText->Visible(!sub_iter->second.Enable);
	}

	XUI::CXUI_Wnd * pDay = pInfo->GetControl(L"SFRM_DAY");
	if( pDay )
	{// 입장 요일
		if( sub_iter->second.Enable )
		{
			pDay->Visible(true);
			XUI::CXUI_Wnd * pText = pDay->GetControl(L"FRM_TEXT");
			if( pText )
			{
				UpdateConstellationInfo_Day( pText, sub_iter->second.Condition.DayofWeek, Position,Difficulty );
			}
		}
		else
		{
			pDay->Visible(false);
		}
	}

	XUI::CXUI_Wnd * pQuest = pInfo->GetControl(L"FRM_TEXT_QUEST");
	if( pQuest )
	{// 입장 퀘스트
		if( sub_iter->second.Enable )
		{
			pQuest->Visible(true);
			XUI::CXUI_Wnd * pText = pQuest->GetControl(L"FRM_TEXT");
			if( pText )
			{
				UpdateConstellationInfo_Quest( pText, sub_iter->second.Condition.QuestNo );
			}
		}
		else
		{
			pQuest->Visible(false);
		}
	}

	XUI::CXUI_Wnd * pKeyItem = pInfo->GetControl(L"SFRM_KEY_ITEM");
	if( pKeyItem )
	{// 입장 아이템
		if( sub_iter->second.Enable )
		{
			pKeyItem->Visible(true);
			XUI::CXUI_Wnd * pIcon = pKeyItem->GetControl(L"ICN_ITEM");
			if( pIcon )
			{
				pIcon->ClearCustomData();
				if( sub_iter->second.Enable )
				{
					pIcon->SetCustomData(&sub_iter->second.Condition.ItemNo, sizeof(sub_iter->second.Condition.ItemNo));
				}
			}
		}
		else
		{
			pKeyItem->Visible(false);
		}
	}
	
	UpdateConstellationInfo_CashItem( pInfo, sub_iter->second.Enable ); // 캐쉬 입장 아이템

	XUI::CXUI_Wnd * pReward = pInfo->GetControl(L"FRM_REWARD");
	if( pReward )
	{// 보상 아이템
		if( sub_iter->second.Enable )
		{
			pReward->Visible(true);
			XUI::CXUI_Wnd * pItem = pReward->GetControl(L"SFRM_REWARD_ITEM");
			if( pItem )
			{
				XUI::CXUI_Wnd * pIcon = pItem->GetControl(L"ICN_ITEM");
				if( pIcon )
				{
					pIcon->ClearCustomData();
					if( sub_iter->second.Enable )
					{
						pIcon->SetCustomData(&sub_iter->second.Reward.ItemNo, sizeof(sub_iter->second.Reward.ItemNo));
					}
				}
			}
		}
		else
		{
			pReward->Visible(false);
		}
	}

	XUI::CXUI_Wnd * pMonster = pInfo->GetControl(L"SFRM_BOX_MONSTER");
	if( pMonster )
	{
		if( sub_iter->second.Enable )
		{
			pMonster->Visible(true);
			UpdateConstellationInfo_Monster( pMonster, sub_iter->second.MonsterNo );
		}
		else
		{
			pMonster->Visible(false);
		}
	}
}

void PgConstellationEnterUIMgr::UpdateRewardInfo()
{
	EnoughConsumeItem(GetPlayerRewardConsumItemCount(m_iNeedRewardConsumeItemNo, m_iNeedRewardConsumeItemCount, m_iHaveRewardConsumeItemCount));
	UpdateRewardBounsConsumeInfo();
}

void PgConstellationEnterUIMgr::ReqConstellationMission()
{
	if( NULL == g_pkWorld )
	{
		return;
	}

	if( false == g_pkWorld->IsHaveAttr(GKIND_CONSTELLATION|GKIND_MISSION) )
	{
		return;
	}

	BM::Stream Packet(PT_C_M_REQ_CONSTELLATION_MISSION);
	NETWORK_SEND(Packet);
}

void PgConstellationEnterUIMgr::ReqConstellationPartyList(XUI::CXUI_Wnd * pWnd)
{
	if( NULL == pWnd )
	{
		return;
	}

	XUI::CXUI_Wnd * pBtn = pWnd->GetControl(L"BTN_REFRESH");
	if( NULL == pBtn )
	{
		return;
	}

	BM::GUID KeyGuid;
	KeyGuid.Generate();

	pWnd->OwnerGuid(KeyGuid);

	m_TempCont.clear();

	XUI::CXUI_List * pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_PARTY"));
	if( pList )
	{
		pList->ClearList();
	}

	BM::Stream Packet(PT_C_M_REQ_CONSTELLATION_PARTY_LIST);
	Packet.Push(KeyGuid);
	Packet.Push(2); // Dungeon Type : 1-Default, 2-Constellation
	NETWORK_SEND(Packet);
}

void PgConstellationEnterUIMgr::FakeUpdateConstellationPartyList(XUI::CXUI_Wnd * pWnd)
{
	if( NULL == pWnd )
	{
		return;
	}

	XUI::CXUI_List * pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_PARTY"));
	if( NULL == pList )
	{
		return;
	}

	pList->ClearList();

	CONT_INDUN_PARTY_CHANNEL::const_iterator channel_it = m_TempCont.begin();
	while(channel_it != m_TempCont.end())
	{
		CONT_INDUN_PARTY::const_iterator party_it = channel_it->second.begin();
		while(party_it != channel_it->second.end())
		{
			AddItem_DungeonPartyList(pList, (*channel_it).first, (*party_it));
			++party_it;
		}
		++channel_it;
	}
}

void PgConstellationEnterUIMgr::OnRefresh_DungeonPartyList(BM::GUID const& KeyGuid, CONT_INDUN_PARTY_CHANNEL const& PartyList)
{
	XUI::CXUI_Wnd * pWnd = XUIMgr.Get(L"FRM_CONSTELLATION_DUNGEON");
	if( NULL == pWnd )
	{
		return;
	}

	XUI::CXUI_Wnd * pPartyWnd = pWnd->GetControl(L"FRM_CONSTELLATION_PARTY");
	if( (NULL == pPartyWnd) || (pPartyWnd->OwnerGuid() != KeyGuid) )
	{
		return;
	}

	XUI::CXUI_Wnd * pLoading = pPartyWnd->GetControl(L"FRM_LOADING");
	if( pLoading )
	{
		pLoading->Visible(false);
	}

	XUI::CXUI_List * pList = dynamic_cast<XUI::CXUI_List*>(pPartyWnd->GetControl(L"LST_PARTY"));
	if( NULL == pList )
	{
		return;
	}

	CONT_INDUN_PARTY_CHANNEL::const_iterator channel_it = PartyList.begin();
	while(channel_it != PartyList.end())
	{
		CONT_INDUN_PARTY IndunParty;
		CONT_INDUN_PARTY::const_iterator party_it = channel_it->second.begin();
		while(party_it != channel_it->second.end())
		{
			IndunParty.push_back( *party_it );

			AddItem_DungeonPartyList(pList, (*channel_it).first, (*party_it));
			++party_it;
		}
		
		CONT_INDUN_PARTY_CHANNEL::iterator temp_iter =  m_TempCont.find(channel_it->first);
		if( m_TempCont.end() != temp_iter )
		{
			CONT_INDUN_PARTY::const_iterator indun_iter = IndunParty.begin();
			while( IndunParty.end() != indun_iter )
			{
				temp_iter->second.push_back( *indun_iter );
				++indun_iter;
			}
		}
		else
		{
			m_TempCont.insert( std::make_pair( channel_it->first, IndunParty ) );
		}

		++channel_it;
	}
}

void PgConstellationEnterUIMgr::AddItem_DungeonPartyList(XUI::CXUI_List * pList, short const sChannel, SIndunPartyInfo const& rInfo)
{
	if( NULL == pList )
	{
		return;
	}

	if( rInfo.iNowUser >= rInfo.iMaxUser )
	{
		return;
	}

	if( rInfo.byPartyState )
	{
		return;
	}
	
	XUI::SListItem * pNewItem = pList->AddItem(rInfo.kPartyGuid.str());
	if( NULL == pNewItem )
	{
		return;
	}

	XUI::CXUI_Wnd * pListItem = pNewItem->m_pWnd;
	if( NULL == pListItem )
	{
		return;
	}

	BM::Stream Packet;
	Packet.Push(sChannel);
	rInfo.WriteToPacket(Packet);
	pListItem->SetCustomData(Packet);
	pListItem->OwnerGuid(rInfo.kMasterGuid);

	XUI::CXUI_Wnd * pMissionName = pListItem->GetControl(L"SFRM_PARTY_MISSION");
	if( pMissionName )
	{
		BM::vstring vStr;
		if( g_kNetwork.NowChannelNo() != sChannel )
		{
			BM::vstring vChannel(TTW(401273));
			vChannel.Replace(L"#NO#", sChannel);
			vStr += vChannel;
		}

		BM::vstring DiffiStr;
		GetString_Difficulty( rInfo.Difficulty, DiffiStr );
		if( DiffiStr.size() )
		{
			vStr += "[";
			vStr += DiffiStr;
			vStr += "]";
		}

		vStr += " ";

		BM::vstring PosStr;
		GetString_Position( rInfo.Position, PosStr );
		if( PosStr.size() )
		{
			vStr += PosStr;
		}

		Quest::SetCutedTextLimitLength(pMissionName, vStr, std::wstring(_T("..")));
	}

	XUI::CXUI_Wnd * pPartyName = pListItem->GetControl(L"SFRM_PARTY_NAME");
	if( pPartyName )
	{
		BM::vstring vStr;
		vStr = rInfo.kPartyTitle;
		Quest::SetCutedTextLimitLength(pPartyName, vStr, std::wstring(_T("..")));
	}

	XUI::CXUI_Wnd * pUser = pListItem->GetControl(L"SFRM_PARTY_PEOPLE");
	if( pUser )
	{
		BM::vstring vStr(TTW(401272));
		vStr.Replace(L"#NOW#", rInfo.iNowUser);
		vStr.Replace(L"#MAX#", rInfo.iMaxUser);
		pUser->Text(vStr);
	}
}

void PgConstellationEnterUIMgr::OnEnter_ConstellationParty(XUI::CXUI_Wnd * pPartyWnd)
{
	if( NULL == pPartyWnd )
	{
		return;
	}

	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pPartyWnd->GetControl(L"LST_PARTY"));
	if( NULL == pList )
	{
		return;
	}

	XUI::SListItem * pListItem = pList->FirstItem();
	while( pListItem )
	{
		XUI::CXUI_Wnd * pWnd = pListItem->m_pWnd;
		pWnd = pWnd ? pWnd->GetControl(L"SFRM_SELECT") : NULL;
		if( pWnd && pWnd->Visible() )
		{
			//아이템의 GetCustomData에서 패킷을 뽑아내고 해당 하는 채널과 파트장에게 요청을 한다.
			BM::Stream ReadPacket;
			pListItem->m_pWnd->GetCustomData(ReadPacket);

			short sChannel = 0;
			SIndunPartyInfo PartyInfo;
			ReadPacket.Pop(sChannel);
			PartyInfo.ReadFromPacket(ReadPacket);

			int NeedItemNo = 0;
			CONT_NEED_KEY_ITEM KeyItem, CashKeyItem;
			if( false == PgClientPartyUtil::IsCanPartyBreakIn(PartyInfo.iMasterMapNo, KeyItem, CashKeyItem) )
			{
				if( PgClientPartyUtil::IsHaveKeyItemPartyBreakIn( KeyItem, CashKeyItem, NeedItemNo ) )
				{
					std::wstring ItemName;
					if( GetItemName(NeedItemNo, ItemName) )
					{
						BM::vstring msg(TTW(700415));
						msg.Replace(L"#ITEM#", ItemName);
						lwCallYesNoMsgBox((std::wstring)msg, PartyInfo.kMasterGuid, MBT_CONFIRM_CONSUME_ITEM_PARTY_BREAK_IN, (int)sChannel);
					}
				}
				else
				{
					std::wstring ItemName;
					if( GetItemName(NeedItemNo, ItemName) )
					{
						BM::vstring msg(TTW(700416));
						msg.Replace(L"#ITEM#", ItemName);
						lwAddWarnDataStr(lwWString((std::wstring)msg), 1, true);
					}
				}
				return;
			}
			
			BM::Stream Packet(PT_C_M_REQ_INDUN_PARTY_ENTER);
			Packet.Push(sChannel);
			Packet.Push(PartyInfo.kMasterGuid);
			NETWORK_SEND(Packet);
			return;
		}
		pListItem = pList->NextItem(pListItem);
	}
}
void PgConstellationEnterUIMgr::UpdateConstellationInfo_Story(XUI::CXUI_Wnd * pWnd ,int const Difficulty,int const TextNo)
{
	
	if( NULL == pWnd )
	{
		return;
	}

	XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_CONSTELLATION_STORY"));
	if( NULL == pkList )
	{
		return;		
	}

	pkList->ClearList();
	XUI::SListItem* pkItem = pkList->AddItem(L"");
	
	BM::vstring vStr(TTW(TextNo));

	pkItem->m_pWnd->Text(std::wstring(vStr));
	XUI::CXUI_Style_String kString = pkItem->m_pWnd->StyleText();
	const POINT kCalcSize = Pg2DString::CalculateOnlySize(kString);
	pkItem->m_pWnd->Size(pkItem->m_pWnd->Size().x,kCalcSize.y +2);
}

void PgConstellationEnterUIMgr::UpdateConstellationInfo_Name(XUI::CXUI_Wnd * pWnd, int const Difficulty, int const TextNo)
{
	if( NULL == pWnd )
	{
		return;
	}

	BM::vstring vStr(TTW(TextNo));
	vStr += "(";
	switch( Difficulty )
	{
	case 1:
		{
			vStr += TTW(3315);
		}break;
	case 2:
		{
			vStr += TTW(403278);
		}break;
	case 3:
		{
			vStr += TTW(3031);
		}break;
	case 4:
		{
			vStr += TTW(50304);
		}break;
	}
	vStr += ")";
	pWnd->Text(std::wstring(vStr));
}

void PgConstellationEnterUIMgr::UpdateConstellationInfo_Day(XUI::CXUI_Wnd * pWnd, VEC_WSTRING const& DayofWeek,int const Position, int const Difficulty)
{
	if( NULL == pWnd )
	{
		return;
	}

	pWnd->Text(L"");

	if( NULL == g_pkWorld )
	{
		return;
	}

	CONT_CONSTELLATION::const_iterator iter = m_ContEnter.find(g_pkWorld->MapNo());
	if( iter == m_ContEnter.end() )
	{
		return;
	}

	ConstellationKey Key( Position,Difficulty);

	CONT_CONSTELLATION_ENTER::const_iterator sub_iter = iter->second.find(Key);
	if( iter->second.end() == sub_iter )
	{
		return;
	}
	SYSTEMTIME sysTime;
	g_kEventView.GetLocalTime(&sysTime);

	SDAYOFWEEK dayOfWeek(EDOW_NONE);
	dayOfWeek.SetDayofWeek(sub_iter->second.Condition.DayofWeek);
	
	BM::vstring vStr;
	BM::vstring vstrTemp( TTW(11010 + sysTime.wDayOfWeek) );
	BM::vstring vSatSun;

	GetString_DayofWeek(DayofWeek, vStr);
	int TotalCount = DayofWeek.size();
	int NowDay = eDayOfWeekIdx[sysTime.wDayOfWeek];
	if(2 == TotalCount)
	{//토요일 일요일일 경우엔 "/"표시로 인해서 지금 작업을 한다.
		//오늘날짜가 토요일이나 일요일이면.허연색 아니면 빨간색 
		if(EDOW_FRI < NowDay)
		{
			vstrTemp = vSatSun;
			vstrTemp += vStr;
		}
	}

	if( vstrTemp == vStr)
	{
		pWnd->Text(std::wstring(vStr));
	}
	else
	{
		BM::vstring vStrRed(TTW(89999));
		vStrRed += vStr;
		pWnd->Text(std::wstring(vStrRed));
	}
}

void PgConstellationEnterUIMgr::UpdateConstellationInfo_Level(XUI::CXUI_Wnd * pWnd, int const MinLevel, int const MaxLevel)
{
	if( NULL == pWnd )
	{
		return;
	}

	BM::vstring vStr;
	vStr = MinLevel; // 입장 최소 레벨
	vStr += " ~ ";
	vStr += MaxLevel; // 입장 최대 레벨

	pWnd->Text((std::wstring)vStr);
}

void PgConstellationEnterUIMgr::UpdateConstellationInfo_Quest(XUI::CXUI_Wnd * pWnd, int const QuestNo)
{
	if( NULL == pWnd )
	{
		return;
	}

	PgQuestInfo const* pQuestInfo = g_kQuestMan.GetQuest( QuestNo );
	if( NULL == pQuestInfo )
	{
		return;
	}

	CONT_DEF_QUEST_REWARD const* pDefQuestReward = NULL;
	g_kTblDataMgr.GetContDef(pDefQuestReward);
	if( NULL == pDefQuestReward )
	{
		return;
	}

	CONT_DEF_QUEST_REWARD::const_iterator quest_iter = pDefQuestReward->find(QuestNo);
	if( pDefQuestReward->end() == quest_iter )
	{
		return;
	}

	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return;
	}

	PgMyQuest const* pMyQuest = pPlayer->GetMyQuest();
	if( NULL == pMyQuest )
	{
		return;
	}

	BM::vstring vStr;
	if( (false == pMyQuest->IsEndedQuest(QuestNo)) && (false == pMyQuest->IsIngQuest(QuestNo)) )
	{
		vStr += TTW(89999);
	}
	vStr += TTW(pQuestInfo->m_iTitleTextNo);
	if( g_pkApp->VisibleClassNo() )
	{
		vStr += L" (";
		vStr += QuestNo;
		vStr += L")";
	}

	pWnd->Text((std::wstring)vStr);
}

int PgConstellationEnterUIMgr::CheckCashItemCount(int & firstItemNo)
{
	CONT_CONSTELLATION_ENTRANCE_CASH_ITEM::const_iterator iter = m_EntranceCashItem.find(GetNowGroundNo());
	if( iter == m_EntranceCashItem.end() )
	{
		return 0;
	}

	if( iter->second.ItemNo.empty() )
	{
		return 0;
	}
	
	firstItemNo = *(iter->second.ItemNo.begin());

	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return 0;
	}

	PgInventory * pInv = pPlayer->GetInven();
	if( NULL == pInv )
	{
		return 0;
	}
	
	int NeedItemCount = 1;
	if( PgClientPartyUtil::IsInParty() )
	{// 파티에 속해 있는가?
		if( PgClientPartyUtil::IsPartyMaster(pPlayer->GetID()) )
		{// 파티장인가?
			if( 3 <= g_kParty.MemberCount() )
			{
				NeedItemCount = 2;
			}
		}
		else
		{
			return 0;
		}
	}

	int CashItemNo = 0;
	int ContSize = iter->second.ItemNo.size() - 1;
	while( ContSize >= 0 )
	{
		int CashItemCount = pInv->GetTotalCount(iter->second.ItemNo.at(ContSize));
		if( NeedItemCount <= CashItemCount )
		{
			CashItemNo = iter->second.ItemNo.at(ContSize);
			break;
		}
		--ContSize;
	}

	return CashItemNo;
}

void PgConstellationEnterUIMgr::UpdateConstellationInfo_CashItem(XUI::CXUI_Wnd * pWnd, bool const enable)
{
	if( NULL == pWnd )
	{
		return;
	}

	XUI::CXUI_Wnd * deregWnd = pWnd->GetControl(L"BTN_DEREG");
	XUI::CXUI_Wnd * regWnd = pWnd->GetControl(L"BTN_REG");
	XUI::CXUI_Wnd * pCashItemWnd = pWnd->GetControl(L"SFRM_CASH_KEY_ITEM");
	XUI::CXUI_Wnd * pText = pWnd->GetControl(L"FRM_TEXT_CASH_ITEM");
	if( NULL == pCashItemWnd || NULL == deregWnd || NULL == regWnd || NULL == pText )
	{
		return;
	}

	pCashItemWnd->Visible(enable);

	if( false == enable )
	{
		deregWnd->Visible(enable);
		regWnd->Visible(enable);
		pText->Visible(enable);
		return;
	}

	XUI::CXUI_Wnd * pIcon = pCashItemWnd->GetControl(L"ICN_ITEM");
	if( NULL == pIcon )
	{
		return;
	}

	if( m_CashItemNo )
	{
		pIcon->SetCustomData(&m_CashItemNo, sizeof(m_CashItemNo));
		pIcon->GrayScale(false);
		regWnd->Visible(false);
		deregWnd->Visible(true);
		return;
	}

	pIcon->ClearCustomData();

	int FirstItemNo = 0;
	if( 0 < CheckCashItemCount(FirstItemNo) )
	{
		pText->Visible(true);
		regWnd->Visible(true);
	}
	else
	{
		pText->Visible(false);
		regWnd->Visible(false);
	}

	pIcon->SetCustomData(&FirstItemNo, sizeof(FirstItemNo));
	pIcon->GrayScale(true);
	deregWnd->Visible(false);
}

void PgConstellationEnterUIMgr::UpdateConstellationInfo_Monster(XUI::CXUI_Wnd * pWnd, VEC_INT const& ContMonsterNo)
{
	if( NULL == pWnd )
	{
		return;
	}

	XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pWnd->GetControl(L"BLD_MON"));
	int const BuildCount = pBuild ? pBuild->CountX() : 0;
	if( BuildCount <= 0 )
	{
		return;
	}

	GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
	CMonsterDef const* pDefMonster = NULL;

	PgPilotMan::stClassInfo ClassInfo;
	VEC_INT::const_iterator c_it = ContMonsterNo.begin();

	BM::vstring vStr;
	for( int i = 0; i < BuildCount; ++i )
	{
		vStr = (L"ICN_MON");
		vStr += i;

		CXUI_Icon* pIcon = dynamic_cast<CXUI_Icon*>(pWnd->GetControl(vStr));
		if( pIcon )
		{
			bool bVisible = false;

			if( c_it != ContMonsterNo.end() )
			{
				int const MonsterNo = (*c_it);
				++c_it;

				pDefMonster = kMonsterDefMgr.GetDef(MonsterNo);
				if(pDefMonster)
				{
					int const IconNo = pDefMonster->GetAbil(AT_DEF_RES_NO);
					if(IconNo)
					{
						bVisible = true;
						pIcon->SetCustomData(&IconNo, sizeof(IconNo));
						
						SIconInfo IconInfo = pIcon->IconInfo();
						IconInfo.iIconKey = MonsterNo;
						pIcon->SetIconInfo(IconInfo);
					}
				}
			}
			pIcon->Visible(bVisible);
		}
	}
}

bool PgConstellationEnterUIMgr::GetConstellationPartyListToolTip(BM::GUID const& guid, std::wstring & TooltipStr)
{
	TooltipStr = _T("");

	CONT_INDUN_PARTY_CHANNEL::const_iterator iter = m_TempCont.begin();
	while( m_TempCont.end() != iter )
	{
		CONT_INDUN_PARTY::const_iterator sub_iter = iter->second.begin();;
		while( iter->second.end() != sub_iter )
		{
			if( guid == sub_iter->kMasterGuid )
			{
				TooltipStr = TTW(401350);
				TooltipStr += L"\n";
				TooltipStr += sub_iter->kPartyTitle;
				TooltipStr += L"\n";
				// 파티장
				BM::vstring Message( TTW(401354) );
				Message.Replace(L"$CLASS$", PgClientPartyUtil::GetClassNameTTW(sub_iter->iMasterClass));
				Message.Replace(L"$CHAR_NAME$", sub_iter->kMasterName);
				Message.Replace(L"$LEVEL$", sub_iter->iMasterLevel);
				TooltipStr += (std::wstring)Message;
				TooltipStr += L"\n";

				TooltipStr += TTW(401359);
				TooltipStr += L" ";

				int const iFilterItem = POI_SafeMask ^ (POE_SafeMask | POI_SafeMask);
				EPartyOptionItem eType = (EPartyOptionItem)(sub_iter->iPartyOption & iFilterItem);			
				if( eType == POI_LikeSolo )//Item
				{
					TooltipStr += TTW(401008);
				}
				else if( eType == POI_Order )
				{
					TooltipStr += TTW(401009);
				}
				TooltipStr += L"\n";

				if( !sub_iter->kPartySubName.empty() )
				{
					TooltipStr += TTW(401353);
					TooltipStr += L" ";
					TooltipStr += sub_iter->kPartySubName;
				}

				return true;
			}
			++sub_iter;
		}
		++iter;
	}
	return false;
}