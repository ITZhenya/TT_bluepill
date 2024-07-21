#include <stdbool.h>

#include "flash.h"
#include "stm32f1xx.h"

void flash_lock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}

void flash_unlock(void)
{
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

bool flash_ready(void) 
{
    return !(FLASH->SR & FLASH_SR_BSY);
}

bool flash_check_EOP(void)
{
    if (FLASH->SR & FLASH_SR_EOP) {
        FLASH->SR |= FLASH_SR_EOP;
        return 1;
    }
    return 0;
}

void flash_erase(uint32_t pageAddress)
{
//    flash_unlock();

    while (FLASH->SR & FLASH_SR_BSY);
    if (FLASH->SR & FLASH_SR_EOP) {
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = pageAddress;
    FLASH->CR |= FLASH_CR_STRT;
    while (!(FLASH->SR & FLASH_SR_EOP));
    FLASH->SR = FLASH_SR_EOP;
    FLASH->CR &= ~FLASH_CR_PER;

//    flash_lock();
}

bool flash_erase_page(uint32_t address)
{
    while (!flash_ready());

    FLASH->CR|= FLASH_CR_PER; 
    FLASH->AR = address;
    FLASH->CR|= FLASH_CR_STRT;
    while (!flash_ready()); 
    FLASH->CR &= ~FLASH_CR_PER;

    return flash_check_EOP();
}

uint32_t flash_read(uint32_t address)
{
    return (*(__IO uint32_t*) address);
}

void flash_write(uint8_t* data, uint32_t address, uint32_t count) 
{
    flash_unlock();

    unsigned int i;

    while (FLASH->SR & FLASH_SR_BSY);
    if (FLASH->SR & FLASH_SR_EOP) {
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR |= FLASH_CR_PG;

    for (i = 0; i < count; i += 2) {
        *(__IO uint16_t*)(address + i) = (((uint16_t)data[i + 1]) << 8) + data[i];
        while (!(FLASH->SR & FLASH_SR_EOP));
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR &= ~(FLASH_CR_PG);

    flash_lock();
}

void flash_clear_app_key(void)
{
    flash_unlock();
    flash_erase_page(FLASH_APP_KEY_ADDRESS) ;
    flash_unlock();
}

void flash_save_app_key(void)
{
    flash_clear_app_key();
    uint8_t key[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    flash_write(key, FLASH_APP_KEY_ADDRESS, 10);
}

/* Define type */
typedef void (*pFunction)(void);

void flash_jump_to_address(uint32_t program_adr)
{
    uint32_t JumpAddress;
    pFunction Jump_To_Application;

    /* Disable all interrupts to prevent disturbances during transition */
    __disable_irq();

    /* The second word in the user code area is the program's start address (reset address) */
    JumpAddress = *(__IO uint32_t *) (program_adr + 4);

    /* Initialize the user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t *) program_adr);

    /* Type conversion */
    Jump_To_Application = (pFunction) JumpAddress;

    /* Transition to the APP */
    Jump_To_Application();
}

void flash_jump_to_boot(void)
{
    flash_jump_to_address(FLASH_BOOT_START_ADDRESS);
}

void flash_jump_to_app(void)
{  
    flash_jump_to_address(FLASH_APP_START_ADDRESS);
}
