release:
	gcc main.c checksum.c ./ff12b/src/diskio.c ./ff12b/src/option/unicode.c ./ff12b/src/ff.c -I /usr/include/libusb-1.0/ -I ./ff12b/src/ -I ./ -lusb-1.0  -o syber_usb
debug:
	gcc main.c checksum.c ./ff12b/src/diskio.c ./ff12b/src/ff.c ./ff12b/src/option/unicode.c -I /usr/include/libusb-1.0/ -I ./ff12b/src/ -I ./ -lusb-1.0 -D SPRD_DEBUG -o syber_usb_debug

all:release debug

install:
	sudo cp syber_usb /usr/local/bin/

clean:
	rm -rf syber_usb_debug syber_usb

