#ifndef _TESSERACT_V2_TCT_FUNCTIONS_H
#define _TESSERACT_V2_TCT_FUNCTIONS_H

void tct_set_mode(short);
void tct_lcd_clear_row();
void tct_scroll_display();
void tct_blink_led();
unsigned short tct_longest_line();
unsigned long tct_create_challenge();
void tct_issue_challenge(unsigned long);

#endif
