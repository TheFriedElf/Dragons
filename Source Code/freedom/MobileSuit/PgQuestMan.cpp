#include "stdafx.h"
#include "Variant/PgStringUtil.h"
#include "Variant/PgQuestInfo.h"
#include "PgPilot.h"
#include "PgPilotman.h"
#include "PgOption.h"
#include "PgQuest.h"
#include "PgQuestUI.h"
#include "PgQuestIngList.h"
#include "PgQuestSelectiveList.h"
#include "PgQuestDialog.h"
#include "PgQuestMan.h"
#include "PgWorld.h"
#include "PgNetwork.h"
#include "Pg2DString.h"
#include "lwUI.h"
#include "PgChatMgrClient.h"
#include "PgEventTimer.h"
#include "PgUIScene.h"
#include "PgUIModel.h"
#include "PgAMPool.h"
#include "ServerLib.h"
#include "PgHelpSystem.h"
#include "PgEventScriptSystem.h"
#include "PgQuestFullScreenDialog.h"
#include "PgDailyQuestUI.h"
#include "lwUIQuest.h"
#include "PgMovieMgr.h"
#include "lwUIMission.h"

int const iMaxWarnCount = 3;
extern bool FormatMoney(__int64 const iTotal, std::wstring &rkOut);

namespace PgQuestManUtil
{
	std::wstring const kFullQuestWndName( _T("FRM_QUEST_FULLSCREEN") );
	std::wstring const kWantedQuestWndName( _T("FRM_QUEST_WANTED") );
	std::wstring const kWantedQuestCompleteWndName( _T("FRM_QUEST_WANTED_COMPLETE") );
	std::wstring const kBoardQuestWndName( L"FRM_VILLAGE_BOARD" );

	std::wstring const kCompleteButtonName( _T("BTN_COMPLETE") );
	std::wstring const kAcceptButtonName( _T("BTN_ACCEPT") );
	std::wstring const kRejectButtonName( _T("BTN_REJECT") );
	std::wstring const kGiveupButtonName( _T("Giveup") );
	std::wstring const kFunc1ButtonName( _T("SELECT_1") );
	std::wstring const kFunc2ButtonName( _T("SELECT_2") );
	std::wstring const kRemoteFormName( L"FORM_REMOTE" );
	std::wstring const kRemoteCompleteBtnName( L"BTN_COMPLETE_REMOTE" );
	std::wstring const kRemoteGiveupBtnName( L"BTN_GIVEUP_REMOTE" );

	inline void BuildNpcDailyInfo(int const iQuestID, ContNpcDailyQuest& rkNpcDailyQuest, ContQuestNpc const& rkQuestNpc)
	{
		ContQuestNpc::const_iterator loop_iter = rkQuestNpc.begin();
		while( rkQuestNpc.end() != loop_iter )
		{
			ContQuestNpc::value_type const& rkElement = (*loop_iter);
			ContNpcDailyQuest::iterator find_iter = rkNpcDailyQuest.find( rkElement.kNpcGuid );
			if( rkNpcDailyQuest.end() == find_iter )
			{
				std::pair<ContNpcDailyQuest::iterator, bool> kRet = rkNpcDailyQuest.insert( std::make_pair(rkElement.kNpcGuid, ContQuestID()) );
				find_iter = kRet.first;
			}

			if( rkNpcDailyQuest.end() != find_iter )
			{
				ContQuestID& rkIDVec = (*find_iter).second;
				if( rkIDVec.end() == std::find(rkIDVec.begin(), rkIDVec.end(), iQuestID) )
				{
					std::back_inserter(rkIDVec) = iQuestID;
				}
			}
			++loop_iter;
		}
	}

	// action lock
	void LockPlayerInputMove(int const iLockType)
	{
		g_kPilotMan.LockPlayerInput( SReqPlayerLock(static_cast< EPlayerLockType >(iLockType), true) );
	}
	void UnLockPlayerInputMove(int const iLockType)
	{
		g_kPilotMan.UnlockPlayerInput( SReqPlayerLock(static_cast< EPlayerLockType >(iLockType), true) );
	}

	// move lock
	void LockPlayerInput(int const iLockType)
	{
		g_kPilotMan.LockPlayerInput( SReqPlayerLock(static_cast< EPlayerLockType >(iLockType), false) );
	}
	void UnLockPlayerInput(int const iLockType)
	{
		g_kPilotMan.UnlockPlayerInput( SReqPlayerLock(static_cast< EPlayerLockType >(iLockType), false) );
	}


	void ParseDialogForStep(std::wstring const &wstrOrigin,  std::wstring const &kKey, ContDialogStep& kCont)
	{
		size_t const kSize = kKey.size();
		std::wstring::size_type start_pos = 0;
		int kIndex = 0;
		kCont.clear();
		while(start_pos != std::wstring::npos)
		{
			std::wstring::size_type const find_pos = wstrOrigin.find( kKey.c_str(), start_pos);

			std::wstring strCut;
			if(find_pos != std::wstring::npos)
			{
				strCut = wstrOrigin.substr(start_pos, find_pos - start_pos);
				start_pos = find_pos+kSize;
			}
			else
			{
				strCut = wstrOrigin.substr(start_pos);
				start_pos = std::wstring::npos;
			}

			if(!strCut.empty())
			{
				kCont.insert(std::make_pair(kIndex++, strCut));
			}
		}
	}

	bool IsInResetScheduleQuest(int const iQuestID)
	{
		typedef std::set< int > ContResetQuestSchedule;
		static ContResetQuestSchedule kFindHelper;

		if( kFindHelper.empty() )
		{
			CONT_DEF_QUEST_RESET_SCHEDULE const* pkDef = NULL;
			g_kTblDataMgr.GetContDef(pkDef);
			
			if( !pkDef
			||	0 == pkDef->size() )
			{
				kFindHelper.insert( 0 ); // 비지 않게
				return false;
			}

			CONT_DEF_QUEST_RESET_SCHEDULE::const_iterator iter = pkDef->begin();
			while( pkDef->end() != iter )
			{
				CONT_DEF_QUEST_RESET_SCHEDULE::value_type const& rkDef = (*iter);
				kFindHelper.insert( rkDef.iQuestID );
				++iter;
			}
		}
		return kFindHelper.end() != kFindHelper.find( iQuestID );
	}

	//
	typedef struct tagOrderNpcQuestMark
	{
		explicit tagOrderNpcQuestMark(int const QuestID, EQuestState const QuestState, EQuestState const ChangedState, EQuestMarkDrawOrder const Order)
			: iQuestID(QuestID), eDefaultMark(QuestState), eChangedMark(ChangedState), eOrder(Order)
		{
		}

		explicit tagOrderNpcQuestMark(tagOrderNpcQuestMark const& rhs)
			: iQuestID(rhs.iQuestID), eDefaultMark(rhs.eDefaultMark), eChangedMark(rhs.eChangedMark), eOrder(rhs.eOrder)
		{
		}

		bool operator <(tagOrderNpcQuestMark const& rhs) const
		{
			return eOrder < rhs.eOrder;
		}

		int iQuestID;
		EQuestState eDefaultMark;
		EQuestState eChangedMark;
		EQuestMarkDrawOrder eOrder;
	} SOrderNpcQuestMark;

	//
	SQuestOrderByResult QuestOrderByState(BM::GUID const &rkGuid, const ContNpcQuestInfo& rkVec)
	{
		if( rkVec.empty() )
		{
			return SQuestOrderByResult(0, QS_None);
		}

		typedef std::list< SOrderNpcQuestMark > ContOrderList;
		ContOrderList kOrderList;

		ContNpcQuestInfo::const_iterator iter = rkVec.begin();
		while( rkVec.end() != iter )
		{
			ContNpcQuestInfo::value_type const pkElement = (*iter);
			int const iQuestID = pkElement->iQuestID;
			PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(iQuestID);
			if( !pkQuestInfo )
			{
				NILOG(PGLOG_WARNING, "[PgQuestMan][PgQuestManUtil::QuestOrderByState] Can't find Quest : ID[ %d ]\n", iQuestID);
				++iter;
				continue;
			}

			EQuestState const eNpcTypeDefault = (EQuestState)pkElement->cState;
			EQuestType const eQuestInfoType = pkQuestInfo->Type();

			SCustomQuestMarker kTemp;
			bool const bUseCustomMarker = g_kQuestMan.CheckCustomQuestMarker( iQuestID, kTemp );
			switch( eNpcTypeDefault )
			{
			case QS_Begin:
				{
					EQuestState kState = QS_None;
					EQuestMarkDrawOrder kOrder = EQMDO_HIGH;

					switch( eQuestInfoType )
					{
					case QT_Day:
					case QT_GuildTactics:
					case QT_Normal_Day:
					case QT_Random:
					case QT_RandomTactics:
					case QT_Wanted:
						{ 
							kState = QS_Begin_Tactics;
							kOrder = EQMDO_ONEDAY_START;
						}break;
					case QT_Week:
						{
							kState = QS_Begin_Weekly;
							kOrder = EQMDO_WEEKLY_START;
						}break;
					case QT_Event_Normal:
					case QT_Event_Normal_Day:
						{ 
							kState = QS_Begin_Event;
							kOrder = EQMDO_ONEDAY_START;
						}break;
					case QT_Loop:
						{
							kState = QS_Begin_Loop;
							kOrder = EQMDO_REPEAT_START;
						}break;
					case QT_Event_Loop:
						{
							kState = QS_Begin_Event;
							kOrder = EQMDO_REPEAT_START;
						}break;
					case QT_Scenario:
						{ 
							kState = QS_Begin_Story;
							kOrder = EQMDO_HERO_START;
						}break;
					case QT_Couple:
					case QT_SweetHeart:
						{
							kState = QS_Begin_Couple;
							kOrder = EQMDO_COUPLE_START;
						}break;
					default:
						{
							if( IsInResetScheduleQuest(iQuestID) )
							{
								kState = QS_Begin_Weekly;
								kOrder = EQMDO_WEEKLY_START;
							}
							else
							{
								kState = eNpcTypeDefault;
								kOrder = EQMDO_ADVENTURE_START;
							}
						}break;
					}

					if( bUseCustomMarker )
					{
						kOrder = EQMDO_CUSTOM_START;
					}

					kOrderList.push_back( SOrderNpcQuestMark(iQuestID, eNpcTypeDefault, kState, kOrder) );
				}break;
			case QS_End:
				{
					EQuestState kState = QS_None;
					EQuestMarkDrawOrder kOrder = EQMDO_HIGH;

					switch( eQuestInfoType )
					{
					case QT_Day:
					case QT_GuildTactics:
					case QT_Normal_Day:
					case QT_Random:
					case QT_RandomTactics:
					case QT_Wanted:
						{
							kState = QS_End_Tactics;
							kOrder = EQMDO_ONEDAY_END;
						}break;
					case QT_Week:
						{
							kState = QS_End_Weekly;
							kOrder = EQMDO_WEEKLY_END;
						}break;
					case QT_Event_Normal:
					case QT_Event_Normal_Day:
						{
							kState = QS_End_Event;
							kOrder = EQMDO_ONEDAY_END;
						}break;
					case QT_Loop:
						{
							kState = QS_End_Loop;
							kOrder = EQMDO_REPEAT_END;
						}break;
					case QT_Event_Loop:
						{
							kState = QS_End_Event;
							kOrder = EQMDO_REPEAT_END;
						}break;
					case QT_Scenario:
						{
							kState = QS_End_Story;
							kOrder = EQMDO_HERO_END;
						}break;
					case QT_Couple:
					case QT_SweetHeart:
						{
							kState = QS_End_Couple;
							kOrder = EQMDO_COUPLE_END;
						}break;
					default:
						{
							if( IsInResetScheduleQuest(iQuestID) )
							{
								kState = QS_End_Weekly;
								kOrder = EQMDO_WEEKLY_END;
							}
							else
							{
								kState = eNpcTypeDefault;
								kOrder = EQMDO_ADVENTURE_END;
							}
						}break;
					}

					if( bUseCustomMarker )
					{
						kOrder = EQMDO_CUSTOM_END;
					}

					kOrderList.push_back( SOrderNpcQuestMark(iQuestID, eNpcTypeDefault, kState, kOrder) );
				}break;
			case QS_Begin_NYet:
				{
					kOrderList.push_back( SOrderNpcQuestMark(iQuestID, eNpcTypeDefault, QS_Begin_NYet, EQMDO_HIGH) );
				}break;
			case QS_Ing:
				{
					kOrderList.push_back( SOrderNpcQuestMark(iQuestID, eNpcTypeDefault, QS_Ing, EQMDO_ING) );
				}break;
			default:
				{
					// do nothing
				}break;
			}
			++iter;
		}

		if( kOrderList.empty() )
		{
			return SQuestOrderByResult(0, QS_None);
		}

		kOrderList.sort();
		ContOrderList::reverse_iterator r_itor = kOrderList.rbegin();
		if( r_itor != kOrderList.rend() )
		{
			return SQuestOrderByResult((*r_itor).iQuestID, (*r_itor).eChangedMark);
		}
		return SQuestOrderByResult(0, QS_None);
	}

	void CALLBACK AddError(PgQuestInfoVerifyUtil::SQuestInfoError const& rkNewError)
	{
		// 에러를 쌓지 않고 그때 그때 출력 한다.
		//BM::vstring kTemp1(PgQuestInfoVerifyUtil::GetText(rkNewError.eType)) << _T("-F[") << rkNewError.kFilePathName << _T("][") << rkNewError.iQuestID << _T("]") << __FL2__(rkNewError.szFunc, rkNewError.iLine) << _T(" ") << rkNewError.kErrorMsg;
		BM::vstring kTemp2 = BM::vstring(_T("F[")) << rkNewError.kFilePathName << _T("][") << rkNewError.iQuestID << _T("]") << _T(" ") << rkNewError.kErrorMsg;
		std::string const kTitle( MB(PgQuestInfoVerifyUtil::GetText(rkNewError.eType)) );
		_PgError(kTitle.c_str(), MB(kTemp2.operator const std::wstring &()));
	}

	void CloseAllQuestTalkUI()
	{
		XUIMgr.Close( kFullQuestWndName );
		XUIMgr.Close( kWantedQuestWndName );
		XUIMgr.Close( kWantedQuestCompleteWndName );
		XUIMgr.Close( DailyQuestUI::kDailyQuestWndName );
		XUIMgr.Close( kBoardQuestWndName );
	}

	class PgNpcNameHelper
	{
		typedef std::map< BM::GUID, std::wstring > CONT_NPC_NAME;
	public:
		PgNpcNameHelper()
		{
		}
		~PgNpcNameHelper()
		{
		}

		void Add(BM::GUID const& rkGuid, std::wstring const& rkName)
		{
			m_kContName.insert( std::make_pair(rkGuid, rkName) );
		}
		bool Get(BM::GUID const& rkGuid, std::wstring& rkOut) const
		{
			CONT_NPC_NAME::const_iterator find_iter = m_kContName.find(rkGuid);
			if( m_kContName.end() != find_iter )
			{
				rkOut = (*find_iter).second;
				return true;
			}
			return false;
		}

	private:
		CONT_NPC_NAME m_kContName;
	};
	PgNpcNameHelper kNpcNameHelper;
}

////////////////////////////////////////////////////////
tagItemDependQuestInfo::tagItemDependQuestInfo()
	: kContQuestID()
{
	ClearLastClearCount();
}
tagItemDependQuestInfo::tagItemDependQuestInfo(tagItemDependQuestInfo const& rhs)
	: kContQuestID(rhs.kContQuestID), m_ulLastClearCount(rhs.m_ulLastClearCount)
{
}
tagItemDependQuestInfo::~tagItemDependQuestInfo()
{
}
void tagItemDependQuestInfo::ClearLastClearCount()
{
	m_ulLastClearCount = ULONG_MAX;
}
void tagItemDependQuestInfo::Add(PgQuestInfo const* pkQuestInfo)
{
	if( pkQuestInfo )
	{
		kContQuestID.insert( pkQuestInfo );
	}
}
bool tagItemDependQuestInfo::IsDepend(PgMyQuest const* pkMyQuest) const
{
	// 퀘스트를 종료 했는지 검사해서
	// 퀘스트를 모두 종료 했으면 퀘스트 마크 제거
	// 퀘스트 중에 일일 퀘스트나 반복퀘스트가 껴있으면 제거하지 않는다
	if( kContQuestID.empty() )
	{
		return false;
	}

	if( ULONG_MAX != m_ulLastClearCount )
	{
		return m_ulLastClearCount != kContQuestID.size();
	}

	size_t iClearCount = 0;
	CONT_DEPEND_QUEST_INFO::const_iterator iter = kContQuestID.begin();
	while( kContQuestID.end() != iter )
	{
		CONT_DEPEND_QUEST_INFO::value_type const pkQuestInfo = (*iter);
		switch( pkQuestInfo->Type() )
		{
		case QT_Random: // 일일 퀘스트, 반복 퀘스트는 수량 검사에서 제외
		case QT_RandomTactics:
		case QT_GuildTactics:
		case QT_Day:
		case QT_Loop:
		case QT_Event_Loop:
		case QT_Scroll:
			{
			}break;
		default:
			{
				if( pkMyQuest->IsEndedQuest(pkQuestInfo->ID()) )
				{
					++iClearCount;
				}
			}break;
		}
		++iter;
	}
	m_ulLastClearCount = iClearCount;
	return iClearCount != kContQuestID.size();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgQuestMan
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PgQuestMan::PgQuestMan()
	:m_kPool_NPCQuestInfo(50, 10), m_kPool_QuestInfo(100, 10), m_kMiniListMng(), m_kContDependItem()
{
	Clear();
}

PgQuestMan::~PgQuestMan()
{
	Clear();
	m_kImportanceQuest.clear();
}

void PgQuestMan::Init()
{
#ifndef USE_INB
	// 클라이언트는 문법 검사만 한다.
	PgQuestInfoVerifyUtil::g_lpAddErrorFunc = PgQuestManUtil::AddError; // 메시지 박스로 파싱 에러 출력
#endif
}

void PgQuestMan::Destroy()
{
	BM::CAutoMutex kLock(m_kMutex);

	NPCQuestInfoList::iterator npc_iter = m_SavedNPCQuestInfoList.begin();
	while(m_SavedNPCQuestInfoList.end() != npc_iter)
	{
		m_kPool_NPCQuestInfo.Delete(*npc_iter);
		++npc_iter;
	}
	m_SavedNPCQuestInfoList.clear();
	m_kRecentQuestID = 0;
	m_kRecentDialogID = 0;
	BeginEventID(0);
	IsComplete(0);

	m_kRecentBeginQuest.clear();
	m_ContQuestOut.clear();

	m_kMiniListMng.Destroy();
	m_kContCompleteQuestID.clear();
}

void PgQuestMan::Clear()
{
	SaveMiniQuest();
	Quest::ClearRecvShareQuest();
	Destroy();
	ClearDependQuestItem();
}

void PgQuestMan::AddNpcName(BM::GUID const& rkNpcGuid, std::wstring const& rkName)
{
	BM::CAutoMutex kLock(m_kMutex);
	PgQuestManUtil::kNpcNameHelper.Add(rkNpcGuid, rkName);
}

bool PgQuestMan::FindQuest(int const iQuestID)
{
	BM::CAutoMutex kLock(m_kMutex);
	ContQuestInfo::iterator quest_iter = m_kQuestMap.find(iQuestID);
	return (m_kQuestMap.end() != quest_iter);
}

bool PgQuestMan::CheckThreadLoading(int const iQuestID)
{
	BM::CAutoMutex kLock(m_kMutex);

	CONT_DEF_QUEST_REWARD const* pkDefQuest = NULL;
	g_kTblDataMgr.GetContDef(pkDefQuest);

	if( !pkDefQuest )
	{
		return false;
	}

	CONT_DEF_QUEST_REWARD::const_iterator def_iter = pkDefQuest->find(iQuestID);
	if( def_iter == pkDefQuest->end() )
	{
		return false;
	}


	CONT_DEF_QUEST_REWARD::mapped_type const& rkDefQuest = (*def_iter).second;
	if( PgQuestInfoUtil::IsLoadXmlType(rkDefQuest.iDBQuestType) )
	{
		if( !PgXmlLoader::PrepareXmlDocument(MB(rkDefQuest.kXmlPath), PXDAW_ParseQuest, false) )
		{
			return false;
		}
	}
	return true;
}

const PgQuestInfo* PgQuestMan::GetQuest(int const iQuestID)
{
	if( !iQuestID || USHRT_MAX == iQuestID )
	{
		//PG_ASSERT_LOG(0 && !iQuestID || USHRT_MAX == iQuestID);
		return NULL;//
	}

	//	Quest 폴더에서 해당 ID의 퀘스트 파일을 찾아서 PgQuestInfo 객체를 생성해서 리턴한다.
	BM::CAutoMutex kLock(m_kMutex);
	bool const bFindQuest = FindQuest(iQuestID);

	//없다
	if( !bFindQuest )
	{
		CheckThreadLoading(iQuestID);

		if( FindQuest(iQuestID) )
		{
			// Resume next
		}
		else
		{
#ifndef USE_INB
			static SET_INT kCont;
			if(kCont.end() == kCont.find(iQuestID))
			{
				kCont.insert(iQuestID);
				BM::vstring kTempStr(BM::vstring(L"Can't find Quest XML QuestID[") << iQuestID << L"]");
				std::wstring const& rkStrTemp = kTempStr;
				//::MessageBox(NULL, rkStrTemp.c_str(), GetClientNameW(), MB_OK);
				_PgMessageBox(GetClientName(), MB(rkStrTemp));
			}
#endif
			return NULL;
		}
	}

	ContQuestInfo::iterator quest_iter = m_kQuestMap.find(iQuestID);
	if( m_kQuestMap.end() != quest_iter )
	{
		ContQuestInfo::mapped_type pkQuestInfo = (*quest_iter).second;
		return pkQuestInfo;//있다
	}
	return NULL;
}

bool PgQuestMan::GetQuest(ContQuestInfo& rkCont, EQuestType eType)
{
	BM::CAutoMutex kLock(m_kMutex);

	ContQuestInfo::const_iterator quest_it = m_kQuestMap.begin();
	while(m_kQuestMap.end() != quest_it)
	{
		ContQuestInfo::mapped_type pkQuestInfo = (*quest_it).second;
		if (eType == pkQuestInfo->Type() || eType == QT_None)	//같은 타입만 모으자. 혹은 전부다
		{
			rkCont.insert(std::make_pair((*quest_it).first,(*quest_it).second));
		}
		++quest_it;
	}

	return true;
}

bool PgQuestMan::RemoveQuest(int const iQuestID)
{
	ContQuestInfo::iterator iter = m_kQuestMap.find(iQuestID);
	if( m_kQuestMap.end() == iter )
	{
		return false;
	}

	ContQuestInfo::mapped_type pkElement = (*iter).second;
	if( pkElement )
	{
		m_kPool_QuestInfo.Delete(pkElement);
	}

	m_kQuestMap.erase(iter);
	return true;
}

size_t PgQuestMan::GetNpcDailyInfo(BM::GUID const& rkNpcGuid, ContQuestID& rkOut)const
{
	ContNpcDailyQuest::const_iterator find_iter = m_kDailyNpcQuest.find(rkNpcGuid);
	if( m_kDailyNpcQuest.end() != find_iter )
	{
		rkOut = (*find_iter).second;
		return rkOut.size();
	}
	return 0;
}

void PgQuestMan::BuildNpcDailyInfo(PgQuestInfo const* pkQuestInfo)
{
	if( !pkQuestInfo )
	{
		return;
	}

	int const iQuestID = pkQuestInfo->ID();
	switch( pkQuestInfo->Type() )
	{
	case QT_Day:
	case QT_GuildTactics:
		{
			PgQuestManUtil::BuildNpcDailyInfo(iQuestID, m_kDailyNpcQuest, pkQuestInfo->m_kNpc_Agent);
			PgQuestManUtil::BuildNpcDailyInfo(iQuestID, m_kDailyNpcQuest, pkQuestInfo->m_kNpc_Payer);
		}break;
	case QT_Random:
	case QT_RandomTactics:
		{
			m_kContRandomQuest.insert( iQuestID );
		}break;
	default:
		{
		}break;
	}
}

bool PgQuestMan::IsDependIngQuestMonster(int const iMonsterNo, int const iGroundNo)
{
	PgPlayer * pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}
	PgInventory * pInv = pkPlayer->GetInven();
	if( !pInv )
	{
		return false;
	}
	PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{
		return false;
	}

	ContUserQuestState kVec;
	pkMyQuest->GetQuestList(kVec);

	ContUserQuestState::const_iterator state_iter = kVec.begin();
	while( kVec.end() != state_iter )
	{
		ContUserQuestState::value_type const& rkUserQuestState = (*state_iter);
		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(rkUserQuestState.iQuestID);
		if( pkQuestInfo
		&&	QS_Ing == rkUserQuestState.byQuestState )
		{
			// !! 콤보 퀘스트는 표시에서 제외 한다
			// 몬스터 퀘스트
			ContQuestMonster::const_iterator mon_iter = pkQuestInfo->m_kDepend_Monster.begin();
			while( pkQuestInfo->m_kDepend_Monster.end() != mon_iter )
			{
				bool bIsDependMonster = true;
				ContQuestMonster::mapped_type const& rkQuestMonster = (*mon_iter).second;
				int const iDelegateClassNo = rkQuestMonster.iClassNo - rkQuestMonster.iClassNo%100;
				switch( rkQuestMonster.iType )
				{
				case QET_MONSTER_IncParam_In:
				case QET_MONSTER_IncParam100_In:
				case QET_MONSTER_InstDrop_In:
				case QET_MONSTER_InstDrop100_In:
					{
						bIsDependMonster = iGroundNo == rkQuestMonster.iTargetGroundNo; // 그라운드에서
					} // no break;
				case QET_MONSTER_IncParam:
				case QET_MONSTER_InstDrop:
					{
						if( QET_MONSTER_IncParam100_In == rkQuestMonster.iType ||
							QET_MONSTER_InstDrop100_In == rkQuestMonster.iType )
						{
							bIsDependMonster = bIsDependMonster && (iMonsterNo >= iDelegateClassNo) && (iMonsterNo < iDelegateClassNo+100);// 몬스터 번호가 같은 그룸이라면 (그라운드 체크 때문에 bIsDependMonster 가 선행)
						}
						else
						{
							bIsDependMonster = bIsDependMonster && (iMonsterNo == rkQuestMonster.iClassNo);// 몬스터 번호가 같다면 (그라운드 체크 때문에 bIsDependMonster 가 선행)
						}
						if( bIsDependMonster )
						{
							bIsDependMonster = rkUserQuestState.byParam[rkQuestMonster.iObjectNo] < pkQuestInfo->GetParamEndCondition(rkQuestMonster.iObjectNo);
						}
					}break;
				case QET_MONSTER_IncParam100:
				case QET_MONSTER_InstDrop100:
					{
						bIsDependMonster = bIsDependMonster && (iMonsterNo >= iDelegateClassNo) && (iMonsterNo < iDelegateClassNo+100);// 몬스터 번호가 같은 그룸이라면 (그라운드 체크 때문에 bIsDependMonster 가 선행)
						if( bIsDependMonster )
						{
							bIsDependMonster = rkUserQuestState.byParam[rkQuestMonster.iObjectNo] < pkQuestInfo->GetParamEndCondition(rkQuestMonster.iObjectNo);
						}
					}break;
				case QET_MONSTER_IncParam_M:
				case QET_MONSTER_InstDrop_M:
					{
						bIsDependMonster = bIsDependMonster && (iMonsterNo == rkQuestMonster.iClassNo);
						if( bIsDependMonster )
						{
							bIsDependMonster = (rkQuestMonster.iMissionKey == g_iMissionKey) && (rkQuestMonster.iMissionLevel == lwGetMissionDifficultyLevel()-1);
						}
					}break;
				case QET_MONSTER_IncParam100_M:
				case QET_MONSTER_InstDrop100_M:
					{
						bIsDependMonster = bIsDependMonster && (iMonsterNo >= iDelegateClassNo) && (iMonsterNo < iDelegateClassNo+100);
						if( bIsDependMonster )
						{
							bIsDependMonster = (rkQuestMonster.iMissionKey == g_iMissionKey) && (rkQuestMonster.iMissionLevel == lwGetMissionDifficultyLevel()-1);
						}
					}break;
				case QET_MONSTER_IncParam_MN:
				case QET_MONSTER_InstDrop_MN:
					{
						bIsDependMonster = bIsDependMonster && (iMonsterNo == rkQuestMonster.iClassNo);
						if( bIsDependMonster )
						{
							bIsDependMonster = (rkQuestMonster.iMissionKey == g_iMissionKey) 
								&& (rkQuestMonster.iMissionLevel == lwGetMissionDifficultyLevel()-1 )
								&& (rkQuestMonster.iStageCount == lwGetMissionMapCount()-1 );		//맵 갯수를 서버와 통일하기위해 하나 뺀다.
						}
					}break;
				case QET_MONSTER_IncParam100_MN:
				case QET_MONSTER_InstDrop100_MN:
					{
						bIsDependMonster = bIsDependMonster && (iMonsterNo >= iDelegateClassNo) && (iMonsterNo < iDelegateClassNo+100);
						if( bIsDependMonster )
						{
							bIsDependMonster = (rkQuestMonster.iMissionKey == g_iMissionKey) 
								&& (rkQuestMonster.iMissionLevel == lwGetMissionDifficultyLevel()-1 )
								&& (rkQuestMonster.iStageCount == lwGetMissionMapCount()-1 );		//맵 갯수를 서버와 통일하기위해 하나 뺀다.
						}
					}break;
				case QET_MONSTER_ComboCheck:
				default:
					{
						bIsDependMonster = false;
					}break;
				}
				if( bIsDependMonster )
				{//몬스터가 주는 아이템을 퀘스트 조건 이상 가지고 있으면 false
					SItemPos kItemPos;
					if( 0!=rkQuestMonster.iItemNo && S_OK==pInv->GetFirstItem(rkQuestMonster.iItemNo, kItemPos) )
					{
						PgBase_Item kItem;
						pInv->GetItem(kItemPos, kItem);
						CONT_DEL_QUEST_ITEM::const_iterator item_iter = pkQuestInfo->m_kCompleteDeleteItem.begin();
						while( pkQuestInfo->m_kCompleteDeleteItem.end() != item_iter )
						{
							if( (*item_iter).iItemNo == kItem.ItemNo() )
							{
								if( kItem.Count() >= (*item_iter).iCount )
								{
									bIsDependMonster = false;
								}
							}
							++item_iter;
						}
						if( bIsDependMonster )
						{
							return true;
						}
					}
					else
					{
						return true;
					}
				}
				++mon_iter;
			}

			//그라운드 퀘스트
			ContQuestGround::const_iterator gnd_iter = pkQuestInfo->m_kDepend_Ground.begin();
			while( pkQuestInfo->m_kDepend_Ground.end() != gnd_iter )
			{
				bool bIsDependGround = false;
				ContQuestGround::mapped_type const& rkQuestGround = (*gnd_iter).second;
				switch( rkQuestGround.iType )
				{
				case QET_GROUND_KillMonster:
				case QET_GROUND_MonsterDrop:
					{
						if( iGroundNo == rkQuestGround.iGroundNo )
						{
							bIsDependGround = rkUserQuestState.byParam[rkQuestGround.iObjectNo] < pkQuestInfo->GetParamEndCondition(rkQuestGround.iObjectNo);
						}
					}break;
				default:
					{
						bIsDependGround = false;
					}break;
				}
				if( bIsDependGround )
				{
					return true;
				}
				++gnd_iter;
			}
		}
		++state_iter;
	}
	return false;
}

void PgQuestMan::BuildDependItem(PgQuestInfo const* pkQuestInfo)
{
	if( !pkQuestInfo )
	{
		return;
	}

	CONT_DEL_QUEST_ITEM::const_iterator iter = pkQuestInfo->m_kCompleteDeleteItem.begin();
	while( pkQuestInfo->m_kCompleteDeleteItem.end() != iter )
	{
		CONT_DEL_QUEST_ITEM::value_type const& rkDependItem = (*iter);

		CONT_QUEST_DEPEND_ITEM::iterator find_iter = m_kContDependItem.find(rkDependItem.iItemNo);
		if( m_kContDependItem.end() == find_iter )
		{
			std::pair<CONT_QUEST_DEPEND_ITEM::iterator, bool> kRet = m_kContDependItem.insert( std::make_pair(rkDependItem.iItemNo, CONT_QUEST_DEPEND_ITEM::mapped_type()) );
			if( kRet.second )
			{
				find_iter = kRet.first;
			}
		}

		if( m_kContDependItem.end() != find_iter )
		{
			(*find_iter).second.Add(pkQuestInfo);
		}
		++iter;
	}
}
bool PgQuestMan::IsDependQuestItem(int const iItemNo) const
{
	// Ver 1
	//return m_kContDependItem.end() != m_kContDependItem.find(iItemNo);

	// Ver 2
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	CONT_QUEST_DEPEND_ITEM::const_iterator find_iter = m_kContDependItem.find(iItemNo);
	if( m_kContDependItem.end() == find_iter )
	{
		return false;
	}
	return (*find_iter).second.IsDepend(pkPlayer->GetMyQuest());
}
void PgQuestMan::ClearDependQuestItem(bool const bNeedUpdate)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}
	PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{
		return;
	}

	CONT_QUEST_DEPEND_ITEM::iterator iter = m_kContDependItem.begin();
	while( m_kContDependItem.end() != iter )
	{
		(*iter).second.ClearLastClearCount();
		if( bNeedUpdate )
		{
			(*iter).second.IsDepend(pkMyQuest);
		}
		++iter;
	}
}


bool PgQuestMan::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	if( pArg )
	{
		PG_ASSERT_LOG(0 && "pArg not use");
	}
	if( !pkNode )
	{
		return false;
	}

	char const* szName = pkNode->Value();
	if( 0 == strcmp("QUEST", szName) )
	{
		BM::CAutoMutex kLock(m_kMutex);
		PgQuestInfo* pkNewQuest = m_kPool_QuestInfo.New();
		if( !pkNewQuest )
		{
			PG_ASSERT_LOG(!"Can't New Memory");
			return false;
		}

		pkNewQuest->Clear();

		if( pkNewQuest->ParseXml((TiXmlElement*)pkNode) )
		{
			int const iQuestID = pkNewQuest->ID();

			if( !FindQuest(iQuestID) )
			{
				pkNewQuest->BuildFromDef(); // 두 함수 순서 바꾸지 말것
				pkNewQuest->Build();
				
				std::pair<ContQuestInfo::iterator, bool> kRet = m_kQuestMap.insert(std::make_pair(iQuestID, pkNewQuest));
				if( kRet.second )
				{
					BuildNpcDailyInfo(pkNewQuest);
					BuildDependItem(pkNewQuest);
					return true;
				}
			}
		}
		m_kPool_QuestInfo.Delete(pkNewQuest);
	}
	else if( 0 == strcmp("REALM_QUEST_EVENT", szName) )
	{
		PgRealmQuestEvent kRealmQuestEvent;
		if( kRealmQuestEvent.ParseXml(dynamic_cast< TiXmlElement const*>(pkNode)) )
		{
			m_kContRealmQuestEvent.insert( std::make_pair(kRealmQuestEvent.RealmQuestID(), kRealmQuestEvent) );
			return true;
		}
	}
	else if( 0 == strcmp("CUSTOM_QUEST_MARKER", szName) )
	{
		return ParseCustomQuestMarker((TiXmlElement*)pkNode);
	}
	else
	{
		if( pkNode )
		{
			return ParseImportanceQuest((TiXmlElement*)pkNode);
		}
	}
	return false;
}

void PgQuestMan::UpdateRealmQuestCount(SRealmQuestInfo const& rkRealmQuestInfo)
{
	CONT_REALM_QUEST_EVENT::iterator iter = m_kContRealmQuestEvent.find(rkRealmQuestInfo.kRealmQuestID);
	if( m_kContRealmQuestEvent.end() == iter )
	{
		return;
	}
	CONT_REALM_QUEST_EVENT::mapped_type& rkRealmQuestEvent = (*iter).second;
	rkRealmQuestEvent.UpdateCount(rkRealmQuestInfo);
}
void PgQuestMan::UpdateRealmQuestNpc(BM::GUID const& rkNpcGuid)
{
	CONT_REALM_QUEST_EVENT::iterator iter = m_kContRealmQuestEvent.begin();
	while( m_kContRealmQuestEvent.end() != iter )
	{
		CONT_REALM_QUEST_EVENT::mapped_type& rkRealmQuestEvent = (*iter).second;
		if( rkRealmQuestEvent.IsLikedNpc(rkNpcGuid) )
		{
			rkRealmQuestEvent.UpdateCount(rkRealmQuestEvent.RealmQuestInfo());
		}
		++iter;
	}
}
void PgQuestMan::CallRealmQuestBuffNotice(std::wstring const& rkRealmQuestID, bool const bSuccess, DWORD const dwNowSec) const
{
	CONT_REALM_QUEST_EVENT::const_iterator iter = m_kContRealmQuestEvent.find(rkRealmQuestID);
	if( m_kContRealmQuestEvent.end() == iter )
	{
		return;
	}
	CONT_REALM_QUEST_EVENT::mapped_type const& rkRealmQuestEvent = (*iter).second;

	std::wstring kMessage;
	if( !bSuccess )
	{
		// 실패
		if( 0 != rkRealmQuestEvent.BuffFailTextID() )
		{
			kMessage = TTW(rkRealmQuestEvent.BuffFailTextID());
		}
	}
	else
	{
		// 성공
		DWORD const dwOneMinuteSec = 60;
		DWORD dwValue = dwNowSec;
		if( dwOneMinuteSec <= dwNowSec )
		{
			if( 0 != rkRealmQuestEvent.BuffMinuteTextID() ) // 1분전
			{
				kMessage = TTW(rkRealmQuestEvent.BuffMinuteTextID());
				dwValue = dwNowSec / dwOneMinuteSec;
			}
		}
		else
		{
			if( 0 == dwNowSec )
			{
				if( 0 != rkRealmQuestEvent.BuffNowTextID() ) // 지금
				{
					kMessage = TTW(rkRealmQuestEvent.BuffNowTextID());
				}
			}
			else
			{
				if( 0 != rkRealmQuestEvent.BuffSecTextID() ) // 초 전
				{
					kMessage = TTW(rkRealmQuestEvent.BuffSecTextID());
				}
			}
		}
		//
		PgStringUtil::ReplaceStr(kMessage, std::wstring(L"$RemainTime$"), static_cast< std::wstring >(BM::vstring(dwValue)), kMessage);
	}

	if( !kMessage.empty() )
	{
		g_kChatMgrClient.AddLogMessage(SChatLog(CT_EVENT_SYSTEM), kMessage, true);
	}
}
void PgQuestMan::CallRealmQuestUI(SRealmQuestInfo const& rkRealmQuestInfo)
{
	UpdateRealmQuestCount(rkRealmQuestInfo); // 

	CONT_REALM_QUEST_EVENT::iterator iter = m_kContRealmQuestEvent.find(rkRealmQuestInfo.kRealmQuestID);
	if( m_kContRealmQuestEvent.end() == iter )
	{
		return;
	}
	CONT_REALM_QUEST_EVENT::mapped_type& rkRealmQuestEvent = (*iter).second;

	std::wstring const kRealmQuestUI(L"SFRM_REALM_QUEST");
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(kRealmQuestUI);
	if( !pkTopWnd )
	{
		pkTopWnd = XUIMgr.Call(kRealmQuestUI);
		if( !pkTopWnd )
		{
			return;
		}
	}
	lwRegistUIAction(lwUIWnd(pkTopWnd), "CloseUI");

	XUI::CXUI_Wnd* pkDescWnd = pkTopWnd->GetControl(L"FRM_DESC");
	if( pkDescWnd )
	{
		pkDescWnd->Text( TTW(rkRealmQuestEvent.DescTextID()) );
	}

	XUI::CXUI_Wnd* pkRewardWnd = pkTopWnd->GetControl(L"FRM_REWARD");
	if( pkRewardWnd )
	{
		pkRewardWnd->Text( TTW(rkRealmQuestEvent.RewardTextID()) );
	}
	UpdateRealmQuestCountUI(pkTopWnd, rkRealmQuestEvent);
}

void PgQuestMan::CallRealmQuestMiniUI(SRealmQuestInfo const& rkRealmQuestInfo)
{
	UpdateRealmQuestCount(rkRealmQuestInfo); // 

	CONT_REALM_QUEST_EVENT::iterator iter = m_kContRealmQuestEvent.find(rkRealmQuestInfo.kRealmQuestID);
	if( m_kContRealmQuestEvent.end() == iter )
	{
		return;
	}
	CONT_REALM_QUEST_EVENT::mapped_type& rkRealmQuestEvent = (*iter).second;

	//
	std::wstring const kRealmQuestMiniUIName(L"FRM_REALM_QUEST_STATUS");
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(kRealmQuestMiniUIName);
	if( !pkTopWnd )
	{
		pkTopWnd = XUIMgr.Call(kRealmQuestMiniUIName);
		if( !pkTopWnd )
		{
			return;
		}
	}
	UpdateRealmQuestCountUI(pkTopWnd, rkRealmQuestEvent);
}
void PgQuestMan::UpdateRealmQuestCountUI(XUI::CXUI_Wnd* pkTopWnd, CONT_REALM_QUEST_EVENT::mapped_type const& rkRealmQuestEvent) const
{
	if( !pkTopWnd )
	{
		return;
	}
	XUI::CXUI_Wnd* pkTitleWnd = pkTopWnd->GetControl(L"FRM_TITLE");
	if( pkTitleWnd )
	{
		pkTitleWnd->Text( TTW(rkRealmQuestEvent.TitleTextID()) );
	}

	XUI::CXUI_Wnd* pkCountWnd = pkTopWnd->GetControl(L"FRM_COUNT");
	if( pkCountWnd )
	{
		std::wstring kTemp = TTW(460103);
		PgStringUtil::ReplaceStr(kTemp, std::wstring(L"$CurrentCount$"), static_cast< std::wstring >(BM::vstring(rkRealmQuestEvent.RealmQuestInfo().iCurCount)), kTemp);
		PgStringUtil::ReplaceStr(kTemp, std::wstring(L"$TargetCount$"), static_cast< std::wstring >(BM::vstring(rkRealmQuestEvent.RealmQuestInfo().iNextCount)), kTemp);
		pkCountWnd->Text( kTemp );
	}
}

//void PgQuestMan::SetRecentQObject(int const iObjType, BM::GUID const &rkGuid)
//{
//	//	가장 최근에 클릭한 퀘스트 오브젝트를 설정한다.
//	BM::CAutoMutex kLock(m_kMutex);
//	m_iRecentQObjectType = iObjType;
//	m_kRecentQObjectGUID = rkGuid;
//}

//void PgQuestMan::GetRecentQObject(int &iObjType, BM::GUID &rkGuid)
//{
//	BM::CAutoMutex kLock(m_kMutex);
//	iObjType = m_iRecentQObjectType;
//	rkGuid = m_kRecentQObjectGUID;
//}

void PgQuestMan::SaveNPCQuestInfo(const SNPCQuestInfo& rkQuestSimpleInfo)
{
	BM::CAutoMutex kLock(m_kMutex);
	NPCQuestInfoList::const_iterator iter = m_SavedNPCQuestInfoList.begin();
	while( m_SavedNPCQuestInfoList.end() != iter )
	{
		if( (*iter)
		&&	SNPCQuestInfo::SimpleEqual(*(*iter), rkQuestSimpleInfo) )
		{
			break;
		}
		++iter;
	}

	if( m_SavedNPCQuestInfoList.end() != iter )
	{
		(*iter)->cState = rkQuestSimpleInfo.cState;
	}
	if( m_SavedNPCQuestInfoList.end() == iter )
	{
		SNPCQuestInfo	*pNewInfo = m_kPool_NPCQuestInfo.New();
		*pNewInfo = rkQuestSimpleInfo;
		m_SavedNPCQuestInfoList.push_back(pNewInfo);
	}
}

size_t PgQuestMan::PopNPCQuestInfo(BM::GUID const &kNPC_GUID, ContNpcQuestInfo &rkVec) const
{
	BM::CAutoMutex kLock(m_kMutex);
	NPCQuestInfoList::const_iterator iter = m_SavedNPCQuestInfoList.begin();
	while(m_SavedNPCQuestInfoList.end() != iter)
	{
		if((*iter)->kNpcGuid  == kNPC_GUID)
		{
			if( (*iter)->cState == QS_End )
			{
				rkVec.push_front((*iter));
			}
			else
			{
				rkVec.push_back((*iter));
			}
		}
		++iter;
	}
	return rkVec.size();
}

void PgQuestMan::ClearNPCQuestInfo()
{
	BM::CAutoMutex kLock(m_kMutex);
	NPCQuestInfoList::iterator kIter = m_SavedNPCQuestInfoList.begin();
	for(; m_SavedNPCQuestInfoList.end() != kIter; ++kIter)
	{
		SNPCQuestInfo *pInfo = *kIter;
		if(!pInfo) continue;

		PgPilot *pkPilot = g_kPilotMan.FindPilot(pInfo->kNpcGuid);
		if(pkPilot)
		{
			PgActor *pkActor = dynamic_cast<PgActor *>(pkPilot->GetWorldObject());
			if(pkActor) pkActor->ClearQuestInfo();
		}

		m_kPool_NPCQuestInfo.Delete(pInfo);
	}
	m_SavedNPCQuestInfoList.clear();
}

size_t PgQuestMan::CanNPCQuestInfo(BM::GUID const &rkNpcGuid)
{
	ContNpcQuestInfo kQuestVec;
	PopNPCQuestInfo(rkNpcGuid, kQuestVec);

	size_t iCount = 0;
	ContNpcQuestInfo::iterator iter = kQuestVec.begin();
	while(kQuestVec.end() != iter)
	{
		const ContNpcQuestInfo::value_type pkElement = (*iter);

		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest( (*pkElement).iQuestID );
		if( pkQuestInfo )
		{
			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if(pPlayer)
			{
				if( QBL_SkillHave == PgQuestInfoUtil::CheckBeginQuest(const_cast<PgPlayer const *>(pPlayer), (*pkElement).iQuestID, pkQuestInfo))
				{
					++iter;
					continue;
				}
			}

			bool const IsDailyQuest = PgQuestInfoUtil::IsDailyUIQuest(pkQuestInfo);
			if( !IsDailyQuest )
			{
				switch( (*pkElement).cState )
				{
				case QS_Ing:
				case QS_Begin:
				case QS_End:
					{
								++iCount;
					}break;
				default:
					{
					}break;
				}
			}
		}
		++iter;
	}
	return iCount;
}





/////////////////////////////////////////////////////////////////////////////////

void PgQuestMan::Send_Drop_Quest(int const iQuestID)
{
	if( !iQuestID )
	{
		return;
	}

	//	특정 퀘스트를 포기할 것을 서버에 전송한다.
	BM::Stream kPacket(PT_C_M_REQ_DROPQUEST, iQuestID);
	NETWORK_SEND(kPacket)
}


void PgQuestMan::Send_Dialog_Select(int const iQuestID, int const iDialogID, int const iSelectID, int const iVal1, int const iVal2)
{
	BM::Stream kPacket(PT_C_M_ANS_SHOWDIALOG);
	kPacket.Push( m_kTalkSessionGuid );
	kPacket.Push( NowInfo().kGuid );
	kPacket.Push( iQuestID );
	kPacket.Push( iDialogID );
	kPacket.Push( iSelectID );
	kPacket.Push( iVal1 );
	kPacket.Push( iVal2 );
	NETWORK_SEND(kPacket)
}

void PgQuestMan::Send_ReqDialogClose(int const iQuestID, int const iDialogID)
{
	Send_Dialog_Select(iQuestID, iDialogID, 0, 0, 0);
}

bool PgQuestMan::CanNextDialog(const PgQuestInfo* pkQuestInfo, const SQuestDialog* pkCurDialog, int const iNextDialogID)
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	if( iNextDialogID
	&&	QDT_Complete == pkCurDialog->eType )
	{
		const SQuestReward& rkQuestReward = pkQuestInfo->m_kReward;
		int const iClassNo = pkPlayer->GetAbil(AT_CLASS);
		int const iGender = pkPlayer->GetAbil(AT_GENDER);
		ContRewardVec::value_type const *pkRewardSet1 = rkQuestReward.GetRewardSet1(iGender, iClassNo);
		ContRewardVec::value_type const *pkRewardSet2 = rkQuestReward.GetRewardSet2(iGender, iClassNo);
		bool const bSelective1 = (pkRewardSet1)? pkRewardSet1->bSelective: false;
		bool const bSelective2 = (pkRewardSet2)? pkRewardSet2->bSelective: false;
		bool bSelect = true;
		bSelect &= (bSelective1)? (0!=SelectItem1()): true;
		bSelect &= (bSelective2)? (0!=SelectItem2()): true;
		if(!bSelect)//선택 했나?
		{
			SChatLog kChatLog(CT_EVENT_SYSTEM);
			g_kChatMgrClient.AddMessage(400100, kChatLog, true);
			return false;//선택을 안했으니 진행금지
		}
	}
	return true;//진행해라
}


void PgQuestMan::User_Quest_State_Changed()
{
	PgQUI_IngQuestList *pIngQuestList = (PgQUI_IngQuestList *)XUIMgr.Get(WSTR_FORM_MY_QUEST_LIST);
	PgQUI_QuestDialog *pQuestDialog = (PgQUI_QuestDialog *)XUIMgr.Get(WSTR_QUEST_INFO);
	PgQUI_QuestDialog *pIngQuestInfoDialog = (PgQUI_QuestDialog *)XUIMgr.Get(WSTR_MY_QUEST_INFO);
	//CXUI_Wnd *pkDailyQuestTopWnd = XUIMgr.Get(DailyQuestUI::kDailyQuestWndName);
	if( pIngQuestList )
	{
		pIngQuestList->UpdateControl();
	}
	if( pQuestDialog )
	{
		pQuestDialog->UpdateControl();
	}
	if( pIngQuestInfoDialog )
	{
		pIngQuestInfoDialog->UpdateControl();
	}

	m_kMiniListMng.Update();
	UpdateQuestMiniIngToolTip(0);
	//if( pkDailyQuestTopWnd )
	//{
	//	int iQuestID = 0;
	//	EQuestState eQuestState = QS_None;
	//	BM::GUID kNpcGuid;
	//	DailyQuestUI::GetDailyQuestCustomData(pkDailyQuestTopWnd, kNpcGuid, eQuestState, iQuestID); // 선택된것 로드

	//	DailyQuestUI::CallDailyQuestUI(lwGUID(kNpcGuid), DailyQuestUI::iLastDailyQuestUITitleNo);
	//}
}

void PgQuestMan::ShowQuestInfo(std::wstring const& rkContents)
{
	if( rkContents.size() )
	{
		m_ContQuestOut.push_back(SQuestOutInfo(g_pkApp->GetAccumTime(), rkContents));
		if (m_ContQuestOut.size() > iMaxWarnCount)
		{
			m_ContQuestOut.pop_front();
		}

		if( !IsQuestDialog() )
		{
			lwActivateUI("QuestOut");
		}
	}
}

bool PgQuestMan::ParseImportanceQuest(TiXmlElement const* pkElement, bool const IsReLoad)
{
	if( IsReLoad || m_kImportanceQuest.empty() )
	{
		while( pkElement )
		{
			assert(pkElement);
			char const* pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "IMPORTANCE_QUEST") != 0)
			{
				return false;
			}

			TiXmlElement const* pkGroupEle = pkElement->FirstChildElement();
			while( pkGroupEle )
			{
				pcTagName = pkGroupEle->Value();
				if(strcmp(pcTagName, "QUEST_GROUP") == 0)
				{
					int QuestGroupNo = 0;
					SImportanceQuestInfo kImportanceQuestInfo;

					TiXmlAttribute const* pkAttr = pkGroupEle->FirstAttribute();
					while(pkAttr)//
					{
						const char *pcAttrName	= pkAttr->Name();
						const char *pcAttrValue	= pkAttr->Value();

						if(strcmp(pcAttrName, "ID") == 0)
						{
							QuestGroupNo = atoi(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "TEXT_NO") == 0)
						{
							kImportanceQuestInfo.iGroupNameNo = atoi(pcAttrValue);
						}
						pkAttr = pkAttr->Next();
					}

					TiXmlElement const* pkQuestEle = pkGroupEle->FirstChildElement();
					while( pkQuestEle )
					{
						pcTagName = pkQuestEle->Value();
						if(strcmp(pcTagName, "QUEST") == 0)
						{
							int iQuestID = 0;
							TiXmlAttribute const* pkAttr = pkQuestEle->FirstAttribute();
							while(pkAttr)//
							{
								const char *pcAttrName	= pkAttr->Name();
								const char *pcAttrValue	= pkAttr->Value();

								if(strcmp(pcAttrName, "NO") == 0)
								{
									iQuestID = atoi(pcAttrValue);
								}
								pkAttr = pkAttr->Next();
							}
							kImportanceQuestInfo.kQuestList.push_back(iQuestID);
						}
						pkQuestEle = pkQuestEle->NextSiblingElement();
					}
					m_kImportanceQuest.insert(std::make_pair(QuestGroupNo, kImportanceQuestInfo));
				}
				pkGroupEle = pkGroupEle->NextSiblingElement();
			}
			pkElement = pkElement->NextSiblingElement();
		}
	}
	return true;
}

bool PgQuestMan::ParseCustomQuestMarker(TiXmlElement const* pkElement, bool const IsReLoad)
{
	if( IsReLoad || m_kCustomQuestMaker.empty() )
	{
		while( pkElement )
		{
			assert(pkElement);
			char const* pcTagName = pkElement->Value();

			if(strcmp(pcTagName, "CUSTOM_QUEST_MARKER") != 0)
			{
				return false;
			}

			TiXmlElement const* pkGroupEle = pkElement->FirstChildElement();
			while( pkGroupEle )
			{
				pcTagName = pkGroupEle->Value();
				if(strcmp(pcTagName, "CQM") == 0)
				{
					SCustomQuestMarker CustomMarker;

					TiXmlAttribute const* pkAttr = pkGroupEle->FirstAttribute();
					while(pkAttr)//
					{
						const char *pcAttrName	= pkAttr->Name();
						const char *pcAttrValue	= pkAttr->Value();

						if(strcmp(pcAttrName, "QUEST_ID") == 0)
						{
							CustomMarker.iQuestID = atoi(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "BEGIN_EFFECT_ID") == 0)
						{
							CustomMarker.strBegin = pcAttrValue;
						}
						else if(strcmp(pcAttrName, "END_EFFECT_ID") == 0)
						{
							CustomMarker.strEnd = pcAttrValue;
						}
						else if(strcmp(pcAttrName, "ING_EFFECT_ID") == 0)
						{
							CustomMarker.strIng = pcAttrValue;
						}
						else if(strcmp(pcAttrName, "NOT_YET_EFFECT_ID") == 0)
						{
							CustomMarker.strNotYet = pcAttrValue;
						}
						else if(strcmp(pcAttrName, "BEGIN_ICON_IDX") == 0)
						{
							CustomMarker.iBeginIconIDX = atoi(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "END_ICON_IDX") == 0)
						{
							CustomMarker.iEndIconIDX = atoi(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "ING_ICON_IDX") == 0)
						{
							CustomMarker.iIngIconIDX = atoi(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "NOT_YET_ICON_IDX") == 0)
						{
							CustomMarker.iNotYetIconIDX = atoi(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "BEGIN_TXT_ID") == 0)
						{
							CustomMarker.iBeginTTID = atoi(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "END_TXT_ID") == 0)
						{
							CustomMarker.iEndTTID = atoi(pcAttrValue);
						}
						else if(strcmp(pcAttrName, "BOOK_ICON_IDX") == 0)
						{
							CustomMarker.iBookIconIDX = atoi(pcAttrValue);
						}
						pkAttr = pkAttr->Next();
					}

					std::pair<CONT_QUESTCUSTOMMARKER::iterator, bool> Rst = m_kCustomQuestMaker.insert(std::make_pair(CustomMarker.iQuestID, CustomMarker));
					if( !Rst.second ){}//로그를 남길까.
				}
				pkGroupEle = pkGroupEle->NextSiblingElement();
			}
			pkElement = pkElement->NextSiblingElement();
		}
	}
	return true;
}

bool PgQuestMan::CheckCustomQuestMarker(int const iQuestID, SCustomQuestMarker& MarkerInfo)
{
	CONT_QUESTCUSTOMMARKER::iterator marker_itor = m_kCustomQuestMaker.find(iQuestID);
	if( marker_itor != m_kCustomQuestMaker.end() )
	{
		CONT_QUESTCUSTOMMARKER::mapped_type const& kMarkerInfo = marker_itor->second;
		MarkerInfo = kMarkerInfo;
		return true;
	}
	return false;
}

void PgQuestMan::ShowQuestInfo(EQuestState const eState, std::wstring const& rkDesc, int const iCur, int const iEnd)
{
	std::wstring kContents;
	switch( eState )
	{
	case QS_Failed:
		{
			kContents = _T("{C=0xFFFF7D00/}");
			kContents += rkDesc;
			kContents += _T(" [") + TTW(20002) + _T("]");
		}break;
	case QS_End:
		{
			kContents = _T("{C=0xFF00FF00/}");
			kContents += rkDesc;
			kContents += _T(" [") + TTW(20001) + _T("]");
		}break;
	case QS_Ing:
	default:
		{
			kContents = _T("{C=0xFFffff00/}");
			kContents += rkDesc; // 아무개 잡기
			kContents += _T(" [") + std::wstring(BM::vstring(iCur)) + _T("/") + std::wstring(BM::vstring(iEnd)) + _T("]");
		}break;
	}
	if( !kContents.empty() )
	{
		ShowQuestInfo( kContents );
	}
}

void PgQuestMan::ShowQuestInfo(const SUserQuestState& rkOldState, const SUserQuestState& rkState)
{
	if( !rkState.iQuestID )
	{
		return;
	}
	if( QS_End == rkOldState.byQuestState
	&&	QS_End == rkState.byQuestState )
	{
		return;
	}

	std::wstring kContents;
	PgQuestInfo const *pkXml = GetQuest(rkState.iQuestID);
	if( pkXml
	&&	((QS_Ing == rkOldState.byQuestState && QS_Ing == rkState.byQuestState) || (QS_End == rkState.byQuestState) || (QS_Failed == rkState.byQuestState)) )
	{
		int iCur = 0;
		//DWORD dwColor = COLOR_WHITE;
		for( int i=0; QUEST_PARAMNUM>i; ++i)
		{
			if(QUEST_PARAMNUM <= iCur) break;

			//std::wstring kObjectDesc;//아무개 잡기 [1/10], 아무개 잡기 [완료]

			int const iTTW = pkXml->GetObjectText(iCur);
			if(	iTTW
			&&	rkOldState.byParam[iCur] != rkState.byParam[iCur])//이전과 현재가 틀릴 때
			{
				ShowQuestInfo(QS_Ing, TTW(iTTW), rkState.byParam[iCur], pkXml->GetParamEndCondition(iCur));
			}

			++iCur;
		}

		if( QS_End == rkState.byQuestState )//이 한퀘스트를 완료 할 수 있게 되었는가?
		{
			ShowQuestInfo(QS_End, TTW(pkXml->m_iTitleTextNo));
		}
		else if( QS_Failed == rkState.byQuestState )//이 실패 했는가?
		{
			ShowQuestInfo(QS_Failed, TTW(pkXml->m_iTitleTextNo));
		}
	}
}

void PgQuestMan::UpdateQuestOut(CXUI_Wnd* pUI)
{
	if (!pUI)
	{
		return;
	}

	ContQuestOut::iterator out_it = m_ContQuestOut.begin();
	size_t iAccumHeight = 0;
	for( int i = 1; i <= iMaxWarnCount; ++i )
	{
		BM::vstring kStr(_T("FRM_QuestOut"));
		kStr += i;
		CXUI_Wnd* pkOut = pUI->GetControl((std::wstring)kStr);

		if( pkOut )
		{
			if( m_ContQuestOut.end() != out_it )
			{
				SQuestOutInfo &kOutInfo = (*out_it);
				if( kOutInfo.IsTimeOver() )
				{
					pkOut->Text( std::wstring() );

					out_it = m_ContQuestOut.erase(out_it);
				}
				else
				{
					pkOut->Text( (*out_it).wstrInfo );
					POINT3I const &rkWndPos = pkOut->Location();
					pkOut->Location(rkWndPos.x, iAccumHeight, rkWndPos.z);
					++out_it;
				}
			}
			else
			{
				pkOut->Text( std::wstring() );
			}

			XUI::CXUI_Style_String kStyleString = pkOut->StyleText();
			iAccumHeight += Pg2DString::CalculateOnlySize(kStyleString).y;
		}
	}

	if (m_ContQuestOut.empty())
	{
		pUI->Close();
		return;
	}
}

void PgQuestMan::CallQuestDialog(BM::GUID const &rkGuid, int const iQuestID, int const iDialogID)//, int const iObjectType, BM::GUID const &rkObjectGuid)
{
	if(!g_pkWorld)
	{
		return;
	}

	const PgQuestInfo* pkQuestInfo = GetQuest(iQuestID);
	if( !pkQuestInfo )
	{return;}

	const SQuestDialog* pkQuestDialog = NULL;
	bool bFindDialog = pkQuestInfo->GetDialog(iDialogID, pkQuestDialog);
	if( !bFindDialog )
	{
		PgQuestManUtil::CloseAllQuestTalkUI();
		return;
	}

	PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
	if( pkMyActor )
	{
		g_pkWorld->AddDrawActorFilter( pkMyActor->GetGuid() );//나와 NPC만 보여준다
		if( BM::GUID::IsNotNull(g_kHelpSystem.HelperGuid()) )
		{
			g_pkWorld->AddDrawActorFilter( g_kHelpSystem.HelperGuid() );//헬퍼도 보여준다.
		}
		PgActor* pkPetActor = pkMyActor->GetMountTargetPet();
		if( pkPetActor && BM::GUID::IsNotNull(pkPetActor->GetGuid()) )
		{
			g_pkWorld->AddDrawActorFilter( pkPetActor->GetGuid() ); //라이딩펫에 탑승 중이면 펫도 함께 보여줘야 한다.
		}
	}
	g_pkWorld->AddDrawActorFilter( rkGuid );

	XUIMgr.Close(_T("SFRM_NEW_QUEST"));//퀘스트 알림목록 창 닫기
	XUIMgr.Close(_T("FORM_MY_QUEST_LIST"));//내 퀘스트 목록 창 닫기
	
	bool const bDifferentDialog = (m_kRecentQuestID != iQuestID) || (m_kRecentDialogID != iDialogID);

	m_kRecentQuestID = iQuestID;//최근 퀘스트 번호를 기억
	m_kRecentDialogID = iDialogID;

	if( bDifferentDialog )
	{
		SFullScreenDialogInfo kInfo(pkQuestDialog->eType);

		if( bDifferentDialog
		&&	!pkQuestDialog->kDialogText.empty() )//다이얼로그가 달라지면 Step을 재 구성한다.
		{
			const ContQuestDialogText::value_type& rkDialogText = pkQuestDialog->kDialogText.front();//임시 항상 첫 Text Body
			kInfo = SFullScreenDialogInfo(pkQuestDialog->eType, rkDialogText);

			if( rkDialogText.iStringNameID )
			{
				std::wstring const *pkDefString = NULL;
				if( GetDefString(rkDialogText.iStringNameID, pkDefString) )
				{
					kInfo.kObjectName = *pkDefString;
				}
			}
			else if( rkDialogText.iTextNameID )
			{
				kInfo.kObjectName = TTW(rkDialogText.iTextNameID);
			}
			if( 0 != rkDialogText.iMovieNo)
			{
				kInfo.iMovieNo = rkDialogText.iMovieNo;
			}
		}

		PgPilot* pkPilot = g_kPilotMan.FindPilot( rkGuid );//Object(NPC, ETC) Name
		if( pkPilot )
		{
			if( kInfo.kObjectName.empty() )
			{
				kInfo.kObjectName = pkPilot->GetName();
			}
		}
		else
		{
			if( kInfo.kObjectName.empty() 
			&&	BM::GUID::IsNotNull(rkGuid) )
			{
				PgQuestManUtil::kNpcNameHelper.Get(rkGuid, kInfo.kObjectName);
			}
		}

		kInfo.kTitleText = TTW(pkQuestInfo->m_iTitleTextNo);
		kInfo.kGuid = rkGuid;

		CallNewDialog(kInfo, pkQuestInfo, pkQuestDialog);
	}
	else
	{
		CallNextDialog();
	}
}



void PgQuestMan::CallNewDialog(const SFullScreenDialogInfo& rkInfo, const PgQuestInfo* pkQuestInfo, const SQuestDialog* pkDialogInfo)
{
	m_kRecentDialogType = rkInfo.eType;
	//
	switch(m_kRecentDialogType)
	{
	case QDT_Wanted:
	case QDT_Wanted_Complete:
	case QDT_VillageBoard:
		{
			UpdateWanted(rkInfo, pkQuestInfo, pkDialogInfo);
		}break;
	case QDT_NpcTalk:
	case QDT_Prolog:
	case QDT_Normal:
	case QDT_Complete:
	case QDT_Guild_Menu:
	case QDT_GuildEmporia_Administrator:
	case QDT_GuildEmporia_Admin_Building:
	case QDT_GuildEmporia_Admin_Removal:
	case QDT_Movie:
	default:
		{
			CallFullScreenTalk(rkInfo, pkQuestInfo, pkDialogInfo);
		}break;
	}
}

void PgQuestMan::CallNextDialog()
{
	//
	switch(m_kRecentDialogType)
	{
	case QDT_Wanted:
	case QDT_VillageBoard:
	case QDT_Movie:
		{
			//
		}break;
	case QDT_Normal:
	case QDT_NpcTalk:
	case QDT_Guild_Menu:
	default:
		{
			if(true == g_kMovieMgr.IsPlay())
			{
				return;
			}
			NextFullScreenTalk();
		}break;
	}
}
void PgQuestMan::ClearRecentQuestInfo()
{
	BM::CAutoMutex kLock(m_kMutex);
	if( 0 != IsComplete() )
	{
		CallCompleteStamp(IsComplete());
		IsComplete(0); // 초기화
	}
	else
	{
		if( 0 != BeginEventID() )
		{
			int const iBeginEventID = BeginEventID();
			BeginEventID(0);
			g_kEventScriptSystem.ActivateEvent(iBeginEventID);
		}
	}

	m_kRecentQuestID = 0;
	m_kRecentDialogID = 0;
	ClearDialogStep();

	m_kRecentDialogType = QDT_None;

	m_kMiniListMng.Update(); // 업데이트
	RandomQuestUI::CallRandomQuestBtn();
}

void PgQuestMan::CallCompleteStamp(VEC_WSTRING const& rkVec)
{
	float const fBaseInterval = 0.5f;
	float const fInterval = 0.2f;
	float const fExtraInterval = 2.7f;
	
	if( lwCheckReservedClassPromotionAction() )
	{// 전직 연출이 예약되어있으면 ui 띄우지 않음
		return;
	}

	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Call(_T("SFRM_QUEST_COMPLETE"));
	if( !pkTopWnd )
	{
		return;
	}

	int const iDefaultHeight = 30;
	XUI::CXUI_Wnd* pkTopImgWnd = pkTopWnd->GetControl(_T("IMG_BG1"));
	if( pkTopImgWnd )
	{
		POINT2 kTopSize = pkTopImgWnd->Size();
		XUI::CXUI_Wnd* pkMiddleImgWnd = pkTopWnd->GetControl(_T("IMG_BG2"));
		if( pkMiddleImgWnd )
		{
			POINT2 kMiddleSize = pkMiddleImgWnd->Size();
			kMiddleSize.y = iDefaultHeight;
			pkMiddleImgWnd->Size(kMiddleSize);
			pkMiddleImgWnd->ImgSize(kMiddleSize);
		}
		XUI::CXUI_Wnd* pkBottomImgWnd = pkTopWnd->GetControl(_T("IMG_BG3"));
		if( pkBottomImgWnd )
		{
			POINT3I kBottomLoc = pkBottomImgWnd->Location();
			kBottomLoc.y = kTopSize.y + iDefaultHeight;
			pkBottomImgWnd->Location(kBottomLoc);
		}
	}

	size_t iCount = 0;
	VEC_WSTRING::const_iterator iter = rkVec.begin();
	while(rkVec.end() != iter)
	{
		VEC_WSTRING::value_type const &rkString = (*iter);
		char szEventID[MAX_PATH] = {0, };
		char szEventScript[MAX_PATH] = {0, };

		sprintf_s(szEventID, MAX_PATH, "EVENT_SFRM_QUEST_COMPLETE_%d", iCount);
		sprintf_s(szEventScript, MAX_PATH, "Add_SFRM_QUEST_COMPLETE_Item('%s')", MB(rkString.c_str()));

		STimerEvent kTimerEvent;
		kTimerEvent.Set(fBaseInterval + (fInterval*iCount), szEventScript);
		g_kEventTimer.AddLocal(szEventID, kTimerEvent);

		++iCount;
		++iter;
	}
	float const fCloseEventTime = fBaseInterval + (fInterval*iCount) + fExtraInterval;
	{
		std::string const kCloseScript = "End_SFRM_QUEST_COMPLETE() End_After_SFRM_QUEST_COMPLETE()";
		STimerEvent kTimerEvent;
		kTimerEvent.Set(fCloseEventTime, kCloseScript.c_str());
		g_kEventTimer.AddLocal("EVENT_END_SFRM_QUEST_COMPLETE", kTimerEvent);
	}
	//float const fCloseAfterEventTime = fCloseEventTime + fAfterInterval;
	//{
	//	std::string const kCloseAfterScript = "";
	//	STimerEvent kTimerEvent;
	//	kTimerEvent.Set(fCloseAfterEventTime, kCloseAfterScript.c_str());
	//	g_kEventTimer.AddLocal("EVENT_END_AFTER_SFRM_QUEST_COMPLETE", kTimerEvent);
	//}
}

void PgQuestMan::CallCompleteStamp(int const iQuestID)
{
	PgQuestInfo const *pkQuestInfo = GetQuest(iQuestID);
	if( !pkQuestInfo )
	{
		return;
	}

	EQuestType eType = pkQuestInfo->Type();
	switch( eType )
	{
	case QT_Wanted:
	case QT_Loop:
	case QT_Event_Loop:
	case QT_Scroll:
		{
		}break;
	default:
		{
			if( m_kContCompleteQuestID.end() != m_kContCompleteQuestID.find(iQuestID) )
			{
				return;
			}
			m_kContCompleteQuestID.insert( iQuestID );
		}break;
	}

	VEC_WSTRING kVec;
	bool const bRet = MakeRewardText(pkQuestInfo, kVec);
	if( bRet )
	{
		CallCompleteStamp(kVec);

		if( -1 != pkQuestInfo->CompleteEventScriptID() )
		{
			g_kEventScriptSystem.ActivateEvent(pkQuestInfo->CompleteEventScriptID());
		}
	}
}

//>>
void MakeItemDesciption(const ContQuestItem::value_type& rkItem, std::vector< std::wstring >& rkOut)
{
	SEnchantInfo kEnchantInfo;
	std::wstring kItemName;
	bool const bMaked = MakeItemName(rkItem.iItemNo, kEnchantInfo, kItemName);
	if( bMaked )
	{
		std::wstring kTemp;
		if( 1 == rkItem.iCount )
		{
			bool const bRet = FormatTTW(kTemp, 450013, kItemName.c_str());
			if( bRet )
			{
				rkOut.push_back(kTemp);
			}
		}
		else if( 1 < rkItem.iCount )
		{
			bool const bRet = FormatTTW(kTemp, 450014, kItemName.c_str(), rkItem.iCount);
			if( bRet )
			{
				rkOut.push_back(kTemp);
			}
		}	
	}
}
void MakeRewardTextSet(ContRewardVec::value_type const *pkRewardSet, size_t const iSelectItem, std::vector< std::wstring >& rkOut)
{
	if( pkRewardSet )
	{
		const ContQuestItem& rkItemList = pkRewardSet->kItemList;
		int iSelectNo = (0 < iSelectItem && rkItemList.size() >= iSelectItem)? iSelectItem-1: 0;//
		if( rkItemList.size() )
		{
			if( iSelectItem
			&&	pkRewardSet->bSelective )
			{
				const ContQuestItem::value_type& rkItem = rkItemList.at(iSelectNo);
				MakeItemDesciption(rkItem, rkOut);
			}
			else
			{
				ContQuestItem::const_iterator iter = rkItemList.begin();
				while(rkItemList.end() != iter)
				{
					const ContQuestItem::value_type& rkItem = *iter;
					MakeItemDesciption(rkItem, rkOut);
					++iter;
				}
			}
		}
	}
}
//<<

bool PgQuestMan::MakeRewardText(PgQuestInfo const *pkQuestInfo, std::vector< std::wstring >& rkOut)
{
	PgPlayer *pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}

	PgMyQuest const* pkMyQuest = pkPlayer->GetMyQuest();
	if( !pkMyQuest )
	{
		return false;
	}

	//퀘스트 보상을 문자열로 만들어 준다.(경험치 얼마 획득, 돈 얼마 획득, 아이템 뭐 획득)
	if( pkQuestInfo )
	{
		//450011	퀘스트 UI	경험치 %d Exp 획득
		//450012	퀘스트 UI	돈 (%s) 획득
		//450013	퀘스트 UI	아이템 %s 획득
		//450014	퀘스트 UI	아이템 %s를 %d개 획득
		//460099	퀘스트 UI	레벨업 %dLV 획득
		int const iClassNo = pkPlayer->GetAbil(AT_CLASS);
		int const iGender = pkPlayer->GetAbil(AT_GENDER);
		ContRewardVec::value_type const *pkRewardSet1 = pkQuestInfo->m_kReward.GetRewardSet1(iGender, iClassNo);
		ContRewardVec::value_type const *pkRewardSet2 = pkQuestInfo->m_kReward.GetRewardSet2(iGender, iClassNo);

		CONT_DEF_QUEST_LEVEL_REWARD const *pDef_LevelReward = NULL;
		g_kTblDataMgr.GetContDef(pDef_LevelReward);

		int const iAddCompleteCount = -1;  // 후 처리라 횟수를 1 감소 시켜야 한다
		int iRewardExp = PgQuestInfoUtil::GetQuestRewardExp(pkQuestInfo, pkMyQuest, iAddCompleteCount);
		if( pDef_LevelReward )
		{
			CONT_DEF_QUEST_LEVEL_REWARD::const_iterator kitor_Reward = pDef_LevelReward->find( pkPlayer->GetAbil(AT_LEVEL) );
			if( pDef_LevelReward->end() != kitor_Reward )
			{
				float const fRate = pkQuestInfo->m_kReward.iLevelPer_Exp_1000 / 1000;
				iRewardExp += (*kitor_Reward).second.iExp * fRate;
			}
		}
		if( iRewardExp )
		{
			std::wstring kTemp;
			int iCalcRewardExp = iRewardExp;
			pkPlayer->CalcExpRate(iCalcRewardExp);
			bool const bRet = FormatTTW(kTemp, 450011, iCalcRewardExp);
			if( bRet )
			{
				rkOut.push_back(kTemp);
			}
		}

		int const iRewardGuildExp = PgQuestInfoUtil::GetQuestRewardGuildExp(pkQuestInfo, pkMyQuest, iAddCompleteCount);
		if( iRewardGuildExp )
		{
			std::wstring kTemp;
			int iCalcRewardGuildExp = iRewardGuildExp;
			pkPlayer->CalcExpRate(iCalcRewardGuildExp);
			bool const bRet = FormatTTW(kTemp, 450270, iCalcRewardGuildExp);
			if( bRet )
			{
				rkOut.push_back(kTemp);
			}
		}

		if( pkQuestInfo->m_kReward.iMoney )
		{
			int iCalcMoney = pkQuestInfo->m_kReward.iMoney;
			if( pDef_LevelReward )
			{
				CONT_DEF_QUEST_LEVEL_REWARD::const_iterator kitor_Reward = pDef_LevelReward->find( pkPlayer->GetAbil(AT_LEVEL) );
				if( pDef_LevelReward->end() != kitor_Reward )
				{
					float const fRate = pkQuestInfo->m_kReward.iLevelPer_Money_1000 / 1000;
					iCalcMoney += (*kitor_Reward).second.iMoney * fRate;
				}
			}
			pkPlayer->CalcMoneyRate(iCalcMoney);

			std::wstring kTemp;
			std::wstring kFomatedMoney;
			if( FormatMoney(iCalcMoney, kFomatedMoney) )
			{
				bool const bRet = FormatTTW(kTemp, 450012, kFomatedMoney.c_str());
				if( bRet )
				{
					rkOut.push_back(kTemp);
				}
			}
		}
		if( pkQuestInfo->m_kReward.iSkillNo )
		{
			GET_DEF(CSkillDefMgr, kSkillDefMgr);
			CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(pkQuestInfo->m_kReward.iSkillNo);
			if( pkSkillDef )
			{
				wchar_t const* pkSkillName = NULL;
				if( GetDefString(pkSkillDef->NameNo(), pkSkillName) )
				{
					std::wstring kTemp;
					if( FormatTTW(kTemp, 450264, pkSkillName) )
					{
						rkOut.push_back(kTemp);
					}
				}
			}
		}
		if( pkQuestInfo->m_kReward.iChangeLevel )
		{
			std::wstring kTemp;
			BM::vstring kText(TTW(460099));
			kText.Replace(L"#LEVEL#", pkQuestInfo->m_kReward.iChangeLevel);
			rkOut.push_back(static_cast<std::wstring>(kText));
		}
		if( QT_Wanted == pkQuestInfo->Type() )
		{
			CONT_DEF_QUEST_WANTED const *pkContWantedQuest = NULL;
			g_kTblDataMgr.GetContDef(pkContWantedQuest);
			if( pkContWantedQuest )
			{
				CONT_DEF_QUEST_WANTED::const_iterator find_iter = pkContWantedQuest->find( pkQuestInfo->m_kBasic.iQuestID );
				if( pkContWantedQuest->end() != find_iter )
				{
					CONT_DEF_QUEST_WANTED::mapped_type const& rkQuestWanted = (*find_iter).second;
					int iItemNo = rkQuestWanted.iItemNo;
					int iItemCount = rkQuestWanted.iItemCount;
					int const iBaseClass = pkPlayer->GetAbil(AT_BASE_CLASS);
					if( UCLASS_SHAMAN == iBaseClass ||
						UCLASS_DOUBLE_FIGHTER == iBaseClass )
					{
						iItemNo = rkQuestWanted.iDra_ItemNo;
						iItemCount = rkQuestWanted.iDra_ItemCount;
					}
					const int iRewardCount = rkQuestWanted.iClearCount;
					const int iClearcount = pkPlayer->GetWantedQuestClearCount(pkQuestInfo->m_kBasic.iQuestID);

					if( iClearcount >= iRewardCount )
					{
						GET_DEF(CItemDefMgr, kItemDefMgr);
						CItemDef const* pkItemDef = kItemDefMgr.GetDef(iItemNo);
						if( pkItemDef )
						{
							wchar_t const* pkItemName = NULL;
							if( GetDefString( pkItemDef->NameNo(), pkItemName ) )
							{
								std::wstring kTemp;
								if( FormatTTW(kTemp, 450014, pkItemName, iItemCount ) )
								{
									rkOut.push_back(kTemp);
								}
							}
						}
					}
				}
			}
		}

		MakeRewardTextSet(pkRewardSet1, SelectItem1(), rkOut);
		MakeRewardTextSet(pkRewardSet2, SelectItem2(), rkOut);
	}
	return (rkOut.size() != 0);
}
bool PgQuestMan::IsDependIngQuestItem(int const iItemNo, CItemDef const* pkItemDef)
{
	//if( pkItemDef )
	//{
	//	if( pkItemDef->IsType(ITEM_TYPE_QUEST) )
	//	{
	//		return true;//퀘스트 아이템이다
	//	}
	//}

	PgPlayer* pkPC = g_kPilotMan.GetPlayerUnit();
	if( pkPC )
	{
		PgMyQuest  const *pkMyQuest = pkPC->GetMyQuest();
		if( !pkMyQuest )
		{
			return false;
		}

		ContUserQuestState kQuestVec;
		bool const bGetList = pkMyQuest->GetQuestList(kQuestVec);
		if( !bGetList )
		{
			return false;
		}

		ContUserQuestState::iterator iter = kQuestVec.begin();
		while(kQuestVec.end() != iter)
		{
			const ContUserQuestState::value_type& rkElement = (*iter);
			const PgQuestInfo* pkQuestInfo = GetQuest(rkElement.iQuestID);
			if( pkQuestInfo )
			{
				CONT_DEL_QUEST_ITEM::const_iterator depend_iter = pkQuestInfo->m_kCompleteDeleteItem.begin();
				while(pkQuestInfo->m_kCompleteDeleteItem.end() != depend_iter)
				{
					if( (*depend_iter).iItemNo == iItemNo )
					{
						return true;
					}
					++depend_iter;
				}
			}

			++iter;
		}
	}
	return false;//퀘스트 아이템 아니다
}
void PgQuestMan::AddRecentBeginQuest(int const iQuestID)
{
	PgQuestInfo const* pkQuestInfo = GetQuest(iQuestID);
	if( pkQuestInfo )
	{
		switch( pkQuestInfo->Type() )
		{
		case QT_Soul:
			{
			}break;
		default:
			{
				ContQuestID::const_iterator find_iter = std::find(m_kRecentBeginQuest.begin(), m_kRecentBeginQuest.end(), iQuestID);
				if( m_kRecentBeginQuest.end() == find_iter )
				{
					std::back_inserter(m_kRecentBeginQuest) = iQuestID;
				}
				m_kRecentBeginQuest.begin();
			}break;
		}
	}
}
void PgQuestMan::DelRecentBeginQuest(int const iQuestID)
{
	ContQuestID::iterator find_iter = std::find(m_kRecentBeginQuest.begin(), m_kRecentBeginQuest.end(), iQuestID);
	if( m_kRecentBeginQuest.end() != find_iter )
	{
		m_kRecentBeginQuest.erase(find_iter);
	}
}
int PgQuestMan::GetRecentBeginQuest()
{
	if( m_kRecentBeginQuest.empty() )
	{
		return 0;
	}
	return m_kRecentBeginQuest.back();
}
void PgQuestMan::IsComplete(int const iQuestID)
{
	PgQuestInfo const* pkQuestInfo = GetQuest(iQuestID);
	if( pkQuestInfo )
	{
		m_kIsComplete = iQuestID;
		
		int const iNextTalkQuestID = pkQuestInfo->CompleteNextQuestID();
		if( 0 != pkQuestInfo->CompleteNextQuestID() )
		{
			ResumeNextQuestTalk( SResumeNextQuestTalk(NowInfo().kGuid, iNextTalkQuestID) );
		}
	}
	else
	{
		m_kIsComplete = 0;
	}
}
void PgQuestMan::SetResumeNextQuestTalk(SResumeNextQuestTalk const& rkResumNextQuestTalk)
{
	PgPlayer const* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}

	PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest(rkResumNextQuestTalk.iQuestID);
	if( !pkQuestInfo )
	{
		return;
	}

	if( pkQuestInfo->m_kLimit.iMinLevel > pkPlayer->GetAbil(AT_LEVEL) )
	{
		return;
	}

	g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_NextQuestTalk, true)); // 락걸고
	g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_NextQuestTalk, false));
	m_kResumeNextQuestTalk = rkResumNextQuestTalk;
}
void PgQuestMan::ResetResumeNextQuestTalk()
{
	if( m_kResumeNextQuestTalk.Empty() )
	{
		return;
	}

	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_NextQuestTalk, true)); // 락풀고
	g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_NextQuestTalk, false));
	m_kResumeNextQuestTalk = SResumeNextQuestTalk();
}
void PgQuestMan::ResumeNextQuestTalk(SResumeNextQuestTalk const& rkResumNextQuestTalk)
{
	if( rkResumNextQuestTalk.Empty() )
	{
		ResetResumeNextQuestTalk();
	}
	else
	{
		SetResumeNextQuestTalk(rkResumNextQuestTalk);
	}
}
bool PgQuestMan::RemoveMiniQuestList(int const iQuestID)
{
	if( m_kMiniListMng.Del(iQuestID) )
	{
		m_kMiniListMng.Save();
		return true;
	}
	return false;
}
bool PgQuestMan::AddMiniQuestList(int const iQuestID)
{
	if( m_kMiniListMng.Add(iQuestID) )
	{
		m_kMiniListMng.Save();
		return true;
	}
	return false;
}
void PgQuestMan::DelFromComleteSet(int const iQuestID)
{
	m_kContCompleteQuestID.erase( iQuestID );
}


//
XUI::CXUI_Wnd* CallQuestWnd(std::wstring const &rkFormName)
{
	XUI::CXUI_Wnd* pQuestWnd = XUIMgr.Get(rkFormName);
	if( !pQuestWnd )
	{
		pQuestWnd = XUIMgr.Call(rkFormName);
	}

	if( pQuestWnd )
	{
		XUIMgr.Close(_T("SFRM_QUEST_COMPLETE"));
	}

	return pQuestWnd;
}