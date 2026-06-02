#include "StdAfx.h"
#include "PgMath.h"
#include "PgMoveAnimateWnd.h"
#include "PgDropBox.h"
#include "PgMobileSuit.h"

extern void CalcAbosolute(NiPoint3 & rkPos, bool const bAbsoluteX=false, bool const bAbsoluteY=false);

namespace PgDropBoxUtil
{
	extern float Bias(float x, float biasAmt);
}

PgMoveAnimateWnd::PgMoveAnimateWnd(void)
	: m_kMoveType(MUIT_DEFAULT)
	, m_kStartPoint()
	, m_kEndPoint()
	, m_kModifyPoint1()
	, m_kModifyPoint2()
	, m_kTotalMoveTime(0.0f)
	, m_kSpeedModifyValue(0.0f)
	, m_kMovePercent(1.0f)
	, m_kStartTime(0.0f)
	, m_kUsePause(false)
	, m_kIsPause(false)
	, m_kPauseTime(0.0f)
	, m_kPauseMovePercent(0.0f)
	, m_kStartPauseTime(0.0f)
	, m_kWatingPauseTime(0.0f)
{
}

PgMoveAnimateWnd::~PgMoveAnimateWnd(void)
{
}

void PgMoveAnimateWnd::VRegistAttr(std::wstring const& wstrName, std::wstring const& wstrValue)
{
	BM::vstring	vValue(wstrValue);

	std::wstring const ATTR_MOVE_TYPE = _T("MOVE_TYPE");
	std::wstring const ATTR_MOVE_TIME = _T("MOVE_TIME");
	std::wstring const ATTR_MDF_SPEED_VAL = _T("MDF_SPEED_VAL");
	std::wstring const ATTR_START_X = _T("START_X");
	std::wstring const ATTR_START_Y = _T("START_Y");
	std::wstring const ATTR_END_X = _T("END_X");
	std::wstring const ATTR_END_Y = _T("END_Y");
	std::wstring const ATTR_PT1_X = _T("MDF_PT1_X");
	std::wstring const ATTR_PT1_Y = _T("MDF_PT1_Y");
	std::wstring const ATTR_PT2_X = _T("MDF_PT2_X");
	std::wstring const ATTR_PT2_Y = _T("MDF_PT2_Y");
	//중간에 멈췄다 이동하는 기능
	std::wstring const ATTR_USE_PAUSE = _T("USE_PAUSE");
	std::wstring const ATTR_PAUSE_MOVE_PERCENT = _T("PAUSE_MOVE_PERCENT");
	std::wstring const ATTR_PAUSE_TIME = _T("PAUSE_TIME");

	if( wstrName == ATTR_MOVE_TYPE )
	{
		m_kMoveType = (EMoveAnimateType)((int)vValue);
	}
	else if( wstrName == ATTR_MOVE_TIME )
	{
		m_kTotalMoveTime = ((int)vValue) / 1000.0f;
	}
	else if( wstrName == ATTR_MDF_SPEED_VAL )
	{
		m_kSpeedModifyValue = ((int)vValue % 100) * 0.01f;
	}
	else if( wstrName == ATTR_START_X )
	{
		m_kStartPoint.x = static_cast<float>((int)vValue);
	}
	else if( wstrName == ATTR_START_Y )
	{
		m_kStartPoint.y = static_cast<float>((int)vValue);
	}
	else if( wstrName == ATTR_END_X )
	{
		m_kEndPoint.x = static_cast<float>((int)vValue);
	}
	else if( wstrName == ATTR_END_Y )
	{
		m_kEndPoint.y = static_cast<float>((int)vValue);
	}
	else if( wstrName == ATTR_PT1_X )
	{
		m_kModifyPoint1.x = static_cast<float>((int)vValue);
	}
	else if( wstrName == ATTR_PT1_Y )
	{
		m_kModifyPoint1.y = static_cast<float>((int)vValue);
	}
	else if( wstrName == ATTR_PT2_X )
	{
		m_kModifyPoint2.x = static_cast<float>((int)vValue);
	}
	else if( wstrName == ATTR_PT2_Y )
	{
		m_kModifyPoint2.y = static_cast<float>((int)vValue);
	}
	else if( wstrName == ATTR_USE_PAUSE )
	{
		m_kUsePause = ( 1 == (int)vValue );
	}
	else if( wstrName == ATTR_PAUSE_MOVE_PERCENT )
	{
		m_kPauseMovePercent = ((int)vValue) / 100.0f;
	}
	else if( wstrName == ATTR_PAUSE_TIME )
	{
		m_kPauseTime =  ((int)vValue) / 1000.0f;
	}
	else
	{
		XUI::CXUI_Wnd::VRegistAttr(wstrName, wstrValue);
	}
}

void PgMoveAnimateWnd::Setup(NiPoint3 const& rkStartPoint, NiPoint3 const& rkEndPoint, float const fTotalMoveTime)
{
	StartPoint(rkStartPoint);
	EndPoint(rkEndPoint);
	TotalMoveTime(fTotalMoveTime);
	Location(StartPoint().x,StartPoint().y);
	StartTime(g_pkApp ? g_pkApp->GetAccumTime() : 0);
	m_kStartPauseTime = 0.0f;
	m_kWatingPauseTime = 0.0f;
	m_kIsPause = false;
	m_kMovePercent = 0.0f;
}

bool PgMoveAnimateWnd::VOnTick( DWORD const dwCurTime )
{
	if( !Visible() )
	{
		return false;
	}
	
	UpdateLocation();	// 현재 TICK_TIME에 영향을 안받고 있음 OnDisplay일때와 동일하게 작동하기 위해.
	
	return XUI::CXUI_Wnd::VOnTick(dwCurTime);
}

bool PgMoveAnimateWnd::VDisplay()
{
	if( !Visible() )
	{
		return false;
	}
	
	return XUI::CXUI_Wnd::VDisplay();
}

bool PgMoveAnimateWnd::IsMoveComplate()const
{
	if( 1.0f == m_kMovePercent )
	{
		float const fAccumTime = g_pkApp->GetAccumTime();
		return (fAccumTime-m_kWatingPauseTime-m_kStartTime) >= m_kTotalMoveTime;
	}
	return false;
}

void PgMoveAnimateWnd::FinishMove()
{
	NiPoint3 kEndPos(m_kEndPoint);
	//CalcAbosolute(kEndPos, IsAbsoluteX(), IsAbsoluteY());
	Location(static_cast<int>(kEndPos.x), static_cast<int>(kEndPos.y));

	m_kMovePercent = 1.0f;
	m_kStartPauseTime = 0.0f;
	m_kWatingPauseTime = 0.0f;
	m_kIsPause = false;
}

bool PgMoveAnimateWnd::UpdateLocation()
{
	if(IsMoveComplate())
	{
		return false;
	}

	float const fAccumTime = g_pkApp->GetAccumTime();
	if( m_kUsePause )
	{
		if( m_kPauseMovePercent <= m_kMovePercent )
		{
			if( 0.0f == m_kStartPauseTime && 0.0f == m_kWatingPauseTime && false == m_kIsPause)
			{
				m_kStartPauseTime = fAccumTime;
				m_kIsPause = true;
			}
			else if( true == m_kIsPause )
			{
				m_kWatingPauseTime = fAccumTime - m_kStartPauseTime;
				if( m_kWatingPauseTime  > m_kPauseTime)
				{
					m_kIsPause = false;
				}
			}
		}
	}
	if(true == m_kIsPause)
	{
		return true;
	}
	float fmu = fAccumTime - m_kWatingPauseTime - m_kStartTime ;
	if( fmu < m_kTotalMoveTime )
	{
		fmu = fmu / m_kTotalMoveTime;
		if( SpeedModifyValue() )
		{
			fmu = PgDropBoxUtil::Bias(fmu, m_kSpeedModifyValue);
		}
	}
	else
	{
		FinishMove();
		return false;
	}

	NiPoint3 kStartPos(m_kStartPoint);
	//CalcAbosolute(kStartPos, IsAbsoluteX(), IsAbsoluteY());

	NiPoint3 kEndPos(m_kEndPoint);
	//CalcAbosolute(kEndPos, IsAbsoluteX(), IsAbsoluteY());	

	NiPoint3 kNowPos;
	switch( MoveType() )
	{
	case MUIT_BEZIER3:
		{
			Bezier3(kStartPos, m_kModifyPoint1, kEndPos, fmu, kNowPos);
			kNowPos.x -= (Size().x * 0.5f);
			kNowPos.y -= (Size().y * 0.5f);
		}break;
	case MUIT_BEZIER4:
		{
			Bezier4(kStartPos, m_kModifyPoint1, m_kModifyPoint2, kEndPos, fmu, kNowPos);
			kNowPos.x -= (Size().x * 0.5f);
			kNowPos.y -= (Size().y * 0.5f);
		}break;
	default:
		{
			kNowPos = kStartPos + ((kEndPos - kStartPos) * fmu);
		}break;
	}
	Location(static_cast<int>(kNowPos.x), static_cast<int>(kNowPos.y));
	m_kMovePercent = fmu;
	return true;
}

XUI::CXUI_Wnd* PgMoveAnimateWnd::VClone()
{
	PgMoveAnimateWnd *pWnd = new PgMoveAnimateWnd;
	*pWnd = *this;
	return pWnd;
}

void PgMoveAnimateWnd::operator = (PgMoveAnimateWnd const &rhs)
{
	this->CXUI_Wnd::operator =(rhs);
	MoveType(rhs.MoveType());
	StartPoint(rhs.StartPoint());
	EndPoint(rhs.EndPoint());
	ModifyPoint1(rhs.ModifyPoint1());
	ModifyPoint2(rhs.ModifyPoint2());
	TotalMoveTime(rhs.TotalMoveTime());
	SpeedModifyValue(rhs.SpeedModifyValue());
	MovePercent(0.0f);
	StartTime(0.0f);
	//
	UsePause(rhs.UsePause());
	IsPause(rhs.IsPause());
	PauseTime(rhs.PauseTime());
	PauseMovePercent(rhs.PauseMovePercent());
	StartPauseTime(0.0f);
	WatingPauseTime(rhs.WatingPauseTime());
}

void PgMoveAnimateWnd::SwapMovePoint()
{
	std::swap(m_kStartPoint,m_kEndPoint);
}