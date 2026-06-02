#ifndef FREEDOM_DRAGONICA_UI_PGTEXTBALLOON_H
#define FREEDOM_DRAGONICA_UI_PGTEXTBALLOON_H

#include "PgBalloon.h"

namespace BalloonUtil
{
	bool RegisterWrapper(lua_State *pkState);

	typedef enum eImagePiece
	{
		IP_LEFT_TOP = 0,
		IP_TOP = 1,
		IP_RIGHT_TOP,
		IP_LEFT,
		IP_CENTER,
		IP_RIGHT,
		IP_LEFT_BOTTOM,
		IP_BOTTOM,
		IP_RIGHT_BOTTOM,
		IP_TAIL,
	} EImagePiece;

	typedef struct tagBalloonImagePiece
	{
		tagBalloonImagePiece();
		tagBalloonImagePiece(float const fX, float const fY, float const fW, float const fH);
		tagBalloonImagePiece(tagBalloonImagePiece const& rhs);

		NiPoint2 kPos;
		NiPoint2 kSize;
	} SBalloonImagePiece;
	typedef std::map< EImagePiece, SBalloonImagePiece > CONT_BALLOON_IMAGE_PIECE;

	typedef struct tagBalloonImage
	{
		tagBalloonImage();
		tagBalloonImage(std::string const& rkPath);
		tagBalloonImage(tagBalloonImage const& rhs);

		bool Add(EImagePiece const eKey, SBalloonImagePiece const& rkNew);
		bool Build(bool const bPatternTop, bool const bPatternLeft);
		SBalloonImagePiece GetPiece(EImagePiece const eKey) const;
		bool IsHasPiece(EImagePiece const eKey) const;

		std::string kImagePath;
		int iMinimumWidth;
		int iMinimumHeight;
		int iPatternTopSize;
		int iPatternLeftSize;
		int iTailWidthScale;
		NiPoint2 kTextOffset;
		NiPoint2 kTextGap;
		NiPoint2 kNameOffset;
		CONT_BALLOON_IMAGE_PIECE kContPiece;
	} SBalloonImage;
	typedef std::map< int, SBalloonImage > CONT_BALLOON_IMAGE;

	//
	class PgBalloonImageMng : public PgIXmlObject
	{
	public:
		PgBalloonImageMng();
		~PgBalloonImageMng();

		void Clear();
		virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);
		SBalloonImage Get(int const iType) const;
		bool IsHave(int const iType) const;

	private:
		CONT_BALLOON_IMAGE m_kCont;
	};

};
#define g_kBalloonImageMng SINGLETON_STATIC(BalloonUtil::PgBalloonImageMng)

class PgToolTipBalloon
	:	public PgBalloon
{
public:
	PgToolTipBalloon(void);
	virtual ~PgToolTipBalloon(void);

	virtual E_BALLOON_TYPE GetType(void)const{return BALLOON_TOOLTIP;}
	virtual void DrawImmediate( PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime );
	virtual bool Is3D(void)const{return false;}

	virtual void SetExistTime( int const iExistTime );				//종료시간 임의 조절

	POINT	GetSize(void)const;
	
	void	SetTargetPos(POINT2	ptPos){m_ptTargetPos=ptPos;}
	void	SetTextFont(std::wstring const &kFontKey){m_kTextFont=kFontKey;}

	virtual void SetNewBalloon( EChatType const eChatType, std::wstring const &rkText, int iExistTime = 4000, bool bFake = false);

	POINT	GetMsgRenderTargetLoc(void)const;

protected:
	bool Draw( NiPoint2 const &ptTarget, NiPoint2 const &ptScreen, int const iBoxWidth, int const iBoxHeight, bool const bActor );
	bool SetChatType( int const iChatType = CT_NORMAL );
	void NewBalloon(std::wstring const &rkText, int iExistTime, bool bFake);
	void CalculateActualSize();
	virtual void OnChangeChatType() {}; // 하위 클래스로, ChatType 말풍선 타입이 변경됨을 알림

protected:

	std::wstring	m_kOriginalText;
	int				m_iExistTime;	//	0 = Infinity
	POINT2			m_ptTargetPos;	//	Used only when m_Type == T_TOOLTIP
	std::wstring	m_kTextFont;
	
	unsigned	long	m_ulSetTextTime;
	int	m_iActualTextWidth,m_iActualTextHeight;
	int m_iChatType;
	bool	m_bCurFake;			//현재 입력중 메시지 인가 아닌가?
	bool	m_bOldFake;			//이전에 입력중 메시지 였는가 아닌가?
	BalloonUtil::SBalloonImage m_kCurBalloonImageInfo;
};

class	PgTextBalloon
	:	public PgToolTipBalloon
{
public:
	PgTextBalloon(void);
	virtual	~PgTextBalloon(void);

	virtual E_BALLOON_TYPE GetType(void)const{return BALLOON_TEXT_ACTOR;}
	virtual void DrawImmediate( PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime );

	void	Init(PgActor *pActor);
	virtual void SetNewBalloon( EChatType const eChatType, std::wstring const &rkText, int iExistTime = 4000, bool bFake = false);

protected:
	void OnChangeChatType();

private:
	PgActor	*m_pActor;
};



class PgBalloonMan
{
public:
	typedef std::map< BM::GUID, PgBalloon* >	ContNode;

public:
	explicit PgBalloonMan( bool const bIs3D );
	~PgBalloonMan(void);

	template< typename T >
	bool CreateNode( T* &pOutPointer )
	{
		pOutPointer = new_tr T;
		if ( pOutPointer )
		{
			PgBalloon * pBallon = dynamic_cast<PgBalloon*>(pOutPointer);
			if ( pBallon )
			{
				if ( pBallon->Is3D() == m_bIs3D )
				{
					std::pair<ContNode::iterator, bool> kPair = m_kContNode.insert( std::make_pair( pBallon->GetID(), pBallon ) );
					if ( true == kPair.second )
					{
						return true;
					}
				}
			}
		}

		PG_ASSERT_LOG(pOutPointer && __FUNCTIONW__);
		SAFE_DELETE(pOutPointer);
		return false;
	}

	template< typename T >
	bool DestroyNode( T* &p )
	{
		if ( p )
		{
			if ( true == DestroyNode( p->GetID() ) )
			{
				p = NULL;
				return true;
			}
		}
		return false;
	}

	template< typename T >
	T* Get( BM::GUID const &kID )const
	{
		ContNode::const_iterator itr = m_kContNode.find( kID );
		if ( itr != m_kContNode.end() )
		{
			return dynamic_cast<T*>(itr->second);
		}
		return NULL;
	}

	bool DestroyNode( BM::GUID const &kID );

	void DrawImmediate( PgRenderer* pkRenderer, NiCameraPtr spCamera, float fFrameTime );
	void Clear(void);
	void Destroy(void);
	void BalloonOff();
	void Visible( bool bValue );
	bool IsVisible(void);
protected:
	void Init();

protected:
	ContNode	m_kContNode;
	bool		m_bIs3D;
	bool		m_bVisible;
private:// Not Use
	PgBalloonMan();
	PgBalloonMan( PgBalloonMan const & );

	PgBalloonMan& operator = ( PgBalloonMan const & );
};

extern PgBalloonMan g_kBalloonMan2D;
extern PgBalloonMan g_kBalloonMan3D;
#endif // FREEDOM_DRAGONICA_UI_PGTEXTBALLOON_H