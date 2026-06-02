#ifndef FREEDOM_DRAGONICA_FILTERSTIRNG_PGCOMMANDMGR_H
#define FREEDOM_DRAGONICA_FILTERSTIRNG_PGCOMMANDMGR_H
#include "CreateUsingNiNew.inl"
#include "PgIXmlObject.h"
#include "PgXmlLoader.h"
#include <regex>

class PgCommandMgr
	:	public	PgIXmlObject
{
public:
	PgCommandMgr(){};
	virtual ~PgCommandMgr(){};

public:
	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	bool FindCommand(std::wstring const &InWord, std::wstring const* &pOut)const;//커멘드를 찾아
protected:
	typedef std::vector< std::wstring > CONT_WORD;
	typedef std::map< CONT_WORD, std::wstring > SCRIPT_HASH;
	SCRIPT_HASH m_kCommand;//말에 따른 스크립트 
};

#define g_kCmdMgr SINGLETON_CUSTOM(PgCommandMgr, CreateUsingNiNew)


#include "BM/PgFilterString.h"
class PgCilentFilterString
	:	public	BM::PgFilterString
	,	public	PgIXmlObject
{
public:
	PgCilentFilterString(){}
	virtual ~PgCilentFilterString(){}

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	virtual bool Filter(std::wstring &str, bool const bIsConvert, EFilterSourceType const eFilterType = FST_BADWORD)const;

protected:
	// 정규표현식을 사용하는 필터링
	bool RegExFilter(std::wstring& kStr, std::wregex const& kRegEx) const;

protected:
	std::wstring m_kBadWordPattern;
	std::wregex m_kBadWordRegex;

	std::wstring m_kBlockNickNamePattern;
	std::wregex m_kBlockNickNameRegex;
};

#define g_kClientFS SINGLETON_CUSTOM(PgCilentFilterString, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_FILTERSTIRNG_PGCOMMANDMGR_H