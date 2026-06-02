#ifndef FREEDOM_DRAGONICA_CONTENTS_LWCHARACTERCARD_H
#define FREEDOM_DRAGONICA_CONTENTS_LWCHARACTERCARD_H

namespace lwCharacterCard
{
	enum EDROP_LIST_INIT_TYPE
	{
		EDROPTYPE_SEX		= 0,
		EDROPTYPE_AGE		= 1,
		EDROPTYPE_LOC,
		EDROPTYPE_BLOODTYPE,
		EDROPTYPE_CONSTELLATION,
		EDROPTYPE_INTEREST,
		EDROPTYPE_STYLE,
	};
	enum ECARD_LIST_UPDATE_TYPE
	{
		ECARDUT_SEX				= 0,
		ECARDUT_AGE				= 1,
		ECARDUT_LOC,
		ECARDUT_BLOODTYPE,
		ECARDUT_CONSTELLATION,
		ECARDUT_INTEREST,
		ECARDUT_STYLE,
		ECARDUT_COMMENT,
	};
	enum ECARD_VIEW_BTN_TYPE
	{
		ECARDVBT_NONE		= 0,
		ECARDVBT_HELP		= 1,
		ECARDVBT_RECOMMAND,
		ECARDVBT_HIDE,
	};


	void RegisterWrapper(lua_State* pkState);

	struct SDROP_ITEM_INFO
	{
		SDROP_ITEM_INFO()
			: Type(EDROPTYPE_SEX)
			, KeyValue(0)
		{}
		SDROP_ITEM_INFO(EDROP_LIST_INIT_TYPE const _Type, BYTE const _KeyValue)
			: Type(_Type)
			, KeyValue(_KeyValue)
		{}
		EDROP_LIST_INIT_TYPE Type;
		int KeyValue;
	};

	//lua open
	void lwCallCharacterCardUI(lwGUID kGuid);
	void lwGetMyCharacterCardInfo();
	void lwSetMyCharacterCard();
	void lwCallCardDropList(lwUIWnd UISelf, char const* pDropListName);
	void lwCallCardDropListByTextWndID(lwUIWnd UISelf, char const* pTextWndID, char const* pDropListName, int const InitType);
	void lwSelectedDropItem(lwUIWnd UISelf);
	void lwUpdateEditInputInfo(lwUIWnd UISelf, int const UpdateType);
	void lwSaveCardEditInfo(lwUIWnd UISelf);
	void lwRecommendTarget(lwGUID kGuid);
	void lwRecommendTargetInUI(lwUIWnd UISelf);
	void lwChangeComment(lwUIWnd UISelf);
	void lwChangeCardOpenState(lwUIWnd UISelf);
	bool lwCheckRecommendLevel();
	bool lwCheckExistCharacterCard(lwGUID kGuid);
	void lwSendMatchUser(lwUIWnd UISelf);
	void lwMatchDefaultInfo(lwUIWnd UISelf);
	void lwDisplayPopularPoint(lwUIWnd UISelf);
	void lwDisplayTodayPopularPoint(lwUIWnd UISelf);
	void lwOnClickSelectMatchUser(lwUIWnd UISelf);
	void lwOnOverSelectMatchUser(lwUIWnd UISelf);
	void lwOnClickManToManTalk(lwUIWnd UISelf);

	//lua close
	void ReceivePacket_Command(WORD const wPacketType, BM::Stream& kPacket);
	bool CheckResultError(HRESULT const Result);
	void CallCharacterCardUI(bool const bEdit, EUseItemCustomType const Type, PgCharacterCard const& kCardInfo, int const ResourceNo = 0, bool const bExist = true);
	void ClearCardEditUI(XUI::CXUI_Wnd* pWnd);
	void InitDropListItem(XUI::CXUI_List* pList, EDROP_LIST_INIT_TYPE const Type);
	bool MakeCardAbilString(WORD wAbilType, int const iValue, std::wstring& kText);
	void SetMatchListInfo(CONT_MATCH_CARD_INFO& kList);
	bool MatchListOrder(SMATCH_CARD_INFO const& rhs, SMATCH_CARD_INFO const lhs);

	std::wstring const GetCardLocalString(int const iKey);
	std::wstring const GetCardKeyString(BYTE const keyType, BYTE const keyValue);
}

#define g_kCharacterCardInfo SINGLETON_STATIC(PgCharacterCard)

#endif // FREEDOM_DRAGONICA_CONTENTS_LWCHARACTERCARD_H