#include "stdafx.h"
#include "lwHelpSystem.H"
#include <shlobj.h>
#include "lwGUID.H"
#include "BM/filesupport.h"
#include "lwHelpObject.H"
#include "lwUI.h"
#include "Pg2DString.h"

using namespace lua_tinker;

lwHelpSystem	lwGetHelpSystem()
{
	return	lwHelpSystem(&g_kHelpSystem);
}

void OnClickDetailHelpView(int const iHelpID, bool bFromMainList)
{
	PgHelpObject const& kObject = g_kHelpSystem.GetHelpObject(iHelpID, bFromMainList);
	
	if( 0 == kObject.GetID() && kObject.GetUIType() != 18 ){ return; }

	bool bIsModal = false;
	if(bFromMainList)
	{
		for(int i = 0; i<6; ++i)
		{
			BM::vstring kName(L"FRM_BOOK");
			if(0<i)	//기본페이지는 여전히 "FRM_BOOK" 이다
			{
				kName+=i;
			}
			bIsModal = (NULL!=XUIMgr.Get((std::wstring const&)kName));	//북이 떠 있으면 모달로
			if(bIsModal)
			{
				break;
			}
		}
	}

	XUI::CXUI_Wnd* pkDetailTop = XUIMgr.Call(L"FRM_HELP_BIG", true);
	if( !pkDetailTop ){ return; }
	pkDetailTop->Priority(97);

	pkDetailTop->SetCustomData(&iHelpID, sizeof(iHelpID));

	XUI::CXUI_Wnd* pkTitle = pkDetailTop->GetControl(L"FRM_TITLE");
	if( pkTitle )
	{
		pkTitle->Text(TTW(kObject.GetExTitleTTID()));
	}

	XUI::CXUI_List* pkContents = dynamic_cast<XUI::CXUI_List*>(pkDetailTop->GetControl(L"LST_CONTENTS"));
	if( pkContents )
	{
		if( 0 == pkContents->GetTotalItemCount() )
		{
			pkContents->AddItem(L"");
		}

		XUI::SListItem* pkItem = pkContents->FirstItem();
		if( pkItem && pkItem->m_pWnd )
		{
			pkItem->m_pWnd->Text(TTW(kObject.GetExMessageTTID()));
			XUI::CXUI_Style_String kString = pkItem->m_pWnd->StyleText();
			POINT2 kTxtSize(Pg2DString::CalculateOnlySize(kString, pkItem->m_pWnd->Size().x));
			pkItem->m_pWnd->Size(pkItem->m_pWnd->Size().x, kTxtSize.y);
		}
	}
	XUI::CXUI_Image* pkImage = dynamic_cast<XUI::CXUI_Image*>(pkDetailTop->GetControl(L"IMG_PREVIEW"));
	if( pkImage )
	{
		pkImage->DefaultImgName(UNI(kObject.GetPreviewImgPath()));
	}

	bool bVisibleHotLink = (kObject.GetExType() != 0)?(true):(false);
	if( bVisibleHotLink && kObject.GetRelatedUIName().empty() )
	{
		bVisibleHotLink = false;
	}
	XUI::CXUI_Wnd* pkBtn = pkDetailTop->GetControl(L"BTN_HOTLINK");
	if( pkBtn )
	{
		pkBtn->Visible(bVisibleHotLink);
		pkBtn->Text(TTW(kObject.GetExBtnTTID()));
		int iTemp = 0;
		pkBtn->SetCustomData(&iHelpID, sizeof(bFromMainList?iHelpID:iTemp));
	}

	XUI::CXUI_Wnd* pkBtnClose = pkDetailTop->GetControl(L"BTN_CLOSELINK");
	if( pkBtnClose )
	{
		pkBtnClose->Location(bVisibleHotLink?270:156, pkBtnClose->Location().y);
	}
}

void lwOnCall_HelpUI2(lwHelpSystem kSystem, lwHelpObject kObject)
{
	lwCloseUI("FRM_HELP_CONTROL");
	lwCloseUI("FRM_HELP_MINI");

	XUI::CXUI_Wnd* pkMain = XUIMgr.Get(L"FRM_HELP_CENTER");
	if( !pkMain )
	{
		pkMain = XUIMgr.Call(L"FRM_HELP_CENTER");
		if( !pkMain )
		{
			return;
		}
	}
	
	pkMain->AliveTime(18000);
	pkMain->RefreshCalledTime();
	int const HELPID = kObject.GetID();
	pkMain->SetCustomData(&HELPID, sizeof(HELPID));

	XUI::CXUI_Wnd* pkIcon = pkMain->GetControl(L"AAA");
	if( pkIcon )
	{//아이콘이 있을 경우에만.
	}

	XUI::CXUI_Wnd* pkText = pkMain->GetControl(L"FRM_TEXT");
	if( pkText )
	{
		BM::vstring vStr(TTW(kObject.GetTitleTTID()));
		vStr += L"\n";
		vStr += L"{T=FTcts_13/C=0xFFFFE9AF/}";
		vStr += TTW(kObject.GetMessageTTID());
		pkText->Text(vStr);

		XUI::CXUI_Style_String kStr = pkText->StyleText();
		POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kStr));
		POINT2 const kTextPos(pkText->TextPos().x, (pkText->Size().y - kTextSize.y) / 2);
		pkText->TextPos(kTextPos);
	}
}

void lwOnClickDetailHelpView(lwUIWnd kParent, bool bDontClose)
{
	XUI::CXUI_Wnd* pParent = kParent.GetSelf();
	if( !pParent ){ return; }

	int iHelpID = 0;
	pParent->GetCustomData(&iHelpID, sizeof(iHelpID));
	if(false==bDontClose)
	{
		pParent->Close();
	}
	OnClickDetailHelpView(iHelpID);
}

void lwOnClickDetailHelpViewMain(lwUIWnd kParent, bool bDontClose)
{
	XUI::CXUI_Wnd* pParent = kParent.GetSelf();
	if( !pParent ){ return; }

	int iHelpID = 0;
	pParent->GetCustomData(&iHelpID, sizeof(iHelpID));
	if(false==bDontClose)
	{
		pParent->Close();
	}
	OnClickDetailHelpView(iHelpID, true);
}

void lwOnClickExBtn(lwUIWnd kParent, bool const bFromList = false)
{
	XUI::CXUI_Wnd* pParent = kParent.GetSelf();
	if( !pParent ){ return; }

	int iHelpID = 0;
	pParent->GetCustomData(&iHelpID, sizeof(iHelpID));
	pParent->Close();

	PgHelpObject const& kObject = g_kHelpSystem.GetHelpObject(iHelpID, bFromList);
	
	if( 0 == kObject.GetID() ){ return; }

	switch( kObject.GetExType() )
	{
	case HAET_UI_LINK:
		{
			lua_tinker::call<void, char const*>("DirectOpenUI", kObject.GetRelatedUIName().c_str());
		}break;
	case HAET_OTHER_HELP:
		{
			int iHelpID = atoi(kObject.GetRelatedUIName().c_str());
			OnClickDetailHelpView(iHelpID);
		}break;
	default:
		return;
	}
}

void lwHelpSystem::RegisterWrapper(lua_State *pkState)
{

	def(pkState, "GetHelpSystem", &lwGetHelpSystem);
	def(pkState, "OnCall_HelpUI2", &lwOnCall_HelpUI2);
	def(pkState, "OnClickDetailHelpView", &lwOnClickDetailHelpView);
	def(pkState, "OnClickDetailHelpViewMain", &lwOnClickDetailHelpViewMain);
	def(pkState, "OnClickDetailHelpViewMainByID", &OnClickDetailHelpView);
	def(pkState, "OnClickExBtn", &lwOnClickExBtn);

	LW_REG_CLASS(HelpSystem)
		LW_REG_METHOD(HelpSystem, ActivateByConditionString)
		LW_REG_METHOD(HelpSystem, ActivateByConditionInt)
		LW_REG_METHOD(HelpSystem, ActivateNextObject)
		LW_REG_METHOD(HelpSystem, ActivatePrevObject)
		LW_REG_METHOD(HelpSystem, LoadHelpInfoFile)
		LW_REG_METHOD(HelpSystem, SaveHelpInfoFile)
		LW_REG_METHOD(HelpSystem, GetActivatedObject)
		LW_REG_METHOD(HelpSystem, ResetActivatingCount)
		LW_REG_METHOD(HelpSystem, InitHelpMain)
		LW_REG_METHOD(HelpSystem, OnClickHelpBeginPage)
		LW_REG_METHOD(HelpSystem, OnClickHelpEndPage)
		LW_REG_METHOD(HelpSystem, OnClickHelpPage)
		LW_REG_METHOD(HelpSystem, OnClickHelpNextPage)
		;

	def(pkState, "RegistHelperGuid", Helper::RegistHelperGuid);
	def(pkState, "RemoveHelperGuid", Helper::RemoveHelperGuid);
	def(pkState, "GetHelperGuid", Helper::GetHelperGuid);
}

//	조건에 의해 헬프 오브젝트를 활성화 시킨다.
void	lwHelpSystem::ActivateByConditionString(char const *kConditionName,char const* kConditionValue)
{
	g_kHelpSystem.ActivateByCondition(kConditionName,std::string(kConditionValue));
}
void	lwHelpSystem::ActivateByConditionInt(char const *kConditionName,int iConditionValue)
{
	g_kHelpSystem.ActivateByCondition(kConditionName,iConditionValue);
}
//	현재 활성화 되어있는 오브젝트를 얻는다.
lwHelpObject	lwHelpSystem::GetActivatedObject()
{
	return	lwHelpObject(g_kHelpSystem.GetActivatedObject());
}

//	다음,이전 헬프 오브젝트를 활성화 시킨다.
void	lwHelpSystem::ActivateNextObject()
{
	g_kHelpSystem.ActivateNextObject();
}
void	lwHelpSystem::ActivatePrevObject()
{
	g_kHelpSystem.ActivatePrevObject();
}
void	lwHelpSystem::LoadHelpInfoFile(lwGUID kCharGUID)
{
	g_kHelpSystem.LoadHelpInfoFile( kCharGUID() );
	
}
void	lwHelpSystem::SaveHelpInfoFile(lwGUID kCharGUID)
{
	g_kHelpSystem.SaveHelpInfoFile( kCharGUID() );
}

void	lwHelpSystem::ResetActivatingCount()
{
	BM::GUID kHelperGuid = g_kHelpSystem.HelperGuid();
	g_kHelpSystem.Clear();
	g_kHelpSystem.HelperGuid(kHelperGuid);
}


void	Helper::RegistHelperGuid(lwGUID kGuid)
{
	g_kHelpSystem.HelperGuid(kGuid());
}
void	Helper::RemoveHelperGuid()
{
	g_kHelpSystem.HelperGuid(BM::GUID::NullData());
}
lwGUID	Helper::GetHelperGuid()
{
	return lwGUID(g_kHelpSystem.HelperGuid());
}

void	lwHelpSystem::InitHelpMain(lwUIWnd kWnd)
{
	if(kWnd.IsNil())	{return;}

	g_kHelpSystem.SortHelpObjectList();
	HelpObjectList const& rkList = g_kHelpSystem.GetHelpObjectList();
	PgPage& rkPage = g_kHelpSystem.GetHelpPage();
	bool const bShowPageBtn = rkList.size() > static_cast<size_t>(rkPage.GetMaxItemSlot()*rkPage.GetMaxViewPage());

	static char const szBtn[8][30] = {"BTN_FIRST", "BTN_PREV", "BTN_NEXT", "BTN_END", "BTN_FIRST_BG", "BTN_PREV_BG", "BTN_NEXT_BG", "BTN_END_BG"};

	for(int i = 0; i<8; ++i)
	{
		lwUIWnd kBtn = kWnd.GetControl(szBtn[i]);
		if(false==kBtn.IsNil())
		{
			kBtn.Visible(bShowPageBtn);
		}
	}
}

namespace lwHomeUIUtil
{
	void CommonPageControl(XUI::CXUI_Wnd* pPageMainUI, PgPage const& kPage);
}

void	lwHelpSystem::OnClickHelpBeginPage(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
	while( pMainUI->Parent() )
	{
		pMainUI = pMainUI->Parent();
	}

	PgPage& kPage = g_kHelpSystem.GetHelpPage();
	int const NowPage = kPage.Now();
	if( NowPage == kPage.PageBegin() )
	{
		return;
	}

	SetHelpList(pMainUI);
	lwHomeUIUtil::CommonPageControl(pMainUI->GetControl(L"FRM_PAGE"), kPage);
}

void	lwHelpSystem::OnClickHelpEndPage(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
	while( pMainUI->Parent() )
	{
		pMainUI = pMainUI->Parent();
	}

	PgPage& kPage = g_kHelpSystem.GetHelpPage();
	int const NowPage = kPage.Now();
	if( NowPage == kPage.PageEnd() )
	{
		return;
	}

	SetHelpList(pMainUI);
	lwHomeUIUtil::CommonPageControl(pMainUI->GetControl(L"FRM_PAGE"), kPage);;
}

void lwHelpSystem::OnClickHelpPage(lwUIWnd kSelf)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	if( kSelf.GetCheckState() )
	{
		return;
	}

	int const iBuildIndex = pSelf->BuildIndex();

	XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
	while( pMainUI->Parent() )
	{
		pMainUI = pMainUI->Parent();
	}

	PgPage& kPage = g_kHelpSystem.GetHelpPage();
	int const NowPage = kPage.Now();
	int iNewPage = (NowPage / kPage.GetMaxViewPage()) * kPage.GetMaxViewPage() + iBuildIndex;

	if( NowPage == iNewPage )
	{
		return;
	}

	if( iNewPage >= kPage.Max() )
	{
		iNewPage = kPage.Max() - 1;
	}

	kPage.PageSet(iNewPage);

	SetHelpList(pMainUI);
	lwHomeUIUtil::CommonPageControl(pMainUI->GetControl(L"FRM_PAGE"), kPage);
}

void lwHelpSystem::OnClickHelpNextPage(lwUIWnd kSelf, bool bPrev)
{
	XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
	if( !pSelf )
	{
		return;
	}

	XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
	while( pMainUI->Parent() )
	{
		pMainUI = pMainUI->Parent();
	}

	PgPage& kPage = g_kHelpSystem.GetHelpPage();
	int const NowPage = kPage.Now() + 1;
	if( NowPage == (bPrev?kPage.PagePrevJump():kPage.PageNextJump()) )
	{
		return;
	}

	SetHelpList(pMainUI);
	lwHomeUIUtil::CommonPageControl(pMainUI->GetControl(L"FRM_PAGE"), kPage);
}

void lwHelpSystem::SetHelpList(XUI::CXUI_Wnd* pMainUI)
{
	if( !pMainUI )
	{
		return;
	}

	XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pMainUI->GetControl(L"BLD_BTN"));
	if( !pBuild )
	{
		return;
	}

	int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();

	HelpObjectList kObj;
	g_kHelpSystem.GetHelpDisplayItem(kObj);
	
	HelpObjectList::const_iterator it = kObj.begin();

	std::wstring kTitleString;
	int iObjID = 0;

	for(int i = 0; i < MAX_SLOT; ++i)
	{
		iObjID = 0;
		BM::vstring vStr(L"BTN_HELP_SLOT");
		vStr += i;

		XUI::CXUI_Wnd* pSlot = pMainUI->GetControl(vStr);
		if( !pSlot )	{++it;continue;}

		kTitleString.clear();	
	
		if( it != kObj.end() )
		{
			kTitleString = TTW((*it)->GetTitleTTID());
			iObjID = (*it)->GetID();
			++it;
		}
		
		pSlot->Text(kTitleString);
		pSlot->SetCustomData(&iObjID, sizeof(iObjID));
	}
}