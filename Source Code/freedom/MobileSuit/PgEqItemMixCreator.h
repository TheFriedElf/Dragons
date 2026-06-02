#ifndef FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGEQITEMMIXCREATOR_H
#define FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGEQITEMMIXCREATOR_H

#include "PgItemMixBase.h"

extern int const DEF_EQ_MIX_INSURANCE_ITEM_NO;
extern int const DEF_EQ_MIX_PROB_ITEM_NO;

class PgEqItemMixCreator
	: public PgItemMixBase
{
public:
	bool SetRecipe(DWORD const iItemNo);
	virtual bool SetRecipe(SItemPos const kItemPos);
	virtual void RequestMix();
	bool SetInsurance(bool const bUICall = false);
	void SetInsurance(SItemPos const& kItemPos);
	bool SetProbAbility(bool const bUICall = false);
	void SetProbAbility(SItemPos const& kItemPos);
	virtual void Clear();

	void ClearInsurance() { m_kInsurance.Clear(); };
	void ClearProbAbility() { m_kProbAbility.Clear(); };
	S_MIX_ITEM_INFO const& GetInsurance() const { return m_kInsurance; };
	S_MIX_ITEM_INFO const& GetProbAbility() const { return m_kProbAbility; };

	PgEqItemMixCreator(void);
	virtual ~PgEqItemMixCreator(void);

protected:

	virtual bool CheckMixType(int const iType) const
	{
		return ((iType & EMAKING_TYPE_MAZE_ITEM) == EMAKING_TYPE_MAZE_ITEM );
	}

	S_MIX_ITEM_INFO	m_kInsurance;
	S_MIX_ITEM_INFO	m_kProbAbility;
};

#define g_kEqItemMixCreator SINGLETON_STATIC(PgEqItemMixCreator)

class PgCommonTypeMixer
	: public PgItemMixBase
{
public:
	enum E_MIX_TYPE
	{
		MT_NONE			= 0,
		MT_MONSTER_CARD	= 1,
		MT_COMPOSITE	= 2,
		MT_END,
	};

	enum E_MIX_ERR
	{
		ME_OK				= 0,
		ME_NO_REG_RECIPE	= 1,
		ME_MATERIAL_LACK	= 2,
		ME_MONEY_LACK,
		ME_END,
	};

	struct SMIXER_INIT_INFO
	{
		SMIXER_INIT_INFO()
			: kAddonText(), iTitleTT(0), iRegistSlotTT(0), iResultSlotTT(0), iOKButtonTT(0)
		{
			for(int i = 0; i < ME_END; ++i )
			{
				iErrTT[i] = 0;
			}		
		}

		void SetBaseInfo(std::wstring const& kAddText, int const iTitleNo, int const iRegSlotNo, int const iRstSlotNo, int const iOKNo)
		{
			kAddonText = kAddText;
			iTitleTT = iTitleNo;
			iRegistSlotTT = iRegSlotNo;
			iResultSlotTT = iRstSlotNo;
			iOKButtonTT = iOKNo;
		}

		std::wstring kAddonText;
		int iTitleTT;
		int iRegistSlotTT;
		int iResultSlotTT;
		int iOKButtonTT;
		int iErrTT[ ME_END ];
	};

	typedef std::map< E_MIX_TYPE, SMIXER_INIT_INFO > CONT_MIXER_INITINFO;

	virtual bool SetRecipe(SItemPos const kItemPos);
	virtual void RequestMix();
	
	bool GetMixerInitInfo(SMIXER_INIT_INFO& kInfo) const;
	E_MIX_TYPE const& MixType() const { return m_kMixType; };

	PgCommonTypeMixer(void);
	virtual ~PgCommonTypeMixer(void);

protected:
	virtual bool CheckMixType(int const iType) const;

	mutable E_MIX_TYPE m_kMixType;
	static CONT_MIXER_INITINFO m_kContMixerInitInfo;
};

#define g_kCommonTypeMixer SINGLETON_STATIC(PgCommonTypeMixer)

#endif // FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGEQITEMMIXCREATOR_H