#ifndef FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTSYSTEM_H
#define FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTSYSTEM_H

#include "PgEventScriptInstance.H"

typedef std::list<PgEventScriptInstance*> EventScriptInstanceCont;
typedef std::set< int > ContReserveEraseEventScript;


namespace PgEventScriptSystemFunction
{
	inline void SafeDelete(PgEventScriptInstance* &pkPointer)
	{
		if( pkPointer )
		{
			delete pkPointer;
			pkPointer = NULL;
		}
	}

	inline void SendActivateEventScript(int const iEventID);
	inline void SendDeActivateEventScript(int const iEventID);
};


class	PgEventScriptSystem
{
private:
	friend class SActionReserveToDelete;
	friend class SActionReserveToDeleteEventID;


	//
	struct SActionReserveToDelete
	{
		inline explicit SActionReserveToDelete(PgEventScriptSystem *pkEventScriptSystem)
			:m_pkEventScriptSystem(pkEventScriptSystem)
		{}

		inline void operator () (PgEventScriptInstance* pkEventInstance)
		{
			if( pkEventInstance )
			{
				m_pkEventScriptSystem->ReserveToDelete(pkEventInstance);
			}
		}
	private:
		PgEventScriptSystem *m_pkEventScriptSystem;
	};


	//
	struct SActionOperatorEqual
	{
		inline SActionOperatorEqual(int const iEventID)
			:m_iEventID(iEventID)
		{
		}

		inline bool operator () (PgEventScriptInstance* &pkPointer) const
		{
			if( !pkPointer )
			{
				return false;
			}
			if( pkPointer->GetEventID() == m_iEventID )
			{
				return true;
			}
			return false;
		}
	private:
		int m_iEventID;
	};


	//
	struct SActionReserveToDeleteEventID
	{
		explicit SActionReserveToDeleteEventID(PgEventScriptSystem *pkEventScriptSystem, int const iEventID)
			:m_pkEventScriptSystem(pkEventScriptSystem), m_iEventID(iEventID)
		{}

		bool operator () (PgEventScriptInstance* &pkEventInstance)
		{
			if( SActionOperatorEqual(m_iEventID)(pkEventInstance) )
			{
				(SActionReserveToDelete(m_pkEventScriptSystem))(pkEventInstance);
				return true;
			}
			return false;
		}
	private:
		PgEventScriptSystem *m_pkEventScriptSystem;
		int const m_iEventID;
	};


	//
	struct SActionOperatorEqualAndDelete
	{
		SActionOperatorEqualAndDelete(int const iEventID)
			:m_iEventID(iEventID)
		{
		}

		bool operator () (PgEventScriptInstance* &pkPointer) const
		{
			if( SActionOperatorEqual(m_iEventID)(pkPointer) )
			{
				PgEventScriptSystemFunction::SafeDelete(pkPointer);
				return true;
			}
			return false;
		}
	private:
		int m_iEventID;
	};


public:
	void	Init();
	void	Terminate();
	void	Update(float fAccumTime,float fFrameTime);
	bool	ActivateEvent(int iEventID, float fDelayTime);
	bool	ActivateEvent(int iEventID);	//	이미 실행중인 이벤트를 다시 실행하는것은 안된다.
	bool	DeactivateEvent(int iEventID);
	bool	DeactivateAll();
	bool	IsNowUpdate()	const	{	return	m_bNowUpdate;	}
	void	SetNowUpdate(bool bUpdate)	{	m_bNowUpdate = bUpdate;	}

public:
	void	ReleaseAll();
	bool	IsNowActivated(int iEventID);
	bool	IsNowActivate();

private:
	void	ReserveToDelete(PgEventScriptInstance *pkInstance);

private:
	EventScriptInstanceCont	m_kProcessing;	//	현재 실행중인 것들
	EventScriptInstanceCont	m_kAddToProcessingReserved;	//	추가 예약
	ContReserveEraseEventScript	m_kDeleteFromProcessingReserved;	//	삭제 예약
	ContReserveEraseEventScript m_kDelayActivateEvent;	// 지연된 실행 예약
	bool	m_bNowUpdate;	//	현재 업데이트 중인가?
};

extern	PgEventScriptSystem	g_kEventScriptSystem;

#endif // FREEDOM_DRAGONICA_CONTENTS_EVENTSCRIPTSYSTEM_PGEVENTSCRIPTSYSTEM_H