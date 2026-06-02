#include "StdAfx.h"
#include "Variant/Global.h"
#include "PgResourceMonitor.h"
#include "PgMobileSuit.h"
#include "PgOption.h"
#include "PgNiFile.h"
#include "PgPilotMan.h"
#include "PgSoundMan.h"

char const *szHeadSound = "SOUND";
char const *szSubBGM = "BGM_VOLUME";
char const *szSubEffect = "EFFECT_VOLUME";
float const fMaxVolume = 0.01f;

#define USE_SOUND_LOAD_THREAD

PgSoundMan::PgSoundMan()
:	m_bInitialized(false),
	m_fBGMVolumeWeight(1.0f),
	m_fEffectVolumeWeight(1.0f),
	m_bSilence(false)
{
}

PgSoundMan::~PgSoundMan()
{
	Destroy();
}


void PgSoundMan::Destroy()
{
	UnloadBgSound();
	//환경사운드 제거
	ReleaseEnvSound();

	m_AudioSrcListMap.clear();
	m_kSoundContainer.clear();
	m_kPathContainer.clear();

	// 사운드 시스템 고잉-다운
	NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if (pkSS && pkSS->GetHWnd())
	{
		NiAudioSource* pkSource = NULL;
		NiTListIterator kIter;

		pkSource = pkSS->GetFirstSource(kIter);
		while (pkSource != NULL)
		{
			if (pkSource->GetStatus() == NiAudioSource::PLAYING)
				pkSource->Stop();

			pkSource = pkSS->GetNextSource(kIter);
		}
		pkSS->Shutdown();
	}

	m_bInitialized = false;
}

void PgSoundMan::ResetAudioSource()
{
	m_kSoundLock.Lock();
	for(SoundContainer::iterator itr = m_kSoundContainer.begin();
		itr != m_kSoundContainer.end();
		++itr)
	{
		NiAudioSource *pkAudioSource = itr->second.m_spSource;
		if(pkAudioSource->GetStatus() == NiAudioSource::PLAYING)
		{
			pkAudioSource->Stop();
			itr->second = 0;
		}
	}
	m_kSoundContainer.clear();
	m_kSoundLock.Unlock();

	m_AudioSrcListMap.clear();	
	m_kPathContainer.clear();

	if(g_bUseSound)
	{
		ParseXml("sound.xml");
	}
}

void PgSoundMan::ResetAudioByID(std::string AudioID)
{
	if( AudioID.size() )
	{
		PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(AudioID, PgResourceMonitor::RESOURCE_TYPE_AUDIO));

		// Path Container에서 Path를 찾는다.
		UPR(AudioID);
		PathContainer::iterator itrPath = m_kPathContainer.find(AudioID.c_str());
		if(itrPath == m_kPathContainer.end())
		{
			return;
		}

		m_kSoundLock.Lock();
		NiAudioSource* pkAudioSource = NULL;
		SoundContainer::iterator iter = m_kSoundContainer.find(itrPath->second.m_strPath.c_str());
		if( iter != m_kSoundContainer.end() )
		{
			m_kSoundContainer.erase(iter);
		}
		m_kSoundLock.Unlock();

		AudioSrcListMap::iterator	itor = m_AudioSrcListMap.find(itrPath->second.m_strPath.c_str());
		if(itor != m_AudioSrcListMap.end())
		{
			AudioSrcList	*pkAudioSourceList = &(itor->second);
			//플레이 중이면 멈추고 짤라내야지
			AudioSrcList::iterator itor2 = pkAudioSourceList->begin();
			while( itor2 != pkAudioSourceList->end() )
			{
				NiAudioSource* pkAudioSource = *itor2;
				pkAudioSource->Stop();
				itor2 = pkAudioSourceList->erase(itor2);
			}

			if(pkAudioSourceList->size() == 0)
			{
				m_AudioSrcListMap.erase(itor);
			}
		}
	}
}

float PgSoundMan::getConfigVolume(char const *szSubkey)
{
	return (g_kGlobalOption.GetValue(szHeadSound, szSubkey)*fMaxVolume);
}

float PgSoundMan::GetConfigBGMVolume()
{
	return getConfigVolume(szSubBGM) * m_fBGMVolumeWeight;
}

float PgSoundMan::GetConfigEffectVolume()
{
	return getConfigVolume(szSubEffect) * m_fEffectVolumeWeight;
}

bool PgSoundMan::Initialize()
{
	NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	PG_ASSERT_LOG(pkSS);
	if (pkSS == NULL)
		return false;

	NiWindowRef hWnd;
    if (g_pkApp->GetFullscreen())
        hWnd = g_pkApp->GetWindowReference();
    else
        hWnd = g_pkApp->GetRenderWindowReference();

	pkSS->SetHWnd(hWnd);

	// 마일즈 오디오를 시작한다.
	if(!pkSS->Startup("../Miles"))
	{
		pkSS->SetHWnd(0);
		NILOG(PGLOG_ERROR, "couldn't initialize audio system(%s)\n", NiAudioSystem::GetAudioSystem()->GetLastError());
		return false;
	}

	pkSS->SetBestSpeakerTypeAvailable();
    pkSS->SetUnitsPerMeter(39.37f);	// 1 meter = 39.37 inch

	g_fBGMVolume = GetConfigBGMVolume();
	g_fEffectVolume = GetConfigEffectVolume();

	m_bInitialized = true;
	return true;
}

NiAudioSourcePtr PgSoundMan::CreateBgSound( char const *pcSndPath, float fVolume, int const iLoopCount )
{
	if (!isInit() || pcSndPath == NULL)
	{
		return NULL;
	}

	NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if(!pkSS)
	{
		return NULL;
	}

	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(pcSndPath, PgResourceMonitor::RESOURCE_TYPE_AUDIO));

	NiAudioSourcePtr spAudioSource = 0;
	std::string strPath = pcSndPath;
	UPR(strPath);

	{
		BM::CAutoMutex kLock(m_kSoundLock);
		SoundContainer::iterator itr = m_kSoundContainer.find(strPath.c_str());
		if( itr == m_kSoundContainer.end() )
		{
			// Cache에 없다면 소스를 만든다.
			NiAudioSource* pkAudioSource = spAudioSource = pkSS->CreateSource(NiAudioSource::TYPE_AMBIENT);
			if( !spAudioSource )
			{
				return NULL;
			}

			// 음원을 로드한다.
			spAudioSource->SetFilename(strPath.c_str());
			std::wstring wstrPackFile = _T("../BGM.DAT");
			std::wstring wstrFolder = _T("../BGM/");
			std::wstring wstrPath = UNI(pcSndPath);
			std::wstring::size_type pos = wstrPath.find(_T("../BGM/"));
			std::wstring wstrRelativePath;
			if (pos != std::wstring::npos)
			{
				wstrRelativePath = wstrPath.erase(pos, wstrFolder.size());
			}
			else
			{
				wstrRelativePath = wstrPath;
			}

			if (g_bUsePackData && PgNiFile::IsPackedFile(pcSndPath, wstrPackFile, (std::wstring)(_T("./"))+wstrRelativePath))
			{
				// 팩 로드.
				std::vector< char > data(0);
				if (!BM::PgDataPackManager::LoadFromPack(wstrPackFile, (std::wstring)(_T("./"))+wstrRelativePath, data))
				{
					NILOG(PGLOG_ERROR, "[PgSoundMan] LoadFromPack failed, %s failed\n", pcSndPath);
					return NULL;
				}

				NiMilesSource* pkMiles = NiDynamicCast(NiMilesSource, spAudioSource);
				//NiMilesSource* pkMiles = dynamic_cast<NiMilesSource*>(pkAudioSource);
				if(pkMiles && !pkMiles->Load(&data.at(0), data.size()))
				{
					return NULL;
				}
			}
			else
			{
				spAudioSource->SetStreamed(true);
				if(!spAudioSource->Load())
				{
					return NULL;
				}
			}

			std::pair<SoundContainer::iterator, bool> ret = m_kSoundContainer.insert(std::make_pair(strPath, stSoundInfo(spAudioSource)));
			if (ret.second == false)
			{	// ???
				NILOG(PGLOG_WARNING, "[PgSoundMan] %s sound inserted already\n", strPath.c_str());
			}

			// Cache의 것을 클론해서 새로운 음원을 만든다.
			spAudioSource = (NiAudioSource*)(ret.first->second.m_spSource->Clone());
		}
		else
		{
			// Cache의 것을 클론해서 새로운 음원을 만든다.
			spAudioSource = (NiAudioSource*)(itr->second.m_spSource->Clone());
		}
	}

	if ( spAudioSource )
	{
		if ( fVolume == 0.0f )
		{
			fVolume = spAudioSource->GetGain();
		}

		spAudioSource->SetGain(fVolume * g_fBGMVolume);
		spAudioSource->SetLoopCount(iLoopCount);

		if(spAudioSource->GetStatus() == NiAudioSource::PLAYING)
		{
			spAudioSource->Stop();
		}
		spAudioSource->Update(0.0f);
	}
	return spAudioSource;
}

bool PgSoundMan::LoadBgSound( char const *pcSndPath, float fVolume, int const iLoopCount, bool bDefault )
{

	NiAudioSourcePtr spAudioSource = CreateBgSound( pcSndPath, fVolume, iLoopCount );
	if ( spAudioSource )
	{
		if( bDefault )
		{
			BGSoundContainer kContTemp;
			kContTemp.swap(m_kBGSoundCont);
			m_kBGSoundCont.push_back( BgmInfo(spAudioSource, fVolume) );
			m_kBGSoundCont.insert(m_kBGSoundCont.end(), kContTemp.begin(), kContTemp.end());
		}
		else
		{
			m_kBGSoundCont.push_back( BgmInfo(spAudioSource, fVolume) );
		}
		return true;
	}
	return false;
}

void PgSoundMan::UnloadBgSound()
{
	StopBgSound();

#ifndef EXTERNAL_RELEASE
	BGSoundContainer::iterator bgsound_itr;
	for( bgsound_itr=m_kBGSoundCont.begin(); bgsound_itr!=m_kBGSoundCont.end(); ++bgsound_itr )
	{
		PG_ASSERT_LOG( (*bgsound_itr).m_spBgmSound->GetRefCount() == 1);
	}
#endif
	m_kBGSoundCont.clear();
}

bool PgSoundMan::PlayBgSound( char const *pcSndPath, float fVolume, int const iLoopCount )
{
	NiAudioSourcePtr spAudioSource = CreateBgSound( pcSndPath, fVolume, iLoopCount );
	if ( spAudioSource )
	{
		if (	m_kCurrentBGM.m_spBgmSound 
			&&	!::strcmp( spAudioSource->GetFilename(), m_kCurrentBGM.m_spBgmSound->GetFilename())
			)
		{
			return false;
		}

		StopBgSound(true);
		Play( BgmInfo(spAudioSource, fVolume) );
		return true;
	}
	return false;
}

void PgSoundMan::PlayBgSound( int iNo, bool bStopImmediate )
{
	if ( iNo >= 0 )
	{
		if ( iNo >= (int)m_kBGSoundCont.size() )
		{
			return;
		}

		if(m_bSilence)
		{
			return;
		}

		StopBgSound(bStopImmediate);
		Play( m_kBGSoundCont.at(iNo) );
	}
}

bool PgSoundMan::Play(BgmInfo const &kBGM)
{
	if (	kBGM.m_spBgmSound != NULL	)
	{
		float fTargetGain = kBGM.m_fVolume * g_fBGMVolume;
		kBGM.m_spBgmSound->SetGain(0.0f);
		kBGM.m_spBgmSound->Play();
		kBGM.m_spBgmSound->FadeToGain( fTargetGain, g_pkApp->GetAccumTime(), lua_tinker::call<float>("GetBgSoundFadeInTime") );
		m_kCurrentBGM = kBGM;
		return true;
	}
	return false;
}

bool PgSoundMan::StopBgSound( bool bStopImmediate )
{
	if ( m_kCurrentBGM.m_spBgmSound )
	{
		if ( bStopImmediate )
		{
			m_kCurrentBGM.m_spBgmSound->Stop();
		}
		else
		{
			m_kCurrentBGM.m_spBgmSound->FadeToGain(0.0f, g_pkApp->GetAccumTime(), lua_tinker::call<float>("GetBgSoundFadeOutTime"));
		}
		m_kCurrentBGM.Clear();
		return true;
	}
	return false;
}

NiAudioSource* PgSoundMan::LoadAudioSource(unsigned int uiType, char const *pcPath, float fVolume, float fDistMin, float fDistMax )
{
	if( !isInit() || pcPath == NULL)
	{
		if (pcPath)
		{
			NILOG(PGLOG_WARNING, "[PgSoundMan] %s audio source can't loaded, before Init.\n", pcPath);
		}
		return 0;
	}
	
	NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if(!pkSS)
	{
		return 0;
	}

	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(pcPath, PgResourceMonitor::RESOURCE_TYPE_AUDIO));
	
	NiAudioSource* pkAudioSource = 0;
	NiAudioSourcePtr spAudioOriginal = NULL;
	std::string strPath = pcPath;
	UPR(strPath);

	m_kSoundLock.Lock();
	SoundContainer::iterator itr = m_kSoundContainer.find(strPath);
	if(itr == m_kSoundContainer.end())
	{
		m_kSoundLock.Unlock();
		// Cache에 없다면 소스를 만든다.
		pkAudioSource = pkSS->CreateSource(uiType);
		if(!pkAudioSource)
		{
			return 0;
		}
		
		// 음원을 로드한다.
		pkAudioSource->SetFilename(strPath.c_str());
		if(uiType == NiAudioSource::TYPE_AMBIENT)
		{
			pkAudioSource->SetStreamed(true);
		}

		if(!pkAudioSource->Load())
		{
			return 0;
		}

		if(uiType == NiAudioSource::TYPE_3D)
		{
			pkAudioSource->SetGain(fVolume * g_fEffectVolume);
			pkAudioSource->SetMinMaxDistance(fDistMin, fDistMax);
		}
		else if(uiType == NiAudioSource::TYPE_AMBIENT)
		{
			pkAudioSource->SetGain(fVolume * g_fBGMVolume);
			pkAudioSource->SetLoopCount(NiAudioSource::LOOP_INFINITE);
		}

		// Cache에 담는다.
		m_kSoundLock.Lock();
		std::pair<SoundContainer::iterator, bool> ret = m_kSoundContainer.insert(std::make_pair(strPath, stSoundInfo(pkAudioSource)));
		if (ret.second == false)
		{	//??? 명시적으로 지워줘야 하나. insert 실패에서 알아서 지워지나..
			NILOG(PGLOG_WARNING, "[PgSoundMan] %s sound inserted already\n", strPath.c_str());
		}
		itr = ret.first;
	}
	spAudioOriginal = itr->second.m_spSource;
	m_kSoundLock.Unlock();

	if (spAudioOriginal == NULL)
		return 0;

	// Cache의 것을 클론해서 새로운 음원을 만든다.
	pkAudioSource = (NiAudioSource*)spAudioOriginal->Clone();
	if(!pkAudioSource)
	{
		return 0;
	}

	if(fVolume == 0)
		fVolume = spAudioOriginal->GetGain();

	if(fDistMin == 0 || fDistMax == 0)
	{
		spAudioOriginal->GetMinMaxDistance(fDistMin,fDistMax);
	}

	// 원하는 옵션으로 음원을 설정한다.
	if(uiType == NiAudioSource::TYPE_3D)
	{
		pkAudioSource->SetGain(fVolume * g_fEffectVolume);
		pkAudioSource->SetMinMaxDistance(fDistMin, fDistMax);
	}
	else if(uiType == NiAudioSource::TYPE_AMBIENT)
	{
		pkAudioSource->SetGain(fVolume * g_fBGMVolume);
		pkAudioSource->SetLoopCount(NiAudioSource::LOOP_INFINITE);
	}

	if(pkAudioSource->GetStatus() == NiAudioSource::PLAYING)
	{
		pkAudioSource->Stop();
	}
	pkAudioSource->Update(0.0f);
	return pkAudioSource;
}
void PgSoundMan::CacheAudioSource(unsigned int uiType, char const *pcID, float fVolume, float fDistMin, float fDistMax)
{
	if( !pcID || !isInit() )
	{
		if (pcID)
		{
			NILOG(PGLOG_WARNING, "[PgSoundMan] %s audio source can't loaded, before Init.\n", pcID);
		}
		return;
	}

	NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if(!pkSS)
	{
		return;
	}

	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(pcID, PgResourceMonitor::RESOURCE_TYPE_AUDIO));

	// Path Container에서 Path를 찾는다.
	std::string strID = pcID;
	UPR(strID);

	PathContainer::iterator itrPath = m_kPathContainer.find(strID.c_str());
	if(itrPath == m_kPathContainer.end())
	{
		return;		
	}
	
	NiAudioSourcePtr spAudioSource = 0;
	std::string strPath = itrPath->second.m_strPath;
	UPR(strPath);

	m_kSoundLock.Lock();
	SoundContainer::iterator itr = m_kSoundContainer.find(strPath);
	if(itr == m_kSoundContainer.end())
	{
		m_kSoundLock.Unlock();
		// Cache에 없다면 소스를 만든다.
		spAudioSource = pkSS->CreateSource(uiType);
		if(!spAudioSource)
		{
			return;
		}
		
		// 음원을 로드한다.
		spAudioSource->SetFilename(strPath.c_str());
		if(uiType == NiAudioSource::TYPE_AMBIENT)
		{
			//	Ambient 사운드는 캐쉬 하지 않는다.
			return;
		}

		if(!spAudioSource->Load())
		{
			return;
		}

		if(uiType == NiAudioSource::TYPE_3D)
		{
			spAudioSource->SetGain(fVolume * g_fEffectVolume);
			spAudioSource->SetMinMaxDistance(fDistMin, fDistMax);
		}
		else if(uiType == NiAudioSource::TYPE_AMBIENT)
		{
			spAudioSource->SetGain(fVolume * g_fBGMVolume);
			spAudioSource->SetLoopCount(NiAudioSource::LOOP_INFINITE);
		}

		// Cache에 담는다.
		m_kSoundLock.Lock();
		std::pair<SoundContainer::iterator, bool> ret = m_kSoundContainer.insert(std::make_pair(strPath, stSoundInfo(spAudioSource)));
		if (ret.second == false)
		{	//??? 명시적으로 지워줘야 하나. insert 실패에서 알아서 지워지나..
			NILOG(PGLOG_WARNING, "[PgSoundMan] %s sound inserted already\n", strPath.c_str());
		}
	}
	m_kSoundLock.Unlock();

}
NiAudioSource* PgSoundMan::GetAudioSource(unsigned int uiType, char const *pcID, float fVolume, float fDistMin, float fDistMax)
{
	if( !pcID || !isInit() )
	{
		if (pcID)
		{
			NILOG(PGLOG_WARNING, "[PgSoundMan] %s audio source can't loaded, before Init.\n", pcID);
		}
		return 0;
	}

	NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if(!pkSS)
	{
		return 0;
	}

	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(pcID, PgResourceMonitor::RESOURCE_TYPE_AUDIO));

	// Path Container에서 Path를 찾는다.
	std::string strID = pcID;
	UPR(strID);

	PathContainer::iterator itrPath = m_kPathContainer.find(strID.c_str());
	if(itrPath == m_kPathContainer.end())
	{
		return 0;		
	}

	// TODO: 아래는 LoadAudioSource와 거의 같다. 여기에서 LoadAudioSource를 부르던지 할 수 있나?
	return	LoadAudioSource(uiType,itrPath->second.m_strPath.c_str(),fVolume,fDistMin,fDistMax);
}
NiAudioSource* PgSoundMan::PlayAudioSourceByPath(unsigned int uiType, char const *pcPath, float fVolume, float fDistMin, float fDistMax,NiNode *pkParent, NiPoint3* pkPos, int const iLoopCount)
{
	if(pkParent)
	{
		if(!CheckDistance(pkParent->GetWorldTranslate(), fDistMax))
		{
			return NULL;
		}
	}
	
	if (pcPath == NULL)
		return NULL;

	std::string strPath = pcPath;
	UPR(strPath);

	AudioSrcListMap::iterator	itor = m_AudioSrcListMap.find(strPath);
	if(itor != m_AudioSrcListMap.end())
	{
		AudioSrcList	*pkAudioSourceList = &(itor->second);
		
		//플래이가 종료된 음원들은 떼어내고 삭제 한다.
		for(AudioSrcList::iterator itor2 = pkAudioSourceList->begin(); itor2 != pkAudioSourceList->end();)
		{
			NiAudioSource	*pkAudioSource = *itor2;

			bool	bDelete = false;
			
			if(pkAudioSource->GetStatus() == NiAudioSource::DONE)
			{
				bDelete = true;
			}

			if(bDelete)
			{
				if(pkAudioSource->GetParent())
				{
					pkAudioSource->GetParent()->DetachChild(pkAudioSource);
				}

				pkAudioSource->Stop();
				itor2 = pkAudioSourceList->erase(itor2);	//	여기서 RefCount 가 0이되어 삭제된다.
				continue;
			}

			++itor2;
		}

		//하나의 음원이 동시에 플래이가능한 최대치를 넘으면 제일 먼저 추가된 사운드를 제거 한다.
		if(MAX_IDENTITY_PLAY_SOUND_SOURCE < (int)pkAudioSourceList->size())
		{
			NiAudioSource	*pkAudioSource = *pkAudioSourceList->begin();
			if(pkAudioSource)
			{
				if(pkAudioSource->GetParent())
				{
					pkAudioSource->GetParent()->DetachChild(pkAudioSource);
				}
			}

			pkAudioSource->Stop();
			pkAudioSourceList->erase(pkAudioSourceList->begin());
		}

		if(pkAudioSourceList->size() == 0)
		{
			m_AudioSrcListMap.erase(itor);
		}
	}

	// TODO: 아래는 LoadAudioSource와 거의 같다. 여기에서 LoadAudioSource를 부르던지 할 수 있나?
	NiAudioSource* pkNewSource = LoadAudioSource(uiType,strPath.c_str(),fVolume,fDistMin,fDistMax);
	if(!pkNewSource)
	{
		return	NULL;
	}

	if(pkNewSource)
	{
		itor = m_AudioSrcListMap.find(std::string(strPath));

		if(itor != m_AudioSrcListMap.end())
		{
			AudioSrcList	*pkAudioSourceList = &(itor->second);
			pkAudioSourceList->push_back(pkNewSource);
		}
		else
		{
			AudioSrcList	kNewList;
			kNewList.push_back(pkNewSource);
			m_AudioSrcListMap.insert(std::make_pair(strPath,kNewList));
		}
	}

	if(pkParent)
	{
		pkParent->AttachChild(pkNewSource, true);
	}

	if(pkPos)
	{
		pkNewSource->SetTranslate(*pkPos);
	}

	pkNewSource->SetLoopCount(iLoopCount);
	pkNewSource->SetGain(g_fEffectVolume);
	pkNewSource->Update(0);
	pkNewSource->Play();
	

	return	pkNewSource;
}

void PgSoundMan::StopAudioSourceByID(std::string const& rkID)
{
	std::string strPath = rkID;
	UPR(strPath);

	PathContainer::iterator itrPath = m_kPathContainer.find(strPath.c_str());
	if(itrPath == m_kPathContainer.end())
	{
		return ;
	}

	AudioSrcListMap::iterator itor = m_AudioSrcListMap.find(itrPath->second.m_strPath.c_str());
	if(itor != m_AudioSrcListMap.end())
	{
		AudioSrcList* pkAudioSourceList = &(itor->second);

		//플래이가 종료된 음원들은 떼어내고 삭제 한다.
		for(AudioSrcList::iterator itor2 = pkAudioSourceList->begin(); itor2 != pkAudioSourceList->end();)
		{
			NiAudioSource* pkAudioSource = *itor2;

			if(pkAudioSource->GetParent())
			{
				pkAudioSource->GetParent()->DetachChild(pkAudioSource);
			}

			pkAudioSource->Stop();
			itor2 = pkAudioSourceList->erase(itor2);	//	여기서 RefCount 가 0이되어 삭제된다.
		}
	}
}

//!	This function plays the audio source specified by pcID.
NiAudioSource* PgSoundMan::PlayAudioSourceByID(unsigned int uiType, char const *pcID, float fVolume, float fDistMin, float fDistMax,NiNode *pkParent, NiPoint3* pkPos, int const iLoopCount)
{
	if( !pcID || !isInit() )
	{
		if (pcID)
		{
			NILOG(PGLOG_WARNING, "[PgSoundMan] %s audio source can't loaded, before Init.\n", pcID);
		}
		return 0;
	}

	PG_RESOURCE_MONITOR(g_kResourceMonitor.IncreaseResourceCounter(pcID, PgResourceMonitor::RESOURCE_TYPE_AUDIO));

	// Path Container에서 Path를 찾는다.
	std::string strID = pcID;
	UPR(strID);

	PathContainer::iterator itrPath = m_kPathContainer.find(strID.c_str());
	if(itrPath == m_kPathContainer.end())
	{
		return NULL;
	}

	if(0.0f == fVolume)
	{
		fVolume = itrPath->second.m_fVolume;
		if( 0.1f > fVolume )
		{
			NILOG(PGLOG_WARNING, "sound[ID: %s] volume[%f] Less then 0.1\n", pcID, fVolume);
		}
	}

	if(0.0f == fDistMin || 0.0f  == fDistMax)
	{
		fDistMin = itrPath->second.m_fMinDist;
		fDistMax = itrPath->second.m_fMaxDist;
		if( 0.1f > fVolume )
		{
			NILOG(PGLOG_WARNING, "sound[ID: %s] distmin[%f] Less then 0.1\n", pcID, fDistMin);
		}
		if( 0.1f > fVolume )
		{
			NILOG(PGLOG_WARNING, "sound[ID: %s] distmin[%f] Less then 0.1\n", pcID, fDistMax);
		}
	}

	if(fDistMin > fDistMax)
	{
		float fTempDist = fDistMax;
		fDistMax = fDistMin;
		fDistMin = fTempDist;
	}

	return	PlayAudioSourceByPath(uiType,itrPath->second.m_strPath.c_str(),fVolume,fDistMin,fDistMax,pkParent, pkPos, iLoopCount);
}

//float AILCALLBACK FAR EnvRollOff(HSAMPLE sample, float fDistance, float fRolloff_factor, float fMin_dist, float fMax_dist)
//{
//	float fMaxValue = (fMin_dist>fDistance) ? fMin_dist : fDistance;
//	return (fMax_dist - fMaxValue) / (fMax_dist-fMin_dist);
//	//return 1.0f;
//}
//맵 로드시 환경사운드 추가
bool PgSoundMan::AddAndPlayEnvSound(char const* pcPath, float const fVol, float const fMin, float const fMax, NiPoint3 const& kPos, bool bRandomPlay, float fRandomProbability, bool bMixBGSound)
{
	if(NULL == pcPath)
	{
		NILOG(PGLOG_WARNING, "EnvSound : Path can not be null\n");
		return false;
	}
	NiMilesSource* pkSource = NiNew NiMilesSource(NiAudioSource::TYPE_3D);
	if(NULL != pkSource)
	{
		pkSource->SetFilename(pcPath);
		pkSource->Load();
		pkSource->SetMinMaxDistance(fMin, fMax);
		pkSource->SetGain(fVol);
		pkSource->SetTranslate(kPos);
		if(true == bRandomPlay)
		{
			pkSource->SetLoopCount(NiAudioSource::LOOP_SINGLE);
		}
		else
		{
			pkSource->SetLoopCount(NiAudioSource::LOOP_INFINITE);
		}
		pkSource->Update(0);
		pkSource->SetAllowSharing(true);
		pkSource->Play();
	}
	else
	{
		NILOG(PGLOG_WARNING, "EnvSound : Path [%s] load failed\n", pcPath);
	}

	EnvSound kTemp(pcPath, pkSource, fVol, fMin, fMax, kPos, bRandomPlay, fRandomProbability, bMixBGSound);
	m_EnvSoundContainer.push_back(kTemp);
	return true;
}

void PgSoundMan::UpdateEnvSound(float const fTime)
{
	PgActor* pkPlayerActor = g_kPilotMan.GetPlayerActor();
	if(NULL != pkPlayerActor)
	{
		NiPoint3 kPlayerPos=pkPlayerActor->GetPos();
		NiPoint3 kSoundPos(0.0f ,0.0f, 0.0f);
		if(false == m_EnvSoundContainer.empty())
		{
			float const fNoGain = 0.f;
			float const fBaseGain = 1.f;
			float fBiggestGain = fNoGain;
			EnvSoundContainer::iterator env_iter = m_EnvSoundContainer.begin();
			for(;env_iter != m_EnvSoundContainer.end();++env_iter)
			{//모든 환경사운드에 대해
				//음원과의 거리
				EnvSoundContainer::value_type& rkEventSound = (*env_iter);
				float const fDistance = (rkEventSound.m_kPos-kPlayerPos).Length();
				float const fMaxValue = (rkEventSound.m_fMin>fDistance) ? rkEventSound.m_fMin : fDistance;
				float fGainScale = ((rkEventSound.m_fMax - fMaxValue) / (rkEventSound.m_fMax - rkEventSound.m_fMin));
				//최소거리값과 음원과의 거리 중 어떤게 더 큰가?
				if(fDistance < rkEventSound.m_fMax)
				{//실제 들릴 음원만 처리
					rkEventSound.m_pkSource->SetGain( rkEventSound.m_fVolume * fGainScale * g_fBGMVolume );
					rkEventSound.m_pkSource->Update(fTime);

					if(NiAudioSource::DONE == rkEventSound.m_pkSource->GetStatus())
					{//사운드가 종료된 상태면
						//다시 재생될 확률에 따라 재생여부 결정, 확률은 초당확률
						if( rkEventSound.m_fRandomProbability*fTime > static_cast<float>(BM::Rand_Range(100)) )
						{		
							rkEventSound.m_pkSource->Play();
						}
					}
				}
				else
				{
					fGainScale = fNoGain;
					rkEventSound.m_pkSource->SetGain(fGainScale);
					rkEventSound.m_pkSource->Update(fTime);
				}
				if( false == rkEventSound.m_bMixBGSound )
				{
					fBiggestGain = std::max(fBiggestGain, fGainScale);
				}
			}

			if( m_kCurrentBGM.m_spBgmSound )
			{
				float const fNewBGMGain = m_kCurrentBGM.m_fVolume * g_fBGMVolume * (fBaseGain - fBiggestGain);
				m_kCurrentBGM.m_spBgmSound->SetGain(fNewBGMGain);
				m_kCurrentBGM.m_spBgmSound->FadeToGain(fNewBGMGain, g_pkApp->GetAccumTime(), lua_tinker::call<float>("GetBgSoundFadeInTime"));
			}
		}
	}
}

//맵 끝날때 환경사운드 제거
void PgSoundMan::ReleaseEnvSound()
{
	if(false == m_EnvSoundContainer.empty())
	{
		EnvSoundContainer::iterator kEnvIter = m_EnvSoundContainer.begin();
		for(;kEnvIter != m_EnvSoundContainer.end();++kEnvIter)
		{
			kEnvIter->m_pkSource->Stop();
			kEnvIter->m_pkSource->Unload();
			NiDelete kEnvIter->m_pkSource;
		}
		m_EnvSoundContainer.clear();
	}
}
//환경사운드 디버그 렌더링 할까 말까?
void PgSoundMan::SetEvnSoundDebugRendering(float bSet)
{
	if(!g_pkWorld)
	{
		return;
	}
	NiNode* pkSceneRoot = g_pkWorld->GetSceneRoot();	
}

bool PgSoundMan::ParseXml(char const *pcXmlPath)
{
	NILOG(PGLOG_LOG, "[PgSoundMan::ParseXml] Start\n");

	int nCount = 0;
	int nFaildCount = 0;

	TiXmlDocument kXmlDoc(pcXmlPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
	{
		PgError1("Parse Failed [%s]", pcXmlPath);
		return false;
	}

	// Root 'SOUND'
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();

	assert(strcmp(pkElement->Value(), "SOUND") == 0);

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "ITEM") == 0)
		{
			char const *pcID = 0;
			char const *pcPath = pkElement->GetText();
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			float fVolume = 1.0f;
			float fMinDist = 250.0f;
			float fMaxDist = 300.0f;

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					pcID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "VOLUME") == 0)
				{
					sscanf_s(pcAttrValue, "%f, %f, %f", &fVolume, &fMinDist, &fMaxDist);
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}
				 
				pkAttr = pkAttr->Next();
			}

			pcPath = pkElement->GetText();

			if(!pcID || !pcPath)
			{
				PgXmlError(pkElement, "Not Enough Effect Data");
				
			}
			else
			{
				std::string strID = pcID;
				std::string strPath = pcPath;
				UPR(strID);
				UPR(strPath);
				
				SoundInitInfo InitInfo;
				InitInfo.m_strPath = strPath;
				InitInfo.m_fVolume = fVolume;
				InitInfo.m_fMinDist = fMinDist;
				InitInfo.m_fMaxDist = fMaxDist;
				
				m_kPathContainer.insert(std::make_pair(strID, InitInfo));

				// 미리 로딩하도록 한다.
				if (fVolume < 0.0f)
				{
					NILOG(PGLOG_WARNING, "[PgSoundMan] %s sound volume(%f) is not correct!\n", pcID, fVolume);
					fVolume = 0.0f;
				}

				if (fVolume > 1.0f)
				{
					NILOG(PGLOG_WARNING, "[PgSoundMan] %s sound volume(%f) is not correct!\n", pcID, fVolume);
					fVolume = 1.0f;
				}

				if (fMinDist >= fMaxDist)
				{
					NILOG(PGLOG_WARNING, "[PgSoundMan] %s sound distance(%f,%f) is not correct!\n", pcID, fMinDist, fMaxDist);
					fMinDist = fMaxDist;
				}

				//CacheAudioSource(NiAudioSource::TYPE_3D, pcID, fVolume, fMinDist, fMaxDist);					
			}
		}
		else if(strcmp(pcTagName, "LOCAL") == 0)
		{
			TiXmlNode const* pkFindLocalNode = PgXmlLocalUtil::FindInLocal(g_kLocal, pkElement, "PgUIScene");
			if( pkFindLocalNode )
			{
				TiXmlElement const* pkResultNode = pkFindLocalNode->FirstChildElement();
				if( pkResultNode )
				{
					ParseXmlLocal( pkResultNode );
				}
			}
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
		}

		pkElement = pkElement->NextSiblingElement();
	}

	SetBGMVolume(0.0f, true);
	SetEffectVolume(0.0f, true);
	NILOG(PGLOG_LOG, "[PgSoundMan::ParseXml] Loading Count (%d/%d), ParseXml End", nCount, nFaildCount);	
	return true;
}

void PgSoundMan::ParseXmlLocal(TiXmlElement const* pkElement)
{
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();

		if(strcmp(pcTagName, "ITEM") == 0)
		{
			char const *pcID = 0;
			char const *pcPath = pkElement->GetText();
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			float fVolume = 1.0f;
			float fMinDist = 250.0f;
			float fMaxDist = 300.0f;

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				if(strcmp(pcAttrName, "ID") == 0)
				{
					pcID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "VOLUME") == 0)
				{
					sscanf_s(pcAttrValue, "%f, %f, %f", &fVolume, &fMinDist, &fMaxDist);
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			pcPath = pkElement->GetText();

			if(!pcID || !pcPath)
			{
				PgXmlError(pkElement, "Not Enough Effect Data");

			}
			else
			{
				std::string strID = pcID;
				std::string strPath = pcPath;
				UPR(strID);
				UPR(strPath);

				SoundInitInfo InitInfo;
				InitInfo.m_strPath = strPath;
				InitInfo.m_fVolume = fVolume;
				InitInfo.m_fMinDist = fMinDist;
				InitInfo.m_fMaxDist = fMaxDist;

				m_kPathContainer.insert(std::make_pair(strID, InitInfo));

				// 미리 로딩하도록 한다.
				if (fVolume < 0.0f)
				{
					NILOG(PGLOG_WARNING, "[PgSoundMan] %s sound volume(%f) is not correct!\n", pcID, fVolume);
					fVolume = 0.0f;
				}

				if (fVolume > 1.0f)
				{
					NILOG(PGLOG_WARNING, "[PgSoundMan] %s sound volume(%f) is not correct!\n", pcID, fVolume);
					fVolume = 1.0f;
				}

				if (fMinDist >= fMaxDist)
				{
					NILOG(PGLOG_WARNING, "[PgSoundMan] %s sound distance(%f,%f) is not correct!\n", pcID, fMinDist, fMaxDist);
					fMinDist = fMaxDist;
				}

				//CacheAudioSource(NiAudioSource::TYPE_3D, pcID, fVolume, fMinDist, fMaxDist);					
			}
		}
		else if(strcmp(pcTagName, "LOCAL") == 0)
		{
			TiXmlNode const* pkFindLocalNode = PgXmlLocalUtil::FindInLocal(g_kLocal, pkElement, "PgUIScene");
			if( pkFindLocalNode )
			{
				TiXmlElement const* pkResultNode = pkFindLocalNode->FirstChildElement();
				if( pkResultNode )
				{
					ParseXmlLocal( pkResultNode );
				}
			}
		}
		else
		{
			PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
		}

		pkElement = pkElement->NextSiblingElement();
	}
}

void PgSoundMan::SetBGMVolume(float fVolume, bool bFromConfig)
{
	//TODO: 첫 실행시 사운드 설정이 0(끄기)라더라도 아래에서 이상하게 사운드 볼륨이 설정된다.
	if (!isInit())	{return;}

	if(bFromConfig)
	{
		fVolume = GetConfigBGMVolume();
	}
	else
	{
		fVolume = fVolume * m_fBGMVolumeWeight;
	}

	NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if (pkSS == NULL) {return;}

	g_fBGMVolume = fVolume;

	if (g_fBGMVolume < 0.0f)
	{
		g_fBGMVolume = 0.0f;
	}

	if (g_fBGMVolume > 1.0f)
	{
		g_fBGMVolume = 1.0f;
	}

	if (	m_kCurrentBGM.m_spBgmSound 
		&&	m_kCurrentBGM.m_spBgmSound->GetStatus() == NiAudioSource::PLAYING )
	{
		m_kCurrentBGM.m_spBgmSound->SetGain(m_kCurrentBGM.m_fVolume * g_fBGMVolume);
		m_kCurrentBGM.m_spBgmSound->FadeToGain( m_kCurrentBGM.m_spBgmSound->GetGain(), g_pkApp->GetAccumTime(), lua_tinker::call<float>("GetBgSoundFadeInTime"));
	}

}

void PgSoundMan::SetEffectVolume(float fVolume, bool bFromConfig)
{
	if (!isInit())
		return;

	if(bFromConfig)
		fVolume = GetConfigEffectVolume();
	else
		fVolume = fVolume * m_fEffectVolumeWeight;

	NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if (pkSS == NULL)
		return;	

	g_fEffectVolume = fVolume;

	if (g_fEffectVolume < 0.0f)
		g_fEffectVolume = 0.0f;

	if (g_fEffectVolume > 1.0f)
		g_fEffectVolume = 1.0f;

	NiTListIterator iter;
	NiAudioSource* pkAudioSource = pkSS->GetFirstSource(iter);
	while (pkAudioSource)
	{
		if (pkAudioSource->GetType() == NiAudioSource::TYPE_3D)
		{
			pkAudioSource->SetGain(g_fEffectVolume);
		}
		else
		{
			bool bBGM = false;
			for (unsigned int i = 0; i < m_kBGSoundCont.size(); i++)
			{
				if (m_kBGSoundCont[i].m_spBgmSound && m_kBGSoundCont[i].m_spBgmSound == pkAudioSource)
				{
					bBGM = true;
					break;
				}
			}

			if (bBGM == false)
			{
				pkAudioSource->SetGain(g_fEffectVolume);
			}
		}
		pkAudioSource = pkSS->GetNextSource(iter);
	}
}

void PgSoundMan::SetSilence(bool bSilence)
{
	m_bSilence = bSilence;	
}
/*
HRESULT PgSoundMan::LoadResource(T_MSG &rkMsg, void *&pkOutRsc)const
{
	
	return S_OK;
}
*/
void	PgSoundMan::RecursiveDetachAllSoundObject(NiAVObject *pkObject)
{
	NiNode	*pkNode = NiDynamicCast(NiNode,pkObject);
	if(pkNode)
	{
		int	iCount = pkNode->GetArrayCount();
		for(int i=0;i<iCount;i++)
		{
			NiAVObject	*pkChild = pkNode->GetAt(i);
			if(!pkChild)
			{
				continue;
			}

			NiAudioSource	*pkAudioSource = NiDynamicCast(NiAudioSource,pkChild);
			if(pkAudioSource)
			{
				pkNode->DetachChildAt(i);
				continue;
			}

			RecursiveDetachAllSoundObject(pkChild);
		}
	}
}
bool PgSoundMan::DoClientWork(WorkData& rkWorkData)
{
	return true;
}

bool PgSoundMan::CheckDistance(NiPoint3 const& kSourcePos, float const fDistMax) const
{
	NiMilesAudioSystem* pkSS = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if(pkSS)
	{
		NiMilesListener *pkListener = pkSS->GetListener();
		if(pkListener)
		{
			float const fDistance = (pkListener->GetWorldTranslate() - kSourcePos).Length();
			if((fDistMax * 3.0f) < fDistance)
			{
				return false;
			}
		}
	}

	return true;
}