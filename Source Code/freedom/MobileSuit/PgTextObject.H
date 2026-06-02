#ifndef FREEDOM_DRAGONICA_RENDER_EFFECT_PGTEXTOBJECT_H
#define FREEDOM_DRAGONICA_RENDER_EFFECT_PGTEXTOBJECT_H

#include <NiNode.H>
#include "Pg2DString.H"

class	XUI::CXUI_Font;
class	PgTextObject	:	public	NiGeometry
{
	NiDeclareRTTI;

public:

	PgTextObject();
	virtual	~PgTextObject()	{	Destroy();	}

	virtual void RenderImmediate(NiRenderer* pkRenderer);

	void	SetTextColor(NiColorA const &kColor)	{	m_kTextColor = kColor;	}
	void	SetText(std::wstring const &kText,XUI::CXUI_Font *pFont);
	void	SetAlpha( float const fAlpha ){m_kTextColor.a = fAlpha;}

	long	GetTextWidth()
	{
		
		if(m_pk2DString)
		{
			return	m_pk2DString->GetSize().x;
		}

		return	-1;
	}

	void	SetDrawShadow(bool bDrawShadow)	{	m_bDrawShadow = bDrawShadow;	}

	virtual	void	UpdateWorldBound();
private:

	void	Destroy();

public:

//	static	NiAlphaPropertyPtr	m_spAlphaProperty;

private:

	NiColorA	m_kTextColor;
	bool	m_bDrawShadow;

	Pg2DString	*m_pk2DString;
};

NiSmartPointer(PgTextObject);

#endif // FREEDOM_DRAGONICA_RENDER_EFFECT_PGTEXTOBJECT_H