#include "StdAfx.h"
#include "PgEffectMan.h"
#include "PgEffect.h"
#include "PgXmlLoader.h"

PgEffectMan::PgEffectMan(void)
{
}

PgEffectMan::~PgEffectMan(void)
{
}

PgEffect *PgEffectMan::GetEffect(int iEffectNo)
{
	Container::iterator itr = m_kContainer.find(rkGuid);

	if(itr == m_kContainer.end())
	{
		PgEffect *pkEffect = dynamic_cast<PgEffect *>(PgXmlLoader::CreateObject(rkGuid));

		if(!pkEffect)
		{
			assert(0 && "failed to loading effect");
			return 0;
		}

		itr = m_kContainer.insert(std::make_pair(rkGuid, pkEffect)).first;
	}

	PgEffect *pkEffect = (PgEffect *)itr->second->Clone();
	if(!pkEffect)
	{
		assert(0 && "failed to clonning effect");
		return 0;
	}

	pkEffect->UpdateProperties();
	pkEffect->UpdateEffects();
	
	return pkEffect;
}


bool PgEffectMan::ParseXml(const char *pcXmlPath, void *pArg)
{
	TiXmlDocument kXmlDoc(pcXmlPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(pcXmlPath)))
	{
		PgError1("Parse Failed [%s]", pcXmlPath);
		return false;
	}
	
	// Root 'EFFECT'
	const TiXmlElement *pkElement = kXmlDoc.FirstChildElement();

	assert(strcmp(pkElement->Value(), "EFFECT") == 0);

	pkElement = pkElement->FirstChildElement();
	const char *pcTagName = pkElement->Value();

	if(strcmp(pcTagName, "SCRIPT") == 0)
	{
		const TiXmlAttribute *pkAttr = pkElement->FirstAttribute();

		while(pkAttr)
		{
			const char *pcAttrName = pkAttr->Name();
			const char *pcAttrValue = pkAttr->Value();

			if(strcmp(pcAttrName, "NAME") == 0)
			{
				m_kScript = pcAttrValue;
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
			m_kPathContainer.insert(std::make_pair(pcID, pcPath));

			// 미리 로딩하도록 한다.
			NiAudioSourcePtr spAudioSource = GetAudioSource(NiAudioSource::TYPE_3D, pcID, fVolume);
			spAudioSource = 0;
		}
	}
	else
	{
		PgXmlError1(pkElement, "XmlParse: Incoreect Tag '%s'", pcTagName);
	}
	
	return true;

}