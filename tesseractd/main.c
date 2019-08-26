#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "serial.h"
#include "const.h"
#include "ini.h"

//define alen(x) (sizeof(x) / sizeof((x)[0]))

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

tesseract_config config;
struct termios SerialPortSettings;

char OUT_BUFFER[256+1];
char IN_BUFFER[64+1];

char TCT_DEVICE[255+1];

int ser = -1;

static int initial_config_handler(void *user, const char *section, const char *name, const char *value) {
	tesseract_config *pconfig = (tesseract_config*) user;
	if (strncmp(section, "SERIAL", 6)) return 0;

	if (!strncmp(name, "DEVICE", 6) && strncmp(value, "", 1)) {
		if (pconfig->SERIAL_DEVICE_COUNT < 10) strncpy(pconfig->SERIAL_DEVICE[pconfig->SERIAL_DEVICE_COUNT++], value, 255);
		else {
			printf("ERROR: Config file contains too many possible device names (max. 10!)\n");
			return 0;
		}
	}
	return 1;
}

static int serial_config_handler(void *user, const char *section, const char *name, const char *value) {
	tesseract_config *pconfig = (tesseract_config*) user;
	
	if (!strncmp(section, "SERIAL", 6)) {
		if (!strncmp(name, "BAUDRATE", 8)) {
			int baudrate = atoi(value);
			switch (baudrate) {
				case 50:
				pconfig->SERIAL_BAUDRATE = B50;
				break;
				
				case 75:
				pconfig->SERIAL_BAUDRATE = B75;
				break;
			
				case 110:
				pconfig->SERIAL_BAUDRATE = B110;
				break;

				case 134:
				pconfig->SERIAL_BAUDRATE = B134;
				break;

				case 150:
				pconfig->SERIAL_BAUDRATE = B150;
				break;

				case 200:
				pconfig->SERIAL_BAUDRATE = B200;
				break;

				case 300:
				pconfig->SERIAL_BAUDRATE = B300;
				break;

				case 600:
				pconfig->SERIAL_BAUDRATE = B600;
				break;

				case 1200:
				pconfig->SERIAL_BAUDRATE = B1200;
				break;

				case 1800:
				pconfig->SERIAL_BAUDRATE = B1800;
				break;

				case 2400:
				pconfig->SERIAL_BAUDRATE = B2400;
				break;

				case 4800:
				pconfig->SERIAL_BAUDRATE = B4800;
				break;

				case 9600:
				pconfig->SERIAL_BAUDRATE = B9600;
				break;

				case 19200:
				pconfig->SERIAL_BAUDRATE = B19200;
				break;

				case 38400:
				pconfig->SERIAL_BAUDRATE = B38400;
				break;

				case 57600:
				pconfig->SERIAL_BAUDRATE = B57600;
				break;

				case 115200:
				pconfig->SERIAL_BAUDRATE = B115200;
				break;

				case 230400:
				pconfig->SERIAL_BAUDRATE = B230400;
				break;

				default:
				printf("Error: BAUDRATE is invalid or zero (hang up)!\n");
				return 0;
				break;
			}
		}
		else if (!strncmp(name, "CFLAGS_PARENB", 13)) {
			if (atoi(value)) pconfig->SERIAL_CFLAGS |= PARENB;
			else pconfig->SERIAL_CFLAGS &= ~PARENB;
		}
		else if (!strncmp(name, "CFLAGS_CSTOPB", 13)) {
			if (atoi(value)) pconfig->SERIAL_CFLAGS |= CSTOPB;
			else pconfig->SERIAL_CFLAGS &= ~CSTOPB;
		}
		else if (!strncmp(name, "CFLAGS_CSIZE", 12)) {
			if (atoi(value)) pconfig->SERIAL_CFLAGS |= CSIZE;
			else pconfig->SERIAL_CFLAGS &= ~CSIZE;
		}
		else if (!strncmp(name, "CFLAGS_CRTSCTS", 14)) {
			if (atoi(value)) pconfig->SERIAL_CFLAGS |= CRTSCTS;
			else pconfig->SERIAL_CFLAGS &= ~CRTSCTS;
		}
		else if (!strncmp(name, "IFLAGS_IXON", 11)) {
			if (atoi(value)) pconfig->SERIAL_IFLAGS |= IXON;
			else pconfig->SERIAL_IFLAGS &= ~IXON;
		}
		else if (!strncmp(name, "IFLAGS_IXOFF", 12)) {
			if (atoi(value)) pconfig->SERIAL_IFLAGS |= IXOFF;
			else pconfig->SERIAL_IFLAGS &= ~IXOFF;
		}
		else if (!strncmp(name, "IFLAGS_IXANY", 12)) {
			if (atoi(value)) pconfig->SERIAL_IFLAGS |= IXANY;
			else pconfig->SERIAL_IFLAGS &= ~IXANY;
		}
		else if (!strncmp(name, "IFLAGS_ICANON", 13)) {
			if (atoi(value)) pconfig->SERIAL_IFLAGS |= ICANON;
			else pconfig->SERIAL_IFLAGS &= ~ICANON;
		}
		else if (!strncmp(name, "IFLAGS_ECHO", 11)) {
			if (atoi(value)) pconfig->SERIAL_IFLAGS |= ECHO;
			else pconfig->SERIAL_IFLAGS &= ~ECHO;
		}
		else if (!strncmp(name, "IFLAGS_ECHOE", 12)) {
			if (atoi(value)) pconfig->SERIAL_IFLAGS |= ECHOE;
			else pconfig->SERIAL_IFLAGS &= ~ECHOE;
		}
		else if (!strncmp(name, "IFLAGS_ISIG", 11)) {
			if (atoi(value)) pconfig->SERIAL_IFLAGS |= ISIG;
			else pconfig->SERIAL_IFLAGS &= ~ISIG;
		}
		else if (!strncmp(name, "OFLAGS_OPOST", 12)) {
			if (atoi(value)) pconfig->SERIAL_OFLAGS |= OPOST;
			else pconfig->SERIAL_OFLAGS &= ~OPOST;
		}
		else if (!strncmp(name, "VMIN", 4)) {
			pconfig->SERIAL_VMIN = atoi(value);
		}
		else if (!strncmp(name, "VTIME", 5)) {
			pconfig->SERIAL_VTIME = atoi(value);
		}
	}
	else if (!strncmp(section, "TESSERACT", 9)) {
		if (!strncmp(name, "HANDSHAKE_CHALLENGE_SECRET", 26)) {
			pconfig->TCT_HANDSHAKE_CHALLENGE_SECRET = strtoul(value, NULL, 16);
		}
	}
	return 1;
}

int strip_newlines(char *buf) {
	int ret = strcspn(buf, "\n\r");
	if (ret == strlen(buf)) return 0;
	else buf[ret] = '\0';
	return ret;
}

int main(int argc, char **argv) {
	config.SERIAL_DEVICE_COUNT = 0;
	char config_file[255+1] = "tesseractd.cfg";
	if (ini_parse(config_file, initial_config_handler, &config) < 0) {
		printf("Could not load config file (%s)\n", config_file);
		return E_BADCONFIG;
	}
	printf("Read configuration from %s.\n", config_file);

	char found_devices[10][255+1];
	unsigned short found_devices_num = 0;
	char current_device[255+1];
	/*	a) file exists
			aa) file is read-writable
				aaa) file is a TESSERACT (responds correctly to handshake request)
					-> set serial filename, end detection
				aab) file is NOT a TESSERACT
					-> continue with the next possible [filename][i+1]
			ab) file is NOT read-writable
				-> continue with the next possible [filename][i+1]
		b) file does not exist
			-> continue with the the next possible [filename+1][0]	*/
	printf("DEBUG: SERIAL_DEVICE_COUNT = %d\n", config.SERIAL_DEVICE_COUNT);
	printf("DEBUG: SERIAL_DEVICE[0] = %s\n", config.SERIAL_DEVICE[0]);
	for (int i = 0; i < config.SERIAL_DEVICE_COUNT; i++) {
		int j = 0;
		while (1) {
			if (!strstr(config.SERIAL_DEVICE[i], "%d")) {
				printf("DEBUG: Non-iterable device name %s\n", config.SERIAL_DEVICE[i]);
				strncpy(current_device, config.SERIAL_DEVICE[i], 255);
			}
			else snprintf(current_device, 255, config.SERIAL_DEVICE[i], j);
			printf("Trying %s...\n", current_device);
			if (access(current_device, F_OK) == -1) break; // file does not even exist; break out of the cycle, we're done, try next pattern!
			if (access(current_device, W_OK | R_OK) == -1) { // file exists, but is not read/writable -- useless for us!
				if (strstr(config.SERIAL_DEVICE[i], "%d")) j++;
				else break;
			}
			else {
				if (found_devices_num >= 10) {
					printf("WARNING: Ignoring potential device %s (too many devices).\n", current_device);
				}
				else {
					assert(strlen(current_device) < 256);
					strncpy(found_devices[found_devices_num++], current_device, 256); // looks promising -- add to list of possible serial devs
				}
				if (strstr(config.SERIAL_DEVICE[i], "%d")) j++;
				else break;
			}
		}
	}
	// found_devices should now contain all possible serial devices
	printf("DEBUG: Possible serial devices found:\n");
	for (int i = 0; i < found_devices_num; i++) printf("%s\n", found_devices[i]);
	printf("\n\n");
	int hs_challenge, hs_response;
	for (unsigned short i = 0; i < found_devices_num; i++) {
		ser = open_serial(found_devices[i], O_RDWR | O_NOCTTY);
		if (ser == -1) {
			printf("DEBUG: Device %s could not be opened.\n", found_devices[i]);
			continue;
		}
		if (ini_parse(config_file, serial_config_handler, &config) < 0) {
			printf("Could not load config file (%s)\n", config_file);
			ser = -1;
			return E_BADCONFIG;
		}
		ser = configure_serial(ser);
		strncpy(OUT_BUFFER, "TCT_HS_BEGIN", 13);
		printf("DEBUG: Trying to write to %s (OUT_BUFFER: \"%s\")\n", found_devices[i], OUT_BUFFER);
		serial_write(ser, OUT_BUFFER, strlen(OUT_BUFFER));
		usleep((12 + 25) * 20000);
		printf("DEBUG: Trying to read from %s...\n", found_devices[i]);
		// ???
		struct timeval timeout;
		fd_set set;
		FD_ZERO(&set);
		FD_SET(ser, &set);
		timeout.tv_sec = 0;
		timeout.tv_usec = 20000;
		int rv = select(ser + 1, &set, NULL, NULL, &timeout);
		if (rv == -1) {
			printf("ERROR: Could not select() device %s\n", found_devices[i]);
			ser = -1;
			continue;
		}
		else if (rv == 0) {
			printf("ERROR: Timeout trying to perform handshake with %s\n", found_devices[i]);
			ser = -1;
			continue;
		}
		// end ???
		serial_read(ser, IN_BUFFER, 25);
		if (!strncmp(IN_BUFFER, "TCT_HS_STATUS:0x04", 18)) {
			printf("DEBUG: SUCCESS - TESSERACT v2 at %s already completed handshake!\n", found_devices[i]);
			assert(strlen(found_devices[i]) < 256);
			strncpy(TCT_DEVICE, found_devices[i], 256);
			break;
		}
		if (!strncmp(IN_BUFFER, "TCT_HS_STATUS:0x02", 18)) {
			printf("DEBUG: Received status 0x02 for some reason, trying again...\n");
			i--;
			continue;
		}
		if (strncmp(IN_BUFFER, "TCT_HS_CHG:0x", 13) != 0) {
			printf("DEBUG: Did not receive a handshake challenge (not a TESSERACT): %s\n", found_devices[i]);
			printf("DEBUG: Received \"%s\"\n", IN_BUFFER);
			ser = -1;
			continue;
		}
		if (sscanf(IN_BUFFER, "TCT_HS_CHG:0x%x", &hs_challenge) != 1) {
			printf("DEBUG: Did not receive a valid handshake challenge (not a TESSERACT v2?)\n");
			ser = -1;
			continue;
		}
		hs_response = config.TCT_HANDSHAKE_CHALLENGE_SECRET ^ hs_challenge;
		printf("DEBUG: Challenge is %x (answer should be 0x%x)\n", hs_challenge, hs_response);
		memset(OUT_BUFFER, 0, sizeof(OUT_BUFFER));
		snprintf(OUT_BUFFER, 22, "TCT_HS_RSP:0x%x", hs_response);
		usleep((21 + 64 + 10) * 20000);
		tcflush(ser, TCIOFLUSH);
		serial_write(ser, OUT_BUFFER, sizeof(OUT_BUFFER)); // TESSERACT v2 completes handshake
		memset(IN_BUFFER, 0, sizeof(IN_BUFFER));
		usleep((21 + 64 + 10) * 20000);
		serial_read(ser, IN_BUFFER, 64); // for some reason, TESSERACT v2 sends 0x02 here?!
		strip_newlines(IN_BUFFER);
		printf("Response from TESSERACT v2: %s\n", IN_BUFFER);
		if (!strncmp(IN_BUFFER, "TCT_HS_STATUS:0x02", 18)) {
			printf("DEBUG: Received status 0x02 for some reason, trying again...\n");
			i--;
			continue;
		}
		if (strncmp(IN_BUFFER, "TCT_HS_ACK", 10) != 0) {
			printf("ERROR: TESSERACT v2 did not accept handshake (response: %s)\n", IN_BUFFER);
			continue;
		}
		assert(strlen(found_devices[i]) < 256);
		strncpy(TCT_DEVICE, found_devices[i], 256);
		break;
	}
	if (ser == -1) {
		printf("ERROR: Could not find a TESSERACT v2 :(\n");
		return E_NOSERIAL;
	}
	printf("DEBUG: Found TESSERACT v2 at %s and completed handshake! :)\n", TCT_DEVICE);
	long herp = 0;
	//serial_write(ser, "TCT_DSP_REQ:0x03", 17);
	usleep((17 + 25) * 10000);
	while (1) {
		snprintf(OUT_BUFFER, 20, "0x%08lx", herp);
		serial_write(ser, OUT_BUFFER, 11);
		herp++;
		usleep((11 + 25) * 10000);
		if (!(herp % 30)) {
			serial_write(ser, "TCT_KA_REQ", 11);
			usleep((11 + 25) * 10000);
		}
		sleep(1);
	}
	return 0;
}
