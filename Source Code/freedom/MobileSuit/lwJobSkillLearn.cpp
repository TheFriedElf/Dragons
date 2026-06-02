#include "stdafx.h"
#include "Variant/TableDataManager.h"
#include "ServerLib.h"
#include "lwJobSkillLearn.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "PgJobSkillLearn.h"

namespace lwJobSkillLearn
{	
	//메인
	std::wstring const JOBSKILL_MAIN(L"SFRM_JOB_SKILL_LEARN");
	std::wstring const JOB_SELECT_SHADOW(L"SFRM_LEARN_SHADOW");
	std::wstring const JOB_ICON_WND(L"SFRM_ICON");
	std::wstring const JOB_ICON(L"JS_Icon");
	int const iViewIconCount = 8;

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "CallJobSkillLearnUI", lwCallJobSkillLearnUI);
	}

	void lwCallJobSkillLearnUI()
	{
		//일단 열자
		XUI::CXUI_Wnd* pMain = XUIMgr.Activate(JOBSKILL_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pShadow = pMain->GetControl(JOB_SELECT_SHADOW);
		if(!pShadow)
		{
			return;
		}		
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		int const arrSkillNo[iViewIconCount] = { 85000100, 85000300, 85000500, 85000700,
												85091601, 85091701, 85091901, 85092001};
		for( int iBuild = 0; iViewIconCount != iBuild; ++iBuild )
		{
			BM::vstring  kBuildStr(JOB_ICON_WND);
			kBuildStr += iBuild;
			XUI::CXUI_Wnd* pIconWnd = pShadow->GetControl( static_cast<std::wstring>(kBuildStr) );
			if(!pIconWnd)
			{
				continue;
			}
			XUI::CXUI_Wnd* pIcon = pIconWnd->GetControl(JOB_ICON);
			if(!pIcon)
			{
				continue;
			}
			int const iSkillNo = arrSkillNo[iBuild];
			CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
			if( !pSkillDef ){ continue; }
			XUI::CXUI_Icon* pIcon_Res = dynamic_cast<XUI::CXUI_Icon*>(pIcon);
			if(pIcon_Res)
			{
				SIconInfo kIconInfo = pIcon_Res->IconInfo();
				kIconInfo.iIconKey = iSkillNo;
				kIconInfo.iIconResNumber = pSkillDef->RscNameNo();
				pIcon_Res->SetIconInfo(kIconInfo);
			}
		}
	}

	void ReturnResult(EJOBSKILL_LEARN_RET const & eRet)
	{
		switch(eRet)
		{
		case JSLR_OK:
			{
				::Notice_Show( TTW(799518), EL_Normal );
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799518, true);
			}break;
		case JSLR_ALREADY_BUY:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799519, true);
			}break;
		case JSLR_NOT_CLASS:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799520, true);
			}break;
		case JSLR_NEED_SKILL:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799521, true);
			}break;
		case JSLR_FULL_COUNT:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799525, true);
			}break;
		case JSLR_NEED_MONEY:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 968, true);
			}break;
		case JSLR_ERR:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799523, true);
			}break;
		default:
			{
			}break;
		}
		lwJobSkillLearn::lwCallJobSkillLearnUI();
	}
}


namespace lwJobSkill_NfySaveIdx
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "JSNfySaveIdx_UpdateUI", lwUpdateUI);
	}
	bool CheckNewSaveIdx(int const iJobSkill, int const iBeforeExp, int const iCurrentExp)
	{
		PgPlayer const *pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return false;
		}
		CONT_DEF_JOBSKILL_SAVEIDX const* pkContDefSaveIdx = NULL;
		g_kTblDataMgr.GetContDef(pkContDefSaveIdx);
		if( !pkContDefSaveIdx )
		{
			return false;
		}

		CONT_DEF_JOBSKILL_SAVEIDX::const_iterator itor_saveidx = pkContDefSaveIdx->begin();
		while( pkContDefSaveIdx->end() != itor_saveidx )
		{
			CONT_DEF_JOBSKILL_SAVEIDX::mapped_type const& rkSaveIdx = (*itor_saveidx).second;
			if( (iJobSkill == rkSaveIdx.iNeedSkillNo01) &&
				(iBeforeExp < rkSaveIdx.iNeedSkillExpertness01) &&
				(iCurrentExp >= rkSaveIdx.iNeedSkillExpertness01) &&
				(0 != rkSaveIdx.iShowUser) )
			{
				g_kJS_NfySaveIdxMgr.Push( (*itor_saveidx).first );
			}
			++itor_saveidx;
		}

		return !g_kJS_NfySaveIdxMgr.Empty();
	}
	void lwUpdateUI()
	{
		if( g_kJS_NfySaveIdxMgr.Empty() )
		{
			return;
		}
		XUI::CXUI_Wnd *pWndMain = XUIMgr.Activate(L"FRM_NFY_SAVEIDX");
		if( !pWndMain )
		{ return; }
		int const iSaveIdx = g_kJS_NfySaveIdxMgr.Pop();
		ShowNfySaveIdx( iSaveIdx );
	}

	void ShowNfySaveIdx(int const iSaveIdx)
	{
		if( 0 == iSaveIdx )
		{ return; }
		SJobSkillSaveIdx const *pSaveIdx = JobSkill_Third::GetJobSkillSaveIdx(iSaveIdx);
		if( !pSaveIdx )
		{ return; }
		GET_DEF( CItemDefMgr, kItemDefMgr);
		CItemDef const *pItemDef = kItemDefMgr.GetDef(pSaveIdx->iBookItemNo);
		if( !pItemDef )
		{ return; }
		
		XUI::CXUI_Wnd *pWndMain = XUIMgr.Activate(L"FRM_NFY_SAVEIDX");
		if( !pWndMain )
		{ return; }
		{
			XUI::CXUI_Wnd *pIconWnd = pWndMain->GetControl(L"SFRM_ICON");
			if( !pIconWnd )
			{ return; }
			XUI::CXUI_Wnd *pIcon = pIconWnd->GetControl(L"JS_Icon");
			if( !pIcon )
			{ return; }
			XUI::CXUI_Icon *pIconRes = dynamic_cast<XUI::CXUI_Icon*>(pIcon);
			if( !pIconRes )
			{ return; }
			SIconInfo kIconInfo = pIconRes->IconInfo();
			kIconInfo.iIconKey = pItemDef->NameNo();
			kIconInfo.iIconResNumber = pItemDef->ResNo();
			pIconRes->SetIconInfo(kIconInfo);
			pIconRes->SetCustomData( &(kIconInfo.iIconResNumber), sizeof(kIconInfo.iIconResNumber) );
		}
		{
			XUI::CXUI_Wnd *pTextWnd = pWndMain->GetControl(L"FRM_TEXT");
			if( !pTextWnd )
			{ return; }
			BM::vstring kText( TTW(799701) );
			
			wchar_t const* pItemName = NULL;
			GetDefString(pItemDef->NameNo(), pItemName);
			if( !pItemName )
			{
				return;
			}
			kText.Replace( L"#ITEM_NAME#", pItemName );
			pTextWnd->Text( static_cast<std::wstring>(kText) );
		}
	}
	void ClearContSaveIdx()
	{
		g_kJS_NfySaveIdxMgr.Clear();
	}
}