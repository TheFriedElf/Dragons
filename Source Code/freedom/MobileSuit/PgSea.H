#pragma once

#include "NiMain.H"
#include "PgRenderer.H"

class	PgSea	:	public	NiMemObject
{

	NiNodePtr	m_spSeaNode;

	NiRenderedTexturePtr m_spRT_UnderWater;	//	물밑 장면
	NiRenderedTexturePtr m_spRT_UponWater;	//	물위 장면

    NiRenderTargetGroupPtr m_spRTG_UnderWater;
    NiRenderTargetGroupPtr m_spRTG_UponWater;

public:

	PgSea()	{	Init();	}
	~PgSea()	{	Destroy();	}

	void	Render(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);

private:

	void	Init();
	void	Destroy();

};