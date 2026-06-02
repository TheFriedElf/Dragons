#include "stdafx.h"
#include "PgScreenEffect.h"
#include "PgNifMan.h"
#include "PgRenderer.h"
#include "PgUIScene.H"

namespace ScreenEffect
{

//화면에 뿌려진 텍스쳐들을 갱신, 시간 경과시 삭제
void PgScreenEffect::Update(float fAccumTime, float fFrameTime)
{
	m_kAccumTimeSaved = fAccumTime;

	if(m_kContNode.size() == 0)
	{
		return;
	}

	ScreenEffectNode_ *pNode = NULL;
	unsigned long ulNow = BM::GetTime32();
	unsigned long ulElapsedTime;
	ContNode::iterator it;
	bool kDeleteIt = false;
	float kAlphaRate = 0.0f;

	BM::CAutoMutex kLock(m_kMutex);

	for(it = m_kContNode.begin(); it != m_kContNode.end();)
	{
		pNode = (*it);
		kDeleteIt = false;	
		if(pNode)
		{
			ContTexture::iterator Texture_it = m_kContTexture.find(pNode->m_kTextureName);
			if(Texture_it!=m_kContTexture.end())
			{
				ulElapsedTime = ulNow - pNode->m_kCreateTime;
				if(0 < pNode->m_kAlphaTime && ulElapsedTime > pNode->m_kAlphaTime && ulElapsedTime < pNode->m_kEndTime)
				{//알파 뺄 타임
					kAlphaRate = static_cast<float>
						((pNode->m_kEndTime - ulElapsedTime)/pNode->m_kAlphaTime);

					NiScreenTexture::ScreenRect &rect = 
						((*Texture_it).second)->GetScreenRect(pNode->m_kRectIndex);
					rect.m_kColor.a = kAlphaRate;
				}
				else
				{
					if(true==pNode->m_kHaveToDelete || ulNow > pNode->m_kEndTime)	//사라질 때
					{
						kDeleteIt = true;
						((*Texture_it).second)->RemoveScreenRect(pNode->m_kRectIndex);
					}
				}
			}
			else	//혹시나 텍스쳐를 못찾았으면
			{
				kDeleteIt = true;	//지우자
			}
		}
		else
		{
			kDeleteIt = true;	//지우자
		}

		if(true==kDeleteIt)
		{
			SAFE_DELETE(pNode);
			it = m_kContNode.erase(it);
			continue;
		}
		++it;
	}

	UpdateRectIndex();
}

void PgScreenEffect::Draw(PgRenderer* pRenderer)
{
	if(0==m_kAccumTimeSaved || m_kContNode.empty()) return;

	ContTexture::const_iterator it = m_kContTexture.begin();
	while(it!=m_kContTexture.end())
	{
		PgUIScene::Render_UIObject(pRenderer,(*it).second);
		++it;
	}
}

void PgScreenEffect::AddNewScreenEffect(std::string const& TexturePath, float const ScreenX, 
		float const ScreenY, int const EffectKeyNo, int const LifeTime, int const AlphaTime)
{
	ContTexture::const_iterator it = m_kContTexture.find(TexturePath);
	NiScreenTexturePtr kPtr;
	if(it==m_kContTexture.end())	//첨 찾는 텍스쳐라면
	{//"../Data/5_effect/9_Tex/EF_screen_break_01.dds" 이런식의 경로가 필요
		NiSourceTexture *pTexture = g_kNifMan.GetTexture(TexturePath.c_str());
		if(pTexture)
		{
			kPtr = NiNew NiScreenTexture(pTexture);
			kPtr->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
			m_kContTexture.insert(std::make_pair(TexturePath, kPtr));
		}
	}
	else
	{
		kPtr = (*it).second;
	}

	if(NULL==kPtr)
	{
		return;
	}

	NiRenderer *pRenderer = NiRenderer::GetRenderer();
	int const kScreenWidth = pRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int const kScreenHeight = pRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	int iCenterX = (int)(kScreenWidth * ScreenX);
	int iCenterY = (int)(kScreenHeight * (1-ScreenY));

	int iPixWidth = kPtr->GetTexture()->GetHeight();
	int iPixHeight = kPtr->GetTexture()->GetWidth();

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
	if(iAdjustX>=iPixWidth || iAdjustY>=iPixHeight) 
	{
		return;
	}

	stScreenEffectNode *pNewNode = new stScreenEffectNode();
	if(NULL==pNewNode)
	{
		return;
	}

	pNewNode->m_kTextureName = TexturePath;
	pNewNode->m_kRectIndex = kPtr->GetNumScreenRects();
	kPtr->AddNewScreenRect(iTop,iLeft,iPixWidth-iAdjustX,iPixHeight-iAdjustY,iAdjustY,iAdjustX);
	pNewNode->m_kCreateTime = BM::GetTime32();
	pNewNode->m_kEndTime = pNewNode->m_kCreateTime + LifeTime;
	pNewNode->m_kAlphaTime = AlphaTime;
	pNewNode->m_kEffectID = EffectKeyNo;

	m_kContNode.push_back(pNewNode);
	kPtr->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
}

void PgScreenEffect::DeleteEffectByNo(int const EffectKeyNo)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContNode::iterator it = m_kContNode.begin();
	while(it!=m_kContNode.end())
	{
		if((*it)->m_kEffectID==EffectKeyNo)
		{
			(*it)->m_kHaveToDelete = true;
		}
		++it;
	}
}

void PgScreenEffect::ClearAll()
{
	ContNode::iterator it = m_kContNode.begin();
	while(it!=m_kContNode.end())
	{
		SAFE_DELETE(*it);
		++it;
	}

	m_kContNode.clear();
}

void PgScreenEffect::Init()
{
	m_kAccumTimeSaved = 0.0f;
}

void PgScreenEffect::Destroy()
{
	ContTexture::iterator it = m_kContTexture.begin();
	while(it!=m_kContTexture.end())
	{
		(*it).second = 0;
		++it;
	}

	m_kContTexture.clear();

	ClearAll();
}

void PgScreenEffect::UpdateRectIndex()
{
	ContNode::iterator it = m_kContNode.begin();
	typedef std::map<std::string, int>	ContPathCount;
	ContPathCount kContPathCount;
	while(it!=m_kContNode.end())
	{
		std::pair<ContPathCount::iterator, bool> ib = kContPathCount.insert(std::make_pair((*it)->m_kTextureName, 0));
		if(!ib.second)	//있는놈이면
		{
			++(*ib.first).second;
		}

		(*it)->m_kRectIndex = (*ib.first).second;
		
		++it;
	}
}

}