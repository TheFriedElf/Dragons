#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWGUID_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWGUID_H

class lwGUID
{
public:
	lwGUID(char const *pcGuid);
	lwGUID(BM::GUID const &rkGuid);

	// 스크립팅 시스템에 등록한다.
	static bool RegisterWrapper(lua_State *pkState);

public:
	//! GUID를 변경한다.
	bool Set(char const *pcGuid);

	//! 진짜 GUID를 반환한다.
	BM::GUID &operator()();

	//! 같은 GUID인지 비교한다.
	bool IsEqual(lwGUID kGuid);

	char const*	GetString();
	void MsgBox(char const *Caption);

	void ODS();

	void Generate();

	bool IsNil()const;

	BM::GUID	GetGUID()const{return m_kGuid;}

protected:
	BM::GUID m_kGuid;
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWGUID_H