#include "StdAfx.h"
#include "PgWeaponSoundMan.h"
#include "PgSoundMan.h"

PgWeaponSoundManager::PgWeaponSoundManager()
{
}

PgWeaponSoundManager::~PgWeaponSoundManager()
{
}

const	std::string&  PgWeaponSoundManager::GetSound(EWeaponSoundType eType, int iWeaponType, char const *pcActionID, unsigned int iWeaponNo)
{
	static	std::string kEmptyText("");
	BM::vstring kFirst;
	BM::vstring kFirst_Weapon; 
	if(iWeaponNo)//무기 번호
	{
		kFirst_Weapon+=iWeaponNo;
	}
	else
	{
		kFirst_Weapon+=iWeaponType;
	}

	kFirst += iWeaponType;
	
	if(pcActionID)
	{
		kFirst += pcActionID;
		kFirst_Weapon += UNI(pcActionID);
	}

	// Cache의 것을 클론해서 새로운 음원을 만든다.
	NiAudioSourcePtr spAudioSource = 0;
	if(eType == WST_HIT)
	{
		//없으면 기본 타입에서 찾아보자
		HitSoundContainer::iterator itr = m_kHitSoundContainer.find(MB(kFirst));
		if(itr == m_kHitSoundContainer.end())
		{
			return kEmptyText;
		}

		return	itr->second;
	}
	else if(eType == WST_DAMAGE)
	{
		DamageSoundContainer::iterator itr = m_kDamageSoundContainer.find(MB(kFirst));
		if(itr == m_kDamageSoundContainer.end())
		{
			return kEmptyText;
		}

		return	itr->second;
	}

	return kEmptyText;

/*	// 원하는 옵션으로 음원을 설정한다.
	spAudioSource->SetGain(g_fEffectVolume);
	if(spAudioSource->GetStatus() == NiAudioSource::PLAYING)
	{
		spAudioSource->Stop();
	}

	spAudioSource->Update(0.0f);

	return spAudioSource;*/
}

bool PgWeaponSoundManager::Initialize()
{
	return ParseXml("weapon_sound.xml");
}

bool PgWeaponSoundManager::ResetWeaponSound()
{
	Destroy();
	return Initialize();
}

void PgWeaponSoundManager::Destroy()
{
	m_kHitSoundContainer.clear();
	m_kDamageSoundContainer.clear();
}

bool PgWeaponSoundManager::ParseXml(char const *pcXmlPath)
{
	TiXmlDocument kXmlDoc(pcXmlPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
	{
		PgError1("Parse Failed [%s]", pcXmlPath);
		return false;
	}
	
	// Root 'WEAPON-SOUND'
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();

	assert(strcmp(pkElement->Value(), "WEAPON-SOUND") == 0);

	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const *pcTagName = pkElement->Value();
		EWeaponSoundType iType = WST_NONE;
		
		if(strcmp(pcTagName, "HIT") == 0)
		{
			iType = WST_HIT;
		}
		else if(strcmp(pcTagName, "GUARD") == 0)
		{
			iType = WST_DAMAGE;
		}

		if(iType != WST_NONE)
		{
			const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();
			int iWeaponType = 0;
			char const *pcActionID = 0;
			int iWeaponNo = 0;

			while(pkAttr)
			{
				char const *pcAttrName = pkAttr->Name();
				char const *pcAttrValue = pkAttr->Value();

				// 무기 종류
				if(strcmp(pcAttrName, "TYPE") == 0)
				{
					iWeaponType = atoi(pcAttrValue);
				}
				else if(strcmp(pcAttrName, "ACTION") == 0)
				{
					pcActionID = pcAttrValue;
				}
				else if(strcmp(pcAttrName, "WEAPON_NO") == 0)	//무기번호
				{
					iWeaponNo = atoi(pcAttrValue);
				}

				pkAttr = pkAttr->Next();
			}


			// 컨테이너에 추가
			BM::vstring kKey;
			if(0<=iWeaponType)
			{
				kKey += iWeaponType;
			}
			else if(0<iWeaponNo)
			{
				BM::vstring kNo(iWeaponNo);
				kKey += MB(kNo);
			}

			if(pcActionID)
			{
				kKey += pcActionID;
			}

			if(iType == WST_HIT)
			{
				m_kHitSoundContainer.insert(std::make_pair(MB(kKey), std::string(pkElement->GetText())));
			}
			else if(iType == WST_DAMAGE)
			{
				m_kDamageSoundContainer.insert(std::make_pair(MB(kKey), std::string(pkElement->GetText())));
			}
		}

		pkElement = pkElement->NextSiblingElement();
	}
	
	return true;
}