#pragma once

class PgEffect;

class PgEffectMan
{
	typedef std::map<BM::GUID, PgEffect *> Container;

public:
	PgEffectMan(void);
	~PgEffectMan(void);

	//! 새 이펙트 인스턴스를 생성한다.
	PgEffect *GetEffect(BM::GUID &rkGuid);

protected:
	Container m_kContainer;
};
