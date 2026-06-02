#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_PGPARTICLEPROCESSORSPRITEANIMATION_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_PGPARTICLEPROCESSORSPRITEANIMATION_H

#include "PgParticleProcessor.H"

typedef struct _tagSpriteFrameInfo
{
	float m_fDelay;

	_tagSpriteFrameInfo() :
		m_fDelay(0.0f)
	{}

	_tagSpriteFrameInfo(float fDelay) :
		m_fDelay(fDelay)
	{}

} SpriteFrameInfo;

class	PgParticleProcessorSpriteAnimation	:	public	PgParticleProcessor
{
	NiDeclareRTTI;
	NiDeclareClone(PgParticleProcessorSpriteAnimation);

	typedef std::vector<SpriteFrameInfo> CONT_FRAME_INFO;
	struct SRandomOffsetPosOption
	{
		typedef enum ERandomApplyType
		{
			E_NONE = 0,
			E_EACH_LOOP = 1,
			E_INTER_TIME = 2,
			E_RANDOM_APPLY_TYPE_MAX
		}ERandomApplyType;
		
		ERandomApplyType eType;
		float fIntervalTIme;
		float fElapsTIme;
		NiPoint3 kOriginPos;
		NiPoint3 kMin;
		NiPoint3 kMax;

		SRandomOffsetPosOption()
			:eType(E_NONE)
			,fIntervalTIme(0.0f)
			,fElapsTIme(0.0f)
			,kOriginPos(0.0f,0.0f,0.0f)
			,kMin(0.0f,0.0f,0.0f)
			,kMax(0.0f,0.0f,0.0f)
		{}
	};

public:

	PgParticleProcessorSpriteAnimation();
	PgParticleProcessorSpriteAnimation(NiAVObject* pkAttachNode);

	virtual ~PgParticleProcessorSpriteAnimation();

	virtual	bool	DoProcess(PgParticle* pkParticle, float fAccumTime,float fFrameTime);
	virtual	PgParticleProcessor::PARTICLE_PROCESSOR_ID	GetProcessorID()	const	{	return	PgParticleProcessor::PPID_SPRITE_ANIMATION;	}

	bool	IsLoop();
	void	SetLoop(bool const bLoop, int const iLoopCount = 0); // iLoopCount가 0이면 무한 루핑 // bLoop = false 이면 iLoopCount는 0이 된다.

	void	SetPlayRate(float const fPlayRate = 1.0f); // 0 ~ 100 사이 값만 동작 하도록 설정함 min/max로 범위 지정 시킴. // fPlayRate = 0 Stop과 같다.

	void	SetPlay();
	void	SetStop();

	//! Parsing Xml
	bool	ParseXml(TiXmlNode const* pkNode, void* pArg = 0);

			//노드를 타면서 기존 텍스쳐를 새로운 텍스쳐로 변경
	bool	ChangeTextureRecursive(NiNode* pkNode, std::string const& kSrcTextureName, std::string const& kDestTextureName);

			//지오메트리에 텍스쳐 정보를 세팅해서 넣어준다.(지오메트리 안에 텍스쳐 정보가 없을 경우)
	void	CreateBaseTextureInGeometry(std::string const& kGeometryName, std::string const& kTextureName);

private:
	bool	m_bPlay;

	bool	m_bLoop;
	int		m_iLoopCount;
	int		m_iCurLoopCount;

	float	m_fPlayRate;	

	float	m_fFrameStartTime;
	int		m_iCurFrame;

	int		m_iUSubdivision; // U V 몇개로 나눌 것인지.
	int		m_iVSubdivision;

	float	m_fUSize; // Texture 안에 사이즈 얼마로 나눌 것인지
	float	m_fVSize;

	float	m_fTexWidth;
	float	m_fTexHeight;

	float	m_fDefaultDelay;

	int		m_iMaxFrame;

	float	m_fStartDelay;
	bool	m_bStartDelayRandom;	//StartDelay true이면 값을 범위로 랜덤으로 설정 false이면 고정값 딜레이

	CONT_FRAME_INFO	m_kFrameInfo;
	std::string		m_kTextureName;
	std::string		m_kTextureNameSrc;
	std::string		m_kTextureNameDest;
	std::string		m_kGeometryName;

	NiNodePtr		m_spSpriteNode;

	NiAlphaProperty::AlphaFunction m_eSrcFunc;
	NiAlphaProperty::AlphaFunction m_eDestFunc;

	bool	m_bAlphaTesting;
	BYTE	m_byAlphaRef;
	
	SRandomOffsetPosOption m_kRandPosOpt;
	//NiPoint3		m_kRandomPosMin;
	//NiPoint3		m_kRandomPosMax;
};

NiSmartPointer(PgParticleProcessorSpriteAnimation);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_PGPARTICLEPROCESSORSPRITEANIMATION_H