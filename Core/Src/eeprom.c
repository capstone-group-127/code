#include "eeprom.h"
#include "main.h"
#include "spi.h"
#include <stdint.h>

// datasheet table 9
#define EEPROM_CMD_WREN  0x06 // Enable Write Operations
#define EEPROM_CMD_WRDI  0x04 // Disable Write Operations
#define EEPROM_CMD_RDSR  0x05 // Read Status Register
#define EEPROM_CMD_WRSR  0x01 // Write Status Register
#define EEPROM_CMD_READ  0x03 // Read Data from Memory
#define EEPROM_CMD_WRITE 0x02 // Write Data to Memory

// table 10
#define EEPROM_STATUS_RDY (1 << 0) // Ready/Busy status (0 = Ready, 1 = Busy)
#define EEPROM_STATUS_WEL (1 << 1) // Write Enable Latch (1 = Enabled)

#define CS_EEPROM_Pin GPIO_PIN_1
#define CS_EEPROM_GPIO_Port GPIOB

// spi handle for eeprom bus
extern SPI_HandleTypeDef hspi1;
#define EEPROM_SPI_HANDLE &hspi1

// Define SPI timeout
#define EEPROM_SPI_TIMEOUT 100 // milliseconds

// Helper function to control CS pin
static inline void eeprom_cs_select() {
  HAL_GPIO_WritePin(CS_EEPROM_GPIO_Port, CS_EEPROM_Pin, GPIO_PIN_RESET);
}

static inline void eeprom_cs_deselect() {
  HAL_GPIO_WritePin(CS_EEPROM_GPIO_Port, CS_EEPROM_Pin, GPIO_PIN_SET);
}

// Helper function to wait until EEPROM is not busy
static HAL_StatusTypeDef eeprom_wait_ready() {
  uint8_t cmd = EEPROM_CMD_RDSR;
  uint8_t status = 0;
  uint32_t start_time = HAL_GetTick();

  // Deselect CS briefly before polling, ensuring previous cycle is complete if any
  eeprom_cs_deselect();
  HAL_Delay(1); // Small delay might be needed depending on SPI speed and timings

  do {
    eeprom_cs_select();
    // Send RDSR command
    if (HAL_SPI_Transmit(EEPROM_SPI_HANDLE, &cmd, 1, EEPROM_SPI_TIMEOUT) != HAL_OK) {
      eeprom_cs_deselect();
      return HAL_ERROR; // SPI Error
    }
    // Receive status register value
    if (HAL_SPI_Receive(EEPROM_SPI_HANDLE, &status, 1, EEPROM_SPI_TIMEOUT) != HAL_OK) {
      eeprom_cs_deselect();
      return HAL_ERROR; // SPI Error
    }
    eeprom_cs_deselect();

    // Check timeout (e.g., 500ms - generous)
    if ((HAL_GetTick() - start_time) > 500) {
        printf("EEPROM Ready Poll Timeout!\r\n");
        return HAL_TIMEOUT;
    }

    // Add a small delay before next poll to avoid flooding SPI bus
    // Adjust delay based on expected tWC and system clock
    HAL_Delay(1); // 1ms delay between polls

  } while (status & EEPROM_STATUS_RDY); // Poll until RDY bit is 0

  return HAL_OK;
}

// --- Public EEPROM Functions ---

// Send Write Enable command
HAL_StatusTypeDef eeprom_write_enable() {
  uint8_t cmd = EEPROM_CMD_WREN;
  HAL_StatusTypeDef status;

  // Wait if EEPROM is busy from a previous operation
  if (eeprom_wait_ready() != HAL_OK) {
       return HAL_ERROR;
  }

  eeprom_cs_select();
  status = HAL_SPI_Transmit(EEPROM_SPI_HANDLE, &cmd, 1, EEPROM_SPI_TIMEOUT);
  eeprom_cs_deselect();

  // Check WEL bit to confirm (Optional but good practice)
  // uint8_t reg_status = eeprom_read_status();
  // if (!(reg_status & EEPROM_STATUS_WEL)) return HAL_ERROR;

  return status;
}

// Send Write Disable command
HAL_StatusTypeDef eeprom_write_disable() {
  uint8_t cmd = EEPROM_CMD_WRDI;
  HAL_StatusTypeDef status;

   // Wait if EEPROM is busy
  if (eeprom_wait_ready() != HAL_OK) {
       return HAL_ERROR;
  }

  eeprom_cs_select();
  status = HAL_SPI_Transmit(EEPROM_SPI_HANDLE, &cmd, 1, EEPROM_SPI_TIMEOUT);
  eeprom_cs_deselect();
  return status;
}

// Read Status Register
uint8_t eeprom_read_status() {
    uint8_t cmd = EEPROM_CMD_RDSR;
    uint8_t status_reg = 0xFF; // Default to error value

    // No need to wait for ready here, RDSR works even when busy

    eeprom_cs_select();
    if (HAL_SPI_Transmit(EEPROM_SPI_HANDLE, &cmd, 1, EEPROM_SPI_TIMEOUT) == HAL_OK) {
        HAL_SPI_Receive(EEPROM_SPI_HANDLE, &status_reg, 1, EEPROM_SPI_TIMEOUT);
    }
    eeprom_cs_deselect();

    return status_reg;
}


/**
 * @brief Writes a batch of data to the EEPROM starting at a specific address.
 *        Handles page boundaries internally if length exceeds page size (requires multiple operations).
 *        Currently implements single page write logic (max 32 bytes respecting page boundary).
 *        For writes larger than a page or crossing boundaries, split into multiple calls.
 * @param data Pointer to the data buffer to write.
 * @param address Starting EEPROM address (0x0000 to 0x07FF for CAT25160).
 * @param length Number of bytes to write. Max recommended per call is 32 bytes, respecting page boundaries.
 * @return HAL_StatusTypeDef HAL_OK on success, HAL_ERROR/HAL_TIMEOUT otherwise.
 */
HAL_StatusTypeDef eeprom_write_batch(const uint8_t *data, uint16_t address, uint16_t length) {
  if (data == NULL || length == 0) {
    return HAL_ERROR; // Invalid arguments
  }

  // Datasheet CAT25160 Address Range: 0x0000 - 0x07FF
  if ((address + length - 1) > 0x07FF) {
     printf("EEPROM Write Error: Address out of bounds\r\n");
     return HAL_ERROR;
  }

  // Handle Page Writes (32-byte pages)
  uint16_t current_address = address;
  uint16_t remaining_length = length;
  const uint8_t *data_ptr = data;

  while (remaining_length > 0) {
      // Calculate bytes remaining in the current page
      uint16_t page_offset = current_address & 0x1F; // Offset within 32-byte page (0x00-0x1F)
      uint16_t bytes_in_page = 32 - page_offset;

      // Determine how many bytes to write in this cycle (min of remaining, or page capacity)
      uint16_t chunk_len = (remaining_length < bytes_in_page) ? remaining_length : bytes_in_page;

      // 1. Enable Write
      if (eeprom_write_enable() != HAL_OK) {
          printf("EEPROM Write Error: Failed to enable write.\r\n");
          return HAL_ERROR;
      }

      // 2. Prepare command and address (MSB first!)
      uint8_t cmd_addr[3];
      cmd_addr[0] = EEPROM_CMD_WRITE;
      cmd_addr[1] = (current_address >> 8) & 0xFF; // Address MSB
      cmd_addr[2] = current_address & 0xFF;        // Address LSB

      // 3. Send WRITE command, address, and data
      eeprom_cs_select();
      if (HAL_SPI_Transmit(EEPROM_SPI_HANDLE, cmd_addr, 3, EEPROM_SPI_TIMEOUT) != HAL_OK) {
          eeprom_cs_deselect();
          printf("EEPROM Write Error: Failed to send command/address.\r\n");
          return HAL_ERROR;
      }
      if (HAL_SPI_Transmit(EEPROM_SPI_HANDLE, (uint8_t*)data_ptr, chunk_len, EEPROM_SPI_TIMEOUT) != HAL_OK) {
          eeprom_cs_deselect();
          printf("EEPROM Write Error: Failed to send data.\r\n");
          return HAL_ERROR;
      }
      eeprom_cs_deselect(); // Deselect CS to start the internal write cycle (tWC)

      // 4. Wait for write completion
      if (eeprom_wait_ready() != HAL_OK) {
          printf("EEPROM Write Error: Wait ready timeout/error after write.\r\n");
          return HAL_ERROR; // Timeout or error waiting for ready
      }

      // Update pointers and remaining length
      data_ptr += chunk_len;
      current_address += chunk_len;
      remaining_length -= chunk_len;

      // Optional: Disable write after each chunk for safety, or do it once after the loop
      // eeprom_write_disable();
  }

  // Optional: Disable write once after the entire batch is done
  // eeprom_write_disable();

  return HAL_OK;
}

/**
 * @brief Reads a batch of data from the EEPROM.
 * @param data Pointer to the buffer where read data will be stored.
 * @param address Starting EEPROM address.
 * @param length Number of bytes to read.
 * @return HAL_StatusTypeDef HAL_OK on success, HAL_ERROR/HAL_TIMEOUT otherwise.
 */
HAL_StatusTypeDef eeprom_read_batch(uint8_t *data, uint16_t address, uint16_t length) {
  if (data == NULL || length == 0) {
    return HAL_ERROR; // Invalid arguments
  }
  // Datasheet CAT25160 Address Range: 0x0000 - 0x07FF
   if ((address + length - 1) > 0x07FF) {
     printf("EEPROM Read Error: Address out of bounds\r\n");
     return HAL_ERROR;
   }

  // 1. Wait if EEPROM is busy (necessary if a write was just performed without polling)
  if (eeprom_wait_ready() != HAL_OK) {
    return HAL_ERROR;
  }

  // 2. Prepare command and address (MSB first!)
  uint8_t cmd_addr[3];
  cmd_addr[0] = EEPROM_CMD_READ;
  cmd_addr[1] = (address >> 8) & 0xFF; // Address MSB
  cmd_addr[2] = address & 0xFF;        // Address LSB

  // 3. Send READ command and address, then receive data
  eeprom_cs_select();
  if (HAL_SPI_Transmit(EEPROM_SPI_HANDLE, cmd_addr, 3, EEPROM_SPI_TIMEOUT) != HAL_OK) {
    eeprom_cs_deselect();
    printf("EEPROM Read Error: Failed to send command/address.\r\n");
    return HAL_ERROR;
  }
  if (HAL_SPI_Receive(EEPROM_SPI_HANDLE, data, length, EEPROM_SPI_TIMEOUT) != HAL_OK) {
    eeprom_cs_deselect();
     printf("EEPROM Read Error: Failed to receive data.\r\n");
    return HAL_ERROR;
  }
  eeprom_cs_deselect();

  return HAL_OK;
}

// --- Single Byte functions (can be implemented using batch functions) ---

HAL_StatusTypeDef eeprom_write(uint8_t data, uint16_t address) {
    return eeprom_write_batch(&data, address, 1);
}

uint8_t eeprom_read(uint16_t address) {
    uint8_t data_read = 0xFF; // Default error value
    eeprom_read_batch(&data_read, address, 1);
    return data_read;
}
