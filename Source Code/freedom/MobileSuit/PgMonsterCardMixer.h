#ifndef FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGMONSTERCARDMIXTER_H
#define FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGMONSTERCARDMIXTER_H

#include "PgItemMixBase.h"

enum E_MON_CARD_GRADE
{
	EMCG_CURSE	= 0,
	EMCG_NORMAL	= 1,
	EMCG_RARE,
	EMCG_SPECIAL,
	EMCG_UNIQUE,
	EMCG_LEGEND,
};

class PgMonsterCardMixer
	: public PgIMixBase
{
public:
	virtual E_MIX_TYPE const Type() const { return EMT_MONSTER_CARD_MIXER; }
	virtual void RequestMix();
	virtual void Clear();
	void SemiClear();

	bool SetMonsterCard(DWORD const dwItemNo, SItemPos const& kItem);

	PgMonsterCardMixer(void);
	virtual ~PgMonsterCardMixer(void);

protected:
	bool AutoEntrySlot(S_MIX_ITEM_INFO& kSlot);

	virtual bool SetMaterialInfo(DWORD const kMakingItem);
	virtual bool CheckMixType(int const iType) const;

protected:
	CLASS_DECLARATION_S(DWORD, DefaultInsItem);
	CLASS_DECLARATION_S(bool, bAutoRegInsItem);
	CLASS_DECLARATION_S(S_MIX_ITEM_INFO, LeftItem);
	CLASS_DECLARATION_S(S_MIX_ITEM_INFO, RightItem);
	CLASS_DECLARATION_S(S_MIX_ITEM_INFO, InsItem);
};

#endif // FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGMONSTERCARDMIXTER_H