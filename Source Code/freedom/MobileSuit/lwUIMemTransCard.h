#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIMEMTRASCARD_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIMEMTRASCARD_H
#include "lwUI.h"
#include "lwGUID.h"
#include "lwPacket.h"
#include "PgScripting.h"

/*	
	해당 위치를 기억
		-위치 기억 버튼을 누르면(또는 단축 명령으로 기억)
			- 현재 위치 저장 패킷 보냄

			- 패킷 받음
				1. 됏을때
					-끗

				2. 안됏을때					
					PT_M_C_ANS_REG_PORTAL : HRESULT kError
						E_PORTAL_FULL            : 더이상 저장불가
						E_PORTAL_ALREADY_EXISTS  : 이미저장된 포탈
						E_NOT_FOUND_PORTAL       : 포탈 정보를 찾을수 없다
						E_CANNOT_SAVE            : 저장할수 없는 맵

			
		-미션, 던전, pvp등의 위치에서는 저장 불가능( 그냥 대필드 일때만?)
			-위치를 기억하였습니다 메세지 출력
		-

	포탈 아이템 사용시
		-기억한 위치 XXXXX로 이동합니다. 아이템을 사용 하시겠습니까? UI 출력
			
			- 해당 위치로 이동 요청 패킷 보냄
				끗
			
			-패킷 받아 처리
				1. 됏을때
					끗
				2. 안됏을때
					위에꺼랑 같으니 같은거 사용		

*/
namespace lwUIMemTransCard
{
	extern bool RegisterWrapper(lua_State *pkState);
	extern void lwSendRegMemPortalReq();	// 위치 등록 요청 패킷 전송
//	extern void lwSendUseMemPortalReq();	// 포탈 사용 요청 패킷 전송
};

class PgMemTransCard
{// 기억 전송 카드
public:
	PgMemTransCard();
	virtual ~PgMemTransCard();
	
	void SendRegMemPortalReq();		// 위치 등록 요청 패킷 전송 
	void SendUseMemPortalReq();		// 포탈 사용 요청 패킷 전송
	void SetCashItemPos(SItemPos const& kPos) { m_kCashItemPos = kPos; }
	// 요청 결과 패킷
	void RecvPortalReqResult(WORD const wPacketType, BM::Stream& rkPacket);
	
	void ClearAll();	
	bool GetSavedMapName(std::wstring& kMapName_out); // 맵 이름 얻어오기

protected:
	SItemPos m_kCashItemPos;
};

#define g_kMemTransCard SINGLETON_STATIC(PgMemTransCard)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIMEMTRASCARD_H