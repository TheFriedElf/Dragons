
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : CreateProxy.h
// Description : Create proxy for class instance
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/10/07 LeeJR First Created
//

#ifndef _BENCHMARK_TOOLS_CREATEPROXY_H__
#define _BENCHMARK_TOOLS_CREATEPROXY_H__


namespace NewWare
{

namespace Tools
{


template < class TSelf, class TAbstract > 
class CreateProxy
{
protected:
    CreateProxy() { /* Nothing */ };

public:
    // General creation functions.
    static TSelf* Create()
    {
        TSelf* ptThis = new TSelf;
        assert( ptThis );
        return ptThis->ReturnInstance();
    }
    template < typename ArgT1 > 
    static TSelf* CreateArg1( ArgT1 tArg1 )
    {
        TSelf* ptThis = new TSelf( tArg1 );
        assert( ptThis );
        return ptThis->ReturnInstance();
    }
    template < typename ArgT1, typename ArgT2 > 
    static TSelf* CreateArg2( ArgT1 tArg1, ArgT2 tArg2 )
    {
        TSelf* ptThis = new TSelf( tArg1, tArg2 );
        assert( ptThis );
        return ptThis->ReturnInstance();
    }


    // Decorator creation functions.
    static TSelf* CreateDecorator( TAbstract* pkDecorator )
    {
        TSelf* ptThis = new TSelf( pkDecorator );
        assert( ptThis );
        return ptThis->ReturnInstance();
    }
    template < typename ArgT1 > 
    static TSelf* CreateDecoratorArg1( TAbstract* pkDecorator, ArgT1 tArg1 )
    {
        TSelf* ptThis = new TSelf( pkDecorator, tArg1 );
        assert( ptThis );
        return ptThis->ReturnInstance();
    }
    template < typename ArgT1, typename ArgT2 > 
    static TSelf* CreateDecoratorArg2( TAbstract* pkDecorator, ArgT1 tArg1, ArgT2 tArg2 )
    {
        TSelf* ptThis = new TSelf( pkDecorator, tArg1, tArg2 );
        assert( ptThis );
        return ptThis->ReturnInstance();
    }


protected:
    virtual TSelf* ReturnInstance()
    {
        return static_cast<TSelf*>(this);
    }
};


} //namespace Tools

} //namespace NewWare


#endif //_BENCHMARK_TOOLS_CREATEPROXY_H__
