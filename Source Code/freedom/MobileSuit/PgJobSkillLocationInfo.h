#ifndef FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_PGJOBSKILLLOCATIONINFO_H
#define FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_PGJOBSKILLLOCATIONINFO_H

class PgJobSkillLocationItem;

typedef enum eJobSkillTriggerLookAtType
{
	EJSTLT_NONE					= 0,		// LookAt 없음
	EJSTLT_LOOKAT_TRIGGER_POS	= 1,		// 트리거 위치를 바라보기
	EJSTLT_LOOKAT_TRIGGER_ID	= 2,		// 지정한 트리거 위치를 바라보기
	EJSTLT_LOOKAT_CAMERA		= 3,		// 카메라 방향쪽으로 바라보기
}EJobSkillTriggerLookAtType;

class PgJobSkillLocationInfo
{// 서버에서 보내준 채집 지역 정보를 클라이언트에서 활용하기 위해 담아두고 관리하는 클래스
private:
	typedef std::map<std::string, int> CONT_SPOT_PARTICLE_SLOT;

public:
	PgJobSkillLocationInfo();
	~PgJobSkillLocationInfo();

	void ProcessMsg(BM::Stream::DEF_STREAM_TYPE const wPacketType, BM::Stream& rkPacket);
	void ReadFromPacket(BM::Stream& rkPacket);

	void RefrashAllSpotRenderObj();
	
	void Clear();

	bool GetGatherType(std::string const& kTriggerID, EGatherType& eOutValue) const;
	bool IsEnable(std::string const& kTriggerID) const;
private:
	void RefrashSpotParticle(std::string const& kTriggerID, SJobSkill_LocationItem& kInfo);
	void AttachSpotParticle(PgTrigger *pkTrigger, std::string const kParticleName);
	void DetachSpotParticle(PgTrigger *pkTrigger);
	
	void SetActorLookAtByJobSkillTrigger(std::string const& kTriggerID, PgActor* pkActor);
private:
	PgJobSkillLocationItem m_kLocationItem;
	CONT_SPOT_PARTICLE_SLOT m_kContSpotSlot;	//월드 객체에 Spot 지역에 붙인 파티클 슬롯 번호
};

#define g_kJobSkillLocationInfo SINGLETON_STATIC(PgJobSkillLocationInfo)

#endif // FREEDOM_DRAGONICA_CONTENTS_JOBSKILL_PGJOBSKILLLOCATIONINFO_H