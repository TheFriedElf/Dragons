#ifndef FREEDOM_DRAGONICA_UI_PGUICALCULATOR_H
#define FREEDOM_DRAGONICA_UI_PGUICALCULATOR_H

typedef enum ECalcInfoType//돈 계산 정보
{
	CCT_NONE = 0,
	CCT_SHOP_BUY = 1,
	CCT_SHOP_SELL = 2,
	CCT_LOTTERY = 3,
	CCT_ITEM_DIVIDE = 4,
	CCT_MARKET_BUY = 5,
	CCT_EMWEEK = 6,
	CCT_STOCK_SHOP_SELL = 7,
	CCT_EMPORIA_WEEK_SELECT = 8,
	CCT_VENDOR_BUY = 9,
}eCalcInfoType;

#pragma pack(1)
typedef struct tagCalcInfo
{
	tagCalcInfo()
	{
		eCallType = CCT_NONE;
		iBasePrice = 0i64;
		iItemNo = 0;
		iIndex = 0;
		iMaxValue = 100;
		cCostType = 0;
	}

	ECalcInfoType eCallType;
	__int64 iBasePrice;
	BM::GUID kGuid;//여러가지로 사용될 Custom Guid
	BM::GUID kGuid1;//여러가지로 사용될 Custom Guid
	int iItemNo;
	SItemPos kItemPos;
	int iMaxValue;
	int iIndex;
	BYTE cCostType;
}SCalcInfo;
#pragma pack()

extern void CallCalculator(const SCalcInfo &kInfo);




#pragma pack(1)
typedef struct tagSMSInfo
{
	tagSMSInfo()
	{
		iItemNo = 0;
		iValue = 0;
		iValue2 = 0;
	}

	BM::GUID kItemGuid;
	int iItemNo;
	int iValue;
	int iValue2;
	SItemPos kItemPos;
}SSMSInfo;
#pragma pack()

extern void CallSMS(const tagSMSInfo &kInfo);//문자 보내기.
#endif // FREEDOM_DRAGONICA_UI_PGUICALCULATOR_H