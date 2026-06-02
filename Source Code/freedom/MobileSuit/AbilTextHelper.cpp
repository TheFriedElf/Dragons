#include "stdafx.h"
#include "AbilTextHelper.h"

// 템플릿 특수화 클래스 define 문
#define REPLACE_ABIL_VAL_TO_STR(OBJ, BM_STR, ABIL_TYPE)	int const iVal = OBJ.GetAbil(ABIL_TYPE); if( 0 != iVal ) { BM_STR.Replace(std::wstring(L"$" L#ABIL_TYPE L"$"), BM::vstring(iVal)); return true; };
#define DEF_REPLACE_ABIL_VAL_TO_STR_FUNC(ABIL_TYPE) template< >	struct ReplaceAbilValueToStr< ABIL_TYPE >	{ static bool Do(CAbilObject const& rkObj, BM::vstring& rkSrcString){REPLACE_ABIL_VAL_TO_STR(rkObj, rkSrcString, ABIL_TYPE); return false;} };
// 원본 템플릿 클래스 맴버 함수 구현 없음
template< EAbilType _ABIL_TYPE >		struct ReplaceAbilValueToStr { static bool Do(CAbilObject const& rkObj, BM::vstring& rkSrcString); }; // 원형은 구현부가 없는것이 맞다(링크 오류 유발)
// 정의
DEF_REPLACE_ABIL_VAL_TO_STR_FUNC( AT_USE_ITEM_CUSTOM_VALUE_1 )
DEF_REPLACE_ABIL_VAL_TO_STR_FUNC( AT_USE_ITEM_CUSTOM_VALUE_2 )
DEF_REPLACE_ABIL_VAL_TO_STR_FUNC( AT_USE_ITEM_CUSTOM_VALUE_3 )
DEF_REPLACE_ABIL_VAL_TO_STR_FUNC( AT_USE_ITEM_CUSTOM_VALUE_4 )

namespace AbilTextHelper
{
	bool ReplaceText(CItemDef const* pkItemDef, BM::vstring& rkTempMsg)
	{
		if( !pkItemDef )
		{
			return false;
		}

		CItemDef const& rkItemDef = *pkItemDef;
		bool bRet = ReplaceAbilValueToStr< AT_USE_ITEM_CUSTOM_VALUE_1 >::Do(rkItemDef, rkTempMsg);
		bRet = ReplaceAbilValueToStr< AT_USE_ITEM_CUSTOM_VALUE_2 >::Do(rkItemDef, rkTempMsg) || bRet; // 결과 누적
		bRet = ReplaceAbilValueToStr< AT_USE_ITEM_CUSTOM_VALUE_3 >::Do(rkItemDef, rkTempMsg) || bRet;
		bRet = ReplaceAbilValueToStr< AT_USE_ITEM_CUSTOM_VALUE_4 >::Do(rkItemDef, rkTempMsg) || bRet;
		return bRet; // true(바뀐것이 있다) / false(없다)
	}
}