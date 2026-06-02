#ifndef FREEDOM_DRAGONICA_OPTION_PGOPTION_H
#define FREEDOM_DRAGONICA_OPTION_PGOPTION_H

#include "CreateUsingNiNew.inl"

#include "PgXmlLoader.h"
#include "PgIXmlObject.h"
#include "FreedomDef.h"

	//Hardware Key Code -> Unique Key Code
	//Unique Key Code = Code, Name(=TTW), ImageName(40x40 size)

	//Run First (Default -> Saved)
	//Set config(Cur -> Current)
	//Apply(Current -> Saved, Aplly System Current)
	//Next Run(Saved -> Current)
	//Set Config(Cur -> Current)
	//Cancel(Saved -> Current)
	//
enum
{
	EOLD_OPTOIN_DB_VER = 104,
	ENEW_OPTION_DB_VER = 902, // New Ver2, 2009 07 13

	ENIKEY_TO_UKEY		= 1000,
	ENIKEY_IGNORE_VALUE	= 0xFF,
};

class PgOption
	: public PgIXmlObject
{
public:
	PgOption();
	virtual ~PgOption();

public:
	typedef std::map< std::string, std::pair< int, std::string > > ContConfigMap;
protected:
	typedef std::map< unsigned int, std::pair< std::wstring, int > > ContKeySetMap;
	typedef std::map< int, std::string > ContKeynoKeystr;
	typedef std::map< int, int > ContUKeyToKey;

	typedef enum eOption
	{
		O_MaxConfigCount = 300,
	} EOption;

public:
	virtual bool ParseXml(TiXmlNode const* pkNode, void* pArg = 0, bool bUTF8 = false);//Path.xml
	bool ParseItem(TiXmlElement const* pkNode);
	bool ParseItem_KeySet(TiXmlElement const* pkNode);

	//
protected:
	bool SetConfig(std::string const& szHeadKey, std::string const& szKey, int const* piValue, char const* szText, ContConfigMap* pkConfigMap);
public:
	int const GetValue(std::string const& szHeadKey, std::string const& szKey) const;
	int const GetDefaultValue(std::string const& szHeadKey, std::string const& szKey) const;
	char const* GetText(std::string const& szHeadKey, std::string const& szKey) const;
	char const* GetDefaultText(std::string const& szHeadKey, std::string const& szKey) const;
	int const GetCurrentGraphicOption(std::string const& szKey) const;
	void SetCurrentGraphicOption(std::string const& szKey, int iValue);

	bool SetConfig(std::string const& szHeadKey, std::string const& szKey, int const iValue, char const* szText);
	bool SysSetConfig(std::string const& szHeadKey, std::string const& szKey, int const iValue, char const* szText);
	bool Save(bool const bSendToServer = false);//Save User xml "Release\Config.xml"
	bool Load();//Load User xml "Release\Config.xml"
	bool Diff();//Config Version Manangement

	void RollBackAll(ContConfigMap* pkFrom, ContConfigMap* pkTo);
	void RollBackConfig(ContConfigMap* pkFrom, ContConfigMap* pkTo);
	void RollBackKeySet(ContConfigMap* pkFrom, ContConfigMap* pkTo);
	//
	void ApplyConfig();
	void DefaultConfig();
	void CancelConfig();
	//
	void ApplyKeySet();
	void DefaultKeySet();
	void CancelKeySet();

	void ApplyKeySet_ToSystem();
	bool GetKeyFuncResource(const unsigned int iUKey, std::wstring& rkName, unsigned int& rkResNo) const;//Key UKEY -> Function Name, Icon Resource No
	void MakeKeynoToKeystr();
	bool GetKeynoToKeystr(int const iKeyNo, std::string& rkKeyStr) const;//iKey No -> Keyname
	int const GetUKeyToKey(int iUKey) const;

	void ReadFromPacket(BM::Stream& rkPacket);	// 서버로 부터 Option값 받아와 시스템에 적용시키기
	void WriteToPacket(BM::Stream& rkPacket);		// 서버에 Option값 저장하기\

	bool IsConfigCreated() const { return m_bConfigCreated; }

	bool const IsDisplayHelmet() const;
	void SetDisplayHelmet(bool const bDisplay);
	
	bool GetUKeyToKeyStr(int const iUKey, std::wstring& kCurrentKeyName_out);

	void OffDisplayHP();// HP 보이기 옵션을 m_bPrevDisplayHP 에 저장하고 꺼줌.
	void RestoreDisplayHP();// 미리 저장된 옵션으로 복구

	void SetSendOptionFlag(int const iValue);//서버가 보낸 옵션 Flag
	bool GetUseLevelRank()const;

	void ShowNavigation(bool const bShow);

private:
	void SetUseLevelRank(bool const IsUse);//제2케릭터생성유도 사용유무

protected:
	ContConfigMap m_kConfigMap;//Current Set "Current Memmory" <Config Type Name, <iVal, Text Val>>
	ContConfigMap m_kSavedMap;//Saved "Release\Config.xml"
	ContConfigMap m_kDefaultMap;//Default "XML\DefualtConfig.xml"

	ContKeySetMap m_kKeyFunctionTable;//Unique Key No, Name, Icon Resource No

	CLASS_DECLARATION_PTR(ContConfigMap* , m_pkCurConfigMap, pCurConfigMap);

	ContKeynoKeystr m_kKeyNoToKeyStr;
	ContUKeyToKey m_kUkeyToKey;
	bool m_bConfigCreated;

	bool m_bPrevDisplayHP;//이벤트 진입시 옵션을 무조건 끄기 위해 이전 값을 저장.
};

#define g_kGlobalOption SINGLETON_CUSTOM(PgOption, CreateUsingNiNew)

#define GET_OPTION_ANTI_ALIAS		(1 == g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_ANTI_ALIAS))
#define GET_OPTION_GLOW_EFFECT		(1 == g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_GLOW))
#define GET_OPTION_VIEW_DISTANCE	(1 == g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_VIEW_DISTANCE))
#define GET_OPTION_WORLD_QUALITY	(1 == g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_WORLD_QUALITY))
#define GET_OPTION_BLOOM			(1 == g_kGlobalOption.GetCurrentGraphicOption(STR_OPTION_WORLD_QUALITY))

//
class lwGlobalOption
{
public:
	lwGlobalOption(PgOption* pkOption)
	{
		m_pkOption = pkOption;
	}

public:
	static bool RegisterWrapper(lua_State* pkState);

private:
	PgOption* m_pkOption;
};

namespace PgOptionUtil
{
	struct SClientDWORDOption
	{
	public:
		CLASS_DECLARATION_S_BIT(bool, DisplayHelmet, 1);		// 1/1  (Byte/Bit)
	private:
		CLASS_DECLARATION_S_BIT(bool, Byte1, 7);				// 1/2~8
		CLASS_DECLARATION_S_BIT(bool, HideCashGlass, 1);		// 2/1
		CLASS_DECLARATION_S_BIT(bool, HideCashHelmet, 1);		// 2/2
		CLASS_DECLARATION_S_BIT(bool, HideCashShoulder, 1);		// 2/3
		CLASS_DECLARATION_S_BIT(bool, HideCashClock, 1);		// 2/4
		CLASS_DECLARATION_S_BIT(bool, HideCashShirts, 1);		// 2/5
		CLASS_DECLARATION_S_BIT(bool, HideCashGlove, 1);		// 2/6
		CLASS_DECLARATION_S_BIT(bool, HideCashWeapon, 1);		// 2/7
		CLASS_DECLARATION_S_BIT(bool, HideCashArm, 1);			// 2/8
		CLASS_DECLARATION_S_BIT(bool, HideCashBoots, 1);		// 3/1
		CLASS_DECLARATION_S_BIT(bool, HideCashPants, 1);		// 3/2
		CLASS_DECLARATION_S_BIT(bool, HideCashBalloon, 1);		// 3/3
		

		//CLASS_DECLARATION_S_BIT(bool, HideGlass, 1);		// 2/1
		//CLASS_DECLARATION_S_BIT(bool, HideNecklace, 1);		// 2/2
		CLASS_DECLARATION_S_BIT(bool, HideHelmet, 1);		// 2/3
		CLASS_DECLARATION_S_BIT(bool, HideShoulder, 1);		// 2/4
		CLASS_DECLARATION_S_BIT(bool, HideClock, 1);		// 2/5
		CLASS_DECLARATION_S_BIT(bool, HideShirts, 1);		// 2/5
		CLASS_DECLARATION_S_BIT(bool, HideGlove, 1);		// 2/6

		//CLASS_DECLARATION_S_BIT(bool, HideBelt, 1);		// 2/7
		CLASS_DECLARATION_S_BIT(bool, HidePants, 1);			// 2/8
		CLASS_DECLARATION_S_BIT(bool, HideBoots, 1);		// 3/1
		//CLASS_DECLARATION_S_BIT(bool, HideRingL, 1);		// 3/2
		//CLASS_DECLARATION_S_BIT(bool, HideRingR, 1);		// 3/2
		//CLASS_DECLARATION_S_BIT(bool, HideEarring, 1);		// 3/2
		//CLASS_DECLARATION_S_BIT(bool, HideAttstone, 1);		// 3/2

		CLASS_DECLARATION_S_BIT(bool, Byte3, 6);				// 3/3~8
		CLASS_DECLARATION_S_BIT(bool, Byte4, 8);				// 4/1~8

	public:
		SClientDWORDOption(DWORD const dwOption);
		SClientDWORDOption(SClientDWORDOption const& rhs);

		void ReadFromDWORD(DWORD const dwOption);
		DWORD WriteToDWORD() const;

		//! Cash Hide Option
		bool IsHideCashInvenPos(EEquipPos const ePos) const;
		void SetHideCashInvenPos(EEquipPos const ePos, bool const bHide);

		//! Equip Hide Option
		bool IsHideEquipInvenPos(EEquipPos const ePos) const;
		void SetHideEquipInvenPos(EEquipPos const ePos, bool const bHide);
	};

	void UpdateOldAndNew(PgActor* pkActor, SClientDWORDOption const& rkOldOption, SClientDWORDOption const& rkNewOption);
	SClientDWORDOption OptionToClientOption(PgOption const& rkOption);
	void ClientOptionToOption(SClientDWORDOption const& rkClientOption, PgOption& rkOut);
};

namespace lwOption
{
	void RegisterWrapper(lua_State* pkState);
}


//
lwGlobalOption GetGlobalOption();

#endif // FREEDOM_DRAGONICA_OPTION_PGOPTION_H