#include <msp430.h>

unsigned char secuencia[] = {12, 6, 3, 9};  // 1100 0110 0011 1001 (para mover el motor, los pasos)
unsigned char i = 0;
volatile unsigned short ADC_result;

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    TA0CCTL0 &=~(1<<0);                 // Apagamos bandera de interrup, 0 en bit menos significativo
    TA0CCR0 = TAR + 2500;
    P2OUT = secuencia[i++%4];
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer
// GPIO
    P2DIR = 0x0F;                       // 0b0000 1111, 15, BIT3 + BIT2 + BIT1 + BIT0  Ponemos los pines como salida

// TIMERS
    // Config timer
    TA0CTL = (2<<8)+(2<<4)+(3<<6);      // Seleccionamos SMCLK,  Modo continuo, divider 8 (cada ciclo toma 8us (microsegs))

    // El del movimiento
    TA0CCTL0 |= (1<<4);                 // Habilitador local para CCR0 la interrupción
    TA0CCR0 = TAR + 2500;


    // El de las conversiones de ADC
    TA0CCTL1 |= (1<<4);                 // Habilitador local para CCR1 la interrupción
    TA0CCR1 = TAR + 6250;               // 50 ms para que empiece la conversión (50,000us / 8us = 6250 -> 50ms)

// ADC
    ADC10AE0|=(1<<1);                   // A1 será entrada analógica
    ADC10CTL1 =INCH_1;                 // (1<<12) Mux Analógico tome la entrada A1
    ADC10CTL0|= ADC10ON + ADC10IE;      // ADC10ON enciende, ADC10IE habilita interrupción del ADC
    ADC10CTL0|=ENC;                     // ENC=1 (enable conversion)
    __bis_SR_register(GIE);             // Global interrupt enable (biS prender biC apagar)

    while(1);
}
