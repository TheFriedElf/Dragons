#ifndef FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXBANKFILE_H
#define FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXBANKFILE_H

#include <NiSmartPointer.h>
#include <NiRefObject.h>

class PgFxBankFile : public NiRefObject
{
public :

	PgFxBankFile()
		: m_nDataSize(0)
		, m_pData(0) {}

	~PgFxBankFile();
	
	bool LoadFile(char const* szPgFxBankFileName);

	// Only call this if the bank has been unloaded from all managers!
	void Release();

	bool IsLoaded() const { return m_pData != 0; }

	const void* GetData() const { return m_pData; }
	unsigned int GetDataSize() const { return m_nDataSize; }


private :

	// This class does not support copying.
	PgFxBankFile(const PgFxBankFile&);
	PgFxBankFile& operator=(const PgFxBankFile&);

	unsigned int	m_nDataSize;
	void*			m_pData;
};

NiSmartPointer(PgFxBankFile);

#endif // FREEDOM_DRAGONICA_RENDER_PARTICLE_FXSTUDIO_PGFXBANKFILE_H