#include "stdafx.h"

#include <NiNode.H>
#include "PgRenderer.h"
#include "PgWorld.H"
#include "PgCameraMan.H"
#include "PgUIScene.H"
#include "PgNifMan.h"

#include "PgDuelTitle.h"

#include "NewWare/Renderer/DrawUtils.h"
#include "NewWare/Scene/ApplyTraversal.h"


NiImplementRTTI(PgDuelTitle, NiGeometry);


std::string const STR_DEFAULT_WINNER_MARK_IMG = "../Data/6_UI/duel/deMarkWin.tga"; //승자
std::string const STR_DEFAULT_WIN_COUNT_IMG = "../Data/6_UI/duel/deWinNum.tga"; //연승횟수
std::string const STR_DEFAULT_CHAMPION_MARK_IMG = "../Data/6_UI/duel/deMark01.tga"; //챔피온(100연승이상)
NiMaterialPropertyPtr		PgDuelTitle::m_spMaterialProperty = NULL;
NiAlphaPropertyPtr			PgDuelTitle::m_spAlphaProperty = NULL;
NiZBufferPropertyPtr		PgDuelTitle::m_spZBufferProperty = NULL;
NiVertexColorPropertyPtr	PgDuelTitle::m_spVertexColorProperty = NULL;


PgDuelTitle::PgDuelTitle()
	: NiGeometry(NiNew NiTriShapeData(0,0,0,0,0,0,NiGeometryData::NBT_METHOD_NONE,0,0)),
	m_wWins(0),
	m_spWinnerMarkTexture(NULL),
	m_spWinCountTexture(NULL),
	m_spChampionMarkTexture(NULL)
{
	Init();
}

PgDuelTitle::~PgDuelTitle()
{
	Cleanup();
}


void PgDuelTitle::Init(void)
{
	NiAlphaPropertyPtr spAlphaProperty = NiNew NiAlphaProperty();
	spAlphaProperty->SetAlphaBlending(true);
	spAlphaProperty->SetAlphaGroup(AG_EFFECT);

	AttachProperty(spAlphaProperty);

	UpdateProperties();
	UpdateEffects();



	if( !m_spWinnerMarkTexture )
	{
		NiSourceTexturePtr pTexture = g_kNifMan.GetTexture(STR_DEFAULT_WINNER_MARK_IMG);
		if( pTexture )
		{
			m_spWinnerMarkTexture = NiNew NiScreenTexture(pTexture);
		}
	}
	if( !m_spWinCountTexture )
	{
		NiSourceTexturePtr pTexture = g_kNifMan.GetTexture(STR_DEFAULT_WIN_COUNT_IMG);
		if( pTexture )
		{
			m_spWinCountTexture = NiNew NiScreenTexture(pTexture);
		}
	}
	if( !m_spChampionMarkTexture )
	{
		NiSourceTexturePtr pTexture = g_kNifMan.GetTexture(STR_DEFAULT_CHAMPION_MARK_IMG);
		if( pTexture )
		{
			m_spChampionMarkTexture = NiNew NiScreenTexture(pTexture);
		}
	}
}

void PgDuelTitle::Set(WORD wWin)
{
	if(m_spChampionMarkTexture && !m_spChampionMarkQuad)
	{
		m_spChampionMarkQuad = CreateWinnerMark_Quad(NiPoint3(-10.f, 5.f, 0), 20.f, 20.f, m_spChampionMarkTexture->GetTexture());
	}
	if(m_spWinnerMarkTexture && !m_spWinnerMarkQuad)
	{
		m_spWinnerMarkQuad = CreateWinnerMark_Quad(NiPoint3(-18.f, 0.f, 0), 13.f, 13.f, m_spWinnerMarkTexture->GetTexture());
	}
	if(m_spWinCountTexture && m_wWins != wWin)
	{
		m_wWins = wWin;
		m_spWinCountQuad = CreateWinCount_Quad( NiPoint3(-3.f, -1.f, 0), m_spWinCountTexture->GetTexture());
	}
}

void PgDuelTitle::Cleanup(void)
{
	m_spChampionMarkQuad = NULL;
	m_spWinnerMarkQuad = NULL;
	m_spWinCountQuad = NULL;
	m_spWinnerMarkTexture = NULL;
	m_spWinCountTexture = NULL;
	m_spChampionMarkTexture = NULL;
}

void PgDuelTitle::DestoryStatic()
{
	m_spMaterialProperty = NULL;
	m_spAlphaProperty = NULL;
	m_spZBufferProperty = NULL;
	m_spVertexColorProperty = NULL;
}

NiBillboardNodePtr PgDuelTitle::CreateWinnerMark_Quad( NiPoint3 vQuadPos, float fWidth, float fHeight, NiTexture* pkTexture )
{
	if( pkTexture )
	{
		NiBillboardNodePtr spQuad = NiNew NiBillboardNode();
		if ( spQuad )
		{
			spQuad->SetMode(NiBillboardNode::RIGID_FACE_CAMERA);
			size_t const iVertexCount = 4;
			NiPoint3	*pkVert = NiNew NiPoint3[iVertexCount];
			if ( pkVert )
			{
				pkVert[0] = NiPoint3(vQuadPos.x + fWidth, vQuadPos.y - fHeight, 0.f); //가로x세로=60x60이라고 가정
				pkVert[1] = NiPoint3(vQuadPos.x + fWidth, vQuadPos.y, 0.f);
				pkVert[2] = NiPoint3(vQuadPos.x, vQuadPos.y - fHeight, 0.f);
				pkVert[3] = NiPoint3(vQuadPos.x, vQuadPos.y, 0.f);
			}

			NiPoint2	*pkTextureVert = NiNew NiPoint2[iVertexCount];
			if ( pkTextureVert )
			{
				pkTextureVert[0] = NiPoint2(1.f, 1.f);
				pkTextureVert[1] = NiPoint2(1.f, 0.f);
				pkTextureVert[2] = NiPoint2(0.f, 1.f);
				pkTextureVert[3] = NiPoint2(0.f, 0.f);
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

NiBillboardNodePtr PgDuelTitle::CreateWinCount_Quad( NiPoint3 vQuadPos, NiTexture* pkTexture )
{ //표시가능한 최대 자릿수는 두자리이다. 100이상이 절대 실행되면 안된다.
	if( pkTexture )
	{
		float const fWidth = 11.f;
		float const fHeight = 11.f;
		float const fTexOffsetU = 0.2f;
		float const fTexOffsetV = 0.5f;
		NiBillboardNodePtr spQuad = NiNew NiBillboardNode();
		if ( spQuad )
		{
			spQuad->SetMode(NiBillboardNode::RIGID_FACE_CAMERA);

			size_t const iVertexCount = 8; //2자리의 숫자로 만들기 위해 8개의 uv좌표가 필요하므로 정점도 8개로 선언
			NiPoint3	*pkVert = NiNew NiPoint3[iVertexCount];
			if ( pkVert )
			{
				pkVert[0] = NiPoint3(vQuadPos.x + (fWidth * 2), vQuadPos.y - fHeight, 0.f);
				pkVert[1] = NiPoint3(vQuadPos.x + (fWidth * 2), vQuadPos.y, 0.f);
				pkVert[2] = NiPoint3(vQuadPos.x + fWidth, vQuadPos.y - fHeight, 0.f);
				pkVert[3] = NiPoint3(vQuadPos.x + fWidth, vQuadPos.y, 0.f);
				pkVert[4] = NiPoint3(vQuadPos.x + fWidth, vQuadPos.y - fHeight, 0.f); //v4,v5는 v2,v3과 겹침
				pkVert[5] = NiPoint3(vQuadPos.x + fWidth, vQuadPos.y, 0.f);
				pkVert[6] = NiPoint3(vQuadPos.x, vQuadPos.y - fHeight, 0.f);
				pkVert[7] = NiPoint3(vQuadPos.x, vQuadPos.y, 0.f);
			}

			NiPoint2	*pkTextureVert = NiNew NiPoint2[iVertexCount];
			if ( pkTextureVert )
			{
				BYTE by1st = static_cast<BYTE>(m_wWins / 10); //첫번째: 10의 자리
				BYTE by2nd = static_cast<BYTE>(m_wWins % 10); //두번째: 1의 자리
				if(by1st >= 10) //만약 100이상이면
				{
					by1st = by2nd = 0; //그냥 0으로 초기화 시켜버림
				}
				NiPoint2 uv1((by1st % 5) * fTexOffsetU, (by1st / 5) * fTexOffsetV);
				NiPoint2 uv2((by2nd % 5) * fTexOffsetU, (by2nd / 5) * fTexOffsetV);

				pkTextureVert[7] = NiPoint2(uv1.x, uv1.y);
				pkTextureVert[6] = NiPoint2(uv1.x, uv1.y + fTexOffsetV);
				pkTextureVert[5] = NiPoint2(uv1.x + fTexOffsetU, uv1.y);
				pkTextureVert[4] = NiPoint2(uv1.x + fTexOffsetU, uv1.y + fTexOffsetV);
				pkTextureVert[3] = NiPoint2(uv2.x, uv2.y);
				pkTextureVert[2] = NiPoint2(uv2.x, uv2.y + fTexOffsetV);
				pkTextureVert[1] = NiPoint2(uv2.x + fTexOffsetU, uv2.y);
				pkTextureVert[0] = NiPoint2(uv2.x + fTexOffsetU, uv2.y + fTexOffsetV);
			}

			NiColorA	*pkColors = NiNew NiColorA[iVertexCount];
			if ( pkColors )
			{
				pkColors[0] = NiColorA::WHITE;
				pkColors[1] = NiColorA::WHITE;
				pkColors[2] = NiColorA::WHITE;
				pkColors[3] = NiColorA::WHITE;
				pkColors[4] = NiColorA::WHITE;
				pkColors[5] = NiColorA::WHITE;
				pkColors[6] = NiColorA::WHITE;
				pkColors[7] = NiColorA::WHITE;
			}

			NiTriShape *pkTriShape = CreateNumShape( pkVert, pkColors, pkTextureVert, pkTexture );
			if ( pkTriShape )
			{
				spQuad->AttachChild( pkTriShape, true);
			}
		}
		return spQuad;
	}
	return NULL;
}

NiTriShape*	PgDuelTitle::CreateShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap)
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
	if(pkShape)
	{
		pkShape->SetConsistency(NiGeometryData::STATIC);
		ApplyDefaultMaterial(pkShape, pkBaseMap);
	}

	return	pkShape;
}

NiTriShape*	PgDuelTitle::CreateNumShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap)
{
	int	nTriangles = 4;
	int	iVertexCount = 8;

	unsigned	short	*pusTriangleList = NiAlloc(unsigned short, nTriangles * 3);

	*(pusTriangleList+0) = 4;	
	*(pusTriangleList+1) = 5;	
	*(pusTriangleList+2) = 7;	

	*(pusTriangleList+3) = 7;	
	*(pusTriangleList+4) = 6;	
	*(pusTriangleList+5) = 4;	

	*(pusTriangleList+6) = 0;	
	*(pusTriangleList+7) = 1;	
	*(pusTriangleList+8) = 3;	

	*(pusTriangleList+9) = 3;	
	*(pusTriangleList+10) = 2;	
	*(pusTriangleList+11) = 0;	

	NiTriShape *pkShape = NiNew NiTriShape(iVertexCount,pkVerts,NULL,pkColors,pkUVs,1,NiGeometryData::NBT_METHOD_NONE, nTriangles,pusTriangleList);
	if(pkShape)
	{
		pkShape->SetConsistency(NiGeometryData::STATIC);
		ApplyDefaultMaterial(pkShape, pkBaseMap);
	}

	return	pkShape;
}

void PgDuelTitle::ApplyDefaultMaterial(NiTriShape *pkShape, NiTexture *pkBaseMap)
{
	if(!pkShape)
	{
		return;
	}

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


	pkShape->UpdateProperties();
	pkShape->UpdateNodeBound();
	pkShape->Update(0);

	pkShape->SetActiveTriangleCount(0);
    NewWare::Scene::ApplyTraversal::Geometry::SetDefaultMaterialNeedsUpdateFlag( pkShape, false );
}

void PgDuelTitle::UpdateWorldBound(void)
{
	m_kWorldBound.SetCenterAndRadius(GetWorldTranslate(), 60);
}

void PgDuelTitle::RenderImmediate(NiRenderer* pkRenderer)
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

void PgDuelTitle::Draw3DPos(PgRenderer* pkRenderer, NiCamera* pkCamera, NiPoint3 const &rkPos)
{
	if( !pkRenderer )			{ return; }
	if( !pkCamera )				{ return; }

	if(m_wWins <= 0)
	{
		return;
	}
	else if(m_wWins >= 100)
	{ //100승이상이라면 챔피언 마크만 표시
		if( !m_spChampionMarkQuad )
		{
			if ( m_spChampionMarkTexture )
			{
				m_spChampionMarkQuad = CreateWinnerMark_Quad(NiPoint3(-10.f, 5.f, 0), 20.f, 20.f, m_spChampionMarkTexture->GetTexture());
			}
		}

		if ( m_spChampionMarkQuad )
		{
			if( 0 != m_spChampionMarkQuad->GetArrayCount() )
			{
				m_spZBufferProperty->SetZBufferWrite(true);

				m_spChampionMarkQuad->SetColorLocal( NiColorA::WHITE );
				m_spChampionMarkQuad->SetTranslate(rkPos);
				m_spChampionMarkQuad->Update(0);

				NiVisibleArray	kVisibleArray;
				pkRenderer->CullingProcess_Deprecated( pkCamera, m_spChampionMarkQuad, &kVisibleArray );
				NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), m_spChampionMarkQuad );
			}
		}
	}
	else
	{ //1~99승이라면 숫자로 표시
		if( !m_spWinnerMarkQuad )
		{
			if ( m_spWinnerMarkTexture )
			{
				m_spWinnerMarkQuad = CreateWinnerMark_Quad(NiPoint3(-18.f, 0.f, 0), 10.f, 10.f, m_spWinnerMarkTexture->GetTexture());
			}
		}

		if ( m_spWinnerMarkQuad )
		{
			if( 0 != m_spWinnerMarkQuad->GetArrayCount() )
			{
				m_spZBufferProperty->SetZBufferWrite(true);

				m_spWinnerMarkQuad->SetColorLocal( NiColorA::WHITE );
				m_spWinnerMarkQuad->SetTranslate(rkPos);
				m_spWinnerMarkQuad->Update(0);

				NiVisibleArray	kVisibleArray;
				pkRenderer->CullingProcess_Deprecated( pkCamera, m_spWinnerMarkQuad, &kVisibleArray );
				NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), m_spWinnerMarkQuad );
			}
		}

		if( !m_spWinCountQuad )
		{
			if ( m_spWinCountTexture )
			{
				m_spWinCountQuad = CreateWinCount_Quad( NiPoint3(-3.f, -1.f, 0), m_spWinCountTexture->GetTexture());
			}
		}

		if ( m_spWinCountQuad )
		{
			if( 0 != m_spWinCountQuad->GetArrayCount() )
			{
				m_spZBufferProperty->SetZBufferWrite(true);

				m_spWinCountQuad->SetColorLocal( NiColorA::WHITE );
				m_spWinCountQuad->SetTranslate(rkPos);
				m_spWinCountQuad->Update(0);

				NiVisibleArray	kVisibleArray;
				pkRenderer->CullingProcess_Deprecated( pkCamera, m_spWinCountQuad, &kVisibleArray );
				NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), m_spWinCountQuad );
			}
		}

	}

}

