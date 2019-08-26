#ifndef _TESSERACT_V2_TCT_CONST_H
#define _TESSERACT_V2_TCT_CONST_H

/*
	PIN constants: Arduino pins to which LEDs, buttons, and the LCD display are connected
*/

const short PIN_LED_GREEN = 18;
const short PIN_LED_RED = 15;

const short PIN_BUTTON_NW = 14;
const short PIN_BUTTON_W = 16;
const short PIN_BUTTON_SW = 10;

const short PIN_BUTTON_N = 9;
const short PIN_BUTTON_MID = 8;
const short PIN_BUTTON_S = 7;

const short PIN_BUTTON_NE = 6;
const short PIN_BUTTON_E = 5;
const short PIN_BUTTON_SE = 4;

// this array contains all buttons so they can be iterated over
const short BUTTONS_ARRAY[] = { PIN_BUTTON_NE, PIN_BUTTON_E, PIN_BUTTON_SE, PIN_BUTTON_N, PIN_BUTTON_MID, PIN_BUTTON_S, PIN_BUTTON_NW, PIN_BUTTON_W, PIN_BUTTON_SW };

// LCD display is hooked up via I2C
const short PIN_DISPLAY_SDA = 2;
const short PIN_DISPLAY_SCL = 3;

// LCD display parameters: I2C address, 20 characters by 4 lines
const short LCD_I2C_ADDR = 0x27;
const short LCD_NUM_COLS = 20;
const short LCD_NUM_LINES = 4;

// modes that TESSERACT v2 can go into
const short MODE_KEYBOARD = 0;
const short MODE_SERIAL = 1;

// Handshake secret needs to be known to host daemon to establish a serial connection
const unsigned long TCT_HANDSHAKE_CHALLENGE_SECRET = 0xDEADBEEF;
const unsigned short TCT_HANDSHAKE_CHALLENGE_MAX_ATTEMPTS = 3;

// stages that a handshake cand be in
const unsigned short TCT_HANDSHAKE_STATUS_NONE = 0x00;		// default status
const unsigned short TCT_HANDSHAKE_STATUS_BEGUN = 0x01;		// host has sent handshake request
const unsigned short TCT_HANDSHAKE_STATUS_CHALLENGED = 0x02;	// TESSERACT v2 has sent challenge
const unsigned short TCT_HANDSHAKE_STATUS_RESPONDED = 0x03;	// host has responded to challenge
const unsigned short TCT_HANDSHAKE_STATUS_ACTIVE = 0x04;	// handshake has succeeded, serial connection is active

const unsigned short TCT_DISPLAY_SCROLL_DELAY = 5000U;	// milliseconds to wait before starting to scroll a line on the LCD display
const unsigned short TCT_DISPLAY_SCROLL_SPEED = 500U;	// milliseconds to wait before scrolling lines one character to the left

#endif
