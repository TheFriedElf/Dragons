#ifndef FREEDOM_DRAGONICA_MOVIE_PGDSHOWPLAY_H
#define FREEDOM_DRAGONICA_MOVIE_PGDSHOWPLAY_H

#include <dshow.h> 
#include <stdio.h> 

#include <mmstream.h> 
#include <amstream.h> 
#include <ddstream.h> 

class PgDshowPlay
{
public:
	PgDshowPlay();
	virtual ~PgDshowPlay();
private:
	bool m_bInit;
	bool m_bPlay;

	HWND m_hMediaWindow;

	HWND m_hVideoWindow;

	IGraphBuilder *m_pGraph;
	IMediaControl *m_pControl;
	IMediaEvent *m_pEvent;
	IVideoWindow *m_pVideo;
	IMediaSeeking *m_pSeeking;

	float m_fVolumeDecTime;

public:
	bool IsInit() { return m_bInit; }
	bool IsPlay() { return m_bPlay; }

	bool Init();
	void Terminate();
	bool InitVideoWin(std::string const& strPath, bool const& bFullScreenMode);
	void ReleaseVideoWin();
	bool Play(std::string const& strPath, float fStartPos = -1, float fEndPos = -1, float fVolumeDecTime = 0, bool const& bFullScreenMode = true);
	bool Stop();
	bool Update();

	bool SetMediaWindow(HWND hMediaWindow);
};

#endif // FREEDOM_DRAGONICA_MOVIE_PGDSHOWPLAY_H