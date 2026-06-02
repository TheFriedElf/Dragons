#ifndef FREEDOM_DRAGONICA_CONTENTS_MISSIONSCORE_LWMISSIONSCOREUIMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_MISSIONSCORE_LWMISSIONSCOREUIMGR_H

#include "lwUI.h"
#include "PgUIDrawObject.h"

typedef enum E_MISSIONSCORE_TYPE
{
	MS_NONE = 0,
	MS_SENSE,
	MS_ABILITY,
}E_MissionScoreType;

class PgMissionScoreUIMgr
{
public:
	PgMissionScoreUIMgr();
	virtual ~PgMissionScoreUIMgr();

	void Start(E_MissionScoreType e_Type);
	void MissionScoreUIUpdate(E_MissionScoreType e_Type);

protected:
	CLASS_DECLARATION_S(bool, bUpdate);

	bool m_bScoreViewSense;
	bool m_bScoreViewAbility;
};

#define g_kMissionScoreUIMgr SINGLETON_STATIC(PgMissionScoreUIMgr)

extern void lwUpdateMissionScoreUI();
extern void lwDrawMissionScoreUI();
extern void lwMissionScoreTypeClear();
extern void lwUpdateMissionScoreStart(int e_Type);

class PgMissionFormAnimatedMoveWnd : public CXUI_Wnd
{
public:
	PgMissionFormAnimatedMoveWnd();
	~PgMissionFormAnimatedMoveWnd();

	virtual CXUI_Wnd* VCreate()const	{ return new PgMissionFormAnimatedMoveWnd; }

	virtual bool VDisplay();

	void StartAni();

	E_MissionScoreType m_eMissionScoreType;
private:
	bool m_bPlayMoveSound;
	bool m_bCanAni;
	float m_fStartAni;	
};

#endif // FREEDOM_DRAGONICA_CONTENTS_MISSIONSCORE_LWMISSIONSCOREUIMGR_H