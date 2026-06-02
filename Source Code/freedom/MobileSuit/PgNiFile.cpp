#include "stdafx.h"
#include "PgNiFile.h"
#include "PgResourceMonitor.h"
#include "PgMobileSuit.h"

bool PgNiFile::ms_bEncrypt = false;
bool PgNiFile::ms_bCompress = false;
bool PgNiFile::ms_bUsePack = false;
LONG PgNiFile::ms_iUseSilentLoading = 0;
DWORD PgNiFile::ms_dwLastLoadingStartTime = 0;
DWORD PgNiFile::ms_dwMainThread = 0;
DWORD PgNiFile::ms_dwLastFrame = 0;
PgNiFile::PackFolderNameMap PgNiFile::ms_kPackFolderName;

PG_STAT_H(PgStatGroup PgNiFile::ms_kFileIOStatGroup("PgNiFile"))
PG_STAT_H(PgStatGroup PgNiFile::ms_kFileStatGroup("PgNiFileList"))

NiImplementDerivedBinaryStream(PgNiFile, PgFileRead, PgFileWrite);


bool PgNiFile::Initialize()
{
	PG_STAT(ms_kFileStatGroup.StartRecording());
	PG_STAT(ms_kFileIOStatGroup.StartRecording());
	NILOG(PGLOG_MEMIO, "[PgNiFile] Initialize %d, %d, %d\n", ms_bEncrypt, ms_bCompress, ms_bUsePack);
	if (ms_bUsePack)
	{
		NiFile::SetCreateDirectoryFunc(PgNiFile::PgCreateDirectoryFunc);
		NiFile::SetDirectoryExistsFunc(PgNiFile::PgDirectoryExistsFunc);
		NiImageConverter::SetFileExistsFunc(PgNiFile::PgFileExistFunc);
	}
	NiFile::SetFileCreateFunc(PgNiFile::PgFileCreateFunc);
	NiFile::SetFileAccessFunc(PgNiFile::PgFileAccessFunc);

	ms_dwMainThread = ::GetCurrentThreadId();
	return true;
}

bool PgNiFile::UnInitialize()
{
	NILOG(PGLOG_MEMIO, "[PgNiFile] UnInitialize\n");
	NiFile::SetCreateDirectoryFunc(NiFile::DefaultCreateDirectoryFunc);
	NiFile::SetDirectoryExistsFunc(NiFile::DefaultDirectoryExistsFunc);
	NiFile::SetFileCreateFunc(NiFile::DefaultFileCreateFunc);
	NiFile::SetFileAccessFunc(NiFile::DefaultFileAccessFunc);
	NiImageConverter::SetFileExistsFunc(NiImageConverter::DefaultFileExistsFunc);
	return true;
}

void PgNiFile::SetFileReadMode(bool bEncrypt, bool bCompress, bool bUsePack)
{
	ms_bEncrypt = bEncrypt;
	ms_bCompress = bCompress;
	ms_bUsePack = bUsePack;

	NILOG(PGLOG_MEMIO, "[PgNiFile] SetFileReadMode %d, %d, %d\n", ms_bEncrypt, ms_bCompress, ms_bUsePack);

	// 평소에는 로그를 남기기 위해 PgNiFile을 그냥 쓴다.
#ifdef EXTERNAL_RELEASE	
	if (ms_bEncrypt == false && ms_bCompress == false && ms_bUsePack == false)
		UnInitialize();
	else
#endif
		Initialize();
}

void PgNiFile::RegisterPackFile(std::string parentFolder, std::string packFileName)
{
	NILOG(PGLOG_LOG, "[PgNiFile] RegisterPackFile %s path, %s file\n", parentFolder.c_str(), packFileName.c_str());
	PackFolderNameMap::iterator iter = ms_kPackFolderName.find(parentFolder);
	if (iter == ms_kPackFolderName.end())
	{
		std::string parent = parentFolder;
		std::string packFile = packFileName;
		UPR(parent);
		BM::ConvFolderMark(parent);
		BM::ConvFolderMark(packFile);
		ms_kPackFolderName.insert(std::make_pair(parent, UNI(packFile)));
	}
	else
	{
		NILOG(PGLOG_WARNING, "[PgNiFile] RegisterPackFile %s path already registered to %s file(%s)\n", parentFolder.c_str(), MB(iter->second), packFileName.c_str());
	}
}

void PgNiFile::SetSilentLoading(bool bSet)
{
	if (bSet)
		::InterlockedIncrement(&ms_iUseSilentLoading);
	else
		::InterlockedDecrement(&ms_iUseSilentLoading);
}

LONG PgNiFile::GetSilentLoading()
{
	return ms_iUseSilentLoading;
}

NiFile* PgNiFile::PgFileCreateFunc(char const *pcName, OpenMode eMode, unsigned int uiSize)
{
	NILOG(PGLOG_MEMIO, "[PgNiFile] FileCreateFunc %s, %d, %d\n", pcName, eMode, uiSize);
	if (eMode == READ_ONLY)
	{
		return NiNew PgNiFile(pcName, eMode, uiSize);
	}
	else
	{
		return NiNew NiFile(pcName, eMode, uiSize);
	}
}

bool PgNiFile::PgFileAccessFunc(char const *pcName, OpenMode eMode)
{
	NILOG(PGLOG_MEMIO, "[PgNiFile] FileAccessFunc %s, %d\n", pcName, eMode);
	if (ms_bUsePack)
	{
		std::wstring packFileName;
		std::wstring childName;
		if (IsPackedFile(pcName, packFileName, childName))
		{
			bool bRet = BM::PgDataPackManager::IsFileExistInPack(packFileName, childName);
#ifndef EXTERNAL_RELEASE
			if (bRet == false)
			{
				NILOG(PGLOG_WARNING, "[PgNiFile] %s file access but not exist in %s pack file\n", pcName, MB(packFileName));
			}
#endif
			return bRet;
		}
	}

	bool bRet = NiFile::DefaultFileAccessFunc(pcName, eMode);
#ifndef EXTERNAL_RELEASE
	if (bRet == false)
	{
		NILOG(PGLOG_WARNING, "[PgNiFile] %s file access but not exist\n", pcName);
	}
#endif
	return bRet;
}

bool PgNiFile::PgFileExistFunc(char const *pcName)
{
	NILOG(PGLOG_MEMIO, "[PgNiFile] FileExistFunc %s\n", pcName);
	if (ms_bUsePack)
	{
		std::wstring packFileName;
		std::wstring childName;
		if (IsPackedFile(pcName, packFileName, childName))
		{
			bool bRet = BM::PgDataPackManager::IsFileExistInPack(packFileName, childName);
#ifndef EXTERNAL_RELEASE
			if (bRet == false)
			{
				NILOG(PGLOG_WARNING, "[PgNiFile] %s file access but not exist in %s pack file\n", pcName, MB(packFileName));
			}
#endif
			return bRet;
		}
	}

	bool bRet = NiImageConverter::DefaultFileExistsFunc(pcName);
#ifndef EXTERNAL_RELEASE
	if (bRet == false)
	{
		NILOG(PGLOG_WARNING, "[PgNiFile] %s file access but not exist\n", pcName);
	}
#endif
	return bRet;
}

bool PgNiFile::PgCreateDirectoryFunc(char const* pcDirName)
{
	NILOG(PGLOG_MEMIO, "[PgNiFile] CreateDirectoryFunc %s\n", pcDirName);
	return NiFile::DefaultCreateDirectoryFunc(pcDirName);
}

bool PgNiFile::PgDirectoryExistsFunc(char const* pcDirName)
{
	NILOG(PGLOG_MEMIO, "[PgNiFile] DirectoryExistsFunc %s\n", pcDirName);
	DWORD dwAttrib = GetFileAttributesA(pcDirName);
	if (dwAttrib == -1)
	{
		return false;
	}

	return (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool PgNiFile::IsPackedFile(std::string fileName, std::wstring& packFile, std::wstring& childName)
{
	PG_STAT(PgStatTimerF timerA(ms_kFileIOStatGroup.GetStatInfo("IsPackedFile"), g_pkApp->GetFrameCount()));
	PackFolderNameMap::iterator iter = ms_kPackFolderName.begin();

	std::string file = fileName;
	
	UPR(file);
	BM::ConvFolderMark(file);
	while (iter != ms_kPackFolderName.end())
	{
		// TODO: 찾는 이름이 경로 중간에 들어가 있으면 잘못된 PackFile에서 찾을 수도 있다. 그 부분을 어떻게 막을 수 있을까?
		// 일단 0 - pos가 다 match될때만 찾으면 되긴 할것 같긴 하다.
		const std::string::size_type pos = file.rfind(iter->first);

		if( std::string::npos != pos)
		{
			std::string child = file.substr(pos + strlen(iter->first.c_str()));
			packFile = iter->second;
			childName =  (std::wstring)(_T("./")) + UNI(child);			
			return true;
		}
		++iter;
	}
	return false;
}
bool PgNiFile::CheckFileExist(char const* pcName)
{
	return PgFileExistFunc(pcName);
}

bool PgNiFile::CheckFileExist(const wchar_t* pcName)
{
	std::string fileName = MB(std::wstring(pcName));
	return PgFileExistFunc(fileName.c_str());
}

//------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//

PgNiFile::PgNiFile(char const* pcName, OpenMode eMode, unsigned int uiBufferSize) : m_kFileData(0)
{
	PG_ASSERT_LOG(pcName);
	if (pcName == NULL)
		return;

	PG_STAT(PgStatTimerF timerA(ms_kFileIOStatGroup.GetStatInfo("PgNiFile"), g_pkApp->GetFrameCount()));	
	doSlientLoading();
	m_pDataBuffer = NULL;
	m_fLiveTime = NiGetCurrentTimeInSec();
	m_bPackFile = false;
	m_iCurPos = 0;
	m_iFileSize = 0;	
	m_bUseFileMapping = ::GetPrivateProfileInt(TEXT("Debug"), TEXT("PgNiFile.FileMapping"), 0, g_pkApp->GetConfigFileName());
	m_bUseTotalAlloc = ::GetPrivateProfileInt(TEXT("Debug"), TEXT("PgNiFile.TotalAlloc"), 0, g_pkApp->GetConfigFileName());
	m_dwCreateThread = ::GetCurrentThreadId();

	PG_ASSERT_LOG(eMode == READ_ONLY);
	if (eMode != READ_ONLY)
	{
		NILOG(PGLOG_ERROR, "[PgNiFile] %s File Open Mode %d\n", pcName, eMode);
	}
	m_eMode = eMode;

	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(pcName, PgResourceMonitor::RESOURCE_TYPE_GENERAL_FILE));

	SetEndianSwap(false);

	char const *pcMode = "rb";

	std::string packFile;
	if (ms_bUsePack && IsPackedFile(pcName, m_kPackFileName, m_kChildName))
	{
		m_bPackFile = true;
		m_bGood = true;
		m_pBuffer = NULL;
		m_pFile = NULL;
		if (BM::PgDataPackManager::LoadFromPack(m_kPackFileName, m_kChildName, m_kFileData))
		{
			NILOG(PGLOG_MEMIO, "[PgNiFile] LoadFrom(%s)Pack %s file succeed, %d size\n", MB(m_kPackFileName), MB(m_kChildName), m_kFileData.size());
			m_iFileSize = m_kFileData.size();
			m_pDataBuffer = &m_kFileData.at(0);
		}
		else
		{
			NILOG(PGLOG_ERROR, "[PgNiFile] LoadFrom(%s)Pack %s file failed\n", MB(m_kPackFileName), MB(m_kChildName));
		}
	}
	else
	{
		m_kChildName = UNI(pcName);
		if(m_bUseFileMapping)
		{
			BM::FileToMem(m_kChildName, m_kContFileData);

//			m_kMapping.Init(m_kChildName, OPEN_EXISTING);

			m_bGood = (m_kContFileData.size() > 0);
	
			if (m_bGood)
			{
				m_iFileSize = m_kContFileData.size();

				if(m_bUseTotalAlloc)
				{
					m_kFileData.resize(m_kContFileData.size());
					if(NiMemcpy(&m_kFileData.at(0), &m_kContFileData.at(0), m_kContFileData.size()) == -1)
					{
						m_kFileData.clear();
						m_bUseTotalAlloc = false;
					}
					else
					{
						m_pDataBuffer = &m_kFileData.at(0);
						
						std::vector<char> temp;
						temp.swap(m_kContFileData);
					}
				}
				else
				{
					m_pDataBuffer = &m_kContFileData.at(0);
				}
			}
	
			m_pBuffer = NULL;
			m_pFile = NULL;
		}
		else
		{
		#if _MSC_VER >= 1400
			m_bGood = (fopen_s(&m_pFile, pcName, pcMode) == 0 && m_pFile != NULL);
		#else //#if _MSC_VER >= 1400
			m_pFile = fopen(pcName, pcMode);
			m_bGood = (m_pFile != NULL);
		#endif //#if _MSC_VER >= 1400

			m_iFileSize = GetFileSize();
			if (m_bUseTotalAlloc)
			{
				uiBufferSize = m_iFileSize;
			}
			else
			{
				if (m_iFileSize > 1024 * 1024)
				{
					uiBufferSize = uiBufferSize * 4;
				}
			}

			m_uiBufferAllocSize = uiBufferSize;
			m_uiPos = m_uiBufferReadSize = 0;

			//! 일단 읽으면 헤더부터 읽는다.
			if (m_bGood && uiBufferSize > 0)
			{
				setvbuf(m_pFile, NULL, _IOFBF, m_uiBufferAllocSize);
				m_pBuffer = NiAlloc(char, m_uiBufferAllocSize);
				PG_ASSERT_LOG(m_pBuffer != NULL);				
				if (m_pBuffer && m_bUseTotalAlloc)
				{
					if (DiskRead(m_pBuffer, m_iFileSize) == 0)
					{
						NILOG(PGLOG_ERROR, "[PgNiFile] %s File can't read from disk\n", pcName);
					}
					else
					{
						//fclose(m_pFile); // TODO?
						//m_pFile = NULL;
					}
				}
			}
			else
			{
				m_pBuffer = NULL;
			}
		}
	}
	doSlientLoading();
}

//---------------------------------------------------------------------------
PgNiFile::~PgNiFile()
{
	m_iFileSize = 0;

	NILOG(PGLOG_MEMIO, "[PgNiFile] ~PgFile %s(packed %d, time %f)\n", MB(m_kChildName), m_bPackFile, NiGetCurrentTimeInSec() - m_fLiveTime);
	doSlientLoading();
	PG_STAT(UpdateFrameCount(*ms_kFileStatGroup.GetStatInfo(MB(m_kChildName)), g_pkApp->GetFrameCount(), NiGetCurrentTimeInSec() - m_fLiveTime))
}


//---------------------------------------------------------------------------
// TODO Compress/Encrypt 파일의 경우 버퍼에 읽어서 그 정확한 위치를 찾아야 한다.
void PgNiFile::Seek(int iOffset, int iWhence)
{
	PG_STAT(PgStatTimerF timerA(ms_kFileIOStatGroup.GetStatInfo("Seek"), g_pkApp->GetFrameCount()));
	if (m_bPackFile)
	{
		NILOG(PGLOG_MEMIO, "[PgNiFile] Seek %s, %d, %d\n", MB(m_kChildName), iOffset, iWhence);
	}

	PG_ASSERT_LOG(iWhence == ms_iSeekSet || iWhence == ms_iSeekCur || iWhence == ms_iSeekEnd);
	PG_ASSERT_LOG(m_eMode != APPEND_ONLY);

	if (m_bGood)
	{
		unsigned int uiNewPos = (int)m_uiAbsoluteCurrentPos + iOffset;
		
		if (m_bPackFile || m_bUseFileMapping || m_bUseTotalAlloc)
		{
			if (iWhence == ms_iSeekCur)
				m_uiAbsoluteCurrentPos = m_uiAbsoluteCurrentPos + iOffset;
			else if (iWhence == ms_iSeekSet)
				m_uiAbsoluteCurrentPos = iOffset;
			else if (iWhence == ms_iSeekEnd)
				m_uiAbsoluteCurrentPos = m_iFileSize - iOffset;
		}
		else
		{
			if (iWhence == ms_iSeekCur)
			{
				// If we can accomplish the Seek by adjusting m_uiPos, do so.

				int iNewPos = (int) m_uiPos + iOffset;
				if (iNewPos >= 0 && iNewPos < (int) m_uiBufferReadSize)
				{
					m_uiPos = iNewPos;
					m_uiAbsoluteCurrentPos = (int)m_uiAbsoluteCurrentPos + 
						iOffset;
					return;
				}

				// User's notion of current file position is different from
				// actual file position because of bufferring implemented by
				// this class. Make appropriate adjustment to offset.

				if (PgNiFile::READ_ONLY == m_eMode)
					iOffset -= (m_uiBufferReadSize - m_uiPos);
			}

			Flush();

			m_bGood = (fseek(m_pFile, iOffset, iWhence) == 0);
			if (m_bGood)
			{
				m_uiAbsoluteCurrentPos = ftell(m_pFile);
#ifdef _DEBUG
				if (iWhence == ms_iSeekCur)
				{
					PG_ASSERT_LOG(uiNewPos == m_uiAbsoluteCurrentPos);
				}
				else if (iWhence == ms_iSeekSet)
				{
					PG_ASSERT_LOG(m_uiAbsoluteCurrentPos == iOffset);
				}
#endif
			}
		}
	}
}

//---------------------------------------------------------------------------
unsigned int PgNiFile::PgFileRead(void* pBuffer, unsigned int uiBytes)
{
	PG_STAT(PgStatTimerF timerA(ms_kFileIOStatGroup.GetStatInfo("PgFileRead"), g_pkApp->GetFrameCount()));	
	PG_ASSERT_LOG(m_eMode == READ_ONLY);

	if (m_bGood == false)
		return 0;

	if (m_bPackFile || (m_bUseFileMapping && m_bUseTotalAlloc))
	{
		if (m_kFileData.size() == 0 || m_uiAbsoluteCurrentPos + uiBytes > m_kFileData.size())
			return 0;

		if (NiMemcpy(pBuffer, &m_kFileData.at(m_uiAbsoluteCurrentPos), uiBytes) == -1)
			return 0;

		return uiBytes;
	}
	else if (m_bUseFileMapping)
	{
		if (m_kContFileData.size() == 0 || m_uiAbsoluteCurrentPos + uiBytes > m_kContFileData.size())
				return 0;

		if (NiMemcpy(pBuffer, &m_kContFileData.at(m_uiAbsoluteCurrentPos), uiBytes) == -1)
			return 0;

		return uiBytes;
	}
	else if (m_bUseTotalAlloc && m_bUseFileMapping == false)
	{
		if (m_iFileSize == 0 || m_uiAbsoluteCurrentPos + uiBytes > m_iFileSize)
			return 0;

		if (NiMemcpy(pBuffer, &m_pBuffer[m_uiAbsoluteCurrentPos], uiBytes) == -1)
			return 0;

		return uiBytes;
	}

	unsigned int uiAvailBufferBytes, uiRead;

	uiRead = 0;
	uiAvailBufferBytes = m_uiBufferReadSize - m_uiPos;
	
	if (uiBytes > uiAvailBufferBytes)
	{
		NILOG(PGLOG_MEMIO, "[PgNiFile] PgFileRead(normal) %s, %d(avail %d)\n", MB(m_kChildName), uiBytes, uiAvailBufferBytes);
		doSlientLoading();
		if (uiAvailBufferBytes > 0)
		{
			NiMemcpy(pBuffer, &m_pBuffer[m_uiPos], uiAvailBufferBytes);
			pBuffer = &(((char *) pBuffer)[uiAvailBufferBytes]);
			uiBytes -= uiAvailBufferBytes;
			uiRead = uiAvailBufferBytes;
		}
		Flush();

		if (uiBytes > m_uiBufferAllocSize)
		{
			return uiRead + DiskRead(pBuffer, uiBytes);
		}
		else
		{
			m_uiBufferReadSize = DiskRead(m_pBuffer, m_uiBufferAllocSize);
			if (m_uiBufferReadSize < uiBytes)
			{
				uiBytes = m_uiBufferReadSize;
			}
		}
	}

	NiMemcpy(pBuffer, &m_pBuffer[m_uiPos], uiBytes);
	m_uiPos += uiBytes;		
	return uiRead + uiBytes;
}

unsigned int PgNiFile::PgFileWrite(void const *pBuffer, unsigned int uiBytes)
{
	PG_ASSERT_LOG(m_eMode != READ_ONLY);
	PG_ASSERT_LOG(uiBytes != 0);

	if (m_bPackFile)
	{
		NILOG(PGLOG_MEMIO, "[PgNiFile] PgFileWrite %s, %d\n", MB(m_kChildName), uiBytes);
	}

	if (m_bGood)
	{
		unsigned int uiAvailBufferBytes, uiWrite;

		uiWrite = 0;
		uiAvailBufferBytes = m_uiBufferAllocSize - m_uiPos;
		if (uiBytes > uiAvailBufferBytes)
		{
			if (uiAvailBufferBytes > 0)
			{
				NiMemcpy(&m_pBuffer[m_uiPos], pBuffer, uiAvailBufferBytes);
				pBuffer = &(((char *) pBuffer)[uiAvailBufferBytes]);
				uiBytes -= uiAvailBufferBytes;
				uiWrite = uiAvailBufferBytes;
				m_uiPos = m_uiBufferAllocSize;
			}

			if (!Flush())
				return 0;

			if (uiBytes >= m_uiBufferAllocSize)
			{
				return uiWrite + DiskWrite(pBuffer, uiBytes);
			}
		}

		NiMemcpy(&m_pBuffer[m_uiPos], pBuffer, uiBytes);
		m_uiPos += uiBytes;
		return uiWrite + uiBytes;
	}
	else
	{
		return 0;
	}
}

unsigned int PgNiFile::GetFileSize() const
{
	PG_STAT(PgStatTimerF timerA(ms_kFileIOStatGroup.GetStatInfo("GetFileSize"), g_pkApp->GetFrameCount()));
	if (m_bPackFile || m_bUseTotalAlloc)
	{
		NILOG(PGLOG_MEMIO, "[PgNiFile] GetFileSize %s,%d\n", MB(m_kChildName), m_kFileData.size());
		PG_ASSERT_LOG(m_iFileSize == m_kFileData.size());
		return m_kFileData.size();
	}

	if (m_bUseFileMapping)
	{
		PG_ASSERT_LOG(m_iFileSize == m_kContFileData.size());
		return m_kContFileData.size();
	}

	if (m_iFileSize != 0)
		return (unsigned int)m_iFileSize;

	if (m_pFile == NULL || m_bGood == false)
		return 0;

	int iCurrent = ftell(m_pFile);
	if (iCurrent < 0)
		return 0;
	fseek(m_pFile, 0, SEEK_END);
	int iSize = ftell(m_pFile);
	fseek(m_pFile, iCurrent, SEEK_SET);
	if (iSize < 0)
		return 0;

	return (unsigned int)iSize;
}

void PgNiFile::doSlientLoading()
{
	if (ms_iUseSilentLoading <= 0 || g_pkApp->IsInitialized() == false)
		return;

	if (ms_dwMainThread == 0 || m_dwCreateThread == ms_dwMainThread)
		return;

	bool bTimeCheck = (BM::GetTime32() - ms_dwLastLoadingStartTime <= 10);
	bool bFrameCheck = (ms_dwLastFrame == g_pkApp->GetFrameCount());
	int iSleepTime = 0;

	if (bTimeCheck == false)
		return;

	if (bFrameCheck)
		iSleepTime = 30;
	else if (bTimeCheck)
		iSleepTime = 10;
	else
		iSleepTime = 3;

	NILOG(PGLOG_THREAD, "[PgNiFile] sleep start(%d,%d,%d,%d)\n", bTimeCheck, bFrameCheck, iSleepTime, g_pkApp->GetFrameCount());
	NiSleep(iSleepTime);
	NILOG(PGLOG_THREAD, "[PgNiFile] sleep end\n", g_pkApp->GetFrameCount());

	ms_dwLastLoadingStartTime = BM::GetTime32();
	ms_dwLastFrame = g_pkApp->GetFrameCount();
}