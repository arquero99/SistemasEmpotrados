/*
 * File:   practicaEntradaSalida1.c
 * Author: alumno
 *
 * Created on 29 de octubre de 2022, 10:33
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
#pragma config IOL1WAY = ON             // Peripheral Pin Select Configuration (Allow Only One Re-configuration)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor (Both Clock Switching and Fail-Safe Clock Monitor are disabled)

// FWDT
#pragma config WDTPOST = PS4096        // Watchdog Timer Postscaler (1:32,768)
#pragma config WDTPRE = PR128           // WDT Prescaler (1:128)
#pragma config WINDIS = OFF             // Watchdog Timer Window (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog timer enabled/disabled by user software)

// FPOR
#pragma config FPWRT = PWR128           // POR Timer Value (128ms)
#pragma config ALTI2C = OFF             // Alternate I2C  pins (I2C mapped to SDA1/SCL1 pins)
#pragma config LPOL = ON                // Motor Control PWM Low Side Polarity bit (PWM module low side output pins have active-high output polarity)
#pragma config HPOL = ON                // Motor Control PWM High Side Polarity bit (PWM module high side output pins have active-high output polarity)
#pragma config PWMPIN = OFF             // Motor Control PWM Module Pin Mode bit (PWM module pins controlled by PWM module at device Reset)

// FICD
#pragma config ICS = PGD1               // Comm Channel Select (Communicate on PGC1/EMUC1 and PGD1/EMUD1)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG is Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.



#include "xc.h"

void init_micro(void)
{
    RCONbits.SWDTEN = 1; // Disable Watchdog Timer

    // Configure Oscillator to operate the device at 40 Mhz
    // Fosc = Fin*M/(N1*N2), Fcy = Fosc/2
    // Fosc = 7.3728*XX/(X*X) = 79.2576 Mhz
    // Fcy = Fosc/2 = 39.6288 MHz

    // Configure PLL prescaler, PLL postscaler and PLL divisor

    PLLFBDbits.PLLDIV = 41; // M =  XX -> PLLDIV = XX - 2 = XX
    CLKDIVbits.PLLPOST = 0; // N2 = 2*(PLLPOST + 1) ? PLLPOST = (N2 / 2) - 1 (Output/N2)
    CLKDIVbits.PLLPRE = 0; // N1 = X + 2 (Input/N1)
    
    // clock switching to incorporate PLL
    __builtin_write_OSCCONH(0x03); // Initiate Clock Switch to Primary
    __builtin_write_OSCCONL(0x01); // Start clock switching

    while (OSCCONbits.COSC != 0b011); // Wait for Clock switch to occur
    while (OSCCONbits.LOCK != 1) {}; // Wait for PLL to lock (If LOCK = 1 -> PLL start-up timer is satisfied)

}

void init_ports(void)
{
    AD1PCFGL = 0xFFFF; //Fuerza los pines A DIGITIALES
    TRISB = 0;
}

/*void __attribute__((__interrupt__(no_auto_psv)))_T1Interrupt(void)
{
    PORTBbits.RB1 = !LATBbits.LATB1;
    TMR1 = 0;
    IFS0bits.T1IF =  0;
}*/
void __attribute__((__interrupt__(no_auto_psv))) _T5Interrupt(void)
{
//Interrupt Service Routine code goes here 
TMR5 = 0x0000; // Clear 32-bit Timer (msw)
TMR4 = 0x0000; // Clear 32-bit Timer (lsw)
PORTBbits.RB1 = !LATBbits.LATB1;
IFS1bits.T5IF = 0; //Clear Timer3 interrupt flag
}

/*void init_tmr1(void)
{
    // Configuraci�n usando Fcyc=79.2576
  
    T1CONbits.TCS = 1; // Select external clock source
    T1CONbits.TSYNC = 1; // Enable Synchronization
    T1CONbits.TCKPS = 0b11; // Select 1:256 Prescaler
    TMR1 = 0x00; // Clear timer register
    PR1 = 30950; // Load the period value
    IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
    IEC0bits.T1IE = 1; // Enable Timer1 interrupt
    T1CONbits.TON = 1;
    */
    /*
    //Configuracion usando reloj 32kHz
    TMR1 = 0x0000;
    PR1 = 57610;//3200; //390

    T1CONbits.TSIDL = 0;
    T1CONbits.TGATE = 0;
    T1CONbits.TCS = 0;
    T1CONbits.TCKPS = 0b10; //11

    IPC0bits.T1IP =  0x01;// Priority level 
    IFS0bits.T1IF =  0;// Clear interrupt flag 
    IEC0bits.T1IE =  1;// Enable T1 interrupts 

    T1CONbits.TON =  1;// Start timer 
}*/

void init_tmr45(void){
T5CONbits.TON = 0; // Stop any 16-bit Timer3 operation
T4CONbits.TON = 0;// Stop any 16/32-bit Timer3 operation
T5CONbits.TSIDL = 0;
T4CONbits.TSIDL = 0;
T4CONbits.T32 = 1; // Enable 32-bit Timer mode
T4CONbits.TCS = 0; // Select internal instruction cycle clock 
T4CONbits.TGATE = 0; // Disable Gated Timer mode
T4CONbits.TCKPS = 0b11;// Select 1:1 Prescaler
TMR5 = 0x0000; // Clear 32-bit Timer (msw)
TMR4 = 0x0000; // Clear 32-bit Timer (lsw)
PR5 = 5; // Load 32-bit period value (msw)
PR4 = 32335; // Load 32-bit period value (lsw)
IFS1bits.T5IF = 0;
IPC2bits.T3IP = 0x01;
IEC1bits.T4IE = 1;
IEC1bits.T5IE = 1;
T4CONbits.TON = 1;
/* Example code for Timer3 ISR*/

}

int main(void) 
{
    init_micro();
    init_ports();
    //init_tmr1();
    init_tmr45();
    
    while(1)
    {
        //PORTBbits.RB1 = !LATBbits.LATB1;
    }
    
    return 0;
}
