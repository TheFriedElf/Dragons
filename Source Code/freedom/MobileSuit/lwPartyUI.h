#ifndef FREEDOM_DRAGONICA_XML_LWPARTYUI_H
#define FREEDOM_DRAGONICA_XML_LWPARTYUI_H
namespace PgClientPartyUtil
{
	void RefreshPartyInfoWnd(lwUIWnd lwPartyFrm);
	void RefreshPartyPeopleFindWnd(lwUIWnd lwPartyFrm);
	void RefreshPartyStateWnd();//배경에 뜨는 파티창 상태 갱신
	void Refresh_Part_People_FindWnd();
	void RefreshPartyTitleWnd();
	int SetPartyOptionArea(int const iAttribute, int const iContinent, bool bAll=false);
	lwWString GetClassName(lwGUID kGuid);
	int GetPartyMemberLocation(lwGUID kCharGuid);
	int GetPartyOptionExp();
	int GetPartyOptionItem();
	int GetPartyOptionPublicTitle();
	int GetOptionAttribute();
	int GetOptionContinent();
	int GetOptionArea_NameNo();
	bool GetPartyOptionState();
	void SetPartyOptionNew(int const iAttribute, int const iContinent, int const iArea, int const MaxMember, lwWString kPartySubName);
	void SetPartyOption(int const iOptionExp, int const iOptionItem, int const iOptionPublicTitle, int const iLevel, bool const bState);
	//void lwSetPartyOption(int const iOptionExp, int const iOptionItem);
	lwWString GetPartyOptionWStr();
	void SearchPeopleUpdate(CONT_SEARCH_UNIT_INFO& unit_itr);
	bool lwHaveIParty();	
	bool lwEnablePartyBreakIn();
	void lwSend_PT_C_M_REQ_JOIN_PARTY_REFUSE(bool const Refuse);
	void lwCheckCanKick(lwGUID lwCharGuid, char const* ReqType);
	bool lwCheckCanKick_Leave();
};

void lwPartyStateCloseBtnDown(lwUIWnd kWnd);
#endif // FREEDOM_DRAGONICA_XML_LWPARTYUI_H