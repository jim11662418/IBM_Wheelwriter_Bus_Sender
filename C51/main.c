//-------------------------------------------------------------------------------------------------------
// Send three digit hex number entered from serial port as a command on the Wheelwriter
// BUS to the Printer board.
//
// for the Keil C51 compiler.
//--------------------------------------------------------------------------------------------------------
// Copyright 2020-2025 Jim Loos
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//----------------------------------------------------------------------------------------------------------
// For use as the 'console', configure Teraterm (or other terminal emulator) for
// 9600bps, N-8-1, no flow control.
//----------------------------------------------------------------------------------------------------------
// Use the following procedure to load object code into the DS89C440 flash memory:
//  1. Configure Teraterm for 4800 bps, N-8-1, no flow control.
//  2. Close the jumper to enable the DS89C440 bootloader.
//  3. Press Enter. The board should respond with "DS89C440 LOADER VERSION 2.1...".
//  4. At the bootloader prompt, type 'K' to clear flash memory followed by 'LB' to load the object code.
//  5. Use the Teraterm 'Send file' function to send the hex object file.
//  6. The DS89C440 Loader will respond with 'G' for each record received and programmed without error.
//  7. Re-configure Teraterm for 9600 bps, N-8-1 and RTS/CTS flow control.
//  8. Remove the jumper to disable the bootloader and restart the application.
//----------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <reg420.h>
#include "uart12.h"
#include "watchdog.h"

#define FALSE 0
#define TRUE  1

sbit switch1 =  P0^0;                   // dip switch connected to pin 39 0=on; 1=off (not used)
sbit switch2 =  P0^1;                   // dip switch connected to pin 38 0=on, 1=off (not used)
sbit switch3 =  P0^2;                   // dip switch connected to pin 37 0=on, 1=off (not used)
sbit switch4 =  P0^3;                   // dip switch connected to pin 36 0=on, 1=off (not used)  

sbit redLED =   P0^4;                   // red LED connected to pin 35 0=on, 1=off
sbit amberLED = P0^5;                   // amber LED connected to pin 34 0=on, 1=off
sbit greenLED = P0^6;                   // green LED connected to pin 33 0=on, 1=off

code char title[] =        "Wheelwriter BUS Sender Version 1.0.0\n"
                           "for Maxim DS89C440 MCU and Keil C51\n"
                           "Compiled on " __DATE__ " at " __TIME__"\n"
                           "Copyright 2020-2025 Jim Loos\n";

code char instructions[] = "Enter three hex digits followed by <CR>. For example: 121<CR>";

//-----------------------------------------------------------
// main()
//-----------------------------------------------------------
void main(){
    unsigned int loopcounter;           // counter for green LED
    unsigned char c,cnt;                // character received from serial0 and count of characters
    char line[5];                       // 4 character (plus null) string which contains input from serial0
    char *lineptr;                      // working pointer
    char *start = line;                 // pointer to the start of 'line'

    disable_watchdog();
    
	EA = 1;    			                // Enable Global Interrupt Flag

    PMR |= 0x01;                        // enable internal SRAM MOVX memory

    greenLED = 1;                       // turn off the green LED connected to T0 (pin 14)
    amberLED = 1;                       // turn off the amber LED connected to INT1 (pin 13)

    init_serial0(9600);                 // initialize serial 0 for mode 1 at 9600bps
    init_serial1();                     // initialize serial 1 for mode 2      

    printf("\r\n\n%s\n",title);
    switch (WDCON & 0x44) {
        case 0x00:
            printf("External reset\r\n");
            break;
        case 0x04:
            printf("Watchdog reset\r\n");
            break;
        case 0x40:
            printf("Power on reset\r\n");
            break;
        default:
            printf("Unknown reset\r\n");
    } // switch (WDCON & 0x44)

    clr_flags();                                            // clear watchdog reset and POR flags for next time
    init_watchdog(3);                                       // WD interval = 5592.4 milliseconds
    enable_watchdog();                                      // enable watchdog reset

    printf("\n%s\r\n",instructions);
    putchar('>');                                          // prompt for input

    lineptr = start;                                        // initialize pointer to beginning of 'line'
    cnt = 0;                                                // and the count
            
    while(TRUE) {

        if (!loopcounter++) {                               // every 65536 times through the loop (at about 2Hz)...
            reset_watchdog();                               // "pet" the watchdog
            greenLED = !greenLED;                           // toggle the green LED
        }

	    if (WWdata_avail()) {                               // if there's data from the Wheelwriter...
            putchar(0x08);                                  // back up the cursor one position
            printf(".%03X\n",get_WWdata());                 // print the data as three hex digits preceded by"."
            putchar('>');                                   // prompt for input
	    }

        if (char_avail()) {                                 // if there's a character from the serial port...
            c=getchar();                                    // retrieve the character

            if (c==0x0D) {                                  // carriage return?
                if (cnt) {                                  // if there's something in the buffer
                    putchar(c);                             // echo it
                    *lineptr = 0;                           // terminate the string with null
                    amberLED = 0;                           // turn on the amber LED
                    send_WWdata((int)strtol(line,NULL,16)); // send it as a 9 bit command to the Wheelwriter
                    amberLED = 1;                           // turn off the amber LED
                    lineptr = start;                        // reset the pointer back to the beginning of the line
                    cnt = 0;                                // and the count
                    putchar('>');                           // prompt for next command to Wheelwriter
                }
            }
            else if (c==0x08 || c==0x7F) {                  // backspace or delete?
                if (cnt) {                                  // if there's anything in the string
                    lineptr--;                              // decrement the pointer
                    cnt--;                                  // and the count
                    putchar(0x08);                          // backspace to the previous character
                    putchar(' ');                           // erase it (overwrite with 'space')    
                    putchar(0x08);                          // backspace to the blank
                }
            }
            else {                                          // all other characters    
                if (isxdigit(c) && cnt<4) {                 // if it's a valid hex digit and there's room in the string
                    putchar(c);                             // echo the character
                    *lineptr = c;                           // store it
                    lineptr++;                              // increment the pointer
                    cnt++;                                  // and the count
                }
            }
        }
    }
}
