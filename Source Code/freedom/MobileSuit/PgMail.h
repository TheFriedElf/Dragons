#ifndef FREEDOM_DRAGONICA_CONTENTS_MAIL_PGMAIL_H
#define FREEDOM_DRAGONICA_CONTENTS_MAIL_PGMAIL_H

#include "Lohengrin/packetstruct.h"
#include "CreateUsingNiNew.inl"

class XUI::CXUI_Wnd;

int const LETTER_PER_PAGE = 8;
int const INVEN_KIND = 4;
int const REST_DAY = 3;
int const MAX_INVEN_COUNT = 72;

typedef struct tagRecvMailInfoEx : tagRecvMailInfo
{
	tagRecvMailInfoEx() { m_kItem.Clear(); }
	explicit tagRecvMailInfoEx(tagRecvMailInfo const & kMailInfo)	//복사 생성자
	{
		tagRecvMailInfo::operator = (kMailInfo);
		MakeSItem(kMailInfo);
	}
	void operator = (tagRecvMailInfo const & kMailInfo)
	{
		tagRecvMailInfo::operator = (kMailInfo);
		MakeSItem(kMailInfo);
	}

	void MakeSItem(tagRecvMailInfo const & kMailInfo)
	{
		m_kItem = kMailInfo.kItem;
	}

	PgBase_Item m_kItem;
}SRecvMailInfoEx;

typedef std::map<__int64, SRecvMailInfoEx> CONT_RECV_MAIL_EX_LIST;

class PgMailMgr
{
public:
	PgMailMgr();
	virtual ~PgMailMgr();

	CLASS_DECLARATION_S(BM::GUID, NpcGuid);
	CLASS_DECLARATION_S(__int64, Index);	//실제 메일의 인덱스
	//CLASS_DECLARATION_S(int, ViewIndex);	//컨테이너 상에서 몇번째 메일이 현재 목록 제일 위에 있는지
	CLASS_DECLARATION_S(int, Page);
	CLASS_DECLARATION_S(int, ReservePageDelta);	//페이지 이동 예약
	CLASS_DECLARATION_S(SItemPos, SendItemPos);
	CLASS_DECLARATION_S(bool, IsNewMailArrived);
	CLASS_DECLARATION_S(BM::GUID, SendItemGuid);
	CLASS_DECLARATION_S(__int64, MinIndex);	//실제 메일의 인덱스
	CLASS_DECLARATION_S(int, TotalCount);
	CLASS_DECLARATION_S(unsigned char, NewMailCount);

	void Init();
	void InitUI(int const iIdx, XUI::CXUI_Wnd* pWnd = NULL);
	bool RecvMail_Command(WORD const wPacketType, BM::Stream &rkPacket);
	bool REQ_MAIL_SEND(XUI::CXUI_Wnd* pkWnd);
	bool REQ_MASSMAIL_SEND(XUI::CXUI_Wnd* pkWnd);

	void ClearMailList();

	bool FindMail_ByGuid(BM::GUID const& rkGuid, SRecvMailInfoEx& rkDest) const;
	void REQ_MAIL_RECV(__int64 const i64Index = 0i64, int const iCount = -LETTER_PER_PAGE);
	void MakeMailList(XUI::CXUI_Wnd * pPost0 = NULL, bool const bCheckReset=true);
	bool WriteToUI(SRecvMailInfoEx const& rkSrc, XUI::CXUI_Wnd* pkWnd, bool const bCheckReset=true);
	PgBase_Item const * GetItemAt(int const iAt) const;
	bool CheckNewMail(CONT_RECV_MAIL_LIST const & rkCont);
	size_t GetNowMailCount() const { return m_kNowList.size(); }
	size_t GetMailCount() const { return m_kMailList.size(); }
	void NextPage();
	void PrevPage();
	__int64 DeleteMailAt(int const iIndex);
	void OpenMailAt(int const iIndex, XUI::CXUI_Wnd* pkWnd = NULL);
	bool SetSrcItem(SItemPos const &rkItemPos);
	void GetAnnex(XUI::CXUI_Wnd* pkWnd);
	void GetAnnex(int const iIndex, bool bOnlyNoPay = false);
	void GetAnnexAll();
	void ReturnMailAt(int const iAt);
	__int64 const GetLatestIndex() const;
	int AddCheckCount(int iType, int iValue = 1);
	int GetTotalCheckCount();
	bool FindMail_ByAt(int const iAt, SRecvMailInfoEx& rkDest);
private:
	void ClearEditText(XUI::CXUI_Wnd* pkWnd, std::wstring const & kName);
	void ClearText(XUI::CXUI_Wnd* pkWnd, std::wstring const & kName, std::wstring const kText = L"");
	void ClearMailUI(XUI::CXUI_Wnd* pkWnd);
	bool DeleteMail(__int64 const & rkIndex);
	bool FindMail_ByIndex(__int64 const i64Index, SRecvMailInfoEx& rkDest);
	void SendModify(SRecvMailInfo const & rkMail, EPostMailModifyType eType);
	int CalcRestDay(BM::DBTIMESTAMP_EX const & rkLimit);
	
	CONT_RECV_MAIL_EX_LIST	m_kMailList;
	CONT_RECV_MAIL_EX_LIST	m_kNowList;

	int m_iCheckCount[INVEN_KIND];
};

#define g_kMailMgr SINGLETON_CUSTOM(PgMailMgr, CreateUsingNiNew)

extern void AttachHeadString(SRecvMailInfo const & rkMail, std::wstring& rkStr);

extern void Update_NOTI_NEW_MAIL();

#endif // FREEDOM_DRAGONICA_CONTENTS_MAIL_PGMAIL_H