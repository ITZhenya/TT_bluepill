#ifndef _FLASH_H_
#define _FLASH_H_

#include <stdint.h>

#define FLASH_BOOT_START_ADDRESS        (0x08000000)
#define FLASH_APP_START_ADDRESS         (0x08001000)
#define FLASH_APP_END_ADDRESS           (0x08010000)
#define FLASH_PAGE_SIZE                 (0x400)

#define FLASH_APP_MAX_SIZE              (FLASH_APP_END_ADDRESS - FLASH_APP_START_ADDRESS)
#define FLASH_APP_PAGE_COUNT            (FLASH_APP_MAX_SIZE /FLASH_PAGE_SIZE)

#define FLASH_APP_KEY_ADDRESS           (FLASH_APP_START_ADDRESS - FLASH_PAGE_SIZE)


void flash_lock(void);
void flash_unlock(void);

void flash_erase(uint32_t pageAddress);
uint32_t flash_read(uint32_t address);
void flash_write(uint8_t* data, uint32_t address, uint32_t count);

void flash_jump_to_app(void);
void flash_jump_to_boot(void);

void flash_clear_app_key(void);
void flash_save_app_key(void);

#endif /* _FLASH_H_ */
