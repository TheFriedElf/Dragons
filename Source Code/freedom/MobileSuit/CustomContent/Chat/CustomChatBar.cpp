#include "stdafx.h"
#include "CustomChatbar.h"

#include "../../Pg2DString.H"
#include "../../PgChatMgrClient.h"

#include "XUI/XUI_Edit.h"
#include "XUI/XUI_Edit_MultiLine.h"
#include "XUI/XUI_Font.h"

namespace
{
	int const kChatEditHorizontalPadding = 8;
	int const kMaximumChatCharacterCount = 200;
	int const kMinimumVisibleLineCount = 1;
	int const kMaximumVisibleLineCount = 6;
	int const kDefaultLineHeight = 16;

	struct SChatBarLayoutState
	{
		bool bInitialized;
		int iChatBarHeight;
		int iButtonBgY;
		int iButtonTellY;
		int iButtonWhisperY;
		int iButtonToggleY;
		int iFormToggleY;
		int iBarBgHeight;
		int iEditBgHeight;
		int iEditBgShadowHeight;
		int iHeadHeight;
		int iIconHeight;
		int iEditHeight;

		SChatBarLayoutState()
			: bInitialized(false)
			, iChatBarHeight(0)
			, iButtonBgY(0)
			, iButtonTellY(0)
			, iButtonWhisperY(0)
			, iButtonToggleY(0)
			, iFormToggleY(0)
			, iBarBgHeight(0)
			, iEditBgHeight(0)
			, iEditBgShadowHeight(0)
			, iHeadHeight(0)
			, iIconHeight(0)
			, iEditHeight(0)
		{}
	};

	int GetChatEditVisibleWidth(XUI::CXUI_Edit* pChatEditControl)
	{
		if (!pChatEditControl)
		{
			return 0;
		}

		return (pChatEditControl->Size().x > kChatEditHorizontalPadding)
			? (pChatEditControl->Size().x - kChatEditHorizontalPadding)
			: 0;
	}

	int GetChatEditLineHeight(XUI::CXUI_Edit* pChatEditControl)
	{
		if (!pChatEditControl)
		{
			return kDefaultLineHeight;
		}

		XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(pChatEditControl->EditFont());
		return pFont ? __max(kMinimumVisibleLineCount, pFont->GetHeight()) : kDefaultLineHeight;
	}

	void ResizeControlHeight(XUI::CXUI_Wnd* pControl, int iBaseHeight, int iExtraHeight)
	{
		if (!pControl)
		{
			return;
		}

		POINT2 kSize = pControl->Size();
		kSize.y = iBaseHeight + iExtraHeight;
		pControl->Size(kSize);
	}

	void MoveControlY(XUI::CXUI_Wnd* pControl, int iBaseY, int iExtraHeight)
	{
		if (!pControl)
		{
			return;
		}

		POINT3I kLocation = pControl->Location();
		kLocation.y = iBaseY + iExtraHeight;
		pControl->Location(kLocation);
	}

	int GetChatBarVisibleLineCount(XUI::CXUI_Edit* pChatEditControl)
	{
		if (!pChatEditControl)
		{
			return kMinimumVisibleLineCount;
		}

		int const iVisibleWidth = GetChatEditVisibleWidth(pChatEditControl);
		if (0 >= iVisibleWidth)
		{
			return kMinimumVisibleLineCount;
		}

		int const iLineHeight = GetChatEditLineHeight(pChatEditControl);

		POINT const kWrappedSize = Pg2DString::CalculateOnlySize(pChatEditControl->StyleText(), iVisibleWidth, true);
		int iLineCount = (kWrappedSize.y > 0) ? ((kWrappedSize.y + iLineHeight - 1) / iLineHeight) : kMinimumVisibleLineCount;
		iLineCount = __max(kMinimumVisibleLineCount, iLineCount);
		iLineCount = __min(kMaximumVisibleLineCount, iLineCount);
		return iLineCount;
	}

	void UpdateChatBarMultilineLayout(XUI::CXUI_Wnd* pChatBarWindow, XUI::CXUI_Edit* pChatEditControl)
	{
		if (!pChatBarWindow || !pChatEditControl)
		{
			return;
		}

		XUI::CXUI_Edit_MultiLine* pChatEditMultiLine = dynamic_cast<XUI::CXUI_Edit_MultiLine*>(pChatEditControl);
		if (!pChatEditMultiLine)
		{
			return;
		}

		static SChatBarLayoutState layoutState;
		XUI::CXUI_Wnd* pBarBg = pChatBarWindow->GetControl(_T("SFRM_BAR_BG"));
		XUI::CXUI_Wnd* pEditBg = pChatBarWindow->GetControl(_T("SFRM_EDT_BG"));
		XUI::CXUI_Wnd* pHead = pChatBarWindow->GetControl(_T("FRM_HEAD"));
		XUI::CXUI_Wnd* pIcon = pChatBarWindow->GetControl(_T("ICON_CHAT"));
		XUI::CXUI_Wnd* pButtonBg = pChatBarWindow->GetControl(_T("SFRM_BTN_BG"));
		XUI::CXUI_Wnd* pTellButton = pChatBarWindow->GetControl(_T("BTN_TELL_TYPE"));
		XUI::CXUI_Wnd* pWhisperButton = pChatBarWindow->GetControl(_T("BTN_WHISPER"));
		XUI::CXUI_Wnd* pToggleButton = pChatBarWindow->GetControl(_T("BTN_TOGGLECHAT"));
		XUI::CXUI_Wnd* pToggleForm = pChatBarWindow->GetControl(_T("FRM_TOGGLECHAT"));
		XUI::CXUI_Wnd* pEditBgShadow = pEditBg ? pEditBg->GetControl(_T("SFRM_EDT_BG_SHADOW")) : NULL;

		if (!layoutState.bInitialized)
		{
			layoutState.bInitialized = true;
			layoutState.iChatBarHeight = pChatBarWindow->Size().y;
			layoutState.iButtonBgY = pButtonBg ? pButtonBg->Location().y : 0;
			layoutState.iButtonTellY = pTellButton ? pTellButton->Location().y : 0;
			layoutState.iButtonWhisperY = pWhisperButton ? pWhisperButton->Location().y : 0;
			layoutState.iButtonToggleY = pToggleButton ? pToggleButton->Location().y : 0;
			layoutState.iFormToggleY = pToggleForm ? pToggleForm->Location().y : 0;
			layoutState.iBarBgHeight = pBarBg ? pBarBg->Size().y : 0;
			layoutState.iEditBgHeight = pEditBg ? pEditBg->Size().y : 0;
			layoutState.iEditBgShadowHeight = pEditBgShadow ? pEditBgShadow->Size().y : 0;
			layoutState.iHeadHeight = pHead ? pHead->Size().y : 0;
			layoutState.iIconHeight = pIcon ? pIcon->Size().y : 0;
			layoutState.iEditHeight = pChatEditControl->Size().y;
		}

		int const iLineHeight = GetChatEditLineHeight(pChatEditControl);
		int const iVisibleLineCount = GetChatBarVisibleLineCount(pChatEditControl);
		int const iExtraHeight = __max(0, (iVisibleLineCount - 1) * iLineHeight);

		POINT2 kChatBarSize = pChatBarWindow->Size();
		kChatBarSize.y = layoutState.iChatBarHeight + iExtraHeight;
		pChatBarWindow->Size(kChatBarSize);

		ResizeControlHeight(pBarBg, layoutState.iBarBgHeight, iExtraHeight);
		ResizeControlHeight(pEditBg, layoutState.iEditBgHeight, iExtraHeight);
		ResizeControlHeight(pEditBgShadow, layoutState.iEditBgShadowHeight, iExtraHeight);
		ResizeControlHeight(pHead, layoutState.iHeadHeight, iExtraHeight);
		ResizeControlHeight(pIcon, layoutState.iIconHeight, iExtraHeight);
		ResizeControlHeight(pChatEditControl, layoutState.iEditHeight, iExtraHeight);

		MoveControlY(pButtonBg, layoutState.iButtonBgY, iExtraHeight);
		MoveControlY(pTellButton, layoutState.iButtonTellY, iExtraHeight);
		MoveControlY(pWhisperButton, layoutState.iButtonWhisperY, iExtraHeight);
		MoveControlY(pToggleButton, layoutState.iButtonToggleY, iExtraHeight);
		MoveControlY(pToggleForm, layoutState.iFormToggleY, iExtraHeight);
	}

	bool IsChatBarEditControl(XUI::CXUI_Edit* pChatEditControl)
	{
		if (!pChatEditControl)
		{
			return false;
		}

		XUI::CXUI_Wnd* pChatBarWindow = XUIMgr.Get(_T("ChatBar"));
		if (!pChatBarWindow)
		{
			return false;
		}

		return (pChatEditControl == dynamic_cast<XUI::CXUI_Edit*>(pChatBarWindow->GetControl(_T("EDT_CHAT"))));
	}

	void CalculateWrappedCaretOffset(XUI::CXUI_Style_String const& wrappedStyleText, int iDefaultLineHeight, POINT2& kOutOffset)
	{
		kOutOffset = POINT2(0, 0);
		int iCurrentLineWidth = 0;
		int iCurrentLineHeight = __max(kMinimumVisibleLineCount, iDefaultLineHeight);

		XUI::CONT_PARSED_CHAR const& kParsedCharacters = wrappedStyleText.GetCharVector();
		for (size_t iCharacterIndex = 0; iCharacterIndex < kParsedCharacters.size(); ++iCharacterIndex)
		{
			XUI::PgParsedChar const& kParsedCharacter = kParsedCharacters[iCharacterIndex];
			if (_T('\r') == kParsedCharacter.m_wChar)
			{
				continue;
			}

			if (_T('\n') == kParsedCharacter.m_wChar)
			{
				kOutOffset.y += iCurrentLineHeight;
				iCurrentLineWidth = 0;
				iCurrentLineHeight = __max(kMinimumVisibleLineCount, iDefaultLineHeight);
				continue;
			}

			iCurrentLineHeight = __max(iCurrentLineHeight, kParsedCharacter.m_iAlignHeight);
			iCurrentLineWidth += XUI::CXUI_Style_String::GetCharWidth(kParsedCharacter);
		}

		kOutOffset.x = iCurrentLineWidth;
	}
}

bool TryGetCustomChatBarCaretPos(XUI::CXUI_Edit* pChatEditControl, std::wstring const& wstrInputText, CS::CARETDATA const& rkData, POINT2 const& kBaseCaretPos, int iWrapWidth, int iDefaultLineHeight, POINT2& kOutCaretPos)
{
	if (!IsChatBarEditControl(pChatEditControl))
	{
		return false;
	}

	XUI::CXUI_Font* pFont = g_kFontMgr.GetFont(pChatEditControl->EditFont());
	if (!pFont)
	{
		return false;
	}

	int const iClampedWrapWidth = __max(0, iWrapWidth);
	if (0 >= iClampedWrapWidth)
	{
		return false;
	}

	int const iClampedCaretPos = __max(0, __min(static_cast<int>(wstrInputText.length()), rkData.iCaretPos));
	std::wstring const wstrInputPrefix = wstrInputText.substr(0, iClampedCaretPos);
	std::wstring const wstrPreviewPrefix = g_kChatMgrClient.ConvertUserCommand(wstrInputPrefix);

	XUI::PgFontDef const kFontDefinition(pFont, pChatEditControl->FontColor(), pChatEditControl->FontFlag());
	XUI::CXUI_Style_String kPreviewPrefixStyle(kFontDefinition, wstrPreviewPrefix);
	XUI::CXUI_Style_String kWrappedPreviewPrefixStyle;
	kPreviewPrefixStyle.ProcessWordWrap(iClampedWrapWidth, true, kWrappedPreviewPrefixStyle);
	kWrappedPreviewPrefixStyle.RecalculateAlignHeight();

	POINT2 kCaretOffset;
	CalculateWrappedCaretOffset(kWrappedPreviewPrefixStyle, iDefaultLineHeight, kCaretOffset);

	kOutCaretPos = kBaseCaretPos + kCaretOffset;
	return true;
}

void CustomChatbar::UpdateChatBarLayout()
{
	XUI::CXUI_Wnd* pChatBarWindow = XUIMgr.Get(_T("ChatBar"));
	if (!pChatBarWindow)
	{
		return;
	}

	XUI::CXUI_Edit* pChatEditControl = dynamic_cast<XUI::CXUI_Edit*>(pChatBarWindow->GetControl(_T("EDT_CHAT")));
	if (!pChatEditControl)
	{
		return;
	}

	pChatEditControl->LimitLength(kMaximumChatCharacterCount);

	UpdateChatBarMultilineLayout(pChatBarWindow, pChatEditControl);
}