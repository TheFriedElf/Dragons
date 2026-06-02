#ifndef FREEDOM_DRAGONICA_UI_PGSCREENBREAK_H
#define FREEDOM_DRAGONICA_UI_PGSCREENBREAK_H
#include <string>
#include <map>
#include <list>

// 몬스터가 날아와 화면에 부딛히면, 화면이 깨진 모습을 보여주는 클래스
int const MAX_SCREEN_BREAK_TEX = 3;

class	PgMobileSuit;
class PgScreenBreak	
{
	friend	class	PgMobileSuit;
	NiScreenTexturePtr m_spTex[MAX_SCREEN_BREAK_TEX];

	struct stScreenBreakNode
	{
		int	iTexIndex;
		int iRectIndex;
		unsigned	long	ulCreateTime;
	};

	typedef std::list<stScreenBreakNode*> NodeList;

	NodeList	m_NodeList;
	float m_fAccumTimeSaved;

public:
	//! WorldObject를 fFrameTime(AccumTime)에 대한 시각으로 갱신
	virtual bool Update(float fAccumTime, float fFrameTime);
	virtual void Draw(PgRenderer* pkRenderer);
	void	AddNewBreak(float fScreenX,float fScreenY);

	void	ClearAll();

protected:

	void	Init();
	void	Destroy();
};

extern	PgScreenBreak	g_kScreenBreak;
#endif //FREEDOM_DRAGONICA_UI_PGSCREENBREAK_H