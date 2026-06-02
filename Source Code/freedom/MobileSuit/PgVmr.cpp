#include "stdafx.h"
#include "PgMobileSuit.h"
#include "PgVmr.h"

PgVmr::PgVmr() 
 : m_bPlay(false), m_bInit(false), m_spScreenPolygon(NULL), m_pDirect3DSurface(NULL)
{
	InitDefaultValues();
}

PgVmr::PgVmr(HWND hMediaWindow, int iNumberOfStream)
{
	InitDefaultValues();
	
	if(::IsWindow(hMediaWindow))
	{
		m_hMediaWindow = hMediaWindow;
	}

	if(iNumberOfStream > 0 && iNumberOfStream < 11)
	{
		m_iNumberOfStream = iNumberOfStream;
	}
}

PgVmr::~PgVmr()
{
	ReleaseAllInterfaces();
}
void PgVmr::Init()
{
	if(m_bInit)
	{
		return;
	}
	if(IsPlay())
		StopGraph();

	CoInitialize(NULL);
	SetMediaWindow(g_pkApp->GetWindowReference());
	//SetMediaFile("d:/KERORO177.avi", 1);
	//SetLayerZOrder(0, 0);

	m_bInit = true;
}

bool PgVmr::Update()
{
	if(!IsPlay())
	{
		return false;
	}

	long lEventCode = 0;
	long lParam1 = 0;
	long lParam2 = 0;
	HRESULT hr = m_pMediaEvent->GetEvent(&lEventCode,&lParam1,&lParam2,0);

	//HDC hDc = ::GetDC(m_hMediaWindow);
	//m_pVMRWindowlessControl->RepaintVideo(m_hMediaWindow, hDc);

	//char szTmp[100]={0,};
	//sprintf_s(szTmp, 100, "%x, %x, %x, %x\n", lEventCode, lParam1, lParam2, hr);
	//OutputDebugStringA(szTmp);

	// MEDIA SIZE
	LONG  Width;
	LONG  Height;
	LONG  ARWidth;
	LONG  ARHeight;
	m_pVMRWindowlessControl->GetNativeVideoSize(&Width, &Height, &ARWidth, &ARHeight);
	RECT mediaRect;
	mediaRect.left = 0;
	mediaRect.top = 0;
	mediaRect.right = Width;
	mediaRect.bottom = Height;
	RECT wndRect;
	GetClientRect(m_hMediaWindow, &wndRect);
	m_pVMRWindowlessControl->SetVideoPosition(&mediaRect, &wndRect);

	if(hr == S_OK && lEventCode == EC_COMPLETE)
	{
		StopGraph();
		return false;
	}
	
	return true;
}
// Graph configuration
bool PgVmr::SetMediaWindow(HWND hMediaWindow)
{
	if (!::IsWindow(hMediaWindow)) {
		ReportError("The window handle is invalid", E_INVALIDARG);
		return false;
	}

	m_hMediaWindow = hMediaWindow;

	return true;
}
bool PgVmr::SetMediaFile(char const* pszFileName, int iLayer)
{
	USES_CONVERSION;

	HRESULT hr;
	bool	bHaveToRun = false;

	if (pszFileName == NULL) {
		ReportError("Could not load a file with an empty file name", E_INVALIDARG);
		return false;
	}

	// ENSURE that a valid graph builder is available
	if (m_pGraphBuilder == NULL) {
		bool bRet = BuildFilterGraph();
		if (!bRet) return bRet;
	}

	// ENSURE that the filter graph is in a stop state
	OAFilterState filterState;
	m_pMediaControl->GetState(500, &filterState);
	if (filterState != State_Stopped) {
		m_pMediaControl->Stop();
		bHaveToRun = true;
	}

	// CHECK a source filter availaibility for the layer
	if (m_srcFilterArray[iLayer] == NULL) {
		char pszFilterName[10];
		sprintf_s(pszFilterName, "SRC%02d", iLayer);
		IBaseFilter* pBaseFilter = NULL;
		//hr = m_pGraphBuilder->AddSourceFilter(A2W(pszFileName), A2W(pszFilterName), &pBaseFilter);
		hr = m_pGraphBuilder->AddSourceFilter(A2W(pszFileName), L"SOURCE", &pBaseFilter);

		if (FAILED(hr)) {
			ReportError("Could not find a source filter for this file", hr);
			return false;
		}
		m_srcFilterArray[iLayer] = pBaseFilter;
	} else {
		// suppress the old src filter
		IBaseFilter* pBaseFilter = m_srcFilterArray[iLayer];
		RemoveFilterChain(pBaseFilter, m_pVMRBaseFilter);
		pBaseFilter->Release();
		m_srcFilterArray[iLayer] = NULL;
		// create a new src filter
		char pszFilterName[10];
		sprintf_s(pszFilterName, "SRC%02d", iLayer);
		hr = m_pGraphBuilder->AddSourceFilter(A2W(pszFileName), A2W(pszFilterName), &pBaseFilter);
		m_srcFilterArray[iLayer] = pBaseFilter;
		if (FAILED(hr)) {
			m_srcFilterArray[iLayer] = NULL;
			ReportError("Could not load the file", hr);
			return false;
		}
	}

	// RENDER the graph
	bool bRet = RenderGraph();
	if (!bRet) return bRet;

	// RUN the graph if previous state was running
	if (bHaveToRun) {
		m_pMediaControl->Run();
	}

	return true;
}
bool PgVmr::PreserveAspectRatio(bool bPreserve)
{
	if (m_pVMRWindowlessControl == NULL) {
		ReportError("Can't set aspect ratio, no VMR", E_FAIL);
		return false;
	}

	if (bPreserve)
		m_pVMRWindowlessControl->SetAspectRatioMode(VMR9ARMode_LetterBox);
	else
		m_pVMRWindowlessControl->SetAspectRatioMode(VMR9ARMode_None);

	return true;
}
IBaseFilter* PgVmr::AddFilter(char const* pszName, const GUID& clsid)
{
	USES_CONVERSION;

	HRESULT hr;

	IBaseFilter* pBaseFilter = NULL;

	if (pszName == NULL) {
		ReportError("Can't add filter, no valid name", E_INVALIDARG);
		return NULL;
	}

	hr = AddFilterByClsid(m_pGraphBuilder, A2W(pszName), clsid, &pBaseFilter);
	if (FAILED(hr)) {
		ReportError("Can't add filter", hr);
		return NULL;
	}

	return pBaseFilter;
}

// Graph control
bool PgVmr::PlayGraph(std::string strPath)
{
	if(!IsInit())
	{
		ReportError("No Initialize", E_FAIL);
		return false;
	}

	SetMediaFile(strPath.c_str(), 0);
	SetLayerZOrder(0, 0);

	if (m_pMediaControl == NULL) {
		ReportError("Can't play, no graph", E_FAIL);
		return false;
	}
	if (m_pVMRWindowlessControl == NULL) {
		ReportError("Can't play, no VMR", E_FAIL);
		return false;
	}

	// MEDIA SIZE
	LONG  Width;
	LONG  Height;
	LONG  ARWidth;
	LONG  ARHeight;
	m_pVMRWindowlessControl->GetNativeVideoSize(&Width, &Height, &ARWidth, &ARHeight);
	RECT mediaRect;
	mediaRect.left = 0;
	mediaRect.top = 0;
	mediaRect.right = Width;
	mediaRect.bottom = Height;
	RECT wndRect;
	GetClientRect(m_hMediaWindow, &wndRect);
	m_pVMRWindowlessControl->SetVideoPosition(&mediaRect, &wndRect);
	
	HDC hDc = ::GetDC(m_hMediaWindow);
	m_pVMRWindowlessControl->RepaintVideo(m_hMediaWindow, hDc);

	// RUN
	m_pMediaControl->Run();


	m_bPlay = true;
	return true;
}
bool PgVmr::StopGraph()
{
	if(!IsInit())
	{
		ReportError("No Initialize", E_FAIL);
		return false;
	}

	if (m_pMediaControl == NULL) {
		ReportError("Can't stop, no graph", E_FAIL);
		return false;
	}

	m_pMediaControl->Stop();

	IMediaSeeking* pMS = GetPtrMediaSeeking();
	if(pMS)
	{
		LONGLONG pos = 0;
		pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
	}

	m_bPlay = false;
	return true;
}
bool PgVmr::ResetGraph()
{
	// STOP the graph
	if (m_pMediaControl != NULL) {
		m_pMediaControl->Stop();
	}

	try {
		ReleaseAllInterfaces();
	} catch(...) {
		ReportError("Can't reset graph, we have serious bugs...", E_FAIL);
		return false;
	}

	return true;
}
bool PgVmr::GetStateGraph()
{
	if(m_pMediaControl != NULL)
	{
		//m_pMediaControl->GetState();
	}

	return true;
}
IMediaEvent* PgVmr::GetPtrMediaEvent()
{
	if (m_pGraphBuilder == NULL) {
		ReportError("Can't return IMediaEvent, no graph", E_FAIL);
		return NULL;
	}

	if (m_pMediaEvent != NULL) {
		m_pMediaEvent->AddRef();
	}

	return m_pMediaEvent;
}
IMediaControl* PgVmr::GetPtrMediaControl()
{
	if (m_pGraphBuilder == NULL) {
		ReportError("Can't return IMediaControl, no graph", E_FAIL);
		return NULL;
	}

	if (m_pMediaControl != NULL) {
		m_pMediaControl->AddRef();
	}

	return m_pMediaControl;
}
IMediaSeeking* PgVmr::GetPtrMediaSeeking()
{
	IMediaSeeking* pMediaSeeking = NULL;

	if (m_pGraphBuilder == NULL) {
		ReportError("Can't return IMediaSeeking, no graph", E_FAIL);
		return NULL;
	}

	m_pGraphBuilder->QueryInterface(IID_IMediaSeeking, (void**) &pMediaSeeking);

	return pMediaSeeking;
}
IBasicAudio* PgVmr::GetPtrBasicAudio()
{
	IBasicAudio* pBasicAudio = NULL;

	if (m_pGraphBuilder == NULL) {
		ReportError("Can't return IBasicAudio, no graph", E_FAIL);
		return NULL;
	}

	m_pGraphBuilder->QueryInterface(IID_IBasicAudio, (void**) &pBasicAudio);

	return pBasicAudio;
}

// Layer control
bool PgVmr::GetVideoRect(LPRECT pRect)
{
	HRESULT hr;

	if (pRect == NULL) {
		ReportError("Can't return video size, rect struct invalid", E_INVALIDARG);
		return false;
	}

	if (m_pVMRWindowlessControl == NULL) {
		ReportError("Can't return video size, no VMR", E_FAIL);
		return false;
	}

	// MEDIA SIZE
	LONG  Width;
	LONG  Height;
	LONG  ARWidth;
	LONG  ARHeight;
	hr = m_pVMRWindowlessControl->GetNativeVideoSize(&Width, &Height, &ARWidth, &ARHeight);

	if (FAILED(hr)) {
		ReportError("Can't return video size, VMR error", hr);
		return false;
	}

	// RECT structure
	pRect->left		= 0;
	pRect->top		= 0;
	pRect->right	= Width;
	pRect->bottom	= Height;

	return true;
}
int PgVmr::GetAlphaLayer(int iLayer)
{
	HRESULT hr;

	if (!IsValidLayer(iLayer)) {
		ReportError("Can't set alpha, incorect layer", E_INVALIDARG);
		return 1;
	}

	if (m_pVMRMixerControl == NULL) {
		ReportError("Can't set alpha, no VMR", E_FAIL);
		return 1;
	}

	float fAlpha;
	hr = m_pVMRMixerControl->GetAlpha(iLayer, &fAlpha);

	return (int)(fAlpha*100);
}
bool PgVmr::SetAlphaLayer(int iLayer, int iAlpha)
{
	HRESULT hr;

	if (!IsValidLayer(iLayer)) {
		ReportError("Can't set alpha, incorect layer", E_INVALIDARG);
		return false;
	}

	if (iAlpha < 0 || iAlpha > 100) {
		ReportError("Can't set alpha, incorrect alpha value", E_INVALIDARG);
		return false;
	}

	if (m_pVMRMixerControl == NULL) {
		ReportError("Can't set alpha, no VMR", E_FAIL);
		return false;
	}

	float fAlpha = ((float)iAlpha) / 100.0f;
	hr = m_pVMRMixerControl->SetAlpha(iLayer, fAlpha);

	return true;
}
DWORD PgVmr::GetLayerZOrder(int iLayer)
{
	HRESULT hr;
	DWORD	dwZOrder;

	if (!IsValidLayer(iLayer)) {
		ReportError("Can't return order, incorect layer", E_INVALIDARG);
		return false;
	}

	if (m_pVMRMixerControl == NULL) {
		ReportError("Can't return order, no VMR", E_FAIL);
		return false;
	}

	hr = m_pVMRMixerControl->GetZOrder(iLayer, &dwZOrder);

	return dwZOrder;
}
bool PgVmr::SetLayerZOrder(int iLayer, DWORD dwZOrder)
{
	HRESULT hr;

	if (!IsValidLayer(iLayer)) {
		ReportError("Can't set order, incorect layer", E_INVALIDARG);
		return false;
	}

	if (m_pVMRMixerControl == NULL) {
		ReportError("Can't set order, no VMR", E_FAIL);
		return false;
	}

	hr = m_pVMRMixerControl->SetZOrder(iLayer, dwZOrder);
	if (FAILED(hr)) {
		ReportError("Can't set ZOrder", hr);
		return false;
	}

	return true;
}
bool PgVmr::SetLayerRect(int iLayer, RECT layerRect)
{
	HRESULT hr;

	if (!IsValidLayer(iLayer)) {
		ReportError("Can't set layer rect, incorect layer", E_INVALIDARG);
		return false;
	}

	if (m_pVMRMixerControl == NULL) {
		ReportError("Can't set layer rect, no VMR", E_FAIL);
		return false;
	}

	if (m_pVMRWindowlessControl == NULL) {
		ReportError("Can't set layer rect, no VMR", E_FAIL);
		return false;
	}

	// VIDEO size
	VMR9NormalizedRect nRect = NormalizeRect(&layerRect);
	hr = m_pVMRMixerControl->SetOutputRect(iLayer, &nRect);
	if (FAILED(hr)) {
		ReportError("Can't set layer rect", hr);
		return false;
	}

	return true;
}

// Bitmap control
bool PgVmr::SetBitmap(char const* pszBitmapFileName, int iAlpha, COLORREF cTransColor, RECT bitmapRect)
{
	HRESULT hr;

	// PARAMS check
	if (m_pVMRMixerBitmap == NULL) {
		ReportError("Can't set bitmap, no VMR", E_FAIL);
		return false;
	}

	if (m_pVMRWindowlessControl == NULL) {
		ReportError("Can't set bitmap, no VMR", E_FAIL);
		return false;
	}

	if (m_pD3DDevice == NULL) {
		ReportError("Can't set bitmap, no Direct3D device", E_FAIL);
		return false;
	}

	// CLEAN-UP old suface
	if (m_pD3DSurface != NULL) {
		m_pD3DSurface->Release();
		m_pD3DSurface = NULL;
	}

	// CREATE a new surface
	long lWidth, lHeight;
	if ( !GetBitmapSize(pszBitmapFileName, lHeight, lWidth) ) {
		lWidth = 128;
		lHeight = 128;
	}
    hr = m_pD3DDevice->CreateOffscreenPlainSurface(	lWidth, lHeight,
													D3DFMT_X8R8G8B8,
													D3DPOOL_SYSTEMMEM,
													&m_pD3DSurface,
													NULL);
	if (FAILED(hr)) {
		ReportError("Can't create a Direct3d surface", hr);
		return false;
	}

	// LOAD bitmap file
	D3DXLoadSurfaceFromFileA(m_pD3DSurface,
							NULL,					// palette
							NULL,					// entire surface - created to be the proper height
							pszBitmapFileName,
							NULL,					// entire image source
							D3DX_DEFAULT, 
							0,						// disable color key
							NULL					// source info
							);

	// MIXER params
	VMR9NormalizedRect nRect = NormalizeRect(&bitmapRect);
	VMR9AlphaBitmap alphaBitmap;
    alphaBitmap.dwFlags			= VMR9AlphaBitmap_EntireDDS | VMR9AlphaBitmap_SrcColorKey;
    alphaBitmap.hdc				= NULL;
    alphaBitmap.pDDS			= m_pD3DSurface;
    alphaBitmap.clrSrcKey		= cTransColor;
    alphaBitmap.rDest.top		= nRect.top;
    alphaBitmap.rDest.left		= nRect.left;
    alphaBitmap.rDest.bottom	= nRect.bottom;
    alphaBitmap.rDest.right		= nRect.right;
    alphaBitmap.fAlpha			= iAlpha / 100.0f;

	// SHOW the bitmap
	hr = m_pVMRMixerBitmap->SetAlphaBitmap(&alphaBitmap);
	if (FAILED(hr)) {
		ReportError("Can't display the bitmap", hr);
		return false;
	}

	return true;
}
bool PgVmr::SetBitmapParams(int iAlpha, COLORREF cTransColor, RECT bitmapRect)
{
	HRESULT hr;

	// PARAMS check
	if (m_pVMRMixerBitmap == NULL) {
		ReportError("Can't set bitmap, no VMR", E_FAIL);
		return false;
	}

	if (m_pVMRWindowlessControl == NULL) {
		ReportError("Can't set bitmap, no VMR", E_FAIL);
		return false;
	}

	if (m_pD3DDevice == NULL) {
		ReportError("Can't set bitmap, no Direct3D device", E_FAIL);
		return false;
	}

	if (m_pD3DSurface == NULL) {
		ReportError("Can't set bitmap, no Direct3D surface", E_FAIL);
		return false;
	}

	// MIXER params
	VMR9NormalizedRect nRect = NormalizeRect(&bitmapRect);
	VMR9AlphaBitmap alphaBitmap;
    alphaBitmap.dwFlags			= VMR9AlphaBitmap_EntireDDS | VMR9AlphaBitmap_SrcColorKey;
    alphaBitmap.hdc				= NULL;
    alphaBitmap.pDDS			= m_pD3DSurface;
    alphaBitmap.clrSrcKey		= cTransColor;
    alphaBitmap.rDest.top		= nRect.top;
    alphaBitmap.rDest.left		= nRect.left;
    alphaBitmap.rDest.bottom	= nRect.bottom;
    alphaBitmap.rDest.right		= nRect.right;
    alphaBitmap.fAlpha			= iAlpha / 100.0f;

	// SHOW the bitmap
	hr = m_pVMRMixerBitmap->SetAlphaBitmap(&alphaBitmap);
	if (FAILED(hr)) {
		ReportError("Can't display the bitmap", hr);
		return false;
	}

	return true;
}

// Reflected from window
bool PgVmr::Repaint()
{
	if (m_pVMRWindowlessControl == NULL) {
		ReportError("Can't repaint, no VMR", E_FAIL);
		return false;
	}

	HDC hdc = GetDC(m_hMediaWindow);
	m_pVMRWindowlessControl->RepaintVideo(m_hMediaWindow, hdc);

	return true;
}
bool PgVmr::Resize()
{
	if (m_pVMRWindowlessControl == NULL) {
		ReportError("Can't resize, no VMR", E_FAIL);
		return false;
	}

	// MEDIA SIZE
	LONG  Width;
	LONG  Height;
	LONG  ARWidth;
	LONG  ARHeight;
	m_pVMRWindowlessControl->GetNativeVideoSize(&Width, &Height, &ARWidth, &ARHeight);
	RECT mediaRect;
	mediaRect.left = 0;
	mediaRect.top = 0;
	mediaRect.right = Width;
	mediaRect.bottom = Height;
	RECT wndRect;
	GetClientRect(m_hMediaWindow, &wndRect);
	m_pVMRWindowlessControl->SetVideoPosition(&mediaRect, &wndRect);

	return true;
}

// helper
char const* PgVmr::GetLastError()
{
	return (char const*)m_pszErrorDescription;
}


// INIT helper methods
void PgVmr::InitDefaultValues()
{
	//char szTmp[100]={0,};
	//DWORD dwTime = BM::GetTime32();
	//sprintf_s(szTmp, 100, "time %d\n", dwTime);
	//OutputDebugStringA(szTmp);

	ZeroMemory(m_pszErrorDescription, 1024);
	m_dwRotId				= -1;
	m_iNumberOfStream		= 4;		// default VMR9 config
	m_hMediaWindow			= NULL;
	// SRC interfaces
	for (int i=0; i<10; i++) {
		m_srcFilterArray[i] = NULL;
	}
	// SOUND interface
	m_pDirectSoundFilter	= NULL;
	// GRAPH interfaces
	m_pGraphUnknown			= NULL;
	m_pGraphBuilder			= NULL;
	m_pFilterGraph			= NULL;
	m_pFilterGraph2			= NULL;
	m_pMediaControl			= NULL;
	m_pMediaEvent			= NULL;
	m_pMediaEventEx			= NULL;
	// VMR9 interfaces
	m_pVMRBaseFilter		= NULL;
	m_pVMRFilterConfig		= NULL;
	m_pVMRMixerBitmap		= NULL;
	m_pVMRMixerControl		= NULL;
	m_pVMRMonitorConfig		= NULL;
	m_pVMRWindowlessControl	= NULL;
	// DIRECT3D interfaces
	m_pD3DDirect3d			= NULL;
	m_pD3DDevice			= NULL;
	m_pD3DSurface			= NULL;
	
	//sprintf_s(szTmp, 100, "time %d\n", BM::GetTime32()-dwTime);
	//OutputDebugStringA(szTmp);
}
void PgVmr::ReleaseAllInterfaces()
{
	if(IsPlay())
		StopGraph();
	// CALLBACK handle
	if (m_pMediaEventEx != NULL) {
		m_pMediaEventEx->SetNotifyWindow(NULL, WM_MEDIA_NOTIF, NULL);
	}
	// SRC interfaces
	for (int i=0; i<10; i++) {
		IBaseFilter* pSrcFilter = m_srcFilterArray[i];
		if (pSrcFilter != NULL) {
			pSrcFilter->Release();
			m_srcFilterArray[i] = NULL;
		}
	}
	// SOUND interfaces
	if (m_pDirectSoundFilter != NULL) {
		m_pDirectSoundFilter->Release();
		m_pDirectSoundFilter = NULL;
	}
	// VMR9 interfaces
	if (m_pVMRFilterConfig != NULL) {
		m_pVMRFilterConfig->Release();
		m_pVMRFilterConfig = NULL;
	}
	if (m_pVMRMixerBitmap != NULL) {
		m_pVMRMixerBitmap->Release();
		m_pVMRMixerBitmap = NULL;
	}
	if (m_pVMRMixerControl != NULL) {
		m_pVMRMixerControl->Release();
		m_pVMRMixerControl = NULL;
	}
	if (m_pVMRMonitorConfig != NULL) {
		m_pVMRMonitorConfig->Release();
		m_pVMRMonitorConfig = NULL;
	}
	if (m_pVMRWindowlessControl != NULL) {
		m_pVMRWindowlessControl->Release();
		m_pVMRWindowlessControl = NULL;
	}
	if (m_pVMRBaseFilter != NULL) {
		m_pVMRBaseFilter->Release();
		m_pVMRBaseFilter = NULL;
	}
	// GRAPH interfaces
	if (m_pGraphBuilder != NULL) {
		m_pGraphBuilder->Release();
		m_pGraphBuilder = NULL;
	}
	if (m_pFilterGraph != NULL) {
		m_pFilterGraph->Release();
		m_pFilterGraph = NULL;
	}
	if (m_pFilterGraph2 != NULL) {
		m_pFilterGraph2->Release();
		m_pFilterGraph2 = NULL;
	}
	if (m_pMediaControl != NULL) {
		m_pMediaControl->Release();
		m_pMediaControl = NULL;
	}
	if (m_pMediaEvent != NULL) {
		m_pMediaEvent->Release();
		m_pMediaEvent = NULL;
	}
	if (m_pMediaEventEx != NULL) {
		m_pMediaEventEx->Release();
		m_pMediaEventEx = NULL;
	}
	if (m_pGraphUnknown != NULL) {
		m_pGraphUnknown->Release();
		m_pGraphUnknown = NULL;
	}
	// DIRECT3D interfaces
	if (m_pD3DSurface != NULL) {
		m_pD3DSurface->Release();
		m_pD3DSurface = NULL;
	}
	if (m_pD3DDevice != NULL) {
		m_pD3DDevice->Release();
		m_pD3DDevice = NULL;
	}
	if (m_pD3DDirect3d != NULL) {
		m_pD3DDirect3d->Release();
		m_pD3DDirect3d = NULL;
	}

	m_bInit = false;
}

// GRAPH methods
bool PgVmr::BuildFilterGraph()
{
	HRESULT hr;

	ReleaseAllInterfaces();
	RemoveFromRot();
	
	// BUILD the filter graph
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IUnknown, (void**) &m_pGraphUnknown);
	if (FAILED(hr)) {
		ReportError("Could not build the graph", hr);
		return false;
	}
	// QUERY the filter graph interfaces
	hr = m_pGraphUnknown->QueryInterface(IID_IGraphBuilder, (void**) &m_pGraphBuilder);
	hr = m_pGraphUnknown->QueryInterface(IID_IFilterGraph, (void**) &m_pFilterGraph);
	hr = m_pGraphUnknown->QueryInterface(IID_IFilterGraph2, (void**) &m_pFilterGraph2);
	hr = m_pGraphUnknown->QueryInterface(IID_IMediaControl, (void**) & m_pMediaControl);
	hr = m_pGraphUnknown->QueryInterface(IID_IMediaEvent, (void**) &m_pMediaEvent);
	hr = m_pGraphUnknown->QueryInterface(IID_IMediaEventEx, (void**) &m_pMediaEventEx);

	// SET the graph state window callback
	if (m_pMediaEventEx != NULL) {
		m_pMediaEventEx->SetNotifyWindow((OAHWND)m_hMediaWindow, WM_MEDIA_NOTIF, NULL);
	}

	BuildSoundRenderer();

// Don't known what's wrong... but RenderEx crash when playing whith graphedit build 021204 ...
	AddToRot(m_pGraphUnknown);

	m_bInit = true;
	return BuildVMR();
}
bool PgVmr::BuildVMR()
{
	HRESULT hr;

	if (m_hMediaWindow == NULL) {
		ReportError("Could not operate without a Window", E_FAIL);
		return false;
	}

	if (m_pGraphBuilder == NULL) {
		ReportError("Could not build the VMR, the graph isn't valid", E_FAIL);
		return false;
	}

	// BUILD the VMR9
	IBaseFilter *pVmr = NULL;
	hr = CoCreateInstance(CLSID_VideoMixingRenderer9, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &m_pVMRBaseFilter);
	if (FAILED(hr)) {
		ReportError("Could not create an instance ofthe VMR9", hr);
		return false;
	}

	// ADD the VMR9 to the graph
	hr = m_pGraphBuilder->AddFilter(m_pVMRBaseFilter, L"VMR9");
	if (FAILED(hr)) {
		ReportError("Could not add the VMR9 to the Graph", hr);
		return false;
	}

	// DIRECT3D
	//bool bD3D = BuildDirect3d();

	// QUERY the VMR9 interfaces
	hr = m_pVMRBaseFilter->QueryInterface(IID_IVMRFilterConfig9, (void**) &m_pVMRFilterConfig);
	if (SUCCEEDED(hr)) {
		// CONFIGURE the VMR9
		//m_pVMRFilterConfig->SetRenderingMode(VMR9Mode_Windowless);
		m_pVMRFilterConfig->SetRenderingMode(VMR9Mode_Windowless);
		m_pVMRFilterConfig->SetNumberOfStreams(m_iNumberOfStream);
	}

	hr = m_pVMRBaseFilter->QueryInterface(IID_IVMRWindowlessControl9, (void**) &m_pVMRWindowlessControl);
	if (SUCCEEDED(hr)) {
		// CONFIGURE the VMR9
		m_pVMRWindowlessControl->SetVideoClippingWindow(m_hMediaWindow);
		m_pVMRWindowlessControl->SetAspectRatioMode(VMR9ARMode_None);
		m_pVMRWindowlessControl->DisplayModeChanged();
	}

	//hr = m_pVMRBaseFilter->QueryInterface(IID_IVMRMixerBitmap9, (void**) &m_pVMRMixerBitmap);
	hr = m_pVMRBaseFilter->QueryInterface(IID_IVMRMixerControl9, (void**) &m_pVMRMixerControl);
	//hr = m_pVMRBaseFilter->QueryInterface(IID_IVMRMonitorConfig9, (void**) &m_pVMRMonitorConfig);

	return true;
}
bool PgVmr::BuildSoundRenderer()
{
	HRESULT hr;

	hr = AddFilterByClsid(m_pGraphBuilder, L"DirectSound", CLSID_DSoundRender, &m_pDirectSoundFilter);
	if (FAILED(hr)) {
		ReportError("Could not add the DirectSoundRenderer", hr);
		return false;
	}
	return true;
}
bool PgVmr::RenderGraph()
{
	HRESULT hr;

	if (m_pFilterGraph2 == NULL) {
		ReportError("Could not render the graph because it is not fully constructed", E_FAIL);
		return false;
	}

	for (int i=0; i<10; i++) {
		IBaseFilter* pBaseFilter = m_srcFilterArray[i];
		if (pBaseFilter != NULL) {
			IPin* pPin = GetPin(pBaseFilter, PINDIR_OUTPUT);
			if (pPin != NULL) {
				try {
				hr = m_pFilterGraph2->RenderEx(pPin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL);
				} catch(...) {
				}
			}
		}
	}
	return true;
}

// DIRECT3D methods
bool PgVmr::BuildDirect3d()
{
	HRESULT hr;

	m_pD3DDirect3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pD3DDirect3d == NULL) {
		ReportError("Cannot initialize Direct3D", E_FAIL);
		return false;
	}

    D3DDISPLAYMODE d3ddm;
    hr = m_pD3DDirect3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
	if (FAILED(hr)) {
		ReportError("Cannot get display adaptater infos from Direct3d", hr);
		return false;
	}

    D3DPRESENT_PARAMETERS d3dpp; 
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed         = true;
    d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = d3ddm.Format;
    d3dpp.hDeviceWindow    = m_hMediaWindow;

    hr = m_pD3DDirect3d->CreateDevice(	D3DADAPTER_DEFAULT, // always the primary display adapter
										D3DDEVTYPE_HAL,
										NULL,
										D3DCREATE_SOFTWARE_VERTEXPROCESSING,
										&d3dpp,
										&m_pD3DDevice);
	if (FAILED(hr)) {
		ReportError("Cannot create the Direct3d device", hr);
	}

	return true;
}
bool PgVmr::GetBitmapSize(char const* pszBitmapFileName,long& lHeight, long& lWidth)
{
    bool ret = false;

	// LOAD Bitmap file
    HANDLE hBitmap = LoadImageA(	NULL,
								pszBitmapFileName,
								IMAGE_BITMAP,
								0,0,
								LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADFROMFILE);

	// GET size
    if( hBitmap ) {
        BITMAP bmpInfo;
        if(::GetObject(hBitmap, sizeof(bmpInfo), &bmpInfo)) {
            lHeight = bmpInfo.bmHeight;
            lWidth  = bmpInfo.bmWidth;
            ret = true;
        }    
    }

	// CLEAN image resource
    DeleteObject(hBitmap);
    return ret;
}

// LAYER helper methods
bool PgVmr::IsValidLayer(int iLayer)
{
	if (iLayer > 9 || iLayer < 0) return false;
	
	IBaseFilter* pBaseFilter = m_srcFilterArray[iLayer];
	if (pBaseFilter == NULL) 
		return false;
	else
		return true;
}
VMR9NormalizedRect PgVmr::NormalizeRect(LPRECT pRect)
{
	VMR9NormalizedRect nRect;
	nRect.top = nRect.left = nRect.bottom = nRect.right = 0.0f;

	// VIDEO size
	RECT videoRect;
	if (GetVideoRect(&videoRect)) {
		int iWidth	= videoRect.right - videoRect.left;
		int iHeight	= videoRect.bottom - videoRect.top;
		// NORMALIZEDRECT
		try {
			nRect.left = ( (float) ( (pRect->left * 100) / iWidth ) ) / 100.0f;
		} catch(...) {
			nRect.left = 0.0f;
		}
		try {
			nRect.right = ( (float) ( (pRect->right * 100) / videoRect.right ) ) / 100.0f;
		} catch(...) {
			nRect.right = 1.0f;
		}
		try {
			nRect.top = ( (float) ( (pRect->top * 100) / iHeight ) ) / 100.0f;
		} catch(...) {
			nRect.top = 0.0f;
		}
		try {
			nRect.bottom = ( (float) ( (pRect->bottom * 100) / videoRect.bottom ) ) / 100.0f;
		} catch(...) {
			nRect.bottom = 1.0f;
		}
	}

	return nRect;
}

// DSOW helper methods
HRESULT PgVmr::AddToRot(IUnknown *pUnkGraph) 
{
	if (pUnkGraph == NULL) {
		return E_INVALIDARG;
	}

    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }
    WCHAR wsz[256];
    swprintf_s(wsz, 256, _T("FilterGraph %08x pid %08x"), (DWORD_PTR)pUnkGraph, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) {
        hr = pROT->Register(0, pUnkGraph, pMoniker, &m_dwRotId);
        pMoniker->Release();
    }
    pROT->Release();

    return hr;
}
void PgVmr::RemoveFromRot()
{
	if (m_dwRotId != -1) {
		IRunningObjectTable *pROT;
		if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
			pROT->Revoke(m_dwRotId);
			m_dwRotId = -1;
			pROT->Release();
		}
	}
}
IPin* PgVmr::GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir)
{
    bool       bFound = false;
    IEnumPins  *pEnum;
    IPin       *pPin;

    pFilter->EnumPins(&pEnum);
    while(pEnum->Next(1, &pPin, 0) == S_OK) {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        if (bFound = (PinDir == PinDirThis))
            break;
        pPin->Release();
    }
    pEnum->Release();

    return (bFound ? pPin : 0);
}
void PgVmr::ReportError(char const* pszError, HRESULT hrCode)
{
	TCHAR szErr[MAX_ERROR_TEXT_LEN];
	DWORD res = AMGetErrorText(hrCode, szErr, MAX_ERROR_TEXT_LEN);
	if (res != 0) {
 		sprintf_s(m_pszErrorDescription, "[ERROR in %s, line %d] %s : COM Error 0x%x, %s", __FILE__, __LINE__, pszError, hrCode, szErr);
	} else {
		sprintf_s(m_pszErrorDescription, "[ERROR in %s, line %d] %s : COM Error 0x%x", __FILE__, __LINE__, pszError, hrCode);
	}
}
HRESULT PgVmr::GetNextFilter(IBaseFilter *pFilter, PIN_DIRECTION Dir, IBaseFilter **ppNext)
{
    if (!pFilter || !ppNext) return E_POINTER;

    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)) return hr;
    while (S_OK == pEnum->Next(1, &pPin, 0)) {
        // See if this pin matches the specified direction.
        PIN_DIRECTION ThisPinDir;
        hr = pPin->QueryDirection(&ThisPinDir);
        if (FAILED(hr)) {
            // Something strange happened.
            hr = E_UNEXPECTED;
            pPin->Release();
            break;
        }
        if (ThisPinDir == Dir) {
            // Check if the pin is connected to another pin.
            IPin *pPinNext = 0;
            hr = pPin->ConnectedTo(&pPinNext);
            if (SUCCEEDED(hr))
            {
                // Get the filter that owns that pin.
                PIN_INFO PinInfo;
                hr = pPinNext->QueryPinInfo(&PinInfo);
                pPinNext->Release();
                pPin->Release();
                pEnum->Release();
                if (FAILED(hr) || (PinInfo.pFilter == NULL))
                {
                    // Something strange happened.
                    return E_UNEXPECTED;
                }
                // This is the filter we're looking for.
                *ppNext = PinInfo.pFilter; // Client must release.
                return S_OK;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching filter.
    return E_FAIL;
}
bool PgVmr::RemoveFilterChain(IBaseFilter* pFilter, IBaseFilter* pStopFilter)
{
	HRESULT hr;

	IBaseFilter* pFilterFound = NULL;
	
	hr = GetNextFilter(pFilter, PINDIR_OUTPUT, &pFilterFound);
	if (SUCCEEDED(hr) && pFilterFound != pStopFilter) {
		RemoveFilterChain(pFilterFound, pStopFilter);
		pFilterFound->Release();
	}

	m_pFilterGraph->RemoveFilter(pFilter);

	return true;
}
HRESULT PgVmr::AddFilterByClsid(IGraphBuilder *pGraph, LPCWSTR wszName, const GUID& clsid, IBaseFilter **ppF)
{
    *ppF = NULL;
    HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)ppF);
    if (SUCCEEDED(hr))
    {
        hr = pGraph->AddFilter((*ppF), wszName);
    }
    return hr;
}

//void PgVmr::CreateTexture()
//{
//	m_spScreenPolygon = CreateScreenPolygon("blue.bmp", 0.25f);
//	g_pkApp->GetScreenElements()->AddFirstEmpty(m_spScreenPolygon);
//
//	NiPropertyState* pkPropState = m_spScreenPolygon->GetPropertyState();
//    if (!pkPropState)
//        return;
//    NiTexturingProperty* pkTexProp = pkPropState->GetTexturing();
//    if (!pkTexProp)
//        return;
//    //pkTexProp->SetBaseTexture((NiTexture*)pkVideoTexture);
//	LPDIRECT3DTEXTURE9 texture;
//	texture->GetSurfaceLevel(0, &m_pDirect3DSurface);
//}
//
//NiScreenElements* PgVmr::CreateScreenPolygon(char const* pcImageFile,
//                                                    float fScale)
//{
//    if ((fScale < 0.05f) || (fScale > 0.9f))    // Sanity check on scale.
//        return NULL;
//
//    NiScreenElements* pkPoly = NiNew NiScreenElements(
//        NiNew NiScreenElementsData(false, false, 1));
//
//    // We know that the polygon handle is zero and will use it directly in the
//    // vertex and texture coordinate assignments.
//    pkPoly->Insert(4);
//    // Create polygon near lower right corner.
//    pkPoly->SetRectangle(0, (0.9f - fScale), (0.9f - fScale), fScale, fScale);
//    pkPoly->UpdateBound();
//    pkPoly->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);
//
//    // Texture for screen space polygon.
//    NiTexturingProperty* pkTexProp = 
//        NiNew NiTexturingProperty(pcImageFile);
//    pkTexProp->SetApplyMode(NiTexturingProperty::APPLY_REPLACE);
//    pkTexProp->SetBaseFilterMode(NiTexturingProperty::FILTER_BILERP);
//    pkTexProp->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
//    pkPoly->AttachProperty(pkTexProp);
//
//    // Don't test the z-buffer for screen polygons.
//    NiZBufferProperty* pkZProp = NiNew NiZBufferProperty;
//    pkZProp->SetZBufferTest(false);
//    pkZProp->SetZBufferWrite(false);
//    pkPoly->AttachProperty(pkZProp);
//
//    pkPoly->UpdateProperties();
//    pkPoly->Update(0.0f);
//
//    return pkPoly;
//}