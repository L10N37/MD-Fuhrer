/*
 * File:   main.c
 * Author: Lionel Sanderson
 *
 * Created on 22 February 2024, 10:24 PM
 */


#include <xc.h>
#include <pic16f628a.h>

// Config bits
#pragma config WDTE = OFF     // Watchdog Timer disabled
#pragma config PWRTE = OFF    // Power-up Timer disabled
#pragma config MCLRE = OFF    // MCLR pin function is digital input
#pragma config CP = OFF       // Code Protection disabled

#define wait_()   while (PORTAbits.RA5 == 0) {;}  // wait for RA5 high again
#define wait()  while (PORTAbits.RA5 == 1) {;}  // wait for RA5 low pulse


void init(void);


void main(void) {
    init();

    while(1) {
        
        wait();
        //Byte 1    
        PORTB = 0B000100001;
        wait_()
                
        wait();
        //Byte 2   
        PORTB = 0B000000001;
        wait_()
        
        wait();
        //Byte 3   
        PORTB = 0B110100001;
        wait_()
                
        wait();
        //Byte 4   
        PORTB = 0B000100101;
        wait_()

        wait();
        //Byte 5   
        PORTB = 0B110111001;
        wait_()
                
        wait();
        //Byte 6   
        PORTB = 0B11000111;
        wait_()
        
        
    // All pins high-z
    TRISA = 0xFF; // Set all pins of PORTA as inputs
    TRISB = 0xFF; // Set all pins of PORTB as inputs
    // Sleep MCU
    SLEEP();
    }

    return;
}

// Function to initialize ports
void init(void) {
    // Configure RA5 as INPUT
    TRISA5 = 1;

    // Configure RA0 as OUTPUT
    TRISA0 = 0;
    // CE PIN HIGH through 1.5k Resistor
    RA0 = 1;

    // Configure RB0 through RB7 as OUTPUT
    TRISB = 0x00;
}
