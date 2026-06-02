
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : BitFlag.h
// Description : Bit flag modules.
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _BENCHMARK_TOOLS_BITFLAG_H__
#define _BENCHMARK_TOOLS_BITFLAG_H__


namespace NewWare
{

namespace Tools
{


template < typename T >
class BitFlag
{
public:
    BitFlag() : m_tFlag(static_cast<T>(0)) { /**/ };


    void Reset() { m_tFlag = static_cast<T>(0); };


    bool operator== ( BitFlag<T> const& rhs ) const { return m_tFlag == rhs.m_tFlag; };
    bool operator== ( T flag ) const { return m_tFlag == flag; };
    bool operator!= ( BitFlag<T> const& rhs ) const { return !(*this == rhs); };
    bool operator!= ( T flag ) const { return !(*this == flag); };


    T GetFlag() const { return m_tFlag; };


    void SetField( T val, T mask, T pos ) { m_tFlag = (m_tFlag & ~mask) | (val << pos); };
    T GetField( T mask, T pos ) const { return (m_tFlag & mask) >> pos; };

    void (SetBit)( bool bVal, T mask ) { // SetBit MACRO FUNCTION!!!!!
        if ( bVal ) { 
            m_tFlag |= mask;
        } else { 
            m_tFlag &= ~mask;
        }
    }
    bool GetBit( T mask ) const { return (m_tFlag & mask) != 0; };


private:
    T m_tFlag;
};


} //namespace Tools

} //namespace NewWare


#endif //_BENCHMARK_TOOLS_BITFLAG_H__
