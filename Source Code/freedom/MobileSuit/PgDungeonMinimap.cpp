#include "StdAfx.h"
#include "PgDungeonMinimap.h"
#include "PgWorld.h"
#include "PgTrigger.h"
#include "Pg2DString.h"
#include "lwUI.h"

const wchar_t* PgDungeonMinimap::m_wszTopWndID = L"SFRM_PROGRESS_MAP";
const wchar_t* PgDungeonMinimap::m_wszIconWndBaseID = L"IMG_ICON";
const wchar_t* PgDungeonMinimap::m_wszIconWndRunRightID = L"IMG_RUN_RIGHT";
const wchar_t* PgDungeonMinimap::m_wszIconWndRunLeftID = L"IMG_RUN_LEFT";
const wchar_t* PgDungeonMinimap::m_wszIconWndStarID = L"IMG_STAR";
const wchar_t* PgDungeonMinimap::m_wszLineWndID = L"IMG_WAY_LINE";
const int PgDungeonMinimap::m_iDungeonMinimapMargin = 10;
const std::wstring PgDungeonMinimap::wstrImgLine[6] =
	{L"../Data/6_ui/dungeon/dgctLine01.tga",
	L"../Data/6_ui/dungeon/dgctLine02.tga",
	L"../Data/6_ui/dungeon/dgctLine03.tga",
	L"../Data/6_ui/dungeon/dgctLine04.tga",
	L"../Data/6_ui/dungeon/dgctLine05.tga",
	L"../Data/6_ui/dungeon/dgctLine06.tga"};



PgDungeonMinimap::PgDungeonMinimap(void) : m_iCurPos(0), m_kContProgInfo(), m_kUIInfo(), m_kContProgWay()
{
}

PgDungeonMinimap::~PgDungeonMinimap(void)
{
}


void PgDungeonMinimap::Init(XUI::CXUI_Wnd* pkTopWnd, PROGRESS_MAP_UI_INFO const& kUIInfo)
{
	if(pkTopWnd == NULL) { return; }
	if(m_kContProgInfo.empty() == false)
	{
		CONT_PROGRESS_INFO kInit;
		m_kContProgInfo.swap(kInit);
	}

	//아이콘 윈도우 초기화
	int iMaxCount = 0;
	XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>( pkTopWnd->GetControl(_T("BLD_ICON")) );
	if(NULL != pkBuilder)
	{
		iMaxCount = pkBuilder->CountX();
	}

	BM::vstring wszIconWndID(_T(""));
	XUI::CXUI_Wnd* pkIconWnd = NULL;
	for(int i = 0; i < iMaxCount; i++)
	{
		wszIconWndID = m_wszIconWndBaseID;
		wszIconWndID += i;
		pkIconWnd = pkTopWnd->GetControl(wszIconWndID);
		if(pkIconWnd == NULL) { continue; }

		pkIconWnd->Visible(false);
	}


	//라인 윈도우 초기화
	pkBuilder = dynamic_cast<XUI::CXUI_Builder*>( pkTopWnd->GetControl(_T("BLD_WAY_LINE")) );
	iMaxCount = 0;
	if(NULL != pkBuilder)
	{
		iMaxCount = pkBuilder->CountX();
	}

	BM::vstring wszLineWndID(_T(""));
	XUI::CXUI_Wnd* pkLineWnd = NULL;
	for(int i = 0; i < iMaxCount; i++)
	{
		wszLineWndID = m_wszLineWndID;
		wszLineWndID += i;
		pkLineWnd = pkTopWnd->GetControl(wszLineWndID);
		if(pkLineWnd == NULL) { continue; }
		pkLineWnd->Visible(false);
	}


	m_kUIInfo = kUIInfo;
}

void PgDungeonMinimap::Cleanup(void)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(m_wszTopWndID);
	if(pkTopWnd == NULL)
	{
		pkTopWnd = XUIMgr.Call(m_wszTopWndID);
		if(pkTopWnd == NULL) { return; }
	}

	if(m_kContProgInfo.empty() == false)
	{
		CONT_PROGRESS_INFO kInit;
		m_kContProgInfo.swap(kInit);
	}

	if(m_kContProgWay.empty() == false)
	{
		CONT_PROGRESS_WAY kInit;
		m_kContProgWay.swap(kInit);
	}

	m_kUIInfo = PROGRESS_MAP_UI_INFO();
	pkTopWnd->Visible(false);
}


bool PgDungeonMinimap::AddProgressArea(size_t iIndex, PROGRESSINFO const& kElem)
{
	std::pair<CONT_PROGRESS_INFO::iterator, bool> kRet = m_kContProgInfo.insert(std::make_pair(iIndex, kElem));
	if(kRet.second == false)
	{
		return false;
	}

	if(m_kContProgInfo.size() == 1) //첫번째 원소인가?
	{
		m_iCurPos = iIndex;
		SetCurPos(iIndex, true);
	}

	XUI::CXUI_Wnd* pkTopWnd = GetMinimap();
	if(pkTopWnd != NULL)
	{
		UpdateIconObject(pkTopWnd, iIndex, kElem);
	}
	return true;
}

bool PgDungeonMinimap::AddProgressWay(PROGRESS_WAY_INFO const& kElem)
{
	if(kElem.iFrom == kElem.iTo) { return false; }

	m_kContProgWay.push_back(kElem);
	size_t iIndex = m_kContProgWay.size() - 1;

	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(m_wszTopWndID);
	if(pkTopWnd == NULL) { return false; }

	PROGRESSINFO* v1 = FindElem(kElem.iFrom);
	PROGRESSINFO* v2 = FindElem(kElem.iTo);
	if(v1 == NULL || v2 == NULL) { return false; }
	if(UpdateWayUI(pkTopWnd, iIndex, kElem, *v1, *v2) == false) { return false; }

	return true;
}


XUI::CXUI_Wnd* PgDungeonMinimap::GetMinimap(void)
{
	XUI::CXUI_Wnd* pkTopWnd = XUIMgr.Get(m_wszTopWndID);
	if(pkTopWnd == NULL)
	{
		pkTopWnd = XUIMgr.Call(m_wszTopWndID);
		if(pkTopWnd == NULL) { return NULL; }
		pkTopWnd->Visible(false);
	}
	return pkTopWnd;
}

bool PgDungeonMinimap::SetCurPos(size_t iPos, bool bInit)
{
	XUI::CXUI_Wnd* pkTopWnd = GetMinimap();
	if(pkTopWnd == NULL) { return false; }

	PROGRESSINFO* pkProgElem = NULL;
	if(iPos == m_iCurPos)
	{
		pkProgElem = FindElem(iPos);
		if(pkProgElem == NULL) { return false; }
		pkProgElem->kType |= PROGRESSINFO::ET_VISITED;
		pkProgElem->kType |= PROGRESSINFO::ET_CURRENT;
		if(false == UpdateIconObject(pkTopWnd, iPos, *pkProgElem)) { return false; }

		if(bInit == false) { return true; }
	}
	else
	{
		pkProgElem = FindElem(m_iCurPos);
		if(pkProgElem == NULL) { return false; } //이전 위치 아이콘 업데이트
		pkProgElem->kType ^= PROGRESSINFO::ET_CURRENT;
		if(false == UpdateIconObject(pkTopWnd, m_iCurPos, *pkProgElem)) { return false; }

		pkProgElem = FindElem(iPos);
		if(pkProgElem == NULL) { return false; }
		pkProgElem->kType |= PROGRESSINFO::ET_VISITED;
		pkProgElem->kType |= PROGRESSINFO::ET_CURRENT;
		if(false == UpdateIconObject(pkTopWnd, iPos, *pkProgElem)) { return false; }
		if(false == UpdateWay(pkTopWnd, iPos)) { return false; }
	}

	//현재 위치 아이콘 업데이트
	//좌,우 달리는 애니 아이콘 on/off
	XUI::CXUI_Wnd* pkRunCurrentDirIcon = NULL;
	XUI::CXUI_Wnd* pkRunOppositeDirIcon = NULL;
	XUI::CXUI_Wnd* pkStarIcon = pkTopWnd->GetControl(m_wszIconWndStarID);
	if(pkProgElem->eDir == PROGRESSINFO::ED_LEFT)
	{
		pkRunCurrentDirIcon = pkTopWnd->GetControl(m_wszIconWndRunLeftID);
		pkRunOppositeDirIcon = pkTopWnd->GetControl(m_wszIconWndRunRightID);
	}
	else
	{
		pkRunCurrentDirIcon = pkTopWnd->GetControl(m_wszIconWndRunRightID);
		pkRunOppositeDirIcon = pkTopWnd->GetControl(m_wszIconWndRunLeftID);
	}
	if(pkRunCurrentDirIcon == NULL || pkRunOppositeDirIcon == NULL || pkStarIcon == NULL) { return false; }

	//이동시 액션 연출 아이콘
	XUI::CXUI_Wnd* pkWave = pkTopWnd->GetControl(L"FRM_MOVE_ACTION_BIG");
	if(NULL == pkWave) { return false; }
	XUI::CXUI_Wnd* pkImgWaveBig = pkWave->GetControl(L"IMG_WAVE");
	if(NULL == pkImgWaveBig) { return false; }

	pkWave = pkTopWnd->GetControl(L"FRM_MOVE_ACTION_SMALL");
	if(NULL == pkWave) { return false; }
	XUI::CXUI_Wnd* pkImgWaveSmall = pkWave->GetControl(L"IMG_WAVE");
	if(NULL == pkImgWaveSmall) { return false; }


	if(true == (pkProgElem->kType & PROGRESSINFO::ET_NONE))
	{
		POINT2 ptLoc(pkProgElem->ix + m_iDungeonMinimapMargin, pkProgElem->iy + m_iDungeonMinimapMargin);
		pkRunCurrentDirIcon->Location(ptLoc);
		pkRunCurrentDirIcon->Visible(true);
		pkRunOppositeDirIcon->Visible(false);
		pkStarIcon->Visible(false);

		//이동시 액션 연출 아이콘
		ptLoc.x -= 9;
		ptLoc.y -= 9;
		pkImgWaveSmall->Visible(false);
		pkImgWaveBig->Visible(true);
		pkImgWaveBig->UVUpdate(1);
		pkImgWaveBig->Location(ptLoc);

	}
	else
	{
		POINT2 ptLoc(pkProgElem->ix + (m_iDungeonMinimapMargin - 5), pkProgElem->iy + (m_iDungeonMinimapMargin - 5));
		pkRunCurrentDirIcon->Visible(false);
		pkRunOppositeDirIcon->Visible(false);
		pkStarIcon->Visible(true);
		pkStarIcon->Location(ptLoc);

		//이동시 액션 연출 아이콘
		ptLoc.x += 7;
		ptLoc.y += 7;
		pkImgWaveBig->Visible(false);
		pkImgWaveSmall->Visible(true);
		pkImgWaveSmall->UVUpdate(1);
		pkImgWaveSmall->Location(ptLoc);

	}

	m_iCurPos = iPos;
	return true;
}

PROGRESSINFO* PgDungeonMinimap::FindElem(size_t iPos)
{
	CONT_PROGRESS_INFO::iterator itFind = m_kContProgInfo.find(iPos);
	if(itFind == m_kContProgInfo.end()) { return NULL; }

	return &itFind->second;
}

bool PgDungeonMinimap::UpdateIconObject(XUI::CXUI_Wnd* pkTopWnd, size_t iPos, PROGRESSINFO const& rkProgElem)
{ //오브젝트 상태에 따라 아이콘 업데이트
	if(pkTopWnd == NULL) { return false; }


	wchar_t wszNewIconWndID[256];
	wsprintfW(wszNewIconWndID, L"%s%d", m_wszIconWndBaseID, iPos);
	XUI::CXUI_Wnd* pkElemIcon = pkTopWnd->GetControl(wszNewIconWndID);
	if(pkElemIcon == NULL) { return false; } //아이콘 오브젝트를 찾을 수 없다면
	if(rkProgElem.kType & PROGRESSINFO::ET_NONE)
	{
		pkElemIcon->Visible(false);
		return true;
	}


	int iUVIndex = 0;
	if(rkProgElem.kType & PROGRESSINFO::ET_START)
	{
		iUVIndex = 7; //수정 필요
	}
	else if(rkProgElem.kType & PROGRESSINFO::ET_MAIN_ROUTE)
	{
		iUVIndex = 3; //수정 필요
	}
	else if(rkProgElem.kType & PROGRESSINFO::ET_SUB_ROUTE)
	{
		iUVIndex = 5; //수정 필요
	}
	else if(rkProgElem.kType & PROGRESSINFO::ET_BOSS)
	{
		iUVIndex = 1;
	}

	if(iUVIndex > 0)
	{
		if( (rkProgElem.kType & PROGRESSINFO::ET_CURRENT) == 0 && (rkProgElem.kType & PROGRESSINFO::ET_VISITED))
		{
			iUVIndex += 1;
		}
		pkElemIcon->Visible(true);
		pkElemIcon->Location(rkProgElem.ix + m_iDungeonMinimapMargin, rkProgElem.iy + m_iDungeonMinimapMargin);
		pkElemIcon->UVUpdate(iUVIndex);
	}
	else
	{
		pkElemIcon->Visible(false);
	}


	return true;
}

bool PgDungeonMinimap::UpdateWay(XUI::CXUI_Wnd* pkTopWnd, size_t iModifiedElem)
{
	if(pkTopWnd == NULL) { return false; }

	if(m_kContProgWay.empty() == true) { return true; }
	PROGRESS_WAY_INFO kFindInfo(m_iCurPos, iModifiedElem, PROGRESS_WAY_INFO::ET_NONE);
	CONT_PROGRESS_WAY::iterator itFindResult = std::find(m_kContProgWay.begin(), m_kContProgWay.end(), kFindInfo);
	if(itFindResult == m_kContProgWay.end()) { return false; }
	PROGRESSINFO *v1 = FindElem(itFindResult->iFrom);;
	PROGRESSINFO *v2 = FindElem(itFindResult->iTo);
	if(v1 == NULL || v2 == NULL) { return false; }

	if( ((v1->kType & PROGRESSINFO::ET_VISITED) || (v1->kType & PROGRESSINFO::ET_CURRENT) || (v1->kType & PROGRESSINFO::ET_START)) &&
		((v2->kType & PROGRESSINFO::ET_VISITED) || (v2->kType & PROGRESSINFO::ET_CURRENT) || (v2->kType & PROGRESSINFO::ET_START)) )
	{ //두 점 모두 방문을 했나?
		itFindResult->kType |= PROGRESS_WAY_INFO::ET_VISITED;
		size_t iCurIndex = itFindResult - m_kContProgWay.begin();
		if(false == UpdateWayUI(pkTopWnd, iCurIndex, *itFindResult, *v1, *v2)) { return false; }
	}
	return true;
}

bool PgDungeonMinimap::UpdateWayUI(XUI::CXUI_Wnd* pkTopWnd, size_t iWayNum, PROGRESS_WAY_INFO const& rkWayElem, PROGRESSINFO const& rkElemV1, PROGRESSINFO const& rkElemV2)
{
	if(pkTopWnd == NULL) { return false; }

	size_t const iHalfIconSize = 18;
	POINT2 pt1(rkElemV1.ix + m_iDungeonMinimapMargin + iHalfIconSize, rkElemV1.iy + m_iDungeonMinimapMargin + iHalfIconSize);
	POINT2 pt2(rkElemV2.ix + m_iDungeonMinimapMargin + iHalfIconSize, rkElemV2.iy + m_iDungeonMinimapMargin + iHalfIconSize);
	POINT2 v1, v2;
	wchar_t wszNewLineWndID[256];
	wsprintfW(wszNewLineWndID, L"%s%d", m_wszLineWndID, iWayNum);
	XUI::CXUI_Wnd* pkWayElem = pkTopWnd->GetControl(wszNewLineWndID);
	if(pkWayElem == NULL) { return false; }
	if(pkWayElem->Visible() == false)
	{
		pkWayElem->Visible(true);
	}

	size_t iUVIndex = 1;
//	if(rkWayElem.kType & PROGRESS_WAY_INFO::ET_MAIN_ROUTE) { iUVIndex = 0; }
//	else if(rkWayElem.kType & PROGRESS_WAY_INFO::ET_SUB_ROUTE) { iUVIndex = 2; }
//	else if(rkWayElem.kType & PROGRESS_WAY_INFO::ET_CUTTED_ROUTE) { iUVIndex = 4; }
//	if( !(rkWayElem.kType & PROGRESS_WAY_INFO::ET_CUTTED_ROUTE) && (rkWayElem.kType & PROGRESS_WAY_INFO::ET_VISITED) ) { iUVIndex += 1; }
	if(rkWayElem.kType & PROGRESS_WAY_INFO::ET_MAIN_ROUTE) { iUVIndex = 1; }
	else if(rkWayElem.kType & PROGRESS_WAY_INFO::ET_SUB_ROUTE) { iUVIndex = 3; }
	else if(rkWayElem.kType & PROGRESS_WAY_INFO::ET_CUTTED_ROUTE) { iUVIndex = 5; }
//	if( !(rkWayElem.kType & PROGRESS_WAY_INFO::ET_CUTTED_ROUTE) && (rkWayElem.kType & PROGRESS_WAY_INFO::ET_VISITED) ) { iUVIndex += 1; }
	pkWayElem->DefaultImgName(wstrImgLine[iUVIndex]);

	if(rkWayElem.kType & PROGRESS_WAY_INFO::ET_VISITED)
	{
		v1.x = pt1.x - 2;	v1.y = pt1.y - 2;
		v2.x = pt2.x - 2;	v2.y = pt2.y - 2;
	}
	else
	{
		v1.x = pt1.x - 1;	v1.y = pt1.y - 1;
		v2.x = pt2.x - 1;	v2.y = pt2.y - 1;
	}
	//사이즈 늘임
	double dwTemp = pow(double(v1.x - v2.x), 2) + pow(double(v1.y - v2.y), 2);
	double dwDist = sqrt(dwTemp);
	POINT2 ptSizeWnd = pkWayElem->Size();
	if(rkWayElem.kType & PROGRESS_WAY_INFO::ET_VISITED)
	{ //방문시 선의 두깨는 4px
		ptSizeWnd.x = 4;
		ptSizeWnd.y = 4;
	}
	else
	{ //미방문시 선의 두께는 2px
		ptSizeWnd.x = 2;
		ptSizeWnd.y = 2;
	}
	ptSizeWnd.x = static_cast<int>(dwDist) + ptSizeWnd.x;
	pkWayElem->Size(ptSizeWnd);
	pkWayElem->ImgSize(ptSizeWnd);


	{//각도를 구하고
		pkWayElem->RotationCenter(POINT2F(0.0f, 1.0f));
		if( v1.x == v2.x )
		{//두 점이 같은 수직선상에 있으면 90도 회전
			pkWayElem->RotationDeg(90);
			if( v1.y < v2.y )
			{
				pkWayElem->Location( v1 );
			}
			else
			{
				pkWayElem->Location( v2 );
			}
		}
		else if( v1.y == v2.y )
		{//두 점이 같은 수직선상에 있으면 회전할 필요 없다.
			pkWayElem->RotationDeg(0);
			if( v1.x < v2.x )
			{
				pkWayElem->Location( v1 );
			}
			else
			{
				pkWayElem->Location( v2 );
			}
		}
		else if( v1.x < v2.x )
		{
			float radian = NiACos( (float)(v2.x - v1.x) / dwDist );
			float degree = radian*(180/PI);
			if( v1.y > v2.y )
			{//L을 중심으로 R의 위치가 1사분면에 있을때
				pkWayElem->RotationDeg(-degree);
			}
			else
			{//4사분면에 잇을 때
				pkWayElem->RotationDeg(degree);
			}
			pkWayElem->Location( v1 );
		}
		else if( v2.x < v1.x )
		{
			float radian = NiACos( (float)(v1.x - v2.x) / dwDist );
			float degree = radian*(180/PI);
			if( v2.y > v1.y )
			{//R을 중심으로 L의 위치가 1사분면에 있을때
				pkWayElem->RotationDeg(-degree);
			}
			else
			{//4사분면에 잇을 때
				pkWayElem->RotationDeg(degree);
			}
			pkWayElem->Location( v2 );
		}
	}

	return true;
}

bool PgDungeonMinimap::SetMainUI(PROGRESS_MAP_UI_INFO const& kUIInfo, bool bShow)
{
	XUI::CXUI_Wnd* pkTopWnd = GetMinimap();
	if(pkTopWnd == NULL) { return false; }

	if(kUIInfo.iImgH == 0 && kUIInfo.iImgH == 0 && kUIInfo.wstrImgPath.empty() == true)
	{
		return false;
	}

	Init(pkTopWnd, kUIInfo);
	if(pkTopWnd->Visible() == false)
	{
		pkTopWnd->Visible(true);
	}

	POINT2 ptSize(kUIInfo.iImgW + (m_iDungeonMinimapMargin * 2), kUIInfo.iImgH + (m_iDungeonMinimapMargin * 2));
	POINT2 ptImgSize(kUIInfo.iImgW, kUIInfo.iImgH);
	pkTopWnd->Size(ptSize);

	//맵 이미지를 설정해주고 이미지 사이즈 조절
	XUI::CXUI_Wnd* pkMiniMapImg = pkTopWnd->GetControl(L"IMG_PROGRESS_MAP");
//	if(NULL == pkMiniMapImg)
//	{
//		pkMiniMap->Close();
//		return false;
//	}

	if(NULL != pkMiniMapImg)
	{
		pkMiniMapImg->DefaultImgName(kUIInfo.wstrImgPath);
		pkMiniMapImg->Size(ptImgSize);
		pkMiniMapImg->ImgSize(ptImgSize);
		pkMiniMapImg->Location(m_iDungeonMinimapMargin, m_iDungeonMinimapMargin);
	}

	//----변경된 UI창 사이즈에 따른 닫기 버튼 재배치---
	static const int iBtnMargin = 7;
	XUI::CXUI_Wnd* pkCloseBtn = pkTopWnd->GetControl(L"BTN_CLOSE");
	if(NULL != pkCloseBtn)
	{
		pkCloseBtn->Location(ptSize.x - (pkCloseBtn->Size().x + iBtnMargin), iBtnMargin);
	}

	//텍스트 위치도 재배치
	static const int iTextMargin = 5;
	XUI::CXUI_Wnd* pkText = pkTopWnd->GetControl(L"FRM_CLICK_DESCRIPTION");
	if(pkText)
	{
		XUI::CXUI_Style_String kStyleString = pkText->StyleText();
		POINT2 kTextSize(Pg2DString::CalculateOnlySize(kStyleString));
		pkText->Size(ptSize.x, kTextSize.y);
		pkText->Location(0, ptSize.y - kTextSize.y - iTextMargin);
		pkText->TextPos( POINT2(ptSize.x / 2, 0) );

	}

	//UI가 재배치 되었으므로 갱신해줘라
	pkTopWnd->Invalidate(true);

	if(kUIInfo.bShowNpcList == true)
	{
		CallNpcList(pkTopWnd);
	}

	return true;
}


bool PgDungeonMinimap::CallUI(bool bShowToggle) //bShowToggle: true이면 토글로 미니맵 켜고 끄기로 동작하며, false이면 무조건 켠다.
{
	if(m_kUIInfo.iImgH == 0 && m_kUIInfo.iImgH == 0 && m_kUIInfo.wstrImgPath.empty() == true)
	{
		return false;
	}

	if(bShowToggle == true)
	{
		XUI::CXUI_Wnd* pkMiniMap = XUIMgr.Get(L"SFRM_PROGRESS_MAP");
		if(NULL != pkMiniMap)
		{
			pkMiniMap->Close();
		}
		else
		{
			pkMiniMap = XUIMgr.Call(L"SFRM_PROGRESS_MAP");
		}
	}
	else
	{
		XUIMgr.Call(L"SFRM_PROGRESS_MAP");
	}

	return true;
}

