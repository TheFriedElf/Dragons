#ifndef FREEDOM_DRAGONICA_UI_PGENERGYGAUGEBIG_H
#define FREEDOM_DRAGONICA_UI_PGENERGYGAUGEBIG_H

#include "PgEnergyGuage.h"

class PgEnergyGaugeBig
	:	public PgEnergyGauge
{
public:
	static std::wstring const	ms_wstrBarName;
	static std::wstring const	ms_wstrBarNameBoss;

	static PgEnergyGaugeBig*	ms_pkRendering;
	static bool 				ms_bShow;

public:
	explicit PgEnergyGaugeBig( PgPilot const *pkPilot );
	virtual ~PgEnergyGaugeBig();

	virtual EEnergyGaugeClassType GetClassType()const{ return EGAUGE_CLASS_BIG; }

	virtual void Update(float fAccumTime,float fFrameTime);
	virtual void DrawImmediate(PgRenderer *pkRenderer){}
	virtual	void SetPosition( NiPoint3 const &kTargetPos, const NiCamera *pkCamera ){}
	virtual void SetBarValue(int iMax,int iBefore,int iNew);
	virtual void Reset( PgPilot const *pkPilot );

	static void Show( bool bShow );

protected:
	bool Show( int const iValue );// iValue가 -1값이면 새로 셋팅하지는 않는다.

	void ClearVisible();

	std::wstring	m_wstrName;
	CLASS_DECLARATION_NO_SET( bool, m_bBoss, IsBoss );
	CLASS_DECLARATION_S(int, BossNo);
	BYTE			m_ucCallWarning;
	int				m_iIconNo;
	bool			m_bIsNoAliveTime;

private:
	PgEnergyGaugeBig();
	PgEnergyGaugeBig( PgEnergyGauge const & );
	PgEnergyGaugeBig& operator=( PgEnergyGauge const & );
};
#endif// FREEDOM_DRAGONICA_UI_PGENERGYGAUGEBIG_H