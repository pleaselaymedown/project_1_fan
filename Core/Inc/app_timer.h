/* app_timer.h */
#ifndef INC_APP_TIMER_H_
#define INC_APP_TIMER_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "fan_controller.h"

// [추가] 버튼 입력 열거형
typedef enum {
    INCREASE,
    DECREASE,
    NEXT, // OK 버튼 역할
    DONE
} ButtonInput;

typedef struct AppTimer_t {
    int timeRemaining;
    int digitCount;
    bool isRunning;
    uint8_t digitSelect;
    FanController_t* pFanCtrl;
    uint16_t blinkCounter;

    // [추가] 설정 상태 플래그
    bool timerIsSettingByButton;
    bool timerIsSettingByRemote;
    bool particularSetting; // true: 상세 설정, false: 빠른 설정
} AppTimer_t;

void AppTimer_Start(AppTimer_t* pTimer);
void AppTimer_Stop(AppTimer_t* pTimer);
void AppTimer_Init(AppTimer_t* pTimer, FanController_t* pFan);
void AppTimer_ProcessInput(AppTimer_t* pTimer, char c);
void AppTimer_ShowDisplay(AppTimer_t* pTimer, bool blinkingLastDigit, bool blinkingAllDigit);
void AppTimer_Tick(AppTimer_t* pTimer);

// [추가] 버튼 처리 함수 선언
void AppTimer_ProcessButton(AppTimer_t* pTimer, ButtonInput btn);

#endif /* INC_APP_TIMER_H_ */
