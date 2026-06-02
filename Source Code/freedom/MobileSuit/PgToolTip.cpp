#include "stdafx.h"
#include "PgToolTip.h"
#include "ServerLib.h"

#pragma pack(1)
typedef struct tagParsedText
{
	DWORD dwOptionFlag;
	DWORD dwColor;
	size_t szSize;
	std::wstring strFont;
	std::wstring strText;
}PgParsedText;
#pragma pack()

bool ParseValue(const std::wstring &rkOrg, wchar_t option, std::wstring &Out)
{
	const std::wstring::size_type pos_org = rkOrg.find(option);

	if(pos_org != std::wstring::npos)
	{
		const std::wstring::size_type value_start = rkOrg.find( _T('='), pos_org);
		const std::wstring::size_type value_end = rkOrg.find( _T('/'), pos_org);

		if(std::wstring::npos != value_start
		&&	std::wstring::npos != value_end)
		{
			Out = rkOrg.substr(value_start+1, value_end-value_start-1);
			return true;
		}
	}
	return false;
}

bool ParseText(const std::wstring &strText, std::list<PgParsedText> &rkOutList)
{
	std::wstring::size_type start = strText.find(_T('<'));
	
	PgParsedText kPrevParseText;

	while(start != std::wstring::npos)
	{
		++start;
		PgParsedText kParseText = kPrevParseText;

		std::wstring::size_type end = strText.find(_T('>'),start);
		if(end != std::wstring::npos)
		{//내부에서 끊자.
			const std::wstring strOption = strText.substr(start, end-start);

			std::wstring strValue;
			if(ParseValue(strOption, _T('S'), strValue))
			{
				BM::vstring vstr(strValue);
				kParseText.szSize = (int)vstr;
			}

			if(ParseValue(strOption, _T('C'), strValue))
			{
				DWORD dwColor = 0;
				::swscanf_s(strValue.c_str(), _T("%x"), &dwColor);
				kParseText.dwColor = dwColor;
			}
			
			if(ParseValue(strOption, _T('O'), strValue))
			{
				strValue.find(_T('B'));
				strValue.find(_T('I'));
				strValue.find(_T('U'));
			}
			
			if(ParseValue(strOption, _T('F'), strValue))
			{
				kParseText.strFont = strValue;
			}
		}

		start = strText.find(_T('<'), start);

		if(start != std::wstring::npos//글이 다 끝났을 수도 있고.
		&& end != std::wstring::npos	)//끝마크가 없을수도 있지.
		{
			kParseText.strText = strText.substr(end+1, start-end-1);
		}
		else
		{
			kParseText.strText = strText.substr(end+1);
		}

		rkOutList.push_back(kParseText);
		kPrevParseText = kParseText;
	}
	return true;
}

PgToolTip::PgToolTip(void)
{
	Location(0,0);
}

PgToolTip::~PgToolTip(void)
{

}

void PgToolTip::VInit()
{
	if(m_iFontHeight > 0)
		return;
	CXUI_Wnd::VInit();
	m_ImgIdx = -1;
}

bool PgToolTip::VPeekEvent(const E_INPUT_EVENT_INDEX &rET, const POINT3I &rPT, const DWORD &dwValue)
{
	return CXUI_Wnd::VPeekEvent(rET, rPT, dwValue);
/*	switch(rET)
	{
	case IEI_MS_MOVE:	
		{
			if(m_spWndMouseOver != 0 && m_sMousePos.x >= 0 && m_sMousePos.y >= 0 )
			{
				Location( m_sMousePos.x,  m_sMousePos.y+30);	//마우스 위치보다 약간 아래쪽에 뿌림
			}
			else
			{
				Location(0,0);
				this->Visible(false);
				m_Text.clear();
			}
		}break;
	}
*/
	return false;
}

void PgToolTip::VOnTick()		//마우스는 안움직이고 창이 사라질 경우가 있으므로
{
/*	if(m_spWndMouseOver == 0 || m_sMousePos.x < 0 || m_sMousePos.y < 0 || 
		m_spWndMouseOver->Location().x < 0 || m_spWndMouseOver->Location().y < 0
		|| m_iFontHeight <= 0 || m_spWndMouseOver->Location().x > 2000 || m_spWndMouseOver->Location().y > 1000)
	{	
		this->Visible(false);
//		Location(0,0);
		m_Text.clear();
		return;
	}
	
	if(m_spWndMouseOver->ToolTip().empty())	//셋팅된 글자가 없으면 뿌리지 않음
	{
		this->Visible(false);
//		Location(0,0);
		m_Text.clear();
		return;
	}

	Location( m_sMousePos.x,  m_sMousePos.y+30);	//마우스 위치보다 약간 아래쪽에 뿌림
	this->Visible(true);
//	MakeToolTipString(m_spWndMouseOver->ToolTip());
*/
}

bool PgToolTip::VDisplay()
{
	//빌드.. 빌드빌드..
	std::wstring wstrRet;

	const CItemDef *pDef = g_ItemDefMgr.GetDef(9100000);

	WORD Type;
	int Value;
	pDef->FirstAbil(Type, Value);

	const wchar_t *pText = 0;
	if(GetAbilName(Type, pText))
	{
		wstrRet += _T("<S=11/C=AAAAAA/O=BIU/F=sysinfo/>");
		wstrRet += pText;
		wstrRet += _T("\n");
	}

	while( pDef->NextAbil(Type, Type, Value) )
	{
		if(GetAbilName(Type, pText))
		{
			wstrRet += _T("<S=11/C=AAAAAA/O=BIU/F=sysinfo/>");
			wstrRet += pText;
			wstrRet += _T("\n");
		}
	}

	std::list<PgParsedText> ParseList;
	ParseText(wstrRet, ParseList);

	POINT2 pt = TotalLocation();

//	std::list<PgParsedText>::iterator itor = ParseList.begin();
//	while(itor != ParseList.end())
//	{
//		RenderText( pt, (*itor).strText, (*itor).strFont, FontFlag() );
//		++itor;
//	}

	if( !Visible() ){return true;}//자식컨트롤도 안그림
	DoScript( SCRIPT_ON_DISPLAY );
	DisplayControl();

	//와우와 흡사하게!
	//아이콘 이미지
	//바탕 이미지.
	//글자

//	SetToolTip( 번호? )
		
//	SetToolTip( XML 데이터? )
	
	//파서 만들어 써야되나??


	IncAlpha();
	if( Text().length()  )
	{
		const POINT3I pt = TotalLocation() + TextPos();

		RenderText( pt, Text(), Font(), FontFlag() );
	}

	void *pImg = DefaultImg();
	int x = ImgSize().x;
	if(m_iLineCount==0)
	{
		x = m_iLastWidth+1;
	}

	if( pImg && m_spRenderer )
	{
		SRenderInfo kRenderInfo;

		SSizedScale &rSS = kRenderInfo.kSizedScale;

		rSS.ptSrcSize = POINT2(128,12);//xxx todo 하드코딩
		rSS.ptDrawSize = POINT2(x,(18)*(m_iLineCount+1));

		kRenderInfo.kUVInfo = UVInfo();
		kRenderInfo.kLoc = TotalLocation();
		GetParentRect(kRenderInfo.rcDrawable);
		kRenderInfo.fAlpha = Alpha();
	
		m_spRenderer->RenderSprite( pImg, m_ImgIdx, kRenderInfo);
	} 
	return true;
}
/*
void PgToolTip::MakeToolTipString(const std::wstring & Val)
{	
	m_iLineCount = 0;	//항상 계산전 초기화
	m_iLastWidth = 0;
	m_Text.clear();
	std::wstring szLineText;	//한줄씩 넣어봐야 하니까
	for(unsigned i = 0; i < Val.size();)
	{
		szLineText+=Val[i];
		m_iLastWidth = CalcWidth(szLineText);
		if(m_iLastWidth <= ImgSize().x )	//10글자 단위로 끊자
		{
			m_Text+=Val[i];
			++i;
		}
		else
		{
			++m_iLineCount;
			m_Text+=_T("\n");
			szLineText.clear();
		}
	}
}
*/
/*
int PgToolTip::CalcWidth(const std::wstring &text) const
{
	unsigned short sCode;
	size_t iLen = text.length();
	int iWidth = 0;
	for(size_t i=0;i<iLen;i++)
	{
		sCode = (unsigned short)text[i];
		iWidth += GetWidth(sCode);
	}
	return iWidth;
}

int PgToolTip::GetWidth(unsigned short code) const
{
	int iError = FT_Load_Char( m_FontFace, code, FT_LOAD_RENDER );
    if ( iError )
	{
		//	failed
      return iError;
	}

	FT_GlyphSlot slot = m_FontFace->glyph;

	int iBitmapWidth = slot->bitmap.width;
	if(slot->bitmap_left>0) iBitmapWidth += slot->bitmap_left;

	if(code == 32 && iBitmapWidth==0) iBitmapWidth = m_iFontHeight/2;

	return iBitmapWidth;
}
*/