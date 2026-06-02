#include "StdAfx.h"
#include "PgRoom.h"
#include "PgWall.h"

PgRoom::PgRoom(NiPoint3 const &rkLeft, NiPoint3 const &rkRight, NiPoint3 const &rkTop)
{
	m_kLeft = rkLeft;
	m_kRight = rkRight;
	m_kTop = rkTop;
	BuildWalls();
}

PgRoom::~PgRoom(void)
{
}

void PgRoom::BuildWalls()
{
	NiPoint3 kA, kB;

	// ¹Ù´Ú
	kA = m_kLeft;
	kB = m_kRight;
	m_kWallContainer.push_back(NiNew PgWall(kA, kB));
	
	// ¿Şº®
	kA = m_kLeft;
	kB = m_kTop;
	kB.x = kA.x;
	m_kWallContainer.push_back(NiNew PgWall(kA, kB));

	// ¿ìº®
	kB = m_kTop;
	kA = m_kLeft;
	kA.x = kB.x;
	m_kWallContainer.push_back(NiNew PgWall(kA, kB));

	// µŞº®
	kB = m_kTop;
	kA = m_kLeft;
	kA.y = kB.y;
	m_kWallContainer.push_back(NiNew PgWall(kA, kB));

	// ÃµÀå
	kB = m_kTop;
	kA = m_kLeft;
	kA.z = kB.z;
	m_kWallContainer.push_back(NiNew PgWall(kA, kB));
}

bool PgRoom::IsInside(NiPoint3 const &rkPt)
{
	for(WallContainer::iterator itr = m_kWallContainer.begin();
		itr != m_kWallContainer.end();
		itr++)
	{
		if((*itr)->IsInside(rkPt))
		{
			return true;
		}
	}

	return false;
}