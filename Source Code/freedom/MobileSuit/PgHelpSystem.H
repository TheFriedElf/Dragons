#ifndef FREEDOM_DRAGONICA_CONTENTS_HELP_PGHELPSYSSTEM_H
#define FREEDOM_DRAGONICA_CONTENTS_HELP_PGHELPSYSSTEM_H

#include <vector>
#include <list>

#include "PgPage.h"

enum E_HELP_ACTION_EX_TYPE
{
	HAET_NONE		= 0,
	HAET_UI_LINK	= 1,
	HAET_OTHER_HELP	= 2,
};

class	PgHelpObject
{
	int m_iHelpUIType;
	std::string	m_kConditionParamValue;
	int	m_iMaxActivatingCount;
	int	m_iID;
	int	m_iTitleTTID;
	int	m_iMessageTTID;
	int m_iHelperMsgTTID;
	std::string	m_kRelatedUIName;
	int m_iCallEventID;
	int m_iMaxLevelLimit;
	int m_iExTitleTTID;
	int m_iExMessageTTID;
	std::string m_kPreviewImgPath;
	int m_iExBtnTTID;
	int m_iExType;

	int	m_iCurrentActivatingCount;
	bool	m_bDoNotActivateAgain;	

public:

	PgHelpObject() : m_iID(0){}
	PgHelpObject(int const iID) : m_iID(iID){}
	PgHelpObject(int const iType, std::string const &kConditionParamValue, int const iMaxActivatingCount,
				int const iID, int const iTitleTTID, int const iMessageTTID, int const iHelperMsgTTID,
				std::string const &kRelatedUIName, int const iCallEventID, int const iMaxLevelLimit,
				int const iExTitleTTID, int const iExMessageTTID, std::string const& PreviewImgPath,
				int const ExBtnTTID, int const ExType);

	virtual	~PgHelpObject();

	virtual	bool	IsCorrectCondition(std::string const &kConditionParamValueNow);
	virtual	void	Activate(bool bIncreaseActivatingCount = true);

	void	SetActivatingCount(int iCount)	{	m_iCurrentActivatingCount = iCount;	}
	int		GetActivatingCount()	{	return	m_iCurrentActivatingCount;	}

public:
	int GetUIType() const			{ return m_iHelpUIType; }
	int	GetID() const				{ return m_iID;	}
	int	GetTitleTTID() const		{ return m_iTitleTTID;	}
	int	GetMessageTTID() const		{ return m_iMessageTTID;	}
	int GetHelperMsgTTID() const	{ return m_iHelperMsgTTID; }
	const std::string&	GetRelatedUIName() const	{ return m_kRelatedUIName;	}
	int GetCallEventID() const		{ return m_iCallEventID; }

	int	GetExTitleTTID() const		{ return m_iExTitleTTID;	}
	int	GetExMessageTTID() const	{ return m_iExMessageTTID;	}
	const std::string&	GetPreviewImgPath() const	{ return m_kPreviewImgPath;	}
	int	GetExBtnTTID() const		{ return m_iExBtnTTID;	}
	int	GetExType() const			{ return m_iExType;	}

	void	SetDoNotActivateAgain(bool bDoNotActivateAgain)	{	m_bDoNotActivateAgain = bDoNotActivateAgain;	}
	bool	GetDoNotActivateAgain()	{	return	m_bDoNotActivateAgain;	}

public:

	static	void	SaveToFile(PgHelpObject *pkHelpObject,FILE *fp);
	static	void	LoadFromFile(PgHelpObject *pkHelpObject,FILE *fp);

};

typedef std::list<PgHelpObject*>	HelpObjectList;
typedef std::vector<PgHelpObject*>	HelpObjectVec;

class	PgHelpConditionUnit
{
	std::string	m_kConditionName;
	HelpObjectVec	m_kHelpObjects;

public:

	PgHelpConditionUnit(std::string const &kConditionName);
	virtual	~PgHelpConditionUnit();

	bool	AddObject(PgHelpObject *pkNewHelpObject);
	void	ActivateObject(std::string const &kConditionParamValueNow);
	void	ActivateObject(int iID);
	int		GetHelpID(std::string const &kConditionParamValueNow);
	
	const	std::string &GetConditionName()	{	return	m_kConditionName;	}

private:

	void	ReleaseAllObject();

};
typedef	std::map<std::string,PgHelpConditionUnit*>		HelpConditionUnitMap;

extern	const	char	*HELP_CONDITION_TRIGGER;
extern	const	char	*HELP_CONDITION_LEVEL_UP;
extern	const	char	*HELP_CONDITION_OPEN_UI;
extern	const	char	*HELP_CONDITION_MAP_MOVE;
extern	const	char	*HELP_CONDITION_PARTY;
extern	const	char	*HELP_CONDITION_REPAIR;
extern	const	char	*HELP_CONDITION_BLACKSMITH;
extern	const	char	*HELP_CONDITION_ITEMNO;
extern	const	char	*HELP_CONDITION_CLASS_ITEM;
extern	const	char	*HELP_CONDITION_MONSTER_KILL_COUNTER;
extern	const	char	*HELP_CONDITION_CHECK_LV;
extern	const	char	*HELP_CONDITION_CURSEITEM;
extern	const	char	*HELP_CONDITION_ACHIEVE;
extern	const	char	*HELP_CONDITION_MISSIONCOMPLETE;

class	PgHelpSystem
{
	friend	class	PgHelpObject;

	HelpConditionUnitMap	m_kHelpConditionUnits;
	HelpObjectVec	m_kHelpObjects;
	HelpObjectList	m_kHelpObjectList;

	PgHelpObject	*m_pkActivatedObject;	//	현재 화면에 나오고 있는 헬프 오브젝트

public:

	PgHelpSystem();
	virtual	~PgHelpSystem();

	void	InitSystem();
	void	TerminateSystem();
	void	Clear();

public:

	//	조건에 의해 헬프 오브젝트를 활성화 시킨다.
	void	ActivateByCondition(std::string const &kConditionName,std::string const kConditionValue);
	void	ActivateByCondition(std::string const &kConditionName,int iConditionValue);
	void	ActivateByHelpID(int const iHelpID);

	int		ChangeConditionNameToHelpID(std::string const &kConditionName, std::string const& kConditionValue);

	//	다음,이전 헬프 오브젝트를 활성화 시킨다.
	void	ActivateNextObject();	
	void	ActivatePrevObject();	

	//	세이브 파일 저장,로딩
	void	LoadHelpInfoFile(BM::GUID const &rkGuid);
	void	SaveHelpInfoFile(BM::GUID const &rkGuid);

	HelpObjectList const& GetHelpObjectList() const {return m_kHelpObjectList;}
	PgPage& GetHelpPage() { return m_kHelpPage; };
	
	void GetHelpDisplayItem(HelpObjectList& rkObjectList);

	PgHelpObject const& GetHelpObject(int iObjectID, bool bFromList = false);
	//  렐름 정보
	CLASS_DECLARATION_S(short, RealmNo);

	void SortHelpObjectList();
public:

	//	현재 활성화된 오브젝트 설정
	void	SetActivatedObject(PgHelpObject *pkObject)	{	m_pkActivatedObject = pkObject;	}
	PgHelpObject*	GetActivatedObject()	{	return	m_pkActivatedObject;	}

	static bool ConvertGuidToSavePathName(BM::GUID const &rkGuid, std::wstring &rkOutPathName);
private:

	PgHelpObject*	GetObject(int iObjectID);
	PgHelpObject*	GetObjectFromList(int iObjectID);

	PgHelpConditionUnit*	GetHelpConditionUnit(std::string const &kConditionName);
	PgHelpConditionUnit*	CreateHelpConditionUnit(std::string const &kConditionName);

	template <typename T>
	void	ParseXML(char const *strXMLPath, T& kArray, bool const bMakeCondition = true);

	void	ReleaseAllObject();

	void	ResetActivatingCount();


	CLASS_DECLARATION_S(BM::GUID, HelperGuid);
	BM::GUID m_kLoadedHelpGuid;
	PgPage m_kHelpPage;
};

extern	PgHelpSystem	g_kHelpSystem;

#endif // FREEDOM_DRAGONICA_CONTENTS_HELP_PGHELPSYSSTEM_H