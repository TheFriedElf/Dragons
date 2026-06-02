#include "StdAfx.h"
#include "PgMobileSuit.h"
#include "PgTextTypistWnd.h"

PgTextTypistWnd::PgTextTypistWnd()
	: m_fIntervalTime(0.f), m_fPrevTime(0.f), m_iLineCount(0), m_iCurStartLine(0)
	, m_kUpBtnName(), m_kDownBtnName()
{
}

PgTextTypistWnd::~PgTextTypistWnd()
{
}

void PgTextTypistWnd::VRegistAttr(std::wstring const& wstrName, std::wstring const& wstrValue)
{
	BM::vstring	vValue(wstrValue);

	std::wstring const ATTR_STEP_INTERVAL = _T("STEP_INTERVAL");
	std::wstring const ATTR_UP_BTN_NAME = _T("UP_SCROLL_BTN");
	std::wstring const ATTR_DOWN_BTN_NAME = _T("DOWN_SCROLL_BTN");
	if( wstrName == ATTR_STEP_INTERVAL )
	{
		m_fIntervalTime = ((int)vValue) / 100.0f;
	}
	else if( wstrName == ATTR_UP_BTN_NAME )
	{
		m_kUpBtnName = wstrValue;
	}
	else if( wstrName == ATTR_DOWN_BTN_NAME )
	{
		m_kDownBtnName = wstrValue;
	}
	else
	{
		CXUI_Wnd::VRegistAttr(wstrName, wstrValue);
	}
}

namespace PgTextTypistWndUtil
{
	using namespace XUI;

	class PgCharCounter
	{
	public:
		typedef std::vector< size_t > ContFindPos;

		explicit PgCharCounter(TCHAR const cCountedChar, CONT_PARSED_CHAR const& rkParsedChar)
			:m_kVec(0)
		{
			std::back_inserter(m_kVec) = 0; // 첫번째 줄 기본
			if( !rkParsedChar.empty() )
			{
				for( size_t iCur = 0; rkParsedChar.size() > iCur; ++iCur )
				{
					if( rkParsedChar.at(iCur).m_wChar == cCountedChar )
					{
						std::back_inserter(m_kVec) = iCur;
					}
				}
			}
		}

		size_t at(size_t const iCur)const
		{
			return (*this)[iCur];
		}
		size_t operator [] (size_t const iCur)const
		{
			if( m_kVec.size() > iCur )
			{
				return m_kVec.at(iCur);
			}
			_PgMessageBox("Error", "Invalidate line number %d/%d", iCur, m_kVec.size());
			return 0;
		}

		size_t Count()const { return m_kVec.size(); }
		ContFindPos const& Vec()const { return m_kVec; }
	private:
		ContFindPos m_kVec;
	};

	CXUI_Style_String GetLineStyleText(size_t const iTrimLineCount, CXUI_Style_String const& rkStyleText, size_t const iStartLine = 0)
	{
		TCHAR const cBreakLineChar = _T('\n');
		PgCharCounter kCharCounter(cBreakLineChar, rkStyleText.GetCharVector());
		
		size_t const iCurLineCount = kCharCounter.Count();
		if( iCurLineCount > iTrimLineCount )
		{
			size_t const iCurStartLine = (0 == iStartLine)? iCurLineCount - iTrimLineCount: std::min(iCurLineCount - iTrimLineCount, iCurLineCount - iTrimLineCount - iStartLine);
			size_t const iEndCharPos = (0 == iStartLine)? rkStyleText.GetCharVector().size(): kCharCounter[iCurStartLine + iTrimLineCount];
			CXUI_Style_String kResultText;
			std::wstring kTempStr;
			for( size_t iCur = kCharCounter[iCurStartLine]; iEndCharPos > iCur; ++iCur )
			{
				PgParsedChar kParsedChar = rkStyleText.GetCharVector().at(iCur);
				kTempStr += kParsedChar.m_wChar;
				kResultText.AddParsedChar(kParsedChar);
			}
			kResultText.SetOriginalString(kTempStr);
			// 잘라서 리턴
			return kResultText;
		}
		else
		{
			// 원본 그대로 리턴
			return rkStyleText;
		}
	}

	inline void SetVisibleChild(CXUI_Wnd* pkWnd, std::wstring const& rkChildName, bool const bVisible)
	{
		if( !pkWnd )
		{
			NILOG(PGLOG_WARNING, "[SetVisibleChild] Self is NULL");
			return;
		}

		CXUI_Wnd* pkBtn = pkWnd->GetControl(rkChildName);
		if( !pkBtn )
		{
			NILOG(PGLOG_WARNING, "[TypistWnd] Can't find child button ID = %s\n", MB(rkChildName));
		}
		else
		{
			pkBtn->Visible(bVisible);
		}
	}

	inline void SetEnableChild(CXUI_Wnd* pkWnd, std::wstring const& rkChildName, bool const bEnable)
	{
		if( !pkWnd )
		{
			NILOG(PGLOG_WARNING, "[SetVisibleChild] Self is NULL");
			return;
		}

		CXUI_Wnd* pkBtn = pkWnd->GetControl(rkChildName);
		if( !pkBtn )
		{
			NILOG(PGLOG_WARNING, "[TypistWnd] Can't find child button ID = %s\n", MB(rkChildName));
		}
		else
		{
			pkBtn->Enable(bEnable);
		}
	}

	inline bool IsFirstLine(size_t const iCurLine, size_t const iTotalLine, size_t const iVisibleLineCount)
	{
		return (iCurLine == iTotalLine - iVisibleLineCount);
	}
	
	inline bool IsLastLine(size_t const iCurLine)
	{
		return (iCurLine == 0);
	}
};

int const iPgTextTypistViewLineCountMax = 4;
bool PgTextTypistWnd::VPeekEvent(XUI::E_INPUT_EVENT_INDEX const &rET, POINT3I const &rPT, DWORD const &dwValue)
{
	if( !Visible() || !Enable() )
	{
		return false;
	}

	if( XUI::IEI_MS_MOVE == rET
	&&	ContainsPoint( m_sMousePos )
	&&	0 != rPT.z ) // 마우스 휠이다.
	{
		if( 0 != rPT.z
		&&	DrawFinish() )
		{
			if( rPT.z > 0 )
			{
				UpWheal();
			}
			else
			{
				DownWheal();
			}
		}
		return true;
	}
	return CXUI_Wnd::VPeekEvent(rET, rPT, dwValue);
}

void PgTextTypistWnd::UpWheal()
{
	//휠 업
	m_iCurStartLine = std::min(++m_iCurStartLine, m_iLineCount - iPgTextTypistViewLineCountMax);
}

void PgTextTypistWnd::DownWheal()
{
	//휠 다운
	if( 0 != m_iCurStartLine )
	{
		--m_iCurStartLine;
	}
}

bool PgTextTypistWnd::VDisplay()
{
	if( !m_bVisible )
	{
		return true;
	}

	if( !m_kStandByWord.empty() )
	{
		float const fNowTime = g_pkApp->GetAccumTime();
		float const fIntervalTime = fNowTime - m_fPrevTime;
		if( fIntervalTime > m_fIntervalTime )
		{
			m_fPrevTime = fNowTime;
			m_kStyleText += m_kStandByWord.front();
			m_kStandByWord.pop_front();

			if( DrawFinish()
			&&	iPgTextTypistViewLineCountMax < m_iLineCount )
			{
				SetButtonVisible(true);
			}
		}
	}

	if( DrawFinish() )
	{
		if( !m_kUpBtnName.empty() )
		{
			PgTextTypistWndUtil::SetEnableChild(this, m_kUpBtnName, !PgTextTypistWndUtil::IsFirstLine(m_iCurStartLine, m_iLineCount, iPgTextTypistViewLineCountMax));
		}
		if( !m_kDownBtnName.empty() )
		{
			PgTextTypistWndUtil::SetEnableChild(this, m_kDownBtnName, !PgTextTypistWndUtil::IsLastLine(m_iCurStartLine));
		}
	}

	DisplayControl();

	POINT3I const pt = TotalLocation() + TextPos();
	RenderText(pt, PgTextTypistWndUtil::GetLineStyleText(iPgTextTypistViewLineCountMax, m_kStyleText, m_iCurStartLine), FontFlag());

	return true;
}

void PgTextTypistWnd::Text(std::wstring const& szString)
{
	Alpha(1.0f);
	if( !szString.size() )
	{
		return;
	}

	m_kStandByWord.clear();

	CXUI_Wnd::Text(szString);

	XUI::CONT_PARSED_CHAR const& kList = m_kStyleText.GetCharVector();
	if( kList.size() )
	{
		XUI::CONT_PARSED_CHAR::const_iterator	c_iter = kList.begin();
		while( c_iter != kList.end() )
		{
			m_kStandByWord.push_back(*c_iter);
			++c_iter;
		}
	}

	PgTextTypistWndUtil::PgCharCounter kCharCounter(_T('\n'), m_kStyleText.GetCharVector());
	m_iLineCount = kCharCounter.Count();
	m_iCurStartLine = 0;

	CXUI_Wnd::Text( std::wstring() );

	m_fPrevTime = g_pkApp->GetAccumTime();
	m_kStyleText.SetOriginalString(szString);

	SetButtonVisible(false);
}

void PgTextTypistWnd::SkipDrawInterval()
{
	if( m_kStandByWord.size() )
	{
		kWordList::iterator	iter = m_kStandByWord.begin();
		while( iter != m_kStandByWord.end() )
		{
			m_kStyleText += *iter;
			++iter;
		}
	}
	m_kStandByWord.clear();

	if( iPgTextTypistViewLineCountMax < m_iLineCount )
	{
		SetButtonVisible(true);
	}
}

void PgTextTypistWnd::SetButtonVisible(bool const bVisible)
{
	if( !m_kUpBtnName.empty() )
	{
		PgTextTypistWndUtil::SetVisibleChild(this, m_kUpBtnName, bVisible);
	}
	if( !m_kDownBtnName.empty() )
	{
		PgTextTypistWndUtil::SetVisibleChild(this, m_kDownBtnName, bVisible);
	}
}