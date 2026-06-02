#ifndef FREEDOM_DRAGONICA_CONTENTS_COUPLE_LWCOUPLE_H
#define FREEDOM_DRAGONICA_CONTENTS_COUPLE_LWCOUPLE_H

namespace lwCouple
{
	void Couple_FindNextPage();
	void Couple_FindPrevPage();
	void DrawItem(lwUIWnd kImgWnd, int const iItemNo, bool const bSetItemCount);
	bool IamHaveCouple();
	void RegisterWrapper(lua_State *pkState);
	void ReqBreakCouple();
	void ReqWarpCouple();
	void Send_AnsCouple(lwGUID kGuid, bool bSayYes, bool bInstance);
	void Send_BreakCouple();
	void Send_ReqCoupleByGuid(lwGUID kGuid, bool bInstance);
	void Send_ReqCoupleByName(lwWString kName, bool bInstance);
	void Send_ReqFind();
	void Send_ReqInfo();
	void Send_WarpCouple();
	void UpdateCoupleUI(lwUIWnd kWnd);
	std::wstring GetCoupleLinkStatusName(BYTE iLinkStatus);
	void CallCoupleWarpItemToolTip(lwUIWnd kControl);
	int const GetMyCoupleMapNo();
	void Send_ReqSweetHeartQuestByGuid(int const iQuestID);
	void Send_AnsSweetHeartQuest(lwGUID kGuid, bool bSayYes, int iQuestID);
	int const GetMyCouplePlayTime();
	void CheckSweetHeartQuestTime();
	void CoupleSweetHeartUI();
	void SweetHeartQuestCompleteBtn(int const iQuestID);
	void Send_AnsSweetHeartComplete(lwGUID kGuid, bool bSayYes, int iQuestID);
	void OnSelectSendMarry(int const iGoldMoney);
	void OnSelectSendMoney(int const iGoldMoney);
	void SetMarryState(int const iType);
	void SetMarryTotalMoney();
	void SweetHeartNPC_TalkCall(int const iTextNo, char const *kNpcGuidString);
	void SweetHeartVillageNPC_TalkCall(char const *kCharName, char const *kCoupleName, char const *kNpcGuidString);
	bool IsSweetHeart();
};

#endif // FREEDOM_DRAGONICA_CONTENTS_COUPLE_LWCOUPLE_H