/* 
* Copyright (C) 2013 Andrzej Surowiec <emeryth@gmail.com>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*
*/


#ifndef AVRDIGIPOT_H_
#define AVRDIGIPOT_H_

void send_pot_value(int8_t value);
inline void set_led_color(uint8_t r, uint8_t g, uint8_t b);
inline void change_led_color(int8_t r, int8_t g, int8_t b);
void rainbow(uint8_t step);
void set_volume(void);
void exec_command(void);
void read_uart(void);
uint8_t color_fade_func(uint16_t step);
inline void write_uart(char chr);
void write_uart_str(char *str,uint8_t size);
inline void power_on(void);
inline void power_off(void);
#endif /* AVRDIGIPOT_H_ */