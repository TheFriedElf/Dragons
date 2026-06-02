#include "stdafx.h"
#include "PgNifMan.h"
#include "PgRenderer.h"
#include "Pg2DString.h"
#include "PgBalloon.h"

#include "NewWare/Renderer/DrawUtils.h"
#include "NewWare/Scene/ApplyTraversal.h"


PgBalloon::PgBalloon(void)
:	m_kID(BM::GUID::Create())
,	m_bEnable(false)
,	m_bShow(true)
,	m_spScreenTexture(NULL)
{
}

PgBalloon::~PgBalloon(void)
{
	CONT_TEXT::iterator itr = m_kContText.begin();
	for ( ; itr != m_kContText.end() ; ++itr )
	{
		itr->second.Release();
	}
	m_kContText.clear();

	m_spScreenTexture = NULL;
}

bool PgBalloon::IsDrawImmediate(void)const
{
	if ( m_spScreenTexture )
	{
		return IsEnable() && IsShow();
	}
	return false;
}

HRESULT PgBalloon::AddText( int const iTextID, Pg2DString *pk2DString, NiPoint3 const &kAdjustPos, NiColorA const &kTextColor, NiColorA const &kOutLineColor, bool const bOutLine, float const fScale )
{
	if ( pk2DString )
	{
		SBalloonTextInfo kTextInfo;
		kTextInfo.pk2DString = pk2DString;
		if ( kTextInfo.pk2DString )
		{
			kTextInfo.kTextColor = kTextColor;
			kTextInfo.kOutLineColor = kOutLineColor;
			kTextInfo.bOutLine = bOutLine;
			kTextInfo.fScale = fScale;
			kTextInfo.kTextAdjustPos = kAdjustPos;

			std::pair<CONT_TEXT::iterator, bool> kPair = m_kContText.insert( std::make_pair( iTextID, kTextInfo ) );
			if ( !kPair.second )
			{
				kPair.first->second.Release();
				kPair.first->second = kTextInfo;
				return S_FALSE;
			}
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT PgBalloon::RemoveText( int const iTextID )
{
	CONT_TEXT::iterator itr = m_kContText.find( iTextID );
	if ( itr != m_kContText.end() )
	{
		itr->second.Release();
		m_kContText.erase( itr );
		return S_OK;
	}
	return E_FAIL;
}

Pg2DString * PgBalloon::GetText( int const iTextID )const
{
	CONT_TEXT::const_iterator itr = m_kContText.find( iTextID );
	if ( itr != m_kContText.end() )
	{
		return itr->second.pk2DString;
	}
	return NULL;
}

void PgBalloon::DrawImmediateText( PgRenderer *pkRender, NiCamera* pCamera, NiPoint3 const &kBasePos )
{
	CONT_TEXT::iterator text_itr = m_kContText.begin();
	for ( ; text_itr != m_kContText.end() ; ++text_itr )
	{
		CONT_TEXT::mapped_type &kElement = text_itr->second;
		NiPoint3 const kTextPos = kBasePos + kElement.kTextAdjustPos;
		if ( true == Is3D() )
		{
			kElement.pk2DString->Draw_3DPos( pkRender, kTextPos, pCamera, kElement.kTextColor, kElement.kOutLineColor, kElement.bOutLine, kElement.fScale, false );
		}
		else
		{
			kElement.pk2DString->Draw( pkRender, static_cast<int>(kTextPos.x), static_cast<int>(kTextPos.y), kElement.kTextColor, kElement.kOutLineColor, kElement.bOutLine );
		}
	}
}

NiMaterialPropertyPtr		PgBoardBalloon::m_spMaterialProperty = NULL;
NiAlphaPropertyPtr			PgBoardBalloon::m_spAlphaProperty = NULL;
NiZBufferPropertyPtr		PgBoardBalloon::m_spZBufferProperty = NULL;
NiVertexColorPropertyPtr	PgBoardBalloon::m_spVertexColorProperty = NULL;

PgBoardBalloon::PgBoardBalloon(void)
:	m_spQuad(NULL)
,	m_ptPos( 0.0f, 0.0f, 0.0f )
,	m_ptAdjustPos( 0.0f, 0.0f, 0.0f )
,	m_fScale(1.0f)
{
}

PgBoardBalloon::~PgBoardBalloon(void)
{
	m_spQuad = NULL;
}

bool PgBoardBalloon::Init( std::string const &strTexutre, NiPoint3 const &pt3AdjustPos, float const fScale )
{
	NiSourceTexturePtr pTexture =  g_kNifMan.GetTexture(strTexutre);
	if ( pTexture )
	{
		m_ptAdjustPos = pt3AdjustPos;
		m_fScale = std::max( fScale, 0.01f );
		m_spScreenTexture = NiNew NiScreenTexture( pTexture );
		m_spQuad = CreateQuad();
		if ( m_spQuad )
		{
			SetEnable(true);
			return true;
		}
	}
	return false;
}

NiBillboardNodePtr PgBoardBalloon::CreateQuad()
{
	if ( !m_spScreenTexture || (0.01f > m_fScale) )
	{
		return NULL;
	}

	NiTexture *pkTexture = m_spScreenTexture->GetTexture();
	if( pkTexture )
	{
		NiBillboardNodePtr spQuad = NiNew NiBillboardNode();
		if ( spQuad )
		{
			spQuad->SetMode(NiBillboardNode::RIGID_FACE_CAMERA);

			float const fTexWidth = static_cast<float>(pkTexture->GetWidth());
			float const fTexHeight = static_cast<float>(pkTexture->GetHeight());

			float const fQuadWidth = fTexWidth * m_fScale * 0.5f;
			float const fQuadHeight = fTexHeight * m_fScale * 0.5f;

			NiPoint3	const vCamRight = NiPoint3::UNIT_X;
			NiPoint3	const vCamUp = NiPoint3::UNIT_Y;

			unsigned short const usVertices = 4;

			NiPoint3 *pkVertexs = NiNew NiPoint3[usVertices];
			if ( pkVertexs )
			{
				pkVertexs[0] = (vCamRight * fQuadWidth) - (vCamUp * fQuadHeight);
				pkVertexs[1] = (vCamRight * fQuadWidth) + (vCamUp * fQuadHeight);
				pkVertexs[2] = (-vCamRight * fQuadWidth) - (vCamUp * fQuadHeight);
				pkVertexs[3] = (-vCamRight * fQuadWidth) + (vCamUp * fQuadHeight);
			}

			NiColorA	*pkColors = NiNew NiColorA[usVertices];
			if ( pkColors )
			{
				pkColors[0] = NiColorA::WHITE;
				pkColors[1] = NiColorA::WHITE;
				pkColors[2] = NiColorA::WHITE;
				pkColors[3] = NiColorA::WHITE;
			}

			NiPoint2 *pkTextureUV = NiNew NiPoint2[usVertices];
			if ( pkTextureUV )
			{
				pkTextureUV[0].x = 1.0f;
				pkTextureUV[0].y = 1.0f;

				pkTextureUV[1].x = 1.0f;
				pkTextureUV[1].y = 0.0f;

				pkTextureUV[2].x = 0.0f;
				pkTextureUV[2].y = 1.0f;

				pkTextureUV[3].x = 0.0f;
				pkTextureUV[3].y = 0.0f;
			}

			unsigned short const usTriangles = 2;
			unsigned short *pusTriangleList = NiAlloc(unsigned short, static_cast<int>(usTriangles) * 3);
			if ( pusTriangleList )
			{
				pusTriangleList[0] = 0;
				pusTriangleList[1] = 1;
				pusTriangleList[2] = 3;

				pusTriangleList[3] = 3;
				pusTriangleList[4] = 2;
				pusTriangleList[5] = 0;
			}
			

			NiTriShape *pkTriShape = CreateShape( usVertices, pkVertexs, pkColors, pkTextureUV, usTriangles, pusTriangleList, pkTexture );
			if ( pkTriShape )
			{
				spQuad->AttachChild( pkTriShape, true);
			}
		}
		return spQuad;
	}
	return NULL;
}

NiTriShape*	PgBoardBalloon::CreateShape( unsigned short usVertices, NiPoint3 *pkVertexs, NiColorA *pkColors, NiPoint2 *pkTextureUV, unsigned short usTriangles, unsigned short *pusTriangleList, NiTexture *pkTexture )
{
	NiTriShape *pkShape = NiNew NiTriShape( usVertices, pkVertexs, NULL, pkColors, pkTextureUV, 1, NiGeometryData::NBT_METHOD_NONE, usTriangles, pusTriangleList );
	if ( pkShape )
	{
		pkShape->SetConsistency(NiGeometryData::STATIC);

		if( !m_spMaterialProperty )
		{
			m_spMaterialProperty = NiNew NiMaterialProperty();
			m_spMaterialProperty->SetAlpha(1.0f);
			m_spMaterialProperty->SetAmbientColor(NiColor::WHITE);
			m_spMaterialProperty->SetDiffuseColor(NiColor::WHITE);
			m_spMaterialProperty->SetEmittance(NiColor::WHITE);
			m_spMaterialProperty->SetSpecularColor(NiColor::BLACK);
		}

		if( !m_spAlphaProperty )
		{
			m_spAlphaProperty	= NiNew	NiAlphaProperty();
			m_spAlphaProperty->SetAlphaBlending(true);
			m_spAlphaProperty->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
			m_spAlphaProperty->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
			m_spAlphaProperty->SetAlphaTesting(true);
			m_spAlphaProperty->SetTestMode(NiAlphaProperty::TEST_GREATER);
			m_spAlphaProperty->SetTestRef(0);
			m_spAlphaProperty->SetAlphaGroup(AG_EFFECT);
		}

		if( !m_spZBufferProperty )
		{
			m_spZBufferProperty	= NiNew	NiZBufferProperty();
			m_spZBufferProperty->SetZBufferWrite(false);
			m_spZBufferProperty->SetZBufferTest(true);
		}

		if( !m_spVertexColorProperty )
		{
			m_spVertexColorProperty = NiNew NiVertexColorProperty();
			m_spVertexColorProperty->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
			m_spVertexColorProperty->SetSourceMode(NiVertexColorProperty::SOURCE_AMB_DIFF);
		}

		pkShape->AttachProperty(m_spAlphaProperty);
		pkShape->AttachProperty(m_spMaterialProperty);
		pkShape->AttachProperty(m_spVertexColorProperty);

		
		NiTexturingPropertyPtr spTexturing = NiNew NiTexturingProperty;
		if ( spTexturing )
		{
			spTexturing->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
			spTexturing->SetBaseTexture(pkTexture);

			pkShape->AttachProperty(spTexturing);	
		}

		pkShape->AttachProperty(m_spZBufferProperty);

		if(	PgRenderer::GetPgRenderer() &&
			PgRenderer::GetPgRenderer()->GetVertexShaderVersion() >= D3DVS_VERSION(1,1) )
		{
			NiShaderPtr spStringScene = NiShaderFactory::GetInstance()->RetrieveShader("StringScene", NiShader::DEFAULT_IMPLEMENTATION, true);
			NiSingleShaderMaterial* pkMaterial = NiSingleShaderMaterial::Create(spStringScene); 
			pkShape->ApplyAndSetActiveMaterial(pkMaterial);
		}

		pkShape->UpdateEffects();
		pkShape->UpdateProperties();
		pkShape->UpdateNodeBound();
		pkShape->Update(0.0f);

		pkShape->SetActiveTriangleCount(0);
        NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkShape, false );

		return	pkShape;
	}
	return NULL;
}

void PgBoardBalloon::DrawImmediate( PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime )
{
	if ( true == IsDrawImmediate() )
	{
		if ( m_spQuad )
		{
			if( 0 != m_spQuad->GetArrayCount() )
			{
				m_spZBufferProperty->SetZBufferWrite(true);
				m_spZBufferProperty->SetZBufferTest(true);

				m_spQuad->SetColorLocal(NiColorA::WHITE);
				m_spQuad->SetTranslate(m_ptPos);
				m_spQuad->Update(0);

				NiVisibleArray	kVisibleArray;
				pkRenderer->CullingProcess_Deprecated( spCamera, m_spQuad, &kVisibleArray);
                NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), m_spQuad );

				DrawImmediateText( pkRenderer, spCamera, m_ptPos );
			}
		}
	}
}

void PgBoardBalloon::DestoryStatic()
{
	m_spMaterialProperty = NULL;
	m_spAlphaProperty = NULL;
	m_spZBufferProperty = NULL;
	m_spVertexColorProperty = NULL;
}
