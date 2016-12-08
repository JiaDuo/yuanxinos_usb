/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */

#include <stdio.h>
#include <sys/types.h>
#include "main.h"
#include "protocol.h"
#include "checksum.h"
#include "stdlib.h"
#include "ff.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		2	/* Example: Map Ramdisk to physical drive 2 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		0	/* Example: Map USB MSD to physical drive 0 */

int USB_disk_initialize(void);
int USB_disk_status(void);
int USB_disk_read(BYTE* buff, DWORD sector, UINT count);
int USB_disk_write(const BYTE* buff, DWORD sector, UINT count);
int USB_disk_ioctl (BYTE cmd, void* buff);

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		//result = MMC_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_USB :
		result = USB_disk_status();

		// translate the reslut code here
		stat = result;

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		//result = MMC_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_USB :
		result = USB_disk_initialize();

		// translate the reslut code here
		stat = result;

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		//result = RAM_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		//result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_read(buff, sector, count);

		// translate the reslut code here
		res = result;

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		//result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		//result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here
		res = result;

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :

		// Process of the command for the RAM drive

		return res;

	case DEV_MMC :

		// Process of the command for the MMC/SD card

		return res;

	case DEV_USB :

		// Process of the command the USB drive
		res = USB_disk_ioctl(CTRL_SPRD_FAT_OPS_END,buff);

		return res;
	}

	return RES_PARERR;
}

/* added by Duo Jia 
*jiaduo@syberos.com
*/
int USB_disk_initialize(void)
{
	int r;int i;int cnt;
	uint8_t ack_buffer[20];
	uint8_t internalsd_partition[84]={
	0x7e,0x00,0x10,0x00,0x4c,0x69,0x00,0x6e,
	0x00,0x74,0x00,0x65,0x00,0x72,0x00,0x6e,
	0x00,0x61,0x00,0x6c,0x00,0x73,0x00,0x64,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
	0xff,0xcb,0x9f,0x7e
	};//no 0x7e 0x7d;
	uint8_t internalsd_partition_ack[8]={
	0x7e,0x00,0x80,0x00,0x00,0xff,0x7f,0x7e
	};
	
        debug_print_hex(internalsd_partition,sizeof(internalsd_partition)); 
	r = sprd_usb_transfer(internalsd_partition,sizeof(internalsd_partition));
	if(r != 0){
		printf("USB_disk_initialize:sprd usb transfer error:%d\n",r);
		return r;
	}
        for(i = 5;i ;i--){
                r = sprd_usb_receive(ack_buffer,&cnt);
                if(r) continue;else break;
        }
        if(!i){
                printf("USB_disk_initialize:sprd usb receive error:%d\n",r);
                return r;
        }
        debug_print_hex(ack_buffer,cnt); 

        if(sprd_verify_frame(ack_buffer,cnt) != 0){
                printf("USB_disk_initialize:sprd verify frame error\n");
                return 1;
        }
        if(ack_buffer[SPRD_FRAME_TYPE_OFF] == BSL_REP_DOWN_SIZE_ERROR){
#ifdef SPRD_DEBUG
                printf("USB_disk_initialize:partition size error(not care!)\n");
#endif
        }
	
	return 0;
}

int USB_disk_status(void)
{
	return RES_OK;
}

int USB_disk_read(BYTE* buff, DWORD sector, UINT count)
{
	int r;int i;int cnt;
	uint8_t com_buffer[84];
	int win_size = 0x3000; //12k default:
	uint32_t buff_offset = 0;

	uint16_t crc;
        uint32_t up_size = _MAX_SS * count;
	uint32_t start_offset = _MAX_SS * sector;
        uint32_t offset = 0;
        uint32_t s_size = 0;

	char *s_buffer = malloc(win_size*2);
        while(up_size){
                s_size = (up_size > win_size) ? win_size:up_size;
                com_buffer[SPRD_FRAME_START_OFF] = SPRD_START_BYTE;
                com_buffer[1] = 0x00;
                com_buffer[SPRD_FRAME_TYPE_OFF] = BSL_CMD_READ_FLASH_MIDST;
                com_buffer[SPRD_FRAME_DATA_SIZE_OFF] = 0x08>>8;
                com_buffer[SPRD_FRAME_DATA_SIZE_OFF+1] = 0x08;
                ((uint32_t*)(com_buffer+SPRD_FRAME_DATA_OFF))[0] = s_size;
                ((uint32_t*)(com_buffer+SPRD_FRAME_DATA_OFF))[1] = offset + start_offset;
                crc = checksum(checksum_type,com_buffer+1,16-4);
                com_buffer[16-3] = crc>>8;
                com_buffer[16-2] = crc;
                com_buffer[16-1] = SPRD_END_BYTE;                       
        

                cnt = sprd_frame_exchange(s_buffer,com_buffer,16,0);
                r = sprd_usb_transfer(s_buffer,cnt);
                debug_print_hex(s_buffer,cnt);
                if(r != 0){
                        printf("USB_disk_read:sprd usb transfer error:%d\n",r);
                        return r;
                }
                int i_temp = 0;
                for(cnt = 0;cnt != s_size + 8;){
                        i = 0;
                        do{
                                r = sprd_usb_receive(s_buffer+i,&i_temp);
                                if(r != 0){
                                        printf("USB_disk_read:sprd usb receive error:%d\n",r);
                                        return r;
                                }
                                i += i_temp;
                        }while(s_buffer[i-1] == 0x7d);//last byte maybe 0x7d
                        debug_print_hex(s_buffer,i);
                        i = sprd_frame_exchange(data_buffer+cnt,s_buffer,i,1);
                        cnt += i;
                }
                if(sprd_verify_frame(data_buffer,cnt) != 0 || data_buffer[SPRD_FRAME_TYPE_OFF] != BSL_REP_READ_FLASH){
                        printf("USB_disk_read:sprd verify frame error\n");
                        return -1;
                }
                //write to buff         cnt = frame_size data_buffer = frame_pointer
		//memcpy(buff,data_buffer+5,cnt-8);
		cnt -= 8;
		for(i = 0;i < cnt;i++){
			buff[buff_offset++] = data_buffer[i+5];
		}

                offset += s_size;
                up_size -= s_size;
        }

        free(s_buffer);
	return 0;
}

int USB_disk_write(const BYTE* buff, DWORD sector, UINT count)
{
	return RES_OK;
}

int USB_disk_ioctl (BYTE cmd, void* buff)
{
	int r;int cnt;
        r = sprd_com_nodata(BSL_CMD_READ_FLASH_END);
        if(r != 0){
                printf("USB_disk_ioctl:sprd com nodata error:%d\n",r);
                return r;
        }
        r = sprd_usb_receive(data_buffer,&cnt);
        if(r != 0){
                printf("USB_disk_ioctl:sprd usb receive error:%d\n",r);
                return r;
        }
        debug_print_hex(data_buffer,cnt);
        if(sprd_verify_frame(data_buffer,cnt) != 0 || data_buffer[SPRD_FRAME_TYPE_OFF] != BSL_REP_ACK){
                printf("USB_disk_ioctl:sprd ack error\n");
                return 1;
        }
	
	return 0;
}




