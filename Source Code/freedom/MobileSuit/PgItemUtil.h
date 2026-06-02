#ifndef FREEDOM_DRAGONICA_UTIL_PGITEMUTIL_H
#define FREEDOM_DRAGONICA_UTIL_PGITEMUTIL_H

#include "loki/typemanip.h"
#include "Variant/Item.h"

template< typename T_PRED >
struct SCompareAbil
{
	explicit SCompareAbil( WORD const _wAbilType )
		:	wAbilType(_wAbilType)
	{}

	~SCompareAbil()
	{}

	bool operator()( CItemDef const *pkLeft, CItemDef const *pkRight )const
	{
		if ( pkLeft && pkRight )
		{
			return T_PRED()( pkLeft->GetAbil( wAbilType ), pkRight->GetAbil( wAbilType ) );
		}
		return ( NULL != pkLeft );
	}

	WORD const wAbilType;
};

class PgItemUtil
{
public:
	enum eType
	{
		E_ABIL_GREATER,
		E_ABIL_LESS,
	};

public:
	explicit PgItemUtil( PgInventory &rkInv ):m_rkInv(rkInv){}
	~PgItemUtil(void){}

	template< typename T_PRED, typename T >
	HRESULT Get( T &rkOut, EInvType const kInvType, EUseItemCustomType const eCustomType, T_PRED const &kPred )const;

	template < typename T_PRED > 
	HRESULT Get( int &rkItemNo, EInvType const kInvType, EUseItemCustomType const eCustomType, T_PRED const &kPred )const
	{
		rkItemNo = GetItemNo( kInvType, eCustomType, kPred );
		return (rkItemNo ? S_OK : E_FAIL);
	}

	template < typename T_PRED > 
	HRESULT Get( SItemPos &rkItemPos, EInvType const kInvType, EUseItemCustomType const eCustomType, T_PRED const &kPred )const
	{
		int const iItemNo = GetItemNo( kInvType, eCustomType, kPred );
		if ( iItemNo )
		{	
			return m_rkInv.GetFirstItem( kInvType, iItemNo, rkItemPos );
		}
		return E_FAIL;
	}

	template < typename T_PRED > 
	HRESULT Get( PgBase_Item &rkItem, EInvType const kInvType, EUseItemCustomType const eCustomType, T_PRED const &kPred )const
	{
		SItemPos kItemPos;
		if ( S_OK == Get( kInvType, eCustomType, kPred ) )
		{
			return m_rkInv.GetItem( kItemPos, rkItem );
		}
		return E_FAIL;
	}

	template< typename T >
	HRESULT Get( Loki::Int2Type<E_ABIL_GREATER>, T &rkOut, EInvType const kInvType, EUseItemCustomType const eCustomType, WORD const wAbilType )const
	{
		SCompareAbil< std::greater<int> > kCompareAbil( wAbilType );
		return Get( rkOut, kInvType, eCustomType, kCompareAbil );
	}

	template< typename T >
	HRESULT Get( Loki::Int2Type<E_ABIL_LESS>, T &rkOut, EInvType const kInvType, EUseItemCustomType const eCustomType, WORD const wAbilType )const
	{
		SCompareAbil< std::less<int> > kCompareAbil( wAbilType );
		return Get( rkOut, kInvType, eCustomType, kCompareAbil );
	}

	template< int iType, typename T >
	HREFTYPE Get( T &rkOut, EInvType const kInvType, EUseItemCustomType const eCustomType, WORD const wAbilType )const
	{
		return Get( Loki::Int2Type<iType>(), rkOut, kInvType, eCustomType, wAbilType );
	}

	template< typename T_FILTER >
	HRESULT GetItemNos( ContHaveItemNoCount& rkItemCont, EInvType const kInvType, EUseItemCustomType const eCustomType, T_FILTER const &rkFilter )const
	{
		if( S_OK == m_rkInv.GetItems( kInvType, eCustomType, rkItemCont, true ) )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);

			ContHaveItemNoCount::iterator item_no_itr = rkItemCont.begin();
			while ( item_no_itr != rkItemCont.end() )
			{
				CItemDef const *pkItemDef = kItemDefMgr.GetDef( item_no_itr->first );
				if( pkItemDef )
				{
					if ( true == rkFilter( pkItemDef ) )
					{
						++item_no_itr;
						continue;
					}
				}

				item_no_itr = rkItemCont.erase( item_no_itr );
			}

			return rkItemCont.size() ? S_OK : E_FAIL;
		}
		return E_FAIL;
	}

protected:

	template < typename T_PRED > 
	int GetItemNo( EInvType const kInvType, EUseItemCustomType const eCustomType, T_PRED const &kPred )const
	{
		ContHaveItemNoCount	kItemCont;
		if( S_OK == m_rkInv.GetItems( kInvType, eCustomType, kItemCont, true ) )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);

			CItemDef const * pkLastChkDef = NULL;

			ContHaveItemNoCount::const_iterator item_no_itr = kItemCont.begin();
			for ( ; item_no_itr != kItemCont.end() ; ++item_no_itr )
			{
				CItemDef const *pkItemDef = kItemDefMgr.GetDef( item_no_itr->first );
				if( pkItemDef )
				{
					if ( !kPred( pkLastChkDef, pkItemDef ) )
					{
						pkLastChkDef = pkItemDef;
					}
				}
			}

			if ( pkLastChkDef )
			{
				return pkLastChkDef->No();
			}
		}
		return 0;
	}

private:
	PgInventory& m_rkInv;

private:// not use
	PgItemUtil(void);
};
#endif // FREEDOM_DRAGONICA_UTIL_PGITEMUTIL_H