#ifndef _UART_H_
#define _UART_H_


#include <stdint.h>

#define USART_BAUDRATE                  115200U
#define USART_DATA_BITS                 8
#define USART_STOP_BITS                 1
#define USART_PARITY                    0


#define TX_RX_BUFFER_SZ                 256
   
extern volatile uint16_t receivesize; 
extern uint8_t rx_tx_buffer[TX_RX_BUFFER_SZ];

void uart_init(void);
void uart_transmit_str(uint8_t *string);
void uart_echo(void) ;

void com_wait(void);
void com_receive_enable(void);
void com_receive_disable(void);


#endif /* _UART_H_ */