#include "StdAfx.h"
#include "PgCustomUI_Summmoner.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgWorld.h"
#include "PgAction.h"
#include "lwUI.h"
#include "lwSummon_Info.h"

bool CustomUISelectSummonedSort(PgSelectSummoned const& lhs, PgSelectSummoned const& rhs)
{
	return (lhs.first < rhs.first);
}

bool IsViewScreenSummoned(PgActor * pkSummoner, PgActor * pkSummoned)
{
	if(!pkSummoner || !pkSummoned)
	{
		return false;
	}

	return true;
}

PgCustomUI_Summmoner::PgCustomUI_Summmoner()
{
}

PgCustomUI_Summmoner::~PgCustomUI_Summmoner()
{
}

void PgCustomUI_Summmoner::Init(PgActor * pkActor)
{
	Clear();

	m_pkActor = pkActor;
	if(m_pkActor && m_pkActor->GetPilot() && m_pkActor->GetPilot()->GetUnit())
	{
		m_pkPilot = m_pkActor->GetPilot();
		m_pkUnit = m_pkPilot->GetUnit();
	}
}

bool PgCustomUI_Summmoner::IsUse()const
{
	if(m_pkActor && m_pkPilot && m_pkUnit
	&& m_pkPilot->GetBaseClassID() == UCLASS_SHAMAN)
	{
		return true;
	}
	return false;
}

void PgCustomUI_Summmoner::Clear()
{
	m_pkActor = NULL;
	m_pkPilot = NULL;
	m_pkUnit = NULL;

	m_iSelectSummonedIdx = 0;
	m_kContSelectSummoned.clear();
	m_kSelectSummoned.first = 0;
	m_kSelectSummoned.second.Clear();
	m_eSelectMenuDir = DIR_NONE;
}

void PgCustomUI_Summmoner::UpdateSelectSummond()
{
	if(!IsUse())
	{
		return;
	}

	PgPilot * pkPilot = NULL;
	VEC_SUMMONUNIT const& kContSummonUnit = m_pkUnit->GetSummonUnit();
	for(VEC_SUMMONUNIT::const_iterator c_it=kContSummonUnit.begin(); c_it!=kContSummonUnit.end(); ++c_it)
	{
		pkPilot = g_kPilotMan.FindPilot((*c_it).kGuid);
		if(pkPilot && pkPilot->GetUnit() && pkPilot->GetUnit()->IsUnitType(UT_SUMMONED) && pkPilot->GetUnit()->GetAbil(AT_HP))
		{
			m_kContSelectSummoned.push_back(std::make_pair(pkPilot->GetUnit()->GetAbil(AT_CLASS), (*c_it).kGuid));
		}
	}
	std::sort(m_kContSelectSummoned.begin(), m_kContSelectSummoned.end(), CustomUISelectSummonedSort);
	m_iSelectSummonedIdx = 0;
}

bool PgCustomUI_Summmoner::CallMenu()
{
	if(!IsUse())
	{
		return false;
	}

	/*if(m_pkActor->GetJump())
	{
		return false;
	}*/

	if( false == IsAble() )
	{
		return false;
	}

	if( m_kContSelectSummoned.empty() )//메뉴가 닫히면 순환 소환체 목록은 초기화됨
	{
		UpdateSelectSummond();
	}

	if( m_kContSelectSummoned.empty() )
	{
		return false;
	}
	
	if( m_kContSelectSummoned.empty() )
	{
		return false;
	}

	if( NULL==GetShowMenu() )
	{
		ShowMenu(true);
	}
	else
	{
		int iSelectidx = m_kSelectSummoned.first ? m_iSelectSummonedIdx+1 : m_iSelectSummonedIdx;
		for(int i=0;i<m_kContSelectSummoned.size(); ++i, ++iSelectidx)
		{
			if(iSelectidx>=m_kContSelectSummoned.size())
			{
				break;
			}
			
			CONT_SELECT_SUMMONDED::value_type const& kSelectData = m_kContSelectSummoned.at(iSelectidx);
			PgActor * pkSummonedActor = g_kPilotMan.FindActor(kSelectData.second);
			if( !pkSummonedActor )
			{
				//m_kContSelectSummoned.erase(iSelectidx);
				continue;
			}
			if( !IsViewScreenSummoned(m_pkActor, pkSummonedActor) )
			{
				continue;
			}

			ViewSelectArrow_All(false);
			pkSummonedActor->ViewSelectArrow(true);
			lwSummon_Info::ShowSelectSummon( kSelectData.second );

			m_iSelectSummonedIdx = iSelectidx;
			m_kSelectSummoned = kSelectData;
			return true;
		}
	}

	//전체 소환체에게 화살표 표시
	ViewSelectArrow_All(true);
	
	m_iSelectSummonedIdx = 0;
	m_kSelectSummoned.first = 0;
	m_kSelectSummoned.second.Clear();
	return true;
}

void PgCustomUI_Summmoner::MenuUpdatePos()
{
	if(!IsUse())
	{
		return;
	}

	XUI::CXUI_Wnd * pkWnd = GetShowMenu();
	if(!pkWnd)
	{
		return;
	}

	if(!g_pkWorld || !g_pkWorld->GetCameraMan() || !g_pkWorld->GetCameraMan()->GetCamera())
	{
		return;
	}

	if(NiNodePtr spDummy = (NiNode*)m_pkActor->GetObjectByName(ATTACH_POINT_HEART))
	{
		NiPoint3 const kStarPos = spDummy->GetWorldTranslate();
		float fX = 0.f, fY = 0.f;
		bool const bRet = g_pkWorld->GetCameraMan()->GetCamera()->WorldPtToScreenPt(kStarPos, fX, fY);
		if( bRet )
		{
			int const iScreenWidth = XUIMgr.GetResolutionSize().x;
			int const iScreenHeight = XUIMgr.GetResolutionSize().y;

			POINT2 pt2(iScreenWidth*fX,iScreenHeight*(1-fY));
			pt2.x -= pkWnd->Size().x / 2;
			pt2.y -= pkWnd->Size().y / 2;
			pkWnd->Location(pt2);
		}
	}
}

void PgCustomUI_Summmoner::ShowMenu(bool const bShow)
{
	if(bShow)
	{
		XUIMgr.Activate(L"FRM_CUSTOMUI_SUMMONER");
		MenuUpdatePos();
	}
	else
	{
		XUIMgr.Close(L"FRM_CUSTOMUI_SUMMONER");
	}
}

XUI::CXUI_Wnd* PgCustomUI_Summmoner::GetShowMenu()const
{
	return XUIMgr.Get(L"FRM_CUSTOMUI_SUMMONER");
}

void PgCustomUI_Summmoner::DoCancel()
{
	if(!IsUse())
	{
		return;
	}

	ViewSelectArrow_All(false);
	ShowMenu(false);

	m_iSelectSummonedIdx = 0;
	m_kContSelectSummoned.clear();
	/*m_kSelectSummoned.first = 0;
	m_kSelectSummoned.second.Clear();*/
}

void PgCustomUI_Summmoner::SelectMenu(Direction const eDir)
{
	lua_tinker::call<void,lwUIWnd,int>("CustomUISelect", lwUIWnd(GetShowMenu()), static_cast<int>(eDir));
}

PgAction* PgCustomUI_Summmoner::DoAction(char const * szActionName)
{
	if( !IsUse() )
	{
		return NULL;
	}

	DoCancel();
	return m_pkActor->ReserveTransitAction(szActionName);
}

bool PgCustomUI_Summmoner::IsAllSelectSummoned()
{
	return 0==m_iSelectSummonedIdx && 0==m_kSelectSummoned.first && m_kSelectSummoned.second.IsNull();
}

void PgCustomUI_Summmoner::ViewSelectArrow_All(bool const bShow)
{
	CONT_SELECT_SUMMONDED::const_iterator c_it = m_kContSelectSummoned.begin();
	while(c_it != m_kContSelectSummoned.end())
	{
		if(PgActor * pkActor = g_kPilotMan.FindActor( (*c_it).second ) )
		{
			pkActor->ViewSelectArrow(bShow);
		}
		++c_it;
	}
	lwSummon_Info::SetSelectSummonAll(bShow);
}

int PgCustomUI_Summmoner::FindTargets(PgActionTargetList & rkFoundTargetList)
{
	if(IsUse() && m_kSelectSummoned.first)
	{
		if(PgActor * pkActor = g_kPilotMan.FindActor( m_kSelectSummoned.second ))
		{
			rkFoundTargetList.GetList().push_back(PgActionTargetInfo(m_kSelectSummoned.second,pkActor->GetHitObjectABVIndex()));
		}
	}
	else
	{
		//소환체 전체넣기
		PgActor * pkActor = NULL;
		CUnit * pkUnit = NULL;
		VEC_SUMMONUNIT const& kContSummonUnit = m_pkUnit->GetSummonUnit();
		for(VEC_SUMMONUNIT::const_iterator c_it=kContSummonUnit.begin(); c_it!=kContSummonUnit.end(); ++c_it)
		{
			if(pkActor = g_kPilotMan.FindActor( (*c_it).kGuid ))
			if(pkUnit = pkActor->GetUnit())
			if(pkUnit->IsUnitType(UT_SUMMONED) && pkUnit->GetAbil(AT_HP))
			{
				rkFoundTargetList.GetList().push_back(PgActionTargetInfo( (*c_it).kGuid, pkActor->GetHitObjectABVIndex()));
			}
		}
	}

	return rkFoundTargetList.GetList().size();
}

bool PgCustomUI_Summmoner::IsAble() const
{
	if(NULL == m_pkActor)
	{
		return false;
	}

	if( m_pkActor->IsRidingPet() )
	{// 펫 탑승중이면 사용 불가
		return false;
	}
	
	return true;
}