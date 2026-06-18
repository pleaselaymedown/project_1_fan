/* fan_controller.c */
#include "fan_controller.h"
#include "led.h" // 기존 led.h가 있다고 가정
#include "app_timer.h"
#include <stdlib.h>

// 외부 핸들 참조
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;

// 상수 및 설정값
static const uint16_t POWER_LEVELS[9] = { 0, 1, 2, 3, 4, 5, 6, 8, 10 };
static const int MAX_IDX = 8;
static const uint16_t SERVO_MIN_180 = 349;
static const uint16_t SERVO_MAX_180 = 2410;
static const uint16_t SERVO_CENTER = 1300;
static const uint16_t SERVO_MIN_90 = 865;
static const uint16_t SERVO_MAX_90 = 1895;
static const uint16_t SERVO_MIN_45 = 1123;
static const uint16_t SERVO_MAX_45 = 1637;

#define EMA_ALPHA 0.003f //지수이동평균의 알파값

static void Fan_ApplySpeed(FanController_t *pFan) { //팬 세기와 LED 적용
	TIM3->CCR1 = POWER_LEVELS[pFan->powerIdx];
	ledOff(8);
	ledOn(pFan->powerIdx);
	if (pFan->powerIdx <= 0)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
}

static void Fan_ApplyServoPos(FanController_t *pFan) {
	TIM2->CCR1 = pFan->servoVal;
}

// 공개 함수 구현
void Fan_Init(FanController_t *pFan) {
	pFan->powerIdx = 0;
    pFan->rotMode = ROT_STOP;
    pFan->servoVal = SERVO_CENTER;
    pFan->servoDir = 2;
    pFan->naturalWind = false;
    pFan->naturalWindCounter = 0;
    pFan->TargetPowerIdx = 0;
    pFan->smoothedPower = 0.0f;
    pFan->currentMinLimit = SERVO_MIN_180;
    pFan->currentMaxLimit = SERVO_MAX_180;

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //팬
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); //서보
    HAL_TIM_Base_Start_IT(&htim5); //tick

    Fan_ApplySpeed(pFan);
    Fan_ApplyServoPos(pFan);
}

void Fan_SetStronger(FanController_t *pFan) {
	if (pFan->naturalWind) {
		Fan_ToggleNaturalWind(pFan);
		return;
	}
	if (pFan->powerIdx < MAX_IDX) {
		pFan->powerIdx++;
		Fan_ApplySpeed(pFan);
	}
}

void Fan_SetWeaker(FanController_t *pFan, AppTimer_t *pTimer) {
	if (pFan->naturalWind) {
		Fan_ToggleNaturalWind(pFan);
		return;
	}
	if (pFan->powerIdx > 1) {
		pFan->powerIdx--;
		Fan_ApplySpeed(pFan);
	} else {
		Fan_TurnOff(pFan, pTimer); //세기가 1일때 또 낮추면 끄기
	}

}

void Fan_ToggleNaturalWind(FanController_t *pFan) {
	if (pFan->powerIdx > 0 && pFan->naturalWind == false) {
		pFan->powerIdxTemp = pFan->powerIdx;
		pFan->naturalWind = true;
		pFan->TargetPowerIdx = pFan->powerIdx;
		pFan->smoothedPower = (float) pFan->powerIdx;
		pFan->naturalWindCounter = 0;
		srand(HAL_GetTick());

	} else if (pFan->naturalWind) {
		pFan->powerIdx = pFan->powerIdxTemp;
		pFan->naturalWind = false;
		Fan_ApplySpeed(pFan);
	}
}

void Fan_ToggleRotation(FanController_t *pFan) {
    // 팬이 꺼져있을 때는 회전 동작 안함 (선택사항)
    if (pFan->powerIdx <= 0) return;

    // 모드 순환: STOP -> 45 -> 90 -> 180 -> STOP
    pFan->rotMode++;
    if (pFan->rotMode > ROT_180) {
        pFan->rotMode = ROT_STOP;
    }

    // 모드에 따른 설정 적용
    ledOff(8);
    switch (pFan->rotMode) {
        case ROT_STOP:
            break;

        case ROT_45:
        	ledOnEach(3);
        	ledOnEach(4);
            pFan->currentMinLimit = SERVO_MIN_45;
            pFan->currentMaxLimit = SERVO_MAX_45;
            break;

        case ROT_90:
        	ledOnEach(2);
        	ledOnEach(5);
            pFan->currentMinLimit = SERVO_MIN_90;
            pFan->currentMaxLimit = SERVO_MAX_90;
            break;

        case ROT_180:
        	ledOnEach(1);
        	ledOnEach(6);
            pFan->currentMinLimit = SERVO_MIN_180;
            pFan->currentMaxLimit = SERVO_MAX_180;
            break;
    }

    __HAL_TIM_SET_COUNTER(&htim11, 0);
    HAL_TIM_Base_Start_IT(&htim11);//3초후 led복귀 타이머 시작
}

void Fan_TurnOff(FanController_t *pFan, AppTimer_t *pTimer) {
	if (pFan->rotMode!=ROT_STOP) {
		pFan->rotMode=ROT_STOP;
	}
	if (pFan->naturalWind) {
		Fan_ToggleNaturalWind(pFan);
	}
	pFan->powerIdx = 0;
	Fan_ApplySpeed(pFan);
	AppTimer_Stop(pTimer);
}

void Fan_Tick(FanController_t *pFan) {

	if (pFan->rotMode != ROT_STOP) {
        pFan->servoVal += pFan->servoDir;

        // 현재 모드의 제한값에 따라 방향 전환
        if (pFan->servoVal >= pFan->currentMaxLimit) {
            pFan->servoVal = pFan->currentMaxLimit;
            pFan->servoDir = -2;
        } else if (pFan->servoVal <= pFan->currentMinLimit) {
            pFan->servoVal = pFan->currentMinLimit;
            pFan->servoDir = 2;
        }
        Fan_ApplyServoPos(pFan);
    }


	//자연풍
	if (pFan->naturalWind && pFan->powerIdx > 0) {
		pFan->naturalWindCounter++;
		// 1초(10ms * 100) 마다 확률적 변동
		if (pFan->naturalWindCounter >= 100) {
			pFan->naturalWindCounter = 0;
			// 10% 확률 (rand() % 10 == 0)
			if ((int)((double)rand() / ((double)RAND_MAX + 1) * 10) == 0) {
				// TargetPowerIdx를 1~8 사이로 랜덤 설정
				pFan->TargetPowerIdx = (int)((double)rand() / ((double)RAND_MAX + 1) * 8) + 1;
			}
		}
		// 지수이동평균 적용
		pFan->smoothedPower = (EMA_ALPHA * (float) pFan->TargetPowerIdx)
				+ ((1.0f - EMA_ALPHA) * pFan->smoothedPower);
		// 정수형 변환 (반올림)
		int nextPowerIdx = (int) (pFan->smoothedPower + 0.5f);

		// 범위 제한
		if (nextPowerIdx < 1)
			nextPowerIdx = 1;
		if (nextPowerIdx > 8)
			nextPowerIdx = 8;
		// 값이 변경되었을 때만 하드웨어(PWM/LED) 업데이트
		if (pFan->powerIdx != nextPowerIdx) {
			pFan->powerIdx = nextPowerIdx;
			Fan_ApplySpeed(pFan);
		}
	}
}

void Fan_RotationDisplayTimeout(FanController_t *pFan) {

    HAL_TIM_Base_Stop_IT(&htim11);// 타이머 정지
    Fan_ApplySpeed(pFan);//led 복구
}
