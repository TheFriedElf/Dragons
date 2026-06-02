#pragma once

class PgWebLogin
{
public:
	PgWebLogin(void);
	~PgWebLogin(void);
public:
	bool Init(std::wstring const &strCmd);
	bool IsCorrectArg()const;
protected:
	CLASS_DECLARATION_S(std::wstring, NexonPassport);
	CLASS_DECLARATION_S(std::wstring, SessKey);
	CLASS_DECLARATION_S(std::wstring, ServerAddr);
	CLASS_DECLARATION_S(std::wstring, MacAddr);
	CLASS_DECLARATION_S(bool, IsDisconnectServer);
};

#define g_kWebLogin SINGLETON_STATIC(PgWebLogin)