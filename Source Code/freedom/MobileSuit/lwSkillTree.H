#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWSKILLTREE_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWSKILLTREE_H

#include "PgSkillTree.h"
class	lwUIWnd;
class	lwUIListItem;
class	XUI::CXUI_Wnd;
class	lwWString;

class	lwSkillTree
{
public:

	lwSkillTree()	{};

	static bool RegisterWrapper(lua_State *pkState);

	int	GetSkillLevel(int const iKeySkillNo, bool bOverSkill = false);
	int	GetMaxSkillLevel(int iKeySkillNo);
	int GetOverSkillLevel(int const iKeySkillNo);
	int	GetNextLevelSkillNo(int iKeySkillNo);
	
	bool	CanLevelUp(int iKeySkillNo);
	bool	CanLevelDown(int iKeySkillNo);
	int	GetSkillNo(int iKeySkillNo);
	int	GetKeySkillNo(int iSkillNo);
	bool	IsTemporaryLevelChanged(int iKeySkillNo);
	bool	IsTemporaryRemainSkillPoint(int iSkillNo);

	int	GetNeedSkillPoint(int iKeySkillNo);

	lwWString	GetSkillName(int iKeySkillNo);
	int	GetSkillIconKey(int iKeySkillNo);
	bool	IsActiveSkill(int iKeySkillNo);

	void	LevelUpTemporary(int iKeySkillNo);
	void	LevelDownTemporary(int iKeySkillNo);

	void	ResetTemporary();
	void	ConfirmTemporary();

	bool	IsLearnedSkill(int iKeySkillNo);
	bool	IsCommandSkill(int iKeySkillNo);
	
	void	SetRemainSkillPoint(int iSkillPoint);
	int	GetRemainSkillPoint();
	int	GetUsedSkillPoint();
	int	GetOriginalSkill(int const iKeySkillNo);

	void	NewSkillLearned(unsigned long ulSkillNo,int iRemainSkillPoint);

	int	GetTotalSkill();
	int	GetKeySkillNoByIndex(int iIndex);
	int	GetKeySkillResNoByIndex(int iIndex);
	int GetKeySkillResNoBySkillNo(int const iNo) const;
	bool GetCanLearn(int iTarget, int iFrom) const;
	bool CheckAllNeedSkill(int const iSkillNo) const;

	void	DeleteSkill(unsigned long ulSkillNo);

	bool IsNil();
//--------------ComboSkill UI -------------------
	void lwCallComboUI( lwUIListItem kMainWnd, int const iClassView );
	void lwClearComboList( XUI::CXUI_Wnd *pWnd );
	void SetCombo( XUI::CXUI_Wnd *pWnd, int const iWeaponType, int const iComboID, bool const bUsable);
	void SetCombo_WeaponName( XUI::CXUI_Wnd *pWnd, int const iWeaponType);
	void SetComboStep( XUI::CXUI_Wnd *pWnd, int const iWeaponType, int const iComboID, bool const bUsable );
	void SetComboStep_Limit( XUI::CXUI_Wnd *pWnd, __int64 i64ClassLimit, int const iLevel );
	void SetComboStep_Star( XUI::CXUI_Wnd *pWnd, int const iStar, bool const bUsable );
	void SetComboStep_StratType( XUI::CXUI_Wnd *pWnd, int const iStartType, bool const bUsable );
	bool SetComboStep_Skill( XUI::CXUI_Wnd *pWnd, int const iSkillNo, bool const bUsable );
	bool SetComboStep_InputKey( XUI::CXUI_Wnd *pWnd, int const iUiKey, eComboDirect const eDirect, bool const bCharge, bool const bUsable );

	int GetMaxComboCount(int const iWeaponType );
//--------------ComboAlram UI -------------------
	void lwSetComboAlramUI(lwUIWnd kWnd, int const iWeaponType, int const iComboID);
};

extern bool IsMyPlayerLearnedAnySpecialSkill();
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_SKILL_LWSKILLTREE_H