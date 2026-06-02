#include "stdafx.h"
#include "PgRope.H"
#include "PgNifMan.H"

NiImplementRTTI(PgRope, NiNode);

PgRope::PgRope() : m_fRange(1.0f)
{
}

void	PgRope::SetRopeData(NiPoint3 const &kStartPos,NiPoint3 const &kEndPos,float fRange,char const *pkTexturePath)
{
	if(m_kStartPos == kStartPos && m_kEndPos == kEndPos && m_fRange == fRange)
	{
		return;
	}

	m_kStartPos = kStartPos;
	m_kEndPos = kEndPos;

	m_fRange = fRange;

	if(kStartPos == kEndPos || fRange<=0)
	{
		return;
	}

	m_vDir = m_kEndPos - m_kStartPos;
	m_vDir.Unitize();

	if(m_vDir == NiPoint3::UNIT_Z || m_vDir == -NiPoint3::UNIT_Z)	//	수직인 로프는 생성 할 수 없다.
	{	
		return;
	}

	m_vRight = m_vDir.Cross(NiPoint3::UNIT_Z);
	m_vRight.Unitize();

	m_vUp = m_vRight.Cross(m_vDir);
	m_vUp.Unitize();

	SetGeometry(pkTexturePath);

}
NiTriShape*	PgRope::SetGeometry(char const *pkTexturePath)
{

	//	기존의 지오메트리를 모두 제거한다.
	unsigned	int	uiTotalChild = GetArrayCount();
	for(unsigned int i=0;i<uiTotalChild;i++)
	{
		NiAVObject	*pkChild = GetAt(i);
		if(pkChild)
		{
			DetachChildAt(i);
		}
	}

	//	텍스쳐 생성
	std::string	kSrcTexturePath("../Data/5_Effect/9_Tex/Rope_01.dds");
	if(pkTexturePath && strlen(pkTexturePath)>0)
	{
		kSrcTexturePath = pkTexturePath;
	}
	NiSourceTexture	*pkTexture = g_kNifMan.GetTexture(kSrcTexturePath);

	//	새 지오메트리를 만들자.
	unsigned	short	const	usTotalDivide = 10;
	if(usTotalDivide<3)		//	무조건 3개 이상이어야 한다.
	{
		return NULL;
	}

	unsigned	short	const	usTotalTriangle = 4*(usTotalDivide-1);
	unsigned	short	const	usTotalVertex = usTotalDivide*2;
	int	const	iTotalIndex = usTotalTriangle*3;

	NiPoint3	*pkVertex = NiNew NiPoint3[usTotalVertex];
	unsigned	short	*pkIndex = NiAlloc(unsigned short,iTotalIndex);

	NiPoint2	*pkUV = pkTexture ? (NiNew NiPoint2[usTotalVertex]) : NULL;
	unsigned	short	const	usTotalUVSet = pkTexture ? 1 : 0;

	float	fFinalRightU = 1.0f;
	if(pkTexture)
	{
		int	const	iTextureWidth = pkTexture->GetWidth();
		int	const	iTextureHeight = pkTexture->GetHeight();

		float	const	fCapCircleLength = 2.0f*3.141592f*m_fRange*m_fRange;
		float	const	fTextureLengthScale = fCapCircleLength/iTextureHeight;

		float	const	fTextureWidthScaled = iTextureWidth * fTextureLengthScale;
		float	const	fRopeLength = (m_kEndPos - m_kStartPos).Length();
		int	const	iQ = (int)(fRopeLength/fTextureWidthScaled);
		float	const	fRemainLength = fRopeLength - iQ * fTextureWidthScaled;
		
		fFinalRightU = iQ+fRemainLength/fTextureWidthScaled;
	}

	//	버텍스,UV 생성
	float	fRotateAngle = 360.0f/usTotalDivide*3.141592f/180.0f;
	NiPoint3	vUp = m_vUp * m_fRange;
	for(int i=0;i<2;i++)
	{
		NiPoint3	const &kCenterPos = (i == 0) ? m_kStartPos : m_kEndPos;
		
		for(int j=0;j<usTotalDivide;j++)
		{
			
			NiMatrix3	kRotMat;
			NiQuaternion	kRotQuat(fRotateAngle * j,m_vDir);
			kRotQuat.ToRotation(kRotMat);

			*(pkUV+i*usTotalDivide+j) = NiPoint2(i==0 ? 0 : fFinalRightU,j*(1.0f/(usTotalDivide-1)));

			NiMatrix3::TransformVertices(kRotMat,kCenterPos,1,&vUp,pkVertex+i*usTotalDivide+j);
		}
	}

	//	인덱스 생성
	int	iTriCount = usTotalDivide - 2;
	for(int i=0;i<iTriCount;i++)
	{
		//	앞마개
		*(pkIndex+6*i+0) = i+2;
		*(pkIndex+6*i+1) = i+1;
		*(pkIndex+6*i+2) = 0;

		//	뒷마개
		*(pkIndex+6*i+3) = usTotalDivide;
		*(pkIndex+6*i+4) = usTotalDivide+i+1;
		*(pkIndex+6*i+5) = usTotalDivide+i+2;
	}
	//	둘레
	int	iIndexStart = 6*iTriCount;
	for(int i=0;i<usTotalDivide;i++)
	{
		if(i < usTotalDivide -1 )
		{
			*(pkIndex+iIndexStart+6*i+0) = i+1;
			*(pkIndex+iIndexStart+6*i+1) = usTotalDivide + i;
			*(pkIndex+iIndexStart+6*i+2) = i;

			*(pkIndex+iIndexStart+6*i+3) = i+1;
			*(pkIndex+iIndexStart+6*i+4) = usTotalDivide+i+1;
			*(pkIndex+iIndexStart+6*i+5) = usTotalDivide+i;
		}
		else
		{
			*(pkIndex+iIndexStart+6*i+0) = 0;
			*(pkIndex+iIndexStart+6*i+1) = usTotalDivide + i;
			*(pkIndex+iIndexStart+6*i+2) = i;

			*(pkIndex+iIndexStart+6*i+3) = 0;
			*(pkIndex+iIndexStart+6*i+4) = usTotalDivide;
			*(pkIndex+iIndexStart+6*i+5) = usTotalDivide+i;
		}
	}

	NiTriShapeData	*pkShapeData = NiNew NiTriShapeData(usTotalVertex,
		pkVertex,NULL,NULL,pkUV,usTotalUVSet,NiGeometryData::NBT_METHOD_NONE,
		usTotalTriangle,pkIndex);

	NiTriShape	*pkTriShape = NiNew NiTriShape(pkShapeData);

	NiStencilProperty	*pkStencil = NiNew NiStencilProperty();
	pkStencil->SetDrawMode(NiStencilProperty::DRAW_CCW);
	pkTriShape->AttachProperty(pkStencil);

	NiMaterialProperty	*pkMaterial = NiNew NiMaterialProperty();
	pkMaterial->SetAlpha(1.0f);
	pkMaterial->SetAmbientColor(NiColor::WHITE);
	pkMaterial->SetDiffuseColor(NiColor::WHITE);
	pkMaterial->SetEmittance(NiColor::WHITE);
	pkTriShape->AttachProperty(pkMaterial);

	if(pkTexture)
	{
		NiTexturingProperty	*pkTexturing =  NiNew NiTexturingProperty();

		pkTexturing->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		pkTexturing->SetBaseClampMode(NiTexturingProperty::WRAP_S_WRAP_T);
		pkTexturing->SetBaseFilterMode(NiTexturingProperty::FILTER_TRILERP);

		pkTexturing->SetBaseTexture(pkTexture);
		pkTriShape->AttachProperty(pkTexturing);

	}

	pkTriShape->UpdateProperties();
	pkTriShape->UpdateEffects();

	AttachChild(pkTriShape, true);

	return	pkTriShape;

}
