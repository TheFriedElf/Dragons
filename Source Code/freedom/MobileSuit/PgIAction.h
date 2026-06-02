#pragma once

#include "PgIXmlObject.h"

class PgActor;

class PgIAction : public PgIXmlObject
{
public:
	class PgSlot
	{
	public:
		PgSlot() : m_kSlotName(""), m_bLoop(0) {}

		std::string m_kSlotName;
		bool m_bLoop;

	};

	typedef std::vector<PgSlot> SlotContainer;
	typedef std::map<unsigned int, std::string> ParamContainer;
	

	//! 생성자
	PgIAction();
	PgIAction(PgIAction *pkSrcAction);

	//! Xml을 파싱한다.
	bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0);

	//!	이 액션으로 진입할 수 있는지 체크한다.
	bool CheckCanEnter(PgActor *pkActor, PgIAction *pkAction);

	//! FSM으로 진입한다.
	bool EnterFSM(PgActor *pkActor, PgIAction *pkAction);

	//! FSM에서 탈출한다.
	bool LeaveFSM(PgActor *pkActor, PgIAction *pkAction,bool bCancel=false);

	//! FSM 업데이트 한다.
	bool UpdateFSM(PgActor *pkActor, float fAccumTime, float fFrameTime);

	//! FSM으로 Animation Event를 넘긴다.
	bool EventFSM(PgActor *pkActor, const char *pcTextKey,NiActorManager::SequenceID seqID);

	//! 지정된 다음 엑션 이름을 반환한다.
	std::string &GetNextActionName();

	//! 액션이 시작되고 나서 시간이 얼마나 지났는지를 반환
	float GetElpasedTime();

	//! 지난 시간을 0초로 리셋
	void ResetElpasedTime();

	/// Setter And Getter
	//! 다음 엑션 이름을 지정한다.
	void SetNextActionName(const char *pcActionName);

	//!	브로드캐스트 할 것인지 셋팅
	void SetBroadCast(bool bEnable);

	//! 브로드 캐스트 해야 하는지를 반환
	bool GetDoNotBroadCast();

	//! Get FSM Scripting Function's Prefix
	std::string &GetFSMScript();

	//! Action Instance ID를 셋팅
	void SetActionInstanceID(int iInstanceID = 0);

	//! Action Instance ID를 얻어온다.
	int GetActionInstanceID();

	//! 이 액션에 대해서, 키가 눌러졌는지 떼어졌는지 체크 할 수 있다.
	bool GetEnable();

	//! 이 액션에 대해서 키가 눌러졌는지 떼어 졌는지 설정할 수 있다.
	void SetEnable(bool bEnable);

	//! Action의 슬롯 개수를 가져온다.
	int GetSlotCount();

	//! Action Type을 반환한다.
	std::string &GetActionType();

	//! 등록된 파라미터를 반환한다.
	const char* GetScriptParam(const std::string &kParamName);

	//! 현재 진행중인 Action의 슬롯 번호를 반환한다.
	int GetCurrentSlot();

	//! 다음 진행할 Action의 슬롯 번호를 셋팅한다.
	bool SetSlot(unsigned int iSlot);

	//! Action의 다음 Slot에 대한 진행을 시도한다.
	bool NextSlot();

	//! Action의 다음 Slot에 대한 진행을 시도한다.
	bool PrevSlot();

	//! 파라메터를 설정
	bool SetParam(unsigned int uiIndex, const char *pcParam);

	//! 파라메터를 가져옴
	const char *GetParam(unsigned int uiIndex);

	//! 앞의 적 타겟을 찾는다.
	PgActor *FindEnemy(float fRange);

	//! 이 액션에 대한 모니터링이 필요하면 True, 아니면 False를 반환
	bool NeedMonitor();

	//! 이 액션이 액터의 포지션에 변화를 주는 액션인가?
	bool CanChangeActorPos();

protected:
	
	//! 다음 액션 이름(현재 액션이 종료 되면, 이 액션으로 자동 전이한다)
	std::string m_kNextActionName;

	//! 액션 타입
	std::string m_kActionType;

	//! Slot Container (Ani에 대한 Slot이 들어있다)
	SlotContainer m_kSlotContainer;

	//! Parameter Container
	ParamContainer m_kParamContainer;

	//! Script
	std::string m_kFSMScript;

	//!	스크립트 실행시 참조될 파라메터들
	//ScriptParamMap	m_ScriptParamMap;

	//! 액션을 반복하여 플레이 할지 결정하는 기준
	bool m_bLoop;

	//! 이 액션에 대해 진입인지 탈출인지 알려준다.
	bool m_bEnable;

	//! 이 액션에 대해 Event 핸들러를 쓸지 안쓸지 체크
	bool m_bUseEvent;

	//! 이 액션이 액터의 포지션에 변화를 주는 액션인가?
	bool m_bCanChangeActorPos;

	//!	브로드캐스팅 하지 않을것인가?
	bool m_bDoBroadCast;

	//!	액션으로 Enter 할 수 있는지 체크하는 스크립트 함수를 호출 할 것인가?
	bool m_bCallCheckCanEnter;

	//! 현재 플레이 중인 애니메이션의 슬롯 번호
	unsigned int m_uiCurrentSlot;
};