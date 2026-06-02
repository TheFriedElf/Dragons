#ifndef FREEDOM_DRAGONICA_MOVIE_PGVMR_H
#define FREEDOM_DRAGONICA_MOVIE_PGVMR_H

#include <dshow.h>
#include <D3d9.h>
#include <Vmr9.h>
#include <d3dx9tex.h>

#pragma comment( lib, "strmiids.lib" )
#pragma comment( lib, "Quartz.lib" )
#pragma comment( lib, "d3d9.lib" )

#define WM_MEDIA_NOTIF		WM_APP + 1

class PgVmr  
{
public:
	PgVmr();
	PgVmr(HWND hMediaWindow, int iNumberOfStream = 4);
	virtual ~PgVmr();

	// Methods
public:
	void Init();
	bool Update();
	// Graph configuration
	//void SetNumberOfLayer(int iNumberOfLayer);
	bool SetMediaWindow(HWND MediaWindow);
	bool SetMediaFile(char const* pszFileName, int iLayer = 0);
	bool PreserveAspectRatio(bool bPreserve = TRUE);
	IBaseFilter* AddFilter(char const* pszName, const GUID& clsid);

	// Graph control
	bool PlayGraph(std::string strPath);
	bool StopGraph();
	bool ResetGraph();
	bool GetStateGraph();
	IMediaEvent* GetPtrMediaEvent();
	IMediaControl* GetPtrMediaControl();
	IMediaSeeking* GetPtrMediaSeeking();
	IBasicAudio* GetPtrBasicAudio();

	// Layer control
	bool GetVideoRect(LPRECT pRect);
	int GetAlphaLayer(int iLayer);
	bool SetAlphaLayer(int iLayer, int iAlpha);
	DWORD GetLayerZOrder(int iLayer);
	bool SetLayerZOrder(int iLayer, DWORD dwZOrder);
	bool SetLayerRect(int iLayer, RECT layerRect);

	// Bitmap control
	bool SetBitmap(char const* pszBitmapFileName, int iAlpha, COLORREF cTransColor, RECT bitmapRect);
	bool SetBitmapParams(int iAlpha, COLORREF cTransColor, RECT bitmapRect);

	// Reflected from window
	bool Repaint();
	bool Resize();

	// helper
	char const* GetLastError();
	void ReleaseAllInterfaces();

protected:
	// INIT helper methods
	void InitDefaultValues();

	// GRAPH methods
	bool BuildFilterGraph();
	bool BuildVMR();
	bool BuildSoundRenderer();
	bool RenderGraph();

	// DIRECT3D methods
	bool BuildDirect3d();
	bool GetBitmapSize(char const* pszBitmapFileName,long& lHeight, long& lWidth);


	// LAYER helper methods
	bool IsValidLayer(int iLayer);
	VMR9NormalizedRect NormalizeRect(LPRECT pRect);

	// DSOW helper methods
	HRESULT AddToRot(IUnknown *pUnkGraph);
	void RemoveFromRot();
	IPin* GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir);
	void ReportError(char const* pszError, HRESULT hrCode);
	HRESULT GetNextFilter(IBaseFilter *pFilter, PIN_DIRECTION Dir, IBaseFilter **ppNext);
	bool RemoveFilterChain(IBaseFilter* pFilter, IBaseFilter* pStopFilter);
	HRESULT AddFilterByClsid(IGraphBuilder *pGraph, LPCWSTR wszName, const GUID& clsid, IBaseFilter **ppF);

protected:
	DWORD						m_dwRotId;
	char						m_pszErrorDescription[1024+MAX_ERROR_TEXT_LEN];
	int							m_iNumberOfStream;
	// MEDIA WINDOW
	HWND						m_hMediaWindow;
	// SRC interfaces array
	IBaseFilter*				m_srcFilterArray[10];
	// SOUND interfaces
	IBaseFilter*				m_pDirectSoundFilter;
	// GRAPH interfaces
	IUnknown*					m_pGraphUnknown;
	IGraphBuilder*				m_pGraphBuilder;
	IFilterGraph*				m_pFilterGraph;
	IFilterGraph2*				m_pFilterGraph2;
	IMediaControl*				m_pMediaControl;
	IMediaEvent*				m_pMediaEvent;
	IMediaEventEx*				m_pMediaEventEx;
	// VMR9 interfaces
	IBaseFilter*				m_pVMRBaseFilter;
	IVMRFilterConfig9*			m_pVMRFilterConfig;
	IVMRMixerBitmap9*			m_pVMRMixerBitmap;
	IVMRMixerControl9*			m_pVMRMixerControl;
	IVMRMonitorConfig9*			m_pVMRMonitorConfig;
	IVMRWindowlessControl9*		m_pVMRWindowlessControl;
	// DIRECT3D interfaces
	IDirect3D9*					m_pD3DDirect3d;
	IDirect3DDevice9*			m_pD3DDevice;
	IDirect3DSurface9*			m_pD3DSurface;

	bool m_bPlay;
	bool m_bInit;
public:
	bool IsPlay(){ return m_bPlay; }
	bool IsInit(){ return m_bInit; }
	IDirect3DSurface9* m_pDirect3DSurface;

	NiScreenElementsPtr m_spScreenPolygon;
	void CreateTexture();
	NiScreenElements* CreateScreenPolygon(char const* pcImageFile, float fScale);
};

#endif // FREEDOM_DRAGONICA_MOVIE_PGVMR_H