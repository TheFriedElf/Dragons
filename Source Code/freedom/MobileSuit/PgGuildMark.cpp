#include "stdafx.h"

#include <NiNode.H>
#include "XUI/XUI_Font.h"
#include "PgRenderer.h"
#include "PgActor.h"
#include "PgNifMan.h"
#include "PgPilot.h"
#include "PgMath.h"
#include "Pg2DString.h"
#include "FreedomPool.H"
#include "PgWorld.H"
#include "PgCameraMan.H"
#include "PgOption.h"
#include "PgUIScene.H"
#include "PgInput.h"

#include "PgGuildMark.h"

#include "NewWare/Renderer/DrawUtils.h"
#include "NewWare/Scene/ApplyTraversal.h"


NiImplementRTTI(PgGuildMark, NiGeometry);

NiMaterialPropertyPtr		PgGuildMark::m_spMaterialProperty = NULL;
NiAlphaPropertyPtr			PgGuildMark::m_spAlphaProperty = NULL;
NiZBufferPropertyPtr		PgGuildMark::m_spZBufferProperty = NULL;
NiVertexColorPropertyPtr	PgGuildMark::m_spVertexColorProperty = NULL;
std::string const STR_DEFAULT_GUILD_MARK_IMG = "../Data/6_UI/guild/GuildIcon01.tga";
BYTE						PgGuildMark::m_byGuildIconNumPerRow = 16;
float						PgGuildMark::m_fGuildIconWidth = 1.0f / static_cast<float>(m_byGuildIconNumPerRow); //위 경로의 길드마크 이미지에서 아이콘의 가로 세로 넓이
std::string const STR_DEFAULT_EMPORIA_MARK_IMG = "../Data/6_UI/guild/GuildStar.tga";
extern unsigned int MakeTextureScreenRect(NiScreenTexturePtr& rkTexture, NiPoint2 kTargetPos, NiPoint2 kSize, NiPoint2 const kTexPos, NiPoint2 const kLeftTop, NiPoint2 const kRightBottom);

float PgGuildMark::GetGuildIconWidth()
{
	return m_fGuildIconWidth;
}

BYTE PgGuildMark::GetGuildIconNumPerRow()
{
	return m_byGuildIconNumPerRow;
}

PgGuildMark::PgGuildMark()
	: NiGeometry(NiNew NiTriShapeData(0,0,0,0,0,0,NiGeometryData::NBT_METHOD_NONE,0,0))
	, m_spScreenTexture(NULL)
	, m_spScreenTexture_EMMark(NULL)
{
	Init();
}

PgGuildMark::~PgGuildMark()
{
	Destroy();
}

void PgGuildMark::UpdateWorldBound()
{
	//if(m_pk2DString == NULL)
	//{
	//	m_kWorldBound.SetCenterAndRadius(GetWorldTranslate(),0);
	//	return;
	//}
	m_kWorldBound.SetCenterAndRadius(GetWorldTranslate(),200);
}

void PgGuildMark::Init()
{
	NiAlphaPropertyPtr spAlphaProperty = NiNew NiAlphaProperty();
	spAlphaProperty->SetAlphaBlending(true);
	spAlphaProperty->SetAlphaGroup(AG_EFFECT);

	AttachProperty(spAlphaProperty);

	UpdateProperties();
	UpdateEffects();

	m_kIsDraw = true;
	m_kGuildMarkNo = 0;
	m_kEmporiaMark = 0;
	m_kNameWidth = 0.0f;
	m_fQuadScale = 1.0f;

	if( !m_spScreenTexture )
	{
		NiSourceTexturePtr pTexture = g_kNifMan.GetTexture(STR_DEFAULT_GUILD_MARK_IMG);
		if( pTexture )
		{
			m_spScreenTexture = NiNew NiScreenTexture(pTexture);
		}
	}
}

void PgGuildMark::Set( BYTE const cGuildMarkNo, BYTE const byEmporiaMark, float const fNameWidth)
{
	if (	m_kGuildMarkNo != cGuildMarkNo
		||	m_kNameWidth != fNameWidth
		)
	{
		m_spQuad = NULL;
	}

	m_kNameWidth = fNameWidth;
	if (	byEmporiaMark != m_kEmporiaMark
		||	m_kNameWidth != fNameWidth	
		)
	{
		m_spQuad_EMMark = NULL;
		if ( byEmporiaMark )
		{
			if ( !m_spScreenTexture_EMMark )
			{
				NiSourceTexturePtr pkTexture = g_kNifMan.GetTexture(STR_DEFAULT_EMPORIA_MARK_IMG);
				if( pkTexture )
				{
					m_spScreenTexture_EMMark = NiNew NiScreenTexture(pkTexture);
				}
			}

			if ( m_spScreenTexture_EMMark )
			{
				m_spQuad_EMMark = CreateQuad( m_spScreenTexture_EMMark->GetTexture(), static_cast<float>( (byEmporiaMark-1) % 4 ), static_cast<float>( (byEmporiaMark-1) / 4 ), false );
			}
		}
	}
	
	m_kGuildMarkNo = cGuildMarkNo;
	m_kEmporiaMark = byEmporiaMark;
}

void PgGuildMark::Destroy()
{
	m_kGuildMarkNo = 0;
	m_spScreenTexture = NULL;
	m_spQuad = NULL;

	m_spQuad_EMMark = NULL;
	m_spScreenTexture_EMMark = NULL;
}

void PgGuildMark::DestoryStatic()
{
	m_spMaterialProperty = NULL;
	m_spAlphaProperty = NULL;
	m_spZBufferProperty = NULL;
	m_spVertexColorProperty = NULL;
}

//void PgGuildMark::DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime)
//{
//	if( !m_kIsDraw )			{ return; }
//	if( !m_spScreenTexture )	{ return; }
//	if( !pkRenderer )			{ return; }
//	if( !pkRenderer->GetDefaultRenderTargetGroup() ) { return; }
//	if( !pkCamera )				{ return; }
//	if( !m_pkActor )			{ return; }
//
//	NiNodePtr spDummy = (NiNode*)m_pkActor->GetObjectByName(ATTACH_POINT_STAR);
//	if( !spDummy ) { return; }
//
//	NiPoint3 const kStarPos = spDummy->GetWorldTranslate() + NiPoint3(0.f, 0.f, 12.f);
//	float fX = 0.f, fY = 0.f;
//	bool const bRet = pkCamera->WorldPtToScreenPt(kStarPos, fX, fY);
//	if( !bRet ) { return; }
//
//	int const iScreenWidth = pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0);
//	int const iScreenHeight = pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0);
//
//	NiPoint2 const kIconSize(32.f, 32.f);
//	int const iRowIcon = 16, iColIcon = 16;
//	NiPoint2 const kTexPos( kIconSize.x * (m_kGuildMarkNo % iRowIcon), kIconSize.y * (int)(m_kGuildMarkNo / iColIcon));
//	NiPoint2 kTargetPos(iScreenWidth * fX, iScreenHeight * (1 - fY) );
//
//	kTargetPos.x -= kIconSize.x / 2;
//
//	NiPoint2 kPos, kLeftTop(0.f, 0.f), kRightBottom((float)iScreenWidth, (float)iScreenHeight);
//
//	m_spScreenTexture->RemoveAllScreenRects();
//
//	kPos = kTargetPos;
//	MakeTextureScreenRect(m_spScreenTexture, kPos, kIconSize, kTexPos, kLeftTop, kRightBottom);
//	//MakeTextureScreenRect(m_spScreenTexture, NiPoint2(0.f, 0.f), NiPoint2(28.f, 57.f), NiPoint2(0.f, 0.f), kLeftTop, kRightBottom);
//
//	m_spScreenTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
//
//	PgUIScene::Render_UIObject(pkRenderer, m_spScreenTexture);
//}

void PgGuildMark::RenderImmediate(NiRenderer* pkRenderer)
{
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

	Draw3DPos(pkPgRenderer, pkCamera, kPos);
}

void PgGuildMark::Draw3DPos(PgRenderer* pkRenderer, NiCamera* pkCamera, NiPoint3 const &rkPos)
{
	if( !m_kIsDraw )			{ return; }
	if( !pkRenderer )			{ return; }
	if( !pkCamera )				{ return; }

	if( !m_spQuad )
	{
		if ( m_spScreenTexture )
		{
			m_spQuad = CreateQuad( m_spScreenTexture->GetTexture(), static_cast<float>(m_kGuildMarkNo % 16), static_cast<float>(m_kGuildMarkNo / 16), true );
		}
	}

	if ( m_spQuad )
	{
		if( 0 != m_spQuad->GetArrayCount() )
		{
			m_spZBufferProperty->SetZBufferWrite(true);

			m_spQuad->SetColorLocal( NiColorA::WHITE );
			m_spQuad->SetTranslate(rkPos);
			m_spQuad->Update(0);

			NiVisibleArray	kVisibleArray;
			pkRenderer->CullingProcess_Deprecated( pkCamera, m_spQuad, &kVisibleArray );
            NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), m_spQuad );
		}
	}

	if ( m_spQuad_EMMark )
	{
		if( 0 != m_spQuad_EMMark->GetArrayCount() )
		{
			m_spZBufferProperty->SetZBufferWrite(true);

			m_spQuad_EMMark->SetColorLocal( NiColorA::WHITE );
			m_spQuad_EMMark->SetTranslate(rkPos);
			m_spQuad_EMMark->Update(0);

			NiVisibleArray kVisibleArray;
			pkRenderer->CullingProcess_Deprecated( pkCamera, m_spQuad_EMMark, &kVisibleArray );
            NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), m_spQuad_EMMark );
		}
	}
}

NiBillboardNodePtr PgGuildMark::CreateQuad( NiTexture* pkTexture, float const fRowIcon, float const fCollIcon, bool const bLeft )
{
	if( pkTexture )
	{
		NiBillboardNodePtr spQuad = NiNew NiBillboardNode();
		if ( spQuad )
		{
			spQuad->SetMode(NiBillboardNode::RIGID_FACE_CAMERA);

			float const fScale = m_fQuadScale * 0.2f * 2.f * 0.8f;

			NiPoint2 const kIconSize(32.0f, 32.0f);
			float const fBoundingWidth = kIconSize.x * fScale;
			float const fBoundingHeight = kIconSize.y * fScale;
			NiPoint3 const vCamRight = NiPoint3::UNIT_X;
			NiPoint3 const vCamUp = NiPoint3::UNIT_Y;
			float const fOrgScale = m_fQuadScale * 0.2f;

			NiPoint3 vQuadPos;
			if ( true == bLeft )
			{		
				vQuadPos = -( vCamRight * fBoundingWidth ) - ( vCamRight*m_kNameWidth*fOrgScale/2.0f) + (vCamUp*fBoundingHeight/2.0f);
			}
			else
			{
				vQuadPos = ( vCamRight*m_kNameWidth*fOrgScale/2.0f) + (vCamUp*fBoundingHeight/2.0f);
			}

			NiPoint2 const kTexPos( kIconSize.x * fRowIcon, kIconSize.y * fCollIcon );
			float const fQuadWidth = kIconSize.x * fScale, fQuadHeight = kIconSize.y * fScale;

			size_t const iVertexCount = 4;
			NiPoint3	*pkVert = NiNew NiPoint3[iVertexCount];
			if ( pkVert )
			{
				pkVert[0] = vQuadPos+vCamRight*fQuadWidth-vCamUp*fQuadHeight;
				pkVert[1] = vQuadPos+vCamRight*fQuadWidth;
				pkVert[2] = vQuadPos-vCamUp*fQuadHeight;
				pkVert[3] = vQuadPos;
			}

			NiPoint2	*pkTextureVert = NiNew NiPoint2[iVertexCount];
			if ( pkTextureVert )
			{
				float const fWidth = static_cast<float>(pkTexture->GetWidth());
				float const fHeight = static_cast<float>(pkTexture->GetHeight());
				pkTextureVert[0] = NiPoint2((kTexPos.x+kIconSize.x)/fWidth, (kTexPos.y+kIconSize.y)/fHeight);
				pkTextureVert[1] = NiPoint2((kTexPos.x+kIconSize.x)/fWidth, kTexPos.y/fHeight);
				pkTextureVert[2] = NiPoint2(kTexPos.x/fWidth, (kTexPos.y+kIconSize.y)/fHeight);
				pkTextureVert[3] = NiPoint2(kTexPos.x/fWidth, kTexPos.y/fHeight);
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
				spQuad->AttachChild( pkTriShape, true);
			}
		}
		return spQuad;
	}
	return NULL;
}

NiTriShape*	PgGuildMark::CreateShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap)
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

	NiTexturingPropertyPtr	spTexturing = NiNew NiTexturingProperty();
	spTexturing->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
	spTexturing->SetBaseTexture(pkBaseMap);

	if( !m_spVertexColorProperty )
	{
		m_spVertexColorProperty = NiNew NiVertexColorProperty();
		m_spVertexColorProperty->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
		m_spVertexColorProperty->SetSourceMode(NiVertexColorProperty::SOURCE_AMB_DIFF);
	}
	
	pkShape->AttachProperty(m_spAlphaProperty);
	pkShape->AttachProperty(m_spMaterialProperty);
	pkShape->AttachProperty(m_spVertexColorProperty);
	pkShape->AttachProperty(spTexturing);
	pkShape->AttachProperty(m_spZBufferProperty);


	//if( PgRenderer::GetPgRenderer()
	//&&	PgRenderer::GetPgRenderer()->GetVertexShaderVersion() >= D3DVS_VERSION(1,1) )
	//{
	//	NiShaderPtr spStringScene = NiShaderFactory::GetInstance()->RetrieveShader("StringScene", NiShader::DEFAULT_IMPLEMENTATION, true);
	//	NiSingleShaderMaterial* pkMaterial = NiSingleShaderMaterial::Create(spStringScene); 
	//	pkShape->ApplyAndSetActiveMaterial(pkMaterial);
	//}

	pkShape->UpdateProperties();
	pkShape->UpdateNodeBound();
	pkShape->Update(0);

	pkShape->SetActiveTriangleCount(0);
    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkShape, false );

	return	pkShape;
}