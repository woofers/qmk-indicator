CC=gcc

main: qmk-indicator.c
	$(CC) qmk-indicator.c  -lhidapi-libusb -o qmk-indicator

clean: qmk-indicator.c
	rm -f qmk-indicator

DEST_DIR ?= /usr/bin/
install: qmk-indicator.c emacs-mode 51-quefrency.rules
	install -m 755 qmk-indicator $(DEST_DIR)
	install -m 755 emacs-mode $(DEST_DIR)
	cp 51-quefrency.rules /etc/udev/rules.d/.
	udevadm control --reload-rules
	udevadm trigger
