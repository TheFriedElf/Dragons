#ifndef FREEDOM_DRAGONICA_UI_PGENERGYGAUGE_H
#define FREEDOM_DRAGONICA_UI_PGENERGYGAUGE_H

#include <NiMain.H>
class	PgRenderer;
class	PgPilot;

typedef enum
{
	EGAUGE_DEFAULT	= 0,
	EGAUGE_CORE		= 1,
	EGAUGE_BOSS		= 2,
	EGAUGE_CHARGING1	= 3,
	EGAUGE_CHARGING2	= 4,
	EGAUGE_ALIVETIME	= 5,
	EGAUGE_PET			= 6,
	EGAUGE_SUMMONED		= 7,
	EGAUGE_ELITE		= 8,
}EEnergyGaugeType;

typedef enum
{
	EGOT_DEFAULT		= 0,
	EGOT_ALWAYSVIEW		= 0x1,
}EEnergyGaugeOptionType;

typedef enum
{
	EGAUGE_CLASS_DEFAULT,
	EGAUGE_CLASS_BIG,
}EEnergyGaugeClassType;

class	PgEnergyGauge	
{
	typedef std::vector< NiScreenTexturePtr > ContExternTexture; // at 연산이 가능
public:
	static bool	ms_bDrawEnergyGaugeBar;
	static bool ms_bDrawEnergyGaugeBarHide;
	static bool ms_bDrawPetMPGaugeBar;

protected:

	struct	stHPPart
	{
		NiPoint2	kStartPos,kCurrentPos;
		float	fStartTime;
		float	fHPStartPos;
		float	fHPLength;
		float	fAlpha;
		float	fScale;
		float	fRotate;
		stHPPart():
		kStartPos(0,0),
			fAlpha(0.0f),
			fScale(1.0f),
			fRotate(0.0f),
			kCurrentPos(0,0),
			fStartTime(0.0f),
			fHPLength(0.0f),
			fHPStartPos(0.0f)
		{
		};
	};

	typedef	std::list<stHPPart> PartCont;

	static float const	ms_fUseTime;
	static float const	ms_fNonVisibleTime;

	NiPoint3	m_kTargetPos;

	NiScreenTexturePtr	m_spGaugeOut;
	NiScreenTexturePtr	m_spGaugeIn;
	NiScreenTexturePtr	m_spPartGauge;
	ContExternTexture	m_kExternTexture;

	PartCont	m_HPPartCont;

	float	m_fVisibleStartTime;
	float	m_fTotalVisibleTime;
	
	float	m_fBeforeHPRate,m_fNewHPRate;
	float	m_fRateChangeSpeed;
	float	m_fCurrentRate;

	bool	m_bVisible;
	bool	m_bDraw;

	bool	m_bBlink;
	bool	m_bBlinkToggle;

	float	m_fLastBlinkTime;
	float	m_fBlinkDuration;
	float	m_fBlinkStartTime;
	float	m_fBlinkTotalTime;

	float	m_fStandardWidth;
	int		m_iType;
	int		m_iOption;

	bool	m_bAutoScale;
	float	m_fScale;

	size_t	m_iCurIndex;

	bool	m_bIsMyPet;

	CLASS_DECLARATION_NO_SET(int, m_iMaxValue, MaxValue);
//	CLASS_DECLARATION_NO_SET(int, m_iBeforeValue, BeforeValue);
//	CLASS_DECLARATION_NO_SET(int, m_iNewValue, NewValue);
public:

	explicit PgEnergyGauge( PgPilot const *pkPilot );
	virtual ~PgEnergyGauge();

	void Init(int const iGuageType, int const iOption=0);
	void Init(PgPilot const* pkPilot);

	virtual EEnergyGaugeClassType GetClassType()const{ return EGAUGE_CLASS_DEFAULT; }

	virtual void	SetBarValue(int iMax,int iBefore,int iNew);

	virtual void Update(float fAccumTime,float fFrameTime);
	virtual void DrawImmediate(PgRenderer *pkRenderer);

	virtual	void SetPosition(NiPoint3 const &kTargetPos,const NiCamera *pkCamera);

	bool IsVisibleTime()const { return m_fVisibleStartTime != ms_fNonVisibleTime; }
	virtual void ResetVisibleStartTime();
	virtual void SetVisible(bool bVisible);
	virtual	bool GetVisible()const;
	virtual void Reset( PgPilot const *pkPilot ) { }
	virtual void EnableDrawImmediate(bool bEnable) { m_bDraw = bEnable; } //렌더링이 가능한지 세팅

	void	Blink(float fTotalTime);

	int GetType()const{return m_iType;}

protected:
	
	void	UpdateBlink(float fAccumTime);

	void	ValueChanged( float const fBefore, float const fNew );
	void	DoClipping();

	void	SetBarLength(const	float fLengthRate);

	void	AddNewHPPart(const	NiPoint2	&kStartPos,float const fHPStartPos,float	fHPLength);
	void	UpdateHPParts(float fAccumTime,float fFrameTime);
	void	DrawHPParts(PgRenderer *pkRenderer);
	void	ClearHPParts();

	bool	IsDefaultView()const;
	bool	IsAlwaysView()const;

protected:
	PgEnergyGauge();

private:
	PgEnergyGauge( PgEnergyGauge const & );
	PgEnergyGauge& operator=( PgEnergyGauge const & );
};

class	PgEnergyGaugeMan
{
public:
	typedef std::list<PgEnergyGauge*> NodeList;

private:

	NodeList	m_NodeList;

public:

	~PgEnergyGaugeMan()		{	Destroy();	}

	void	Destroy();

	PgEnergyGauge*	CreateNewGauge(const PgPilot *pkPilot);
	PgEnergyGauge*	CreateInstallNewGauge(float fTotalTime = 10.0f); //설치류 타이머 게이지
	PgEnergyGauge*	CreateAliveTimeNewGauge(float fTotalTime); //살아있는 시간 게이지

	void	DestroyGauge(PgEnergyGauge *pkGauge);

	void Update(float fAccumTime,float fFrameTime);
	void DrawImmediate(PgRenderer *pkRenderer);
};

extern	PgEnergyGaugeMan	g_kEnergyGaugeMan;



class	PgInstallTimerGauge //설치류 타이머 게이지
	:	public PgEnergyGauge
{
public :
	bool	m_bUpdatePosition;
	float	m_fStartAccumTime;
	float	m_fTotalTime;
	float	m_fCurTime;
public :
	PgInstallTimerGauge();
	virtual ~PgInstallTimerGauge();

	virtual void Update(float fAccumTime,float fFrameTime);
	virtual void DrawImmediate(PgRenderer *pkRenderer);
	virtual void SetBarValue(int iMax,int iBefore,int iNew);
	virtual	void SetPosition(NiPoint3 const &kTargetPos,const NiCamera *pkCamera);

	void		 SetTotalTime(float fTotalTime);

};

class	PgAliveTimeGauge //살아있는 시간 게이지
	:	public PgEnergyGauge
{
public :
	bool	m_bUpdatePosition;
	float	m_fStartAccumTime;
	float	m_fTotalTime;
	float	m_fCurTime;
public :
	PgAliveTimeGauge();
	virtual ~PgAliveTimeGauge();

	virtual void Update(float fAccumTime,float fFrameTime);
	virtual void DrawImmediate(PgRenderer *pkRenderer);
	virtual void SetBarValue(int iMax,int iBefore,int iNew);
	virtual	void SetPosition(NiPoint3 const &kTargetPos,const NiCamera *pkCamera);

	void		 SetTotalTime(float fTotalTime);

};
#endif // FREEDOM_DRAGONICA_UI_PGENERGYGAUGE_H