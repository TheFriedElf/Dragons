#include "stdafx.h"
#include "Variant/PgEmporiaFunction.h"
#include "PgActor.h"
#include "PgActorNpc.h"
#include "PgPilot.h"
#include "PgPilotMan.h"
#include "PgTrigger.h"
#include "PgChatMgrClient.h"
#include "PgTextBalloon.h"
#include "PgBuilding.h"

PgBuilding::PgBuilding(void)
:	m_pkObject(NULL)
,	m_pkTrigger(NULL)
,	m_bConstructed(true)
,	m_fDisableAlpha(0.0f)
{
}

PgBuilding::~PgBuilding(void)
{
	g_kBalloonMan3D.DestroyNode( m_kBoardBalloonID );
}

int PgBuilding::GetTTWName( short nFuncNo )
{
// 	switch( nFuncNo )
// 	{
// 	case EMPORIA_CONTRIBUTE:{return 72006;}break;
// 	case EMPORIA_WAREHOUSE:{return 72005;}break;
// 	}
	return 0;
}

void PgBuilding::Init( NiAVObject *pkObject, float const fDisableAlpha, PgBoardBalloon * pkBoardBallon )
{
	m_fDisableAlpha = fDisableAlpha;
	m_pkObject = pkObject;
	Destroy();

	if ( pkBoardBallon )
	{
		m_kBoardBalloonID = pkBoardBallon->GetID();

		if ( m_pkObject )
		{
			pkBoardBallon->SetPos( m_pkObject->GetWorldTranslate() );
		}
		else if ( BM::GUID::IsNotNull(m_kNpcID) )
		{
			PgPilot *pkPilot = g_kPilotMan.FindPilot(m_kNpcID);
			if(pkPilot)
			{
				PgIWorldObject *pkWorldObject = pkPilot->GetWorldObject();
				if ( pkWorldObject )
				{
					pkBoardBallon->SetPos( pkWorldObject->GetWorldTranslate() );
				}
			}
		}
	}
}

void PgBuilding::AddToggleObject( NiAVObject *pkObject, bool const bToggleOnShow )
{
	m_kContObjectList.push_back( std::make_pair(pkObject,bToggleOnShow) );
}

bool PgBuilding::Construct( SEmporiaFunction const &kFunc )
{
	if ( !m_bConstructed )
	{
		if ( m_pkObject )
		{
			if ( 0.0f == m_fDisableAlpha )
			{
				m_pkObject->SetAppCulled(false);
			}
			else
			{
				SetChangeAlpha( m_pkObject, 1.0f );
			}

			CONT_OBJECT_LIST::iterator toggle_itr = m_kContObjectList.begin();
			for ( ; toggle_itr!=m_kContObjectList.end() ; ++toggle_itr )
			{
				(*toggle_itr).first->SetAppCulled( !(*toggle_itr).second );
			}
		}

		if ( BM::GUID::IsNotNull(m_kNpcID) )
		{
			PgPilot *pkPilot = g_kPilotMan.FindPilot(m_kNpcID);
			if(pkPilot)
			{
				pkPilot->SetAbil( AT_HP, 1 );
				PgActorNpc *pkActorNpc = dynamic_cast<PgActorNpc*>(pkPilot->GetWorldObject());
				if(pkActorNpc)
				{
					pkActorNpc->SetHide(false);
					pkActorNpc->SetHideShadow(false);
					if ( 0.0f != m_fDisableAlpha )
					{
						pkActorNpc->UseSmoothShow( 1.0f );
						pkActorNpc->SetTargetAlpha( 0.0f, 1.0f, 1.0f );
					}
				}
			}
		}
		
		m_bConstructed = true;
		if ( m_pkTrigger )
		{
			m_pkTrigger->Enable(true);
		}

		if ( BM::GUID::IsNotNull(m_kBoardBalloonID) )
		{
			PgBoardBalloon *pkBalloon = g_kBalloonMan3D.Get<PgBoardBalloon>( m_kBoardBalloonID );
			if ( pkBalloon )
			{
				pkBalloon->SetShow( true );

				Pg2DString *pk2DString = pkBalloon->GetText(3);
				if ( pk2DString )
				{
					int const iRemainHourTime = static_cast<int>((kFunc.i64ExpirationDate - g_kEventView.GetLocalSecTime(CGameTime::DEFAULT)) / CGameTime::HOUR);
					if ( iRemainHourTime > 0i64 )
					{
						std::wstring wstrText;
						WstringFormat( wstrText, 100, TTW(122).c_str(), iRemainHourTime / 24, iRemainHourTime % 24 );

						XUI::CXUI_Font *pkFont = g_kFontMgr.GetFont( FONT_NAME );
						if ( pkFont )
						{
							XUI::CXUI_Style_String kSrcStyleString( XUI::PgFontDef( pkFont, 0xFFFFFFFF ), wstrText );
							pk2DString->SetText( kSrcStyleString );
						}
					}
				}
			}
		}

		return  true;
	}
	return false;
}

bool PgBuilding::Destroy()
{
	if ( m_bConstructed )
	{
		if ( m_pkObject )
		{
			PgBoardBalloon *pkBalloon = g_kBalloonMan3D.Get<PgBoardBalloon>( m_kBoardBalloonID );

			if ( 0.0f == m_fDisableAlpha )
			{
				m_pkObject->SetAppCulled(true);
				if ( pkBalloon )
				{
					pkBalloon->SetShow( false );
				}
			}
			else
			{
				SetChangeAlpha( m_pkObject, m_fDisableAlpha );
			}

			if ( pkBalloon )
			{
				Pg2DString * pk2DString = pkBalloon->GetText( 3 );
				if ( pk2DString )
				{
					XUI::CXUI_Font *pkFont = g_kFontMgr.GetFont( FONT_NAME );
					if ( pkFont )
					{
						XUI::CXUI_Style_String kSrcStyleString( XUI::PgFontDef( pkFont, 0xFFFFFFFF ), std::wstring() );
						pk2DString->SetText( kSrcStyleString );
					}
				}
			}
		}

		if ( BM::GUID::IsNotNull(m_kNpcID) )
		{
			PgPilot *pkPilot = g_kPilotMan.FindPilot(m_kNpcID);
			if(pkPilot)
			{
				pkPilot->SetAbil( AT_HP, 0 );
				PgActorNpc *pkActorNpc = dynamic_cast<PgActorNpc*>(pkPilot->GetWorldObject());
				if(pkActorNpc)
				{
					if ( 0.0f == m_fDisableAlpha )
					{
						pkActorNpc->SetHide( true );
						pkActorNpc->SetHideShadow( true );
					}
					else
					{
						pkActorNpc->SetHide( false );
						pkActorNpc->SetHideShadow( false );
						pkActorNpc->UseSmoothShow( m_fDisableAlpha );
						pkActorNpc->SetTargetAlpha(pkActorNpc->GetAlpha(), m_fDisableAlpha, 1.0f );
					}	
				}
			}
		}

		CONT_OBJECT_LIST::iterator toggle_itr = m_kContObjectList.begin();
		for ( ; toggle_itr!=m_kContObjectList.end() ; ++toggle_itr )
		{
			(*toggle_itr).first->SetAppCulled( (*toggle_itr).second );
		}

		m_bConstructed = false;

		if ( m_pkTrigger )
		{
			m_pkTrigger->Enable(false);
		}

		return  true;
	}
	return false;
}

void PgBuilding::SetTrigger( PgTrigger *pkTrigger )
{
	m_pkTrigger=pkTrigger;
	if ( m_pkTrigger )
	{
		m_pkTrigger->Enable(false);
	}
}

void PgBuilding::SetChangeAlpha( NiAVObject* pkAVObject, float const fAlpha )
{
	if ( !pkAVObject )
	{
		return;
	}

	NiGeometry* pkGeometry = NULL; 
	if( pkGeometry = NiDynamicCast( NiGeometry, pkAVObject ) )
	{
		// MaterialProperty를 가져온다.
		NiMaterialProperty *pkMaterialProp = pkGeometry->GetPropertyState()->GetMaterial();
		if(pkMaterialProp)
		{
			if ( pkMaterialProp->GetAlpha() != fAlpha )
			{
				// 블랜딩 속성 장착
				NiAlphaProperty* pkAlphaProp = NiDynamicCast(NiAlphaProperty, pkAVObject->GetProperty(NiAlphaProperty::GetType()));
				if( !pkAlphaProp )
				{
					// 알파 속성이 없으므로 새로 만든다
					pkAlphaProp = NiNew NiAlphaProperty;
				}

				if ( pkAlphaProp )
				{
					pkAlphaProp->SetAlphaBlending(true);
					pkAlphaProp->SetAlphaTesting(false);
					pkAlphaProp->SetTestRef(0);
					pkAlphaProp->SetTestMode(NiAlphaProperty::TEST_GREATER);
					pkAlphaProp->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
					pkAlphaProp->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
					pkAlphaProp->SetAlphaGroup(AG_OBJECT);

					pkGeometry->AttachProperty(pkAlphaProp);
				}

				pkMaterialProp->SetAlpha( fAlpha );
				pkGeometry->SetNeedUpdateAllShaderConstant(true);
				pkGeometry->UpdateProperties();
			}
		}
	}

	NiNode* pkNode = NiDynamicCast(NiNode, pkAVObject);
	if(pkNode)
	{
		for(unsigned int un = 0; un < pkNode->GetArrayCount(); ++un)
		{
			SetChangeAlpha( pkNode->GetAt(un), fAlpha );
		}
	}
}

