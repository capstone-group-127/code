#ifndef EEPROM_H
#define EEPROM_H

#include "stm32f0xx_hal.h"

void eeprom_write(uint8_t data, uint16_t address);
void eeprom_write_multi(uint8_t *data, uint16_t address, uint16_t length);
uint8_t eeprom_read(uint16_t address);
void eeprom_read_multi(uint8_t *data, uint16_t address, uint16_t length);


#endif