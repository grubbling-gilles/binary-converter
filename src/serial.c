//
// Created by gilles on 22/03/2020.
//
#ifndef F_CPU
#define F_CPU 16000000UL // or whatever may be your frequency
#endif
#ifndef BAUD
#define BAUD 9600 // or whatever may be your frequency
#endif

#include <util/setbaud.h>
#include <avr/io.h>
#include <stdio.h>
#include "serial.h"

void uart_init(void) {
    // Setting the baud rate is done by writing to the UBRR0H and UBRR0L --
    // use values generating by including util/setbaud
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    // UCSR0A
    // U2X0 is a macro that defines which bit to toggle in UCSR0A to indicating using 2X (i.e. the u2x0 bit) -- simply an int
    // _BV(n) is a macro that performs 1 << n -- result is a string 0001000 with the 1 on the n'th position
    // if USE_2X, turn this bit on by performing OR operations
    // else, turn it off by performing AND with it's negation 11101111 -- this will leave all other bits unchanged
#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

void uart_putchar(char c, FILE *stream){
    if (c == '\n') {
        uart_putchar('\r', stream);
    }

    /*
     * loop_until_bit_is_set is macro that does nothing until the relevant bit is set
     * UDRE0: USART Data Register Empty in UCSR0A register
     * UCSR0A is a macro that defines address of this register (?), UDRE0 is macro that defines index of the UDRE0 bit
     * in this register
     */
    loop_until_bit_is_set(UCSR0A, UDRE0);

    /*
     * To write the char, set the UDR0 register to the char
     * UDR0 USAR Data Register 0 -- USART I/O data register
     */
    UDR0 = c;
}

char uart_getchar(FILE *stream) {
    /*
     * UCSR0A USART Control and Status Register 0 A
     * RXC0 -- USART Receive Complete
     */
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}
