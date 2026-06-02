
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 Barunson Interactive, Inc
//        Name : Singleton.h
// Description : Singleton pattern for class (CRTP)
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * .
// 
// Revisions :
//  09/09/30 LeeJR First Created
//

#ifndef _BENCHMARK_TOOLS_SINGLETON_H__
#define _BENCHMARK_TOOLS_SINGLETON_H__


namespace NewWare
{

namespace Tools
{


// Template class for creating single-instance global classes.
template < class T > 
class Singleton
{
protected:
    Singleton()
    {
        assert( NULL == ms_ptSingleton );
#if defined(_MSC_VER) && _MSC_VER < 1200
        int offset = (int)(T*)1 - (int)(Singleton<T>*)(T*)1;
        ms_ptSingleton = (T*)((int)this + offset);
#else  //#if defined(_MSC_VER) && _MSC_VER < 1200
        ms_ptSingleton = static_cast<T*>(this);
#endif //#if defined(_MSC_VER) && _MSC_VER < 1200
    }
    virtual ~Singleton()
    {
        assert( NULL != ms_ptSingleton );
        ms_ptSingleton = NULL;
    }


public:
    __declspec(noinline) static T* GetSingleton() { return ms_ptSingleton; };
    __declspec(noinline) static T* GetInstance() { 
        T* ptInstance = GetSingleton();
        assert( NULL != ptInstance );
        return ptInstance;
    }


private:
    static T* ms_ptSingleton;
};

template < class T > T* Singleton<T>::ms_ptSingleton = NULL;


#define GetSingletonProxy(class_name)   class_name##::GetSingleton()
#define GetInstanceProxy(class_name)    class_name##::GetInstance()


} //namespace Tools

} //namespace NewWare


#endif //_BENCHMARK_TOOLS_SINGLETON_H__
