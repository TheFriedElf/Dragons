#include "stdafx.h"
#include "lwUI.h"
#include "lwBase.h"
#include "lwPacket.h"
#include "ServerLib.h"
#include "variant/PgMission.h"
#include "PgUIScene.h"
#include "PgMissionComplete.h"
#include "variant/TableDataManager.h"
#include "PgPilotMan.h"
#include "PgQuestMan.h"
#include "lwUIMission.h"
#include "PgMissionQuestMan.h"
#include "PgNiFile.h"
#include "PgClientParty.h"
#include "PgPilot.h"
#include "PgUtilAction.h"
#include "Lohengrin/packetstruct.h"
#include "Variant/PgHiddenRewordItemMgr.h"
#include "PgChatMgrClient.h"
#include "Variant/PgMissionInfo.h"
#include "PgTrigger.h"
#include "PgParticleMan.h"
#include "PgEventTimer.h"
#include "PgUISound.h"
#include "lwQuaternion.h"
#include "PgWorldMapUI.h"

int g_iMissionDifficultyLevel = -1;
int g_iMissionMapCount = 5;
int g_iMissionNo = 0;
int g_iMissionKey = 0;
static bool g_bAutoLevel = false;
char* szMissionQuestUI = "FRM_MISSION_QUEST";
std::wstring const WSTR_SPACE_CHAR =_T(" ");
VEC_INT	g_VecLV5QuestValue;
VEC_INT	g_VecLV6QuestValue;
VEC_INT g_VecHiden5ClearQuestvalue;
VEC_INT g_VecNeedItem;
VEC_INT g_VecRequiredItemNo;

int g_iDefence7MaxLevel = 0;
int g_iDefence7MinLevel = 0;
int g_iEventMission = 0;

PgPlayer_MissionData PlayerMissionData;
int g_MissionQuestCount[2] = {0};		// [0] : 아케이드모드, [1] : 카오스모드
int g_MissionAchievementCount[2] = {0};	// [0] : 아케이드모드, [1] : 카오스모드

extern bool GetTimeString(int iTime, std::wstring &rkOutString, bool const bEndLine);
extern bool lwIsEndedQuest(unsigned int sQuestID);


int GetDefenceModeType(int const iGameLevel);
void SetPointUI(XUI::CXUI_Wnd* pkWnd);
bool GetDefence7Mission(SMISSION_DEFENCE7_MISSION & kMission, int const iMissionNo=0, int const iMissionLv=0);

int lwGetMissionMapCount()
{
	return g_iMissionMapCount;
}

int lwGetMissionDifficultyLevel()
{
	return g_iMissionDifficultyLevel;
}

void lwSetMissionNo(int iMissionNo)
{
	g_iMissionNo = iMissionNo;

	CONT_DEF_MISSION_ROOT const* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);
	if( pContDefMap )
	{
		if( g_iMissionNo )
		{
			CONT_DEF_MISSION_ROOT::const_iterator	root_it = pContDefMap->find(g_iMissionNo);
			if( root_it != pContDefMap->end())
			{
				g_iMissionKey = root_it->second.iKey;
			}
		}
	}
}

int lwGetMissionNo()
{
	return g_iMissionNo;
}

int lwGetGameLevel()
{
	return g_pkWorld ? (g_pkWorld->GetDifficultyLevel()) : 0;
}

int lwGetDefence7MinLevel()
{
	return g_iDefence7MinLevel;
}

int lwGetDefence7MaxLevel()
{
	return g_iDefence7MaxLevel;
}

bool lwDefence7EnterLevelCheck()
{
	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		int const MyLevel = pPlayer->GetAbil(AT_LEVEL);
		
		return (lwGetDefence7MinLevel() <= MyLevel) && (MyLevel <= lwGetDefence7MaxLevel());
	}
	return true;
}

bool IsAutoLevel(int const iMissionNo)
{
	CONT_DEF_MISSION_ROOT const* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);
	if(!pContDefMap)
	{
		return false;
	}
	CONT_DEF_MISSION_ROOT::const_iterator root_it = pContDefMap->find(iMissionNo);
	if(pContDefMap->end() == root_it)
	{
		return false;
	}

	for(int i=0; i<MAX_MISSION_LEVEL; ++i)
	{
		if(0==root_it->second.aiLevel_AvgMin[i] && 0==root_it->second.aiLevel_AvgMax[i] && (0 < root_it->second.aiLevel[i]))
		{
			return true;
		}
	}
	return false;
}

bool IsHaveArcadeMode()
{
	PgActor * pActor = g_kPilotMan.GetPlayerActor();
	if( !pActor )
	{
		return false;
	}
	PgTrigger * pCurrTrigger = pActor->GetCurrentTrigger();
	if( !pCurrTrigger )
	{//현재 트리거가 없는데 이 정보를 요청한다는 것은 미션바로가기 버튼을 눌렀다는 뜻.
		if( !g_pkWorld )
		{
			return false;
		}
		CONT_TRIGGER_INFO ContTrigger;
		g_pkWorld->GetTriggerByType(PgTrigger::TRIGGER_TYPE_MISSION_EVENT_NOT_HAVE_ARCADE, ContTrigger);
		if( ContTrigger.empty() )
		{//현재 월드에 MISSION_EVENT_NOT_HAVE_ARCADE 타입의 트리거가 하나도 없으면
			return true;
		}
		else
		{
			return false;
		}
	}
	if( PgTrigger::TRIGGER_TYPE_MISSION_EVENT_NOT_HAVE_ARCADE == pCurrTrigger->GetTriggerType() )
	{
		return false;
	}
	else
	{
		return true;
	}
	return false;
}

void lwSetMissionGray(lwUIWnd kWnd, int const iMissionNo, int const iLevel)
{
	XUI::CXUI_Wnd* pkTopWnd = kWnd();
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pkWndCardBG = pkTopWnd->GetControl(std::wstring(_T("IMG_BG")));
	XUI::CXUI_Wnd* pkWndCardBG1 = pkTopWnd->GetControl(std::wstring(_T("IMG_BG1")));

	const CONT_DEF_MISSION_ROOT* pContDefRoot = NULL;
	g_kTblDataMgr.GetContDef(pContDefRoot);

	if(!pContDefRoot)
	{
		return;
	}

	CONT_DEF_MISSION_ROOT::const_iterator root_it = pContDefRoot->find((iMissionNo));
	if (pContDefRoot->end() != root_it)
	{
		bool bGray = false;
		TBL_DEF_MISSION_ROOT const& rkRoot = (*root_it).second;
		if( 0 == rkRoot.aiLevel[iLevel] )
		{
			bGray = true;
		}

		if( pkWndCardBG )
		{
			pkWndCardBG->GrayScale(bGray);
		}
		if( pkWndCardBG1 ) 
		{
			pkWndCardBG1->GrayScale(bGray);
		}
	}
}

void lwCallRankInfo(int const iMissionKey, int const iLevel)
{
	SMissionKey kKey;

	kKey.iKey = iMissionKey;
	kKey.iLevel = iLevel;

	BM::Stream kPacket(PT_C_M_REQ_RANK_PAGE);
	kPacket.Push(kKey);
	NETWORK_SEND(kPacket)
}

void lwCallRankTop(int const iMissionKey, int const iLevel)
{
	SMissionKey kKey;

	kKey.iKey = iMissionKey;
	kKey.iLevel = iLevel;

	BM::Stream kPacket(PT_C_M_REQ_RANK_TOP);
	kPacket.Push(kKey);
	NETWORK_SEND(kPacket)
}

void lwSetMission_Defence_Hp(lwGUID kGuid, lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pkTopWnd = kWnd();
	if( !pkTopWnd )
	{
		pkTopWnd = XUIMgr.Call(std::wstring(_T("FRM_MISSION_DEFENCE_HP_BAR")));
	}
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_AniBar* pAniBarHp = dynamic_cast<XUI::CXUI_AniBar*>(pkTopWnd->GetControl(std::wstring(L"BAR_HP")));

	if( !pAniBarHp )
	{
		return;
	}

	pAniBarHp->SetCustomData(&kGuid(), sizeof(kGuid()));

	XUI::CXUI_Wnd* pkIcon = pkTopWnd->GetControl(_T("IMG_ICON"));
	if( !pkIcon )
	{
		return;
	}

	char szName[255] = {0,};
	_snprintf(szName, 255, "../Data/6_ui/mission/msDfsLft_%d.tga", lwGetMissionNo());
	if( false == PgNiFile::CheckFileExist(szName) )
	{
		_snprintf(szName, 255, "../Data/6_ui/mission/msDfsLft.tga");		
	}
	pkIcon->DefaultImgName(UNI(szName));
}

int lwRecv_PT_M_C_ANS_MISSION_INFO(lwPacket kPacket)
{//미션 정보 왔음
	//SFRM_CREATE_FIELD 부르면 되고

	g_iDefence7MaxLevel = 0;
	g_iDefence7MinLevel = 0;
	int iEventMission = 0;
	
	int iType = 0;
	BM::Stream *pkPacket = kPacket();

	if(pkPacket)
	{
		SMissionInfo kMission;		
		VEC_INT	VecClearQuestValue;
		VEC_INT	VecLv6ClearQuestValue;
		bool bDisableExchangeSoulStoneBtn = false;

		g_VecLV5QuestValue.clear();
		g_VecLV6QuestValue.clear();

		PlayerMissionData.ReadFromPacket(*pkPacket);
		kMission.ReadFromPacket(*pkPacket);
		pkPacket->Pop(iType);
		pkPacket->Pop(VecClearQuestValue);
		pkPacket->Pop(VecLv6ClearQuestValue);
		pkPacket->Pop(g_VecLV5QuestValue);
		pkPacket->Pop(g_VecLV6QuestValue);
		pkPacket->Pop(g_iDefence7MinLevel);
		pkPacket->Pop(g_iDefence7MaxLevel);
		pkPacket->Pop(g_VecNeedItem);
		pkPacket->Pop(iEventMission);
		pkPacket->Pop(g_VecHiden5ClearQuestvalue);
		pkPacket->Pop(g_VecRequiredItemNo);
		
		g_kMissionMgr.SetClearQuestNo(VecClearQuestValue);
		g_kMissionMgr.SetClearQuestNoLv6(VecLv6ClearQuestValue);
		lwSetEventMission(iEventMission);

		g_iMissionNo = kMission.m_iMissionNo;

		PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
		if( pPlayer )
		{
			if( PgClientPartyUtil::IsInParty() && !PgClientPartyUtil::IsPartyMaster(pPlayer->GetID()) )
			{
				return 0;
			}
		}

		//g_iMissionMapCount = 5; //여기가 아니야
		g_bAutoLevel = IsAutoLevel(kMission.m_iMissionNo);

		PgActor *pkActor = g_kPilotMan.GetPlayerActor();
		if (pkActor)
		{
			std::wstring kFrmName = _T("");
			switch( static_cast<EMissionTypeKind>(iType) )
			{
			case MT_EVENT_MISSION:
				{
					Mission_Item_Info_Event(kMission);
					kFrmName = _T("FRM_CREATE_EVENT_ITEM");
				}break;
			case MT_EVENT_MISSION_NOT_HAVE_ARCADE:
				{
					Mission_Scenario_Not_Have_Arcade_Info(kMission);
					Mission_Item_Not_Have_Arcade_Info(kMission);
					kFrmName = _T("FRM_CREATE_SCENARIO_ITEM");
				}break;
			default:
				{
					Mission_Scenario_Info(kMission,g_VecNeedItem);
					Mission_Item_Info(kMission);
					Mission_PartyList_Info(kMission);
					kFrmName = _T("FRM_CREATE_SCENARIO_ITEM");
				}break;
			}
			
			if( MT_EVENT_HIDDEN == static_cast<EMissionTypeKind>(iType) ) // Event Map일 경우
			{
				bool bAllClearMode = true;

				int const iDiff = 4;	// 아이템 모드 상태인 경우만 처리한다. Lv 4까지
				for (int i = 0; i < iDiff; ++i)
				{
					BYTE const btDoor = kMission.m_kDoor[i];
					if( SMissionInfo::MDOOR_CLEARED != btDoor )
					{
						bAllClearMode = false;
						break;
					}
				}

				if( true == bAllClearMode )
				{
					// 히든 모드로 진입
					Mission_Hidden_Info(kMission);
				}
			}

			if( g_pkWorld && !g_pkWorld->IsHaveAttr(GATTR_MISSION) )
			{
				g_kUIScene.RegistUIAction(kFrmName, _T("CloseUI"), pkActor->GetTranslate());
			}
		}
	}
	return iType;
}

bool lwIsAutoLevel(int const iMissionNo)
{
	return IsAutoLevel(iMissionNo);
}

bool lwIsHaveArcadeMode()
{
	return IsHaveArcadeMode();
}

void Mission_Scenario_Info(SMissionInfo& kMission,VEC_INT const& VecNeedItem)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Call(std::wstring(_T("FRM_CREATE_SCENARIO_ITEM")));
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD_SCENARIO"));

	if(pWnd == NULL)
	{
		return;
	}

	XUI::CXUI_Wnd* pBgImg1 = pWnd->GetControl(_T("FRM_BG_ITEM"));
	XUI::CXUI_Wnd* pBgImg2 = pWnd->GetControl(_T("FRM_BG_ITEM_AUTO"));
	XUI::CXUI_Wnd* pItemText = pWnd->GetControl(_T("FRM_ITEM_AUTO_TEXT"));
	XUI::CXUI_Wnd* pBgNotHaveArcade = pWnd->GetControl(_T("FRM_BG_EVENT_NOT_HAVE_ARCADE"));
	if(!pBgImg1 || !pBgImg2 || !pItemText || !pBgNotHaveArcade)
	{
		return;
	}

	pWnd->SetCustomData(&kMission.m_iMissionKey, sizeof(kMission.m_iMissionKey));	//미션 번호 저장

	CONT_DEF_QUEST_REWARD const* pkDefReward = NULL;
	g_kTblDataMgr.GetContDef(pkDefReward);

	if( !pkDefReward )
	{
		return;
	}

	int iLv5QuestNo = 0;
	int iLv6QuestNo = 0;
	bool bIsGray = true;
	char szWndName[255] = {0,};
	int const iDiff = MAX_MISSION_LEVEL;
	for (int i = 0; i < iDiff; ++i)
	{
		sprintf_s(szWndName, 255, "FRM_CARD%d", i);
		XUI::CXUI_Wnd* pkWndCard = pWnd->GetControl(UNI(szWndName));
		if (pkWndCard)
		{
			BYTE const btDoor = kMission.m_kDoor[i];
			if (SMissionInfo::MDOOR_NONE!=btDoor)
			{
				pkWndCard->SetCustomData(&i, sizeof(i));
			}
			// Scenario
			//if( kMission.m_kDoor[4] == SMissionInfo::MDOOR_CLOSE )
			{
				XUI::CXUI_Wnd* pkWndCardBG = pkWndCard->GetControl(std::wstring(_T("IMG_BG")));
				if( pkWndCardBG )
				{
					if( i == 0 )
					{
						XUI::CXUI_Wnd* pkWndCardClear = pkWndCardBG->GetControl(std::wstring(_T("IMG_CLEAR")));
						if (pkWndCardClear)
						{
							bool bIsCleared = SMissionInfo::MDOOR_CLEARED==kMission.m_kDoor[4];
							pkWndCardClear->Visible(bIsCleared);

							XUI::CXUI_Wnd* pkLV5Text = pkWndCardBG->GetControl(_T("FRM_LV5_QUEST"));
							XUI::CXUI_Wnd* pkLV6Text = pkWndCardBG->GetControl(_T("FRM_LV6_QUEST"));
							XUI::CXUI_Wnd* pkLV6TextNeedItem = pkWndCardBG->GetControl(_T("FRM_LV6_NEED_ITEM"));
							if( pkLV5Text && pkLV6Text )
							{
								if( bIsCleared )
								{
									POINT3I const kPos5 = pkLV5Text->Location();
									pkLV5Text->Location(kPos5.x + 25, kPos5.y);
									POINT3I const kPos6 = pkLV6Text->Location();
									pkLV6Text->Location(kPos6.x + 25, kPos6.y);

									POINT3I const kPos6NeedItem = pkLV6TextNeedItem->Location();
									pkLV6TextNeedItem->Location(kPos6NeedItem.x + 25, kPos6NeedItem.y);
								}
								else
								{
									POINT3I const kPos5 = pkLV5Text->Location();
									pkLV5Text->Location(kPos5.x - 25, kPos5.y);
									POINT3I const kPos6 = pkLV6Text->Location();
									pkLV6Text->Location(kPos6.x - 25, kPos6.y);

									POINT3I const kPos6NeedItem = pkLV6TextNeedItem->Location();
									pkLV6TextNeedItem->Location(kPos6NeedItem.x - 25, kPos6NeedItem.y);
								}
								PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
								if( pkPlayer )
								{
									PgMyQuest const* pkUserQuest = pkPlayer->GetMyQuest();
									if( pkUserQuest )
									{
										if( 1 < g_VecLV5QuestValue.size() )
										{
											int idx = 0;
											if( pkPlayer->UnitRace() == ERACE_DRAGON )
											{
												idx = (pkPlayer->GetAbil(AT_BASE_CLASS) % 10) + 4; // 인간 클래스 더함.
											}
											else
											{
												idx = pkPlayer->GetAbil(AT_BASE_CLASS);	// 1. 전사 2. 마법사 3. 궁수 4. 도둑
											}

											if( idx <= g_VecLV5QuestValue.size() )
											{
												iLv5QuestNo = g_VecLV5QuestValue.at(idx - 1);
											}
										}
										else if( !g_VecLV5QuestValue.empty() )
										{
											iLv5QuestNo = g_VecLV5QuestValue.at(0);
										}

										if( 1 < g_VecLV6QuestValue.size() )
										{
											int idx = 0;
											if( pkPlayer->UnitRace() == ERACE_DRAGON )
											{
												idx = (pkPlayer->GetAbil(AT_BASE_CLASS) % 10) + 4; // 인간 클래스 더함.
											}
											else
											{
												idx = pkPlayer->GetAbil(AT_BASE_CLASS);	// 1. 전사 2. 마법사 3. 궁수 4. 도둑
											}

											if( idx <= g_VecLV6QuestValue.size() )
											{
												iLv6QuestNo = g_VecLV6QuestValue.at(idx - 1);	// 1. 전사2. 마법사3. 궁수4. 도둑
											}
										}
										else if( !g_VecLV6QuestValue.empty() )
										{
											iLv6QuestNo = g_VecLV6QuestValue.at(0);
										}

										if( 0 != iLv5QuestNo )
										{
											CONT_DEF_QUEST_REWARD::const_iterator iter = pkDefReward->find(iLv5QuestNo);
											if( iter != pkDefReward->end() )
											{
												wchar_t kTemp[256]={0,};

												if( pkUserQuest->IsEndedQuest(iLv5QuestNo) )
												{
													pkLV5Text->FontColor(0xFF42FF00);
													wsprintf(kTemp, L"%s%s",TTW(iter->second.iTitleText).c_str(), TTW(799381).c_str());
												}
												else
												{
													pkLV5Text->FontColor( (0xFFFFFFFF) );
													wsprintf(kTemp, L"%s(%s.%d)(%s)", TTW(iter->second.iTitleText).c_str(), TTW(224).c_str(), iter->second.iLevelMin, TTW(65).c_str());
												}

												std::wstring QuestText(kTemp);
												if( g_pkApp->VisibleClassNo() )
												{
													std::wostringstream ostr;
													ostr.clear();
													ostr << L"(" << iter->second.iQuestID << L")";
													QuestText += ostr.str();
												}

												pkLV5Text->Text(QuestText);
												pkLV5Text->Visible(true);
											}
										}
										else
										{
											pkLV5Text->Visible(false);
										}

										if(	0 != iLv6QuestNo && pkUserQuest->IsEndedQuest(iLv5QuestNo)  )
										{
											CONT_DEF_QUEST_REWARD::const_iterator iter = pkDefReward->find(iLv6QuestNo);
											if( iter != pkDefReward->end() )
											{
												wchar_t kTemp[256]={0,};											

												if( pkUserQuest->IsEndedQuest(iLv6QuestNo) )
												{
													pkLV6Text->FontColor(0xFF42FF00);
													wsprintf(kTemp, L"%s%s",TTW(iter->second.iTitleText).c_str(), TTW(799381).c_str());
												}
												else
												{
													pkLV6Text->FontColor( (0xFFFFFFFF) );
													wsprintf(kTemp, L"%s(%s.%d)(%s)", TTW(iter->second.iTitleText).c_str(), TTW(224).c_str(), iter->second.iLevelMin, TTW(65).c_str());
												}

												std::wstring QuestText(kTemp);
												if( g_pkApp->VisibleClassNo() )
												{
													std::wostringstream ostr;
													ostr.clear();
													ostr << L"(" << iter->second.iQuestID << L")";
													QuestText += ostr.str();
												}

												pkLV6Text->Text(std::wstring(QuestText));
												pkLV6Text->Visible(true);
											}
										}
										else
										{
											pkLV6Text->Visible(false);
											pkLV6TextNeedItem->Visible(false);
										}

										if(pkUserQuest->IsIngQuest(iLv5QuestNo) || pkUserQuest->IsIngQuest(iLv6QuestNo))
										{
											if( SMissionInfo::MDOOR_CLEARED == kMission.m_kDoor[0] )
											{
												bIsGray = false;
											}
										}

										if(pkUserQuest->IsEndedQuest(iLv6QuestNo) && (kMission.m_kDoor[5] == SMissionInfo::MDOOR_NEWOPEN))//아이템체크도해야함
										{
											PgInventory *pkInv = pkPlayer->GetInven();
											if( pkInv )
											{
												int const iInGameItemNo= *VecNeedItem.rbegin();
												int const iCashGameItemNo= *VecNeedItem.begin();
												int const iInGameItemCount = pkInv->GetTotalCount(iInGameItemNo);
												int const iCashGameItemCount = 	pkInv->GetTotalCount(iCashGameItemNo);
												const wchar_t *pName = NULL;
												pkLV5Text->Visible(false);

												if(iInGameItemCount == 0 &&
													iCashGameItemCount == 0 &&
													GetDefString(iInGameItemNo, pName))
												{//아이템이 없다면 텍스트 레드
													
													pkLV6TextNeedItem->FontColor(0xFFFF0000);//레드
													pkLV6TextNeedItem->Text(pName);		
													pkLV6TextNeedItem->Visible(true);
												}
												else if(iInGameItemCount == 0 && iCashGameItemCount != 0)
												{
													GetDefString(iCashGameItemNo, pName);
													pkLV6TextNeedItem->FontColor(0xFF42FF00);//초록
													pkLV6TextNeedItem->Text(pName);		
													pkLV6TextNeedItem->Visible(true);

												}
												else if(iInGameItemCount != 0 && iCashGameItemCount == 0)
												{
													GetDefString(iInGameItemNo, pName);
													pkLV6TextNeedItem->FontColor(0xFF42FF00);//초록
													pkLV6TextNeedItem->Text(pName);		
													pkLV6TextNeedItem->Visible(true);
												}	


												if( iInGameItemCount != 0 || iCashGameItemCount != 0)
												{//아이템 인게임,케시 아이템 둘중에 하나라도 있으면 그레이스케일 적용되지 않음.
													
													bIsGray = false;

												}
												else
												{													
													bIsGray = true;
												}
											}											
										}
									}
									if( (pkUserQuest->IsEndedQuest(iLv5QuestNo) && pkUserQuest->IsEndedQuest(iLv6QuestNo))
										|| (pkUserQuest->IsEndedQuest(iLv5QuestNo) && !pkUserQuest->IsIngQuest(iLv6QuestNo)) )
									{
										kMission.m_kDoor[4] = SMissionInfo::MDOOR_CLEARED;
									}
								}
							}
						}
					}
					pkWndCardBG->GrayScale( bIsGray );
				}
			}
		}
	}

	pBgNotHaveArcade->Visible(false);
	if( g_bAutoLevel )
	{
		pBgImg1->Visible(false);
		pBgImg2->Visible(true);
		pItemText->Visible(true);
	}
	else
	{
		pBgImg1->Visible(true);
		pBgImg2->Visible(false);
		pItemText->Visible(false);
	}

	XUI::CXUI_Wnd* pkWndProg = pWnd->GetControl(std::wstring(_T("SFRM_BOSS_BG")));
	if( pkWndProg )
	{
		XUI::CXUI_Wnd* pkImg = pkWndProg->GetControl(_T("IMG_BOSS"));
		if (pkImg)
		{
			char szName[255] = {0,};
			_snprintf(szName, 255, "../Data/6_ui/mission/msBossImg_%d.tga", kMission.m_iMissionNo);
			if(PgNiFile::CheckFileExist(szName))
			{
				pkImg->DefaultImgName(UNI(szName));
				pkImg->Visible(true);
			}
			else
			{
				pkImg->Visible(false);
			}
		}
	}

	XUI::CXUI_Wnd* pWndDefence = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD_DEFENCE"));
	if(!pWndDefence)
	{
		return;
	}
	XUI::CXUI_Wnd* pCardWnd = pWndDefence->GetControl(_T("FRM_CARD0"));
	if( pCardWnd )
	{
		XUI::CXUI_Wnd* pQuestText = pCardWnd->GetControl(_T("FRM_QUEST"));
		if( pQuestText )
		{
			pQuestText->Visible(false);
		}
	}

	int iDefenceHeight = 0;
	if( IsDeffenseMode(kMission.m_iMissionNo) )
	{
		pWndDefence->SetCustomData(&kMission.m_iMissionNo, sizeof(kMission.m_iMissionNo));
		pWndDefence->Visible(true);
		iDefenceHeight = 0;

		lua_tinker::call< void, int >("SetDefenceEnterImg", kMission.m_iMissionNo);

        if( lwIsOpenDefenseModeTable(EDFST_F8, kMission.m_iMissionNo) ||
			SMissionInfo::MDOOR_CLEARED == kMission.m_kDoor[4] )
		{
			XUI::CXUI_Wnd* pDefenceBgImg = pWndDefence->GetControl(_T("FRM_CARD0"))->GetControl(_T("IMG_BG"));
			if( !pDefenceBgImg )
			{
				return;
			}

			pDefenceBgImg->GrayScale(false);
		}

		int iTextNo = 400956;
		if( lwIsOpenDefenseModeTable(EDFST_F7, kMission.m_iMissionNo)
		||	lwIsOpenDefenseModeTable(EDFST_F8, kMission.m_iMissionNo) )
		{
			iTextNo = 401171;
		}
		
		XUI::CXUI_Wnd* pTmp = pWndDefence->GetControl(L"FRM_BG_ITEM");
		pTmp = pTmp ? pTmp->GetControl(L"FRM_TEXT") : NULL;
		if(pTmp)
		{
			pTmp->Text(TTW(iTextNo));
		}
	}
	else
	{
		iDefenceHeight = pWndDefence->Height();
	}
	
	/*int const iScreenCenterY = XUIMgr.GetResolutionSize().y / 2;
	POINT3I kPoint = pkTopWnd->Location();
	kPoint.y = iScreenCenterY - ((pkTopWnd->Height()/2) + iDefenceHeight);
	pkTopWnd->Location(kPoint);*/
}

void Mission_Item_Info(SMissionInfo& kMission)
{
	XUI::CXUI_Wnd* pkTopWnd = lwGetUIWnd("FRM_CREATE_SCENARIO_ITEM").GetSelf();
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* psWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD_SCENARIO"));
	if( !psWnd )
	{
		return;
	}

	int const iDiff2 = MAX_MISSION_LEVEL;
	for (int i = 0; i < iDiff2; ++i)
	{
		BM::vstring vStr("FRM_CARD");
		vStr += i;
		XUI::CXUI_Wnd* pkWndCard = psWnd->GetControl(vStr);
		if (pkWndCard)
		{
			pkWndCard->Visible(true);
		}
	}

	XUI::CXUI_Wnd* pDefenceWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD_DEFENCE"));
	if( pDefenceWnd )
	{
		XUI::CXUI_Wnd* pBgWnd = pDefenceWnd->GetControl(_T("FRM_BG_ITEM"));
		if( pBgWnd )
		{
			pBgWnd->Visible(true);
		}
		XUI::CXUI_Wnd* pTitleWnd = pDefenceWnd->GetControl(_T("FRM_TITLE"));
		if( pTitleWnd )
		{
			pTitleWnd->Visible(false);
		}
	}

	XUI::CXUI_Wnd* pWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD"));
	
	if(pWnd == NULL)
	{
		pWnd = XUIMgr.Call(std::wstring(_T("FRM_CREATE_FIELD")));
	}

	if(pWnd == NULL)
	{
		return;
	}
	pWnd->Visible(true);

	pWnd->SetCustomData(&kMission.m_iMissionKey, sizeof(kMission.m_iMissionKey));	//미션 번호 저장
	XUI::CXUI_Wnd* pkWndProg = pWnd->GetControl(std::wstring(_T("FRM_PROG")));
	if (pkWndProg)
	{
		BM::vstring kProg(((int)kMission.m_kTotalPercent));
		kProg+=_T("%");
		pkWndProg->Text((std::wstring)(kProg));
	}

	char szWndName[255] = {0,};
	int const iDiff = MILT_LEVLE2; // MAX_MISSION_LEVEL;
	for (int i = 0; i < iDiff; ++i)
	{
		sprintf_s(szWndName, 255, "FRM_CARD%d", i);
		XUI::CXUI_Wnd* pkWndCard = pWnd->GetControl(UNI(szWndName));
		if (pkWndCard)
		{
			BYTE btDoor = kMission.m_kDoor[i];

			if (SMissionInfo::MDOOR_NONE!=btDoor)
			{
				pkWndCard->SetCustomData(&i, sizeof(i));
				for (int j = 0; j < iDiff; ++j)
				{
					sprintf_s(szWndName, 255, "IMG_STAR%d", j);
					XUI::CXUI_Wnd* pkWndStar = pkWndCard->GetControl(UNI(szWndName));
					if (pkWndStar)
					{
						pkWndStar->Visible(i >= j);
					}
				}
				
				XUI::CXUI_Wnd* pkWndCardBG = pkWndCard->GetControl(std::wstring(_T("IMG_BG")));
				XUI::CXUI_Wnd* pkWndCardBG1 = pkWndCard->GetControl(std::wstring(_T("IMG_BG1")));
				if (pkWndCardBG)
				{
					XUI::CXUI_Wnd* pkWndCardClear = pkWndCardBG->GetControl(std::wstring(_T("IMG_CLEAR")));
					if (pkWndCardClear)
					{
						pkWndCardClear->Visible(SMissionInfo::MDOOR_CLEARED==btDoor);
					}
					pkWndCardBG->GrayScale(SMissionInfo::MDOOR_CLOSE==btDoor);
					if( pkWndCardBG1 ) 
					{
						pkWndCardBG1->GrayScale(SMissionInfo::MDOOR_CLOSE==btDoor);
					}
				}

				XUI::CXUI_Wnd* pkWndCardNew = pkWndCard->GetControl(std::wstring(_T("IMG_NEW")));
				if (pkWndCardNew)
				{
					pkWndCardNew->Visible(SMissionInfo::MDOOR_NEWOPEN==btDoor);
				}

			}
		}
	}

	XUI::CXUI_Wnd* pkBgImg = pWnd->GetControl(L"FRM_BG_ITEM");
	XUI::CXUI_Builder* pkBuildCard = dynamic_cast<XUI::CXUI_Builder*>(pWnd->GetControl(L"BLD_CARD1"));

	if( pkBgImg )
	{
		XUI::CXUI_Wnd* pkText = pkBgImg->GetControl(L"FRM_TEXT1");
		if( pkText )
		{
			pkText->Text(TTW(400953));
		}
	}

	if( pkBuildCard )
	{
		for(int i=0; i<pkBuildCard->CountX();++i)
		{
			sprintf_s(szWndName, 255, "FRM_ITEMTEXT%d", i);
			XUI::CXUI_Wnd* pkItemText = pWnd->GetControl(UNI(szWndName));
			if(pkItemText)
			{
				pkItemText->Visible(!g_bAutoLevel);
			}
		}
	}
}

void Mission_PartyList_Info(SMissionInfo& Mission)
{
	XUI::CXUI_Wnd* pTopWnd = XUIMgr.Get(L"FRM_CREATE_SCENARIO_ITEM");
	if( NULL == pTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* psWnd = pTopWnd->GetControl(L"FRM_CREATE_FIELD_SCENARIO");
	if( NULL == psWnd )
	{
		return;
	}

	CONT_DEF_MISSION_ROOT const * pContDefMission = NULL;
	g_kTblDataMgr.GetContDef(pContDefMission);
	if( NULL == pContDefMission )
	{
		return;
	}

	CONT_DEF_MISSION_CANDIDATE const * pContDefMissionCandi = NULL;	// 미션 Candidate 테이블 가져오기
	g_kTblDataMgr.GetContDef(pContDefMissionCandi);
	if ( NULL == pContDefMissionCandi )
	{
		return;
	}

	CONT_MISSION_BONUSMAP const* pContMissionBonusMap = NULL;
	g_kTblDataMgr.GetContDef(pContMissionBonusMap);
	if( NULL == pContMissionBonusMap )
	{
		return;
	}

	// 미션번호로 해당 미션 찾기
	int const ACADE_MISSION_MODE = 2;
	int RootLevelValue[ACADE_MISSION_MODE] = {0};
	CONT_DEF_MISSION_ROOT::const_iterator mission_itor = pContDefMission->find(Mission.m_iMissionNo);
	if( pContDefMission->end() != mission_itor )
	{// 미션의 각 맵의 번호가 저장된 Candidate에 접근하기 위한 키값을 찾는다.
		RootLevelValue[0] = mission_itor->second.aiLevel[0]; // 아케이드
		RootLevelValue[1] = mission_itor->second.aiLevel[1]; // 카오스
	}

	if ( 0 == RootLevelValue[0] )
	{
		return;
	}

	std::vector<int> ContMapNo;
	for( int Mode = 0; Mode < ACADE_MISSION_MODE; ++Mode )
	{
		// 키 값으로 유저가 선택한 미션의 Candidate에 접근
		CONT_DEF_MISSION_CANDIDATE::const_iterator candi_itr = pContDefMissionCandi->find(RootLevelValue[Mode]);
		if( pContDefMissionCandi->end() != candi_itr )
		{
			for( int candi_index = 0; candi_index < 6; ++candi_index )
			{
				if( candi_itr->second.aiCandidate[candi_index] )
				{
					ContMapNo.push_back(candi_itr->second.aiCandidate[candi_index]);
				}
			}
		}
		
		CONT_MISSION_BONUSMAP::const_iterator bonus_itr = pContMissionBonusMap->find(RootLevelValue[Mode]);
		if( pContMissionBonusMap->end() != bonus_itr )
		{
			if( bonus_itr->second.iBonusMap1 )
			{
				ContMapNo.push_back(bonus_itr->second.iBonusMap1);
			}
			if( bonus_itr->second.iBonusMap2 )
			{
				ContMapNo.push_back(bonus_itr->second.iBonusMap2);
			}
		}
	}

	XUI::CXUI_Wnd * pPartyWnd = XUIMgr.Call(L"SFRM_DUNGEON_PARTY");
	if( NULL == pPartyWnd )
	{
		return;
	}

	XUI::CXUI_Wnd * pRefreshBtn = pPartyWnd->GetControl(L"BTN_REFRESH");
	if( NULL == pRefreshBtn )
	{
		return;
	}

	BM::Stream Packet;
	Packet.Push(ContMapNo.size());
	std::vector<int>::const_iterator MapNo_iter = ContMapNo.begin();
	for( ; MapNo_iter != ContMapNo.end(); ++MapNo_iter )
	{
		Packet.Push(*MapNo_iter);
	}

	pRefreshBtn->SetCustomData(Packet);

	lua_tinker::call<void,lwUIWnd>("OnCall_MissionEnterParty", lwUIWnd(pTopWnd));
}

void Mission_Hidden_Info(SMissionInfo& kMission)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Call(std::wstring(_T("FRM_CREATE_SCENARIO_ITEM")));
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD_SCENARIO"));

	if(pWnd == NULL)
	{
		return;
	}

	XUI::CXUI_Wnd* pTextWnd = pWnd->GetControl(_T("FRM_BG_ITEM"));

	if( !pTextWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pTextWnd0 = pTextWnd->GetControl(_T("FRM_TEXT"));
	if( pTextWnd0 )
	{
		pTextWnd0->Text(_T(""));
	}
	XUI::CXUI_Wnd* pTextWnd1 = pTextWnd->GetControl(_T("FRM_TEXT1"));
	if( pTextWnd1 )
	{
		pTextWnd1->Text(_T(""));
	}
	XUI::CXUI_Wnd* pStarWnd0 = pTextWnd->GetControl(_T("IMG_STAR0"));
	if( pStarWnd0 )
	{
		pStarWnd0->Visible(false);
	}
	XUI::CXUI_Wnd* pStarWnd1 = pTextWnd->GetControl(_T("IMG_STAR1"));
	if( pStarWnd1 )
	{
		pStarWnd1->Visible(false);
	}

	XUI::CXUI_Wnd* pItemWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD"));
	if( !pItemWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pTextWnd2 = pItemWnd->GetControl(_T("FRM_BG_ITEM"));

	if( !pTextWnd2 )
	{
		return;
	}

	XUI::CXUI_Wnd* pTextWnd3 = pTextWnd2->GetControl(_T("FRM_TEXT"));
	if( pTextWnd3 )
	{
		pTextWnd3->Text(_T(""));
	}
	XUI::CXUI_Wnd* pTextWnd4 = pTextWnd2->GetControl(_T("FRM_TEXT1"));
	if( pTextWnd4 )
	{
		pTextWnd4->Text(_T(""));
	}

	XUI::CXUI_Wnd* pProgWnd = pItemWnd->GetControl(_T("FRM_PROG"));
	if( pProgWnd )
	{
		pProgWnd->Visible(false);
	}	

	XUI::CXUI_Wnd* pLine1 = pItemWnd->GetControl(_T("IMG_LINE"));
	if( pLine1 )
	{
		pLine1->Visible(false);
	}
	XUI::CXUI_Wnd* pLine2 = pItemWnd->GetControl(_T("IMG_LINE2"));
	if( pLine2 )
	{
		pLine2->Visible(false);
	}
	XUI::CXUI_Wnd* pLine3 = pItemWnd->GetControl(_T("IMG_TEXT_BG"));
	if( pLine3 )
	{
		pLine3->Visible(false);
	}
}

void Mission_Item_Info_Event(SMissionInfo& kMission)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Activate( _T("FRM_CREATE_EVENT_ITEM") );
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD"));
	
	if(pWnd == NULL)
	{
		pWnd = XUIMgr.Call(std::wstring(_T("FRM_CREATE_FIELD")));
	}

	if(pWnd == NULL)
	{
		return;
	}
	pWnd->Visible(true);

	pWnd->SetCustomData(&kMission.m_iMissionKey, sizeof(kMission.m_iMissionKey));	//미션 번호 저장
	XUI::CXUI_Wnd* pkWndProg = pWnd->GetControl(std::wstring(_T("FRM_PROG")));
	if (pkWndProg)
	{
		BM::vstring kProg(((int)kMission.m_kTotalPercent));
		kProg+=_T("%");
		pkWndProg->Text((std::wstring)(kProg));
	}

	char szWndName[255] = {0,};
	int const iDiff = MAX_MISSION_LEVEL;
	for (int i = 0; i < iDiff; ++i)
	{
		sprintf_s(szWndName, 255, "FRM_CARD%d", i);
		XUI::CXUI_Wnd* pkWndCard = pWnd->GetControl(UNI(szWndName));
		if (pkWndCard)
		{
			BYTE const btDoor = kMission.m_kDoor[i];

			XUI::CXUI_Wnd* pkWndCardBG = pkWndCard->GetControl(std::wstring(_T("IMG_BG")));
			if( pkWndCardBG )
			{
				if( i == 0 )
				{
					XUI::CXUI_Wnd* pkWndCardClear = pkWndCardBG->GetControl(std::wstring(_T("IMG_CLEAR")));
					if (pkWndCardClear)
					{
						pkWndCardClear->Visible(SMissionInfo::MDOOR_CLEARED==kMission.m_kDoor[4]);
					}
				}
				pkWndCardBG->GrayScale(SMissionInfo::MDOOR_CLOSE==kMission.m_kDoor[4]);
			}

			if (SMissionInfo::MDOOR_NONE!=btDoor)
			{
				pkWndCard->SetCustomData(&i, sizeof(i));
				for (int j = 0; j < iDiff; ++j)
				{
					sprintf_s(szWndName, 255, "IMG_STAR%d", j);
					XUI::CXUI_Wnd* pkWndStar = pkWndCard->GetControl(UNI(szWndName));
					if (pkWndStar)
					{
						pkWndStar->Visible(i >= j);
					}
				}

				XUI::CXUI_Wnd* pkWndCardBG = pkWndCard->GetControl(std::wstring(_T("IMG_BG")));
				if (pkWndCardBG)
				{
					XUI::CXUI_Wnd* pkWndCardClear = pkWndCardBG->GetControl(std::wstring(_T("IMG_CLEAR")));
					if (pkWndCardClear)
					{
						pkWndCardClear->Visible(SMissionInfo::MDOOR_CLEARED==btDoor);
					}
					pkWndCardBG->GrayScale(SMissionInfo::MDOOR_CLOSE==btDoor);
				}

				XUI::CXUI_Wnd* pkWndCardNew = pkWndCard->GetControl(std::wstring(_T("IMG_NEW")));
				if (pkWndCardNew)
				{
					pkWndCardNew->Visible(SMissionInfo::MDOOR_NEWOPEN==btDoor);
				}
			}
		}
	}
}

void Mission_Scenario_Not_Have_Arcade_Info(SMissionInfo& kMission)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Call(std::wstring(_T("FRM_CREATE_SCENARIO_ITEM")));
	if( !pkTopWnd )
	{
		return;
	}
	XUI::CXUI_Wnd* pWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD_SCENARIO"));
	if( !pWnd )
	{
		return;
	}
	XUI::CXUI_Wnd* pWndDefence = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD_DEFENCE"));
	if(!pWndDefence)
	{
		return;
	}

	pWnd->SetCustomData(&kMission.m_iMissionKey, sizeof(kMission.m_iMissionKey));	//미션 번호 저장

	int TextNo = 0;
	bool const IsOpenF6 = lwIsOpenDefenseModeTable(EDFST_F6, kMission.m_iMissionNo);
	bool const IsOpenF7 = lwIsOpenDefenseModeTable(EDFST_F7, kMission.m_iMissionNo);
	if( !IsOpenF6 && !IsOpenF7 )
	{// 디펜스 모드가 없으면
		return;
	}
	else
	{
		XUI::CXUI_Wnd* pCardWnd = pWndDefence->GetControl(_T("FRM_CARD0"));
		if(!pCardWnd)
		{
			return;
		}
		XUI::CXUI_Wnd* pDefenceBgImg = pCardWnd->GetControl(_T("IMG_BG"));
		if( !pDefenceBgImg )
		{
			return;
		}
		
		pDefenceBgImg->GrayScale(false); // 입장 조건이 없기 때문에 무조건 활성화
		pWndDefence->SetCustomData(&kMission.m_iMissionNo, sizeof(kMission.m_iMissionNo));

		if( IsOpenF6 && IsOpenF7 )
		{// 둘 다 열려 있는 미션이면 확장버튼 추가
			lua_tinker::call< void, lwUIWnd, int >("SetDefenceModeEnterImg", lwUIWnd(pCardWnd), 6);
			lua_tinker::call< void, lwUIWnd, int >("SetDefanceInfoText", lwUIWnd(pCardWnd), 6);

			TextNo = 401171; // TEXT : 디펜스 / 전략 모드
		}
		else if( IsOpenF6 )
		{// F6만 있는 미션
			lua_tinker::call< void, lwUIWnd, int >("SetDefenceModeEnterImg", lwUIWnd(pCardWnd), 6);
			lua_tinker::call< void, lwUIWnd, int >("SetDefanceInfoText", lwUIWnd(pCardWnd), 6);

			TextNo = 400956; // TEXT : 디펜스 모드
		}
		else if( IsOpenF7 )
		{// F7만 있는 미션
			lua_tinker::call< void, lwUIWnd, int >("SetDefenceModeEnterImg", lwUIWnd(pCardWnd), 7);
			lua_tinker::call< void, lwUIWnd, int >("SetDefanceInfoText", lwUIWnd(pCardWnd), 7);

			TextNo = 400931; // TEXT : 전략 모드
		}

		XUI::CXUI_Wnd* pTmp = pWndDefence->GetControl(L"FRM_TITLE");
		pTmp = pTmp ? pTmp->GetControl(L"FRM_TEXT") : NULL;
		if(pTmp)
		{
			pTmp->Text(TTW(TextNo));
		}

		{// 퀘스트 표시
			XUI::CXUI_Wnd* pQuestText = pCardWnd->GetControl(_T("FRM_QUEST"));
			if( !pQuestText )
			{
				return;
			}
			CONT_DEF_QUEST_REWARD const* pkDefReward = NULL;
			g_kTblDataMgr.GetContDef(pkDefReward);
			if( !pkDefReward )
			{
				return;
			}

			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( pkPlayer )
			{
				PgMyQuest const* pkUserQuest = pkPlayer->GetMyQuest();
				if( pkUserQuest )
				{
					if( !IsOpenF6 && !IsOpenF7 )
					{// 디펜스 모드가 없으면
						return;
					}
					else
					{
						int ClearQuestNo = 0;
						std::wstring ClearQuestText;

						if( IsOpenF6 && IsOpenF7 )
						{// 둘 다 열려 있는 미션
						}
						else if( IsOpenF6 )
						{// F6만 있는 미션
							ClearQuestNo = g_kMissionMgr.GetClearQuestNo(DEFENCE_MISSION_LEVEL - 1);
							ClearQuestText = g_kMissionMgr.GetClearQuestOriginalText(DEFENCE_MISSION_LEVEL - 1);
						}
						else if( IsOpenF7 )
						{// F7만 있는 미션
							ClearQuestNo = g_kMissionMgr.GetClearQuestNo(DEFENCE7_MISSION_LEVEL - 1);
							ClearQuestText = g_kMissionMgr.GetClearQuestOriginalText(DEFENCE7_MISSION_LEVEL - 1);
						}
						if( ClearQuestNo )
						{
							BM::vstring vStr;
							vStr = ClearQuestText;
							bool bGrayScale = false;

							CONT_DEF_QUEST_REWARD::const_iterator iter = pkDefReward->find(ClearQuestNo);
							if( iter != pkDefReward->end() )
							{
								if( pkUserQuest->IsEndedQuest(ClearQuestNo) )
								{
									pQuestText->FontColor(0xFF42FF00);
									vStr += TTW(799381);
									bGrayScale = false;
								}
								else
								{
									pQuestText->FontColor(0xFFFFFFFF);
									vStr += L"(";
									vStr += TTW(224);
									vStr += L".";
									vStr += iter->second.iLevelMin;
									vStr += L")";
									bGrayScale = true;
								}
							}
							if( g_pkApp->VisibleClassNo() )
							{
								vStr += L"(";
								vStr += iter->second.iQuestID;
								vStr += L")";
							}
							pQuestText->Text( vStr );
							pQuestText->Visible( true );
							pDefenceBgImg->GrayScale( bGrayScale );
						}
					}
				}
			}
		}
	}
}

void Mission_Item_Not_Have_Arcade_Info(SMissionInfo& kMission)
{
	XUI::CXUI_Wnd* pkTopWnd = lwGetUIWnd("FRM_CREATE_SCENARIO_ITEM").GetSelf();
	if( !pkTopWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* psWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD_SCENARIO"));
	if( !psWnd )
	{
		return;
	}

	XUI::CXUI_Wnd* pBgImg1 = psWnd->GetControl(_T("FRM_BG_ITEM"));
	XUI::CXUI_Wnd* pBgImg2 = psWnd->GetControl(_T("FRM_BG_ITEM_AUTO"));
	XUI::CXUI_Wnd* pItemText = psWnd->GetControl(_T("FRM_ITEM_AUTO_TEXT"));
	XUI::CXUI_Wnd* pBgNotHaveArcade = psWnd->GetControl(_T("FRM_BG_EVENT_NOT_HAVE_ARCADE"));
	if(!pBgImg1 || !pBgImg2 || !pItemText || !pBgNotHaveArcade)
	{
		return;
	}

	pBgImg1->Visible(false);
	pBgImg2->Visible(false);
	pItemText->Visible(false);
	pBgNotHaveArcade->Visible(true);

	XUI::CXUI_Wnd* pkWndProg = psWnd->GetControl(std::wstring(_T("SFRM_BOSS_BG")));
	if( pkWndProg )
	{
		XUI::CXUI_Wnd* pkImg = pkWndProg->GetControl(_T("IMG_BOSS"));
		if (pkImg)
		{
			char szName[255] = {0,};
			_snprintf(szName, 255, "../Data/6_ui/mission/msBossImg_%d.tga", kMission.m_iMissionNo);
			if(PgNiFile::CheckFileExist(szName))
			{
				pkImg->DefaultImgName(UNI(szName));
				pkImg->Visible(true);
			}
			else
			{
				pkImg->Visible(false);
			}
		}
	}

	int const iDiff = MAX_MISSION_LEVEL;
	for (int i = 0; i < iDiff; ++i)
	{
		BM::vstring vStr("FRM_CARD");
		vStr += i;
		XUI::CXUI_Wnd* pkWndCard = psWnd->GetControl(vStr);
		if (pkWndCard)
		{
			pkWndCard->Visible(false);
		}
	}


	XUI::CXUI_Wnd* pDefenceWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD_DEFENCE"));
	if( pDefenceWnd )
	{
		pDefenceWnd->Visible(true);
		XUI::CXUI_Wnd* pBgWnd = pDefenceWnd->GetControl(_T("FRM_BG_ITEM"));
		if( pBgWnd )
		{
			pBgWnd->Visible(false);
		}
		XUI::CXUI_Wnd* pTitleWnd = pDefenceWnd->GetControl(_T("FRM_TITLE"));
		if( pTitleWnd )
		{
			pTitleWnd->Visible(true);
		}
	}

	XUI::CXUI_Wnd* pWnd = pkTopWnd->GetControl(_T("FRM_CREATE_FIELD"));
	if( pWnd )
	{
		pWnd->Visible(false);
	}
	return;
}

char const* lwGetMissionImgPath(int const iNum)
{
	std::string pkPath = g_kUIScene.GetMissionMapPath(iNum);

	std::string kWString = "../Data/6_ui/mission/ms010100_";

	if (!pkPath.empty())
	{
		kWString = pkPath;
	}

	static char szString[255] = {0,};
	szString[0] = '\0';
	strcpy_s(szString, sizeof(szString), kWString.c_str());

	return szString;
}

int lwGetMissionKey( int const iMissionNo )
{
	CONT_DEF_MISSION_ROOT const *pkMissionRoot = NULL;
	g_kTblDataMgr.GetContDef( pkMissionRoot );
	
	if ( pkMissionRoot )
	{
		CONT_DEF_MISSION_ROOT::const_iterator itr = pkMissionRoot->find( iMissionNo );
		if ( itr != pkMissionRoot->end() )
		{
			return itr->second.iKey;
		}
	}

	return -1;
}

void lwSendMemo(char const* pkStr)
{
	g_kMissionComplete.SendMemo(pkStr);
}

void lwSendReqRankInfo()
{
	g_kMissionComplete.SendReqRankInfo();
}

int lwMissionRankingRewardItem( const int iMissionNo, const int iMissionLevel, const int iMissionRanking )
{	
	CONT_MISSION_RANK_REWARD_BAG::const_iterator item_itr;
	const CONT_MISSION_RANK_REWARD_BAG *pkContItem = NULL;
	g_kTblDataMgr.GetContDef( pkContItem );
	int iRewardItemNo = 0;

	if( !pkContItem )
	{
		return 0;
	}
	else
	{
		CONT_MISSION_RANK_REWARD_BAG::key_type kKey( iMissionNo, iMissionLevel, iMissionRanking );

		item_itr = pkContItem->find( kKey );
		if( item_itr == pkContItem->end() )
		{
			return 0;
		}
		else
		{	// 현재는 각 등수마다 아이템이 정해져 있어서 kCont에 아이템이 하나밖에 안들어있다.
			// 그래서 그냥 begin() 으로 가져오는데, 나중에 여러개의 아이템이 등록되면 확률로 정해줘야 하는데..
			// 그럼 미션 시작될 때 보상 아이템을 결정해서 보여줘야해? - _-;;
			CONT_MISSION_RANK_REWARD_BAG::mapped_type const &kElement = (*item_itr).second;
			CONT_MISSION_RANK_REWARD_ITEM::const_iterator iter = kElement.kCont.begin();

			if( iter != kElement.kCont.end() )
			{
				iRewardItemNo = (*iter).iItemNo;
			}			
		}
	}

	return iRewardItemNo;
}

void lwSetMissionCountMap(int const iCount)
{
	g_iMissionMapCount = iCount;
}

void lwSetMissionDifficultyLevel(int const iLevel)
{
	g_iMissionDifficultyLevel = iLevel;
}

void lwSetScenarioMissionMapCount(int const iNum, int const iDiff)
{
	const CONT_DEF_MISSION_ROOT* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);

	g_iMissionNo = iNum;
	g_iMissionMapCount = 5;

	if (pContDefMap)
	{
		CONT_DEF_MISSION_ROOT::const_iterator root_itr = pContDefMap->find(iNum);
		if (pContDefMap->end() != root_itr)
		{
			TBL_DEF_MISSION_ROOT const& element = root_itr->second;
			const CONT_DEF_MISSION_CANDIDATE * pContDefMapMis = NULL;
			g_kTblDataMgr.GetContDef(pContDefMapMis);
			if (pContDefMapMis)
			{
				CONT_DEF_MISSION_CANDIDATE::const_iterator cand_it = pContDefMapMis->find(element.aiLevel[__max(0,iDiff-1)]);
				if (pContDefMapMis->end() != cand_it)
				{
					const TBL_DEF_MISSION_CANDIDATE& rkCand = (*cand_it).second;
					g_iMissionMapCount = rkCand.iUseCount;
				}
			}
		}
	}
}

int lwCalcMissionCount(int const iNum, int const iDiff)
{
	/*const CONT_DEF_MISSION_ROOT* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);

	g_iMissionNo = iNum;
	g_iMissionMapCount = 5;

	if (pContDefMap)
	{
		CONT_DEF_MISSION_ROOT::const_iterator root_itr = pContDefMap->find(iNum);
		if (pContDefMap->end() != root_itr)
		{
			TBL_DEF_MISSION_ROOT const& element = root_itr->second;
			const CONT_DEF_MISSION_CANDIDATE * pContDefMapMis = NULL;
			g_kTblDataMgr.GetContDef(pContDefMapMis);
			if (pContDefMapMis)
			{
				CONT_DEF_MISSION_CANDIDATE::const_iterator cand_it = pContDefMapMis->find(element.aiLevel[__max(0,iDiff-1)]);
				if (pContDefMapMis->end() != cand_it)
				{
					const TBL_DEF_MISSION_CANDIDATE& rkCand = (*cand_it).second;
					g_iMissionMapCount = rkCand.iUseCount;
				}
			}
		}
	}*/

	return g_iMissionMapCount;
}

lwWString lwSetMissionSelectMapName(int const iLevel, int const iIndex)
{
	const CONT_DEF_MISSION_ROOT* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);

	g_iMissionNo = lwGetMissionNo();
	if (pContDefMap)
	{
		CONT_DEF_MISSION_ROOT::const_iterator root_itr = pContDefMap->find(g_iMissionNo);
		if (pContDefMap->end() != root_itr)
		{
			TBL_DEF_MISSION_ROOT const& element = root_itr->second;
			const CONT_DEF_MISSION_CANDIDATE * pContDefMapMis = NULL;
			g_kTblDataMgr.GetContDef(pContDefMapMis);
			if (pContDefMapMis)
			{
				CONT_DEF_MISSION_CANDIDATE::const_iterator cand_it = pContDefMapMis->find(element.aiLevel[__max(0, iLevel)]);
				if (pContDefMapMis->end() != cand_it)
				{
					const TBL_DEF_MISSION_CANDIDATE& rkCand = (*cand_it).second;
					if( MAX_MISSION_CANDIDATE > iIndex )
					{
						int const iGroundNo = rkCand.aiCandidate[iIndex];
						if( 0 < iGroundNo )
						{
							return lwWString(GetMapName(iGroundNo));
						}
					}
				}
			}
		}
	}
	return lwWString(std::wstring());
}

void lwSend_PT_C_N_REQ_MISSION_RESTART()
{
	//if (g_kMissionComplete.IsOwner())
	bool bIsPartyMaster = false;
	PgPilot* pkMyPilot = g_kPilotMan.GetPlayerPilot();	
	if( pkMyPilot )
	{
		bIsPartyMaster = (g_kParty.MasterGuid() == pkMyPilot->GetGuid());
	}
	if( bIsPartyMaster || g_kMissionComplete.IsOwner() )
	{
		BM::Stream kPacket(PT_C_M_REQ_MISSION_RESTART);
		NETWORK_SEND(kPacket)
	}

	g_kMissionComplete.CompleteState(PgMissionComplete::E_MSN_CPL_SELECT_NEXT);
}

lwUIWnd lwCallMissionRetryUI()
{
	return lwUIWnd(g_kMissionComplete.CallRetryUI());
}

bool lwCallMissionRebirthUI(bool bUseCoin)
{
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(!pkUnit)
	{
		return false;	
	}

	int iText = 80023;
	if(bUseCoin)
	{	
		PgInventory *pkInv = pkUnit->GetInven();
		if(pkInv)
		{
			PgUtilAction_HaveReviveItem kAction( pkInv, pkUnit->GetAbil(AT_LEVEL) );
			if( true == kAction.IsHave( IT_CONSUME )
			||	true == kAction.IsHave( IT_CASH ) )
			{
				if( NULL == XUIMgr.Get(_T("SFRM_REBIRTH_REQ_BY_COIN")))
				{
					XUIMgr.Call(_T("SFRM_REBIRTH_REQ_BY_COIN"));
				}

				return true;	
			}
			else
			{
				lua_tinker::call<void, int, int>("OnCallStaticCashItemBuy", 6, pkUnit->GetAbil(AT_LEVEL));
				return false;
			}
		}
	}

	int iCustomData = iText == 80030 ? 1 : 0;

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(_T("SFRM_REBIRTH_REQ"));
	if (pkWnd)
	{
		pkWnd->SetCustomData(&iCustomData, sizeof(iCustomData));//코인으로 부활할지 돈으로 부활할지
		XUI::CXUI_Wnd* pkColor = pkWnd->GetControl(_T("SFRM_COLOR"));
		if (pkColor)
		{
			pkColor->Text(TTW(iText));
		}
		
		XUI::CXUI_Wnd* pkGoldWnd = pkWnd->GetControl(_T("SFRM_GOLD"));
		if (pkGoldWnd)
		{
			XUI::CXUI_Wnd* pkGold = pkGoldWnd->GetControl(_T("FRM_GOLD"));
			XUI::CXUI_Wnd* pkSilver= pkGoldWnd->GetControl(_T("FRM_SILVER"));
			XUI::CXUI_Wnd* pkCopper = pkGoldWnd->GetControl(_T("FRM_COPPER"));
			if( pkGold && pkSilver && pkCopper)
			{
				__int64 const iMoney = pkUnit->GetAbil64(AT_MONEY);
				GET_DEF(PgDefSpendMoneyMgr, kDefSpendMoneyMgr);
				int iRevMoney = kDefSpendMoneyMgr.GetSpendMoney(ESMT_REVIVE_MISSION,pkUnit->GetAbil(AT_LEVEL));
				int const iOriginMoney = iRevMoney;

				int const iGold = iRevMoney/10000;
				if (0<iGold)
				{
					BM::vstring kText(iGold);
					pkGold->Text(static_cast<std::wstring>(kText));
					iRevMoney -= iGold*10000;
				}
				int const iSilver = iRevMoney/100;
				if (0<iSilver)
				{
					BM::vstring kText(iSilver);
					pkSilver->Text(static_cast<std::wstring>(kText));
					iRevMoney -= iSilver*100;
				}
				int const iCopper = iRevMoney;
				if (0<iCopper)
				{
					BM::vstring kText(iCopper);
					pkCopper->Text(static_cast<std::wstring>(kText));
				}
				if (iMoney<iOriginMoney)
				{
					BM::vstring vString(TTW(80022));
					vString+=iGold;
					vString+=TTW(401111);
					vString+=iSilver;
					vString+=TTW(401112);
					vString+=iCopper;
					vString+=TTW(401113);
					lwAddWarnDataStr(lwWString((std::wstring const&)vString), 0);
					return false;
				}
			}
		}
	}

	return true;
}

void lwSend_REQ_DEFENCE_INFALLIBLE_SELECTION(BYTE const kDirection)
{
	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("FRM_MISSION_SELECT_DIRECTION"));
	if( !pkWndTop )
	{
		return;
	}
	XUI::CXUI_Wnd* pkWndItemLeft = pkWndTop->GetControl(L"BTN_USE_ITEM_LEFT");
	if( !pkWndItemLeft )
	{
		return;
	}
	XUI::CXUI_Wnd* pkWndItemRight = pkWndTop->GetControl(L"BTN_USE_ITEM_RIGHT");
	if( !pkWndItemRight )
	{
		return;
	}
	XUI::CXUI_Wnd* pkWndIcon = pkWndItemLeft->GetControl(L"ICN_SRC");
	if( !pkWndIcon )
	{
		return;
	}

	DWORD dwItemNo = 0;
	pkWndIcon->GetCustomData(&dwItemNo, sizeof(dwItemNo));

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	SItemPos kItemPos;
	if( S_OK != pkInv->GetFirstItem(dwItemNo, kItemPos) )
	{//아이템을 찾을 수 없습니다
		::Notice_Show_ByTextTableNo(403103, EL_Warning);
		return;
	}

	if(kDirection == D_LEFT)
	{
		pkWndItemRight->Enable(false);
	}
	else if(kDirection == D_RIGHT)
	{
		pkWndItemLeft->Enable(false);
	}
	
	BM::Stream kPacket(PT_C_M_REQ_DEFENCE_INFALLIBLE_SELECTION);
	kPacket.Push(kDirection);
	kPacket.Push(kItemPos);
	NETWORK_SEND(kPacket);
}

void lwDisplayIcon_InfallibleSelection()
{
	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("FRM_MISSION_SELECT_DIRECTION"));
	if( !pkWndTop ) return;
	XUI::CXUI_Wnd* pkWndItemLeft = pkWndTop->GetControl(L"BTN_USE_ITEM_LEFT");
	if( !pkWndItemLeft ) return;
	XUI::CXUI_Wnd* pkWndIconLeft = pkWndItemLeft->GetControl(L"ICN_SRC");
	if( !pkWndIconLeft ) return;

	XUI::CXUI_Wnd* pkWndItemRight = pkWndTop->GetControl(L"BTN_USE_ITEM_RIGHT");
	if( !pkWndItemRight ) return;
	XUI::CXUI_Wnd* pkWndIconRight = pkWndItemRight->GetControl(L"ICN_SRC");
	if( !pkWndIconRight ) return;

	DWORD dwItemNo = 0;
	pkWndIconLeft->GetCustomData(&dwItemNo, sizeof(dwItemNo));

	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return;
	}

	bool bGray = false;
	if( !pkInv->GetTotalCount(dwItemNo) )
	{//아이템 없다면 그레이 스케일 적용
		bGray = true;
	}

	POINT2 kPtLocationLeft =	pkWndIconLeft->TotalLocation();
	POINT2 kPtLocationRight = pkWndIconRight->TotalLocation();

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(dwItemNo);
	if(pItemDef)
	{
		g_kUIScene.RenderIcon( pItemDef->ResNo(), kPtLocationLeft, false, 40, 40, bGray );
		g_kUIScene.RenderIcon( pItemDef->ResNo(), kPtLocationRight, false, 40, 40, bGray );
	}
}

lwWString lwMissionClearQuestText(int const iLevel)
{
	return lwWString(g_kMissionMgr.GetClearQuestText(iLevel));
}

void lwUpdate_DefenceSelectUI(BM::Stream &rkPacket)
{
	CONT_USER_DIRECTION kContUserDirection;
	if( !PU::TLoadTable_AA(rkPacket, kContUserDirection) )
	{
		return;
	}
	bool bCloseSelectDirection = false;
	bool bUsedInfallibleItem = false;
	rkPacket.Pop(bCloseSelectDirection);
	rkPacket.Pop(bUsedInfallibleItem);

	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("FRM_MISSION_SELECT_DIRECTION"));
	if( !pkWndTop ) return;
	XUI::CXUI_Wnd* pkWndLeftArrowCount = pkWndTop->GetControl(L"FRM_LEFT_ARROW_COUNT");
	if( !pkWndLeftArrowCount ) return;
	XUI::CXUI_Wnd* pkWndLeftCountImg = pkWndLeftArrowCount->GetControl(L"IMG_NUMBER");
	if( !pkWndLeftCountImg ) return;
	XUI::CXUI_Wnd* pkWndLeftArrowSelect = pkWndTop->GetControl(L"IMG_LEFT_ARROW_SELECT");
	if( !pkWndLeftArrowSelect ) return;
	XUI::CXUI_Wnd* pkWndLeftArrow = pkWndTop->GetControl(L"IMG_LEFT_ARROW");
	if( !pkWndLeftArrow ) return;
	XUI::CXUI_Wnd* pkWndRightArrowCount = pkWndTop->GetControl(L"FRM_RIGHT_ARROW_COUNT");
	if( !pkWndRightArrowCount ) return;
	XUI::CXUI_Wnd* pkWndRightCountImg = pkWndRightArrowCount->GetControl(L"IMG_NUMBER");
	if( !pkWndRightCountImg ) return;
	XUI::CXUI_Wnd* pkWndRightArrowSelect = pkWndTop->GetControl(L"IMG_RIGHT_ARROW_SELECT");
	if( !pkWndRightArrowSelect ) return;
	XUI::CXUI_Wnd* pkWndRightArrow = pkWndTop->GetControl(L"IMG_RIGHT_ARROW");
	if( !pkWndRightArrow ) return;
	XUI::CXUI_Wnd* pkWndGo = pkWndTop->GetControl(L"BTN_GO");
	if( !pkWndGo ) return;

	bool bIsPartyMaster = false;
	PgPilot* pkMyPilot = g_kPilotMan.GetPlayerPilot();	
	if( pkMyPilot )
	{
		bIsPartyMaster = (g_kParty.MasterGuid() == pkMyPilot->GetGuid());
	}

	int const iPartyMemberCount = std::max<int>( 1, g_kParty.MemberCount() );
	bool bEnableBtn = false, bVisibleBtn = false;
	if( bIsPartyMaster || (1==iPartyMemberCount) || (1==kContUserDirection.size()&&bCloseSelectDirection))
	{//파티장만 버튼이 나옴.
		bVisibleBtn = true;
		
		if( true == bCloseSelectDirection )
		{//버튼 활성화
			bEnableBtn = true;
		}
	}

	if(true == bUsedInfallibleItem && true == bIsPartyMaster)
	{
		bVisibleBtn = true;
		bEnableBtn = true;
	}

	pkWndGo->Visible(bVisibleBtn);
	pkWndGo->Enable(bEnableBtn);

	int iLeft = 0, iRight = 0;
	CONT_USER_DIRECTION::const_iterator direction_itor = kContUserDirection.begin();
	while( kContUserDirection.end() != direction_itor )
	{
		EDefenceMonsterGen eDirection = static_cast<EDefenceMonsterGen>(direction_itor->second);
		switch(eDirection)
		{
		case D_LEFT: { ++iLeft; }break;
		case D_RIGHT: { ++iRight; }break;
		case D_RANDOM: {} break;
		}
		++direction_itor;
	}

	if( iLeft > iRight )
	{
		pkWndLeftArrowSelect->Visible(true);
		pkWndLeftArrow->Visible(false);
		pkWndRightArrowSelect->Visible(false);
		pkWndRightArrow->Visible(true);
	}
	else if( iLeft < iRight )
	{
		pkWndLeftArrowSelect->Visible(false);
		pkWndLeftArrow->Visible(true);
		pkWndRightArrowSelect->Visible(true);
		pkWndRightArrow->Visible(false);
	}
	else
	{
		pkWndLeftArrowSelect->Visible(false);
		pkWndLeftArrow->Visible(true);
		pkWndRightArrowSelect->Visible(false);
		pkWndRightArrow->Visible(true);
	}

	pkWndLeftCountImg->UVUpdate(iLeft+1);
	pkWndRightCountImg->UVUpdate(iRight+1);
	pkWndLeftCountImg->Invalidate();
	pkWndRightCountImg->Invalidate();
}

size_t GetNeedInfallibleItemCount(PgMission_ResultC const& rkMissionResult)
{
	return ((rkMissionResult.UseSelectItem() == true) ? 3 : 1);
}

void lwSet_DefenceSelectUI(int const iStageNo)
{
	PgMission_ResultC kMission_ResultC = g_kMissionComplete.GetMission_ResultC();

	XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("FRM_MISSION_SELECT_DIRECTION"));
	if( !pkWndTop ) return;
	XUI::CXUI_Wnd* pkWndCorrectChain = pkWndTop->GetControl(L"FRM_TEXT_CORRECT_CHAIN");
	if( !pkWndCorrectChain ) return;
	XUI::CXUI_Wnd* pkWndExplain = pkWndTop->GetControl(L"FRM_TEXT_EXPLAIN");
	if( !pkWndExplain ) return;
	XUI::CXUI_Wnd* pkWndItemLeft = pkWndTop->GetControl(L"BTN_USE_ITEM_LEFT");
	if( !pkWndItemLeft ) return;
	XUI::CXUI_Wnd* pkWndItemRight = pkWndTop->GetControl(L"BTN_USE_ITEM_RIGHT");
	if( !pkWndItemRight ) return;

	pkWndItemRight->Enable(true);
	pkWndItemLeft->Enable(true);

	//이전 사용 여부에 따라 아이템 사용갯수 표시
	size_t const iNeedItemCount = GetNeedInfallibleItemCount(kMission_ResultC); //이전판에 썼으면 3개. 안 썼으면 1개로 표시
	BM::vstring strItemCount(_T(""));
	strItemCount += iNeedItemCount;

	XUI::CXUI_Wnd* pkWndItemLeftCount = pkWndItemLeft->GetControl(L"ICN_SRC");
	if(NULL != pkWndItemLeftCount)
	{
		XUI::CXUI_Wnd* pkWndCount = pkWndItemLeftCount->GetControl(L"ITEM_COUNT");
		if(NULL != pkWndCount)
		{
			pkWndCount->Text(strItemCount);
		}
	}
	XUI::CXUI_Wnd* pkWndItemRightCount = pkWndItemRight->GetControl(L"ICN_SRC");
	if(NULL != pkWndItemRightCount)
	{
		XUI::CXUI_Wnd* pkWndCount = pkWndItemRightCount->GetControl(L"ITEM_COUNT");
		if(NULL != pkWndCount)
		{
			pkWndCount->Text(strItemCount);
		}
	}

	pkWndItemLeft->SetCustomData( &iNeedItemCount, sizeof(iNeedItemCount) );
	pkWndItemRight->SetCustomData( &iNeedItemCount, sizeof(iNeedItemCount) );

	XUI::CXUI_Wnd* pkFrmLCount = pkWndItemLeft->GetControl(L"FRM_L_COUNT_INFALLIBLE_COUNT");
	XUI::CXUI_Wnd* pkFrmRCount = pkWndItemRight->GetControl(L"FRM_R_COUNT_INFALLIBLE_COUNT");
	if(NULL != pkFrmLCount && NULL != pkFrmRCount)
	{
		int iInfallibleItemCount = 0;
		pkFrmLCount->SetCustomData(&iInfallibleItemCount, sizeof(int));
		pkFrmRCount->SetCustomData(&iInfallibleItemCount, sizeof(int));

		if(0 != lwGetEventMission())
		{
			int iMaxPlayer = 1;
			PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
			if(NULL != pkMyPlayer)
			{
				if(true == pkMyPlayer->GetPartyGuid().IsNotNull())
				{
					iMaxPlayer = g_kParty.MemberCount();
				}
			}
			strItemCount = _T("");
			strItemCount += iInfallibleItemCount;
			strItemCount += _T("/");
			strItemCount += iMaxPlayer;

			pkFrmLCount->Visible(true);
			pkFrmLCount->Text(strItemCount);
			pkFrmRCount->Visible(true);
			pkFrmRCount->Text(strItemCount);
		}
		else
		{
			pkFrmLCount->Visible(false);
			pkFrmRCount->Visible(false);
		}
	}

	//연속 성공횟수
	BM::vstring vStrText(::TTW(400966)), vStrTemp;
	vStrTemp = kMission_ResultC.SuccessCount();
	vStrText.Replace(L"#SUCCESS_COUNT#", vStrTemp);
	pkWndCorrectChain->Text(vStrText);
	

	//아래쪽 아이템 박스 
	XUI::CXUI_Wnd* pkWndItemBox = XUIMgr.Get(_T("FRM_REWARD_ITEM_BOX"));
	if( !pkWndItemBox ) return;

	
	GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
	CMonsterDef const * pkMonsterDef = kMonsterDefMgr.GetDef(kMission_ResultC.MonsterNo());
	if(!pkMonsterDef) return;

	//보스 아이콘
	XUI::CXUI_Wnd* pkBossIcon = pkWndItemBox->GetControl(L"IMG_BOSS_ICON");
	if( !pkBossIcon ) return;

	bool bShowIcon = false;
	CONT_DEF_DEFENCE_ADD_MONSTER::mapped_type kAddMonsterInfo;
	if( ::GetDefenceAddMonsterInfo(iStageNo, kMission_ResultC.SuccessCount()+1, kAddMonsterInfo) )
	{//정보가 있음.
		if( PgNiFile::CheckFileExist( kAddMonsterInfo.wstrIconPath.c_str() ) )
		{
			pkBossIcon->DefaultImgName( kAddMonsterInfo.wstrIconPath );
			bShowIcon = true;
		}
	}
	pkBossIcon->Visible(bShowIcon);

	//몬스터 뭐시기 등장함 설명
	XUI::CXUI_Wnd* pkMonsterText = pkWndItemBox->GetControl(L"FRM_MONSTER_EXPLAIN");
	if( !pkMonsterText ) return;

	vStrText = ::TTW(400918);

	wchar_t const * pkMonName = NULL;
	if( false == GetDefString(pkMonsterDef->NameNo(),pkMonName) )
	{
		pkMonName = L" ";
	}
	vStrText.Replace(L"#MON_NAME#", pkMonName);
	pkMonsterText->Text(vStrText);


	int const iCurrentIndex = 2;
	int iCurrentItemCount = 1;
	
	//아이템 셋팅
	int iSuccessCount = kMission_ResultC.SuccessCount() - 1;
	for(int iIndex=0; iIndex<5; ++iIndex,++iSuccessCount)
	{
		BM::vstring kString(L"ICN_ITEM");
		kString += iIndex;
		XUI::CXUI_Wnd* pkItemIcon = pkWndItemBox->GetControl(kString.operator wchar_t const*());
		if( !pkItemIcon ) 
		{
			continue;
		}

		int iItemNo = 0;
		int iItemCount = 0;
		CONT_DEF_DEFENCE_ADD_MONSTER::mapped_type kAddMonsterInfo;
		if( ::GetDefenceAddMonsterInfo(iStageNo, iSuccessCount, kAddMonsterInfo) )
		{//정보가 있음.
			iItemNo = kAddMonsterInfo.iItemNo;
			iItemCount = kAddMonsterInfo.iItemCount;
			if(iIndex == iCurrentIndex)
			{
				iCurrentItemCount = std::max(1, iItemCount);
			}
		}

		BM::vstring szItemCount(_T(""));
		if(iItemCount > 0)
		{
			szItemCount += iItemCount;
		}

		pkItemIcon->SetCustomData(&iItemNo, sizeof(iItemNo));
		pkItemIcon->Text(szItemCount);

		kString = L"FRM_NUMBER";
		kString += iIndex;
		XUI::CXUI_Wnd* pkItemNumber = pkWndItemBox->GetControl(kString.operator wchar_t const*());
		if( !pkItemNumber ) 
		{
			continue;
		}

		BM::vstring kNumber(L"");
		if( 0 < iSuccessCount )
		{
			kNumber = iSuccessCount;
		}
		pkItemNumber->Text(kNumber);
	}

	//아이템 몇퍼 준다 설명
	XUI::CXUI_Wnd* pkItemText = pkWndItemBox->GetControl(L"FRM_ITEM_EXPLAIN");
	if( !pkItemText ) return;

	vStrText = ::TTW(400919);
	vStrTemp = kMission_ResultC.DropRate();
	vStrText.Replace(L"#DROP_RATE#", vStrTemp);

	std::wstring strDropCount;
	WstringFormat( strDropCount, MAX_PATH, TTW(750050).c_str(), iCurrentItemCount);
	vStrText += strDropCount;


	pkItemText->Text(vStrText);

}

void lwUpdateRewardItemCount(size_t iInfallibleItemCount)
{
	XUI::CXUI_Wnd* pkWndItemBox = XUIMgr.Get(_T("FRM_REWARD_ITEM_BOX"));
	if( NULL == pkWndItemBox ) { return; }


	XUI::CXUI_Wnd* pkItemText = pkWndItemBox->GetControl(_T("FRM_ITEM_EXPLAIN"));
	if( NULL == pkItemText ) { return; }

	PgMission_ResultC const kMission_ResultC = g_kMissionComplete.GetMission_ResultC();
	int const iSuccessCount = kMission_ResultC.SuccessCount() + 1;
	CONT_DEF_DEFENCE_ADD_MONSTER::mapped_type kAddMonsterInfo;
	int const iStageNo = lua_tinker::call< int >("GetDefenceStageNo");
	if( true == ::GetDefenceAddMonsterInfo(iStageNo, iSuccessCount, kAddMonsterInfo) )
	{
		size_t iCurrentItemCount = std::max(1, kAddMonsterInfo.iItemCount);

		BM::vstring vStrText(::TTW(400919)), vStrTemp;
		vStrTemp = kMission_ResultC.DropRate();
		vStrText.Replace(L"#DROP_RATE#", vStrTemp);

		std::wstring strDropCount;
		WstringFormat( strDropCount, MAX_PATH, TTW(750050).c_str(), iCurrentItemCount * iInfallibleItemCount);
		vStrText += strDropCount;


		pkItemText->Text(vStrText);
	}
}

void lwDisplayIcon_DefenceSelectUI(int const iBuildIndex, int const iItemNo)
{
	XUI::CXUI_Wnd* pkWndItemBox = XUIMgr.Get(_T("FRM_REWARD_ITEM_BOX"));
	if( !pkWndItemBox ) return;

	BM::vstring kString(L"ICN_ITEM");
	kString += iBuildIndex;
	XUI::CXUI_Wnd* pkItemIcon = pkWndItemBox->GetControl(kString.operator wchar_t const*());
	if( !pkItemIcon ) return;

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pItemDef = kItemDefMgr.GetDef(iItemNo);
	if(pItemDef)
	{//아이템 있다.. 렌더링하자.
		bool bGray = (iBuildIndex < 2 ? true : false);
		g_kUIScene.RenderIcon(pItemDef->ResNo(), pkItemIcon->TotalLocation(), false, 40, 40, bGray);
	}
}

//void lwMQCard_Select(lwUIWnd UISelf)
//{
//	if( UISelf.IsNil() )
//	{
//		return;
//	}
//
//	if( UISelf.GetSelf()->GrayScale() )
//	{
//		return;
//	}
//
//	int const iCardID = UISelf.GetCustomData<int>();
//
//	if(	lwMQCard_ForwardID(UISelf, "FRM_MISSION_QUEST_TURN") )
//	{	
//		lwCloseUI("FRM_MISSION_QUEST");
//	}
//}

//void lwMQCard_Update(int const iIndex)
//{
//	int	iCardIndex = iIndex;
//
//	if( 0 > iCardIndex )
//	{
//		iCardIndex = 0;
//	}
//	else if( g_kQuestMan.MQCardList_GetCardCount() < iCardIndex )
//	{
//		iCardIndex = g_kQuestMan.MQCardList_GetCardCount();
//	}
//
//	MissionQuestUISetting(iCardIndex);
//}

//void lwMQCard_QuestInfo(DWORD dwQuestID)
//{
//	//	카드를 닫고
//	XUI::CXUI_Wnd* pTurnWnd = XUIMgr.Get(L"FRM_MISSION_QUEST");
//	if( pTurnWnd )
//	{
//		pTurnWnd->Close();
//	}
//
//	//	나를 열고
//	XUI::CXUI_Wnd* MeWnd = XUIMgr.Call(L"FRM_MISSION_QUEST_INFO");
//	if( MeWnd )
//	{
//		const PgQuestInfo* pkQuestInfo = g_kQuestMan.GetQuest(dwQuestID);
//		if( !pkQuestInfo )
//		{
//			return;
//		}
//
//		std::wstring	wstrText = L"";
//		XUI::CXUI_Wnd* pChild = MeWnd->GetControl(L"FRM_TEXT_TITLE");
//		if( pChild )
//		{
//			//	제목을 넣어줘야해
//			pChild->Text(TTW(pkQuestInfo->m_iTitleTextNo).c_str());
//		}
//
//		//	여기선 지역별로 카드 일러스트를 바꾼다
//		XUI::CXUI_Wnd*	pIllustChild = pChild->GetControl(L"IMG_CARD_ILLU");
//		if( pIllustChild )
//		{
//			std::wstring	wstrPath = g_kMissionQuestMng.GetMQCardIllustDirToQuestID(pkQuestInfo->ID());
//			if( wstrPath.size() )
//			{
//				pIllustChild->DefaultImgName(wstrPath); 
//			}
//		}
//		
//		//	텍스트를 뿌리고
//		pChild = MeWnd->GetControl(L"FRM_TEXT");
//		if( pChild )
//		{
//			SQuestDialog const*	TextDlg;
//			bool const bRet =  pkQuestInfo->GetDialog(101, TextDlg);
//			if( bRet )
//			{
//				ContQuestDialogText::const_iterator	iter = TextDlg->kDialogText.begin();
//				while(iter != TextDlg->kDialogText.end())
//				{
//					if( wstrText.size() )
//					{
//						wstrText += L"\n";
//					}
//					wstrText += TTW(iter->iTextNo);
//					++iter;
//				}
//
//				pChild->Text(wstrText);
//			}
//		}
//		
//		wstrText.clear();
//
//		//	보상을 뿌리고
//		pChild = MeWnd->GetControl(L"FRM_REWORD");
//		if( pChild )
//		{
//			wchar_t	szTemp[MAX_PATH] = {0,};
//
//			//	경험치
//			swprintf(szTemp, MAX_PATH, TTW(8005).c_str(), pkQuestInfo->m_kReward.iExp);
//			wstrText += szTemp;
//			wstrText += L"\n";
//			
//			//	돈
//			swprintf(szTemp, MAX_PATH, TTW(8005).c_str(), pkQuestInfo->m_kReward.iMoney);
//			wstrText += szTemp;
//
//			pChild->Text(wstrText);
//		}
//	}
//}

//void lwMQCard_TimeText(lwUIWnd UISelf, int const iTime)
//{
//	wchar_t	szTemp[MAX_PATH] = {0,};
//
//	swprintf(szTemp, MAX_PATH, TTW(8002).c_str(), iTime);
//	UISelf.SetStaticTextW(szTemp);
//}

//void MissionQuestUISetting(int const iCardStart)
//{
//	std::wstring const CARD_FORM = L"FRM_CARD";
//	int const MAX_CARD = 4;
//
//	int const iCardCnt = g_kQuestMan.MQCardList_GetCardCount();
//
//	//	Main Form
//	XUI::CXUI_Wnd* pCardWnd = XUIMgr.Get(std::wstring(L"FRM_MISSION_QUEST"));
//	if( pCardWnd )
//	{
//		XUI::CXUI_Button* pLeftBtnWnd = dynamic_cast<XUI::CXUI_Button*>(pCardWnd->GetControl(L"BTN_SELECT_ARROW_LEFT"));
//		XUI::CXUI_Button* pRightBtnWnd = dynamic_cast<XUI::CXUI_Button*>(pCardWnd->GetControl(L"BTN_SELECT_ARROW_RIGHT"));
//
//		//	왼쪽 버튼 활성화 조건
//		if( iCardStart != 0 && iCardCnt > MAX_CARD )
//		{ 
//			pLeftBtnWnd->IsClosed(false);
//			pLeftBtnWnd->Disable(false);
//		}
//		else
//		{ 
//			pLeftBtnWnd->IsClosed(true);
//			pLeftBtnWnd->Disable(true);
//		}
//		
//		//	오른쪽 버튼 활성화 조건
//		if( iCardCnt - iCardStart <= MAX_CARD )
//		{ 
//			pRightBtnWnd->IsClosed(true);
//			pRightBtnWnd->Disable(true);  
//		}
//		else
//		{
//			pRightBtnWnd->IsClosed(false);
//			pRightBtnWnd->Disable(false); 
//		}
//		
//		for(int i = 0; i < MAX_CARD; ++i)
//		{
//			int	const	iCardNum = iCardStart + i;
//			BM::vstring kFormName(CARD_FORM);
//			kFormName += i;
//
//			//	Child Card Form Search
//			XUI::CXUI_Wnd*	pChild = pCardWnd->GetControl((std::wstring const&)kFormName);
//			XUI::CXUI_Wnd*	pIllustChild = pChild->GetControl(L"IMG_CARD_ILLU");
//			XUI::CXUI_Wnd*	pBackChild = pChild->GetControl(L"IMG_CARD_BACK");
//			if( pChild && pIllustChild )
//			{
//				pChild->GrayScale(false);
//				pIllustChild->Visible(true);
//				pBackChild->Visible(false);
//				pBackChild->UVUpdate(i+1);
//
//				//카드의 회색화 조건
//				if( iCardCnt - iCardStart <= MAX_CARD && i >= iCardCnt - iCardStart)
//				{
//					pIllustChild->Visible(false);
//					pChild->GrayScale(true);
//					pBackChild->Visible(true);
//				}
//				else
//				{
//					//	Card Item ID
//					int const iCardID = g_kQuestMan.MQCardList_SelectCard(iCardNum);
//
//					//	Item Get
//					GET_DEF(CItemDefMgr, kItemDefMgr);
//					CItemDef const *pDef = kItemDefMgr.GetDef(iCardID);
//					if( pDef )
//					{
//						//	여기서 랭크별로 카드 이미지를 바꾼다
//						std::wstring	wstrPath = g_kMissionQuestMng.GetMQCardGradeBg(pDef->GetAbil(AT_ITEM_QUEST_CARD_GRADE));
//						if( wstrPath.size() )
//						{
//							pChild->DefaultImgName(); 
//						}
//
//						wstrPath = g_kMissionQuestMng.GetMQCardIllustDirToCardID(iCardID);
//						if( wstrPath.size() )
//						{
//							pIllustChild->DefaultImgName(wstrPath); 
//						}
//
//						//	카드에 번호를 넣자
//						pChild->SetCustomData(&iCardID, sizeof(iCardID));
//					}
//					else
//					{
//						//	해당 카드가 없다
//					}
//				}
//			}
//		}
//	}
//}

//bool lwMQCard_ForwardID(lwUIWnd UISour, char const* pDest)
//{
//	if( UISour.IsNil() == false )
//	{
//		int const iCustomData = UISour.GetCustomData<int>();
//		if( !iCustomData )
//		{
//			return false;
//		}
//
//		XUI::CXUI_Wnd*	pDestWnd = XUIMgr.Call(UNI(pDest));
//		if( !pDestWnd )
//		{
//			return false;
//		}
//
//		pDestWnd->Location(UISour.GetLocation().GetX(), UISour.GetLocation().GetY());
//		pDestWnd->SetCustomData(&iCustomData, sizeof(iCustomData));
//	}
//
//	return	true;
//}

//void lwMQCard_Scale(lwUIWnd UISelf, float const fMQCLStartTime, float const fVScale)
//{
//	int const INFO_X = 378;
//	int const INFO_Y = 175;
//	static int iTickMovePosX = 0;
//		
//	XUI::CXUI_Wnd*	pWnd = UISelf.GetSelf();
//	if( pWnd )
//	{
//		if( fVScale > 2.3f )
//		{
//			if( iTickMovePosX != 0 )
//			{
//				BM::Stream kPacket(PT_C_M_REQ_MISSION_QUEST, (BYTE)MQC_Req_SelectCard);
//				kPacket.Push(UISelf.GetCustomData<int>());
//				NETWORK_SEND(kPacket)
//				iTickMovePosX = 0;
//			}
//			return;
//		}
//
//		if( iTickMovePosX == 0 )
//		{
//			iTickMovePosX = (512 - (UISelf.GetLocation().GetX() + (pWnd->Width() / 2))) / 8;
//		}
//
//		int const iOld_Width = (int)(pWnd->Width() * pWnd->Scale());
//		int const iOld_Height = (int)(pWnd->Height() * pWnd->Scale());
//		//	Scale 처리
// 		pWnd->Scale(fVScale);
//		pWnd->Location(UISelf.GetLocation().GetX() - ((int)(pWnd->Width() * fVScale) - iOld_Width) / 2 + iTickMovePosX,
//					   UISelf.GetLocation().GetY() - ((int)(pWnd->Height() * fVScale) - iOld_Height) / 2 + 5);
//	}
//}

void SetDefenceMissionLevelingText(XUI::CXUI_Wnd *pWnd, int const iNum)
{
	if( !pWnd ){ return; }

	const CONT_MISSION_DEFENCE7_MISSION_BAG *pkMission;
	g_kTblDataMgr.GetContDef(pkMission);
	if( !pkMission )
	{
		return;
	}

	int const iModeType = GetDefenceModeType( iNum+1 );
	CONT_MISSION_DEFENCE7_MISSION_BAG::key_type		kKey(lwGetMissionNo(), iModeType);

	CONT_MISSION_DEFENCE7_MISSION_BAG::const_iterator iter = pkMission->find(kKey);
	if( iter == pkMission->end() || iter->second.kCont.empty() )
	{
		return;
	}

	SMISSION_DEFENCE7_MISSION kValue = iter->second.kCont.at(0);

	BM::vstring vStr(TTW(8012));
	vStr.Replace(L"#MIN#", kValue.iMin_Level);
	vStr.Replace(L"#MAX#", kValue.iMax_Level);
	pWnd->Text(vStr);
}

int lwGetEnterDefenceMinLevel(int const iMissionNo, int const iMissionLv)
{
	SMISSION_DEFENCE7_MISSION kMission;
	GetDefence7Mission(kMission, iMissionNo, iMissionLv);
	return kMission.iMin_Level;
}

int lwGetEnterDefenceMaxLevel(int const iMissionNo, int const iMissionLv)
{
	SMISSION_DEFENCE7_MISSION kMission;
	GetDefence7Mission(kMission, iMissionNo, iMissionLv);
	return kMission.iMax_Level;
}

void lwSetMissionLevelingText(lwUIWnd UISelf, int const iNum)
{
	if(!g_pkWorld)
	{
		return;
	}
	if( MAX_MISSION_LEVEL<=iNum )
	{
		SetDefenceMissionLevelingText(UISelf(),iNum);
		return;
	}

	CONT_DEF_MISSION_ROOT const* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);	

	if( pContDefMap )
	{
		if( !g_pkWorld->IsHaveAttr(GATTR_MISSION) && false==g_bAutoLevel )
		{
			TBL_DEF_MAP const* pDefMap = PgWorld::GetMapData(g_pkWorld->MapNo());
			if( pDefMap )
			{
				if (pDefMap->kContMissionNo.empty())
				{
					g_iMissionNo = 0;
					NILOG(PGLOG_WARNING, "[%s] Cannot find Mission in the Map[%d]", __FUNCTION__, g_pkWorld->MapNo());
					return;
				}
				else
				{
					g_iMissionNo = pDefMap->kContMissionNo.at(0);					
				}
			}
		}

		if( g_iMissionNo )
		{
			CONT_DEF_MISSION_ROOT::const_iterator	root_it = pContDefMap->find(g_iMissionNo);
			if( root_it != pContDefMap->end())
			{
				if( 0 < root_it->second.aiLevel[iNum] )
				{
					UISelf.Visible(true);
					if(0 == root_it->second.aiLevel_AvgMin[iNum] && 0 == root_it->second.aiLevel_AvgMax[iNum])
					{
						UISelf.SetStaticTextW(TTW(8008));
					}
					else
					{
						wchar_t	szTemp[MAX_PATH] = {0,};
						swprintf(szTemp, MAX_PATH, TTW(8007).c_str(), root_it->second.aiLevel_AvgMin[iNum], root_it->second.aiLevel_AvgMax[iNum]);
						UISelf.SetStaticTextW(szTemp);
					}
				}
				else
				{
					UISelf.Visible(false);
				}
			}
		}
	}
}

bool lwGetMissionLevelArea(int const iNum)
{
	if( g_iMissionNo )
	{
		CONT_DEF_MISSION_ROOT const* pContDefMap = NULL;
		g_kTblDataMgr.GetContDef(pContDefMap);	

		if( pContDefMap )
		{
			CONT_DEF_MISSION_ROOT::const_iterator	root_it = pContDefMap->find(g_iMissionNo);
			if( root_it != pContDefMap->end())
			{
				if( 0 < root_it->second.aiLevel[iNum] )
				{
					return true;
				}
			}
		}
	}
	return false;
}

lwWString lwGetMissionLevelingText(lwUIWnd UISelf, int const iNum)
{
	if(!g_pkWorld)
	{
		return lwWString("");
	}
	CONT_DEF_MISSION_ROOT const* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);	

	wchar_t	szTemp[MAX_PATH] = {0,};
	lwWString kWString = L"";

	if( pContDefMap )
	{
		if( !g_pkWorld->IsHaveAttr(GATTR_MISSION) )
		{
			TBL_DEF_MAP const* pDefMap = PgWorld::GetMapData(g_pkWorld->MapNo());
			if( pDefMap )
			{
				if( pDefMap->kContMissionNo.empty() )
				{
					g_iMissionNo = 0;
					PgError("[lwGetMissionLevelingText] pDefMap->kContMissionNo empty!");
				}
				else
				{
					g_iMissionNo = pDefMap->kContMissionNo.at(0);					
				}
			}
		}

		if( g_iMissionNo )
		{
			CONT_DEF_MISSION_ROOT::const_iterator	root_it = pContDefMap->find(g_iMissionNo);
			if( root_it != pContDefMap->end())
			{				
				swprintf(szTemp, MAX_PATH, TTW(8007).c_str(), root_it->second.aiLevel_AvgMin[iNum], root_it->second.aiLevel_AvgMax[iNum]);				
			}
		}
	}

	wchar_t szText[256] = {0,};
	swprintf( szText, TTW(9010 + iNum).c_str(), szTemp );

	return lwWString((std::wstring const&)szText);
}

bool lwSendTakeCoupon(char const* pkStr)
{
	if(!pkStr)
	{
		return false;
	}

	BM::Stream kPacket(PT_C_M_TRY_TAKE_COUPON);
	kPacket.Push((std::wstring)UNI(pkStr));

	NETWORK_SEND(kPacket)

	return true;
}

void lwDisPlayMissionUpdateScore(int const iType, int const iScore)
{
	XUI::CXUI_Wnd* m_pkWnd = XUIMgr.Get(_T("FRM_MISSION_SCORE"));
	if (!m_pkWnd) { return; }

	int iValue = 0;

	switch( iType )
	{
	case 0:
		{
			XUI::CXUI_AniBar* pAniBar = dynamic_cast<XUI::CXUI_AniBar*>(m_pkWnd->GetControl(std::wstring(L"ANI_SENSE")));
			if( pAniBar )
			{
				iValue = (iScore > MISSION_SCOREUP_MAX)?(MISSION_SCOREUP_MAX):(iScore);
				
				pAniBar->Now(iValue);
				pAniBar->Max(MISSION_SCOREUP_MAX);
			}
		}break;
	case 1:
		{
			XUI::CXUI_AniBar* pAniBar = dynamic_cast<XUI::CXUI_AniBar*>(m_pkWnd->GetControl(std::wstring(L"ANI_ABILITY")));
			if( pAniBar )
			{
				iValue = (iScore > MISSION_SCOREUP_MAX)?(MISSION_SCOREUP_MAX):(iScore);
				
				pAniBar->Now(iValue);
				pAniBar->Max(MISSION_SCOREUP_MAX);
			}
		}break;
	case 2:
		{
			XUI::CXUI_AniBar* pAniBar = dynamic_cast<XUI::CXUI_AniBar*>(m_pkWnd->GetControl(std::wstring(L"ANI_TOTALSCORE")));
			if( pAniBar )
			{
				iValue = (iScore > MISSION_TOTALSCORE_MAX)?(MISSION_TOTALSCORE_MAX):(iScore);
				
				pAniBar->Now(iValue);
				pAniBar->Max(MISSION_TOTALSCORE_MAX);
			}
	
			int iIndex = 0;
			for(int i=0; i<MISSION_RANK;)
			{
				if( iValue >= ms_aiResultRank[i++] )
				{
					iIndex = (EMissionRank)i;
					break;
				}
			}

			XUI::CXUI_Wnd* m_pkRankWnd = dynamic_cast<XUI::CXUI_Wnd*>(m_pkWnd->GetControl(std::wstring(L"TOTALSCORE_BG")));
			if( m_pkRankWnd )
			{
				SUVInfo kUVInfo= m_pkRankWnd->UVInfo();
				if(kUVInfo.Index != iIndex)
				{
					kUVInfo.Index = iIndex;
					m_pkRankWnd->UVInfo(kUVInfo);
					m_pkRankWnd->SetInvalidate();	
				}
			}
		}break;
	default:
		{
			return;
		}break;
	}
}

void lwOnSelectGadaCoinBox()
{
	g_kMissionComplete.OnSelectGadaCoinBox();
}

int lwGetMissionType()
{
	return g_kMissionComplete.GetMissionType();
}

float GetMissionCountAddExpText(int iMissionCount)
{
	if( 1 >= iMissionCount )
	{
		return 1.0f;
	}

	if( 15 <= iMissionCount )
	{
		return static_cast<float>( 2.5f );
	}
	else
	{
		return static_cast<float>( ((iMissionCount*0.1f) + 1.0f) );
	}
}

bool lwGetMissionExpUI(int const iMissionNo)
{
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(!pkUnit)
	{
		return false;	
	}

	if( iMissionNo == pkUnit->GetAbil(AT_MISSION_NO) )
	{
		// 1번만 클리어 하면 다음부터 나와야 된다.
		if( 1 <= pkUnit->GetAbil(AT_MISSION_COUNT) )
		{
			return true;
		}
	}
	return false;
}

void lwGetMissionPlayCount(lwUIWnd UISelf)
{
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if(!pkUnit)
	{
		return;	
	}

	if( UISelf.IsNil() )
	{
		return;
	}

	XUI::CXUI_Wnd* m_pkWnd = UISelf.GetSelf();
	if( !m_pkWnd )
	{
		return;
	}

	// Clear 할 경우 증가 되기 때문에 값이 1인 경우는 2번째의 경우이다.
	int const iMissionCount = (pkUnit->GetAbil(AT_MISSION_COUNT) + 1);
	std::wstring wstrTemp = _T("");

	XUI::CXUI_Wnd* pText1 = m_pkWnd->GetControl(L"FRM_TEXT1");
	if( pText1 )
	{		
		FormatTTW( wstrTemp, 401065,  GetMissionCountAddExpText( iMissionCount-1 ));
		pText1->Text(wstrTemp);
	}
	XUI::CXUI_Wnd* pText2 = m_pkWnd->GetControl(L"FRM_TEXT2");
	if( pText2 )
	{
		FormatTTW( wstrTemp, 401066,  GetMissionCountAddExpText( iMissionCount ));
		pText2->Text(wstrTemp);
	}
	XUI::CXUI_Wnd* pText3 = m_pkWnd->GetControl(L"FRM_TEXT3");
	if( pText3 )
	{
		FormatTTW( wstrTemp, 401065,  GetMissionCountAddExpText( iMissionCount+1 ));
		pText3->Text(wstrTemp);
	}
	XUI::CXUI_Wnd* pText4 = m_pkWnd->GetControl(L"FRM_TEXT4");
	if( pText4 )
	{
		FormatTTW( wstrTemp, 401065,  GetMissionCountAddExpText( iMissionCount+2 ));
		pText4->Text(wstrTemp);
	}
}

int lwHiddenRewordItemView(int const iItemNo)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return 0;}
	PgInventory *pInv = pkPlayer->GetInven();
	if(!pInv){return 0;}

	int iItemCount = pInv->GetTotalCount(iItemNo);

	return iItemCount;
}

int lwHiddenRewordItemView2(int const iItemCount)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return 0;}


	PgHiddenRewordItemMgr	m_kHiddenRewordItemMgr;

	int const iLevel = pkPlayer->GetAbil(AT_LEVEL);

	int const iItemTotalCount = m_kHiddenRewordItemMgr.GetMyItem_ViewCount(iLevel, iItemCount);

	return iItemTotalCount;
}

int lwHiddenRewordItemViewIcon(int const iItemCount)
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(!pkPlayer){return 0;}

	PgHiddenRewordItemMgr	m_kHiddenRewordItemMgr;

	int const iLevel = pkPlayer->GetAbil(AT_LEVEL);

	int const iItemIconNo = m_kHiddenRewordItemMgr.GetMyItem_ViewItemNo(iLevel, iItemCount);

	return iItemIconNo;
}

bool lwIsMissionComplatedQuest(int const iLevel)
{
	if(7==iLevel)
	{
		return g_kMissionMgr.IsMode7ComplatedQuest();
	}
	else if(6==iLevel)
	{
		return g_kMissionMgr.IsMode6ComplatedQuest();
	}
	return true;
}

bool lwIsOpenDefenseModeTable(eDEFENSEMODE_TYPE const eType, int const iMissionNo )
{
	if(eType==EDFST_F8 && false==lua_tinker::call<bool>("LocalCheck_IsOpenDeffencMode8") )
	{
		return false;
	}

	CONT_DEF_MISSION_ROOT const* pkContDefMissionRoot = NULL;
	g_kTblDataMgr.GetContDef(pkContDefMissionRoot);
	if( !pkContDefMissionRoot )
	{
		return false;
	}

	CONT_DEF_MISSION_ROOT::const_iterator Root_itor = pkContDefMissionRoot->find(iMissionNo);
	if( pkContDefMissionRoot->end() !=  Root_itor )
	{
		CONT_DEF_MISSION_ROOT::mapped_type kElement = Root_itor->second;
        
        if(eType==DEFST_ALL && 0 < kElement.iDefence && 0 < kElement.iDefence7 && 0 < kElement.iDefence8)
        {
            return true;
        }
        if(eType==DEFST_STRATEGIC && 0 < kElement.iDefence7 && 0 < kElement.iDefence8)
        {
            return true;
        }
        if(eType==EDFST_F6 && 0 < kElement.iDefence )
		{
			return true;
		}
        if(eType==EDFST_F7 && 0 < kElement.iDefence7 )
		{
			return true;
		}
		if(eType==EDFST_F8 && 0 < kElement.iDefence8 )
		{
			return true;
		}
	}
 
	return false;
}

extern bool IsDeffenseMode( int const iMissionNo )
{
	CONT_DEF_MISSION_ROOT const* pkContDefMissionRoot = NULL;
	g_kTblDataMgr.GetContDef(pkContDefMissionRoot);
	if( !pkContDefMissionRoot )
	{
		return false;
	}

	CONT_DEF_MISSION_ROOT::const_iterator Root_itor = pkContDefMissionRoot->find(iMissionNo);
	if( pkContDefMissionRoot->end() !=  Root_itor )
	{
		CONT_DEF_MISSION_ROOT::mapped_type kElement = Root_itor->second;
		if( 0 < kElement.iDefence )
		{
			return true;	
		}
	}
 
	return false;
}

extern bool lwEnter_DefenceMission( int const iMissionNo )
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer )
	{
		return false;
	}
	PgInventory *pkInv = pkPlayer->GetInven();
	if( !pkInv )
	{
		return false;
	}

	ContHaveItemNoCount kContHaveItemNoCount;
	if( S_OK == pkInv->GetItems(UICT_MISSION_DEFENCE_CONSUME, kContHaveItemNoCount ) )
	{//해당 미션번호에 맞는 아이템이 있는지 먼저 검사
		if( true == Find_DefenceConsumeItem(kContHaveItemNoCount, iMissionNo) )
		{
			return true;
		}
	}

	kContHaveItemNoCount.clear();
	if( S_OK == pkInv->GetItems(UICT_MISSION_DEFENCE_CONSUME_ALL, kContHaveItemNoCount ) )
	{//캐시템이 있는지 찾는다
		if( true == Find_DefenceConsumeItem(kContHaveItemNoCount, 0) )
		{
			return true;
		}
	}

	//둘다 없음 헐
	lwAddWarnDataTT(400976);
	return false;
}

bool Find_DefenceConsumeItem( ContHaveItemNoCount& rkContItemNoCount, int const iMissionNo)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);

	ContHaveItemNoCount::const_iterator itor = rkContItemNoCount.begin();
	while( rkContItemNoCount.end() != itor )
	{
		int const iItemNo = itor->first;
		CItemDef const* pkItemDef = kItemDefMgr.GetDef(iItemNo);
		if( pkItemDef )
		{
			if( iMissionNo == pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1) )
			{//찾았다
				return true;
			}
		}
		++itor;
	}

	return false;
}

bool Result_DefenceInfallibleSelection(BM::Stream& rkPacket)
{
	HRESULT hRet(E_FAIL);
	rkPacket.Pop(hRet);

	BM::vstring vStrNotice;
	ENoticeLevel kNoticeLevel = EL_Warning;

	switch( hRet )
	{
	case E_INFALLIBLE_SELECTION_SUCCESS:
		{
			BM::GUID kGuid;
			bool bStart = false;
			EDefenceMonsterGen kDir = D_RANDOM;
			int iInfallibleItemCount = 0;

			rkPacket.Pop(kGuid);
			rkPacket.Pop(bStart);
			rkPacket.Pop(kDir);
			rkPacket.Pop(iInfallibleItemCount);

			if(true == bStart) //서버에서 즉시 시작하라면 즉시 시작
			{
				BM::Stream kPacket(PT_C_M_NFY_DEFENCE_ENDSTAGE);
				NETWORK_SEND(kPacket);
			}

			PgPlayer * pkMyPlayer = g_kPilotMan.GetPlayerUnit();
			if ( !pkMyPlayer )
			{
				return false;
			}

			int const iItemNo = lwGetMissionCashItemNo();
			wchar_t const* pkItemName = NULL;
			GetDefString(iItemNo, pkItemName);
			if( pkMyPlayer->GetID() == kGuid )
			{
				vStrNotice = ::TTW(400922);//'백발백중' 아이템을 사용하여 효과를 발휘합니다.

				vStrNotice.Replace(L"#ITEM#", pkItemName);
			}
			else
			{
				SPartyMember const* kPartyMember = NULL;
				if( g_kParty.GetMember( kGuid, kPartyMember) )
				{
					vStrNotice = ::TTW(400921);//#NAME# 님이
					vStrNotice.Replace(L"#NAME#", kPartyMember->kName);

					vStrNotice += ::TTW(400922);//'백발백중' 아이템을 사용하여 효과를 발휘합니다.
					vStrNotice.Replace(L"#ITEM#", pkItemName);
				}
			}
			kNoticeLevel = EL_Normal;
			g_kChatMgrClient.ShowNoticeUI(vStrNotice, kNoticeLevel, false);

			{
				XUI::CXUI_Wnd* pkWndTop = XUIMgr.Get(_T("FRM_MISSION_SELECT_DIRECTION"));
				if( !pkWndTop )
				{
					break;
				}
				XUI::CXUI_Wnd* pkWndItemLeft = pkWndTop->GetControl(L"BTN_USE_ITEM_LEFT");
				if( !pkWndItemLeft )
				{
					break;
				}
				XUI::CXUI_Wnd* pkWndItemRight = pkWndTop->GetControl(L"BTN_USE_ITEM_RIGHT");
				if( !pkWndItemRight )
				{
					break;
				}

				XUI::CXUI_Wnd* pkFrmLCount = pkWndItemLeft->GetControl(L"FRM_L_COUNT_INFALLIBLE_COUNT");
				XUI::CXUI_Wnd* pkFrmRCount = pkWndItemRight->GetControl(L"FRM_R_COUNT_INFALLIBLE_COUNT");
				if(NULL != pkFrmLCount && NULL != pkFrmRCount)
				{
					pkFrmLCount->SetCustomData(&iInfallibleItemCount, sizeof(int));
					pkFrmRCount->SetCustomData(&iInfallibleItemCount, sizeof(int));

					if(pkFrmLCount->Visible() == true && pkFrmRCount->Visible() == true)
					{
						if(0 != lwGetEventMission())
						{
							int iMaxPlayer = 1;
							if(true == pkMyPlayer->GetPartyGuid().IsNotNull())
							{
								iMaxPlayer = g_kParty.MemberCount();
							}
							BM::vstring strItemCount(_T(""));
							strItemCount += iInfallibleItemCount;
							strItemCount += _T("/");
							strItemCount += iMaxPlayer;

							pkFrmLCount->Text(strItemCount);
							pkFrmRCount->Text(strItemCount); 					
						}
					}
				}

				if(kDir == D_LEFT)
				{
					pkWndItemRight->Enable(false);
				}
				else if(kDir == D_RIGHT)
				{
					pkWndItemLeft->Enable(false);
				}

				if(g_kParty.MasterGuid() == pkMyPlayer->GetID() && false == bStart)
				{ //내가 파티장이라면
					XUI::CXUI_Wnd* pkBtnGo = pkWndTop->GetControl(L"BTN_GO");
					if(NULL != pkBtnGo)
					{
						pkBtnGo->Visible(true);
						pkBtnGo->Enable(true);
					}
				}
				lwUpdateRewardItemCount(iInfallibleItemCount);
			}
			return true;
		}break;
	case E_INFALLIBLE_SELECTION_NOT_MISSION:			{ vStrNotice = ::TTW(400917);	}break;//디펜스 모드에서만 사용할 수 있는 아이템 입니다.
	case E_INFALLIBLE_SELECTION_NOT_FOUND_ITEM:	{ vStrNotice = ::TTW(2851);		}break;//아이템을 찾을 수 없습니다.
	case E_INFALLIBLE_SELECTION_NOT_ENOUGH_ITEM:	{ vStrNotice = ::TTW(1240);		}break;//아이템이 부족합니다.
	case E_INFALLIBLE_SELECTION_CANT_USE:				{ vStrNotice = ::TTW(10410);	}break;//아이템을 사용할 수 없는 상태입니다
	case E_INFALLIBLE_SELECTION_DBERROR:
	default:
		{//시스템 에러입니다. GM에게 문의해주시기 바랍니다.
			vStrNotice = ::TTW(10410);
		}break;
	}

	if( vStrNotice.size() )
	{
		::Notice_Show( vStrNotice, kNoticeLevel );
	}

	return true;
}

void Result_DefenceHpPlus(BM::Stream& rkPacket)
{
	HRESULT hRet(E_FAIL);
	rkPacket.Pop(hRet);

	BM::vstring vStrNotice;
	ENoticeLevel kNoticeLevel = EL_Warning;

	switch( hRet )
	{
	case S_OK:
		{
			int iHpPlus = 0;
			rkPacket.Pop(iHpPlus);

			//공지 메시지
			BM::vstring kTime(iHpPlus);
			vStrNotice = ::TTW(400927);//수호석의 HP가 #HP# 증가 하였습니다.
			vStrNotice.Replace(L"#HP#", kTime);

			kNoticeLevel = EL_Normal;
		}break;
	case E_CANNOT_ITEM:			{ vStrNotice = ::TTW(400929);	}break;//50% 이하가 아니라서 사용 못한다.
	case E_NOT_ENOUGH:			{ vStrNotice = ::TTW(400928);	}break;//피가 만땅이다.
	case E_NOT_FOUND_ITEM:		{ vStrNotice = ::TTW(2851);		}break;//아이템을 찾을 수 없습니다.
	case E_NOT_ENOUGH_ITEM:		{ vStrNotice = ::TTW(1240);		}break;//아이템이 부족합니다.
	case E_FAIL:				{ vStrNotice = ::TTW(10410);	}break;//아이템을 사용할 수 없는 상태입니다
	default:
		{//시스템 에러입니다. GM에게 문의해주시기 바랍니다.
			vStrNotice = ::TTW(10410);
		}break;
	}

	if( vStrNotice.size() )
	{
		::Notice_Show( vStrNotice, kNoticeLevel );
	}
}

bool Result_DefenceTimePlus(BM::Stream& rkPacket)
{
	HRESULT hRet(E_FAIL);
	rkPacket.Pop(hRet);

	BM::vstring vStrNotice;
	ENoticeLevel kNoticeLevel = EL_Warning;

	switch( hRet )
	{
	case S_OK:
		{
			int iTimePlus = 0;
			rkPacket.Pop(iTimePlus);

			//UI에 제한시간 증가 시킴
			lua_tinker::call< void, int >("Add_Defence_Time", iTimePlus);

			//초단위로 바꾸고 소수점 버림
			float iSecTime = iTimePlus * 0.001f;
			iTimePlus = static_cast<int>(iSecTime);
			
			//공지 메시지
			BM::vstring kTime(iTimePlus);
			vStrNotice = ::TTW(400923);//스테이지 제한 시간이 #TIME# 초 증가 하였습니다.
			vStrNotice.Replace(L"#TIME#", kTime);

			kNoticeLevel = EL_Normal;
		}break;
	case E_COMMON_TIMEOUT:	{ vStrNotice = ::TTW(400924); }break;//제한시간이 10초 보다 적어서 사용할 수 없습니다
	case E_INCORRECT_ITEM:		{ vStrNotice = ::TTW(401304);	}break;//잘못된 아이템 정보입니다.
	case E_NOT_FOUND_ITEM:	{ vStrNotice = ::TTW(2851);		}break;//아이템을 찾을 수 없습니다.
	case E_NOT_ENOUGH_ITEM:	{ vStrNotice = ::TTW(1240);		}break;//아이템이 부족합니다.
	case E_FAIL:						{ vStrNotice = ::TTW(10410);	}break;//아이템을 사용할 수 없는 상태입니다
	default:
		{//시스템 에러입니다. GM에게 문의해주시기 바랍니다.
			vStrNotice = ::TTW(10410);
		}break;
	}

	if( vStrNotice.size() )
	{
		::Notice_Show( vStrNotice, kNoticeLevel );
	}

	return true;
}

bool IsDefenceMode()
{
	return g_pkWorld ? (lwGetGameLevel()>=7) : false;
}

bool IsDefenceMode6()
{
	return g_pkWorld ? (lwGetGameLevel()==7) : false;
}

bool IsDefenceMode7()
{
	return g_pkWorld ? (lwGetGameLevel()==8) : false;
}

bool IsDefenceMode8()
{
	return g_pkWorld ? (lwGetGameLevel()==9) : false;
}

int GetDefenceModeType(int const iGameLevel)
{
	switch(iGameLevel)
	{
	case 7: return MO_DEFENCE;
	case 8: return MO_DEFENCE7;
	case 9: return MO_DEFENCE8;
	}

	return 0;
}
int lwGetDefenceMode7_ResultStage(int const iStage)
{
	int const iMissionNo = g_iMissionNo;
	int const iPartyNumber = std::max<int>( 1, g_kParty.MemberCount() );

	const CONT_MISSION_DEFENCE7_STAGE_BAG *pkStage;
	g_kTblDataMgr.GetContDef(pkStage);
	if( !pkStage )
	{
		return 0;
	}
	else
	{
		for(int i=iStage; iStage!=0; ++i)
		{
			int const iModeType = GetDefenceModeType( lwGetGameLevel() );
			CONT_MISSION_DEFENCE7_STAGE_BAG::key_type kKey(iMissionNo, iModeType, iPartyNumber, i);

			CONT_MISSION_DEFENCE7_STAGE_BAG::const_iterator iter = pkStage->find(kKey);
			if( iter != pkStage->end() )
			{
				CONT_MISSION_DEFENCE7_STAGE_BAG::mapped_type kElement = (*iter).second;
				CONT_MISSION_DEFENCE7_STAGE::value_type &kValue = kElement.kCont.at(0);

				if( 0 < kValue.iResult_ItemNo || 0 < kValue.iResult_No )
				{
					return i;
				}
			}
			else
			{
				return 0;
			}
		}
	}

	return 0;
}

int lwGetDefenceMode6_ResultStage(int const iStage)
{
	int const iMissionNo = g_iMissionNo;
	int const iPartyNumber = std::max<int>( 1, g_kParty.MemberCount() );

	const CONT_MISSION_DEFENCE_STAGE_BAG *pkStage;
	g_kTblDataMgr.GetContDef(pkStage);
	if( !pkStage )
	{
		return 0;
	}
	else
	{
		for(int i=iStage; iStage!=0; ++i)
		{
			CONT_MISSION_DEFENCE_STAGE_BAG::key_type kKey(iMissionNo, iPartyNumber, i);

			CONT_MISSION_DEFENCE_STAGE_BAG::const_iterator iter = pkStage->find(kKey);
			if( iter != pkStage->end() )
			{
				CONT_MISSION_DEFENCE_STAGE_BAG::mapped_type kElement = (*iter).second;
				CONT_MISSION_DEFENCE_STAGE::value_type &kValue = kElement.kCont.at(0);

				if( 0 < kValue.iResultNo )
				{
					return i;
				}
			}
			else
			{
				return 0;
			}
		}
	}

	return 0;
}


int lwGetDefenceResultStage(int const iStage)
{
	if( IsDefenceMode7() )
	{
		return lwGetDefenceMode7_ResultStage(iStage);
	}
	else
	{
		return lwGetDefenceMode6_ResultStage(iStage);
	}

	return 0;
}


typedef std::vector<DWORD> CONT_ITEMBAG;

void GetDefence7ItemContainer(int const iMaxStage, CONT_ITEMBAG & kContItemBag)
{
	CONT_MISSION_DEFENCE7_STAGE_BAG const * pkStage = NULL;
	g_kTblDataMgr.GetContDef(pkStage);
	if( !pkStage ){ return; }

	int const iMissionNo = g_iMissionNo;
	int const iPartyNumber = std::max<int>( 1, g_kParty.MemberCount() );
	int const iModeType = GetDefenceModeType( lwGetGameLevel() );

	for(int i=1; i<=iMaxStage+1; ++i)
	{
		CONT_MISSION_DEFENCE7_STAGE_BAG::key_type kKey(iMissionNo, iModeType, iPartyNumber, i);
		CONT_MISSION_DEFENCE7_STAGE_BAG::const_iterator stage_itr = pkStage->find(kKey);		
		if( stage_itr == pkStage->end() )
		{
			break;
		}

		CONT_MISSION_DEFENCE7_STAGE_BAG::mapped_type const & kElement = (*stage_itr).second;
		CONT_MISSION_DEFENCE7_STAGE::value_type const &kValue = kElement.kCont.at(0);
		if( 0 < kValue.iResult_ItemNo )
		{
			kContItemBag.push_back(kValue.iResult_ItemNo);
		}
	}
}

void GetDefence7ItemBag(int const iMaxStage, CONT_ITEMBAG & kContItemBag)
{
	CONT_ITEMBAG kContItemContainer;
	GetDefence7ItemContainer(iMaxStage, kContItemContainer);

	if( kContItemContainer.empty() )
	{
		return;
	}

	CONT_DEF_ITEM_CONTAINER const *pContContainer = NULL;
	g_kTblDataMgr.GetContDef(pContContainer);
	
	if( !pContContainer )
	{
		return;
	}

	CONT_ITEMBAG::const_iterator c_it = kContItemContainer.begin();
	while( c_it!=kContItemContainer.end() )
	{
		CONT_DEF_ITEM_CONTAINER::const_iterator item_cont_itor = pContContainer->find(*c_it);
		if(item_cont_itor != pContContainer->end())
		{
			kContItemBag.push_back(item_cont_itor->second.iRewordItem);
		}

		++c_it;
	}
}


int GetViewArrowIndex(int const iNowStage, int const iMaxStage)
{
	CONT_MISSION_DEFENCE7_STAGE_BAG const * pkStage = NULL;
	g_kTblDataMgr.GetContDef(pkStage);
	if( !pkStage ){ return 0; }

	int const iMissionNo = g_iMissionNo;
	int const iPartyNumber = std::max<int>( 1, g_kParty.MemberCount() );
	int const iModeType = GetDefenceModeType( lwGetGameLevel() );

	int iViewArrow = 0;
	int iCount = 0;
	for(int i=1; i<=iMaxStage+1; ++i)
	{
		CONT_MISSION_DEFENCE7_STAGE_BAG::key_type kKey(iMissionNo, iModeType, iPartyNumber, i);
		CONT_MISSION_DEFENCE7_STAGE_BAG::const_iterator stage_itr = pkStage->find(kKey);		
		if( stage_itr == pkStage->end() )
		{
			break;
		}

		CONT_MISSION_DEFENCE7_STAGE_BAG::mapped_type const & kElement = (*stage_itr).second;
		CONT_MISSION_DEFENCE7_STAGE::value_type const &kValue = kElement.kCont.at(0);
		if( 0 < kValue.iResult_ItemNo )
		{
			++iCount;
			if(i < iNowStage)
			{
				iViewArrow = iCount;
			}
		}
	}
	return iViewArrow;
}

void lwCallDefenceMissionReward(int const iNowStage, int const iMaxStage)
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Activate(L"FRM_DEFENCE_MISSION_RESULT");
	if( !pkWnd ){ return; }

	//
	CONT_ITEMBAG kContItemBag;
	GetDefence7ItemBag(iMaxStage, kContItemBag);
	if( kContItemBag.empty() )
	{
		pkWnd->Close();
	}

	int const iViewArrow = GetViewArrowIndex(iNowStage, iMaxStage);

	XUI::CXUI_Builder* pkBuildItem = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_ITEM"));
	int const iMaxSlot = pkBuildItem ? pkBuildItem->CountX() : 0;

	CONT_ITEMBAG::iterator bag_itr = kContItemBag.begin();
	int iRealViewArrow = iViewArrow;
	for(int i=0; i< iViewArrow-(iMaxSlot-2); ++i)
	{
		if(kContItemBag.size()<=iMaxSlot)
		{
			break;
		}
		--iRealViewArrow;
		bag_itr = kContItemBag.erase(bag_itr);
	}

	for(int i=0; i<iMaxSlot; ++i)
	{
		BM::vstring vStr(L"ICN_ITEM");
		vStr += i;

		XUI::CXUI_Wnd* pkItem = pkWnd->GetControl(vStr);
		if(pkItem)
		{
			XUI::CXUI_Wnd* pkBg = pkItem->GetControl(L"IMG_BG");
			if(bag_itr != kContItemBag.end())
			{
				DWORD iItemNo = (*bag_itr);
				pkItem->SetCustomData( &iItemNo, sizeof(iItemNo) );
				++bag_itr;

				pkBg ? pkBg->Visible(false) : 0;
			}
			else
			{
				pkItem->ClearCustomData();
				pkBg ? pkBg->Visible(true) : 0;
			}
		}
	}

	XUI::CXUI_Builder* pkBuildArrow = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_ARROW"));
	int const iMaxArrow = pkBuildArrow ? pkBuildArrow->CountX() : 0;
	for(int i=0; i<iMaxArrow; ++i)
	{
		BM::vstring vStr(L"FRM_ARROW");
		vStr += i;

		XUI::CXUI_Wnd* pkArrow = pkWnd->GetControl(vStr);
		if(pkArrow)
		{
			pkArrow->Visible( (iRealViewArrow==i) ? true : false);
		}
	}

	XUI::CXUI_Wnd* pkTmp = pkWnd->GetControl(L"FRM_TEXT");
	if( pkTmp )
	{
		int const iNextResultStage = lwGetDefenceResultStage(iNowStage);
		BM::vstring vStr(TTW(400943));
		vStr.Replace(L"#STAGE#", iNextResultStage);
		pkTmp->Text(vStr);
	}
}

SMISSION_DEFENCE7_GUARDIAN const& GetDefGuardian(int const iKey,bool const bMonsterNo=false)
{
	const CONT_MISSION_DEFENCE7_GUARDIAN_BAG *pkGuardian;
	g_kTblDataMgr.GetContDef(pkGuardian);
	
	if( false==bMonsterNo )
	{
		CONT_MISSION_DEFENCE7_GUARDIAN_BAG::const_iterator guardian_it = pkGuardian->find(iKey);
		if(guardian_it != pkGuardian->end())
		{
			return guardian_it->second.kCont.at(0);
		}
	}
	else
	{
		CONT_MISSION_DEFENCE7_GUARDIAN_BAG::const_iterator guardian_it = pkGuardian->begin();
		while(guardian_it!=pkGuardian->end())
		{
			SMISSION_DEFENCE7_GUARDIAN const & kMonster = guardian_it->second.kCont.at(0);
			if(kMonster.iMonsterNo==iKey)
			{
				return guardian_it->second.kCont.at(0);
			}
			++guardian_it;
		}
		if(guardian_it != pkGuardian->end())
		{
			return guardian_it->second.kCont.at(0);
		}
	}

	static SMISSION_DEFENCE7_GUARDIAN kNullData;
	return kNullData;
}

int GetGuardianNeedPoint(int const iKey)
{
	const CONT_MISSION_DEFENCE7_GUARDIAN_BAG *pkGuardian;
	g_kTblDataMgr.GetContDef(pkGuardian);
	if( !pkGuardian ){ return 0; }

	CONT_MISSION_DEFENCE7_GUARDIAN_BAG::const_iterator guardian_it = pkGuardian->find(iKey);
	if(guardian_it != pkGuardian->end())
	{
		SMISSION_DEFENCE7_GUARDIAN const & kMonster = guardian_it->second.kCont.at(0);
		return kMonster.iNeed_StrategicPoint;
	}

	return 0;
}

int GetSkillNeedPoint(int const iGuardianNo, int const iSkillNo)
{
	return PgDefenceMissionSkill::GetGuardianSkillPoint(iGuardianNo, iSkillNo);
}

int GetMyTeam()
{
	if( IsDefenceMode7() )
	{
		return 1;
	}
	else
	{
		CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
		if(pkUnit)
		{
			return pkUnit->GetAbil(AT_TEAM);
		}
	}

	return 0;
}
int GetMyTeamPoint()
{
	return g_kMissionMgr.GetStrategicPoint(GetMyTeam());
}

void lwUpdateDefenceMissionGuardian(lwUIWnd kWnd)
{
	if( kWnd.IsNil() ) 
	{
		return;
	}

	const CONT_MISSION_DEFENCE7_MISSION_BAG *pkMission;
	const CONT_MISSION_DEFENCE7_GUARDIAN_BAG *pkGuardian;
	g_kTblDataMgr.GetContDef(pkMission);
	g_kTblDataMgr.GetContDef(pkGuardian);
	if( !pkMission || !pkGuardian )
	{
		return;
	}

	int const iModeType = GetDefenceModeType( lwGetGameLevel() );
	CONT_MISSION_DEFENCE7_MISSION_BAG::key_type		kKey(lwGetMissionNo(), iModeType);

	CONT_MISSION_DEFENCE7_MISSION_BAG::const_iterator iter = pkMission->find(kKey);
	if( iter == pkMission->end() || iter->second.kCont.empty() )
	{
		return;
	}

	SMISSION_DEFENCE7_MISSION kValue = iter->second.kCont.at(0);
	
	XUI::CXUI_Wnd* pkWnd = kWnd();
	XUI::CXUI_Builder* pkBuildItem = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_MONS"));
	if( !pkBuildItem )
	{
		return;
	}
	int const iCount = pkBuildItem->CountX() * pkBuildItem->CountY();
	for(int i=0; i<iCount; ++i)
	{
		BM::vstring vStr(L"FRM_MON");
		vStr += i;

		XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
		if(!pkFrm)
		{
			continue;
		}

		int iGuardianNo = 0;
		int iMonsterNo = 0;
		int iNeedPoint = 0;
		bool bGrayScale = true;
		XUI::CXUI_Wnd* pkIcon = pkFrm->GetControl(L"ICN_MON");
		if(pkIcon)
		{
			int const iKey = *((&kValue.iSlot_F1)+i);

			if( iKey > 0 )
			{
				SMISSION_DEFENCE7_GUARDIAN kGuardian = GetDefGuardian(iKey);

				GuardianInfo kInfo;
				kInfo.iGuardianNo = iGuardianNo = iKey;
				kInfo.iMonsterNo = iMonsterNo = kGuardian.iMonsterNo;
				iNeedPoint = kGuardian.iNeed_StrategicPoint;

				pkIcon->SetCustomData( &kInfo, sizeof(kInfo) );
				bGrayScale = iNeedPoint > GetMyTeamPoint();
			}
			else
			{
				pkIcon->ClearCustomData();
			}
			pkIcon->GrayScale(bGrayScale);
		}

		g_kMissionMgr.SetUI(pkFrm, iGuardianNo, iMonsterNo, BM::vstring(iNeedPoint), bGrayScale);
	}

	SetPointUI(pkWnd);
}

void lwUpdateDefenceMissionSkill(lwUIWnd kWnd)
{
	if( kWnd.IsNil() ) 
	{
		return;
	}

	XUI::CXUI_Wnd* pkWnd = kWnd();
	XUI::CXUI_Builder* pkBuildItem = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SKILLS"));
	if( !pkBuildItem )
	{
		return;
	}

	for(int i=0; i<pkBuildItem->CountX(); ++i)
	{
		BM::vstring vStr(L"FRM_SKILL");
		vStr += i;

		XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
		if(!pkFrm)
		{
			continue;
		}

		XUI::CXUI_Wnd* pkIcon = pkFrm->GetControl(L"ICN_SKILL");
		if(pkIcon)
		{
			pkIcon->ClearCustomData();
		}
	}
}

bool GetDefenceAddMonsterInfo(int const iStageNo, int const iSelect_SuccessCount, CONT_DEF_DEFENCE_ADD_MONSTER::mapped_type& kElement)
{
	CONT_MISSION_DEFENCE_STAGE_BAG::mapped_type kStageData;
	if( GetDefenceStage(iStageNo, kStageData) )	
	{
		CONT_MISSION_DEFENCE_STAGE::value_type &kValue = kStageData.kCont.at(0);
		return GetDefenceAddMonster(kValue.iAddMonster_GroupNo, iSelect_SuccessCount, kElement);
	}
	return false;
}

bool GetDefenceStage(int const iStage, CONT_MISSION_DEFENCE_STAGE_BAG::mapped_type& kElement)
{
	const CONT_MISSION_DEFENCE_STAGE_BAG *pkStage;
	g_kTblDataMgr.GetContDef(pkStage);
	if( !pkStage )
	{
		return false;
	}
	else
	{
		int const iPartyMemberCount = std::max<int>( 1, g_kParty.MemberCount() );

		CONT_MISSION_DEFENCE_STAGE_BAG::key_type kKey(lwGetMissionNo(), iPartyMemberCount, iStage);

		CONT_MISSION_DEFENCE_STAGE_BAG::const_iterator stage_itr = pkStage->find(kKey);		
		if( stage_itr == pkStage->end() )
		{
			// 더이상 진행할 스테이지 없음
			return false;
		}
		else
		{
			kElement = (*stage_itr).second;

			return true;
		}
	}

	return false;
}

bool GetDefenceAddMonster(int const iMonsterGroupNo, int const iSelect_SuccessCount, CONT_DEF_DEFENCE_ADD_MONSTER::mapped_type& kElement)
{
	CONT_DEF_DEFENCE_ADD_MONSTER const *pkAddMonsterBag;
	g_kTblDataMgr.GetContDef(pkAddMonsterBag);
	if( !pkAddMonsterBag )
	{
		return false;
	}

	CONT_DEF_DEFENCE_ADD_MONSTER::key_type kKey(iMonsterGroupNo, iSelect_SuccessCount);
	CONT_DEF_DEFENCE_ADD_MONSTER::const_iterator AddMonster_itor = pkAddMonsterBag->find(kKey);
	if( pkAddMonsterBag->end() == AddMonster_itor )
	{
		return false;
	}

	kElement = (*AddMonster_itor).second;
	return true;
}

bool lwStrategyDefenceResultUI(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pkWnd = kWnd();
	if( !pkWnd )
	{
		pkWnd = XUIMgr.Call(std::wstring(L"FRM_DEFENCE_RESULT"));
	}
	if( !pkWnd )
	{ 
		return false;
	}

	PgMission_ResultC const kMission_Result = g_kMissionComplete.GetMission_ResultC();
	//
	CONT_ITEMBAG kContItemBag;
	if( kMission_Result.IsLastStage() )
	{
		GetDefence7ItemBag(kMission_Result.StageCount(), kContItemBag);
	}
	else
	{
		GetDefence7ItemBag(g_kMissionMgr.NowStage()-1, kContItemBag);
	}

	XUI::CXUI_Builder* pkBuildItem = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_BONUS"));
	if( pkBuildItem )
	{
		CONT_ITEMBAG::const_iterator bag_itr = kContItemBag.begin();
		XUI::CXUI_Wnd *pkIcon = NULL;
		XUI::CXUI_Wnd *pkIconBg = NULL;
		for(int i=0; i<pkBuildItem->CountX(); ++i)
		{
			BM::vstring vStr(L"SFRM_BONUS");
			vStr += i;

			XUI::CXUI_Wnd* pkItem = pkWnd->GetControl(vStr);
			if( !pkItem ){ continue; }

			pkIcon = pkItem->GetControl(L"IMG_BONUS");
			pkIconBg = pkItem->GetControl(L"IMG_BONUS_BG");
			if( !pkIcon || !pkIconBg ){ continue; }
			
			if(bag_itr != kContItemBag.end())
			{
				DWORD iItemNo = (*bag_itr);

				PgBase_Item kItem;
				CreateSItem(iItemNo, 1, 0, kItem);
				
				BM::Stream kCustomData;
				kItem.WriteToPacket(kCustomData);
				pkIcon->SetCustomData(kCustomData.Data());
				lwDrawMissionRewardItem(lwUIWnd(pkIcon, pkIcon != NULL));
				pkIconBg->Visible(false);

				++bag_itr;
			}
			else
			{
				pkIcon->ClearCustomData();
				pkIconBg->Visible(true);
			}
		}
	}
	pkWnd->SetInvalidate();

	CXUI_Button* pkButton = dynamic_cast<CXUI_Button*>(pkWnd->GetControl(_T("BTN_QS")));
	if( pkButton )
	{
		pkButton->Visible(true);

		if( MT_MISSION == kMission_Result.GetMissionType() )
		{
			pkButton->Disable(false);
		}
		else
		{
			pkButton->Disable(true);
		}
	}

	XUI::CXUI_Wnd *pkGadaWnd = pkWnd->GetControl(std::wstring(_T("SFRM_GADACOIN")));
	if( pkGadaWnd )
	{
		pkGadaWnd->Visible(false);

		XUI::CXUI_Image* const pkIcon = dynamic_cast<XUI::CXUI_Image*>(pkGadaWnd->GetControl(_T("IMG_GADACOIN")));
		if( pkIcon )
		{
			pkIcon->ClearCustomData();
		}
	}

	XUI::CXUI_Wnd *pkMyForm0 = pkWnd->GetControl(_T("FRM_COIN_TEXT0"));
	if( pkMyForm0 )
	{
		TCHAR szTemp[512] = {0 ,};
		std::wstring kTTWForm = TTW(401054);

		if( MT_MISSION == kMission_Result.GetMissionType() )
		{
			_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(kMission_Result.GetGadaCoinNeedCount()));
		}
		else
		{
			_sntprintf_s(szTemp, 512, kTTWForm.c_str(), 0);
		}
		
		std::wstring kLog = szTemp;	

		pkMyForm0->Text(kLog);
	}

	XUI::CXUI_Wnd *pkMyForm = pkWnd->GetControl(_T("FRM_COIN_TEXT1"));
	if( pkMyForm )
	{
		TCHAR szTemp[512] = {0 ,};
		std::wstring kTTWForm = TTW(401054);
		_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(kMission_Result.GetGadaCoin()));
		std::wstring kLog = szTemp;	

		pkMyForm->Text(kLog);
	}

	XUI::CXUI_Wnd *pkMyForm2 = pkWnd->GetControl(_T("FRM_COIN_TEXT2"));
	if( pkMyForm2 )
	{
		if( MT_MISSION == kMission_Result.GetMissionType() )
		{
			pkMyForm2->Visible(true);
		}
		else
		{
			pkMyForm2->Visible(false);
		}
	}

	XUI::CXUI_Wnd *m_pkExitWnd = pkWnd->GetControl(std::wstring(_T("SFRM_EXIT_BG")));
	if( m_pkExitWnd )
	{
		m_pkExitWnd->Visible(false);
	}

	return true;
}

bool lwIsMissionLastStage()
{
	return g_kMissionComplete.GetMission_ResultC().IsLastStage();
}

void SetPointText( XUI::CXUI_Wnd* pkWnd, int const iPoint )
{
	if( !pkWnd )
	{
		return;
	}

	pkWnd->Text( BM::vstring(iPoint) );
}

void lwDisplayStrategicPoint(lwUIWnd kWnd)
{
	if( kWnd.IsNil() )
	{
		return;
	}
	
	lua_tinker::call<void,lwUIWnd,int>("SetDefencePoint", kWnd.GetControl("FRM_POINT_RED"),g_kMissionMgr.GetStrategicPoint(1));
	lua_tinker::call<void,lwUIWnd,int>("SetDefencePoint", kWnd.GetControl("FRM_POINT_BLUE"),g_kMissionMgr.GetStrategicPoint(2));
	//SetPointText( kWnd.GetControl("FRM_POINT_RED")(), g_kMissionMgr.GetStrategicPoint(1) );
	//SetPointText( kWnd.GetControl("FRM_POINT_BLUE")(), g_kMissionMgr.GetStrategicPoint(2) );
}

void lwUpdateStrategicPointUI()
{
	lwDisplayStrategicPoint( lwUIWnd(XUIMgr.Get(L"FRM_DEFENCE_F7_CENTER_INFO")) );
	lwUpdateDefenceMissionGuardian( lwUIWnd(XUIMgr.Get(L"SFRM_INSTALL_GUARDIAN")) );
}

void lwStrategyItemDoAction(int const iIndex)
{
	g_kMissionMgr.StrategyItemDoAction(iIndex);
}

void lwMissionMgrInit()
{
	g_kMissionMgr.Init();
}

lwPoint3 lwGetDefenceGuardianNeedPos()
{
	return g_kMissionMgr.GetNeedGuardianPos();
}

void lwViewGuardianDetectRangeEffect(lwUIWnd kWnd)
{
	g_kMissionMgr.ViewGuardianDetectRangeEffect(kWnd());
}

void lwClearGuardianDetectRangeEffect()
{
	g_kMissionMgr.ViewGuardianDetectRangeEffect(NULL);
}

void lwOnDefenceGuardianMouseOver(lwUIWnd kWnd)
{
	g_kMissionMgr.OnGuardianMouseOver(kWnd);
}

void lwOnDefenceGuardianMouseOut()
{
	g_kMissionMgr.OnGuardianMouseOut();
}

bool lwIsGuardianInstall(lwGUID kGuid)
{
	return g_kMissionMgr.IsGuardianInstall( kGuid() );
}

bool lwIsGuardianInstallStr(char const * pkTriggerName)
{
	if(pkTriggerName)
	{
		return g_kMissionMgr.IsGuardianInstall( UNI(pkTriggerName) );
	}
	return false;
}

void lwSetGuardianInstallLoc(char const* pkLoc)
{
	if(pkLoc)
	{
		g_kMissionMgr.SetGuardianInstallLoc( BM::vstring(pkLoc) );
	}
}

void lwCallGuardianUI(lwGUID kGuid)
{
	g_kMissionMgr.CallGuardianUI( kGuid() );
}

void lwCallInstallGuardian(lwUIWnd kWnd)
{
	g_kMissionMgr.CallInstallGuardian(kWnd());
}

void lwCallUpgradeGuardian(lwUIWnd kWnd)
{
	g_kMissionMgr.CallUpgradeGuardian( kWnd );
}

void lwCallRemoveGuardian()
{
	g_kMissionMgr.CallRemoveGuardian();
}

void lwCallDefenceSkillLearn(lwUIWnd kWnd)
{
	g_kMissionMgr.CallDefenceSkillLearn(kWnd());
}

void lwOnCallDefenceLearnSkill(lwUIWnd kWnd)
{
	g_kMissionMgr.OnCallDefenceLearnSkill( kWnd() );
}

lwWString lwGetGuardianPosStr(lwGUID kGuid)
{
	return g_kMissionMgr.GuardianPosStr(kGuid());
}

void lwOnEnterGuardianInstallTrigger(char const* pkTriggerName, lwPoint3 kPos)
{
	if(!pkTriggerName)
	{
		return;
	}

	g_kMissionMgr.OnEnterGuardianInstallTrigger(BM::vstring(pkTriggerName), NiPoint3(kPos.GetX(),kPos.GetY(),kPos.GetZ()));
}

void lwOnLeaveGuardianInstallTrigger()
{
	g_kMissionMgr.OnLeaveGuardianInstallTrigger();
}

std::wstring GetMonsterName(int const iMonsterNo)
{
	GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
	const CMonsterDef *pDef = kMonsterDefMgr.GetDef(iMonsterNo);
	if (!pDef)
	{
		return L"";
	}

	const wchar_t *pName = NULL;
	GetDefString(pDef->NameNo(), pName);
	return pName ? pName : L"";
}

typedef enum {
	ED7GT_NONE		= 0,
	ED7GT_TRANSFOR	= 1,
	ED7GT_UPGRADE	= 2,
} EDEFENCE7_GUARDIANTYPE;

PgMissionMgr::PgMissionMgr()
	: m_kNowStage(0)
	, m_kStagePoint(0)
	, m_iGuardianUpgradeNo(0)
	, m_iSelectGuardianDetectRangeSlot(-1)
	, m_iStageClearPoint(0)
	, m_iEnterGuardianTriggerEffectSlotNo(0)
	, m_iMode6ClearQuestNo(0)
	, m_iMode7ClearQuestNo(0)
{
}

PgMissionMgr::~PgMissionMgr()
{
}

void PgMissionMgr::Init()
{
	Clear();

	PgActor * pkActor = g_kPilotMan.GetPlayerActor();
	if( !pkActor || !g_pkWorld ){ return; }

	if( IsDefenceMode7() || IsDefenceMode8() )
	{
		int const SECTOR_MAX = 10;
		//int const SECTOR_SUB_MAX = 9;
		NiPoint3 kTriggerLoc;
		for(int i=0; i<SECTOR_MAX; ++i)
		{
			for(int j=0; j<SECTOR_MAX; ++j)
			{
				BM::vstring vPos(L"Guardian_");
				vPos  += i;
				vPos  += j;

				if( g_pkWorld->FindTriggerLoc(MB(vPos), kTriggerLoc) )
				{
					m_kContGuardianTriggerPos.insert( std::make_pair(vPos,kTriggerLoc) );
				}
			}
		}
	}
}

void PgMissionMgr::Clear()
{
	m_kTempGuardianSlot.clear();
	m_kContStrategicPoint.clear();
	m_kContDefenceMyItem.clear();
	m_kContGuardianTriggerPos.clear();

	m_kTempGuardianInstall.iGuardianNo = 0;
	m_kTempGuardianInstall.kGuid.Clear();
	m_kGuardianInstall.clear();
	m_kDefenceSkill.Clear();

	NowStage(0);
	StagePoint(0);

	m_iSelectGuardianDetectRangeSlot = -1;
	m_kGuardianUpgradeLoc.clear();
	m_iGuardianUpgradeNo = 0;
	ClearGuardianDetectRangeEffect();
	m_kMinimapGuardian.clear();
	m_iStageClearPoint = 0;

	//OnLeaveGuardianInstallTrigger();
}

void PgMissionMgr::SetClearQuestNo(VEC_INT const& kContQuest)
{
	m_iMode7ClearQuestNo = 0;
	if( kContQuest.empty() )
	{
		return;
	}

	m_iMode7ClearQuestNo = kContQuest[0];
}

void PgMissionMgr::SetClearQuestNoLv6(VEC_INT const& kContQuest)
{
	m_iMode6ClearQuestNo = 0;
	if( kContQuest.empty() )
	{
		return;
	}

	m_iMode6ClearQuestNo = kContQuest[0];
}

int PgMissionMgr::GetClearQuestNo(int const Level)const
{
	if( 6 == Level )
	{
		return m_iMode6ClearQuestNo;
	}
	else if( 7 == Level )
	{
		return m_iMode7ClearQuestNo;
	}
	return 0;
}

bool PgMissionMgr::IsMode7ComplatedQuest()const
{
	if( 0!=m_iMode7ClearQuestNo )
	{
		return lwIsEndedQuest(m_iMode7ClearQuestNo);
	}
	return true;
}

bool PgMissionMgr::IsMode6ComplatedQuest()const
{
	if( 0!=m_iMode6ClearQuestNo )
	{
		return lwIsEndedQuest(m_iMode6ClearQuestNo);
	}
	return true;
}

std::wstring PgMissionMgr::GetClearQuestOriginalText(int const Level)const
{
	BM::vstring vStr;
	PgQuestInfo const* pkQuestInfo = NULL;
	if( 6 == Level )
	{
		pkQuestInfo = g_kQuestMan.GetQuest( m_iMode6ClearQuestNo );
	}
	else if( 7 == Level )
	{
		pkQuestInfo = g_kQuestMan.GetQuest( m_iMode7ClearQuestNo );
	}
	if( pkQuestInfo )
	{
		vStr = TTW(pkQuestInfo->m_iTitleTextNo);
	}
	return static_cast<std::wstring>(vStr);
}

std::wstring PgMissionMgr::GetClearQuestText(int const iLevel)const
{
	BM::vstring vStr;
	if( 7==iLevel )//전략모드인경우
	{
		PgQuestInfo const* pkQuestInfo = g_kQuestMan.GetQuest( m_iMode7ClearQuestNo );
		if( pkQuestInfo )
		{
			int iMapNo = 0;
			std::wstring kNpcName;
			if( !pkQuestInfo->m_kNpc_Payer.empty() )
			{
				SQuestNpc const& rkNPC = (*pkQuestInfo->m_kNpc_Payer.begin());
				VEC_INT kMapArray;
				if( g_kWorldMapUI.GetNpcGuidToMapNo(rkNPC.kNpcGuid, kMapArray) )
				{
					iMapNo = kMapArray[0];
				}

				SNpcCreateInfo kInfo;
				if( g_kWorldMapUI.GetNpc(rkNPC.kNpcGuid, kInfo) )
				{
					kNpcName = kInfo.wstrName;
				}
			}

			vStr = TTW(8014);
			vStr.Replace(L"#QUESTNAME#", TTW(pkQuestInfo->m_iTitleTextNo));

			if( 0!=iMapNo && !kNpcName.empty() )
			{
				vStr += L"\n";
				vStr += TTW(8015);
				vStr.Replace(L"#MAP#", GetMapName(iMapNo));
				vStr.Replace(L"#NPC#", kNpcName);
			}
		}
	}
	return static_cast<std::wstring>(vStr);
}

int PgMissionMgr::GetStrategicPoint(int const iTeam)const
{
	CONT_STRATEGIC_POINT::const_iterator it = m_kContStrategicPoint.find(iTeam);
	if(it != m_kContStrategicPoint.end())
	{
		return (*it).second;
	}
	return 0;
}

int PgMissionMgr::GetDefenceMyItem(int const iIndex)const
{
	if(iIndex < m_kContDefenceMyItem.size())
	{
		return m_kContDefenceMyItem.at(iIndex);
	}

	return 0;
}

void PgMissionMgr::GurdianDoAction(int const iGuardinaNo)
{
	if( !iGuardinaNo )
	{
		return;
	}

	std::wstring kGuardianPosStr = m_kGuardianUpgradeLoc;
	if(false==kGuardianPosStr.empty())
	{
		BM::Stream kPacket(PT_C_M_REQ_GUARDIAN_SET);
		kPacket.Push( iGuardinaNo );
		kPacket.Push( kGuardianPosStr );
		NETWORK_SEND(kPacket);

		XUIMgr.Close(L"SFRM_INSTALL_GUARDIAN");
	}
}

std::wstring PgMissionMgr::GetNeedGuardianPosStr()const
{
	static std::wstring const kNullData;
	PgActor * pkActor = g_kPilotMan.GetPlayerActor();
	if( !pkActor ){ return kNullData; }

	NiPoint3 const kActorPos = pkActor->GetPos();
	float fMinDistance = 9999999.f;
	std::wstring kRtStr;
	
	CONT_TRIGGERPOS::const_iterator c_iter = m_kContGuardianTriggerPos.begin();
	while(c_iter != m_kContGuardianTriggerPos.end())
	{
		float const fDistance = (c_iter->second - kActorPos).Length();

		if(fMinDistance > fDistance)
		{
			fMinDistance = fDistance;
			kRtStr = c_iter->first;
		}
		++c_iter;
	}

	return kRtStr;
}

NiPoint3 PgMissionMgr::GetNeedGuardianPos()const
{	
	CONT_TRIGGERPOS::const_iterator c_iter = m_kContGuardianTriggerPos.find( GetNeedGuardianPosStr() );
	if(c_iter != m_kContGuardianTriggerPos.end())
	{
		return c_iter->second;
	}

	static NiPoint3 const kNullData(0,0,0);
	return kNullData;
}

void PgMissionMgr::RemoveTempGuardian()
{
	if( !g_pkWorld ){ return; }

	if( m_kTempGuardianInstall.kGuid.IsNotNull() )
	{
		g_pkWorld->RemoveObjectOnNextUpdate(m_kTempGuardianInstall.kGuid);
	}

	VEC_INT::const_iterator c_iter = m_kTempGuardianSlot.begin();
	while( c_iter!=m_kTempGuardianSlot.end() )
	{
		g_pkWorld->DetachParticle(*c_iter);
		++c_iter;
	}
	m_kTempGuardianSlot.clear();
}

void PgMissionMgr::UpdateEffectGuardianPos()
{
	BM::CAutoMutex kLock(m_kMutex);
	if( !g_pkWorld ){ return; }

	static VEC_INT kContTemp;
	VEC_INT::const_iterator del_it = kContTemp.begin();
	while(del_it!=kContTemp.end())
	{
		g_pkWorld->DetachParticle(*del_it);		
		++del_it;
	}
	kContTemp.clear();

	float const fPos = 20.f;
	CONT_TRIGGERPOS::const_iterator trigger_it = m_kContGuardianTriggerPos.begin();
	while(trigger_it != m_kContGuardianTriggerPos.end())
	{
		CONT_GUARDIAN_INSTALL::const_iterator c_it = m_kGuardianInstall.find((*trigger_it).first);
		if(c_it == m_kGuardianInstall.end())
		{
			PgParticle *pkParticle = g_kParticleMan.GetParticle("EF_Pattern_mission_guarian_zone");
			if(pkParticle)
			{
				pkParticle->SetScale(0.28f);
				pkParticle->SetLoop(true);
				NiPoint3 kEffectPos = (*trigger_it).second;
				kEffectPos.z += fPos;
				NiPoint3 kFloorPos = g_pkWorld->ThrowRay(kEffectPos,NiPoint3(0,0,-1),500);
				
				int const iSlot = g_pkWorld->AttachParticle(pkParticle, kFloorPos);
				kContTemp.push_back(iSlot);
			}
		}
		++trigger_it;
	}
}

void PgMissionMgr::OnEnterGuardianInstallTrigger(BM::vstring const & rkTriggerName, NiPoint3 kPos)
{
	if(!g_pkWorld)
	{
		return;
	}
	BM::CAutoMutex kLock(m_kMutex);

	if( false==IsGuardianInstall( rkTriggerName.operator std::wstring const&() ) )
	{
		PgParticle *pkParticle = g_kParticleMan.GetParticle("eff_common_defence_light00");
		if(pkParticle)
		{
			pkParticle->SetScale(1.2f);
			kPos.z += 50;
			kPos = g_pkWorld->ThrowRay(kPos,NiPoint3(0,0,-1),500);
			
			m_iEnterGuardianTriggerEffectSlotNo = g_pkWorld->AttachParticle(pkParticle, kPos);
		}
	}
}

void PgMissionMgr::OnLeaveGuardianInstallTrigger()
{
	BM::CAutoMutex kLock(m_kMutex);

	if(g_pkWorld && m_iEnterGuardianTriggerEffectSlotNo)
	{
		g_pkWorld->DetachParticle(m_iEnterGuardianTriggerEffectSlotNo);
		m_iEnterGuardianTriggerEffectSlotNo = 0;
	}
}

void PgMissionMgr::UpdateGuardianName()
{
	if( !g_pkWorld ){ return; }

	CONT_GUARDIAN_INSTALL::const_iterator c_it = m_kGuardianInstall.begin();
	while(c_it != m_kGuardianInstall.end())
	{
		PgPilot * pkPilot = g_kPilotMan.FindPilot((*c_it).second.kGuid);
		if(pkPilot)
		{
			SMISSION_DEFENCE7_GUARDIAN kGuardian = GetDefGuardian((*c_it).second.iGuardianNo);
			pkPilot->SetName( GetMonsterName(kGuardian.iMonsterNo) );
			
			PgActor* pkActor = dynamic_cast<PgActor*>( pkPilot->GetWorldObject() );
			pkActor ? pkActor->UpdateName() : 0;
		}
		++c_it;
	}
}

void PgMissionMgr::UpdateMinimapGuardian()
{
	m_kMinimapGuardian.clear();
	CONT_GUARDIAN_INSTALL::const_iterator c_it = m_kGuardianInstall.begin();
	while(c_it != m_kGuardianInstall.end())
	{
		PgPilot * pkPilot = g_kPilotMan.FindPilot((*c_it).second.kGuid);
		if( pkPilot )
		{
			PgActor* pkActor = dynamic_cast<PgActor* >(pkPilot->GetWorldObject());
			if( pkActor )
			{
				CONT_MISSION_UNIT_POS::value_type kData;
				kData.kGuid = (*c_it).second.kGuid;
				kData.kPos = pkActor->GetPos();
				kData.iTeam = pkPilot->GetAbil(AT_TEAM);

				m_kMinimapGuardian.push_back(kData);
			}
		}
		++c_it;
	}
}

//const int MON_REALDETECTRANGE_DIFF = -50;
const char * MINIMAPCAMERA_NAME[TEAM_MAX] = {"mini_01", "mini_01", "mini_02"};
void PgMissionMgr::SetDefenceMinimapCamera()const
{
	if( !g_pkWorld ){ return; }

	int const iTeam = (GetMyTeam() < TEAM_MAX ? GetMyTeam() : 0);
	g_pkWorld->SetMinimapCamera(MINIMAPCAMERA_NAME[iTeam]);	
}

int GetGuardianAbil(int const iMonAbilNo, int const iType)
{
	CONT_DEFMONSTERABIL const* pkDefMonAbil = NULL;
	g_kTblDataMgr.GetContDef(pkDefMonAbil);
	if(pkDefMonAbil)
	{
		CONT_DEFMONSTERABIL::const_iterator c_it = pkDefMonAbil->find(iMonAbilNo);
		if(c_it!=pkDefMonAbil->end())
		{
			CONT_DEFMONSTERABIL::mapped_type const& abil = (*c_it).second;
			for(int i=0; i<MAX_MONSTER_ABIL_ARRAY; ++i)
			{
				if(abil.aType[i]==iType)
				{
					return abil.aValue[i];
				}
			}
		}
	}

	return 0;
}

int GetDefMonsterAbil(int const iMonsterNo, int const iAbil)
{
	// 1. GuardianAbil 찾기
	SMISSION_DEFENCE7_GUARDIAN const& kGuardian = GetDefGuardian(iMonsterNo,true);
	if( kGuardian.iMonsterNo )
	{
		int iValue = 0;
		if( 0==iValue ){ iValue = GetGuardianAbil(kGuardian.iGuardian_Abil01, iAbil); }
		if( 0==iValue ){ iValue = GetGuardianAbil(kGuardian.iGuardian_Abil02, iAbil); }
		if( 0!=iValue ){ return iValue; }
	}

	// 2. MonsterAbil 찾기
	GET_DEF(CMonsterDefMgr, kMonsterDefMgr);
	const CMonsterDef *pDefMonster = kMonsterDefMgr.GetDef(iMonsterNo);
	if( pDefMonster )
	{
		int iValue = pDefMonster->GetAbil(iAbil);
		if( 0!=iValue ){ return iValue; }
	}

	// 3. ClassAbil 찾기
	int iLevel = iMonsterNo%100;
	iLevel = iLevel ? iLevel/10 : 10;

	GET_DEF(PgClassDefMgr, kClassDefMgr);
	CLASS_DEF_BUILT const* pkClassDef = kClassDefMgr.GetDef(SClassKey(iMonsterNo,iLevel));
	if( pkClassDef )
	{
		int iValue = 0;
		CLASSDEF_ABIL_CONT::const_iterator c_it = pkClassDef->kAbil.find(iAbil);
		if(c_it != pkClassDef->kAbil.end())
		{
			iValue = (*c_it).second;
		}
		
		if( 0!=iValue ){ return iValue; }
	}

	return 0;
}

int GetCalcDefMonsterAbil(int const iMonsterNo, int const iAbil)
{
	int iValue = 0;
	switch(iAbil)
	{
	case AT_R_ATTACK_RANGE:
		{
			iValue = GetDefMonsterAbil(iMonsterNo, AT_DETECT_RANGE);
		}break;
	default:
		{
			iValue = GetDefMonsterAbil(iMonsterNo, iAbil);
		}break;
	}
	
	switch(iAbil)
	{
	case AT_R_ATTACK_RANGE:
	case AT_DETECT_RANGE:
		{
			return iValue;
		}break;
	case AT_R_PHY_ATTACK_MAX:
		{
			float const fValue = static_cast<float>(iValue)/ABILITY_RATE_VALUE64;
			return GetDefMonsterAbil(iMonsterNo,AT_PHY_ATTACK_MIN) * (1+fValue);
		}break;
	case AT_R_MAGIC_ATTACK_MAX:
		{
			float const fValue = static_cast<float>(iValue)/ABILITY_RATE_VALUE64;
			return GetDefMonsterAbil(iMonsterNo,AT_MAGIC_ATTACK_MIN) * (1+fValue);
		}break;
	}
	return iValue;
}

float GetMonsterScale(int const iMonsterNo)
{
	int const iScale = GetCalcDefMonsterAbil(iMonsterNo,AT_UNIT_SCALE);
	return (iScale ? iScale/ABILITY_RATE_VALUE_FLOAT : 1.f);
}

bool IsGuardianSkill(int const iSkillNo)
{
	return PgDefenceMissionSkill::IsGuardianSkill(iSkillNo);
}

CEffect* GetGuardianSkill(int const iSkillNo, BM::GUID const & kGuardianGuid)
{
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	CSkillDef const* pkSkill = kSkillDefMgr.GetDef(iSkillNo);
	if( !pkSkill )
	{
		return NULL;
	}
	int const iEffectNo = pkSkill->GetEffectNo();

	PgPilot * pkGuardianPilot = g_kPilotMan.FindPilot(kGuardianGuid);
	CUnit* pkGuardianUnit = pkGuardianPilot ? pkGuardianPilot->GetUnit() : NULL;
	CEffect* pkEffect = pkGuardianUnit ? pkGuardianUnit->GetEffect(iEffectNo) : NULL;
	return pkEffect;
}

void PgMissionMgr::ClearGuardianDetectRangeEffect()
{
	if(g_pkWorld && m_iSelectGuardianDetectRangeSlot >= 0)
	{
		g_pkWorld->DetachParticle(m_iSelectGuardianDetectRangeSlot);
		m_iSelectGuardianDetectRangeSlot = -1;
	}
}

void PgMissionMgr::ViewGuardianDetectRangeEffect(XUI::CXUI_Wnd* pkWnd)
{
	if( !g_pkWorld )
	{
		return; 
	}

	ClearGuardianDetectRangeEffect();

	if(pkWnd && !m_kGuardianUpgradeLoc.empty())
	{
		NiPoint3 kEffectPos;
		CONT_TRIGGERPOS::const_iterator c_iter = m_kContGuardianTriggerPos.find( m_kGuardianUpgradeLoc );
		if(c_iter != m_kContGuardianTriggerPos.end())
		{
			kEffectPos = c_iter->second;
		}

		if( kEffectPos != NiPoint3::ZERO )
		{
			float const fPos = 20.f;
			kEffectPos.z += fPos;
			NiPoint3 kFloorPos = g_pkWorld->ThrowRay(kEffectPos,NiPoint3(0,0,-1),500);
			kFloorPos.z += 4.f;

			GuardianInfo kInfo;
			pkWnd->GetCustomData(&kInfo, sizeof(kInfo));
			if(kInfo.iMonsterNo > 0)
			{
				PgParticle *pkParticle = g_kParticleMan.GetParticle( lua_tinker::call<char*>("GuardianDetectRangeEffectName") );
				float const fEffectSize = lua_tinker::call<float>("GuardianDetectRangeEffectSize");
				if(pkParticle)
				{
					pkParticle->SetScale(GetCalcDefMonsterAbil(kInfo.iMonsterNo,AT_DETECT_RANGE) / fEffectSize);
					pkParticle->SetLoop(true);
					m_iSelectGuardianDetectRangeSlot = g_pkWorld->AttachParticle(pkParticle, kFloorPos);
				}
			}
		}
	}
}

void PgMissionMgr::OnGuardianMouseOver(lwUIWnd & kWnd)
{
	if( kWnd.IsNil() ){ return; }
	if( !g_pkWorld ){ return; }
	
	PgActor * pkActor = g_kPilotMan.GetPlayerActor();
	if( !pkActor ){ return ; }

	kWnd.CallIconToolTip();
	
	/*NiPoint3 kNeedPos = GetNeedGuardianPos();
	if( kNeedPos==NiPoint3::ZERO )
	{
		return;
	}*/
	
	GuardianInfo kInfo;
	kWnd()->GetCustomData(&kInfo, sizeof(kInfo));

	if( kInfo.iMonsterNo < 1 )
	{
		return;
	}

	float fMinDistance = 9999999.f;
	NiPoint3 kNeedPos(0,0,0);
	CONT_TRIGGERPOS::const_iterator c_iter = m_kContGuardianTriggerPos.begin();
	while(c_iter != m_kContGuardianTriggerPos.end())
	{
		float const fDistance = (c_iter->second - pkActor->GetPos()).Length();
		if(fMinDistance > fDistance)
		{
			fMinDistance = fDistance;
			kNeedPos = c_iter->second;
			m_kGuardianUpgradeLoc = c_iter->first;
		}
		++c_iter;
	}
	if( kNeedPos==NiPoint3::ZERO )
	{
		return;
	}
	
	RemoveTempGuardian();
	m_kTempGuardianInstall.kGuid.Generate();
	
	if( IsGuardianInstall() )
	{ 
		return;
	}

	PgPilot * pkGuardian = g_kPilotMan.NewPilot(m_kTempGuardianInstall.kGuid, kInfo.iMonsterNo);
	if( pkGuardian )
	{
		g_pkWorld->AddObject(m_kTempGuardianInstall.kGuid, pkGuardian->GetWorldObject(), kNeedPos, OGT_ENTITY);
		PgActor* pkGuActor = dynamic_cast<PgActor* >(pkGuardian->GetWorldObject());
		if( pkGuActor )
		{
			PgParticle *pkParticle = g_kParticleMan.GetParticle( lua_tinker::call<char*>("GuardianDetectRangeEffectName") );
			float const fEffectSize = lua_tinker::call<float>("GuardianDetectRangeEffectSize");
			if(pkParticle)
			{
				pkParticle->SetScale(GetCalcDefMonsterAbil(kInfo.iMonsterNo,AT_DETECT_RANGE) / fEffectSize);
				pkParticle->SetLoop(true);
				float const fPos = 20.f;
				NiPoint3 kEffectPos = pkGuActor->GetPos();
				kEffectPos.z += fPos;
				NiPoint3 kFloorPos = g_pkWorld->ThrowRay(kEffectPos,NiPoint3(0,0,-1),500);
				kFloorPos.z += 4.f;
				int const iSlot = g_pkWorld->AttachParticle(pkParticle, kFloorPos);
				m_kTempGuardianSlot.push_back(iSlot);
			}
			pkGuActor->ReserveTransitAction("a_idle");
			pkGuActor->LockBidirection(false);
			pkGuActor->SetLookingDirection((GetMyTeam()==TEAM_RED ? DIR_DOWN : DIR_UP), false);
			pkGuActor->SetTargetScale(GetMonsterScale(kInfo.iMonsterNo),300);
		}
	}
}

void PgMissionMgr::OnGuardianMouseOut()
{
	lwCloseToolTip();
	RemoveTempGuardian();
}

std::wstring PgMissionMgr::GuardianPosStr(BM::GUID const kGuid)const
{
	CONT_GUARDIAN_INSTALL::const_iterator c_it = m_kGuardianInstall.begin();
	while(c_it != m_kGuardianInstall.end())
	{
		if( (*c_it).second.kGuid == kGuid )
		{
			return (*c_it).first;
		}
		++c_it;
	}
	return L"";
}

bool PgMissionMgr::IsGuardianInstall(std::wstring const& kTriggerName)const
{
	CONT_GUARDIAN_INSTALL::const_iterator c_it = m_kGuardianInstall.begin();
	while(c_it != m_kGuardianInstall.end())
	{
		if( (*c_it).first == kTriggerName )
		{
			return true;
		}
		++c_it;
	}
	return false;
}

bool PgMissionMgr::IsGuardianInstall(BM::GUID const kGuid)const
{
	CONT_GUARDIAN_INSTALL::const_iterator c_it = m_kGuardianInstall.begin();
	while(c_it != m_kGuardianInstall.end())
	{
		if( (*c_it).second.kGuid == kGuid )
		{
			return true;
		}
		++c_it;
	}
	return false;
}

bool PgMissionMgr::IsGuardianInstall()const
{
	PgActor * pkActor = g_kPilotMan.GetPlayerActor();
	if( !pkActor ){ return false; }

	std::wstring kGuardianTriggerName;
	float fMinDistance = 9999999.f;

	CONT_TRIGGERPOS::const_iterator c_iter = m_kContGuardianTriggerPos.begin();
	while(c_iter != m_kContGuardianTriggerPos.end())
	{
		float const fDistance = (c_iter->second - pkActor->GetPos()).Length();

		if(fMinDistance > fDistance)
		{
			fMinDistance = fDistance;
			kGuardianTriggerName = c_iter->first;
		}
		++c_iter;
	}

	return m_kGuardianInstall.find(kGuardianTriggerName) != m_kGuardianInstall.end();
}

void PgMissionMgr::StrategyItemDoAction(int const iIndex)
{
	if(iIndex >= m_kContDefenceMyItem.size())
	{
		return;
	}

	int const iItemNo = m_kContDefenceMyItem.at(iIndex);
	if(iItemNo)
	{
		int iOption = 0;
		BM::Stream kPacket(PT_C_M_REQ_DEFENCE_ITEM_USE);
		kPacket.Push(iItemNo);
		kPacket.Push(SItemPos());
		kPacket.Push(iOption);
		NETWORK_SEND(kPacket);
	}
}

int PgMissionMgr::GetStageClearPoint()const
{
	return m_iStageClearPoint;
	/*const CONT_MISSION_DEFENCE7_STAGE_BAG *pkStage;
	g_kTblDataMgr.GetContDef(pkStage);
	if( !pkStage )
	{
		return 0;
	}

	int const iModeType = GetDefenceModeType( lwGetGameLevel() );
	int const iPartyCount = std::max<int>( 1, g_kParty.MemberCount() );
	CONT_MISSION_DEFENCE7_STAGE_BAG::key_type kKey(lwGetMissionNo(),iModeType,iPartyCount,NowStage()+1);
	CONT_MISSION_DEFENCE7_STAGE_BAG::const_iterator c_it = pkStage->find(kKey);
	if( c_it != pkStage->end() )
	{ 
		CONT_MISSION_DEFENCE7_STAGE_BAG::mapped_type kElement = (*c_it).second;
		CONT_MISSION_DEFENCE7_STAGE::value_type &kValue = kElement.kCont.at(0);
		return kValue.iClear_StategicPoint;
	}

	return 0;*/
}

void PgMissionMgr::MissionStageSubInfo()const
{
	if( IsDefenceMode7() || IsDefenceMode8() )
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"FRM_MISSION_STAGE_SUBINFO");
		if(!pkWnd)
		{
			return;
		}

		BM::vstring vStr(TTW(401120));
		vStr.Replace(L"#POINT#", GetStageClearPoint());
		
		XUI::CXUI_Wnd* pkText = pkWnd->GetControl(L"FRM_TEXT");
		pkText ? pkText->Text(vStr) : 0;
	}
}

void PgMissionMgr::StratagyMissionStageResult()const
{
	CUnit *pkUnit = g_kPilotMan.GetPlayerUnit();
	if( !pkUnit )
	{ 
		return;
	}

	PgMission_ResultC const & kMission_Result = g_kMissionComplete.GetMission_ResultC();
	SMissionPlayerInfo_Client OhterInfo;
 
	for (int i = 0; i < PARTY_MEMBER_COUNT; ++i)
	{
		OhterInfo.Clear();
		if( true == kMission_Result.GetInfoFromUserInfoAt(i, OhterInfo) )
		{
			if(pkUnit->GetID() == OhterInfo.kCharGuid)
			{
				RESULT_SITEM_BAG_LIST_CONT::const_iterator c_it = OhterInfo.kContItem.begin();
				while(c_it != OhterInfo.kContItem.end())
				{
					//XUIMgr.Call();
					int const iItemNo = (*c_it).ItemNo();

					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const* pkItemDef = kItemDefMgr.GetDef(iItemNo);
					if( pkItemDef )
					{
						std::wstring kTempStr;
						wchar_t const* pkItemName = NULL;
						if( GetDefString(pkItemDef->NameNo(), pkItemName) )
						{
							if( FormatTTW(kTempStr, 460032, pkItemName) )
							{
								g_kChatMgrClient.AddLogMessage(SChatLog(CT_ITEM), kTempStr, true);
							}
						}
					}					
					break;
				}
				break;
			}
		}
	}
}

void PgMissionMgr::StratagyMissionStageEnd(int const iSec)const
{
	XUIMgr.Close(L"FRM_CONSTELLATION_RESULT");
	lua_tinker::call<void,int>("SetDefence7_DeadCount", iSec);
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"FRM_DEFENCE_NEXT_DELAY");
	if( pkWnd )
	{	// 포인트 복제기의 누적 점수를 요청.
		if( PgClientPartyUtil::IsInParty() )
		{	// 파티에 속해 있다면.
			PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
			if( pPlayer )
			{
				if( PgClientPartyUtil::IsPartyMaster(pPlayer->GetID()) )
				{	// 파티장일 경우에만 요청할 수 있음.
					BM::Stream Packet(PT_C_M_REQ_POINT_COPY);
					NETWORK_SEND(Packet);
				}
			}
		}
		else
		{	// 파티에 속해 있지 않다면 바로 요청.
			BM::Stream Packet(PT_C_M_REQ_POINT_COPY);
			NETWORK_SEND(Packet);
		}
		return;
	}
}

void PgMissionMgr::SetUI(XUI::CXUI_Wnd* pkWnd, int const iGuardianNo, int const iMonsterNo, BM::vstring & vPoint, bool const bGrayScale)
{
	if( !pkWnd )
	{
		return;
	}

	BM::vstring vStr;
	if(iGuardianNo)
	{
		vStr = vPoint;
	}

	GuardianInfo kInfo;
	kInfo.iGuardianNo = iGuardianNo;
	kInfo.iMonsterNo = iMonsterNo;

	XUI::CXUI_Wnd* kTmp = NULL;
	kTmp = pkWnd->GetControl(L"SFRM_TEXT");
	kTmp ? kTmp->Text(vStr) : 0;

	kTmp = pkWnd->GetControl(L"ICN_GUARDIAN");
	if( kTmp )
	{
		kTmp->SetCustomData(&kInfo, sizeof(kInfo));
		kTmp->GrayScale(bGrayScale);
	}
	
	kTmp = pkWnd->GetControl(L"FRM_HOTKEY");
	kTmp ? kTmp->Visible(iMonsterNo) : 0;
}

void PgMissionMgr::SetGuardianTransforUI(XUI::CXUI_Wnd* pkWnd, SMISSION_DEFENCE7_GUARDIAN const & kDef)
{
	if( !pkWnd )
	{
		return;
	}
	
	XUI::CXUI_Builder* pkBuildItem = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_GUARDIANS"));
	if( !pkBuildItem )
	{
		return;
	}
	int const iCount = pkBuildItem->CountX() * pkBuildItem->CountY();
	int const iMyTeamPoint = GetMyTeamPoint();
	for(int i=0; i<iCount; ++i)
	{
		int const iGuardianNo = i<MAX_DEFENCE7_GUARDIAN_UPGRADE ? kDef.iUpgrade[i] : 0;
		SMISSION_DEFENCE7_GUARDIAN kUpgradeGuardian = GetDefGuardian(iGuardianNo);
		bool const bGrayScale = kUpgradeGuardian.iNeed_StrategicPoint > iMyTeamPoint;
		BM::vstring vStr(L"FRM_GUARDIAN");
		vStr += i;

		XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
		BM::vstring vPoint;
		vPoint = kUpgradeGuardian.iNeed_StrategicPoint;
		SetUI(pkFrm, kDef.iUpgrade[i], kUpgradeGuardian.iMonsterNo, vPoint, bGrayScale);
	}
}

void SetSkillUI(XUI::CXUI_Wnd* pkWnd, int const iGuardianNo, int const iSkillNo, int const iNeedPoint)
{
	if( !pkWnd ) return;
	XUI::CXUI_Wnd* pkIcon = pkWnd->GetControl(L"ICN_SKILL");
	if( !pkIcon ) return;

	CSkillDef const * pSkillDef = NULL;
	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	pSkillDef = kSkillDefMgr.GetDef( iSkillNo );

	if( iSkillNo && pSkillDef )
	{
		SGuardian_Skill kSkillInfo(iGuardianNo, iSkillNo);
		kSkillInfo.kGuardianGuid = g_kMissionMgr.GetSelectGuardianGuid();
		pkIcon->SetCustomData(&kSkillInfo, sizeof(kSkillInfo));
		pkIcon->GrayScale( iNeedPoint>GetMyTeamPoint() );

		// 이름
		wchar_t const* pName = NULL;
		if( GetDefString(pSkillDef->NameNo(),pName) )
		{
			BM::vstring vStr(pName);
			int const iLevel = pSkillDef->GetAbil(AT_LEVEL);
			if( iLevel )
			{
				vStr += L" ";
				vStr += TTW(224);
				vStr += pSkillDef->GetAbil(AT_LEVEL);
			}
			pkWnd->Text(vStr);
		}	

		//	설명
		const TCHAR *pText = NULL;
		int Value = pSkillDef->GetAbil(ATI_EXPLAINID);
		if(Value && GetDefString(Value, pText))
		{
			XUI::CXUI_Wnd* kTmp = NULL;
			kTmp = pkWnd->GetControl(L"FRM_TEXT");
			kTmp ? kTmp->Text(pText) : 0;
		}
	}
	else
	{
		pkIcon->ClearCustomData();

		pkWnd->Text(L"");
		XUI::CXUI_Wnd* kTmp = NULL;
		kTmp = pkWnd->GetControl(L"FRM_TEXT");
		kTmp ? kTmp->Text(L"") : 0;
	}


	XUI::CXUI_Wnd* kTmp = NULL;
	kTmp = pkWnd->GetControl(L"IMG_POINT");
	kTmp ? kTmp->Text(BM::vstring(iNeedPoint)) : 0;

	kTmp = pkWnd->GetControl(L"FRM_HOTKEY");
	kTmp ? kTmp->Visible(iSkillNo) : 0;
}

BM::GUID PgMissionMgr::GetSelectGuardianGuid()const
{
	CONT_GUARDIAN_INSTALL::const_iterator c_it = m_kGuardianInstall.find(m_kGuardianUpgradeLoc);
	if( c_it != m_kGuardianInstall.end() )
	{
		return (*c_it).second.kGuid;
	}
	return BM::GUID::NullData();
}

void PgMissionMgr::SetGuardianUpgradeUI(XUI::CXUI_Wnd* pkWnd, int const iGuardianNo, SMISSION_DEFENCE7_GUARDIAN const & kDef)
{
	if( !pkWnd )
	{
		return;
	}
	
	pkWnd->SetCustomData(&iGuardianNo, sizeof(iGuardianNo));
	/*if( 0==kDef.iUpgrade[0] )
	{
		pkWnd->Close();
		lwAddWarnDataTT(401125);
		return;
	}*/

	int const iMyTeamPoint = GetMyTeamPoint();
	
	XUI::CXUI_Wnd* pkFirst = pkWnd->GetControl(L"FRM_GUARDIAN0");
	if( pkFirst )
	{
		XUI::CXUI_Wnd* pkName = pkWnd->GetControl(L"SFRM_GUARDIAN_NAME");
		pkName ? pkName->Text( GetMonsterName(kDef.iMonsterNo) ) : 0;

		SetUI(pkFirst, iGuardianNo, kDef.iMonsterNo, BM::vstring(kDef.iNeed_StrategicPoint), false);

		BM::vstring vPoint(TTW(401159));
		vPoint.Replace(L"#POINT#",kDef.Sell_StrategicPoint);
		pkWnd->Text(vPoint);
	}
	
	bool bBeforGrayScale = false;
	for(int i=0; i<MAX_DEFENCE7_GUARDIAN_UPGRADE; ++i)
	{
		int const iGuardianNo = kDef.iUpgrade[i];

		SMISSION_DEFENCE7_GUARDIAN kUpgradeGuardian = GetDefGuardian(iGuardianNo);
		BM::vstring vPoint(kUpgradeGuardian.iNeed_StrategicPoint);
		bool const bGrayScale = (0==i) ? (kUpgradeGuardian.iNeed_StrategicPoint > iMyTeamPoint) : bBeforGrayScale;
		bBeforGrayScale = bGrayScale; //회색이면 계속 회색

		BM::vstring vStr(L"FRM_GUARDIAN");
		vStr += i+1;

		XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
		if( !pkFrm )
		{
			continue;
		}

		SetUI(pkFrm, iGuardianNo, kUpgradeGuardian.iMonsterNo, vPoint, bGrayScale);		

		XUI::CXUI_Wnd* pkArrow = pkFrm->GetControl(L"FRM_ARROW");
		if(0==i && pkArrow)
		{
			pkArrow->GrayScale(bGrayScale);
			if( bGrayScale )
			{
				SUVInfo kUVInfo = pkArrow->UVInfo();
				kUVInfo.Index = 2;
				pkArrow->UVInfo(kUVInfo);
				pkArrow->SetInvalidate();
			}
		}
		pkArrow ? pkArrow->Visible(kUpgradeGuardian.iMonsterNo) : 0;
	}

	SetSkillUI(pkWnd->GetControl(L"FRM_SKILL0"), iGuardianNo, kDef.Skill01_No, kDef.Skill01_StrategicPoint);
	SetSkillUI(pkWnd->GetControl(L"FRM_SKILL1"), iGuardianNo, kDef.Skill02_No, kDef.Skill02_StrategicPoint);

	SetPointUI(pkWnd);
}

void PgMissionMgr::CallInstallGuardian(XUI::CXUI_Wnd* pkWnd)
{
	if( !pkWnd )
	{
		return;
	}

	GuardianInfo kInfo;
	pkWnd->GetCustomData(&kInfo, sizeof(kInfo));
	if( 0==kInfo.iGuardianNo || IsGuardianInstall() )
	{
		return;
	}

	SMISSION_DEFENCE7_GUARDIAN kInstallGuardian = GetDefGuardian(kInfo.iGuardianNo);
	if( kInstallGuardian.iNeed_StrategicPoint <= GetMyTeamPoint())
	{
		std::wstring const kName = GetMonsterName(kInstallGuardian.iMonsterNo);

		BM::vstring vStr(TTW(401121));
		vStr.Replace(L"#NAME#", kName);
		vStr.Replace(L"#POINT#", kInstallGuardian.iNeed_StrategicPoint);
		lwCallYesNoMsgBox(static_cast<std::wstring>(vStr), BM::GUID::NullData(), MBT_DEFENCE_GUARDIAN_INSTALL, kInfo.iGuardianNo);
	}
	else
	{
		lwAddWarnDataTT(401139);
	}
}

void PgMissionMgr::CallUpgradeGuardian(lwUIWnd & kWnd)
{
	if( kWnd.IsNil() )
	{
		return;	
	}
	
	XUI::CXUI_Wnd* pkParent = kWnd();
	pkParent = pkParent ? pkParent->Parent() : NULL;
	pkParent = pkParent ? pkParent->Parent() : NULL;
	if( !pkParent )
	{
		return;
	}

	if( !m_iGuardianUpgradeNo || m_kGuardianUpgradeLoc.empty() )
	{
		pkParent->Close();
		return;
	}

	GuardianInfo kInfo;
	kWnd()->GetCustomData(&kInfo, sizeof(kInfo));
	if( !kInfo.iGuardianNo )
	{
		return;
	}

	if(kWnd.IsGrayScale())
	{
		return;
	}

	SMISSION_DEFENCE7_GUARDIAN kInstallGuardian = GetDefGuardian(m_iGuardianUpgradeNo);
	SMISSION_DEFENCE7_GUARDIAN kUpgradeGuardian = GetDefGuardian(kInfo.iGuardianNo);

	int iTTNo = 0;
	if(ED7GT_TRANSFOR == kInstallGuardian.iUpgrade_Type)
	{
		iTTNo = 401127;
	}
	else if(ED7GT_UPGRADE == kInstallGuardian.iUpgrade_Type)
	{
		iTTNo = 401126;

		XUI::CXUI_Wnd* pkParent = kWnd.GetParent()();
		if(!pkParent || pkParent->BuildIndex() != 1)
		{
			return;
		}
	}

	pkParent->Close();

	std::wstring const kBeforName = GetMonsterName(kInstallGuardian.iMonsterNo);
	std::wstring const kAfterName = GetMonsterName(kInfo.iMonsterNo);

	BM::vstring vStr(TTW(iTTNo));
	vStr.Replace(L"#BEFOR#", kBeforName);
	vStr.Replace(L"#AFTER#", kAfterName);
	vStr.Replace(L"#POINT#", kUpgradeGuardian.iNeed_StrategicPoint);
	lwCallYesNoMsgBox(static_cast<std::wstring>(vStr), BM::GUID::NullData(), MBT_DEFENCE_GUARDIAN_UPGRADE, kInfo.iGuardianNo);
}

void PgMissionMgr::CallRemoveGuardian()
{
	CONT_GUARDIAN_INSTALL::const_iterator c_it = m_kGuardianInstall.find(m_kGuardianUpgradeLoc);
	if( c_it == m_kGuardianInstall.end() )
	{
		return;
	}

	int const iGuardianNo = (*c_it).second.iGuardianNo;
	SMISSION_DEFENCE7_GUARDIAN kRemoveGuardian = GetDefGuardian(iGuardianNo);

	BM::vstring vStr(TTW(401160));
	vStr.Replace(L"#POINT#", kRemoveGuardian.iNeed_StrategicPoint);
	lwCallYesNoMsgBox(static_cast<std::wstring>(vStr), BM::GUID::NullData(), MBT_DEFENCE_GUARDIAN_REMOVE, iGuardianNo);
}

void PgMissionMgr::CallDefenceSkillLearn(XUI::CXUI_Wnd* pkWnd)
{
	if( !pkWnd )
	{
		return;
	}

	SGuardian_Skill kSkillInfo;
	pkWnd->GetCustomData(&kSkillInfo, sizeof(kSkillInfo));

	if( kSkillInfo.iSkillNo && GetMyTeamPoint()>=GetSkillNeedPoint(kSkillInfo.iGuardianNo, kSkillInfo.iSkillNo) )
	{
		BM::GUID kGuardianGuid;
		if( IsGuardianSkill(kSkillInfo.iSkillNo) )
		{
			CONT_GUARDIAN_INSTALL::const_iterator c_it = m_kGuardianInstall.find(m_kGuardianUpgradeLoc);
			if( c_it != m_kGuardianInstall.end() )
			{
				kGuardianGuid = (*c_it).second.kGuid;


				GET_DEF(CSkillDefMgr, kSkillDefMgr);
				CSkillDef const* pkSkill = kSkillDefMgr.GetDef(kSkillInfo.iSkillNo);
				if( !pkSkill )
				{
					return;		
				}
				int const iEffectNo = pkSkill->GetEffectNo();

				PgPilot * pkGuardianPilot = g_kPilotMan.FindPilot(kGuardianGuid);
				CUnit* pkGuardianUnit = pkGuardianPilot ? pkGuardianPilot->GetUnit() : NULL;
				CEffect* pkEffect = pkGuardianUnit ? pkGuardianUnit->GetEffect(iEffectNo) : NULL;
				if(pkEffect)
				{
					g_kChatMgrClient.ShowNoticeUI(TTW(235), 2, true, true);
					return;
				}
			}
		}
		BM::Stream kPacket(PT_C_M_REQ_DEFENCE_SKILL_USE);
		kPacket.Push(kSkillInfo.iGuardianNo);
		kPacket.Push(kSkillInfo.iSkillNo);
		kPacket.Push(kGuardianGuid);
		NETWORK_SEND(kPacket);
	}
}

bool GetDefence7Mission(SMISSION_DEFENCE7_MISSION & kMission, int const iMissionNo, int const iMissionLv)
{
	const CONT_MISSION_DEFENCE7_MISSION_BAG *pkMission;
	g_kTblDataMgr.GetContDef(pkMission);
	if( !pkMission )
	{
		return false;
	}

	int const iModeType = GetDefenceModeType(0==iMissionLv ? lwGetGameLevel() : iMissionLv);
	CONT_MISSION_DEFENCE7_MISSION_BAG::key_type		kKey(0==iMissionNo ? lwGetMissionNo() : iMissionNo, iModeType);

	CONT_MISSION_DEFENCE7_MISSION_BAG::const_iterator iter = pkMission->find(kKey);
	if( iter == pkMission->end() || iter->second.kCont.empty() )
	{
		return false;
	}

	kMission = iter->second.kCont.at(0);
	return true;
}

void UpdateCreateDefenceUI(CONT_DEFENCE_PARTY_LIST const & rkWaitCont, CONT_DEFENCE_PARTY_LIST const & rkPlayCont)
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"FRM_CREATE_DEFENCE");
	if( !pkWnd )
	{
		return;
	}

	CONT_DEFENCE_PARTY_LIST::const_iterator wait_it=rkWaitCont.begin();
	while( wait_it!=rkWaitCont.end() )
	{
		BM::vstring vStr(L"FRM_WAIT");
		vStr += (*wait_it).first;

		XUI::CXUI_Wnd* pkInfo = pkWnd->GetControl(vStr);
		pkInfo ? pkInfo->Text( BM::vstring((*wait_it).second) ) : 0;
		++wait_it;
	}

	CONT_DEFENCE_PARTY_LIST::const_iterator play_it=rkPlayCont.begin();
	while( play_it!=rkPlayCont.end() )
	{
		BM::vstring vStr(L"FRM_PLAY");
		vStr += (*play_it).first;

		XUI::CXUI_Wnd* pkInfo = pkWnd->GetControl(vStr);
		pkInfo ? pkInfo->Text( BM::vstring((*play_it).second) ) : 0;
		++play_it;
	}
}

void SetPointUI(XUI::CXUI_Wnd* pkWnd)
{
	if(pkWnd)
	{
		XUI::CXUI_Wnd* kPoint = pkWnd->GetControl(L"FRM_POINT");
		kPoint = pkWnd->GetControl(L"FRM_POINT");
		kPoint ? kPoint->Text(BM::vstring(GetMyTeamPoint())) : 0;
	}
}

void PgMissionMgr::OnCallDefenceLearnSkill(XUI::CXUI_Wnd* pkWnd)
{
	if( !pkWnd )
	{
		return;
	}

	SMISSION_DEFENCE7_MISSION kValue;
	if( false == GetDefence7Mission(kValue) )
	{
		return;
	}

	XUI::CXUI_Builder* pkBuildItem = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SKILLS"));
	if( !pkBuildItem )
	{
		return;
	}

	int const iMyTeamPoint = GetMyTeamPoint();
	int const iCount = pkBuildItem->CountX() * pkBuildItem->CountY();
	for(int i=0; i<iCount && i<MAX_DEFENCE7_MISSION_SKILL; ++i)
	{
		BM::vstring vStr(L"FRM_SKILL");
		vStr += i;

		XUI::CXUI_Wnd* pkFrm = pkWnd->GetControl(vStr);
		if(!pkFrm){ continue; }

		XUI::CXUI_Wnd* pkIcon = pkFrm->GetControl(L"ICN_SKILL");
		if(!pkIcon){ continue; }

		int const iNextSkillNo = m_kDefenceSkill.GetNextSkillNo(GetMyTeam(), kValue.iSkill[i]);

		BM::vstring vPoint;
		if( iNextSkillNo )
		{
			int const iNeedPoint = GetSkillNeedPoint(0,iNextSkillNo);

			if( false==m_kDefenceSkill.IsLearn(GetMyTeam(), iNextSkillNo) )
			{
				vPoint = iNeedPoint;
				pkIcon->GrayScale(iNeedPoint>iMyTeamPoint);
			}
			else
			{
				vPoint = TTW(400176);
				pkIcon->GrayScale(false);
			}

			SGuardian_Skill kSkillInfo;
			kSkillInfo.iSkillNo = iNextSkillNo;
			pkIcon->SetCustomData(&kSkillInfo, sizeof(kSkillInfo) );
		}
		else
		{
			pkIcon->ClearCustomData();
		}

		XUI::CXUI_Wnd* kTmp = NULL;
		kTmp = pkFrm->GetControl(L"SFRM_TEXT");
		kTmp ? kTmp->Text(vPoint) : 0;

		kTmp = pkFrm->GetControl(L"FRM_HOTKEY");
		kTmp ? kTmp->Visible(iNextSkillNo) : 0;
	}

	SetPointUI(pkWnd);
}

void PgMissionMgr::UpdateGuardianSkillUI()
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_UPGRADE_GUARDIAN");
	if( !pkWnd )
	{
		return;
	}

	int iGuardianNo = 0;
	pkWnd->GetCustomData(&iGuardianNo, sizeof(iGuardianNo));
	SetGuardianUpgradeUI(pkWnd, iGuardianNo, GetDefGuardian(iGuardianNo));
}

void PgMissionMgr::GurdianUpgradeDoAction(int const iGuardianNo)
{
	if( !iGuardianNo || m_kGuardianUpgradeLoc.empty() )
	{
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_GUARDIAN_UPGRADE);
	kPacket.Push( iGuardianNo );
	kPacket.Push( m_kGuardianUpgradeLoc );
	NETWORK_SEND(kPacket);
}

void PgMissionMgr::GurdianRemoveDoAction()
{
	if( m_kGuardianUpgradeLoc.empty() )
	{
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_GUARDIAN_REMOVE);
	kPacket.Push( m_kGuardianUpgradeLoc );
	NETWORK_SEND(kPacket);
	XUIMgr.Close(L"SFRM_UPGRADE_GUARDIAN");
}

void PgMissionMgr::CallGuardianUI(BM::GUID const kGuid)
{
	SGuardian_Install kGuardian;
	CONT_GUARDIAN_INSTALL::const_iterator c_it = m_kGuardianInstall.begin();
	while(c_it != m_kGuardianInstall.end())
	{
		if( (*c_it).second.kGuid == kGuid )
		{
			m_kGuardianUpgradeLoc = (*c_it).first;
			m_iGuardianUpgradeNo = (*c_it).second.iGuardianNo;

			kGuardian = (*c_it).second;
			break;
		}
		++c_it;
	}

	XUI::CXUI_Wnd* pkWnd = NULL;
	SMISSION_DEFENCE7_GUARDIAN kDefGuardian = GetDefGuardian(kGuardian.iGuardianNo);
	if(ED7GT_TRANSFOR == kDefGuardian.iUpgrade_Type)
	{
		XUIMgr.Close(L"SFRM_UPGRADE_GUARDIAN");
		if( false==XUIMgr.Close(L"SFRM_TRANSFOR_GUARDIAN") )
		{
			pkWnd = XUIMgr.Call(L"SFRM_TRANSFOR_GUARDIAN");
			SetGuardianTransforUI(pkWnd, kDefGuardian);
		}
	}
	else if(ED7GT_UPGRADE == kDefGuardian.iUpgrade_Type)
	{
		XUIMgr.Close(L"SFRM_TRANSFOR_GUARDIAN");
		if( false==XUIMgr.Close(L"SFRM_UPGRADE_GUARDIAN") )
		{
			pkWnd = XUIMgr.Call(L"SFRM_UPGRADE_GUARDIAN");
			SetGuardianUpgradeUI(pkWnd, kGuardian.iGuardianNo, kDefGuardian);
		}
	}

	SetPointUI(pkWnd);
}

static void GetMonsterAbilName(int iMonsterNo, WORD const AbilType, std::wstring & kOutText)
{
	if( !iMonsterNo )
	{
		return;
	}

	const TCHAR *pText = NULL;
	if( GetAbilName(AbilType, pText) )
	{
		BM::vstring vAbil;
		std::wstring kEtc;
		switch(AbilType)
		{
		case AT_R_ATTACK_SPEED:
			{
				wchar_t	szTemp[MAX_PATH] = {0,};
				swprintf(szTemp, MAX_PATH, L"%0.1f", GetCalcDefMonsterAbil(iMonsterNo,AT_CUSTOM_TOOLTIP_01)/1000.f);
				vAbil = szTemp;
				kEtc = TTW(9);
			}break;
		default:
			{
				vAbil = GetCalcDefMonsterAbil(iMonsterNo,AbilType);
			}break;
		}
		kOutText += pText;
		kOutText += _T(": ") + (std::wstring)(vAbil);
		kOutText += kEtc;
		kOutText += _T("\n");
	}
}

//static void GetMonsterAbilName(CMonsterDef const *pDef, WORD const AbilType, std::wstring & kOutText)
//{
//	if( !pDef )
//	{
//		return;
//	}
//
//	const TCHAR *pText = NULL;
//	if( GetAbilName(AbilType, pText) )
//	{
//		BM::vstring vAbil;
//		std::wstring kEtc;
//		switch(AbilType)
//		{
//		case AT_R_PHY_ATTACK_MAX:
//			{
//				float const fValue = 1.f + pDef->GetAbil(AT_R_PHY_ATTACK_MAX);
//				vAbil = static_cast<int>( pDef->GetAbil(AT_PHY_ATTACK_MIN) * (fValue/ABILITY_RATE_VALUE64) );
//			}break;
//		case AT_R_MAGIC_ATTACK_MAX:
//			{
//				float const fValue = 1.f + pDef->GetAbil(AT_R_MAGIC_ATTACK_MAX);
//				vAbil = static_cast<int>( pDef->GetAbil(AT_MAGIC_ATTACK_MIN) * (fValue/ABILITY_RATE_VALUE64) );
//			}break;
//		case AT_R_ATTACK_SPEED:
//			{
//				wchar_t	szTemp[MAX_PATH] = {0,};
//				swprintf(szTemp, MAX_PATH, L"%0.1f", pDef->GetAbil(AT_CUSTOM_TOOLTIP_01)/1000.f);
//				vAbil = szTemp;
//				kEtc = TTW(9);
//			}break;
//		case AT_R_ATTACK_RANGE:
//			{
//				vAbil = pDef->GetAbil(AT_DETECT_RANGE);
//			}break;
//		default:
//			{
//				vAbil = pDef->GetAbil(AbilType);
//			}break;
//		}
//		kOutText += pText;
//		kOutText += _T(": ") + (std::wstring)(vAbil);
//		kOutText += kEtc;
//		kOutText += _T("\n");
//	}
//}

bool PgMissionMgr::MakeGuardianToolTipText(int iIconKey, std::wstring &wstrTextOut)
{
	if (!iIconKey)
	{
		return false;
	}

	int const iGuardianNo = iIconKey;
	SMISSION_DEFENCE7_GUARDIAN const& kDefGuardian = GetDefGuardian(iGuardianNo);
	int const iMonsterNo = kDefGuardian.iMonsterNo;
#ifndef EXTERNAL_RELEASE
	wstrTextOut += _T("NUMBER : ") + (std::wstring)(BM::vstring(iMonsterNo));
	wstrTextOut += _T("\n");
	wstrTextOut += _T("\n");
#endif

	BM::vstring vStr;

	// Name
	wstrTextOut += TTW(40002);
	wstrTextOut += GetMonsterName(iMonsterNo);

	//전략포인트
	int const iNeedPoint = GetGuardianNeedPoint(iGuardianNo);
	wstrTextOut += _T("\n");
	wstrTextOut += _T("\n");
	wstrTextOut += TTW(GetMyTeamPoint()>=iNeedPoint ? 401143 : 401144);
	wstrTextOut += TTW(401140);
	wstrTextOut += static_cast<std::wstring>( BM::vstring(iNeedPoint) );

	wstrTextOut += _T("\n");
	wstrTextOut += _T("\n");
	wstrTextOut += TTW(401141);
	wstrTextOut += TTW(40011);	//폰트 초기화

	GetMonsterAbilName(iMonsterNo, AT_R_PHY_ATTACK_MAX, wstrTextOut);	// 물리공격력	
	GetMonsterAbilName(iMonsterNo, AT_R_MAGIC_ATTACK_MAX, wstrTextOut);	// 마법공격력
	GetMonsterAbilName(iMonsterNo, AT_R_ATTACK_SPEED, wstrTextOut);		// 공격속도
	GetMonsterAbilName(iMonsterNo, AT_R_ATTACK_RANGE, wstrTextOut);		// 사정거리
	
	// 최대타격
	wstrTextOut += TTW(401147);
	wstrTextOut += _T(": ") + (std::wstring)(BM::vstring(GetCalcDefMonsterAbil(iMonsterNo,AT_MAX_TARGETNUM)));
	wstrTextOut += TTW(401148);
	wstrTextOut += _T("\n");
	
	int const iDescr = GetCalcDefMonsterAbil(iMonsterNo,ATI_EXPLAINID);
	if(0<iDescr)
	{
		const wchar_t *pDescr = NULL;
		if(GetDefString(iDescr, pDescr) && pDescr)
		{
			wstrTextOut += _T("\n");
			wstrTextOut += TTW(401142);
			wstrTextOut += TTW(40051);
			wstrTextOut += pDescr;
		}
	}
	return true;
}

bool PgMissionMgr::MakeStratagySkillToolTipText(int const iGuardianNo, int iSkillNo, std::wstring &wstrTextOut, std::wstring& wstrType, bool bNextLevel)
{
	WCHAR	temp[500];
	wstrTextOut.clear();
	CSkillDef const * pSkillDef = NULL;

	GET_DEF(CSkillDefMgr, kSkillDefMgr);
	pSkillDef = kSkillDefMgr.GetDef( bNextLevel ? ++iSkillNo : iSkillNo );
	
	if(!pSkillDef)
	{
		return false;
	}

	bool const bCouple = pSkillDef->GetAbil(AT_IS_COUPLE_SKILL);
	
	//	현재 레벨 스킬의 정보를 보여준다.
	wchar_t const* pName = NULL;
	GetDefString(pSkillDef->NameNo(),pName);

	//	이름용 색상 
	if(!bNextLevel)
	{
		wstrTextOut += TTW(40102); //이름용
	}
	else
	{
		wstrTextOut += TTW(234); //이름용
	}

	//	이름
	if(pName)
	{	
		int const iLevel = pSkillDef->GetAbil(AT_LEVEL);
		wchar_t const * pText = iLevel ? _T("%s {C=0xFF000000}%s %d") : _T("%s");
		swprintf(temp,500,pText,pName,TTW(224).c_str(),iLevel);
		wstrTextOut+=temp;
	}
	else
	{
		wstrTextOut+=_T("Undefined Name");
	}

	//	분류
	if(!bNextLevel)
	{
		wstrType+=TTW(40140+pSkillDef->m_byType);
	}

	bool const bGuildSkill = SDT_Guild == pSkillDef->GetAbil(AT_SKILL_DIVIDE_TYPE);
	if( bGuildSkill )
	{
		wstrTextOut += _T("\n ") + TTW(400823);//[길드 스킬]
	}

#ifndef USE_INB
	if( g_pkApp->VisibleClassNo() )
	{
		BM::vstring kSkillNo(pSkillDef->No());
		wstrTextOut += _T("\n SkillNo : ") + (std::wstring)kSkillNo;
	}
#endif

	//전략포인트
	int const iNeedPoint = GetSkillNeedPoint(iGuardianNo, iSkillNo);
	wstrTextOut += _T("\n");
	wstrTextOut += _T("\n");
	wstrTextOut += TTW(GetMyTeamPoint()>=iNeedPoint ? 401143 : 401144);
	wstrTextOut += TTW(401140);
	wstrTextOut += static_cast<std::wstring>( BM::vstring(iNeedPoint) );

	//	공백
	wstrTextOut+=_T("\n\n");

	//	일반 색상
	if(!bNextLevel)
		wstrTextOut += TTW(40011);	//폰트 초기화
	else
		wstrTextOut += TTW(226);	//폰트 초기화

	if(!bNextLevel)
	{
		//	선행스킬
		int kArray[LEAD_SKILL_COUNT] = {0,};
		if (g_kSkillTree.GetNeedSkill(iSkillNo, kArray))
		{
			wstrTextOut += TTW(40130);
			wstrTextOut += _T(" : ");

			for (int i = 0; i < LEAD_SKILL_COUNT; ++i)
			{
				int const iNo = kArray[i];
				if (0!=iNo)
				{
					int const iKey = g_kSkillTree.GetKeySkillNo(iNo);
					PgSkillTree::stTreeNode* pNode = g_kSkillTree.GetNode(iKey);
					if (pNode)
					{
						const CSkillDef* pkLeadDef = pNode->GetSkillDef();
						if (pkLeadDef)
						{
							if (i>0)
							{
								wstrTextOut+=_T(", ");
							}
							if((pNode->IsLearned() || pNode->m_bTemporaryLearned) && pNode->m_ulSkillNo >= iNo)
							{		
								wstrTextOut+=TTW(40106);
							}
							else
							{
								wstrTextOut+=TTW(40105);
							}

							std::wstring const *pLeadName = NULL;

							if(GetDefString(pkLeadDef->NameNo(),pLeadName))
							{
								wstrTextOut += std::wstring(*pLeadName);
								wstrTextOut += L" ";
								wstrTextOut += TTW(224);
								wstrTextOut += std::wstring(BM::vstring(iNo-iKey+1));
							}
						}
					}
				}
			}

			wstrTextOut+=_T('\n');
		}

		//	색상 초기화
		if(!bNextLevel)
			wstrTextOut += TTW(40011);	//폰트 초기화
		else
			wstrTextOut += TTW(226);	//폰트 초기화
	}

	const CONT_DEFSKILL *pkContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pkContDefMap);
	if( !bNextLevel )
	{
		//	스킬 커맨드
		if (pkContDefMap)
		{
			CONT_DEFSKILL::const_iterator def_it = pkContDefMap->find(iSkillNo);
			if (pkContDefMap->end() != def_it)
			{
				CONT_DEFSKILL::mapped_type kT = (*def_it).second;
				if( 0 != kT.iCmdStringNo )
				{
					wstrTextOut+=TTW(40107);
					wstrTextOut+=_T(" : ");

					const wchar_t *pText = NULL;
					if(GetDefString(kT.iCmdStringNo, pText))
					{
						wstrTextOut+=pText;
					}
					wstrTextOut+=_T("\n");
				}
			}
		}
	}

	//	필요 SP
	if(!bCouple)
	{
		int const iNeed = pSkillDef->GetAbil(AT_NEED_SP);

		if(iNeed > 0)
		{
			wstrTextOut+=TTW(40131);
			wstrTextOut+=_T(" : ");
			if (g_kSkillTree.GetRemainSkillPoint() < iNeed)
			{
				wstrTextOut+=TTW(40105);
			}
			swprintf(temp,500,_T("%d\n"),iNeed);			
			wstrTextOut+=temp;
		}

		//	색상 초기화
		if(!bNextLevel)
			wstrTextOut += TTW(40011);	//폰트 초기화
		else
			wstrTextOut += TTW(226);	//폰트 초기화
	}

	//	종류(보류)
//	swprintf(temp,100,_T("%s : \n"),TTW(228).c_str());
//	wstrTextOut+=temp;

	//	소모 MP
	if (EST_PASSIVE != pSkillDef->m_byType)	//패시브 스킬아닌 경우
	{
		if(0<pSkillDef->m_sMP)
		{
			swprintf(temp,500,_T("%s : %d\n"),TTW(40133).c_str(), pSkillDef->m_sMP);
			wstrTextOut+=temp;
		}
		else
		{
			//소모 MP가 MAXMP의 %를 소모 할 경우
			int const iNeedRateMaxMP = pSkillDef->GetAbil(AT_NEED_MAX_R_MP);
			if(0 < iNeedRateMaxMP)
			{
				int const iRateMP = iNeedRateMaxMP / 100;
				swprintf(temp,500,_T("%s : %s %d%%\n"),TTW(40133).c_str(), TTW(512).c_str(), iRateMP);
				wstrTextOut+=temp;
			}
		}

		if(0<pSkillDef->GetAbil(AT_NEED_AWAKE) || 0<pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE))
		{
			wstrTextOut += TTW(1306);//노랑
			//if(3000<=pSkillDef->GetAbil(AT_NEED_AWAKE) 
			//	|| 6000<=pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE))
			//{// 대량 소모한다 표시
			//	wstrTextOut += TTW(5060);
			//}
			//else
			//{// 그냥 소모 한다 표시
			//	wstrTextOut += TTW(5044);
			//}
			{
				wstrTextOut += TTW(5061);
				wstrTextOut += L" : ";
				BM::vstring vStr;
				if(0 < pSkillDef->GetAbil(AT_NEED_AWAKE))
				{
					vStr += pSkillDef->GetAbil(AT_NEED_AWAKE);
				}
				else if(0 < pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE))
				{
					vStr += pSkillDef->GetAbil(AT_ADD_NEED_R_AWAKE)/ABILITY_RATE_VALUE;
					vStr += L"%";
				}
				wstrTextOut += static_cast<std::wstring>(vStr);
			}
			wstrTextOut += TTW(40011);
			wstrTextOut += L"\n";
		}
		else
		{
			if(0<pSkillDef->GetAbil(AT_ALL_NEED_AWAKE))
			{
				wstrTextOut += TTW(1306);//노랑
				wstrTextOut += TTW(5045);
				wstrTextOut += TTW(40011);
				wstrTextOut += L"\n";
			}
		}
	}

	// 소모 HP
	if (0 < pSkillDef->m_sHP)
	{
		swprintf(temp,500,_T("%s : %d\n"),TTW(40134).c_str(), pSkillDef->m_sHP);
		wstrTextOut+=temp;		
	}
	else
	{
		//소모 MP가 MAXHP의 %를 소모 할 경우
		int const iNeedRateMaxHP = pSkillDef->GetAbil(AT_NEED_MAX_R_HP);
		if(0 < iNeedRateMaxHP)
		{
			int const iRateHP = iNeedRateMaxHP / 100;
			swprintf(temp,500,_T("%s : %s %d%%\n"),TTW(40134).c_str(), TTW(511).c_str(), iRateHP);
			wstrTextOut+=temp;
		}
	}

	CSkillDef const* pBackupSkillDef = NULL;
	//실제 공격 스킬은 따로 존재 하는 경우
	int iRealSkill = pSkillDef->GetAbil(AT_CHILD_CAST_SKILL_NO);
	if(0 < iRealSkill)
	{
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		CSkillDef const* pChildSkillDef = kSkillDefMgr.GetDef(iRealSkill);
		if(pChildSkillDef)
		{
			pBackupSkillDef = pSkillDef;
			pSkillDef = pChildSkillDef;
		}
	}

	//데미지를 나눠서 공격하는 스킬은 한발 데미지로 나누어 주어야 한다.
	int iComboCount = pSkillDef->GetAbil(AT_COMBO_HIT_COUNT);

	//	물리대미지
	int iAtk = pSkillDef->GetAbil(AT_I_PHY_ATTACK_ADD_RATE);
	if(0 < iComboCount)
	{
		iAtk /= iComboCount;
	}

	if( 0 < iAtk )
	{
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(40136).c_str(), ((float)(iAtk) * 0.01f));
		wstrTextOut+=temp;
		iAtk = pSkillDef->GetAbil(AT_PHY_ATTACK);
		if(0 < iComboCount)
		{
			iAtk /= iComboCount;
		}

		if( 0 < iAtk )
		{
			swprintf(temp, 499, _T(" + %d"), iAtk);
			wstrTextOut+=temp;
		}
		wstrTextOut+=L"\n";
	}

	//	마법대미지
	int iMAtk = pSkillDef->GetAbil(AT_I_MAGIC_ATTACK_ADD_RATE);
	if(0 < iComboCount)
	{
		iMAtk /= iComboCount;
	}

	if( 0 < iMAtk )
	{
		swprintf(temp, 499, _T("%s : %.1f%%"), TTW(40137).c_str(), ((float)(iMAtk) * 0.01f));
		wstrTextOut+=temp;
		iMAtk = pSkillDef->GetAbil(AT_MAGIC_ATTACK);
		if(0 < iComboCount)
		{
			iMAtk /= iComboCount;
		}
		if( 0 < iMAtk )
		{
			swprintf(temp, 499, _T(" + %d"), iMAtk);
			wstrTextOut+=temp;
		}
		wstrTextOut+=L"\n";
	}

	//데미지만 실제 공격 스킬에서 얻어 오고 원래 스킬로 돌려 준다.
	if(NULL != pBackupSkillDef)
	{
		pSkillDef = pBackupSkillDef;
	}

	wstrTextOut += TTW(40011);	//폰트 초기화

	//	색상 초기화
	if(!bNextLevel)
	{
		wstrTextOut += TTW(40011);	//폰트 초기화
	}
	else
	{
		wstrTextOut += TTW(226);	//폰트 초기화
	}

	//	제한직업
	/*std::wstring kLimitClass;
	if(MakeSkillLimitClassText(pSkillDef,kLimitClass))
	{
		swprintf(temp,100,_T("%s : %s %s\n"),TTW(229).c_str(),kLimitClass.c_str(),TTW(233).c_str());
		wstrTextOut+=temp;
	}*/

	//	제한레벨
	PgPilot	*pkPlayerPilot = g_kPilotMan.GetPlayerPilot();
	if(pkPlayerPilot)
	{
		if(pkPlayerPilot->GetAbil(AT_LEVEL)<pSkillDef->m_sLevelLimit)	//	빨간색으로 표시하자
		{
			wstrTextOut+=TTW(225);
		}
	}

	if(0<pSkillDef->m_sLevelLimit)
	{
		swprintf(temp,100,_T("%s : %d %s\n"),TTW(230).c_str(),pSkillDef->m_sLevelLimit,TTW(232).c_str());
		wstrTextOut+=temp;
	}

	if(!bNextLevel)
		wstrTextOut += TTW(40011);	//폰트 초기화
	else
		wstrTextOut += TTW(226);	//폰트 초기화


	//	길드 제한 레벨
	//CONT_DEF_GUILD_SKILL const *pkDefGuildSkill = NULL;
	//g_kTblDataMgr.GetContDef(pkDefGuildSkill);
	//if( pkDefGuildSkill
	//&&	bGuildSkill )
	//{
	//	CONT_DEF_GUILD_SKILL::const_iterator guildskill_iter = pkDefGuildSkill->find(iSkillNo);
	//	if( pkDefGuildSkill->end() != guildskill_iter )
	//	{
	//		CONT_DEF_GUILD_SKILL::mapped_type const &rkDefGuildSkill = (*guildskill_iter).second;

	//		if( g_kGuildMgr.Level() < rkDefGuildSkill.sGuildLv )
	//		{
	//			wstrTextOut+=TTW(225);//붉은색
	//		}

	//		swprintf(temp,100,_T("%s : %d %s\n"),TTW(400824).c_str(),rkDefGuildSkill.sGuildLv,TTW(232).c_str());
	//		wstrTextOut+=temp;

	//		if( !bNextLevel )	{ wstrTextOut += TTW(40011); }	//폰트 초기화
	//		else				{ wstrTextOut += TTW(226); }	//폰트 초기화
	//	}
	//}

	//	제한무기
	if (pkContDefMap)
	{
		CONT_DEFSKILL::const_iterator def_it = pkContDefMap->find(iSkillNo);
		if (pkContDefMap->end() != def_it)
		{
			CONT_DEFSKILL::mapped_type kT = (*def_it).second;
			if(kT.iWeaponLimit)
			{
				wstrTextOut+=TTW(40132);
				wstrTextOut+=_T(" : ");
				int iCount = 0;
				for (int i = 0; i< 12; ++i)
				{
					int const iWeaponType = 1<<i;
					if (kT.iWeaponLimit & iWeaponType)
					{ 
						if (iCount>0)
						{
							wstrTextOut+=_T(", ");
						}
						int iType = (int)(NiFastLog(iWeaponType) + 1);
						wstrTextOut+=TTW(2000+iType);
						++iCount;
					}
				}
				//int iType = (int)(NiFastLog(kT.iWeaponLimit) + 1);
				//swprintf(temp,500,_T("%s : %s\n"),TTW(40132).c_str(), TTW(2000+iType).c_str());
				wstrTextOut+=_T("\n");
			}
		}
	}
	
	//	시전 시간
	/*int const iCast = pSkillDef->GetAbil(AT_CAST_TIME);
	
	if (0<iCast)
	{
		if( 0 < ((iCast % 1000) * 0.01) )
			swprintf(temp,500,_T("%s : %.1f%s\n"),TTW(47).c_str(), iCast*0.001f, TTW(54).c_str());
		else
			swprintf(temp,500,_T("%s : %d%s\n"),TTW(47).c_str(), static_cast<int>(iCast*0.001f), TTW(54).c_str());
	}
	else
	{
		swprintf(temp, 500, _T("%s : %s\n"), TTW(47).c_str(), TTW(251).c_str());
	}
	wstrTextOut += temp;*/

	//	쿨타임
	int const iCool = pSkillDef->GetAbil(ATS_COOL_TIME);
	if (0<iCool)
	{
		if( 0 < ((iCool % 1000) * 0.01) )
			swprintf(temp,500,_T("%s : %.1f%s\n"),TTW(250).c_str(), iCool*0.001f, TTW(54).c_str());
		else
			swprintf(temp,500,_T("%s : %d%s\n"),TTW(250).c_str(), static_cast<int>(iCool*0.001f), TTW(54).c_str());
		wstrTextOut+=temp;
	}

	//지속 시간
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	const	CEffectDef* pEffDef = kEffectDefMgr.GetDef(pSkillDef->No());
	if (pEffDef)
	{
		int const iDuration = pEffDef->GetDurationTime()/1000;
		if (0<iDuration)
		{
			swprintf(temp,500,_T("%s : "),TTW(40135).c_str());
			wstrTextOut+=temp;
			GetTimeString(iDuration, wstrTextOut, true);wstrTextOut+=_T("\n");
		}
	}

	//	설명
	const TCHAR *pText = NULL;
	int Value = pSkillDef->GetAbil(ATI_EXPLAINID);
	if(Value && GetDefString(Value, pText))
	{
		if (strlenT(pText))
		{
			wstrTextOut+=TTW(40050);
			wstrTextOut+=TTW(226);	//폰트 초기화
			wstrTextOut+=pText;
		}
	}

	return	true;
}

char const * EVENT_MODE7_DPGI = "EVENT_MODE7_DETACHPARTICLE_GUARDIANINSTALL";
float const EFFECT_LIMIT_TIME = 5.0f;
bool PgMissionMgr::ProcessMsg(WORD const wPacketType, BM::Stream &rkPacket)
{
	switch(wPacketType)
	{
	case PT_M_C_NFY_DEFENCE_STAGE:
		{
			//현재 스테이지, 최대 스테이지, 젠 방향, 방향맞추기 성공 여부, 성공횟수
			int iMissionNo = 0, iMissionLv = 0, iStageNo = 0, iStageCount = 0, iSuccessCount;
			BYTE kMonsterDirection = 0;
			bool bSelectSuccess = false;
			
			rkPacket.Pop(iMissionNo);
			rkPacket.Pop(iMissionLv);
			rkPacket.Pop(iStageNo);
			rkPacket.Pop(iStageCount);
			rkPacket.Pop(kMonsterDirection);
			rkPacket.Pop(bSelectSuccess);
			rkPacket.Pop(iSuccessCount);

			lua_tinker::call< void, int, BYTE >("Start_Defence_Mission", iMissionNo, iMissionLv, iStageNo, iStageCount, kMonsterDirection, bSelectSuccess, iSuccessCount);

			NowStage(iStageNo);
			StagePoint(0);
			UpdateEffectGuardianPos();
			SetDefenceMinimapCamera();
		}break;
	case PT_M_C_NFY_DEFENCE_STAGETIME:
		{
			int iStageTime = 0;
			rkPacket.Pop(iStageTime);
			lua_tinker::call< void, int >("Set_Defence_Timer", iStageTime);
		}break;
	case PT_M_C_NFY_DEFENCE_WAVE:
		{
			bool bIsLastWave = false;
			BYTE iDirection = 0;

			rkPacket.Pop(bIsLastWave);
			rkPacket.Pop(iDirection);
			lua_tinker::call< void, bool, BYTE >("Start_Defence_Wave", bIsLastWave, iDirection);
		}break;
	case PT_M_C_NFY_DEFENCE_ENDSTAGE:
		{
			//이거 안씀.
			//lua_tinker::call< void >("End_Defence_Stage");
			/*bool bEnd = false;
			rkPacket.Pop(bEnd);

			// 스테이지 종료 : 다음 스테이지 있다 없다
			if( true == bEnd )
			{
				Net_RecentMapMove();
			}
			else
			{
				// 모든 파티원들이 다 보내줘야 한다. 결과창이 있던 없던....
				BM::Stream kPacket(PT_C_M_NFY_DEFENCE_ENDSTAGE);
				NETWORK_SEND(kPacket)
			}*/
		}break;
	case PT_M_C_NFY_DEFENCE_FAILSTAGE:
		{
			// 스테이지 클리어 실패해서 실패 연출 보여주고 끝나면 마을로 돌아감
			lua_tinker::call< void >("End_Defence_Mission");
		}break;
	case PT_M_C_NFY_DEFENCE_DIRECTION:
		{
			lwUpdate_DefenceSelectUI(rkPacket);
		}break;
	case PT_M_C_NFY_DEFENCE_INFALLIBLE_SELECTION:
		{
			Result_DefenceInfallibleSelection(rkPacket);
		}break;
	case PT_M_C_ANS_DEFENCE_TIMEPLUS:
		{
			Result_DefenceTimePlus(rkPacket);
		}break;
	case PT_M_C_ANS_DEFENCE_POTION:
		{
			Result_DefenceHpPlus(rkPacket);
		}break;
	case PT_M_C_NFY_DEFENCE_STRATEGIC_POINT:
		{
			int const iOldPoint = GetMyTeamPoint();
			int iTeam = 0;
			int OriValue = 0;
			int MulValue = 0;
			EDefencePointType eType = DP_NONE;

			rkPacket.Pop( iTeam );
			rkPacket.Pop( OriValue );
			rkPacket.Pop( MulValue );
			rkPacket.Pop( eType );
			rkPacket.Pop( m_kContStrategicPoint );
			lwUpdateStrategicPointUI();
	
			int const iPoint = GetMyTeamPoint() - iOldPoint;
			if( iPoint > 0 )
			{
				m_kStagePoint += iPoint;
			}			
			
			switch(eType)
			{
			case DP_STAGEEND:
				{
					m_iStageClearPoint = OriValue + MulValue;
				}break;
			}

			if(iTeam==GetMyTeam() && OriValue)
			{
				int const iNo = (OriValue && MulValue) ? 401150 : 401151;

				BM::vstring vStr(TTW(iNo));
				vStr.Replace(L"#POINT#", OriValue);
				vStr.Replace(L"#BONUS#", MulValue);
				vStr.Replace(L"#SUM#", OriValue+MulValue);

				g_kChatMgrClient.AddLogMessage(SChatLog(CT_EXP), vStr);
			}

			SetPointUI(XUIMgr.Get(L"SFRM_DEFENCE_LEARNSKILL"));
			SetPointUI(XUIMgr.Get(L"SFRM_INSTALL_GUARDIAN"));
			SetPointUI(XUIMgr.Get(L"SFRM_UPGRADE_GUARDIAN"));
		}break;
	case PT_M_C_ANS_GUARDIAN_SET:
		{
			BM::GUID kOwnerGuid;
			CONT_GUARDIAN_INSTALL const kOldInstall = m_kGuardianInstall;

			m_kGuardianInstall.clear();
			rkPacket.Pop(kOwnerGuid);
			PU::TLoadTable_AM(rkPacket, m_kGuardianInstall);

			CONT_GUARDIAN_INSTALL kNowInstall = m_kGuardianInstall;

			for(CONT_GUARDIAN_INSTALL::const_iterator c_it=kOldInstall.begin(); c_it!=kOldInstall.end(); ++c_it)
			{
				CONT_GUARDIAN_INSTALL::iterator del_it = kNowInstall.find((*c_it).first);
				if(del_it != kNowInstall.end())
				{
					if( (*del_it).second.kGuid == (*c_it).second.kGuid )
					{
						kNowInstall.erase(del_it);
					}
				}
			}

			UpdateEffectGuardianPos();
			UpdateMinimapGuardian();

			g_kUISound.PlaySoundByID(L"defence-comeon");

			bool bResetUpgradeUI = false;
			for(CONT_GUARDIAN_INSTALL::const_iterator c_it=kNowInstall.begin(); c_it!=kNowInstall.end(); ++c_it)
			{
				if(m_kGuardianUpgradeLoc == (*c_it).first)
				{
					bResetUpgradeUI = true;
				}

				if(false==g_kPilotMan.IsMyPlayer(kOwnerGuid))
				{
					continue;
				}

				//5초간 시야 이펙트 보여주기
				NiPoint3 kEffectPos(0.f,0.f,0.f);
				CONT_TRIGGERPOS::const_iterator c_iter = m_kContGuardianTriggerPos.find( (*c_it).first );
				if(c_iter != m_kContGuardianTriggerPos.end())
				{
					kEffectPos = c_iter->second;
				}

				if( g_pkWorld
					&& kEffectPos != NiPoint3::ZERO && (*c_it).second.iGuardianNo
					)
				{
					float const fPos = 20.f;
					kEffectPos.z += fPos;
					NiPoint3 kFloorPos = g_pkWorld->ThrowRay(kEffectPos,NiPoint3(0,0,-1),500);
					kFloorPos.z += 4.f;

					lwActor kGuardianActor(g_kPilotMan.FindActor((*c_it).second.kGuid));
					if(false==kGuardianActor.IsNil())
					{
						SMISSION_DEFENCE7_GUARDIAN kGuardian = GetDefGuardian((*c_it).second.iGuardianNo);
						if(kGuardian.iMonsterNo > 0)
						{
							int const iSlotNo = 463743;
							float fEffectSize = lua_tinker::call<float>("GuardianDetectRangeEffectSize");
							fEffectSize = GetCalcDefMonsterAbil(kGuardian.iMonsterNo,AT_DETECT_RANGE) / fEffectSize;
							kGuardianActor.AttachParticleToPointWithRotate(iSlotNo,lwPoint3(kFloorPos),lua_tinker::call<char*>("GuardianDetectRangeEffectName"),lwQuaternion(0,0,0,0),fEffectSize);

							BM::vstring vStr(EVENT_MODE7_DPGI);
							static int iIdx = 0;
							vStr += (++iIdx);
							char szEventScript[MAX_PATH] = {0, };
							sprintf_s(szEventScript, MAX_PATH, "local kActor = g_world:FindActor(GUID('%s')); if false==kActor:IsNil() then kActor:DetachFrom(%d) end", kGuardianActor.GetPilotGuid().GetString(), iSlotNo);

							STimerEvent kTimerEvent;
							kTimerEvent.Set(EFFECT_LIMIT_TIME, szEventScript);
							g_kEventTimer.AddLocal(MB(vStr), kTimerEvent);
						}
					}
				}
			}

			if(bResetUpgradeUI)
			{
				CONT_GUARDIAN_INSTALL::const_iterator guardian_it = m_kGuardianInstall.find(m_kGuardianUpgradeLoc);
				if(guardian_it != m_kGuardianInstall.end())
				{
					int const iGuardianNo = (*guardian_it).second.iGuardianNo;
					SetGuardianUpgradeUI(XUIMgr.Get(L"SFRM_UPGRADE_GUARDIAN"), iGuardianNo, GetDefGuardian(iGuardianNo));
				}
			}
		}break;
	case PT_M_C_ANS_DEFENCE_SKILL_USE:
		{
			CONT_DEFENCE_SKILL kContDefenceSkill;
			PU::TLoadTable_AA(rkPacket, kContDefenceSkill);
			m_kDefenceSkill.Set(kContDefenceSkill);
			OnCallDefenceLearnSkill(XUIMgr.Get(L"SFRM_DEFENCE_LEARNSKILL"));
			UpdateGuardianSkillUI();
			lwCloseToolTip();
		}break;
	case PT_M_C_ANS_TEAM_POINT:
		{
			CONT_TEAM_POINT	m_kTeamPoint;

			rkPacket.Pop(m_kTeamPoint);
		}break;
	case PT_M_C_ANS_DEFENCE_PARTYLIST:
		{
			CONT_DEFENCE_PARTY_LIST rkWaitCont, rkPlayCont;

			rkPacket.Pop( rkWaitCont );	// 대기중<팀 파티원 수, 참여인원>
			rkPacket.Pop( rkPlayCont ); // 진행중<팀 파티원 수, 참여인원>

			UpdateCreateDefenceUI(rkWaitCont, rkPlayCont);
		}break;
	case PT_M_C_ANS_MISSION_START:
		{
			lua_tinker::call<void>("CallMissionStart");
		}break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

bool lwCheckHaveMissionEnterItem(int iIndex)
{ //미션 입장시 필요 아이템이 있는지 여부 체크
	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if(NULL == pkMyPlayer) { return false; }

	PgInventory* pkInv = pkMyPlayer->GetInven();
	if(NULL == pkInv) { return false; }

	int iNeedItemNo = lwGetMissionRequiredItemNo(iIndex);
	if(iNeedItemNo > 0)
	{
		SItemPos kItemPos;
		if(S_OK != pkInv->GetFirstItem(iNeedItemNo, kItemPos, false))
		{
			return false;
		}
	}

	return true;
}

bool CheckScenarioMissionEnter()
{
	int iLv5QuestNo = 0;
	int iLv6QuestNo = 0;
	bool IsLv5Ended = false;
	bool IsLv6Ended = false;

	CONT_DEF_QUEST_REWARD const* pkDefReward = NULL;
	g_kTblDataMgr.GetContDef(pkDefReward);

	if( pkDefReward )
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			PgMyQuest const* pkUserQuest = pkPlayer->GetMyQuest();
			if( pkUserQuest )
			{
				if( 1 < g_VecLV5QuestValue.size() )
				{	
					int idx = 0;
					if( pkPlayer->UnitRace() == ERACE_DRAGON )
					{
						idx = (pkPlayer->GetAbil(AT_BASE_CLASS) % 10) + 4; // 인간 클래스 더함.
					}
					else
					{
						idx = pkPlayer->GetAbil(AT_BASE_CLASS);	// 1. 전사 2. 마법사 3. 궁수 4. 도둑
					}

					if( idx <= g_VecLV5QuestValue.size() )
					{
						iLv5QuestNo = g_VecLV5QuestValue.at(idx - 1);
					}
				}
				else if( !g_VecLV5QuestValue.empty() )
				{
					iLv5QuestNo = g_VecLV5QuestValue.at(0);
				}

				if( 1 < g_VecLV6QuestValue.size() )
				{	
					int idx = 0;
					if( pkPlayer->UnitRace() == ERACE_DRAGON )
					{
						idx = (pkPlayer->GetAbil(AT_BASE_CLASS) % 10) + 4; // 인간 클래스 더함.
					}
					else
					{
						idx = pkPlayer->GetAbil(AT_BASE_CLASS);	// 1. 전사 2. 마법사 3. 궁수 4. 도둑			
					}

					if( idx <= g_VecLV6QuestValue.size() )
					{
						iLv6QuestNo = g_VecLV6QuestValue.at(idx - 1);	
					}
				}
				else if( !g_VecLV6QuestValue.empty() )
				{
					iLv6QuestNo = g_VecLV6QuestValue.at(0);
				}

				if( ( (iLv5QuestNo != 0) ? (pkUserQuest->IsEndedQuest(iLv5QuestNo)) : true ) 
					&& ( (iLv6QuestNo != 0)  ? (pkUserQuest->IsEndedQuest(iLv6QuestNo)) : true ) 
					&& g_VecHiden5ClearQuestvalue.size() == 0 )
				{
					std::wstring kText(TTW(799384));
					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddLogMessage(kChatLog, kText, true);
					return false;
				}

				if( pkUserQuest->IsIngQuest(iLv5QuestNo) )
				{
					return true;
				}
				else if( !pkUserQuest->IsIngQuest(iLv5QuestNo) )
				{
					if( 0 == iLv6QuestNo )
					{
						CONT_DEF_QUEST_REWARD::const_iterator iter = pkDefReward->find(iLv5QuestNo);
						if( iter != pkDefReward->end() )
						{
							BM::vstring kText(TTW(799383));
							kText.Replace(L"#QUEST#", TTW(iter->second.iTitleText));
							SChatLog kChatLog(CT_EVENT);
							g_kChatMgrClient.AddLogMessage(kChatLog, kText, true);
							return false;
						}
					}
					else
					{
						if( pkUserQuest->IsEndedQuest(iLv5QuestNo) && pkUserQuest->IsIngQuest(iLv6QuestNo) )
						{
							return true;
						}
						else if(pkUserQuest->IsEndedQuest(iLv5QuestNo) && pkUserQuest->IsEndedQuest(iLv6QuestNo))
						{
							return true;
						}
						else if( pkUserQuest->IsEndedQuest(iLv5QuestNo) && !pkUserQuest->IsIngQuest(iLv6QuestNo) )
						{
							CONT_DEF_QUEST_REWARD::const_iterator iter = pkDefReward->find(iLv6QuestNo);
							if( iter != pkDefReward->end() )
							{
								BM::vstring kText(TTW(799383));
								kText.Replace(L"#QUEST#", TTW(iter->second.iTitleText));
								SChatLog kChatLog(CT_EVENT);
								g_kChatMgrClient.AddLogMessage(kChatLog, kText, true);
								return false;
							}
						}
						else
						{
							CONT_DEF_QUEST_REWARD::const_iterator iter = pkDefReward->find(iLv5QuestNo);
							if( iter != pkDefReward->end() )
							{
								BM::vstring kText(TTW(799383));
								kText.Replace(L"#QUEST#", TTW(iter->second.iTitleText));
								SChatLog kChatLog(CT_EVENT);
								g_kChatMgrClient.AddLogMessage(kChatLog, kText, true);
								return false;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

int lwGetDefence7_RelayPoint(int const iStage)
{
	const CONT_MISSION_DEFENCE7_STAGE_BAG *pkStage;
	g_kTblDataMgr.GetContDef(pkStage);
	if( !pkStage )
	{
		return 0;
	}

	int const iMissionNo = g_iMissionNo;
	int const iPartyNumber = std::max<int>( 1, g_kParty.MemberCount() );
	int const iModeType = GetDefenceModeType( lwGetGameLevel() );
	CONT_MISSION_DEFENCE7_STAGE_BAG::key_type kKey(iMissionNo, iModeType, iPartyNumber, iStage);

	CONT_MISSION_DEFENCE7_STAGE_BAG::const_iterator iter = pkStage->find(kKey);
	if( iter != pkStage->end() )
	{
		CONT_MISSION_DEFENCE7_STAGE_BAG::mapped_type const& kElement = (*iter).second;
		CONT_MISSION_DEFENCE7_STAGE::value_type const& kValue = kElement.kCont.at(0);
		return kValue.iUseCoin_StrategicPoint;
	}
	return 0;
}

int lwRecommendMissionQuestCount(int const index)
{
	if( 0 == index || 1 == index )
	{
		return g_MissionQuestCount[index];
	}
	return -1;
}

int lwGetMissionRequiredItemNo(int const index)
{
	if(g_VecRequiredItemNo.size() <= index) { return 0; }

	return g_VecRequiredItemNo.at(index);
}

void lwSetEventMission(int iEventMission)
{
	g_iEventMission = iEventMission;
}

int lwGetEventMission(void)
{
	return g_iEventMission;
}

int lwRecommendMissionAchievementCount(int const index)
{
	if( 0 == index || 1 == index )
	{
		return g_MissionAchievementCount[index];
	}
	return -1;
}

void lwMissionUISetQuest(lwUIWnd lwWnd)
{
	XUI::CXUI_Wnd * TopWnd = lwWnd();
	if( NULL == TopWnd )
	{
		return;
	}

	CONT_DEF_MISSION_ROOT const * pContDefMission = NULL;
	g_kTblDataMgr.GetContDef(pContDefMission);
	if( NULL == pContDefMission )
	{
		return;
	}

	CONT_DEF_MISSION_CANDIDATE const * pContDefMissionCandi = NULL;	// 미션 Candidate 테이블 가져오기
	g_kTblDataMgr.GetContDef(pContDefMissionCandi);
	if ( NULL == pContDefMissionCandi )
	{
		return;
	}

	
	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if( NULL == pPlayer )
	{
		return;
	}

	PgMyQuest const * pMyQuest = pPlayer->GetMyQuest();
	if( NULL == pMyQuest )
	{
		return;
	}

	ContUserQuestState QuestCont;
	pMyQuest->GetQuestList(QuestCont);	// 유저가 진행중인 퀘스트 리스트를 가져온다.

	for( int index = 0; index < 2; ++index )
	{
		BM::vstring FormName("FRM_CARD");
		FormName += index;
		XUI::CXUI_Wnd * CardWnd = TopWnd->GetControl(FormName);
		if( CardWnd )
		{
			XUI::CXUI_Wnd * QuestWnd1 = CardWnd->GetControl(_T("FRM_QUEST_1"));
			XUI::CXUI_Wnd * QuestWnd2 = CardWnd->GetControl(_T("FRM_QUEST_2"));
			XUI::CXUI_Wnd * QuestWnd3 = CardWnd->GetControl(_T("FRM_QUEST_3"));
			if( QuestWnd1 && QuestWnd2 && QuestWnd3 )
			{
				QuestWnd1->Visible( false );
				QuestWnd2->Visible( false );
				QuestWnd3->Visible( false );

				CONT_RECOMMEND_MISSION_GROUND MissionGround;
				CONT_RECOMMEND_MISSION_QUEST MissionQuest;
				if( RecommendMisssionMap(index, pContDefMission, pContDefMissionCandi, QuestCont, MissionGround, MissionQuest) )
				{
					int const QuestCount = MissionQuest.size();
					if( 1 == QuestCount )
					{
						QuestWnd3->Visible( true );
					}
					else if( 2 == QuestCount )
					{
						QuestWnd2->Visible( true );
						QuestWnd3->Visible( true );
					}
					else if( 2 < QuestCount )
					{
						QuestWnd1->Visible( true );
						QuestWnd2->Visible( true );
						QuestWnd3->Visible( true );
					}

					CONT_RECOMMEND_MISSION_QUEST::const_iterator iter = MissionQuest.begin();
					if( QuestWnd3->Visible() && MissionQuest.end() != iter )
					{
						std::wstring ContentText(TTW(iter->second));
						SetTextToFitWindowSize(QuestWnd3, ContentText, iter->first, 55);
						++iter;
					}
					if( QuestWnd2->Visible() && MissionQuest.end() != iter )
					{
						std::wstring ContentText(TTW(iter->second));
						SetTextToFitWindowSize(QuestWnd2, ContentText, iter->first, 55);
						++iter;
					}
					if( QuestWnd1->Visible() && MissionQuest.end() != iter )
					{
						std::wstring ContentText(TTW(iter->second));
						SetTextToFitWindowSize(QuestWnd1, ContentText, iter->first, 55);
					}
				}
			}
		}
	}
}

bool RecommendMisssionMap(int const index,
						  CONT_DEF_MISSION_ROOT const * pContDefMission,
						  CONT_DEF_MISSION_CANDIDATE const * pContDefMissionCandi,
						  ContUserQuestState const & QuestCont,
						  CONT_RECOMMEND_MISSION_GROUND &MissionGround,
						  CONT_RECOMMEND_MISSION_QUEST &MissionQuest)
{
	if( !(0 == index || 1 == index) )
	{// index가 0 이면 아케이드, 1 이면 카오스
		return false;
	}

	if( 0 == g_iMissionNo )
	{// 현재 선택한 미션의 번호를 저장한 전역변수
		return false;
	}

	// 미션번호로 해당 미션 찾기
	int RootLevelValue = 0;
	CONT_DEF_MISSION_ROOT::const_iterator mission_itor = pContDefMission->find(g_iMissionNo);
	if( pContDefMission->end() != mission_itor )
	{// 미션의 각 맵의 번호가 저장된 Candidate에 접근하기 위한 키값을 찾는다.
	 // 참고로 index는 미션의 종류.  0-아케이드 1-카오스
	 // 미션 창의 index(빌드로 되어있음)와 배열의 순서가 같으므로 그대로 사용함.
		RootLevelValue = mission_itor->second.aiLevel[index];
	}

	if ( 0 == RootLevelValue )
	{
		return false;
	}

	// 키 값으로 유저가 선택한 미션의 Candidate에 접근
	CONT_DEF_MISSION_CANDIDATE::const_iterator candi_itr = pContDefMissionCandi->find(RootLevelValue);
	if( pContDefMissionCandi->end() == candi_itr )
	{
		return false;
	}

	// 유저가 선택한 미션의 각 맵번호가 저장된 공간에 접근 완료
	const CONT_DEF_MISSION_CANDIDATE::mapped_type Element = candi_itr->second;
	if( 0 == Element.iCandidate_End)
	{
		return false;
	}

	enum RecommendType
	{
		RT_NONE = 0,
		RT_QUEST,
		RT_ACHIEVEMENT,
	};

	// 퀘스트 추천 /////////////////////////////////////////////////////////////////////////////
	int RecommendGroundCount = 0;
	ContUserQuestState::const_iterator state_iter;
	for( state_iter = QuestCont.begin(); QuestCont.end() != state_iter; ++state_iter )
	{
		ContUserQuestState::value_type const& UserQuestState = (*state_iter);
		if( QS_Ing == UserQuestState.byQuestState )
		{
			PgQuestInfo const* pQuestInfo = g_kQuestMan.GetQuest(UserQuestState.iQuestID);
			if( pQuestInfo )
			{// 퀘스트 정보
				ContQuestMonster::const_iterator mon_iter = pQuestInfo->m_kDepend_Monster.begin();
				ContQuestGround::const_iterator gnd_iter = pQuestInfo->m_kDepend_Ground.begin();
				ContQuestDependMission::const_iterator mis_iter = pQuestInfo->m_kDepend_Mission.begin();
				ContQuestLocation::const_iterator loc_iter = pQuestInfo->m_kDepend_Location.begin();

				for( ; pQuestInfo->m_kDepend_Location.end() != loc_iter; ++loc_iter )
				{
					ContQuestLocation::value_type const& QuestLocation = (*loc_iter);
					
					bool bQuestLocationClear = false;
					if( pQuestInfo->m_kObject.GetEndCount(QuestLocation.iObjectNo) > 0 )
					{// 퀘스트 목표 갯수가 0개 이상일 때
						if( UserQuestState.byParam[QuestLocation.iObjectNo] >= pQuestInfo->m_kObject.GetEndCount(QuestLocation.iObjectNo) )
						{// 해당 조건을 완료했다.
							bQuestLocationClear = true;
						}
					}

					if( QuestLocation.iGroundNo && (false == bQuestLocationClear) )
					{
						if( QuestLocation.iGroundNo == Element.iCandidate_End )
						{
							if( RecommendGroundCount < 1)
							{
								RecommendGroundCount = 1;
							}
							MissionQuest.insert( std::make_pair( pQuestInfo->ID(), pQuestInfo->m_iTitleTextNo ) );
						}
						else
						{
							for( int Candi_index = 0; Candi_index < MAX_MISSION_CANDIDATE; ++Candi_index )
							{// 모든 미션 맵들에 대해 검사한다.
								if ( 0 != Element.aiCandidate[Candi_index] )
								{// 미션 맵의 번호가 유효하고
									if( QuestLocation.iGroundNo == Element.aiCandidate[Candi_index] )
									{// 몬스터의 맵번호와 미션의 맵번호가 같다면
										MissionQuest.insert( std::make_pair( pQuestInfo->ID(), pQuestInfo->m_iTitleTextNo ) );
										MissionGround.insert( std::make_pair( QuestLocation.iGroundNo, RT_QUEST ) );
									}
								}
							}
						}
					}
				}

				for( ; pQuestInfo->m_kDepend_Mission.end() != mis_iter; ++mis_iter )
				{
					if( ( mis_iter->iMissionKey == mission_itor->second.iKey )
						&& ( mis_iter->iVal1 == index ) )
					{
						int TempCount = 0;
						switch( mis_iter->iType )
						{
						case QET_MISSION_Rank:	// 지정한 미션에서 지정한 랭크를 달성시 단순 n증가
						case QET_MISSION_RankN:	// 지정한 미션의 지정된 맵수량에서 지정한 랭크를 달성시 단순 n 증가
							{
								TempCount = 3;
							}break;
						case QET_MISSION_Clear:	// 지정한 미션을 클리어 했거나(한다면) 단순 n증가
							{
								TempCount = 1;
							}break;
						case QET_MISSION_ClearN:// 지정한 미션의 지정된 맵수량을 클리어 했거나(한다면) 단순 n 증가
							{
								TempCount = mis_iter->iStageCount;
							}break;
						case QET_MONSTER_IncParam_M:	// 몬스터를 죽이면 Parameter 증가. 단, 특정 미션의 난이도에서
						case QET_MONSTER_IncParam100_M:
						case QET_MONSTER_IncParam_MN:	// 몬스터를 죽이면 Parameter 증가. 단, 특정 미션의 특정 난이도를 특정 맵장수로 입장했을 때
						case QET_MONSTER_IncParam100_MN:
						case QET_MONSTER_InstDrop_M:	// 몬스터를 죽이면 아이템 드롭. 단, 특정 미션의 난이도에서
						case QET_MONSTER_InstDrop100_M:
						case QET_MONSTER_InstDrop_MN:	// 몬스터를 죽이면 아이템 드롭. 단, 특정 미션의 특정 난이도를 특정 맵장수로 입장했을 때
						case QET_MONSTER_InstDrop100_MN:
							{
								if( mis_iter->iStageCount )
								{
									TempCount = mis_iter->iStageCount;
								}
								else
								{
									TempCount = 1;
								}
							}break;
						}
						if( RecommendGroundCount < TempCount)
						{
							RecommendGroundCount = TempCount;
						}
						MissionQuest.insert( std::make_pair( pQuestInfo->ID(), pQuestInfo->m_iTitleTextNo ) );
						break;
					}
				}

				for( ; pQuestInfo->m_kDepend_Ground.end() != gnd_iter; ++gnd_iter )
				{// 컨테이너를 순회하며 퀘스트에 필요한 모든 그라운드를 비교
					ContQuestGround::mapped_type const& QuestGround = (*gnd_iter).second;
					if( QuestGround.iGroundNo == Element.iCandidate_End )
					{
						if( RecommendGroundCount < 1)
						{
							RecommendGroundCount = 1;
						}
						MissionQuest.insert( std::make_pair( pQuestInfo->ID(), pQuestInfo->m_iTitleTextNo ) );
					}
					else
					{
						for( int Candi_index = 0; Candi_index < MAX_MISSION_CANDIDATE; ++Candi_index )
						{// 모든 미션 맵들에 대해 검사한다.
							if ( 0 != Element.aiCandidate[Candi_index] )
							{// 미션 맵의 번호가 유효하고
								if( QuestGround.iGroundNo == Element.aiCandidate[Candi_index] )
								{// 퀘스트 그라운드 번호와 미션의 맵번호가 같다면
									MissionQuest.insert( std::make_pair( pQuestInfo->ID(), pQuestInfo->m_iTitleTextNo ) );
									MissionGround.insert( std::make_pair( QuestGround.iGroundNo, RT_QUEST ) );
								}
							}
						}
					}
				}

				int TempCount2 = INT_MAX;
				for( ; pQuestInfo->m_kDepend_Monster.end() != mon_iter; ++mon_iter )
				{// 컨테이너를 순회하며 퀘스트에 필요한 모든 몬스터를 비교
					ContQuestMonster::mapped_type const& QuestMonster = (*mon_iter).second;
					
					bool bQuestMonsterClear = false;
					if( pQuestInfo->m_kObject.GetEndCount(QuestMonster.iObjectNo) > 0 )
					{// 퀘스트 목표 갯수가 0개 이상일 때
						if( UserQuestState.byParam[QuestMonster.iObjectNo] >= pQuestInfo->m_kObject.GetEndCount(QuestMonster.iObjectNo) )
						{// 해당 조건을 완료했다.
							bQuestMonsterClear = true;
						}
					}

					switch( mon_iter->second.iType )
					{
					case QET_MONSTER_IncParam_In:
					case QET_MONSTER_IncParam100_In:
					case QET_MONSTER_InstDrop_In:
					case QET_MONSTER_InstDrop100_In:
						{//몬스터가 등장할 맵이 명확할 때
							if( QuestMonster.iTargetGroundNo && (false == bQuestMonsterClear) )
							{
								if( QuestMonster.iTargetGroundNo == Element.iCandidate_End )
								{
									if( RecommendGroundCount < 1)
									{
										RecommendGroundCount = 1;
									}
									MissionQuest.insert( std::make_pair( pQuestInfo->ID(), pQuestInfo->m_iTitleTextNo ) );
								}
								else
								{
									for( int Candi_index = 0; Candi_index < MAX_MISSION_CANDIDATE; ++Candi_index )
									{// 모든 미션 맵들에 대해 검사한다.
										if ( 0 != Element.aiCandidate[Candi_index] )
										{// 미션 맵의 번호가 유효하고
											if( QuestMonster.iTargetGroundNo == Element.aiCandidate[Candi_index] )
											{// 몬스터의 맵번호와 미션의 맵번호가 같다면
												MissionQuest.insert( std::make_pair( pQuestInfo->ID(), pQuestInfo->m_iTitleTextNo ) );
												MissionGround.insert( std::make_pair( QuestMonster.iTargetGroundNo, RT_QUEST ) );
											}
										}
									}
								}
							}
						}break;
					case QET_MONSTER_IncParam_M:
					case QET_MONSTER_IncParam100_M:
					case QET_MONSTER_IncParam_MN:
					case QET_MONSTER_IncParam100_MN:
					case QET_MONSTER_InstDrop_M:
					case QET_MONSTER_InstDrop100_M:
					case QET_MONSTER_InstDrop_MN:
					case QET_MONSTER_InstDrop100_MN:
						{//몬스터가 등장할 맵이 정해져있지 않고 미션 키만 있을 때
							if( mon_iter->second.iMissionLevel == index )
							{
								if( mon_iter->second.iMissionKey == mission_itor->second.iKey )
								{
									if( TempCount2 > mon_iter->second.iStageCount )
									{
										if( mon_iter->second.iStageCount )
										{
											TempCount2 = mon_iter->second.iStageCount;
										}
										else
										{
											TempCount2 = 1;
										}
										MissionQuest.insert( std::make_pair( pQuestInfo->ID(), pQuestInfo->m_iTitleTextNo ) );
									}
								}
							}
						}break;
					}
				}

				if( TempCount2 != INT_MAX && RecommendGroundCount < TempCount2 )
				{
					RecommendGroundCount = TempCount2;
				}
			}
		}
	}
	// 퀘스트 추천 끝 //////////////////////////////////////////////////////////////////////////

	// 달성률 추천 /////////////////////////////////////////////////////////////////////////////
	if( MissionQuest.empty() )
	{// 퀘스트가 없을 경우에만 미달성 스테이지 갯수 체크함
		for( int StageNo = 0; StageNo < MAX_MISSION_CANDIDATE; ++StageNo )
		{
			if( false == PlayerMissionData.IsClearStage(index, StageNo) )
			{
				if( candi_itr->second.aiCandidate[StageNo] )
				{
					MissionGround.insert( std::make_pair( candi_itr->second.aiCandidate[StageNo], RT_ACHIEVEMENT ) );
				}
			}
		}
	}
	// 달성률 추천 끝 //////////////////////////////////////////////////////////////////////////

	if( 0 == index || 1 == index )
	{
		g_MissionQuestCount[index] = 0;
		g_MissionAchievementCount[index] = 0;

		CONT_RECOMMEND_MISSION_GROUND::const_iterator iter;
		for( iter = MissionGround.begin(); iter != MissionGround.end(); ++iter )
		{
			if( iter->second == RT_QUEST )
			{
				++g_MissionQuestCount[index];
			}
			else if( iter->second == RT_ACHIEVEMENT )
			{
				++g_MissionAchievementCount[index];
			}
		}
		
		if( g_MissionQuestCount[index] < RecommendGroundCount )
		{
			g_MissionQuestCount[index] = RecommendGroundCount;
		}
	}
	return true;
}
