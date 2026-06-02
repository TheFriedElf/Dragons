#ifndef FREEDOM_DRAGONICA_CONTENTS_IMAGEPEICE_PGSTITCHIMAGEMNG_H
#define FREEDOM_DRAGONICA_CONTENTS_IMAGEPEICE_PGSTITCHIMAGEMNG_H

//
extern size_t const iMaxStitchImageWndCount;
extern size_t const iMaxStitchTextWndCount;


//
typedef struct tagImagePeice
{
	std::wstring kImagePath;
	POINT3I kPos;
	POINT2 kSize;

	tagImagePeice(std::wstring const &rkImagePath, POINT3I const &rkPos, POINT2 const &rkSize)
	{
		kImagePath = rkImagePath;
		kPos = rkPos;
		kSize = rkSize;
	};
} SImagePeice;
typedef std::vector< SImagePeice > ContImagePeice;


typedef struct tagTextPeice
{
	int iTextID;
	POINT3I kPos;
	POINT2 kSize;
	std::wstring kFontName;
	DWORD dwFontOption; //EXUIFontFlag
	DWORD dwFontColor;
	DWORD dwOutlineColor;

	tagTextPeice(int const TextID, POINT3I const &rkPos, POINT2 const &rkSize, std::wstring const &rkFontName, DWORD const dwNewFontOption, DWORD const dwNewFontColor, DWORD const dwNewOutlineColor)
	{
		iTextID = TextID;
		kPos = rkPos;
		kSize = rkSize;
		kFontName = rkFontName;
		dwFontOption = dwNewFontOption;
		dwFontColor = dwNewFontColor;
		dwOutlineColor = dwNewOutlineColor;
	}
} STextPeice;
typedef std::vector< STextPeice > ContTextPeice;


//
typedef struct tagStitchImage
{
	std::wstring kID;
	POINT3I kPos;
	ContImagePeice kPeices;
	ContTextPeice kTexts;

	tagStitchImage()
	{
		kID.clear();
		kPos.Clear();
		kPeices.clear();
		kTexts.clear();
	}

	tagStitchImage(std::wstring const &rkID, POINT3I const &rkPos, ContImagePeice const &rkPeices, ContTextPeice const &rkTexts)
	{
		kID = rkID;
		kPos = rkPos;
		kPeices = rkPeices;
		kTexts = rkTexts;
	};
} SStitchImage;
typedef std::map< std::wstring, SStitchImage > ContStitchImage;


//
class PgStitchImageMng : public PgIXmlObject
{
public:
	PgStitchImageMng();
	virtual ~PgStitchImageMng();

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

	void Reload();
	bool Get(std::wstring const &rkID, SStitchImage &rkOut) const;

private:
	ContStitchImage m_kCont;
};


//
#define g_kStitchImageMng SINGLETON_CUSTOM(PgStitchImageMng, CreateUsingNiNew)

#endif // FREEDOM_DRAGONICA_CONTENTS_IMAGEPEICE_PGSTITCHIMAGEMNG_H