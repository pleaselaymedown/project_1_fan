/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// 분리된 모듈 헤더 포함
#include "button.h"
#include "fan_controller.h"
#include "app_timer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// 버튼 설정을 위한 구조체
typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
} PinConfig_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// 전역 객체 인스턴스 (구조체 변수)
FanController_t gFan;
AppTimer_t gTimer;
Button_t gBtns[4];
uint8_t rxData;

// 버튼 하드웨어 매핑 설정
const PinConfig_t BTN_PINS[4] = { { GPIOC, GPIO_PIN_9 }, // btn1
		{ GPIOA, GPIO_PIN_5 }, // btn2
		{ GPIOB, GPIO_PIN_9 }, // btn3
		{ GPIOA, GPIO_PIN_7 }  // btn4
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void beep() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	__HAL_TIM_SET_COUNTER(&htim10, 0);
	HAL_TIM_Base_Start_IT(&htim10);
}

void beepWhenTurnOn() {
	if (gFan.powerIdx <= 0)
		return;
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	__HAL_TIM_SET_COUNTER(&htim10, 0);
	HAL_TIM_Base_Start_IT(&htim10);
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */

	// 1. 모듈 초기화
	Fan_Init(&gFan);               // 선풍기/서보 초기화
	AppTimer_Init(&gTimer, &gFan); // 타이머 초기화 (선풍기 제어권 전달)

	// 2. 버튼 초기화
	for (int i = 0; i < 4; i++) {
		Button_Init(&gBtns[i], BTN_PINS[i].port, BTN_PINS[i].pin,
				GPIO_PIN_RESET);
	}

	// 3. 시스템 동작 시작
	HAL_UART_Receive_DMA(&huart1, &rxData, 1); // UART 수신 대기
	HAL_TIM_Base_Start_IT(&htim4);             // 디스플레이용 타이머 시작

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
		// 버튼 폴링 및 기능 실행
		if (Button_GetEvent(&gBtns[0]) == BTN_PRESSED) {
			beep();
			if(gTimer.timerIsSettingByButton) AppTimer_ProcessButton(&gTimer, INCREASE);
			else Fan_SetStronger(&gFan);
		}

		if (Button_GetEvent(&gBtns[1]) == BTN_PRESSED) {
			beepWhenTurnOn();
			if(gTimer.timerIsSettingByButton) AppTimer_ProcessButton(&gTimer, DECREASE);
			else Fan_SetWeaker(&gFan, &gTimer);
		}

		ButtonEvent_t btn2Evt = Button_GetEvent(&gBtns[2]);
		ButtonEvent_t btn3Evt = Button_GetEvent(&gBtns[3]);

		if (btn2Evt == BTN_RELEASED) {
			beepWhenTurnOn();
			Fan_ToggleRotation(&gFan);
		}

		if (btn2Evt == BTN_LONG) {
			beepWhenTurnOn();
			Fan_ToggleNaturalWind(&gFan);
		}

		if (btn3Evt == BTN_RELEASED) {
			beepWhenTurnOn();
			AppTimer_ProcessButton(&gTimer, NEXT);
		}

		if (btn3Evt == BTN_LONG) {
			beepWhenTurnOn();
			AppTimer_ProcessButton(&gTimer, DONE);
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

//UART 수신 완료 콜백
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {

		HAL_UART_Receive_DMA(&huart1, &rxData, 1);             // 다음 수신 준비

		// 숫자/특수문자 -> 타이머 입력 처리
		if ((rxData >= '0' && rxData <= '9') || rxData == '*') {
			beepWhenTurnOn();
			AppTimer_ProcessInput(&gTimer, rxData);
		}

		// 알파벳 -> 선풍기 제어
		switch (rxData) {
		case 'A':
			beep();
			Fan_SetStronger(&gFan);
			break;
		case 'B':
			beepWhenTurnOn();
			Fan_SetWeaker(&gFan, &gTimer);
			break;
		case 'C':
			beepWhenTurnOn();
			Fan_ToggleRotation(&gFan);
			break;
		case 'D':
			Fan_ToggleNaturalWind(&gFan);
			break;
		default:
			break;
		}
	}
}

//타이머 콜백
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// TIM1: 1초 (타이머 카운트다운)
	if (htim->Instance == TIM1) {
		AppTimer_Tick(&gTimer);
	}
	// TIM4: 1ms (FND 디스플레이 멀티플렉싱)
	if (htim->Instance == TIM4) {
		AppTimer_ShowDisplay(&gTimer, gTimer.particularSetting, (gTimer.timerIsSettingByButton && !gTimer.particularSetting)); //타이머 버튼으로 상세 세팅중인 상태면 마지막 자리 깜빡이기
	}
	// TIM5: 10ms (서보모터 회전 제어, 자연풍 제어)
	if (htim->Instance == TIM5) {
		Fan_Tick(&gFan);
	}
	// TIM10: 50ms버저 끄기
	if (htim->Instance == TIM10) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
		HAL_TIM_Base_Stop_IT(&htim10);
	}
	// TIM11: 3초 LED 표시 원래대로 전환
	if (htim->Instance == TIM11){
		Fan_RotationDisplayTimeout(&gFan);
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
