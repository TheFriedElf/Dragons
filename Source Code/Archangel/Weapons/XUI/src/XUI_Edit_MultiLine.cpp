#include "stdafx.h"

#define DIRECTINPUT_VERSION (0x0800)

#include "XUI_Edit_MultiLine.h"
#include "XUI_Manager.h"

extern CS::CCSIME g_kMultiIME;
bool TryGetCustomChatBarCaretPos(XUI::CXUI_Edit* pChatEditControl, std::wstring const& wstrInputText, CS::CARETDATA const& rkData, POINT2 const& kBaseCaretPos, int iWrapWidth, int iDefaultLineHeight, POINT2& kOutCaretPos);

using namespace XUI;

CXUI_Edit_MultiLine::CXUI_Edit_MultiLine(void)
{
	MultiLineCount(2);
	IsMultiLine(true);
	m_bIsSame = false;
	NoWordWrap(false);
}
CXUI_Edit_MultiLine::~CXUI_Edit_MultiLine(void)
{}

void CXUI_Edit_MultiLine::VRegistAttr(std::wstring const& wstrName, std::wstring const& wstrValue)
{
	CXUI_Edit::VRegistAttr(wstrName, wstrValue);

	BM::vstring vValue(wstrValue);

	if (ATTR_EDIT_MULTI_LINE_COUNT == wstrName)
	{
		int iVal = (int)vValue;
		if (iVal)
		{
			MultiLineCount(iVal);
		}
	}
}

void CXUI_Edit_MultiLine::VOnClose()
{
	CXUI_Edit::DelGroupEdit(this);
	CXUI_Wnd::VOnClose();
	if (CXUI_Edit::GetFocusedEdit() == this)
	{
		CXUI_Edit::SetFocusedEdit(NULL);
	}

	m_iEndTextPos = 0;
	m_iStartTextPos = 0;

	SAFE_DELETE(m_p2DString);
	g_kMultiIME.SetString();
}

bool CXUI_Edit_MultiLine::VOnTick(DWORD const dwCurTime)
{
	if (!CXUI_Wnd::VOnTick(dwCurTime))
	{
		return false;
	}

	CS::CARETDATA Data = g_kMultiIME.GetCaretPos();

	SRenderTextInfo kRenderTextInfo;

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(EditFont());

	m_wstrInputText = g_kMultiIME.GetResultStr();
	if (m_iLimitLength > 0 && static_cast<int>(m_wstrInputText.length()) > m_iLimitLength)
	{
		m_wstrInputText.erase(m_iLimitLength);
		g_kMultiIME.SetLimitLength(m_iLimitLength, false);
		g_kMultiIME.SetString(m_wstrInputText);
		g_kMultiIME.SetCaretPos(__min(Data.iSelectStart, m_iLimitLength), __min(Data.iSelectEnd, m_iLimitLength));
		Data = g_kMultiIME.GetCaretPos();
	}

	std::wstring szRealText;
	m_bIsSame = false;
	int const iLine = MakeEditString(szRealText, Data.iCaretPos);

	if (!m_bIsSame)
	{
		Text(m_wstrRealString);
	}

	if (GetFocusedEdit() == this && m_spWndMouseFocus == this)
	{
		if (dwCurTime - CarotBlinkTime() > 400)	//������ �ӵ�����
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

	if (!CXUI_Wnd::VDisplay()) { return false; }

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


	if (GetFocusedEdit() == this && m_spWndMouseFocus == this)	//�۸� ������� �ٸ� �����츦 Ŭ���� �� �����Ƿ� m_spWndMouseFocus�� üũ�ؾ� ��
	{
		RenderBlock(szRealText);//���� ���� ���

		if (CarotBlink())
		{
			POINT2 kCaretPos = CalcCaretPos(m_wstrInputText, Data, pFont, iLine);
			RenderCarot(kCaretPos);
		}
	}

	return true;
}

int const CXUI_Edit_MultiLine::MakeEditString(std::wstring& Val, int const iCarot)
{
	int iLineCount = 1;
	if (m_wstrInputText.empty())
	{
		m_kVecLine.clear();
		m_wstrRealString.clear();
		return iLineCount;
	}

	if (m_wstrInputText == m_wstrPastInputText && m_wstrInputText.size() == m_wstrPastInputText.size() && m_iPastCarotPos == iCarot && !m_wstrRealString.empty())
	{	//�ؽ�Ʈ�� �ɷ� ��ġ�� ������� �ʾ��� �� �� ����� �� ���� �ʰ� �ϱ� ����.
		Val = m_wstrRealString;
		iLineCount = GetLineCount(Val, iCarot);
		if (iLineCount > MultiLineCount())
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
	if (MultiLineCount() < kVecCount)
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

void CXUI_Edit_MultiLine::RenderBlock(std::wstring& Val)
{
	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(EditFont());

	if (!CXUI_Edit::m_spTextBlockBgImg) { return; }
	if (m_wstrInputText.empty()) { return; }

	CS::CARETDATA const Data = g_kMultiIME.GetCaretPos();

	if (Data.iSelectStart == Data.iSelectEnd) { return; }//���� ���� ����.

	POINT3I pt = TotalLocation() + EditTextPos();

	SRenderInfo kRenderInfo;

	kRenderInfo.bGrayScale = GrayScale();

	SSizedScale& rSS = kRenderInfo.kSizedScale;
	rSS.ptSrcSize = POINT2(128, 12);//xxx todo �ϵ��ڵ�
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
	int iMax = __min(iLineCount - 1, (int)m_kVecLine.size());
	for (int i = 0; i < iMax; ++i)
	{
		iStartPos -= (int)(m_kVecLine[i].m_kWstr.length());
		if (m_kVecLine[i].m_bReturn)
		{
			iStartPos -= add;
		}
	}

	while (iBlockLen > 0 && iLineCount - 1 < (int)m_kVecLine.size())
	{
		int iEndPos = __min((int)(m_kVecLine[iLineCount - 1].m_kWstr.length()), iStartPos + iBlockLen);
		std::wstring wstrSub = m_kVecLine[iLineCount - 1].m_kWstr.substr(iStartPos, iEndPos - iStartPos);
		int iSubLen = pFont->CalcWidth(wstrSub);
		int iFrontLen = 0;
		if (iStartPos > 0)
		{
			iFrontLen = pFont->CalcWidth(m_kVecLine[iLineCount - 1].m_kWstr.substr(0, iStartPos));
		}
		ptPos = pt;
		ptPos.x += iFrontLen;
		ptPos.y += (iLineCount - 1) * pFont->GetHeight();

		ptSize.x = iSubLen;

		if (0 < ptSize.x && 0 < ptSize.y)
		{
			rSS.ptDrawSize = ptSize;
			kRenderInfo.kLoc = ptPos;
			GetParentDrawRect(kRenderInfo.rcDrawable);
			m_spRenderer->RenderSprite(CXUI_Edit::m_spTextBlockBgImg, m_siBlockImgIdx, kRenderInfo);
			m_siBlockImgIdx = -1;
		}

		iBlockLen -= (int)wstrSub.length();
		if (m_kVecLine[iLineCount - 1].m_bReturn)
		{
			iBlockLen -= add;
		}
		iStartPos = 0;
		++iLineCount;
	}
}

POINT2 CXUI_Edit_MultiLine::CalcCaretPos(std::wstring const& wstrReal, CS::CARETDATA const& rkData, XUI::CXUI_Font* pFont, int const iLine)
{
	POINT2 ptLastPos = TotalLocation() + EditTextPos();
	POINT2 kCustomCaretPos = ptLastPos;
	if (TryGetCustomChatBarCaretPos(this, m_wstrInputText, rkData, ptLastPos, m_Size.x - EditTextPos().x, pFont ? pFont->GetHeight() : 0, kCustomCaretPos))
	{
		return kCustomCaretPos;
	}

	if (!wstrReal.empty())
	{
		std::wstring szFront;
		ptLastPos.y += (pFont->GetHeight() * (iLine - 1));

		if (!m_wstrInputText.empty())
		{
			if (!m_kVecLine.empty())
			{
				int iLen = __min((int)wstrReal.length(), rkData.iCaretPos - m_iStartTextPos);
				int iMinLine = __min(iLine - 1, (int)m_kVecLine.size() - 1);
				int iMinLen = 0;

				for (int i = 0; i < iMinLine; ++i)
				{
					iLen = iLen - (int)(m_kVecLine[i].m_kWstr.length());
					if (m_kVecLine[i].m_bReturn)
					{
						iLen -= 2;
					}
				}

				iLen = __max(0, iLen);

				iMinLen = __min(iLen, (int)(m_kVecLine[iMinLine].m_kWstr.size()));
				for (int i = 0; i < iMinLen; ++i)
				{
					wchar_t wC = m_kVecLine[iMinLine].m_kWstr[i];
					szFront += wC;
				}
			}

			ptLastPos.x += pFont->CalcWidth(szFront);//�ణ ������ ������ ��
		}
	}

	return ptLastPos;
}

void CXUI_Edit_MultiLine::MoveCarotToClickPos(int& iStart, int& iEnd)
{
	if (m_wstrInputText.empty())
	{
		g_kMultiIME.SetCaretPos(0, 0);
		return;
	}

	if (iStart < m_iStartTextPos)
	{
		iStart = m_iStartTextPos;
	}

	if (iStart > m_iEndTextPos)
	{
		iEnd = m_iEndTextPos;
	}

	g_kMultiIME.SetCaretPos(iStart, iEnd);
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
	if (m_wstrInputText.empty())
	{
		return iSize;
	}

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(EditFont());

	return iSize;
}

bool CXUI_Edit_MultiLine::VPeekEvent(E_INPUT_EVENT_INDEX const& rET, POINT3I const& rPT, DWORD const& dwValue)
{
	if (!CXUI_Wnd::Visible() || IsClosed()) { return false; }//�ڽĵ� ����.
	if (!Enable()) { return false; }

	//XUI_Edit�� Ư�� �޽��� ó���� �Ѵ�.
	bool bRet = false;
	m_bDBLClick = false;

#ifdef XUI_USE_SCRIPT_CALL_OPTIMIZE
	unsigned int wstrScriptKey = SCRIPT_MAX_NUM;
#else
	std::wstring wstrScriptKey;
#endif

	switch (rET)
	{
	case IEI_KEY_DOWN:
		//case IEI_KEY_UP:
	{//��Ʈ�ѿ� ���� �Ǿ����. dxxx todo 
		if (IsFocus())
		{
			if (g_kMultiIME.IsNowComp() == false)
			{
				BM::vstring vstr(dwValue);
				bRet = DoHotKey(vstr);
			}
			return true;//���� ��� �Ǵ� ��Ŀ���� �������� ��ǲ�� �Ծ�ġ���� ��
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
		if (ContainsPoint(m_sMousePos))//���콺 �̺�Ʈ�� ���� üũ �ʼ�.
		{
			m_spWndMouseOver = this;//Edit ���� �߰�

			if (MEI_BTN_0 == dwValue)
			{
				IsMouseDown(true);
				VAcquireFocus(this);

				static DWORD dwLastDownTime = 0;
				DWORD const dwNow = BM::GetTime32();

				if (LastMouseDownPos() - XUIMgr.DblClickBound() <= m_sMousePos && LastMouseDownPos() + XUIMgr.DblClickBound() >= m_sMousePos)
				{//���� ��ǥ����.
					if (dwLastDownTime)
					{
						//�־��� //���� �ð� ���� ������.
						if ((dwNow - dwLastDownTime) < XUIMgr.DblClickTick())//200 �и� ���Ϸ� ���Դ�..
						{
							wstrScriptKey = SCRIPT_ON_L_BTN_DBL_DOWN;
							dwLastDownTime = 0;//�̺�Ʈ�� �ð��� 0���� �������ϰ�.
							m_bDBLClick = true;

							bRet = true;
						}
					}
				}

				dwLastDownTime = dwNow;//���� �Ƶ� ������ �ٿ�ð��� ���

				wstrScriptKey = SCRIPT_ON_L_BTN_DOWN;
				LastMouseDownPos(m_sMousePos);

				if (m_spWndMouseOver == this && GetFocusedEdit() != this)//>>Edit ���� �߰�
				{
					if (!m_wstrInputText.empty())//�Էµ� ���ڿ��� ���� ��� �ʱ�ȭ ��Ű�� �ȵ�
					{
						g_kMultiIME.SetString(m_wstrInputText);
						g_kMultiIME.SetCaretPos(false);//ĳ���� �� �ڷ�
						SetEditFocus(true);
					}
					else
					{
						SetEditFocus(false);//������ �Էµ� ���ڰ� Ŭ���� �ǹǷ� �и���
					}
				}
				else if (m_spWndMouseOver == this && GetFocusedEdit() == this)//��Ŀ�� ���� ���¿��� �ٽ� ���� Ŭ��
				{
					int pos = GetClickTextPos();
					//MoveCarotToClickPos(pos, pos);
					if (m_bDBLClick)//��Ŀ�� ���� ���¿��� ����Ŭ���Ǹ� ��� ��������
					{
						g_kMultiIME.SetCaretPos(0, (int)m_wstrInputText.length());
					}
					else
					{
						g_kMultiIME.SetCaretPos(pos, pos);
					}

				}//<<Edit ���� �߰�
			}
			else if (MEI_BTN_1 == dwValue)
			{
				wstrScriptKey = SCRIPT_ON_R_BTN_DOWN;
			}
			bRet = true;
		}
	}break;
	case IEI_MS_UP:
	{
		if (ContainsPoint(m_sMousePos))//���콺 �̺�Ʈ�� ���� üũ �ʼ�.
		{
			if (MEI_BTN_0 == dwValue)
			{
				IsMouseDown(false);
				wstrScriptKey = SCRIPT_ON_L_BTN_UP;
			}
			else if (MEI_BTN_1 == dwValue)
			{
				wstrScriptKey = SCRIPT_ON_R_BTN_UP;
			}
			bRet = true;
		}
	}break;
	case IEI_MS_MOVE:
	{
		bool const bIsBeforeMouseOver = IsMouseOver();
		if (ContainsPoint(m_sMousePos))//���콺 �̺�Ʈ�� ���� üũ �ʼ�.
		{
			bRet = true;
			if (m_spWndMouseOver != this)
			{
				if (m_spWndMouseOver)
				{
					m_spWndMouseOver->DoScript(SCRIPT_ON_MOUSE_OUT);//���� ���콺 �������� �ƿ� ó��.
					m_spWndMouseOver->IsMouseDown(false);
				}

				m_spWndMouseOver = this;// ���콺 �ö�Ծ�.

				wstrScriptKey = SCRIPT_ON_MOUSE_OVER;
			}

			if (IsMouseDown() && CanDrag())
			{
				POINT3I ptOrg = Location();
				Location(ptOrg + rPT);
			}

			if (IsMouseDown())//>>Edit ���� �߰�
			{
				int pos = GetClickTextPos();
				if (g_kMultiIME.GetCaretPos().iSelectStart > pos)
				{
					int start = g_kMultiIME.GetCaretPos().iSelectEnd;
					MoveCarotToClickPos(pos, start);
				}
				else if (g_kMultiIME.GetCaretPos().iSelectStart < pos)
				{
					int start = g_kMultiIME.GetCaretPos().iSelectStart;
					MoveCarotToClickPos(start, pos);
				}
			}//<<Edit ���� �߰�

			if (rPT.z != 0 && ContainsPoint(m_sMousePos)) // ���콺 ���̴�.
			{
				if (rPT.z > 0) { wstrScriptKey = SCRIPT_ON_WHEEL_UP; }
				else { wstrScriptKey = SCRIPT_ON_WHEEL_DOWN; }
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

	bool const bScriptRet = DoScript(wstrScriptKey);//���� �ȵǴ� ������ �����ϱ� ����.

	return (bRet || bScriptRet);
}

void CXUI_Edit_MultiLine::OnHookEvent()
{
	m_wstrInputText = g_kMultiIME.GetResultStr();
}

void CXUI_Edit_MultiLine::EditText(std::wstring const& wstrValue, bool bKeepTextBlock)
{
	m_wstrInputText = wstrValue;
	g_kMultiIME.SetLimitLength(m_iLimitLength, false);//���� ������ �����Ѵ�.
	g_kMultiIME.SetOnlyNumeric(IsOnlyNum(), false);
	g_kMultiIME.SetMultiLine(IsMultiLine(), m_iLimitLength, MultiLineCount(), false);
	g_kMultiIME.SetString(m_wstrInputText);
	g_kMultiIME.SetCaretPos(false);//ĳ���� �� �ڷ�
}

bool CXUI_Edit_MultiLine::SetEditFocus(bool const bIsJustFocus)
{
	if (IsEditDisable())
	{//�����Ұ���!!
		return false;
	}

	if (!m_wstrBlockPath.empty())
	{
		m_spRscMgr->ReleaseRsc(CXUI_Edit::m_spTextBlockBgImg);
		CXUI_Edit::m_spTextBlockBgImg = m_spRscMgr->GetRsc(m_wstrBlockPath);
		m_siBlockImgIdx = -2;
	}

	if (!m_wstrCarotPath.empty())
	{
		m_spRscMgr->ReleaseRsc(CXUI_Edit::m_spCarotImg);
		CXUI_Edit::m_spCarotImg = m_spRscMgr->GetRsc(m_wstrCarotPath);
		m_siCarotImgIdx = -1;
	}

	VAcquireFocus(this);
	CXUI_Edit::SetFocusedEdit(this);

	if (!bIsJustFocus)
	{
		m_wstrInputText = _T("");
	}

	g_kMultiIME.SetLimitLength(m_iLimitLength, false);//���� ������ �����Ѵ�.
	g_kMultiIME.SetOnlyNumeric(IsOnlyNum(), false);
	g_kMultiIME.SetMultiLine(IsMultiLine(), m_iLimitLength, MultiLineCount(), false);
	g_kMultiIME.SetString(m_wstrInputText);
	g_kMultiIME.SetPasswordMode(IsSecret());
	g_kMultiIME.SetEnglishIME(false);
	g_kMultiIME.SetEnableIME(true);
	if (IsNativeIME())
	{
		g_kMultiIME.SetNativeIME();
	}

	DoScript(SCRIPT_ON_FOCUS);	//��Ŀ���� �������� (ON_FOCUS)
	return true;
}


void CXUI_Edit_MultiLine::RenderCarot(POINT2& pt)	//ĳ�����
{
	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(EditFont());

	if (CXUI_Edit::m_spTextBlockBgImg == NULL
		|| CXUI_Edit::m_spCarotImg == NULL
		|| !pFont)
	{
		assert(NULL);
		return;
	}

	void* pImg = NULL;
	int* pImgIdx = NULL;
	SRenderInfo kRenderInfo;
	int iAdd = 0;

	if (g_kMultiIME.IsNowComp())	//�ѱ��̸�
	{
		pt.x -= pFont->GetHeight();

		pImg = CXUI_Edit::m_spTextBlockBgImg;
		pImgIdx = &m_siBlockImgIdx;

		SSizedScale& rSS = kRenderInfo.kSizedScale;
		rSS.ptSrcSize = POINT2(16, 16);//xxx todo �ϵ��ڵ�
		rSS.ptDrawSize = POINT2(pFont->GetHeight() - 4, pFont->GetHeight());
		iAdd = 4;//xxx todo �ϵ��ڵ�
	}
	else
	{
		pImg = CXUI_Edit::m_spCarotImg;
		pImgIdx = &m_siCarotImgIdx;

		SSizedScale& rSS = kRenderInfo.kSizedScale;
		rSS.ptSrcSize = POINT2(16, 16);//xxx todo �ϵ��ڵ�
		rSS.ptDrawSize = POINT2(1, pFont->GetHeight());
	}

	kRenderInfo.kUVInfo = UVInfo();
	kRenderInfo.kLoc = pt;
	kRenderInfo.kLoc.x += iAdd;
	GetParentDrawRect(kRenderInfo.rcDrawable);
	kRenderInfo.fAlpha = Alpha();

	if (pImg)
	{
		m_spRenderer->RenderSprite(pImg, *pImgIdx, kRenderInfo);
	}
}