#ifndef FREEDOM_DRAGONICA_MOVIE_PGAXPLAY_H
#define FREEDOM_DRAGONICA_MOVIE_PGAXPLAY_H
//#include <atlbase.h>
//#include <atlcom.h>
//#include <atlwin.h>
#ifdef UNICODE

#ifdef GetClassName
#undef GetClassName
#endif
#define GetClassName GetClassNameW
#endif

#include <atlhost.h>
#include <exdispid.h> 

#define SINKID_COUNTEREVENTS 0 

class ATL_NO_VTABLE PgIESink : 
	public CComObjectRootEx<CComSingleThreadModel>, 
	public IDispEventImpl<SINKID_COUNTEREVENTS, PgIESink , &DIID_DWebBrowserEvents2> 
{ 
private: 
	CComPtr<IUnknown> m_pUnkIE; 
	CComQIPtr<IWebBrowser2> m_pWB2; 

public: 
	PgIESink () {} 

	HRESULT AdviseToIE(CComPtr<IUnknown> pUnkIE, CComQIPtr<IWebBrowser2> pWB2) 
	{ 
		m_pUnkIE = pUnkIE; 
		m_pWB2 = pWB2; 

		AtlGetObjectSourceInterface(pUnkIE, &m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum); 

		HRESULT hr = this->DispEventAdvise(pUnkIE, &m_iid); 
		
		return hr; 
	} 

	BEGIN_COM_MAP(PgIESink ) 
		COM_INTERFACE_ENTRY_IID(DIID_DWebBrowserEvents2, PgIESink ) 
	END_COM_MAP() 

	BEGIN_SINK_MAP(PgIESink ) 
		SINK_ENTRY_EX(SINKID_COUNTEREVENTS, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete) 
	END_SINK_MAP() 

	void _stdcall OnDocumentComplete(IDispatch** ppDisp, VARIANT *pvUrl);
}; 


class PgAXPlay
{
public:
	PgAXPlay();
	~PgAXPlay();

	HWND m_hWndIE; 
private:
	CComQIPtr<IWebBrowser2> m_pWB2; 
	
	CComObject<PgIESink> * m_pSink; 
	CComPtr<IUnknown> m_pUnkIE;

	bool m_bPlay;
	bool m_bInit;

public:
	bool Init();
	bool CreateWB();
	void Terminate();
	bool Play(std::string strPath);
	bool Stop();
	bool Update();

	bool IsPlay() { return m_bPlay; }
	bool IsInit() { return m_bInit; }
};

#endif // FREEDOM_DRAGONICA_MOVIE_PGAXPLAY_H