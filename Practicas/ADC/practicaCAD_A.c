/*
 * File:   
 * Author: JA
 *
 * 
 * 
 * It reads periodically 1 sensor, and writes the AD conversions results.
 * 
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


int channel, ADC_result;
char temp_byte, ADC_finished, T1_int, rx_byte, tx_byte;

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
    
    /* RA0 = AN0
       RA1 = AN1
       RB0 = AN2 */
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISBbits.TRISB0 = 1;
    AD1PCFGLbits.PCFG0 = 0;
    AD1PCFGLbits.PCFG1 = 0;
    AD1PCFGLbits.PCFG2 = 0;

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
    U1MODEbits.UEN = 0;     // Only RX and TX are used (non CTS, RTS and BCLK)
    U1MODEbits.WAKE = 0;    // Wake up on start bit is disabled
    U1MODEbits.LPBACK = 0;  // Loopback mode disabled
    U1MODEbits.ABAUD = 0;   // No automatic baud rate
    U1MODEbits.URXINV = 0;  // Non polarity inversion. Idle state is 1
    U1MODEbits.BRGH = 0;    // High baude rate disabled
    U1MODEbits.PDSEL = 0;   // 8 bit data with no parity
    U1MODEbits.STSEL = 0;   // One stop bit.
    
    U1STAbits.URXISEL = 0;  // Interrupt on each character received
    
    U1BRG = 257; // 9600 Baudios. 39.6288*10**6/(16*9600) - 1
    
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

void init_ADC(void)
{
AD1CON1bits.AD12B = 0; /* 10 bit conversi?n */
AD1CON2bits.VCFG = 0; /* Vdd and Vss as voltage references */
AD1CON3bits.ADRC = 0 ; /* clock derived from system clock */
AD1CON3bits.ADCS = 2; /* Clock for a TAD of 2 us */
AD1CHS0bits.CH0SA = 0; /* Channel 0 positive input as AN2 */
AD1CHS0bits.CH0NA = 0; /* Channel 0 negative input as -Vref */
AD1CON2bits.CHPS = 0; /* Channel 0 selected for sample and conversion CAMBIAR EN VERSION CON VARIOS CANALES*/
AD1CON2bits.CSCNA = 0 ; /* Do not scan inputs PROBAR */
AD1CON1bits.ASAM = 0; /* Sample begins when SAMP bit is set */
AD1CON1bits.SAMP = 0;
AD1CON1bits.SSRC = 0; /* Clearing sample bit ends sampling and start conversion */
AD1CON1bits.FORM = 0; /* Output format = integer */
AD1CON2bits.BUFM = 0; /* Always starts filling buffer at address 0 */
AD1CON2bits.ALTS = 0; /* Always uses channel input selects for Sample A */
AD1CON2bits.SMPI = 0; /* One conversion per interrupt */
IPC3bits.AD1IP = 7;
IEC0bits.AD1IE = 1;
IFS0bits.AD1IF = 0;

AD1CON1bits.ADON = 1;
}

void init_tmr1(void)
{
    TMR1 = 0;
    PR1 = 51600; /* 3 interrupts per second */

    T1CONbits.TSIDL = 1; /* Stop on idle mode */
    T1CONbits.TGATE = 0; /* Internal clock source */
    T1CONbits.TCS = 0;
    T1CONbits.TCKPS = 3; /* 256 prescaler */

    IPC0bits.T1IP = 6; /* Priority level */
    IFS0bits.T1IF = 0; /* Clear interrupt flag */
    IEC0bits.T1IE = 1; /* Enable T1 interrupts */

    T1CONbits.TON = 1; /* Start timer */
}

void putch(char c)
{
    U1TXREG = c;
 }

void __attribute__((__interrupt__, no_auto_psv)) _ADC1Interrupt(void)
{
    ADC_finished = 1;
    ADC_result = ADCBUF0;
    IFS0bits.AD1IF = 0; //
       
    return;
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void)
{    
    T1_int = 1;
    IFS0bits.T1IF = 0;	/* Clear interrupt flag */
    return;
}

int main(void) 
{
    init_micro();
    init_ports();
    init_uart();
    init_ADC();
    init_tmr1();
    
    /* Set current IPL level */
    SRbits.IPL = 5; // Interrupts of level 6 and 7 are allowed
    
    channel = 0;    
    ADC_finished = 0;
    T1_int = 0;
    
    while(1)
    {
        while(!T1_int); //Wait timer 1
        /* Start sampling */
        
        T1_int = 0;
        
        for (channel = 0; channel < 3; channel++){
            AD1CHS0bits.CH0SA = channel;
            __delay32(40); //Aprox 1us
            AD1CON1bits.SAMP = 1; 
            
            /* Wait adquisition (R = 3250 Ohm, C = 4.4 pF */
            /* They are only needed 3 Tcy but 12 Tcy are de minumum for delay32 */
            __delay32(12);
                   
            /* Start conversion */
            AD1CON1bits.SAMP = 0;
    

            while(!ADC_finished);
            ADC_finished = 0;
            switch(channel){
                case 0: 
                    printf("Temperatura: ");
                    break;
                case 1: 
                    printf("Luminosidad: ");
                    break;
                case 2: 
                    printf("Potenciometro: ");
                    break;
                default: 
                    printf("Invalid Channel");
            }
            printf("%d", ADC_result);
            printf("\r\n");
           }
        printf("**************************************");
        printf("\r\n");
        }
         return 0;
    }
   



