#ifndef FREEDOM_DRAGONICA_RENDER_EFFECT_PGBOWSTRING_H
#define FREEDOM_DRAGONICA_RENDER_EFFECT_PGBOWSTRING_H

#include "NiMain.h"

class	PgBowString	:	public	NiNode
{

	NiLinesPtr		m_spStringLine;

	NiNodePtr	m_spStringPoint[3];
	NiPoint3	m_kBeforePoint[3];

public:

	PgBowString()	:	NiNode(2)
	{
		m_spStringLine = 0;

		m_spStringPoint[0]=m_spStringPoint[1]=m_spStringPoint[2]=0;
	};
	virtual	~PgBowString()
	{
		m_spStringLine = 0;

		m_spStringPoint[0]=m_spStringPoint[1]=m_spStringPoint[2]=0;
	}

	virtual	void	UpdateDownwardPass(float fTime, bool bUpdateControllers);

	bool	AttachTo(NiNodePtr	m_spAttachTargetNode,const NiColorA& kBowStringColor);

	static	bool	HasBowStringHelper(NiNodePtr spNode);

};

#endif // FREEDOM_DRAGONICA_RENDER_EFFECT_PGBOWSTRING_H