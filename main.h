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

extern libusb_device **devs;
extern libusb_device *sprd_dev;
extern libusb_device_handle *sprd_handle;	

#endif
