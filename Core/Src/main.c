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
#include <stdint.h>

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
uint8_t receivedData[15];
uint8_t index = 0;
uint32_t totalBytes = 0;

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
  /* USER CODE BEGIN 2 */

  setbuf(stdout, NULL);

  /* setup the timer for PWM */
  TIM1->ARR = 1000;
  TIM1->CCR1 = 1;
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  /* initialize USB */
  HAL_GPIO_WritePin(USB_RESET_GPIO_Port, USB_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(USB_RESET_GPIO_Port, USB_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(USB_RESET_GPIO_Port, USB_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("\e[2J");
  printf("\r\nd888888P dP                   dP     dP                                  888888ba                                      ");
  printf("\r\n   88    88                   88     88                                  88    `8b                                     ");
  printf("\r\n   88    88d888b. .d8888b.    88aaaaa88a .d8888b. 88d888b. 88d8b.d8b.    88     88 88d888b. .d8888b. 88d888b. .d8888b. ");
  printf("\r\n   88    88'  `88 88ooood8    88     88  88'  `88 88'  `88 88'`88'`88    88     88 88'  `88 88'  `88 88'  `88 88ooood8 ");
  printf("\r\n   88    88    88 88.  ...    88     88  88.  .88 88       88  88  88    88    .8P 88       88.  .88 88    88 88.  ... ");
  printf("\r\n   dP    dP    dP `88888P'    dP     dP  `88888P8 dP       dP  dP  dP    8888888P  dP       `88888P' dP    dP `88888P' ");
  printf("\r\n\n\nCommands:");
  printf("\r\n  - save: saves 1KB of data to the EEPROM");
  printf("\r\n  - read: reads 1KB of data from the EEPROM");
  printf("\r\n  - breathe: dims and brightens the LED repeatedly");
  printf("\r\n  - brightness: sets the LED brightness");
  printf("\r\n  - write: writes 4GB of data to the SD card");
  printf("\r\n\n\n\e[32m➜ \e[0m ");
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // echoing the user's input
    HAL_UART_Receive(&huart1, uartRxBuf, 1, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, uartRxBuf, 1, HAL_MAX_DELAY);

    if (uartRxBuf[0] == '\r') // if the user hits ENTER
    {
      receivedData[index++] = '\r';
      receivedData[index++] = '\n';
      printf("\r\n");

      if (strncmp((char *)receivedData, "breathe", 7) == 0) {
        printf("breathing for 10 cycles");
        uint16_t step = 1;
        uint16_t brightness = 1;
        uint16_t round = 0;

        while (round < 10) {
          TIM1->CCR1 = brightness;
          HAL_Delay(1); // adjust delay for breathing speed

          brightness += step;

          if (brightness == 800 || brightness == 1) {
            step = -step; // Reverse direction at bounds
            round++;
            printf(".");
          }
        }
        printf("\r\n");
      } else if (strncmp((char *)receivedData, "write", 5) == 0) {
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
      } else if (strncmp((char *)receivedData, "ping", 4) == 0) {
        printf("\r\nPong!\r\n");
      } else if (strncmp((char *)receivedData, "pong", 4) == 0) {
        printf("\r\nPing!\r\n");
      } else if (strncmp((char *)receivedData, "clear", 5) == 0) {
        printf("\e[2J");
      } else if (strncmp((char *)receivedData, "brightness", 10) == 0) {
        printf("\r\nEnter brightness (1-100): ");
        uint8_t brightnessInput[4] = {0};
        uint8_t brightnessIndex = 0;
        while (1) {
          HAL_UART_Receive(&huart1, uartRxBuf, 1, HAL_MAX_DELAY);
          HAL_UART_Transmit(&huart1, uartRxBuf, 1, HAL_MAX_DELAY);

          if (uartRxBuf[0] == '\r') { // User pressed ENTER
            brightnessInput[brightnessIndex] = '\0';
            printf("\r\n");
            break;
          } else if (uartRxBuf[0] == '\b' || uartRxBuf[0] == 127) { // BACKSPACE
            if (brightnessIndex > 0) {
              brightnessIndex--;
              printf("\033[2D");
              printf(" ");
              printf("\033[1D");
            } else {
              printf("\033[1D");
            }
          } else {
            brightnessInput[brightnessIndex++] = uartRxBuf[0];
          }
        }

        int brightnessValue = atoi((char *)brightnessInput);
        if (brightnessValue >= 1 && brightnessValue <= 100) {
          TIM1->CCR1 = (brightnessValue * 10); // Interpolate to range 1-1000
          printf("Brightness set to %d\r\n", brightnessValue);
        } else {
          printf("Invalid brightness value. Please enter a number between 1 "
                 "and 100.\r\n");
        }
      } else if (strncmp((char *)receivedData, "save", 4) == 0) {
      printf("Writing test pattern to EEPROM...\r\n");
          uint8_t write_error = 0;
          for (uint16_t address = 0; address < 0x7D0; address += 16) {
            // Note: sizeof("PSSC COMPLETION") is 16 (includes the null terminator)
            uint8_t data[] = "PSSC COMPLETION";
            // Check return status
            if (eeprom_write_batch(data, address, sizeof(data)) != HAL_OK) {
               printf("Error writing data to address %04X\r\n", address);
               write_error = 1;
               break; // Stop on error
            }
            // Optional: Small delay to allow printf buffer to flush if needed, or remove
            // HAL_Delay(1);
            // Reduce print frequency if it's too slow/spammy
            if ((address % 0x100) == 0) { // Print every 256 bytes
                 printf("Written up to address %04X...\r\n", address);
            }
          }

          if (!write_error) {
              printf("\r\nData write sequence completed. 1KB successfully written.\r\n");
              // Optional: Disable write protect at the end
              // eeprom_write_disable();
          } else {
               printf("\r\nData write sequence failed.\r\n");
          }
      } else if (strncmp((char *)receivedData, "read", 4) == 0) {
      printf("Reading data from EEPROM...\r\n");
          uint8_t read_error = 0;
          for (uint16_t address = 0; address < 0x7D0; address += 16) {
            // Buffer size 16 for data + 1 for null terminator
            uint8_t data[17] = {0};
            // *** Read exactly 16 bytes ***
            const uint16_t bytes_to_read = 16;

            // Check return status
            if (eeprom_read_batch(data, address, bytes_to_read) != HAL_OK) {
                printf("Error reading data from address %04X\r\n", address);
                read_error = 1;
                break; // Stop on error
            }

            // Ensure null termination just in case (should already be zeroed)
            data[bytes_to_read] = '\0';
            printf("Address %04X: %s\r\n", address, data);

             // Optional: Add a small delay if printf output gets garbled / too fast
             // HAL_Delay(5);
          }
           if (!read_error) {
              printf("\r\nData read sequence completed. 1KB successfully read.\r\n");
          } else {
               printf("\r\nData read sequence failed.\r\n");
          }
      } else if(strncmp((char *)receivedData, "reboot", 6) == 0) {
      	NVIC_SystemReset();
      } else {
        printf("\r\nInvalid command: %s", receivedData);
        printf("Valid commands: prox, sram, gps, ping, pong, clear, pic\r\n");
      }

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