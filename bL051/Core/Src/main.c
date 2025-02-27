/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "crc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
//#include <stdio.h>
#include <stdbool.h>
//#include <string.h>
#include "Uart.h"
#include "Flash.h"
#include "Rcc.h"
#include "Crc.h"
#include "Hal.h"
#include "Utils.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
	uint32_t v[0x10];
} uninitBuff_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uninitBuff_t *uninit;
uint8_t hexLine[100];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// 	 uint32_t crcinv= crcres ^ 0xffffffff;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */
	// bootloader
	uint32_t resetFlags;
	char rbuff[15];
	bool correctCrc = false;
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */

	halHAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_CRC_Init();
	/* USER CODE BEGIN 2 */

	uninit = (uninitBuff_t*) 0x20000000;
	for (int i = 0; i < 4; i++) {
		uninit->v[i] = 0;
	}

	resetFlags = *(uint32_t*) 0x40021050;
	uninit->v[0] = resetFlags;
	RCC->CSR |= RCC_CSR_RMVF;
	uintToHex(resetFlags, rbuff, 0x0a);
	// uartSendResponse(rbuff);
	resetFlags = *(uint32_t*) 0x40021050;

	bool attf;
	bool app_maybe_present;
	app_maybe_present = *(uint32_t*) AppAddr ? true : false;
	uint32_t reg = GPIOA->IDR;

	if (app_maybe_present) {
		// 	 uint32_t crcres= HAL_CRC_Calculate(&hcrc, AppAddr,10384);
		// 	 uint32_t crcinv= crcres ^ 0xffffffff;
	}

	if ((GPIOA->IDR & ATTF_Pin) != (uint32_t) GPIO_PIN_RESET)
		attf = true;
	else
		attf = false;

	if (!attf && app_maybe_present) {
		HAL_UART_MspDeInit(&huart2);
		__disable_irq();
		start_application(AppAddr);
	}
		/* USER CODE END 2 */

		/* Infinite loop */
		/* USER CODE BEGIN WHILE */

		while (1) {
			int r;
			r = flashReadOneLine();
			r = flashDecodeLine();
			if (r == FLASH_FIN) {
				r = crcCalcCrc(&correctCrc);
			} else {
				r = flashWriteData();
			}
			uintToHex((uint32_t) r, rbuff, 0x0a);
			uartSendResponse(rbuff);
			/* USER CODE END WHILE */

			/* USER CODE BEGIN 3 */
		}
		/* USER CODE END 3 */
	}

	/**
	 * @brief System Clock Configuration
	 * @retval None
	 */
	void SystemClock_Config(void) {
		RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
		RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
		RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

		/** Configure the main internal regulator output voltage
		 */
		__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

		/** Initializes the RCC Oscillators according to the specified parameters
		 * in the RCC_OscInitTypeDef structure.
		 */
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
		RCC_OscInitStruct.HSIState = RCC_HSI_ON;
		RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
		RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
		RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_3;
		RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
		if (rccHAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/** Initializes the CPU, AHB and APB buses clocks
		 */
		RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
				| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
		RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
		RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
		RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

		if (rccHAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)
				!= HAL_OK) {
			Error_Handler();
		}
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
		PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
		if (rccHAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
			Error_Handler();
		}
	}

	/* USER CODE BEGIN 4 */

	/* USER CODE END 4 */

	/**
	 * @brief  This function is executed in case of error occurrence.
	 * @retval None
	 */
	void Error_Handler(void) {
		/* USER CODE BEGIN Error_Handler_Debug */
		/* User can add his own implementation to report the HAL error return state */
		__disable_irq();
		while (1) {
		}
		/* USER CODE END Error_Handler_Debug */
	}

#ifdef  USE_FULL_ASSERT
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
