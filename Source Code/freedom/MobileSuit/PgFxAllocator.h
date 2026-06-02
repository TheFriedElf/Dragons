/*****************************************************************

    MODULE    : PgFxAllocator.h

    PURPOSE   : Reference implementation for a basic allocator class.

    CREATED   : 8/20/2008 

    COPYRIGHT : (C) 2008 Aristen, Inc.

*****************************************************************/

#ifndef PgFxAllocator_H_
#define PgFxAllocator_H_

#include "FxStudioRT.h"

class PgFxAllocator : public FxStudio::Allocator
{
public :

	PgFxAllocator();
	virtual ~PgFxAllocator();

	// numBytes will always be greater than zero.
	virtual void* AllocateBytes(std::size_t nNumBytes, MemoryCategory eCategory);

	// pBytes will never be zero.
	virtual void  ReleaseBytes(void* pBytes, MemoryCategory eCategory);


	void LogReport();

private :

	FxStudio::Allocator& m_OldAllocator;

#ifndef NISHIPPING

	int			m_nBytesAllocated[MaxMemoryCategories];
	int			m_nBytesReleased[MaxMemoryCategories];

	int			m_nMaxConcurrentlyAllocated[MaxMemoryCategories];
	std::size_t	m_nMaxAllocation[MaxMemoryCategories];

	int			m_nNumAllocations[MaxMemoryCategories];
	int			m_nNumDeallocations[MaxMemoryCategories];

	int			m_nTotalCurrentlyAllocated;
	int			m_nMaxTotalConcurrentlyAllocated;

#endif

};


#endif // PgFxAllocator_H_

