#ifndef FREEDOM_DRAGONICA_UI_PGUISOUND_H
#define FREEDOM_DRAGONICA_UI_PGUISOUND_H
#include <string>
#include "niaudiosource.h"
#include "lwGuid.h"
#include "XUI/XUI_MediaPlayer_Base.h"
#include "Loki/Singleton.h"

class PgUISound
	:	public XUI::CXUI_MediaPlayer_Base
{
	friend struct Loki::CreateUsingNew< PgUISound >;
public:
	PgUISound();
	virtual ~PgUISound();

	static bool RegisterWrapper(lua_State *pkState);

	virtual bool PlaySound(std::wstring const &wstrFileName);
	virtual bool PlaySoundByID(std::wstring const &wstrFileName);

	//NiAudioSourcePtr CreateSrc(std::wstring const &rkFileName);
	//NiAudioSourcePtr GetSrc(std::wstring const &rkFileName);

	void Update(float fAccumTime);

	void	Destroy();
protected:
	typedef std::map< std::wstring, NiAudioSourcePtr > SoundContainer;	// leesg213 2006-11-24, NiAudioSource* -->NiAudioSourcePtr 로 바꿈(스마트 포인터 사용)

	SoundContainer m_kSoundContainer;

	typedef std::list< NiAudioSourcePtr > SOUND_LIST;
	SOUND_LIST m_klstPlaying;
};

extern bool lwPlaySound(char *szFileName);
extern bool lwPlaySoundByID( char *szID );
extern bool lwStopSoundByID( char* szID );

#define g_kUISound SINGLETON_CUSTOM(PgUISound, Loki::CreateUsingNew)
#endif // FREEDOM_DRAGONICA_UI_PGUISOUND_H