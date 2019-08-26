/*
	TESSERACT v2 firmware
	(c) 2019 by tnbp
	
	FEATURE COMPLETE ON 2019-08-09, 12:07

	CHANGELOG:
	2019-08-09
	* Implemented control over LED_RED	

	2019-08-08
	* Shortened strings to save RAM
	* Refactoring

	2019-08-07
	* Handshake completed
	* Display control completed
	* Keyup-/Keydown events completed (serial mode only)
	* Scrolling	

	2019-08-06
	* Dual mode (keyboard / serial)
		- Handshake (begin)
		- Input handling (keyboard / serial)
	
	2019-08-04
	* initial functions (test cases)
*/

#include "tct_const.h"
#include "tct_strings.h"
#include "tct_functions.h"
#include "Arduino.h"

#include <assert.h>
#include <limits.h>
#include <Keyboard.h>							// for Keyboard HID mode
#include <LiquidCrystal_I2C.h>						// for LCD display control

#define alen(x) (sizeof(x) / sizeof((x)[0]))				// number of values in an array

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_NUM_COLS, LCD_NUM_LINES);	// initialize display

short DEVICE_MODE = MODE_KEYBOARD;					// TESSERACT v2 defaults to keyboard mode
unsigned long SERIAL_LIFETIME = 0;
unsigned short SERIAL_BYTES_AVAILABLE = 0;

char SERIAL_RX_BUFFER[256+1];						// TESSERACT v2 can receive 256 characters at once
char SERIAL_TX_BUFFER[64+1];						// TESSERACT v2 can only send 64 characters at once

char DISPLAY_BUFFER[4][256+1];
char DISPLAY_VISIBLE[4][LCD_NUM_COLS+1];

unsigned short TCT_HANDSHAKE_STATUS = TCT_HANDSHAKE_STATUS_NONE;
unsigned long TCT_HANDSHAKE_CHALLENGE;
unsigned short TCT_HANDSHAKE_CHALLENGE_ATTEMPTS = 0;

unsigned short TCT_DISPLAY_SELECTED_LINE = 0xFF;

unsigned long time, dtime;

unsigned long scroll_begin = 0;
unsigned long scroll_offset = 0;
bool is_scrolling = false;
bool line_scrolling[] = { false, true, false, false };
unsigned short longest_length;

short LED_INTERVAL = 0;
bool LED_TOGGLE = false;

bool button_status[alen(BUTTONS_ARRAY)];

void setup() {
	Serial.begin(9600);
	randomSeed(analogRead(1) ^ millis());
	pinMode(PIN_LED_GREEN, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	
	for (unsigned short i = 0; i < alen(BUTTONS_ARRAY); i++) {
		pinMode(BUTTONS_ARRAY[i], INPUT);
		button_status[i] = false;
	}

	lcd.init();
	lcd.backlight();
	lcd.setCursor(0,0);
	lcd.print(TCT_VERSION_STRING);
	lcd.setCursor(0,2);
	lcd.print(TCT_COPYRIGHT_STRING);
	delay(5000);

	lcd.setCursor(0,0);

	time = millis();
}

void loop() {
	if ((SERIAL_BYTES_AVAILABLE = min(Serial.available(), 256))) {
		for (unsigned short i = 0; i < SERIAL_BYTES_AVAILABLE; i++) SERIAL_RX_BUFFER[i] = Serial.read();
		while (Serial.available()) Serial.read(); // throw away excess bytes
		for (unsigned short i = SERIAL_BYTES_AVAILABLE; i <= 256; i++) SERIAL_RX_BUFFER[i] = '\0';
		if (SERIAL_LIFETIME > 0) {
			if (!(strncmp(SERIAL_RX_BUFFER, TCT_KEEPALIVE_STRING_REQUEST, strlen(TCT_KEEPALIVE_STRING_REQUEST)))) {
				Serial.println(TCT_KEEPALIVE_STRING_ACK);
				SERIAL_LIFETIME = 60 * 1000UL;
			}
			else if (!(strncmp(SERIAL_RX_BUFFER, TCT_DISPLAY_REQUEST_STRING, strlen(TCT_DISPLAY_REQUEST_STRING) - 2))) {
				sscanf(SERIAL_RX_BUFFER, TCT_DISPLAY_REQUEST_STRING, &TCT_DISPLAY_SELECTED_LINE);
				if (TCT_DISPLAY_SELECTED_LINE >= 0 && TCT_DISPLAY_SELECTED_LINE < LCD_NUM_LINES) {
					memset(SERIAL_TX_BUFFER, 0, 64+1);
					snprintf(SERIAL_TX_BUFFER, 64, TCT_DISPLAY_READY_STRING, TCT_DISPLAY_SELECTED_LINE);
					Serial.println(SERIAL_TX_BUFFER);
				}
				else {
					TCT_DISPLAY_SELECTED_LINE = -1;
					Serial.println(TCT_DISPLAY_NREADY_STRING);
				}
				SERIAL_LIFETIME = 60 * 1000UL;
			}
			else if (!(strncmp(SERIAL_RX_BUFFER, TCT_SCROLL_LINE_REQUEST_STRING, strlen(TCT_SCROLL_LINE_REQUEST_STRING) - 6))) {
				unsigned short scroll_request_line;
				int scroll_request_flag;
				sscanf(SERIAL_RX_BUFFER, TCT_SCROLL_LINE_REQUEST_STRING, &scroll_request_line, &scroll_request_flag);
				if (scroll_request_line >= 0 && scroll_request_line < LCD_NUM_LINES) {
					line_scrolling[scroll_request_line] = (scroll_request_flag ? true : false);
					memset(SERIAL_TX_BUFFER, 0, 64+1);
					snprintf(SERIAL_TX_BUFFER, 64, TCT_SCROLL_LINE_ACK_STRING, scroll_request_line, line_scrolling[scroll_request_line]);
					Serial.println(SERIAL_TX_BUFFER);
					is_scrolling = false;
					SERIAL_LIFETIME = 60 * 1000UL;
				}
				else Serial.println(TCT_SCROLL_LINE_NACK_STRING);
			}
			else if (TCT_DISPLAY_SELECTED_LINE >= 0 && TCT_DISPLAY_SELECTED_LINE < LCD_NUM_LINES) {
				strncpy(DISPLAY_BUFFER[TCT_DISPLAY_SELECTED_LINE], SERIAL_RX_BUFFER, 256);
				lcd.setCursor(0, TCT_DISPLAY_SELECTED_LINE);
				strncpy(DISPLAY_VISIBLE[TCT_DISPLAY_SELECTED_LINE], DISPLAY_BUFFER[TCT_DISPLAY_SELECTED_LINE], LCD_NUM_COLS);
				for (unsigned short i = strlen(DISPLAY_VISIBLE[TCT_DISPLAY_SELECTED_LINE]); i < LCD_NUM_COLS; i++) DISPLAY_VISIBLE[TCT_DISPLAY_SELECTED_LINE][i] = ' ';
				DISPLAY_VISIBLE[TCT_DISPLAY_SELECTED_LINE][LCD_NUM_COLS] = '\0';
				lcd.print(DISPLAY_VISIBLE[TCT_DISPLAY_SELECTED_LINE]);
				SERIAL_LIFETIME = 60 * 1000UL;
				longest_length = tct_longest_line();
				is_scrolling = false;
			}
			else if (!(strncmp(SERIAL_RX_BUFFER, TCT_LED_REQUEST_STRING, strlen(TCT_LED_REQUEST_STRING) - 2))) {
				int requested_interval;
				if (sscanf(SERIAL_RX_BUFFER, TCT_LED_REQUEST_STRING, &requested_interval) != 1) {
					Serial.println(TCT_LED_NACK_STRING);
				}
				else if (requested_interval > SHRT_MAX || requested_interval < SHRT_MIN) {
					Serial.println(TCT_LED_NACK_STRING);
				}
				else {
					assert(requested_interval <= SHRT_MAX && requested_interval >= SHRT_MIN);
					LED_INTERVAL = requested_interval;
					memset(SERIAL_TX_BUFFER, 0, 64+1);
					snprintf(SERIAL_TX_BUFFER, 64, TCT_LED_ACK_STRING, LED_INTERVAL);
					Serial.println(SERIAL_TX_BUFFER);
				}
				SERIAL_LIFETIME = 60 * 1000UL;
			}
			else if (!(strncmp(SERIAL_RX_BUFFER, TCT_HANDSHAKE_STRING_RELEASE, strlen(TCT_HANDSHAKE_STRING_RELEASE)))) {
				SERIAL_LIFETIME = 0;
				TCT_HANDSHAKE_STATUS = TCT_HANDSHAKE_STATUS_NONE;
				memset(SERIAL_TX_BUFFER, 0, 64+1);
				snprintf(SERIAL_TX_BUFFER, 64, TCT_HANDSHAKE_STRING_STATUS, TCT_HANDSHAKE_STATUS);
				Serial.println(SERIAL_TX_BUFFER);
				tct_set_mode(MODE_KEYBOARD);
			}
			else if (!(strncmp(SERIAL_RX_BUFFER, TCT_HANDSHAKE_STRING_BEGIN, 7))) {
				// sent TCT_HANDSHAKE_* string, but handshake has already been completed?!
				memset(SERIAL_TX_BUFFER, 0, 64+1);
				snprintf(SERIAL_TX_BUFFER, 64, TCT_HANDSHAKE_STRING_STATUS, TCT_HANDSHAKE_STATUS);
				Serial.println(SERIAL_TX_BUFFER);
			}
			else Serial.println(TCT_ILLEGAL_INSTRUCTION_STRING);
		} // else attempt a handshake
		else {
			if (!(strncmp(SERIAL_RX_BUFFER, TCT_HANDSHAKE_STRING_BEGIN, strlen(TCT_HANDSHAKE_STRING_BEGIN)))) {
				/*TCT_HANDSHAKE_CHALLENGE = random(LONG_MAX);
				TCT_HANDSHAKE_CHALLENGE <<= 9;
				TCT_HANDSHAKE_CHALLENGE ^= random(512); // :((( WHY IS THIS NECESSARY?!*/
				TCT_HANDSHAKE_CHALLENGE = tct_create_challenge();
				tct_issue_challenge(TCT_HANDSHAKE_CHALLENGE);
				/*memset(SERIAL_TX_BUFFER, 0, 64+1);
				snprintf(SERIAL_TX_BUFFER, 64, TCT_HANDSHAKE_STRING_CHALLENGE, TCT_HANDSHAKE_CHALLENGE);
				Serial.println(SERIAL_TX_BUFFER);
				TCT_HANDSHAKE_STATUS = TCT_HANDSHAKE_STATUS_CHALLENGED;*/
			}
			else switch (TCT_HANDSHAKE_STATUS) {
				case TCT_HANDSHAKE_STATUS_CHALLENGED:
				unsigned long TCT_HANDSHAKE_RESPONSE_EXPECTED = TCT_HANDSHAKE_CHALLENGE ^ TCT_HANDSHAKE_CHALLENGE_SECRET;
				char TCT_HANDSHAKE_STRING_RESPONSE_EXPECTED[34+1];
				snprintf(TCT_HANDSHAKE_STRING_RESPONSE_EXPECTED, 34, TCT_HANDSHAKE_STRING_RESPONSE, TCT_HANDSHAKE_RESPONSE_EXPECTED);
				if (!(strncmp(SERIAL_RX_BUFFER, TCT_HANDSHAKE_STRING_RESPONSE_EXPECTED, strlen(TCT_HANDSHAKE_STRING_RESPONSE_EXPECTED)))) {
					tct_set_mode(MODE_SERIAL);
					Serial.println(TCT_HANDSHAKE_STRING_ACCEPT);
					SERIAL_LIFETIME = 60 * 1000UL;
					TCT_HANDSHAKE_STATUS = TCT_HANDSHAKE_STATUS_ACTIVE;
				}
				else if (!(strncmp(SERIAL_RX_BUFFER, TCT_HANDSHAKE_STRING_RESPONSE, strlen(TCT_HANDSHAKE_STRING_RESPONSE) - 5))) {
					if (++TCT_HANDSHAKE_CHALLENGE_ATTEMPTS < TCT_HANDSHAKE_CHALLENGE_MAX_ATTEMPTS) {
						Serial.println(TCT_HANDSHAKE_STRING_REJECT);
						//memset(SERIAL_TX_BUFFER, 0, 64+1);
						//snprintf(SERIAL_TX_BUFFER, 64, TCT_HANDSHAKE_STRING_CHALLENGE, TCT_HANDSHAKE_CHALLENGE);
						//Serial.println(SERIAL_TX_BUFFER);
					}
					else {
						//Serial.println(TCT_HANDSHAKE_STRING_REJECT);
						//TCT_HANDSHAKE_STATUS = TCT_HANDSHAKE_STATUS_NONE;
						TCT_HANDSHAKE_CHALLENGE = tct_create_challenge();
						tct_issue_challenge(TCT_HANDSHAKE_CHALLENGE);
						TCT_HANDSHAKE_CHALLENGE_ATTEMPTS = 0;
					}
				}
				else {
					memset(SERIAL_TX_BUFFER, 0, 64+1);
					snprintf(SERIAL_TX_BUFFER, 64, TCT_HANDSHAKE_STRING_STATUS, TCT_HANDSHAKE_STATUS);
					Serial.println(SERIAL_TX_BUFFER);
				}
				break;

				case TCT_HANDSHAKE_STATUS_ACTIVE:
				default:
				memset(SERIAL_TX_BUFFER, 0, 64+1);
				snprintf(SERIAL_TX_BUFFER, 64, TCT_HANDSHAKE_STRING_STATUS, TCT_HANDSHAKE_STATUS);
				Serial.println(SERIAL_TX_BUFFER);
				break;
			}
		}
	}
	
	for (unsigned short i = 0; i < alen(BUTTONS_ARRAY); i++) {
		if ((digitalRead(BUTTONS_ARRAY[i]) == HIGH) != button_status[i]) {
			if (button_status[i]) {
				if (DEVICE_MODE == MODE_SERIAL) {
					memset(SERIAL_TX_BUFFER, 0, 64+1);
					snprintf(SERIAL_TX_BUFFER, 64, TCT_BUTTON_RELEASE_STRING, BUTTONS_ARRAY[i]);
					Serial.println(SERIAL_TX_BUFFER);
				}
				button_status[i] = false;
			}
			else {
				if (DEVICE_MODE == MODE_KEYBOARD) {
					Keyboard.write((char)(100+BUTTONS_ARRAY[i]));
				}
				else if (DEVICE_MODE == MODE_SERIAL) {
					memset(SERIAL_TX_BUFFER, 0, 64+1);
					snprintf(SERIAL_TX_BUFFER, 64, TCT_BUTTON_DOWN_STRING, BUTTONS_ARRAY[i]);
					Serial.println(SERIAL_TX_BUFFER);
				}
				button_status[i] = true;
			}
		}
	}
	delay(20);
	
	if (millis() < time) { // overflow!!
		time = millis();
		scroll_begin = 0;
		is_scrolling = false;
		return;
	}
	dtime = millis() - time;
	time = millis();
	if (SERIAL_LIFETIME > dtime) SERIAL_LIFETIME -= dtime;
	else {
		SERIAL_LIFETIME = 0;
		if (TCT_HANDSHAKE_STATUS == TCT_HANDSHAKE_STATUS_ACTIVE) {
			tct_set_mode(MODE_KEYBOARD);
			Serial.println(TCT_TIMEOUT_STRING);
			TCT_HANDSHAKE_STATUS = TCT_HANDSHAKE_STATUS_NONE;
		}
	}
	tct_scroll_display();
	tct_blink_led();
}

void tct_set_mode(short mode) {
	DEVICE_MODE = mode;
	switch (mode) {
		case MODE_SERIAL:
		digitalWrite(PIN_LED_GREEN, HIGH);
		break;

		case MODE_KEYBOARD:
		digitalWrite(PIN_LED_GREEN, LOW);
		
		default:
		break;
	}
}

void lcd_clear_row() {
	for (short i = 0; i < LCD_NUM_COLS; i++) lcd.print(" ");
}

void tct_scroll_display() {
	if (!is_scrolling) {
		is_scrolling = true;
		scroll_begin = millis();
		scroll_offset = 0;
		return;
	}
	unsigned long scroll_offset_new;
	if ((scroll_offset_new = millis() - scroll_begin) < 5000) return; // do not scroll for 5 seconds
	scroll_offset_new -= TCT_DISPLAY_SCROLL_DELAY;
	scroll_offset_new /= TCT_DISPLAY_SCROLL_SPEED; // scroll once every 500ms
	if (scroll_offset_new <= scroll_offset) return;
	scroll_offset = scroll_offset_new;
	for (unsigned short i = 0; i < LCD_NUM_LINES; i++) {
		if (!line_scrolling[i]) continue;
		unsigned short current_line_len = strlen(DISPLAY_BUFFER[i]);
		if (current_line_len > LCD_NUM_COLS) {
			if (scroll_offset > (current_line_len - LCD_NUM_COLS)) continue;
			strncpy(DISPLAY_VISIBLE[i], DISPLAY_BUFFER[i] + scroll_offset, LCD_NUM_COLS);
			lcd.setCursor(0,i);
			lcd.print(DISPLAY_VISIBLE[i]);
		}
	}
	if (scroll_offset > (longest_length + 10 - LCD_NUM_COLS)) {
		for (unsigned short i = 0; i < LCD_NUM_LINES; i++) {
			strncpy(DISPLAY_VISIBLE[i], DISPLAY_BUFFER[i], LCD_NUM_COLS);
			lcd.setCursor(0,i);
			lcd.print(DISPLAY_VISIBLE[i]);
		}
		is_scrolling = false;
	}
}

void tct_blink_led() {
	switch (LED_INTERVAL) {
		case -1:
		if (LED_TOGGLE) break;
		else {
			LED_TOGGLE = true;
			digitalWrite(PIN_LED_RED, HIGH);
		}
		break;

		case 0:
		if (!LED_TOGGLE) break;
		else {
			LED_TOGGLE = false;
			digitalWrite(PIN_LED_RED, LOW);
		}
		break;

		default:
		unsigned short t = (millis() / LED_INTERVAL) % 2;
		digitalWrite(PIN_LED_RED, t ? HIGH : LOW);
		LED_TOGGLE = t ? true : false;
		break;
	} 
}

unsigned short tct_longest_line() {
	unsigned short longest = 0;
	for (unsigned short i = 0; i < LCD_NUM_LINES; i++) {
		if (strlen(DISPLAY_BUFFER[i]) > longest) longest = strlen(DISPLAY_BUFFER[i]);
	}
	return min(256, longest);
}

unsigned long tct_create_challenge() {
	unsigned long challenge = random(LONG_MAX);
	challenge <<= 9;
	challenge ^= random(512); // :((( WHY IS THIS NECESSARY?!
	return challenge;
}

void tct_issue_challenge(unsigned long challenge) {
	memset(SERIAL_TX_BUFFER, 0, 64+1);
	snprintf(SERIAL_TX_BUFFER, 64, TCT_HANDSHAKE_STRING_CHALLENGE, challenge);
	Serial.println(SERIAL_TX_BUFFER);
	TCT_HANDSHAKE_STATUS = TCT_HANDSHAKE_STATUS_CHALLENGED;
}
