#ifndef FREEDOM_DRAGONICA_UTIL_PGUTILACTION_H
#define FREEDOM_DRAGONICA_UTIL_PGUTILACTION_H

#include "Variant/PgDefSpendMoney.h"
// class PgUtilAction
// {
// public:
// 	PgUtilAction(){}
// 	virtual ~PgUtilAction(){};
// 
// 	bool DoAction();
// protected:
// 
// };

class PgPilot;

class PgUtilAction_HaveReviveItem
{
public:
	explicit PgUtilAction_HaveReviveItem( PgInventory * const pkInv, int const iLevel );
	~PgUtilAction_HaveReviveItem(){};

	bool IsHave( const EInvType kInvType )const;
	bool GetItemPos( SItemPos &rkOutItemPos, const EInvType kInvType )const;// 최적조건의 아이템 포지션을 리턴

private:
	PgInventory &m_rkInv;
	int const m_iLevel;

private:
	PgUtilAction_HaveReviveItem();
};

class PgUtilAction_HaveCrossItem
{
public:
	explicit PgUtilAction_HaveCrossItem( PgInventory * const pkInv );
	~PgUtilAction_HaveCrossItem(){};

	bool IsHave()const;
	bool GetItemPos( SItemPos &rkOutItemPos )const;

private:
	PgInventory &m_rkInv;

private:
	PgUtilAction_HaveCrossItem();
};

class PgUtilAction_Revive
{
public:
	explicit PgUtilAction_Revive(const ESpendMoneyType kType,PgPilot* pkPilot);
	virtual ~PgUtilAction_Revive(){};

	bool DoAction(bool const bRealAction);
	__int64 GetNeedMoney()const;

protected:
	const ESpendMoneyType m_kType;
	PgPilot *m_pkPilot;

private:
	PgUtilAction_Revive();
};

#endif //FREEDOM_DRAGONICA_UTIL_PGUTILACTION_H