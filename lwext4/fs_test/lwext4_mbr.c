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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include <ext4.h>
#include <ext4_mbr.h>
#include "../blockdev/linux/ext4_filedev.h"
#include "../blockdev/windows/io_raw.h"

/**@brief   Input stream name.*/
const char *input_name = NULL;

/**@brief   Block device handle.*/
static struct ext4_blockdev *bd;

/**@brief   Indicates that input is windows partition.*/
static bool winpart = false;

static bool verbose = false;

static const char *usage = "                                    \n\
Welcome in lwext4_mbr tool.                                     \n\
Copyright (c) 2015 Grzegorz Kostka (kostka.grzegorz@gmail.com)  \n\
Usage:                                                          \n\
[-i] --input   - input file name (or blockdevice)               \n\
[-w] --wpart   - windows partition mode                         \n\
[-v] --verbose - verbose mode		                        \n\
\n";


static bool open_linux(void)
{
	ext4_filedev_filename(input_name);
	bd = ext4_filedev_get();
	if (!bd) {
		printf("open_filedev: fail\n");
		return false;
	}
	return true;
}

static bool open_windows(void)
{
#ifdef WIN32
	ext4_io_raw_filename(input_name);
	bd = ext4_io_raw_dev_get();
	if (!bd) {
		printf("open_winpartition: fail\n");
		return false;
	}
	return true;
#else
	printf("open_winpartition: this mode should be used only under windows "
	       "!\n");
	return false;
#endif
}

static bool open_filedev(void)
{
	return winpart ? open_windows() : open_linux();
}

static bool parse_opt(int argc, char **argv)
{
	int option_index = 0;
	int c;

	static struct option long_options[] = {
	    {"input", required_argument, 0, 'i'},
	    {"wpart", no_argument, 0, 'w'},
	    {"verbose", no_argument, 0, 'v'},
	    {"version", no_argument, 0, 'x'},
	    {0, 0, 0, 0}};

	while (-1 != (c = getopt_long(argc, argv, "i:wvx",
				      long_options, &option_index))) {

		switch (c) {
		case 'i':
			input_name = optarg;
			break;
		case 'w':
			winpart = true;
			break;
		case 'v':
			verbose = true;
			break;
		case 'x':
			puts(VERSION);
			exit(0);
			break;
		default:
			printf("%s", usage);
			return false;
		}
	}

	return true;
}

int main(int argc, char **argv)
{
	int r;
	if (!parse_opt(argc, argv)){
		printf("parse_opt error\n");
		return EXIT_FAILURE;
	}

	if (!open_filedev()) {
		printf("open_filedev error\n");
		return EXIT_FAILURE;
	}

	if (verbose)
		ext4_dmask_set(DEBUG_ALL);

	printf("ext4_mbr\n");
	struct ext4_mbr_bdevs bdevs;
	r = ext4_mbr_scan(bd, &bdevs);
	if (r != EOK) {
		printf("ext4_mbr_scan error\n");
		return EXIT_FAILURE;
	}

	int i;
	printf("ext4_mbr_scan:\n");
	for (i = 0; i < 4; i++) {
		printf("mbr_entry %d:\n", i);
		if (!bdevs.partitions[i].bdif) {
			printf("\tempty/unknown\n");
			continue;
		}

		printf("\toffeset: 0x%"PRIx64", %"PRIu64"MB\n",
			bdevs.partitions[i].part_offset,
			bdevs.partitions[i].part_offset / (1024 * 1024));
		printf("\tsize:    0x%"PRIx64", %"PRIu64"MB\n",
			bdevs.partitions[i].part_size,
			bdevs.partitions[i].part_size / (1024 * 1024));
	}


	return EXIT_SUCCESS;
}
