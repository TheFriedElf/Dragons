#include "stdafx.h"
#include "PgNifMan.h"
#include "PgWorld.h"
#include "PgRenderer.h"
#include "PgMobileSuit.H"
#include "PgMissionComplete.h"
#include "XUI/XUI_Manager.h"
#include "PgUIScene.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "lwUI.h"
#include "PgSoundMan.h"
#include "lwBase.h"
#include "PgPartyBalloon.h"
#include "PgExpeditionBalloon.h"
#include "PgTextBalloon.h"
#include "ServerLib.h"
#include "PgRenderMan.h"
#include "variant/TableDataManager.h"
#include "PgClientParty.H"
#include "PgClientExpedition.H"
#include "PgCommandMgr.h"
#include "PgEventScriptSystem.H"
#include "lwmonsterKillUIMgr.h"
#include "PgChatMgrClient.h"
#include "lwUIQuest.h"
#include "PgEnergyGuage.H"
#include "PgOption.h"
#include "lwUIMission.h"
#include "PgHelpSystem.h"
#include "lwUI.h"

// EXPEDITION ////////////////////////////////////////////////
float const RESULT_MOVE_TIME = 0.2f;		// 타이틀 이미지 움직이는 시간
float const RESULT_MOVE_END_TIME = 0.6;		// 타이틀 무브 페이즈에 총 걸리는 시간
float const BESTPLAY_MOVE_TIME = 0.2f;		// 베스트플레이 창 나오는 시간
float const REWARD_MOVE_TIME = 0.2f;		// 보상 창 나오는 시간
float const TENDER_MOVE_TIME = 0.4f;		// 특별보상 창 나오는 시간
float const BESTPLAY_REWARD_TIME = 1.0f;	// 특별보상창 나오기 전의 대기 시간
float const BESTPLAY_DISPLAY_TIME = 0.2f;	// 베스트플레이어 연출 나오는 시간
float const TENDER_ITEM_SHOW = 3.0f;		// 특별보상 아이템 이벤트 시간
float const TENDER_WINNER_SHOW = 3.0f;		// 당첨자 이벤트 시간
float const TENDER_START_WAIT_TIME = 20.0f;	// 특별보상 관련 패킷 기다리는 시간
float const NAME_CHANGE_TIME = 0.1;			// 당첨자 연출 때 이름 바뀌는 시간의 간격
float const ITEM_CHANGE_TIME = 0.3;			// 아이템 연출 때 아이템 바뀌는 시간의 간격
float const TENDER_LIMIT_TIME = 20.0f;		// 특별보상 입찰제한 시간
float const ENDING_WAIT_TIME = 1.0f;		// 엔딩 전 대기 시간
float const ENDING_UI_MOVE_TIME = 0.5f;		// UI가 좌우로 빠지는 시간
//////////////////////////////////////////////////////////////
float const BG_COME_TIME = 0.5f;	//배경이 다가올 시간
float const BOX_INTER_X = 60.0f;	//박스사이의 간격
float const BOX_INTERVAL = 32.0f;	//박스와 이름등의 간격
float const BOX_DROP_TIME = 0.2f;	//박스가 떨어지는 시간
float const BOX_DROP_TIME_STEP = 0.5f;	// 박스가 떨어지는 시간의 간격
float const MSN_CPL_WAIT_TIME = 0.0f;	//완료화면 나오기 전까지 대기시간
float const MSN_CPL_WAIT_TIME_BOSS = 15.0f;	//완료화면 나오기 전까지 대기시간
float const BOX_OPEN_BEFORE_MOVE_TIME = 0.5f;	// 박스가 열리기전에 위치를 잡는 시간
float const BOX_OPEN_TIME = 1.5f;	//박스가 열리고 빛이 나오는 시간
float const BOX_SELECT_TIME = 5.0f;	//박스 고르는 시간
float const BOX_SELECT_DELAY_TIME = 1.0f;	//박스 고르는 대기 시간
float const BG_COME_LENGTH = 650.0f;	//배경이 움직일 거리
float const BOX_COME_LENGTH = 240;	//박스가 움직일 거리
float const BG_FADE_TIME = 2.5f;		//화면이 검게 변하는 시간
float const BG_FADE_TIME_MISSION = 2.0f;		//화면이 검게 변하는 시간
float const UI_MOVE_TIME = 0.2f;		//양쪽 UI가 닫히는데 걸리는 시간.
float const BG_WHITE_FADE_TIME = 0.8f;		//화면이 하얗게 변하는 시간
float const UI_ICON_APPEAR_TIME = 2.0f;	//아이콘이 등장하는데 걸리는 시간
float const UI_CLOSE_TIME = 6.0f;		//아이콘이 등장하고 UI가 닫히는데 걸리는 시간.
float const UI_TITLE_MOVE_LENGTH = 300;	//미션 리절트 그림이 움직이는 거리
float const ENDING_TIME = 1.0f;			//배경 알파가 빠지는 시간
float const ENDING_UI_TIME = 0.3f;			//UI가 좌우로 움직이는 시간
float const BOX_BACK_LENGTH = 60.0f;		// 카메라 뒤로 얼마나 빠질건가
float const BOX_UP_LENGTH = 20.0f;			// 박스가 업벡터 방향으로 얼마나 올라갈 건가

//////////////////////////////////////////////////////////////////////////
float const UI_CLOSE_TIME_2 = 40.0f;	//완료버튼 안눌러서 자동으로 UI가 닫혀야 하는 시간
float const WAIT_RETRY_TIME = 180.0f;	//재시작 기다리는 시간
float const UI_CLOSE_BUTTON_TIME = 10.0f;	//아이템 지급후 완료 버튼이 나오는 시간(최대)

int const MAX_EXPEDITION_REWARD_ITEM_COUNT = 6;
int const MAX_DEFENCE_REWARD_ITEM_COUNT = 3;
int const MAX_REWARD_ITEM_COUNT = 4;

extern NiQuaternion g_kSpeed;

bool IsPlayedBoxSound[REWARD_BOX_COUNT] = {0,};
bool bTimerStart = false;

char const UIMODEL_NAME[] = "ef_ms_rank";
char const UIMODEL_NIF_NAME[] = "ef_ms_rank_NIF";
char const UIMODEL_RANK_PATH[] = "../Data/5_Effect/4_UI/ef_ms_rank.nif";
const POINT2 UIMODEL_POS(100, 230);//const POINT2 UIMODEL_POS(0, 50);

int const UI_EFF_CNT = 6;
char const UIMODEL_NUM_EFF_NAME[] = "ef_endspot";
//char const UIMODEL_NUM_EFF_NIF_NAME[UI_EFF_CNT][] = {"ef_endspot_NIF_0", "ef_endspot_NIF_1", "ef_endspot_NIF_2", "ef_endspot_NIF_3", "ef_endspot_NIF_4", "ef_endspot_NIF_5"};
//char const UIMODEL_NUM_EFF_PATH[UI_EFF_CNT][] ={ "../Data/5_Effect/4_UI/ef_endspot_01.nif",  "../Data/5_Effect/4_UI/ef_endspot_01.nif",
//"../Data/5_Effect/4_UI/ef_endspot_01.nif",  "../Data/5_Effect/4_UI/ef_endspot_01.nif",  "../Data/5_Effect/4_UI/ef_endspot_01.nif",  "../Data/5_Effect/4_UI/ef_endspot_01.nif"};
char const UIMODEL_NUM_EFF_NIF_NAME[] = "ef_endspot_NIF";
char const UIMODEL_NUM_EFF_PATH[] = "../Data/5_Effect/4_UI/ef_endspot_01.nif";
const POINT2 UIMODEL_NUM_EFF_POS(285, 55);
const POINT3 UIMODEL_NUM_EFF_LOCAL_POS[UI_EFF_CNT];

int const MAX_RANK_BONUS = 120;

int const RANK1_IMG_INDEX = 2;
int const RANK2_IMG_INDEX = 3;
int const RANK3_IMG_INDEX = 4;
unsigned long const RANK1_TAIL_TEXTID = 404900;
unsigned long const RANK2_TAIL_TEXTID = 404901;
unsigned long const RANK3_TAIL_TEXTID = 404902;
unsigned long const AFTERRANK3_TAIL_TEXTID = 404903;

PgMissionComplete::PgMissionComplete() : m_spWhiteBillboard(0), m_spAlphaMaterial(0), m_spBGAlphaMaterial(0), m_spNewAlphaProp(0), m_pkWndModel(NULL)
{
	m_vecBoxPos.reserve(REWARD_BOX_COUNT);
	m_vecBoxName.reserve(REWARD_BOX_COUNT);

	TimeStamp(0);
	m_pkWnd = NULL;
	m_spBackGround = 0;
	Init();
	MemberCount(0);

	g_kUIScene.InitRenderModel(UIMODEL_NAME, POINT2(550,550), UIMODEL_POS, false);

	m_pkWndUIModel = g_kUIScene.FindUIModel(UIMODEL_NAME);
	if (m_pkWndUIModel)
	{
		m_pkWndUIModel->AddNIF(UIMODEL_NIF_NAME, g_kNifMan.GetNif(UIMODEL_RANK_PATH), false, true);
		m_pkWndUIModel->SetCameraZoomMinMax(-30, 30);
		m_pkWndUIModel->CameraZoom(5.8f);
	}

	g_kUIScene.InitRenderModel(UIMODEL_NUM_EFF_NAME, POINT2(600,600), UIMODEL_NUM_EFF_POS, false);

	m_pkWndUIModel_NumEff = g_kUIScene.FindUIModel(UIMODEL_NUM_EFF_NAME);
	if (m_pkWndUIModel_NumEff)
	{
		m_pkWndUIModel_NumEff->AddNIF(UIMODEL_NUM_EFF_NIF_NAME, g_kNifMan.GetNif(UIMODEL_NUM_EFF_PATH), false, true);
		/*for(int i = 0; i<UI_EFF_CNT; ++i)
		{
			m_pkWndUIModel_NumEff->AddNIF(UIMODEL_NUM_EFF_NIF_NAME[i], g_kNifMan.GetNif(UIMODEL_NUM_EFF_PATH[i]), true, true);	
			lwPoint3 kPoint3 = lua_tinker::call<lwPoint3, int>("GetUIMODEL_NUM_EFF_LOCAL_POS", i);
			std::string const strName = UIMODEL_NUM_EFF_NIF_NAME[i];
			NiNode* pkNode = m_pkWndUIModel_NumEff->GetNIF(strName);
			if(pkNode)
			{
				pkNode->SetTranslate(NiPoint3(kPoint3.GetX(), kPoint3.GetY(), kPoint3.GetZ()));
			}
		}*/
		
		m_pkWndUIModel_NumEff->SetCameraZoomMinMax(-30, 30);
		m_pkWndUIModel_NumEff->CameraZoom(29.0f);
		m_pkWndUIModel_NumEff->SetEnableUpdate(true);
	}
}

PgMissionComplete::~PgMissionComplete()
{
	Destroy();
}

void PgMissionComplete::Destroy()
{
	m_spBackGround = 0; 
	m_spWhiteBillboard = 0;
	m_spAlphaMaterial = 0;
	m_spBGAlphaMaterial = 0;
	m_spCamera = NULL;
	m_vecBoxName.clear();
	m_spNewAlphaProp = 0;
	for (int i = 0; i < REWARD_BOX_COUNT; i++)
	{
		m_kRewardBox[i].Destroy();
	}
}

void PgMissionComplete::Init()
{
	m_pkWnd = XUIMgr.Get(std::wstring(L"FRM_MISSION_COMPLETE"));
	if (m_pkWnd)
	{
		m_pkWnd->Close();
	}
	m_pkWnd = NULL;
	m_pkWndModel = NULL;
	
	SelectBoxNum(-1);
	ItemCount(0);
	CamWorldPos(NiPoint3(0,0,0));
	CompleteState(E_MSN_CPL_NONE);
	CompleteUIState(E_MSN_UI_NONE);
	MyNumber(0);
	
	NowViewItem(-1);
	BossMission(false);
	IsRevcRank(false);
	AniBar_Value(0);
	RankItemNo(0);
	NewRank(0);
	SpaceStop(false);
	MissionEnd(false);
	m_kIncreaseNumberList.clear();
	m_kContUserResultItemCheckList.clear();
	m_kContTimeStamp.clear();

	m_spCamera = 0;
/*
	if (m_spBackGround == 0)
	{
		m_spBackGround = g_kNifMan.GetNif("../Data/4_Item/9_Etc/MissionMapCube/plan.nif");

		if (m_spBackGround)
		{
			m_spBackGround->SetScale(1.1f);
			m_spBackGround->UpdateNodeBound();
			m_spBackGround->UpdateProperties();
			m_spBackGround->UpdateEffects();
			m_spBackGround->Update(0);
		}
	}

	if (m_spWhiteBillboard == 0)
	{
		m_spWhiteBillboard = g_kNifMan.GetNif_DeepCopy("../Data/6_UI/billboard_quad.nif");
		if(m_spWhiteBillboard) 
		{
			NiGeometry	*pkTextGeom = (NiGeometry*)m_spWhiteBillboard->GetObjectByName("Plane");
			if (pkTextGeom)
			{
				NiTexturingProperty	*pkProperty = pkTextGeom->GetPropertyState()->GetTexturing();

				NiSourceTexturePtr spSimpleTextTex = g_kNifMan.GetTexture("../Data/6_UI/whitebg.tga");
				pkProperty->SetBaseTexture(spSimpleTextTex);

				NiPropertyPtr spAlpha = pkTextGeom->GetProperty(NiAlphaProperty::GetType());
				if (spAlpha)
				{
					m_spAlphaMaterial = pkTextGeom->GetPropertyState()->GetMaterial();
					if (m_spAlphaMaterial)
					{
						m_spAlphaMaterial->SetAlpha(0.0f);
					}
				}

				m_spWhiteBillboard->SetScale(10.0f);
				m_spWhiteBillboard->UpdateProperties();
				m_spWhiteBillboard->UpdateEffects();
				m_spWhiteBillboard->Update(0);
			}
		}
	}

	m_vecBoxPos.clear();
	m_vecBoxName.clear();

	std::wstring kTempString = _T("");
	Pg2DString kTemp2DString(XUI::PgFontDef(g_kFontMgr.GetFont(FONT_NAME)), kTempString );
	for(int i = 0; i!=REWARD_BOX_COUNT; ++i)
	{
		m_kRewardBox[i].Init();
		m_kReward[i].Init();
		m_vecBoxName.push_back( kTemp2DString );
	}
	//배경에 알파 프로퍼티가 없어서 하나 만들어놓자
	m_pkNewAlphaProp = NiNew NiAlphaProperty;
	m_pkNewAlphaProp->SetAlphaBlending(true);
	m_pkNewAlphaProp->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
	m_pkNewAlphaProp->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
*/
// 	//////////////////////////////////////////////////////////////////////////
// 	//임시 데이터
 //	m_kReward[0].iNormalExp = 1;
 //	m_kReward[0].iStrangthExp = 10;
 //	m_kReward[0].iCrackExp = 100;
 //
 //	m_kReward[0].iGetExp = 100;
 //	m_kReward[0].iTotalExp = 200;
 //	m_kReward[0].iLevel = 99;
 //	m_kReward[0].wstrName = L"임시입니다1";
 //
 //	m_kReward[0].iItemCount = 4;
 //	m_kReward[0].iRewardItem[0] = 20106002;
 //	m_kReward[0].iRewardItem[1] = 20106014;
 //	m_kReward[0].iRewardItem[2] = 20121001;
 //	m_kReward[0].iRewardItem[3] = 20122001;
// 	//
// 	//////////////////////////////////////////////////////////////////////////
}

void PgMissionComplete::Start()
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
	{
		return;
	}

	PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
	if(pkMyPlayer)
	{
		if(pkMyPlayer->IsDead() == true)
		{
			std::wstring Message = TTW(22010);
			if( !Message.empty() )
			{
				lwAddWarnDataStr(lwWString(Message), 1, false);
				return;
			}
		}
	}
	
	Init();
	CompleteState(E_MSN_CPL_WAIT);
	CompleteUIState(E_MSN_UI_MISSION);
	TimeStamp(g_pkWorld->GetAccumTime());

	m_spCamera = g_pkWorld->GetCameraMan()->GetCamera();

	g_pkWorld->RemoveAllMonster(1);	//일반 몬스터 모두 제거
	g_pkWorld->RemoveAllMonster(2);	//강화 몬스터 모두 제거

	bTimerStart = false;

	lua_tinker::call<void, bool>("LockGlobalHotKey", true);	//글로벌 핫키 고정 고정	

/*	PgWorld::CompleteBGContainer kContainer;
	g_pkWorld->GetCompleteBG(kContainer);

	if (kContainer.empty()) // 일반 미션
	{
		char szName[255] = {0,};
		for (int i = 0; i < 4; ++i)
		{
			_snprintf(szName, 254, "../Data/4_Item/9_Etc/MissionMapCube/plan_0%d.dds", i+1);
			kContainer.insert(std::make_pair(i, std::string(szName)));
		}
	}

	PgWorld::CompleteBGContainerIter it = kContainer.begin();
	int i = 0;
	while(it != kContainer.end())
	{
		char szName[255] = {0,};
		_snprintf(szName, 254, "Plane02:%d", i);
		NiGeometry	*pkTextGeom = (NiGeometry*)m_spBackGround->GetObjectByName(szName);
		if (pkTextGeom)
		{
			NiTexturingProperty	*pkProperty = pkTextGeom->GetPropertyState()->GetTexturing();
			if (pkProperty)
			{
				NiSourceTexturePtr spSimpleTextTex = g_kNifMan.GetTexture((it->second).c_str());
				pkProperty->SetBaseTexture(spSimpleTextTex);
			}
		}
		++i;
		++it;
	}

	m_spBackGround->UpdateNodeBound();
	m_spBackGround->UpdateProperties();
	m_spBackGround->UpdateEffects();
	m_spBackGround->Update(0);
*/
	_PgOutputDebugString("PgMissionComplete::Start\n");
}

bool PgMissionComplete::IsOwner() const
{
	return m_kMission_Result.IsOwner();
}

bool PgMissionComplete::Update(float const fAccumTime, float const fFrameTime)
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
	{
		CompleteState(E_MSN_CPL_NONE);
		return false;
	}
	if (E_MSN_CPL_NONE == CompleteState()) 
	{ 
		XUIMgr.BlockGlobalScript(false);
		return false; 
	}
	if (!g_pkWorld->GetCameraMan() || !g_pkWorld->GetCameraMan()->GetCamera())
	{
		return false;
	}
	
	CamWorldPos(g_pkWorld->GetCameraMan()->GetCamera()->GetWorldTranslate());	//카메라가 이동되므로 매프레임 체크하자.

	NiQuaternion kCurruntRot;
	if (!m_spCamera)
	{
		return false;
	}
	m_spCamera->GetRotate(kCurruntRot);
	NiPoint3 kDir = m_spCamera->GetWorldDirection();
	kDir.Unitize();
	NiMatrix3 kCamRot = m_spCamera->GetRotate();
	float const fFlowTime = fAccumTime - TimeStamp();

	switch(CompleteState())
	{
	case E_MSN_CPL_WAIT:
		{
			if (fFlowTime >= MSN_CPL_WAIT_TIME)
			{
				//XUIMgr.Close(_T("FRM_MISSION_END"));
				g_kBalloonMan2D.BalloonOff(); //말풍선은 모두 끄자

				g_pkWorld->SetShowWorldFocusFilter("../Data/5_Effect/9_Tex/EF_blackBG.tga", 0.0f, 0.5f, BG_FADE_TIME, false);	//화면 페이드
				
				//m_spBackGround->SetTranslate(CamWorldPos());	// 카메라 위치에 갖다놓기
				//여기서 사운드 재생

				//m_spBackGround->SetScale(1.7f);
				//m_spBackGround->SetTranslate(CamWorldPos() + kDir*3.0f);	//카메라 위치에서 약간 앞으로
				//NiMatrix3 mat(m_spCamera->GetWorldDirection(),
				//	m_spCamera->GetWorldUpVector(),
				//	m_spCamera->GetWorldRightVector());

				//m_spBackGround->SetRotate(kCamRot.TransposeTimes(mat));
				
				//m_spBackGround->Update(0);
				//PastCamSpeed(m_kSpeed);					//현재 카메라 속도 저장
				//m_kSpeed = NiQuaternion(0, 0, 0, 0);	//카메라 고정				

				PastCamSpeed(g_kSpeed);					//현재 카메라 속도 저장, 위에 왜 막았는지 모르겠음;;
				lwCloseUI("FRM_CALL_ORDER");

				TimeStamp(fAccumTime);
				lwUIOff();		
				lwActivateUI("QuestOut");
//				lwActivateUI("ChatBar"); // 채팅 출력/입력 창만 표시
//				XUIMgr.Activate(L"ChatOut");
				XUIMgr.BlockGlobalScript(true);
				PgCameraMan* pkCamMan = g_pkWorld->GetCameraMan();
				if (pkCamMan)
				{ 
					pkCamMan->ClearQuake();
					pkCamMan->LockInput(true);
				}

				if( m_kMission_Result.IsRecvGameDefence() )
				{//디펜스 모드라면, 보상화면으로 바로 넘어감.
					UITimeStamp(fAccumTime);
					if(m_kMission_Result.IsLastStage())
					{
						CompleteState(E_MSN_CPL_END);
					}
					else
					{
						CompleteState(E_MSN_CPL_LIGHT);
						CompleteUIState(E_MSN_UI_NEXT_ITEM);
					}
					lwCallUI("FRM_MISSION_STAGE_SUCCESS");
					g_kMissionMgr.MissionStageSubInfo();
				}
				else 
				{
					CompleteState(E_MSN_CPL_FADE);
					lwCallUI("FRM_MISSION_END");
					g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정
					g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정
				}
				break;
			}
		}break;
	case E_MSN_CPL_FADE:
		{
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정
			if (fFlowTime >= BG_FADE_TIME_MISSION)
			{
				g_kSoundMan.StopBgSound();
				PlaySound("MSC_Opening");
			//	SetAlphaByRecursiveProcess(m_spBackGround, 1.0f);
				//PlaySound("MSC_Opening");
			//	m_spBackGround->UpdateProperties();
			//	m_spBackGround->Update(0);
				TimeStamp(fAccumTime);
				//CompleteState(E_MSN_CPL_BG);
				CompleteState(E_MSN_CPL_LIGHT);	 //단계를 건너뛰자
				g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정
				g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정 
				if( m_kMission_Result.IsRecvGameDefence() )
				{
					lwCloseUI("FRM_MISSION_STAGE_SUCCESS");
				}
				else
				{
					lwCloseUI("FRM_MISSION_END");
				}
				m_pkWnd = XUIMgr.Call(std::wstring(L"FRM_MISSION_COMPLETE_NEW"));
				PutMyInfoToUI(m_pkWnd, true);		//초기화
				PutOtherInfoToUI(m_pkWnd, true);	//초기화

				if( (0 < RankItemNo()) && (0 < NewRank()) )
				{
					CallRankResultItem(RankItemNo(), NewRank());
				}
			}
		}break;

	case E_MSN_CPL_LIGHT:
		{
			//m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			//m_spBackGround->Update(0);
			//UpdateBox(fAccumTime, fFrameTime);

			UpdateMyInfo(m_pkWnd, fAccumTime);

			switch(CompleteUIState())
			{
			case E_MSN_UI_MISSION:
				{
					PutMyInfoToUI(m_pkWnd);		// 내정보를 UI에 설정
					//PutRankInfoToUI(m_pkWnd);
					CompleteUIState(E_MSN_UI_SCORE0);	//아직은 하는 일 없음
					UITimeStamp(fAccumTime);
					m_pkWndModel = m_pkWnd->GetControl(std::wstring(_T("FRM_RANK_UIMODEL")));
					if (m_pkWndModel)
					{
						m_pkWndModel->Visible(false);
					}
					return true;
				}break;
			case E_MSN_UI_SCORE0:
				{
					
					if ( (fAccumTime - UITimeStamp() >= 0.5f) || (true == SpaceStop()) )
					{
						MissionTotalUI(m_pkWnd, E_MSN_UI_SCORE0);
						CompleteUIState(E_MSN_UI_SCORE1);
						UITimeStamp(fAccumTime);
					}
					return true;
				}break;
			case E_MSN_UI_SCORE1:
				{
					if ( (fAccumTime - UITimeStamp() >= 0.5f) || (true == SpaceStop()) )
					{
						MissionTotalUI(m_pkWnd, E_MSN_UI_SCORE1);
						CompleteUIState(E_MSN_UI_SCORE2);
						UITimeStamp(fAccumTime);
					}
					return true;
				}break;
			case E_MSN_UI_SCORE2:
				{
					if ( (fAccumTime - UITimeStamp() >= 0.5f) || (true == SpaceStop()) )
					{
						MissionTotalUI(m_pkWnd, E_MSN_UI_SCORE2);
						CompleteUIState(E_MSN_UI_USER1);
						UITimeStamp(fAccumTime);
					}
					return true;
				}break;
			case E_MSN_UI_USER1:
				{
					if ( (fAccumTime - UITimeStamp() >= 0.5f) || (true == SpaceStop()) )
					{
						MissionPartyScoreUI(m_pkWnd, 0);
						CompleteUIState(E_MSN_UI_USER2);
						UITimeStamp(fAccumTime);
					}
					return true;
				}break;
			case E_MSN_UI_USER2:
				{
					if ( (fAccumTime - UITimeStamp() >= 0.5f) || (true == SpaceStop()) )
					{					
						MissionPartyScoreUI(m_pkWnd, 1);
						CompleteUIState(E_MSN_UI_USER3);
						UITimeStamp(fAccumTime);
					}
					return true;
				}break;
			case E_MSN_UI_USER3:
				{
					if ( (fAccumTime - UITimeStamp() >= 0.5f) || (true == SpaceStop()) )
					{
						MissionPartyScoreUI(m_pkWnd, 2);
						CompleteUIState(E_MSN_UI_USER4);
						UITimeStamp(fAccumTime);
					}
					return true;
				}break;
			case E_MSN_UI_USER4:
				{
					if ( (fAccumTime - UITimeStamp() >= 0.5f) || (true == SpaceStop()))
					{
						MissionPartyScoreUI(m_pkWnd, 3);
						CompleteUIState(E_MSN_UI_ANIBAR_TOTAL);
						UITimeStamp(fAccumTime);
						AniBar_Value(0);
					}
					return true;
				}break;
			case E_MSN_UI_ANIBAR_TOTAL:
				{
					if ( (fAccumTime - UITimeStamp() >= 2.0f) || (true == SpaceStop()) )
					{
						if (!m_pkWnd) { return false; }

						XUI::CXUI_Wnd *pkMyForm = m_pkWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
						if (!pkMyForm) { return false; }
						int iTotalScore = m_kMission_Result.GetPoint(7);
						// 개인능력
						XUI::CXUI_AniBar* pAniBarTotal = dynamic_cast<XUI::CXUI_AniBar*>(pkMyForm->GetControl(std::wstring(L"BAR_XP")));
						if( !pAniBarTotal )
						{
							return false;
						}

						if( 0 == AniBar_Value() )
						{
							PlaySound("Mission_Clear04");
						}

						if( AniBar_Value() <= iTotalScore )
						{
							iTotalScore = AniBar_Value();
							AddAniBar_Value();
						}
						else
						{
							AniBar_Value(0);
							CompleteUIState(E_MSN_UI_ANIBAR_SENSE);
							UITimeStamp(fAccumTime);
						}
						pAniBarTotal->Now(iTotalScore);
						pAniBarTotal->Max(MISSION_TOTALSCORE_MAX);
					}
					return true;
				}break;
			case E_MSN_UI_ANIBAR_SENSE:
				{
					if ( (fAccumTime - UITimeStamp() >= 0.8f) || (true == SpaceStop()) )
					{
						if (!m_pkWnd) { return false; }

						XUI::CXUI_Wnd *pkMyForm = m_pkWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
						if (!pkMyForm) { return false; }
						// Sense
						XUI::CXUI_AniBar* pAniBarSense = dynamic_cast<XUI::CXUI_AniBar*>(pkMyForm->GetControl(std::wstring(L"ANI_MY_SENSE")));
						if( !pAniBarSense )
						{
							return false;
						}
						int kSenseScore = 0;
						int kAbilityScore = 0;
						GetSenseAbilityTotalScore(kSenseScore, kAbilityScore);
						int kValue = kSenseScore;

						TCHAR szTemp[512] = {0 ,};
						std::wstring kTTWForm = TTW(401049);
						_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(kValue));
						std::wstring kLog = szTemp;	

						pAniBarSense->Visible(true);
						pAniBarSense->Text(kLog);

						if( 0 == AniBar_Value() )
						{
							PlaySound("Mission_Clear05");
						}

						if( AniBar_Value() <= kValue )
						{
							kValue = AniBar_Value();
							AddAniBar_Value();
						}
						else
						{
							AniBar_Value(0);
							CompleteUIState(E_MSN_UI_ANIBAR_ABILITY);
							UITimeStamp(fAccumTime);
						}
						pAniBarSense->Now(kValue);
						pAniBarSense->Max(100);
					}
					return true;
				}break;
			case E_MSN_UI_ANIBAR_ABILITY:
				{
					if ( (fAccumTime - UITimeStamp() >= 0.8f) || (true == SpaceStop()) )
					{
						if (!m_pkWnd) { return false; }

						XUI::CXUI_Wnd *pkMyForm = m_pkWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
						if (!pkMyForm) { return false; }
						// Ability
						XUI::CXUI_AniBar* pAniBarAbility = dynamic_cast<XUI::CXUI_AniBar*>(pkMyForm->GetControl(std::wstring(L"ANI_MY_ABILITY")));
						if( !pAniBarAbility )
						{
							return false;
						}
						int kSenseScore = 0;
						int kAbilityScore = 0;
						GetSenseAbilityTotalScore(kSenseScore, kAbilityScore);
						int kValue = kAbilityScore;

						TCHAR szTemp[512] = {0 ,};
						std::wstring kTTWForm = TTW(401049);
						_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(kValue));
						std::wstring kLog = szTemp;	

						pAniBarAbility->Visible(true);
						pAniBarAbility->Text(kLog);

						if( 0 == AniBar_Value() )
						{
							PlaySound("Mission_Clear05");
						}

						if( AniBar_Value() <= kValue )
						{
							kValue = AniBar_Value();
							AddAniBar_Value();
						}
						else
						{
							CompleteUIState(E_MSN_UI_TIME);
							UITimeStamp(fAccumTime);
						}
						pAniBarAbility->Now(kValue);
						pAniBarAbility->Max(100);
					}
					return true;
				}break;
			case E_MSN_UI_TIME:
				{
					if ( (fAccumTime - UITimeStamp() >= 0.8f) || (true == SpaceStop()) )
					{
						if (!m_pkWnd) { return false; }

						XUI::CXUI_Wnd *pkMyForm = m_pkWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
						if (!pkMyForm) { return false; }
						// Time
						PlaySound("Mission_Clear06");

						if( true == SpaceStop() )
						{
							XUI::CXUI_Wnd *pkMyTime = pkMyForm->GetControl(std::wstring(L"FRM_TIME_NUM"));
							if (!pkMyTime) { return false; }
							PutTimerText(pkMyTime, m_kMission_Result.ClearTime());
						}
						else
						{
							AddNewNum(std::wstring(_T("FRM_TIME_NUM")), static_cast<float>(m_kMission_Result.ClearTime()));
						}
						CompleteUIState(E_MSN_UI_NUM);
						UITimeStamp(fAccumTime);
					}
					return true;
				}break;
			case E_MSN_UI_NUM:
				{
					if ( (fAccumTime - UITimeStamp() >= 1.0f) || (true == SpaceStop()) )
					{
						for (int i = 0; i < MBONUSPOINT_MAX; ++i)
						{
							int const iPoint = m_kMission_Result.GetBonusPoint(i);
							if( iPoint != 0 )
							{
								/*if( i == MBONUSPOINT_PARTY )
								{
									if( iPoint <= 10 ) 
									{
										continue;
									}
								}*/
								DWORD dwColor = 0xFF5af8fe;
								std::wstring strImgPath = _T("../Data/6_ui/mission/msMsOn.tga");
								BM::vstring	kVstr;
								kVstr = _T("");

								/*if( i == MBONUSPOINT_PARTY )
								{
									kVstr = _T("*");

									wchar_t szValue[255] = {0,};									
									swprintf_s(szValue, 254, L"%.1f", iPoint/10.0f);
									kVstr += szValue;
								}
								else
								{
									kVstr = _T("+");
									kVstr += iPoint;
								}*/
								PutRankBonusInfoUI(i, m_pkWnd, dwColor, strImgPath, (std::wstring const&)kVstr, true);
							}
						}

						CompleteUIState(E_MSN_UI_BONUSEXP);	//아직은 하는 일 없음
						UITimeStamp(fAccumTime);
					}
					return true;
				}break;
			/*case E_MSN_UI_EXP:
				{
					SMissionPlayerInfo_Client MyInfo;

					if(m_kMission_Result.GetInfoFromUserInfoAt(MyNumber(), MyInfo))
					{
						int iTotal = 0;
						iTotal = m_kMission_Result.GetPoint(MPOINT_BONUSEXP);
						iTotal = __max(0, iTotal);
						AddNewNum(std::wstring(_T("FRM_EXP_NUM")), __max(0, iTotal), 0.1f);
						if (m_pkWndUIModel_NumEff)
						{
							m_pkWndUIModel_NumEff->ResetNIFAnimation(UIMODEL_NUM_EFF_NIF_NAME);
							m_pkWndUIModel_NumEff->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_NUM_EFF_POS);
						}
					}
					CompleteUIState(E_MSN_UI_EXP_PLUS);	//아직은 하는 일 없음
					UITimeStamp(fAccumTime);
					return true;
					
				} break;*/
			/*case E_MSN_UI_EXP_PLUS:
				{
					if (fAccumTime - UITimeStamp() >= 1.0f)
					{
						int iTotal = 0;
						for (int i = 0; i < MPOINT_MAX; ++i)
						{
							iTotal += m_kMission_Result.GetPoint(i) + m_kMission_Result.GetBonusPoint(i);
						}
						iTotal = __max(0, iTotal);
						BM::vstring kPlus(_T("+"));
						kPlus += iTotal;

						//PutMyInfoToUI(m_pkWnd, _T("FRM_PLUS_EXP_NUM"), kPlus);
						CompleteUIState(E_MSN_UI_BONUSEXP);
						UITimeStamp(fAccumTime);
					}
					return true;
				}   break;*/
			/*case E_MSN_UI_EXP_SUM:
				{
					if (fAccumTime - UITimeStamp() >= 1.0f)
					{
						SMissionPlayerInfo_Client MyInfo;

						if(m_kMission_Result.GetInfoFromUserInfoAt(MyNumber(), MyInfo))
						{
							//PutMyInfoToUI(m_pkWnd, _T("FRM_EXP_NUM"), BM::vstring(MyInfo.iAccExp));
							PutMyInfoToUI(m_pkWnd, _T("FRM_EXP_NUM"), BM::vstring(m_kMission_Result.GetPoint(MPOINT_BONUSEXP)));							
							CompleteUIState(E_MSN_UI_BONUSEXP);
							UITimeStamp(fAccumTime);
							if (m_pkWndUIModel_NumEff)
							{
								m_pkWndUIModel_NumEff->ResetNIFAnimation(UIMODEL_NUM_EFF_NIF_NAME);
							}
						}
					}

					if (m_pkWndUIModel_NumEff)
					{
						m_pkWndUIModel_NumEff->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_NUM_EFF_POS);
					}

					return false;
				}break;*/
			case E_MSN_UI_BONUSEXP:
				{
					if ( (fAccumTime - UITimeStamp() >= 1.0f) || (true == SpaceStop()) )
					{
						if (!m_pkWnd) { return false; }
						XUI::CXUI_Wnd *pkMyForm = m_pkWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
						if (!pkMyForm) { return false; }
						// 보너스경험치
						XUI::CXUI_Wnd *pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_MISSION_BONUSEXP_NUM"));
						if( pkWnd )
						{
							BM::vstring	kVstr;
							kVstr = _T("");
							kVstr += m_kMission_Result.GetPoint(MPOINT_BONUSEXP);
							pkWnd->Text(kVstr);

							PlaySound("Mission_Clear03");
						}

						CompleteUIState(E_MSN_UI_RANK);
						UITimeStamp(fAccumTime);
					}
					return true;
				}break;
			case E_MSN_UI_RANK:
				{
					if (m_pkWndUIModel_NumEff)
					{
						m_pkWndUIModel_NumEff->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_NUM_EFF_POS);
					}

					if (fAccumTime - UITimeStamp() <= 1.0f)
					{
						return false;
					}

					if (m_pkWndUIModel)
					{
						NiNode* pRankNif = m_pkWndUIModel->GetNIF(UIMODEL_NIF_NAME);
						if(pRankNif)
						{
							NiNode* pkPlane02 = NiDynamicCast(NiNode,pRankNif->GetObjectByName("Plane02"));
							NiNode* pkPlane04 = NiDynamicCast(NiNode,pRankNif->GetObjectByName("Plane04"));
							BM::vstring kVstr(_T("../Data/5_Effect/9_Tex/ef_ms_rank_"));

							SMissionPlayerInfo_Client MyInfo;
							int iMyRank = 5;
							if(m_kMission_Result.GetInfoFromUserInfoAt(MyNumber(), MyInfo))
							{
								iMyRank = (int)(MyInfo.kRank-1);
							}

							char szPath[255] = {0,};
							_snprintf(szPath, 254, "../Data/5_Effect/9_Tex/ef_ms_rank_%d.dds", iMyRank);
							NiSourceTexturePtr spSimpleTextTex = g_kNifMan.GetTexture(szPath);

							if (pkPlane02)
							{
								NiGeometry* pkTextGeom2 = NiDynamicCast(NiGeometry,pkPlane02->GetAt(0));
								if (pkTextGeom2)
								{
									NiPropertyStatePtr pkPropertyState = pkTextGeom2->GetPropertyState();
									if (pkPropertyState)
									{
										NiTexturingProperty	*pkProperty = pkPropertyState->GetTexturing();
										if (pkProperty)
										{
											pkProperty->SetBaseTexture(spSimpleTextTex);
										}

									}

								}
							}

							if (pkPlane04)
							{
								NiGeometry* pkTextGeom4 = NiDynamicCast(NiGeometry,pkPlane04->GetAt(0));
								if (pkTextGeom4)
								{
									NiPropertyStatePtr pkPropertyState = pkTextGeom4->GetPropertyState();
									if (pkPropertyState)
									{
										NiTexturingProperty	*pkProperty = pkPropertyState->GetTexturing();
										if (pkProperty)
										{
											pkProperty->SetBaseTexture(spSimpleTextTex);
										}

									}

								}
							}

							m_pkWndUIModel->SetEnableUpdate(true);
							m_pkWndUIModel->Update(fAccumTime, fFrameTime);
							m_pkWndUIModel->ResetNIFAnimation(UIMODEL_NIF_NAME);
							POINT2 kAddPos = (XUIMgr.GetResolutionSize()-POINT2(EXV_DEFAULT_SCREEN_WIDTH, EXV_DEFAULT_SCREEN_HEIGHT));
							kAddPos.x/=2; kAddPos.y/=2;
							m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_POS + kAddPos);
							m_pkWndModel = m_pkWnd->GetControl(std::wstring(_T("FRM_RANK_UIMODEL")));
							if (m_pkWndModel)
							{
								m_pkWndModel->Visible(true);
							}
						}
					}

					PlaySound("Mission_Rank");
					CompleteUIState(E_MSN_UI_NEXT_ITEM);
					UITimeStamp(fAccumTime);					
					return true;
				}break;
			case E_MSN_UI_NEXT_ITEM:
				{
					UpdateMyInfo(m_pkWnd, fAccumTime);

					if (m_pkWndUIModel)
					{
						POINT2 kAddPos = (XUIMgr.GetResolutionSize()-POINT2(EXV_DEFAULT_SCREEN_WIDTH, EXV_DEFAULT_SCREEN_HEIGHT));
						kAddPos.x/=2; kAddPos.y/=2;
						m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_POS+kAddPos);
					}

					float fDelayTime = 5.0f;
					if( m_kMission_Result.IsRecvGameDefence() )
					{
						fDelayTime = 3.0f;
					}

					if (fAccumTime - UITimeStamp() >= fDelayTime)
					{
						if( m_kMission_Result.IsRecvGameDefence() )
						{
							if( false == m_kMission_Result.IsResultStage() )
							{
								//디펜스 모드인데 보상없는 스테이지임. 아이템 보상 화면 패스
								CompleteState(E_MSN_CPL_UI_CLOSE);
								return true;
							}

							if( m_kMission_Result.IsRecvDefence() )
							{
								lwCallUI("FRM_DEFENCE_RESULT_EXP");
								int const iBonusExp = std::min<int>( INT_MAX, m_kMission_Result.BonusExp() );
								lua_tinker::call< void, int >("SetDefenceResultExpText", iBonusExp );
							}
						}
						else
						{
							g_pkWorld->SetShowWorldFocusFilter(false, "../Data/5_Effect/9_Tex/EF_blackBG.tga", 0.8f);
							lwCallUI("FRM_POINT_RESULT");
							lua_tinker::call< void, int >("SetResultPointText", m_kMission_Result.GetMissionPoint() );
						}
						SpaceStop(false);
						lwCloseUI("FRM_MISSION_COMPLETE_NEW");
						if(false==m_kMission_Result.IsConstellationMission())
						{
							m_pkWnd = XUIMgr.Call(std::wstring(L"FRM_MY_INFO_NEXT"));
							if( !m_pkWnd )
							{
								return false;
							}
							PutMyInfoToUINext(m_pkWnd, true);		//초기화
							PutRankInfoToUI(m_pkWnd);
							PutOtherInfoToUI(m_pkWnd, true);

							if( m_kMission_Result.GetAddItemResultNo() )
							{
								wchar_t const* pkItemName = NULL;
								if( GetItemName(m_kMission_Result.GetAddItemResultNo(), pkItemName) )
								{
									BM::vstring vStr(TTW(401172));
									vStr.Replace(L"#NAME#", pkItemName);
									XUI::CXUI_Wnd *pkText = XUIMgr.Call(L"FRM_ADD_RESUILTITEM");
									//pkText = pkText ? pkText->GetControl(L"FRM_TEXT") : NULL;
									pkText ? pkText->Text(vStr) : 0;
								}
							}

							CompleteUIState(E_MSN_UI_OTHER);
							UITimeStamp(fAccumTime);
						}
						else
						{
							CompleteState(E_MSN_CPL_REWARD_WAIT);
							CompleteUIState(E_MSN_UI_NONE);
							UITimeStamp(fAccumTime);

							m_kContTimeStamp[E_TS_CONSTELLATION_RESULT_TIME] = fAccumTime;
						}
					}
					return true;
				}break;
			case E_MSN_UI_OTHER:
				{
					if (fAccumTime - UITimeStamp() >= 0.8f)
					{
						SetRoluetteStopRewardItemDraw(m_pkWnd, BM::GUID::NullData());
						//PutOtherInfoToUI(m_pkWnd);	// 다른사람 정보를 UI에 설정	
						VEC_GUID::iterator iter = m_kContUserResultItemCheckList.begin();
						while( m_kContUserResultItemCheckList.end() != iter )
						{
							SetRoluetteStopRewardItemDraw(m_pkWnd, (*iter));
							++iter;
						}
						CompleteUIState(E_MSN_UI_END);
						UITimeStamp(fAccumTime);
					}
					return true;
				}break;
			case E_MSN_UI_END:
				{					
					TimeStamp(fAccumTime);
					CompleteState(E_MSN_CPL_REWARD);					
					CompleteUIState(E_MSN_UI_NONE);
				}break;
			}

			/*if (fFlowTime >= BG_WHITE_FADE_TIME)	
			{
				m_spAlphaMaterial->SetAlpha(0.0f);
			}
			else
			{
				float const fAlpha = sin(NI_HALF_PI*(fFlowTime+BOX_OPEN_TIME)/(BOX_OPEN_TIME+BG_WHITE_FADE_TIME));
				m_spAlphaMaterial->SetAlpha(fAlpha);
			}
			if (m_spWhiteBillboard)
			{
				m_spWhiteBillboard->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH*0.2f);
				m_spWhiteBillboard->UpdateProperties();
				m_spWhiteBillboard->Update(0);
			}*/
			
		}break;
	case E_MSN_CPL_REWARD:	//UI움직이기
		{
			//m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			//m_spBackGround->Update(0);

			UpdateMyInfo(m_pkWnd, fAccumTime);

			if (m_pkWndUIModel)
			{
				m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_POS);
			}

			if (fFlowTime < UI_MOVE_TIME)
			{
/*				if (m_pkWnd)
				{
					float const fRate = fFlowTime/UI_MOVE_TIME;
					XUI::CXUI_Wnd *pkForm_Left = m_pkWnd->GetControl(std::wstring(L"SFRM_MY_INFO"));
					if (pkForm_Left)
					{
						pkForm_Left->Location((int)((fRate-1.0f)*pkForm_Left->Size().x), pkForm_Left->Location().y);
					}
					XUI::CXUI_Wnd *pkForm_Right = m_pkWnd->GetControl(std::wstring(L"SFRM_OTHER_INFO"));
					if (pkForm_Right)
					{
						pkForm_Right->Location(EXV_DEFAULT_SCREEN_WIDTH - (int)(fRate*pkForm_Right->Size().x), pkForm_Right->Location().y);
					}
					//if (!BossMission())
					{
						XUI::CXUI_Wnd *pkForm_Title = m_pkWnd->GetControl(std::wstring(L"FRM_RESULT_TITLE"));
						if (pkForm_Title)
						{
							pkForm_Title->Location(pkForm_Title->Location().x, pkForm_Title->Location().y+(int)(fFrameTime/UI_MOVE_TIME*UI_TITLE_MOVE_LENGTH));
						}
					}
				}*/
			}
			else
			{
				/*if (m_pkWnd)
				{
					XUI::CXUI_Wnd *pkForm_Left = m_pkWnd->GetControl(std::wstring(L"SFRM_MY_INFO"));
					if (pkForm_Left)
					{
						pkForm_Left->Location(0, pkForm_Left->Location().y);
					}
					XUI::CXUI_Wnd *pkForm_Right = m_pkWnd->GetControl(std::wstring(L"SFRM_OTHER_INFO"));
					if (pkForm_Right)
					{
						pkForm_Right->Location(EXV_DEFAULT_SCREEN_WIDTH - pkForm_Right->Size().x, pkForm_Right->Location().y);
					}
					//if (!BossMission())
					{
						XUI::CXUI_Wnd *pkForm_Title = m_pkWnd->GetControl(std::wstring(L"FRM_RESULT_TITLE"));
						if (pkForm_Title)
						{
							pkForm_Title->Location(pkForm_Title->Location().x, UI_TITLE_MOVE_LENGTH-pkForm_Title->Size().y);
						}
					}
				}*/
				//lua_tinker::call<bool, float, float, int, float, int>("QuakeCamera", 0.4f, 1.5f,0,0,0);	//화면 흔들림
				PlaySound("MSC_UI_Comming");

				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_REWARD_DISPLAY);
				//m_pkWndModel = XUIMgr.Call(std::wstring(L"FRM_UIMODEL"));
				// UI Model
			}

		}break;

	case E_MSN_CPL_REWARD_DISPLAY:	// 화면에 표시
		{
			//m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			//m_spBackGround->Update(0);

			UpdateMyInfo(m_pkWnd, fAccumTime);
			if (m_pkWndUIModel)
			{
				m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_POS);
			}

			XUI::CXUI_Wnd *pkForm_Left = m_pkWnd;
			if ( !pkForm_Left )
			{
				break;
			}

/*			if (!m_pkWndUIModel)	//없으면 한번 갖고와 보고
			{
				m_pkWndUIModel = g_kUIScene.GetRenderModel(g_kPilotMan.GetPlayerActor()->GetID().c_str());
				if (m_pkWndUIModel) //있으면 승리포즈 취해 주자
				{
					m_pkWndUIModel->SetTargetAnimation(2000035);
					_PgOutputDebugString("m_pkWndUIModel->SetTargetAnimation(2000035)\n");
				}
			}*/

			float const fItemTime = UI_ICON_APPEAR_TIME/(float)MAX_REWARD_ITEM_COUNT;
			SMissionPlayerInfo_Client MyInfo;
			if(!m_kMission_Result.GetInfoFromUserInfoAt(MyNumber(), MyInfo))
			{
				break;
			}
			size_t iItemNum = MyInfo.kContItem.size();

			if( MAX_REWARD_ITEM_COUNT < iItemNum )
			{
				// 최대 4개 까지만 된다.
				iItemNum = MAX_REWARD_ITEM_COUNT;
			}
			float const fPastTime = fItemTime*iItemNum;
			if( (fFlowTime < fPastTime) && (false == SpaceStop()) )	//4개라서 4.0. 아이템 카운트는 다 같으니까 아무거나 갖고 온것. 매직넘버 좋지 않아;;
			{
				int const iRealTime = (int)(fFlowTime/fItemTime);
				if (iRealTime != NowViewItem() && iRealTime < (int)iItemNum)	//단위 시간당 하나의 아이템 그림을 화면에 표시
				{
					NowViewItem(iRealTime);

					BM::vstring kImgName(L"SFRM_BONUS");
					kImgName+=iRealTime;
					//XUI::CXUI_Wnd *pkIcon = pkForm_Left->GetControl(kImgName)->GetControl(std::wstring(_T("IMG_BONUS")));
					XUI::CXUI_Wnd *pkForm_ImgName = pkForm_Left->GetControl(kImgName);
					if( pkForm_ImgName )
					{
						PgFormRouletteWnd *pkIcon = dynamic_cast<PgFormRouletteWnd *>( pkForm_ImgName->GetControl(std::wstring(_T("IMG_BONUS"))) );
						if( pkIcon )
						{
							PgBase_Item & kItem = MyInfo.kContItem[iRealTime];
							if (kItem.ItemNo() == 0)
							{ 
								kItem.ItemNo(20400217);
							}
							pkIcon->SetRoulette(MyInfo.iResultBagNo, kItem, iItemNum);
							//BM::Stream kCustomData;
							//kItem.WriteToPacket(kCustomData);
							//pkIcon->SetCustomData(kCustomData.Data());
							if(pkIcon->Scale() <= 1.0f) 
							{
								pkIcon->VScale(2.0f);
							}
								
							//lwDrawMissionRewardItem(lwUIWnd(pkIcon, pkIcon != NULL));
							char const * pkSoundName = lua_tinker::call<char const*>("GetRewardItemOpenSoundName");
							PlaySound(pkSoundName);
							//m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);//계속 업데이트 시켜주면 카메라 흔들어도 못느낌
							//lua_tinker::call<bool, float, float, int, float, int>("QuakeCamera", 0.4f, 1,0,0,0);	//화면 흔들림
							_PgOutputDebugString("ItemNo : %d", kItem.ItemNo());
						}
					}				

				}
			}
			else
			{
				int iCount = 0;
				bool bDisplayView = false;

				for(int i=0; i<MAX_REWARD_ITEM_COUNT; ++i)
				{
					BM::vstring kImgName(L"SFRM_BONUS");
					kImgName+=i;
					XUI::CXUI_Wnd *pkForm_ImgName = pkForm_Left->GetControl(kImgName);
					if( pkForm_ImgName )
					{
						PgFormRouletteWnd *pkIcon = dynamic_cast<PgFormRouletteWnd *>( pkForm_ImgName->GetControl(std::wstring(_T("IMG_BONUS"))) );
						if( pkIcon )
						{
							pkIcon->SetStopKeyState();
						}
						if( pkIcon->GetCompleteState() )
						{
							++iCount;
							if( iCount >= iItemNum )
							{
								bDisplayView = true;
							}
						}
						if( iItemNum == 0 )
						{
							bDisplayView = true;
						}
					}
				}

				PgPlayer* pkMyPlayer = g_kPilotMan.GetPlayerUnit();
				if( pkMyPlayer )
				{
					if( pkMyPlayer->IsUse() )
					{
						if( pkMyPlayer->GetExpRate() == 0 )
						{
							bDisplayView = true;
						}
					}
				}

				if( fFlowTime > UI_CLOSE_BUTTON_TIME )
				{
					bDisplayView = true;
				}

				if( bDisplayView )
				{
					if( m_pkWnd )
					{
						PutRankInfoToUI(m_pkWnd);
					}

					BM::Stream kPacket( PT_C_M_REQ_MISSION_ROULETTE_STOP );
					NETWORK_SEND(kPacket)

					// 화면에 아이콘들 다 표시되면 종료버튼 보여주기
					if( m_kMission_Result.IsRecvGameDefence() && !m_kMission_Result.IsLastStage() )
					{//디펜스 모드고 마지막 스테이지가 아닐경우, "확인"으로 표시함.
						m_pkWnd->GetControl(std::wstring(L"SFRM_EXIT_BG"))->GetControl(std::wstring(L"BTN_EXIT"))->Text(::TTW(400975));
					}
					else
					{
						m_pkWnd->GetControl(std::wstring(L"SFRM_EXIT_BG"))->GetControl(std::wstring(L"BTN_EXIT"))->Text(::TTW(400208));
					}
					m_pkWnd->GetControl(std::wstring(L"SFRM_EXIT_BG"))->Visible(true);
					TimeStamp(fAccumTime);
					CompleteState(E_MSN_CPL_REWARD_WAIT);
					WORD const swStand = m_kMission_Result.Standing();

					SMissionPlayerInfo_Client MyInfo;
					m_kMission_Result.GetInfoFromUserInfoAt(MyNumber(), MyInfo);
					if(MyInfo.kRank != MRANK_SSS)
					{ //SSS 이상만 코멘트 창이 뜬다.
					}
					else if(BM::GUID::NullData() != m_kMission_Result.GetMemoGuid())
					{
						XUI::CXUI_Wnd *pkWnd = XUIMgr.Call(_T("FRM_MISSION_MEMO"), true);	//모달로 띄우자
						XUI::CXUI_Wnd *pkRank10 = pkWnd->GetControl(_T("IMG_RANK_10"));
						
						if (pkRank10)
						{// 10의 자리 이미지 출력 여부
							pkRank10->Visible((bool)(swStand/10));
						}
						
						XUI::CXUI_Wnd *pkRank1 = pkWnd->GetControl(_T("IMG_RANK_1"));
						if (pkRank1)
						{// 1의 자리 이미지 index 구함
							SUVInfo kUVInfo= pkRank1->UVInfo();
							kUVInfo.Index = __max(1,(swStand+1)%11);
							pkRank1->UVInfo(kUVInfo);
							pkRank1->SetInvalidate();
						}
						
						XUI::CXUI_Wnd *pkRankTailWord = pkWnd->GetControl(_T("FRM_RANK1"));
						if(pkRankTailWord)
						{							
							if(true != pkRank10->Visible())
							{// 두자리 랭크가 아니고

								SUVInfo const kUVInfo= pkRank1->UVInfo();
								switch( kUVInfo.Index )
								{
								case RANK1_IMG_INDEX:
									{// 1위 일때
										pkRankTailWord->Text(TTW(RANK1_TAIL_TEXTID));
									}break;
								case RANK2_IMG_INDEX:
									{// 2위 일때
										pkRankTailWord->Text(TTW(RANK2_TAIL_TEXTID));
									}break;
								case RANK3_IMG_INDEX:
									{// 3위 일때
										pkRankTailWord->Text(TTW(RANK3_TAIL_TEXTID));
									}break;
								default:
									{// 4위~9위 일때
										pkRankTailWord->Text(TTW(AFTERRANK3_TAIL_TEXTID));
									}break;
								}
							}
							else
							{// 10위 이후 일때
								pkRankTailWord->Text(TTW(AFTERRANK3_TAIL_TEXTID));
							}
						}
					}
					else
					{
						if (USHRT_MAX == swStand && NULL == XUIMgr.Get(std::wstring(_T("SFRM_MSG_COMMON"))))
						{
							//레벨 제한
							BM::vstring kVstr(TTW(401102));
							kVstr += _T("\n");
							kVstr += TTW(400326);
							int const iMinLim = CalcMinLevelLimit(m_kMission_Result.MissionNo(), m_kMission_Result.MissionLevel());
							int const iMaxLim = CalcMaxLevelLimit(m_kMission_Result.MissionNo(), m_kMission_Result.MissionLevel());
							kVstr += _T(" ");
							kVstr += TTW(224);
							kVstr += _T(" ");
							kVstr += iMinLim;
							kVstr += _T(" ~ ");
							kVstr += TTW(224);
							kVstr += _T(" ");
							kVstr += iMaxLim;
							
							lua_tinker::call<void, char const*, bool, int>("CommonMsgBox", MB(kVstr), true, -10);
						}
					}
				}
			}
		}break;
	case E_MSN_CPL_REWARD_WAIT: 
		{
			UpdateMyInfo(m_pkWnd, fAccumTime);
			if (m_pkWndUIModel)
			{
				m_pkWndUIModel->RenderFrame(NiRenderer::GetRenderer(), UIMODEL_POS);
			}
			//m_spBackGround->Update(0);

			//if (m_pkWndUIModel->IsAnimationDone())
			//{
			//	m_pkWndUIModel->SetTargetAnimation(2000035);
			//}

			if (fFlowTime > UI_CLOSE_TIME_2)
			{
//				m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
//				m_spBackGround->Update(0);
				//m_pkWnd->GetControl(std::wstring(L"SFRM_EXIT_BG"))->Visible(false);
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_UI_CLOSE);
			}
		}break;
	case E_MSN_CPL_UI_CLOSE:
		{
			bool bResultStage = false;
			g_kSpeed = PastCamSpeed();
			lua_tinker::call<void, bool>("LockGlobalHotKey", false);	//글로벌 핫키 고정 고정
			g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정 해제 
			g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정 해제 

			MissionEnd(true);

			if( false == m_kMission_Result.IsRecvDefence() )
			{
				g_pkWorld->SetShowWorldFocusFilter(false, "../Data/5_Effect/9_Tex/EF_blackBG.tga", 0.8f);
			}
			else if( true == m_kMission_Result.IsResultStage() )
			{
				if( (m_kMission_Result.IsRecvDefence7() || m_kMission_Result.IsRecvDefence8()) && false==m_kMission_Result.IsLastStage() )
				{
					g_kMissionMgr.StratagyMissionStageResult();
				}
				else
				{
					bResultStage = true;
				}
			}
			if (m_pkWnd)
			{
				m_pkWnd->Close();

				if( true == bResultStage )
				{
					BM::Stream kClosePacket( PT_C_C_NFY_MISSION_CLOSE );
					NETWORK_SEND(kClosePacket)
				}
			}

			lwUIOn();
			XUIMgr.BlockGlobalScript(false);

			PgCameraMan* pkCamMan = g_pkWorld->GetCameraMan();
			if (pkCamMan)
			{
				pkCamMan->LockInput(false);
			}

			if (m_pkWndModel)
			{
				m_pkWndModel->Close();
				m_pkWndModel = NULL;
			}

			if (m_pkWndUIModel)
			{
				m_pkWndUIModel->SetEnableUpdate(false);
			}
			
			if( (MT_EVENT_MISSION == m_kMission_Result.GetMissionType()) || (MT_EVENT_HIDDEN == m_kMission_Result.GetMissionType()) || m_kMission_Result.IsRecvGameDefence() )
			{
				if( true == bResultStage )
				{
					BM::Stream kConfirmPacket( PT_C_M_REQ_MISSION_RESTART );
					NETWORK_SEND(kConfirmPacket);
					CompleteState(E_MSN_CPL_SELECT_NEXT);
				}
				else
				{
					CompleteState(E_MSN_CPL_END);
				}
			}
			else
			{
				CallRetryUI();			
				CompleteState(E_MSN_CPL_WAIT_RETRY);
			}

			g_kHelpSystem.ActivateByCondition(std::string(HELP_CONDITION_MISSIONCOMPLETE), 0); // 미션 클리어

			lua_tinker::call<void, bool>("MissionCompletePartyStateChange", false); // 파티 가입 신청 거절 상태 해제.

			MemberCount(0);

			XUIMgr.Activate(_T("ChatOut"));
			XUIMgr.Activate(_T("SysChatOut"));
			g_kChatMgrClient.FlushDelayLog(CIE_Mission);

		}break;
	case E_MSN_CPL_WAIT_RETRY:
		{
			if (fFlowTime > WAIT_RETRY_TIME)
			{
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_END);
			}
		}break;
	case E_MSN_CPL_SELECT_NEXT:
		{
			TimeStamp(fAccumTime);
			//CompleteState(E_MSN_CPL_NONE);
			if( m_kMission_Result.IsRecvDefence() && m_kMission_Result.IsConstellationMission() )
			{
				if(NULL == XUIMgr.Get(L"FRM_CONSTELLATION_RESULT"))
				{
					CompleteState(E_MSN_CPL_END);
				}
			}
		}break;
	case E_MSN_CPL_END:
		{
			lua_tinker::call<void, bool>("LockGlobalHotKey", false);	//글로벌 핫키 고정 고정
			TimeStamp(fAccumTime);
			CompleteState(E_MSN_CPL_NONE);

			if( m_kMission_Result.IsRecvGameDefence() )
			{
				if( m_kMission_Result.IsLastStage() )
				{//마지막 스테이지라면 클리어 연출 시작
					//if( false == m_kMission_Result.IsConstellationMission() )
					{
						lua_tinker::call<void>("End_Defence_Stage");
					}
				}
				else
				{
					//lua_tinker::call<void>("SetCamerModeDefault");
					g_kSoundMan.PlayBgSound(0);
					if(m_kMission_Result.IsRecvDefence())
					{
						lua_tinker::call<void>("SetCamerModeDefault");
						// 마지막 스테이지가 아니라면 방향 고르기 UI가 나타남.
						CallSelectDirectionUI();
					}
					else
					{
						int iSec = 60;
						if(m_kMission_Result.IsConstellationMission() && m_kMission_Result.IsResultStage())
						{
							iSec += UI_CLOSE_TIME_2 - (fAccumTime-m_kContTimeStamp[E_TS_CONSTELLATION_RESULT_TIME]) - 1;
						}

						g_kMissionMgr.StratagyMissionStageEnd(iSec);
					}
				}

				Init();
				lwCloseUI("FRM_DEFENCE_RESULT_EXP");
				return true;
			}
			else
			{
				Init();
				Net_RecentMapMove();

				MemberCount(0);
				m_kMission_Result.Clear();
			}
		}
		break;
	}

	return true;
}

void PgMissionComplete::AddAniBar_Value()
{
	AniBar_Value(AniBar_Value() + 10);
}

void PgMissionComplete::SetRoluetteStopRewardItemDraw(BM::GUID const &rkGuid)
{
	m_kContUserResultItemCheckList.push_back(rkGuid);
	SetRoluetteStopRewardItemDraw(m_pkWnd, rkGuid);
}

void PgMissionComplete::GadaCoinItemView(BM::Stream &kPacket)
{
	PgBase_Item kItem;
	kItem.ReadFromPacket( kPacket );

	XUI::CXUI_Wnd	*m_pkWnd = XUIMgr.Get(std::wstring(L"FRM_MY_INFO_NEXT"));
	if( !m_pkWnd )
	{
		return;
	}

	XUI::CXUI_Wnd	*m_pkGadaWnd;
	m_pkGadaWnd = m_pkWnd->GetControl(std::wstring(_T("SFRM_GADACOIN")));
	if( !m_pkGadaWnd )
	{
		return;
	}

	XUI::CXUI_Image* const pkIcon = dynamic_cast<XUI::CXUI_Image*>(m_pkGadaWnd->GetControl(_T("IMG_GADACOIN")));
	if( pkIcon )
	{
		CXUI_Button* pkButton = dynamic_cast<CXUI_Button*>(m_pkWnd->GetControl(_T("BTN_QS")));
		if( !pkButton )
		{
			return;
		}

		pkButton->Visible(false);

		BM::Stream kCustomData;
		kItem.WriteToPacket(kCustomData);
		pkIcon->SetCustomData(kCustomData.Data());

		PgUISpriteObject *pkSprite = g_kUIScene.GetIconTexture(kItem.ItemNo());

		m_pkGadaWnd->Visible(0<kItem.ItemNo());

		if (!pkSprite) 
		{ 
			pkIcon->DefaultImgTexture(NULL);
			pkIcon->SetInvalidate();
			return; 
		}

		
		PgUIUVSpriteObject *pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);

		if(!pkUVSprite) { return; }

		pkIcon->DefaultImgTexture(pkUVSprite);
		SUVInfo &rkUV = pkUVSprite->GetUVInfo();
		pkIcon->UVInfo(rkUV);
		POINT2 kPoint(40*rkUV.U, 40*rkUV.V);//이미지 원본 사이즈 복구
		pkIcon->ImgSize(kPoint);
		pkIcon->VScale(2.0f);
		pkIcon->SetInvalidate();

		XUI::CXUI_Wnd *pkMyForm = m_pkWnd->GetControl(_T("FRM_COIN_TEXT1"));
		if( pkMyForm )
		{
			TCHAR szTemp[512] = {0 ,};
			std::wstring kTTWForm = TTW(401054);
			int iCoin = abs(m_kMission_Result.GetGadaCoin());
			int iNeedCoin = abs(m_kMission_Result.GetGadaCoinNeedCount());
			int iViewCoin = (iCoin-iNeedCoin);
			if( 0 > iViewCoin )
			{
				iViewCoin = 0;
			}
			_sntprintf_s(szTemp, 512, kTTWForm.c_str(), iViewCoin);
			std::wstring kLog = szTemp;	

			pkMyForm->Text(kLog);
		}
	}
}

bool PgMissionComplete::PutRankBonusInfoUI(int idx, XUI::CXUI_Wnd* pkParentWnd, DWORD const dwColor, std::wstring const& ImgPath, std::wstring const& Value, bool const IsVisible)
{
	BM::vstring	kVstr;

	XUI::CXUI_Wnd*	pkForm = pkParentWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
	if( !pkForm )
	{
		return false;
	}

	kVstr = _T("FRM_MISSION_BONUS_");
	kVstr+= (idx+1);
	XUI::CXUI_Wnd* pkMission = pkForm->GetControl((std::wstring const&)kVstr);
	if( !pkMission )
	{
		return false;
	}
	pkMission->FontColor(dwColor);
	pkMission->DefaultImgName(ImgPath);

	kVstr = _T("FRM_MISSION_BONUS_POINT");
	kVstr+= (idx+1);
	XUI::CXUI_Wnd* pkMsPoint = pkForm->GetControl((std::wstring const&)kVstr);
	if( !pkMsPoint )
	{
		return false;
	}
	//pkMsPoint->Text(Value);
	pkMsPoint->Visible(IsVisible);

	return true;
}
void PgMissionComplete::UpdateMyInfo(XUI::CXUI_Wnd* pkParentWnd, float const fAccumTime)
{
	if( m_kMission_Result.IsRecvDefence7() || m_kMission_Result.IsRecvDefence8() )
	{
		return;
	}

	BM::vstring kVstr;
	if (!pkParentWnd)
	{
		return;
	}

	XUI::CXUI_Wnd *pkMyForm = pkParentWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
	if (!pkMyForm) { return; }

	XUI::CXUI_Wnd *pkWnd = NULL;

	IncreaseNumberMap::iterator num_iter = m_kIncreaseNumberList.begin();
	while (m_kIncreaseNumberList.end() != num_iter)
	{
		BM::vstring	kVstr;
		SIncreaseNumber rkNum;

		if((*num_iter).second.Update(fAccumTime))
		{
			rkNum = (*num_iter).second;
			num_iter = m_kIncreaseNumberList.erase(num_iter);
		}
		else
		{
			rkNum = (*num_iter).second;
			++num_iter;
		}

		kVstr = rkNum.m_fDisplayNum;
		pkWnd = pkMyForm->GetControl(rkNum.m_wstrName);
		if (pkWnd)
		{
			if (rkNum.m_wstrName == L"FRM_TIME_NUM")	//타이머 예외처리
			{
				PutTimerText(pkWnd, (int)(rkNum.m_fDisplayNum));//GetIncleaseNumber(std::wstring(L"FRM_TIME_NUM"))) );
			}
			else
			{
				BM::vstring kVstr((int)(rkNum.m_fDisplayNum));

				pkWnd->Text(kVstr);
			}
		}
	}

	/*XUI::CXUI_Wnd *pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_MON_NUM"));
	DisplayNum(std::wstring(L"FRM_MON_NUM"), pkWnd);

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_COMBO_NUM"));
	DisplayNum(std::wstring(L"FRM_COMBO_NUM"), pkWnd);

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_ATT_NUM"));
	DisplayNum(std::wstring(L"FRM_ATT_NUM"), pkWnd);

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_DMG_NUM"));
	DisplayNum(std::wstring(L"FRM_DMG_NUM"), pkWnd);

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_EXP_NUM"));
	DisplayNum(std::wstring(L"FRM_EXP_NUM"), pkWnd);

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_TIME_NUM"));
	PutTimerText(pkWnd, (int)(GetIncleaseNumber(std::wstring(L"FRM_TIME_NUM"))) );*/
}

float PgMissionComplete::GetIncleaseNumber(std::wstring const &strName)
{
	IncreaseNumberMap::const_iterator num_iter = m_kIncreaseNumberList.find(strName);
	if (m_kIncreaseNumberList.end() != num_iter)
	{
		return (*num_iter).second.m_fDisplayNum;
	}

	return 0.0f;
}

void PgMissionComplete::DisplayNum(std::wstring const &kstrName, XUI::CXUI_Wnd *pkWnd)
{
	if (!pkWnd)
	{
		return;
	}

	float const fNum = GetIncleaseNumber(kstrName);

	BM::vstring kVstr((int)(fNum));

	pkWnd->Text(kVstr);
}

bool PgMissionComplete::AddNewNum(std::wstring &strName, float fOrigin, float fDelayTime, float fMainTime, bool bInc)
{
	if(!g_pkWorld)
	{
		return false;
	}

	SIncreaseNumber kNew;
	kNew.Set(strName, fOrigin, g_pkWorld->GetAccumTime(), fMainTime, bInc);
	kNew.m_fDelayTime = fDelayTime;

	std::pair<IncreaseNumberMap::iterator, bool> ib = m_kIncreaseNumberList.insert(std::make_pair(kNew.m_wstrName, kNew));
	return ib.second;
}

XUI::CXUI_Wnd* PgMissionComplete::CallRetryUI()
{
	// 죽었을때 불리면 무조건 죽은 UI를 띄워야 한다.
	PgPilot* pkMyPilot = g_kPilotMan.GetPlayerPilot();
	if ( pkMyPilot )
	{
		if ( pkMyPilot->GetAbil(AT_HP) <= 0 )
		{// 죽어있으면 다시 UI를 띄운다
			lwActivateUI("FRM_DEAD_MISSION");
			return NULL;
		}
	}

	if ( CompleteState() != E_MSN_CPL_WAIT_RETRY && CompleteState() != E_MSN_CPL_UI_CLOSE && CompleteState() != E_MSN_CPL_SELECT_NEXT)
	{
		return NULL;
	}

	XUI::CXUI_Wnd* pkMsg = NULL;
	//if (m_kMission_Result.IsOwner())	//방장이면	
	bool bIsPartyMaster = false;
	if( pkMyPilot )
	{
		bIsPartyMaster = (g_kParty.MasterGuid() == pkMyPilot->GetGuid());
	}
	if( bIsPartyMaster || m_kMission_Result.IsOwner() )
	{
		pkMsg = XUIMgr.Call(std::wstring(_T("SFRM_RETRY_MISSION")));
	}
	else
	{
		BM::Stream kConfirmPacket( PT_C_M_REQ_MISSION_RESTART );
		NETWORK_SEND(kConfirmPacket);
	}

	return pkMsg;
}

void PgMissionComplete::Draw(PgRenderer *pkRenderer, float fFrameTime)
{
	if (E_MSN_CPL_NONE == CompleteState() || !pkRenderer || !g_pkWorld) { return; }

	pkRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_ZBUFFER);

	pkRenderer->SetCameraData(g_pkWorld->GetCameraMan()->GetCamera());

	
	NiDX9Renderer *pDX9Renderer = NiDX9Renderer::GetRenderer();

	switch(CompleteState())
	{
	case E_MSN_CPL_BG:
		{
//			pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
		}break;	
	case E_MSN_CPL_DROP:
		{
//			pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
//			DrawBox(pkRenderer, fFrameTime);
		}break;
	case E_MSN_CPL_SELECT:	//선택 대기중
		{
/*			pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
			DrawBox(pkRenderer, fFrameTime);

			// 이름 Draw
			NiPoint3	spPos;
			for ( int i=0; i!=REWARD_BOX_COUNT; ++i )
			{
				spPos = m_kRewardBox[i].GetTranslate();
				spPos.z -= BOX_INTERVAL;
				m_vecBoxName[i].Draw( m_spCamera, spPos, NiColorA(1.0f, 1.0f, 0.0f, 1.0f ) );
			}*/
		}break;
	case E_MSN_CPL_MOVE_START:
	case E_MSN_CPL_MOVE:
		{
//			pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
//			DrawBox(pkRenderer, fFrameTime);
		}break;
	case E_MSN_CPL_OPEN:
	case E_MSN_CPL_LIGHT:
		{
//			g_kUIScene.AddToDrawListRenderModel(UIMODEL_NAME);
		}break;
	
	case E_MSN_CPL_REWARD_DISPLAY:
	case E_MSN_CPL_REWARD_WAIT:
		{
//			g_kUIScene.AddToDrawListRenderModel(UIMODEL_NAME);
		}
//		pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
		/*if (m_pkWndModel)
		{
			PgActor *pkObj = g_kPilotMan.GetPlayerActor();
			if (pkObj)
			{
				g_kUIScene.RenderModel(pkObj->GetID().c_str(), m_pkWndModel->Size(), m_pkWndModel->TotalLocation(), pkObj->GetActorManager(), pkObj->GetUIModelUpdate());
			}
		}*/
		break;

	case E_MSN_CPL_REWARD:
	case E_MSN_CPL_END:
		{
//			g_kUIScene.AddToDrawListRenderModel(UIMODEL_NAME);
		}
//		pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
		break;
	default:
		break;
	}
	pkRenderer->EndUsingRenderTargetGroup();
}

void PgMissionComplete::CloseMissionTimer(BM::Stream &kPacket)
{
	int iScriptNo = 0;

	kPacket.Pop(iScriptNo);

	if ( !g_pkWorld )
	{
		return;
	}

	XUIMgr.Close(std::wstring(_T("FRM_MISSION_SCENARIO_TIMER")));
	XUIMgr.Close(std::wstring(_T("FRM_TIMELIMIT")));	
	XUIMgr.Close(std::wstring(_T("FRM_REST_MON_NUM")));	

	if( iScriptNo )
	{
		g_kEventScriptSystem.ActivateEvent(iScriptNo);
	}
}

void PgMissionComplete::CloseBonusStageMissionTimer(BM::Stream &kPacket)
{
	XUIMgr.Close(std::wstring(_T("FRM_BONUS_TIMELIMIT")));

	lwActivateUI("FRM_BONUS_MISSION_FAILED");	
}

void PgMissionComplete::CloseMissionScore(BM::Stream &kPacket)
{
	bool bUIClose = false;

	kPacket.Pop(bUIClose);

	if( bUIClose )
	{
		XUIMgr.Close(std::wstring(_T("FRM_MISSION_SCORE")));
		XUIMgr.Close(std::wstring(_T("FRM_MISSION_EXP")));		
		XUIMgr.Close(std::wstring(_T("FRM_TIMELIMIT")));
		XUIMgr.Close(std::wstring(_T("FRM_REST_MON_NUM")));	
	}
}

void PgMissionComplete::ReadFromPacket(BM::Stream &kPacket)
{
	if ( !g_pkWorld /*|| !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT))*/ )
	{
		return;
	}

	XUIMgr.Close(std::wstring(_T("FRM_MISSION_TIMER")));
	XUIMgr.Close(std::wstring(_T("FRM_MISSION_SCENARIO_TIMER")));
	m_kMission_Result.ReadFromPacket(kPacket);
	if (m_kMission_Result.IsRecvRank())
	{
		IsRevcRank(true);
		CallRankUI();
	}
	else if (m_kMission_Result.IsRecvResult() || m_kMission_Result.IsRecvGameDefence() )
	{
		XUIMgr.Close(_T("FRM_DEAD_MISSION"));
		size_t const iNum = m_kMission_Result.GetUserCount();

		MemberCount(iNum);

		Start();

		VEC_MS_PLAYER_INFO_CLIENT::const_iterator c_it = m_kMission_Result.GetFirstElementFromUserInfo();

		int iCoint = 0;
		for ( size_t i = 0; i < iNum; ++i, ++iCoint, ++c_it)
		{
			const SMissionPlayerInfo_Client &kInfo = (*c_it);
			if ( g_kPilotMan.IsMyPlayer( kInfo.kCharGuid) )
			{
				MyNumber(i);
			}
		}

		_PgOutputDebugString("PgMissionComplete::ReadFromPacket %d\n",iNum);
	}
}

void PgMissionComplete::ReadFromPacketSelect(BM::Stream &kPacket)
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
	{
		return;
	}
	XUI::CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_NAME);

	int iNum = -1;
	kPacket.Pop(iNum);
	if ( -1 <= iNum && REWARD_BOX_COUNT > iNum)
	{
		BM::GUID kCharGuid;
		kPacket.Pop( kCharGuid );
		if ( g_kPilotMan.IsMyPlayer( kCharGuid ) )
		{
			SelectBoxNum( iNum );
			m_kRewardBox[iNum].BoxState(PgRewardBox::E_REWORD_BOX_SELECT);
			PlaySound("MSC_Click");
		}

		for( int i=0; i!=REWARD_BOX_COUNT; ++i )
		{
			if ( m_kReward[i].kCharGuid == kCharGuid )
			{
				if(pFont)
				{
					m_vecBoxName[iNum].SetText( XUI::PgFontDef(pFont),m_kReward[i].wstrName );
				}
				break;
			}
		}
	}
	_PgOutputDebugString("PgMissionComplete::ReadFromPacketSelect\n");
}
void PgMissionComplete::ReadFromPacketOpen(BM::Stream &kPacket)
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
	{
		return;
	}
	CompleteState(E_MSN_CPL_MOVE_START);
	PlaySound("MSC_White_Out");
	_PgOutputDebugString("PgMissionComplete::ReadFromPacketOpen\n");
}

bool PgMissionComplete::UpdateBox(float fAccumTime, float fFrameTime)
{
	for(int i = 0; i < REWARD_BOX_COUNT; ++i)
	{
		m_kRewardBox[i].Update(fAccumTime, fFrameTime);
	}
	return true;
}
void PgMissionComplete::DrawBox(PgRenderer *pkRenderer, float fFrameTime)
{
	for(int i = 0; i < REWARD_BOX_COUNT; ++i)	
	{
		m_kRewardBox[i].Draw(pkRenderer, fFrameTime);
	}
}

void PgMissionComplete::Pickup(int const iValue)
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
	{
		return;
	}
	if (E_MSN_CPL_SELECT != CompleteState())
	{
		return;
	}
	if ( SelectBoxNum() > -1 && SelectBoxNum() <= REWARD_BOX_COUNT )
	{
		return;
	}
	NiPoint3 kOrgPt;
	NiPoint3 kRayDir;
	POINT2 ptXUIPos = XUIMgr.MousePos();

	if (!m_spCamera)
	{
		return;
	}
	m_spCamera->WindowPointToRay(ptXUIPos.x, ptXUIPos.y, kOrgPt, kRayDir);
	for(int i = 0; i < REWARD_BOX_COUNT; ++i )
	{	
		// 박스가 선택가능한 상태일때만..
		if ( PgRewardBox::E_REWORD_BOX_SELECTING & m_kRewardBox[i].BoxState() )
		{
			if ( m_kRewardBox[i].Picked(kOrgPt, kRayDir) )
			{
				if ( 1001 == iValue )
				{
					BM::Stream kPacket(PT_C_M_NFY_SELECTED_BOX, i);
					NETWORK_SEND(kPacket)	//서버로 패킷 보내자
					_PgOutputDebugString("PT_C_M_NFY_SELECTED_BOX %d \n", i);
				}
				else
				{
					m_kRewardBox[i].BoxState(PgRewardBox::E_REWORD_BOX_OVER);
				}
			}
			else
			{
				m_kRewardBox[i].BoxState(PgRewardBox::E_REWORD_BOX_IDLE);
			}
		}
	}
}

bool PgMissionComplete::MissionTotalUI(XUI::CXUI_Wnd* pkParentWnd, E_MSN_UI_STATE eType)
{
	if (!pkParentWnd) { return false; }

	XUI::CXUI_Wnd *pkMyForm = pkParentWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
	if (!pkMyForm) { return false; }
	XUI::CXUI_Wnd *pkWnd = NULL;

	int index = 0;
	int const iTotalScore = m_kMission_Result.GetPoint(7);

	switch( eType )
	{
	case E_MSN_UI_SCORE0:
		{
			pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_MISSION_TOTAL1_MIN0"));
			if( pkWnd )
			{
				if( 0 == iTotalScore/100 )
				{
					index = 1;
				}
				else
				{
					index = (iTotalScore/100)+1;
				}
			}
		}break;
	case E_MSN_UI_SCORE1:
		{
			pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_MISSION_TOTAL1_MIN1"));
			if( pkWnd )
			{
				if( 0 == (iTotalScore%100)/10 )
				{
					index = 1;
				}
				else
				{
					index = ((iTotalScore%100)/10)+1;
				}
			}
		}break;
	case E_MSN_UI_SCORE2:
		{
			pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_MISSION_TOTAL1_MIN2"));
			if( pkWnd )
			{
				if( 0 == (iTotalScore%10) )
				{
					index = 1;
				}
				else
				{
					index = (iTotalScore%10)+1;
				}
			}
		}break;
	default:
		{
			return false;
		}break;
	}
	pkWnd->Visible(true);
	SUVInfo kInfo = pkWnd->UVInfo();
	kInfo.Index = index;
	pkWnd->UVInfo(kInfo);
	pkWnd->SetInvalidate();

	PlaySound("Mission_Clear02");

	return true;
}
bool PgMissionComplete::MissionPartyScoreUI(XUI::CXUI_Wnd* pkParentWnd, int const iNumber)
{
	size_t kTempCount = 0;
	XUI::CXUI_Wnd *pkOtherForm = NULL;
	SMissionPlayerInfo_Client OhterInfo;	
	BM::vstring kVstr;

	int iTotalValue = 0;
	int iMaxValue = 0;
	int iPersentValue = 0;

	for (int i = 0; i < PARTY_MEMBER_COUNT; ++i)
	{
		OhterInfo.Clear();
		if( true == m_kMission_Result.GetInfoFromUserInfoAt(i, OhterInfo) )
		{
			iTotalValue += OhterInfo.iScore;			
		}
	}

	for (int i = 0; i < PARTY_MEMBER_COUNT; ++i)
	{
		OhterInfo.Clear();
		if( true == m_kMission_Result.GetInfoFromUserInfoAt(i, OhterInfo) )
		{
			iMaxValue = __max(iMaxValue, i);
			if( 0!= iTotalValue )
			{
				iPersentValue += ((OhterInfo.iScore * 100) / iTotalValue);
			}
		}
	}

	if( 100 > iPersentValue )
	{
		iPersentValue = (100 - iPersentValue);
	}
	else
	{
		iPersentValue = 0;
	}


	kVstr = L"FRM_MISSION_MEMBER_SCORE_TEXT";
	kVstr += (iNumber+1);

	XUI::CXUI_Wnd *pkMyForm = pkParentWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
	if (!pkMyForm) { return false; }
	pkOtherForm = pkMyForm->GetControl(kVstr);
	OhterInfo.Clear();
	if(pkOtherForm)
	{
		int kValue = 0;
		TCHAR szTemp[512] = {0 ,};
		std::wstring kTTWForm;
		std::wstring kLog;

		OhterInfo.Clear();
		if( true == m_kMission_Result.GetInfoFromUserInfoAt(iNumber, OhterInfo) )
		{
			if( 0 != iTotalValue )
			{
				kValue = (OhterInfo.iScore * 100) / iTotalValue;

				if( iNumber == iMaxValue )
				{
					kValue += iPersentValue;
				}
			}
			kTTWForm = TTW(401049);
			_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(kValue));
			kLog = szTemp;

			PlaySound("Mission_Clear03");
		}
		else
		{
			kLog = _T("");
		}		

		pkOtherForm->Text(kLog);
	}
	return true;
}

void PgMissionComplete::GetSenseAbilityTotalScore(int &iSense, int &iAbility)
{
	int kSenseTotalScore = 0;
	int const iSenseAdd = m_kMission_Result.GetSense();
	if( (iSenseAdd) >= MISSION_SCORE_PERSENT )
	{
		int iTotalAdd = (iSenseAdd / MISSION_SCORE_PERSENT);

		if( iTotalAdd >= 0 )
		{
			kSenseTotalScore = (iTotalAdd * (ms_TotalScore_TotalUpPersent*MISSION_SCORE_CALC));
			if( 0 >= kSenseTotalScore )
			{
				kSenseTotalScore = 0;
			}
		}
	}
	int kAbilityTotalScore = 0;
	int const iAbilityAdd = m_kMission_Result.GetAbility();
	if( (iAbilityAdd) >= MISSION_SCORE_PERSENT )
	{
		int iTotalAdd = (iAbilityAdd / MISSION_SCORE_PERSENT);

		if( iTotalAdd >= 0 )
		{
			kAbilityTotalScore = (iTotalAdd * (ms_TotalScore_TotalUpPersent*MISSION_SCORE_CALC));
			if( 0 >= kAbilityTotalScore )
			{
				kAbilityTotalScore = 0;
			}
		}
	}

	int iTotalValue = (kSenseTotalScore + kAbilityTotalScore);
	if( 0 != iTotalValue )
	{
		iSense = ((kSenseTotalScore * 100) / iTotalValue);
		iAbility = ((kAbilityTotalScore * 100) / iTotalValue);		

		if(100 > (iSense + iAbility))
		{
			if(iSense > iAbility)
			{
				++iSense;
			}
			else if(iSense < iAbility)
			{
				++iAbility;
			}
		}
	}
	else
	{
		iSense = 0;
		iAbility = 0;
	}
}

bool PgMissionComplete::PutMyInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit)
{
	if (!pkParentWnd) { return false; }

	XUI::CXUI_Wnd *pkMyForm = pkParentWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
	if (!pkMyForm) { return false; }
	std::wstring kstr;
	BM::vstring kVstr;
	SMissionPlayerInfo_Client MyInfo;
	XUI::CXUI_Wnd *pkWnd = NULL;

	if (!bInit)
	{
		if(!m_kMission_Result.GetInfoFromUserInfoAt(MyNumber(), MyInfo))
		{
			return false;
		}
	}
	else	// 초기화
	{
		for(int i=0; i<3; ++i)
		{
			kVstr = L"FRM_MISSION_TOTAL1_MIN";
			kVstr += i;
			pkWnd = pkMyForm->GetControl(kVstr);
			if( pkWnd )
			{
				pkWnd->Visible(false);
				SUVInfo kInfo = pkWnd->UVInfo();
				kInfo.Index = 1;
				pkWnd->UVInfo(kInfo);
				pkWnd->SetInvalidate();
			}
		}
	}

	// 총점


	// 기여도
	if( bInit )
	{
		/*kVstr = _T("");
		kVstr += MyInfo.wstrName;
		pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_MISSION_MEMBER_NAME1_TEXT"));
		pkWnd->Text(kVstr);*/

		size_t kTempCount = 0;
		XUI::CXUI_Wnd *pkOtherForm = NULL;
		XUI::CXUI_Wnd *pkCursorForm = NULL;
		SMissionPlayerInfo_Client OhterInfo;	

		for (int i = 0; i < PARTY_MEMBER_COUNT; ++i)
		{
			bool bScoreView = false;
			kTempCount = i + 1;
			kVstr = _T("");
			kVstr = L"FRM_MISSION_MEMBER_NAME_TEXT";
			kVstr += static_cast<int>(kTempCount);

			pkOtherForm = pkMyForm->GetControl(kVstr);
			OhterInfo.Clear();
			if (pkOtherForm)
			{
				kVstr = L"FRM_MISSION_MEMBER_CURSOR";
				kVstr += i;
				pkCursorForm = pkMyForm->GetControl(kVstr);
				if( true == m_kMission_Result.GetInfoFromUserInfoAt(i, OhterInfo) )
				{
					pkOtherForm->Text(OhterInfo.wstrName);

					if( pkCursorForm )
					{
						pkCursorForm->Visible((MyNumber() == i));
					}
					else
					{
						pkCursorForm->Visible(false);
					}
					bScoreView = true;
				}
				else
				{
					if( pkCursorForm )
					{
						pkCursorForm->Visible(false);
					}
					kVstr = TTW(401040);
					pkOtherForm->Text(kVstr);
					bScoreView = false;
				}
			}

			kTempCount = i + 1;
			kVstr = _T("");
			kVstr = L"FRM_MISSION_MEMBER_SCORE_TEXT";
			kVstr += static_cast<int>(kTempCount);

			pkOtherForm = pkMyForm->GetControl(kVstr);
			OhterInfo.Clear();
			if(pkOtherForm)
			{
				int const kValue = 0;
				TCHAR szTemp[512] = {0 ,};
				std::wstring kTTWForm = TTW(401049);
				_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(kValue));
				std::wstring kLog = szTemp;	

				if( false == bScoreView )
				{
					kLog = _T("");
				}
				pkOtherForm->Text(kLog);
			}
		}
	}

	// 총점 그래프
	if( bInit )
	{
		XUI::CXUI_AniBar* pAniBarTotal = dynamic_cast<XUI::CXUI_AniBar*>(pkMyForm->GetControl(std::wstring(L"BAR_XP")));
		if( pAniBarTotal )
		{
			pAniBarTotal->Now(0);
			pAniBarTotal->Max(MISSION_TOTALSCORE_MAX);		
		}

		// 개인능력
		// Sense
		XUI::CXUI_AniBar* pAniBarSense = dynamic_cast<XUI::CXUI_AniBar*>(pkMyForm->GetControl(std::wstring(L"ANI_MY_SENSE")));
		if( pAniBarSense )
		{
			int const kValue = 0;

			TCHAR szTemp[512] = {0 ,};
			std::wstring kTTWForm = TTW(401049);
			_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(kValue));
			std::wstring kLog = szTemp;	

			pAniBarSense->Visible(false);
			pAniBarSense->Text(kLog);
			pAniBarSense->Now(kValue);
			pAniBarSense->Max(100);
		}
		// Ability
		XUI::CXUI_AniBar* pAniBarAbility = dynamic_cast<XUI::CXUI_AniBar*>(pkMyForm->GetControl(std::wstring(L"ANI_MY_ABILITY")));
		if( pAniBarAbility )
		{
			int const kValue = 0;

			TCHAR szTemp[512] = {0 ,};
			std::wstring kTTWForm = TTW(401049);
			_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(kValue));
			std::wstring kLog = szTemp;	

			pAniBarAbility->Visible(false);
			pAniBarAbility->Text(kLog);
			pAniBarAbility->Now(kValue);
			pAniBarAbility->Max(100);
		}
	}

	// Hit
	if( bInit )
	{
		TCHAR szTemp[512] = {0 ,};
		std::wstring kTTWForm = TTW(401050);
		_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(m_kMission_Result.GetPoint(3)));
		std::wstring kLog = szTemp;		
		
		pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_HIT_NUM"));
		pkWnd->Text(kLog);
	}

	// Time
	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_TIME_NUM"));
	int iTime = 0;
	/*if (!bInit)
	{
		AddNewNum(std::wstring(_T("FRM_TIME_NUM")), static_cast<float>(m_kMission_Result.ClearTime()));
	}*/
	if( pkWnd )
	{
		PutTimerText(pkWnd, iTime);
	}

	// 보너스경험치	
	if(bInit)
	{
		pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_MISSION_BONUSEXP_NUM"));
		if( pkWnd )
		{
			std::wstring kLog = _T("0");
			pkWnd->Text(kLog);
		}
	}

	for (int i = 0; i < MBONUSPOINT_MAX; ++i)
	{
		DWORD dwColor = 0xFF5e5e5e;
		std::wstring strImgPath = _T("");
		std::wstring strValue = L"";

		PutRankBonusInfoUI(i, pkParentWnd, dwColor, strImgPath, strValue, false);
	}

	return true;
}

void PgMissionComplete::OnSelectGadaCoinBox()
{
	if( m_kMission_Result.GetGadaCoinNeedCount() > m_kMission_Result.GetGadaCoin() )
	{
		lwAddWarnDataTT( 550005 );
		return;
	}

	BM::Stream kPacket(PT_C_M_REQ_MISSION_GADACOIN_ITEM);	
	NETWORK_SEND(kPacket);
}

int PgMissionComplete::GetMissionType()
{
	return static_cast<int>(m_kMission_Result.GetMissionType());
}

bool PgMissionComplete::PutMyInfoToUINext(XUI::CXUI_Wnd* pkParentWnd, bool bInit)
{
	if (!pkParentWnd) { return false; }
	
	std::wstring kstr;
	BM::vstring kVstr;
	SMissionPlayerInfo_Client MyInfo;

	if (!bInit)
	{
		if(!m_kMission_Result.GetInfoFromUserInfoAt(MyNumber(), MyInfo))
		{
			return false;
		}
	}
	else	// 초기화
	{
		XUI::CXUI_Wnd *pkIcon = NULL;
		for (int i = 0; i<MAX_REWARD_ITEM_COUNT; ++i )
		{
			BM::vstring kImgName(L"SFRM_BONUS");
			kImgName+=i;
			pkIcon = pkParentWnd->GetControl(kImgName)->GetControl(std::wstring(_T("IMG_BONUS")));
			if (pkIcon)
			{
				int const kNull = 0;
				pkIcon->SetCustomData(&kNull, sizeof(kNull));
				pkIcon->DefaultImgTexture(NULL);
			}
		}
		pkParentWnd->SetInvalidate();

		CXUI_Button* pkButton = dynamic_cast<CXUI_Button*>(pkParentWnd->GetControl(_T("BTN_QS")));
		if( pkButton )
		{
			pkButton->Visible(true);

			if( MT_MISSION == m_kMission_Result.GetMissionType() )
			{
				pkButton->Disable(false);
			}
			else
			{
				pkButton->Disable(true);
			}
		}

		XUI::CXUI_Wnd *m_pkGadaWnd = pkParentWnd->GetControl(std::wstring(_T("SFRM_GADACOIN")));
		if( m_pkGadaWnd )
		{
			m_pkGadaWnd->Visible(false);

			XUI::CXUI_Image* const pkIcon = dynamic_cast<XUI::CXUI_Image*>(m_pkGadaWnd->GetControl(_T("IMG_GADACOIN")));
			if( pkIcon )
			{
				PgBase_Item kItem;
				BM::Stream kCustomData;
				kItem.WriteToPacket(kCustomData);
				pkIcon->SetCustomData(kCustomData.Data());
			}
		}

		XUI::CXUI_Wnd *pkMyForm0 = pkParentWnd->GetControl(_T("FRM_COIN_TEXT0"));
		if( pkMyForm0 )
		{
			TCHAR szTemp[512] = {0 ,};
			std::wstring kTTWForm = TTW(401054);

			if( MT_MISSION == m_kMission_Result.GetMissionType() )
			{
				_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(m_kMission_Result.GetGadaCoinNeedCount()));
			}
			else
			{
				_sntprintf_s(szTemp, 512, kTTWForm.c_str(), 0);
			}
			
			std::wstring kLog = szTemp;	

			pkMyForm0->Text(kLog);
		}

		XUI::CXUI_Wnd *pkMyForm = pkParentWnd->GetControl(_T("FRM_COIN_TEXT1"));
		if( pkMyForm )
		{
			TCHAR szTemp[512] = {0 ,};
			std::wstring kTTWForm = TTW(401054);
			_sntprintf_s(szTemp, 512, kTTWForm.c_str(), abs(m_kMission_Result.GetGadaCoin()));
			std::wstring kLog = szTemp;	

			pkMyForm->Text(kLog);
		}

		XUI::CXUI_Wnd *pkMyForm2 = pkParentWnd->GetControl(_T("FRM_COIN_TEXT2"));
		if( pkMyForm2 )
		{
			if( MT_MISSION == m_kMission_Result.GetMissionType() )
			{
				pkMyForm2->Visible(true);
			}
			else
			{
				pkMyForm2->Visible(false);
			}
		}

		XUI::CXUI_Wnd *m_pkExitWnd = pkParentWnd->GetControl(std::wstring(_T("SFRM_EXIT_BG")));
		if( m_pkExitWnd )
		{
			m_pkExitWnd->Visible(false);
		}
	}

	return true;
}

/*bool PgMissionComplete::PutMyInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit)
{
	if (!pkParentWnd) { return false; }

	XUI::CXUI_Wnd *pkMyForm = pkParentWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
	if (!pkMyForm) { return false; }
	std::wstring kstr;
	BM::vstring kVstr;
	SMissionPlayerInfo_Client MyInfo;

	if (!bInit)
	{
		if(!m_kMission_Result.GetInfoFromUserInfoAt(MyNumber(), MyInfo))
		{
			return false;
		}
	}
	else	// 초기화
	{
		XUI::CXUI_Wnd *pkIcon = NULL;
		for (int i = 0; i<MAX_REWARD_ITEM_COUNT; ++i )
		{
			BM::vstring kImgName(L"SFRM_BONUS");
			kImgName+=i;
			pkIcon = pkMyForm->GetControl(kImgName)->GetControl(std::wstring(_T("IMG_BONUS")));
			if (pkIcon)
			{
				int const kNull = 0;
				pkIcon->SetCustomData(&kNull, sizeof(kNull));
				pkIcon->DefaultImgTexture(NULL);
			}
		}
		pkMyForm->SetInvalidate();
	}

	kVstr = TTW(224) + L". ";
	kVstr+=MyInfo.iLevel;
	kVstr+=L" ";
	kVstr+= MyInfo.wstrName;

	pkMyForm->Text(kVstr);

	// 0:사냥점수, 1:콤보점수, 2:히트점수, 3:피격감점 4:사망감점
	// 전투력 측정 AniBar
	XUI::CXUI_AniBar* pAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pkMyForm->GetControl(std::wstring(L"BAR_XP")));
	if( pAniBar )
	{
		int RankBonus = 0;

		RankBonus = static_cast<int>( (MISSION_NORMALSCORE*static_cast<int>(m_kMission_Result.GetBonusPoint(MBONUSPOINT_PARTY)/10.0f)) + (m_kMission_Result.GetPoint(MPOINT_COMBO)+m_kMission_Result.GetPoint(MPOINT_ATTACK)) - (m_kMission_Result.GetPoint(MPOINT_DEMAGE)+ms_aiDownValue[m_kMission_Result.GetPoint(MPOINT_DIE)]) );
		RankBonus = (RankBonus > MAX_RANK_BONUS)?(MAX_RANK_BONUS):(RankBonus);
		
		pAniBar->Now(RankBonus);
		pAniBar->Max(MAX_RANK_BONUS);
	}

	XUI::CXUI_Wnd *pkWnd = NULL;

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_COMBO_NUM"));
	if (pkWnd)//콤보점수
	{
		if (bInit)
		{
			kVstr = _T("");
		}
		else
		{
			kVstr = _T("0");
			AddNewNum(std::wstring(_T("FRM_COMBO_NUM")), static_cast<float>(m_kMission_Result.GetPoint(1)), 2.5f);
		}
		pkWnd->Text(kVstr);
	}

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_ATTACK_NUM"));
	if (pkWnd)//히트감점
	{
		if (bInit)
		{
			kVstr = _T("");
		}
		else
		{
			kVstr = _T("0");
			AddNewNum(std::wstring(_T("FRM_ATTACK_NUM")), static_cast<float>(m_kMission_Result.GetPoint(2)), 3.0f);
		}
		pkWnd->Text(kVstr);
	}

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_DEMAGE_NUM"));
	if (pkWnd)//피격점수
	{
		if (bInit)
		{
			kVstr = _T("");
		}
		else
		{
			kVstr = _T("0");
			AddNewNum(std::wstring(_T("FRM_DEMAGE_NUM")), static_cast<float>(m_kMission_Result.GetPoint(3)), 3.5f);
		}

		pkWnd->Text(kVstr);
	}

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_DIE_POINT"));
	if (pkWnd)//사망감점
	{
		if (bInit)
		{
			kVstr = _T("");
		}
		else
		{
			kVstr = _T("0");
			int iTemp = m_kMission_Result.GetPoint(4);
			if( iTemp >= MISSION_DOWN )
			{
				iTemp = MISSION_DOWN - 1;
			}
			else if( 0 >= iTemp )
			{
				iTemp = 0;
			}
			AddNewNum(std::wstring(_T("FRM_DIE_POINT")), static_cast<float>(ms_aiDownValue[iTemp]), 4.0f);
		}

		pkWnd->Text(kVstr);
	}


	PutMyInfoToUI(pkParentWnd, _T("FRM_EXP_NUM"), std::wstring(_T("")));
	PutMyInfoToUI(pkParentWnd, _T("FRM_PLUS_EXP_NUM"), std::wstring(_T("")) );

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_TIME_NUM"));
	int iTime = 0;
	if (!bInit)
	{
		AddNewNum(std::wstring(_T("FRM_TIME_NUM")), static_cast<float>(m_kMission_Result.ClearTime()));
	}
	
	PutTimerText(pkWnd, iTime);

	for (int i = 0; i < 4; ++i)
	{
		DWORD dwColor = 0xFFA1A1A1;
		std::wstring strImgPath = _T("");
		std::wstring strValue = L"";

		PutRankBonusInfoUI(i, pkParentWnd, dwColor, strImgPath, strValue, false);
	}

	return true;
}*/

bool PgMissionComplete::PutTimerText(XUI::CXUI_Wnd* pkWnd, int const iValue)
{
	if (!pkWnd)
	{
		return false;
	}

	int const iTotal = iValue / 1000;
	int const iSec1 = iTotal%10;
	int const iSec10 = (iTotal%60)/10;
	int const iMin1 = (iTotal/60)%10;
	int const iMin10 = ((iTotal/60)%60)/10;
	int const iHour1 = (iTotal/3600)%10;
	int const iHour10 = (iTotal/3600)/10;

	BM::vstring kVstr(iHour10);

	kVstr += iHour1;
	kVstr += _T(":");
	kVstr += iMin10;
	kVstr += iMin1;
	kVstr += _T(":");
	kVstr += iSec10;
	kVstr += iSec1;

	pkWnd->Text(kVstr);

	return true;
}

bool PgMissionComplete::SetRoluetteStopRewardItemDraw(XUI::CXUI_Wnd* pkParentWnd, BM::GUID const &rkCharGuid)
{
	if (!pkParentWnd) { return false; }

	BM::vstring kVstr;
	size_t kTempCount = 0;
	XUI::CXUI_Wnd *pkOtherForm = NULL;
	SMissionPlayerInfo_Client OhterInfo;	

	for (int i = 0; i < PARTY_MEMBER_COUNT; ++i)
	{
		bool bDraw = false;

		if (MyNumber() == i) {	continue; }

		kVstr = L"FRM_OTHER_INFO";
		kVstr+=(int)(kTempCount);
		pkOtherForm = pkParentWnd->GetControl(kVstr);
		OhterInfo.Clear();
		if (pkOtherForm)
		{
			if( true == m_kMission_Result.GetInfoFromUserInfoAt(i, OhterInfo) )
			{
				if( OhterInfo.kCharGuid == rkCharGuid )
				{
					bDraw = true;
				}
			}

			if( bDraw )
			{
				XUI::CXUI_Wnd *pkIcon = NULL;
				for (size_t j = 0; j<MAX_REWARD_ITEM_COUNT; ++j )
				{
					BM::vstring kImgName(L"SFRM_BONUS");
					kImgName+=(int)j;
					pkIcon = pkOtherForm->GetControl(kImgName)->GetControl(std::wstring(_T("IMG_BONUS")));
					if (pkIcon)
					{
						PgBase_Item kItem;
						if (j < OhterInfo.kContItem.size())
						{
							kItem = OhterInfo.kContItem.at(j);
						}
						
						BM::Stream kCustomData;
						kItem.WriteToPacket(kCustomData);
						pkIcon->SetCustomData(kCustomData.Data());
						lwDrawMissionRewardItem(lwUIWnd(pkIcon, pkIcon != NULL));
					}
				}
			}

			kVstr = L""; 
			if (OhterInfo.iLevel>0)
			{
				kVstr = TTW(224) + L". ";
				kVstr+=OhterInfo.iLevel;
				kVstr+= L" ";
				kVstr+= OhterInfo.wstrName;
			}
			
			pkOtherForm->Text(kVstr);	//레벨, 이름
			
			XUI::CXUI_Wnd *pkRank = pkOtherForm->GetControl(std::wstring(L"IMG_RANK_CHAR"));
			if (pkRank)
			{
				pkRank->Visible((BM::GUID::NullData() != OhterInfo.kCharGuid));

				if (pkRank->Visible())
				{
					SUVInfo kInfo = pkRank->UVInfo();
					kInfo.Index = OhterInfo.kRank;
					pkRank->UVInfo(kInfo);
				}
			}
		}

		++kTempCount;
		if ( MemberCount() <= kTempCount)
		{
			break;	//더 돌 필요 없으니까 튕기자
		}
	}

	return true;
}

bool PgMissionComplete::PutOtherInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit)
{
	if (!pkParentWnd) { return false; }

	BM::vstring kVstr;
	size_t kTempCount = 0;
	XUI::CXUI_Wnd *pkOtherForm = NULL;
	SMissionPlayerInfo_Client OhterInfo;

	for (int i = 0; i < PARTY_MEMBER_COUNT; ++i)
	{
		if (MyNumber() == i) {	continue; }

		kVstr = L"FRM_OTHER_INFO";
		kVstr+=(int)(kTempCount);
		pkOtherForm = pkParentWnd->GetControl(kVstr);
		OhterInfo.Clear();
		if (pkOtherForm)
		{
			if (!bInit)
			{
				if(!m_kMission_Result.GetInfoFromUserInfoAt(i, OhterInfo))
				{
					//continue;
				}
			} 

			//bInit가 true 일 경우 OhterInfo가 초기화 됬다는 가정 하에 
			XUI::CXUI_Wnd *pkIcon = NULL;
			for (size_t j = 0; j<MAX_REWARD_ITEM_COUNT; ++j )
			{
				BM::vstring kImgName(L"SFRM_BONUS");
				kImgName+=(int)j;
				pkIcon = pkOtherForm->GetControl(kImgName)->GetControl(std::wstring(_T("IMG_BONUS")));
				if (pkIcon)
				{
					BM::Stream kCustomData;

					if( false == bInit )
					{
						PgBase_Item kItem;
						if (j < OhterInfo.kContItem.size())
						{
							kItem = OhterInfo.kContItem.at(j);
						}						
						
						kItem.WriteToPacket(kCustomData);
					}
					else
					{
						kCustomData.Clear();
					}

					pkIcon->SetCustomData(kCustomData.Data());
					lwDrawMissionRewardItem(lwUIWnd(pkIcon, pkIcon != NULL));
				}
			}


			kVstr = L""; 
			if (!bInit && OhterInfo.iLevel>0)
			{
				kVstr = TTW(224) + L". ";
				kVstr+=OhterInfo.iLevel;
				kVstr+= L" ";
				kVstr+= OhterInfo.wstrName;
			}
			
			pkOtherForm->Text(kVstr);	//레벨, 이름

			/*XUI::CXUI_Wnd *pExpForm = pkOtherForm->GetControl(std::wstring(L"FRM_EXP_NUM"));
			if (pExpForm)
			{
				if (bInit || OhterInfo.iLevel<=0)
				{
					kVstr = L""; 
				}
				else
				{
					kVstr = OhterInfo.iAccExp;//시간점수를  더함
				}
				
				pExpForm->Text(kVstr);
			}*/
			
			XUI::CXUI_Wnd *pkRank = pkOtherForm->GetControl(std::wstring(L"IMG_RANK_CHAR"));
			if (pkRank)
			{
				pkRank->Visible(!bInit && (BM::GUID::NullData() != OhterInfo.kCharGuid));

				if (pkRank->Visible())
				{
					SUVInfo kInfo = pkRank->UVInfo();
					kInfo.Index = OhterInfo.kRank;
					pkRank->UVInfo(kInfo);
				}
			}
		}

		++kTempCount;

		if( false == bInit )	// 초기화가 아닌 경우만...초기화인 경우는 무조건 다 초기화
		{			
			if ( MemberCount() <= kTempCount)
			{
				break;	//더 돌 필요 없으니까 튕기자
			}
		}

	}

	return true;
}

bool PgMissionComplete::PutRankInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit)
{
	if (!pkParentWnd)
	{
		return false;
	}

	XUI::CXUI_Wnd *pkRank = pkParentWnd->GetControl(_T("FRM_RANK"));
	if (!pkRank)
	{
		return false;
	}

	const wchar_t *pText = 0;
	BM::vstring kVstr;
	if(GetDefString(m_kMission_Result.MissionNo(), pText))
	{
		kVstr = pText;
	}

	XUI::CXUI_Wnd *pkWndMissionName = pkRank->GetControl(_T("FRM_MISSION_TEXT"));
	if(NULL != pkWndMissionName )
	{// 미션 이름이 너무 길면, 말줄임표를 사용한다
		int const iWidth = pkWndMissionName->Width();
		Quest::SetCutedTextLimitLength(pkWndMissionName, static_cast<std::wstring>(kVstr), _T("..."), iWidth);		
	}
	
	int const iDiff = m_kMission_Result.MissionLevel() + 1;

	for (int i = 0; i<iDiff; ++i)
	{
		kVstr = _T("IMG_STAR");
		kVstr+=i;
		XUI::CXUI_Wnd *pkStar = pkRank->GetControl(kVstr);
		if (pkStar)
		{
			pkStar->Visible(true);
		}
	}

	CONT_MISSION_RANK::const_iterator rank_it = m_kMission_Result.GetFirstElementFromRankData();

	if (!m_kMission_Result.IsRankEnd(rank_it))
	{
		const CONT_MISSION_RANK::value_type &vt = (*rank_it);
		BM::vstring kVstr;
		XUI::CXUI_Wnd *pkWnd = pkRank->GetControl(std::wstring(_T("FRM_RANK_NUM")));
		if (pkWnd)
		{
			kVstr = 1;	//첨엔 무조건 일등밖에 안보여준다
			pkWnd->Text(kVstr);
		}

		pkWnd = pkRank->GetControl(std::wstring(_T("FRM_TIME")));
		if (pkWnd)
		{
			PutTimerText(pkWnd, vt.iPlayTime);
		}

		pkWnd = pkRank->GetControl(std::wstring(_T("FRM_POINT")));
		if (pkWnd)
		{
			kVstr = vt.iPoint;
			pkWnd->Text(kVstr);
		}

		pkWnd = pkRank->GetControl(std::wstring(_T("FRM_LEVEL")));
		if (pkWnd)
		{
			int const iLevel = vt.iUserLevel;
			kVstr = iLevel;
			pkWnd->Text(kVstr);
		}

		pkWnd = pkRank->GetControl(std::wstring(_T("FRM_CLASS")));
		if (pkWnd)
		{
			pkWnd->Text(TTW(30000+vt.kClass));
		}

		pkWnd = pkRank->GetControl(std::wstring(_T("FRM_NAME")));
		if (pkWnd)
		{
			pkWnd->Text(vt.wstrName);
		}

		pkWnd = pkRank->GetControl(std::wstring(_T("FRM_MEMO")));
		if (pkWnd)
		{
			pkWnd->Text(vt.wstrMemo);
		}

	}

	return true;
}

bool PgMissionComplete::PutMyInfoToUI(XUI::CXUI_Wnd* pkParentWnd, std::wstring const &rkName, std::wstring const &rkText)
{
	if (!pkParentWnd)
	{
		return false;
	}

	XUI::CXUI_Wnd *pkMyForm = pkParentWnd->GetControl(std::wstring(L"FRM_MY_INFO"));
	if (!pkMyForm)
	{
		return false;
	}

	XUI::CXUI_Wnd *pkWnd = pkMyForm->GetControl(rkName);
	if (!pkWnd)
	{
		return false;
	}

	pkWnd->Text(rkText);

	return true;
}

void PgMissionComplete::SetAlphaByRecursiveProcess(NiAVObject* pkObject, float const fAlpha)
{
	if (NiIsKindOf(NiGeometry, pkObject))
	{
		NiGeometry	*pkGeom = NiDynamicCast(NiGeometry, pkObject);
		NiPropertyPtr pkProp = pkGeom->GetProperty(NiProperty::ALPHA);
		
		if (!pkProp)
		{
			pkGeom->AttachProperty(m_spNewAlphaProp);
			pkGeom->UpdateProperties();
			pkProp = pkGeom->GetProperty(NiProperty::ALPHA);
		}	
		if (pkProp)
		{
			NiMaterialPropertyPtr spAlpha = pkGeom->GetPropertyState()->GetMaterial();
			if (spAlpha)
			{
				spAlpha->SetAlpha(fAlpha);
				return;
			}
		}
	}

	if(NiIsKindOf(NiNode, pkObject))
	{
		NiNode* pkNode = (NiNode*)pkObject;
		for(unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				SetAlphaByRecursiveProcess(pkChild, fAlpha);
			}
		}
	}
}

bool PgMissionComplete::PlaySound(char const *szPath)
{ 
	if (!szPath)
	{
		return false;
	}

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szPath, 0.0f,80,180,g_kPilotMan.GetPlayerPilot()->GetWorldObject());

	return true;
}


void PgMissionComplete::CleanUp()
{
	CompleteState(E_MSN_CPL_NONE);

	m_pkWnd = XUIMgr.Get(std::wstring(L"FRM_MISSION_COMPLETE"));
	if (m_pkWnd)
	{
		m_pkWnd->Close();
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정 해제
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정 해제
		m_pkWnd = NULL;
		m_pkWndModel = NULL;
	}

	MemberCount(0);
	if (g_pkWorld)
	{
		PgCameraMan* pkCamMan = g_pkWorld->GetCameraMan();
		if (pkCamMan)
		{
			pkCamMan->LockInput(false);
		}
	}
}

void PgMissionComplete::SendMemo(char const* pkStr)
{
	if (BM::GUID::NullData() != m_kMission_Result.GetMemoGuid() && m_kMission_Result.Standing())
	{
		BM::Stream kPacket(PT_C_N_ANS_MISSION_RANK_INPUTMEMO);
		kPacket.Push(m_kMission_Result.GetMemoGuid());
		std::wstring kString;
		if (pkStr)
		{
			kString = UNI(pkStr);
			g_kClientFS.Filter(kString, true, FST_BADWORD);
			if(!g_kUnicodeFilter.IsCorrect(UFFC_MISSION_RANK_COMMENT, kString))
			{
				kString = _T("");
				lwAddWarnDataTT(700406);				
			}
		}
		kString = kString.substr(0,MAX_MISSION_RANK_INPUTMEMO_LEN);
		kPacket.Push(kString);

		NETWORK_SEND(kPacket)
	}
}

void PgMissionComplete::SendReqRankInfo()
{
	if (E_MSN_CPL_REWARD_WAIT != CompleteState())	//대기중일 때 만
	{
		return;
	}

	if (IsRevcRank())
	{
		CallRankUI();
		return;
	}
	BM::Stream kPacket(PT_C_M_REQ_RANK_PAGE);
	kPacket.Push(m_kMission_Result.MissionKey());
	NETWORK_SEND(kPacket)
}

void PgMissionComplete::CallRankUI()
{
	XUI::CXUI_Wnd *pkWnd = XUIMgr.Call(_T("FRM_RANK_LIST"), true);
	if (!pkWnd)
	{
		return;
	}

	XUI::CXUI_Wnd *pkBG = pkWnd->GetControl(_T("SFRM_RANK_BG"));
	if (pkBG)
	{
		BM::vstring kVstr;

		const wchar_t *pText = 0;
		
		if(GetDefString(g_iMissionNo, pText))
		{
			kVstr = pText + std::wstring(L" ") + TTW(301205);
		}

		pkBG->Text(kVstr);

		int const iDiff = m_kMission_Result.MissionLevel() + 1;

		for (int i = 0; i<iDiff; ++i)
		{
			kVstr = _T("IMG_STAR_BG");
			kVstr+=i;
			XUI::CXUI_Wnd *pkStarBG = pkBG->GetControl(kVstr);
			if (pkStarBG)
			{
				XUI::CXUI_Wnd *pkStar = pkStarBG->GetControl(std::wstring(_T("IMG_STAR")));

				pkStar->Visible(true);
			}
		}
	}

	CONT_MISSION_RANK::const_iterator rank_it = m_kMission_Result.GetFirstElementFromRankData();
	BM::vstring kName;
	int iCount = 0;
	while (!m_kMission_Result.IsRankEnd(rank_it))
	{
		kName = _T("FRM_RANK_ELE");
		kName+=iCount;
		XUI::CXUI_Wnd *pkWndRank = pkWnd->GetControl(kName)	;
		if (pkWnd)
		{
			BM::vstring kColor(_T(""));//_T("{C=0xFF4D3413/}");
			DWORD dwColor = 0xFF4D3413;
			if (3>iCount)
			{
				kColor = _T("{C=0xFFC1320C/}");
				dwColor = 0xFFC1320C;
			}

			const CONT_MISSION_RANK::value_type &vt = (*rank_it);
			XUI::CXUI_Wnd *pkWndRankNum = pkWndRank->GetControl(std::wstring(_T("SFRM_RANK_NUM")));
			if (pkWndRankNum)
			{
				BM::vstring kRankNum;
				if (3<=iCount)
				{
					kRankNum+=(iCount+1);
					kRankNum+=TTW(401100);
					pkWndRankNum->FontColor(dwColor);
					pkWndRankNum->Text(kRankNum);
				}
			}
			
			XUI::CXUI_Wnd *pkWndTimeNum = pkWndRank->GetControl(std::wstring(_T("SFRM_TIME_NUM")));
			if (pkWndTimeNum)
			{
				pkWndTimeNum->FontColor(dwColor);
				PutTimerText(pkWndTimeNum, vt.iPlayTime);
			}

			XUI::CXUI_Wnd *pkWndPointNum = pkWndRank->GetControl(std::wstring(_T("SFRM_POINT_NUM")));
			if (pkWndPointNum)
			{
				pkWndPointNum->FontColor(dwColor);
				pkWndPointNum->Text(BM::vstring(vt.iPoint));
			}

			XUI::CXUI_Wnd *pkWndLevelNum = pkWndRank->GetControl(std::wstring(_T("SFRM_LEVEL_NUM")));
			if (pkWndLevelNum)
			{
				BM::vstring kLevNum(TTW(224));
				kLevNum+=_T(".");
				kLevNum+=vt.iUserLevel;

				pkWndLevelNum->FontColor(dwColor);
				pkWndLevelNum->Text(kLevNum);
			}

			XUI::CXUI_Wnd *pkWndClass = pkWndRank->GetControl(std::wstring(_T("SFRM_CLASS")));
			if (pkWndClass)
			{
				pkWndClass->FontColor(dwColor);
				pkWndClass->Text(TTW(30000+vt.kClass));
				XUI::CXUI_Wnd *pkWndClassImg = pkWndClass->GetContAt(0);// 하나밖에 없잖아
				if (pkWndClassImg)
				{// 직업 아이콘 설정
					pkWndClassImg->Visible(true);
					lwSetMiniClassIconIndex(pkWndClassImg, vt.kClass);
				}
			}

			XUI::CXUI_Wnd *pkWndName = pkWndRank->GetControl(std::wstring(_T("SFRM_NAME_TEXT")));
			if (pkWndName)
			{
				pkWndName->FontColor(dwColor);
				pkWndName->Text(vt.wstrName);
			}

			XUI::CXUI_Wnd *pkWndMemo = pkWndRank->GetControl(std::wstring(_T("SFRM_MEMO_TEXT")));
			if (pkWndMemo)
			{
				pkWndMemo->FontColor(dwColor);
				pkWndMemo->Text(vt.wstrMemo);
			}
		}
		++rank_it;
		++iCount;
	}
} 

int PgMissionComplete::CalcMinLevelLimit(int const iMissionNo, int const iLev)
{
	int iLimitNum = 0;
	if(0>=iMissionNo)
	{
		return iLimitNum;
	}

	const CONT_DEF_MISSION_ROOT* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);

	if (pContDefMap)
	{
		CONT_DEF_MISSION_ROOT::const_iterator root_itr = pContDefMap->find(iMissionNo);
		if (pContDefMap->end() != root_itr)
		{
			const TBL_DEF_MISSION_ROOT& element = (*root_itr).second;
			iLimitNum = element.aiLevel_Min[__min(MAX_MISSION_LEVEL, iLev)];
		}
	}

	return iLimitNum;
}

int PgMissionComplete::CalcMaxLevelLimit(int const iMissionNo, int const iLev)
{
	int iLimitNum = 0;
	if(0>=iMissionNo)
	{
		return iLimitNum;
	}

	const CONT_DEF_MISSION_ROOT* pContDefMap = NULL;
	g_kTblDataMgr.GetContDef(pContDefMap);

	if (pContDefMap)
	{
		CONT_DEF_MISSION_ROOT::const_iterator root_itr = pContDefMap->find(iMissionNo);
		if (pContDefMap->end() != root_itr)
		{
			const TBL_DEF_MISSION_ROOT& element = (*root_itr).second;
			iLimitNum = element.aiLevel_Max[__min(MAX_MISSION_LEVEL, iLev)];
		}
	}

	return iLimitNum;
}

void PgMissionComplete::CallRankResultItem(int const iItemNo, int const iNewRank)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if( NULL == pkItemDef )
	{
		return;
	}

	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Call( std::wstring(L"FRM_MISSION_RESULT_ITEM") );
	if( !pkTopWnd )
	{
		return;
	}

	std::wstring kTempStr;
	wchar_t const* pkItemName = NULL;
	if( GetDefString(pkItemDef->NameNo(), pkItemName) )
	{
		if( FormatTTW(kTempStr, 401063, iNewRank, pkItemName) )
		{			
		}
	}

	pkTopWnd->Text( kTempStr );

	XUI::CXUI_Wnd* pkIconWnd = pkTopWnd->GetControl( std::wstring(L"ICN_ITEM") );
	if( pkIconWnd )
	{
		SQuestItem const kQuestItem(iItemNo, 1);
		pkIconWnd->SetCustomData(&kQuestItem, sizeof(SQuestItem));
		Quest::lwOnCallQuestRewardItemImage( lwUIWnd(pkIconWnd) );
		pkIconWnd->Scale( 1.5f ); // 1.5배
	}
}

void PgMissionComplete::StopResult()
{
	if( true == SpaceStop() )
	{
		return;
	}

	XUI::CXUI_Wnd *pkForm_Left = XUIMgr.Call(std::wstring(L"FRM_MY_INFO_NEXT"));

	if ( !pkForm_Left )
	{
		return;
	}

	SMissionPlayerInfo_Client MyInfo;
	if(!m_kMission_Result.GetInfoFromUserInfoAt(MyNumber(), MyInfo))
	{
		return;
	}
	size_t iItemNum = MyInfo.kContItem.size();

	if( MAX_REWARD_ITEM_COUNT < iItemNum )
	{
		// 최대 4개 까지만 된다.
		iItemNum = MAX_REWARD_ITEM_COUNT;
	}

	for(int i=0; i<iItemNum; ++i)
	{
		BM::vstring kImgName(L"SFRM_BONUS");
		kImgName+=i;
		XUI::CXUI_Wnd *pkForm_ImgName = pkForm_Left->GetControl(kImgName);
		if( pkForm_ImgName )
		{
			PgFormRouletteWnd *pkIcon = dynamic_cast<PgFormRouletteWnd *>( pkForm_ImgName->GetControl(std::wstring(_T("IMG_BONUS"))) );
			if( pkIcon )
			{
				PgBase_Item & kItem = MyInfo.kContItem[i];
				if (kItem.ItemNo() == 0)
				{ 
					kItem.ItemNo(20400217);
				}
				pkIcon->SetRoulette(MyInfo.iResultBagNo, kItem, iItemNum);
				if(pkIcon->Scale() <= 1.0f) 
				{
					pkIcon->VScale(2.0f);
				}

				pkIcon->SetStopKeyState();
				pkIcon->SetStopKey();				

				_PgOutputDebugString("ItemNo : %d", kItem.ItemNo());
			}
		}
	}

	XUI::CXUI_Wnd	*m_pkGadaWnd;
	m_pkGadaWnd = pkForm_Left->GetControl(std::wstring(_T("SFRM_GADACOIN")));
	if( m_pkGadaWnd )
	{
		XUI::CXUI_Image* const pkIcon = dynamic_cast<XUI::CXUI_Image*>(m_pkGadaWnd->GetControl(_T("IMG_GADACOIN")));
		if( pkIcon )
		{
			SUVInfo rkUV = pkIcon->UVInfo();
			pkIcon->UVInfo(rkUV);
			POINT2 kPoint(40*rkUV.U, 40*rkUV.V);//이미지 원본 사이즈 복구
			pkIcon->ImgSize(kPoint);
			pkIcon->VScale(2.0f);
			pkIcon->SetInvalidate();
		}
	}

	SpaceStop(true);
}

void PgMissionComplete::StopMissionResult()
{
	if( true == SpaceStop() )
	{
		return;
	}

	XUI::CXUI_Wnd *pkForm_Left = XUIMgr.Call(std::wstring(L"FRM_MISSION_COMPLETE_NEW"));

	if ( !pkForm_Left )
	{
		return;
	}

	SpaceStop(true);
}

void PgMissionComplete::CallSelectDirectionUI()
{
	int iBonusExp = std::min( static_cast<int>(m_kMission_Result.BonusExp()), INT_MAX );
	if( true == m_kMission_Result.IsResultStage() )
	{
		// 보상 받은 스테이지
		iBonusExp = 0;
	}
	lua_tinker::call< void, int >("SetDefenceBonusExp", iBonusExp);
	XUI::CXUI_Wnd *pkWndTop  = XUIMgr.Call(L"FRM_MISSION_SELECT_DIRECTION");
	if(!pkWndTop)
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
	int const iItemNo = lwGetMissionCashItemNo(); 
	pkWndIcon->SetCustomData(&iItemNo,sizeof(iItemNo));
	

	int const iStageNo = lua_tinker::call< int >("GetDefenceStageNo");
	lwSet_DefenceSelectUI(iStageNo);
}

void PgMissionComplete::MissionEndUnlockInput()
{
	if( false == MissionEnd() )
	{
		lua_tinker::call<void, bool>("LockGlobalHotKey", false);	//글로벌 핫키 고정 고정
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정 해제 
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정 해제
	}
}

int PgMissionComplete::GetUICloseTime2()const
{
	float const fAccumTime = g_pkWorld ? g_pkWorld->GetAccumTime() : TimeStamp();
	float const fFlowTime = fAccumTime - TimeStamp();
	return std::max<int>(0, UI_CLOSE_TIME_2 - fFlowTime);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

PgBossComplete::PgBossComplete() : m_spWhiteBillboard(0), m_spAlphaMaterial(0), m_spBGAlphaMaterial(0), m_spNewAlphaProp(0), m_pkWndModel(NULL)
{
	m_vecBoxPos.reserve(REWARD_BOX_COUNT);
	m_vecBoxName.reserve(REWARD_BOX_COUNT);

	TimeStamp(0);
	m_pkWnd = NULL;
	m_spBackGround = 0;
	Init();
	MemberCount(0);
}

PgBossComplete::~PgBossComplete()
{
	Destroy();
}

void PgBossComplete::Destroy()
{
	m_spBackGround = 0; 
	m_spWhiteBillboard = 0;
	m_spAlphaMaterial = 0;
	m_spBGAlphaMaterial = 0;
	m_spCamera = NULL;
	m_vecBoxName.clear();
	m_spNewAlphaProp = 0;
	for (int i = 0; i < REWARD_BOX_COUNT; i++)
	{
		m_kRewardBox[i].Destroy();
	}
}

void PgBossComplete::Init()
{
	m_pkWnd = NULL;
	m_pkWndUIModel = NULL;
	SelectBoxNum(-1);
	ItemCount(0);
	CamWorldPos(NiPoint3(0,0,0));
	CompleteState(E_MSN_CPL_NONE);
	MyNumber(0);

	NowViewItem(-1);
	BossMission(false);

	if (m_spBackGround == 0)
	{
		m_spBackGround = g_kNifMan.GetNif("../Data/4_Item/9_Etc/MissionMapCube/plan.nif");

		if (m_spBackGround)
		{
			m_spBackGround->SetScale(1.2f);
			m_spBackGround->UpdateNodeBound();
			m_spBackGround->UpdateProperties();
			m_spBackGround->UpdateEffects();
			m_spBackGround->Update(0);
		}
	}

	if (m_spWhiteBillboard == 0)
	{
		m_spWhiteBillboard = g_kNifMan.GetNif_DeepCopy("../Data/6_UI/billboard_quad.nif");
		if(m_spWhiteBillboard) 
		{
			NiGeometry	*pkTextGeom = (NiGeometry*)m_spWhiteBillboard->GetObjectByName("Plane");
			if (pkTextGeom)
			{
				NiTexturingProperty	*pkProperty = pkTextGeom->GetPropertyState()->GetTexturing();

				NiSourceTexturePtr spSimpleTextTex = g_kNifMan.GetTexture("../Data/6_UI/whitebg.tga");
				pkProperty->SetBaseTexture(spSimpleTextTex);

				NiPropertyPtr spAlpha = pkTextGeom->GetProperty(NiAlphaProperty::GetType());
				if (spAlpha)
				{
					m_spAlphaMaterial = pkTextGeom->GetPropertyState()->GetMaterial();
					if (m_spAlphaMaterial)
					{
						m_spAlphaMaterial->SetAlpha(0.0f);
					}
				}

				m_spWhiteBillboard->SetScale(10.0f);
				m_spWhiteBillboard->UpdateProperties();
				m_spWhiteBillboard->UpdateEffects();
				m_spWhiteBillboard->Update(0);
			}
		}
	}

	m_vecBoxPos.clear();
	m_vecBoxName.clear();
	m_spCamera = 0;

	std::wstring kTempString = _T("");
	CXUI_Font	*pkFont = g_kFontMgr.GetFont(FONT_NAME);
	PG_ASSERT_LOG(pkFont);

	Pg2DString kTemp2DString( XUI::PgFontDef(pkFont), kTempString );
	for(int i = 0; i!=REWARD_BOX_COUNT; ++i)
	{
		m_kRewardBox[i].Init();
		m_kReward[i].Init();
		m_vecBoxName.push_back( kTemp2DString );
	}
	//배경에 알파 프로퍼티가 없어서 하나 만들어놓자
	m_spNewAlphaProp = NiNew NiAlphaProperty;
	m_spNewAlphaProp->SetAlphaBlending(true);
	m_spNewAlphaProp->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
	m_spNewAlphaProp->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);

	// 	//////////////////////////////////////////////////////////////////////////
	// 	//임시 데이터
	//	m_kReward[0].iNormalExp = 1;
	//	m_kReward[0].iStrangthExp = 10;
	//	m_kReward[0].iCrackExp = 100;
	//
	//	m_kReward[0].iGetExp = 100;
	//	m_kReward[0].iTotalExp = 200;
	//	m_kReward[0].iLevel = 99;
	//	m_kReward[0].wstrName = L"임시입니다1";
	//
	//	m_kReward[0].iItemCount = 4;
	//	m_kReward[0].iRewardItem[0] = 20106002;
	//	m_kReward[0].iRewardItem[1] = 20106014;
	//	m_kReward[0].iRewardItem[2] = 20121001;
	//	m_kReward[0].iRewardItem[3] = 20122001;
	// 	//
	// 	//////////////////////////////////////////////////////////////////////////
}

void PgBossComplete::Start()
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
	{
		return;
	}

	Init();
	CompleteState(E_MSN_CPL_WAIT);
	TimeStamp(g_pkWorld->GetAccumTime());

	m_spCamera = g_pkWorld->GetCameraMan()->GetCamera();

//	g_pkWorld->RemoveAllMonster(1);	//일반 몬스터 모두 제거
//	g_pkWorld->RemoveAllMonster(2);	//강화 몬스터 모두 제거

	bTimerStart = false;

	lua_tinker::call<void, bool>("LockGlobalHotKey", true);	//글로벌 핫키 고정 고정
}

void PgBossComplete::ResizeBGByResolution()
{
	if(!g_pkWorld)
	{
		return;
	}
	PgWorld::CompleteBGContainer kContainer;
	g_pkWorld->GetCompleteBG(kContainer);

	if (kContainer.empty()) // 일반 미션
	{
		char szName[255] = {0,};
		for (int i = 0; i < 4; ++i)
		{
			_snprintf(szName, 254, "../Data/4_Item/9_Etc/MissionMapCube/plan_0%d.dds", i+1);
			kContainer.insert(std::make_pair(i, std::string(szName)));
		}
	}

	PgWorld::CompleteBGContainerIter it = kContainer.begin();

	NiRenderer *pkRenderer = NiRenderer::GetRenderer();
	assert(pkRenderer);

	unsigned int uiWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	float	fAspect = static_cast<float>(uiWidth)/static_cast<float>(uiHeight);

	int i = 0;
	while(it != kContainer.end())
	{
		char szName[255] = {0,};
		_snprintf(szName, 254, "Plane02:%d", i);
		NiGeometry	*pkTextGeom = (NiGeometry*)m_spBackGround->GetObjectByName(szName);
		if (pkTextGeom)
		{
			NiTexturingProperty	*pkProperty = pkTextGeom->GetPropertyState()->GetTexturing();
			if (pkProperty)
			{
				NiSourceTexturePtr spSimpleTextTex = g_kNifMan.GetTexture((it->second).c_str());
				pkProperty->SetBaseTexture(spSimpleTextTex);
			}

			NiGeometryData* pkData = pkTextGeom->GetModelData();
			NiPoint3* kPoint[4];
			for(int i = 0; i<4; ++i)
			{
				kPoint[i] = (pkData->GetVertices() + i);
			}
			float fHeight = fabs(kPoint[0]->y - kPoint[1]->y)*0.75f;//가로세로 길이 같아서 새로 길이가 3/4로 계산되어야 함
			if(0.0f==fHeight)	//가로세로 길이가 같고 세로 길이는 변하지 않는다는 가정 하에
			{					//중앙이 0,0,0이라는 가정 하에
				fHeight = fabs(kPoint[2]->y - kPoint[3]->y);
			}

			float fwidth = fAspect*fHeight;
			for(int i = 0; i<4; ++i)
			{
				if(kPoint[i]->x)
				{
					kPoint[i]->x = 0 < kPoint[i]->x ? fwidth : -fwidth;
				}
			}
			
			pkData->SetConsistency(NiGeometryData::MUTABLE);
			pkData->MarkAsChanged(NiGeometryData::VERTEX_MASK);
		}
		++i;
		++it;
	}

	m_spBackGround->UpdateNodeBound();
	m_spBackGround->UpdateProperties();
	m_spBackGround->UpdateEffects();
	m_spBackGround->Update(0);
}

bool PgBossComplete::Update(float const fAccumTime, float const fFrameTime)
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
	{
		CompleteState(E_MSN_CPL_NONE);
		return false;
	}
	if (E_MSN_CPL_NONE == CompleteState())
	{
		XUIMgr.BlockGlobalScript(false);
		return false;
	}
	if (g_pkWorld==NULL)
	{
		return false;
	}
	if (!g_pkWorld->GetCameraMan() || !g_pkWorld->GetCameraMan()->GetCamera())
	{
		return false;
	}

	//CamWorldPos(g_pkWorld->GetCameraMan()->GetCamera()->GetWorldTranslate());	//카메라가 이동되므로 매프레임 체크하자.

	NiQuaternion kCurruntRot;
	if (!m_spCamera)
	{
		return false;
	}
	m_spCamera->GetRotate(kCurruntRot);
	NiPoint3 kDir = m_spCamera->GetWorldDirection();
	kDir.Unitize();
	NiMatrix3 kCamRot = m_spCamera->GetRotate();
	float const fFlowTime = fAccumTime - TimeStamp();

	XUI::CXUI_Wnd* pkDead =	XUIMgr.Get(_T("FRM_DEAD_MISSION"));
	if (pkDead)
	{
		pkDead->Close();
	}

	switch(CompleteState())
	{
	case E_MSN_CPL_WAIT:
		{
			if (fFlowTime >= MSN_CPL_WAIT_TIME_BOSS)
			{
				CamWorldPos(g_pkWorld->GetCameraMan()->GetCamera()->GetWorldTranslate());	//카메라가 이동되므로 매프레임 체크하자.

				g_kBalloonMan2D.BalloonOff(); //말풍선은 모두 끄자
				CompleteState(E_MSN_CPL_FADE);
				g_pkWorld->SetShowWorldFocusFilter("../Data/5_Effect/9_Tex/EF_blackBG.tga", 0.0f, 0.5f, BG_FADE_TIME, false);	//화면 페이드

				//m_spBackGround->SetTranslate(CamWorldPos());	// 카메라 위치에 갖다놓기
				//여기서 사운드 재생

				m_spBackGround->SetScale(1.7f);
				m_spBackGround->SetTranslate(CamWorldPos() + kDir*3.0f);	//카메라 위치에서 약간 앞으로
				NiMatrix3 mat(m_spCamera->GetWorldDirection(),
					m_spCamera->GetWorldUpVector(),
					m_spCamera->GetWorldRightVector());

				m_spBackGround->SetRotate(kCamRot.TransposeTimes(mat));

				m_spBackGround->Update(0);
				PastCamSpeed(g_kSpeed);					//현재 카메라 속도 저장
				g_kSpeed = NiQuaternion(0, 0, 0, 0);	//카메라 고정
				g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정
				g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정
				TimeStamp(fAccumTime);
				lwUIOff();
//				lwActivateUI("ChatBar"); // 채팅 출력/입력 창만 표시
//				XUIMgr.Activate(L"ChatOut");
				XUIMgr.BlockGlobalScript(true);
				PgCameraMan* pkCamMan = g_pkWorld->GetCameraMan();
				if (pkCamMan)
				{ 
					pkCamMan->ClearQuake();
					pkCamMan->LockInput(true);
				}
				break;
			}
		}break;
	case E_MSN_CPL_FADE:
		{
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정
			if (fFlowTime >= BG_FADE_TIME)
			{
				g_kSoundMan.StopBgSound();
				PlaySound("MSC_Opening");
				SetAlphaByRecursiveProcess(m_spBackGround, 1.0f);
				//PlaySound("MSC_Opening");
				m_spBackGround->UpdateProperties();
				m_spBackGround->Update(0);
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_BG);
				g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정
				g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정
				ResizeBGByResolution();
			}
		}break;
	case E_MSN_CPL_BG:
		{
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정
			kCurruntRot.Normalize();
			NiPoint3 kRot(kCurruntRot.GetX(), kCurruntRot.GetY(), kCurruntRot.GetZ());
			if (fFlowTime >= BG_COME_TIME)
			{
				m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
				m_spBackGround->Update(0);

				NiPoint3 kRVec = m_spCamera->GetWorldRightVector();	//카메라의 우측 벡터
				NiPoint3 kPos = kRVec*((REWARD_BOX_COUNT-1)*BOX_INTER_X*-0.5f);

				for(int i = 0; i < REWARD_BOX_COUNT; ++i)	// 우측 벡터 방향으로 상자들 기본좌표 설정
				{
					m_vecBoxPos.push_back(kPos);
					kPos += kRVec*BOX_INTER_X;
				}

				BoxPosVec::iterator it = m_vecBoxPos.begin();
				for(int i = 0; i < REWARD_BOX_COUNT; ++i)	// 월드좌표에 상자들 배치
				{
					NiPoint3 kPos = (*it);
					kPos+=CamWorldPos();
					m_kRewardBox[i].SetTranslate(kPos);
					m_kRewardBox[i].BoxState(PgRewardBox::E_REWORD_BOX_DROP);	//상자 상태 변경
					m_kRewardBox[i].BoxState(PgRewardBox::E_REWORD_BOX_IDLE);
					m_kRewardBox[i].UpDownVec(m_spCamera->GetWorldUpVector());
					//m_kRewardBox[i].FaceToCamera(CamWorldPos());
					m_kRewardBox[i].Update(fAccumTime, fFrameTime);
					++it;
				}
				g_pkWorld->SetShowWorldFocusFilter(false, "../Data/5_Effect/9_Tex/EF_blackBG.tga", 0.8f);	//다 움직였으면 화면을 가득 채우므로
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_DROP);
				//lua_tinker::call<void>("LockPlayerInput");	//캐릭터 고정
			}
			else
			{
				NiPoint3 kNow = CamWorldPos() + kDir * fFlowTime / BG_COME_TIME * BG_COME_LENGTH;
				m_spBackGround->SetTranslate(kNow);	// 항상 카메라를 따라다니도록	
				m_spBackGround->Update(0);
			}
		}break;
	case E_MSN_CPL_DROP:
		{
			g_pkWorld->RemoveAllMonster(EMGRADE_NORMAL);	//일반 몬스터 모두 제거
			g_pkWorld->RemoveAllMonster(EMGRADE_UPGRADED);	//강화 몬스터 모두 제거

			NiPoint3 kPos;
			float fStartTime;
			NiPoint3 kUp = m_spCamera->GetWorldUpVector()*BOX_UP_LENGTH;
			for ( int i=0; i!=REWARD_BOX_COUNT; ++i )
			{
				fStartTime = BOX_DROP_TIME_STEP * i;
				if ( fFlowTime >= (BOX_DROP_TIME + fStartTime) )
				{

					if(!IsPlayedBoxSound[i])
					{
						PlaySound("MSC_Drop_Box");
						IsPlayedBoxSound[i] = true;
					}

					//상자들을 카메라가 보는 방향으로 움직이자

					NiPoint3 kUp = m_spCamera->GetWorldUpVector()*BOX_UP_LENGTH;
					kPos = m_vecBoxPos[i];
					kPos+=(CamWorldPos() + kDir * BOX_COME_LENGTH + kUp);
					m_kRewardBox[i].SetTranslate(kPos);
					m_kRewardBox[i].Update(fAccumTime, fFrameTime);
					m_kRewardBox[i].UpDown(true);

					if ( REWARD_BOX_COUNT == (i+1) )
					{
						TimeStamp(fAccumTime);
						CompleteState(E_MSN_CPL_SELECT);

						for (int j = 0; j < REWARD_BOX_COUNT; ++j)
						{
							IsPlayedBoxSound[j] = false;
						}
						m_pkWnd = XUIMgr.Call(std::wstring(L"FRM_MISSION_COMPLETE"));
						if (m_pkWnd)
						{
							m_pkWnd->Size(XUIMgr.GetResolutionSize());
							m_pkWnd->Location(0,0);
							
							XUI::CXUI_Wnd *pExit = m_pkWnd->GetControl(std::wstring(L"SFRM_EXIT_BG"));
							if (pExit)
							{
								pExit->Visible(false);
								pExit->VAlign();
							}
							PutMyInfoToUI(m_pkWnd, true);		//초기화
							PutOtherInfoToUI(m_pkWnd, true);	//초기화
							XUI::CXUI_Wnd *pkTimerForm = m_pkWnd->GetControl(std::wstring(L"FRM_TIMER"));
							if (pkTimerForm)
							{
								pkTimerForm->VAlign();
								pkTimerForm->Visible(true);
							}

							XUI::CXUI_Wnd *pkTextForm = m_pkWnd->GetControl(std::wstring(L"FRM_TEXT"));
							if (pkTextForm)
							{
								pkTextForm->Visible(true);
							}

							XUI::CXUI_Wnd *pkForm_Left = m_pkWnd->GetControl(std::wstring(L"SFRM_MY_INFO"));
							XUI::CXUI_Wnd *pkForm_Right = m_pkWnd->GetControl(std::wstring(L"SFRM_OTHER_INFO"));
							if ( pkForm_Left && pkForm_Right )
							{
								pkForm_Left->SetState( ST_STOP );
								pkForm_Left->Visible(false);
								pkForm_Right->SetState( ST_STOP );
								pkForm_Right->Visible(false);
							}

							pExit = m_pkWnd->GetControl(std::wstring(L"FRM_TEXT"));
							if (pExit)
							{
								pExit->VAlign();
							}
						}
					}
				}
				else if ( fFlowTime >= fStartTime )
				{
					float const fRate = (fFlowTime-fStartTime) / BOX_DROP_TIME;
					float const fZ = fRate * BOX_COME_LENGTH;
					//상자들을 카메라가 보는 방향으로 움직이자
					kPos = m_vecBoxPos[i];
					if(fZ<BOX_COME_LENGTH)
					{
						kPos += (CamWorldPos() + kDir*fZ + kUp*fRate);
						m_kRewardBox[i].SetTranslate(kPos);
						//	m_kRewardBox[i].FaceToCamera(CamWorldPos());
					}

					m_kRewardBox[i].Update(fAccumTime, fFrameTime);
				}
			}

			m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			m_spBackGround->Update(0);
		}break;
	case E_MSN_CPL_SELECT:
		{
			m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			m_spBackGround->Update(0);

			//		if (fFlowTime < BOX_SELECT_TIME)
			//		{
			Pickup();
			UpdateBox(fAccumTime, fFrameTime);

			float fDelta = BOX_SELECT_TIME + BOX_SELECT_DELAY_TIME - fFlowTime;
			if (fDelta<0)
			{
				fDelta = 0;
			}
			else if (BOX_SELECT_TIME <= fDelta)
			{
				fDelta = BOX_SELECT_TIME;
			}
			else if (!bTimerStart)
			{
				PlaySound("MSC_Timer");
				bTimerStart = true;
			}

			int iDelta = (int)fDelta;//소숫점 떨구기 위해 
			int const iSec1 = (iDelta%60)%10 + 1;
			int const iSec10 = (iDelta%60)/10 + 1;
			int const iSec01 = (int)((fDelta-iDelta)*10)+1;
			int const iSec001 = (int)((fDelta*10.0f-(int)(fDelta*10.0f))*10.0f) + 1;
			if (!m_pkWnd){break;}
			XUI::CXUI_Wnd *pkTimerForm = m_pkWnd->GetControl(std::wstring(L"FRM_TIMER"));
			if (pkTimerForm)
			{
				XUI::CXUI_Wnd *pkMin10 = pkTimerForm->GetControl( std::wstring(L"FRM_MIN10") );
				if (pkMin10) { pkMin10->UVUpdate( iSec10 ); }
				XUI::CXUI_Wnd *pkMin1 = pkTimerForm->GetControl( std::wstring(L"FRM_MIN1") );
				if (pkMin1) { pkMin1->UVUpdate( iSec1 ); }
				XUI::CXUI_Wnd *pkSec10 = pkTimerForm->GetControl( std::wstring(L"FRM_SEC10") );
				if (pkSec10) { pkSec10->UVUpdate( iSec01 ); }
				XUI::CXUI_Wnd *pkSec1 = pkTimerForm->GetControl( std::wstring(L"FRM_SEC1") );
				if (pkSec1) { pkSec1->UVUpdate( iSec001 ); }
			}
		}break;
		/*			else if (SelectBoxNum() < 0)
		{
		//서버에서 자동선택
		m_kRewardBox[SelectBoxNum()-1].BoxState(PgRewardBox::E_REWORD_BOX_SELECT);
		//TimeStamp(fAccumTime);
		//CompleteState(E_MSN_CPL_OPEN);	//임시. 서버에서 패킷 보내야만 다음 스테이트로 넘어가야 함.
		}*/
		//		else
		//		{
		//			TimeStamp(fAccumTime);
		//			CompleteState(E_MSN_CPL_OPEN);	//임시. 서버에서 패킷 보내야만 다음 스테이트로 넘어가야 함.
		// 			m_kRewardBox[SelectBoxNum()].BoxState(PgRewardBox::E_REWORD_BOX_OPEN);
		//		}
	case E_MSN_CPL_MOVE_START:
		{
			m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			m_spBackGround->Update(0);

			for ( int i=0; i!=REWARD_BOX_COUNT; ++i  )
			{
				if ( i != SelectBoxNum() )
				{
					m_kRewardBox[i].BoxState( PgRewardBox::E_REWORD_BOX_DISABLE);
				}
			}

			if(	!m_vecBoxPos.empty()
				||	m_vecBoxPos.size() > SelectBoxNum() )
			{
				m_vecBoxPos[SelectBoxNum()] = m_kRewardBox[SelectBoxNum()].GetTranslate();
			}

			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정
			CompleteState(E_MSN_CPL_MOVE);
			TimeStamp( fAccumTime );
		}break;
	case E_MSN_CPL_MOVE:
		{
			m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			m_spBackGround->Update(0);

			if ( fFlowTime >= BOX_OPEN_BEFORE_MOVE_TIME )
			{
				if (m_pkWnd)
				{
					XUI::CXUI_Wnd *pkTimer = m_pkWnd->GetControl(std::wstring(L"FRM_TIMER"));
					if (pkTimer)
					{
						pkTimer->Visible(false);
					}

					XUI::CXUI_Wnd *pkTextForm = m_pkWnd->GetControl(std::wstring(L"FRM_TEXT"));
					if (pkTextForm)
					{
						pkTextForm->Visible(false);
					}
				}
				CompleteState(E_MSN_CPL_OPEN);
				TimeStamp(fAccumTime);
				m_kRewardBox[SelectBoxNum()].BoxState(PgRewardBox::E_REWORD_BOX_OPEN);
			}
			else
			{
				// 				NiPoint3 kMoveDir = m_spCamera->GetTranslate() - m_vecBoxPos[SelectBoxNum()];
				// 				NiPoint3 ptPos = m_vecBoxPos[SelectBoxNum()] + ( fFlowTime/BOX_OPEN_BEFORE_MOVE_TIME * kMoveDir );  
				// 				m_kRewardBox[SelectBoxNum()].SetTranslate(ptPos);
				// 				m_kRewardBox[SelectBoxNum()].Update( fAccumTime, fFrameTime );
			}

			UpdateBox(fAccumTime, fFrameTime);
		}break;
	case E_MSN_CPL_OPEN:
		{
			m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			m_spBackGround->Update(0);

			if ( fFlowTime >= BOX_OPEN_TIME )
			{
				if (m_pkWnd)
				{
					XUI::CXUI_Wnd *pkTimer = m_pkWnd->GetControl(std::wstring(L"FRM_TIMER"));
					if (pkTimer)
					{
						pkTimer->Visible(false);
					}
				}
				CompleteState(E_MSN_CPL_LIGHT);
				TimeStamp(fAccumTime);
			}
			else
			{
				float const fAlpha = sin(NI_HALF_PI*fFlowTime/(BOX_OPEN_TIME+BG_WHITE_FADE_TIME));
				m_spAlphaMaterial->SetAlpha(fAlpha);
				//m_spWhiteBillboard->SetTranslate(CamWorldPos() + kDir*BOX_COME_LENGTH*0.5f);
				m_spWhiteBillboard->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH*0.2f);
				m_spWhiteBillboard->UpdateProperties();
				m_spWhiteBillboard->Update(0);
			}

			UpdateBox(fAccumTime, fFrameTime);
		}break;
	case E_MSN_CPL_LIGHT:
		{
			m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			m_spBackGround->Update(0);
			UpdateBox(fAccumTime, fFrameTime);

			if (fFlowTime >= BG_WHITE_FADE_TIME)	
			{
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_REWARD);

				if (m_pkWnd)
				{
					XUI::CXUI_Wnd *pkForm_Left = m_pkWnd->GetControl(std::wstring(L"SFRM_MY_INFO"));
					XUI::CXUI_Wnd *pkForm_Right = m_pkWnd->GetControl(std::wstring(L"SFRM_OTHER_INFO"));

					if ( !pkForm_Left || !pkForm_Right )
					{
						break;
					}
					pkForm_Left->Visible(true);
					pkForm_Right->Visible(true);
				}

				PutMyInfoToUI(m_pkWnd);		// 내정보를 UI에 설정
				PutOtherInfoToUI(m_pkWnd);	// 다른사람 정보를 UI에 설정
				m_spAlphaMaterial->SetAlpha(0.0f);
			}
			else
			{
				float const fAlpha = sin(NI_HALF_PI*(fFlowTime+BOX_OPEN_TIME)/(BOX_OPEN_TIME+BG_WHITE_FADE_TIME));
				m_spAlphaMaterial->SetAlpha(fAlpha);
			}
			if (m_spWhiteBillboard)
			{
				m_spWhiteBillboard->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH*0.2f);
				m_spWhiteBillboard->UpdateProperties();
				m_spWhiteBillboard->Update(0);
			}

		}break;
	case E_MSN_CPL_REWARD:	//UI움직이기
		{
			m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			m_spBackGround->Update(0);

			if (fFlowTime < UI_MOVE_TIME)
			{
				if (m_pkWnd)
				{
					float const fRate = fFlowTime/UI_MOVE_TIME;
					XUI::CXUI_Wnd *pkForm_Left = m_pkWnd->GetControl(std::wstring(L"SFRM_MY_INFO"));
					if (pkForm_Left)
					{
						pkForm_Left->Location((fRate-1.0f)*pkForm_Left->Size().x, pkForm_Left->Location().y);
					}
					XUI::CXUI_Wnd *pkForm_Right = m_pkWnd->GetControl(std::wstring(L"SFRM_OTHER_INFO"));
					if (pkForm_Right)
					{						
						pkForm_Right->Location(XUIMgr.GetResolutionSize().x - fRate*pkForm_Right->Size().x, pkForm_Right->Location().y);
					}
					//if (!BossMission())
					{
						XUI::CXUI_Wnd *pkForm_Title = m_pkWnd->GetControl(std::wstring(L"FRM_RESULT_TITLE"));
						if (pkForm_Title)
						{
							pkForm_Title->Visible(true);
							pkForm_Title->Location(pkForm_Title->Location().x, pkForm_Title->Location().y+fFrameTime/UI_MOVE_TIME*UI_TITLE_MOVE_LENGTH);
						}
					}
				}
			}
			else
			{
				if (m_pkWnd)
				{
					XUI::CXUI_Wnd *pkForm_Left = m_pkWnd->GetControl(std::wstring(L"SFRM_MY_INFO"));
					if (pkForm_Left)
					{
						pkForm_Left->Location(0, pkForm_Left->Location().y);
					}
					XUI::CXUI_Wnd *pkForm_Right = m_pkWnd->GetControl(std::wstring(L"SFRM_OTHER_INFO"));
					if (pkForm_Right)
					{
						pkForm_Right->Location(XUIMgr.GetResolutionSize().x - pkForm_Right->Size().x, pkForm_Right->Location().y);
					}
					//if (!BossMission())
					{
						XUI::CXUI_Wnd *pkForm_Title = m_pkWnd->GetControl(std::wstring(L"FRM_RESULT_TITLE"));
						if (pkForm_Title)
						{
							pkForm_Title->Location(pkForm_Title->Location().x, UI_TITLE_MOVE_LENGTH-pkForm_Title->Size().y);
						}
					}
				}
				lua_tinker::call<bool, float, float, int, float, int>("QuakeCamera", 0.4f, 1.5f,0,0,0);	//화면 흔들림
				PlaySound("MSC_UI_Comming");

				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_REWARD_DISPLAY);
				//m_pkWndModel = XUIMgr.Call(std::wstring(L"FRM_UIMODEL"));
				// UI Model
				m_pkWndUIModel = NULL;
			}

		}break;

	case E_MSN_CPL_REWARD_DISPLAY:	// 화면에 표시
		{
			//m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
			m_spBackGround->Update(0);

			XUI::CXUI_Wnd *pkForm_Left = NULL;
			if (m_pkWnd)
			{
				pkForm_Left = m_pkWnd->GetControl(std::wstring(L"SFRM_MY_INFO"));
			}
			 
			if ( !pkForm_Left )
			{
				break;
			}

			/*			if (!m_pkWndUIModel)	//없으면 한번 갖고와 보고
			{
			m_pkWndUIModel = g_kUIScene.GetRenderModel(g_kPilotMan.GetPlayerActor()->GetID().c_str());
			if (m_pkWndUIModel) //있으면 승리포즈 취해 주자
			{
			m_pkWndUIModel->SetTargetAnimation(2000035);
			_PgOutputDebugString("m_pkWndUIModel->SetTargetAnimation(2000035)\n");
			}
			}*/

			float const fItemTime = UI_ICON_APPEAR_TIME/(float)MAX_REWARD_ITEM_COUNT;
			float const fPastTime = fItemTime*m_kReward[0].iItemCount;
			if(fFlowTime < fPastTime)	//4개라서 4.0. 아이템 카운트는 다 같으니까 아무거나 갖고 온것. 매직넘버 좋지 않아;;
			{
				int const iRealTime = fFlowTime/fItemTime;
				if (iRealTime != NowViewItem())	//단위 시간당 하나의 아이템 그림을 화면에 표시
				{
					NowViewItem(iRealTime);

					BM::vstring kImgName(L"IMG_BONUS");
					kImgName+=iRealTime;
					XUI::CXUI_Wnd *pkIcon = pkForm_Left->GetControl(std::wstring(L"FRM_ITEM_SDW"))->GetControl(kImgName);
					if (pkIcon)
					{
						SMissionRewardInfo &MyInfo = m_kReward[MyNumber()];

						int iLoop = iRealTime;
						CONT_ITEM_CREATE_ORDER::const_iterator item_itr = MyInfo.kResultItemList.begin();
						for ( ; item_itr != MyInfo.kResultItemList.end() ; ++item_itr )
						{
							if ( iLoop-- == 0 )
							{
								PgBase_Item const &kItem = *item_itr;

								BM::Stream kCustomData;
								kItem.WriteToPacket(kCustomData);
								pkIcon->SetCustomData( kCustomData.Data() );
							}
						}

						XUI::CXUI_Wnd * pkRate = pkForm_Left->GetControl(L"FRM_PARTICIPATION_RATE");
						if(pkRate)
						{
							BM::vstring vStr(TTW(401104));
							vStr.Replace(L"#RATE#", static_cast<int>(MAKE_ABIL_RATE(MyInfo.fParticipationRate)));
							pkRate->Text(vStr);
						}

// 						int iRewardItem = MyInfo.iRewardItem[iRealTime];
// 						if (iRewardItem==0)
// 						{ 
// 							iRewardItem = 20400217;
// 						}
// 						pkIcon->SetCustomData(&iRewardItem, sizeof(iRewardItem));

						lwDrawMissionRewardItem(lwUIWnd(pkIcon, pkIcon != NULL));
						char const * pkSoundName = lua_tinker::call<char const*>("GetRewardItemOpenSoundName");	//캐릭터 고정 해제
						PlaySound(pkSoundName);
						m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);//계속 업데이트 시켜주면 카메라 흔들어도 못느낌
						lua_tinker::call<bool, float, float, int, float, int>("QuakeCamera", 0.4f, 1,0,0,0);	//화면 흔들림
					}
				}
			}
			else
			{
				// 화면에 아이콘들 다 표시되면 종료버튼 보여주기
				g_kChatMgrClient.FlushDelayLog(CIE_Mission);

				m_pkWnd->GetControl(std::wstring(L"SFRM_EXIT_BG"))->Visible(true);
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_REWARD_WAIT);
			}
		}break;
	case E_MSN_CPL_REWARD_WAIT: 
		{
			m_spBackGround->Update(0);

			//if (m_pkWndUIModel->IsAnimationDone())
			//{
			//	m_pkWndUIModel->SetTargetAnimation(2000035);
			//}

			if (fFlowTime > UI_CLOSE_TIME)
			{
				m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
				m_spBackGround->Update(0);
				//m_pkWnd->GetControl(std::wstring(L"SFRM_EXIT_BG"))->Visible(false);
				TimeStamp(fAccumTime);
				//	CompleteState(E_MSN_CPL_END);
			}
		}break;
	case E_MSN_CPL_END:
		{
			XUI::CXUI_Wnd *pkForm_Left = m_pkWnd->GetControl(std::wstring(L"SFRM_MY_INFO"));
			XUI::CXUI_Wnd *pkForm_Right = m_pkWnd->GetControl(std::wstring(L"SFRM_OTHER_INFO"));
			XUI::CXUI_Wnd *pkForm_Title = m_pkWnd->GetControl(std::wstring(L"FRM_RESULT_TITLE"));

			if (fFlowTime < ENDING_TIME)
			{	
				if (fFlowTime < ENDING_UI_TIME)
				{
					if (pkForm_Left)
					{
						pkForm_Left->Location(-(fFlowTime/ENDING_UI_TIME)*pkForm_Left->Size().x, pkForm_Left->Location().y);
					}

					if (pkForm_Right)
					{
						pkForm_Right->Location(XUIMgr.GetResolutionSize().x-(1-fFlowTime/ENDING_UI_TIME)*pkForm_Right->Size().x, pkForm_Right->Location().y);
					}

					if (pkForm_Title)
					{
						pkForm_Title->Location(pkForm_Title->Location().x, pkForm_Title->Location().y-(fFrameTime/ENDING_UI_TIME)*UI_TITLE_MOVE_LENGTH);
					}
				}
				else
				{
					if (pkForm_Left)
					{
						pkForm_Left->Visible(false);
					}

					if (pkForm_Right)
					{
						pkForm_Right->Visible(false);
					}

					if (pkForm_Title)
					{
						pkForm_Title->Visible(false);
					}
				}

				SetAlphaByRecursiveProcess(m_spBackGround, 1-fFlowTime/ENDING_TIME);
			}
			else
			{
				if (pkForm_Left)
				{
					pkForm_Left->Location(-pkForm_Left->Size().x, pkForm_Left->Location().y);
					pkForm_Left->Visible(false);
				}

				if (pkForm_Right)
				{
					pkForm_Right->Location(XUIMgr.GetResolutionSize().x-1, pkForm_Right->Location().y);
					pkForm_Right->Visible(false);
				}

				if (pkForm_Title)
				{
					pkForm_Title->Location(pkForm_Title->Location().x, 1-pkForm_Title->Size().y);
					pkForm_Title->Visible(false);
				}

				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_NONE);
				g_kSpeed = PastCamSpeed();
				Init();
				PgEnergyGauge::ms_bDrawEnergyGaugeBarHide = true;
				lua_tinker::call<void, bool>("LockGlobalHotKey", false);	//글로벌 핫키 고정 고정
				g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정 해제 
				g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정 해제 
				lwUIOn();
				XUIMgr.BlockGlobalScript(false);

				BM::Stream kEndPacket( PT_C_M_REQ_RESULT_END );
				NETWORK_SEND(kEndPacket);

				if (m_pkWnd)
				{
					m_pkWnd->Close();
				}

				MemberCount(0);
				PgCameraMan* pkCamMan = g_pkWorld->GetCameraMan();
				if (pkCamMan)
				{
					pkCamMan->LockInput(false);
				}
				//g_kUIScene.RemoveModel(g_kPilotMan.GetPlayerActor()->GetID().c_str());
				if (m_pkWndModel)
				{
					m_pkWndModel->Close();
				}
			}
			if (m_spBackGround)
			{
				m_spBackGround->SetTranslate(CamWorldPos() + kDir*BG_COME_LENGTH);
				m_spBackGround->UpdateProperties();
				m_spBackGround->Update(0);
			}
		}

		break;
	}

	return true;
}

void PgBossComplete::Draw(PgRenderer *pkRenderer, float fFrameTime)
{
	if (E_MSN_CPL_NONE == CompleteState() || !pkRenderer || !g_pkWorld) { return; }

	pkRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_ZBUFFER);

	pkRenderer->SetCameraData(g_pkWorld->GetCameraMan()->GetCamera());


	NiDX9Renderer *pDX9Renderer = (NiDX9Renderer*)pkRenderer->GetRenderer();

	switch(CompleteState())
	{
	case E_MSN_CPL_BG:
		{
			PgEnergyGauge::ms_bDrawEnergyGaugeBarHide = false;
			pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
		}break;	
	case E_MSN_CPL_DROP:
		{
			pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
			DrawBox(pkRenderer, fFrameTime);
		}break;
	case E_MSN_CPL_SELECT:	//선택 대기중
		{
			pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
			DrawBox(pkRenderer, fFrameTime);

			// 이름 Draw
			NiPoint3	spPos;
			for ( int i=0; i!=REWARD_BOX_COUNT; ++i )
			{
				spPos = m_kRewardBox[i].GetTranslate();
				spPos.z -= BOX_INTERVAL;
				m_vecBoxName[i].Draw_3DPos(pkRenderer, spPos,m_spCamera,NiColorA(1.0f, 1.0f, 0.0f, 1.0f ) );
			}
		}break;
	case E_MSN_CPL_MOVE_START:
	case E_MSN_CPL_MOVE:
		{
			pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
			DrawBox(pkRenderer, fFrameTime);
		}break;
	case E_MSN_CPL_OPEN:
	case E_MSN_CPL_LIGHT:
		{
			pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
			DrawBox(pkRenderer, fFrameTime);
			pkRenderer->PartialRenderClick_Deprecated(m_spWhiteBillboard);
		}break;

	case E_MSN_CPL_REWARD_DISPLAY:
	case E_MSN_CPL_REWARD_WAIT:
		pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
		/*if (m_pkWndModel)
		{
		PgActor *pkObj = g_kPilotMan.GetPlayerActor();
		if (pkObj)
		{
		g_kUIScene.RenderModel(pkObj->GetID().c_str(), m_pkWndModel->Size(), m_pkWndModel->TotalLocation(), pkObj->GetActorManager(), pkObj->GetUIModelUpdate());
		}
		}*/

		break;

	case E_MSN_CPL_REWARD:
	case E_MSN_CPL_END:
		pkRenderer->PartialRenderClick_Deprecated(m_spBackGround);
		break;
	default:
		break;
	}
	pkRenderer->EndUsingRenderTargetGroup();
}

void PgBossComplete::ReadFromPacket(BM::Stream &kPacket)
{
	int iNum = 0;
	kPacket.Pop(iNum);
	MemberCount(iNum);

	Start();

	for ( int i = 0; i < iNum; ++i)
	{
		m_kReward[i].ReadFromPacket(kPacket);
		if ( g_kPilotMan.IsMyPlayer( m_kReward[i].kCharGuid ) )
		{
			MyNumber(i);
		}
	}

	_PgOutputDebugString("PgBossComplete::ReadFromPacket %d\n",iNum);
}

void PgBossComplete::ReadFromPacketSelect(BM::Stream &kPacket)
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
	{
		return;
	}
	XUI::CXUI_Font *pFont = g_kFontMgr.GetFont(FONT_NAME);

	int iNum = -1;
	kPacket.Pop(iNum);
	if ( -1 <= iNum && REWARD_BOX_COUNT > iNum)
	{
		BM::GUID kCharGuid;
		kPacket.Pop( kCharGuid );
		if ( g_kPilotMan.IsMyPlayer( kCharGuid ) )
		{
			SelectBoxNum( iNum );
			m_kRewardBox[iNum].BoxState(PgRewardBox::E_REWORD_BOX_SELECT);
			PlaySound("MSC_Click");
		}

		for( int i=0; i!=REWARD_BOX_COUNT; ++i )
		{
			if ( m_kReward[i].kCharGuid == kCharGuid )
			{
				if(pFont)
				{
					m_vecBoxName[iNum].SetText(XUI::PgFontDef(pFont),m_kReward[i].wstrName );
				}
				break;
			}
		}
	}
	_PgOutputDebugString("PgBossComplete::ReadFromPacketSelect\n");
}
void PgBossComplete::ReadFromPacketOpen(BM::Stream &kPacket)
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
	{
		return;
	}
	CompleteState(E_MSN_CPL_MOVE_START);
	PlaySound("MSC_White_Out");
	_PgOutputDebugString("PgBossComplete::ReadFromPacketOpen\n");
}

bool PgBossComplete::UpdateBox(float fAccumTime, float fFrameTime)
{
	for(int i = 0; i < REWARD_BOX_COUNT; ++i)
	{
		m_kRewardBox[i].Update(fAccumTime, fFrameTime);
	}
	return true;
}
void PgBossComplete::DrawBox(PgRenderer *pkRenderer, float fFrameTime)
{
	for(int i = 0; i < REWARD_BOX_COUNT; ++i)	
	{
		m_kRewardBox[i].Draw(pkRenderer, fFrameTime);
	}
}

void PgBossComplete::Pickup(int const iValue)
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_FLAG_RESULT)) )
	{
		return;
	}
	if (E_MSN_CPL_SELECT != CompleteState())
	{
		return;
	}
	if ( SelectBoxNum() > -1 && SelectBoxNum() <= REWARD_BOX_COUNT )
	{
		return;
	}
	NiPoint3 kOrgPt;
	NiPoint3 kRayDir;
	POINT2 ptXUIPos = XUIMgr.MousePos();

	if (!m_spCamera)
	{
		return;
	}
	m_spCamera->WindowPointToRay(ptXUIPos.x, ptXUIPos.y, kOrgPt, kRayDir);
	for(int i = 0; i < REWARD_BOX_COUNT; ++i )
	{	
		// 박스가 선택가능한 상태일때만..
		if ( PgRewardBox::E_REWORD_BOX_SELECTING & m_kRewardBox[i].BoxState() )
		{
			if ( m_kRewardBox[i].Picked(kOrgPt, kRayDir) )
			{
				if ( 1001 == iValue )
				{
					BM::Stream kPacket(PT_C_M_NFY_SELECTED_BOX, i);
					NETWORK_SEND(kPacket)	//서버로 패킷 보내자
					_PgOutputDebugString("PT_C_M_NFY_SELECTED_BOX %d \n", i);
				}
				else
				{
					m_kRewardBox[i].BoxState(PgRewardBox::E_REWORD_BOX_OVER);
				}
			}
			else
			{
				m_kRewardBox[i].BoxState(PgRewardBox::E_REWORD_BOX_IDLE);
			}
		}
	}
}

bool PgBossComplete::PutMyInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit)
{
	if (!pkParentWnd) { return false; }

	XUI::CXUI_Wnd *pkMyForm = pkParentWnd->GetControl(std::wstring(L"SFRM_MY_INFO"));
	if (!pkMyForm) { return false; }
	std::wstring kstr;
	BM::vstring kVstr;
	SMissionRewardInfo MyInfo;
	if (!bInit)
	{
		MyInfo = m_kReward[MyNumber()];
	}
	else	// 초기화
	{
		XUI::CXUI_Wnd *pkIcon = NULL;
		for (int i = 0; i<MAX_REWARD_ITEM_COUNT; ++i )
		{
			BM::vstring kImgName(L"IMG_BONUS");
			kImgName+=i;
			pkIcon = pkMyForm->GetControl(std::wstring(L"FRM_ITEM_SDW"))->GetControl(kImgName);
			if (pkIcon)
			{
				int const kNull = 0;
				pkIcon->SetCustomData(&kNull, sizeof(kNull));
				pkIcon->DefaultImgTexture(NULL);
			}
		}
		pkMyForm->SetInvalidate();
	}

	kVstr = TTW(224) + L". ";
	kVstr+=MyInfo.iLevel;
	kVstr+=L" ";
	kVstr+= MyInfo.wstrName;

	pkMyForm->Text(kVstr);

	XUI::CXUI_Wnd *pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_NORMAL"))->GetControl(std::wstring(L"FRM_NUM"));
	if (pkWnd)//일반몹
	{
		kVstr = MyInfo.iNormalExp;
		pkWnd->Text(kVstr);
	}

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_STRENGTH"))->GetControl(std::wstring(L"FRM_NUM"));
	if (pkWnd)//강화몹
	{
		kVstr = MyInfo.iStrangthExp;
		pkWnd->Text(kVstr);
	}

	pkWnd = pkMyForm->GetControl(std::wstring(L"FRM_CRACK"))->GetControl(std::wstring(L"FRM_NUM"));
	if (pkWnd)//정예몹
	{
		kVstr = MyInfo.iCrackExp;
		pkWnd->Text(kVstr);
	}

	XUI::CXUI_Wnd *pkMyExpForm = pkMyForm->GetControl(std::wstring(L"SFRM_EXP"));
	if (pkMyExpForm)
	{	
		pkWnd = pkMyExpForm->GetControl(std::wstring(L"FRM_TOTAL_EXP"))->GetControl(std::wstring(L"FRM_NUM"));
		if (pkWnd)
		{
			kVstr = MyInfo.iGetExp;
			pkWnd->Text(kVstr);
		}

		pkWnd = pkMyExpForm->GetControl(std::wstring(L"FRM_CLEAR_EXP"))->GetControl(std::wstring(L"FRM_NUM"));
		if (pkWnd)
		{
			kVstr = MyInfo.iTotalExp;
			pkWnd->Text(kVstr);
		}
	}

	return true;
}

bool PgBossComplete::PutOtherInfoToUI(XUI::CXUI_Wnd* pkParentWnd, bool bInit)
{
	if (!pkParentWnd) { return false; }

	XUI::CXUI_Wnd *pkParentWnd2 = NULL;
	pkParentWnd2 = pkParentWnd->GetControl(std::wstring(L"SFRM_OTHER_INFO"));
	if (!pkParentWnd2)
	{
		return false;
	}

	BM::vstring kVstr;
	int kTempCount = 0;
	XUI::CXUI_Wnd *pkOtherForm = NULL;
	SMissionRewardInfo OhterInfo;

	for (int i = 0; i < PARTY_MEMBER_COUNT; ++i)
	{
		if (MyNumber() == i) {	continue; }

		kVstr = L"SFRM_OTHER_INFO";
		kVstr+=kTempCount;
		pkOtherForm = pkParentWnd2->GetControl(kVstr);
		if (pkOtherForm)
		{
			if (!bInit)
			{
				OhterInfo = m_kReward[i];
			}

			//bInit가 true 일 경우 OhterInfo가 초기화 됬다는 가정 하에 
			int iIndex = 0;
			XUI::CXUI_Wnd *pkIcon = NULL;

			CONT_ITEM_CREATE_ORDER::const_iterator item_itr = OhterInfo.kResultItemList.begin();
			for ( ;item_itr != OhterInfo.kResultItemList.end(); ++item_itr )
			{
				BM::vstring kImgName(L"SFRM_BONUS");
				kImgName += iIndex;
				pkIcon = pkOtherForm->GetControl(kImgName)->GetControl(std::wstring(L"IMG_BONUS"));

				PgBase_Item const &kItem = *item_itr;

				BM::Stream kCustomData;
				kItem.WriteToPacket(kCustomData);
				
				pkIcon->SetCustomData( kCustomData.Data() );
				lwDrawMissionRewardItem(lwUIWnd(pkIcon, pkIcon != NULL));
				++iIndex;
			}

			while ( iIndex < MAX_REWARD_ITEM_COUNT )
			{
				BM::vstring kImgName(L"SFRM_BONUS");
				kImgName += iIndex;
				pkIcon = pkOtherForm->GetControl(kImgName)->GetControl(std::wstring(L"IMG_BONUS"));

				lwDrawMissionRewardItem(lwUIWnd(pkIcon, pkIcon != NULL));
				++iIndex;
			}
// 			for (int j = 0; j<MAX_REWARD_ITEM_COUNT; ++j )
// 			{
// 				BM::vstring kImgName = L"SFRM_BONUS";
// 				kImgName+=j;
// 				pkIcon = pkOtherForm->GetControl(kImgName)->GetControl(std::wstring(L"IMG_BONUS"));
// 				if (pkIcon)
// 				{
// 					int kNull = OhterInfo.iRewardItem[j];
// 					if (kNull==0 && j < OhterInfo.iItemCount)
// 					{
// 						kNull = NULL_ITEM_NUM;
// 					}
// 					pkIcon->SetCustomData(&kNull, sizeof(kNull));
// 					lwDrawMissionRewardItem(lwUIWnd(pkIcon, pkIcon != NULL));
// 				}
// 			}

			kVstr = L""; 
			if (!bInit && OhterInfo.iLevel>0)
			{
				kVstr = TTW(224) + L". ";
				kVstr+=OhterInfo.iLevel;
				kVstr+= L" ";
				kVstr+= OhterInfo.wstrName;
			}

			pkOtherForm->Text(kVstr);	//레벨, 이름

			XUI::CXUI_Wnd *pExpForm = pkOtherForm->GetControl(std::wstring(L"FRM_TOTAL_EXP"))->GetControl(std::wstring(L"FRM_NUM"));
			if (pExpForm)
			{
				if (bInit || OhterInfo.iLevel<=0)
				{
					kVstr = L""; 
				}
				else
				{
					kVstr = OhterInfo.iGetExp;
				}

				pExpForm->Text(kVstr);
			}

			pExpForm = pkOtherForm->GetControl(std::wstring(L"FRM_CLEAR_EXP"))->GetControl(std::wstring(L"FRM_NUM"));
			if (pExpForm)
			{
				if (bInit || OhterInfo.iLevel<=0)
				{
					kVstr = L""; 
				}
				else
				{
					kVstr = OhterInfo.iTotalExp;
				}
				pExpForm->Text(kVstr);
			}
		}

		++kTempCount;
		if ( MemberCount() <= kTempCount)
		{
			break;	//더 돌 필요 없으니까 튕기자
		}

	}

	return true;
}

void PgBossComplete::SetAlphaByRecursiveProcess(NiAVObject* pkObject, float const fAlpha)
{
	if (NiIsKindOf(NiGeometry, pkObject))
	{
		NiGeometry	*pkGeom = NiDynamicCast(NiGeometry, pkObject);
		NiPropertyPtr pkProp = pkGeom->GetProperty(NiProperty::ALPHA);

		if (!pkProp)
		{
			pkGeom->AttachProperty(m_spNewAlphaProp);
			pkGeom->UpdateProperties();
			pkProp = pkGeom->GetProperty(NiProperty::ALPHA);
		}	
		if (pkProp)
		{
			NiMaterialPropertyPtr spAlpha = pkGeom->GetPropertyState()->GetMaterial();
			if (spAlpha)
			{
				spAlpha->SetAlpha(fAlpha);
				return;
			}
		}
	}

	if(NiIsKindOf(NiNode, pkObject))
	{
		NiNode* pkNode = (NiNode*)pkObject;
		for(unsigned int i = 0; i < pkNode->GetArrayCount(); i++)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				SetAlphaByRecursiveProcess(pkChild, fAlpha);
			}
		}
	}
}

bool PgBossComplete::PlaySound(char const *szPath)
{ 
	if (!szPath)
	{
		return false;
	}

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szPath, 0.0f,80,180,g_kPilotMan.GetPlayerPilot()->GetWorldObject());

	return true;
}


void PgBossComplete::CleanUp()
{
	CompleteState(E_MSN_CPL_NONE);

	if (m_pkWnd)
	{
		XUI::CXUI_Wnd *pkForm_Left = m_pkWnd->GetControl(std::wstring(L"SFRM_MY_INFO"));
		XUI::CXUI_Wnd *pkForm_Right = m_pkWnd->GetControl(std::wstring(L"SFRM_OTHER_INFO"));
		XUI::CXUI_Wnd *pkForm_Title = m_pkWnd->GetControl(std::wstring(L"FRM_RESULT_TITLE"));

		if (pkForm_Left)
		{
			pkForm_Left->Location(-pkForm_Left->Size().x, pkForm_Left->Location().y);
		}

		if (pkForm_Right)
		{
			pkForm_Right->Location(XUIMgr.GetResolutionSize().x-1, pkForm_Right->Location().y);
		}

		if (pkForm_Title)
		{
			pkForm_Title->Location(pkForm_Title->Location().x, 1-pkForm_Title->Size().y);
		}

		m_pkWnd->Close(); 
		m_pkWnd = NULL;

		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정 해제
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정 해제
	}

	MemberCount(0);
	if (g_pkWorld)
	{
		PgCameraMan* pkCamMan = g_pkWorld->GetCameraMan();
		if (pkCamMan)
		{
			pkCamMan->LockInput(false);
		}
	}

	if (m_pkWndModel)
	{
		m_pkWndModel->Close();
	}
}

//-----------------------------------------------------------------------------------------------//
//- ExpeditionComplete --------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------//

namespace PgExpeditionCompleteUtil
{
	void UpdateRewardInfo(lwUIWnd lwWnd)
	{
		XUI::CXUI_Form *pWnd = dynamic_cast<XUI::CXUI_Form*>(lwWnd());
		if( NULL == pWnd )
		{
			return;
		}
		g_kExpeditionComplete.PutRewardInfoToUI(pWnd);
	}

	int GetTenderChance()
	{
		return g_kExpeditionComplete.TenderChance();
	}

	int GetCompleteState()
	{
		return g_kExpeditionComplete.GetCompleteState();
	}
	
	void StampEffectUI(lwUIWnd lwWnd, float const fFlowTime, float const fDuring)
	{
		XUI::CXUI_Form *pWnd = static_cast<XUI::CXUI_Form*>(lwWnd());
		if( NULL == pWnd )
		{
			return;
		}

		float fRate = (fFlowTime)/(fDuring);

		pWnd->Scale((5.0f - fRate*(5.0f)));
		if( fRate >= 0.9f )
		{
			pWnd->Scale(1.0f);
			return;
		}
		pWnd->ScaleCenter(POINT2F(0.5f, 0.5f));
		if( pWnd->Scale() <= 0.8f )
		{
			lua_tinker::call<bool, float, float, int, float, int>("QuakeCamera", 0.3f, 3.0, 0, 0, 0); // 화면 흔들림
			pWnd->Scale(1.1f);
		}
	}

	void Send_PT_C_N_REQ_TENDER_ITEM()
	{ // 아이템 입찰 요청
		if(!g_pkWorld)
		{
			return;
		}
		if ( false == PgClientExpeditionUtil::IsInExpedition() )
		{ // 원정대에 속해 있지 않으면 요청불가
			return;
		}
		if ( false == g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_GROUND) )
		{ // 던전 안이 아니면 요청불가
			return;
		}
		if ( false == g_kExpeditionComplete.IsHaveTenderChance() )
		{ // 입찰기회가 없으면 요청불가
			SChatLog ChatLog(CT_EVENT);
			std::wstring const& Message = TTW(720046);
			g_kChatMgrClient.AddLogMessage(ChatLog, Message, true);
			return;
		}
		if ( g_kExpeditionComplete.E_MSN_CPL_TENDER_SELECT != g_kExpeditionComplete.CompleteState() )
		{ // 입찰 선택 페이즈가 아니면
			return;
		}

		int MyChance = g_kExpeditionComplete.TenderChance();
		g_kExpeditionComplete.TenderChance(--MyChance);

		BM::GUID MyGuid;
		g_kPilotMan.GetPlayerPilotGuid(MyGuid);

		BM::Stream Packet(PT_C_N_REQ_TENDER_ITEM);
		Packet.Push(MyGuid);
		NETWORK_SEND(Packet);
		g_kExpeditionComplete.TenderPassButtonState(g_kExpeditionComplete.GetParentWnd(), true);
	}

	void Send_PT_C_N_REQ_GIVEUP_TENDER()
	{ // 아이템 포기 요청
		if(!g_pkWorld)
		{
			return;
		}
		if ( false == PgClientExpeditionUtil::IsInExpedition() )
		{ // 원정대에 속해 있지 않으면 요청불가
			return;
		}
		if ( false == g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_GROUND) )
		{ // 던전 안이 아니면 요청불가
			return;
		}
		if ( g_kExpeditionComplete.E_MSN_CPL_TENDER_SELECT != g_kExpeditionComplete.CompleteState() )
		{ // 입찰 선택 페이즈가 아니면
			return;
		}

		BM::GUID MyGuid;
		g_kPilotMan.GetPlayerPilotGuid(MyGuid);

		BM::Stream Packet(PT_C_N_REQ_GIVEUP_TENDER);
		Packet.Push(MyGuid);
		NETWORK_SEND(Packet);
		g_kExpeditionComplete.TenderPassButtonState(g_kExpeditionComplete.GetParentWnd(), true);
	}
}

PgExpeditionComplete::PgExpeditionComplete() :
m_spBackGround(0)
, m_spWhiteBillboard(0)
, m_spNewAlphaProp(0)
, m_spAlphaMaterial(0)
, m_pWnd(NULL)
, m_spCamera(NULL)
{
	bExitButtonDown = false;
	Init();
}

PgExpeditionComplete::~PgExpeditionComplete()
{
	Destroy();
}

void PgExpeditionComplete::Destroy()
{
	m_spBackGround = 0;
	m_spWhiteBillboard = 0;
	m_spAlphaMaterial = 0;
	m_spCamera = NULL;
	m_spNewAlphaProp = 0;
	m_pWnd = NULL;

	TimeStamp(0);
	CompleteState(E_MSN_CPL_NONE);
	MemberCount(0);
	CurrTender(-1); // 시작할때 바로 +1하기 때문에 -1로 초기화 한다.
	TenderTime(0);
	NameShuffleTime(0);
	ItemShuffleTime(0);

	m_kWinnerGuid.Clear();
	m_kWinnerName.clear();
	
	TenderChance(1);
	ResultSound(false);
	BestPlayerSound(false);

	TenderLimitTime(TENDER_LIMIT_TIME);

	m_BestInfo.Clear();
	m_RewardItemList.clear();
	m_TenderItemList.Clear();

	NextNameCount = 0;
}

void PgExpeditionComplete::Start()
{
	if ( !g_pkWorld || !(g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_GROUND)) )
	{
		return;
	}
	
	Init();
	CompleteState(E_MSN_CPL_WAIT);
	TimeStamp(g_pkWorld->GetAccumTime());

	m_spCamera = g_pkWorld->GetCameraMan()->GetCamera();

	lua_tinker::call<void, bool>("LockGlobalHotKey", true);	//글로벌 핫키 고정

	_PgOutputDebugString("PgExpeditionComplete::Start\n");
}

void PgExpeditionComplete::Init()
{
	m_pWnd = NULL;
	m_spCamera = NULL;
	CamWorldPos(NiPoint3(0,0,0));
	TimeStamp(0);
	CompleteState(E_MSN_CPL_NONE);
	MemberCount(0);
	CurrTender(-1); // 시작할때 바로 +1하기 때문에 -1로 초기화 한다.
	TenderTime(0);
	NameShuffleTime(0);
	ItemShuffleTime(0);

	m_kWinnerGuid.Clear();
	m_kWinnerName.clear();
	
	TenderChance(1);
	ResultSound(false);
	BestPlayerSound(false);

	TenderLimitTime(TENDER_LIMIT_TIME);

	m_BestInfo.Clear();
	m_RewardItemList.clear();
	m_TenderItemList.Clear();

	NextNameCount = 0;

	if (m_spBackGround == 0)
	{
		m_spBackGround = g_kNifMan.GetNif("../Data/4_Item/9_Etc/MissionMapCube/plan.nif");

		if (m_spBackGround)
		{
			m_spBackGround->SetScale(1.2f);
			m_spBackGround->UpdateNodeBound();
			m_spBackGround->UpdateProperties();
			m_spBackGround->UpdateEffects();
			m_spBackGround->Update(0);
		}
	}

	if (m_spWhiteBillboard == 0)
	{
		m_spWhiteBillboard = g_kNifMan.GetNif_DeepCopy("../Data/6_UI/billboard_quad.nif");
		if(m_spWhiteBillboard) 
		{
			NiGeometry	*pTextGeom = (NiGeometry*)m_spWhiteBillboard->GetObjectByName("Plane");
			if (pTextGeom)
			{
				NiTexturingProperty	*pProperty = pTextGeom->GetPropertyState()->GetTexturing();

				NiSourceTexturePtr spSimpleTextTex = g_kNifMan.GetTexture("../Data/6_UI/whitebg.tga");
				pProperty->SetBaseTexture(spSimpleTextTex);

				NiPropertyPtr spAlpha = pTextGeom->GetProperty(NiAlphaProperty::GetType());
				if (spAlpha)
				{
					m_spAlphaMaterial = pTextGeom->GetPropertyState()->GetMaterial();
					if (m_spAlphaMaterial)
					{
						m_spAlphaMaterial->SetAlpha(0.0f);
					}
				}

				m_spWhiteBillboard->SetScale(10.0f);
				m_spWhiteBillboard->UpdateProperties();
				m_spWhiteBillboard->UpdateEffects();
				m_spWhiteBillboard->Update(0);
			}
		}
	}
	//배경에 알파 프로퍼티가 없어서 하나 만들어놓자
	m_spNewAlphaProp = NiNew NiAlphaProperty;
	m_spNewAlphaProp->SetAlphaBlending(true);
	m_spNewAlphaProp->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
	m_spNewAlphaProp->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);

	_PgOutputDebugString("PgExpeditionComplete::Init\n");
}

bool PgExpeditionComplete::Update(float const fAccumTime, float const fFrameTime)
{
	if (E_MSN_CPL_NONE == CompleteState())
	{
		if(false==g_kEventScriptSystem.IsNowActivate())
		{
			XUIMgr.BlockGlobalScript(false);
		}
		g_kBalloonMan2D.Visible(true);
		return false;
	}
	if (NULL == g_pkWorld)
	{
		CompleteState(E_MSN_CPL_NONE);
		return false;
	}
	if ( !(g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_GROUND)) )
	{
		CleanUp();
		return false;
	}
	if (!g_pkWorld->GetCameraMan() || !g_pkWorld->GetCameraMan()->GetCamera())
	{
		CompleteState(E_MSN_CPL_NONE);
		return false;
	}
	PgPlayer * pPlayer = g_kPilotMan.GetPlayerUnit();
	if ( NULL == pPlayer )
	{
		CompleteState(E_MSN_CPL_NONE);
		return false;
	}
	if( false == pPlayer->IsAlive() )
	{
		CompleteState(E_MSN_CPL_NONE);
		return false;
	}

	NiQuaternion CurruntRot;
	if (!m_spCamera)
	{
		CompleteState(E_MSN_CPL_NONE);
		return false;
	}
	m_spCamera->GetRotate(CurruntRot);
	NiPoint3 Dir = m_spCamera->GetWorldDirection();
	Dir.Unitize();
	NiMatrix3 CamRot = m_spCamera->GetRotate();

	float const fFlowTime = fAccumTime - TimeStamp();

	XUI::CXUI_Wnd* pDead =	XUIMgr.Get(L"FRM_DEAD_MISSION");
	if (pDead)
	{
		pDead->Close();
	}

	switch(CompleteState())
	{
	case E_MSN_CPL_WAIT:
		{
			CamWorldPos(g_pkWorld->GetCameraMan()->GetCamera()->GetWorldTranslate());	//카메라가 이동되므로 매프레임 체크하자.

			g_pkWorld->SetShowWorldFocusFilter("../Data/5_Effect/9_Tex/EF_blackBG.tga", 0.0f, 0.5f, BG_FADE_TIME, false);	//화면 페이드

			//여기서 사운드 재생
			g_kSoundMan.StopBgSound(false);
			g_kSoundMan.PlayBgSound(1);

			m_spBackGround->SetScale(1.7f);
			m_spBackGround->SetTranslate(CamWorldPos() + Dir*3.0f);	//카메라 위치에서 약간 앞으로
			NiMatrix3 mat(m_spCamera->GetWorldDirection(),
				m_spCamera->GetWorldUpVector(), m_spCamera->GetWorldRightVector());
			m_spBackGround->SetRotate(CamRot.TransposeTimes(mat));
			m_spBackGround->Update(0);

			g_kBalloonMan2D.BalloonOff();	//말풍선은 모두 끄자
			g_kBalloonMan2D.Visible(false);	//말풍선 앞으로 그리지 않는다.(다 끝나면 true 해줘야됨)

			PastCamSpeed(g_kSpeed);					//현재 카메라 속도 저장
			g_kSpeed = NiQuaternion(0, 0, 0, 0);	//카메라 고정
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정
			g_kPilotMan.LockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정
			XUIMgr.Close(L"FRM_HCD_BOSS_ING");	// UI 끄기
			lwUIOff();
			XUIMgr.BlockGlobalScript(true);
			PgCameraMan* pCamMan = g_pkWorld->GetCameraMan();
			if (pCamMan)
			{
				pCamMan->ClearQuake();
				pCamMan->LockInput(true);
			}
			TimeStamp(fAccumTime);
			CompleteState(E_MSN_CPL_START);
		}break;
	case E_MSN_CPL_START:
		{
			if (fFlowTime >= BG_FADE_TIME)
			{
				PlaySound("c1_uizoomin"); // 효과음
				SetAlphaByRecursiveProcess(m_spBackGround, 1.0f);
				m_spBackGround->UpdateProperties();
				m_spBackGround->Update(0);
				ResizeBGByResolution();

				m_pWnd = XUIMgr.Call(L"FRM_EXPEDITION_COMPLETE");
				if (m_pWnd)
				{
					m_pWnd->Size(XUIMgr.GetResolutionSize().x, XUIMgr.GetResolutionSize().y);
					m_pWnd->Location(0,0);

					// 위치 초기화 및 윈도우 보이기
					XUI::CXUI_Wnd* pForm_Result = m_pWnd->GetControl(L"FRM_EXPEDITION_RESULT");
					if (pForm_Result)
					{
						pForm_Result->Location(XUIMgr.GetResolutionSize().x
								- ((XUIMgr.GetResolutionSize().x + pForm_Result->Size().x) / 2)
								, pForm_Result->Location().y);
						pForm_Result->Visible(false);
					}

					XUI::CXUI_Wnd* pForm_Best = m_pWnd->GetControl(L"FRM_EXPEDITION_BEST");
					if (pForm_Best)
					{
						pForm_Best->Location(XUIMgr.GetResolutionSize().x, pForm_Best->Location().y);
						pForm_Best->Visible(true);
						
						XUI::CXUI_Image* pImg_Best = dynamic_cast<XUI::CXUI_Image*>(pForm_Best->GetControl(L"IMG_BONUS"));
						if(pImg_Best)
						{
							pImg_Best->Visible(false);
						}
					}

					XUI::CXUI_Wnd* pForm_Reward = m_pWnd->GetControl(L"FRM_EXPEDITION_REWARD");
					if (pForm_Reward)
					{
						pForm_Reward->Location(XUIMgr.GetResolutionSize().x, pForm_Reward->Location().y);
						pForm_Reward->Visible(true);
					}
					
					XUI::CXUI_Wnd* pForm_Tender = m_pWnd->GetControl(L"FRM_EXPEDITION_TENDER");
					if (pForm_Tender)
					{
						pForm_Tender->Location(-pForm_Tender->Size().x, pForm_Tender->Location().y);
						pForm_Tender->Visible(true);
					}
					
					// 초기화
					PutBestPlayInfoToUI(m_pWnd);	//베스트플레이 세팅
					PutRewardInfoToUI(m_pWnd);		//아이템보상 세팅
					PutTenderInfoToUI(m_pWnd);		//특별보상 세팅
					TenderPassButtonState(m_pWnd, true);	//입찰 포기 버튼 비활성화
				}
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_WAIT_2);
			}
		}break;
	case E_MSN_CPL_WAIT_2:
		{ // TimeStamp를 초기화 시켜주기 위해서 만든 페이즈 (바로 넘어가면 애니메이션이 매끄럽지 못함)
			if(fFlowTime >= BG_COME_TIME)
			{
				m_spBackGround->SetTranslate(CamWorldPos() + Dir*BG_COME_LENGTH);
				m_spBackGround->Update(0);

				g_pkWorld->RemoveAllMonster(EMGRADE_NORMAL);	//일반 몬스터 모두 제거
				g_pkWorld->RemoveAllMonster(EMGRADE_UPGRADED);	//강화 몬스터 모두 제거

				g_pkWorld->SetShowWorldFocusFilter(false, "../Data/5_Effect/9_Tex/EF_blackBG.tga", 0.8f);	//다 움직였으면 화면을 가득 채우므로
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_MOVE_RESULT);
			}
			else
			{
				NiPoint3 Now = CamWorldPos() + Dir * fFlowTime / BG_COME_TIME * BG_COME_LENGTH;
				m_spBackGround->SetTranslate(Now);	// 항상 카메라를 따라다니도록	
				m_spBackGround->Update(0);
			}
		}break;
	case E_MSN_CPL_MOVE_RESULT:
		{ // RESULT 이미지가 등장하는 페이즈
			if (fFlowTime < RESULT_MOVE_END_TIME)
			{
				if (fFlowTime <= RESULT_MOVE_TIME)
				{
					if (m_pWnd)
					{
						XUI::CXUI_Wnd* pForm_Result = m_pWnd->GetControl(L"FRM_EXPEDITION_RESULT");
						if (pForm_Result)
						{
							pForm_Result->Visible(true);
							PgExpeditionCompleteUtil::StampEffectUI(pForm_Result, fFlowTime, RESULT_MOVE_TIME);
						}
					}
				}
				else
				{
					if( false == ResultSound() )
					{
						PlaySound("c2_result");
						ResultSound(true);
					}
				}
			}
			else
			{
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_BEST_MOVE);
				PlaySound("c3_bestplay");
			}
		}break;
	case E_MSN_CPL_BEST_MOVE:
		{ // BESTPLAY 창이 등장하는 페이즈
			if (fFlowTime < BESTPLAY_MOVE_TIME)
			{
				if (m_pWnd)
				{
					float const fRate = fFlowTime/BESTPLAY_MOVE_TIME;
					XUI::CXUI_Wnd* pForm_Best = m_pWnd->GetControl(L"FRM_EXPEDITION_BEST");
					if (pForm_Best)
					{
						pForm_Best->Location(XUIMgr.GetResolutionSize().x - fRate*pForm_Best->Size().x,
							pForm_Best->Location().y);
					}
				}
			}
			else
			{
				if (m_pWnd)
				{
					XUI::CXUI_Wnd* pForm_Best = m_pWnd->GetControl(L"FRM_EXPEDITION_BEST");
					if (pForm_Best)
					{
						pForm_Best->Location(XUIMgr.GetResolutionSize().x - pForm_Best->Size().x,
							pForm_Best->Location().y);
					}
				}
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_REWARD_MOVE);
			}
		}break;
	case E_MSN_CPL_REWARD_MOVE:
		{ // 기본 보상창이 등장하는 페이즈
			if (fFlowTime < REWARD_MOVE_TIME)
			{
				if (m_pWnd)
				{
					float const fRate = fFlowTime/REWARD_MOVE_TIME;
					XUI::CXUI_Wnd* pForm_Reward = m_pWnd->GetControl(L"FRM_EXPEDITION_REWARD");
					if (pForm_Reward)
					{
						pForm_Reward->Location(XUIMgr.GetResolutionSize().x - fRate*pForm_Reward->Size().x,
							pForm_Reward->Location().y);
					}
				}
			}
			else
			{
				if (m_pWnd)
				{
					XUI::CXUI_Wnd* pForm_Reward = m_pWnd->GetControl(L"FRM_EXPEDITION_REWARD");
					if (pForm_Reward)
					{
						pForm_Reward->Location(XUIMgr.GetResolutionSize().x - pForm_Reward->Size().x,
							pForm_Reward->Location().y);
					}
				}
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_BESTPLAY_DISPLAY);
			}
		}break;
	case E_MSN_CPL_BESTPLAY_DISPLAY:
		{ // 베스트 플레이어 마크를 찍어주는 페이즈
			if (fFlowTime < BESTPLAY_REWARD_TIME)
			{
				if (fFlowTime >= BESTPLAY_DISPLAY_TIME)
				{
					if (m_pWnd)
					{
						XUI::CXUI_Wnd* pForm_Best = m_pWnd->GetControl(L"FRM_EXPEDITION_BEST");
						if(pForm_Best)
						{
							XUI::CXUI_Image* pImg_Best = dynamic_cast<XUI::CXUI_Image*>(pForm_Best->GetControl(L"IMG_BONUS"));
							if(pImg_Best)
							{
								pImg_Best->Visible(true);
								PgExpeditionCompleteUtil::StampEffectUI(pImg_Best, fFlowTime - BESTPLAY_DISPLAY_TIME, BESTPLAY_DISPLAY_TIME);
							}
						}
					}
					if (fFlowTime > BESTPLAY_DISPLAY_TIME * 2)
					{
						if( false == BestPlayerSound() )
						{
							PlaySound("c4_tender_number");
							BestPlayerSound(true);
						}
					}
				}
			}
			else
			{
				PlaySound("c5_item_tender");
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_TENDER_MOVE);
			}
		}break;
	case E_MSN_CPL_TENDER_MOVE:
		{ // 특별보상 창이 등장하는 페이즈
			if (fFlowTime < TENDER_MOVE_TIME)
			{
				if (m_pWnd)
				{
					float const fRate = fFlowTime/TENDER_MOVE_TIME;
					XUI::CXUI_Wnd* pForm_Tender = m_pWnd->GetControl(L"FRM_EXPEDITION_TENDER");
					if (pForm_Tender)
					{
						pForm_Tender->Location((fRate-1.0f)*pForm_Tender->Size().x, pForm_Tender->Location().y);
					}
				}
			}
			else
			{
				if (m_pWnd)
				{
					XUI::CXUI_Wnd* pForm_Tender = m_pWnd->GetControl(L"FRM_EXPEDITION_TENDER");
					XUI::CXUI_Wnd* pForm_Exit = m_pWnd->GetControl(L"FRM_EXPEDITION_EXIT");
					if (pForm_Tender && pForm_Exit)
					{
						pForm_Tender->Location(0, pForm_Tender->Location().y);
						pForm_Exit->VAlign();
						pForm_Exit->Visible(true);
						XUI::CXUI_Button * pBtn_Exit = dynamic_cast<XUI::CXUI_Button*>(pForm_Exit->GetControl(L"BTN_EXIT"));
						if( pBtn_Exit )
						{
							pBtn_Exit->Disable(false);
						}
					}
					XUIMgr.Activate(L"ChatOut");	// 채팅창 보이기
					XUIMgr.Activate(L"CHATBAR");
				}
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_TENDER_ITEM_DISPLAY);
			}
		}break;
	case E_MSN_CPL_TENDER_WAIT:
		{ // 서버에서 패킷이 올때까지 대기
			if( m_kCurrTender >= m_TenderItemList.ItemCount - 1 ) // - 1을 해주는 이유는 ItemCount는 단순한 갯수이고 m_kCurrTender는 인덱스이기 때문에
			{ // 모든 특별 보상 입찰이 끝났다면
				if( m_pWnd )
				{
					XUI::CXUI_Wnd* pForm_Exit	= m_pWnd->GetControl(L"FRM_EXPEDITION_EXIT");
					if( pForm_Exit )
					{
						XUI::CXUI_Image * pImg = dynamic_cast<XUI::CXUI_Image*>(pForm_Exit->GetControl(L"IMG_BG_TIMER"));
						if( pImg )
						{ // 강제 로비 귀환까지 남은 시간 표시
							pImg->Visible(true);
						}
					}
				}
			}
			TenderTime(0);
			ItemShuffleTime(0);
			NameShuffleTime(0);
			TenderLimitTime(TENDER_LIMIT_TIME);
			TimeStamp(fAccumTime);
		}break;
	case E_MSN_CPL_TENDER_ITEM_DISPLAY:
		{
			// 아이템 셔플
			if( NULL == m_pWnd )
			{
				return false;
			}
			XUI::CXUI_Wnd* pForm_Tender = m_pWnd->GetControl(L"FRM_EXPEDITION_TENDER");
			if( pForm_Tender )
			{
				XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pForm_Tender->GetControl(L"LST_TENDER"));
				if( NULL == pList )
				{
					return false;
				}

				for( int i = 0; i < m_TenderItemList.ItemCount; ++i )
				{
					XUI::SListItem* pItem = pList->GetItemAt(i);	// 특별보상 리스트에서 현재 진행중인 아이템을 가져온다.
					if( pItem )
					{
						XUI::CXUI_Wnd* pWnd = pItem->m_pWnd;
						if( pWnd )
						{
							BM::vstring ImgName(L"IMG_BONUS");
							PgFormRouletteWnd * pIcon = dynamic_cast<PgFormRouletteWnd *>(pWnd->GetControl(ImgName));
							if( pIcon )
							{
								CONT_SPECIALITEM::const_iterator TenderItem_iter = m_TenderItemList.ResultItemList.begin();
								if( TenderItem_iter != m_TenderItemList.ResultItemList.end() )
								{
									for( int ItemCount = 0; ItemCount < i; ++ItemCount )
									{
										++TenderItem_iter;
									}
								}
								PgBase_Item const & TenderItem = TenderItem_iter->ResultItem;
								pIcon->SetRoulette( TenderItem_iter->BagGrpNo, TenderItem, 0);
								
								char const * pSoundName = lua_tinker::call<char const*>("GetRewardItemOpenSoundName");
								PlaySound(pSoundName);
							}
						}
					}
				}
			}
			TimeStamp(fAccumTime);
			CompleteState(E_MSN_CPL_TENDER_START);
		}
	case E_MSN_CPL_TENDER_START:
		{ // TENDER 아이템 등장 연출 시간
			if (fFlowTime >= TENDER_ITEM_SHOW)
			{
				if( NULL == m_pWnd )
				{
					return false;
				}
				XUI::CXUI_Wnd* pForm_Tender = m_pWnd->GetControl(L"FRM_EXPEDITION_TENDER");
				if( pForm_Tender )
				{
					XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pForm_Tender->GetControl(L"LST_TENDER"));
					if( NULL == pList )
					{
						return false;
					}

					for( int i = 0; i < m_TenderItemList.ItemCount; ++i )
					{
						XUI::SListItem* pItem = pList->GetItemAt(i);	// 특별보상 리스트에서 현재 진행중인 아이템을 가져온다.
						if( pItem )
						{
							XUI::CXUI_Wnd* pWnd = pItem->m_pWnd;
							if( pWnd )
							{
								BM::vstring ImgName(L"IMG_BONUS");
								PgFormRouletteWnd * pIcon = dynamic_cast<PgFormRouletteWnd *>(pWnd->GetControl(ImgName));
								if( pIcon )
								{ // 아이템 연출 멈추고
									pIcon->SetStopKeyState();
									pIcon->SetStopKey();
								}
							}
						}
					}
				}
				TimeStamp(fAccumTime);
				TenderTime(0);
				ItemShuffleTime(0);
				TenderLimitTime(TENDER_LIMIT_TIME);
				CompleteState(E_MSN_CPL_TENDER_WAIT);
			}
		}break;
	case E_MSN_CPL_TENDER_INIT:
		{
			TimeStamp(fAccumTime);
			TenderTime(0);
			ItemShuffleTime(0);
			TenderLimitTime(TENDER_LIMIT_TIME);
			CompleteState(E_MSN_CPL_TENDER_SELECT);
		}break;
	case E_MSN_CPL_TENDER_SELECT:
		{ //TENDER 입찰 선택 시간
			if (fFlowTime < TENDER_LIMIT_TIME + 1)
			{ // 입찰 제한시간 동안
				if( fFlowTime - TenderTime() >= 1 )
				{ // 1초가 지나면
					int LastTime = (int)(fFlowTime - TenderTime());
					TenderLimitTime(TenderLimitTime() - LastTime);		// 제한시간을 지난 시간만큼 감소시킨다.
					if( 0 >= TenderLimitTime() )
					{
						TenderLimitTime(0);
					}
					if( m_pWnd )
					{ // 제한시간을 업데이트
						UpdateTenderLimitTime(m_pWnd);
					}
					TenderTime(fFlowTime); // 마지막으로 비교할 시간을 저장한다.
				}
			}
			else
			{
				TenderLimitTime(0);
			}

			if( 5 >= TenderLimitTime() )
			{ // 남은 시간이 5초 이하면 당첨자 연출 시작한다.
				if( fFlowTime - NameShuffleTime() >= NAME_CHANGE_TIME )
				{
					if (m_pWnd)
					{
						NameShuffleTime(fFlowTime);
						UpdateTenderWinnerName(m_pWnd);
					}
				}
			}
			if( 0 >= TenderLimitTime() )
			{ // 남은시간이 없으면 당첨자 발표한다.
				TenderLimitTime(0);
				if( m_pWnd )
				{ // 제한시간을 업데이트
					UpdateTenderLimitTime(m_pWnd);
				}
				if( m_kWinnerName.empty() )
				{
					m_kWinnerName = L"";
				}
				if( g_kPilotMan.IsMyPlayer( m_kWinnerGuid ) )
				{ // 당첨자 본인에게만 연출
					NotifyEarnItem();
				}
				UpdateTenderWinnerName(m_pWnd, &m_kWinnerName); // 당첨자 이름으로 교체

				TimeStamp(fAccumTime);
				TenderTime(0);
				NameShuffleTime(0);
				TenderLimitTime(TENDER_LIMIT_TIME);
				CompleteState(E_MSN_CPL_TENDER_WAIT);
			}
		}break;
	case E_MSN_CPL_END_WAIT:
		{
			if (fFlowTime > ENDING_WAIT_TIME)
			{
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_END_MOVE);
			}
		}break;
	case E_MSN_CPL_END_MOVE:
		{
			if (fFlowTime < ENDING_TIME)
			{	
				if (fFlowTime <= ENDING_UI_MOVE_TIME)
				{
					if (m_pWnd)
					{
						XUI::CXUI_Wnd* pForm_Result = m_pWnd->GetControl(L"FRM_EXPEDITION_RESULT");
						XUI::CXUI_Wnd* pForm_Best	= m_pWnd->GetControl(L"FRM_EXPEDITION_BEST");
						XUI::CXUI_Wnd* pForm_Reward = m_pWnd->GetControl(L"FRM_EXPEDITION_REWARD");
						XUI::CXUI_Wnd* pForm_Tender = m_pWnd->GetControl(L"FRM_EXPEDITION_TENDER");
						XUI::CXUI_Wnd* pForm_Exit	= m_pWnd->GetControl(L"FRM_EXPEDITION_EXIT");
						if(pForm_Exit && pForm_Exit->Visible())
						{
							XUI::CXUI_Button * pBtn_Exit = dynamic_cast<XUI::CXUI_Button*>(pForm_Exit->GetControl(L"BTN_EXIT"));
							if( pBtn_Exit )
							{
								pBtn_Exit->Disable(true);
							}
							XUI::CXUI_Image * pImg = dynamic_cast<XUI::CXUI_Image*>(pForm_Exit->GetControl(L"IMG_BG_TIMER"));
							if( pImg )
							{
								pImg->Visible(false);
							}
							pForm_Exit->Visible(false);		// 나가기 버튼을 숨기고
						}
						float const fRate = fFlowTime/ENDING_UI_MOVE_TIME;
						if (pForm_Result)
						{
							pForm_Result->Location( ((XUIMgr.GetResolutionSize().x - pForm_Result->Size().x) / 2)
								- (fRate)*((XUIMgr.GetResolutionSize().x + pForm_Result->Size().x) / 2),
								pForm_Result->Location().y);
						}
						if (pForm_Best)
						{
							pForm_Best->Location((XUIMgr.GetResolutionSize().x - pForm_Best->Size().x)
								+ (fRate)*pForm_Best->Size().x, pForm_Best->Location().y);
						}
						if (pForm_Reward)
						{
							pForm_Reward->Location((XUIMgr.GetResolutionSize().x - pForm_Reward->Size().x)
								+ (fRate)*pForm_Reward->Size().x, pForm_Reward->Location().y);
						}
						if (pForm_Tender)
						{
							pForm_Tender->Location(-(fRate)*pForm_Tender->Size().x, pForm_Tender->Location().y);
						}
					}
				}
				else
				{
					if (m_pWnd)
					{
						XUI::CXUI_Wnd* pForm_Result = m_pWnd->GetControl(L"FRM_EXPEDITION_RESULT");
						XUI::CXUI_Wnd* pForm_Best = m_pWnd->GetControl(L"FRM_EXPEDITION_BEST");
						XUI::CXUI_Wnd* pForm_Reward = m_pWnd->GetControl(L"FRM_EXPEDITION_REWARD");
						XUI::CXUI_Wnd* pForm_Tender = m_pWnd->GetControl(L"FRM_EXPEDITION_TENDER");
						if (pForm_Result)
						{
							pForm_Result->Location(XUIMgr.GetResolutionSize().x
								- ((XUIMgr.GetResolutionSize().x + pForm_Result->Size().x) / 2)
								, pForm_Result->Location().y);
							pForm_Result->Visible(false);
						}
						if (pForm_Best)
						{
							pForm_Best->Location(XUIMgr.GetResolutionSize().x, pForm_Best->Location().y);
							pForm_Best->Visible(false);
							XUI::CXUI_Image* pImg_Best = dynamic_cast<XUI::CXUI_Image*>(pForm_Best->GetControl(L"IMG_BONUS"));
							if(pImg_Best)
							{
								pImg_Best->Visible(false);
							}
						}
						if (pForm_Reward)
						{
							pForm_Reward->Location(XUIMgr.GetResolutionSize().x, pForm_Reward->Location().y);
							pForm_Reward->Visible(false);
						}
						if (pForm_Tender)
						{
							pForm_Tender->Location(-pForm_Tender->Size().x, pForm_Tender->Location().y);
							pForm_Tender->Visible(false);
						}
					}
				}
				SetAlphaByRecursiveProcess(m_spBackGround, 1-fFlowTime/ENDING_TIME); // 배경 사라지게 하기
			}
			else
			{
				TimeStamp(fAccumTime);
				CompleteState(E_MSN_CPL_END);
			}
		}break;
	case E_MSN_CPL_END:
		{
			if (m_pWnd)
			{
				if (fFlowTime >= ENDING_TIME)
				{
					g_kSpeed = PastCamSpeed();
					PgEnergyGauge::ms_bDrawEnergyGaugeBarHide = true;
					lua_tinker::call<void, bool>("LockGlobalHotKey", false);	//글로벌 핫키 고정 해제
					g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정 해제 
					g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정 해제 
					lwUIOn();
					XUIMgr.BlockGlobalScript(false);

					m_pWnd->Close();

					PgCameraMan* pCamMan = g_pkWorld->GetCameraMan();
					if (pCamMan)
					{
						pCamMan->LockInput(false);
					}
					
					g_kBalloonMan2D.Visible(true);	// 말풍선 그리게 한다.

					Init();

					if ( bExitButtonDown )	// 나가기 버튼을 눌렀을 때
					{
						bExitButtonDown = false;	// Init()에 넣어두면 안된다.
						BM::Stream Packet(PT_C_N_REQ_EXIT_EXPEDITION_COMPLETE);
						NETWORK_SEND(Packet);
					}
				}
			}
		}break;
	}
	return true;
}

void PgExpeditionComplete::Draw(PgRenderer *pRenderer, float fFrameTime)
{
	if (E_MSN_CPL_NONE == CompleteState() || !pRenderer || !g_pkWorld)
	{
		return;
	}
	pRenderer->BeginUsingDefaultRenderTargetGroup(NiRenderer::CLEAR_ZBUFFER);
	pRenderer->SetCameraData(g_pkWorld->GetCameraMan()->GetCamera());
	NiDX9Renderer* pDX9Renderer = (NiDX9Renderer*)pRenderer->GetRenderer();

	switch(CompleteState())
	{
	case E_MSN_CPL_WAIT_2:
	case E_MSN_CPL_MOVE_RESULT:			//RESULT 이미지 이동
	case E_MSN_CPL_BEST_MOVE:			//BESTPLAY 윈도우 이동
	case E_MSN_CPL_REWARD_MOVE:			//REWARD 윈도우 이동
	case E_MSN_CPL_BESTPLAY_DISPLAY:	//BESTPLAY 연출
	case E_MSN_CPL_TENDER_MOVE:			//TENDER 윈도우 이동
	case E_MSN_CPL_TENDER_WAIT:			//특별보상 준비
	case E_MSN_CPL_TENDER_ITEM_DISPLAY:	//특별보상 아이템 연출
	case E_MSN_CPL_TENDER_START:		//TENDER 아이템 보여지는 시간
	case E_MSN_CPL_TENDER_INIT:			//특별보상 입찰 전 변수 초기화
	case E_MSN_CPL_TENDER_SELECT:		//TENDER 입찰 선택 시간
	case E_MSN_CPL_END_WAIT:			//마지막 대기시간
	case E_MSN_CPL_END_MOVE:			//창 원위치로 이동
		{
			pRenderer->PartialRenderClick_Deprecated(m_spBackGround);
		}break;
	case E_MSN_CPL_END:					//끝
		{
		}break;
	default:
		{
		}break;
	}
	pRenderer->EndUsingRenderTargetGroup();
}

void PgExpeditionComplete::CleanUp()
{
	if (m_pWnd)
	{
		XUI::CXUI_Wnd* pForm_Result = m_pWnd->GetControl(L"FRM_EXPEDITION_RESULT");
		XUI::CXUI_Wnd* pForm_Best = m_pWnd->GetControl(L"FRM_EXPEDITION_BEST");
		XUI::CXUI_Wnd* pForm_Reward = m_pWnd->GetControl(L"FRM_EXPEDITION_REWARD");
		XUI::CXUI_Wnd* pForm_Tender = m_pWnd->GetControl(L"FRM_EXPEDITION_TENDER");

		// 모든 결과창 관련 윈도우 위치 및 상태 초기화
		if (pForm_Result)
		{
			pForm_Result->Location(XUIMgr.GetResolutionSize().x
								- ((XUIMgr.GetResolutionSize().x + pForm_Result->Size().x) / 2)
								, pForm_Result->Location().y);
			pForm_Result->Visible(false);
		}
		if (pForm_Best)
		{
			pForm_Best->Location(XUIMgr.GetResolutionSize().x, pForm_Best->Location().y);
			pForm_Best->Visible(false);
			XUI::CXUI_Image* pImg_Best = dynamic_cast<XUI::CXUI_Image*>(pForm_Best->GetControl(L"IMG_BONUS"));
			if(pImg_Best)
			{
				pImg_Best->Visible(false);
			}
		}
		if (pForm_Reward)
		{
			pForm_Reward->Location(XUIMgr.GetResolutionSize().x, pForm_Reward->Location().y);
			pForm_Reward->Visible(false);
		}
		if (pForm_Tender)
		{
			pForm_Tender->Location(-pForm_Tender->Size().x, pForm_Tender->Location().y);
			pForm_Tender->Visible(false);
		}

		g_kSpeed = PastCamSpeed();
		PgEnergyGauge::ms_bDrawEnergyGaugeBarHide = true;
		lua_tinker::call<void, bool>("LockGlobalHotKey", false);	//글로벌 핫키 고정 해제
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, true));	//캐릭터 고정 해제 
		g_kPilotMan.UnlockPlayerInput(SReqPlayerLock(EPLT_MissionResult, false));	//캐릭터 고정 해제 
		lwUIOn();
		XUIMgr.BlockGlobalScript(false);

		if (g_pkWorld)
		{
			PgCameraMan* pCamMan = g_pkWorld->GetCameraMan();
			if (pCamMan)
			{
				pCamMan->LockInput(false);
			}
		}
		
		m_pWnd->Close();
		Init();
		Destroy();	// 릴리즈
	}
}

void PgExpeditionComplete::SetExitButtonDown(bool const bValue)
{
	bExitButtonDown = bValue;
}

bool PgExpeditionComplete::IsHaveTenderChance()
{
	bool bRet = ( m_kTenderChance > 0 );
	return bRet;
}

void PgExpeditionComplete::PutBestPlayInfoToUI(XUI::CXUI_Wnd* pParentWnd)
{
	if( NULL == pParentWnd )
	{
		return;
	}
	XUI::CXUI_Wnd* pForm_Best = pParentWnd->GetControl(L"FRM_EXPEDITION_BEST");
	if( pForm_Best )
	{
		XUI::CXUI_Wnd* pForm_Team = pForm_Best->GetControl(L"FRM_BESTTEAM");
		XUI::CXUI_Wnd* pForm_Player = pForm_Best->GetControl(L"FRM_BESTPLAYER");
		XUI::CXUI_Wnd* pForm_Supporter = pForm_Best->GetControl(L"FRM_BESTSUPPORTER");
		if( pForm_Team )
		{
			XUI::CXUI_Wnd* pTeam = pForm_Team->GetControl(L"FRM_TEAM");
			if( pTeam )
			{
				if( m_BestInfo.BestTeamNum )
				{
					BM::vstring vStr(m_BestInfo.BestTeamNum);
					vStr += TTW(710027);
					pTeam->Text(std::wstring(vStr));
				}
			}
			XUI::CXUI_Wnd* pDP = pForm_Team->GetControl(L"FRM_DP");
			if( pDP )
			{
				if( m_BestInfo.DP_BestTeam )
				{
					BM::vstring vStr(TTW(710038));	// 텍스트 번호를 받아 내용에 접근해서 텍스트를 치환한다.
					vStr.Replace(L"#DP#", m_BestInfo.DP_BestTeam);
					pDP->Text(std::wstring(vStr));
				}
			}
		}
		if( pForm_Player )
		{
			XUI::CXUI_Wnd* pPlayer = pForm_Player->GetControl(L"FRM_PLAYER");
			if( pPlayer )
			{
				if( BM::GUID::NullData() != m_BestInfo.BestPlayerGuid )
				{
					SExpeditionMember const * BestPlayer = PgClientExpeditionUtil::GetMember(m_BestInfo.BestPlayerGuid);
					if( BestPlayer )
					{
						pPlayer->Text(BestPlayer->CharName);
					}
				}
			}
			XUI::CXUI_Wnd* pDP = pForm_Player->GetControl(L"FRM_DP");
			if( pDP )
			{
				if( m_BestInfo.DP_BestPlayer )
				{
					BM::vstring vStr(TTW(710038));	// 텍스트 번호를 받아 내용에 접근해서 텍스트를 치환한다.
					vStr.Replace(L"#DP#", m_BestInfo.DP_BestPlayer);
					pDP->Text(std::wstring(vStr));
				}
			}
		}
		if( pForm_Supporter )
		{
			XUI::CXUI_Wnd* pSupporter = pForm_Supporter->GetControl(L"FRM_SUPPORTER");
			if( pSupporter )
			{
				if( BM::GUID::NullData() != m_BestInfo.BestSupporterGuid )
				{
					SExpeditionMember const * BestPlayer = PgClientExpeditionUtil::GetMember(m_BestInfo.BestSupporterGuid);
					if( BestPlayer )
					{
						pSupporter->Text(BestPlayer->CharName);
					}
				}
				else
				{
					pSupporter->Text(L"");
				}
			}
			XUI::CXUI_Wnd* pHP = pForm_Supporter->GetControl(L"FRM_HP");
			if( pHP )
			{
				if( m_BestInfo.HP_BestSupporter )
				{
					BM::vstring vStr(TTW(710039));	// 텍스트 번호를 받아 내용에 접근해서 텍스트를 치환한다.
					vStr.Replace(L"#HP#", m_BestInfo.HP_BestSupporter);
					pHP->Text(std::wstring(vStr));
				}
				else
				{
					pHP->Text(L"");
				}
			}
		}
	}
}

void PgExpeditionComplete::PutRewardInfoToUI(XUI::CXUI_Wnd* pParentWnd)
{
	if( NULL == pParentWnd )
	{
		return;
	}
	XUI::CXUI_Wnd* pForm_Reward = pParentWnd->GetControl(L"FRM_EXPEDITION_REWARD");
	int SelectTeam = 0;
	if( pForm_Reward )
	{
		XUI::CXUI_CheckButton* pCheckButton1 = dynamic_cast<XUI::CXUI_CheckButton*>(pForm_Reward->GetControl(L"CHK_TEAM1"));
		XUI::CXUI_CheckButton* pCheckButton2 = dynamic_cast<XUI::CXUI_CheckButton*>(pForm_Reward->GetControl(L"CHK_TEAM2"));
		XUI::CXUI_CheckButton* pCheckButton3 = dynamic_cast<XUI::CXUI_CheckButton*>(pForm_Reward->GetControl(L"CHK_TEAM3"));
		XUI::CXUI_CheckButton* pCheckButton4 = dynamic_cast<XUI::CXUI_CheckButton*>(pForm_Reward->GetControl(L"CHK_TEAM4"));
		if( pCheckButton1 )
		{
			if( pCheckButton1->Check() )	// 1번 팀이 체크 상태일 때
			{
				SelectTeam = 1;
			}
		}
		if( pCheckButton2 )
		{
			if( pCheckButton2->Check() )	// 2번 팀이 체크 상태일 때
			{
				SelectTeam = 2;
			}
		}
		if( pCheckButton3 )
		{
			if( pCheckButton3->Check() )	// 3번 팀이 체크 상태일 때
			{
				SelectTeam = 3;
			}
		}
		if( pCheckButton4 )
		{
			if( pCheckButton4->Check() )	// 4번 팀이 체크 상태일 때
			{
				SelectTeam = 4;
			}
		}

		if( SelectTeam == 0 )	// 현재 선택중인 팀이 없으면 종료
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pForm_Reward->GetControl(L"LST_EXPEDITION_REWARD"));
		if( NULL == pList )
		{
			return;
		}

		pList->ClearList();

		if( false == PgClientExpeditionUtil::IsInExpedition() )	// 원정대가 아니면 수행하지 않음
		{
			return;
		}
		ContExpeditionMember::const_iterator iter;	// 원정대 멤버를 전부 순회하면서
		for( iter = g_kExpedition.GetMembers().begin(); iter != g_kExpedition.GetMembers().end(); ++iter )
		{
			if( NULL == (*iter) )
			{
				continue;
			}
			if( (*iter)->sTeam == SelectTeam )		// 원정대 멤버의 팀과 내가 선택한 팀 번호가 같으면
			{
				// 새로운 항목을 아이템보상 리스트에 등록
				XUI::SListItem* pItem = pList->AddItem(L"");
				if( pItem )
				{
					XUI::CXUI_Wnd* pWnd = pItem->m_pWnd;
					if( pWnd )
					{
						pWnd->OwnerGuid((*iter)->kCharGuid);// 리스트의 항목에 캐릭터 GUID를 등록한다.

						XUI::CXUI_Form* pID = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"FRM_USER_ID"));
						if( pID )
						{
							pID->Text((*iter)->CharName);	// 원정대원 아이디
						}

						XUI::CXUI_Form* pHighlight = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(L"FRM_HIGHLIGHT"));
						if( pHighlight )
						{
							BM::GUID MyGuid;
							g_kPilotMan.GetPlayerPilotGuid(MyGuid);	// 자신의 GUID를 가져와서
							if( (*iter)->kCharGuid == MyGuid )	// 아이템의 GUID와 비교한다.
							{
								pHighlight->Visible(true);		// 그리고 이 아이템이 자신의 것이면 테두리 ON
							}
							else
							{
								pHighlight->Visible(false);		// 자신의 것이 아니면 테두리 OFF
							}
						}

						ContRewardItem::const_iterator Item_iter;
						for( Item_iter = m_RewardItemList.begin(); Item_iter != m_RewardItemList.end(); ++Item_iter )
						{ // 기본 보상 아이템 정보 중에서
							if( (*iter)->kCharGuid == Item_iter->CharGuid )
							{ // 현재 리스트의 항목과 같은 GUID를 가진 아이템 정보를 찾았으면
								size_t ItemNum = Item_iter->ItemCount;
								if( MAX_EXPEDITION_REWARD_ITEM_COUNT < ItemNum )
								{ // 최대 6개까지만 된다.
									ItemNum = MAX_REWARD_ITEM_COUNT;
								}
								int Index = 0;
								CONT_ITEM_CREATE_ORDER::const_iterator ResultItem_iter = Item_iter->ResultItemList.begin();
								while( ResultItem_iter != Item_iter->ResultItemList.end() )									
								{
									BM::vstring ImgName(L"FRM_BONUS");
									ImgName += Index;
									XUI::CXUI_Form * pIcon = dynamic_cast<XUI::CXUI_Form*>(pWnd->GetControl(ImgName));
									if( pIcon )
									{
										XUI::CXUI_Image * pImage = dynamic_cast<XUI::CXUI_Image*>(pIcon->GetControl(L"IMG_BONUS"));
										if( pImage )
										{
											PgBase_Item const & MyItem = (*ResultItem_iter);

											BM::Stream CustomData;
											MyItem.WriteToPacket(CustomData);
											pImage->SetCustomData( CustomData.Data() );
											lwDrawMissionRewardItem(lwUIWnd(pImage, pImage != NULL));
										}
									}
									++Index;
									++ResultItem_iter;
								}
							}
						}
					}
				}
			}
		}
	}
}

void PgExpeditionComplete::PutTenderInfoToUI(XUI::CXUI_Wnd* pParentWnd)
{
	if( NULL == pParentWnd )
	{
		return;
	}
	XUI::CXUI_Wnd* pForm_Tender = pParentWnd->GetControl(L"FRM_EXPEDITION_TENDER");
	if( pForm_Tender )
	{
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pForm_Tender->GetControl(L"LST_TENDER"));
		if( NULL == pList )
		{
			return;
		}

		pList->ClearList();

		for(int i = 0; i < m_TenderItemList.ItemCount; ++i)	// 특별보상 갯수만큼
		{
			// 새로운 항목을 특별보상 아이템 리스트에 등록
			XUI::SListItem* pItem = pList->AddItem(L"");
		}
	}
	UpdateTenderInfoToUI(pParentWnd);
}

void PgExpeditionComplete::UpdateTenderInfoToUI(XUI::CXUI_Wnd* pParentWnd)
{
	if( NULL == pParentWnd )
	{
		return;
	}
	XUI::CXUI_Wnd* pForm_Tender = pParentWnd->GetControl(L"FRM_EXPEDITION_TENDER");
	if( pForm_Tender )
	{
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pForm_Tender->GetControl(L"LST_TENDER"));
		if( NULL == pList )
		{
			return;
		}

		// 특별 보상 창 스크롤 바 세팅
		if( 0 >= m_TenderItemList.ItemCount )
		{
			return;
		}
		int ItemSize = (int)pList->GetTotalItemLen() / m_TenderItemList.ItemCount;
		pList->DisplayStartPos( m_kCurrTender * ItemSize );
		pList->AdjustMiddleBtnPos();

		for(int i = 0; i < m_TenderItemList.ItemCount; ++i)
		{
			XUI::SListItem* pItem = pList->GetItemAt(i);	// 총 특별보상 아이템 갯수까지 한개씩 가져온다.
			if( pItem )
			{
				XUI::CXUI_Wnd* pWnd = pItem->m_pWnd;
				if( pWnd )
				{
					XUI::CXUI_Wnd* pOutLine = pWnd->GetControl(L"FRM_OUTLINE");
					XUI::CXUI_Wnd* pTenDigit = pWnd->GetControl(L"IMG_TIME_10");
					XUI::CXUI_Wnd* pFirstDigit = pWnd->GetControl(L"IMG_TIME_1");
					XUI::CXUI_Button* pNeed = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_NEED"));
					XUI::CXUI_Button* pPass = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_PASS"));
					if( pOutLine )
					{
						if( m_kCurrTender == i )	// 현재 진행중인 보상창이라면
						{
							pOutLine->Visible(true);	// 테두리 ON
							PlaySound("c6_item_select");
						}
						else
						{
							pOutLine->Visible(false);	// 테두리 OFF
						}
					}
					if( pTenDigit && pFirstDigit )
					{
						if( m_kCurrTender == i )	// 현재 진행중인 보상창이라면
						{
							pTenDigit->Visible(true);	// 십의 자리 ON
							pFirstDigit->Visible(true);	// 일의 자리 ON
						}
						else
						{
							pTenDigit->Visible(false);	// 십의 자리 OFF
							pFirstDigit->Visible(false);// 일의 자리 OFF
						}
					}
				}
			}
		}
	}
	UpdateTenderLimitTime(pParentWnd);	//제한시간 세팅
}

void PgExpeditionComplete::TenderPassButtonState(XUI::CXUI_Wnd* pParentWnd, bool const bDisable)
{
	if( NULL == pParentWnd )
	{
		return;
	}
	XUI::CXUI_Wnd* pForm_Tender = pParentWnd->GetControl(L"FRM_EXPEDITION_TENDER");
	if( pForm_Tender )
	{
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pForm_Tender->GetControl(L"LST_TENDER"));
		if( NULL == pList )
		{
			return;
		}
		
		for(int i = 0; i < m_TenderItemList.ItemCount; ++i)
		{
			XUI::SListItem* pItem = pList->GetItemAt(i);	// 총 특별보상 아이템 갯수까지 한개씩 가져온다.
			if( pItem )
			{
				XUI::CXUI_Wnd* pWnd = pItem->m_pWnd;
				if( pWnd )
				{
					XUI::CXUI_Button* pNeed = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_NEED"));
					XUI::CXUI_Button* pPass = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_PASS"));
					if( pNeed && pPass )
					{
						if( bDisable )
						{
							pNeed->Disable(true);	// 입찰버튼 비활성화
							pPass->Disable(true);	// 포기버튼 비활성화
						}
						else
						{
							if( m_kCurrTender == i )	// 현재 진행중인 보상창이라면
							{
								pNeed->Disable(false);	// 입찰버튼 활성화
								pPass->Disable(false);	// 포기버튼 활성화
							}
							else
							{
								pNeed->Disable(true);	// 입찰버튼 비활성화
								pPass->Disable(true);	// 포기버튼 비활성화
							}
						}
					}
				}
			}
		}
	}
}

void PgExpeditionComplete::UpdateTenderLimitTime(XUI::CXUI_Wnd* pParentWnd)
{
	if( NULL == pParentWnd )
	{
		return;
	}
	XUI::CXUI_Wnd* pForm_Tender = pParentWnd->GetControl(L"FRM_EXPEDITION_TENDER");
	if( pForm_Tender )
	{
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pForm_Tender->GetControl(L"LST_TENDER"));
		if( NULL == pList )
		{
			return;
		}

		XUI::SListItem* pItem = pList->GetItemAt(m_kCurrTender);	// 특별보상 리스트에서 현재 진행중인 아이템을 가져온다.
		if( pItem )
		{
			XUI::CXUI_Wnd* pWnd = pItem->m_pWnd;
			if( pWnd )
			{
				XUI::CXUI_Wnd* pTenDigit = pWnd->GetControl(L"IMG_TIME_10");
				XUI::CXUI_Wnd* pFirstDigit = pWnd->GetControl(L"IMG_TIME_1");
				XUI::CXUI_Button* pNeedBtn = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_NEED"));
				XUI::CXUI_Button* pPassBtn = dynamic_cast<XUI::CXUI_Button*>(pWnd->GetControl(L"BTN_PASS"));

				if( pTenDigit && pFirstDigit )
				{
					if( m_kTenderLimitTime > 0 )	// 제한시간이 0초 이상
					{
						if( m_kTenderLimitTime >= 10 )	// 남은 시간이 10초 이상이면
						{
							pTenDigit->UVUpdate( (m_kTenderLimitTime / 10) + 1 );	// 십의 자리 세팅
						}
						else
						{
							pTenDigit->UVUpdate( 1 );	// 10초 이하이면 십의 자리는 0
						}
						pFirstDigit->UVUpdate( (m_kTenderLimitTime % 10) + 1 );		// 일의 자리 세팅
					}
					else if( m_kTenderLimitTime <= 0 )
					{ // 제한시간이 0 이하이면
						pTenDigit->UVUpdate( 1 );		// 십의 자리 0
						pFirstDigit->UVUpdate( 1 );		// 일의 자리 0
					}

					if( pNeedBtn && pPassBtn )
					{
						if( m_kTenderLimitTime <= 5 )
						{ // 제한시간이 5초 이하일 때
							pNeedBtn->Disable(true);	// 입찰버튼 비활성화
							pPassBtn->Disable(true);	// 포기버튼 비활성화
						}
					}
				}
			}
		}
	}
}

void PgExpeditionComplete::UpdateTenderWinnerName(XUI::CXUI_Wnd* pParentWnd, std::wstring const * const pWinnerName)
{
	if( NULL == pParentWnd )
	{
		return;
	}
	if( false == PgClientExpeditionUtil::IsInExpedition() )	// 원정대가 아니면 수행하지 않음
	{
		return;
	}

	XUI::CXUI_Wnd* pForm_Tender = pParentWnd->GetControl(L"FRM_EXPEDITION_TENDER");
	if( pForm_Tender )
	{
		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pForm_Tender->GetControl(L"LST_TENDER"));
		if( NULL == pList )
		{
			return;
		}

		XUI::SListItem* pItem = pList->GetItemAt(m_kCurrTender);
		if( pItem )
		{
			XUI::CXUI_Wnd* pWnd = pItem->m_pWnd;
			if( pWnd )
			{
				XUI::CXUI_Wnd* pName = pWnd->GetControl(L"FRM_WINNER");
				if( pName )
				{
					BM::vstring vStr(TTW(710041));	// 당첨자 : #CHARNAME#
					if( NULL == pWinnerName )
					{ // 당첨자 이름이 없을 경우
						ContExpeditionMember const & Members = g_kExpedition.GetMembers();
						if( 0 == Members.size() )
						{
							return;
						}
						if( NextNameCount < Members.size() )
						{ // 다음 사람 이름으로 교체
							vStr.Replace(L"#CHARNAME#", Members[NextNameCount]->CharName);
							NextNameCount++;
						}
						else
						{
							NextNameCount = 0;
							vStr.Replace(L"#CHARNAME#", Members[NextNameCount]->CharName);
							NextNameCount++;
						}
					}
					else
					{ // 당첨자 있으면 그 이름으로 교체
						vStr.Replace(L"#CHARNAME#", pWinnerName->c_str());
					}
					pName->Text(vStr);
				}
			}
		}
	}
}

void PgExpeditionComplete::SetAlphaByRecursiveProcess(NiAVObject* pObject, float const fAlpha)
{
	if (NiIsKindOf(NiGeometry, pObject))
	{
		NiGeometry	*pGeom = NiDynamicCast(NiGeometry, pObject);
		NiPropertyPtr pProp = pGeom->GetProperty(NiProperty::ALPHA);

		if (!pProp)
		{
			pGeom->AttachProperty(m_spNewAlphaProp);
			pGeom->UpdateProperties();
			pProp = pGeom->GetProperty(NiProperty::ALPHA);
		}	
		if (pProp)
		{
			NiMaterialPropertyPtr spAlpha = pGeom->GetPropertyState()->GetMaterial();
			if (spAlpha)
			{
				spAlpha->SetAlpha(fAlpha);
				return;
			}
		}
	}

	if(NiIsKindOf(NiNode, pObject))
	{
		NiNode* pNode = (NiNode*)pObject;
		for(unsigned int i = 0; i < pNode->GetArrayCount(); i++)
		{
			NiAVObject* pChild = pNode->GetAt(i);
			if(pChild)
			{
				SetAlphaByRecursiveProcess(pChild, fAlpha);
			}
		}
	}
}

void PgExpeditionComplete::ResizeBGByResolution()
{
	if(!g_pkWorld)
	{
		return;
	}
	PgWorld::CompleteBGContainer Container;
	g_pkWorld->GetCompleteBG(Container);

	if (Container.empty()) // 일반 미션
	{
		char szName[255] = {0,};
		for (int i = 0; i < 4; ++i)
		{
			_snprintf(szName, 254, "../Data/4_Item/9_Etc/MissionMapCube/plan_0%d.dds", i+1);
			Container.insert(std::make_pair(i, std::string(szName)));
		}
	}

	PgWorld::CompleteBGContainerIter it = Container.begin();

	NiRenderer *pRenderer = NiRenderer::GetRenderer();
	assert(pRenderer);

	unsigned int uiWidth = pRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
	unsigned int uiHeight = pRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);

	float	fAspect = static_cast<float>(uiWidth)/static_cast<float>(uiHeight);

	int i = 0;
	while(it != Container.end())
	{
		char szName[255] = {0,};
		_snprintf(szName, 254, "Plane02:%d", i);
		NiGeometry	*pTextGeom = (NiGeometry*)m_spBackGround->GetObjectByName(szName);
		if (pTextGeom)
		{
			NiTexturingProperty	*pProperty = pTextGeom->GetPropertyState()->GetTexturing();
			if (pProperty)
			{
				NiSourceTexturePtr spSimpleTextTex = g_kNifMan.GetTexture((it->second).c_str());
				pProperty->SetBaseTexture(spSimpleTextTex);
			}

			NiGeometryData* pData = pTextGeom->GetModelData();
			NiPoint3* Point[4];
			for(int i = 0; i<4; ++i)
			{
				Point[i] = (pData->GetVertices() + i);
			}
			float fHeight = fabs(Point[0]->y - Point[1]->y)*0.75f;//가로세로 길이 같아서 새로 길이가 3/4로 계산되어야 함
			if(0.0f==fHeight)	//가로세로 길이가 같고 세로 길이는 변하지 않는다는 가정 하에
			{					//중앙이 0,0,0이라는 가정 하에
				fHeight = fabs(Point[2]->y - Point[3]->y);
			}

			float fwidth = fAspect*fHeight;
			for(int i = 0; i<4; ++i)
			{
				if(Point[i]->x)
				{
					Point[i]->x = 0 < Point[i]->x ? fwidth : -fwidth;
				}
			}
			
			pData->SetConsistency(NiGeometryData::MUTABLE);
			pData->MarkAsChanged(NiGeometryData::VERTEX_MASK);
		}
		++i;
		++it;
	}

	m_spBackGround->UpdateNodeBound();
	m_spBackGround->UpdateProperties();
	m_spBackGround->UpdateEffects();
	m_spBackGround->Update(0);
}

void PgExpeditionComplete::NotifyEarnItem() const
{ // 원정대 당첨자 본인에게 팡파레 알림
	if(m_kWinnerName.empty())
	{
		return;
	}

	std::wstring Message;
	Message = PgChatMgrUtil::ColorHexToString(0xFFFFFFFF) + m_kWinnerName + TTW(700033) + TTW(720047);
	// 행운 알림 UI가 떠있으면,
	XUI::CXUI_Wnd * pNoti = XUIMgr.Call(L"FRM_NOTIFY_EARN_INGACHA_ITEM");
	pNoti->Text( Message );
}

bool PgExpeditionComplete::PlaySound(char const *szPath)
{ 
	if (!szPath)
	{
		return false;
	}

	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, szPath, 0.0f,80,180,g_kPilotMan.GetPlayerPilot()->GetWorldObject());

	return true;
}

bool PgExpeditionComplete::RegisterWrapper(lua_State *pState)
{
	if( !pState )
	{
		return false;
	}
	using namespace lua_tinker;

	def(pState, "UpdateRewardInfo", PgExpeditionCompleteUtil::UpdateRewardInfo);
	def(pState, "GetTenderChance", PgExpeditionCompleteUtil::GetTenderChance);
	def(pState, "GetCompleteState", PgExpeditionCompleteUtil::GetCompleteState);
	def(pState, "Send_PT_C_N_REQ_TENDER_ITEM", PgExpeditionCompleteUtil::Send_PT_C_N_REQ_TENDER_ITEM);
	def(pState, "Send_PT_C_N_REQ_GIVEUP_TENDER", PgExpeditionCompleteUtil::Send_PT_C_N_REQ_GIVEUP_TENDER);
	return true;
}

bool PgExpeditionComplete::ProcessMsg(unsigned short const usType, BM::Stream& Packet)
{
	switch( usType )
	{
	case PT_N_C_NFY_EXPEDITION_BOSS_RESULT:			{ Recv_PT_N_C_NFY_EXPEDITION_BOSS_RESULT(Packet); }break;
	case PT_N_C_NFY_START_TENDER:					{ Recv_PT_N_C_NFY_START_TENDER(Packet); }break;
	case PT_N_C_NFY_ACCEPTANCE_USER:				{ Recv_PT_N_C_NFY_ACCEPTANCE_USER(Packet); }break;
	case PT_M_C_NFY_EXPEDITION_RESULT_END:			{ Recv_PT_M_C_NFY_EXPEDITION_RESULT_END(Packet); }break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

void PgExpeditionComplete::Recv_PT_N_C_NFY_EXPEDITION_BOSS_RESULT(BM::Stream& Packet)
{
	if ( g_pkWorld || (g_pkWorld->IsHaveAttr(GATTR_EXPEDITION_GROUND)) )
	{
		bool bIsSuccess = false;
		Packet.Pop( bIsSuccess );

		if ( true == bIsSuccess )
		{
			Start();

			m_BestInfo.ReadFromPacket(Packet);	// 베스트 플레이 정보 얻기

			Packet.Pop(m_kMemberCount);
			m_RewardItemList.resize(m_kMemberCount);

			for( int i = 0; i < m_kMemberCount; ++i )
			{ // 원정대원 수만큼 기본보상 정보 얻기
				m_RewardItemList[i].ReadFromPacket(Packet);
			}

			m_TenderItemList.ReadFromPacket(Packet); // 특별보상 아이템 리스트 정보 얻기

			if( true == g_kPilotMan.IsMyPlayer(m_BestInfo.BestPlayerGuid) )
			{ // 베스트 플레이어가 나라면 입찰기회 1 증가
				m_kTenderChance += 1;
			}
		}
		else
		{
			Net_RecentMapMove();
		}
	}
}

void PgExpeditionComplete::Recv_PT_N_C_NFY_START_TENDER(BM::Stream& Packet)
{ // 특별보상을 시작하라고 패킷이 오면
	if( E_MSN_CPL_END_MOVE == CompleteState()
		|| E_MSN_CPL_END == CompleteState() )
	{ // 결산창이 종료 되는 중이면 특별보상 시작 패킷 무시한다.
		return;
	}

	m_kCurrTender++; // 다음 특별보상 아이템을 진행하기 위한 변수를 증가시키고

	if( m_kCurrTender < m_TenderItemList.ItemCount )	// 다음 보상창이 유효하다면
	{
		if( m_pWnd )
		{
			UpdateTenderInfoToUI(m_pWnd);		// 다음 보상창을 활성화
		}
		TenderPassButtonState(m_pWnd, false);	// 입찰 포기 버튼 활성화
		CompleteState(E_MSN_CPL_TENDER_INIT);	// 다시 반복(입찰 시작으로 감)
	}
	else										// 더 이상 입찰할 특별보상이 없으면
	{
		CompleteState(E_MSN_CPL_END_WAIT);		// 종료 페이즈로 감
	}
}

void PgExpeditionComplete::Recv_PT_N_C_NFY_ACCEPTANCE_USER(BM::Stream& Packet)
{ // 당첨자 패킷을 받으면(당첨자는 잔여시간이 5초가 됐을 때나 모든 원정대원이 입찰 또는 포기를 했을 때 옴)
	if( E_MSN_CPL_END_MOVE == CompleteState()
		|| E_MSN_CPL_END == CompleteState() )
	{ // 결산창이 종료 되는 중이면 당첨자 패킷 무시한다.
		return;
	}
	Packet.Pop(m_kWinnerGuid);
	SExpeditionMember const * WinnerMember = PgClientExpeditionUtil::GetMember(m_kWinnerGuid);
	if( WinnerMember )
	{ // 당첨자 이름을 찾아서 저장한다.
		m_kWinnerName = WinnerMember->CharName;
	}
	if( m_kTenderLimitTime > 5 )
	{ // 입찰 남은 시간이 5초 이상일 때 당첨자가 왔다면 잔여시간을 강제로 5초로 만들어버린다.
		TenderLimitTime(6);
	}
	CompleteState(E_MSN_CPL_TENDER_SELECT);
}

void PgExpeditionComplete::Recv_PT_M_C_NFY_EXPEDITION_RESULT_END(BM::Stream& Packet)
{ // 결산을 종료하라고 패킷이 오면 종료 페이즈로 이동시킨다.
	CompleteState(E_MSN_CPL_END_WAIT);
}