/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Servo.h"
#include <stdio.h>
#include <string.h>
#include "OLED.h"
#include "Key.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SERVO_REFRESH_INTERVAL_MS      1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//key function
//key0（1自动模式 0按键模式）
//key1（按键模式下按下+15.0f）
//key2（按键模式下按下-15.0f）
Key_T key0 = {0 , 0 , KEY0_GPIO_Port, KEY0_Pin, 1};
Key_T key1 = {0 , 0 , KEY1_GPIO_Port, KEY1_Pin, 0};
Key_T key2 = {0 , 0 , KEY2_GPIO_Port, KEY2_Pin, 0};

//servo
static const float angle_list[] = {0.0f, 45.0f, 90.0f, 135.0f, 180.0f};
static uint8_t angle_idx = 0;
static uint32_t t_servo_last = 0;
float g_angle = 0.0f;

//oled
uint32_t t_oled_last = 0;
uint8_t oled_buffer[60] = {0};

//usart
uint8_t tx_buffer[60] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  OLED_Init();
  t_oled_last = HAL_GetTick();

  t_servo_last = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (key0.tri && HAL_GetTick() - key0.t_last > KEY_DEBOUNCE_MS)
    {
      key0.tri = 0;
      if (HAL_GPIO_ReadPin(key0.port, key0.pin) == GPIO_PIN_RESET)
      {
        key0.function = !key0.function;
      }
    }

    switch (key0.function)
    {
      case 0:
      if (key1.tri && HAL_GetTick() - key1.t_last > KEY_DEBOUNCE_MS)
      {
        key1.tri = 0;
        if (HAL_GPIO_ReadPin(key1.port, key1.pin) == GPIO_PIN_RESET)
        {
          //角度限幅
          g_angle += 15.0f;
          if (g_angle > SERVO_MAX_DEG) g_angle = SERVO_MAX_DEG;

          //舵机业务
          uint16_t temp_ccr = Servo_SetAngle(g_angle);

          //oled&串口调试
          Servo_ShowOnOLED(g_angle, temp_ccr);
          printf("Target=%3d  CCR=%4d  Pulse=%d.%03dms\r\n",
          (int)g_angle, temp_ccr, temp_ccr / 1000, temp_ccr % 1000);
        }
      }

      if (key2.tri && HAL_GetTick() - key2.t_last > KEY_DEBOUNCE_MS)
      {
        key2.tri = 0;
        if (HAL_GPIO_ReadPin(key2.port, key2.pin) == GPIO_PIN_RESET)
        {
          //角度限幅
          g_angle -= 15.0f;
          if (g_angle < SERVO_MIN_DEG) g_angle = SERVO_MIN_DEG;

          //舵机业务
          uint16_t temp_ccr = Servo_SetAngle(g_angle);

          //oled&串口调试
          Servo_ShowOnOLED(g_angle, temp_ccr);
          printf("Target=%3d  CCR=%4d  Pulse=%d.%03dms\r\n",
          (int)g_angle, temp_ccr, temp_ccr / 1000, temp_ccr % 1000);
        }
      }

      break;

      case 1:
      if (HAL_GetTick() - t_servo_last > SERVO_REFRESH_INTERVAL_MS)
      {
        //时间戳更新
        t_servo_last = HAL_GetTick();

        //舵机业务
        g_angle = angle_list[angle_idx];
        angle_idx = (angle_idx + 1) % (sizeof(angle_list) / sizeof(angle_list[0]));
        uint16_t temp_ccr = Servo_SetAngle(g_angle);

        //oled&串口调试
        Servo_ShowOnOLED(g_angle, temp_ccr);
        printf("Target=%3d  CCR=%4d  Pulse=%d.%03dms\r\n",
        (int)g_angle, temp_ccr, temp_ccr / 1000, temp_ccr % 1000);

      }
      break;

      default:
      key0.function = 1;
      break;
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == key0.pin)
  {
    key0.tri = 1;
    key0.t_last = HAL_GetTick();
  }

  if (GPIO_Pin == key1.pin)
  {
    key1.tri = 1;
    key1.t_last = HAL_GetTick();
  }

  if (GPIO_Pin == key2.pin)
  {
    key2.tri = 1;
    key2.t_last = HAL_GetTick();
  }
}

int _write(int file, char *data, int len)
{
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 100);
  return len;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
