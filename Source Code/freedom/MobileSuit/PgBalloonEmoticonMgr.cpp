#include "StdAfx.h"
#include "PgNifMan.h"
#include "PgBalloonEmoticonMgr.h"
#include "PgPilot.h"
#include "PgPilotMan.h"


void lwBalloonEmoticonUtil::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;
	def(pkState, "BalloonEmoticonSwitch", lwBalloonEmoticonSwitch);
}

void lwBalloonEmoticonUtil::lwBalloonEmoticonSwitch(bool const bPause)
{
	g_kBalloonEmoticonMgr.Pause(bPause);
}

PgBalloonEmoticonMgr::PgBalloonEmoticonMgr(void)
{
}

PgBalloonEmoticonMgr::~PgBalloonEmoticonMgr(void)
{
}

bool PgBalloonEmoticonMgr::Initialize()
{
	TiXmlDocument kXmlDoc("ui/BalloonEmoticon.XML");
	if( !PgXmlLoader::LoadFile(kXmlDoc, UNI("ui/BalloonEmoticon.XML")) )
	{
		m_bAlive = false;
	}

	TiXmlElement const* pkElement = kXmlDoc.FirstChildElement();
	while( pkElement )
	{
		char const* pcTagName = pkElement->Value();
		if( strcmp(pcTagName, "BALLOON_EMOTICON") == 0 )
		{
			if( !ParseXML(pkElement->FirstChildElement()) )
			{
				m_bAlive = false;
			}
		}
		else
		{
			//None Def TagName
		}
		pkElement = pkElement->NextSiblingElement();
	}

	m_bAlive = true;
	return m_bAlive;
}

void PgBalloonEmoticonMgr::Update(float fAccumTime,float fFrameTime)
{
	if( !m_bAlive )
	{
		return;
	}

	kInstanceContainer::iterator	iter = m_kInstanceContainer.begin();
	while( m_kInstanceContainer.end() != iter )
	{
		PgBalloonEmoticonInstance* pInstance = iter->second;
		if( pInstance && !pInstance->Update(fAccumTime, fFrameTime, Pause()) )
		{
			m_kEraseContainer.push_back(iter->second);
			iter->second = NULL;
		}
		++iter;
	}
}

void PgBalloonEmoticonMgr::DrawImmediate(PgRenderer *pkRenderer)
{
	if( !m_bAlive || Pause() )
	{
		return;
	}
	kInstanceContainer::iterator	iter = m_kInstanceContainer.begin();
	while( m_kInstanceContainer.end() != iter )
	{
		PgBalloonEmoticonInstance* pInstance = iter->second;
		if( pInstance == NULL )
		{
			iter = m_kInstanceContainer.erase(iter);
			continue;
		}

		PgActor* pActor = g_kPilotMan.FindActor(iter->first);
		if( !pActor )
		{
			m_kEraseContainer.push_back(iter->second);
			iter = m_kInstanceContainer.erase(iter);
			continue;
		}

		pInstance->DrawImmediate(pkRenderer, pActor);
		++iter;
	}
	EraseInstance();
}

void PgBalloonEmoticonMgr::Terminate()
{
	if( true == m_bAlive )
	{
		m_bAlive = false;
	}

	ClearInstance();
	EraseInstance();

	kDataContainer::iterator	Dat_iter = m_kDataContainer.begin();
	while( m_kDataContainer.end() != Dat_iter )
	{
		Dat_iter = m_kDataContainer.erase(Dat_iter);
	}
}


bool PgBalloonEmoticonMgr::AddNewBalloonEmoticon(PgActor* pActor, int const ID)
{
	if( !pActor )
	{
		return false;
	}

	kDataContainer::iterator	Dat_iter = m_kDataContainer.find(ID);
	if( m_kDataContainer.end() != Dat_iter )
	{
		PgBalloonEmoticonInstance*	pEmoticon = new PgBalloonEmoticonInstance;
		if( pEmoticon && pEmoticon->Initialize(&Dat_iter->second) )
		{
			std::pair<kInstanceContainer::iterator, bool> result = m_kInstanceContainer.insert(std::make_pair(pActor->GetGuid(), pEmoticon));
			if( !result.second )
			{// ÀÌ¹Ì ÀÖÀ»¶©?
				m_kEraseContainer.push_back(result.first->second);
				result.first->second = pEmoticon;
			}
		}
		return false;
	}
	return true;
}

void PgBalloonEmoticonMgr::ClearInstance()
{
	kInstanceContainer::iterator	ins_iter = m_kInstanceContainer.begin();
	while( m_kInstanceContainer.end() != ins_iter )
	{
		m_kEraseContainer.push_back(ins_iter->second);
		ins_iter->second = NULL;
		++ins_iter;
	}
}

void PgBalloonEmoticonMgr::EraseInstance()
{
	kEraseContainer::iterator	erase_iter = m_kEraseContainer.begin();
	while( m_kEraseContainer.end() != erase_iter )
	{
		SAFE_DELETE((*erase_iter));
		erase_iter = m_kEraseContainer.erase(erase_iter);
	}
}

bool PgBalloonEmoticonMgr::Reload()
{
	Terminate();
	return Initialize();
}

bool PgBalloonEmoticonMgr::ParseXML(TiXmlElement const* pkElement)
{
	while( pkElement )
	{
		char const* pcTagName = pkElement->Value();
		if( strcmp(pcTagName, "EMOTICON") == 0 )
		{
			PgBalloonEmoticonData	EmoticonData;
			if( !ParseEmoticonAttribute(pkElement->FirstAttribute(), EmoticonData) )
			{
				return false;
			}

			NiSourceTexturePtr	pTexture = g_kNifMan.GetTexture(EmoticonData.ImagePath());
			if( !pTexture )
			{
				return false;
			}
			NiScreenTexturePtr	pScreenTexture = NiNew NiScreenTexture(pTexture);
			if( !pScreenTexture )
			{
				return false;
			}

			TiXmlElement const* pkChildElem = pkElement->FirstChildElement();
			while( pkChildElem )
			{
				pcTagName = pkChildElem->Value();
				if( strcmp(pcTagName, "FRAME") == 0 )
				{
					PgBalloonEmoticonFrameData	FrameData;
					if( !ParseFrameAttribute(pkChildElem->FirstAttribute(), FrameData) )
					{
						return false;
					}

					FrameData.SrcTexture(pScreenTexture);
					EmoticonData.AddFrame(FrameData);
				}
				pkChildElem = pkChildElem->NextSiblingElement();
			}
			std::pair<kDataContainer::iterator, bool> result = m_kDataContainer.insert(std::make_pair(EmoticonData.ID(), EmoticonData));
			if( !result.second )
			{
				return false;
			}
		}
		else
		{	
			//None Def TagName
		}
		pkElement = pkElement->NextSiblingElement();
	}

	return true;
}

bool PgBalloonEmoticonMgr::ParseEmoticonAttribute(TiXmlAttribute const* pkAttr, PgBalloonEmoticonData& EmoticonInfo)
{
	while( pkAttr )
	{
		char const* pcTagName = pkAttr->Name();
		char const* pcTagValue = pkAttr->Value();

		if( strcmp( pcTagName, "ID" ) == 0 )
		{
			EmoticonInfo.ID(atoi(pcTagValue));
		}
		else if( strcmp( pcTagName, "IMAGE_PATH" ) == 0 )
		{
			EmoticonInfo.ImagePath(pcTagValue);
		}
		else if( strcmp( pcTagName, "FRAME_SIZE" ) == 0 )
		{
			typedef std::list< std::wstring > CUT_STR;
			CUT_STR kList;
			BM::vstring::CutTextByKey<std::wstring>(UNI(pcTagValue), _T(","), kList);
			if( kList.size() != 2 )
			{
				return false;
			}

			CUT_STR::const_iterator c_iter = kList.begin();
			POINT2	kSize;
			kSize.x = _wtoi(c_iter++->c_str());
			kSize.y = _wtoi(c_iter->c_str());
			EmoticonInfo.FrameSize(kSize);
		}
		else if( strcmp( pcTagName, "U" ) == 0 )
		{
			EmoticonInfo.U(atoi(pcTagValue));
		}
		else if( strcmp( pcTagName, "LOOP_COUNT" ) == 0 )
		{
			EmoticonInfo.LoopType(atoi(pcTagValue));
		}
		else
		{
			//None Def TagName
		}
		pkAttr = pkAttr->Next();
	}
	return true;
}

bool PgBalloonEmoticonMgr::ParseFrameAttribute(TiXmlAttribute const* pkAttr, PgBalloonEmoticonFrameData& FrameInfo)
{
	while( pkAttr )
	{
		char const* pcTagName = pkAttr->Name();
		char const* pcTagValue = pkAttr->Value();

		if( strcmp( pcTagName, "POS" ) == 0 )
		{
			FrameInfo.FrameIdx(static_cast<WORD>(atoi(pcTagValue)));
		}
		else if( strcmp( pcTagName, "DELAY" ) == 0 )
		{
			FrameInfo.DelayTime(atoi(pcTagValue) * 0.001f);
		}
		else
		{
			//None Def TagName
		}
		pkAttr = pkAttr->Next();
	}
	return true;
}




