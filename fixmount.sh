#!/bin/bash

#run as root!
if [ "$#" -ne 2 ]
then
	echo Usage:error parm!.
	exit 1
fi

if [ ! -f "$1" ]
then
	echo File $1 not found!.
	exit 1
fi

if [ ! -d "$2" ]
then
	mkdir $2
else
	files=`ls $2`
	if [ ! -z "$files" ]; then
		echo "Folder $2 is already mounted(not empty)."
		exit 1
	fi	
fi

block_size=4096
total_blocks=1048576	#4G/4096

file_size=$(stat -c %s $1 | tr -d '\n')
blocks=$[file_size/block_size]
patch_blocks=$[total_blocks-blocks]
mount_file=$1.mount.fixed

if [ ! -f "$mount_file" ]
then
	echo Creating \'$mount_file\'.
	cp $1 $mount_file
	dd if=/dev/zero bs=$block_size seek=$blocks count=$patch_blocks of=$mount_file &> /dev/null

	echo Fixing the ext4 image file \'$mount_file\' using fsck.ext4.
	fsck.ext4 $mount_file -y &> /dev/null
fi

echo Mount \'$mount_file\' to \'$2\' directory.
mount $mount_file $2
