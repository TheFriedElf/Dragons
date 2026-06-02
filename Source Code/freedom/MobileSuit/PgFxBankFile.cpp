#include "stdafx.h"
#include "PgFxBankFile.h"
#include "stdio.h"
#include "PgNiFile.H"

PgFxBankFile::~PgFxBankFile()
{
	Release();
}

bool PgFxBankFile::LoadFile(char const* szPgFxBankFileName)
{
	Release();

	NiFile* pkPgFxBankFile = NiFile::GetFile(szPgFxBankFileName, NiFile::READ_ONLY);

	if( !pkPgFxBankFile || !(*pkPgFxBankFile) )
	{
		// Failed to find the FxBank file.
		if( pkPgFxBankFile )
		{
			NiDelete pkPgFxBankFile;
			pkPgFxBankFile = NULL;
		}
		return false;
	}

	m_nDataSize = pkPgFxBankFile->GetFileSize();
	m_pData = NiMalloc(m_nDataSize);
	if( m_pData )
	{
		pkPgFxBankFile->Read(m_pData, m_nDataSize);
	}

	NiDelete pkPgFxBankFile;
	pkPgFxBankFile = NULL;

	return	true;
}

void PgFxBankFile::Release()
{
	if( m_pData )
		NiFree(m_pData);

	m_pData = NULL;
	m_nDataSize = 0;
}
