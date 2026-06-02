#include "stdafx.h"
#include "NiAVObject.h"
#include "PgRenderer.h"
#include "PgAVObjectAlphaProcessManager.h"
#include "PgParticle.h"
#include "PgWorkerThread.h"
#include "PgWorld.H"

PgAVObjectAlphaProcessManager::PgAVObjectAlphaProcessManager()
{
	m_kObjectInfoContainer.clear();
	m_pkCulledToTrueMainTainContainer = NULL;
	m_pkCulledToFalseMainTainContainer = NULL;
}

PgAVObjectAlphaProcessManager::~PgAVObjectAlphaProcessManager()
{

}

void PgAVObjectAlphaProcessManager::Release()
{
	for(ObjectInfoItor itor = m_kObjectInfoContainer.begin(); itor != m_kObjectInfoContainer.end(); ++itor)
	{
		if(ALPHA_MANAGER_PROCESS_TYPE_OBJECT_MAINTAIN == (*itor).m_cObjectProcessType)
		{
			// 월드에는 존재해야하 할때
		}
		else
		{
			// 월드에서 사라져야할때
			THREAD_DELETE_PARTICLE((*itor).m_spAVObject);
		}
	}

	m_kObjectInfoContainer.clear();
	m_pkCulledToTrueMainTainContainer = NULL;
	m_pkCulledToFalseMainTainContainer = NULL;
}

void PgAVObjectAlphaProcessManager::AddAVObject(NiAVObject* pkAVObject, float const fEndTime, BYTE const cAlphaProcessType, BYTE const cObjectProcessType, bool const bOverride, bool const bDetach)
{
	if(!g_pkWorld)
	{
		return;
	}

	if(bOverride == true)
	{
		for(ObjectInfoItor itor = m_kObjectInfoContainer.begin(); itor != m_kObjectInfoContainer.end(); ++itor)
		{
			if((*itor).m_spAVObject == pkAVObject)
			{
				//같은 타입이면 세팅하지 않는다.
				if((*itor).m_cAlphaProcessType != cAlphaProcessType)
				{
					(*itor).m_cAlphaProcessType = cAlphaProcessType;
					(*itor).m_cObjectProcessType = cObjectProcessType;
					(*itor).m_fEndTime = g_pkWorld->GetAccumTime() + fEndTime;
					(*itor).m_fStartTime = g_pkWorld->GetAccumTime();
					(*itor).m_fCurTime = g_pkWorld->GetAccumTime();
					(*itor).m_bDetach = bDetach;
					(*itor).m_spAVObject->SetAppCulled(false);
					return;
				}
			}		
		}
	}
	
	//리스트에 값이 없을 경우 새로 추가 된다.
	AddObjectProcessInfo(pkAVObject, fEndTime, cAlphaProcessType, cObjectProcessType, bDetach);
}

void PgAVObjectAlphaProcessManager::Update(float fAccumTime,float fFrameTIme)
{
	if(!g_pkWorld)
	{
		return;
	}

	ObjectInfoItor itor = m_kObjectInfoContainer.begin();
	while(itor != m_kObjectInfoContainer.end())
	{
		ObjectProcessInfo* pObject = &(*itor);

		float fTotalTime = pObject->m_fEndTime - pObject->m_fStartTime;
		float fRemainTime = pObject->m_fEndTime - pObject->m_fCurTime;
		float fAlpha = 0.0f;

		if(ALPHA_MANAGER_PROCESS_TYPE_ALPHA_ADD == pObject->m_cAlphaProcessType)
		{
			fAlpha = 1.0f - (fRemainTime / fTotalTime);
			fAlpha = NiClamp(fAlpha, 0.0f, 1.0f);
		}
		else if(ALPHA_MANAGER_PROCESS_TYPE_ALPHA_SUB == pObject->m_cAlphaProcessType)
		{
			fAlpha = (fRemainTime / fTotalTime);
			fAlpha = NiClamp(fAlpha, 0.0f, 1.0f);

		}
		
		SetChangeAlpha(pObject->m_spAVObject, fAlpha, pObject->m_cAlphaProcessType);
		
		PgParticle* pkParticle = NULL;
		if(pkParticle = NiDynamicCast(PgParticle, pObject->m_spAVObject))
		{
			pkParticle->Update(fAccumTime, fFrameTIme);
		}
		else
		{
			pObject->m_spAVObject->Update(fAccumTime);
		}

		pObject->m_fCurTime = g_pkWorld->GetAccumTime();

		// 시간이 지나면 리스트에서 삭제 해주어야 한다.
		if(pObject->m_fCurTime >= pObject->m_fEndTime)
		{
			if(ALPHA_MANAGER_PROCESS_TYPE_OBJECT_DELETE == pObject->m_cObjectProcessType)
			{		
				if(pObject->m_bDetach)
				{
					if(pObject->m_spAVObject->GetParent())
					{
						NiAVObjectPtr pAVObject = pObject->m_spAVObject->GetParent()->DetachChild(pObject->m_spAVObject);
						pObject->m_spAVObject = pAVObject;
					}
				}
				// 월드에서 사라져야할때
				if(NiIsKindOf(PgParticle, pObject->m_spAVObject))
				{
					THREAD_DELETE_PARTICLE(pObject->m_spAVObject);
				}
				else
				{
					THREAD_DELETE_OBJECT(pObject->m_spAVObject);
				}
			}
			else
			{
				// 월드에는 존재해야 하는 오브젝트
				if(ALPHA_MANAGER_PROCESS_TYPE_ALPHA_SUB == pObject->m_cAlphaProcessType)
				{//알파가 0 이되어서 컬링되어야 하는 오브젝트 리스트
					SetChangeAlpha(pObject->m_spAVObject, 0.0f, pObject->m_cAlphaProcessType, true);
					if(m_pkCulledToTrueMainTainContainer)
					{
						m_pkCulledToTrueMainTainContainer->push_back(NiDynamicCast(NiNode, pObject->m_spAVObject));
						RestoreAlphaProperty(pObject->m_spAVObject, m_kObjectAlphaContainer);
					}
				}
				else
				{//알파가 1이 되어서 컬링 되지 않아야 하는 오브젝트 리스트
					SetChangeAlpha(pObject->m_spAVObject, 1.0f, pObject->m_cAlphaProcessType, true);
					if(m_pkCulledToFalseMainTainContainer)
					{						
						m_pkCulledToFalseMainTainContainer->push_back(NiDynamicCast(NiNode, pObject->m_spAVObject));
						RestoreAlphaProperty(pObject->m_spAVObject, m_kObjectAlphaContainer);
					}
				}
			}
			
			itor = m_kObjectInfoContainer.erase(itor);
		}
		else
		{
			++itor;
		}		
	}
}

void PgAVObjectAlphaProcessManager::Draw(PgRenderer* pkRenderer, NiCamera* pkCamera, float fFrameTime, NiVisibleArray& kVisibleScene)
{
	for(ObjectInfoItor itor = m_kObjectInfoContainer.begin(); itor != m_kObjectInfoContainer.end(); ++itor)
	{
		ObjectProcessInfo* pObject = &(*itor);

		// 파티클의 경우는 월드 및 actor에 붙어 있다가 떨어져 나올때 이곳에 들어오므로 여기서 렌더링 해주어야 한다.
		if(NiIsKindOf(PgParticle, pObject->m_spAVObject))
		{
			NiAVObject* pkObject = pObject->m_spAVObject;
			PgParticle* pkParticle = static_cast<PgParticle*>(pkObject);
			pkRenderer->CullingProcess_Deprecated(pkCamera, pkParticle, &kVisibleScene);
            NiDrawVisibleArrayAppend( kVisibleScene );
		}
		// 파티클이 아닌 오브젝트들은 월드에 물려 있기 때문에 월드에서 렌더링시 추가 된다.
	}
}

void PgAVObjectAlphaProcessManager::SetChangeAlpha(NiAVObject* pkAVObject, float const fAlpha, BYTE const cAlphaProcessType, bool bSetForceAlpha)
{
	NiNode* pkNode = NULL;
	if(pkNode = NiDynamicCast(NiNode,pkAVObject))
	{
		unsigned int uiArrayCount = pkNode->GetArrayCount();
		for (unsigned int i = 0; i < uiArrayCount; ++i)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if(!pkChild)
			{
				continue;
			}

			NiGeometry* pkGeometry = NULL; 
			if(pkGeometry = NiDynamicCast(NiGeometry,pkChild))
			{
				// MaterialProperty를 가져온다.

				bool	bPropertyUpdate = false;

				NiMaterialProperty* pkMaterialProp = pkGeometry->GetPropertyState()->GetMaterial();
				if(pkMaterialProp)
				{
					if(bSetForceAlpha)
					{
						pkMaterialProp->SetAlpha(fAlpha);
						bPropertyUpdate = true;
					}
					else
					{
						bool bUpdate = false;
						// 알파가 증가 할때
						if(ALPHA_MANAGER_PROCESS_TYPE_ALPHA_ADD == cAlphaProcessType)
						{
							if(fAlpha >= pkMaterialProp->GetAlpha())
							{
								bUpdate = true;								
							}
						}
						// 알파가 줄어들 때
						else
						{
							if(fAlpha <= pkMaterialProp->GetAlpha())
							{
								bUpdate = true;
							}
						}

						if(bUpdate)
						{
							pkMaterialProp->SetAlpha(fAlpha);
							bPropertyUpdate = true;

							SetChangeAlphaTest(pkGeometry, fAlpha);
						}
					}
				}

				if(bPropertyUpdate)
				{
					pkGeometry->SetNeedUpdateAllShaderConstant(true);
					pkGeometry->UpdateProperties();
				}
			}

			SetChangeAlpha(pkChild, fAlpha, cAlphaProcessType, bSetForceAlpha);
		}
	}		
}

void PgAVObjectAlphaProcessManager::SetChangeAlphaTest(NiGeometry* pkGeometry, float const fAlpha)
{
	NiAlphaProperty* pkAlpha = pkGeometry->GetPropertyState()->GetAlpha();
	if(pkAlpha)
	{
		//기존에 알파 프로퍼티가 존재 하는 것이라면
		ObjectAlphaContainer::const_iterator itor = m_kObjectAlphaContainer.find(dynamic_cast<NiAVObject*>(pkGeometry));
		if(itor != m_kObjectAlphaContainer.end())
		{
			//원본의 AlphaTest값이 존재할 경우
			if(0 < (*itor).second->GetTestRef())
			{
				int iTestRef = static_cast<int>(fAlpha * 255.0f);
				iTestRef -= 1;

				// Alpha 값이 AlphaTest
				if(iTestRef < (*itor).second->GetTestRef())
				{
					iTestRef = __max(iTestRef, 0);

					pkAlpha->SetTestRef(iTestRef);
				}
			}
		}
	}
}

void PgAVObjectAlphaProcessManager::SetAppCulledToTrueMainTainList(std::list<NiNode*>* pkMainMTainContainer)
{
	m_pkCulledToTrueMainTainContainer = pkMainMTainContainer;	
}

void PgAVObjectAlphaProcessManager::SetAppCulledToFalseMainTainList(std::list<NiNode*>* pkMainMTainContainer)
{
	m_pkCulledToFalseMainTainContainer = pkMainMTainContainer;	
}

void PgAVObjectAlphaProcessManager::AddObjectProcessInfo(NiAVObject* pkAVObject, float const fEndTime, BYTE const cAlphaProcessType, BYTE const cObjectProcessType, bool const bDetach)
{	
	if(!g_pkWorld)
	{
		return;
	}

	ObjectProcessInfo kObjectInfo;
	kObjectInfo.m_spAVObject = pkAVObject;
	kObjectInfo.m_cAlphaProcessType = cAlphaProcessType;
	kObjectInfo.m_cObjectProcessType = cObjectProcessType;
	kObjectInfo.m_fEndTime = g_pkWorld->GetAccumTime() + fEndTime;
	kObjectInfo.m_fStartTime = g_pkWorld->GetAccumTime();
	kObjectInfo.m_fCurTime = g_pkWorld->GetAccumTime();
	kObjectInfo.m_bDetach = bDetach;
	kObjectInfo.m_spAVObject->SetAppCulled(false);

	if(!NiDynamicCast(PgParticle, pkAVObject))
	{
		AddAlphaProperty(pkAVObject, m_kObjectAlphaContainer);
	}

	if(ALPHA_MANAGER_PROCESS_TYPE_ALPHA_ADD == cAlphaProcessType)
	{
		SetChangeAlpha(pkAVObject, 0.0f, cAlphaProcessType, true);
	}
	
	m_kObjectInfoContainer.push_back(kObjectInfo);
}


void PgAVObjectAlphaProcessManager::AddAlphaProperty(NiAVObject* pkAVObject, ObjectAlphaContainer& m_kObjectAlphaContainer)
{
	NiNode* pkNode = NULL;
	if(pkNode = NiDynamicCast(NiNode, pkAVObject))
	{
		unsigned int uiArrayCount = pkNode->GetArrayCount();
		for (unsigned int i = 0; i < uiArrayCount; ++i)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if(!pkChild)
			{
				continue;
			}

			NiGeometry* pkGeometry = NULL; 
			if(pkGeometry = NiDynamicCast(NiGeometry,pkChild))
			{
				NiAlphaProperty* pkAlphaProperty = pkGeometry->GetPropertyState()->GetAlpha();
				
				// 알파 프로퍼티가 있으면 새로 등록
				if(pkAlphaProperty)
				{
					//기존 AlphaProperty를 backup한다.
					m_kObjectAlphaContainer.insert(std::make_pair(pkChild, pkAlphaProperty));
					unsigned char ucAlphaTest = pkAlphaProperty->GetTestRef();
					NiAlphaProperty::TestFunction kTestMode = pkAlphaProperty->GetTestMode();
					
					pkGeometry->DetachProperty(pkAlphaProperty);
					
					pkAlphaProperty = NiNew NiAlphaProperty;
					pkAlphaProperty->SetAlphaBlending(true);
					pkAlphaProperty->SetAlphaTesting(true);
					pkAlphaProperty->SetTestRef(ucAlphaTest);
					pkAlphaProperty->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
					pkAlphaProperty->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
					pkAlphaProperty->SetTestMode(kTestMode);
					pkAlphaProperty->SetAlphaGroup(AG_EFFECT);

					pkGeometry->AttachProperty(pkAlphaProperty);
					pkGeometry->UpdateProperties();
				}
				// 알파 프로퍼티가 없으면
				else if(!pkAlphaProperty)
				{
					pkAlphaProperty = NiNew NiAlphaProperty;
					pkAlphaProperty->SetAlphaBlending(true);
					pkAlphaProperty->SetAlphaTesting(true);
					pkAlphaProperty->SetSrcBlendMode(NiAlphaProperty::ALPHA_SRCALPHA);
					pkAlphaProperty->SetDestBlendMode(NiAlphaProperty::ALPHA_INVSRCALPHA);
					pkAlphaProperty->SetTestMode(NiAlphaProperty::TEST_GREATER);
					pkAlphaProperty->SetAlphaGroup(AG_EFFECT);

					pkGeometry->AttachProperty(pkAlphaProperty);
					pkGeometry->UpdateProperties();
				}
			}

			AddAlphaProperty(pkChild, m_kObjectAlphaContainer);
		}
	}
}

void PgAVObjectAlphaProcessManager::RestoreAlphaProperty(NiAVObject* pkAVObject, ObjectAlphaContainer& m_kObjectAlphaContainer)
{
	NiNode* pkNode = NULL;
	if(pkNode = NiDynamicCast(NiNode, pkAVObject))
	{
		unsigned int uiArrayCount = pkNode->GetArrayCount();
		for (unsigned int i = 0; i < uiArrayCount; ++i)
		{
			NiAVObject* pkChild = pkNode->GetAt(i);
			if(!pkChild)
			{
				continue;
			}

			NiGeometry* pkGeometry = NULL; 
			if(pkGeometry = NiDynamicCast(NiGeometry,pkChild))
			{
				NiAlphaProperty* pkAlphaProperty = pkGeometry->GetPropertyState()->GetAlpha();
				if(pkAlphaProperty)
				{
					ObjectAlphaContainer::iterator itor = m_kObjectAlphaContainer.find(pkChild);
					if(itor != m_kObjectAlphaContainer.end())
					{
						//백업된 알파 프로퍼티가 있으면
						pkGeometry->DetachProperty(pkAlphaProperty);
						pkGeometry->AttachProperty((*itor).second);
						pkGeometry->UpdateProperties();
						m_kObjectAlphaContainer.erase(itor);
					}
					else
					{
						pkGeometry->DetachProperty(pkAlphaProperty);
						pkGeometry->UpdateProperties();
					}
				}
			}

			RestoreAlphaProperty(pkChild, m_kObjectAlphaContainer);
		}
	}
}
