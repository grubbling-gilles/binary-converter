//
// Created by gilles on 22/03/2020.
//

#ifndef BLINKING_LED_SERIAL_H
#define BLINKING_LED_SERIAL_H
#include <stdio.h>

void uart_init(void);

void uart_putchar(char c, FILE *stream);

char uart_getchar(FILE *stream);

#endif //BLINKING_LED_SERIAL_H
