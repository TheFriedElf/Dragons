#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMCOOKING_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMCOOKING_H

class lwUIItemCooking
{
public:
	lwUIItemCooking(lwUIWnd kWnd);
	static bool RegisterWrapper(lua_State *pkState);

public:
	void DisplaySrcIcon();
	void DisplayNeedItemIcon();
	void DisplayResultItem();
	void ClearCookingData();
	bool SendReqItemCooking(bool bIsTrueSend);
	int GetCookingNeedMoney();
	void CallComfirmMessageBox();
	void Clear();
	int CheckOK();	//클라에서 먼저 한번 체크해 주자
	int const GetNowNeedItemCount(int const iNeed) const;
	void SetNowCooking(bool bValue);
	bool NowCooking();
	int AddMakingCount(int i);
	int GetMakingCount();
	void SetUseOptionItem(bool const bUse);
	void NextResultItem();

protected:
	XUI::CXUI_Wnd *self;
};

class PgItemCookingMgr
{
public:
	typedef std::vector<std::pair<int,int> > CONT_RESULT;

	struct SData
	{
		virtual int NeedMoney()const = 0;
		virtual int NeedItemNo(int const idx)const = 0;
		virtual int NeedItemCount(int const idx)const = 0;
		virtual bool IsOptionItem(int const idx)const = 0;
		virtual int CheckNeedItem(PgInventory const* pkInv, int const iMakingCount, bool const bUseOptionItem)const = 0;
		virtual void GetResultItem(CONT_RESULT & kContOut, bool const bUseOptionItem)const = 0;
		virtual int MaximumCount(PgInventory const* pkInv, int const iRecipeCount, bool const bUseOptionItem)const = 0;		

		static int CalcMaximumCount(int iCount, PgInventory const* pkInv, int const iNeedItemNo, int const iNeedCount)
		{
			if(iNeedItemNo && iNeedCount)	//필요 아이템이 있으면
			{
				int const iTotalCount = pkInv->GetTotalCount(iNeedItemNo);
				if (iNeedCount*iCount > iTotalCount)
				{
					iCount = __min(iCount, iTotalCount/iNeedCount);
				}
			}
			return iCount;
		}
	};

	struct SCookingData : public SData
	{
		//virtual function
		int NeedMoney()const
		{
			return m_pkDef ? m_pkDef->iNeedMoney : 0;
		}

		int NeedItemNo(int const idx)const
		{
			if(m_kContNeedItem.size() > idx)
			{
				return m_kContNeedItem.at(idx).iItemNo;
			}
			return 0;
		}

		int NeedItemCount(int const idx)const
		{
			if(m_kContNeedItem.size() > idx)
			{
				return m_kContNeedItem.at(idx).iCount;
			}
			return 0;
		}

		bool IsOptionItem(int const idx)const
		{
			if(m_kContNeedItem.size() > idx)
			{
				return m_kContNeedItem.at(idx).bOptionItem;
			}
			return false;
		}

		int CheckNeedItem(PgInventory const* pkInv, int const iMakingCount, bool const bUseOptionItem)const
		{
			if(NULL == pkInv){ return 0; }
			for(CONT_NEEDITEM::const_iterator c_it=m_kContNeedItem.begin(); c_it!=m_kContNeedItem.end(); ++c_it)
			{
				if((false == (*c_it).bOptionItem) || (bUseOptionItem && (*c_it).bOptionItem))
				{
					if ((*c_it).iCount*iMakingCount > pkInv->GetTotalCount((*c_it).iItemNo))
					{
						return (*c_it).iItemNo;
					}
				}
			}
			return 0;
		}

		void GetResultItem(CONT_RESULT & kContOut, bool const bUseOptionItem)const
		{
			if(NULL == m_pkDef){ return; }
			for (int i = 0 ; i < TBL_DEF_COOKING::MAX_RESULT; ++i)
			{
				int const iRetItemNo = m_pkDef->kResult[i].iItemNo;
				int const iRetItemCount = m_pkDef->kResult[i].iCount;
				int const iRate = bUseOptionItem ? m_pkDef->kResult[i].iOptionRate : m_pkDef->kResult[i].iRate;
				if(iRetItemNo && iRetItemCount && iRate > 0)
				{
					kContOut.push_back(std::make_pair(m_pkDef->kResult[i].iItemNo,m_pkDef->kResult[i].iCount));
				}
			}
		}

		int MaximumCount(PgInventory const* pkInv, int const iRecipeCount, bool const bUseOptionItem)const
		{
			int iCount = iRecipeCount;
			if(pkInv)
			{
				for(CONT_NEEDITEM::const_iterator c_it=m_kContNeedItem.begin(); c_it!=m_kContNeedItem.end(); ++c_it)
				{
					if((false == (*c_it).bOptionItem) || (bUseOptionItem && (*c_it).bOptionItem))
					{
						iCount = SData::CalcMaximumCount(iCount, pkInv, (*c_it).iItemNo, (*c_it).iCount);
					}
				}
			}
			return iCount;
		}

		//custom function
		void SetDef(TBL_DEF_COOKING const* pkDef)
		{
			m_pkDef = pkDef;
			if(m_pkDef)
			{
				for (int i = 0; i < TBL_DEF_COOKING::MAX_NEEDITEM; ++i)
				{
					int const iNeedItemNo = m_pkDef->kNeedItem[i].iItemNo;
					int const iNeedItemCount = m_pkDef->kNeedItem[i].iCount;
					if(iNeedItemNo && iNeedItemCount)
					{
						m_kContNeedItem.push_back(SNeedItemData(iNeedItemNo, iNeedItemCount, false));
					}
				}

				int const iOptionItemNo = m_pkDef->kOptionItem.iItemNo;
				int const iOptionItemCount = m_pkDef->kOptionItem.iCount;
				if(iOptionItemNo && iOptionItemCount)
				{
					m_kContNeedItem.push_back(SNeedItemData(iOptionItemNo, iOptionItemCount, true));
				}
			}
		}

		//타입
		struct SNeedItemData
		{
			SNeedItemData(int const no=0, int const count=0, bool const option=false)
				: iItemNo(no), iCount(count), bOptionItem(option)
			{}
			int iItemNo;
			int iCount;
			bool bOptionItem;
		};
		typedef std::vector<SNeedItemData> CONT_NEEDITEM;

		//맴버변수
		TBL_DEF_COOKING const* m_pkDef;
		CONT_NEEDITEM m_kContNeedItem;
	};

	struct SMakingData : public SData
	{
		int NeedMoney()const
		{
			return m_kMakingData.iNeedMoney;
		}

		int NeedItemNo(int const idx)const
		{
			if (MAX_SUCCESS_RATE_ARRAY > idx)
			{
				return m_kMakingData.kNeedElements.aElement[idx];
			}
			return 0;
		}

		int NeedItemCount(int const idx)const
		{
			if (MAX_SUCCESS_RATE_ARRAY > idx)
			{
				return m_kMakingData.kNeedCount.aCount[idx];
			}
			return 0;
		}

		bool IsOptionItem(int const idx)const
		{
			return false;
		}

		int CheckNeedItem(PgInventory const* pkInv, int const iMakingCount, bool const bUseOptionItem)const
		{
			if(NULL == pkInv){ return 0; }
			for (int i = 0; i < MAX_SUCCESS_RATE_ARRAY; ++i)
			{
				int const iNeedItemNo = m_kMakingData.kNeedElements.aElement[i];
				int const iNeedCount = m_kMakingData.kNeedCount.aCount[i];
				if(iNeedItemNo)	//필요 아이템이 있으면
				{
					if (iNeedCount*iMakingCount > pkInv->GetTotalCount(iNeedItemNo))
					{
						return iNeedItemNo;
					}
				}
			}
			return 0;
		}

		void GetResultItem(CONT_RESULT & kContOut, bool const bUseOptionItem)const
		{
			for (int i=0 ; i<MAX_ITEMMAKING_ARRAY; i++)
			{
				for (int j=0 ; j<MAX_SUCCESS_RATE_ARRAY ; j++)
				{
					kContOut.push_back(std::make_pair(m_kMakingData.akResultItemElements[i].aElement[j], m_kMakingData.akResultItemCount[i].aCount[j]));
					i = MAX_ITEMMAKING_ARRAY;
					j = MAX_SUCCESS_RATE_ARRAY;
				}
			}
		}

		int MaximumCount(PgInventory const* pkInv, int const iRecipeCount, bool const bUseOptionItem)const
		{
			int iCount = iRecipeCount;
			if(pkInv)
			{
				for (int i = 0; i < MAX_SUCCESS_RATE_ARRAY; ++i)
				{
					iCount = SData::CalcMaximumCount(iCount, pkInv, m_kMakingData.kNeedElements.aElement[i], m_kMakingData.kNeedCount.aCount[i]);
				}
			}
			return iCount;
		}

		SDefItemMakingData m_kMakingData;
	};

	typedef std::tr1::shared_ptr<SData> SP_CookingData;

public:
	PgItemCookingMgr();
	virtual ~PgItemCookingMgr(){}

public:
	void Clear();
	int CallComfirmMessageBox();
	int GetCookingNeedMoney();
	bool SetSrcItem(const SItemPos &rkItemPos);
	void SetUseOptionItem(XUI::CXUI_Wnd *pWnd, bool const bUse);
	void NextResultItem();
	void DisplayNeedItemIcon(int const iNeedIndex, XUI::CXUI_Wnd *pWnd);
	void DisplaySrcItem(XUI::CXUI_Wnd *pWnd);
	void DisplayResultItem(XUI::CXUI_Wnd *pWnd);

	bool SendReqItemCooking(bool bIsTrueSend);
	int CheckNeedItem();
	int const GetNowNeedItemCount(int const iNeed) const;
	int MaximumCount();

protected:
	void UpdateResultItem();
	void NextDisplayResultItem(DWORD const dwNow);

public:
	CLASS_DECLARATION_S( int, CurMakingNo );
	CLASS_DECLARATION_S( int, CurMakingType );
	CLASS_DECLARATION_S( bool, UseOptionItem );
	CLASS_DECLARATION_S( bool, NowCooking );
	CLASS_DECLARATION_S( int, MakingCount );	//한번에 몇개 만들거냐
	CLASS_DECLARATION_S( int, RecipeCount );	//같고있는 총 래시피 갯수
	
protected:
	BM::GUID m_guidSrcItem;//아이템 위치가 변경되거나 할 수 있으므로.
	SItemPos m_kSrcItemPos;

	PgBase_Item m_kItem;
	//SItem	m_kResultItem;
	CONT_RESULT m_kContResultItem;
	int m_iDisplayResultItemIdx;
	int m_iDisplayResultItemTime;

	SP_CookingData m_spCookingData;		// 요리의 데이터.
	//SNeedItemPlusUpgrade m_kItemArray[MAX_ITEM_PLUS_UPGRADE_NEED_ARRAY+1];
};

#define g_kItemCookingMgr SINGLETON_STATIC(PgItemCookingMgr)
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIITEMCOOKING_H