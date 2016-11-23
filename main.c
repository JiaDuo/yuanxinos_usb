
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libusb.h>

#include "main.h"
#include "checksum.h"
#include "protocol.h"


/* usb bulk transfer buffer */
uint8_t data_buffer[DATA_BUFFER_SIZE];

/* libusb variable */
libusb_device **devs;
libusb_device *sprd_dev = NULL;
libusb_device_handle *sprd_handle;	
/* check sum type */
uint8_t checksum_type = TYPE_CRC;

char part_table[][15]={
"prodnv",	//05000000 = 80m
"miscdata",	//01000000 = 16m
"l_fixnv1",	//01000000 = 16m
"l_fixnv2",	//01000000 = 16m
"l_runtimenv1",	//01000000 = 16m
"l_runtimenv2",	//01000000 = 16m
"l_modem",	//0c000000 = 192m
"l_ldsp",	//03000000 = 48m
"l_gdsp",	//03000000 = 48m
"l_warm",	//03000000 = 48m
"pm_sys",	//01000000 = 16m
"sml",		//01000000 = 16m
"logo",		//01000000 = 16m
"fbootlogo",	//01000000 = 16m
"wcnfdl",	//01000000 = 16m
"wcnmodem",	//0a000000 = 160m
"boot",		//01000000 = 16m
"system",	//b0040000 = 2.9g(about)
"cache",	//96000000 = 2.4g
"recovery",	//14000000 = 320m
"misc",		//01000000 = 16m
"userdata",	//00340000 = 3328k
"ubootlogo",	//01000000 = 16m
"security",	//32000000 = 800m
"dt",		//04000000 = 64m
"cboot",	//20000000 = 512m
"syberfs",	//e8030000 = 3.8g(about)
"data",		//00100000 = 1m(actual=4g)
"internalsd",	//ffffffff = 4g
"",		//end
};
char usage[][80]={
"Usage:",
"sudo ./syber_usb [ready|reset|read] [partition name] [size] [file]",
"  ready|reset|read     - Connect device(ready)",
"                         Reset device(reset)",
"                         Read partition(read)",
"  partition name       - The name of the partition to read (only read)",
"  size                 - The size of the partition to read (only read)",
"                         Support 'm/M' 'k/K'",
"  file                 - The name of the file to store (only read)",
"                                                               ",
"Example:",
"sudo ./syber_usb",
"        upload \"internalsd\" 200m(part)",
"        upload \"data\" 200m(part)",
"        upload \"boot\" 16m(full)",
"sudo ./syber_usb ready",
"sudo ./syber_usb read internalsd 200m internalsd200m.bin",
"sudo ./syber_usb read boot 4096k boot4m.bin",
"sudo ./syber_usb read boot 4096 boot4096bytes.bin",
"sudo ./syber_usb reset",
"",
};

int is_sprd_dev(libusb_device *dev)
{
	struct libusb_device_descriptor desc;

	int r = libusb_get_device_descriptor(dev,&desc);
	if( r == 0){
		if(desc.idProduct == SPRD_PID && desc.idVendor == SPRD_VID){
			printf("Found sprd usb\n");
			printf("PID = 0x%X , VID = 0x%X\n",desc.idProduct,desc.idVendor);
			return 0;
		}
		else return 1;
	}
	else{
		return r;
	}
}

void print_hex(uint8_t* str,int length)
{
	int i;
        for(i = 0; i < length; i++){
                if(!(i%8)) putchar('\n');
                printf("0x%02x ",str[i]);
        }
	if(i && (i%9))
	        putchar('\n');
}

void debug_print_hex(uint8_t* str,int length)
{
        int i;
#ifdef SPRD_DEBUG
	printf("debug:");
        for(i = 0; i < length; i++){
                if(!(i%8)) putchar('\n');
                printf("0x%02x ",str[i]);
        }
	if(i && (i%9))
	        putchar('\n');
#endif
}

unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = -1;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
}  


void print_device_descriptor(struct libusb_device_descriptor *desc_p)
{
	int i;
        for(i = 0; i < desc_p->bLength; i++){
                if(!(i%8)) putchar('\n');
                printf("0x%02x ",((uint8_t *)desc_p)[i]);
        }
}

void print_config_descriptor(struct libusb_config_descriptor *desc_p)
{
	int i;
        for(i = 0; i < desc_p->bLength; i++){
                if(!(i%8)) putchar('\n');
                printf("0x%02x ",((uint8_t *)desc_p)[i]);
        }
}

void print_interface_descriptor(struct libusb_interface_descriptor *desc_p)
{
	int i;
        for(i = 0; i < desc_p->bLength; i++){
                if(!(i%8)) putchar('\n');
                printf("0x%02x ",((uint8_t *)desc_p)[i]);
        }
}

void printf_endpoint_descriptor(struct libusb_endpoint_descriptor *desc_p)
{
        int i;
        for(i = 0; i < desc_p->bLength; i++){
                if(!(i%8)) putchar('\n');
                printf("0x%02x ",((uint8_t *)desc_p)[i]);
        }
}

/* printf the device descriptor & active config descriptor */
int print_descriptor(libusb_device *sprd_dev)
{
	struct libusb_device_descriptor device_desc;
	struct libusb_config_descriptor *config_desc;
	struct libusb_interface_descriptor *interface_desc;
	struct libusb_endpoint_descriptor *endpoint_desc;
	int i,j;
	int r;

	r = libusb_get_device_descriptor(sprd_dev,&device_desc);
	if(r != 0)
		return r;
	r = libusb_get_active_config_descriptor(sprd_dev,&config_desc);
	//r = libusb_get_config_descriptor(sprd_dev,0,&config_desc);
	if(r != 0)
		return r;

	/* device descritpors */
	printf("Device descriptor:");
	print_device_descriptor(&device_desc);
	putchar('\n');

	/* config descriptors */
	printf("Active config descriptor:");
	print_config_descriptor(config_desc);	
	putchar('\n');	

	/* interface & endpoint descriptors */
	printf("Interface number:0x%02x\n",config_desc->bNumInterfaces);	
	for(i = 0; i < config_desc->bNumInterfaces; i++){
		printf("Interface descriptor:");
		interface_desc = \
			(struct libusb_interface_descriptor*)config_desc->interface[i].altsetting;
		print_interface_descriptor(interface_desc);
		putchar('\n');		

		printf("Endpoint descriptor(num=%02x):",interface_desc->bNumEndpoints);
		endpoint_desc = \
			(struct libusb_endpoint_descriptor*)interface_desc->endpoint;
		for(j = 0; j < interface_desc->bNumEndpoints ;j++){
			printf_endpoint_descriptor(&endpoint_desc[j]);
		}
		
		putchar('\n');
	}
	
	/* free the config struct!!! */
	libusb_free_config_descriptor(config_desc);

	return 0;
}

/* return 0 - normal  no 0 - error */
int sprd_usb_transfer(uint8_t* data,int size)
{
	int r;int cnt;
	r = libusb_bulk_transfer(sprd_handle,SPRD_ENDP_OUT,data,size,&cnt,200);
	return r;
}

int sprd_usb_receive(uint8_t* data,int *size)
{
	int r;
	r = libusb_bulk_transfer(sprd_handle,SPRD_ENDP_IN,data,DATA_BUFFER_SIZE,size,200);
	return r;
}

int sprd_verify_frame(uint8_t* frame,int frame_size)
{
	if(frame[0] != SPRD_START_BYTE || frame[frame_size-1] != SPRD_END_BYTE) 
		return -1;
	if(checksum(checksum_type,frame+1,frame_size-4) != ((frame[frame_size-3]<<8)/*'( )'!!*/ | frame[frame_size-2]))
		return -1;
	return 0;
}
/* send com frame without data field */
int sprd_com_nodata(uint8_t bsl_com_byte)
{
	int r;
	uint8_t data[8];
	uint16_t crc;
        data_buffer[0] = SPRD_START_BYTE;       //header
        data_buffer[1] = 0x00;          
        data_buffer[2] = bsl_com_byte;	       //type
        data_buffer[3] = 0x00;
        data_buffer[4] = 0x00;                  //data size
        crc = checksum(checksum_type,&data_buffer[1],4);
        data_buffer[5] = crc>>8;
        data_buffer[6] = (crc&0xff);              //crc
        data_buffer[7] = SPRD_END_BYTE;         //ender				
        r = sprd_usb_transfer(data_buffer,8);
	debug_print_hex(data_buffer,8);

        if(r) return r;
	
	return 0;	
}
/* phone ack only first commication (check_baudrate)*/
int sprd_version(void)
{
	int r;int cnt;
        data_buffer[0] = SPRD_START_BYTE;
        r = sprd_usb_transfer(data_buffer,1);
        if(r != 0) return r;
        r = sprd_usb_receive(data_buffer,&cnt);
        if(r != 0) return r;
        r = sprd_verify_frame(data_buffer,cnt);
        if(r != 0) return r;
        debug_print_hex(data_buffer,cnt);	
	
	return 0;
}

int sprd_connect(void)
{
	int cnt;int r;

	r = sprd_com_nodata(BSL_CMD_CONNECT);
	if(r) return r;
	r = sprd_usb_receive(data_buffer,&cnt);
	if(r) return r;
	r = sprd_verify_frame(data_buffer,cnt);
	if(r) return r;
        debug_print_hex(data_buffer,cnt);
        if(data_buffer[SPRD_FRAME_TYPE_OFF] != BSL_REP_ACK){
                return -1;
        }

	return 0;
}

int sprd_exec_data(void)
{
	int cnt;int r;
	r = sprd_com_nodata(BSL_CMD_EXEC_DATA);
	if(r) return r;
	r = sprd_usb_receive(data_buffer,&cnt);
	if(r) return r;
	r = sprd_verify_frame(data_buffer,cnt);
	if(r) return r;
	debug_print_hex(data_buffer,cnt);
        if(data_buffer[SPRD_FRAME_TYPE_OFF] == BSL_REP_ACK )
                return 0;
	//else if(data_buffer[SPRD_FRAME_TYPE_OFF] == BSL_INCOMPATIBLE_PARTITION)
	//	return 1;
	else return -1;
	
	return 0;
}

/* reset to normal state */
int sprd_normal_reset(void)
{
	int r;int cnt;
	r = sprd_com_nodata(BSL_CMD_NORMAL_RESET);
	if(r) return r;
	r = sprd_usb_receive(data_buffer,&cnt);
	if(r) return r;
	r = sprd_verify_frame(data_buffer,cnt);
	if(r) return r;
	debug_print_hex(data_buffer,cnt);
	if(data_buffer[SPRD_FRAME_TYPE_OFF] == BSL_REP_ACK)
		return 0;
	else return -1;

	return 0;
}

/*
*dir:0 0x7e->0x7d 0x5e
*      0x7d->0x7d 0x5d
*dir:1 0x7d 0x5e->0x7e
*      0x7d 0x5d->0x7d
*return:exchange size
*/
int sprd_frame_exchange(char *dst, const char *src, int src_size, int dir)
{
	int i,cnt;
	dir &= 1;
	if(dir == 0){
        	for(i = 0,cnt = 0;i < src_size; i++){
         	       switch(src[i]){
                	        case 0x7e:
                        	        if(i && (i != src_size-1)){
                                	        dst[cnt++] = 0x7d;dst[cnt++] = 0x5e;
	                                }else dst[cnt++] = src[i];
        	                                break;
                	        case 0x7d:
                        	        dst[cnt++] = 0x7d;dst[cnt++] = 0x5d;
                                	break;
	                        default:
        	                        dst[cnt++] = src[i];
                	                break;
                        }
        	}
	}
	else if(dir == 1){
		for(i = 0,cnt = 0;i < src_size;i++){
			if(src[i] == 0x7d){ 
				switch(src[i+1]){
					case 0x5e:dst[cnt++] = 0x7e;
						break;
					case 0x5d:dst[cnt++] = 0x7d;
						break;
					default:break;
				}
				i++;
			}
			else{
				dst[cnt++] = src[i];
			}
		}	
	}
	return cnt;
}

/* send file to destnation addr */
int sprd_download(const char *file_name,uint32_t download_size,uint32_t dst_addr,uint32_t win_size)
{
	int r;int i=0;int cnt;
	uint16_t crc;
	uint8_t com_buff[16];
	/* start */
#ifdef SPRD_DEBUG
	printf("sprd download step:start\n");
#endif
	com_buff[i++] = SPRD_START_BYTE;
	com_buff[i++] = 0x00;
	com_buff[i++] = BSL_CMD_START_DATA; //com
	com_buff[i++] = 0x00;
	com_buff[i++] = 0x08; //data size
	com_buff[i++] = dst_addr>>24;
	com_buff[i++] = dst_addr>>16;
	com_buff[i++] = dst_addr>>8;
	com_buff[i++] = dst_addr; //distnation
	com_buff[i++] = download_size>>24;
	com_buff[i++] = download_size>>16;
	com_buff[i++] = download_size>>8;
	com_buff[i++] = download_size; // size
	crc = checksum(checksum_type,com_buff+1,12);
	com_buff[i++] = crc>>8;
	com_buff[i++] = crc;	//crc16
	com_buff[i++] = SPRD_END_BYTE;
	debug_print_hex(com_buff,16);
	
	r = sprd_usb_transfer(com_buff,16);
	if(r != 0) return r;
	r = sprd_usb_receive(data_buffer,&cnt);
	if(r != 0) return r;
	r = sprd_verify_frame(data_buffer,cnt);
	if(r != 0) return r;
	if(data_buffer[SPRD_FRAME_TYPE_OFF] != BSL_REP_ACK){
		return -1;
	}
	debug_print_hex(data_buffer,cnt);

	/* middle */
#ifdef SPRD_DEBUG
	printf("sprd download step:middle\n");
#endif
	int fd = open(file_name,O_RDONLY);
	if(fd == -1){
		printf("middle:open %s error\n",file_name);
		return -1;
	}
	
	ssize_t r_size;
	char *s_buffer = malloc(win_size*2);
	while(download_size){
#ifdef SPRD_DEBUG
		printf("download_size is %d\n",download_size);
#endif
		r_size = (download_size > win_size ) ? win_size:download_size;
		r_size = read(fd,(void*)(data_buffer+SPRD_FRAME_DATA_OFF),r_size);
		if(r_size == 0){
			printf("middle:read file error\n");
			return -1;
		}
		download_size -= r_size;

	        data_buffer[SPRD_FRAME_START_OFF] = SPRD_START_BYTE;
	        data_buffer[1] = 0x00;
	        data_buffer[SPRD_FRAME_TYPE_OFF] = BSL_CMD_MIDST_DATA;
	        data_buffer[SPRD_FRAME_DATA_SIZE_OFF] = r_size>>8;
	        data_buffer[SPRD_FRAME_DATA_SIZE_OFF+1] = r_size;
		crc = checksum(checksum_type,data_buffer+1,r_size+4);
		data_buffer[SPRD_FRAME_DATA_OFF+r_size] = crc>>8;
		data_buffer[SPRD_FRAME_DATA_OFF+r_size+1] = crc;
		data_buffer[r_size+8-1] = SPRD_END_BYTE;
		//send frame steaming 
		//0x7e = 0x7d 0x7e^0x20 0x7d = 0x7d 0x7d^0x20 , except header & ender		
		for(i = 0,cnt = 0; i < r_size+8; i++){
			switch(data_buffer[i]){
				case 0x7e:
					if(i && (i != r_size+7)){
						s_buffer[cnt++] = 0x7d;s_buffer[cnt++] = 0x5e;
					}else s_buffer[cnt++] = data_buffer[i];
					break;
				case 0x7d:
					s_buffer[cnt++] = 0x7d;s_buffer[cnt++] = 0x5d;
					break;
				default:
					s_buffer[cnt++] = data_buffer[i];
					break;
			}
		}
		debug_print_hex(s_buffer,cnt);

		r = sprd_usb_transfer(s_buffer,cnt);
		if(r) {
			printf("sprd_usb_transfer error\n");
			free(s_buffer);
			return r;
		}
		r = sprd_usb_receive(data_buffer,&cnt);
		if(r){
			printf("sprd_usb_receive error\n");
			free(s_buffer);
			return r;
		}
		r = sprd_verify_frame(data_buffer,cnt);
		if(r){
			printf("sprd verify error\n");
			free(s_buffer);
			return r;
		}
		debug_print_hex(data_buffer,cnt);
		if(data_buffer[SPRD_FRAME_TYPE_OFF] != BSL_REP_ACK){
			printf("sprd ack error\n");
			free(s_buffer);
			return -1;
		}
	}
	free(s_buffer);
	if(close(fd) == -1){
		printf("close file error\n");
		return -1;
	}
	/* end */
#ifdef SPRD_DEBUG
	printf("sprd download step:end\n");
#endif
	r = sprd_com_nodata(BSL_CMD_END_DATA);
	if(r) return r;
	r = sprd_usb_receive(data_buffer,&cnt);
	if(r) return r;
        r = sprd_verify_frame(data_buffer,cnt);
        if(r) {
		return r;
	}
	debug_print_hex(data_buffer,cnt);
        if(data_buffer[SPRD_FRAME_TYPE_OFF] != BSL_REP_ACK){
        	return -1;
        }
		
	return 0;
}
/* read partition to file 
*part_name - partition name
*up_size - size of read
*win_size - size of one receive
*         Larger and faster, according to the mobile phone transmission capacity adjustment
          win_size < mobile maximum transmission size
*file_name - file to store
*/
int sprd_upload(char* part_name,uint32_t up_size,uint32_t win_size,char *file_name)
{
	int i;int r;int cnt;
	uint16_t crc;
	uint8_t com_buffer[84];
	char *s_buffer = malloc(win_size*2);

	printf("Saving partition:'%s'(size=0x%x) to '%s'\n",part_name,up_size,file_name);
	/* start */
#ifdef SPRD_DEBUG
	printf("sprd upload step:start\n");
#endif
	memset(com_buffer,0x00,84);
        com_buffer[SPRD_FRAME_START_OFF] = SPRD_START_BYTE;
        com_buffer[1] = 0x00;
        com_buffer[SPRD_FRAME_TYPE_OFF] = BSL_CMD_READ_FLASH_START;
        com_buffer[SPRD_FRAME_DATA_SIZE_OFF] = 0x4c>>8;
        com_buffer[SPRD_FRAME_DATA_SIZE_OFF+1] = 0x4c;
	com_buffer[84-1] = SPRD_END_BYTE;
	for(i = 0;part_name[i] != '\0';i++){
		com_buffer[SPRD_FRAME_DATA_OFF+i*2] = part_name[i];
		com_buffer[SPRD_FRAME_DATA_OFF+i*2+1] = 0x00;
	}
	*((uint32_t*)(com_buffer+77)) = /*0x01000000*/0xffffffff; //max size???
	crc = checksum(checksum_type,com_buffer+1,84-4);
	com_buffer[84-3] = crc>>8;
	com_buffer[84-2] = crc;
	
	cnt = sprd_frame_exchange(s_buffer,com_buffer,84,0);
	debug_print_hex(s_buffer,cnt);

	r = sprd_usb_transfer(s_buffer,cnt);
	if(r != 0){
		printf("start:sprd usb transfer error:%d\n",r);
		return r;
	}
	for(i = 5;i ;i--){
		r = sprd_usb_receive(data_buffer,&cnt);
		if(r) continue;else break;
	}
	if(!i){
		printf("start:sprd usb receive error:%d\n",r);
		return r;
	}
	debug_print_hex(data_buffer,cnt);	
	if(sprd_verify_frame(data_buffer,cnt) != 0){
		printf("start:sprd verify frame error\n");
		return -1;
	}
	if(data_buffer[SPRD_FRAME_TYPE_OFF] == BSL_REP_DOWN_SIZE_ERROR){
#ifdef SPRD_DEBUG
		printf("partition  size error(not care!)\n");
#endif
	}
	
	/* middle */
#ifdef SPRD_DEBUG
	printf("sprd upload step:middle\n");
#endif
	umask(0);
	int fd = open(file_name,O_CREAT|O_WRONLY|O_TRUNC,00666);
        if(fd == -1){
               	printf("middle:open or create %s error\n",file_name);
	               return -1;
        }
	uint32_t up_size_count = up_size;
	uint32_t up_size_percent = 0;
	uint32_t offset = 0;
	uint32_t s_size = 0;
	while(up_size){
		s_size = (up_size > win_size) ? win_size:up_size;
		com_buffer[SPRD_FRAME_START_OFF] = SPRD_START_BYTE;
		com_buffer[1] = 0x00;
		com_buffer[SPRD_FRAME_TYPE_OFF] = BSL_CMD_READ_FLASH_MIDST;
		com_buffer[SPRD_FRAME_DATA_SIZE_OFF] = 0x08>>8;
		com_buffer[SPRD_FRAME_DATA_SIZE_OFF+1] = 0x08;
		((uint32_t*)(com_buffer+SPRD_FRAME_DATA_OFF))[0] = s_size;
		((uint32_t*)(com_buffer+SPRD_FRAME_DATA_OFF))[1] = offset;
		crc = checksum(checksum_type,com_buffer+1,16-4);
		com_buffer[16-3] = crc>>8;
		com_buffer[16-2] = crc;
		com_buffer[16-1] = SPRD_END_BYTE;			
	

		cnt = sprd_frame_exchange(s_buffer,com_buffer,16,0);
		r = sprd_usb_transfer(s_buffer,cnt);
		debug_print_hex(s_buffer,cnt);
		if(r != 0){
			printf("middle:sprd usb transfer error:%d\n",r);
			return r;
		}
		int i_temp = 0;
		for(cnt = 0;cnt != s_size + 8;){
			i = 0;
			do{
				r = sprd_usb_receive(s_buffer+i,&i_temp);
				if(r != 0){
					printf("middle:sprd usb receive error:%d\n",r);
					return r;
				}
				i += i_temp;
			}while(s_buffer[i-1] == 0x7d);//last byte maybe 0x7d
			debug_print_hex(s_buffer,i);
			i = sprd_frame_exchange(data_buffer+cnt,s_buffer,i,1);
			cnt += i;
		}
		if(sprd_verify_frame(data_buffer,cnt) != 0 || data_buffer[SPRD_FRAME_TYPE_OFF] != BSL_REP_READ_FLASH){
			printf("middle:sprd verify frame error\n");
			return -1;
		}
		//write to file 	cnt = frame_size data_buffer = frame_pointer
		r = write(fd,data_buffer+5,cnt-8);			
		if(r == -1){
			printf("middle:write to %s error\n",file_name);
			return r;	
		}
		if(r != cnt-8){
			printf("middle:write %x bytes,not complete\n",r);
			return r;
		}

		offset += s_size;
		up_size -= s_size;

		if(up_size_percent !=  ((unsigned long)offset*100/up_size_count)){
			up_size_percent = (unsigned long)offset*100/up_size_count;
			printf("\rupload percent:%%%d",up_size_percent);
			fflush(stdout);
			if(up_size_percent == 100) putchar('\n');
		}
	}

	free(s_buffer);
	if(close(fd) != 0){
		printf("close file error\n");
		return -1;
	}
	/* end */
#ifdef SPRD_DEBUG
	printf("sprd upload step:end\n");
#endif
	r = sprd_com_nodata(BSL_CMD_READ_FLASH_END);
	if(r != 0){
		printf("end:sprd com nodata error:%d\n",r);
		return r;
	}
	r = sprd_usb_receive(data_buffer,&cnt);
	if(r != 0){
		printf("end:sprd usb receive error:%d\n",r);
		return r;
	}
	debug_print_hex(data_buffer,cnt);
	if(sprd_verify_frame(data_buffer,cnt) != 0 || data_buffer[SPRD_FRAME_TYPE_OFF] != BSL_REP_ACK){
		printf("end:sprd ack error\n");
		return -1;
	}

	return 0;
}

/* commucate with bootrom */
int sprd_task_bootrom(void)
{
	int r;

	/* sprd version */
	r = sprd_version();
	if(r != 0) {
		printf("sprd version error\n");
	}else{
		printf("sprd_version:%s\n",data_buffer+SPRD_FRAME_DATA_OFF);
	}

	/* sprd connect */
	r = sprd_connect();
	if(r != 0){
		printf("sprd_connect failed\n");
		return r;
	}else{
		printf("sprd_connect success\n");
	}
	
	/* sprd send fdl1 */
	printf("fdl1.bin file size is %d\n",(int)get_file_size("fdl1.bin"));	
	r = sprd_download("fdl1.bin",get_file_size("fdl1.bin"),0x50000000,528);
	if(r != 0){
		printf("sprd_download fdl1.bin error\n");
		return r;
	}else{
		printf("sprd_download fld1.bin success\n");
	}

	/* running fdl1 */
	r = sprd_exec_data();
	if(r != 0){
		printf("fdl1 exec failed\n");	
		return r;
	}else{
		printf("fdl1 running\n");
	}
	
	return 0;
}
/* commucate with fdl1 */
int sprd_task_fdl1(void)
{
        int r;int i = 0;
	int cnt;
        /* sprd version */
	for(i = 0;i < 2;i++){
        	if(sprd_version())
	                continue;
        	else{
                	printf("sprd_version:%s\n",data_buffer+SPRD_FRAME_DATA_OFF);
			break;
		}
        }

        /* sprd connect */
        r = sprd_connect();
        if(r != 0){
                printf("sprd_connect failed\n");
                return r;
        }else{
                printf("sprd_connect success\n");
        }

        /* sprd send fdl2 */
        printf("fdl2.bin file size is %d\n",(int)get_file_size("fdl2.bin"));
        r = sprd_download("fdl2.bin",get_file_size("fdl2.bin"),0x9f000000,2112);
        if(r != 0){
                printf("sprd_download fdl2.bin error\n");
                return r;
        }else{
                printf("sprd_download fld2.bin success\n");
        }

        /* running fdl2 */
	sprd_com_nodata(BSL_CMD_EXEC_DATA);
        for(i = 15;i ;i--){
                if(sprd_usb_receive(data_buffer,(int*)&cnt) == 0)
                        break;
                else continue;
        }
        if(i){
		if(sprd_verify_frame(data_buffer,cnt) == 0 && data_buffer[SPRD_FRAME_TYPE_OFF] == BSL_INCOMPATIBLE_PARTITION){
			printf("fdl2 running && imcompatible repartiton\n");
			debug_print_hex(data_buffer,(int)cnt);
		}
	}else{
		printf("fdl2 exec failed\n");
		return -1;
	}
	return 0;
}


int main(int argc,char **argv)
{
	ssize_t cnt;
	int r = 0;int i;

	/* help info */
	if(argc == 2 && strcmp(argv[1],"help") == 0){
		for(i = 0;usage[i][0] != '\0';i++){
			printf("%s\n",usage[i]);
		}		
		return 0;
	}

	checksum_type = TYPE_CRC;	
	/* init libusb */
	r = libusb_init(NULL);
	if(r < 0)
		return r;

	/* find the sprd device */
	cnt = libusb_get_device_list(NULL,&devs);
	if(cnt < 0)
		return (int)cnt;

	for(i = 0; i < cnt; i++){
		if(is_sprd_dev(devs[i]) == 0){
			sprd_dev = devs[i];
			break;
		}
	}		

	if(sprd_dev == NULL){
		printf("sprd_dev is null(not find the device)\n");					
		libusb_free_device_list(devs,1);
		libusb_exit(NULL);
		return ;
	}
		
	/* print descriptor */
	r = print_descriptor(sprd_dev);
	if( r != 0)
		return;
	/* open & operate */
	r = libusb_open(sprd_dev,&sprd_handle);
	if(r != 0){
		printf("open sprd_dev error(please run as root):%d\n",r);	
		libusb_free_device_list(devs,1);
		libusb_exit(NULL);
		return ;
	}
	r = libusb_claim_interface(sprd_handle,SPRD_INTERFACE);//interface 0
	if(r != 0){
		printf("claim interface error:%d\n",r);
	}
#ifdef SPRD_DEBUG
	printf("argc:%d\n",argc);
	for(i = 0;i < argc;i++){	
		printf("argv[i]:%s\n",argv[i]);	
	}
#endif
	if(argc == 1){
		printf("start default demo\n");
		//demo task:read boot-16m,internalsd-200m,data-200m,reset
		checksum_type = TYPE_CRC;
		sprd_task_bootrom();
		checksum_type = TYPE_IPSUM;
		sprd_task_fdl1();
        	sprd_upload("boot",0x01000000,0x3000,"boot-16m.img");
		sprd_upload("internalsd",200*1024*1024,0x3000,"internalsd-200m.img");
		sprd_upload("data",200*1024*1024,0x3000,"data-200m.img");

	        if(sprd_normal_reset() == 0){
                	printf("sprd reset to normal\n");
        	}else printf("sprd reset error\n");		
	}
	else if(strcmp(argv[1],"ready") == 0 && argc == 2){
		//read task:fdl1,fdl2 enter
		checksum_type = TYPE_CRC;
		r = sprd_task_bootrom();
		if(r != 0){
			printf("sprd_task_bootrom() error:%d\n",r);
			goto error_release;
		}
		checksum_type = TYPE_IPSUM;
		r = sprd_task_fdl1();
		if(r != 0){
			printf("srpd_task_fdl1() error:%d\n",r);
			goto error_release;
		}
		printf("ready:ok\n");
	}
	else if(strcmp(argv[1],"reset") == 0 && argc == 2){
		//reset task:reset phone to normal
		checksum_type = TYPE_IPSUM;
		r = sprd_normal_reset();
		if(r != 0){
			printf("sprd_normal_reset() error:%d\n",r);
			goto error_release;
		}
		printf("reset:ok\n");
	}
	else if(strcmp(argv[1],"read") == 0 && argc == 5){
		//read task:check argv[?],read partition	
		checksum_type = TYPE_IPSUM;
		for(i = 0;part_table[i][0] != '\0';i++){
			if(strcmp(argv[2],part_table[i]) == 0)
				break;
			else continue;
		}
		if(part_table[i][0] == '\0'){
			printf("partition name %s error\n",argv[2]);
			goto error_release;
		}
		uint32_t i_size = atoi(argv[3]);
		switch(argv[3][strlen(argv[3])-1]){
			case 'm':
			case 'M':i_size = i_size * 1024 * 1024;
			       	break;
			case 'k':
			case 'K':i_size = i_size * 1024;
				break;
			default:break;
		}
		r = sprd_upload(argv[2],i_size,0x3000,argv[4]);//12K
		if(r != 0){
			printf("sprd_upload error:%d\n",r);
			goto error_release;
		}		
	}
	else{
		printf("param not correct\n");
		goto error_release;
	}

error_release:
	libusb_release_interface(sprd_handle,0x00);
	libusb_close(sprd_handle);

	libusb_free_device_list(devs,1);		
	libusb_exit(NULL);		

	return r;
}
