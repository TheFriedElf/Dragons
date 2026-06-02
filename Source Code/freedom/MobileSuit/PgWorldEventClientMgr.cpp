#include "stdafx.h"
#include "Variant/PgWorldEvent.h"
#include "PgWorldEventClientMgr.h"
#include "PgTrigger.H"
#include "PgActor.H"
#include "PgNetwork.h"

namespace WorldEventClientMgrUtil
{
	void Net_PT_C_M_CHECK_WORLDEVENT_CONDITION(PgActor* pkActor, WORD const wEventID)
	{
		NiPoint3 const kNiPos(pkActor->GetPos());
		POINT3 const kPos(kNiPos.x, kNiPos.y, kNiPos.z);

		BM::Stream kPacket(PT_C_M_CHECK_WORLDEVENT_CONDITION);
		kPacket.Push( wEventID );
		kPacket.Push( kPos );
		NETWORK_SEND(kPacket);
	}
};

PgWorldEventClientMgr::PgWorldEventClientMgr()
	: PgWorldEventBaseMgr()
{
}

PgWorldEventClientMgr::~PgWorldEventClientMgr()
{
}

void PgWorldEventClientMgr::SyncFromServer(BM::Stream& rkPacket)
{
	ClearWorldEvent();

	size_t iCount = 0;
	rkPacket.Pop( iCount );

	while( 0 < iCount )
	{
		PgWorldEventState kNewState;
		kNewState.ReadFromPacket( rkPacket );

		AddWorldEvent( kNewState );

		--iCount;
	}
}

void PgWorldEventClientMgr::AddWorldEvent(PgWorldEventState const& rkState)
{
	ContWorldEventState::iterator find_iter = m_kEventState.find( rkState.ID() );
	if( m_kEventState.end() == find_iter )
	{
		std::pair<ContWorldEventState::iterator, bool> kRet = m_kEventState.insert( std::make_pair(rkState.ID(), rkState) );
		//if( kRet.second )
		//{
		//}
	}
	else
	{
		(*find_iter).second = rkState;
	}
}
void	PgWorldEventClientMgr::OnPhysXTrigger(PgTrigger *pkTrigger,PgActor *pkActor,WorldEventCondition::EConditionType kConditionType)
{
	if(!pkTrigger)
	{
		return;
	}

	if(!pkActor)
	{
		return;
	}

	for(ContWorldEvent::iterator itor = m_kEvent.begin(); itor != m_kEvent.end(); ++itor)
	{
		PgWorldEvent	&kEvent = itor->second;

		//	We skip a disabled event
		ContWorldEventState::iterator state_itor = m_kEventState.find(kEvent.ID());
		if(state_itor != m_kEventState.end())
		{
			PgWorldEventState const &kState = state_itor->second;
			if(kState.Enable() == false)
			{
				continue;
			}
		}
	
		ContWorldEventCondition	const &kAndCondition = kEvent.AndCondition();
		ContWorldEventCondition	const &kORCondition = kEvent.OrCondition();

		for(ContWorldEventCondition::const_iterator itor= kAndCondition.begin(); itor != kAndCondition.end(); ++itor)
		{
			SWorldEventCondition	const	&kCondition = *itor;
			if(kCondition.ConditionType() != kConditionType)
			{
				continue;
			}

			switch(kCondition.TargetType())
			{
			case	WorldEventTarget::WETT_Trigger:
				{
					BM::vstring const &kTriggerID = kCondition.TargetID();
					if(MB(kTriggerID) == pkTrigger->GetID())
					{
						WorldEventClientMgrUtil::Net_PT_C_M_CHECK_WORLDEVENT_CONDITION(pkActor,kEvent.ID());
					}
				}
				break;
			}
		}
		for(ContWorldEventCondition::const_iterator itor= kORCondition.begin(); itor != kORCondition.end(); ++itor)
		{
			SWorldEventCondition	const	&kCondition = *itor;
			if(kCondition.ConditionType() != kConditionType)
			{
				continue;
			}

			switch(kCondition.TargetType())
			{
			case	WorldEventTarget::WETT_Trigger:
				{
					BM::vstring const &kTriggerID = kCondition.TargetID();
					if(MB(kTriggerID) == pkTrigger->GetID())
					{
						WorldEventClientMgrUtil::Net_PT_C_M_CHECK_WORLDEVENT_CONDITION(pkActor,kEvent.ID());
					}
				}
				break;
			}
		}
	}
}

void PgWorldEventClientMgr::ClearWorldEvent()
{
	m_kEventState.clear();
}

void PgWorldEventClientMgr::ClearParsedWorldEvent()
{
	PgWorldEventBaseMgr::m_kEvent.clear();
}

bool PgWorldEventClientMgr::ParseWorldEvent(TiXmlElement const* pkRoot)
{
	return PgWorldEventBaseMgr::ParseWorldEvent(pkRoot);
}

bool PgWorldEventClientMgr::GetWorldEvent(WORD const& rkWorldEventID, PgWorldEvent const* &pkOut) const
{
	return PgWorldEventBaseMgr::GetWorldEvent(rkWorldEventID, pkOut);
}