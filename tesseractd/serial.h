#ifndef TESSERACTD_SERIAL_H_
#define TESSERACTD_SERIAL_H_

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

int open_serial(char*, int);
int configure_serial(int);
int serial_read(int, char*, size_t);
int serial_write(int, char*, size_t);

#endif
