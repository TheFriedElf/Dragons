#include "stdafx.h"
#include "XUI/XUI_Font.h"
#include "Variant/PgStringUtil.h"
#include "Variant/Global.h"
#include "PgRenderer.h"
#include "PgActor.h"
#include "PgNifMan.h"
#include "PgPilot.h"
#include "PgMath.h"
#include "Pg2DString.h"
#include "FreedomPool.H"
#include "PgWorld.H"
#include "PgCameraMan.H"
#include "PgOption.h"
#include "PgUIScene.H"

#include "PgPartyBalloon.h"
#include "PgTextBalloon.h"

std::string const kChatBallloonNormal = "../Data/6_UI/ChatBalloon/Balloon_BG.dds";
//std::string const kChatBallloonNormal = "../Data/6_UI/ChatBalloon/Balloon_img001.dds";
std::string const kChatBallloonParty = "../Data/6_UI/ChatBalloon/Balloon_BG_2.dds";

namespace BalloonUtil
{
	// enum
	int const iNameTextID = 2;

	//
	static int iForceBalloonID = 0;
	void SetBalloonForceID(int const iForceID)
	{
		iForceBalloonID = iForceID;
	}
	bool RegisterWrapper(lua_State *pkState)
	{
#ifndef EXTERNAL_RELEASE
		using namespace lua_tinker;
		def(pkState, "SetBalloonForceID", SetBalloonForceID);
#endif
		return true;
	}

	//
	typedef std::pair< EImagePiece, std::string > PgImagePiece;
	static PgImagePiece pkPieceArray[] = {	PgImagePiece(IP_LEFT_TOP, std::string("LEFT_TOP")),
											PgImagePiece(IP_TOP, std::string("TOP")),
											PgImagePiece(IP_RIGHT_TOP, std::string("RIGHT_TOP")),
											PgImagePiece(IP_LEFT, std::string("LEFT")),
											PgImagePiece(IP_CENTER, std::string("CENTER")),
											PgImagePiece(IP_RIGHT, std::string("RIGHT")),
											PgImagePiece(IP_LEFT_BOTTOM, std::string("LEFT_BOTTOM")),
											PgImagePiece(IP_BOTTOM, std::string("BOTTOM")),
											PgImagePiece(IP_RIGHT_BOTTOM, std::string("RIGHT_BOTTOM")),
											PgImagePiece(IP_TAIL, std::string("TAIL")) };
	
	bool StringToEnum(char const* szString, EImagePiece& eType)
	{
		if( szString )
		{
			PgImagePiece* pkBegin = pkPieceArray;
			while( PgArrayUtil::GetEndArray(pkPieceArray) != pkBegin )
			{
				if( (*pkBegin).second == szString )
				{
					eType = (*pkBegin).first;
					return true;
				}
				++pkBegin;
			}
		}
		return false;
	}


	//
	tagBalloonImagePiece::tagBalloonImagePiece()
		: kPos(0.f, 0.f), kSize(0.f, 0.f)
	{
	}
	tagBalloonImagePiece::tagBalloonImagePiece(float const fX, float const fY, float const fW, float const fH)
		: kPos(fX, fY), kSize(fW, fH)
	{
	}
	tagBalloonImagePiece::tagBalloonImagePiece(tagBalloonImagePiece const& rhs)
		: kPos(rhs.kPos), kSize(rhs.kSize)
	{
	}

	//
	tagBalloonImage::tagBalloonImage()
		: kImagePath(), iMinimumWidth(0), iMinimumHeight(0), iPatternTopSize(0), iPatternLeftSize(0), iTailWidthScale(0), kTextOffset(), kTextGap(), kNameOffset(), kContPiece()
	{
	}
	tagBalloonImage::tagBalloonImage(std::string const& rkPath)
		: kImagePath(rkPath), iMinimumWidth(0), iMinimumHeight(0), iPatternTopSize(0), iPatternLeftSize(0), iTailWidthScale(0), kTextOffset(), kTextGap(), kNameOffset(), kContPiece()
	{
	}
	tagBalloonImage::tagBalloonImage(tagBalloonImage const& rhs)
		: kImagePath(rhs.kImagePath), iMinimumWidth(rhs.iMinimumWidth), iMinimumHeight(rhs.iMinimumHeight)
		, iPatternTopSize(rhs.iPatternTopSize), iPatternLeftSize(rhs.iPatternLeftSize), iTailWidthScale(rhs.iTailWidthScale)
		, kTextOffset(rhs.kTextOffset), kTextGap(rhs.kTextGap), kNameOffset(rhs.kNameOffset), kContPiece(rhs.kContPiece)
	{
	}
	bool tagBalloonImage::Add(EImagePiece const eKey, SBalloonImagePiece const& rkNew)
	{
		std::pair<CONT_BALLOON_IMAGE_PIECE::iterator, bool> kRet = kContPiece.insert( std::make_pair(eKey, rkNew) );
		return kRet.second;
	}
	bool tagBalloonImage::Build(bool const bPatternTop, bool const bPatternLeft)
	{
		SBalloonImagePiece const kLeftTop( GetPiece(IP_LEFT_TOP) );
		SBalloonImagePiece const kRightTop( GetPiece(IP_RIGHT_TOP) );
		SBalloonImagePiece const kLeftBottom( GetPiece(IP_LEFT_BOTTOM) );
		SBalloonImagePiece const kRightBottom( GetPiece(IP_RIGHT_BOTTOM) );
		SBalloonImagePiece const kTop( GetPiece(IP_TOP) );
		SBalloonImagePiece const kLeft( GetPiece(IP_LEFT) );
		SBalloonImagePiece const kTail( GetPiece(IP_TAIL) );
		iMinimumWidth = static_cast< int >(kLeftTop.kSize.x + kRightTop.kSize.x);
		iMinimumHeight = static_cast< int >(kLeftTop.kSize.y + kLeftBottom.kSize.y);
		iTailWidthScale = static_cast< int >( kTail.kSize.x / kTop.kSize.x);
		if( bPatternTop )
		{
			iPatternTopSize = static_cast< int >(kTop.kSize.x);
		}
		if( bPatternLeft )
		{
			iPatternLeftSize = static_cast< int >(kLeft.kSize.y);
		}

		return	IsHasPiece( IP_LEFT_TOP )
			&&	IsHasPiece( IP_TOP )
			&&	IsHasPiece( IP_RIGHT_TOP )
			&&	IsHasPiece( IP_LEFT )
			&&	IsHasPiece( IP_CENTER )
			&&	IsHasPiece( IP_RIGHT )
			&&	IsHasPiece( IP_LEFT_BOTTOM )
			&&	IsHasPiece( IP_BOTTOM )
			&&	IsHasPiece( IP_RIGHT_BOTTOM )
			&&	IsHasPiece( IP_TAIL );
	}
	SBalloonImagePiece tagBalloonImage::GetPiece(EImagePiece const eKey) const
	{
		CONT_BALLOON_IMAGE_PIECE::const_iterator find_iter = kContPiece.find(eKey);
		if( kContPiece.end() != find_iter )
		{
			return (*find_iter).second;
		}
		return SBalloonImagePiece();
	}
	bool tagBalloonImage::IsHasPiece(EImagePiece const eKey) const
	{
		return kContPiece.end() != kContPiece.find(eKey);
	}

	//
	PgBalloonImageMng::PgBalloonImageMng()
		: m_kCont()
	{
	}
	PgBalloonImageMng::~PgBalloonImageMng()
	{
	}
	void PgBalloonImageMng::Clear()
	{
		m_kCont.clear();
	}
	bool PgBalloonImageMng::ParseXml(const TiXmlNode *pkNode, void *pArg, bool bUTF8)
	{
		if( pArg )
		{
			PG_ASSERT_LOG(0 && "pArg not use");
		}
		if( !pkNode )
		{
			return false;
		}
		char const* szName = pkNode->Value();
		if( 0 != strcmp(XML_ELEMENT_BALLOON_FORM, szName) )
		{
			return false;
		}

		typedef std::list< BM::vstring > CONT_ERROR_MSG;
		CONT_ERROR_MSG kContErrorMsg;

		TiXmlElement const* pkChildNode = pkNode->FirstChildElement();
		while( pkChildNode )
		{
			char const* szName = pkChildNode->Value();
			if( 0 == strcmp("BALLOON", szName) )
			{
				CONT_BALLOON_IMAGE::mapped_type kNewBalloonImage;

				int const iID = PgStringUtil::SafeAtoi(pkChildNode->Attribute("ID"));
				bool const bPaternTop = PgStringUtil::SafeAtob(pkChildNode->Attribute("PATTERN_TOP"));
				bool const bPaternLeft = PgStringUtil::SafeAtob(pkChildNode->Attribute("PATTERN_LEFT"));
				char const* szImagePath = pkChildNode->Attribute("IMG");
				if( szImagePath )
				{
					kNewBalloonImage.kImagePath = std::string(szImagePath);

					TiXmlElement const* pkTextNode = pkChildNode->FirstChildElement("TEXT_OFFSET");
					if( pkTextNode )
					{
						kNewBalloonImage.kTextOffset.x = PgStringUtil::SafeAtof(pkTextNode->Attribute("X"));
						kNewBalloonImage.kTextOffset.y = PgStringUtil::SafeAtof(pkTextNode->Attribute("Y"));
						kNewBalloonImage.kTextGap.x = PgStringUtil::SafeAtof(pkTextNode->Attribute("GAP_RIGHT"));
						kNewBalloonImage.kTextGap.y = PgStringUtil::SafeAtof(pkTextNode->Attribute("GAP_BOTTOM"));
					}
					TiXmlElement const* pkNameNode = pkChildNode->FirstChildElement("NAME_OFFSET");
					if( pkNameNode )
					{
						kNewBalloonImage.kNameOffset.x = PgStringUtil::SafeAtof(pkNameNode->Attribute("X"));
						kNewBalloonImage.kNameOffset.y = PgStringUtil::SafeAtof(pkNameNode->Attribute("Y"));
					}

					TiXmlElement const* pkPieceNode = pkChildNode->FirstChildElement("PIECE");
					while( pkPieceNode )
					{
						char const* szName = pkPieceNode->Attribute("NAME");
						float fX = PgStringUtil::SafeAtof(pkPieceNode->Attribute("X"));
						float fY = PgStringUtil::SafeAtof(pkPieceNode->Attribute("Y"));
						float fW = PgStringUtil::SafeAtof(pkPieceNode->Attribute("W"));
						float fH = PgStringUtil::SafeAtof(pkPieceNode->Attribute("H"));

						EImagePiece eType = IP_LEFT_TOP;
						if( StringToEnum(szName, eType) )
						{
							CONT_BALLOON_IMAGE_PIECE::mapped_type const kNewPiece(fX, fY, fW, fH);
							if( !kNewBalloonImage.Add(eType, kNewPiece) )
							{
								kContErrorMsg.push_back( BM::vstring() << __FL__ << L"duplicate piece name[" << szName << L"] ID[" << iID << L"]" );
							}
						}
						else
						{
							kContErrorMsg.push_back( BM::vstring() << __FL__ << L"wrong piece name[" << szName << L"] ID[" << iID << L"]" );
						}

						pkPieceNode = pkPieceNode->NextSiblingElement("PIECE");
					}

					bool const bAllPieceIsIn = kNewBalloonImage.Build(bPaternTop, bPaternLeft);

					if( bAllPieceIsIn )
					{
						std::pair<CONT_BALLOON_IMAGE::iterator, bool> kRet = m_kCont.insert( std::make_pair(iID, kNewBalloonImage) );
						if( !kRet.second )
						{
							kContErrorMsg.push_back( BM::vstring() << __FL__ << L"duplicate ID[" << iID << L"]" );
						}
					}
					else
					{
						kContErrorMsg.push_back( BM::vstring() << __FL__ << L"image all piece is not in, current count[" << kNewBalloonImage.kContPiece.size() << L"]" );
					}
				}
				else
				{
					kContErrorMsg.push_back( BM::vstring() << __FL__ << L"image path is NULL, ID[" << iID << L"]" );
				}
			}
			else
			{
				kContErrorMsg.push_back( BM::vstring() << __FL__ << L"Wrong element name[" << std::string(szName) << L"]" );
			}

			pkChildNode = pkChildNode->NextSiblingElement();
		}


		bool const bRet = kContErrorMsg.empty();
		{
			CONT_ERROR_MSG kTemp;
			kTemp.swap( kContErrorMsg );
#ifndef EXTERNAL_RELEASE
			CONT_ERROR_MSG::const_iterator iter = kTemp.begin();
			while( kTemp.end() != iter )
			{
				PgMessageBox( "BalloonForm.xml Error" ,MB((*iter)) );
				++iter;
			}
#endif
		}
		return bRet;
	}
	SBalloonImage PgBalloonImageMng::Get(int const iType) const
	{
		CONT_BALLOON_IMAGE::const_iterator find_iter = m_kCont.find(iType);
		if( m_kCont.end() != find_iter )
		{
			return (*find_iter).second;
		}
		int const iDefaultBalloon = 0; // 0은 항상 있어야 한다!
		return Get(iDefaultBalloon);
	}
	bool PgBalloonImageMng::IsHave(int const iType) const
	{
		return m_kCont.end() != m_kCont.find(iType);
	}

	//
	void AddNewScreenRects(NiScreenTexturePtr pTexture, NiPoint2 const& rkTarget2, NiPoint2 const& rkClippedAdjust, SBalloonImagePiece const& rkImgPiece)
	{
		if( pTexture )
		{
			pTexture->AddNewScreenRect(
				(short)rkTarget2.y,
				(short)rkTarget2.x,
				(unsigned short)rkImgPiece.kSize.x,
				(unsigned short)rkImgPiece.kSize.y,
				(unsigned short)(rkImgPiece.kPos.y + rkClippedAdjust.y),
				(unsigned short)(rkImgPiece.kPos.x + rkClippedAdjust.x) );
		}
	}
};

PgToolTipBalloon::PgToolTipBalloon(void)
:	m_iChatType(CT_NONE)
,	m_bCurFake(false)
,	m_bOldFake(false)
,	m_iExistTime(0)
,	m_ulSetTextTime(0)
,	m_iActualTextWidth(0)
,	m_iActualTextHeight(0)
,	m_kTextFont(FONT_CHAT)
,	m_kCurBalloonImageInfo()
{
}

PgToolTipBalloon::~PgToolTipBalloon(void)
{
}

POINT PgToolTipBalloon::GetMsgRenderTargetLoc(void)const
{
	BalloonUtil::SBalloonImagePiece const kLeftTopPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_LEFT_TOP) );
	NiPoint2 const kTempPos = kLeftTopPiece.kSize + m_kCurBalloonImageInfo.kTextOffset;
	POINT kLoc = m_ptTargetPos;
	kLoc.x += static_cast< int >(kTempPos.x);
	kLoc.y += static_cast< int >(kTempPos.y);
	return	kLoc;
}

POINT PgToolTipBalloon::GetSize(void)const
{
	POINT	ptSize;
	ptSize.x = m_iActualTextWidth + m_kCurBalloonImageInfo.iMinimumWidth;
	ptSize.y = m_iActualTextHeight + m_kCurBalloonImageInfo.iMinimumHeight;
	return ptSize;
}

void PgToolTipBalloon::SetExistTime( int const iExistTime )
{
	m_ulSetTextTime = BM::GetTime32();
	m_iExistTime = iExistTime;
}

bool PgToolTipBalloon::SetChatType( int const iChatType )
{
	if( m_iChatType != iChatType || !m_spScreenTexture )
	{
		m_iChatType = iChatType;
		m_spScreenTexture = NULL;//

		int const iDefaultBalloonID = 0;
		int const iPartyBalloonID = 1;
		int const iGuildBalloonID = 6;
		switch(iChatType)
		{
		case CT_PARTY:
			{
				m_kCurBalloonImageInfo = g_kBalloonImageMng.Get(iPartyBalloonID);
			}break;
		case CT_GUILD:
			{
				m_kCurBalloonImageInfo = g_kBalloonImageMng.Get(iGuildBalloonID);
			}break;
		case CT_NORMAL:
		default:
			{
				if( g_kBalloonImageMng.IsHave(iChatType) )
				{
					m_kCurBalloonImageInfo = g_kBalloonImageMng.Get(iChatType);
				}
				else
				{
					m_kCurBalloonImageInfo = g_kBalloonImageMng.Get(iDefaultBalloonID);
				}
			}break;
		}

		NiSourceTexturePtr pTexture = g_kNifMan.GetTexture(m_kCurBalloonImageInfo.kImagePath);
		if( pTexture )
		{
			m_spScreenTexture = NiNew NiScreenTexture(pTexture);

			OnChangeChatType();

			return true;
		}
	}
	return false;
}

void PgToolTipBalloon::SetNewBalloon( EChatType const eChatType, std::wstring const &rkText, int iExistTime, bool bFake )
{
	SetChatType( eChatType );//Refresh Texture
	NewBalloon(rkText, iExistTime, bFake);
}

void PgToolTipBalloon::NewBalloon(std::wstring const &rkText, int iExistTime, bool bFake)
{
	m_iExistTime = iExistTime;
	m_bOldFake = m_bCurFake;		//이전 상태 정보 저장
	m_bCurFake = bFake;				//현재 상태 정보 저장

	XUI::CXUI_Font	*pFont = g_kFontMgr.GetFont(m_kTextFont);
	assert(pFont);
	if( pFont )
	{
		bool bUseWordWrap = true;
		int	iTextWidthLimit = 256,	iTextHeightLimit = 256;
		DWORD const dwFontColor = COLOR_WHITE;

		switch( this->GetType() )
		{
		case BALLOON_TEXT_ACTOR:
			{
			}break;
		case BALLOON_TOOLTIP:
			{
				bUseWordWrap = false;
				iTextWidthLimit = 600;
				iTextHeightLimit = 600;
			}break;
		default:
			{
			}break;
		}

		BalloonUtil::SBalloonImagePiece const kLeftTop( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_LEFT_TOP) );
		NiPoint2 const kTempPos = kLeftTop.kSize + m_kCurBalloonImageInfo.kTextOffset;
		NiPoint3 kAdjustPos( kTempPos.x, kTempPos.y, 0.0f );
		NiColorA const kTextColor(1.f, 1.f, 1.f, 1.f);
		NiColorA const kTextShadow(0.f, 0.f, 0.f, 1.f);

		CXUI_Style_String kSrcStyleString;
		kSrcStyleString.CreateStyleString( XUI::PgFontDef( pFont, dwFontColor ), rkText );
		kSrcStyleString.SetOriginalString( rkText );

		Pg2DString *pkText = new Pg2DString( kSrcStyleString, true, false, iTextWidthLimit, bUseWordWrap );
		if ( pkText )
		{
			AddText( 1, pkText, kAdjustPos, kTextColor, kTextShadow, false, 1.0f );

			m_iActualTextWidth = static_cast< int >(pkText->GetSize().x + m_kCurBalloonImageInfo.kTextOffset.x);
			m_iActualTextHeight = static_cast< int >(pkText->GetSize().y + m_kCurBalloonImageInfo.kTextOffset.y);

			CalculateActualSize();

			m_ulSetTextTime = BM::GetTime32();
			SetEnable(true);
		}	
	}
}
void PgToolTipBalloon::CalculateActualSize()
{
	if( 0 < m_kCurBalloonImageInfo.iPatternTopSize
	&&	0 < m_iActualTextWidth % m_kCurBalloonImageInfo.iPatternTopSize )
	{
		int const iRemainWidth = m_iActualTextWidth % m_kCurBalloonImageInfo.iPatternTopSize;
		if( iRemainWidth <= m_kCurBalloonImageInfo.kTextGap.x )
		{
			m_iActualTextWidth -= iRemainWidth;
		}
		else
		{
			m_iActualTextWidth += m_kCurBalloonImageInfo.iPatternTopSize - iRemainWidth;
		}
		switch( m_kCurBalloonImageInfo.iTailWidthScale )
		{
		case 1: // Top : Tail width = 1 : 1 (항상 홀수를 유지)
			{
				if( 0 == (m_iActualTextWidth / m_kCurBalloonImageInfo.iPatternTopSize) % 2 )
				{
					m_iActualTextWidth += m_kCurBalloonImageInfo.iPatternTopSize;
				}
			}break;
		default:
			{
				// Top : Tail width = 1 : 2 OK
			}break;
		}
	}
	if( 0 < m_kCurBalloonImageInfo.iPatternLeftSize
	&&	0 < m_iActualTextHeight % m_kCurBalloonImageInfo.iPatternLeftSize )
	{
		int const iRemainHeight = m_iActualTextHeight % m_kCurBalloonImageInfo.iPatternLeftSize;
		if( iRemainHeight <= m_kCurBalloonImageInfo.kTextGap.y )
		{
			m_iActualTextHeight -= iRemainHeight;
		}
		else
		{
			m_iActualTextHeight += m_kCurBalloonImageInfo.iPatternLeftSize - iRemainHeight;
		}
	}

	if( 0 > m_iActualTextWidth )	{ m_iActualTextWidth = 0; }
	if( 0 > m_iActualTextHeight )	{ m_iActualTextHeight = 0; }
}

void PgToolTipBalloon::DrawImmediate( PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime )
{
	if ( !IsDrawImmediate() )
	{
		return;
	}

	NiPoint2 const ptTarget = NiPoint2( static_cast<float>(m_ptTargetPos.x), static_cast<float>(m_ptTargetPos.y) );

	//	0~1 초동안 박스를 확장시킨다.
	int	iElapsedTime = BM::GetTime32() - m_ulSetTextTime;
	float	fScale = 1.0f;
	if(iElapsedTime>0 && iElapsedTime<100 && !(m_bOldFake == true && m_bCurFake == false))
	{
		fScale = iElapsedTime/100.0f;
	}
	if( 0 < m_iExistTime )
	{
		// 3.9~4초 동안 박스를 축소시킨다.
		if(iElapsedTime>m_iExistTime-100 && iElapsedTime<m_iExistTime)
		{
			fScale = 1-(iElapsedTime-(m_iExistTime-100))/100.0f;
		}
		// 4초가 지나면 렌더링 종료
		if(iElapsedTime>=m_iExistTime)
		{
			SetEnable(false);
			m_bOldFake = m_bCurFake;
			m_bCurFake = false;
			RemoveText( 1 );
			return;
		}
	}
	else if( 0 > m_iExistTime )
	{
		fScale = 1.0f;
	}

	if( 0.0f < fScale )
	{
		int	const iBoxWidth = static_cast<int>( m_iActualTextWidth * fScale );
		int	const iBoxHeight = static_cast<int>( m_iActualTextHeight * fScale );

		NiPoint2 const ptScreen( static_cast<float>(pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0)), static_cast<float>(pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0)) );

		if ( true == Draw( ptTarget, ptScreen, iBoxWidth, iBoxHeight, false ) )
		{
			PgUIScene::Render_UIObject( pkRenderer, m_spScreenTexture );

			if ( 1.0f == fScale )
			{
				NiPoint3 const pt3Target( ptTarget.x, ptTarget.y, 0.0f );
				DrawImmediateText( pkRenderer, spCamera, pt3Target );
			}
		}
	}
}

bool PgToolTipBalloon::Draw( NiPoint2 const &ptTarget, NiPoint2 const &ptScreen, int const iBoxWidth, int const iBoxHeight, bool const bActor )
{
	if( ptTarget.x>=ptScreen.x
	||	ptTarget.y>=ptScreen.y
	||	0 > ptTarget.x+iBoxWidth+m_kCurBalloonImageInfo.iMinimumWidth
	||	0 > ptTarget.y+iBoxHeight+m_kCurBalloonImageInfo.iMinimumHeight )
	{
		return false;
	}

	m_spScreenTexture->RemoveAllScreenRects();

	NiPoint2 ptClippedAdjust,ptTarget2,ptClipLeftTop(0,0);

	//
	BalloonUtil::SBalloonImagePiece kLeftTopPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_LEFT_TOP) );
	BalloonUtil::SBalloonImagePiece kRightTopPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_RIGHT_TOP) );
	BalloonUtil::SBalloonImagePiece kLeftBottomPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_LEFT_BOTTOM) );
	BalloonUtil::SBalloonImagePiece kRightBottomPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_RIGHT_BOTTOM) );

	NiPoint2 const kOrgLeftTopSize( kLeftTopPiece.kSize );

	//	좌상단 모서리
	ptTarget2 = ptTarget;
	if(Get_Clipped_Region(ptTarget2,kLeftTopPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
	{
		BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kLeftTopPiece);
	}
	
	//	우상단 모서리
	ptTarget2 = NiPoint2(ptTarget.x+kRightTopPiece.kSize.x+iBoxWidth, ptTarget.y);
	if(Get_Clipped_Region(ptTarget2,kRightTopPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
	{
		BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kRightTopPiece);
	}
	
	//	좌하단 모서리
	ptTarget2 = NiPoint2(ptTarget.x, ptTarget.y+kLeftBottomPiece.kSize.y+iBoxHeight);
	if(Get_Clipped_Region(ptTarget2,kLeftBottomPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
	{
		BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kLeftBottomPiece);
	}
	
	//	우하단 모서리
	ptTarget2 = NiPoint2(ptTarget.x+kRightBottomPiece.kSize.x+iBoxWidth, ptTarget.y+kRightBottomPiece.kSize.y+iBoxHeight);
	if(Get_Clipped_Region(ptTarget2,kRightBottomPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
	{
		BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kRightBottomPiece);
	}
	
	//
	BalloonUtil::SBalloonImagePiece const kOrgTopPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_TOP) );
	BalloonUtil::SBalloonImagePiece const kOrgBottomPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_BOTTOM) );
	BalloonUtil::SBalloonImagePiece const kOrgTailPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_TAIL) );
	BalloonUtil::SBalloonImagePiece const kOrgLeftPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_LEFT) );
	BalloonUtil::SBalloonImagePiece const kOrgCenterPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_CENTER) );
	BalloonUtil::SBalloonImagePiece const kOrgRightPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_RIGHT) );

	//	상하단 라인
	int	iNum = iBoxWidth / static_cast< int >(kOrgTopPiece.kSize.x);
	int	iRemain = iBoxWidth % static_cast< int >(kOrgTopPiece.kSize.x);
	int	iArrow = iNum / 2;	//	꼬리표 들어갈 곳
	switch( m_kCurBalloonImageInfo.iTailWidthScale )
	{
	case 2: // 꼬리가 2배 일 때
		{
			if( 0 < m_kCurBalloonImageInfo.iPatternTopSize
			&&	0 == (iNum%2) )
			{
				iArrow -= 1;
			}
		}break;
	default:
		{
			if( iNum == 2 )	{ iArrow = 0; }
		}break;
	}

	for(int i=0; i<iNum; ++i)
	{
		{
			BalloonUtil::SBalloonImagePiece kTopPiece( kOrgTopPiece );
			ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+i*kTopPiece.kSize.x, ptTarget.y);
			if(Get_Clipped_Region(ptTarget2,kTopPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
			{
				BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kTopPiece);
			}
		}

		if( bActor && (i==iArrow && iBoxWidth>=kOrgTailPiece.kSize.x))
		{
			BalloonUtil::SBalloonImagePiece kTailPiece( kOrgTailPiece );
			ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+i*kOrgTopPiece.kSize.x, ptTarget.y+kOrgLeftTopSize.y+iBoxHeight);
			if(Get_Clipped_Region(ptTarget2,kTailPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
			{
				BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kTailPiece);
			}

			switch( m_kCurBalloonImageInfo.iTailWidthScale )
			{
			case 2: // 꼬리가 2배 일 때 (상단을 하나 더 그린다)
				{
					++i;
					if( i<iNum )
					{
						BalloonUtil::SBalloonImagePiece kTopPiece( kOrgTopPiece );
						ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+i*kTopPiece.kSize.x, ptTarget.y);
						if(Get_Clipped_Region(ptTarget2,kTopPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
						{
							BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kTopPiece);
						}
					}
				}break;
			default:
				{
				}break;
			}
		}
		else
		{
			BalloonUtil::SBalloonImagePiece kBottomPiece( kOrgBottomPiece );
			ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+i*kBottomPiece.kSize.x, ptTarget.y+kOrgLeftTopSize.y+iBoxHeight);
			if(Get_Clipped_Region(ptTarget2,kBottomPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
			{
				BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kBottomPiece);
			}
		}
	}

	if(iRemain>0)
	{
		BalloonUtil::SBalloonImagePiece kTopPiece( kOrgTopPiece );
		ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+kTopPiece.kSize.x*iNum, ptTarget.y);
		kTopPiece.kSize.x = static_cast< float >(iRemain);
		if(Get_Clipped_Region(ptTarget2,kTopPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
		{
			BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kTopPiece);
		}

		BalloonUtil::SBalloonImagePiece kBottomPiece( kOrgBottomPiece );
		ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+kBottomPiece.kSize.x*iNum, ptTarget.y+kOrgLeftTopSize.y+iBoxHeight);
		kBottomPiece.kSize.x = static_cast< float >(iRemain);
		if(Get_Clipped_Region(ptTarget2,kBottomPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
		{
			BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kBottomPiece);
		}
	}

	//	좌우 라인
	iNum = iBoxHeight / static_cast< int >(kOrgLeftPiece.kSize.y);
	iRemain = iBoxHeight % static_cast< int >(kOrgLeftPiece.kSize.y);
	for(int i=0; i<iNum; ++i)
	{
		BalloonUtil::SBalloonImagePiece kLeftPiece( kOrgLeftPiece );
		ptTarget2 = NiPoint2(ptTarget.x, ptTarget.y+kOrgLeftTopSize.y+i*kLeftPiece.kSize.y);
		if(Get_Clipped_Region(ptTarget2,kLeftPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
		{
			BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kLeftPiece);
		}

		BalloonUtil::SBalloonImagePiece kRightPiece( kOrgRightPiece );
		ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+iBoxWidth, ptTarget.y+kOrgLeftTopSize.y+i*kRightPiece.kSize.y);
		if(Get_Clipped_Region(ptTarget2,kRightPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
		{
			BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kRightPiece);
		}
	}

	if(iRemain>0)
	{
		BalloonUtil::SBalloonImagePiece kLeftPiece( kOrgLeftPiece );
		ptTarget2 = NiPoint2(ptTarget.x, ptTarget.y+kOrgLeftTopSize.y+iNum*kLeftPiece.kSize.y);
		kLeftPiece.kSize.y = static_cast< float >(iRemain);
		if(Get_Clipped_Region(ptTarget2,kLeftPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
		{
			BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kLeftPiece);
		}
		
		BalloonUtil::SBalloonImagePiece kRightPiece( kOrgRightPiece );
		ptTarget2 = NiPoint2( ptTarget.x+kOrgLeftTopSize.x+iBoxWidth, ptTarget.y+kOrgLeftTopSize.y+iNum*kRightPiece.kSize.y);
		kRightPiece.kSize.y = static_cast< float >(iRemain);
		if(Get_Clipped_Region(ptTarget2,kRightPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
		{
			BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kRightPiece);
		}
	}

	//	가운데 
	int	iHorizCount = iBoxWidth / static_cast< int >(kOrgCenterPiece.kSize.x);
	int	iHorizRemain = iBoxWidth % static_cast< int >(kOrgCenterPiece.kSize.x);
	int	iVertCount = iBoxHeight / static_cast< int >(kOrgCenterPiece.kSize.y);
	int	iVertRemain = iBoxHeight % static_cast< int >(kOrgCenterPiece.kSize.y);

	for(int i=0;i<iVertCount;i++)
	{
		for(int j=0;j<iHorizCount;j++)
		{
			BalloonUtil::SBalloonImagePiece kCenterPiece( kOrgCenterPiece );
			ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+j*kCenterPiece.kSize.x, ptTarget.y+kOrgLeftTopSize.y+i*kCenterPiece.kSize.y);
			if(Get_Clipped_Region(ptTarget2,kCenterPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
			{
				BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kCenterPiece);
			}
		}
	}

	if(iHorizRemain>0)
	{
		for(int i=0;i<iVertCount;i++)
		{
			BalloonUtil::SBalloonImagePiece kCenterPiece( kOrgCenterPiece );
			ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+iHorizCount*kCenterPiece.kSize.x, ptTarget.y+kOrgLeftTopSize.y+i*kCenterPiece.kSize.y);
			kCenterPiece.kSize.x = static_cast< float >( iHorizRemain );
			if(Get_Clipped_Region(ptTarget2,kCenterPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
			{
				BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kCenterPiece);
			}
		}
	}

	if(iVertRemain>0)
	{
		for(int i=0;i<iHorizCount;i++)
		{
			BalloonUtil::SBalloonImagePiece kCenterPiece( kOrgCenterPiece );
			ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+i*kCenterPiece.kSize.x, ptTarget.y+kOrgLeftTopSize.y+iVertCount*kCenterPiece.kSize.y);
			kCenterPiece.kSize.y = static_cast< float >(iVertRemain);
			if(Get_Clipped_Region(ptTarget2,kCenterPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
			{
				BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kCenterPiece);
			}
		}
	}

	if(iHorizRemain>0 && iVertRemain>0)
	{
		BalloonUtil::SBalloonImagePiece kCenterPiece( kOrgCenterPiece );
		ptTarget2 = NiPoint2(ptTarget.x+kOrgLeftTopSize.x+iHorizCount*kCenterPiece.kSize.x, ptTarget.y+kOrgLeftTopSize.y+iVertCount*kCenterPiece.kSize.y);
		kCenterPiece.kSize.x = static_cast< float >(iHorizRemain);
		kCenterPiece.kSize.y = static_cast< float >(iVertRemain);
		if(Get_Clipped_Region(ptTarget2,kCenterPiece.kSize,ptClippedAdjust,ptClipLeftTop,ptScreen))
		{
			BalloonUtil::AddNewScreenRects(m_spScreenTexture, ptTarget2, ptClippedAdjust, kCenterPiece);
		}
	}

	m_spScreenTexture->MarkAsChanged(NiScreenTexture::EVERYTHING_MASK);
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//	class	PgTextBalloon
////////////////////////////////////////////////////////////////////////////////////////////////////

PgTextBalloon::PgTextBalloon()
:	m_pActor(NULL)
{
}

PgTextBalloon::~PgTextBalloon()
{
}

void PgTextBalloon::Init(PgActor *pActor)
{
	m_pActor = pActor;
}
void PgTextBalloon::SetNewBalloon( EChatType const eChatType, std::wstring const &rkText, int iExistTime, bool bFake)
{
	int iNewChatType = eChatType;
	if( (eChatType == CT_NORMAL && m_pActor) )
	{
		if( 0 != BalloonUtil::iForceBalloonID )
		{
			iNewChatType = BalloonUtil::iForceBalloonID;
		}
		else
		{
			// 장착한 아이템에 따른 말풍선 타입 변경
			PgPilot* pkPilot = m_pActor->GetPilot();
			if( pkPilot
			&&	pkPilot->GetAbil(AT_CHAT_BALLOOON_TYPE) )
			{
				iNewChatType = pkPilot->GetAbil(AT_CHAT_BALLOOON_TYPE);
			}
		}
	}
	SetChatType( iNewChatType );//Refresh Texture

	PgToolTipBalloon::NewBalloon(rkText, iExistTime, bFake);

	Pg2DString* pkNameText = GetText(BalloonUtil::iNameTextID);
	if( m_pActor
	&&	pkNameText )
	{
		if( pkNameText->GetSize().x > m_iActualTextWidth )
		{
			m_iActualTextWidth = pkNameText->GetSize().x; // Actor에 붙는 말풍선은 항상, 이름크기보다 커야 한다
			CalculateActualSize();
		}
	}
}
void PgTextBalloon::OnChangeChatType()
{
	if( !m_pActor )
	{
		return;
	}

	RemoveText(BalloonUtil::iNameTextID);

	PgPilot *pkPilot = m_pActor->GetPilot();
	if ( pkPilot )
	{
		XUI::CXUI_Font *pFont = g_kFontMgr.GetFont(m_kTextFont);
		assert(pFont);
		if( pFont )
		{
			g_kFontMgr.SetFont(m_kTextFont);

			NiColorA const kNameColor(1.f, 1.f, 0.f, 1.f);
			NiColorA const kNameShadow(0.f, 0.f, 0.f, 1.f);

			std::wstring wstrName = pkPilot->GetName();
			PgStringUtil::EraseStr<std::wstring>( wstrName, _T("\n"), wstrName );

			Pg2DString *pkText = new Pg2DString( XUI::PgFontDef(pFont,COLOR_WHITE,CXUI_Font::FS_BOLD), wstrName );
			if( pkText )
			{
				float const fFontHeight = static_cast<float>(pFont->GetHeight()) + 3.0f;
				BalloonUtil::SBalloonImagePiece const kLeftTopPiece( m_kCurBalloonImageInfo.GetPiece(BalloonUtil::IP_LEFT_TOP) );
				NiPoint2 const kNamePosTemp = kLeftTopPiece.kSize + m_kCurBalloonImageInfo.kNameOffset;
				NiPoint3 kAdjustPos( kNamePosTemp.x, kNamePosTemp.y - fFontHeight, 0.0f );
				AddText( BalloonUtil::iNameTextID, pkText, kAdjustPos, kNameColor, kNameShadow, true, 1.0f );
			}
		}
	}
}

void PgTextBalloon::DrawImmediate(PgRenderer *pkRenderer, NiCameraPtr spCamera, float fFrameTime)
{
	if ( !IsDrawImmediate() )
	{
		return;
	}

	if ( NULL == m_pActor )
	{
		return;
	}
	
	NiPoint3 const kLengthPos = m_pActor->GetPos() - spCamera->GetTranslate();
	if( 1000.f < kLengthPos.Length() )
	{
		return;
	}

	//NiNodePtr spDummy = (NiNode*)m_pActor->GetObjectByName(ATTACH_POINT_STAR);
	NiNodePtr spDummy = m_pActor->GetNodePointStar();
	if( NULL == spDummy )
	{
		return;
	}

	float	fBx,fBy;
	if ( !spCamera->WorldPtToScreenPt(spDummy->GetWorldTranslate(),fBx,fBy) )
	{
		return;
	}

	NiPoint2 const ptScreen( static_cast<float>(pkRenderer->GetDefaultRenderTargetGroup()->GetWidth(0)), static_cast<float>(pkRenderer->GetDefaultRenderTargetGroup()->GetHeight(0)) );
	NiPoint2 ptTarget( static_cast<float>(ptScreen.x) * fBx, static_cast<float>(ptScreen.y) * (1-fBy) );

	//	0~1 초동안 박스를 확장시킨다.
	int	iElapsedTime = BM::GetTime32() - m_ulSetTextTime;
	float fScale = 1.0f;
	if(iElapsedTime>0 && iElapsedTime<100 && !(m_bOldFake == true && m_bCurFake == false))
	{
		fScale = iElapsedTime/100.0f;
	}
	if( 0 < m_iExistTime )
	{
		// 3.9~4초 동안 박스를 축소시킨다.
		if(iElapsedTime>m_iExistTime-100 && iElapsedTime<m_iExistTime)
		{
			fScale = 1-(iElapsedTime-(m_iExistTime-100))/100.0f;
		}
		// 4초가 지나면 렌더링 종료
		if(iElapsedTime>=m_iExistTime)
		{
			SetEnable(false);
			m_bOldFake = m_bCurFake;
			m_bCurFake = false;
			RemoveText(1);
			return;
		}
	}
	else if( 0 > m_iExistTime )
	{
		fScale = 1.0f;
	}

	if( 0.0f < fScale )
	{
		int	const iBoxWidth = static_cast<int>( m_iActualTextWidth * fScale );
		int	const iBoxHeight = static_cast<int>( m_iActualTextHeight * fScale );

		ptTarget.x -= ( iBoxWidth + m_kCurBalloonImageInfo.iMinimumWidth ) / 2;
		ptTarget.y -= ( iBoxHeight + m_kCurBalloonImageInfo.iMinimumHeight );

		if ( true == Draw( ptTarget, ptScreen, iBoxWidth, iBoxHeight, true ) )
		{
			PgUIScene::Render_UIObject(pkRenderer,m_spScreenTexture);

			if ( 1.0f == fScale )
			{
				NiPoint3 const pt3Target( ptTarget.x, ptTarget.y, 0.0f );
				DrawImmediateText( pkRenderer, spCamera, pt3Target );
			}
		}
	}
}

PgBalloonMan g_kBalloonMan2D(false);
PgBalloonMan g_kBalloonMan3D(true);

PgBalloonMan::PgBalloonMan( bool const bIs3D )
:	m_bIs3D(bIs3D)
{
	m_bVisible = true;
}

PgBalloonMan::~PgBalloonMan(void)
{
	Clear();
}

void PgBalloonMan::Init()
{
	//미리 로딩
	g_kNifMan.GetTexture(kChatBallloonNormal);
	g_kNifMan.GetTexture(kChatBallloonParty);
}

bool PgBalloonMan::DestroyNode( BM::GUID const &kID )
{
	ContNode::iterator itr = m_kContNode.find( kID );
	if ( itr != m_kContNode.end() )
	{
		SAFE_DELETE( itr->second );
		m_kContNode.erase( itr );
		return true;
	}
	return false;
}

void PgBalloonMan::DrawImmediate( PgRenderer* pkRenderer, NiCameraPtr spCamera, float fFrameTime )
{
	if ( m_bVisible && spCamera )
	{
		ContNode::iterator itr = m_kContNode.begin();
		for ( ; itr != m_kContNode.end() ; ++itr )
		{
			itr->second->DrawImmediate( pkRenderer, spCamera, fFrameTime );
		}
	}
}

void PgBalloonMan::Clear(void)
{
	ContNode::iterator itr = m_kContNode.begin();
	for ( ; itr != m_kContNode.end() ; ++itr )
	{
		SAFE_DELETE( itr->second );
	}
	m_kContNode.clear();
}

void PgBalloonMan::Destroy(void)
{
	Clear();
}

void PgBalloonMan::BalloonOff()
{
	PgToolTipBalloon *pkToolTipBallon = NULL;

	ContNode::iterator itr = m_kContNode.begin();
	for ( ; itr != m_kContNode.end() ; ++itr )
	{
		itr->second->SetExistTime( 10 );
	}
}

void PgBalloonMan::Visible( bool bValue )
{
	m_bVisible = bValue;
}

bool PgBalloonMan::IsVisible()
{
	return m_bVisible;
}