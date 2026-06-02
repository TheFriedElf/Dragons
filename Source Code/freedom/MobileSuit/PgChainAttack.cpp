#include "stdafx.h"
#include "Variant/PgComboCounter.h"
#include "PgChainAttack.h"
#include "PgActor.h"
#include "PgNifMan.h"
#include "PgRenderer.H"
#include "PgMobileSuit.H"
#include "PgUIScene.H"

PgChainAttack	g_kChainAttack;

namespace PgChainAttackUtil
{
	int const iOneTimeAddCount = 1;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgChainAttack
////////////////////////////////////////////////////////////////////////////////////////////////////
PgChainAttack::PgChainAttack()
{
}

PgChainAttack::~PgChainAttack()
{
}

bool PgChainAttack::LoadResource(stComboNumNode *pkTargetNode)
{
	NiSourceTexturePtr spNumberTex = m_spNumberTex ? m_spNumberTex : g_kNifMan.GetTexture("../Data/6_UI/main/mnNumCom.dds");
	NiSourceTexturePtr spComboTex = m_spComboTex ? m_spComboTex : g_kNifMan.GetTexture("../Data/6_UI/main/combo.tga");

	pkTargetNode->m_spNumber = NiNew NiScreenTexture(spNumberTex);
	pkTargetNode->m_spComboText = NiNew NiScreenTexture(spComboTex);

	pkTargetNode->m_spNumber->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
	pkTargetNode->m_spComboText->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

	return true;
}
bool PgChainAttack::LoadResource(stRaitingNode *pkTargetNode,bool bCreateExpNode)
{
	NiSourceTexturePtr	spRatingTex = m_spRatingTex ? m_spRatingTex : g_kNifMan.GetTexture("../Data/6_UI/main/comboText.tga");
	NiSourceTexturePtr	spExpTex = m_spExpTex ? m_spExpTex : g_kNifMan.GetTexture("../Data/6_UI/main/expText.dds");

	pkTargetNode->m_spRatingText = NiNew NiScreenElements(NiNew NiScreenElementsData(false, true, 1));
	pkTargetNode->m_spExpText = bCreateExpNode ? (NiNew NiScreenElements(NiNew NiScreenElementsData(false, true, 1))) : NULL;

	pkTargetNode->m_spRatingText->Insert(4);
    pkTargetNode->m_spRatingText->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
    pkTargetNode->m_spRatingText->UpdateBound();
    pkTargetNode->m_spRatingText->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.00f);  
	pkTargetNode->m_spRatingText->SetColors(0,NiColorA(1,1,1,1),NiColorA(1,1,1,1),NiColorA(1,1,1,1),NiColorA(1,1,1,1));

	NiTexturingProperty* pkTexturing = NiNew NiTexturingProperty;
	pkTexturing->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
	pkTexturing->SetBaseTexture(spRatingTex);

	NiMaterialProperty* pkMat = NiNew NiMaterialProperty();
	NiAlphaProperty* pkAlpha = NiNew NiAlphaProperty();
	pkAlpha->SetAlphaBlending(true);
	pkAlpha->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
	pkAlpha->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);

	NiVertexColorProperty* pkVertexColor = NiNew NiVertexColorProperty();
	pkVertexColor->SetSourceMode(NiVertexColorProperty::SOURCE_EMISSIVE);
	pkVertexColor->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
	
	pkTargetNode->m_spRatingText->AttachProperty(pkAlpha);
	pkTargetNode->m_spRatingText->AttachProperty(pkVertexColor);
	pkTargetNode->m_spRatingText->AttachProperty(pkTexturing);
	pkTargetNode->m_spRatingText->AttachProperty(pkMat);
  	pkTargetNode->m_spRatingText->UpdateEffects();
	pkTargetNode->m_spRatingText->UpdateProperties();
    pkTargetNode->m_spRatingText->Update(0.0f);

	if(bCreateExpNode)
	{
		pkTargetNode->m_spExpText->Insert(4);
		pkTargetNode->m_spExpText->SetRectangle(0, 0.0f, 0.0f, 1.0f, 1.0f);
		pkTargetNode->m_spExpText->UpdateBound();
		pkTargetNode->m_spExpText->SetTextures(0, 0, 0.0f, 0.0f, 1.0f, 1.0f);  
		pkTargetNode->m_spExpText->SetColors(0,NiColorA(1,1,1,1),NiColorA(1,1,1,1),NiColorA(1,1,1,1),NiColorA(1,1,1,1));

		pkTexturing = NiNew NiTexturingProperty;
		pkTexturing->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		pkTexturing->SetBaseTexture(spExpTex);

		pkMat = NiNew NiMaterialProperty();
		pkAlpha = NiNew NiAlphaProperty();
		pkAlpha->SetAlphaBlending(true);
		pkAlpha->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
		pkAlpha->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);

		pkVertexColor = NiNew NiVertexColorProperty();
		pkVertexColor->SetSourceMode(NiVertexColorProperty::SOURCE_EMISSIVE);
		pkVertexColor->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
		pkTargetNode->m_spExpText->AttachProperty(pkAlpha);
		pkTargetNode->m_spExpText->AttachProperty(pkVertexColor);
		pkTargetNode->m_spExpText->AttachProperty(pkTexturing);
 		pkTargetNode->m_spExpText->AttachProperty(pkMat);
 		pkTargetNode->m_spExpText->UpdateEffects();
		pkTargetNode->m_spExpText->UpdateProperties();
		pkTargetNode->m_spExpText->Update(0.0f);
	}

	return true;
}
void PgChainAttack::Init()
{
	ClearAll();
	m_spNumberTex = g_kNifMan.GetTexture("../Data/6_UI/main/mnNumCom.dds");
	m_spComboTex = g_kNifMan.GetTexture("../Data/6_UI/main/combo.tga");
	m_spRatingTex = g_kNifMan.GetTexture("../Data/6_UI/main/comboText.tga");
	m_spExpTex = g_kNifMan.GetTexture("../Data/6_UI/main/expText.dds");
}
void PgChainAttack::ClearAll()
{
	m_bUpdateDisplay = false;
	m_iChainAttackCount = 0;
	m_iTargetChainAttackCount = 0;
	m_fRecentChainAttackTime = 0;
	m_fTotalFadeOutTime = 0.2f;
	m_fTotalMovingTime = 0.1f;
	m_iRateIndex = -1;
	m_kContLazyCombo.clear();
	m_kContLazyComboClientFirst.clear();
	ClearAllNode();
}
void PgChainAttack::Destroy()
{
	ClearAllNode();
	m_spNumberTex = NULL;
	m_spComboTex = NULL;
	m_spRatingTex = NULL;
	m_spExpTex = NULL;
}
void	PgChainAttack::ClearAllNode()
{	
	for(ComboNumNodeList::iterator itor = m_ComboNumNodeList.begin(); itor != m_ComboNumNodeList.end(); ++itor)
	{
		SAFE_DELETE(*itor);
	}
	m_ComboNumNodeList.clear();

	for(RatingNodeList::iterator itor = m_RatingNodeList.begin(); itor != m_RatingNodeList.end(); ++itor)
	{
		SAFE_DELETE(*itor);
	}
	m_RatingNodeList.clear();
}
void PgChainAttack::DrawImmediate(PgRenderer *pkRenderer)
{
	for(ComboNumNodeList::iterator itor = m_ComboNumNodeList.begin(); itor != m_ComboNumNodeList.end(); ++itor)
	{
		PgUIScene::Render_UIObject(pkRenderer, (*itor)->m_spNumber);
		PgUIScene::Render_UIObject(pkRenderer, (*itor)->m_spComboText);
	}

	for(RatingNodeList::iterator itor = m_RatingNodeList.begin(); itor != m_RatingNodeList.end(); ++itor)
	{
		PgUIScene::Render_UIObject(pkRenderer, (*itor)->m_spRatingText);
		if((*itor)->m_spExpText)
		{
			PgUIScene::Render_UIObject(pkRenderer, (*itor)->m_spExpText);
		}
	}
}

void	PgChainAttack::FadeOutEveryComboNumNode()
{
	for(ComboNumNodeList::iterator itor = m_ComboNumNodeList.begin(); itor != m_ComboNumNodeList.end(); ++itor)
	{
		if((*itor)->m_iState != 2)	// 이전것들은 퇴장시키자.
		{
			if((*itor)->m_iState == 0)
			{
				(*itor)->m_bAutoFadeOut = true;
			}
			else
			{
				(*itor)->m_iState = 2;
				(*itor)->m_ptStartLoc = (*itor)->m_ptTargetLoc;
				(*itor)->m_ptTargetLoc.y -= 50;
				(*itor)->m_fStartTime = g_pkApp->GetAccumTime();
			}
		}
	}
}
void	PgChainAttack::FadeOutEveryRatingNode()
{
	for(RatingNodeList::iterator itor = m_RatingNodeList.begin(); itor != m_RatingNodeList.end(); ++itor)
	{
		if((*itor)->m_iState != 2)	// 이전것들은 퇴장시키자.
		{
			(*itor)->m_iState = 2;
			(*itor)->m_fStartTime = g_pkApp->GetAccumTime();
		}
	}
}
bool	PgChainAttack::UpdateComboNumNode(stComboNumNode *pkNode,float fAccumeTime)
{
	PG_ASSERT_LOG(pkNode);
	if(!pkNode)
	{
		return false;
	}

	float fElapsedTime = fAccumeTime - pkNode->m_fStartTime;
	NiRenderer* pkRenderer = NiRenderer::GetRenderer();
	int iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);

	switch(pkNode->m_iState)
	{
	case 0:
		{
			//	알파값이 0->1로 증가한다.
			//	좌표가 StartLoc -> TargetLoc 으로 이동한다.
			float fRate = fElapsedTime / m_fTotalMovingTime;
			if(fRate > 1 )
			{
				fRate = 1.0f;
			}
			
			float fAlpha = fRate;
			int	iStartX = pkNode->m_ptStartLoc.x;
			int	iTargetX = pkNode->m_ptTargetLoc.x;
			int	iCurrentX = iStartX + static_cast<int>((iTargetX - iStartX)*fRate);

			PG_ASSERT_LOG(pkNode->m_spNumber);

			int	iNumRect = static_cast<int>(pkNode->m_spNumber->GetNumScreenRects());
			for(int i = 0;i < iNumRect; ++i)
			{
				NiScreenTexture::ScreenRect &kScreenRect = pkNode->m_spNumber->GetScreenRect(i);
				kScreenRect.m_sPixLeft = iCurrentX;
				kScreenRect.m_kColor.a = fAlpha;

				if(kScreenRect.m_sPixLeft>=iScreenWidth)
				{
					kScreenRect.m_usPixWidth = 0;
				}
				else if(kScreenRect.m_sPixLeft+pkNode->m_iNumberWidth[i]>=iScreenWidth)
				{
					kScreenRect.m_usPixWidth = iScreenWidth - kScreenRect.m_sPixLeft - 1;
				}
				else
				{
					kScreenRect.m_usPixWidth = pkNode->m_iNumberWidth[i];
				}

				iCurrentX+=static_cast<int>(kScreenRect.m_usPixWidth*0.7);
			}

			NiScreenTexture::ScreenRect& kScreenRect = pkNode->m_spComboText->GetScreenRect(0);
			kScreenRect.m_sPixLeft = iCurrentX;
			kScreenRect.m_kColor.a = fAlpha;
			if(kScreenRect.m_sPixLeft>=iScreenWidth)
			{
				kScreenRect.m_usPixWidth = 0;
			}
			else if(kScreenRect.m_sPixLeft+130>=iScreenWidth)
			{
				kScreenRect.m_usPixWidth = iScreenWidth - kScreenRect.m_sPixLeft - 1;
			}
			else
			{
				kScreenRect.m_usPixWidth = 130;
			}

			pkNode->m_spNumber->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
			pkNode->m_spComboText->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);

			if(fRate == 1)	//	등장 시간이 종료되었다면, 1번 상태로 전환.
			{
				pkNode->m_fStartTime = fAccumeTime;
				pkNode->m_iState = 1;
			}
		}
		break;
	case 1:
		{
			if(pkNode->m_bAutoFadeOut)
			{
				pkNode->m_iState = 2;
				pkNode->m_ptStartLoc = pkNode->m_ptTargetLoc;
				pkNode->m_ptTargetLoc.y -= 50;
				pkNode->m_fStartTime = fAccumeTime;
			}
		}
		break;
	case 2:
		{
			//	알파가 1->0 으로 변경된다.
			//	좌표가 StartLoc -> TargetLoc 으로 이동된다.
			//	알파가 다 빠지면 삭제된다.
			float fRate = fElapsedTime/m_fTotalFadeOutTime;
			if(fRate > 1 )
			{
				fRate = 1.0f;
			}
			
			float fAlpha = 1 - fRate;

			int	iStartY = pkNode->m_ptStartLoc.y;
			int	iTargetY = pkNode->m_ptTargetLoc.y;
			int	iCurrentY = iStartY + static_cast<int>((iTargetY - iStartY)*fRate);

			PG_ASSERT_LOG(pkNode->m_spNumber);

			int	iNumRect =	static_cast<int>(pkNode->m_spNumber->GetNumScreenRects()); 
		
			for(int i = 0; i < iNumRect; ++i)
			{
				NiScreenTexture::ScreenRect &kScreenRect = pkNode->m_spNumber->GetScreenRect(i);
				kScreenRect.m_sPixTop = iCurrentY;
				kScreenRect.m_kColor.a = fAlpha;
			}

			NiScreenTexture::ScreenRect &kScreenRect = pkNode->m_spComboText->GetScreenRect(0);
			kScreenRect.m_sPixTop = iCurrentY;
			kScreenRect.m_kColor.a = fAlpha;

			pkNode->m_spNumber->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
			pkNode->m_spComboText->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);

			if(fRate == 1)	
			{
				return	false;
			}
		}
		break;
	}

	return	true;
}

bool	PgChainAttack::UpdateRatingNode(stRaitingNode *pkNode,float fAccumeTime)
{
	float fElapsedTime = fAccumeTime - pkNode->m_fStartTime;
	NiRenderer* pkRenderer = NiRenderer::GetRenderer();
	int iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);

	NiMaterialProperty* pkMat;

	switch(pkNode->m_iState)
	{
	case 0:
		{
			//	알파값이 0->1로 증가한다.
			float fRate = fElapsedTime/m_fTotalMovingTime;
			if(fRate > 1 )
			{
				fRate = 1.0f;
			}
			
			float	fAlpha = fRate;

			pkMat = pkNode->m_spRatingText->GetPropertyState()->GetMaterial();
			pkMat->SetAlpha(fAlpha);

			if(pkNode->m_spExpText)
			{
				pkMat = pkNode->m_spExpText->GetPropertyState()->GetMaterial();
				pkMat->SetAlpha(fAlpha);
			}

			ScaleRatingNode(pkNode,fRate);
			pkNode->m_spRatingText->SetColors(0,NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha));
			if(pkNode->m_spExpText)
			{
				fAlpha = pkNode->m_bNoExpTextPlay ? 1.0f : fAlpha;
				pkNode->m_spExpText->SetColors(0,NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha));
			}

			if(fRate == 1)	//	등장 시간이 종료되었다면, 1번 상태로 전환.
			{
				pkNode->m_fStartTime = fAccumeTime;
				pkNode->m_iState = 1;
			}
		}
		break;
	case 1:
		{
		}
		break;
	case 2:
		{
			//	알파가 1->0 으로 변경된다.
			float fRate = fElapsedTime/m_fTotalFadeOutTime;
			if(fRate > 1 )
			{
				fRate = 1.0f;
			}
			
			float fAlpha = 1-fRate;

			pkMat = pkNode->m_spRatingText->GetPropertyState()->GetMaterial();
			pkMat->SetAlpha(fAlpha);
			if(pkNode->m_spExpText)
			{
				pkMat = pkNode->m_spExpText->GetPropertyState()->GetMaterial();
				pkMat->SetAlpha(fAlpha);
			}

			ScaleRatingNode(pkNode,fRate+1);
			pkNode->m_spRatingText->SetColors(0,NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha));
			if(pkNode->m_spExpText)
			{
				pkNode->m_spExpText->SetColors(0,NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha),NiColorA(1,1,1,fAlpha));
			}

			if(fRate == 1)	
			{
				return	false;
			}
		}
		break;
	}


	return	true;
}

void PgChainAttack::Update(float fAccumTime,float fFrameTime)
{
	NiRenderer* pkRenderer = NiRenderer::GetRenderer();
	int iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);

	// 버그 수정된 최신의 콤보 시스템
	if(	0 != m_fRecentChainAttackTime
	&&	g_fComboContinueTime < (fAccumTime - m_fRecentChainAttackTime) )
	{
		//	콤보가 끊어졌다면 기존것을 퇴장시키자.
		FadeOutEveryComboNumNode();
		FadeOutEveryRatingNode();
	}
	else
	{
		if( m_bUpdateDisplay )
		{
			m_bUpdateDisplay = false;
			AddNewComboNumNode(m_iChainAttackCount);
		}
	}

	for(ComboNumNodeList::iterator itor = m_ComboNumNodeList.begin(); itor != m_ComboNumNodeList.end();)
	{
		if(!UpdateComboNumNode(*itor,fAccumTime))
		{
			delete *itor;
			itor = m_ComboNumNodeList.erase(itor);
		}
		else
		{
			++itor;
		}
	}
	for(RatingNodeList::iterator itor = m_RatingNodeList.begin(); itor != m_RatingNodeList.end();)
	{
		if(!UpdateRatingNode(*itor,fAccumTime))
		{
			delete *itor;
			itor = m_RatingNodeList.erase(itor);
		}
		else
		{
			++itor;
		}
	}
}

void PgChainAttack::NewCount(int const iNewCombo, int const iSkillNo)
{
	int const iDiffCount = (iNewCombo - m_iTargetChainAttackCount);
	m_iTargetChainAttackCount = iNewCombo;

	if( 0 == iNewCombo ) // 리셋
	{
		ClearAllNode();
		m_kContLazyCombo.clear();
		m_kContLazyComboClientFirst.clear();
		m_iChainAttackCount = 0;
		m_iRateIndex = -1;
	}
	else
	{
		int const iNormalCombo = 1;
		if( PgChainAttackUtil::iOneTimeAddCount == iDiffCount ) // 이전과 차이가 1이면 먼저 간다
		{
			SetChainAttackCount( m_iChainAttackCount + PgChainAttackUtil::iOneTimeAddCount );
		}
		else
		{
			// 이전과 차이가 아래들과 같으면 클라이언트 연출에 맞춰서 간다
			//	블릿츠 3 
			//	울프 5
			//	애로우 샤워 8
			//	형간다 8
			//	형왓다 3
			//	팔콘 3

			int iAddCount = iDiffCount;
			ContLazyCombo::iterator find_iter = m_kContLazyComboClientFirst.find( iSkillNo );
			if( m_kContLazyComboClientFirst.end() != find_iter )
			{
				// 클라이언트가 미리 뺀 값을 적용하고
				int& iLazyCombo = (*find_iter).second;
				if( iAddCount < abs(iLazyCombo) )
				{
					iLazyCombo = iLazyCombo + iAddCount;
					iAddCount = 0;
				}
				else
				{
					iAddCount = iAddCount + iLazyCombo; // iLazyCombo는 음수
					iLazyCombo = 0;
				}

				if( 0 == iLazyCombo )
				{
					m_kContLazyComboClientFirst.erase(find_iter);
				}
			}

			if( 0 >= iAddCount )
			{
				return;
			}

			std::pair<ContLazyCombo::iterator, bool> kRet = m_kContLazyCombo.insert( std::make_pair(iSkillNo, iAddCount) ); // 남는 값을 기억 시킨다
			if( !kRet.second )
			{
				(*kRet.first).second += iAddCount;
			}
		}
	}
}

void PgChainAttack::IncreaseChainAttackCount(int const iSkillNo)
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	if( !pkSkillDef )
	{
		return;
	}

	if( 0 == pkSkillDef->GetAbil(AT_COMBO_HIT_COUNT) ) // 타격대 콤보가 1:1인 경우
	{
		return;
	}

	ContLazyCombo::iterator find_iter = m_kContLazyCombo.find( iSkillNo );
	if( m_kContLazyCombo.end() != find_iter )
	{
		// 서버 패킷이 먼저 도착한 경우
		// 미리 도착한 카운트를 감소 시키면서, 연출을 증가 시킨다.
		int& iLazyCount = (*find_iter).second;
		if( 0 < iLazyCount )
		{
			//if( LOCAL_MGR::NC_CHINA == g_kLocal.ServiceRegion()
			//&&	0 != pkSkillDef->GetAbil(AT_COMBO_HIT_COUNT)
			//&&	0 == pkSkillDef->GetAbil(AT_COMBO_DELAY) )
			//{
			//	int const iAddedCount = pkSkillDef->GetAbil(AT_COMBO_HIT_COUNT);
			//	iLazyCount -= iAddedCount; // 내부 감소
			//	SetChainAttackCount( m_iChainAttackCount + iAddedCount ); // 지연된 표시 증가
			//}
			//else
			{
				iLazyCount -= PgChainAttackUtil::iOneTimeAddCount; // 내부 감소
				SetChainAttackCount( m_iChainAttackCount + PgChainAttackUtil::iOneTimeAddCount ); // 지연된 표시 증가
			}
		}

		if( 0 == iLazyCount )
		{
			m_kContLazyCombo.erase(find_iter);
		}
	}
	else
	{
		// 클라 연출이 먼저 간 경우 (서버로 부터 패킷이 늦는다)
		// 미리 연출을 증가 시키고, 패킷이 도착시 값을 감소 시킨다.
		find_iter = m_kContLazyComboClientFirst.find( iSkillNo );
		if( m_kContLazyComboClientFirst.end() == find_iter )
		{
			std::pair<ContLazyCombo::iterator, bool> kRet = m_kContLazyComboClientFirst.insert( std::make_pair(iSkillNo, 0) );
			if( !kRet.second )
			{
				return;
			}
			find_iter = kRet.first;
		}

		int& iLazyCount = (*find_iter).second;

		//if( LOCAL_MGR::NC_CHINA == g_kLocal.ServiceRegion()
		//&&	0 != pkSkillDef->GetAbil(AT_COMBO_HIT_COUNT)
		//&&	0 == pkSkillDef->GetAbil(AT_COMBO_DELAY) )
		//{
		//	int const iAddedCount = pkSkillDef->GetAbil(AT_COMBO_HIT_COUNT);
		//	iLazyCount -= iAddedCount; // 내부 감소
		//	SetChainAttackCount( m_iChainAttackCount + iAddedCount ); // 지연된 표시 증가
		//}
		//else
		{
			--iLazyCount; // 음수로 간다
			SetChainAttackCount( m_iChainAttackCount + PgChainAttackUtil::iOneTimeAddCount ); // 미리 표시 증가
		}
	}
}

//! Chain Attack Count 를 설정.
void PgChainAttack::SetChainAttackCount(int iCount)
{
	m_fRecentChainAttackTime = g_pkApp->GetAccumTime();
	m_iChainAttackCount = iCount;

	int const iMinimumDisplayCount = 2;
	if( iMinimumDisplayCount <= iCount )
	{
		m_bUpdateDisplay = true;
	}
}

void PgChainAttack::AddNewRatingNode(int iChainCount)
{
	iChainCount = std::min(100, iChainCount);
	int iRatingIndex = ((iChainCount - (iChainCount % 10)) / 10) - 1;
	if( m_iRateIndex == iRatingIndex )
	{
		return;
	}
	m_iRateIndex = iRatingIndex;
	if( 0 > m_iRateIndex )
	{
		return;
	}

	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	int iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	int	iRatingTexLoc[]={
		140,0,108,32,	//	x,y,w,h
		130,32,118,35,
		149,67,99,36,
		117,103,131,35,
		79,138,169,34,
		50,172,198,36,
		86,208,162,35,
		65,243,183,36,
		0,279,248,34,
		63,313,185,37
	};
	int	iExpTexLoc[]={
		0,0,150,40,	//	x,y,w,h
		0,40,150,40,
		0,80,150,40,
		0,120,150,40,
		0,160,150,40,
		0,200,150,40,
		0,240,150,40,
		0,280,150,40,
		0,320,150,40,
		0,360,150,40
	};

	int	iExpIndex = -1;
	if(iRatingIndex>0)
	{
		iExpIndex = (iRatingIndex+1)/2-1;
	}

	float fRatingTexWidth = 248,fRatingTexHeight = 350;
	float fExpTexWidth = 256,fExpTexHeight = 512;

	stRaitingNode* pkNewNode = new stRaitingNode();
	LoadResource(pkNewNode,(iExpIndex>=0));

	pkNewNode->m_iState = 0;
	pkNewNode->m_fStartTime = g_pkApp->GetAccumTime();
	pkNewNode->m_iRatingIndex = iRatingIndex;

	POINT ptTargetLoc;
	ptTargetLoc.x = iScreenWidth - 258;
	ptTargetLoc.y = static_cast<long>(iScreenHeight*0.4+50);

	float fScreenX = ptTargetLoc.x/((float)iScreenWidth);
	float fScreenY = ptTargetLoc.y/((float)iScreenHeight);
	float fWidth = iRatingTexLoc[iRatingIndex*4+2]/((float)iScreenWidth);
	float fHeight = iRatingTexLoc[iRatingIndex*4+3]/((float)iScreenHeight);
	float fTexLeft = iRatingTexLoc[iRatingIndex*4+0]/fRatingTexWidth;
	float fTexTop = iRatingTexLoc[iRatingIndex*4+1]/fRatingTexHeight;
	float fTexWidth = iRatingTexLoc[iRatingIndex*4+2]/fRatingTexWidth;
	float fTexHeight = iRatingTexLoc[iRatingIndex*4+3]/fRatingTexHeight;

	pkNewNode->m_spRatingText->SetRectangle(0,fScreenX,fScreenY,fWidth,fHeight);
	pkNewNode->m_spRatingText->SetTextures(0,0,fTexLeft,fTexTop,fTexLeft+fTexWidth,fTexTop+fTexHeight);

	if(iExpIndex>=0)
	{
		ptTargetLoc.x = iScreenWidth - iRatingTexLoc[iRatingIndex*4+2]-20;
		ptTargetLoc.y = static_cast<long>(iScreenHeight*0.4+90);

		fScreenX = ptTargetLoc.x/((float)iScreenWidth);
		fScreenY = ptTargetLoc.y/((float)iScreenHeight);
		fWidth = iExpTexLoc[iExpIndex*4+2]/((float)iScreenWidth);
		fHeight = iExpTexLoc[iExpIndex*4+3]/((float)iScreenHeight);
		fTexLeft = iExpTexLoc[iExpIndex*4+0]/fExpTexWidth;
		fTexTop = iExpTexLoc[iExpIndex*4+1]/fExpTexHeight;
		fTexWidth = iExpTexLoc[iExpIndex*4+2]/fExpTexWidth;
		fTexHeight = iExpTexLoc[iExpIndex*4+3]/fExpTexHeight;

		pkNewNode->m_spExpText->SetRectangle(0,fScreenX,fScreenY,fWidth,fHeight);
		pkNewNode->m_spExpText->SetTextures(0,0,fTexLeft,fTexTop,fTexLeft+fTexWidth,fTexTop+fTexHeight);
	}

	if((iRatingIndex+1)%2 != 0 && false == m_RatingNodeList.empty())
	{
		stRaitingNode	*pkLastNode = m_RatingNodeList.back();
		if(pkLastNode)
		{
			pkLastNode->m_spExpText = 0;
		}

		pkNewNode->m_bNoExpTextPlay = true;
	}

	FadeOutEveryRatingNode();

	m_RatingNodeList.push_back(pkNewNode);
}
void	PgChainAttack::ScaleRatingNode(stRaitingNode *pkNode,float fScale)
{
	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	int iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	int	iRatingTexLoc[]={
		140,0,108,32,	//	x,y,w,h
		130,32,118,35,
		149,67,99,36,
		117,103,131,35,
		79,138,169,34,
		50,172,198,36,
		86,208,162,35,
		65,243,183,36,
		0,279,248,34,
		63,313,185,37
	};
	int	iExpTexLoc[]={
		0,0,150,40,	//	x,y,w,h
		0,40,150,40,
		0,80,150,40,
		0,120,150,40,
		0,160,150,40,
		0,200,150,40,
		0,240,150,40,
		0,280,150,40,
		0,320,150,40,
		0,360,150,40
	};

	float fRatingTexWidth = 248,fRatingTexHeight = 350;
	float fExpTexWidth = 256,fExpTexHeight = 512;

	int iRatingIndex = pkNode->m_iRatingIndex;

	POINT ptTargetLoc;
	ptTargetLoc.x = iScreenWidth - iRatingTexLoc[iRatingIndex*4+2]-20;
	ptTargetLoc.y = static_cast<long>(iScreenHeight*0.4+50);

	float fScreenX = ptTargetLoc.x/((float)iScreenWidth);
	float fScreenY = ptTargetLoc.y/((float)iScreenHeight);
	float fWidth = iRatingTexLoc[iRatingIndex*4+2]/((float)iScreenWidth);
	float fHeight = iRatingTexLoc[iRatingIndex*4+3]/((float)iScreenHeight);


	pkNode->m_spRatingText->SetRectangle(0,
		(float)(fScreenX-(fWidth*fScale-fWidth)*0.5),
		(float)(fScreenY-(fHeight*fScale-fHeight)*0.5),
		(float)(fWidth*fScale),
		(float)(fHeight*fScale));

	if(pkNode->m_spExpText)
	{
		fScale = pkNode->m_bNoExpTextPlay ? 1.0f : fScale;

		ptTargetLoc.x = iScreenWidth - 160;
		ptTargetLoc.y = static_cast<long>(iScreenHeight*0.4+90);

		fScreenX = ptTargetLoc.x/((float)iScreenWidth);
		fScreenY = ptTargetLoc.y/((float)iScreenHeight);
		fWidth = iExpTexLoc[iRatingIndex*4+2]/((float)iScreenWidth);
		fHeight = iExpTexLoc[iRatingIndex*4+3]/((float)iScreenHeight);

		pkNode->m_spExpText->SetRectangle(0,
			(float)(fScreenX-(fWidth*fScale-fWidth)*0.5),
			(float)(fScreenY-(fHeight*fScale-fHeight)*0.5),
			(float)(fWidth*fScale),
			(float)(fHeight*fScale));
	}
}


void	PgChainAttack::AddNewComboNumNode(int iChainCount)
{
	if(iChainCount<=0)
	{
		return;
	}

	NiRenderer* pkRenderer = NiRenderer::GetRenderer();
	int iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	int iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);
	
	int	iNumTexLoc[]={
		6,0,38,47,	//	x,y,width,height //0
		62,0,24,47,		//1
		107,0,35,47,	//2
		157,0,37,47,	//3
		205,0,41,47,	//4
		6,50,36,47,		//5
		57,50,37,47,	//6
		107,50,36,47,	//7
		157,50,35,47,	//8
		207,50,36,47	//9
	};

	AddNewRatingNode(iChainCount); // 10 ~ 100 까지 10단위로 들어와야 한다

	/*stComboNumNode	*pkLastNode = NULL;
	if(m_ComboNumNodeList.size()>0)
	{
		pkLastNode = *m_ComboNumNodeList.begin();
		if(pkLastNode->m_fStartTime == g_pkApp->GetAccumTime())
		{
			return;
		}
	}*/


	stComboNumNode* pkNewNode = new stComboNumNode();
	LoadResource(pkNewNode);

	pkNewNode->m_fStartTime = g_pkApp->GetAccumTime();
	pkNewNode->m_iState = 0;

	FadeOutEveryComboNumNode();

	m_ComboNumNodeList.push_back(pkNewNode);

	char numbers[100];
	sprintf_s(numbers,100,"%d",iChainCount);

	int	iNumberWidth = 0;
	int	iNumberIndex;
	int	iNumberSpace = 0;
	int	iNumLen = strlen(numbers);
	for(int i = 0; i < iNumLen; ++i)
	{
		iNumberIndex = numbers[i]-'0';

		PG_ASSERT_LOG(iNumberIndex>=0 && iNumberIndex<=9);

		pkNewNode->m_iNumberWidth[i] = iNumTexLoc[iNumberIndex*4+2]+iNumberSpace;

		iNumberWidth+=static_cast<int>(pkNewNode->m_iNumberWidth[i]*0.7);
	}

	POINT ptTargetLoc;

	ptTargetLoc.x = iScreenWidth - 130 - iNumberWidth;
	ptTargetLoc.y = static_cast<long>(iScreenHeight*0.4);

	pkNewNode->m_ptTargetLoc = ptTargetLoc;

	ptTargetLoc.x = iScreenWidth-120;

	pkNewNode->m_ptStartLoc = ptTargetLoc;

	for(int i = 0; i < iNumLen; ++i)
	{
		iNumberIndex = numbers[i]-'0';
		PG_ASSERT_LOG(iNumberIndex>=0 && iNumberIndex<=9);

		pkNewNode->m_spNumber->AddNewScreenRect(
			(short)ptTargetLoc.y,
			(short)ptTargetLoc.x,
			iNumTexLoc[iNumberIndex*4+2],iNumTexLoc[iNumberIndex*4+3],
			iNumTexLoc[iNumberIndex*4+1],iNumTexLoc[iNumberIndex*4+0]);

		ptTargetLoc.x+=pkNewNode->m_iNumberWidth[i];
	}

	pkNewNode->m_spComboText->AddNewScreenRect((short)(ptTargetLoc.y+10),(short)(ptTargetLoc.x),130,40,0,0);



}