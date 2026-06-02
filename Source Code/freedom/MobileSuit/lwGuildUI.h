#ifndef FREEDOM_DRAGONICA_CONTENTS_CHAT_GUILD_LWGUILDUI_H
#define FREEDOM_DRAGONICA_CONTENTS_CHAT_GUILD_LWGUILDUI_H

namespace lwGuild
{
	bool RegisterWrapper(lua_State *pkState);

	bool Send_PreCreateGuild(lwWString kString);
	bool Send_CreateGuild();
	bool Send_DestroyGuild();
	bool Send_LeaveGuild();
	bool Send_GuildKickUser(lwGUID kCharGuid);
	bool Send_ReqJoinGuildName(lwWString kName);
	bool Send_ReqJoinGuildGuid(lwGUID kGuildGuid);
	bool Send_ReqKickMemberName(lwWString kMemberName);
	bool Send_ReqGuildLvUp();
	bool Send_ReqGuildInventoryCreate();	
	void SendReqChangeMemberGrade(lwGUID kCharGuid, int iNewGrade);
	bool ReqChangeOwner(lwGUID kNewOwnerGuid);
	void CallGuildUI();
	bool UpdateGuildUI(lwUIWnd kTopWnd);
	bool UpdateGuildSkillUI(lwUIWnd kTopWnd);
	bool AmIGuildMaster();
	bool AmIGuildOwner();
	bool IsGuildMaster(lwGUID kCharGuid);
	bool HaveGuild();
	bool HaveGuildInventory();
	int GetGuildLevel();
	int GetGuildLevelIntroText(int const iGuildLv);
	int GetGuildLevelResultText(int const iGuildLv);
	void UpdateGuildSkillBuyUI(lwUIWnd kTopWnd);
	void ShowSingleResultWindow(SIconInfo const &rkIconInfo, std::wstring const &rkNotice, std::wstring const &rkText, const bool bGrayScale = false, PgBase_Item const *pkItem = NULL);
	bool IsHaveGuild(lwGUID kCharGuid);
	void Send_CreateGuildTest();
	void SetTempGuildEmblem(int iEmblem);
	int GetTempGuildEmblem();
	void SetGuildListMode(int iMode);
	void SetGuildNotice(lwWString kNewNotice);
	lwWString GetGuildNotice();

	BYTE GetMyEmporiaKey();
	void EmporiaGiveUp();
	bool Req_JoinEmporia();
	bool IsHaveEmporiaFunc( short nEmporiaFuncNo );
	void KeepEmporiaFunc( short nEmporiaFuncNo );
//	short GetEmporiaFuncPrice( short nEmporiaFuncNo );

	typedef enum eGuildSettingUIType
	{
		EGSUT_GUILD_ENTRANCE = 0,	// 길드가입 설정
		EGSUT_GUILD_APPLICANT,		// 길드가입 신청자 설정
		EGSUT_MERCENARY,			// 용병 설정
	}EGuildSettingUIType;

	typedef enum eGuildEntranceBtnType
	{
		EGEBT_ENTRANCE_OK = 0,
		EGEBT_ENTRANCE_NO,
		EGEBT_LEVEL,
		EGEBT_ALL_JOB,
		EGEBT_FIGHTER,
		EGEBT_MAGICIAN,
		EGEBT_ARCHER,
		EGEBT_THIEF,
		EGEBT_SHAMAN,
		EGEBT_DOUBLE_FIGHTER,
	};

	typedef enum eMercenaryBtnType
	{
		EMSBT_USE_OK	= 0,
		EMSBT_USE_NO,
		EMSBT_LEVEL,
		EMSBT_ALL_JOB,
		EMSBT_FIGHTER,
		EMSBT_MAGICIAN,
		EMSBT_ARCHER,
		EMSBT_THIEF,
	}EMercenaryBtnType;

	typedef enum eGuildInvAuthority
	{
		EGIA_OWNER_IN	= 0,
		EGIA_MASTER_IN	= 1,
		EGIA_MEMBER_IN	= 2,

		EGIA_OWNER_OUT	= 0,
		EGIA_MASTER_OUT	= 1,
		EGIA_MEMBER_OUT	= 2,
	}EGuildInvAuthority;

	void CallGuildSettingUI();
	void OnClickChangeGuildSettingTab(lwUIWnd kSelf, int const iBuildIndex);
	void OnClickGuildEntranceSetBtn(lwUIWnd kSelf, int const Type);
	void OnClickGuildEntranceSetOK(lwUIWnd kParent);
	void OnCallGuildEntranceLevelDrop(lwUIWnd kSelf, int const iMaxLevel);
	void CallGuildApplicantAcceptReject(lwUIWnd kSelf);
	void GuildEntranceProcess(bool const bAccept);

	void SetMercenaryUIBtn(XUI::CXUI_Wnd* pkBtn, bool const bIsAccess, bool const bClickLock = false);
	void SetMercenaryUIBtnDisable(XUI::CXUI_Wnd* pkBtn, bool const bDisable);
	
	void GuildEntranceSetting();
	void GuildApplicantSetting();
	void MercenarySetting();

	void OnCallMercenaryJoinLevelDrop(lwUIWnd kSelf, int const iMaxLevel);
	void SetDisableJobSelectBtn(XUI::CXUI_Wnd* pMainUI, bool const bDisable);
	void OnClickMercenarySetBtn(lwUIWnd kSelf, int const Type);
	void OnClickMercenartSetOK(lwUIWnd kParent);
	
	void lwChangeGuildInvTab(const BYTE byInvType);
	void lwSendGuildInvOpen(const BYTE byInvType, lwGUID kNpcGuid);
	void lwSendGuildInvClose();
	void lwSendGuildInvMoney();
	void lwSendGuildMoney(const int iType);
	void lwRequestGuildInvLog(lwUIWnd kSelf);
	void SendRequestGuildInvLog(const int iPage);
	void lwGuildLogPageMove(const int iMovePage);
	
	void lwCallGuildInvSuperVision();
	void lwOnCallGuildInvAuthoritylDrop(lwUIWnd kSelf, const int iType);
	void lwOnClickGuildInvAuthoritySetBtn(lwUIWnd kSelf, int const iType);
	void lwOnClickGuildInvAuthorityOK(lwUIWnd kParent);
	void CallGuildInv(const BYTE byInvType);
	void CallGuildLog(BM::Stream* pkPacket);
	bool lwIsMyGuildMemeber(lwGUID kGuid);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_CHAT_GUILD_LWGUILDUI_H