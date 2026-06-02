
#ifndef FREEDOM_DRAGONICA_RENDER_RENDERER_PGBATCHRENDER_H
#define FREEDOM_DRAGONICA_RENDER_RENDERER_PGBATCHRENDER_H

#include <NiMain.h>
#include "NewWare/Renderer/DrawWorkflow.h"


int const BATCH_GROUP_ID_NO_USE_BATCH = -100;
int const BATCH_GROUP_ID_RESET_BATCH = -1;
class	PgBatchRenderExtraData	:	public	NiExtraData
{
    NiDeclareRTTI;

public:

	static	std::string	m_kDataID;

public:

	PgBatchRenderExtraData()
		:m_iBatchRenderInstanceID(BATCH_GROUP_ID_RESET_BATCH),m_iBatchGroupID(BATCH_GROUP_ID_RESET_BATCH),
		m_iBatchGroupInstanceID(-1)
	{
	};
	bool IsStreamable() const
	{
		return	false;
	}

	bool IsCloneable() const
	{
		return	false;
	}

	int	GetBatchRenderInstanceID()	const	{	return	m_iBatchRenderInstanceID;	}
	void	SetBatchRenderInstanceID(int	iID)	{	m_iBatchRenderInstanceID = iID;	}

	int	GetBatchGroupID()	const	{	return	m_iBatchGroupID;	}
	void	SetBatchGroupID(int	iID)	{	m_iBatchGroupID = iID;	}

	int	GetBatchGroupInstanceID()	const	{	return	m_iBatchGroupInstanceID;	}
	void	SetBatchGroupInstanceID(int	iID)	{	m_iBatchGroupInstanceID = iID;	}



private:

	int	m_iBatchRenderInstanceID;
	int	m_iBatchGroupID;
	int	m_iBatchGroupInstanceID;

};
typedef NiPointer<PgBatchRenderExtraData> PgBatchRenderExtraDataPtr;


class	PgBatchRender	:	public	NiMemObject
{
public:
	struct	stBatchGroup	:	public	NiMemObject
	{
		NiPropertyStatePtr	m_spPropertyState;
		NiDynamicEffectStatePtr	m_spEffectState;
		int	m_iBatchGroupID;
		int	m_iInstanceID;
		NiVisibleArray	m_kNodes;
		bool	m_bSkinning;
		bool	m_bColor,m_bNormal;
		int	m_iTextureSetCount;
		int	m_iAddCheckValue;

		float	m_fLastUsedTime;
		bool	m_bDisposed;

		stBatchGroup():
		m_spPropertyState(NULL),
			m_spEffectState(NULL),
			m_iBatchGroupID(BATCH_GROUP_ID_RESET_BATCH),
			m_bSkinning(false),
			m_iTextureSetCount(0),
			m_bColor(false),
			m_bNormal(false),
			m_iAddCheckValue(-1),
			m_fLastUsedTime(0),
			m_bDisposed(false),
			m_iInstanceID(0)
		{
		}
		~stBatchGroup()
		{
		}

		void	SetDisposed(bool bDisposed)	{	m_bDisposed = bDisposed;	}
		bool	GetDisposed()	const	{	return	m_bDisposed;	}

		void	SetLastUsedTime(float fTime)	{	m_fLastUsedTime = fTime;	}
		float	GetLastUsedTime()	const	{	return	m_fLastUsedTime;	}

		int	GetInstanceID()	const	{	return	m_iInstanceID;	}
		void	SetInstanceID(int iInstanceID)	{	m_iInstanceID = iInstanceID;	}

		void	Clear()
		{
			m_bColor = false;
			m_bNormal = false;
			m_bSkinning = false;
			m_iTextureSetCount = 0;
			m_spPropertyState = 0;
			m_spEffectState = 0;
			m_iBatchGroupID = BATCH_GROUP_ID_RESET_BATCH;
			m_kNodes.RemoveAll();
		}

		void	Copy(stBatchGroup *pkDest)
		{
			pkDest->m_spPropertyState = m_spPropertyState;
			pkDest->m_spEffectState = m_spEffectState;
			pkDest->m_iBatchGroupID = m_iBatchGroupID;
			pkDest->m_bSkinning = m_bSkinning;
			pkDest->m_iTextureSetCount = m_iTextureSetCount;
			pkDest->m_bColor = m_bColor;
			pkDest->m_bNormal = m_bNormal;

			int	iTotalNode = m_kNodes.GetCount();
			for(int i=0;i<iTotalNode;i++)
			{
				pkDest->m_kNodes.Add(m_kNodes.GetAt(i));
			}
		}
	};

	typedef	std::vector<stBatchGroup*> VBatchGroup;
	typedef	std::vector<int> IntVector;
	typedef std::list<int> IntList;

private:

	VBatchGroup	m_kBatchGroupCont;
	int	m_iNumBatchNode;
	int	m_iMaxNumBatchNodeArray;
	int	m_iMaxBatchNodeID;
	IntVector	m_kVisibleBatchGroupCont;
	int	m_iAddCheckValue;
	static	int	m_iBatchRenderInstanceID;
	IntList	m_kDisposedBatchGroupIDCont;
	float	m_fLastBatchGroupCleanUpTime;

public:

	PgBatchRender();
	virtual	~PgBatchRender();

	void	AddObject(NiAVObject *pkAVObject);
	bool	AddGeometry(NiGeometry *pkGeom);
	
	void	Draw(PgRenderer *pkRenderer, NiCamera *pkCamera,float fAccumTime);
	void	ClearVisibleBatchGroup();
	void	Reset();
	void	Init();

	int		MakeNewBatchRenderID();

	void	ClearAllBatchGroupNodes()
	{
		for(unsigned int ui=0;ui<m_kBatchGroupCont.size();++ui)
		{
			stBatchGroup	*pkBatchGroup = m_kBatchGroupCont[ui];
			if(!pkBatchGroup)
			{
				continue;
			}
			pkBatchGroup->m_kNodes.RemoveAll();
		}
	}

private:

	void	CleanUpExpiredBatchGroup(float fTime);
	int	ReviveDisposedBatchGroup();
	void	DisposedBatchGroup(stBatchGroup *pkBatchGroup);
	void	ResizeArray(int iSize);
	static	void	IncreaseBatchRenderInstanceID()
	{
		++m_iBatchRenderInstanceID;
	}
	void	ReleaseAllBatchGroup();
	void	ReleaseAll();

	stBatchGroup*	GetSameBatchGroup(NiGeometry *pkGeom,NiPropertyState *pkProperty,NiDynamicEffectState *pkEffect)
	{


		NiTexturingProperty	*pkDestTexturing = pkProperty->GetTexturing();
		if(!pkDestTexturing)
		{
			return	NULL;
		}

		for(VBatchGroup::size_type i = 0;i < m_kBatchGroupCont.size(); ++i)
		{
			if(m_kBatchGroupCont[i]->GetDisposed())
			{
				continue;
			}
			if( (m_kBatchGroupCont[i]->m_spEffectState && !pkEffect) ||
				(!m_kBatchGroupCont[i]->m_spEffectState && pkEffect))
			{
				continue;
			}
			if(m_kBatchGroupCont[i]->m_spEffectState->Equal(pkEffect) == false) 
			{
				continue;
			}

			if( (pkGeom->GetSkinInstance() &&  m_kBatchGroupCont[i]->m_bSkinning == false) ||
				(!pkGeom->GetSkinInstance() &&  m_kBatchGroupCont[i]->m_bSkinning))
			{
				continue;
			}

			if( pkGeom->GetTextureSets() != m_kBatchGroupCont[i]->m_iTextureSetCount )
			{
				continue;
			}

			if(  (pkGeom->GetNormals() && m_kBatchGroupCont[i]->m_bNormal == false) ||
				(!pkGeom->GetNormals() && m_kBatchGroupCont[i]->m_bNormal))
			{
				continue;
			}

			if(  (pkGeom->GetColors() && m_kBatchGroupCont[i]->m_bColor == false) ||
				(!pkGeom->GetColors() && m_kBatchGroupCont[i]->m_bColor))
			{
				continue;
			}


			NiAlphaProperty	*pkDestAlpha = pkProperty->GetAlpha();
			NiAlphaProperty	*pkSrcAlpha = m_kBatchGroupCont[i]->m_spPropertyState->GetAlpha();
			if((pkDestAlpha && !pkSrcAlpha) || (!pkDestAlpha && pkSrcAlpha))
			{
				continue;
			}

			if(pkSrcAlpha && pkSrcAlpha->IsEqual(pkDestAlpha) == false)
			{
				continue;
			}

			NiStencilProperty	*pkStencilSrc = m_kBatchGroupCont[i]->m_spPropertyState->GetStencil();
			NiStencilProperty	*pkStencilDest = pkProperty->GetStencil();
			if(pkStencilSrc && pkStencilDest)
			{
				if(pkStencilDest->GetDrawMode() != pkStencilSrc->GetDrawMode())
				{
					continue;
				}
			}

			NiTexturingProperty	*pkSrcTexturing = m_kBatchGroupCont[i]->m_spPropertyState->GetTexturing();
			if(pkSrcTexturing->IsEqualFast(*pkDestTexturing))
			{
				return	(m_kBatchGroupCont[i]);
			}
			if(pkSrcTexturing->GetMaps().GetEffectiveSize() != pkDestTexturing->GetMaps().GetEffectiveSize())
			{
				continue;
			}
			if((pkSrcTexturing->GetBaseMap() && !pkDestTexturing->GetBaseMap()) ||
				(!pkSrcTexturing->GetBaseMap() && pkDestTexturing->GetBaseMap()))
			{
				continue;
			}

			if(pkSrcTexturing->GetBaseMap())
			{
				if(pkSrcTexturing->GetBaseClampMode() != pkDestTexturing->GetBaseClampMode())
				{
					continue;
				}
				NiSourceTexture	*pkSrcTex = (NiSourceTexture*)pkSrcTexturing->GetBaseMap()->GetTexture();
				NiSourceTexture*	pkDestTex = (NiSourceTexture*)pkDestTexturing->GetBaseMap()->GetTexture();
				
				if(pkSrcTex->GetFilename() != pkDestTex->GetFilename())
				{
					continue;
				}

				if((pkSrcTex->GetFilename().GetLength() == 0 || 
					pkDestTex->GetFilename().GetLength() == 0))
				{
					continue;
				}
			}

			if((pkSrcTexturing->GetGlowMap() && !pkDestTexturing->GetGlowMap()) ||
				(!pkSrcTexturing->GetGlowMap() && pkDestTexturing->GetGlowMap()))
			{
				continue;
			}
			if(pkSrcTexturing->GetGlowMap())
			{
				if(pkSrcTexturing->GetGlowMap()->GetClampMode() != pkDestTexturing->GetGlowMap()->GetClampMode())
				{
					continue;
				}
				if(pkSrcTexturing->GetGlowMap()->GetEnable() != pkDestTexturing->GetGlowMap()->GetEnable())
				{
					continue;
				}
				NiSourceTexture	*pkSrcTex = (NiSourceTexture*)pkSrcTexturing->GetGlowMap()->GetTexture();
				NiSourceTexture	*pkDestTex = (NiSourceTexture*)pkDestTexturing->GetGlowMap()->GetTexture();
				
				if(pkSrcTex->GetFilename() != pkDestTex->GetFilename())
				{
					continue;
				}
			}
			if((pkSrcTexturing->GetDetailMap() && !pkDestTexturing->GetDetailMap()) ||
				(!pkSrcTexturing->GetDetailMap() && pkDestTexturing->GetDetailMap()))
			{
				continue;
			}
			if(pkSrcTexturing->GetDetailMap())
			{
				if(pkSrcTexturing->GetDetailMap()->GetClampMode() != pkDestTexturing->GetDetailMap()->GetClampMode())
				{
					continue;
				}
				NiSourceTexture	*pkSrcTex = (NiSourceTexture*)pkSrcTexturing->GetDetailMap()->GetTexture();
				NiSourceTexture	*pkDestTex = (NiSourceTexture*)pkDestTexturing->GetDetailMap()->GetTexture();
				
				if(pkSrcTex->GetFilename() != pkDestTex->GetFilename())
				{
					continue;
				}
			}

			//if(pkSrcTexturing->IsEqual(pkDestTexturing)==false)
			//{
			//	continue;
			//}
			return	m_kBatchGroupCont[i];
		}

		return	NULL;
	}


private:
    friend void NewWare::Renderer::SetRenderStateTagExtraDataNumber( NiGeometry&, int );

    static void SetBatchGroupID( NiGeometry* pkGeom, int iBatchGroupID )
    {
        if ( NULL == pkGeom )
            return;

        PgBatchRenderExtraData* pkBatchExtraData = 
            NiDynamicCast(PgBatchRenderExtraData, pkGeom->GetExtraData(PgBatchRenderExtraData::m_kDataID.c_str()));
        if ( NULL == pkBatchExtraData )
        {
            pkBatchExtraData = NiNew PgBatchRenderExtraData();
            pkGeom->AddExtraData( PgBatchRenderExtraData::m_kDataID.c_str(), pkBatchExtraData );
        }
        // this operation means that this geometry does not be affected by the batch rendering system.
        pkBatchExtraData->SetBatchGroupID( iBatchGroupID );
    }
};

#endif // FREEDOM_DRAGONICA_RENDER_RENDERER_PGBATCHRENDER_H