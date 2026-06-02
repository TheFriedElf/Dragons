#include "stdafx.h"
#include "PgTextObject.H"
#include "PgRenderer.H"
#include "Pg2DString.H"

NiImplementRTTI(PgTextObject, NiGeometry);

//NiAlphaPropertyPtr PgTextObject::m_spAlphaProperty = NULL;

PgTextObject::PgTextObject()
:NiGeometry(NiNew NiTriShapeData(0,0,0,0,0,0,NiGeometryData::NBT_METHOD_NONE,0,0)),
m_pk2DString(NULL),
m_bDrawShadow(true),
m_kTextColor(1,1,1,1)
{
	NiAlphaPropertyPtr spAlphaProperty = NiNew NiAlphaProperty();
	if ( spAlphaProperty )
	{
		spAlphaProperty->SetAlphaBlending(true);
		spAlphaProperty->SetAlphaGroup(AG_EFFECT);

		AttachProperty(spAlphaProperty);

		UpdateProperties();
		UpdateEffects();
	}
}

void	PgTextObject::Destroy()
{
	SAFE_DELETE(m_pk2DString);
}
void	PgTextObject::UpdateWorldBound()
{
	if(m_pk2DString == NULL)
	{
		m_kWorldBound.SetCenterAndRadius(GetWorldTranslate(),0);
		return;
	}

	m_kWorldBound.SetCenterAndRadius(GetWorldTranslate(),10);
}
void PgTextObject::RenderImmediate(NiRenderer* pkRenderer)
{
	if(!m_pk2DString)
	{
		return;
	}

	PgRenderer	*pkPgRenderer = PgRenderer::GetPgRenderer();
	if(!pkPgRenderer)
	{
		return;
	}

	NiCamera	*pkCamera = pkPgRenderer->GetCameraData();
	if(!pkCamera)
	{
		return;
	}

	NiPoint3	const	&vCamUp = pkCamera->GetWorldUpVector();
	NiPoint3	const	&vCamRight = pkCamera->GetWorldRightVector();

	NiColorA kShadowColor = NiColorA(0.f, 0.f, 0.f, m_kTextColor.a);

	float const	fOutlineThickness = 0.5f;

	NiPoint3 const	kTextPos = GetWorldTranslate();
	NiPoint3 const	kShadowTextPos = kTextPos - vCamUp * fOutlineThickness + vCamRight * fOutlineThickness;

	if(m_bDrawShadow)
	{
		m_pk2DString->Draw_3DPos(pkPgRenderer,kShadowTextPos, pkCamera, kShadowColor,kShadowColor,false, GetScale(), true,XUI::XTF_ALIGN_CENTER);
	}
	m_pk2DString->Draw_3DPos(pkPgRenderer,kTextPos, pkCamera, m_kTextColor,kShadowColor,false, GetScale(), false,XUI::XTF_ALIGN_CENTER);
}

void PgTextObject::SetText(std::wstring const &kText,XUI::CXUI_Font *pFont)
{

	if(!pFont)
	{
		return;
	}
	if(!m_pk2DString)
	{
		m_pk2DString = new Pg2DString(XUI::PgFontDef(pFont, 0xFFFFFFFF, XUI::XTF_ALIGN_CENTER), kText);
	}
	else
	{
		if(m_pk2DString->GetText() == kText)
		{
			return;
		}
		m_pk2DString->SetText(XUI::PgFontDef(pFont),kText);
	}
}
