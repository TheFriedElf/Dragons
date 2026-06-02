#ifndef FREEDOM_DRAGONICA_SOUND_PGSOUNDMAN_H
#define FREEDOM_DRAGONICA_SOUND_PGSOUNDMAN_H
#include "PgWorkerThread.h"
//#include "PgRscContainer.h"

//하나의 원음이 동시에 플래이 될 수 있는 최대 개수
const int MAX_IDENTITY_PLAY_SOUND_SOURCE = 6;

class PgSoundMan : public PgIClientWorker
//	: public PgRscContainer
{
public:
	struct	stSoundInfo
	{
		NiAudioSourcePtr	m_spSource;

		stSoundInfo(NiAudioSource *pkSource)
		{
			m_spSource = pkSource;
		}

		~stSoundInfo()
		{
			m_spSource = 0;
		}
	};

private:
	typedef struct tagSoundInfo
	{
		std::string	m_strPath;
		float		m_fVolume;
		float		m_fMinDist;
		float		m_fMaxDist;
	} SoundInitInfo;

	typedef struct tagBgmInfo
	{
		tagBgmInfo()
		{
			Clear();
		}

		explicit tagBgmInfo( NiAudioSourcePtr spBgm, float fVolume )
		{
			m_spBgmSound = spBgm;
			m_fVolume = fVolume;
		}

		tagBgmInfo( tagBgmInfo const &rhs )
			:	m_spBgmSound(rhs.m_spBgmSound)
			,	m_fVolume(rhs.m_fVolume)
		{}

		bool operator==( tagBgmInfo const &rhs )const
		{
			if ( m_spBgmSound && rhs.m_spBgmSound )
			{
				return !::strcmp( m_spBgmSound->GetFilename(), rhs.m_spBgmSound->GetFilename() );
			}
			return false;
		};

		bool operator!=( tagBgmInfo const &rhs )const
		{
			return !(*this == rhs);
		}

		void Clear()
		{
			m_spBgmSound = NULL;
			m_fVolume = 0.0f;
		}

		NiAudioSourcePtr m_spBgmSound;
		float m_fVolume;
	} BgmInfo;

	typedef class tagEnvSound
	{
	private:
		std::string m_kMediaPath;
		NiMilesSource* m_pkSource;
		float m_fVolume;
		float m_fMin;
		float m_fMax;
		NiPoint3 m_kPos;
		bool m_bRandomPlay;
		float m_fRandomProbability;
		bool m_bMixBGSound;
		explicit tagEnvSound(std::string const kPath, NiMilesSource* pkSource, float const fVol, 
			float const fMin, float const fMax, NiPoint3 const& kPos, bool bRandomPlay, float fRandomProbability, bool bMixBGSound)
			: m_kMediaPath(kPath), m_pkSource(pkSource), m_fVolume(fVol), m_fMin(fMin), m_fMax(fMax), 
			m_kPos(kPos), m_bRandomPlay(bRandomPlay), m_fRandomProbability(fRandomProbability), m_bMixBGSound(bMixBGSound)
		{		}

		bool operator==(tagEnvSound const& rhs)
		{
			return (m_kMediaPath == rhs.m_kMediaPath);
		}
		friend class PgSoundMan;
	}EnvSound;

	typedef	std::list<NiAudioSourcePtr> AudioSrcList;
	typedef	std::map<std::string,AudioSrcList> AudioSrcListMap;

	typedef std::map<std::string, SoundInitInfo> PathContainer;
	typedef std::map<std::string, stSoundInfo > SoundContainer;
	typedef std::vector<BgmInfo>	BGSoundContainer;
	typedef std::vector<EnvSound> EnvSoundContainer;

public:
	PgSoundMan();
	~PgSoundMan();

//	virtual HRESULT LoadResource(T_MSG &rkMsg, void *&pkOutRsc)const;

	//! 사운드 시스템을 초기화한다.
	bool Initialize();
	void Destroy();

	bool LoadBgSound(char const *pcSndPath, float fVolume, int const iLoopCount=0, bool bDefault=true );
	void UnloadBgSound();
	bool PlayBgSound( char const *pcSndPath, float fVolume, int const iLoopCount );
	void PlayBgSound(int iNo = -1, bool bStopImmediate = false);
	bool StopBgSound( bool bStopImmediate = false );

	//!	This function plays the audio source specified by pcPath.
	NiAudioSource*	PlayAudioSourceByID(unsigned int uiType, char const *pcID, float fVolume, float fDistMin = 80, float fDistMax = 180,NiNode *pkParent = NULL, NiPoint3* pkPos = NULL, int const iLoopCount = 1);
	NiAudioSource*	PlayAudioSourceByPath(unsigned int uiType, char const *pcPath, float fVolume, float fDistMin = 80, float fDistMax = 180,NiNode *pkParent = NULL, NiPoint3* pkPos = NULL, int const iLoopCount = 1);
	void			StopAudioSourceByID(std::string const& rkID);
	
	//! Sound Source List를 가진 Xml파일을 파싱한다.
	bool ParseXml(char const *pcXmlPath);
	void ParseXmlLocal(TiXmlElement const * pkElement);

	//! Audio Source를 리셋한다.
	void ResetAudioSource();
	void ResetAudioByID(std::string AudioID);

	void SetBGMVolume(float fVolume, bool bFromConfig=false);
	void SetEffectVolume(float fVolume, bool bFromConfig=false);
	float GetConfigBGMVolume();
	float GetConfigEffectVolume();

	void SetSilence(bool bSilence);
	virtual bool DoClientWork(WorkData& rkWorkData);

	void	RecursiveDetachAllSoundObject(NiAVObject *pkObject);

	//맵 로드시 환경사운드 추가
	bool AddAndPlayEnvSound(char const* pcPath, float const fVol, float const fMin, float const fMax, NiPoint3 const& kPos, bool bRandomPlay, float fRandomProbability, bool bMixBGSound);
	//환경사운드 업데이트
	void UpdateEnvSound(float const fTime);
	//맵 끝날때 환경사운드 제거
	void ReleaseEnvSound();	
	//환경사운드 디버그 렌더링 할까 말까?
	void SetEvnSoundDebugRendering(float bSet);

	bool CheckDistance(NiPoint3 const& kSourcePos, float const fDistMax) const;

protected:	
	bool isInit()	{	return m_bInitialized;	}
	float getConfigVolume(char const *szSubkey);
	
	bool m_bInitialized;
	float m_fBGMVolumeWeight;
	float m_fEffectVolumeWeight;

private:

	NiAudioSourcePtr CreateBgSound( char const *pcSndPath, float fVolume, int const iLoopCount );

	//! Path를 주면, 그에 해당하는 음원을 Clone후 반환 -> 앞으로 안 쓸 예정임
	NiAudioSource* LoadAudioSource(unsigned int uiType, char const *pcPath, float fVolume, float fDistMin = 80, float fDistMax = 180 );
	
	//! ID를 주면, 그에 해당하는 음원을 Clone해서 반환한다.
	NiAudioSource* GetAudioSource(unsigned int uiType, char const *pcID, float fVolume, float fDistMin = 80, float fDistMax = 180);
	void CacheAudioSource(unsigned int uiType, char const *pcID, float fVolume, float fDistMin = 80, float fDistMax = 180);

	bool Play( BgmInfo const &kBGM );

protected:

	//! 음원 Cache 컨테이너
	SoundContainer m_kSoundContainer;
	PathContainer m_kPathContainer;
	BGSoundContainer m_kBGSoundCont;
	BgmInfo m_kCurrentBGM;

	AudioSrcListMap	m_AudioSrcListMap;
	EnvSoundContainer		m_EnvSoundContainer;

	bool m_bSilence;// 참인경우 PlayBgSound 를 막는다.
	Loki::Mutex m_kSoundLock;
};

#define g_kSoundMan SINGLETON_STATIC(PgSoundMan)

#endif // FREEDOM_DRAGONICA_SOUND_PGSOUNDMAN_H