/* button.c */
#include "button.h"

#define LONG_PRESS_MS 2000  // 길게 누르기 기준
#define DEBOUNCE_MS   50    // 디바운싱 기준

void Button_Init(Button_t *pBtn, GPIO_TypeDef *port, uint16_t pin,
		GPIO_PinState onState) {
	pBtn->port = port;
	pBtn->pin = pin;
	pBtn->onState = onState;
	pBtn->prevTime = 0;
	pBtn->pressStartTime = 0;
	pBtn->isPressed = false;
	pBtn->longPressHandled = false;
}

ButtonEvent_t Button_GetEvent(Button_t *pBtn) {
	GPIO_PinState currentState = HAL_GPIO_ReadPin(pBtn->port, pBtn->pin);
	uint32_t currTime = HAL_GetTick();

	// 1. 버튼이 눌려있는 상태
	if (currentState == pBtn->onState) {

		// 1-1. 디바운싱 체크 (노이즈가 아닌 진짜 눌림인지)
		if (currTime - pBtn->prevTime > DEBOUNCE_MS) {

			// 1-2. 방금 막 눌린 경우 (Rising Edge)
			if (pBtn->isPressed == false) {
				pBtn->isPressed = true;          // 상태 변경
				pBtn->pressStartTime = currTime; // 누른 시각 저장
				pBtn->longPressHandled = false;  // 롱프레스 플래그 초기화

				return BTN_PRESSED; //버튼 눌림 감지
			}

			// 1-3. 계속 누르고 있는 경우
			else {
				// 길게 누르기 시간이 지났고 && 아직 처리가 안 됐다면
				if (!pBtn->longPressHandled
						&& (currTime - pBtn->pressStartTime > LONG_PRESS_MS)) {
					pBtn->longPressHandled = true;
					return BTN_LONG; //길게 누르기 감지
				}
			}
		}
	}
	// 2. 버튼이 떨어져 있는 상태
	else {
		pBtn->prevTime = currTime; // 버튼이 떨어진 시점을 계속 갱신 (디바운싱 기준점)

		// 2-1. 방금 막 뗀 경우 (Falling Edge)
		if (pBtn->isPressed == true) {
			pBtn->isPressed = false; // 상태 해제

			// 2-2. 길게 누르기가 처리된 적이 없다면 -> 짧게 누르고 뗀 것
			if (pBtn->longPressHandled == false) {
				return BTN_RELEASED; //버튼 뗌 감지 (실제 동작 실행)
			}
			// 길게 누르기가 처리되었다면 -> 아무것도 반환하지 않음
		}
	}

	return BTN_NONE;
}
