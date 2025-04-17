/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "fatfs.h"
#include "gpio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "eeprom.h"
#include "sram.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;
char buffer[100];
uint8_t uartRxBuf[1];
uint8_t receivedData[7];
uint8_t index = 0;

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
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_FATFS_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  setbuf(stdout, NULL);

  TIM2->ARR = 1000;
  TIM2->CCR3 = 200;
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);

  HAL_GPIO_WritePin(USB_RESET_GPIO_Port, USB_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(USB_RESET_GPIO_Port, USB_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(USB_RESET_GPIO_Port, USB_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("\r\n\e[32m➜ \e[0m ");
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // echoing the user's input
    HAL_UART_Receive(&huart1, uartRxBuf, 1, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, uartRxBuf, 1, HAL_MAX_DELAY);

    if (uartRxBuf[0] == '\r') // if the user hits ENTER
    {
      printf("\r\n");

      f_mount(&fs, "", 0);
      f_open(&fil, "sensor.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
      uint8_t randomData[UINT8_MAX];
      for (uint8_t i = 0; i < UINT8_MAX; i++) {
          randomData[i] = 'A' + (rand() % 26); // Generate random letters
      }
      randomData[UINT8_MAX] = '\0'; // Null-terminate the string
      for (uint32_t i = 0; i < 1024; i++) {
     	printf(".");
      	for (uint32_t j = 0; j < 1024; j++) {
       		f_puts(randomData, &fil);
       }
      }
      f_close(&fil);
      f_mount(NULL, "", 1);

      index = 0; // Reset index for next message
      printf("\e[32m➜ \e[0m ");
    } else if (uartRxBuf[0] == '\b' ||
               uartRxBuf[0] == 127) { // if the user hits BACKSPACE
      if (index > 0) {
        index--; // Reduce the index
        printf("\033[2D");
        printf(" ");
        printf("\033[1D");
      } else {
        printf("\033[1D");
      }
    } else {
      receivedData[index++] = uartRxBuf[0];
    }
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
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

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

PUTCHAR_PROTOTYPE { HAL_UART_Transmit(&huart1, &ch, 1, HAL_MAX_DELAY); }
