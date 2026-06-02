#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWTRANSTOWER_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWTRANSTOWER_H

#include "Variant/Global.h"
#include "lwGuid.h"
#include "lwUI.h"

namespace lwTransTower
{
	class PgTransPortListUI
		:	public XUI::CXUI_List2_Item
	{
	public:
		struct SSort
		{
			bool operator()( XUI::CXUI_List2_Item *pLeft, XUI::CXUI_List2_Item *pRight )const
			{
				PgTransPortListUI *pkLeft = dynamic_cast<PgTransPortListUI*>(pLeft);
				PgTransPortListUI *pkRight = dynamic_cast<PgTransPortListUI*>(pRight);
				if ( pkLeft && pkRight )
				{
					if ( pkLeft->m_kData.nSort == pkRight->m_kData.nSort )
					{
						return pkLeft->m_wstrName < pkRight->m_wstrName;
					}

					return pkLeft->m_kData.nSort < pkRight->m_kData.nSort;
				}
				return false;
			}
		};
	public:
		PgTransPortListUI( BM::GUID const &kOwnerGuid )
			:	XUI::CXUI_List2_Item( kOwnerGuid )
			,	m_bIsOpen(false)
			,	m_bHere(false)
		{}

		virtual ~PgTransPortListUI(){}
		virtual void Refresh();

		std::wstring				m_wstrName;
		TBL_DEF_TRANSTOWER_TARGET	m_kData;
		bool						m_bIsOpen;
		bool						m_bHere;
	};

	template< typename T=PgTransPortListUI >
	class PgTransPortListUI_CreatePolicy
		:	public XUI::CXUI_List2_CreatePolicy<T>
	{
	public:
		explicit PgTransPortListUI_CreatePolicy( TBL_DEF_TRANSTOWER_TARGET const &kData, int const iDisCountRate, bool const bIsOpen, bool const bIsVilliage, bool const bHere )
			:	m_rkData(kData)
			,	m_iDisCountRate(iDisCountRate)
			,	m_bIsOpen(bIsOpen)
			,	m_bIsVilliage(bIsVilliage)
			,	m_bHere(bHere)
		{}

		virtual void Set( T* pType )const
		{
			pType->m_kData = m_rkData;
			pType->m_kData.i64Price -= SRateControl::GetValueRate( m_rkData.i64Price, static_cast<__int64>(m_iDisCountRate) );
			pType->m_bIsOpen = m_bIsOpen;
			pType->m_bHere= m_bHere;

			std::wstring const *pkMapName = NULL;
			if ( true == GetDefString( m_rkData.iMemo, pkMapName) )
			{
				if ( true == m_bIsVilliage )
				{
					pType->m_wstrName = L"{C=0xFF14C814/}";
				}

				pType->m_wstrName += *pkMapName;
			}
		}

	private:
		TBL_DEF_TRANSTOWER_TARGET const	&m_rkData;
		int const						m_iDisCountRate;
		bool							m_bIsOpen;
		bool							m_bIsVilliage;
		bool							m_bHere;
	private:
		PgTransPortListUI_CreatePolicy();
	};


	extern bool RegisterWrapper(lua_State *pkState);

	extern bool CallTransTowerList( lwGUID kGuid, lwUIWnd kListWnd, short int sContinent = 0 );
	extern bool OnClickTransPort( lwUIWnd kUIWnd );
	extern bool OnClickOpenMap( lwUIWnd kUIWnd );
	extern bool OnClickSelectOpenMapItem( lwUIWnd kUIWnd );
	extern bool SendReqTransport(XUI::CXUI_Wnd* pkWnd, XUI::CXUI_List2 *pkList, bool bCallerIsPet = false); //펫전용 순간이동 요청
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWTRANSTOWER_H