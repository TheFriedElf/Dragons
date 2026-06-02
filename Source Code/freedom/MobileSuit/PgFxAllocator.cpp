#include "stdafx.h"
#include "PgFxAllocator.h"
#include <NiAllocator.h>
#include <NiLog.h>


#ifndef NISHIPPING

char const* g_MemoryCategoryName[FxStudio::Allocator::MaxMemoryCategories] =
{
	"ManagerMemory",
	"PreviewMemory",
	"BankMemory",

	"FxMemory",
	"FxComponentMemory",
	"FxInputMemory",
	"FxInputDataMemory",

	"ComponentMemory"
};

#endif // NISHIPPING

PgFxAllocator::PgFxAllocator()
	: m_OldAllocator(FxStudio::GetAllocator())
{
#ifndef NISHIPPING

	m_nTotalCurrentlyAllocated = 0;
	m_nMaxTotalConcurrentlyAllocated = 0;

	memset(m_nBytesAllocated, 0, sizeof(m_nBytesAllocated) );
	memset(m_nBytesReleased, 0, sizeof(m_nBytesReleased) );

	memset(m_nMaxConcurrentlyAllocated, 0, sizeof(m_nMaxConcurrentlyAllocated) );
	memset(m_nMaxAllocation, 0, sizeof(m_nMaxAllocation) );

	memset(m_nNumAllocations, 0, sizeof(m_nNumAllocations) );
	memset(m_nNumDeallocations, 0, sizeof(m_nNumDeallocations) );

#endif // NISHIPPING

	FxStudio::RegisterAllocator(*this);
}

PgFxAllocator::~PgFxAllocator()
{
	FxStudio::RegisterAllocator(m_OldAllocator);
}


void* PgFxAllocator::AllocateBytes(std::size_t nNumBytes, MemoryCategory eCategory) 
{
#ifndef NISHIPPING

	++m_nNumAllocations[eCategory];

	m_nBytesAllocated[eCategory] += nNumBytes;

	if( nNumBytes > m_nMaxAllocation[eCategory] )
		m_nMaxAllocation[eCategory] = nNumBytes;

	const int nCurrentlyAllocated = m_nBytesAllocated[eCategory] - m_nBytesReleased[eCategory];
	if( nCurrentlyAllocated > m_nMaxConcurrentlyAllocated[eCategory] )
		m_nMaxConcurrentlyAllocated[eCategory] = nCurrentlyAllocated;

	m_nTotalCurrentlyAllocated += nNumBytes;
	if( m_nTotalCurrentlyAllocated > m_nMaxTotalConcurrentlyAllocated )
		m_nMaxTotalConcurrentlyAllocated = m_nTotalCurrentlyAllocated;


	int* pMemory = static_cast<int*>(NiMalloc(nNumBytes + sizeof(int)));
	*pMemory = nNumBytes;

	return pMemory + 1;

#else

	return NiMalloc(nNumBytes);

#endif // NISHIPPING
}

void PgFxAllocator::ReleaseBytes(void* pBytes, MemoryCategory eCategory)
{
#ifndef NISHIPPING

	++m_nNumDeallocations[eCategory];

	int* pMemory = static_cast<int*>(pBytes) - 1;

	const int nNumBytes = *pMemory;

	m_nBytesReleased[eCategory] += nNumBytes;
	m_nTotalCurrentlyAllocated -= nNumBytes;

	NiFree(pMemory);

#else

	NiFree(pBytes);

#endif // NISHIPPING
}		

void PgFxAllocator::LogReport()
{
#ifndef NISHIPPING


	int nTotalAllocations = 0;
	int nTotalDeallocations = 0;

	int nTotalAllocated = 0;
	int nTotalDeallocated = 0;

	NILOG( "---------------------------------------------------\n" );
	NILOG( "\n" );
	NILOG( "PgFxAllocator Report :\n" );
	for( int i = 0; i < FxStudio::Allocator::MaxMemoryCategories; ++i )
	{
		nTotalAllocations += m_nNumAllocations[i];
		nTotalDeallocations += m_nNumDeallocations[i];

		nTotalAllocated += m_nBytesAllocated[i];
		nTotalDeallocated += m_nBytesReleased[i];

		NILOG( "\n" );
		NILOG( "\t%s\n", g_MemoryCategoryName[i] );
		NILOG( "\t\tMax Single Allocation      : %d\n", m_nMaxAllocation[i] );
		NILOG( "\t\tMax Concurrent Allocations : %d\n", m_nMaxConcurrentlyAllocated[i] );
		NILOG( "\t\tBytes Allocated   : %d\n", m_nBytesAllocated[i] );
		NILOG( "\t\tBytes Released    : %d\n", m_nBytesReleased[i] );		
		NILOG( "\t\tNum Allocations   : %d\n", m_nNumAllocations[i] );
		NILOG( "\t\tNum Deallocations : %d\n", m_nNumDeallocations[i] );
	};

	NILOG( "\n" );
	NILOG( "Allocations Outstanding : %d\n", nTotalAllocations - nTotalDeallocations );
	NILOG( "Bytes Outstanding       : %d\n", nTotalAllocated - nTotalDeallocated );

	NILOG( "\n" );
	NILOG( "Maximum Memory Size     : %d\n", m_nMaxTotalConcurrentlyAllocated );

	NILOG( "\n" );
	NILOG( "Total Bytes Allocated   : %d\n", nTotalAllocated );
	NILOG( "Total Allocations       : %d\n", nTotalAllocations );
	NILOG( "\n" );
	NILOG( "---------------------------------------------------\n" );

#endif // NISHIPPING

}