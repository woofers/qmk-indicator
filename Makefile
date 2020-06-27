CC=gcc

main: qmk-indicator.c
	$(CC) qmk-indicator.c  -lhidapi-libusb -o qmk-indicator

clean: qmk-indicator.c
	rm -f qmk-indicator
