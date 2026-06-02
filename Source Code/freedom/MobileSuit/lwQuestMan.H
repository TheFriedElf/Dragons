#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWQUESTMAN_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWQUESTMAN_H

#include "lwUI.h"

class PgQuestMan;
class lwQuestMan
{
public:

	//! 생성자
	lwQuestMan(PgQuestMan *pQuestMan);

	//! Wrapper를 등록한다.
	static bool RegisterWrapper(lua_State *pkState);
	bool IsNil();
	void ClearNPCQuestInfo();
	void CallEventFullScreenTalk(lwWString kObjectName,lwWString kTitleText,lwWString kDialogText,lwWString kFaceID,char const *szStitchImageID=NULL);
	bool IsFullQuestDialog();
	bool IsEventScriptDialog();
	bool NextQuestDialog(lwUIWnd kTop);
	void CallNpcTalk(lwGUID kNpcGuid);
	void CallNpcWarning(lwGUID kNpcGuid);
	void ClearRecentQuestInfo();
	bool GetMiniQuestState();
	void SetMiniQuestState(bool bVisible);
	void UpdateQuestOut(lwUIWnd lwUI);
	void SendDialogTrigger(lwGUID kGuid, int const iQuestID, int const iDialogID);
	void NextLetter();
	void PrevLetter();
	void Load();
	bool ResumeNextQuestTalk();
	bool IsMenuDialog();
	void RemoteAcceptQuest(lwUIWnd kSelf);
private:
	PgQuestMan	*m_pQuestMan;
};

extern bool lwCanQuestTalk(lwGUID kNpcGuid);
extern void NET_C_M_REQ_TRIGGER(int const iObjectType, lwGUID kGuid, int const iActionType);
extern inline void CallGuildNpcFunctionUI(int const iDialogType, int const iTTW);
extern void CallNpcFunctionUI(int const iDialogType, lwGUID kNpcGuid, char const *szFaceID, int const iTTW);
extern void CallNpcFunctionUI2(int const iDialogType, lwGUID kNpcGuid, char const *szFaceID, lwWString kText);
extern void CallQuestMiniIngToolTip(lwUIWnd kTopWnd);
extern void CallLevelUpMsg();
#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_LWQUESTMAN_H