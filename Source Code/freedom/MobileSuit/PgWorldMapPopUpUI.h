#ifndef FREEDOM_DRAGONICA_UI_PGWORLDMAPPOPUPUI_H
#define FREEDOM_DRAGONICA_UI_PGWORLDMAPPOPUPUI_H

#include "lwUI.h"
#include "PgWorldMapUI.h"
#include "PgResourceIcon.h"

typedef std::set< std::wstring >		CONT_SET_WSTRING;
typedef std::list<CONT_SET_DATA> CONT_LIST_SET;

typedef struct tagPopupInfo 
{
	void Clear()
	{
		iMapNo = 0;
		kSetData.clear();
		kSetElite.clear();
		for (int i = 0; i<MAX_MISSION_LEVEL; ++i)
		{
			kSetMission[i].clear();
		}
	}
	tagPopupInfo()
	{
		Clear();
	}
	int iMapNo;
	CONT_SET_DATA kSetData;	//마을일 때는 NPC, 일반 필드일 때는 필드몹
	CONT_SET_DATA kSetElite;	//정예만 모으자
	CONT_SET_DATA kSetMission[MAX_MISSION_LEVEL];//미션 난이도 별로 몬스터 모으자
	
} SPopupInfo;

typedef std::map<int, SPopupInfo>	CONT_MAP_POPUP_INFO;
class PgWorldMapPopUpUI : public NiRefObject
{
public:
	typedef enum eXuiType
	{
		XT_ICON = 0,
		XT_FORM = 1,
	} EXuiType;

public:
	PgWorldMapPopUpUI(void);
	virtual ~PgWorldMapPopUpUI(void);

	bool Initialize(XUI::CXUI_Form* pkForm, lwUIWnd pkIconState, lwUIWnd pkFormState);
	void Terminate();
	void ReleaseControl();
	void MouseClickWorldMapPU(const POINT2& pt);
	void MouseOverWorldMapPU(const POINT2& pt);
	static bool FindNPCFromActorName(SResourceIcon& rkRscIcon, SIconImage& rkIconImage, std::wstring const &rkActorName);
	static bool FindMonsterFromXMLPath(SResourceIcon& rkRscIcon, SIconImage& rkIconImage, std::wstring const &rkXMLPath);
	static void GetMonsterList(SMonsterControl const& rkGetMon, CMonsterDefMgr const& rkMonsterDefMgr, SPopupInfo& rkPopInfo);

protected:
	lwUIWnd NewXuiForm(	EXuiType const eType,
						XUIFormCont& rkFormCont,
						XUIFormCont::iterator &frm_itr,
						std::wstring wstrID,
						lwPoint3 const& ptPos,
						lwPoint2& ptSize,
						std::string strImg,
						SUVInfo const& rkUVInfo = SUVInfo(1,1,1),
						bool bIncreaseFrmItr = true);

	bool AddIcon(
				 const SResourceIcon& rkRscIcon,
				 const SIconImage& rkIconImage,
				 int iCount,
				 std::wstring wstrIconName,
				 XUIFormCont& rkIconCont,
				 XUIFormCont::iterator &icon_itr,
				 char* szControlName,
				 int iNo,
				 POINT3 kDefaultPos,
				 POINT2 kGab,
				 int iMaxX );

	void RefreshForm(int iMapNo);
	void RefreshForm_Field();
	void RefreshForm_Town();
	void RefreshForm_Dungeon();

protected:
	XUI::CXUI_Form* m_pkForm;
	lwUIWnd m_kIconState;
	lwUIWnd m_kFormState;
	lwUIWnd m_kSubForm;

	lwUIWnd m_kSubField;
	lwUIWnd m_kSubTown;

	XUIFormCont m_kUI_IconCont;
	XUIFormCont m_kUI_FormCont;

	CONT_DEFMAP const * m_pContDefMap;
	CONT_DEF_MISSION_CANDIDATE const * m_pContDefCanidate;
	CONT_DEF_MISSION_ROOT const * m_pContDefRoot;

	int m_iMapNo;

	CONT_MAP_POPUP_INFO	m_kContPopupInfo;
};
#endif //FREEDOM_DRAGONICA_UI_PGWORLDMAPPOPUPUI_H