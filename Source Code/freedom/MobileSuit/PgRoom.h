#pragma once

class PgWall;

class PgRoom : public NiMemObject
{
	typedef std::list<PgWall *> WallContainer;

public:
	// 생성자
	PgRoom(NiPoint3 const &rkLeft, NiPoint3 const &rkRight, NiPoint3 const &rkTop);
	// 소멸자
	~PgRoom();

	// 벽들을 구성한다.
	void BuildWalls();

	// 점이 방안(벽)에 있는지 검사한다.
	bool IsInside(NiPoint3 const &rkPt);

protected:
	WallContainer m_kWallContainer;

	NiPoint3 m_kLeft;
	NiPoint3 m_kRight;
	NiPoint3 m_kTop;
};