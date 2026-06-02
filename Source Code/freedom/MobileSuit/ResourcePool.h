#ifndef FREEDOM_DRAGONICA_UTIL_RESOURCEPOOL_H
#define FREEDOM_DRAGONICA_UTIL_RESOURCEPOOL_H

template<typename T_Key, typename T_Res>
class CResourcePool
{
	typedef std::map<T_Key, T_Res> Container;

public:
	CResourcePool(void)
	{
	}

	~CResourcePool(void)
	{
	}

	bool Add(T_Key& rkKey, T_Res& rkRes)
	{
		return m_kContainer.insert(std::make_pair(rkKey, rkRes)).second;
	}

	void Delete(T_Key& rkKey)
	{
		m_kContainer.erase(rkKey);
	}

	T_Res& Get(T_Key& rkKey)
	{
		static T_Res null;
		Container::iterator itr = m_kContainer.find(rkKey);

		if(itr != m_kContainer.end())
		{
			return itr->second;
		}

		assert(!"resouce not found");
		return null;
	}
	//	leesg213 2006-11-22
	//	리소스 풀 내의 오브젝트를 메모리 해제하기 위한 메소드
	void	ReleaseAll()
	{
		for(Container::iterator itr = m_kContainer.begin();
			itr != m_kContainer.end();
			++itr)
		{
			SAFE_DELETE_NI(itr->second);
		}
		m_kContainer.clear();
	}
	//	leesg213 2006-11-27
	//	리소스 풀 내의 오브젝트들을 컨테이너에서 제거
	void	clear()
	{
		m_kContainer.clear();
	}

protected:
	Container m_kContainer;
};

#endif / /FREEDOM_DRAGONICA_UTIL_RESOURCEPOOL_H