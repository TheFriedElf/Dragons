#include "stdafx.h"
#include "Pg2DString.h"
#include "PgNifMan.h"
#include "PgStat.h"
#include "PgMobileSuit.h"
#include "PgUIScene.H"
#include "PgMath.H"
#include "PgRenderer.H"

#include "NewWare/Renderer/DrawUtils.h"
#include "NewWare/Scene/ApplyTraversal.h"

using namespace XUI;


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	Pg2DString
/////////////////////////////////////////////////////////////////////////////////////////////////////////

int const ALPHA_CHAR_NUM = 3;
float const ALPHA_TIME_BY_CHAR = 0.2f;

const	int	g_iTexturePageWidth = 256;
const	int	g_iTexturePageHeight = 64;

NiMaterialPropertyPtr	Pg2DString::m_spMaterialProperty = 0;
NiAlphaPropertyPtr	Pg2DString::m_spAlphaProperty = 0;
NiZBufferPropertyPtr	Pg2DString::m_spZBufferProperty = 0;
NiVertexColorPropertyPtr	Pg2DString::m_spVertexColorProperty = 0;

void	Pg2DString::ResetInitTime()
{
	if (m_bDrawByTime)
	{
		m_fStartTime = g_pkApp->GetAccumTime();
	}
	else
	{
		m_fStartTime = static_cast<float>(-1);
	}
}
bool	Pg2DString::GetParsedCharAt(int iTargetX,int iTargetY,XUI::PgParsedChar &kfoundChar)
{

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.Draw"), g_pkApp->GetFrameCount()));

	if (m_StringPieceList.empty())
	{
		return false;
	}
	stStringPiece	*pstrPiece = NULL;

	NiScreenTexturePtr	spScreenTexture = 0;
	for(STR_PIECE_LIST::const_iterator itor = m_StringPieceList.begin(); itor != m_StringPieceList.end(); itor++)
	{
		pstrPiece = *itor;

		spScreenTexture = pstrPiece->m_spScreenTexture;

		NiScreenTexture::ScreenRect	&rt = spScreenTexture->GetScreenRect(0);

		int	iLeft = pstrPiece->m_ptLastRenderPos.x;
		int	iTop = pstrPiece->m_ptLastRenderPos.y;
		if(iLeft<iTargetX && (iLeft+rt.m_usPixWidth)>iTargetX &&
			iTop<iTargetY && (iTop+rt.m_usPixHeight)>iTargetY)
		{

			int	iLockPitch = 0;
			int iDx2 = 0;
			int iDy2 = 0;
			int iAccumHeight = 0;
			const	int	iLen = pstrPiece->m_kString.Length();
			for(int i=0;i<iLen;i++)
			{ 
				PgParsedChar &kChar = pstrPiece->m_kString[i];
				CXUI_Font	*pkFont = kChar.m_FontDef.m_pFont;
				iDy2 = iAccumHeight + kChar.m_iAlignHeight - kChar.m_FontDef.m_pFont->GetHeight();

				if((iLeft+iDx2)<iTargetX && (iLeft+iDx2+kChar.Width())>iTargetX &&
					(iTop+iDy2)<iTargetY && (iTop+iDy2+kChar.m_iAlignHeight)>iTargetY)
				{
					kfoundChar = kChar;
					return	true;
				}

				iDx2+=kChar.Width();

				if( _T('\n') == kChar.m_wChar )
				{
					iAccumHeight += kChar.m_iAlignHeight; //이전 줄 높이 기억
				}
			}

		}
	}


	return	false;
}

NiTriShape*	Pg2DString::CreateShape(NiPoint3 *pkVerts,NiColorA *pkColors,NiPoint2 *pkUVs,NiTexture *pkBaseMap)
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

	NiTriShape *pkShape = NiNew NiTriShape(iVertexCount,pkVerts,NULL,pkColors,pkUVs,1,NiGeometryData::NBT_METHOD_NONE,
		nTriangles,pusTriangleList);
	pkShape->SetConsistency(NiGeometryData::STATIC);

	if(m_spMaterialProperty == 0)
	{
		m_spMaterialProperty = NiNew NiMaterialProperty();
		m_spMaterialProperty->SetAlpha(1.0f);
		m_spMaterialProperty->SetAmbientColor(NiColor::WHITE);
		m_spMaterialProperty->SetDiffuseColor(NiColor::WHITE);
		m_spMaterialProperty->SetEmittance(NiColor::WHITE);
		m_spMaterialProperty->SetSpecularColor(NiColor::BLACK);
	}

	if(m_spAlphaProperty == 0)
	{
		m_spAlphaProperty	=	NiNew	NiAlphaProperty();
		m_spAlphaProperty->SetAlphaBlending(true);
		m_spAlphaProperty->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
		m_spAlphaProperty->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
		m_spAlphaProperty->SetAlphaTesting(true);
		m_spAlphaProperty->SetTestMode(NiAlphaProperty::TEST_GREATER);
		m_spAlphaProperty->SetTestRef(0);
		m_spAlphaProperty->SetAlphaGroup(AG_EFFECT);
	}

	if(m_spZBufferProperty == 0)
	{
		m_spZBufferProperty	=	NiNew	NiZBufferProperty();
		m_spZBufferProperty->SetZBufferWrite(false);
		m_spZBufferProperty->SetZBufferTest(true);
	}

	NiTexturingPropertyPtr	spTexturing = NiNew NiTexturingProperty();
	spTexturing->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
	spTexturing->SetBaseTexture(pkBaseMap);

	if(m_spVertexColorProperty == 0)
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


	if(PgRenderer::GetPgRenderer() &&
		PgRenderer::GetPgRenderer()->GetVertexShaderVersion()>=D3DVS_VERSION(1,1))
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

void	Pg2DString::CreateQuad(float fScale)
{
	PG_ASSERT_LOG(fScale > 0.01f);
	if (fScale <= 0.01f)
	{
		return;
	}

	m_fQuadScale = fScale;

	if(m_StringPieceList.size() == 0)
	{
		m_spQuad = 0;
		return;
	}

	m_spQuad = NiNew NiBillboardNode();
	m_spQuad->SetMode(NiBillboardNode::RIGID_FACE_CAMERA);

	stStringPiece	*pstrPiece = NULL;

	int iLineNum = 0;
	int iLastHeight = 0;
	float	fHeight = 0;
	int iAlignLeft = m_kAlignLeft[iLineNum];

	float	const fBoundingWidth = m_ptSize.x * 0.2f*fScale;
	fScale = fBoundingWidth/m_ptSize.x;

	float	const fBoundingHeight = m_ptSize.y*fScale;

	NiPoint3	const vCamRight = NiPoint3::UNIT_X;
	NiPoint3	const vCamUp = NiPoint3::UNIT_Y;

	NiPoint3	vQuadPos = -(vCamRight*fBoundingWidth/2) + (vCamRight*(iAlignLeft*fScale)) + (vCamUp*fBoundingHeight/2);

	float	fQuadWidth=0.0f,fQuadHeight=0.0f;
	float	fTexWidth = (float)g_iTexturePageWidth,fTexHeight = (float)g_iTexturePageHeight;

	for(STR_PIECE_LIST::iterator itor = m_StringPieceList.begin(); itor != m_StringPieceList.end(); itor++)
	{
		pstrPiece = *itor;
		if(!pstrPiece->m_spScreenTexture)
		{
			m_spQuad = 0;
			return;
		}

		const	int	iLineSize = pstrPiece->m_iLineNum - iLineNum;
		if(iLineSize>0)
		{
			ContAlignLeft::iterator align_iter = m_kAlignLeft.find(pstrPiece->m_iLineNum);
			if( m_kAlignLeft.end() != align_iter )
			{
				iAlignLeft = (*align_iter).second;
			}
			else
			{
				iAlignLeft = 0;
			}

			iLineNum = pstrPiece->m_iLineNum;
			fHeight+=fQuadHeight*iLineSize;

			vQuadPos = -(vCamRight*fBoundingWidth/2) + (vCamRight*(iAlignLeft*fScale))  + (vCamUp*(fBoundingHeight/2-fHeight));
		}

		fQuadWidth = (pstrPiece->m_rtArea.right - pstrPiece->m_rtArea.left)*fScale;
		fQuadHeight = (pstrPiece->m_rtArea.bottom - pstrPiece->m_rtArea.top)*fScale;

		int	iVertexCount = 4;

		NiPoint3	*pkVertex = NiNew NiPoint3[iVertexCount];
		NiPoint2	*pkTexture = NiNew NiPoint2[iVertexCount];
		NiColorA	*pkColors = NiNew NiColorA[iVertexCount];

		NiPoint3	*pkVertStart = pkVertex;
		NiPoint2	*pkTextureStart = pkTexture;
		NiColorA	*pkColorsStart = pkColors;

		*pkVertex = vQuadPos+vCamRight*fQuadWidth-vCamUp*fQuadHeight;	pkVertex++;
		*pkVertex = vQuadPos+vCamRight*fQuadWidth;	pkVertex++;
		*pkVertex = vQuadPos-vCamUp*fQuadHeight;	pkVertex++;
		*pkVertex = vQuadPos;	pkVertex++;

		*pkColors = pstrPiece->m_kColor;	pkColors++;
		*pkColors = pstrPiece->m_kColor;	pkColors++;
		*pkColors = pstrPiece->m_kColor;	pkColors++;
		*pkColors = pstrPiece->m_kColor;	pkColors++;

		*pkTexture = NiPoint2(pstrPiece->m_rtArea.right/fTexWidth,pstrPiece->m_rtArea.bottom/fTexHeight);	pkTexture++;
		*pkTexture = NiPoint2(pstrPiece->m_rtArea.right/fTexWidth,pstrPiece->m_rtArea.top/fTexHeight);	pkTexture++;
		*pkTexture = NiPoint2(pstrPiece->m_rtArea.left/fTexWidth,pstrPiece->m_rtArea.bottom/fTexHeight);	pkTexture++;
		*pkTexture = NiPoint2(pstrPiece->m_rtArea.left/fTexWidth,pstrPiece->m_rtArea.top/fTexHeight);	pkTexture++;

		NiTriShape*	pkTriShape = CreateShape(pkVertStart,pkColorsStart,pkTextureStart,pstrPiece->m_spScreenTexture->GetTexture());
		m_spQuad->AttachChild(pkTriShape, true);

		vQuadPos = vQuadPos+vCamRight*fQuadWidth;
	}
}
void	Pg2DString::Draw_3DPos(PgRenderer *pkRenderer,NiPoint3 const &kTargetPos,NiCamera* pkCamera,const NiColorA &kColor,const NiColorA &kOutlineColor,bool bOutline,float fScale,bool bNoZWrite, DWORD const dwFontOption)
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.Draw_3DPos"), g_pkApp->GetFrameCount()));

	if(m_StringPieceList.size() == 0)
	{
		return;
	}

	CalculateAlignLeft(dwFontOption);

	stStringPiece	*pstrPiece = NULL;

	const NiFrustum &kFrustum = pkCamera->GetViewFrustum();

	if(GetDistanceFromCamPlane(pkCamera,kTargetPos)>kFrustum.m_fFar)	//	카메라 Far 보다 멀리 있으면 그리지 않는다.
	{
		return;
	}

	if( m_bHaveAnimatedLetter )
	{
		bool bHaveAnimatedLetter = false;
		STR_PIECE_LIST::iterator piece_iter = m_StringPieceList.begin();
		while(m_StringPieceList.end() != piece_iter)
		{
			STR_PIECE_LIST::value_type pkStrPiece = (*piece_iter);
			if( pkStrPiece->m_bHasAnimatedFont )
			{
				WriteStringPieceToTexture(pkStrPiece);
			}
			++piece_iter;
		}
	}

	if(!m_spQuad || m_fQuadScale != fScale)
	{
		CreateQuad(fScale);
	}

	if(!m_spQuad)
	{
		return;
	}

	int	iArrayCount = m_spQuad->GetArrayCount();
	if(iArrayCount == 0)
	{
		return;
	}

	m_spZBufferProperty->SetZBufferWrite(bNoZWrite==false);

//	m_spQuad->SetColorLocal(NiColorA(kColor.r,kColor.g,kColor.b,1));
	m_spQuad->SetColorLocal( kColor );
	m_spQuad->SetTranslate(kTargetPos);
	m_spQuad->Update(0);

	NiVisibleArray	kVisibleArray;
	pkRenderer->CullingProcess_Deprecated(pkCamera, (NiNode*)m_spQuad, &kVisibleArray);
    NewWare::Renderer::DrawUtils::DrawImmediate( pkRenderer->GetRenderer(), m_spQuad );
}
void	Pg2DString::Draw(HDC dc,int iScreenWidth,int iScreenHeight,int iScreenX,int iScreenY,const NiColorA &kColor,const NiColorA &kOutlineColor,bool bOutline, DWORD const dwFontOption)
{

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.Draw"), g_pkApp->GetFrameCount()));

	if (m_StringPieceList.empty())
	{
		return;
	}
	stStringPiece	*pstrPiece = NULL;
	if (m_bDrawByTime)
	{
		float const fAccm = g_pkApp->GetAccumTime();	//	fAccm 이 const 인 이유는???
		if (m_fStartTime == -1)
		{
			m_fStartTime = fAccm;
		}

		m_iCurrentCharNum = static_cast<int>(static_cast<float>((fAccm - m_fStartTime)/ALPHA_TIME_BY_CHAR));
		IsDrawingDone( (m_iCurrentCharNum - ALPHA_CHAR_NUM) > static_cast<int>(m_StringPieceList.size()) );
	}
	else
	{
		IsDrawingDone(true);	//천천히 그리는 놈이 아니면 무조건 한번에 다 그림
	}

	int iLineNum = 0;
	int iLastHeight = 0;
	int	iDx=iScreenX,iDy=iScreenY;
	int iNowPiece = 0;
	NiScreenTexturePtr	spScreenTexture = 0;
	for(STR_PIECE_LIST::iterator itor = m_StringPieceList.begin(); itor != m_StringPieceList.end(); itor++)
	{
		pstrPiece = *itor;

		int	const	iLineSize = pstrPiece->m_iLineNum - iLineNum;
		if(iLineSize > 0)
		{
			iLineNum = pstrPiece->m_iLineNum;
			iDy+=iLastHeight*iLineSize;
			iDx = iScreenX;
		}

		spScreenTexture = pstrPiece->m_spScreenTexture;

		NiScreenTexture::ScreenRect	&rt = spScreenTexture->GetScreenRect(0);
		NiScreenTexture::ScreenRect	rtOriginal = rt;
 
		rt.m_sPixLeft = iDx;
		rt.m_sPixTop = iDy; 
		iLastHeight = rt.m_usPixHeight;

		//	클리핑
		if(rt.m_sPixLeft<iScreenWidth && rt.m_sPixTop+rt.m_usPixHeight<iScreenHeight && rt.m_sPixLeft+rt.m_usPixWidth>=0 && rt.m_sPixTop+rt.m_usPixHeight>=0)
		{
			spScreenTexture->MarkAsChanged(NiScreenTexture::VERTEX_MASK | NiScreenTexture::TEXTURE_MASK | NiScreenTexture::COLOR_MASK);

			rt.m_kColor = kColor;

			if (m_iCurrentCharNum - ALPHA_CHAR_NUM >= iNowPiece || !m_bDrawByTime)
			{
				rt.m_kColor.a = kColor.a;	//풀 알파
			}
			else
			{
				float const f = (static_cast<float>(m_iCurrentCharNum - iNowPiece))/(static_cast<float>(ALPHA_CHAR_NUM))*kColor.a +
					((g_pkApp->GetAccumTime() - m_fStartTime) - (static_cast<float>(iNowPiece))*ALPHA_TIME_BY_CHAR)/(ALPHA_TIME_BY_CHAR*m_fAlphaByChar);

				rt.m_kColor.a = __min(f, kColor.a);
			}

			rt.m_kColor = rt.m_kColor*pstrPiece->m_kColor;

			int iXAdjust = 0;
			int iYAdjust = 0;
			if(rt.m_sPixLeft<0)
			{
				iXAdjust = static_cast<int>(-rt.m_sPixLeft);
			}
			if(rt.m_sPixTop<0)
			{
				iYAdjust = static_cast<int>(-rt.m_sPixTop);
			}
			if(rt.m_sPixLeft+rt.m_usPixWidth>=static_cast<unsigned int>(iScreenWidth))
			{
				rt.m_usPixWidth = static_cast<unsigned short>(iScreenWidth - rt.m_sPixLeft -1);
			}
			if(rt.m_sPixTop+rt.m_usPixHeight>=static_cast<unsigned int>(iScreenHeight))
			{
				rt.m_usPixHeight = static_cast<unsigned short>(iScreenHeight - rt.m_sPixTop -1);
			}

			rt.m_sPixLeft += iXAdjust;
			rt.m_sPixTop += iYAdjust;
			rt.m_usPixWidth -= iXAdjust;
			rt.m_usPixHeight -= iYAdjust;
			rt.m_usTexLeft += iXAdjust;
			rt.m_usTexTop +=iYAdjust;

			PgUIScene::Render_DynamicTexture(
				dc,iScreenWidth,iScreenHeight,
				pstrPiece->m_pkDTexture,spScreenTexture,stRenderOption(kOutlineColor,bOutline));
		}

		rt = rtOriginal;

		iDx+=rt.m_usPixWidth;

		if ( m_bDrawByTime && m_iCurrentCharNum <= iNowPiece)
		{
			break;
		}
		++iNowPiece;
	}
}

void	Pg2DString::Draw(PgRenderer *pkRenderer,int iScreenX,int iScreenY,const NiColorA &kColor,const NiColorA &kOutlineColor,bool bOutline, DWORD const dwFontOption)
{

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.Draw"), g_pkApp->GetFrameCount()));

	if (m_StringPieceList.empty())
	{
		return;
	}
	NiScreenTexturePtr	spScreenTexture = NULL;
	NiRenderer	*pkNiRenderer = pkRenderer->GetRenderer();
	stStringPiece	*pstrPiece = NULL;
	if(!pkNiRenderer->GetCurrentRenderTargetGroup())
	{
		return;
	}

	int	iScreenWidth = pkNiRenderer->GetCurrentRenderTargetGroup()->GetWidth(0);
	int	iScreenHeight = pkNiRenderer->GetCurrentRenderTargetGroup()->GetHeight(0);

	if (m_bDrawByTime)
	{
		float const fAccm = g_pkApp->GetAccumTime();	//	fAccm 이 const 인 이유는???
		if (m_fStartTime == -1)
		{
			m_fStartTime = fAccm;
		}

		m_iCurrentCharNum = static_cast<int>(static_cast<float>((fAccm - m_fStartTime)/ALPHA_TIME_BY_CHAR));
		IsDrawingDone( (m_iCurrentCharNum - ALPHA_CHAR_NUM) > static_cast<int>(m_StringPieceList.size()) );
	}
	else
	{
		IsDrawingDone(true);	//천천히 그리는 놈이 아니면 무조건 한번에 다 그림
	}

	CalculateAlignLeft(dwFontOption);

	int iLineNum = 0;
	int iLastHeight = 0;
	int	iDx=iScreenX + m_kAlignLeft[iLineNum];
	int iDy=iScreenY;
	int iNowPiece = 0;
	int iAlignLeft = 0;
	for(STR_PIECE_LIST::iterator itor = m_StringPieceList.begin(); itor != m_StringPieceList.end(); itor++)
	{
		pstrPiece = *itor;

		int	const	iLineSize = pstrPiece->m_iLineNum - iLineNum;
		if(iLineSize> 0)
		{
			ContAlignLeft::iterator align_iter = m_kAlignLeft.find(pstrPiece->m_iLineNum);
			if( m_kAlignLeft.end() != align_iter )
			{
				iAlignLeft = (*align_iter).second;
			}
			else
			{
				iAlignLeft = 0;
			}

			iLineNum = pstrPiece->m_iLineNum;
			iDy+=iLastHeight*iLineSize;
			iDx = iScreenX + iAlignLeft;
		}


		if(pstrPiece->m_bHasAnimatedFont)
		{
			WriteStringPieceToTexture(pstrPiece);
		}

		spScreenTexture = pstrPiece->m_spScreenTexture;
		if(!spScreenTexture)
		{
			return;
		}

		NiScreenTexture::ScreenRect	&rt = spScreenTexture->GetScreenRect(0);
		NiScreenTexture::ScreenRect	rtOriginal = rt;
 
		rt.m_sPixLeft = iDx;
		rt.m_sPixTop = iDy; 
		iLastHeight = rt.m_usPixHeight;

		pstrPiece->m_ptLastRenderPos.x = iDx;
		pstrPiece->m_ptLastRenderPos.y = iDy;

		//	클리핑
		if(rt.m_sPixLeft<iScreenWidth && rt.m_sPixTop+rt.m_usPixHeight<iScreenHeight && rt.m_sPixLeft+rt.m_usPixWidth>=0 && rt.m_sPixTop+rt.m_usPixHeight>=0)
		{
			spScreenTexture->MarkAsChanged(NiScreenTexture::VERTEX_MASK | NiScreenTexture::TEXTURE_MASK | NiScreenTexture::COLOR_MASK);

			rt.m_kColor = kColor;

			if (m_iCurrentCharNum - ALPHA_CHAR_NUM >= iNowPiece || !m_bDrawByTime)
			{
				rt.m_kColor.a = kColor.a;	//풀 알파
			}
			else
			{
				float const f = (static_cast<float>(m_iCurrentCharNum - iNowPiece))/(static_cast<float>(ALPHA_CHAR_NUM))*kColor.a +
					((g_pkApp->GetAccumTime() - m_fStartTime) - (static_cast<float>(iNowPiece))*ALPHA_TIME_BY_CHAR)/(ALPHA_TIME_BY_CHAR*m_fAlphaByChar);

				rt.m_kColor.a = __min(f, kColor.a);
			}

			rt.m_kColor = rt.m_kColor*pstrPiece->m_kColor;

			int iXAdjust = 0;
			int iYAdjust = 0;
			if(rt.m_sPixLeft<0)
			{
				iXAdjust = static_cast<int>(-rt.m_sPixLeft);
			}
			if(rt.m_sPixTop<0)
			{
				iYAdjust = static_cast<int>(-rt.m_sPixTop);
			}
			if(rt.m_sPixLeft+rt.m_usPixWidth>=static_cast<unsigned int>(iScreenWidth))
			{
				rt.m_usPixWidth = static_cast<unsigned short>(iScreenWidth - rt.m_sPixLeft -1);
			}
			if(rt.m_sPixTop+rt.m_usPixHeight>=static_cast<unsigned int>(iScreenHeight))
			{
				rt.m_usPixHeight = static_cast<unsigned short>(iScreenHeight - rt.m_sPixTop -1);
			}

			rt.m_sPixLeft += iXAdjust;
			rt.m_sPixTop += iYAdjust;
			rt.m_usPixWidth -= iXAdjust;
			rt.m_usPixHeight -= iYAdjust;
			rt.m_usTexLeft += iXAdjust;
			rt.m_usTexTop +=iYAdjust;

			PgUIScene::Render_UIObject(pkRenderer,spScreenTexture,PgRenderer::E_PS_NONE,
				stRenderOption(kOutlineColor,bOutline));
		}

		rt = rtOriginal;

		iDx+=rt.m_usPixWidth;

		if ( m_bDrawByTime && m_iCurrentCharNum <= iNowPiece)
		{
			break;
		}
		++iNowPiece;
	}
}


POINT	Pg2DString::GetLinePosition(int iRequestedLineNum)
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.GetLinePosition"), g_pkApp->GetFrameCount()));
	POINT	ptPos;
	SetPoint(ptPos,0,0);

	NiScreenTexturePtr	spScreenTexture = NULL;
	int iLineNum = 0;
	int iLastHeight = 0;

	if(iRequestedLineNum == 0) 
	{
		return ptPos;
	}

	stStringPiece	*pstrPiece = NULL;

	for(STR_PIECE_LIST::iterator itor = m_StringPieceList.begin(); itor != m_StringPieceList.end(); itor++)
	{
		pstrPiece = *itor;

		if(iLineNum != pstrPiece->m_iLineNum)
		{
			iLineNum = pstrPiece->m_iLineNum;

			SetPoint(ptPos, 0, (iLastHeight*iRequestedLineNum));

			if(iLineNum == iRequestedLineNum) 
			{
				return	ptPos;
			}
		}

		spScreenTexture = pstrPiece->m_spScreenTexture;

		NiScreenTexture::ScreenRect	&rt = spScreenTexture->GetScreenRect(0);
		NiScreenTexture::ScreenRect	rtOriginal = rt;
 
		iLastHeight = static_cast<int>(rt.m_usPixHeight);

		ptPos.x+=rt.m_usPixWidth;
	}

	return	ptPos;
}

void	Pg2DString::Add_String_Piece(XUI::CXUI_Style_String &kString,PgDynamicTexture *pkTexture,const RECT &TextAreaInTexture,int iLineNum,DWORD dwFontColor)
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.Add_String_Piece"), g_pkApp->GetFrameCount()));

	const	int	iLen = kString.Length();
	if(iLen == 0)
	{
		return;
	}

	const	int iTextWidth = TextAreaInTexture.right-TextAreaInTexture.left;
	const	int	iTextHeight	=	TextAreaInTexture.bottom-TextAreaInTexture.top;

	bool	bHasAnimatedFont = false;
	PgParsedChar	kChar(_T('\0'),NULL,0,-1);

	for(int i=0;i<iLen;i++)
	{
		kChar = kString[i];
		if(kChar.m_FontDef.m_pFont)
		{
			if(kChar.m_FontDef.m_pFont->IsNeedAnimation(kChar.m_wChar))
			{
				bHasAnimatedFont = true;
				break;
			}
		}
	}

	NiScreenTexturePtr spScreenTexture = NULL;

	if(pkTexture)
	{
		spScreenTexture = NiNew NiScreenTexture(pkTexture->GetTexture());
		spScreenTexture->AddNewScreenRect(0,0,(unsigned short)iTextWidth,(unsigned short)iTextHeight,(unsigned short)TextAreaInTexture.top,(unsigned short)TextAreaInTexture.left);
		spScreenTexture->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		spScreenTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);	
	}

	stStringPiece *pnewPiece = new stStringPiece();
	pnewPiece->m_kString = kString;
	pnewPiece->m_iLineNum = iLineNum;
	pnewPiece->m_spScreenTexture = spScreenTexture;
	pnewPiece->m_rtArea = TextAreaInTexture;
	pnewPiece->m_pkDTexture = pkTexture;
	pnewPiece->m_bHasAnimatedFont = bHasAnimatedFont;

	m_bHaveAnimatedLetter = (bHasAnimatedFont)? bHasAnimatedFont: m_bHaveAnimatedLetter;

	NiColorA	kColor( ((dwFontColor&0x00ff0000)>>16)/256.0f,((dwFontColor&0x0000ff00)>>8)/256.0f,((dwFontColor&0x000000ff))/256.0f,((dwFontColor&0xff000000)>>24)/256.0f);

	pnewPiece->m_kColor = kColor;

	m_StringPieceList.push_back(pnewPiece);
}

void	Pg2DString::Destroy()
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.Destroy"), g_pkApp->GetFrameCount()));
	for(CONT_DYNAMIC_TEXTURE::iterator itor = m_Textures.begin(); itor != m_Textures.end(); itor++)
	{
		g_DynamicTextureMan.ReleaseDynamicTexture(*itor);
	}
	m_Textures.clear();
	m_iTotalUsingTexture = 0;

	for(STR_PIECE_LIST::iterator itor = m_StringPieceList.begin(); itor !=m_StringPieceList.end(); itor++)
	{
		SAFE_DELETE(*itor);
	}

	m_StringPieceList.clear();

	m_spQuad = 0;
}

void	Pg2DString::SetText(const XUI::PgFontDef &kFontDef, std::wstring const &wText,int iLimitWidth,bool bUseWordWrap)
{
	if(m_kText == wText)
	{
		return; 
	}
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.SetText"), g_pkApp->GetFrameCount()));
 
	m_kText.CreateStyleString(kFontDef,wText);
	//m_bBold = bBold;
	//if (fBoldColor)
	//{
	//	int R = fBoldColor[0] * 255;
	//	int G = fBoldColor[1] * 255;
	//	int B = fBoldColor[2] * 255;
	//	int A = fBoldColor[3] * 255;
	//	m_kBoldColor.sARGB = (unsigned short)((A>>4)<<12 | (R>>4)<<8 | (G>>4)<<4 | (B >> 4)); 
	//}

	SetResource(false,false,iLimitWidth,bUseWordWrap);
}

void	Pg2DString::SetText(const XUI::CXUI_Style_String &kText,int iLimitWidth,bool bUseWordWrap)
{
	if(m_kText == kText) return; 
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.SetText"), g_pkApp->GetFrameCount()));

	m_kText = kText;
	//m_bBold = bBold;
	//if (fBoldColor)
	//{
	//	int R = fBoldColor[0] * 255;
	//	int G = fBoldColor[1] * 255;
	//	int B = fBoldColor[2] * 255;
	//	int A = fBoldColor[3] * 255;
	//	m_kBoldColor.sARGB = (A>>4) << 12 | (R>>4)<<8 | (G>>4)<<4 | (B >> 4); 
	//}
	SetResource(false,false,iLimitWidth,bUseWordWrap);
}

void	Pg2DString::WriteStringPieceToTexture(stStringPiece *pkStringPiece)
{

	if(!pkStringPiece || !pkStringPiece->m_bHasAnimatedFont)
	{
		return;
	}

	if(!pkStringPiece->m_pkDTexture)
	{
		return;
	}

	int	iLockPitch = 0;
	BYTE	*pLockBuffer = (BYTE*)pkStringPiece->m_pkDTexture->Lock(iLockPitch);
	if(!pLockBuffer)
	{
		return;
	}

	int iDx = pkStringPiece->m_rtArea.left;
	int iDy = 0;
	int iAccumHeight = pkStringPiece->m_rtArea.top;
	const	int	iLen = pkStringPiece->m_kString.Length();
	_ARGB16	color; 
	color.sARGB = 0xffff;

	for(int i=0;i<iLen;i++)
	{ 
		PgParsedChar &kChar = pkStringPiece->m_kString[i];
		CXUI_Font	*pkFont = kChar.m_FontDef.m_pFont;
		iDy = iAccumHeight + kChar.m_iAlignHeight - kChar.m_FontDef.m_pFont->GetHeight();
		if( pkFont->IsNeedAnimation(kChar.m_wChar) )
		{
			pkFont->SetStyle((XUI::CXUI_Font::FONT_STYLE)kChar.m_FontDef.dwOptionFlag);
			iDx+=pkFont->Draw((_ARGB16*)pLockBuffer,g_iTexturePageWidth*g_iTexturePageHeight,iDx,iDy,iLockPitch/2,kChar.m_wChar,color);
		}
		else
		{
			iDx+=kChar.Width();
		}

		if( _T('\n') == kChar.m_wChar )
		{
			iAccumHeight += kChar.m_iAlignHeight; //이전 줄 높이 기억
		}
	}

	pkStringPiece->m_pkDTexture->UnLock();
}
void	Pg2DString::WriteTextLineToTexture(const POINT &kLineStartPos, XUI::CXUI_Style_String &kText,void *pLockBuffer,int iLockPitch)
{
	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.WriteTextLineToTexture"), g_pkApp->GetFrameCount()));
	_ARGB16	color; 
	color.sARGB = 0xffff;
 
	//int	A = 0, R = 0,G = 0,B = 0;
	int iDx = kLineStartPos.x;
	int iDy = 0;
	for(int i=0;i<(int)kText.Length();i++)
	{ 
		PgParsedChar const &kChar = kText[i];

		color.sARGB = 0xffff ;

		iDy = kLineStartPos.y + kChar.m_iAlignHeight - kChar.m_FontDef.m_pFont->GetHeight();
		if( kChar.m_FontDef.m_pFont->IsEmoticon() )
		{
			//int const iCharHeight = kChar.m_FontDef.m_pFont->GetHeight();
			//iDy = iLineHeight - kChar.m_FontDef.m_pFont->GetHeight();
		}
		else
		{
			//iDy = kLineStartPos.y+iLineHeight/2 - kChar.m_FontDef.m_pFont->GetHeight()/2;
		}

		kChar.m_FontDef.m_pFont->SetStyle((XUI::CXUI_Font::FONT_STYLE)kChar.m_FontDef.dwOptionFlag);
		iDx+=kChar.m_FontDef.m_pFont->Draw((_ARGB16*)pLockBuffer,g_iTexturePageWidth*g_iTexturePageHeight,iDx,iDy,iLockPitch/2,kChar.m_wChar,color);
	}
}
POINT	Pg2DString::CalculateOnlySize(XUI::CXUI_Style_String const &kText,int iLimitWidth,bool bUseWordWrap) 
{
	Pg2DString	kString(kText,false);
	return	kString.SetResource(true,true,iLimitWidth,bUseWordWrap);
}
PgDynamicTexture*	Pg2DString::CreateDynamicTexture(unsigned int uiWidth, unsigned int uiHeight, NiDynamicTexture::FormatPrefs& kPrefs,bool bOnlySystemMem)
{
	if(false == g_DynamicTextureMan.GetAlive())
	{
		return	NULL;
	}

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.CreateDynamicTexture"), g_pkApp->GetFrameCount()));
	int iCount =0 ;
	if((static_cast<int>(m_Textures.size()))>m_iTotalUsingTexture)
	{
		for(CONT_DYNAMIC_TEXTURE::iterator itor = m_Textures.begin(); itor != m_Textures.end(); ++itor)
		{
			if(iCount == m_iTotalUsingTexture)
			{
				m_iTotalUsingTexture++;
				return	*itor;
			}
			iCount++;
		}
		return	NULL;
	}
	PgDynamicTexture	*pTexture	=	g_DynamicTextureMan.CreateDynamicTexture(uiWidth,uiHeight,kPrefs,bOnlySystemMem);
	PG_ASSERT_LOG(pTexture);

	if(!pTexture) 
	{
		return	NULL;
	}
	m_Textures.push_back(pTexture);
	m_iTotalUsingTexture++;

	return	pTexture;
}

void	Pg2DString::ProcessWordWrap(int iLimitWidth,bool bUseWordWrap)
{
	XUI::CXUI_Style_String kResultString;

	m_kText.ProcessWordWrap(iLimitWidth,bUseWordWrap,kResultString);

	m_kText = kResultString;
}

POINT	Pg2DString::SetResource(bool bCalculateOnlySize,bool bOnlySystemMem,int iLimitWidth,bool bUseWordWrap)
{
//	return;
	POINT	kResultSize;
	SetPoint(kResultSize,0,0);

	PG_STAT(PgStatTimerF timer(g_kUIStatGroup.GetStatInfo("Pg2DString.SetResource"), g_pkApp->GetFrameCount()));
	//NILOG(PGLOG_LOG,"Pg2DString::SetResource() Text : %s\n",MB(m_kText.GetOriginalString()));
	//	일단 다 지우고
	for(STR_PIECE_LIST::iterator itor = m_StringPieceList.begin(); itor !=m_StringPieceList.end(); itor++)
	{
		SAFE_DELETE(*itor);
	}
	m_StringPieceList.clear();
	m_iTotalUsingTexture = 0;
	SetPoint(m_ptSize,0,0);
	m_bHaveAnimatedLetter = false;
	m_kAlignLeft.clear();
	
	if(bCalculateOnlySize == false)
	{
		m_spQuad = 0;
	}

	if(iLimitWidth > 0)
	{
		ProcessWordWrap(iLimitWidth,bUseWordWrap);
	}

	int	iTextLength = m_kText.Length();
	int	iTexturePageWidth = g_iTexturePageWidth;
	int	iTexturePageHeight = g_iTexturePageHeight;

	if(iTextLength == 0)	//	길이가 0 이면 리턴
	{
		return kResultSize;
	}

	NiTexture::FormatPrefs kFormat;
	kFormat.m_eAlphaFmt = NiTexture::FormatPrefs::SMOOTH;
	kFormat.m_ePixelLayout = NiTexture::FormatPrefs::HIGH_COLOR_16;

	PgDynamicTexture	*pTexture	=	NULL;
	
	if(!bCalculateOnlySize)
	{
		pTexture = CreateDynamicTexture(iTexturePageWidth,iTexturePageHeight,kFormat,bOnlySystemMem);
		if(!pTexture)
		{
			Destroy();
			return kResultSize;
		}
	}

	int iLockPitch = 0;
	void	*pLockBuff = NULL;

	if(!bCalculateOnlySize)
	{
		pLockBuff = pTexture->Lock(iLockPitch,true);
	}

	//PgParsedChar	kChar(_T(''),NULL,0);

	int	iTextureLineHeight=0;
	int iDrawLineHeight = 0;
	
	XUI::CXUI_Style_String kWorkingString;
	
	int	iPrintLineNum=0;
	int	iCharWidth = 0;
	
	RECT	rtArea;
	SetRectEmpty(&rtArea);
	
	POINT	ptTextureWriteTarget;
	SetPoint(ptTextureWriteTarget,0,0);

	POINT ptWorkingStringSize;
	SetPoint(ptWorkingStringSize,0,0);

	int	iLineWidth = 0;

	DWORD	dwFontColor = 0xffffffff;
	DWORD	dwBeforeFontColor = 0xffffffff;
	bool	bBeforeFontColorSet = false;

	m_kText.RecalculateAlignHeight(); // 최대 높이 계산

	int	iLastSpacePos = -1;

	for( int i=0; iTextLength > i; ++i )
	{
		PgParsedChar &kChar = m_kText[i]; 

		iCharWidth = kChar.Width();

		int	iNextWidth = ptWorkingStringSize.x+iCharWidth;

		if( kChar == _T('\n') )
		{  
			iLineWidth = 0;

			ptWorkingStringSize.y = kWorkingString.GetSize().y;
			if( ptWorkingStringSize.y > iDrawLineHeight )
			{
				iDrawLineHeight = ptWorkingStringSize.y;
			}

			if( ptTextureWriteTarget.y+ptWorkingStringSize.y >= iTexturePageHeight )
			{
				if(!bCalculateOnlySize)
				{
					if( !UnlockAndCreateNewTexturePage(iTexturePageWidth, iTexturePageHeight, kFormat, pTexture, pLockBuff, iLockPitch, bOnlySystemMem) )
					{
						Destroy();
						return kResultSize;
					}

				}
				SetPoint(ptTextureWriteTarget,0,0);
			}

			if( !bCalculateOnlySize )
			{
				//WriteTextLineToTexture(iDrawLineHeight,ptTextureWriteTarget,kWorkingString,pLockBuff,iLockPitch);
				WriteTextLineToTexture(ptTextureWriteTarget,kWorkingString,pLockBuff,iLockPitch);
			}

		
			SetRect(&rtArea, ptTextureWriteTarget.x, ptTextureWriteTarget.y, ptTextureWriteTarget.x+ptWorkingStringSize.x, ptTextureWriteTarget.y+iDrawLineHeight);

			Add_String_Piece(kWorkingString, pTexture, rtArea, iPrintLineNum, dwBeforeFontColor);
			kWorkingString.Clear();
			bBeforeFontColorSet = false;
			ptTextureWriteTarget.x += ptWorkingStringSize.x;

			SetPoint(ptWorkingStringSize,0,0);

			iDrawLineHeight = 0;
			iPrintLineNum++;

			if( kChar.m_wChar!=_T('\n') )
			{
				i--;
			}
			continue;
		}


		if( iNextWidth+ptTextureWriteTarget.x >= iTexturePageWidth )
		{
			ptWorkingStringSize.y = kWorkingString.GetSize().y;
			if( ptWorkingStringSize.y > iDrawLineHeight )
			{
				iDrawLineHeight = ptWorkingStringSize.y;
			}

			if( (ptTextureWriteTarget.y+ptWorkingStringSize.y) >= iTexturePageHeight)
			{
				if( !bCalculateOnlySize )
				{
					if( !UnlockAndCreateNewTexturePage(iTexturePageWidth, iTexturePageHeight, kFormat, pTexture, pLockBuff, iLockPitch, bOnlySystemMem) )
					{
						Destroy();
						return kResultSize;
					}
				}
				SetPoint(ptTextureWriteTarget,0,0);
				i--;
				continue;
			}

			if( !bCalculateOnlySize )
			{
				//WriteTextLineToTexture(iDrawLineHeight,ptTextureWriteTarget,kWorkingString,pLockBuff,iLockPitch);
				WriteTextLineToTexture(ptTextureWriteTarget,kWorkingString,pLockBuff,iLockPitch);
			}

			if( kWorkingString.Length()>0 )
			{	
				SetRect(&rtArea, ptTextureWriteTarget.x, ptTextureWriteTarget.y, ptTextureWriteTarget.x+ptWorkingStringSize.x, ptTextureWriteTarget.y+iDrawLineHeight);

				Add_String_Piece(kWorkingString,pTexture,rtArea,iPrintLineNum,dwBeforeFontColor);
				bBeforeFontColorSet = false;
				kWorkingString.Clear();
				SetPoint(ptWorkingStringSize,0,0);
			}

			if( !bCalculateOnlySize )
			{
				if( !UnlockAndCreateNewTexturePage(iTexturePageWidth, iTexturePageHeight, kFormat, pTexture, pLockBuff, iLockPitch, bOnlySystemMem) )
				{
					Destroy();
					return kResultSize;
				}
				SetPoint(ptTextureWriteTarget,0,0);
			}

			SetPoint(ptTextureWriteTarget,0,ptTextureWriteTarget.y+iDrawLineHeight);
			i--;
			continue;
		}


		if( !bCalculateOnlySize )
		{
			dwFontColor = kChar.m_FontDef.dwColor;

			if( !bBeforeFontColorSet )
			{
				bBeforeFontColorSet = true;
				dwBeforeFontColor = dwFontColor;
			}
			else
			{
				if( dwFontColor != dwBeforeFontColor )
				{
					
					if( 0 < kWorkingString.Length() )
					{
						ptWorkingStringSize.y = kWorkingString.GetSize().y;
						if(ptWorkingStringSize.y>iDrawLineHeight) 
						{
							iDrawLineHeight = ptWorkingStringSize.y;
						}

						if( (ptTextureWriteTarget.y+ptWorkingStringSize.y) >= iTexturePageHeight)
						{
							if(!bCalculateOnlySize)
							{
								if( !UnlockAndCreateNewTexturePage(iTexturePageWidth, iTexturePageHeight, kFormat, pTexture, pLockBuff, iLockPitch, bOnlySystemMem) )
								{
									Destroy();
									return kResultSize;
								}
							}
							SetPoint(ptTextureWriteTarget,0,0);
							i--;
							continue;
						}


						if(!bCalculateOnlySize)
						{
							//WriteTextLineToTexture(iDrawLineHeight,ptTextureWriteTarget,kWorkingString,pLockBuff,iLockPitch);
							WriteTextLineToTexture(ptTextureWriteTarget,kWorkingString,pLockBuff,iLockPitch);
						}

						SetRect(&rtArea, ptTextureWriteTarget.x, ptTextureWriteTarget.y, ptTextureWriteTarget.x+ptWorkingStringSize.x, ptTextureWriteTarget.y+iDrawLineHeight);
				
						Add_String_Piece(kWorkingString,pTexture,rtArea,iPrintLineNum,dwBeforeFontColor);
						dwBeforeFontColor = dwFontColor;
						kWorkingString.Clear();

						ptTextureWriteTarget.x += ptWorkingStringSize.x;

						SetPoint(ptWorkingStringSize,0,0);

					}

				}
			}
		}

		kWorkingString.AddParsedChar(kChar,iCharWidth);
		ptWorkingStringSize.x += iCharWidth;
		iLineWidth += iCharWidth;
	}

	if( ptWorkingStringSize.x > 0 )
	{
		ptWorkingStringSize.y = kWorkingString.GetSize().y;
		if( ptWorkingStringSize.y>iDrawLineHeight )
		{
			iDrawLineHeight = ptWorkingStringSize.y;
		}

		if( (ptTextureWriteTarget.y+ptWorkingStringSize.y) >=iTexturePageHeight )
		{
			if(!bCalculateOnlySize)
			{
				if( !UnlockAndCreateNewTexturePage(iTexturePageWidth, iTexturePageHeight, kFormat, pTexture, pLockBuff, iLockPitch, bOnlySystemMem) )
				{
					Destroy();
					return kResultSize;
				}
			}
			SetPoint(ptTextureWriteTarget,0,0);

		}

		if(!bCalculateOnlySize)
		{
			//WriteTextLineToTexture(iDrawLineHeight,ptTextureWriteTarget,kWorkingString,pLockBuff,iLockPitch);
			WriteTextLineToTexture(ptTextureWriteTarget,kWorkingString,pLockBuff,iLockPitch);
			pTexture->UnLock();
		}

		if(kWorkingString.Length()>0)
		{	
			SetRect(&rtArea,
				ptTextureWriteTarget.x,
				ptTextureWriteTarget.y,
				ptTextureWriteTarget.x+ptWorkingStringSize.x,
				ptTextureWriteTarget.y+iDrawLineHeight);

			Add_String_Piece(kWorkingString,pTexture,rtArea,iPrintLineNum,dwFontColor);
		}
	}
	else
	{
		if(!bCalculateOnlySize)
		{
			pTexture->UnLock();
		}
	}



	int iLineNum = -1;
	int iMaxHeight = 0;
	int	iMaxWidth = 0;
	stStringPiece	*pstrPiece = NULL;
	
	for(STR_PIECE_LIST::iterator itor = m_StringPieceList.begin(); itor != m_StringPieceList.end(); itor++)
	{
		pstrPiece = *itor;

		const	RECT	&kRect = pstrPiece->m_rtArea;

		const	int	iWidth = abs(kRect.right - kRect.left);
		const	int	iHeight = abs(kRect.bottom - kRect.top);

		int	const	iLineSize = pstrPiece->m_iLineNum - iLineNum;
		if(iLineSize > 0)
		{
			iLineNum = pstrPiece->m_iLineNum;
			kResultSize.y+=iMaxHeight*iLineSize;
			if(kResultSize.x<iMaxWidth) 
			{
				kResultSize.x = iMaxWidth;
			}
			iMaxHeight = 0;
			iMaxWidth = 0;
		}

		if(iHeight>iMaxHeight) 
		{
			iMaxHeight = iHeight;
		}
		iMaxWidth+=iWidth;
	}
	if(iMaxWidth>0)
	{
		kResultSize.y+=iMaxHeight;
		if(kResultSize.x<iMaxWidth) 
		{
			kResultSize.x = iMaxWidth;
		}
		iMaxHeight = 0;
		iMaxWidth = 0;
	}

	if(!bCalculateOnlySize)
	{
		m_ptSize = kResultSize;
	}

	if(!bCalculateOnlySize)
	{
		if (m_bDrawByTime)
		{
			PG_ASSERT_LOG(ALPHA_CHAR_NUM > 0 );

			m_fAlphaByChar = 1.0f/(float)ALPHA_CHAR_NUM;	//일단 3단위로 나눈다고 생각하자
			ResetInitTime();
		}
		else
		{
			m_iCurrentCharNum = m_StringPieceList.size();//일단 글자조각들로 하자
		}
		IsDrawingDone(false);
	}

	return	kResultSize;
} 

bool	Pg2DString::UnlockAndCreateNewTexturePage(int iTexWidth,int iTexHeight,NiTexture::FormatPrefs &kFormat,PgDynamicTexture *&pkTexture,void *&pkLockBuff,int &iLockPitch,bool bOnlySystemMem)
{
	PG_ASSERT_LOG(pkTexture);
	if(!pkTexture) 
	{
		Destroy();
		return	false;
	}
	pkTexture->UnLock();
	pkTexture = CreateDynamicTexture(iTexWidth,iTexHeight,kFormat,bOnlySystemMem);	// 문제 있는거 아냐?
	if(!pkTexture)
	{
		Destroy();
		return false;
	}
	pkLockBuff = pkTexture->Lock(iLockPitch,true);
	PG_ASSERT_LOG(pkLockBuff);
	if(!pkLockBuff)
	{
		Destroy();
		return false;
	}
	return true;
}

void Pg2DString::CalculateAlignLeft(DWORD const dwFontOption)
{
	if( !m_kAlignLeft.empty() )
	{
		return;
	}
	//m_kAlignLeft.clear(); // Test Code

	//DWORD dwFontOption = 0;
	//if( 0 == (dwFontOption & (XUI::XTF_ALIGN_CENTER|XUI::XTF_ALIGN_RIGHT)) )
	//{
	//	return;
	//}

	int iLineNum = 0;
	int iMaxAlignValue = 0;
	STR_PIECE_LIST::value_type pkLineStartPiece = (*m_StringPieceList.begin());
	STR_PIECE_LIST::iterator iter = m_StringPieceList.begin();
	size_t iCurCount = 0;
	int iLineWidth = 0;
	while(m_StringPieceList.end() != iter)
	{
		STR_PIECE_LIST::value_type pkCurStrPiece = (*iter);
		if( iLineNum != pkCurStrPiece->m_iLineNum )
		{
			// Start -> Cur-1
			std::inserter(m_kAlignLeft, m_kAlignLeft.end()) = std::make_pair( iLineNum, iLineWidth );
			iLineNum = pkCurStrPiece->m_iLineNum;
			iMaxAlignValue = std::max(iMaxAlignValue, iLineWidth);
			pkLineStartPiece = pkCurStrPiece;
			iLineWidth = 0;
		}
		if( m_StringPieceList.size() == (iCurCount+1) )
		{
			// Start -> cur (Last)
			iLineNum = pkLineStartPiece->m_iLineNum;
			iLineWidth += pkCurStrPiece->m_rtArea.right - pkCurStrPiece->m_rtArea.left;
			std::inserter(m_kAlignLeft, m_kAlignLeft.end()) = std::make_pair( iLineNum, iLineWidth );

			iMaxAlignValue = std::max(iMaxAlignValue, iLineWidth);
		}
		iLineWidth += pkCurStrPiece->m_rtArea.right - pkCurStrPiece->m_rtArea.left;
		++iter;
		++iCurCount;
	}

	ContAlignLeft::iterator align_iter = m_kAlignLeft.begin();
	while(m_kAlignLeft.end() != align_iter)
	{
		if( 0 != (dwFontOption & XUI::XTF_ALIGN_CENTER) )
		{
			(*align_iter).second = (iMaxAlignValue - (*align_iter).second) / 2;
		}
		else if( 0 != (dwFontOption & XUI::XTF_ALIGN_RIGHT) )
		{
			(*align_iter).second = (iMaxAlignValue - (*align_iter).second);
		}
		else
		{
			(*align_iter).second = 0;
		}
		++align_iter;
	}
}

XUI::CXUI_2DString	*gCreate2DStringFunc(const XUI::CXUI_Style_String &kText)
{
 	return	new Pg2DString(kText);  
}

unsigned int gCreateFontFunc(std::wstring const& wstrFontFileName, FT_Library &ftlib, FT_Face *pkFace, FT_Byte*& pkOutPtr)
{
	NiFile	*pkFile = NiFile::GetFile(MB(wstrFontFileName), NiFile::READ_ONLY);
	if(NULL == pkFile)
	{
		return 0;//Cannot_Open_Resource
	}

	unsigned int const uiSize = pkFile->GetFileSize();
	FT_Byte* pkNew = new FT_Byte[uiSize];
	if(NULL == pkNew)
	{
		SAFE_DELETE(pkFile);
		return 0;//Out_Of_Memory
	}

	pkFile->Read(pkNew, uiSize);

	int const iRet = FT_New_Memory_Face(ftlib, pkNew, uiSize, 0, pkFace);
	if(iRet || 0==uiSize)	//0이면 Success
	{	
		SAFE_DELETE(pkNew);
	}
	SAFE_DELETE(pkFile);
	
	pkOutPtr = pkNew;
	return uiSize;
}