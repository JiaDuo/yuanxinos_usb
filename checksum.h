
#ifndef __CHECKSUM_H
#define __CHECKSUM_H

#include "libusb.h"

#define TYPE_CRC 0
#define TYPE_IPSUM 1

extern uint16_t const crc16_table[256];

extern uint16_t crc16(uint16_t crc, const uint8_t *buffer, size_t len);

uint16_t ipcheck(uint16_t* buffer, int size);

uint16_t checksum(uint8_t type,uint8_t const *buffer, size_t len);

static inline uint16_t crc16_byte(uint16_t crc, const uint8_t data)
{
	return (crc << 8) ^ crc16_table[(crc >> 8 ^ data) & 0xff];
}

#endif /* __CRC16_H */

