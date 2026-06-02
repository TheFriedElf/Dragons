
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : Noncopyable.h
// Description : None copyable of class.
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _BENCHMARK_TOOLS_NONCOPYABLE_H__
#define _BENCHMARK_TOOLS_NONCOPYABLE_H__


namespace NewWare
{

namespace Tools
{


class __declspec(novtable) Noncopyable
{
protected:
    Noncopyable() { /* Nothing */ };
    ~Noncopyable() { /* Nothing */ };


private: // emphasize the following members are private
    Noncopyable( Noncopyable const& );                   // no implementation
    Noncopyable const& operator= ( Noncopyable const& ); // no implementation
};


} //namespace Tools

} //namespace NewWare


#endif //_BENCHMARK_TOOLS_NONCOPYABLE_H__
