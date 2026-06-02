#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIFIRELOVE_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIFIRELOVE_H
#include "lwUI.h"
#include "lwGUID.h"
#include "lwPacket.h"
#include "PgScripting.h"

class PgFireLove
{
public:
	typedef struct tagFireloveSMSInfo
	{
		tagFireloveSMSInfo();
		tagFireloveSMSInfo(std::wstring const& kBoyName_in, std::wstring const& kGirlName_in, int iTime);
		std::wstring kBoyName;
		std::wstring kGirlName;
		int iDurationTime;
		SYSTEMTIME  kSMSRecvTime; 
	}SFireLoveSMSInfo;
	typedef std::deque<SFireLoveSMSInfo> CONT_FIRELOVE;

public:
	PgFireLove();
	~PgFireLove();
		
	bool PushDataAndSetToUI(XUI::CXUI_Wnd* const pkWnd, SFireLoveSMSInfo const& kInfo);
	bool PopAndRemainSmsShow();	
	void Clear();
private:
	bool SetDataToUI(XUI::CXUI_Wnd* const pkWnd) const;
	bool Empty() const;
	bool Pop();
private:
	CONT_FIRELOVE m_kCont;
};

#define g_kFireLove SINGLETON_STATIC(PgFireLove)

namespace lwUIFireLove
{
	extern bool RegisterWrapper(lua_State *pkState);
	extern bool Show_FireLoveRemainSMS();
	extern void Clear_FireLoveRemainSMS();
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIFIRELOVE_H