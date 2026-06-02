#ifndef FREEDOM_DRAGONICA_CONTENTS_SUPERGROUND_PGLEGENDITEMMAKEHELPMNG_H
#define FREEDOM_DRAGONICA_CONTENTS_SUPERGROUND_PGLEGENDITEMMAKEHELPMNG_H

//
typedef struct tagLegendItemHelpItem
{
	tagLegendItemHelpItem();
	tagLegendItemHelpItem(int const& riItemNo, int const& riCount);
	tagLegendItemHelpItem(tagLegendItemHelpItem const& rhs);
	~tagLegendItemHelpItem();

	bool ParseString(char const* szStr);

	int iItemNo;
	int iCount;
} SLegendItemHelpItem;

//
typedef enum eLegendItemMakeHelpStepType
{
	LIMHST_TRADE = 0,
	LIMHST_MAKE = 1,
} ELegendItemMakeHelpStepType;

typedef struct tagLegendItemMakeHelpStep
{
	tagLegendItemMakeHelpStep();
	tagLegendItemMakeHelpStep(tagLegendItemMakeHelpStep const& rhs);
	~tagLegendItemMakeHelpStep();

	bool ParseXml(TiXmlElement const* pkRoot);

	int iStepID;
	ELegendItemMakeHelpStepType eType;
	SLegendItemHelpItem kSrcItem1;
	SLegendItemHelpItem kSrcItem2;
	SLegendItemHelpItem kResultItem;
	int iExtVal;
	int iExtVal2;
} SLegendItemMakeHelpStep;
typedef std::map< int, SLegendItemMakeHelpStep > CONT_LEGEND_ITEM_MAKE_HELP_STEP;

//
typedef struct tagLegendItemMakeHelpInfo
{
	tagLegendItemMakeHelpInfo();
	tagLegendItemMakeHelpInfo(tagLegendItemMakeHelpInfo const& rhs);
	~tagLegendItemMakeHelpInfo();

	bool ParseXml(TiXmlElement const* pkRoot);

	int iItemNo;
	int iDescTextID;
	int iTitleTextID;
	CONT_LEGEND_ITEM_MAKE_HELP_STEP kContStep;
} SLegendItemMakeHelpInfo;
typedef std::map< int, SLegendItemMakeHelpInfo > CONT_LEGEND_ITEM_MAKE_HELP;


//
class PgLegendItemMakeHelpMng : public PgIXmlObject
{
public:
	PgLegendItemMakeHelpMng();
	~PgLegendItemMakeHelpMng();

	virtual bool ParseXml(const TiXmlNode *pkNode, void *pArg = 0, bool bUTF8 = false);

private:
	CONT_LEGEND_ITEM_MAKE_HELP m_kContMakeHelp;

protected:
	void SetMakeHelpUIData(XUI::CXUI_Wnd* pkTopWnd, const CONT_LEGEND_ITEM_MAKE_HELP_STEP::mapped_type &kCont);

public:
	void Clear();
	bool CallMakeHelpUI(const int iItemNo);
	bool CallMakeHelpUI(const int iItemNo, int iStepNo);
	void UpdateItemCount(lwUIWnd kWnd, const int iItemNo);
	int GetStepSize(const int iItemNo);

	static void lwUpdateItemCount( lwUIWnd kWnd, const int iItemNo);
	static void lwCallMakeHelpBigUI(const int iItemNo);
	static void lwCallMakeHelpSmallUI(const int iItemNo, const int iStepNo);
	static void lwCallHowToCurrentStep(lwUIWnd kTop);
	static void lwCallMakeHelpDetailDesc(lwUIWnd kWnd);
	static void RegisterWrapper(lua_State* pkState);
};

#define g_kLegendItemMakeHelpMng SINGLETON_STATIC(PgLegendItemMakeHelpMng)

#endif // FREEDOM_DRAGONICA_CONTENTS_SUPERGROUND_PGLEGENDITEMMAKEHELPMNG_H