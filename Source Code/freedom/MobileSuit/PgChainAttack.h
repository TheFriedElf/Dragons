#ifndef FREEDOM_DRAGONICA_UI_PGCHAINATTACK_H
#define FREEDOM_DRAGONICA_UI_PGCHAINATTACK_H

class PgActor;
class	PgRenderer;
class	NiCamera;
class	PgMobileSuit;
// 2연타 이상했을 경우 콤보 수를 보여주는 클래스(3초 이내에만 때리면 무조건 연타임)
class PgChainAttack 
{
	friend	class	PgMobileSuit;

public:


	struct	stComboNumNode
	{
		NiScreenTexturePtr	m_spComboText,m_spNumber;

		int	m_iNumberWidth[100];
		POINT	m_ptTargetLoc,m_ptStartLoc;

		bool	m_bAutoFadeOut;

		int	m_iState;		//	0 : 등장 1 : 등장 후 정지 2 : 퇴장
		float	m_fStartTime;

		stComboNumNode()
		{
			m_bAutoFadeOut = false;
		}
	};

	struct	stRaitingNode
	{
		NiScreenElementsPtr	m_spRatingText;
		NiScreenElementsPtr	m_spExpText;

		int	m_iState;	//	0 : 등장 1 : 등장 후 정지 2 : 퇴장
		float	m_fStartTime;

		int	m_iRatingIndex;

		bool	m_bNoExpTextPlay;

		stRaitingNode():
			m_bNoExpTextPlay(false)
		{
		};
	};

	typedef	std::list<stComboNumNode*>	ComboNumNodeList;
	typedef	std::list<stRaitingNode*>	RatingNodeList;
	typedef std::map< int, int > ContLazyCombo;


public:
	float m_fRecentChainAttackTime;					//! 최근 Chain Attack 시간
	int	m_iChainAttackCount;			// 연출상 Chain Attack Count
	int	m_iTargetChainAttackCount;		//! 최근 Chain Attack Count
	ContLazyCombo	m_kContLazyCombo;				// 서버로 부터 먼저 도착한
	ContLazyCombo	m_kContLazyComboClientFirst;	// 클라가 먼저 시작한
	bool			m_bUpdateDisplay;

	float	m_fTotalFadeOutTime;					// 사라지는 시간
	float	m_fTotalMovingTime;						// 등장시간

	ComboNumNodeList	m_ComboNumNodeList;
	RatingNodeList	m_RatingNodeList;

public:

	PgChainAttack();
	virtual ~PgChainAttack();

	void Update(float fAccumTime,float fFrameTime);
	void DrawImmediate(PgRenderer *pkRenderer);

	void NewCount(int const iNewCombo, int const iSkillNo);
	void IncreaseChainAttackCount(int const iSkillNo);				//! 연출상 Chain Attack Count 를 1 증가시킨다
	void ClearAll();

protected:
	void SetChainAttackCount(int iCount);

	void Init();
	void Destroy();


private:

	NiSourceTexturePtr m_spNumberTex;
	NiSourceTexturePtr m_spComboTex;
	NiSourceTexturePtr m_spRatingTex;
	NiSourceTexturePtr m_spExpTex;

	int		m_iRateIndex;

	void	AddNewComboNumNode(int iChainCount);
	void	AddNewRatingNode(int iChainCount);

	void	FadeOutEveryComboNumNode();
	void	FadeOutEveryRatingNode();

	bool	LoadResource(stComboNumNode *pkTargetNode);
	bool	LoadResource(stRaitingNode *pkTargetNode,bool bCreateExpNode);

	void	ClearAllNode();

	bool	UpdateComboNumNode(stComboNumNode *pkNode,float fAccumeTime);
	bool	UpdateRatingNode(stRaitingNode *pkNode,float fAccumeTime);

	void	ScaleRatingNode(stRaitingNode *pkNode,float fScale);
};

extern	PgChainAttack	g_kChainAttack;
#endif // FREEDOM_DRAGONICA_UI_PGCHAINATTACK_H