 
/*
 * File:   usart-v0.c
 * Author: norberto
 *
 * Created on 30 de noviembre de 2018, 11:47
 * 
 * Escribe caracteres (del 0 al 9) de forma indefinida
 */

// DSPIC33FJ128MC802 Configuration Bit Settings

// 'C' source line config statements

// FBS

#pragma config BWRP = WRPROTECT_OFF     // Boot Segment Write Protect (Boot Segment may be written)
#pragma config BSS = NO_FLASH           // Boot Segment Program Flash Code Protection (No Boot program Flash segment)
#pragma config RBS = NO_RAM             // Boot Segment RAM Protection (No Boot RAM)

// FSS
#pragma config SWRP = WRPROTECT_OFF     // Secure Segment Program Write Protect (Secure segment may be written)
#pragma config SSS = NO_FLASH           // Secure Segment Program Flash Code Protection (No Secure Segment)
#pragma config RSS = NO_RAM             // Secure Segment Data RAM Protection (No Secure RAM)

// FGS
#pragma config GWRP = OFF               // General Code Segment Write Protect (User program memory is not write-protected)
#pragma config GSS = OFF                // General Segment Code Protection (User program memory is not code-protected)

// FOSCSEL
#pragma config FNOSC = PRI              // Oscillator Mode (Primary Oscillator (XT, HS, EC))
#pragma config IESO = OFF               // Internal External Switch Over Mode (Start-up device with user-selected oscillator source)

// FOSC
#pragma config POSCMD = XT              // Primary Oscillator Source (XT Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function (OSC2 pin has clock out function)
#pragma config IOL1WAY = OFF            // Peripheral Pin Select Configuration (Allow Multiple Re-configurations)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler (1:32,768)
#pragma config WDTPRE = PR128           // WDT Prescaler (1:128)
#pragma config WINDIS = OFF             // Watchdog Timer Window (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog timer enabled/disabled by user software)

// FPOR
#pragma config FPWRT = PWR1             // POR Timer Value (Disabled)
#pragma config ALTI2C = OFF             // Alternate I2C  pins (I2C mapped to SDA1/SCL1 pins)
#pragma config LPOL = ON                // Motor Control PWM Low Side Polarity bit (PWM module low side output pins have active-high output polarity)
#pragma config HPOL = ON                // Motor Control PWM High Side Polarity bit (PWM module high side output pins have active-high output polarity)
#pragma config PWMPIN = ON              // Motor Control PWM Module Pin Mode bit (PWM module pins controlled by PORT register at device Reset)

// FICD
#pragma config ICS = PGD2               // Comm Channel Select (Communicate on PGC2/EMUC2 and PGD2/EMUD2)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG is Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include "xc.h"
#include <libpic30.h>
#include <stdio.h>
#include <stdlib.h>
#define FCY 39626800
#define BAUDRATE 9600
#define BRGVAL ((FCY/BAUDRATE)/16)-1


void init_micro(void)
{
    RCONbits.SWDTEN = 0; // Disable Watchdog Timer

    // Configure Oscillator to operate the device at 40 Mhz
    // Fosc = Fin*M/(N1*N2), Fcy = Fosc/2
    // Fosc = 7.3728*43/(2*2) = 79.2576 Mhz
    // Fcy = Fosc/2 = 39.6288 MHz

    // Configure PLL prescaler, PLL postscaler and PLL divisor

    PLLFBDbits.PLLDIV = 41; // M = PLLDIV + 2 = 43 -> PLLDIV = 43 - 2 = 41
    CLKDIVbits.PLLPOST = 0; // N2 = 2 (Output/2)
    CLKDIVbits.PLLPRE = 0; // N1 = 2 (Input/2)

    // clock switching to incorporate PLL
    __builtin_write_OSCCONH(0x03); // Initiate Clock Switch to Primary
    __builtin_write_OSCCONL(0x01); // Start clock switching

    while (OSCCONbits.COSC != 0b011); // Wait for Clock switch to occur
    while (OSCCONbits.LOCK != 1) {}; // Wait for PLL to lock (If LOCK = 1 -> PLL start-up timer is satisfied)

}

void init_ports(void)
{
    /* All possible analog bits are configured as digital */
    AD1PCFGL = 0xFFFF;

    /* Set TRISx registers for uart */
    TRISBbits.TRISB6 = 1; /* RB6 is configured as input (UART1 RX) */
    TRISBbits.TRISB7 = 0; /* RB7 is configured as output (UART1 TX) */
}

void init_uart(void)
{
    /* Specified pins for UART1 */
    RPINR18bits.U1RXR = 6; /* Pin RP6 asigned to UART1 RX */
    RPOR3bits.RP7R = 0b00011; /* UART1 TX asigned to RP7 */
    
    U1MODEbits.USIDL = 1;   // Stop on idle mode
    U1MODEbits.IREN = 0;    // disable IrDA operation
    U1MODEbits.UEN = 00;     // Only RX and TX are used (non CTS, RTS and BCLK)
    U1MODEbits.WAKE = 0;    // Wake up on start bit is disabled
    U1MODEbits.LPBACK = 0;  // Loopback mode disabled
    U1MODEbits.ABAUD = 0;   // No automatic baud rate
    U1MODEbits.URXINV = 0;  // Non polarity inversion. Idle state is 1
    U1MODEbits.BRGH = 0;    // High baude rate disabled
    U1MODEbits.PDSEL = 00;   // 8 bit data with no parity
    U1MODEbits.STSEL = 0;   // One stop bit.
    
    U1STAbits.URXISEL = 0;  // Interrupt on each character received
    
    U1BRG = BRGVAL; // 9600 Baudios.
    
    /* In this configuration uart interrupts are not allowed */
    IPC2bits.U1RXIP = 0; 
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 0;
    
    U1MODEbits.UARTEN = 1; // Enable UART operation
    U1STAbits.UTXEN = 1;    // Enable uart1 TX (must be done after UARTEN)
    
    /* It is needed to wait one transmision bit to ensure start bit detection 
     When 9600 Baud rate is selected it is necessary to wait 104 us */
    __delay32(4122);
}

void init_tmr45(void)
{
T5CONbits.TON = 0; // Stop any 16-bit Timer3 operation
T4CONbits.TON = 0;// Stop any 16/32-bit Timer3 operation
T4CONbits.T32 = 1; // Enable 32-bit Timer mode
T4CONbits.TCS = 0; // Select internal instruction cycle clock 
T4CONbits.TGATE = 0; // Disable Gated Timer mode
T4CONbits.TCKPS = 0b11;// Select 1:1 Prescaler
TMR5HLD = 0x00; // Clear 32-bit Timer (msw)
TMR4 = 0x00; // Clear 32-bit Timer (lsw)
PR5 = 2; // Load 32-bit period
PR4 = 23728; // Load 32-bit period value (lsw)
IPC7bits.T5IP = 0x03;
IFS1bits.T5IF = 0;
//IEC1bits.T4IE = 1;
IEC1bits.T5IE = 1;
T4CONbits.TON = 1;
/* Example code for Timer3 ISR*/
}

int canWrite = 0;
void __attribute__((__interrupt__(no_auto_psv))) _T5Interrupt(void)
{
//Interrupt Service Routine code goes here
    canWrite = 1;
    IFS1bits.T5IF = 0; //Clear Timer5 interrupt flag
}

char car;

void putch(char c)
{
    U1TXREG = c;
 }


int main(void) 
{
    init_micro();
    init_ports();
    //init_tmr45();
    init_uart();
        
    while(1)
    {
       while(U1STAbits.UTXBF);
       printf("Hola\n");
    }
    
}


