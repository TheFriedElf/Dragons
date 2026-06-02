#ifndef FREEDOM_DRAGONICA_CONTENTS_SKILL_PGSKILLTREE_H
#define FREEDOM_DRAGONICA_CONTENTS_SKILL_PGSKILLTREE_H

#include <vector>
#include "PgIXmlObject.h"

class	PgPilot;
class	PgAction;
#define	MAX_ONE_PLAYER_SKILL_CLASS	4	//	한 캐릭터가 가질 수 있는 최대 스킬 클래스 개수
#define TOTAL_SKILL_CLASS	36	//	총 스킬 클래스 갯수
#define	LEAD_SKILL_COUNT 10		// 선행스킬 갯수

enum eComboDirect
{
	ECD_NONE = 0,
	ECD_LEFT = 1,
	ECD_RIGHT = 2,
	ECD_UP = 3,
	ECD_DOWN = 4,
};
struct stComboStep
{
	int iStep;
	int iSkillNo;
	int iUiKey;
	bool bCharge;
	eComboDirect eDirect;
};
typedef std::list<stComboStep> CONT_COMBO_STEP;
enum eComboStartType
{
	ECST_ERROR = 0,
	ECST_NORMAL = 1,
	ECST_DASH = 2,
};
struct stComboData
{
	int iID;
	int iComboLevel;
	int iStarCount;
	__int64 i64ClassLimit;
	eComboStartType eStartType;
	CONT_COMBO_STEP kContComboStep;
};

typedef std::list<stComboData> CONT_COMBO_DATA;
typedef std::map<int, CONT_COMBO_DATA> CONT_BASIC_COMBO_DATA;
class	PgSkillTree
{
public:

	enum	NO_LEVELUP_REASON	//	스킬 레벨업 할 수 없는 이유
	{
		NLR_NONE=0,	//	레벨업 할 수 있음
		NLR_SKILLPOINT=(1<<0),	//	스킬 포인트가 모자람
		NLR_MAXLEVEL=(1<<1),	//	이미 최대 레벨임
		NLR_NEEDSKILL=(1<<2),	//	필요 스킬이 없음
		NLR_CHAR_CLASS=(1<<3),	//	배울 수 없는 캐릭터 클래스임
		NLR_CHAR_LEVEL=(1<<4),	//	캐릭터 레벨이 모자람
		NLR_UNKNOWN=(1<<5),	//	정의되지 않은 이유.
	};

	struct	stTreeNode;
	typedef	std::map<int,stTreeNode*> VTree;
	typedef	std::multimap<int,stTreeNode*> VTreeByDepth;

	struct	stTreeNode
	{
		unsigned	long	m_ulKeySkillNo;	// 같은 스킬인데 레벨만 다를때, 가장 낮은 레벨의 스킬이 KeySkill 이 된다.(일종의 대표 스킬)
		unsigned	long	m_ulSkillNo;
		int	m_iMaxSkillLevel;
		int	m_iOriginalSkillLevel;
		int m_iOverSkillLevel; 

		const CSkillDef *m_pkSkillDef;

		bool	m_bLearned;	//	배운 스킬인가
		bool	m_bTemporaryLearned;	//	임시로 배웠음

	public:

		stTreeNode(unsigned long const ulKeySkillNo, unsigned long const ulSkillNo, CSkillDef const* pkSkillDef, bool const bLearned,int const iOriginalSkillLevel=-1);
		virtual	~stTreeNode();

		bool	IsTemporaryLevelChanged() const;	//	현재 이 스킬이 임시로 레벨을 올린상태인가

		void	SetValue(unsigned long const ulSkillNo, CSkillDef const* pkSkillDef, int const iSkillLevel, bool const bLearned)
		{
			m_ulSkillNo = ulSkillNo;
			m_pkSkillDef = pkSkillDef;
			m_iOriginalSkillLevel = iSkillLevel;
			m_bLearned = m_bTemporaryLearned = bLearned;
		}

		void	SetOverSkillLevel(int const iOverSkillLevel) { m_iOverSkillLevel = iOverSkillLevel; }
		int		GetOverSkillLevel() const;

		CSkillDef const* GetSkillDef() const {	return	m_pkSkillDef;	}

		bool	IsLearned()	const {	return	m_bLearned;	}	

		void	SetMaxSkillLevel(int const iLevel)	{	m_iMaxSkillLevel = iLevel;	}
		int		GetMaxSkillLevel() const {	return	m_iMaxSkillLevel;	}

		void	ResetTemporary();
		int		ConfirmTemporary() const;
		int		GetOriginalSkillNo() const { return m_ulKeySkillNo+m_iOriginalSkillLevel-1; }
		int		GetOriginalSkillLevel() const { return m_iOriginalSkillLevel; }
	};

private:

	VTree	m_vSkills;

	int	m_iUsedSkillPoint;
	int	m_iRemainSkillPoint;
	int	m_iOriginalRemainSkillPoint;
	unsigned long m_iLastFoundSkillNo;
	stTreeNode* m_pkLastFoundTreeNode;

public:

	PgSkillTree();
	virtual	~PgSkillTree();

	void	Terminate()	{	ClearSkillTree();	}

	void	CreateSkillTree( PgPlayer const* pkPlayer );
	VTree&	GetTree();

	void	NewSkillLearned(unsigned long const ulSkillNo,int const iRemainSkillPoint);
	int		GetKeySkillNo(int const iSkillNo) const;
	int		GetKeySkillNo(PgAction const* pkAction) const;

	void	SetRemainSkillPoint(int const iSkillPoint);
	int		GetRemainSkillPoint() const;
	int		GetUsedSkillPoint() const;

	stTreeNode*	GetNode(unsigned long const ulKeySkillNo, bool const bIgnoreLastFound = false);

	int		GetNextLevelSkillNo(int const iKeySkillNo);
	int		CanLevelUp(int const iKeySkillNo);
	bool	CanLevelDown(int const iKeySkillNo);

	bool	IsTemporaryRemainSkillPoint(int const iSkillNo = 0);

	void	LevelUpTemporary(int const iKeySkillNo);
	void	LevelDownTemporary(int const iKeySkillNo);

	void	ResetTemporary();
	void	ConfirmTemporary();

	int		GetNeedSkillPoint(int const iKeySkillNo);
	bool	CanLearn(int const iTarget, int const iFrom = 0);
	bool	CheckAllNeedSkill(int const iSkillNo);
	bool	GetNeedSkill(int const iKeySkillNo, int* pkArray);

	void	ClearSkillTreeOverSkillLevel();
	void	AddPetSkillToTree(unsigned long const ulSkillNo, CSkillDef const* pkSkillDef);
	void	DeleteAllPetSkill();//최대한 적게 쓰자

	void	DeleteSkill(unsigned long const ulSkillNo);

private:
	void	ClearSkillTree();
	void	AddSkillToTree( PgPlayer const* pkPlayer , VTree& kDestTree, unsigned long const ulSkillNo, CSkillDef const* pkSkillDef );

//---------기본 콤보 UI---------------//	
public:
	void ParseXml_BasicCombo();

	int GetMaxComboCount(int const iWeaponType);
	int GetMaxComboStepCount( int const iWeaponType, int const iComboID);

	int GetCombo_Level( int const iWeaponType, int const iComboID);
	__int64 GetCombo_ClassLimit( int const iWeaponType, int const iComboID);
	int GetCombo_StarCount( int const iWeaponType, int const iComboID);
	int GetCombo_StartType( int const iWeaponType, int const iComboID);
	int GetComboStep_SkillNo( int const iWeaponType, int const iComboID, int const iStep);
	int GetComboStep_UiKey( int const iWeaponType, int const iComboID, int const iStep);
	eComboDirect GetComboStep_Direct( int const iWeaponType, int const iComboID, int const iStep);
	bool GetComboStep_IsCharge( int const iWeaponType, int const iComboID, int const iStep);
	
	bool IsComboData(int const iWeaponType, int const iComboID);
	bool IsHaveComboStep(int const iWeaponType, int const iComboID, int const iStep);
	bool IsUsableCombo(int const iWeaponType, int const iComboID, int const iCheck_Level, int const iCheck_Class);

private:
	CONT_BASIC_COMBO_DATA m_kBasicComboData;
	bool ParseXml_ComboData( TiXmlNode const *pNode, CONT_COMBO_DATA &rkContComboData );
	bool ParseXml_ComboInfo( TiXmlElement const *pElement, stComboData &rkComboData );
	bool ParseXml_ComboStep( TiXmlElement const *pElement, CONT_COMBO_STEP &rkContComboStep );
	eComboStartType GetComboStartType( char const* pcAttrValue );

	bool GetContComboData(int const iWeaponType, CONT_COMBO_DATA &rkData);
	bool GetComboData( int const iWeaponType, int const iComboID, stComboData &rkData);
	bool GetContComboStep(int const iWeaponType, int const iComboID, CONT_COMBO_STEP &rkData);
	bool GetComboStep(int const iWeaponType, int const iComboID, int const iStep, stComboStep &rkData);
//--------- 끝 : 기본 콤보 UI---------------//
//--------- 신규 콤보 사용가능 알림 UI ------------//
public:
	int FindCombo( int const iWeaponType, int const iLevel, int const iClass );
//--------- 끝 : 신규 콤보 사용가능 알림 UI ------------//
};

extern	PgSkillTree	g_kSkillTree;


#endif // FREEDOM_DRAGONICA_CONTENTS_SKILL_PGSKILLTREE_H