#include "stm32f1xx.h"
#include "systick.h"

#define SYSTICK_PERIOD                  (1000) // for 1 ms
volatile static uint32_t delay;



/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void systick_config(void)
{  
    /* setup systick timer for 1000Hz interrupts */
    if(SysTick_Config(SystemCoreClock / SYSTICK_PERIOD)){
        /* capture error */
        while(1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t count)
{
    delay = count;

    while(0U != delay) {
    }
}

/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)
{
    if(0U != delay){
        delay--;
    }
}




