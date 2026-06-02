
/*
게임패드UI 기능 제공 클래스
관련 파일 XML/UI/GamePadUI.xml
*/

#ifndef FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIGAMEPAD_H
#define FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIGAMEPAD_H

#include "PgScripting.h"
#include "lwWString.h"

class PgInputSystem;

class	lwGamePadUI
{
private:
	//가장 최근에 눌린 UI버튼의 이름(상,하,단축슬롯1 등등...)
	static lwWString m_kPressedButtonName;
	//가장 최근에 눌린 UI버튼의 인덱스 = BTN_BUTTON(N)
	static unsigned int m_iPressedButtonIndex;
	//가장 최근에 눌린 UI버튼에 물린 기능에 해당하는 UKey
	static int m_iPressedUkey;
	//바인드가 제거된 UI버튼의 이름
	static int m_iUnbindedButtonIndex;
	

public:
	lwGamePadUI();

	static void RegisterWrapper(lua_State *pkState);
	
	PgInputSystem* GetInput();

	//패드가 있는지 검사 / 패드가 없으면 UI를 닫기위해
	bool IsPadExist();

	//가장 최근에 눌린 UI버튼의 이름을 저장한다.
	void RecordPressedButtonName(lwWString kButtonName);
	
	//가장 최근에 눌린 UI버튼의 이름을 가져온다.
	lwWString GetPressedButtonName() const;
	
	//가장 최근에 눌린 UI버튼의 인덱스를 저장한다.
	void RecordPressedButtonIndex(unsigned int iButtonIndex);
	
	//가장 최근에 눌린 UI버튼의 인덱스를 가져온다.
	unsigned int GetPressedButtonIndex() const;

	//가장 최근에 눌린 UI버튼의 Ukey를 저장한다.
	void RecordPressedUkey(int iUkey);
	
	//가장 최근에 눌린 UI버튼의 Ukey를 가져온다.
	int GetPressedUkey() const;

	//바인드가 제거된 UI버튼의 이름을 가져옴
	//중복된 바인드의 제거는 아래 GetPadButtonNameOnTextTableBindPadButton에서 한다.
	int GetUnbindedUIButtonIndex() const;

	//UI버튼 인덱스를 기반으로 바인드된 패드버튼을 찾아 바인딩정보를 삭제한다.
	void UnbindButton(int iUIButtonIndex);

	//UI에서 패드버튼이 어느 UI버튼에 바인딩 되었는지 식별하기 위해 UI버튼 인덱스를 찾아오는 함수
	int GetUIButtonIndexByPressedPadButton();

	//언바인드된 UI버튼인덱스를 리셋한다.
	void EraseUnbindButtonInfo();

	//패드 셋팅을 백업한다.(취소버튼을 누를경우 복구하기 위해)
	void BackupSetting();

	//패드 셋팅을 복구한다.
	void RestoreSetting();

	void SetAxisSensitivityThreshold(float fValue);

	//UI버튼에 바인드된 패드버튼이름을 가져온다.
	lwWString GetPadButtonBindWithUIButton(int iButtonIndex);
	
	//리스너UI에서 주기적으로 호출되면서
	//패드버튼의 입력을 검출하고 패드버튼의 이름을 텍스트테이블에서 얻어오는 함수
	//여기서 얻어온 패드버튼의 이름을 UI버튼의 텍스트로 대체
	lwWString GetPadButtonNameOnTextTableAndBindPadButton();
};
#endif // FREEDOM_DRAGONICA_SCRIPTING_UI_LWUIGAMEPAD_H