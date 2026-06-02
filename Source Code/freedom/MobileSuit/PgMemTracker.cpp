#include "stdafx.h"
#include <NiStandardAllocator.H>
#include "PgMemTracker.H"
#include "PgMemStatusManager.H"

PgMemTracker::PgMemTracker()
:NiMemTracker(new NiStandardAllocator())
{
}
void* PgMemTracker::Allocate(size_t& stSizeinBytes, size_t& stAlignment,
    NiMemEventType eEventType, bool bProvideAccurateSizeOnDeallocate,
    char const* pcFile, int iLine, char const* pcFunction)
{
    m_kCriticalSection.Lock();
    
    size_t stSizeOriginal = stSizeinBytes;
    float fTime = NiGetCurrentTimeInSec();  
    
    if (m_bCheckArrayOverruns)
    {
       stSizeinBytes= PadForArrayOverrun(stAlignment, stSizeinBytes);
    }

    // If you hit this NIMEMASSERT, you requested a breakpoint on a specific 
    // function name.
    NIMEMASSERT(strcmp(ms_pcBreakOnFunctionName, pcFunction) != 0);

    // If you hit this NIMEMASSERT, you requested a breakpoint on a specific 
    // allocation ID.
	NIMEMASSERT(ms_stBreakOnAllocID != m_stCurrentAllocID);

    // If you hit this NIMEMASSERT, you requested a breakpoint on a specific 
    // allocation request size.
	NIMEMASSERT(ms_stBreakOnSizeRequested != stSizeOriginal);

    // Actually perform the allocation. Note that size and alignment
    // may be adjusted by the call.
    void* pvMem = m_pkActualAllocator->Allocate(stSizeinBytes, stAlignment,
        eEventType, bProvideAccurateSizeOnDeallocate, 
        pcFile, iLine, pcFunction);

	if (pvMem == NULL)
	{
		// If you hit this, your memory request was not satisfied
		m_kCriticalSection.Unlock();
		return NULL;
	}

    // update summary statistics
	m_stActiveAllocationCount++;
	m_stAccumulatedAllocationCount++;
	if (m_stActiveAllocationCount > m_stPeakAllocationCount)
    {
        m_stPeakAllocationCount = m_stActiveAllocationCount;
        m_fPeakAllocationCountTime = fTime;
    }

    m_stActiveMemory += stSizeinBytes;
	m_stAccumulatedMemory += stSizeinBytes;
	if (m_stActiveMemory > m_stPeakMemory)
    {
        m_stPeakMemory = m_stActiveMemory;
        m_fPeakMemoryTime = fTime;
    }

    // If you hit this NIMEMASSERT, your memory request result was smaller 
    // than the input.
    NIMEMASSERT(stSizeinBytes >= stSizeOriginal);

    // If you hit this NIMEMASSERT, you requested a breakpoint on a specific
    // allocation address range.
    NIMEMASSERT(pvMem < ms_pvBreakOnAllocRangeStart || 
        pvMem > ms_pvBreakOnAllocRangeEnd);

    // Pad the start and end of the allocation with the pad character 
    // so that we can check for array under and overruns. Note that the
    // address is shifted to hide the padding before the allocation.
    if (m_bCheckArrayOverruns)
    {
        MemoryFillForArrayOverrun(pvMem, stAlignment, stSizeOriginal);
    }

    // Fill the originally requested memory size with the pad character.
    // This will allow us to see how much of the allocation was 
    // actually used.
    MemoryFillWithPattern(pvMem, stSizeOriginal);

#ifdef NIMEMORY_ENABLE_EXCEPTIONS
    try
    {
#endif

    // If you hit this NIMEMASSERT, the somehow you have allocated a memory
    // unit to an address that already exists. This should never happen
    // and is an indicator that something has gone wrong in the sub-allocator.
    NIMEMASSERT(FindAllocUnit(pvMem) == NULL);

    // Grow the tracking unit reservoir if necessary
    if (!m_pkReservoir)
        GrowReservoir();

    // If you hit this NIMEMASSERT, the free store for allocation units
    // does not exist. This should only happen if the reservoir 
    // needed to grow and was unable to satisfy the request. In other words,
    // you may be out of memory.
    NIMEMASSERT (m_pkReservoir != NULL);

    // Get an allocation unit from the reservoir
    NiAllocUnit* pkUnit = m_pkReservoir;
	m_pkReservoir = pkUnit->m_pkNext;

    // fill in the known information
    pkUnit->Reset();
    pkUnit->m_stAllocationID = m_stCurrentAllocID;
    pkUnit->m_stAlignment = stAlignment;
    pkUnit->m_ulAllocThreadId = NiGetCurrentThreadId();
    pkUnit->m_eAllocType = eEventType;
    pkUnit->m_fAllocTime = fTime;
    pkUnit->m_pvMem = pvMem;
    pkUnit->m_stSizeRequested = stSizeOriginal;
    pkUnit->m_stSizeAllocated = stSizeinBytes;
    pkUnit->m_kFLF.Set(pcFile, iLine, pcFunction);

    // If you hit this NIMEMASSERT, then this allocation was made from a 
    // source that isn't setup to use this memory tracking software, use the 
    // stack frame to locate the source and include NiMemManager.h.
	NIMEMASSERTUNIT(eEventType != NI_UNKNOWN, pkUnit);

    // Insert the new allocation into the hash table
    InsertAllocUnit(pkUnit);
	g_kMemStatusManager.OnAlloc(pkUnit);

	// Validate every single allocated unit in memory
	if (m_bAlwaysValidateAll)
    {
        bool bValidateAllAllocUnits = ValidateAllAllocUnits();
        NIMEMASSERTUNIT(bValidateAllAllocUnits, pkUnit);
    }

#ifdef NIMEMORY_ENABLE_EXCEPTIONS
    }
	catch(const char *err)
	{
		// Deal with the errors
		// Deal with the errors
	}
#endif

    ++m_stCurrentAllocID;
    m_kCriticalSection.Unlock();
    
    return pvMem;
}

void PgMemTracker::Deallocate(void* pvMemory, 
    NiMemEventType eEventType, size_t stSizeinBytes)
{
    if (pvMemory)
    {

        m_kCriticalSection.Lock();
    
#ifdef NIMEMORY_ENABLE_EXCEPTIONS
        try {
#endif
        float fTime = NiGetCurrentTimeInSec();

        // Search the tracking unit hash table to find the address
        NiAllocUnit* pkUnit = FindAllocUnit(pvMemory);

        if (pkUnit == NULL)
        {
		    // If you hit this NIMEMASSERT, you tried to deallocate RAM that 
            // wasn't allocated by this memory manager. This may also
            // be indicative of a double deletion. Please check the pkUnit
            // FLF for information about the allocation.
    		NIMEMASSERT(pkUnit != NULL);
#ifdef NIMEMORY_ENABLE_EXCEPTIONS
            throw "Request to deallocate RAM that was never allocated";
#endif
            m_kCriticalSection.Unlock();
    
            return;
        }

        // If you hit this NIMEMASSERT, you requested a breakpoint on a 
        // specific allocation ID.
        NIMEMASSERTUNIT(ms_stBreakOnAllocID != pkUnit->m_stAllocationID, 
            pkUnit);

        // If you hit this NIMEMASSERT, you requested a breakpoint on a 
        // specific allocation request size.
	    NIMEMASSERTUNIT(ms_stBreakOnSizeRequested != pkUnit->m_stSizeRequested,
            pkUnit);

        // If you hit this NIMEMASSERT, you requested a breakpoint on a 
        // specific allocation address range.
        NIMEMASSERTUNIT(pkUnit->m_pvMem < ms_pvBreakOnAllocRangeStart || 
            pkUnit->m_pvMem > ms_pvBreakOnAllocRangeEnd, pkUnit);

        // If you hit this NIMEMASSERT, then the allocation unit that is about 
        // to be deleted requested an initial size that doesn't match
        // what is currently the 'size' argument for deallocation.
        // This is most commonly caused by the lack of a virtual destructor
        // for a class that is used polymorphically in an array.
        if (stSizeinBytes!= NI_MEM_DEALLOC_SIZE_DEFAULT)
		{
            NIMEMASSERTUNIT(stSizeinBytes== pkUnit->m_stSizeRequested, pkUnit);
		}

		// If you hit this NIMEMASSERT, then the allocation unit that is about 
        // to be deallocated is damaged.
        bool bValidateAllocUnit = ValidateAllocUnit(pkUnit);
 	    NIMEMASSERTUNIT(bValidateAllocUnit, pkUnit);

		// If you hit this NIMEMASSERT, then this deallocation was made from a 
        // source that isn't setup to use this memory tracking software, 
        // use the stack frame to locate the source and include
        // NiMemManager.h
		NIMEMASSERTUNIT(eEventType != NI_UNKNOWN, pkUnit);

		// If you hit this NIMEMASSERT, you were trying to deallocate RAM that 
        // was not allocated in a way that is compatible with the 
        // deallocation method requested. In other words, you have a 
        // allocation/deallocation mismatch.
		NIMEMASSERTUNIT((eEventType == NI_OPER_DELETE && 
            pkUnit->m_eAllocType == NI_OPER_NEW) ||
			(eEventType == NI_OPER_DELETE_ARRAY && 
            pkUnit->m_eAllocType == NI_OPER_NEW_ARRAY) ||
			(eEventType == NI_FREE && 
            pkUnit->m_eAllocType == NI_MALLOC) ||
			(eEventType == NI_FREE && 
            pkUnit->m_eAllocType == NI_REALLOC) ||
			(eEventType == NI_REALLOC && 
            pkUnit->m_eAllocType == NI_MALLOC) ||
			(eEventType == NI_REALLOC && 
            pkUnit->m_eAllocType == NI_REALLOC) ||
			(eEventType == NI_ALIGNEDFREE && 
            pkUnit->m_eAllocType == NI_ALIGNEDMALLOC) ||
			(eEventType == NI_ALIGNEDFREE && 
            pkUnit->m_eAllocType == NI_ALIGNEDREALLOC) ||
			(eEventType == NI_ALIGNEDREALLOC && 
            pkUnit->m_eAllocType == NI_ALIGNEDMALLOC) ||
			(eEventType == NI_ALIGNEDREALLOC && 
            pkUnit->m_eAllocType == NI_ALIGNEDREALLOC) ||
			(eEventType == NI_UNKNOWN), pkUnit);

        // update allocation unit
        NiMemEventType eDeallocType = eEventType;
        float fDeallocTime = fTime;

        // Determine how much memory was actually set
        size_t stSizeUnused = MemoryBytesWithPattern(pvMemory,
            pkUnit->m_stSizeRequested);
        m_stUnusedButAllocatedMemory += stSizeUnused;

        // Save the thread id that freed the memory
        unsigned long ulFreeThreadId = NiGetCurrentThreadId();

        if (m_bCheckArrayOverruns)
        {
            // If you hit this NIMEMASSERT, you have code that overwrites
            // either before or after the range of an allocation.
            // Check the pkUnit for information about which allocation
            // is being overwritten. 
            bool bCheckForArrayOverrun = CheckForArrayOverrun(pvMemory,
                pkUnit->m_stAlignment, pkUnit->m_stSizeRequested);
            NIMEMASSERTUNIT(!bCheckForArrayOverrun, pkUnit);

            if (stSizeinBytes!= NI_MEM_DEALLOC_SIZE_DEFAULT)
            {
                stSizeinBytes= PadForArrayOverrun(pkUnit->m_stAlignment,
                    stSizeinBytes);
            }
        }

        // Perform the actual deallocation
        m_pkActualAllocator->Deallocate(pvMemory,
            eEventType, stSizeinBytes);

		// Remove this allocation unit from the hash table
        RemoveAllocUnit(pkUnit);
		g_kMemStatusManager.OnDealloc(pkUnit);

        // update summary statistics
	    --m_stActiveAllocationCount;
        m_stActiveMemory -= pkUnit->m_stSizeAllocated;
        
        // Validate every single allocated unit in memory
        if (m_bAlwaysValidateAll)
        {
            bool bValidateAllAllocUnits = ValidateAllAllocUnits();
            NIMEMASSERTUNIT(bValidateAllAllocUnits, pkUnit);
        }

        // Write out the freed memory to the memory log
        //LogAllocUnit(pkUnit, MEM_LOG_COMPLETE, "\t", eDeallocType,
        //    fDeallocTime, ulFreeThreadId, stSizeUnused);
        
        // Recycle the allocation unit
        // Add it to the front of the reservoir 
		pkUnit->m_kFLF = NiFLF::UNKNOWN;
		pkUnit->m_pkNext = m_pkReservoir;
		m_pkReservoir = pkUnit;
   
        // Validate every single allocated unit in memory
        if (m_bAlwaysValidateAll)
        {
            bool bValidateAllAllocUnits = ValidateAllAllocUnits();
            NIMEMASSERTUNIT(bValidateAllAllocUnits, pkUnit);
        }
        m_kCriticalSection.Unlock();
    
#ifdef NIMEMORY_ENABLE_EXCEPTIONS

    }
	catch(const char *err)
	{
		// Deal with the errors

	}
#endif
    }
}
