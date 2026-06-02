#include "stdafx.h"
#include "XUI/XUI_Manager.h"
#include "Variant/PgQuestInfo.h"
#include "PgQuest.h"
#include "PgQuestMan.h"
#include "PgQuestUI.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "Variant/PgMyQuest.h"
#include "Variant/PgPlayer.h"
#include "PgQuest.h"
#include "PgUIScene.h"
#include "PgActor.h" 
#include "Pg2DString.h"
#include "lwGuid.h"
#include "lwWString.h"
#include "lwQuestMan.h"
#include "PgOption.h"
#include "Serverlib.h"
#include "PgNetwork.h"
#include "PgChatMgrClient.h"

//>>
int Point2Length(POINT2 const rkVal)
{
	return abs((rkVal.x * rkVal.x) + (rkVal.y * rkVal.y));
}
bool EraseSnapChild(ContWndID &rkCont, ContWndID::value_type const &rkID)
{
	ContWndID::iterator find_iter = std::find(rkCont.begin(), rkCont.end(), rkID);
	if( rkCont.end() != find_iter )
	{
		rkCont.erase(find_iter);
		return true;
	}
	return false;
}
void ClearSnapChild(ContWndID &rkCont)
{
	if( !rkCont.empty() )
	{
		ContWndID::const_iterator iter = rkCont.begin();
		while( rkCont.end() != iter )
		{
			PgFormSnapedWnd *pkTempParentWnd = dynamic_cast<PgFormSnapedWnd *>(XUIMgr.Get(*iter));
			if( pkTempParentWnd )
			{
				pkTempParentWnd->ClearSnapParent();
			}
			++iter;
		}
		rkCont.clear();
	}
}
void MoveSnapChild(ContWndID const &rkCont, POINT3I const &rkMove, bool const bIsModifyBoundingBox)
{
	POINT3I kTargetPos;
	CXUI_Wnd *pkWnd = NULL;
	ContWndID::const_iterator iter = rkCont.begin();
	while( rkCont.end() != iter )
	{
		pkWnd = XUIMgr.Get( *iter );
		if( pkWnd )
		{
			POINT3I const &rkPos = pkWnd->Location();
			kTargetPos.Set(rkPos.x + rkMove.x, rkPos.y + rkMove.y, rkPos.z);

			//자식들 이동
			PgFormSnapedWnd *pkSnapedWnd = dynamic_cast<PgFormSnapedWnd *>(pkWnd);
			if( pkSnapedWnd )
			{
				pkSnapedWnd->LocationSnapChild(kTargetPos, bIsModifyBoundingBox);
			}

			// 대상 이동
			pkWnd->Location(kTargetPos.x, kTargetPos.y, kTargetPos.z, bIsModifyBoundingBox);
		}
		
		++iter;
	}
}
bool FindSnapChild(ContWndID const &rkCont, std::wstring const &rkID)
{
	return rkCont.end() != std::find(rkCont.begin(), rkCont.end(), rkID);
}
typedef struct tagMiniQuestWndCompareObject
{
	tagMiniQuestWndCompareObject(std::wstring const& rkWndName)
		: m_pkUserQuestState(NULL), m_pkQuestInfo(NULL)
	{
		PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
			if( pkMyQuest )
			{
				CXUI_Wnd* pkWnd = XUIMgr.Get(rkWndName);
				if( pkWnd )
				{
					int iQuestID = 0;
					if( pkWnd->GetCustomData(&iQuestID, sizeof(iQuestID)) )
					{
						m_pkUserQuestState = pkMyQuest->Get(iQuestID);
						m_pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
					}
				}
			}
		}
	}
	tagMiniQuestWndCompareObject(tagMiniQuestWndCompareObject const& rhs)
		: m_pkUserQuestState(rhs.m_pkUserQuestState), m_pkQuestInfo(rhs.m_pkQuestInfo)
	{
	}
	bool Compare(tagMiniQuestWndCompareObject const& rhs) const
	{
		// operator <
		if( !IsEmpty() && !rhs.IsEmpty() )
		{
			// 퀘스트 완료 했냐 안했냐 우선
			bool const bIsEndQuest = (QS_End == m_pkUserQuestState->byQuestState) || (QS_End == rhs.m_pkUserQuestState->byQuestState);
			if( m_pkUserQuestState->byQuestState != rhs.m_pkUserQuestState->byQuestState
			&&	bIsEndQuest )
			{
				return QS_End == m_pkUserQuestState->byQuestState;
			}
			else
			{
				// 퀘스트 종류에 우선
				bool const bIsScenarioQuest = (QT_Scenario == m_pkQuestInfo->Type()) || (QT_Scenario == rhs.m_pkQuestInfo->Type());
				if( m_pkQuestInfo->Type() != rhs.m_pkQuestInfo->Type()
				&&	bIsScenarioQuest )
				{
					return QT_Scenario == m_pkQuestInfo->Type();
				}
				else
				{
					// 퀘스트 Min 레벨에 우선
					return m_pkQuestInfo->m_kLimit.iMinLevel < rhs.m_pkQuestInfo->m_kLimit.iMinLevel;
				}
			}
		}
		return IsEmpty();
	}
	bool IsEmpty() const
	{
		return (NULL == m_pkUserQuestState) || (NULL == m_pkQuestInfo);
	}

private:
	SUserQuestState const* m_pkUserQuestState;
	PgQuestInfo const* m_pkQuestInfo;
} SMiniQuestWndCompareObject;
bool CompareMinQuestWnd(std::wstring const& lhs, std::wstring const& rhs)
{
	return SMiniQuestWndCompareObject(lhs).Compare(SMiniQuestWndCompareObject(rhs));
}
//<<

//
PgFormSnapedWnd::ContSnapGroup PgFormSnapedWnd::m_kSnapGroup;
PgFormSnapedWnd::ContSnapGroupSaveInfo PgFormSnapedWnd::m_kSaveSnapInfo;

//
void PgFormSnapedWnd::tagSnapSaveInfo::WriteToPacket(BM::Stream& rkPacket) const
{
	rkPacket.Push( kSnapParentID );
	rkPacket.Push( bSnaped );
	PU::TWriteArray_A(rkPacket, kSnapChildTop );
	PU::TWriteArray_A(rkPacket, kSnapChildBottom );
	PU::TWriteArray_A(rkPacket, kSnapChildLeft );
	PU::TWriteArray_A(rkPacket, kSnapChildRight );
	rkPacket.Push( kLocation );
}
void PgFormSnapedWnd::tagSnapSaveInfo::ReadFromPacket(BM::Stream& rkPacket)
{
	rkPacket.Pop( kSnapParentID );
	rkPacket.Pop( bSnaped );
	PU::TLoadArray_A(rkPacket, kSnapChildTop );
	PU::TLoadArray_A(rkPacket, kSnapChildBottom );
	PU::TLoadArray_A(rkPacket, kSnapChildLeft );
	PU::TLoadArray_A(rkPacket, kSnapChildRight );
	rkPacket.Pop( kLocation );
}
size_t PgFormSnapedWnd::tagSnapSaveInfo::min_size() const
{
	return sizeof(kSnapParentID.size()) + sizeof(bSnaped) + (sizeof(kSnapChildTop) * 3) + sizeof(kLocation);
}

//
PgFormSnapedWnd::PgFormSnapedWnd()
{
	m_iSnapGroup = 0;
	m_bSnaped = false;
	m_kSnapParentID.clear();
	m_kSnapChildTop.clear();
	m_kSnapChildBottom.clear();
	m_kSnapChildLeft.clear();
	m_kSnapChildRight.clear();
}

PgFormSnapedWnd::~PgFormSnapedWnd()
{
}

bool PgFormSnapedWnd::VDisplay()
{
	return CXUI_Wnd::VDisplay();
}

void PgFormSnapedWnd::VOnClose()
{
	RemoveSnapGroup(m_iSnapGroup, ID());

	PgFormSnapedWnd *pkTempParentWnd = dynamic_cast<PgFormSnapedWnd *>(XUIMgr.Get( m_kSnapParentID ));
	if( pkTempParentWnd )
	{
		pkTempParentWnd->RemoveSnapChild( ID() );
	}
	::ClearSnapChild(m_kSnapChildTop);
	::ClearSnapChild(m_kSnapChildBottom);
	::ClearSnapChild(m_kSnapChildLeft);
	::ClearSnapChild(m_kSnapChildRight);
	m_kSnapParentID.clear();
	m_bSnaped = false;

	CXUI_Wnd::VOnClose();
}

void PgFormSnapedWnd::VOnCall()
{
	CXUI_Wnd::VOnCall();
	RegistSnapGroup(m_iSnapGroup, ID());
}

void PgFormSnapedWnd::VRegistAttr(std::wstring const &wstrName, std::wstring const &wstrValue)
{
	if( wstrName == _T("SNAP_GROUP") )
	{
		BM::vstring kTemp(wstrValue);
		m_iSnapGroup = (int)kTemp;
	}
	else
	{
		CXUI_Wnd::VRegistAttr(wstrName, wstrValue);
	}
}

void PgFormSnapedWnd::Size(POINT2 const &rPT, bool const bIsModifyBoundingBox)
{
	POINT2 const kDiff = rPT - CXUI_Wnd::Size();
	//::MoveSnapChild(m_kSnapChildTop, kDiff, bIsModifyBoundingBox);
	::MoveSnapChild(m_kSnapChildBottom, POINT3I(0, kDiff.y, 0), bIsModifyBoundingBox);
	//::MoveSnapChild(m_kSnapChildLeft, kDiff, bIsModifyBoundingBox);
	::MoveSnapChild(m_kSnapChildRight, POINT3I(kDiff.x, 0, 0), bIsModifyBoundingBox);

	if( !SnapParentID().empty() )
	{
		PgFormSnapedWnd *pkSnapParentWnd = dynamic_cast<PgFormSnapedWnd *>(XUIMgr.Get( SnapParentID() ));
		if( pkSnapParentWnd )
		{
			POINT3I const kCurPos = CXUI_Wnd::Location();
			ESnapPosType const eSnapPos = pkSnapParentWnd->FindSnapChild( ID() );
			switch( eSnapPos )
			{
			case SnapPT_Top: // 내가 부모 위에 붙었다
				{
					POINT3I const kTargetPos = POINT3I(kCurPos.x, kCurPos.y - kDiff.y, kCurPos.z);
					LocationSnapChild(kTargetPos, bIsModifyBoundingBox);
					CXUI_Wnd::Location(kTargetPos.x, kTargetPos.y, kTargetPos.z);
				}break;
			case SnapPT_Left:
				{
					POINT3I const kTargetPos = POINT3I(kCurPos.x - kDiff.x, kCurPos.y, kCurPos.z);
					LocationSnapChild(kTargetPos, bIsModifyBoundingBox);
					CXUI_Wnd::Location(kTargetPos.x, kTargetPos.y, kTargetPos.z);
				}break;
			}
		}
	}

	CXUI_Wnd::Size(rPT, bIsModifyBoundingBox);
}

void PgFormSnapedWnd::RegistSnapChild(std::wstring const &rkID, ESnapPosType const eSanpPos)
{
	ContWndID *pkTargetCont = NULL;
	switch( eSanpPos )
	{
	case SnapPT_Top:		{ pkTargetCont = &m_kSnapChildTop; }break;
	case SnapPT_Bottom:		{ pkTargetCont = &m_kSnapChildBottom; }break;
	case SnapPT_Left:		{ pkTargetCont = &m_kSnapChildLeft; }break;
	case SnapPT_Right:		{ pkTargetCont = &m_kSnapChildRight; }break;
	default:				{ }break;
	}
	if( !pkTargetCont )
	{
		PG_ASSERT_LOG(false && "Invalide Snap Pos Type")
		return;
	}
	ContWndID::const_iterator find_iter = std::find(pkTargetCont->begin(), pkTargetCont->end(), rkID);
	if( pkTargetCont->end() != find_iter )
	{
		return;
	}
	std::back_inserter(*pkTargetCont) = rkID;
}

ESnapPosType PgFormSnapedWnd::RemoveSnapChild(std::wstring const &rkID)
{
	ESnapPosType eRet = SnapPT_None;
	if( ::EraseSnapChild(m_kSnapChildBottom, rkID) )
	{
		CXUI_Wnd *pkWnd = NULL;
		POINT3I kTargetPos;
		int iSumPos = CXUI_Wnd::Location().y + CXUI_Wnd::Size().y;
		ContWndID::const_iterator iter = m_kSnapChildBottom.begin();
		while( m_kSnapChildBottom.end() != iter )
		{
			pkWnd = XUIMgr.Get( *iter );
			if( pkWnd )
			{
				POINT3I const &rkPos = pkWnd->Location();
				kTargetPos.Set(rkPos.x, iSumPos, rkPos.z);

				if( kTargetPos != pkWnd->Location() )
				{
					PgFormSnapedWnd *pkSnapedWnd = dynamic_cast<PgFormSnapedWnd *>(pkWnd);
					if( pkSnapedWnd )
					{
						pkSnapedWnd->LocationSnapChild(kTargetPos, true);
					}
					pkWnd->Location(kTargetPos.x, kTargetPos.y, kTargetPos.z);
				}
				iSumPos += pkWnd->Size().y;
			}
			++iter;
		}
		eRet = SnapPT_Bottom;
	}
	if( ::EraseSnapChild(m_kSnapChildTop, rkID) )
	{
		PG_ASSERT_LOG(SnapPT_None == eRet && __FUNCTION__ && MB(rkID.c_str()));
		eRet = SnapPT_Top;
	}
	if( ::EraseSnapChild(m_kSnapChildLeft, rkID) )
	{
		PG_ASSERT_LOG(SnapPT_None == eRet && __FUNCTION__ && MB(rkID.c_str()));
		eRet = SnapPT_Left;
	}
	if( ::EraseSnapChild(m_kSnapChildRight, rkID) )
	{
		PG_ASSERT_LOG(SnapPT_None == eRet && __FUNCTION__ && MB(rkID.c_str()));
		eRet = SnapPT_Right;
	}
	return eRet;
}

ESnapPosType PgFormSnapedWnd::FindSnapChild(std::wstring const &rkID)
{
	ESnapPosType eRet = SnapPT_None;
	if( ::FindSnapChild(m_kSnapChildTop, rkID) )
	{
		eRet = SnapPT_Top;
	}
	if( ::FindSnapChild(m_kSnapChildBottom, rkID) )
	{
		PG_ASSERT_LOG(SnapPT_None == eRet && __FUNCTION__ && MB(rkID.c_str()));
		eRet = SnapPT_Bottom;
	}
	if( ::FindSnapChild(m_kSnapChildLeft, rkID) )
	{
		PG_ASSERT_LOG(SnapPT_None == eRet && __FUNCTION__ && MB(rkID.c_str()));
		eRet = SnapPT_Left;
	}
	if( ::FindSnapChild(m_kSnapChildRight, rkID) )
	{
		PG_ASSERT_LOG(SnapPT_None == eRet && __FUNCTION__ && MB(rkID.c_str()));
		eRet = SnapPT_Right;
	}
	return eRet;
}

void PgFormSnapedWnd::LocationSnapChild(POINT3I const &rPT, bool const bIsModifyBoundingBox)
{
	// 자식들을 전부 움직여보자
	POINT3I const kDiff = rPT - CXUI_Wnd::Location();
	::MoveSnapChild(m_kSnapChildTop, kDiff, bIsModifyBoundingBox);
	::MoveSnapChild(m_kSnapChildBottom, kDiff, bIsModifyBoundingBox);
	::MoveSnapChild(m_kSnapChildLeft, kDiff, bIsModifyBoundingBox);
	::MoveSnapChild(m_kSnapChildRight, kDiff, bIsModifyBoundingBox);
}

void PgFormSnapedWnd::NewSnapParnet(std::wstring const &rkNewParentID)
{
	m_kSnapChildTop.clear();
	m_kSnapChildBottom.clear();
	m_kSnapChildLeft.clear();
	m_kSnapChildRight.clear();
	m_kSnapParentID = rkNewParentID;
	m_bSnaped = true;
}

void PgFormSnapedWnd::ClearSnapChild()
{
	::ClearSnapChild(m_kSnapChildTop);
	::ClearSnapChild(m_kSnapChildBottom);
	::ClearSnapChild(m_kSnapChildLeft);
	::ClearSnapChild(m_kSnapChildRight);
	m_kSnapParentID.clear();
	m_bSnaped = false;
}

#ifdef USE_HEAD_SNAP
int PgFormSnapedWnd::GetSnapedNextY()
#else
int PgFormSnapedWnd::GetSnapedNextY(std::wstring &rkOutID)
#endif
{
	if( !m_kSnapChildBottom.empty() )
	{
		ContWndID::reverse_iterator iter = m_kSnapChildBottom.rbegin();
		if( m_kSnapChildBottom.rend() != iter )
		{
			CXUI_Wnd *pkWnd = XUIMgr.Get( *iter );
			if( pkWnd )
			{
				PgFormSnapedWnd *pkSnapedWnd = dynamic_cast<PgFormSnapedWnd *>(pkWnd);
				if( pkSnapedWnd )
				{
#ifdef USE_HEAD_SNAP
					return pkSnapedWnd->GetSnapedNextY();
#else
					return pkSnapedWnd->GetSnapedNextY(rkOutID);
#endif
				}
#ifdef USE_HEAD_SNAP
#else
				rkOutID = pkWnd->ID();
#endif
				return pkWnd->Location().y + pkWnd->Size().y;
			}
		}
	}
#ifdef USE_HEAD_SNAP
#else
	rkOutID = ID();
#endif
	return Location().y + CXUI_Wnd::Size().y;
}

void PgFormSnapedWnd::ReAlignSnapChild(ESnapPosType const eType)
{
	switch( eType )
	{
	case SnapPT_Left:
		{
		}break;
	case SnapPT_Right:
		{
		}break;
	case SnapPT_Top:
		{
		}break;
	case SnapPT_Bottom:
		{
			std::sort( m_kSnapChildBottom.begin(), m_kSnapChildBottom.end(), CompareMinQuestWnd);

			int iNextPos = CXUI_Wnd::Location().y + CXUI_Wnd::Size().y;
			ContWndID::const_iterator loop_iter = m_kSnapChildBottom.begin();
			while( m_kSnapChildBottom.end() != loop_iter )
			{
				CXUI_Wnd *pkWnd = XUIMgr.Get( (*loop_iter) );
				if( pkWnd )
				{
					POINT3I const kCurPos = pkWnd->Location();
					POINT3I const kPos(kCurPos.x, iNextPos, kCurPos.z);
					PgFormSnapedWnd *pkSnapedWnd = dynamic_cast<PgFormSnapedWnd *>( pkWnd );
					if( pkSnapedWnd )
					{
						pkSnapedWnd->LocationSnapChild(kPos, true);
					}

					pkWnd->Location(kPos.x, kPos.y, kPos.z);

					if( pkSnapedWnd )
					{
						iNextPos = pkSnapedWnd->GetSnapedNextY();
					}
					else
					{
						iNextPos = iNextPos + pkWnd->Size().y;
					}
				}
				++loop_iter;
			}
		}break;
	}
}

void PgFormSnapedWnd::SaveSnapInfo(SSnapSaveInfo &rkOut) const
{
	rkOut.kSnapParentID		= m_kSnapParentID;
	rkOut.bSnaped			= m_bSnaped;
	rkOut.kSnapChildTop		= m_kSnapChildTop;
	rkOut.kSnapChildBottom	= m_kSnapChildBottom;
	rkOut.kSnapChildLeft	= m_kSnapChildLeft;
	rkOut.kSnapChildRight	= m_kSnapChildRight;
	rkOut.kLocation			= CXUI_Wnd::Location();
}

void PgFormSnapedWnd::LoadSnapInfo(SSnapSaveInfo const &rkIn)
{
	m_kSnapParentID		= rkIn.kSnapParentID;
	m_bSnaped			= rkIn.bSnaped;
	m_kSnapChildTop		= rkIn.kSnapChildTop;
	m_kSnapChildBottom	= rkIn.kSnapChildBottom;
	m_kSnapChildLeft	= rkIn.kSnapChildLeft;
	m_kSnapChildRight	= rkIn.kSnapChildRight;
	CXUI_Wnd::Location(rkIn.kLocation.x, rkIn.kLocation.y, rkIn.kLocation.z);
}
void PgFormSnapedWnd::WriteToPacketSnapInfo(size_t const iGroup, BM::Stream& rkPacket)
{
	SaveSnapInfo(iGroup);
	ContSnapGroupSaveInfo::iterator find_iter = m_kSaveSnapInfo.find(iGroup);
	if( m_kSaveSnapInfo.end() == find_iter )
	{
		size_t const iEmpty = 0;
		rkPacket.Push( iEmpty );
		return;
	}

	ContSnapSaveInfo const& rkSnapSaveInfo = (*find_iter).second;
	PU::TWriteTable_AM(rkPacket, rkSnapSaveInfo);

	m_kSaveSnapInfo.erase(find_iter);
}
void PgFormSnapedWnd::ReadFromPacketSnapInfo(size_t const iGroup, BM::Stream& rkPacket)
{
	ContSnapSaveInfo kSnapSaveInfo;
	PU::TLoadTable_AM(rkPacket, kSnapSaveInfo);

	m_kSaveSnapInfo.insert( std::make_pair(iGroup, kSnapSaveInfo) );
	LoadSnapInfo(iGroup);
}
void PgFormSnapedWnd::SaveSnapInfo(size_t const iGroup)
{
	std::pair<ContSnapGroupSaveInfo::iterator, bool> kRet = m_kSaveSnapInfo.insert( std::make_pair(iGroup, ContSnapGroupSaveInfo::mapped_type()) );
	if( m_kSaveSnapInfo.end() == kRet.first )
	{
		return;
	}

	ContSnapGroupSaveInfo::mapped_type &rkSnapSave = (*kRet.first).second;
	ContSnapSaveInfo::mapped_type kSnapSaveInfo;
	std::pair<ContSnapSaveInfo::iterator, bool> kRet2;

	ContWndID kVec;
	GetSnapGroup(iGroup, kVec);

	ContWndID::iterator loop_iter = kVec.begin();
	while( kVec.end() != loop_iter )
	{
		PgFormSnapedWnd *pkWnd = dynamic_cast<PgFormSnapedWnd *>( XUIMgr.Get(*loop_iter) );
		if( pkWnd )
		{
			pkWnd->SaveSnapInfo(kSnapSaveInfo);

			kRet2 = rkSnapSave.insert( std::make_pair(pkWnd->ID(), kSnapSaveInfo) );
			if( !kRet2.second )
			{
				(*kRet2.first).second = kSnapSaveInfo;
			}
		}
		++loop_iter;
	}
}

void PgFormSnapedWnd::LoadSnapInfo(size_t const iGroup)
{
	ContSnapGroupSaveInfo::iterator find_iter = m_kSaveSnapInfo.find(iGroup);
	if( m_kSaveSnapInfo.end() == find_iter )
	{
		return;
	}
	ContSnapGroupSaveInfo::mapped_type &rkSnapSave = (*find_iter).second;
	ContSnapSaveInfo::iterator loop_iter = rkSnapSave.begin();
	while( rkSnapSave.end() != loop_iter )
	{
		ContSnapSaveInfo::key_type const &rkKey = (*loop_iter).first;
		ContSnapSaveInfo::mapped_type const &rkSnapSaveInfo = (*loop_iter).second;

		PgFormSnapedWnd *pkWnd = dynamic_cast<PgFormSnapedWnd *>( XUIMgr.Get(rkKey) );
		if( pkWnd )
		{
			pkWnd->LoadSnapInfo(rkSnapSaveInfo);
		}
		++loop_iter;
	}
	m_kSaveSnapInfo.erase(find_iter);
}

void PgFormSnapedWnd::RegistSnapGroup(size_t const iGroup, std::wstring const &rkID)
{
	std::pair<ContSnapGroup::iterator, bool> kRet = m_kSnapGroup.insert( std::make_pair(iGroup, ContSnapGroup::mapped_type()) );
	ContSnapGroup::iterator group_iter = kRet.first;
	if( m_kSnapGroup.end() == group_iter )
	{
		return;
	}

	ContSnapGroup::mapped_type &rkVec = (*group_iter).second;
	ContWndID::iterator find_iter = std::find(rkVec.begin(), rkVec.end(), rkID);
	if( rkVec.end() == find_iter )
	{
		std::back_inserter(rkVec) = rkID;
	}
}

void PgFormSnapedWnd::RemoveSnapGroup(size_t const iGroup, std::wstring const &rkID)
{
	ContSnapGroup::iterator group_iter = m_kSnapGroup.find(iGroup);
	if( m_kSnapGroup.end() == group_iter )
	{
		return;
	}
	ContSnapGroup::mapped_type &rkVec = (*group_iter).second;
	ContWndID::iterator find_iter = std::find(rkVec.begin(), rkVec.end(), rkID);
	if( rkVec.end() == find_iter )
	{
		return;
	}
	rkVec.erase(find_iter);
}

void PgFormSnapedWnd::GetSnapGroup(size_t const iGroup, ContWndID &rkOut)
{
	ContSnapGroup::iterator group_iter = m_kSnapGroup.find(iGroup);
	if( m_kSnapGroup.end() == group_iter )
	{
		return;
	}
	rkOut = (*group_iter).second;
}

bool PgFormSnapedWnd::CanRegistSnapChild(std::wstring const &rkMyID, PgFormSnapedWnd *pkParent)
{
	if( !pkParent )
	{
		return false;
	}

	if( rkMyID == pkParent->SnapParentID() )
	{
		return false;
	}

	std::wstring kParnetID = pkParent->SnapParentID();
	PgFormSnapedWnd *pkSnapedWnd = NULL;
	do
	{
		pkSnapedWnd = dynamic_cast<PgFormSnapedWnd *>(XUIMgr.Get( kParnetID ));
		if( pkSnapedWnd )
		{
			if( rkMyID == pkSnapedWnd->SnapParentID() )
			{
				return false;
			}

			kParnetID = pkSnapedWnd->SnapParentID();
		}
	} while( pkSnapedWnd && !kParnetID.empty() );
	
	return true;
}


//
PgFormSnapedHeadWnd::PgFormSnapedHeadWnd()
{
}

PgFormSnapedHeadWnd::~PgFormSnapedHeadWnd()
{
}

void PgFormSnapedHeadWnd::Location(POINT3I const &rPT, bool const bIsModifyBoundingBox)
{
	LocationSnapChild(rPT, bIsModifyBoundingBox);
	CXUI_Wnd::Location(rPT, bIsModifyBoundingBox);
}

void PgFormSnapedHeadWnd::ReLocationSnapChild()
{
#ifdef USE_HEAD_SNAP
	PgFormSnapedWnd::ClearSnapChild();
	ContWndID kVec;
	GetSnapGroup(m_iSnapGroup, kVec);

	std::sort( kVec.begin(), kVec.end(), CompareMinQuestWnd);

	ContWndID::const_iterator iter = kVec.begin();
	size_t iSumY = CXUI_Wnd::Size().y;
	POINT3I kTargetPos;
	while( kVec.end() != iter )
	{
		CXUI_Wnd *pkSnapWnd = XUIMgr.Get( *iter );
		if( pkSnapWnd
		&&	pkSnapWnd->ID() != ID() )
		{
			kTargetPos = CXUI_Wnd::Location() + POINT3I(0, iSumY, 0);
			pkSnapWnd->Location(kTargetPos.x, kTargetPos.y, kTargetPos.z);
			iSumY += pkSnapWnd->Size().y;

			RegistSnapChild( pkSnapWnd->ID(), SnapPT_Bottom );

			PgFormSnapedWnd *pkSnapedWnd = dynamic_cast<PgFormSnapedWnd *>(pkSnapWnd);
			if( pkSnapedWnd )
			{
				pkSnapedWnd->NewSnapParnet( ID() );
			}
		}
		++iter;
	}
#else
	ContWndID kVec;
	GetSnapGroup(m_iSnapGroup, kVec);

	PgFormSnapedWnd::ClearSnapChild();

	{
		ContWndID::const_iterator iter = kVec.begin();
		POINT3I kTargetPos;
		while( kVec.end() != iter )
		{
			CXUI_Wnd *pkSnapWnd = XUIMgr.Get( *iter );
			if( pkSnapWnd
			&&	pkSnapWnd->ID() != ID() )
			{
				PgFormSnapedSubWnd *pkSnapSubWnd = dynamic_cast<PgFormSnapedSubWnd *>( pkSnapWnd );
				if( pkSnapSubWnd )
				{
					pkSnapSubWnd->ClearSnapChild();
					pkSnapSubWnd->RegistHeadSnapChild();
				}
			}
			++iter;
		}
	}
#endif
}

//
PgFormSnapedSubWnd::PgFormSnapedSubWnd()
{
	m_kLastLocation.Clear();
}

PgFormSnapedSubWnd::~PgFormSnapedSubWnd()
{
}

void PgFormSnapedSubWnd::VOnCall()
{
	PgFormSnapedWnd::VOnCall();
//#ifdef USE_HEAD_SNAP
//	RegistHeadSnapChild();
//#else
//#endif
}

void PgFormSnapedSubWnd::Location(POINT3I const &rPT, bool const bIsModifyBoundingBox)
{
	int const iSnapBoundLength = 9 * 9;
	int const iUnSnapBoundLength = 10 * 10;
	if( m_bSnaped )
	{
		// 떨어져라
		m_kLastLocation += POINT2(rPT.x - CXUI_Wnd::Location().x, rPT.y - CXUI_Wnd::Location().y);
		if( iUnSnapBoundLength < ::Point2Length(m_kLastLocation) )
		{
			POINT2 const kTempPos = CXUI_Wnd::Location() + m_kLastLocation;
			POINT3I const kUnSnapPos(kTempPos.x, kTempPos.y, rPT.z);

			m_kLastLocation.Clear();
			m_bSnaped = false;
			CXUI_Wnd *pkWnd = XUIMgr.Get(m_kSnapParentID);
			PgFormSnapedWnd *pkSnapedWnd = dynamic_cast<PgFormSnapedWnd*>(pkWnd);
			if( pkWnd
			&&	pkSnapedWnd )
			{
				pkSnapedWnd->RemoveSnapChild(ID());
			}
			m_kSnapParentID.clear();
			
			LocationSnapChild(kUnSnapPos, bIsModifyBoundingBox);
			CXUI_Wnd::Location(kUnSnapPos, bIsModifyBoundingBox);
		}
	}
	else
	{
		// 붙어라
		bool bSnaped = false;
		ESnapPosType eSnapPos = SnapPT_None;
		CXUI_Wnd *pkWnd = NULL;
		PgFormSnapedWnd *pkSnapParentWnd = NULL;
		POINT3I kTargetPos = rPT;
		POINT2 kTempPos;
		ContWndID kVec;
		GetSnapGroup(m_iSnapGroup, kVec);
		ContWndID::const_iterator iter = kVec.begin();
		for(; kVec.end() != iter; ++iter)
		{
			ContWndID::value_type const &rkID = (*iter);
			if( ID() == rkID )
			{
				continue;
			}

			pkWnd = XUIMgr.Get( rkID );
			pkSnapParentWnd = dynamic_cast<PgFormSnapedWnd *>(pkWnd);
			if( !pkWnd
			||	!pkSnapParentWnd )
			{
				continue;
			}

			
			if( FindSnapChild(pkSnapParentWnd->ID()) )
			{
				continue;
			}

			POINT2 const kPosXY = POINT2(pkWnd->Location().x, pkWnd->Location().y);
			POINT2 const kSize = pkWnd->Size();
			POINT2 const kMyPosXY = POINT2(CXUI_Wnd::Location().x, CXUI_Wnd::Location().y);

			// 좌상단 기준 체크
			kTempPos = POINT2(kPosXY.x, kPosXY.y - CXUI_Wnd::Size().y) - kMyPosXY; // 상
			bSnaped = iSnapBoundLength > ::Point2Length(kTempPos);
			if( bSnaped )
			{
				kTargetPos = POINT3I(kPosXY.x, kPosXY.y - CXUI_Wnd::Size().y, rPT.z);
				eSnapPos = SnapPT_Top;
				break;
			}
			kTempPos = POINT2(kPosXY.x, kPosXY.y + kSize.y) - kMyPosXY; // 하
			bSnaped = iSnapBoundLength > ::Point2Length(kTempPos);
			if( bSnaped )
			{
				kTargetPos = POINT3I(kPosXY.x, kPosXY.y + kSize.y, rPT.z);
				eSnapPos = SnapPT_Bottom;
				break;
			}
			kTempPos = POINT2(kPosXY.x - CXUI_Wnd::Size().x, kPosXY.y) - kMyPosXY; // 좌
			bSnaped = iSnapBoundLength > ::Point2Length(kTempPos);
			if( bSnaped )
			{
				kTargetPos = POINT3I(kPosXY.x - CXUI_Wnd::Size().x, kPosXY.y, rPT.z);
				eSnapPos = SnapPT_Left;
				break;
			}
			kTempPos = POINT2(kPosXY.x + kSize.x, kPosXY.y) - kMyPosXY; // 우
			bSnaped = iSnapBoundLength > ::Point2Length(kTempPos);
			if( bSnaped )
			{
				kTargetPos = POINT3I(kPosXY.x + kSize.x, kPosXY.y, rPT.z);
				eSnapPos = SnapPT_Right;
				break;
			}
		}

		if( bSnaped
		&&	pkWnd
		&&	pkSnapParentWnd
		&&	eSnapPos != SnapPT_None
		&&	PgFormSnapedWnd::CanRegistSnapChild(ID(), pkSnapParentWnd) )
		{
			m_bSnaped = true;
			m_kSnapParentID = pkSnapParentWnd->ID();
			pkSnapParentWnd->RegistSnapChild( ID(), eSnapPos );

			LocationSnapChild(kTargetPos, bIsModifyBoundingBox);
			CXUI_Wnd::Location(kTargetPos, bIsModifyBoundingBox);
		}
		else
		{
			LocationSnapChild(rPT, bIsModifyBoundingBox);
			CXUI_Wnd::Location(rPT, bIsModifyBoundingBox);
		}
	}
}

void PgFormSnapedSubWnd::RegistHeadSnapChild()
{
	CXUI_Wnd *pkWnd = NULL;
	PgFormSnapedHeadWnd *pkHeadWnd = NULL;
	ContWndID kVec;
	GetSnapGroup(m_iSnapGroup, kVec);
	ContWndID::const_iterator iter = kVec.begin();
	while( kVec.end() != iter )
	{
		pkWnd = XUIMgr.Get( *iter );
		pkHeadWnd = dynamic_cast<PgFormSnapedHeadWnd*>( pkWnd );
		if( pkWnd
		&&	pkHeadWnd )
		{
#ifdef USE_HEAD_SNAP
			POINT3I const kHeadLoc = pkWnd->Location();
			int const iSnapedY = pkHeadWnd->GetSnapedNextY();
			CXUI_Wnd::Location(kHeadLoc.x, iSnapedY);

			m_bSnaped = true;
			pkHeadWnd->RegistSnapChild(ID(), SnapPT_Bottom);
			m_kSnapParentID = pkHeadWnd->ID();
#else
			std::wstring kSnapParentID;
			POINT3I const kHeadLoc = pkWnd->Location();
			int const iSnapedY = pkHeadWnd->GetSnapedNextY(kSnapParentID);
			CXUI_Wnd::Location(kHeadLoc.x, iSnapedY);

			m_bSnaped = true;

			PgFormSnapedWnd *pkSnapParentWnd = dynamic_cast<PgFormSnapedWnd *>( XUIMgr.Get( kSnapParentID ) );
			if( pkSnapParentWnd )
			{
				pkSnapParentWnd->RegistSnapChild(ID(), SnapPT_Bottom);
				m_kSnapParentID = kSnapParentID;
			}
#endif
			break;
		}
		++iter;
	}
}