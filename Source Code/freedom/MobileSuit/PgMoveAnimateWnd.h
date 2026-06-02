#ifndef FREEDOM_DRAGONICA_UI_PGMOVEANIMATION_H
#define FREEDOM_DRAGONICA_UI_PGMOVEANIMATION_H

class PgMoveAnimateWnd
	: public XUI::CXUI_Wnd
{
public:
	PgMoveAnimateWnd(void);
	virtual ~PgMoveAnimateWnd(void);

	virtual void VRegistAttr(std::wstring const& wstrName, std::wstring const& wstrValue);
	virtual CXUI_Wnd* VCreate() const { return new PgMoveAnimateWnd; }
	virtual bool VOnTick( DWORD const dwCurTime );
	virtual bool VDisplay();
	virtual CXUI_Wnd* VClone();

	void operator = (PgMoveAnimateWnd const &rhs);

	typedef enum eMoveAnimateType
	{
		MUIT_DEFAULT = 0,
		MUIT_BEZIER3 = 1,
		MUIT_BEZIER4,
	}EMoveAnimateType;

	void SwapMovePoint();
	bool IsMoveComplate()const;
	void Setup(NiPoint3 const& rkStartPoint, NiPoint3 const& rkEndPoint, float const fTotalMoveTime);

protected:
	bool UpdateLocation();
	void FinishMove();

private:
	CLASS_DECLARATION_S(EMoveAnimateType, MoveType);
	CLASS_DECLARATION_S(NiPoint3, StartPoint);
	CLASS_DECLARATION_S(NiPoint3, EndPoint);
	CLASS_DECLARATION_S(NiPoint3, ModifyPoint1);
	CLASS_DECLARATION_S(NiPoint3, ModifyPoint2);
	CLASS_DECLARATION_S(float, TotalMoveTime);
	CLASS_DECLARATION_S(float, SpeedModifyValue);
	CLASS_DECLARATION_S(float, MovePercent);
	CLASS_DECLARATION_S(float, StartTime);
	//중간에 멈췄다 다시 이동하는 기능 추가
	CLASS_DECLARATION_S(bool, UsePause);
	CLASS_DECLARATION_S(float, PauseTime);
	CLASS_DECLARATION_S(float, PauseMovePercent);
	CLASS_DECLARATION_S(float, StartPauseTime);
	CLASS_DECLARATION_S(float, WatingPauseTime);
	CLASS_DECLARATION_S(bool, IsPause);
	
};
#endif //FREEDOM_DRAGONICA_UI_PGMOVEANIMATION_H