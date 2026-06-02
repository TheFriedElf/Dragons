#ifndef FREEDOM_DRAGONICA_SERVER_SERVERLIB_H
#define FREEDOM_DRAGONICA_SERVER_SERVERLIB_H

#include "Variant/MonsterDefMgr.h"
#include "Variant/DefAbilType.h"
#include "Variant/TableDataManager.h"

//const extern	CONT_DEFSKILL	*g_pDefSkill;
const extern	CONT_DEFUPGRADECLASS	*g_pDefUpgradeClass;

//extern CMonsterDefMgr g_MonsterMgr;
extern bool LoadTBData(std::wstring const &strFolder);
extern bool AfterLoadTBData();
extern bool GetAbilName(WORD const AbilType, wchar_t const *&pString);
extern bool GetItemName(DWORD const dwItemNo, wchar_t const *&pString);
extern bool GetEffectName(DWORD const dwEffectID, wchar_t const *&pString);
extern bool GetDefString(int const iTextNo, std::wstring const *&pkOut);
extern bool GetDefString(int const iTextNo, wchar_t const *&pString);

extern	int	GetClassUpgradeOrder(int iLastClassID,int *pkout_UpgradeOrderArray);

#ifndef USE_INB
namespace PgDefStringDebugUtil
{
	void ClearDebugString();
	std::wstring const* GetDebugTextString(unsigned long const iTextNo, std::wstring const& rkOriginalStr);
}
#endif
#endif // FREEDOM_DRAGONICA_SERVER_SERVERLIB_H