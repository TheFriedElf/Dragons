#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWBASEITEMSET_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWBASEITEMSET_H

class PgBaseItemSet;

class lwBaseItemSet
{
public:
	//! 생성자
	lwBaseItemSet(PgBaseItemSet *pkBaseItemSet);

	//! Wrapper Class로 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

	//! Null 체크
	bool IsNil();

	//! 일반 아이템의 경우 아이템은 하나
	int AddItem(int iItemNo, char const *pcIconPath);

	//! 셋트 아이템의 경우, 추가로 ItemNo를 더 넣어주어야 한다.
	void AddItemInfo(int iIndex, int iItemNo);

	//! 노출된 슬롯 개수를 설정한다.
	void SetExposedSlot(int iNbExposedSlot);

	//! 형변환 Operator
	PgBaseItemSet *operator()();

protected:
	PgBaseItemSet *m_pkBaseItemSet;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWBASEITEMSET_H