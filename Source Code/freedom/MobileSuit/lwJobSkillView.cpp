#include "stdafx.h"
#include "Variant/TableDataManager.h"
#include "Variant/PgJobSkill.h"
#include "ServerLib.h"
#include "lwJobSkillView.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"

namespace lwJobSkillView
{
	//메인
	std::wstring const JOBSKILL_MAIN(L"SFRM_JOB_SKILL_VIEW");
	//배울 수 있는 직업 목록
	std::wstring const VIEW_SHADOW(L"SFRM_VIEW_SHADOW");
	std::wstring const VIEW_MAJORSKILL_ICON(L"FRM_MAJORSKILL_ICON");
	std::wstring const VIEW_SKILL(L"FRM_VIEW_");
	std::wstring const VIEW_ICON(L"SFRM_ICON");
	std::wstring const VIEW_ICON_RES(L"JS_Icon");
	std::wstring const VIEW_TEXT(L"SFRM_TEXT");
	std::wstring const VIEW_TEXT_TITLE(L"FRM_TEXT_TITLE");
	std::wstring const VIEW_TEXT_INFO(L"FRM_TEXT_INFO");
	std::wstring const VIEW_NOT_HAVE_SKILL(L"FRM_NOT_HAVE_SKILL");	
	//보조툴 사용 가능 표기
	std::wstring const ENALBE_SUBTOOL(L"IMG_ENALBE_SUBTOOL");
	//배운아이템
	std::wstring const BTN_LEARNED_ITME(L"BTN_LEARNED_ITEM");
	//피로도
	std::wstring const VIEW_TIRED(L"FRM_TIRED");
	//피로도/축복 게이지(공통)
	std::wstring const ANI_GAUGE(L"FRM_GAUGE");

	int const MAX_SHOW_LIST = 5;
	//
	int g_kCurrentPage = 1;
	
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "CallJobSkillViewUI", lwCallJobSkillViewUI);
		def(pkState, "CallToolTip_EnableSubTool", lwCallToolTip_EnableSubTool);
	}

	bool IsActivateMainWnd()
	{
		XUI::CXUI_Wnd* pkWnd = NULL;
		return XUIMgr.IsActivate(JOBSKILL_MAIN, pkWnd);
	}

	void UpdateMainWnd()
	{
		bool const bRet = lwShowList();
		VisibleEmptyText(!bRet);
	}

	void lwCallJobSkillViewUI()
	{
		bool const bRet = lwShowList();
		VisibleEmptyText(!bRet);
	}

	bool lwShowList()
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Activate(JOBSKILL_MAIN);
		if(!pMain)
		{
			return false;
		}
		XUI::CXUI_Wnd* pViewSD = pMain->GetControl(VIEW_SHADOW);
		if(!pViewSD)
		{
			return false;
		}
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)
		{
			return false;
		}
		PgJobSkillExpertness const& rkMyJobSkill = pPlayer->JobSkillExpertness();
		PgJobSkillExpertness::CONT_EXPERTNESS const& rkContJobSkill = rkMyJobSkill.GetAllSkillExpertness();
		if( !rkContJobSkill.size() )
		{//배운게 아무것도 없으면 실패
			return false;
		}
		//대표스킬
		int const iMajorSkill = 85092201;
		BM::vstring kIconText(VIEW_MAJORSKILL_ICON);
		XUI::CXUI_Wnd* pIconForm = pViewSD->GetControl( static_cast<std::wstring>(kIconText) );
		if(!pIconForm)
		{
			return false;
		}
		if( !lwSetIcon( iMajorSkill, pIconForm ) )
		{
			return false;
		}
		//스킬목록
		int const iSkillNum = 8;
		int const arrSkill[iSkillNum] = { 85000100, 85091601,
										  85000300, 85091701,
										  85000500, 85091901,
										  85000700, 85092001 };		//가데이터
		for( int iBuild=0; iSkillNum!=iBuild; ++iBuild )
		{
			BM::vstring kIconText(VIEW_SKILL);
			kIconText += iBuild;
			XUI::CXUI_Wnd* pIconForm = pViewSD->GetControl( static_cast<std::wstring>(kIconText) );
			if(!pIconForm)
			{
				return false;
			}
			if( !lwSetIcon( arrSkill[iBuild], pIconForm ) )
			{
				return false;
			}
			if( !lwSetExpertness( arrSkill[iBuild], pIconForm ) )
			{
				return false;
			}
			lwSetEnableSubSkill(arrSkill[iBuild], pIconForm);
		}

		SetTiredGauge(pPlayer);


		PgJobSkillExpertness::CONT_EXPERTNESS::const_iterator itor_Job = rkContJobSkill.begin();

		return true;
	}
	bool lwSetIcon( int const iSkillNo, XUI::CXUI_Wnd* pSkillForm)
	{
		if( !pSkillForm )	
		{	return false;	}
		XUI::CXUI_Wnd* pIconWnd = pSkillForm->GetControl( VIEW_ICON );
		if(!pIconWnd)	
		{	return false;	}
		XUI::CXUI_Wnd* pIcon = pIconWnd->GetControl( VIEW_ICON_RES );
		if(!pIcon)	
		{	return false;	}
		XUI::CXUI_Icon* pIcon_Res = dynamic_cast<XUI::CXUI_Icon*>(pIcon);
		if(!pIcon_Res)	
		{	return false;	}
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
		if( !pSkillDef )	
		{ 	return false; 	}
		SIconInfo kIconInfo = pIcon_Res->IconInfo();
		kIconInfo.iIconKey = iSkillNo;
		kIconInfo.iIconResNumber = pSkillDef->RscNameNo();
		pIcon_Res->SetIconInfo(kIconInfo);
		return true;
	}
	bool lwSetExpertness( int const iSkillNo, XUI::CXUI_Wnd* pSkillForm )
	{
		if( !pSkillForm )	
		{	return false;	}
		XUI::CXUI_Wnd* pTextWnd = pSkillForm->GetControl( VIEW_TEXT );
		if(!pTextWnd)	
		{	return false;	}
		XUI::CXUI_Wnd* pTitleWnd = pTextWnd->GetControl( VIEW_TEXT_TITLE );
		if(!pTitleWnd)	
		{	return false;	}
		XUI::CXUI_Wnd* pInfoWnd = pTextWnd->GetControl( VIEW_TEXT_INFO );
		if(!pInfoWnd)	
		{	return false;	}
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(iSkillNo);
		if( !pSkillDef )	
		{ 	return false; 	}
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pPlayer)	
		{	return false;	}
		CONT_DEF_JOBSKILL_SKILL const* pkContDefJobSkill;
		g_kTblDataMgr.GetContDef(pkContDefJobSkill);
		if(!pkContDefJobSkill)	
		{	return false;	}
		//스킬이름
		wchar_t const* pName = NULL;
		GetDefString(pSkillDef->NameNo(),pName);
		if( !pName )	
		{	return false;	}
		pTitleWnd->Text( pName );
		//숙련도
		PgJobSkillExpertness const& rkMyJobSkill = pPlayer->JobSkillExpertness();
		PgJobSkillExpertness::VALUE_TYPE iCurrentExp;
		if( !rkMyJobSkill.Get( iSkillNo, iCurrentExp) )	{	return false;	}
		BM::vstring kExpText( TTW(799532) );
		kExpText += (iCurrentExp/JSE_EXPERTNESS_DEVIDE);
		pInfoWnd->Text( static_cast<std::wstring>(kExpText) );
		return true;
	}

	void VisibleEmptyText(bool const bVisible)
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(JOBSKILL_MAIN);
		if(!pMain)
		{
			return;
		}

		XUI::CXUI_Wnd* pErrWnd = pMain->GetControl(VIEW_NOT_HAVE_SKILL);
		if(pErrWnd)
		{
			pErrWnd->Visible(bVisible);
		}
	}

	void SetTiredGauge(PgPlayer* pPlayer)
	{
		if(!pPlayer)
		{
			return;
		}
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(JOBSKILL_MAIN);
		if(!pMain)
		{
			return;
		}
		XUI::CXUI_Wnd* pTiredWnd = pMain->GetControl(VIEW_TIRED);
		if(!pTiredWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pGaugeWnd = pTiredWnd->GetControl(ANI_GAUGE);
		if(!pGaugeWnd)
		{
			return;
		}
		XUI::CXUI_AniBar* pAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pGaugeWnd);
		if(!pAniBar)
		{
			return;
		}

		PgJobSkillExpertness const &rkExpertness = pPlayer->JobSkillExpertness();
		int const iMax = JobSkillExpertnessUtil::GetMaxExhaustion_1ST(pPlayer->GetPremium(), rkExpertness.GetAllSkillExpertness());
		int const iNow = rkExpertness.CurExhaustion();
		
		pAniBar->Max(iMax);
		pAniBar->Now(iMax-iNow);
		wchar_t szBuf[MAX_PATH] ={0,};
		wsprintfW(szBuf, L"%d / %d", iMax-iNow, iMax );
		pAniBar->Text(szBuf);
	}
	
	void lwSetEnableSubSkill( int const iSkillNo, XUI::CXUI_Wnd* pSkillForm )
	{
		if( !pSkillForm )
		{ return; }
		XUI::CXUI_Wnd *pImgWnd = pSkillForm->GetControl(ENALBE_SUBTOOL);
		if( !pImgWnd )
		{ return; }
		XUI::CXUI_Image *pImg = dynamic_cast<XUI::CXUI_Image *>(pImgWnd);
		if( !pImg )
		{ return; }

		int const iSubSkillNo = JobSkill_Util::GetJobSkill_SubSkill(iSkillNo);
		if( !iSubSkillNo )
		{
			pImg->Visible(false);
			return;
		}
		PgPlayer const* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{ return; }

		bool const bEnableSubSkill = JobSkill_LearnUtil::IsEnableUseJobSkill(pPlayer, iSubSkillNo);
		pImg->Visible(true);
		SUVInfo kInfo = pImg->UVInfo();
		int const iUVIndex = (true==bEnableSubSkill ? 1 : 2);
		kInfo.Index = iUVIndex ;
		pImg->UVInfo(kInfo);
		pImg->SetCustomData( &iSubSkillNo, sizeof(iSubSkillNo) );
	}
	
	void lwCallToolTip_EnableSubTool( lwUIWnd kWnd, lwPoint2 &pt )
	{
		if( kWnd.IsNil() )
		{ return; }
		int const iSubSkill = kWnd.GetCustomData<int>();
		if( !iSubSkill )
		{ return; }
		CallJobSkillToolTip( iSubSkill, pt );
	}
}