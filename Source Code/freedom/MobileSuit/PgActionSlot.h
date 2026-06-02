#ifndef FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONSLOT_H
#define FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONSLOT_H

#include "PgIXmlObject.h"

class PgActionSlot : public PgIXmlObject
{
public:
	
	struct	stSoundInfo
	{
		std::string	m_kSoundID;
		float	m_fVolume,m_fMinDist,m_fMaxDist;
		stSoundInfo()
		{
			m_fVolume=m_fMinDist=m_fMaxDist=0;
		}
		stSoundInfo(std::string const &kSoundID,float const &fVolume,float const &fMinDist,float const &fMaxDist)
		{
			m_kSoundID = kSoundID;
			m_fVolume = fVolume;
			m_fMinDist = fMinDist;
			m_fMaxDist = fMaxDist;
		}
	};

	typedef std::map<std::string, std::string> AnimationInfoMap;
	typedef std::map<std::string, int> DefaultAnimationContainer;
	//	typedef std::map<NiActorManager::SequenceID, NiAudioSourcePtr> SoundContainer;	// leesg213 2006-11-24, NiAudioSource* -->NiAudioSourcePtr 로 바꿈(스마트 포인터 사용)
	typedef std::map<std::string, stSoundInfo> SoundContainer;	// leesg213 2006-11-24, NiAudioSource* -->NiAudioSourcePtr 로 바꿈(스마트 포인터 사용)

	typedef struct stAnimationInfo {
		int iRandom;
		AnimationInfoMap kInfoMap;
		//float fSpeed;
		//std::string stdSoundPath;
		//float fSoundVolume;
		//float fSoundMinDist;
		//float fSoundMaxDist;

		stAnimationInfo()
		{
			iRandom = 100;
			//fSpeed = 0.0f
			//fSoundVolume = 0.0f;
			//fSoundMinDist = 1000.0f;
			//fSoundMaxDist = 30000.0f;
		}
	} AnimationInfo;

	typedef std::vector<std::pair<NiActorManager::SequenceID, AnimationInfo> > SequenceContainer;
	typedef std::map<std::string, SequenceContainer> AnimationContainer;
public:
	~PgActionSlot();

	virtual bool ParseXml(const TiXmlNode *kNode, void *pArg = 0, bool bUTF8 = false);
	
	bool GetDefaultAnimation(std::string const& rkSlotName, NiActorManager::SequenceID &rkSeqID_out);

	bool GetAnimation(std::string const &rkSlotName, NiActorManager::SequenceID &rkSeqID_out,bool bNoRandom=false)const;
	bool GetAnimationInfo(std::string const& rkSlotName,int iSeqID, std::string const& rkInfoName, std::string& rkInfoOut)const;
//	bool GetSound(NiActorManager::SequenceID, NiAudioSourcePtr &rpkSound_out);
	bool GetSound(std::string const& rkSeqID, stSoundInfo &kSoundInfo_Out)const;
	PgActionSlot *Clone();
	AnimationContainer const& GetAnimationCont() const;
private:
	DefaultAnimationContainer m_kDefaultAnimationContainer;
	AnimationContainer m_kAnimationContainer;
	SoundContainer m_kSoundContainer;
};

#endif // FREEDOM_DRAGONICA_RENDER_WORDOBJECT_ACTION_PGACTIONSLOT_H