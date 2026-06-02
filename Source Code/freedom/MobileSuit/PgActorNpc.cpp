#include "Stdafx.h"
#include "PgRenderer.h"
#include "PgActor.h"
#include "Pg2DString.h"
#include "PgSoundMan.h"
#include "PgQuestMan.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgParticle.h"
#include "PgParticleMan.h"
#include "PgNifMan.h"
#include "PgUiScene.h"
#include "PgActorNpc.h"
#include "PgAction.h"
#include "PgOption.h"
#include "PgNetwork.h"
#include "PgQuestMarkHelp.h"
#include "lwGuid.h"
#include "PgTextDialogMng.h"

int const iQuestAttachPos = 102781;
std::string const kHelpQuestMarkTexture("../Data/6_UI/main/mnHelpQst.tga");
bool g_kHelpQuestMark = true;

////
PgSayItemChecker::PgSayItemChecker(int const iPlayerLevel, const PgMyQuest *pkMyQuest)
	: m_iPlayerLevel(iPlayerLevel), m_pkMyQuest(pkMyQuest)
{
}

PgSayItemChecker::~PgSayItemChecker()
{
}

bool PgSayItemChecker::operator ()(const ContSayItem::value_type& rkItem) const
{
	if( rkItem.iMinLevel
	&&	m_iPlayerLevel < rkItem.iMinLevel )
	{
		return true;
	}

	if( rkItem.iMaxLevel 
	&&	m_iPlayerLevel > rkItem.iMaxLevel )
	{
		return true;//삭제
	}

	const ContQuestID &rkCompleteQuest = rkItem.kCompleteQuest;
	if( !rkCompleteQuest.empty() )
	{
		bool bRet = false;
		ContQuestID::const_iterator complete_iter = rkCompleteQuest.begin();
		while(rkCompleteQuest.end() != complete_iter)
		{
			bRet = m_pkMyQuest->IsEndedQuest(*complete_iter) || bRet;
			++complete_iter;
		}

		if( !bRet )//아무것도 완료 한것이 없으면
		{
			return true;//삭제
		}
	}

	const ContQuestID &rkNotCompleteQuest = rkItem.kNotCompleteQuest;
	if( !rkNotCompleteQuest.empty() )
	{
		ContQuestID::const_iterator notcomplete_iter = rkNotCompleteQuest.begin();
		while(rkNotCompleteQuest.end() != notcomplete_iter)
		{
			if( m_pkMyQuest->IsEndedQuest(*notcomplete_iter) )
			{
				return true;//삭제
			}
			++notcomplete_iter;
		}
	}

	return false;//삭제금지
}

//////////////////////////////////////////////////////////////////////////////

NiImplementRTTI(PgActorNpc, PgActor);

PgActorNpc::PgActorNpc()
{
	m_spNowSaySound = NULL;
	m_pkQuestMarkHelp = NULL;
}

void PgActorNpc::Terminate()
{
	PgActor::Terminate();

	//	여긴 MultiThread 로 호출되는 부분이다. 따라서, 이런 코드가 들어가면 매니저 내의 컨테이너가 깨질 가능성이 있다. 
	//	게다가 BeforeCleanUp 에서 이미 호출을 해 주기때문에 또다시 할 필요도 없다.leesg213
	//	g_kQuestMarkHelpMng.DestroyNode(m_pkQuestMarkHelp);	
	m_spNowSaySound = NULL;
}

void PgActorNpc::UpdatePhysX(float fAccumTime, float fFrameTime)
{
	PgAction* pkAction = GetAction();
	if(pkAction)
	{
		if(IsMeetFloor() && pkAction->GetActionOptionEnable(PgAction::AO_CAN_CHANGE_ACTOR_POS) == false)
		{
			return;
		}
	}

	PgActor::UpdatePhysX(fAccumTime,fFrameTime);
}

bool PgActorNpc::AddSayAction(const ContSayItem &rkVec)
{
	ContSayItem::const_iterator kSay_iter = rkVec.begin();
	while(rkVec.end() != kSay_iter)
	{
		m_kActionSay.AddSay(*kSay_iter);
		++kSay_iter;
	}

	return (rkVec.size() != 0);
}

bool PgActorNpc::ShowChatBaloon_ByItem(const SSayItem& rkItem)
{
	if( rkItem.iTTW )
	{
		std::wstring const kText = TTW(rkItem.iTTW);
		if( kText.size() )
		{
			ShowChatBalloon(CT_NORMAL, kText, (int)(rkItem.fUpTime*1000));
		}
	}

	std::string const kName = GetAction()->GetID();
	if( rkItem.kActionName.size() && kName != rkItem.kActionName)
	{
		const PgAction *pkAction = ReserveTransitAction(rkItem.kActionName.c_str());
	}

	const std::string& rkSoundID = rkItem.kSoundID;
	if( rkSoundID.size() )//사운드
	{
		if( !m_spNowSaySound || m_spNowSaySound->GetStatus() == NiAudioSource::DONE)//이전 사운드가 없어야 재생
		{
 			m_spNowSaySound = g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, rkSoundID.c_str(), 0.0f,0.0f,0.0f,this);
		}
	}
	return true;
}

bool PgActorNpc::Update(float fAccumTime, float fFrameTime)
{
	PgActor::Update(fAccumTime, fFrameTime);//Actor Update

	ESayActionStatus eRet = m_kActionSay.Update(fAccumTime, fFrameTime);
	if(SAS_Run == eRet)
	{
		SSayItem kItem;
		if(m_kActionSay.GetCur(kItem))
		{
			if( !IsHide() && !g_kPilotMan.IsHideBalloon())
			{
				return ShowChatBaloon_ByItem(kItem);
			}
		}
	}

	if( m_pkQuestMarkHelp )
	{
		NiNodePtr pkDummy = (NiNode*)GetObjectByName(ATTACH_POINT_STAR);
		if( pkDummy )
		{
			bool const bHaveHideAbil = ((GetPilot())? GetPilot()->IsHide() : false);
			m_pkQuestMarkHelp->Update(pkDummy->GetWorldTranslate(), (IsHide() || bHaveHideAbil || g_kPilotMan.IsHideBalloon()));
		}
	}
	return true;
}

bool PgActorNpc::OnClickSay()
{
	if ( true == IsActivate() )
	{
		ESayActionStatus eRet = m_kActionSay.OnClick();
		if( SAS_Run == eRet )
		{
			SSayItem kItem;
			if( m_kActionSay.GetCur(kItem) )
			{
				return ShowChatBaloon_ByItem(kItem);
			}
		}
	}
	return false;
}

bool PgActorNpc::OnClickTalk()	
{
	if ( true == IsActivate() )
	{
		ESayActionStatus const eRet = m_kActionSay.OnTalk();
		if( SAS_Run == eRet )
		{
			BM::Stream kPacket(PT_C_M_REQ_NPC_TALK);
			kPacket.Push(GetGuid());
			NETWORK_SEND(kPacket);

			SSayItem kItem;
			if( m_kActionSay.GetCur(kItem) )
			{
				std::wstring const &kPilotName = (GetPilot())? GetPilot()->GetName(): _T("");
				SFullScreenDialogInfo kInfo(kItem, kPilotName);
				g_kQuestMan.CallNewDialog(kInfo, NULL, NULL);
				return true;
			}
		}
		else
		{
			if( 0 != m_kActionSay.TextDialogsID() )
			{
				lwTextDialog::CallTextDialog(m_kActionSay.TextDialogsID(), lwGUID(GetPilotGuid()));
			}
		}
	}
	return false;
}

bool PgActorNpc::OnClickWarning()
{
	if ( true == IsActivate() )
	{
		ESayActionStatus const eRet = m_kActionSay.OnWarning();
		if( SAS_Run == eRet )
		{
			SSayItem kItem;
			if( m_kActionSay.GetCur(kItem) )
			{
				std::wstring const &kPilotName = (GetPilot())? GetPilot()->GetName(): _T("");
				SFullScreenDialogInfo kInfo(kItem, kPilotName);
				g_kQuestMan.CallWarningDialog(kInfo);
				return true;
			}
		}
	}
	return false;
}

bool PgActorNpc::IsActivate(void)const
{
	PgPilot *pkPilot = GetPilot();
	if ( pkPilot )
	{
		return 0 < pkPilot->GetAbil(AT_HP);
	}
	return false;
}

bool PgActorNpc::IsHaveTalk(void)const
{
	return ( IsActivate() && (0 < m_kActionSay.TalkCount() || 0 != m_kActionSay.TextDialogsID()) );
}

bool PgActorNpc::IsHaveWarning(void)const
{
	return ( IsActivate() && (0 < m_kActionSay.WarningCount()) );
}

bool PgActorNpc::checkVisible()
{
	bool bRet = PgActor::checkVisible();
	if (m_eInvisibleGrade > PgActor::VISIBLE && m_pkController)
	{
		NxActor* pkActor = m_pkController->getActor();
		if (pkActor && pkActor->isSleeping() == false)
			pkActor->putToSleep();
	}
	return bRet;
}

void PgActorNpc::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
{
	PgActor::DrawImmediate(pkRenderer, pkCamera, fFrameTime);
}

bool PgActorNpc::BeforeCleanUp()
{
	g_kQuestMarkHelpMng.DestroyNode(m_pkQuestMarkHelp);
	m_kActionSay.Clear();
	return PgActor::BeforeCleanUp();
}
void PgActorNpc::DoLoadingFinishWork()
{
	PgActor::DoLoadingFinishWork();
	g_kQuestMan.UpdateRealmQuestNpc(GetGuid());
}


//!	줄 수 있는 퀘스트 정보 업데이트
void	PgActorNpc::ClearQuestInfo()
{
	DetachFrom(iQuestAttachPos);
	g_kQuestMarkHelpMng.DestroyNode(m_pkQuestMarkHelp);
}

void	PgActorNpc::PopSavedQuestSimpleInfo()	//	퀘스트 매니저가 저장하고 있는 퀘스트 정보를 가져온다.
{
	PgPilot	*pkPilot = GetPilot();
	if( !pkPilot )
	{
		return;
	}

	const PgPlayer *pkPC = g_kPilotMan.GetPlayerUnit();
	if( !pkPC )
	{
		return;
	}

	int const iPlayerLevel = pkPC->GetAbil(AT_LEVEL);
	BM::GUID const &rkGuid = pkPilot->GetGuid();
	
	ContNpcQuestInfo kVec, kResultVec;
	size_t const iCnt = g_kQuestMan.PopNPCQuestInfo(rkGuid, kVec);

	int const iShowAllQuest = g_kGlobalOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_SHOW_ALL_QUEST);
	if( 0 != iShowAllQuest )
	{
		kResultVec.insert(kResultVec.end(), kVec.begin(), kVec.end());
	}
	else
	{
		//10레벨 이상 차이나는 시작가능한 퀘스트 표시 생략
		std::remove_copy_if(kVec.begin(), kVec.end(), std::back_inserter(kResultVec), SNPCQuestInfo::SPlayerLevelDiff(iPlayerLevel));
	}

	// 스크롤 퀘스트 마크 제거
	ContNpcQuestInfo::iterator new_end = std::remove_if( kResultVec.begin(), kResultVec.end(), SHideScrollQuestMark() );
	kResultVec.erase( new_end, kResultVec.end() );

	SQuestOrderByResult const eQuestState = PgQuestManUtil::QuestOrderByState(rkGuid, kResultVec);

	ClearQuestInfo();
	std::string kParticleName;
	int iHelpText = 0;

	bool bUseCustomMarker = false;
	SCustomQuestMarker kMarkerInfo;
	if( g_kQuestMan.CheckCustomQuestMarker(eQuestState.iQuestID, kMarkerInfo) )
	{
		switch( eQuestState.eState )
		{
		case QS_Begin:		
		case QS_Begin_Loop:	
		case QS_Begin_Tactics:
		case QS_Begin_Weekly:
		case QS_Begin_Story:
		case QS_Begin_Couple:
		case QS_Begin_Event:
			{
				kParticleName = kMarkerInfo.strBegin;
				iHelpText = kMarkerInfo.iBeginTTID;
				bUseCustomMarker = true;
			}break;
		case QS_End_Story:	
		case QS_End_Loop:	
		case QS_End_Tactics:
		case QS_End_Weekly:	
		case QS_End_Couple:	
		case QS_End_Event:	
			{ 
				kParticleName = kMarkerInfo.strEnd;
				iHelpText = kMarkerInfo.iEndTTID;
				bUseCustomMarker = true;
			}break;
		case QS_Ing:
			{
				kParticleName = kMarkerInfo.strIng;
				bUseCustomMarker = true;
			}break;
		case QS_Begin_NYet:
			{
				kParticleName = kMarkerInfo.strNotYet;
				bUseCustomMarker = true;
			}break;
		}
	}

	if( false == bUseCustomMarker )
	{
		switch(eQuestState.eState)
		{
		case QS_End:
			{
				kParticleName = "QuestNotify_Nice";
				iHelpText = 450018;
			}break;	
		case QS_Begin:
			{
				kParticleName = "QuestNotify_Help";
				iHelpText = 450017;
			}break;
		case QS_Ing:
			{
				kParticleName = "QuestNotify_Ing";
				iHelpText = 450022;
			}break;
		case QS_Begin_NYet:
			{
				kParticleName = "QuestNotify_NotYet";
				iHelpText = 450021;
			}break;
		case QS_Begin_Story:
			{
				kParticleName = "QuestNotify_HelpStory";
				iHelpText = 450015;
			}break;
		case QS_End_Story:
			{
				kParticleName = "QuestNotify_NiceStory";
				iHelpText = 450016;
			}break;
		case QS_Begin_Loop:
			{
				kParticleName = "QuestNotify_HelpLoop";
				iHelpText = 450019;
			}break;
		case QS_End_Loop:
			{
				kParticleName = "QuestNotify_NiceLoop";
				iHelpText = 450020;
			}break;
		case QS_Begin_Tactics:
			{
				kParticleName = "QuestNotify_HelpTactics";
				iHelpText = 450050;
			}break;
		case QS_End_Tactics:
			{
				kParticleName = "QuestNotify_NiceTactics";
				iHelpText = 450051;
			}break;
		case QS_Begin_Weekly:
			{
				kParticleName = "QuestNotify_HelpWeekly";
				iHelpText = 450055;
			}break;
		case QS_End_Weekly:
			{
				kParticleName = "QuestNotify_NiceWeekly";
				iHelpText = 450056;
			}break;
		case QS_Begin_Couple:
			{
				kParticleName = "QuestNotify_HelpCouple";
				iHelpText = 450008;
			}break;
		case QS_End_Couple:
			{
				kParticleName = "QuestNotify_NiceCouple";
				iHelpText = 450009;
			}break;
		case QS_Begin_Event:
			{
				kParticleName = "QuestNotify_CharCard_Help";
				iHelpText = 450006;
			}break;
		case QS_End_Event:
			{
				kParticleName = "QuestNotify_CharCard_Nice";
				iHelpText = 450007;
			}break;
		default:
			{
			}break;
		}
	}

	if( 10 < iPlayerLevel )//플레이어 10레벨 넘으면 도움말 끔
	{
		if( m_pkQuestMarkHelp )
		{
			g_kQuestMarkHelpMng.DestroyNode(m_pkQuestMarkHelp);
		}
		bool g_kHelpQuestMark = false;
	}
	else if( iHelpText )
	{
		if( !m_pkQuestMarkHelp )
		{
			m_pkQuestMarkHelp = g_kQuestMarkHelpMng.CreateNode();
		}

		std::wstring const kHelpText = TTW(iHelpText);
		if( m_pkQuestMarkHelp
		&&	!kHelpText.empty() )
		{
			m_pkQuestMarkHelp->SetHelpText(kHelpText, kHelpQuestMarkTexture);
		}
	}
	
	if( kParticleName.empty() )
	{
		return ;
	}
	NiAVObject *pkParticle = g_kParticleMan.GetParticle(kParticleName.c_str());
	if( !pkParticle )
	{
		PG_ASSERT_LOG(pkParticle);
		return;
	}

	if( !GetObjectByName(ATTACH_POINT_STAR) )
	{
		return;
	}

	if(!AttachTo(iQuestAttachPos, ATTACH_POINT_STAR, NiDynamicCast(NiAVObject, pkParticle)))
	{
		THREAD_DELETE_PARTICLE(pkParticle);
	}
}