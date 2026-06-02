#ifndef FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PGZONEDRAWING_H
#define FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PGZONEDRAWING_H


#ifndef USE_INB
#include <NiMain.H>
#include <list>

class	PgZoneDrawing	
{
	friend	class	PgMobileSuit;
private:
	POINT3I m_kMinPos;		//맵 바운딩 박스 최소점
	POINT3I m_kMaxPos;		//맵 바운딩 박스 최대점
	POINT3I m_kZoneSize;	//존 사이즈(가로, 세로, 높이)
	POINT3I m_kZoneCount;	//가로 세로 높이 존 갯수

	//존을 그리기 위한 리소스
	NiPoint3*			m_pkZoneVertexArray;	
	NiMaterialProperty*	m_pkMaterial;
	NiLinesPtr			m_pkZoneLine;	
	NiBool*				m_pkConnect;

	bool m_bZoneDraw;
	bool m_bZoneWholeDraw;

	// 
	
public:
	PgZoneDrawing();
	virtual	~PgZoneDrawing();

	void Init(); //존을 그릴 리소스에 할당을 하고 가로세로높이 1의 정육면체 생성, 엔진 초기화 완료 후 호출해야 함.
	void Destroy(); //해제

	void SetDraw(bool bDraw) { m_bZoneDraw = bDraw; }
	void SetWholeDraw(bool bW) { m_bZoneWholeDraw = bW; }

	POINT3I const& GetMinPos() { return m_kMinPos; }
	POINT3I const& GetMaxPos() { return m_kMaxPos; }
	POINT3I const& GetZoneSize() { return m_kZoneSize; }
	void SetZoneInfo(POINT3I const& kMinPos, POINT3I const& kMaxPos, POINT3I const& kZoneSize, POINT3I const& kZoneCount);

	//존을 그린다.
	void DrawImmediate(PgRenderer *pkRenderer, NiCamera *pkCamera, float fFrameTime);
};

extern	PgZoneDrawing	g_kZoneDrawer;

#endif//USE_INB

#endif // FREEDOM_DRAGONICA_SCRIPTING_WORLDOBEJCT_PGZONEDRAWING_H