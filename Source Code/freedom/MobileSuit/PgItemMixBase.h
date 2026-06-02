#ifndef FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGITEMMIXBASE_H
#define FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGITEMMIXBASE_H

enum E_MIX_ERRROR
{
	EMIX_ERROR_RECIPE_TYPE
};

enum E_ITEM_TYPE
{
	EIT_RECIPE		= 0,
	EIT_PROBABILITY	= 1,
	EIT_INSURANCE,
	EIT_MATERIAL,
};

enum E_MIX_TYPE
{
	EMT_NONE				= 0,
	EMT_MONSTER_CARD_MIXER	= 1,
	EMT_END,
};

struct S_MIX_ITEM_INFO
{
	S_MIX_ITEM_INFO()
		: kItemNo(0), kItemPos(), iHave(0), iNeed(0)
	{};
	explicit S_MIX_ITEM_INFO(DWORD const dwItemNo)
		: kItemNo(dwItemNo), kItemPos(), iHave(0), iNeed(0)
	{};
	void Clear()
	{
		kItemNo = 0;
		kItemPos.Clear();
		iHave = 0;
		iNeed = 0;
	};
	bool IsEmpty() const
	{
		return (kItemNo == 0);
	};
	DWORD		kItemNo;
	SItemPos	kItemPos;
	int			iHave;
	int			iNeed;
};

typedef std::map< DWORD, S_MIX_ITEM_INFO > CONT_MATERIAL;

class PgIMixBase
{
public:
	virtual E_MIX_TYPE const Type() const { return EMT_NONE; }
	virtual void RequestMix() = 0;
	virtual void Clear();

	PgIMixBase();
	virtual ~PgIMixBase();

protected:
	virtual bool SetMaterialInfo(DWORD const kMakingItem) = 0;
	virtual bool CheckMixType(int const iType) const = 0;

protected:
	CLASS_DECLARATION_S(bool, LackMaterial);
	CLASS_DECLARATION_S(bool, bMixing);
	CLASS_DECLARATION_S(int, Cost);
	CLASS_DECLARATION_S_NO_SET(DWORD, RewordItemNo);
	CLASS_DECLARATION_NO_SET(int, m_iMakingNo, MakingNo);
};

class PgItemMixBase : public PgIMixBase
{
public:
	virtual bool SetRecipe(SItemPos const kItemPos);
	virtual void Clear();

	S_MIX_ITEM_INFO const& GetRecipe() const { return m_kRecipe; };
	CONT_MATERIAL const& GetMaterialInfo() const { return m_kMaterialContainer; };	

	PgItemMixBase(void);
	virtual ~PgItemMixBase(void);

protected:
	bool SetMaterial(DWORD const dwItemNo);
	bool GetItemInfo(S_MIX_ITEM_INFO& kItemInfo);

	bool AutoEntryMaterial();

	virtual bool SetMaterialInfo(DWORD const kMakingItem);

	CLASS_DECLARATION_S(int, MakeCount);
protected:
	CONT_MATERIAL	m_kMaterialContainer;
	S_MIX_ITEM_INFO	m_kRecipe;
};

class PgItemMixMgr
{
public:
	typedef std::map< int, PgIMixBase* >	CONT_ITEM_MIXER;

	PgItemMixMgr();
	~PgItemMixMgr();

public:
	void Clear();
	bool CreateMixer(E_MIX_TYPE const& MixType);
	PgIMixBase* GetMixer(E_MIX_TYPE const& MixType);
	void DeleteMixer(E_MIX_TYPE const& MixType);

protected:
	PgIMixBase* NewMixer(E_MIX_TYPE const& MixType);

private:
	CONT_ITEM_MIXER	m_kContMixer;
};

#define g_kItemMixMgr	SINGLETON_STATIC(PgItemMixMgr)

#endif // FREEDOM_DRAGONICA_CONTENTS_ITEMMIXING_PGITEMMIXBASE_H