#ifndef EEPROM_H
#define EEPROM_H

#include "eeprom.h"
#include "main.h"
#include "spi.h"
#include <stdint.h> // Include for uint8_t, uint16_t

HAL_StatusTypeDef eeprom_write(uint8_t data, uint16_t address);
HAL_StatusTypeDef eeprom_write_batch(const uint8_t *data, uint16_t address, uint16_t length);
uint8_t eeprom_read(uint16_t address);
HAL_StatusTypeDef eeprom_read_batch(uint8_t *data, uint16_t address, uint16_t length);

#endif