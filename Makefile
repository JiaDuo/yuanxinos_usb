src-lwext4=lwext4/src/ext4.c
src-lwext4+=lwext4/src/ext4_balloc.c
src-lwext4+=lwext4/src/ext4_bcache.c
src-lwext4+=lwext4/src/ext4_bitmap.c
src-lwext4+=lwext4/src/ext4_blockdev.c
src-lwext4+=lwext4/src/ext4_block_group.c
src-lwext4+=lwext4/src/ext4_crc32.c
src-lwext4+=lwext4/src/ext4_debug.c
src-lwext4+=lwext4/src/ext4_dir.c
src-lwext4+=lwext4/src/ext4_dir_idx.c
src-lwext4+=lwext4/src/ext4_extent.c
src-lwext4+=lwext4/src/ext4_fs.c
src-lwext4+=lwext4/src/ext4_hash.c
src-lwext4+=lwext4/src/ext4_ialloc.c
src-lwext4+=lwext4/src/ext4_inode.c
src-lwext4+=lwext4/src/ext4_journal.c
src-lwext4+=lwext4/src/ext4_mbr.c
src-lwext4+=lwext4/src/ext4_mkfs.c
src-lwext4+=lwext4/src/ext4_super.c
src-lwext4+=lwext4/src/ext4_trans.c
src-lwext4+=lwext4/src/ext4_xattr.c
src-lwext4+=lwext4/blockdev/blockdev.c
src-lwext4+=lwext4/fs_test/common/test_lwext4.c
src-lwext4+=lwext4/fs_test/lwext4_generic.c

src-fat=ff12b/src/diskio.c
src-fat+=ff12b/src/option/unicode.c
src-fat+=ff12b/src/ff.c

src-main=main.c
src-main+=checksum.c

inc-lwext4=-I ./lwext4/include/misc/ -I ./lwext4/include/ -I ./lwext4/include/generated/ -I ./lwext4/blockdev/ -I ./lwext4/fs_test/common
inc-fat=-I ./ff12b/src/
inc-main=-I ./ -I /usr/include/libusb-1.0/

src-all=$(src-main) $(src-lwext4) $(src-fat)
inc-all=$(inc-main) $(inc-lwext4) $(inc-fat)

install-dir=/usr/local/bin

CC_FLAGS=-std=gnu99 -lusb-1.0

release:
	gcc $(src-all) $(inc-all) $(CC_FLAGS) -o syber_usb
debug:
	gcc $(src-all) $(inc-all) $(CC_FLAGS) -g -D SPRD_DEBUG -o syber_usb_debug

all:release debug

install:
	cp syber_usb fdl1.bin fdl2.bin $(install-dir)
uninstall:
	rm -rf $(install-dir)/syber_usb $(install-dir)/fdl1.bin $(install-dir)/fdl2.bin

clean:
	rm -rf syber_usb_debug syber_usb

