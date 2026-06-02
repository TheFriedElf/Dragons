
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : MemPool.h
// Description : Memory pooling module (None-CRTP type class).
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/13 LeeJR First Created
//

#ifndef _BENCHMARK_TOOLS_NPMEMPOOL_H__
#define _BENCHMARK_TOOLS_NPMEMPOOL_H__


namespace NewWare
{

namespace Tools
{


template < typename T, unsigned int SIZE = 1024 > 
class MemPool
{
private:
    struct Plex
    {
        static Plex* pkFreeListHead;
        static unsigned int uiNumOfUsed;

        union { 
            BYTE storeBuffer[ sizeof(T) ];
            Plex* pkNext;
        };
    };


public:
    void* operator new( size_t )
    {
        // Enough memory free-list?
        if ( Plex::pkFreeListHead )
        {
            Plex* pkTmp = Plex::pkFreeListHead;
            Plex::pkFreeListHead = Plex::pkFreeListHead->pkNext;
            return pkTmp;
        }
        // Enough memory slot?
        else if ( Plex::uiNumOfUsed < SIZE )
            return &ms_akPool[ Plex::uiNumOfUsed++ ]; // Return to the unused memory slot
        // Not enough memory!
        else
        {
            assert( 0 && "Not enough memory!" );
            throw std::bad_alloc();
        }
    }

    // Add to memory free-list.
    void operator delete( void* p )
    {
        static_cast<Plex*>(p)->pkNext = Plex::pkFreeListHead;
        Plex::pkFreeListHead = static_cast<Plex*>(p);
    }


private:
    static Plex ms_akPool[ SIZE ]; // Memory slot
};


template < typename T, unsigned int SIZE > 
typename MemPool<T, SIZE>::Plex* MemPool<T, SIZE>::Plex::pkFreeListHead = NULL;

template < typename T, unsigned int SIZE > 
unsigned int MemPool<T, SIZE>::Plex::uiNumOfUsed = 0;

template < typename T, unsigned int SIZE > 
typename MemPool<T, SIZE>::Plex MemPool<T, SIZE>::ms_akPool[SIZE];


} //namespace Tools

} //namespace NewWare


#endif //_BENCHMARK_TOOLS_NPMEMPOOL_H__
