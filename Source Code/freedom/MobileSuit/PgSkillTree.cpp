#include "stdafx.h"
#include "Variant/PgControlDefMgr.h"
#include "Variant/constant.h"
#include "Variant/PgPlayer.h"

#include "PgSkillTree.h"
#include "ServerLib.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgAction.H"

PgSkillTree	g_kSkillTree;

namespace PgPetUIUtil
{
	bool PetSkillToSkillTree(PgPlayer* pkPlayer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	struct	PgSkillTree::stTreeNode
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PgSkillTree::stTreeNode::stTreeNode(unsigned long const ulKeySkillNo, unsigned long const ulSkillNo,
									CSkillDef const* pkSkillDef,
									bool const bLearned,int const iOriginalSkillLevel)

									:m_ulSkillNo(ulSkillNo),
									m_ulKeySkillNo(ulKeySkillNo),
									m_iOverSkillLevel(0),
									m_pkSkillDef(pkSkillDef),
									m_bLearned(bLearned),
									m_iMaxSkillLevel(1),
									m_bTemporaryLearned(bLearned)
{
	if(m_pkSkillDef)
	{
		m_iOriginalSkillLevel =  (iOriginalSkillLevel == -1) ? m_pkSkillDef->GetAbil(AT_LEVEL) : iOriginalSkillLevel;
	}
}
PgSkillTree::stTreeNode::~stTreeNode()
{
}

bool PgSkillTree::stTreeNode::IsTemporaryLevelChanged()	const //	현재 이 스킬이 임시로 레벨을 올린상태인가
{
	if(m_bLearned != m_bTemporaryLearned)
	{
		return	true;
	}
	if(m_pkSkillDef && m_pkSkillDef->GetAbil(AT_LEVEL) != m_iOriginalSkillLevel)
	{
		return	true;
	}

	return	false;
}

int	PgSkillTree::stTreeNode::ConfirmTemporary() const
{
	int iDepth = -1;
	if(m_pkSkillDef)
	{
		if(m_bTemporaryLearned != m_bLearned || m_iOriginalSkillLevel != m_pkSkillDef->GetAbil(AT_LEVEL))
		{
			//lua_tinker::call<void,int>("Net_C_M_REQ_LEARN_SKILL",m_ulSkillNo);
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			kSkillDefMgr.GetNeedSkillDepth(m_ulKeySkillNo, iDepth);
		}
	}

	return iDepth;
}
void	PgSkillTree::stTreeNode::ResetTemporary()
{
	m_bTemporaryLearned = m_bLearned;
	m_ulSkillNo = GetOriginalSkillNo();

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	m_pkSkillDef = kSkillDefMgr.GetDef(m_ulSkillNo);
}

int PgSkillTree::stTreeNode::GetOverSkillLevel() const
{
	int iOverSkillLevel = m_iOverSkillLevel;

	if(0 == iOverSkillLevel)
	{
		return 0;
	}

	if(GetSkillDef())
	{
		int const iLevel = GetSkillDef()->GetAbil(AT_LEVEL);

		GET_DEF( CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkDef = NULL;

		do
		{
			pkDef = kSkillDefMgr.GetDef(m_ulKeySkillNo + iLevel + iOverSkillLevel - 1);

			if(NULL == pkDef)
			{
				--iOverSkillLevel;
			}
			else
			{
				// 실제로 존재하면 정지
				break;
			}

		}while(NULL != pkDef || 0 < iOverSkillLevel);

	}

	return iOverSkillLevel;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgSkillTree
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


PgSkillTree::PgSkillTree() 
: m_iUsedSkillPoint(0),
m_iRemainSkillPoint(0),
m_iLastFoundSkillNo(0),
m_pkLastFoundTreeNode(NULL)
{
}

PgSkillTree::~PgSkillTree()
{
	Terminate();
}
int	PgSkillTree::GetKeySkillNo(int const iSkillNo) const
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);
	if(!pkSkillDef)
	{
		return iSkillNo;
	}
	int iKeySkillNo = pkSkillDef->GetParentSkill();

	if(0 == iKeySkillNo)
	{
		iKeySkillNo = iSkillNo;
	}

	return iKeySkillNo;
}
int	PgSkillTree::GetKeySkillNo(PgAction const* pkAction) const
{
	if(!pkAction)
	{
		return	0;
	}

	CSkillDef const* pkSkillDef = pkAction->GetSkillDef();
	
	if(!pkSkillDef)
	{
		return	GetKeySkillNo(pkAction->GetActionNo());
	}

	int iKeySkillNo = pkSkillDef->GetParentSkill();

	if(0 == iKeySkillNo)
	{
		iKeySkillNo = pkAction->GetActionNo();
	}

	return	iKeySkillNo;
}

void PgSkillTree::NewSkillLearned(unsigned long const ulSkillNo, int const iRemainSkillPoint)
{
	ResetTemporary();
	m_iOriginalRemainSkillPoint = m_iRemainSkillPoint = iRemainSkillPoint;

	if(g_kPilotMan.GetPlayerUnit())
	{
		PgMySkill* pkMySkill = g_kPilotMan.GetPlayerUnit()->GetMySkill();
		if(pkMySkill)
		{
			pkMySkill->LearnNewSkill(ulSkillNo);
		}
	}

	unsigned long ulKeySkillNo = GetKeySkillNo(ulSkillNo);
	stTreeNode*	pNode = GetNode(ulKeySkillNo);

	if(pNode)
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(ulSkillNo);
		PG_ASSERT_LOG(pkSkillDef);
		if(!pkSkillDef)
		{
			return;
		}

		pNode->SetValue(ulSkillNo,pkSkillDef,pkSkillDef->GetAbil(AT_LEVEL),true);
	}
}
void	PgSkillTree::DeleteSkill(unsigned long const ulSkillNo)
{
	ResetTemporary();

	if(g_kPilotMan.GetPlayerUnit())
	{
		PgMySkill* pkMySkill = g_kPilotMan.GetPlayerUnit()->GetMySkill();
		if(pkMySkill)
		{
			pkMySkill->Delete(ulSkillNo);
		}
	}

	unsigned long ulKeySkillNo = GetKeySkillNo(ulSkillNo);
	stTreeNode*	pNode = GetNode(ulKeySkillNo);
	if(pNode)
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(ulSkillNo);
		PG_ASSERT_LOG(pkSkillDef);
		if(!pkSkillDef)
		{
			return;
		}

		pNode->SetValue(ulSkillNo,pkSkillDef,pkSkillDef->GetAbil(AT_LEVEL),false);
	}
}

void	PgSkillTree::ClearSkillTree()
{
	m_iUsedSkillPoint = 0;
	m_iRemainSkillPoint = 0;
	m_iLastFoundSkillNo = 0;
	m_pkLastFoundTreeNode = NULL;
	
	for(VTree::iterator itor = m_vSkills.begin(); itor != m_vSkills.end(); ++itor)
	{
		SAFE_DELETE(itor->second);
	}
	m_vSkills.clear();
}

void	PgSkillTree::CreateSkillTree( PgPlayer const* pkPlayer )
{
	if ( pkPlayer )
	{
		CONT_DEFSKILL const *pkDefSkill = NULL;
		g_kTblDataMgr.GetContDef( pkDefSkill );
		if ( !pkDefSkill )
		{
			_PgMessageBox( "[Skill Tree]", " Critical Error : Define Skill is NULL" );
			return;
		}

		ClearSkillTree();

		//	클래스를 얻어온다.
		int const iClassID = pkPlayer->GetAbil( AT_CLASS );

		m_iOriginalRemainSkillPoint = m_iRemainSkillPoint = pkPlayer->GetAbil(AT_SP);

		int const iMinSkillID = 90000001;

		int	iSkillNo = 0;
		CONT_DEFSKILL::const_iterator skill_itor = pkDefSkill->begin();
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		while( skill_itor!= pkDefSkill->end() )
		{
			iSkillNo = (*skill_itor).second.SkillNo;
			CSkillDef const *pkSkillDef = kSkillDefMgr.GetDef(iSkillNo);

			if(pkSkillDef)
			{
				bool const bWrightSkillType = pkSkillDef->GetAbil(AT_TYPE) == EST_ACTIVE || pkSkillDef->GetAbil(AT_TYPE) == EST_PASSIVE || pkSkillDef->GetAbil(AT_TYPE) == EST_TOGGLE;
				bool const bCorrectClass = IS_CLASS_LIMIT( pkSkillDef->GetAbil64(AT_CLASSLIMIT), iClassID);

				if( iSkillNo >= iMinSkillID
					&&	bWrightSkillType
					&&	bCorrectClass
					&&	(pkSkillDef->GetAbil(AT_SKILL_KIND) != ESK_NONE) )
				{
					AddSkillToTree( pkPlayer, m_vSkills, static_cast<unsigned long>(iSkillNo), pkSkillDef );
				}
			}
			++skill_itor;
		}
		PgPetUIUtil::PetSkillToSkillTree(g_kPilotMan.GetPlayerUnit());

	}
	else
	{
		_PgMessageBox( "[Skill Tree]", " Critical Error : NULL Player" );
	}		
}

int	PgSkillTree::GetNextLevelSkillNo(int const iKeySkillNo)
{
	PgSkillTree::stTreeNode* pFound = GetNode(iKeySkillNo);
	if(!pFound)
	{
		return	0;
	}

	if(false == pFound->m_bLearned) 
	{
		return	pFound->m_ulSkillNo;
	}

	return	pFound->m_ulSkillNo + 1;
}
void	PgSkillTree::ResetTemporary()
{
	for(VTree::iterator itor = m_vSkills.begin();
		itor != m_vSkills.end(); ++itor)
	{
		itor->second->ResetTemporary();
	}

	m_iRemainSkillPoint = m_iOriginalRemainSkillPoint;
}
void	PgSkillTree::ConfirmTemporary()
{
	VTreeByDepth kDepthMap;
	for(VTree::iterator itor = m_vSkills.begin();
		itor != m_vSkills.end(); ++itor)
	{
		int const iDepth = itor->second->ConfirmTemporary();
		if (0<=iDepth)
		{
			kDepthMap.insert(std::make_pair(iDepth, itor->second));
		}
	}

	for (VTreeByDepth::const_iterator it = kDepthMap.begin(); it != kDepthMap.end(); ++it)
	{
		const stTreeNode* pkNode = (*it).second;
		if (pkNode)
		{
			lua_tinker::call<void,int>("Net_C_M_REQ_LEARN_SKILL",pkNode->m_ulSkillNo);
		}
	}
}
int	PgSkillTree::GetNeedSkillPoint(int const iKeySkillNo)
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkCurSkillDef = kSkillDefMgr.GetDef(iKeySkillNo);
	
	int iParentSkillNo = pkCurSkillDef->GetParentSkill();
	if(0 == iParentSkillNo)
	{
		iParentSkillNo = iKeySkillNo;
	}
	
	PgSkillTree::stTreeNode* pFound = GetNode(iParentSkillNo);
	if(!pFound)
	{
		return	0;
	}

	if(pFound->m_bLearned == false && pFound->m_bTemporaryLearned == false)
	{
		return pFound->m_pkSkillDef->GetAbil(AT_NEED_SP);
	}

	if(pFound->m_pkSkillDef->GetAbil(AT_LEVEL) == pFound->GetMaxSkillLevel())
	{
		return -1;
	}

	CSkillDef const* pkNextLevel = kSkillDefMgr.GetDef(pFound->m_ulSkillNo+1);
	if(pkNextLevel)
	{
		return	pkNextLevel->GetAbil(AT_NEED_SP);
	}

	return	-1;
}

void	PgSkillTree::LevelUpTemporary(int const iKeySkillNo)
{
	if(CanLevelUp(iKeySkillNo) != NLR_NONE) return;

	stTreeNode*	pNode = GetNode(iKeySkillNo);

	int	iNeedSP = GetNeedSkillPoint(iKeySkillNo);
	if(iNeedSP>=0)
	{
		m_iRemainSkillPoint-=iNeedSP;
	}

	if(pNode)
	{
		if(!pNode->m_bTemporaryLearned)
		{
			pNode->m_bTemporaryLearned = true;
			return;
		}
		pNode->m_ulSkillNo++;
		pNode->m_bTemporaryLearned = true;
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		pNode->m_pkSkillDef = kSkillDefMgr.GetDef(pNode->m_ulSkillNo);
	}
}
void	PgSkillTree::LevelDownTemporary(int const iKeySkillNo)
{
	stTreeNode*	pNode = GetNode(iKeySkillNo);

	if(pNode)
	{
		CSkillDef const* pkSkillDef = pNode->m_pkSkillDef;
		if(pkSkillDef->GetAbil(AT_LEVEL)==pNode->m_iOriginalSkillLevel)
		{
			if(pNode->m_bLearned == false && pNode->m_bTemporaryLearned)
			{
				m_iRemainSkillPoint += pkSkillDef->GetAbil(AT_NEED_SP);
				pNode->m_bTemporaryLearned = false;
			}
		}
		else if(pkSkillDef->GetAbil(AT_LEVEL)>pNode->m_iOriginalSkillLevel)
		{
			m_iRemainSkillPoint += pkSkillDef->GetAbil(AT_NEED_SP);
			GET_DEF(CSkillDefMgr, kSkillDefMgr);

			pNode->m_ulSkillNo--;
			pNode->m_pkSkillDef = kSkillDefMgr.GetDef(pNode->m_ulSkillNo);
		}
	}
}
bool	PgSkillTree::IsTemporaryRemainSkillPoint(int const iSkillNo)
{
	if (0<iSkillNo)
	{
		PgSkillTree::stTreeNode const *pFound = GetNode(GetKeySkillNo(iSkillNo));
		if(!pFound)
		{
			return	false;
		}	

		if(EST_PASSIVE == pFound->m_pkSkillDef->GetAbil(AT_TYPE) )//패시브 스킬이면
		{
			VTree::const_iterator skill_it = m_vSkills.begin();
			while (skill_it != m_vSkills.end())
			{
				stTreeNode const * pkValue = (*skill_it).second;
				if(pkValue)
				{
					CSkillDef const* pkSkillDef = pkValue->m_pkSkillDef;
					if(pkSkillDef)
					{
						if(pkSkillDef->GetAbil(AT_TYPE) == EST_PASSIVE 
							&& pkSkillDef->GetAbil(AT_SKILL_DIVIDE_TYPE) <= SDT_Normal 
							&& pkValue->m_bTemporaryLearned		// 90000001커플 스킬
							)
						{
							return true;
						}
					}
					else
					{// ERROR
						_PgMessageBox("ERROR", "PgSkillTree::IsTemporaryRemainSkillPoint()\n%d DefSkill Is NULL", pkValue->m_ulSkillNo);
						return false;
					}
				}
				else
				{// ERROR
					_PgMessageBox("ERROR", "PgSkillTree::IsTemporaryRemainSkillPoint()\n TreeNode Is NULL");
					return false;
				}
				++skill_it;
			}
		}
	}
	return	m_iRemainSkillPoint != m_iOriginalRemainSkillPoint;
}
bool PgSkillTree::CanLevelDown(int const iKeySkillNo)
{
	PgSkillTree::stTreeNode* pFound = GetNode(iKeySkillNo);
	if(!pFound)
	{
		return	false;
	}
	int	const iSkillLevel = pFound->GetSkillDef()->GetAbil(AT_LEVEL);

	PgSkillTree::stTreeNode* pKeyFound = GetNode(GetKeySkillNo(iKeySkillNo));
	if(!pKeyFound)
	{
		return false;
	}

	int const iMaxNeedSkill = LEAD_SKILL_COUNT;
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	for(int i=0;i<iMaxNeedSkill;++i)// 키스킬도 점검하자
	{
		int	iNeedSkillNo = pKeyFound->m_pkSkillDef->GetAbil(AT_NEED_SKILL_01+i);
		if(0 < iNeedSkillNo)
		{
			CSkillDef const* pNeedSkillDef = kSkillDefMgr.GetDef(iNeedSkillNo);
			int const iNeedSkillKeyNo = GetKeySkillNo(iNeedSkillNo);
			PgSkillTree::stTreeNode* pFoundNeedSkill = GetNode(iNeedSkillKeyNo);
			if(!pFoundNeedSkill)
			{
				pFound->m_bLearned = false;
				pFound->m_bTemporaryLearned = false;
				return	false;
			}
			else if((pFoundNeedSkill->m_bLearned == false && pFoundNeedSkill->m_bTemporaryLearned == false)) 
			{
				pFound->m_bLearned = false;
				pFound->m_bTemporaryLearned = false;
				return	false;
			}
			else if(pFoundNeedSkill->m_pkSkillDef->m_byLv<pNeedSkillDef->m_byLv) 	//	레벨이 부족함
			{
				pFound->m_bLearned = false;
				pFound->m_bTemporaryLearned = false;
				return	false;
			}
		}
	}

	if(iSkillLevel>pFound->m_iOriginalSkillLevel || pFound->m_bTemporaryLearned != pFound->m_bLearned)
	{
		return	true;
	}

	return	false;
}
int	PgSkillTree::CanLevelUp(int const iKeySkillNo)
{
	int	iNoLevelUpReason = NLR_NONE;

	int const iRemainSkillPoint = GetRemainSkillPoint();

	/*if(iRemainSkillPoint == 0) 
	{
		iNoLevelUpReason|=NLR_SKILLPOINT;
	}*/

	VTree& kTree = GetTree();

	VTree::iterator itor = kTree.find(iKeySkillNo);
	if(itor == kTree.end())
	{
		iNoLevelUpReason|=NLR_UNKNOWN;
		return iNoLevelUpReason;
	}

	PgSkillTree::stTreeNode* pFound = itor->second;
	if(!pFound) 
	{
		iNoLevelUpReason|=NLR_UNKNOWN;
		return iNoLevelUpReason;
	}

	if(pFound->m_pkSkillDef->m_byLv >= pFound->GetMaxSkillLevel() && (pFound->m_bLearned == true || pFound->m_bTemporaryLearned == true) ) 
	{
		iNoLevelUpReason|=NLR_MAXLEVEL;
	}

	CSkillDef const* pkNextLevel = NULL;
	
	if(pFound->m_bLearned == true || pFound->m_bTemporaryLearned == true)
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		pkNextLevel = kSkillDefMgr.GetDef(pFound->m_ulSkillNo+1);
	}
	else
	{
		pkNextLevel = pFound->m_pkSkillDef;
	}

	if(!pkNextLevel)	
	{
		iNoLevelUpReason|=NLR_MAXLEVEL;
		return iNoLevelUpReason;
	}

	//	스킬 포인트 체크
	int	const iNeedSP = pkNextLevel->GetAbil(AT_NEED_SP);
	if(iRemainSkillPoint < iNeedSP) 
	{
		iNoLevelUpReason |= NLR_SKILLPOINT;
	}

	//	필요 스킬(Need Skill) 체크
	int const iMaxNeedSkill = LEAD_SKILL_COUNT;
	for(int i=0;i<iMaxNeedSkill;++i)
	{
		int	iNeedSkillNo = pkNextLevel->GetAbil(AT_NEED_SKILL_01+i);
		if(iNeedSkillNo>0)
		{
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pNeedSkillDef = kSkillDefMgr.GetDef(iNeedSkillNo);
			int const iNeedSkillKeyNo = GetKeySkillNo(iNeedSkillNo);
			PgSkillTree::stTreeNode* pFoundNeedSkill = GetNode(iNeedSkillKeyNo);
			if(!pFoundNeedSkill)
			{
				iNoLevelUpReason|=NLR_NEEDSKILL;
			}
			else if((pFoundNeedSkill->m_bLearned == false && pFoundNeedSkill->m_bTemporaryLearned == false)) 
			{
				iNoLevelUpReason|=NLR_NEEDSKILL;
			}
			else if(pFoundNeedSkill->m_pkSkillDef->m_byLv<pNeedSkillDef->m_byLv) 	//	레벨이 부족함
			{
				iNoLevelUpReason|=NLR_NEEDSKILL;
			}
		}
	}

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	for(int i=0;i<iMaxNeedSkill;++i)// 키스킬도 점검하자
	{
		int	iNeedSkillNo = pFound->m_pkSkillDef->GetAbil(AT_NEED_SKILL_01+i);
		if(iNeedSkillNo>0)
		{
			CSkillDef const* pNeedSkillDef = kSkillDefMgr.GetDef(iNeedSkillNo);
			int const	iNeedSkillKeyNo = GetKeySkillNo(iNeedSkillNo);
			PgSkillTree::stTreeNode* pFoundNeedSkill = GetNode(iNeedSkillKeyNo);
			if(!pFoundNeedSkill)
			{
				pFound->m_bLearned = false;
				pFound->m_bTemporaryLearned = false;
				iNoLevelUpReason|=NLR_NEEDSKILL;
			}
			else if((pFoundNeedSkill->m_bLearned == false && pFoundNeedSkill->m_bTemporaryLearned == false)) 
			{
				pFound->m_bLearned = false;
				pFound->m_bTemporaryLearned = false;
				iNoLevelUpReason|=NLR_NEEDSKILL;
			}
			else if(pFoundNeedSkill->m_pkSkillDef->m_byLv<pNeedSkillDef->m_byLv) 	//	레벨이 부족함
			{
				pFound->m_bLearned = false;
				pFound->m_bTemporaryLearned = false;
				iNoLevelUpReason|=NLR_NEEDSKILL;
			}

		}
	}

	//	캐릭터 제한 체크
	PgPilot *pkPilot = g_kPilotMan.GetPlayerPilot();
	if(!pkPilot)
	{
		iNoLevelUpReason|=NLR_UNKNOWN;
		return iNoLevelUpReason;
	}

	PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();

	if(!pkPlayer) 
	{
		iNoLevelUpReason|=NLR_UNKNOWN;
		return iNoLevelUpReason;
	}
	int const iClass = pkPlayer->GetAbil(AT_CLASS);
	//if((pkNextLevel->m_i64ClassLimit & (1i64<<iClass))==0) 
	if (IS_CLASS_LIMIT(pkNextLevel->m_i64ClassLimit, iClass) == false)
	{
		iNoLevelUpReason|=NLR_CHAR_CLASS;
	}

	//	레벨 제한 체크
	int const iLevel = pkPlayer->GetAbil(AT_LEVEL);
	if(iLevel<pkNextLevel->m_sLevelLimit) 
	{
		iNoLevelUpReason|=NLR_CHAR_LEVEL;
	}

	if(pFound->m_bLearned == false || pFound->m_bTemporaryLearned == false || pFound->m_pkSkillDef->m_byLv < pFound->GetMaxSkillLevel()) 
	{
		return	iNoLevelUpReason;
	}

	iNoLevelUpReason|=NLR_UNKNOWN;
	
	return	iNoLevelUpReason;
}

void	PgSkillTree::AddSkillToTree( PgPlayer const* pkPlayer, VTree& kDestTree, unsigned long const ulSkillNo, CSkillDef const* pkSkillDef )
{
	unsigned long ulKeySkillNo = 0;

	PgMySkill const* pkMySkill = pkPlayer->GetMySkill();
	bool const bIsLearnedSkill = pkMySkill->IsExist((int)ulSkillNo);

	if( true == bIsLearnedSkill )
	{
		++m_iUsedSkillPoint;
	}

	int const iSkillLevel = pkSkillDef->GetAbil(AT_LEVEL);
	_PgOutputDebugString("ulSkillNo : %u Learned : %d iSkillLevel : %d\n",ulSkillNo,bIsLearnedSkill,iSkillLevel);

	unsigned long ulParentSkillNo = pkSkillDef->GetParentSkill();

	if(ulParentSkillNo == 0)
	{
		ulParentSkillNo = ulSkillNo;
	}
	else //	레벨이 여러개 있는 스킬이다.
	{
		VTree::iterator itor = kDestTree.find(ulParentSkillNo);
		if(itor != kDestTree.end())
		{
			//	이미 같은 스킬이 존재한다. 단 스킬 번호가 다르다.
			stTreeNode* pkExistNode = itor->second;

			// 해당 번호에 해당되는 OverLevel이 있을 경우 세팅 +@를 세팅
			int iOverSkillLevel = pkMySkill->GetOverSkillLevel(static_cast<int>(ulSkillNo));
			if(iOverSkillLevel)
			{
				pkExistNode->SetOverSkillLevel(iOverSkillLevel);
			}
			
			if(bIsLearnedSkill)
			{
				if(pkExistNode->IsLearned() == false || 
					pkExistNode->GetSkillDef()->GetAbil(AT_LEVEL) < iSkillLevel)
				{
					//	교체한다.
					pkExistNode->SetValue(
						ulSkillNo,
						pkSkillDef,
						pkSkillDef->GetAbil(AT_LEVEL),
						bIsLearnedSkill);
				}
			}
			else if(pkExistNode->IsLearned() == false)
			{
				if(pkExistNode->GetSkillDef()->GetAbil(AT_LEVEL) > iSkillLevel)
				{
					//	교체한다.
					pkExistNode->SetValue(
						ulSkillNo,
						pkSkillDef,
						pkSkillDef->GetAbil(AT_LEVEL),
						bIsLearnedSkill);
				}
			}

			int const iMaxLevelLimit = MAX_SKILL_LEVEL + pkPlayer->GetMySkill()->GetExtendLevel(ulSkillNo);

			// 스킬의 MaxLevel은 5로 되 어있다.(5 이상은 오버 스킬이다. Item에 의한 + 되는 스킬
			if(pkExistNode->GetMaxSkillLevel() < iSkillLevel)
			{				
				pkExistNode->SetMaxSkillLevel(iSkillLevel <= iMaxLevelLimit ? (iSkillLevel) : (iMaxLevelLimit));
			}

			return;
		}
	}

	//	새로 만들어 추가한다.
	kDestTree.insert(std::make_pair(ulParentSkillNo,new stTreeNode(ulParentSkillNo,ulSkillNo,pkSkillDef,bIsLearnedSkill)));
}

void PgSkillTree::SetRemainSkillPoint(int const iSkillPoint)
{
	ResetTemporary();
	m_iOriginalRemainSkillPoint = m_iRemainSkillPoint = iSkillPoint;
}
int	PgSkillTree::GetRemainSkillPoint() const
{
	return	m_iRemainSkillPoint;
}
int	PgSkillTree::GetUsedSkillPoint() const
{
	return	m_iUsedSkillPoint;
}

PgSkillTree::VTree& PgSkillTree::GetTree()
{
	return	m_vSkills;
}

bool PgSkillTree::CanLearn(int const iTarget, int const iFrom)
{
	if (0 >= iFrom)
	{
		PgSkillTree::stTreeNode* pFound = GetNode(iTarget);
		if(!pFound)
		{
			return	false;
		}

		return pFound->m_bLearned;
	}

	bool bRet = false;
	VTree& kTree = GetTree();

	VTree::iterator itor = kTree.find(iTarget);
	if(itor == kTree.end()) { return bRet; }
	PgSkillTree::stTreeNode* pFound = itor->second;
	if(!pFound) { return bRet; } 

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkDef = kSkillDefMgr.GetDef(pFound->m_ulKeySkillNo);
	if (!pkDef) { return bRet; } 
	int i = 1;
	int const iPreLearnCnt = LEAD_SKILL_COUNT;
	int const iPreLearn[iPreLearnCnt] = {pkDef->GetAbil(AT_NEED_SKILL_01), 
		pkDef->GetAbil(AT_NEED_SKILL_01+1), 
		pkDef->GetAbil(AT_NEED_SKILL_01+2), 
		pkDef->GetAbil(AT_NEED_SKILL_01+3), 
		pkDef->GetAbil(AT_NEED_SKILL_01+4), 
		pkDef->GetAbil(AT_NEED_SKILL_01+5), 
		pkDef->GetAbil(AT_NEED_SKILL_01+6), 
		pkDef->GetAbil(AT_NEED_SKILL_01+7), 
		pkDef->GetAbil(AT_NEED_SKILL_01+8), 
		pkDef->GetAbil(AT_NEED_SKILL_01+9) 
	};

	for (i = 0; i<iPreLearnCnt; ++i)
	{
		int const iKey = GetKeySkillNo(iPreLearn[i]);
		if (iKey == iFrom)
		{
			PgSkillTree::stTreeNode* pPreFound = GetNode(iKey);
			if(!pPreFound || (!pPreFound->m_bLearned && !pPreFound->m_bTemporaryLearned))
			{
				return	false;
			}
			else
			{
				return pPreFound->m_ulSkillNo >= iPreLearn[i];		
			}
		}
	}

	return bRet;
}

PgSkillTree::stTreeNode* PgSkillTree::GetNode(unsigned long const ulKeySkillNo, bool const bIgnoreLastFound)
{
	if(0 == ulKeySkillNo)
	{
		return	NULL;
	}

	if ((false==bIgnoreLastFound) && (ulKeySkillNo == m_iLastFoundSkillNo))
	{
		return m_pkLastFoundTreeNode;
	}

	VTree::const_iterator itor = m_vSkills.find(ulKeySkillNo);
	if(itor != m_vSkills.end())
	{
		stTreeNode* pFound = itor->second;
		if (pFound)
		{
			m_iLastFoundSkillNo = ulKeySkillNo;
			m_pkLastFoundTreeNode = pFound;
			return pFound;
		}
	}

	return	NULL;
}

bool PgSkillTree::CheckAllNeedSkill(int const iSkillNo)
{
	bool bRet = false;
	VTree& kTree = GetTree();

	VTree::iterator itor = kTree.find(iSkillNo);
	if(itor == kTree.end()) { return bRet; }

	PgSkillTree::stTreeNode* pFound = itor->second;
	if(!pFound) { return bRet; } 

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkDef = kSkillDefMgr.GetDef(pFound->m_ulKeySkillNo);
	if (!pkDef) { return bRet; } 

	int i = 1;
	int const iPreLearnCnt = LEAD_SKILL_COUNT;
	int const iPreLearn[iPreLearnCnt] = {pkDef->GetAbil(AT_NEED_SKILL_01), 
		pkDef->GetAbil(AT_NEED_SKILL_01+1), 
		pkDef->GetAbil(AT_NEED_SKILL_01+2), 
		pkDef->GetAbil(AT_NEED_SKILL_01+3), 
		pkDef->GetAbil(AT_NEED_SKILL_01+4), 
		pkDef->GetAbil(AT_NEED_SKILL_01+5), 
		pkDef->GetAbil(AT_NEED_SKILL_01+6), 
		pkDef->GetAbil(AT_NEED_SKILL_01+7), 
		pkDef->GetAbil(AT_NEED_SKILL_01+8), 
		pkDef->GetAbil(AT_NEED_SKILL_01+9) 
	};

	for (i = 0; i<iPreLearnCnt; ++i)
	{
		int const iKey = GetKeySkillNo(iPreLearn[i]);
		PgSkillTree::stTreeNode* pPreFound = GetNode(iKey);
		int const iOriginKey = GetKeySkillNo(iSkillNo);
		int iTemp = pFound->m_ulSkillNo;
		if(pPreFound)
		{
			if( iPreLearn[i] > pPreFound->m_ulSkillNo || (!pPreFound->m_bLearned && !pPreFound->m_bTemporaryLearned))
			{
				if (pFound->m_bTemporaryLearned)
				{
					while (iOriginKey <= iTemp)
					{
						LevelDownTemporary(iOriginKey);
						--iTemp;
					}
				}
				return bRet;
			}
		}
		else if (0!=iPreLearn[i])
		{
			if (pFound->m_bTemporaryLearned)
			{
				while (iOriginKey <= iTemp)
				{
					LevelDownTemporary(iOriginKey);
					--iTemp;
				}
			}
			return bRet;
		}
	}

	return true;
}

bool PgSkillTree::GetNeedSkill(int const iKeySkillNo, int* pkArray)
{
	if (0 > iKeySkillNo || !pkArray) { return 0; }

	VTree& kTree = GetTree();
	bool bRet = false;

	int const iKey = GetKeySkillNo(iKeySkillNo);

	VTree::iterator itor = kTree.find(iKey);
	if(itor == kTree.end()) { return bRet; }
	PgSkillTree::stTreeNode* pFound = itor->second;
	if(!pFound) { return bRet; } 

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkDef = kSkillDefMgr.GetDef(pFound->m_ulKeySkillNo);
	if (!pkDef) { return bRet; } 
	int i = 1;

	for (int i = 0; i < LEAD_SKILL_COUNT; ++i)
	{
		int const iNo = pkDef->GetAbil(AT_NEED_SKILL_01+i);
		pkArray[i] = iNo;
		if (iNo) { bRet = true; }
	}

	return bRet;
}

void PgSkillTree::ClearSkillTreeOverSkillLevel()
{
	for(VTree::iterator itor = m_vSkills.begin(); itor != m_vSkills.end(); ++itor)
	{
		(*itor).second->SetOverSkillLevel(0);
	}
}

void PgSkillTree::AddPetSkillToTree(unsigned long const ulSkillNo, CSkillDef const* pkSkillDef)
{//쿨타임 용이니까 일단 무조건 넣고보자.
	if(0>=ulSkillNo ||!pkSkillDef)	{return;}
	unsigned long ulParentSkillNo = pkSkillDef->GetParentSkill();
	VTree::iterator find_it = m_vSkills.find(static_cast<int>(ulParentSkillNo));
	if(m_vSkills.end()!=find_it)	{return;}

	m_vSkills.insert(std::make_pair(ulParentSkillNo,new stTreeNode(ulParentSkillNo,ulSkillNo,pkSkillDef,true)));
}

void PgSkillTree::DeleteAllPetSkill()
{//Lock이 필요한데...
	VTree::iterator it = m_vSkills.begin();
	while(it!=m_vSkills.end())
	{
		stTreeNode* pkNode = (*it).second;
		if(pkNode && pkNode->GetSkillDef())
		{
			if(90000001 > pkNode->GetSkillDef()->No() && 80000000 < pkNode->GetSkillDef()->No())
			{
				m_vSkills.erase(it++);
				continue;
			}
		}
		++it;
	}
}
const char* STR_PATH_BASIC_COMBO = "ui/BasicComboData.xml";
void PgSkillTree::ParseXml_BasicCombo()
{
	TiXmlDocument kXmlDoc(STR_PATH_BASIC_COMBO);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(STR_PATH_BASIC_COMBO)))
	{
		if( kXmlDoc.Error() )
		{
			CAUTION_LOG(BM::LOG_LV1, __FL__ << L"Can't open file: " << STR_PATH_BASIC_COMBO << L"Error Msg["<<kXmlDoc.ErrorDesc()<<L"]");
		}
		else
		{
			CAUTION_LOG(BM::LOG_LV1, __FL__ << L"Can't open file: " << STR_PATH_BASIC_COMBO);
		}
		return;
	}
	TiXmlNode* pRoot = kXmlDoc.FirstChild("BASIC_COMBO_DATA");
	if( !pRoot )
	{
		CAUTION_LOG(BM::LOG_LV1, __FL__ << "Error From "<< STR_PATH_BASIC_COMBO << L" : Not Found Tag : BASIC_COMBO_DATA");
		return;
	}

	m_kBasicComboData.clear();
	TiXmlElement* pElement = pRoot->FirstChildElement();
	while( pElement )
	{
		char const* pcChildTagName = pElement->Value();
		if(0 != strcmp(pcChildTagName, "WEAPON"))
		{
			break;
		}
		TiXmlAttribute const* pkAttr = pElement->FirstAttribute();
		char const* pcAttrName = pkAttr->Name();
		if(0 != strcmp(pcAttrName, "WEAPON_TYPE"))
		{
			PgXmlError1(pElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
			break;
		}
		int iWeaponType = 0;
		CONT_COMBO_DATA kContComboData;
		char const* pcAttrValue = pkAttr->Value();
		if( !pcAttrValue )
		{
			PgXmlError1(pElement, "XmlParse: Incoreect Attr Value'%s'", pcAttrName);
			break;
		}
		iWeaponType = ::atoi(pcAttrValue);

		TiXmlNode* pNode = pElement->FirstChild();
		if( false == ParseXml_ComboData(pNode, kContComboData) )
		{
			PgXmlError(pElement, "XmlParse: Error ComboData");
			return;
		}

		std::pair<CONT_BASIC_COMBO_DATA::iterator, bool> kRet = m_kBasicComboData.insert( std::make_pair( iWeaponType, kContComboData ) );
		if( false == kRet.second )
		{
			return;
		}
		pElement = pElement->NextSiblingElement();
	}
	return;
}

bool PgSkillTree::ParseXml_ComboData( TiXmlNode const *pNode, CONT_COMBO_DATA &rkContComboData )
{
	if( !pNode )
	{
		return false;
	}
	TiXmlElement const *pElement = dynamic_cast<TiXmlElement const*>(pNode);
	while( pElement )
	{
		//콤보 기본 정보
		stComboData kComboData;
		if( false == ParseXml_ComboInfo(pElement, kComboData) )
		{
			return false;
		}
		// 콤보 스탭 정보
		CONT_COMBO_STEP kContComboStep;
		TiXmlElement const *pChildElement = pElement->FirstChildElement();
		while( pChildElement )
		{
			if( false == ParseXml_ComboStep( pChildElement, kContComboStep ) )
			{
				return false;
			}
			pChildElement = pChildElement->NextSiblingElement();
		}
		kComboData.kContComboStep.swap(kContComboStep);

		CONT_COMBO_DATA::const_iterator iter_data = rkContComboData.begin();
		while( rkContComboData.end() != iter_data )
		{
			if( iter_data->iID == kComboData.iID )
			{
				PgXmlError1(pElement, "XmlParse: Already Input Data - ComboID[%d] ComboLevel[%d] ClassLimit[%d]", kComboData.iID, kComboData.iComboLevel, kComboData.i64ClassLimit);
				return false;
			}
			++iter_data;
		}
		rkContComboData.push_back(kComboData);

		pElement = pElement->NextSiblingElement();
	}

	return true;
}

bool PgSkillTree::ParseXml_ComboInfo( TiXmlElement const *pElement, stComboData &rkComboData )
{
	if( !pElement )
	{
		return false;
	}
	TiXmlAttribute const *pAttr = pElement->FirstAttribute();
	while( pAttr )
	{
		char const* pcAttrName = pAttr->Name();
		char const* pcAttrValue = pAttr->Value();
		if(0 == strcmp(pcAttrName, "ID"))
		{
			rkComboData.iID = ::atoi(pcAttrValue);
		}
		else if(0 == strcmp(pcAttrName, "STAR_COUNT"))
		{
			rkComboData.iStarCount = ::atoi(pcAttrValue);
		}
		else if(0 == strcmp(pcAttrName, "COMBO_LEVEL"))
		{
			rkComboData.iComboLevel = ::atoi(pcAttrValue);
		}
		else if(0 == strcmp(pcAttrName, "START_TYPE"))
		{
			rkComboData.eStartType = GetComboStartType(pcAttrValue);
			if( ECST_ERROR == rkComboData.eStartType )
			{
				return false;
			}
		}
		else if(0 == strcmp(pcAttrName, "CLASS_LIMIT"))
		{
			rkComboData.i64ClassLimit = ::_atoi64(pcAttrValue);
		}
		else
		{
			return false;
		}

		pAttr = pAttr->Next();
	}
	return true;
}

eComboStartType PgSkillTree::GetComboStartType( char const* pcAttrValue )
{
	if( !pcAttrValue )
	{
		return ECST_ERROR;
	}
	else if(0 == strcmp(pcAttrValue, "NORMAL"))
	{	
		return ECST_NORMAL;
	}
	else if(0 == strcmp(pcAttrValue, "DASH"))
	{
		return ECST_DASH;
	}
	return ECST_ERROR;
}

bool PgSkillTree::ParseXml_ComboStep( TiXmlElement const *pElement, CONT_COMBO_STEP &rkContComboStep )
{
	if( !pElement )
	{
		return false;
	}
	stComboStep kComboStep;
	kComboStep.iStep = 0;
	kComboStep.iSkillNo = 0;
	kComboStep.iUiKey = 0;
	kComboStep.bCharge = false;
	kComboStep.eDirect = ECD_NONE;
	TiXmlAttribute const* pAttr = pElement->FirstAttribute();
	while( pAttr )
	{
		char const *pcAttrName = pAttr->Name();
		char const *pcAttrValue = pAttr->Value();
		if( !pcAttrName || !pcAttrValue )
		{
			return false;
		}
		else if(0 == strcmp(pcAttrName, "STEP"))
		{	
			kComboStep.iStep = ::atoi(pcAttrValue);
		}
		else if(0 == strcmp(pcAttrName, "SKILL_NO"))
		{
			kComboStep.iSkillNo = ::atoi(pcAttrValue);
		}
		else if(0 == strcmp(pcAttrName, "INPUT_KEY"))
		{	
			kComboStep.iUiKey = ::atoi(pcAttrValue);
		}
		else if(0 == strcmp(pcAttrName, "IS_CHARGE"))
		{	
			kComboStep.bCharge = (1 == ::atoi(pcAttrValue));
		}
		else if(0 == strcmp(pcAttrName, "ADD_DIRECT"))
		{	
			if( 0 == strcmp( pcAttrValue, "LEFT") )
			{
				kComboStep.eDirect = ECD_LEFT;
			}
			else if( 0 == strcmp( pcAttrValue, "RIGHT") )
			{
				kComboStep.eDirect = ECD_RIGHT;
			}
			else if( 0 == strcmp( pcAttrValue, "UP") )
			{
				kComboStep.eDirect = ECD_UP;
			}
			else if( 0 == strcmp( pcAttrValue, "DOWN") )
			{
				kComboStep.eDirect = ECD_DOWN;
			}
		}
		else
		{
			return false;
		}
		pAttr = pAttr->Next();
	}
	rkContComboStep.push_back(kComboStep);
	return true;
}

bool PgSkillTree::GetContComboData(int const iWeaponType, CONT_COMBO_DATA &rkData)
{
	CONT_BASIC_COMBO_DATA::const_iterator itor_data = m_kBasicComboData.find( iWeaponType );
	if( m_kBasicComboData.end() != itor_data )
	{
		rkData = (*itor_data).second;
		return true;
	}
	return false;
}
bool PgSkillTree::GetComboData( int const iWeaponType, int const iComboID, stComboData &rkData)
{
	CONT_COMBO_DATA kContData;
	if( false == GetContComboData(iWeaponType, kContData) )
	{
		return false;
	}
	CONT_COMBO_DATA::const_iterator itor_data = kContData.begin();
	while( kContData.end() != itor_data )
	{
		if( (*itor_data).iID == iComboID )
		{
			rkData = (*itor_data);
			return true;
		}
		++itor_data;
	}
	return false;
}
bool PgSkillTree::GetContComboStep(int const iWeaponType, int const iComboID, CONT_COMBO_STEP &rkData)
{
	stComboData kComboData;
	if( false == GetComboData(iWeaponType, iComboID, kComboData) )
	{
		return false;
	}
	rkData = kComboData.kContComboStep;
	return true;
}
bool PgSkillTree::GetComboStep(int const iWeaponType, int const iComboID, int const iStep, stComboStep &rkData)
{
	CONT_COMBO_STEP kContStep;
	if( false == GetContComboStep(iWeaponType, iComboID, kContStep) )
	{
		return false;
	}
	CONT_COMBO_STEP::const_iterator itor_step = kContStep.begin();
	while( kContStep.end() != itor_step )
	{
		if( (*itor_step).iStep == iStep )
		{
			rkData = (*itor_step);
			return true;
		}
		++itor_step;
	}
	return false;
}

int PgSkillTree::GetMaxComboCount(int const iWeaponType)
{
	CONT_COMBO_DATA kContData;
	if( true == GetContComboData(iWeaponType, kContData) )
	{
		return kContData.size();
	}
	return 0;
}
int PgSkillTree::GetMaxComboStepCount( int const iWeaponType, int const iComboID)
{
	CONT_COMBO_STEP kContStep;
	if( true == GetContComboStep(iWeaponType, iComboID, kContStep) )
	{
		return kContStep.size();
	}
	return 0;
}

int PgSkillTree::GetCombo_Level( int const iWeaponType, int const iComboID)
{
	stComboData kData;
	if( true == GetComboData(iWeaponType, iComboID, kData) )
	{
		return kData.iComboLevel;
	}
	return 0;
}
int PgSkillTree::GetCombo_StarCount( int const iWeaponType, int const iComboID)
{
	stComboData kData;
	if( true == GetComboData(iWeaponType, iComboID, kData) )
	{
		return kData.iStarCount;
	}
	return 0;
}
int PgSkillTree::GetCombo_StartType( int const iWeaponType, int const iComboID)
{
	stComboData kData;
	if( true == GetComboData(iWeaponType, iComboID, kData) )
	{
		return static_cast<int>(kData.eStartType);
	}
	return 0;
}
__int64 PgSkillTree::GetCombo_ClassLimit( int const iWeaponType, int const iComboID)
{
	stComboData kData;
	if( true == GetComboData(iWeaponType, iComboID, kData) )
	{
		return kData.i64ClassLimit;
	}
	return 0;
}
int PgSkillTree::GetComboStep_SkillNo( int const iWeaponType, int const iComboID, int const iStep)
{
	stComboStep kData;
	if( true == GetComboStep(iWeaponType, iComboID, iStep, kData) )
	{
		return kData.iSkillNo;
	}
	return 0;
}
int PgSkillTree::GetComboStep_UiKey( int const iWeaponType, int const iComboID, int const iStep)
{
	stComboStep kData;
	if( true == GetComboStep(iWeaponType, iComboID, iStep, kData) )
	{
		return kData.iUiKey;
	}
	return 0;
}
eComboDirect PgSkillTree::GetComboStep_Direct( int const iWeaponType, int const iComboID, int const iStep)
{
	stComboStep kData;
	if( true == GetComboStep(iWeaponType, iComboID, iStep, kData) )
	{
		return kData.eDirect;
	}
	return ECD_NONE;
}
bool PgSkillTree::GetComboStep_IsCharge( int const iWeaponType, int const iComboID, int const iStep)
{
	stComboStep kData;
	if( true == GetComboStep(iWeaponType, iComboID, iStep, kData) )
	{
		return kData.bCharge;
	}
	return false;
}

bool PgSkillTree::IsComboData(int const iWeaponType, int const iComboID)
{
	stComboData kData;
	return GetComboData(iWeaponType, iComboID, kData);
}

bool PgSkillTree::IsHaveComboStep(int const iWeaponType, int const iComboID, int const iStep)
{
	stComboStep kData;
	return GetComboStep(iWeaponType, iComboID, iStep, kData);
}

bool PgSkillTree::IsUsableCombo(int const iWeaponType, int const iComboID, int const iCheck_Level, int const iCheck_Class)
{
	stComboData kData;
	if( true == GetComboData(iWeaponType, iComboID, kData) )
	{
		__int64 iUnionClassLimit = (kData.i64ClassLimit << DRAGONIAN_LSHIFT_VAL) | kData.i64ClassLimit;
		if( iCheck_Level >= kData.iComboLevel &&
		   IS_CLASS_LIMIT( iUnionClassLimit, iCheck_Class) )
		{
			return true;
		}
	}
	return false;
}

int PgSkillTree::FindCombo( int const iWeaponType, int const iLevel, int const iClass)
{
	CONT_BASIC_COMBO_DATA::const_iterator itor_data = m_kBasicComboData.find(iWeaponType);
	if( m_kBasicComboData.end() == itor_data )
	{
		return 0;
	}
	CONT_COMBO_DATA::const_iterator itor_combo = (*itor_data).second.begin();
	while( (*itor_data).second.end() != itor_combo )
	{
		if( (*itor_combo).iComboLevel == iLevel &&
			IS_CLASS_LIMIT( (*itor_combo).i64ClassLimit, iClass) )
		{
			return (*itor_combo).iID;
		}
		++itor_combo;
	}
	return 0;
}