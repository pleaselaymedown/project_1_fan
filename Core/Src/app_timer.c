/* app_timer.c */
#include "app_timer.h"
#include <stdlib.h>

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim10;

// 핀 정의 구조체
typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
} PinDef_t;

// FND 핀
static const PinDef_t fndSel[4] = { { GPIOC, GPIO_PIN_10 },
		{ GPIOC, GPIO_PIN_11 }, { GPIOC, GPIO_PIN_12 }, { GPIOD, GPIO_PIN_2 } };

static const PinDef_t fndSeg[7] = { { GPIOB, GPIO_PIN_6 },
		{ GPIOC, GPIO_PIN_7 }, { GPIOA, GPIO_PIN_8 }, { GPIOB, GPIO_PIN_10 }, {
				GPIOB, GPIO_PIN_4 }, { GPIOB, GPIO_PIN_5 },
		{ GPIOB, GPIO_PIN_3 } };

static const uint8_t fndData[10] = { 0b11000000, 0b11111001, 0b10100100,
		0b10110000, 0b10011001, 0b10010010, 0b10000010, 0b11111000, 0b10000000,
		0b10010000 };

void AppTimer_Start(AppTimer_t *pTimer) {
	HAL_TIM_Base_Start_IT(&htim1);
	pTimer->isRunning = true;
}

void AppTimer_Stop(AppTimer_t *pTimer) {
	HAL_TIM_Base_Stop_IT(&htim1);
	pTimer->timeRemaining = 0;
	pTimer->digitCount = 0;
	pTimer->isRunning = false;
	pTimer->timerIsSettingByButton = false;
	pTimer->timerIsSettingByRemote = false;
	pTimer->particularSetting = false;
}

void AppTimer_Init(AppTimer_t *pTimer, FanController_t *pFan) {
	pTimer->timeRemaining = 0;
	pTimer->digitCount = 0;
	pTimer->isRunning = false;
	pTimer->digitSelect = 0;
	pTimer->pFanCtrl = pFan;
	pTimer->blinkCounter = 0;
	pTimer->timerIsSettingByButton = false;
	pTimer->timerIsSettingByRemote = false;
	pTimer->particularSetting = false;
}

void AppTimer_ProcessInput(AppTimer_t *pTimer, char c) {
	if (pTimer->timerIsSettingByButton)
		return;

	if (c >= '0' && c <= '9') {
		if (pTimer->isRunning || pTimer->pFanCtrl->powerIdx <= 0)
			return; //선풍기가 꺼져있거나 타이머가 진행중일 경우 타이머 설정 불가

		if (pTimer->digitCount < 4) {
			pTimer->timeRemaining = pTimer->timeRemaining * 10 + (c - '0');
			pTimer->digitCount++;
		}
	} else if (c == '*') {
		pTimer->digitCount = 0;
		if (pTimer->isRunning)
			AppTimer_Stop(pTimer);
		else
			AppTimer_Start(pTimer);
	}
}

void AppTimer_ProcessButton(AppTimer_t *pTimer, ButtonInput btn) {
	// 1. 차단 조건 (리모컨 설정 중이거나 팬이 꺼져있으면 무시)
	// 단, 팬이 꺼져있어도 isRunning 체크를 먼저 해서 정지시키는 로직이 필요할 수 있으나,
	// 요구사항 순서상 "리모컨 설정중" -> "isRunning 검사" -> "Fan Power 검사" 순으로 배치합니다.

	if (pTimer->timerIsSettingByRemote)
		return;

	// 2. 타이머 동작 중일 때 처리
	if (pTimer->isRunning) {
		if (btn == NEXT) { // OK 버튼이 들어오면 타이머 중지
			AppTimer_Stop(pTimer);
		}
		return; // 동작 중에는 다른 버튼 무시
	}

	// 3. 팬이 꺼져있으면 설정 진입 불가
	if (pTimer->pFanCtrl->powerIdx <= 0)
		return;

	// 4. 타이머 설정 모드 (버튼)
	if (pTimer->timerIsSettingByButton) {

		if (pTimer->particularSetting) {
			// ==========================
			// [상세 설정 모드] (Digit-by-Digit)
			// ==========================
			switch (btn) {
			case INCREASE:
				// 현재 자리수(1의 자리) 증가 (0~9 순환)
				// (timeRemaining / 10 * 10) : 10의 자리 이상 유지
				// (timeRemaining % 10 + 1) % 10 : 1의 자리만 1 증가 후 0~9 wrap
				pTimer->timeRemaining = (pTimer->timeRemaining / 10 * 10)
						+ ((pTimer->timeRemaining % 10 + 1) % 10);
				break;

			case DECREASE:
				// 현재 자리수(1의 자리) 감소 (0~9 순환)
				pTimer->timeRemaining = (pTimer->timeRemaining / 10 * 10)
						+ ((pTimer->timeRemaining % 10 + 9) % 10);
				break;

			case NEXT: // OK 버튼
				// 4번째 자리(digitCount 3) 설정 중 OK 누르면 완료
				if (pTimer->digitCount >= 3) {
					pTimer->timerIsSettingByButton = false;
					pTimer->particularSetting = false;
					AppTimer_Start(pTimer);
				} else {
					// 다음 자리로 이동 (기존 값에 10 곱하기)
					pTimer->timeRemaining *= 10;
					pTimer->digitCount++;
				}
				break;

			case DONE: // 설정 완료 (즉시 시작)
				pTimer->timerIsSettingByButton = false;
				pTimer->particularSetting = false;
				AppTimer_Start(pTimer);
				break;
			}
		} else {
			// ==========================
			// [빠른 설정 모드] (900s 단위)
			// ==========================
			switch (btn) {
			case INCREASE:
				if (pTimer->timeRemaining + 900 <= 9900) {
					pTimer->timeRemaining += 900;
				}
				break;

			case DECREASE:
				if (pTimer->timeRemaining - 900 >= 900) {
					pTimer->timeRemaining -= 900;
				}
				break;

			case NEXT: // OK, DONE 둘 다 설정 완료 처리
			case DONE:
				pTimer->timerIsSettingByButton = false;
				// particularSetting은 이미 false임
				AppTimer_Start(pTimer);
				break;
			}
		}
	} else {
		// 5. 설정 모드 진입 (현재 설정 중이 아닐 때)
		if (btn == DONE) {
			// [상세 설정 모드 진입]
			pTimer->timerIsSettingByButton = true;
			pTimer->particularSetting = true;
			pTimer->timeRemaining = 0; // 초기값 0
			pTimer->digitCount = 0;    // 첫째 자리부터
		} else if (btn == NEXT) {
			// [빠른 설정 모드 진입]
			pTimer->timerIsSettingByButton = true;
			pTimer->particularSetting = false;
			pTimer->timeRemaining = 900; // 진입 시 기본값 (최소단위 900)
		}
	}
}

void AppTimer_ShowDisplay(AppTimer_t *pTimer, bool blinkingLastDigit, bool blinkingAllDigit) {
    // 1. 잔상 방지 (모든 자리 OFF)
    for (uint8_t i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(fndSel[i].port, fndSel[i].pin, GPIO_PIN_RESET);
    }

    // 기본 동작: 타이머가 꺼져있고 0이면 아무 것도 표시하지 않음
    // 예외: 점멸 모드(마지막 자리 점멸 / 전체 점멸)일 때는 0이라도 표시/점멸 가능
    if (!pTimer->isRunning && pTimer->timeRemaining == 0) {
        if (!blinkingLastDigit && !blinkingAllDigit)
            return;
    }

    // 2. 깜빡임 카운터 계산 (500ms 주기)
    pTimer->blinkCounter++;
    if (pTimer->blinkCounter >= 500) {
        pTimer->blinkCounter = 0;
    }

    // 3. 전체 점멸 모드: OFF 구간이면 모든 자리 표시를 생략
    if (blinkingAllDigit && (pTimer->blinkCounter >= 250)) {
        pTimer->digitSelect++;
        if (pTimer->digitSelect >= 4) pTimer->digitSelect = 0;
        return;
    }

    // 4. 마지막 자리 인덱스 계산 (표시할 문자열 기준)
    int lastDigitIndex = 0;
    if (pTimer->timeRemaining < 10)       lastDigitIndex = 0;
    else if (pTimer->timeRemaining < 100) lastDigitIndex = 1;
    else if (pTimer->timeRemaining < 1000)lastDigitIndex = 2;
    else                                  lastDigitIndex = 3;

    // 5. 마지막 자리 점멸 모드: OFF 구간이면 해당 자리만 표시 생략
    // timeRemaining==0이고 blinkingLastDigit==true인 경우에도 여기로 들어와서
    // 첫째자리(인덱스 0)의 '0'이 깜빡이게 됨
    if (blinkingLastDigit && (pTimer->digitSelect == lastDigitIndex) && (pTimer->blinkCounter >= 250)) {
        pTimer->digitSelect++;
        if (pTimer->digitSelect >= 4) pTimer->digitSelect = 0;
        return;
    }

    // 6. itoa 변환
    char disp[5] = {0, 0, 0, 0, 0};
    itoa(pTimer->timeRemaining, disp, 10);

    // 7. 값 표시
    if (disp[pTimer->digitSelect] != 0) {
        uint8_t num = disp[pTimer->digitSelect] - '0';

        for (uint8_t i = 0; i < 7; i++) {
            GPIO_PinState state = (GPIO_PinState)((fndData[num] >> i) & 1);
            HAL_GPIO_WritePin(fndSeg[i].port, fndSeg[i].pin, state);
        }

        // 자릿수 선택 핀 ON
        HAL_GPIO_WritePin(fndSel[pTimer->digitSelect].port, fndSel[pTimer->digitSelect].pin, GPIO_PIN_SET);
    }

    // 8. 다음 자릿수 준비
    pTimer->digitSelect++;
    if (pTimer->digitSelect >= 4) pTimer->digitSelect = 0;
}

void AppTimer_Tick(AppTimer_t *pTimer) {
	pTimer->timeRemaining--;
	if (pTimer->timeRemaining <= 0) {
		pTimer->timeRemaining = 0;
		Fan_TurnOff(pTimer->pFanCtrl, pTimer);
	}
}
