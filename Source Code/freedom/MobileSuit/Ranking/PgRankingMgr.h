#ifndef FREEDOM_DRAGONICA_RANKING_PGRANKINGMGR_H
#define FREEDOM_DRAGONICA_RANKING_PGRANKINGMGR_H

#include	<wininet.h>
#pragma	comment(lib, "wininet.lib")

#include "../CreateUsingNiNew.inl"

typedef enum
{
	RANKTYPE_RANK_PVP						= 1002,// PvP 순위
	RANKTYPE_RANK_MAXCOMBO					= 1003,// 최대콤보 순위
	RANKTYPE_RANK_LEVELUP_20				= 1004,// 20레벨업 순위
	RANKTYPE_RANK_MONEY						= 1005,// 부자 순위
	RANKTYPE_RANK_FOODMAKE					= 1006,// 음식을 많이 만든 순위
//	RANKTYPE_RANK_CRYSTALSTONE				= 1007,// 크리스탈스톤획득이 높은 순위
	RANKTYPE_RANK_RECVMAIL					= 1008,// 메일을 받은 순위
	RANKTYPE_RANK_ENCHANT_SUCCEED			= 1009,// Enchant 성공 순위
	RANKTYPE_RaNK_ENCHANT_FAILED			= 1010,// Enchant 실패 순위
	RANKTYPE_RANK_LEVELUP_40				= 1011,// 40레벨업 순위
	RANKTYPE_RANK_LEVELUP_60				= 1012,// 60레벨업 순위
	RANKTYPE_RANK_LEVELUP_80				= 1013,// 80레벨업 순위
	RANKTYPE_RANK_EXP						= 1014,// 경험치(레벨)순위
	RANKTYPE_RANK_LEVELUP_100				= 1015,// 100레벨업 순위
	RANKTYPE_RANK_GUILD_USERCOUNT			= 1018,// 길드 인원수 순위
	RANKTYPE_RANK_GUILD_LEVEL				= 1019,// 길드 레벨 순위
	RANKTYPE_RANK_GUILD_EXP					= 1020,// 길드 경험치 순위
	RANKTYPE_RANK_USEPHOENIXFEATHER_COUNT	= 1021,// 불사조깃털 사용 횟수 순위
}E_RANKTYPE;

typedef enum
{
	RANKFILTER_CLASS_NONE			= 0,
	RANKFILTER_CLASS_ONE			= 1,
	RANKFILTER_CLASS_PARTY			= 2,
}E_RANK_FILTER_CLASS;

typedef struct tagRankData
{
	tagRankData()
		:	usRank(0)
		,	usLevel(0)
		,	iValue(0)
		,	usClass(0)
	{}

	size_t min_size()const
	{
		return 
			sizeof(usRank)+
			sizeof(kCharGuid)+
			sizeof(size_t)+//wstrName)+
			sizeof(size_t)+//wstrGuildName)+
			sizeof(usLevel)+
			sizeof(iValue)+
			sizeof(usClass);
	}

	void WriteToPacket( BM::Stream &kPacket )const
	{
		kPacket.Push(usRank);
		kPacket.Push(kCharGuid);
		kPacket.Push(wstrName);
		kPacket.Push(wstrGuildName);
		kPacket.Push(usLevel);
		kPacket.Push(iValue);
		kPacket.Push(usClass);
	}

	void ReadFromPacket( BM::Stream &kPacket )
	{
		kPacket.Pop(usRank);
		kPacket.Pop(kCharGuid);
		kPacket.Pop(wstrName);
		kPacket.Pop(wstrGuildName);
		kPacket.Pop(usLevel);
		kPacket.Pop(iValue);
		kPacket.Pop(usClass);
	}

	WORD			usRank;
	BM::GUID		kCharGuid;
	std::wstring	wstrName;
	std::wstring	wstrGuildName;
	WORD			usLevel;
	__int64			iValue;
	WORD			usClass;
}SRankData;

//Find
struct SRankData_Find
{
	SRankData_Find( int &_iCount )
		:	iCount(_iCount)
	{}

	int &iCount;
};

struct SRankData_FindGUID
	:	public SRankData_Find
{
	SRankData_FindGUID( BM::GUID const &_kCharGuid, int &_iCount )
		:	kCharGuid(_kCharGuid)
		,	SRankData_Find(_iCount)
	{}

	bool operator()( SRankData const &rhs )
	{
		++iCount;
		return kCharGuid == rhs.kCharGuid;
	}

	BM::GUID const &kCharGuid;
};

struct SRankData_FindName
	:	public SRankData_Find
{
	SRankData_FindName( std::wstring const &_wstrName, int &_iCount )
		:	wstrName(_wstrName)
		,	SRankData_Find(_iCount)
	{}

	bool operator()( SRankData const &rhs )
	{
		++iCount;
		return wstrName == rhs.wstrName;
	}

	std::wstring const &wstrName;
};

typedef struct tagRakingKey
{
	tagRakingKey( int const _iRealmNo=0, int const _iType=0, int const _iClass=0 )
		:	iRealmNo(_iRealmNo)
		,	iType(_iType)
		,	iClass(_iClass)
	{}

	bool operator<( tagRakingKey const &rhs )const
	{
		if ( iRealmNo != rhs.iRealmNo ){return iRealmNo < rhs.iRealmNo;}
		if ( iType == rhs.iType ){return iClass < rhs.iClass;}
		return iType < rhs.iType;
	}

	int		iRealmNo;
	int		iType;
	int		iClass;
}SRankingKey;

typedef struct tagRankingKeyPage
{
	tagRankingKeyPage()
		:	iPage(0)
	{}

	tagRankingKeyPage( tagRakingKey const &_kKey, int const _iPage=0 )
		:	kKey(_kKey)
		,	iPage(_iPage)
	{}

	SRankingKey		kKey;
	int				iPage;
}SRankingKeyPage;

class PgRankingMgr
{
public:
	static const int ms_iItemCountForPage	= 18;

	typedef std::vector<SRankData>					VEC_RANKDATA;
	typedef std::map< SRankingKey,VEC_RANKDATA >	CONT_RANKDATA;

public:
	PgRankingMgr();
	virtual ~PgRankingMgr();

	static TCHAR ms_szURL[MAX_PATH];
	static void RegisterWrapper(lua_State *pkState);
	static E_RANK_FILTER_CLASS GetFilterClass( int const iType );

	void Clear();
	bool Init();
	bool Call( CONT_RANKDATA::key_type &kKey );
	bool RefreshRanking( int iPage=0 );

	HRESULT FindRank( BM::GUID const &kCharGuid );
	HRESULT FindRank( std::wstring& wstrCharacterName );
	
protected:

	bool IsInit()const{return m_iNextUpdateDate>0;}
	bool Call( XUI::CXUI_Wnd* pkWnd, SRankingKeyPage &kKeyPage );
	HRESULT GetHttpData( CONT_RANKDATA::key_type const &kKey, CONT_RANKDATA::mapped_type &kCont );
	bool Parse( TiXmlElement const *pkElement, CONT_RANKDATA::mapped_type::value_type &kData );
	void RefreshNextUpdate( __int64 i64NowTime );
	bool Check( SRankingKey& kKey );

	bool LoadDump();
	bool Dump();

	CLASS_DECLARATION_S(int, RankType);

protected:
	CONT_RANKDATA	m_kContData;
	__int64			m_iLastUpdateDate;
	__int64			m_iNextUpdateDate;
};

#define g_kRanking SINGLETON_CUSTOM(PgRankingMgr, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_RANKING_PGRANKINGMGR_H