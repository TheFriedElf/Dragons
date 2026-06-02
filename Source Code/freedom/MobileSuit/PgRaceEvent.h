#ifndef FREEDOM_DRAGONICA_CONTENTS_COMMUNITYEVENT_PGRACEEVENT_H
#define FREEDOM_DRAGONICA_CONTENTS_COMMUNITYEVENT_PGRACEEVENT_H


class PgActor;
class PgAction;
namespace RACEEVENT
{
	typedef struct _tagRANK_MODIFIED_INFO
	{
		std::wstring wstrName;
		int iRecentPos;

		_tagRANK_MODIFIED_INFO() : wstrName(), iRecentPos(0) {}
		_tagRANK_MODIFIED_INFO(std::wstring& wstrName_new, int iRecentPos_new) : wstrName(wstrName_new), iRecentPos(iRecentPos_new) {}
		bool operator ==(_tagRANK_MODIFIED_INFO& rhs) { return (wstrName == rhs.wstrName && iRecentPos == rhs.iRecentPos); }
	}RANK_MODIFIED_INFO;

	bool ProcessMsg(unsigned short const usType, BM::Stream& Packet);//패킷 처리
	void Recv_PT_M_C_NFY_RACE_EVENT_READY(BM::Stream& Packet);
	void Recv_PT_M_C_NFY_RACE_EVENT_START(BM::Stream& Packet);
	void Recv_PT_M_C_NFY_RACE_EVENT_END(BM::Stream& Packet);
	void Recv_PT_M_C_NFY_RACE_RANK_INFO(BM::Stream& Packet);
	void Recv_PT_M_C_NFY_RACE_EVENT_GROUND_MOVE_COMPLETE(BM::Stream& Packet);

	bool CheckSkillEvent(PgActor* pkActor, PgAction *pkNextAction); //이벤트 내에서 스킬 사용에 제한을 두기 위해 체크하는 함수. true면 사용 가능하다는 의미.

}


#endif
