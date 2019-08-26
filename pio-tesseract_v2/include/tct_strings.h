#ifndef _TESSERACT_V2_TCT_STRINGS_H
#define _TESSERACT_V2_TCT_STRINGS_H

/*
	This file contains definitions of almost all strings used by TESSERACT v2
*/

const char TCT_VERSION_STRING[] = "TESSERACT v2";
const char TCT_COPYRIGHT_STRING[] = "(c) 2019 by tnbp";

/*
	 strings to do with the HANDSHAKE 
*/

// string from the host to begin a handshake
const char TCT_HANDSHAKE_STRING_BEGIN[] = "TCT_HS_BEGIN";
// string sent by TESSERACT v2 as a challenge; ends in a random 32-bit hexadecimal integer
const char TCT_HANDSHAKE_STRING_CHALLENGE[] = "TCT_HS_CHG:0x%08lx";
// string returned by the host as a response: ends in a 32-bit hexadecimal integer that the host has calculated from the challenge
const char TCT_HANDSHAKE_STRING_RESPONSE[] = "TCT_HS_RSP:0x%08lx";
// string sent by TESSERACT v2 to acknowledge successful handshake (response was correct)
const char TCT_HANDSHAKE_STRING_ACCEPT[] = "TCT_HS_ACK";
// string sent by TESSERACT v2 to reject handshake (response was incorrect)
const char TCT_HANDSHAKE_STRING_REJECT[] = "TCT_HS_REJ";
// string sent by TESSERACT v2 to inform host of inappropriate handshake request
const char TCT_HANDSHAKE_STRING_STATUS[] = "TCT_HS_STATUS:0x%02x";
// string sent to TESSERACT v2 by the host to end session
const char TCT_HANDSHAKE_STRING_RELEASE[] = "TCT_HS_REL";
/*
	strings to do with serial connection KEEPALIVE
*/

// string sent by the host to renew serial connection (set timeout to 60s)
const char TCT_KEEPALIVE_STRING_REQUEST[] = "TCT_KA_REQ";
// string sent by TESSERACT v2 to acknowledge successful renewal
const char TCT_KEEPALIVE_STRING_ACK[] = "TCT_KA_ACK";

// string sent by TESSERACT v2 when the serial connection times out (new handshake required)
const char TCT_TIMEOUT_STRING[] = "TCT_TIMEOUT";

/*
	strings to do with LCD display control
*/

// string sent by the host to take control of LCD display line; ends in 1-byte hexadecimal integer specifying line number
const char TCT_DISPLAY_REQUEST_STRING[] = "TCT_DSP_REQ:0x%x";
// string returned by TESSERACT v2 to acknowledge host control; ends in 1-byte hexadecimal integer specifying line number
const char TCT_DISPLAY_READY_STRING[] = "TCT_DSP_RDY:0x%02x";
// string returned by TESSERACT v2 to inform host of unsuccessful request for control of LCD display
const char TCT_DISPLAY_NREADY_STRING[] = "TCT_DSP_NRDY";
// string returned by TESSERACT v2 to inform host that message has been received and displayed on active LCD display line
const char TCT_DISPLAY_ACK_STRING[] = "TCT_DSP_ACK";

// string sent by host to control LCD display lines scolling or not;
// ends in 1-byte hexadecimal integer specifying line number; semicolon; and scrolling flag (0 = OFF, anything else = ON)
const char TCT_SCROLL_LINE_REQUEST_STRING[] = "TCT_SCR_LN_REQ:0x%02x;%d";
// string returned by TESSERACT v2 to acknowledge the host changing line scrolling (see above)
const char TCT_SCROLL_LINE_ACK_STRING[] = "TCT_SCR_LN_ACK:0x%02x;%d";
// string returned by TESSERACT v2 to inform host of unsuccessful request for changing line scrolling
const char TCT_SCROLL_LINE_NACK_STRING[] = "TCT_SCR_LN_NACK";
/*
	strings to do with LED_RED control
*/

// string sent by the host to control the red LED; ends in signed short integer specifying blink interval or toggle it on/off
// -1: always ON; 0: always OFF; any other value: interval between ON/OFF
const char TCT_LED_REQUEST_STRING[] = "TCT_LED_REQ:%d";
// string returned by TESSERACT v2 to acknowledge interval sent by host
const char TCT_LED_ACK_STRING[] = "TCT_LED_ACK:%d";
// string returned by TESSERACT v2 to inform host of unsuccessful request for interval to be changed
const char TCT_LED_NACK_STRING[] = "TCT_LED_NACK";

// string sent by TESSERACT v2 to inform host of illegal instruction
const char TCT_ILLEGAL_INSTRUCTION_STRING[] = "TCT_ILL_INST";

/*
	strings to do with BUTTON up/down events
*/

// string sent by TESSERACT v2 to inform host of button being pressed; ends in 1-byte hexadecimal integer specifying button code 
const char TCT_BUTTON_DOWN_STRING[] = "TCT_BTN_D:0x%02x";
// string sent by TESSERACT v2 to inform host of button being released; ends in 1-byte hexadecimal integer specifying button code
const char TCT_BUTTON_RELEASE_STRING[] = "TCT_BTN_U:0x%02x";

#endif
