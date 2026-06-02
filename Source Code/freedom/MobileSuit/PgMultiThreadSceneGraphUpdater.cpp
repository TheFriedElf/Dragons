#include "stdafx.h"
#include "PgMultiThreadSceneGraphUpdater.H"

///////////////////////////////////////////////////////////////////////////////////////////
//	class	PgMultiThreadSceneGraphUpdateProcedure
///////////////////////////////////////////////////////////////////////////////////////////
PgMultiThreadSceneGraphUpdateProcedure::PgMultiThreadSceneGraphUpdateProcedure(int iThreadIndex)
:m_fAccumTime(0),m_fFrameTime(0),m_iThreadIndex(iThreadIndex)
{
}


bool PgMultiThreadSceneGraphUpdateProcedure::LoopedProcedure(void* pvArg)
{

	while(m_kObjectQueue.IsEmpty() == false)
	{
		NiAVObject	*pkObject = NiDynamicCast(NiAVObject,m_kObjectQueue.GetHead());
		m_kObjectQueue.RemoveHead();
		
		pkObject->Update(m_fAccumTime);
	}

	return	true;
}

void	PgMultiThreadSceneGraphUpdateProcedure::AddObject(NiAVObject* pkObject)
{
	m_kObjectQueue.AddTail(pkObject);
}

///////////////////////////////////////////////////////////////////////////////////////////
//	class	PgMultiThreadSceneGraphUpdater
///////////////////////////////////////////////////////////////////////////////////////////
NiImplementRTTI(PgMultiThreadSceneGraphUpdater, NiObject);

PgMultiThreadSceneGraphUpdater::PgMultiThreadSceneGraphUpdater()
:m_iThreadIndex(0)
{
	Init();
}
PgMultiThreadSceneGraphUpdater::~PgMultiThreadSceneGraphUpdater()
{
	Terminate();
}

void	PgMultiThreadSceneGraphUpdater::UpdateObject(NiAVObject *pkAVObject)
{
	if(!pkAVObject)
	{
		return;
	}
	if(m_kThreadCont.size() == 0)
	{
		pkAVObject->Update(m_fAccumTime);
		return;
	}

	NiThread	*pkThread = m_kThreadCont[m_iThreadIndex];


	PgMultiThreadSceneGraphUpdateProcedure	*pkProcedure = 
		(PgMultiThreadSceneGraphUpdateProcedure*)pkThread->GetProcedure();


	pkProcedure->AddObject(pkAVObject);
	

	++m_iThreadIndex;
	if(m_iThreadIndex>=m_kThreadCont.size())
	{
		m_iThreadIndex = 0;
	}
}

bool	PgMultiThreadSceneGraphUpdater::WaitFinish()
{
	for(int i=0;i<m_kThreadCont.size(); ++i)
	{
		NiLoopedThread	*pkThread = (NiLoopedThread*)m_kThreadCont[i];
		PgMultiThreadSceneGraphUpdateProcedure	*pkProcedure = 
			(PgMultiThreadSceneGraphUpdateProcedure*)pkThread->GetProcedure();

		pkThread->DoLoop();
	}
	for(int i=0;i<m_kThreadCont.size(); ++i)
	{
		NiLoopedThread	*pkThread = (NiLoopedThread*)m_kThreadCont[i];
		PgMultiThreadSceneGraphUpdateProcedure	*pkProcedure = 
			(PgMultiThreadSceneGraphUpdateProcedure*)pkThread->GetProcedure();

		pkThread->WaitForLoopCompletion();
	}
	return	true;
}

void	PgMultiThreadSceneGraphUpdater::Init()
{
	int	iMaxThread = NiSystemDesc::GetSystemDesc().GetLogicalProcessorCount();
	if(iMaxThread == 1 || false == g_bUseMTSGU)
	{
		return;
	}
	for(int i=0;i<iMaxThread;++i)
	{
		PgMultiThreadSceneGraphUpdateProcedure	*pkProcedure = NiNew PgMultiThreadSceneGraphUpdateProcedure(i);
		NiLoopedThread	*pkThread = NiLoopedThread::Create(pkProcedure);
		pkThread->Resume();
		m_kThreadCont.push_back(pkThread);
	}
}
void	PgMultiThreadSceneGraphUpdater::Terminate()
{
	WaitFinish();
	for(int i=0;i<m_kThreadCont.size();++i)
	{
		NiThread	*pkThread = m_kThreadCont[i];
		NiThreadProcedure	*pkProcedure = pkThread->GetProcedure();
		SAFE_DELETE_NI(pkProcedure);
		SAFE_DELETE_NI(pkThread);

	}

	m_kThreadCont.clear();
}