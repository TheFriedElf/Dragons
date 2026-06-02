#ifndef FREEDOM_DRAGONICA_UTIL_PGNIFILE_H
#define FREEDOM_DRAGONICA_UTIL_PGNIFILE_H

#include "NiFile.h"
#include "NiImageConverter.h"
#include "BM/FileSupport.h"
#include "BM/PgFileMapping.h"
#include "PgStat.h"

// PgNiFile has no writing functions
class PgNiFile : public NiFile
{
	NiDeclareDerivedBinaryStream();
public:
	PgNiFile(char const* pcName, OpenMode eMode, unsigned int uiBufferSize = 262144);
	virtual ~PgNiFile();

	// Create a single directory
	static bool CreateDirectory(char const* pcDirName);
	static bool DirectoryExists(char const* pcDirName);
	static bool Initialize();
	static bool UnInitialize();
	static void SetFileReadMode(bool bEncrypt, bool bCompress, bool bUsePack);
	static void RegisterPackFile(std::string parentFolder, std::string packFileName);
	static void SetSilentLoading(bool bSet);
	static LONG GetSilentLoading();
	static bool IsPackedFile(std::string fileName, std::wstring& packFile, std::wstring& childName);
	static bool CheckFileExist(char const* pcName);
	static bool CheckFileExist(const wchar_t* pcName);
	
	virtual void Seek(int iOffset, int iWhence);
	virtual unsigned int GetFileSize() const;

	virtual void SetEndianSwap(bool bDoSwap);

	PG_STAT_H(static PgStatGroup ms_kFileIOStatGroup)
	PG_STAT_H(static PgStatGroup ms_kFileStatGroup)
protected:	
	//! static member functions and variables
	static NiFile* PgFileCreateFunc(char const *pcName, OpenMode eMode, unsigned int uiBufferSize);
	static bool PgFileAccessFunc(char const *pcName, OpenMode eMode);
	static bool PgFileExistFunc(char const* pcDir);	// for NiImageConverter
	static bool PgCreateDirectoryFunc(char const* pcDir);
	static bool PgDirectoryExistsFunc(char const* pcDir);	
	void doSlientLoading();

	static bool ms_bEncrypt;
	static bool ms_bCompress;
	static bool ms_bUsePack;
	static LONG ms_iUseSilentLoading;
	static DWORD ms_dwLastLoadingStartTime;
	static DWORD ms_dwMainThread;
	static DWORD ms_dwLastFrame;
	typedef std::map<std::string, std::wstring> PackFolderNameMap;
	static PackFolderNameMap ms_kPackFolderName;	

	//! member functions and variables
	unsigned int PgFileRead(void* pvBuffer, unsigned int uiBytes);
	unsigned int PgFileWrite(void const* pvBuffer, unsigned int uiBytes);
	
	std::wstring m_kPackFileName;
	std::wstring m_kChildName;
	std::vector<char> m_kFileData;
	size_t m_iCurPos;
	size_t m_iFileSize;
	float m_fLiveTime;
	bool m_bPackFile;
//	PgFileMapping m_kMapping;
	std::vector<char> m_kContFileData;
	bool m_bUseFileMapping;
	bool m_bUseTotalAlloc;
	DWORD m_dwCreateThread;
	char* m_pDataBuffer;
};
#endif // FREEDOM_DRAGONICA_UTIL_PGNIFILE_H