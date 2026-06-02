#include "stdafx.h"
#include "PgHeadBuffIcon.H"
#include "PgUIScene.H"
#include "PgMobileSuit.H"
const	static	float	g_fHeadBuffIconSize = 10.0f;

PgHeadBuffIconListMgr	g_kHeadBuffIconListMgr;

////////////////////////////////////////////////////////////////////////////////
//	class	PgHeadBuffIcon
////////////////////////////////////////////////////////////////////////////////
PgHeadBuffIcon::PgHeadBuffIcon(const	int iEffectNo)
:
m_fPos(0.0f),
m_fTargetScale(0.0f),
m_fStartScale(0.0f),
m_fCurrentScale(0.0f),
m_spIconObj(0),
m_iIconResNo(0),
m_iEffectNo(iEffectNo),
m_kListCenterPos(0,0,0),
m_fStateChangeTime(0.0f),
m_State(PgHeadBuffIcon::S_NONE)
{
	GET_DEF(CEffectDefMgr, kEffectDefMgr);
	const	CEffectDef* pDef = kEffectDefMgr.GetDef(m_iEffectNo);
	PG_WARNING_LOG(pDef);

	if(!pDef)
	{
		return;
	}

	m_iIconResNo = pDef->GetAbil(AT_DEF_RES_NO);

	if(0 == m_iIconResNo)
	{
		return;
	}

	if(!InitIconObj())
	{
		return;
	}

}
PgHeadBuffIcon::~PgHeadBuffIcon()
{
	m_spIconObj = 0;
}
bool	PgHeadBuffIcon::InitIconObj()
{
	NiTexture	*pkSrcTexture = NULL;
	float	fU=0.0f,fV=0.0f,fW=0.0f,fH=0.0f;

	g_kUIScene.GetIconInfo(m_iIconResNo,false,32,32,pkSrcTexture,fU,fV,fW,fH);

	PG_WARNING_LOG(pkSrcTexture);

	if(!pkSrcTexture)
	{
		return false;
	}

	const	float	fIconSize = g_fHeadBuffIconSize;

	const	unsigned	short	usVertCount = 4;
	NiPoint3	*pkVertex = NiNew NiPoint3[usVertCount];
	NiPoint2	*pkTextures = NiNew NiPoint2[usVertCount];
	NiColorA	*pkColors = NiNew NiColorA[usVertCount];
	
	unsigned	short	*pusStripLengths = NiAlloc(unsigned short,1);
	*pusStripLengths = 4;

	unsigned	short	*pusList = NiAlloc(unsigned short,4);

	*(pusList+0) = 0;
	*(pusList+1) = 1;
	*(pusList+2) = 2;
	*(pusList+3) = 3;


	for(int i=0;i<usVertCount;i++)
	{
		*(pkColors+i) = NiColorA(1,1,1,1);
	}

	*(pkVertex+0) = NiPoint3(-fIconSize/2,fIconSize/2,0);
	*(pkVertex+1) = NiPoint3(-fIconSize/2,-fIconSize/2,0);
	*(pkVertex+2) = NiPoint3(fIconSize/2,fIconSize/2,0);
	*(pkVertex+3) = NiPoint3(fIconSize/2,-fIconSize/2,0);

	*(pkTextures+0) = NiPoint2(fU,fV);
	*(pkTextures+1) = NiPoint2(fU,fV+fH);
	*(pkTextures+2) = NiPoint2(fU+fW,fV);
	*(pkTextures+3) = NiPoint2(fU+fW,fV+fH);

	NiTriStripsData	*pkData = NiNew NiTriStripsData(
		usVertCount,pkVertex,NULL,pkColors,pkTextures,1,
		NiGeometryData::NBT_METHOD_NONE,
		2,1,pusStripLengths,pusList);

	m_spIconObj = NiNew NiTriStrips(pkData);

	NiTexturingPropertyPtr	spTexturing = NiNew NiTexturingProperty();
	spTexturing->SetBaseTexture(pkSrcTexture);
	spTexturing->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);

	NiVertexColorPropertyPtr	spVertexColor = NiNew NiVertexColorProperty();
	spVertexColor->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
	spVertexColor->SetSourceMode(NiVertexColorProperty::SOURCE_EMISSIVE);

	NiStencilPropertyPtr	spStencil =	NiNew	NiStencilProperty();
	spStencil->SetDrawMode(NiStencilProperty::DRAW_BOTH);

	NiAlphaPropertyPtr	spAlpha = NiNew NiAlphaProperty();
	spAlpha->SetAlphaBlending(true);
	
	NiMaterialPropertyPtr	spMaterial = NiNew NiMaterialProperty();

	m_spIconObj->AttachProperty(spAlpha);
	m_spIconObj->AttachProperty(spMaterial);
	m_spIconObj->AttachProperty(spStencil);
	m_spIconObj->AttachProperty(spTexturing);
	m_spIconObj->AttachProperty(spVertexColor);

	m_spIconObj->UpdateProperties();
	m_spIconObj->UpdateEffects();
	m_spIconObj->Update(0);

	return	true;
}

void PgHeadBuffIcon::Update(float fAccumTime,float fFrameTime)
{
	const	static	float	fScaleChangeMaxTime = 0.2f;

	switch(m_State)
	{
	case S_APPEARNING:
		{
			const	float	fElapsedTime = fAccumTime - m_fStateChangeTime;
			float	fRate = fElapsedTime/fScaleChangeMaxTime;
			if(fRate>1.0f)
			{
				fRate = 1.0f;
			}
			float	fScaleRate = sin(fRate*90.0f*3.141592f/180.0f);
			
			m_fCurrentScale = m_fStartScale+(m_fTargetScale-m_fStartScale)*fScaleRate;

			if(m_spIconObj)
			{
				NiMaterialPropertyPtr	spMaterial = m_spIconObj->GetPropertyState()->GetMaterial();
				if(spMaterial)
				{
					spMaterial->SetAlpha(fScaleRate);
				}
			}

			if(fRate == 1.0f)
			{
				SetState(S_IDLE);
			}
		}
		break;
	case S_IDLE:
		{
		}
		break;
	case S_DISAPPEARING:
		{
			const	float	fElapsedTime = fAccumTime - m_fStateChangeTime;
			float	fRate = fElapsedTime/fScaleChangeMaxTime;

			if(fRate>1.0f)
			{
				fRate = 1.0f;
			}

			float	fScaleRate = sin(fRate*90.0f*3.141592f/180.0f);

			m_fCurrentScale = m_fStartScale+(m_fTargetScale-m_fStartScale)*fScaleRate;

			if(m_spIconObj)
			{
				NiMaterialPropertyPtr	spMaterial = m_spIconObj->GetPropertyState()->GetMaterial();
				if(spMaterial)
				{
					spMaterial->SetAlpha(fScaleRate);
				}
			}

			if(fRate == 1.0f)
			{
				SetState(S_DISAPPEARED);
			}
		}
		break;
	case S_DISAPPEARED:
		{
		}
		break;
	}

	if(m_spIconObj)
	{
		m_spIconObj->SetScale(m_fCurrentScale);
		m_spIconObj->Update(fAccumTime);
	}
	else
	{
		SetState(S_DISAPPEARED);
	}
}
void PgHeadBuffIcon::DrawImmediate(PgRenderer *pkRenderer,NiCamera *pkCamera)
{
	if(GetNoRender())
	{
		return;
	}

	if(m_spIconObj)
	{
		const	NiPoint3	&vCamRight = pkCamera->GetWorldRightVector();
		const	NiPoint3	&vCamUp = pkCamera->GetWorldUpVector();
		
		//	위치 맞추기
		NiPoint3	kNewPos = m_kListCenterPos+vCamRight*m_fPos;
		m_spIconObj->SetTranslate(kNewPos);

		//	카메라를 보도록 해야한다.
		NiTriStripsData	*pkStripsData =static_cast<NiTriStripsData*>(m_spIconObj->GetModelData());
		if(pkStripsData)
		{
			NiPoint3	*pkVerts = pkStripsData->GetVertices();

			*pkVerts = -vCamRight*g_fHeadBuffIconSize/2.0f+vCamUp*g_fHeadBuffIconSize/2.0f;	pkVerts++;
			*pkVerts = -vCamRight*g_fHeadBuffIconSize/2.0f-vCamUp*g_fHeadBuffIconSize/2.0f;	pkVerts++;
			*pkVerts = vCamRight*g_fHeadBuffIconSize/2.0f+vCamUp*g_fHeadBuffIconSize/2.0f;	pkVerts++;
			*pkVerts = vCamRight*g_fHeadBuffIconSize/2.0f-vCamUp*g_fHeadBuffIconSize/2.0f;	

			pkStripsData->MarkAsChanged(NiGeometryData::VERTEX_MASK);
		}

		//	렌더링
		m_spIconObj->Update(0);
		m_spIconObj->RenderImmediate(pkRenderer->GetRenderer());
	}
}
void	PgHeadBuffIcon::StartAppearing()
{
	if(GetState() == S_APPEARNING)
	{
		return;
	}

	m_fStartScale = m_fCurrentScale;
	m_fTargetScale = 1.0f;

	SetState(S_APPEARNING);
}
void	PgHeadBuffIcon::StartDisappearing()
{
	if(GetState() == S_DISAPPEARING)
	{
		return;
	}

	m_fStartScale = m_fCurrentScale;
	m_fTargetScale = 0.0f;

	SetState(S_DISAPPEARING);
}

void PgHeadBuffIcon::SetState(State kNewState)
{
	if(!g_pkWorld)
	{
		return;
	}
	m_State = kNewState;

	m_fStateChangeTime = g_pkWorld->GetAccumTime();
}

////////////////////////////////////////////////////////////////////////////////
//	class	PgHeadBuffIconList
////////////////////////////////////////////////////////////////////////////////
PgHeadBuffIconList::PgHeadBuffIconList()
:m_kPos(0,0,0)
{
}
PgHeadBuffIconList::~PgHeadBuffIconList()
{
	int	iTotal = m_Icons.size();
	for(int i=0;i<iTotal;i++)
	{
		SAFE_DELETE(m_Icons[i]);
	}
	m_Icons.clear();
}

void PgHeadBuffIconList::Update(float fAccumTime,float fFrameTime)
{

	bool	bDeleted = false;
	PgHeadBuffIcon	*pkIcon = NULL;
	for(HeadBuffIconCont::iterator itor = m_Icons.begin(); itor != m_Icons.end();)
	{
		pkIcon = *itor;

		pkIcon->Update(fAccumTime,fFrameTime);

		if(pkIcon->GetState() == PgHeadBuffIcon::S_DISAPPEARED)
		{
			SAFE_DELETE(pkIcon);
			itor = m_Icons.erase(itor);
			bDeleted = true;
			continue;
		}

		itor++;
	}

	if(bDeleted)
	{
		ArrangeIcons();
	}
}
void PgHeadBuffIconList::DrawImmediate(PgRenderer *pkRenderer,NiCamera *pkCamera)
{
	int	iTotal = m_Icons.size();
	for(int i=0;i<iTotal;i++)
	{
		m_Icons[i]->SetListCenterPos(m_kPos);
		m_Icons[i]->DrawImmediate(pkRenderer,pkCamera);
	}
}

void	PgHeadBuffIconList::AddNewIcon(const	int	iEffectNo)
{
	//	이미 있으면 리턴
	
	PgHeadBuffIcon	*pkNewIcon = GetIcon(iEffectNo);
	if(pkNewIcon)
	{
		pkNewIcon->StartAppearing();
		return;
	}

	pkNewIcon = new PgHeadBuffIcon(iEffectNo);

	//아이콘이 없으면
	if(0 == pkNewIcon->GetIconResNo())
	{
		delete pkNewIcon;
		return;
	}

	pkNewIcon->StartAppearing();

	m_Icons.push_back(pkNewIcon);

	ArrangeIcons();
}
int	PgHeadBuffIconList::GetValidIconCount()
{
	int	iValidCount = 0;
	int	iTotal = m_Icons.size();
	for(int i=0;i<iTotal;i++)
	{
		switch(m_Icons[i]->GetState())
		{
		case PgHeadBuffIcon::S_APPEARNING:
		case PgHeadBuffIcon::S_IDLE:
			{
				++iValidCount;
			}break;
		}
	}
	return	iValidCount;
}
void	PgHeadBuffIconList::RemoveAllIcon()
{
	int	iTotal = m_Icons.size();
	for(int i=0;i<iTotal;i++)
	{
		PG_ASSERT_LOG(m_Icons[i]);
		if (m_Icons[i] != NULL)
			m_Icons[i]->StartDisappearing();
	}
}

void	PgHeadBuffIconList::RemoveIcon(const	int	iEffectNo)
{
	int	iTotal = m_Icons.size();
	for(int i=0;i<iTotal;i++)
	{
		PG_ASSERT_LOG(m_Icons[i]);
		if(m_Icons[i] != NULL && m_Icons[i]->GetEffectNo() == iEffectNo)
		{
			m_Icons[i]->StartDisappearing();
		}
	}
}
void	PgHeadBuffIconList::ArrangeIcons()
{
	const	int	iTotalIcons = m_Icons.size();

	//	최대 보여주는 갯수 맞추기
	const	static	int	iMaxIconInOneList = 5;	

	//	최대 iMaxIconInOneList 개 까지만 유지되어야 한다.
	//	나머지 아이콘들은 렌더링하지 않는다.	
	for(int i=0;i<iTotalIcons;i++)
	{
		if(i<iMaxIconInOneList)
		{
			m_Icons[iTotalIcons-i-1]->SetNoRender(false);
		}
		else
		{
			m_Icons[iTotalIcons-i-1]->SetNoRender(true);
		}
	}

	//	좌표 정렬

	const	int	iTotalVisibleIcons = NiMin(iTotalIcons,iMaxIconInOneList);

	float	fGapSize = 1.0f;
	float	fStartPos = -(g_fHeadBuffIconSize+fGapSize)*(iTotalVisibleIcons-1)/2.0f;

	for(int i=0;i<iTotalVisibleIcons;i++)
	{
		m_Icons[iTotalIcons-i-1]->SetPos(fStartPos+i*(g_fHeadBuffIconSize+fGapSize));
	}	

}
PgHeadBuffIcon*	PgHeadBuffIconList::GetIcon(const	int	iEffectNo)
{
	int	iTotal = m_Icons.size();
	for(int i=0;i<iTotal;i++)
	{
		if(m_Icons[i]->GetEffectNo() == iEffectNo)
		{
			return m_Icons[i];
		}
	}
	return	NULL;
}
void	PgHeadBuffIconList::SetPosition(NiPoint3 const &kPos)
{
	m_kPos = kPos;
}

////////////////////////////////////////////////////////////////////////////////
//	class	PgHeadBuffIconListMgr
////////////////////////////////////////////////////////////////////////////////
PgHeadBuffIconListMgr::PgHeadBuffIconListMgr():m_bAlive(true)
{
}
PgHeadBuffIconListMgr::~PgHeadBuffIconListMgr()
{
	Terminate();
}

void	PgHeadBuffIconListMgr::Terminate()
{
	for(HeadBuffIconListCont::iterator itor = m_IconLists.begin();
		itor != m_IconLists.end(); itor++)
	{
		SAFE_DELETE(*itor);
	}
	m_IconLists.clear();

	m_bAlive = false;
}
void	PgHeadBuffIconListMgr::Init()
{
	m_bAlive = true;
}

PgHeadBuffIconList*	PgHeadBuffIconListMgr::CreateNewIconList()
{
	PgHeadBuffIconList	*pkNew = new PgHeadBuffIconList();
	m_IconLists.push_back(pkNew);
	return	pkNew;
}
void	PgHeadBuffIconListMgr::ReleaseIconList(PgHeadBuffIconList *pkIconList)
{	
	if(!pkIconList)
	{
		return;
	}

	for(HeadBuffIconListCont::iterator itor = m_IconLists.begin();
		itor != m_IconLists.end(); itor++)
	{
		if( (*itor) == pkIconList )
		{
			SAFE_DELETE(pkIconList);
			m_IconLists.erase(itor);
			return;
		}
	}
}

void PgHeadBuffIconListMgr::Update(float fAccumTime,float fFrameTime)
{
	PgHeadBuffIconList	*pkList = NULL;
	for(HeadBuffIconListCont::iterator itor = m_IconLists.begin();
		itor != m_IconLists.end(); itor++)
	{
		pkList = *itor;
		pkList->Update(fAccumTime,fFrameTime);
	}
}
void PgHeadBuffIconListMgr::DrawImmediate(PgRenderer *pkRenderer,NiCamera *pkCamera)
{
	PgHeadBuffIconList	*pkList = NULL;
	for(HeadBuffIconListCont::iterator itor = m_IconLists.begin();
		itor != m_IconLists.end(); itor++)
	{
		pkList = *itor;
		pkList->DrawImmediate(pkRenderer,pkCamera);
	}
}