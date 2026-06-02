#include "stdafx.h"
#include "Variant/TableDataManager.h"
#include "Variant/PgJobSkill.h"
#include "ServerLib.h"
#include "lwJobSkillItem.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgMoveAnimateWnd.h"
#include "PgChatMgrClient.h"
#include "PgUIScene.h"
#include "PgUIModel.h"
#include "PgUISound.h"
#include "lwUIQuest.h"

extern void  lwSetReqInvWearableUpdate(bool bValue);

namespace lwJobSkillItem
{
	struct SortSaveIdxKey
	{
		bool operator < ( SortSaveIdxKey const& rhs )const
		{
			if( iNeedSkillNo < rhs.iNeedSkillNo )	{return true; }
			if( iNeedSkillNo > rhs.iNeedSkillNo )	{return false;}

			if( iNeedSkillExpertness < rhs.iNeedSkillExpertness )	{return true; }
			if( iNeedSkillExpertness > rhs.iNeedSkillExpertness )	{return false;}

			if( iNo < rhs.iNo )	{return true; }
			if( iNo > rhs.iNo )	{return false;}

			return false;
		}
		int iNo;
		int iNeedSkillNo;
		int iNeedSkillExpertness;
	};

	typedef std::map<SortSaveIdxKey, SJobSkillSaveIdx> CONT_SORT_JOBSKILL_SAVEIDX;


	//메인
	std::wstring const JOBSKILL_MAIN(L"SFRM_JOB_SKILL_ITEM");
	//종류 선택
	std::wstring const JOB_SELECT_SHADOW(L"SFRM_JOB_SELECT_SHADOW");
	std::wstring const JOB_SELECT(L"BTN_JOB_SELECT");
	//종류 리스트
	std::wstring const JOB_SELECT_ITEMTYPE(L"SFRM_SELECT_ITEMTYPE_LIST");
	std::wstring const JOB_SELECT_ITEMTYPE_LIST(L"LST_SELECT_TYPE");
		
	//배울 수 있는 직업 목록
	std::wstring const ITEM_SHADOW(L"SFRM_ITEM_SHADOW");
	std::wstring const ITEM_MAIN(L"FRM_ITEM");
	std::wstring const ITEM_ICON(L"SFRM_ICON");
	std::wstring const ITEM_TEXT(L"SFRM_TEXT");
	std::wstring const ITEM_TEXT_TITLE(L"FRM_TEXT_TITLE");
	std::wstring const ITEM_TEXT_INFO(L"FRM_TEXT_INFO");
	//페이지 넘김
	std::wstring const BTN_PAGE_L(L"BTN_PAGE_L");
	std::wstring const FRM_PAGE_BG(L"FRM_PAGE_BG");
	std::wstring const BTN_PAGE_R(L"BTN_PAGE_R");
	
	//3차
	std::wstring const JOBSKILL3_ITEM_CREATE(L"SFRM_JL3_ITEM_CREATE");
	std::wstring const JOBSKILL3_ITEM_SELECT(L"SFRM_JL3_ITEM_SELECT");
	std::wstring const ITEM_LIST(L"BLD_ITEM_LIST");
	BM::vstring  const JOBSKILL3_TRG_SLOT(L"FMA_JS3_TRG_SLOT");

	std::wstring const CBTN_FILTER_ONLY_LEARN(L"CBTN_FILTER_ONLY_LEARN");

	int const MAX_SHOW_LIST = 6;

	int g_kCurrentPage = 1;
	int g_kCurrentGatherType = 0;
	bool g_bJS3_CreateItemSend = false;

	//3차 함수선언
	void lwSetJobSkill3CreateItem(lwUIWnd kWnd, int const iSaveIdx);
	void lwOnHistoryItemChangePage(lwUIWnd kWnd, int const iCalc);
	void lwUpdateJobSkill3NeedUI(lwUIWnd kWnd, int const iItemNo);
	bool lwJS3_NextPage();
	bool lwJS3_BeforePage();
	void lwCallJobSkill3ItemUI();
	void lwCloseJobSkill3ItemUI();
	void lwJS3_SetGatherType(std::wstring const &kFilterTitle, int const iGatherType);
	void lwJS3_DoActionCreateItem(lwUIWnd kWnd);
	void lwJS3_UpdateSendCreateItem(lwUIWnd kWnd);
	bool lwIsJobSkill3Learn();
	void lwJS3_Tick(lwUIWnd kWnd);
	void lwJS3_FullResItemSlot(lwUIWnd kWnd);
	void lwJS3_ClearResItemSlot(lwUIWnd kWnd);
	void lwJS3_SetResItemSlot(lwUIWnd kWnd, bool const bSet);
	void JS3_RefreshResItem();
	bool lwJS3_ShowList(int iPage, int const iGatherType );
	void lwJS3_ShowSelectItemList();
	void lwJS3_SetFilterOnlyLearn(bool const bCheck);

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "CallJobSkillItemUI", lwCallJobSkillItemUI);
		def(pkState, "JobSkillItem_NextPage", lwNextPage);
		def(pkState, "JobSkillItem_BeforePage", lwBeforePage);
		def(pkState, "CloseJobSkillItemUI", lwCloseJobSkillItemUI);
		def(pkState, "SetGatherType", lwSetGatherType);
		def(pkState, "SetJobSkill3CreateItem", lwSetJobSkill3CreateItem);
		def(pkState, "OnHistoryItemChangePage", lwOnHistoryItemChangePage);
		def(pkState, "UpdateJobSkill3NeedUI", lwUpdateJobSkill3NeedUI);
		def(pkState, "JobSkillItem3_NextPage", lwJS3_NextPage);
		def(pkState, "JobSkillItem3_BeforePage", lwJS3_BeforePage);
		def(pkState, "CallJobSkill3ItemUI", lwCallJobSkill3ItemUI);
		def(pkState, "CloseJobSkill3ItemUI", lwCloseJobSkill3ItemUI);
		def(pkState, "JobSkillItem3_SetGatherType", lwJS3_SetGatherType);
		def(pkState, "JobSkillItem3_DoActionCreateItem", lwJS3_DoActionCreateItem);
		def(pkState, "JobSkillItem3_UpdateSendCreateItem", lwJS3_UpdateSendCreateItem);
		def(pkState, "IsJobSkill3Learn", lwIsJobSkill3Learn);
		def(pkState, "JobSkillItem3_SelectMakeItem", lwJS3_SelectMakeItem);
		def(pkState, "JobSkillItem3_Tick", lwJS3_Tick);
		def(pkState, "JobSkillItem3_FullResItemSlot", lwJS3_FullResItemSlot);
		def(pkState, "JobSkillItem3_ClearResItemSlot", lwJS3_ClearResItemSlot);
		def(pkState, "JobSkillItem3_SetResItemSlot", lwJS3_SetResItemSlot);
		def(pkState, "LearnFilter_JobSkillItem", lwLearnFilter_JobSkillItem);
		def(pkState, "JobSkillItem3_ShowSelectItemList", lwJS3_ShowSelectItemList);
		def(pkState, "JobSkillItem3_FilterOnlyLearn", lwJS3_SetFilterOnlyLearn);
	}
	void lwCallJobSkillItemUI()
	{
		if( true == lwShowList(g_kCurrentPage, g_kCurrentGatherType))
		{
			SetFilterItemType();
		}
	}
	bool lwNextPage()
	{
		bool const bViewOnlyLearn = IsOnlyLearnView();
		if(!lwShowList(g_kCurrentPage + 1, g_kCurrentGatherType, bViewOnlyLearn))
		{
			lwCloseJobSkillItemUI();
			return false;
		}
		return true;
	}
	bool lwBeforePage()
	{
		bool const bViewOnlyLearn = IsOnlyLearnView();
		if( 1 == g_kCurrentPage)
		{
			++g_kCurrentPage;
		}
		if(!lwShowList(g_kCurrentPage - 1, g_kCurrentGatherType, bViewOnlyLearn))
		{
			lwCloseJobSkillItemUI();
			return false;
		}
		return true;
	}
	void lwSetGatherType(std::wstring const &kFilterTitle, int const iGatherType)
	{
		SetFilterTitle(kFilterTitle);
		g_kCurrentGatherType = iGatherType;
		lwClearAll();
		bool const bViewOnlyLearn = IsOnlyLearnView();
		lwShowList(g_kCurrentPage, g_kCurrentGatherType, bViewOnlyLearn);
	}
	void SetFilterTitle(std::wstring const& kTitle)
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(JOBSKILL_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pFilter = pMain->GetControl(JOB_SELECT_SHADOW);
		if(!pFilter)
		{
			return;
		}
		pFilter->Text(kTitle);
	}
	void SetFilterItemType()
	{//드랍다운 메뉴 설정
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(JOBSKILL_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pSelectShadow = pMain->GetControl(JOB_SELECT_ITEMTYPE);
		if(!pSelectShadow)
		{
			return;
		}
		XUI::CXUI_Wnd* pSelectWnd = pSelectShadow->GetControl(JOB_SELECT_ITEMTYPE_LIST);
		if(!pSelectWnd)
		{
			return;
		}
		XUI::CXUI_List* pSelectList = dynamic_cast<XUI::CXUI_List*>(pSelectWnd);
		if(!pSelectList)
		{
			return;
		}
		pSelectList->ClearList();
		CONT_DEF_JOBSKILL_SKILL const* pkContDefJobSkill;
		g_kTblDataMgr.GetContDef(pkContDefJobSkill);
		if(!pkContDefJobSkill->size())
		{
			return;
		}
		//전체 보기 미리 추가
		XUI::SListItem* pItem = pSelectList->AddItem(L"");
		if(pItem)
		{
			pItem->m_pWnd->Text( TTW(799501) );
			int const iJobSkillType = 0;
			pItem->m_pWnd->SetCustomData(&iJobSkillType, sizeof(iJobSkillType));
		}
		PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return;
		}
		CONT_DEF_JOBSKILL_SAVEIDX const* pkContDefSaveIdx;
		g_kTblDataMgr.GetContDef(pkContDefSaveIdx);
		//1. 도감 전체 컨테이너 구하고
		if(!pkContDefSaveIdx->size())
		{
			return;
		}
		//2. 하나씩 돌면서 임시 컨테이너에 사용 가능한 도감의 메인스킬 번호만 넣고
		CONT_DEF_JOBSKILL_SKILL kContLearnSkill;
		CONT_DEF_JOBSKILL_SAVEIDX::const_iterator saveIdx_iter = pkContDefSaveIdx->begin();
		while( saveIdx_iter != pkContDefSaveIdx->end())
		{
			int const iSaveIndex = (*saveIdx_iter).first;
			CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIdx = (*saveIdx_iter).second;

			if( JobSkillSaveIdxUtil::IsUseableSaveIdx(pPlayer, iSaveIndex) )
			{//스킬도 배운 상태라면
				CONT_DEF_JOBSKILL_SKILL::const_iterator temp_iter = pkContDefJobSkill->find(rkSaveIdx.iNeedSkillNo01);
				if(pkContDefJobSkill->end() != temp_iter)
				{//임시컨테이너에 없는 스킬이면 추가
					kContLearnSkill.insert( std::make_pair((*temp_iter).first, (*temp_iter).second));
				}
			}
			++saveIdx_iter;
		}
		//3. 다음에 전체 돌면서 안배운거 넣어주기
		CONT_DEF_JOBSKILL_SKILL::const_iterator temp_iter = kContLearnSkill.begin();
		while(kContLearnSkill.end() != temp_iter)
		{
			int const iJobSkillNo = (*temp_iter).first;
			CONT_DEF_JOBSKILL_SKILL::mapped_type const &rkJobSkillInfo = (*temp_iter).second;
			if( (JSTT_ALL != rkJobSkillInfo.iGatherType) 
				&&  (JST_1ST_MAIN == rkJobSkillInfo.eJobSkill_Type || 
				JST_3RD_MAIN == rkJobSkillInfo.eJobSkill_Type) )
			{//주스킬의 경우만 받아서 Add하기

				InsertFilterItem(pSelectList, iJobSkillNo, rkJobSkillInfo, false);
			}
			++temp_iter;
		}

		CONT_DEF_JOBSKILL_SKILL::const_iterator iter_Job = pkContDefJobSkill->begin();
		while( iter_Job != pkContDefJobSkill->end() )
		{
			int const iJobSkillNo = (*iter_Job).first;
			CONT_DEF_JOBSKILL_SKILL::mapped_type const &rkJobSkillInfo = (*iter_Job).second;

			CONT_DEF_JOBSKILL_SKILL::const_iterator temp_iter = kContLearnSkill.find(iJobSkillNo);
			if(kContLearnSkill.end() == temp_iter)
			{//임시컨테이너에서 넣었던 스킬이랑 안겹치도록
				if( (JSTT_ALL != rkJobSkillInfo.iGatherType) 
				&&  (JST_1ST_MAIN == rkJobSkillInfo.eJobSkill_Type || 
					 JST_3RD_MAIN == rkJobSkillInfo.eJobSkill_Type) )
				{//주스킬의 경우만 받아서 Add하기
					InsertFilterItem(pSelectList, iJobSkillNo, rkJobSkillInfo, true);
				}
			}
			++iter_Job;
		}
	}

	void InsertFilterItem(XUI::CXUI_List* pSelectList, int iJobSkillNo, CONT_DEF_JOBSKILL_SKILL::mapped_type const &rkJobSkillInfo, bool const bGray)
	{
		if(!pSelectList)
		{
			return;
		}
		wchar_t const* pName = NULL;
		if(!GetDefString(iJobSkillNo ,pName))
		{
			return;
		}
		BM::vstring kTitle;
		SetFilterText(kTitle, rkJobSkillInfo.eJobSkill_Type);
		kTitle += BM::vstring(pName);
		XUI::SListItem* pItem = pSelectList->AddItem(L"");
		if(pItem)
		{
			pItem->m_pWnd->Text(kTitle);
			int const iGatherType = rkJobSkillInfo.iGatherType;
			pItem->m_pWnd->SetCustomData(&iGatherType, sizeof(iGatherType));
			pItem->m_pWnd->GrayScale(bGray);
		}
	}

	bool lwShowList(int iPage, int const iGatherType, bool const bShowOnlyLearn )
	{//아이템 목록 보여주기
		CONT_DEF_JOBSKILL_SAVEIDX kContSaveIndex;
		bool bFilterRet = FilterGatherType(kContSaveIndex, iGatherType);
		if(!bFilterRet)
		{
			return false;
		}
		if( bShowOnlyLearn)
		{
			FilterOnlyLearn(kContSaveIndex);
		}
		
		//배운아이템 따로 가져오기
		CONT_DEF_JOBSKILL_SAVEIDX kContLearned;
		GetLearnedSaveIndex(kContSaveIndex, kContLearned);

		//페이지 설정
		int iShowItemCount = 0;
		if( !bShowOnlyLearn)
		{
			iShowItemCount = kContSaveIndex.size();
		}
		else 
		{
			iShowItemCount = kContLearned.size();
		}

		int iMaxPage = iShowItemCount / MAX_SHOW_LIST;
		if( 0 != (iShowItemCount % MAX_SHOW_LIST) )
		{
			++iMaxPage;
		}
		if(  1 > iPage || 0 == iMaxPage)
		{//페이지 없으면....아이템 안들어가 잇는거
			return false;
		}
		if( iMaxPage < iPage )
		{
			iPage = iMaxPage;
		}

		XUI::CXUI_Wnd* pMain = XUIMgr.Activate(JOBSKILL_MAIN);
		if(!pMain)
		{
			return false;
		}
		XUI::CXUI_Wnd* pItemSD = pMain->GetControl(ITEM_SHADOW);
		if(!pItemSD)
		{
			return false;
		}
		//현재 정렬 타입 텍스트 설정
		lwClearAll();
		BM::vstring kFilterText;
		//현재 보여지는 페이지 설정
		SetPageUI(iPage, iMaxPage);
		
		//지정된 페이지의 스킬 목록 설정
		CONT_DEF_JOBSKILL_SAVEIDX::const_iterator itor_LearnedSaveIndex = kContLearned.begin();
		CONT_DEF_JOBSKILL_SAVEIDX::const_iterator itor_SaveIndex = kContSaveIndex.begin();
		int iPageCount = 0;
		while( iPageCount < ( (iPage-1)*MAX_SHOW_LIST) )
		{//지정된 페이지로 이동
			
			if( itor_LearnedSaveIndex != kContLearned.end() )
			{ //배운 도감 먼저 넘기고,
				++itor_LearnedSaveIndex;
				++iPageCount;	
				continue;
			}
			if( itor_SaveIndex == kContSaveIndex.end() )
			{ //
				break;
			}
			++itor_SaveIndex;
			++iPageCount;
		}
		//페이지 출력
		int iListCount = 0;
		while( iListCount < MAX_SHOW_LIST )
		{
			if( 0 != kContLearned.size() && itor_LearnedSaveIndex != kContLearned.end())
			{//배운 아이템 먼저 출력하기
				CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIndexInfo = (*itor_LearnedSaveIndex).second;
				BM::vstring kItem = BM::vstring(ITEM_MAIN) + iListCount;
				XUI::CXUI_Wnd* pItem = pItemSD->GetControl(kItem);
				if(!pItem)
				{
					return false;
				}
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pDef = kItemDefMgr.GetDef(rkSaveIndexInfo.iBookItemNo);
				if(pDef)
				{
					SetIconInfo(pItem, pDef, rkSaveIndexInfo.iSaveIdx);
					XUI::CXUI_Wnd* pTitle = pItem->GetControl(ITEM_TEXT);
					if(pTitle)
					{
						SetTitleText(pTitle, rkSaveIndexInfo.iBookItemNo);
						SetInfoText(pTitle, rkSaveIndexInfo);
					}
				}
				++iListCount;
				++itor_LearnedSaveIndex;
				continue;
			}

			{// 안배운 아이템 출력하기
				if(itor_SaveIndex == kContSaveIndex.end())
				{
					break;
				}
				if(kContLearned.end() == kContLearned.find((*itor_SaveIndex).first))
				{//현재 출력하려는게 배운아이템에 있으면 패스
					CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIndexInfo = (*itor_SaveIndex).second;
					
					BM::vstring kItem = BM::vstring(ITEM_MAIN) + iListCount;
					XUI::CXUI_Wnd* pItem = pItemSD->GetControl(kItem);
					if(!pItem)
					{
						return false;
					}
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pDef = kItemDefMgr.GetDef(rkSaveIndexInfo.iBookItemNo);
					if(pDef)
					{
						SetIconInfo(pItem, pDef, rkSaveIndexInfo.iSaveIdx);
						XUI::CXUI_Wnd* pTitle = pItem->GetControl(ITEM_TEXT);
						if(pTitle)
						{
							SetTitleText(pTitle, rkSaveIndexInfo.iBookItemNo);
							SetInfoText(pTitle, rkSaveIndexInfo);
						}
					}
					++iListCount;
				}
				++itor_SaveIndex;
			}
		}
		g_kCurrentPage = iPage;
		return true;
	}
	
	bool GetLearnedSaveIndex(CONT_DEF_JOBSKILL_SAVEIDX& rkContSaveIndex, CONT_DEF_JOBSKILL_SAVEIDX& rkOutCont)
	{// 사용 가능한 아이템만 따로 저장
		PgPlayer const * pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return false;
		}
		
		CONT_DEF_JOBSKILL_SAVEIDX::const_iterator iter_SaveIdx = rkContSaveIndex.begin();
		while(iter_SaveIdx != rkContSaveIndex.end() )
		{
			CONT_DEF_JOBSKILL_SAVEIDX::key_type const &rkSaveIdx = (*iter_SaveIdx).first;
			CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIdxInfo = (*iter_SaveIdx).second;
			

			if( JobSkillSaveIdxUtil::IsUseableSaveIdx(pkPlayer, rkSaveIdx) )
			{
				rkOutCont.insert( std::make_pair(rkSaveIdx, rkSaveIdxInfo) );
			}
			++iter_SaveIdx;
		}
		return true; 
	}

	bool GetLearnedSaveIndex(CONT_SORT_JOBSKILL_SAVEIDX& rkContSortSaveIdx, CONT_DEF_JOBSKILL_SAVEIDX& rkOutCont)
	{// 사용 가능한 아이템만 따로 저장
		PgPlayer const * pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return false;
		}
		
		CONT_SORT_JOBSKILL_SAVEIDX::const_iterator iter_SaveIdx = rkContSortSaveIdx.begin();
		while(iter_SaveIdx != rkContSortSaveIdx.end() )
		{
			CONT_SORT_JOBSKILL_SAVEIDX::key_type const &rkSaveIdx = (*iter_SaveIdx).first;
			CONT_SORT_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIdxInfo = (*iter_SaveIdx).second;
			if(  JobSkillSaveIdxUtil::IsUseableSaveIdx(pkPlayer, rkSaveIdx.iNo) )
			{
				rkOutCont.insert( std::make_pair(rkSaveIdx.iNo, rkSaveIdxInfo) );
			}
			++iter_SaveIdx;
		}
		return true; 
	}
	
	bool FilterGatherType(CONT_DEF_JOBSKILL_SAVEIDX& rkContSaveIndex, int const iGatherType)
	{
		CONT_DEF_JOBSKILL_SAVEIDX const* pkContDefSaveIndex = NULL;
		g_kTblDataMgr.GetContDef(pkContDefSaveIndex);
		if(!pkContDefSaveIndex)
		{
			return false;
		}
		if( 0 == pkContDefSaveIndex->size())
		{
			return false;
		}

		CONT_DEF_JOBSKILL_SKILL const* pkContDefJobSkill = NULL;
		g_kTblDataMgr.GetContDef(pkContDefJobSkill);
		if( 0 == pkContDefJobSkill->size())
		{
			return false;
		}
		CONT_DEF_JOBSKILL_SAVEIDX::const_iterator iter_SaveIndex = pkContDefSaveIndex->begin();
		while(iter_SaveIndex != pkContDefSaveIndex->end() )
		{
			CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkJobSkillInfo = (*iter_SaveIndex).second;

			int const iCompType = JobSkillUtil::GetGatherType(rkJobSkillInfo.iNeedSkillNo01, *pkContDefJobSkill);
			if((0 == iGatherType) || (iGatherType == iCompType))
			{
				if( 0 != rkJobSkillInfo.iShowUser )
				{
					rkContSaveIndex.insert(std::make_pair((*iter_SaveIndex).first, (*iter_SaveIndex).second ) );
				}
			}
			++iter_SaveIndex;
		}
		return true;
	}

	bool FilterType(CONT_DEF_JOBSKILL_SAVEIDX& rkContSaveIndex, int const iGatherType, EJobSkillType const eJobSkillType)
	{
		CONT_DEF_JOBSKILL_SKILL const* pkDefJSSkill = NULL;
		g_kTblDataMgr.GetContDef(pkDefJSSkill);
		if( !pkDefJSSkill )
		{
			return false;
		}

		if( FilterGatherType(rkContSaveIndex, iGatherType) )
		{
			CONT_DEF_JOBSKILL_SAVEIDX::iterator iter_SaveIndex = rkContSaveIndex.begin();
			while(iter_SaveIndex != rkContSaveIndex.end() )
			{
				CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkJobSkillInfo = (*iter_SaveIndex).second;

				EJobSkillType const eCompType = JobSkillUtil::GetJobSkillType(rkJobSkillInfo.iNeedSkillNo01, *pkDefJSSkill);
				if(eJobSkillType != eCompType)
				{
					iter_SaveIndex = rkContSaveIndex.erase(iter_SaveIndex);
				}
				else
				{
					++iter_SaveIndex;
				}
			}
		}
		return !rkContSaveIndex.empty();
	}

	void SetFilterText(BM::vstring& kText, int const iJobType)
	{
		switch(iJobType)
		{
		case JST_1ST_MAIN:
			{
				kText = TTW(799583);
			}break;
		case JST_2ND_MAIN:
			{
				kText = TTW(799584);
			}break;
		case JST_3RD_MAIN:
			{
				kText = TTW(799585);
			}break;
		default:
			{
				kText = TTW(799501);
			}break;
		}
	}


	void SetIconInfo(XUI::CXUI_Wnd* pItem, CItemDef const *pDef, int const iSaveIndex)
	{
		if(!pItem || !pDef)
		{
			return;
		}
		XUI::CXUI_Wnd* pIconMain = pItem->GetControl(ITEM_ICON);
		if(!pIconMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pIconWnd = pIconMain->GetControl(L"JS_Icon");
		if(pIconWnd)
		{
			XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pIconWnd);
			if(pIcon)
			{
				PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
				if(!pPlayer)
				{
					return;
				}
				bool const bHave = JobSkillSaveIdxUtil::IsUseableSaveIdx(pPlayer, iSaveIndex);

				SIconInfo kIconInfo = pIcon->IconInfo();
				kIconInfo.iIconKey = pDef->NameNo();
				kIconInfo.iIconResNumber = pDef->ResNo();
				pIcon->SetIconInfo(kIconInfo);
				pIcon->GrayScale(!bHave);
				pIcon->SetCustomData( &(kIconInfo.iIconResNumber), sizeof(kIconInfo.iIconResNumber) );
			}		
		}
	}
	void SetTitleText(XUI::CXUI_Wnd* pItem,int const iNameNo)
	{
		if(!pItem)
		{
			return;
		}
		XUI::CXUI_Wnd* pTitle = pItem->GetControl(ITEM_TEXT_TITLE);
		if(!pTitle)
		{
			return;
		}
		wchar_t const* pName = NULL;
		if(!GetDefString(iNameNo, pName))
		{
			return;
		}
		Quest::SetCutedTextLimitLength(pTitle, pName, L"...");
	}
	void SetInfoText(XUI::CXUI_Wnd* pItem, CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIndexInfo)
	{
		if(!pItem)
		{
			return;
		}
		XUI::CXUI_Wnd* pInfo = pItem->GetControl(ITEM_TEXT_INFO);
		if(!pInfo)
		{
			return;
		}
		PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return;
		}
		if( false == JobSkillSaveIdxUtil::IsUseableSaveIdx(pPlayer, rkSaveIndexInfo.iSaveIdx) )
		{
			//필요스킬이름+'숙련도' + '수치' + 이상
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pSkillDef;
			pSkillDef = kSkillDefMgr.GetDef(rkSaveIndexInfo.iNeedSkillNo01);
			if(pSkillDef)
			{
				BM::vstring kInfo;
				wchar_t const* pNeedSkillName = NULL;
				GetDefString(pSkillDef->NameNo(),pNeedSkillName);
				wchar_t szBuf[MAX_PATH] ={0,};
				int const iNeedExpertness = rkSaveIndexInfo.iNeedSkillExpertness01 / JSE_EXPERTNESS_DEVIDE;
				wsprintfW(szBuf, TTW(799509).c_str(), iNeedExpertness);
				kInfo =  BM::vstring(pNeedSkillName) + BM::vstring(szBuf);
				Quest::SetCutedTextLimitLength(pInfo, kInfo, L"...");
			}
			return;
		}
	}
	void SetPageUI(int const iCurrent, int const iMax)
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(JOBSKILL_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pPage = pMain->GetControl(FRM_PAGE_BG);
		if(pPage)
		{
			wchar_t szBuf[200] ={0,};
			wsprintfW(szBuf, TTW(799506).c_str(), iCurrent, iMax);
			pPage->Text(szBuf);
		}
	}
	void lwClearAll()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Activate(JOBSKILL_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pItemSD = pMain->GetControl(ITEM_SHADOW);
		if(!pItemSD)
		{
			return;
		}
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return;
		}

		SetPageUI(0,0);
		g_kCurrentPage = 1;

		int iListCount = 0;
		while( iListCount < MAX_SHOW_LIST )
		{
			BM::vstring kView = BM::vstring(ITEM_MAIN) + iListCount;
			XUI::CXUI_Wnd* pItem = pItemSD->GetControl(kView);
			if(!pItem)
			{
				return;
			}
			lwClearList(pItem);
			++iListCount;
		}
		return;
	}
	void lwClearList(XUI::CXUI_Wnd* pItem)
	{
		if(pItem)
		{
			XUI::CXUI_Wnd* pIconMain = pItem->GetControl(ITEM_ICON);
			if(pIconMain)
			{
				XUI::CXUI_Wnd* pIconWnd = pIconMain->GetControl(L"JS_Icon");
				if(pIconWnd)
				{
					pIconWnd->ClearCustomData();
					XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pIconWnd);
					if(pIcon)
					{
						SIconInfo kIconInfo = pIcon->IconInfo();
						kIconInfo.iIconKey = 0;
						kIconInfo.iIconResNumber = 0;
						pIcon->SetIconInfo(kIconInfo);
					}
				}
			}
			XUI::CXUI_Wnd* pText = pItem->GetControl(ITEM_TEXT);
			if(pText)
			{
				XUI::CXUI_Wnd* pTitle = pText->GetControl(ITEM_TEXT_TITLE);
				if(pTitle)
				{
					pTitle->Text(L"");
				}
				XUI::CXUI_Wnd* pInfo = pText->GetControl(ITEM_TEXT_INFO);
				if(pInfo)
				{
					pInfo->Text(L"");
				}
			}
		}
	}
	
	void lwCloseJobSkillItemUI()
	{
		XUIMgr.Close(JOBSKILL_MAIN);
		g_kCurrentPage = 1;
		g_kCurrentGatherType = 0;
	}

	//
	// 채집3차 함수정의
	//
	
	// UI CustomData : SFRM_JL3_ITEM_CREATE
	// SFRM_JL3_ITEM_CREATE : 선택된 SaveIdx
	// + ICN_SELECT : BookItemNo
	// + ICN_NEED : Resource_GroupNo==ItemNo, Icon표시
	CONT_HISTORYJOBITEM g_kContJobSkillHistoryItem;
	CONT_DEF_JOBSKILL_RECIPE::mapped_type g_kJobSkillRecipe;
	int g_iChangeRecipeItemNo = 0;
	bool g_bChangeRecipeItemClosing = false;
	float const fJS3MaxResultAniTime = 0.07f;
	void RecvContJobSkillHistoryItem(BM::Stream & rkPacket)
	{
		rkPacket.Pop(g_kContJobSkillHistoryItem);

		lwOnHistoryItemChangePage(lwUIWnd(XUIMgr.Get(JOBSKILL3_ITEM_CREATE)), 0);
	}

	int GetResSlotTotalItemCount(XUI::CXUI_Wnd* pkWnd)
	{
		if( !pkWnd )
		{
			return 0;
		}

		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SRC_SLOT"));
		if( !pkBuild )
		{
			return 0;
		}

		int iTotalItemCount = 0;
		for(int i=0; i<pkBuild->CountX(); ++i)
		{
			BM::vstring vStr("FRM_SRC_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pkSlot = pkWnd->GetControl(vStr);
			pkSlot = pkSlot ? pkSlot->GetControl(L"ICON_SRC_SLOT") : 0;
			if( pkSlot )
			{
				std::pair<int,int> kData;
				if( pkSlot->GetCustomData(&kData, sizeof(kData)) )
				{
					iTotalItemCount += kData.second;
				}
			}
		}

		return iTotalItemCount;
	}

	std::wstring GetJobSkill3_UpgradeNeedItemName(int const iGroupNo, int const iGrade)
	{
		CONT_DEF_JOBSKILL_ITEM_UPGRADE const* pkDefJobSkill_ItemUpgrade = NULL;
		g_kTblDataMgr.GetContDef(pkDefJobSkill_ItemUpgrade);
		CONT_DEF_JOBSKILL_ITEM_UPGRADE::const_iterator find_iter = pkDefJobSkill_ItemUpgrade->begin();
		while( pkDefJobSkill_ItemUpgrade->end() != find_iter)
		{
			if( (*find_iter).second.iResourceGroupNo==iGroupNo
			&& (*find_iter).second.iGrade==iGrade )
			{
				wchar_t const* pkItemName = NULL;
				if( GetItemName((*find_iter).second.iItemNo, pkItemName) )
				{
					return pkItemName;
				}
				break;
			}
			++find_iter;
		}

		return L"";
	}

	void UpdateResItem(XUI::CXUI_Wnd* pkWnd, SJobSkillRes const & kRes)
	{
		PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer ){ return; }

		PgInventory * const pkInv = pkPlayer->GetInven();
		if( !pkInv ){ return; }

		if( !kRes.iGroupNo ){ return; }

		if( !pkWnd ){ return; }

		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SRC_SLOT"));
		if( !pkBuild ){ return; }
		int const iCount = pkBuild->CountX();

		if(0==kRes.iGrade)
		{				
			for(int i=0; i<iCount; ++i)
			{
				BM::vstring vStr(L"FRM_SRC_SLOT");
				vStr += i;
				XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
				if(pkFrm)
				{
					XUI::CXUI_Wnd* pkSlot = pkFrm->GetControl(L"ICON_SRC_SLOT");
					if(pkSlot)
					{
						pkSlot->ClearCustomData();
					}

					XUI::CXUI_Wnd* pkBtn = NULL;
					pkBtn = pkFrm->GetControl(L"BTN_REG");
					pkBtn ? pkBtn->Visible(false) : 0;

					pkBtn = pkFrm->GetControl(L"BTN_DEREG");
					pkBtn ? pkBtn->Visible(false) : 0;
				}
			}

			BM::vstring vStr(L"FRM_SRC_SLOT0");
			XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
			if(pkFrm)
			{
				XUI::CXUI_Wnd* pkSlot = pkFrm->GetControl(L"ICON_SRC_SLOT");
				if(pkSlot)
				{
					std::pair<int,int> kData(kRes.iGroupNo,std::min<int>(pkInv->GetTotalCount(kRes.iGroupNo),kRes.iCount));
					pkSlot->SetCustomData(&kData, sizeof(kData));
					pkSlot->GrayScale(kData.second==0);
				}
			}
		}
		else
		{
			VEC_INT kContItemNo;
			JobSkill_Third::GetContGroupItemNo(kRes.iGroupNo, kRes.iGrade, kContItemNo);
			
			int iIdx = 0;
			for(VEC_INT::const_iterator c_it=kContItemNo.begin(); c_it!=kContItemNo.end(); ++c_it)
			{
				std::pair<int,int> kData((*c_it), 0);

				BM::vstring vStr(L"FRM_SRC_SLOT");
				vStr += iIdx;
				XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
				if(pkFrm)
				{
					bool bVisible = 0<pkInv->GetTotalCount(kData.first);
					XUI::CXUI_Wnd* pkSlot = pkFrm->GetControl(L"ICON_SRC_SLOT");
					if(pkSlot)
					{
						pkSlot->SetCustomData(&(kData), sizeof(kData));
						pkSlot->GrayScale(!bVisible);
					}					

					XUI::CXUI_Wnd* pkBtn = NULL;
					pkBtn = pkFrm->GetControl(L"BTN_REG");
					pkBtn ? pkBtn->Visible(bVisible) : 0;

					pkBtn = pkFrm->GetControl(L"BTN_DEREG");
					pkBtn ? pkBtn->Visible(false) : 0;
				}

				++iIdx;
			}

			for(int i=iIdx; i<iCount; ++i)
			{
				BM::vstring vStr(L"FRM_SRC_SLOT");
				vStr += i;
				XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
				if(pkFrm)
				{
					XUI::CXUI_Wnd* pkSlot = pkFrm->GetControl(L"ICON_SRC_SLOT");
					if(pkSlot)
					{
						pkSlot->ClearCustomData();
					}

					XUI::CXUI_Wnd* pkBtn = NULL;
					pkBtn = pkFrm->GetControl(L"BTN_REG");
					pkBtn ? pkBtn->Visible(false) : 0;

					pkBtn = pkFrm->GetControl(L"BTN_DEREG");
					pkBtn ? pkBtn->Visible(false) : 0;
				}
			}
		}
	}

	void UpdateWndNeedCount(XUI::CXUI_Wnd* pkWnd, SJobSkillRes const & kRes)
	{
		if(!pkWnd)
		{
			return;
		}

		BM::vstring vText;
		if( kRes.iGroupNo )
		{
			BM::vstring vStr;
			int const iHave = GetResSlotTotalItemCount(pkWnd);
			BM::vstring vColor( TTW(iHave<kRes.iCount ? 799553 : 799552) );
			if(0==kRes.iGrade)
			{
				TCHAR const* szNeedItemName = NULL;
				if( GetItemName(kRes.iGroupNo, szNeedItemName) )
				{
					vStr = szNeedItemName;
				}
			}
			else
			{
				vStr = TTW(799821);
				vStr.Replace(L"#GRADE#", kRes.iGrade);
			}
			vText += vStr;
			
			{//Line 2
				vStr = TTW(799820);
				vStr.Replace(L"#COLOR#", vColor);
				vStr.Replace(L"#HAVE#", iHave);
				vStr.Replace(L"#NEED#", kRes.iCount);
			}
			vText += vStr;
		}

		pkWnd->Text(vText);
	}

	//기획팀장님 요청으로 빈 슬롯으로 처리해야 하는 경우 때문에 인덱스 순서를 바꿈
	//DB원본과 다른경우 문제가 발생할 수 있으니 추후 코드 삽입시 참고
	SJobSkillRecipe JS3_UpdateRecipeResource(SJobSkillRecipe const& rkJobSkillRecipe)
	{
		SJobSkillRecipe kTmpRecipe = rkJobSkillRecipe;
		if(rkJobSkillRecipe.iItemNo == 0)
		{
			kTmpRecipe.Clear();
			return kTmpRecipe;
		}		
		
		int iIdx = 0;
		for(int i=0; i<MAX_JS3_RECIPE_RES; ++i)
		{
			SJobSkillRes const & kRes = rkJobSkillRecipe.kResource[i];			
			if(0==kRes.iGroupNo || 0==kRes.iCount)
			{
				continue;
			}
			
			kTmpRecipe.kResource[iIdx] = kRes;
			++iIdx;
		}

		for(int i=iIdx; i<MAX_JS3_RECIPE_RES; ++i)
		{
			kTmpRecipe.kResource[i].Clear();
		}

		return kTmpRecipe;
	}

	int GetAddResItemCount(XUI::CXUI_Wnd* pkWnd)
	{
		if( !pkWnd ){ return 0; }

		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SRC_SLOT"));
		if( !pkBuild ){ return 0; }
		int const iCount = pkBuild->CountX();

		int iItemCount = 0;
		for(int i=0; i<iCount; ++i)
		{
			BM::vstring vStr(L"FRM_SRC_SLOT");
			vStr += i;
			XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
			if(pkFrm)
			{
				std::pair<int,int> kData;
				XUI::CXUI_Wnd* pkSlot = pkFrm->GetControl(L"ICON_SRC_SLOT");
				if(pkSlot)
				{
					pkSlot->GetCustomData(&kData, sizeof(kData));
				}

				iItemCount += kData.second;
			}
		}
		return iItemCount;
	}

	void JS3_UpdateBtn(XUI::CXUI_Wnd* pkWnd, SJobSkillRes const& kRes)
	{
		PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer ){ return; }

		PgInventory * const pkInv = pkPlayer->GetInven();
		if( !pkInv ){ return; }

		if( !kRes.iGroupNo ){ return; }

		if( !pkWnd ){ return; }

		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SRC_SLOT"));
		if( !pkBuild ){ return; }
		int const iCount = pkBuild->CountX();

		if(0==kRes.iGrade)
		{
			for(int i=0; i<iCount; ++i)
			{
				BM::vstring vStr(L"FRM_SRC_SLOT");
				vStr += i;
				XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
				if(pkFrm)
				{
					XUI::CXUI_Wnd* pkBtn = NULL;
					pkBtn = pkFrm->GetControl(L"BTN_REG");
					pkBtn ? pkBtn->Visible(false) : 0;

					pkBtn = pkFrm->GetControl(L"BTN_DEREG");
					pkBtn ? pkBtn->Visible(false) : 0;
				}
			}
		}
		else
		{
			for(int i=0; i<iCount; ++i)
			{
				BM::vstring vStr(L"FRM_SRC_SLOT");
				vStr += i;
				XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
				if(pkFrm)
				{
					std::pair<int,int> kData;
					XUI::CXUI_Wnd* pkSlot = pkFrm->GetControl(L"ICON_SRC_SLOT");
					if(pkSlot)
					{
						pkSlot->GetCustomData(&kData, sizeof(kData));
					}

					bool const bVisible = 0<kData.first;
					int const iUseCount = std::min<int>(pkInv->GetTotalCount(kData.first), kRes.iCount);					

					XUI::CXUI_Wnd* pkBtn = NULL;
					pkBtn = pkFrm->GetControl(L"BTN_REG");
					pkBtn ? pkBtn->Visible(bVisible && kData.second<iUseCount && GetAddResItemCount(pkWnd)<kRes.iCount) : 0;

					pkBtn = pkFrm->GetControl(L"BTN_DEREG");
					pkBtn ? pkBtn->Visible(bVisible && (0<kData.second)) : 0;

					pkFrm->GrayScale(0==iUseCount);
				}
			}
		}
	}

	void lwUpdateJobSkill3NeedUI(lwUIWnd kWnd, int const iItemNo)
	{
		XUI::CXUI_Wnd * pParent = kWnd();
		if( !pParent )
		{
			return;
		}

		CONT_DEF_JOBSKILL_RECIPE const* pkDefJobSkillRecipe = NULL;
		g_kTblDataMgr.GetContDef(pkDefJobSkillRecipe);
		if( !pkDefJobSkillRecipe )
		{
			return;
		}

		CONT_DEF_JOBSKILL_RECIPE::const_iterator iter = pkDefJobSkillRecipe->find(iItemNo);
		if( pkDefJobSkillRecipe->end() != iter )
		{
			g_kJobSkillRecipe = JS3_UpdateRecipeResource((*iter).second);
		}
		else
		{
			g_kJobSkillRecipe.Clear();
		}

		for(int i=0; i<MAX_JS3_RECIPE_RES; ++i)
		{
			BM::vstring vStr("FRM_NEED_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pkNeed = pParent->GetControl(vStr);
			XUI::CXUI_Wnd* pkIcon = pkNeed ? pkNeed->GetControl(L"ICN_NEED") : 0;
			if( pkNeed && pkIcon )
			{
				SJobSkillRes const & kRes = g_kJobSkillRecipe.kResource[i];
				DWORD const iItemNo = kRes.iGroupNo;
				pkIcon->SetCustomData(&iItemNo,sizeof(iItemNo));
				UpdateResItem(pkNeed,kRes);
				UpdateWndNeedCount(pkNeed,kRes);
				JS3_UpdateBtn(pkNeed,kRes);;
			}
		}
	}

	int GetJobSkill3CreateSaveIdx(XUI::CXUI_Wnd* pkWnd)
	{
		if( !pkWnd )
		{
			return 0;
		}

		int iSaveIdx = 0;
		pkWnd->GetCustomData(&iSaveIdx,sizeof(iSaveIdx));

		return iSaveIdx;
	}

	bool GetContResItemInfo(XUI::CXUI_Wnd* pkWnd, CONT_JS3_RESITEM_INFO & kContResItemInfo)
	{
		if( !pkWnd )
		{
			return false;
		}

		CONT_JS3_RESITEM_INFO::value_type kItemInfo;
		for(int i=0; i<MAX_JS3_RECIPE_RES; ++i)
		{
			BM::vstring vStr("FRM_NEED_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pkNeed = pkWnd->GetControl(vStr);
			if( !pkNeed )
			{
				continue;
			}

			XUI::CXUI_Wnd* pkIcon = pkNeed->GetControl(L"ICN_NEED");
			if( !pkIcon )
			{
				continue;
			}

			DWORD iItemNo = 0;
			pkIcon->GetCustomData(&iItemNo,sizeof(iItemNo));
			if(0==iItemNo)
			{
				continue;
			}

			XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkNeed->GetControl(L"BLD_SRC_SLOT"));
			if( !pkBuild )
			{
				continue;
			}

			for(int i=0; i<pkBuild->CountX(); ++i)
			{
				BM::vstring vStr(L"FRM_SRC_SLOT");
				vStr += i;
				XUI::CXUI_Wnd* pkSlot = pkNeed->GetControl(vStr);
				pkSlot = pkSlot ? pkSlot->GetControl(L"ICON_SRC_SLOT") : 0;
				if(pkSlot && pkSlot->GetCustomData(&kItemInfo, sizeof(kItemInfo)))
				{
					if(kItemInfo.first)
					{
						kContResItemInfo.push_back(kItemInfo);
					}
				}
			}
		}

		return !kContResItemInfo.empty();
	}

	BM::vstring GetTargetImgPath(int const iItemNo, int const iRate)
	{
		static BM::vstring const vFilePathImg(L"../Data/6_ui/tech/tcBoxLine0");
		static BM::vstring const vExePathImg(L".tga");
		static int const MAX_UVINDEX = 6;
		int const iTotalRate = JobSkill_Third::GetTotalProbability(iItemNo);
		int iCalcRate = 0;
		if(iTotalRate)
		{
			iCalcRate = (static_cast<float>(iRate) / iTotalRate) * ABILITY_RATE_VALUE;
		}
		int iIdx = lua_tinker::call<int,int>("Get_JS3ResultRateUVIdx", iCalcRate);
		BM::vstring vStr(vFilePathImg);
		vStr += std::min<int>(iIdx, MAX_UVINDEX);
		vStr += vExePathImg;
		return vStr;
	}

	void ChangeRecipeItemNo(int const iRecipeItemNo)
	{
		g_iChangeRecipeItemNo = iRecipeItemNo;
		g_bChangeRecipeItemClosing = false;
	}

	void UpdateJobSkill3ResultUI(XUI::CXUI_Wnd* pkWnd, int const iItemNo)
	{
		if( !pkWnd )
		{
			return;
		}

		PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		CONT_JS3_RESITEM_INFO kContResItemInfo;
		GetContResItemInfo(pkWnd, kContResItemInfo);
		
		JobSkill_Third::CONT_JS3_RESULT_ITEM kContItem;
		bool bRemainResultItem = false;
		int iContItemSize = 0;
		JobSkill_Third::GetResProbabilityItem(pkPlayer->GetInven(), iItemNo, kContResItemInfo, kContItem, bRemainResultItem);
		iContItemSize = kContItem.size()+1;
		iContItemSize += bRemainResultItem ? 1 : 0;

		XUI::CXUI_Wnd* pkDummy = pkWnd->GetControl(L"DMY_TRG_ITEM");
		if(pkDummy)
		{
			pkDummy->ClearOutside();

			int const iCenterX = pkDummy->Size().x / iContItemSize;
			int iIdx = 0;
			
			//
			for(JobSkill_Third::CONT_JS3_RESULT_ITEM::const_iterator it=kContItem.begin(); it!=kContItem.end(); ++it)
			{
				PgMoveAnimateWnd* pkChild = dynamic_cast<PgMoveAnimateWnd*>(XUIMgr.AddChildUI(pkDummy, JOBSKILL3_TRG_SLOT, JOBSKILL3_TRG_SLOT+iIdx, true));
				if(pkChild)
				{
					++iIdx;
					int const iChildHalfWidth = pkChild->Size().x/2;
					XUI::CXUI_Wnd* pkSlot = pkChild->GetControl(L"ICON_TRG_SLOT");
					if(pkSlot)
					{
						DWORD const iItemNo = (*it).second.iResultNo;
						pkSlot->SetCustomData(&iItemNo, sizeof(iItemNo));
					}

					XUI::CXUI_Wnd* pkClosed = pkChild->GetControl(L"IMG_CLOSED");
					pkClosed ? pkClosed->Visible(0==iItemNo) : 0;

					XUI::CXUI_Wnd* pkAni = pkChild->GetControl(L"IMG_SLOT_ANI");
					pkAni ? pkAni->DefaultImgName( GetTargetImgPath(iItemNo, (*it).first) ) : 0;

					XUI::CXUI_Wnd* pkCount = pkChild->GetControl(L"IMG_COUNT");
					if(pkCount){ pkCount->Visible(true); pkCount->Text(BM::vstring((*it).second.iCount)); }

					NiPoint3 kLoc(pkChild->Location().x, pkChild->Location().y, pkChild->Location().z);
					kLoc.x = (iCenterX*iIdx)-iChildHalfWidth;
					//pkChild->Location(kLoc);
					NiPoint3 kEndLoc = kLoc;
					kEndLoc.x = (pkDummy->Size().x/2)-(pkChild->Size().x/2);
					pkChild->Setup(kEndLoc, kLoc, fJS3MaxResultAniTime);
				}
			}

			//확률은 안되지만 뽑을 수 있는 아이템이 더 존재하는 경우
			if(bRemainResultItem)
			{
				PgMoveAnimateWnd* pkChild = dynamic_cast<PgMoveAnimateWnd*>(XUIMgr.AddChildUI(pkDummy, JOBSKILL3_TRG_SLOT, JOBSKILL3_TRG_SLOT+L"_REMAIN", true));
				if(pkChild)
				{
					++iIdx;
					int const iChildHalfWidth = pkChild->Size().x/2;
					XUI::CXUI_Wnd* pkSlot = pkChild->GetControl(L"ICON_TRG_SLOT");
					if(pkSlot)
					{
						DWORD const iItemNo = 0;
						pkSlot->SetCustomData(&iItemNo, sizeof(iItemNo));
					}

					XUI::CXUI_Wnd* pkRemain = pkChild->GetControl(L"IMG_REMAIN");
					pkRemain ? pkRemain->Visible(true) : 0;						

					XUI::CXUI_Wnd* pkAni = pkChild->GetControl(L"IMG_SLOT_ANI");
					pkAni ? pkAni->DefaultImgName( GetTargetImgPath(iItemNo, 0) ) : 0;

					XUI::CXUI_Wnd* pkCount = pkChild->GetControl(L"IMG_COUNT");
					 pkCount ? pkCount->Visible(false) : 0;

					NiPoint3 kLoc(pkChild->Location().x, pkChild->Location().y, pkChild->Location().z);
					kLoc.x = (iCenterX*iIdx)-iChildHalfWidth;
					//pkChild->Location(kLoc);
					NiPoint3 kEndLoc = kLoc;
					kEndLoc.x = (pkDummy->Size().x/2)-(pkChild->Size().x/2);
					pkChild->Setup(kEndLoc, kLoc, fJS3MaxResultAniTime);
				}
			}
		}
	}

	void ClearJobSkill3Slot(XUI::CXUI_Wnd* pkWnd)
	{
		if( !pkWnd )
		{
			return;
		}

		for(int i=0; i<MAX_JS3_RECIPE_RES; ++i)
		{
			BM::vstring vStr("FRM_NEED_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pkNeed = pkWnd->GetControl(vStr);
			if( pkNeed )
			{
				XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkNeed->GetControl(L"BLD_SRC_SLOT"));
				if( pkBuild )
				{
					int const iCount  = pkBuild->CountX();
					for(int i=0; i<iCount; ++i)
					{
						BM::vstring vStr(L"FRM_SRC_SLOT");
						vStr += i;
						XUI::CXUI_Wnd* pkFrm = pkNeed->GetControl(vStr);
						if(pkFrm)
						{
							XUI::CXUI_Wnd* pkSlot = pkFrm->GetControl(L"ICON_SRC_SLOT");
							if(pkSlot)
							{
								pkSlot->ClearCustomData();
							}

							XUI::CXUI_Wnd* pkBtn = NULL;
							pkBtn = pkFrm->GetControl(L"BTN_REG");
							pkBtn ? pkBtn->Visible(false) : 0;

							pkBtn = pkFrm->GetControl(L"BTN_DEREG");
							pkBtn ? pkBtn->Visible(false) : 0;
						}
					}
				}
			}
		}
	}

	void JS3_UpdateArrow(XUI::CXUI_Wnd* pkWnd)
	{
		if( !pkWnd )
		{
			return;
		}

		PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		CONT_JS3_RESITEM_INFO kContResItemInfo;
		GetContResItemInfo(pkWnd, kContResItemInfo);
		bool bDoAction = S_OK==JobSkill_Third::CheckNeedItems(pkPlayer->GetInven(), g_kJobSkillRecipe.iItemNo, kContResItemInfo);

		XUI::CXUI_Wnd* pkTmp = NULL;
		pkTmp = pkWnd->GetControl(L"IMG_ARROW_INIT");
		pkTmp ? pkTmp->Visible(!bDoAction) : 0;

		pkTmp = pkWnd->GetControl(L"IMG_ARROW_USE");
		pkTmp ? pkTmp->Visible(bDoAction) : 0;

		pkTmp = pkWnd->GetControl(L"IMG_ARROW_PLAY");
		pkTmp ? pkTmp->Visible(false) : 0;

		XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
		pButton ? pButton->Disable(!bDoAction) : 0;
	}

	void lwSetJobSkill3CreateItem(lwUIWnd kWnd, int const iSaveIdx)
	{
		XUI::CXUI_Wnd* pParent = kWnd();
		if( !pParent )
		{
			return;
		}
		
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return;
		}

		pParent->SetCustomData(&iSaveIdx,sizeof(iSaveIdx));

		int iItemNo = 0;
		int iRecipeItemNo = 0;
		SJobSkillSaveIdx const * const pkDefSaveIdx = JobSkill_Third::GetJobSkillSaveIdx(iSaveIdx);
		if( pkDefSaveIdx )
		{
			iItemNo = pkDefSaveIdx->iBookItemNo;
			iRecipeItemNo = pkDefSaveIdx->iRecipeItemNo;
		}

		XUI::CXUI_Wnd* pkIcon = pParent->GetControl(L"ICN_SELECT");
		if(pkIcon)
		{
			pkIcon->SetCustomData(&iItemNo,sizeof(iItemNo));
		}

		ClearJobSkill3Slot(pParent);
		lwUpdateJobSkill3NeedUI(pParent, iRecipeItemNo);
		JS3_UpdateArrow(pParent);
		if(0==iRecipeItemNo)
		{
			UpdateJobSkill3ResultUI(pParent, 0);
		}
		else
		{
			ChangeRecipeItemNo(iRecipeItemNo);
		}

		XUI::CXUI_Wnd* pkBottomText = pParent->GetControl(L"IMG_BOTTOM_BG");
		pkBottomText ? pkBottomText->Text(TTW(799550)) : 0;

		XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pParent->GetControl(L"BTN_OK"));
		if(pButton)
		{
			BM::vstring vStr(TTW(799551));
			vStr += TTW(2200);
			pButton->Text(vStr);
		}
		g_bJS3_CreateItemSend = false;
	}

	int g_iHistoryPage = 0;
	void lwOnHistoryItemChangePage(lwUIWnd kWnd, int const iCalc)
	{
		XUI::CXUI_Wnd * pParent = kWnd();
		if( !pParent )
		{
			return;
		}

		if( 0==iCalc )
		{
			g_iHistoryPage = 0;
		}

		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pParent->GetControl(L"BLD_HISTORY_SLOT"));
		if( !pkBuild )
		{
			return;
		}
		int const iCount = pkBuild->CountX();

		int iCurPage = g_iHistoryPage + iCalc;
		if( iCurPage < 0 )
		{
			return;
		}
		else if( iCurPage > MAX_JOBSKILL_HISTORYITEM-iCount )
		{
			return;
		}
		else
		{
			g_iHistoryPage = iCurPage;

			CONT_HISTORYJOBITEM::const_iterator c_it = g_kContJobSkillHistoryItem.begin();
			while(c_it != g_kContJobSkillHistoryItem.end())
			{
				if(iCurPage < 1)
				{
					break;
				}
				--iCurPage;
				++c_it;
			}

			XUI::CXUI_Wnd * pIcon = NULL;
			XUI::CXUI_Wnd * pDummy = NULL;
			for(int i=0; i<iCount; ++i)
			{
				DWORD iSaveIdx = 0;
				if(c_it != g_kContJobSkillHistoryItem.end())
				{
					iSaveIdx = (*c_it);
					++c_it;
				}

				DWORD iItemNo = 0;
				SJobSkillSaveIdx const * const pkDefSaveIdx = JobSkill_Third::GetJobSkillSaveIdx(iSaveIdx);
				if( pkDefSaveIdx )
				{
					iItemNo = pkDefSaveIdx->iBookItemNo;
				}

				BM::vstring vStr(L"ICN_HISTORY");
				vStr += i;
				pIcon = pParent->GetControl(vStr);
				if(pIcon)
				{
					pIcon->SetCustomData(&iItemNo, sizeof(iItemNo));
					pDummy = pIcon->GetControl(L"DMY_SAVEIDX");
					pDummy ? pDummy->SetCustomData(&iSaveIdx, sizeof(iSaveIdx)) : 0;
				}
			}

			XUI::CXUI_Button* pButton = NULL;
			pButton = dynamic_cast<XUI::CXUI_Button*>(pParent->GetControl(L"BTN_PAGE_L"));
			pButton ? pButton->Disable(g_iHistoryPage<=0) : 0;

			pButton = dynamic_cast<XUI::CXUI_Button*>(pParent->GetControl(L"BTN_PAGE_R"));
			pButton ? pButton->Disable(g_iHistoryPage>=MAX_JOBSKILL_HISTORYITEM-iCount) : 0;
		}
	}

	int GetMaxShowListCount()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(JOBSKILL3_ITEM_SELECT);
		if(pMain)
		{
			XUI::CXUI_Wnd* pItemSD = pMain->GetControl(ITEM_SHADOW);
			if(pItemSD)
			{
				XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pItemSD->GetControl(ITEM_LIST));
				if(pBuild)
				{
					return pBuild->CountX() * pBuild->CountY();
				}
			}
		}
		return 0;
	}

	void JS3_SetPageUI(int const iCurrent, int const iMax)
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(JOBSKILL3_ITEM_SELECT);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pPage = pMain->GetControl(FRM_PAGE_BG);
		if(pPage)
		{
			wchar_t szBuf[200] ={0,};
			wsprintfW(szBuf, TTW(799506).c_str(), iCurrent, iMax);
			pPage->Text(szBuf);
		}
	}

	void lwJS3_ClearAll()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Activate(JOBSKILL3_ITEM_SELECT);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pItemSD = pMain->GetControl(ITEM_SHADOW);
		if(!pItemSD)
		{
			return;
		}
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return;
		}

		JS3_SetPageUI(0,0);
		g_kCurrentPage = 1;

		int iListCount = 0;
		int const iMaxShowListCount = GetMaxShowListCount();
		while( iListCount < iMaxShowListCount )
		{
			BM::vstring kView = BM::vstring(ITEM_MAIN) + iListCount;
			XUI::CXUI_Wnd* pItem = pItemSD->GetControl(kView);
			if(!pItem)
			{
				return;
			}
			lwClearList(pItem);
			++iListCount;
		}
		return;
	}

	void JS3_SetIconInfo(XUI::CXUI_Wnd* pItem, CItemDef const *pDef, int const iSaveIndex)
	{
		if(!pItem || !pDef)
		{
			return;
		}
		XUI::CXUI_Wnd* pIconMain = pItem->GetControl(ITEM_ICON);
		if(!pIconMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pIconWnd = pIconMain->GetControl(L"JS_Icon");
		if(pIconWnd)
		{
			XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pIconWnd);
			if(pIcon)
			{
				PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
				if(!pPlayer)
				{
					return;
				}
				bool const bHave = JobSkillSaveIdxUtil::IsUseableSaveIdx(pPlayer, iSaveIndex);

				SIconInfo kIconInfo = pIcon->IconInfo();
				kIconInfo.iIconKey = pDef->NameNo();
				kIconInfo.iIconResNumber = pDef->ResNo();
				pIcon->SetIconInfo(kIconInfo);
				pIcon->GrayScale(!bHave);
				pIcon->SetCustomData( &iSaveIndex, sizeof(iSaveIndex) );
			}		
		}
	}

	void GetSortJobSkillSaveIdx(CONT_DEF_JOBSKILL_SAVEIDX const& kContSaveIndex, CONT_SORT_JOBSKILL_SAVEIDX & kContSortSaveIdx)
	{
		CONT_SORT_JOBSKILL_SAVEIDX::key_type kKey;
		CONT_DEF_JOBSKILL_SAVEIDX::const_iterator c_it = kContSaveIndex.begin();
		while(c_it!=kContSaveIndex.end())
		{
			kKey.iNo = (*c_it).first;
			kKey.iNeedSkillNo = (*c_it).second.iNeedSkillNo01;
			kKey.iNeedSkillExpertness = (*c_it).second.iNeedSkillExpertness01;
			kContSortSaveIdx.insert(std::make_pair(kKey,(*c_it).second));
			++c_it;
		}
	}

	bool JS3_ProcessLearnedSaveIndex(CONT_SORT_JOBSKILL_SAVEIDX & rkContSortSaveIdx)
	{// 사용 가능한 아이템만 남김
		PgPlayer const * pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return false;
		}
		
		CONT_SORT_JOBSKILL_SAVEIDX::iterator iter_SaveIdx = rkContSortSaveIdx.begin();
		while(iter_SaveIdx != rkContSortSaveIdx.end() )
		{
			CONT_SORT_JOBSKILL_SAVEIDX::key_type const &rkSaveIdx = (*iter_SaveIdx).first;
			CONT_SORT_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIdxInfo = (*iter_SaveIdx).second;
			if( false==JobSkillSaveIdxUtil::IsUseableSaveIdx( pkPlayer, rkSaveIdx.iNo ) )
			{
				iter_SaveIdx = rkContSortSaveIdx.erase(iter_SaveIdx);
			}
			else
			{
				++iter_SaveIdx;
			}
		}
		return true; 
	}

	bool g_bFilterOnlyLearn = false;
	void lwJS3_SetFilterOnlyLearn(bool const bCheck)
	{
		g_bFilterOnlyLearn = bCheck;
	}
	void lwJS3_ShowSelectItemList()
	{
		lwJS3_ShowList(g_kCurrentPage, g_kCurrentGatherType);
	}

	bool lwJS3_ShowList(int iPage, int const iGatherType )
	{//아이템 목록 보여주기
		XUI::CXUI_Wnd* pMain = XUIMgr.Activate(JOBSKILL3_ITEM_SELECT);
		if(!pMain)
		{
			return false;
		}

		XUI::CXUI_Wnd* pItemSD = pMain->GetControl(ITEM_SHADOW);
		if(!pItemSD)
		{
			return false;
		}
		
		lwJS3_ClearAll();

		//아이템 정렬
		CONT_SORT_JOBSKILL_SAVEIDX kContSortSaveIdx;
		{
			CONT_DEF_JOBSKILL_SAVEIDX kContSaveIndex;
			if( false==FilterType(kContSaveIndex, iGatherType, JST_3RD_MAIN) )
			{
				return false;
			}
			GetSortJobSkillSaveIdx(kContSaveIndex, kContSortSaveIdx);
			if( g_bFilterOnlyLearn )
			{
				JS3_ProcessLearnedSaveIndex(kContSortSaveIdx);
			}
		}

		//페이지 설정
		int const iMaxShowLIst = GetMaxShowListCount();
		if(0==iMaxShowLIst)
		{
			return false;
		}
		int iMaxPage = kContSortSaveIdx.size() / iMaxShowLIst;
		if( 0 != (kContSortSaveIdx.size() % iMaxShowLIst) )
		{
			++iMaxPage;
		}
		if(  1 > iPage || 0 == iMaxPage)
		{//페이지 없으면....아이템 안들어가 잇는거
			return false;
		}
		if( iMaxPage < iPage )
		{
			iPage = iMaxPage;
		}
		//현재 정렬 타입 텍스트 설정
		BM::vstring kFilterText;
		//현재 보여지는 페이지 설정
		JS3_SetPageUI(iPage, iMaxPage);

		//배운아이템 따로 가져오기
		CONT_DEF_JOBSKILL_SAVEIDX kContLearned;
		GetLearnedSaveIndex(kContSortSaveIdx, kContLearned);

		//지정된 페이지의 스킬 목록 설정
		CONT_DEF_JOBSKILL_SAVEIDX::const_iterator itor_LearnedSaveIndex = kContLearned.begin();
		CONT_SORT_JOBSKILL_SAVEIDX::const_iterator itor_SaveIndex = kContSortSaveIdx.begin();
		int iPageCount = 0;
		while( iPageCount < ( (iPage-1)*iMaxShowLIst) )
		{//지정된 페이지로 이동
			
			if( itor_LearnedSaveIndex != kContLearned.end() )
			{ //배운 도감 먼저 넘기고,
				++itor_LearnedSaveIndex;
				++iPageCount;	
				continue;
			}		
			if( itor_SaveIndex == kContSortSaveIdx.end() )
			{ //
				break;
			}
			++itor_SaveIndex;
			++iPageCount;
		}
		//페이지 출력
		int iListCount = 0;
		while( iListCount < iMaxShowLIst )
		{
			if( 0 != kContLearned.size() && itor_LearnedSaveIndex != kContLearned.end())
			{//배운 아이템 먼저 출력하기
				CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIndexInfo = (*itor_LearnedSaveIndex).second;
				BM::vstring kItem = BM::vstring(ITEM_MAIN) + iListCount;
				XUI::CXUI_Wnd* pItem = pItemSD->GetControl(kItem);
				if(!pItem)
				{
					return false;
				}
				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pDef = kItemDefMgr.GetDef(rkSaveIndexInfo.iBookItemNo);
				if(pDef)
				{
					JS3_SetIconInfo(pItem, pDef, rkSaveIndexInfo.iSaveIdx);
					XUI::CXUI_Wnd* pTitle = pItem->GetControl(ITEM_TEXT);
					if(pTitle)
					{
						SetTitleText(pTitle, rkSaveIndexInfo.iBookItemNo);
						SetInfoText(pTitle, rkSaveIndexInfo);
					}
				}
				++iListCount;
				++itor_LearnedSaveIndex;
				continue;
			}

			{// 안배운 아이템 출력하기
				if(itor_SaveIndex == kContSortSaveIdx.end())
				{
					break;
				}
				if(kContLearned.end() == kContLearned.find((*itor_SaveIndex).first.iNo))
				{//현재 출력하려는게 배운아이템에 있으면 패스
					CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIndexInfo = (*itor_SaveIndex).second;
					
					BM::vstring kItem = BM::vstring(ITEM_MAIN) + iListCount;
					XUI::CXUI_Wnd* pItem = pItemSD->GetControl(kItem);
					if(!pItem)
					{
						return false;
					}
					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const *pDef = kItemDefMgr.GetDef(rkSaveIndexInfo.iBookItemNo);
					if(pDef)
					{
						JS3_SetIconInfo(pItem, pDef, rkSaveIndexInfo.iSaveIdx);
						XUI::CXUI_Wnd* pTitle = pItem->GetControl(ITEM_TEXT);
						if(pTitle)
						{
							SetTitleText(pTitle, rkSaveIndexInfo.iBookItemNo);
							SetInfoText(pTitle, rkSaveIndexInfo);
						}
					}
					++iListCount;
				}
				++itor_SaveIndex;
			}
		}
		g_kCurrentPage = iPage;
		return true;
	}

	bool lwJS3_NextPage()
	{
		if(!lwJS3_ShowList(g_kCurrentPage + 1, g_kCurrentGatherType))
		{
			//lwCloseJobSkill3ItemUI();
			return false;
		}
		return true;
	}

	bool lwJS3_BeforePage()
	{
		if( 1 == g_kCurrentPage)
		{
			++g_kCurrentPage;
		}
		if(!lwJS3_ShowList(g_kCurrentPage - 1, g_kCurrentGatherType))
		{
			//lwCloseJobSkill3ItemUI();
			return false;
		}
		return true;
	}

	void SetFilterItem3Type()
	{//드랍다운 메뉴 설정
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(JOBSKILL3_ITEM_SELECT);
		if(!pMain)
		{ 
			return;
		}
		XUI::CXUI_Wnd* pSelectShadow = pMain->GetControl(JOB_SELECT_ITEMTYPE);
		if(!pSelectShadow)
		{
			return;
		}
		XUI::CXUI_Wnd* pSelectWnd = pSelectShadow->GetControl(JOB_SELECT_ITEMTYPE_LIST);
		if(!pSelectWnd)
		{
			return;
		}
		XUI::CXUI_List* pSelectList = dynamic_cast<XUI::CXUI_List*>(pSelectWnd);
		if(!pSelectList)
		{
			return;
		}
		pSelectList->ClearList();
		CONT_DEF_JOBSKILL_SKILL const* pkContDefJobSkill;
		g_kTblDataMgr.GetContDef(pkContDefJobSkill);
		if(!pkContDefJobSkill)
		{
			return;
		}
		//전체 보기 미리 추가
		XUI::SListItem* pItem = pSelectList->AddItem(L"");
		if(pItem)
		{
			pItem->m_pWnd->Text( TTW(799501) );
			int const iJobSkillType = 0;
			pItem->m_pWnd->SetCustomData(&iJobSkillType, sizeof(iJobSkillType));
		}
		//배운 아이템을 가진 순서대로 저장하기
		PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return;
		}
		CONT_DEF_JOBSKILL_SAVEIDX const* pkContDefSaveIdx;
		g_kTblDataMgr.GetContDef(pkContDefSaveIdx);
		//1. 도감 전체 컨테이너 구하고
		if(!pkContDefSaveIdx)
		{
			return;
		}
		//2. 하나씩 돌면서 임시 컨테이너에 사용 가능한 도감의 메인스킬 번호만 넣고
		CONT_DEF_JOBSKILL_SKILL kContLearnSkill;
		CONT_DEF_JOBSKILL_SAVEIDX::const_iterator saveIdx_iter = pkContDefSaveIdx->begin();
		while( saveIdx_iter != pkContDefSaveIdx->end())
		{
			int const iSaveIndex = (*saveIdx_iter).first;
			CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const &rkSaveIdx = (*saveIdx_iter).second;
			if( JobSkillSaveIdxUtil::IsUseableSaveIdx(pPlayer, iSaveIndex) )
			{
				CONT_DEF_JOBSKILL_SKILL::const_iterator temp_iter = pkContDefJobSkill->find(rkSaveIdx.iNeedSkillNo01);
				if(pkContDefJobSkill->end() != temp_iter)
				{//임시컨테이너에 없는 스킬이면 추가
					kContLearnSkill.insert( std::make_pair((*temp_iter).first, (*temp_iter).second));
				}
			}
			++saveIdx_iter;
		}
		//3. 다음에 전체 돌면서 안배운거 넣어주기
		CONT_DEF_JOBSKILL_SKILL::const_iterator temp_iter = kContLearnSkill.begin();
		while(kContLearnSkill.end() != temp_iter)
		{
			int const iJobSkillNo = (*temp_iter).first;
			CONT_DEF_JOBSKILL_SKILL::mapped_type const &rkJobSkillInfo = (*temp_iter).second;
			if( JST_3RD_MAIN == rkJobSkillInfo.eJobSkill_Type )
			{//주스킬의 경우만 받아서 Add하기
				InsertFilterItem(pSelectList, iJobSkillNo, rkJobSkillInfo, false);
			}
			++temp_iter;
		}

		CONT_DEF_JOBSKILL_SKILL::const_iterator iter_Job = pkContDefJobSkill->begin();
		while( iter_Job != pkContDefJobSkill->end() )
		{
			int const iJobSkillNo = (*iter_Job).first;
			CONT_DEF_JOBSKILL_SKILL::mapped_type const &rkJobSkillInfo = (*iter_Job).second;

			CONT_DEF_JOBSKILL_SKILL::const_iterator temp_iter = kContLearnSkill.find(iJobSkillNo);
			if(kContLearnSkill.end() == temp_iter)
			{//임시컨테이너에서 넣었던 스킬이랑 안겹치도록
				if( JST_3RD_MAIN == rkJobSkillInfo.eJobSkill_Type )
				{//주스킬의 경우만 받아서 Add하기
					InsertFilterItem(pSelectList, iJobSkillNo, rkJobSkillInfo, true);
				}
			}
			++iter_Job;
		}
	}

	void lwCallJobSkill3ItemUI()
	{
		lwJS3_ShowList(g_kCurrentPage, g_kCurrentGatherType);
		SetFilterItem3Type();
	}

	void lwCloseJobSkill3ItemUI()
	{
		XUIMgr.Close(JOBSKILL3_ITEM_SELECT);
		g_kCurrentPage = 1;
		g_kCurrentGatherType = 0;
	}

	void JS3_SetFilterTitle(std::wstring const& kTitle)
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(JOBSKILL3_ITEM_SELECT);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pFilter = pMain->GetControl(JOB_SELECT_SHADOW);
		if(!pFilter)
		{
			return;
		}
		pFilter->Text(kTitle);
	}

	void lwJS3_SetGatherType(std::wstring const &kFilterTitle, int const iGatherType)
	{
		JS3_SetFilterTitle(kFilterTitle);
		g_kCurrentGatherType = iGatherType;
		lwJS3_ClearAll();
		lwJS3_ShowList(g_kCurrentPage, g_kCurrentGatherType);
	}

	void lwJS3_SelectMakeItem(int const iSaveIdx)
	{
		PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer ){ return; }

		SJobSkillSaveIdx const * const pkDefSaveIdx = JobSkill_Third::GetJobSkillSaveIdx(iSaveIdx);
		if( !pkDefSaveIdx )
		{
			//제작 가능한 아이템이 없습니다.
			::Notice_Show_ByTextTableNo(799811, EL_Warning);
			return;
		}
		
		int const iRecipeItemNo = pkDefSaveIdx->iRecipeItemNo;

		if( S_OK != JobSkill_Third::CheckNeedSkill(pkPlayer, iSaveIdx) )
		{
			::Notice_Show_ByTextTableNo(799812, EL_Warning);
			return;
		}

		//생산력 체크
		if( S_OK != JobSkill_Third::CheckNeedProductPoint(pkPlayer, iRecipeItemNo) )
		{
			//[피로도]가 부족합니다. 다른 아이템을 선택하세요.
			::Notice_Show_ByTextTableNo(799810, EL_Warning);
			return;
		}

		lwSetJobSkill3CreateItem(lwUIWnd(XUIMgr.Get(JOBSKILL3_ITEM_CREATE)), iSaveIdx);
	}

	void lwJS3_Tick(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd();
		if( !pkWnd )
		{
			return;
		}

		if(0==g_kJobSkillRecipe.iItemNo)
		{
			return;
		}

		if(g_iChangeRecipeItemNo)
		{
			XUI::CXUI_Wnd* pkDummy = pkWnd->GetControl(L"DMY_TRG_ITEM");
			if(pkDummy)
			{
				if(pkDummy->HaveOutside() && false==g_bChangeRecipeItemClosing)
				{
					int const iCenterX = pkDummy->Size().x / 2;

					int iIdx = 0;
					PgMoveAnimateWnd* pkMoveTarget = NULL;
					pkMoveTarget = dynamic_cast<PgMoveAnimateWnd*>(pkDummy->GetControl(JOBSKILL3_TRG_SLOT+iIdx));
					while( pkMoveTarget )
					{
						NiPoint3 kStartLoc(pkMoveTarget->Location().x,pkMoveTarget->Location().y,pkMoveTarget->Location().z);
						NiPoint3 kEndLoc(kStartLoc);
						kEndLoc.x = iCenterX-(pkMoveTarget->Size().x/2);
						pkMoveTarget->Setup(kStartLoc, kEndLoc, fJS3MaxResultAniTime);

						++iIdx;
						pkMoveTarget = dynamic_cast<PgMoveAnimateWnd*>(pkDummy->GetControl(JOBSKILL3_TRG_SLOT+iIdx));
					}

					pkMoveTarget = dynamic_cast<PgMoveAnimateWnd*>(pkDummy->GetControl(JOBSKILL3_TRG_SLOT+L"_REMAIN"));
					if( pkMoveTarget )
					{
						NiPoint3 kStartLoc(pkMoveTarget->Location().x,pkMoveTarget->Location().y,pkMoveTarget->Location().z);
						NiPoint3 kEndLoc(kStartLoc);
						kEndLoc.x = iCenterX-(pkMoveTarget->Size().x/2);
						pkMoveTarget->Setup(kStartLoc, kEndLoc, fJS3MaxResultAniTime);
					}
					g_bChangeRecipeItemClosing = true;
				}
				else
				{
					PgMoveAnimateWnd* pkMoveTarget = dynamic_cast<PgMoveAnimateWnd*>(pkDummy->GetControl(JOBSKILL3_TRG_SLOT+0));
					if(NULL==pkMoveTarget || pkMoveTarget->IsMoveComplate())
					{
						g_bChangeRecipeItemClosing = false;
						pkDummy->ClearOutside();
						UpdateJobSkill3ResultUI(pkWnd, g_iChangeRecipeItemNo);
						g_iChangeRecipeItemNo = 0;
					}
				}
			}
		}
	}

	bool IsJS3_Sending()
	{
		XUI::CXUI_Wnd* pkMain = XUIMgr.Get(JOBSKILL3_ITEM_CREATE);
		if(!pkMain)
		{
			return false;
		}

		XUI::CXUI_Wnd* pkPlay = pkMain->GetControl(L"IMG_ARROW_PLAY");
		if(!pkPlay)
		{
			return false;
		}

		return pkPlay->Visible();;
	}

	void lwJS3_FullResItemSlot(lwUIWnd kWnd)
	{
		if( IsJS3_Sending() )
		{
			return;
		}

		PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer ){ return; }

		PgInventory * const pkInv = pkPlayer->GetInven();
		if( !pkInv ){ return; }

		XUI::CXUI_Wnd* pkWnd = kWnd();
		if( !pkWnd ){ return; }

		if( !pkWnd->Parent() || !pkWnd->Parent()->Parent() ){ return; }
		XUI::CXUI_Wnd* pkParent = pkWnd->Parent()->Parent();

		SJobSkillRes const & kRes = g_kJobSkillRecipe.kResource[pkParent->BuildIndex()];
		if(0==kRes.iGroupNo || 0==kRes.iGrade){ return; }

		std::pair<int,int> kData;
		pkWnd->GetCustomData(&kData, sizeof(kData));
		int const iOldCount = kData.second;
		int const iTotalCount = GetAddResItemCount(pkParent);
		int const iHaveCount = pkInv->GetTotalCount(kData.first);
		int const iNeedTotalCount = kRes.iCount;

		int const iNeedMore = std::max<int>(0, iNeedTotalCount-iTotalCount);	//이만큼이 더 필요함
		int const iHaveMore = std::max<int>(0, iHaveCount-iOldCount);	//이만큼을 더 등록할 수 있음
		kData.second += std::min<int>(iNeedMore, iHaveMore);

		if(iOldCount != kData.second)
		{
			pkWnd->SetCustomData(&kData, sizeof(kData));
			JS3_UpdateBtn(pkParent, kRes);		
			JS3_RefreshResItem();
		}
	}

	void lwJS3_ClearResItemSlot(lwUIWnd kWnd)
	{
		if( IsJS3_Sending() )
		{
			return;
		}

		XUI::CXUI_Wnd* pkWnd = kWnd();
		if( !pkWnd ){ return; }

		if( !pkWnd->Parent() || !pkWnd->Parent()->Parent() ){ return; }
		XUI::CXUI_Wnd* pkParent = pkWnd->Parent()->Parent();

		SJobSkillRes const & kRes = g_kJobSkillRecipe.kResource[pkParent->BuildIndex()];
		if(0==kRes.iGroupNo || 0==kRes.iGrade){ return; }

		std::pair<int,int> kData;
		pkWnd->GetCustomData(&kData, sizeof(kData));
		if(0==kData.second)
		{ 
			return; 
		}

		kData.second = 0;
		
		pkWnd->SetCustomData(&kData, sizeof(kData));
		JS3_UpdateBtn(pkParent, kRes);
		JS3_RefreshResItem();
	}

	void lwJS3_SetResItemSlot(lwUIWnd kWnd, bool const bSet)
	{
		if( IsJS3_Sending() )
		{
			return;
		}

		XUI::CXUI_Wnd* pkWnd = kWnd();
		if( !pkWnd ){ return; }

		if( !pkWnd->Parent() ){ return; }
		XUI::CXUI_Wnd* pkParent = pkWnd->Parent();

		SJobSkillRes const & kRes = g_kJobSkillRecipe.kResource[pkParent->BuildIndex()];
		if(0==kRes.iGroupNo || 0==kRes.iGrade){ return; }

		XUI::CXUI_Wnd* pkSlot = pkWnd->GetControl(L"ICON_SRC_SLOT");
		if( !pkSlot ){ return; }

		std::pair<int,int> kData;
		pkSlot->GetCustomData(&kData, sizeof(kData));

		PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer ){ return; }

		PgInventory * const pkInv = pkPlayer->GetInven();
		if( !pkInv ){ return; }

		int const iOldCount = kData.second;
		int const iTotalCount = GetAddResItemCount(pkParent);
		int const iHaveCount = pkInv->GetTotalCount(kData.first);
		int const iNeedTotalCount = kRes.iCount;

		if(bSet)
		{
			if( (iTotalCount+1<=iNeedTotalCount) && (kData.second+1<=iHaveCount) )
			{
				kData.second += 1;
			}
		}
		else
		{
			kData.second = std::max<int>(0,kData.second-1);
		}
		

		if(iOldCount != kData.second)
		{
			pkSlot->SetCustomData(&kData, sizeof(kData));
			JS3_UpdateBtn(pkWnd->Parent(), kRes);
			JS3_RefreshResItem();
		}
	}

	void JS3_RefreshResItem()
	{
		XUI::CXUI_Wnd* pkMain = XUIMgr.Get(JOBSKILL3_ITEM_CREATE);
		if(!pkMain)
		{
			return;
		}

		if(0==g_kJobSkillRecipe.iItemNo)
		{
			return;
		}

		if(g_bJS3_CreateItemSend)
		{ 
			return;
		}

		for(int i=0; i<MAX_JS3_RECIPE_RES; ++i)
		{
			BM::vstring vStr("FRM_NEED_SLOT");
			vStr += i;
			UpdateWndNeedCount(pkMain->GetControl(vStr), g_kJobSkillRecipe.kResource[i]);
		}

		JS3_UpdateArrow(pkMain);
		ChangeRecipeItemNo(g_kJobSkillRecipe.iItemNo);
	}

	void JS3_AddResItem(XUI::CXUI_Wnd* pWnd, SItemPos const& kItemPos, bool const bDivideReserve)
	{
		if( !pWnd || !pWnd->Parent() || !(pWnd->Parent()->BuildIndex() < MAX_JS3_RECIPE_RES))
		{
			return;
		}
		XUI::CXUI_Wnd* pParent = pWnd->Parent();

		if(g_kJobSkillRecipe.iItemNo == 0)
		{
			return;
		}

		if(g_bJS3_CreateItemSend)
		{ 
			return; 
		}

		SJobSkillRes const & kRes = g_kJobSkillRecipe.kResource[pParent->BuildIndex()];
		if(kRes.iGroupNo == 0)
		{
			//등록이 불가능한 슬롯
			::Notice_Show_ByTextTableNo(799817, EL_Warning);
			return;
		}

		PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer || !pkPlayer->GetInven() )
		{
			return;
		}
		PgInventory * const pkInv = pkPlayer->GetInven();

		SItemPos kOldItemPos;
		bool const bOldAddItem = pWnd->GetCustomData(&kOldItemPos, sizeof(kOldItemPos));
		pWnd->ClearCustomData();

		PgBase_Item kAddItem;
		if(S_OK != pkInv->GetItem(kItemPos, kAddItem))
		{
			//올바르지 않은 아이템
			if(bOldAddItem)
			{
				JS3_RefreshResItem();
			}
			return;
		}
		//같은 ItemPos가 등록되는 경우 막음..
		CONT_DEF_JOBSKILL_ITEM_UPGRADE::mapped_type kItemUpgrade;
		JobSkill_Util::GetJobSkill_Item(kAddItem.ItemNo(), kItemUpgrade);
		if( false==JobSkill_Third::UseResItem(kRes.iGroupNo, kRes.iGrade, kAddItem.ItemNo(), kItemUpgrade.iResourceGroupNo, kItemUpgrade.iGrade) )
		{
			//올바르지 않은 재료 아이템
			::Notice_Show_ByTextTableNo(799813, EL_Warning);
			return;
		}

		int const iTotalCount = GetResSlotTotalItemCount(pParent);
		if(kRes.iCount <= iTotalCount)
		{
			//이미 [아이템이름]은(는) 충분합니다. 더 이상 올릴 수 없습니다.
			wchar_t const* pkItemName = NULL;
			GetItemName(kRes.iGroupNo, pkItemName);
			if(pkItemName)
			{
				BM::vstring vStr(TTW(799557));
				vStr.Replace(L"#ITEM_NAME#", pkItemName);
				::Notice_Show(vStr, EL_Warning);
			}
			return;
		}

		int const iMod = kRes.iCount-(kAddItem.Count()+iTotalCount);
		if(iMod < 0)
		{
			//등록가능한 수량을 초과했습니다.
			if( bDivideReserve )
			{
				PgActor * pkActor = g_kPilotMan.GetPlayerActor();
				if( pkActor )
				{
					BM::Stream kPacket;
					kPacket.Push(pParent->BuildIndex());	//NeedSlotIdx
					kPacket.Push(pWnd->BuildIndex());		//SrcSlotIdx
					pkActor->AddDivideReservedTransit(EIDRT_JOBSKILL3_CREATE, kItemPos, iMod * -1, kPacket);
				}
			}
			return;
		}

		pWnd->SetCustomData(&kItemPos, sizeof(kItemPos));
		JS3_RefreshResItem();
	}

	void JS3_AddResItem(SItemPos const& kItemPos)
	{
		if(g_kJobSkillRecipe.iItemNo == 0)
		{
			return;
		}

		if(g_bJS3_CreateItemSend)
		{ 
			return; 
		}

		XUI::CXUI_Wnd* pParent = XUIMgr.Get(JOBSKILL3_ITEM_CREATE);
		if(!pParent)
		{
			return;
		}
		
		PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer || !pkPlayer->GetInven() )
		{
			return;
		}
		PgInventory * const pkInv = pkPlayer->GetInven();

		PgBase_Item kAddItem;
		if(S_OK != pkInv->GetItem(kItemPos, kAddItem))
		{
			return;
		}

		CONT_DEF_JOBSKILL_ITEM_UPGRADE::mapped_type kItemUpgrade;
		JobSkill_Util::GetJobSkill_Item(kAddItem.ItemNo(), kItemUpgrade);
		for(int i=0; i<MAX_JS3_RECIPE_RES; ++i)
		{
			SJobSkillRes const & kRes = g_kJobSkillRecipe.kResource[i];
			if( false==JobSkill_Third::UseResItem(kRes.iGroupNo, kRes.iGrade, kAddItem.ItemNo(), kItemUpgrade.iResourceGroupNo, kItemUpgrade.iGrade) )
			{ continue; }

			BM::vstring vStr("FRM_NEED_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pkNeed = pParent->GetControl(vStr);
			if(!pkNeed)
			{ continue; }

			XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkNeed->GetControl(L"BLD_SRC_SLOT"));
			if(!pkBuild)
			{ continue; }

			int const iCount  = pkBuild->CountX();
			for(int i=0; i<iCount; ++i)
			{
				BM::vstring vStr(L"FRM_SRC_SLOT");
				vStr += i;
				XUI::CXUI_Wnd* pkSlot = pkNeed->GetControl(vStr);
				pkSlot = pkSlot ? pkSlot->GetControl(L"ICON_SRC_SLOT") : 0;
				if(pkSlot)
				{
					SItemPos kSlotItemPos;
					pkSlot->GetCustomData(&kSlotItemPos, sizeof(kSlotItemPos));
					if(SItemPos::NullData() == kSlotItemPos)
					{
						JS3_AddResItem(pkSlot, kItemPos);
						break;
					}
				}
			}
		}
	}

	float const fJS3MaxMoveAniTime = 0.7f;
	float const fJS3MaxSendTime = fJS3MaxMoveAniTime+0.2f;
	float g_fJS3_CreateItemSendTime = 0.f;
	void lwJS3_DoActionCreateItem(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd();
		if( !pkWnd )
		{
			return;
		}

		if(g_bJS3_CreateItemSend)
		{
			lwSetJobSkill3CreateItem(kWnd, 0);
		}
		else
		{
			g_fJS3_CreateItemSendTime = 0.f;

			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
			pButton ? pButton->Disable(true) : 0;

			XUI::CXUI_Wnd* pkTmp = pkWnd->GetControl(L"IMG_ARROW_PLAY");
			pkTmp ? pkTmp->Visible(true) : 0;

			//합체 연출 설정!!
			XUI::CXUI_Wnd* pkDummy = pkWnd->GetControl(L"DMY_TRG_ITEM");
			if( pkDummy )
			{
				int const iCenterX = pkDummy->Size().x / 2;

				int iIdx = 0;
				PgMoveAnimateWnd* pkMoveTarget = dynamic_cast<PgMoveAnimateWnd*>(pkDummy->GetControl(JOBSKILL3_TRG_SLOT+iIdx));
				while( pkMoveTarget )
				{
					//
					NiPoint3 kStartLoc(pkMoveTarget->Location().x,pkMoveTarget->Location().y,pkMoveTarget->Location().z);
					NiPoint3 kEndLoc(kStartLoc);
					kEndLoc.x = iCenterX-(pkMoveTarget->Size().x/2);
					pkMoveTarget->Setup(kStartLoc, kEndLoc, fJS3MaxMoveAniTime);

					//
					pkMoveTarget->ClearCustomData();

					XUI::CXUI_Wnd* pkRemain = pkMoveTarget->GetControl(L"IMG_REMAIN");
					pkRemain ? pkRemain->Visible(true) : 0;

					XUI::CXUI_Wnd* pkAni = pkMoveTarget->GetControl(L"IMG_SLOT_ANI");
					pkAni ? pkAni->DefaultImgName( GetTargetImgPath(0, 0) ) : 0;

					XUI::CXUI_Wnd* pkCount = pkMoveTarget->GetControl(L"IMG_COUNT");
					pkCount ? pkCount->Visible(false) : 0;

					//
					++iIdx;
					pkMoveTarget = dynamic_cast<PgMoveAnimateWnd*>(pkDummy->GetControl(JOBSKILL3_TRG_SLOT+iIdx));
				}

				pkMoveTarget = dynamic_cast<PgMoveAnimateWnd*>(pkDummy->GetControl(JOBSKILL3_TRG_SLOT+L"_REMAIN"));
				if( pkMoveTarget )
				{
					NiPoint3 kStartLoc(pkMoveTarget->Location().x,pkMoveTarget->Location().y,pkMoveTarget->Location().z);
					NiPoint3 kEndLoc(kStartLoc);
					kEndLoc.x = iCenterX-(pkMoveTarget->Size().x/2);
					pkMoveTarget->Setup(kStartLoc, kEndLoc, fJS3MaxMoveAniTime);
				}
			}

			lwPlaySoundByID("Enchant");
		}
	}

	void JS3_Send()
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(JOBSKILL3_ITEM_CREATE);
		int const iSaveIdx = GetJobSkill3CreateSaveIdx(pkWnd);
		CONT_JS3_RESITEM_INFO kContResItemInfo;
		if(iSaveIdx && GetContResItemInfo(pkWnd, kContResItemInfo))
		{
			BM::Stream kPacket(PT_C_M_REQ_JOBSKILL3_CREATEITEM);
			kPacket.Push(iSaveIdx);
			kPacket.Push(kContResItemInfo);
			NETWORK_SEND(kPacket)
		}
	}

	void lwJS3_UpdateSendCreateItem(lwUIWnd kWnd)
	{
		if( !g_pkWorld ) { return; }

		XUI::CXUI_Wnd* pkWnd = kWnd();
		if( !pkWnd ) { return; }

		if(false==g_bJS3_CreateItemSend)
		{
			g_fJS3_CreateItemSendTime += g_pkWorld->GetFrameTime();
			if(g_fJS3_CreateItemSendTime > fJS3MaxSendTime)
			{
				g_bJS3_CreateItemSend = true;
				JS3_Send();
			}
		}
	}

	void JS3_RecvCreateItem(BM::Stream & rkPacket)
	{
		HRESULT hResult = S_OK;
		HRESULT hResult2 = S_OK;
		DWORD iSaveIdx = 0;
		DWORD iResultItemNo = 0;
		int iResultItemCount = 0;
		rkPacket.Pop(hResult);
		rkPacket.Pop(hResult2);
		rkPacket.Pop(iSaveIdx);
		rkPacket.Pop(iResultItemNo);
		rkPacket.Pop(iResultItemCount);

		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(JOBSKILL3_ITEM_CREATE);
		if( !pkWnd )
		{
			return;
		}

		lwSetReqInvWearableUpdate(true);
		ClearJobSkill3Slot(pkWnd);
		if(S_OK != hResult)
		{
			int iTTWNo = 0;
			switch(hResult)
			{
			case E_JS3_CREATEITEM_NOT_LEARN_NEEDSKILL:				{ iTTWNo = 799812;	}break;
			case E_JS3_CREATEITEM_NOT_ENOUGH_NEED_PRODUCTPOINT:		{ iTTWNo = 799810;	}break;
			case E_JS3_CREATEITEM_NOT_FOUND_ITEM:					{ iTTWNo = 799813;	}break;
			case E_JS3_CREATEITEM_NOT_ENOUGH_NEED_ITEM:				{ iTTWNo = 1501;	}break;
			case E_JS3_CREATEITEM_NOT_ENOUGH_INVENTORY:				{ iTTWNo = 2854;	}break;
			}
			iTTWNo ? ::Notice_Show_ByTextTableNo(iTTWNo, EL_Warning) : 0;

			lwSetJobSkill3CreateItem(lwUIWnd(pkWnd), iSaveIdx);
			return;
		}

		bool const bDestory = (0==iResultItemNo);
		XUI::CXUI_Wnd* pkDummy = pkWnd->GetControl(L"DMY_TRG_ITEM");
		if(pkDummy)
		{
			pkDummy->ClearOutside();
			int const iCenterX = pkDummy->Size().x / 2;
			XUI::CXUI_Wnd * pkChild = XUIMgr.AddChildUI(pkDummy, JOBSKILL3_TRG_SLOT, JOBSKILL3_TRG_SLOT+0, true);
			if(pkChild)
			{
				int const iChildHalfWidth = pkChild->Size().x/2;
				XUI::CXUI_Wnd* pkSlot = pkChild->GetControl(L"ICON_TRG_SLOT");
				if(pkSlot)
				{
					pkSlot->SetCustomData(&iResultItemNo, sizeof(iResultItemNo));
				}

				XUI::CXUI_Wnd* pkClosed = pkChild->GetControl(L"IMG_CLOSED");
				pkClosed ? pkClosed->Visible(bDestory) : 0;

				XUI::CXUI_Wnd* pkAni = pkChild->GetControl(L"IMG_SLOT_ANI");
				if(pkAni)
				{
					pkAni->DefaultImgName( GetTargetImgPath(0, 0) );
				}

				XUI::CXUI_Wnd* pkCount = pkChild->GetControl(L"IMG_COUNT");
				if(pkCount)
				{
					pkCount->Visible(true);
					pkCount->Text(BM::vstring(iResultItemCount));
				}

				POINT3I kLoc(pkChild->Location());
				kLoc.x = iCenterX-iChildHalfWidth;
				pkChild->Location(kLoc);
			}

			PgUIModel* pkModel = g_kUIScene.FindUIModel("JS3_CREATE_ITEM_EFFECT");
			if( pkModel )
			{
				pkModel->SetNIFEnableUpdate("Success", true);
				pkModel->ResetNIFAnimation("Success");
				pkModel->SetEnableUpdate(true);
			}
		}

		lwPlaySoundByID(bDestory ? "EnchantFail" : "EnchantSuccess");

		//화살표 변경
		XUI::CXUI_Wnd* pkTmp = NULL;
		pkTmp = pkWnd->GetControl(L"IMG_ARROW_INIT");
		pkTmp ? pkTmp->Visible(false) : 0;

		pkTmp = pkWnd->GetControl(L"IMG_ARROW_USE");
		pkTmp ? pkTmp->Visible(true) : 0;

		pkTmp = pkWnd->GetControl(L"IMG_ARROW_PLAY");
		pkTmp ? pkTmp->Visible(false) : 0;

		//설명 문구 변경
		pkTmp = pkWnd->GetControl(L"IMG_BOTTOM_BG");
		pkTmp ? pkTmp->Text(TTW(bDestory ? 799823 : 799819)) : 0;

		XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
		if( pButton )
		{
			BM::vstring vStr(TTW(50601));
			vStr += TTW(2200);
			pButton->Text(vStr);
			pButton->Disable(false);
		}
	}

	bool lwIsJobSkill3Learn()
	{
		PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer )
		{
			return false;
		}

		CONT_DEF_JOBSKILL_SKILL const* pkContDefJobSkill = NULL;
		g_kTblDataMgr.GetContDef(pkContDefJobSkill);
		if(!pkContDefJobSkill || !pkContDefJobSkill->size())
		{
			return false;
		}
		CONT_DEF_JOBSKILL_SKILL::const_iterator iter_Job = pkContDefJobSkill->begin();
		while( iter_Job != pkContDefJobSkill->end() )
		{
			int const iJobSkillNo = (*iter_Job).first;
			CONT_DEF_JOBSKILL_SKILL::mapped_type const &rkJobSkillInfo = (*iter_Job).second;

			if( JST_3RD_MAIN == rkJobSkillInfo.eJobSkill_Type )
			{//주스킬의 경우만 받아서 Add하기
				if( JobSkill_LearnUtil::IsEnableUseJobSkill( pPlayer, iJobSkillNo) )
				{
					return true;
				}
			}
			++iter_Job;
		}
		
		return false;
	}
	
	void lwLearnFilter_JobSkillItem(bool const bCheck)
	{
		lwShowList(g_kCurrentPage, g_kCurrentGatherType, bCheck);
	}
	void FilterOnlyLearn(CONT_DEF_JOBSKILL_SAVEIDX& rkContSaveIndex)
	{
		PgPlayer const * pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return;
		}
		CONT_DEF_JOBSKILL_SAVEIDX::iterator iter = rkContSaveIndex.begin();
		while( iter != rkContSaveIndex.end() )
		{
			int const iIdx = (*iter).first;
			if( false == JobSkillSaveIdxUtil::IsUseableSaveIdx(pPlayer, iIdx) )
			{
				rkContSaveIndex.erase(iter++);
				continue;
			}
			++iter;
		}
	}
	
	bool IsOnlyLearnView()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(JOBSKILL_MAIN);
		if(!pMain)
		{
			return false;
		}
		XUI::CXUI_Wnd* pCBtnWnd = pMain->GetControl(CBTN_FILTER_ONLY_LEARN);
		if(!pCBtnWnd)
		{
			return false;
		}
		XUI::CXUI_CheckButton* pCBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pCBtnWnd);
		if(!pCBtn)
		{
			return false;
		}		
		return pCBtn->Check();
	}
}