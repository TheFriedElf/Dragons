#ifndef DRAGONICA_PGSCREENEFFECT_H
#define DRAGONICA_PGSCREENEFFECT_H
#include <string>
#include <map>
#include <list>

//상태이상Effect를 사용하여 화면에 텍스쳐를 출력

namespace ScreenEffect
{

class PgScreenEffect
{
public:
	PgScreenEffect()	{ Init(); }
	virtual ~PgScreenEffect()	{ Destroy(); }

	virtual void Update(float kAccumTime, float kFrameTime);
	virtual void Draw(PgRenderer* pRenderer);
	virtual void AddNewScreenEffect(std::string const& TexturePath, float const ScreenX, 
		float const ScreenY, int const EffectKeyNo, int const LifeTime, int const AlphaTime = 0);
	virtual void DeleteEffectByNo(int const EffectKeyNo);
	virtual void ClearAll();

private:
	void	Init();
	void	Destroy();
	void	UpdateRectIndex();

	typedef struct stScreenEffectNode
	{
		std::string			m_kTextureName;
		int					m_kRectIndex;
		unsigned	long	m_kCreateTime;
		unsigned	long	m_kEndTime;
		unsigned	long	m_kAlphaTime;
		int					m_kEffectID;
		bool				m_kHaveToDelete;	

		stScreenEffectNode():m_kRectIndex(0), m_kCreateTime(0), m_kEndTime(0),
			m_kAlphaTime(0), m_kEffectID(0), m_kHaveToDelete(false)
		{}
	}ScreenEffectNode_;

	typedef std::list<ScreenEffectNode_*> ContNode;
	ContNode		m_kContNode;

	typedef std::map<std::string, NiScreenTexturePtr> ContTexture;
	ContTexture		m_kContTexture;

	float m_kAccumTimeSaved;	//업데이트용 누적시간 저장
	Loki::Mutex m_kMutex;

};//PgScreenEffect

} //ScreenEffect

#define g_kScreenEffect SINGLETON_CUSTOM(ScreenEffect::PgScreenEffect, CreateUsingNiNew)

#endif	// DRAGONICA_PGSCREENEFFECT_H