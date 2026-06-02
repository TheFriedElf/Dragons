
inline void SET_XUI_CONTROL_TEXT(XUI::CXUI_Wnd * pkWnd, wchar_t const* pkFind,  wchar_t const* pkText)
{
	if(!pkWnd || !pkFind)
	{
		return;
	}

	XUI::CXUI_Wnd * pkChildWnd = pkWnd->GetControl(pkFind);
	if( pkChildWnd )
	{
		pkChildWnd->Text(pkText ? pkText : L"");
	}
}