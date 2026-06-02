#include "stdafx.h"
#include "PgStitchImageMng.h"

size_t const iMaxStitchImageWndCount = 10;
size_t const iMaxStitchTextWndCount = 10;

PgStitchImageMng::PgStitchImageMng()
{
}


PgStitchImageMng::~PgStitchImageMng()
{
}


void PgStitchImageMng::Reload()
{
	m_kCont.clear();
	PgXmlLoader::ReleaseXmlDocumentInCacheByID(XML_ELEMENT_STITCH_IMAGE);
	PgXmlLoader::CreateObject(XML_ELEMENT_STITCH_IMAGE);
}


bool PgStitchImageMng::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
{
	if( !pkNode )
	{
		return false;
	}

	TiXmlElement const *pkImageNode = pkNode->FirstChildElement("IMAGE");
	while( pkImageNode )
	{
		ContImagePeice kPeiceVec;
		ContTextPeice kTextVec;

		POINT3I kImagePos;
		char const *szID = pkImageNode->Attribute("ID");
		pkImageNode->Attribute("X", &kImagePos.x);
		pkImageNode->Attribute("Y", &kImagePos.y);


		//
		TiXmlElement const *pkPeiceNode = pkImageNode->FirstChildElement("PEICE");
		while( pkPeiceNode )
		{
			POINT3I kPos;
			int iWidth = 0, iHeight = 0;
			char const *szImagePath = pkPeiceNode->Attribute("PATH");
			pkPeiceNode->Attribute("X", &kPos.x);
			pkPeiceNode->Attribute("Y", &kPos.y);
			pkPeiceNode->Attribute("Z", &kPos.z);
			pkPeiceNode->Attribute("W", &iWidth);
			pkPeiceNode->Attribute("H", &iHeight);


			if( szImagePath )
			{
				std::wstring kImagePath = UNI(szImagePath);
				ContImagePeice::value_type kNewElement(kImagePath, kPos, POINT2(iWidth, iHeight));
				std::back_inserter(kPeiceVec) = kNewElement;
			}
			pkPeiceNode = pkPeiceNode->NextSiblingElement();
		}


		//
		TiXmlElement const *pkTextNode = pkImageNode->FirstChildElement("TEXT");
		while( pkTextNode )
		{
			int iTextID = 0;
			POINT3I kTextPos;
			int iTextWidth = 0, iTextHeight = 0;
			DWORD dwFontOption = 0;
			DWORD dwFontColor = COLOR_BLACK, dwOutlineColor = COLOR_WHITE;

			pkTextNode->Attribute("TEXT_ID", &iTextID);
			char const* szFontName = pkTextNode->Attribute("FONT");
			char const* szAlign = pkTextNode->Attribute("ALIGN");
			char const* szOutLine = pkTextNode->Attribute("OUTLINE");
			char const* szFontColor = pkTextNode->Attribute("COLOR");
			char const* szOutlineColor = pkTextNode->Attribute("OUTLINE_COLOR");
			pkTextNode->Attribute("X", &kTextPos.x);
			pkTextNode->Attribute("Y", &kTextPos.y);
			pkTextNode->Attribute("Z", &kTextPos.z);
			pkTextNode->Attribute("W", &iTextWidth);
			pkTextNode->Attribute("H", &iTextHeight);
			
			if( szOutLine && 0 != strcmp("0", szOutLine) )
			{
				dwFontOption |= XUI::XTF_OUTLINE;
			}
			if( szAlign && 0 == strcmp("CENTER", szAlign) )
			{
				dwFontOption |= XUI::XTF_ALIGN_CENTER;
			}
			if( szAlign && 0 == strcmp("RIGHT", szAlign) )
			{
				dwFontOption |= XUI::XTF_ALIGN_RIGHT;
			}
			if( szFontColor )
			{
				sscanf_s(szFontColor, "%x", &dwFontColor);
			}
			if( szOutlineColor )
			{
				sscanf_s(szOutlineColor, "%x", &dwOutlineColor);
			}

			if( iTextID && szFontName )
			{
				ContTextPeice::value_type kNewText(iTextID, kTextPos, POINT2(iTextWidth, iTextHeight), UNI(szFontName), dwFontOption, dwFontColor, dwOutlineColor);
				std::back_inserter(kTextVec) = kNewText;
			}

			pkTextNode = pkTextNode->NextSiblingElement();
		}

		if( szID )
		{
			if( iMaxStitchImageWndCount <= kPeiceVec.size() )
			{
			//	VERIFY_INFO_LOG(false, BM::LOG_LV1, _T("[%s]-[%d] Can't over than %d Stitch image Peices Current:%u"), __FUNCTIONW__, __LINE__, iMaxStitchImageWndCount, kPeiceVec.size());
			}
			if( iMaxStitchTextWndCount <= kTextVec.size() )
			{
			//	VERIFY_INFO_LOG(false, BM::LOG_LV1, _T("[%s]-[%d] Can't over than %d Stitch Texts Current:%u"), __FUNCTIONW__, __LINE__, iMaxStitchTextWndCount, kTextVec.size());
			}

			std::wstring kID = UNI(szID);
			UPR(kID);
			ContStitchImage::mapped_type kNewElement(kID, kImagePos, kPeiceVec, kTextVec);
			std::pair<ContStitchImage::iterator, bool> kRet = m_kCont.insert( std::make_pair(kID, kNewElement) );
			if( !kRet.second )
			{
			//	VERIFY_INFO_LOG(false, BM::LOG_LV1, _T("[%s]-[%d] can't insert Stitch Image ID:%s"), __FUNCTIONW__, __LINE__, kID.c_str());
			}

			ContStitchImage m_kCont;
		}

		pkImageNode = pkImageNode->NextSiblingElement();
	}

	return true;
}


bool PgStitchImageMng::Get(std::wstring const &rkID, SStitchImage &rkOut) const
{
	std::wstring kTemp = rkID;
	UPR(kTemp);
	ContStitchImage::const_iterator find_iter = m_kCont.find(kTemp);
	if( m_kCont.end() == find_iter )
	{
		return false;
	}
	rkOut = (*find_iter).second;
	return true;
}