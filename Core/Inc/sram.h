#ifndef SRAM_H
#define SRAM_H

#include "stm32f0xx_hal.h"

void sram_test(void);
void sram_write(uint8_t data, uint16_t address);
uint8_t sram_read(uint16_t address);

#endif