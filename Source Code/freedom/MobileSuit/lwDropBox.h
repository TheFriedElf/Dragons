#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWDROPBOX_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWDROPBOX_H

#include "lwGUID.h"
#include "lwPacket.h"

class PgDropBox;

class lwDropBox
{
public:
	lwDropBox(PgDropBox *pkDropBox);
	
	//! 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//! 박스 자기자신의 Guid를 설정한다.
	void SetGuid(lwGUID kGuid);

	//! 박스 자기자신의 Guid를 반환한다.
	lwGUID GetGuid();

	//! OwnerGuid를 설정한다.
	void SetOwnerGuid(lwGUID kGuid);

	//! OwnerGuid를 반환한다.
	lwGUID GetOwnerGuid();

	//! PgDropBox로 바꿔준다.
	PgDropBox *lwDropBox::operator()();

	//! 드롭박스가 Nil인가!
	bool IsNil();

	//! 아이템을 하나 추가한다.
	bool AddItems(lwPacket kPacket);

	lwPoint3 GetPos();//	return the world position of this box

	//! 엑션을 구동한다.
	bool TransitAction(char const *pcActionName);

	//! 아이템 개수를 반환한다.
	int ItemCount();

	void SetNameColor(int iColor);
	void SetItemNum(int iNum);

	bool IsMine();
	bool IsMoney();
	int GetAbil(WORD const wAbil);

	bool SetOfferer(lwGUID kGuid, lwPoint3 kPos);

protected:
	PgDropBox *m_pkDropBox;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWDROPBOX_H