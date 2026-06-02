#include "stdafx.h"
#include "PgEmoticonFont.H"

using namespace XUI;

void CutPackNameAndEmoticonFileName(char const* strPatch, std::string& kPackName, std::string& kFileName)
{
	std::string	strPack = "../Data/6_UI";
	std::string	strFile = strPatch;
	std::string strCutText = strFile.substr(strPack.size(), strFile.size() - strPack.size());

	kPackName = strPack + ".DAT";
	kFileName = "." + strCutText;
}

PgEmoticonFont::stEmoticonData::stEmoticonData(bool const bUsePack, int const iID, char const *strPath)
		:m_iID(iID),m_kPath(strPath),m_iTotalFrame(0),m_pkImageCache(NULL),m_pkImage(NULL)
{
	std::vector<char> kData;
	if( bUsePack )
	{
		std::string kPackName, kFileName;
		CutPackNameAndEmoticonFileName(strPath, kPackName, kFileName);
		BM::PgDataPackManager::LoadFromPack(UNI(kPackName), UNI(kFileName), kData);
		if( kData.size() )
		{
			m_pkImage = new CxImage((BYTE*)&kData.at(0), kData.size(), CXIMAGE_FORMAT_GIF);
		}
		else
		{
			return;
		}
	}
	else
	{
		m_pkImage  = new CxImage(UNI(strPath), CXIMAGE_FORMAT_GIF);
	}

	PG_ASSERT_LOG(m_pkImage);
	if (NULL == m_pkImage)
	{
		return;
	}

	if(false == m_pkImage->IsValid())
	{
		SAFE_DELETE(m_pkImage);
		return;
	}

	m_iTotalFrame = m_pkImage->GetNumFrames();

	if(0 >= m_iTotalFrame)
	{
		SAFE_DELETE(m_pkImage);
		return;
	}

	m_pkImageCache = new stImageCache * [m_iTotalFrame];

	memset(m_pkImageCache, 0, sizeof(stImageCache*) * m_iTotalFrame);

	if(1 < m_pkImage->GetNumFrames())
	{
		m_pkImage->SetRetreiveAllFrames(true);
		m_pkImage->SetFrame(m_pkImage->GetNumFrames() - 1);

		if( bUsePack )
		{
			if( kData.size() )
			{
				m_pkImage->Decode((BYTE*)&kData.at(0), kData.size(), CXIMAGE_FORMAT_GIF);
			}
		}
		else
		{
			m_pkImage->Load(UNI(strPath), CXIMAGE_FORMAT_GIF);
		}


		if(false == m_pkImage->IsValid())
		{
			SAFE_DELETE(m_pkImage);
			return;
		}
	}

}


PgEmoticonFont::PgEmoticonFont(std::wstring const& wFontKey, char const* strXMLPath)
:CXUI_Font(wFontKey)
{
	ParseXML(strXMLPath);
}
PgEmoticonFont::~PgEmoticonFont()
{
	ClearAllData();
}

PgEmoticonFont::stEmoticonData*	PgEmoticonFont::GetEmoticonData(int const iEmoticonID)	const
{
	EmoticonCont::const_iterator itor = m_kEmoticonCont.find(iEmoticonID);
	if(itor == m_kEmoticonCont.end())
	{
		return NULL;
	}

	return (*itor).second;
}

CxImage* PgEmoticonFont::GetEmoticonImage(int const iEmoticonID)	const
{
	EmoticonCont::const_iterator itor = m_kEmoticonCont.find(iEmoticonID);
	if(itor == m_kEmoticonCont.end())
	{
		return NULL;
	}

	if(NULL == (*itor).second)
	{
		return NULL;
	}

	return (*itor).second->m_pkImage;
}
bool PgEmoticonFont::IsNeedAnimation(TCHAR const code) const
{
	unsigned short sCode = (unsigned short)code;

	CxImage* pkImage = GetEmoticonImage(sCode);
	if(pkImage)
	{
		return pkImage->GetNumFrames() > 1;
	}

	return false;
}

int PgEmoticonFont::GetWidth(TCHAR const code) const
{
	unsigned short sCode = (unsigned short)code;

	CxImage* pkImage = GetEmoticonImage(sCode);
	if(pkImage)
	{
		return pkImage->GetWidth();
	}

	return 0;
}
int PgEmoticonFont::GetHeight(TCHAR const code) const
{
	unsigned short sCode = (unsigned short)code;

	CxImage* pkImage = GetEmoticonImage(sCode);
	if(pkImage)
	{
		return pkImage->GetHeight();
	}

	return 0;
}

int PgEmoticonFont::GetHeight() const
{
	return m_iFontHeight;
}

int PgEmoticonFont::MaxHeight(std::wstring const& text) const
{
	size_t iLen = text.length();
	int iHeight = GetHeight();
	int iMaxHeight = iHeight;
	for(size_t i = 0;i < iLen; ++i)
	{
		iHeight = GetHeight(text[i]);
		if(iHeight>iMaxHeight)
		{
			iMaxHeight = iHeight;
		}
	}

	return iMaxHeight;
}
PgEmoticonFont::stImageCache* PgEmoticonFont::GetCachedImage(stEmoticonData const* pkEmoticonData, int const iFrameNum)	const
{
	if(pkEmoticonData->m_iTotalFrame <= iFrameNum || iFrameNum < 0)
	{
		return NULL;
	}

	stImageCache* pkImageCache = *(pkEmoticonData->m_pkImageCache + iFrameNum);
	return pkImageCache;
}

PgEmoticonFont::stImageCache* PgEmoticonFont::CacheImage(stEmoticonData* pkEmoticonData,int const iFrameNum, CxImage *pkImage)
{
	if(pkEmoticonData->m_iTotalFrame <= iFrameNum || iFrameNum < 0)
	{
		return NULL;
	}

	stImageCache* pkImageCache = new stImageCache();
	PG_ASSERT_LOG(pkImageCache);
	
	SAFE_DELETE(*(pkEmoticonData->m_pkImageCache+iFrameNum));
	*(pkEmoticonData->m_pkImageCache+iFrameNum) = pkImageCache;

	int	iWidth = pkImage->GetWidth();
	int	iHeight = pkImage->GetHeight();

	int	iMaxBuffSize = iWidth*iHeight;

	SAFE_DELETE_ARRAY(pkImageCache->m_sColorBuffer);

	pkImageCache->m_sColorBuffer = new short[iMaxBuffSize];
	PG_ASSERT_LOG(pkImageCache->m_sColorBuffer);
	memset(pkImageCache->m_sColorBuffer, 0, sizeof(short) * iMaxBuffSize);

	pkImageCache->m_iWidth = iWidth;
	pkImageCache->m_iHeight = iHeight;

	short* pkBuff = pkImageCache->m_sColorBuffer;

	for(int i = 0;i < iHeight; ++i)
	{
		for(int j = 0;j < iWidth; ++j)
		{
			RGBQUAD	kRGBQuad = pkImage->GetPixelColor(j,iHeight-i-1);

			int	iBufLoc = i*iWidth+j;
			if(iBufLoc<iMaxBuffSize && iBufLoc>=0)
			{
				BYTE byAlpha = 0xff;
				if(pkImage->IsTransparent())
				{
					if(pkImage->GetTransIndex() == pkImage->GetPixelIndex(j,iHeight-i-1))
					{
						byAlpha = 0;
					}
				}
				short const sColor = ((byAlpha&0xf0)<<8) | ((kRGBQuad.rgbRed&0xf0)<<4) | (kRGBQuad.rgbGreen&0xf0) | (kRGBQuad.rgbBlue>>4);
				*(pkBuff+iBufLoc) = sColor;
			}
		}
	}

	return pkImageCache;
}

int PgEmoticonFont::Draw(_ARGB16* ptr, int const iMaxBuffSize, int const dx, int const dy, int const pitch, unsigned short const code,_ARGB16 const &color)
{
	stEmoticonData* pkEmoticonData = GetEmoticonData(code);
	if(!pkEmoticonData)
	{
		return 0;
	}

	CxImage* pkImage = pkEmoticonData->m_pkImage;
	if(!pkImage)
	{
		return 0;
	}

	int	iNumFrame = pkImage->GetNumFrames();
	int	iNowFrame = 0;

	std::vector<DWORD> kFrameDelayCont;
	kFrameDelayCont.reserve(iNumFrame);
	int const iDelaySpeed = 12;
	if(1 < iNumFrame)
	{	
		int	iTotalFrameLength = 0;

		for(int i = 0; i < iNumFrame; ++i)
		{
			CxImage* pkCurFrameImage = pkImage->GetFrame(i);
			if(pkCurFrameImage)
			{				
				DWORD dwFrameDelay = pkCurFrameImage->GetFrameDelay() * iDelaySpeed;
				if(0 >= dwFrameDelay)
				{
					dwFrameDelay = 9 * iDelaySpeed; // 프레임 딜레이 값이 없으면
				}

				kFrameDelayCont.push_back(dwFrameDelay); // Delay 순서대로 기록

				iTotalFrameLength += dwFrameDelay;
			}
		}

		if(0 >= iTotalFrameLength)
		{
			iTotalFrameLength = 1;
		}

		int	iNowTime =  BM::GetTime32() % iTotalFrameLength;
		
		iNowFrame = -1;

		if(!kFrameDelayCont.empty())
		{
			for(std::vector<DWORD>::const_iterator itor = kFrameDelayCont.begin(); itor != kFrameDelayCont.end(); ++itor)
			{
				iNowTime -= (*itor);
				++iNowFrame;
				if(0 >= iNowTime)
				{
					break;
				}
			}
		}
		else
		{
			iNowFrame = 0;
		}

		iNowFrame = NiClamp(iNowFrame, 0, iNumFrame);
	}

	stImageCache* pkCacheImage = GetCachedImage(pkEmoticonData,iNowFrame);
	if(!pkCacheImage)
	{
		if(pkImage->GetFrame(iNowFrame))
		{
			pkImage = pkImage->GetFrame(iNowFrame);
		}

		pkCacheImage = CacheImage(pkEmoticonData,iNowFrame,pkImage);
	}

	if (NULL == pkCacheImage || NULL == pkCacheImage->m_sColorBuffer)
	{
		return 0;
	}

	int	iWidth = pkCacheImage->m_iWidth;
	int	iHeight = pkCacheImage->m_iHeight;
	int const idy = std::max(m_iFontHeight - iHeight, 0); // 하단 정렬

	size_t const iOneRowSize = iWidth * sizeof(short);
	int const iMin = (idy + dy) * pitch + dx;
	int const iMax = (idy + dy + iHeight) * pitch + dx + iOneRowSize;
	if( 0 > iMin
	&&	iMaxBuffSize <= iMax )
	{
		return iWidth;
	}

	int	const iMaxHeight = (iMaxBuffSize - (dx+iWidth-1)) / (pitch + 1 - dy - idy);
	iHeight = std::min(iHeight,iMaxHeight);

	if(0 >= iHeight)
	{
		return iWidth;
	}

	for( int i = 0; iHeight > i; ++i) // 항상 0 ~ Height
	{
		memcpy(ptr+(i+dy+idy)*pitch+dx, pkCacheImage->m_sColorBuffer+i*iWidth, iOneRowSize);
	}

	return iWidth;
}

void	PgEmoticonFont::ParseXML(char const* strXMLPath)
{
	ClearAllData();

	TiXmlDocument kXmlDoc(strXMLPath);
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(strXMLPath)))
	{
		PgError1("Parse Failed [%s]", strXMLPath);
		return;
	}

	// Find Root
	TiXmlElement const* pkElement = kXmlDoc.FirstChildElement();

	if(pkElement)
	{
		TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();

		int	iFontHeight = 19;

		while(pkAttr)
		{
			char const* pcAttrName = pkAttr->Name();
			char const* pcAttrValue = pkAttr->Value();

			if(0 == strcmp(pcAttrName, "FONT_HEIGHT"))
			{
				iFontHeight = atoi(pcAttrValue);
			}
			else
			{
				PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
			}

			pkAttr = pkAttr->Next();
		}

		m_iFontHeight = iFontHeight;
	}


	pkElement = pkElement->FirstChildElement();
	while(pkElement)
	{
		char const* pcTagName = pkElement->Value();

		if(0 == strcmp(pcTagName,"EMOTICON"))
		{

			int	iID = 0;
			char strPath[512] = {0, };

			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();

			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "ID"))
				{
					iID = atoi(pcAttrValue);
				}
				else if(0 == strcmp(pcAttrName, "PATH"))
				{
					strcpy_s(strPath,512,pcAttrValue);
				}
				else
				{
					PgXmlError1(pkElement, "XmlParse: Incoreect Attr '%s'", pcAttrName);
				}

				pkAttr = pkAttr->Next();
			}

			stEmoticonData* pkEmoticonData = new stEmoticonData(g_bUsePackData, iID,strPath);
			m_kEmoticonCont.insert(std::make_pair(iID, pkEmoticonData));
		}

		pkElement = pkElement->NextSiblingElement();
	}
}
void PgEmoticonFont::ClearAllData()
{
	for(EmoticonCont::iterator itor = m_kEmoticonCont.begin(); itor != m_kEmoticonCont.end(); ++itor)
	{
		SAFE_DELETE((*itor).second);
	}

	m_kEmoticonCont.clear();
}