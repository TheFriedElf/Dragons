#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIMANUFACTURE_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIMANUFACTURE_H

#include "PgScripting.h"
#include "lwPacket.h"

namespace lwManufacture
{
	void RegisterWrapper(lua_State *pkState);
	bool SendReqManufacture(BYTE byScore);
	bool CheckCanUseManufacture(void);

	bool InsertBundleManMtrl(PgBase_Item const* pkItem, SItemPos const & rkItemInvPos);
	void RemoveBundleManMtrl(void);
	void SetBundleManMtrlIcon(int iItemNo);
	void SetBundleManMtrlCount(int iCount);
	int GetBundleManMtrlCount(void);
	bool SendReqBundleMan(void);
}

class PgManufacture
{
public:
	PgManufacture();
	virtual ~PgManufacture() {}

	void Clear();
	bool SendReqItem(BYTE byScore);
	bool SetMaterialItem(SItemPos const& rkItemPos, CItemDef const *pkItemDef, int iItemNo);
	bool CheckCanUse(void);

protected:
	SItemPos m_kSrcItemPos;
	int m_iSrcItemNo;
};

#define g_kManufacture SINGLETON_STATIC(PgManufacture)

#endif //FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIMANUFACTURE_H
