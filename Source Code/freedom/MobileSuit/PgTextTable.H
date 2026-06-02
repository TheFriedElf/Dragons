#ifndef FREEDOM_DRAGONICA_UTIL_PGTEXTTABLE_H
#define FREEDOM_DRAGONICA_UTIL_PGTEXTTABLE_H

#include "PgIXmlObject.h"
#include "PgXmlLoader.h"


class	PgTextTable	:	public	PgIXmlObject
{
	typedef std::map< unsigned long, std::wstring> TextMap;

	TextMap	m_TextMap;

public:
	PgTextTable()	{	Init();	}
	virtual	~PgTextTable()	{	Destroy();	}

	void Clear();

	//! Node를 파싱한다.
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

	std::wstring const &GetTextW(unsigned long ulIndex)const;

	static void WordWrap( std::wstring &wstrWord );

private:

	void	Init();
	void	Destroy();
};

extern	PgTextTable	*g_pkTT;

inline	std::wstring const&	TTW(unsigned long ulIndex)
{
	PG_ASSERT_LOG(g_pkTT);
	if(!g_pkTT)
	{
		const	static		std::wstring kTempStr(_T(""));
		return	kTempStr;
	}
	return	g_pkTT->GetTextW(ulIndex);
}

bool FormatTTW(std::wstring& rkOut, int const iTTW, ...);


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Format 된 메시지를 원하는 문자로 변환하기 위한 함수

typedef struct tagSTextChangeInfo
{
	std::wstring wFrom;
	std::wstring wTo;

	tagSTextChangeInfo(std::wstring wFind, std::wstring wChangeTo)
	{
		wFrom = wFind;
		wTo = wChangeTo;
	}

} STextChangeInfo;

typedef std::vector<STextChangeInfo> VEC_TextChangeInfo;

extern void TextChange_Item(std::wstring &strText, CItemDef const *pkItemDef);
#endif // FREEDOM_DRAGONICA_UTIL_PGTEXTTABLE_H