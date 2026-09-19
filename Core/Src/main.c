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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LIGHT_ADC_MIN 1650
#define LIGHT_ADC_MAX 3950
#define PWM_MAX       999
#define LIGHT_FILTER_SIZE  8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t adc_buf[2];   // ADC raw val

uint16_t light_samples[LIGHT_FILTER_SIZE] = {0};
uint32_t light_sum = 0;
uint8_t light_index = 0;
uint8_t light_sample_count = 0;
 
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
float Thermistor_VoltageToTemp(float vout)
{
    const float VCC = 3.3f;
    const float R_FIXED = 10000.0f;   //  
    const float R0 = 10000.0f;        // NTC 25�C    10k
    const float BETA = 3950.0f;
    const float T0 = 298.15f;         // 25�C in Kelvin

    float r_ntc;
    float temp_k;

    r_ntc = R_FIXED * vout / (VCC - vout);

    temp_k = 1.0f /
             (1.0f / T0 +
              (1.0f / BETA) * logf(r_ntc / R0));

    return temp_k - 273.15f;
}
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
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
 

 

	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, 2);
	HAL_TIM_Base_Start(&htim3);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	    static uint32_t last_print = 0;

    if (HAL_GetTick() - last_print >= 500)
    {
        last_print = HAL_GetTick();

        float light_voltage;
        float temp_voltage;
        float temperature;

        light_voltage = adc_buf[0] / 4095.0f * 3.3f;
        temp_voltage  = adc_buf[1] / 4095.0f * 3.3f;
        temperature   = Thermistor_VoltageToTemp(temp_voltage);

        printf("Light: %.3f V, Temp: %.2f C\r\n",
               light_voltage, temperature);
    }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        uint16_t light;
        uint32_t ccr;
		// 8-sample moving average filter
		// light_index points to the oldest sample / next position to overwrite
        // remove oldest sample from sum
        light_sum -= light_samples[light_index];

        // store newest sample
        light_samples[light_index] = adc_buf[0];

        // add newest sample to sum
        light_sum += light_samples[light_index];

        // move index forward
        light_index++;

        if (light_index >= LIGHT_FILTER_SIZE)
        {
            light_index = 0;
        }

        // during startup, we may not have 8 samples yet
        if (light_sample_count < LIGHT_FILTER_SIZE)
        {
            light_sample_count++;
        }

        // moving average
        light = light_sum / light_sample_count;

        if (light <= LIGHT_ADC_MIN)
        {
            ccr = 0;
        }
        else if (light >= LIGHT_ADC_MAX)
        {
            ccr = PWM_MAX;
        }
        else
        {
            ccr = (light - LIGHT_ADC_MIN) * PWM_MAX / (LIGHT_ADC_MAX - LIGHT_ADC_MIN);
        }

        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ccr);
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
