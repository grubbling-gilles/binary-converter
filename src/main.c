#ifndef F_CPU
#define F_CPU 16000000UL // or whatever may be your frequency
#endif
#ifndef BAUD
#define BAUD 9600 // or whatever may be your frequency
#endif

// port B
#define RS 3
#define ENABLE 4
// port D
#define DB4 2
#define DB5 3
#define DB6 4
#define DB7 5

#include <stdio.h>
#include "serial.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
    
FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

uint8_t querySwitches(void){
    uint8_t switches = (PINC & (_BV(0) | _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5)));
    switches |= (PINB & (_BV(0) | _BV(1))) << 6;
    return switches;
}

void pulseEnable(void) {
    // turn off Enable bit
    PORTB &= ~_BV(ENABLE);
    _delay_us(1);
    PORTB |= _BV(ENABLE);
    _delay_us(1);    // enable pulse must be >450ns
    PORTB &= ~_BV(ENABLE);
    _delay_us(100);    //  commands need > 37us to settle
}

void writeToBus(uint8_t data){
    /*
     * write the 4 least significant in data to DB4-DB7
    */

    // disregard 4 most significant bits
    data &= 0x0f;

    //shift over a number of bits so that data corresponds to DB4
    data = data << DB4;

    // clear PORTD, write data to Port D
    PORTD = 0;
    PORTD = data;
}

/*
 * general LCD functions -- refer to datasheet for more info
 */
void initializeLCD(void){
    // initialization only requires writing data to bus -- only use DB7:4
    // wait at least 40 ms after power on
    _delay_ms(60);

    /*
     * 3 times function set intstruction -- indicate reset
     * (at this point, 8 bit interface still used
     * function set instruction 0011 xxxx (least significant bits ignored)
     * 0011 = 0x3
     */
    writeToBus(0x3);
    pulseEnable();
    _delay_ms(5);

    writeToBus(0x3);
    pulseEnable();
    _delay_ms(1);

    writeToBus(0x3);
    pulseEnable();
    _delay_ms(1);

    /*
     * Set interface to 4 bit -- write 0010 = 0x2 to bus
     */
    writeToBus(0x2);
    pulseEnable();

    // now we are using 4 bit interface -- each instruction consists of sequence of 2 writes to bus

    /*
     * 1. function set instruction 0b0010 = 0x2
     * 2. N=1, (2 rows), F=0 (to use correct font) -- 0b1000 = 0x8
     * specify number of lines, font size -- first 4 bits specify instruction, following 4 are arguments
     */
    writeToBus(0x2);
    pulseEnable();
    writeToBus(0x8);
    pulseEnable();

    /*
     * instruction 0000 1000 -- 0000 1DCB, D=0, C=0, B=0
     * Turn display off, cursor off, blink off
     */
    writeToBus(0x0);
    pulseEnable();
    writeToBus(0x8);
    pulseEnable();

    /*
     * instruction 0000 0001 -- clear screen (takes slightly longer)
     */
    writeToBus(0x0);
    pulseEnable();
    writeToBus(0x1);
    pulseEnable();
    _delay_ms(5);

    /*
     * instruction 0000 01(ID)S --
     * ID=1 -> increment, S=0 -> no shift
     */
    writeToBus(0x0);
    pulseEnable();
    writeToBus(0x6);
    pulseEnable();
    _delay_ms(1);
}

void turnOnLCD(int cursor, int blink){
    // instruction 0000 1DCB (D has to be 1 to turn display on (?))
    // D - set display on/off
    // C - curson on/off
    // B - blink on/off
    writeToBus(0x0);
    pulseEnable();
    writeToBus(_BV(3) | _BV(2) | cursor << 1  | blink);
    pulseEnable();

}

void writeChar(char c){
    // RS bit = 1
    PORTB = 0;
    PORTB |= _BV(RS);

    // write 4 most siginificant bits to bus, then the 4 least significant bits
    writeToBus(c >> 4);
    pulseEnable();
    writeToBus(c);
    pulseEnable();
    PORTB = 0;
}

void clearScreen(void){
    // RS = 0
    // instruction = 0000 0001

    writeToBus(0x0);
    pulseEnable();
    writeToBus(0x1);
    pulseEnable();
    // clearing screen takes longer than other instructions (5 ms is likely overkill though)
    _delay_ms(5);

}

void writeString(char* string){
    for (int i=0; i < strlen(string); i++){
        writeChar(string[i]);
    }
}

void setDDRAMAddress(uint8_t addr){
    // RS = 0, R/W = 0,
    // DB7 = 1, rest = addr
    uint8_t instruction = 1 << 7;
    instruction |= (addr & ~(1<<7));

    writeToBus(instruction >> 4);
    pulseEnable();
    writeToBus(instruction);
    pulseEnable();
}

void shift(int display, int right){
    /*
     * shift cursor or display, left or right
     */
    // RS = 0, 0001 (inc)(cur)xx
    uint8_t instruction = 1 << 4 | (display << 3) | (right << 2);
    writeToBus(instruction >> 4);
    pulseEnable();
    writeToBus(instruction);
    pulseEnable();
}

void displayBits(uint8_t bitState){
    setDDRAMAddress(0);

    char binary[10], hex[10], decimal[10];

    // first construct the binary representation
    binary[0] = '0';
    binary[1] = 'b';
    for (int i = 0; i < 8; i++){
        if ((bitState >> i) & 1){
            binary[9 - i] = '1';
        }
        else{
            binary[9 - i] = '0';
        }
    }

    // for hex and decimal, use sprintf to output formatted string into the array (there is no formatting specifier for binary)
    sprintf(&hex,"0x%.2x", bitState);
    sprintf(&decimal,"0d%.2d", bitState);

    setDDRAMAddress(0);
    writeString(&binary);
    setDDRAMAddress(40);
    writeString(&hex);

    // shift to the right twice, print decimal notation (note, for some reason setDDRAMAddress does not work for addresses > 40)
    shift(0, 1);
    shift(0, 1);
    writeString(&decimal);
}
int main() {

    /*
    * initialize uart
    */

    uart_init();

    stdout = &uart_output;
    stdin = &uart_input;

    // output -- arduino pins D2, D3, D4, D5, D11, D12 = D2, D3, D4, D5, B3, B4 -- set DDRxn to 1 for these pins

    DDRD |= _BV(DB4)| _BV(DB5) |  _BV(DB6) |  _BV(DB7);
    DDRB |= _BV(RS) | _BV(ENABLE) ;

    initializeLCD();
    turnOnLCD(0, 0);

    // keep track of the switch state, 1 if the switch is turned on
    uint8_t switchState = 0;

    // bit state we want to toggle based on switch input
    uint8_t bit_state = 0;

    // use a lock -- when the bit_state is toggled, turn on the lock so that it cannot be toggled again until the
    // switch has been released
    int lock = 0;

    while(1){
        switchState = querySwitches();
        // switch is turned on -- wait 25ms and check if it is still turned on
        if (switchState && !lock){
            _delay_ms(25);
            switchState = querySwitches();
            // if after 25 ms the switchState has not changed, change the bit state
            if (switchState == querySwitches()){
                // all bits that correspond to a turned on switch need to be toggled
                bit_state = bit_state ^ switchState;
                lock = 1;
            }
        }

        // if the lock is turned on, and no switches are turned on, turn off the lock
        // ater first checking if nothing has changed 25 ms later
        else if (lock && !switchState){
            switchState = querySwitches();
            if (!switchState){
                lock = 0;
            }
        }

        // display bit state to LCD screen
        displayBits(bit_state);
    }
}
