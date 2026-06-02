#include "StdAfx.h"
#include "PgUIScene.h"
#include "lwUI.h"
#include "lwMinimap_Script.h"

wchar_t const * const NPC_ICON_TYPE_NAME = _T("ICON_PATH_NPC");

namespace lwMinimap
{
	void RegisterWrapper(lua_State *pkState)
	{
		using namespace lua_tinker;

		def(pkState, "OnOverMinimapIconItem", OnOverMinimapIconItem);
		def(pkState, "OnClickMinimapIconItem", OnClickMinimapIconItem);
		def(pkState, "ClearMinimapIconInfo", ClearMinimapIconInfo);
		def(pkState, "CallMinimapObjectList", CallMinimapObjectList);
		def(pkState, "CloseMinimapObjectList", CloseMinimapObjectList);
	}

	void OnOverMinimapIconItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pkSelf = kSelf.GetSelf();
		if( !pkSelf )
		{
			return;
		}

		std::wstring kIconID = kSelf.GetCustomDataAsStr().GetWString();

		PgAlwaysMiniMap* pkMinimap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI("MiniMap_Whole"));
		if( !pkMinimap ){ return; }

		pkMinimap->SelectObject( kIconID );
	}

	void OnClickMinimapIconItem(lwUIWnd kSelf)
	{
		XUI::CXUI_Wnd* pkSelf = kSelf.GetSelf();
		if( !pkSelf )
		{
			return;
		}

		std::wstring kIconID = kSelf.GetCustomDataAsStr().GetWString();

		PgAlwaysMiniMap* pkMinimap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI("MiniMap_Whole"));
		if( !pkMinimap ){ return; }

		pkMinimap->SelectObject( kIconID );
		pkMinimap->OnClickIconInfoListItem();
	}

	void ClearMinimapIconInfo()
	{
		PgAlwaysMiniMap* pkMinimap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI("MiniMap_Whole"));
		if( !pkMinimap ){ return; }

		pkMinimap->SelectObject( L"" );
	}

	void CallMinimapObjectList(lwUIWnd kMainUI)
	{
		if(false==g_kUIScene.IsAlwaysMinimapDraw())
		{
			return;
		}

		XUI::CXUI_Wnd* pkMainUI = kMainUI.GetSelf();
		if( !pkMainUI )
		{
			pkMainUI = XUIMgr.Get(L"SFRM_BIG_MAP");
			if( !pkMainUI ){ return; }
		}

		PgAlwaysMiniMap* pkMinimap = dynamic_cast<PgAlwaysMiniMap*>(g_kUIScene.GetMiniMapUI("MiniMap_Whole"));
		if( !pkMinimap ){ return; }

		CONT_MINIMAP_ICON_INFO kMapIconList;
		if( pkMinimap->GetMinimapIconInfo(kMapIconList) )
		{
			UpdateNpcListUI(pkMainUI, kMapIconList);
		}
	}

	bool UpdateNpcListUI(XUI::CXUI_Wnd* pkMainUI, CONT_MINIMAP_ICON_INFO& kMapIconList)
	{
		if(NULL == pkMainUI || kMapIconList.empty())
		{
			return false;
		}

		int const iMaxSlot = 3;
		int iListCount = kMapIconList.size()/iMaxSlot;
		if(0 != kMapIconList.size()%iMaxSlot)
		{
			++iListCount;
		}

		XUI::CXUI_Wnd* pkListMainUI = NULL;
		if(1==iListCount || 2==iListCount)
		{
			pkListMainUI = XUIMgr.Call(L"SFRM_ACTOR_LIST_0");
		}
		else
		{
			pkListMainUI = XUIMgr.Call(L"SFRM_ACTOR_LIST");
		}
		
		if( !pkListMainUI ){ return false; }

		XUI::CXUI_List* pkList = dynamic_cast<XUI::CXUI_List*>(pkListMainUI->GetControl(L"LST_ACTORS"));
		if( !pkList )
		{
			pkListMainUI->Close(); 
			return false; 
		}
		
		POINT3I kLocation = pkMainUI->Location();
		POINT2 kSize = pkMainUI->Size();
		kLocation.y += pkMainUI->Size().y;
		pkListMainUI->Location(kLocation);
		pkList->DisplayStartPos(0);
		pkList->AdjustMiddleBtnPos();

		int iWidth = pkListMainUI->Size().x;
		int iHeight = 108;
		if(1 == iListCount)
		{
			iHeight = 60;
		}
		pkListMainUI->Size(POINT2(iWidth,iHeight));
		
		
		//int const iGab = pkListMainUI->Size().y - pkMainUI->Size().y;
		//pkListMainUI->Size(POINT2(pkListMainUI->Size().x, pkMainUI->Size().y));

		XUIListUtil::SetMaxItemCount( pkList, iListCount );

		CONT_MINIMAP_ICON_INFO::iterator icon_itor = kMapIconList.begin();
		XUI::SListItem* pkItem = pkList->FirstItem();
		wchar_t szTemp[MAX_PATH] = {0,};
		int iCount = 0;
		while( pkItem && pkItem->m_pWnd )
		{
			if(iCount < iMaxSlot)
			{
				swprintf_s(szTemp, MAX_PATH, L"FRM_M_ACTOR_SLOT%d", iCount);
				XUI::CXUI_Wnd* pkSlot = pkItem->m_pWnd->GetControl(szTemp);
				if(pkSlot)
				{
					pkSlot->Visible(false);
					if( icon_itor != kMapIconList.end() )
					{
						pkSlot->Visible(true);
						CONT_MINIMAP_ICON_INFO::value_type& kIconInfo = (*icon_itor);
						SetMinimapObjectListItem( pkSlot, kIconInfo );
						++icon_itor;
					}
					else
					{
						if(0 == iCount)
						{
							iCount = 0;
							pkItem = pkList->DeleteItem(pkItem);
							continue;
						}
					}
				}

				++iCount;
			}
			else
			{
				iCount = 0;
				pkItem = pkList->NextItem(pkItem);
			}
		}
		return true;
	}

	void CloseMinimapObjectList()
	{
		bool bClear = false;
		bClear |= XUIMgr.Close(L"SFRM_ACTOR_LIST_0");
		bClear |= XUIMgr.Close(L"SFRM_ACTOR_LIST");

		if( bClear )
		{
			ClearMinimapIconInfo();
		}
	}
	
	void SetMinimapObjectListItem(XUI::CXUI_Wnd* pkSlot, SMINIMAP_ICON_INFO const& kInfo)
	{
		if( !pkSlot ){ return; }

		lwUIWnd(pkSlot).SetCustomDataAsStr(MB(kInfo.kIconID));

		XUI::CXUI_Wnd* pkIcon = pkSlot->GetControl(L"IMG_ACTOR");
		if( pkIcon )
		{//얼굴 설정
			std::wstring	wstrIconID;
			SResourceIcon	rkRscIcon;
			bool bUseFace = false;
			if(g_kResourceIcon.GetIconIDFromActorName(kInfo.kIconID, wstrIconID))
			{
				if(g_kResourceIcon.GetIcon(rkRscIcon, wstrIconID))
				{
					std::wstring const sub = rkRscIcon.wstrImageID.substr(0, 13);
					if(sub.compare(NPC_ICON_TYPE_NAME) == 0)
					{
						SIconImage IconImage;
						if(true==g_kResourceIcon.GetIconImage(IconImage, rkRscIcon.wstrImageID))
						{
							bUseFace = true;
							pkIcon->DefaultImgName(IconImage.wstrPath);
							pkIcon->UVUpdate( rkRscIcon.iIdx );
						}
					}
				}
			}

			if( !bUseFace )
			{
				pkIcon->UVUpdate(114);
			}
		}

		XUI::CXUI_Wnd* pkName = pkSlot->GetControl(L"FRM_NAME");
		if( pkName )
		{//이름 설정
			pkName->Text(kInfo.kIconName);
		}		
	}
}