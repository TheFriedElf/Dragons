#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_MULTITHREADSCENEGRAPHUPDATE_PGMULTITHREADSCENEGRAPHUPDATE_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_MULTITHREADSCENEGRAPHUPDATE_PGMULTITHREADSCENEGRAPHUPDATE_H
#include <NiMain.H>


class	PgMultiThreadSceneGraphUpdateProcedure	:	public	NiLoopedThreadProcedure
{

public:

	PgMultiThreadSceneGraphUpdateProcedure(int iThreadIndex);

	bool	LoopedProcedure(void* pvArg);

	void	AddObject(NiAVObject* pkObject);

	void	SetTime(float fAccumTime,float fFrameTime)	
	{
		m_fAccumTime = fAccumTime;
		m_fFrameTime = fFrameTime;
	}

private:

	int	m_iThreadIndex;
	NiObjectList	m_kObjectQueue;

	float	m_fAccumTime,m_fFrameTime;

};

class	PgMultiThreadSceneGraphUpdater	:	public	NiObject
{
    NiDeclareRTTI;


public:

	PgMultiThreadSceneGraphUpdater();
	virtual	~PgMultiThreadSceneGraphUpdater();


public:
	void	SetTime(float fAccumTime,float fFrameTime)	
	{
		for(ThreadVec::size_type i = 0; i < m_kThreadCont.size(); ++i)
		{
			NiThread	*pkThread = m_kThreadCont[i];
			PgMultiThreadSceneGraphUpdateProcedure	*pkProcedure = 
				(PgMultiThreadSceneGraphUpdateProcedure*)pkThread->GetProcedure();

			pkProcedure->SetTime(fAccumTime,fFrameTime);
		}

		m_fAccumTime = fAccumTime;
		m_fFrameTime = fFrameTime;
	}

	void	UpdateObject(NiAVObject *pkAVObject);

	bool	WaitFinish();

private:

	void	Init();
	void	Terminate();

private:

	typedef	std::vector<NiThread*>	ThreadVec;
	ThreadVec	m_kThreadCont;

	float	m_fAccumTime,m_fFrameTime;

	int	m_iThreadIndex;
	NiSemaphore	m_kSemaphore;


};
NiSmartPointer(PgMultiThreadSceneGraphUpdater);

#endif // FREEDOM_DRAGONICA_RENDER_RENDERER_MULTITHREADSCENEGRAPHUPDATE_PGMULTITHREADSCENEGRAPHUPDATE_H