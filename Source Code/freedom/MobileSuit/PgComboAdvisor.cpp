#include <stdafx.h>
#include "PgComboAdvisor.h"
#include "PgNifMan.H"
#include "PgPilotMan.H"
#include "PgPilot.H"
#include "PgActor.H"
#include "PgMobileSuit.H"
#include "PgUIScene.H"
#include "lwActor.H"
#include "PgAction.h"
#include "PgOption.h"


RECT const kRectHitEffect[4] = 
{
	{0,0,110,110},
	{110,0,220,110},
	{0,110,110,220},
	{110,110,220,220}
};

PgComboAdvisor	g_kComboAdvisor;

void	PgComboAdvisor::Destroy()
{
	m_spComboTexture = 0;
	m_spComboEffectTexture = 0;
	m_spMaterialProperty=0;
	m_spAlphaProperty=0;
	m_spTexturingProperty=0;
	m_spTexturingProperty_Effect=0;
	m_spVertexColorProperty=0;
	m_iHitEffectIndex = 0;
	m_iAlwaysBlink = false;
	ResetComboAdvisor();
	m_vIconData.clear();

	NodeList::iterator iter_node = m_NodeList.begin();
	while(m_NodeList.end() != iter_node )
	{
		(*iter_node)->ClearChilds();
		++iter_node;
	}
	m_NodeList.clear();
}
void PgComboAdvisor::Update(float fAccumTime,float fFrameTime)
{
	stComboTreeNode	*pkNode = NULL;
	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end();)
	{
		pkNode = *itor;
		UpdateComboNode(pkNode,fAccumTime,fFrameTime);

		if(pkNode->m_fStartScale == 0.0f)
		{
			SAFE_DELETE(pkNode);
			itor = m_NodeList.erase(itor);
			continue;
		}
		itor++;
	}

	if(pkNode)
	{
		stComboTreeNode	*pkChild = NULL;
		for(int i=0;i<pkNode->m_iChildCount;i++)
		{
			pkChild = pkNode->m_pkChilds[i];
			UpdateComboNode(pkChild,fAccumTime,fFrameTime);		
		}
	}

}
void	PgComboAdvisor::UpdateComboNode(stComboTreeNode *pkNode,float fAccumTime,float fFrameTime)
{
	if(pkNode->m_State == CTNS_NEXT)
	{
		NiColorA	kColor;
		if(pkNode->m_spScreenElement[0])
		{
			pkNode->m_spScreenElement[0]->GetColor(0,0,kColor);
			if( pkNode->m_bEnableInputKey )
			{
				if(kColor.a == 0.5)
				{
					kColor.a = 1;
				}
				else
				{
					kColor.a = 0.5;
				}
			}
			else
			{
				kColor.a = 0.5;
			}
			pkNode->m_spScreenElement[0]->SetColors(0,kColor,kColor,kColor,kColor);
			pkNode->m_spScreenElement[1]->SetColors(0,kColor,kColor,kColor,kColor);

			pkNode->m_spScreenElement[0]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);
			pkNode->m_spScreenElement[1]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);


			pkNode->m_spScreenElement[0]->Update(0);
			pkNode->m_spScreenElement[1]->Update(0);
			
			kColor.a = 0.0f;
			pkNode->m_spScreenElement[2]->SetColors(0,kColor,kColor,kColor,kColor);
			pkNode->m_spScreenElement[2]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);
			pkNode->m_spScreenElement[2]->Update(0);

			if( pkNode->m_bEnableInputKey )
			{
				if(pkNode->m_fTargetScale == pkNode->m_fStartScale)
				{
					if(pkNode->m_fStartScale == 0.8)
					{
						pkNode->m_fStartScale = 0.8f;
						pkNode->m_fTargetScale = 1.2f;
					}
					else
					{
						pkNode->m_fStartScale = 1.2f;
						pkNode->m_fTargetScale = 0.8f;
					}

					pkNode->m_fScaleStartTime = fAccumTime;
				}
			}
		}
	}
	else if(pkNode->m_State == CTNS_CURRENT)
	{
		float	fElapsedTime = fAccumTime - pkNode->m_fCreationTime;
		float	fDestroyTime = 3.0f;
		if( pkNode->m_bCompleteCombo )
		{
			fDestroyTime = 1.0f;
		}
		if( 1 == g_kGlobalOption.GetValue("ETC", "USE_OLD_COMBO") )
		{//올드 콤보 삭제 타임은 예전과 동일
			fDestroyTime = 1.0f;
		}
		if(pkNode->m_iChildCount>0 && fElapsedTime>fDestroyTime)
		{
			pkNode->ClearChilds();
		}
		if( fElapsedTime>fDestroyTime )
		{
			NodeList::iterator iter_Node = m_NodeList.begin();
			while( m_NodeList.end() != iter_Node )
			{
				(*iter_Node)->m_fStartScale = 0.0f;
				(*iter_Node)->m_fTargetScale = 0.0f;
				++iter_Node;
			};
			return;
		}

		if( 0.2f > fElapsedTime || true==m_iAlwaysBlink)
		{
			if( pkNode->m_spScreenElement[2] )
			{
				if( 4 == m_iHitEffectIndex )
				{
					m_iHitEffectIndex = 0;
				}
				float	fTexWidth = (float)m_spComboEffectTexture->GetWidth();
				float	fTexHeight =(float)m_spComboEffectTexture->GetHeight();

				pkNode->m_spScreenElement[2]->SetTextures(0,0, 
					kRectHitEffect[m_iHitEffectIndex].left	/ fTexWidth, 
					kRectHitEffect[m_iHitEffectIndex].top	/ fTexHeight, 
					kRectHitEffect[m_iHitEffectIndex].right / fTexWidth, 
					kRectHitEffect[m_iHitEffectIndex].bottom/fTexHeight);
				pkNode->m_spScreenElement[2]->Update(0);
				++m_iHitEffectIndex;
			}
		}
		else
		{
			NiColorA kColor;
			pkNode->m_spScreenElement[0]->GetColor(0,0,kColor);
			kColor.a = 0;
			pkNode->m_spScreenElement[2]->SetColors(0,kColor,kColor,kColor,kColor);
			pkNode->m_spScreenElement[2]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);
			pkNode->m_spScreenElement[2]->Update(0);
		}
	}
	else if(pkNode->m_State == CTNS_PASSED)
	{
		if( 0.0f == pkNode->m_fStartScale )
		{
			return;
		}
		float	fElapsedTime = fAccumTime - pkNode->m_fCreationTime;
		float	fDestroyTime = 10.0f;
		if( 1 == g_kGlobalOption.GetValue("ETC", "USE_OLD_COMBO") )
		{//올드 콤보 삭제 타임은 예전과 동일
			fDestroyTime = 1.0f;
		}
		if(pkNode->m_iChildCount>0 && fElapsedTime>fDestroyTime)
		{
			pkNode->ClearChilds();
		}

		NiColorA kColor;
		pkNode->m_spScreenElement[0]->GetColor(0,0,kColor);
		if(kColor.a == 1)
		{
			kColor.a = 0.5;

			pkNode->m_spScreenElement[0]->SetColors(0,kColor,kColor,kColor,kColor);
			pkNode->m_spScreenElement[1]->SetColors(0,kColor,kColor,kColor,kColor);

			pkNode->m_spScreenElement[0]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);
			pkNode->m_spScreenElement[1]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);

			pkNode->m_spScreenElement[0]->Update(0);
			pkNode->m_spScreenElement[1]->Update(0);
		}

		kColor.a = 0.0f;
		pkNode->m_spScreenElement[2]->SetColors(0,kColor,kColor,kColor,kColor);
		pkNode->m_spScreenElement[2]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);
		pkNode->m_spScreenElement[2]->Update(0);

	}

	if(pkNode->m_fStartScale != pkNode->m_fTargetScale)
	{
		float	fTotalScaleTime = 0.1f;
		float	fElapsedTime = fAccumTime-pkNode->m_fScaleStartTime;
		float	fRate = fElapsedTime/fTotalScaleTime;
		if(fRate>1) 
		{
			fRate = 1;
		}

		float	fNextScale = pkNode->m_fStartScale+(pkNode->m_fTargetScale-pkNode->m_fStartScale)*fRate;

		if(fRate == 1)
		{
			pkNode->m_fStartScale = pkNode->m_fTargetScale;
		}

		if(pkNode->m_spScreenElement[0])
		{
			if(pkNode->m_fTargetScale == 0)
			{
				NiColorA kColor(1,1,1,(1.0f-fRate)*0.5f);

				pkNode->m_spScreenElement[0]->SetColors(0,kColor,kColor,kColor,kColor);
				pkNode->m_spScreenElement[1]->SetColors(0,kColor,kColor,kColor,kColor);
				if( pkNode->m_State == CTNS_CURRENT )
				{
					pkNode->m_spScreenElement[2]->SetColors(0,kColor,kColor,kColor,kColor);
				}
				else
				{
					NiColorA kColor(0,0,0,0.0f);
					pkNode->m_spScreenElement[2]->SetColors(0,kColor,kColor,kColor,kColor);
				}

				pkNode->m_spScreenElement[0]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);
				pkNode->m_spScreenElement[1]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);
				pkNode->m_spScreenElement[2]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);
			}

			pkNode->m_spScreenElement[0]->SetScale(fNextScale);
			pkNode->m_spScreenElement[1]->SetScale(fNextScale);
			pkNode->m_spScreenElement[2]->SetScale(fNextScale);

			pkNode->m_spScreenElement[0]->Update(0);
			pkNode->m_spScreenElement[1]->Update(0);
			pkNode->m_spScreenElement[2]->Update(0);
		}
	}
	if(pkNode->m_bCompleteCombo)
	{
		if(pkNode->m_fTargetScale == pkNode->m_fStartScale)
		{
			if(pkNode->m_fStartScale == 0.8)
			{
				pkNode->m_fStartScale = 0.8f;
				pkNode->m_fTargetScale = 1.2f;
			}
			else
			{
				pkNode->m_fStartScale = 1.2f;
				pkNode->m_fTargetScale = 0.8f;
			}
			pkNode->m_fScaleStartTime = fAccumTime;
		}
	}
}

void PgComboAdvisor::DrawImmediate(PgRenderer *pkRenderer)
{
	PgActor	*pkPlayerActor = g_kPilotMan.GetPlayerActor();
	if(!pkPlayerActor) return;

	bool	bActorLeft = false;
	NiPoint3	kLookingDir = pkPlayerActor->GetLookingDir();
	bActorLeft = ((pkPlayerActor->GetPathNormal().Cross(kLookingDir).z>0) ? true : false);

	stComboTreeNode	*pkNode = NULL;
	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end(); ++itor)
	{
		pkNode = *itor;
		if( pkNode->m_State == CTNS_CURRENT )
		{
			PgUIScene::Render_UIObject(pkRenderer,pkNode->m_spScreenElement[2]);
		}
		PgUIScene::Render_UIObject(pkRenderer,pkNode->m_spScreenElement[pkNode->m_iScreenElementIndex]);
	}

	if(pkNode)
	{
		stComboTreeNode	*pkChild = NULL;
		for(int i=0;i<pkNode->m_iChildCount;++i)
		{
			pkChild = pkNode->m_pkChilds[i];

			int	iScreenElementIndex = 0;
			if (bActorLeft == false) 
			{
				iScreenElementIndex = 1;
			}
			PgUIScene::Render_UIObject(pkRenderer,pkChild->m_spScreenElement[2]);
			PgUIScene::Render_UIObject(pkRenderer,pkChild->m_spScreenElement[iScreenElementIndex]);
		}
	}
}

void	PgComboAdvisor::AddNextAction(char const* strActionID)
{
	if(m_NodeList.size() == 0) return;
	if(NULL == strActionID || strlen(strActionID) == 0) return;

	//	캐릭터 레벨이 10이상이면 이 기능을 사용하지 않는다.
	PgPilot	*pkPlayer = g_kPilotMan.GetPlayerPilot();
	if(pkPlayer)
	{
		PgPilot* pkPilot = g_kPilotMan.GetPlayerPilot();
		if( pkPilot )
		{
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			int const iActionID = kSkillDefMgr.GetSkillNum( UNI(strActionID) );
			if( !pkPilot->IsHaveComboAction(iActionID) )
			{
				if(pkPlayer->GetAbil(AT_LEVEL)>=10)	
				{
					return;
				}
			}
			else if( 1 == g_kGlobalOption.GetValue("ETC", "USE_OLD_COMBO") )
			{//신규 기본콤보 기능이 꺼져있으면 사용하지 않는다.
				return;
			}
		}
	}

	PgActor	*pkPlayerActor = g_kPilotMan.GetPlayerActor();
	if(!pkPlayerActor) return;

	if(PgAction::CheckCanEnter(pkPlayerActor,strActionID,false) == false)
	{
		return;
	}

	stComboTreeNode	*pkLastNode = m_NodeList.back();

	if(pkLastNode->m_iChildCount+1>MAX_COMBO_NODE_CHILD) return;

	NiScreenElementsPtr	spElement[3];

	spElement[0] = CreateScreenElement(std::string(strActionID),"LEFT");
	spElement[1] = CreateScreenElement(std::string(strActionID),"RIGHT");
	spElement[2] = CreateScreenElement(std::string(strActionID),"HIT");

	if(spElement[0] == NULL || spElement[1] == NULL || spElement[2] == NULL) return;

	stComboTreeNode *pkNewNode = new stComboTreeNode();

	pkNewNode->m_pkParent = pkLastNode;
	pkNewNode->m_kActionID = std::string(strActionID);
	pkNewNode->m_iChildIndex = pkLastNode->m_iChildCount;
	pkNewNode->m_spScreenElement[0] = spElement[0];
	pkNewNode->m_spScreenElement[1] = spElement[1];
	pkNewNode->m_spScreenElement[2] = spElement[2];
	pkNewNode->m_fCreationTime = g_pkApp->GetAccumTime();

	pkLastNode->m_pkChilds[pkLastNode->m_iChildCount++] = pkNewNode;

	stComboTreeNode	*pkChild = NULL;
	for(int i=0;i<pkLastNode->m_iChildCount;i++)
	{
		pkChild = pkLastNode->m_pkChilds[i];
		SetComboNodeState(pkChild,CTNS_NEXT,NULL);		
	}	
}
void	PgComboAdvisor::ClearNextAction()
{
	if(m_NodeList.size() == 0) return;

	stComboTreeNode	*pkLastNode = m_NodeList.back();

	pkLastNode->ClearChilds();
}

void	PgComboAdvisor::OnNewActionEnter(char const* strActionID, bool const bAlwaysBlink)
{
	PgPilot	*pkPlayer = g_kPilotMan.GetPlayerPilot();
	if(pkPlayer)
	{
		PgActor* pkActor = g_kPilotMan.GetPlayerActor();
		if( !pkActor)
		{
			return;
		}
		PgPilot* pkPilot = pkActor->GetPilot();
		if( !pkPilot )
		{
			return;
		}
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		int const iActionID = kSkillDefMgr.GetSkillNum( UNI(strActionID) );
		int const iWeaponType = pkActor->GetEquippedWeaponType();
		if( !pkPilot->IsHaveComboAction(iActionID) &&
			!pkPilot->IsHaveConnectComboAction(iWeaponType, iActionID ) )
		{
			if(pkPlayer->GetAbil(AT_LEVEL)>=10)	
			{//	캐릭터 레벨이 10이상이면 이 기능을 사용하지 않는다.
				return;
			}
		}
		else if( 1 == g_kGlobalOption.GetValue("ETC", "USE_OLD_COMBO") )
		{//신규 기본콤보 기능이 꺼져있으면 사용하지 않는다.
			return;
		}
	}
	m_iHitEffectIndex = 0; 
	m_iAlwaysBlink = bAlwaysBlink;
	ClearNextAction();

	NiScreenElementsPtr	spElement[3];
	spElement[0] = CreateScreenElement(std::string(strActionID),"LEFT");
	spElement[1] = CreateScreenElement(std::string(strActionID),"RIGHT");
	spElement[2] = CreateScreenElement(std::string(strActionID),"HIT");

	if(spElement[0] == NULL || spElement[1] == NULL || spElement[2] == NULL) return;

	stComboTreeNode	*pkPrevNode = NULL;
	if(m_NodeList.size()>0)
	{
		pkPrevNode = m_NodeList.back();
	}

	stComboTreeNode	*pkNewNode = new stComboTreeNode();

	pkNewNode->m_kActionID = std::string(strActionID);
	pkNewNode->m_spScreenElement[0] = spElement[0];
	pkNewNode->m_spScreenElement[1] = spElement[1];
	pkNewNode->m_spScreenElement[2] = spElement[2];
	pkNewNode->m_fCreationTime = g_pkApp->GetAccumTime();

	PgActor	*pkPlayerActor = g_kPilotMan.GetPlayerActor();
	if(!pkPlayerActor) 
	{
		return;
	}
	bool	bActorLeft = false;
	NiPoint3	kLookingDir = pkPlayerActor->GetLookingDir();
	bActorLeft = ((pkPlayerActor->GetPathNormal().Cross(kLookingDir).z>0) ? true : false);
	if( bActorLeft )
	{
		pkNewNode->m_iScreenElementIndex = 0;
	}
	else
	{
		pkNewNode->m_iScreenElementIndex = 1;
	}

	SetComboNodeState(pkNewNode,CTNS_CURRENT,pkPrevNode);

	m_NodeList.push_back(pkNewNode);

	if(m_NodeList.size() > MAX_COMBO_NODE)
	{
		SAFE_DELETE(*m_NodeList.begin());
		m_NodeList.erase(m_NodeList.begin());
	}

}
void	PgComboAdvisor::ResetComboAdvisor()
{
	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end(); itor++)
	{
		SAFE_DELETE(*itor);
	}
	m_NodeList.clear();
}

void PgComboAdvisor::ChangeChildNode(bool const bEnableInputKey)
{
	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end(); itor++)
	{
		for( int iCount = 0; iCount < (*itor)->m_iChildCount; ++iCount )
		{
			stComboTreeNode *pChildNode = (*itor)->m_pkChilds[iCount];
			if( !pChildNode )
			{
				break;
			}
			if( CTNS_NEXT == pChildNode->m_State )
			{
				pChildNode->m_bEnableInputKey = bEnableInputKey;
			}
		}
	}
}

void PgComboAdvisor::CompleteNode(bool const bCompleteCombo)
{
	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end(); itor++)
	{
		(*itor)->m_bCompleteCombo = bCompleteCombo;
	}
}

void PgComboAdvisor::DeleteChildNode()
{
	for(NodeList::iterator itor = m_NodeList.begin(); itor != m_NodeList.end(); itor++)
	{
		(*itor)->ClearChilds();
	}
}

void	PgComboAdvisor::SetComboNodeState(stComboTreeNode *pkNode,ComboTreeNodeState kState,stComboTreeNode *pkPrevNode)
{
	pkNode->m_State = kState;

	if(kState == CTNS_DISABLED) 
	{
		return;
	}

	SetComboNodeScale(pkNode, kState);
	SetComboNodePos(pkNode, kState, pkPrevNode);

	if( CTNS_CURRENT == kState )
	{
		NodeList::reverse_iterator riter_Node = m_NodeList.rbegin();
		while( m_NodeList.rend() != riter_Node )
		{
			if( pkPrevNode && (*riter_Node) != pkNode )
			{
				SetComboNodeState((*riter_Node), CTNS_PASSED, pkPrevNode);
			}
			pkPrevNode = (*riter_Node);
			++riter_Node;
		}
	}

	float	fAlpha = 1;
	if(kState == CTNS_NEXT)
	{
		fAlpha = 0.5;
	}

	pkNode->m_spScreenElement[0]->SetColors(0,
		NiColorA(1,1,1,fAlpha),
		NiColorA(1,1,1,fAlpha),
		NiColorA(1,1,1,fAlpha),
		NiColorA(1,1,1,fAlpha));
	pkNode->m_spScreenElement[1]->SetColors(0,
		NiColorA(1,1,1,fAlpha),
		NiColorA(1,1,1,fAlpha),
		NiColorA(1,1,1,fAlpha),
		NiColorA(1,1,1,fAlpha));

	pkNode->m_spScreenElement[0]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);
	pkNode->m_spScreenElement[1]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);

	pkNode->m_spScreenElement[0]->Update(0);
	pkNode->m_spScreenElement[1]->Update(0);
	
	if( kState == CTNS_CURRENT )
	{
		pkNode->m_spScreenElement[2]->SetColors(0,
			NiColorA(1,1,1,fAlpha),
			NiColorA(1,1,1,fAlpha),
			NiColorA(1,1,1,fAlpha),
			NiColorA(1,1,1,fAlpha));
		pkNode->m_spScreenElement[2]->GetModelData()->MarkAsChanged(NiGeometryData::COLOR_MASK);
		pkNode->m_spScreenElement[2]->Update(0);
	}

}

void	PgComboAdvisor::SetComboNodeScale(stComboTreeNode *pkNode,ComboTreeNodeState kState)
{
	//노드 아이콘 크기
	if(kState == CTNS_CURRENT )
	{
		pkNode->m_fStartScale = 2.0f;
		pkNode->m_fTargetScale = 1.2f;
	}
	else if(kState == CTNS_PASSED)
	{
		pkNode->m_fStartScale = 1.2f;
		pkNode->m_fTargetScale = 0.8f;
	}
	else if(kState == CTNS_NEXT)
	{
		pkNode->m_fStartScale = 1.2f;
		pkNode->m_fTargetScale = 0.8f;
	}
	else
	{
		pkNode->m_fStartScale = 1.2f;
		pkNode->m_fTargetScale = 0.8f;
	}
	pkNode->m_fScaleStartTime = g_pkApp->GetAccumTime();
}

void	PgComboAdvisor::SetComboNodePos(stComboTreeNode *pkNode,ComboTreeNodeState kState, stComboTreeNode *pkPrevNode)
{
//노드 위치
	float	fScreenWidth = static_cast<float>(NiRenderer::GetRenderer()->GetDefaultBackBuffer()->GetWidth());
	float	fScreenHeight = static_cast<float>(NiRenderer::GetRenderer()->GetDefaultBackBuffer()->GetHeight());
	NiPoint3	kStartPoint(0.5f,0.8f,0.0f);
	if(kState == CTNS_CURRENT)
	{
		pkNode->m_spScreenElement[0]->SetTranslate(kStartPoint);
		pkNode->m_spScreenElement[1]->SetTranslate(kStartPoint);
		pkNode->m_spScreenElement[2]->SetTranslate(kStartPoint);

		pkNode->m_spScreenElement[0]->Update(0);
		pkNode->m_spScreenElement[1]->Update(0);
		pkNode->m_spScreenElement[2]->Update(0);
	}
	else if(kState == CTNS_NEXT)
	{
		stComboTreeNode	*pkParent = pkNode->m_pkParent;
		if(pkParent && pkParent->m_spScreenElement[0])
		{
			float	fLeft,fTop,fWidth,fHeight,fWidth2,fHeight2;

			GetComboNodeRect(pkNode,fLeft,fTop,fWidth2,fHeight2);
			GetComboNodeRect(pkParent,fLeft,fTop,fWidth,fHeight);

			fLeft = kStartPoint.x + fWidth2 + 0.025;

			int const iSiblingCount = pkParent->m_iChildCount;
			int	const iSibilngIndex = pkNode->m_iChildIndex;

			fTop = (kStartPoint.y+fHeight/2.0f)-iSiblingCount*(36.0f/fScreenHeight)/2.0f+iSibilngIndex*(36.0f/fScreenHeight);

			kStartPoint = NiPoint3(fLeft,fTop,0);

			pkNode->m_spScreenElement[0]->SetTranslate(kStartPoint);
			pkNode->m_spScreenElement[1]->SetTranslate(kStartPoint);

			pkNode->m_spScreenElement[0]->Update(0);
			pkNode->m_spScreenElement[1]->Update(0);
		}
	}
	else if( kState == CTNS_PASSED )
	{
		if(pkPrevNode && pkPrevNode->m_spScreenElement[0])
		{	
			float	fLeft = 0.5f,fTop = 0.8f,fLeftTemp=0.0f,fTopTemp=0.0f;
			float	fWidth,fHeight;
			GetComboNodeRect(pkPrevNode,fLeftTemp,fTopTemp,fWidth,fHeight);
			fLeft = fLeftTemp - (fWidth*1.5f);
			if(pkNode->m_spScreenElement[0])
			{
				pkNode->m_spScreenElement[0]->SetTranslate(NiPoint3(fLeft,fTop,0));
				pkNode->m_spScreenElement[1]->SetTranslate(NiPoint3(fLeft,fTop,0));

				pkNode->m_spScreenElement[0]->Update(0);
				pkNode->m_spScreenElement[1]->Update(0);
			}
		}
	}
}

void	PgComboAdvisor::GetComboNodeRect(stComboTreeNode *pkNode,float &fLeft,float &fTop,float &fWidth,float &fHeight)
{
	
	fLeft = pkNode->m_spScreenElement[0]->GetTranslate().x;
	fTop = pkNode->m_spScreenElement[0]->GetTranslate().y;
	
	
	float	fLeftTemp,fTopTemp;
	pkNode->m_spScreenElement[0]->GetRectangle(0,fLeftTemp,fTopTemp,fWidth,fHeight);

}
void	PgComboAdvisor::Init()
{
	m_spComboTexture = g_kNifMan.GetTexture("../Data/6_UI/main/comboKey.tga");
	m_spComboEffectTexture = g_kNifMan.GetTexture("../Data/6_UI/skill/skKeyEf.tga");

	m_spMaterialProperty = NiNew NiMaterialProperty();
	m_spAlphaProperty = NiNew NiAlphaProperty();
	m_spAlphaProperty->SetAlphaBlending(true);
	m_spAlphaProperty->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
	m_spAlphaProperty->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);

	m_spTexturingProperty = NiNew NiTexturingProperty;
	m_spTexturingProperty->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
	m_spTexturingProperty->SetBaseTexture(m_spComboTexture);

	m_spTexturingProperty_Effect = NiNew NiTexturingProperty;
	m_spTexturingProperty_Effect->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
	m_spTexturingProperty_Effect->SetBaseTexture(m_spComboEffectTexture);

	m_spVertexColorProperty = NiNew NiVertexColorProperty();
	m_spVertexColorProperty->SetSourceMode(NiVertexColorProperty::SOURCE_EMISSIVE);
	m_spVertexColorProperty->SetLightingMode(NiVertexColorProperty::LIGHTING_E);

	m_iHitEffectIndex = 0;

	ParseXML();

}
void	PgComboAdvisor::ParseComboTreeIcon(const TiXmlElement *pkElement)
{
	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName,"ACTION")==0)
		{
			stComboIconData	kIconData;

			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					kIconData.m_kActionID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "DIR") == 0)
				{
					kIconData.m_kDir = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "IconRect") == 0)
				{
					int	x,y,w,h;
					sscanf(pcAttrValue,"%d,%d,%d,%d",&x,&y,&w,&h);
					kIconData.m_kIconRect.left = x;
					kIconData.m_kIconRect.right = x+w;
					kIconData.m_kIconRect.top = y;
					kIconData.m_kIconRect.bottom = y+h;
				}
				else if(strcmp(pcAttrName, "AlwaysBlink") == 0)
				{
					kIconData.m_kDir = pcAttrValue;
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			m_vIconData.push_back(kIconData);

		}

		pkElement = pkElement->NextSiblingElement();
	}
}

int	PgComboAdvisor::CountChildNode(const TiXmlElement *pkElement)
{
	int	iCount = 0;
	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		iCount++;
		pkElement = pkElement->NextSiblingElement();
	}

	return	iCount;
}
void	PgComboAdvisor::ParseXML()
{
	char	*strXmlPath = "UI/ComboTree.Xml";

	TiXmlDocument kXmlDoc(strXmlPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(strXmlPath)))
	{
		PgError1("Parse Failed [%s]", strXmlPath);
		return;
	}

	// Find Root
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "COMBO_TREE_ICON") == 0)
		{
			ParseComboTreeIcon(pkElement);
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
		}

		pkElement = pkElement->NextSiblingElement();
	}

}

NiScreenElementsPtr	PgComboAdvisor::CreateScreenElement(std::string kActionID,std::string kDir)
{
	float	fScreenWidth = static_cast<float>(NiRenderer::GetRenderer()->GetDefaultBackBuffer()->GetWidth());
	float	fScreenHeight = static_cast<float>(NiRenderer::GetRenderer()->GetDefaultBackBuffer()->GetHeight());

	if( 0==strcmp(kDir.c_str(), "HIT") )
	{
		NiScreenElementsPtr	spNewScreenElement = NiNew NiScreenElements(
			NiNew NiScreenElementsData(false,true,1)
			);

		float const fWidth = 110/fScreenWidth;
		float const fHeight = 110/fScreenHeight;
		float	fTexWidth = (float)m_spComboEffectTexture->GetWidth();
		float	fTexHeight =(float)m_spComboEffectTexture->GetHeight();

		spNewScreenElement->Insert(4);
		spNewScreenElement->SetRectangle(0,-fWidth/2,-fHeight/2,fWidth,fHeight);
		spNewScreenElement->UpdateBound();
		spNewScreenElement->SetTextures(0,0, 
			kRectHitEffect[m_iHitEffectIndex].left/fTexWidth, 
			kRectHitEffect[m_iHitEffectIndex].top/fTexHeight, 
			kRectHitEffect[m_iHitEffectIndex].right/fTexWidth, 
			kRectHitEffect[m_iHitEffectIndex].bottom/fTexHeight);
		spNewScreenElement->SetColors(0,NiColorA(1,1,1,1),NiColorA(1,1,1,1),NiColorA(1,1,1,1),NiColorA(1,1,1,1));

		spNewScreenElement->AttachProperty(m_spMaterialProperty);
		spNewScreenElement->AttachProperty(m_spAlphaProperty);
		spNewScreenElement->AttachProperty(m_spVertexColorProperty);
		spNewScreenElement->AttachProperty(m_spTexturingProperty_Effect);

		spNewScreenElement->SetConsistency(NiGeometryData::VOLATILE);

		spNewScreenElement->UpdateEffects();
		spNewScreenElement->UpdateProperties();
		spNewScreenElement->Update(0.0f);

		return	spNewScreenElement;
	}

	float	fTexWidth = (float)m_spComboTexture->GetWidth();
	float	fTexHeight =(float)m_spComboTexture->GetHeight();
	int	iIconDataSize = m_vIconData.size();
	for(int i=0;i<iIconDataSize;i++)
	{
		if(m_vIconData[i].m_kActionID != kActionID) continue;
		if(m_vIconData[i].m_kDir != "" && m_vIconData[i].m_kDir != kDir) continue;

		NiScreenElementsPtr	spNewScreenElement = NiNew NiScreenElements(
			NiNew NiScreenElementsData(false,true,1)
			);

		float const fWidth = fabs(((float)(m_vIconData[i].m_kIconRect.right - m_vIconData[i].m_kIconRect.left)))/fScreenWidth;
		float const fHeight = fabs(((float)(m_vIconData[i].m_kIconRect.bottom - m_vIconData[i].m_kIconRect.top)))/fScreenHeight;

		spNewScreenElement->Insert(4);
		spNewScreenElement->SetRectangle(0,-fWidth/2,-fHeight/2,fWidth,fHeight);
		spNewScreenElement->UpdateBound();
		spNewScreenElement->SetTextures(0,0,
			m_vIconData[i].m_kIconRect.left/fTexWidth
			,m_vIconData[i].m_kIconRect.top/fTexHeight
			,m_vIconData[i].m_kIconRect.right/fTexWidth
			,m_vIconData[i].m_kIconRect.bottom/fTexHeight);
		spNewScreenElement->SetColors(0,NiColorA(1,1,1,1),NiColorA(1,1,1,1),NiColorA(1,1,1,1),NiColorA(1,1,1,1));

		spNewScreenElement->AttachProperty(m_spMaterialProperty);
		spNewScreenElement->AttachProperty(m_spAlphaProperty);
		spNewScreenElement->AttachProperty(m_spVertexColorProperty);
		spNewScreenElement->AttachProperty(m_spTexturingProperty);

		spNewScreenElement->SetConsistency(NiGeometryData::VOLATILE);

		spNewScreenElement->UpdateEffects();
		spNewScreenElement->UpdateProperties();
		spNewScreenElement->Update(0.0f);

		return	spNewScreenElement;

	}

	return	NULL;

}
