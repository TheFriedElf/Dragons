#include "StdAfx.h"
#include "lwUI.h"
#include "PgPilotMan.h"
#include "PgNetwork.h"
#include "lwCashItem.h"
#include "PgMyActorViewMgr.h"
#include "lwWorld.h"
#include "PgUIScene.h"
#include "PgBalloonEmoticonMgr.h"
#include "PgChatMgrClient.h"
#include "ServerLib.h"
#include "PgActionPool.h"
#include "PgAction.h"
#include "lwUIItemRarityUpgrade.h"
#include "PgMobileSuit.h"
#include "PgSoundMan.h"
#include "Variant/PgStringUtil.h"
#include "Variant/pgitemrarityupgradeformula.h"
#include "lwUIToolTip.h"
#include "PgPilot.h"

extern bool Recv_PT_X_X_ANS_ERROR_CHECK(HRESULT const Result);
extern bool MakeAbilNameString(WORD const Type, std::wstring &wstrText);
extern HRESULT MakeAbilValueString(PgBase_Item const &kItem, CAbilObject const* pDef, WORD const Type, int iValue, std::wstring &wstrText, bool const EqComp);
extern void GetEqiupPosString(DWORD const dwEquipLimit, bool const bIsPet, std::wstring& wstrText);
extern bool lwPlaySoundByID( char* szID );
extern bool CheckEnchantBundle(PgBase_Item const& kItem);

namespace Quest
{
	extern void SetCutedTextLimitLength(XUI::CXUI_Wnd *pkWnd, std::wstring const &rkText, std::wstring kTail, long const iCustomTargetWidth = 0);
}

int const MAX_TAB_COUNT = 3;
int const MAX_ITEM_COUNT = 36;

void PgCashItemUIRegWrapper(lua_State *pkState)
{
	lwStyleModifier::RegisterWrapper(pkState);
	lwUniEmotiUI::RegisterWrapper(pkState);
	lwLuckyChanger::RegisterWrapper(pkState);
	lwAccumlationExpCard::RegisterWrapper(pkState);
	lwCraftOptChanger::RegisterWrapper(pkState);
	lwPetOptChanger::RegisterWrapper(pkState);
	lwSkillExtend::RegisterWrapper(pkState);
	lwUseUnlockItem::RegisterWrapper(pkState);
	PgEnchantShift::RegisterWrapper(pkState);

}

namespace lwStyleModifier
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "StyleChangeTabControl", lwStyleChangeTabControl);
		def(pkState, "SetChangeStyleConfirm", lwSetChangeStyleConfirm);
		def(pkState, "CloseStyleUI", lwCloseStyleUI);
		def(pkState, "SelectStyleItem", lwSelectStyleItem);
		def(pkState, "DrawStyleItem", lwDrawStyleItem);
		def(pkState, "CallStyleItemToolTip", lwCallStyleItemToolTip);
		def(pkState, "ClearStyleItem", lwClearStyleItem);
	}

	void lwStyleChangeTabControl(lwUIWnd UISelf)
	{
		XUI::CXUI_CheckButton* pSelfBtn = dynamic_cast<XUI::CXUI_CheckButton*>(UISelf.GetSelf());
		if( !pSelfBtn )
		{
			return;
		}
		
		XUI::CXUI_Wnd* pStyleUI = pSelfBtn->Parent();
		if( !pStyleUI )
		{
			return;
		}

		for(int i = 0; i < MAX_TAB_COUNT; ++i)
		{
			BM::vstring vStr(L"CBTN_TAB");
			vStr += i;

			XUI::CXUI_CheckButton* pTempBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pStyleUI->GetControl(vStr));
			if( pTempBtn )
			{
				pTempBtn->ClickLock(false);
				pTempBtn->Check(false);
			}
		}

		pSelfBtn->Check(true);
		pSelfBtn->ClickLock(true);
		
		SetStyleUI(pStyleUI, (E_STYLE_TYPE)pSelfBtn->BuildIndex());
	}

	void lwSetChangeStyleConfirm(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pButton = UISelf.GetSelf();
		if( !pButton )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			g_kStyleModifier.SendPacket(pkPlayer->GetInven());
		}

		XUI::CXUI_Wnd* pMainUI = pButton->Parent();
		if( pMainUI )
		{
			pMainUI->Close();
		}
	}

	void lwCloseStyleUI()
	{
		g_kMyActorViewMgr.DeleteActor("STYLEINFO");
		g_kStyleModifier.Clear();
	}

	void lwSelectStyleItem(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pListItem = pSelf->Parent();
		if( !pListItem )
		{
			return;
		}

		XUI::CXUI_Wnd* pList = pListItem->Parent();
		if( !pList )
		{
			return;
		}	

		XUI::CXUI_Wnd* pMainUI = pList->Parent();
		if( !pMainUI )
		{
			return;
		}

		DWORD dwItemNo = 0;
		pSelf->GetCustomData(&dwItemNo, sizeof(dwItemNo));

		GET_DEF(CItemDefMgr, kItemDefMgr);
		const CItemDef* pItemDef = kItemDefMgr.GetDef(dwItemNo);
		if( pItemDef )
		{
			SetSelectStyleItem(pMainUI, (E_STYLE_TYPE)pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1), dwItemNo);
		}
	}

	void lwDrawStyleItem(lwUIWnd UISelf)
	{
		XUI::CXUI_Image* pIcon = dynamic_cast<XUI::CXUI_Image*>(UISelf.GetSelf());
		if( !pIcon )
		{ 
			return; 
		}

		DWORD dwItemNo = 0;
		pIcon->GetCustomData(&dwItemNo, sizeof(dwItemNo));

		GET_DEF(CItemDefMgr, kItemDefMgr);
		const CItemDef* pItemDef = kItemDefMgr.GetDef(dwItemNo);

		PgUISpriteObject* pkSprite = g_kUIScene.GetIconTexture(dwItemNo);
		if( !pkSprite )
		{
			pIcon->DefaultImgTexture(NULL);
			pIcon->SetInvalidate();
			return;
		}

		PgUIUVSpriteObject* pkUVSprite = dynamic_cast<PgUIUVSpriteObject*>(pkSprite);
		if( !pkUVSprite ){ return; }

		pIcon->DefaultImgTexture(pkUVSprite);
		SUVInfo& rkUV = pkUVSprite->GetUVInfo();
		pIcon->UVInfo(rkUV);
		POINT2	kPoint(40*rkUV.U, 40*rkUV.V);
		pIcon->ImgSize(kPoint);
		pIcon->SetInvalidate();
	}

	void lwCallStyleItemToolTip(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pIcon = UISelf.GetSelf();
		if( !pIcon )
		{ 
			return; 
		}

		DWORD dwItemNo = 0;
		pIcon->GetCustomData(&dwItemNo, sizeof(dwItemNo));

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pDef = kItemDefMgr.GetDef(dwItemNo);
		if( !pDef )
		{
			return;
		}

		PgBase_Item kItem;
		kItem.ItemNo(dwItemNo);
		kItem.Count(pDef->GetAbil(AT_DEFAULT_AMOUNT));

		CallToolTip_SItem(&kItem, UISelf.GetTotalLocation());
	}

	void lwClearStyleItem(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		if( !pMainUI )
		{
			return;
		}

		for(int i = ESTYLE_BEGIN; i < ESTYLE_END; ++i)
		{
			BM::vstring vStr(L"ICN_STYLE");
			vStr += i;

			XUI::CXUI_Wnd* pIcon = pMainUI->GetControl(vStr);
			if( pIcon )
			{
				pIcon->ClearCustomData();
			}
		}

		g_kStyleModifier.ColorItem(0);
		g_kStyleModifier.HairItem(0);
		g_kStyleModifier.FaceItem(0);

		SetSelectStyleItem(pMainUI, ESTYLE_COLOR, g_kStyleModifier.BaseColorItem(), true);
		SetSelectStyleItem(pMainUI, ESTYLE_HAIR, g_kStyleModifier.BaseHairItem(), true);
		SetSelectStyleItem(pMainUI, ESTYLE_FACE, g_kStyleModifier.BaseFaceItem(), true);
	}

	void CallStyleModifyUI(E_STYLE_TYPE const Type, DWORD const dwItemNo)
	{
		if( false == g_kStyleModifier.IsAlive() )
		{
			g_kStyleModifier.Build();

			g_kMyActorViewMgr.Update("STYLEINFO", g_kPilotMan.PlayerPilotGuid());
		}
		XUI::CXUI_Wnd* pStyleUI = g_kStyleModifier.GetWnd();
		if( !pStyleUI )
		{
			return;
		}
		lwClearStyleItem(pStyleUI->GetControl(L"BTN_RESET"));

		for(int i = 0; i < MAX_TAB_COUNT; ++i)
		{
			BM::vstring vStr(L"CBTN_TAB");
			vStr += i;

			XUI::CXUI_CheckButton* pTempBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pStyleUI->GetControl(vStr));
			if( pTempBtn )
			{
				pTempBtn->ClickLock(false);
				pTempBtn->Check(false);
			}
		}

		BM::vstring vStr(L"CBTN_TAB");
		vStr += Type;
		XUI::CXUI_CheckButton* pSelfBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pStyleUI->GetControl(vStr));
		if( pSelfBtn )
		{
			pSelfBtn->Check(true);
			pSelfBtn->ClickLock(true);
			
			SetStyleUI(pStyleUI, Type);
		}

		//SetSelectStyleItem(pStyleUI, Type, dwItemNo);
	}

	void SetStyleUI(XUI::CXUI_Wnd* pStyleUI, E_STYLE_TYPE const Type)
	{
		if( !pStyleUI )
		{
			return;
		}

		XUI::CXUI_List* pList = dynamic_cast<XUI::CXUI_List*>(pStyleUI->GetControl(L"LST_ITEM"));
		if( !pList )
		{
			return;
		}

		XUI::SListItem*	pItem = NULL;
		if( 0 == pList->GetTotalItemCount() )
		{
			pItem = pList->AddItem(L"");
		}
		else
		{
			pItem = pList->FirstItem();
		}

		if( !pItem || !pItem->m_pWnd )
		{
			return;
		}
		SetStyleItemInv(pItem->m_pWnd, Type);
	}

	void SetStyleItemInv(XUI::CXUI_Wnd* pItemUI, E_STYLE_TYPE const Type)
	{
		if( !pItemUI )
		{
			return;
		}

		CONT_STYLE_ITEM kList;
		g_kStyleModifier.GetStyleTypeList(Type, kList);

		PgPlayer *pPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pPlayer )
		{
			return;
		}
		PgInventory *pInv = pPlayer->GetInven();
		if( !pInv )
		{
			return;
		}
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CONT_STYLE_ITEM::iterator iter_Check = kList.begin();
		while( kList.end() != iter_Check )
		{
			PgBase_Item kItem;
			if( E_FAIL == pInv->GetItem( iter_Check->first, kItem ) )
			{
				kList.erase(iter_Check++);
				continue;
			}
			CItemDef const *pItemDef = kItemDefMgr.GetDef(iter_Check->second);
			if(!pItemDef)
			{
				kList.erase(iter_Check++);
				continue;
			}
			__int64 const i64ClassLimit = pItemDef->GetAbil64(AT_CLASSLIMIT);
			int const iGenderLimit = pItemDef->GetAbil(AT_GENDERLIMIT);
			if( IS_CLASS_LIMIT(i64ClassLimit, pPlayer->GetAbil(AT_CLASS) ) == false)
			{
				kList.erase(iter_Check++);
				continue;
			}
			int const iGender = pPlayer->GetAbil(AT_GENDER);
			if(  iGender != iGenderLimit&&  
				3 != iGenderLimit)
			{//공용이 아닌데 내 성별이랑 다르면 안됨
				kList.erase(iter_Check++);
				continue;
			}

			++iter_Check;
		}


		CONT_STYLE_ITEM::const_iterator	c_iter = kList.begin();
		for(int i = 0; i < MAX_ITEM_COUNT; ++i)
		{
			BM::vstring	vStr(L"ICN_ICON");
			BM::vstring	vStrBg(L"SFRM_ICON_BG");
			vStr += i;
			vStrBg += i;

			XUI::CXUI_Wnd* pIcon = pItemUI->GetControl(vStr);
			XUI::CXUI_Wnd* pIconBg = pItemUI->GetControl(vStrBg);
			if( pIcon && pIconBg )
			{
				if( c_iter != kList.end() )
				{
					pIcon->SetCustomData(&c_iter->second, sizeof(c_iter->second));
					pIconBg->SetCustomData(&c_iter->first, sizeof(c_iter->first));
					++c_iter;
				}
				else
				{
					pIcon->ClearCustomData();
					pIconBg->ClearCustomData();
				}
			}
		}
	}

	void SetSelectStyleItem(XUI::CXUI_Wnd* pMainUI, E_STYLE_TYPE const Type, DWORD const dwItemNo, bool const bReset)
	{
		if( false == bReset )
		{
			BM::vstring vStr(L"ICN_STYLE");
			vStr += Type;
			XUI::CXUI_Wnd* pIcon = pMainUI->GetControl(vStr);
			if( pIcon )
			{
				pIcon->SetCustomData(&dwItemNo, sizeof(dwItemNo));
				switch( Type )
				{
				case ESTYLE_COLOR:	{ g_kStyleModifier.ColorItem(dwItemNo);	} break;
				case ESTYLE_HAIR:	{ g_kStyleModifier.HairItem(dwItemNo);	} break;
				case ESTYLE_FACE:	{ g_kStyleModifier.FaceItem(dwItemNo);	} break;
				}
			}
		}

		if( g_kMyActorViewMgr.BeginChangeParts("STYLEINFO") )
		{
			if( ESTYLE_COLOR == Type )
			{
				g_kMyActorViewMgr.ChangePartsColor(EQUIP_LIMIT_HAIR, dwItemNo);
			}
			g_kMyActorViewMgr.ChangeParts(dwItemNo, CET_DEL);
			g_kMyActorViewMgr.ChangeParts(dwItemNo, CET_ADD);
			g_kMyActorViewMgr.EndChangeParts();
		}
	}
};


PgStyleModifier::PgStyleModifier()
{
	Clear();
}

PgStyleModifier::~PgStyleModifier()
{
	Clear();
}

void PgStyleModifier::Build()
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}

	PgInventory* pInv = pPlayer->GetInven();
	if( !pInv )
	{
		return;
	}

	PgActor* pActor = g_kPilotMan.GetPlayerActor();
	if( !pActor )
	{
		return;
	}

	Clear();

	m_kBaseColorItem = pActor->GetDefaultItem(EQUIP_LIMIT_HAIR_COLOR);
	m_kBaseHairItem = pActor->GetDefaultItem(EQUIP_LIMIT_HAIR);
	m_kBaseFaceItem = pActor->GetDefaultItem(EQUIP_LIMIT_FACE);

	int const iGender = pPlayer->GetAbil(AT_GENDER);

	for(int i = IT_EQUIP; i < IT_SAFE; ++i)
	{
		int const MAX_IDX = pInv->GetMaxIDX((EInvType)i);
		for(int j = 0; j < MAX_IDX; ++j)
		{
			SItemPos kPos(i, j);
			PgBase_Item kItem;

			if( S_OK != pInv->GetItem(kPos, kItem) )
			{
				continue;
			}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const* pDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( !pDef )
			{
				assert(0);
				continue;
			}

			int const iGenderLimit = pDef->GetAbil(AT_GENDERLIMIT);
			if( (iGenderLimit != UG_UNISEX) && (iGenderLimit != iGender))
			{//다른 성별 아이템이므로 패스
				continue;
			}

			if( UICT_STYLEITEM == pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
			{
				int const StyleType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
				if( ESTYLE_BEGIN <= StyleType && ESTYLE_END > StyleType )
				{
					m_kContStyleItem[StyleType].insert(std::make_pair(kPos, kItem.ItemNo()));
				}
			}
		}
	}
}

XUI::CXUI_Wnd* PgStyleModifier::GetWnd()
{
	if( !m_pMainUI )
	{
		m_pMainUI = XUIMgr.Call(L"SFRM_STYLE_EDIT");
		if( !m_pMainUI )
		{
			return NULL;
		}
	}

	return m_pMainUI;
}

void PgStyleModifier::GetStyleTypeList(E_STYLE_TYPE const Type, CONT_STYLE_ITEM& kList)
{
	kList.insert(m_kContStyleItem[Type].begin(), m_kContStyleItem[Type].end());
}

void PgStyleModifier::Clear()
{
	if( m_pMainUI ){ m_pMainUI = NULL; }

	for( int i = ESTYLE_BEGIN; i < ESTYLE_END; ++i )
	{
		m_kContStyleItem[i].clear();
	}

	m_kColorItem = 0;
	m_kHairItem = 0;
	m_kFaceItem = 0;
	m_kBaseColorItem = 0;
	m_kBaseHairItem = 0;
	m_kBaseFaceItem = 0;

	m_kSendPacket = false;
}

void PgStyleModifier::SendPacket(PgInventory* pkInv)
{
	if( NULL == pkInv )
	{
		return;
	}
	SendPacketIsAction(pkInv, m_kColorItem);
	SendPacketIsAction(pkInv, m_kHairItem);
	SendPacketIsAction(pkInv, m_kFaceItem);
}

bool PgStyleModifier::SendPacketIsAction(PgInventory* pkInv, DWORD& dwItemNo)
{
	if( NULL == pkInv || 0 == dwItemNo )
	{
		return false;
	}

	SItemPos kTempPos;
	if(E_FAIL == pkInv->GetFirstItem(dwItemNo, kTempPos))
	{
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 51208, true);
		return false;
	}

	BM::Stream Packet(PT_C_M_REQ_ITEM_ACTION, kTempPos);
	Packet.Push(lwGetServerElapsedTime32());
	NETWORK_SEND(Packet);
	dwItemNo = 0;
	return true;
}

namespace lwUniEmotiUI
{
	int const MAX_EMOTI_LINE_ITEM_SLOT = 8;
	int const MAX_EMOTI_ITEM_LINE = 3;
	int const MAX_EMOTI_ITEM_SLOT = MAX_EMOTI_LINE_ITEM_SLOT * MAX_EMOTI_ITEM_LINE;

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "CallUniEmotiUI", &lwCallUniEmotiUI);
		def(pkState, "OnClickEmotiPage", &lwOnClickEmotiPage);
		def(pkState, "OnClickEmotiCategory", &lwOnClickEmotiCategory);
		def(pkState, "OnSelectSubTypeItem", &lwOnSelectSubTypeItem);
		def(pkState, "OnSelectEmotiItem", &lwOnSelectEmotiItem);
		def(pkState, "OnOverEmotiItem", &lwOnOverEmotiItem);
		def(pkState, "IsMyPlayerLearnInteractiveEmotion", &lwIsMyPlayerLearnInteractiveEmotion);
	}

	void lwCallUniEmotiUI()
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"FRM_NOVEL_ACTION");
		if( !pMainUI || pMainUI->IsClosed() )
		{
			pMainUI = XUIMgr.Call(L"FRM_NOVEL_ACTION");
			if( !pMainUI ){ return; }

			XUI::CXUI_Wnd* pButton = pMainUI->GetControl(L"CBTN_NOVEL_TYPE0");
			if( pButton )
			{
				lwOnClickEmotiPage(lwUIWnd(pButton));
			}			
		}
		else
		{
			pMainUI->Close();
		}
	}

	void lwOnClickEmotiPage(lwUIWnd UISelf)
	{
		XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(UISelf.GetSelf());
		if( !pCheckBtn ){ return; }

		XUI::CXUI_Wnd* pParent = pCheckBtn->Parent();
		if( !pParent ){ return; }

		for(int i = 0; i < 4; ++i)
		{
			BM::vstring	vStr(L"CBTN_NOVEL_TYPE");
			vStr += i;

			XUI::CXUI_CheckButton* pCheckType = dynamic_cast<XUI::CXUI_CheckButton*>(pParent->GetControl(vStr));
			if( pCheckType )
			{
				pCheckType->ClickLock(false);
				pCheckType->Check(false);
			}
		}

		pCheckBtn->Check(true);
		pCheckBtn->ClickLock(true);

		int const iBuildIndex = pCheckBtn->BuildIndex();

		switch( iBuildIndex )
		{
		case 0:
			{
				SetTabPage(EMOTITT_EMOTION, pParent);
			}break;
		case 1:
			{
				SetTabPage(EMOTITT_CHAT, pParent);
			}break;
		default:
			{
				SetTabPage(static_cast<E_EMOTI_TAB_TYPE>(iBuildIndex), pParent);
			}break;
		}
	}

	void lwOnClickEmotiCategory(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( !pSelf ){ return; }

		int iType = 0;
		pSelf->GetCustomData(&iType, sizeof(iType));

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent ){ return; }
		EEMOTION_TYPE Type;
		pParent->GetCustomData(&Type, sizeof(Type));

		XUI::CXUI_Wnd* pValueText = pParent->GetControl(L"FRM_SUB_TYPE_TEXT");
		if( !pValueText ){ return; }

		XUI::CXUI_Wnd* pDropWnd = XUIMgr.Get(L"FRM_EMOTI_SUB_TYPE_DROP");
		if( pDropWnd )
		{
			EEMOTION_TYPE PrevType;
			pDropWnd->GetCustomData(&PrevType, sizeof(PrevType));
			if( PrevType == Type )
			{
				pDropWnd->Close();
				return;
			}
		}
		else
		{
			pDropWnd = XUIMgr.Call(L"FRM_EMOTI_SUB_TYPE_DROP");
		}
		if( !pDropWnd ){ return; }
		pDropWnd->SetCustomData(&Type, sizeof(Type));

		XUI::CXUI_List* pDropList = dynamic_cast<XUI::CXUI_List*>(pDropWnd->GetControl(L"LST_DROP"));
		if( !pDropList ){ return; }

		pDropWnd->Location(pValueText->TotalLocation().x + 2, pValueText->TotalLocation().y + pValueText->Size().y + 2);

		CONT_EMOTION const * pContEmotion = NULL;
		g_kTblDataMgr.GetContDef(pContEmotion);	//맵 데프

		PgEmotiMgr::CONT_IDARRAY kArray;
		if( GetEmotiCategoryList(static_cast<E_EMOTI_TAB_TYPE>(Type), kArray) )
		{
			int iItemCount = pDropList->GetTotalItemCount();

			if( kArray.size() < iItemCount )
			{
				for(; iItemCount > kArray.size(); --iItemCount)
				{
					pDropList->DeleteItem(pDropList->FirstItem());
				}
			}
			else if( kArray.size() > iItemCount )
			{
				for(; iItemCount < kArray.size(); ++iItemCount)
				{
					pDropList->AddItem(L"");
				}
			}

			::CONT_EMOTION_GROUP const* pContEmotiGroup = NULL;
			g_kTblDataMgr.GetContDef(pContEmotiGroup);	//맵 데프
			if( !pContEmotiGroup || pContEmotiGroup->empty() )
			{
				pDropWnd->Close();
				return;
			}

			PgEmotiMgr::CONT_IDARRAY::iterator	iter = kArray.begin();
			XUI::SListItem* pItem = pDropList->FirstItem();
			while(iter != kArray.end())
			{
				if( pItem && pItem->m_pWnd )
				{//그룹정보를 세팅한다
					CONT_EMOTION_GROUP::const_iterator def_iter = pContEmotiGroup->find(SEMOTIONGROUPKEY(Type, (*iter)));
					if( def_iter == pContEmotiGroup->end() )
					{
						++iter;
						continue;
					}

					wchar_t const* szName = NULL;
					if( !GetDefString(def_iter->second, szName) )
					{
						++iter;
						continue;
					}

					pItem->m_pWnd->Text(szName);
					pItem->m_pWnd->SetCustomData(&(*iter), sizeof((*iter)));
					pItem = pDropList->NextItem(pItem);
				}
				++iter;
			}
		}
	}

	void lwOnSelectSubTypeItem(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( !pSelf ){ return; }

		XUI::CXUI_Wnd* pList = pSelf->Parent();
		if( !pList ){ return; }

		XUI::CXUI_Wnd* pParent = pList->Parent();
		if( !pParent ){ return; }

		EEMOTION_TYPE Type;
		int iSubType = 0;

		pParent->GetCustomData(&Type, sizeof(Type));
		pSelf->GetCustomData(&iSubType, sizeof(iSubType));


		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"FRM_NOVEL_ACTION");
		if( !pMainUI ){ return; }

		XUI::CXUI_Wnd* pUniTypeTab = pMainUI->GetControl(L"FRM_SUB_NOVEL");
		if( !pUniTypeTab )
		{
			return;
		}
		pUniTypeTab->Visible(true);
		pUniTypeTab->SetCustomData(&Type, sizeof(Type));

		switch( Type )
		{
		case EMOTITT_CHAT:		{ SetChatEmoticonPage(pUniTypeTab, iSubType);	} break;
		case EMOTITT_BALLOON:	{ SetBalloonPage(pUniTypeTab, iSubType);		} break;
		case EMOTITT_EMOTION:	{ SetEmotionPage(pUniTypeTab, iSubType);		} break;
		default:{}return;
		}
	}

	void lwOnSelectEmotiItem(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( !pSelf ){ return; }

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent ){ return; }

		EEMOTION_TYPE Type;
		pParent->GetCustomData(&Type, sizeof(Type));
		int const iEmotiID = lwUIWnd(pSelf).GetCustomData<int>();
		if( iEmotiID == 0 )
		{
			return;
		}

		switch(Type)
		{
		case ET_EMOTICON:
			{ 
				lua_tinker::call<void, int>("AddEmoticon", iEmotiID);
			} break;
		case ET_BALLOON:
			{
				SendEmotionPacket(g_kPilotMan.PlayerActor(), ET_BALLOON, iEmotiID);
			} break;
		case ET_EMOTION:
			{
				if(CheckCanEmotion(iEmotiID))
				{
					SendEmotionPacket(g_kPilotMan.PlayerActor(), ET_EMOTION, iEmotiID);
				}
			}break;
		}
	}

	void lwOnOverEmotiItem(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( !pSelf ){ return; }

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent ){ return; }

		EEMOTION_TYPE Type;
		pParent->GetCustomData(&Type, sizeof(Type));
		int const iEmotiID = lwUIWnd(pSelf).GetCustomData<int>();
		std::wstring kText = g_kPgEmotiMgr.GetChatString(Type, iEmotiID);
		if( 0 != iEmotiID && !kText.empty() )
		{
			lwCallToolTipByText(0, kText, lwPoint2(XUIMgr.MousePos().x + 3,XUIMgr.MousePos().y + 3));
		}
		else if(ET_EMOTION == Type)
		{
			PgEmotiMgr::tagEmotion const &kEmotion	= g_kPgEmotiMgr.GetEmotion(iEmotiID);
			if(!kEmotion.ActionID().empty())
			{
				PgEmotiMgr::tagEmotion::CONT_PARAM const& kCont = kEmotion.ParamCont();
				PgEmotiMgr::tagEmotion::CONT_PARAM::const_iterator kItor = kCont.find(L"INTERACTIVE");
				if(kCont.end() != kItor)
				{
					kItor = kCont.find(L"ACTION_NO");
					if(kCont.end() != kItor)
					{
						PgEmotiMgr::tagEmotion::tagParam kParam = kItor->second;
						int const iActionNo = ::_wtoi(kParam.VAL.c_str());
						GET_DEF(CSkillDefMgr, kSkillDefMgr);
						CSkillDef const* pkSkillDef = kSkillDefMgr.GetDef(iActionNo);
						const wchar_t *pwcText = NULL;
						int iValue = pkSkillDef->GetAbil(ATI_EXPLAINID);
						if(iValue && GetDefString(iValue, pwcText))
						if(pwcText)
						{
							lwCallToolTipByText(0, pwcText, lwPoint2(XUIMgr.MousePos().x + 3,XUIMgr.MousePos().y + 3));
						}
					}
				}
			}
		}

		XUI::CXUI_Wnd* pOverImg = pParent->GetControl(L"IMG_CURSOR");
		if( !pOverImg ){ return; }

		if( !pOverImg->Visible() )
		{ 
			pOverImg->Visible(true); 
		}
		pOverImg->Location(pSelf->Location().x, pSelf->Location().y);
	}

	void SetTabPage(E_EMOTI_TAB_TYPE const Type, XUI::CXUI_Wnd* pParent)
	{
		XUI::CXUI_Wnd* pUniTypeTab = pParent->GetControl(L"FRM_SUB_NOVEL");
		if( !pUniTypeTab )
		{
			return;
		}
		pUniTypeTab->Visible(true);
		pUniTypeTab->SetCustomData(&Type, sizeof(Type));

		XUI::CXUI_Wnd* pkDropBtn = pUniTypeTab->GetControl(L"FRM_DROP_SUB_TYPE");
		if( pkDropBtn )
		{
			PgEmotiMgr::CONT_IDARRAY kArray;
			pkDropBtn->Visible( GetEmotiCategoryList(Type, kArray) );
		}

		switch( Type )
		{
		case EMOTITT_CHAT:		{ SetChatEmoticonPage(pUniTypeTab, 0);	} break;
		case EMOTITT_BALLOON:	{ SetBalloonPage(pUniTypeTab, 0);		} break;
		case EMOTITT_EMOTION:	{ SetEmotionPage(pUniTypeTab, 0);		} break;
		default:{}return;
		}
	}

	void SetChatEmoticonPage(XUI::CXUI_Wnd* pPage, int const iGroup)
	{
		if( !pPage ){ return; }

		XUI::CXUI_Wnd* pSubTypeText = pPage->GetControl(L"FRM_SUB_TYPE_TEXT");
		if( pSubTypeText )
		{
			::CONT_EMOTION_GROUP const* pContEmotiGroup = NULL;
			g_kTblDataMgr.GetContDef(pContEmotiGroup);	//맵 데프
			if( pContEmotiGroup )
			{
				CONT_EMOTION_GROUP::const_iterator def_iter = pContEmotiGroup->find(SEMOTIONGROUPKEY(ET_EMOTICON, iGroup));
				if( def_iter != pContEmotiGroup->end() )
				{
					wchar_t const* szName = NULL;
					if( GetDefString(def_iter->second, szName) )
					{
						pSubTypeText->Text(szName);
					}
					else
					{
						pSubTypeText->Text(L"");
					}
				}
			}
		}

		PgEmotiMgr::CONT_IDARRAY	kCont;
		if( g_kPgEmotiMgr.GetEmotiGroupItem(ET_EMOTICON, iGroup, kCont) )
		{
			PgEmotiMgr::CONT_IDARRAY::iterator iter = kCont.begin();
			for(int i = 0; i < MAX_EMOTI_ITEM_SLOT; ++i)
			{
				BM::vstring	vStr(L"FRM_ICON");
				vStr += i;
				XUI::CXUI_Wnd* pIcon = pPage->GetControl(vStr);
				if( pIcon )
				{
					pIcon->DefaultImgName(L"");
					if( iter != kCont.end() )
					{
						BM::vstring	TextStr(L"#");
						TextStr += (*iter);
						pIcon->Text(g_kChatMgrClient.ConvertUserCommand(TextStr));
						pIcon->SetCustomData(&(*iter), sizeof((*iter)));
						++iter;
					}
					else
					{
						pIcon->Text(L"");
						pIcon->ClearCustomData();
					}
				}
			}
		}
	}

	void SetBalloonPage(XUI::CXUI_Wnd* pPage, int const iGroup)
	{
		if( !pPage ){ return; }

		XUI::CXUI_Wnd* pSubTypeText = pPage->GetControl(L"FRM_SUB_TYPE_TEXT");
		if( pSubTypeText )
		{
			::CONT_EMOTION_GROUP const* pContEmotiGroup = NULL;
			g_kTblDataMgr.GetContDef(pContEmotiGroup);	//맵 데프
			if( pContEmotiGroup )
			{
				CONT_EMOTION_GROUP::const_iterator def_iter = pContEmotiGroup->find(SEMOTIONGROUPKEY(ET_BALLOON, iGroup));
				if( def_iter != pContEmotiGroup->end() )
				{
					wchar_t const* szName = NULL;
					if( GetDefString(def_iter->second, szName) )
					{
						pSubTypeText->Text(szName);
					}
					else
					{
						pSubTypeText->Text(L"");
					}
				}
			}
		}

		PgEmotiMgr::CONT_IDARRAY	kCont;
		if( g_kPgEmotiMgr.GetEmotiGroupItem(ET_BALLOON, iGroup, kCont) )
		{
			PgEmotiMgr::CONT_IDARRAY::iterator iter = kCont.begin();
			for(int i = 0; i < MAX_EMOTI_ITEM_SLOT; ++i)
			{
				BM::vstring	vStr(L"FRM_ICON");
				vStr += i;
				XUI::CXUI_Wnd* pIcon = pPage->GetControl(vStr);
				if( pIcon )
				{
					pIcon->Text(L"");
					if( iter != kCont.end() )
					{
						pIcon->DefaultImgName(g_kPgEmotiMgr.GetBalloonIcon((*iter)));
						pIcon->ImgPos(POINT2(1, 1));
						pIcon->ImgSize(POINT2(25, 25));
						pIcon->SetCustomData(&(*iter), sizeof((*iter)));
						++iter;
					}
					else
					{
						pIcon->DefaultImgName(L"");
						pIcon->ClearCustomData();
					}
				}
			}
		}
	}

	void SetEmotionPage(XUI::CXUI_Wnd* pPage, int const iGroup)
	{
		if( !pPage ){ return; }

		XUI::CXUI_Wnd* pSubTypeText = pPage->GetControl(L"FRM_SUB_TYPE_TEXT");
		if( pSubTypeText )
		{
			::CONT_EMOTION_GROUP const* pContEmotiGroup = NULL;
			g_kTblDataMgr.GetContDef(pContEmotiGroup);	//맵 데프
			if( pContEmotiGroup )
			{
				CONT_EMOTION_GROUP::const_iterator def_iter = pContEmotiGroup->find(SEMOTIONGROUPKEY(ET_EMOTION, iGroup));
				if( def_iter != pContEmotiGroup->end() )
				{
					wchar_t const* szName = NULL;
					if( GetDefString(def_iter->second, szName) )
					{
						pSubTypeText->Text(szName);
					}
					else
					{
						pSubTypeText->Text(L"");
					}
				}
			}
		}

		PgEmotiMgr::CONT_IDARRAY	kCont;
		if( g_kPgEmotiMgr.GetEmotiGroupItem(ET_EMOTION, iGroup, kCont) )
		{
			PgEmotiMgr::CONT_IDARRAY::iterator iter = kCont.begin();
			for(int i = 0; i < MAX_EMOTI_ITEM_SLOT; ++i)
			{
				BM::vstring	vStr(L"FRM_ICON");
				vStr += i;
				XUI::CXUI_Wnd* pIcon = pPage->GetControl(vStr);
				if( pIcon )
				{
					pIcon->Text(L"");
					if( iter != kCont.end() )
					{
						PgEmotiMgr::tagEmotion const &kEmotion = g_kPgEmotiMgr.GetEmotion((*iter));
						pIcon->DefaultImgName(kEmotion.IconPath());
						pIcon->ImgPos(POINT2(1, 1));
						pIcon->ImgSize(POINT2(25, 25));
						pIcon->SetCustomData(&(*iter), sizeof((*iter)));
						++iter;
					}
					else
					{
						pIcon->DefaultImgName(L"");
						pIcon->ClearCustomData();
					}
				}
			}
		}
	}

	bool GetEmotiCategoryList(E_EMOTI_TAB_TYPE const Type, std::vector< int >& kArray)
	{
		CONT_EMOTION const * pContEmotion = NULL;
		g_kTblDataMgr.GetContDef(pContEmotion);	//맵 데프

		if( g_kPgEmotiMgr.GetEmotiGroupArray(static_cast<EEMOTION_TYPE>(Type), kArray) )
		{
			PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
			if( pPlayer )
			{
				PgEmotiMgr::CONT_IDARRAY::iterator iter = kArray.begin();
				while( iter != kArray.end() )
				{
					PgEmotiMgr::CONT_IDARRAY kEmotiArray;
					if( !g_kPgEmotiMgr.GetEmotiGroupItem(static_cast<EEMOTION_TYPE>(Type), (*iter), kEmotiArray) )
					{
						iter = kArray.erase(iter);
						continue;
					}

					if( kEmotiArray.empty() )
					{
						iter = kArray.erase(iter);
						continue;
					}

					::CONT_EMOTION const * pContEmotion = NULL;
					g_kTblDataMgr.GetContDef(pContEmotion);	//맵 데프
					if( !pContEmotion )
					{
						return false;
					}

					::CONT_EMOTION::const_iterator def_iter = pContEmotion->find(SEMOTIONKEY(Type, (*kEmotiArray.begin())));
					if( def_iter == pContEmotion->end() )
					{
						iter = kArray.erase(iter);
						continue;
					}

					PgBitArray<MAX_DB_EMOTICON_SIZE> kCont;
					if( pPlayer->GetContEmotion(Type, kCont) )
					{
						if( !kCont.Get(def_iter->second) )
						{
							iter = kArray.erase(iter);
							continue;
						}
					}	
					++iter;
				}
			}
		}
		return !kArray.empty();
	}

	void ReceivePacket_Command(WORD const wPacketType, BM::Stream& kPacket)
	{
		switch( wPacketType )
		{
		case PT_M_C_ANS_USE_EMOTION:
			{
				HRESULT	kResult;
				kPacket.Pop(kResult);
				switch( kResult )
				{
				case E_EM_NOT_FOUND_EMOTION:
				case E_EM_NOT_HAVE_EMOTION:
				case E_EM_ALREADY_HAVE:
				case E_EM_INVALID_TYPE:
				case E_EM_INVALID_EMOTION:
					{
						lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 403851 + (kResult - E_EM_NOT_FOUND_EMOTION), true);
					}break;
				default: break;
				}
			}break;
		case PT_C_M_NFY_USE_EMOTION:
			{
				BM::GUID kGuid;
				BYTE bEmotiType;
				int iEmotiNo;

				kPacket.Pop(kGuid);
				kPacket.Pop(bEmotiType);
				kPacket.Pop(iEmotiNo);

				PgActor* pTargetActor = g_kPilotMan.FindActor(kGuid);
				if( !pTargetActor )
				{
					return;
				}

				PgActor* pActor = g_kPilotMan.FindActor(kGuid);
				if( !pActor )
				{
					return;
				}

				switch( bEmotiType )
				{
				case ET_EMOTION:
					{//해당 엑터에 이모션을 시킨다. 맨마지막에 구현하세?
						PgEmotiMgr::tagEmotion const &kEmotion = g_kPgEmotiMgr.GetEmotion(iEmotiNo);
						if( !kEmotion.IsEmpty() )
						{
							if( !kEmotion.ParamCont().empty() )
							{
								PgEmotiMgr::tagEmotion::CONT_PARAM::const_iterator iter = kEmotion.ParamCont().begin();
								while( iter != kEmotion.ParamCont().end() )
								{
									pActor->SetParam(MB(iter->second.KEY), MB(iter->second.VAL));
									++iter;
								}
							}
							pActor->ReserveTransitAction(MB(kEmotion.ActionID()));
						}
					}break;
				case ET_BALLOON:
					{//해당 엑터에 풍선 이모티콘을 붙인다.
						if( g_kBalloonEmoticonMgr.IsAlive() )
						{
							g_kBalloonEmoticonMgr.AddNewBalloonEmoticon(pActor, iEmotiNo);
						}
					}break;
				default: break;
				}
			}break;
		default: return;
		}
	}
	
	bool lwIsMyPlayerLearnInteractiveEmotion()
	{
		PgEmotiMgr::CONT_IDARRAY kArray;
		GetEmotiCategoryList(EMOTITT_EMOTION, kArray);
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( pkPlayer )
		{
			PgEmotiMgr::CONT_IDARRAY::iterator iter = kArray.begin();
			while( iter != kArray.end() )
			{
				PgEmotiMgr::CONT_IDARRAY kEmotiArray;
				if( !g_kPgEmotiMgr.GetEmotiGroupItem(ET_EMOTION, (*iter), kEmotiArray) )
				{// ?
					iter = kArray.erase(iter);
					continue;
				}
				if( kEmotiArray.empty() )
				{// ?
					iter = kArray.erase(iter);
					continue;
				}
				::CONT_EMOTION const * pContEmotion = NULL;
				g_kTblDataMgr.GetContDef(pContEmotion);	//맵 데프
				if( !pContEmotion )
				{// 이모션에 대한 DB가 존재해야 하고
					return false;
				}
				::CONT_EMOTION::const_iterator def_iter = pContEmotion->find(SEMOTIONKEY(ET_EMOTION, (*kEmotiArray.begin())));
				if( def_iter == pContEmotion->end() )
				{
					iter = kArray.erase(iter);
					continue;
				}
				PgBitArray<MAX_DB_EMOTICON_SIZE> kCont;
				if( pkPlayer->GetContEmotion(ET_EMOTION, kCont) )
				{// Player로 부터 이모션, 이모티콘등의 컨테이너를 얻어온다
					if( !kCont.Get(def_iter->second) )
					{
						iter = kArray.erase(iter);
						continue;
					}
				}
				++iter;
			}
		}
		return !kArray.empty();
	}
}

PgEmotiMgr::PgEmotiMgr()
{
}

PgEmotiMgr::~PgEmotiMgr()
{
}

bool PgEmotiMgr::ParseXml(TiXmlNode const* pkNode, void* pArg, bool bUTF8)
{
	int const iType = pkNode->Type();
	
	while(pkNode)
	{
		switch(iType)
		{
		case TiXmlNode::ELEMENT:
			{
				TiXmlElement *pkElement = (TiXmlElement *)pkNode;
				PG_ASSERT_LOG(pkElement);
				
				char const *pcTagName = pkElement->Value();

				if(strcmp(pcTagName, "LINE_COMMAND")==0)
				{
					TiXmlElement* pkSubElem = pkElement->FirstChildElement();
					while( pkSubElem )
					{
						pcTagName = pkSubElem->Value();
						if(strcmp(pcTagName, "EMOTION_KEYWORD")==0)
						{
							if( !ParseEmotionXml(pkSubElem->FirstChildElement()) )
							{
								//Error
							}
						}
						else if(strcmp(pcTagName, "BALLOON_KEYWARD") == 0)
						{
							if( !ParseBalloonKeywardXml(pkSubElem->FirstChildElement()) )
							{
								//Error
							}
						}
						else
						{
							return false;					
						}
						pkSubElem = pkSubElem->NextSiblingElement();
					}
				}
			}
		}
		pkNode = pkNode->NextSibling();
	}
	return OnBuild();
}

bool PgEmotiMgr::OnBuild()
{
	::CONT_EMOTION const * pContEmotion = NULL;
	g_kTblDataMgr.GetContDef(pContEmotion);	//맵 데프
	if( !pContEmotion )
	{
		return false;
	}

	::CONT_EMOTION_GROUP const * pContEmotiGroup = NULL;
	g_kTblDataMgr.GetContDef(pContEmotiGroup);	//맵 데프
	if( !pContEmotiGroup )
	{
		return false;
	}

	CONT_EMOTION_GROUP::const_iterator g_iter = pContEmotiGroup->begin();
	while( g_iter != pContEmotiGroup->end() )
	{
		CONT_BUILDDATA* pCont = NULL;
		switch( g_iter->first.bType )
		{
		case ET_EMOTICON:	{ pCont = &m_kEmoticon; }break;
		case ET_EMOTION:	{ pCont = &m_kEmotion;  }break;
		case ET_BALLOON:	{ pCont = &m_kBalloon;  }break;
		default: 
			{ 
				++g_iter;
				continue; 
			}break;
		}

		if( pCont )
		{
			CONT_IDARRAY	kArray;
			std::pair<CONT_BUILDDATA::iterator, bool> result = pCont->insert(std::make_pair(g_iter->first.iGroupNo, kArray));
			if( !result.second )
			{ 
				assert(0);
			}
		}
		++g_iter;
	}

	::CONT_EMOTION::const_iterator c_iter = pContEmotion->begin();
	while( c_iter != pContEmotion->end() )
	{
		switch( c_iter->first.bType )
		{
		case ET_EMOTICON:
			{
				AddBuildData(m_kEmoticon, c_iter->second, c_iter->first.iEMotionNo);
			}break;
		case ET_EMOTION:
			{
				AddBuildData(m_kEmotion, c_iter->second, c_iter->first.iEMotionNo);
			}break;
		case ET_BALLOON:
			{
				AddBuildData(m_kBalloon, c_iter->second, c_iter->first.iEMotionNo);
			}break;
		default: break;
		}
		++c_iter;
	}

	return true;
}

bool PgEmotiMgr::ParseEmotionXml(TiXmlElement const* pkElement)
{
	while(pkElement)
	{
		char const* pkEleName = pkElement->Value();
		if( strcmp( pkEleName, "EMOTION" ) == 0 )
		{
			tagEmotion	kEmotion;

			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while( pkAttr )
			{
				char const* pcTagName = pkAttr->Name();
				char const* pcTagValue = pkAttr->Value();

				if( strcmp( pcTagName, "ID" ) == 0 )
				{
					kEmotion.ID(atoi(pcTagValue));
				}
				else if( strcmp( pcTagName, "ACTION_NAME" ) == 0 )
				{
					kEmotion.ActionID(UNI(pcTagValue));
				}
				else if( strcmp( pcTagName, "INPUT" ) == 0 )
				{
					BM::vstring kStr(pcTagValue);
					kEmotion.ParseKeyward(kStr);
				}
				else if( strcmp( pcTagName, "ICON_PATH" ) == 0 )
				{
					kEmotion.IconPath(UNI(pcTagValue));
				}
				else
				{
					//None Def TagName
				}
				pkAttr = pkAttr->Next();
			}

			TiXmlElement const* pSubParam = pkElement->FirstChildElement();
			while( pSubParam )
			{
				std::wstring kKey, kVal;

				TiXmlAttribute const* pkAttr = pSubParam->FirstAttribute();
				while( pkAttr )
				{
					char const* pcTagName = pkAttr->Name();
					char const* pcTagValue = pkAttr->Value();

					if( strcmp( pcTagName, "KEY" ) == 0 )
					{
						kKey = UNI(pcTagValue);
					}
					else if( strcmp( pcTagName, "VALUE" ) == 0 )
					{
						kVal = UNI(pcTagValue);
					}
					else
					{
						//None Def TagName
					}
					pkAttr = pkAttr->Next();
				}
				kEmotion.AddParam(kKey, kVal);
				pSubParam = pSubParam->NextSiblingElement();
			}

			std::pair<CONT_EMOTION::iterator, bool> Result = m_kEmotionContainer.insert(std::make_pair(kEmotion.ID(), kEmotion));
			if( !Result.second )
			{
				//Same ID
			}
		}
		pkElement = pkElement->NextSiblingElement();
	}

	return true;
}

bool PgEmotiMgr::ParseBalloonKeywardXml(TiXmlElement const* pkElement)
{
	while(pkElement)
	{
		char const* pkEleName = pkElement->Value();
		if( strcmp( pkEleName, "BALLOON" ) == 0 )
		{
			tagEmotiExtend kBalloon;

			TiXmlAttribute const* pkAttr = pkElement->FirstAttribute();
			while( pkAttr )
			{
				char const* pcTagName = pkAttr->Name();
				char const* pcTagValue = pkAttr->Value();

				if( strcmp( pcTagName, "ID" ) == 0 )
				{
					kBalloon.ID(atoi(pcTagValue));
				}
				else if( strcmp( pcTagName, "INPUT" ) == 0 )
				{
					BM::vstring kStr(pcTagValue);
					kBalloon.ParseKeyward(kStr);
				}
				else if( strcmp( pcTagName, "ICON_PATH" ) == 0 )
				{
					kBalloon.IconPath(UNI(pcTagValue));
				}
				else
				{
					//None Def TagName
				}
				pkAttr = pkAttr->Next();
			}

			std::pair<CONT_BALLOON_EMOTI::iterator, bool> Result = m_kBalloonEmoticonContainer.insert(std::make_pair(kBalloon.ID(), kBalloon));
			if( !Result.second )
			{
				//Same ID
			}
		}
		pkElement = pkElement->NextSiblingElement();
	}
	return true;
}

bool PgEmotiMgr::AddBuildData(CONT_BUILDDATA& kCont, int const GroupID, int const EmotiID)
{
	CONT_BUILDDATA::iterator	iter = kCont.find(GroupID);
	if( iter == kCont.end() )
	{
		return false;
	}
	iter->second.push_back(EmotiID);
	return true;
}

bool PgEmotiMgr::FindEmotionByKeyward(std::wstring const &InWord, int& ID, std::wstring::size_type const pos ) const
{
	std::vector<std::wstring> kKeywordCont;
	CONT_INT kContID;

	CONT_EMOTION::const_iterator c_iter = m_kEmotionContainer.begin();
	while( c_iter != m_kEmotionContainer.end() )
	{
		CONT_KEYWORD::const_iterator keyword_iter = c_iter->second.KeywordCont().begin();
		while( keyword_iter != c_iter->second.KeywordCont().end() )
		{
			std::wstring::size_type const find_pos = InWord.find(*keyword_iter);
			
			if ( std::wstring::npos != find_pos )
			{
				// 찾긴 찼았다.
				if ( (pos == find_pos) || (pos == std::wstring::npos) )
				{
					kKeywordCont.push_back(*keyword_iter);
					kContID.push_back(c_iter->first);
				}
			}
			++keyword_iter;
		}
		++c_iter;
	}
	if( !kKeywordCont.empty() 
		&& !kContID.empty() 
		&& kKeywordCont.size() == kContID.size()
		)
	{// 찾은 것들 중에서
		int iSizestIdx = 0;
		std::wstring kStr;
		for(int i =0; i < kKeywordCont.size(); ++i)
		{
			if( kStr.empty()
				|| kStr.size() < kKeywordCont.at(i).size()
				)
			{// 키워드 사이즈가 더 큰것이 보다 정확하므로 ex) - '안녕하세요'를 입력하고, '안녕', '안녕하세요'를 찾을수 있지만, 결과적으로 더 긴 '안녕하세요'가 정확하다
				kStr = kKeywordCont.at(i);
				iSizestIdx = i;
			}
		}
		if( kContID.size() > iSizestIdx )
		{// 해당 ID를 결과로 넘겨준다.
			ID = kContID.at(iSizestIdx);
			return true;
		}
	}
	return false;
}

bool PgEmotiMgr::FindBalloonByKeyward(std::wstring const &InWord, int& ID, std::wstring::size_type const pos ) const
{
	CONT_BALLOON_EMOTI::const_iterator c_iter = m_kBalloonEmoticonContainer.begin();
	while( c_iter != m_kBalloonEmoticonContainer.end() )
	{
		CONT_KEYWORD::const_iterator keyword_iter = c_iter->second.KeywordCont().begin();
		while( keyword_iter != c_iter->second.KeywordCont().end() )
		{
			std::wstring::size_type const find_pos = InWord.find(*keyword_iter);
			if ( std::wstring::npos != find_pos )
			{
				// 찾긴 찼았다.
				if ( (pos == find_pos) || (pos == std::wstring::npos) )
				{
					ID = c_iter->first;
					return true;
				}
			}
			++keyword_iter;
		}
		++c_iter;
	}
	return false;
}

PgEmotiMgr::tagEmotion const &PgEmotiMgr::GetEmotion(int const ID)const
{
	CONT_EMOTION::const_iterator c_iter = m_kEmotionContainer.find(ID);
	if( c_iter != m_kEmotionContainer.end() )
	{
		return c_iter->second;
	}
	static tagEmotion kEmotion;
	return kEmotion;
}

std::wstring const& PgEmotiMgr::GetBalloonIcon(int const ID)
{
	CONT_BALLOON_EMOTI::const_iterator c_iter = m_kBalloonEmoticonContainer.find(ID);
	if( c_iter != m_kBalloonEmoticonContainer.end() )
	{
		return c_iter->second.IconPath();
	}
	static std::wstring kTemp;
	return kTemp;
}

std::wstring const PgEmotiMgr::GetChatString(EEMOTION_TYPE Type, int const ID)
{
	BM::vstring kText;
	switch( Type )
	{
	case ET_EMOTICON:
		{
			kText = L"#";
			kText += ID;
		}break;
	case ET_EMOTION: 
		{ 
			CONT_EMOTION::const_iterator c_iter = m_kEmotionContainer.find(ID);
			if( c_iter != m_kEmotionContainer.end() )
			{
				kText = c_iter->second.GetStringToolTip();
			}
		}break;
	case ET_BALLOON: 
		{ 
			CONT_BALLOON_EMOTI::const_iterator c_iter = m_kBalloonEmoticonContainer.find(ID);
			if( c_iter != m_kBalloonEmoticonContainer.end() )
			{
				kText = c_iter->second.GetStringToolTip();
			}
		}break;
	}
	return std::wstring(kText);
}

bool PgEmotiMgr::GetEmotiGroupArray(EEMOTION_TYPE Type, CONT_IDARRAY& kArray)
{
	CONT_BUILDDATA const* pCont = NULL;
	switch( Type )
	{
	case ET_EMOTICON:{ pCont = &m_kEmoticon; }break;
	case ET_EMOTION: { pCont = &m_kEmotion;	 }break;
	case ET_BALLOON: { pCont = &m_kBalloon;  }break;
	default: return false;
	}

	if( pCont )
	{
		CONT_BUILDDATA::const_iterator c_iter = pCont->begin();
		while( c_iter != pCont->end() )
		{
			kArray.push_back(c_iter->first);
			++c_iter;
		}
		return true;
	}
	return false;
}

bool PgEmotiMgr::GetEmotiGroupItem(EEMOTION_TYPE Type, int const GroupID, CONT_IDARRAY& kArray)
{
	CONT_BUILDDATA const* pCont = NULL;
	switch( Type )
	{
	case ET_EMOTICON:{ pCont = &m_kEmoticon; }break;
	case ET_EMOTION: { pCont = &m_kEmotion;	 }break;
	case ET_BALLOON: { pCont = &m_kBalloon;  }break;
	default: return false;
	}

	if( pCont )
	{
		CONT_BUILDDATA::const_iterator c_iter = pCont->find(GroupID);
		if( c_iter != pCont->end() )
		{
			kArray.insert(kArray.end(), c_iter->second.begin(), c_iter->second.end());
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------------------------------
//	Item Option Change UI
//------------------------------------------------------------------------------------------------------------------
int const MAX_LUCKY_ITEM_SLOT = 4;

namespace lwLuckyChanger
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "OnClickLuckyUISlot", lwOnClickLuckyUISlot);
		def(pkState, "OnOKLuckyChange", lwOnOKLuckyChange);
	};

	void lwOnClickLuckyUISlot(lwUIWnd UISelf)
	{
		XUI::CXUI_CheckButton* pSelf = dynamic_cast<XUI::CXUI_CheckButton*>(UISelf.GetSelf());
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pParent->Parent();
		if( !pMainUI )
		{
			return;
		}

		for(int i = 0; i < MAX_LUCKY_ITEM_SLOT; ++i)
		{
			BM::vstring vStr(L"FRM_ITEM");
			vStr += i;

			XUI::CXUI_Wnd* pItem = pMainUI->GetControl(vStr);
			if( pItem )
			{
				XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pItem->GetControl(L"CBTN_ITEM"));
				if( pCheckBtn )
				{
					pCheckBtn->ClickLock(false);
					pCheckBtn->Check(false);
				}
			}
		}

		pSelf->Check(true);
		pSelf->ClickLock(true);
	}

	void lwOnOKLuckyChange(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( !pSelf )
		{
			return;
		}

		XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
		if( !pMainUI )
		{
			return;
		}
		
		SItemPos kTargetItemPos;
		XUI::CXUI_Wnd* pIcon = pMainUI->GetControl(L"ICN_ICON");
		if( pIcon )
		{
			pIcon->GetCustomData(&kTargetItemPos, sizeof(kTargetItemPos));
		}

		SItemPos kResetItemPos;
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		int SelectOptionSlot = 0;

		PgInventory* pkInv = pkPlayer->GetInven();
		if( pkInv )
		{
			PgBase_Item kItem;
			if(S_OK != pkInv->GetItem(kTargetItemPos,kItem))
			{
				return;
			}

			CONT_ENCHANT_ABIL kContAbil;
			GetDefaultOption(kItem, kContAbil);

			CONT_ENCHANT_ABIL::const_iterator iter = kContAbil.begin();

			for(int i = 0; i < MAX_LUCKY_ITEM_SLOT; ++i)
			{
				if(iter == kContAbil.end())
				{
					return;
				}

				CONT_ENCHANT_ABIL::value_type const & kAbil = (*iter);
				++iter;
		
				BM::vstring vStr(L"FRM_ITEM");
				vStr += i;

				XUI::CXUI_Wnd* pItem = pMainUI->GetControl(vStr);
				if( pItem )
				{
					XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pItem->GetControl(L"CBTN_ITEM"));
					if( pCheckBtn )
					{
						if( pCheckBtn->Check() )
						{
							SelectOptionSlot = kAbil.iIdx;//i;
							break;
						}
					}
				}
			}

			for(int i = IT_EQUIP; i < IT_SAFE; ++i)
			{
				switch( i )
				{
				case IT_EQUIP:
				case IT_ETC:
					{ 
						continue; 
					}break;
				}

				int const MAX_IDX = pkInv->GetMaxIDX((EInvType)i);
				for(int j = 0; j < MAX_IDX; ++j)
				{
					SItemPos kPos(i, j);
					PgBase_Item kItem;

					if( S_OK != pkInv->GetItem(kPos, kItem) )
					{
						continue;
					}

					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const* pDef = kItemDefMgr.GetDef(kItem.ItemNo());
					if( !pDef )
					{
						assert(0);
						continue;
					}

					if( UICT_ROLLBACK == pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
					{
						kResetItemPos = kPos;
						break;
					}
				}
			}
		}

		if( SItemPos::NullData() == kTargetItemPos )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50616, true);
			return;
		}
		else if( SItemPos::NullData() == kResetItemPos )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50617, true);
			return;
		}

		BM::Stream kPacket(PT_C_M_REQ_ROLLBACK_ENCHANT);
		kPacket.Push(SelectOptionSlot);
		kPacket.Push(kTargetItemPos);
		kPacket.Push(kResetItemPos);
		lwCallCommonMsgYesNoBox(MB(TTW(50615)), lwPacket(&kPacket), true, MBT_COMMON_YESNO_TO_PACKET);
		pMainUI->Close();
	}

	void ReceivePacket_Command(WORD const wPacketType, BM::Stream& rkPacket)
	{
		switch( wPacketType )
		{
		case PT_M_C_ANS_ROLLBACK_ENCHANT:
			{
				HRESULT kResult;
				rkPacket.Pop(kResult);
				switch( kResult )
				{
				case EC_OK:
					{
						lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50604, true);
					}break;
				case E_RBE_NOT_FOUND_TARGET_ITEM:
				case E_RBE_NOT_FOUND_ROLLBACK_ITEM:
				case E_RBE_NOT_FOUND_ENCHANTINFO:
					{
						lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50605 + (kResult - E_RBE_NOT_FOUND_TARGET_ITEM), true);
					}break;
				default: return;
				}
			}break;
		}
	}

	void CallLuckyChanger()
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_LUCKY_CHANGER");
		if( pMainUI )
		{
			XUI::CXUI_Wnd* pMainBg = pMainUI->GetControl(L"SFRM_BG");
			if( !pMainBg )
			{
				return;
			}

			XUI::CXUI_Wnd* pIcon = pMainUI->GetControl(L"ICN_ICON");
			if( pIcon )
			{
				pIcon->ClearCustomData();
			}

			for(int i = 0; i < MAX_LUCKY_ITEM_SLOT; ++i)
			{
				BM::vstring vStr(L"FRM_ITEM");
				BM::vstring vStrBg(L"SFRM_ITEMBG");
				vStr += i;
				vStrBg+= i;

				XUI::CXUI_Wnd* pItem = pMainUI->GetControl(vStr);
				if( pItem )
				{
					pItem->Visible(false);
				}

				XUI::CXUI_Wnd* pItemBg = pMainBg->GetControl(vStrBg);
				if( pItemBg )
				{
					pItemBg->Visible(false);
				}
			}
		}
	}

	void OnDragUpItem(XUI::CXUI_Wnd* pSelf)
	{
		if( !pSelf )
		{
			return;
		}

		SItemPos	kTargetItemPos;
		pSelf->GetCustomData(&kTargetItemPos, sizeof(SItemPos));

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( pkInv )
		{
			PgBase_Item kItem;
			if( S_OK == pkInv->GetItem(kTargetItemPos, kItem) )
			{
				XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
				if( !pMainUI )
				{
					return;
				}

				XUI::CXUI_Wnd* pMainBg = pMainUI->GetControl(L"SFRM_BG");
				if( !pMainBg )
				{
					return;
				}

				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
				if(!pDef)
				{
					return;
				}

				CONT_ENCHANT_ABIL kEnchantAbil;
				GetDefaultOption(kItem, kEnchantAbil);

				//아이템 생성시 정해지는 랜덤 옵션
				if( !kEnchantAbil.empty() )
				{
					CONT_ENCHANT_ABIL::const_iterator kItor = kEnchantAbil.begin();

					for(int i = 0; i < MAX_LUCKY_ITEM_SLOT; ++i)
					{
						BM::vstring vStr(L"FRM_ITEM");
						BM::vstring vStrBg(L"SFRM_ITEMBG");
						vStr += i;
						vStrBg+= i;

						XUI::CXUI_Wnd* pItem = pMainUI->GetControl(vStr);
						XUI::CXUI_Wnd* pItemBg = pMainBg->GetControl(vStrBg);
						if( !pItem || !pItemBg )
						{
							continue;
						}

						if(kItor != kEnchantAbil.end() )
						{
							std::wstring kAbilString;
							MakeAbilString(kItem, pDef, (*kItor).wType, (*kItor).iValue, kAbilString);//레벨
							
							++kItor;

							pItem->Visible(true);
							pItemBg->Visible(true);
							pItem->Text(kAbilString);
							XUI::CXUI_CheckButton* pCheckBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pItem->GetControl(L"CBTN_ITEM"));
							if( pCheckBtn )
							{
								pCheckBtn->Check(i == 0);
								pCheckBtn->ClickLock(pCheckBtn->Check());
							}
						}
						else
						{
							pItem->Visible(false);
							pItemBg->Visible(false);
						}
					}
				}
				else
				{
					for(int i = 0; i < MAX_LUCKY_ITEM_SLOT; ++i)
					{
						BM::vstring vStr(L"FRM_ITEM");
						BM::vstring vStrBg(L"SFRM_ITEMBG");
						vStr += i;
						vStrBg+= i;

						XUI::CXUI_Wnd* pItem = pMainUI->GetControl(vStr);
						if( pItem )
						{
							pItem->Visible(false);							
						}

						XUI::CXUI_Wnd* pItemBg = pMainBg->GetControl(vStrBg);
						if( pItemBg )
						{
							pItemBg->Visible(false);
						}
					}
				}
			}
		}
	}

	bool SetItem(SItemPos const & kItemPos)
	{
		if( kItemPos.x == KUIG_CONSUME 
		||  kItemPos.x == KUIG_ETC )
		{
			return false;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer || !pkPlayer->GetInven() ){ return false; }
		PgInventory *pkInv = pkPlayer->GetInven();
		
		PgBase_Item kItem;
		if(S_OK != pkInv->GetItem(kItemPos, kItem))
		{
			return false;
		}

		E_ITEM_GRADE const Grade = GetItemGrade(kItem);
		switch( Grade )
		{
		case IG_SEAL:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50617, true);
			}return false;
		case IG_CURSE:
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50618, true);
			}return false;
		}

		CONT_ENCHANT_ABIL kEnchantAbil;
		GetDefaultOption(kItem, kEnchantAbil);
		if( true == kEnchantAbil.empty() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50607, true);
			return false;
		}

		if( true == kItem.EnchantInfo().IsBinding() )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50609, true);
			return false;
		}
		if( LOCAL_MGR::NC_JAPAN == g_kLocal.ServiceRegion() 
			&& CheckIsCashItem(kItem))
		{//일본의 경우 캐시 아이템은 소울 크래프트 관련 작업 불가
			lwAddWarnDataTT(59002);
			return false;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( !pDef )
		{
			return false;
		}
		SEnchantInfo const& kEhtInfo = kItem.EnchantInfo();
		int const iAbil = pDef->GetAbil(AT_ATTRIBUTE);
		if( (iAbil & ICMET_Cant_Rollback) == ICMET_Cant_Rollback )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50608, true);
			return false;
		}

		if( !pDef->CanEquip() )
		{
			return false;
		}

		return true;
	}
};

//------------------------------------------------------------------------------------------------------------------
//	경험치 카드
//------------------------------------------------------------------------------------------------------------------
namespace lwAccumlationExpCard
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "CallFullAccumulateCardUse", lwCallFullAccumulateCardUse);
		def(pkState, "UdateAccumulateCardUsing", lwUdateAccumulateCardUsing);
		def(pkState, "OnClickFullAccumulateCardUseOK", lwOnClickFullAccumulateCardUseOK);
		def(pkState, "OnClickFullAccumulateCardUseRealOK", lwOnClickFullAccumulateCardUseRealOK);
		def(pkState, "CheckUsingExpCard", lwCheckUsingExpCard);
		def(pkState, "CheckExpCardPercentText", lwCheckExpCardPercentText);
	}

	void lwCallFullAccumulateCardUse(lwGUID kGuid)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return;
		}

		PgBase_Item kItem;
		SItemPos	kPos;
		if( S_OK == pkInv->GetItem(kGuid.GetGUID(), kItem, kPos) )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( !pDef )
			{
				return;
			}

			SExpCard kExpInfo;
			if( UICT_EXPCARD != pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE)
			|| !kItem.Get(kExpInfo) )
			{
				return;
			}

			int const iCardLevel = pDef->GetAbil(AT_MAX_LEVELLIMIT);
			int iBookItemNo = 0;

			ContHaveItemNoCount kCont;
			if( S_OK == pkInv->GetItems(UICT_EXPCARD_BREAKER, kCont) )
			{
				ContHaveItemNoCount::iterator item_iter = kCont.begin();
				while( item_iter != kCont.end() )
				{
					ContHaveItemNoCount::key_type const& kItemNo = item_iter->first;
					CItemDef const *pBookDef = kItemDefMgr.GetDef(kItemNo);
					if( !pBookDef )
					{
						continue;
					}

					int const iBookLevel = pBookDef->GetAbil(AT_MAX_LEVELLIMIT);
					if( iCardLevel == iBookLevel )
					{
						iBookItemNo = kItemNo;
						break;
					}
					++item_iter;
				}
			}

			if( 0 == iBookItemNo )
			{
				lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50766, true);
				return;
			}

			SItemPos	kBookPos;
			if( S_OK != pkInv->GetFirstItem(iBookItemNo, kBookPos) )
			{
				return;
			}

			PgBase_Item	kBookItem;
			if( S_OK != pkInv->GetItem(kBookPos, kBookItem) )
			{
				return;
			}

			XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_ACCUMULATED_EXPCARD_USE");
			if( !pMainUI )
			{
				return;
			}

			XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(L"ICN_EXP");
			if( pTemp )
			{
				pTemp->OwnerGuid(kItem.Guid());
				pTemp->SetCustomData(&kItem.ItemNo(), sizeof(kItem.ItemNo()));
			}

			pTemp = pMainUI->GetControl(L"ICN_BOOK");
			if( pTemp )
			{
				pTemp->OwnerGuid(kBookItem.Guid());
				pTemp->SetCustomData(&kBookItem.ItemNo(), sizeof(kBookItem.ItemNo()));
			}

			pTemp = pMainUI->GetControl(L"BTN_USE");
			if( pTemp )
			{
				pTemp->OwnerGuid(kGuid.GetGUID());
			}
		}
	}

	void lwUdateAccumulateCardUsing(lwGUID kGuid)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return;
		}

		PgBase_Item kItem;
		SItemPos	kPos;
		if( S_OK == pkInv->GetItem(kGuid.GetGUID(), kItem, kPos) )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( !pDef )
			{
				return;
			}

			SExpCard kExpInfo;
			if( UICT_EXPCARD == pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE)
			&& kItem.Get(kExpInfo)
			&& pkInv->ExpCardItem() == kGuid.GetGUID() )
			{
				std::wstring const* kName = NULL;
				GetDefString( pDef->NameNo(), kName);
				XUI::CXUI_Wnd* pMainUI = XUIMgr.Get(L"FRM_ACCUMULATION_EXPCARD");
				if( !pMainUI )
				{
					pMainUI = XUIMgr.Call(L"FRM_ACCUMULATION_EXPCARD");
					if( !pMainUI )
					{
						return;
					}
				}
				pMainUI->Text(kName->c_str());

				XUI::CXUI_Wnd* pTemp = pMainUI->GetControl(L"ICN_EXPCARD");
				if( pTemp )
				{
					pTemp->OwnerGuid(kGuid.GetGUID());
					pTemp->SetCustomData(&kItem.ItemNo(), sizeof(kItem.ItemNo()));
				}

				int const iPercent = static_cast<int>((kExpInfo.CurExp() / static_cast<double>(kExpInfo.MaxExp())) * 100);
				pTemp = pMainUI->GetControl(L"FRM_ACCUM_RATE");
				if( pTemp )
				{
					pTemp->FontColor(0xFFFFFF00);
					if( iPercent == 100 )
					{
						pTemp->FontColor(0xFF00FF00);
					}
					pTemp->Text(BM::vstring(iPercent));
				}

				pTemp = pMainUI->GetControl(L"ANI_ACCUM_RATE_IMG");
				if( pTemp )
				{
					XUI::CXUI_AniBar* pAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pTemp);
					if( pAniBar )
					{
						pAniBar->Max(100);
						pAniBar->Now(iPercent);
					}
				}

				if( 100 == iPercent )
				{
					pMainUI->Close();
					pkInv->ExpCardItem(BM::GUID::NullData());
					lwAddWarnDataTT(50765);
				}
			}
		}
	}

	void lwOnClickFullAccumulateCardUseOK(lwUIWnd kSelf)
	{
		XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(kSelf.GetSelf());
		if( !pButton )
		{
			return;
		}

		BM::GUID const kGuid = pButton->OwnerGuid();
		if( kGuid.IsNotNull() )
		{
			SItemPos kCardPos;
			int iCardLevel = 0;
			__int64 iExpValue = 0;
			int iNameNo = 0;
			if( GetExpToCardItem(kGuid, kCardPos, iCardLevel, iExpValue, iNameNo) )
			{
				SItemPos kItemPos;
				if( GetExpToBookItem(iCardLevel, kItemPos) )
				{
					XUI::CXUI_Wnd* pMainUI = XUIMgr.Call(L"SFRM_ACCUMULATED_EXPCARD_USE2", true);
					if( !pMainUI )
					{
						return;
					}

					XUI::CXUI_Wnd* pText = pMainUI->GetControl(L"SFRM_BG");
					if( pText )
					{
						std::wstring const* kStrName = NULL;
						GetDefString(iNameNo, kStrName);
						wchar_t szTemp[MAX_PATH] = {0,};
						swprintf_s(szTemp, MAX_PATH, TTW(50754).c_str(), kStrName->c_str(), iExpValue);
						pText->Text(szTemp);

						XUI::CXUI_Style_String kString = pText->StyleText();
						POINT2 const kTextSize(Pg2DString::CalculateOnlySize(kString));
						pText->TextPos(POINT2(pText->TextPos().x, (pText->Size().y - kTextSize.y) * 0.5f));
					}

					XUI::CXUI_Wnd* pWnd = pMainUI->GetControl(L"BTN_USE");
					if( pWnd )
					{
						pWnd->OwnerGuid(kGuid);
					}
				}
			}
		}
		pButton->Parent()->Close();
	}

	void lwOnClickFullAccumulateCardUseRealOK(lwUIWnd kSelf)
	{
		XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(kSelf.GetSelf());
		if( !pButton )
		{
			return;
		}

		BM::GUID const kGuid = pButton->OwnerGuid();
		if( kGuid.IsNotNull() )
		{
			SItemPos kCardPos;
			int iCardLevel = 0;
			__int64 iExpValue = 0;
			int iNameNo = 0;
			if( GetExpToCardItem(kGuid, kCardPos, iCardLevel, iExpValue, iNameNo) )
			{
				SItemPos kItemPos;
				if( GetExpToBookItem(iCardLevel, kItemPos) )
				{
					BM::Stream kPacket(PT_C_M_REQ_EXPCARD_USE);
					kPacket.Push(kCardPos);
					kPacket.Push(kItemPos);
					NETWORK_SEND(kPacket);
				}
			}
		}
		pButton->Parent()->Close();
	}

	void lwCheckUsingExpCard()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{ 
			return; 
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return;
		}

		if( pkInv->ExpCardItem() != BM::GUID::NullData() )
		{
			lwUdateAccumulateCardUsing(lwGUID(pkInv->ExpCardItem()));
		}
	}

	void lwCheckExpCardPercentText(lwUIWnd UISelf)
	{
		XUI::CXUI_Wnd* pSelf = UISelf.GetSelf();
		if( pSelf )
		{
			pSelf->Text(L"");
			BM::GUID const& kGuid = pSelf->OwnerGuid();
			if( kGuid != BM::GUID::NullData() )
			{
				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pkPlayer )
				{ 
					return; 
				}

				PgInventory* pkInv = pkPlayer->GetInven();
				if( !pkInv )
				{
					return;
				}

				PgBase_Item kItem = lwUIWnd(pSelf).GetCustomDataAsSItem();
				if( !kItem.IsEmpty() && kItem.Guid() == kGuid )
				{
					SExpCard kCard;
					if( kItem.Get(kCard) )
					{
						int iPercent = static_cast<int>((kCard.CurExp() / static_cast<double>(kCard.MaxExp())) * 100);
						pSelf->FontColor((iPercent == 100)?(0xFF00FF00):(0xFFFFFF00));
						pSelf->Text(BM::vstring(iPercent));
					}
				}
			}
		}
	}

	void ReceivePacket_Command(WORD const wPacketType, BM::Stream& rkPacket)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{ 
			return; 
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return;
		}

		switch( wPacketType )
		{
		case PT_M_C_ANS_EXPCARD_ACTIVATE:
			{
				HRESULT kResult = E_FAIL;
				rkPacket.Pop(kResult);
				if( S_OK != kResult )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50757 + (kResult - E_EXPCARD_NOT_FOUND_ITEM), true);
				}
				else
				{
					PgUIData_ExpCard* pkExpData = NULL;
					if( g_kUIDataMgr.Get(UIDATA_EXPCARD, pkExpData) )
					{
						pkInv->ExpCardItem(pkExpData->m_kActivateGuid);
						lwUdateAccumulateCardUsing(pkInv->ExpCardItem());
						g_kUIDataMgr.Remove(UIDATA_EXPCARD);
					}
				}
			}break;
		case PT_M_C_ANS_EXPCARD_DEACTIVATE:
			{
				HRESULT kResult = E_FAIL;
				rkPacket.Pop(kResult);
				if( S_OK != kResult )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50757 + (kResult - E_EXPCARD_NOT_FOUND_ITEM), true);
				}
				else
				{
					pkInv->ExpCardItem(BM::GUID::NullData());
					XUIMgr.Close(L"FRM_ACCUMULATION_EXPCARD");
				}
			}break;
		case PT_M_C_ANS_EXPCARD_USE:
			{
				HRESULT	kResult = E_FAIL;
				__int64 kExp = 0;
				rkPacket.Pop(kResult);
				if( S_OK != kResult )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50757 + (kResult - E_EXPCARD_NOT_FOUND_ITEM), true);
				}
				else
				{
					rkPacket.Pop(kExp);
					wchar_t	szTemp[MAX_PATH] = {0,};
					swprintf_s(szTemp, MAX_PATH, TTW(50756).c_str(), kExp);
					lua_tinker::call<void, char const*, bool>("CommonMsgBox", MB(szTemp), true);
				}
			}break;
		}
	}

	void SendActivateExpCard(SItemPos kPos)
	{
		BM::Stream kPacket(PT_C_M_REQ_EXPCARD_ACTIVATE);
		kPacket.Push(kPos);
		NETWORK_SEND(kPacket);
	}

	void SendDeactivateExpCard()
	{
		BM::Stream kPacket(PT_C_M_REQ_EXPCARD_DEACTIVATE);
		NETWORK_SEND(kPacket);
	}

	void CheckUseExpCard(SItemPos const& kItemPos)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{ 
			return; 
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( pkInv )
		{
			PgBase_Item kItem;
			if( S_OK != pkInv->GetItem(kItemPos, kItem) )
			{
				return;
			}

			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( !pItemDef )
			{
				return;
			}

			int const iType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
			if( iType != UICT_EXPCARD )
			{
				return;
			}

			SExpCard kExpCard;
			if( !kItem.Get(kExpCard) )
			{
				return;
			}

			if( kExpCard.CurExp() >= kExpCard.MaxExp() )
			{
				lwCallFullAccumulateCardUse(kItem.Guid());
			}
			else
			{
				BM::GUID kActivateGuid = pkInv->ExpCardItem();
				if( kItem.Guid() == kActivateGuid )
				{
					SendDeactivateExpCard();
				}
				else
				{
					PgUIData_ExpCard* pkExpData = new PgUIData_ExpCard;
					pkExpData->m_kActivateGuid = kItem.Guid();
					g_kUIDataMgr.Add(pkExpData);
					SendActivateExpCard(kItemPos);
				}
			}
		}
	}

	void CheckUsingExpCardInfo(XUI::CXUI_Wnd* pIcon, PgBase_Item const& kItem, bool const bIsBuild, bool const bHaveToggle)
	{
		if( !pIcon )
		{
			return;
		}

		XUI::CXUI_Wnd* pParent = pIcon->Parent();
		if( !pParent )
		{
			return;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{ 
			return; 
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return;
		}

		BM::vstring kStr(L"FRM_EXPCARD");
		if( bIsBuild )
		{
			kStr += pIcon->BuildIndex();
		}
		XUI::CXUI_Wnd* pRateform = pParent->GetControl(kStr);
		XUI::CXUI_Wnd* pToggleExp = NULL;
		if( bHaveToggle )
		{
			pToggleExp = pIcon->GetControl(L"FRM_TOGGLE_IMG");
		}

		if( !kItem.IsEmpty() )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( !pDef )
			{
				return;
			}

			switch(pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE))
			{
			case UICT_EXPCARD:
				{
					SExpCard kExpCard;
					if( !kItem.Get(kExpCard) )
					{
						return;
					}

					if( pToggleExp )
					{
						bool const bView = pkInv->ExpCardItem() == kItem.Guid();
						if( bView != pToggleExp->Visible() )
						{
							pToggleExp->Visible(bView);
						}
					}

					if( pRateform )
					{
						pRateform->ClearCustomData();
						lwUIWnd(pRateform).SetCustomDataAsSItem(kItem);
						if( pRateform->OwnerGuid() != kItem.Guid() )
						{
							pRateform->OwnerGuid(kItem.Guid());
							pRateform->Visible(true);
						}
					}
				}break;
			default:
				{
					if( pToggleExp && pToggleExp->Visible() )
					{
						pToggleExp->Visible(false);
					}
					if( pRateform && pRateform->Visible() )
					{
						pRateform->Visible(false);
						pRateform->OwnerGuid(BM::GUID::NullData());
						pRateform->ClearCustomData();
					}
				}break;
			}
		}
		else
		{
			if( pToggleExp && pToggleExp->Visible() )
			{
				pToggleExp->Visible(false);
			}
			if( pRateform && pRateform->Visible() )
			{
				pRateform->Visible(false);
				pRateform->OwnerGuid(BM::GUID::NullData());
			}
		}
	}

	bool GetExpToCardItem(BM::GUID const& kGuid, SItemPos& kItemPos, int& iLevel, __int64& iExpValue, int& iNameNo)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return false;
		}

		PgBase_Item kItem;
		if( S_OK == pkInv->GetItem(kGuid, kItem, kItemPos) )
		{
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CItemDef const *pDef = kItemDefMgr.GetDef(kItem.ItemNo());
			if( !pDef )
			{
				return false;
			}

			SExpCard kExpInfo;
			if( UICT_EXPCARD == pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE)
			&& kItem.Get(kExpInfo) )
			{
				iLevel = pDef->GetAbil(AT_MAX_LEVELLIMIT);
				iExpValue = kExpInfo.CurExp();
				iNameNo = pDef->NameNo();
				return true;
			}
		}
		lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50757, true);
		return false;
	}

	bool GetExpToBookItem(int const iExpLv, SItemPos& kItemPos)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{ 
			return false; 
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return false;
		}

		int iBookItemNo = 0;

		ContHaveItemNoCount kCont;
		if( S_OK == pkInv->GetItems(UICT_EXPCARD_BREAKER, kCont) )
		{
			ContHaveItemNoCount::iterator item_iter = kCont.begin();
			while( item_iter != kCont.end() )
			{
				ContHaveItemNoCount::key_type const& kItemNo = item_iter->first;

				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const *pDef = kItemDefMgr.GetDef(kItemNo);
				if( !pDef )
				{
					continue;
				}

				int const iBookLevel = pDef->GetAbil(AT_MAX_LEVELLIMIT);
				if( iExpLv == iBookLevel )
				{
					iBookItemNo = kItemNo;
					break;
				}
				++item_iter;
			}
		}

		if( 0 == iBookItemNo || S_OK != pkInv->GetFirstItem(iBookItemNo, kItemPos) )
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 50766, true);
			return false;
		}
		return true;
	}
};

void SendEmotionPacket(PgActor* pkActor, BYTE btEmotionType, int iEmotionID)
{
	if(pkActor && pkActor->IsActionShift())
	{
		BM::Stream kPacket(PT_C_M_REQ_USE_EMOTION);
		kPacket.Push(btEmotionType);
		kPacket.Push(iEmotionID);

		NiPoint3 kPos = pkActor->GetPosition(true);
		kPos.z -= PG_CHARACTER_Z_ADJUST;
		POINT3 ptPos(kPos.x, kPos.y, kPos.z);

		kPacket.Push(ptPos);

		NETWORK_SEND(kPacket);
	}
}

bool CheckCanEmotion(int const iEmotionID)
{
	// 실제 이모션에 해당 되는 액션번호를 얻어 와야 한다.
	PgEmotiMgr::tagEmotion const &kEmotion = g_kPgEmotiMgr.GetEmotion(iEmotionID);
	if( !kEmotion.IsEmpty() )
	{
		PgActor* pkMyActor = g_kPilotMan.GetPlayerActor();
		if(!pkMyActor)
		{
			return false;
		}

		// 넘어져있거나 공중에 뜬 경우라면 사용 불가
		if(pkMyActor->IsRidingPet() || pkMyActor->IsDownState() || pkMyActor->IsBlowUp())
		{
			return false;
		}

		if(!pkMyActor->IsMeetFloor())
		{
			return false;
		}

		// 임시 액션을 만들어 실제로 사용 가능한지 체크 한다.
		PgAction* pkAction = g_kActionPool.CreateAction(MB(kEmotion.ActionID()));
		if(!pkAction)
		{
			g_kActionPool.ReleaseAction(pkAction);
			return false;
		}

		// 실제로 사용가능하다면 불가능 하면
		if(false == pkAction->CheckCanEnter(pkMyActor, pkAction))
		{
			g_kActionPool.ReleaseAction(pkAction);
			return false;
		}

		g_kActionPool.ReleaseAction(pkAction);
	}

	return true;
}

//------------------------------------------------------------------------------------------------------------------
//	크래프트 옵션 체인저
//------------------------------------------------------------------------------------------------------------------
namespace lwCraftOptChanger
{
	wchar_t const* const SZ_MAIN_NAME = L"SFRM_SOUL_OPTION_CHANGER";
	wchar_t const* const SZ_OPT_SLOT_BLD1 = L"BLD_OPTION_SLOT1";
	wchar_t const* const SZ_OPT_SLOT_BLD2 = L"BLD_OPTION_SLOT2";
	wchar_t const* const SZ_OPT_SLOT = L"SFRM_OPTION_SLOT";
	wchar_t const* const SZ_OPT_SLOT_VALUE = L"SFRM_OPTION_VALUE";
	wchar_t const* const SZ_TARGET_ICON = L"ICN_ITEM_SLOT";
	wchar_t const* const SZ_SOUL_CHANGER_ICON = L"ICN_SOUL_SLOT";
	wchar_t const* const SZ_SOUL_CHANGER_VALUE = L"SFRM_SOUL_TEXT";
	wchar_t const* const SZ_SOUL_NOTICE = L"SFRM_SOUL_NOTICE";
	wchar_t const* const SZ_SOUL_ACTION = L"BTN_ACTION";
	wchar_t const* const SZ_SOUL_CANCEL = L"BTN_CANCEL";

	wchar_t const* const SZ_CRAFT_OPT_MODEL = L"EFFECT_VIEW";
	wchar_t const* const SZ_CRAFT_OPT_EFF1 = L"CraftSpin";
	wchar_t const* const SZ_CRAFT_OPT_EFF2 = L"CraftFlash";

	wchar_t const* const SZ_CRAFT_OPT_PROGRESS_SND = L"Item-try";
	wchar_t const* const SZ_CRAFT_OPT_SUCCESS_SND = L"Item-Success";

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "OnClickCraftOptChangeAction", lwOnClickCraftOptChangeAction);
		def(pkState, "OnEffectViewAndSending", lwOnEffectViewAndSending);
		def(pkState, "CloseCraftOptChanger", lwCloseCraftOptChanger);
		def(pkState, "UnRegCraftOptItem", lwUnRegCraftOptItem);
	}

	void Recv_ANS_Packet(WORD const wPacketType, BM::Stream& kPacket)
	{
		switch( wPacketType )
		{
		case PT_M_C_ANS_USE_REDICE_OPTION_ITEM:
			{
				HRESULT kResult = S_OK;
				kPacket.Pop(kResult);
				if( Recv_PT_X_X_ANS_ERROR_CHECK(kResult) )
				{
					SEnchantInfo kInfo;
					kPacket.Pop(kInfo);
					CallCraftOptChanger(kInfo);
				}
			}break;
		}
	}

	void lwOnClickCraftOptChangeAction(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( pSelf )
		{
			bool bResult = false;
			pSelf->GetCustomData(&bResult, sizeof(bResult));

			XUI::CXUI_Wnd* pkMain = pSelf->Parent();
			XUI::CXUI_Wnd* pkSoulText = pkMain->GetControl(SZ_SOUL_CHANGER_VALUE);
			if( pkMain && pkSoulText )
			{
				if( ECOS_NOTHING != pkMain->OwnerState() )
				{
					return;
				}

				SItemPos kItemPos;
				SItemPos kRediceItemPos;
				pkMain->GetCustomData(&kItemPos, sizeof(kItemPos));
				pkSoulText->GetCustomData(&kRediceItemPos, sizeof(kRediceItemPos));

				if( kItemPos == SItemPos::NullData() )
				{
					lwAddWarnDataTT(5813 + ECOE_NO_ITEM_REGIST);
					return;
				}

				if( kRediceItemPos == SItemPos::NullData() )
				{
					lwAddWarnDataTT(5813 + ECOE_NO_EXIST_OPT_CHANGE_ITEM);
					return;
				}

				if( bResult )
				{
					CallCraftOptChanger(kItemPos);
				}
				else
				{
					pkMain->OwnerState(ECOS_PREV_EFFECT_STARTED);
					XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pSelf);
					if( pkBtn )
					{
						pkBtn->Disable(true);
					}
				}
			}
		}
	}

	void lwOnEffectViewAndSending(lwUIWnd kSelf)
	{
		static float fStartTime = 0.0f;
		float const RARITY_PROGRESS_TIME = 0.667f; //진행 시간은 1초
		float const RARITY_FINISHED_TIME = 0.53f; //진행 시간은 1초

		XUI::CXUI_Wnd* pkMain = kSelf.GetSelf();
		if( pkMain )
		{
			PgUIModel* pkModel = g_kUIScene.FindUIModel(MB(SZ_CRAFT_OPT_MODEL));
			if( pkModel )
			{
				E_CRAFT_OPT_STATE const eState = static_cast<E_CRAFT_OPT_STATE>(pkMain->OwnerState());
				if( ECOS_NOTHING == eState )
				{
					return;
				}

				if( ECOS_NEXT_EFFECT_STARTED == pkMain->OwnerState() )
				{
					if( 0.0f == fStartTime )
					{
						fStartTime = g_pkApp->GetAccumTime();
						pkModel->SetNIFEnableUpdate(MB(SZ_CRAFT_OPT_EFF2), true);
						pkModel->ResetNIFAnimation(MB(SZ_CRAFT_OPT_EFF2));
						pkModel->SetEnableUpdate(true);
						g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, MB(SZ_CRAFT_OPT_PROGRESS_SND), 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
					}
					else
					{
						float fPassTime = g_pkApp->GetAccumTime() - fStartTime;
						if( fPassTime < RARITY_FINISHED_TIME )
						{
							return;
						}

						pkModel->SetNIFEnableUpdate(MB(SZ_CRAFT_OPT_EFF2), false);
						pkModel->SetEnableUpdate(false);
						pkMain->OwnerState(ECOS_NOTHING);
						fStartTime = 0.0f;

						XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkMain->GetControl(SZ_SOUL_ACTION));
						if( pkBtn )
						{
							pkBtn->Disable(false);
						}
					}
				}
				else
				{
					if( 0.0f == fStartTime )
					{
						fStartTime = g_pkApp->GetAccumTime();
						pkModel->SetNIFEnableUpdate(MB(SZ_CRAFT_OPT_EFF1), true);
						pkModel->ResetNIFAnimation(MB(SZ_CRAFT_OPT_EFF1));
						pkModel->SetEnableUpdate(true);
						g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, MB(SZ_CRAFT_OPT_SUCCESS_SND), 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
					}
					else
					{
						float fPassTime = g_pkApp->GetAccumTime() - fStartTime;
						if( fPassTime < RARITY_PROGRESS_TIME )
						{
							return;
						}

						pkModel->SetNIFEnableUpdate(MB(SZ_CRAFT_OPT_EFF1), false);
						pkModel->SetEnableUpdate(false);
						pkMain->OwnerState(ECOS_NOTHING);
						fStartTime = 0.0f;

						XUI::CXUI_Wnd* pkSoulText = pkMain->GetControl(SZ_SOUL_CHANGER_VALUE);
						if( pkSoulText )
						{
							SItemPos kItemPos;
							SItemPos kRediceItemPos;
							pkMain->GetCustomData(&kItemPos, sizeof(kItemPos));
							pkSoulText->GetCustomData(&kRediceItemPos, sizeof(kRediceItemPos));

							HRESULT hResult = CheckSendOK(kItemPos, kRediceItemPos);
							switch( hResult )
							{
							case ECOE_OK:
								{
									BM::Stream kPacket(PT_C_M_REQ_USE_REDICE_OPTION_ITEM);
									kPacket.Push(kRediceItemPos);
									kPacket.Push(kItemPos);
									NETWORK_SEND(kPacket);
								}break;
							default:
								{
									XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkMain->GetControl(SZ_SOUL_ACTION));
									if( pkBtn )
									{
										pkBtn->Disable(false);
									}

									lwAddWarnDataTT(5813 + hResult);
								}break;
							}
						}
					}
				}
			}
		}
	}

	void lwCloseCraftOptChanger(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( pSelf )
		{
			if( pSelf->OwnerState() == ECOS_NOTHING )
			{
				XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pSelf->GetControl(SZ_SOUL_ACTION));
				pkBtn ? pkBtn->Disable(false) : 0;

				pSelf->Close();
			}
			else
			{
				lwAddWarnDataTT(5809);
			}
		}
	}

	void lwUnRegCraftOptItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Icon* pkIcon = dynamic_cast<XUI::CXUI_Icon*>(kSelf.GetSelf());
		if( pkIcon )
		{
			CallCraftOptChanger(SItemPos());
		}
	}

	void CallCraftOptChanger(SEnchantInfo const& kEnchantInfo)
	{
		XUI::CXUI_Wnd* pkMain = XUIMgr.Get(SZ_MAIN_NAME);
		if( !pkMain || pkMain->IsClosed() )
		{
			pkMain = XUIMgr.Call(SZ_MAIN_NAME);
			if( !pkMain )
			{
				return;
			}
		}

		if( ECOS_NOTHING != pkMain->OwnerState() )
		{
			return;
		}

		PgUIModel* pkModel = g_kUIScene.FindUIModel(MB(SZ_CRAFT_OPT_MODEL));
		if( pkModel )
		{
			pkMain->OwnerState(ECOS_NEXT_EFFECT_STARTED);
		}

		SItemPos kItemPos;
		pkMain->GetCustomData(&kItemPos, sizeof(kItemPos));

		XUI::CXUI_Wnd* pkTemp = pkMain->GetControl(SZ_SOUL_ACTION);
		if( pkTemp )
		{
			bool bResult = true;
			pkTemp->Text(TTW(50601) + TTW(2200));
			pkTemp->SetCustomData(&bResult, sizeof(bResult));
		}

		if( kItemPos != SItemPos::NullData() )
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer ){ return; }

			PgInventory* pkInv = pkPlayer->GetInven();
			if( !pkInv ){ return; }

			PgBase_Item kItem;
			if( S_OK == pkInv->GetItem(kItemPos, kItem) )
			{
				kItem.EnchantInfo(kEnchantInfo);
				SetCraftOptTargetItem(pkMain, kItemPos, kItem, true);	
			}
		}
	}

	void CallCraftOptChanger(SItemPos const kItemPos)
	{
		XUI::CXUI_Wnd* pkMain = XUIMgr.Get(SZ_MAIN_NAME);
		if( !pkMain || pkMain->IsClosed() )
		{
			pkMain = XUIMgr.Call(SZ_MAIN_NAME);
			if( !pkMain )
			{
				return;
			}
		}
		
		if( ECOS_NOTHING != pkMain->OwnerState() )
		{
			return;
		}

		XUI::CXUI_Wnd* pkTemp = NULL;
		XUI::CXUI_Builder* pkBuilder = dynamic_cast<XUI::CXUI_Builder*>(pkMain->GetControl(SZ_OPT_SLOT_BLD1));
		XUI::CXUI_Builder* pkBuilder2 = dynamic_cast<XUI::CXUI_Builder*>(pkMain->GetControl(SZ_OPT_SLOT_BLD2));
		if( pkBuilder && pkBuilder2 )
		{
			int const MAX_SLOT = (pkBuilder->CountX() * pkBuilder->CountY()) + (pkBuilder2->CountX() * pkBuilder2->CountY());

			for( int i = 0; i < MAX_SLOT; ++i )
			{
				BM::vstring vStr(SZ_OPT_SLOT);
				vStr += i;

				pkTemp = pkMain->GetControl(vStr);
				if( pkTemp )
				{
					pkTemp->Text(L"");
					pkTemp = pkTemp->GetControl(SZ_OPT_SLOT_VALUE);
					if( pkTemp )
					{
						pkTemp->Text(L"");
					}
				}
			}
		}

		pkTemp = pkMain->GetControl(SZ_SOUL_CHANGER_ICON);
		if( pkTemp )
		{
			pkTemp->ClearCustomData();
		}

		pkTemp = pkMain->GetControl(SZ_SOUL_CHANGER_VALUE);
		if( pkTemp )
		{
			BM::vstring vStr(TTW(5805));
			vStr.Replace(L"#HAVE#", 0);
			vStr.Replace(L"#NEED#", 0);
			pkTemp->Text(vStr);
			pkTemp->ClearCustomData();
		}

		pkTemp = pkMain->GetControl(SZ_SOUL_NOTICE);
		if( pkTemp )
		{
			CONT_DEFREDICEOPTIONCOST const * pContOptCost = NULL;
			g_kTblDataMgr.GetContDef(pContOptCost);	//맵 데프

			BM::vstring vStr(TTW(5806));
			if( pContOptCost )
			{
				for( int i = IG_RARE; i < IG_MAX; ++ i)
				{
					std::wstring kChangeStr;
					int iValue = 0;
					switch( i )
					{
					case IG_RARE:		{ kChangeStr = L"#R_VALUE#"; } break;
					case IG_UNIQUE:		{ kChangeStr = L"#S_VALUE#"; } break;
					case IG_ARTIFACT:	{ kChangeStr = L"#H_VALUE#"; } break;
					case IG_LEGEND:		{ kChangeStr = L"#L_VALUE#"; } break;
					}

					CONT_DEFREDICEOPTIONCOST::const_iterator cost_itor = pContOptCost->find(i);	//맵번호로 맵 데프에서 찾아서
					if (pContOptCost->end() != cost_itor)
					{
						iValue = cost_itor->second;
					}
					vStr.Replace(kChangeStr, iValue);
				}
			}
			pkTemp->Text(vStr);
		}

		pkTemp = pkMain->GetControl(SZ_SOUL_ACTION);
		if( pkTemp )
		{
			bool bResult = false; 
			pkTemp->Text(TTW(5807) + TTW(2200));
			pkTemp->SetCustomData(&bResult, sizeof(bResult));
		}

		pkTemp = pkMain->GetControl(SZ_TARGET_ICON);
		if( pkTemp )
		{
			if( kItemPos != SItemPos::NullData() )
			{
				pkMain->ClearCustomData();

				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pkPlayer ){ return; }

				PgInventory* pkInv = pkPlayer->GetInven();
				if( !pkInv ){ return; }

				PgBase_Item kItem;
				if( S_OK == pkInv->GetItem(kItemPos, kItem) )
				{
					HRESULT hResult = SetCraftOptTargetItem(pkMain, kItemPos, kItem, false);
					switch( hResult )
					{
					case ECOE_OK:
						{
							pkMain->SetCustomData(&kItemPos, sizeof(kItemPos));
							pkTemp->SetCustomData(&kItemPos, sizeof(kItemPos));
							return;
						}break;
					default:
						{
							pkMain->ClearCustomData();
							pkTemp->ClearCustomData();
							lwAddWarnDataTT(5813 + hResult);
						}break;
					}
				}
				else
				{
					pkMain->ClearCustomData();
					pkTemp->ClearCustomData();
				}
			}
			else
			{
				pkMain->ClearCustomData();
				pkTemp->ClearCustomData();
			}
		}
	}

	HRESULT SetCraftOptTargetItem(XUI::CXUI_Wnd* pkMain, SItemPos const& kItemPos, PgBase_Item const& kItem, bool bIsResult)
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( !pDef )
		{
			return ECOE_WRONG_ITEM;
		}

		if( !pDef->CanEquip() || PgItemRarityUpgradeMgr::RCE_OK != PgItemRarityUpgradeMgr::CheckRarityBundle(kItem) )
		{
			return ECOE_NO_CRAFT_ITEM;
		}

		if( LOCAL_MGR::NC_JAPAN == g_kLocal.ServiceRegion() 
			&& CheckIsCashItem(kItem))
		{//일본의 경우 캐시 아이템은 인챈트 관련 작업 불가
			return ECOE_NO_CRAFT_ITEM;
		}

		SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();
		int const iEquipPos = pDef->EquipPos();
		int const iLevelLimit = pDef->GetAbil(AT_LEVELLIMIT);
		SBasicOptionAmpKey const kAmpKey(GetEquipType(pDef), iLevelLimit, kEnchantInfo.BasicAmpLv());

		CONT_ENCHANT_ABIL kEnchantAbil;
		GetAbilObject(kEnchantInfo.BasicType1(), iEquipPos, kEnchantInfo.BasicLv1(), 0, 0, kEnchantAbil, kAmpKey);
		GetAbilObject(kEnchantInfo.BasicType2(), iEquipPos, kEnchantInfo.BasicLv2(), 0, 1, kEnchantAbil, kAmpKey);
		GetAbilObject(kEnchantInfo.BasicType3(), iEquipPos, kEnchantInfo.BasicLv3(), 0, 2, kEnchantAbil, kAmpKey);
		GetAbilObject(kEnchantInfo.BasicType4(), iEquipPos, kEnchantInfo.BasicLv4(), 0, 3, kEnchantAbil, kAmpKey);
		if( kEnchantAbil.empty() )
		{//소울 옵션이 없음
			return ECOE_NOT_EXIST_CRAFT_OPT;
		}
		else
		{//소울 옵션이 있음
			if( !bIsResult )
			{
				XUI::CXUI_Wnd* pkSoulText = pkMain->GetControl(SZ_SOUL_CHANGER_VALUE);
				if( pkSoulText )
				{
					CONT_DEFREDICEOPTIONCOST const * pContOptCost = NULL;
					g_kTblDataMgr.GetContDef(pContOptCost);	//맵 데프
					CONT_DEFREDICEOPTIONCOST::const_iterator cost_itor = pContOptCost->find(GetItemGrade(kItem));	//맵번호로 맵 데프에서 찾아서
					if (pContOptCost->end() != cost_itor)
					{
						PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
						if( !pkPlayer ){ return ECOE_CLIENT_ERROR; }

						PgInventory* pkInv = pkPlayer->GetInven();
						if( !pkInv ){ return ECOE_CLIENT_ERROR; }

						BM::vstring vStr(TTW(5805));

						ContHaveItemNoCount kItems;
						if( S_OK != pkInv->GetItems(UICT_REDICE_ITEM_OPTION, kItems) )
						{
							vStr.Replace(L"#HAVE#", 0);
						}
						else
						{
							int iHaveCount = 0;
							int iRediceItemNo = 0;

							ContHaveItemNoCount::const_iterator have_itor = kItems.begin();
							while( kItems.end() != have_itor )
							{
								if( cost_itor->second < have_itor->second )
								{
									iHaveCount = have_itor->second;
									iRediceItemNo = have_itor->first;
									break;
								}
								else
								{
									if( iHaveCount < have_itor->second )
									{
										iHaveCount = have_itor->second;
										iRediceItemNo = have_itor->first;
									}
								}
								++have_itor;
							}

							SItemPos kRediceItemPos;
							if( S_OK == pkInv->GetFirstItem(iRediceItemNo, kRediceItemPos) )
							{
								XUI::CXUI_Icon* pkRediceItem = dynamic_cast<XUI::CXUI_Icon*>(pkMain->GetControl(SZ_SOUL_CHANGER_ICON));
								if( pkRediceItem )
								{
									pkRediceItem->SetCustomData(&iRediceItemNo, sizeof(iRediceItemNo));
									pkRediceItem->GrayScale(iHaveCount < cost_itor->second);
								}
								pkSoulText->SetCustomData(&kRediceItemPos, sizeof(kRediceItemPos));
							}
							vStr.Replace(L"#HAVE#", std::min(iHaveCount, static_cast<int>(cost_itor->second)));
						}

						vStr.Replace(L"#NEED#", cost_itor->second);
						pkSoulText->Text(vStr);
					}
				}
			}
			SetCraftOptSlot(pkMain, kItem, pDef, kEnchantAbil, bIsResult);
		}
		return ECOE_OK;
	}

	void SetCraftOptSlot(XUI::CXUI_Wnd* pkMain, PgBase_Item const& kItem, CItemDef const* pkDef, CONT_ENCHANT_ABIL const& kAbilCont, bool bIsResult)
	{
		if( !pkMain ){ return; }

		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkMain->GetControl(SZ_OPT_SLOT_BLD1));
		if( !pkBuild ){ return; }

		int const Y_SLOT_CNT = pkBuild->CountY();
		int const START_SLOT = (bIsResult)?(Y_SLOT_CNT):(0);
		int const MAX_SLOT = pkBuild->CountX() * pkBuild->CountY() + START_SLOT;

		CONT_ENCHANT_ABIL::const_iterator abil_itor = kAbilCont.begin();
		for( int i = START_SLOT; i < MAX_SLOT; ++i )
		{
			BM::vstring vStr(SZ_OPT_SLOT);
			vStr += i;

			XUI::CXUI_Wnd* pkSlot = pkMain->GetControl(vStr);
			if( pkSlot )
			{
				XUI::CXUI_Wnd* pkValue = pkSlot->GetControl(SZ_OPT_SLOT_VALUE);
				if( pkValue )
				{
					if( abil_itor != kAbilCont.end() )
					{
						std::wstring kStr;
						if( MakeAbilNameString(abil_itor->wType, kStr) )
						{
							pkSlot->Text(kStr);
						}
						kStr.clear();
						if( S_FALSE == MakeAbilValueString(kItem, pkDef, abil_itor->wType, abil_itor->iValue, kStr, false) )
						{
							pkValue->Text(kStr);
						}
						++abil_itor;
					}
					else
					{
						pkSlot->Text(L"");
						pkSlot->Text(L"");
					}
				}
			}
		}
	}

	void MoveCraftOptSlotValue(XUI::CXUI_Wnd* pkMain)
	{
		if( !pkMain ){ return; }

		XUI::CXUI_Builder* pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkMain->GetControl(SZ_OPT_SLOT_BLD1));
		if( !pkBuild ){ return; }

		int const Y_SLOT_CNT = pkBuild->CountY();
		int const MAX_SLOT = pkBuild->CountX() * pkBuild->CountY();

		for( int i = 0; i < MAX_SLOT; ++i )
		{
			BM::vstring vStr(SZ_OPT_SLOT);
			vStr += i;

			XUI::CXUI_Wnd* pkDSlot = pkMain->GetControl(vStr);

			vStr = SZ_OPT_SLOT;
			vStr += (i + Y_SLOT_CNT);

			XUI::CXUI_Wnd* pkSSlot = pkMain->GetControl(vStr);

			if( pkDSlot && pkSSlot )
			{
				pkDSlot->Text( pkSSlot->Text() );
				XUI::CXUI_Wnd* pkDValue = pkDSlot->GetControl(SZ_OPT_SLOT_VALUE);
				XUI::CXUI_Wnd* pkSValue = pkSSlot->GetControl(SZ_OPT_SLOT_VALUE);
				if( pkDValue && pkSValue )
				{
					pkDValue->Text( pkSValue->Text() );
				}
			}
		}
	}

	HRESULT CheckSendOK(SItemPos const& kTargetItemPos, SItemPos const& kRediceItemPos)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer)
		{
			return ECOE_CLIENT_ERROR;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return ECOE_CLIENT_ERROR;
		}

		PgBase_Item kItem;
		if( S_OK != pkInv->GetItem( kTargetItemPos, kItem ) )
		{
			return ECOE_NO_ITEM_REGIST;
		}

		GET_DEF(CItemDefMgr, kItemDef);
		CItemDef const* pkDef = kItemDef.GetDef(kItem.ItemNo());
		if( !pkDef )
		{
			return ECOE_NOT_EXIST_DB;
		}

		SEnchantInfo const& kEnchantInfo = kItem.EnchantInfo();
		int const iEquipPos = pkDef->EquipPos();
		int const iLevelLimit = pkDef->GetAbil(AT_LEVELLIMIT);
		SBasicOptionAmpKey const kAmpKey(GetEquipType(pkDef), iLevelLimit, kEnchantInfo.BasicAmpLv());

		CONT_ENCHANT_ABIL kEnchantAbil;
		GetAbilObject(kEnchantInfo.BasicType1(), iEquipPos, kEnchantInfo.BasicLv1(), 0, 0, kEnchantAbil, kAmpKey);
		GetAbilObject(kEnchantInfo.BasicType2(), iEquipPos, kEnchantInfo.BasicLv2(), 0, 1, kEnchantAbil, kAmpKey);
		GetAbilObject(kEnchantInfo.BasicType3(), iEquipPos, kEnchantInfo.BasicLv3(), 0, 2, kEnchantAbil, kAmpKey);
		GetAbilObject(kEnchantInfo.BasicType4(), iEquipPos, kEnchantInfo.BasicLv4(), 0, 3, kEnchantAbil, kAmpKey);
		if( kEnchantAbil.empty() )
		{//소울 옵션이 없음
			return ECOE_NOT_EXIST_CRAFT_OPT;
		}

		PgBase_Item kRediceItem;
		if( S_OK != pkInv->GetItem( kRediceItemPos, kRediceItem ) )
		{
			return ECOE_NO_EXIST_OPT_CHANGE_ITEM;
		}

		CItemDef const* pkRediceItemDef = kItemDef.GetDef( kRediceItem.ItemNo() );
		if( !pkRediceItemDef )
		{
			return ECOE_NOT_EXIST_DB;
		}

		ContHaveItemNoCount kItems;
		if( S_OK != pkInv->GetItems( static_cast<EUseItemCustomType const>(pkRediceItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE)), kItems) )
		{
			return ECOE_NO_EXIST_OPT_CHANGE_ITEM;
		}

		ContHaveItemNoCount::const_iterator item_itor = kItems.find( kRediceItem.ItemNo() );
		if( item_itor != kItems.end() )
		{
			E_ITEM_GRADE kGrade = GetItemGrade(kItem);

			CONT_DEFREDICEOPTIONCOST const * pContOptCost = NULL;
			g_kTblDataMgr.GetContDef(pContOptCost);

			if( pContOptCost )
			{
				CONT_DEFREDICEOPTIONCOST::const_iterator cost_itor = pContOptCost->find(static_cast<BYTE>(kGrade));
				if( cost_itor != pContOptCost->end() )
				{
					if(item_itor->second >= cost_itor->second)
					{
						return ECOE_OK;
					}
					else
					{
						return ECOE_LACK_OPT_CHANGE_ITEM;
					}
				}
			}
		}
		return ECOE_NO_DEFINED_ERROR;
	}
}

namespace lwPetOptChanger
{
	//상수
	wchar_t const* const SZ_OPT_MODEL		 = L"REDICE_EFFECT_VIEW";
	wchar_t const* const SZ_OPT_EFF_PROGRESS = L"Progress";
	wchar_t const* const SZ_OPT_EFF_SUCCESS  = L"Success";
	wchar_t const* const SZ_OPT_SND_PROGRESS = L"Item-try";
	wchar_t const* const SZ_OPT_SND_SUCCESS  = L"Item-Success";
	int const REDICE_PET_ITEMNO = 98006720;

	typedef enum eRediceOpt_State
	{
		EROS_NOTHING			= 0,
		EROS_PROGRESS_EFFECT	= 1,
		EROS_SUCCESS_EFFECT		= 2,
		EROS_CONFIRM			= 3,
	}E_Redice_OPT_STATE;

	//변수
	static SItemPos g_kRediceItemPos;
	static SItemPos g_kItemPos;
	static E_Redice_OPT_STATE g_eState = EROS_NOTHING;
	static float g_fStartTime = 0.0f;

	//함수 선언
	bool lwDoActionRediceOption();
	PgBase_Item const GetSrcItem();
	void lwOnEffectViewAndSending_Redice(lwUIWnd kSelf);
	void SetEnchantText(CXUI_Wnd* pkWnd, PgBase_Item const& kItem, CItemDef const* pkDef, CONT_ENCHANT_ABIL const & kEnchantAbil, bool bRight);
	void lwOnCloseRediceOption();
	void lwInitDyeingEffect(void);
	void lwUpdateDyeingEffect(void);

	//루아 래핑
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "OnEffectViewAndSending_Redice", lwOnEffectViewAndSending_Redice);
		def(pkState, "DoActionRediceOption", lwDoActionRediceOption);
		def(pkState, "RediceOptionInit", lwRediceOptionInit);
		def(pkState, "OnCloseRediceOption", lwOnCloseRediceOption);

		def(pkState, "InitDyeingEffect", lwInitDyeingEffect);
		def(pkState, "UpdateDyeingEffect", lwUpdateDyeingEffect);
	}	

	//함수 정의
	bool lwRediceOptionInit(lwUIWnd kWnd)
	{
		XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
		if( !pkWnd ){ return false; }

		g_eState = EROS_NOTHING;
		g_fStartTime = 0.0f;
		g_kRediceItemPos.Clear();
		g_kItemPos.Clear();

		XUI::CXUI_Wnd* pkTmp = NULL;
		pkTmp = pkWnd->GetControl(L"ICN_REGICON");
		if( pkTmp ){ pkTmp->ClearCustomData(); }

		pkTmp = pkWnd->GetControl(L"ICN_USEICON");
		if( pkTmp )
		{ 
			pkTmp->SetCustomData(&REDICE_PET_ITEMNO, sizeof(REDICE_PET_ITEMNO)); 
			pkTmp->GrayScale(true);
		}

		pkTmp = pkWnd->GetControl(L"SFRM_NEED");
		if( pkTmp )
		{
			BM::vstring vStr(TTW(3402));
			vStr.Replace(L"#HAVE#",0);
			pkTmp->Text(vStr);
		}

		PgBase_Item kItem;
		CONT_ENCHANT_ABIL kEnchantAbil;
		SetEnchantText(pkWnd, kItem, NULL, kEnchantAbil, false);
		SetEnchantText(pkWnd, kItem, NULL, kEnchantAbil, true);
		
		XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
		if( pkBtn )
		{
			pkBtn->Text(TTW(256)+TTW(2200));
			pkBtn->Disable(false);
		}
		return true;
	}

	void lwInitDyeingEffect(void)
	{
		g_eState = EROS_PROGRESS_EFFECT;
		g_fStartTime = 0.0f;
	}

	void Call(SItemPos const & rkItemInvPos)
	{
		CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_REDICE_ITEM" );
		if( pkWnd )
		{
			pkWnd->Close();
		}
		else
		{
			XUIMgr.Call( L"SFRM_REDICE_ITEM" );
		}
	}

	void lwOnCloseRediceOption()
	{
		if( (EROS_NOTHING==g_eState) || (EROS_CONFIRM==g_eState) )
		{
			XUIMgr.Close( L"SFRM_REDICE_ITEM" );
		}
		else
		{
			lwAddWarnDataTT(5809);
		}
	}

	bool lwDoActionRediceOption()
	{
		CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_REDICE_ITEM");
		if( !pkWnd ){ return false; }

		if(g_eState==EROS_NOTHING)
		{
			PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer ){ return false; }

			PgInventory* pkInv = pkPlayer->GetInven();
			if( !pkInv ){ return false; }

			if(SItemPos::NullData()==g_kItemPos)
			{
				lwAddWarnDataTT(3406);
				return false;
			}
			ContHaveItemNoCount kItems;
			if(S_OK != pkInv->GetItems(UICT_REDICE_PET_OPTION, kItems))
			{
				lwAddWarnDataTT(3407);
				return false;
			}
			if(SItemPos::NullData()==g_kRediceItemPos)
			{
				lwAddWarnDataTT(3408);
				return false;
			}

			g_eState = EROS_PROGRESS_EFFECT;
		}
		else
		{
			if( SetSrcItem(g_kItemPos) )
			{
				g_eState = EROS_NOTHING;
			}
		}
		return true;
	}

	void SendPacket()
	{
		BM::Stream kPacket(PT_C_M_REQ_USE_REDICE_OPTION_PET);
		kPacket.Push(g_kRediceItemPos);//변경권 위치
		kPacket.Push(g_kItemPos);//대상 아이템 위치
		NETWORK_SEND(kPacket);
	}

	void lwOnEffectViewAndSending_Redice(lwUIWnd kSelf)
	{
		float const RARITY_PROGRESS_TIME = 0.667f; //진행 시간은 1초
		float const RARITY_FINISHED_TIME = 0.53f; //진행 시간은 1초

		if( EROS_NOTHING==g_eState || EROS_CONFIRM==g_eState ){ return; }
		
		PgUIModel* pkModel = g_kUIScene.FindUIModel(MB(SZ_OPT_MODEL));
		if( !pkModel ){ return; }

		if( EROS_PROGRESS_EFFECT == g_eState )
		{
			if( 0.0f == g_fStartTime )
			{
				g_fStartTime = g_pkApp->GetAccumTime();
				pkModel->SetNIFEnableUpdate(MB(SZ_OPT_EFF_PROGRESS), true);
				pkModel->ResetNIFAnimation(MB(SZ_OPT_EFF_PROGRESS));
				pkModel->SetEnableUpdate(true);
				g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, MB(SZ_OPT_SND_PROGRESS), 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());

				CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_REDICE_ITEM");
				if( pkWnd )
				{ 
					XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
					if( pkBtn )
					{
						pkBtn->Disable(true);
					}
				}
			}
			else
			{
				float fPassTime = g_pkApp->GetAccumTime() - g_fStartTime;
				if( fPassTime < RARITY_FINISHED_TIME )
				{
					return;
				}

				pkModel->SetNIFEnableUpdate(MB(SZ_OPT_EFF_PROGRESS), false);
				pkModel->SetEnableUpdate(false);
				g_eState = EROS_NOTHING;
				g_fStartTime = 0.0f;

				PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pkPlayer ){ return; }

				PgInventory* pkInv = pkPlayer->GetInven();
				if( !pkInv ){ return; }

				ContHaveItemNoCount kItems;
				if( S_OK != pkInv->GetItems(UICT_REDICE_PET_OPTION, kItems) )
				{
					lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 3407, true);
					return;
				}

				SendPacket();
			}
		}
		else if( EROS_SUCCESS_EFFECT == g_eState )
		{
			if( 0.0f == g_fStartTime )
			{
				g_fStartTime = g_pkApp->GetAccumTime();
				pkModel->SetNIFEnableUpdate(MB(SZ_OPT_EFF_SUCCESS), true);
				pkModel->ResetNIFAnimation(MB(SZ_OPT_EFF_SUCCESS));
				pkModel->SetEnableUpdate(true);
				g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, MB(SZ_OPT_SND_SUCCESS), 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());

				CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_REDICE_ITEM");
				if( pkWnd )
				{ 
					XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
					if( pkBtn )
					{
						pkBtn->Disable(true);
					}
				}
			}
			else
			{
				float fPassTime = g_pkApp->GetAccumTime() - g_fStartTime;
				if( fPassTime < RARITY_PROGRESS_TIME )
				{
					return;
				}

				pkModel->SetNIFEnableUpdate(MB(SZ_OPT_EFF_SUCCESS), false);
				pkModel->SetEnableUpdate(false);
				g_eState = EROS_CONFIRM;
				g_fStartTime = 0.0f;

				CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_REDICE_ITEM");
				if( !pkWnd ){ return; }

				XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
				if(pkBtn)
				{
					pkBtn->Disable(false);
				}
			}
		}
	}

	void lwUpdateDyeingEffect(void)
	{
		float const RARITY_PROGRESS_TIME = 0.667f; //진행 시간은 1초
		float const RARITY_FINISHED_TIME = 0.53f; //진행 시간은 1초

		if( EROS_NOTHING==g_eState || EROS_CONFIRM==g_eState ){ return; }
		
		PgUIModel* pkModel = g_kUIScene.FindUIModel(MB(SZ_OPT_MODEL));
		if( !pkModel ){ return; }

		if( EROS_PROGRESS_EFFECT == g_eState )
		{
			if( 0.0f == g_fStartTime )
			{
				g_fStartTime = g_pkApp->GetAccumTime();
				pkModel->SetNIFEnableUpdate(MB(SZ_OPT_EFF_PROGRESS), true);
				pkModel->ResetNIFAnimation(MB(SZ_OPT_EFF_PROGRESS));
				pkModel->SetEnableUpdate(true);
				g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, MB(SZ_OPT_SND_PROGRESS), 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
			}
			else
			{
				float fPassTime = g_pkApp->GetAccumTime() - g_fStartTime;
				if( fPassTime < RARITY_FINISHED_TIME )
				{
					return;
				}

				pkModel->SetNIFEnableUpdate(MB(SZ_OPT_EFF_PROGRESS), false);
				pkModel->SetEnableUpdate(false);
				g_eState = EROS_SUCCESS_EFFECT;
				g_fStartTime = 0.0f;
			}
		}
		else if( EROS_SUCCESS_EFFECT == g_eState )
		{
			if( 0.0f == g_fStartTime )
			{
				g_fStartTime = g_pkApp->GetAccumTime();
				pkModel->SetNIFEnableUpdate(MB(SZ_OPT_EFF_SUCCESS), true);
				pkModel->ResetNIFAnimation(MB(SZ_OPT_EFF_SUCCESS));
				pkModel->SetEnableUpdate(true);
				g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, MB(SZ_OPT_SND_SUCCESS), 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());
			}
			else
			{
				float fPassTime = g_pkApp->GetAccumTime() - g_fStartTime;
				if( fPassTime < RARITY_PROGRESS_TIME )
				{
					return;
				}

				pkModel->SetNIFEnableUpdate(MB(SZ_OPT_EFF_SUCCESS), false);
				pkModel->SetEnableUpdate(false);
				g_eState = EROS_NOTHING;
				g_fStartTime = 0.0f;
			}
		}
	}

	void SetEnchantText(CXUI_Wnd* pkWnd, PgBase_Item const& kItem, CItemDef const* pkDef, CONT_ENCHANT_ABIL const & kEnchantAbil, bool bRight)
	{
		if(!pkWnd){ return; }

		XUI::CXUI_Builder* pkBuild = NULL;
		if( !bRight )
		{
			pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_ATTR_LEFT"));
		}
		else
		{
			pkBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_ATTR_RIGHT"));
		}
		if( !pkBuild ){ return; }
		int const SlotMax = pkBuild->CountY();

		std::wstring kAttrText;
		CONT_ENCHANT_ABIL::const_iterator kItor = kEnchantAbil.begin();
		for(int i=0;i<SlotMax;++i)
		{
			BM::vstring vStr;
			if( !bRight )
			{
				vStr = L"SFRM_ATTR_LEFT";
			}
			else
			{
				vStr = L"SFRM_ATTR_RIGHT";
			}
			vStr += i;
			CXUI_Wnd* pkSlot = pkWnd->GetControl(vStr);
			if( !pkSlot ){ continue; }
			CXUI_Wnd* pkValue = pkSlot->GetControl(L"SFRM_VALUE");
			if( !pkValue ){ continue; }
			
			std::wstring kText;
			std::wstring kValue;
			if(kItor!=kEnchantAbil.end())
			{
				MakeAbilNameString((*kItor).wType, kText);
				if(pkDef)
				{
					MakeAbilValueString(kItem, pkDef, (*kItor).wType, (*kItor).iValue, kValue, false);
				}

				++kItor;
			}

			pkSlot->Text(kText);
			pkValue->Text(kValue);

			if(bRight && !kText.empty())
			{
				kAttrText += kText;
				kAttrText += L" +";
				kAttrText += kValue;
				kAttrText += L", ";
			}
		}

		if( !kAttrText.empty() )
		{
			PgStringUtil::RTrim<std::wstring>(kAttrText, L", ");
			BM::vstring vStrAttr(TTW(3413));
			vStrAttr.Replace(L"#ATTR#", kAttrText);

			SChatLog kChatLog(CT_EVENT);
			g_kChatMgrClient.AddLogMessage(kChatLog, vStrAttr, true, EL_Notice1);
		}
	}

	void Recv_ANS_Packet(WORD const wPacketType, BM::Stream& rkPacket)
	{
		HRESULT bRet = S_OK;
		rkPacket.Pop(bRet);
		switch(bRet)
		{
		case E_REDICE_NOT_FOUND_PET:
		case E_REDICE_NOT_PET_ITEM:
		case E_REDICE_CANT_GENOPT_PET:
			{
				g_eState = EROS_NOTHING;

				lwAddWarnDataTT(3410 + (bRet-E_REDICE_NOT_FOUND_PET));
				SetSrcItem(SItemPos());

				CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_REDICE_ITEM");
				if( !pkWnd ){ break; }
				
				XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
				if( pkBtn )
				{
					std::wstring kText = TTW(400533)+TTW(2200);
					pkBtn->Text(kText);
					pkBtn->Disable(false);
				}
			}break;
		case S_OK:
			{
				g_eState = EROS_SUCCESS_EFFECT;

				CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_REDICE_ITEM");
				if( !pkWnd ){ break; }

				GET_DEF(CItemDefMgr, kItemDefMgr);
				CItemDef const* pItemDef = kItemDefMgr.GetDef(GetSrcItem().ItemNo());
				if( !pItemDef ){ break; }

				SEnchantInfo kEnchInfo;
				rkPacket.Pop(kEnchInfo);

				CONT_ENCHANT_ABIL kEnchantAbil;
				GetDefaultOption(GetSrcItem(), kEnchantAbil, true, &kEnchInfo);
				SetEnchantText(pkWnd, GetSrcItem(), pItemDef, kEnchantAbil, true);
				if(kEnchantAbil.empty())
				{
					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddMessage(3404, kChatLog, true, EL_Warning);
				}

				XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
				if(pkBtn)
				{
					std::wstring kText = TTW(400533)+TTW(2200);
					pkBtn->Text(kText);
					pkBtn->Disable(true);
				}
			}break;
		}
	}

	bool IsRegItem(PgBase_Item const & kItem)
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(_T("SFRM_REDICE_ITEM"));
		if(!pkWnd) {return false;}

		PgItem_PetInfo *pkPetInfo = NULL;
		if(false==kItem.GetExtInfo(pkPetInfo)) {return false;}

		GET_DEF( PgClassPetDefMgr, kClassPetDefMgr);
		PgClassPetDef kDef;
		if(false==kClassPetDefMgr.GetDef(pkPetInfo->ClassKey(), &kDef))	{return false;}

		if(EPET_TYPE_1 != kDef.GetPetType() && EPET_TYPE_3 != kDef.GetPetType()) {return false;}

		if(EROS_NOTHING!=g_eState && EROS_CONFIRM!=g_eState){return false;}

		return true;
	}

	PgBase_Item const GetSrcItem()
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(pkPlayer)
		{
			PgInventory *pkInv = pkPlayer->GetInven();
			if(pkInv)
			{
				PgBase_Item kItem;
				if(S_OK==pkInv->GetItem(g_kItemPos, kItem))
				{
					return kItem;
				}
			}
		}
		return PgBase_Item::NullData();
	}

	bool SetSrcItem(const SItemPos &rkItemPos)
	{
		if(EROS_NOTHING!=g_eState && EROS_CONFIRM!=g_eState)
		{
			return false;
		}

		g_kItemPos = rkItemPos;

		if(SItemPos::NullData() == rkItemPos)
		{
			CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_REDICE_ITEM"); 
			if( !pkWnd ){ return false; }

			lwRediceOptionInit(lwUIWnd(pkWnd));
			return true;
		}

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if(!pkPlayer){return false;}

		PgInventory *pkInv = pkPlayer->GetInven();
		if(!pkInv){return false;}
		
		PgBase_Item kItem;
		if(S_OK!=pkInv->GetItem(rkItemPos, kItem))
		{
			return false;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( !pItemDef )
		{
			return false;
		}
		
		int const iType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
		if(UICT_PET!=iType)
		{
			lwAddWarnDataTT(3411);
			return false;
		}

		PgItem_PetInfo *pkPetInfo = NULL;
		if(false==kItem.GetExtInfo(pkPetInfo)) {return false;}

		GET_DEF( PgClassPetDefMgr, kClassPetDefMgr);
		PgClassPetDef kDef;
		if(false==kClassPetDefMgr.GetDef(pkPetInfo->ClassKey(), &kDef))	{return false;}

		if(EPET_TYPE_1 != kDef.GetPetType() && EPET_TYPE_3 != kDef.GetPetType() ) {lwAddWarnDataTT(3410); return false;}

		CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_REDICE_ITEM"); 
		if( !pkWnd ){ return false; }

		BM::vstring vStr(TTW(3402));
		int iHaveItem = 0;

		CXUI_Wnd* pkUseIcon = pkWnd->GetControl(L"ICN_USEICON");
		pkUseIcon ? pkUseIcon->GrayScale(true) : 0;

		ContHaveItemNoCount kItems;
		if( S_OK == pkInv->GetItems(UICT_REDICE_PET_OPTION, kItems) )
		{
			ContHaveItemNoCount::const_iterator have_itor = kItems.begin();

			if( S_OK == pkInv->GetFirstItem(have_itor->first, g_kRediceItemPos) )
			{
				iHaveItem = 1;
				pkUseIcon ? pkUseIcon->GrayScale(false) : 0;
			}
		}
		vStr.Replace(L"#HAVE#", iHaveItem);

		
		CXUI_Wnd* pkTmp = NULL;
		pkTmp = pkWnd->GetControl(L"SFRM_NEED");
		if( pkTmp )
		{
			pkTmp->Text(vStr);
		}

		pkTmp = pkWnd->GetControl(L"ICN_REGICON");
		if( pkTmp )
		{
			pkTmp->SetCustomData(&g_kItemPos,sizeof(g_kItemPos));
		}
		
		CONT_ENCHANT_ABIL kEnchantAbil;
		SetEnchantText(pkWnd, kItem, pItemDef, kEnchantAbil, true);
		GetDefaultOption(kItem, kEnchantAbil, true);
		SetEnchantText(pkWnd, kItem, pItemDef, kEnchantAbil, false);

		XUI::CXUI_Button* pkBtn = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_OK"));
		if( pkBtn )
		{
			pkBtn->Text(TTW(256)+TTW(2200));
		}
		g_eState = EROS_NOTHING;
		return true;
	}
}

void lwPetUpgrade::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "PetUpgradeInit", lwPetUpgrade::lwPetUpgradeInit);
	def(pkState, "PetUpgradeCall", lwPetUpgrade::Call);
	def(pkState, "RequestPetUpgrade", lwPetUpgrade::RequestUpgrade);

}	

//함수 정의
bool lwPetUpgrade::lwPetUpgradeInit(lwUIWnd kWnd)
{
	XUI::CXUI_Wnd* pkWnd = kWnd.GetSelf();
	if( !pkWnd ){ return false; }
	pkWnd->ClearCustomData();

	return true;
}

void lwPetUpgrade::lwInitUpgradeEffect(void)
{
//	g_eState = EROS_PROGRESS_EFFECT;
//	g_fStartTime = 0.0f;
}

void lwPetUpgrade::Call(void)
{
	CXUI_Wnd* pkWnd = XUIMgr.Get( L"SFRM_PET_UPGRADE" );
	if( NULL == pkWnd )
	{
		pkWnd = XUIMgr.Call( L"SFRM_PET_UPGRADE" );
	}
	if( NULL == pkWnd ) { return; }


	XUI::CXUI_Wnd* pInv = XUIMgr.Get(L"Inv");
	if( pInv )
	{
		pInv->Visible(true);
	}
	else
	{
		pInv = XUIMgr.Call(L"Inv");
	}
	if(pInv)
	{
		lua_tinker::call<void,int>("PreChangeInvViewGroupTop", 4);
	}

	POINT2 ptLoc(pInv->Location().x - pkWnd->Size().x, pInv->Location().y);
	if(ptLoc.x < 0) { ptLoc.x = 0; }
	pkWnd->Location(ptLoc);
}

bool lwPetUpgrade::SetSrcItem(const SItemPos &rkItemPos)
{
	CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_PET_UPGRADE"); 
	if( NULL == pkWnd ){ return false; }


	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if(NULL == pkPlayer){return false;}

	PgInventory *pkInv = pkPlayer->GetInven();
	if(NULL == pkInv){return false;}
	
	PgBase_Item kItem;
	if(S_OK!=pkInv->GetItem(rkItemPos, kItem))
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
	if( NULL == pItemDef )
	{
		return false;
	}
	
	int const iType = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
	if(UICT_PET!=iType)
	{
		lwAddWarnDataTT(3411);
		return false;
	}

	PgItem_PetInfo *pkPetInfo = NULL;
	if(false==kItem.GetExtInfo(pkPetInfo)) {return false;}

	GET_DEF( PgClassPetDefMgr, kClassPetDefMgr);
	PgClassPetDef kDef;
	if(false==kClassPetDefMgr.GetDef(pkPetInfo->ClassKey(), &kDef))	{return false;}

	if(EPET_TYPE_2 != kDef.GetPetType() && EPET_TYPE_3 != kDef.GetPetType() ) {lwAddWarnDataTT(3422); return false;}

	int iGrade = PgClassPetDef::GetClassGrade( pkPetInfo->ClassKey().iClass);
	if(iGrade >= 2)
	{ //MSG: 3단계 등급의 펫은 전직을 할 수 없다
		lwAddWarnDataTT(3417);
		return false;
	}
	if ( !kClassPetDefMgr.IsChangeClass( pkPetInfo->ClassKey() ) )
	{
		lwAddWarnDataTT(347);
		return false;
	}


	CXUI_Wnd* pkIconBg = pkWnd->GetControl(_T("FRM_ICON_BG"));
	if(NULL == pkIconBg) { return false; }

	CXUI_Wnd* pkIcon = pkIconBg->GetControl(_T("ICN_REGICON"));
	if(NULL == pkIcon) { return false; }
	pkIcon->SetCustomData(&rkItemPos,sizeof(rkItemPos));

	CXUI_Wnd* pkPrice = pkWnd->GetControl(_T("SFRM_PRICE_BG"));
	if(NULL == pkPrice) { return false; }

	__int64 i64UpgradeCost = PgClassPetDef::GetPetUpgradeCost(pkPetInfo);
	std::wstring const wstrMoney = GetMoneyString( i64UpgradeCost, false );
	pkPrice->Text(wstrMoney);

	CXUI_Button* pkBtnOK = dynamic_cast<CXUI_Button*>( pkWnd->GetControl(_T("BTN_OK")) );
	if(NULL == pkBtnOK) { return false; }
	pkBtnOK->Disable(false);

	return true;
}

bool lwPetUpgrade::RequestUpgrade(void)
{
	CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_PET_UPGRADE"); 
	if(NULL == pkWnd ){ return false; }

	CXUI_Wnd* pkIconBg = pkWnd->GetControl(_T("FRM_ICON_BG"));
	if(NULL == pkIconBg) { return false; }

	CXUI_Wnd* pkIcon = pkIconBg->GetControl(_T("ICN_REGICON"));
	if(NULL == pkIcon) { return false; }

	SItemPos kItemPos;
	pkIcon->GetCustomData(&kItemPos,sizeof(kItemPos));
	if(SItemPos::NullData() == kItemPos) { return false; }

	PgPilot* pkPilot = g_kPilotMan.GetPlayerPilot();
	if(NULL == pkPilot) { return false; }
	CUnit* pkPlayerUnit = pkPilot->GetUnit();
	if(NULL == pkPlayerUnit) { return false; }
	PgInventory* pkInv = pkPlayerUnit->GetInven();
	if(NULL == pkInv) { return false; }

	PgBase_Item kItem;
	if(S_OK!=pkInv->GetItem(kItemPos, kItem))
	{
		return false;
	}
	PgItem_PetInfo *pkPetInfo = NULL;
	if(false==kItem.GetExtInfo(pkPetInfo)) {return false;}

	__int64 i64HaveMoney = 0;
	__int64 i64UpgradeCost = PgClassPetDef::GetPetUpgradeCost(pkPetInfo);
	if(g_kPilotMan.GetPlayerPilot())
	{
		i64HaveMoney = g_kPilotMan.GetPlayerPilot()->GetAbil64( AT_MONEY );
		if(i64UpgradeCost > i64HaveMoney)
		{ //돈이 부족한가?
			lwAddWarnDataTT(700036); //소지금이 부족합니다.
			return false;
		}
	}

	BM::Stream kPacket(PT_C_M_REQ_PET_UPGRADE, kItemPos);
	kPacket.Push(lwGetServerElapsedTime32());
	NETWORK_SEND(kPacket);

	return true;
}

namespace lwSkillExtend
{
	typedef enum eSkillExtend_State 
	{
		ESES_NOTHING		= 0,
		ESES_SENDING		= 1,
		ESES_RECV			= 2,
	}E_SKILLEXTEND_STATE;

	typedef struct tagViewSkill
	{
		tagViewSkill() : bClassLimit(false),iSkillNo(0) {}
		tagViewSkill(tagViewSkill const & other)
		{
			*this = other;
		}
		tagViewSkill & operator=(tagViewSkill const & other)
		{
			if(&other != this)
			{
				bClassLimit = other.bClassLimit;
				iSkillNo = other.iSkillNo;
				kIconInfo = other.kIconInfo;
				kSkillLv = other.kSkillLv;
				kClassName = other.kClassName;
				kSkillName = other.kSkillName;
			}
			return *this;
		}

		void WriteToPacket(BM::Stream & kPacket) const
		{
			kPacket.Push(bClassLimit);
			kPacket.Push(iSkillNo);
			kPacket.Push(kIconInfo);
			kPacket.Push(kSkillLv);
			kPacket.Push(kClassName);
			kPacket.Push(kSkillName);
		}

		void ReadFromPacket(BM::Stream & kPacket)
		{
			kPacket.Pop(bClassLimit);
			kPacket.Pop(iSkillNo);
			kPacket.Pop(kIconInfo);
			kPacket.Pop(kSkillLv);
			kPacket.Pop(kClassName);
			kPacket.Pop(kSkillName);
		}

		bool bClassLimit;
		int iSkillNo;
		SIconInfo kIconInfo;
		std::wstring kSkillLv;
		std::wstring kClassName;
		std::wstring kSkillName;
	}SViewSkill;
	typedef std::deque<SViewSkill> CONT_VIEWSKILL;

	//변수
	static SItemPos g_kItemPos;
	static int g_iItemNo = 0;
	static int g_iSkillNo = 0;
	static SIconInfo g_kIconInfo;
	static E_SKILLEXTEND_STATE g_eState = ESES_NOTHING;
	static CONT_VIEWSKILL g_kContViewSkill;

	//함수선언
	bool lwInit(lwUIWnd kWnd);
	void lwDoActionSkillExtend(lwUIWnd kWnd);
	void lwSelectSkillExtend(lwUIWnd kWnd);
	bool SendSkillExtend();
	

	//함수정의
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "SkillExtendInit", lwInit);
		def(pkState, "DoActionSkillExtend", lwDoActionSkillExtend);
		def(pkState, "SelectSkillExtend", lwSelectSkillExtend);
	}

	bool lwInit(lwUIWnd kWnd)
	{
		assert(ESES_SENDING==g_eState);
		g_eState = ESES_NOTHING;

		CXUI_Wnd* pkWnd = kWnd();
		if( !pkWnd ){ return false; }
		if( SItemPos::NullData()==g_kItemPos ){ return false; }

		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer ){ return false; }

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv ){ return false; }

		PgBase_Item kItem;
		if( S_OK!=pkInv->GetItem(g_kItemPos, kItem) ){ return false; }

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pItemDef = kItemDefMgr.GetDef(kItem.ItemNo());
		if( !pItemDef ){ return false; }

		if( UICT_SKILL_EXTEND!=pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
		{ 
			lwAddWarnDataTT(790444);
			return false; 
		}
		int const iExtendIdx = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
		int const iMonGrade = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);
		if(iMonGrade<1)
		{
			lua_tinker::call<void, int, bool>("CommonMsgBoxByTextTable", 790432, true);
			return false;
		}
		
		CONT_DEFSKILLEXTENDITEM const* kContDefSkillExtend;
		g_kTblDataMgr.GetContDef(kContDefSkillExtend);
		if( !kContDefSkillExtend ){ return false; }

		CONT_DEFSKILLEXTENDITEM::const_iterator kDefSkillExtend_iter = kContDefSkillExtend->find( iExtendIdx );
		if( kDefSkillExtend_iter==kContDefSkillExtend->end() )
		{
			lwAddWarnDataTT(790440);
			return false; 
		}

		int const iClass = pkPlayer->GetAbil(AT_CLASS);
		g_iItemNo = kItem.ItemNo();

		//Top UI
		CXUI_Wnd* pkMonText = pkWnd->GetControl(L"SFRM_TOP");
		CXUI_Wnd* pkIconMon = pkWnd->GetControl(L"ICN_MONSTER");
		if(pkMonText)
		{
			wchar_t const* pName = NULL;
			GetDefString(pItemDef->NameNo(),pName);

			BM::vstring vStr(TTW(790429));
			vStr.Replace(L"#GREAD#", TTW(790434+iMonGrade));
			vStr.Replace(L"#CARDNAME#", pName);
			vStr.Replace(L"#ADD#", iMonGrade);

			pkMonText->Text(vStr);
		}

		if(pkIconMon)
		{
			pkIconMon->SetCustomData(&g_iItemNo, sizeof(g_iItemNo));
		}

		XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_USE"));
		pButton ? pButton->Disable(true) : 0;
		
		g_kContViewSkill.clear();

		CONT_DEFSKILLEXTENDITEM::mapped_type const & kSkillExtendItem = kDefSkillExtend_iter->second;
		GET_DEF(CSkillDefMgr, kSkillDefMgr);
		for(CONT_EXTEND_SKILLSET::const_iterator skillno_iter=kSkillExtendItem.kCont.begin();
			skillno_iter!=kSkillExtendItem.kCont.end();++skillno_iter)
		{
			CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(*skillno_iter);
			if( !pSkillDef ){ continue; }

			int iNowLv = 0;
			BM::vstring vStr;
			bool const bClassLimit = IS_CLASS_LIMIT(pSkillDef->GetAbil64(AT_CLASSLIMIT),iClass);
			if(bClassLimit)
			{
				PgSkillTree::stTreeNode *pTreeNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(*skillno_iter));
				if( pTreeNode && pTreeNode->GetSkillDef() )
				{
					pSkillDef = pTreeNode->GetSkillDef();
					iNowLv = pTreeNode->IsLearned() ? pSkillDef->m_byLv : 0;
				}
			}

			CONT_VIEWSKILL::value_type element;
			element.bClassLimit = bClassLimit;
			element.iSkillNo = *skillno_iter;
			element.kIconInfo.iIconKey = pSkillDef->No();
			element.kIconInfo.iIconResNumber = pSkillDef->RscNameNo();

			vStr = TTW(790464);
			vStr.Replace(L"#NOW#", iNowLv);
			vStr.Replace(L"#MAX#", MAX_SKILL_LEVEL+iMonGrade);
			element.kSkillLv = static_cast<std::wstring>(vStr);

			MakeSkillLimitClassText(pSkillDef, element.kClassName);

			wchar_t const* pName = NULL;
			GetDefString(pSkillDef->NameNo(),pName);
			element.kSkillName = pName;

			if(bClassLimit)
			{
				g_kContViewSkill.push_front(element);
			}
			else
			{
				g_kContViewSkill.push_back(element);
			}
		}


		XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkWnd->GetControl(L"LST_SKILL"));
		if( !pkList ){ return false; }

		int const iViewSize = g_kContViewSkill.size();
		for(int i=0; i<iViewSize; ++i)
		{
			XUI::SListItem* pkItem = pkList->AddItem(std::wstring());
			if( !pkItem || !pkItem->m_pWnd ){ continue; }

			XUI::CXUI_Wnd* pkItemWnd = pkItem->m_pWnd;
			CONT_VIEWSKILL::value_type const & element = g_kContViewSkill[i];

			pkItemWnd->SetCustomData(&i, sizeof(i));

			CXUI_Icon *pkIconSkill = dynamic_cast<CXUI_Icon*>(pkItemWnd->GetControl(L"ICN_SKILL"));
			CXUI_Wnd* pkSkillLv = pkItemWnd->GetControl(L"IMG_SKILL_INFO_BG");
			CXUI_Wnd* pkClass = pkItemWnd->GetControl(L"SFRM_CLASS");
			CXUI_Wnd* pkSkillName = pkItemWnd->GetControl(L"SFRM_SKILL");

			if(pkIconSkill)
			{
				pkIconSkill->GrayScale(!element.bClassLimit);
				SIconInfo kIconInfo = pkIconSkill->IconInfo();
				kIconInfo.iIconKey = element.kIconInfo.iIconKey;
				kIconInfo.iIconResNumber = element.kIconInfo.iIconResNumber;
				pkIconSkill->SetIconInfo(kIconInfo);
			}

			pkSkillLv ? pkSkillLv->Text(element.kSkillLv) : 0;
			pkClass ? pkClass->Text(element.kClassName) : 0;
			pkSkillName ? pkSkillName->Text(element.kSkillName) : 0;
		}

		return true;
	}

	void Recv_ANS_Packet(WORD const wPacketType, BM::Stream& rkPacket)
	{
		switch( wPacketType )
		{
		case PT_M_C_ANS_SKILL_EXTEND:
			{
				HRESULT kResult = S_OK;
				rkPacket.Pop(kResult);
				if( Recv_PT_X_X_ANS_ERROR_CHECK(kResult) )
				{
					PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
					if( !pkPlayer ){ break; }
					int const iClass = pkPlayer->GetAbil(AT_CLASS);

					GET_DEF(CSkillDefMgr, kSkillDefMgr);
					CSkillDef const* pSkillDef = kSkillDefMgr.GetDef(g_kIconInfo.iIconKey);
					if( !pSkillDef ){ break; }

					bool const bClassLimit = IS_CLASS_LIMIT(pSkillDef->GetAbil64(AT_CLASSLIMIT),iClass);
					int iNowLv = 0;
					if(bClassLimit)
					{
						PgSkillTree::stTreeNode *pTreeNode = g_kSkillTree.GetNode(g_kSkillTree.GetKeySkillNo(g_kIconInfo.iIconKey));
						if( pTreeNode && pTreeNode->GetSkillDef() )
						{
							pSkillDef = pTreeNode->GetSkillDef();
							iNowLv = pTreeNode->IsLearned() ? pSkillDef->m_byLv : 0;
						}
					}

					GET_DEF(CItemDefMgr, kItemDefMgr);
					CItemDef const* pItemDef = kItemDefMgr.GetDef(g_iItemNo);
					if( !pItemDef ){ break; }

					int const iExtendIdx = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
					int const iMonGrade = pItemDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);

					XUI::CXUI_Wnd* pkWnd = XUIMgr.Activate(L"SFRM_USE_MONSTERCARD_RESULT");
					if(pkWnd)
					{
						CXUI_Icon *pkIconSkill = dynamic_cast<CXUI_Icon*>(pkWnd->GetControl(L"ICN_SKILL"));
						XUI::CXUI_Wnd* pkSkillLv = pkWnd->GetControl(L"IMG_SKILL_INFO_BG");
						XUI::CXUI_Wnd* pkText = pkWnd->GetControl(L"SFRM_TEXT");

						if(pkIconSkill)
						{
							g_kIconInfo.iIconGroup = pkIconSkill->IconInfo().iIconGroup;
							pkIconSkill->SetIconInfo(g_kIconInfo);
						}

						if(pkSkillLv)
						{
							BM::vstring vStr(TTW(790464));
							vStr.Replace(L"#NOW#", iNowLv);
							vStr.Replace(L"#MAX#", MAX_SKILL_LEVEL+iMonGrade);
							pkSkillLv->Text(vStr);
						}

						if(pkText)
						{
							wchar_t const* pCardName = NULL;
							wchar_t const* pSkillName = NULL;
							GetDefString(pItemDef->NameNo(),pCardName);							
							GetDefString(pSkillDef->NameNo(),pSkillName);

							BM::vstring vStr(TTW(790430));
							vStr.Replace(L"#GREAD#", TTW(790434+iMonGrade));
							vStr.Replace(L"#CARDNAME#", pCardName);
							vStr.Replace(L"#SKILL#", pSkillName);
							vStr.Replace(L"#MAXLEVEL#", MAX_SKILL_LEVEL+iMonGrade);

							pkText->Text(vStr);

							//Log
							BM::vstring vStrLog(TTW(790431));
							vStrLog.Replace(L"#GREAD#", TTW(790434+iMonGrade));
							vStrLog.Replace(L"#CARDNAME#", pCardName);
							vStrLog.Replace(L"#SKILL#", pSkillName);
							vStrLog.Replace(L"#MAXLEVEL#", MAX_SKILL_LEVEL+iMonGrade);

							SChatLog kChatLog(CT_ITEM);
							g_kChatMgrClient.AddLogMessage(kChatLog, vStrLog);
						}
					}
				}
			}break;
		}

		g_eState = ESES_RECV;
	}

	void lwDoActionSkillExtend(lwUIWnd kWnd)
	{
		if( kWnd.IsNil() ){ return; }

		if( SendSkillExtend() )
		{
			kWnd.Close();
		}
	}

	void lwSelectSkillExtend(lwUIWnd kWnd)
	{
		CXUI_Wnd* pkWnd = kWnd();
		if( !pkWnd ){ return; }
		
		int iViewAt = 0;
		pkWnd->GetCustomData(&iViewAt, sizeof(iViewAt));

		if(iViewAt<0 || g_kContViewSkill.size()<=iViewAt){ return; }

		CONT_VIEWSKILL::value_type const & element = g_kContViewSkill[iViewAt];

		g_iSkillNo = element.iSkillNo;
		g_kIconInfo = element.kIconInfo;
		
		CXUI_Wnd* pkMainWnd = XUIMgr.Get(L"SFRM_USE_MONSTERCARD");
		if( !pkMainWnd ){ return; }

		XUI::CXUI_List* pkListWnd = dynamic_cast<XUI::CXUI_List*>(pkMainWnd->GetControl(L"LST_SKILL"));
		if( !pkListWnd ){ return; }

		XUI::SListItem* pkBegin = pkListWnd->FirstItem();
		while( pkBegin )
		{
			CXUI_Wnd* pkBtn = (pkBegin->m_pWnd ? pkBegin->m_pWnd->GetControl(L"CBTN_SKILL_SLOT") : NULL);
			XUI::CXUI_CheckButton* pSelfBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkBtn);
			if( pSelfBtn )
			{
				pSelfBtn->Check(false);
			}
			pkBegin = pkListWnd->NextItem(pkBegin);
		}

		XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkMainWnd->GetControl(L"BTN_USE"));
		pButton ? pButton->Disable(!element.bClassLimit) : 0;
	}

	bool SendSkillExtend()
	{
		if(g_eState==ESES_SENDING){ return false; }
		g_eState = ESES_SENDING;

		BM::Stream kPacket(PT_C_M_REQ_SKILL_EXTEND);
		kPacket.Push(g_kItemPos);//사용할 몬스터 카드 위치
		kPacket.Push(g_iSkillNo);//SkillNo
		NETWORK_SEND(kPacket);
		return true;
	}

	void Call(SItemPos const & rkItemInvPos)
	{
		g_kItemPos = rkItemInvPos;
		CXUI_Wnd* pkWnd = XUIMgr.Call(L"SFRM_USE_MONSTERCARD");
		if( pkWnd && !lwInit(lwUIWnd(pkWnd)) )
		{
			pkWnd->Close();
		}
	}
}

//------------------------------------------------------------------------------------------------------------------
//	귀속 해제 아이템
//------------------------------------------------------------------------------------------------------------------
namespace lwUseUnlockItem
{
	PgUIData_UseUnlockItem::ERESULT PgUIData_UseUnlockItem::SetUnLockItem( SItemPos const& kItemPos )
	{
		SItemPos kUnLockItemPos = kItemPos;
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return E_NO_DEFINE_ERR;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return E_NO_DEFINE_ERR;
		}

		PgBase_Item kItem;
		if( S_OK != pkInv->GetItem( kUnLockItemPos, kItem ) )
		{
			ContHaveItemNoCount kCont;
			if( S_OK != pkInv->GetItems( UICT_RESET_ATTACHED, kCont ) )
			{
				return E_NO_EXIST_UNLOCK_ITEM;
			}

			if( S_OK != pkInv->GetFirstItem( kCont.begin()->first, kUnLockItemPos ) )
			{
				return E_NO_EXIST_UNLOCK_ITEM;
			}

			if( S_OK != pkInv->GetItem( kUnLockItemPos, kItem ) )
			{
				return E_NO_EXIST_UNLOCK_ITEM;
			}
		}

		m_kUnLockItem.kItemPos = kUnLockItemPos;
		m_kUnLockItem.dwItemNo = kItem.ItemNo();
		return E_OK;
	}

	PgUIData_UseUnlockItem::ERESULT PgUIData_UseUnlockItem::SetTargetItem( SItemPos const& kItemPos )
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return E_NO_DEFINE_ERR;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return E_NO_DEFINE_ERR;
		}

		PgBase_Item kItem;
		if( S_OK != pkInv->GetItem( kItemPos, kItem ) )
		{
			return E_NO_EXIST_ITEM;
		}

		GET_DEF(CItemDefMgr, kItemDefMgr);
		CItemDef const* pDef = kItemDefMgr.GetDef( kItem.ItemNo() );
		if( !pDef )
		{
			return E_NO_DEFINE_ERR;
		}

		GET_DEF(CItemDefMgr, ItemDefMgr);
		CItemDef const* pUnlockDef = ItemDefMgr.GetDef( m_kUnLockItem.dwItemNo );
		if( pUnlockDef )
		{
			int iFirstGrade = pUnlockDef->GetAbil( AT_USE_ITEM_CUSTOM_VALUE_1 );
			int iSecendGrade = pUnlockDef->GetAbil( AT_USE_ITEM_CUSTOM_VALUE_2 );

			CItemDef const* pCostumeDef = ItemDefMgr.GetDef( kItem.ItemNo() );
			if( !pCostumeDef )
			{
				return E_NO_DEFINE_ERR;
			}

			if( !pCostumeDef->CanEquip() )
			{
				return E_NO_EQUIP_ITEM;
			}

			int const iCostumeGrade = pCostumeDef->GetAbil(AT_COSTUME_GRADE);
			if( iFirstGrade > iCostumeGrade || iSecendGrade < iCostumeGrade )
			{
				return E_NO_SPECIAL;
			}
		}

		if( false == kItem.EnchantInfo().IsAttached() )
		{
			return E_NO_LOCKED_ITEM;
		}

		m_kTargetItem.kItemPos = kItemPos;
		m_kTargetItem.dwItemNo = kItem.ItemNo();
		return E_OK;
	}

	void PgUIData_UseUnlockItem::SendPacket()
	{
		BM::Stream kPacket(PT_C_M_REQ_RESET_ATTATCHED);
		kPacket.Push( m_kUnLockItem.kItemPos );
		kPacket.Push( m_kTargetItem.kItemPos );
		NETWORK_SEND( kPacket );
	}

	void PgUIData_UseUnlockItem::Clear()
	{
		m_kUnLockItem = ITEMINFO();
		m_kTargetItem = ITEMINFO();
	}

	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "OnClickSpecialUnlockUIAction", lwOnClickStartAction);
		def(pkState, "OnUpdateSpecialUnlockUI", lwUpdate);
		def(pkState, "OnCloseSpecialUnlockUI", lwClose);
	}

	void lwOnClickStartAction(lwUIWnd kSelf)
	{
		PgUIData_UseUnlockItem* pkUnlockItem = NULL;
		if( !g_kUIDataMgr.Get( UIDATA_USE_UNLOCK_ITEM, pkUnlockItem ) )
		{
			return;
		}

		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( pSelf )
		{
			XUI::CXUI_Wnd* pMainUI = pSelf->Parent();
			if( pMainUI )
			{
				switch( pMainUI->OwnerState() )
				{
				case EUS_PREPARE:
					{
						if( CheckSendOK( pkUnlockItem ) )
						{
							Start(pMainUI);
							XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pMainUI->GetControl(L"BTN_TRY_REFINE"));
							if( pButton )
							{
								pButton->Disable(true);
							}
						}
					}break;
				case EUS_RESULT:
					{
						SItemPos kItemPos = pkUnlockItem->m_kUnLockItem.kItemPos;
						pkUnlockItem->Clear();
						Call( kItemPos, false );
					}break;
				}
			}
		}
	}

	void lwUpdate(lwUIWnd kSelf)
	{
		float const PROGRESS_TIME = 0.667f; //진행 시간은 1초
		float const FINISHED_TIME = 0.53f; //진행 시간은 1초

		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( pSelf )
		{
			float fStartTime = 0;
			if( !pSelf->GetCustomData(&fStartTime, sizeof( fStartTime )) || 0 == fStartTime )
			{
				return;
			}

			float const fNowTime = g_pkApp->GetAccumTime();

			PgUIModel* pkModel = g_kUIScene.FindUIModel("SPECIAL_UNLOCK");
			if( !pkModel )
			{
				return;
			}

			switch( pSelf->OwnerState() )
			{
			case EUS_START:
				{
					if( ( fNowTime - fStartTime ) < PROGRESS_TIME)
					{
						return;
					}

					pkModel->SetNIFEnableUpdate("CraftSpin", false);
					pkModel->SetEnableUpdate(false);

					PgUIData_UseUnlockItem* pkUnlockItem = NULL;
					if( g_kUIDataMgr.Get( UIDATA_USE_UNLOCK_ITEM, pkUnlockItem ) )
					{
						if( CheckSendOK( pkUnlockItem ) )
						{
							pSelf->ClearCustomData();
							pkUnlockItem->SendPacket();
							pSelf->OwnerState(EUS_WAIT);
						}
						else
						{
							pSelf->OwnerState( EUS_PREPARE );
						}
					}
				}break;
			case EUS_RESULT:
				{
					if( ( fNowTime - fStartTime ) < FINISHED_TIME)
					{
						return;
					}

					PgUIData_UseUnlockItem* pkUnlockItem = NULL;
					if( g_kUIDataMgr.Get( UIDATA_USE_UNLOCK_ITEM, pkUnlockItem ) )
					{
						GET_DEF(CItemDefMgr, kItemDefMgr);
						CItemDef const* pDef = kItemDefMgr.GetDef( pkUnlockItem->m_kTargetItem.dwItemNo );
						if( pDef )	
						{
							std::wstring const* pStr = NULL;
							if( GetDefString( pDef->NameNo(), pStr ) )
							{
								BM::vstring vStr(TTW(405315));
								vStr.Replace(L"#NAME#", pStr->c_str());

								SChatLog kLog(CT_ITEM);
								g_kChatMgrClient.AddLogMessage(kLog, vStr);
							}
						}
					}

					pkModel->SetNIFEnableUpdate("CraftFlash", false);
					pkModel->SetEnableUpdate(false);

					XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pSelf->GetControl(L"BTN_TRY_REFINE"));
					if( pButton )
					{
						pButton->Disable(false);
						pSelf->ClearCustomData();
					}
				}break;
			}
		}
	}

	void lwClose(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pSelf = kSelf.GetSelf();
		if( pSelf && !pSelf->Parent() )
		{
			switch( pSelf->OwnerState() )
			{
			case EUS_PREPARE:
			case EUS_RESULT:
				{
					g_kUIDataMgr.Remove( UIDATA_USE_UNLOCK_ITEM );
					pSelf->Close();
				}break;
			}
		}
	}

	void CreateCall(SItemPos const& rkItemInvPos)
	{
		PgUIData_UseUnlockItem* pkUnlockItem = NULL;
		if( !g_kUIDataMgr.Get( UIDATA_USE_UNLOCK_ITEM, pkUnlockItem ) )
		{
			pkUnlockItem = new PgUIData_UseUnlockItem;
			if( !g_kUIDataMgr.Add( pkUnlockItem ) )
			{
				g_kUIDataMgr.Remove( UIDATA_USE_UNLOCK_ITEM );
				return;
			}
		}

		Call( rkItemInvPos, false );
	}

	void Call(SItemPos const& rkItemInvPos, bool bResult)
	{
		PgUIData_UseUnlockItem* pkUnlockItem = NULL;
		if( g_kUIDataMgr.Get( UIDATA_USE_UNLOCK_ITEM, pkUnlockItem ) )
		{
			PgUIData_UseUnlockItem::ERESULT eResult = pkUnlockItem->SetUnLockItem( rkItemInvPos );
			switch( eResult )
			{
			case PgUIData_UseUnlockItem::E_OK:
				{
					XUI::CXUI_Wnd* pMainUI = XUIMgr.Activate( L"SFRM_SPECIAL_UNLOCK" );
					if( false == pMainUI ){ return; }

					pMainUI->OwnerState( EUS_PREPARE );

					XUI::CXUI_Wnd* pModel = pMainUI->GetControl(L"FRM_UIMODEL");
					if( pModel )
					{
						pModel->Text(TTW(405302));
					}

					XUI::CXUI_Wnd* pTargetIcon = pMainUI->GetControl(L"ICN_TARGET");
					if( pTargetIcon )
					{
						pTargetIcon->ClearCustomData();
					}

					XUI::CXUI_Wnd* pNotice = pMainUI->GetControl(L"SFRM_SHADOW");
					if( pNotice )
					{
						BM::vstring vStr(L"");
						GET_DEF(CItemDefMgr, ItemDefMgr);
						CItemDef const* pDef = ItemDefMgr.GetDef( pkUnlockItem->m_kUnLockItem.dwItemNo );
						if( pDef )
						{
							int iFirstGrade = pDef->GetAbil( AT_USE_ITEM_CUSTOM_VALUE_1 );
							int iSecendGrade = pDef->GetAbil( AT_USE_ITEM_CUSTOM_VALUE_2 );
							if( iFirstGrade == iSecendGrade )
							{
								vStr = TTW(405312);
								vStr.Replace(L"#FIRST#", TTW(790705 + iFirstGrade));
							}
							else
							{
								vStr = TTW(405313);
								vStr.Replace(L"#FIRST#", TTW(790705 + iFirstGrade));
								vStr.Replace(L"#SECOND#", TTW(790705 + iFirstGrade));
							}
						}
						pNotice->Text(vStr);
					}

					XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>( pMainUI->GetControl(L"BTN_TRY_REFINE") );
					if( pButton )
					{
						pButton->Disable( false );
						pButton->Text( TTW(405305) + TTW(2200) );
					}
				}break;
			default:
				{
					lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 405306 + (eResult - PgUIData_UseUnlockItem::E_NO_DEFINE_ERR), true);
				}break;
			}
		}
	}

	void Start(XUI::CXUI_Wnd* pMainUI)
	{
		if( !pMainUI ){ return; }

		PgUIModel* pkModel = g_kUIScene.FindUIModel("SPECIAL_UNLOCK");
		if( pkModel )
		{
			std::string PlayEftName;
			std::string PlaySndName;
			switch( pMainUI->OwnerState() )
			{
			case EUS_PREPARE:
				{
					PlayEftName = "CraftSpin";
					PlaySndName = "Item-try";
					pMainUI->OwnerState( EUS_START );
				}break;
			case EUS_WAIT:
				{
					PlayEftName = "CraftFlash";
					PlaySndName = "EnchantFail";//"Item-Success";
					pMainUI->OwnerState( EUS_RESULT );
				}break;
			default:
				{
				}return;
			}

			pkModel->SetNIFEnableUpdate(PlayEftName, true);
			pkModel->ResetNIFAnimation(PlayEftName);
			pkModel->SetEnableUpdate(true);
			g_kSoundMan.PlayAudioSourceByID(NiAudioSource::TYPE_3D, PlaySndName.c_str(), 0.0f, 80, 100, g_kPilotMan.GetPlayerActor());

			float const fStartTime = g_pkApp->GetAccumTime();
			pMainUI->SetCustomData( &fStartTime, sizeof( fStartTime ) );
		}
	}

	void Result(void)
	{
		XUI::CXUI_Wnd* pMainUI = XUIMgr.Get( L"SFRM_SPECIAL_UNLOCK" );
		if( false == pMainUI ){ return; }

		XUI::CXUI_Wnd* pModel = pMainUI->GetControl(L"FRM_UIMODEL");
		if( pModel )
		{
			pModel->Text(TTW(405303));
		}

		XUI::CXUI_Wnd* pButton = pMainUI->GetControl(L"BTN_TRY_REFINE");
		if( pButton )
		{
			pButton->Text( TTW(400002) + TTW(2200) );
		}

		Start( pMainUI );
	}

	void SetTargetItem(SItemPos const& rkItemInvPos)
	{
		PgUIData_UseUnlockItem* pkUnlockItem = NULL;
		if( g_kUIDataMgr.Get( UIDATA_USE_UNLOCK_ITEM, pkUnlockItem ) )
		{
			XUI::CXUI_Wnd* pMainUI = XUIMgr.Get( L"SFRM_SPECIAL_UNLOCK" );
			if( false == pMainUI ){ return; }

			PgUIData_UseUnlockItem::ERESULT eResult = pkUnlockItem->SetTargetItem( rkItemInvPos );
			switch( eResult )
			{
			case PgUIData_UseUnlockItem::E_OK:
				{
					XUI::CXUI_Wnd* pTargetIcon = pMainUI->GetControl(L"ICN_TARGET");
					if( pTargetIcon )
					{
						pTargetIcon->SetCustomData(&rkItemInvPos, sizeof(rkItemInvPos));
					}
				}break;
			case PgUIData_UseUnlockItem::E_NO_EQUIP_ITEM:
				{
					lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 5813, true);
				}break;
			default:
				{
					lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 405306 + (eResult - PgUIData_UseUnlockItem::E_NO_DEFINE_ERR), true);
				}break;
			}
		}
	}

	void Recv_ANS_Packet(WORD const wPacketType, BM::Stream& rkPacket)
	{
		switch( wPacketType )
		{
		case PT_M_C_ANS_RESET_ATTATCHED:
			{
				HRESULT hRst = S_OK;
				rkPacket.Pop( hRst );

				switch( hRst )
				{
				case EC_OK:
					{
						Result();
					}break;
				default:
					{
						//E_RESET_ATTACHED_NOT_FOUND_ITEM	아템없음
						//E_RESET_ATTACHED_NOT_ATTACHED		귀속아님
						//E_RESET_ATTACHED_CANT_RESET		리셋불가
						lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 405320 + (hRst - E_RESET_ATTACHED_NOT_FOUND_ITEM), true);
					}break;
				}
			}break;
		}		
	}

	bool CheckSendOK(PgUIData_UseUnlockItem* pkSystem)
	{
		PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer )
		{
			return false;
		}

		PgInventory* pkInv = pkPlayer->GetInven();
		if( !pkInv )
		{
			return false;
		}

		PgBase_Item kItem;
		if( S_OK == pkInv->GetItem( pkSystem->m_kUnLockItem.kItemPos, kItem ) )
		{
			if( S_OK == pkInv->GetItem( pkSystem->m_kTargetItem.kItemPos, kItem ) )
			{
				return true;
			}
			else
			{
				lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 403076, true);
			}
		}
		else
		{
			lua_tinker::call< void, int, bool >("CommonMsgBoxByTextTable", 405306, true);
		}
		return false;
	}
};


//------------------------------------------------------------------------------------------------------------------
//	인첸트 전이
//------------------------------------------------------------------------------------------------------------------
typedef enum eITEM_UVINDEX
{
	EIUV_NONE				= 0,
	EIUV_WEAPON_NORMAL		= 1,
	EIUV_WEAPON_FIGHTER		= 1,
	EIUV_WEAPON_MAGICIAN	= 2,
	EIUV_WEAPON_ARCHER		= 3,
	EIUV_WEAPON_THIEF		= 4,
	EIUV_DEFENCE_NORMAL		= 3,
	EIUV_DEFENCE_FACE		= 7,
	EIUV_DEFENCE_SHOULDER	= 2,
	EIUV_DEFENCE_NECKLACE	= 10,
	EIUV_DEFENCE_EARRING	= 9,
	EIUV_DEFENCE_RING		= 12,
	EIUV_DEFENCE_BELT		= 11,
	EIUV_DEFENCE_HELMET		= 1,
	EIUV_DEFENCE_SHIRTS		= 3,
	EIUV_DEFENCE_PANTS		= 5,
	EIUV_DEFENCE_BOOTS		= 6,
	EIUV_DEFENCE_GLOVE		= 4,
	EIUV_DEFENCE_SHEILD		= 8,
	EIUV_DEFENCE_ARM		= 8,
	EIUV_DEFENCE_KICKBALL	= 8,
}E_ITEM_UVINDEX;

int const WND_ENCHANTSHIFT_SPACE		= 15;
int const ENCHANT_SHIFT_ITEM_NO			= 99600180;
int const ENCHANT_OFFENCE_ITEM_NO		= 79000010;
int const ENCHANT_DEFENCE_ITEM_NO		= 79000020;

namespace lwEnchantShift
{
	typedef enum eWNDINIT_TYPE
	{
		EWIT_ALL			= 0,
		EWIT_NOT_OPTION		= 0x01,
	}E_WNDINIT_TYPE;

	void SetEnchantShiftIcon(XUI::CXUI_Wnd* pkWnd)
	{
		if( !pkWnd ){ return; }

		XUI::CXUI_Wnd* pkWndEnchantShift =pkWnd->GetControl(L"FRM_MT_SLOT1");//구슬
		if( !pkWndEnchantShift ){ return; }

		XUI::CXUI_Wnd* pkTmp = NULL;
		pkTmp = pkWndEnchantShift->GetControl(L"ICN_MT");
		pkTmp ? pkTmp->ClearCustomData() : 0;

		pkTmp = pkWndEnchantShift->GetControl(L"FRM_NAME");
		pkTmp ? pkTmp->Text(L"") : 0;
	}

	bool IsSendResult(PgEnchantShift::E_INSURANCETYPE const eType)
	{
		switch(eType)
		{
		case PgEnchantShift::EINSUR_DESTORY:	return g_kEnchantShift.GetSendResult()==ESR_DELETE;
		case PgEnchantShift::EINSUR_MINUS_2:	return g_kEnchantShift.GetSendResult()==ESR_MINUS2;
		case PgEnchantShift::EINSUR_MINUS_1:	return g_kEnchantShift.GetSendResult()==ESR_MINUS1;
		case PgEnchantShift::EINSUR_ZERO:		return g_kEnchantShift.GetSendResult()==ESR_PLUS0;
		case PgEnchantShift::EINSUR_PLUS_1:		return g_kEnchantShift.GetSendResult()==ESR_PLUS1;
		case PgEnchantShift::EINSUR_PLUS_2:		return g_kEnchantShift.GetSendResult()==ESR_PLUS2;
		}
		return false;
	}

	eEnchantShiftRate GetSlotTypeC2S(PgEnchantShift::E_INSURANCETYPE const eType)
	{
		switch(eType)
		{
		case PgEnchantShift::EINSUR_DESTORY:	return ESR_DELETE;
		case PgEnchantShift::EINSUR_MINUS_2:	return ESR_MINUS2;
		case PgEnchantShift::EINSUR_MINUS_1:	return ESR_MINUS1;
		case PgEnchantShift::EINSUR_ZERO:		return ESR_PLUS0;
		case PgEnchantShift::EINSUR_PLUS_1:		return ESR_PLUS1;
		case PgEnchantShift::EINSUR_PLUS_2:		return ESR_PLUS2;
		}
		return ESR_NUM;
	}

	PgEnchantShift::E_INSURANCETYPE GetSlotTypeS2C(eEnchantShiftRate const eType)
	{
		switch(eType)
		{
		case ESR_DELETE: return PgEnchantShift::EINSUR_DESTORY;
		case ESR_MINUS2: return PgEnchantShift::EINSUR_MINUS_2;
		case ESR_MINUS1: return PgEnchantShift::EINSUR_MINUS_1;
		case ESR_PLUS0:  return PgEnchantShift::EINSUR_ZERO;
		case ESR_PLUS1:  return PgEnchantShift::EINSUR_PLUS_1;
		case ESR_PLUS2:  return PgEnchantShift::EINSUR_PLUS_1;
		}
		return PgEnchantShift::EINSUR_DESTORY;
	}

	void lwCall()
	{
		XUI::CXUI_Wnd* pkWnd = XUIMgr.Call(L"SFRM_ENCHANT_SHIFT");
		if( !pkWnd ){ return; }

		g_kEnchantShift.SetState(PgEnchantShift::ES_OPEN);
	}

	void lwClose()
	{
		if(!g_kEnchantShift.IsBeforOpenInv())
		{
			XUIMgr.Close(L"Inv");
		}
	}

	void lwUpdate()
	{
		g_kEnchantShift.Update();
	}

	void lwSetInsurance_Display(int const iType, int iItemNo)
	{
		XUI::CXUI_Wnd* pMain = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
		if(!pMain)
		{
			return;
		}
		BM::vstring vStr(L"FRM_RESULT_SLOT");
		vStr += iType;
		XUI::CXUI_Wnd* pResult = pMain->GetControl(vStr);
		if(!pResult)
		{
			return;
		}
		XUI::CXUI_Wnd* pIcon = pResult->GetControl(L"ICN_RATEUP");
		if(!pIcon)
		{
			return;
		}
		pIcon->ClearCustomData();
		pIcon->SetCustomData(&iItemNo, sizeof(iItemNo));	
		pIcon->GrayScale( false );

		std::wstring kReg( L"BTN_REG" );
		std::wstring kDeReg( L"BTN_DEREG" );

		XUI::CXUI_Wnd* pReg = pResult->GetControl(kReg);
		XUI::CXUI_Wnd* pDeReg = pResult->GetControl(kDeReg);
		if(!pReg || !pDeReg)
		{
			return;
		}
		pReg->Visible(false);
		pDeReg->Visible(true);
	}

	void lwBtnDownInsurance(lwUIWnd kSelf)
	{
		if( g_kEnchantShift.IsRunning() )
		{
			lwAddWarnDataTT(1278);
			return;
		}
		PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
		if( !pkPlayer || !pkPlayer->GetInven() ){ return; }
		PgInventory * const pkInv = pkPlayer->GetInven();

		XUI::CXUI_Button* pSelf = dynamic_cast<XUI::CXUI_Button*>(kSelf.GetSelf());
		if( !pSelf ){ return; }

		XUI::CXUI_Wnd* pParent = pSelf->Parent();
		if( !pParent ){ return; }

		XUI::CXUI_Wnd* pIcon = pParent->GetControl(L"ICN_RATEUP");
		if( !pIcon ){ return; }
		int iItemNo = 0;
		pIcon->GetCustomData(&iItemNo, sizeof(iItemNo));

		std::wstring kOtherBtnName( L"BTN_REG" );
		bool const bIsReg = (pSelf->ID() == kOtherBtnName);
		if( bIsReg )
		{
			kOtherBtnName = L"BTN_DEREG";
		}

		XUI::CXUI_Wnd* pOther = pParent->GetControl(kOtherBtnName);
		if( !pOther ){ return; }

		PgEnchantShift::E_INSURANCETYPE const eType = static_cast<PgEnchantShift::E_INSURANCETYPE>(pParent->BuildIndex());
		if( bIsReg )
		{
			switch(g_kLocal.ServiceRegion())
			{
			case LOCAL_MGR::NC_JAPAN:
				{//일본의 경우 캐시 보험스크롤만 사용 불가함.
					if( false == g_kEnchantShift.SetInsurance(eType, iItemNo, true) )
					{
						lwAddWarnDataTT(1299);
						return;
					}
				}break;
			default:
				{
					ContHaveItemNoCount rkItemCont;
					g_kEnchantShift.GetInsuranceItem(eType, rkItemCont);
					if( false==rkItemCont.empty() )
					{
						//사용가능한 아이템 갯수를 추리자
						ContHaveItemNoCount::iterator item_itor = rkItemCont.begin();
						while( rkItemCont.end() != item_itor )
						{
							ContHaveItemNoCount::key_type const &rkItemNo = item_itor->first;
							ContHaveItemNoCount::mapped_type &rkItemCount = item_itor->second;
							rkItemCount = g_kEnchantShift.UsableInsuranceCount(rkItemNo);
							if( rkItemCount == 0 )
							{
								rkItemCont.erase(item_itor++);
								continue;
							}
							++item_itor;
						}
						//아이템 종류가 두개 이상이면 선택하도록 해 주자.
						if( rkItemCont.size() > 1 )
						{
							UIItemUtil::CONT_CUSTOM_PARAM	kParam;
							kParam.insert(std::make_pair(L"CallYesNoBox", 0));
							std::pair<UIItemUtil::CONT_CUSTOM_PARAM::iterator, bool> ParamRst = kParam.insert(std::make_pair(std::wstring(L"CALL_UI"), UIItemUtil::EICUT_ENCHANT_SHIFT));
							if( ParamRst.second )
							{
								UIItemUtil::CallCommonUseCustomTypeItems(rkItemCont, UIItemUtil::ECIUT_CUSTOM_DEFINED, kParam, UIItemUtil::CONT_CUSTOM_PARAM_STR());
								g_kEnchantShift.eReadyInsertInsurance(eType);
								return;					
							}
						}
						else if( rkItemCont.size() == 1 )
						{//아이템 목록 갯수가 하나면 바로 넣자.
							ContHaveItemNoCount::const_iterator iter_c_item = rkItemCont.begin();
							ContHaveItemNoCount::key_type const &rkKey = iter_c_item->first;
							bool const bRet = g_kEnchantShift.SetInsurance( eType, rkKey, true );
							if( bRet )
							{
								lwSetInsurance_Display(eType, rkKey );
							}
							else
							{
								lwAddWarnDataTT(1299);
								return;
							}
						}
						else
						{
							lwAddWarnDataTT(1299);
							return;
						}
					}
					else if( PgEnchantShift::E_INSURANCETYPE::EINSUR_DESTORY == eType )
					{
						bool const bRet = g_kEnchantShift.SetInsurance( eType, iItemNo, true );
						if( bRet )
						{
							lwSetInsurance_Display(eType, iItemNo );
						}
						else
						{
							lwAddWarnDataTT(1299);
							return;
						}
					}
					else
					{
						lwAddWarnDataTT(1299);
						return;
					}
				}break;
			}
		}
		else
		{
			g_kEnchantShift.ClearInsurance(eType);
		}

		//pIcon->SetCustomData(&iItemNo, sizeof(iItemNo));
		pIcon->GrayScale( !bIsReg && g_kEnchantShift.GetInsurance(eType).IsEmpty() );
		pSelf->Visible(false);
		pOther->Visible(true);
	}

	void lwStartMixing()
	{
		if(g_kEnchantShift.IsReset())
		{
			g_kEnchantShift.DoStartMixing();
		}
		else if(g_kEnchantShift.CheckDestoryInurance() && g_kEnchantShift.DestoryInuranceEmpty())
		{
			lwCallYesNoMsgBox(TTW(1307), BM::GUID::NullData(), MBT_ENCHANT_SHIFT_DESTORY_INSURANCE);
		}
		else if(g_kEnchantShift.IsUpgradeEnchantTargetItem())
		{
			lwCallYesNoMsgBox(TTW(1361), BM::GUID::NullData(), MBT_ENCHANT_SHIFT_START);
		}
		else
		{
			g_kEnchantShift.DoStartMixing();
		}
	}

	void WndInit(XUI::CXUI_Wnd* pkWnd, E_WNDINIT_TYPE const eType=EWIT_ALL)
	{
		if( !pkWnd ){ return; }

		SetEnchantShiftIcon(pkWnd);
		
		bool const bVisible = false;

		if(0 == (EWIT_NOT_OPTION&eType))
		{
			XUI::CXUI_CheckButton* pkBtn = NULL;
			pkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_SOUL_INFO"));
			pkBtn ? pkBtn->Check(bVisible) : 0;

			pkBtn = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_BASE_OPT"));
			pkBtn ? pkBtn->Check(bVisible) : 0;
		}

		XUI::CXUI_AniBar* pkAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pkWnd->GetControl(L"ANB_PERCENT"));
		if( pkAniBar )
		{
			pkAniBar->Now(0);
			pkAniBar->Max(0);
		}

		XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SLOT"));
		if( pBuild )
		{ 
			int const iMaxSlot = pBuild->CountX() * pBuild->CountY();
			for(int i=0; i<iMaxSlot; ++i)
			{
				BM::vstring vStr(L"FRM_RESULT_SLOT");
				vStr += i;

				XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
				if( pSlot )
				{
					XUI::CXUI_Wnd* pTmp = NULL;
					pTmp = pSlot->GetControl(L"IMG_SELECT");
					pTmp ? pTmp->Visible(bVisible) : 0;

					pTmp = pSlot->GetControl(L"IMG_SELECT_ANI");
					pTmp ? pTmp->Visible(bVisible) : 0;

					pTmp = pSlot->GetControl(L"ICN_RATEUP");
					if(pTmp)
					{
						pTmp->Visible(bVisible);
						pTmp->ClearCustomData();
					}

					pTmp = pSlot->GetControl(L"ICN_RST");
					if(pTmp)
					{
						pTmp->ClearCustomData();
						pTmp->GrayScale(bVisible);
						pTmp->Visible(bVisible);
					}

					pTmp = pSlot->GetControl(L"FRM_PER");
					if(pTmp)
					{
						pTmp->Text(L"");
						pTmp->ClearCustomData();
						pTmp->Visible(bVisible);
					}

					pTmp = pSlot->GetControl(L"BTN_DEREG");
					pTmp ? pTmp->Visible(bVisible) : 0;

					pTmp = pSlot->GetControl(L"BTN_REG");
					pTmp ? pTmp->Visible(bVisible) : 0;
					
					pTmp = pSlot->GetControl(L"FRM_RESULT_CV");
					pTmp ? pTmp->Visible(!bVisible) : 0;

					pTmp = pSlot->GetControl(L"IMG_CLOSE");
					pTmp ? pTmp->Visible(bVisible) : 0;
				}
			}
		}

		XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_SHIFT"));
		if(pButton)
		{
			std::wstring kText( TTW(1296) );
			kText += TTW(2200);
			pButton->Text(kText);
			pButton->Disable(!bVisible);
		}
	}

	void lwUpdateResultSlot(lwUIWnd kSelf)
	{
		XUI::CXUI_CheckButton* pSelf = dynamic_cast<XUI::CXUI_CheckButton*>(kSelf.GetSelf());
		if( !pSelf ){ return; }
		
		bool const bNowCheck = pSelf->Check();
		pSelf->Check(!bNowCheck);

		if( g_kEnchantShift.IsRunning() )
		{
			lwAddWarnDataTT(1278);
			return;
		}

		g_kEnchantShift.RefreshResultUI();
		pSelf->Check(bNowCheck);
	}
}

PgEnchantShift::PgEnchantShift()
	: m_eState(ES_NONE)
	, m_eProcessState(EPS_NONE)
	, m_bIsOpenInv(false)
	, m_eResult(ESR_NUM)
	, m_eNoticeLevel(EL_Normal)
	, m_bHaveNeedItems(false)
	, m_bHaveShiftCost(false)
	, m_kCheckDestoryInurance(false)
{
	 eReadyInsertInsurance(EINSUR_MAX);
}

PgEnchantShift::~PgEnchantShift()
{
}

void PgEnchantShift::Clear(bool const bClearState)
{
	if(bClearState)
	{
		m_eState = ES_NONE;
		m_eProcessState = EPS_NONE;
	}
	ClearSrcItem();
	ClearTargetItem();
	for(int i=0; i<EINSUR_MAX; ++i)
	{
		m_kInsurance[i].Clear();
	}
	
	m_bIsOpenInv = false;
	m_eResult = ESR_NUM;
	m_kResultText.clear();
	m_eNoticeLevel = EL_Normal;
	m_bHaveNeedItems = false;
	m_bHaveShiftCost = false;
}

void PgEnchantShift::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "CallEnchantShift", lwEnchantShift::lwCall);
	def(pkState, "CloseEnchantShift", lwEnchantShift::lwClose);
	def(pkState, "UpdateEnchantShift", lwEnchantShift::lwUpdate);
	def(pkState, "UpdateResultSlot", lwEnchantShift::lwUpdateResultSlot);
	def(pkState, "BtnDownEnchantShiftInsuranc", lwEnchantShift::lwBtnDownInsurance);
	def(pkState, "SetInsurance_Display", lwEnchantShift::lwSetInsurance_Display);
	def(pkState, "StartMixing", lwEnchantShift::lwStartMixing);
}

void PgEnchantShift::DoStartMixing()
{
	SetState(PgEnchantShift::ES_PROCESS_START);
}

bool PgEnchantShift::DestoryInuranceEmpty()const
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
	if(pkWnd)
	{
		XUI::CXUI_Wnd* pkSlot = pkWnd->GetControl(L"FRM_RESULT_SLOT0");
		pkSlot = pkSlot ? pkSlot->GetControl(L"IMG_CLOSE") : 0;
		if( pkSlot && false==pkSlot->Visible() && g_kEnchantShift.GetInsurance(EINSUR_DESTORY).IsEmpty() )
		{
			return true;
		}
	}
	
	return false;
}

bool PgEnchantShift::IsUseableItem(SItemPos const& rkItemPos, bool & rbGrayVisible)const
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
	if( !pkWnd )
	{ 
		return false;
	}

	rbGrayVisible = true;
	switch(m_eState)
	{
	case ES_STEP_1:
		{
			if(S_OK == CheckSrcItem(rkItemPos))
			{
				rbGrayVisible = false;
			}
		}break;
	case ES_STEP_2:
	case ES_STEP_3:
		{
			if(S_OK == CheckTargetItem(rkItemPos))
			{
				rbGrayVisible = false;
			}
		}break;
	default:
		{
			return false;
		}break;
	}
	return true;
}

void MakeItemName(int const iItemNo, int const iPlusLv, std::wstring & kItemName)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(iItemNo);
	if( pDef==NULL ){ return; }

	const wchar_t *pName = NULL;
	if( GetDefString(pDef->NameNo(), pName) )
	{
		kItemName.clear();

		if(iPlusLv > 0)
		{
			kItemName += _T("+");
			kItemName += (std::wstring const)BM::vstring(iPlusLv);
			kItemName += _T(" ");
		}

		kItemName += pName;
	}
}

bool MakeEquipName(int const iItemNo, std::wstring & wstrText)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkSItemDef = kItemDefMgr.GetDef(iItemNo);
	if( !pkSItemDef ){ return L""; }

	wstrText.clear();
	int const iEquipLimit = pkSItemDef->GetAbil(AT_EQUIP_LIMIT);
	if( 0 < (EQUIP_LIMIT_WEAPON & iEquipLimit) )
	{
		switch(pkSItemDef->GetAbil(AT_WEAPON_TYPE))
		{
		case 1:		{ wstrText = TTW(2001); }break;
		case 2:		{ wstrText = TTW(2002); }break;
		case 4:		{ wstrText = TTW(2003); }break;
		case 8:		{ wstrText = TTW(2004); }break;
		case 16:	{ wstrText = TTW(2005); }break;
		case 32:	{ wstrText = TTW(2006); }break;
		case 64:	{ wstrText = TTW(2007); }break;
		case 128:	{ wstrText = TTW(2008); }break;
		}
	}
	else
	{
		GetEqiupPosString(iEquipLimit, false, wstrText);
	}
	return !wstrText.empty();
}

int GetEquipUVIndex(int const iItemNo)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkItemDef = kItemDefMgr.GetDef(iItemNo);
	if( !pkItemDef ){ return EIUV_NONE; }

	int const iEquipLimit = pkItemDef->GetAbil(AT_EQUIP_LIMIT);
	if( 0 < (EQUIP_LIMIT_WEAPON & iEquipLimit) )
	{
		switch(pkItemDef->GetAbil(AT_WEAPON_TYPE))
		{
		case 1:		{ return EIUV_WEAPON_FIGHTER; }break;
		case 2:		{ return EIUV_WEAPON_FIGHTER; }break;
		case 4:		{ return EIUV_WEAPON_MAGICIAN; }break;
		case 8:		{ return EIUV_WEAPON_MAGICIAN; }break;
		case 16:	{ return EIUV_WEAPON_ARCHER; }break;
		case 32:	{ return EIUV_WEAPON_ARCHER; }break;
		case 64:	{ return EIUV_WEAPON_THIEF; }break;
		case 128:	{ return EIUV_WEAPON_THIEF; }break;
		}
		return EIUV_WEAPON_NORMAL;
	}
	else
	{
		bool const bPetItem = pkItemDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE)==UICT_PET_EQUIP;
		switch(iEquipLimit & 0xFFFFFFFF)
		{
		case EQUIP_LIMIT_FACE:		{return EIUV_DEFENCE_FACE;}break;//얼굴
		case EQUIP_LIMIT_SHOULDER:	{return EIUV_DEFENCE_SHOULDER;}break;//어깨
		case EQUIP_LIMIT_NECKLACE:	{return EIUV_DEFENCE_NECKLACE;}break;//목걸이
		case EQUIP_LIMIT_EARRING:	{return EIUV_DEFENCE_EARRING;}break;//귀걸이
		case EQUIP_LIMIT_RING:		{return EIUV_DEFENCE_RING;}break;//반지
		case EQUIP_LIMIT_BELT:		{return EIUV_DEFENCE_BELT;}break;//벨트
		case EQUIP_LIMIT_HELMET:	{return (!bPetItem?EIUV_DEFENCE_HELMET:EIUV_DEFENCE_NORMAL);}break;//투구//펫 악세사리
		case EQUIP_LIMIT_SHIRTS:	{return EIUV_DEFENCE_SHIRTS;}break;//상의 
		case EQUIP_LIMIT_PANTS:		{return EIUV_DEFENCE_PANTS;}break;//하의 
		case EQUIP_LIMIT_BOOTS:		{return (!bPetItem?EIUV_DEFENCE_BOOTS:EIUV_DEFENCE_NORMAL);}break;//부츠//펫모자
		case EQUIP_LIMIT_GLOVE:		{return (!bPetItem?EIUV_DEFENCE_GLOVE:EIUV_DEFENCE_NORMAL);}break;//장갑 //펫 상의
		case EQUIP_LIMIT_SHEILD:	{return EIUV_DEFENCE_SHEILD;}break;//방패
		case EQUIP_LIMIT_ARM:		{return EIUV_DEFENCE_ARM;}break;//암즈
		case EQUIP_LIMIT_KICKBALL:	{return EIUV_DEFENCE_KICKBALL;}break;//킥볼
		}
		return EIUV_DEFENCE_NORMAL;
	}
	return EIUV_NONE;
}

bool MakeEnchantInfo(SItemPos const & kSrcItemPos, SItemPos const & kTargetItemPos, PgBase_Item & kTargetItem, bool const bShiftSoul, bool const bShiftSpecial, bool const bShiftSocket, bool const bShiftLv=true)
{
	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return false; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	PgBase_Item kSrcItem;
	if(S_OK != pkInv->GetItem(kSrcItemPos, kSrcItem))
	{
		return false;
	}

	PgBase_Item kCopyItem;
	if(S_OK != pkInv->GetItem(kTargetItemPos, kCopyItem))
	{
		return false;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkDItemDef = kItemDefMgr.GetDef(kCopyItem.ItemNo());
	if( !pkDItemDef ){ return false; }

	SEnchantInfo const & kSEnchantInfo = kSrcItem.EnchantInfo();
	SEnchantInfo kNewEnchantInfo = kCopyItem.EnchantInfo();

	if(true == bShiftSoul)
	{
		kNewEnchantInfo.Rarity(kSEnchantInfo.Rarity());
		kNewEnchantInfo.BasicType1(kSEnchantInfo.BasicType1());
		kNewEnchantInfo.BasicType2(kSEnchantInfo.BasicType2());
		kNewEnchantInfo.BasicType3(kSEnchantInfo.BasicType3());
		kNewEnchantInfo.BasicType4(kSEnchantInfo.BasicType4());

		int const iEquipPos = pkDItemDef->EquipPos();
		int const iLevelLimit = pkDItemDef->GetAbil(AT_LEVELLIMIT);

		if(kNewEnchantInfo.BasicType1()){kNewEnchantInfo.BasicLv1(FinddMaxAbilLevel(iEquipPos, kNewEnchantInfo.BasicType1(), iLevelLimit));}
		if(kNewEnchantInfo.BasicType2()){kNewEnchantInfo.BasicLv2(FinddMaxAbilLevel(iEquipPos, kNewEnchantInfo.BasicType2(), iLevelLimit));}
		if(kNewEnchantInfo.BasicType3()){kNewEnchantInfo.BasicLv3(FinddMaxAbilLevel(iEquipPos, kNewEnchantInfo.BasicType3(), iLevelLimit));}
		if(kNewEnchantInfo.BasicType4()){kNewEnchantInfo.BasicLv4(FinddMaxAbilLevel(iEquipPos, kNewEnchantInfo.BasicType4(), iLevelLimit));}

		kNewEnchantInfo.BasicAmpLv(kSEnchantInfo.BasicAmpLv());
	}

	if(true == bShiftSpecial)
	{
		kNewEnchantInfo.SkillOpt(kSEnchantInfo.SkillOpt());
		kNewEnchantInfo.SkillOptLv(kSEnchantInfo.SkillOptLv());

		kNewEnchantInfo.RareOptType1(kSEnchantInfo.RareOptType1());
		kNewEnchantInfo.RareOptType2(kSEnchantInfo.RareOptType2());
		kNewEnchantInfo.RareOptType3(kSEnchantInfo.RareOptType3());
		kNewEnchantInfo.RareOptType4(kSEnchantInfo.RareOptType4());

		kNewEnchantInfo.RareOptLv1(kSEnchantInfo.RareOptLv1());
		kNewEnchantInfo.RareOptLv2(kSEnchantInfo.RareOptLv2());
		kNewEnchantInfo.RareOptLv3(kSEnchantInfo.RareOptLv3());
		kNewEnchantInfo.RareOptLv4(kSEnchantInfo.RareOptLv4());

		kNewEnchantInfo.MagicOpt(kSEnchantInfo.MagicOpt());
	}

	if(true == bShiftSocket)
	{
		kNewEnchantInfo.GenSocketState(kSEnchantInfo.GenSocketState());
		kNewEnchantInfo.MonsterCard(kSEnchantInfo.MonsterCard());
		kNewEnchantInfo.GenSocketState2(kSEnchantInfo.GenSocketState2());
		kNewEnchantInfo.MonsterCard2(kSEnchantInfo.MonsterCard2());
		kNewEnchantInfo.GenSocketState3(kSEnchantInfo.GenSocketState3());
		kNewEnchantInfo.MonsterCard3(kSEnchantInfo.MonsterCard3());
	}

	kCopyItem.EnchantInfo(kNewEnchantInfo);
	
	if(true == bShiftLv)
	{
		E_ITEM_GRADE const eItemGrade = ::GetItemGrade(kCopyItem);

		int iMaxLv = 0;
		if ( false == PgItemRarityUpgradeFormula::GetMaxGradeLevelRestrict( eItemGrade, false, iMaxLv, kCopyItem ) )
		{
			return false;
		}

		kNewEnchantInfo.PlusType( kSEnchantInfo.PlusType() );
		kNewEnchantInfo.PlusLv( std::min<int>(kSEnchantInfo.PlusLv(), iMaxLv) );

		kCopyItem.EnchantInfo(kNewEnchantInfo);
	}

	kTargetItem.Swap(kCopyItem);
	return true;
}

void PgEnchantShift::RecvPacket(BM::Stream& kPacket)
{	
	HRESULT kResult = S_OK;
	kPacket.Pop(kResult);
	if( ErrorCheck(kResult) )
	{
		kPacket.Pop(kResult);
		switch(kResult)
		{
		case E_ENCHANT_SHIFT_FAIL_WIDTH_INSURANCE:
			{
				kPacket.Pop(m_eResult);
				m_kResultText = TTW(1271);
				m_eNoticeLevel = EL_Notice3;
			}break;
		case E_ENCHANT_SHIFT_SUCCESS:
			{
				int iNewLevel = 0;
				kPacket.Pop(m_eResult);
				kPacket.Pop(iNewLevel);

				PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
				if( !pkPlayer || !pkPlayer->GetInven() ){ return; }
				PgInventory * const pkInv = pkPlayer->GetInven();

				PgBase_Item kItem;
				if(S_OK != pkInv->GetItem(m_kSrcItem.kItemPos, kItem))
				{
					return;
				}
				int const iCurLevel = kItem.EnchantInfo().PlusLv();

				std::wstring kSItemName;
				MakeItemName(m_kSrcItem.kItemNo, iCurLevel, kSItemName);

				std::wstring kDItemName;
				MakeItemName(m_kTargetItem.kItemNo, iNewLevel, kDItemName);
				
				BM::vstring vStr( TTW(1272) );
				vStr.Replace(L"#SRC_ITEM#", kSItemName);
				vStr.Replace(L"#TARGET_ITEM#", kDItemName);

				m_kResultText = TTW(1260);
				m_kResultText += L"\n";
				m_kResultText += static_cast<std::wstring>(vStr);
				m_eNoticeLevel = EL_Notice3;

				//결과가 닫힌 슬롯이면 열린 슬롯()으로 바꿔줌
				XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
				if( pkWnd )
				{
					E_INSURANCETYPE eLastOpen = EINSUR_PLUS_2;
					for(int i=EINSUR_PLUS_2; i>=0;--i)
					{
						BM::vstring vStr(L"FRM_RESULT_SLOT");
						vStr += i;
						XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
						if( pSlot )
						{
							XUI::CXUI_Wnd* pImg = pSlot->GetControl(L"IMG_CLOSE");
							bool const bClose = (pImg ? pImg->Visible() : true);
							if(!bClose)
							{
								eLastOpen = static_cast<E_INSURANCETYPE>(i);
								break;
							}
						}
					}
					for(int i=0; i<EINSUR_MAX;++i)
					{
						BM::vstring vStr(L"FRM_RESULT_SLOT");
						vStr += i;
						XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
						if( pSlot )
						{
							XUI::CXUI_Wnd* pImg = pSlot->GetControl(L"IMG_CLOSE");
							bool const bClose = (pImg ? pImg->Visible() : true);
							if(bClose)
							{
								E_INSURANCETYPE const eInsurType = lwEnchantShift::GetSlotTypeS2C(m_eResult);
								if(eInsurType == i)
								{
									m_eResult = lwEnchantShift::GetSlotTypeC2S(eLastOpen);
									break;
								}
							}
						}
					}
				}
			}break;
		case E_ENCHANT_SHIFT_FAIL_DESTROY:
			{
				m_eResult = ESR_DELETE;
				m_kResultText = TTW(1270);
				m_eNoticeLevel = EL_Warning;
			}break;
		}
	}

	m_eProcessState = EPS_RECV;
}

void PgEnchantShift::Update()
{
	static const int PERCENT_MAX = 100;
	static const float fConstSendTime = 0.6f;
	static const float fConstAniMaxTime = fConstSendTime+3.5f;
	static const float fConstAniSleepTime = 0.05f;
	static float fOldTime = 0.f;
	static float fOldAniTime = 0.f;
	static float fFastAniTime = 0.f;
	static BM::vstring vOldStr;
	static E_INSURANCETYPE eType = EINSUR_DESTORY;
	float fNowTime = g_pkApp->GetAccumTime();
	typedef std::vector<E_INSURANCETYPE> CONT_CLOSESLOT;
	static CONT_CLOSESLOT kContCloseSlot;

	switch(GetState())
	{
	case ES_PROCESSING:
		{
			//상태변화
			switch(GetProcessState())
			{
			case EPS_NONE:
			case EPS_INIT:
				{
					fOldTime = g_pkApp->GetAccumTime();
					fOldAniTime = g_pkApp->GetAccumTime();
					fFastAniTime = 0.f;
					eType = EINSUR_DESTORY;
					vOldStr = L"";
					SetProcessState(EPS_NORMAL);

					XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
					if( !pkWnd )
					{
						return;
					}

					XUI::CXUI_AniBar* pkAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pkWnd->GetControl(L"ANB_PERCENT"));
					if( pkAniBar )
					{
						pkAniBar->Now(0);
						pkAniBar->Max(PERCENT_MAX);
					}
					lwPlaySoundByID("inchant-change-arrowdown");	//화살표 내려올때

					kContCloseSlot.clear();
					for(int i=0; i<=EINSUR_PLUS_2;++i)
					{
						BM::vstring vStr(L"FRM_RESULT_SLOT");
						vStr += i;
						XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
						if( pSlot )
						{
							XUI::CXUI_Wnd* pImg = pSlot->GetControl(L"IMG_CLOSE");
							bool const bClose = (pImg ? pImg->Visible() : true);
							if(bClose)
							{
								kContCloseSlot.push_back(static_cast<E_INSURANCETYPE>(i));
							}
						}
					}
				}break;
			case EPS_NORMAL:
				{
					if(fNowTime-fOldTime > fConstSendTime )
					{
						SetProcessState( EPS_SENDING );
					}
				}break;
			case EPS_SENDING:
				{
					Send();
					SetProcessState( EPS_RECV_WAIT );
				}break;
			case EPS_RECV:
				{
					if(GetSendResult()==ESR_NUM)
					{
						SetState(ES_RESET);
					}

					int const iRate = ((fNowTime-fOldTime)/fConstAniMaxTime)*100;
					if(iRate >= 95)			{	fFastAniTime += 0.0001f;	}
					else if(iRate >= 90)	{	fFastAniTime  = 0.32f;		}
					else if(iRate >= 70)	{	fFastAniTime  = 0.16f;		}
					else if(iRate >= 50)	{	fFastAniTime  = 0.08f;		}
					else if(iRate >= 30)	{	fFastAniTime  = 0.02f;		}
					else					{	fFastAniTime  = 0.0f;		}
				}break;
			case EPS_FINISH:
				{
					XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
					if( pkWnd )
					{
						for(int i=0; i<EINSUR_MAX;++i)
						{
							bool const bSelect = lwEnchantShift::IsSendResult(static_cast<PgEnchantShift::E_INSURANCETYPE>(i));

							BM::vstring vStr(L"FRM_RESULT_SLOT");
							vStr += i;
							XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
							if( pSlot )
							{
								XUI::CXUI_Wnd* pTmp = NULL;
								pTmp = pSlot->GetControl(L"IMG_SELECT");
								pTmp ? pTmp->Visible(bSelect) : 0;

								pTmp = pSlot->GetControl(L"IMG_SELECT_ANI");
								pTmp ? pTmp->Visible(bSelect) : 0;

								pTmp = pSlot->GetControl(L"ICN_RST");
								if(pTmp)
								{
									lwUIWnd kWnd(pTmp);
									PgBase_Item kItem = kWnd.GetCustomDataAsSItem();
									SEnchantInfo kEnchantInfo = kItem.EnchantInfo();
									kEnchantInfo.IsTimeLimit(!bSelect);
									kItem.EnchantInfo(kEnchantInfo);
									kWnd.SetCustomDataAsSItem(kItem);
								}
							}
						}
					}

					if(ESR_DELETE == GetSendResult())
					{
						lwPlaySoundByID("inchant-change-fail");	//인챈트 실패
					}
					else if(ESR_PLUS2 == GetSendResult())
					{
						lwPlaySoundByID("inchant-change-+2");		//인챈트 +2 성공
					}
					else
					{
						lwPlaySoundByID("inchant-change-success");	//인챈트 성공
					}

					SChatLog kChatLog(CT_EVENT);
					g_kChatMgrClient.AddLogMessage(kChatLog, m_kResultText, true, m_eNoticeLevel);

					E_INSURANCETYPE const eType = lwEnchantShift::GetSlotTypeS2C(GetSendResult());
					if( m_kInsurance[eType].IsEmpty() )
					{
						ClearSrcItem();
					}

					SetState(ES_RESET);
				}break;
			}

			//애니메이션
			switch(GetProcessState())
			{
			case EPS_NORMAL:
			case EPS_SENDING:
			case EPS_RECV_WAIT:
			case EPS_RECV:
				{
					XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
					if( !pkWnd )
					{
						SetProcessState( EPS_FINISH );
						break;
					}

					int const iPercent = std::min<int>(PERCENT_MAX,((fNowTime-fOldTime)/fConstSendTime)*100);
					XUI::CXUI_AniBar* pkAniBar = dynamic_cast<XUI::CXUI_AniBar*>(pkWnd->GetControl(L"ANB_PERCENT"));
					pkAniBar ? pkAniBar->Now(iPercent) : 0;

					if( (PERCENT_MAX <= iPercent) &&
						(fNowTime-fOldAniTime > fConstAniSleepTime+fFastAniTime) )
					{
						fOldAniTime = fNowTime;
						
						XUI::CXUI_Wnd* pOldSlot = pkWnd->GetControl(vOldStr);
						if( pOldSlot )
						{
							XUI::CXUI_Wnd* pTmp = NULL;
							pTmp = pOldSlot->GetControl(L"IMG_SELECT");
							pTmp ? pTmp->Visible(false) : 0;

							pTmp = pOldSlot->GetControl(L"IMG_SELECT_ANI");
							pTmp ? pTmp->Visible(false) : 0;
						}
						vOldStr = L"";

						E_INSURANCETYPE const eOldType = eType;
						for(int i=0; i<EINSUR_MAX;++i)
						{
							int iSelect = BM::Rand_Range(EINSUR_PLUS_2,EINSUR_DESTORY);
							eType = static_cast<E_INSURANCETYPE>(iSelect);
							if(eOldType == eType)
							{
								continue;
							}

							CONT_CLOSESLOT::const_iterator find_it = std::find(kContCloseSlot.begin(), kContCloseSlot.end(), eType);
							if(find_it!=kContCloseSlot.end())
							{
								continue;
							}

							BM::vstring vStr(L"FRM_RESULT_SLOT");
							vStr += iSelect;
							XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
							if( pSlot )
							{
								lwPlaySoundByID("inchant-change-ing");	//인챈트 진행

								XUI::CXUI_Wnd* pTmp = NULL;
								pTmp = pSlot->GetControl(L"IMG_SELECT");
								pTmp ? pTmp->Visible(true) : 0;

								pTmp = pSlot->GetControl(L"IMG_SELECT_ANI");
								pTmp ? pTmp->Visible(true) : 0;

								vOldStr = vStr;
							}
							break;
						}
					}

					if(GetProcessState() == EPS_RECV)
					{
						if( fNowTime-fOldTime>fConstAniMaxTime+fConstAniSleepTime )
						{
							SetProcessState( EPS_FINISH );
						}
					}
				}break;
			}
		}break;
	}
}

void PgEnchantShift::Send()const
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
	if( !pkWnd ){ return; }

	bool bVisibleInsur[EINSUR_MAX] = {0,};
	for(int i=0; i<EINSUR_MAX; ++i)
	{
		BM::vstring vStr(L"FRM_RESULT_SLOT");
		vStr += i;

		XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
		if( pSlot )
		{
			XUI::CXUI_Wnd* pTmp = pSlot->GetControl(L"ICN_RATEUP");
			bVisibleInsur[i] = pTmp ? pTmp->Visible() : false;
		}
	}

	BM::Stream Packet(PT_C_M_REQ_ENCHANT_SHIFT);
	Packet.Push(m_kSrcItem.kItemPos);
	Packet.Push(m_kTargetItem.kItemPos);
	Packet.Push(bVisibleInsur[EINSUR_PLUS_2]  ? m_kInsurance[EINSUR_PLUS_2].kItemPos  : SItemPos::NullData());
	Packet.Push(bVisibleInsur[EINSUR_PLUS_1]  ? m_kInsurance[EINSUR_PLUS_1].kItemPos  : SItemPos::NullData());
	Packet.Push(bVisibleInsur[EINSUR_ZERO]	  ? m_kInsurance[EINSUR_ZERO].kItemPos    : SItemPos::NullData());
	Packet.Push(bVisibleInsur[EINSUR_MINUS_1] ? m_kInsurance[EINSUR_MINUS_1].kItemPos : SItemPos::NullData());
	Packet.Push(bVisibleInsur[EINSUR_MINUS_2] ? m_kInsurance[EINSUR_MINUS_2].kItemPos : SItemPos::NullData());
	Packet.Push(bVisibleInsur[EINSUR_DESTORY] ? m_kInsurance[EINSUR_DESTORY].kItemPos : SItemPos::NullData());
	Packet.Push(IsShiftSoul());
	Packet.Push(IsShiftSpecial());
	Packet.Push(IsShiftSocket());
	NETWORK_SEND(Packet);
}

PgEnchantShift::S_ITEM_INFO const & PgEnchantShift::GetInsurance(PgEnchantShift::E_INSURANCETYPE const eType)const
{
	if(EINSUR_DESTORY<=eType && eType<EINSUR_MAX)
	{
		return m_kInsurance[eType];
	}
	static S_ITEM_INFO kNullData;
	return kNullData;
}

void PgEnchantShift::ClearSrcItem()
{
	m_kSrcItem.Clear();

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
	if( !pkWnd ){ return; }
	XUI::CXUI_Wnd* pkTmp = NULL;
	pkTmp = pkWnd->GetControl(L"ICN_SOURCE");
	pkTmp ? pkTmp->ClearCustomData() : 0;
}

void PgEnchantShift::ClearTargetItem()
{
	m_kTargetItem.Clear();

	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
	if( !pkWnd ){ return; }
	XUI::CXUI_Wnd* pkTmp = NULL;
	pkTmp = pkWnd->GetControl(L"ICN_TARGET");
	pkTmp ? pkTmp->ClearCustomData() : 0;
}

void PgEnchantShift::ClearInsurance(E_INSURANCETYPE const eType)
{
	if(EINSUR_DESTORY<=eType && eType<EINSUR_MAX)
	{
		m_kInsurance[eType].Clear();
	}
	else if( EINSUR_MAX == eType )
	{
		m_kInsurance[EINSUR_DESTORY].Clear();
		m_kInsurance[EINSUR_MINUS_2].Clear();
		m_kInsurance[EINSUR_MINUS_1].Clear();
		m_kInsurance[EINSUR_ZERO].Clear();
		m_kInsurance[EINSUR_PLUS_1].Clear();
		m_kInsurance[EINSUR_PLUS_2].Clear();
	}
}

HRESULT PgEnchantShift::CheckSrcItem(SItemPos const & rkItem)const
{
	if(SItemPos::NullData() == rkItem)
	{
		return E_ENCHANT_SHIFT_NOT_FIND_SRCITEM;
	}

	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return E_FAIL; }
	PgInventory * const pkInv = pkPlayer->GetInven();
	
	PgBase_Item kItem;
	if(S_OK != pkInv->GetItem(rkItem, kItem))
	{
		return E_ENCHANT_SHIFT_NOT_FIND_SRCITEM;
	}
	return CheckSrcItem(kItem);
}

HRESULT PgEnchantShift::CheckSrcItem(PgBase_Item const& rkItem)const
{
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkSItemDef = kItemDefMgr.GetDef(rkItem.ItemNo());
	if( !pkSItemDef )
	{
		return E_ENCHANT_SHIFT_NOT_FOUND_ITEM;
	}

	/*int const iEquipLimit = pkSItemDef->GetAbil(AT_EQUIP_LIMIT);
	if( 0 < (EQUIP_LIMIT_WEAPON & iEquipLimit) )
	{
		return E_ENCHANT_SHIFT_CANT_SHIFT_WEAPON;
	}*/

	SEnchantInfo const & kSEnchantInfo = rkItem.EnchantInfo();
	if( kSEnchantInfo.IsCurse() ||
		kSEnchantInfo.IsSeal() ||
		kSEnchantInfo.IsBinding() ||
		(0 == pkSItemDef->GetAbil(AT_EQUIP_LIMIT)) ||
		(pkSItemDef->GetAbil(AT_ATTRIBUTE) & ICMET_Cant_Enchant) )
	{
		return E_ENCHANT_SHIFT_CANT_SHIFT_ITEM;
	}

	if( kSEnchantInfo.PlusLv() < ENCHANT_SHIFT_MIN_PLUS_LEVEL )
	{
		return E_ENCHANT_SHIFT_SRCITEM_MIN_PLUS_LEVEL;
	}

	return S_OK;
}

bool PgEnchantShift::SetSrcItem(SItemPos const & rkItem)
{
	if( IsRunning() )
	{
		lwAddWarnDataTT(1278);
		return false;
	}

	switch(m_eState)
	{
	case ES_STEP_1:
	case ES_STEP_2:
	case ES_STEP_3:
		{
			PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer || !pkPlayer->GetInven() ){ return false; }
			PgInventory * const pkInv = pkPlayer->GetInven();
			
			PgBase_Item kItem;
			if(S_OK != pkInv->GetItem(rkItem, kItem))
			{
				return false;
			}

			if( LOCAL_MGR::NC_JAPAN == g_kLocal.ServiceRegion() 
				&& false == CheckEnchantBundle(kItem) )
			{//일본의 경우 캐시 아이템은 인챈트 관련 작업 불가
				lwAddWarnDataTT(59002);
				return false;
			}

			HRESULT hResult = CheckSrcItem(rkItem);
			if(false==ErrorCheck(hResult, true))
			{
				lwPlaySoundByID("inchant-change-warning");	//경고음
				return false;
			}

			m_kSrcItem.kItemNo = kItem.ItemNo();
			m_kSrcItem.kItemPos = rkItem;
			lwSetReqInvWearableUpdate(true);
			g_kEnchantShift.CheckDestoryInurance(true);

			SetState(ES_STEP_2);
			return true;
		}break;
	default:
		{
			lwAddWarnDataTT(5822);
		}break;
	}

	return false;
}

HRESULT PgEnchantShift::CheckTargetItem(SItemPos const & rkItemPos)const
{
	if(m_kSrcItem.kItemPos == rkItemPos)
	{
		return E_ENCHANT_SHIFT_SAME_SRC_AND_TARGET;
	}

	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return E_FAIL; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	PgBase_Item kTargetItem;
	pkInv->GetItem(rkItemPos, kTargetItem);
	return CheckTargetItem(kTargetItem, rkItemPos);
}

HRESULT PgEnchantShift::CheckTargetItem(PgBase_Item const & kTargetItem, SItemPos const & rkItemPos)const
{
	if( m_kSrcItem.IsEmpty() )
	{
		return E_ENCHANT_SHIFT_NOT_FIND_SRCITEM;
	}

	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return E_FAIL; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	PgBase_Item kSrcItem;
	pkInv->GetItem(m_kSrcItem.kItemPos, kSrcItem);

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkSItemDef = kItemDefMgr.GetDef(kSrcItem.ItemNo());
	CItemDef const * pkDItemDef = kItemDefMgr.GetDef(kTargetItem.ItemNo());
	if( !pkSItemDef || !pkDItemDef )
	{ 
		return E_ENCHANT_SHIFT_NOT_FOUND_ITEM; 
	}

	SEnchantInfo const & kTEnchantInfo = kTargetItem.EnchantInfo();
	if( kTEnchantInfo.IsCurse() ||
		kTEnchantInfo.IsSeal() ||
		kTEnchantInfo.IsBinding() ||
		(0 == pkDItemDef->GetAbil(AT_EQUIP_LIMIT)) ||
		(pkDItemDef->GetAbil(AT_ATTRIBUTE) & ICMET_Cant_Enchant) )
	{
		return E_ENCHANT_SHIFT_CANT_SHIFT_ITEM;
	}
	
	if(0 == (pkSItemDef->GetAbil(AT_EQUIP_LIMIT) & pkDItemDef->GetAbil(AT_EQUIP_LIMIT)))
	{
		return E_ENCHANT_SHIFT_NOT_MATCH_EQUIP_LIMIT;
	}

	if( (0 < (EQUIP_LIMIT_WEAPON & pkSItemDef->GetAbil(AT_EQUIP_LIMIT))) &&
		(0 == (pkSItemDef->GetAbil(AT_WEAPON_TYPE) & pkDItemDef->GetAbil(AT_WEAPON_TYPE))))
	{
		return E_ENCHANT_SHIFT_NOT_MATCH_WEAPON_TYPE;
	}

	int const iCheckLevelLimit = pkDItemDef->GetAbil(AT_LEVELLIMIT) - pkSItemDef->GetAbil(AT_LEVELLIMIT);
	/*if(ENCHANT_SHIFT_MIN_LEVEL > iCheckLevelLimit)
	{
		return E_ENCHANT_SHIFT_UNDER_SHIFT_LEVELLIMIT;
	}*/
	if(ENCHANT_SHIFT_MAX_LEVEL < iCheckLevelLimit)
	{
		return E_ENCHANT_SHIFT_OVER_SHIFT_LEVELLIMIT;
	}

	bool bAllDisable = true;
	for(int i = 0; i < ESR_NUM; ++i)
	{
		E_INSURANCETYPE const eInsurType = lwEnchantShift::GetSlotTypeS2C(static_cast<eEnchantShiftRate>(i));
		if( EINSUR_DESTORY!=eInsurType && IsOpenResultSlot(eInsurType, rkItemPos) )
		{
			bAllDisable = false;
			break;
		}
	}
	if(bAllDisable)
	{
		return E_ENCHANT_SHIFT_NOT_ENOUGH_GRADE;
	}

	return S_OK;
}

bool PgEnchantShift::ErrorCheck(HRESULT const hResult, bool const bIsMsg)const
{
	switch(hResult)
	{
	case S_OK:
		{
			return true;
		}break;
	case E_ENCHANT_SHIFT_NOT_FIND_SRCITEM:
		{
			bIsMsg ? lwAddWarnDataTT(1277) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_SAME_SRC_AND_TARGET:
		{
			bIsMsg ? lwAddWarnDataTT(1248) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_NOT_FOUND_ITEM:
		{
			bIsMsg ? lwAddWarnDataTT(1261) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_CANT_SHIFT_ITEM:
		{
			bIsMsg ? lwAddWarnDataTT(1262) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_NOT_MATCH_EQUIP_LIMIT:
		{
			bIsMsg ? lwAddWarnDataTT(1264) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_NOT_MATCH_WEAPON_TYPE:
		{
			bIsMsg ? lwAddWarnDataTT(1265) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_UNDER_SHIFT_LEVELLIMIT:
		{
			bIsMsg ? lwAddWarnDataTT(1241) : 0;
			return false;
		}
	case E_ENCHANT_SHIFT_OVER_SHIFT_LEVELLIMIT:
		{
			if(bIsMsg)
			{
				BM::vstring vStr(TTW(1269));
				vStr.Replace(L"#LIMIT#", ENCHANT_SHIFT_MAX_LEVEL+1);

				lwAddWarnDataStr(lwWString(static_cast<std::wstring>(vStr)), EL_Warning);
			}
			return false;
		}break;
	case E_ENCHANT_SHIFT_NOT_MATCH_INSURANCE:
		{
			bIsMsg ? lwAddWarnDataTT(1298) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_NOT_MATCH_LEVELLIMIT:
		{
			bIsMsg ? lwAddWarnDataTT(1263) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_NOT_ENOUGH_SHIFT_ITEM:
		{
			bIsMsg ? lwAddWarnDataTT(1294) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_NOT_ENOUGH_SHIFT_INSURANCE:
		{
			bIsMsg ? lwAddWarnDataTT(1299) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_NOT_ENOUGH_SHIFT_GEM:
		{
			bIsMsg ? lwAddWarnDataTT(1267) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_SRCITEM_MIN_PLUS_LEVEL:
		{
			if(bIsMsg)
			{
				BM::vstring vStr(TTW(1276));
				vStr.Replace(L"#MIN#", ENCHANT_SHIFT_MIN_PLUS_LEVEL);

				lwAddWarnDataStr(lwWString(static_cast<std::wstring>(vStr)), EL_Warning);
			}
			return false;
		}break;
	case E_ENCHANT_SHIFT_CANT_SHIFT_WEAPON:
		{
			bIsMsg ? lwAddWarnDataTT(1295) : 0;
			return false;
		}break;
	case E_ENCHANT_SHIFT_NOT_ENOUGH_GRADE:
		{
			bIsMsg ? lwAddWarnDataTT(1362) : 0;
			return false;
		}break;
	}

	bIsMsg ? lwAddWarnDataTT(790413) : 0;
	return false;
}

bool PgEnchantShift::SetTargetItem(SItemPos const & rkItem, bool const bIsMsg)
{
	if( IsRunning() )
	{
		bIsMsg ? lwAddWarnDataTT(1278) : 0;
		return false;
	}

	switch(m_eState)
	{
	case ES_STEP_1:
	case ES_STEP_2:
	case ES_STEP_3:
		{
			PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
			if( !pkPlayer || !pkPlayer->GetInven() ){ return false; }
			PgInventory * const pkInv = pkPlayer->GetInven();

			PgBase_Item kTargetItem;
			if(S_OK != pkInv->GetItem(rkItem, kTargetItem))
			{
				return false;
			}

			HRESULT hResult = CheckTargetItem(rkItem);
			if(false==ErrorCheck(hResult, bIsMsg))
			{
				lwPlaySoundByID("inchant-change-warning");	//경고음
				return false;
			}

			m_kTargetItem.kItemNo = kTargetItem.ItemNo();
			m_kTargetItem.kItemPos = rkItem;
			lwSetReqInvWearableUpdate(true);

			SetState(ES_STEP_3);
			return true;
		}break;
	default:
		{
			bIsMsg ? lwAddWarnDataTT(5822) : 0;
		}break;
	}

	return false;
}

bool PgEnchantShift::IsRunning()const
{
	switch(m_eState)
	{
	case ES_PROCESS_START:
	case ES_PROCESSING:
	case ES_RESET:
		{
			return true;
		}
	}

	return false;
}

int PgEnchantShift::UsableInsuranceCount( int const iItemNo )
{
	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return false; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	int iItemCount = 0;
	for(int i=0; i<EINSUR_MAX; ++i)
	{
		E_INSURANCETYPE eType = static_cast<E_INSURANCETYPE>(i);
		if( !GetInsurance(eType).IsEmpty()
			&& GetInsurance(eType).kItemNo == iItemNo)
		{
			++iItemCount;
		}
	}

	int const iTotalCount = pkInv->GetInvTotalCount(iItemNo);

	return iTotalCount - iItemCount;
}

bool PgEnchantShift::SetInsurance(PgEnchantShift::E_INSURANCETYPE const eType, int const iItemNo, bool const bReal)
{
	if( IsRunning() )
	{
		lwAddWarnDataTT(1278);
		return false;
	}

	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return false; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	bool bSuccess = false;
	ContHaveItemNoCount rkItemCont;
	GetInsuranceItem(eType, rkItemCont);

	if(rkItemCont.find(iItemNo) == rkItemCont.end())
	{
		return false;
	}

	int iItemCount = 0;
	for(int i=0; i<EINSUR_MAX; ++i)
	{
		E_INSURANCETYPE eType = static_cast<E_INSURANCETYPE>(i);
		if( !GetInsurance(eType).IsEmpty()
			&& GetInsurance(eType).kItemNo == iItemNo)
		{
			++iItemCount;
		}
	}

	SItemPos kItemPos;
	if( S_OK == pkInv->GetFirstItem(iItemNo, kItemPos) )
	{
		PgBase_Item kItem;
		do
		{
			pkInv->GetItem(kItemPos, kItem);

			iItemCount -= kItem.Count();
			if(0 > iItemCount)
			{
				if(bReal)
				{
					m_kInsurance[eType].kItemNo = iItemNo;
					m_kInsurance[eType].kItemPos = kItemPos;
				}
				return true;
			}
		}while( E_FAIL != pkInv->GetNextItem(iItemNo, kItemPos) );
	}
	return false;
}

void PgEnchantShift::GetInsuranceItem(E_INSURANCETYPE const eType, ContHaveItemNoCount & rkItemCont)const
{
	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return; }
	PgInventory * const pkInv = pkPlayer->GetInven();
	if( !pkInv ){ return; }

	if( LOCAL_MGR::NC_JAPAN != g_kLocal.ServiceRegion() )
	{//일본을 제외하고
		pkInv->GetItems(UICT_ENCHANT_INSURANCE, rkItemCont);
		if( EINSUR_DESTORY == eType )
		{//파괴 슬롯에만 인게임 보험 스크롤 목록에 넣어주자.(있을때만)
			if(int const iItemNo = GetCurrentInsurance())
			{
				size_t const iItemCount = pkInv->GetTotalCount(iItemNo);
				if( 0 < iItemCount )
				{
					rkItemCont.insert( std::make_pair( iItemNo, iItemCount ) );
				}
			}
		}
	}
	else
	{
		if(int const iItemNo = GetCurrentInsurance())
		{
			size_t const iItemCount = pkInv->GetTotalCount(iItemNo);
			if( 0 < iItemCount )
			{
				rkItemCont.insert( std::make_pair( iItemNo, iItemCount ) );
			}
		}
	}
}

void PgEnchantShift::UpdateArrow(XUI::CXUI_Wnd * const pkWnd, PgEnchantShift::E_ARROWTYPE const eType)
{
	if( !pkWnd ){ return; }

	XUI::CXUI_Wnd* pkTmp = NULL;
	pkTmp = pkWnd->GetControl(L"SFRM_ARROW_SOURCE");
	pkTmp ? pkTmp->Visible(EAT_SOURCE==eType) : 0;

	pkTmp = pkWnd->GetControl(L"SFRM_ARROW_TARGET");
	pkTmp ? pkTmp->Visible(EAT_TARGET==eType) : 0;

	XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SLOT"));
	if( pBuild )
	{
		int const iMaxSlot = pBuild->CountX() * pBuild->CountY();
		for(int i=0; i<iMaxSlot; ++i)
		{
			BM::vstring vStr(L"FRM_RESULT_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
			if( pSlot )
			{
				XUI::CXUI_Wnd * pTmp = NULL;
				pTmp = pSlot->GetControl(L"ICN_RATEUP");
				bool const bVisible = (pTmp ? pTmp->Visible() : false);

				pTmp = pSlot->GetControl(L"SFRM_ARROW_SLOT");
				pTmp ? pTmp->Visible(bVisible && EAT_INSURANCE==eType) : 0;
			}
		}
	}
}

void PgEnchantShift::UpdateCheckOption(XUI::CXUI_Wnd * const pkWnd)
{
	if( !pkWnd ){ return; }

	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	bool bSoulcraft = false;
	if( !m_kSrcItem.IsEmpty() && !m_kTargetItem.IsEmpty())
	{
		PgBase_Item kSrcItem;
		PgBase_Item kTargetItem;
		pkInv->GetItem(m_kSrcItem.kItemPos, kSrcItem);
		pkInv->GetItem(m_kTargetItem.kItemPos, kTargetItem);

		if( kSrcItem.EnchantInfo().Rarity() >= kTargetItem.EnchantInfo().Rarity() )
		{
			bSoulcraft = true;
		}
	}

	XUI::CXUI_CheckButton* pkTmp = NULL;
	pkTmp = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_SOUL_INFO"));
	pkTmp ? pkTmp->Check(bSoulcraft) : 0;

	pkTmp = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_BASE_OPT"));
	pkTmp ? pkTmp->Check(false) : 0;
}

void PgEnchantShift::UpdateTargetImg(XUI::CXUI_Wnd * const pkWnd)
{
	if( !pkWnd ){ return; }

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkSItemDef = kItemDefMgr.GetDef(m_kSrcItem.kItemNo);

	if( NULL==pkSItemDef )
	{
		XUI::CXUI_Wnd* pTmp = NULL;
		pTmp = pkWnd->GetControl(L"IMG_TARGET_WEAPON_BG");
		pTmp ? pTmp->Visible(false) : 0;
		pTmp = pkWnd->GetControl(L"IMG_TARGET_DEFENCE_BG");
		pTmp ? pTmp->Visible(false) : 0;
	}
	else
	{
		if(0 < (EQUIP_LIMIT_WEAPON & pkSItemDef->GetAbil(AT_EQUIP_LIMIT)))
		{
			XUI::CXUI_Wnd* pkTmp = NULL;
			pkTmp = pkWnd->GetControl(L"IMG_TARGET_WEAPON_BG");
			pkTmp ? pkTmp->Visible(m_kTargetItem.IsEmpty()) : 0;
			pkTmp ? pkTmp->UVUpdate(GetEquipUVIndex(m_kSrcItem.kItemNo)) : 0;

			pkTmp = pkWnd->GetControl(L"IMG_TARGET_DEFENCE_BG");
			pkTmp ? pkTmp->Visible(false) : 0;
		}
		else
		{
			XUI::CXUI_Wnd* pkTmp = NULL;
			pkTmp = pkWnd->GetControl(L"IMG_TARGET_WEAPON_BG");
			pkTmp ? pkTmp->Visible(false) : 0;

			pkTmp = pkWnd->GetControl(L"IMG_TARGET_DEFENCE_BG");
			pkTmp ? pkTmp->Visible(m_kTargetItem.IsEmpty()) : 0;
			pkTmp ? pkTmp->UVUpdate(GetEquipUVIndex(m_kSrcItem.kItemNo)) : 0;
		}
	}
}

void PgEnchantShift::UpdateNeedItems(XUI::CXUI_Wnd * const pkWnd)
{
	if( !pkWnd ){ return; }

	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	if( m_kSrcItem.IsEmpty() )
	{
		XUI::CXUI_Wnd* pTmp = NULL;

		XUI::CXUI_Wnd* pWndNeed = pkWnd->GetControl(L"FRM_MT_SLOT0");// 무기/방어구 강화 가루
		if( pWndNeed )
		{
			XUI::CXUI_Wnd* pkTmp = NULL;
			pkTmp = pWndNeed->GetControl(L"ICN_MT");
			pkTmp ? pkTmp->ClearCustomData() : 0;

			pkTmp = pWndNeed->GetControl(L"FRM_NAME");
			pkTmp ? pkTmp->Text(L"") : 0;

			pkTmp = pWndNeed->GetControl(L"FRM_COUNT");
			pkTmp ? pkTmp->Text(L"") : 0;
		}

		XUI::CXUI_Wnd* pWndShift = pkWnd->GetControl(L"FRM_MT_SLOT1");//구슬
		if( pWndShift )
		{
			XUI::CXUI_Wnd* pTmp = NULL;
			pTmp = pWndShift->GetControl(L"ICN_MT");
			pTmp ? pTmp->ClearCustomData() : 0;

			pTmp = pWndShift->GetControl(L"FRM_NAME");
			pTmp ? pTmp->Text(L"") : 0;
			
			pTmp = pWndShift->GetControl(L"FRM_COUNT");
			pTmp ? pTmp->Text(L"") : 0;
		}

		XUI::CXUI_Wnd* pWndCost = pkWnd->GetControl(L"FRM_COST");
		pWndCost ? pWndCost->Text(GetMoneyString(0i64,false)) : 0;
		return;
	}

	PgBase_Item kSrcItem;
	PgBase_Item kTargetItem;
	pkInv->GetItem(m_kSrcItem.kItemPos, kSrcItem);
	pkInv->GetItem(m_kTargetItem.kItemPos, kTargetItem);
	int const iCurLevel = kSrcItem.EnchantInfo().PlusLv();

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkSItemDef = kItemDefMgr.GetDef(m_kSrcItem.kItemNo);
	CItemDef const * pkDItemDef = kItemDefMgr.GetDef(m_kTargetItem.kItemNo);
	TBL_DEF_ITEMENCHANTSHIFT kEnchantShiftPlusInfo;
	if( pkSItemDef && pkDItemDef )
	{ 
		TBL_DEF_ITEMENCHANTSHIFT const * pkPlusInfo = PgItemRarityUpgradeFormula::GetEnchantShiftPlusInfo(pkSItemDef->No(), iCurLevel, pkDItemDef->GetAbil(AT_LEVELLIMIT));
		if(pkPlusInfo)
		{
			kEnchantShiftPlusInfo = *pkPlusInfo;
		}
	}
	

	int const iMaxEnchantShiftItemCount = kEnchantShiftPlusInfo.EnchantItemCount;		// 무기/방어구 강화 가루
	int	const iMaxEnchantShiftGemCount = kEnchantShiftPlusInfo.EnchantShiftGemCount;		// 인첸트 전이 구슬
	m_bHaveNeedItems = true;

	XUI::CXUI_Wnd* pWndNeed = pkWnd->GetControl(L"FRM_MT_SLOT0");// 무기/방어구 강화 가루
	if( pWndNeed )
	{
		int const iItemNo = kEnchantShiftPlusInfo.EnchantItemNo;
		const wchar_t * pkName = NULL;

		CItemDef const *pDef = kItemDefMgr.GetDef(iItemNo);
		if( pDef )
		{
			GetDefString(pDef->NameNo(), pkName);
		}

		int const iHaveCount = pkInv->GetTotalCount(iItemNo);


		XUI::CXUI_Wnd* pkTmp = NULL;
		pkTmp = pWndNeed->GetControl(L"ICN_MT");
		pkTmp ? pkTmp->SetCustomData(&iItemNo, sizeof(iItemNo)) : 0;	

		pkTmp = pWndNeed->GetControl(L"FRM_NAME");
		if(pkTmp)
		{
			pkTmp->Text(pkName ? pkName : L"");
		}

		m_bHaveNeedItems = m_bHaveNeedItems && !(iHaveCount<iMaxEnchantShiftItemCount);
		BM::vstring vStr(TTW(1273));
		vStr.Replace(L"#COLOR#", TTW(iHaveCount<iMaxEnchantShiftItemCount ? 1275 : 1274));
		vStr.Replace(L"#HAVE#", std::min(iHaveCount,iMaxEnchantShiftItemCount));
		vStr.Replace(L"#NEED#", iMaxEnchantShiftItemCount);
		pkTmp = pWndNeed->GetControl(L"FRM_COUNT");
		if(pkTmp)
		{
			pkTmp->Text(iMaxEnchantShiftItemCount ? vStr : L"");
		}
	}

	XUI::CXUI_Wnd* pWndShift = pkWnd->GetControl(L"FRM_MT_SLOT1");//구슬
	if( pWndShift )
	{
		CItemDef const * pkDItemDef = kItemDefMgr.GetDef(m_kTargetItem.kItemNo);
		int const iGemItemNo = kEnchantShiftPlusInfo.EnchantShiftGemNo;

		XUI::CXUI_Wnd* pTmp = NULL;
		pTmp = pWndShift->GetControl(L"ICN_MT");
		if(pTmp)
		{
			if( iGemItemNo )
			{
				pTmp->SetCustomData(&iGemItemNo, sizeof(iGemItemNo));
			}
			else
			{
				pTmp->ClearCustomData();
			}
			pTmp->GrayScale(0==iMaxEnchantShiftGemCount);
		}

		const wchar_t * pkName = NULL;
		CItemDef const *pDef = kItemDefMgr.GetDef(iGemItemNo);
		if( pDef )
		{
			GetDefString(pDef->NameNo(), pkName);
		}
		if( pkName )
		{
			Quest::SetCutedTextLimitLength(pWndShift->GetControl(L"FRM_NAME"), pkName, L"...");
		}
		else
		{
			pTmp = pWndNeed->GetControl(L"FRM_NAME");
			pTmp ? pTmp->Text(L"") : 0;
		}

		int const iHaveCount = pkInv->GetTotalCount(iGemItemNo);

		m_bHaveNeedItems = m_bHaveNeedItems && !(iHaveCount<iMaxEnchantShiftGemCount);
		BM::vstring vStr(TTW(1273));
		vStr.Replace(L"#COLOR#", TTW(iHaveCount<iMaxEnchantShiftGemCount ? 1275 : 1274));
		vStr.Replace(L"#HAVE#", std::min(iHaveCount,iMaxEnchantShiftGemCount));
		vStr.Replace(L"#NEED#", iMaxEnchantShiftGemCount);
		pTmp = pWndShift->GetControl(L"FRM_COUNT");
		pTmp ? pTmp->Text(iMaxEnchantShiftGemCount ? vStr : L"") : 0;
	}

	XUI::CXUI_Wnd* pTmp = NULL;
	__int64 const iCost = kEnchantShiftPlusInfo.EnchantShiftCost;
	pTmp = pkWnd->GetControl(L"FRM_COST");
	pTmp ? pTmp->Text(GetMoneyString(iCost,false)) : 0;
	m_bHaveShiftCost = (iCost <= pkInv->Money());
}

void PgEnchantShift::UpdateEnchantShiftRate(XUI::CXUI_Wnd * const pkWnd)
{
	if( !pkWnd ){ return; }
	if( m_kSrcItem.IsEmpty() ){ return; }

	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	PgBase_Item kItem;
	pkInv->GetItem(m_kSrcItem.kItemPos, kItem);
	int const iCurLevel = kItem.EnchantInfo().PlusLv();

	PgBase_Item kTargetItem;
	MakeEnchantInfo(m_kSrcItem.kItemPos, m_kTargetItem.kItemPos, kTargetItem, IsShiftSoul(), IsShiftSpecial(), IsShiftSocket(), false);	

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkSItemDef = kItemDefMgr.GetDef(m_kSrcItem.kItemNo);
	CItemDef const * pkDItemDef = kItemDefMgr.GetDef(m_kTargetItem.kItemNo);
	TBL_DEF_ITEMENCHANTSHIFT const * pkPlusInfo = NULL;
	if( pkSItemDef && pkDItemDef )
	{ 
		pkPlusInfo = PgItemRarityUpgradeFormula::GetEnchantShiftPlusInfo(pkSItemDef->No(), iCurLevel, pkDItemDef->GetAbil(AT_LEVELLIMIT));
	}

	int iTotalRate = 0;
	for(int i = 0;i < ESR_NUM && pkPlusInfo;++i)
	{
		iTotalRate += pkPlusInfo->EnchantShiftRate[i];
	}

	XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SLOT"));
	if( !pBuild ){ return; }

	wchar_t wszTemp[MAX_PATH] = {0,};
	int const iMaxSlot = pBuild->CountX() * pBuild->CountY();
	float fSaveRate = 0.f;
	for(int i=iMaxSlot-1; 0<=i; --i)
	{
		BM::vstring vStr(L"FRM_RESULT_SLOT");
		vStr += i;

		XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
		if( pSlot )
		{
			E_INSURANCETYPE const eInsurType = static_cast<E_INSURANCETYPE>(i);
			eEnchantShiftRate const eRate = lwEnchantShift::GetSlotTypeC2S(eInsurType);
			float fRate = 0.f;
			if(pkPlusInfo && iTotalRate)
			{
				fRate = (pkPlusInfo->EnchantShiftRate[eRate]/static_cast<float>(iTotalRate)) * 100;
			}

			//인챈트 레벨
			if( false==IsOpenResultSlot(eInsurType, m_kTargetItem.kItemPos) )
			{
				fSaveRate += fRate;
				fRate = 0.f;
			}
			else if(100.f<=fSaveRate+fRate && 0==i)
			{
				fRate = 0.f;
			}
			else
			{
				fRate += fSaveRate;
				fSaveRate = 0.f;
			}

			bool const bVisible = (fRate > 0.f) || m_kTargetItem.IsEmpty();
			if(pkPlusInfo)
			{
				swprintf_s(wszTemp, MAX_PATH, L"%.1f%%", fRate);
			}
			else
			{
				wszTemp[0] = NULL;
			}

			XUI::CXUI_Wnd* pTmp = pSlot->GetControl(L"FRM_PER");
			if(pTmp)
			{
				pTmp->Text(wszTemp);
				pTmp->SetCustomData(&fRate, sizeof(fRate));
				pTmp->Visible(bVisible);
			}
			
			pTmp = pSlot->GetControl(L"FRM_RESULT_CV");
			pTmp ? pTmp->Visible(bVisible) : 0;

			pTmp = pSlot->GetControl(L"ICN_RST");
			pTmp ? pTmp->Visible(bVisible) : 0;

			pTmp = pSlot->GetControl(L"IMG_CLOSE");
			if(pTmp)
			{
				pTmp->Visible(!bVisible);

				lwUIWnd kClose( pTmp );
				if(false==bVisible)
				{
					E_ITEM_GRADE const eSrcItemGrade = ::GetItemGrade(kTargetItem);
					std::wstring const kGrade(TTW(50300 + eSrcItemGrade));//등급명
					std::wstring kGradeColor;
					int iMaxLv = 0;

					SetGradeColor(eSrcItemGrade, false, kGradeColor);
					PgItemRarityUpgradeFormula::GetMaxGradeLevelRestrict( eSrcItemGrade, false, iMaxLv, kTargetItem);

					BM::vstring vStr(TTW(1297));
					vStr.Replace(L"#NORMAL#", IPULL_NORMAL_LIMIT);
					vStr.Replace(L"#RARE#", IPULL_RARE_LIMIT);
					vStr.Replace(L"#UNIQUE#", IPULL_UNIQUE_LIMIT);
					vStr.Replace(L"#ARTIFACT#", IPULL_ARTIFACT_LIMIT);
					vStr.Replace(L"#LEGEND#", IPULL_LEGEND_LIMIT);
					vStr.Replace(L"#GRADE_COLOR#", kGradeColor);
					vStr.Replace(L"#GRADE#", kGrade);
					vStr.Replace(L"#MAX#", iMaxLv);
					
					kClose.SetCustomDataAsStr(MB(vStr));

					//등록된 보험해제
					ClearInsurance(eInsurType);
				}
				else
				{
					kClose.ClearCustomData();
				}
			}
		}
	}
}

bool PgEnchantShift::IsShiftSoul()const
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
	if( !pkWnd ){ return false; }

	XUI::CXUI_CheckButton* pkTmp = NULL;
	pkTmp = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_SOUL_INFO"));
	return pkTmp ? pkTmp->Check() : false;
}

bool PgEnchantShift::IsShiftSpecial()const
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
	if( !pkWnd ){ return false; }

	XUI::CXUI_CheckButton* pkTmp = NULL;
	pkTmp = dynamic_cast<XUI::CXUI_CheckButton*>(pkWnd->GetControl(L"CBTN_BASE_OPT"));
	return pkTmp ? pkTmp->Check() : false;
}

bool PgEnchantShift::IsShiftSocket()const
{
	return false;
}



bool CheckInsurance(SItemPos & const kSrcItemPos, int const iItemNo)
{
	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return false; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	PgBase_Item kSrcItem;
	if(S_OK != pkInv->GetItem(kSrcItemPos, kSrcItem))
	{
		return false;
	}
	SEnchantInfo const & kSEnchantInfo = kSrcItem.EnchantInfo();

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkDef = kItemDefMgr.GetDef(iItemNo);
	if( pkDef )
	{
		int const iMinLv = pkDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
		int const iMaxLv = pkDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_2);

		if( (kSEnchantInfo.PlusLv() < iMinLv) ||
			(kSEnchantInfo.PlusLv() > iMaxLv) )
		{
			return false;
		}
		return true;
	}

	return false;
}

bool PgEnchantShift::IsOpenResultSlot(PgEnchantShift::E_INSURANCETYPE const eType, SItemPos const& rkTargetItemPos)const
{
	if( m_kSrcItem.IsEmpty() || rkTargetItemPos==SItemPos::NullData() ){ return false; }

	PgBase_Item kTargetItem;
	if( false == MakeEnchantInfo(m_kSrcItem.kItemPos, rkTargetItemPos, kTargetItem, IsShiftSoul(), IsShiftSpecial(), IsShiftSocket(), false) )
	{
		return false;
	}

	E_ITEM_GRADE const eItemGrade = ::GetItemGrade(kTargetItem);

	int iMaxLv = 0;
	if( false == PgItemRarityUpgradeFormula::GetMaxGradeLevelRestrict( eItemGrade, false, iMaxLv, kTargetItem ) )
	{
		return false;
	}

	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return false; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	PgBase_Item kItem;
	if( S_OK != pkInv->GetItem(m_kSrcItem.kItemPos, kItem) )
	{
		return false;
	}
	int const iCurLevel = kItem.EnchantInfo().PlusLv();

	int const iShiftValue = static_cast<int>(eType)-EINSUR_ZERO;
	int iNewLevel = iCurLevel + iShiftValue;

	if(iNewLevel > iMaxLv)
	{
		return false;
	}

	return true;
}

void PgEnchantShift::UpdateResultSlot(XUI::CXUI_Wnd * const pkWnd)
{
	if( !pkWnd ){ return; }

	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return; }
	PgInventory * const pkInv = pkPlayer->GetInven();

	PgBase_Item kItem;
	if( S_OK != pkInv->GetItem(m_kSrcItem.kItemPos, kItem) )
	{
		return;
	}
	SEnchantInfo const & kSEnchantInfo = kItem.EnchantInfo();
	int const iCurLevel = kItem.EnchantInfo().PlusLv();

	PgBase_Item kTargetItem;
	MakeEnchantInfo(m_kSrcItem.kItemPos, m_kTargetItem.kItemPos, kTargetItem, IsShiftSoul(), IsShiftSpecial(), IsShiftSocket(), false);

	E_ITEM_GRADE const eItemGrade = ::GetItemGrade(kTargetItem);

	int iMaxLv = 0;
	PgItemRarityUpgradeFormula::GetMaxGradeLevelRestrict( eItemGrade, false, iMaxLv, kTargetItem );

	XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SLOT"));
	if( pBuild )
	{
		SEnchantInfo kNewEnchantInfo = kTargetItem.EnchantInfo();
		int const iMaxSlot = pBuild->CountX() * pBuild->CountY();
		for(int i=0; i<iMaxSlot; ++i)
		{
			if(EINSUR_DESTORY == i)
			{
				continue;
			}

			BM::vstring vStr(L"FRM_RESULT_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
			if( pSlot )
			{
				int iNewLevel = iCurLevel;
				switch(i)
				{
				case EINSUR_MINUS_2:{	iNewLevel -= 2;	}break;
				case EINSUR_MINUS_1:{	iNewLevel -= 1;	}break;
				case EINSUR_PLUS_1: {	iNewLevel += 1;	}break;
				case EINSUR_PLUS_2:	{	iNewLevel += 2;	}break;
				}
				iNewLevel = std::min<int>(iNewLevel, iMaxLv);

				kNewEnchantInfo.PlusType(kSEnchantInfo.PlusType());
				kNewEnchantInfo.PlusLv(iNewLevel);

				PgBase_Item kCopyItem(kTargetItem);
				kCopyItem.EnchantInfo(kNewEnchantInfo);

				XUI::CXUI_Wnd *pTmp = NULL;
				pTmp = pSlot->GetControl(L"ICN_RST");
				if(pTmp)
				{
					if(iNewLevel)
					{
						lwUIWnd kWnd(pTmp);
						kWnd.SetCustomDataAsSItem(kCopyItem);
					}
					else
					{
						pTmp->ClearCustomData();
					}
				}
			}
		}
	}
}

int PgEnchantShift::GetCurrentInsurance()const
{
	PgPlayer * const pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return 0; }
	PgInventory * const pkInv = pkPlayer->GetInven();
	int iItemNo = 0;
	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const * pkSItemDef = kItemDefMgr.GetDef(m_kSrcItem.kItemNo);
	CItemDef const * pkDItemDef = kItemDefMgr.GetDef(m_kTargetItem.kItemNo);
	if( pkSItemDef && pkDItemDef )
	{ 
		PgBase_Item kSrcItem;
		pkInv->GetItem(m_kSrcItem.kItemPos, kSrcItem);

		TBL_DEF_ITEMENCHANTSHIFT const * pkPlusInfo = PgItemRarityUpgradeFormula::GetEnchantShiftPlusInfo(pkSItemDef->No(), kSrcItem.EnchantInfo().PlusLv(), pkDItemDef->GetAbil(AT_LEVELLIMIT));
		if(pkPlusInfo)
		{
			iItemNo = pkPlusInfo->InsuranceItemNo;
		}
	}
	return iItemNo;
}

void PgEnchantShift::UpdateInsurance(XUI::CXUI_Wnd * const pkWnd)
{
	if( !pkWnd ){ return; }

	int const iItemNo = GetCurrentInsurance();

	bool const bVisible = (ES_STEP_3 == GetState());
	XUI::CXUI_Builder* pBuild = dynamic_cast<XUI::CXUI_Builder*>(pkWnd->GetControl(L"BLD_SLOT"));
	if( pBuild )
	{
		int const iMaxSlot = pBuild->CountX() * pBuild->CountY();
		int iLastClose = iMaxSlot;
		for(int i=iMaxSlot-1; i>=0; --i)
		{
			BM::vstring vStr(L"FRM_RESULT_SLOT");
			vStr += i;

			XUI::CXUI_Wnd* pSlot = pkWnd->GetControl(vStr);
			if( pSlot )
			{
				XUI::CXUI_Wnd* pTmp = NULL;
				pTmp = pSlot->GetControl(L"IMG_CLOSE");
				bool bNotClose = bVisible && (pTmp ? !pTmp->Visible() : false);
				int const iOldLastClose = iLastClose;
				if( !bNotClose )
				{
					iLastClose = i;
				}
				else if(bNotClose && iOldLastClose == i+1 && 0!=i)
				{
					bNotClose = false;
				}

				pTmp = pSlot->GetControl(L"ICN_RATEUP");
				if(pTmp)
				{
					pTmp->Visible(bNotClose);
					pTmp->GrayScale(!bNotClose || m_kInsurance[i].IsEmpty());
					if( m_kInsurance[i].IsEmpty() )
					{
						int iSlotItemNo = iItemNo;
						if( LOCAL_MGR::NC_JAPAN != g_kLocal.ServiceRegion() )
						{//일본을 제외하고
							if( EINSUR_DESTORY != i)
							{//파괴 슬롯 아니면 캐시 보험 스크롤로 기본 이미지 변경.
								iSlotItemNo = 98000040;
							}
						}
						pTmp->SetCustomData(&iSlotItemNo, sizeof(iSlotItemNo));
					}
					else
					{
						pTmp->SetCustomData(&m_kInsurance[i].kItemNo, sizeof(m_kInsurance[i].kItemNo));
					}
				}

				pTmp = pSlot->GetControl(L"BTN_DEREG");
				pTmp ? pTmp->Visible(bNotClose && EINSUR_PLUS_2!=i && !m_kInsurance[i].IsEmpty()) : 0;

				pTmp = pSlot->GetControl(L"BTN_REG");
				pTmp ? pTmp->Visible(bNotClose && EINSUR_PLUS_2!=i && m_kInsurance[i].IsEmpty()) : 0;
			}
		}
	}
}

void PgEnchantShift::RefreshInsurance()
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() ){ return; }
	PgInventory* pkInv = pkPlayer->GetInven();

	int iItemNo = GetCurrentInsurance();

	S_ITEM_INFO kTempInsurance[EINSUR_MAX];
	//임시 컨테이너에 이전에 등록했던 목록 저장
	kTempInsurance[EINSUR_DESTORY] = m_kInsurance[EINSUR_DESTORY];
	kTempInsurance[EINSUR_MINUS_2] = m_kInsurance[EINSUR_MINUS_2];
	kTempInsurance[EINSUR_MINUS_1] = m_kInsurance[EINSUR_MINUS_1];
	kTempInsurance[EINSUR_ZERO] = m_kInsurance[EINSUR_ZERO];
	kTempInsurance[EINSUR_PLUS_1] = m_kInsurance[EINSUR_PLUS_1];
	kTempInsurance[EINSUR_PLUS_2] = m_kInsurance[EINSUR_PLUS_2];
	//테이블 비우기.
	ClearInsurance(EINSUR_MAX);
	//임시테이블의 내용 적용시키기.
	for(int i=0; i<EINSUR_MAX; ++i)
	{
		if( kTempInsurance[i].IsEmpty() ){ continue; }

		if(false==SetInsurance(static_cast<E_INSURANCETYPE>(i), kTempInsurance[i].kItemNo, true))
		{
			kTempInsurance[i].Clear();
		}
	}

	std::map<SItemPos,int> kContUseInsurance;
	for(int i=0; i<EINSUR_MAX; ++i)
	{
		if( m_kInsurance[i].IsEmpty() ){ continue; }

		++kContUseInsurance[m_kInsurance[i].kItemPos];
	}

	for(std::map<SItemPos,int>::const_iterator iter = kContUseInsurance.begin();iter != kContUseInsurance.end();++iter)
	{
		SItemPos const & kInsurancePos = (*iter).first;
		int const iInsuranceCount = (*iter).second;

		PgBase_Item kInsuranceItem;
		if( (S_OK != pkInv->GetItem(kInsurancePos, kInsuranceItem)) ||
			(kInsuranceItem.Count() < iInsuranceCount) )
		{
			for(int i=0; i<EINSUR_MAX; ++i)
			{
				if( m_kInsurance[i].kItemPos == kInsurancePos )
				{
					m_kInsurance[i].Clear();
					continue;
				}
			}
		}
	}
}


void PgEnchantShift::CallWndInv()
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
	if( !pkWnd ){ return; }

	XUI::CXUI_Wnd* pInv = XUIMgr.Get(L"Inv");
	if( pInv )
	{
		pInv->Visible(true);
		m_bIsOpenInv = true;
	}
	else
	{
		m_bIsOpenInv = false;
		pInv = XUIMgr.Call(L"Inv");
	}

	if( pInv )
	{
		POINT3I kLocation = pkWnd->Location();
		POINT2 kSize = pkWnd->Size();
		POINT2 kPos(kLocation.x+kSize.x+WND_ENCHANTSHIFT_SPACE, kLocation.y);
		pInv->Location(kPos);

		lua_tinker::call<void,int>("PreChangeInvViewGroupTop", 1);
	}
}

bool PgEnchantShift::IsUpgradeEnchantTargetItem()const
{
	PgPlayer* pkPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pkPlayer || !pkPlayer->GetInven() )
	{
		return false;
	}
	PgInventory* pkInv = pkPlayer->GetInven();

	PgBase_Item kItem;
	if( S_OK == pkInv->GetItem( m_kTargetItem.kItemPos, kItem ) )
	{
		return kItem.EnchantInfo().PlusLv() > 0;
	}
	return false;
}

bool PgEnchantShift::IsReset()const
{
	return ES_RESET==m_eState;
}

void PgEnchantShift::SetState(PgEnchantShift::E_STATE const eState, bool const bLoop)
{
	E_STATE const eOldState = m_eState;
	m_eState = eState;

	switch(m_eState)
	{
	case ES_OPEN:
		{
			Clear(true);
			CallWndInv();
			SetState(ES_STEP_1);
		}break;
	case ES_STEP_1:
		{
			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
			if( !pkWnd ){ return; }

			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_SHIFT"));
			pButton ? pButton->Disable(true) : 0;

			!bLoop ? lwEnchantShift::WndInit(pkWnd) : 0;

			XUI::CXUI_Wnd* pkTmp = pkWnd->GetControl(L"SFRM_STEP_TEXT");
			pkTmp ? pkTmp->Text(TTW(1252)) : 0;

			if( S_OK != CheckSrcItem(m_kSrcItem.kItemPos) )
			{
				ClearSrcItem();
			}

			if( S_OK != CheckTargetItem(m_kTargetItem.kItemPos) )
			{
				ClearTargetItem();
			}

			!bLoop ? UpdateCheckOption(pkWnd) : 0;
			UpdateTargetImg(pkWnd);
			UpdateNeedItems(pkWnd);
			UpdateEnchantShiftRate(pkWnd);
			UpdateResultSlot(pkWnd);
			UpdateInsurance(pkWnd);
			UpdateArrow(pkWnd, EAT_SOURCE);

			if( !m_kSrcItem.IsEmpty() )
			{
				SetState(ES_STEP_2, true);
				!bLoop ? lwPlaySoundByID("inchant-change-steppass") : 0;	//스텝넘김
			}
		}break;
	case ES_STEP_2:
		{
			!bLoop ? lwPlaySoundByID("inchant-change-steppass") : 0;	//스텝넘김

			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
			if( !pkWnd ){ return; }

			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_SHIFT"));
			pButton ? pButton->Disable(true) : 0;

			std::wstring kNeedItem;
			MakeEquipName(m_kSrcItem.kItemNo, kNeedItem);

			BM::vstring vStr(TTW(1253));
			vStr.Replace(L"#ITEM#", kNeedItem);
			XUI::CXUI_Wnd* pkTmp = pkWnd->GetControl(L"SFRM_STEP_TEXT");
			pkTmp ? pkTmp->Text(static_cast<std::wstring>(vStr)) : 0;

			if( S_OK != CheckTargetItem(m_kTargetItem.kItemPos) )
			{
				ClearTargetItem();
			}

			UpdateTargetImg(pkWnd);
			UpdateNeedItems(pkWnd);
			UpdateEnchantShiftRate(pkWnd);
			UpdateResultSlot(pkWnd);
			UpdateInsurance(pkWnd);
			UpdateArrow(pkWnd, EAT_TARGET);

			if( !m_kTargetItem.IsEmpty() )
			{
				SetState(ES_STEP_3, true);
				!bLoop ? lwPlaySoundByID("inchant-change-steppass") : 0;	//스텝넘김
			}
		}break;
	case ES_STEP_3:
		{
			!bLoop ? lwPlaySoundByID("inchant-change-steppass") : 0;	//스텝넘김

			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
			if( !pkWnd ){ return; }

			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_SHIFT"));
			pButton ? pButton->Disable(false) : 0;

			XUI::CXUI_Wnd* pkTmp = pkWnd->GetControl(L"SFRM_STEP_TEXT");
			pkTmp ? pkTmp->Text(TTW(1254)) : 0;

			!bLoop ? UpdateCheckOption(pkWnd) : 0;
			UpdateTargetImg(pkWnd);
			UpdateNeedItems(pkWnd);
			UpdateEnchantShiftRate(pkWnd);
			UpdateResultSlot(pkWnd);
			RefreshInsurance();
			UpdateInsurance(pkWnd);
			UpdateArrow(pkWnd, EAT_INSURANCE);
		}break;
	case ES_PROCESS_START:
		{
			lwSetReqInvWearableUpdate(true);
			if( ES_RESET==eOldState )
			{
				SetState(ES_RESET_OK,true);
				return;
			}

			if( m_kSrcItem.IsEmpty() || m_kTargetItem.IsEmpty())
			{
				lwAddWarnDataTT(1293);
				SetState(ES_STEP_1,true);
				return;
			}

			if( !m_bHaveNeedItems )
			{
				lwAddWarnDataTT(1294);
				SetState(ES_STEP_1,true);
				return;
			}

			if( !m_bHaveShiftCost )
			{
				lwAddWarnDataTT(1266);
				SetState(ES_STEP_1,true);
				return;
			}

			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
			if( !pkWnd ){ return; }

			XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_SHIFT"));
			pButton ? pButton->Disable(true) : 0;

			UpdateArrow(pkWnd, EAT_NONE);

			m_eResult = ESR_NUM;
			SetState(ES_PROCESSING,true);
			SetProcessState(EPS_INIT);
		}break;
	case ES_PROCESSING:
		{
		}break;
	case ES_RESET:
		{
			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
			if( pkWnd )
			{
				XUI::CXUI_Button* pButton = dynamic_cast<XUI::CXUI_Button*>(pkWnd->GetControl(L"BTN_SHIFT"));
				if(pButton)
				{
					std::wstring kText( TTW(400533) );
					kText += TTW(2200);
					pButton->Text(kText);
					pButton->Disable(false);
				}
			}
		}break;
	case ES_RESET_OK:
		{
			XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
			lwEnchantShift::E_WNDINIT_TYPE const bInitType = bLoop ? lwEnchantShift::EWIT_NOT_OPTION : lwEnchantShift::EWIT_ALL;
			lwEnchantShift::WndInit(pkWnd, bInitType);
			SetState(ES_STEP_1,true);
		}break;
	}
}

void PgEnchantShift::SetProcessState(E_PROCESS_STATE const eState)
{
	m_eProcessState = eState;
}

void PgEnchantShift::Refresh(bool const bReset)
{
	lwSetReqInvWearableUpdate(true);
	SetState(bReset ? ES_STEP_1 : GetState(),false);
}

PgEnchantShift::E_ARROWTYPE PgEnchantShift::GetNowArrowType()const
{
	switch(GetState())
	{
	case ES_STEP_1:	{ return EAT_SOURCE;	}
	case ES_STEP_2:	{ return EAT_TARGET;	}
	case ES_STEP_3:	{ return EAT_INSURANCE;	}
	}
	return EAT_NONE;
}

void PgEnchantShift::RefreshResultUI()
{
	XUI::CXUI_Wnd* pkWnd = XUIMgr.Get(L"SFRM_ENCHANT_SHIFT");
	if( !pkWnd ){ return; }

	UpdateEnchantShiftRate(pkWnd);
	UpdateResultSlot(pkWnd);
	UpdateInsurance(pkWnd);
	UpdateArrow(pkWnd, GetNowArrowType());
}
