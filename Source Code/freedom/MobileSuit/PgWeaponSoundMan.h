#ifndef FREEDOM_DRAGONICA_SOUND_PGWEAPONSOUNDMAN_H
#define FREEDOM_DRAGONICA_SOUND_PGWEAPONSOUNDMAN_H

#include "PgIXmlObject.h"

class PgWeaponSoundManager
{
	typedef std::map<std::string, std::string> HitSoundContainer;
	typedef std::map<std::string, std::string> DamageSoundContainer;

public:
	typedef enum eWeaponSoundType
	{
		WST_NONE = 0,
		WST_HIT = 1,
		WST_DAMAGE = 2,
	} EWeaponSoundType;

	PgWeaponSoundManager();
	~PgWeaponSoundManager();

	bool Initialize();
	void Destroy();
	bool ParseXml(char const *pcXmlPath);
	bool ResetWeaponSound();

	const	std::string& GetSound(EWeaponSoundType eType, int iWeaponType, char const *pcActionID, unsigned int iWeaponNo = 0);

protected:
	HitSoundContainer m_kHitSoundContainer;
	
	DamageSoundContainer m_kDamageSoundContainer;
};

#define g_kWeaponSoundMan SINGLETON_STATIC(PgWeaponSoundManager)
#endif // FREEDOM_DRAGONICA_SOUND_PGWEAPONSOUNDMAN_H