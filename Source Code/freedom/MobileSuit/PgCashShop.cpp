#include "stdafx.h"
#include "PgCashShop.h"
#include "variant/PgControlDefMgr.h"
#include "variant/Cashshop.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "lwUI.h"
#include "PgFurniture.h"
#include "PgUIModel.h"
#include "PgUIScene.h"

extern DWORD lwGetServerElapsedTime32(bool const bUseSavedTime = true);

CONT_CS_ITEMTYPE_TO_ITEMNO PgCashShop::m_kItemTypeToItemNo;

PgCashShop::PgCashShop() 
	: m_kActorModel("", "")
	, m_pkItemFurniture(NULL)
	, m_kStaticItemType(-1)
	, m_kStaticItemSubKey(-1)
	, m_kStaticItemNo(0)
{ 
	Init();
	m_kItemTypeToItemNo.clear();
}

PgCashShop::~PgCashShop()
{
}

bool PgCashShop::IsPackegeArticle(TBL_DEF_CASH_SHOP_ARTICLE const& kArticle)
{
	return (kArticle.bSaleType == ECST_PACKEGE);
}

bool PgCashShop::IsStyleItem(CItemDef const*& pDef)
{
	if( !pDef )
	{
		return false;
	}

	return ( UICT_STYLEITEM == pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) );
}

void PgCashShop::InitMileage()
{
	PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
	if( pPlayer )
	{
		pPlayer->SetAbil64(AT_MILEAGE_TOTAL_CASH,0);
		pPlayer->SetAbil64(AT_MILEAGE_100,0);
		pPlayer->SetAbil64(AT_MILEAGE_1000,0);
	}
}

void PgCashShop::Init()
{
	m_kActorModel = SModelActorInfo("","");
	m_iRecentTopCategory = -1;
	m_iRecentSubCategory = -1;
	m_bUseMileage = false;
	m_eShopType = ECASH_SHOP;
	m_i64ItemMileage = 0;
	m_i64TotalPrice = 0;
	InitPageArticle();
	m_kGiftContRecv.clear();
	m_kGiftContSend.clear();
	ClearPreviewEquip();
	m_kCashShopData.clear();
	m_kMileageShopData.clear();
}

bool PgCashShop::SetPreviewEquip(TBL_DEF_CASH_SHOP_ARTICLE const& Item, RECV_UI_EQUIP_PREVIEW_CONT& UIItemCont)
{
	GET_DEF(CItemDefMgr, kItemDefMgr);

	//내 성별을 찾는다.
	PgPlayer*	pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return false;
	}
	CONT_DEFITEM const* pkDefItem = NULL;
	g_kTblDataMgr.GetContDef(pkDefItem);
	if( !pkDefItem )
	{
		return false;
	}

	int const iGender = pPlayer->GetAbil(AT_GENDER);

	if( Item.bSaleType != ECST_PACKEGE )
	{
		CONT_CASH_SHOP_ITEM::const_iterator item_iter = Item.kContCashItem.begin();
		if( item_iter == Item.kContCashItem.end() || Item.kContCashItem.size() != 1 )
		{
			// 일반 아이템이 아니다
			return false;
		}
		CONT_CASH_SHOP_ITEM::value_type const& kCashItem = (*item_iter);

		CItemDef const *pDef = kItemDefMgr.GetDef(kCashItem.iItemNo);
		if(!pDef)	
		{
			return false;
		}
		
		int const iGenderLimit = pDef->GetAbil(AT_GENDERLIMIT);
		if( (iGenderLimit != UG_UNISEX) && (iGenderLimit != iGender))
		{//다른 성별 아이템이므로 에러 출력
			lwAddWarnDataTT(1992);
			return false;
		}
		
		if(!IS_CLASS_LIMIT(pDef->GetAbil64(AT_CLASSLIMIT), pPlayer->GetAbil(AT_CLASS)))
		{//다른 클래스 아이템이므로 에러 출력
			lwAddWarnDataTT(1993);
			return false;
		}

		if(pDef->CanEquip())	//장착 할 수 있는 아이템이면
		{
			// 컨테이너 검사 아이템이 1개인데 패키지면 클리어한다
			if( !m_kPreviewEqItem.empty() )
			{
				EQUIP_PREVIEW_CONT::iterator Prev_iter = m_kPreviewEqItem.begin();
				while( Prev_iter != m_kPreviewEqItem.end() )
				{
					EQUIP_PREVIEW_CONT::value_type const& PrevItem = (*Prev_iter);

					if( PrevItem.bSaleType == ECST_PACKEGE )
					{
						ClearPreviewEquip();
						break;
					}
					else
					{
						if( PrevItem.iIdx == Item.iIdx )
						{
							return false;
						}

						bool bIsBreak = false;

						CONT_CASH_SHOP_ITEM::const_iterator	item_it = PrevItem.kContCashItem.begin();
						while( item_it != PrevItem.kContCashItem.end() )
						{
							CONT_CASH_SHOP_ITEM::value_type const& kNewItem = (*item_it);

							CItemDef const *pDestDef = kItemDefMgr.GetDef(kNewItem.iItemNo);
							if( !pDestDef )
							{//없는 아이템?
								assert(0);
								return false;
							}
							else if( pDestDef->CanEquip() && (pDef->EquipPos() == pDestDef->EquipPos()) )
							{//착용위치가 같은 아이템이다
								DelPreviewEquip(Item.iIdx);
								m_kPreviewEqItem.erase(Prev_iter);
								bIsBreak = true;
								break;
							}
							++item_it;
						}

						if( bIsBreak )
						{
							break;
						}
					}
					++Prev_iter;
				}
			}
			m_kPreviewEqItem.push_back(Item);
			SetPreviewEquipItem(kCashItem.iItemNo);
		}
		else
		{
			if( UICT_STYLEITEM != pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE) )
			{
				return false;	
			}

			EQUIP_PREVIEW_CONT::iterator Prev_iter = m_kPreviewEqItem.begin();
			while( Prev_iter != m_kPreviewEqItem.end() )
			{
				EQUIP_PREVIEW_CONT::value_type const& PrevItem = (*Prev_iter);

				if( PrevItem.bSaleType == ECST_PACKEGE )
				{
					ClearPreviewEquip();
					break;
				}
				else
				{
					if( PrevItem.iIdx == Item.iIdx )
					{
						return false;
					}

					bool bIsBreak = false;

					CONT_CASH_SHOP_ITEM::const_iterator	item_it = PrevItem.kContCashItem.begin();
					while( item_it != PrevItem.kContCashItem.end() )
					{
						CONT_CASH_SHOP_ITEM::value_type const& kNewItem = (*item_it);

						CItemDef const *pDestDef = kItemDefMgr.GetDef(kNewItem.iItemNo);
						if( pDestDef && !pDestDef->CanEquip() )
						{
							int const iNewType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
							int const iOldType = pDestDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
							if( iNewType == iOldType )
							{
								int const iNewTypeCustomVal1 = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1 );
								int const iOldTypeCustomVal1 = pDestDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1 );
								if(iNewTypeCustomVal1 == iOldTypeCustomVal1)
								{
									m_kPreviewEqItem.erase(Prev_iter);
									bIsBreak = true;
									break;
								}
							}
						}
						++item_it;
					}

					if( bIsBreak )
					{
						break;
					}
				}
				++Prev_iter;
			}
			m_kPreviewEqItem.push_back(Item);
			SetPreviewEquipItem(kCashItem.iItemNo);
		}
	}
	else
	{
		EQUIP_PREVIEW_CONT::iterator iter = m_kPreviewEqItem.begin();
		if( iter != m_kPreviewEqItem.end() )
		{//아이템이 있을경우
			if( iter->iIdx == Item.iIdx )
			{//같은 패키지다
				return false;
			}
		}

		CONT_CASH_SHOP_ITEM::const_iterator item_it = Item.kContCashItem.begin();
		while(Item.kContCashItem.end() != item_it)
		{
			CItemDef const *pDef = kItemDefMgr.GetDef((*item_it).iItemNo);
			if( !pDef )
			{
				++item_it;
				continue;	
			}
			int ItemNo = (*item_it).iItemNo;
			int ClassNo = pPlayer->GetAbil(AT_CLASS);
			CONT_DEFITEM::const_iterator def_item = pkDefItem->find(ItemNo);
			if( pkDefItem->end() == def_item )
			{
				return false;
			}
			if( pDef->CanEquip())
			{
				if( IS_CLASS_LIMIT(pDef->GetAbil64(AT_CLASSLIMIT), pPlayer->GetAbil(AT_CLASS)) )
				{//패키지는 Only다 다른것 다 지운다
					ClearPreviewEquip();
					m_kPreviewEqItem.push_back(Item);
					break;
				}	
			}
			else
			{
				int const iCustomType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
				if( iCustomType == UICT_STYLEITEM )
				{
					ClearPreviewEquip();
					m_kPreviewEqItem.push_back(Item);
					break;
				}
			}
			
			//내가 착용 불가능한게 하나라도 있으면 리턴시키자
			int const iGenderLimit = pDef->GetAbil(AT_GENDERLIMIT);
			if( (iGenderLimit != UG_UNISEX) && (iGenderLimit != iGender))
			{//다른 성별 아이템이므로 에러 출력
				lwAddWarnDataTT(1992);
				return false;
			}
			if(def_item->second.i64ClassLimit == 0 )
			{
				lwAddWarnDataTT(1997);
				return false;
			}
			else if(def_item->second.i64DraClassLimit == 0 )
			{
				lwAddWarnDataTT(1997);
				return false;
			}

			++item_it;
		}
	}

	GetPreviewEquipItemList( Item.bSaleType == ECST_PACKEGE, UIItemCont);
	return true;
}

void PgCashShop::GetPreviewEquipItemList(bool const bSetterIsCall, RECV_UI_EQUIP_PREVIEW_CONT& UIItemCont) const
{
	//내 성별을 찾는다.
	PgPlayer*	pPlayer = g_kPilotMan.GetPlayerUnit();
	if( !pPlayer )
	{
		return;
	}
	int const iGender = pPlayer->GetAbil(AT_GENDER);

	//장착 가능한 것만 뽑아
	EQUIP_PREVIEW_CONT::const_iterator	iter = m_kPreviewEqItem.begin();
	while( iter != m_kPreviewEqItem.end() )
	{
		GET_DEF(CItemDefMgr, kItemDefMgr);
		CONT_CASH_SHOP_ITEM::const_iterator	item_it = iter->kContCashItem.begin();
		while( item_it != iter->kContCashItem.end() )
		{
			CItemDef const *pDef = kItemDefMgr.GetDef(item_it->iItemNo);
			if( !pDef )
			{
				++item_it;
				continue;
			}

			//패키지 때문에 검사
			int const iGenderLimit = pDef->GetAbil(AT_GENDERLIMIT);
			if((iGenderLimit != UG_UNISEX) && (iGenderLimit != iGender))
			{
				++item_it;
				continue;
			}

			bool bCanEquip = true;
			bool bStyleItem = false;
			if( pDef->CanEquip() )
			{
				bCanEquip = IS_CLASS_LIMIT(pDef->GetAbil64(AT_CLASSLIMIT), pPlayer->GetAbil(AT_CLASS));
			}
			else
			{
				int const iCustomType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
				bStyleItem = iCustomType == UICT_STYLEITEM;
			}

			if( bCanEquip )
			{
				if( bSetterIsCall )
				{
					SetPreviewEquipItem(item_it->iItemNo);
				}
				SPreviewItemInfo	Info;
				Info.dwItemNo = item_it->iItemNo;
				Info.iArticleNo = iter->iIdx;
				Info.iEquipPos = pDef->EquipPos();
				Info.bStyle = bStyleItem;
				UIItemCont.push_back(Info);				
			}
			++item_it;
		}
		++iter;
	}
}

bool PgCashShop::ReSetPreviewEquip(int const iArticleNo, RECV_UI_EQUIP_PREVIEW_CONT& UIItemCont)
{
	EQUIP_PREVIEW_CONT::iterator article_itor = m_kPreviewEqItem.begin();
	while( article_itor != m_kPreviewEqItem.end() )
	{
		EQUIP_PREVIEW_CONT::value_type const& kArticle = (*article_itor);

		if( kArticle.iIdx == iArticleNo )
		{//아이템이 있고
			GET_DEF(CItemDefMgr, kItemDefMgr);
			CONT_CASH_SHOP_ITEM::const_iterator item_itor = kArticle.kContCashItem.begin();
			while( item_itor != kArticle.kContCashItem.end() )
			{//내부에도 아이템이 있고
				CONT_CASH_SHOP_ITEM::value_type const& kItemInfo = (*item_itor);
				CItemDef const* pDef = kItemDefMgr.GetDef(kItemInfo.iItemNo);
				if( pDef && pDef->CanEquip() )
				{//디비에도 있고 장착 가능한거면
					DelPreviewEquip(kItemInfo.iItemNo);
				}
				else
				{
					if( IsStyleItem(pDef) )
					{					
						DelPreviewEquip(kItemInfo.iItemNo);
					}
				}
				++item_itor;
			}
			m_kPreviewEqItem.erase(article_itor);
			break;
		}
		++article_itor;
	}
	GetPreviewEquipItemList(false, UIItemCont);
	return true;
}

bool PgCashShop::SetPreviewFurniture(TBL_DEF_CASH_SHOP_ARTICLE const& kArticle)
{
	if( kArticle.kContCashItem.empty() )
	{
		return false;
	}

	CONT_CASH_SHOP_ITEM::const_iterator item_iter = kArticle.kContCashItem.begin();
	if( item_iter != kArticle.kContCashItem.end() )
	{
		CONT_CASH_SHOP_ITEM::value_type const& kItemInfo = (*item_iter);

		PgFurniture* pkFurniture = PgFurniture::GetFurnitureFromDef(kItemInfo.iItemNo);
		if( pkFurniture )
		{
			SetPreviewFurnitureModelProperty(pkFurniture);
			DetachPreviewFurnitureModelPhysics(pkFurniture);
			m_pkItemFurniture = pkFurniture;
			m_kPreviewFurniture = kArticle;
			return true;
		}
	}
	return false;
}

bool PgCashShop::SetPreviewFurnitureModelProperty(NiNode* pkNode)
{
	if( NiIsKindOf(NiGeometry, pkNode) )
	{
		SetPreviewFurnitureModelPropertyReal(pkNode);
	}

	unsigned int uiCount = pkNode->GetArrayCount();
	for(unsigned int i = 0; i < uiCount; ++i)
	{
		NiAVObject* pkChild = pkNode->GetAt(i);
		if( !pkChild )
		{
			continue;
		}

		if( NiIsKindOf(NiGeometry, pkChild) )
		{
			SetPreviewFurnitureModelPropertyReal((NiNode*)pkChild);
		}
		else if(NiIsKindOf(NiNode, pkChild))
		{
			SetPreviewFurnitureModelProperty((NiNode*)pkChild);
		}
	}
	return true;
}

bool PgCashShop::SetPreviewFurnitureModelPropertyReal(NiNode* pkNode)
{
	if( !pkNode )
	{
		return false;
	}

	NiMaterialProperty* pkMtProp = NiDynamicCast(NiMaterialProperty, pkNode->GetProperty(NiMaterialProperty::GetType()));
	NiVertexColorProperty* pkVtProp = NiDynamicCast(NiVertexColorProperty, pkNode->GetProperty(NiVertexColorProperty::GetType()));

	if( pkMtProp ){ pkNode->DetachProperty(pkMtProp); }
	if( pkVtProp ){ pkNode->DetachProperty(pkVtProp); }

	NiMaterialProperty *pkNewMtProp = NiNew NiMaterialProperty;
	NiVertexColorProperty *pkNewVtProp = NiNew NiVertexColorProperty;

	if( pkNewMtProp && pkNewVtProp )
	{
		pkNewMtProp->SetAmbientColor(NiColor::BLACK);
		pkNewMtProp->SetDiffuseColor(NiColor::BLACK);
		pkNewMtProp->SetEmittance(NiColor::WHITE);
		pkNewMtProp->SetAlpha(1.0f);

		pkNewVtProp->SetSourceMode(NiVertexColorProperty::SOURCE_IGNORE);
		pkNewVtProp->SetLightingMode(NiVertexColorProperty::LIGHTING_E);

		pkNode->AttachProperty(pkNewMtProp);
		pkNode->AttachProperty(pkNewVtProp);
		pkNode->UpdateProperties();
	}
	return true;
}

bool PgCashShop::DetachPreviewFurnitureModelPhysics(NiNode* pNode)
{
	if( NULL == pNode )
	{
		return false;
	}

	NiFixedString kStr = pNode->GetName();
	if( true == kStr.EqualsNoCase("physx") )
	{
		return true;
	}

	unsigned int uiCount = pNode->GetArrayCount();
	for(unsigned int i = 0; i < uiCount; ++i)
	{
		NiAVObject* pkChild = pNode->GetAt(i);
		if( !pkChild )
		{
			continue;
		}

		if(NiIsKindOf(NiNode, pkChild))
		{
			if( DetachPreviewFurnitureModelPhysics((NiNode*)pkChild) )
			{
				NiAVObjectPtr pDelNode = pNode->DetachChildAt(i);
				pDelNode = NULL;
				pNode->Update(0.0f);
			}
		}
	}
	return false;
}

void PgCashShop::ClearPreviewFurniture()
{
	if( !m_pkItemFurniture )
	{
		PgFurniture::DeleteFurniture(m_pkItemFurniture);
		m_pkItemFurniture = NULL;
	}
	m_kPreviewFurniture = TBL_DEF_CASH_SHOP_ARTICLE();
}

PgActor* PgCashShop::GetPreviewActor() const
{ 
	PgUIModel* pkModel = g_kUIScene.FindUIModel(m_kActorModel.kPgUIModelID);
	if( pkModel )
	{
		return pkModel->GetPgActor(m_kActorModel.kObjectID);
	}
	return NULL;
}

void PgCashShop::ClearPreviewEquip()
{
	PgActor* pkActor = GetPreviewActor();
	if( !pkActor )
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);

	EQUIP_PREVIEW_CONT::iterator	iter = m_kPreviewEqItem.begin();
	while( iter != m_kPreviewEqItem.end() )
	{
		CONT_CASH_SHOP_ITEM::iterator	item_it = iter->kContCashItem.begin();
		while( item_it != iter->kContCashItem.end() )
		{
			DelPreviewEquip(item_it->iItemNo);
			++item_it;
		}
		iter = m_kPreviewEqItem.erase(iter);
	}

	m_kPreviewEqItem.clear();
	PREV_EQUIP_ITEM_CONT::iterator style_iter = m_kStyleItemCont.begin();
	while( style_iter != m_kStyleItemCont.end() )
	{
		SetPreviewEquipItem(style_iter->second);
		++style_iter;
	}
}

int PgCashShop::SetPreviewEquipItem(int const iItemNo) const
{
	PgActor* pkActor = GetPreviewActor();
	if( !pkActor )
	{
		return -2;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(iItemNo);
	if(pDef && pDef->CanEquip())
	{
		PREV_EQUIP_ITEM_CONT::const_iterator	iter = m_kDefaultItemCont.find(pDef->EquipPos());
		if( iter != m_kDefaultItemCont.end() )
		{
			pkActor->UnequipItem(IT_FIT_CASH, (EEquipPos)pDef->EquipPos(), pDef->No() );
		}
		pkActor->AddEquipItem( iItemNo, false, PgItemEx::LOAD_TYPE_INSTANT );
	}
	else if( pDef )
	{
		int const iCustomType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
		if( iCustomType == UICT_STYLEITEM )
		{
			int const iCustomValue = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);
			switch( iCustomValue )
			{
			case 0:
				{
					pkActor->SetItemColor(EQUIP_LIMIT_HAIR, iItemNo);
				}break;
			case 1:
			case 2:
				{
					pkActor->EquipItemProc(iItemNo,true,NULL,PgItemEx::LOAD_TYPE_INSTANT);
				}break;
			}
		}
	}

	return true;
}

E_CASHSHOP_RESULT PgCashShop::AddNewArticleToBasket(int const iArticleIDX)
{
	TBL_DEF_CASH_SHOP_ARTICLE kArticle;
	if( !FindPageArticle(iArticleIDX, kArticle) )
	{
		if( !GetItem(iArticleIDX, kArticle) )
		{
			return ECSR_NOT_EXIST_ARTICLE_IDX;
		}
	}

	if( !PgCashShop::ArticleCostDeleteToOneSave(kArticle.kContCashItemPrice) )
	{
		return ECSR_NOT_EXIST_ARTICLE_IDX;
	}


	CONT_SHOPPING_BASKET::iterator iter = m_kShoppingBasket.begin();
	while( iter != m_kShoppingBasket.end() )
	{
		if( 0 == (*iter).iIdx )
		{
			(*iter) = kArticle;
			return ECSR_OK;
		}
		++iter;
	}

	m_kShoppingBasket.push_back( kArticle );
	return ECSR_OK;
}

void PgCashShop::DelArticleToBasket(int const iSlotIDX, int const iArticleIDX)
{
	if( m_kShoppingBasket.empty() )
	{
		return;
	}

	CONT_SHOPPING_BASKET::iterator iter = m_kShoppingBasket.begin();
	for( int i = 0; i < iSlotIDX; ++i )
	{
		if( iter != m_kShoppingBasket.end() )
		{
			++iter;
		}
		else
		{
			return;
		}
	}

	if( iter != m_kShoppingBasket.end() && (*iter).iIdx == iArticleIDX )
	{
		(*iter).iIdx = 0;
	}
}

bool PgCashShop::GetShoppingBasket(CONT_SHOPPING_BASKET& kBasket)
{
	kBasket.insert(kBasket.end(), m_kShoppingBasket.begin(), m_kShoppingBasket.end());
	return (!kBasket.empty());
}

bool PgCashShop::GetShoppingBasket(CONT_SHOPPING_BASKET_SLOT_IDX& kIDX, EQUIP_PREVIEW_CONT& kItems)
{
	CONT_SHOPPING_BASKET::iterator iter = m_kShoppingBasket.begin();
	int i = 0;
	while( iter != m_kShoppingBasket.end() )
	{
		if( (*iter).iIdx != 0 )
		{
			kIDX.push_back(i);
			kItems.push_back((*iter));
		}
		++i;
		++iter;
	}
	return (!kItems.empty());
}

void PgCashShop::UpDateShoppingBasketItemCost(int const iIdx, int const iArticleNo, TBL_DEF_CASH_SHOP_ITEM_PRICE const& kCost)
{
	CONT_SHOPPING_BASKET::iterator iter = m_kShoppingBasket.begin();
	for( int i = 0; i < iIdx; ++i )
	{
		if( iter != m_kShoppingBasket.end() )
		{
			++iter;
		}
		else
		{
			return;
		}
	}

	if( iter != m_kShoppingBasket.end() )
	{
		TBL_DEF_CASH_SHOP_ITEM_PRICE_KEY	Key(kCost.bTimeType, kCost.iItemUseTime);
		iter->kContCashItemPrice.clear();
		std::pair<CONT_CASH_SHOP_ITEM_PRICE::iterator, bool> Rst = iter->kContCashItemPrice.insert(std::make_pair(Key, kCost));
		if( !Rst.second )
		{//error
			assert(0);
		}
	}
}

bool PgCashShop::ChangeShoppingBasketSlotIDXToArticleIDX(int const iSlotIDX, TBL_DEF_CASH_SHOP_ARTICLE &rkArticle)
{
	if( m_kShoppingBasket.empty() )
	{
		return false;
	}

	CONT_SHOPPING_BASKET::iterator iter = m_kShoppingBasket.begin();
	for( int i = 0; i < iSlotIDX; ++i )
	{
		if( iter != m_kShoppingBasket.end() )
		{
			++iter;
		}
		else
		{
			return false;
		}
	}

	if( iter == m_kShoppingBasket.end() )
	{
		return false;
	}

	rkArticle = (*iter);
	return ((*iter).iIdx != 0);
}

void PgCashShop::ClearShoppingBasket()
{
	m_kShoppingBasket.clear();
}

int PgCashShop::ShoppingBasketSize()
{
	int iCount = 0;
	CONT_SHOPPING_BASKET::iterator iter = m_kShoppingBasket.begin();
	while(iter != m_kShoppingBasket.end())
	{
		if(iter->iIdx != 0)
		{
			++iCount;
		}
		++iter;
	}
	return iCount;
}

void PgCashShop::UpdateValidShoppingBasketList()
{
	CONT_SHOPPING_BASKET::iterator iter = m_kShoppingBasket.begin();
	while(iter != m_kShoppingBasket.end())
	{
		TBL_DEF_CASH_SHOP_ARTICLE &kItem = (*iter);
		TBL_DEF_CASH_SHOP_ARTICLE kArticle;
		if( GetItem(kItem.iIdx, kArticle) )
		{
			if( !(kArticle.IsComprise(kItem)) )
			{
				kItem.iIdx = 0;
			}
		}
		else
		{
			kItem.iIdx = 0;
		}
		++iter;
	}
}

int PgCashShop::GetItemTypeToArticleNo(int const iItemType, int const SubKey)
{
	int const iItemNo = GetItemTypeToItemNo(iItemType, SubKey);
	if( 0 != iItemNo )
	{
		CONT_DEF_CASH_SHOP_ARTICLE::const_iterator art_it = m_kDefCashShopArticle.begin();
		while( art_it != m_kDefCashShopArticle.end() )
		{
			CONT_DEF_CASH_SHOP_ARTICLE::key_type const& kKey = art_it->first;
			CONT_DEF_CASH_SHOP_ARTICLE::mapped_type const& kArticle = art_it->second;

			if( kArticle.bSaleType != ECST_PACKEGE && kArticle.kContCashItem.size() == 1 
				&& kArticle.bState == 0)
			{
				if( kArticle.kContCashItem.begin()->iItemNo == iItemNo )
				{
					return kKey;
				}
			}
			++art_it;
		}				
	}
	return 0;
}

int PgCashShop::GetItemTypeToItemNo(int const iItemType, int const SubKey)
{
	if( ECSIT_BEGIN <= iItemType && ECSIT_END > iItemType )
	{
		if( !m_kItemTypeToItemNo.empty() && m_kItemTypeToItemNo.size() > iItemType )
		{
			CONT_CS_SUB_TYPE_TO_ITEMNO::iterator conv_itor = m_kItemTypeToItemNo[iItemType].kCont.find(SubKey);
			if( conv_itor != m_kItemTypeToItemNo[iItemType].kCont.end() )
			{
				CONT_CS_SUB_TYPE_TO_ITEMNO::mapped_type const& kItemNo = conv_itor->second;
				return kItemNo;
			}
		}
	}
	return 0;
}

int PgCashShop::GetItemTypeToSubKey(int const iItemType, int const CustomValue)
{
	if( !m_kItemTypeToItemNo.empty() && m_kItemTypeToItemNo.size() == ECSIT_END )
	{
		switch( iItemType )
		{
		case ECSIT_RESURRECTION:
		case ECSIT_SKILL_RESET:
			{
				GET_DEF(CItemDefMgr, kItemDefMgr);

				CONT_CS_SUB_TYPE_TO_ITEMNO::const_iterator item_itor = m_kItemTypeToItemNo[iItemType].kCont.begin();
				while( item_itor != m_kItemTypeToItemNo[iItemType].kCont.end() )
				{
					CONT_CS_SUB_TYPE_TO_ITEMNO::mapped_type const& kCS_ItemNo = item_itor->second;
					CItemDef const* pkDef = kItemDefMgr.GetDef( kCS_ItemNo );
					if( pkDef )
					{
						int LowLv = 0;
						int MaxLv = 0;
						switch( iItemType )
						{
						case ECSIT_RESURRECTION:
							{
								LowLv = 1;
								MaxLv = 1;
							}break;
						case ECSIT_SKILL_RESET:
							{
								LowLv = 1;
								MaxLv = 1;
							}break;
						}
						if( CustomValue >= LowLv && CustomValue <= MaxLv )
						{
							return item_itor->first;
						}
					}
					++item_itor;
				}
			}break;
		default:
			{
				return CustomValue;
			}break;
		};
	}
	return -1;
}

int PgCashShop::GetItemTypeToConfirmTTID(int const iItemType)
{
	if( ECSIT_BEGIN <= iItemType && ECSIT_END > iItemType )
	{
		if( !m_kItemTypeToItemNo.empty() && m_kItemTypeToItemNo.size() > iItemType )
		{
			return m_kItemTypeToItemNo[ iItemType ].iTTW;
		}
	}
	return 0;
}

void PgCashShop::DelPreviewEquip(int const iItemNo)
{
	PgActor* pkActor = GetPreviewActor();
	if( !pkActor )
	{
		return;
	}

	GET_DEF(CItemDefMgr, kItemDefMgr);
	CItemDef const *pDef = kItemDefMgr.GetDef(iItemNo);
	if( pDef && pDef->CanEquip() )
	{
		pkActor->UnequipItem(IT_FIT_CASH, (EEquipPos)pDef->EquipPos(), pDef->No());

		PREV_EQUIP_ITEM_CONT::iterator	iter = m_kDefaultItemCont.find(pDef->EquipPos());
		if( iter != m_kDefaultItemCont.end() )
		{
			SetPreviewEquipItem(iter->second);
		}
	}
	else if( pDef )
	{
		int const iCustomType = pDef->GetAbil(AT_USE_ITEM_CUSTOM_TYPE);
		if( iCustomType == UICT_STYLEITEM )
		{
			int const iCustomValue = pDef->GetAbil(AT_USE_ITEM_CUSTOM_VALUE_1);

			PREV_EQUIP_ITEM_CONT::iterator style_iter = m_kStyleItemCont.find(iCustomValue);
			if( style_iter != m_kStyleItemCont.end() )
			{
				SetPreviewEquipItem(style_iter->second);
			}
		}
	}
}

bool PgCashShop::InitPreviewActor()
{
	PgActor* pkActor = GetPreviewActor();
	if( !pkActor ){ return false; }

	PgPilot* pPilot = pkActor->GetPilot();
	if( !pPilot )
	{
		return false;
	}

	CUnit* pUnit = pPilot->GetUnit();
	if( !pUnit )
	{
		return false;
	}

	PgInventory* pkInv = pUnit->GetInven();
	if( !pkInv )
	{
		return false;
	}

	m_kDefaultItemCont.clear();
	m_kStyleItemCont.clear();

	for( int i = EQUIP_POS_NONE; i < EQUIP_POS_MAX; ++i )
	{
		switch( i )
		{
		case EQUIP_POS_HAIR:
		case EQUIP_POS_HAIR_COLOR:
		case EQUIP_POS_FACE:
			{
				DWORD iItemNo = pkActor->GetDefaultItem(static_cast<eEquipLimit>(0x00000001 << i));
				if( iItemNo != 0 )
				{
					std::pair<PREV_EQUIP_ITEM_CONT::iterator, bool> Rst = m_kStyleItemCont.insert(std::make_pair(i, iItemNo));
					if( !Rst.second )
					{
						return false;
					}
				}
			}break;
		case EQUIP_POS_GLASS:
		case EQUIP_POS_WEAPON:
		case EQUIP_POS_SHEILD:
		case EQUIP_POS_SHOULDER:
		case EQUIP_POS_HELMET:
		case EQUIP_POS_SHIRTS:
		case EQUIP_POS_PANTS:
		case EQUIP_POS_BOOTS:
		case EQUIP_POS_GLOVE:
		case EQUIP_POS_CLOAK:
		case EQUIP_POS_MEDAL:
		case EQUIP_POS_KICKBALL:
			{
				PgBase_Item	kItem;
				if( pkInv->GetItem(IT_FIT_CASH, i, kItem) == S_OK )
				{
					std::pair<PREV_EQUIP_ITEM_CONT::iterator, bool> Rst = m_kDefaultItemCont.insert(std::make_pair(i, kItem.ItemNo()));
					if( !Rst.second )
					{
						return false;
					}
				}
			}break;
		}
	}
	return true;
}

void PgCashShop::GetPreviewItems(EQUIP_PREVIEW_CONT& kItems) const
{
	kItems.insert(kItems.end(), m_kPreviewEqItem.begin(), m_kPreviewEqItem.end());
}

void PgCashShop::UpDatePreiewItemCost(int const iArticleNo, TBL_DEF_CASH_SHOP_ITEM_PRICE const& kCost)
{
	EQUIP_PREVIEW_CONT::iterator	iter = m_kPreviewEqItem.begin();
	while( iter != m_kPreviewEqItem.end() )
	{
		if( iter->iIdx == iArticleNo )
		{
			TBL_DEF_CASH_SHOP_ITEM_PRICE_KEY	Key(kCost.bTimeType, kCost.iItemUseTime);
			iter->kContCashItemPrice.clear();
			std::pair<CONT_CASH_SHOP_ITEM_PRICE::iterator, bool> Rst = iter->kContCashItemPrice.insert(std::make_pair(Key, kCost));
			if( !Rst.second )
			{//error
				assert(0);
			}
			break;
		}
		++iter;
	}
}

__int64 PgCashShop::GetPreviewItemTotalCost()
{// 모두 구매의 총 비용을 계산하고, 지급할 마일리지 값도 계산해서 기억한다
	__int64 i64TotalCost = 0;
	__int64 i64BuyCash = 0;
	__int64 i64AccMileage = 0;
	EQUIP_PREVIEW_CONT::const_iterator	c_iter = m_kPreviewEqItem.begin();
	while( c_iter != m_kPreviewEqItem.end() )
	{
		CONT_CASH_SHOP_ITEM_PRICE::value_type	Type = (*c_iter->kContCashItemPrice.begin());
		i64BuyCash = (Type.second.IsDiscount)?(Type.second.iDiscountCash):(Type.second.iCash);
		i64TotalCost += i64BuyCash;
		int iMileage = c_iter->iMileage;
		i64AccMileage += CalcAddBonus(i64BuyCash, iMileage);
		++c_iter;
	}
	__int64 i64TotalBonusMileage = CalcLocalAddBonus(i64TotalCost);
	SetRememberItemMileage(i64AccMileage+i64TotalBonusMileage);
	return i64TotalCost;
}

__int64 const PgCashShop::GetBasketItemTotalCost()
{
	__int64 i64TotalCost = 0;
	__int64 i64BuyCash = 0;
	__int64 i64AccMileage = 0;
	CONT_SHOPPING_BASKET::const_iterator	c_iter = m_kShoppingBasket.begin();
	while( c_iter != m_kShoppingBasket.end() )
	{
		if( (*c_iter).iIdx != 0 )
		{
			CONT_CASH_SHOP_ITEM_PRICE::value_type	Type = (*c_iter->kContCashItemPrice.begin());
			i64BuyCash = (Type.second.IsDiscount)?(Type.second.iDiscountCash):(Type.second.iCash);
			i64TotalCost += i64BuyCash;
			int iMileage = c_iter->iMileage;
			i64AccMileage += CalcAddBonus(i64BuyCash, iMileage);
		}
		++c_iter;
	}
	__int64 i64TotalBonusMileage = CalcLocalAddBonus(i64TotalCost);
	SetRememberItemMileage(i64AccMileage+i64TotalBonusMileage);
	return i64TotalCost;
}

bool PgCashShop::GetItem(int const iIndex, TBL_DEF_CASH_SHOP_ARTICLE& rkArticle)
{
	CONT_DEF_CASH_SHOP_ARTICLE::const_iterator art_it = m_kDefCashShopArticle.find(iIndex);
	if(m_kDefCashShopArticle.end() != art_it)
	{
		rkArticle = (*art_it).second;
		return true;
	}

	return false;
}

bool PgCashShop::GetLimitedInfo(CONT_CASH_SHOP_ITEM_LIMITSELL& rContLimitedOut) const
{
	if(m_kDefCashItemLimitSell.empty()) return false;
	rContLimitedOut = m_kDefCashItemLimitSell;

	return true;
}


void PgCashShop::OnBuild(BM::GUID const &kValueKey)
{
	// 캐시샵 내용을 다시 구성할때 초기화 한다
	Init();

	m_kValueKey = kValueKey;//
	
	// 먼저 캐쉬샵 카테고리 형태와
	TABLE_LOCK(CONT_DEF_CASH_SHOP) kObjLock;
	g_kTblDataMgr.GetContDef(kObjLock);
	m_kDefCashShop = *kObjLock.Get();
	
	// 캐쉬샵에 들어갈 아이템(아티클)들 정보를 얻어온다
	TABLE_LOCK(CONT_DEF_CASH_SHOP_ARTICLE) kObjLock2;
	g_kTblDataMgr.GetContDef(kObjLock2);
	m_kDefCashShopArticle = *kObjLock2.Get();

	// 캐쉬샵에 들어갈 아이템(한정판매)
	TABLE_LOCK(CONT_CASH_SHOP_ITEM_LIMITSELL) kObjLock3;
	g_kTblDataMgr.GetContDef(kObjLock3);
	m_kDefCashItemLimitSell = *kObjLock3.Get();

	// 캐쉬샵과, 마일리지 샵에 들어갈 카테고리 정보를 복사 해야 하기위해 	
	CONT_DEF_CASH_SHOP kTempShop = m_kDefCashShop;		// 캐쉬샵에 들어갈 아이템 정보 저장소
	CONT_DEF_CASH_SHOP kTempMileageShop = m_kDefCashShop; // 마일리지샵에 들어갈 아이템 정보 저장소

	std::wstring const pattern(L"\r");
	std::wstring const replace(L"");

	for(CONT_DEF_CASH_SHOP_ARTICLE::iterator aiter = m_kDefCashShopArticle.begin();aiter != m_kDefCashShopArticle.end();++aiter)
	{// 아이템들(아티클들) 리스트에서
		if( 1 == aiter->second.bState )
		{
			continue;
		}
		/*
		SYSTEMTIME kTempTime;
		if( !g_kEventView.GetLocalTime(&kTempTime) )
		{
			continue;
		}

		BM::PgPackedTime kNowTime(kTempTime);
		if( !aiter->second.kSaleStartDate.IsNull() )
		{
			if( kNowTime < aiter->second.kSaleStartDate )
			{
				continue;
			}
		}

		if( !aiter->second.kSaleEndDate.IsNull() )
		{
			if( aiter->second.kSaleEndDate < kNowTime )
			{
				continue;
			}
		}
		*/

		//"\r\n"이 들어가면 음표나 네모(폰트없어서)가 출력되는 현상을 막기위해, "\r"제거
		CONT_DEF_CASH_SHOP_ARTICLE::mapped_type & kArticle = aiter->second;

		std::wstring::size_type pos = 0;
		std::wstring::size_type offset = 0;
		
		while((pos = kArticle.wstrDiscription.find(pattern, offset)) != std::wstring::npos)   
		{
			kArticle.wstrDiscription.replace(kArticle.wstrDiscription.begin() + pos, kArticle.wstrDiscription.begin() + pos + pattern.size(), replace);
			offset = pos + replace.size();
		}

		CONT_CASH_SHOP_ITEM_PRICE const& kContItemPrice = (*aiter).second.kContCashItemPrice;
		CONT_CASH_SHOP_ITEM_PRICE::const_iterator itor = kContItemPrice.begin();
		CONT_CASH_SHOP_ITEM_PRICE::const_iterator itor_first = kContItemPrice.begin();
		bool bMileageType = true;
		bool bSutable = true;
		while(itor != kContItemPrice.end())
		{// 마일리지 아이템들을 구별해 
			TBL_DEF_CASH_SHOP_ITEM_PRICE const& kItemPrice = itor->second;			
			TBL_DEF_CASH_SHOP_ITEM_PRICE const& kFirstItemPrice = itor_first->second;
			if(kFirstItemPrice.IsOnlyMileage != kItemPrice.IsOnlyMileage)
			{// 같은 idx를 가진 아이템이 마일리지 몰과, 캐시샵에 동시에 존재 하면 error라고 알려주고
				bSutable = false;				
				TBL_DEF_CASH_SHOP_ARTICLE const& kTempArticle = aiter->second;				
				_PgMessageBox("PgCashShop::OnBuild", "Item exist MileageMall and CashShop.\n check DB\nitem idx:%d name:%s", kTempArticle.iIdx, MB(kTempArticle.wstrName));
				break;
			}
			if(!kItemPrice.IsOnlyMileage)
			{
				bMileageType = false;
			}
			else
			{// 마일리지 아이템에
				if(0 < itor->first.iUseTime)
				{// 기간제 아이템을 등록 하지 않으려면 아래 주석을 푼다.
					// 마일리지 몰의 기간제 아이템은 절대로 캐시샵에 없어야 한다!!!
					//bSutable = false;
					//TBL_DEF_CASH_SHOP_ARTICLE const& kTempArticle = aiter->second;
					//_PgMessageBox("PgCashShop::OnBuild", "term Item can't exist MileageMall.\ncheck DB\nitem idx:%d name:%s", kTempArticle.iIdx, MB(kTempArticle.wstrName));
				}
			}
			++itor;
		}
		if(bSutable)
		{// 등록해도 되는 아이템은
			if(true == bMileageType)
			{// 마일리지 카테고리와
				CONT_DEF_CASH_SHOP::iterator finditer = kTempMileageShop.find((*aiter).second.iCategoryIdx);
				if(finditer != kTempMileageShop.end())
				{
					(*finditer).second.kContArticle.insert(std::make_pair((*aiter).first,(*aiter).second));
				}
			}
			else
			{// 캐쉬샵 카테고리에 넣는다
				CONT_DEF_CASH_SHOP::iterator finditer = kTempShop.find((*aiter).second.iCategoryIdx);
				if(finditer != kTempShop.end())
				{
					(*finditer).second.kContArticle.insert(std::make_pair((*aiter).first,(*aiter).second));
				}
			}
		}
	}
	// 카테고리 별로 아이템들을 정리 한다
	SortShopArticlesByCategory(kTempShop, m_kCashShopData);
	SortShopArticlesByCategory(kTempMileageShop, m_kMileageShopData);
}

void PgCashShop::ParseXml()
{
	std::string const XMLPATH = "CSTypeToItemNo.xml";
	TiXmlDocument kXmlDoc(XMLPATH.c_str());
	if(!PgXmlLoader::LoadFile(kXmlDoc, UNI(XMLPATH)))
	{
		return;
	}

	m_kItemTypeToItemNo.clear();
	m_kItemTypeToItemNo.resize(ECSIT_END);

	TiXmlElement const* pkElement = kXmlDoc.FirstChildElement();
	while( pkElement )
	{
		char const* pcTagName = pkElement->Value();

		if( strcmp(pcTagName, "CS_TYPE_TO_ITEMNO") == 0 )
		{
			TiXmlElement const* pkTypeEle = pkElement->FirstChildElement();
			
			while( pkTypeEle )
			{
				pcTagName = pkTypeEle->Value();

				if( strcmp(pcTagName, "TYPE") == 0 )
				{
					E_CASHSHOP_ITEM_TYPE_TO_VALUE kItemList;
					int iItemType = 0;

					{
						TiXmlAttribute const* pkAttr = pkTypeEle->FirstAttribute();
						while( pkAttr )
						{
							char const* pcAttrName = pkAttr->Name();
							char const* pcAttrValue = pkAttr->Value();

							if( strcmp(pcAttrName, "KEY") == 0 )
							{
								iItemType = atoi(pcAttrValue);
							}
							else if( strcmp(pcAttrName, "TTW") == 0 )
							{
								kItemList.iTTW = atoi(pcAttrValue);
							}
							pkAttr = pkAttr->Next();
						}
					}

					TiXmlElement const* pkItemEle = pkTypeEle->FirstChildElement();
					while( pkItemEle )
					{
						pcTagName = pkItemEle->Value();

						if( strcmp(pcTagName, "ITEM") == 0 )
						{
							int iItemNo = 0;
							int KeyNo = 0;

							TiXmlAttribute const* pkAttr = pkItemEle->FirstAttribute();
							while( pkAttr )
							{
								char const* pcAttrName = pkAttr->Name();
								char const* pcAttrValue = pkAttr->Value();

								if( strcmp(pcAttrName, "KEY") == 0 )
								{
									KeyNo = atoi(pcAttrValue);
								}
								else if( strcmp(pcAttrName, "ITEM_NO") == 0 )
								{
									iItemNo = atoi(pcAttrValue);
								}
								pkAttr = pkAttr->Next();
							}

							kItemList.kCont.insert( std::make_pair(KeyNo, iItemNo) );
						}
						pkItemEle = pkItemEle->NextSiblingElement();
					}
					m_kItemTypeToItemNo[iItemType].iTTW = kItemList.iTTW;
					m_kItemTypeToItemNo[iItemType].kCont.swap( kItemList.kCont );
				}
				pkTypeEle = pkTypeEle->NextSiblingElement();
			}
		}
		pkElement = pkElement->NextSiblingElement();
	}
}

void PgCashShop::SortShopArticlesByCategory(CONT_DEF_CASH_SHOP& kContShop, CONT_DEF_CASH_SHOP_VEC& kContShopData_out)
{
	if(kContShop.empty())
	{
		return;
	}
	
	for(int i = CSCT_SMALL;i > CSCT_LARGE;i--)
	{
		for(CONT_DEF_CASH_SHOP::const_iterator siter = m_kDefCashShop.begin();siter != m_kDefCashShop.end();++siter)
		{
			if(i != (*siter).second.bStep)
			{
				continue;
			}

			switch((*siter).second.bStep)
			{
			case CSCT_LARGE:
				{
					continue;
				}break;
			case CSCT_MIDDLE:
				{
					CONT_DEF_CASH_SHOP::iterator finditem = kContShop.find((*siter).first);
					if(finditem != kContShop.end())
					{// 아이템을 해당 카테고리에 넣어 정리 한다
						TBL_DEF_CASH_SHOP_CATEGORY const& kShopCategory = (*siter).second;						
						CONT_DEF_CASH_SHOP::iterator findcont = kContShop.find(kShopCategory.iRootIdx);
						if(findcont != kContShop.end())
						{
							TBL_DEF_CASH_SHOP_CATEGORY& kCategory = (*findcont).second;
							CONT_DEF_CASH_SHOP& kSubCategory = kCategory.kSubCategory;
							kSubCategory.insert(std::make_pair((*finditem).first, (*finditem).second));
							kContShop.erase(finditem);
						}
					}
				}break;
			case CSCT_SMALL:
				{
					CONT_DEF_CASH_SHOP::iterator finditem = kContShop.find((*siter).first);
					if(finditem != kContShop.end())
					{
						CONT_DEF_CASH_SHOP::iterator findcont = kContShop.find((*siter).second.iParentIdx);
						if(findcont != kContShop.end())
						{
							(*findcont).second.kSubCategory.insert(std::make_pair((*finditem).first,(*finditem).second));
							kContShop.erase(finditem);
						}
					}
				}break;
			}
		}
	}

	CONT_DEF_CASH_SHOP tempShop;
	CONT_DEF_CASH_SHOP::const_iterator shop_it = kContShop.begin();
	int i = 0;	//카테고리가 0부터 시작해서 1씩 증가한다는 가정하게
	while(kContShop.end() != shop_it)
	{
		tempShop.insert(std::make_pair((*shop_it).second.bRank, (*shop_it).second));	//랭크로 정렬
		++shop_it;
	}	

	CONT_DEF_CASH_SHOP::iterator temp_it = tempShop.begin();
	while(tempShop.end() != temp_it)
	{
		TBL_DEF_CASH_SHOP_CATEGORY& rkCate = (*temp_it).second;
		if (!rkCate.kContArticle.empty())	//최상위 카테고리에 아이템이 있으면
		{
			CONT_DEF_CASH_SHOP_ARTICLE kTempAricle;
			CONT_DEF_CASH_SHOP_ARTICLE::iterator art_it = rkCate.kContArticle.begin();
			while(rkCate.kContArticle.end() != art_it)	//랭크로 정렬
			{
				kTempAricle.insert(std::make_pair((*art_it).second.wDisplayRank, (*art_it).second));
				++art_it;
			}
			rkCate.kContArticle = kTempAricle; //정렬된 맵으로 교체

		}

		CONT_DEF_CASH_SHOP::iterator sub_it = rkCate.kSubCategory.begin();
		while(rkCate.kSubCategory.end() != sub_it)	//중분류 카테고리를 검색
		{
			CONT_DEF_CASH_SHOP_ARTICLE kTempAricle;
			CONT_DEF_CASH_SHOP_ARTICLE::iterator art_it = (*sub_it).second.kContArticle.begin();
			while((*sub_it).second.kContArticle.end() != art_it)	//랭크로 정렬
			{
				kTempAricle.insert(std::make_pair((*art_it).second.wDisplayRank, (*art_it).second));
				++art_it;
			}
			(*sub_it).second.kContArticle = kTempAricle; //정렬된 맵으로 교체

			++sub_it;
		}

		kContShopData_out.push_back((*temp_it).second);	//랭크로 정렬된 데이터를 저장
		++temp_it;
	}
}

bool PgCashShop::GetCategory(int const iCategory, TBL_DEF_CASH_SHOP_CATEGORY &rkCate)
{
	CONT_DEF_CASH_SHOP_VEC const* kContShop = NULL;
	switch(m_eShopType)
	{
	case ECASH_SHOP:
		{
			kContShop = &m_kCashShopData;
		}break;
	case EMILEAGE_SHOP:
		{
			kContShop = &m_kMileageShopData;
		}break;
	default:
		{
			return false;
		}break;
	}
	
	if ( kContShop->empty() ) {return false;}

	if ( 0>iCategory || kContShop->size() <= (size_t)iCategory )
	{
		return false;
	}

	int iMileage = m_eShopType == EMILEAGE_SHOP ? ECSCATEGORYNUM_END : 0;
	rkCate = kContShop->at(iCategory + iMileage);
	m_kRecentTopCategory = rkCate;
	m_iRecentTopCategory = iCategory + iMileage;

	return true;
}

bool PgCashShop::GetSubategory(int const iCategory, TBL_DEF_CASH_SHOP_CATEGORY &rkCate)		// 중분류 탭을 눌렀을 때
{
	if ( 0 > iCategory || m_kRecentTopCategory.kSubCategory.size() <= iCategory )
	{
		return false;
	}
	CONT_DEF_CASH_SHOP::const_iterator cate_it = m_kRecentTopCategory.kSubCategory.begin();
	for(int i = 0; i<iCategory; ++i, ++cate_it)
	{
		if( m_kRecentTopCategory.kSubCategory.end() == cate_it )
		{
			return false;
		}
	}

	rkCate = (*cate_it).second;
	m_kRecentSubCategory = rkCate;
	m_iRecentSubCategory = iCategory;

	return true;
}

bool PgCashShop::RecvCashShop_Command(WORD const wPacketType, BM::Stream &rkPacket)
{
	return true;
}


size_t PgCashShop::GetItemByDispType(EQUIP_PREVIEW_CONT& rkOutputMap, ECashItemDisplayType const eType)
{
	m_kBestItem.clear();
	CONT_DEF_CASH_SHOP_ARTICLE::const_iterator	a_iter = m_kDefCashShopArticle.begin();
	while(a_iter != m_kDefCashShopArticle.end() )
	{
		if( a_iter->second.bDisplayTypeIdx == eType )
		{
			m_kBestItem.push_back(a_iter->second);
		}
		++a_iter;
	}
	m_kBestItem.sort(PgCashShop::BestItem_Greater);
	rkOutputMap.insert(rkOutputMap.end(), m_kBestItem.begin(), m_kBestItem.end());
	return rkOutputMap.size();

	//if ( pkArticleMap->empty() )
	//{
	//	return 0;
	//}

	//rkOutputMap.clear();

	//CONT_DEF_CASH_SHOP_ARTICLE::const_iterator art_it = pkArticleMap->begin();
	//while(pkArticleMap->end() != art_it)
	//{
	//	if ( (*art_it).second.iDisplayTypeIdx == eType ) 
	//	{
	//		CONT_DEF_CASH_SHOP_ARTICLE::const_iterator art2_it = pkArticleMap->find((*art_it).second.iIdx);
	//		if ( pkArticleMap->end() != art2_it )
	//		{
	//			rkOutputMap.insert(std::make_pair((*art_it).second.iItemIdx, (*art_it).second));//등수 구분은 어떻게?
	//		}
	//		else	//있으면
	//		{
	//			//if () //비교후

	//		}
	//	}
	//	++art_it;
	//}

	//return rkOutputMap.size();
}

bool PgCashShop::SetLimitedArticle(const CONT_CASH_SHOP_ITEM_LIMITSELL& rContLimitedArticle)
{ //받아온 한정판매용 아이템 정보가 실제로 유효한지 체크
/*	CONT_DEF_CASH_SHOP_ARTICLE::const_iterator iterArticle = m_kDefCashShopArticle.end();
	CONT_CASH_SHOP_ITEM_LIMITSELL::const_iterator iterLimitedArticle;
	for(iterLimitedArticle = rContLimitedArticle.begin();
		iterLimitedArticle != rContLimitedArticle.end();
		++iterLimitedArticle)
	{
		if((iterArticle = m_kDefCashShopArticle.find( iterLimitedArticle->first.kKey )) == m_kDefCashShopArticle.end() )
		{
			return false;
		}
		if(iterArticle->second.kSaleStartDate.IsNull() || iterArticle->second.kSaleEndDate.IsNull() )
		{
			return false;
		}
	}
*/
	m_kDefCashItemLimitSell = rContLimitedArticle;
	return true;
}

void PgCashShop::AddToPageArticle(TBL_DEF_CASH_SHOP_ARTICLE const & rkArticle)
{
	m_kRecentPageArticle.insert(std::make_pair(rkArticle.iIdx, rkArticle));
}

bool PgCashShop::FindPageArticle(int const iIndex, TBL_DEF_CASH_SHOP_ARTICLE &rkArticle)
{
	CONT_DEF_CASH_SHOP_ARTICLE::iterator art_it = m_kRecentPageArticle.find(iIndex);
	if(art_it==m_kRecentPageArticle.end())
	{
		return false;
	}

	rkArticle = (*art_it).second;
	return true;
}

bool PgCashShop::FindArticle(DWORD const dwItemNo, int& Idx)
{
	CONT_DEF_CASH_SHOP_ARTICLE::const_iterator c_iter = m_kDefCashShopArticle.begin();
	while( m_kDefCashShopArticle.end() != c_iter )
	{
		if( c_iter->second.bSaleType != ECST_PACKEGE )
		{
			if( c_iter->second.kContCashItem.size() )
			{
				if( c_iter->second.kContCashItem.begin()->iItemNo == dwItemNo )
				{
					Idx = c_iter->first.kKey;
					return true;	
				}
			}
		}
		++c_iter;
	}
	return false;
}

bool PgCashShop::FindArticle(DWORD const dwItemNo, int& iIdx, TBL_DEF_CASH_SHOP_ARTICLE& rkArticle) const
{
		CONT_DEF_CASH_SHOP_ARTICLE::const_iterator c_iter = m_kDefCashShopArticle.begin();
	while( m_kDefCashShopArticle.end() != c_iter )
	{
		if( c_iter->second.bSaleType != ECST_PACKEGE )
		{
			if( c_iter->second.kContCashItem.size() )
			{
				if( c_iter->second.kContCashItem.begin()->iItemNo == dwItemNo )
				{
					iIdx = c_iter->first.kKey;
					rkArticle = c_iter->second;
					return true;
				}
			}
		}
		++c_iter;
	}
	return false;
}

bool PgCashShop::FindArticleByKey(TBL_KEY_INT kKey, TBL_DEF_CASH_SHOP_ARTICLE& rkArticleOut)
{
	CONT_DEF_CASH_SHOP_ARTICLE::const_iterator iterFound = m_kDefCashShopArticle.find(kKey);
	if(iterFound == m_kDefCashShopArticle.end()) return false;
	rkArticleOut = iterFound->second;

	return true;
}


void PgCashShop::SetRankCont(CONT_CASHRANKINFO const& rkCont)
{
	m_kRankCont.clear();
	m_kRankCont.insert(m_kRankCont.end(), rkCont.begin(), rkCont.end());
}
void PgCashShop::SetGiftCont(CONT_CASHGIFTINFO const& rkCont)
{
	m_kGiftContSend.clear();
	m_kGiftContRecv.clear();
	CONT_CASHGIFTINFO::const_iterator gift_it = rkCont.begin();
	while(rkCont.end() != gift_it)
	{
		if(0==(*gift_it).bType)
		{
			m_kGiftContSend.push_back((*gift_it));
		}
		else
		{
			m_kGiftContRecv.push_back((*gift_it));
		}
		++gift_it;
	}
}

void PgCashShop::GetGiftContRecv(CONT_CASHGIFTINFO& rkCont, int const iPage) const
{
	if(iPage*CASH_GIFT_PER_PAGE>=m_kGiftContRecv.size())
	{
		return;
	}
	CONT_CASHGIFTINFO::const_iterator gift_it = m_kGiftContRecv.begin();
	for(int i = 0; i<iPage*CASH_GIFT_PER_PAGE; ++i)
	{
		if(m_kGiftContRecv.end() == gift_it)	
		{
			break;
		}
		++gift_it;
	}

	for(int i = 0; i < CASH_GIFT_PER_PAGE; ++i)
	{
		if(m_kGiftContRecv.end() == gift_it)	
		{
			return;
		}
		rkCont.push_back((*gift_it));
		++gift_it;
	}
}

void PgCashShop::GetGiftContSend(CONT_CASHGIFTINFO& rkCont, int const iPage) const
{
	if(iPage*CASH_GIFT_PER_PAGE>=m_kGiftContSend.size())
	{
		return;
	}
	CONT_CASHGIFTINFO::const_iterator gift_it = m_kGiftContSend.begin();
	for(int i = 0; i<iPage*CASH_GIFT_PER_PAGE; ++i)
	{
		 if(m_kGiftContSend.end() == gift_it)
		 {
			 break;
		 }
		 ++gift_it;
	}

	for(int i = 0; i < CASH_GIFT_PER_PAGE; ++i)
	{
		if(m_kGiftContSend.end() == gift_it)	
		{
			return;
		}
		rkCont.push_back((*gift_it));
		++gift_it;
	}
}

void PgCashShop::DeleteGift(BM::GUID const& rkGuid)
{
	if(BM::GUID::IsNull(rkGuid))
	{
		return;
	}

	CONT_CASHGIFTINFO::iterator gift_it = m_kGiftContRecv.begin();
	while(m_kGiftContRecv.end() != gift_it)
	{
		if((*gift_it).kGiftGuId == rkGuid)
		{
			m_kGiftContRecv.erase(gift_it);
			return;
		}
		++gift_it;
	}
}

bool PgCashShop::BestItem_Greater(TBL_DEF_CASH_SHOP_ARTICLE const& kLeft, TBL_DEF_CASH_SHOP_ARTICLE const& kRight)
{
	if( kLeft.wDisplayRank < kRight.wDisplayRank )
	{
		return true;
	}
	else if( kLeft.wDisplayRank == kRight.wDisplayRank )
	{
		if( kLeft.iIdx < kRight.iIdx )
		{
			return true;
		}
	}
	return false;
}

bool PgCashShop::ArticleCostDeleteToOneSave(CONT_CASH_SHOP_ITEM_PRICE& kPrice)
{
	if( kPrice.size() > 1 )
	{
		CONT_CASH_SHOP_ITEM_PRICE::iterator price_iter = kPrice.begin();
		if( price_iter == kPrice.end() )
		{
			return false;
		}

		TBL_DEF_CASH_SHOP_ITEM_PRICE_KEY	Key = price_iter->first;
		TBL_DEF_CASH_SHOP_ITEM_PRICE	Price = price_iter->second;

		kPrice.clear();
		std::pair<CONT_CASH_SHOP_ITEM_PRICE::iterator, bool> Rst = kPrice.insert(std::make_pair(Key, Price));
		if( !Rst.second )
		{
			return false;
		}
	}
	return true;
}

void PgCashShop::SetUseMileage(bool const bUse, XUI::CXUI_Wnd* const pkWnd)
{
	if(!pkWnd)
	{
		return;
	}
//	XUI::CXUI_Wnd* const pBuyWnd = XUIMgr.Get(_T("SFRM_CASHSHOP_BUY"));
	if(!pkWnd)
	{// 물건 구매시
		m_bUseMileage = false;
	}
	XUI::CXUI_Edit* const pDpEdt = static_cast<XUI::CXUI_Edit*>( pkWnd->GetControl(_T("EDT_DP")) );
	if(!pDpEdt)
	{
		m_bUseMileage = false;
		return;
	}
	XUI::CXUI_Wnd* const pBgWnd = pkWnd->GetControl(_T("SFRM_DPNUM_BG"));
	if(!pDpEdt)
	{
		m_bUseMileage = false;
		return;
	}
	if(bUse)
	{// 마일리지 사용 체크를 키면
		pDpEdt->Visible(true);
		pBgWnd->Visible(true);
		pDpEdt->SetEditFocus(true);
	}
	else
	{// 마일리지 사용 체크를 끄면
		pDpEdt->Visible(false);
		pDpEdt->EditText(_T(""));
		pBgWnd->Visible(false);
	}
	m_bUseMileage = bUse;
}

bool PgCashShop::SetShopType(ECashShopType const eType)
{
	switch(eType)
	{
	case ECASH_SHOP: 		
	case EMILEAGE_SHOP:
	case ECASHSHOP_GACHA:
	case ECOSTUME_MIX:
		{
			m_eShopType = eType;			
		}break;
	default:
		{
			_PgMessageBox("PgCashShop::SetShopType()", "Unknown ShopType");
			return false;
		}break;
	}
	return true;
}

bool PgCashShop::SetBuyTypeUI(ECashShopType const eType)
{
	switch(eType)
	{
	case ECASH_SHOP: 		
	case EMILEAGE_SHOP:
		{
			m_eBuyTypeUI = eType;			
		}break;
	default:
		{
			_PgMessageBox("PgCashShop::SetShopType()", "Unknown ShopType");
			return false;
		}break;
	}
	return true;
}

bool PgCashShop::SetShopSubType(const ECashShopForm eFormType)
{
	if(GetShopType() != ECASH_SHOP && GetShopType() != EMILEAGE_SHOP) return false;
	m_eShopSubType = eFormType;

	return true;
}

__int64 const PgCashShop::CalcAddBonus(__int64 const i64Cost,int const iMileage) const
{// 개별 마일리지 계산	
	return static_cast<__int64>((i64Cost * iMileage)/100);
}

__int64 const PgCashShop::CalcLocalAddBonus(__int64 const i64Cost)
{// 전체 구매시 보너스 마일리지
	if( CashShopUtil::IsCanUseMileageServiceRegion() )
	{
		PgPilot const * pkPilot = g_kPilotMan.GetPlayerPilot();
		if(pkPilot)
		{
			__int64 const i64TotalUsedCash = pkPilot->GetAbil64(AT_MILEAGE_TOTAL_CASH) + i64Cost;
			__int64 const i64UsedCash_100 = i64TotalUsedCash - pkPilot->GetAbil64(AT_MILEAGE_100);
			__int64 const i64UsedCash_1000 = i64TotalUsedCash - pkPilot->GetAbil64(AT_MILEAGE_1000);
			
			int const i100_Normalizer = lua_tinker::call<int>("Milage_100Nomalizer");
			int const i100_Amp = lua_tinker::call<int>("Milage_100UnitAmp");
			int const i1000_Normalizer = lua_tinker::call<int>("Milage_1000Nomalizer");
			int const i1000_Amp = lua_tinker::call<int>("Milage_1000UnitAmp");

			__int64 const i64BonusPoint_100 = (i64UsedCash_100/i100_Normalizer) * i100_Amp;
			__int64 const i64BonusPoint_1000 = (i64UsedCash_1000/i1000_Normalizer) * i1000_Amp;

			// 대만 기준 값
			//__int64 const i64BonusPoint_100 = (i64UsedCash_100/100) * 3;
			//__int64 const i64BonusPoint_1000 = (i64UsedCash_1000/1000) * 10;
			return i64BonusPoint_100 + i64BonusPoint_1000;
		}
	}
	return 0;
}

int PgCashShop::GetItemPrice(int const iItemNo, BYTE bTimeType, int iUseTime, bool bDiscount)
{
	CONT_DEF_CASH_SHOP_ARTICLE::const_iterator itr = m_kDefCashShopArticle.begin();
	while( itr != m_kDefCashShopArticle.end() )
	{
		TBL_DEF_CASH_SHOP_ARTICLE kArticle = itr->second;
		if( itr->second.kContCashItem.size() == 1 )
		{
			if( kArticle.kContCashItem.front().iItemNo == iItemNo )
			{
				CONT_CASH_SHOP_ITEM_PRICE::iterator itr_price = kArticle.kContCashItemPrice.begin();
				while( itr_price != kArticle.kContCashItemPrice.end() )
				{
					if( (itr_price->second.bTimeType == bTimeType) && (itr_price->second.iItemUseTime == iUseTime) )
					{
						if( bDiscount )
						{
							return itr_price->second.iDiscountCash;
						}
						else
						{
							return itr_price->second.iCash;
						}
					}
					++itr_price;
				}
			}
		}

		++itr;
	}

	return 0;
}