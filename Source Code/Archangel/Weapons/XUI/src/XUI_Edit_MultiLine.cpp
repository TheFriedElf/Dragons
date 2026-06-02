#include "stdafx.h"

#define DIRECTINPUT_VERSION (0x0800)

#include "XUI_Edit_MultiLine.h"
#include "XUI_Manager.h"

extern CS::CCSIME g_kMultiIME;

using namespace XUI;

CXUI_Edit_MultiLine::CXUI_Edit_MultiLine(void)
{
	MultiLineCount(2);
	IsMultiLine(true);
	m_bIsSame = false;
	NoWordWrap(false);
}
CXUI_Edit_MultiLine::~CXUI_Edit_MultiLine(void)
{
}

void CXUI_Edit_MultiLine::VRegistAttr(std::wstring const& wstrName, std::wstring const& wstrValue)
{
	CXUI_Edit::VRegistAttr(wstrName, wstrValue);

	BM::vstring vValue(wstrValue);

	if (ATTR_EDIT_MULTI_LINE_COUNT == wstrName)
	{
		int iVal = (int)vValue;
		if(iVal)
		{
			MultiLineCount(iVal);
		}
	}
}

void CXUI_Edit_MultiLine::VOnClose()
{	
	CXUI_Edit::DelGroupEdit(this);
	CXUI_Wnd::VOnClose();
	if( CXUI_Edit::GetFocusedEdit() == this )
	{
		CXUI_Edit::SetFocusedEdit(NULL);
	}

	m_iEndTextPos = 0;
	m_iStartTextPos = 0;

	SAFE_DELETE(m_p2DString);
	g_kMultiIME.SetString();
}

bool CXUI_Edit_MultiLine::VOnTick( DWORD const dwCurTime )
{
	if ( !CXUI_Wnd::VOnTick(dwCurTime) )
	{
		return false;
	}

	CS::CARETDATA Data = g_kMultiIME.GetCaretPos();

	SRenderTextInfo kRenderTextInfo;

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(EditFont());

	m_wstrInputText = g_kMultiIME.GetResultStr();

	std::wstring szRealText;
	m_bIsSame = false;
	int const iLine = MakeEditString(szRealText, Data.iCaretPos);

	if (!m_bIsSame)
	{
		Text(m_wstrRealString);
	}

	if( GetFocusedEdit() == this && m_spWndMouseFocus == this )
	{
		if( dwCurTime - CarotBlinkTime() > 400 )	//깜빡임 속도조절
		{
			CarotBlink(!CarotBlink());
			CarotBlinkTime(dwCurTime);
		}

		SetInvalidate();
	}
	return true;
}

bool CXUI_Edit_MultiLine::VDisplay()
{

	TextPos(EditTextPos());

	if(!CXUI_Wnd::VDisplay() ){return false;}

	CS::CARETDATA Data = g_kMultiIME.GetCaretPos();

	//SRenderTextInfo kRenderTextInfo;

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(EditFont());

	m_wstrInputText = g_kMultiIME.GetResultStr();

	std::wstring szRealText;//(m_wstrInputText.size(), 0);
	int const iLine = MakeEditString(szRealText, Data.iCaretPos);

	////kRenderTextInfo.dwColor = FontColor();
	//kRenderTextInfo.dwOutLineColor = OutLineColor();
	//kRenderTextInfo.fAlpha = Alpha();
	//kRenderTextInfo.kLoc = TotalLocation()+EditTextPos();
	//kRenderTextInfo.dwTextFlag = FontFlag();
	//GetParentDrawRect(kRenderTextInfo.rcDrawable);

	//kRenderTextInfo.wstrFontKey = EditFont();
	//kRenderTextInfo.wstrText.clear();
	//for (VEC_LINE::const_iterator line_it = m_kVecLine.begin();
	//	m_kVecLine.end() != line_it; ++line_it)
	//{		
	//	kRenderTextInfo.wstrText += (*line_it).m_kWstr;
	//	VEC_LINE::const_iterator next_it = line_it;
	//	++next_it;
	//	if (m_kVecLine.end() != next_it)
	//	{
	//		kRenderTextInfo.wstrText += _T('\n');
	//	}
	//}

	//if(m_p2DString == NULL)
	//{
	//	m_p2DString = (CXUI_2DString*)g_kFontMgr.CreateNew2DString(PgFontDef(pFont, FontColor(), FontFlag()),	kRenderTextInfo.wstrText);
	//}
	//else
	//{
	//	m_p2DString->SetText(PgFontDef(pFont,FontColor(),FontFlag()),
	//		kRenderTextInfo.wstrText);
	//}
	//kRenderTextInfo.m_p2DString = (void*)m_p2DString;
	//m_spRenderer->RenderText(kRenderTextInfo);


	if( GetFocusedEdit() == this && m_spWndMouseFocus == this )	//글만 적어놓고 다른 윈도우를 클릭할 수 있으므로 m_spWndMouseFocus도 체크해야 함
	{
		RenderBlock(szRealText);//글자 블럭 출력

		if(CarotBlink())
		{
			POINT2 kCaretPos = CalcCaretPos(m_wstrInputText, Data, pFont, iLine);
			RenderCarot(kCaretPos);
		}
	}

	return true;
}

int const CXUI_Edit_MultiLine::MakeEditString(std::wstring & Val, int const iCarot)
{
	int iLineCount = 1;
	if( m_wstrInputText.empty() )
	{
		m_kVecLine.clear();
		m_wstrRealString.clear();
		return iLineCount;
	}

	if( m_wstrInputText == m_wstrPastInputText && m_wstrInputText.size() == m_wstrPastInputText.size() && m_iPastCarotPos == iCarot && !m_wstrRealString.empty())
	{	//텍스트와 케럿 위치가 변경되지 않았을 때 또 계산을 해 주지 않게 하기 위해.
		Val = m_wstrRealString;
		iLineCount = GetLineCount(Val, iCarot);
		if(iLineCount>MultiLineCount())
		{
			iLineCount = MultiLineCount();
		}

		m_bIsSame = true;

		return iLineCount;
	}

	CarotBlink(true); 
	CarotBlinkTime(BM::GetTime32());

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(EditFont());

	m_kVecLine.clear();

	pFont->CalcWidthAddReturn(m_wstrInputText, Val, m_kVecLine, m_Size.x - EditTextPos().x); 
	int iNewLineCount = GetLineCount(Val, iCarot);
	size_t kVecCount = m_kVecLine.size();
	if(MultiLineCount()<kVecCount)
	{
		Val.clear();
		Val = m_kOldVal;
		CS::CARETDATA const kCaretData = m_kOldCaretData;
		g_kMultiIME.SetString(m_wstrPastInputText);
		g_kMultiIME.SetCaretPos(kCaretData.iCaretPos, kCaretData.iCaretPos);
		m_wstrInputText = m_wstrPastInputText;
	}
	else
	{
		m_kOldVal = Val;
		m_kOldCaretData = g_kMultiIME.GetCaretPos();

		m_wstrPastInputText = m_wstrInputText;
		m_iPastCarotPos = iCarot;
	}

	//m_wstrInputText = Val;
	m_wstrRealString = Val;
	
	return iNewLineCount;
}

void CXUI_Edit_MultiLine::RenderBlock(std::wstring & Val)
{
	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(EditFont());

	if( ! CXUI_Edit::m_spTextBlockBgImg ){return;}
	if( m_wstrInputText.empty() ){return;}

	CS::CARETDATA const Data = g_kMultiIME.GetCaretPos();

	if( Data.iSelectStart == Data.iSelectEnd ){return;}//선택 영역 없음.

	POINT3I pt = TotalLocation()+EditTextPos();

	SRenderInfo kRenderInfo;

	kRenderInfo.bGrayScale = GrayScale();

	SSizedScale &rSS = kRenderInfo.kSizedScale;
	rSS.ptSrcSize = POINT2(128,12);//xxx todo 하드코딩
	kRenderInfo.fAlpha = Alpha();
	m_siBlockImgIdx = -2;

	int iStartPos = Data.iSelectStart;
	int iBlockLen = Data.iSelectEnd - Data.iSelectStart;
	bool bBreak = false;
	std::wstring::size_type loc1 = iStartPos;
	int iPastPos = 0;
	int iLineCount = GetLineCount(Val, iStartPos);
	POINT2 ptSize(0, pFont->GetHeight());
	POINT2 ptPos = pt;

	int const add = 2;
	int iMax = __min(iLineCount -1, (int)m_kVecLine.size());
	for (int i = 0; i < iMax; ++i)
	{
		iStartPos-=(int)(m_kVecLine[i].m_kWstr.length());
		if (m_kVecLine[i].m_bReturn)
		{
			iStartPos-=add;
		}
	}

	while (iBlockLen>0 && iLineCount - 1 < (int)m_kVecLine.size())
	{
		int iEndPos = __min((int)(m_kVecLine[iLineCount-1].m_kWstr.length()), iStartPos + iBlockLen);
		std::wstring wstrSub = m_kVecLine[iLineCount-1].m_kWstr.substr(iStartPos, iEndPos - iStartPos);
		int iSubLen = pFont->CalcWidth(wstrSub);
		int iFrontLen = 0;
		if (iStartPos > 0)
		{
			iFrontLen = pFont->CalcWidth(m_kVecLine[iLineCount-1].m_kWstr.substr(0, iStartPos));
		}
		ptPos = pt;
		ptPos.x+=iFrontLen;
		ptPos.y+=(iLineCount-1)*pFont->GetHeight();

		ptSize.x = iSubLen;

		if (0 < ptSize.x && 0 < ptSize.y)
		{
			rSS.ptDrawSize = ptSize;
			kRenderInfo.kLoc = ptPos;
			GetParentDrawRect(kRenderInfo.rcDrawable);
			m_spRenderer->RenderSprite( CXUI_Edit::m_spTextBlockBgImg, m_siBlockImgIdx, kRenderInfo);
			m_siBlockImgIdx = -1;
		}

		iBlockLen -= (int)wstrSub.length();
		if (m_kVecLine[iLineCount-1].m_bReturn)
		{
			iBlockLen-=add;
		}
		iStartPos = 0;
		++iLineCount;
	}
}

POINT2 CXUI_Edit_MultiLine::CalcCaretPos(std::wstring const& wstrReal, CS::CARETDATA const& rkData, XUI::CXUI_Font* pFont, int const iLine)
{
	POINT2 ptLastPos = TotalLocation()+EditTextPos();

	if (!wstrReal.empty())
	{
		std::wstring szFront;
		ptLastPos.y+=(pFont->GetHeight()*(iLine-1));
		
		if( !m_wstrInputText.empty() )
		{
			if (!m_kVecLine.empty())
			{
				int iLen = __min((int)wstrReal.length(), rkData.iCaretPos-m_iStartTextPos);
				int iMinLine = __min(iLine - 1 , (int)m_kVecLine.size() - 1);
				int iMinLen = 0;

				for(int i = 0; i < iMinLine; ++i)
				{
					iLen = iLen - (int)(m_kVecLine[i].m_kWstr.length());
					if (m_kVecLine[i].m_bReturn)
					{
						iLen-=2;
					}
				}

				iLen = __max(0, iLen);

				iMinLen = __min(iLen , (int)(m_kVecLine[iMinLine].m_kWstr.size()));
				for(int i = 0; i < iMinLen; ++i)
				{
					wchar_t wC = m_kVecLine[iMinLine].m_kWstr[i];
					szFront+=wC;
				}
			}

			ptLastPos.x+= pFont->CalcWidth(szFront);//약간 앞으로 당겨줘야 함
		}
	}

	return ptLastPos;
} 

void CXUI_Edit_MultiLine::MoveCarotToClickPos(int &iStart, int &iEnd)
{
	if( m_wstrInputText.empty() )
	{
		g_kMultiIME.SetCaretPos(0,0);
		return; 
	}

	if( iStart < m_iStartTextPos )
	{
		iStart = m_iStartTextPos;
	}

	if( iStart > m_iEndTextPos )
	{
		iEnd = m_iEndTextPos;
	}

	g_kMultiIME.SetCaretPos(iStart,iEnd);
}

void CXUI_Edit_MultiLine::VLoseFocus(bool const bUpToParent)
{
	if (E_XUI_EDIT == m_spWndMouseFocus->VType() || E_XUI_EDIT_MULTILINE == m_spWndMouseFocus->VType())
	{
		CXUI_Wnd::VLoseFocus(bUpToParent);
		if (this == CXUI_Edit::GetFocusedEdit())
		{
			CXUI_Edit::SetFocusedEdit(NULL);
			IsNativeIME(g_kMultiIME.GetIME_CMODE() == IME_CMODE_NATIVE);
			//g_kMultiIME.SetEnglishIME(true);
			g_kMultiIME.SetEnableIME(false);
		}
		g_kMultiIME.SetOnlyNumeric(false, false);
		m_bIsSame = false;	
	}
} 

int CXUI_Edit_MultiLine::GetClickTextPos()
{
	int iSize = 0;
	if( m_wstrInputText.empty() )
	{
		return iSize;
	}

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(EditFont());

	return iSize;
}

bool CXUI_Edit_MultiLine::VPeekEvent(E_INPUT_EVENT_INDEX const& rET, POINT3I const& rPT, DWORD const& dwValue)
{
	if( !CXUI_Wnd::Visible() || IsClosed() ){return false;}//자식도 안함.
	if( !Enable() ){return false;}

	//XUI_Edit만 특별 메시지 처리를 한다.
	bool bRet = false;
	m_bDBLClick = false;

#ifdef XUI_USE_SCRIPT_CALL_OPTIMIZE
	unsigned int wstrScriptKey = SCRIPT_MAX_NUM;
#else
	std::wstring wstrScriptKey;
#endif

	switch(rET)
	{
	case IEI_KEY_DOWN:
		//case IEI_KEY_UP:
		{//컨트롤에 전달 되어야함. dxxx todo 
			if(IsFocus())
			{
				if (g_kMultiIME.IsNowComp() == false)
				{
					BM::vstring vstr(dwValue);
					bRet = DoHotKey(vstr);
				}
				return true;//뭐가 어떻게 되던 포커스를 가진놈이 인풋을 먹어치워야 함
			}
			/*BM::vstring vstr = (int)dwValue;
			bRet = DoHotKey(vstr);
			if(!bRet)
			{
				return false;
			}*/
			return false;
		}break;
	case IEI_MS_DOWN:
		{
			if( ContainsPoint( m_sMousePos ) )//마우스 이벤트는 영역 체크 필수.
			{
				m_spWndMouseOver = this;//Edit 전용 추가

				if(MEI_BTN_0 == dwValue)
				{
					IsMouseDown(true);
					VAcquireFocus(this);

					static DWORD dwLastDownTime = 0;
					DWORD const dwNow = BM::GetTime32();

					if( LastMouseDownPos()-XUIMgr.DblClickBound()<=m_sMousePos && LastMouseDownPos()+XUIMgr.DblClickBound()>=m_sMousePos )
					{//같은 좌표에서.
						if(dwLastDownTime)
						{
							//있었고 //일정 시간 내에 들어오면.
							if((dwNow - dwLastDownTime) < XUIMgr.DblClickTick())//200 밀리 이하로 들어왔다..
							{
								wstrScriptKey = SCRIPT_ON_L_BTN_DBL_DOWN;
								dwLastDownTime = 0;//이벤트후 시간은 0으로 만들어야하고.
								m_bDBLClick = true;

								bRet = true;
							}
						}
					}

					dwLastDownTime = dwNow;//뭐가 됐든 마지막 다운시간은 기록

					wstrScriptKey = SCRIPT_ON_L_BTN_DOWN;
					LastMouseDownPos(m_sMousePos);

					if(m_spWndMouseOver==this && GetFocusedEdit()!=this)//>>Edit 전용 추가
					{
						if(!m_wstrInputText.empty())//입력된 문자열이 있을 경우 초기화 시키면 안됨
						{
							g_kMultiIME.SetString(m_wstrInputText);
							g_kMultiIME.SetCaretPos(false);//캐럿을 맨 뒤로
							SetEditFocus(true);
						}
						else
						{
							SetEditFocus(false);//무조건 입력된 문자가 클리어 되므로 분리함
						}
					}
					else if(m_spWndMouseOver==this && GetFocusedEdit()==this)//포커스 먹은 상태에서 다시 왼쪽 클릭
					{
						int pos = GetClickTextPos();
						//MoveCarotToClickPos(pos, pos);
						if(m_bDBLClick)//포커스 먹은 상태에서 더블클릭되면 모두 블럭지정
						{
							g_kMultiIME.SetCaretPos(0, (int)m_wstrInputText.length());
						}
						else
						{
							g_kMultiIME.SetCaretPos(pos, pos);
						}

					}//<<Edit 전용 추가
				}
				else if( MEI_BTN_1 == dwValue )
				{ 
					wstrScriptKey = SCRIPT_ON_R_BTN_DOWN;
				}
				bRet = true;
			}
		}break;
	case IEI_MS_UP:
		{
			if( ContainsPoint( m_sMousePos ) )//마우스 이벤트는 영역 체크 필수.
			{
				if(MEI_BTN_0 == dwValue)
				{
					IsMouseDown(false);	
					wstrScriptKey = SCRIPT_ON_L_BTN_UP; 
				}
				else if( MEI_BTN_1 == dwValue )
				{ 
					wstrScriptKey = SCRIPT_ON_R_BTN_UP;
				}
				bRet = true;
			}
		}break;
	case IEI_MS_MOVE:
		{
			bool const bIsBeforeMouseOver = IsMouseOver();
			if( ContainsPoint( m_sMousePos ) )//마우스 이벤트는 영역 체크 필수.
			{
				bRet = true;
				if(m_spWndMouseOver != this)
				{
					if(m_spWndMouseOver)
					{
						m_spWndMouseOver->DoScript(SCRIPT_ON_MOUSE_OUT);//이전 마우스 오버에서 아웃 처리.
						m_spWndMouseOver->IsMouseDown(false);
					}

					m_spWndMouseOver = this;// 마우스 올라왔어.

					wstrScriptKey = SCRIPT_ON_MOUSE_OVER;
				}

				if(IsMouseDown() && CanDrag())
				{
					POINT3I ptOrg = Location();
					Location( ptOrg+rPT );
				}

				if(IsMouseDown())//>>Edit 전용 추가
				{
					int pos = GetClickTextPos();
					if(g_kMultiIME.GetCaretPos().iSelectStart > pos)
					{
						int start = g_kMultiIME.GetCaretPos().iSelectEnd;
						MoveCarotToClickPos(pos, start);
					}
					else if(g_kMultiIME.GetCaretPos().iSelectStart < pos)
					{
						int start = g_kMultiIME.GetCaretPos().iSelectStart;
						MoveCarotToClickPos(start, pos);
					}
				}//<<Edit 전용 추가

				if(rPT.z != 0 && ContainsPoint(m_sMousePos)) // 마우스 휠이다.
				{
					if (rPT.z > 0){wstrScriptKey = SCRIPT_ON_WHEEL_UP;}
					else{wstrScriptKey = SCRIPT_ON_WHEEL_DOWN;}
					SetCustomData(&rPT.z, sizeof(rPT.z));
					bRet = true;
				}
			}
		}break;
	default:
		{
			return true;
		}break;
	}

	bool const bScriptRet = DoScript( wstrScriptKey );//실행 안되는 현상을 방지하기 위함.

	return (bRet || bScriptRet);
}

void CXUI_Edit_MultiLine::OnHookEvent()
{
	m_wstrInputText = g_kMultiIME.GetResultStr();
}

void CXUI_Edit_MultiLine::EditText(std::wstring const& wstrValue,bool bKeepTextBlock)
{
	m_wstrInputText = wstrValue;
	g_kMultiIME.SetLimitLength(m_iLimitLength, false);//문제 제한을 변경한다.
	g_kMultiIME.SetOnlyNumeric(IsOnlyNum(), false);
	g_kMultiIME.SetMultiLine(IsMultiLine(), m_iLimitLength, MultiLineCount(), false);
	g_kMultiIME.SetString( m_wstrInputText );
	g_kMultiIME.SetCaretPos(false);//캐럿을 맨 뒤로
}

bool CXUI_Edit_MultiLine::SetEditFocus(bool const bIsJustFocus)
{
	if(IsEditDisable())
	{//수정불가능!!
		return false;
	}

	if( !m_wstrBlockPath.empty() )
	{
		m_spRscMgr->ReleaseRsc(CXUI_Edit::m_spTextBlockBgImg);
		CXUI_Edit::m_spTextBlockBgImg = m_spRscMgr->GetRsc( m_wstrBlockPath );
		m_siBlockImgIdx = -2;
	}

	if( !m_wstrCarotPath.empty() )
	{
		m_spRscMgr->ReleaseRsc(CXUI_Edit::m_spCarotImg);
		CXUI_Edit::m_spCarotImg = m_spRscMgr->GetRsc( m_wstrCarotPath );
		m_siCarotImgIdx = -1;
	}

	VAcquireFocus(this);
	CXUI_Edit::SetFocusedEdit(this);

	if(!bIsJustFocus)
	{
		m_wstrInputText = _T("");
	}
	
	g_kMultiIME.SetLimitLength(m_iLimitLength, false);//문제 제한을 변경한다.
	g_kMultiIME.SetOnlyNumeric(IsOnlyNum(), false);
	g_kMultiIME.SetMultiLine(IsMultiLine(), m_iLimitLength, MultiLineCount(), false);
	g_kMultiIME.SetString( m_wstrInputText );
	g_kMultiIME.SetPasswordMode(IsSecret());
	g_kMultiIME.SetEnglishIME(false);
	g_kMultiIME.SetEnableIME(true);
	if (IsNativeIME())
	{
		g_kMultiIME.SetNativeIME();
	}

	DoScript(SCRIPT_ON_FOCUS);	//포커스를 가졌을때 (ON_FOCUS)
	return true;
}


void CXUI_Edit_MultiLine::RenderCarot(POINT2& pt)	//캐럿출력
{
	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(EditFont());

	if(CXUI_Edit::m_spTextBlockBgImg == NULL
		||	CXUI_Edit::m_spCarotImg == NULL 
		||	!pFont)
	{
		assert(NULL);
		return;
	}

	void *pImg = NULL;
	int *pImgIdx = NULL;
	SRenderInfo kRenderInfo;
	int iAdd = 0;

	if( g_kMultiIME.IsNowComp() )	//한글이면
	{
		pt.x -= pFont->GetHeight();
		
		pImg = CXUI_Edit::m_spTextBlockBgImg;
		pImgIdx = &m_siBlockImgIdx;

		SSizedScale &rSS = kRenderInfo.kSizedScale;
		rSS.ptSrcSize = POINT2(16,16);//xxx todo 하드코딩
		rSS.ptDrawSize = POINT2(pFont->GetHeight()-4, pFont->GetHeight());
		iAdd = 4;//xxx todo 하드코딩
	}
	else
	{
		pImg = CXUI_Edit::m_spCarotImg;
		pImgIdx = &m_siCarotImgIdx;

		SSizedScale &rSS = kRenderInfo.kSizedScale;
		rSS.ptSrcSize = POINT2(16,16);//xxx todo 하드코딩
		rSS.ptDrawSize = POINT2(1, pFont->GetHeight());
	}

	kRenderInfo.kUVInfo = UVInfo();
	kRenderInfo.kLoc = pt;
	kRenderInfo.kLoc.x+=iAdd;
	GetParentDrawRect(kRenderInfo.rcDrawable);
	kRenderInfo.fAlpha = Alpha();

	if( pImg )
	{
		m_spRenderer->RenderSprite( pImg, *pImgIdx, kRenderInfo);
	}
}