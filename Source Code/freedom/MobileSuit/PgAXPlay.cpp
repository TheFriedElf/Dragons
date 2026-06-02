#include "stdafx.h"
#include "PgMobileSuit.h"
#include "PgAXPlay.h"
#include "PgMovieMgr.h"

CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap) 
END_OBJECT_MAP() 

PgAXPlay::PgAXPlay()
: m_bPlay(false), m_bInit(false), m_hWndIE(NULL)
{
	AtlAxWinInit(); 
	GUID guid;
	_Module.Init(ObjectMap, g_pkApp->GetInstanceReference(), &guid);
}
PgAXPlay::~PgAXPlay()
{
	_Module.Term(); 
	AtlAxWinTerm();
}
bool PgAXPlay::Init()
{
	if(IsInit())
		return true;

	if(IsPlay())
		Stop();

	m_bInit = true;
	return true;
}
bool PgAXPlay::CreateWB()
{
	if(m_hWndIE)
		return false;

	RECT RealRect;
	GetClientRect(g_pkApp->GetWindowReference(), &RealRect);
	int const iWidth = RealRect.right - RealRect.left;
	int const iHeight = RealRect.bottom - RealRect.top;
    //윈도우7에서 모니터해상도보다 게임해상도가 크거나 같으면, 윈도우 크기를 줄여버림. 때문에 실제 윈도우 크기를 받아서 생성하도록 수정
	m_hWndIE = CreateWindow(_T(ATLAXWIN_CLASS), L"Shell.Explorer.2", WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS, 0, 0, 
		iWidth, iHeight, g_pkApp->GetWindowReference(), 
		(HMENU)0, g_pkApp->GetInstanceReference(), NULL);
	
	if (m_hWndIE == 0)
	{
		MessageBox(g_pkApp->GetWindowReference(), L"Unalbe to Initialize WB", L"WB Failed", MB_OK);
		return false;
		//int i = GetLastError();
	}

	if (AtlAxGetControl(m_hWndIE, &m_pUnkIE) == S_OK) 
	{ 
		m_pWB2 = m_pUnkIE; 
		if (m_pWB2) 
		{ 
			CComObject<PgIESink>::CreateInstance(&m_pSink); 
			m_pSink->AddRef(); 
			HRESULT hr = m_pSink->AdviseToIE(m_pUnkIE, m_pWB2); 
			m_pWB2->put_FullScreen(VARIANT_TRUE); 
		} 
		else 
		{ 
			MessageBox(g_pkApp->GetWindowReference(), L"Unalbe to Initialize WB", L"WB Failed", MB_OK);
			return false;
		} 
	}
	else
	{
		MessageBox(g_pkApp->GetWindowReference(), L"Unalbe to Initialize WB", L"WB Failed", MB_OK);
		return false;
	}

	ShowWindow(m_hWndIE, HIDE_WINDOW);
	UpdateWindow(m_hWndIE);
	//ShowWindow(m_hWndIE, SW_RESTORE);
	//UpdateWindow(m_hWndIE);
	return true;
}
void PgAXPlay::Terminate()
{
	if(!IsInit())
		return;

	Stop();

	m_bInit = false;
}
bool PgAXPlay::Play(std::string strPath)
{
	//DWORD dwTimeOld = BM::GetTime32();
	if(!IsInit())
		return false;
	//DWORD dwTimeNew = BM::GetTime32();
	//_PgOutputDebugString("Time Init %d\n", dwTimeNew - dwTimeOld);
	//dwTimeOld = dwTimeNew;

	if(!CreateWB())
		return false;
	//dwTimeNew = BM::GetTime32();
	//_PgOutputDebugString("Time WB %d\n", dwTimeNew - dwTimeOld);
	//dwTimeOld = dwTimeNew;
	
	if(!m_hWndIE || !m_pWB2)
		return false;

	Sleep(2500);

	ShowWindow(m_hWndIE, SW_SHOWNORMAL);
	UpdateWindow(m_hWndIE);

	char pszPath[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, pszPath);
	char pszBuf[256]={0,};
	sprintf_s(pszBuf, 256, "file:///%s/%s", pszPath, strPath.c_str());


	CComVariant urlString( pszBuf ); 

	CComVariant empty; 
	CComVariant targetFrameName("_self");
	m_pWB2->Navigate2(&urlString, &empty, &targetFrameName, &empty, &empty);
	//dwTimeNew = BM::GetTime32();
	//_PgOutputDebugString("Time Play %d\n", dwTimeNew - dwTimeOld);
	//dwTimeOld = dwTimeNew;

	m_bPlay = true;
	return true;
}

bool PgAXPlay::Stop()
{
	if(!IsInit())
		return false;
	if(m_pSink)
	{
		m_pSink->DispEventUnadvise(m_pUnkIE);
		m_pSink = NULL;
	}
	if(m_hWndIE)
	{
		SendMessage(m_hWndIE, WM_CLOSE, 0, 0);
	}

	m_hWndIE = NULL;
	m_pUnkIE = NULL;

	if(m_pWB2)
	{
		m_pWB2.Release();
	}

	m_bPlay = false;
	return true;
}
bool PgAXPlay::Update()
{
	return m_bPlay;
}

void _stdcall PgIESink::OnDocumentComplete(IDispatch** ppDisp, VARIANT *pvUrl)
{
	if(pvUrl == NULL || pvUrl->bstrVal == NULL)
		return;

	std::string strName(MB(pvUrl->bstrVal));
	if(strName == "about:blank")
	{
		g_kMovieMgr.Stop();
	}
}