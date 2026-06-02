#ifndef FREEDOM_DRAGONICA_APP_MEMORYTRACKING_PGMEMTRACKER_H
#define FREEDOM_DRAGONICA_APP_MEMORYTRACKING_PGMEMTRACKER_H

#include <NiMemTracker.H>
#include "PgMemStatusManager.H"

class	PgMemTracker	:	public	NiMemTracker
{

public:

	PgMemTracker();

    virtual void* Allocate(size_t& stSize, size_t& stAlignment,
        NiMemEventType eEventType, bool bProvideAccurateSizeOnDeallocate,
        char const* pcFile, int iLine, char const* pcFunction);
    virtual void Deallocate(void* pvMemory, 
        NiMemEventType eEventType, size_t stSizeinBytes);

private:

	PgMemStatusManager	m_kMemStatusManager;
};

#endif // FREEDOM_DRAGONICA_APP_MEMORYTRACKING_PGMEMTRACKER_H