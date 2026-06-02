#include "stdafx.h"
#include "PgScreenBreak.h"
#include "PgNifMan.h"
#include "PgRenderer.h"
#include "PgUIScene.H"
////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgScreenBreak
////////////////////////////////////////////////////////////////////////////////////////////////////
PgScreenBreak	g_kScreenBreak;

//! WorldObject를 fFrameTime(AccumTime)에 대한 시각으로 갱신
bool PgScreenBreak::Update(float fAccumTime, float fFrameTime)
{
	m_fAccumTimeSaved = fAccumTime;

	if(m_NodeList.size() == 0) return true;

	//	생성한지 1초가 지나면 알파값이 작아진다.
	//	생성한지 2초가 지나면 사라진다.
	stScreenBreakNode *pNode;
	unsigned long ulNow = BM::GetTime32();
	unsigned long ulElapsedTime;
	NodeList::iterator itr;
	bool bAlphaChanged = false;
	bool bDeleted = false;
	for(itr = m_NodeList.begin();itr != m_NodeList.end();)
	{
		pNode = *itr;

		ulElapsedTime = ulNow - pNode->ulCreateTime;
		if(ulElapsedTime>1000 && ulElapsedTime<2000)
		{
			float fAlphaRate = (2000-ulElapsedTime)/1000.0f;
			NiScreenTexture::ScreenRect &rect = m_spTex[pNode->iTexIndex]->GetScreenRect(pNode->iRectIndex);

			rect.m_kColor.a = fAlphaRate;
			bAlphaChanged = true;

		}
		else if(ulElapsedTime>2000)
		{
			bDeleted = true;
			m_spTex[pNode->iTexIndex]->RemoveScreenRect(pNode->iRectIndex);
			SAFE_DELETE(pNode);
			itr = m_NodeList.erase(itr);
			continue;
		}
		itr++;
	}
	int iIndex[MAX_SCREEN_BREAK_TEX];
	for(int i=0;i<MAX_SCREEN_BREAK_TEX;i++) iIndex[i] = 0;

	for(itr = m_NodeList.begin();itr != m_NodeList.end();itr++)
	{
		pNode = *itr;
		pNode->iRectIndex = iIndex[pNode->iTexIndex]++;
	}

	if(bAlphaChanged || bDeleted)
	{
		for(int i=0;i<MAX_SCREEN_BREAK_TEX;i++) 
			m_spTex[i]->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	}

	return true;
}
//! pkRenderer를 이용해서 Draw
void PgScreenBreak::Draw(PgRenderer *pkRenderer)
{
	if(m_fAccumTimeSaved == 0 || m_NodeList.size() == 0) return;

	for(int i=0;i<MAX_SCREEN_BREAK_TEX;i++)
		PgUIScene::Render_UIObject(pkRenderer,m_spTex[i]);

}
void	PgScreenBreak::AddNewBreak(float fScreenX,float fScreenY)
{

	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	int iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	int iCenterX = (int)(iScreenWidth * fScreenX);
	int iCenterY = (int)(iScreenHeight * (1-fScreenY));

	int iPixWidth = 512;
	int iPixHeight = 512;

	int iLeft = iCenterX - iPixWidth/2;
	int iTop = iCenterY - iPixHeight/2;
	int iAdjustX = 0,iAdjustY = 0;
	if(iLeft < 0)
	{
		iAdjustX = -iLeft;
		iLeft = 0;
	}
	if(iTop<0)
	{
		iAdjustY = -iTop;
		iTop = 0;
	}
	if(iAdjustX>=iPixWidth || iAdjustY>=iPixHeight) return;

	stScreenBreakNode *pNewNode = new stScreenBreakNode();

	pNewNode->iTexIndex = BM::Rand_Index(MAX_SCREEN_BREAK_TEX);
	pNewNode->iRectIndex = m_spTex[pNewNode->iTexIndex]->GetNumScreenRects();

	m_spTex[pNewNode->iTexIndex]->AddNewScreenRect(iTop,iLeft,iPixWidth-iAdjustX,iPixHeight-iAdjustY,iAdjustY,iAdjustX);
	pNewNode->ulCreateTime = BM::GetTime32();

	m_NodeList.push_back(pNewNode);
	m_spTex[pNewNode->iTexIndex]->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
}
void	PgScreenBreak::Init()
{
	m_fAccumTimeSaved = 0;

	NiSourceTexture *pTexture = g_kNifMan.GetTexture("../Data/5_effect/9_Tex/EF_screen_break_01.dds");
	m_spTex[0] = NiNew NiScreenTexture(pTexture);
	m_spTex[0]->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

	pTexture = g_kNifMan.GetTexture("../Data/5_effect/9_Tex/EF_screen_break_02.dds");
	m_spTex[1] = NiNew NiScreenTexture(pTexture);
	m_spTex[1]->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

	pTexture = g_kNifMan.GetTexture("../Data/5_effect/9_Tex/EF_screen_break_03.dds");
	m_spTex[2] = NiNew NiScreenTexture(pTexture);
	m_spTex[2]->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
}
void	PgScreenBreak::Destroy()
{
	m_spTex[0] = 0;
	m_spTex[1] = 0;
	m_spTex[2] = 0;

	ClearAll();
}
void	PgScreenBreak::ClearAll()
{
	for(NodeList::iterator itr = m_NodeList.begin();
		itr != m_NodeList.end();
		++itr)
	{
		SAFE_DELETE(*itr);
	}

	m_NodeList.clear();
}