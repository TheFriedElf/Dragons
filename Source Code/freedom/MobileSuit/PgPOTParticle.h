#ifndef FREEDOM_DRAGONICA_CONTENTS_POINTOFTIMEPARTICLE_PGPOTPPARTICLE_H
#define FREEDOM_DRAGONICA_CONTENTS_POINTOFTIMEPARTICLE_PGPOTPPARTICLE_H

#include "PgItemMan.h"
#include "NiNode.H"

class PgActor;
//struct SPOTParticleInfo;

class PgPOTParticle
{
public:
	typedef enum eAttachPointOfTime
	{
		E_APOT_NONE				= 0x00000000,
		E_APOT_MELEE_ATK		= 0x00000001,
		E_APOT_PROJELCTILE_ATK	= 0x00000002,
		E_APOT_EVENT_HITKEY		= 0x00000004,
		E_APOT_MAX
	}EAttachPointOfTime;

public:
	PgPOTParticle();
	~PgPOTParticle();
	
	// 시점 파티클 정보를 추가하고
	void AddInfo(SPOTParticleInfo const& kInfo);
	// 제거한다
	void RemoveInfo(EAttachPointOfTime ePOT);
	void ClearAll();

	// 파티클 정보를 얻는다
	bool GetInfo(EAttachPointOfTime ePOT, SPOTParticleInfo& kResult) const;
private:
	CONT_SETITEM_POT_PARTICLE_INFO m_kContInfo;
};

namespace PgPOTParticleUtile
{
	// 어떤시점에 Actor에 파티클을 붙이고
	bool AttachParticle(PgPOTParticle& kPOTParticle, PgPOTParticle::EAttachPointOfTime ePOT, PgActor& kActor, std::string const kEventKey=std::string());
	bool AttachParticle(PgPOTParticle& kPOTParticle, PgPOTParticle::EAttachPointOfTime ePOT, NiNodePtr& spNif);
	// 뗀다
	bool DetachParticle(PgPOTParticle& kPOTParticle, PgPOTParticle::EAttachPointOfTime ePOT, PgActor& kActor);
	// 액션 종료후에 마무리
	void FinishUpWhenActionEnd(PgPOTParticle& kPOTParticle, PgActor& kActor);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_POINTOFTIMEPARTICLE_PGPOTPPARTICLE_H