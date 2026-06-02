#include "stdafx.h"
#include "PgMobileSuit.h"
#include "PgDshowPlay.h"

LONGLONG const llMediaTime = 10000000;

long const VIEWSIZE_X = 428;
long const VIEWSIZE_Y = 320;

PgDshowPlay::PgDshowPlay() 
: m_bInit(false), m_bPlay(false), m_hMediaWindow(NULL), m_pGraph(NULL), m_pControl(NULL), m_pEvent(NULL), m_pVideo(NULL), m_hVideoWindow(NULL), m_pSeeking(NULL)
{

}
PgDshowPlay::~PgDshowPlay()
{

}

bool PgDshowPlay::Init()
{
	if(m_bInit)
	{
		return true;
	}
	HRESULT hr = CoInitialize(NULL);

	if (FAILED(hr)) 
	{
		_PgOutputDebugString("ERROR - Could not initialize COM library"); 
		return false; 
	} 

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
		IID_IGraphBuilder, (void **) &m_pGraph); 
	if (FAILED(hr)) 
	{ 
		_PgOutputDebugString("ERROR - Could not create the Filter Graph Manager. "); 
		return false; 
	} 

	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **) &m_pControl); 
	if (FAILED(hr)) 
	{ 
		_PgOutputDebugString("ERROR - Could not have MediaControl. "); 
		return false; 
	} 
	hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void **) &m_pEvent); 
	if (FAILED(hr)) 
	{ 
		_PgOutputDebugString("ERROR - Could not have MediaEvent. "); 
		return false; 
	} 
	hr = m_pGraph->QueryInterface(IID_IMediaSeeking, (void **) &m_pSeeking);
	if(FAILED(hr))
	{
		_PgOutputDebugString("ERROR - Could not have MediaSeeking.");
		return false;
	}

	if(!g_pkApp)
	{
		return false;
	}

	HWND hWnd;
	hWnd = g_pkApp->GetWindowReference();
	if(!SetMediaWindow(hWnd)) 
	{
		return false;
	}

	m_bInit = true;
	return true;
}
void PgDshowPlay::Terminate()
{
	if(!m_bInit)
	{
		return;
	}

	ReleaseVideoWin();

	if(m_pControl)
	{
		m_pControl->Release(); 
		m_pControl = NULL;
	}
	if(m_pEvent)
	{
		m_pEvent->Release(); 
		m_pEvent = NULL;
	}
	if(m_pSeeking)
	{
		m_pSeeking->Release();
		m_pSeeking = NULL;
	}
	if(m_pGraph)
	{
		m_pGraph->Release();
		m_pGraph = NULL;
	}

	CoUninitialize();

	m_bInit = false;
}
bool PgDshowPlay::Play(std::string const& strPath, float fStartPos, float fEndPos, float fVolumeDecTime, bool const& bFullScreenMode)
{
	if(strPath.empty())
	{
		return false;
	}

	Init();
	InitVideoWin(strPath, bFullScreenMode);

	if( -1 != fStartPos )
	{
		LONGLONG pos = static_cast<LONGLONG>(fStartPos * llMediaTime);
		if( -1 != fEndPos )
		{
			LONGLONG endpos = static_cast<LONGLONG>(fEndPos * llMediaTime);
			m_pSeeking->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, &endpos, AM_SEEKING_AbsolutePositioning);
		}
		else
		{
			m_pSeeking->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
		}
	}

	HRESULT hr = m_pControl->Run(); 
	if (FAILED(hr)) 
	{
		return false;
	}

	//hr = m_pSeek->GetPositions(&nPosition, &nStop); 
	//nPosition += (LONGLONG)(nAddPos * 10000000); 
	//m_pSeek->SetPositions(&nPosition, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning); 
	//m_pControl->Run(); 

	m_pVideo->HideCursor(OATRUE);

	m_fVolumeDecTime = fVolumeDecTime;
	m_bPlay = true;

	return true;
}
bool PgDshowPlay::Stop()
{
	if(!m_bInit)
	{
		return true;
	}
	if(!m_pControl)
	{
		return false;
	}
	HRESULT hr = m_pControl->Stop();
	if(FAILED(hr))
	{
		return false;
	}

	//{
	//	LONGLONG pos = 0;
	//	m_pSeeking->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
	//	m_pControl->Pause();
	//}

	//ReleaseVideoWin();
	Terminate();
	
	m_bPlay = false;
	return true;
}
bool PgDshowPlay::Update()
{
	if(!IsPlay())
	{
		return false;
	}

	long lEventCode = 0;
	long lParam1 = 0;
	long lParam2 = 0;
	HRESULT hr = m_pEvent->GetEvent(&lEventCode,&lParam1,&lParam2,0);

	if(hr == S_OK && lEventCode == EC_COMPLETE)
	{
		Stop();
		return false;
	}

	if( m_fVolumeDecTime )
	{
		LONGLONG llCurrent, llStop;
		hr = m_pSeeking->GetPositions(&llCurrent, &llStop);
		if( hr == S_OK )
		{
			double lfDec = llMediaTime * m_fVolumeDecTime;
			if( llStop - llCurrent < lfDec )
			{
				float fPos = static_cast<float>(llStop - llCurrent) / lfDec;
				if( fPos < 0 )
				{
					fPos = 0;
				}
				CComQIPtr<IBasicAudio>pBA(m_pGraph);
				if( NULL != pBA )
				{
					double lfPower10 = pow(static_cast<double>(10),static_cast<double>(-10));
					double lfVolume = (1-lfPower10)*fPos + lfPower10;
					lfVolume = 10*log10(lfVolume);
					lfVolume *= 100;
					pBA->put_Volume(static_cast<long>(lfVolume));
				}
			}
		}
	}

	if(lEventCode)
	{
		_PgOutputDebugString("DS Event %d\n", lEventCode);
	}

	return true;
}

bool PgDshowPlay::SetMediaWindow(HWND hMediaWindow)
{
	if (!::IsWindow(hMediaWindow)) {
		return false;
	}

	m_hMediaWindow = hMediaWindow;

	return true;
}
bool PgDshowPlay::InitVideoWin(std::string const& strPath, bool const& bFullScreenMode)
{
	HRESULT hr = m_pGraph->QueryInterface(IID_IVideoWindow, (void**)&m_pVideo);
	if (FAILED(hr)) 
	{ 
		_PgOutputDebugString("ERROR - Could not have VideoWindow. "); 
		return false; 
	}

	m_pVideo->put_Owner((OAHWND)m_hMediaWindow); 
	m_pVideo->put_MessageDrain((OAHWND)m_hMediaWindow);
	
	hr = m_pGraph->RenderFile(UNI(strPath.c_str()), NULL);

	if(FAILED(hr))
	{
		return false;
	}

	RECT wndRect;
	GetClientRect(m_hMediaWindow, &wndRect);

	if( bFullScreenMode )
	{
		m_pVideo->put_WindowStyle(WS_CHILD|WS_CLIPSIBLINGS); 
		if(g_pkApp && g_pkApp->GetFullscreen())
		{
			m_pVideo->put_FullScreenMode(TRUE);
		}

		m_pVideo->SetWindowPosition(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
	}
	else
	{	
		m_pVideo->put_WindowStyle(WS_CHILD|WS_CLIPSIBLINGS); 

		long const lHalfViewSize_X = VIEWSIZE_X/2;
		long const lHalfViewSize_Y = VIEWSIZE_Y/2;
		long const lCenter_X = wndRect.right/2;
		long const lCenter_Y = wndRect.bottom/2;
		m_pVideo->SetWindowPosition(lCenter_X-lHalfViewSize_X, lCenter_Y-lHalfViewSize_Y, VIEWSIZE_X, VIEWSIZE_Y);
	}

	m_hVideoWindow =::FindWindow(NULL, L"ActiveMovie Window");
	if(!m_hVideoWindow)
	{
		return false;
	}

	::SetParent(m_hVideoWindow, m_hMediaWindow);
	InvalidateRect(m_hMediaWindow, NULL, true);
	m_pVideo->put_Visible(OATRUE);	//이게 위에 있으면 먼저 한번 뜨고 위치이동해서 잔상이 남아서 옮김
	ShowWindow(m_hVideoWindow, SW_SHOWNORMAL);
	UpdateWindow(m_hVideoWindow);

	return true;
}
void PgDshowPlay::ReleaseVideoWin()
{
	if(m_pVideo)
	{
		//m_pVideo->put_Owner(NULL); 
		//m_pVideo->put_MessageDrain(NULL);
		m_pVideo->put_Visible(OAFALSE);
		m_pVideo->Release();
		m_pVideo = NULL;
	}

	if(m_hVideoWindow)
	{
		ShowWindow(m_hVideoWindow, HIDE_WINDOW);
		UpdateWindow(m_hVideoWindow);
	//	::SetParent(m_hVideoWindow, NULL);
	//	SendMessage(m_hVideoWindow, WM_CLOSE, 0, 0);
		//m_hVideoWindow = NULL;

	//	//BOOL bdis = DestroyWindow(m_hVideoWindow);
	//	//m_hVideoWindow = NULL;
	}
}