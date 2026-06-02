#ifndef FREEDOM_DRAGONICA_CONTENTS_LWLUCKYSTARUI_H
#define FREEDOM_DRAGONICA_CONTENTS_LWLUCKYSTARUI_H

namespace lwLuckyStarUI
{
	typedef enum eLuckyStarDialogState
	{
		LSDS_NONE = 0,
		LSDS_SELECTPOP = 1,
		LSDS_RESULTPOP = 2,
		LSDS_RESULTCUSTOMPOP = 3,
		LSDS_INFOPOP = 4,
		LSDS_CHANGEPOP = 5,
	}ELUCKYSTAR_DLG_STATE;

	void RegisterWrapper(lua_State *pkState);

	void lwLuckyStarEventInit();
	void lwLuckyStarEventUI();
	lwGUID lwGetLuckyStarEventGuid();
	bool lwIsLuckyStarAutoPopup(); 
	void lwLuckyStarAutoPopup(bool bState);
	void lwLuckyStarPopupOpen();
	void lwLuckyStarPopupClose(lwUIWnd UIWnd);
	void lwLuckyStarResultDisplay(lwUIWnd UIWnd);
	void lwLuckyStarResultCustomDisplay(lwUIWnd UIWnd);
	void lwLuckyStarInfoDisplay(lwUIWnd UIWnd);
	lwWString lwLuckyStarNextEventTime();

	void RecvLuckyStar_Command(WORD const wPacketType, BM::Stream& rkPacket);
	bool LuckyStarResult(ELUCKYSTAR_EVENT_RESULT const iErrorType);
	void LuckyStarEventClose();
}

#endif // FREEDOM_DRAGONICA_CONTENTS_LWLUCKYSTARUI_H