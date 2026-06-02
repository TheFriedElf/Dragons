#include "stdafx.h"
#include "CustomEmoticons.h"
#include "CustomChatbar.h"

#include "../../PgChatMgrClient.h"
#include "../../Pg2DString.H"

#include "XUI/XUI_Edit.h"

namespace
{
	int const kChatEditHorizontalPadding = 8;

	void ResetPreviewState(std::wstring& previousInputText, bool& hadPreviewLastUpdate)
	{
		previousInputText.clear();
		hadPreviewLastUpdate = false;
	}

	void StorePreviewState(std::wstring& previousInputText, bool& hadPreviewLastUpdate, std::wstring const& inputText, bool bHasPreview)
	{
		previousInputText = inputText;
		hadPreviewLastUpdate = bHasPreview;
	}

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

	bool CanDisplayPreviewText(XUI::CXUI_Edit* pChatEditControl, std::wstring const& previewText)
	{
		if (!pChatEditControl)
		{
			return false;
		}

		pChatEditControl->Text(previewText);

		POINT const previewSize = Pg2DString::CalculateOnlySize(pChatEditControl->StyleText());
		int const visibleWidth = GetChatEditVisibleWidth(pChatEditControl);
		return (previewSize.x <= visibleWidth);
	}
}

void CustomEmoticons::UpdateChatBarPreview(PgChatMgrClient& kChatMgrClient)
{
	static std::wstring previousInputText;
	static bool hadPreviewLastUpdate = false;

	XUI::CXUI_Wnd* pChatBarWindow = XUIMgr.Get(_T("ChatBar"));
	if (!pChatBarWindow)
	{
		ResetPreviewState(previousInputText, hadPreviewLastUpdate);
		return;
	}

	XUI::CXUI_Edit* pChatEditControl = dynamic_cast<XUI::CXUI_Edit*>(pChatBarWindow->GetControl(_T("EDT_CHAT")));
	if (!pChatEditControl)
	{
		ResetPreviewState(previousInputText, hadPreviewLastUpdate);
		return;
	}

	if (pChatEditControl->IsMultiLine())
	{
		std::wstring const inputText = pChatEditControl->EditText();
		std::wstring const previewText = kChatMgrClient.ConvertUserCommand(inputText);
		bool const hasPreview = (previewText != inputText);

		pChatEditControl->Text(hasPreview ? previewText : inputText);
		CustomChatbar::UpdateChatBarLayout();

		StorePreviewState(previousInputText, hadPreviewLastUpdate, inputText, hasPreview);
		return;
	}

	std::wstring const inputText = pChatEditControl->EditText();
	std::wstring const previewText = kChatMgrClient.ConvertUserCommand(inputText);
	bool const hasPreview = (previewText != inputText);

	if (!hadPreviewLastUpdate && !hasPreview && previousInputText == inputText)
	{
		return;
	}

	std::wstring const* pDisplayText = &inputText;
	if (hasPreview)
	{
		bool bCanDisplayPreview = true;
		if (!pChatEditControl->IsMultiLine())
		{
			bCanDisplayPreview = CanDisplayPreviewText(pChatEditControl, previewText);
		}

		if (bCanDisplayPreview)
		{
			pDisplayText = &previewText;
		}
	}

	pChatEditControl->Text(*pDisplayText);
	CustomChatbar::UpdateChatBarLayout();

	StorePreviewState(previousInputText, hadPreviewLastUpdate, inputText, hasPreview);
}