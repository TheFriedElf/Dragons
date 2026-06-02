#ifndef FREEDOM_DRAGONICA_UI_PGPETITIONMGR_H
#define FREEDOM_DRAGONICA_UI_PGPETITIONMGR_H
#include "CreateUsingNiNew.inl"

class PgPetitionMgr
{
private:

protected:

public:
	PgPetitionMgr(void);
	~PgPetitionMgr(void);

	bool Send_Petition(std::wstring const &wsTitle, std::wstring const &wsNote);
	bool Select_Remainder_Petition();
	bool PetitionDlg_Show(int iNumber);
	bool ReceiptPetition_Success(int ReceiptIndex);
	void WaitReceipt_Petition();
};
#define g_kPetitionMgr SINGLETON_CUSTOM(PgPetitionMgr, CreateUsingNiNew)
#endif //FREEDOM_DRAGONICA_UI_PGPETITIONMGR_H