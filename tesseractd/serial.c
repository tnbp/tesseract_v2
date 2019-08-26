#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "serial.h"

typedef struct {
	tcflag_t SERIAL_CFLAGS;
	tcflag_t SERIAL_IFLAGS;
	tcflag_t SERIAL_OFLAGS;
	speed_t SERIAL_BAUDRATE;
	cc_t SERIAL_VMIN;
	cc_t SERIAL_VTIME;
	char SERIAL_DEVICE[10][255+1];
	unsigned short SERIAL_DEVICE_COUNT;
	unsigned long TCT_HANDSHAKE_CHALLENGE_SECRET;
} tesseract_config;

extern tesseract_config config;
extern struct termios SerialPortSettings;

int open_serial(char *file, int flags) {
	int fd = open(file, flags);
	if (fd == -1) return -1;
	
	tcgetattr(fd, &SerialPortSettings);
	config.SERIAL_CFLAGS = SerialPortSettings.c_cflag;
	config.SERIAL_IFLAGS = SerialPortSettings.c_iflag;
	config.SERIAL_OFLAGS = SerialPortSettings.c_oflag;
	/*cfsetispeed(&SerialPortSettings, config.SERIAL_BAUDRATE);
	cfsetospeed(&SerialPortSettings, config.SERIAL_BAUDRATE);

	SerialPortSettings.c_cflag |= SERIAL_DATABITS;
	SerialPortSettings.c_cflag |= CREAD | CLOCAL;

	SerialPortSettings.c_cc[VMIN] = SERIAL_VMIN;
	SerialPortSettings.c_cc[VTIME] = SERIAL_VTIME;

	if ((tcsetattr(fd, TCSANOW, &SerialPortSettings)) != 0) return -1;*/
	return fd;
}

int configure_serial(int fd) {
	cfsetispeed(&SerialPortSettings, config.SERIAL_BAUDRATE);
	cfsetospeed(&SerialPortSettings, config.SERIAL_BAUDRATE);
	SerialPortSettings.c_cc[VMIN] = config.SERIAL_VMIN;
	SerialPortSettings.c_cc[VTIME] = config.SERIAL_VTIME;
	SerialPortSettings.c_cflag = config.SERIAL_CFLAGS;
	SerialPortSettings.c_oflag = config.SERIAL_OFLAGS;
	SerialPortSettings.c_iflag = config.SERIAL_IFLAGS;

	SerialPortSettings.c_cflag |= CS8;
	SerialPortSettings.c_cflag |= CREAD | CLOCAL;

	/*SerialPortSettings.c_iflag |= IGNCR;

	SerialPortSettings.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

	cfsetispeed(&SerialPortSettings, B9600);
	cfsetospeed(&SerialPortSettings, B9600);

	SerialPortSettings.c_cflag &= ~PARENB;
	SerialPortSettings.c_cflag &= ~CSTOPB;
	SerialPortSettings.c_cflag &= ~CSIZE;
	SerialPortSettings.c_cflag |= CS8;

	SerialPortSettings.c_cflag &= ~CRTSCTS;
	SerialPortSettings.c_cflag |= CREAD | CLOCAL;

	SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);
	SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
	SerialPortSettings.c_oflag &= ~OPOST;

	SerialPortSettings.c_cc[VMIN] = 10;
	SerialPortSettings.c_cc[VTIME] = 0;*/
	
	if ((tcsetattr(fd, TCSANOW, &SerialPortSettings)) != 0) return -1;
	return fd;
}

int serial_read(int fd, char *buffer, size_t bytes) {
	memset(buffer, 0, sizeof(buffer)/sizeof(buffer[0]));
	int ret = read(fd, buffer, bytes);
	tcflush(fd, TCIFLUSH);
	return ret;
}

int serial_write(int fd, char *buffer, size_t bytes) {
	tcflush(fd, TCOFLUSH);
	int ret = write(fd, buffer, bytes);
	tcdrain(fd);
	return ret;
}
