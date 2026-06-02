#ifndef FREEDOM_DRAGONICA_UTIL_PGERRORCODETABLE_H
#define FREEDOM_DRAGONICA_UTIL_PGERRORCODETABLE_H

#include "PgIXmlObject.h"
#include "PgXmlLoader.h"


class	PgErrorCodeTable	:	public	PgIXmlObject
{
	typedef std::map< unsigned long, std::wstring> TextMap;

	TextMap	m_TextMap;

public:
	PgErrorCodeTable()	{	Init();	}
	virtual	~PgErrorCodeTable()	{	Destroy();	}

	//! Node¸¦ ÆÄ½ÌÇÑ´Ù.
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

	std::wstring const &GetTextW(unsigned long ulIndex)const;

private:

	void	Init();
	void	Destroy();
};

extern	PgErrorCodeTable	*g_pkErrorCT;
#define ECTW(index) (g_pkErrorCT->GetTextW(index))
#endif // FREEDOM_DRAGONICA_UTIL_PGERRORCODETABLE_H