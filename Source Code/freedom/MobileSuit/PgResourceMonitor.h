#ifndef FREEDOM_DRAGONICA_UTIL_PGRESOURCEMONITOR_H
#define FREEDOM_DRAGONICA_UTIL_PGRESOURCEMONITOR_H

#ifndef EXTERNAL_RELEASE
	#define PG_RESOURCE_MONITOR(x)	(x);
#else
	#define PG_RESOURCE_MONITOR(x)	__noop;
#endif


class PgResourceMonitor
{
public:
	typedef enum 
	{
		RESOURCE_TYPE_NONE = -1,
		RESOURCE_TYPE_GENERAL_FILE,
		RESOURCE_TYPE_NIF,
		RESOURCE_TYPE_TEXTURE,
		RESOURCE_TYPE_PARTICLE,
		RESOURCE_TYPE_AUDIO,
		RESOURCE_TYPE_ACTION,
		MAX_RESOURCE_TYPE_NUM,
	} ResourceType;

	typedef struct _ResourceCounter
	{
		_ResourceCounter() { iTotalCount = 0; fFirstResourceRequestTime = 0.0f; fLastResourceRequestTime = 0.0f; fAverageResourceReuqestGap = 0.0f; eResourceType = RESOURCE_TYPE_NONE; }
		//std::string strName;
		unsigned int iTotalCount;
		float fFirstResourceRequestTime;
		float fLastResourceRequestTime;
		float fAverageResourceReuqestGap;
		ResourceType eResourceType;
	} ResourceCounter;

	PgResourceMonitor();
	~PgResourceMonitor();

	void IncreaseResourceCounter(char const* pcResourceName, ResourceType eType = RESOURCE_TYPE_GENERAL_FILE);
	void IncreaseResourceCounter(const std::string& rkResourceName, ResourceType eType = RESOURCE_TYPE_GENERAL_FILE);
	void PrintResourceCounters(ResourceType eType = MAX_RESOURCE_TYPE_NUM);
	void PrintResourceCountersForCSV(ResourceType eType = MAX_RESOURCE_TYPE_NUM);

protected:
	typedef std::map<std::string, ResourceCounter> ResourceMap;
	ResourceMap m_kResourceMap;
	mutable Loki::Mutex m_kMutex;
};

extern PgResourceMonitor g_kResourceMonitor;
#endif // FREEDOM_DRAGONICA_UTIL_PGRESOURCEMONITOR_H