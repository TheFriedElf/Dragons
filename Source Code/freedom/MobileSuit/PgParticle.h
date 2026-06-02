#ifndef FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLE_H
#define FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLE_H

#include "PgParticleProcessor.H"

class PgParticle : public NiNode
{
	NiDeclareRTTI;

public:

	typedef	std::vector<PgParticleProcessorPtr> ParticleProcessorVec;
public:

	enum	OPTION
	{
		O_NONE=0,
		O_SCALE=(1<<0),
		O_LOOP=(1<<1),
		O_ZTEST=(1<<2),
		O_SOUNDTIME=(1<<3),
		O_AUTOGROUND=(1<<4),
		O_USEAPPACCUMTIME=(1<<5),
		O_NOFOLLOWPARENTROTATION=(1<<6),
		O_SPRITE_ANIMATION=(1<<7),
		O_ALL=O_SCALE|O_LOOP|O_ZTEST|O_SOUNDTIME|O_AUTOGROUND|O_USEAPPACCUMTIME|O_NOFOLLOWPARENTROTATION|O_SPRITE_ANIMATION,
		O_MAX
	};

	struct	stParticleInfo
	{
		std::string	m_kPath;
		float	m_fScale;
		bool	m_bLoop;
		bool	m_bZTest;
		float	m_fSoundTime;
		bool	m_bAutoGround;
		bool	m_bUseAppAccumTime;
		bool	m_bNoFollowParentRotation;
		bool	m_bNoUseCacheAndDeepCopy;
		float	m_fAliveTime;
		std::string	m_kSoundID;
		std::string m_kSourceType;
		std::string m_kSpriteXML;

		stParticleInfo()
			:m_fScale(1.0f),
			m_bLoop(false),
			m_bZTest(false),
			m_fSoundTime(0.0f),
			m_bAutoGround(false),
			m_bUseAppAccumTime(false),
			m_bNoFollowParentRotation(false),
			m_bNoUseCacheAndDeepCopy(false),
			m_fAliveTime(0.0f)
		{
		};
		stParticleInfo(std::string const& kPath,float fScale,bool bLoop,bool bZTest,float fSoundTime,bool bAutoGround,bool bUseAppAccumTime, bool bNoFollowParentRotation,std::string const& kSoundID,std::string const& kSourceType, std::string const& kSpriteXML, bool bNoUseCacheAndDeepCopy, float const fAliveTime)
			:m_kPath(kPath),
			m_fScale(fScale),
			m_bLoop(bLoop),
			m_bZTest(bZTest),
			m_fSoundTime(fSoundTime),
			m_bAutoGround(bAutoGround),
			m_bUseAppAccumTime(bUseAppAccumTime),
			m_bNoFollowParentRotation(bNoFollowParentRotation),
			m_kSoundID(kSoundID),
			m_kSourceType(kSourceType),
			m_kSpriteXML(kSpriteXML),
			m_bNoUseCacheAndDeepCopy(bNoUseCacheAndDeepCopy),
			m_fAliveTime(fAliveTime)
		{
		};
	};

public:

	PgParticle();
	virtual ~PgParticle();

	virtual	bool Update(float fAccumTime, float fFrameTime);
	
	virtual void UpdateDownwardPass(float fTime, bool bUpdateControllers);	

	//! NIF 파일을 로딩한다.
	virtual	bool Create(NiAVObject* pkParticleRoot,std::string const& kID,stParticleInfo const& kParticleInfo) =0;

	virtual	void	ApplyOptions(OPTION kValidOptions,float fScale = 1.0f, bool bLoop = false, bool bZTest = true, float bSoundTime = 0.0f,bool bAutoGround = false,bool bUseAppAccumTime=false, bool bNoFollowParentRotation = false);

public:

	void	SetParticleProcessor(PgParticleProcessor* pkParticleProcessor)
	{
		m_kParticleProcessorCont[pkParticleProcessor->GetProcessorID()] = pkParticleProcessor;
	}
	void	RemoveParticleProcessor(PgParticleProcessor::PARTICLE_PROCESSOR_ID const& kProcessorID)
	{
		m_kParticleProcessorCont[kProcessorID] = 0;
	}
	PgParticleProcessor*	GetParticleProcessor(PgParticleProcessor::PARTICLE_PROCESSOR_ID const& kProcessorID)
	{
		return	m_kParticleProcessorCont[kProcessorID];
	}
	bool	IsAutoGround()const
	{
		return m_kParticleProcessorCont[PgParticleProcessor::PPID_AUTO_GROUND];
	}
public:

	bool	IsFinished()	const	{	return	m_bFinished;	}
	bool GetZTest();
	bool IsLoop();

	void	SetNoFollowParentRotation(bool bNoFollow)	{	m_bNoFollowParentRotation = bNoFollow;	}	
	bool	GetNoFollowParentRotation()	{	return	m_bNoFollowParentRotation;	}

	void	SetID(std::string const& kID)	{	m_kID = kID;	}
	std::string const& GetID()	const	{	return	m_kID;	}

	void	SetNow(float fNow)	{	m_fNow = fNow;	}
	float	GetNow()	const	{	return	m_fNow;	}

public:

	void SetLoop(bool bLoop);
	void SetZTest(bool bZTest);

	void	SetOriginalScale(float fScale)	{	m_fOriginalScale = fScale;	}
	float	GetOriginalScale()	const	{	return	m_fOriginalScale;	}
	void	SetOriginalRotate(NiMatrix3 const & kRotate)	{	m_kOriginalRotate = kRotate;	}
	NiMatrix3 const&	GetOriginalRotate()	const	{	return m_kOriginalRotate;	}

	void	SetAutoGround(bool bAutoGround);
	void	SetSound(float fSoundTime,std::string const& kSoundID);

	void	SetSprite(std::string const& kSpriteXml);

	void	SetUseAppAccumTime(bool bUseAppAccumTime)	{	m_bUseAppAccumTime = bUseAppAccumTime;	}
	bool	GetUseAppAccumTime()	{	return	m_bUseAppAccumTime;	}
	
	void	SetFinished(bool bFinished)	{	m_bFinished = bFinished;	}

	void	SetAliveTime(float const fTotalAliveTime);
	bool	UseAliveTime()	const	{	return m_bUseAliveTime;		}
	void	CheckAliveTime(float const fFrameTime);
	
	void	SetPlaySpeed(float const fPlaySpeed);
protected:

	void UpdateNowTime(float fAccumTime);
	bool	DoParticleProcessor(float fAccumTime,float fFrameTime);

	virtual	void	ApplyParticleInfo(stParticleInfo const& kParticleInfo);
	virtual	void	CheckFinish(float fAccumTime,float fFrameTime);

	void CopyMembers(PgParticle* pDest, NiCloningProcess& kCloning);

protected:

	ParticleProcessorVec	m_kParticleProcessorCont;

	float	m_fStartTime;
	float	m_fNow;
	bool	m_bLoop;
	bool	m_bZTest;

	bool	m_bUseAppAccumTime;
	bool	m_bNoFollowParentRotation;	//	부모의 회전값을 사용하지 않는다.

	std::string	m_kID;

	NiMatrix3 m_kOriginalRotate;
	float	m_fOriginalScale;
	bool	m_bFinished;

	bool	m_bUseAliveTime;
	float	m_fTotalAliveTime;
};

NiSmartPointer(PgParticle);

namespace PgParticleUtil
{
	void SetParticlePlaySpeed(NiAVObject* pkAVObj, float const fPlaySpeed);
}
#endif // FREEDOM_DRAGONICA_RENDER_STREAM_PGPARTICLE_H