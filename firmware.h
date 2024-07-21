#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
HEADER (size = 12b)
    2b - id = 100
    2b - block num = 0
    4b - firmware size in bytes
    2b - firmware crc
    2b - header crc

DATA PACKET (size = FIRMWARE_BLOCK_SZ)
    2b - id = 101
    2b - block num = 1..n
    122b - data
    2b - packet crc
*******************************************************************************/

#define FIRMWARE_BLOCK_SZ       128

#define FIRMWARE_HEADER_SZ      12
#define FIRMWARE_HEADER_ID      100
#define FIRMWARE_DATA_PACKET_ID 101

typedef enum {
  SR_OK            = 0x00u, /**< The action was successful. */
  SR_ERROR_CRC     = 0x01u, /**< CRC calculation error. */
  SR_ERROR_NUMBER  = 0x02u, /**< Packet number mismatch error. */
  SR_ERROR_UART    = 0x04u, /**< UART communication error. */
  SR_ERROR_FLASH   = 0x08u, /**< Flash related error. */
  SR_ERROR         = 0xFFu  /**< Generic error. */
} e_status_report;


void firmware_process(void);



#endif /* _FIRMWARE_H_ */