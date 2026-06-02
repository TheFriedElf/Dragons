#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIACTIVESTATUS_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIACTIVESTATUS_H

#include "BM/twrapper.h"

typedef enum
{
	E_ACTSTAT_NONE = 0,
	E_ACTSTAT_STR  = 1,
	E_ACTSTAT_INT  = 2,
	E_ACTSTAT_CON  = 3,
	E_ACTSTAT_DEX  = 4,
	E_ACTSTAT_MAX,
}eActiveStatus;

class lwUIWnd;

class PgUIActiveStatus
{
	typedef std::map<int, int> CONT_STAT;
public:
	PgUIActiveStatus();
	~PgUIActiveStatus();	

	void InitUIData();
	
	void CallUI();
	
	void UpdateUI();
	void UpdateElem(eActiveStatus const eType);	

	void ModifyStat(eActiveStatus const eType, bool const bInc);		
	
	bool SendModifyAddStatus();
	bool RecvPacket(BM::Stream& rkPacket);	

protected:
	CLASS_DECLARATION(int, m_iRemainStatus, RemainStatus);	// 남은 보너스 수치
	CLASS_DECLARATION(int, m_iStr, Str);					// 추가할 스탯 값
	CLASS_DECLARATION(int, m_iInt, Int);
	CLASS_DECLARATION(int, m_iDex, Dex);
	CLASS_DECLARATION(int, m_iCon, Con);	
};

#define g_kUIActiveStatus SINGLETON_STATIC(PgUIActiveStatus)

namespace lwUIActiveStatus
{
	void RegisterWrapper(lua_State *pkState);
	
	void InitUIData_AS();

	void Call_AS();
	void Update_AS();

	void ModifyStat_AS(int const iType, bool const bInc);

	bool SendModifyAddStatus_AS();

	bool UseInitStatusItem_AS(int const iType);
	bool UseInitAllStatusItem_AS();
	int  GetAddtionlaStatus_AS(int const iType);
	bool IsModifiedStatusInUI_AS();

	bool UseInitStatusItem(int const iItemNo);

};
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIACTIVESTATUS_H