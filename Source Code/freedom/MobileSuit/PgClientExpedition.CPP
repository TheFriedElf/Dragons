#include "stdafx.h"
#include "Variant/PgWaiter.h"
#include "PgError.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "pgchatMgrClient.h"
#include "PgPartyBalloon.h"
#include "PgActor.h"
#include "PgNetwork.h"
#include "PgWorld.h"
#include "PgUIScene.h"
#include "PgClientParty.h"
#include "PgClientExpedition.h"
#include "PgQuestMan.h"
#include "PgMissionComplete.h"

int const EXPEDITION_JOIN_REQ_WAIT_TIME = 15;

typedef enum eExpeditionMessageSet
{
	EMS_Is2 =						700032, //님을
	EMS_Is =						700033, //님이
	EMS_Confirm_Enter =				720010, //이동한 이후에는 인원 변경 및 추가를 하실 수 없습니다. 해당 컨텐츠로 이동 하시겠습니까?
	EMS_CantArea =					720015, //원정대 가입/탈퇴/변경을 할수 없는 지역입니다.
	EMS_Invite =					700016, //원정대 초청 메시지
	EMS_Invite_Fail =				700138, //원정대 초청 실패시(오프라인 or 없는이름)
	EMS_Invite_Fail2 =				700056, //이미 다른 파티에 가입되어 있습니다. (초대상대가 이미 파티에 있음)
	EMS_Invite_Fail3 =				402200, //하드코어 던젼 투표중이라 가입할 수 없습니다.
	EMS_Invite_Reject =				720017, //원정대 초청을 거부 하였습니다.
	EMS_Leave =						720018, //원정대를 탈퇴 하였습니다.
	EMS_Kickout =					720019, //원정대에서 추방 당하였습니다.
	EMS_NewMaster =					720020, //새로운 원정대장이 되었습니다.
	EMS_Join_Success =				720021, //원정대에 가입 되었습니다.
	EMS_Join_Fail =					720022,	//원정대 가입에 실패 하였습니다.
	EMS_Join_Full =					700063,	//인원수를 초과하여 더 이상 가입 할 수 없습니다.
	EMS_Join_HardCoreVote =			402201, //하드코어 던젼 입장 투표중인 파티라 가입 할 수 없습니다.
	EMS_NotMaster =					720023,	//원정대장이어야 가능합니다.
	EMS_DidntMe	=					700065,	//나를 초대 하거나, 추방 할 수 없습니다.
	EMS_SameExpedition =			720024, //이미 같은 원정대원입니다.
	EMS_FailChangeMaster =			720025, //원정대장을 변경 할 수 없습니다.
	EMS_ExpeditionCreate =			720026, //원정대 신청 대기 상태가 되었습니다.
	EMS_ExpeditionDestroy =			720027, //원정대가 해체 되었습니다.
	EMS_InvitePop =					720009, //님께서 원정대 초대를 요청하였습니다.
	EMS_FailReqJoin_Inviter =		700071, //파티 대기자라 다른 사람을 초대 할 수 업습니다.
	EMS_NotInviteMe =				720028, //나를 초대 하지 마
	EMS_DupPartyName =				700104, //
	EMS_Join_Level_Fail =			720029, //레벨이 맞지 않아 원정대 참가 안됨
	EMS_ChangeName_Success =		720030, //원정대명이 변경 되었습니다.
	EMS_Fail_ReqJoin =				720031, //원정대 가입신청이 거절 되었습니다.
	EMS_ExpeditionInvite_Success =	720032, //원정대 가입신청을 하였습니다.
	EMS_DupReqJoin =				720036, //이미 다른 원정대에 가입신청을 한 상태입니다.
	EMS_AnswerWait =				720033, //원정대 수락 대기중입니다.
	EMS_KickUserNoArea =			720034, //원정대원이 추방할수 없는 지역에 있습니다.
	EMS_InviteArea =				720035, //상대방이 파티를 할 수 없는 지역에 있습니다.
	EMS_Enter_Fail_Item =			720038, //원정대원 중 해당 원정대 메달이 없는 대원이 있어 해당 컨텐츠로 이동할 수 없습니다.
	EMS_Enter_Fail_Team =			720041, //원정대 교체대기 목록에 인원이 있어 입장할 수 없습니다.
	EMS_HardCoreVote =				402202, //하드코어 던젼 투표중에는 사용할 수 없는 기능이다.
	EMS_MoveTeam_Fail =				720004, //해당 팀의 인원이 꽉 차서 팀 배정을 할 수 없습니다.
} EExpeditionMessageSet;

namespace PgClientExpeditionUtil
{
	void SetExpeditionNpcGuid(lwGUID lwNpcGuid)
	{
		BM::GUID const& ExpeditionNpcGuid = lwNpcGuid();

		g_kExpedition.SetExpeditionInfo(ExpeditionNpcGuid);
	}

	lwGUID GetExpeditionNpcGuid()
	{
		BM::GUID const & NpcGuid = g_kExpedition.NpcGuid();
		return lwGUID(NpcGuid);
	}

	void SetCharacterExpeditionGuid(BM::GUID const& CharGuid, BM::GUID const& ExpeditionGuid)
	{
		PgPilot	*pPilot = g_kPilotMan.FindPilot(CharGuid);
		if( NULL == pPilot )
		{
			return;
		}

		PgActor *pActor = dynamic_cast<PgActor *>(pPilot->GetWorldObject());
		if( NULL == pActor )
		{
			return;
		}

		PgPlayer* pPlayer = dynamic_cast<PgPlayer*>(pPilot->GetUnit());
		if( NULL == pPlayer )
		{
			return;
		}

		if( BM::GUID::IsNotNull(pPlayer->ExpeditionGuid())	// 어떤 플레이어가 현재 원정대인데
		&&	BM::GUID::IsNull(ExpeditionGuid) )				// 원정대GUID가 NULL이라면
		{
			// 그 플레이어를 내 원정대 리스트에서 찾아 삭제하고 비었으면 원정대도 삭제한다.
			g_kExpedition.RemoveExpedition(pPlayer->ExpeditionGuid(), pPlayer->GetID());
		}

		pPlayer->ExpeditionGuid(ExpeditionGuid);	// 그 플레이어의 원정대GUID를 NULL로 만든다.
		pActor->UpdateName();
	}

	bool CheckEnableEnter()
	{
		std::vector< SExpeditionMember* >::const_iterator iter;

		for(iter = g_kExpedition.GetMembers().begin(); iter != g_kExpedition.GetMembers().end(); ++iter)
		{
			if( NULL == (*iter) )
			{
				continue;
			}
			// 대기팀에 인원이 있는지 체크
			if( EET_TEAM_SHIFT == (*iter)->sTeam )	// 대기 팀에 누군가 있다면
			{
				lua_tinker::call<void, char const*, bool >("CommonMsgBox", MB(TTW(EMS_Enter_Fail_Team)), true);
				return false;
			}
		}
		return true;
	}
	
	bool CheckExpeditionDailyQuest()
	{ // 원정대 일일퀘스트 완료 체크
		ContNpcQuestInfo NpcQuest;
		size_t QuestCount = g_kQuestMan.PopNPCQuestInfo(g_kExpedition.NpcGuid(), NpcQuest);
		if( !QuestCount )
		{
			return false;
		}
		return true;
	}

	void ClearNotHaveKeyItemMemberList()
	{
		g_kExpedition.ClearNotHaveKeyItemMember();
	}

	void ShowExpeditionTitleText(lwUIWnd lwWnd)
	{
		XUI::CXUI_Form *pWnd = dynamic_cast<XUI::CXUI_Form*>(lwWnd());
		if( NULL == pWnd )
		{
			return;
		}
		int const& DungeonStr = g_kExpedition.DungeonStringNo();
		pWnd->Text(TTW(DungeonStr));	// 원정대 모집 공고에 원정대 타이틀을 보여준다.
	}

	void ShowExpeditionInfoText(lwUIWnd lwWnd)
	{
		XUI::CXUI_Form *pWnd = dynamic_cast<XUI::CXUI_Form*>(lwWnd());
		if( NULL == pWnd )
		{
			return;
		}
		int const& DungeonInfoStr = g_kExpedition.DungeonInfoStringNo();
		pWnd->Text(TTW(DungeonInfoStr));	// 원정대 모집 공고에 원정대 내용을 보여준다.
	}

	void ShowExpeditionName(lwUIWnd lwWnd)
	{
		XUI::CXUI_Form *pWnd = dynamic_cast<XUI::CXUI_Form*>(lwWnd());
		if( NULL == pWnd )
		{
			return;
		}
		std::wstring const& MyExpeditionName = g_kExpedition.ExpeditionName();
		pWnd->Text(MyExpeditionName);
	}

	void ShowExpeditionArea(lwUIWnd lwWnd)
	{
		XUI::CXUI_Form *pWnd = dynamic_cast<XUI::CXUI_Form*>(lwWnd());
		if( NULL == pWnd )
		{
			return;
		}
		BM::vstring vStr(TTW(710024));				// "원정장소 :"
		int const& MyMapNo = g_kExpedition.MapNo();
		BM::vstring vStr2(GetMapName(MyMapNo));		// 번호에 해당되는 맵 이름을 가져온다.
		vStr += vStr2;
		pWnd->Text(vStr);							// 결과를 윈도우에 출력
	}

	void ShowExpeditionCurrMemNum(lwUIWnd lwWnd)
	{
		XUI::CXUI_Form *pWnd = dynamic_cast<XUI::CXUI_Form*>(lwWnd());
		if( NULL == pWnd )
		{
			return;
		}
		BM::vstring vStr(TTW(710025));								// "원정대원 :"
		BM::vstring vStr2(g_kExpedition.GetNumExpeditionMember());	// 원정대인원을 가져와서
		vStr += vStr2;												// 문자열을 합치고
		pWnd->Text(vStr);											// 결과를 윈도우에 출력
	}
	
	void ShowExpeditionImage(lwUIWnd lwWnd)
	{
		XUI::CXUI_Wnd* pWnd = (XUI::CXUI_Wnd*)lwWnd();
		if( NULL == pWnd )
		{
			return;
		}
		XUI::CXUI_Image* const pImage = dynamic_cast<XUI::CXUI_Image*>(pWnd->GetControl(L"IMG_EPIMG"));
		if( NULL == pImage )
		{
			return;
		}
	
		pImage->DefaultImgName(g_kExpedition.ImgPath());
	}

	void UpdateExpeditionMemberInfo(lwUIWnd lwWnd, int const teamNum)
	{
		XUI::CXUI_Wnd* pWnd = (XUI::CXUI_Wnd*)lwWnd();
		if( NULL == pWnd )
		{
			return;
		}
		
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_TEAM_SLOT_LIST"));
		if( NULL == pList )
		{
			return;
		}

		pList->ClearList();	// 리스트 초기화

		// 멤버들의 팀 넘버를 조사하겠습니다.
		std::vector< SExpeditionMember* >::const_iterator iter;
		
		int MemberNum = 0;	// 팀의 인원수를 카운트할 변수
		
		for(iter = g_kExpedition.GetMembers().begin(); iter != g_kExpedition.GetMembers().end(); ++iter)
		{ // 내 모든 원정대 멤버들을 조사한다.
			if( NULL == (*iter) )
			{
				continue;
			}
			if( (*iter)->sTeam == teamNum )
			{ // 창에 보여주고 싶은 팀 번호와 멤버의 팀 번호가 일치하면
				XUI::SListItem* pItem = pList->AddItem(L""); // 리스트에 등록시킨다.
				if( pItem )
				{
					XUI::CXUI_Wnd* pItemWnd = pItem->m_pWnd;
					if( pItemWnd )
					{
						pItemWnd->OwnerGuid((*iter)->kCharGuid);				// 리스트에 캐릭터 GUID를 등록한다.
						pItemWnd->SetCustomData(&MemberNum, sizeof(MemberNum));	// 그리고 0~3까지 고유 번호 등록(팀당 4명이므로)
						MemberNum++;	// 먼저 등록하고 증가시키는 이유는 해당되는 리스트가 0번부터 시작하기 때문에

						XUI::CXUI_Form* pClass = dynamic_cast<XUI::CXUI_Form*>(pItemWnd->GetControl(L"FRM_SLOT_MARK"));
						if( pClass )
						{
							XUI::CXUI_Image* const pClassMark = 
								dynamic_cast<XUI::CXUI_Image*>(pClass->GetControl(L"IMG_CLASS"));
							XUI::CXUI_Image* const pMasterMark = 
								dynamic_cast<XUI::CXUI_Image*>(pClass->GetControl(L"IMG_MASTER"));
							if( pClassMark && pMasterMark )
							{
								// 원정대장이라면
								if( (*iter)->kCharGuid == g_kExpedition.MasterGuid() )
								{ // 원정대장 고유마크를 보여준다
									pClassMark->Visible(false);
									pMasterMark->Visible(true);
								}
								else
								{ // 원정대장이 아니라면 직업마크를 보여준다.
									pClassMark->Visible(true);
									pMasterMark->Visible(false);
									lwSetMiniClassIconIndex(pClassMark, (*iter)->iClass);
								}
							}
						}
						
						XUI::CXUI_Form* pID = dynamic_cast<XUI::CXUI_Form*>(pItemWnd->GetControl(L"FRM_SLOT_ID"));
						if( pID )
						{
							std::wstring CharName = (*iter)->CharName;

							if( 10 < CharName.size() )
							{ // 원정대원 이름의 크기가 10 이상이면 짤라서 보여줌
								CharName = CharName.substr(0, 10) + _T("...");
							}
							pID->Text(CharName);		// 원정대원 아이디
						}
						
						XUI::CXUI_Form* pLevel = dynamic_cast<XUI::CXUI_Form*>(pItemWnd->GetControl(L"FRM_SLOT_LEVEL"));
						if( pLevel )
						{
							BM::vstring vStr((*iter)->sLevel);	// 원정대원의 레벨
							vStr += TTW(799359);				// "LV"을 뒤에 붙이고
							pLevel->Text(vStr);					// 결과를 윈도우에 출력
						}

						XUI::CXUI_Form* pKeyItem = dynamic_cast<XUI::CXUI_Form*>(pItemWnd->GetControl(L"FRM_SLOT_KEYITEM"));
						if( pKeyItem )
						{ // 원정대 메달을 가지고 있는지 체크해서 등록창에 표시한다.
							// 메달을 가지고 있지 않은 원정대 멤버의 정보를 가져오고
							ContUserGuid const& NotHaveItemMember = g_kExpedition.GetNotHaveKeyItemMember();
							//if( true == NotHaveItemMember.empty() )
							//{ // 컨테이너가 비었다면 이 체크는 할필요 없으므로 패스
							//	continue;
							//}
							pKeyItem->Visible(true);	// 우선 이미지를 다 보이게 한다.(메달 이미지)
							
							// 우선 메달이 있는 마크로 해둔 다음에.
							SUVInfo UVInfo = pKeyItem->UVInfo();
							UVInfo.Index = 1;
							pKeyItem->UVInfo(UVInfo);
							pKeyItem->SetInvalidate();

							ContUserGuid::const_iterator iter2;
							for( iter2 = NotHaveItemMember.begin(); iter2 != NotHaveItemMember.end(); ++iter2 )
							{ // 메달을 가지지 않은 원정대원과 현재 리스트의 주인이 같은지 비교를 해서
								if( (*iter)->kCharGuid == (*iter2) )	// 리스트의 주인이 키아이템이 없는 상태라면
								{ // 메달 마크를 X 마크로 바꾼다.
									SUVInfo UVInfo = pKeyItem->UVInfo();
									UVInfo.Index = 2;
									pKeyItem->UVInfo(UVInfo);
									pKeyItem->SetInvalidate();
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	void UpdateExpeditionMainBarInfo(lwUIWnd lwWnd, int const teamNum)
	{
		XUI::CXUI_Wnd* pWnd = (XUI::CXUI_Wnd*)lwWnd();
		if( NULL == pWnd )
		{
			return;
		}
		
		XUI::CXUI_List* const pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_MAIN_TEAM_LIST"));
		if( NULL == pList )
		{
			return;
		}

		// 멤버들의 팀 넘버 조사
		std::vector< SExpeditionMember* >::const_iterator iter;
		
		int MemberNum = 0;	// 팀의 인원수를 카운트할 변수
		XUI::SListItem* pItem = pList->FirstItem();
		for(iter = g_kExpedition.GetMembers().begin(); iter != g_kExpedition.GetMembers().end(); ++iter)
		{
			if( NULL == (*iter) )
			{
				continue;
			}
			if( (*iter)->sTeam == teamNum )
			{ // 보여주고 싶은 팀 번호와 멤버의 팀 번호가 일치할 때
				if( NULL == pItem )
				{ // 아이템이 하나도 없으면 새로운 항목을 등록하고
					pItem = pList->AddItem(L"");
				}
				if( pItem )
				{ // 있으면 갱신한다.
					XUI::CXUI_Wnd* pItemWnd = pItem->m_pWnd;
					if( pItemWnd )
					{
						pItemWnd->OwnerGuid((*iter)->kCharGuid);				// 리스트의 항목에 캐릭터 GUID를 등록한다.
						pItemWnd->SetCustomData(&MemberNum, sizeof(MemberNum));	// 그리고 0~3까지 고유 번호 등록(팀당 4명이므로)
						MemberNum++;	// 먼저 등록하고 증가시키는 이유는 해당되는 리스트가 0번부터 시작하기 때문에
						
						XUI::CXUI_Form* pID = dynamic_cast<XUI::CXUI_Form*>(pItemWnd->GetControl(L"FRM_USER_ID"));
						if( pID )
						{
							std::wstring CharName = (*iter)->CharName;

							if( 10 < CharName.size() )
							{ // 원정대원 이름의 크기가 10 이상이면 짤라서 보여줌
								CharName = CharName.substr(0, 10) + _T("...");
							}
							pID->Text(CharName);			// 원정대원 아이디
							if( false == (*iter)->bAlive )
							{ // 해당 유저가 죽은 상태라면 아이디를 붉은색으로 표시한다.
								pID->FontColor(0xFFFF0000);
							}
							else
							{ // 살아있으면 흰색으로 표시
								pID->FontColor(0xFFFFFFFF);
							}
						}

						XUI::CXUI_Form* pClass = dynamic_cast<XUI::CXUI_Form*>(pItemWnd->GetControl(L"FRM_SLOT_MARK"));
						if( pClass )
						{
							XUI::CXUI_Image* const pClassMark = dynamic_cast<XUI::CXUI_Image*>(pClass->GetControl(L"IMG_CLASS"));
							XUI::CXUI_Image* const pMasterMark = dynamic_cast<XUI::CXUI_Image*>(pClass->GetControl(L"IMG_MASTER"));
							if( pClassMark && pMasterMark )
							{
								// 원정대장이라면
								if( (*iter)->kCharGuid == g_kExpedition.MasterGuid() )
								{
									// 원정대장 고유마크를 보여준다
									pClassMark->Visible(false);
									pMasterMark->Visible(true);
								}
								else
								{
									// 원정대장이 아니라면 직업마크를 보여준다.
									pClassMark->Visible(true);
									pMasterMark->Visible(false);
									lwSetMiniClassIconIndex(pClassMark, (*iter)->iClass);
								}
							}
						}
					}
				}
				g_kExpedition.UpdateExpeditionAniBar((*iter)->kCharGuid);
				pItem = pList->NextItem(pItem);
			}
		}
		while( pItem )
		{ // 등록이 끝났는데 뒤에 아이템이 더 남아있다면 삭제
			pItem = pList->DeleteItem(pItem);
		}
		XUI::CXUI_Button* const pButtonUp = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_MAIN_ARW_UP"));
		XUI::CXUI_Button* const pButtonDw = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_MAIN_ARW_DOWN"));
		XUI::CXUI_Image* const pBarImg = dynamic_cast<XUI::CXUI_Image*>(pWnd->GetControl(L"IMG_MAIN_TEAM_BG"));
		if( NULL == pButtonUp || NULL == pButtonDw || NULL == pBarImg )
		{
			return;
		}
		pItem = pList->FirstItem();
		if( NULL == pItem )
		{
			return;
		}
		XUI::CXUI_Wnd* pItemWnd = pItem->m_pWnd;
		if( NULL == pItemWnd )
		{
			return;
		}
		if( pWnd->Visible() )
		{
			if( pButtonUp->Visible() )	// 업 버튼이 보여지고 있을 때 리스트 갯수만큼 창 크기 확장
			{
				const int ItemCount = pList->GetTotalItemCount();
				pWnd->Size(POINT2(pWnd->Size().x, (ItemCount * pItemWnd->Size().y) + pBarImg->Size().y));
			}
			if( pButtonDw->Visible() )	// 다운 버튼이 보여지고 있을 때 원래 창 크기로 축소
			{
				pWnd->Size(POINT2(pWnd->Size().x, pBarImg->Size().y));
			}
		}
	}

	void UpdateExpeditionUserList(lwUIWnd lwWnd)
	{
		XUI::CXUI_Wnd* pWnd = (XUI::CXUI_Wnd*)lwWnd();
		if( NULL == pWnd )
		{
			return;
		}
		
		XUI::CXUI_List* const pList =
			dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_EXPEDITION_INVITE_LIST"));
		if( NULL == pList )
		{
			return;
		}

		pList->ClearList();

		ContOtherPlayer::const_iterator iter;
		
		int PlayerNum = 0;	// 각 리스트 항목에 번호 부여를 위한 변수
		for(iter = g_kExpedition.GetOtherPlayers().begin();
			iter != g_kExpedition.GetOtherPlayers().end(); ++iter)
		{
			// 초대 리스트에 등록
			XUI::SListItem* pItem = pList->AddItem(L"");			// 새로운 항목 생성
			if( pItem )
			{
				if( EV_MAX_USER_VIEW_LIST_CNT <= PlayerNum )
				{ // 등록된 항목이 최대 갯수를 넘어가면 그 뒤로는 패스
					return;
				}

				XUI::CXUI_Wnd* pWnd = pItem->m_pWnd;
				if( pWnd )
				{
					pWnd->OwnerGuid(iter->first);						// 캐릭터 GUID를 창에 등록
					pWnd->SetCustomData(&PlayerNum, sizeof(PlayerNum));	// 고유번호 등록
					PlayerNum++;

					XUI::CXUI_Image* const pClassMark =
						dynamic_cast<XUI::CXUI_Image*>(pWnd->GetControl(L"IMG_USER_CLASS"));
					if( pClassMark )
					{
						lwSetMiniClassIconIndex(pClassMark, iter->second.iClass);
					}
					
					XUI::CXUI_Form* pID = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"FRM_USER_ID"));
					if( pID )
					{
						std::wstring CharName = iter->second.CharName;

						if( 10 < CharName.size() )
						{ // 원정대원 이름의 크기가 10 이상이면 짤라서 보여줌
							CharName = CharName.substr(0, 10) + _T("...");
						}
						pID->Text(CharName);		// 원정대원 아이디
					}

					XUI::CXUI_Form* pLevel = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"FRM_USER_LEVEL"));
					if( pLevel )
					{
						BM::vstring vStr(iter->second.sLevel);	// 원정대원의 레벨
						BM::vstring vStr2(TTW(799359));			// "LV"
						vStr += vStr2;							// 레벨"LV"이 되도록 합치고
						pLevel->Text(vStr);						// 결과를 윈도우에 출력
					}

					XUI::CXUI_Form* pKeyItem = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"FRM_SLOT_KEYITEM"));
					if( pKeyItem )
					{
						pKeyItem->Visible(true);	// 아이템 보유 마크를 일단 보여 주고.
						
						bool const bIsHaveKeyItem = iter->second.bHaveKeyItem;
						if( !bIsHaveKeyItem )	// 아이템을 가지고 있지 않으면 X표시로 바꿔준다.
						{
							SUVInfo UVInfo = pKeyItem->UVInfo();
							UVInfo.Index = 2;
							pKeyItem->UVInfo(UVInfo);
							pKeyItem->SetInvalidate();
						}
					}
				}
			}
		}
	}

	SExpeditionMember const* GetMember(std::wstring const& CharName)
	{
		ContExpeditionMember::const_iterator iter;

		for(iter = g_kExpedition.GetMembers().begin();
			iter != g_kExpedition.GetMembers().end(); ++iter)
		{
			if( NULL == (*iter) )
			{
				continue;
			}
			if( (*iter)->CharName == CharName )
			{
				return (*iter);
			}
		}
		return NULL;
	}

	SExpeditionMember const* GetMember(BM::GUID const& CharGuid)
	{
		ContExpeditionMember::const_iterator iter;

		for(iter = g_kExpedition.GetMembers().begin();
			iter != g_kExpedition.GetMembers().end(); ++iter)
		{
			if( NULL == (*iter) )
			{
				continue;
			}
			if( (*iter)->kCharGuid == CharGuid )
			{
				return (*iter);
			}
		}
		return NULL;
	}

	int GetMaxTeamNumber()
	{
		return g_kExpedition.GetMaxTeamNumber();	// 원정대의 최대 팀 갯수를 리턴한다. (1팀에서 4팀까지)
	}

	int GetMemberTeamNumber(lwGUID lwCharGuid)
	{
		return g_kExpedition.GetMemberTeamNumber(lwCharGuid());	// 해당 캐릭터의 원정대 팀번호를 알아낸다.
	}

	int GetExpeditionTeamMemberNum(int const teamNum)
	{
		std::vector< SExpeditionMember* >::const_iterator iter;
		int iMemberNum = 0;	// 팀의 인원수를 카운트할 변수
		for(iter = g_kExpedition.GetMembers().begin(); iter != g_kExpedition.GetMembers().end(); ++iter)
		{
			if( NULL == (*iter) )
			{
				continue;
			}
			if( (*iter)->sTeam == teamNum )	// 알고 싶은 팀 번호와 멤버의 팀 번호가 일치하면
			{
				++iMemberNum;	// 인원 수를 증가
			}
		}
		return iMemberNum;	// 해당 팀의 총 인원 수를 리턴
	}
	
	bool GetExpeditionJoinLevelCheck(BM::GUID const &CharGuid)
	{
		const PgPilot* pPilot = g_kPilotMan.FindPilot(CharGuid);
		if( NULL == pPilot )
		{
			return false;
		}

		CUnit* pUnit = pPilot->GetUnit();
		if( NULL == pUnit )
		{
			return false;
		}

		PgPlayer* pPlayer = dynamic_cast<PgPlayer*>(pUnit);
		if( NULL == pPlayer )
		{
			return false;
		}

		const SExpeditionOption& Option = g_kExpedition.Option();

		if( pPlayer->GetAbil(AT_LEVEL) < Option.GetOptionLevel() )
		{
			std::wstring const& Message = TTW(EMS_Join_Level_Fail);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
			return false;
		}

		return true;
	}

	int GetExpeditionOptionExp()
	{
		return g_kExpedition.Option().GetOptionExp();
	}
	int GetExpeditionOptionItem()
	{
		return g_kExpedition.Option().GetOptionItem();
	}
	int GetExpeditionOptionPublic()
	{
		return g_kExpedition.Option().GetOptionPublicTitle();
	}
	int GetExpeditionOptionLevel()
	{
		return g_kExpedition.Option().GetOptionLevel();
	}

	lwWString GetExpeditionOptionWStr()
	{
		std::wstring Temp;
		bool const bRet = ExpeditionOptionStr(g_kExpedition.Option(), Temp, false);
		return lwWString(Temp);
	}

	int GetClassNumber(lwGUID lwCharGuid)
	{ // GUID로 캐릭터를 찾아서 클래스 번호를 가져온다.
		BM::GUID const & CharGuid = lwCharGuid();
		PgPilot const * pPilot = g_kPilotMan.FindPilot(CharGuid);
		if( NULL == pPilot )
		{
			return 0;
		}

		PgPlayer const * pPlayer = dynamic_cast<PgPlayer*>(pPilot->GetUnit());
		if( NULL == pPlayer )
		{
			return 0;
		}

		int const iClassNo = pPlayer->GetAbil(AT_CLASS);
		return iClassNo;
	}

	lwWString ReplaceSystemMgs(int const TextNumber)
	{
		BM::vstring vStr(TTW(TextNumber));	// 텍스트 번호를 받아 내용에 접근해서 텍스트를 치환한다.
		vStr.Replace(L"#NAME#", g_kPilotMan.GetPlayerUnit()->Name());
		vStr.Replace(L"#LEVEL#", g_kPilotMan.GetPlayerUnit()->GetAbil(AT_LEVEL));
		vStr.Replace(L"#CLASS#", g_kPilotMan.GetPlayerUnit()->GetAbil(AT_CLASS));
		return lwWString(MB(vStr));
	}

	std::wstring GetClassNameTTW(int const ClassNo)
	{
		int const TTWClassNameBase = 30000;		// 클래스 번호로 클래스 이름을 알아낸다.
		return TTW(ClassNo + TTWClassNameBase);
	}

	bool IsInExpedition()
	{
		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return false;
		}

		bool const bIsInExpedition = !( pPlayer->ExpeditionGuid() == BM::GUID::NullData() );
		return bIsInExpedition;
	}

	bool IsInExpeditionMemberGuid(lwGUID lwCharGuid)
	{
		if( !IsInExpedition() )
		{
			return false;
		}
		BM::GUID const & CharGuid = lwCharGuid();
		return g_kExpedition.IsMember(CharGuid);
	}

	bool IsExpeditionMaster(lwGUID lwCharGuid)
	{
		if( false == IsInExpedition() )
		{
			return false; //원정대 없으므로 원정대장도 아님
		}
		BM::GUID const & CharGuid = lwCharGuid();
		bool const bIsExpeditionMaster = (g_kExpedition.MasterGuid() == CharGuid); //원정대장과 대상의 GUID가 같으면 OK
		return bIsExpeditionMaster;
	}

	void ExpeditionListNextPage()
	{ // 원정대 검색 리스트를 이전 페이지로
		int NextPage = g_kExpedition.ExpeditionListNowPage() + 1;
		g_kExpedition.ExpeditionListViewRefresh( NextPage );
	}
	
	void ExpeditionListPrevPage()
	{ // 원정대 검색 리스트를 다음 페이지로
		int PrevPage = g_kExpedition.ExpeditionListNowPage() - 1;
		g_kExpedition.ExpeditionListViewRefresh( PrevPage );
	}
	
	bool IsInExpeditionMember(BM::GUID const& rkCharGuid)
	{
		if( !IsInExpedition() )
		{
			return false;
		}
		return g_kExpedition.IsMember(rkCharGuid);
	}

	void SetExpeditionOption(int const OptionExp, int const OptionItem,
		int const OptionPublicTitle, int const Level)
	{ // SetExpeditionOption은 원정대 NPC GUID를 먼저 얻은 뒤에 가능하다.
	  // 원정대 GUID는 NPC에게 말을 거는 순간 얻게 되어있고(Lua에서 SetExpeditionNpcGuid 호출)
	  // NPC GUID가 있어야 원정대 정보를 찾아 입력하게 된다. (여기서는 원정대 맵번호)
		SExpeditionOption Option = g_kExpedition.Option();
		Option.SetOptionArea_NameNo(g_kExpedition.MapNo());
		Option.SetOptionExp((EExpeditionOptionExp)OptionExp);
		Option.SetOptionItem((EExpeditionOptionItem)OptionItem);
		Option.SetOptionPublicTitle((EExpeditionOptionPublicTitle)OptionPublicTitle);
		Option.SetOptionLevel(Level);
		
		CONT_DEFMAP const* pContMap = NULL;
		g_kTblDataMgr.GetContDef(pContMap);
		if ( pContMap )
		{ // 대륙 번호를 알아내기 위해 데이터베이스에서 맵 컨테이너를 가져온다.
			if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_LOBBY) )
			{ // 유저가 현재 위치한 원정대 로비의 맵번호로 대륙 번호를 찾는다.
				int const MapNo = g_pkWorld->MapNo();
				CONT_DEFMAP::const_iterator iter = pContMap->find(MapNo);
				if( iter != pContMap->end() )
				{ // 데이터가 존재하면 옵션에 넣는다.
					Option.SetOptionContinent(iter->second.sContinent);
				}
			}
		}

		g_kExpedition.Option(Option);
	}

	bool ExpeditionOptionStr(const SExpeditionOption& NewOption, std::wstring& Out, bool bChange)
	{ //파티 설정이 어쩌구 저쩌구
		int ItemTTW = 0;
		int PublicTitleTTW = 0;
		int Level = -1;
		bool bItem = false, bTitle = false, bLevel = false;

		SExpeditionOption ExpeditionOption = g_kExpedition.Option();

		if( ExpeditionOption.GetOptionItem() != NewOption.GetOptionItem() || !bChange )
		{
			if( NewOption.GetOptionItem() == EOI_LikeSolo )//Item
			{
				ItemTTW = 401008;
			}
			else if( NewOption.GetOptionItem() == EOI_Order )
			{
				ItemTTW = 401009;
			}
			bItem = true;
		}

		if( ExpeditionOption.GetOptionPublicTitle() != NewOption.GetOptionPublicTitle() || !bChange )
		{
			if( NewOption.GetOptionPublicTitle() == EOT_Public )//PublicTitle
			{
				PublicTitleTTW = 401204;
			}
			else
			{
				PublicTitleTTW = 401205;
			}
			bTitle = true;
		}

		if( ExpeditionOption.GetOptionLevel() != NewOption.GetOptionLevel() || !bChange )
		{
			Level = NewOption.GetOptionLevel();
			bLevel = true;
		}

		std::wstring wStr;

		int nPlyerLevel = 0;
		PgPilot * pMaster = g_kPilotMan.FindPilot(g_kExpedition.MasterGuid());
		if( pMaster )
		{
			nPlyerLevel = pMaster->GetAbil(AT_LEVEL);
		}
		if( 0 < Level )
		{
			wStr += TTW(401358);
			wStr += _T(" ");

			char temp[100];
			sprintf(temp, "%d", nPlyerLevel - Level);
			wStr += UNI(temp) + TTW(46) + _T(" ~ ");
			
			char temp2[100];
			sprintf(temp2, "%d", nPlyerLevel + Level);
			wStr += UNI(temp2) + TTW(46);
		}
		else if( 0 == Level )
		{
			wStr += TTW(401358);
			wStr += _T(" ");

			wStr += TTW(700552);
		}

		if( ItemTTW )
		{
			if( bLevel )
			{
				wStr += _T(", ");
			}
			wStr += TTW(ItemTTW);
		}

		if( PublicTitleTTW )
		{
			if( bLevel || bItem )
			{
				wStr += _T(", ");
			}
			wStr += TTW(PublicTitleTTW);
		}

		if( bChange )
		{
			if( bTitle || bLevel || bItem )
			{
				Out = TTW(720043) + wStr + TTW(401357);
			}	
		}
		else
		{
			Out = wStr;
		}
		
		return !wStr.empty();
	}

	void Send_C_N_REQ_CREATE_EXPEDITION()
	{
		if( IsInExpedition() )
		{ // 원정대에 속해 있으면 요청 불가
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}
		
		if( false == PgClientPartyUtil::IsPartyMaster(pPlayer->GetID()) )
		{ // 파티장이 아니면 요청 불가
			return;
		}

		BM::GUID const& NpcGuid = g_kExpedition.NpcGuid();
		if( BM::GUID::NullData() == NpcGuid )
		{ // 원정대 NPC GUID가 없으면 요청 불가
			return;
		}
		
		SExpeditionOption const& Option = g_kExpedition.Option();
		std::wstring const& ExpeditionName = g_kParty.PartyName();

		BM::Stream Packet(PT_C_N_REQ_CREATE_EXPEDITION);	// 원정대 생성 프로토콜
		Packet.Push(pPlayer->GetID());						// 원정대장 GUID
		Packet.Push(NpcGuid);								// 원정대 NPC GUID
		Option.WriteToPacket(Packet);						// 원정대 옵션
		Packet.Push(ExpeditionName);						// 원정대 이름
		NETWORK_SEND(Packet);
	}

	void Send_C_N_REQ_DISPERSE_EXPEDITION()
	{
		if( false == IsInExpedition() )	// 원정대에 속해 있지 않으면 요청 불가
		{
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}
		
		if( false == g_kExpedition.IsExpeditionMaster(pPlayer->GetID()) )	// 원정대장이 아니면 요청 불가
		{
			return;
		}

		BM::Stream Packet(PT_C_N_REQ_DISPERSE_EXPEDITION);
		Packet.Push(pPlayer->GetID());
		NETWORK_SEND(Packet);
	}

	void Send_C_N_REQ_INVITE_EXPEDITION(lwUIWnd lwWnd)	// 원정대 초대하기
	{
		if( false == IsInExpedition() )
		{ // 원정대에 속해 있지 않으면 요청 불가
			return;
		}
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}
		if( false == g_kExpedition.IsExpeditionMaster(pPlayer->GetID()) )
		{ // 원정대장이 아니면 요청 불가
			return;
		}
		XUI::CXUI_Wnd* pWnd = (XUI::CXUI_Wnd*)lwWnd();
		if( NULL == pWnd )
		{
			return;
		}
		XUI::CXUI_List* const pList =
			dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_EXPEDITION_INVITE_LIST"));
		if( NULL == pList )
		{
			return;
		}
		int ItemCount = pList->GetTotalItemCount();
		if( ItemCount == 0 )
		{ // 아이템이 하나도 없으면
			return;
		}

		VEC_GUID UserGuidVector;				// 초대할 유저들의 GUID를 담을 컨테이너
		for( int i = 0; i < ItemCount; ++i )	// 리스트 아이템 갯수만큼 반복한다.
		{
			XUI::SListItem* pItem = pList->GetItemAt(i);	// 순서대로 아이템을 가져온다.
			if( pItem )
			{
				XUI::CXUI_Wnd* pWnd = pItem->m_pWnd;
				if( pWnd )
				{
					XUI::CXUI_CheckButton* const pChkButton =
						dynamic_cast<XUI::CXUI_CheckButton*>(pWnd->GetControl(L"CHK_INVITESTATE"));
					if( pChkButton )
					{
						if( pChkButton->Check() )
						{ // 체크버튼이 ON 상태라면 초대하려는 유저이므로
							BM::GUID const& UserGuid = pWnd->OwnerGuid();		// 아이템에 등록된 유저의 GUID를
							if( false == g_kExpedition.CheckLastInviteTime(UserGuid, (float)EXPEDITION_JOIN_REQ_WAIT_TIME) )
							{ // 이미 초대를 한 상태이거나 거절한 유저인지 체크
								continue;
							}
							else
							{ // 거절한 유저가 아니면 패킷으로 보낼 컨테이너에 저장한다.
								g_kExpedition.AddCheckInviteTime(UserGuid);
								UserGuidVector.push_back(UserGuid);				
							}
						}
					}
				}
			}
		}
		if( UserGuidVector.empty() )
		{ // 초대할 유저가 하나도 없으면
			return;
		}
		BM::GUID const& ExpeditionGuid = g_kExpedition.ExpeditionGuid();
		BM::Stream Packet(PT_C_N_REQ_INVITE_EXPEDITION);
		Packet.Push(ExpeditionGuid);	// 원정대 GUID
		Packet.Push(UserGuidVector);	// 유저 GUID들
		NETWORK_SEND(Packet);
	}

	void Send_C_N_REQ_INVITE_EXPEDITION_BYGUID(lwGUID lwCharGuid) // 원정대 초대하기
	{
		if ( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_FLAG_MISSION) )
		{ // 미션 중(?)
			return;
		}

		if( false == PgClientPartyUtil::IsCanPartyArea(true) )
		{ // 파티가 가능한 지역인가?
			return;
		}

		if( BM::GUID::NullData() == g_kExpedition.ExpeditionGuid() )
		{ // 원정대인가?
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( false == g_kExpedition.IsExpeditionMaster(pPlayer->GetID()) )
		{ // 원정대 초대는 원정대장만 가능
			return;
		}

		BM::GUID const& CharGuid = lwCharGuid();
		if( g_kExpedition.IsMember(CharGuid))	// 초대 대상이 이미 내 원정대 멤버일 경우
		{
			std::wstring const& Message = TTW(EMS_SameExpedition);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
			return;
		}
		
		BM::Stream Packet(PT_C_N_REQ_INVITE_EXPEDITION);
		Packet.Push(CharGuid);
		NETWORK_SEND(Packet);

		std::wstring Message;
		PgPilot const* pPilot = g_kPilotMan.FindPilot(CharGuid);
		if( pPilot )
		{
			std::wstring const& CharName = pPilot->GetName();
			Message = CharName + TTW(EMS_Is2) + TTW(EMS_Invite); //누구 님을 초대
		}
		else
		{
			Message = TTW(EMS_Invite); //초대
		}	

		SChatLog ChatLog(CT_EVENT_SYSTEM);
		g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
	}
	
	void Send_C_N_ANS_INVITE_EXPEDITION(bool bAccept, BM::GUID const& ExpeditionGUID)
	{
		if ( g_pkWorld && (g_pkWorld->IsHaveAttr(GATTR_FLAG_NOPARTY) || g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND)) )
		{
			return;
		}

		PgPlayer* pMyPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pMyPlayer )
		{
			return;
		}

		BM::Stream Packet(PT_C_N_ANS_INVITE_EXPEDITION);
		Packet.Push(ExpeditionGUID);		// 가입할 원정대 GUID
		Packet.Push(pMyPlayer->GetID());	// 자신의 GUID
		Packet.Push(bAccept);				// 수락 여부
		NETWORK_SEND(Packet);
	}

	void Send_C_N_ANS_JOIN_EXPEDITION(bool bAccept, BM::GUID const& UserGUID)
	{
		if ( g_pkWorld && (g_pkWorld->IsHaveAttr(GATTR_FLAG_NOPARTY) || g_pkWorld->IsHaveAttr(GATTR_EVENT_GROUND)) )
		{
			return;
		}

		PgPlayer* pMyPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pMyPlayer )
		{
			return;
		}

		BM::Stream Packet(PT_C_N_ANS_JOIN_EXPEDITION);
		Packet.Push(pMyPlayer->ExpeditionGuid());	// 자신의 원정대 GUID
		Packet.Push(UserGUID);						// 가입 신청한 유저의 GUID
		Packet.Push(bAccept);						// 수락 여부
		NETWORK_SEND(Packet)
	}

	void Send_C_N_REQ_JOIN_EXPEDITION(lwGUID lwExpeditionGuid)
	{
		if( false == PgClientPartyUtil::IsCanPartyArea(true) )
		{ //파티가 가능한 지역인가?
			lwAddWarnDataTT(720015);
			return;
		}
		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}
		if( BM::GUID::NullData() != pPlayer->PartyGuid() )
		{ // 파티에 속해있으면 신청 불가
			std::wstring const& Message = TTW(720052);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
			return;
		}
		if( BM::GUID::NullData() != pPlayer->ExpeditionGuid() )
		{ // 원정대에 속해있으면 신청 불가
			std::wstring const& Message = TTW(720001);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
			return;
		}
		BM::GUID const& ExpeditionGuid = lwExpeditionGuid();
		if( BM::GUID::NullData() == ExpeditionGuid )
		{
			return;
		}

		BM::GUID const & MasterGuid = g_kExpedition.GetRejectedMaster(ExpeditionGuid);
		if( BM::GUID::NullData() != MasterGuid )
		{
			if( false == g_kExpedition.CheckLastInviteTime(MasterGuid, (float)EXPEDITION_JOIN_REQ_WAIT_TIME) )
			{ // 먼저 신청을 거절한 원정대라면 바로 신청 불가
				std::wstring const& Message = TTW(720039);
				SChatLog ChatLog(CT_EVENT);
				g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
				return;
			}
		}
		if( false == g_kExpedition.CheckLastJoinTime(ExpeditionGuid, (float)EXPEDITION_JOIN_REQ_WAIT_TIME) )
		{ // 신청 대기중인 원정대인지 검사
			std::wstring const& Message = TTW(EMS_AnswerWait);
			SChatLog ChatLog(CT_EVENT_SYSTEM);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
			return;
		}
		g_kExpedition.AddCheckJoinTime(ExpeditionGuid);	// 신청 대기중이 아니면 중복 신청 방지를 위해 등록시킴

		std::wstring const& InviteSuccessMsg = TTW(EMS_ExpeditionInvite_Success);
		SChatLog ChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(ChatLog, InviteSuccessMsg, true);

		BM::GUID const& MyGuid = pPlayer->GetID();
		BM::Stream Packet(PT_C_N_REQ_JOIN_EXPEDITION);
		Packet.Push(ExpeditionGuid);		// 대상 원정대 GUID
		Packet.Push(MyGuid);				// 가입요청자 GUID
		NETWORK_SEND(Packet);
	}

	void Send_C_N_REQ_LEAVE_EXPEDITION()
	{
		if( false == IsInExpedition() )
		{ // 원정대에 속해있지 않으면 요청 불가
			return;
		}
		
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}
		
		BM::GUID const& MyGuid = pPlayer->GetID();
		BM::GUID const& ExpeditionGuid = g_kExpedition.ExpeditionGuid();
		BM::Stream Packet(PT_C_N_REQ_LEAVE_EXPEDITION);
		Packet.Push(ExpeditionGuid);
		Packet.Push(MyGuid);
		NETWORK_SEND(Packet);
	}

	void Send_C_N_REQ_CHANGEMASTER_EXPEDITION(lwGUID lwNewMasterGuid)
	{
		if( false == IsInExpedition() )
		{ // 원정대에 속해있지 않으면 요청 불가
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}
		
		if( false == g_kExpedition.IsExpeditionMaster(pPlayer->GetID()) )
		{ // 원정대장이 아니면 요청 불가
			return;
		}
		
		BM::GUID const& NewMasterGuid = lwNewMasterGuid();
		BM::GUID const& ExpeditionGuid = g_kExpedition.ExpeditionGuid();
		BM::Stream Packet(PT_C_N_REQ_CHANGEMASTER_EXPEDITION);
		Packet.Push(ExpeditionGuid);
		Packet.Push(NewMasterGuid);
		NETWORK_SEND(Packet);
	}

	void Send_C_N_REQ_LIST_EXPEDITION(int const iContinent)
	{
		g_kExpedition.NowPage(1);
		BM::Stream Packet(PT_C_N_REQ_LIST_EXPEDITION);
		Packet.Push(iContinent);
		NETWORK_SEND(Packet);
	}

	void Send_C_N_REQ_RENAME_EXPEDITION(lwWString lwNewName)
	{
		if( false == IsInExpedition() )
		{ // 원정대가 아니면 요청 불가
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( false == g_kExpedition.IsExpeditionMaster(pPlayer->GetID()) )
		{ // 원정대장이 아니면 요청 불가
			return;
		}

		std::wstring const& NewExpeditionName = lwNewName();
		if( NewExpeditionName.empty() )
		{
			return;
		}
		else
		{
			if( MAX_PARTYNAME_LEN < NewExpeditionName.size() )
			{
				return;
			}
		}

		if( g_kExpedition.ExpeditionName() == NewExpeditionName )
		{ // 변경할 이름과 현재 이름이 같으면 패스
			return;
		}

		BM::GUID const& ExpeditionGuid = g_kExpedition.ExpeditionGuid();
		BM::Stream Packet(PT_C_N_REQ_RENAME_EXPEDITION);
		Packet.Push(ExpeditionGuid);		// 원정대 GUID
		Packet.Push(NewExpeditionName);		// 변경할 원정대 이름
		NETWORK_SEND(Packet);
	}
	
	void Send_C_N_REQ_CHANGEOPTION_EXPEDITION(int const OptionExp,
		int const OptionItem, int const OptionPublicTitle, int const Level)
	{
		if( false == IsInExpedition() )
		{ // 원정대가 아니면 요청 불가
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( false == g_kExpedition.IsExpeditionMaster(pPlayer->GetID()) )
		{ // 원정대장이 아니면 요청 불가
			return;
		}

		SExpeditionOption Option = g_kExpedition.Option();
		Option.SetOptionExp((EExpeditionOptionExp)OptionExp);
		Option.SetOptionItem((EExpeditionOptionItem)OptionItem);
		Option.SetOptionPublicTitle((EExpeditionOptionPublicTitle)OptionPublicTitle);
		Option.SetOptionLevel(Level);

		BM::GUID const& ExpeditionGuid = g_kExpedition.ExpeditionGuid();
		BM::Stream Packet(PT_C_N_REQ_CHANGEOPTION_EXPEDITION);
		Packet.Push(ExpeditionGuid);	// 원정대 GUID
		Option.WriteToPacket(Packet);	// 변경할 원정대 옵션
		NETWORK_SEND(Packet);
	}

	void Send_C_N_REQ_KICKOUT_EXPEDITION(lwGUID lwCharGuid)
	{
		if( false == IsInExpedition() )	// 원정대가 아니면 요청 불가
		{
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( false == g_kExpedition.IsExpeditionMaster(pPlayer->GetID()) )	// 원정대장이 아니면 요청 불가
		{
			return;
		}

		BM::GUID const& CharGuid = lwCharGuid();
		BM::GUID const& ExpeditionGuid = g_kExpedition.ExpeditionGuid();
		BM::Stream Packet(PT_C_N_REQ_KICKOUT_EXPEDITION);
		Packet.Push(ExpeditionGuid);
		Packet.Push(CharGuid);
		NETWORK_SEND(Packet);
	}

	void Send_C_N_REQ_TEAM_MOVE_EXPEDITION(lwGUID lwCharGuid, int const TeamNum)
	{
		if( false == IsInExpedition() )	// 원정대가 아니면 요청 불가
		{
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( false == g_kExpedition.IsExpeditionMaster(pPlayer->GetID()) )	// 원정대장이 아니면 요청 불가
		{
			return;
		}

		int TeamCount = 0;
		ContExpeditionMember const & MyMembers = g_kExpedition.GetMembers();
		ContExpeditionMember::const_iterator iter = MyMembers.begin();
		while( iter != MyMembers.end() )
		{ // 내 원정대 멤버들을 순회하면서 이동하려는 팀의 인원이 몇명인지 센다.
			if( NULL == (*iter) )
			{
				break;
			}
			if( TeamNum == (*iter)->sTeam )
			{
				TeamCount++;
			}
			++iter;
		}
		if( TeamCount >= 4 )
		{ // 이동하려는 팀에 4명이 있으면 배정할 수 없다.
			std::wstring const& Message = TTW(EMS_MoveTeam_Fail);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
			return;
		}

		BM::GUID const& CharGuid = lwCharGuid();
		BM::GUID const& ExpeditionGuid = g_kExpedition.ExpeditionGuid();
		BM::Stream Packet(PT_C_N_REQ_TEAM_MOVE_EXPEDITION);
		Packet.Push(ExpeditionGuid);
		Packet.Push(CharGuid);
		Packet.Push(TeamNum);
		NETWORK_SEND(Packet);
	}
	
	void Send_C_M_REQ_LIST_USER_EXPEDITION()
	{
		if( false == IsInExpedition() )	// 원정대가 아니면 요청 불가
		{
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( false == g_kExpedition.IsExpeditionMaster(pPlayer->GetID()) )	// 원정대장이 아니면 요청 불가
		{
			return;
		}

		BM::Stream Packet(PT_C_M_REQ_LIST_USER_EXPEDITION);
		NETWORK_SEND(Packet);
	}

	void Send_C_M_REQ_NPC_ENTER_EXPEDITION()
	{
		if( false == IsInExpedition() )	// 원정대가 아니면 요청 불가
		{
			return;
		}

		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( false == g_kExpedition.IsExpeditionMaster(pPlayer->GetID()) )	// 원정대장이 아니면 요청 불가
		{
			return;
		}

		bool const EnableEnter = CheckEnableEnter();
		if( false == EnableEnter )
		{
			return;
		}

		BM::vstring Message( TTW(EMS_Confirm_Enter) );
		CallYesNoMsgBox(Message, g_kExpedition.NpcGuid(), MBT_EXPEDITION_CONFIRM_ENTER);
	}

	void Send_C_M_REQ_REFRESH_NEED_ITEM_INFO()
	{
		BM::Stream Packet(PT_C_M_REQ_REFRESH_NEED_ITEM_INFO);
		NETWORK_SEND(Packet);
	}
}

PgClientExpedition::PgClientExpedition()
	:m_MemberPool(16)
{
	Clear();
}

PgClientExpedition::~PgClientExpedition()
{
	Clear();
	ClearExpeditionInfo();
}

bool PgClientExpedition::RegisterWrapper(lua_State* pState)
{
	if( !pState )
	{
		return false;
	}

	using namespace lua_tinker;

	def(pState, "SetExpeditionNpcGuid", &PgClientExpeditionUtil::SetExpeditionNpcGuid);
	def(pState, "GetExpeditionNpcGuid", &PgClientExpeditionUtil::GetExpeditionNpcGuid);
	def(pState, "CheckEnableEnter", &PgClientExpeditionUtil::CheckEnableEnter);
	def(pState, "CheckExpeditionDailyQuest", &PgClientExpeditionUtil::CheckExpeditionDailyQuest);
	def(pState, "ClearNotHaveKeyItemMemberList", &PgClientExpeditionUtil::ClearNotHaveKeyItemMemberList);
	def(pState, "ShowExpeditionTitleText", &PgClientExpeditionUtil::ShowExpeditionTitleText);
	def(pState, "ShowExpeditionInfoText", &PgClientExpeditionUtil::ShowExpeditionInfoText);
	def(pState, "ShowExpeditionName", &PgClientExpeditionUtil::ShowExpeditionName);
	def(pState, "ShowExpeditionArea", &PgClientExpeditionUtil::ShowExpeditionArea);
	def(pState, "ShowExpeditionCurrMemNum", &PgClientExpeditionUtil::ShowExpeditionCurrMemNum);
	def(pState, "ShowExpeditionImage", &PgClientExpeditionUtil::ShowExpeditionImage);
	def(pState, "UpdateExpeditionMemberInfo", &PgClientExpeditionUtil::UpdateExpeditionMemberInfo);
	def(pState, "UpdateExpeditionMainBarInfo", &PgClientExpeditionUtil::UpdateExpeditionMainBarInfo);
	def(pState, "UpdateExpeditionUserList", &PgClientExpeditionUtil::UpdateExpeditionUserList);
	def(pState, "GetMaxTeamNumber", &PgClientExpeditionUtil::GetMaxTeamNumber);
	def(pState, "GetMemberTeamNumber", &PgClientExpeditionUtil::GetMemberTeamNumber);
	def(pState, "GetExpeditionTeamMemberNum", &PgClientExpeditionUtil::GetExpeditionTeamMemberNum);
	def(pState, "GetExpeditionOptionExp", &PgClientExpeditionUtil::GetExpeditionOptionExp);
	def(pState, "GetExpeditionOptionItem", &PgClientExpeditionUtil::GetExpeditionOptionItem);
	def(pState, "GetExpeditionOptionPublic", &PgClientExpeditionUtil::GetExpeditionOptionPublic);
	def(pState, "GetExpeditionOptionLevel", &PgClientExpeditionUtil::GetExpeditionOptionLevel);
	def(pState, "GetExpeditionOptionWStr", &PgClientExpeditionUtil::GetExpeditionOptionWStr);
	def(pState, "GetClassNumber", &PgClientExpeditionUtil::GetClassNumber);
	def(pState, "ReplaceSystemMgs", &PgClientExpeditionUtil::ReplaceSystemMgs);
	def(pState, "IsInExpedition", &PgClientExpeditionUtil::IsInExpedition);
	def(pState, "IsInExpeditionMemberGuid", &PgClientExpeditionUtil::IsInExpeditionMemberGuid);
	def(pState, "IsExpeditionMaster", &PgClientExpeditionUtil::IsExpeditionMaster);
	def(pState, "ExpeditionListNextPage", &PgClientExpeditionUtil::ExpeditionListNextPage);
	def(pState, "ExpeditionListPrevPage", &PgClientExpeditionUtil::ExpeditionListPrevPage);
	def(pState, "IsInExpeditionMember", &PgClientExpeditionUtil::IsInExpeditionMember);
	def(pState, "SetExpeditionOption", &PgClientExpeditionUtil::SetExpeditionOption);
	def(pState, "Send_C_N_REQ_CREATE_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_CREATE_EXPEDITION);
	def(pState, "Send_C_N_REQ_DISPERSE_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_DISPERSE_EXPEDITION);
	def(pState, "Send_C_N_REQ_JOIN_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_JOIN_EXPEDITION);
	def(pState, "Send_C_N_REQ_INVITE_EXPEDITION_BYGUID", &PgClientExpeditionUtil::Send_C_N_REQ_INVITE_EXPEDITION_BYGUID);
	def(pState, "Send_C_N_REQ_LEAVE_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_LEAVE_EXPEDITION);
	def(pState, "Send_C_N_REQ_CHANGEMASTER_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_CHANGEMASTER_EXPEDITION);
	def(pState, "Send_C_N_REQ_LIST_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_LIST_EXPEDITION);
	def(pState, "Send_C_N_REQ_RENAME_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_RENAME_EXPEDITION);
	def(pState, "Send_C_N_REQ_CHANGEOPTION_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_CHANGEOPTION_EXPEDITION);
	def(pState, "Send_C_N_REQ_KICKOUT_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_KICKOUT_EXPEDITION);
	def(pState, "Send_C_N_REQ_TEAM_MOVE_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_TEAM_MOVE_EXPEDITION);
	def(pState, "Send_C_M_REQ_LIST_USER_EXPEDITION", &PgClientExpeditionUtil::Send_C_M_REQ_LIST_USER_EXPEDITION);
	def(pState, "Send_C_N_REQ_INVITE_EXPEDITION", &PgClientExpeditionUtil::Send_C_N_REQ_INVITE_EXPEDITION);
	def(pState, "Send_C_M_REQ_NPC_ENTER_EXPEDITION", &PgClientExpeditionUtil::Send_C_M_REQ_NPC_ENTER_EXPEDITION);
	def(pState, "Send_C_M_REQ_REFRESH_NEED_ITEM_INFO", &PgClientExpeditionUtil::Send_C_M_REQ_REFRESH_NEED_ITEM_INFO);
	return true;
}

//	신규 멤버를 추가
void PgClientExpedition::AddMember(SExpeditionUserInfo const& MemberInfo)
{
	BM::CAutoMutex kLock(m_Mutex);
	SExpeditionMember *pMember = GetMember(MemberInfo.CharName);
	if( !pMember )
	{
		pMember = m_MemberPool.New();
		if( pMember )
		{
			*pMember = MemberInfo;
			if( pMember->kCharGuid == MasterGuid())	// 원정대장이면 맨 앞에 삽입
			{
				m_Members.insert(m_Members.begin(), pMember);
			}
			else
			{
				m_Members.push_back(pMember);
			}
		}
	}

	PgClientExpeditionUtil::SetCharacterExpeditionGuid(MemberInfo.kCharGuid, ExpeditionGuid());
}

bool const PgClientExpedition::IsMember(std::wstring const& CharName)const
{
	BM::CAutoMutex kLock(m_Mutex);
	return NULL != GetMember(CharName);
}

bool const PgClientExpedition::IsMember(BM::GUID const& CharGuid)const
{
	BM::CAutoMutex kLock(m_Mutex);
	return NULL != GetMember(CharGuid);
}

bool const PgClientExpedition::IsExpeditionMaster(std::wstring const& CharName) const
{
	BM::CAutoMutex kLock(m_Mutex);
	SExpeditionMember * pMember = GetMember(MasterGuid());	// 원정대에서 원정대장의 정보를 가져옴
	if( NULL == pMember )
	{
		return false;
	}
	
	bool const bIsExpeditionMaster = (pMember->CharName == CharName);	// 원정대장의 이름과 같으면 OK
	return bIsExpeditionMaster;
}

bool const PgClientExpedition::IsExpeditionMaster(BM::GUID const& CharGuid) const
{
	BM::CAutoMutex kLock(m_Mutex);
	bool const bIsExpeditionMaster = (MasterGuid() == CharGuid); //원정대장과 대상의 GUID가 같으면 OK
	return bIsExpeditionMaster;
}

// 원정대 모든 정보를 초기화
void PgClientExpedition::Clear()
{
	BM::CAutoMutex kLock(m_Mutex);

	ContExpeditionMember::iterator iter;
	for(iter = m_Members.begin(); iter != m_Members.end(); iter++)
	{
		SExpeditionMember* pMember = *iter;
		if( pMember )
		{
			if( g_kPilotMan.IsMyPlayer(pMember->kCharGuid) )
			{
				PgClientExpeditionUtil::SetCharacterExpeditionGuid(pMember->kCharGuid, BM::GUID::NullData());
			}
			m_MemberPool.Delete(pMember);
		}
	}

	m_Members.clear();
	m_Waiter.Clear();
	m_RejectedUser.clear();
	m_WaitExpedition.clear();
	m_NotHaveItemMember.clear();

	m_kExpeditionName.clear();
	m_kExpeditionGuid.Clear();
	m_kMasterGuid.Clear();
	m_kOption.Clear();
}

void PgClientExpedition::AddExpedition(SExpeditionInfo const& Expedition)
{	
	BM::CAutoMutex kLock(m_Mutex);

	BM::GUID OldMasterGuid;

	std::pair<ContOtherExpedition::iterator, bool> Ret =
		m_OtherExpedition.insert( std::make_pair(Expedition.ExpeditionGuid, Expedition) );
	if( !Ret.second ) // 이미 있음
	{
		ContOtherExpedition::iterator iter = Ret.first;
		SExpeditionInfo& FindExpedition = (*iter).second;

		if( false == Expedition.ExpeditionName.empty()		// 추가할 원정대가 이름을 가지고 있고
		&&	false == FindExpedition.ExpeditionName.empty()	// 이미 존재하는 원정대 또한 이름을 가지고 있는데
		&&	FindExpedition.ExpeditionName != Expedition.ExpeditionName	// 서로의 이름이 다르고
		&&	Expedition.ExpeditionGuid == ExpeditionGuid() )	// 추가할 원정대가 내 원정대라면
		{ // 원정대 이름이 바뀐다.
			std::wstring const& Message = TTW(EMS_ChangeName_Success);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
		}

		OldMasterGuid = FindExpedition.MasterGuid;	// 기존의 마스터GUID를 저장하고
		FindExpedition.SetOnlyInfo(Expedition);	// 덮어 씌움
	}
	else	// 원정대 목록에 없음
	{
		if( false == Expedition.ExpeditionName.empty()		// 추가할 원정대가 이름을 가지고 있고
		&&	ExpeditionName() != Expedition.ExpeditionName	// 추가할 원정대의 이름이 내 원정대의 이름과 다른데
		&& Expedition.ExpeditionGuid == ExpeditionGuid() )	// 추가할 원정대가 내 원정대라면
		{
			std::wstring const& Message = TTW(EMS_ChangeName_Success);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
		}
		OldMasterGuid = Expedition.MasterGuid;
	}

	if( Expedition.ExpeditionGuid == m_kExpeditionGuid )	// 추가할 원정대가 내 원정대이면 정보 갱신
	{
		m_kExpeditionName	= Expedition.ExpeditionName;
		m_kMasterGuid		= Expedition.MasterGuid;
	}

	if( OldMasterGuid != Expedition.MasterGuid )
	{
		UpdateExpeditionBalloon(OldMasterGuid);
	}
	UpdateExpeditionBalloon(Expedition.MasterGuid);

	ContInfoWaiter::ContWaiter Vec;
	bool const bRet = m_Waiter.GetWaiter(Expedition.ExpeditionGuid, Vec);
	if( !bRet )
	{
		return;
	}
	m_Waiter.DelWaiter(Expedition.ExpeditionGuid);

	ContInfoWaiter::ContWaiter::iterator wait_iter = Vec.begin();
	while(Vec.end() != wait_iter)
	{
		ContInfoWaiter::ContWaiter::value_type const& CharGuid = (*wait_iter);
		UpdateExpeditionBalloon(CharGuid);
		++wait_iter;
	}
}

void PgClientExpedition::RemoveMember(BM::GUID const& CharGuid, std::wstring* pOutName)
{
	BM::CAutoMutex kLock(m_Mutex);
	PgClientExpeditionUtil::SetCharacterExpeditionGuid(CharGuid, BM::GUID::NullData());
	g_kUIScene.RemoveMiniMapIcon(CharGuid);

	ContExpeditionMember::iterator iter = m_Members.begin();
	while(m_Members.end() != iter)
	{
		ContExpeditionMember::value_type Element = (*iter);
		if( NULL == Element )
		{
			break;
		}
		if( Element->kCharGuid == CharGuid )
		{
			if ( pOutName )
			{
				*pOutName = Element->CharName;
			}

			m_MemberPool.Delete(Element);
			iter = m_Members.erase(iter);
			break;
		}
		else
		{
			++iter;
		}
	}
}

bool PgClientExpedition::GetExpedition(BM::GUID const& ExpeditionGuid, SExpeditionInfo& Out) const
{
	BM::CAutoMutex kLock(m_Mutex);
	ContOtherExpedition::const_iterator iter = m_OtherExpedition.find(ExpeditionGuid);
	if( m_OtherExpedition.end() != iter )
	{
		Out = (*iter).second;
		return true;
	}
	return false;
}

bool PgClientExpedition::GetExpedition(BM::GUID const& ExpeditionGuid,
									   BM::GUID const& CharGuid, SExpeditionInfo& Out)
{
	BM::CAutoMutex kLock(m_Mutex);
	ContOtherExpedition::iterator iter = m_OtherExpedition.find(ExpeditionGuid);
	if( m_OtherExpedition.end() != iter )
	{
		ContGuidSet& Set = (*iter).second.GuidSet;
		if( Set.end() == Set.find(CharGuid) )
		{
			Set.insert(CharGuid);
		}

		Out = (*iter).second;
		return true;
	}
	return false;
}

int const PgClientExpedition::GetNumExpeditionMember() const
{
	BM::CAutoMutex kLock(m_Mutex);
	return m_Members.size();
}

ContExpeditionMember const& PgClientExpedition::GetMembers() const
{
	BM::CAutoMutex kLock(m_Mutex);
	return m_Members;
}

ContOtherPlayer const& PgClientExpedition::GetOtherPlayers() const
{
	BM::CAutoMutex kLock(m_Mutex);
	return m_OtherPlayers;
}

ContOtherExpedition const& PgClientExpedition::GetOtherExpedition() const
{
	BM::CAutoMutex kLock(m_Mutex);
	return m_OtherExpedition;
}

ContUserGuid const& PgClientExpedition::GetNotHaveKeyItemMember() const
{
	BM::CAutoMutex kLock(m_Mutex);
	return m_NotHaveItemMember;
}

SExpeditionMember* PgClientExpedition::GetMember(std::wstring const& CharName) const
{
	BM::CAutoMutex kLock(m_Mutex);
	
	ContExpeditionMember::const_iterator iter;
	for(iter = m_Members.begin(); iter != m_Members.end(); iter++)
	{
		const ContExpeditionMember::value_type pElement = (*iter);
		if( pElement
		&&	pElement->CharName == CharName )
		{
			return pElement;
		}
	}
	return 0;
}

SExpeditionMember* PgClientExpedition::GetMember(BM::GUID const& CharGuid) const
{
	BM::CAutoMutex kLock(m_Mutex);
	
	ContExpeditionMember::const_iterator iter;
	for(iter = m_Members.begin(); iter != m_Members.end(); ++iter)
	{
		const ContExpeditionMember::value_type pElement = (*iter);
		if( pElement
		&&	pElement->kCharGuid == CharGuid)
		{
			return (*iter);
		}
	}
	return 0;
}

int const PgClientExpedition::GetMaxTeamNumber() const
{
	BM::CAutoMutex kLock(m_Mutex);
	return (m_kMaxExpeditionMember / DEF_PV_MAX_MEMBER_CNT);
}

int const PgClientExpedition::GetMemberTeamNumber(BM::GUID const& CharGuid) const
{
	BM::CAutoMutex kLock(m_Mutex);
	ContExpeditionMember::const_iterator iter = m_Members.begin();
	for( iter = m_Members.begin(); iter != m_Members.end(); iter++ )
	{
		if( NULL == (*iter) )
		{
			continue;
		}
		if( (*iter)->kCharGuid == CharGuid )
		{
			return (*iter)->sTeam;
		}
	}
	return 0;
}

void PgClientExpedition::RemoveExpedition(BM::GUID const& ExpeditionGuid, BM::GUID const& CharGuid)
{
	BM::CAutoMutex kLock(m_Mutex);
	ContOtherExpedition::iterator iter = m_OtherExpedition.find(ExpeditionGuid);
	if( m_OtherExpedition.end() != iter )
	{
		ContGuidSet& Set = (*iter).second.GuidSet;
		ContGuidSet::iterator find_iter = Set.find(CharGuid);
		if( Set.end() != find_iter )
		{
			Set.erase(find_iter);
		}

		if( Set.empty() )
		{
			m_OtherExpedition.erase(iter);
		}
	}
}

void PgClientExpedition::ReqOtherExpeditionInfo(BM::GUID const& ExpeditionGuid, BM::GUID const& CharGuid)
{
	BM::CAutoMutex kLock(m_Mutex);
	if( GUID_NULL == ExpeditionGuid
	||	GUID_NULL == CharGuid )
	{
		return;
	}

	ContOtherExpedition::iterator iter = m_OtherExpedition.find(ExpeditionGuid);
	if( m_OtherExpedition.end() != iter )
	{
		return;
	}

	bool const bAddRet = m_Waiter.AddWaiter(ExpeditionGuid, CharGuid);
	if( bAddRet )
	{
		BM::Stream Packet(PT_C_N_REQ_INFO_EXPEDITION);
		Packet.Push(ExpeditionGuid);
		NETWORK_SEND(Packet);
	}
}

void PgClientExpedition::AddSelfExpedition()
{
	BM::CAutoMutex kLock(m_Mutex);
	if( BM::GUID::NullData() == m_kExpeditionGuid )
	{
		return;
	}

	SExpeditionInfo NewExpedition;
	NewExpedition.ExpeditionGuid = m_kExpeditionGuid;
	NewExpedition.ExpeditionName = m_kExpeditionName;
	NewExpedition.cCurMember = GetNumExpeditionMember();
	NewExpedition.cMaxMember = m_kMaxExpeditionMember;
	NewExpedition.MasterGuid = m_kMasterGuid;
	NewExpedition.ExpeditionOption = Option();

	AddExpedition(NewExpedition);
}

void PgClientExpedition::UpdateExpeditionBalloon(BM::GUID const& CharGuid)
{
	if( !g_pkWorld )
	{
		return;
	}

	if( BM::GUID::IsNotNull(CharGuid) )
	{
		PgActor *pActor = dynamic_cast<PgActor*>(g_pkWorld->FindObject(CharGuid));
		if( pActor )
		{
			pActor->UpdateName();
		}
	}
}

bool PgClientExpedition::SetExpeditionInfo(BM::GUID const& ExpeditionNpcGuid)
{
	BM::CAutoMutex kLock(m_Mutex);
	m_kNpcGuid = ExpeditionNpcGuid;

	CONT_DEF_EXPEDITION_NPC const* pContExpeditionNpc = NULL;
	g_kTblDataMgr.GetContDef(pContExpeditionNpc);
	if( NULL == pContExpeditionNpc )
	{
		return false;
	}

	CONT_DEF_EXPEDITION_NPC::const_iterator iter = pContExpeditionNpc->find(ExpeditionNpcGuid);
	if( iter == pContExpeditionNpc->end() )
	{
		return false;
	}

	m_kMapNo = iter->second.iMapNo;
	m_kMaxExpeditionMember = iter->second.iMaxPlayerCnt;
	m_kMinExpeditionMember = iter->second.iMinPlayerCnt;
	m_kNeedItemNo = iter->second.iNeedItemNo;
	m_kNeedItemCnt = iter->second.iNeedItemCnt;
	m_kDungeonStringNo = iter->second.iDungeonStringNo;
	m_kImgPath = iter->second.kStrImgPath;
	m_kDungeonInfoStringNo = iter->second.iDungeonInfoStringNo;

	return true;
}

void PgClientExpedition::ClearExpeditionInfo()
{
	BM::CAutoMutex kLock(m_Mutex);
	m_kMapNo = 0;
	m_kMaxExpeditionMember = 0;
	m_kMinExpeditionMember = 0;
	m_kNeedItemNo = 0;
	m_kNeedItemCnt = 0;
	m_kDungeonStringNo = 0;
	m_kImgPath.clear();
	m_kDungeonInfoStringNo = 0;
}

void PgClientExpedition::ClearNotHaveKeyItemMember()
{
	BM::CAutoMutex kLock(m_Mutex);
	m_NotHaveItemMember.clear();
}

bool PgClientExpedition::CheckLastInviteTime(std::wstring const& CharName, float const CheckTime)
{
	BM::CAutoMutex kLock(m_Mutex);
	ContExpeditionRejectUser::iterator iter = m_RejectedUser.begin();
	while(m_RejectedUser.end() != iter)
	{
		const ContExpeditionRejectUser::value_type& Element = (*iter);
		if( Element.kCharName == CharName )
		{
			if(	CheckTime > (g_pkApp->GetAccumTime() - Element.fRejectedTime) )
			{
				return false;
			}
			else
			{
				iter = m_RejectedUser.erase(iter);
				continue;
			}
		}
		++iter;
	}
	return true;
}

bool PgClientExpedition::CheckLastInviteTime(BM::GUID const& CharGuid, float const CheckTime)
{
	BM::CAutoMutex kLock(m_Mutex);
	ContExpeditionRejectUser::iterator iter = m_RejectedUser.begin();
	while(m_RejectedUser.end() != iter)
	{
		const ContExpeditionRejectUser::value_type& Element = (*iter);
		if( Element.kCharGuid == CharGuid )
		{
			if(	CheckTime > (g_pkApp->GetAccumTime() - Element.fRejectedTime) )
			{
				return false;
			}
			else
			{
				iter = m_RejectedUser.erase(iter);
				continue;
			}
		}
		++iter;
	}
	return true;
}

void PgClientExpedition::AddCheckInviteTime(ContExpeditionRejectUser::value_type& NewRejectedUser)
{
	BM::CAutoMutex kLock(m_Mutex);
	NewRejectedUser.fRejectedTime = g_pkApp->GetAccumTime();
	ContExpeditionRejectUser::const_iterator iter = m_RejectedUser.begin();
	while( iter != m_RejectedUser.end() )
	{
		if( iter->kCharGuid == NewRejectedUser.kCharGuid )
		{
			return;
		}
		if( iter->kCharName == NewRejectedUser.kCharName )
		{
			return;
		}
		++iter;
	}
	m_RejectedUser.push_back(NewRejectedUser);	// 컨테이너에 없는 새로운 정보만 삽입
}

void PgClientExpedition::AddCheckInviteTime(BM::GUID const& NewRejectedUserGuid)
{
	BM::CAutoMutex kLock(m_Mutex);
	SExpeditionRejectedUser NewRejectedUser;
	NewRejectedUser.fRejectedTime = g_pkApp->GetAccumTime();
	NewRejectedUser.kExpeditionGuid = BM::GUID::NullData();
	NewRejectedUser.kCharGuid = NewRejectedUserGuid;
	NewRejectedUser.kCharName = std::wstring(L"");
	ContExpeditionRejectUser::const_iterator iter = m_RejectedUser.begin();
	while( iter != m_RejectedUser.end() )
	{
		if( iter->kCharGuid == NewRejectedUser.kCharGuid )
		{
			return;
		}
		++iter;
	}
	m_RejectedUser.push_back(NewRejectedUser);	// 컨테이너에 없는 새로운 정보만 삽입
}

void PgClientExpedition::AddCheckInviteTime(std::wstring const& NewRejectedUserName)
{
	BM::CAutoMutex kLock(m_Mutex);
	SExpeditionRejectedUser NewRejectedUser;
	NewRejectedUser.fRejectedTime = g_pkApp->GetAccumTime();
	NewRejectedUser.kExpeditionGuid = BM::GUID::NullData();
	NewRejectedUser.kCharGuid = BM::GUID::NullData();
	NewRejectedUser.kCharName = NewRejectedUserName;
	ContExpeditionRejectUser::const_iterator iter = m_RejectedUser.begin();
	while( iter != m_RejectedUser.end() )
	{
		if( iter->kCharName == NewRejectedUser.kCharName )
		{
			return;
		}
		++iter;
	}
	m_RejectedUser.push_back(NewRejectedUser);	// 컨테이너에 없는 새로운 정보만 삽입
}

bool PgClientExpedition::CheckLastJoinTime(BM::GUID const& ExpeditionGuid, float const CheckTime)
{
	BM::CAutoMutex kLock(m_Mutex);
	ContWaitExpedition::iterator iter = m_WaitExpedition.begin();
	while(m_WaitExpedition.end() != iter)
	{
		const ContExpeditionRejectUser::value_type& Element = (*iter);
		if( Element.kExpeditionGuid == ExpeditionGuid )
		{
			if(	CheckTime > (g_pkApp->GetAccumTime() - Element.fRejectedTime) )
			{
				return false;
			}
			else
			{
				iter = m_WaitExpedition.erase(iter);
				continue;
			}
		}
		++iter;
	}
	return true;
}

void PgClientExpedition::AddCheckJoinTime(BM::GUID const & NewExpedition)
{
	BM::CAutoMutex kLock(m_Mutex);
	SExpeditionRejectedUser NewJoinExpedition;
	NewJoinExpedition.fRejectedTime = g_pkApp->GetAccumTime();
	NewJoinExpedition.kExpeditionGuid = NewExpedition;
	NewJoinExpedition.kCharGuid = BM::GUID::NullData();
	NewJoinExpedition.kCharName = std::wstring(L"");
	ContWaitExpedition::const_iterator iter = m_WaitExpedition.begin();
	while( m_WaitExpedition.end() != iter )
	{
		const ContExpeditionRejectUser::value_type& Element = (*iter);
		if( Element.kExpeditionGuid == NewExpedition )
		{
			return;
		}
		++iter;
	}
	m_WaitExpedition.push_back(NewJoinExpedition);	// 컨테이너에 없는 새로운 정보만 삽입
}

BM::GUID const & PgClientExpedition::GetRejectedMaster(BM::GUID const & ExpeditionGuid)
{
	ContExpeditionRejectUser::const_iterator iter = m_RejectedUser.begin();
	while( iter != m_RejectedUser.end() )
	{ // 가입요청을 거절한 마스터 컨테이너 중에서
		if( iter->kExpeditionGuid == ExpeditionGuid )
		{ // 내가 원하는 원정대의 원정대장 GUID를 반환한다.
			return iter->kCharGuid;
		}
		++iter;
	}
	return BM::GUID::NullData();
}

void PgClientExpedition::UpdateBossGndInfo(BM::GUID const& ExpeditionGuid, DWORD EndTimeTick)
{
	XUI::CXUI_Wnd *pUI = XUIMgr.Activate( L"FRM_HCD_BOSS_ING" );
	if ( pUI )
	{
		pUI->OwnerGuid( ExpeditionGuid );
		pUI->Location(pUI->Location().x, 100);

		if ( 0 < EndTimeTick )
		{
			XUI::CXUI_Wnd *pControl = pUI->GetControl(  L"FRM_DUMMY" );
			if ( pControl )
			{
				int RemainTimeSecond = (EndTimeTick / 1000);
				if ( 0 > RemainTimeSecond )
				{
					RemainTimeSecond = INT_MAX;
				}
				pControl->SetCustomData( &RemainTimeSecond, sizeof(RemainTimeSecond) );
			}
		}
	}
}

void PgClientExpedition::UpdateExpeditionAniBar(BM::GUID const & CharGuid)
{
	SExpeditionMember * pMember = GetMember(CharGuid);
	if( NULL == pMember )
	{
		return;
	}

	XUI::CXUI_Wnd * pWnd = NULL;
	switch(pMember->sTeam)
	{
	case EET_TEAM_1:
		{
			pWnd = XUIMgr.Get(L"FRM_EXPEDITION_MAIN_ITEM_HEAD");
		}break;
	case EET_TEAM_2:
		{
			pWnd = XUIMgr.Get(L"FRM_EXPEDITION_MAIN_ITEM_SUB1");
		}break;
	case EET_TEAM_3:
		{
			pWnd = XUIMgr.Get(L"FRM_EXPEDITION_MAIN_ITEM_SUB2");
		}break;
	case EET_TEAM_4:
		{
			pWnd = XUIMgr.Get(L"FRM_EXPEDITION_MAIN_ITEM_SUB3");
		}break;
	default:
		{
			return;
		}
	}
	if( NULL == pWnd )
	{
		return;
	}
	XUI::CXUI_List * const pList = dynamic_cast<XUI::CXUI_List*>(pWnd->GetControl(L"LST_MAIN_TEAM_LIST"));
	if( NULL == pList )
	{
		return;
	}

	int const ItemCount = pList->GetTotalItemCount();
	for( int i = 0; i < ItemCount; ++i )
	{
		SListItem * pItem = pList->GetItemAt(i);
		if( NULL == pItem )
		{
			continue;
		}
		XUI::CXUI_Wnd * const pItemWnd = pItem->m_pWnd;
		if( NULL == pItemWnd)
		{
			continue;
		}
		BM::GUID ItemCharGuid = pItemWnd->OwnerGuid();
		if( BM::GUID::NullData() == ItemCharGuid )
		{
			continue;
		}
		if( CharGuid != ItemCharGuid )
		{
			continue;
		}

		XUI::CXUI_AniBar * const pAniBar_HP = dynamic_cast<XUI::CXUI_AniBar*>(pItemWnd->GetControl(L"BAR_HP"));
		if( NULL == pAniBar_HP )
		{
			continue;
		}
		pAniBar_HP->Max(PMCA_MAX_PERCENT);
		pAniBar_HP->Now(pMember->sHP);
	}
}

bool PgClientExpedition::ProcessMsg(unsigned short const usType, BM::Stream& Packet)
{
	BM::CAutoMutex kLock(m_Mutex);
	switch( usType )
	{
	case PT_N_C_NFY_CREATE_EXPEDITION:				{ Recv_PT_N_C_NFY_CREATE_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_CREATE_EXPEDITION:				{ Recv_PT_N_C_ANS_CREATE_EXPEDITION(Packet); }break;
	case PT_N_C_NFY_DISPERSE_EXPEDITION:			{ Recv_PT_N_C_NFY_DISPERSE_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_DISPERSE_EXPEDITION:			{ Recv_PT_N_C_ANS_DISPERSE_EXPEDITION(Packet); }break;
	case PT_N_C_REQ_JOIN_EXPEDITION:				{ Recv_PT_N_C_REQ_JOIN_EXPEDITION(Packet); }break;
	case PT_N_C_NFY_JOIN_EXPEDITION:				{ Recv_PT_N_C_NFY_JOIN_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_JOIN_EXPEDITION:				{ Recv_PT_N_C_ANS_JOIN_EXPEDITION(Packet); }break;
	case PT_N_C_NFY_LEAVE_EXPEDITION:				{ Recv_PT_N_C_NFY_LEAVE_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_LEAVE_EXPEDITION:				{ Recv_PT_N_C_ANS_LEAVE_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_RENAME_EXPEDITION:				{ Recv_PT_N_C_ANS_RENAME_EXPEDITION(Packet); }break;
	case PT_N_C_NFY_CHANGEMASTER_EXPEDITION:		{ Recv_PT_N_C_NFY_CHANGEMASTER_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_CHANGEMASTER_EXPEDITION:		{ Recv_PT_N_C_ANS_CHANGEMASTER_EXPEDITION(Packet); }break;
	case PT_N_C_NFY_CHANGEOPTION_EXPEDITION:		{ Recv_PT_N_C_NFY_CHANGEOPTION_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_CHANGEOPTION_EXPEDITION:		{ Recv_PT_N_C_ANS_CHANGEOPTION_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_LIST_EXPEDITION:				{ Recv_PT_N_C_ANS_LIST_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_INFO_EXPEDITION:				{ Recv_PT_N_C_ANS_INFO_EXPEDITION(Packet); }break;
	case PT_M_C_NFY_EXPEDITION_JOIN:				{ Recv_PT_M_C_NFY_EXPEDITION_JOIN(Packet); }break;
	case PT_N_C_NFY_KICKOUT_EXPEDITION:				{ Recv_PT_N_C_NFY_KICKOUT_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_KICKOUT_EXPEDITION:				{ Recv_PT_N_C_ANS_KICKOUT_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_TEAM_MOVE_EXPEDITION:			{ Recv_PT_N_C_ANS_TEAM_MOVE_EXPEDITION(Packet); }break;
	case PT_N_C_NFY_TEAM_MOVE_EXPEDITION:			{ Recv_PT_N_C_NFY_TEAM_MOVE_EXPEDITION(Packet); }break;
	case PT_M_C_ANS_LIST_USER_EXPEDITION:			{ Recv_PT_M_C_ANS_LIST_USER_EXPEDITION(Packet); }break;
	case PT_N_C_REQ_INVITE_EXPEDITION:				{ Recv_PT_N_C_REQ_INVITE_EXPEDITION(Packet); }break;
	case PT_N_C_ANS_INVITE_EXPEDITION_TO_MASTER:	{ Recv_PT_N_C_ANS_INVITE_EXPEDITION_TO_MASTER(Packet); }break;
	case PT_N_C_NFY_EXPEDITION_MODIFY_MEMBER_STATE:	{ Recv_PT_N_C_NFY_EXPEDITION_MODIFY_MEMBER_STATE(Packet); }break;
	case PT_M_C_ANS_NPC_ENTER_EXPEDITION:			{ Recv_PT_M_C_ANS_NPC_ENTER_EXPEDITION(Packet); }break;
	case PT_M_C_NFY_NPC_ENTER_EXPEDITION:			{ Recv_PT_M_C_NFY_NPC_ENTER_EXPEDITION(Packet); }break;
	case PT_M_C_NFY_EXPEDITION_REMAIN_TIME:			{ Recv_PT_M_C_NFY_EXPEDITION_REMAIN_TIME(Packet); }break;
	case PT_M_C_NFY_EXPEDITION_LIMITED_TIME:		{ Recv_PT_M_C_NFY_EXPEDITION_LIMITED_TIME(Packet); }break;
	case PT_M_C_NFY_CHANGE_EXPEDITION_MEMBER_ABIL:	{ Recv_PT_M_C_NFY_CHANGE_EXPEDITION_MEMBER_ABIL(Packet); }break;
	case PT_C_M_ANS_REFRESH_NEED_ITEM_INFO:			{ Recv_PT_C_M_ANS_REFRESH_NEED_ITEM_INFO(Packet); }break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

void PgClientExpedition::Recv_PT_N_C_NFY_CREATE_EXPEDITION(BM::Stream& Packet)
{
	int vectorsize = 0;

	std::vector< SExpeditionMember > Members;	// 원정대 멤버 리스트를 받기 위한 임시 공간

	Packet.Pop(m_kExpeditionGuid);		// 원정대 GUID
	Packet.Pop(m_kMasterGuid);			// 원정대장 GUID
	m_kOption.ReadFromPacket(Packet);	// 원정대 옵션
	Packet.Pop(m_kExpeditionName);		// 원정대 이름
	Packet.Pop(vectorsize);				// 원정대 멤버 갯수

	for(int i = 0 ; i < vectorsize ; ++i)	// 갯수만큼 멤버 가져오기
	{
		SExpeditionUserInfo Member;
		Member.ReadFromPacket(Packet);
		AddMember(Member);
	}

	Packet.Pop(m_kNpcGuid);				// 원정대 NPC GUID
	SetExpeditionInfo(m_kNpcGuid);		// 테이블에서 원정대 정보 가져오기

	AddSelfExpedition();

	XUIMgr.Activate(L"FRM_EXPEDITION_REGISTER");
	XUIMgr.Close(L"FRM_EXPEDITION_INFO");
	lua_tinker::call<void>("UpdateExpeditionInfo"); // 원정대 정보 모두 갱신

	std::wstring const& Message = TTW(EMS_ExpeditionCreate);
	SChatLog ChatLog(CT_EVENT);
	g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
}

void PgClientExpedition::Recv_PT_N_C_ANS_CREATE_EXPEDITION(BM::Stream& Packet)
{
	HRESULT hResult = 0;

	Packet.Pop(hResult);			// 결과부터 Pop
	if( PRC_Success == hResult )	// 성공
	{
	}
	else 
	{
		//에러 처리부분
	}
}

void PgClientExpedition::Recv_PT_N_C_NFY_DISPERSE_EXPEDITION(BM::Stream& Packet)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return;
	}
	// 파티부터 삭제
	g_kParty.RemovePartyName(g_kParty.PartyGuid(), pPlayer->GetID());
	g_kParty.Clear(true);	// PgPlayer에 있는 파티GUID를 지우기 위해 호출

	Packet.Pop(m_kExpeditionGuid);
	
	Clear(); // 여기서 PgPlayer에 있는 원정대GUID 삭제하고 맴버 메모리풀 해제

	lua_tinker::call<void>("AllCloseExpeditionWindow"); // 원정대와 관련된 모든 창을 닫는다.

	std::wstring const& Message = TTW(EMS_ExpeditionDestroy);
	SChatLog ChatLog(CT_EVENT);
	g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
}

void PgClientExpedition::Recv_PT_N_C_ANS_DISPERSE_EXPEDITION(BM::Stream& Packet)
{
	HRESULT hResult = 0;

	Packet.Pop(hResult);			// 결과부터 Pop
	if( PRC_Success == hResult )	// 성공
	{
	}
	else 
	{
		//에러 처리부분
	}
}

void PgClientExpedition::Recv_PT_N_C_REQ_JOIN_EXPEDITION(BM::Stream& Packet)
{
	SExpeditionUserInfo NewMember;

	Packet.Pop(NewMember.CharName);
	Packet.Pop(NewMember.kCharGuid);
	Packet.Pop(NewMember.sLevel);
	Packet.Pop(NewMember.iClass);
	
	// 받은 요청자 정보를 화면에 보여주고 가입수락 여부를 기다린다.
	BM::vstring Message( TTW(720014) );
	Message.Replace(L"$CHAR_NAME$", NewMember.CharName);
	Message.Replace(L"$LEVEL$", NewMember.sLevel);
	Message.Replace(L"$CLASS$", PgClientExpeditionUtil::GetClassNameTTW(NewMember.iClass));
	BM::vstring NowMessage(Message);
	NowMessage.Replace(L"$REMAIN_SEC$", EXPEDITION_JOIN_REQ_WAIT_TIME);
	CallYesNoMsgBoxLimit(NowMessage, NewMember.kCharGuid, MBT_EXPEDITION_JOIN, Message);
}

void PgClientExpedition::Recv_PT_N_C_ANS_JOIN_EXPEDITION(BM::Stream& Packet)
{
	HRESULT hResult = 0;
	int vectorsize = 0;

	lua_tinker::call<void>("ReqUnRegistPrivate"); // 개인등록 및 찾기 취소

	Packet.Pop(hResult);			// 결과부터 Pop
	if( PRC_Success == hResult )	// 성공
	{
		Packet.Pop(m_kExpeditionGuid);		// 원정대 GUID
		Packet.Pop(m_kMasterGuid);			// 원정대장 GUID
		m_kOption.ReadFromPacket(Packet);	// 원정대 옵션
		Packet.Pop(m_kExpeditionName);		// 원정대 이름
		Packet.Pop(vectorsize);				// 원정대 멤버 갯수

		for(int i = 0 ; i < vectorsize ; ++i)	// 갯수만큼 멤버 가져오기
		{
			SExpeditionUserInfo Member;
			Member.ReadFromPacket(Packet);
			AddMember(Member);
		}

		Packet.Pop(m_kNpcGuid);				// 원정대 NPC GUID
		SetExpeditionInfo(m_kNpcGuid);		// 테이블에서 원정대 정보 가져오기

		AddSelfExpedition();

		XUIMgr.Activate(L"FRM_EXPEDITION_REGISTER");

		std::wstring const& Message = TTW(EMS_Join_Success);
		SChatLog ChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
	}
	else 
	{
		//에러 처리부분
		if( PRC_Fail_Refuse == hResult )
		{ // 가입 요청 거절
			SExpeditionRejectedUser NewRejectedUser;
			Packet.Pop(NewRejectedUser.kExpeditionGuid);// 원정대 GUID
			Packet.Pop(NewRejectedUser.kCharGuid);		// 원정대장 GUID
			Packet.Pop(NewRejectedUser.kCharName);		// 원정대 이름
			AddCheckInviteTime(NewRejectedUser);

			std::wstring const& Message = TTW(EMS_Fail_ReqJoin);
			SChatLog ChatLog(CT_EVENT_SYSTEM);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
		}
		else if( PRC_Fail_Level == hResult )
		{
			BM::GUID ExpeditionGUID;
			Packet.Pop(ExpeditionGUID);
			CheckLastJoinTime(ExpeditionGUID, 0.0f); // 원정대 수락 대기 리스트에서 지운다.

			std::wstring const& Message = TTW(EMS_Join_Level_Fail);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
		}
	}
}

void PgClientExpedition::Recv_PT_N_C_NFY_JOIN_EXPEDITION(BM::Stream& Packet)
{
	SExpeditionUserInfo NewMember;
	NewMember.ReadFromPacket(Packet);
	AddMember(NewMember);

	AddSelfExpedition();

	lua_tinker::call<void>("UpdateExpeditionInfo"); // 원정대 정보 모두 갱신
	UpdateExpeditionBalloon(m_kMasterGuid);

	std::wstring const& Message = NewMember.CharName + TTW(EMS_Is) + TTW(EMS_Join_Success);
	SChatLog ChatLog(CT_EVENT);
	g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
}

void PgClientExpedition::Recv_PT_N_C_ANS_INVITE_EXPEDITION_TO_MASTER(BM::Stream& Packet)
{
	HRESULT hResult = 0;
	
	Packet.Pop(hResult);
	//거부 했다.
	//최대 인원 풀이다.
	//모르겠다 등등..

	if( PRC_Fail_Refuse == hResult )
	{
		SExpeditionRejectedUser NewRejecteUser;
		NewRejecteUser.kExpeditionGuid = BM::GUID::NullData();
		Packet.Pop(NewRejecteUser.kCharGuid);
		Packet.Pop(NewRejecteUser.kCharName);
		AddCheckInviteTime(NewRejecteUser);

		SChatLog ChatLog(CT_EVENT);
		std::wstring const& Message = NewRejecteUser.kCharName + TTW(EMS_Is) + TTW(EMS_Invite_Reject);
		g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
	}
	else if( PRC_Fail_MemberCount == hResult )
	{
		// 인원이 다 찼다는 메세지
		SChatLog ChatLog(CT_EVENT);
		std::wstring const& Message = TTW(EMS_MoveTeam_Fail);
		g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
	}
}

void PgClientExpedition::Recv_PT_N_C_NFY_LEAVE_EXPEDITION(BM::Stream& Packet)
{
	BM::GUID LeaveUserGuid;

	Packet.Pop(LeaveUserGuid);

	std::wstring LeaveName;
	RemoveMember( LeaveUserGuid, &LeaveName );

	lua_tinker::call<void>("UpdateExpeditionInfo");	// 원정대 정보 모두 갱신
	UpdateExpeditionBalloon(m_kMasterGuid);

	SChatLog ChatLog(CT_EVENT);
	std::wstring const& Message = LeaveName + TTW(EMS_Is) + TTW(EMS_Leave);
	g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
}

void PgClientExpedition::Recv_PT_N_C_ANS_LEAVE_EXPEDITION(BM::Stream& Packet)
{
	HRESULT hResult = 0;

	Packet.Pop(hResult);

	if( PRC_Success == hResult )
	{	
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}
		// 파티부터 삭제
		g_kParty.RemovePartyName(g_kParty.PartyGuid(), pPlayer->GetID());
		g_kParty.Clear(true);	// PgPlayer에 있는 파티GUID를 지우기 위해 호출
		
		Clear(); // 여기서 PgPlayer에 있는 원정대GUID 삭제하고 맴버 메모리풀 해제

		lua_tinker::call<void>("AllCloseExpeditionWindow");	// 원정대와 관련된 모든 창을 닫는다.

		std::wstring const& Message = TTW(EMS_Leave);
		SChatLog ChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
	}
	else
	{
	}
}

void PgClientExpedition::Recv_PT_N_C_ANS_RENAME_EXPEDITION(BM::Stream& Packet)
{
	HRESULT hResult = 0;

	Packet.Pop(hResult);			// 결과부터 Pop

	if( hResult )	// 성공
	{
	}
	else 
	{
		//에러 처리부분
	}
}

void PgClientExpedition::Recv_PT_N_C_NFY_CHANGEMASTER_EXPEDITION(BM::Stream& Packet)
{
	BM::GUID NewMasterGuid;

	Packet.Pop(NewMasterGuid);		// 새로운 원정대장의 GUID를 꺼낸다.
	m_kMasterGuid = NewMasterGuid;	// 원정대장 GUID를 갱신한다.
	ContExpeditionMember::iterator iter;
	for( iter = m_Members.begin(); iter != m_Members.end(); iter++ )
	{
		if( NULL == (*iter) )
		{
			continue;
		}
		if( (*iter)->kCharGuid == m_kMasterGuid )	// 원정대장의 멤버를 찾으면
		{
			SExpeditionUserInfo NewMaster;			// 멤버를 임시저장해 놓고
			NewMaster = (*(*iter));
			RemoveMember((*iter)->kCharGuid, NULL);	// 리스트 맨 앞으로 삽입하기 위해 일단 삭제
			AddMember(NewMaster);					// 원정대장 추가
			
			lua_tinker::call<void>("UpdateExpeditionInfo"); // 원정대 정보 모두 갱신
			
			XUIMgr.Close(L"FRM_EXPEDITION_OPTION");			// 옵션창 닫기
			XUIMgr.Close(L"FRM_EXPEDITION_INVITATION");		// 초대창 닫기

			UpdateExpeditionBalloon(m_kMasterGuid);

			std::wstring const& Message = NewMaster.CharName + TTW(EMS_Is) + TTW(EMS_NewMaster);
			SChatLog ChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
			break;
		}
	}
}

void PgClientExpedition::Recv_PT_N_C_ANS_CHANGEMASTER_EXPEDITION(BM::Stream& Packet)
{
	HRESULT hResult = 0;

	Packet.Pop(hResult);			// 결과부터 Pop

	if( hResult )	// 성공
	{
	}
	else 
	{
		//에러 처리부분
	}
}

void PgClientExpedition::Recv_PT_N_C_NFY_CHANGEOPTION_EXPEDITION(BM::Stream& Packet)
{
	SExpeditionOption NewOption;
	NewOption.ReadFromPacket(Packet);
	
	std::wstring Temp;
	bool const bRet = PgClientExpeditionUtil::ExpeditionOptionStr(NewOption, Temp, true);
	if( bRet )
	{
		std::wstring Message = Temp;
		SChatLog ChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
	}
	m_kOption = NewOption;
}

void PgClientExpedition::Recv_PT_N_C_ANS_CHANGEOPTION_EXPEDITION(BM::Stream& Packet)
{
	HRESULT hResult = 0;

	Packet.Pop(hResult);			// 결과부터 Pop

	if( hResult )	// 성공
	{
	}
	else 
	{
		//에러 처리부분
	}
}

void PgClientExpedition::Recv_PT_N_C_ANS_LIST_EXPEDITION(BM::Stream& Packet)
{
	HRESULT hResult = 0;

	Packet.Pop( hResult );
	
	SetExpeditionListInit();

	if( PRC_Success == hResult )
	{
		PU::TLoadArray_M( Packet, m_ExpeditionList );
		ExpeditionListViewRefresh( NowPage() );
	}
	else
	{
		m_ExpeditionList.clear();
		ExpeditionListViewRefresh( NowPage() );
	}
}

void PgClientExpedition::Recv_PT_M_C_ANS_LIST_USER_EXPEDITION(BM::Stream& Packet)
{
	SExpeditionUser UserInfo;

	int vectorsize = 0;

	m_OtherPlayers.clear();

	Packet.Pop(vectorsize);

	for(int i = 0 ; i < vectorsize ; ++i)	// 갯수만큼 유저 리스트 가져오기
	{
		UserInfo.ReadFromPacket(Packet);
		std::pair<ContOtherPlayer::iterator, bool> Ret = m_OtherPlayers.insert( std::make_pair(UserInfo.CharGuid, UserInfo) );
	}
	lua_tinker::call<void>("LuaCall_ExpeditionUserList");
}

void PgClientExpedition::Recv_PT_N_C_ANS_INFO_EXPEDITION(BM::Stream& Packet)
{
	SExpeditionInfo NewExpedition;

	Packet.Pop(NewExpedition.ExpeditionGuid);
	Packet.Pop(NewExpedition.ExpeditionName);
	Packet.Pop(NewExpedition.cCurMember);
	Packet.Pop(NewExpedition.cMaxMember);
	Packet.Pop(NewExpedition.MasterGuid);
	NewExpedition.ExpeditionOption.ReadFromPacket(Packet);

	if( NewExpedition.ExpeditionGuid == ExpeditionGuid() )	// 내 원정대라면 내 정보도 갱신
	{
		m_kExpeditionGuid = NewExpedition.ExpeditionGuid;
		m_kMasterGuid = NewExpedition.MasterGuid;
		m_kExpeditionName = NewExpedition.ExpeditionName;
		Option(NewExpedition.ExpeditionOption);
		m_kMaxExpeditionMember = NewExpedition.cMaxMember;
	}

	AddExpedition(NewExpedition);

	VEC_GUID CharList;
	PU::TLoadArray_A(Packet, CharList);

	if( !CharList.empty() )
	{
		m_NotHaveItemMember.swap(CharList);
	}

	if( ExpeditionGuid().IsNotNull() )	// 원정대에 속해있고
	{
		if( NULL != g_pkWorld )
		{
			if( g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_GROUND) )
			{ // 원정대 던전 안이고
				if ( g_kExpeditionComplete.E_MSN_CPL_NONE == g_kExpeditionComplete.GetCompleteState() )
				{ // 결산 상태가 아니면
					lua_tinker::call<void>("OnCall_ExpeditionMainBar");	// 원정대 미니바 띄우기!
					lua_tinker::call<void>("UpdateExpeditionMainbar");	// 미니바 정보 갱신
				}
			}
			if(g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_LOBBY) )
			{
				XUIMgr.Activate(L"FRM_EXPEDITION_REGISTER");
				lua_tinker::call<void>("UpdateExpeditionInfo"); // 원정대 정보 모두 갱신
			}
		}
	}
}

void PgClientExpedition::Recv_PT_M_C_NFY_EXPEDITION_JOIN(BM::Stream& Packet) //타인의 원정대 가입
{
	BM::GUID CharGuid;
	BM::GUID ExpeditionGuid;

	Packet.Pop(CharGuid);
	Packet.Pop(ExpeditionGuid);

	PgClientExpeditionUtil::SetCharacterExpeditionGuid(CharGuid, ExpeditionGuid);
}

void PgClientExpedition::Recv_PT_N_C_NFY_EXPEDITION_MODIFY_MEMBER_STATE(BM::Stream& Packet) //원정대원 사망
{
	BM::GUID CharGuid;

	Packet.Pop(CharGuid);

	SExpeditionMember* pDeadMan = GetMember(CharGuid);
	if( pDeadMan )
	{
		if( g_pkWorld && g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_GROUND) )
		{
			bool IsAlive = true;
			Packet.Pop(IsAlive);
			pDeadMan->bAlive = IsAlive;
			lua_tinker::call<void>("UpdateExpeditionMainbar");	// 미니바 정보 업데이트
		}
	}
}

void PgClientExpedition::Recv_PT_N_C_NFY_KICKOUT_EXPEDITION(BM::Stream& Packet)
{
	BM::GUID KickUserGuid;

	Packet.Pop(KickUserGuid);

	SExpeditionMember* KickMember = GetMember(KickUserGuid);

	if( KickMember )
	{
		std::wstring LeaveName;
		RemoveMember( KickUserGuid, &LeaveName );

		lua_tinker::call<void>("UpdateExpeditionInfo");	// 원정대 정보 모두 갱신
		UpdateExpeditionBalloon(m_kMasterGuid);

		std::wstring const& Message = LeaveName + TTW(EMS_Is) + TTW(EMS_Kickout);
		SChatLog ChatLog(CT_EVENT);
		g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
	}
}

void PgClientExpedition::Recv_PT_N_C_ANS_KICKOUT_EXPEDITION(BM::Stream& Packet)
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return;
	}
	// 파티부터 삭제
	g_kParty.RemovePartyName(g_kParty.PartyGuid(), pPlayer->GetID());
	g_kParty.Clear(true);	// PgPlayer에 있는 파티GUID를 지우기 위해 호출

	Clear(); // 여기서 PgPlayer에 있는 원정대GUID 삭제하고 맴버 메모리풀 해제

	lua_tinker::call<void>("AllCloseExpeditionWindow");	// 원정대와 관련된 모든 창을 닫는다.

	// 강퇴되었다는 메세지
	std::wstring const& Message = TTW(EMS_Kickout);
	SChatLog ChatLog(CT_EVENT);
	g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
}

void PgClientExpedition::Recv_PT_N_C_ANS_TEAM_MOVE_EXPEDITION(BM::Stream& Packet)
{
	HRESULT hResult = 0;

	Packet.Pop(hResult);
	if( PRC_Success == hResult ) // 성공
	{
	}
	else
	{
		// 예외 처리부분
		if( PRC_Fail_MemberCount == hResult )
		{
			// 인원이 다 찼다는 메세지
			SChatLog ChatLog(CT_EVENT);
			std::wstring const& Message = TTW(EMS_MoveTeam_Fail);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
		}
	}
}

void PgClientExpedition::Recv_PT_N_C_NFY_TEAM_MOVE_EXPEDITION(BM::Stream& Packet)
{
	BM::GUID CharGuid;

	SExpeditionUserInfo UserInfo;
	
	UserInfo.ReadFromPacket(Packet);

	SExpeditionMember* MoveMember = GetMember(UserInfo.kCharGuid);

	if( MoveMember )
	{
		MoveMember->sTeam = UserInfo.sTeam;
		
		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( NULL == pPlayer )
		{
			return;
		}

		if( MoveMember->kCharGuid == pPlayer->GetID() )	// 이동된 사람이 자신이라면
		{
			lua_tinker::call<void>("UpdateExpeditionMyTeamInfo");	// 변경된 자신의 팀 정보로 갱신
		}
		else
		{
			lua_tinker::call<void>("UpdateExpeditionInfo");	// 아니면 현재 선택된 팀 정보로 갱신
		}
	}
}

void PgClientExpedition::Recv_PT_N_C_REQ_INVITE_EXPEDITION(BM::Stream& Packet)
{
	BM::GUID ExpeditionGuid;
	std::wstring ExpeditionName;
	SExpeditionUserInfo Master;

	Packet.Pop(ExpeditionGuid);
	Packet.Pop(ExpeditionName);

	Packet.Pop(Master.kCharGuid);
	Packet.Pop(Master.CharName);
	Packet.Pop(Master.sLevel);
	Packet.Pop(Master.iClass);

	// 받은 요청자 정보를 화면에 보여주고 가입수락 여부를 기다린다.
	BM::vstring Message( TTW(EMS_InvitePop) );
	Message.Replace(L"$CHAR_NAME$", Master.CharName);
	Message.Replace(L"$LEVEL$", Master.sLevel);
	Message.Replace(L"$CLASS$", PgClientExpeditionUtil::GetClassNameTTW(Master.iClass));
	BM::vstring NowMessage(Message);
	NowMessage.Replace(L"$REMAIN_SEC$", EXPEDITION_JOIN_REQ_WAIT_TIME);
	CallYesNoMsgBoxLimit(NowMessage, ExpeditionGuid, MBT_EXPEDITION_INVITE, Message);
}

void PgClientExpedition::Recv_PT_M_C_ANS_NPC_ENTER_EXPEDITION(BM::Stream& Packet)
{
}

void PgClientExpedition::Recv_PT_M_C_NFY_NPC_ENTER_EXPEDITION(BM::Stream& Packet)
{ // 입장 실패 시 예외처리를 여기서 한다
	HRESULT hResult = 0;

	Packet.Pop(hResult);

	if( PRC_Fail == hResult )
	{
	}
	else if( PRC_Fail_WaitPlayer == hResult )
	{
		// 교체대기에 원정대원이 있다는 메세지
		SChatLog ChatLog(CT_EVENT);
		std::wstring const& Message = TTW(EMS_Enter_Fail_Team);
		g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
	}
	else if( PRC_Fail_NoHaveKeyItem == hResult )
	{
		int vectorsize = 0;

		Packet.Pop(vectorsize);

		m_NotHaveItemMember.clear();	// 먼저 비우고

		for( int i = 0; i < vectorsize; ++i )
		{
			BM::GUID NotHaveKeyItemUser;
			Packet.Pop(NotHaveKeyItemUser);
			if( BM::GUID::NullData() != NotHaveKeyItemUser )
			{	// 키 아이템을 가지고 있지 않은 유저들을 컨테이너에 담는다.
				m_NotHaveItemMember.push_back(NotHaveKeyItemUser);
			}
		}
		// 입장아이템이 없다는 메세지
		SChatLog ChatLog(CT_EVENT);
		std::wstring const& Message = TTW(EMS_Enter_Fail_Item);
		g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
		lua_tinker::call<void>("UpdateExpeditionInfo");	// 원정대 정보 갱신
	}
}

void PgClientExpedition::Recv_PT_M_C_NFY_EXPEDITION_REMAIN_TIME(BM::Stream& Packet)
{
	if( BM::GUID::NullData() == m_kExpeditionGuid )
	{
		return;
	}
	DWORD EndTimeTick = 0;
	__int64 SeverNowTime = 0i64;
	Packet.Pop( EndTimeTick );
	Packet.Pop( SeverNowTime );
	if( 0 < EndTimeTick )
	{
		if( 0i64 < SeverNowTime )
		{
			__int64 const ClientNowTime = g_kEventView.GetLocalSecTime( CGameTime::DEFAULT );
			DWORD DelayTimeTick = (ClientNowTime - SeverNowTime) / CGameTime::MILLISECOND;
			EndTimeTick = EndTimeTick - DelayTimeTick;
		}
		UpdateBossGndInfo( m_kExpeditionGuid, EndTimeTick );
	}
}

void PgClientExpedition::Recv_PT_M_C_NFY_EXPEDITION_LIMITED_TIME(BM::Stream& Packet)
{
	XUI::CXUI_Form * pForm = dynamic_cast<XUI::CXUI_Form*>(XUIMgr.Call(L"FRM_MISSION_STAGE_FAILED"));
	if( pForm )
	{
		XUI::CXUI_Form * pText = dynamic_cast<XUI::CXUI_Form*>(pForm->GetControl(L"FRM_TEXT"));
		if( pText)
		{
			pText->Text(TTW(710066));
		}
	}
}

void PgClientExpedition::Recv_PT_M_C_NFY_CHANGE_EXPEDITION_MEMBER_ABIL(BM::Stream& Packet)
{
	BM::GUID CharGuid;
	BYTE ChangeAbilType = 0;
	unsigned short Val = 0;

	Packet.Pop(CharGuid);
	Packet.Pop(ChangeAbilType);

	SExpeditionMember * pMember = GetMember(CharGuid);
	if( NULL == pMember )
	{
		return;
	}

	if( 0 != (ChangeAbilType & PMCAT_Class) )
	{
		Packet.Pop(Val);
		pMember->iClass = Val;
	}
	if( 0 != (ChangeAbilType & PMCAT_Level) )
	{
		Packet.Pop(Val);
		pMember->sLevel = Val;
	}
	if( 0 != (ChangeAbilType & PMCAT_HP) )
	{
		Packet.Pop(Val);
		pMember->sHP = Val;
	}
	if( 0 != (ChangeAbilType & PMCAT_MP) )
	{
		Packet.Pop(Val);
		pMember->sMP = Val;
	}
	UpdateExpeditionAniBar(CharGuid);
}

void PgClientExpedition::Recv_PT_C_M_ANS_REFRESH_NEED_ITEM_INFO(BM::Stream& Packet)
{
	m_NotHaveItemMember.clear();
	PU::TLoadArray_A( Packet, m_NotHaveItemMember );
	lua_tinker::call<void>("UpdateExpeditionInfo");	// 원정대 정보 갱신
}
// 원정대 참가 관련 코드 /////////////////////////////////////////////////////////////////////////

int PgClientExpedition::ExpeditionListMaxPage()
{
	int OtherExpeditionSize = m_ExpeditionList.size();
	int AddPage = ( OtherExpeditionSize > EV_MAX_EXPEDITION_VIEW_LIST_CNT ) ? 1 : 0;
	int Page = std::max< int >( ( OtherExpeditionSize / EV_MAX_EXPEDITION_VIEW_LIST_CNT ), 1 ) + AddPage;
	if( EV_MAX_EXPEDITION_VIEW_LIST_CNT < Page )
	{
		Page = EV_MAX_EXPEDITION_VIEW_LIST_CNT;
	}
	return Page;
}

int PgClientExpedition::ExpeditionListNowPage()
{
	return NowPage();
}

void PgClientExpedition::SetExpeditionListInit()
{
	m_ExpeditionList.clear();
}

void PgClientExpedition::ExpeditionListViewRefresh(int NowPage)
{
	if( NowPage > ExpeditionListMaxPage() )
	{ // 다음페이지가 최대 페이지를 벗어나면
		return;
	}
	if( NowPage < 1 )
	{ // 다음페이지가 최소 페이지를 벗어나면
		return;
	}
	// 파티찾기창의
	XUI::CXUI_Wnd* pTopWnd = XUIMgr.Get(_T("SFRM_PARTY_PART_FIND"));
	if( !pTopWnd )
	{
		return;
	}
	// 원정대 참가 창을 가져온다
	XUI::CXUI_Wnd* pTopFom = pTopWnd->GetControl(_T("FRM_EX_ITEM"));
	if( !pTopFom )
	{
		return;
	}
	// 원정대 리스트
	XUI::CXUI_List* pListWnd = dynamic_cast<XUI::CXUI_List*>(pTopFom->GetControl(_T("LST_EXPEDITION_PART_LIST")));
	if( !pListWnd )
	{
		return;
	}
	// 페이지 번호가 들어갈 윈도우
	XUI::CXUI_Form* pPageWnd = dynamic_cast<XUI::CXUI_Form*>(pTopWnd->GetControl(_T("FRM_PAGE2")));
	if( !pPageWnd )
	{
		return;
	}

	BM::vstring vCur(NowPage);
	BM::vstring vMax(ExpeditionListMaxPage());
	vCur += _T("/");
	vCur += vMax;
	pPageWnd->Text(vCur);

	pListWnd->ClearList();
	m_kNowPage = NowPage;			// 현재 페이지 갱신

	XUI::SListItem* pItem = pListWnd->FirstItem();	// 첫번째 아이템부터
	int CustomCount = 1;
	int NowPreview = ((NowPage - 1) * EV_MAX_EXPEDITION_VIEW_LIST_CNT);	// 페이지에 해당하는 최초 아이템 위치

	ExpeditionInfoList::const_iterator iter = m_ExpeditionList.begin();
	while (m_ExpeditionList.end() != iter)
	{
		if( 0 < NowPreview )
		{ // 보여줄 항목까지 이터레이터 이동시킨다.
			--NowPreview;
			++iter;
			continue;
		}

		if( !pItem )
		{ // 아이템이 없다면 새로 등록함
			pItem = pListWnd->AddItem(L"");
		}

		if(	pItem )
		{
			XUI::CXUI_Form* pForm = dynamic_cast<XUI::CXUI_Form*>(pItem->m_pWnd);

			if(pForm)
			{
				BM::GUID const& ExpeditionGuid = iter->ExpeditionGuid;

				if( BM::GUID::NullData() == ExpeditionGuid )
				{ // 원정대 GUID가 없으면 다음 리스트로
					++iter;
					continue;
				}

				pForm->OwnerGuid(ExpeditionGuid);	//아이템에 원정대 GUID를 등록
				pForm->SetCustomData(&CustomCount, sizeof(CustomCount));	// 고유번호 등록

				XUI::CXUI_Form *pNameForm = dynamic_cast<XUI::CXUI_Form*>(pForm->GetControl(L"FRM_NAME"));
				if( pNameForm )
				{
					std::wstring MasterName = iter->MasterName;

					if( 10 < MasterName.size() )
					{ // 원정대장 이름의 크기가 10 이상이면 짤라서 보여줌
						MasterName = MasterName.substr(0, 10) + _T("...");
					}
					pNameForm->Text(MasterName);	//원정대장 이름
				}
				XUI::CXUI_Wnd *pArea = pForm->GetControl(std::wstring(_T("FRM_AREA")));
				if( pArea )
				{ // 원정대 장소
					BM::vstring vStr(GetMapName(iter->MapNo));	// 번호에 해당되는 맵 이름을 가져온다.
					pArea->Text(vStr);							// 결과를 윈도우에 출력
				}
				XUI::CXUI_Wnd *pMember = pForm->GetControl(std::wstring(_T("FRM_MEMBER")));
				if( pMember )
				{ // 원정대 맴버 수
					BM::vstring vCur((int)iter->CurMemberCount);
					BM::vstring vMax((int)iter->MaxMemberCount);

					vCur += _T("/");
					vCur += vMax;

					pMember->Text(vCur);
				}	
			}
		}
		++CustomCount;
		++iter;
		pItem = pListWnd->NextItem(pItem);

		if( CustomCount > EV_MAX_EXPEDITION_VIEW_LIST_CNT )
		{ // 등록한 아이템이 12개가 넘으면 끝
			break;
		}
	}
}