#include "stdafx.h"
#include "lwSkillTree.h"
#include "lwWString.h"
#include "ServerLib.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "Variant/PgPlayer.h"
#include "Variant/PgControlUnit.h"
#include "lwUnit.h"
#include "lwUI.h"
#include "PgNetwork.h"

lwSkillTree	GetSkillTree()
{
	return	lwSkillTree();
}

bool IsMyPlayerLearnedAnySpecialSkill()
{// 플레이어가
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return false;
	}
	PgMySkill* pMySkill = pkPlayer->GetMySkill();
	if(!pMySkill)
	{
		return false;
	}
	//스페셜 스킬을 소유하고 있는가?
	ContSkillNo kContSkillNo;
	pMySkill->GetHaveSkills(SDT_Special, kContSkillNo);	
	return !kContSkillNo.empty();
}

extern bool lwGetHaveSkill(lwUnit kUnit, int iSkillNo, bool const bOverSkill = false)
{
	if(kUnit.IsNil())
	{
		return false;
	}

	PgControlUnit* pkConUnit = dynamic_cast<PgControlUnit*>(kUnit());
	if(!pkConUnit)
	{
		return false;
	}

	PgMySkill* pkMySkill = pkConUnit->GetMySkill();
	if(!pkMySkill)
	{
		return false;
	}

	return pkMySkill->IsExist(iSkillNo, bOverSkill);
}

void lwSwapStrategySkill()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer ){ return; }

	ESkillTabType eTabType = ESTT_BASIC;
	if( pkPlayer->GetSwapStrategySkillTabNo(eTabType) )
	{
		BM::Stream kPacket(PT_C_M_REQ_CLIENT_CUSTOMDATA);
		kPacket.Push(ECCDT_SKILLTABNO);
		kPacket.Push(static_cast<int>(eTabType));
		NETWORK_SEND(kPacket);
	}
}

bool lwIsOpenStrategySkillSecond()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer ){ return false; }

	return pkPlayer->IsOpenStrategySkill(ESTT_SECOND);
}

int lwGetStrategySkillTabNo()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer ){ return 0; }

	return pkPlayer->GetAbil(AT_STRATEGYSKILL_TABNO);
}

void lwOnClickStrategySkill(int const iType)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer ){ return; }

	if( iType==pkPlayer->GetAbil(AT_STRATEGYSKILL_TABNO) ){ return; }

	ESkillTabType const eTabType = static_cast<ESkillTabType>(iType);
	if( pkPlayer->IsOpenStrategySkill(eTabType) )
	{
		BM::Stream kPacket(PT_C_M_REQ_CLIENT_CUSTOMDATA);
		kPacket.Push(ECCDT_SKILLTABNO);
		kPacket.Push(static_cast<int>(eTabType));
		NETWORK_SEND(kPacket);
	}
}

bool lwSkillTree::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "GetSkillTree", &GetSkillTree);
	def(pkState, "IsMyPlayerLearnedAnySpecialSkill", &IsMyPlayerLearnedAnySpecialSkill);
	def(pkState, "GetHaveSkill", &lwGetHaveSkill);

	def(pkState, "IsOpenStrategySkillSecond", &lwIsOpenStrategySkillSecond);
	def(pkState, "GetStrategySkillTabNo", &lwGetStrategySkillTabNo);
	def(pkState, "SwapStrategySkill", &lwSwapStrategySkill);
	def(pkState, "OnClickStrategySkill", &lwOnClickStrategySkill);

	class_<lwSkillTree>(pkState, "SkillTree")
		.def(pkState, constructor<void>())
		.def(pkState, "GetSkillLevel", &lwSkillTree::GetSkillLevel)
		.def(pkState, "GetOverSkillLevel", &lwSkillTree::GetOverSkillLevel)		
		.def(pkState, "GetMaxSkillLevel", &lwSkillTree::GetMaxSkillLevel)
		.def(pkState, "GetSkillName", &lwSkillTree::GetSkillName)
		.def(pkState, "GetSkillIconKey", &lwSkillTree::GetSkillIconKey)
		.def(pkState, "IsActiveSkill", &lwSkillTree::IsActiveSkill)
		.def(pkState, "IsLearnedSkill", &lwSkillTree::IsLearnedSkill)
		.def(pkState, "IsCommandSkill", &lwSkillTree::IsCommandSkill)
		.def(pkState, "GetRemainSkillPoint", &lwSkillTree::GetRemainSkillPoint)
		.def(pkState, "GetUsedSkillPoint", &lwSkillTree::GetUsedSkillPoint)
		.def(pkState, "CanLevelUp", &lwSkillTree::CanLevelUp)
		.def(pkState, "CanLevelDown", &lwSkillTree::CanLevelDown)
		.def(pkState, "GetSkillNo", &lwSkillTree::GetSkillNo)
		.def(pkState, "GetKeySkillNo", &lwSkillTree::GetKeySkillNo)
		.def(pkState, "NewSkillLearned", &lwSkillTree::NewSkillLearned)
		.def(pkState, "DeleteSkill", &lwSkillTree::DeleteSkill)
		.def(pkState, "GetNextLevelSkillNo", &lwSkillTree::GetNextLevelSkillNo)
		.def(pkState, "GetTotalSkill", &lwSkillTree::GetTotalSkill)
		.def(pkState, "GetKeySkillNoByIndex", &lwSkillTree::GetKeySkillNoByIndex)
		.def(pkState, "GetKeySkillResNoByIndex", &lwSkillTree::GetKeySkillResNoByIndex)
		.def(pkState, "GetKeySkillResNoBySkillNo", &lwSkillTree::GetKeySkillResNoBySkillNo)
		.def(pkState, "GetNeedSkillPoint", &lwSkillTree::GetNeedSkillPoint)
		.def(pkState, "LevelUpTemporary", &lwSkillTree::LevelUpTemporary)
		.def(pkState, "LevelDownTemporary", &lwSkillTree::LevelDownTemporary)
		.def(pkState, "ResetTemporary", &lwSkillTree::ResetTemporary)
		.def(pkState, "ConfirmTemporary", &lwSkillTree::ConfirmTemporary)
		.def(pkState, "IsTemporaryLevelChanged", &lwSkillTree::IsTemporaryLevelChanged)
		.def(pkState, "IsTemporaryRemainSkillPoint", &lwSkillTree::IsTemporaryRemainSkillPoint)
		.def(pkState, "SetRemainSkillPoint", &lwSkillTree::SetRemainSkillPoint)
		.def(pkState, "GetCanLearn", &lwSkillTree::GetCanLearn)
		.def(pkState, "CheckAllNeedSkill", &lwSkillTree::CheckAllNeedSkill)
		.def(pkState, "GetOriginalSkill", &lwSkillTree::GetOriginalSkill)
		.def(pkState, "IsNil", &lwSkillTree::IsNil)
		.def(pkState, "CallComboUI", &lwSkillTree::lwCallComboUI)
		.def(pkState, "GetMaxComboCount", &lwSkillTree::GetMaxComboCount)
		.def(pkState, "SetComboAlramUI", &lwSkillTree::lwSetComboAlramUI)
		;

	return	true;
}

bool lwSkillTree::IsNil()
{
	return	false;	//임시
}

int	lwSkillTree::GetSkillNo(int iKeySkillNo)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);
	if(!pFound)
	{
		return	0;
	}

	return	pFound->m_ulSkillNo;
}
int	lwSkillTree::GetNextLevelSkillNo(int iKeySkillNo)
{
	return	g_kSkillTree.GetNextLevelSkillNo(iKeySkillNo);
}
void	lwSkillTree::SetRemainSkillPoint(int iSkillPoint)
{
	g_kSkillTree.SetRemainSkillPoint(iSkillPoint);
}
int	lwSkillTree::GetRemainSkillPoint()
{
	return	g_kSkillTree.GetRemainSkillPoint();
}
int	lwSkillTree::GetUsedSkillPoint()
{
	return	g_kSkillTree.GetUsedSkillPoint();
}
void lwSkillTree::NewSkillLearned(unsigned long ulSkillNo,int iRemainSkillPoint)
{
	g_kSkillTree.NewSkillLearned(ulSkillNo,iRemainSkillPoint);
}
void lwSkillTree::DeleteSkill(unsigned long ulSkillNo)
{
	g_kSkillTree.DeleteSkill(ulSkillNo);
}
int	lwSkillTree::GetKeySkillNo(int iSkillNo)
{
	return	g_kSkillTree.GetKeySkillNo(iSkillNo);
}
int	lwSkillTree::GetNeedSkillPoint(int iKeySkillNo)
{
	return	g_kSkillTree.GetNeedSkillPoint(iKeySkillNo);
}

bool	lwSkillTree::CanLevelUp(int iKeySkillNo)
{
	return (g_kSkillTree.CanLevelUp(iKeySkillNo) == PgSkillTree::NLR_NONE);
}
bool	lwSkillTree::CanLevelDown(int iKeySkillNo)
{
	return	g_kSkillTree.CanLevelDown(iKeySkillNo);
}
bool	lwSkillTree::IsTemporaryLevelChanged(int iKeySkillNo)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);
	if(!pFound)
	{
		return	false;
	}

	return	pFound->IsTemporaryLevelChanged();
}
bool	lwSkillTree::IsTemporaryRemainSkillPoint(int iSkillNo)
{
	return	g_kSkillTree.IsTemporaryRemainSkillPoint(iSkillNo);
}
int	lwSkillTree::GetSkillLevel(int iKeySkillNo, bool bOverSkill)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);
	if(!pFound)
	{
		return	0;
	}

	if(pFound->m_bLearned == false && pFound->m_bTemporaryLearned == false)
	{
		return 0;
	}

	if(bOverSkill)
	{
		return pFound->GetOverSkillLevel();
	}

	return	pFound->m_pkSkillDef->GetAbil(AT_LEVEL);
}

int lwSkillTree::GetOverSkillLevel(int const iKeySkillNo)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);

	if(!pFound)
	{
		return	0;
	}

	if(pFound->m_bLearned == false && pFound->m_bTemporaryLearned == false)
	{
		return 0;
	}

	return pFound->GetOverSkillLevel();
	
}

int	lwSkillTree::GetMaxSkillLevel(int iKeySkillNo)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);
	if(!pFound)
	{
		return	0;
	}
	return	pFound->GetMaxSkillLevel();
}

lwWString	lwSkillTree::GetSkillName(int iKeySkillNo)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);

	if(pFound == NULL) return lwWString(_T("Undefined"));

	const wchar_t *pText = NULL;
	if(GetDefString(pFound->m_pkSkillDef->NameNo(),pText))
	{
		return	lwWString(pText);
	}
	return lwWString(_T("Undefined"));
}
int	lwSkillTree::GetSkillIconKey(int iKeySkillNo)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);
	if(!pFound)
	{
		return	0;
	}

	return	pFound->m_pkSkillDef->RscNameNo();
}
void	lwSkillTree::LevelUpTemporary(int iKeySkillNo)
{
	g_kSkillTree.LevelUpTemporary(iKeySkillNo);
}
void	lwSkillTree::LevelDownTemporary(int iKeySkillNo)
{
	g_kSkillTree.LevelDownTemporary(iKeySkillNo);
}
void	lwSkillTree::ResetTemporary()
{
	g_kSkillTree.ResetTemporary();
}
void	lwSkillTree::ConfirmTemporary()
{
	g_kSkillTree.ConfirmTemporary();
}
bool	lwSkillTree::IsLearnedSkill(int iKeySkillNo)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);
	if(!pFound)
	{
		return	false;
	}

	return pFound->m_bLearned;
}

bool	lwSkillTree::IsCommandSkill(int iKeySkillNo)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);
	if(pFound)
	{
		CONT_DEFSKILL const* pkContDefMap = NULL;
		g_kTblDataMgr.GetContDef(pkContDefMap);
		if( pkContDefMap )
		{
			CONT_DEFSKILL::const_iterator def_it = pkContDefMap->find(pFound->m_ulKeySkillNo);
			if (pkContDefMap->end() != def_it)
			{
				CONT_DEFSKILL::mapped_type const& kT = (*def_it).second;
				if( 0 != kT.iCmdStringNo )
				{
					return true;
				}
			}
		}
	}
	return false;
}

int		lwSkillTree::GetOriginalSkill(int const iKeySkillNo)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);
	if(!pFound)
	{
		return	0;
	}

	if (!pFound->m_bLearned)
	{
		return iKeySkillNo;
	}

	return pFound->GetOriginalSkillNo();
}

bool	lwSkillTree::IsActiveSkill(int iKeySkillNo)
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(iKeySkillNo);
	if(!pFound)
	{
		return	false;
	}

	BYTE	bySkillType = pFound->m_pkSkillDef->GetType();

	return  ((bySkillType == EST_ACTIVE) || (bySkillType == EST_TOGGLE));

}
int	lwSkillTree::GetTotalSkill()
{
	PgSkillTree::VTree&	kTree = g_kSkillTree.GetTree();
	return	kTree.size();
}
int	lwSkillTree::GetKeySkillNoByIndex(int iIndex)
{
	PgSkillTree::VTree&	kTree = g_kSkillTree.GetTree();

	int	iCount = 0;
	for(PgSkillTree::VTree::iterator itor = kTree.begin(); itor != kTree.end(); itor++)
	{
		if( iCount == iIndex )
		{
			return	itor->first;
		}
		iCount++;
	}

	return	0;
}

int	lwSkillTree::GetKeySkillResNoByIndex(int iIndex)
{
	PgSkillTree::VTree&	kTree = g_kSkillTree.GetTree();
	int	iCount = 0;
	for(PgSkillTree::VTree::iterator itor = kTree.begin(); itor != kTree.end(); itor++)
	{
		if( iCount == iIndex )
		{
			PgSkillTree::stTreeNode *pFound = itor->second;
			if(pFound && pFound->m_pkSkillDef)
			{
				return pFound->m_pkSkillDef->RscNameNo();
			}
			else
			{
				return 0;
			}
		}
		iCount++;
	}
	return 0;
}

int	lwSkillTree::GetKeySkillResNoBySkillNo(int const iNo) const
{
	PgSkillTree::stTreeNode *pFound = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(iNo));
	if(pFound && pFound->m_pkSkillDef)
	{
		return pFound->m_pkSkillDef->RscNameNo();
	}

	return 0;
}

bool	lwSkillTree::GetCanLearn(int iTarget, int iFrom) const
{
	return g_kSkillTree.CanLearn(iTarget, iFrom);
}

bool lwSkillTree::CheckAllNeedSkill(int const iSkillNo) const
{
	return g_kSkillTree.CheckAllNeedSkill(iSkillNo);
}

//--------------ComboSkill UI -------------------
void lwSkillTree::lwCallComboUI( lwUIListItem kListItem, int const iClassView )
{
	lwUIWnd kMain = kListItem.GetWnd();
	XUI::CXUI_Wnd* pkMainWnd = kMain.GetSelf();
	if( !pkMainWnd )
	{
		return;
	}
	lwClearComboList(pkMainWnd);
	//캐릭터 무기번호 세팅
	PgActor* pActor = g_kPilotMan.GetPlayerActor();
	if( !pActor )
	{
		return;
	}
	int const iWeaponType = pActor->GetEquippedWeaponType();

	PgPilot* pPilot = pActor->GetPilot();
	if( !pPilot )
	{
		return;
	}
	int const iClass = pPilot->GetAbil(AT_CLASS);
	int const iLevel = pPilot->GetAbil(AT_LEVEL);
	int const iMaxCount = g_kSkillTree.GetMaxComboCount(iWeaponType);
	if( 0 == iMaxCount )
	{
		return;
	}
	bool bSetFilter = false;
	if( ( IS_CLASS_LIMIT(UCLIMIT_COMMON_WARRIOR, iClass) && PgItemEx::IT_SWORD == iWeaponType ) ||
		( IS_CLASS_LIMIT(UCLIMIT_BATTELMAGE, iClass) && PgItemEx::IT_SPR == iWeaponType ) ||
		( IS_CLASS_LIMIT(UCLIMIT_RANGER, iClass) && PgItemEx::IT_CROSSBOW == iWeaponType ) ||
		( IS_CLASS_LIMIT(UCLIMIT_ASSASSIN, iClass) && PgItemEx::IT_KAT == iWeaponType ))
	{	// 투사 계열-양손검 / 전투마법사 계열-창 / 레인저 계열-석궁 / 어쎄신 계열-카타르
		// 계열과 맞지 않는 아이템(기본아이템)을 착용했을 경우 사용불가능한 콤보는 보여주지 않도록 필터링하자.
		bSetFilter = true;
	}
	int iCount = 1;
	int iComboID = 1;
	while( iCount <= iMaxCount )
	{
		if( g_kSkillTree.IsComboData(iWeaponType, iComboID) )
		{
			bool const bUsable = g_kSkillTree.IsUsableCombo( iWeaponType, iComboID, iLevel, iClass );
			if( bSetFilter &&
				true == bUsable )
			{
				SetCombo(pkMainWnd, iWeaponType, iComboID, bUsable);
			}
			else if( !bSetFilter )
			{
				SetCombo(pkMainWnd, iWeaponType, iComboID, bUsable);
			}
		}
		else
		{
			++iComboID;
			continue;
		}
		++iComboID;
		++iCount;
	}
}
void lwSkillTree::lwClearComboList( XUI::CXUI_Wnd *pWnd )
{
	//콤보 리스트 초기화
	if( !pWnd )
	{
		return;
	}
	XUI::CXUI_Wnd* pListWnd = pWnd->GetControl(L"LST_COMBO");
	if( !pListWnd )
	{
		return;
	}
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
	if( !pList )
	{
		return;
	}
	pList->ClearList();
}
void lwSkillTree::SetCombo( XUI::CXUI_Wnd *pWnd, int const iWeaponType, int const iComboID, bool const bUsable )
{
	if( !pWnd )
	{
		return;
	}
	SetCombo_WeaponName(pWnd, iWeaponType);
	XUI::CXUI_Wnd* pListWnd = pWnd->GetControl(L"LST_COMBO");
	if( !pListWnd )
	{
		return;
	}
	XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pListWnd);
	if( !pList )
	{
		return;
	}
	XUI::SListItem* pItem = pList->AddItem(L"");
	if( !pItem )
	{
		return;
	}
	XUI::CXUI_Wnd* pItemWnd = pItem->m_pWnd;
	if( !pItemWnd )
	{
		return;
	}
	
	//레벨제한 텍스트
	int const iLevel = g_kSkillTree.GetCombo_Level(iWeaponType, iComboID);
	__int64 const i64ClassLimit = g_kSkillTree.GetCombo_ClassLimit(iWeaponType, iComboID);
	SetComboStep_Limit(pItemWnd, i64ClassLimit, iLevel );
	//별 갯수
	int const iStar = g_kSkillTree.GetCombo_StarCount(iWeaponType, iComboID);
	SetComboStep_Star(pItemWnd, iStar, bUsable);

	XUI::CXUI_Wnd* pComboStep = pItemWnd->GetControl(L"FRM_COMBO_STEP");
	if( !pComboStep )
	{
		return;
	}
	//콤보 스탭 정보 입력
	SetComboStep( pComboStep, iWeaponType, iComboID, bUsable);
}
void lwSkillTree::SetCombo_WeaponName( XUI::CXUI_Wnd *pWnd, int const iWeaponType)
{//무기이름
	XUI::CXUI_Wnd* pWeapon = pWnd->GetControl(L"SFRM_WEAPON_TEXT");
	if( !pWeapon )
	{
		return;
	}
	std::wstring kWeaponName;
	if( true == MakeWeaponTypeName( iWeaponType, kWeaponName) )
	{
		pWeapon->Text(kWeaponName);
	}
}
void lwSkillTree::SetComboStep( XUI::CXUI_Wnd *pWnd, int const iWeaponType, int const iComboID, bool const bUsable )
{
	if( !pWnd )
	{
		return;
	}
	// 시작 타입 - 현재 대쉬인지만 체크
	int const iStartType = g_kSkillTree.GetCombo_StartType(iWeaponType, iComboID);
	SetComboStep_StratType(pWnd, iStartType, bUsable);

	int const MAX_STEP_COUNT = 6;
	int const iMaxStepCount = g_kSkillTree.GetMaxComboStepCount( iWeaponType, iComboID);
	int iStep = 1;
	int iCount = 0;
	while( MAX_STEP_COUNT>iCount )
	{
		BM::vstring kStep(L"FRM_COMBO_STEP_ELEMENT#NUM#");
		kStep.Replace(L"#NUM#", iStep-1);
		XUI::CXUI_Wnd* pStepWnd = pWnd->GetControl(kStep);
		if( !pStepWnd )
		{
			break;
		}
		if( !g_kSkillTree.IsHaveComboStep(iWeaponType, iComboID, iStep) )
		{
			pStepWnd->Visible(false);
			++iStep;
			continue;
		}
		pStepWnd->Visible(true);
		int const iSkill = g_kSkillTree.GetComboStep_SkillNo( iWeaponType, iComboID, iStep );
		int const iUiKey = g_kSkillTree.GetComboStep_UiKey( iWeaponType, iComboID, iStep );
		eComboDirect const eDirect = g_kSkillTree.GetComboStep_Direct( iWeaponType, iComboID, iStep );
		bool const bCharge = g_kSkillTree.GetComboStep_IsCharge( iWeaponType, iComboID, iStep );
		if( false == SetComboStep_Skill( pStepWnd, iSkill, bUsable ) )
		{//스킬 셋팅
			pStepWnd->Visible(false);
		}
		if( false == SetComboStep_InputKey(pStepWnd, iUiKey, eDirect, bCharge, bUsable) )
		{//키입력 셋팅
			pStepWnd->Visible(false);
		}
		++iStep;
		++iCount;
	}
}
void lwSkillTree::SetComboStep_Limit( XUI::CXUI_Wnd *pWnd, __int64 i64ClassLimit, int const iLevel )
{//레벨제한 텍스트
	XUI::CXUI_Wnd* pText_LevelLimit = pWnd->GetControl(L"FRM_TEXT");
	if( !pText_LevelLimit )
	{
		return;
	}
	BM::vstring kLevelLimit( TTW(799968) );
	std::wstring wstrClassLimit;
	PgPlayer *pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}
	int iClass = pPlayer->GetAbil(AT_BASE_CLASS);
	if( !IS_CLASS_LIMIT( UCLIMIT_ALL_HUMAN, iClass ) )
	{
		i64ClassLimit = (i64ClassLimit << DRAGONIAN_LSHIFT_VAL);
	}
	MakeClassLimitText(i64ClassLimit, wstrClassLimit);
	kLevelLimit.Replace(L"#CLASS_LIMIT#", wstrClassLimit);
	kLevelLimit.Replace(L"#LEVEL_LIMIT#", iLevel);
	pText_LevelLimit->Text( kLevelLimit );
}
void lwSkillTree::SetComboStep_Star( XUI::CXUI_Wnd *pWnd, int const iStar, bool const bUsable )
{//별 갯수
	int const MAX_STAR_COUNT = 5;
	for( int iCount = 0; iCount < MAX_STAR_COUNT; ++iCount )
	{
		BM::vstring kStar(L"IMG_STAR#NUM#");
		kStar.Replace(L"#NUM#", iCount);
		XUI::CXUI_Wnd* pStarWnd = pWnd->GetControl(kStar);
		if( !pStarWnd )
		{
			break;
		}
		if( iStar <= iCount )
		{
			pStarWnd->Visible(false);
		}
		else
		{
			pStarWnd->Visible(true);
		}
		if( bUsable )
		{
			pStarWnd->GrayScale(false);
		}
		else
		{
			pStarWnd->GrayScale(true);
		}
	}
}
void lwSkillTree::SetComboStep_StratType( XUI::CXUI_Wnd *pWnd, int const iStartType, bool const bUsable )
{//시작 타입 설정
	if( ECST_DASH == iStartType)
	{
		XUI::CXUI_Wnd* pComboStep = pWnd->GetControl(L"IMG_START_TYPE");
		if( !pComboStep )
		{
			return;
		}
		pComboStep->Visible(true);
		if( bUsable )
		{
			pComboStep->GrayScale(false);
		}
		else
		{
			pComboStep->GrayScale(true);
		}
	}
}
bool lwSkillTree::SetComboStep_Skill( XUI::CXUI_Wnd *pWnd, int const iSkillNo, bool const bUsable )
{//스킬 설정
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const *pDef = kSkillDefMgr.GetDef(iSkillNo);
	if( !pDef )
	{
		return false;
	}
	int const iResNo = pDef->RscNameNo();
	const CONT_DEFRES* pContDefRes = NULL;
	g_kTblDataMgr.GetContDef(pContDefRes);
	CONT_DEFRES::const_iterator itor_res = pContDefRes->find(iResNo);
	if( pContDefRes->end() == itor_res )
	{
		return false;
	}
	
	XUI::CXUI_Wnd *pSkillWnd = pWnd->GetControl(L"ICN_SKILL");
	if( !pSkillWnd )
	{
		return false;
	}
	XUI::CXUI_Wnd *pSkillConnectWnd = pWnd->GetControl(L"ICN_SKILL_CONNECT");
	if( !pSkillConnectWnd )
	{
		return false;
	}
	if( iSkillNo )
	{
		pSkillWnd->Visible(true);
		pSkillConnectWnd->Visible(false);

		SUVInfo kUVInfo = pSkillWnd->UVInfo();
		kUVInfo.Set( (*itor_res).second.U, (*itor_res).second.V, (*itor_res).second.UVIndex );
		pSkillWnd->UVInfo(kUVInfo);
		pSkillWnd->DefaultImgName( (*itor_res).second.strIconPath );
		pSkillWnd->SetCustomData( &iSkillNo, sizeof(iSkillNo) );
	}
	else
	{
		pSkillWnd->Visible(false);
		pSkillConnectWnd->Visible(true);
	}

	if( bUsable )
	{
		pSkillWnd->GrayScale(false);
	}
	else
	{
		pSkillWnd->GrayScale(true);
	}
	return true;
}

bool lwSkillTree::SetComboStep_InputKey( XUI::CXUI_Wnd *pWnd, int const iUiKey, eComboDirect const eDirect, bool const bCharge, bool const bUsable )
{
	if(!pWnd)
	{
		return false;
	}
	XUI::CXUI_Wnd *pUiKey_X = pWnd->GetControl(L"IMG_KEY_X");
	XUI::CXUI_Wnd *pUiKey_Z = pWnd->GetControl(L"IMG_KEY_Z");
	XUI::CXUI_Wnd *pUiKey_X_Charge = pWnd->GetControl(L"IMG_KEY_X_CHARGE");
	XUI::CXUI_Wnd *pUiKey_Z_Charge = pWnd->GetControl(L"IMG_KEY_Z_CHARGE");
	XUI::CXUI_Wnd *pUiKey_X_Left	= pWnd->GetControl(L"IMG_KEY_X_LEFT");
	XUI::CXUI_Wnd *pUiKey_X_Right	= pWnd->GetControl(L"IMG_KEY_X_RIGHT");
	XUI::CXUI_Wnd *pUiKey_X_Up		= pWnd->GetControl(L"IMG_KEY_X_UP");
	XUI::CXUI_Wnd *pUiKey_X_Down	= pWnd->GetControl(L"IMG_KEY_X_DOWN");
	XUI::CXUI_Wnd *pUiKey_Z_Left	= pWnd->GetControl(L"IMG_KEY_Z_LEFT");
	XUI::CXUI_Wnd *pUiKey_Z_Right	= pWnd->GetControl(L"IMG_KEY_Z_RIGHT");
	XUI::CXUI_Wnd *pUiKey_Z_Up		= pWnd->GetControl(L"IMG_KEY_Z_UP");
	XUI::CXUI_Wnd *pUiKey_Z_Down	= pWnd->GetControl(L"IMG_KEY_Z_DOWN");

	if( !pUiKey_X || !pUiKey_Z || !pUiKey_X_Charge || !pUiKey_Z_Charge ||
		!pUiKey_X_Left || !pUiKey_X_Right || !pUiKey_X_Up || !pUiKey_X_Down ||
		!pUiKey_Z_Left || !pUiKey_Z_Right || !pUiKey_Z_Up || !pUiKey_Z_Down )
	{
		return false;
	}
	pUiKey_X->Visible(false);
	pUiKey_Z->Visible(false);
	pUiKey_X_Charge->Visible(false);
	pUiKey_Z_Charge->Visible(false);
	pUiKey_X_Left->Visible(false);
	pUiKey_X_Right->Visible(false);
	pUiKey_X_Up->Visible(false);
	pUiKey_X_Down->Visible(false);
	pUiKey_Z_Left->Visible(false);
	pUiKey_Z_Right->Visible(false);
	pUiKey_Z_Up->Visible(false);
	pUiKey_Z_Down->Visible(false);

	switch( iUiKey)
	{
	case 1045:
		{
			switch( eDirect )
			{
			case ECD_NONE:
				{
					if( bCharge )
					{
						pUiKey_X_Charge->Visible(true);
					}
					else
					{
						pUiKey_X->Visible(true);
					}
				}break;
			case ECD_LEFT:
				{
					pUiKey_X_Left->Visible(true);
				}break;
			case ECD_RIGHT:
				{
					pUiKey_X_Right->Visible(true);
				}break;
			case ECD_UP:
				{
					pUiKey_X_Up->Visible(true);
				}break;
			case ECD_DOWN:
				{
					pUiKey_X_Down->Visible(true);
				}break;
			}
		}break;
	case 1044:
		{
			switch( eDirect )
			{
			case ECD_NONE:
				{
					if( bCharge )
					{
						pUiKey_Z_Charge->Visible(true);
					}
					else
					{
						pUiKey_Z->Visible(true);
					}
				}break;
			case ECD_LEFT:
				{
					pUiKey_Z_Left->Visible(true);
				}break;
			case ECD_RIGHT:
				{
					pUiKey_Z_Right->Visible(true);
				}break;
			case ECD_UP:
				{
					pUiKey_Z_Up->Visible(true);
				}break;
			case ECD_DOWN:
				{
					pUiKey_Z_Down->Visible(true);
				}break;
			}
		}break;
	}

	if( bUsable )
	{
		pUiKey_X->GrayScale(false);
		pUiKey_Z->GrayScale(false);
		pUiKey_X_Charge->GrayScale(false);
		pUiKey_Z_Charge->GrayScale(false);
		pUiKey_X_Left->GrayScale(false);
		pUiKey_X_Right->GrayScale(false);
		pUiKey_X_Up->GrayScale(false);
		pUiKey_X_Down->GrayScale(false);
		pUiKey_Z_Left->GrayScale(false);
		pUiKey_Z_Right->GrayScale(false);
		pUiKey_Z_Up->GrayScale(false);
		pUiKey_Z_Down->GrayScale(false);
	}
	else
	{
		pUiKey_X->GrayScale(true);
		pUiKey_Z->GrayScale(true);
		pUiKey_X_Charge->GrayScale(true);
		pUiKey_Z_Charge->GrayScale(true);
		pUiKey_X_Left->GrayScale(true);
		pUiKey_X_Right->GrayScale(true);
		pUiKey_X_Up->GrayScale(true);
		pUiKey_X_Down->GrayScale(true);
		pUiKey_Z_Left->GrayScale(true);
		pUiKey_Z_Right->GrayScale(true);
		pUiKey_Z_Up->GrayScale(true);
		pUiKey_Z_Down->GrayScale(true);
	}
	return true;
}

int lwSkillTree::GetMaxComboCount(int const iWeaponType )
{
	return g_kSkillTree.GetMaxComboCount(iWeaponType);
}

void lwSkillTree::lwSetComboAlramUI(lwUIWnd kWnd, int const iWeaponType, int const iComboID)
{
	if(kWnd.IsNil())
	{
		return;
	}
	//레벨제한 텍스트
	int const iLevel = g_kSkillTree.GetCombo_Level(iWeaponType, iComboID);
	__int64 const i64ClassLimit = g_kSkillTree.GetCombo_ClassLimit(iWeaponType, iComboID);
	SetComboStep_Limit(kWnd.GetSelf(), i64ClassLimit, iLevel );
	//별 갯수
	int const iStar = g_kSkillTree.GetCombo_StarCount(iWeaponType, iComboID);
	SetComboStep_Star(kWnd.GetSelf(), iStar, true);

	XUI::CXUI_Wnd* pComboStep = kWnd.GetSelf()->GetControl(L"FRM_COMBO_STEP");
	if( !pComboStep )
	{
		return;
	}
	//콤보 스탭 정보 입력
	SetComboStep( pComboStep, iWeaponType, iComboID, true);
}
