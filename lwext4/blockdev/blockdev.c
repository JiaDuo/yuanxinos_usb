/*
 * Copyright (c) 2015 Grzegorz Kostka (kostka.grzegorz@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <ext4_config.h>
#include <ext4_blockdev.h>
#include <ext4_errno.h>

#include "main.h"
#include "protocol.h"
#include "checksum.h"

#define EXT4_BLOCKDEV_BSIZE (uint64_t)(512) //phy block size = 512bytes(depend on hardware)
#define EXT4_BLOCKDEV_BCNT (uint64_t)(8*1024*1024) //4G/EXT4_BLOCKDEV_BSIZE

/**********************BLOCKDEV INTERFACE**************************************/
static int blockdev_open(struct ext4_blockdev *bdev);
static int blockdev_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
			 uint32_t blk_cnt);
static int blockdev_bwrite(struct ext4_blockdev *bdev, const void *buf,
			  uint64_t blk_id, uint32_t blk_cnt);
static int blockdev_close(struct ext4_blockdev *bdev);
static int blockdev_lock(struct ext4_blockdev *bdev);
static int blockdev_unlock(struct ext4_blockdev *bdev);

/******************************************************************************/
EXT4_BLOCKDEV_STATIC_INSTANCE(syberfsdev, EXT4_BLOCKDEV_BSIZE, EXT4_BLOCKDEV_BCNT, blockdev_open,
			      blockdev_bread, blockdev_bwrite, blockdev_close,
			      0, 0);

EXT4_BLOCKDEV_STATIC_INSTANCE(datadev, EXT4_BLOCKDEV_BSIZE, EXT4_BLOCKDEV_BCNT, blockdev_open,
			      blockdev_bread, blockdev_bwrite, blockdev_close,
			      0, 0);

/******************************************************************************/
static int blockdev_open(struct ext4_blockdev *bdev)
{
	/*blockdev_open: skeleton*/
	int r;int i;int cnt;
	uint8_t ack_buffer[20];
	uint8_t syberfs_partition[84]={
	0x7e,0x00,0x10,0x00,0x4c,0x73,0x00,0x79,
	0x00,0x62,0x00,0x65,0x00,0x72,0x00,0x66,
	0x00,0x73,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
	0xff,0x01,0xa1,0x7e
	};//no 0x7e 0x7d;
	uint8_t data_partition[84]={
	0x7e,0x00,0x10,0x00,0x4c,0x64,0x00,0x61,
	0x00,0x74,0x00,0x61,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
	0xff,0x65,0xa2,0x7e 
	};//no 0x7e 0x7d
	uint8_t internalsd_partition_ack[8]={
	0x7e,0x00,0x80,0x00,0x00,0xff,0x7f,0x7e
	};
	if(bdev == &syberfsdev){
	        debug_print_hex(syberfs_partition,sizeof(syberfs_partition)); 
		r = sprd_usb_transfer(syberfs_partition,sizeof(syberfs_partition));
	}else if(bdev == &datadev){
	        debug_print_hex(data_partition,sizeof(data_partition)); 
		r = sprd_usb_transfer(data_partition,sizeof(data_partition));
	}else{
		printf("blockdev_open:bdev error\n");
		return 1;
	}

	if(r != 0){
		printf("blockdev_open:sprd usb transfer error:%d\n",r);
		return r;
	}
        for(i = 5;i ;i--){
                r = sprd_usb_receive(ack_buffer,&cnt);
                if(r) continue;else break;
        }
        if(!i){
                printf("blockdev_open:sprd usb receive error:%d\n",r);
                return r;
        }
        debug_print_hex(ack_buffer,cnt); 

        if(sprd_verify_frame(ack_buffer,cnt) != 0){
                printf("blockdev_open:sprd verify frame error\n");
                return 1;
        }
        if(ack_buffer[SPRD_FRAME_TYPE_OFF] == BSL_REP_DOWN_SIZE_ERROR){
#ifdef SPRD_DEBUG
                printf("blockdev_open:partition size error(not care!)\n");
#endif
        }

	return EOK;
}

/******************************************************************************/

static int blockdev_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
			 uint32_t blk_cnt)
{
        int r;int i;int cnt;
        uint8_t com_buffer[84];
        int win_size = 0x3000; //12k default:
        uint32_t buf_offset = 0;

        uint16_t crc;
        uint32_t up_size = EXT4_BLOCKDEV_BSIZE * blk_cnt;
        uint32_t start_offset = EXT4_BLOCKDEV_BSIZE * blk_id;
        uint32_t offset = 0;
        uint32_t s_size = 0;

	/*blockdev_bread: skeleton*/
	if(bdev != &syberfsdev && bdev != &datadev){
		printf("blockdev_bread:bdev error\n");
		return 1;
	}
		
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
                        printf("blockdev_bread:sprd usb transfer error:%d\n",r);
                        return r;
                }
                int i_temp = 0;
                for(cnt = 0;cnt != s_size + 8;){
                        i = 0;
                        do{
                                r = sprd_usb_receive(s_buffer+i,&i_temp);
                                if(r != 0){ 
                                        printf("blockdev_bread:sprd usb receive error:%d\n",r);
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
                        ((char*)buf)[buf_offset++] = data_buffer[i+5];
                }

                offset += s_size;
                up_size -= s_size;
        }

        free(s_buffer);
		
	return EOK;
}


/******************************************************************************/
static int blockdev_bwrite(struct ext4_blockdev *bdev, const void *buf,
			  uint64_t blk_id, uint32_t blk_cnt)
{
	/*blockdev_bwrite: skeleton*/
	return EOK;
}
/******************************************************************************/
static int blockdev_close(struct ext4_blockdev *bdev)
{
	/*blockdev_close: skeleton*/
        int r;int cnt;
        r = sprd_com_nodata(BSL_CMD_READ_FLASH_END);
        if(r != 0){ 
                printf("blockdev_close:sprd com nodata error:%d\n",r);
                return r;
        }   
        r = sprd_usb_receive(data_buffer,&cnt);
        if(r != 0){ 
                printf("blockdev_close:sprd usb receive error:%d\n",r);
                return r;
        }   
        debug_print_hex(data_buffer,cnt);
        if(sprd_verify_frame(data_buffer,cnt) != 0 || data_buffer[SPRD_FRAME_TYPE_OFF] != BSL_REP_ACK){
                printf("blockdev_close:sprd ack error\n");
                return 1;
        }  
	return EOK;
}

static int blockdev_lock(struct ext4_blockdev *bdev)
{
	/*blockdev_lock: skeleton*/
	return EOK;
}

static int blockdev_unlock(struct ext4_blockdev *bdev)
{
	/*blockdev_unlock: skeleton*/
	return EOK;
}

/******************************************************************************/
//struct ext4_blockdev *ext4_blockdev_get(void)
//{
//	return &blockdev;
//}
struct ext4_blockdev *ext4_syberfsdev_get(void)
{
	return &syberfsdev;
}
struct ext4_blockdev *ext4_datadev_get(void)
{
	return &datadev;
}
/******************************************************************************/

