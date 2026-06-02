#ifndef FREEDOM_DRAGONICA_CONTENTS_PGMYACTORVIEWMGR_H
#define FREEDOM_DRAGONICA_CONTENTS_PGMYACTORVIEWMGR_H

#include "lwUI.h"

namespace lwMyActorView
{
	void RegisterWrapper(lua_State *pkState);
	void lwInitActor(lwUIWnd UISelf, char const* szAddName, bool bOrtho);
	void lwUpdateActor(char const* szAddName);
	void lwUpdateOtherActor(char const* szAddName, lwGUID Guid);
	void lwRotateActor(char const* szAddName, float const fRad);
	void lwSetMyActorEquip(char const* szAddName);
	void lwDrawMyActorView(lwUIWnd UISelf, char const* szAddName);
	void lwAddToDrawListMyActorView(lwUIWnd UISelf, char const* szAddName);
	void lwDeleteMyActor(lwUIWnd UISelf, char const* szAddName);
	void lwUpdateOrthoZoom(char const* szAddName, float const fScale, int const iX, int const iY);
	void lwUpdateOrthoZoomMinMax(char const* szAddName, float const fMin, float const fMax);

	void lwUpdateMyActorPetView(char const* szAddName);
	void lwUpdateOtherActorPetView(char const* szAddName, lwGUID Guid);
};

typedef std::set< std::string >	CONT_VIEW_ACTOR;
extern char const* const STR_DEF_MODEL_NAME;

typedef enum eChangeEquipType
{
	CET_ADD = 0,
	CET_DEL = 1,
}EChangeEquipType;

class PgActorPet;
class PgMyActorViewMgr
{
public:
	PgMyActorViewMgr(void);
	~PgMyActorViewMgr(void);

	bool Init(XUI::CXUI_Wnd* pTarget, char const* szAddName, bool bOrtho = false);
	bool Update(char const* szAddName);
	bool Update(char const* szAddName, BM::GUID const& Guid);
	bool UpdatePet(char const* szAddName, BM::GUID const& Guid);
	bool DeleteActor(char const* szAddName);
	bool ChangeEquip(char const* szAddName, BM::GUID const& Guid);
	bool ChangePetEquip(char const* szAddName, BM::GUID const& Guid);
	bool ChangePetColor(char const* szAddName, BM::GUID const& Guid);
	bool BeginChangeParts(char const* szAddName);
	bool ChangeParts(int const iItemNo, EChangeEquipType const Type);
	bool ChangePartsColor(eEquipLimit const Limit, DWORD const iItemNo);
	bool EndChangeParts();
	void Del(char const* szAddName);
	void Rotate(char const* szAddName, float const fRad);
	void Draw(XUI::CXUI_Wnd* pTarget, char const* szAddName);
	void AddToDrawList(XUI::CXUI_Wnd* pTarget, char const* szAddName);
	void UpdateOrthoZoom(char const* szAddName, float const fScale, int const iX = 0, int const iY = 0);
	void UpdateOrthoZoomMinMax(char const* szAddName, float const fMin, float const fMax);
	void ChangePetAction(char const* szAddName, std::string const& strActionName);

private:
	PgActor* GetActor(char const* szAddName);
	PgActorPet* GetPet(char const* szAddName);
private:
	CONT_VIEW_ACTOR	m_kActorContainer;
	PgActor*		m_pkTempActor;
};

#define g_kMyActorViewMgr	SINGLETON_STATIC(PgMyActorViewMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_PGMYACTORVIEWMGR_H