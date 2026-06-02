#include "stdafx.h"
#include "PgControllerSequenceManager.H"
#include "PgNIFile.H"

PgControllerSequenceManager	g_kControllerSequenceManager;

PgControllerSequenceManager::PgControllerSequenceManager()
{
}
PgControllerSequenceManager::~PgControllerSequenceManager()
{
}

void	PgControllerSequenceManager::Init()
{
}
void	PgControllerSequenceManager::Terminate()
{
	IntSeqMap::iterator iter = m_kConSeqMap.begin();
	while (iter != m_kConSeqMap.end())
	{
		SAFE_DELETE(iter->second);
		++iter;
	}
	
	m_kConSeqMap.clear();
}
NiControllerSequencePtr	PgControllerSequenceManager::CreateNewControllerSequence(NiKFMTool *pkKFMTool,NiActorManager::SequenceID kSequenceID)
{
    // Get sequence information.
    NiKFMTool::Sequence* pkKFMSequence = pkKFMTool->GetSequence(
        kSequenceID);
    if (!pkKFMSequence)
    {
        return NULL;
    }

	// TODO: 락을 잡아야 하지 않을까?

    // Get KF filename.
	std::string kKFFilename = (char const*)pkKFMTool->GetFullKFFilename(kSequenceID);

	int iAnimIndex = pkKFMSequence->GetAnimIndex();

	return	CreateNewControllerSequence(kKFFilename,iAnimIndex);
}

NiControllerSequencePtr	PgControllerSequenceManager::CreateNewControllerSequence(std::string const &kKFFilename,int iAnimIndex)
{
	stFileBuf	*pkBuffer = Find(kKFFilename);
	bool	bNewBuffer = false;
	if(!pkBuffer)
	{
		NiFile	*pkFile = NiFile::GetFile(kKFFilename.c_str(),NiFile::READ_ONLY);
		if(!pkFile)
		{
			return	NULL;
		}

		pkBuffer = new stFileBuf();
		pkBuffer->m_iLength = pkFile->GetFileSize();
		pkBuffer->m_pkBuf = new BYTE[pkBuffer->m_iLength];

		NiBinaryStream	*pkBinaryStream = (NiBinaryStream*)pkFile;

		pkBinaryStream->Read(pkBuffer->m_pkBuf,pkBuffer->m_iLength);

		bNewBuffer = true;

		SAFE_DELETE_NI(pkFile);

	}

	NiStream	kStream;

	bool bSuccess = kStream.Load((char *)pkBuffer->m_pkBuf,pkBuffer->m_iLength);
    if (!bSuccess)
    {
        NILOG("PgControllerSequenceManager: Failed to load KF File: %s\n", kKFFilename.c_str());
        return NULL;
    }

    NiControllerSequencePtr spSequence =
        NiControllerSequence::CreateSequenceFromFile(kStream, iAnimIndex);
    if (!spSequence)
    {
        NILOG("PgControllerSequenceManager: Failed to add sequence at index %d in %s\n", iAnimIndex, kKFFilename.c_str());
        return NULL;
    }	

	if(bNewBuffer)
	{
		IntSeqMap::iterator itor = m_kConSeqMap.find(kKFFilename);
		if(itor == m_kConSeqMap.end())
		{
			m_kConSeqMap.insert(std::make_pair(kKFFilename,pkBuffer));
		}
	}

	return	spSequence;

}

PgControllerSequenceManager::stFileBuf*	PgControllerSequenceManager::Find(std::string const &kKFFilename) const
{
	IntSeqMap::const_iterator itor = m_kConSeqMap.find(kKFFilename);
	if(itor == m_kConSeqMap.end())
	{
		return	NULL;
	}

	stFileBuf *pkFileBuf = itor->second;
	return	pkFileBuf;
}