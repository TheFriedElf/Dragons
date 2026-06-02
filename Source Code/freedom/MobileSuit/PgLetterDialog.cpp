#include "stdafx.h"
#include "Variant/PgQuestInfo.h"
#include "PgQuestMan.h"
#include "PgQuestFullScreenDialog.h"
#include "PgLetterDialog.h"

std::wstring const WSTR_FRM_LETTER_1 = _T("FRM_LETTER_1");

PgLetterDialog::PgLetterDialog()
{
	m_iNowLetterMaxStep = 0;
	m_iNowLetterStep = 0;
	m_kContLetterStep.clear();
}

PgLetterDialog::~PgLetterDialog()
{

}

bool PgLetterDialog::CallNewLetter(int const iTTW)
{
	std::wstring const kLetterText = TTW(iTTW);
	if( kLetterText.empty() )
	{
		return false;
	}

	PgQuestManUtil::ParseDialogForStep(kLetterText, _T("%%"), m_kContLetterStep);

	if( m_kContLetterStep.empty() )
	{
		return false;
	}

	m_iNowLetterStep = 0;
	m_iNowLetterMaxStep = m_kContLetterStep.size() - 1;

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(WSTR_FRM_LETTER_1);
	PG_ASSERT_LOG(pkWnd && "Can't Make Interface");

	UpdateLetter();
	return true;
}

void PgLetterDialog::NextLetter()
{
	if( NowLetterStep() == NowLetterMaxStep() )
	{
		return;
	}

	++m_iNowLetterStep;

	UpdateLetter();
}

void PgLetterDialog::PrevLetter()
{
	if( 0 == NowLetterStep() )
	{
		return;
	}

	--m_iNowLetterStep;

	UpdateLetter();
}

void PgLetterDialog::UpdateLetter()
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(WSTR_FRM_LETTER_1);
	if( !pkWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pkPageWnd = pkWnd->GetControl(_T("FRM_PAGE"));
	if( !pkPageWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pkTextWnd = pkWnd->GetControl(_T("FRM_TEXT"));
	if( !pkTextWnd )
	{
		return;
	}

	ContDialogStep::iterator iter =  m_kContLetterStep.find(NowLetterStep());
	if( m_kContLetterStep.end() == iter )
	{
		return;
	}

	std::wstring const &rkNowLetterText = (*iter).second;

	if( 1 < m_kContLetterStep.size() )
	{
		TCHAR szTemp[1024] = {0, };
		_stprintf_s(szTemp, _T("{C=0xFF2d6ea5/}%2d{C=0xFF482e26/}/%2d"), NowLetterStep()+1, NowLetterMaxStep()+1);
		pkPageWnd->Text(szTemp);
		pkPageWnd->Visible(true);
	}
	else
	{
		pkPageWnd->Visible(false);
	}

	pkTextWnd->Text(rkNowLetterText);
}