#ifndef FREEDOM_DRAGONICA_UI_PGHEADBUFFICON_H
#define FREEDOM_DRAGONICA_UI_PGHEADBUFFICON_H
#include "NiMain.H"

//	버프 혹은 디버프가 걸렸을 때, 해당 버프 ICON 을 머리위에 그려줄 때
//	사용되는 클래스들.

class	PgRenderer;

class	PgHeadBuffIcon
{
public:

	enum	State
	{
		S_NONE=0,
		S_APPEARNING,	//	등장하는 중
		S_IDLE,	//	등장한 후 대기 상태
		S_DISAPPEARING,	//	사라지는 중
		S_DISAPPEARED,	//	사라졌음
	};

private:

	//	부모 Icon 리스트의 중심좌표
	NiPoint3	m_kListCenterPos;

	//	중심으로부터의 가로축 거리
	float	m_fPos;
	
	//	스케일
	float	m_fTargetScale;
	float	m_fStartScale;
	float	m_fCurrentScale;

	//	이미지
	NiTriStripsPtr	m_spIconObj;

	//	Icon Res No
	int	m_iIconResNo;
	int	m_iEffectNo;

	//	State
	State	m_State;
	float	m_fStateChangeTime;

	bool	m_bNoRender;

public:

	PgHeadBuffIcon(const	int iEffectNo);
	virtual	~PgHeadBuffIcon();

	void Update(float fAccumTime,float fFrameTime);
	void DrawImmediate(PgRenderer *pkRenderer,NiCamera *pkCamera);

	void	SetListCenterPos(const	NiPoint3	&kCenterPos)	
	{
		m_kListCenterPos = kCenterPos;
	}
	const	NiPoint3&	GetListCenterPos()	const
	{
		return	m_kListCenterPos;
	}

	void	StartAppearing();
	void	StartDisappearing();

	void	SetPos(float fPos)	{	m_fPos = fPos;	}
	float	GetPos()	const	{	return	m_fPos;	}

	void	SetNoRender(bool bNoRender)	{	m_bNoRender = bNoRender;	}
	bool	GetNoRender()	const	{	return	m_bNoRender;	}

	State	GetState()	const	{	return	m_State;	}

	int	GetEffectNo()	const	{	return	m_iEffectNo;	}
	int GetIconResNo() const { return m_iIconResNo; }

private:

	void	SetState(State kNewState);

	bool	InitIconObj();
};

class	PgHeadBuffIconList
{
public:

	typedef	std::vector<PgHeadBuffIcon*> HeadBuffIconCont;

private:

	NiPoint3	m_kPos;

	HeadBuffIconCont	m_Icons;

public:

	PgHeadBuffIconList();
	virtual	~PgHeadBuffIconList();

	void Update(float fAccumTime,float fFrameTime);
	void DrawImmediate(PgRenderer *pkRenderer,NiCamera *pkCamera);

	void	AddNewIcon(const	int	iEffectNo);
	void	RemoveIcon(const	int	iEffectNo);
	void	RemoveAllIcon();

	void	SetPosition(NiPoint3 const &kPos);

private:

	void	ArrangeIcons();
	PgHeadBuffIcon*	GetIcon(const	int	iEffectNo);
	int	GetValidIconCount();
};

class	PgMobileSuit;
class	PgHeadBuffIconListMgr
{
	friend	class	PgMobileSuit;

public:

	typedef	std::list<PgHeadBuffIconList*> HeadBuffIconListCont;

private:

	HeadBuffIconListCont	m_IconLists;
	
	bool	m_bAlive;

public:

	PgHeadBuffIconListMgr();
	virtual	~PgHeadBuffIconListMgr();


	PgHeadBuffIconList*	CreateNewIconList();
	void	ReleaseIconList(PgHeadBuffIconList *pkIconList);

	void Update(float fAccumTime,float fFrameTime);
	void DrawImmediate(PgRenderer *pkRenderer,NiCamera *pkCamera);

	bool	IsAlive()	{	return	m_bAlive;	}

protected:


	void	Terminate();
	void	Init();

};


extern	PgHeadBuffIconListMgr	g_kHeadBuffIconListMgr;
#endif // FREEDOM_DRAGONICA_UI_PGHEADBUFFICON_H