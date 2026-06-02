#ifndef FREEDOM_DRAGONICA_CONTENTS_COMMUNITYEVENT_PGCOMMUNITYEVENT_H
#define FREEDOM_DRAGONICA_CONTENTS_COMMUNITYEVENT_PGCOMMUNITYEVENT_H

#include "lwUI.h"
//#include "Lohengrin/dbtables.h"

class PgPlayer;

namespace lwCommunityEvent
{
	typedef enum eEventState : int
	{
		ES_ALL_CLEAR = 0,
		ES_NOT_ENTER_TIME,
		ES_CHECK_CONDITION,
		ES_MAX
	}EEventState;

	typedef std::vector<SEventScheduleData> CONT_SCHEDULE_DATA;
	typedef std::map<EEventState, CONT_SCHEDULE_DATA> CONT_CHECK_SCHEDULE;

	void RegisterWrapper(lua_State *pState);//루아 노출

	void SortEventList(CONT_CHECK_SCHEDULE & outCont);//이벤트 리스트 정렬
	void lwSetEventList(lwUIWnd lwWnd, char const* TypeName, char const* ModeName = NULL);//입장 UI에 이벤트 리스트 등록
	void lwSendReqCommunityEventEnter(int const EventNo);//입장 요청
	void lwSendReqEventProgressInfo();

	void CommunityEventsCheckResult(DWORD const& Result);//입장 체크 에러 처리

	bool ProcessMsg(unsigned short const usType, BM::Stream& Packet);//패킷 처리

	void Recv_PT_M_C_ANS_MOVE_EVENT_GROUND(BM::Stream& Packet);//맵이동 실패 시 처리하는 패킷
	void Recv_PT_M_C_NFY_COMMUNITY_EVENT_READY(BM::Stream& Packet);//이벤트가 준비상태가 됨
	void Recv_PT_M_C_NFY_COMMUNITY_EVENT_START(BM::Stream& Packet);//이벤트가 시작 됨
	void Recv_PT_M_C_NFY_COMMUNITY_EVENT_END(BM::Stream& Packet);//이벤트가 끝남
	void Recv_PT_M_C_NFY_EVENT_GROUND_MOVE_COMPLETE(BM::Stream& Packet);//이벤트 맵으로 이동완료 했다는 알림
	void Recv_PT_M_C_NFY_EVENT_GROUND_USER_COUNT_MODIFY(BM::Stream& Packet);//인원 변동시 알림
	void Recv_PT_M_C_NFY_REMAIN_TIME_MONSTER_GENERATE(BM::Stream& Packet);//몬스터 젠 남은 시간 알림
	void Recv_PT_M_C_ANS_EVENT_PROGRESS_INFO(BM::Stream& Packet);//이벤트 오픈 여부
}

namespace COMMUNITY_EVENT
{
	typedef enum tagEENTER_RESULT : DWORD
	{
		EER_OK						= 0x0000,
		EER_ERROR_LV_MIN			= 0x0001, //최소 레벨 제한에 걸렸을 때
		EER_ERROR_LV_MAX			= 0x0002, //최대 레벨 제한에 걸렸을 때
		EER_ERROR_PARTY_MIN			= 0x0004, //파티 최소 인원 제한에 걸렸을 때
		EER_ERROR_HUMAN_CLASS		= 0x0008, //인간족 클래스 제한에 걸렸을 때
		EER_ERROR_DRAGON_CLASS		= 0x0010, //용족 클래스 제한에 걸렸을 때
		EER_ERROR_EVENT_BEGIN_DAY	= 0x0020, //이벤트 시작 날짜에 걸렸을 때
		EER_ERROR_EVENT_END_DAY		= 0x0040, //이벤트 종료 날짜에 걸렸을 때
		EER_ERROR_DAY				= 0x0080, //요일에 걸렸을 때
		EER_ERROR_QUEST				= 0x0100, //퀘스트 필요 조건에 걸렸을 때
		EER_ERROR_ITEM				= 0x0200, //아이템 필요 조건에 걸렸을 때
		EER_ERROR_SYSTEM			= 0x8000, //시스템 내부적 에러
	}EENTER_RESULT;

	DWORD CheckCanEnter(PgPlayer const* pkPlayer, SEventScheduleData const& rkEventSchedule);
	void AttachTeamEffect(void);
	void DetachTeamEffect(void);

	bool GetEventSchedule(SEventScheduleData& rkOut);

	void OnPlayerDead(void);
};










#endif // FREEDOM_DRAGONICA_CONTENTS_COMMUNITYEVENT_PGCOMMUNITYEVENT_H