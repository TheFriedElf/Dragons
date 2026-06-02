#ifndef FREEDOM_DRAGONICA_UI_PGWORLDMAPUI_H
#define FREEDOM_DRAGONICA_UI_PGWORLDMAPUI_H

#include "lwUI.h"

typedef struct tagWorldMapBGForm
{
	tagWorldMapBGForm()
	{
		ptPos.x = 0;
		ptPos.y = 0;
		ptPos.z = 0;
		ptSize.x = 0;
		ptSize.y = 0;
		wstrImg.clear();
		wstrOverImg.clear();
		pkWnd = 0;
	}
	~tagWorldMapBGForm()
	{
		pkWnd = 0;
	}
	std::vector<std::string> kVecId;
	std::wstring wstrImg;
	std::wstring wstrOverImg;	
	POINT3 ptPos;
	POINT ptSize;
	XUI::CXUI_Wnd* pkWnd;
} SWorldMapBGForm;
typedef std::map< std::wstring, SWorldMapBGForm >		CONT_WM_BGFORM;

typedef struct tagWorldMapShard : public tagWorldMapBGForm
{
	typedef enum tag_eShardType
	{
		EShardType_None,
		EShardType_Field,
		EShardType_Town,
		EShardType_Dungeon,
	} EShardType;
	tagWorldMapShard()
	{
		tagWorldMapBGForm();
		wstrChild.clear();
		eShardType = EShardType_None;
		iLocationTTW = 0;
	}
	std::wstring wstrNameDefString;
	std::wstring wstrChild;
	EShardType eShardType;
	int iLocationTTW;			// 대게 현재 이 Shard의 위치를 나타낸다. EX) 잉그리드, 혼돈의 땅
} SWorldMapShard;
typedef std::map< std::wstring, SWorldMapShard >		CONT_WM_SHARD;

typedef struct tagWorldMapFont
{
	tagWorldMapFont()
	{
		ptPos.z = ptPos.y = ptPos.x = 0;
		iTTW = 0;
	}
	POINT3 ptPos;
	int iTTW;
} SWorldMapFont;
typedef std::map< std::wstring, SWorldMapFont >			CONT_WM_FONT;

typedef struct tagWorldMap
{
	tagWorldMap()
	{
		wstrType.clear();
		wstrParent.clear();
	}

	std::wstring wstrID;
	std::wstring wstrType;
	std::wstring wstrParent;
	CONT_WM_SHARD kShardCont;
	CONT_WM_BGFORM kBGFormCont;
	CONT_WM_FONT kFontCont;
} SWorldMap;

typedef std::map< std::wstring, SWorldMap >				WorldMapCont;
typedef std::map< std::wstring, XUI::CXUI_Form* >		XUIFormCont;

typedef std::vector< SNpcCreateInfo >					MapNpcSet;
typedef std::map< std::wstring, MapNpcSet >				MapNpcCont;		// XMLPath / NPC Name
typedef std::map<std::wstring, NiSourceTexture*>		WorldMapTextureCont;
typedef std::map< std::wstring, std::wstring >			MiniMapPathCont;// XMLPath / Path
typedef std::map<std::wstring, int>						MapDescriptionIDCont;// XMLPath / ID
typedef std::multimap<std::wstring, int>				NpcFromMapNo;	//NPC이름으로 맵번호 찾기
typedef std::multimap<BM::GUID, int>				NpcGuidToMapNo;	//NPC Guid로 맵번호 찾기

class PgWorldMapUI :
	public PgIXmlObject
{
public:

//Function
public:
	PgWorldMapUI();
	virtual ~PgWorldMapUI();
	void	Destroy();

	void OnBuild();

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
	bool ParseXml_Shard(const TiXmlNode *pkNode, void *pArg, SWorldMap& rkWorldMap);
	bool ParseXml_WorldNpc(const TiXmlNode *pkNode, char const *pkXMLPath, int iMapNo);
	bool ParseXml_WorldNpc(const TiXmlNode *pkNode, MapNpcSet& rkNpcSet, char const *pkXMLPath = NULL);

	bool Initialize(XUI::CXUI_Form* pkForm);
	void Terminate();
	void ReleaseControl();
	void RenderWorldMap();
	void PulseWorldMap();
	void InSideWorldMap();
	void OutSideWorldMap();
	void MouseClickWorldMap(const POINT2& pt);
	void MouseOverWorldMap(const POINT2& pt)const;
	static void AddIconToParent(lwUIWnd kParent, XUI::CXUI_Form* pkChildForm, std::wstring wstrID, const lwPoint3& ptPos, const POINT2& ptSize, std::string strImg);
	bool GetImgFromShardId(std::wstring& rkOutImg, std::wstring wstrID)const;
	bool GetShardFromId(SWorldMapShard& rkOutShard, std::wstring wstrID)const;

	// World NPC
	bool GetNpcSet(int iMapNo, MapNpcSet& rkNpcSet)const;
	bool GetNpcSet(std::wstring kXMLPath, MapNpcSet& rkNpcSet)const;
	bool GetNpc(BM::GUID const& rkGuid, SNpcCreateInfo& kInfo) const;
	bool GetMiniMapPath(int iMapNo, std::wstring& rkPath )const;
	bool GetMapDiscriptionID(int iMapNo, std::wstring& kStr, std::wstring const& rkPath=L"")const;
	int GetAlphaFromImg(char const* szPath, int iX, int iY);
	int const GetNpcMapNo(std::wstring const& rkName) const;
	bool const GetNpcGuidToMapNo(BM::GUID const& rkGuid, std::vector< int >& rkMapNoArray) const;

protected:
	void RefreshForm(SWorldMap& rkWorldMap);
	bool ChangeWorldMap(std::wstring wstrID);
	bool FindCurShardFromMapNo(SWorldMapShard& rkRetShard, int iMapNo, std::wstring wstrCurWorldID);

// Variable
public:
	// WorldMapNo
	CLASS_DECLARATION_S(int, PlayerWorldMapNo);
	CLASS_DECLARATION_S(int, ClickedMapNo);

protected:
	// 메인.
	XUI::CXUI_Form *m_pkForm;
	XUI::CXUI_Wnd* m_pkSubForm;

	// 데이터.
	std::wstring		m_wstrCurrentWorldID;
	WorldMapCont		m_kWorldMapCont;
	// NPC 데이터
	MapNpcCont			m_kMapNpcCont;
	// 미니맵 데이터
	MiniMapPathCont		m_kMiniMapPath;
	// 맵 설명용 DefScrtip
	MapDescriptionIDCont	m_kMapDescriptionIDCont;

	// 기존 UI를 지우지 않고 재활용을 하기 위해 저장 해놓는 Wnd
	XUIFormCont	m_kUICont;

	// Target의 좌표.
	// TODO : 파티원 등 다른 정보도 표시 해줘야 함.
	lwPoint3 m_kTargetOrgPt;
	lwPoint3 m_kTargetDeltaPt;
	bool m_bTargetMoveUp;
	XUI::CXUI_Form* m_pkTargetForm;

	//! 클릭한 맵 넘버
	int m_iClickedMapNo;

	//! 현 좌표 표시 해주는 화살표 조작용.
	float m_fArrowStartAccumTime;
	float m_fArrowAccumTime;

	WorldMapTextureCont	m_kWorldMapTextureCont;
	Loki::Mutex m_kNpcLock;

	NpcFromMapNo		m_kNpcFromMapNo;
	NpcGuidToMapNo		m_kNpcGuidToMapNo;

	static void ConvertSNpcCreateInfoToSWorldNpc(SWorldNpc const& rkWorldNpc, SNpcCreateInfo& rkCreateInfo);
};

#define g_kWorldMapUI SINGLETON_CUSTOM(PgWorldMapUI, CreateUsingNiNew)

extern bool ParseNpcName(const wchar_t *pName, std::wstring& rkOut);

#endif // FREEDOM_DRAGONICA_UI_PGWORLDMAPUI_H