#include "stdafx.h"
//#include "Variant/PgJobSkillLocationItem.h"
#include "PgPOTParticle.h"
#include "PgActor.h"
#include "lwActor.h"
#include "PgItemMan.h"
#include "PgParticleMan.h"
#include "lwQuaternion.h"

PgPOTParticle::PgPOTParticle()
{
}

PgPOTParticle::~PgPOTParticle()
{
}

void PgPOTParticle::AddInfo(SPOTParticleInfo const& kInfo)
{// 시점 파티클 정보를 추가한다
	if(kInfo.iAttachPointOfTime & E_APOT_MELEE_ATK)
	{// 근접 공격 액션이 시작할때 붙는때
		m_kContInfo.insert(std::make_pair(E_APOT_MELEE_ATK, kInfo));
	}
	if(kInfo.iAttachPointOfTime & E_APOT_PROJELCTILE_ATK)
	{// 발사체에 붙어야 할 때
		m_kContInfo.insert(std::make_pair(E_APOT_PROJELCTILE_ATK, kInfo));
	}
	if(kInfo.iAttachPointOfTime & E_APOT_EVENT_HITKEY)
	{// 애니메이션에 특정 event key가 있을경우
		m_kContInfo.insert(std::make_pair(E_APOT_EVENT_HITKEY, kInfo));
	}
}

void PgPOTParticle::RemoveInfo(EAttachPointOfTime ePOT)
{// 제거한다
	if(ePOT & E_APOT_MELEE_ATK)
	{// 근접 공격 액션이 시작할때 붙는때
		m_kContInfo.erase(static_cast<int>(E_APOT_MELEE_ATK));
	}
	if(ePOT & E_APOT_PROJELCTILE_ATK)
	{// 발사체에 붙어야 할 때
		m_kContInfo.erase(static_cast<int>(E_APOT_PROJELCTILE_ATK));
	}
	if(ePOT & E_APOT_EVENT_HITKEY)
	{// 애니메이션에 특정 event key가 있을경우
		m_kContInfo.erase(static_cast<int>(E_APOT_EVENT_HITKEY));
	}
}

void PgPOTParticle::ClearAll()
{
	m_kContInfo.clear();
}

bool PgPOTParticle::GetInfo(EAttachPointOfTime ePOT, SPOTParticleInfo& kResult) const
{// 파티클 정보를 얻는다
	
	CONT_SETITEM_POT_PARTICLE_INFO::const_iterator kItor =  m_kContInfo.find(static_cast<int>(ePOT));
	if(m_kContInfo.end() != kItor)
	{
		kResult = kItor->second;
		return true;
	}
	return false;
}

namespace PgPOTParticleUtile
{
	int const iPOTIndex = 60000; // 파티클이 사용할 Actor의 Slot

	bool AttachParticle(PgPOTParticle& kPOTParticle, PgPOTParticle::EAttachPointOfTime ePOT, PgActor& kActor,std::string const kEventKey)
	{// 어떤시점에 Actor에 파티클을 붙이고
		SPOTParticleInfo kTempInfo;
		if(kPOTParticle.GetInfo(ePOT, kTempInfo))
		{// 여기서 한번더 정보를 얻어와 파티클을 붙인다면 어러개를 붙일수 있음
			lwActor kTempActor(&kActor);
			std::string kEffectID(kTempInfo.kEffectID);
			if(PgPOTParticle::E_APOT_PROJELCTILE_ATK & ePOT)
			{
				kEffectID = kTempInfo.kProjectileEffectID;
			}

			if(kEventKey.empty()
				|| kTempInfo.kEventKey == kEventKey
				)
			{
				if(kTempInfo.kAttachNode.empty())
				{// 노드 위치에 붙이기
					lwPoint3 kPos =  kTempActor.GetNodeWorldPos( kTempInfo.kAttachNodePos.c_str() );
					kTempActor.AttachParticleToPointWithRotate(iPOTIndex , kPos, kEffectID.c_str(), kTempActor.GetRotateQuaternion(), kTempInfo.fScale);
				}
				else
				{// 노드에 붙이기
					kTempActor.AttachParticleS(iPOTIndex , kTempInfo.kAttachNode.c_str(), kEffectID.c_str(), kTempInfo.fScale);
				}
			}
			else
			{
				return false;
			}
			return true;
		}
		return false;
	}
	
	bool AttachParticle(PgPOTParticle& kPOTParticle, PgPOTParticle::EAttachPointOfTime ePOT, NiNodePtr& spNif)
	{// 어떤시점에 어떤노드에 파티클을 붙이고(발사체용으로 만듬)
		SPOTParticleInfo kTempInfo;
		if(kPOTParticle.GetInfo(ePOT, kTempInfo))
		{// 여기서 한번더 정보를 얻어와 파티클을 붙인다면 어러개를 붙일수 있음
			std::string kEffectID(kTempInfo.kEffectID);
			if(PgPOTParticle::E_APOT_PROJELCTILE_ATK & ePOT)
			{
				kEffectID = kTempInfo.kProjectileEffectID;
			}
			NiAVObject *pkParticle = g_kParticleMan.GetParticle(kEffectID.c_str(), PgParticle::O_SCALE, kTempInfo.fScale);
			if(pkParticle)
			{
				spNif->AttachChild(pkParticle);
				return true;
			}
		}
		return false;
	}

	bool DetachParticle(PgPOTParticle& kPOTParticle, PgPOTParticle::EAttachPointOfTime ePOT, PgActor& kActor)
	{// 뗀다
		//kPOTParticle.RemoveInfo(ePOT);
		kActor.DetachFrom(iPOTIndex, true);
		return true;
	}

	void FinishUpWhenActionEnd(PgPOTParticle& kPOTParticle, PgActor& kActor)
	{
		for(int i=static_cast<int>(PgPOTParticle::E_APOT_MELEE_ATK); 
			static_cast<int>(PgPOTParticle::E_APOT_MAX) > i; i*=2)
		{
			SPOTParticleInfo kTempInfo;
			if(kPOTParticle.GetInfo(static_cast<PgPOTParticle::EAttachPointOfTime>(i), kTempInfo))
			{
				if(kTempInfo.bDetachWhenActionEnd)
				{
					//kPOTParticle.RemoveInfo(ePOT);
					kActor.DetachFrom(iPOTIndex, true);
				}
			}
		}
	}
};