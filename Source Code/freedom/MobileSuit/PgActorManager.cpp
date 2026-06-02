#include "stdafx.h"
#include "PgActorManager.H"
#include "NiAnimationMetrics.h"
#include "NiCloningProcess.h"
#include <NiFilename.h>
#include "PgControllerSequenceManager.H"

bool	PgActorManager::m_bUseNewClone = false;

//---------------------------------------------------------------------------
NiActorManager* PgActorManager::CreatePG(char const* pcKFMFilename,
    bool bCumulativeAnimations, bool bLoadFilesFromDisk, NiStream* pkStream)
{
    assert(pcKFMFilename);

    NiKFMTool::KFM_RC eRC, eSuccess = NiKFMTool::KFM_SUCCESS;

    // Create KFM tool and load KFM file.
    NiKFMToolPtr spKFMTool = NiNew NiKFMTool;
    eRC = spKFMTool->LoadFile(pcKFMFilename);
    if (eRC != eSuccess)
    {
        return NULL;
    }

    return CreatePG(spKFMTool, pcKFMFilename, bCumulativeAnimations,
        bLoadFilesFromDisk, pkStream);
}
//---------------------------------------------------------------------------
NiActorManager* PgActorManager::CreatePG(NiKFMTool* pkKFMTool,
    char const* pcKFMFilePath, bool bCumulativeAnimations,
    bool bLoadFilesFromDisk, NiStream* pkStream)
{
    assert(pkKFMTool && pcKFMFilePath);

    // Build the KFM path.
    NiFilename kFilename(pcKFMFilePath);	
    char acKFMPath[NI_MAX_PATH];
    NiSprintf(acKFMPath, NI_MAX_PATH, "%s%s", kFilename.GetDrive(), 
        kFilename.GetDir());
    pkKFMTool->SetBaseKFMPath(acKFMPath);

    // Create actor manager.
    PgActorManager* pkActorManager = NiNew PgActorManager(pkKFMTool,
        bCumulativeAnimations);

	pkActorManager->m_kKFMFileName = pcKFMFilePath;
    // Initialize actor manager.
    if (bLoadFilesFromDisk)
    {
        // Create the stream if not provided.
        bool bDeleteStream = false;
        if (!pkStream)
        {
            pkStream = NiNew NiStream;
            bDeleteStream = true;
        }

        // Initialize the actor manager and load all files.
        if (!pkActorManager->Initialize(*pkStream))
        {
            NiDelete pkActorManager;
            return NULL;
        }

        // Delete stream if created earlier.
        if (bDeleteStream)
        {
            NiDelete pkStream;
        }
    }

    return pkActorManager;
}
PgActorManager::PgActorManager(NiKFMTool* pkKFMTool, bool bCumulativeAnimations)
:NiActorManager(pkKFMTool,bCumulativeAnimations),
m_bSlightClonned(false),
m_spSourceNIFRoot(NULL)
{
}

PgActorManager::~PgActorManager()
{
	m_spSourceNIFRoot = 0;
}
void	PgActorManager::UnloadAllSequence()
{
    NiTMapIterator kItr = m_kSequenceMap.GetFirstPos();
    while (kItr)
    {
        SequenceID eID;
        NiControllerSequence* pkCSeq;
        m_kSequenceMap.GetNext(kItr, eID, pkCSeq);
		if(m_spManager)
		{
			m_spManager->RemoveSequence(pkCSeq);
		}
    }
	m_kSequenceMap.RemoveAll();
}
NiActorManager* PgActorManager::Clone(NiCloningProcess* pkCloningProcess)
{
	if(GetUseNewClone())
	{
		float	fStartTime = NiGetCurrentTimeInSec();
		NiActorManager *pkAM = 	CloneEx(pkCloningProcess);
		float	fElapsedTime = NiGetCurrentTimeInSec() - fStartTime;
		_PgOutputDebugString("CloneTime[%s] : %f\n",GetKFMFileName(),fElapsedTime);
		return	pkAM;
	}

	return	NiActorManager::Clone(pkCloningProcess);
}
NiActorManager* PgActorManager::CloneEx(NiCloningProcess* pkCloningProcess)
{
	NiNode	*pkSourceNIF = GetSourceNIFRoot();
	PG_ASSERT_LOG(pkSourceNIF);
	if(!pkSourceNIF)
	{
		return	NULL;
	}
    // Clone the NIF root.
    bool bDeleteCloningProcess = false;
    if (!pkCloningProcess)
    {
        pkCloningProcess = NiNew NiCloningProcess();
        bDeleteCloningProcess = true;
    }
    pkCloningProcess->m_eCopyType = NiObjectNET::COPY_EXACT;
	// 클로닝 할때, particle은 클로닝 안되는가?
    NiAVObject* pkNIFRoot = (NiAVObject*) pkSourceNIF->Clone(
        *pkCloningProcess);
    if (bDeleteCloningProcess)
    {
        NiDelete pkCloningProcess;
    }

    // Get model root name.
    char const* pcModelRoot = m_spKFMTool->GetModelRoot();
    if (!pcModelRoot)
    {
		//여기서 위에 cloning한거 지워야 함.
        return NULL;
    }

    // Find the actor root.
    NiAVObject* pkActorRoot = pkNIFRoot->GetObjectByName(pcModelRoot);
    if (!pkActorRoot)
    {
		//여기서 위에 cloning한거 지워야 함.
        return NULL;
    }

    // Create the controller manager.
    NiControllerManager* pkManager = NiNew NiControllerManager(pkActorRoot,
        m_bCumulativeAnimations);

	PgActorManager* pkActorManager = NiNew PgActorManager(m_spKFMTool,
        m_bCumulativeAnimations);

    pkActorManager->m_spNIFRoot = pkNIFRoot;
    pkActorManager->m_spManager = pkManager;
	pkActorManager->m_kKFMFileName = m_kKFMFileName;
	
	pkActorManager->SetSourceNIFRoot(pkSourceNIF);
	pkActorManager->SetSlightClonned(true);

	return	pkActorManager;
}

bool PgActorManager::SetTargetAnimation(SequenceID eSequenceID)
{
    if (m_eTargetID == eSequenceID)
    {
        return true;
    }

    if (eSequenceID != INVALID_SEQUENCE_ID)
    {
        NiControllerSequence *pkSequence = NULL;
        if (!m_kSequenceMap.GetAt(eSequenceID, pkSequence))
        {

			if(GetUseNewClone() && GetSlightClonned())
			{
				NiControllerSequencePtr spNewSequence = g_kControllerSequenceManager.CreateNewControllerSequence(m_spKFMTool,eSequenceID);
				if(!spNewSequence)
				{
					return	false;
				}

				//m_spManager->RemoveSequence(spNewSequence);


				ChangeSequence(eSequenceID, spNewSequence,false);

				if(!m_kSequenceMap.GetAt(eSequenceID, pkSequence))
				{
					return	false;
				}
			}
			else
	            return false;
        }
    }

    m_eTargetID = eSequenceID;
    m_bBuildTimeline = true;

    return true;
}
NiControllerSequence* PgActorManager::GetSequence(SequenceID eSequenceID)
{// 시퀀스를 얻어오는데
    NiControllerSequence* pkSequence;
    if (m_kSequenceMap.GetAt(eSequenceID, pkSequence))
    {// 이미 저장되어있으면 그것을 사용하고
        return pkSequence;
    }
	// 저장되어 있지 않으면
	if(GetUseNewClone() && GetSlightClonned())
	{// 시퀀스를 만들어
		NiControllerSequencePtr spNewSequence = g_kControllerSequenceManager.CreateNewControllerSequence(m_spKFMTool,eSequenceID);
		if(!spNewSequence)
		{
			return	NULL;
		}

		//m_spManager->RemoveSequence(spNewSequence);
		// 저장해 두고
		ChangeSequence(eSequenceID, spNewSequence,false);

		if(m_kSequenceMap.GetAt(eSequenceID, pkSequence))
		{// 다시 얻어 반환 한다.
			return	pkSequence;
		}
	}
    return NULL;
}
bool PgActorManager::ChangeNIFRoot(NiAVObject* pkNIFRoot)
{
	if(GetUseNewClone() == false)
	{
		return	NiActorManager::ChangeNIFRoot(pkNIFRoot);
	}

    if (!pkNIFRoot)
    {
        return false;
    }

    assert(m_spKFMTool);

    // Get model root name.
    char const* pcModelRoot = m_spKFMTool->GetModelRoot();
    if (!pcModelRoot)
    {
        return false;
    }

    // Find the actor root.
    NiAVObject* pkActorRoot = pkNIFRoot->GetObjectByName(pcModelRoot);
    if (!pkActorRoot)
    {
        return false;
    }

    // Turn off all sequences.
    Reset();

	m_spManager = 0;
	m_kSequenceMap.RemoveAll();

    // Create the controller manager.
    NiControllerManager* pkManager = NiNew NiControllerManager(pkActorRoot,
        m_bCumulativeAnimations);

    m_spNIFRoot = pkNIFRoot;
    m_spManager = pkManager;
    m_spBoneLOD = FindBoneLODController(m_spNIFRoot);

    return true;
}
bool PgActorManager::ChangeSequence(SequenceID eSequenceID,NiControllerSequence* pkSequence,bool bUnloadSequnce)
{
   // Remove old sequence mapped to this ID, if it exists.
    if(bUnloadSequnce)
	{
		UnloadSequence(eSequenceID);
	}
	else
	{
		int iIndex = m_kExtraSequences.Find(eSequenceID);

		if (eSequenceID == m_eCurID || eSequenceID == m_eNextID ||
			eSequenceID == m_eTargetID || iIndex > -1)
		{
			Reset();
		}

		m_kSequenceMap.RemoveAt(eSequenceID);
	}

    bool bSuccess = m_spManager->AddSequence(pkSequence);
    if (bSuccess)
    {
        m_kSequenceMap.SetAt(eSequenceID, pkSequence);

		/// Barunson Interactive Mod
		// Add Additional Events (Effects, Sounds)
		if(m_spKFMTool)
		{
			// Register Sound Events
			
			for(unsigned int uiType = 0;uiType<NiActorManager::EVENT_TYPE_MAX;++uiType)
			{
				NiAnimationEventSet *pkEventSet = m_spKFMTool->GetAnimationEventSet(uiType,eSequenceID);
				if(!pkEventSet)
				{
					continue;
				}

				pkEventSet->RegisterCallback(this,pkSequence,eSequenceID);
			}
		}
    }

    return bSuccess;
}

bool PgActorManager::Initialize(NiStream& kStream)
{
	if(GetUseNewClone())
	{
		if (!LoadNIFFile(kStream, true))
		{
			return false;
		}

		NiCloningProcess	kClone;
		kClone.m_eCopyType = NiObjectNET::COPY_EXACT;
		NiNode* pkClonNIFRoot = (NiNode*) GetNIFRoot()->Clone(kClone);

		SetSourceNIFRoot(pkClonNIFRoot);

		SetSlightClonned(true);

		return true;
	}

	return	NiActorManager::Initialize(kStream);
}
