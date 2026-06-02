
#ifndef FREEDOM_DRAGONICA_RENDER_EFFECT_PGROPE_H
#define FREEDOM_DRAGONICA_RENDER_EFFECT_PGROPE_H

#include <NiMain.H>

//	일단 줄타기에 쓰려고 PgRope 라고 이름을 지었지만, 사실은 그냥 실린더(원통) 지오메트리를 만들어주는 클래스.

class	PgRope	:	public	NiNode
{
	NiDeclareRTTI;

	NiPoint3	m_kStartPos,m_kEndPos;
	NiPoint3	m_vUp,m_vRight,m_vDir;

	float	m_fRange;	//	반지름

public:

	PgRope();

	float	GetRopeRange()	{	return	m_fRange;	}

	void	SetRopeData(NiPoint3 const &kStartPos,NiPoint3 const &kEndPos,float fRange=1.0f,char const *pkTexturePath=NULL);

	NiPoint3	const&	GetStartPos()	{	return	m_kStartPos;	}
	NiPoint3	const&	GetEndPos()	{	return	m_kEndPos;	}

private:

	NiTriShape*	SetGeometry(char const *pkTexturePath = NULL);
};

#endif // FREEDOM_DRAGONICA_RENDER_EFFECT_PGROPE_H