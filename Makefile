release:
	gcc main.c checksum.c -I /usr/include/libusb-1.0/  -lusb-1.0  -o syber_usb
debug:
	gcc main.c checksum.c -I /usr/include/libusb-1.0/  -lusb-1.0 -D SPRD_DEBUG -o syber_usb_debug

all:release debug

