#include "uart.h"
#include "stm32f1xx.h"


#define USART_BUS_FREQ                  (SystemCoreClock)

uint8_t rx_tx_buffer[TX_RX_BUFFER_SZ];
uint16_t transfersize = 0;
uint16_t txcount = 0;
volatile uint16_t receivesize = 0;


void uart_init(void)
{
    // enable USART1 and port A clock
    RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN);

    //TX1 (PA9)
    GPIOA->CRH &= (~GPIO_CRH_CNF9_0);
    GPIOA->CRH |= (GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9);

    //RX1 (PA10)
    GPIOA->CRH &= (~GPIO_CRH_CNF10_0);
    GPIOA->CRH |= GPIO_CRH_CNF10_1;
    GPIOA->CRH &= (~(GPIO_CRH_MODE10));

    USART1->BRR = (uint16_t)(USART_BUS_FREQ / USART_BAUDRATE);
    USART1->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE); //|USART_CR1_TXEIE

    NVIC_EnableIRQ (USART1_IRQn);
}

void uart_echo(void) 
{
    while ((USART1->SR & USART_SR_RXNE) == 0) {}
    uint8_t d = USART1->DR;

    while ((USART1->SR & USART_SR_TXE) == 0) {}
    USART1->DR = d;
}

void uart_transmit_str(uint8_t* string)
{
    for (uint32_t index = 0; string[index] != '\0'; index++) {
        char ch = string[index];
        // wait until data is transferred to the shift register
        while ((USART1->SR & USART_SR_TXE) == 0) { }
        // write data to the data register
        USART1->DR = (uint32_t)ch;
    }
}


void USART1_IRQHandler(void)
{
    /* receive data */
    if (USART1->SR & ~USART_SR_RXNE) {
        char symbol = USART1->DR;
        if (receivesize >= TX_RX_BUFFER_SZ) {
            receivesize = 0;
        }
        rx_tx_buffer[receivesize++] = symbol;
    }
    if( USART1->SR & USART_SR_TXE ) {
        USART1->DR = rx_tx_buffer[txcount++];
        if (txcount >= transfersize) {
            USART1->CR1 ^= USART_CR1_TXEIE;
            USART1->CR1 |= USART_CR1_RXNEIE;
        }
    }
}

void com_wait(void)
{
    USART1->SR = USART1->SR & ~USART_SR_IDLE; //clear flag IDLE
    while((USART1->SR & USART_SR_IDLE) == RESET) {} // waiting IDLE
}

void com_receive_enable(void)
{
    receivesize = 0;
    USART1->CR1 |= USART_CR1_RXNEIE;
}
void com_receive_disable(void) 
{
    USART1->CR1 ^= USART_CR1_RXNEIE;
}

void com_transmitter_enable(uint16_t msg_length)
{
    receivesize = 0; //clear buffer
    txcount = 0;
    transfersize = msg_length;
    USART1->CR1 |= USART_CR1_TXEIE;
}



