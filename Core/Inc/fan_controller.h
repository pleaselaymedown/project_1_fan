/* fan_controller.h */
#ifndef INC_FAN_CONTROLLER_H_
#define INC_FAN_CONTROLLER_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>

// app_timer.h를 include 하지 않고도 포인터형을 사용할 수 있게 해줌
typedef struct AppTimer_t AppTimer_t;

typedef enum {
    ROT_STOP = 0, // 정지
    ROT_45,       // 45도 회전
    ROT_90,       // 90도 회전
    ROT_180       // 180도 회전
} RotationMode;

// 상태 관리를 위한 구조체 정의
typedef struct {
	int powerIdx;
	int powerIdxTemp;
	int TargetPowerIdx;
	RotationMode rotMode;
	bool naturalWind;
	uint16_t servoVal;
	uint16_t naturalWindCounter;
	int servoDir;
	float smoothedPower;
	uint16_t currentMinLimit;
	uint16_t currentMaxLimit;
} FanController_t;

// 함수 프로토타입
void Fan_Init(FanController_t *pFan);
void Fan_SetStronger(FanController_t *pFan);
void Fan_SetWeaker(FanController_t *pFan, AppTimer_t *pTimer);
void Fan_ToggleNaturalWind(FanController_t *pFan);
void Fan_ToggleRotation(FanController_t *pFan);
void Fan_TurnOff(FanController_t *pFan, AppTimer_t *pTimer);
void Fan_Tick(FanController_t *pFan); // 10ms 인터럽트용
void Fan_RotationDisplayTimeout(FanController_t *pFan);

#endif /* INC_FAN_CONTROLLER_H_ */
