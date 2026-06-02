
#include "stdafx.h"
#include "lwUIGamePad.h"
#include "PgLocalManager.h"
#include "PgInputSystem.h"

lwWString lwGamePadUI::m_kPressedButtonName = "";
int lwGamePadUI::m_iUnbindedButtonIndex = 0;
unsigned int  lwGamePadUI::m_iPressedButtonIndex = 0;
int lwGamePadUI::m_iPressedUkey = 0;

lwGamePadUI	GetGamePadUI()
{
	return	lwGamePadUI();
}
lwGamePadUI::lwGamePadUI()
{
}

void lwGamePadUI::RegisterWrapper(lua_State *pkState)
{
	using namespace lua_tinker;

	def(pkState, "GetGamePadUI", &GetGamePadUI);
	class_<lwGamePadUI>(pkState, "GamePadUI")
		.def(pkState, constructor<void>())
		.def(pkState, "IsPadExist", &lwGamePadUI::IsPadExist)
		.def(pkState, "RecordPressedButtonName", &lwGamePadUI::RecordPressedButtonName)
		.def(pkState, "GetPressedButtonName", &lwGamePadUI::GetPressedButtonName)
		.def(pkState, "RecordPressedButtonIndex", &lwGamePadUI::RecordPressedButtonIndex)
		.def(pkState, "GetPressedButtonIndex", &lwGamePadUI::GetPressedButtonIndex)
		.def(pkState, "GetUnbindedUIButtonIndex", &lwGamePadUI::GetUnbindedUIButtonIndex)
		.def(pkState, "RecordPressedUkey", &lwGamePadUI::RecordPressedUkey)
		.def(pkState, "GetPressedUkey", &lwGamePadUI::GetPressedUkey)
		.def(pkState, "EraseUnbindButtonInfo", &lwGamePadUI::EraseUnbindButtonInfo)
		.def(pkState, "UnbindButton", &lwGamePadUI::UnbindButton)
		.def(pkState, "BackupSetting", &lwGamePadUI::BackupSetting)
		.def(pkState, "RestoreSetting", &lwGamePadUI::RestoreSetting)
		.def(pkState, "GetPadButtonBindWithUIButton", &lwGamePadUI::GetPadButtonBindWithUIButton)
		.def(pkState, "GetUIButtonIndexByPressedPadButton", &lwGamePadUI::GetUIButtonIndexByPressedPadButton)
		.def(pkState, "GetPadButtonNameOnTextTableAndBindPadButton", &lwGamePadUI::GetPadButtonNameOnTextTableAndBindPadButton)
		.def(pkState, "SetAxisSensitivityThreshold", &lwGamePadUI::SetAxisSensitivityThreshold)
		;
}

PgInputSystem* lwGamePadUI::GetInput()
{
	return NiDynamicCast(PgInputSystem, g_pkLocalManager->GetInputSystem());
}

//패드가 있는지 검사 / 패드가 없으면 UI를 닫기위해
bool lwGamePadUI::IsPadExist()
{
	if(NULL != GetInput())
	{
		return NULL != GetInput()->GetGamePad(0);
	}
	return false;
}

//버튼네임(게임패드 버튼이 아닌 UI윈도우의 버튼 이름) 저장
void lwGamePadUI::RecordPressedButtonName(lwWString kButtonName)
{
	m_kPressedButtonName = kButtonName;
}

//버튼네임(게임패드 버튼이 아닌 UI윈도우의 버튼 이름) 가져오기
lwWString lwGamePadUI::GetPressedButtonName() const
{
	return m_kPressedButtonName;
}

//가장 최근에 눌린 버튼(게임패드 버튼이 아닌 UI윈도우의 버튼 이름)의 인덱스를 저장한다.
void lwGamePadUI::RecordPressedButtonIndex(unsigned int iButtonIndex)
{
	m_iPressedButtonIndex = iButtonIndex;
}

//가장 최근에 눌린 버튼(게임패드 버튼이 아닌 UI윈도우의 버튼 이름)의 인덱스를 가져온다.
unsigned int lwGamePadUI::GetPressedButtonIndex() const
{
	return m_iPressedButtonIndex;
}

//가장 최근에 눌린 UI버튼의 Ukey를 저장한다.
void lwGamePadUI::RecordPressedUkey(int iUkey)
{
	m_iPressedUkey = iUkey;
}

//가장 최근에 눌린 UI버튼의 Ukey를 가져온다.
int lwGamePadUI::GetPressedUkey() const
{
	return m_iPressedUkey;
}

int lwGamePadUI::GetUnbindedUIButtonIndex() const
{
	return m_iUnbindedButtonIndex;
}

//언바인드된 UI버튼인덱스를 리셋한다.
void lwGamePadUI::EraseUnbindButtonInfo()
{
	m_iUnbindedButtonIndex = 0;
}

//패드 셋팅을 백업한다.(취소버튼을 누를경우 복구하기 위해)
void lwGamePadUI::BackupSetting()
{
	if(NULL == GetInput())
	{
		return;
	}
	
	GetInput()->BackupPadSettingTable();
}

//패드 셋팅을 복구한다.
void lwGamePadUI::RestoreSetting()
{
	if(NULL == GetInput())
	{
		return;
	}
	
	GetInput()->RestorePaddSettingTable();
}

void lwGamePadUI::SetAxisSensitivityThreshold(float fValue)
{
	if(NULL == GetInput())
	{
		return;
	}
	
	GetInput()->SetGamePadAxisSensitivityThreshold(fValue);
}

//UI버튼에 바인드된 패드버튼이름을 가져온다.
lwWString lwGamePadUI::GetPadButtonBindWithUIButton(int iButtonIndex)
{
	if(NULL == GetInput())
	{
		return "";
	}
	int const iNoneButtonCode = 90001001;
	int iButtonCode =  GetInput()->GetPadButtonRefPressedIndex(iButtonIndex);
	return lwWString(TTW(iNoneButtonCode + iButtonCode));
}

//UI버튼 인덱스를 기반으로 바인드된 패드버튼을 찾아 바인딩정보를 삭제한다.
void lwGamePadUI::UnbindButton(int iUIButtonIndex)
{
	if(NULL == GetInput())
	{
		return;
	}

	for(int iIndex=NiInputGamePad::NIGP_LUP ; NiInputGamePad::NIGP_NUMBUTTONS > iIndex ; ++iIndex)
	{
		if(iUIButtonIndex == GetInput()->GetUIButtonIndex(static_cast<NiInputGamePad::Button>(iIndex)))
		{
			GetInput()->BindButtonPairWithNiKeycode(
				static_cast<NiInputGamePad::Button>(iIndex), 
				0, 0);
		}
	}
}

//UI에서 패드버튼이 어느 UI버튼에 바인딩 되었는지 식별하기 위해 UI버튼 인덱스를 찾아오는 함수
int lwGamePadUI::GetUIButtonIndexByPressedPadButton()
{
	if(NULL == GetInput())
	{
		return 0;
	}
	if(NULL == GetInput()->GetGamePad(0))
	{
		return 0;
	}
	unsigned int iCurrentButtons = GetInput()->GetGamePad(0)->GetCurrentButtons();
	for(int iIndex=NiInputGamePad::NIGP_LUP ; NiInputGamePad::NIGP_NUMBUTTONS > iIndex ; ++iIndex)
	{//총 버튼 갯수만큼
		if(iCurrentButtons & (1 << iIndex) || GetInput()->GetAxisToButtonPressed(static_cast<NiInputGamePad::Button>(iIndex))) //이 검사식은 NiInputGamePad::ButtonMask에 기반한다.
		{//현재패드버튼 중 눌린 걸 찾는다.
			//눌린 버튼이 바인드 된 UI버튼의 인덱스를 가져온다.
			return GetInput()->GetUIButtonIndex(static_cast<NiInputGamePad::Button>(iIndex));
		}
	}
	return 0;
}

//리스너UI에서 주기적으로 호출되면서
//패드버튼의 입력을 검출하고 패드버튼의 이름을 텍스트테이블에서 얻어오는 함수
//여기서 얻어온 패드버튼의 이름을 UI버튼의 텍스트로 대체
lwWString lwGamePadUI::GetPadButtonNameOnTextTableAndBindPadButton()
{
	if(NULL == GetInput())
	{
		return lwWString("");
	}
	if(NULL == GetInput()->GetGamePad(0))
	{
		return "";
	}

	unsigned int iCurrentButtons = GetInput()->GetGamePad(0)->GetCurrentButtons();
	unsigned int const iNoneButtonCode = 90001001; //텍스트 테이블의 BUTTON없음 코드

	for(int iIndex=NiInputGamePad::NIGP_LUP ; NiInputGamePad::NIGP_NUMBUTTONS > iIndex ; ++iIndex)
	{//총 버튼 갯수만큼
		if(iCurrentButtons & (1 << iIndex) || GetInput()->GetAxisToButtonPressed(static_cast<NiInputGamePad::Button>(iIndex))) //이 검사식은 NiInputGamePad::ButtonMask에 기반한다.
		{//현재패드버튼 중 눌린 걸 찾는다.
			//눌린 패드 버튼을 UI버튼에 해당하는 키보드키와 매핑한다.
			//여기서 바인딩을 하게 되는데....
			
			//눌린 패드 버튼이 바인드되어있는지 검사한다.
			int iBindedUIButton = GetInput()->IsAlreadyBind(static_cast<NiInputGamePad::Button>(iIndex));
			//찾아낸 키가 기존에 쓰던 패드 버튼이 있으면
			if(0 != iBindedUIButton)
			{
				//바인드가 해제된 키의 정보(바인드가 해제된 UI버튼)를 기록한다
				m_iUnbindedButtonIndex = iBindedUIButton;
				//바인드가 지워진 UI버튼에 바인드되어있던 패드버튼의 바인드를 지운다
				//패드버튼은 최근에 눌린 UI버튼의 인덱스로 찾는다.
				NiInputGamePad::Button kTargetButton = GetInput()->GetPadButtonRefPressedIndex(m_iPressedButtonIndex);
				GetInput()->BindButtonPairWithNiKeycode(kTargetButton, 0, 0);

			}
			//찾아낸 키를 바인딩
			GetInput()->BindButtonPairWithNiKeycode(
				static_cast<NiInputGamePad::Button>(iIndex), 
				m_iPressedUkey, m_iPressedButtonIndex);


			//결과를 리턴한다 (BUTTON1~16)
			return lwWString(TTW(iNoneButtonCode + iIndex));
		}
	}

	return lwWString("");
}