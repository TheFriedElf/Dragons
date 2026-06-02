#include "stdafx.h"
#include "Variant/PgStringUtil.h"
#include "Variant/ClientOption.h"
#include "lwUI.h"

#include "PgOption.h"
#include "PgLocalManager.h"
#include "PgInputSystem.h"
#include "PgNetwork.h"

#include "PgWorld.h"
#include "PgPilotMan.h"
#include "PgPilot.h"
#include "PgActor.h"
#include "PgMobileSuit.H"
#include "PgMyActorViewMgr.h"
#include "PgEnergyGuage.h"

namespace PgOptionUtil
{
	SClientDWORDOption::SClientDWORDOption(DWORD const dwOption)
	{
		ReadFromDWORD(dwOption);
	}

	SClientDWORDOption::SClientDWORDOption(SClientDWORDOption const& rhs)
	{
		ReadFromDWORD( rhs.WriteToDWORD() );
	}

	void SClientDWORDOption::ReadFromDWORD(DWORD const dwOption)
	{
		*((DWORD*)this) = dwOption;
	}
	DWORD SClientDWORDOption::WriteToDWORD() const
	{
		return *((DWORD*)this);
	}

	bool SClientDWORDOption::IsHideEquipInvenPos(EEquipPos const ePos) const
	{
		switch( ePos )
		{
		//case EQUIP_POS_GLASS:		{ return HideGlass(); }break;
		//case EQUIP_POS_NECKLACE:	{ return HideNecklace(); }break;
		case EQUIP_POS_HELMET:		{ return HideHelmet(); }break;
		case EQUIP_POS_SHOULDER:	{ return HideShoulder(); }break;
		case EQUIP_POS_CLOAK:		{ return HideClock(); }break;
		case EQUIP_POS_SHIRTS:		{ return HideShirts(); }break;
		case EQUIP_POS_GLOVE:		{ return HideGlove(); }break;

		//case EQUIP_POS_BELT:			{ return HideBelt(); }break;
		case EQUIP_POS_PANTS:		{ return HidePants(); }break;
		case EQUIP_POS_BOOTS:		{ return HideBoots(); }break;
		//case EQUIP_POS_RING_L:		{ return HideRingL(); }break;
		//case EQUIP_POS_RING_R:		{ return HideRingR(); }break;
		//case EQUIP_POS_EARRING:	{ return HideEarring(); }break;
		//case EQUIP_POS_ATTSTONE:	{ return HideAttstone(); }break;

		default:
			{
				// do nothing
			}break;
		}
		return false;
	}
	void SClientDWORDOption::SetHideEquipInvenPos(EEquipPos const ePos, bool const bHide)
	{
		switch( ePos )
		{
		//case EQUIP_POS_GLASS:		{ HideGlass(bHide); }break;
		//case EQUIP_POS_NECKLACE:	{ HideNecklace(bHide); }break;
		case EQUIP_POS_HELMET:		{ HideHelmet(bHide); }break;
		case EQUIP_POS_SHOULDER:	{ HideShoulder(bHide); }break;
		case EQUIP_POS_CLOAK:		{ HideClock(bHide); }break;
		case EQUIP_POS_SHIRTS:		{ HideShirts(bHide); }break;
		case EQUIP_POS_GLOVE:		{ HideGlove(bHide); }break;

		//case EQUIP_POS_BELT:			{ HideBelt(bHide); }break;
		case EQUIP_POS_PANTS:		{ HidePants(bHide); }break;
		case EQUIP_POS_BOOTS:		{ HideBoots(bHide); }break;
		//case EQUIP_POS_RING_L:		{ HideRingL(bHide); }break;
		//case EQUIP_POS_RING_R:		{ HideRingR(bHide); }break;
		//case EQUIP_POS_EARRING:	{ HideEarring(bHide); }break;
		//case EQUIP_POS_ATTSTONE:	{ HideAttstone(bHide); }break;

		default:
			{
				// do nothing
			}break;
		}
	}
	bool SClientDWORDOption::IsHideCashInvenPos(EEquipPos const ePos) const
	{
		switch( ePos )
		{
		case EQUIP_POS_GLASS:		{ return HideCashGlass(); }break;
		case EQUIP_POS_HELMET:		{ return HideCashHelmet(); }break;
		case EQUIP_POS_SHOULDER:	{ return HideCashShoulder(); }break;
		case EQUIP_POS_CLOAK:		{ return HideCashClock(); }break;
		case EQUIP_POS_SHIRTS:		{ return HideCashShirts(); }break;
		case EQUIP_POS_GLOVE:		{ return HideCashGlove(); }break;
		case EQUIP_POS_WEAPON:	{ return HideCashWeapon(); }break;
		case EQUIP_POS_ARM:			{ return HideCashArm(); }break;
		case EQUIP_POS_BOOTS:		{ return HideCashBoots(); }break;
		case EQUIP_POS_PANTS:		{ return HideCashPants(); }break;
		case EQUIP_POS_MEDAL:		{ return HideCashBalloon(); }break;
		default:
			{
				// do nothing
			}break;
		}
		return false;
	}

	void SClientDWORDOption::SetHideCashInvenPos(EEquipPos const ePos, bool const bHide)
	{
		switch( ePos )
		{
		case EQUIP_POS_GLASS:		{ HideCashGlass(bHide); }break;
		case EQUIP_POS_HELMET:		{ HideCashHelmet(bHide); }break;
		case EQUIP_POS_SHOULDER:	{ HideCashShoulder(bHide); }break;
		case EQUIP_POS_CLOAK:		{ HideCashClock(bHide); }break;
		case EQUIP_POS_SHIRTS:		{ HideCashShirts(bHide); }break;
		case EQUIP_POS_GLOVE:		{ HideCashGlove(bHide); }break;
		case EQUIP_POS_WEAPON:	{ HideCashWeapon(bHide); }break;
		case EQUIP_POS_ARM:			{ HideCashArm(bHide); }break;
		case EQUIP_POS_BOOTS:		{ HideCashBoots(bHide); }break;
		case EQUIP_POS_PANTS:		{ HideCashPants(bHide); }break;
		case EQUIP_POS_MEDAL:		{ HideCashBalloon(bHide); }break;
		default:
			{
				// do nothing
			}break;
		}
	}

	//
	inline int BoolToInt(bool const bVal)
	{
		return (bVal)? 1: 0;
	}
	inline bool IntToBool(int const iVal)
	{
		return 0 != iVal;
	}

	std::string const STR_OPTION_CASH_EQUIP_HIDE_GLASS("OPTION_CASH_EQUIP_HIDE_GLASS");
	std::string const STR_OPTION_CASH_EQUIP_HIDE_HELMET("OPTION_CASH_EQUIP_HIDE_HELMET");
	std::string const STR_OPTION_CASH_EQUIP_HIDE_SHOULDER("OPTION_CASH_EQUIP_HIDE_SHOULDER");
	std::string const STR_OPTION_CASH_EQUIP_HIDE_CLOAK("OPTION_CASH_EQUIP_HIDE_CLOAK");
	std::string const STR_OPTION_CASH_EQUIP_HIDE_SHIRTS("OPTION_CASH_EQUIP_HIDE_SHIRTS");
	std::string const STR_OPTION_CASH_EQUIP_HIDE_GLOVE("OPTION_CASH_EQUIP_HIDE_GLOVE");
	std::string const STR_OPTION_CASH_EQUIP_HIDE_WEAPON("OPTION_CASH_EQUIP_HIDE_WEAPON");
	std::string const STR_OPTION_CASH_EQUIP_HIDE_ARM("OPTION_CASH_EQUIP_HIDE_ARM");
	std::string const STR_OPTION_CASH_EQUIP_HIDE_BOOTS("OPTION_CASH_EQUIP_HIDE_BOOTS");
	std::string const STR_OPTION_CASH_EQUIP_HIDE_PANTS("OPTION_CASH_EQUIP_HIDE_PANTS");
	std::string const STR_OPTION_CASH_EQUIP_HIDE_BALLOON("OPTION_CASH_EQUIP_HIDE_BALLOON");

	//std::string const STR_OPTION_EQUIP_HIDE_GLASS("OPTION_EQUIP_HIDE_GLASS");
	//std::string const STR_OPTION_EQUIP_HIDE_NECKLACE("OPTION_EQUIP_HIDE_NECKLACE");
	std::string const STR_OPTION_EQUIP_HIDE_HELMET("OPTION_EQUIP_HIDE_HELMET");
	std::string const STR_OPTION_EQUIP_HIDE_SHOULDER("OPTION_EQUIP_HIDE_SHOULDER");
	std::string const STR_OPTION_EQUIP_HIDE_CLOAK("OPTION_EQUIP_HIDE_CLOAK");
	std::string const STR_OPTION_EQUIP_HIDE_SHIRTS("OPTION_EQUIP_HIDE_SHIRTS");
	std::string const STR_OPTION_EQUIP_HIDE_GLOVE("OPTION_EQUIP_HIDE_GLOVE");

	//std::string const STR_OPTION_EQUIP_HIDE_BELT("OPTION_EQUIP_HIDE_BELT");
	std::string const STR_OPTION_EQUIP_HIDE_PANTS("OPTION_EQUIP_HIDE_PANTS");
	std::string const STR_OPTION_EQUIP_HIDE_BOOTS("OPTION_EQUIP_HIDE_BOOTS");
	//std::string const STR_OPTION_EQUIP_HIDE_RINGL("OPTION_EQUIP_HIDE_RINGL");
	//std::string const STR_OPTION_EQUIP_HIDE_RINGR("OPTION_EQUIP_HIDE_RINGR");
	//std::string const STR_OPTION_EQUIP_HIDE_EARRING("OPTION_EQUIP_HIDE_EARRING");
	//std::string const STR_OPTION_EQUIP_HIDE_ATTSTONE("OPTION_EQUIP_HIDE_ATTSTONE");

	void UpdateHideInvenOption(PgActor* pkActor, EEquipPos const kItemPos, SClientDWORDOption const& rkClientOption)
	{
		if( !pkActor )
		{
			return;
		}

		//pkActor->EquipAllItem();

		PgPlayer* pkPlayer = NULL;
		PgPilot* pkPilot = pkActor->GetPilot();
		if( pkPilot )
		{
			pkPlayer = dynamic_cast< PgPlayer* >(pkPilot->GetUnit());
		}

		if( !pkPlayer )
		{
			return;
		}

		PgInventory* pkInven = pkPlayer->GetInven();

		PgBase_Item kFitItem, kCashFitItem;
		bool const bHaveFitNormal = S_OK == pkInven->GetItem(IT_FIT, kItemPos, kFitItem);
		bool const bHaveCashFitCash = S_OK == pkInven->GetItem(IT_FIT_CASH, kItemPos, kCashFitItem);

		if(bHaveFitNormal)
		{
			if( rkClientOption.IsHideEquipInvenPos(kItemPos) )
			{//! 장비 아이템 숨김
				pkActor->UnequipItem(IT_FIT, kItemPos, 0, PgItemEx::LOAD_TYPE_INSTANT, false, false);
			}
			else
			{//! 장비 아이템 보임
				pkActor->EquipItemByPos(IT_FIT, kItemPos, false);
			}
		}

		if(bHaveCashFitCash)
		{
			if( rkClientOption.IsHideCashInvenPos(kItemPos) )
			{//! 캐쉬 아이템 숨김
				pkActor->UnequipItem(IT_FIT_CASH, kItemPos, 0, PgItemEx::LOAD_TYPE_INSTANT, false, false);
			}
			else
			{//! 캐쉬 아이템 보임
				pkActor->EquipItemByPos(IT_FIT_CASH, kItemPos, false);
			}
		}

		if( g_kPilotMan.IsMyPlayer(pkPlayer->GetID()) )
		{
			char const* szActorViewerAddName = "CHARINFO";
			g_kMyActorViewMgr.ChangeEquip(szActorViewerAddName, pkPlayer->GetID());
		}

		if(EEquipPos::EQUIP_POS_HELMET == kItemPos)
		{// 투구를 감췄고
			bool bDisplayHelmet = 0;
			int const iOption = pkPlayer->GetAbil(AT_CLIENT_OPTION_SAVE) ;
			READ_DWORD_TO_BOOL(iOption, OPTION_TYPE_SHOW_HELMET, bDisplayHelmet);
			if(true == bDisplayHelmet)
			{// 투구 강제 보이기 옵션이 켜져있다면
				g_kStatusEffectMan.DisplayHeadTransformEffect(pkActor, false);
			}
		}
	}

	void UpdateOldAndNew(PgActor* pkActor, SClientDWORDOption const& rkOldOption, SClientDWORDOption const& rkNewOption)
	{
		if( rkOldOption.DisplayHelmet() != rkNewOption.DisplayHelmet() )
		{
			g_kStatusEffectMan.DisplayHeadTransformEffect(pkActor, !rkNewOption.DisplayHelmet());
		}
		size_t iCur = EQUIP_POS_NONE;
		for( ; EQUIP_POS_MAX > iCur; ++iCur )
		{
			EEquipPos const ePos = static_cast< EEquipPos >(iCur);
			if( (rkOldOption.IsHideCashInvenPos(ePos) != rkNewOption.IsHideCashInvenPos(ePos)) ||
				(rkOldOption.IsHideEquipInvenPos(ePos) != rkNewOption.IsHideEquipInvenPos(ePos)) )
			{
				UpdateHideInvenOption(pkActor, ePos, rkNewOption);
			}
		}
	}

	SClientDWORDOption OptionToClientOption(PgOption const& rkOption)
	{
		SClientDWORDOption kClientOption(0);
		kClientOption.DisplayHelmet( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_DISPLAY_HELMET)) );
		kClientOption.HideCashGlass( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_GLASS)) );
		kClientOption.HideCashHelmet( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_HELMET)) );
		kClientOption.HideCashShoulder( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_SHOULDER)) );
		kClientOption.HideCashClock( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_CLOAK)) );
		kClientOption.HideCashShirts( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_SHIRTS)) );
		kClientOption.HideCashGlove( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_GLOVE)) );
		kClientOption.HideCashWeapon( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_WEAPON)) );
		kClientOption.HideCashArm( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_ARM)) );
		kClientOption.HideCashBoots( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_BOOTS)) );
		kClientOption.HideCashPants( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_PANTS)) );
		kClientOption.HideCashBalloon( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_BALLOON)) );

		//kClientOption.HideGlass( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_GLASS)) );
		//kClientOption.HideNecklace( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_NECKLACE)) );
		kClientOption.HideHelmet( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_HELMET)) );
		kClientOption.HideShoulder( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_SHOULDER)) );
		kClientOption.HideClock( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_CLOAK)) );
		kClientOption.HideShirts( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_SHIRTS)) );
		kClientOption.HideGlove( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_GLOVE)) );

		//kClientOption.HideBelt( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_BELT)) );
		kClientOption.HidePants( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_PANTS)) );
		kClientOption.HideBoots( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_BOOTS)) );
		//kClientOption.HideRingL( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_RINGL)) );
		//kClientOption.HideRingR( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_RINGR)) );
		//kClientOption.HideEarring( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_EARRING)) );
		//kClientOption.HideAttstone( IntToBool(rkOption.GetValue(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_ATTSTONE)) );
		return kClientOption;
	}

	void ClientOptionToOption(SClientDWORDOption const& rkClientOption, PgOption& rkOut)
	{
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_DISPLAY_HELMET, BoolToInt(rkClientOption.DisplayHelmet()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_GLASS, BoolToInt(rkClientOption.HideCashGlass()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_HELMET, BoolToInt(rkClientOption.HideCashHelmet()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_SHOULDER, BoolToInt(rkClientOption.HideCashShoulder()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_CLOAK, BoolToInt(rkClientOption.HideCashClock()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_SHIRTS, BoolToInt(rkClientOption.HideCashShirts()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_GLOVE, BoolToInt(rkClientOption.HideCashGlove()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_WEAPON, BoolToInt(rkClientOption.HideCashWeapon()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_ARM, BoolToInt(rkClientOption.HideCashArm()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_BOOTS, BoolToInt(rkClientOption.HideCashBoots()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_PANTS, BoolToInt(rkClientOption.HideCashPants()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_CASH_EQUIP_HIDE_BALLOON, BoolToInt(rkClientOption.HideCashBalloon()), NULL);

		//rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_GLASS, BoolToInt(rkClientOption.HideGlass()), NULL);
		//rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_NECKLACE, BoolToInt(rkClientOption.HideNecklace()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_HELMET, BoolToInt(rkClientOption.HideHelmet()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_SHOULDER, BoolToInt(rkClientOption.HideShoulder()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_CLOAK, BoolToInt(rkClientOption.HideClock()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_SHIRTS, BoolToInt(rkClientOption.HideShirts()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_GLOVE, BoolToInt(rkClientOption.HideGlove()), NULL);

		//rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_BELT, BoolToInt(rkClientOption.HideBelt()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_PANTS, BoolToInt(rkClientOption.HidePants()), NULL);
		rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_BOOTS, BoolToInt(rkClientOption.HideBoots()), NULL);
		//rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_RINGL, BoolToInt(rkClientOption.HideRingL()), NULL);
		//rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_RINGR, BoolToInt(rkClientOption.HideRingR()), NULL);
		//rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_EARRING, BoolToInt(rkClientOption.HideEarring()), NULL);
		//rkOut.SetConfig(XML_ELEMENT_ETC, STR_OPTION_EQUIP_HIDE_ATTSTONE, BoolToInt(rkClientOption.HideAttstone()), NULL);

		////
		PgPilot* const pkPlayer = g_kPilotMan.GetPlayerPilot();
		if( pkPlayer )
		{
			// 옵션이 변경 됐으므로, 해당 어빌을 설정
			pkPlayer->SetAbil(AT_CLIENT_OPTION_SAVE, rkClientOption.WriteToDWORD()); // 이곳에서 투구 보이기 감추기를 호출
		}
	}

	inline void InsertOrSet(PgOption::ContConfigMap& rkConfigMap, std::string const& rkKey, int iValue)
	{
		typedef PgOption::ContConfigMap ContConfigMap;
		ContConfigMap::iterator itor = rkConfigMap.find( rkKey );
		if( rkConfigMap.end() != itor )
		{
			itor->second.first = iValue;
			itor->second.second = std::string();
		}
		else
		{
			rkConfigMap.insert( std::make_pair(rkKey, std::make_pair(iValue, std::string())) );
		}
	}

	void DBOption_WriteToPacket(PgOption const& rkOption, PgOption::ContConfigMap const& rkCurConfig, BM::Stream& rkPacket)
	{
		typedef PgOption::ContConfigMap ContConfigMap;

		ClientOption kOption;
		kOption.sVersion = ENEW_OPTION_DB_VER;



		char chKey[MAX_PATH] = {0, };
		short sCur = 0;
		for( ; OPTION_MAX_KEYSET_SIZE > sCur; ++sCur )
		{
			sprintf_s(chKey, "%s%s%u", XML_ELEMENT_KEYSET, STR_OPTION_SEPARATER, static_cast< unsigned long >( sCur )); // KEYSET/XXX
			std::string const rkConfigKey( chKey );

			BYTE& byKeySet = kOption.byKeySet[sCur];

			ContConfigMap::const_iterator find_iter = rkCurConfig.find( rkConfigKey );
			if( rkCurConfig.end() != find_iter )
			{
				int const iUKey = (*find_iter).second.first;

				if( ENIKEY_TO_UKEY <= iUKey )
				{
					byKeySet = static_cast< BYTE >( (*find_iter).second.first - ENIKEY_TO_UKEY );
				}
				else if(iUKey == 0)
				{
					byKeySet = 0;
				}
				else
				{
					byKeySet = ENIKEY_IGNORE_VALUE; // 150~165 까지
				}
			}
			else
			{
				byKeySet = ENIKEY_IGNORE_VALUE; // 없을 수도 있다 (145~149)
			}
		}

		{
			// 조이스틱 저장한다
			// kOption.byJoystick[]
			PgInputSystem* pkInput = NiDynamicCast(PgInputSystem, g_pkLocalManager->GetInputSystem());
			if(pkInput)
			{
				SPadButtonInformation* kPadTable = pkInput->GetPadSettingTable();
				if(kPadTable)
				{
					for(int i=0;NiInputGamePad::NIGP_NUMBUTTONS>i;++i)
					{
						kOption.byJoystick[i*2] = (kPadTable+i)->m_iUkey - ENIKEY_TO_UKEY;
						kOption.byJoystick[i*2+1] = (kPadTable+i)->m_iUIButtonIndex;
					}
				}
			}
		}

		// 32가지의 bool형 옵션을 저장 가능
		PgOptionUtil::SClientDWORDOption const kClientOption( PgOptionUtil::OptionToClientOption(rkOption) );
		kOption.dwShowOption = kClientOption.WriteToDWORD();

		rkPacket.Push(kOption);
	}

	void DBOption_ReadFromPacket(PgOption& rkOption, PgOption::ContConfigMap& rkCurConfig, BM::Stream& rkPacket)
	{
		typedef PgOption::ContConfigMap ContConfigMap;
#pragma pack(1)
		typedef struct tagOldClientOption {
			short sVersion;								// 2
			unsigned short usKeySetSize;				// 4
			BYTE byKeySet[OPTION_MAX_KEYSET_SIZE*3];	// 502(BYTE byKey + SHORT usKeyValue)
			DWORD dwShowOption;							// 506
			BYTE byReserved[OPTION_RESERVED];			// 520
		} OldClientOption;
#pragma pack()

		//
		short sVersion = 0;
		size_t const iRdPos = rkPacket.RdPos();
		rkPacket.Pop( sVersion );
		rkPacket.RdPos(iRdPos); // 원래로 돌린다

		DWORD dwShowOption = 0;

		switch( sVersion )
		{
		case EOLD_OPTOIN_DB_VER:
			{
				OldClientOption kOption;
				rkPacket.Pop( kOption );

				unsigned short usBufferPos = 0;
				char chKey[MAX_PATH] = {0, };
				BYTE byKey = 0;
				unsigned short usKeyValue = 0;

				if( OPTION_MAX_KEYSET_SIZE >= kOption.usKeySetSize )
				{
					unsigned short usKeySetSize = kOption.usKeySetSize;
					while( usKeySetSize-- > 0 )
					{
						byKey = *(reinterpret_cast<BYTE*>(&kOption.byKeySet[usBufferPos]));
						usKeyValue = *(reinterpret_cast<unsigned short*>(&kOption.byKeySet[usBufferPos+1]));
						usBufferPos += (sizeof(BYTE) + sizeof(unsigned short));
						sprintf_s(chKey, "%s%s%u", XML_ELEMENT_KEYSET, STR_OPTION_SEPARATER, static_cast<unsigned long>(byKey));

						InsertOrSet( rkCurConfig, std::string(chKey), usKeyValue );
					}
				}
				else
				{
					rkOption.DefaultKeySet();
				}

				// DefaultJoystick(); // 조이스틱 정보가 없는 버젼이다(초기화 해주자)
				rkOption.ApplyKeySet();
				// ApplyJoystick();

				dwShowOption = kOption.dwShowOption;
			}break;
		case ENEW_OPTION_DB_VER:
			{
				ClientOption kOption;
				rkPacket.Pop( kOption );

				char chKey[MAX_PATH] = {0, };

				short sCur = 0;
				for( ; OPTION_MAX_KEYSET_SIZE > sCur; ++sCur )
				{
					sprintf_s(chKey, "%s%s%u", XML_ELEMENT_KEYSET, STR_OPTION_SEPARATER, static_cast< unsigned long >( sCur )); // KEYSET/XXX

					BYTE const& byKeySet = kOption.byKeySet[sCur];
					if( ENIKEY_IGNORE_VALUE != byKeySet )
					{
						InsertOrSet( rkCurConfig, std::string(chKey), ENIKEY_TO_UKEY + byKeySet );
					}
				}

				{
					// kOption.byJoystick[]
					// JoyStick 정보를 풀어낸다
					PgInputSystem* pkInput = NiDynamicCast(PgInputSystem, g_pkLocalManager->GetInputSystem());
					if(pkInput)
					{
						SPadButtonInformation* kPadTable = pkInput->GetPadSettingTable();
						if(kPadTable)
						{
							for(int i=0;NiInputGamePad::NIGP_NUMBUTTONS>i;++i)
							{
								(kPadTable+i)->m_iUkey = kOption.byJoystick[i*2] + ENIKEY_TO_UKEY;
								(kPadTable+i)->m_iUIButtonIndex = kOption.byJoystick[i*2+1];
							}
						}
					}
				}

				rkOption.ApplyKeySet();
				// ApplyJoystick();

				dwShowOption = kOption.dwShowOption;
			}break;
		default:
			{
				rkOption.DefaultKeySet();
				rkOption.ApplyKeySet();
				return;
			}break;
		}

		// 32가지의 bool형 옵션을 저장 가능
		PgOptionUtil::SClientDWORDOption const kClientShowOption(dwShowOption);
		PgOptionUtil::ClientOptionToOption(kClientShowOption, rkOption);
	}
};


namespace lwOption
{
	//-------------------------------------------------------------------------
	//! for Cash Item  ---------------------------------------------------- 
	//-------------------------------------------------------------------------
	void SaveCashItemEquipHide()
	{
		g_kGlobalOption.ApplyConfig();
		g_kGlobalOption.Save(true);
	}

	void SetCashItemEquipHide(int const iItemPos, bool const bHide)
	{
		PgOptionUtil::SClientDWORDOption kClientOption(PgOptionUtil::OptionToClientOption(g_kGlobalOption));
		kClientOption.SetHideCashInvenPos(static_cast< EEquipPos >(iItemPos), bHide);
		PgOptionUtil::ClientOptionToOption(kClientOption, g_kGlobalOption);

		//PgOptionUtil::UpdateHideCashInvenOption(g_kPilotMan.GetPlayerActor(), static_cast< EEquipPos >(iItemPos), kClientOption);
		SaveCashItemEquipHide(); // UI 바뀔 시마다 저장
	}

	bool GetCashItemEquipHide(int const iItemPos)
	{
		PgOptionUtil::SClientDWORDOption const kClientOption(PgOptionUtil::OptionToClientOption(g_kGlobalOption));
		return kClientOption.IsHideCashInvenPos( static_cast< EEquipPos >(iItemPos) );
	}

	//-------------------------------------------------------------------------
	//! for Equip Item  ---------------------------------------------------- 
	//-------------------------------------------------------------------------
	void SaveItemEquipHide()
	{
		g_kGlobalOption.ApplyConfig();
		g_kGlobalOption.Save(true);
	}

	bool SetItemEquipHide(int const iItemPos, bool const bHide)
	{
		PgPlayer* pPlayer = g_kPilotMan.GetPlayerUnit();
		if(pPlayer)
		{
			int iClass = pPlayer->GetAbil(AT_CLASS);
			if( IS_CLASS_LIMIT(UCLIMIT_COMMON_DOUBLE_FIGHTER, iClass) 
				|| pPlayer->UnitType() == UT_SUB_PLAYER )
			{
				if( EQUIP_POS_BOOTS == static_cast< EEquipPos >(iItemPos))
				{
					PgActor* pkActor = g_kPilotMan.GetPlayerActor();
					if(pkActor)
					{
						if( pkActor->IsExistParts(EQUIP_LIMIT_KICKBALL) )
						{
							lwAddWarnDataStr(lwWString(TTW(791253)), 1, true);
							return false;
						}
					}
				}
			}
		}
		PgOptionUtil::SClientDWORDOption kClientOption(PgOptionUtil::OptionToClientOption(g_kGlobalOption));
		kClientOption.SetHideEquipInvenPos(static_cast< EEquipPos >(iItemPos), bHide);
		PgOptionUtil::ClientOptionToOption(kClientOption, g_kGlobalOption);

		//PgOptionUtil::UpdateHideCashInvenOption(g_kPilotMan.GetPlayerActor(), static_cast< EEquipPos >(iItemPos), kClientOption);
		SaveItemEquipHide(); // UI 바뀔 시마다 저장
		return true;
	}

	bool GetItemEquipHide(int const iItemPos)
	{
		PgOptionUtil::SClientDWORDOption const kClientOption(PgOptionUtil::OptionToClientOption(g_kGlobalOption));
		return kClientOption.IsHideEquipInvenPos( static_cast< EEquipPos >(iItemPos) );
		return false;
	}
	

	void RegisterWrapper(lua_State* pkState)
	{
		using namespace lua_tinker;
		//! for Cash Item
		def(pkState, "Option_GetCashItemEquipHide", GetCashItemEquipHide);
		def(pkState, "Option_SetCashItemEquipHide", SetCashItemEquipHide);

		//! for Equip Item
		def(pkState, "Option_GetItemEquipHide", GetItemEquipHide);
		def(pkState, "Option_SetItemEquipHide", SetItemEquipHide);
	}
};


PgOption::PgOption() : 
m_pkCurConfigMap(NULL),
m_bConfigCreated(false),
m_bPrevDisplayHP(false)
{}

PgOption::~PgOption()
{
//	_PgOutputDebugString("PgOption :: Destructor at %d\n -- UsingNiNew", timeGetTime());
}

bool PgOption::Save(bool const bSendToServer)
{
	TiXmlDocument kDocu(STR_OPTION_USER_FILE);

	TiXmlNode* pkNewNode = kDocu.InsertEndChild(TiXmlElement(XML_ELEMENT_CONFIG));
	if(NULL != pkNewNode)
	{		
		for(ContConfigMap::iterator kIter = m_kSavedMap.begin(); m_kSavedMap.end() != kIter; ++kIter)
		{
			int iValue = kIter->second.first;//VALUE
			char const* szText = (0 == kIter->second.second.size()) ? (NULL) : (kIter->second.second.c_str());//TEXT
			std::string kKeyName = kIter->first.c_str();//KeyName
			std::vector< std::string > kVec;
			PgStringUtil::BreakSep(kKeyName, kVec, STR_OPTION_SEPARATER);
			if(2 == kVec.size())
			{
				//부모 이름을 찾는다.
				TiXmlElement* pkParent = pkNewNode->FirstChildElement(kVec[0].c_str());
				if(!pkParent)
				{//부모 키가 없다.
					TiXmlNode* pkParentNode = pkNewNode->InsertEndChild(TiXmlElement(kVec[0].c_str()));
					pkParent = dynamic_cast<TiXmlElement*>(pkParentNode);
					if(NULL == pkParent)
					{
						NILOG(PGLOG_ERROR, "don't insert element in document config.xml");
						assert(0);
						return false;
					}
				}
				//부모 키 가 있다.
				TiXmlNode* pkChildNode = pkParent->InsertEndChild(TiXmlElement(XML_ELEMENT_ITEM));
				TiXmlElement* pkChild = dynamic_cast<TiXmlElement*>(pkChildNode);
				if(NULL != pkChild)
				{
					pkChild->SetAttribute(XML_ATTR_NAME, kVec[1].c_str());
					pkChild->SetAttribute(XML_ATTR_VALUE, iValue);
					if(NULL != szText)
					{
						pkChild->InsertEndChild(TiXmlText(szText));
					}
				}
				else
				{
					NILOG(PGLOG_ERROR, "don't insert element in document config.xml");
					assert(0);
					return false;
				}
			}
			else
			{
				NILOG(PGLOG_ERROR, "don't support multi tree option");
				assert(0);
			}
		}
	}
	else
	{
		NILOG(PGLOG_ERROR, "Insert Root element failed");
		assert(0);
	}
	if(!kDocu.SaveFile())
	{
		NILOG(PGLOG_ERROR, "Save faild config.xml");
		return false;
	}
	
	if (bSendToServer)
	{
		BM::Stream kOPacket(PT_C_T_REQ_SAVE_OPTION);
		WriteToPacket(kOPacket);
		NETWORK_SEND(kOPacket)
	}

	return true;
}
bool PgOption::Load()
{
	bool bRet = true;

	TiXmlDocument kDocu;
	bRet = kDocu.LoadFile(STR_OPTION_USER_FILE);
	if(bRet)
	{
		pCurConfigMap(&m_kSavedMap);
		bRet = ParseXml(kDocu.FirstChild());
	}

	if(false == bRet)
	{//Default Set
		m_bConfigCreated = true;
		m_kSavedMap = m_kDefaultMap;//Cur = Saved = Default

		//최초 실행시 전체화면 해상도로 실행되기 때문에
		int const x = XUIMgr.GetResolutionSize().x;
		int const y = XUIMgr.GetResolutionSize().y;
		
		//int const fullScreen = (true==NiApplication::ms_pkApplication->GetFullscreen()) ? 1 : 2;

		SetConfig(XML_ELEMENT_GAME, STR_GAME_RESOLUTION_WIDTH, &x, NULL, &m_kSavedMap);
		SetConfig(XML_ELEMENT_GAME, STR_GAME_RESOLUTION_HEIGHT, &y, NULL, &m_kSavedMap);
		//SetConfig(XML_ELEMENT_GAME, STR_GAME_FULL_SCREEN, &fullScreen, NULL, &m_kSavedMap);
				
		//int iGraphicSet = 1;
		//if (PgComputerInfo::GetGpuGrade() >= DEVICE_GRADE_HIGH)
		//{
		//	iGraphicSet = 1;
		//	SetConfig(XML_ELEMENT_OPTION, STR_OPTION_GRAPHIC_SET, &iGraphicSet, NULL, &m_kSavedMap);
		//}
		//else if (PgComputerInfo::GetGpuGrade() >= DEVICE_GRADE_MEDIUM)
		//{
		//	iGraphicSet = 3; // custom setting을 이용
		//	SetConfig(XML_ELEMENT_OPTION, STR_OPTION_GRAPHIC_SET, &iGraphicSet, NULL, &m_kSavedMap);
		//}
		//else
		//{
		//	iGraphicSet = 2;
		//	SetConfig(XML_ELEMENT_OPTION, STR_OPTION_GRAPHIC_SET, &iGraphicSet, NULL, &m_kSavedMap);
		//}
		Save();
		m_kConfigMap = m_kSavedMap;
	}
	else
	{//Saved Set
		m_bConfigCreated = false;
		m_kConfigMap = m_kSavedMap;//Cur Set = Saved Set

		bRet = Diff();//Config Version Check
	}
	if(0 == GetValue(XML_ELEMENT_GAME,"FLY_TO_CAMERA"))
	{// 최초 로딩시 화면깨짐 값이 없다면 켜짐 값으로 세팅한다
		SetConfig(XML_ELEMENT_GAME, "FLY_TO_CAMERA", 1, NULL);
	}

	ApplyConfig();

	return bRet;
}
bool PgOption::Diff()
{
	PG_ASSERT_LOG(m_kDefaultMap.size());
	if (0 == m_kDefaultMap.size())
	{
		return false;
	}

	//Config Version Check
	std::string kKey = XML_ELEMENT_VERSION;
	kKey.append(STR_OPTION_SEPARATER);
	kKey.append(XML_ELEMENT_CONF_VER);
	int const iCurrentVersion = GetValue(XML_ELEMENT_VERSION, XML_ELEMENT_CONF_VER);//
	int const iDefaultVersion = m_kDefaultMap[kKey].first;//DefaultConfig.xml 에는 항상 있을것이라는 가정

	if(iCurrentVersion != iDefaultVersion)
	{//Sync ConfigMap from DefaultMap
		m_kConfigMap[kKey] = m_kDefaultMap[kKey];//Set Version
		
		for(ContConfigMap::iterator kIter = m_kDefaultMap.begin(); m_kDefaultMap.end() != kIter; ++kIter)
		{
			std::vector<std::string> kVec;
			PgStringUtil::BreakSep(kIter->first, kVec, STR_OPTION_SEPARATER);

			int iValue = GetValue(kVec[0], kVec[1]);
			std::string kValue = GetText(kVec[0], kVec[1]);

			ContConfigMap::iterator kSubIter = m_kConfigMap.find(kIter->first);
			if(m_kConfigMap.end()==kSubIter)//찾아서 없으면 추가
			{
				std::pair<ContConfigMap::iterator, bool> eRet = m_kConfigMap.insert(*kIter);
				if(!eRet.second)
				{
					NILOG(PGLOG_ERROR, "Map insert failed..."); assert(0);
					return false;
				}
			}
			//else
			//{
			//	어떤 기준으로? 이전의 값을 바꿀것이냐? 사용자 설정은?
			//	없던 값을 추가 하는것을 기본으로 가고, 이후 버젼이 바뀌면서 완전히 변경되는 값에 대해서는
			//	새로운 Config를 추가 하는것으로 한다. 정해진 갯수 제한(300개?)를 넘쳐서 나는 미래 에러에 대해서는
			//	이전의 Config.xml을 삭제 하는 조치를 취하거나
			//	Config.xml을 여러가지로 나누어 영향력이 다른 Config들을 각각 사용하게 제작 한다.
			//}
		}
		
		//없어야 하는데 있는 값들을 삭제 한다.
		for(ContConfigMap::iterator kIter = m_kConfigMap.begin(); m_kConfigMap.end() != kIter; ++kIter)
		{
			ContConfigMap::iterator kSubIter = m_kDefaultMap.find(kIter->first);
			if(m_kDefaultMap.end() == kSubIter)
			{
				kIter = m_kConfigMap.erase(kIter);
			}
		}
		RollBackAll(&m_kConfigMap, &m_kSavedMap);
		Save();
	}

	return true;
}

void PgOption::ApplyKeySet_ToSystem()
{
	int const iPassedKey = GetValue(STR_OPTION_KEYSET, STR_OPTION_PASSED_KEY);
	
	for(ContConfigMap::iterator kIter = m_kSavedMap.begin(); m_kSavedMap.end() != kIter; ++kIter)
	{
		if(iPassedKey == kIter->second.first)//Passed setting key (is NULL KEY)
		{
			continue;
		}

		std::vector< std::string > kVec;
		PgStringUtil::BreakSep(kIter->first, kVec, STR_OPTION_SEPARATER);
		if(0 == strcmp(kVec[0].c_str(), XML_ELEMENT_KEYSET))//KEYSET Element 와 같으면
		{
			int iUKey = kIter->second.first;
			int const iKey = atoi(kVec[1].c_str());
			if(NiInputKeyboard::KEY_TOTAL_COUNT <= iKey)//엔진에서 지원하는 입력기준은 무시한다.
			{// 사용 하지 않는 키로 이동 이므로, 해당 Scan키 입력에 대응되는 UKey값은 0으로 세팅해주어 동작이 일어나지 않게 해준다
				iUKey = 0;
			}
			bool bRet = g_pkLocalManager->ActionKeyboard_Set(kVec[1].c_str(), iUKey);
			if(!bRet)
			{
				NILOG(PGLOG_ERROR, "KeyBoard setting failed");
				assert(0);
			}

			//새로운 UKey -> Key 번호 기억
			if(bRet)
			{
				std::pair<ContUKeyToKey::iterator, bool> eRet = m_kUkeyToKey.insert(std::make_pair(iUKey, iKey));
				if(!eRet.second)
				{
					NILOG(PGLOG_ERROR, "Insert Failed, maybe duplicated key");
					assert(0);
				}
			}
		}
	}
}

void PgOption::RollBackAll(ContConfigMap* pkFrom, ContConfigMap* pkTo)
{
	*pkTo = *pkFrom;
}
void PgOption::RollBackConfig(ContConfigMap* pkFrom, ContConfigMap* pkTo)
{
	ContConfigMap::iterator kIter = pkFrom->begin();
	for(; pkFrom->end() != kIter; ++kIter)
	{
		std::vector<std::string> kVec;
		PgStringUtil::BreakSep(kIter->first, kVec, STR_OPTION_SEPARATER);
		bool bConfig = false;
		if(0 != strcmp(kVec[0].c_str(), XML_ELEMENT_GRAPHIC3))	{ bConfig = true; } //Custome Graphic set
		if(0 != strcmp(kVec[0].c_str(), XML_ELEMENT_SOUND))		{ bConfig = true; } //Sound
		if(0 != strcmp(kVec[0].c_str(), XML_ELEMENT_ETC))		{ bConfig = true; } //ETC
		if(0 != strcmp(kVec[0].c_str(), XML_ELEMENT_OPTION))	{ bConfig = true; } //User Option
		
		if(bConfig)
		{
			SetConfig(kVec[0], kVec[1], &kIter->second.first, kIter->second.second.c_str(), pkTo);
		}
	}
}
void PgOption::RollBackKeySet(ContConfigMap* pkFrom, ContConfigMap* pkTo)
{
	for(ContConfigMap::iterator kIter = pkFrom->begin(); pkFrom->end() != kIter; ++kIter)
	{
		std::vector<std::string> kVec;
		PgStringUtil::BreakSep(kIter->first, kVec, STR_OPTION_SEPARATER);

		if(0 == strcmp(kVec[0].c_str(), XML_ELEMENT_KEYSET))//KEYSET Element 와 같으면
		{
			SetConfig(kVec[0], kVec[1], &kIter->second.first, kIter->second.second.c_str(), pkTo);
		}
	}
}

extern void _TextOrTTW(char const* szTTW, char const* szText, std::wstring& rkText);

bool PgOption::ParseItem_KeySet(TiXmlElement const* pkNode)
{
	TiXmlElement const* pkChild = pkNode;
	{
		char const* szParentName = pkChild->Parent()->Value();
		char const* szUKey = pkChild->Attribute(XML_ATTR_UKEY);
		char const* szIconNo = pkChild->Attribute(XML_ATTR_ICONNO);
		char const* szTTW = pkChild->Attribute(XML_ATTR_TTW);
		char const* szText = pkChild->GetText();

		std::wstring kFunctionName;

		_TextOrTTW(szTTW, szText, kFunctionName);

		unsigned int iUKey = atoi(szUKey);
		if(0 == iUKey)
		{
			NILOG(PGLOG_ERROR, "Unique Key is NULL or invalid");
			assert(0);
			return false;
		}

		unsigned int iIconNo = atoi(szIconNo);
		if(0 == iUKey)
		{
			NILOG(PGLOG_ERROR, "Icon Resource is null or invalid");
			assert(0);
			return false;
		}

		//중복체크
		ContKeySetMap::iterator kIter = m_kKeyFunctionTable.find(iUKey);
		if(m_kKeyFunctionTable.end() != kIter)
		{
			NILOG(PGLOG_ERROR, "Duplicate keyset table Unique Key");
			assert(0);
			return false;
		}

		std::pair<ContKeySetMap::iterator, bool> eRet = m_kKeyFunctionTable.insert(std::make_pair(iUKey, std::make_pair(kFunctionName, iIconNo)));
		if(!eRet.second)
		{
			NILOG(PGLOG_ERROR, "Critical Error failed insert item to map");
			assert(0);
			return false;
		}
	}

	return true;
}
bool PgOption::ParseItem(TiXmlElement const* pkNode)
{
	TiXmlElement const* pkChild = pkNode;
	//while(pkChild)
	{
		char const* szParentName = pkChild->Parent()->Value();
		char const* szName = pkChild->Attribute(XML_ATTR_NAME);
		char const* szText = pkChild->GetText();

		char const* szValue = pkChild->Attribute(XML_ATTR_VALUE);

		if(NULL == szName)
		{
			PgXmlError(pkChild, "null option name");
			return false;
		}

		if(NULL == szValue && NULL == szText)
		{
			PgXmlError1(pkChild, "Can't found VALUE='' attribute and TEXT in Element %s", szName);
			return false;
		}

		int iValue = 0;
		if(NULL != szValue)
		{
			iValue = atoi(szValue);
		}

		//HIGH,LOW,CUSTOME, SOUND, ETC
		if(false == SetConfig(szParentName, szName, &iValue, szText, pCurConfigMap()))
		{
			NILOG(PGLOG_ERROR, "Map insert failed in Config Map");
			return false;
		}
		
		pkChild = pkChild->NextSiblingElement();
	}
	return true;
}

bool PgOption::ParseXml(TiXmlNode const* pkNode, void* pArg, bool bUTF8)
{
	if(0 == m_kKeyNoToKeyStr.size())
	{
		MakeKeynoToKeystr();
	}

	int const iType = pkNode->Type();
	if(NULL == pkNode || TiXmlNode::ELEMENT != iType)
	{
		return false;
	}

	if(NULL == pCurConfigMap())
	{
		pCurConfigMap(&m_kDefaultMap);
	}

	TiXmlElement const* pkChild = pkNode->FirstChildElement();
	while(pkChild)
	{
		char const* szParentName = pkChild->Parent()->Value();
		char const* szName = pkChild->Value();
		char const* szText = pkChild->GetText();
		
		if(0 == strcmp(szName, XML_ELEMENT_CONFIG))
		{
			ParseXml(pkChild);
		}
		else if(0 == strcmp(szName, XML_ELEMENT_GRAPHIC1))
		{
			ParseXml(pkChild);
		}
		else if(0 == strcmp(szName, XML_ELEMENT_GRAPHIC2))
		{
			ParseXml(pkChild);
		}
		else if(0 == strcmp(szName, XML_ELEMENT_GRAPHIC3))
		{
			ParseXml(pkChild);
		}
		else if(0 == strcmp(szName, XML_ELEMENT_SOUND))
		{
			ParseXml(pkChild);
		}
		else if(0 == strcmp(szName, XML_ELEMENT_ETC))
		{
			ParseXml(pkChild);
		}
		else if(0 == strcmp(szName, XML_ELEMENT_OPTION))
		{
			ParseXml(pkChild);
		}
		else if(0 == strcmp(szName, XML_ELEMENT_ITEM))
		{
			ParseItem(pkChild);
		}
		else if(0 == strcmp(szName, XML_ELEMENT_KEYSET))
		{
			ParseXml(pkChild);
		}
		else if(0 == strcmp(szName, XML_ELEMENT_KEYSET_TABLE))
		{
			ParseXml(pkChild);
		}
		else if(0 == strcmp(szName, XML_ELEMENT_KEYMAP))
		{
			ParseItem_KeySet(pkChild);
		}
		else
		{
			//PgXmlError1(pkChild, "Invaid Element: %s", szName);
			NILOG(PGLOG_LOG, "[PgOption] is it Invalid Config name ?: %s[Row:%d, Col:%d]\n", szName, pkChild->Row(), pkChild->Column());
			if(!ParseXml(pkChild))
			{
				return false;
			}
		}
		pkChild = pkChild->NextSiblingElement();
	}
	
	return true;
}

bool PgOption::SetConfig(std::string const& szHeadKey, std::string const& szKey, int const* piValue, char const* szText, ContConfigMap* pkConfigMap)
{
	std::string kValue = ((NULL == szText) ? "" : szText);
	int iValue = ((NULL == piValue) ? 0 : *piValue);

	if(NULL == pkConfigMap)
	{
		pkConfigMap = &m_kConfigMap;
	}
	
	std::string szKeyName = szHeadKey;
	szKeyName.append(STR_OPTION_SEPARATER);
	szKeyName.append(szKey);

	ContConfigMap::iterator kIter = pkConfigMap->find(szKeyName);
	if(pkConfigMap->end() != kIter)
	{
		if(piValue)	{ kIter->second.first = iValue; }
		if(szText)	{ kIter->second.second = kValue; }

		return true;
	}

	//최대 사이즈 제한
	if(O_MaxConfigCount <= pkConfigMap->size())
	{
		NILOG(PGLOG_LOG, "over %d config count", O_MaxConfigCount);
		assert(0);
		return false;
	}

	//
	std::pair<ContConfigMap::iterator, bool> eRet = pkConfigMap->insert(std::make_pair(szKeyName, std::make_pair(iValue, kValue)));
	if(!eRet.second)
	{
		NILOG(PGLOG_ERROR, "failed Insert item to map");
		assert(0);
		return false;
	}
	return true;
}
bool PgOption::SetConfig(std::string const& szHeadKey, std::string const& szKey, int const iValue, char const* szText)
{
	int iVal = iValue;
	return SetConfig(szHeadKey, szKey, &iVal, szText, &m_kConfigMap);;
}

bool PgOption::SysSetConfig(std::string const& szHeadKey, std::string const& szKey, int const iValue, char const* szText)
{
	int iVal = iValue;
	bool bRet = true;
	bRet = (bRet & SetConfig(szHeadKey, szKey, &iVal, szText, &m_kSavedMap));
	bRet = (bRet & SetConfig(szHeadKey, szKey, &iVal, szText, &m_kConfigMap));
	return bRet;
}
int const PgOption::GetValue(std::string const& szHeadKey, std::string const& szKey) const
{
	std::string kKeyName = szHeadKey;
	kKeyName.append(STR_OPTION_SEPARATER);
	kKeyName.append(szKey);
	ContConfigMap::const_iterator kIter = m_kConfigMap.find(kKeyName);
	if(m_kConfigMap.end() != kIter)
	{
		return (kIter->second).first;
	}
	return 0;
}
int const PgOption::GetDefaultValue(std::string const& szHeadKey, std::string const& szKey) const
{
	std::string kKeyName = szHeadKey;
	kKeyName.append(STR_OPTION_SEPARATER);
	kKeyName.append(szKey);
	ContConfigMap::const_iterator kIter = m_kDefaultMap.find(kKeyName);
	if(m_kDefaultMap.end() != kIter)
	{
		return (kIter->second).first;
	}
	return 0;
}

int const PgOption::GetCurrentGraphicOption(std::string const& szKey) const
{
	int retValue = 0;
	int const graphicSet = GetValue(XML_ELEMENT_OPTION, STR_OPTION_GRAPHIC_SET);
	switch(graphicSet)
	{
	case 1:
		retValue = GetValue(XML_ELEMENT_GRAPHIC1, szKey);
		break;
	case 2:
		retValue = GetValue(XML_ELEMENT_GRAPHIC2, szKey);
		break;
	case 3:
		retValue = GetValue(XML_ELEMENT_GRAPHIC3, szKey);
		break;
	default:
		break;
	}

	return retValue;
}

void PgOption::SetCurrentGraphicOption(std::string const& szKey, int iValue)
{
	int const graphicSet = GetValue(XML_ELEMENT_OPTION, STR_OPTION_GRAPHIC_SET);
	switch(graphicSet)
	{
	case 1:
		SetConfig(XML_ELEMENT_GRAPHIC1, szKey, iValue, NULL);
		break;
	case 2:
		SetConfig(XML_ELEMENT_GRAPHIC2, szKey, iValue, NULL);
		break;
	case 3:
		SetConfig(XML_ELEMENT_GRAPHIC3, szKey, iValue, NULL);
		break;
	default:
		break;
	}

}

char const* PgOption::GetText(std::string const& szHeadKey, std::string const& szKey) const
{
	std::string kKeyName = szHeadKey;
	kKeyName.append(STR_OPTION_SEPARATER);
	kKeyName.append(szKey);
	ContConfigMap::const_iterator kIter = m_kConfigMap.find(kKeyName);
	if(m_kConfigMap.end() != kIter)
	{
		return (kIter->second).second.c_str();
	}

	return "";
}

char const* PgOption::GetDefaultText(std::string const& szHeadKey, std::string const& szKey) const
{
	std::string kKeyName = szHeadKey;
	kKeyName.append(STR_OPTION_SEPARATER);
	kKeyName.append(szKey);
	ContConfigMap::const_iterator kIter = m_kDefaultMap.find(kKeyName);
	if(m_kDefaultMap.end() != kIter)
	{
		return (kIter->second).second.c_str();
	}

	return "";
}

bool PgOption::GetKeyFuncResource(const unsigned int iUKey, std::wstring &rkName, unsigned int &rkResNo) const
{
	ContKeySetMap::const_iterator kIter = m_kKeyFunctionTable.find(iUKey);
	if(m_kKeyFunctionTable.end() != kIter)
	{
		rkName = kIter->second.first;
		rkResNo = kIter->second.second;

		return true;
	}

	return false;
}

void PgOption::MakeKeynoToKeystr()
{
	for(int i = 0; i < NiInputKeyboard::KEY_TOTAL_COUNT + 200; ++i)//pkIcon의 KeyNo를 저장한다. (여유분 +200);
	{
		char szTemp[255] = {0, };
		std::string kTemp;
		errno_t eRet = _itoa_s(i, szTemp, 255, 10);
		if(eRet == 0)
		{
			kTemp = szTemp;
			std::pair<ContKeynoKeystr::iterator, bool> ret = m_kKeyNoToKeyStr.insert(std::make_pair(i, kTemp));
			if(!ret.second)
			{
				NILOG(PGLOG_ERROR, "insert failed at map");
				assert(0);
				return;
				
			}
		}
		else
		{
			NILOG(PGLOG_ERROR, "unknown error T_T");
			assert(0);
		}
	}
	
}
bool PgOption::GetKeynoToKeystr(int const iKeyNo, std::string &rkKeyStr) const
{
	ContKeynoKeystr::const_iterator kIter = m_kKeyNoToKeyStr.find(iKeyNo);
	if(m_kKeyNoToKeyStr.end() != kIter)
	{
		rkKeyStr = kIter->second;

		return true;
	}

	return false;
}
int const PgOption::GetUKeyToKey(int iUKey) const
{
	ContUKeyToKey::const_iterator kIter = m_kUkeyToKey.find(iUKey);
	if(m_kUkeyToKey.end() != kIter)
	{
		return kIter->second;
	}

	return NULL;
}

void PgOption::ReadFromPacket(BM::Stream& rkPacket)
{	
	g_kGlobalOption.DefaultKeySet();
	
	// 서버로 부터 Option값 받아와 시스템에 적용시키기
	PgOptionUtil::DBOption_ReadFromPacket( *this, m_kConfigMap, rkPacket );

	m_bPrevDisplayHP = GetValue(XML_ELEMENT_ETC, "BATTLEOP_DISPLAY_HPBAR" );
}

void PgOption::WriteToPacket(BM::Stream& rkPacket)
{// 서버에 Option값 저장하기
	PgOptionUtil::DBOption_WriteToPacket( *this, m_kConfigMap, rkPacket );
}

void PgOption::ApplyConfig()
{
	RollBackConfig(&m_kConfigMap, &m_kSavedMap);
	
	lwUpdateEtcConfig();
}

void PgOption::DefaultConfig()
{
	RollBackConfig(&m_kDefaultMap, &m_kConfigMap);
}

void PgOption::CancelConfig()
{
	RollBackConfig(&m_kSavedMap, &m_kConfigMap);
}

void PgOption::ApplyKeySet()
{
	RollBackKeySet(&m_kConfigMap, &m_kSavedMap);
	m_kUkeyToKey.clear();
	ApplyKeySet_ToSystem();
}

void PgOption::DefaultKeySet()
{
	RollBackKeySet(&m_kDefaultMap, &m_kConfigMap);
}

void PgOption::CancelKeySet()
{
	RollBackKeySet(&m_kSavedMap, &m_kConfigMap);
}

bool const PgOption::IsDisplayHelmet() const
{// 강제 투구 보이기 옵션이 켜져있는가
	return PgOptionUtil::OptionToClientOption(*this).DisplayHelmet();
}

void PgOption::SetDisplayHelmet(bool const bDisplay)
{// 강제 투구 보이기 옵션 설정
 // 파일 및, 서버에 저장 하려면 Save() 멤버를 호출 해야 한다
	PgOptionUtil::SClientDWORDOption kClientOption( PgOptionUtil::OptionToClientOption(*this) );
	kClientOption.DisplayHelmet( bDisplay );
	PgOptionUtil::ClientOptionToOption(kClientOption, *this);
}

bool PgOption::GetUKeyToKeyStr(int const iUKey, std::wstring& kCurrentKeyName_out)
{
	int const iKey = GetUKeyToKey(iUKey);
	std::string kKetStr;
	if(true == GetKeynoToKeystr(iKey, kKetStr))
	{
		kCurrentKeyName_out = UNI(kKetStr);
		return true;
	}
	kCurrentKeyName_out.clear();
	return false;
}

void PgOption::OffDisplayHP()
{
	//! 옵션을 미리 저장하고..
	m_bPrevDisplayHP = GetValue(XML_ELEMENT_ETC, "BATTLEOP_DISPLAY_HPBAR" );
	
	//! 무조건 끈다.
	SetConfig(XML_ELEMENT_ETC, "BATTLEOP_DISPLAY_HPBAR", PgOptionUtil::BoolToInt(false), NULL);
	PgEnergyGauge::ms_bDrawEnergyGaugeBar = false;
}
void PgOption::RestoreDisplayHP()
{
	//! 미리 저장된 옵션으로 복구
	SetConfig(XML_ELEMENT_ETC, "BATTLEOP_DISPLAY_HPBAR", PgOptionUtil::BoolToInt(m_bPrevDisplayHP), NULL);
	PgEnergyGauge::ms_bDrawEnergyGaugeBar = m_bPrevDisplayHP;
}

void PgOption::SetSendOptionFlag(int const iValue)
{
	SetUseLevelRank(iValue&SOF_LEVELRANK);
}

void PgOption::SetUseLevelRank(bool const IsUse)
{
	SetConfig(XML_ELEMENT_ETC, "LEVEL_RANK", static_cast<int>(IsUse), NULL);
}

bool PgOption::GetUseLevelRank()const
{
	return GetValue(XML_ELEMENT_ETC, "LEVEL_RANK") ? true : false;
}

void PgOption::ShowNavigation(bool const bShow)
{
	if(bShow)
	{
		lua_tinker::call<bool>("CallNaviMap_WorldMap");
	}
	else
	{
		lwCloseUI("SFRM_NAVI_MAP");
	}
}

lwGlobalOption GetGlobalOption()
{
	return &g_kGlobalOption;
}