#include "sram.h"
#include "main.h"
#include "spi.h"

static const uint8_t CMD_READ = 0x03;
static const uint8_t CMD_WRITE = 0x02;

void sram_write(uint8_t data, uint16_t address) {
    uint8_t memory_address[3] = {(address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};

    HAL_GPIO_WritePin(GPIOB, SS3_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&CMD_WRITE, 1, 100);
    HAL_SPI_Transmit(&hspi1, memory_address, 3, 100);
    HAL_SPI_Transmit(&hspi1, &data, 1, 100);
    HAL_GPIO_WritePin(GPIOB, SS3_Pin, GPIO_PIN_SET);
}

uint8_t sram_read(uint16_t address) {
    uint8_t memory_address[3] = {(address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};
    uint8_t data_read_back = 0;

    HAL_GPIO_WritePin(GPIOB, SS3_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&CMD_READ, 1, 100);
    HAL_SPI_Transmit(&hspi1, memory_address, 3, 100);
    HAL_SPI_Receive(&hspi1, &data_read_back, 1, 100);
    HAL_SPI_Receive(&hspi1, &data_read_back, 1, 100);
    HAL_GPIO_WritePin(GPIOB, SS3_Pin, GPIO_PIN_SET);

    return data_read_back;
}

void sram_write_multi(uint8_t *data, uint16_t address, uint16_t length) {
	for (uint16_t i = 0; i < length; i++) {
		sram_write(data[i], address + i);
	}
}

void sram_test(void) {
    for (uint8_t index = 1; index < 256; index++) {
        printf("Executing SRAM Test...\r\n");
        uint8_t data_to_write = index;
        uint8_t data_read_back = 0;
        uint16_t address = index;

        sram_write(data_to_write, address);
        data_read_back = sram_read(address);

        // Validate results and check for errors
        printf("Data Written: %d, Data Read: %d.\r\n", data_to_write, data_read_back);
        if (data_to_write != data_read_back) {
            printf("Error: Mismatch between written and read data.\r\n");
            break;
        }

        uint8_t spi_error_code = HAL_SPI_GetError(&hspi1);
        printf("SPI Error Code: %d\r\n", spi_error_code);
        if (spi_error_code != HAL_SPI_ERROR_NONE) {
            break;
        }

        uint8_t spi_status = HAL_SPI_GetState(&hspi1);
        printf("SPI Current State: %d\r\n", spi_status);
        if (spi_status != HAL_SPI_STATE_READY) {
            break;
        }
    }
}

void sram_read_multi(uint8_t *data, uint16_t address, uint16_t length) {
	for (uint16_t i = 0; i < length; i++) {
		data[i] = sram_read(address + i);
	}
}