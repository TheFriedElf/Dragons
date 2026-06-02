#ifndef FREEDOM_DRAGONICA_UTIL_PGIRESOURCELOADCONTROLLER_H
#define FREEDOM_DRAGONICA_UTIL_PGIRESOURCELOADCONTROLLER_H

class PgIResourceLoadController
{
public:
	PgIResourceLoadController();
	virtual ~PgIResourceLoadController();

	bool LoadResourcesAtStage(int iStage);
	bool LoadAllResource();
	void RegisterResource(char const* pcResourcePath, int iStage);

	bool IsStageLoaded(int iStage);
	bool IsAllResourceLoaded();

protected:
	virtual bool LoadResource(char const* pcResourcePath) = 0;

	typedef std::list<std::string> RESOURCE_PATH_LIST;
	typedef std::map<int, RESOURCE_PATH_LIST> STAGE_MAP;

	STAGE_MAP m_kStageMap;
};
#endif // FREEDOM_DRAGONICA_UTIL_PGIRESOURCELOADCONTROLLER_H