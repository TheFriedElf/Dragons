#ifndef FREEDOM_DRAGONICA_XML_PGACTIONXMLDATA_H
#define FREEDOM_DRAGONICA_XML_PGACTIONXMLDATA_H

class PgActionXmlData
{
public:
	typedef std::vector<std::string> CONT_STR;
public:
	bool Init();
	bool GetXmlPath(std::string kActionID, std::string& rkOut) const
	{
		rkOut.clear();
		UPR(kActionID);
		XmlPathContainer::const_iterator kItor = m_kContPath.find(kActionID);
		if(m_kContPath.end() == kItor)
		{
			return false;
		}
		rkOut = (*kItor).second;
		return true;
	}

	bool GetXmlData_ByPath(std::string kPath, std::string& rkOut) const
	{
		rkOut.clear();
		UPR(kPath);
		XmlPathContainer::const_iterator kItor = m_kContXmlData.find(kPath);
		if(m_kContXmlData.end() == kItor)
		{
			return false;
		}
		rkOut = (*kItor).second;
		return true;
	}

	bool GetXmlData_ByActionID(std::string kActionID, std::string& rkOut) const
	{
		UPR(kActionID);
		std::string kPath; 
		if( !GetXmlPath(kActionID, kPath) )
		{
			return false;
		}		
		return GetXmlData_ByPath( kPath, rkOut);
	}

	CONT_STR const& GetContLuaData() const
	{
		return m_kContLuaText;
	}

private:
	XmlPathContainer m_kContPath; 		// ActionID, Path	
	XmlPathContainer m_kContXmlData;	// Path, XmlData;
	CONT_STR m_kContLuaText;
};

#define g_kActionXmlData SINGLETON_STATIC(PgActionXmlData)

#endif // FREEDOM_DRAGONICA_XML_PGXMLLOADER_H
