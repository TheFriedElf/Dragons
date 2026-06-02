#include "StdAfx.h"
#include "PgAchievementTitle.h"
#include "PgRenderer.H"
#include "Pg2DString.H"
#include "PgNifMan.h"

#include "NewWare/Renderer/DrawUtils.h"
#include "NewWare/Scene/ApplyTraversal.h"


NiColorA const ConvColor(DWORD const kColor)
{
	NiColorA Color(((kColor >> 16) & 0xFF) / 255.f
				  ,((kColor >> 8) & 0xFF) / 255.f
				  , (kColor & 0xFF) / 255.f
				  ,((kColor >> 24) & 0xFF) / 255.f );
	return Color;
}

std::wstring const ACHIEVEMENT_BG_TEX_NAME = L"../Data/6_ui/works/wsTxtBg.tga";

NiImplementRTTI(PgAchievementTitle, NiGeometry);

NiMaterialPropertyPtr		PgAchievementTitle::m_spMaterialProperty = NULL;
NiAlphaPropertyPtr			PgAchievementTitle::m_spAlphaProperty = NULL;
NiZBufferPropertyPtr		PgAchievementTitle::m_spZBufferProperty = NULL;
NiVertexColorPropertyPtr	PgAchievementTitle::m_spVertexColorProperty = NULL;

PgAchievementTitle::PgAchievementTitle(void)
	: NiGeometry(NiNew NiTriShapeData(0,0,0,0,0,0,NiGeometryData::NBT_METHOD_NONE,0,0))
	, m_pk2DString(NULL)
	, m_spBgTexture(NULL)
	, m_spBgTextureBill(NULL)
{
	Init();
}

PgAchievementTitle::~PgAchievementTitle(void)
{
	Destroy();
}

void PgAchievementTitle::Init(void)
{
	NiAlphaPropertyPtr spAlphaProperty = NiNew NiAlphaProperty();
	spAlphaProperty->SetAlphaBlending(true);
	spAlphaProperty->SetAlphaGroup(AG_EFFECT);

	AttachProperty(spAlphaProperty);

	UpdateProperties();
	UpdateEffects();

	m_fQuadScale = 1.f;
	m_bAlive = false;
	m_bDrawShadow = false;
	m_bUseBgColor = true;
	m_kBgColor = NiColorA(1.f, 1.f, 1.f, 1.f);
	m_kTextColor = NiColorA(1.f, 1.f, 1.f, 1.f);
	m_wstrAchievementText.clear();
}

void PgAchievementTitle::Set(XUI::CXUI_Font* pFont, std::wstring const& kAchievementText, DWORD const BgColor, DWORD const FontColor)
{
	m_bAlive = false;

	m_kTextColor = ConvColor(FontColor);
	m_kBgColor = ConvColor(BgColor);
	m_wstrAchievementText = kAchievementText;

	if( m_wstrAchievementText.empty() )
	{
		return;
	}

	if( !m_pk2DString )
	{
		m_pk2DString = new Pg2DString(XUI::PgFontDef(pFont, 0xFFFFFFFF, XUI::XTF_ALIGN_CENTER), kAchievementText);
	}
	
	if( !m_pk2DString )
	{
		return;
	}

	m_pk2DString->SetText(XUI::PgFontDef(pFont), kAchievementText);	

	if( m_bUseBgColor )
	{
		if( !m_spBgTexture )
		{
			NiSourceTexturePtr pkTexture = g_kNifMan.GetTexture(MB(ACHIEVEMENT_BG_TEX_NAME));
			if( pkTexture )
			{
				m_spBgTexture = NiNew NiScreenTexture(pkTexture);
			}
		}

		if( m_spBgTexture )
		{
			m_spBgTextureBill = CreateQuad( m_spBgTexture->GetTexture(), m_pk2DString->GetSize().x, m_pk2DString->GetSize().y);
		}
	}

	m_bAlive = true;
}

POINT PgAchievementTitle::Get2DStringSize()
{
	POINT pt;
	if(m_pk2DString)
	{
		pt.x = m_pk2DString->GetSize().x;
		pt.y = m_pk2DString->GetSize().y;
	}
	else
	{
		pt.x = 0;
		pt.y = 0;
	}

	return pt;
}

void PgAchievementTitle::Destroy(void)
{
	SAFE_DELETE(m_pk2DString);
}

void PgAchievementTitle::DestroyStatic(void)
{
	m_spMaterialProperty = NULL;
	m_spAlphaProperty = NULL;
	m_spZBufferProperty = NULL;
	m_spVertexColorProperty = NULL;
}

void PgAchievementTitle::UpdateWorldBound()
{
	if( m_pk2DString )
	{
		m_kWorldBound.SetCenterAndRadius(GetWorldTranslate(),0);
	}
	m_kWorldBound.SetCenterAndRadius(GetWorldTranslate(), 10);
}

void PgAchievementTitle::RenderImmediate(NiRenderer* pkRenderer)
{
	if( !m_pk2DString )
	{
		return;
	}

	PgRenderer *pkPgRenderer = PgRenderer::GetPgRenderer();
	if( !pkPgRenderer )
	{
		return;
	}

	NiCamera *pkCamera = pkPgRenderer->GetCameraData();
	if( !pkCamera )
	{
		return;
	}

	NiPoint3 const kPos = GetWorldTranslate();

	if( m_bUseBgColor )
	{
		Draw3DPos(pkPgRenderer, pkCamera, kPos);
	}

	NiPoint3	const	&vCamUp = pkCamera->GetWorldUpVector();
	NiPoint3	const	&vCamRight = pkCamera->GetWorldRightVector();


	float const	fOutlineThickness = 0.5f;

	NiPoint3 const	kTextPos = kPos;
	NiPoint3 const	kShadowTextPos = kTextPos - vCamUp * fOutlineThickness + vCamRight * fOutlineThickness;

	NiColorA kShadowColor = NiColorA(0.f, 0.f, 0.f, m_kTextColor.a);
	if(m_bDrawShadow)
	{
		m_pk2DString->Draw_3DPos(pkPgRenderer,kShadowTextPos, pkCamera, kShadowColor, kShadowColor, false, GetScale(), true, XUI::XTF_ALIGN_CENTER);
	}
	m_pk2DString->Draw_3DPos(pkPgRenderer,kTextPos, pkCamera, m_kTextColor, kShadowColor, false, GetScale(), false, XUI::XTF_ALIGN_CENTER);
}

void PgAchievementTitle::Draw3DPos(PgRenderer* pkRenderer, NiCamera* pkCamera, NiPoint3 const &rkPos)
{
	if( !m_bAlive )		{ return; }
	if( !pkRenderer )	{ return; }
	if( !pkCamera )		{ return; }

	if ( m_spBgTextureBill )
	{
		if( 0 != m_spBgTextureBill->GetArrayCount() )
		{
			m_spZBufferProperty->SetZBufferWrite(true);

			m_spBgTextureBill->SetColorLocal( m_kBgColor );
			m_spBgTextureBill->SetTranslate(rkPos);
			m_spBgTextureBill->Update(0);

			NiVisibleArray kVisibleArray;
			pkRenderer->CullingProcess_Deprecated( pkCamera, (NiNode*)m_spBgTextureBill, &kVisibleArray ); // Update billboard!
            NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), m_spBgTextureBill );
		}
	}
}

NiBillboardNodePtr PgAchievementTitle::CreateQuad( NiTexture* pkTexture, int const TxtWidth, int const TxtHeight)
{
	if( pkTexture )
	{
		NiBillboardNodePtr BgTextureBill = NiNew NiBillboardNode();
		if ( BgTextureBill )
		{
			BgTextureBill->SetMode(NiBillboardNode::RIGID_FACE_CAMERA);

			float const fScale = m_fQuadScale * 0.2f * 2.f * 0.6f;

			float const fBoundingWidth = TxtWidth * fScale;
			float const fBoundingHeight = TxtHeight * fScale;
			NiPoint3 const vCamRight = NiPoint3::UNIT_X;
			NiPoint3 const vCamUp = NiPoint3::UNIT_Y;
			NiPoint3 vQuadPos = ( vCamRight * fBoundingWidth / 2.0f ) * (-1) + ( (vCamUp * 0.9f) * fBoundingHeight / 2.0f);
			vQuadPos.z = -NiPoint3::UNIT_Z.z;
			float const fPieceWidth = pkTexture->GetWidth() / 3.f;
			float const fPieceHeight = pkTexture->GetHeight() / 3.f;

			for(int i = 0; i < 9; ++i)
			{
				NiPoint3 vRealPos = vQuadPos;
				float fRealBoundingWidth = fPieceWidth * fScale;	// 이미지 PIECE WIDTH * SCALE (DEFAULT)
				float fRealBoundingHeight = fPieceHeight * fScale;	// 이미지 PIECE HEIGHT * SCALE (DEFAULT)

				switch( ( i / 3 ) )
				{
				case 1://MIDDLE
					{
						vRealPos -= ( vCamUp * fRealBoundingHeight );
						fRealBoundingHeight = fBoundingHeight - ( fRealBoundingHeight * 2 );
					}break;
				case 2:// TOP
					{
						vRealPos -= ( vCamUp * ( fBoundingHeight - fRealBoundingHeight ) );
					}break;
				}

				switch( ( i % 3 ) )
				{
				case 1://CENTER
					{
						vRealPos += ( vCamRight * fRealBoundingWidth );
						fRealBoundingWidth = fBoundingWidth - ( fRealBoundingWidth * 2 );
					}break;
				case 2://RIGHT
					{
						vRealPos += ( vCamRight * ( fBoundingWidth - fRealBoundingWidth ) );
					}break;
				}

				size_t const iVertexCount = 4;
				NiPoint3	*pkVert = NiNew NiPoint3[iVertexCount];
				if ( pkVert )
				{
					pkVert[0] = vRealPos + vCamRight * fRealBoundingWidth - vCamUp * fRealBoundingHeight;
					pkVert[1] = vRealPos + vCamRight * fRealBoundingWidth;
					pkVert[2] = vRealPos - vCamUp * fRealBoundingHeight;
					pkVert[3] = vRealPos;
				}

				NiPoint2	*pkTextureVert = NiNew NiPoint2[iVertexCount];
				if ( pkTextureVert )
				{
					float const fGab = 1.f / 3;
					float const fP_X = (i % 3) * fGab;
					float const fP_Y = (i / 3) * fGab;

					pkTextureVert[0] = NiPoint2(fP_X + fGab, fP_Y + fGab);
					pkTextureVert[1] = NiPoint2(fP_X + fGab, fP_Y);
					pkTextureVert[2] = NiPoint2(fP_X, fP_Y + fGab);
					pkTextureVert[3] = NiPoint2(fP_X, fP_Y);
				}

				NiColorA	*pkColors = NiNew NiColorA[iVertexCount];
				if ( pkColors )
				{
					pkColors[0] = NiColorA::WHITE;
					pkColors[1] = NiColorA::WHITE;
					pkColors[2] = NiColorA::WHITE;
					pkColors[3] = NiColorA::WHITE;
				}

				NiTriShape *pkTriShape = CreateShape( pkVert, pkColors, pkTextureVert, pkTexture );
				if ( pkTriShape )
				{
					BgTextureBill->AttachChild( pkTriShape, true);
				}
			}
		}
		return BgTextureBill;
	}
	return NULL;
}

NiTriShape*	PgAchievementTitle::CreateShape(NiPoint3 *pkVerts, NiColorA *pkColors, NiPoint2 *pkUVs, NiTexture *pkBaseMap)
{
	int	nTriangles = 2;
	int	iVertexCount = 4;

	unsigned	short	*pusTriangleList = NiAlloc(unsigned short, nTriangles * 3);

	*(pusTriangleList+0) = 0;	
	*(pusTriangleList+1) = 1;	
	*(pusTriangleList+2) = 3;	

	*(pusTriangleList+3) = 3;	
	*(pusTriangleList+4) = 2;	
	*(pusTriangleList+5) = 0;	

	NiTriShape *pkShape = NiNew NiTriShape(iVertexCount,pkVerts,NULL,pkColors,pkUVs,1,NiGeometryData::NBT_METHOD_NONE, nTriangles,pusTriangleList);
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
		m_spAlphaProperty	=	NiNew	NiAlphaProperty();
		m_spAlphaProperty->SetAlphaBlending(true);
		m_spAlphaProperty->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
		m_spAlphaProperty->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
		m_spAlphaProperty->SetAlphaTesting(true);
		m_spAlphaProperty->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);
		m_spAlphaProperty->SetTestRef(10);
	}

	if( !m_spZBufferProperty )
	{
		m_spZBufferProperty	=	NiNew	NiZBufferProperty();
		m_spZBufferProperty->SetZBufferWrite(false);
		m_spZBufferProperty->SetZBufferTest(true);
	}

	if( !m_spVertexColorProperty )
	{
		m_spVertexColorProperty = NiNew NiVertexColorProperty();
		m_spVertexColorProperty->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
		m_spVertexColorProperty->SetSourceMode(NiVertexColorProperty::SOURCE_AMB_DIFF);
	}
	
	NiTexturingPropertyPtr	spTexturing = NiNew NiTexturingProperty();
	if( spTexturing )
	{
		spTexturing->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		spTexturing->SetBaseTexture(pkBaseMap);
		pkShape->AttachProperty(m_spAlphaProperty);
	}

	pkShape->AttachProperty(m_spZBufferProperty);

	pkShape->AttachProperty(m_spMaterialProperty);
	pkShape->AttachProperty(m_spVertexColorProperty);
	pkShape->AttachProperty(spTexturing);

	pkShape->UpdateProperties();
	pkShape->UpdateNodeBound();
	pkShape->Update(0);

	pkShape->SetActiveTriangleCount(0);
    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkShape,false );

	return	pkShape;
}