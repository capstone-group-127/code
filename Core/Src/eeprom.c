#include "eeprom.h"
#include "main.h"
#include "spi.h"

static const uint8_t CMD_WRITE_BIT = 0x06;
static const uint8_t CMD_WRDI_BIT = 0x04;
static const uint8_t CMD_WRITE = 0x02;
static const uint8_t CMD_READ = 0x03;

void eeprom_write(uint8_t data, uint16_t address) {
  // Set the write enable latch
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&CMD_WRITE_BIT, 1, 100);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

  HAL_Delay(100);
  // Send the data
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&CMD_WRITE, 1, 100);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&address, 2, 100);
  HAL_SPI_Transmit(&hspi1, &data, 1, 100);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

  HAL_Delay(100);
  // Disable the write enable latch
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&CMD_WRDI_BIT, 1, 100);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
}

void eeprom_write_multi(uint8_t *data, uint16_t address, uint16_t length) {
  for (uint16_t i = 0; i < length; i++) {
    eeprom_write(data[i], address + i);
  }
}

uint8_t eeprom_read(uint16_t address) {
  uint8_t data_read_back;

  // Send the read command
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&CMD_READ, 1, 100);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&address, 2, 100);
  HAL_SPI_Receive(&hspi1, &data_read_back, 1, 100);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

  return data_read_back;
}

void eeprom_read_multi(uint8_t *data, uint16_t address, uint16_t length) {
  for (uint16_t i = 0; i < length; i++) {
    data[i] = eeprom_read(address + i);
  }
}
