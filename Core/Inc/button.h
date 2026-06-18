/* button.h */
#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>

// 버튼 이벤트 정의
typedef enum {
    BTN_NONE,    // 아무 상태 아님
    BTN_PRESSED,     // 1. 버튼이 눌림 (누르기 시작)
    BTN_RELEASED,    // 2. 버튼을 뗌 (짧게 눌렀다 뗄 때 발생 - 실행 동작)
    BTN_LONG         // 3. 길게 누름 (누르고 2초 이상 경과)
} ButtonEvent_t;

typedef struct {
    GPIO_TypeDef* port;
    uint16_t      pin;
    GPIO_PinState onState;

    uint32_t      prevTime;       // 디바운싱용 시간 기록
    uint32_t      pressStartTime; // 길게 누르기 측정용 시작 시간
    bool          isPressed;      // 현재 눌려있는지 상태
    bool          longPressHandled; // 길게 누르기 처리 여부
} Button_t;

void Button_Init(Button_t* pBtn, GPIO_TypeDef* port, uint16_t pin, GPIO_PinState onState);
ButtonEvent_t Button_GetEvent(Button_t* pBtn);

#endif /* INC_BUTTON_H_ */
