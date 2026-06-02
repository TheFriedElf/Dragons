#ifndef FREEDOM_DRAGONICA_CONTENTS_PGITEMUSECHECKER_H
#define FREEDOM_DRAGONICA_CONTENTS_PGITEMUSECHECKER_H

namespace PgItemUseChecker
{
	bool PickOutOtherExtendInvItem(ContHaveItemNoCount& kItemCont, int const INV_TYPE, int const UICT_TYPE);
	bool CommonExtendInvTypeCheck(int const UICT_TYPE, int iInvType, SItemPos const& kItemPos);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_PGITEMUSECHECKER_H