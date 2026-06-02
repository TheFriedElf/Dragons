#ifndef FREEDOM_DRAGONICA_CONTENTS_BALLOONEMOTICON_PGBALLOONEMOTICONDATA_H
#define FREEDOM_DRAGONICA_CONTENTS_BALLOONEMOTICON_PGBALLOONEMOTICONDATA_H

typedef enum eBalloonEmoticonLoopType
{
	BELT_ETERNITY	= -1,
	BELT_ONE		= 0,
}EBE_LOOP_TYPE;

typedef struct tagBalloonEmoticonFrameData
{
	tagBalloonEmoticonFrameData()
		: wFrameIdx(0)
		, fDelayTime(0.0f)
		, pScrTexture(0)
	{};
	~tagBalloonEmoticonFrameData()
	{
		pScrTexture = NULL;
	}
protected:
	CLASS_DECLARATION(WORD, wFrameIdx, FrameIdx);
	CLASS_DECLARATION(float, fDelayTime, DelayTime);
	CLASS_DECLARATION(NiScreenTexturePtr, pScrTexture, SrcTexture);
}PgBalloonEmoticonFrameData;

class PgBalloonEmoticonData
{
public:
	void AddFrame(PgBalloonEmoticonFrameData const& FrameInfo);
	PgBalloonEmoticonFrameData const* GetFrame(int const FrameIdx) const;
	
	int GetTotalFrame() const { return m_kFrameContainer.size(); };

	PgBalloonEmoticonData(void);
	~PgBalloonEmoticonData(void);

	typedef std::vector< PgBalloonEmoticonFrameData >	kFrameContainer;
protected:

	CLASS_DECLARATION_S(int, ID);
	CLASS_DECLARATION_S(int, U);
	CLASS_DECLARATION_S(int, LoopType);
	CLASS_DECLARATION_S(POINT2, FrameSize);
	CLASS_DECLARATION_S(std::string, ImagePath);

	kFrameContainer	m_kFrameContainer;
};

#endif // FREEDOM_DRAGONICA_CONTENTS_BALLOONEMOTICON_PGBALLOONEMOTICONDATA_H