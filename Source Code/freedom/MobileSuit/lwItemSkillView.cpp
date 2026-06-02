#include "stdafx.h"
#include "Variant/ItemSkillUtil.h"
#include "lwItemSkillView.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "ServerLib.h"

namespace lwItemSkillUI
{	
	//메인
	std::wstring const ITEMSKILL_MAIN(L"SFRM_ITEM_SKILL");
	std::wstring const ITEMSKILL_SHADOW(L"SFRM_SHADOW");
	std::wstring const ITEMSKILL_LIST(L"LST_CHANNEL");

	std::wstring const ITEMSKILL_ICON(L"SFRM_ICON");
	std::wstring const ITEMSKILL_TEXT(L"SFRM_TEXT");
	std::wstring const TEXT_TITLE(L"FRM_TEXT_TITLE");
	std::wstring const TEXT_INFO(L"FRM_TEXT_INFO");

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "CallItemSkillUI", lwCallItemSkillUI);
	}
	//UI 열기
	void lwCallItemSkillUI()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Activate(ITEMSKILL_MAIN);
		if(!pMain)
		{
			return;
		}
		UpdateList();
	}
	//목록 갱신
	void UpdateList()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(ITEMSKILL_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pShadow = pMain->GetControl(ITEMSKILL_SHADOW);
		if(!pShadow)
		{
			return;
		}
		XUI::CXUI_Wnd* pListWnd = pShadow->GetControl(ITEMSKILL_LIST);
		if(!pListWnd)
		{
			return;
		}
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
		if(!pList)
		{
			return;
		}
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return;
		}
		std::set<int> kContSkillNo;
		ItemSkillUtil::GetAllItemSkillFromEquipItem(pkPlayer, kContSkillNo, IT_FIT);
		ItemSkillUtil::GetAllItemSkillFromEquipItem(pkPlayer, kContSkillNo, IT_FIT_CASH);

		ClearList();

		std::set<int>::const_iterator skill_iter = kContSkillNo.begin();
		while( skill_iter != kContSkillNo.end())
		{
			int const iExplainNo = GetExplainNo( (*skill_iter) );
			if( 0 != iExplainNo )
			{
				if( ItemSkillUtil::CheckUseItemSkill(pkPlayer, (*skill_iter), IT_FIT)
				||  ItemSkillUtil::CheckUseItemSkill(pkPlayer, (*skill_iter), IT_FIT_CASH) )
				{
					XUI::SListItem* pListItem = NULL;
					pListItem = pList->AddItem(L"");
					if(pListItem)
					{
						SetSkill(pListItem, (*skill_iter), iExplainNo);
					}
				}
			}
			++skill_iter;
		}
	}
	//스킬 정보 설정
	void SetSkill( XUI::SListItem* pListItem, int const iSkillNo, int const iExplainNo )
	{
		if(!pListItem)
		{
			return;
		}
		XUI::CXUI_Wnd* pItem = pListItem->m_pWnd;
		if(pItem)
		{
			SetIcon(pItem, iSkillNo);
			SetInfo(pItem, iSkillNo, iExplainNo);
		}
	}
	//스킬정보 - 아이콘 설정
	void SetIcon( XUI::CXUI_Wnd* pItem, int const iSkillNo )
	{
		if(!pItem)
		{
			return;
		}
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
		if(!pSkillDef)
		{
			return;
		}
		XUI::CXUI_Wnd* pIconForm = pItem->GetControl(ITEMSKILL_ICON);
		if(!pIconForm)
		{
			return;
		}
		XUI::CXUI_Wnd* pIconWnd = pIconForm->GetControl(L"Skill_Icon");
		if(!pIconWnd)
		{
			return;
		}		
		XUI::CXUI_Icon* pIcon = dynamic_cast<XUI::CXUI_Icon*>(pIconWnd);
		if(pIcon)
		{
			SIconInfo kIconInfo = pIcon->IconInfo();
			kIconInfo.iIconKey = iSkillNo;
			kIconInfo.iIconResNumber = pSkillDef->RscNameNo();
			pIcon->SetIconInfo(kIconInfo);
		}
	}
	//스킬정보 - 텍스트 설정
	void SetInfo( XUI::CXUI_Wnd* pItem, int const iSkillNo, int const iExplainNo )
	{
		if(!pItem)
		{
			return;
		}
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
		if(!pSkillDef)
		{
			return;
		}
		XUI::CXUI_Wnd* pText = pItem->GetControl(ITEMSKILL_TEXT);
		if(pText)
		{
			XUI::CXUI_Wnd* pTitle = pText->GetControl(TEXT_TITLE);
			if(pTitle)
			{
				const wchar_t *pSkillName = NULL;
				if( GetDefString(pSkillDef->NameNo(), pSkillName) )
				{
					pTitle->Text(pSkillName);
				}
			}
			XUI::CXUI_Wnd* pInfo = pText->GetControl(TEXT_INFO);
			if(pInfo)
			{
				const wchar_t *pSkillInfo = NULL;
				if( GetDefString(iExplainNo, pSkillInfo) )
				{
					pInfo->Text(pSkillInfo);
				}
			}
		}
	}
	//목록 초기화
	void ClearList()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Activate(ITEMSKILL_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pShadow = pMain->GetControl(ITEMSKILL_SHADOW);
		if(!pShadow)
		{
			return;
		}
		XUI::CXUI_Wnd* pListWnd = pShadow->GetControl(ITEMSKILL_LIST);
		if(!pListWnd)
		{
			return;
		}
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
		if(!pList)
		{
			return;
		}

		pList->ClearList();
	}

	//아이템스킬의 설명 텍스트 설정
	int GetExplainNo(int const iSkillNo)
	{
		GET_DEF( CSkillDefMgr, kSkillDefMgr);
		CSkillDef const *pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
		if(!pkSkillDef)
		{
			return 0;
		}
		int const iTextNo = pkSkillDef->GetAbil(AT_ITEM_SKILL_EXPLAIN_ID);

		return iTextNo;
	}
}