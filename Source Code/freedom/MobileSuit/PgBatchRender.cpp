#include "stdafx.h"
#include "PgBatchRender.H"
#include "PgRenderer.H"
///////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgBatchRenderExtraData
///////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PgBatchRenderExtraData::m_kDataID = "__BATCH_RENDER_INFO";
NiImplementRTTI(PgBatchRenderExtraData, NiExtraData);

///////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgBatchRender
///////////////////////////////////////////////////////////////////////////////////////////////////////

int	PgBatchRender::m_iBatchRenderInstanceID=0;
PgBatchRender::PgBatchRender()
:m_fLastBatchGroupCleanUpTime(0)
{
	Init();
}
PgBatchRender::~PgBatchRender()
{
	ReleaseAll();
}
void	PgBatchRender::Reset()
{
	ReleaseAll();
	Init();
}
void	PgBatchRender::Init()
{
	m_iMaxNumBatchNodeArray = 300;
	m_iNumBatchNode = 0;
	m_iMaxBatchNodeID = 0;
	m_iAddCheckValue = 0;

	IncreaseBatchRenderInstanceID();
}
void	PgBatchRender::ReleaseAll()
{
	ClearVisibleBatchGroup();
	ReleaseAllBatchGroup();
}
void	PgBatchRender::ReleaseAllBatchGroup()
{
	for(int i=0;i<m_kBatchGroupCont.size();++i)
	{
		SAFE_DELETE_NI(m_kBatchGroupCont[i]);
	}
	m_kBatchGroupCont.clear();
	m_kDisposedBatchGroupIDCont.clear();
}
bool	PgBatchRender::AddGeometry(NiGeometry *pkGeom)
{
	if(!pkGeom)
	{
		return true;
	}

	NiPropertyState	*pkProperty = pkGeom->GetPropertyState();


	NiDynamicEffectState *pkEffect = pkGeom->GetEffectState();
	PgBatchRenderExtraData	*pkBatchData = NiDynamicCast(PgBatchRenderExtraData,pkGeom->GetExtraData(PgBatchRenderExtraData::m_kDataID.c_str()));

	if(!pkBatchData)
	{
		pkBatchData = NiNew PgBatchRenderExtraData();
		pkGeom->AddExtraData(PgBatchRenderExtraData::m_kDataID.c_str(),pkBatchData);
	}

	if(pkBatchData->GetBatchGroupID() == BATCH_GROUP_ID_NO_USE_BATCH)
	{
		return	false;
	}

	if(pkBatchData->GetBatchGroupID() == BATCH_GROUP_ID_RESET_BATCH)
	{
		if(PgRenderer::HasGeomMorphController(pkGeom) ||
			PgRenderer::HasTextureTransformController(pkGeom)
			)	//	A geometry that has a geometry morph controller cannot be rendered using batch rendering
		{
			pkBatchData->SetBatchGroupID(BATCH_GROUP_ID_NO_USE_BATCH);
			return	false;
		}
	}

	bool	bFullCompare = false;
	if(pkBatchData->GetBatchRenderInstanceID() != m_iBatchRenderInstanceID
		|| pkBatchData->GetBatchGroupID() == BATCH_GROUP_ID_RESET_BATCH)
	{
		bFullCompare = true;
	}

	int	iBatchGroupID = pkBatchData->GetBatchGroupID();
	stBatchGroup	*pkBatchGroup = 0;

	if(!bFullCompare && m_kBatchGroupCont[iBatchGroupID]->GetInstanceID() != pkBatchData->GetBatchGroupInstanceID())
	{
		bFullCompare = true;
	}	

	if(!bFullCompare)
	{
		pkBatchGroup = m_kBatchGroupCont[iBatchGroupID];
	}
	else
	{

		pkBatchGroup = GetSameBatchGroup(pkGeom,pkProperty,pkEffect);
		if(!pkBatchGroup)
		{
			if(m_kDisposedBatchGroupIDCont.size() > 0)
			{
				iBatchGroupID = ReviveDisposedBatchGroup();
				pkBatchGroup = m_kBatchGroupCont[iBatchGroupID];
			}
			else
			{
				iBatchGroupID = m_kBatchGroupCont.size();
				pkBatchGroup = NiNew stBatchGroup();
				m_kBatchGroupCont.push_back(pkBatchGroup);
			}

			pkBatchGroup->Clear();
			pkBatchGroup->m_iBatchGroupID = iBatchGroupID;

			if(pkProperty)
			{
				pkBatchGroup->m_spPropertyState = PgRenderer::CopyPropertyState(pkProperty);
			}
			if(pkEffect)
			{
				pkBatchGroup->m_spEffectState = pkEffect->Copy();
			}
			pkBatchGroup->m_bColor = (pkGeom->GetColors() != NULL);
			pkBatchGroup->m_bNormal = (pkGeom->GetNormals() != NULL);
			pkBatchGroup->m_bSkinning = (pkGeom->GetSkinInstance() != NULL);
			pkBatchGroup->m_iTextureSetCount = pkGeom->GetTextureSets();
			
		}

		pkBatchData->SetBatchRenderInstanceID(m_iBatchRenderInstanceID);
		pkBatchData->SetBatchGroupID(pkBatchGroup->m_iBatchGroupID);
		pkBatchData->SetBatchGroupInstanceID(pkBatchGroup->GetInstanceID());
	}
	if(!pkBatchGroup)
	{
		return false;
	}

	if(pkBatchGroup->m_iAddCheckValue != m_iAddCheckValue)
	{
		pkBatchGroup->m_iAddCheckValue = m_iAddCheckValue;
		m_kVisibleBatchGroupCont.push_back(pkBatchGroup->m_iBatchGroupID);
	}

	pkBatchGroup->m_kNodes.Add(*pkGeom);

	return	true;
}
void	PgBatchRender::AddObject(NiAVObject *pkAVObject)
{
	if(!pkAVObject)
	{
		return;
	}

	NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int	iCount = pkNode->GetArrayCount();
		for(int i=0;i<iCount;i++)
		{
			NiAVObject	*pkChild  = pkNode->GetAt(i);
			if(pkChild)
			{
				AddObject(pkChild);
			}
		}

		return;
	}

	NiGeometry	*pkGeom = NiDynamicCast(NiGeometry,pkAVObject);
	if(pkGeom)
	{
		AddGeometry(pkGeom);
	}
}


int PgBatchRender::MakeNewBatchRenderID()
{
	return ++m_iMaxBatchNodeID;
}

void	PgBatchRender::Draw(PgRenderer *pkRenderer, NiCamera *pkCamera,float fAccumTime)
{


	NiDX9Renderer	*pkDX9Renderer = (NiDX9Renderer*)pkRenderer->GetRenderer();
	if(!pkDX9Renderer)
	{
		return;
	}

	for(int j=0;j<m_kVisibleBatchGroupCont.size();j++)
	{
		stBatchGroup	*pkBatchGroup = m_kBatchGroupCont[m_kVisibleBatchGroupCont[j]];
		int	iObjCount = pkBatchGroup->m_kNodes.GetCount();

		if(iObjCount == 0)
		{
			continue;
		}

		pkBatchGroup->SetLastUsedTime(fAccumTime);
		if(iObjCount == 1)
		{
			NiGeometry	*pkObject = &(pkBatchGroup->m_kNodes.GetAt(0));
			pkObject->RenderImmediate(pkRenderer->GetRenderer());
			pkBatchGroup->m_kNodes.RemoveAll();
			continue;
		}
		
		pkDX9Renderer->BeginBatch(pkBatchGroup->m_spPropertyState,pkBatchGroup->m_spEffectState);

		for(int k=0;k<iObjCount;k++)
		{
			NiGeometry	*pkObject = &(pkBatchGroup->m_kNodes.GetAt(k));
			NiTriShape	*pkTriShape = NiDynamicCast(NiTriShape,pkObject);
			if(pkTriShape)
			{
				pkDX9Renderer->BatchRenderShape(pkTriShape);
				continue;
			}

			NiTriStrips	*pkTriStrips = NiDynamicCast(NiTriStrips,pkObject);
			if(pkTriStrips)
			{
				pkDX9Renderer->BatchRenderStrips(pkTriStrips);
				continue;
			}

			pkObject->RenderImmediate(pkRenderer->GetRenderer());

		}
		pkBatchGroup->m_kNodes.RemoveAll();

		pkDX9Renderer->EndBatch();
	}

	CleanUpExpiredBatchGroup(fAccumTime);
	
}
void	PgBatchRender::ClearVisibleBatchGroup()
{
	++m_iAddCheckValue;
	m_kVisibleBatchGroupCont.clear();
}
void	PgBatchRender::CleanUpExpiredBatchGroup(float fTime)
{
	float	const	fDisposeLimitTime = 10;
	float	const	fCheckTime = 5;

	if(fTime - m_fLastBatchGroupCleanUpTime < fCheckTime)
	{
		return;
	}

	m_fLastBatchGroupCleanUpTime = fTime;

	for(int i=0;i<m_kBatchGroupCont.size();i++)
	{
		if(m_kBatchGroupCont[i]->GetDisposed())
		{
			continue;
		}

		if(fTime - m_kBatchGroupCont[i]->GetLastUsedTime() > fDisposeLimitTime)
		{
			DisposedBatchGroup(m_kBatchGroupCont[i]);
		}
	}
}
int	PgBatchRender::ReviveDisposedBatchGroup()
{
	int	iIndex = *m_kDisposedBatchGroupIDCont.begin();
	m_kDisposedBatchGroupIDCont.erase(m_kDisposedBatchGroupIDCont.begin());

	m_kBatchGroupCont[iIndex]->SetDisposed(false);
	return	iIndex;
}
void	PgBatchRender::DisposedBatchGroup(stBatchGroup *pkBatchGroup)
{
	pkBatchGroup->SetDisposed(true);
	pkBatchGroup->SetInstanceID(pkBatchGroup->GetInstanceID()+1);
	m_kDisposedBatchGroupIDCont.push_back(pkBatchGroup->m_iBatchGroupID);
}