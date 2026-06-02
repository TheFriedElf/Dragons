#include <stdafx.h>
#include "PgMobileSuit.h"
#include "PgScripting.h"
#include "PgEventTimer.h"
#include "PgRenderMan.h"
#include "PgWorld.h"

BM::TObjectPool< STimerEvent > g_kTimerEventPool(10, 5);

/////////////////////////////////////////////////////////////

tagTimerEvent::tagTimerEvent()
	:m_kKey("", "")
{
	Clear();
}

void tagTimerEvent::Clear()
{
	m_kKey.Clear();
	fInterval = 0.f;
	iInitCount = 0;
	kScript.clear();

	fTargetTime = 0.f;
	iRemainCount = 0;
	bPause = false;
}

void tagTimerEvent::Set(float const fTime, char const* szEventScript, int const iRunCount)
{
	Clear();
	fInterval = fTime;

	//fTargetTime = fTrgTime;
	kScript = szEventScript;
	iRemainCount = iInitCount = iRunCount;
}

bool tagTimerEvent::Empty() const
{
	if( 0.0f == fInterval
	||	!iRemainCount
	||	!iInitCount
	||	kScript.empty() )
	{
		return true;
	}
	return false;
}

void tagTimerEvent::Start(float const fAccumTime)
{
	fTargetTime = fAccumTime + fInterval;
}

void tagTimerEvent::Reset(float const fAccumTime, int const iNewCount)
{
	Start(fAccumTime);
	if( iNewCount )
	{
		iInitCount = iRemainCount = iNewCount;
	}
}

bool tagTimerEvent::RunAble(float const fAccumTime)
{
	if( Pause() )
	{
		return false;
	}
	return fAccumTime >= fTargetTime;
}

bool tagTimerEvent::RunScript(float const fAccumTime)
{
	bool const bRet = PgScripting::DoBuffer(kScript.c_str(), kScript.size());

	--iRemainCount;
	if( !bRet
	||	0 >= iRemainCount )
	{
		return true;//삭제
	}
	fTargetTime = fAccumTime + fInterval;//재설정
	return false;//삭제 하지 마라
}

void tagTimerEvent::Pause(float const fAccumTime)
{
	if( !bPause )
	{
		fTargetTime = fAccumTime - fTargetTime;//남은 시간 임시 기억
		bPause = true;
	}
}
void tagTimerEvent::Resume(float const fAccumTime)
{
	if( bPause )
	{
		fTargetTime = fAccumTime + fTargetTime;//남은 시간 다시 시작
		bPause = false;
	}
}

bool tagTimerEvent::Pause() const
{
	return bPause;
}

bool const tagTimerEvent::operator < (const tagTimerEvent& rhs) const
{
	return fTargetTime < rhs.fTargetTime;
}

////////////////////////////////////////////////////////////


PgEventTimer::PgEventTimer()
{
	m_kTimerEvent.clear();
	m_kDelReserve.clear();
}

PgEventTimer::~PgEventTimer()
{
}

float PgEventTimer::UpdatedTime()const
{
	return g_pkApp->GetAccumTime();
}

void PgEventTimer::Clear()
{
	BM::CAutoMutex kLock(m_kMutex);

	ContTimerEvent::iterator iter = m_kTimerEvent.begin();
	while(m_kTimerEvent.end() != iter)
	{
		Del(iter, iter);//모두 삭제
	}

	m_kDelReserve.clear();
}

void PgEventTimer::Update(float const fAccumTime, float const fFrameTime)
{
	BM::CAutoMutex kLock(m_kMutex);

	ContTimerEvent::iterator iter = m_kTimerEvent.begin();
	while(m_kTimerEvent.end() != iter)
	{
		ContTimerEvent::mapped_type pkElement = (*iter).second;
		if( pkElement->RunAble(fAccumTime) )
		{
			bool bEraseEvent = false;
			bool const bRet = RunEvent(fAccumTime, pkElement, bEraseEvent);
			if( bEraseEvent )//이벤트가 삭제
			{
				Del(iter, iter);
				continue;
			}
		}
		++iter;
	}

	if( !m_kDelReserve.empty() )//Public: Del(char const*, char const*)는 실제 삭제는 안한다.
	{
		ContTimerEventKey::iterator del_iter = m_kDelReserve.begin();
		while(m_kDelReserve.end() != del_iter)
		{
			const ContTimerEventKey::value_type& rkKey = (*del_iter);
			Del(rkKey);
			++del_iter;
		}
		m_kDelReserve.clear();
	}

	//ContTimerEventList::iterator iter = m_kTimerEventList.begin();
	//while(m_kTimerEventList.end() != iter)
	//{
	//	ContTimerEventList::value_type pkElement = (*iter);
	//	if( pkElement->RunAble(fAccumTime) )
	//	{
	//		bool bEraseEvent = false;
	//		bool const bRet = RunEvent(fAccumTime, pkElement, bEraseEvent);
	//		if( bEraseEvent )//이벤트가 삭제
	//		{
	//			Del(iter, iter);
	//			continue;
	//		}
	//	}
	//	++iter;
	//}
}

bool PgEventTimer::AddLocal(char const* szEventID ,const STimerEvent& rkTimerEvent, bool const bImmediateRun)
{
	BM::CAutoMutex kLock(m_kMutex);
	if(	szEventID)
	{
		std::string kScendID;
		bool const bFind = g_kRenderMan.GetFirstTypeID<PgWorld>(kScendID);
		if( bFind )
		{
			STimerEventKey kKey(kScendID.c_str(), szEventID);
			return Add(kKey, rkTimerEvent, bImmediateRun);
		}
	}
	return false;
}

bool PgEventTimer::AddGlobal(char const* szEventID ,const STimerEvent& rkTimerEvent, bool const bImmediateRun)
{
	BM::CAutoMutex kLock(m_kMutex);
	if(	szEventID )
	{
		STimerEventKey kKey("", szEventID);
		return Add(kKey, rkTimerEvent, bImmediateRun);
	}
	return false;
}

bool PgEventTimer::Del(char const* szSceneID, char const* szEventID)
{
	BM::CAutoMutex kLock(m_kMutex);
	if(	szSceneID
	&&	szEventID )
	{
		STimerEventKey kKey(szSceneID, szEventID);
		ContTimerEvent::iterator iter;
		if( Get(kKey, iter) )
		{
			bool const bDelRet = Del(iter, iter, true);//삭제 예약만 한다.
			return bDelRet;
		}
	}
	return false;
}

bool PgEventTimer::Pause(char const* szSceneID, char const* szEventID)
{
	BM::CAutoMutex kLock(m_kMutex);
	STimerEvent* pkTimerEvent = NULL;
	if(	Get(szSceneID, szEventID, pkTimerEvent) )
	{
		pkTimerEvent->Pause(UpdatedTime());
		return true;
	}
	return false;
}

bool PgEventTimer::Resume(char const* szSceneID, char const* szEventID)
{
	BM::CAutoMutex kLock(m_kMutex);
	STimerEvent* pkTimerEvent = NULL;
	if(	Get(szSceneID, szEventID, pkTimerEvent) )
	{
		pkTimerEvent->Resume(UpdatedTime());
		return true;
	}
	return false;
}

bool PgEventTimer::Reset(char const* szSceneID, char const* szEventID, int const iNewCount)
{
	BM::CAutoMutex kLock(m_kMutex);
	STimerEvent* pkTimerEvent = NULL;
	if(	Get(szSceneID, szEventID, pkTimerEvent) )
	{
		pkTimerEvent->Reset(UpdatedTime(), iNewCount);
		return true;
	}
	return false;
}

void PgEventTimer::ChangeScene(char const* szNewSceneID)
{
	if( !szNewSceneID )
	{
		return;
	}

	BM::CAutoMutex kLock(m_kMutex);
	std::string const kNewSceneID = szNewSceneID;

	ContTimerEvent::iterator iter = m_kTimerEvent.begin();
	while(m_kTimerEvent.end() != iter)
	{
		const ContTimerEvent::key_type& rkKey = (*iter).first;
		const ContTimerEvent::mapped_type pkElement = (*iter).second;
		if( !rkKey.kSceneID.empty()
		&&	szNewSceneID != rkKey.kSceneID )//새로운 scend과 틀리면
		{
			Del(iter, iter);//삭제
		}
		else
		{
			++iter;
		}
	}
}

bool PgEventTimer::IsHaveLocal(char const* szEventID)
{
	if(	szEventID )
	{
		std::string kScendID;
		bool const bFind = g_kRenderMan.GetFirstTypeID<PgWorld>(kScendID);
		if( bFind )
		{
			STimerEventKey kKey(kScendID.c_str(), szEventID);
			ContTimerEvent::iterator iter = m_kTimerEvent.find(kKey);
			if(m_kTimerEvent.end() != iter)
			{
				return true;
			}
		}
	}
	return false;
}

bool PgEventTimer::IsHaveGlobal(char const* szEventID)
{
	if( szEventID )
	{
		STimerEventKey kKey("", szEventID);
		ContTimerEvent::iterator iter = m_kTimerEvent.find(kKey);
		if(m_kTimerEvent.end() != iter)
		{
			return true;
		}
	}
	return false;
}

bool PgEventTimer::RightNowLocal(char const* szEventID)
{
	if(	!szEventID ){ return false; }
	
	std::string kScendID;
	bool const bFind = g_kRenderMan.GetFirstTypeID<PgWorld>(kScendID);
	if( !bFind ){ return false; }
	
	STimerEventKey kKey(kScendID.c_str(), szEventID);
	ContTimerEvent::iterator iter;
	if( Get(kKey, iter) )
	{
		ContTimerEvent::mapped_type pkElement = (*iter).second;
		if( pkElement )
		{
			bool bEraseRet = false;
			bool const bRet = RunEvent(UpdatedTime(), pkElement, bEraseRet);
			if( bEraseRet )
			{
				Del(kKey);
			}
			return bRet;
		}
	}
	return true;
}

bool PgEventTimer::RightNowGlobal(char const* szEventID)
{
	if(	szEventID ){ return false; }
	
	STimerEventKey kKey("", szEventID);
	ContTimerEvent::iterator iter;
	if( Get(kKey, iter) )
	{
		ContTimerEvent::mapped_type pkElement = (*iter).second;
		if( pkElement )
		{
			bool bEraseRet = false;
			bool const bRet = RunEvent(UpdatedTime(), pkElement, bEraseRet);
			if( bEraseRet )
			{
				Del(kKey);
			}
			return bRet;
		}
	}
	return true;
}

bool PgEventTimer::Get(char const* szSceneID, char const* szEventID, STimerEvent*& pkOut)
{
	if(	szSceneID
	&&	szEventID )
	{
		STimerEventKey kKey(szSceneID, szEventID);
		ContTimerEvent::iterator iter;
		if( Get(kKey, iter) )
		{
			ContTimerEvent::mapped_type pkElement = (*iter).second;
			if( pkElement )
			{
				pkOut = pkElement;
				return true;
			}
		}
	}
	return false;
}

bool PgEventTimer::Get(const STimerEventKey& rkKey, ContTimerEvent::iterator& iter_out)
{
	ContTimerEvent::iterator iter = m_kTimerEvent.find(rkKey);
	if(m_kTimerEvent.end() != iter)
	{
		iter_out = iter;
		return true;
	}
	return false;
}

bool PgEventTimer::Add(const STimerEventKey& rkKey, const STimerEvent& rkTimerEvent, bool const bImmediateRun)
{
	if( rkKey.kEventID.empty()//
	||	rkTimerEvent.Empty() )
	{
		return false;
	}
	
	ContTimerEvent::mapped_type pkNewEvent = g_kTimerEventPool.New();
	if( !pkNewEvent )
	{
		return false;
	}

	*pkNewEvent = rkTimerEvent;//Copy
	pkNewEvent->Start(UpdatedTime());
	pkNewEvent->Key(rkKey);

	std::pair<ContTimerEvent::iterator, bool> kRet = m_kTimerEvent.insert( std::make_pair(rkKey, pkNewEvent) );
	if( !kRet.second )//중복되면 업데이트
	{
		ContTimerEvent::mapped_type pkElement = (*kRet.first).second;

		(*kRet.first).second = pkNewEvent; // 새로운 걸로 교체

		g_kTimerEventPool.Delete(pkElement);//기존것 삭제
	}

	//ContTimerEventList::iterator list_iter = m_kTimerEventList.insert(m_kTimerEventList.end(), pkNewEvent);//추가
	//m_kTimerEventList.sort();//정렬 남은 시간대비

	if( bImmediateRun )//추가 되지 마자 삭제 될수도
	{
		bool bEraseRet = false;
		bool const bRet = RunEvent(UpdatedTime(), pkNewEvent, bEraseRet);
		if( bEraseRet )
		{
			//Del(list_iter, list_iter);
			Del(kRet.first, kRet.first);
		}
	}
	return true;
}

bool PgEventTimer::RunEvent(float const fAccumTime, ContTimerEvent::mapped_type pkTimerEvent, bool& bEraseOut) const
{
	if( pkTimerEvent
	&&	!pkTimerEvent->Pause() )
	{
		bEraseOut = pkTimerEvent->RunScript(fAccumTime);
		return true;//실행 성공
	}
	return false;//실패
}

//bool PgEventTimer::Del(const ContTimerEventList::iterator iter, ContTimerEventList::iterator& iter_out)
//{
//	if( m_kTimerEventList.end() != iter )
//	{
//		const STimerEventKey& rkKey = (*iter)->Key();
//		m_kTimerEventList.erase(iter);
//		ContTimerEvent::iterator map_iter = m_kTimerEvent.end();
//		if( Get(rkKey, map_iter) )
//		{
//			Del(map_iter, map_iter);
//		}
//		return true;
//	}
//	return false;
//}

bool PgEventTimer::Del(const ContTimerEvent::iterator iter, ContTimerEvent::iterator& iter_out, bool const bReserve)
{
	if( m_kTimerEvent.end() == iter )//끝은 삭제 안되
	{
		return false;//실패
	}
	ContTimerEvent::mapped_type pkElement = (*iter).second;

	if( !bReserve )
	{
		iter_out = m_kTimerEvent.erase(iter);

		if( pkElement )
		{
			g_kTimerEventPool.Delete(pkElement);
			return true;// 풀까지 지워야 성공
		}
	}
	else
	{
		const ContTimerEvent::key_type& rkKey = (*iter).first;
		m_kDelReserve.push_back(rkKey);
		return true;
	}
	return false;//실패
}

bool PgEventTimer::Del(const STimerEventKey& rkKey)
{
	ContTimerEvent::iterator iter = m_kTimerEvent.find(rkKey);
	if( m_kTimerEvent.end() != iter )
	{
		return Del(iter, iter);
	}
	return false;
}