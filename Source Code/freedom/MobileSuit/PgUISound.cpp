#include "stdafx.h"
#include "PgUISound.h"
#include "PgSoundMan.h"
#include "PgWorkerThread.h"

PgUISound::PgUISound()
{
	m_klstPlaying.clear();
}

PgUISound::~PgUISound()
{
}

void	PgUISound::Destroy()
{
{
	SOUND_LIST::iterator itor = m_klstPlaying.begin();
	while( itor != m_klstPlaying.end())
	{
		(*itor)->Stop();
		itor = m_klstPlaying.erase(itor);
	}
}
{
	SoundContainer::iterator itor = m_kSoundContainer.begin();
	while(itor != m_kSoundContainer.end())
	{
		itor->second = 0;
		itor = m_kSoundContainer.erase(itor);
	}
}
}

bool PgUISound::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	
	def(pkState, "PlaySound", &lwPlaySound);
	def(pkState, "PlaySoundByID", &lwPlaySoundByID);
	def(pkState, "StopSoundByID", &lwStopSoundByID);
	return true;
}
//
//NiAudioSourcePtr PgUISound::GetSrc(std::wstring const &rkFileName)
//{
//	SoundContainer::iterator itr = m_kSoundContainer.find(rkFileName);
//	
//	if(itr != m_kSoundContainer.end())
//	{
//		return (*itr).second;
//	}
//	return 0;
//}
//
//NiAudioSourcePtr PgUISound::CreateSrc(std::wstring const &rkFileName)
//{
//	NiAudioSourcePtr pkAudioSource = GetSrc(rkFileName);
//
//	if(!pkAudioSource)
//	{
//		NiAudioSourcePtr pkAudioSource = g_kSoundMan.LoadAudioSource(NiAudioSource::TYPE_3D, MB(rkFileName), 1.0f, 100, 100);
//				
//		if(pkAudioSource)
//		{
//			pkAudioSource->SetLoopCount(NiAudioSource::LOOP_SINGLE);
//			std::pair<SoundContainer::iterator, bool> ret = m_kSoundContainer.insert(std::make_pair(rkFileName, pkAudioSource));
//			return pkAudioSource;
//		}
//	}
//	return pkAudioSource;
//}

bool PgUISound::PlaySound(std::wstring const &wstrFileName)
{
	g_kSoundMan.PlayAudioSourceByPath(NiAudioSource::TYPE_AMBIENT, MB(wstrFileName), 0.0f);
	return true;
}

bool PgUISound::PlaySoundByID(std::wstring const &wstrFileName)
{
	g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_AMBIENT, MB(wstrFileName), 0.0f);

	return true;
}

bool lwPlaySound(char *szFileName)
{
	std::wstring const wstrFileName = UNI(szFileName);
	return g_kUISound.PlaySound(wstrFileName);
}

bool lwPlaySoundByID( char *szID )
{
	std::wstring const wstrID = UNI(szID);
	return g_kUISound.PlaySoundByID( wstrID );
}
bool lwStopSoundByID( char* szID )
{
	std::wstring const wstrID = UNI(szID);
	g_kSoundMan.ResetAudioByID(MB(wstrID));	
	return true;
}

void PgUISound::Update(float fAccumTime)
{
	SOUND_LIST::iterator itor = m_klstPlaying.begin();
	while( itor != m_klstPlaying.end())
	{
		(*itor)->Update(fAccumTime);

		if((*itor)->GetStatus() == NiAudioSource::DONE)
		{
			NiAudioSourcePtr sptmp = (*itor);
			itor = m_klstPlaying.erase(itor);
			THREAD_DELETE_OBJECT((NiAVObject*)sptmp);
			continue;
		}
		++itor;
	}
}