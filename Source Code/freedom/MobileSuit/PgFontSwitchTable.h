#ifndef FREEDOM_DRAGONICA_UI_PGFONTSWITCHTABLE_H
#define FREEDOM_DRAGONICA_UI_PGFONTSWITCHTABLE_H

typedef struct tagSysEmotionKey
{
	tagSysEmotionKey(){ Clear(); }
	explicit tagSysEmotionKey(int const Key) : iKey(Key), kText() { kText.clear(); }
	tagSysEmotionKey(int const Key, std::string const& Text) : iKey(Key), kText(Text) {}
	void Clear()
	{
		iKey = 0;
		kText.clear();
	}

	bool operator == (std::string const& rhs) const
	{
		return kText == rhs;
	}

	bool operator == (tagSysEmotionKey const& rhs) const
	{
		return iKey == rhs.iKey;	
	}

	bool operator < (tagSysEmotionKey const& rhs) const
	{
		return iKey < rhs.iKey;
	}

	bool operator > (tagSysEmotionKey const& rhs) const
	{
		return rhs < *this;
	}

	int iKey;
	std::string kText;
}S_SYS_EMO_KEY;

class PgFontSwitchTable
{
public:
	PgFontSwitchTable();
	virtual ~PgFontSwitchTable();

	bool ParseXML(char const* strXMLPath);
	void ClearAllData();
	bool Trans_key_value(int const Key, int& TransKey);
	std::wstring Trans_Key_SysFontString(S_SYS_EMO_KEY const& Key);
	std::wstring Trans_Key_SysFontString(int const Key);
	std::wstring Trans_Key_SysFontString(std::string const& Key);

protected:
	bool ExplorerNode(TiXmlNode const* pNode, bool const bSysFont = false);

	typedef std::map< int, int >	kFontSwitchTable;
	typedef std::map< S_SYS_EMO_KEY, int > kSysFontSwitchTable;

	kFontSwitchTable	m_kCont;
	kSysFontSwitchTable	m_kSysCont;
};

#define g_kEmoFontMgr	SINGLETON_STATIC(PgFontSwitchTable)
#endif // FREEDOM_DRAGONICA_UI_PGFONTSWITCHTABLE_H