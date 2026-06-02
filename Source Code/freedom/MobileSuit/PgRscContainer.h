#ifndef FREEDOM_DRAGONICA_PGRSCCONTAINER_H
#define FREEDOM_DRAGONICA_PGRSCCONTAINER_H
#include "BM/PgTask.h"
#include "BM/Observer.h"

class PgRscContainer;

#pragma pack(1)
typedef struct tagRscMessage
	: public BM::Stream
{
	tagRscMessage()
	{
		Set(_T(""),false);
		m_pkSubject = NULL;
	}

	void Set(std::wstring const &rkRscName, bool const bIsImmidiate = false)
	{
		IsImmidiate(bIsImmidiate);
		RscName(rkRscName);
	}

	CLASS_DECLARATION_S(bool, IsImmidiate);
	CLASS_DECLARATION_S(std::wstring, RscName);
	
	PgRscContainer* m_pkSubject;

	bool operator < (const tagRscMessage &rhs)const
	{//사실 아래 애들은 필요없지;
		if( RscName() < rhs.RscName() )	{return true;}
		if( RscName() > rhs.RscName() )	{return false;}
		return false;
	}
}SRscMessage;
#pragma pack()

class PgRscContainer
	:	public BM::CSubject<tagRscMessage>
{
protected:
	typedef tagRscMessage T_MSG;
	//Stage 
	// 1 -> 로딩화면.
	// 2 -> 로그인 화면.
	// 3 -> 기타 등등.
	// 4 -> 기타 등등...... 
	// .
	// .
public:
	PgRscContainer();
	virtual ~PgRscContainer();

public:
	HRESULT RegistLoadRsc(int const iStage, const T_MSG &kRscMsg)
	{
		std::pair<CONT_STAGE::iterator, bool> ret = m_kContStage.insert(std::make_pair(iStage, CONT_STAGE::mapped_type()));
		
		CONT_STAGE::iterator stage_itor = ret.first;
		
		(*stage_itor).second.push_back(kRscMsg);

		return S_OK;
	}

	HRESULT LoadStage(int const iStage)
	{
		BM::CAutoMutex kLock(m_kMutex);

		CONT_STAGE::const_iterator stage_itor = m_kContStage.find(iStage);
		
		if(stage_itor != m_kContStage.end())
		{
			const CONT_STAGE::mapped_type& rElement = (*stage_itor).second;
			CONT_STAGE::mapped_type::const_iterator path_itor = rElement.begin();
			while(path_itor != rElement.end())
			{
				VNotify(*path_itor);
				++path_itor;
			}
		}
		return S_OK;
	}

	void* GetRsc(std::wstring const &rkResourcePath)const//이거 포인터 빼주면 이후 책임은 못짐.
	{
		BM::CAutoMutex kLock(m_kMutex);
		
		SRscMessage kMsg;
		kMsg.RscName(rkResourcePath);

		CONT_LOADED_RSC::const_iterator loaded_itor = m_kContLoadedRsc.find(kMsg);
		if(loaded_itor != m_kContLoadedRsc.end())
		{
			return (*loaded_itor).second;
		}
		return false;
	}

	virtual HRESULT LoadResource(T_MSG &rkMsg, void *&pkOutRsc)const = 0;
//	{
//		if(GetRsc(rkResourcePath))
//		{
//			return E_FAIL;
//		}
//
//		return S_OK;
//	}

	virtual HRESULT UnloadResource(void *&pkOutRsc)const
	{
		return S_OK;
	}

	virtual void LoadComplete(const T_MSG &rkMsg, void *pkOutRsc)
	{
		BM::CAutoMutex kLock(m_kMutex);
		std::pair<CONT_LOADED_RSC::iterator, bool> ret = m_kContLoadedRsc.insert(std::make_pair(rkMsg, pkOutRsc));
		
		if(ret.second)
		{//스레드니까 행여 두번 로딩 하게 될 수 있음. 
			UnloadResource(pkOutRsc);
		}
	}

protected:
	typedef std::list< T_MSG > CONT_RSC_KEY;//ID / Path
	typedef std::map<int, CONT_RSC_KEY> CONT_STAGE;
	CONT_STAGE m_kContStage;

	typedef std::map<T_MSG, void*> CONT_LOADED_RSC;
	CONT_LOADED_RSC m_kContLoadedRsc;

	mutable Loki::Mutex m_kMutex;
};

#endif // FREEDOM_DRAGONICA_PGRSCCONTAINER_H