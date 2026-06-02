
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : Compiles.h
// Description : Support for compile time and compiler.
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _BENCHMARK_TOOLS_COMPILES_H__
#define _BENCHMARK_TOOLS_COMPILES_H__


namespace NewWare
{

namespace Tools
{


namespace Compiles
{


// Sealing classes
template < int T > 
class _Sealed
{
protected:
    _Sealed() { /* Nothing */ };
};

#define Sealed(name_space)  private virtual name_space##::Compiles::_Sealed<__COUNTER__>
#define Final(name_space)   Sealed(name_space)

/** @example - test code

    #include "Compiles.h"

    class Foo0 : Sealed {};
    class Foo1 : public Foo0 {};
    Foo1 kFoo1; // error C2248

    class Foo2 : public Foo0, Sealed {};
    Foo2 kFoo2; // error C2248
*/


} //namespace Compiles


} //namespace Tools

} //namespace NewWare


#endif //_BENCHMARK_TOOLS_COMPILES_H__
