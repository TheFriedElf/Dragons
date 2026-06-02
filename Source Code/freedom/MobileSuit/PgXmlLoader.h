#ifndef FREEDOM_DRAGONICA_XML_PGXMLLOADER_H
#define FREEDOM_DRAGONICA_XML_PGXMLLOADER_H

#include "PgWorkerThread.h"

typedef enum ePrepareXmlDocumentAfterWork
{
	PXDAW_None = 0,
	PXDAW_ParseQuest,
} EPrepareXmlDocumentAfterWork;

class PgIXmlObject;

typedef std::list< std::string > CONT_XML_ID;

typedef std::map<std::string, std::string> XmlPathContainer;		// < ID, Path >
typedef std::map<std::string, std::string> XmlTypeContainer;		// < ID, Type >
typedef std::map<std::string, TiXmlDocument *> XmlCacheContainer;	// < Path, * >

class PgXmlLoader : public NiMemObject, public PgIClientWorker
{
protected:
	PgXmlLoader();

public:
	~PgXmlLoader();

	//	leesg213 2006-11-24
	//	½Ì±ÛÅæ ¸Þ¸ð¸® ÇØÁ¦
	static	void	DestroySingleton();

	static PgXmlLoader *Get();
	static bool PrepareObject(char const *pcID);
	static bool PrepareXmlDocument(char const *pcPath, int const iAfterWork = PXDAW_None, bool const bUseThreadLoading = true);
	static PgIXmlObject *CreateObject(char const *pcID, void *pArg = 0, PgIXmlObject *pkObject = 0, char const* pcObjectType = 0, int const iDataType = 0);
	static void CreateObjectByType(char const *pcType);
	static PgIXmlObject *CreateObject(const TiXmlNode *pkNode, void *pArg = 0, PgIXmlObject *pkObject = 0, char const* pcObjectType = 0, bool bUTF8 = false);
	static PgIXmlObject *CreateObjectFromFile(char const *pcPath, void *pArg = 0, PgIXmlObject *pkObject = 0);
	static PgIXmlObject *CreateObjectFromBuffer(char const *pcPath,char const *pcBuffer,void *pArg = 0,PgIXmlObject *pkObject = 0);
	static void LoadAllDocumentToCache();
	static bool IsFile(char const * szFileName);
	static bool LoadFile(TiXmlDocument &rkXmlDoc, std::wstring const &rkFileName);
	static void ReleaseXmlDocumentInCacheByPath(char const *pcPath);
	static void ReleaseXmlDocumentInCacheByID(char const *pcID);
	static void ReleaseXmlDocumentInCacheByType(char const *pcType);
	static TiXmlDocument *GetXmlDocumentInCacheByPath(char const *pcPath, char const* pcID = 0);
	static TiXmlDocument *GetXmlDocumentByID(char const *pcID);
	static char const* GetXmlTagValue(const TiXmlNode* pkRootNode, char const* pcTargetTagName, char const* pcTargetAttributeName = NULL);
	static char const* GetPathByID(char const *pcID);
	static	char	const*	GetPackFilePath();

	//static bool ReloadObject(BM::GUID %rkGuid, PgIXmlObject *pkObject);

	virtual bool DoClientWork(WorkData& rkWorkData);
protected:
	bool Initialize();
	bool LoadPath(char const *pcPathXml);
	bool ParserXml_Path(TiXmlElement const* pkElement, XmlPathContainer & rkXmlPathContainer, XmlTypeContainer & rkXmlTypeContainer);
	PgIXmlObject *CreateInstance(char const *pcClassName, char const* pcObjectType);
	static void GetXmlIDByType(PgXmlLoader* pkSelf, CONT_XML_ID& rkList, char const* pcType);
		
protected:
	static PgXmlLoader *ms_pkSelf;
	XmlPathContainer m_kXmlPathContainer;
	XmlTypeContainer m_kXmlTypeContainer;
	XmlCacheContainer m_kXmlCacheContainer;

	Loki::Mutex m_kCacheLock;
	static Loki::Mutex m_kCreateLock;
};
#endif // FREEDOM_DRAGONICA_XML_PGXMLLOADER_H