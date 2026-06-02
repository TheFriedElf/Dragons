#include "StdAfx.h"
#include "PgEventCamera.h"
#include "PgWorld.h"
#include "PgRenderer.h"

PgEventCamera::PgEventCamera()
{
}
PgEventCamera::~PgEventCamera()
{
	ClearEventObject();
}

NiCamera* PgEventCamera::FindCamera(NiAVObject* pObject)
{
	NiCamera* pkCamera = NULL;

	if(NiIsKindOf(NiCamera, pObject))
	{
		pkCamera = NiDynamicCast(NiCamera, pObject);
	}

	NiNode* pkNode = NiDynamicCast(NiNode, pObject);
	if(pkNode)
	{
		for(unsigned int i=0; i<pkNode->GetArrayCount(); ++i)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				NiCamera* pkCameraChild;
				pkCameraChild = FindCamera(pkChild);
				if(!pkCamera)
					pkCamera = pkCameraChild;
			}
		}
	}

	return pkCamera;
}

void PgEventCamera::InsertEventObject(std::string strName, NiNode* pkNode)
{
	//if(!FindCamera(pkNode))
	//{
	//	return;
	//}

	m_smEventNode.insert(std::make_pair(strName, pkNode));
}
void PgEventCamera::ClearEventObject()
{
	m_smEventNode.clear();
}

NiNode* PgEventCamera::GetEventObject(std::string strName)
{
	NiNode* pkRtn = NULL;
	ContEventNode::iterator itr = m_smEventNode.find(strName);
	if(itr != m_smEventNode.end())
	{
		pkRtn = itr->second;
	}

	return pkRtn;
}
NiCamera* PgEventCamera::GetCamera(std::string strName)
{
	NiCamera* pkRtn = NULL;
	ContEventNode::iterator itr = m_smEventNode.find(strName);
	if(itr != m_smEventNode.end())
	{
		NiNode* pkNode = itr->second;
		pkRtn = FindCamera(pkNode);
	}
	return pkRtn;
}

bool PgEventCamera::StartAnimation(std::string strName)
{
	NiNode* pkNode = NULL;
	ContEventNode::iterator itr = m_smEventNode.find(strName);
	if(itr != m_smEventNode.end())
	{
		pkNode = itr->second;
	}

	if(!pkNode)
		return false;

	// 애니메이션을 돌린다.
	if(g_pkWorld)
	{
		pkNode->SetAppCulled(false);
		PgWorld::SetAniType(pkNode, NiTimeController::APP_TIME);
		PgWorld::SetAniCycleType(pkNode, NiTimeController::CLAMP);

		PgRenderer::RestartAnimation(pkNode, g_pkWorld->GetAccumTime());
	}

	return true;
}

void RecursiveStopAnimation(NiAVObject *pkAVObject)
{
	if(!pkAVObject)
	{
		return;
	}
	NiTimeController	*pkController = pkAVObject->GetControllers();

	while(pkController)
	{
		pkController->Stop();
		pkController = pkController->GetNext();
	}

	NiNode	*pkNode = NiDynamicCast(NiNode,pkAVObject);
	if(pkNode)
	{
		int iChildCount = pkNode->GetArrayCount();
		for(int i=0;i<iChildCount;i++)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(pkChild)
			{
				RecursiveStopAnimation(pkChild);
			}
		}
	}

}

bool PgEventCamera::StopAnimation(std::string strName)
{
	NiNode* pkNode = NULL;
	ContEventNode::iterator itr = m_smEventNode.find(strName);
	if(itr != m_smEventNode.end())
	{
		pkNode = itr->second;
	}

	if(!pkNode)
		return false;

	RecursiveStopAnimation(pkNode);

	// 애니메이션을 멈춘다.


	return true;
}