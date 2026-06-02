#include "stdafx.h"
#include "PgPSRoom.H"
#include "PgRenderer.H"
#include "PgOption.H"
#include "PgActor.H"
#include "PgPilotMan.H"
#include "PgPilot.H"

#include "NewWare/Scene/ApplyTraversal.h"


NiImplementRTTI(PgPSRoom, NiRoom);

PgPSRoom::PgPSRoom()
{
	m_spSelectiveRoot = NiNew NiNode();
	m_spStaticRoot = NiNew NiNode();

	AttachFixture(m_spSelectiveRoot);
	AttachFixture(m_spStaticRoot);
}
PgPSRoom::~PgPSRoom()
{
}
void	PgPSRoom::UpdateSelected(NiCamera *pkCamera,float fTime,NiNode *pkSelectiveNodeRoot)
{
	// NOTE: When changing UpdateDownwardPass, UpdateSelectedDownwardPass,
	// or UpdateRigidDownwardPass, remember to make equivalent changes
	// the all of these functions.
	NiFrustumPlanes	kPlanes(*pkCamera);

	NiPoint3	const	&kCamPos = pkCamera->GetWorldTranslate();

	pkSelectiveNodeRoot->UpdateObjectControllers(fTime, pkSelectiveNodeRoot->GetSelectiveUpdatePropertyControllers());

	if (pkSelectiveNodeRoot->GetSelectiveUpdateTransforms())
		pkSelectiveNodeRoot->UpdateWorldData();

	NiBound kBound;

	pkSelectiveNodeRoot->SetWorldBound(kBound);

	float	fLowUpdateDistance = 1500;
	float	fDistanceFromCam = 0.0f;
	bool	bLowUpdate = g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_WORLD_QUALITY) != 1;
	bool	bUseUpdateLOD = false;

	NiPoint3	kPlayerPos;
	PgActor* pkPlayerActor = g_kPilotMan.GetPlayerActor();
	if(pkPlayerActor)
	{
		kPlayerPos = pkPlayerActor->GetPosition();
	}

	// To avoid having to call UpdateWorldBound and therefore making another
	// iteration through the node's children, the world bound is calculated
	// during this loop.
	int	const	iChildCount = pkSelectiveNodeRoot->GetArrayCount();
	for (int i = 0; i < iChildCount; ++i)
	{
		NiAVObject* pkChild = pkSelectiveNodeRoot->GetAt(i);
		if (pkChild)
		{
			bool	bVisible = true;
			unsigned int j = 0;

			bUseUpdateLOD = (pkChild->GetExtraData("USE_UPDATE_LOD")==NULL);

			NiBound	kChildBound = pkChild->GetWorldBound();
			fDistanceFromCam = (kChildBound.GetCenter()-kCamPos).Length() - kChildBound.GetRadius();

			if(bUseUpdateLOD)
			{
				if(fDistanceFromCam > 1000)
				{
					for (j = 0; j < NiFrustumPlanes::MAX_PLANES; ++j)
					{
						int iSide = kChildBound.WhichSide(
							kPlanes.GetPlane(j));

						if (iSide == NiPlane::NEGATIVE_SIDE)
						{
							// The object is not visible since it is on the negative
							// side of the plane.
							bVisible = false;
							break;
						}
					}		
				}
			}

			bool	bUpdate = false;
			if(bVisible)
			{
				if(bLowUpdate && bUseUpdateLOD)
				{
					if(fDistanceFromCam<fLowUpdateDistance)
					{
						bUpdate = true;
					}
				}
				else
				{
					bUpdate = true;
				}
			}

			bool	bPreviousUpdate = (pkChild->GetExtraData("UPDATED")!=NULL);

			if(bUpdate)
			{
				if(!bPreviousUpdate)	//	이전에 업데이트를 안했다면,
				{
                    NewWare::Scene::ApplyTraversal::Geometry::
                                        SetShaderConstantUpdateOptimizeFlag( pkChild, false, true ); //	쉐이더 상수 업데이트 최적화 끄기

					if(m_spExtraData == 0)
					{
						m_spExtraData = NiNew NiExtraData();
					}
					pkChild->AddExtraData("UPDATED",m_spExtraData);
				}

				pkChild->DoSelectedUpdate(fTime);

				if (pkChild->IsVisualObject())
				{
					if (kBound.GetRadius() == 0.0f)
					{
						kBound = pkChild->GetWorldBound();
					}
					else
					{
						kBound.Merge(&pkChild->GetWorldBound());
					}
				}
			}
			else
			{
				if(bPreviousUpdate)	//	이전에 업데이트를 했다면
				{
					pkChild->RemoveExtraData("UPDATED");
                    NewWare::Scene::ApplyTraversal::Geometry::
                                        SetShaderConstantUpdateOptimizeFlag( pkChild, true, true ); //	쉐이더 상수 업데이트 최적화 켜기
				}
			}
        }
    }

	pkSelectiveNodeRoot->SetWorldBound(kBound);

	if (pkSelectiveNodeRoot->GetParent())
		pkSelectiveNodeRoot->GetParent()->UpdateUpwardPass();

}
void	PgPSRoom::UpdateSelective(NiCamera *pkCamera,float fTime,bool bUpdateControllers)
{
    NIMETRICS_MAIN_INCREMENTUPDATES();

    if (bUpdateControllers)
        UpdateObjectControllers(fTime);

    UpdateWorldData();
	UpdateColorData();

	m_kWorldBound = m_spStaticRoot->GetWorldBound();
	UpdateSelected(pkCamera,fTime,m_spSelectiveRoot);
	m_kWorldBound.Merge(&m_spSelectiveRoot->GetWorldBound());
}


void	PgPSRoom::AssignObject(PgWorld::MapObjectMap &kObjectMap,bool bAssignEverything)
{

	if(bAssignEverything)
	{
		for(PgWorld::MapObjectMap::iterator itor = kObjectMap.begin(); itor != kObjectMap.end(); ++itor)
		{
			PgWorld::stMapObject const &kObject = itor->second;
			if(kObject.m_bNoPortalSystem)
			{
				continue;
			}
			if(kObject.m_bHasAnimation)
			{
				m_spSelectiveRoot->AttachChild(kObject.m_spAVObject);
			}
			else
			{
				m_spStaticRoot->AttachChild(kObject.m_spAVObject);
			}

			m_kObjectCont.insert(std::make_pair(itor->first,kObject));

		}
		kObjectMap.clear();
	}
	else
	{

		int	iTotal = m_kObjectNameCont.size();
		for(int i=0;i<iTotal;++i)
		{
			PgWorld::MapObjectMap::iterator itor = kObjectMap.find(m_kObjectNameCont[i].c_str());
			if(itor == kObjectMap.end())
			{
				continue;
			}
			PgWorld::stMapObject const &kObject = itor->second;
			if(kObject.m_bNoPortalSystem)
			{
				continue;
			}
			if(kObject.m_bHasAnimation)
			{
				m_spSelectiveRoot->AttachChild(kObject.m_spAVObject);
			}
			else
			{
				m_spStaticRoot->AttachChild(kObject.m_spAVObject);
			}

			m_kObjectCont.insert(std::make_pair(itor->first,kObject));
			kObjectMap.erase(itor);
		}
	}
}