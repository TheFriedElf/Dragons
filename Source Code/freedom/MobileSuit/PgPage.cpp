#include "StdAfx.h"
#include "PgPage.h"

PgPage::PgPage(void)
	: m_kMaxItem(0)
	, m_kNow(0)
	, m_kMax(0)
{
}

PgPage::~PgPage(void)
{
}

void PgPage::SetPageAttribute(int const MAX_SLOT, int const MAX_PAGE)
{
	M_MAX_SLOT = MAX_SLOT;
	M_MAX_PAGE = MAX_PAGE;
}

void PgPage::SetMaxItem(int const iMaxItem, bool bResetNowPage)
{
	m_kMaxItem = iMaxItem;
	m_kMax = m_kMaxItem / M_MAX_SLOT;
	m_kMax += ( 0 < ( m_kMaxItem % M_MAX_SLOT ) )?(1):(0);
	m_kNow = (!bResetNowPage || (Now() < Max()))?(Now()):(0);
}

bool PgPage::PageCheck(int const iPageIdx) const
{
	return ( 0 <= iPageIdx && Max() > iPageIdx );
}

int PgPage::PageSet(int const iPageIdx) const
{
	if( 0 <= iPageIdx && Max() > iPageIdx )
	{
		m_kNow = iPageIdx;
	}
	return Now() + 1;
}

int PgPage::PagePrev() const
{
	if( 0 < Now() )
	{
		--m_kNow;
	}
	return Now() + 1;
}

int PgPage::PageNext() const
{
	if( Max() > (Now() + 1) )
	{
		++m_kNow;
	}
	return Now() + 1;
}

int PgPage::PagePrevJump() const
{
	if( 0 < Now() )
	{
		int const iStart = Now() / M_MAX_PAGE * M_MAX_PAGE;
		if( 0 > (iStart - M_MAX_PAGE))
		{
			m_kNow = 0;
		}
		else
		{
			m_kNow = iStart - M_MAX_PAGE;
		}
	}
	return Now() + 1;
}

int PgPage::PageNextJump() const
{
	if( Max() > (Now() + 1) )
	{
		int const iStart = Now() / M_MAX_PAGE * M_MAX_PAGE;
		if( Max() < (iStart + M_MAX_PAGE + 1))
		{
			m_kNow = Max() - 1;
		}
		else
		{
			m_kNow = iStart + M_MAX_PAGE;
		}
	}
	return Now() + 1;
}

int PgPage::PageBegin() const
{
	if( 0 < Now() )
	{
		m_kNow = 0;
	}
	return Now() + 1;
}

int PgPage::PageEnd() const
{
	if( Max() > (Now() + 1) )
	{
		m_kNow = Max() - 1;
	}
	return Now() + 1;
}

namespace UIPageUtil
{
	std::wstring const WSTR_BUILDER_TAG = L"BLD_PAGENUM";
	std::wstring const WSTR_PAGE_BTN_TAG = L"CBTN_NUM";
	std::wstring const WSTR_PAGE_START_TAG = L"BTN_FIRST";
	std::wstring const WSTR_PAGE_END_TAG = L"BTN_END";
	std::wstring const WSTR_PAGE_JUMP_P_TAG = L"BTN_PREV";
	std::wstring const WSTR_PAGE_JUMP_N_TAG = L"BTN_NEXT";

	void PageControl(XUI::CXUI_Wnd* pPageMainUI, PgPage const& kPage)
	{
		if( !pPageMainUI )
		{
			return;
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pPageMainUI->GetControl(WSTR_BUILDER_TAG));
		if( !pBuild )
		{
			return;
		}

		int const NowPage = kPage.Now();
		int const StartPage = (NowPage / kPage.GetMaxViewPage()) * kPage.GetMaxViewPage();
		int const MaxPage = kPage.Max();

		XUI::CXUI_Button* pTemp = dynamic_cast<XUI::CXUI_Button*>(pPageMainUI->GetControl(WSTR_PAGE_START_TAG));
		if( pTemp )
		{
			pTemp->Disable( kPage.Max() < kPage.GetMaxViewPage() );
		}
		pTemp = dynamic_cast<XUI::CXUI_Button*>(pPageMainUI->GetControl(WSTR_PAGE_END_TAG));
		if( pTemp )
		{
			pTemp->Disable( kPage.Max() < kPage.GetMaxViewPage() );
		}
		pTemp = dynamic_cast<XUI::CXUI_Button*>(pPageMainUI->GetControl(WSTR_PAGE_JUMP_P_TAG));
		if( pTemp )
		{
			pTemp->Disable( kPage.Max() < kPage.GetMaxViewPage() );
		}
		pTemp = dynamic_cast<XUI::CXUI_Button*>(pPageMainUI->GetControl(WSTR_PAGE_JUMP_N_TAG));
		if( pTemp )
		{
			pTemp->Disable( kPage.Max() < kPage.GetMaxViewPage() );
		}
		
		int const MAX_SLOT = pBuild->CountX() * pBuild->CountY();
		for(int i = 0; i < MAX_SLOT; ++i)
		{
			BM::vstring vStr(WSTR_PAGE_BTN_TAG);
			vStr += i;

			XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pPageMainUI->GetControl(vStr));
			if( pCheckBtn )
			{
				pCheckBtn->ClickLock((StartPage + i) == NowPage);
				pCheckBtn->Check(pCheckBtn->ClickLock());		//이놈은 현제 페이지
				pCheckBtn->Visible((StartPage + i) < MaxPage);	//이놈은 넘었다.
				vStr = (StartPage + i) + 1;						//0부터 시작이니까.
				pCheckBtn->Text(vStr);
			}
		}
	}
}
