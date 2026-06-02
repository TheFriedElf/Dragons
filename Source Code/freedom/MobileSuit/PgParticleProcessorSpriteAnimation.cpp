#include "stdafx.h"
#include "PgParticleProcessorSpriteAnimation.H"
#include "PgParticle.H"
#include "PgNifMan.h"

NiAlphaProperty::AlphaFunction TransAlphaFunctionFromString(std::string strFunctionName)
{
	if("ALPHA_ONE" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_ONE;
	}
	else if("ALPHA_ZERO" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_ZERO;
	}
	else if("ALPHA_SRCCOLOR" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_SRCCOLOR;
	}
	else if("ALPHA_INVSRCCOLOR" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_INVSRCCOLOR;
	}
	else if("ALPHA_DESTCOLOR" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_DESTCOLOR;
	}
	else if("ALPHA_INVDESTCOLOR" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_INVDESTCOLOR;
	}
	else if("ALPHA_SRCALPHA" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_SRCALPHA;
	}
	else if("ALPHA_INVSRCALPHA" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_INVSRCALPHA;
	}
	else if("ALPHA_DESTALPHA" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_DESTALPHA;
	}
	else if("ALPHA_INVDESTALPHA" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_INVDESTALPHA;
	}
	else if("ALPHA_SRCALPHASAT" == strFunctionName)
	{
		return NiAlphaProperty::ALPHA_SRCALPHASAT;
	}

	return NiAlphaProperty::ALPHA_SRCCOLOR;
}

NiImplementRTTI(PgParticleProcessorSpriteAnimation, PgParticleProcessor);

PgParticleProcessorSpriteAnimation::PgParticleProcessorSpriteAnimation() :
	m_bPlay(false),
	m_bLoop(false),
	m_iLoopCount(0),
	m_iCurLoopCount(0),
	m_fPlayRate(1.0f),
	m_fFrameStartTime(0.0f),
	m_iCurFrame(0),
	m_iUSubdivision(0),
	m_iVSubdivision(0),
	m_iMaxFrame(0),
	m_fUSize(0.0f),
	m_fVSize(0.0f),
	m_fTexWidth(0.0f),
	m_fTexHeight(0.0f),
	m_fDefaultDelay(0.0f),
	m_fStartDelay(0.0f),
	m_bStartDelayRandom(false),
	m_eSrcFunc(NiAlphaProperty::ALPHA_SRCCOLOR),
	m_eDestFunc(NiAlphaProperty::ALPHA_INVSRCALPHA),
	m_bAlphaTesting(false),
	m_byAlphaRef(0)
{
}

PgParticleProcessorSpriteAnimation::PgParticleProcessorSpriteAnimation(NiAVObject* pkAttachNode)  :
	//m_spSpriteNode SpriteAnimation이 붙어있는 PgParticle Node이다. //외부에서 받아와 PgParticle 데이터에 접근하기 위해서 저장해둠
	m_spSpriteNode(NiDynamicCast(NiNode, pkAttachNode)),
	m_bPlay(false),
	m_bLoop(false),
	m_iLoopCount(0),
	m_iCurLoopCount(0),
	m_fPlayRate(1.0f),
	m_fFrameStartTime(0.0f),
	m_iCurFrame(0),
	m_iUSubdivision(0),
	m_iVSubdivision(0),
	m_iMaxFrame(0),
	m_fUSize(0.0f),
	m_fVSize(0.0f),
	m_fTexWidth(0.0f),
	m_fTexHeight(0.0f),
	m_fDefaultDelay(0.0f),
	m_fStartDelay(0.0f),
	m_bStartDelayRandom(false),
	m_eSrcFunc(NiAlphaProperty::ALPHA_SRCCOLOR),
	m_eDestFunc(NiAlphaProperty::ALPHA_INVSRCALPHA),
	m_bAlphaTesting(false),
	m_byAlphaRef(0)
{
}

PgParticleProcessorSpriteAnimation::~PgParticleProcessorSpriteAnimation()
{
}

bool	PgParticleProcessorSpriteAnimation::DoProcess(PgParticle* pkParticle,float fAccumTime,float fFrameTime)
{
	// m_spSpriteNode이 pkParticle과 같다. 생성자에서 인자로 받아 저장해둔다.
	if(!pkParticle)
	{
		return	false;
	}

	if(0.0f >= m_fFrameStartTime)
	{// 처음 시작
		m_fFrameStartTime = fAccumTime;
		
		if(0.0f < m_fStartDelay)
		{
			int iDelay = m_fStartDelay * 1000;
			if(m_bStartDelayRandom)
			{
				iDelay = BM::Rand_Range(iDelay, 0);
			}

			m_fFrameStartTime += static_cast<float>(iDelay) / 1000.0f;
		}
		m_kRandPosOpt.kOriginPos = pkParticle->GetTranslate();	//최초 위치점 기억
		m_kRandPosOpt.fElapsTIme = m_fFrameStartTime;	// 여기서 ElapsTime은 시작시 누적시간을 담는 용도로 사용된다
	}

	bool bUpdate = false;

	if(m_bPlay)
	{
		if(m_kFrameInfo.empty())
		{
			return true;
		}

		SpriteFrameInfo& rkFrameInfo = m_kFrameInfo[m_iCurFrame];

		if(m_fFrameStartTime + rkFrameInfo.m_fDelay < fAccumTime)
		{
			++m_iCurFrame;
			bUpdate = true;
			m_fFrameStartTime = fAccumTime;

		}

		int const iMaxSize = static_cast<int>(m_kFrameInfo.size());

		if(iMaxSize <= m_iCurFrame)
		{// 정해진 프레임 까지 애니메이션이 완료 되었고
			if(m_bLoop)
			{// 반복 되어야 한다면
				if(0 < m_iLoopCount)
				{// LoopCount가 있으면 체크 하고
					if(m_iLoopCount <= m_iCurLoopCount)
					{
						return false;
					}

					++m_iCurFrame;
				}
				m_iCurFrame = 0;
				
				if(SRandomOffsetPosOption::E_EACH_LOOP == m_kRandPosOpt.eType)
				{// Loop 완료시 파티클 랜덤 위치 설정이 존재 한다면 xml에서 얻어온 정보로 위치를 다시 설정한다
					NiPoint3 const& kMin = m_kRandPosOpt.kMin;
					NiPoint3 const& kMax = m_kRandPosOpt.kMax;

					NiPoint3 kRandPos(static_cast<float>(BM::Rand_Range(static_cast<int>(kMax.x), static_cast<int>(kMin.x)))
						,static_cast<float>(BM::Rand_Range(static_cast<int>(kMax.y), static_cast<int>(kMin.y)))
						,static_cast<float>(BM::Rand_Range(static_cast<int>(kMax.z), static_cast<int>(kMin.z)))
						);
					NiPoint3 kPos = m_kRandPosOpt.kOriginPos + kRandPos;
					pkParticle->SetTranslate(kPos);
				}
			}
			else
			{
				m_iCurFrame = iMaxSize - 1;
			}

			bUpdate = true;
		}	

		{
			if(SRandomOffsetPosOption::E_INTER_TIME == m_kRandPosOpt.eType	// 시간에 따라 랜덤으로 이동하게 되고
				&& 0 < m_kRandPosOpt.fIntervalTIme
				&& m_kRandPosOpt.fIntervalTIme+m_kRandPosOpt.fElapsTIme < fAccumTime)
			{// 시작에 딜레이가 들어가는 경우가 있으므로 시작 시간을 더해서 계산한다
				NiPoint3 const& kMin = m_kRandPosOpt.kMin;
				NiPoint3 const& kMax = m_kRandPosOpt.kMax;

				NiPoint3 kRandPos(static_cast<float>(BM::Rand_Range(static_cast<int>(kMax.x), static_cast<int>(kMin.x)))
					,static_cast<float>(BM::Rand_Range(static_cast<int>(kMax.y), static_cast<int>(kMin.y)))
					,static_cast<float>(BM::Rand_Range(static_cast<int>(kMax.z), static_cast<int>(kMin.z)))
					);
				NiPoint3 kPos = m_kRandPosOpt.kOriginPos + kRandPos;
				pkParticle->SetTranslate(kPos);
				m_kRandPosOpt.fElapsTIme = fAccumTime; // 여기서 ElapsTime은 현재 누적시간을 담는 용도로 사용된다
			}
		}

		if(bUpdate)
		{
			if(!m_spSpriteNode)
			{
				return false;
			}

			NiGeometry* pkGeom = NiDynamicCast(NiGeometry, m_spSpriteNode->GetObjectByName(m_kGeometryName.c_str()));
			if(!pkGeom)
			{
				return false;
			}

			NiGeometryData* pkGeomData = pkGeom->GetModelData();
			if(!pkGeomData)
			{
				return false;
			}

			NiPoint2* pkTexCoord = pkGeomData->GetTextures();
			if(!pkTexCoord)
			{
				return false;
			}

			float tx = static_cast<float>(static_cast<int>(m_fUSize * (m_iCurFrame % m_iUSubdivision)));
			float ty = static_cast<float>(static_cast<int>(m_fVSize * (m_iCurFrame / m_iVSubdivision)));
			float tw = tx + m_fUSize;
			float th = ty + m_fVSize;

			pkTexCoord->x = tx/m_fTexWidth;
			pkTexCoord->y = ty/m_fTexHeight;

			++pkTexCoord;

			pkTexCoord->x = tx/m_fTexWidth;
			pkTexCoord->y = th/m_fTexHeight;
			
			++pkTexCoord;

			pkTexCoord->x = tw/m_fTexWidth;
			pkTexCoord->y = ty/m_fTexHeight;
			
			++pkTexCoord;

			pkTexCoord->x = tw/m_fTexWidth;
			pkTexCoord->y = th/m_fTexHeight;

			pkGeomData->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
			pkGeom->SetAppCulled(false);
			pkGeom->UpdateProperties();	
		}
	}

	return	true;
}

NiImplementCreateClone(PgParticleProcessorSpriteAnimation);

void PgParticleProcessorSpriteAnimation::CopyMembers(PgParticleProcessorSpriteAnimation* pDest, NiCloningProcess& kCloning)
{
	PgParticleProcessor::CopyMembers(pDest,kCloning);

	pDest->m_bPlay = m_bPlay;
	pDest->m_bLoop = m_bLoop;
	pDest->m_iLoopCount = m_iLoopCount;
	pDest->m_iCurLoopCount = m_iCurLoopCount;
	pDest->m_fPlayRate = m_fPlayRate;
	pDest->m_fFrameStartTime = m_fFrameStartTime;
	pDest->m_iCurFrame = m_iCurFrame;
	pDest->m_iUSubdivision = m_iUSubdivision;
	pDest->m_iVSubdivision = m_iVSubdivision;
	pDest->m_iMaxFrame = m_iMaxFrame;
	pDest->m_fUSize = m_fUSize;
	pDest->m_fVSize = m_fVSize;
	pDest->m_fTexWidth = m_fTexWidth;
	pDest->m_fTexHeight = m_fTexHeight;
	pDest->m_fDefaultDelay = m_fDefaultDelay;
	pDest->m_kFrameInfo = m_kFrameInfo;
	pDest->m_kTextureName = m_kTextureName;
	pDest->m_kTextureNameSrc = m_kTextureNameSrc;
	pDest->m_kTextureNameDest = m_kTextureNameDest;
	pDest->m_kGeometryName = m_kGeometryName;
	pDest->m_spSpriteNode = m_spSpriteNode;
	pDest->m_fStartDelay = m_fStartDelay;
	pDest->m_bStartDelayRandom = m_bStartDelayRandom;
	pDest->m_kRandPosOpt = m_kRandPosOpt;
}

bool PgParticleProcessorSpriteAnimation::ParseXml(TiXmlNode const* pkNode, void* pArg)
{
	TiXmlElement const* pkFirstElement = (TiXmlElement const*)pkNode;
	PG_ASSERT_LOG(pkFirstElement);

	char const* pcFirstTagName = pkFirstElement->Value();	

	// ITEM Tag 파싱
	if(0 == strcmp(pcFirstTagName, "SPRITE"))
	{
		TiXmlAttribute const* pkAttr = pkFirstElement->FirstAttribute();
		while(pkAttr)
		{
			pkAttr = pkAttr->Next();
		}
	}

	typedef std::map<int, float> CONT_FRAME_INDEX_DELAY;
	CONT_FRAME_INDEX_DELAY kLocalFrameInfo;

	TiXmlElement const* pkElement = pkFirstElement->FirstChildElement();
	while(pkElement)
	{
		char const* pcTagName = pkElement->Value();
		
		if(0 == strcmp(pcTagName, "NIF_GEOMETRY"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "NAME"))
				{
					m_kGeometryName = pcAttrValue;
				}

				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "TEXTURE"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "SRC"))
				{
					m_kTextureNameSrc = pcAttrValue;
					m_kTextureNameDest = pkElement->GetText();
				}
				else if(0 == strcmp(pcAttrName, "NAME"))
				{
					m_kTextureName = pcAttrValue;
				}

				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "MAX_FRAME"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "FRAME"))
				{
					m_iMaxFrame = atoi(pcAttrValue);
				}

				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "SUBDIVISION"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "U_SUBDIVISION"))
				{
					m_iUSubdivision = atoi(pcAttrValue);
				}
				else if(0 == strcmp(pcAttrName, "V_SUBDIVISION"))
				{
					m_iVSubdivision = atoi(pcAttrValue);
				}

				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "TEXTURE_SIZE"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "WIDTH"))
				{
					m_fTexWidth = static_cast<float>(atof(pcAttrValue));
				}
				else if(0 == strcmp(pcAttrName, "HEIGHT"))
				{
					m_fTexHeight = static_cast<float>(atof(pcAttrValue));
				}

				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "FRAME_SIZE"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "WIDTH"))
				{
					m_fUSize = static_cast<float>(atof(pcAttrValue));
				}
				else if(0 == strcmp(pcAttrName, "HEIGHT"))
				{
					m_fVSize = static_cast<float>(atof(pcAttrValue));
				}

				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "FRAME_INFO"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "DEFALUT_DELAY"))
				{
					m_fDefaultDelay = static_cast<float>(atof(pcAttrValue));
				}

				pkAttr = pkAttr->Next();
			}			

			TiXmlElement const* pkChildElement = (TiXmlElement*)(pkElement->FirstChild());

			while(pkChildElement)
			{
				char const* pcLocalTagName = pkChildElement->Value();

				int iFrameNo = -1;
				float fFrameDelay = 0.0f;

				if(0 == strcmp(pcLocalTagName, "FRAME"))
				{
					TiXmlAttribute const* pkAttr = pkChildElement->FirstAttribute();
					while(pkAttr)
					{
						char const* pcAttrName = pkAttr->Name();
						char const* pcAttrValue = pkAttr->Value();

						if(0 == strcmp(pcAttrName, "NO"))
						{
							iFrameNo = atoi(pcAttrValue);
						}
						else if(0 == strcmp(pcAttrName, "DELAY"))
						{
							fFrameDelay = static_cast<float>(atof(pcAttrValue));
						}					

						pkAttr = pkAttr->Next();
					}
				}

				if(0 <= iFrameNo && 0 <= fFrameDelay)
				{
					kLocalFrameInfo.insert(std::make_pair(iFrameNo, fFrameDelay));
				}

				pkChildElement = pkChildElement->NextSiblingElement();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "LOOP"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "VALUE"))
				{
					if(0 == strcmpi("true", pcAttrValue))
					{
						m_bLoop = true;
					}
					else
					{
						m_bLoop = false;
					}
				}
				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "START_DELAY"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "VALUE"))
				{
					m_fStartDelay = static_cast<float>(atof(pcAttrValue));
				}
				else if(0 == strcmp(pcAttrName, "RANDOM"))
				{
					if(0 == strcmpi("true", pcAttrValue))
					{
						m_bStartDelayRandom = true;
					}
					else
					{
						m_bStartDelayRandom = false;
					}
				}

				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "SRC_BLEND_MODE"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "MODE"))
				{
					m_eSrcFunc = TransAlphaFunctionFromString(pcAttrValue);
				}
				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "DEST_BLEND_MODE"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "MODE"))
				{
					m_eDestFunc = TransAlphaFunctionFromString(pcAttrValue);
				}
				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "ALPHA_TEST"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "VALUE"))
				{
					if(0 == strcmpi("true", pcAttrValue))
					{
						m_bAlphaTesting = true;
					}
					else
					{
						m_bAlphaTesting = false;
					}
				}
				else if(0 == strcmp(pcAttrName, "REF"))
				{
					if(0 == strcmp(pcAttrName, "NO"))
					{
						m_byAlphaRef = static_cast<BYTE>(atoi(pcAttrValue));
					}
				}
				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else if(0 == strcmp(pcTagName, "RANDOM_POS"))
		{
			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while(pkAttr)
			{
				char const* pcAttrName = pkAttr->Name();
				char const* pcAttrValue = pkAttr->Value();

				if(0 == strcmp(pcAttrName, "TYPE"))
				{
					int const iType = atoi(pcAttrValue);
					if(SRandomOffsetPosOption::E_RANDOM_APPLY_TYPE_MAX > iType)
					{
						m_kRandPosOpt.eType = static_cast<SRandomOffsetPosOption::ERandomApplyType>(iType);
					}
				}
				else if(0 == strcmp(pcAttrName, "MIN"))
				{
					float fX = 0, fY = 0, fZ = 0;
					sscanf_s(pcAttrValue,"%f,%f,%f",&fX,&fY,&fZ);
					m_kRandPosOpt.kMin.x = fX;
					m_kRandPosOpt.kMin.y = fY;
					m_kRandPosOpt.kMin.z = fZ;
					
				}
				else if(0 == strcmp(pcAttrName, "MAX"))
				{
					float fX = 0, fY = 0, fZ = 0;
					sscanf_s(pcAttrValue,"%f,%f,%f",&fX,&fY,&fZ);
					m_kRandPosOpt.kMax.x = fX;
					m_kRandPosOpt.kMax.y = fY;
					m_kRandPosOpt.kMax.z = fZ;
				}
				else if(0 == strcmp(pcAttrName, "INERVAL_TIME"))
				{
					m_kRandPosOpt.fIntervalTIme = static_cast<float>(atof(pcAttrValue));
				}

				pkAttr = pkAttr->Next();
			}

			pkElement = pkElement->NextSiblingElement();
		}
		else
		{
			break;
		}
	}

	if(!m_spSpriteNode)
	{
		return false;
	}

	//기존 텍스쳐를 새로운 텍스쳐로 변경이 실패 한경우 -> 지오메트리 안에 텍스쳐가 없는 경우
	if(false == ChangeTextureRecursive(m_spSpriteNode, m_kTextureNameSrc, m_kTextureNameDest))
	{
		CreateBaseTextureInGeometry(m_kGeometryName, m_kTextureName);		
	}
	m_spSpriteNode->UpdateProperties();

	m_kFrameInfo.resize(m_iMaxFrame, SpriteFrameInfo(m_fDefaultDelay)); // MaxFrame만큼 생성 시킨다.

	for(CONT_FRAME_INDEX_DELAY::const_iterator itor = kLocalFrameInfo.begin(); itor != kLocalFrameInfo.end(); ++itor)
	{
		if(static_cast<int>(kLocalFrameInfo.size()) > (*itor).first)
		{
			m_kFrameInfo[(*itor).first].m_fDelay = (*itor).second;
		}
	}

	m_bPlay = true;

	return true;
}

bool PgParticleProcessorSpriteAnimation::ChangeTextureRecursive(NiNode *pkNode, std::string const& kSrcTextureName, std::string const& kDestTextureName)
{
	if(NULL == pkNode)
	{
		return false;
	}

	bool bChanged = false;

	unsigned int uiArrayCount = pkNode->GetArrayCount();

	for (unsigned int i = 0; i < uiArrayCount; ++i)
	{
		NiAVObject* pkChild = pkNode->GetAt(i);

		if(!pkChild)
		{
			continue;
		}

		if(NiIsKindOf(NiGeometry, pkChild))
		{
			NiGeometry* pkGeometry = NiDynamicCast(NiGeometry, pkChild);

			// 기존의 TextureProperty를 가져온다.
			NiTexturingProperty* pkTextureProp = NiDynamicCast(NiTexturingProperty, pkGeometry->GetProperty(NiTexturingProperty::GetType()));
			
			if(!pkTextureProp)
			{
				continue;
			}

			NiTexturingProperty::NiMapArray const& kMaps = pkTextureProp->GetMaps();
			unsigned int const iTotalMap = kMaps.GetSize();
			for(unsigned int i = 0; i < iTotalMap; ++i)
			{
				NiTexturingProperty::Map* pkMap = kMaps.GetAt(i);
				if(pkMap)
				{
					// 기존의 텍스쳐를 가져온다.
					NiTexture* pkTex = pkMap->GetTexture();
					NiSourceTexture* pkSrcTex = NiDynamicCast(NiSourceTexture, pkTex);

					if(NULL == pkSrcTex)
					{
						continue;
					}

					// 텍스쳐 파일 이름을 알아온다.
					NiFixedString const& rkFileName = pkSrcTex->GetFilename();
					if(0 >= rkFileName.GetLength())
					{
						continue;
					}

					char const* pcSrcTexture = strrchr((char const*)rkFileName, '\\');
					pcSrcTexture = (pcSrcTexture ? pcSrcTexture + 1 : pkSrcTex->GetFilename());

					// 기존에 붙어 있는 텍스쳐가 변경해야 하는 텍스쳐인지 검사.
					NiString kSrcTexture(pcSrcTexture);
					if(0 == kSrcTexture.Length())
					{
						continue;
					}

					kSrcTexture.ToLower();
					NiString kSrcTexture2(kSrcTextureName.c_str());
					kSrcTexture2.ToLower();

					if(kSrcTexture2 != kSrcTexture)
					{
						continue;
					}

					NiSourceTexture* pkNewTex = g_kNifMan.GetTexture(kDestTextureName);

					if(!pkNewTex)
					{
						continue;
					}

					// 새로운 텍스쳐를 설정한다.
					if(pkGeometry->GetModelData())
					{
						pkMap->SetTexture(pkNewTex);
						pkGeometry->GetModelData()->MarkAsChanged(NiGeometryData::TEXTURE_MASK);
						bChanged = true;
					}
				}
			}			
		}
		else if(NiIsKindOf(NiNode, pkChild))
		{
			bChanged |= ChangeTextureRecursive(NiDynamicCast(NiNode, pkChild), kSrcTextureName, kDestTextureName);
		}
	}

	return bChanged;
}

void PgParticleProcessorSpriteAnimation::CreateBaseTextureInGeometry(std::string const& kGeometryName, std::string const& kTextureName)
{
	//지오메티르 정보를 찾아서 텍스쳐 정보가 없을 경우 생성해서 넣어 주어야 한다.
	NiGeometry* pkGeom = NiDynamicCast(NiGeometry, m_spSpriteNode->GetObjectByName(kGeometryName.c_str()));
	if(NULL == pkGeom)
	{
		return;
	}

	NiGeometryData* pkGeomData = pkGeom->GetModelData();
	if(pkGeomData)
	{
		pkGeomData->SetConsistency(NiGeometryData::VOLATILE);
		if(NULL == pkGeomData->GetTextures())
		{
			pkGeomData->CreateTextures(true, 1);
		}

		if(NULL == pkGeomData->GetColors())
		{
			pkGeomData->CreateColors(true);

			int const iVertexCount = pkGeomData->GetVertexCount();

			NiColorA* pkColor = pkGeomData->GetColors();
			for(int i = 0; i < iVertexCount; ++i)
			{
				pkColor[i] = NiColorA::WHITE;
			}

			pkGeomData->MarkAsChanged(NiGeometryData::COLOR_MASK);
		}
	}
	NiTexturingPropertyPtr spTextureProp = NiNew NiTexturingProperty;
	if(spTextureProp)
	{
		spTextureProp->SetBaseFilterMode(NiTexturingProperty::FILTER_TRILERP);
		spTextureProp->SetApplyMode(NiTexturingProperty::APPLY_MODULATE);
		spTextureProp->SetBaseClampMode(NiTexturingProperty::CLAMP_S_WRAP_T);
		pkGeom->AttachProperty(spTextureProp);

		NiTexturePtr spTexture = g_kNifMan.GetTexture(kTextureName);
		spTextureProp->SetBaseTexture(spTexture);
	}

	NiAlphaPropertyPtr spAlphaProp = NiNew NiAlphaProperty;
	if(spAlphaProp)
	{
		spAlphaProp->SetAlphaBlending(true);
		spAlphaProp->SetSrcBlendMode(m_eSrcFunc);
		spAlphaProp->SetDestBlendMode(m_eDestFunc);
		spAlphaProp->SetAlphaTesting(m_bAlphaTesting);
		spAlphaProp->SetTestRef(m_byAlphaRef);

		pkGeom->AttachProperty(spAlphaProp);
	}
}