#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

#include <libusb.h>

/* debug MICRO */
//#define  SPRD_DEBUG

#define SPRD_VID 0x1782  
#define SPRD_PID 0X4d00
//#define SPRD_PID 0x4002 

#define DATA_BUFFER_SIZE 0x400000 //4MB

/* sprd bulk information */
#define SPRD_INTERFACE	0x00
#define SPRD_ENDP_IN	0x85
#define SPRD_ENDP_OUT	0x06

/* usb bulk transfer buffer */
extern uint8_t data_buffer[DATA_BUFFER_SIZE];
extern uint8_t checksum_type;
extern libusb_device **devs;
extern libusb_device *sprd_dev;
extern libusb_device_handle *sprd_handle;	

int is_sprd_dev(libusb_device *dev);
void print_hex(uint8_t* str,int length);
void debug_print_hex(uint8_t* str,int length);
unsigned long get_file_size(const char *path);
int sprd_usb_transfer(uint8_t* data,int size);
int sprd_usb_receive(uint8_t* data,int *size);
int sprd_verify_frame(uint8_t* frame,int frame_size);
int sprd_com_nodata(uint8_t bsl_com_byte);
int sprd_frame_exchange(char *dst, const char *src, int src_size, int dir);
uint32_t get_sum_file(const char* pathname);

#endif
