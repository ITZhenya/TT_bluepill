#include <stdint.h>
#include <string.h>

#include "main.h"
#include "stm32f1xx.h"
#include "systick.h"
#include "uart.h"
#include "firmware.h"
#include "flash.h"

#ifdef PROGRAM_BOOTLOADER
#define TOGGLE_PERIOD   (50)
#endif // PROGRAM_BOOTLOADER

#ifdef PROGRAM_VER1
#define TOGGLE_PERIOD   (200)
#endif // PROGRAM_VER1

#ifdef PROGRAM_VER2
#define TOGGLE_PERIOD   (400)
#endif // PROGRAM_VER2


#define LED_PORT_NUM            (13)
#define LED_GPIO                (GPIOC)
#define LED_CRH_MODE            (GPIO_CRH_MODE13)
#define LED_CRH_MODE_Pos        (GPIO_CRH_MODE13_Pos)
#define LED_CRH_CNF             (GPIO_CRH_CNF13)
#define LED_CRH_CNF_Pos         (GPIO_CRH_CNF13_Pos)


void led_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;   
    LED_GPIO->CRH &= ~(LED_CRH_MODE | LED_CRH_CNF);
    LED_GPIO->CRH |= (0x02 << LED_CRH_MODE_Pos) | (0x00 << LED_CRH_CNF_Pos);
}

#define LED_ON()        SET_BIT(LED_GPIO->ODR, (1<<LED_PORT_NUM))
#define LED_OFF()       CLEAR_BIT(LED_GPIO->ODR, (1<<LED_PORT_NUM))

void led_toggle()
{ 
    LED_GPIO->ODR ^= (1<<LED_PORT_NUM);
}

void systick_event(void)
{
    static uint32_t timingdelaylocal = TOGGLE_PERIOD;

    if (timingdelaylocal) {
      timingdelaylocal--;
    } else {
      led_toggle();
      timingdelaylocal = TOGGLE_PERIOD;
    }
}

void com_message_check()
{
    char str[5]="";

    if (receivesize > 0) { // if data transfer has started
        com_wait();// waiting for end of data transfer
        
        if (receivesize < 4) {
            receivesize = 0;
            return;
        }
        memcpy(str, rx_tx_buffer, 4);
        receivesize = 0;
#ifdef PROGRAM_BOOTLOADER
        if (strcmp(str, "RUN!") == 0) {
            uart_transmit_str((uint8_t*)"Send a new binary file to update the firmware.\n\r");          
            firmware_process();
#else
        if (strcmp(str, "BOOT") == 0) {
            uart_transmit_str((uint8_t*)"\n\rok\n\r");
            flash_jump_to_boot();
#endif // PROGRAM_BOOTLOADER
        } else {
            uart_transmit_str((uint8_t*)"\n\rUnknown command... Try again.\n\r");
        }
    }
}

int main()
{
    __set_PRIMASK(1);
#ifdef PROGRAM_BOOTLOADER
    SCB->VTOR = FLASH_BOOT_START_ADDRESS;
#else
    SCB->VTOR = FLASH_APP_START_ADDRESS;
#endif // PROGRAM_BOOTLOADER
    __set_PRIMASK(0);  
    __enable_irq();

    led_init();
    systick_config();
    uart_init();
    
#ifdef PROGRAM_BOOTLOADER
    uart_transmit_str((uint8_t*)"BOOTLOADER welcomes you!>>\n\r");
#endif // PROGRAM_BOOTLOADER
#if defined(PROGRAM_VER1) || defined(PROGRAM_VER2)
    uart_transmit_str((uint8_t*)"APPLICATION welcomes you!>>\n\r");
#endif // defined(PROGRAM_VER1) || defined(PROGRAM_VER2)
    
    while (1) {
       com_message_check();
    }
}
