#include "stdafx.h"
#include "Variant/PgJobSkillLocationItem.h"
#include "Variant/PgJobSkillWorkBench.h"
#include "Variant/PgJobSkillWorkBenchMgr.h"
#include "Variant/item.h"
#include "Variant/Global.h"
#include "Lohengrin/PacketStruct.h"
#include "PgNetwork.h"
#include "PgHome.h"
#include "PgJobSkillWorkMachine.h"
#include "PgPilotMan.h"
#include "PgCoupleMgr.h"
#include "PgFriendMgr.h"
#include "PgGuild.h"
#include "PgChatMgrClient.h"
#include "lwWString.h"
#include "lwUI.h"
#include "lwUIQuest.h"
#include "PgUISound.h"
#include "lwGUID.h"
#include "PgMobileSuit.h"

std::wstring const WSTR_FISH(L"job_aquarium");
std::wstring const WSTR_GARDEN(L"job-garden");
std::wstring const WSTR_JEWEL(L"job-jewellerymaker");
std::wstring const WSTR_WOOD(L"job-lathe");
std::wstring const WSTR_SMELTING(L"job-smelting");

int const MAX_PROGRESS = 1000;
float g_Last_REQ_INSERT_TO_WORKBENCH_Time = 0.0f;

XUI::CXUI_Wnd* GetMainUI();
XUI::CXUI_Wnd* GetDurabilityUI();
XUI::CXUI_Wnd* GetMaterialUpgradeInfoUI();
XUI::CXUI_List* GetLogList();
XUI::CXUI_Wnd* GetOwnerNameUI();
XUI::CXUI_Wnd* GetMachineNameUI();
XUI::CXUI_AniBar* GetProgressUI();
XUI::CXUI_Wnd* GetCompleteMsgUI(int const iSlot);
XUI::CXUI_Wnd* GetSlotUI(int const iSlot);
XUI::CXUI_List* GetLogListUI();
XUI::CXUI_Wnd* GetMiniUI();

PgJobSkillWorkMachine::PgJobSkillWorkMachine()
{}

PgJobSkillWorkMachine::~PgJobSkillWorkMachine()
{}

void PgJobSkillWorkMachine::ProcessMsg(BM::Stream::DEF_STREAM_TYPE const wPacketType, BM::Stream& rkPacket)
{
	switch( wPacketType )
	{
	case PT_N_C_ANS_HOME_WORKBENCH_INFO:
		{// 마이홈 작업대 정보 응답
			// 정보를 업데이트 하고
			//m_kContWorkBenchUpgradeTime.clear();
			Clear();
			m_kWorkBenchMgr.ReadFromPacket(rkPacket);
			PU::TLoadTable_AM(rkPacket, m_kContWorkBenchEvent);
			// 보여지는것들을 업데이트 한다
			UpdateAllFurnitureMeshState();	// 가구 메쉬
			UpdateUI();						// UI
		}break;
	case PT_N_C_ANS_INSERT_TO_WORKBENCH:
	case PT_N_C_ANS_GET_ITEM_FROM_WORKBENCH:
	case PT_N_C_NFY_HOME_WORKBENCH_INFO:
		{// 마이홈 작업대 정보 알림
			BM::GUID kCharGuid;
			EWorkBenchResult eResult = WBR_SUCCESS;
			if(PT_N_C_NFY_HOME_WORKBENCH_INFO != wPacketType)
			{
				rkPacket.Pop(kCharGuid);
				rkPacket.Pop(eResult);
			}

			if(WBR_SUCCESS == eResult)
			{// 성공하면, 작업대 GUID를 얻어와 정보를 갱신하거나 추가하고
				BM::GUID kWorkBenchGUID;		
				rkPacket.Pop(kWorkBenchGUID);

				// 이전 정보
				PgJobSkillWorkBench kWorkBench_PrevInfo;
				bool const bGetPrevInfo = m_kWorkBenchMgr.GetWorkBench(kWorkBenchGUID, kWorkBench_PrevInfo);
				if(!bGetPrevInfo) { return; }
				int const iPrevCurUpgradeSlot = kWorkBench_PrevInfo.CurUpgradeSlot();

				// 지금 정보
				m_kWorkBenchMgr.ReadFromPacket(kWorkBenchGUID, rkPacket);
				PgJobSkillWorkBench kWorkBench_NewInfo;
				bool const bGetNewInfo = m_kWorkBenchMgr.GetWorkBench(kWorkBenchGUID, kWorkBench_NewInfo);
				if(!bGetNewInfo) { return; }
				int const iNewCurUpgradeSlot = kWorkBench_NewInfo.CurUpgradeSlot();
				
				__int64 const i64CurTime = g_kEventView.GetLocalSecTime();
				size_t const stMaxSlot = JobSkillWorkBenchUtil::GetWorkBenchSlotCount(kWorkBench_NewInfo.ItemNo());
				for(size_t stSlot = JobSkillWorkBenchUtil::iStartSlotNo; stSlot < stMaxSlot+JobSkillWorkBenchUtil::iStartSlotNo; ++stSlot )
				{
					SJobSkillWorkBenchSlotItemStatus kPrevUpgradeInfo;
					SJobSkillWorkBenchSlotItemStatus kNewUpgradeInfo;
					bool const bCanGetPrev = kWorkBench_PrevInfo.GetUpgradeSlotItemStatus(stSlot, kPrevUpgradeInfo);
					bool const bCanGetNew = kWorkBench_NewInfo.GetUpgradeSlotItemStatus(stSlot, kNewUpgradeInfo);
					int const iPrevRemainCnt = JobSkillWorkBenchUtil::GetRemainUpgradeCount(kPrevUpgradeInfo.iItemNo);
					int const iNewRemainCnt = JobSkillWorkBenchUtil::GetRemainUpgradeCount(kNewUpgradeInfo.iItemNo);

					bool const bIsUpgraded = (iPrevRemainCnt > iNewRemainCnt) && (iPrevCurUpgradeSlot == iNewCurUpgradeSlot);					//이번에 업그레이드 됨.
					//bool const bIsRepaired	= ( kWorkBench_PrevInfo.HasTrouble() && !kWorkBench_NewInfo.HasTrouble() );							// 막 수리된 상황
					//bool const bIsTrouble	= ( !kWorkBench_PrevInfo.HasTrouble() && kWorkBench_NewInfo.HasTrouble() );							// 고장난 상황
					bool const bIsBlessed	= ( (0 == kWorkBench_PrevInfo.BlessDurationSec()) && (0 < kWorkBench_NewInfo.BlessDurationSec()) );	// 도움받은 상황
					//bool const bIsBlessedEnd= ( (0 < kWorkBench_PrevInfo.BlessDurationSec()) && (0 == kWorkBench_NewInfo.BlessDurationSec()) );	// 도움 끝난상황
					bool const bBeginUpgradeNextSlot = (iPrevCurUpgradeSlot != iNewCurUpgradeSlot) && (iNewCurUpgradeSlot == static_cast<int>(stSlot) );	// 한슬롯이 끝나서 다음슬롯으로 넘어간 상태

					if( (!bCanGetPrev && bCanGetNew)	// 새로 삽입 되었거나
						|| (bCanGetPrev && bCanGetNew && bIsUpgraded)	// 아이템이 업그레이드 되었다면
						|| bIsBlessed					// 축복을 받았거나
						//|| bIsBlessedEnd				// 축복이 끝났다면
						//|| bIsTrouble					// 고장 났다면
						|| bBeginUpgradeNextSlot		// 한슬롯이 끝나서 이제 이슬롯이 작업된다면
						)
					{// 시간 기억
						InitInfoRecvTime(kWorkBenchGUID, kNewUpgradeInfo, stSlot, i64CurTime, true);
					}

					if(!bCanGetNew)
					{//아이템이 빠졌으므로 시간 정보 삭제
						ClearWorkBenchSlotInfoRecvTime(kWorkBenchGUID, stSlot);
					}
				}

				if(PT_N_C_NFY_HOME_WORKBENCH_INFO == wPacketType)
				{
					SWorkBenchEvent& kEvt = m_kContWorkBenchEvent[kWorkBenchGUID];
					SWorkBenchLog kLog;
					kLog.ReadFromPacket(rkPacket);
					if( false == JobSkillWorkBenchUtil::IsSystemLog(kLog.eEventType) )
					{
						kEvt.m_kContLog.push_front(kLog);
					}
				}
				UpdateFurnitureMeshState(kWorkBenchGUID, kWorkBench_NewInfo);	// 보여지는 메쉬를 업데이트
				UpdateUI();	// UI가 켜져있으면 업데이트 한다
			}
			else
			{// 에러 메세지
				ShowErrMsg(eResult);
			}
		}break;
	case PT_N_C_ANS_ADD_WORKBENCH:
		{// 작업대 설치 결과
			BM::GUID kCharGuid;
			EWorkBenchResult eResult = WBR_SUCCESS;
			rkPacket.Pop(kCharGuid);
			rkPacket.Pop(eResult);
			if(WBR_SUCCESS == eResult)
			{// 성공하면, 작업대 GUID를 얻어와 정보를 갱신하거나 추가하고
				BM::GUID kWorkBenchGUID;		
				rkPacket.Pop(kWorkBenchGUID);
				m_kWorkBenchMgr.ReadFromPacket(kWorkBenchGUID, rkPacket);
				PgJobSkillWorkBench kWorkBench_NewInfo;
				if(!m_kWorkBenchMgr.GetWorkBench(kWorkBenchGUID, kWorkBench_NewInfo))	{	return;	}
				UpdateFurnitureMeshState(kWorkBenchGUID, kWorkBench_NewInfo);
				UpdateUI();
			}
			else
			{// 에러 메세지
				ShowErrMsg(eResult);
			}
		}break;
	case PT_N_C_ANS_DEL_WORKBENCH:
		{// 작업대 제거 결과 패킷을 받으면
			BM::GUID kCharGuid;
			EWorkBenchResult eResult = WBR_SUCCESS;
			rkPacket.Pop(kCharGuid);
			rkPacket.Pop(eResult);
			if(WBR_SUCCESS == eResult)
			{// 성공하면, 작업대 GUID를 얻어와
				BM::GUID kWorkBenchGUID;
				rkPacket.Pop(kWorkBenchGUID);
				// 해당 작업대에 대한 정보를 삭제한다
				bool bIsPublicAlterWorkBench = false;
				m_kWorkBenchMgr.Del(kWorkBenchGUID, bIsPublicAlterWorkBench);
				ClearWorkBenchInfoRecvTime(kWorkBenchGUID);
				UpdateUI();
			}
			else
			{// 에러 메세지
				ShowErrMsg(eResult);
			}
		}break;
	case PT_N_C_NFY_JS_WORKBENCH_LOG:
		{// 로그만 갱신됨
			BM::GUID kWorkBenchGUID;
			rkPacket.Pop(kWorkBenchGUID);
			{
				SWorkBenchEvent& kEvt = m_kContWorkBenchEvent[kWorkBenchGUID];
				SWorkBenchLog kLog;
				kLog.ReadFromPacket(rkPacket);
				kEvt.m_kContLog.push_front(kLog);
				if( GetMainUI()
					&& GetCurWorkBenchGUID() == kWorkBenchGUID 
					)
				{
					UpdateLog(kWorkBenchGUID);
				}
			}
		}break;
	case PT_M_C_NFY_JS_WORKBENCH_ERR:
		{
			EWorkBenchResult eResult;
			rkPacket.Pop(eResult);
			ShowErrMsg(eResult);
		}break;
	case PT_N_C_ANS_BLESS_WORKBENCH_MSG:
		{
			if(g_pkWorld)
			{
				PgHome* pkHome = g_pkWorld->GetHome();
				if(!pkHome)
				{
					return;
				}
				PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
				if(!pkHomeUnit)
				{
					return;
				}
				std::wstring const& rkOwnerName = pkHomeUnit->OwnerName();
				BM::vstring vStr(TTW(799798));
				vStr.Replace(L"#NAME#", rkOwnerName);
				::Notice_Show(static_cast<std::wstring>(vStr), 9, true);
			}
		}break;
	case PT_N_C_NFY_WORKBENCH_MSG:
		{
			int iMsgType =0;
			rkPacket.Pop(iMsgType);
			BM::vstring vStr;
			switch(iMsgType)
			{
			case WBMTO_HELP:
				{// 누군가에게 도움 받았을때
					std::wstring kCharName;
					rkPacket.Pop(kCharName);
					vStr = TTW(799720);
					vStr.Replace(L"#WHO#", kCharName);
				}break;
			case WBMTO_REPAIR:
				{// 누군가에게 수리를 받았을때
					std::wstring kCharName;
					rkPacket.Pop(kCharName);
					vStr = TTW(799721);
					vStr.Replace(L"#WHO#", kCharName);
				}break;
			case WBMTO_DURATION_ZERO:
				{// 내구도 0이 되어 가공 정지
					vStr = TTW(799725);
				}break;
			case WBMTO_TROUBLE:
				{// 가공장치에 고장
					vStr = TTW(799726);
				}break;
			case WBMTO_UPGRADE_COMPLETE:
				{// 아이템 완료된것 알림
					std::wstring kItemStr;
					int iItemNo = 0;
					rkPacket.Pop(iItemNo);

					vStr = TTW(799807);
					::GetItemName(iItemNo, kItemStr);
					vStr.Replace(L"#FROM#", kItemStr);
				}break;
			case WBMTO_AUTO_REPAIR:
			case WBMTO_MGR_AUTO_REPAIR:
				{// 스스로 자동수리
					vStr = TTW(799826);
					BM::GUID kWorkBenchGuid;
					rkPacket.Pop(kWorkBenchGuid);
					PgJobSkillWorkBench kWorkBench;
					if( true == m_kWorkBenchMgr.GetWorkBench(kWorkBenchGuid, kWorkBench) )
					{
						std::wstring kItemStr;
						::GetItemName(kWorkBench.ItemNo(), kItemStr);
						vStr.Replace(L"#NAME#", kItemStr);
					}
				}break;
			case WBMTO_COMPLETE_ITEM_SEND_MAIL:
				{// 완료 아이템 메일로 발송
					vStr = TTW(799827);
					std::wstring kItemStr;
					int iItemNo = 0;
					rkPacket.Pop(iItemNo);
					::GetItemName(iItemNo, kItemStr);
					vStr.Replace(L"#FROM#", kItemStr);
				}break;
			case WBMTO_ALREADY_EXIST_PUBLIC_ALTER:
				{// 한개만 설치가능해 더이상 설치할수 없다
					vStr = TTW(799829);
				}break;
			default:
				{// 정의 되지않은 오류
					vStr = TTW(5822);
				}break;
			}
			::Notice_Show(static_cast<std::wstring>(vStr), 9, true);
		}break;
	default:
		{
		}break;
	}
}

void PgJobSkillWorkMachine::ReqInsertToWorkBench(SItemPos const& rkItemPos)
{// 작업대에 아이템 넣기 요청
	if(!g_pkWorld)
	{
		return;
	}
	PgHome* pkHome = g_pkWorld->GetHome();
	if( !pkHome )
	{
		return;
	}

	PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
	if( !pkHomeUnit )
	{
		return;
	}

	BM::GUID const& kHomeGuid = pkHomeUnit->GetID();
	BM::GUID const& kWorkBenchGuid = GetCurWorkBenchGUID();
	
	PgJobSkillWorkBench kWorkBench;
	if(!m_kWorkBenchMgr.GetWorkBench(kWorkBenchGuid, kWorkBench))
	{
		return;
	}

	if(g_pkWorld)
	{
		float nowtime = g_pkApp->GetAccumTime();
		if((nowtime-g_Last_REQ_INSERT_TO_WORKBENCH_Time)<1.2f)	
		{//복사방지. 1.2초에 한번만 패킷 보낼 수 있도로 제한
			lwAddWarnDataTT(700113);//잠시 기다려야 합니다.
			return;
		}
		else
		{
			g_Last_REQ_INSERT_TO_WORKBENCH_Time = nowtime;
		}
	}

	size_t const stEmptySlot = kWorkBench.GetEmptySlotNo();
	if(0 >= stEmptySlot)
	{// 비어있는 슬롯이 없다면 메세지 박스로 알려주고
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790721, true);
		return;
	}
	size_t const stMaxSlot = static_cast<size_t>( JobSkillWorkBenchUtil::GetWorkBenchSlotCount(kWorkBench.ItemNo()) );
	
	BM::Stream kPacket(PT_C_M_REQ_INSERT_TO_WORKBENCH);
	kPacket.Push( kHomeGuid );
	kPacket.Push( kWorkBenchGuid );
	kPacket.Push( stEmptySlot );
	kPacket.Push( rkItemPos );
	NETWORK_SEND( kPacket );
	//사운드 재생
	PgActor *pkPlayerActor = g_kPilotMan.GetPlayerActor();
	if(pkPlayerActor)
	{
		pkPlayerActor->PlayNewSound(NiAudioSource::TYPE_3D, "button_UI_Open", 0.0f);
	}
}

void PgJobSkillWorkMachine::ShowErrMsg(int const iResult) const
{
	switch(iResult)
	{
	case WBR_SYSTEM_ERROR:
		{// 시스템 에러
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799700, true);
		}break;
	case WBR_WORONG_TYPE:
		{// 작업대 타입과 맞지 않는 재료 아이템
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799701, true);
		}break;
	case WBR_NOT_EXIST_ITEM:
		{// 아이템이 존재하지 않는다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799702, true);
		}break;
	case WBR_DURATION_ZERO:
		{// 내구도가 0이다.
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799703, true);
		}break;
	case WBR_NOT_OWNER:
		{// 소유자가 아니다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799704, true);
		}break;
	case WBR_SLOT_IS_NOT_EMPTY:
		{// 빈 슬롯이 아니다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799705, true);
		}break;
	case WBR_IS_NOT_EMPTY:
		{// 슬롯에 뭔가 있어서 삭제할 수 없다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799706, true);
		}break;
	case WBR_IS_NOT_UPGRADE_ITEM:
		{// 채집 업그레이드 가능한 아이템이 아니다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799707, true);
		}break;
	case WBR_EMPTY_BLESS_POINT:
		{// 축복 게이지를 모두 소비 했다
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799715, true);
		}break;
	case WBR_CANT_UPGRADE_COUNT:
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799795, true);
		}break;
	default:
		{
		}break;
	}
}

void PgJobSkillWorkMachine::SetCurWorkBenchGUID(BM::GUID const& kWorkBenchGUID)
{
	m_kCurWorkBenchGUID = kWorkBenchGUID;
}

void PgJobSkillWorkMachine::InitUI(PgMyHome* const pkHomeUnit, PgFurniture* const pkWorkBenchFurniture)
{
	if(!pkHomeUnit
		|| !pkWorkBenchFurniture
		)
	{
		return;
	}
	CItemDef* pkItemDef = pkWorkBenchFurniture->GetItemDef();
	if(!pkItemDef)
	{
		return;
	}	
	// 현재 UI에 보여줄 작업대의 GUID를 설정하고
	BM::GUID const& rkWorkBenchGUID = pkWorkBenchFurniture->GetGuid();
	SetCurWorkBenchGUID(rkWorkBenchGUID);

	PgJobSkillWorkBench kWorkBench;
	if(!m_kWorkBenchMgr.GetWorkBench(rkWorkBenchGUID, kWorkBench))
	{// // 작업대의 정보를 얻어와
		return;
	}
	int const iJobSkillFunitureType = pkItemDef->GetAbil(AT_JOBSKILL_FURNITURE_TYPE);
	// 기계 그림
	lwJobSkillWorkMachine::lwSetMachineImg(iJobSkillFunitureType);
	// 기계 이름 설정하고
	lwJobSkillWorkMachine::lwSetMachineName(iJobSkillFunitureType);
	//내구도를 보여주고
	int const iDur = kWorkBench.Duration();
	int const iMaxDur = pkItemDef->MaxAmount();
	lwJobSkillWorkMachine::lwSetMachineDurability(iDur, iMaxDur);
	// 기계 소유자 플레이어 이름을 보여주고
	std::wstring const& rkOwnerName = pkHomeUnit->OwnerName();
	lwJobSkillWorkMachine::lwSetOwnerName(lwWString(rkOwnerName));
	// 고장, 도움 받은 상태인가 확인해서 UI 표시
	bool const bHelped = ( 0 < kWorkBench.BlessDurationSec() ) ? true : false;
	lwJobSkillWorkMachine::lwShowRepairHelpIcon(kWorkBench.HasTrouble(), bHelped);
	
	{/// 작업대 슬롯에 올라가있는 아이템과 X표를 보여준다
		int const iMaxSlot_UI = lwJobSkillWorkMachine::GetUIMaxSlotCnt();									// UI에서 표시할수 있는 최대 슬롯갯수를 얻고
		int const iMaxSlot_WorkBench = JobSkillWorkBenchUtil::GetWorkBenchSlotCount(kWorkBench.ItemNo());	// 작업대에서 사용하는 최대 슬롯 갯수를 얻은 후

		for(int i=static_cast<int>(JobSkillWorkBenchUtil::iStartSlotNo);
			i<iMaxSlot_UI+static_cast<int>(JobSkillWorkBenchUtil::iStartSlotNo);
			++i
			)
		{//UI에서 사용하는 갯수가 더 큰것이 정상이므로, UI에서 사용하는 최대슬롯 갯수만큼 loop를 돌며 슬롯에 있는 업그레이드 정보를 얻어와
			SJobSkillWorkBenchSlotItemStatus kUpgradeInfo;
			kWorkBench.GetUpgradeSlotItemStatus(i, kUpgradeInfo);
			bool const bClose = (iMaxSlot_WorkBench<i);
			bool const bStrokeAni = (static_cast<size_t>(i) == kWorkBench.CurUpgradeSlot() && kUpgradeInfo.iItemNo != 0) ? true : false;
			lwJobSkillWorkMachine::SetSlotUI(kUpgradeInfo.iItemNo, i, bClose, bStrokeAni, !kUpgradeInfo.IsRemainUpgrade());	// 세팅
		}
	}

	SJobSkillWorkBenchSlotItemStatus kCurrentUpgradeInfo;
	if(kWorkBench.GetCurrentUpgradeSlotItemStatus(kCurrentUpgradeInfo))
	{// 현재 작업되고있는 슬롯의 남은 횟수와, 남은 시/분을 보여주고
		__int64 const i64CurTime = g_kEventView.GetLocalSecTime();
		if( InitInfoRecvTime(rkWorkBenchGUID, kCurrentUpgradeInfo, kWorkBench.CurUpgradeSlot(), i64CurTime, false) )
		{
			UpdateTimeUI(kWorkBench);
		}
	}
	else
	{// 현재 작업중인 슬롯이 없으면 시간표시와 프로그래스바를 없애고
		lwJobSkillWorkMachine::lwSetUpgradeInfo(0,0,0,0);
		lwJobSkillWorkMachine::lwSetProgressBarSize(0, MAX_PROGRESS);
	}

	{/// 완료된 슬롯에 완료 표시
		for(int i=0; i< 4; ++i)
		{// 슬롯에 완료 UI를 모두 off 한후
			lwJobSkillWorkMachine::lwShowUpgradeCompleteUI(i, false);
		}
		PgJobSkillWorkBench::CONT_INT kContCompleteSlot;
		kWorkBench.GetCompleteSlotNum(kContCompleteSlot);
		PgJobSkillWorkBench::CONT_INT::const_iterator kItor = kContCompleteSlot.begin();
		while(kContCompleteSlot.end() != kItor)
		{// 완료된 슬롯들을 찾아 완료 표시를 켜 준다
			int const iSlot = (*kItor);
			int const iUISlotNo = iSlot - static_cast<int>(JobSkillWorkBenchUtil::iStartSlotNo);
			SJobSkillWorkBenchSlotItemStatus kUpgradeInfo;
			if(kWorkBench.GetUpgradeSlotItemStatus(iSlot, kUpgradeInfo) )
			{
				__int64 const i64CurTime = g_kEventView.GetLocalSecTime();
				InitInfoRecvTime(rkWorkBenchGUID, kUpgradeInfo, iSlot, i64CurTime, true);
			}
			lwJobSkillWorkMachine::lwShowUpgradeCompleteUI(iUISlotNo, true);
			++kItor;
		}
	}
	
	{// 가구 메쉬 상태 표현
		int const iWorkBenchMaxSlot = JobSkillWorkBenchUtil::GetWorkBenchSlotCount( kWorkBench.ItemNo() );
		bool bIsWorking = false;
		for(int i=static_cast<int>(JobSkillWorkBenchUtil::iStartSlotNo); 
			i<iWorkBenchMaxSlot+static_cast<int>(JobSkillWorkBenchUtil::iStartSlotNo); 
			++i
			)
		{// 하나라도 작업중인지 확인해서
			SJobSkillWorkBenchSlotItemStatus kUpgradeInfo;
			kWorkBench.GetUpgradeSlotItemStatus(i, kUpgradeInfo);
			if(kUpgradeInfo.IsRemainUpgrade())
			{
				bIsWorking = true;
				break;
			}
		}

		CONT_DEF_JOBSKILL_ITEM_UPGRADE::mapped_type kDefJSUpgradeInfo;
		if(JobSkillWorkBenchUtil::GetUpgradeInfo(kCurrentUpgradeInfo.iItemNo, kDefJSUpgradeInfo))
		{// 가구 메쉬를 현재 상태에 맞게 보일수 있게 하고
			SetFurnitureMeshState(pkWorkBenchFurniture, kWorkBench.GetStatus(), kWorkBench.HasTrouble(), &kDefJSUpgradeInfo, bIsWorking);
		}
	}
	
	// 로그 설정
	UpdateLog(rkWorkBenchGUID);
}

void PgJobSkillWorkMachine::UpdateTimeUI(PgJobSkillWorkBench const& rkWorkBench)
{
	SJobSkillWorkBenchSlotItemStatus kCurrentUpgradeInfo;
	if(!rkWorkBench.GetCurrentUpgradeSlotItemStatus(kCurrentUpgradeInfo))
	{
		return;
	}
	
	STimeInfo kTimeInfo(0,0);
	if( !GetInfoRecvTime( GetCurWorkBenchGUID(), GetCurUpgradeSlot(), kTimeInfo) )
	{
		return;
	}
		
	__int64 const i64CurTime = g_kEventView.GetLocalSecTime();
	__int64 i64ElapsedTime = kTimeInfo.GetElapsedTime();
	
	__int64 i64TotalSec_Text = 0;
	__int64 i64TotalSec = 0;
	__int64 i64Per1000 = 0;
	__int64 iTurnSec = 0;
	int const iAdjustRate = m_kWorkBenchMgr.PublicAlterInfo().GetWorkTimeRate() + rkWorkBench.AlterInfo().GetWorkTimeRate();
	// 시간 글자 업데이트
	if(kTimeInfo.m_bSeted)
	{
		i64TotalSec = kTimeInfo.m_i64_Text_RemainTime;
		iTurnSec = kTimeInfo.m_i64_Progress_TurnTime;
	}
	else
	{
		i64TotalSec = kTimeInfo.GetCalcRemainTotalSec( rkWorkBench, rkWorkBench.CurUpgradeSlot(), iAdjustRate );
		kTimeInfo.m_i64_Text_RemainTime = i64TotalSec;
		// 프로그래스바 업데이트
		iTurnSec = kCurrentUpgradeInfo.iTurnSec;
		if(0 < rkWorkBench.BlessDurationSec())
		{
			if( rkWorkBench.BlessDurationSec() > kTimeInfo.m_i64RemainTime ) 
			{
				iTurnSec = iTurnSec/PgJobSkillWorkMachine::GetRemainTimeReduceValue();
			}
			else
			{
				iTurnSec = iTurnSec - rkWorkBench.BlessDurationSec() * PgJobSkillWorkMachine::GetRemainTimeReduceValue() + rkWorkBench.BlessDurationSec();
			}
		}
		iTurnSec = (iTurnSec*ABILITY_RATE_VALUE)/(iAdjustRate+ABILITY_RATE_VALUE); // 지나가는 시간은 1배율이므로
		kTimeInfo.m_i64_Progress_TurnTime = iTurnSec;
		kTimeInfo.m_bSeted = true;
		OverrideInfoRecvTime(GetCurWorkBenchGUID(), GetCurUpgradeSlot(), kTimeInfo);
	}

	i64TotalSec_Text = i64TotalSec - i64ElapsedTime;
	int iHour = 0, iMin = 0, iSec = 0;
	GetCalcSecToTime(i64TotalSec_Text, iHour, iMin, iSec);
	
	int const iRemainCnt = JobSkillWorkBenchUtil::GetRemainUpgradeCount(kCurrentUpgradeInfo.iItemNo);
	lwJobSkillWorkMachine::lwSetUpgradeInfo(iRemainCnt, iHour, iMin, iSec);
	
	if(0 < iRemainCnt
		&& 0 < iTurnSec
		)
	{
		__int64 const i64CalcSec = i64TotalSec - i64ElapsedTime;
		i64Per1000 = MAX_PROGRESS - (i64CalcSec * MAX_PROGRESS) / iTurnSec;
	}
	else if(0 >= iTurnSec)
	{
		i64Per1000 = 0;
	}
	else
	{
		i64Per1000 = MAX_PROGRESS;
	}
	lwJobSkillWorkMachine::lwSetProgressBarSize(static_cast<int>(i64Per1000), MAX_PROGRESS);
}

//void PgJobSkillWorkMachine::UpdateTimeUI(PgJobSkillWorkBench const& rkWorkBench)
//{
//	SJobSkillWorkBenchSlotItemStatus kCurrentUpgradeInfo;
//	if(!rkWorkBench.GetCurrentUpgradeSlotItemStatus(kCurrentUpgradeInfo))
//	{
//		return;
//	}
//	
//	STimeInfo kTimeInfo(0,0);
//	if( !GetInfoRecvTime( GetCurWorkBenchGUID(), GetCurUpgradeSlot(), kTimeInfo) )
//	{
//		return;
//	}
//		
//	__int64 const i64CurTime = g_kEventView.GetLocalSecTime();
//	__int64 i64ElapsedTime = kTimeInfo.GetElapsedTime();
//	int const iRemainCnt = JobSkillWorkBenchUtil::GetRemainUpgradeCount(kCurrentUpgradeInfo.iItemNo);
//	
//	__int64 i64TotalSec = 0;
//	__int64 iTurnSec = 0;
//	
//	if( kTimeInfo.Test(rkWorkBench, GetRemainTimeReduceValue(), i64TotalSec, iTurnSec) )
//	{
//		OverrideInfoRecvTime(GetCurWorkBenchGUID(), GetCurUpgradeSlot(), kTimeInfo);
//	}
//
//	// 시간 글자 업데이트
//	__int64 i64TotalSec_Text = i64TotalSec - i64ElapsedTime;
//	int iHour = 0, iMin = 0, iSec = 0;
//	GetCalcSecToTime(i64TotalSec_Text, iHour, iMin, iSec);
//	lwJobSkillWorkMachine::lwSetUpgradeInfo(iRemainCnt, iHour, iMin, iSec);
//	
//	// 프로그래스 업데이트
//	__int64 i64Per1000 = MAX_PROGRESS;
//	if(0 < iRemainCnt)
//	{
//		__int64 const i64CalcSec = i64TotalSec - i64ElapsedTime;
//		i64Per1000 = MAX_PROGRESS - (i64CalcSec * MAX_PROGRESS) / iTurnSec;
//	}
//
//	lwJobSkillWorkMachine::lwSetProgressBarSize(static_cast<int>(i64Per1000), MAX_PROGRESS);
//}

void PgJobSkillWorkMachine::UpdateLog(BM::GUID const& rkWorkBenchGUID)
{
	lwJobSkillWorkMachine::lwClearLogList();
	m_kContLogText.clear();

	CONT_JS_WORKBENCH_EVENT::const_iterator kItor = m_kContWorkBenchEvent.find(rkWorkBenchGUID);
	if(m_kContWorkBenchEvent.end() != kItor)
	{
		SWorkBenchEvent const& kEvt = (*kItor).second;
		CONT_JS_WORKBENCH_LOG const& kContLog = kEvt.m_kContLog;
		CONT_JS_WORKBENCH_LOG::const_iterator kLog_Itor = kContLog .begin();
		int iIdx =0;
		while(kContLog.end() != kLog_Itor)
		{
			SWorkBenchLog const& kLog = (*kLog_Itor);
			std::wstring const& kWho = kLog.kWhoName;
			int iFromItemNo=0;
			int iToItemNo=0;

			BM::vstring kOutputStr;
			std::wstring kFromItemStr;
			std::wstring kToItemStr;
			
			switch(kLog.eEventType)
			{
			case WBET_NONE:
			case WBET_BLESS_END:
				{
				}break;
			case WBET_BLESS: 
				{// 축복 주었다(Who)
					kOutputStr = TTW(799720);
					kOutputStr.Replace(L"#WHO#", kWho);
				}break;
			case WBET_REPAIR:
				{// 누가 수리했다(Who)
					kOutputStr = TTW(799721);
					kOutputStr.Replace(L"#WHO#", kWho);
				}break;
			case WBET_UPGRADE:
				{// 업그레이드 되었다(From -> To)
					kOutputStr = TTW(799722);
					kLog.Read(iFromItemNo, iToItemNo);
					::GetItemName(iFromItemNo, kFromItemStr);
					::GetItemName(iToItemNo, kToItemStr);
					kOutputStr.Replace(L"#FROM#", kFromItemStr);
					kOutputStr.Replace(L"#TO#", kToItemStr);
				}break;
			case WBET_UPGRADE_FAIL:
				{// 업그레이드 하지만 실패다
					kOutputStr = TTW(799723);
					kLog.Read(iFromItemNo, iToItemNo);
					::GetItemName(iFromItemNo, kFromItemStr);
					::GetItemName(iToItemNo, kToItemStr);
					kOutputStr.Replace(L"#FROM#", kFromItemStr);
					kOutputStr.Replace(L"#TO#", kToItemStr);
				}break;
			case WBET_UPGRADE_TROUBLE_FAIL:
				{// 업그레이드 하지만 고장으로 실패다
					kOutputStr = TTW(799724);
					kLog.Read(iFromItemNo, iToItemNo);
					::GetItemName(iFromItemNo, kFromItemStr);
					::GetItemName(iToItemNo, kToItemStr);
					kOutputStr.Replace(L"#FROM#", kFromItemStr);
					kOutputStr.Replace(L"#TO#", kToItemStr);
				}break;
			case WBET_STOP:
				{// 내구도가 다되어 정지 됬다
					kOutputStr = TTW(799725);
				}break;
			case WBET_TROUBLE:
				{// 고장 났다
					kOutputStr = TTW(799726);
				}break;
			case WBET_MISSING_ITEM:
				{// 완료 되었는데 시간이 지나 사라졌다
					kOutputStr = TTW(799727);
					kLog.Read(iFromItemNo, iToItemNo);
					::GetItemName(iFromItemNo, kFromItemStr);
					kOutputStr.Replace(L"#FROM#", kFromItemStr);
				}break;
			case WBET_END:
				{// 가공이 완료 되었다.
					kOutputStr = TTW(799807);
					kLog.Read(iFromItemNo, iToItemNo);
					::GetItemName(iFromItemNo, kFromItemStr);
					kOutputStr.Replace(L"#FROM#", kFromItemStr);
				}break;
			case WBET_AUTO_REPAIR:
			case WBET_MGR_AUTO_REPAIR :
				{// 스스로 자동수리
					kOutputStr = TTW(799828);
				}break;
			case WBET_COMPLETE_ITEM_SEND_MAIL:
				{// 가공 완료 아이템이 메일로 발송
					kOutputStr = TTW(799827);
					kLog.Read(iFromItemNo, iToItemNo);
					::GetItemName(iFromItemNo, kFromItemStr);
					kOutputStr.Replace(L"#FROM#", kFromItemStr);
				}break;
			default:
				{
				}break;
			}
			BM::DBTIMESTAMP_EX const& kDateTime = kLog.kDateTime;
			AddMachineLog(iIdx, kDateTime.month, kDateTime.day, kDateTime.hour, kDateTime.minute, kOutputStr);
			++iIdx;
			++kLog_Itor;
		}
	}
}

void PgJobSkillWorkMachine::SetFurnitureMeshState(PgFurniture* pkWorkBenchFuniture, int const iState, bool const bTrouble, SJobSkillItemUpgrade const* pkItemUpgradeInfo, bool const bWorking) const
{//현재 상태에 맞는 동작노드 키기
	if(!pkWorkBenchFuniture)
	{
		return;
	}
	NiAVObject* pkRootObj = pkWorkBenchFuniture->GetNIFRoot();
	if(!pkRootObj)
	{
		return;
	}
	if(!pkRootObj)
	{
		return;
	}
	static int const IDLE			= 1<<0;	// 대기
	static int const GRADE_1		= 1<<1;	
	static int const GRADE_2		= 1<<2;	
	static int const GRADE_3		= 1<<3;	
	static int const GRADE_4		= 1<<4;	
	static int const BROKEN			= 1<<5;	// 고장
	static int const WORKING_PARTICLE = 1<<6;	// 작업중 파티클 표시

	int iFlag = 0;

	if(WBS_NONE == iState
		|| WBS_STOP== iState		// 내구도가 다되어 정지되는 경우 
		)
	{//대기 상태
		iFlag=IDLE;
	}
	else if(bTrouble)
	{// 고장상태 
		iFlag = BROKEN;
	}

	if(WBS_WORKING == iState
		|| bTrouble
		|| bWorking
		)
	{// 작업중
		if(pkItemUpgradeInfo)
		{
			switch(pkItemUpgradeInfo->iGrade)
			{
			case 1:
			case 2:
				{
					iFlag |= GRADE_1;
				}break;
			case 3:
			case 4:
				{
					iFlag |= GRADE_2;
				}break;
			case 5:
				{
					iFlag |= GRADE_3;
				}break;
			case 6:
				{
					iFlag |= GRADE_4;
				}break;
			}
			
		}

		if(WBS_WORKING_TROUBLE != iState)
		{
			iFlag |= WORKING_PARTICLE;
		}
	}

	NiAVObject* pkObj = pkRootObj->GetObjectByName("Dummy_Step00");
	if(pkObj)
	{// 대기 상태
		pkObj->SetAppCulled( !(iFlag&IDLE) );
	}
	pkObj = pkRootObj->GetObjectByName("Dummy_Step01");
	if(pkObj)
	{// 진행률 0~33%
		pkObj->SetAppCulled( !(iFlag&GRADE_1) );
	}
	pkObj = pkRootObj->GetObjectByName("Dummy_Step02");
	if(pkObj)
	{// 33~66%
		pkObj->SetAppCulled( !(iFlag&GRADE_2) );
	}
	pkObj = pkRootObj->GetObjectByName("Dummy_Step03");
	if(pkObj)
	{// 66~99%
		pkObj->SetAppCulled( !(iFlag&GRADE_3) );
	}
	pkObj = pkRootObj->GetObjectByName("Dummy_Step04");
	if(pkObj)
	{// 작업 완료 상태
		pkObj->SetAppCulled( !(iFlag&GRADE_4) );
	}		
	pkObj = pkRootObj->GetObjectByName("Dummy_dmg");
	if(pkObj)
	{// 고장상태
		pkObj->SetAppCulled( !(iFlag&BROKEN) );
	}	
	pkObj = pkRootObj->GetObjectByName("Dummy_effect");
	if(pkObj)
	{// 작업 중 파티클 표시
		pkObj->SetAppCulled( !(iFlag&WORKING_PARTICLE) );
	}
}

void PgJobSkillWorkMachine::Clear()
{
	m_kCurWorkBenchGUID.Clear();
	m_kWorkBenchMgr.Clear();
	m_kContWorkBenchEvent.clear();
	m_kContWorkBenchOnUI.clear();
	m_kContWorkBenchUpgradeTime.clear();
	m_kContLogText.clear();
	XUIMgr.Close(L"SFRM_JOB_IMPROVE_MINIUI");
	XUIMgr.Close(L"SFRM_JOB_IMPROVE_MACHINE");
}

void PgJobSkillWorkMachine::UpdateAllFurnitureMeshState()
{// 모든 작업대 메쉬의 상태를 갱신 한다
	PgJobSkillWorkBenchMgr::CONT_JS_WORK_BENCH const kContWorkBench = m_kWorkBenchMgr.CopyAllWorkBenchInfo();
	PgJobSkillWorkBenchMgr::CONT_JS_WORK_BENCH::const_iterator kItor = kContWorkBench.begin();
	while(kContWorkBench.end() != kItor)
	{
		BM::GUID const& kGUID = (*kItor).first;
		PgJobSkillWorkBench const& kWorkBench = (*kItor).second;
		UpdateFurnitureMeshState(kGUID, kWorkBench);
		++kItor;
	}
	
}

void PgJobSkillWorkMachine::UpdateFurnitureMeshState(BM::GUID const& rkFurnitureGUID, PgJobSkillWorkBench const& rkWorkBench)
{// 어떤 작업대 메쉬의 상태를 갱신한다
	if(!g_pkWorld)
	{
		return;
	}
	PgHome* pkHome = g_pkWorld->GetHome();
	if(pkHome)
	{
		PgFurniture* pkWorkBenchFurniture = pkHome->GetFurniture(rkFurnitureGUID);
		if(pkWorkBenchFurniture)
		{
			SJobSkillWorkBenchSlotItemStatus kCurrentUpgradeInfo;
			if(!rkWorkBench.GetCurrentUpgradeSlotItemStatus(kCurrentUpgradeInfo))
			{// 가구는 존재하고, 정보는 없으므로, 대기 상태
				SetFurnitureMeshState(pkWorkBenchFurniture, WBS_NONE, false);
				return;
			}
			
			int const iWorkBenchMaxSlot = JobSkillWorkBenchUtil::GetWorkBenchSlotCount( rkWorkBench.ItemNo() );
			bool bIsWorking = false;
			for(int i=static_cast<int>(JobSkillWorkBenchUtil::iStartSlotNo); 
				i<iWorkBenchMaxSlot+static_cast<int>(JobSkillWorkBenchUtil::iStartSlotNo); 
				++i
				)
			{
				SJobSkillWorkBenchSlotItemStatus kUpgradeInfo;
				rkWorkBench.GetUpgradeSlotItemStatus(i, kUpgradeInfo);
				if(kUpgradeInfo.IsRemainUpgrade())
				{
					bIsWorking = true;
					break;
				}
			}

			CONT_DEF_JOBSKILL_ITEM_UPGRADE::mapped_type kDefJSUpgradeInfo;
			if(JobSkillWorkBenchUtil::GetUpgradeInfo(kCurrentUpgradeInfo.iItemNo, kDefJSUpgradeInfo))
			{
				SetFurnitureMeshState(pkWorkBenchFurniture, rkWorkBench.GetStatus(), rkWorkBench.HasTrouble(), &kDefJSUpgradeInfo, bIsWorking);
			}
		}
	}
}

void PgJobSkillWorkMachine::UpdateUI()
{
	if(!g_pkWorld)
	{
		return;
	}
	XUI::CXUI_Wnd* pkWnd = GetMainUI();
	if(pkWnd)
	{// UI가 켜져있으면 UI를 업데이트 한다
		PgHome* pkHome = g_pkWorld->GetHome();
		if(!pkHome)
		{
			return;
		}
		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if(!pkHomeUnit)
		{
			return;
		}
		PgFurniture* pkWorkBenchFurniture = pkHome->GetFurniture(m_kCurWorkBenchGUID);
		InitUI(pkHomeUnit, pkWorkBenchFurniture);
	}
	if( m_kWorkBenchMgr.IsExistWorkBench() )
	{// 작업대가 존재 하면 미니 UI를 키고
		CallMiniUI();
	}
	else
	{// 아니면 삭제
		XUIMgr.Close(L"SFRM_JOB_IMPROVE_MINIUI");
	}
}

void PgJobSkillWorkMachine::ReqRepair() const
{
	if(!g_pkWorld)
	{
		return;
	}
	PgJobSkillWorkBench kWorkBench;
	if(!m_kWorkBenchMgr.GetWorkBench(GetCurWorkBenchGUID(), kWorkBench))
	{
		return;
	}

	if(kWorkBench.HasTrouble())
	{// 현재 가구가 부서진 상태라면
		PgHome* pkHome = g_pkWorld->GetHome();
		if(!pkHome)	{ return; }	
		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if(!pkHomeUnit) { return; }
		
		BM::Stream kPacket( PT_C_M_REQ_REPAIR_WORKBENCH );
		kPacket.Push( pkHomeUnit->GetID() );	// HomeGuid
		kPacket.Push( GetCurWorkBenchGUID() );	// 작업대 GUID
		NETWORK_SEND( kPacket );
	}
	else
	{// 안됨 메세지
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799708, true);
	}
}

void PgJobSkillWorkMachine::ReqTakeItem(int iSlot, bool const bNoConfirmMsg) const 
{
	if(!g_pkWorld)
	{
		return;
	}
	BM::GUID kMyPlayerGUID;
	if(!g_kPilotMan.GetPlayerPilotGuid(kMyPlayerGUID))
	{
		return;
	}
	if(!g_kJobSkillWorkMachine.IsWorkBenchOwner(kMyPlayerGUID))
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799704, true);
		return;
	}
	PgHome* pkHome = g_pkWorld->GetHome();
	if(!pkHome)
	{
		return;
	}
	PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
	if(!pkHomeUnit)
	{
		return;
	}
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer)
	{
		return;
	}
	PgInventory* pkInv =  pkPlayer->GetInven();
	if(!pkInv )
	{
		return;
	}
	if(!pkInv->GetEmptyPosCount(IT_ETC))
	{
		lwAddWarnDataTT(3074);
		return;
	}

	PgJobSkillWorkBench kWorkBench;
	if(!m_kWorkBenchMgr.GetWorkBench(GetCurWorkBenchGUID(), kWorkBench))
	{
		return;
	}

	if(0 >= iSlot)
	{
		if(kWorkBench.IsHaveComplete())
		{// 완료된
			PgJobSkillWorkBench::CONT_INT kContSlotNum;
			kWorkBench.GetCompleteSlotNum(kContSlotNum);
			if(!kContSlotNum.empty())
			{// 슬롯번호
				iSlot = *(kContSlotNum.begin());
			}
		}
		else
		{// 작업중이면 현재 작업중인 슬롯
			iSlot = kWorkBench.CurUpgradeSlot();
		}
	}
	SJobSkillWorkBenchSlotItemStatus kUpgradeInfo;
	kWorkBench.GetUpgradeSlotItemStatus(iSlot, kUpgradeInfo);
	
	if(!kWorkBench.IsHaveComplete()
		&& 0 == kUpgradeInfo.iItemNo
		)
	{// 비어있다면
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799716, true);
	}
	else
	{
		BM::Stream kPacket( PT_C_M_REQ_GET_ITEM_FROM_WORKBENCH );
		kPacket.Push( pkHomeUnit->GetID() );
		kPacket.Push( GetCurWorkBenchGUID() );
		kPacket.Push( iSlot );
		if(bNoConfirmMsg
			|| kWorkBench.IsHaveComplete()
			|| 0 >= kWorkBench.Duration()	// 가공이 안되는 상태이므로 메세지박스를 보여줄 필요 없음
			|| iSlot !=  kWorkBench.CurUpgradeSlot()
			)
		{
			NETWORK_SEND( kPacket );
			PgActor *pkPlayerActor = g_kPilotMan.GetPlayerActor();
			if(pkPlayerActor)
			{
				pkPlayerActor->PlayNewSound(NiAudioSource::TYPE_3D, "button_UI_Open", 0.0f);
			}
		}
		else
		{
			lwCallCommonMsgYesNoBox( MB(TTW(799713)), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET );
		}
	}
}

bool PgJobSkillWorkMachine::IsAbleUseHelp() const
{
	if(!g_pkWorld)
	{
		return false;
	}
	PgHome* pkHome = g_pkWorld->GetHome();
	if(!pkHome)
	{
		return false;
	}
	BM::GUID kMyPlayerGUID;
	if(!g_kPilotMan.GetPlayerPilotGuid(kMyPlayerGUID))
	{
		return false;
	}
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkMyPlayer)
	{
		return false;
	}

	PgJobSkillWorkBench kWorkBench;
	if(!m_kWorkBenchMgr.GetWorkBench(GetCurWorkBenchGUID(), kWorkBench))
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799710, true);
		return false;
	}
	if(0 == kWorkBench.CurUpgradeSlot())
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799710, true);
		return false;
	}
	
	// 작업대 소유자가
	SGuildMemberInfo kMemberInfo;
	SFriendItem kFriendItem;
	bool const bCouple = pkMyPlayer->CoupleGuid() == m_kWorkBenchMgr.OwnerGuid(); // 커플
	bool const bGuildMember = g_kGuildMgr.GetMemberByGuid(m_kWorkBenchMgr.OwnerGuid(), kMemberInfo);// 길드원
	bool const bFriend = g_kFriendMgr.Friend_Find_ByGuid(m_kWorkBenchMgr.OwnerGuid(), kFriendItem); // 친구
	if(kMyPlayerGUID == pkHome->GetOwnerGuid()
		|| (!bCouple && !bGuildMember && !bFriend)
		)
	{
		::Notice_Show_ByTextTableNo(799711, 9, true);
		return false;
	}

	if(pkMyPlayer->GetMemberGUID() == m_kWorkBenchMgr.OwnerMemberGuid())
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799794, true);
		return false;
	}
		
	__int64 i64BlessRemainTime = m_kWorkBenchMgr.GetBlessRemainTime(kMyPlayerGUID);
	if(0 < i64BlessRemainTime)
	{
		int iHour = static_cast<int>(i64BlessRemainTime / 3600);
		i64BlessRemainTime -= iHour*3600;
		int iMin = static_cast<int>(i64BlessRemainTime / 60);
		iHour = std::max(0,iHour);
		iMin = std::max(0,iMin);

		int iTTNo = 799793;
		if(0 >= iHour)
		{
			iTTNo = 799797;
		}
		else if(0 >= iMin)
		{
			iTTNo = 799806;
		}
		BM::vstring vStr(TTW(iTTNo));
		vStr.Replace(L"#HOUR#", iHour);
		vStr.Replace(L"#MIN#", iMin);
		//lua_tinker::call<void, lwWString, bool>("CommonMsgBoxByText", lwWString(static_cast<std::wstring>(vStr)), true);
		::Notice_Show(static_cast<std::wstring>(vStr), 9, true);
		return false;
	}

	return true;
}

size_t PgJobSkillWorkMachine::GetCurUpgradeSlot() const
{
	PgJobSkillWorkBench kWorkBench;
	if(!m_kWorkBenchMgr.GetWorkBench(GetCurWorkBenchGUID(), kWorkBench))
	{
		return 0;
	}
	return kWorkBench.CurUpgradeSlot();
}

void PgJobSkillWorkMachine::CallMiniUI()
{
	XUI::CXUI_Wnd* pkMainFrame = XUIMgr.Call(L"SFRM_JOB_IMPROVE_MINIUI");
	if(!pkMainFrame)
	{
		return;
	}

	XUI::CXUI_Builder* pkBld = dynamic_cast<XUI::CXUI_Builder*>(pkMainFrame->GetControl(L"BLD_MINI_STATE"));
	if(!pkBld)
	{
		return;
	}
	m_kContWorkBenchOnUI.clear();

	int const iMaxCnt = pkBld->CountX() * pkBld->CountY();
	//int const iMaxCnt = lua_tinker::call< int >("test");
	int const iGabX = pkBld->GabX();
	int const iGabY = pkBld->GabY();
	
	POINT2 const kScreenSize =  XUIMgr.GetResolutionSize();
	int const iFirstPosX = kScreenSize.x - iGabX - pkMainFrame->Location().x;
	int iNewPosX = iFirstPosX;
	int iNewPosY = 0;

	int const iMaxColumn =  pkBld->CountX(); // 한 행에 놓을수 있는 최대 개수
	PgJobSkillWorkBenchMgr::CONT_JS_WORK_BENCH const kContWorkBench = m_kWorkBenchMgr.CopyAllWorkBenchInfo();
	PgJobSkillWorkBenchMgr::CONT_JS_WORK_BENCH::const_iterator kItor = kContWorkBench.begin();
	int const iWorkBenchCnt = static_cast<int>( kContWorkBench.size() );
	for(int j=0; j < pkBld->CountY(); ++j)
	{
		for(int i=0; i < iMaxColumn; ++i)
		{
			BM::vstring vStr(L"SFRM_MINI_STATE");
			int const iBldIndex = (i+(j*iMaxColumn));
			vStr+=iBldIndex;

			XUI::CXUI_Wnd* pkState = pkMainFrame->GetControl(vStr);
			if(!pkState) { continue;}

			if(kContWorkBench.end() != kItor)
			{
				BM::GUID const& kWorkBenchGUID = (*kItor).first;
				PgJobSkillWorkBench const& kWorkBench = (*kItor).second;
				CONT_DEF_JOBSKILL_MACHINE::mapped_type kWorkBenchInfo;
				if(true == JobSkillWorkBenchUtil::GetWorkbenchInfo(kWorkBench.ItemNo(), kWorkBenchInfo))
				{// 작업대 타입을얻어와서
					// 미니 UI 클릭시 이 GUID를 통해 해당 작업대를 호출할수 있게 기억해두고
					//if(GT_Manager == kWorkBenchInfo.iGatherType) { ++kItor; continue; }	// 관리 기계는 UI를 추가해줄 필요가 없음

					m_kContWorkBenchOnUI.insert( std::make_pair(iBldIndex, kWorkBenchGUID) );
					
					pkState->Visible(true);
					pkState->Location(iNewPosX, iNewPosY);
					{// 머신 이미지
						BM::vstring vImgName(L"FRM_IMG");
						vImgName+= (kWorkBenchInfo.iGatherType%10);
						XUI::CXUI_Wnd* pkImg = pkState->GetControl(vImgName);
						if(pkImg)
						{
							pkImg->Visible(true);
						}
					}
					{//완료된 슬롯의 수
						XUI::CXUI_Wnd* pkCompleteCnt = pkState->GetControl(L"FRM_COMPLETE_CNT");
						if(pkCompleteCnt)
						{
							int const iCnt = kWorkBench.GetCompleteSlotCnt();
							BM::vstring kCnt(iCnt);
							pkCompleteCnt->Text(kCnt);
						}
					}
					{//비어있는 슬롯의 수
						XUI::CXUI_Wnd* pkRemainSlotCnt = pkState->GetControl(L"FRM_REMAIN_SLOT_CNT");
						if(pkRemainSlotCnt)
						{
							int const iCnt = kWorkBench.GetEmptySlotCnt();
							BM::vstring kCnt(iCnt);
							pkRemainSlotCnt->Text(kCnt);
						}
					}
					{// 작업 되고 있는기계가 있으면 애니 On 아니면 Off
						bool const bUpgrading = ( 0 != kWorkBench.CurUpgradeSlot() ) && ( 0 < kWorkBench.Duration() );
						XUI::CXUI_Wnd* pkBg = pkState->GetControl(L"FRM_BG");
						if(pkBg)
						{
							pkBg->Visible(!bUpgrading);
						}
						XUI::CXUI_Wnd* pkAniBg = pkState->GetControl(L"FRM_BG_ANI");
						if(pkAniBg)
						{
							pkAniBg->Visible(bUpgrading);
						}
					}
					{// 고장난게 있는가
						bool const bTouble = kWorkBench.HasTrouble();
						XUI::CXUI_Wnd* pkImg = pkState->GetControl(L"IMG_REPAIR");
						if(pkImg)
						{
							pkImg->Visible(bTouble);
						}
					}
					{// 축복받은게 있는가
						bool const bHelped = ( 0 < kWorkBench.BlessDurationSec() ) ? true : false;
						XUI::CXUI_Wnd* pkImg = pkState->GetControl(L"IMG_HELP");
						if(pkImg)
						{
							pkImg->Visible(bHelped);
						}
					}

					++kItor;
					iNewPosX-=iGabX;	// 다음 UI의 X 위치 갱신
				}
			}
			else
			{// UI를 감춰야지
				pkState->Visible(false);
			}
		}
		if(iWorkBenchCnt > iMaxColumn)
		{// 다음 UI의 위치 갱신
			iNewPosX = iFirstPosX;
			iNewPosY+=iGabY;
		}
	}
	
	XUI::CXUI_Wnd* pkTotal = pkMainFrame->GetControl(L"FRM_TOTAL");
	if(pkTotal)
	{
		BM::vstring vStr(TTW(791111));
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(pkPlayer) 
		{
			if( m_kWorkBenchMgr.OwnerGuid() == pkPlayer->GetID() )
			{
				pkTotal->Visible(true);
				// 현재 설치된 작업대의 개수 
				int const iCur = static_cast<int>( m_kWorkBenchMgr.GetWorkBenchCnt() );
				// 현재 설치할수 있는 작업대의 최대개수
				int const iMax = static_cast<int>( JobSkillExpertnessUtil::GetBiggestMachineCount(pkPlayer->JobSkillExpertness().GetAllSkillExpertness()) );
				vStr.Replace(L"#CUR#", iCur);
				vStr.Replace(L"#MAX#", iMax);
				pkTotal->Text(vStr);
				POINT3I kPos = pkTotal->Location();
				
				int const iOrigY = 70;
				if(iWorkBenchCnt > iMaxColumn)
				{
					int const iVal = (iWorkBenchCnt-1);
					kPos.y = iGabY * (iVal/iMaxColumn);
					kPos.y += iOrigY;
				}
				else
				{
					kPos.y = iOrigY;
				}
				pkTotal->Location(kPos);
			}
			else
			{
				pkTotal->Visible(false);
			}
		}
	}
}

void PgJobSkillWorkMachine::ShowHelpSkillUI() const
{
	if(!IsAbleUseHelp())	{ return; }

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer) { return;	}

	CONT_HELP_SKILLINFO kContHelpSkillInfo;
	{
		PgJobSkillExpertness const& kExpertness = pkPlayer->JobSkillExpertness();
		PgJobSkillExpertness::CONT_EXPERTNESS kContExpertness =  kExpertness.GetAllSkillExpertness();
		PgJobSkillExpertness::CONT_EXPERTNESS::const_iterator kItor = kContExpertness.begin();
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		{			
			while(kContExpertness.end() != kItor)
			{
				int const iJobSkillNo = (*kItor).first;
				CSkillDef const* pkJobSkillDef = kSkillDefMgr.GetDef(iJobSkillNo);
				if(pkJobSkillDef)
				{
					int const iToolType = pkJobSkillDef->GetAbil(AT_JOBSKILL_TOOL_TYPE);
					if(ETOOL_WORKBENCH_SKILL_WOOD <= iToolType
						&& ETOOL_WORKBENCH_SKILL_JEWEL >= iToolType
						)
					{// 버프 주는 스킬이라면
						SHelpSkillInfo kInfo(iToolType, iJobSkillNo);
						kContHelpSkillInfo.push_back( kInfo );
					}
				}
				++kItor;
			}
		}
	}

	if(kContHelpSkillInfo.empty())
	{// 보조 스킬이 없다면
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799709, true);
		return;
	}
	if( m_kWorkBenchMgr.IsBlessedWorkBench(m_kCurWorkBenchGUID) )
	{// 축복 받은 상태이면
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799714, true);
		return;
	}

	CONT_HELP_SKILLINFO::const_iterator kItor_HelpSkillInfo = kContHelpSkillInfo.begin();
	if(1 == kContHelpSkillInfo.size())
	{
		SHelpSkillInfo const& rkInfo = (*kItor_HelpSkillInfo);
		lwJobSkillWorkMachine::lwReqdHelpJobSkill(rkInfo.iJobSkillNo);
		return;
	}

	XUI::CXUI_Wnd* pkMain = XUIMgr.Call(L"SFRM_JOB_HELP", true);
	if(!pkMain) { return; }

	POINT3I kPos = XUIMgr.MousePos();
	kPos.y+= 10;
	pkMain->Location(kPos);

	XUI::CXUI_Builder* pkBld = dynamic_cast<XUI::CXUI_Builder*>(pkMain->GetControl(L"BLD_ICON"));
	if(!pkBld) { return; }
	int const iMax = pkBld->CountX() * pkBld->CountY();

	for(int i=0; i < iMax; ++i)
	{
		BM::vstring vStr(L"FRM_ICON");
		vStr+=i;
		XUI::CXUI_Wnd* pkIcon = pkMain->GetControl(vStr);
		if(!pkIcon)	{ break; }

		if(kContHelpSkillInfo.end() != kItor_HelpSkillInfo)
		{
			SHelpSkillInfo const& rkInfo = (*kItor_HelpSkillInfo);
			pkIcon->Visible(true);
			pkIcon->SetCustomData(&rkInfo.iJobSkillNo, sizeof(rkInfo.iJobSkillNo));
			SUVInfo kUVInfo = pkIcon->UVInfo();
			kUVInfo.Index = rkInfo.iToolType + 55;
			pkIcon->UVInfo(kUVInfo);
			++kItor_HelpSkillInfo;
		}
		else
		{
			pkIcon->Visible(false);
		}
	}
}

bool PgJobSkillWorkMachine::CallWorkBenchUI(int const iMiniUIBldIdx)
{
	if(!g_pkWorld)
	{
		return false;
	}
	PgHome* pkHome = g_pkWorld->GetHome();
	if(!pkHome)
	{
		return false;
	}
	PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
	if(!pkHomeUnit)
	{
		return false;
	}

	CONT_WORKBENCH_GUID_ON_UI::const_iterator kItor =  m_kContWorkBenchOnUI.find(iMiniUIBldIdx);
	if(m_kContWorkBenchOnUI.end() == kItor)
	{
		return false;
	}
	BM::GUID const& kWorkBenchGUID =  (*kItor).second;
	PgFurniture* const pkWorkBenchFurniture = pkHome->GetFurniture(kWorkBenchGUID);
	if(!pkWorkBenchFurniture)
	{
		return false;
	}
	if( MAS_NOT_BIDDING != pkHomeUnit->GetAbil(AT_MYHOME_STATE) )
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 799789, true);
		return false;
	}

	{// UI가 뜰때 작업대 소리 재생
		CItemDef* pkItemDef = pkWorkBenchFurniture->GetItemDef();
		if(!pkItemDef)
		{
			return false;
		}
		int const iJobSkillFunitureType = pkItemDef->GetAbil(AT_JOBSKILL_FURNITURE_TYPE);
		lwJobSkillWorkMachine::lwPlayMachineSound(iJobSkillFunitureType);
	}

	XUIMgr.Call(L"SFRM_JOB_IMPROVE_MACHINE");
	InitUI(pkHomeUnit, pkWorkBenchFurniture);
	return true;
}

void PgJobSkillWorkMachine::CallSlotItemToolTip(int iSlot)
{
	BM::GUID const& kWorkBenchGuid = GetCurWorkBenchGUID();
	PgJobSkillWorkBench kWorkBench;
	if(!m_kWorkBenchMgr.GetWorkBench(kWorkBenchGuid, kWorkBench))
	{
		return;
	}
	SJobSkillWorkBenchSlotItemStatus kUpgradeInfo;
	if(!kWorkBench.GetUpgradeSlotItemStatus(iSlot, kUpgradeInfo))
	{
		return;
	}
	PgBase_Item kItem;
	kItem.ItemNo(kUpgradeInfo.iItemNo);
	std::wstring wstrText;
	std::wstring wstrLank;
	MakeToolTipText_JobSkill_Item(kItem, wstrText, wstrLank, TBL_SHOP_IN_GAME::NullData());
	lwPoint2 kPt = lwGetCursorPos();
	lwCallToolTipByText(kUpgradeInfo.iItemNo, wstrText, kPt, 0, 0, MB(wstrLank));
}

void PgJobSkillWorkMachine::UpdateCurWorkBenchRemainTime()
{
	PgJobSkillWorkBench kWorkBench;
	if(!m_kWorkBenchMgr.GetWorkBench(GetCurWorkBenchGUID(), kWorkBench))
	{// 정보를 얻어와
		return;
	}
	if( 0 < kWorkBench.Duration() )
	{
		UpdateTimeUI(kWorkBench);
	}
	else
	{
		XUI::CXUI_Wnd* pkInfo = GetMaterialUpgradeInfoUI();
		pkInfo->Text(TTW(799796));
	}
}

void PgJobSkillWorkMachine::ShowLogByToolTip(int const iIdx)
{
	CONT_LOG_TEXT::const_iterator kItor = m_kContLogText.find(iIdx);
	if(m_kContLogText.end() == kItor)
	{
		return;
	}
	std::wstring const& kStr = (*kItor).second;
	lwPoint2 kCurPos = lwGetCursorPos();
	lwCallToolTipByText(0, kStr, kCurPos);
}

void PgJobSkillWorkMachine::AddMachineLog(int const iIdx, int const iMonth, int const iDay, int const iHour, int const iMin, BM::vstring const kLog)
{//.로그를 추가해서 보여주는 부분
	XUI::CXUI_List* pkList = GetLogListUI();
	if(!pkList)
	{
		return;
	}
	XUI::SListItem* pkItem = pkList->AddItem(L"");
	if(!pkItem)
	{
		return;
	}

	XUI::CXUI_Wnd* pkEle = pkItem->m_pWnd;
	if(!pkEle)
	{
		return;
	}
	XUI::CXUI_Wnd* pkLogTime = pkEle->GetControl(L"SFRM_TIME");
	if(!pkLogTime)
	{// 로그 등록된 시간 기록하고
		return;
	}
	BM::vstring vStr(TTW(791103));;
	vStr.Replace(L"#MONTH#"	, iMonth);
	vStr.Replace(L"#DAY#"	, iDay);
	vStr.Replace(L"#HOUR#"	, iHour);
	{
		BM::vstring kTemp;
		(10 > iMin) ? kTemp+="0", kTemp+=iMin : kTemp = iMin;
		vStr.Replace(L"#MIN#"	, kTemp);
	}
	pkLogTime->Text(vStr);

	XUI::CXUI_Wnd* pkLogText = pkEle->GetControl(L"SFRM_LOG_TEXT");
	if(!pkLogTime)
	{// 로그 메세지 추가
		return;
	}

	pkEle->SetCustomData(&iIdx, sizeof(iIdx));
	std::wstring kTempLog = static_cast<std::wstring>(kLog);
	m_kContLogText.insert( std::make_pair(iIdx, kTempLog) );
	Quest::SetCutedTextLimitLength(pkLogText, kTempLog, L"...");
}

bool PgJobSkillWorkMachine::IsWorkBenchOwner(BM::GUID const& kGUID) const
{
	if(m_kWorkBenchMgr.OwnerGuid() == kGUID)
	{
		return true;
	}
	return false;
}

void PgJobSkillWorkMachine::DisplayRemainDestroyTime(int iSlot)
{
	PgJobSkillWorkBench kWorkBench;
	if(!m_kWorkBenchMgr.GetWorkBench(GetCurWorkBenchGUID(), kWorkBench))
	{
		return;
	}
	SJobSkillWorkBenchSlotItemStatus kUpgradeInfo;
	if(!kWorkBench.GetUpgradeSlotItemStatus(iSlot, kUpgradeInfo))
	{
		return;
	}

	STimeInfo kTimeInfo(0,0);
	if( !GetInfoRecvTime(GetCurWorkBenchGUID() , iSlot, kTimeInfo) )
	{
		return;
	}

	int const iRemainCnt = JobSkillWorkBenchUtil::GetRemainUpgradeCount(kUpgradeInfo.iItemNo);
	if(0 < iRemainCnt)
	{
		lwJobSkillWorkMachine::lwSetRemainTimeToDestroy(iSlot, 0, 0, 0);
		return;
	}

	// 시간 글자 표시
	__int64 const i64CurTime = g_kEventView.GetLocalSecTime();
	//DWORD const dwElapedTime_TimeText = (dwCurTime - dwRecvedTimeFromServer);
	__int64 const i64ElapedTime_TimeText = kTimeInfo.GetElapsedTime();

	//__int64 i64TotalSec = kUpgradeInfo.iTurnSec - kUpgradeInfo.iEleapsedSec;
	__int64 i64TotalSec = kTimeInfo.GetCalcRemainTotalSec( kWorkBench, iSlot); //CalcRemainTotalSec(rkWorkBench);
	i64TotalSec -= i64ElapedTime_TimeText;
	int iHour = 0;
	int iMin = 0;
	int iSec = 0;
	GetCalcSecToTime(i64TotalSec, iHour, iMin, iSec);
	
	int const iUIBldIdx = iSlot-JobSkillWorkBenchUtil::iStartSlotNo;
	if(0 > i64TotalSec) 
	{
		i64TotalSec = 0;
		iHour = iMin = iSec = 0;
	}
	lwJobSkillWorkMachine::lwSetRemainTimeToDestroy(iUIBldIdx , iHour, iMin, iSec);
}

void PgJobSkillWorkMachine::GetCalcSecToTime(__int64 i64TotalSec, int& iHour, int& iMin, int& iSec)
{
	iHour = static_cast<int>(i64TotalSec / 3600);
	i64TotalSec -= iHour*3600;
	iMin = static_cast<int>(i64TotalSec / 60);
	i64TotalSec -= iMin*60;
	iSec = static_cast<int>(i64TotalSec);

	iHour = std::max(0,iHour);
	iMin  = std::max(0,iMin);
	iSec  = std::max(0,iSec);
}

bool PgJobSkillWorkMachine::InitInfoRecvTime(BM::GUID const& rkWorkBenchGUID, SJobSkillWorkBenchSlotItemStatus const& kSlotItemStatus, int const iSlot, __int64 const i64CurTime, bool const bEnforcement)
{// 가공 완료되는 시간 및 파괴 되는 시간 계산하기 위해 서버에서 메세지 받은 시점을 기억해둠
	CONT_WORKBENCH_PROCESS_TIME kTempCont;
	CONT_WORKBENCH_UPGRADE_TIME::_Pairib kRet_Main = m_kContWorkBenchUpgradeTime.insert( std::make_pair(rkWorkBenchGUID, kTempCont) );
	
	__int64 const i64RemainTime = kSlotItemStatus.iTurnSec - kSlotItemStatus.iEleapsedSec;
	STimeInfo kTime(i64RemainTime, i64CurTime);

	CONT_WORKBENCH_PROCESS_TIME& kContSlotTime = kRet_Main.first->second;
	CONT_WORKBENCH_PROCESS_TIME::_Pairib kRet_Elem = kContSlotTime.insert( std::make_pair(iSlot, kTime) );
	if(bEnforcement)
	{// 이미 존재하더라도 초기화 시킴
		if(!kRet_Elem.second)
		{
			//(*kRet_Elem.first).second.m_i64RemainTime = i64RemainTime;
			//(*kRet_Elem.first).second.m_i64RecvedTime = i64CurTime;
			//(*kRet_Elem.first).second.m_bSeted = false;
			//(*kRet_Elem.first).second.m_i64_Text_RemainTime = 0;
			//(*kRet_Elem.first).second.m_i64_Progress_TurnTime = 0;
			(*kRet_Elem.first).second.Init(i64RemainTime, i64CurTime);
			return true;
		}
	}
	return kRet_Elem.second;
}

bool PgJobSkillWorkMachine::OverrideInfoRecvTime(BM::GUID const& rkWorkBenchGUID, int const iSlot, STimeInfo const& rkTimeInfo)
{
	CONT_WORKBENCH_UPGRADE_TIME::iterator kItor_Main = m_kContWorkBenchUpgradeTime.find( rkWorkBenchGUID );
	if(kItor_Main != m_kContWorkBenchUpgradeTime.end())
	{
		CONT_WORKBENCH_PROCESS_TIME& kContSlotTime = kItor_Main->second;
		CONT_WORKBENCH_PROCESS_TIME::iterator kItor_Elem = kContSlotTime.find( iSlot );
		if( kItor_Elem != kContSlotTime.end() )
		{
			kItor_Elem->second = rkTimeInfo;
			return true;
		}
	}
	return false;
}

bool PgJobSkillWorkMachine::GetInfoRecvTime(BM::GUID const& rkWorkBenchGUID, int const iSlot, STimeInfo& rkOut) const
{
	CONT_WORKBENCH_UPGRADE_TIME::const_iterator kItor =  m_kContWorkBenchUpgradeTime.find( rkWorkBenchGUID );
	if(m_kContWorkBenchUpgradeTime.end() == kItor)
	{
		return false;
	}
	CONT_WORKBENCH_PROCESS_TIME const& kContSlotTime = (*kItor).second;
	CONT_WORKBENCH_PROCESS_TIME::const_iterator kItor_Elem = kContSlotTime.find(iSlot);
	if(kContSlotTime.end() == kItor_Elem)
	{
		return false;
	}
	rkOut = (*kItor_Elem).second;
	return true;
}

bool PgJobSkillWorkMachine::ClearWorkBenchInfoRecvTime(BM::GUID const& rkWorkBenchGUID)
{
	CONT_WORKBENCH_UPGRADE_TIME::iterator kItor = m_kContWorkBenchUpgradeTime.find(rkWorkBenchGUID);
	if( m_kContWorkBenchUpgradeTime.end() == kItor )
	{
		return false;
	}

	CONT_WORKBENCH_PROCESS_TIME& kContSlotTime = (*kItor).second;
	kContSlotTime.clear();
	return true;
}

bool PgJobSkillWorkMachine::ClearWorkBenchSlotInfoRecvTime(BM::GUID const& rkWorkBenchGUID, int const iSlot)
{
	CONT_WORKBENCH_UPGRADE_TIME::iterator kItor = m_kContWorkBenchUpgradeTime.find(rkWorkBenchGUID);
	if( m_kContWorkBenchUpgradeTime.end() == kItor )
	{
		return false;
	}

	CONT_WORKBENCH_PROCESS_TIME& kContSlotTime = (*kItor).second;
	CONT_WORKBENCH_PROCESS_TIME::iterator kItor_Elem = kContSlotTime.find(iSlot);
	if(kContSlotTime.end() == kItor_Elem)
	{
		return false;
	}
	kContSlotTime.erase(kItor_Elem);
	return true;
}

float PgJobSkillWorkMachine::GetRemainTimeReduceValue()
{
	float const fVal = static_cast<float>( (JobSkillWorkBenchUtil::iBlessBaseValue + g_kEventView.VariableCont().iJobSkill_BlessRate) / JobSkillWorkBenchUtil::iBlessBaseValue );
	return fVal;
}

bool PgJobSkillWorkMachine::IsPublicAlterSeted() const
{
	return m_kWorkBenchMgr.PublicAlterSeted();
}

// - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - - 

__int64 PgJobSkillWorkMachine::STimeInfo::GetElapsedTime() const
{
	__int64 const i64CurTime = g_kEventView.GetLocalSecTime();
	__int64 const i64Result = i64CurTime - m_i64RecvedTime;
	return i64Result;
}

__int64 PgJobSkillWorkMachine::STimeInfo::GetCalcRemainTotalSec(PgJobSkillWorkBench const& rkWorkBench, int const iSlot, int const iRate) const
{
	SJobSkillWorkBenchSlotItemStatus kUpgradeInfo;
	if(!rkWorkBench.GetUpgradeSlotItemStatus(iSlot, kUpgradeInfo))
	{
		return 0;
	}
	int const iRemainCnt = JobSkillWorkBenchUtil::GetRemainUpgradeCount(kUpgradeInfo.iItemNo);

	__int64 i64TotalSec = 0;
	if( 0 < rkWorkBench.BlessDurationSec() 
		&& 0 < iRemainCnt
		)
	{
		__int64 i64RemainTime = m_i64RemainTime;
		if( 0 < rkWorkBench.BlessDurationSec() )
		{// 축복 시간이 있으면, 축복 적용 상태로서 남은 시간을 계산해야 한다
			i64RemainTime = i64RemainTime / PgJobSkillWorkMachine::GetRemainTimeReduceValue();
		}

		if( rkWorkBench.BlessDurationSec() > i64RemainTime )
		{
			i64TotalSec = m_i64RemainTime / PgJobSkillWorkMachine::GetRemainTimeReduceValue();
		}
		else
		{
			i64TotalSec = m_i64RemainTime 
				- ( rkWorkBench.BlessDurationSec() * PgJobSkillWorkMachine::GetRemainTimeReduceValue() )
				 + rkWorkBench.BlessDurationSec();
		}
	}
	else
	{
		i64TotalSec = m_i64RemainTime;
	}
	i64TotalSec = (i64TotalSec*ABILITY_RATE_VALUE)/(iRate+ABILITY_RATE_VALUE);

	return i64TotalSec;
}

//bool PgJobSkillWorkMachine::STimeInfo::Test(PgJobSkillWorkBench const& rkWorkBench, float const fRemainTimeReduceValue, __int64& iOutTotalSec, __int64& iOutTurnTime )
//{
//	if( IsSetted() )
//	{// 계산된값이 있으면 얻어오고
//		iOutTotalSec = GetRemainTime();
//		iOutTurnTime = GetProgressTurnTime();
//		return false;
//	}
//
//	int const iCurSlot = rkWorkBench.CurUpgradeSlot();
//	SJobSkillWorkBenchSlotItemStatus kCurrentUpgradeInfo;
//	if(!rkWorkBench.GetCurrentUpgradeSlotItemStatus(kCurrentUpgradeInfo))
//	{
//		return false;
//	}
//
//	int const iRemainCnt = JobSkillWorkBenchUtil::GetRemainUpgradeCount(kCurrentUpgradeInfo.iItemNo);
//	
//	// 가공 진행 시간 표현에 필요한 계산값
//	__int64 const i64TotalSec = GetCalcRemainTotalSec(rkWorkBench, iCurSlot);
//	m_i64_Text_RemainTime = i64TotalSec;
//	iOutTotalSec = i64TotalSec;
//
//	// 프로그래스바에 필요한 계산값
//	iOutTurnTime = kCurrentUpgradeInfo.iTurnSec;
//	if(0 < rkWorkBench.BlessDurationSec())
//	{
//		if( rkWorkBench.BlessDurationSec() > m_i64RemainTime )
//		{// 도움(축복) 시간이 남은 시간보다 클경우
//			iOutTurnTime = iOutTurnTime / fRemainTimeReduceValue;
//		}
//		else
//		{// 도움(축복) 시간이 남은 시간보다 작은경우
//			iOutTurnTime = iOutTotalSec - rkWorkBench.BlessDurationSec() * fRemainTimeReduceValue + rkWorkBench.BlessDurationSec();
//		}
//	}
//	m_i64_Progress_TurnTime = iOutTurnTime;
//	m_bSeted = true;
//	
//	return true;
//}

// - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - - 

namespace PgJobSkillWorkMachineWorkUtil
{

};

// - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - -  - - - - 
namespace lwJobSkillWorkMachine
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;
		def(pkState, "JobSkill_SetMachineDurability", lwSetMachineDurability);			// 기계 내구도
		def(pkState, "JobSkill_SetUpgradeInfo"		, lwSetUpgradeInfo);				// 업그레이드 정보(남은횟수, 남은시간
				
		def(pkState, "JobSkill_ClearLogList"		, lwClearLogList);					// 로그 모두 지움
		def(pkState, "JobSkill_SetMachineImg"		, lwSetMachineImg);					// 기계 이미지 설정
		
		def(pkState, "JobSkill_SetMachineName"		, lwSetMachineName);				// 기계 이름 설정
		def(pkState, "JobSkill_SetOwnerName"		, lwSetOwnerName);					// 기계 소유자 플레이어 이름
		def(pkState, "JobSkill_SetProgressBarSize"	, lwSetProgressBarSize);			// 진행바 크기 설정
		def(pkState, "JobSkill_ShowUpgradeCompleteUI"	, lwShowUpgradeCompleteUI);		// 완료 표시, 제거
		def(pkState, "JobSkill_SetRemainTimeToDestroy"	, lwSetRemainTimeToDestroy);	// 첫번재 메트리얼(작업되고 있는) 남은 시간 세팅

		def(pkState, "JobSkill_CallMiniUI"	, lwCallMiniUI);
		def(pkState, "JobSkill_ReqJobSkillWorkBenchInfo"	, lwReqJobSkillWorkBenchInfo);

		def(pkState, "JobSkill_ShowHelpSkillUI"	, lwShowHelpSkillUI);
		def(pkState, "JobSkill_SendHelpJobSkill", lwReqdHelpJobSkill);
		
		def(pkState, "JobSkill_Repair"	, lwReqRepair);
		def(pkState, "JobSkill_ReqTakeItem", lwReqTakeItem);

		def(pkState, "JobSkill_CallWorkBenchUI", lwCallWorkBenchUI);
		def(pkState, "JobSkill_ShowRepairHelpIcon", lwShowRepairHelpIcon);
		def(pkState, "JobSkill_CallToolTipText", lwCallToolTipText);
		def(pkState, "JobSkill_UpdateRemainTime", lwUpdateRemainTime);
		def(pkState, "JobSkill_ShowLog", lwShowLog);
		
		def(pkState, "JobSkill_HelpIconToolTip", lwHelpIconToolTip);
		def(pkState, "JobSkill_DisplayRemainDestroyTime", lwDisplayRemainDestroyTime);

		def(pkState, "JobSkill_IsMyPlayerWorkBenchOwner", lwIsMyPlayerWorkBenchOwner);
		def(pkState, "JobSkill_GetCurrentWorkBenchGUID", lwGetCurrentWorkBenchGUID);
		def(pkState, "JobSkill_HelpSkillIconToolTip", lwHelpSkillIconToolTip);
	}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	void lwReqJobSkillWorkBenchInfo()
	{
		if(!g_pkWorld)
		{
			return;
		}
		BM::Stream kPacket_ReqJobSkillWorkBenchInfo(PT_C_N_REQ_HOME_WORKBENCH_INFO);
		PgHome* pkHome = g_pkWorld->GetHome();
		if( pkHome )
		{// 작업대 정보를 
			PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
			if( pkHomeUnit )
			{// 요구하고
				kPacket_ReqJobSkillWorkBenchInfo.Push( pkHomeUnit->GetID() );
				NETWORK_SEND( kPacket_ReqJobSkillWorkBenchInfo );
			}
		}
	}

	void lwCallMiniUI()
	{
		g_kJobSkillWorkMachine.CallMiniUI();
	}

	void SetSlotUI(int const iItemNo, int const iSlot, bool const bClose, bool const bStrokeAni, bool const bComplete)
	{
		XUI::CXUI_Wnd* pkMain = GetMainUI();
		if(!pkMain)
		{
			return;
		}
		BM::vstring vStr(L"SFRM_SLOT");
		vStr+=iSlot - static_cast<int>(JobSkillWorkBenchUtil::iStartSlotNo);	// 코드에서 사용하는 슬롯은 index가 iStartSlotNo부터 시작이므로, iStartSlotNo를 빼준다
		XUI::CXUI_Wnd* pkSlotMain = (pkMain->GetControl(vStr));
		if(!pkSlotMain)
		{
			return ;
		}
		static int const EMPTY			= 1<<0;	// 
		static int const CLOSED			= 1<<1;	// 
		static int const ANI_SLOT		= 1<<2;	// 
		int iFlag = 0;
		XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(pkSlotMain->GetControl(L"ICN_SLOT"));
		if(!pkIcon)	{ return; }
				
		SIconInfo kIconInfo;
		kIconInfo.iIconGroup = KUIG_JOBSKILL_MACHINE_WORK;
		kIconInfo.iIconKey = iSlot;
		pkIcon->SetIconInfo(kIconInfo);
		if(0 < iItemNo)
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const* pkItemDef = kItemDefMgr.GetDef(iItemNo);
			if(pkItemDef)
			{
				kIconInfo.iIconResNumber = pkItemDef->ResNo();
				pkIcon->SetIconInfo(kIconInfo);
			}
		}
		else
		{// 아이템 번호가 없다면
			if(bClose)
			{// 닫히는 Slot 표현을 하거나
				iFlag |= CLOSED;
			}
			else
			{// 비어있는 표현을 해야 한다
				iFlag |= EMPTY;
			}
		}
		bool const bEmpty = iFlag & EMPTY;
		bool const bClosed = iFlag & CLOSED;

		lwDrawIconToItemNo(pkIcon, iItemNo, 1.0f, lwPoint2F(0.5f, 0.5f));
		
		//pkIcon->Visible(!bEmpty);

		XUI::CXUI_Wnd* pkClosed = pkSlotMain->GetControl(L"IMG_CLOSED");
		if(!pkClosed) { return; }
		pkClosed->Visible(bClosed);
		
		XUI::CXUI_Wnd* pkAni = pkSlotMain->GetControl(L"IMG_SLOT_ANI");
		if(!pkAni) { return; }
		pkAni->Visible(bStrokeAni);

		XUI::CXUI_Wnd* pkTime = pkSlotMain->GetControl(L"FORM_REMAIN_HOUR");
		if(!pkTime)	{ return; }
		pkTime->Text(L"");
		pkTime->Visible(false);
	}
	
	void lwShowRepairHelpIcon(bool const bTrouble, bool const bHelped)
	{
		XUI::CXUI_Wnd* pkMain = GetMainUI();
		if(!pkMain)
		{
			return;
		}
		{// 고장난게 있는가
			XUI::CXUI_Wnd* pkImg = pkMain->GetControl(L"IMG_REPAIR");
			if(pkImg)
			{
				pkImg->Visible(bTrouble);
			}
		}
		{// 축복받은게 있는가
			XUI::CXUI_Wnd* pkImg = pkMain->GetControl(L"IMG_HELP");
			if(pkImg)
			{
				pkImg->Visible(bHelped);
			}
		}
	}

	void lwShowHelpSkillUI()
	{// 도움주기 UI를 호출
		g_kJobSkillWorkMachine.ShowHelpSkillUI();
	}

	void lwReqdHelpJobSkill(int const iHelpJobSkillNo)
	{
		if(!g_pkWorld)
		{
			return;
		}
		PgHome* pkHome = g_pkWorld->GetHome();
		if(!pkHome)	{ return; }

		PgMyHome* pkHomeUnit = pkHome->GetHomeUnit();
		if(!pkHomeUnit)	{ return; }

		BM::Stream kPacket(PT_C_M_REQ_BLESS_WORKBENCH);
		kPacket.Push(iHelpJobSkillNo);
		kPacket.Push(pkHomeUnit->GetID());							// HomeGUID
		kPacket.Push(g_kJobSkillWorkMachine.GetCurWorkBenchGUID()); // 작업대 GUID
		NETWORK_SEND(kPacket);
	}

	void lwReqRepair()
	{// 수리하기 버튼 클릭시
		g_kJobSkillWorkMachine.ReqRepair();
	}

	void lwReqTakeItem(int const iSlot, bool const bNoConfirmMsg)
	{// 꺼내기 버튼 클릭시
		g_kJobSkillWorkMachine.ReqTakeItem(iSlot, bNoConfirmMsg);
	}
	
	void lwShowLog(int const iIdx)
	{
		g_kJobSkillWorkMachine.ShowLogByToolTip(iIdx);
	}

	void lwClearLogList()
	{// 로그 리스트를 모두 지운다
		XUI::CXUI_List* pkList = GetLogListUI();
		if(!pkList)
		{
			return;
		}
		pkList->ClearList();
	}

	void lwShowUpgradeCompleteUI(int const iSlot, bool const bShow)
	{//완료 표시, 제거
		XUI::CXUI_Wnd* pkMsg = GetCompleteMsgUI(iSlot);
		if(!pkMsg)
		{
			return;
		}
		pkMsg->Visible(bShow);
	}

	void lwSetRemainTimeToDestroy(int const iSlot, int const iHour, int const iMin, int const iSec)
	{//첫번째 메트리얼(작업되고 있는) 남은 시간 세팅
		XUI::CXUI_Wnd* pkSlot = GetSlotUI(iSlot);
		if(!pkSlot)
		{
			return;
		}
		//XUI::CXUI_Wnd* pkAni = pkSlot->GetControl(L"IMG_SLOT_ANI");
		//if(!pkAni)
		//{
		//	return;
		//}
		//pkAni->Visible(false);

		XUI::CXUI_Wnd* pkTime = pkSlot->GetControl(L"FORM_REMAIN_HOUR");
		if(!pkTime)
		{
			return;
		}
		pkTime->Visible(true);
		if(iHour)
		{
			BM::vstring vStr(TTW(799803));
			vStr.Replace(L"#HOUR#", iHour);
			pkTime->Text(vStr);
			return;
		}
		if(iMin)
		{
			BM::vstring vStr(TTW(799804));
			vStr.Replace(L"#MIN#", iMin);
			pkTime->Text(vStr);
			return;
		}
		if(iSec)
		{
			BM::vstring vStr(TTW(799805));
			vStr.Replace(L"#SEC#", iSec);
			pkTime->Text(vStr);
			return;
		}
		pkTime->Text(L"");
		pkTime->Visible(false);
	}

	void lwSetProgressBarSize(int const iNow, int const iMax)
	{// 작업 진행 바
		XUI::CXUI_AniBar* pkProgressBar = GetProgressUI();
		if(!pkProgressBar)
		{
			return;
		}
		if(iMax)
		{
			pkProgressBar->Max(iMax);
		}
		pkProgressBar->Now(iNow);
	}

	void lwSetMachineName(int iGatherType)
	{// 가공 기계 이름을 UI에 설정
		XUI::CXUI_Wnd* pkName = GetMachineNameUI();
		if(!pkName)
		{
			return;
		}
		iGatherType%=10;
		int iTextNo = 0;
		switch(iGatherType)
		{
		case GT_WoodMachine:
			{
				iTextNo = 791001;
			}break;
		case GT_Smelting:
			{
				iTextNo = 791002;
			}break;
		case GT_Garden:
			{
				iTextNo = 791003;
			}break;
		case GT_Fishbowl:
			{
				iTextNo = 791004;
			}break;
		case GT_Jewelry:
			{
				iTextNo = 791005;
			}break;
		default:
			{// 오류
				iTextNo = 18011;
			}break;
		}
		BM::vstring vStr(TTW(791000));
		vStr.Replace(L"#MACHINE_TYPE#", TTW(iTextNo));
		pkName->Text(vStr);
	}

	void lwSetUpgradeInfo(int const iRemainUpgradeCnt, int iHour, int iMin, int iSec)
	{//.업그레이드 정보 (남은 성장 횟수, 남은 성장 시간)
		XUI::CXUI_Wnd* pkInfo = GetMaterialUpgradeInfoUI();
		if(!pkInfo)
		{
			return;
		}
		iHour = std::max(0,iHour);
		iMin = std::max(0,iMin);
		iSec = std::max(0,iSec);

		if(iRemainUpgradeCnt == 0
			&& iHour == 0
			&& iMin == 0
			&& iSec == 0
			)
		{
			pkInfo->Text(L"");
			return;
		}
		
		BM::vstring vStr;
		int iHourTT = 791101;
		int iMinTT = 791112;
		if(0 == iRemainUpgradeCnt)
		{// 파괴되기 까지 시간을 보여줘야 한다면
			iHourTT = 799791;
			iMinTT  = 799792;
		}

		if(0 < iHour)
		{
			vStr = TTW(iHourTT);
			vStr.Replace(L"#REMAIN_HOUR#", iHour);
		}
		else
		{
			vStr = TTW(iMinTT);
			vStr.Replace(L"#REMAIN_SEC#", ((0 > iSec) ? 0 : iSec) );
		}

		if(0 < iRemainUpgradeCnt)
		{
			vStr.Replace(L"#REMAIN_UPGRD#", iRemainUpgradeCnt);
		}

		vStr.Replace(L"#REMAIN_MIN#", iMin);
		pkInfo->Text(vStr);
	}

	void lwSetOwnerName(lwWString kOwnerName)
	{// 소유자 이름 세팅
		XUI::CXUI_Wnd* pkName = GetOwnerNameUI();
		if(!pkName)
		{
			return;
		}
		pkName->Text(kOwnerName());
	}

	void lwSetMachineDurability(int const iCur, int const iMax)
	{//.내구도 변화 받아 적용하는 부분
		XUI::CXUI_Wnd* pkDur = GetDurabilityUI();
		if(!pkDur)
		{
			return;
		}
		BM::vstring vStr(TTW(791100));
		vStr.Replace(L"#CUR#", iCur);
		vStr.Replace(L"#MAX#", iMax);
		pkDur->Text(vStr);
	}

	void lwSetMachineImg(int iGatherType)
	{// 기계 이미지 표시 ( bit flag를 이용해 on/off)
		XUI::CXUI_Wnd* pkMain = GetMainUI();
		if(!pkMain)
		{
			return;
		}
		iGatherType%=10;
		int const iFlag = 1 << iGatherType;
		XUI::CXUI_Wnd* pkWood = pkMain->GetControl(L"IMG_WOOD");
		if(pkWood)
		{
			pkWood->Visible(iFlag & 1 << GT_WoodMachine);
		}		
		XUI::CXUI_Wnd* pkSmelter = pkMain->GetControl(L"IMG_SMELTER");
		if(pkSmelter)
		{
			pkSmelter->Visible(iFlag & 1 << GT_Smelting);
		}
		XUI::CXUI_Wnd* pkGarden = pkMain->GetControl(L"IMG_GARDEN");
		if(pkGarden)
		{
			pkGarden->Visible(iFlag & 1 << GT_Garden);
		}
		XUI::CXUI_Wnd* pkFish = pkMain->GetControl(L"IMG_FISH");
		if(pkFish)
		{
			pkFish->Visible(iFlag & 1 << GT_Fishbowl);
		}		
		XUI::CXUI_Wnd* pkJewel = pkMain->GetControl(L"IMG_JEWEL");
		if(pkJewel)
		{
			pkJewel->Visible(iFlag & 1 << GT_Jewelry);
		}
	}

	void lwPlayMachineSound(int iGatherType)
	{// 기계 이미지 표시 ( bit flag를 이용해 on/off)
		iGatherType%=10;
		std::wstring kSoundID;
		switch(iGatherType)
		{
		case GT_WoodMachine:
			{
				kSoundID =  WSTR_WOOD;
			}break;
		case GT_Smelting:
			{
				kSoundID =  WSTR_SMELTING;
			}break;
		case GT_Garden:
			{
				kSoundID =  WSTR_GARDEN;
			}break;
		case GT_Fishbowl:
			{
				kSoundID =  WSTR_FISH;
			}break;
		case GT_Jewelry:
			{
				kSoundID =  WSTR_JEWEL;
			}break;
		default:
			{
				return;
			}break;
		}
		g_kUISound.PlaySoundByID( kSoundID );
	}

	int GetUIMaxSlotCnt()
	{
		XUI::CXUI_Wnd* pkMain = GetMainUI();
		if(!pkMain)
		{
			return 0;
		}
		XUI::CXUI_Builder* pkBld = dynamic_cast<XUI::CXUI_Builder*>(pkMain->GetControl(L"BLD_SLOT"));
		if(!pkBld)
		{
			return 0;
		}
		return pkBld->CountX() * pkBld->CountY();
	}
	
	bool lwCallWorkBenchUI(int const iMiniUIBldIdx)
	{
		return g_kJobSkillWorkMachine.CallWorkBenchUI(iMiniUIBldIdx);
	}
	
	void lwCallToolTipText(int const iSlot)
	{
		g_kJobSkillWorkMachine.CallSlotItemToolTip(iSlot);
	}
	
	void lwUpdateRemainTime()
	{
		g_kJobSkillWorkMachine.UpdateCurWorkBenchRemainTime();
	}
	
	bool lwIsMyPlayerWorkBenchOwner()
	{
		BM::GUID kMyPlayerGUID;
		if(!g_kPilotMan.GetPlayerPilotGuid(kMyPlayerGUID))
		{
			return false;
		}
		return g_kJobSkillWorkMachine.IsWorkBenchOwner(kMyPlayerGUID);
	}

	void lwHelpIconToolTip(int iMiniUIBldIdx)
	{
		BM::vstring vStr(TTW(799802));
		float fVal = PgJobSkillWorkMachine::GetRemainTimeReduceValue();
		BM::vstring vToString(fVal, L"%.0f");
		vStr.Replace(L"#VAL#", vToString);
		lwPoint2 kPt = lwGetCursorPos();
		lwCallMutableToolTipByText(lwWString(static_cast<std::wstring>(vStr)), kPt,0, NULL, false);
	}
	
	void lwDisplayRemainDestroyTime(int iSlotNo)
	{
		g_kJobSkillWorkMachine.DisplayRemainDestroyTime(iSlotNo);
	}
	
	lwGUID lwGetCurrentWorkBenchGUID()
	{
		return g_kJobSkillWorkMachine.GetCurWorkBenchGUID();
	}
	
	void lwHelpSkillIconToolTip(int iSkillNo)
	{
		lwPoint2 kPt = lwGetCursorPos();
		CallJobSkillToolTip(g_kSkillTree.GetKeySkillNo(iSkillNo), kPt);
	}

	void TestJobMachine_MeshChange(PgFurniture* pkWorkBenchFuniture, int const iTestState)
	{//현재 상태에 맞는 동작노드 키기
		if(!pkWorkBenchFuniture)
		{
			return;
		}
		NiAVObject* pkRootObj = pkWorkBenchFuniture->GetNIFRoot();
		if(!pkRootObj)
		{
			return;
		}
		if(!pkRootObj)
		{
			return;
		}
		
		static int const IDLE			= 1<<0;	// 대기
		static int const GRADE_1		= 1<<1;
		static int const GRADE_2		= 1<<2;
		static int const GRADE_3		= 1<<3;
		static int const GRADE_4		= 1<<4;
		static int const BROKEN			= 1<<5;	// 고장
		static int const WORKING_PARTICLE = 1<<6;	// 작업중 파티클 표시

		int iFlag = 1 << iTestState;
		if(1 <= iTestState
			&& 3 >= iTestState
			)
		{
			iFlag |= WORKING_PARTICLE;
		}

		NiAVObject* pkObj = pkRootObj->GetObjectByName("Dummy_Step00");
		if(pkObj)
		{// 대기 상태
			pkObj->SetAppCulled( !(iFlag&IDLE) );
		}
		pkObj = pkRootObj->GetObjectByName("Dummy_Step01");
		if(pkObj)
		{// 진행률 0~33%
			pkObj->SetAppCulled( !(iFlag&GRADE_1) );
		}
		pkObj = pkRootObj->GetObjectByName("Dummy_Step02");
		if(pkObj)
		{// 33~66%
			pkObj->SetAppCulled( !(iFlag&GRADE_2) );
		}
		pkObj = pkRootObj->GetObjectByName("Dummy_Step03");
		if(pkObj)
		{// 66~99%
			pkObj->SetAppCulled( !(iFlag&GRADE_3) );
		}
		pkObj = pkRootObj->GetObjectByName("Dummy_Step04");
		if(pkObj)
		{// 작업 완료 상태
			pkObj->SetAppCulled( !(iFlag&GRADE_4) );
		}		
		pkObj = pkRootObj->GetObjectByName("Dummy_dmg");
		if(pkObj)
		{// 고장상태
			pkObj->SetAppCulled( !(iFlag&BROKEN) );
		}	
		pkObj = pkRootObj->GetObjectByName("Dummy_effect");
		if(pkObj)
		{// 작업 중 파티클 표시
			pkObj->SetAppCulled( !(iFlag&WORKING_PARTICLE) );
		}
	}
};


XUI::CXUI_Wnd* GetMainUI()
{
	return XUIMgr.Get(L"SFRM_JOB_IMPROVE_MACHINE");
}

XUI::CXUI_Wnd* GetDurabilityUI()
{
	XUI::CXUI_Wnd* pkMain = GetMainUI();
	if(!pkMain)
	{
		return NULL;
	}
	return pkMain->GetControl(L"SFRM_DURABILITY");
}

XUI::CXUI_Wnd* GetMaterialUpgradeInfoUI()
{
	XUI::CXUI_Wnd* pkMain = GetMainUI();
	if(!pkMain)
	{
		return NULL;
	}
	return pkMain->GetControl(L"SFRM_MATERIAL_UPGRADE_INFO");
}

XUI::CXUI_List* GetLogList()
{
	XUI::CXUI_Wnd* pkMain = GetMainUI();
	if(!pkMain)
	{
		return NULL;
	}
	XUI::CXUI_Wnd* pkListBg = pkMain->GetControl(L"SFRM_LOG_BG");//상태 설명. 건드릴 필요 없을수 있음
	if(!pkListBg)
	{
		return NULL;
	}
	return dynamic_cast<XUI::CXUI_List*>(pkListBg->GetControl(L"LST_LOG"));
}

XUI::CXUI_Wnd* GetOwnerNameUI()
{
	XUI::CXUI_Wnd* pkMain = GetMainUI();
	if(!pkMain)
	{
		return NULL;
	}
	return pkMain->GetControl(L"SFRM_OWNER_NAME");
}

XUI::CXUI_Wnd* GetMachineNameUI()
{
	XUI::CXUI_Wnd* pkMain = GetMainUI();
	if(!pkMain)
	{
		return NULL;
	}
	return pkMain->GetControl(L"SFRM_MACHINE_NAME");
}

XUI::CXUI_AniBar* GetProgressUI()
{
	XUI::CXUI_Wnd* pkMain = GetMainUI();
	if(!pkMain)
	{
		return NULL;
	}
	return dynamic_cast<XUI::CXUI_AniBar*>(pkMain->GetControl(L"ANIBAR_PROGRESS"));
}

XUI::CXUI_Wnd* GetCompleteMsgUI(int const iSlot)
{
	XUI::CXUI_Wnd* pkMain = GetMainUI();
	if(!pkMain)
	{
		return NULL;
	}
	BM::vstring vStr(L"SFRM_SLOT");
	vStr+=iSlot;
	XUI::CXUI_Wnd* pkSlot = pkMain->GetControl(vStr);
	if(!pkSlot)
	{
		return NULL;
	}
	return pkSlot->GetControl(L"SFRM_COMPLETE_MSG");
}

XUI::CXUI_Wnd* GetSlotUI(int const iSlot)
{
	XUI::CXUI_Wnd* pkMain = GetMainUI();
	if(!pkMain)
	{
		return NULL;
	}
	BM::vstring vStr(L"SFRM_SLOT");
	vStr += iSlot;
	return dynamic_cast<XUI::CXUI_Wnd*>(pkMain->GetControl(vStr));
}

XUI::CXUI_List* GetLogListUI()
{
	XUI::CXUI_Wnd* pkMain = GetMainUI();
	if(!pkMain)
	{
		return NULL;
	}
	XUI::CXUI_Wnd* pkBg = pkMain->GetControl(L"SFRM_LOG_BG");
	if(!pkBg)
	{
		return NULL;
	}			

	return dynamic_cast<XUI::CXUI_List*>(pkBg->GetControl(L"LST_LOG"));
}

XUI::CXUI_Wnd* GetMiniUI()
{
	return XUIMgr.Get(L"SFRM_JOB_IMPROVE_MINIUI");
}