#pragma once

template <class T> 
struct CreateUsingNiNew
{
	static T* Create()
	{
		return ::new T; 
	}

	static void Destroy(T* p)
	{
		if(p)
		{
			::delete p; 
		}
	}
};

template <class T>
struct CreateArrayUsingNiNew
{
  static T* Create(const size_t count)
  { 
	  return NiNew T[count]; 
  }
  
  static void Destroy(T* &p)
  { 
	  NiDelete []p; 
	  p = NULL;
  }
};