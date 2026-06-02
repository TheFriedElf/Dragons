#ifndef FREEDOM_DRAGONICA_UI_PGRESOURCEICON_H
#define FREEDOM_DRAGONICA_UI_PGRESOURCEICON_H

typedef struct tagIconImage
{
	tagIconImage()
	{
		iWidth = 0;
		iHeight = 0;
		iMaxX = 0;
		iMaxY = 0;
	}
	std::wstring wstrID;
	int iWidth;
	int iHeight;
	int iMaxX;
	int iMaxY;
	std::wstring wstrPath;
} SIconImage;

typedef struct tagResourceIcon
{
	tagResourceIcon()
	{
		iIdx = 0;
	}
	std::wstring wstrID;
	std::wstring wstrImageID;
	int iIdx;
} SResourceIcon;

class PgResourceIcon : public PgIXmlObject
{
public:
	typedef std::map< std::wstring, SIconImage >		IconImageCont;
	typedef std::map< std::wstring, SResourceIcon >		IconCont;
	typedef std::map< std::wstring, std::wstring >		IconIDCont;

public:
	PgResourceIcon(void);
	virtual ~PgResourceIcon(void);

	bool Initialize();
	void Terminate();

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	bool PgResourceIcon::ParseXml_IconImage(const TiXmlNode *pkNode, void *pArg);

	bool GetIcon(SResourceIcon& rkRetIcon, std::wstring wstrID);
	bool GetIconImage(SIconImage& rkRetIconImage, std::wstring wstrID);

	bool AddIconID(std::wstring wstrActorName, std::wstring wstrXMLFilePath, std::wstring wstrID);
	bool GetIconIDFromXMLPath(std::wstring wstrXMLFilePath, std::wstring& rkRetID);
	bool GetIconIDFromActorName(std::wstring wstrActorName, std::wstring& rkRetID);

protected:
	IconImageCont	m_kIconImageCont;
	IconCont		m_kIconCont;
	IconIDCont		m_kIconID_KeyPath;
	IconIDCont		m_kIconID_KeyName;
	Loki::Mutex		m_kIconLock;
};

#define g_kResourceIcon SINGLETON_CUSTOM(PgResourceIcon, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_UI_PGRESOURCEICON_H