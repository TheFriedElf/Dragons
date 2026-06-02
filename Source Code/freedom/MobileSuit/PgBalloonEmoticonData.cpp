#include "StdAfx.h"
#include "PgBalloonEmoticonData.h"

int const DEFAULT_U_IDX = 2;

PgBalloonEmoticonData::PgBalloonEmoticonData(void)
	: m_kID(0)
	, m_kU(DEFAULT_U_IDX)
	, m_kLoopType(0)
	, m_kFrameSize()
	, m_kImagePath()
{
}

PgBalloonEmoticonData::~PgBalloonEmoticonData(void)
{
	m_kFrameContainer.clear();
}

void PgBalloonEmoticonData::AddFrame(PgBalloonEmoticonFrameData const& FrameInfo)
{
	m_kFrameContainer.push_back(FrameInfo);
}

PgBalloonEmoticonFrameData const* PgBalloonEmoticonData::GetFrame(int const FrameIdx) const
{
	if( FrameIdx < m_kFrameContainer.size() )
	{
		return &(m_kFrameContainer.at(FrameIdx));
	}
	return NULL;
}