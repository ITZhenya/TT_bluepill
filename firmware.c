#include <string.h>

#include "firmware.h"
#include "flash.h"
#include "uart.h"

#define FIRMWARE_START_ADDR     FLASH_APP_START_ADDRESS
#define FIRMWARE_MAX_LEN        FLASH_APP_MAX_SIZE

#define FIRMWARE_TIME_OUT       (50000000)

uint8_t firmware_block[FIRMWARE_BLOCK_SZ];
uint16_t firmware_crc = 0;
uint32_t firmware_len = 0;


const uint16_t crc_16_tab[] = {
  0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
  0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
  0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
  0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
  0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
  0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
  0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
  0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
  0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
  0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
  0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
  0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
  0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
  0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
  0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
  0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
  0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
  0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
  0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
  0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
  0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
  0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
  0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
  0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
  0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
  0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
  0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
  0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
  0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
  0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
  0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
  0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

uint16_t crc16(char *buf, uint32_t bsize)
{
    register uint32_t crc = 0xffff;
    while (bsize--) {
      crc = (crc >> 8) ^ crc_16_tab[(crc ^ (*buf++)) & 0xff];
    }
    return (uint16_t)crc;
}

uint16_t crc16add(uint32_t crc, char *buf, uint32_t bsize)
{
    while (bsize--) {
      crc = (crc >> 8) ^ crc_16_tab[(crc ^ *buf++) & 0xff];
    }
    return (uint16_t)crc;
}

/* save crc */
void firmware_set_crc(uint16_t crc)
{
    firmware_crc = crc;
}
/* check crc */ 
bool firmware_check_crc(void)
{
    static char tmp[4];
    uint16_t crc = 0;
    uint32_t bsz = 0;
    uint32_t address = FIRMWARE_START_ADDR;

    for (int i = 0; i < firmware_len; i+=4) {
        uint32_t data = flash_read(address);
        memcpy(tmp, &data, 4);
        bsz = (firmware_len - i >= 4 ? 4 : firmware_len - i);
        if (i == 0) {
            crc = crc16(tmp, bsz);
        } else {
            crc = crc16add(crc, tmp, bsz);
        }
    }
    return (crc == firmware_crc);
}

/* add firmware part*/
void firmware_add(uint8_t *buf, uint32_t shift, uint32_t size)
{
    // so that there is alignment of 4 bytes
    for (int i = 0; i < size; i++) {
      firmware_block[i] = buf[i]; 
    }
    // write to flash
    flash_write(firmware_block, FIRMWARE_START_ADDR + shift, size);
}

uint8_t firmware_parse_header(uint8_t *buf, uint32_t bsize)
{
    if (bsize != FIRMWARE_HEADER_SZ) {
        return SR_ERROR;
    }
    uint16_t crc = crc16((char*)buf, bsize);
    if (crc != *((uint16_t*)(buf + 10))) {
        return SR_ERROR_CRC;
    }
    if (*((uint16_t*)(buf + 0)) != FIRMWARE_HEADER_ID) {
        return SR_ERROR_NUMBER;
    }
    if (*((uint16_t*)(buf + 2)) != 0) {
        return SR_ERROR_NUMBER; //block num
    }
    firmware_len = *((uint32_t*)(buf + 4));
    if (firmware_len > FIRMWARE_MAX_LEN) {
        return SR_ERROR;
    }
    firmware_crc = *((uint16_t*)(buf + 8));
    return SR_OK;
}

uint8_t firmware_parse_data_packet(uint8_t *buf, uint32_t bsize)
{
    if (bsize != FIRMWARE_BLOCK_SZ) {
        return SR_ERROR;
    }
    uint16_t crc = crc16((char*)buf, bsize);
    if (crc != *((uint16_t*)(buf + FIRMWARE_BLOCK_SZ - 2))) {
        return SR_ERROR_CRC;
    }    
    if (*((uint16_t*)(buf + 0)) != FIRMWARE_DATA_PACKET_ID) {
        return SR_ERROR_NUMBER;
    }
    uint16_t block_num = *((uint16_t*)(buf + 2));
    if (block_num == 0) {
        return SR_ERROR_NUMBER;
    }    
    uint32_t shift = (block_num - 1) * (bsize - 6);
    firmware_add(buf + 4, shift, bsize - 6);
    return SR_OK;
}

void firmware_send_report(uint8_t status)
{
    switch (status) {
      case SR_OK: 
          uart_transmit_str((uint8_t*)"The action was successful.\n\r");
          break;
      case SR_ERROR_CRC:
          uart_transmit_str((uint8_t*)"CRC calculation error. \n\r");
          break;
      case SR_ERROR_NUMBER:
          uart_transmit_str((uint8_t*)"Packet number mismatch error. \n\r");
          break;
      case SR_ERROR_UART:
          uart_transmit_str((uint8_t*)"UART communication error.\n\r");
          break;
      case SR_ERROR_FLASH:
          uart_transmit_str((uint8_t*)"Flash related error.\n\r");
          break;
      case SR_ERROR:
          uart_transmit_str((uint8_t*)"Generic error.\n\r");
          break;
    }
}

bool firmware_wait_data_block(void)
{
    uint32_t timeout = 0;
    while (receivesize ==0 && timeout < FIRMWARE_TIME_OUT) {
        timeout++;
    }
    if (timeout >= FIRMWARE_TIME_OUT) {
        return false;
    }
    com_wait();
    return true;
}

void firmware_process(void)
{
    uint32_t shift = 0;
    uint8_t status = SR_OK;
    receivesize = 0;
    uart_transmit_str((uint8_t*)"BOOTLOADER wait bin-file>>\n\r");
    if (!firmware_wait_data_block()) {//wait header
        firmware_send_report(SR_ERROR_UART);
        return;
    }
    status = firmware_parse_header(rx_tx_buffer, receivesize);
    receivesize = 0;
    firmware_send_report(status);
    if (status != SR_OK) {
        return;
    }
    //wait data packet
    while (shift < firmware_len) {
        if (!firmware_wait_data_block()) {
            firmware_send_report(SR_ERROR_UART);
            return;
        }
        status = firmware_parse_data_packet(rx_tx_buffer, receivesize);
        receivesize = 0;
        firmware_send_report(status);
        if (status != SR_OK) {
            return;
        }
        shift += receivesize - 6;
    }
    //check crc
    if (firmware_check_crc()) { 
        uart_transmit_str((uint8_t*)"\n\rFirmware updated!\n\r");
        uart_transmit_str((uint8_t*)"Jumping to user application...\n\r");
        flash_jump_to_app();
    } else {
        firmware_send_report(SR_ERROR_CRC); 
    }
}

void firmware_clear(void)
{
    uint32_t page_num = FLASH_APP_PAGE_COUNT;
    uint32_t page_addr = FIRMWARE_START_ADDR;

    flash_unlock();
    //FLASH->SR |= FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPERR;
    for (uint32_t i = 0U; i < page_num; i++, page_addr += FLASH_PAGE_SIZE) {
        flash_erase(page_addr);
    }
    flash_lock();
}