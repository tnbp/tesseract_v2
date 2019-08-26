CC = gcc -O2 -Wall
.PHONY: clean

all: tesseractd

tesseractd:	main.c serial.h serial.c const.h ini.c ini.h
		$(CC) -o tesseractd main.c serial.c ini.c && strip tesseractd

clean: 
	rm -f tesseractd
